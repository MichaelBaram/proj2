#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "lib_tar.h"
#include <errno.h>
#define BUFSIZE 100
/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {
    int nbHeaders = 0;
    struct posix_header *header = malloc(sizeof(struct posix_header));
    while(read(tar_fd,header, sizeof(struct posix_header))>0){
        long sum = 0;
        for(int i =0; i<512;i++){
            if(i>=148&&i<156){
                sum += ' ';
            }else{
                char* c = (char*) header+i;
                sum += *c;
            }

        }
        if(sum==256){
            break;
        }

        if(strcmp(header->magic,TMAGIC)!=0){
            lseek(tar_fd,0,SEEK_SET);
            return -1;
        }
        if(strncmp(header->version,TVERSION,2)!=0){
            lseek(tar_fd,0,SEEK_SET);
            return -2;
        }
        if(sum!=(TAR_INT(header->chksum))){
            lseek(tar_fd,0,SEEK_SET);
            return -3;
        }
        nbHeaders++;
        unsigned int size = TAR_INT(header->size);
        if(size%512!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
        }else{
            lseek(tar_fd,((size/512))*512,SEEK_CUR);
        }
    }

    lseek(tar_fd,0,SEEK_SET);

    return nbHeaders;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    struct posix_header *header = malloc(sizeof(struct posix_header));
    while(read(tar_fd,header, sizeof(struct posix_header))>0){
        if(strcmp(path,header->name)==0){
            lseek(tar_fd,0,SEEK_SET);
            return 1;
        }

        unsigned int size = TAR_INT(header->size);
        if(size%512!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
        }else{
            lseek(tar_fd,((size/512))*512,SEEK_CUR);
        }

    }
    lseek(tar_fd,0,SEEK_SET);
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
    struct posix_header *header = malloc(sizeof(struct posix_header));
    while(read(tar_fd,header, sizeof(struct posix_header))>0){

        if(strcmp(path,header->name)==0){
            if(header->typeflag==DIRTYPE){
                lseek(tar_fd,0,SEEK_SET);
                return 1;
            }
        }

        unsigned int size = TAR_INT(header->size);
        if(size%512!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
        }else{
            lseek(tar_fd,((size/512))*512,SEEK_CUR);
        }

    }
    lseek(tar_fd,0,SEEK_SET);
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
    struct posix_header *header = malloc(sizeof(struct posix_header));
    while(read(tar_fd,header, sizeof(struct posix_header))>0){

        if(strcmp(path,header->name)==0){
            if(header->typeflag==REGTYPE || header->typeflag==AREGTYPE){
                lseek(tar_fd,0,SEEK_SET);
                return 1;
            }
        }

        unsigned int size = TAR_INT(header->size);
        if(size%512!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
        }else{
            lseek(tar_fd,((size/512))*512,SEEK_CUR);
        }

    }
    lseek(tar_fd,0,SEEK_SET);
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */

 //printf("Header name : %s\nHeader flag : %c\n", header->name,header->typeflag);
int is_symlink(int tar_fd, char *path) {
    struct posix_header *header = malloc(sizeof(struct posix_header));
    while(read(tar_fd,header, sizeof(struct posix_header))>0){


        if(strcmp(path,header->name)==0){

            if(header->typeflag==SYMTYPE){
                lseek(tar_fd,0,SEEK_SET);
                return 1;
            }
        }

        unsigned int size = TAR_INT(header->size);
        if(size%512!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
        }else{
            lseek(tar_fd,((size/512))*512,SEEK_CUR);
        }

    }
    lseek(tar_fd,0,SEEK_SET);
    return 0;
}
int depthPath(char *path){
    char* pathCpy = (char *)calloc(strlen(path),sizeof(char));
    strcpy(pathCpy,path);
    char *token;
    size_t NPath = 0;
    const char delim = '/';
    token = strtok(pathCpy, &delim);
    while( token != NULL ) {
        NPath++;
        token = strtok(NULL, &delim);
    }
    return NPath;
}

/**
 * Lists the entries at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entry in entries.
 *                   The callee set it to the number of entry listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */

void error(int err,char *msg){
    fprintf(stderr,"%s a retourné %d, message d'erreur : %s\n", msg,err,strerror(errno));
    exit(EXIT_FAILURE);
}
    void deleten (char* path, char* buf, int nchar){
        //suppose buf is long enough
        int size  = strlen(path);
        int i = 0;
        while (i<size && i<(size-nchar)) {
            buf[i] = path[i];
            i++;
        }
    }
    void findfilefromslink (char* slink, char* buf,char* filename){
        // sizenamelink : size of name of link. Ex: dirarchive/myslink,
        //int sizenamelink = 7
        // filename est testsym.txt par exemple.
        //suppose buf is long enough;
        // slink : dirarchive/myslink par exemple
        int sizenamelink=0;
        int sizelink = strlen(slink);
        int i = sizelink-1;
        while(i>=0){
            if(slink[i]=='/')break;
            sizenamelink++;
            i--;
        }
        deleten(slink,buf,sizenamelink);
        strcat(buf,filename);
        printf("Le path fichier qui est pointe par le slink : %s\n", buf);
    }

int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    struct posix_header *header = malloc(sizeof(struct posix_header));
    int NPath = depthPath(path);
    int lenPath = strlen(path);
    int index = 0;
    while(read(tar_fd,header, sizeof(struct posix_header))>0){

        if(depthPath(header->name)-1==NPath) {
         //verifie si le fichier un repertoire de plus que le path
           char *str = calloc(lenPath, sizeof(char));
           char *strToCmp = strncat(str, header->name, lenPath);

           if (strcmp(path, strToCmp) == 0 && *no_entries>index) {
               if (header->typeflag==SYMTYPE) {
                   char filename[BUFSIZE];
                   char bufslink[BUFSIZE];
                   int sizelink;
                   sizelink = readlink( header->name, filename, (BUFSIZE*sizeof(char)));
                   if(sizelink == -1) error(sizelink,"readlink");
                   filename[sizelink] = '\0';

                   printf("----Nom du fichier pointe par slink: %s, SIZE : %d\n",filename,sizelink );
                   findfilefromslink(header->name,bufslink,filename);
                   printf("----BUFSLINK 2: %s, SIZE : %d\n",bufslink,sizelink );

                   strcpy(entries[index], bufslink);
               }
               else{
                   strcpy(entries[index], header->name);
               }
               index++;
           }
       }
   }

       unsigned int size = TAR_INT(header->size);
       if(size%512!=0){
           lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
       }
       else{
            lseek(tar_fd,((size/512))*512,SEEK_CUR);
        }
    *no_entries=index;
    lseek(tar_fd,0,SEEK_SET);
    if(index==0){
        return 0;
    }
    return 1;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {

    struct posix_header *header = malloc(sizeof(struct posix_header));
    while(read(tar_fd,header, sizeof(struct posix_header))>0){
        unsigned int size = TAR_INT(header->size);
        if(strcmp(path,header->name)==0 && (header->typeflag==REGTYPE || header->typeflag==AREGTYPE)){
            if(size<offset){
                return -2;
            }
            lseek(tar_fd,offset,SEEK_CUR);
            int nbByteToRead;
            if(*len < (size-offset)){
                nbByteToRead = *len;
            } else{
                nbByteToRead = size-offset;
            }
            int byteRead = read(tar_fd,dest,nbByteToRead);
            *len = byteRead;
            return (ssize_t) (size-offset) - byteRead;
        }
        if (header->typeflag==SYMTYPE) {

            //char *bufslink = (char*) malloc(BUFSIZE*sizeof(char));
            char filename[BUFSIZE];
            char bufslink[BUFSIZE];
            int sizelink;
            sizelink = readlink( header->name, filename, (BUFSIZE*sizeof(char)));
            if(sizelink == -1) error(sizelink,"readlink");
            filename[sizelink] = '\0';

            printf("----Nom du fichier pointe par slink: %s, SIZE : %d\n",filename,sizelink );
            findfilefromslink(header->name,bufslink,filename);
            return read_file(tar_fd, bufslink,offset,dest,len);
        }


        if(size%512!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
        }else{
            lseek(tar_fd,((size/512))*512,SEEK_CUR);
        }

    }
    lseek(tar_fd,0,SEEK_SET);
    return -1;
}
//2 tests list avec symbollink;
// et symboling de read_file;
