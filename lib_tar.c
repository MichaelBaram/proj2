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
	free(header);
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
    free(header);
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
    free(header);
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
    free(header);
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
    free(header);
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
    free(pathCpy);
    return NPath;
}

char* getEndPath(char *path,int lenPath){
    int len = 0;
    const char delim = '/';
    char* token = strtok(path, &delim);
    while( len < lenPath-1 ) {
        len++;

        token = strtok(NULL, &delim);
    }
    return token;
}




int findPathFromFilename(int tar_fd, char *path,char *filename){

    struct posix_header *header = malloc(sizeof(struct posix_header));
    while(read(tar_fd,header, sizeof(struct posix_header))>0){

        char* pathCpy = (char *)calloc(strlen(header->name),sizeof(char));
        strcpy(pathCpy,header->name);
        if(getEndPath(pathCpy,depthPath(pathCpy)) != NULL && strcmp(filename,getEndPath(pathCpy,depthPath(pathCpy)))==0){
                free(pathCpy);
                strcpy(path,header->name);
                lseek(tar_fd,0,SEEK_SET);
                return 1;
        }
        free(pathCpy);
        unsigned int size = TAR_INT(header->size);
        if(size%512!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
        }else{
            lseek(tar_fd,((size/512))*512,SEEK_CUR);
        }

    }
    free(header);
    lseek(tar_fd,0,SEEK_SET);
    return 0;
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



int list(int tar_fd, char *path, char **entries, size_t *no_entries) {

    if(is_dir(tar_fd,path)==0 && is_symlink(tar_fd,path)==0) {
        *no_entries=0;
        return 0;
    }
    struct posix_header *header = malloc(sizeof(struct posix_header));
    int NPath = depthPath(path);
    int lenPath = strlen(path);
    int index = 0;
    while(read(tar_fd,header, sizeof(struct posix_header))>0){

        if (strcmp(path, header->name) == 0 && header->typeflag==SYMTYPE) {

            char* pathCpy = (char *)calloc(strlen(header->linkname),sizeof(char));
            strcpy(pathCpy,header->linkname);
            lseek(tar_fd,0,SEEK_SET);
            char* pathToFind = calloc(100, sizeof(char));
            findPathFromFilename(tar_fd,pathToFind,getEndPath(pathCpy,depthPath(pathCpy)));
            free(pathCpy);
            return list(tar_fd,pathToFind,entries,no_entries);
        }

        if(depthPath(header->name)-1==NPath) {
            //verifie si le fichier un repertoire de plus que le path
            char *str = calloc(lenPath, sizeof(char));
            char *strToCmp = strncat(str, header->name, lenPath);

            if (strcmp(path, strToCmp) == 0 && *no_entries>index) {
                    strcpy(entries[index],header->name);
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
    free(header);
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
                lseek(tar_fd,0,SEEK_SET);
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
            lseek(tar_fd,0,SEEK_SET);
            return (ssize_t) (size-offset) - byteRead;
        }
        if (strcmp(path,header->name)==0 && header->typeflag==SYMTYPE) {
            lseek(tar_fd,0,SEEK_SET);
            return read_file(tar_fd,header->linkname,offset,dest,len);
        }


        if(size%512!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
        }else{
            lseek(tar_fd,((size/512))*512,SEEK_CUR);
        }

    }
    free(header);
    lseek(tar_fd,0,SEEK_SET);
    return -1;
}


