#include <stdio.h>
#include <string.h>


int main(void) {
    char str[100] = "  hello?  \t world! \n";

    printf("Tokens:\n");
    char *token = strtok(str, " \t\n");
    while (token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, " \t\n");
    }

    printf("What if we use the original `str` now?:\n");
    printf("%s\n", str);

    return 0;
}
