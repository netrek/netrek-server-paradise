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

/*
  This is probably broken in anything but the default config
*/

#include "config.h"

#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "struct.h"
#include "data.h"

int     topn = 1;
char    name[40] = "";		/* if we want stats for a particular name */
int     nplayers;
struct statentry *database;

struct highscore {
    char    name[32];
    int     di, tkills, tlosses, tarmsbomb, tresbomb, tdooshes, tplanets;
    int     ticks;
    struct highscore *next;
};

struct highscore *scores;
int     scoresize, nscores;

void 
subbrag(name, stuff, time, title, descr, num)
    char   *name;
    int     stuff, time;
    char   *title;
    char   *descr;
    int     num;
{
    char    full[64];
    char    line[80];
    int     len, tlen;
    int     i;
    double  rate;

    if (title) {
	sprintf(full, "%s: %s", title, name);

	len = strlen(full);
	tlen = title ? strlen(title) : 0;

	for (i = len; i - tlen < 10; i++)
	    full[i] = ' ';
	full[i] = 0;
    }
    else {
	strcpy(full, name);
    }

    if (topn != 1)
	sprintf(line, "%15s %3d over ", full, stuff);
    else
	sprintf(line, "%-30s (%2d) %3d %s, over ", full, num, stuff, descr);

    if (time / 10.0 > 3600)
	sprintf(line, "%s%1.2f hours ", line, time / 36000.0);
    else
	sprintf(line, "%s%1.2f minutes ", line, time / 600.0);

    if (topn == 1)
	sprintf(line, "%s\n%40s", line, "");

    rate = stuff / (time / 600.0);

    if (rate < 1) {
	printf(line);
	printf("(%1.2f minutes per)\n", 1 / rate);
    }
    else if (rate > 60) {
	printf(line);
	printf("(%1.2f per hour)\n", rate / 60);
    }
    else {
	printf(line);
	printf("(%1.2f per minute)\n", rate);
    }
}

void 
brag(title, descr, offset)
    char   *title, *descr;
{
    int     i;
    if (name[0] != 0) {
	for (i = 0; i < nscores; i++) {
	    if (0 == strcmp(scores[i].name, name)) {
		printf("#%5d: ", i + 1);
		subbrag("", *(int *) (offset + (char *) &scores[i]),
			scores[i].ticks, title, descr, i);
		break;
	    }
	}
    }
    else {
	if (topn != 1)
	    printf("\n%s (%s)\n", title, descr);
	for (i = 0; i < topn && i < nscores; i++) {
            printf("%10s", "");
	    subbrag(scores[i].name, *(int *) (offset + (char *) &scores[i]),
		    scores[i].ticks, topn == 1 ? title : (char *) 0, descr, i);
	}
    }
}

#if __STDC__
#define COMPUTE(STN) \
    do { \
      currscore->STN = currplayer.stats.st_ ## STN - \
	   prevplayer->stats.st_ ## STN; \
    } while (0)


#define COMPARE(STN) \
int cmp_raw ## STN(a,b) \
     struct highscore *a, *b; \
{ \
  int	diff = a->STN - b->STN; \
 \
  if (diff<0) \
    return 1; \
  else if (diff==0) \
    return 0; \
  else \
    return -1; \
} \
 \
int cmp_per ## STN(a,b) \
     struct highscore *a, *b; \
{ \
  double	diff = a->STN/(double)a->ticks - b->STN/(double)b->ticks; \
 \
  if (diff<0) \
    return 1; \
  else if (diff==0) \
    return 0; \
  else \
    return -1; \
}
#else
#define COMPUTE(STN) \
    do { \
      currscore->STN = currplayer.stats.st_/**/STN - \
	   prevplayer->stats.st_/**/STN; \
    } while (0)


#define COMPARE(STN) \
int cmp_raw/**/STN(a,b) \
     struct highscore *a, *b; \
{ \
  int	diff = a->STN - b->STN; \
 \
  if (diff<0) \
    return 1; \
  else if (diff==0) \
    return 0; \
  else \
    return -1; \
} \
 \
