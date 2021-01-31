# CS537 SP2021 DIS-315 Week 2

Copyright 2021 Guanzhou Hu

Topics:

- Makefile
- Introduction to xv6
    - In kernel: where to find system calls implementation, PCB
    - User-level: syscall wrappers, user programs
- Debugging xv6 w/ GDB
- P2 overview; how to submit

Links:

- GNU Make official manual: [https://www.gnu.org/software/make/manual/](https://www.gnu.org/software/make/manual/)
- xv6 MIT public release: [https://github.com/mit-pdos/xv6-public](https://github.com/mit-pdos/xv6-public)
    - Please use our mirrored archive at `~cs537-1/projects/xv6.tar.gz` as your project codebase
- QEMU official website: [https://www.qemu.org/](https://www.qemu.org/)

## Makefile

GNU Make is a tool that automates the building process of your project.

It takes a `Makefile` of specific syntax to figure out how you wanna build your code. In the Makefile, you specify *targets*, the dependencies of each target, and the commands to run to build each target.

```bash
target: dependencies
    commands_to_run
```

To see the basic syntax, check out `Makefile.simple` and `Makefile.advanced`. To try it out, rename one of them to `Makefile`:

```bash
$ cp Makefile.advanced Makefile
$ make [target_name]    # If no target given, defaults to the first target met
                        # So, we typically use an `all` target at the top
```

> Note: Makefile is sensitive to tabs vs. spaces. It requires commands to be indented with ONE TAB.

> You may also be interested in `CMake`, a higher-level building tool that "generates" Makefiles for you. You may see use cases in larger, more complex projects.

## Introduction to xv6

xv6 is a simple operating system for teaching purposes, initially released by the MIT PDOS group. The OSTEP book uses xv6 for its projects.

To run it we need an emulator to emulate a hardware platform for us (a "fake" machine) and then boot xv6 on that emulator. System developers typically use QEMU. QEMU is already installed on CSL machines.

The Makefile has taken care of everything we need to boot it up on QEMU:

```bash
$ make qemu-nox

Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ ls    # Now this is the shell of xv6.
.              1 1 512
..             1 1 512
README         2 2 2286
cat            2 3 16672
echo           2 4 15564
forktest       2 5 9264
grep           2 6 19836
init           2 7 16096
kill           2 8 15620
ln             2 9 15492
ls             2 10 18076
mkdir          2 11 15652
rm             2 12 15628
sh             2 13 31448
stressfs       2 14 16504
usertests      2 15 67400
wc             2 16 17240
zombie         2 17 15196
console        3 18 0
$ 
```

Use `ctrl-A`, release, then `X` to abort out of QEMU.

### Adding a New System Call

**Question**: what do you need to have to implement a system call for a user program to use?

Based on the answer to this question, these are the potential places you need to look at to add in a new system call:

- In kernel:
    - `syscall.c`: code of the syscall handler
    - `sysproc.c`: implementation of some individual syscall kernel logic
    - `syscall.h`: header defining syscall numbers
- At user-level:
    - `user.h`: header of user-level wrappers for user prgrams to include
    - `usys.S`: implementation of user-level wrappers

### Process Control Blocks

**Question**: if you want to remember information about a process, where should you store that information?

Process related stuff resides in these places in the kernel:

- `proc.h`: you will find the PCB structure definition here
- `proc.c`: routines related to allocating, modifying, and killing processes

### Adding a User Program

You will need to write a new user program which invokes the new system calls you added. To add a new user program, you could add a new file named `<user-prog-name>.c`.

Notes:

- Check out an existing user program, e.g., `echo.c`, to see how to code one.
- You cannot include C standard libraries - you can only include things that xv6 provides for its user programs. `user.h` contains a list of some of them, where the implementations can be found in `ulib.c`.
- You will need to modify the `Makefile` to include your new user program into the building process.

## Debugging xv6 w/ GDB

First, include xv6's GDB init script into your auto loading path by:

```bash
$ vim ~/.gdbinit
Add the line:
add-auto-load-safe-path /path/to/your/xv6/.gdbinit
```

The Makefile, again, has taken care of hooking up GDB with xv6 for you:

```bash
# In one shell, under xv6 folder:
$ make qemu-nox-gdb

Booting from Hard Disk..xv6...
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inode8
init: starting sh
$ ls    # Do `ls` after setting up the breakpoint on `exec`
```

```bash
# In another shell, also in xv6 folder:
$ gdb

(gdb) c     # This will continue the xv6 booting
Continuing.

# Do `Ctrl-C` in GDB to manually break the xv6.
# At this time, you will notice that the xv6 shell is not responding
# to your inputs, because it has been paused by GDB.

(gdb) b exec    # Set breakpoint on `exec`
(gdb) c     # Continue xv6; it is now responding again
Continuing.

# Doing `ls` in xv6 shell invokes `exec` function, so you will see
# GDB breaking at the breakpoint you set:
=> 0x80100bcc <exec+9>: call   0x801042a9 <myproc>

Breakpoint 2, exec (path=0x19c0 "\033[A\033[Bls",
    argv=0x8df23ec8) at exec.c:20
20        struct proc *curproc = myproc();
```

## P2 Testing & Submission

We provide some public tests for p2. Please see `~cs537-1/tests/p2/README.md` on how to use the testing framework. Again, there will be a small portion of hidden tests.

Please read the "Handing It In" section of the P2 spec carefully on how to submit.
