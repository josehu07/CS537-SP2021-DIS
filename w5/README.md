# CS537 SP2021 DIS-315 Week 4

Copyright 2021 Guanzhou Hu

Topics:

- Advanced features of P3 shell
    - Parsing arguments w/ `strtok()`
    - Redirection, manipulating file descriptors w/ `dup()` and `dup2()`
    - Aliasing
- Midterm 1 Q&A session

Links:

- Remember [cppreference.com](https://en.cppreference.com/w/c) for standard library references
- Remember the `man` command or [online man pages](https://man7.org/linux/man-pages/man3/exec.3.html) for Linux syscall/utilities references
- Midterm 1 practice exams: [practice exams](https://canvas.wisc.edu/courses/230447/pages/exams)

I recommend implementing features of your P3 shell in the order of the following sections. **Test one feature thoroughly before moving on to the next one**. You should have a simple fork-exec shell working at this time - if not, review `w4/` material.

## Parsing a String with `strtok()`

The `strtok()` function parses out *tokens* of a string by **modifying the string in place**, writing null bytes to replace any *delimiter* character:

```C
char str[100] = "  hello?  \t world! \n";

printf("Tokens:\n");
char *token = strtok(str, " \t\n");     // initial call: give a pointer to original string
while (token != NULL) {                 //               , and a string of delimiters
    printf("%s\n", token);
    token = strtok(NULL, " \t\n");      // subsequent calls: give NULL as the first arg
}                                       //                   , indicating "resume from last call"

printf("What if we use the original `str` now?:\n");
printf("%s\n", str);
```

One important thing to remember is that this function modifies the original string in place.
- C strings always end with a null byte `\0` (effectively a zero byte in memory)
- Seeing a `\0` marks the end of a string - bytes after that are not considered belonging to the string
- Hence, `strtok()` utilizes this fact to provide you an "illusion" of parsing out tokens

In the above example, the memory content of the `str` array along the execution looks like:

```C
 a b        c  d
 | |        |  |
 v v        v  v
"  hello?   \t world!  \n"      // originally
"  hello?\0 \t world!  \n"      // after the initial call above the while loop
"  hello?\0 \t world!\0\n"      // after the 0-th iteration of the while loop
                                // no more tokens afterwards, while loop ends here
```

**Question: Where does `token` point to after the initial call? Where does `token` point to after the 0-th iteration of the while loop?**  Choose your answer among `a`, `b`, `c`, or `d`.

**Question: What will the last `printf()` print out as it tries to print the original `str` as a string afterwards?**  Try it yourself with the `strtok-example` in this repo.

Since `strtok()` modifies the string in place, **remember to duplicate it beforehand and work on the duplicated copy**, in case you need the original string afterwards.

## Handling `stdout` Redirection with `dup()` & `dup2()`

After you implement argument parsing and can successfully `exec()` on the correct program name and the correct list of arguments, move on to handling stdout redirection.

- Stdout redirection requires you to detect a `>` char in the user line:
    - If `>` appears, it MUST happen exactly once and MUST have exactly one word after it
    - There might not be whitespaces around `>`; E.g., `/bin/ls -l>file.txt` is valid
    - Hence, detection and parsing of `>` should happen BEFORE the argument parsing loop of `strtok()`
- Once you know you are redirecting the child process's stdout to `file.txt`:
    - First, open the file with `open()` and make sure it opens - use `open()` instead of `fopen()` because we need a file descriptor here
    - BEFORE `fork()`ing the child, do `dup()` to save a copy of the original STDOUT
    - BEFORE `fork()`ing the child, do `dup2()` to replace STDOUT with the opened file
    - After waited for the child, you need to `dup2()` the old stdout back

A typical workflow would look like:

```C
int file_fd = open(...);
if (fild_fd == -1) {
    // always check for failure
    // ...
    return;
}

int old_stdout_fd = dup(STDOUT_FILENO);
if (old_stdout_fd == -1) {
    // always check for failures
    // ...
    return;
}

int dup2_ret = dup2(file_fd, STDOUT_FILENO);
if (dup2_ret == -1) {
    // always check for failures
    // ...
    return;
}

int pid = fork();
if (pid < 0) {
    // fork failed
    // ...

} else if (pid == 0) {
    // in child process
    // exec() ...
    _exit(1);

} else {
    // in parent process
    // waitpid() ...
    dup2(old_stdout_fd, STDOUT_FILENO);
    close(file_fd);
}
```

## Doing Aliasing

After you implement stdout redirection and can successfully direct output to the file, move on to supporting *aliasing*.

- The word `alias` is a *built-in* command - you shell program itself recognizes this special word
    - You are NOT calling a program like `/bin/alias`
    - Your P3 shell recognizes three special built-in words: `alias`, `unalias`, `exit`
    - Normal UNIX shells treat `cd` as built-in as well; We do not consider paths in P3 so you don't have to worry about that
- The detection of the built-in words should happen BEFORE stdout redirection
- Make some kind of data structure to hold the mapping information
    - A linked-list should be the most handy
    - Write helper functions that appends to, deletes from, or prints (part of) of the list; Always good practice to *modularize* your code

In summary, the workflow of your shell should look like:

```text
(1) Read in a user line
(2) Check the beginning word of the line (perhaps manually):
    - Is `exit`?: exit
    - Is `alias`?: check the remaining line, add an alias or print aliases as needed
    - Is `unalias`?: check the remianing line, do unaliasing
    - Otherwise?: move on
(3) Search for the char `>` in the line:
    - Appears?: check if appears exactly once and has exactly one word after it
        - If not: error and go back to (1)
        - If yes: save the filename, set a flag saying "will do redirection for this command",
                  strip off the string after `>`, then move on
    - No `>`?: flag not set, move on
(4) Feed the line to your `strtok()` argument parsing loop
(5) Do the `fork()`-`exec()` combo
    - If redirection flag set, handle it properly
    - Child does `exec()` on the argument list
    - Parent waits on child pid
(6) Go back to (1)...
```

## Midterm 1 Q&A Session

Ask me any question you have about the conceptual content of this course, past exams, etc.
