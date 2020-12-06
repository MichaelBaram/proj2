#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
  char* string;

    if (argc < 3) {
        printf("Usage: %s tar_file fichier_existe\n", argv[0]);
        return -1;
    }
    printf("\n------DEBUT DE PROGRAMME DE TEST avec arg : %s %s -----\n",argv[1],argv[2]);


    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }
    printf("\n%s\n", "------TEST CHECK ARCHIVE -----");
    int ret = check_archive(fd);
    printf("check_archive returned %d\n", ret);

    printf("\n------TEST EXISTS fichier : %s-----\n",argv[2]);
    ret = exists(fd,argv[2]);
    if(ret == 1){string = "files exists";}
    else{string = "files does not exist";}
    printf("exists returned : %s\n", string);

    printf("\n------TEST IS DIRECTORY fichier : %s-----\n",argv[2]);
    ret = is_dir(fd,argv[2]);
    if(ret == 1){string = "files is a directory";}
    else{string = "files is not a directory";}
    printf("is_dir returned : %s\n", string);


    printf("\n------TEST IS FILE fichier : %s-----\n",argv[2]);
    ret = is_file(fd,argv[2]);
    if(ret == 1){string = "files is a file";}
    else{string = "files is not a file";}
    printf("is_file returned : %s\n", string);


    printf("\n------TEST IS SYMLINK fichier : %s-----\n",argv[2]);
    ret = is_symlink(fd,argv[2]);
    if(ret == 1){string = "files is a symlink";}
    else{string = "files is not a symlink";}
    printf("is_symlink returned : %s\n", string);

    /*size_t* entries = malloc(sizeof(size_t));

    ret = list(fd,argv[2],buf,entries);
    printf("list returned %d\n", ret);*/

    close(fd);
    return 0;
}
