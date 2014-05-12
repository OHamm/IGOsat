#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
#include <sys/mman.h>

//TODO TEST
//00 01
int getNbrCapteur(int val){
	int nbrCapteur, i;
	int capt[5];
	nbrCapteur = 0;
	for(i=0;i<5;i++){
		capt[i] = val;
	}
	/* Bug sans les 0b000... */
	capt[0] = val&0b1100000000;
	capt[1] = val&0b0011000000;
	capt[2] = val&0b0000110000;
	capt[3] = val&0b0000001100;
	capt[4] = val&0b0000000011;
	for(i=0;i<5;i++){
		capt[4-i] = capt[4-i] >> i*2;
		if(capt[4-i] == 0 || capt[4-i] == 1){
			nbrCapteur++;
		}
		if(capt[4-i] == 0){
			printf("Capteur %d: Bande 0\n",4-i);
		}else if(capt[4-i] == 1){
			printf("Capteur %d: Bande 1\n",4-i);
		}else if(capt[4-i] == 2){
			printf("Capteur %d: -1 (pas de données)\n",4-i);
		}else if(capt[4-i] == 3){
			printf("Capteur %d: 2 (saturé)\n",4-i);
		}else{
			printf("Problème, données probablement corrompues!\n");
		}
	}
	return nbrCapteur;
}
//16 bits
void getCapteurVal(char *fd, int pos, int nbrCapteur){
	int i, j, nbrBuf, nbrBitRead, exitFlag, tmp, val;
	int *vals;
	nbrBuf = pos%8 != 0?3:2;
	nbrBuf *= nbrCapteur;
	nbrBitRead = 0;
	if((vals = (int*)calloc(nbrCapteur, sizeof(int))) == NULL){
		printf("ERR CALLOC\n");
		perror("");
		return;
	}
	val = 0;
	tmp = pos%8;
	exitFlag = -1;
	for(i=pos/8;i<(pos/8)+nbrBuf;i++){
		for(j=tmp;j<8;j++){
			if(exitFlag >= nbrCapteur){
				break;
			}
			if(nbrBitRead%16 == 0){
				nbrBitRead = 0;
				vals[exitFlag] = val;
				exitFlag++;
				val = 0;
			}
			if(fd[i]>>(7-j)&1){
				val *= 2;
				val += 1;
			}else{
				val *= 2;
			}
			nbrBitRead++;
		}
		if(exitFlag >= nbrCapteur){
			break;
		}
		tmp = 0;
	}
	//bits in vals need to be reversed beg -> end
	printf("Valeur capteurs:\n");
	for(i=0;i<nbrCapteur;i++){
		printf("%d ",vals[i]);
	}
	printf("\n");
	free(vals);
	return;
}
//10 bits
//TODO CHAIN TO A IDENTIFIER FOR BITS!
int getCapteurOnOff(char *fd, int pos){
	int i, j, nbrBuf, nbrBitRead, tmp, val;
	
	//Peut être sur 3 bytes ex: 0 12345678 9
	nbrBuf = pos%8 == 7?3:2;
	nbrBitRead = 0;
	val = 0;
	tmp = pos%8;
	for(i=pos/8;i<(pos/8)+nbrBuf;i++){
		for(j=tmp;j<8;j++){
			if(nbrBitRead >= 10){
				break;
			}
			if(fd[i]>>(7-j)&1){
				val *= 2;
				val += 1;
			}else{
				val *= 2;
			}
			nbrBitRead++;
		}
		if(nbrBitRead >= 10){
			break;
		}
		tmp = 0;
	}
	return getNbrCapteur(val);
}
long long int getDelta(char *fd, int pos, int length){
	int i, j, nbrBitRead, nbrBuf, tmp;
	long long int val;
	double tmpAcc;
	tmpAcc = (double)(length+pos%8)/8;
	nbrBuf = (int)tmpAcc;
	if(tmpAcc > (double)nbrBuf){
		nbrBuf++;
	}
	nbrBitRead = 0;
	val = 0;
	tmp = pos%8;
	for(i=pos/8;i<(pos/8)+nbrBuf;i++){
		for(j=tmp;j<8;j++){
			if(nbrBitRead >= length){
				break;
			}
			if(fd[i]>>(7-j)&1){
				val *= 2;
				val += 1;
			}else{
				val *= 2;
			}
			nbrBitRead++;
		}
		if(nbrBitRead >= length){
			break;
		}
		tmp = 0;
	}
	return val;
}
//pos = position of first bit in buffer
//nbrBuf = read 1 or 2 char
//Will read 6 bits
int getDeltaSize(char *fd, int pos){
	int i, j, val, tmp, nbrBuf, nbrBitRead;
	nbrBuf = pos%8 <= 2?1:2;
	tmp = pos%8;
	nbrBitRead = 0;
	val = 0;
	for(i=pos/8;i<(pos/8)+nbrBuf;i++){
		for(j=tmp;j<8;j++){
			if(nbrBitRead >= 6){
				break;
			}
			if(fd[i]>>(7-j)&1){
				val *= 2;
				val += 1;
			}else{
				val *= 2;
			}
			nbrBitRead++;
		}
		if(nbrBitRead >= 6){
			break;
		}
		tmp = 0;
	}
	return val;
}

int main(int argc, char **argv){
	int fd, counter, tmpDeltaSize, tmpCapteurOnOff, size;
	long long int old, next;
	char* map;
	if((fd = open(argv[1], O_RDONLY)) < 0){
		printf("ERR OPEN READ\n");
		return 1;
	}
	size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	
	if ((map = (char*)mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
		close(fd);
		perror("Error mmapping the file");
		exit(EXIT_FAILURE);
	}
	
	tmpDeltaSize = 0;
	old = 0;
	//counter = nbr bits read in file
	while((tmpDeltaSize = getDeltaSize(map, counter))>0){
		counter += 6;
		next = getDelta(map, counter, tmpDeltaSize);
		printf("Temps: %lld\n",old+next);
		old += next;
		counter += tmpDeltaSize;
		tmpCapteurOnOff = getCapteurOnOff(map, counter);
		counter += 10;
		if(tmpCapteurOnOff != 0){
			getCapteurVal(map,counter,tmpCapteurOnOff);
			counter += 16*tmpCapteurOnOff;
		}
		printf("__________________________________________\n");
	}
	printf("Terminé\n");
	close(fd);
}