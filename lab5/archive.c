#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define BUF_SIZE 4096
#define FILENAME_SIZE 256

struct arch_header {
    char arch_name[FILENAME_SIZE];
    size_t files_count;
    int64_t size;
};

struct file_info {
    char filename[FILENAME_SIZE];
    off_t offset; // позиция в файле архива
    off_t lenght;
};


void process_archive(char* arch_name, int* arch_fd);
void create_archive(char* arch_name, int* arch_fd);

int process_filenames(int argc, char** argv, char** filenames);

void options_processing(int rez, int arch_fd, char** filenames, int count);

void input_files(int arch_fd, char** filenames, int files_count);
void extract(int arch_fd, char** filenames, int count);
void print_archive_info(int arch_fd);
void help();

void update_archive_header(int arch_fd, char* filenames[FILENAME_SIZE], int count);
off_t get_file_size(char* filename);

struct arch_header read_archive_header(int arch_fd);
void write_archive_header(int arch_fd, struct arch_header* header);
bool dir_exists(char *path);
bool is_target_file_exists(int arch_fd, size_t files_count, char* target_filename);

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

    *arch_fd = open(arch_name, O_CREAT | O_RDWR | O_TRUNC, 0644);
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
        if (count > 0) {
            input_files(arch_fd, filenames, count);
        }
        break;

    case 'e':
        if (count > 0) {
            extract(arch_fd, filenames, count);
        }
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

void input_files(int arch_fd, char** filenames, int input_files_count) {
    struct arch_header header = read_archive_header(arch_fd);
    
    for (int i = 0; i < input_files_count; ++i) {
        if (is_target_file_exists(arch_fd, header.files_count, filenames[i])) {
            fprintf(stderr, "Ошибка: файл %s уже существует в архиве\n"
                            "Для добавления нового файла выполните извлечение старого\n"
                , filenames[i]);

            close(arch_fd);
            exit(1);
        }
    }

    struct file_info infos[input_files_count];

    // резервируем место под заголовки
    off_t headers_size = sizeof(struct arch_header) + input_files_count * sizeof(struct file_info);
    lseek(arch_fd, headers_size, SEEK_SET);

    for (int i = 0; i < input_files_count; ++i) {
        int source_fd = open(filenames[i], O_RDONLY);
        if (source_fd == -1) {
            int error = errno;
            fprintf(stderr, "Ошибка открытия добавляемого файла: %s\n", strerror(error));
            exit(1);
        }

        struct file_info info = {0};
        snprintf(info.filename, sizeof(info.filename), "%s", filenames[i]);

        info.offset = lseek(arch_fd, 0, SEEK_CUR);

        
        char buf[BUF_SIZE];
        ssize_t n;

        while ((n = read(source_fd, buf, sizeof(buf))) > 0) {
            write(arch_fd, buf, n);            
            info.lenght += n;          
        }
        close(source_fd);
        infos[i] = info;

    }
    

    // запись arch_header и file_info
    lseek(arch_fd, sizeof(struct arch_header), SEEK_SET);

    write(arch_fd, infos, sizeof(infos));

    update_archive_header(arch_fd, filenames, input_files_count);

    lseek(arch_fd, 0, SEEK_END);
}

