#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct byteandpos {
	int *t;
	int pos;
} bnp;

unsigned long long int pow2[64];

void displayAsBits(char arg) {
	int i;
	for(i=7; i>=0; i--) 
		printf("%lld", ((arg/pow2[i]) % 2));
}

char makeBit(int tab[8]) {
	char c = tab[7];
	int i;
	for(i=6; i>=0; i--) c += pow2[7-i]*tab[i];
	return c;
}

void addBit(int *t, int bit, int *pos) {
	t[*pos] = bit;
	if(*pos == 7) printf("%c", makeBit(t));
	*pos = (*pos + 1) % 8;
}

void addBitStr(bnp *arg, int bit) {
	addBit(arg->t, bit, &(arg->pos));
}

void finalize(bnp *arg) {
	if(arg->pos != 0) {
		addBitStr(arg, 0);
		finalize(arg);
	}
}

void addIntWithNBits(bnp *arg, unsigned long long int n, int nb_bits) {
	int i;
	if(n < 0) {
		addIntWithNBits(arg, pow2[nb_bits + 1] + n, nb_bits);
		return;
	}
	for(i=nb_bits-1; i>=0; i--) {
		/* printf("avdiv\n"); */
		/* printf("%lld/(pow2[%d] = %lld)\n", n, i, pow2[i]); */
		addBitStr(arg, (n/pow2[i]) % 2);
		/* printf("apdiv\n"); */
	}
}

int next_state(int n) {
	int m = (n+1) % 33;
	/* printf("m = %d\n", m); */
	return m;
}

unsigned long long int addNewHexa(long int number, char hexa) {
	if(hexa >= '0' && hexa <= '9') number = number*16 + hexa - '0';
	else if(hexa >= 'a' && hexa <= 'f') number = number*16 + 10 + hexa - 'a';
	else printf("WTF ??\n");
	return number;
}

int main(int argc, char** argv) {
	bnp addChar;
	int fd, nb_lus;
	unsigned long long int number = 0;
	char *buffer = (char*) calloc(1024, sizeof(char));
	int i;
	int neg = 1;
	int state = 0;
	int n_bits;
	int nb_ligne = 0;
	addChar.t = (int*) calloc(8, sizeof(int));
	addChar.pos = 0;

	if(argc == 1) {
		printf("Use : %s [filename]\n", argv[0]);
		return EXIT_FAILURE;
	}
	fd = open(argv[1], O_RDONLY);

	pow2[0] = 1;
	for(i=1; i<64; i++) pow2[i] = pow2[i-1]*2;
	/*
	addBitStr(&addChar, 0);
	addBitStr(&addChar, 1);
	addBitStr(&addChar, 1);
	addBitStr(&addChar, 0);
	addBitStr(&addChar, 0);
	addBitStr(&addChar, 0);
	addBitStr(&addChar, 1);
	addBitStr(&addChar, 1);

	addBitStr(&addChar, 0);
	addBitStr(&addChar, 1);
	addBitStr(&addChar, 1);
	addBitStr(&addChar, 0);
	addBitStr(&addChar, 0);
	addBitStr(&addChar, 1);
	 */

	while((nb_lus = read(fd, buffer, 1023)) != 0) {
		/* printf("read %d bytes\n", nb_lus); */
		for(i=0; i<nb_lus; i++) {
			/* printf("i = %d\n", i); */
			/* printf("number = %lld\n", number); */
			switch(buffer[i]) {
				case('\n') :
				  break;
					/* printf("Ligne %d\n", l++); */
				case(' ') :
					n_bits = (state == 0 ? 60 : (state%2 == 1 ?  2 : 14));
					addIntWithNBits(&addChar, neg * ((long long int) number), n_bits);
					/* printf("Wrote %lld on %d bits\n", neg * number, n_bits); */
					number = 0;
					neg = 1;
					if(buffer[i] == '\n') printf("nb_ligne = %d\n", nb_ligne++);
					while((buffer[i] == ' ' || buffer[i] == '\n') && i < nb_lus) i++;
					if(buffer[i] == EOF) return EXIT_SUCCESS;
					if(i == nb_lus) break;
					i--;
					/* printf("i = %d\n", i); */
					/* printf("nb_lus = %d\n", nb_lus); */
					/* printf("state = %d\n", state); */
					state = next_state(state);
					/* printf("state = %d\n", state); */
					break;
				case(EOF) :
					return EXIT_SUCCESS;
					break;
				case('-') :
					neg = -1;
					break;
				default :
					number = addNewHexa(number, buffer[i]);
					break;
			}
						/* printf("Fin boucle for\n"); */
						/* printf("i = %d\n", i); */
		}
	}
	/* addIntWithNBits(&addChar, -1, 2); */
	finalize(&addChar);
	return EXIT_SUCCESS;
}
