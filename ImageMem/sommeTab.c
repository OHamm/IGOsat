#include <stdio.h>
#include <stdlib.h>

#define GETQUAL(c) ((c) & 0x0000C000)
#define GETVAL(c) ((c) & 0x00003FFF)

int somme_capteurs(int *capteurs, int *retour) {
				free(retour);
				/*
				 * Capteurs contient les 16 capteurs sous la forme :
				 * XXYYYYYY YYYYYYYY
				 * XX = qualite
				 * YYYYYY YYYYYYYY = intensité
				 */
				printf("Calloc in progress\n");
				retour = (int*) calloc(5, sizeof(int));
				printf("Calloc done\n");

				char tmp = GETVAL(capteurs[0]);

				retour[0] &= ((char)(GETQUAL(capteurs[0])) << 2);
				retour[0] += ((char)(GETVAL(capteurs[0])));
				printf("retour[0] done\n");
				retour[0] &= (GETQUAL(capteurs[1]) << 2);
				retour[0] += (GETVAL(capteurs[1]));
				retour[0] &= (GETQUAL(capteurs[2]) << 2);
				retour[0] += (GETVAL(capteurs[2]));
				retour[0] &= (GETQUAL(capteurs[3]) << 2);
				retour[0] += (GETVAL(capteurs[3]));

				printf("First group done\n");

				retour[1] &= (GETQUAL(capteurs[4]) << 2);
				retour[1] += (GETVAL(capteurs[4]));
				retour[1] &= (GETQUAL(capteurs[5]) << 2);
				retour[1] += (GETVAL(capteurs[5]));
				retour[1] &= (GETQUAL(capteurs[6]) << 2);
				retour[1] += (GETVAL(capteurs[6]));

				retour[2] &= (GETQUAL(capteurs[7]) << 2);
				retour[2] += (GETVAL(capteurs[7]));
				retour[2] &= (GETQUAL(capteurs[8]) << 2);
				retour[2] += (GETVAL(capteurs[8]));
				retour[2] &= (GETQUAL(capteurs[9]) << 2);
				retour[2] += (GETVAL(capteurs[9]));

				retour[3] &= (GETQUAL(capteurs[10]) << 2);
				retour[3] += (GETVAL(capteurs[10]));
				retour[3] &= (GETQUAL(capteurs[11]) << 2);
				retour[3] += (GETVAL(capteurs[11]));
				retour[3] &= (GETQUAL(capteurs[12]) << 2);
				retour[3] += (GETVAL(capteurs[12]));

				retour[4] &= (GETQUAL(capteurs[13]) << 2);
				retour[4] += (GETVAL(capteurs[13]));
				retour[4] &= (GETQUAL(capteurs[14]) << 2);
				retour[4] += (GETVAL(capteurs[14]));
				retour[4] &= (GETQUAL(capteurs[15]) << 2);
				retour[4] += (GETVAL(capteurs[15]));

				return EXIT_SUCCESS;
}
