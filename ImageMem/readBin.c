#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
				char c;
				int p;
} bnp;

typedef struct {
				char* buffer;
				int size;
				int pos;
} read_buffer;

#define BUFFER_SIZE 1024

int pow2[10];

void showOneBit(bnp* binp) {
				if((binp->c & pow2[binp->p]) == pow2[binp->p]) printf("1");
				else printf("0");
				binp->p = (binp->p + 1) % 8;
}

void showNBits(read_buffer *rdb, int n_arg) {
				int nb_bits = 0;
				bnp *binp = (bnp*) malloc(sizeof(bnp));
				binp->p = 0;
				rdb->pos = 0;
				while(nb_bits < n_arg) {
								if(binp->p == 0) {
												binp->c = rdb->buffer[rdb->pos++];
												if(rdb->pos >= rdb->size) {
																printf("Demer\n");
																return;
												}
								}
								showOneBit(binp);
								nb_bits++;
				}
}


void showBit(char c) {
				int i;
				for(i=0; i<8; i++) {
								if((c & pow2[7-i]) == pow2[7-i]) printf("1");
								else printf("0");
				}
				printf("\n");
}

void showBufferBin(read_buffer* rb) {
				int i;
				for(i=0; i<rb->size; i++) showBit(rb->buffer[i]);
}

read_buffer* readFile(int fd) {
				read_buffer *rb = (read_buffer*) malloc(sizeof(read_buffer));
				rb->buffer = (char*) calloc(BUFFER_SIZE, sizeof(char));
				if((rb->size = read(fd, rb->buffer, BUFFER_SIZE)) < 0)
								return NULL;
				return rb;
}

void show(read_buffer *rb) {
				write(STDOUT_FILENO, rb->buffer, rb->size);
}

int main(int argc, char** argv) { 
  int fd;
  read_buffer *rb;
	int i;
	if(argc == 1) {
					printf("Use : %s [fichier bin]\n", argv[0]);
					return EXIT_FAILURE;
	}
	if((fd = open(argv[1], O_RDONLY)) < 0) {
					printf("Couldn't open %s\n", argv[1]);
					return EXIT_FAILURE;
	}
	pow2[0] = 1;
	for(i=1; i<10; i++) pow2[i] = pow2[i-1]*2;
	rb = readFile(fd);
	showNBits(rb, 60);
	printf("\n");
	showNBits(rb, 2);
	printf("\n");
	showNBits(rb, 14);
	printf("\n");
	return EXIT_SUCCESS;
}
