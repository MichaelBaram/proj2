CFLAGS=-g -Wall -Werror

all: tests lib_tar.o

#cc -g -Wall -Werror   -c -o lib_tar.o lib_tar.c
lib_tar.o: lib_tar.c lib_tar.h

#cc -g -Wall -Werror    tests.c lib_tar.o   -o tests
tests: tests.c lib_tar.o
	tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c testf1.txt testf2.txt>   testarchive.tar

clean:
	rm -f lib_tar.o tests soumission.tar

submit: all
	tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c *.h *.c Makefile > soumission.tar
