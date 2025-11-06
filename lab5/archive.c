#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 4096
#define FILENAME_SIZE 1024

/*
./archiver arch_name –i(--input) file1
*/
struct arch_header {
    char arch_name[FILENAME_SIZE];
    size_t files_count;
    int64_t size;
};


void process_archive(char* arch_name, int* arch_fd);
void create_archive(char* arch_name, int* arch_fd);

int process_filenames(int argc, char** argv, char** filenames);

void options_processing(int rez, int arch_fd, char** filenames, int count);

void input_files(int arch_fd, char** filenames, int files_count);
void extract(char* filename);
void print_archive_info(int arch_fd);
void help();

void update_count_and_size_in_header(char* arch_name, char* filenames[FILENAME_SIZE], int count);
off_t get_file_size(char* filename);

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Слишком мало аргументов\nДля получения справки: %s -h\n", argv[0]);
        exit(1);
    }

    const char* options = "iesh";
    static struct option arg_options[] = {
        {"input", no_argument, NULL, 'i'},
        {"extract", no_argument, NULL, 'e'},
        {"stat", no_argument, NULL, 's'},
        {"help", no_argument, NULL, 'h'},
        {0, 0, 0, 0}
    };

    int rez;
    int arch_fd;
    while ((rez = getopt_long(argc, argv, options, arg_options, NULL)) != -1) {
        if (argc > 2 && argv[1][0] != '-') {
            process_archive(argv[1], &arch_fd);
        }

        // обработка имён файлов
        char* filenames[FILENAME_SIZE];
        int count = process_filenames(argc, argv, filenames);

        if (count > 0) {
            update_count_and_size_in_header(argv[1], filenames, count);
        }
        
        options_processing(rez, arch_fd, filenames, count);
    }
    close(arch_fd);
    return 0;
}

void process_archive(char* arch_name, int* arch_fd) {
    *arch_fd = open(arch_name, O_RDWR | O_APPEND, 0644);
    
    if (*arch_fd == -1) {   
        if (errno == ENOENT) { // файл не существует    
            create_archive(arch_name, arch_fd);
        }
        else { // другая ошибка
            perror("Ошибка открытия файла\n");
            exit(1);
        }
    }
}

