# CS537 SP2021 DIS-315 Week 3

Copyright 2021 Guanzhou Hu

Topics:

- More on xv6
    - Makefile tweaks
    - Debugging the kernel
    - Debugging a user process
- P2 Q&A

Links:

- None

## Makefile Tweaks

To ease debugging, make the following Makefile changes and recompile xv6:

```bash
CPUS=1              # make it single core by default
CFLAGS=... -Og ...  # instead of -O2 optimization
```

Remember to clean up the build to ensure the changes take effect:

```bash
$ make clean
```

To include a new user program, locate:

```bash
UPROGS=...\
       ...\
       _newuprog\   # append to the list here
```

## Debugging xv6

Please see last week's discussion section material under `w2/`.

It is possible to debug a user application, however there are some things to note.

First, we need to explicitly load the symbol file when entering GDB:

```bash
$ gdb
...
(gdb) symbol-file _newuprog
(gdb) b yourfunc
```

Second, GDB breaks on virtual addresses. Multiple user processes may happen to have some functions at the same beginning address. Whenever GDB breaks, we need to check that we are indeed at the target user process. One convenient (though not safe) way to verify this would be to dump the assembly of the application and see if the function addresses match:

```bash
objdump -d _newuprog
```

**Why it is safe to debug kernel names without having this issue?** Recall the virtual address space layout and how the kernel is mapped into every user process's address space.

## P2 Q&A

Please feel free to ask anything about P2: spec questions, underlying mechanisms, ...
