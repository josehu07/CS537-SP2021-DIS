# CS537 SP2021 DIS-315 Week 11

Copyright 2021 Guanzhou Hu

Topics:

- CLOCK algorithm & implementation
- P6 overview & tips

Links:

- Be sure to fully understand this [user-level CLOCK impl](https://github.com/josehu07/CS537-SP2021-DIS/blob/main/w11/clock-example.c) before trying anything in kernel

## CLOCK Algorithm

The **CLOCK alagorithm == "second-chance" FIFO**. It behaves like a FIFO, but when trying to evict a page, we give a page a second chance if it was referenced recently (has ref bit set). In this case, we clear its ref bit and move on to the next one. If all pages in queue were recently referenced, then we eventually loop back to the first page we examined (it now has ref bit cleared) so we still pick it.

There are two ways to implement the CLOCK algorithm:

- As a linked list FIFO, as shown in the spec (*hard to write in xv6 kernel, but easy to do page removal*)
- As a fixed ring buffer + a clock hand index pointing to the current tail (*easy to write in xv6 kernel, but hard to do page removal*)

Feel free to choose whichever way you think the most convenient for you. I will show you an implementation with the second option, which is how most CLOCK algorithms get implemented in cache eviction systems.

Be sure to try out the `clock-example` user-level implementation:

```bash
$ make
$ ./clock-example
```

Go through the code of `clock-example.c`...

## Putting the Clock into xv6 Kernel

Think about the following questions:
- What should the **initial state of pages** of a process be?  **Answer: all encrypted.**
- What happens when you **access a page**?  **Answer:**
  - if the page has `PTE_P` set, then nothing happens, no faults; the page must be in queue and in clear text
  - if there is a fault and the page cannot be found in queue, this is an encrypted page and needs to be inserted into the clock queue, possibly evicting another page
- What **happens on `fork()`**?  **Answer: child copies the exact clock queue of its parent.**

These two places need to be changed as well for P6 (which were not needed in P5):
- What **happens on `exec()`**?  **Answer: clear the clock queue, encrypt all the pages.**
- What **happens on `proc.c: growproc()`**?  **Answer**:
  - if growing (due to, say, `sbrk()`), encrypt all the newly allocated pages
  - if shrinking (deallocating pages), need to remove all those pages from the clock queue

Hardware access bit `PTE_A`:
- Please use the hardware `PTE_A` bit - it will greatly simplify the reference bit handling
- `PTE_A == 0x020` is supported by x86 hardware, but xv6 currently is not using it; add a macro definition in `mmu.h`
- Hardware automatically sets this bit when referencing a page; we only clear this bit in the clock algorithm

## P6 Overview & Tips

P6 spec has not yet been finalized, so please expect small changes. Nonetheless, the CLOCK algorithm will be used so it is a good time to start making a CLOCK implementation.

Tips:
- Be sure to get a user-level CLOCK algorithm working before trying anything in kernel!
- Understand where in xv6 kernel you encrypt/decrypt pages: see the above section. Implementing a CLOCK is straight-forward (if you have a working user-level implementation). Figuring out when to encrypt/decrypt pages and when to interact with the clock is trickier.
