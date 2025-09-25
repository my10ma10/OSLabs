#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define BUF_SIZE 1024

void command_n(char* buffer, int* strCnt) {
    char tmp[BUF_SIZE];

    ++(*strCnt);
    snprintf(tmp, sizeof(tmp), "%6d    %s", *strCnt, buffer);

    strcpy(buffer, tmp);
}

void command_b(char* buffer, int* strCnt) {
    char tmp[BUF_SIZE];

    if((buffer[0] == '\0') || (buffer[0] == '\n')) {
        snprintf(tmp, sizeof(tmp), "\n");
    }
    else {
        command_n(buffer, strCnt);
    }
    
}

void command_E(char* buffer) {
    char tmp[BUF_SIZE];
    if (strstr(buffer, "\n") != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        snprintf(tmp, sizeof(tmp), "%s$\n", buffer);
    }
    else {
        snprintf(tmp, sizeof(tmp), "%s\n", buffer);
    }
    strcpy(buffer, tmp);
}

int checkFlag(char* flags, char fl) {
    return strchr(flags, fl) != NULL;
}

void cat(int argc, char** argv) {
    char buffer[BUF_SIZE];
    char flags[100];
    char line_flags[BUF_SIZE];
    int line_flagsCount = 0;
    int flagsCount = 0;
    char * filename;

    if (argc == 1) {
        while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            fprintf(stdout, "%s", buffer);
        }
    }

    for (int i = 1; i < argc; ++i) {
        if (strstr(argv[i], "--") != NULL) {
            line_flags[line_flagsCount++] = argv[i];
        }
        else if (argv[i][0] == '-') {
            for (size_t j = 1; j < strlen(argv[i]); ++j) {
                flags[flagsCount++] = argv[i][j];
            }
        }
        else {
            filename = argv[i];
        }
    }

    FILE * file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stdout, "cat: %s", filename);
        exit(1);
    }
    
    /// lines parsing
    int strCnt = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        if (flagsCount != 0) {
            if (checkFlag(flags, 'b')) {
                command_b(buffer, &strCnt);
            }
            else if (checkFlag(flags, 'n') || checkFlag(flags, "number")) {
                command_n(buffer, &strCnt);
            }

            if (checkFlag(flags, 'E')) {
                command_E(buffer);
            }
        }
        fprintf(stdout, "%s", buffer);
    }
    fclose(file);
    
}
