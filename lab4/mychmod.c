#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

void processMask(const char* filename, const char* mask_str);

void processAsBitMask(const char* filename, const char* mask_str);
void processAsLetterMask(const char* filename, const char* mask_str);

// void checkAndSetMode(const char* file);
void checkDigitMask(const char* number);
void checkLetterMask(const char* letters);

void onExitHandler(int exit_code, void* arg);

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Too few arguments\n");
        return 1;
    }
    
    const char* filename = argv[2]; 
    const char* mask = argv[1];

    on_exit(onExitHandler, argv[1]);
    processMask(filename, mask);

    return 0;
}

void processMask(const char* filename, const char* mask_str) {
    // стейты: пользователь (ugoa), операция (+-=), цифра (0...7), права (rwx)
    const char symbol = mask_str[0];
    
    if (isdigit(symbol) && symbol >= '0' && symbol <= '7') {
        processAsBitMask(filename, mask_str);
    }
    else {
        processAsLetterMask(filename, mask_str);
    }

}

void processAsBitMask(const char* filename, const char* mask_str) {
    checkDigitMask(mask_str);
    
    char* endptr;
    long mask = strtol(mask_str, &endptr, 8);
    if (*endptr != '\0') {
        exit(3);
    }

    if (chmod(filename, (mode_t)mask) == -1) {
        exit(2);
    }

}

void processAsLetterMask(const char* filename, const char* mask_str) {
    checkLetterMask(mask_str);
    
    mode_t mask = 0;
    for (size_t i = 0; i < strlen(mask_str); ++i) {
        switch (mask_str[i]) {
            case 'u':
                mask |= S_IRUSR | S_IWUSR | S_IXUSR;
                break;
            case 'w':
                mask |= S_IRGRP | S_IWGRP | S_IXGRP;
                break;
            case 'o':
                mask |= S_IROTH | S_IWOTH | S_IXOTH;
                break;
            case 'a':
                mask |= S_IRUSR | S_IWUSR | S_IXUSR;
                mask |= S_IRGRP | S_IWGRP | S_IXGRP;
                mask |= S_IROTH | S_IWOTH | S_IXOTH;
                break;
            default:
                break;
            }
    }
    
    if (chmod(filename, mask) == -1) {
        exit(2);
    }

}

void checkAndSetMode(const char* file) {
    mode_t mask = 0;
    mask |= S_IRUSR | S_IWUSR | S_IXUSR;
    mask |= S_IRGRP | S_IWGRP | S_IXGRP;
    mask |= S_IROTH | S_IWOTH | S_IXOTH;

    
    if (chmod(file, mask) == -1) {
        exit(2);
    }
}

void checkDigitMask(const char* number) {
    for (size_t i = 0; i < strlen(number); ++i) {
        int digit = number[i];
        if (!isdigit(digit) || digit < '0' || digit > '7') {
            exit(1);
        }
    }
}

void checkLetterMask(const char* letters) {
    for (size_t i = 0; i < strlen(letters); ++i) {
        if (!strchr("ugoarwx+-=", letters[i])) {
            exit(1);
        }
    }
}

void onExitHandler(int exit_code, void* arg) {
    switch (exit_code) {
        case 0:
            return;
        case 1:
            fprintf(stderr, "mychmod: invalid mode: '%s'\n", (char*)arg);
            fprintf(stderr, "Try 'chmod --help' for more information.\n");
            break;
        default:
            int error = errno;
            fprintf(stderr, "Error: %d (%s)\n", error, stderror(error));
            break;
    }
}
