#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#define DEBUG printf("Line : %d\n", __LINE__);

int pow2[32];

void initpow2() {
				int i;
				pow2[0] = 1;
				for(i=1; i<32; i++) pow2[i] = pow2[i-1] * 2;
}

void print_one_bin(unsigned char c) {
				int i;
				for(i=0; i<8; i++) printf("%d", (c / pow2[7 - i]) % 2);
				printf(" ");
}

void print_bin(char* line, int size) {
				int i;
				for(i=0; i<size; i++) print_one_bin(line[i]);
				printf("\n");
}

int opti_line(char* line, int pair, char* gpes) {
				/* La ligne fait 316 char de long.
				 * Si c'est une ligne paire, le premier char est normal,
				 * sinon, c'est juste 4 bits qu'il faut combiner avec
				 * le suivant.
				 */
				memset(gpes, 0, 12 * sizeof(char));
				char *capteurs = line + 7 + (pair == 0 ? 1 : 0);
				char first, second;
				char *raw32 = (char*) calloc(32, sizeof(char));
				/* Capteurs est au début du capteur.
				 * Dans le cas d'une ligne paire, on doit shift de 4 à chaque fois.
				 * Dans le cas contraire, il n'y a rien à faire
				 */
				DEBUG
								int i;
				for(i=0; i<16; i++) {
								first = capteurs[2*i];
								second = capteurs[2*i+1];
								printf("First : ");
								print_one_bin(first);
								printf("\n");
								printf("Second : ");
								print_one_bin(second);
								printf("\n");
								if(pair) {
												printf("PAIR\n");
												first <<= 4;
												printf("After decalage : ");
												print_one_bin(first);
												printf("\n");
												printf("To add : ");
												print_one_bin((second >> 4) & 15);
												printf("\n");
												first += ((second >> 4) & 15);
												second <<= 4;
												second += ((capteurs[2*i+2] >> 4) & 15);
								}
								printf("First : ");
								print_one_bin(first);
								printf("\n");
								printf("Second : ");
								print_one_bin(second);
								printf("\n");
								raw32[2*i] = first;
								raw32[2*i+1] = second;
				}
				DEBUG
								printf("raw32 : ");
				print_bin(raw32, 32);
				/* Ici, raw32 vaut les valeurs des capteurs. Par groupe de 2 chars,
				 * ils représentent les 16 bits correspondants aux 2 de qualité et
				 * aux 14 de valeur
				 * 
				 * Nous allons dans un premier temps regrouper les groupes de
				 * capteurs en fonction de leurs positions. En effet, dans le
				 * scintillateur, les capteurs sont regroupés comme suit :
				 * [1, 2, 3, 4], [5, 6, 7], [8, 9, 10], [11, 12, 13], [14, 15, 16].
				 */
				/* On a déclaré gpes de taille 12, car nous avons besoin de
				 * 16 bits par groupe, accompagnés de 2 bits pour la qualité
				 * du signal. Cette qualité sera calculée comme la plus mauvaise 
				 * de l'ensemble des capteurs.
				 * Cela nous fait donc 18*5 = 90 bits ~ 12 octets
				 */
				/* Pour chaque groupe de capteurs, il nous faut lire la qualité
				 * ainsi que la valeur. Si la qualité spécifie un dépassement ou
				 * une valeur inutilisable, tout le groupe est passé.
				 * Sinon, on garde la plus grande des deux qualités.
				 */
				char qualite_globale[5] = {0, 0, 0, 0, 0};
				char qualite = 0;
				int **n_capt = (int**) calloc(5, sizeof(int*));
				for(i=0; i<5; i++) n_capt[i] = (int*) calloc(2, sizeof(int));
				DEBUG
								n_capt[0][0] = 0;
				n_capt[0][1] = 4;
				n_capt[1][0] = 4;
				n_capt[1][1] = 7;
				n_capt[2][0] = 7;
				n_capt[2][1] = 10;
				n_capt[3][0] = 10;
				n_capt[3][1] = 13;
				n_capt[4][0] = 13;
				n_capt[4][1] = 15;
				DEBUG
								int j;
				for(i=0; i<5; i++) {
								for(j=n_capt[i][0]; j<n_capt[i][1]; j++) {
												qualite = raw32[j*2] >> 6;
												qualite = qualite % 4;
												qualite_globale[i] |= qualite;
								}
				}
				char first_qual = 
								((qualite_globale[0] & 3) << 6) +
								((qualite_globale[1] & 3) << 4) +
								((qualite_globale[2] & 3) << 2) +
								(qualite_globale[3] & 3);
				for(i=0; i<5; i++) printf("Qualité globale[%d] = %d\n", 
												i, qualite_globale[i]);
				print_one_bin(first_qual);
				printf("\n");
				int index = 0;
				char second_qual = qualite_globale[4] << 6;
				int intens = 0;
				for(i=0; i<5; i++) {
								printf("Qualité globale[%d] = %d\n", i, qualite_globale[i]);
								if(qualite_globale[i] == 1 || qualite_globale[i] == 2) {
												intens = 0;
												for(j=n_capt[i][0]; j<n_capt[i][1]; j++) {
																printf("intens : %d\n", intens);
																printf("raw32[2*%d] & 63 = ", j);
																print_one_bin((raw32[2*j] & 63));
																printf("\n");
																intens += (raw32[2*j] & 63)*256;
																printf("raw32[2*%d] = ", j+1);
																print_one_bin((raw32[2*j+1]));
																printf("\n");
																intens += raw32[2*j+1];
																intens += 256;
																printf("intens : %d\n", intens);
												}
												intens -= 256;
												printf("intens to write : %d : ", intens);
												print_one_bin(intens/256);
												print_one_bin(intens);
												printf("\n");
												/* Ecrire les 2 octets */
												second_qual += intens >> 10;
												/* On récupère que les 6 derniers bits qu'on
												 * ajoute */
												/* write first et second */
												gpes[index++] = first_qual;
												gpes[index++] = second_qual;
												printf("First Qual : ");
												print_one_bin(first_qual);
												printf("\nSecond Qual :");
												print_one_bin(second_qual);
												printf("\n");
												first_qual = (intens >> 2) % 256;
												second_qual = intens << 6;
								}
				}
				printf("First Qual : ");
				print_one_bin(first_qual);
				printf("\nSecond Qual :");
				print_one_bin(second_qual);
				printf("\n");
				gpes[index++] = first_qual;
				print_bin(gpes, 12);
				return (index + 1);
}

int main(int argc, char** argv) {
				int fd = 0, nb_octets;
				char *buffer = (char*) calloc(320, sizeof(char));
				char *gpes = (char*) calloc(12, sizeof(char));
				if(argc == 1) {
								printf("Use : %s [filename]\n", argv[0]);
								return EXIT_FAILURE;
				}
				fd = open(argv[1], O_RDONLY);
				if(fd <= 0) {
								printf("Couldn't open %s\n", argv[1]);
								return EXIT_FAILURE;
				}
				initpow2();
				if(read(fd, buffer, 320) != 320) {
								printf("Couldn't read correctly\n");
								close(fd);
								return EXIT_FAILURE;
				}
				nb_octets = opti_line(buffer, 1, gpes);
				printf("Original : \n");
				print_bin(buffer, 40);
				printf("\n");
				printf("Compressé : \n");
				print_bin(gpes, nb_octets);
				printf("\n");
				nb_octets = opti_line(buffer+39, 0, gpes);
				printf("Original : \n");
				print_bin(buffer+39, 40);
				printf("\n");
				printf("Compressé : \n");
				print_bin(gpes, nb_octets);
				printf("\n");
				return EXIT_SUCCESS;
}
