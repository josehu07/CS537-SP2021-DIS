# CS537 SP2021 DIS-315 Week 7

Copyright 2021 Guanzhou Hu

Topics:

- xv6 sleep/wakeup mechanism
  - Its original design
  - What you need to change
- Compensation ticks behavior

Links:

- Scheduler description in the xv6 doc: [Chapter 5](https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf)
- Remzi's discussion section video (old, not the same spec): [scheduler walkthrough](https://www.youtube.com/watch?v=eYfeOT1QYmg)

Make sure you have a **working version of the round-robin scheduler with timeslices** before moving on to handling sleep/wakeups and compensation ticks!

## xv6 Sleep/Wakeup Mechanism

There are typically three cases of *voluntary blocking* of a user process in xv6:

* It calls the `sleep(num_ticks)` syscall, which will be served by the in-kernel function `sys_sleep()`
* It calls the `wait()` syscall, trying to wait for a child process to finish
* It tries to do `read()` on a blocking pipe - a mechanism for doing inter-process communications

A slightly confusing naming here: all these three situations will eventually call an internal helper function, `proc.c: sleep(chan)`, that performs the blocking. Though named `sleep`, this internal helper function is used not only by `sys_sleep()`, but also by all other mechanisms of blocking. I will prefer calling this internal helper function as `block(chan)`.

This internal sleep takes an argument called a *channel* (`chan`). The channel is essentially a pointer value serving as an *identifier*, used by the `wakeup()` function to identify which processes it needs to wake up. The above 3 blocking scenarios block on different `chan`'s:

* `sleep()` syscall - `chan == (int *) &ticks`, where `ticks` is a global variable holding the number of timer ticks elapsed since boot up
* `wait()` syscall - `chan == (strct proc *) curproc`, where `curproc` is a pointer to the calling process's PCB
* `read()` syscall - let's ignore this

We will focus on the `sys_sleep()` code path. The original design of the sleep/wakeup mechanism of xv6 conflicts with our scheduler, so you will need to change how it behaves, in order to enable the compensation ticks feature.

### xv6 Original Design

TL;DR: in original xv6, all sleeping processes blocking on `chan == &ticks` will be waken up at every timer tick, marked as RUNNABLE, and tried to be scheduled. The check of whether or not sleep length has elapsed is performed in `sys_sleep()`, not in the wakeup mechanism.

An example with a single process A on the system, calls the sleep syscall:

```text
User process A:
    sleep(num_ticks) syscall
    sys_sleep() while loop
    ticks_elapsed < num_ticks ? YES
      so sleep(chan = &ticks)

                                ---swtch--->

                                          Scheduler:
                                              no process is RUNNABLE
                                              continue looping

                                          Timer interrupt:
                                              trap() with IRQ0_TIMER
                                              wakeup(all chan == &ticks)
                                              mark A as RUNNABLE

                                          Scheduler:
                                              A is RUNNAABLE! pick A
                                              swtch() -> A

                      [let's call this a "false" wakeup]
                                <---swtch---

User process A:
    wakes up from sleep(chan = &ticks)
    hit the next iter of while loop
    ticks_elapsed < num_ticks ? YES
      so sleep(chan = &ticks)

                                ---swtch--->

                                          Scheduler:
                                              no process is RUNNABLE
                                              continue looping

                                   ......
                  [same pattern repeats until eventually...]

                                          Timer interrupt:
                                              trap() with IRQ0_TIMER
                                              wakeup(all chan == &ticks)
                                              mark A as RUNNABLE

                                          Scheduler:
                                              A is RUNNAABLE! pick A
                                              swtch() -> A

                                <---swtch---

User process A:
    wakes up from sleep(chan = &ticks)
    hit the next iter of while loop
    ticks_elapsed < num_ticks ? NO
      ends the while loop
    return from sys_sleep() syscall
    continue user execution
```

You should be able to infer this logic from the following code pieces:

- `sysproc.c: sys_sleep()`
- `proc.c: sleep()`
- `trap.c: trap()`
- `proc.c: wakeup1()`

### What You Need to Change

Our final goal:

* At the end, `sleep(num_ticks)` needs to function correctly and give out compensation ticks properly,
* We are only testing the compensation behavior using the `sleep()` syscall - though your implementation will probably be able to handle other situations as well, as they all have the same underlying mechanism
* You may safely ignore `read()` for now - though your implementation will probably handle `read()`'s correctly as well

We need to change `sys_sleep()`, `sleep()`, and `wakeup1()` so that **we make the num_ticks check happen in `wakeup1()` and avoid false wakeups**

1. Add fields into PCB, used to remember whether a process is sleeping and its target wakeup time
2. At `sys_sleep()`, write these fields
3. At `wakeup1()`, check if target time has arrived; If not, do not actually make it as RUNNABLE

After the fix, the flowchart looks like:

```text
User process A:
    sleep(num_ticks) syscall
    enters sys_sleep()
    calculate target wakeup time
    sleep(chan = &ticks)

                                ---swtch--->

                                          Scheduler:
                                              no process is RUNNABLE
                                              continue looping

                                          Timer interrupt:
                                              trap() with IRQ0_TIMER
                                              wakeup(all chan == &ticks)
                                              ticks >= target wakeup time ? NO
                                                pass

                                          Scheduler:
                                              no process is RUNNABLE
                                              continue looping

                                   ......
                           [until eventually...]

                                          Timer interrupt:
                                              trap() with IRQ0_TIMER
                                              wakeup(all chan == &ticks)
                                              ticks >= target wakeup time ? YES
                                                now mark A as RUNNABLE

                                          Scheduler:
                                              A is RUNNAABLE! pick A
                                              swtch() -> A

                                <---swtch---

User process A:
    wakes up from sleep(chan = &ticks)
    return from sys_sleep() syscall
    continue user execution
```

### Compensation Ticks

See the P4 spec for examples.

* Compensation is given when a process finishes its sleep, for a number of ticks that it has just slept
* Gets discarded if not used up; Does not accumulate

Implementation should be easy once you have fully understood the sleep/wakeup mechanism and changed it correctly!
