#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>

long long int getTime(int fd, int first){//0 true, 1 false
	char *buf;
	long long int val;
	int i, j, tabpos = 0;
	int *tab;
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
	if((buf = (char*)calloc(40, sizeof(char))) == NULL){
		printf("ERR CALLOC\n");
		return -1;
	}
	if((tab = (int*)calloc(17, sizeof(int))) == NULL){
		printf("ERR CALLOC\n");
		return -1;
	}
	if(read(fd, buf, 40) < 0){
		printf("ERR READ\n");
		return -2;
	}

	//Convertir les bits en long long int (vérifier endian)
	//Pour temps
	val = 0;
	for(i=0;i<8;i++){
		for(j=0;j<8;j++){
			if(buf[i]>>(7-j)&1){
				val *= 2;
				val += 1;
			}else{
				val *= 2;
			}
		}
	}
	//Suppression de bits inutiles
	if(first == 1){
		val = val<<4;
	}
	val = val>>4;
	//Pour capteurs
	
	//IF XXXXYYYY YYYYYYYY YYYYZZZZ
	//Lire 4 dernier bits, 8 bits, 4 premier bits
	//IF YYYYYYYY YYYYYYYY
	//Lire 8x2 bits
	//no shift
	if(first == 1){// YYYYYYYY YYYYYYYY
		for(i=8;i<40;i=i+2){
			for(j=0;j<8;j++){
				if(buf[i]>>(7-j)&1){
					tab[tabpos] *= 2;
					tab[tabpos] += 1;
				}else{
					tab[tabpos] *= 2;
				}
			}
			for(j=0;j<8;j++){
				if(buf[i+1]>>(7-j)&1){
					tab[tabpos] *= 2;
					tab[tabpos] += 1;
				}else{
					tab[tabpos] *= 2;
				}
			}
			tabpos++;
		}
	}
	else{// XXXXYYYY YYYYYYYY YYYYZZZZ
		for(i=7;i<40;i=i+2){
			for(j=4;j<8;j++){
				if(buf[i]>>(7-j)&1){
					tab[tabpos] *= 2;
					tab[tabpos] += 1;
				}else{
					tab[tabpos] *= 2;
				}
			}
			for(j=0;j<8;j++){
				if(buf[i+1]>>(7-j)&1){
					tab[tabpos] *= 2;
					tab[tabpos] += 1;
				}else{
					tab[tabpos] *= 2;
				}
			}
			for(j=0;j<4;j++){
				if(buf[i+2]>>(7-j)&1){
					tab[tabpos] *= 2;
					tab[tabpos] += 1;
				}else{
					tab[tabpos] *= 2;
				}
			}
			tabpos++;
		}
	}
	for(i=0;i<16;i++){
		printf(" TABVAL: %d",tab[i]);
	}
	printf("\n");
	free(tab);
	
	//Aller à la valeur suivante.
	//-1 car modulo
	if(first == 0){
		lseek(fd, -1, SEEK_CUR);
	}
	printf("Time: %lld",val);
	free(buf);
	return val;
}

//Gain moyen avec le fichier donné >= 30 bits, soit >= 50%
long long int deltacompression(long long int old, long long int next){
	return next - old;
}
//nombres de bits dans un long long int
int getSize(long long int val){
	int i, ret;
	ret = 0;
	for(i=0;i<64;i++){
		if((val>>i)&1){
			ret = i;
		}
	}
	//+1 car décallage avec boucle for i
	return ret+1;
}
//nombres de bits dans unlong long int encodé en binaire sur 6 bits (2 premiers du char à ignorer)
char getLength(long long int val){
	char ret = getSize(val);
	return ret;
}
void writedelta(long long int val, int fd){
	/* Write:
	Write last 6 bits of getLength()
	Write last getSize() bits of deltacompression()
	*/
	/* Expl:
	XXXXXX YYY...YYY 
	
	XXXXXX:
		Size of delta = 6bits <-> longueur de delta 1 -> 63 bits
		Or taille temps max = 60 bits.
	
	YYY...YYY:
		Delta, varie entre 0 et 60 bits.

	PROBLEME:
		Necessité d'un tampon pour écrire les bits.
		taille variable, si multiple de 8bits écrit alors écrire
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
	
	if((fdw = open(argv[2], O_RDONLY | O_CREAT, 00333)) < 0){
		printf("ERR OPEN WRITE\n");
		return 1;
	}
	//Initialisation du delta
	old = getTime(fdr,0);
	printf(" Delta %lld\n",old);
	for(i=1;(next = getTime(fdr,i%2))>=0;i++){
		//Alterner First et Second
		printf(" Length: %d",getSize(deltacompression(old,next)));
		printf(" Delta: %lld\n",deltacompression(old,next));
		old = next;
	}
	close(fdr);
	close(fdw);
	return 0;
}
