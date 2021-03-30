# CS537 SP2021 DIS-315 Week 9

Copyright 2021 Guanzhou Hu

Topics:

- A few points to add about P5
- P5 Q&A Session

Links:

- None

## A Few Points to Add about P5

How to flip every bit of a page?

- A `char` type (to be more precise, `unsigned char`) in C is a byte (i.e., 8 bits)
- Flipping a <ins>bit</ins> means doing XOR with a bit of 1: `0 ^ 1 = 1`, `1 ^ 1 = 0`
- Flipping a <ins>byte</ins> means doing bitwise XOR with a byte of `0xFF`: `byte = byte ^ 0xFF;`
    - NOT XORing with a char value of `1`! A char `1` is `b00000001`
- Flipping every bit of a page: simply iterate thorugh every byte:
    ```C
    char *ka = ...;     // How to obtain the kernel address ka for a given user address uva?
                        // See last week's discussion material.
    for (int i = 0; i < PGSIZE; ++i)
        *(ka + i) ^= 0xFF;
    ```

How to set/clear/test a flag bit in the PTE?

```C
pte_t *pte = ...;   // a pointer to a PTE, so *pte is the PTE content
*pte = (*pte) | PTE_P;      // set PTE_P bit
*pte = (*pte) & (~PTE_P);   // clear PTE_P bit
if ((*pte) & PTE_P) {...};  // test PTE_P bit is set
```

How to capture a page fault in the trap handler?

- Make a helper function, e.g., `int decrypt(char *uva);`
    - Returns `0` on success
    - Returns other values on failures, e.g., if the page's PTE does not have `PTE_E` set
- Use the page fault trap number `T_PGFLT`
- The faulty address can be acquired by `rcr2()`
    - A helper that fetches the value of the CR2 register, which contains the faulty address on a page fault
- If `decrypt(rcr2())` is successful, simply return from `trap()`
    - Else, it indiciates a real page fault, so let it fall through to the `default` case of the switch to panic
    - DO NOT do `exit()` inside the kernel, though it might work in xv6 code

## P5 Q&A Session

Please feel free to ask any question on P5 or course material!
