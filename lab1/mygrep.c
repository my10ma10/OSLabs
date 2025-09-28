#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 4096

#define RESET_COLOR "\033[0m"
#define RED "\033[31m"
#define BOLD "\033[1m"


void format(char* line) {
    size_t len = strlen(line);
    if (len > 0 && line[len - 1] == '\n') {
        line[len - 1] = '\0';
    }
}

void find_target(const char* line, const char* substring) {
   char* found_pos;

    if ((found_pos = strstr(line, substring)) != NULL) {
        // before target
        printf("%.*s", (int)(found_pos - line), line);

        // target pattern
        printf(RED BOLD "%.*s" RESET_COLOR, (int)strlen(substring), substring);

        // after target
        printf("%s\n", found_pos + strlen(substring)); 
    }
}

void grep(int argc, char** argv) {
    char line[BUF_SIZE];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        format(line);
        find_target(line, argv[1]);

        fflush(stdout);
    }
}

int main(int argc, char** argv) {
    grep(argc, argv);
    return 0;
}