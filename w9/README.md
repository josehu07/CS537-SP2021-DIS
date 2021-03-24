# CS537 SP2021 DIS-315 Week 9

Copyright 2021 Guanzhou Hu

Topics:

- xv6 virtual memory design
    - Memory layout
    - Built-in utilities to help you translate between:
        - User virtual address (uva)
        - Physical address (pa)
        - Kernel address (ka)
    - Two-level page table
- P5 (part A) overview & tips
- Page fault trap mechanism

Links:

- Chapter 2 & 3 of the [xv6 book](https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf)

## xv6 Virtual Memory Design

Recall that xv6 is a 32-bit system, so a virtual address is 32-bit in length, able to address up to a 4GB virtual memory address space. xv6 follows a typical "higher-half" kernel memory layout design.

- Each user process has the illustion of its virtual address space, size 4GB (0x00000000 to 2^32-1). User program uses virtual address
- Lower half belongs to the user:
    - Code at the bottom
    - Data starts small but is growable - remember the *stack* and the *heap*
- The kernel owns the higher half (0x80000000 to top), user not allowed to access these parts:
    - Entire physical memory maps onto the begining of the kernel half

### Memory Layout Graph

This figure is an overview of the memory layout:

![VMLayout](higher-half-kernel-vm-layout.png)

**Question: say a user process calls some syscall/traps into the kernel. How does the kernel access/modify the byte at user address `uva`?**

In summary, we have the following three addresses for a user byte:

1. User virtual address (uva): from user's view, the virtual address of the byte in lower half
2. Physical address (pa): where is the byte actually in physical memory
3. Kernel address (ka or kva): == pa + KERNBASE, i.e., the virtual address of the mapped physical byte

**Answer to above question**: the kernel walks the page table to find out the physical address `pa`. Then, simply adds `KERNBASE` to it to get `ka`. Then, access/modify the byte at `ka`.

### Translation Utilities

xv6 has a bunch of utilities for translation:

- pa to ka: `P2V()` (**How?**  Simply `+= KERNBASE 0x80000000`)
- ka to pa: `V2P()` (**How?**  Simply `-= KERNBASE 0x80000000`)
- uva to pa: **Question: who carries out this translation?**
- pa to uva: not necessary
- uva to ka: `uva2ka()`, combines uva-to-pa with pa-to-ka
- Rounding down an address to the page start: `PGROUNDDOWN()`

**Answer to above question**: the page table of current process. See the `vm.c: walkpgdir()`. The entire page table is in memory, but may be cached by the CPU (recall TLBs), so after you modify anything in a page table entry, do `switchuvm(myproc())` to force a reload.

### Two-level Page Table

This part should have been well covered in lectures.

Specifically in xv6, it uses a two-level page table:

- The root level is named the **page directory**
- Each entry of the page directory (`pde`) points to an inner level **page table** (`pte`)
- Each `pte` holds the physical address of that virtual page and some flag bits

To see the definition of a PTE and what each bit means, see `mmu.h`.

```text
// A virtual address:

+--------10------+-------10-------+---------12----------+
| Page Directory |   Page Table   | Offset within Page  |
|      Index     |      Index     |                     |
+----------------+----------------+---------------------+
 \--- PDX(va) --/ \--- PTX(va) --/

// A page table entry:

+---------------20----------------+---------12----------+
|      Physical page number       |       Flags         |
+---------------------------------+---------------------+

#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
```

Page size (so physical frame size as well) in xv6 is <ins>4KB</ins>.

**Quiz time: I have a user virtual address `uva1 = 0x00123000`. Walking the page table gives me `walkpgdir(myproc(), uva1) -> pte == 0x00A71007 == 0000 0000 1010 0111 0001 0000 0000 0111`.**

- What is `PGROUNDDOWN(uva1)`?
- Is this page present?
- Is this page accessible to the user?
- What is the physical address of the user virtual address `uva2 = 0x00123050`?

## P5 (Part A) Overview

The spec is pretty straight-forward. The only tricky thing here is that set the encrypted bit `PTE_E` and we clear the present bit `PTE_P` in order to trigger a page fault later, where we do decryption.

Files you will need to change:

- Adding new syscalls - you are an expert at it
    - Except that this time, internal handlers go in `vm.c`, not `proc.c`
- In `vm.c`, handlers for `mencrypt()` ※, `getpgtable()`, and `dump_rawphymem()`
- In `vm.c`, `mdecrypt()` - this is not a syscall handler, but will be used by the trap handler for implicit decryption
- In `trap.c: trap()`, handle the `T_PGFLT` case where `PTE_E` is set

## Page Fault Trap Mechanism

You already know what is a trap handler, what is a page fault, and what happends when the hardware meets a page fault. Some information you may find useful for this project:

- Trap numbers are defined as macros in `traps.h`
- The trap handler `trap.c: trap()` has good examples of checking on the trap number and making corresponding reactions
- Page faults now just fall into the default case

What you need to add into `trap.c` for decryption:

- Add a checking on the page fault trapno `T_PGFLT`, check if it is an encrpted page (has `PTE_E` set)
- If so, it is not an actual page fault - we just decrypt the page and return
    - You can get the faulty address by `rcr2()`
    - Remember to clear the `PTE_E` bit and set the `PTE_P` bit back so later accesses to the page are no longer faults
- If not, it is an actual page fault - let it fall into the default case