int cmp_per/**/STN(a,b) \
     struct highscore *a, *b; \
{ \
  double	diff = a->STN/(double)a->ticks - b->STN/(double)b->ticks; \
 \
  if (diff<0) \
    return 1; \
  else if (diff==0) \
    return 0; \
  else \
    return -1; \
}
#endif

COMPARE(di)
COMPARE(tkills)
COMPARE(tlosses)
COMPARE(tarmsbomb)
COMPARE(tresbomb)
COMPARE(tdooshes)
COMPARE(tplanets)
    int     cmp_ticks(a, b)
    struct highscore *a, *b;
{
    int     diff = a->ticks - b->ticks;

    if (diff < 0)
	return 1;
    else if (diff == 0)
	return 0;
    else
	return -1;
}

struct statentry zeroplayer;

int 
different(one, two)
    struct highscore *one, *two;
{
    return 0 != strcmp(one->name, two->name);
}

double atof();

int 
main(argc, argv)
    int     argc;
    char  **argv;
{
    struct stat fstats;
    FILE   *fp;
    int     i;
    int     code = 0;
    struct statentry currplayer;
    char  **av;
    int     usage = 0;
    float mintime=30.0;

    for (av = &argv[1]; *av && (*av)[0] == '-'; av++) {
	if (0 == strcmp(*av, "-n") && av[1]) {
	    topn = atoi(*++av);
	}
	else if (0 == strcmp(*av, "-c") && av[1]) {
	    code = atoi(*++av);
	}
	else if (0 == strcmp(*av, "-name") && av[1]) {
	    strcpy(name, *++av);
	}
	else if (0 == strcmp(*av, "-time") && av[1]) {
	    mintime=atof(*++av);
	}
	else {
	    usage = 1;
	    break;
	}
    }
    if (argc - (av - argv) != 2) {
	usage = 1;
    }

    if (usage) {
        int x;
        char message[][255] = {
            "\nHigh Scores, created by comparing two databases.\n",
            "\n\t'%s -n <num> -c <num> [-name <name>] <old db> <new db>'\n\n",
            "Options:\n",
            "\t-n num        How many high scores to print\n",
            "\t-c num        Which category (0 is all, max available is 15)\n",
            "\t-name string  print ranking for a particular player\n",
            "\nExample:\t'%s -n 5 -c 1 .players.bak .players'\n\n",
            "\0"
        };

        fprintf(stderr, "--- %s ---\n", PARAVERS);
        for (x=0; *message[x] != '\0'; x++)
            fprintf(stderr, message[x], argv[0]);

	exit(1);
    }

    fp = fopen(av[0], "r");
    if (fp == 0) {
	fprintf(stderr, "Couldn't open file %s for read", av[0]);
	perror("");
	exit(1);
    }

    if (fstat(fileno(fp), &fstats) < 0) {
	fprintf(stderr, "Couldn't fstat file %s", av[0]);
	perror("");
	exit(1);
    }

    nplayers = fstats.st_size / sizeof(*database);
    database = (struct statentry *) malloc(sizeof(*database) * nplayers);

    i = fread(database, sizeof(*database), nplayers, fp);

    if (i == 0) {
	fprintf(stderr, "failed to read any player records from file %s\n", av[0]);
	exit(1);
    }
    if (i != nplayers) {
	fprintf(stderr, "failed to read all player records from file %s (%d of %d)\n", av[0], i, nplayers);
	nplayers = i;
    }

    fclose(fp);

    fp = fopen(av[1], "r");
    if (fp == 0) {
	fprintf(stderr, "Couldn't open file %s for read", av[1]);
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

	dt = currplayer.stats.st_tticks - prevplayer->stats.st_tticks;

	if (dt < mintime /* minutes */ * 60 * 10)
	    continue;

	if (nscores >= scoresize) {
	    scores = (struct highscore *) realloc(scores, sizeof(*scores) * (scoresize *= 2));
	}
	currscore = &scores[nscores++];
	strcpy(currscore->name, currplayer.name);
	currscore->ticks = dt;

	COMPUTE(di);
	COMPUTE(tkills);
	COMPUTE(tlosses);
	COMPUTE(tarmsbomb);
	COMPUTE(tresbomb);
	COMPUTE(tdooshes);
	COMPUTE(tplanets);
    }


#define offset(field) ( (int)&(((struct highscore*)0)->field) )

    if (!code || code == 1) {
	qsort(scores, nscores, sizeof(*scores), cmp_rawdi);
	brag("Lord of Destruction", "most destruction inflicted", offset(di));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 2) {
	qsort(scores, nscores, sizeof(*scores), cmp_perdi);
	brag("BlitzMeister", "fastest destruction inflicted", offset(di));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 3) {
	qsort(scores, nscores, sizeof(*scores), cmp_rawtkills);
	brag("Hitler", "most opponents defeated", offset(tkills));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 4) {
	qsort(scores, nscores, sizeof(*scores), cmp_pertkills);
	brag("Terminator", "fastest opponents defeated", offset(tkills));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 5) {
	qsort(scores, nscores, sizeof(*scores), cmp_rawtlosses);
	brag("Kamikaze", "most times down in flames", offset(tlosses));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 6) {
	qsort(scores, nscores, sizeof(*scores), cmp_pertlosses);
	brag("Speed Kamikaze", "fastest times down in flames", offset(tlosses));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 7) {
	qsort(scores, nscores, sizeof(*scores), cmp_rawtarmsbomb);
	brag("Carpet Bomber", "most armies bombed", offset(tarmsbomb));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 8) {
	qsort(scores, nscores, sizeof(*scores), cmp_pertarmsbomb);
	brag("NukeMeister", "fastest armies bombed", offset(tarmsbomb));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 9) {
	qsort(scores, nscores, sizeof(*scores), cmp_rawtresbomb);
	brag("Terrorist", "most resources leveled", offset(tresbomb));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 10) {
	qsort(scores, nscores, sizeof(*scores), cmp_pertresbomb);
	brag("Democrat", "fastest resources leveled", offset(tresbomb));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 11) {
	qsort(scores, nscores, sizeof(*scores), cmp_rawtdooshes);
	brag("Executioner", "most armies dooshed", offset(tdooshes));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 12) {
	qsort(scores, nscores, sizeof(*scores), cmp_pertdooshes);
	brag("DooshMeister", "fastest armies dooshed", offset(tdooshes));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 13) {
	qsort(scores, nscores, sizeof(*scores), cmp_rawtplanets);
	brag("Diplomat", "most planets taken", offset(tplanets));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 14) {
	qsort(scores, nscores, sizeof(*scores), cmp_pertplanets);
	brag("speed Diplomat", "fastest planets taken", offset(tplanets));
    }

    if (!code && topn > 5)
	printf("\t@@b\n");

    if (!code || code == 15) {
	qsort(scores, nscores, sizeof(*scores), cmp_ticks);
	if (name[0] != 0) {
	    for (i = 0; i < nscores; i++) {
		if (0 == strcmp(scores[i].name, name)) {
		    printf("#%5d:%30s with %1.2f hours\n", i + 1, scores[i].name,
			   scores[i].ticks / 36000.0);
		    break;
		}
	    }
	}
	else if (topn > 1) {
	    int     i;
	    printf("Addicts:\n");
	    for (i = 0; i < topn && i < nscores; i++) {
		printf("%30s with %1.2f hours\n", scores[i].name, scores[i].ticks / 36000.0);
	    }
	}
	else {
	    printf("Addict: %s with %1.2f hours\n", scores[0].name, scores[0].ticks / 36000.0);
	}
    }

    exit(0);
}
