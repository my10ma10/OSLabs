#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#define ADD_MASK 0
#define SUBTRACT_MASK 1
#define ASSIGN_MASK 2

void processMask(const char* filename, const char* mask_str);

void processAsBitMask(const char* filename, const char* mask_str);
void processAsLetterMask(const char* filename, const char* mask_str);

void checkDigitMask(const char* number);
int checkLetterMaskAndDefineOperation(const char* letters);

void combine(mode_t* initMask, mode_t newMask, int operation);
void changeMod(const char* filename, mode_t initMask);

void getCurrentFilePermissions(const char* filename, struct stat* st);
int getOperation(const char letter);
void onExitHandler(int exit_code, void* arg);

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Too few arguments\n");
        return 1;
    }
    
    const char* mask = argv[1];
    on_exit(onExitHandler, argv[1]);

    for (int i = 2; i < argc; ++i) { 
        const char* filename = argv[2]; 
        processMask(filename, mask);
    }
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

    struct stat st;
    getCurrentFilePermissions(filename, &st);
    
    mode_t initMask = st.st_mode;
    
    char* endptr;
    long newMask = strtol(mask_str, &endptr, 8);
    if (*endptr != '\0') {
        exit(2);
    }

    combine(&initMask, newMask, -1);
    changeMod(filename, (mode_t)initMask);
}

void processAsLetterMask(const char* filename, const char* mask_str) {
    int operation = checkLetterMaskAndDefineOperation(mask_str);
    
    struct stat st;
    getCurrentFilePermissions(filename, &st);

    mode_t initMask = st.st_mode;
    mode_t newMask = 0;

    for (size_t i = 0; i < strlen(mask_str); ++i) {
        switch (mask_str[i]) {
        case 'u':
            newMask = S_IRUSR | S_IWUSR | S_IXUSR;
            break;
        case 'g':
            newMask |= S_IRGRP | S_IWGRP | S_IXGRP;
            break;
        case 'o':
            newMask |= S_IROTH | S_IWOTH | S_IXOTH;
            break;
        case 'a':
            newMask |= S_IRUSR | S_IWUSR | S_IXUSR;
            newMask |= S_IRGRP | S_IWGRP | S_IXGRP;
            newMask |= S_IROTH | S_IWOTH | S_IXOTH;
            break;
        default:
            break;
        }
    }

    combine(&initMask, newMask, operation);    
    changeMod(filename, initMask);
}

void checkDigitMask(const char* number) {
    for (size_t i = 0; i < strlen(number); ++i) {
        int digit = number[i];
        if (!isdigit(digit) || digit < '0' || digit > '7') {
            exit(1);
        }
    }
}

int checkLetterMaskAndDefineOperation(const char* letters) {
    int res = -1;
    for (size_t i = 0; i < strlen(letters); ++i) {
        if (!strchr("ugoarwx+-=", letters[i])) {
            exit(1);
        }
        if (strchr("+-=", letters[i])) {
            if (res != -1) exit(1);
            res = getOperation(letters[i]);
        }
    }
    return res;
}

void combine(mode_t* initMask, mode_t newMask, int operation) {
    if (operation == -1) {
        *initMask = newMask;
    }
    switch (operation)
    {
    case ADD_MASK:
        *initMask |= newMask;
        break;
    case SUBTRACT_MASK:
        *initMask &= ~newMask;
        break;
    case ASSIGN_MASK:
        *initMask = newMask;
        break;    
    default:
        break;
    }
}

void changeMod(const char *filename, mode_t initMask) {
    if (chmod(filename, initMask) == -1) {
        exit(2);
    }
}

void getCurrentFilePermissions(const char* filename, struct stat* st) {
    if (stat(filename, st) == -1) {
        perror("mychmod error");
        exit(2);
    }
}

int getOperation(const char letter) {
    int res = -1;
    switch (letter) {
        case '+':
            res = ADD_MASK;
            break;
        case '-':
            res = SUBTRACT_MASK;
            break;
        case '=':
            res = ASSIGN_MASK;
            break;                
        default:
            break;
    }
    return res;
}

void onExitHandler(int exit_code, void *arg)
{
    switch (exit_code) {
        case 0:
            return;
        case 1:
            fprintf(stderr, "mychmod: invalid mode: '%s'\n", (char*)arg);
            fprintf(stderr, "Try 'chmod --help' for more information.\n");
            break;
        default:
            int error = errno;
            fprintf(stderr, "Error: %d (%s)\n", error, strerror(error));
            break;
    }
}
