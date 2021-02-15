#include <stdio.h>
#include <unistd.h>

int main(void) {
    if (fork() || fork())
        fork();
    printf("I'm a process\n");
    return 0;
}
