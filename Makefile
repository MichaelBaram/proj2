CFLAGS=-g -Wall -Werror

all: tests lib_tar.o

lib_tar.o: lib_tar.c lib_tar.h
	gcc -g -Wall -Werror   -c -o lib_tar.o lib_tar.c

#
tests: tests.c lib_tar.o
	gcc -g -Wall -Werror    tests.c lib_tar.o   -o tests
	tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c testf1.txt testf2.txt testdir>   testarchive
	./tests testarchive.tar testdir/

clean:
	rm -f lib_tar.o tests soumission.tar

submit: all
	tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c *.h *.c Makefile > soumission.tar
