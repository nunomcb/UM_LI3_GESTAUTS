#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "reading.h"

#define NR_OPTIONS		16
#define NR_FUNCTIONS	15
#define NR_ERRORS		2
#define GREET()			( printf("WELCOME TO GESTAUTS.\n") )
#define BID_FAREWELL()	( printf("BYE BYE.\n") )
#define valid_input(i)  ( ( (i) > -1 ) && ( (i) < NR_FUNCTIONS ) )
#define TIME(f) \
	begin = clock(); \
	f				 \
	end = clock();   \

#define PRINT_TIME_SPENT(msg) printf("\n\nTIME SPENT " msg ": %f\n\n", (double)(end - begin) / CLOCKS_PER_SEC);

#define TOTAL_DIGITS(size, n)   \
	size = 0;					\
	do {						\
		size ++;				\
	} while(n /= 10);			\

static int inGestAuts = 1;
static int populated_db = 0;

static char* options[NR_OPTIONS] = {
    "ENTER ONE OF THE FOLLOWING OPTIONS",
    "EXIT",
	"READ FROM FILE",
	"TOTAL PUBLICATIONS BY YEAR",
	"GET AUTHOR PUBLICATIONS IN YEAR",
	"GET TOTAL SOLO AUTHORS",
	"GET AUTHOR PUBLICATIONS BY YEAR",
	"GET AUTHORS BY INITIAL",
	"TOTAL PUBLICATIONS IN INTERVAL",
	"TO BE IMPLEMENTED",
	"TO BE IMPLEMENTED",
	"GET YEAR AUTHOR STATS",
	"GET CSV FILE",
	"TOP AUTHORS IN YEAR",
	"AUTHOR YEAR RATIO",
	"PRINT NAME STATISTICS"
};
/* Functions to be replaced with queries */
static void failureprnt() { printf("NON EXISTING FUNCTION\n\n"); }
static void invalid_option_err() { printf("INVALID OPTION\n\n"); }
static void no_file_read_err() { printf("NO FILE WAS READ YET. PLEASE READ FROM A FILE\n\n"); }

static void exitGestAuts() {
	if (populated_db) leaveGestauts();
	inGestAuts = 0;
}

static void query1() {
	char* stats;
#ifdef DEBUG
	read_file("publicx.txt");
#else
	#ifdef DEBUG2
	read_file("publicx4.txt");
	#else
	char filename[1024];
	int valid_input = 0;

	if (populated_db) {
		leaveGestauts();
		initializeGestauts();
	}

	while ( !valid_input ) {
		printf("ENTER A FILENAME:\n");
		scanf("%s", filename);

		if ( read_file(filename) == -1 )
			printf("ERROR: FILE DOESN'T EXIST\n\n");
		else
			valid_input = 1;
	}

	#endif
#endif
	stats = getReadStats();
	printf( "%s\n\n", stats );
	free(stats);

	populated_db = 1;
}

static void query2() {
	int *totals;
	int min, max, i;

	totals = getYearsTotal(&min, &max);

	printf(" _______ _______ \t _______ _______\n");
	printf("|\t|\t|\t|\t|\t|\n");
	printf("|Year\t|Total\t|\t|Year\t|Total\t|\n");
	printf("|_______|_______|\t|_______|_______|\n");
	for (i = min; i <= max; i += 2) {
		printf("|\t|\t|\t|\t|\t|\n");
		if (i < max)
			printf("|%d\t|%d\t|\t|%d\t|%d\t|\n", i, totals[i - min], i + 1, totals[i + 1 - min]);
		else
			printf("|%d\t|%d\t|\t|\t|\t|\n", i, totals[i - min]);
		printf("|_______|_______|\t|_______|_______|\n");
	}

	printf("\n\n");

	free(totals);
}

static void query3() {
	char author[128];
	int year, total;

	printf("INSERT AUTHOR NAME:\n");
	getchar();
	scanf("%[^\n]", author);

	printf("INSERT YEAR:\n");
	scanf("%d", &year);

	total = authorPublicationsInYear(author, year);

	if (total == -1) {
		printf("SERIOUSLY? ARE YOU MAD OR SOMETHING? FUCKING FUCKTARD!\n");
		return;
	}

	printf("\n\n%d\n\n", total);

	return;
}

