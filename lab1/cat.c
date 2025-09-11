#include <stdio.h>
#include <ctype.h>

void print_visible(unsigned char c);

void cat() {


}

void print_visible(unsigned char c) {
    if (c >= 32 && c <= 126) {
        putchar(c); 
    } else if (c == '\n' || c == '\t' || c == '\r') {
        putchar(c); 
    } else if (c < 32) {
        printf("^%c", c + 64);
    } else if (c == 127) {
        printf("^?");
    } else {
        printf("M-");
        print_visible(c - 128);
    }
}


void command_b(char* buffer, int* strCnt) {
    if((buffer[0] == '\0') || (buffer[0] == '\n')) {
        fprintf(stdout, "\n");
    }
    else {
        ++strCnt;
        fprintf(stdout, "    %d  %s", *strCnt, buffer);
    }
}

void command_n(char* buffer, int* strCnt) {
    ++strCnt;
    fprintf(stdout, "    %d  %s", *strCnt, buffer);
}

void command_E(char* buffer) {
    if (strstr(buffer, "\n") != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        fprintf(stdout, "%s$\n", buffer);
    }
    else {
        fprintf(stdout, "%s", buffer);
    }
}

void command_v(char* buffer) {
    for (size_t i = 0; i < strlen(buffer); ++i) {
        print_visible(buffer[i]);
    }
}
			
void command_T(char* buffer) {
    for (size_t i = 0; i < strlen(buffer); ++i) {
        if (buffer[i] == '\t') {
            fprintf(stdout, "^I");
        }
        else {
            fprintf(stdout, "%c", buffer[i]);
        }
    }
}