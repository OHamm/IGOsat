#include <stdio.h>
#include <stdlib.h>
#include "structToBin.h"

/*
typedef struct {
	long long int date;
	int qual[16];
	int stre[16];
} ligne;

long long int getDate(char *line, int *index);
int getQuality(char *line, int *index);
int getStrength(char *line, int *index);
ligne* getLine(char *buffer, int *index);
void printLine(ligne *l);
 */

void print_long_long_int_bin(long long int n) {
	int i;
	for(i=0; i<64; i++) 
		printf("%d", (int) (n >> (63 - i)) & 1);
}

void print_char(char c) {
	int i;
	for(i=0; i<8; i++) printf("%d", (c >> (7-i)) & 1);
}

void print_chars(char* str, int l) {
	int i;
	for(i=0; i<l; i++) print_char(str[l-1-i]);
}

/*
char combine_two(char a, char b, int n) {
	char retour = 0;
	char first_part;
	char second_part;
	first_part = (a << (8 - n));
	b >>= n;
	switch(n) {
		case(0) : second_part = 0b11111111 & b; break;
		case(1) : second_part = 0b01111111 & b; break;
		case(2) : second_part = 0b00111111 & b; break;
		case(3) : second_part = 0b00011111 & b; break;
		case(4) : second_part = 0b00001111 & b; break;
		case(5) : second_part = 0b00000111 & b; break;
		case(6) : second_part = 0b00000011 & b; break;
		case(7) : second_part = 0b00000001 & b; break;
		case(8) : second_part = 0b00000000 & b; break;
		default: printf("GTFO\n"); break;
	}
	retour |= first_part;
	retour |= second_part;
	return retour;
}
*/

char combine_two(char first, char second, int how_many) {
	switch(how_many) {
		case(8) : return ((0b00000000 & second) + (0b11111111 & first));
		case(7) : return ((0b00000001 & second) + (0b11111110 & first));
		case(6) : return ((0b00000011 & second) + (0b11111100 & first));
		case(5) : return ((0b00000111 & second) + (0b11111000 & first));
		case(4) : return ((0b00001111 & second) + (0b11110000 & first));
		case(3) : return ((0b00011111 & second) + (0b11100000 & first));
		case(2) : return ((0b00111111 & second) + (0b11000000 & first));
		case(1) : return ((0b01111111 & second) + (0b10000000 & first));
		case(0) : return ((0b11111111 & second) + (0b00000000 & first));
		default : return 0;
	}
}

void fillStruct(ligne *l, bin_line *res) {
	int decalage;
	int index = 0;
	long long int tmp;
	int i;
	char first_octet, second_octet, quality_octet;
	if(res->position == END) decalage = 0;
	else decalage = 4;
	tmp = l->date << 4;
	tmp >>= decalage;
	for(index=0; index<8; index++) 
		res->content[index] = ((char*) (&tmp))[7-index];
	if(res->position == END) index--;
	for(i=0; i<16; i++) {
		quality_octet = ((char*) (&(l->qual[i])))[0];
		first_octet = ((char*) (&(l->stre[i])))[1];
		second_octet = ((char*) (&(l->stre[i])))[0];
		if(res->position == END) {
			res->content[index] = combine_two(
				res->content[index],
				quality_octet << 2,
				4);
			res->content[index] = combine_two(
				res->content[index],
				first_octet >> 4,
				6);
			index++;
			res->content[index] = combine_two(
				first_octet << 4,
				second_octet >> 4,
				4);
			index++;
			res->content[index] = second_octet << 4;
		}
		if(res->position == BEGIN) {
			res->content[index] = combine_two(
				quality_octet << 6,
				first_octet,
				2);
			index++;
			res->content[index] = second_octet;
			index++;
		}
	}
}