static void query4() {
	int total = totalSoloAuthors();
	printf("TOTAL NUMBER OF SOLO AUTHORS: %d\n", total);
}

static void query5() {
	int size, i;
	int** matrix;
	char buffer[1024];
	char* author_name;

	printf("ENTER A NAME: ");
	/*fgets(buffer, sizeof(char) * 1024, stdin);*/
	scanf("\n%1024[^\n]s]", buffer);
	author_name = strtrim(buffer);

	matrix = getAuthorPublicationsByYear(author_name, &size);

	if (!size)
		printf("AUTHOR NOT FOUND\n\n");

	else {
		putchar('\n');
		for(i = 0; i < size; i++) {
			printf(" ______________\n");
			printf("|  %d  |  %d  |\n", matrix[i][0], matrix[i][1]);
		}
		printf(" ______________\n\n");
	}

	free(author_name);
	deleteAuthorPublicationsMatrix(matrix);
}

static void query6() {
	char* author_list[24];
	char initial, option;
	int i, valid_option, number_read, has_finished;
	int wants_to_read = 1;

	printf("\nENTER AN INITIAL: ");
	scanf("\t%c", &initial);

	while( wants_to_read ) {
		has_finished = getAuthorsBy(initial, author_list, 24, &number_read);

		for(i = 0; i < number_read; i++) {
			printf( "%s\n", author_list[i] );
			free( author_list[i] );
		}

		putchar('\n');

		if ( has_finished )
			wants_to_read = 0;

		else {
			valid_option = 0;

			while (!valid_option) {
				printf("MORE AUTHORS AVAILABLE. CONTINUE READING? [Y/N] ");
				scanf("\t%c", &option);

                option = toupper(option);

				if (option != 'Y' && option != 'N')
					printf("INVALID OPTION\n");

				else {
					if ( option == 'N' )
						wants_to_read = 0;

					valid_option = 1;
				}
			}
		}
	}

	resetAuthorBy(initial);
}

static void query7() {
	int min, max, total;

	printf("ENTER FIRST YEAR:\n");
	scanf("%d%*[^\n]s%*c", &min);

	printf("ENTER LAST YEAR:\n");
	scanf("%d%*[^\n]s%*c", &max);

	total = getYearsTotalByInterval(min, max);

	printf("TOTAL PUBLICATIONS BETWEEN YEARS %d AND %d: %d\n\n\n\n", min, max, total);
}

static void query8() {
	char buffer[1024];
	char* author_name;
	char** list;
	int list_size, i, nr_publications;

	printf("ENTER A NAME: ");
	scanf("\n%1024[^\n]s]", buffer);
	author_name = strtrim(buffer);

	list = getTopCoauthors(author_name, &list_size, &nr_publications);

	if (!list_size)
		printf("AUTHOR NOT FOUND\n\n");

	else {
		( list_size > 1 ) ? printf("\nCOAUTHORS ") : printf("\nCOAUTHOR ");
		printf("WITH MOST PUBLICATIONS (%d ", nr_publications);
		( nr_publications > 1 ) ? printf("PUBLICATIONS) ") : printf("PUBLICATION) ");
		printf("ALONG SIDE %s:\n", author_name);

		for (i = 0; i < list_size; i++)
			printf("%s\n", list[i]);

		putchar('\n');
	}

	free(author_name);
	deleteAuthorList(list, list_size);

}

