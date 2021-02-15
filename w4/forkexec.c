#include <stdio.h>
#include <unistd.h>

int main(void) {
    pid_t pid = fork();

    if (pid != -1) {
        if (pid == 0) {
            // child
            printf("child: execing /bin/ls -l\n");

            char args[3] = {"/bin/ls", "-l", NULL};
            execv(args[0], args);

            // if child reached here, exec failed
            printf("child: exec failed\n");
            _exit(1);
        
        } else {
            // parent
            waitpid(pid);
            printf("parent: child process exits\n");
        }

    } else {
        // fork failed
        printf("parent: fork failed")
    }

    return 0;
}
