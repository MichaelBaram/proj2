#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "lib_tar.h"

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
    struct posix_header *header = malloc(sizeof(struct posix_header));
    while(read(tar_fd,header, sizeof(struct posix_header))>0){
        long sum = 0;
        for(int i =0; i<512;i++){
            if(i>=148&&i<156){
                sum += ' ';
            }
            char* c = (char*) header+i;
            sum += *c;
        }
        if(sum==256){
            continue;
        }
        if(strcmp(header->magic,TMAGIC)!=0){
            lseek(tar_fd,0,SEEK_SET);
            return -1;
        }
        if(strcmp(header->version,TVERSION)!=0){
            lseek(tar_fd,0,SEEK_SET);
            return -2;
        }
        if(sum!=(TAR_INT(header->chksum))){
            lseek(tar_fd,0,SEEK_SET);
            return -3;
        }

        unsigned int size = TAR_INT(header->size);
        if(size!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
        }
    }

    lseek(tar_fd,0,SEEK_SET);

    return 0;
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
        if(size!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
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
        if(size!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
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
        if(size!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
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
int is_symlink(int tar_fd, char *path) {
    struct posix_header *header = malloc(sizeof(struct posix_header));
    while(read(tar_fd,header, sizeof(struct posix_header))>0){

        if(strcmp(path,header->name)==0){
            if(header->typeflag==LNKTYPE){
                lseek(tar_fd,0,SEEK_SET);
                return 1;
            }
        }

        unsigned int size = TAR_INT(header->size);
        if(size!=0){
            lseek(tar_fd,((size/512)+1)*512,SEEK_CUR);
        }

    }
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
    return 0;
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
    return 0;
}
