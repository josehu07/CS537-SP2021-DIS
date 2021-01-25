# CS537 SP2021 DIS-315 Week 1

Copyright 2021 Guanzhou Hu

Topics:

- C file I/O review
- Using gdb, linter, & valgrind
- P1 overview; How to compile, test, debug, & submit

Links:

- C standard lib reference: [https://en.cppreference.com/w/c](https://en.cppreference.com/w/c)
- My GDB commands reference: [https://www.josehu.com/assets/file/gdb-usage.pdf](https://www.josehu.com/assets/file/gdb-usage.pdf)
- 

## Example C Code

Please see `simple-cat.c`. To compile, do:

```bash
$ make
```

Try it out:

```bash
# Repeats the first line of the input file. Unless the file is
# completely empty, adds a newline at the end.
$ ./simple-cat -f testfile.txt

# If -f not given, uses stdin as the input. Example of piping
# output of `echo` into `simple-cat`:
$ echo -e "123\n456" | ./simple-cat
```

Please the comments in the code file for details.

Clean it up with:

```bash
$ make clean
```

Makefiles will be covered in future discussion sections.

## Debugging w/ GDB

Once compiled with the `-g` flag, the object file will contain full debugging symbol information.

Example of debugging through this code:

```bash
$ gdb ./testfile
...
(gdb) b cat_first_line      # break at any entry of `cat_first_line()`
(gdb) r -f testfile.txt     # run with the arguments
...
(gdb) l     # list code lines around breakpoint
(gdb) n     # step next (finishes func calls in the line)
(gdb) p something   # print something interesting
(gdb) c     # continue
...
(gdb) q     # quit GDB
```

## Code Style Linting

We use the Google style linter (with slightly looser constraints).

On a CSL lab machine, lint the code with:

```bash
$ ~cs537-1/projects/lint/cpplint.py --root=~cs537-1/handin --extensions=c,h simple-cat.c
```

Output should be:

```text
simple-cat.c:88:  If an else has a brace on one side, it should have it on both  [readability/braces] [5]
Done processing simple-cat.c
Total errors found: 1
```

## Detecting Memory Leak with Valgrind

Valgrind is a tool that runs your code in a virtually wrapped environment and detects any memory issues.

You can turn tracing options on/off. We are interested in memory leak issues:

```bash
$ valgrind --show-reachable=yes ./simple-cat -f testfile.txt
```

Output should be:

```text
==50432== Memcheck, a memory error detector
==50432== Copyright (C) 2002-2017, and GNU GPL'd, by Julian Seward et al.
==50432== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==50432== Command: ./simple-cat -f testfile.txt
==50432==
hello
==50432==
==50432== HEAP SUMMARY:
==50432==     in use at exit: 0 bytes in 0 blocks
==50432==   total heap usage: 3 allocs, 3 frees, 5,592 bytes allocated
==50432==
==50432== All heap blocks were freed -- no leaks are possible
==50432==
==50432== For lists of detected and suppressed errors, rerun with: -s
==50432== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

Note that linting & Valgrind checking can also be automated into the building process using make. We will cover that later.

## P1 Testing & Submission

We provide some public tests for p1. Please see `~cs537-1/tests/p1/README.md` on how to use the testing framework.

We will hide ~20% of the tests as private. These hidden tests will not trick you in corner cases. But still, passing all the public tests does not necessarily guarantee full points on the testcases.

Please read section 4 of the P1 spec carefully on how to submit and our slip-day policy.
