typedef struct {
	long long int date;
	int qual[16];
	int stre[16];
} ligne;

long long int getDate(char *line, int *index);
int getQuality(char *line, int *index);
int getStrength(char *line, int *index);
ligne* getLine(char *buffer, int *index);
void printLine(ligne *l);
