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

void checkDigitMask(const char* number);
void checkLetterMask(const char* letters);

mode_t setWhoMask(char whoSymb);
mode_t setPermsMask(char permsSymb);

mode_t applyPerms(mode_t whoMask, mode_t permsMask);

void combine(mode_t* initMask, mode_t newMask, int operation);
void changeMod(const char* filename, mode_t initMask);

void getCurrentFilePermissions(const char* filename, struct stat* st);
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
    checkLetterMask(mask_str);
    
    struct stat st;
    getCurrentFilePermissions(filename, &st);

    int operation;

    mode_t initMask = st.st_mode;
    mode_t whoMask = 0;
    mode_t permsMask = 0;
    mode_t newMask = 0;

    for (size_t i = 0; i < strlen(mask_str); ++i) {
        if (strchr("ugoa", mask_str[i])) {
            whoMask |= setWhoMask(mask_str[i]);
        }

        if (strchr("+-=", mask_str[i])) {
            operation = mask_str[i];
        }

        if (strchr("rwx", mask_str[i])) {
            permsMask |= setPermsMask(mask_str[i]);
        }
    }

    newMask |= applyPerms(whoMask, permsMask);

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

void checkLetterMask(const char* letters) {
    for (size_t i = 0; i < strlen(letters); ++i) {
        if (!strchr("ugoarwx+-=", letters[i])) {
            exit(1);
        }
    }
}

mode_t setWhoMask(char who) {
    mode_t whoMask = 0;
    switch (who) {
        case 'u':
            whoMask |= S_IRWXU;
            break;
        case 'g':
            whoMask |= S_IRWXG;
            break;
        case 'o':
            whoMask |= S_IRWXO;
            break;
        case 'a':
            whoMask |= S_IRWXU | S_IRWXG | S_IRWXO;
            break;
        default:
            break;
    }
    return whoMask;
}

mode_t setPermsMask(char permsSymb) {
    mode_t permsMask = 0;
    switch (permsSymb) {
        case 'r':
            permsMask |= S_IRUSR | S_IRGRP | S_IROTH;
            break;
        case 'w':
            permsMask |= S_IWUSR | S_IWGRP | S_IWOTH;
            break;
        case 'x':
            permsMask |= S_IXUSR | S_IXGRP | S_IXOTH;
            break;            
        default:
            break;
    }
    return permsMask;
}

mode_t applyPerms(mode_t whoMask, mode_t permsMask) {
    mode_t newMask = 0;
    if (whoMask & S_IRWXU) newMask |= permsMask & S_IRWXU;
    if (whoMask & S_IRWXG) newMask |= permsMask & S_IRWXG;
    if (whoMask & S_IRWXO) newMask |= permsMask & S_IRWXO;
    return newMask;
}

void combine(mode_t* initMask, mode_t newMask, int operation) {
    switch (operation)
    {
    case '+':
        *initMask |= newMask;
        break;
    case '-':
        *initMask &= ~newMask;
        break;
    case '=':
        *initMask = newMask;
        break;    
    default:
        break;
    }
    if (operation == -1) {
        *initMask = newMask;
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
            fprintf(stderr, "Error: %d (%s)\n", error, strerror(error));
            break;
    }
}
