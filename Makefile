CFLAGS=-g -Wall -Werror

all: tests lib_tar.o clean

lib_tar.o: lib_tar.c lib_tar.h
	gcc -g -Wall -Werror   -c -o lib_tar.o lib_tar.c

#
tests: tests.c lib_tar.o
	gcc -g -Wall -Werror    tests.c lib_tar.o   -o tests
	tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c dirarchive/testf1.txt dirarchive/testf2.txt dirarchive/testdir >   dirarchive/testarchive.tar
	./tests dirarchive/testarchive.tar dirarchive/testdir/
	# ca fonctionne
	#./tests dirarchive/testarchive.tar dirarchive/myslink ca fonctionne

clean:
	rm -f lib_tar.o tests soumission.tar

submit: all
	tar --posix --pax-option delete=".*" --pax-option delete="*time*" --no-xattrs --no-acl --no-selinux -c *.h *.c Makefile > soumission.tar
