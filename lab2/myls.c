#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define RED_COLOR "\033[31m"
#define GREEN_COLOR "\033[32m"
#define YELLOW_COLOR "\033[33m"
#define BLUE_COLOR "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define LIGHT_BLUE_COLOR "\033[36m"
#define GREEN_BACK "\033[42m"

#define RESET_COLOR "\033[0m"

#define BOLD "\033[1m"

#define BUF_SIZE 4096

void ls(int* a_flag, int* l_flag, const char** path);
int endswith(const char* str, const char* suffix);
void handle_params(int argc, char **argv, int* a_flag, int* l_flag, const char** path);

void simple_format(struct stat st, const char* name, char* buf, size_t size);
void full_format(struct stat st, const char* name, char* buf, size_t size);


int main(int argc, char** argv) {
    const char* path = ".";
    int a_flag = 0, l_flag = 0;

    if (argc > 1) {
        handle_params(argc, argv, &a_flag, &l_flag, &path);
    }
    ls(&a_flag, &l_flag, &path);
    return 0;
}

void ls(int* a_flag, int* l_flag, const char** path) {
    char fullpath[BUF_SIZE];
    struct dirent* entry;
    struct stat st;

    DIR* dir = opendir(*path);
    if (!dir) {
        perror("Cannot open dir");
        printf("path: %s", *path);
        exit(1);
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", *path, entry->d_name);
        
        if (!*a_flag && entry->d_name[0] == '.') {
            continue;
        }
        
        // init struct stat
        if (lstat(fullpath, &st) == -1) {
            perror("Cannot init stat struct");
            exit(1);
        }

        char buf[BUF_SIZE];
        if (*l_flag) {
            full_format(st, entry->d_name, buf, sizeof(buf));
        }
        else {
            simple_format(st, entry->d_name, buf, sizeof(buf));   
        }
        printf("%s", buf);
    }

    printf("\n");

    closedir(dir);
}

void handle_params(int argc, char** argv, int* a_flag, int* l_flag, const char** path) {
    int res = 0;
    while ((res = getopt(argc, argv, "al")) != -1) {
        switch (res) {
            case 'a':
                *a_flag = 1;
                break;
            case 'l': 
                *l_flag = 1;
                break;
            default:
                break;
        }
    }
    
    if (argc > optind) {
        *path = argv[optind];
    }
}

void simple_format(struct stat st, const char* name, char* buf, size_t size) {
    if (S_ISREG(st.st_mode)) // regular file
        snprintf(buf, size, GREEN_COLOR BOLD "%s  " RESET_COLOR, name);
    else if (S_ISLNK(st.st_mode)) // link
        snprintf(buf, size, LIGHT_BLUE_COLOR BOLD "%s  " RESET_COLOR, name);
    else if (S_ISDIR(st.st_mode)) // directory
        snprintf(buf, size, BLUE_COLOR GREEN_BACK "%s" RESET_COLOR "  ", name);
    else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode)) // device
        snprintf(buf, size, YELLOW_COLOR BOLD "%s" RESET_COLOR, name);
}

void full_format(struct stat st, const char* name, char* buf, size_t size) {
    
}


int endswith(const char* str, const char* suffix) {
    if (!str || !suffix) 
        return 0;
    
    if (strlen(suffix) > strlen(str)) 
        return 0;
    
    return strncmp(str + strlen(str) - strlen(suffix), suffix, strlen(suffix)) == 0;
}