static void query10() {
	int year, totals[3], size, offset, i, j;
	char fileName[50], temp[1024];
	FILE *f;

	size = 0;
	offset = 0;

	printf("ENTER FILE NAME:\n");
	scanf("%s", fileName);
#ifdef DEBUG
	sprintf(fileName + strlen(fileName), ".q10");
#endif
#ifdef DEBUG2
	sprintf(fileName + strlen(fileName), ".q10");
#endif

	f = fopen(fileName, "a+");

	printf("ENTER A YEAR:\n");
	scanf("%d%*[^\n]s%*c", &year);
	totals[0] = getYearCoAuthorsTotal(year, 1);
	totals[1] = getYearCoAuthorsTotal(year, 2);
	totals[2] = getYearCoAuthorsTotal(year, 3);

	sprintf(temp,
		" ______________\n"
		"|              |\n"
		"| %d", year);

	TOTAL_DIGITS(size, year);

	offset += 35 + size;

	for (i = 0; i < (14 - size - 1); i++) {
		sprintf(temp + offset + i, " ");
	}
	sprintf(temp + offset + i, "|\n");
	offset += i + 2;

	sprintf(temp + offset, "|______________|______________ \n"
						   "|              |              |\n"
						   "| Authors      | Publications |\n"
		                   "|______________|______________|\n");

	offset += 32 * 4;

	for (j = 0; j < 3; j++) {
		sprintf(temp + offset, "|              |              |\n"
			                   "| %d            | %d", j + 1, totals[j]);

		TOTAL_DIGITS(size, totals[j]);

		offset += 32 + 17 + size;

		for (i = 0; i < (14 - size - 1); i++) {
			sprintf(temp + offset + i, " ");
		}
		sprintf(temp + offset + i, "|\n");
		offset += i + 2;

		sprintf(temp + offset, "|______________|______________|\n" );
		offset += 32;
	}
	sprintf(temp + offset, "\n\n");

	printf("%s", temp);
	fprintf(f, "%s", temp);

	fclose(f);
}
static void query11() {
	int test;
	char *yearCSV, fileName[50];
	FILE *f;

	printf("ENTER FILE NAME:\n");
	scanf("%s", fileName);
	sprintf(fileName + strlen(fileName), ".csv");
	f = fopen(fileName, "a+");
	fprintf(f, "\"Year\",\"#Authors\",\"Publications\"\n");
	for (;;) {
		test = yieldYearCSV(&yearCSV);

		if (!test){
			fprintf(f, "%s", yearCSV);
			free(yearCSV);
		}
		else {
			if (test == 1) {
				fprintf(f, "%s", yearCSV);
				free(yearCSV);
			}

			break;
		}
	}

	fclose(f);
}

static void query12() {
	int year, n, i;
	char **authors;
	clock_t begin, end;

	printf("ENTER A YEAR:\n");
	scanf("%d", &year);
	printf("ENTER NUMBER OF AUTHORS:\n");
	scanf("%d", &n);


	TIME(
	authors = topAuthorsInYear(year, n);
	)

	if (!authors) {
		printf("FUCK YOU, BITCH! FUCK YOU!\n");
		return;
	}

	for (i = 0; i < n; i++) {
		printf("%s\n", authors[i]);
		free(authors[i]);
	}

	PRINT_TIME_SPENT("CALCULATING TOP AUTHORS")

	free(authors);

	return;
}

static void query13() {
	char author[128];
	int year;
	float ratio;

	printf("INSERT AUTHOR NAME:\n");
	getchar();
	scanf("%[^\n]", author);

	printf("INSERT YEAR:\n");
	scanf("%d", &year);

	ratio = getAuthorYearRatio(author, year);

	if (ratio < 0) {
		printf("WHAT THE FUCK, DUDE?\n");
		return;
	}

	printf("\n\n%f%%\n\n", ratio * 100.0);

	return;
}

static void query14() {
	char* stats = getAuthorStats();
	printf("%s\n\n", stats );
	free(stats);
}

static void(* functions[NR_FUNCTIONS + NR_ERRORS] )() = {
	&exitGestAuts,
	&query1,
	&query2,
	&query3,
	&query4,
	&query5,
	&query6,
	&query7,
	&query8,
	&failureprnt,
	&query10,
	&query11,
	&query12,
	&query13,
	&query14,
    &invalid_option_err,
    &no_file_read_err
};

static void print_options() {
	int i = 1;

	printf("%s\n", options[0]);
	while (i < NR_OPTIONS) {
		printf("\x1B[33m(%d)\x1B[37m - %s\n", i - 1, options[i]);
		i++;
	}

	putchar('\n');
}

static int get_option() {
	int index;

	scanf("%d%*[^\n]s", &index);

	/* Error handling */
	if( !valid_input( index ) )
		index = NR_FUNCTIONS;
	else if ( index > 1 && !populated_db )
		index = NR_FUNCTIONS + 1;

	return index;
}

static void call_option(int index) {
	(*functions[index])();
}

static void cmd_interpreter() {
	GREET();

#ifdef DEBUG
	call_option(1);
#endif
#ifdef DEBUG2
	call_option(1);
#endif
	while(inGestAuts) {
		print_options();
		call_option( get_option() );
	}

	BID_FAREWELL();
}

int main() {
	initializeGestauts();
	cmd_interpreter();
	return 0;
}

