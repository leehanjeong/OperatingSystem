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

int fd, n;
DIR *dp;
struct dirent *dent;
struct stat buffer;
int find = 0;
time_t mtime1, mtime2;
char buf[256];

void find_fifo(char *filename) {

    while(find == 0) {
        while ((dent = readdir(dp)) != NULL) {
	        if (strcmp(dent->d_name, filename) != 0) {
		        continue;
	        }
            else {
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

                if ((fd = open(filename, O_RDONLY)) == -1) {
                    perror("Open file");
                    exit(1);
                }
                find = 1;
                
                break;
            }
        }
	rewinddir(dp);
    }
    closedir(dp);
}

void find_regular(char *filename) {

    while(find == 0) {
        while ((dent = readdir(dp)) != NULL) {
	        if (strcmp(dent->d_name, filename) != 0) {
		        continue;
	        }
            else {
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
                    
                mtime1 = buffer.st_mtime;

                if ((fd = open(filename, O_RDONLY)) == -1) {
                    perror("Open file");
                    exit(1);
                }
                find = 1;
                
                break;
            }
        }
	rewinddir(dp);
    }
    closedir(dp);
}

void read_fifo() {

    while(1) {
    
        n = read(fd, buf, 255);
        buf[n] = '\0';
        if (n==-1) {
            perror("Read error");
        } else if (n == 0) continue;
        write(1, "Recv>> ", 7);
        write(1, buf, n);
        if(n == 1 && buf[0] == 'q') {
            write(1, "Terminate\n", 10);
            break;
        }
    
    }	
    close(fd);
}

void read_regular(char *filename) {
    
    while(1) {
        stat(filename, &buffer);
        mtime2 = buffer.st_mtime;

        if (mtime1 == mtime2) {
            continue;
        } 
        else {
            n = read(fd, buf, 255);
            buf[n] = '\0';
            if (n==-1) {
                perror("Read error");
            } else if (n == 0) continue;
            write(1, "Recv>> ", 7);
            write(1, buf, n);
            if(n == 1 && buf[0] == 'q') {
                write(1, "Terminate\n", 10);
                break;
            }
        }
    }	
    close(fd);
}

int main(int argc, char *argv[]) {
    int o;
    char *fname = NULL;
    char *arg = NULL;
    
    fname = getenv("COM_FILE");
    
    if((dp = opendir("./")) == NULL) {
	    perror("opendir: source");
	    exit(1);
    }

    if ((o = getopt(argc, argv, ":t:f:")) != -1) {
        switch(o) {
            case 't':
                arg = optarg;

                if (strcmp(arg, "f") == 0) {
                    if (fname == NULL) {
                        find_fifo("data.fifo");
                        read_fifo();
                    }
                    
                    else {
                        find_fifo(fname);
                        read_fifo();
                    }
                }

                if (strcmp(arg, "r") == 0) {
                    if (fname == NULL) {
                        find_regular("data.txt");
                        read_regular("data.txt");
                    }
                    else {
                        find_regular(fname);
                        read_regular(fname);
                    }
                }

                break;

            case 'f':
                fname = optarg;

                if (fname == NULL) {
                    perror("-f has no argument");
                    exit(1);
                }
                find_regular(fname);
                read_regular(fname);

                break;

            default:
                find_regular("data.txt");
                read_regular("data.txt");

                break;
        }
    }
    else {
        find_regular("data.txt");
        read_regular("data.txt");
    }

    return 0;
}
