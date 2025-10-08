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

int find_target(const char* line, const char* substring) {
   char* found_pos;

    if ((found_pos = strstr(line, substring)) != NULL) {
        // before target
        printf("%.*s", (int)(found_pos - line), line);

        // target pattern
        printf(RED BOLD "%.*s" RESET_COLOR, (int)strlen(substring), substring);

        // after target
        if (find_target(found_pos + strlen(substring), substring) == 0) {
            printf("%s\n", found_pos + strlen(substring)); 
        }
        return 1;
    }
    return 0;
}

void grep(int argc, char** argv) {
    char line[BUF_SIZE];
    if (argc == 2) {
        while (fgets(line, sizeof(line), stdin) != NULL) {
            format(line);
            find_target(line, argv[1]);

            fflush(stdout);
        }
    }
    else if (argc == 3) {
        FILE* file = fopen(argv[2], "r");
        while (fgets(line, sizeof(line), file)) {
            format(line);
            find_target(line, argv[1]);
            
        }

        fclose(file);
    }
}

int main(int argc, char** argv) {
    grep(argc, argv);
    return 0;
}