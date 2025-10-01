#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define GREEN_COLOR "\033[32m"
#define BLUE_COLOR "\033[34m"
#define LIGHT_BLUE_COLOR "\033[36m"
#define GREEN_BACK "\033[42m"
#define RESET_COLOR "\033[0m"

#define BOLD "\033[1m"

#define BUF_SIZE 4096

#define PERMS_SIZE 11

void ls(int* a_flag, int* l_flag, const char** path);
void handle_params(int argc, char **argv, int* a_flag, int* l_flag, const char** path);

void colorize_filename(struct stat st, const char* name);
void full_format(struct stat st, const char* filename);

void add_permissions(mode_t mode);
void add_nlinks(struct stat st);
void add_user(struct stat st);
void add_group(struct stat st);
void add_time(struct stat st);


int main(int argc, char** argv) {
    const char* path = ".";
    int a_flag = 0, l_flag = 0;

    if (argc > 1) {
        handle_params(argc, argv, &a_flag, &l_flag, &path);
    }
    ls(&a_flag, &l_flag, &path);
    return 0;
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

        if (*l_flag) {
            full_format(st, entry->d_name);
        }
        else {
            colorize_filename(st, entry->d_name);
        }
    }

    if (!*l_flag) {
        printf("\n");
    }
    closedir(dir);
}

void colorize_filename(struct stat st, const char* name) {
    if (S_ISREG(st.st_mode)) { // regular file
        printf(GREEN_COLOR BOLD "%s  " RESET_COLOR, name);
    }
    else if (S_ISLNK(st.st_mode)) { // link
        printf(LIGHT_BLUE_COLOR BOLD "%s  " RESET_COLOR, name);
    }
    else if (S_ISDIR(st.st_mode)) { // directory
        printf(BLUE_COLOR GREEN_BACK "%s" RESET_COLOR "  ", name);
    }
    else {
        printf("%s  ", name);
    }
}

void full_format(struct stat st, const char* filename) {
    add_permissions(st.st_mode);

    add_nlinks(st);

    add_user(st);
    
    add_group(st);
    
    printf("%ld\t", st.st_size); // size of file

    add_time(st);

    
    colorize_filename(st, filename);
    printf("\n");
}

void add_time(struct stat st) {
    char* time_str = ctime(&st.st_ctime);

    char formatted_time[12];
    strncpy(formatted_time, time_str + 4, 12);
    printf("%s  ", formatted_time);
}

void add_user(struct stat st) {
    struct passwd* pwd_info = getpwuid(st.st_uid);
    if (pwd_info == NULL) {
        perror("Error pwd_info");
        exit(1);
    }
    printf("%s ", pwd_info->pw_name);
}

void add_permissions(mode_t mode) {
    char perms[PERMS_SIZE];

    if (S_ISREG(mode)) { // regular file
        perms[0] = '-';
    }
    else if (S_ISLNK(mode)) { // link
        perms[0] = 'l';
    }
    else if (S_ISDIR(mode)) { // directory
        perms[0] = 'd';
    }
    else if (S_ISCHR(mode)) { // char device
        perms[0] = 'c';
    }
    else if (S_ISBLK(mode)) { // block device
        perms[0] = 'b';
    }
    else {
        perms[0] = '?'; // other
    }

    // find permissions
    int target_perm = S_IRUSR;
    for (int i = 0; i < 9; target_perm >>= 1, ++i) {
        perms[i+1] = (mode & target_perm) ? "rwx"[i%3] : '-';
    } 
    perms[10] = '\0';

    // add permissions to out
    printf("%s ", perms);
}

void add_nlinks(struct stat st) {
    printf("%lu ", (unsigned long int)st.st_nlink);
}

void add_group(struct stat st) {
    struct group* grp_info = getgrgid(st.st_gid);

    if (grp_info == NULL) {
        perror("Error grp_info");
        exit(1);
    }
    printf("%s ", grp_info->gr_name);
}