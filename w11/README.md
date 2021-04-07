# CS537 SP2021 DIS-315 Week 11

Copyright 2021 Guanzhou Hu

Topics:

- CLOCK algorithm & implementation
- P6 overview & tips

Links:

- Be sure to fully understand this [user-level CLOCK impl]() before trying anything in kernel

## CLOCK Algorithm

The **CLOCK alagorithm == A "second-chance" FIFO**. It behaves like a FIFO, but when trying to evict a page, we give a page a second chance if it was referenced recently (has ref bit set). In this case, we clear its ref bit and move on to the next one. If all pages in queue were recently referenced, then we eventually loop back to the first page we examined (it now has ref bit cleared) so we still pick it.

There are many ways to implement the CLOCK algorithm:

- As a linked list FIFO (*hard in xv6 kernel*)
- As a fixed ring buffer, but shifting things around to make it behave like a FIFO (*not very efficient*)
- As a fixed ring buffer + a **clock hand** index pointing to the current tail (*classic impl, recommended*)

Feel free to choose whichever way you think the most convenient for you. I will show you an implementation with the third option, which is how most CLOCK algorithms get implemented in cache eviction systems.

Be sure to try out the `clock-example` user-level implementation:

```bash
$ make
$ ./clock-example
```

Go through the code of `clock-example.c`...

## P6 Overview & Tips

Please see the P6 spec.

Be sure to get a user-level CLOCK algorithm working before trying anything in kernel!

Think about the following questions:
- What should the **initial state of pages** of a process be?  Answer: all encrypted.
- What happens when you **access a page**?  Answer:
  - if the page is in-queue and has PTE_P set, then nothing happens, no faults
  - if there is a fault but the page is actually in queue, this happens for a page with cleared PTE_R bit
  - if there is a fault and the page cannot be found in queue, this is an encrypted page and needs to be inserted into the clock queue, possibly evicting another page
