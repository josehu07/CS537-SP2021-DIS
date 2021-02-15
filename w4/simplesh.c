#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


static void handle_line(const char *userline) {
    if (strlen(userline) <= 0)
        return;

    // Make a copy in case we need to modify in place.
    char *line = strdup(userline);

    // Remove trailing newline.
    for (int i = strlen(line) - 1; i >= 0; --i) {
        if (line[i] == '\n')
            line[i] = '\0';
    }

    if (strlen(line) > 0) {
        // Fork a child process to exec the binary.
        pid_t pid = fork();

        if (pid != -1) {
            if (pid == 0) { // child
                char *args[2];
                args[0] = line;
                args[1] = NULL;

                execv(args[0], args);

                // if child reached here, exec failed
                printf("simplesh: exec failed\n");
                _exit(1);
            
            } else { // parent
                int status;
                waitpid(pid, &status, 0);
            }

        } else { // fork failed
            printf("simplesh: fork failed\n");
        }
    }
    
    // Remember to free the strdup'ed string.
    free(line);
}


int main(void) {
    char userline[512];

    while (1) {
        // Use write() to avoid output buffering.
        write(STDOUT_FILENO, "simplesh> ", 10);

        // Wait for user input line.
        char *ret = fgets(userline, 512, stdin);
        if (ret == NULL)    // EOF
            break;

        handle_line(userline);
    }

    return 0;
}
