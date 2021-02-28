# CS537 SP2021 DIS-315 Week 6

Copyright 2021 Guanzhou Hu

Topics:

- xv6 scheduling overview
- Basic round-robin scheduling
- P4 scheduler spec & tips

Links:

- Scheduler description in the xv6 doc: [Chapter 5](https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf)
- Remzi's discussion section video (old, not the same spec): [scheduler walkthrough](https://www.youtube.com/watch?v=eYfeOT1QYmg)

## Scheduling in xv6

Let's just assume a single-core OS. On that CPU core, we have the notion of the current **execution context**, which basically means the current values of the following registers:

```C
// proc.h
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;     // backup of stack pointer
  uint eip;     // instruction pointer
};
```

Each CPU core has its current context. The context defines where in code this CPU core is currently running.

To jump between different processes, we "switch" the context - this is called a process **context switch**.
- We save the current running process P1's context somewhere - **To where?**  Into P1's PCB.
- We set the CPU context to be the context we previously saved for the to-be-scheduled process P2 - **From where?**  From P2's PCB.

When the OS boots up and is ready to run a user process, it kicks off the CPU to run the `scheduler()` function in `proc.c`.

```C
// main.c
// Common CPU setup code.
static void
mpmain(void)
{
  cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
  idtinit();       // load idt register
  xchg(&(mycpu()->started), 1); // tell startothers() we're up
  scheduler();     // start running processes
}

// proc.c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    
    for (some kind of loop over all RUNNABLE processes) {
      p = pick_some_process_to_run_next();

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      // This is the context switch.
      swtch(&(c->scheduler), p->context);
      
      // When the above swtch() returns, it means the user process just swtch()'ed
      // back to the scheduler -- Time for the next scheduling decision!
      switchkvm();
    }


    // Process is done running for now.
    // It should have changed its p->state before coming back.
    c->proc = 0;

    release(&ptable.lock);
  }
}
```

The CPU **never leaves** this `scheduler()` function! Whenever it gets the chance to pick a user process to run, it `swtch()` to, i.e., context switches to that user process. At some timepoint, the user process will context switch back to this `scheduler()` function.

**When will a user process swtch back to the scheduler?**  Basically, whenever the process calls `sched()`. There are three cases where a process could come into kernel mode and call `sched()`:

- The process *exits*
- The process goes to *block* voluntarily, examples:
  - It calls the sleep syscall
  - It calls the wait syscall
  - It tries to read on a pipe
- The process "*yield*"s - typically at a timer interrupt
  - At every ~10ms, the timer issues a hardware interrupt
  - This forces the CPU to trap into the kernel, see `trap()` in `trap.c`, the `IRQ_TIMER` case
  - xv6 then increments a global counter named `ticks`, then `yield()` the current running process

**What is the first user process that gets scheduled on the CPU?**  The `init` process. The `init` then forks a child `sh`, which runs the xv6 shell program. The `init` then waits on `sh`, and the `sh` process will at some timepoint be scheduled -- this is when you see that active xv6 shell prompting you for some input!

## Basic Round-Robin (RR) Scheduling

The following is the default xv6 `scheduler()` code. Guess **what scheduling algorithm it is using?** (You will need knowledge of when a user process will swtch back to the scheduler, from the section above).

```C
for(;;) {
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state != RUNNABLE)
      continue;

    // Switch to chosen process.  It is the process's job
    // to release ptable.lock and then reacquire it
    // before jumping back to us.
    c->proc = p;
    switchuvm(p);
    p->state = RUNNING;

    swtch(&(c->scheduler), p->context);

    switchkvm();

    // Process is done running for now.
    // It should have changed its p->state before coming back.
    c->proc = 0;
  }
}
```

Answer: it is a *Round-Robin* (RR) scheduler <u>over the ptable</u>. At a decision point, it always picks the next RUNNABLE process in the ptable order. When it reaches the end of the ptable, it wraps around.

If we have exactly three RUNNABLE process in the ptable, named A, B, and C, and they happen to appear in the ptable next to each other in that order, then a possible scheduling behavior looks like:

```text
 timer ticks  ...  16  17  18  19  20  21  22  23  ...
running proc  ...   C   A   B   C   A   B   C   A  ...
```

## P4 Scheduler Features & Tips

You need to change/add two features to the default primitive RR scheduler:

1. Allow different processes to have different **timeslice** values - their share of ticks in an RR scheduling cycle
2. Give **compensation** ticks to a process if it went to sleep previously

Our RR scheduler should operate over some <u>circular queue-like structure</u> (that you need to create), <u>NOT over the ptable</u>. When a new process gets created, it gets added to the *tail* of the queue. Our scheduler loops over that queue.

With the timeslice feature, different processes can have different timeslice values. Say we have exactly three RUNNABLE process in the ptable, named A, B, and C, created in that order. They have the following timeslice values:
- A: 4
- B: 1
- C: 3

Then, a possible scheduling behavior looks like:

```text
 timer ticks  ...  16  17  18  19  20  21  22  23  24  25  26  27  ...
running proc  ...   C   C   C   A   A   A   A   B   C   C   C   A  ...
```

We will talk about the compensation ticks feature next week. But please try to figure that out by yourself by reading the P4 spec.

Tips on P4 implementation: please check out the "Tips" section of P4 spec. Try to follow the order of the 5 bullet points!