void extract(int arch_fd, char** filenames, int target_count) {
    struct arch_header header = read_archive_header(arch_fd);

    for (int i = 0; i < target_count; ++i) {
        if (!is_target_file_exists(arch_fd, header.files_count, filenames[i])) {
            fprintf(stderr, "Ошибка: файл %s не найден в архиве\n", filenames[i]);
            close(arch_fd);
            exit(1);
        }
    }


    int tmp_arch_fd = open("tmp_archive", O_CREAT | O_EXCL | O_RDWR, 0644);
    if (tmp_arch_fd == -1) {
        perror("Ошибка создания временного архива");
        exit(1);
    }
    // резервирование места под заголовки
    off_t headers_size = sizeof(struct arch_header) + header.files_count * sizeof(struct file_info);
    lseek(tmp_arch_fd, headers_size, SEEK_SET);
    
    if (header.files_count <= 0) {
        fprintf(stderr, "Ошибка: архив не содержит файлов\n");
        close(tmp_arch_fd);
        remove("tmp_archive");
        return;
    }

    // создание директории для извлечения файлов
    char dir_name[BUF_SIZE];
    snprintf(dir_name, sizeof(dir_name), "ex_%s", header.arch_name);

    if (!dir_exists(dir_name)) {
        if (mkdir(dir_name, 0777) != 0) {
            int error = errno;
            fprintf(stderr, "Ошибка создания директории для извлеченных файлов: %s\n", strerror(error));
            close(tmp_arch_fd);
            close(arch_fd);
            exit(1);
        }
    }

    size_t new_file_index = 0;
    off_t new_data_size = 0;

    // перебор заголовков файлов
    for (size_t j = 0; j < header.files_count; ++j) {         
        off_t cur_info_offset = sizeof(struct arch_header) + j * sizeof(struct file_info);
        lseek(arch_fd, cur_info_offset, SEEK_SET);
        
        struct file_info info = {0};
        if (read(arch_fd, &info, sizeof(struct file_info)) != sizeof(struct file_info)) {
            perror("Ошибка чтения заголовка файла");
            close(arch_fd);
            close(tmp_arch_fd);
            exit(1);
        }
        
        bool should_extract_from_archive = false;
        for (int i = 0; i < target_count; ++i) {
            if (strcmp(info.filename, filenames[i]) == 0) {
                should_extract_from_archive = true;
                break;
            }
        }

        lseek(arch_fd, info.offset, SEEK_SET);

        // запись нецелевых файлов в новый временный архив
        if (!should_extract_from_archive) {
            off_t new_data_offset = lseek(tmp_arch_fd, 0, SEEK_CUR);
            info.offset = new_data_offset;

            // чтение из старого и запись в новый архив данных файла
            char buf[BUF_SIZE];
            int bytes_left = info.lenght;
            while (bytes_left > 0) {
                size_t nbytes = bytes_left > (ssize_t)sizeof(buf) ? sizeof(buf) : (size_t)bytes_left;

                ssize_t read_n = read(arch_fd, buf, nbytes);
                
                if (read_n <= 0) {
                    if (read_n < 0) perror("Ошибка чтения файла из архива");
                    break;
                }
                
                
 
                if (write(tmp_arch_fd, buf, read_n) != read_n) {
                    perror("Ошибка записи в новый архив");
                    close(arch_fd);
                    close(tmp_arch_fd);
                    exit(1);
                } 
                
                bytes_left -= read_n;
            }
            new_data_size += info.lenght;


            off_t new_headers_offset = sizeof(struct arch_header) + new_file_index * sizeof(struct file_info);

            // запись обновленного заголовка файла
            lseek(tmp_arch_fd, new_headers_offset, SEEK_SET);
            
            if (write(tmp_arch_fd, &info, sizeof(struct file_info)) != sizeof(struct file_info)) {
                perror("Ошибка записи заголовка файла во временный архив");
                close(arch_fd);
                close(tmp_arch_fd);
                exit(1);
            }

            lseek(tmp_arch_fd, 0, SEEK_END);

            ++new_file_index;
        }
        else { // извлечение данных из архива в новый файл
            char extracted_file_name[BUF_SIZE * 2];
            snprintf(extracted_file_name, sizeof(extracted_file_name), 
                "%s/%s", dir_name, info.filename);

            int extracted_file_fd = open(extracted_file_name, 
                O_CREAT | O_WRONLY | O_TRUNC, 0644);
            if (extracted_file_fd == -1) {
                perror("Ошибка создания извлеченного файла");
                close(arch_fd);
                close(tmp_arch_fd);
                exit(1);
            }

            char buf[BUF_SIZE];
            ssize_t bytes_left = info.lenght;

            while (bytes_left > 0) {
                size_t nbytes = bytes_left > (ssize_t)sizeof(buf) ? sizeof(buf) : (size_t)bytes_left;
                int read_n = read(arch_fd, buf, nbytes);
                
                if (read_n <= 0) {
                    if (read_n < 0) perror("Ошибка чтения извлекаемого файла");
                    break;
                }

                if (write(extracted_file_fd, buf, read_n) != read_n) {
                    perror("Ошибка записи извлекаемого файла");
                    close(arch_fd);
                    close(tmp_arch_fd);
                    close(extracted_file_fd);
                    exit(1);
                }
                bytes_left -= read_n;
            }
            close(extracted_file_fd);
        }
    }

    header.files_count = new_file_index;
    header.size = new_data_size;

    lseek(tmp_arch_fd, 0, SEEK_SET);
    if (write(tmp_arch_fd, &header, sizeof(struct arch_header)) != sizeof(struct arch_header)) {
        perror("Ошибка записи заголовка архива");
        close(arch_fd);
        close(tmp_arch_fd);
        exit(1);
    }

    close(arch_fd);
    close(tmp_arch_fd);
    
    remove(header.arch_name);

    if (rename("tmp_archive", header.arch_name) != 0) {
        perror("Ошибка переименования временного архива");
        exit(1);
    }
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
        fprintf(stderr, "Ошибка чтения при выводе информации об архиве: %s\n", strerror(error));
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
    printf("Для получения объёма всего архива введите ls -l %s\n", header.arch_name);
}

