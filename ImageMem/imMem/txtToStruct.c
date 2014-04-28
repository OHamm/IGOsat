#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "txtToStruct.h"

long long int computeDate(char *line, int *index) {
	long long int retour = 0;
	for(; line[(*index)] != ' '; (*index)++) {
		retour *= 16;
		if(line[(*index)] >= '0' && line[(*index)] <= '9') retour += line[(*index)] - '0';
		else if(line[(*index)] >= 'a' && line[(*index)] <= 'f') retour += line[(*index)] - 'a' + 10;
	}
	return retour;
}

int getQuality(char *line, int *index) {
	if(line[(*index)] == '-') {
		*index += 2;
		return -1;
	}
	return line[(*index)++] - '0';
}

int getStrength(char *line, int *index) {
	return (int) (computeDate(line, index));
}

ligne* getLine(char *buffer, int *index) {
	int i;
	ligne *retour = (ligne*) malloc(sizeof(ligne));
	memset(retour, 0, sizeof(ligne));
	if(buffer[*index] == EOF) return NULL;
	retour->date = computeDate(buffer, index);
	(*index)++;
	for(i=0; i<16; i++) {
		retour->qual[i] = getQuality(buffer, index);
		(*index)++;
		retour->stre[i] = getStrength(buffer, index);
		(*index)++;
	}
	return retour;
}

void printLine(ligne *l) {
	int i;
	printf("Date : %lld\n", l->date);
	for(i=0; i<16; i++) {
		printf("Qual : %d\n", l->qual[i]);
		printf("Stre : %d\n", l->stre[i]);
	}
}
