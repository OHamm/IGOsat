#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <sys/mman.h>

typedef struct buffer{
	int bitPosFd;
	char* fd;
	int fdFile;
} buffer;
/** GETTING VALS FROM BINARY FILE **/
long long int getVals(char *map, int *pos, int first, int* tab, int fileSize){//0 true, 1 false
	long long int val;
	int i, j, tabpos = 0;
	if(fileSize == pos[0]){
		return -1;
	}
	/* Expl
	* Type particule = 2bits
	* 	 Valeur enregistrée = 14 bits
	* 	 Temps = 60 bits
	* First:
	* 12345678.(x6).1234XXXX
	* Second:
	* 32 bytes d'écart (2+14 *16 bits),
	* mais 4 bits de First = décalage de 4 bits
	* <=>
	* 1234XXXX XXXXXXXX XXXXYYYY YYYYYYYY
	* YYYY.(x13).WWWWZZZZ ZZZZ1234 5678...
	* <=> La valeur suivante aura un décalage de 8 bits (1 byte)
	* <=> Le schéma est identique à First
	*/
	for(i=0;i<5;i++){
		tab[i] = 0;
	}
	//Convertir les bits en long long int (vérifier endian)
	//Pour temps
	val = 0;
	for(i=0+pos[0];i<8+pos[0];i++){
		for(j=0;j<8;j++){
			if(map[i]>>(7-j)&1){
				val *= 2;
				val += 1;
			}else{
				val *= 2;
			}
		}
	}
	//Suppression de bits inutiles
	if(first == 1){
		val *= 16;
	}
	val /= 16;
	//Pour capteurs
	
	//IF XXXXYYYY YYYYYYYY YYYYZZZZ
	//Lire 4 dernier bits, 8 bits, 4 premier bits
	//IF YYYYYYYY YYYYYYYY
	//Lire 8x2 bits
	//no shift
	if(first == 1){// YYYYYYYY YYYYYYYY
			for(i=8+pos[0];i<40+pos[0];i=i+2){
				for(j=0;j<8;j++){
					if(map[i]>>(7-j)&1){
						tab[tabpos] *= 2;
						tab[tabpos] += 1;
					}else{
						tab[tabpos] *= 2;
					}
				}
				for(j=0;j<8;j++){
					if(map[i+1]>>(7-j)&1){
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
			for(i=7+pos[0];i<40+pos[0];i=i+2){
				for(j=4;j<8;j++){
					if(map[i]>>(7-j)&1){
						tab[tabpos] *= 2;
						tab[tabpos] += 1;
					} else {
						tab[tabpos] *= 2;
					}
				}
				for(j = 0; j < 8; j++) {
					if(map[i+1] >> (7-j) & 1) {
						tab[tabpos] *= 2;
						tab[tabpos] += 1;
					} else {
						tab[tabpos] *= 2;
					}
				}
				for(j=0;j<4;j++){
					if(map[i+2] >> (7-j) & 1) {
						tab[tabpos] *= 2;
						tab[tabpos] += 1;
					} else {
						tab[tabpos] *= 2;
					}
				}
				tabpos++;
			}
	}
	if(first == 0){
		pos[0] += 39;
	}else{
		pos[0] += 40;
	}
	return val;
}
/** END GETTING VALS FROM BINARY FILE **/
/** DELTA STUFF **/
long long int deltacompression(long long int old, long long int next){
	return next - old;
}
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
char getLength(long long int val){
	char ret = getSize(val);
	return ret;
}
/** END DELTA STUFF **/
/** WRITING STUFF **/
buffer *initBufferStruct(int fd, int size){
	buffer *buf;
	
	if((buf = (buffer*)calloc(1, sizeof(buffer))) == NULL){
		printf("ERR CALLOC\n");
		return NULL;
	}
	if ((buf->fd  = (char*)mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)) == MAP_FAILED) {
		printf("ERR MMAP\n");
		perror("");
		close(fd);
		return NULL;
	}
	buf->fdFile = fd;
	buf->bitPosFd = 0;
	return buf;
}
void freeAll(buffer *buf){
	int size;
	double totSize;
	totSize = buf->bitPosFd/8;
	
	if((double)(buf->bitPosFd/8) > totSize){
		size = ((int)totSize)+1;
	}else{
		size = ((int)totSize);
	}
	if(write(buf->fdFile,buf->fd,size)<0){
		perror("ERR WRITE\n");
	}
	//munmap(buf->fd, size);
}
void writeInFile(buffer *buf, int val){
	buf->fd[(int)buf->bitPosFd/8] *= 2;
	buf->fd[(int)buf->bitPosFd/8] += val;
	buf->bitPosFd++;
}
//16bits
void addCapteurVal(buffer *buf, int val){
	int i;
	for(i=0;i<16;i++){
		if(val>>(15-i)&1){
			writeInFile(buf,1);
		}else{
			writeInFile(buf,0);
		}
	}
}
//10bits
void addCapteurInd(buffer *buf, int val){
	int i;
	//printf("Val: %d\n");
	for(i=0;i<10;i++){
		if(val>>(9-i)&1){
			writeInFile(buf,1);
		}else{
			writeInFile(buf,0);
		}
	}
}
//Delta
void addDelta(buffer *buf, long long int val){
	int i,length;
	length = getSize(val);
	//Write length
	for(i=0; i<6; i++){
		if(length>>(5-i)&1){
			writeInFile(buf,1);
		}else{
			writeInFile(buf,0);
		}
	}
	//Write delta
	for(i=0; i<length; i++){
		if(val>>((length-1)-i)&1){
			writeInFile(buf,1);
		}else{
			writeInFile(buf,0);
		}
	}
}
/** END WRITING STUFF **/
/** CAPTEUR STUFF **/
void addLoop(int beg, int end, int *tab, int *captVals, int *indVals, int pos){
	/* NOTE: bug sur mon pc, d'ou le 0b000.... */
	int tmpIndVals = -1, tmp;
	for(;beg<end;beg++){
		tmp = tab[beg]&0b00000000000000001100000000000000;
		if(tmp>>14 == 0b00){
			if(tmpIndVals == -1 || tmpIndVals == 3){
				tmpIndVals = 0;
			}
			captVals[pos] += tab[beg]&0b00000000000000000011111111111111;
		}
		if(tmp>>14 == 0b01){
			if(tmpIndVals == -1 || tmpIndVals == 3){
				tmpIndVals = 1;
			}
			captVals[pos] += tab[beg]&0b00000000000000000011111111111111;
		}
		if(tmp>>14 == 0b10){
			tmpIndVals = 2;
			break;
		}
		if(tmp>>14 == 0b11){
			if(tmpIndVals == -1){
				tmpIndVals = 3;
			}
		}
	}
	if(tmpIndVals == 3 || tmpIndVals == 2){
		captVals[pos] = 0;
	}
	#ifdef DEBUG
	if(tmpIndVals == 0){
		printf("Capteur %d: Bande 0\n",pos);
	}else if(tmpIndVals == 1){
		printf("Capteur %d: Bande 1\n",pos);
	}else if(tmpIndVals == 2){
		printf("Capteur %d: -1 (pas de données)\n",pos);
	}else if(tmpIndVals == 3){
		printf("Capteur %d: 2 (saturé)\n",pos);
	}else{
		printf("Problème, données probablement corrompues!\n");
	}
	#endif
	indVals[pos] = tmpIndVals;
	return;
}
void addCapteur(buffer *buf, int *tab){
	int *captVals, *indVals;
	int tmpInd, i;
	if((captVals = (int*)calloc(5, sizeof(int))) == NULL){
		printf("ERR CALLOC\n");
		perror("");
		return;
	}
	if((indVals = (int*)calloc(5, sizeof(int))) == NULL){
		printf("ERR CALLOC\n");
		perror("");
		return;
	}
	/* Groupes de Capteurs */
#ifdef DEBUG
	
#endif
	addLoop(0, 4, tab, captVals, indVals, 0);
	addLoop(4, 7, tab, captVals, indVals, 1);
	addLoop(7, 10, tab, captVals, indVals, 2);
	addLoop(10, 13, tab, captVals, indVals, 3);
	addLoop(13, 16, tab, captVals, indVals, 4);
	
	tmpInd = 0;
	for(i=0;i<5;i++){
		tmpInd <<= 2;
		//00 01 10 11
		//val 00 01
		//0 1
		if(indVals[i] == 0){
			tmpInd += 0b00;
		}else if(indVals[i] == 1){
			tmpInd += 0b01;
		}else if(indVals[i] == 2){
			tmpInd += 0b10;
		}else if(indVals[i] == 3){
			tmpInd += 0b11;
		}else{
			printf("ERR");
		}
	}
	addCapteurInd(buf, tmpInd);
#ifdef DEBUG
	printf("Valeur capteurs:\n");
#endif
	for(i=0;i<5;i++){
		if(captVals[i] != 0){
#ifdef DEBUG
			printf("%d \n",captVals[i]);
#endif
			addCapteurVal(buf,captVals[i]);
		}
	}
	free(captVals);
	free(indVals);
}
/** END CAPTEUR STUFF **/
//argv[1] = name of file READ, argv[2] = name of file WRITTEN
int main(int argc, char **argv){
	int fdr, fdw, i;
	long long int old, next;
	int* capteurs = NULL;
	char *map;
	int fileSize;
	int *pos;
	buffer *buf;
	if((fdr = open(argv[1], O_RDONLY)) < 0){
		printf("ERR OPEN READ\n");
		return 1;
	}
	if((fdw = open(argv[2], O_RDWR|O_CREAT|O_TRUNC)) < 0){
		printf("ERR OPEN WRITE\n");
		return 1;
	}
	if((capteurs = (int*)calloc(16, sizeof(int))) == NULL){
		printf("ERR CALLOC\n");
		perror("");
		return -1;
	}
	if((pos = (int*)calloc(1, sizeof(int))) == NULL){
		printf("ERR CALLOC\n");
		perror("");
		return -1;
	}
	pos[0] = 0;
	fileSize = lseek(fdr, 0, SEEK_END);
	lseek(fdr, 0, SEEK_SET);
	if ((map  = (char*)mmap(0, fileSize, PROT_READ, MAP_PRIVATE, fdr, 0)) == MAP_FAILED) {
		printf("ERR MMAP\n");
		perror("");
		return EXIT_FAILURE;
	}
#ifdef DEBUG
	printf("Start:\n");
#endif
	buf = initBufferStruct(fdw,fileSize);
	//End init structure
	
	//Initialisation du delta
	old = getVals(map, pos, 0, capteurs, fileSize);
	addDelta(buf, old);
#ifdef DEBUG
	printf("Temps: %lld\n",old);
#endif
	addCapteur(buf, capteurs);
#ifdef DEBUG
	printf("__________________________________________\n");
#endif
	//End init
	for(i=1;(next = getVals(map, pos, i%2, capteurs, fileSize))>0;i++){
		//Alterner First et Second
		addDelta(buf, deltacompression(old,next));
#ifdef DEBUG
		printf("Temps: %lld\n",next);
#endif
		old = next;
		addCapteur(buf, capteurs);
#ifdef DEBUG
		printf("__________________________________________\n");
#endif
	}
	freeAll(buf);
	close(fdr);
	close(fdw);
	if(capteurs != NULL){
		free(capteurs);
	}
	return 0;
}