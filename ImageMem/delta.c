#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>

long long int getTime(int fd, int first){//0 true, 1 false
	char *buf;
	long long int val;
	int i, j;
	/*Expl
	Type particule = 2bits
	Valeur enregistrée = 14 bits
	Temps = 60 bits
	
	First:
	12345678.(x6).1234XXXX
	Second:
	32 bytes d'écart (2+14 *16 bits), mais 4 bits de First = décalage de 4 bits
	<=>
	1234XXXX XXXXXXXX XXXXYYYY YYYYYYYY YYYY.(x13).WWWWZZZZ ZZZZ1234 5678...
	<=> La valeur suivante aura un décalage de 8 bits (1 byte)
	<=> Le schéma est identique à First

	Le saut des 31 bytes joue sur le modulo 1 byte via la variable first!
	*/
	if((buf = (char*)calloc(8+first, sizeof(char))) == NULL){
		printf("ERR CALLOC\n");
		return -1;
	}
	if(read(fd, buf, 8+first) < 0){
		printf("ERR READ\n");
		return -2;
	}

	//Convertir les bits en long long int (vérifier endian)
	val = 0;
	for(i=0;i<8;i++){
		for(j=0;j<8;j++){
			if(buf[i]>>(7-j)&1){
				val*=2;
				val += 1;
			}else{
				val*=2;
			}
		}
	}
	//Suppression de bits inutiles
	if(first == 1){
		val = val<<4;
	}
	val = val>>4;
	
	/* Detail explicatif de suppression:
	if(first == 0){
		//Supprimer les 4 derniers bits qui ne sont pas du temps
		val = val>>4;
	}else{
		//Supprimer les 4 premiers bits qui ne sont pas du temps
		val = val<<4;
		val = val>>4;
	}
	*/
	
	//Aller à la valeur suivante.
	lseek(fd, 31, SEEK_CUR);
	printf("Time: %lld",val);
	free(buf);
	return val;
}

//Gain moyen avec le fichier donné >= 30 bits, soit >= 50%
long long int deltacompression(long long int old, long long int next){
	return next - old;
}

void writedelta(long long int val, int fd){
	/* Expl:
	XXXXXX YYY...YYY 
	
	XXXXXX:
		Size of delta = 6bits <-> longueur de delta 1 -> 63 bits
		Or taille temps max = 60 bits.
	
	YYY...YYY:
		Delta, varie entre 0 et 60 bits.

	PROBLEME:
		Necessité d'un tampon pour écrire les bits.
		taille?
	*/
}
//argv[1] = name of file READ, argv[2] = name of file WRITTEN
int main(int argc, char **argv){
	int fdr, fdw, i; 
	long long int old, next;
	if((fdr = open(argv[1], O_RDONLY)) < 0){
		printf("ERR OPEN READ\n");
		return 1;
	}
	if((fdw = open(argv[2], O_RDONLY)) < 0){
		printf("ERR OPEN WRITE\n");
		return 1;
	}
	//Initialisation du delta
	old = getTime(fdr,0);
	printf(" Delta %lld\n",old);
	for(i=1;(next = getTime(fdr,i%2))>=0;i++){
		//Alterner First et Second
		printf(" Delta: %lld\n",deltacompression(old,next));
		old = next;
	}
	close(fd);
	return 0;
}