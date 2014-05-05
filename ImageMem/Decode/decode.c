#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <math.h>
//16 bits
//TODO TEST
int getNbrCapteur(int val){
	return -1;
}
void getCapteurVal(int fd, int pos, int nbrCapteur){
	char *buf;
	int i, j, nbrBuf, nbrBitRead, exitFlag, tmp, val;
	int *vals;
	
	nbrBuf = pos!=0?5:4;
	nbrBitRead = 0;
	
	if((buf = (char*)calloc(2*nbrBuf, sizeof(char))) == NULL){
		printf("ERR CALLOC\n");
		perror("");
		return;
	}
	if((vals = (int*)calloc(nbrBuf, sizeof(int))) == NULL){
		printf("ERR CALLOC\n");
		perror("");
		return;
	}
	if(read(fd, buf, 2*nbrCapteur) < 0){
		printf("ERR READ\n");
		perror("");
		return;
	}
	val = 0;
	tmp = pos;
	exitFlag = 0;
	for(i=0;i<nbrBuf;i++){
		for(j=tmp;j<8;j++){
			nbrBitRead++;
			if(exitFlag >= nbrCapteur){
				break;
			}
			if(nbrBitRead%16 == 0){
				nbrBitRead = 0;
				vals[exitFlag] = val;
				exitFlag++;
				val = 0;
			}
			if(buf[i]>>(7-j)&1){
				val *= 2;
				val += 1;
			}else{
				val *= 2;
			}
		}
		if(exitFlag >= nbrCapteur){
			break;
		}
		tmp = 0;
	}
	//TODO CHECK
	if(pos != 0){
		lseek(fd, -1, SEEK_CUR);
	}
	for(i=0;i<nbrCapteur;i++){
		printf("%d\n",vals[i]);
	}
	free(vals);
	free(buf);
	return;
}
//10 bits
//TODO CHAIN TO A IDENTIFIER FOR BITS!
int getCapteurOnOff(int fd, int pos){
	char *buf;
	int i, j, nbrBuf, nbrBitRead, tmp, val;
	
	//Peut être sur 3 bytes ex: 0 12345678 9
	nbrBuf = pos==7?3:2;
	nbrBitRead = 0;
	
	if((buf = (char*)calloc(nbrBuf, sizeof(char))) == NULL){
		printf("ERR CALLOC\n");
		perror("");
		return -1;
	}
	if(read(fd, buf, nbrBuf) < 0){
		printf("ERR READ\n");
		perror("");
		return -2;
	}
	val = 0;
	tmp = pos;
	for(i=0;i<nbrBuf;i++){
		for(j=tmp;j<8;j++){
			if(nbrBitRead > 10){
				break;
			}
			if(buf[i]>>(7-j)&1){
				val *= 2;
				val += 1;
			}else{
				val *= 2;
			}
			nbrBitRead++;
		}
		if(nbrBitRead > 10){
			break;
		}
		tmp = 0;
	}
	
	//TODO CHECK
	if((pos+10)%8 != 0){
		lseek(fd, -1, SEEK_CUR);
	}
	free(buf);
	return getNbrCapteur(val);
}
long long int getDelta(int fd, int pos, int length){
	char *buf;
	int i, j, nbrBitRead, nbrBuf, tmp;
	long long int val;
	nbrBuf = 1 + (length/8);
	nbrBitRead = 0;
	if((buf = (char*)calloc(nbrBuf, sizeof(char))) == NULL){
		printf("ERR CALLOC\n");
		perror("");
		return -1;
	}
	if(read(fd, buf, nbrBuf) < 0){
		printf("ERR READ\n");
		perror("");
		return -2;
	}
	val = 0;
	tmp = pos;
	for(i=0;i<nbrBuf;i++){
		for(j=tmp;j<8;j++){
			if(nbrBitRead > length){
				break;
			}
			if(buf[i]>>(7-j)&1){
				val *= 2;
				val += 1;
			}else{
				val *= 2;
			}
			nbrBitRead++;
		}
		if(nbrBitRead > length){
			break;
		}
		tmp = 0;
	}
	if((pos+length)%8 != 0){
		#ifdef DEBUG
			printf("OHYEAH %d %d\n", pos, length);
		#endif
		lseek(fd, -1, SEEK_CUR);
	}
	free(buf);
	return val;
}
//pos = position of first bit in buffer
//nbrBuf = read 1 or 2 char
//Will read 6 bits
int getDeltaSize(int fd, int pos){
	char *buf;
	int i, j, val, tmp, nbrBuf;
	nbrBuf = pos==0?1:2;
	tmp = pos==0?6:8;
	if((buf = (char*)calloc(nbrBuf, sizeof(char))) == NULL){
		printf("ERR CALLOC\n");
		perror("");
		return -1;
	}
	if(read(fd, buf, nbrBuf) < 0){
		printf("ERR READ\n");
		perror("");
		return -2;
	}
	val = 0;
	for(i=0;i<nbrBuf;i++){
		for(j=pos;j<tmp;j++){
			if(buf[i]>>(7-j)&1){
				val *= 2;
				val += 1;
			}else{
				val *= 2;
			}
		}
		tmp -= pos;
		pos = 0;
	}
	//Cas ou taille du delta ne termine pas le buffer
	//ie: bit de delta à la fin du buffer de taille.
	if(pos!=2){
		lseek(fd, -1, SEEK_CUR);
	}
	free(buf);
	return val;
}

int main(int argc, char **argv){
	int fd, counter, tmpDeltaSize, tmpCapteurOnOff;
	if((fd = open(argv[1], O_RDONLY)) < 0){
		printf("ERR OPEN READ\n");
		return 1;
	}
	//counter = nbr bits read in file
	for(counter=0; (tmpDeltaSize = getDeltaSize(fd, 0))>0; counter+=tmpDeltaSize){
		printf("Val %d\n",tmpDeltaSize);
		printf("Delta %lld\n",getDelta(fd, 6, tmpDeltaSize));
		//??????????????????????
		tmpCapteurOnOff = getCapteurOnOff(fd, counter%8);
		//??????????????????????
		
		counter+= tmpCapteurOnOff;
		
		//??????????????????????
		getCapteurVal(fd,counter%8,tmpCapteurOnOff);
		//??????????????????????
	}
	close(fd);
}