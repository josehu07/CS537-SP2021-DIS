# CS537 SP2021 DIS-315 Week 4

Copyright 2021 Guanzhou Hu

Topics:

- Understanding `fork()` and `exec()` variants
- Simple shell skeleton code
- Advanced features of a shell
    - Getting arguments w/ `strtok()`
    - Redirection, manipulating file descriptors w/ `dup()` and `dup2()`
    - Aliasing
- P3 overview

Links:

- Remember [cppreference.com](https://en.cppreference.com/w/c) for standard library references
- Remember the `man` command or [online man pages](https://man7.org/linux/man-pages/man3/exec.3.html) for Linux syscall/utilities references

There are several example programs in this repo. Do:

```bash
make
```

to build them and try them by yourself.

## Understanding `fork()`

The [`fork()` syscall](https://man7.org/linux/man-pages/man2/fork.2.html) creates a new child process by duplicating the calling process. Both parent and chlid process, after forking, continues execution right after the line that called `fork()`.

- The child process is an **exact duplicate** of the parent process:
    - **What are duplicated?** The whole address space, open files, etc.
    - What are not duplicated? PID, PPID (parent pid), and a few other things.
- Returns `0` on the child process; Returns the child's PID on the parent process.

Exampel usage:

```C
pid_t retval = fork();

if (pid > 0) {    // -1 means fork failed
    if (retval == 0) {
        // child
        printf("parent PID: %d, child PID: %d\n", getppid(), getpid());
    } else {
        // parent
        printf("parent PID: %d, child PID: %d\n", getpid(), retval);
    }
}
```

**Classic but interesting puzzle** about fork: how many processes will be created (including the initial parent) by the following program?

```C
int main(void) {
    if (fork() || fork())
        fork();
    printf("I'm a process\n");
    return 0;
}
```

The answer could be found [here](https://www.geeksforgeeks.org/fork-practice-questions/). You could also try the `forkquiz` exmaple found in this repo.

**Can forked processes communicate over shared memory, e.g., a global static variable?** Processes vs. threads, processes have isolated virtual address space while threads share the same one.

## Understanding `exec()`

The [`exec()` syscall](https://man7.org/linux/man-pages/man3/exec.3.html), or to be more precise, the `execve()` syscall, replaces the current process image with a new one. It essentially executes the target binary executable by replacing the current process's code segment with it.

- On successful execution, `exec()` never returns; If it does return, it indicates a failure.
- There are quite a bunch of wrapper variants of execve, as listed in the man page:
    - `l` indicates it takes in arguments like `printf()`
    - `v` indicates it takes in arguments as an array of strings, with **the last argument must being a NULL pointer**
    - `p` indicates it will search for the target name in all PATH locations; otherwise, must be an absolute path or relative to cwd
    - `e` indicates it takes in an extra list of environment variables that will be fed to the target executable

`exec()` is often combined with `fork()` in the following way when the parent process wants to spawn a child process and let it execute a certain executable:

```C
int main(void) {
    pid_t pid = fork();

    if (pid != -1) {
        if (pid == 0) { // child
            printf("child: execing /bin/ls -l\n");
            
            char args[3] = {"/bin/ls", "-l", NULL};
            execv(args[0], args);

            // if child reached here, exec failed
            printf("child: exec failed\n");
            _exit(1);
        
        } else { // parent
            waitpid(pid);
            printf("parent: child process exits\n");
        }

    } else { // fork failed
        printf("parent: fork failed")
    }

    return 0;
}
```

Try the `forkexec` example in this repo.

## Simple Shell Skeleton Code

A "shell" is just a user application - it has nothing special. When a UNIX-like system boots up, it initializes a first user process called `init` (in this sense, `init` is indeed a special process), and `init` in turn forks a child process to exec a shell program `sh` (`sh` has nothing special). That `sh` program is what you are writing for P3.

Please see `simplesh.c` in this repo - a piece of skeleton code for a very simple shell that loops and does the following things every iteration:

1. Prints a prompt, then waits for a user input line
2. Loops on input lines from stdin, assumes each line is just a single word (a binary name)
3. For every line, tries to fork a child process and let it exec the binary
4. Parent process waits for the child process to exit

Try this simple shell by (assume you `make`d):

```bash
./simplesh  # here you are in the host shell
simplesh> /bin/ls   # here `simplesh` is executing - type something to feed to it
...
# Use ctrl-D to send an EOF to end simplesh.
```

## Advanced Features of a Shell

As you can see, the `simplesh` above is quite primitive. There are some (actually quite a lot of) advanced features not included yet. What you will need to add for P3, based on this skeleton code, include:

- **Batch mode** (or *script* mode): Read input lines from a file instead of from stdin
    - This one should be trivial
- **Parsing string segments** in the line, feed them as arguments to exec
    - You may want to use the [`strtok()`](https://en.cppreference.com/w/c/string/byte/strtok) library function - it is for parsing out "tokens" of a string
    - `strtok()` modifies the string in place, so you might want to do `strdup()` once before to save a copy of the original line, in case you need that later
- **Stdout redirection**: when `>` appears in the line followed by a filename, the child process's stdout should be replaced by the file
    - You open the file in the child process, before execing the command
    - On successful file open, you then need to replace the process's stdout fd by the fd of the file; Check out [`dup()` and `dup2()`](https://man7.org/linux/man-pages/man2/dup2.2.html)
- **Aliasing**: allow alias names of commands, e.g., `alias ll ls -l` allows you to type `ll` for `ls -l` in the future
    - You will need some kind of data structure to store the current aliasing mappings; Could be just fixed arrays, but linked-lists are better for adding/removing elements
    - If user types `alias` alone, should print a thorough list of all the current aliasing mappings
    - Aliases can be removed by doing `unalias aliasname`

We will talk about these in more depth next week.

## P3 Overview & Tips

For P3, the spec is again quite long and informative. Try to read through it and understand it.

**Tip**: in general, start your implementation from a simple, primitive shell. Once that works, **add in features one by one, thoroughly testing each before moving on** to the next feature.
