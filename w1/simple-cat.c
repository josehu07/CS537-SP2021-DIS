/**
 * CS537 SP2021 - DIS W1
 * Simple C code example. Repeats the first line of input
 *     $ ./simple-cat [-f filename]
 *
 * Copyright 2021 Guanzhou Hu
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>


#define BUF_SIZE 128


/**
 * Repeat the first line of the file.
 * If the first line does not end with a newline symbol `\n`, add one.
 * Special case: if the file is empty, does nothing. Does not add a
 * newline.
 */
static void
cat_first_line(FILE *fp) {
    char buf[BUF_SIZE];

    /**
     * Try to read a line and repeat it.
     * For C standard lib functions, check out the cppreference doc:
     * e.g., https://en.cppreference.com/w/c/io/fgets
     */
    if (fgets(buf, BUF_SIZE, fp) != NULL) {
        size_t len = strlen(buf);
        assert(len > 0);

        if (buf[len - 1] == '\n')   /** Only works on UNIX-like. */
            printf("%s", buf);
        else
            printf("%s\n", buf);
    }
}


static bool use_stdin = true;
static char *fname = NULL;

static void
parse_args(int argc, char *argv[]) {
    /** Possible option: -f with a filename. */
    int opt;
    while ((opt = getopt(argc, argv, "f:")) != -1) {
        switch (opt) {
            case 'f':
                use_stdin = false;
                fname = optarg;
                break;
            default:
                printf("example: invalid command line\n");
                exit(1);
        }
    }

    /** Cannot have any other arguments. */
    if (optind != argc) {
        printf("example: invalid command line\n");
        exit(1);
    }
}


int
main(int argc, char *argv[]) {
    /** Parse cmd-line args. */
    parse_args(argc, argv);

    /** Open the file. */
    FILE *fp;
    if (!use_stdin) {
        fp = fopen(fname, "r");
        if (fp == NULL) {
            fprintf(stderr, "example: cannot open file\n");
            exit(1);
        }
    } else
        fp = stdin;
        /** The above else branch should trigger a linter warning. */

    /** Reads the first line of the file and print it out. */
    cat_first_line(fp);

    /** Close the file. */
    if (!use_stdin)
        fclose(fp);

    return 0;
}
