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

**When will a user process swtch back to the scheduler?**  There are three cases:
- The process exits
- The process goes to block voluntarily, examples:
  - It calls the sleep syscall
  - It calls the wait syscall
  - It tries to read on a pipe
- The process "yield"s - typically at a timer interrupt
  - At every ~10ms, the timer issues a hardware interrupt
  - This forces the CPU to trap into the kernel, see `trap()` in `trap.c`
  - xv6 then increments a global counter named `ticks`, then `yield()` the current running process

**What is the first user process that gets scheduled on the CPU?**  The `init` process. The `init` then forks a child `sh`, which runs the xv6 shell program. The `init` then waits on `sh`, and the `sh` process will at some timepoint be scheduled -- this is when you see that active xv6 shell prompting you for some input!

## Basic Round-Robin (RR) Scheduling


