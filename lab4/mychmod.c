#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

struct Mask {
    char c;
};

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Too few arguments\n");
        exit(1);
    }
    if (chmod(argv[2], S_IRUSR | S_IWUSR | S_IXUSR) == -1) {
        int error = errno;

        fprintf(stderr, "Error %d (%s)\n", error, strerror(error));

        return 1;
    }
}