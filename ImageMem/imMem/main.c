#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "structToBin.h"

void combineTwoLines(bin_line *l1, bin_line *l2) {
	int i;
	for(i=0; i<39; i++) printf("%c", l1->content[i]);
	printf("%c", combine_two(l1->content[39], l2->content[0], 4));
	for(i=1; i<40; i++) printf("%c", l2->content[i]);
}

int newLineBeforeEnd(char *buffer, int size, int pos) {
				int i;
				for(i=pos; i<size; i++) if(buffer[i] == '\n') return 1;
				return 0;
}

int main(int argc, char** argv) {
	int i;
	int index = 0;
	int fd;
	char *buffer = (char*) calloc(1024, sizeof(char));
	char *tmp = (char*) calloc(1024, sizeof(char));
	ligne *l = NULL;
	if(argc == 1) {
		printf("Use : %s [filename]\n", argv[0]);
		return EXIT_FAILURE;
	}
	fd = open(argv[1], O_RDONLY);
	if(fd <= 0) {
		printf("Couldn't open %s\n", argv[1]);
		return EXIT_FAILURE;
	}
	bin_line *l1 = (bin_line*) calloc(1, sizeof(bin_line));
	bin_line *l2 = (bin_line*) calloc(1, sizeof(bin_line));
	l1->position = END;
	l2->position = BEGIN;
	int nb_lus;
	int tic = 0;
	nb_lus = read(fd, buffer, 1023);
	do {
		if(!newLineBeforeEnd(buffer, 1024, index)) {
			for(i=index; i<1024; i++) tmp[i-index] = buffer[i];
			nb_lus = read(fd, tmp + (1024 - index), 1024 - 1);
			buffer = tmp;
			index = 1;
		}
		/* write(STDOUT_FILENO, buffer, 1023); */
		if(nb_lus == 0) break;
		l = getLine(buffer, &index);
		if(tic % 2 == 0) fillStruct(l, l1);
		if(tic % 2 == 1) {
						fillStruct(l, l2);
						combineTwoLines(l1, l2);
		}
		/*if(l != NULL) printLine(l);*/
		index++;
		tic++;
	} while(l != NULL);
	/*
	printLine(getLine(buffer, &index));
	index++;
	printLine(getLine(buffer, &index));
	print_long_long_int_bin(tmp);
	printf("\n");
	print_long_long_int_bin(tmp << 4);
	printf("\n");
	print_chars((char*)(&tmp), 8);
	*/
	/*
	int i;
	tmp <<= 4;
	for(i=0; i<8; i++) printf("%c", ((char*) (&tmp))[7-i]);
	 */
	 /*
	int t = 1023;
	printf("%c", combine_two(0b11110010, 0b11001100, 3));
	printf("%c%c", ((char*) (&t))[0], ((char*) (&t))[1]);
	*/
	/*
	read(fd, buffer, 1023);
	do {
		if(index > (1023 - 316)) {
			for(i=index; i<1023; i++)
				tmp[i - 1023 + index] = buffer[i];
		}
		buffer = tmp;
		read(fd, buffer + index, 1023 - index);
		l = getLine(buffer, &index);
	} while(l != NULL);
	*/
	/*
	while(getLine(buffer, &index) != NULL) index++;
	return EXIT_SUCCESS;
	ligne *l = getLine(buffer, &index);
	*/
	/*
	l1->position = END;
	fillStruct(l, l1);
	index++;
	l = getLine(buffer, &index);
	l2->position = BEGIN;
	fillStruct(l, l2);
	combineTwoLines(l1, l2);
	*/
}
