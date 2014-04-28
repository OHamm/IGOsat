#define BEGIN 0
#define END 1

#include "txtToStruct.h"

typedef struct {
	int position;
	char content[40]; // 40 * 8 bits = 320 ~ 316
} bin_line;

void print_long_long_int_bin(long long int);
void fillStruct(ligne*, bin_line*);
void print_chars(char*, int);
char combine_two(char, char, int);