void create_archive(char* arch_name, int* arch_fd) {
    printf("Создаём архив\n");

    *arch_fd = open(arch_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (*arch_fd == -1) {
        perror("Ошибка создания файла\n");
        exit(1);
    }

    struct arch_header header = {
        .arch_name = {},
        .files_count = 0,
        .size = 0
    };
    snprintf(header.arch_name, sizeof(header.arch_name), "%s", arch_name);

    int res = write(*arch_fd, &header, sizeof(header));
    if (res == -1) {
        perror("Ошибка записи заголовка архива");
        close(*arch_fd);
        exit(1);
    }
}

int process_filenames(int argc, char** argv, char** filenames) {
    int count = 0;
    if (optind < argc) {
        while (optind < argc) {
            filenames[count++] = argv[optind++];
        }
    }
    return count;
}

void options_processing(int rez, int arch_fd, char** filenames, int count) {
    switch (rez) {
    case 'i':
        input_files(arch_fd, filenames, count);
        break;

    case 'e':
        // extract();
        break;

    case 's':
        print_archive_info(arch_fd);
        
        break;

    case 'h':
        help();
        break;
    
    default:
    {
        break;
    }
    }
}

void input_files(int arch_fd, char** filenames, int files_count) {
    lseek(arch_fd, 0, SEEK_END);
    
    for (int i = 0; i < files_count; ++i) {
        int source_fd = open(filenames[i], O_RDONLY);
        if (source_fd == -1) {
            int error = errno;
            fprintf(stderr, "open source error: %s\n", strerror(error));
            exit(1);
        }
        
        char buf[BUF_SIZE];
        ssize_t n;

        while ((n = read(source_fd, buf, sizeof(buf))) > 0) {
            ssize_t wr_n = write(arch_fd, buf, n); 
            printf("Записано %ld байт для %s\n", wr_n, filenames[i]);

            if (wr_n != n) {
                int error = errno;
                fprintf(stderr, "Ошибка записи файла: %s\n", strerror(error));
                close(source_fd);
                exit(1);
            }   
            
        }
        if (n == -1) {
            int error = errno;
            fprintf(stderr, "Ошибка чтения файла: %s\n", strerror(error));
            close(source_fd);
            exit(1);
        }
        close(source_fd);
    }    
}

void extract(char* filename) {
    printf("Извлекаем %s", filename);
}

enum byte_sizes {
    BYTES = 0,
    KB, 
    MB, 
    GB
};

void print_archive_info(int arch_fd) {    
    lseek(arch_fd, 0, SEEK_SET);
    struct arch_header header = {0};

    int n = read(arch_fd, &header, sizeof(header));
    if (n == -1) {
        int error = errno;
        fprintf(stderr, "Ошибка чтения: %s\n", strerror(error));
        exit(1);
    }

    double formatted_size = (double)header.size;
    
    int unit_num = 0;
    while (unit_num <= 3 && formatted_size > 1024.0) {
        ++unit_num;
        formatted_size /= 1024.0;
    }

    char* unit;
    int dec_places = 3;
    switch (unit_num)
    {
    case BYTES:
        unit = "байт";
        dec_places = 0;
        break;

    case KB:
        unit = "KB";
        break;
        
    case MB:
        unit = "MB";
        break;
        
    case GB:
        unit = "GB";
        break;
    
    default:
    {
        fprintf(stderr, "Ошибка: неизвестная единица измерения\n");
        break;
    }
    }

    printf("Имя архива: %s\n", header.arch_name);
    printf("Количество файлов: %ld\n", header.files_count);
    printf("Объём данных: %.*f %s\n", dec_places, formatted_size, unit);
}

void update_count_and_size_in_header(char* arch_name, char* filenames[FILENAME_SIZE], int count) {
    int arch_fd = open(arch_name, O_RDWR, 0644);
    if (arch_fd == -1) {
        perror("Ошибка открытия файла");
        exit(1);
    }

    struct arch_header header;
    int rd_n = read(arch_fd, &header, sizeof(struct arch_header));
    if (rd_n == -1) {
        perror("Ошибка чтения заголовка");
        exit(1);
    }

    header.files_count += count;
    for (int i = 0; i < count; ++i) {
        header.size += get_file_size(filenames[i]);
    }

    lseek(arch_fd, 0, SEEK_SET);
    int wr_n = write(arch_fd, &header, sizeof(header));
    // printf("Записано %d байт заголовка\n", wr_n);
    
    if (wr_n == -1) {
        perror("Ошибка записи заголовка");
        exit(1);
    }

    close(arch_fd);
}

off_t get_file_size(char* filename) {
    struct stat st;

    int n = stat(filename, &st);
    if (n == -1) {
        perror("Ошибка при вызове stat()");
        return -1;
    }

    printf("Размер %s = %ld\n", filename, st.st_size);
    return st.st_size;
}

void help() {
    printf(
        "  ИМЯ\n\t"
            "archiver - утилита примитивного архиватора без сжатия\n\n"
        
        "  ИСПОЛЬЗОВАНИЕ\n\t"
            "./archive [ИМЯ_АРХИВА] [ОПЦИИ] [ФАЙЛ...]\n\n"

        "  ОПЦИИ\n"
        "-i, --input\n\t"
            "Добавляет файлы в архив\n"
        "-e, --extract\n\t"
            "Удаляет файлы в архив\n"
        "-s, --stat\n\t"
            "Выводит текущее состояние архива\n"
        "-h, --help\n\t"
            "Выводит справку по работе архива\n\n"
    );
}
