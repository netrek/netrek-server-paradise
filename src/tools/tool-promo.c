/*--------------------------------------------------------------------------
NETREK II -- Paradise

Permission to use, copy, modify, and distribute this software and its
documentation, or any derivative works thereof, for any NON-COMMERCIAL
purpose and without fee is hereby granted, provided that this copyright
notice appear in all copies.  No representations are made about the
suitability of this software for any purpose.  This software is provided
"as is" without express or implied warranty.

    Xtrek Copyright 1986                            Chris Guthrie
    Netrek (Xtrek II) Copyright 1989                Kevin P. Smith
                                                    Scott Silvey
    Paradise II (Netrek II) Copyright 1993          Larry Denys
                                                    Kurt Olsen
                                                    Brandon Gillespie
--------------------------------------------------------------------------*/

#include "config.h"

#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "struct.h"
#include "data.h"

int     nplayers;
struct statentry *database;

struct highscore {
    char    name[32];
    int     advance, newrank;
    float   didiff;
};

struct highscore *scores;
int     scoresize, nscores;

int 
cmp_score(a, b)
    struct highscore *a, *b;
{
    float   diff = a->newrank - b->newrank;

    if (diff < 0)
	return 1;
    else if (diff > 0)
	return -1;

    diff = a->advance - b->advance;

    if (diff < 0)
	return 1;
    else if (diff > 0)
	return -1;

    diff = a->didiff - b->didiff;

    if (diff < 0)
	return 1;
    else if (diff > 0)
	return -1;
    else
	return 0;
}

struct statentry zeroplayer;

int 
main(argc, argv)
    int     argc;
    char  **argv;
{
    struct stat fstats;
    FILE   *fp;
    int     i;
    int     threshold;
    struct statentry currplayer;

    if (argc != 4) {
	int x;
	char message[][255] = {
	    "\n\t'%s n oldfile newfile'\n",
            "\nLists all players who have been promoted more than n ranks.\n",
            ""
        };

        fprintf(stderr, "-- Netrek II (Paradise), %s --\n", PARAVERS);
        for (i=0; *message[i] != '\0'; i++)
            fprintf(stderr, message[i], argv[0]);

	exit(1);
    }

    threshold = atoi(argv[1]);


    fp = fopen(argv[2], "r");
    if (fp == 0) {
	fprintf(stderr, "Couldn't open file %s for read", argv[1]);
	perror("");
	exit(1);
    }

    if (fstat(fileno(fp), &fstats) < 0) {
	fprintf(stderr, "Couldn't fstat file %s", argv[1]);
	perror("");
	exit(1);
    }

    nplayers = fstats.st_size / sizeof(*database);
    database = (struct statentry *) malloc(sizeof(*database) * nplayers);

    i = fread(database, sizeof(*database), nplayers, fp);

    if (i == 0) {
	fprintf(stderr, "failed to read any player records from file %s\n", argv[1]);
	exit(1);
    }
    if (i != nplayers) {
	fprintf(stderr, "failed to read all player records from file %s (%d of %d)\n", argv[1], i, nplayers);
	nplayers = i;
    }

    fclose(fp);

    fp = fopen(argv[3], "r");
    if (fp == 0) {
	fprintf(stderr, "Couldn't open file %s for read", argv[2]);
	perror("");
	exit(1);
    }


    scores = (struct highscore *) malloc(sizeof(*scores) * (scoresize = 256));
    nscores = 0;

    while (1) {
	int     delta;
	int     dt;
	struct statentry *prevplayer;
	struct highscore *currscore;

	i = fread(&currplayer, sizeof(currplayer), 1, fp);
	if (i < 0) {
	    fprintf(stderr, "error reading player record, aborting loop\n");
	    perror("");
	}
	if (i <= 0)
	    break;

	for (i = 0; i < nplayers; i++) {
	    if (0 == strcmp(database[i].name, currplayer.name))
		break;
	}
	if (i < nplayers)
	    prevplayer = &database[i];
	else
	    prevplayer = &zeroplayer;

	if (currplayer.stats.st_rank - prevplayer->stats.st_rank <= threshold)
	    continue;		/* didn't advance enough */

	if (nscores >= scoresize) {
	    scores = (struct highscore *) realloc(scores, sizeof(*scores) * (scoresize *= 2));
	}
	currscore = &scores[nscores++];
	strcpy(currscore->name, currplayer.name);
	currscore->newrank = currplayer.stats.st_rank;
	currscore->advance = currplayer.stats.st_rank - prevplayer->stats.st_rank;
	currscore->didiff = currplayer.stats.st_di - prevplayer->stats.st_di;
    }


#define offset(field) ( (int)&(((struct highscore*)0)->field) )

    qsort(scores, nscores, sizeof(*scores), cmp_score);

    printf("Congratulations to the following warriors:\n");
    for (i = 0; i < nscores; i++) {
	struct highscore *curr = &scores[i];
	int     j;

	printf("%s ", curr->name);
	for (j = strlen(curr->name); j < 18; j++)
	    putchar(0x1f);
	printf(" promoted from %s to %s", ranks[curr->newrank - curr->advance].name,
	       ranks[curr->newrank].name);
	if (curr->advance > 1) {
	    printf(" (%d ranks)", curr->advance);
	}
	printf("\n");
    }
    printf("(Your new insignia will be provided as soon as we get\n\
 enough scrap plastic donations.)");

    exit(0);
}
