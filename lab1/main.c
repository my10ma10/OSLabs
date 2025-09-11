#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "cat.c"

bool checkFlag(char* flags, char fl);

int main(int argc, char** argv) {
    char buffer[1024];
    char flags[100];
    int flagsCount = 0;
    char * filename;

    if (argc == 1) {
        while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            fprintf(stdout, "%s", buffer);
        }
    }

    for (int i = 1; i < argc; ++i) {
        if (strstr(argv[i], "--") != NULL) {
            /* code */
        }
        else if (argv[i][0] == '-') {
            for (size_t j = 1; j < strlen(argv[i]); ++j) {
                flags[flagsCount++] = argv[i][j];
                // fprintf(stdout, "arg = %c, flagsCount = %d\n", argv[i][j], flagsCount);
            }
        }
        else {
            filename = argv[i];
            // fprintf(stdout, "filename = %s\n", argv[i]);
        }
    }




    FILE * file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stdout, "cat: %s", filename);
        return 1;
    }
    
    // check flags

    // File line number
    int strCnt = 0;
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        if (flagsCount != 0) {
            if (checkFlag(flags, 'b')) {
                command_b(buffer, &strCnt);
            }
            else if (checkFlag(flags, 'n')) {
                command_n(buffer, &strCnt);
            }

            if (checkFlag(flags, 'E')) {
                command_E(buffer);
            }

            if (checkFlag(flags, 'v')) {
                command_v(buffer);
            }

            if (checkFlag(flags, 'T')) {
                command_T(buffer);
            }
        }
        else {
            fprintf(stdout, "%s", buffer);
        }
    }
    fclose(file);

    return 0;
}




bool checkFlag(char* flags, char fl) {
    return strchr(flags, fl) != NULL;
}