void update_archive_header(int arch_fd, char* filenames[FILENAME_SIZE], int count) {
    struct arch_header header = read_archive_header(arch_fd);

    header.files_count += count;
    for (int i = 0; i < count; ++i) {
        header.size += get_file_size(filenames[i]);
    }

    lseek(arch_fd, 0, SEEK_SET);

    if (write(arch_fd, &header, sizeof(header)) == -1) {
        perror("Ошибка записи заголовка");
    }
    close(arch_fd);
}

off_t get_file_size(char* filename) {
    struct stat st;

    if (stat(filename, &st) == -1) {
        perror("Ошибка при вызове stat()");
        return -1;
    }
    return st.st_size;
}

struct arch_header read_archive_header(int arch_fd) {
    struct arch_header header;

    lseek(arch_fd, 0, SEEK_SET);

    int n = read(arch_fd, &header, sizeof(struct arch_header));

    if (n == -1) {
        perror("Ошибка чтения заголовка");
        exit(1);
    }
    return header;
}

void write_archive_header(int fd, struct arch_header* header) {
    int n = write(fd, header, sizeof(struct arch_header));

    if (n == -1) {
        perror("Ошибка записи заголовка");
        close(fd);
        exit(1);
    }
}

bool dir_exists(char *path) {
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        return true;
    }
    return false;
}

bool is_target_file_exists(int arch_fd, size_t arch_files_count, char* target_filename) {
    for (size_t i = 0; i < arch_files_count; ++i) {
        off_t cur_info_offset = sizeof(struct arch_header) + i * sizeof(struct file_info);
        lseek(arch_fd, cur_info_offset, SEEK_SET);

        struct file_info info = {0};
        read(arch_fd, &info, sizeof(struct file_info));

        if (strcmp(info.filename, target_filename) == 0) {
            return true;
        }
    }

    return false;
}

void help() {
    printf(
        "  ИМЯ\n\t"
            "archiver - утилита примитивного архиватора без сжатия\n\n"
        
        "  ИСПОЛЬЗОВАНИЕ\n\t"
            "./archiver [ИМЯ_АРХИВА] [ОПЦИИ] [ФАЙЛ...]\n\n"

        "  ОПЦИИ\n"
        "-i, --input\n\t"
            "Добавляет файлы в архив\n"
        "-e, --extract\n\t"
            "Удаляет файлы в архив и помещает в директорию `ex_[ИМЯ_АРХИВА]`\n"
        "-s, --stat\n\t"
            "Выводит текущее состояние архива\n"
        "-h, --help\n\t"
            "Выводит справку по работе архива\n\n"
    );
}
