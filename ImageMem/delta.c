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
	//Aller à la valeur suivante.
	if(first == 0){
		//Supprimer les 4 derniers bits qui ne sont pas du temps
		val = val>>4;
	}else{
		//Supprimer les 4 premiers bits qui ne sont pas du temps
		val = val<<4;
		val = val>>4;
	}
	lseek(fd, 31, SEEK_CUR);
	printf("Time: %lld",val);
	free(buf);
	return val;
}

//Gain moyen avec le fichier donné >= 30 bits, soit >= 50%
long long int deltacompression(long long int old, long long int next){
	return next - old;
}
//argv[1] = name of file
int main(int argc, char **argv){
	int fd,i;
	long long int old, next;
	if((fd = open(argv[1], O_RDONLY)) < 0){
		printf("ERR OPEN\n");
		return 1;
	}
	//Initialisation du delta
	old = getTime(fd,0);
	printf(" Delta %lld\n",old);
	//Boucle temporaire limitée à 40 valeurs due à un bug dans le fichier.
	//Celle-ci sera modifiée en une boucle infinie qui s'arretera
	//lorsque la lecture du fichier sera terminée.
	for(i=1;i<40;i++){
		//Alterner First et Second
		next = getTime(fd,i%2);
		printf(" Delta: %lld\n",deltacompression(old,next));
		old = next;
	}
	close(fd);
	return 0;
}