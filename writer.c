// 20191650 이한정

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

struct stat buffer;
DIR *dp;
struct dirent *dent;
int find_fifo = 0;
int find_regular = 0;
mode_t mode = S_IRUSR | S_IWUSR;
int fd;

void check_fifo(char *filename) {
    while ((dent = readdir(dp)) != NULL) { // data.fifo가 존재하는지 확인
        if (strcmp(dent->d_name, filename) != 0) {
            continue;
        }

        if (strcmp(dent->d_name, filename) == 0) { // data.fifo가 존재함
            stat(filename, &buffer);

            // 파일 종류 확인 - fifo파일인지
            if (!(S_ISFIFO(buffer.st_mode))) {
                perror("file is not a FIFO file");
                exit(1);
            }
            // 파일 접근 권한 확인
            if (((buffer.st_mode & S_IWUSR) == 0) || ((buffer.st_mode & S_IRGRP)!= 0) || ((buffer.st_mode & S_IWGRP) != 0) || ((buffer.st_mode & S_IROTH) != 0) || ((buffer.st_mode & S_IWOTH) != 0)) {
                perror("file must be protected");
                exit(1);
            }
            find_fifo = 1;
        }

        if (find_fifo == 1) {
            break;
        }
    }

    if (find_fifo == 0) { // fifo 파일이 존재하지 않는 경우 처리
        if (mkfifo(filename, mode) == -1) {
            perror("can't make FIFO file");
            exit(1);
        }
    }
    if ((fd = open(filename, O_WRONLY | O_TRUNC)) == -1) {
        perror("Open file");
        exit(1);
    }
}

void check_regular(char *filename) {
    while ((dent = readdir(dp)) != NULL) { 
        if (strcmp(dent->d_name, filename) != 0) {
            continue;
        }

        if (strcmp(dent->d_name, filename) == 0) { // data.txt가 존재함
            stat(filename, &buffer);

            // 파일 종류 확인 - 일반파일인지
            if (!(S_ISREG(buffer.st_mode))) {
                perror("file is not a general file");
                exit(1);
            }
            // 파일 접근 권한 확인
            if (((buffer.st_mode & S_IWUSR) == 0) || ((buffer.st_mode & S_IRGRP)!= 0) || ((buffer.st_mode & S_IWGRP) != 0) || ((buffer.st_mode & S_IROTH) != 0) || ((buffer.st_mode & S_IWOTH) != 0)) {
                perror("file must be protected");
                exit(1);
            }
            find_regular = 1;
        }

        if (find_regular == 1) {
            break;
        }
    }
    if (find_regular == 0) {
        if (creat(filename, mode) == -1) {
            perror("can't make regular file");
            exit(1);
        }
    }
    if ((fd = open(filename, O_WRONLY | O_TRUNC)) == -1) {
        perror("Open file");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    int n, o ;
    char buf[256];

    char *fname = NULL;
    char *arg = NULL;

    fname = getenv("COM_FILE"); // 환경변수로 부터 얻어온 파일 이름

    if ((dp = opendir("./")) == NULL) {
        perror("opendir: source");
        exit(1);
    }

    if ((o = getopt(argc, argv, "t:f:")) != -1) {
        switch(o) {
            case 't':
                arg = optarg;
                if (strcmp(arg, "f") == 0) { // fifo 모드

                    if (fname == NULL) { // 환경변수 파일이 NULL이면 data.fifo
                        check_fifo("data.fifo");
                    }
                    else { // 환경변수 파일이 존재 하는 경우
                        check_fifo(fname);
                    }
                }

                if (strcmp(arg, "r") == 0) { // regular 모드 

                    if (fname == NULL) {
                        check_regular("data.txt");
                    }
                    else {
                        check_regular(fname);
                    }
                }
                break;

            case 'f':
                fname = optarg;

                if (fname == NULL) {
                    perror("-f has no argument");
                    exit(1);
                }

                check_regular(fname);
                break;

            default:
                check_regular("data.txt");
                break;

        }   
    }
    else {
        check_regular("data.txt");
    }
    /////////////////////////////////////////////////

    
    while(1) {
        write(1, ">> ", 3);
        n = read(0, buf, 255);
        buf[n] = '\0';
        if( n > 0 ) {
            if( write(fd, buf, n) != n ) {
                perror("Write error");
            }
        } else if (n==-1) {
            perror("Read error");
        }
        if(n == 1 && buf[0] == 'q') {
            write(1, "Terminate\n", 10);
            break;
        }
        write(1, buf, n);
    }    
    
    close(fd);

    return 0;
}
