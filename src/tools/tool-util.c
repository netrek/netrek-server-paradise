#include "config.h"

#include "tool-util.h"
#include "shmem.h"
#include "data.h"

char 
team_to_letter(t)
    int     t;
{
    switch (t) {
    case FED:
	return 'F';
    case ROM:
	return 'R';
    case KLI:
	return 'K';
    case ORI:
	return 'O';
    default:
	return 'I';
    }
}

int 
letter_to_team(ch)
    char    ch;
{
    switch (ch) {
    case 'I':
    case 'i':
	return 0;
    case 'F':
    case 'f':
	return FED;
    case 'R':
    case 'r':
	return ROM;
    case 'K':
    case 'k':
	return KLI;
    case 'O':
    case 'o':
	return ORI;
    default:
	return -1;
    }
}

int 
letter_to_pnum(ch)
    char    ch;
{
    switch (ch) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	return ch - '0';

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
	return ch - 'a' + 10;
    default:
	return -1;
    }
}

char 
pnum_to_letter(ch)
    int     ch;
{
    switch (ch) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
	return ch + '0';
    default:
	return ch - 10 + 'a';
    }
}


char   *
twoletters(pl)
    struct player *pl;
/* calculate the two letters that form the players designation (e.g. R4) */
{
#define RINGSIZE MAXPLAYER+3
    static char buf[RINGSIZE][3];	/* ring of buffers so that this */
    static int idx;		/* proc can be called several times before
				   the results are used */
    if (idx >= RINGSIZE)
	idx = 0;
    buf[idx][0] = teams[pl->p_team].letter;
    buf[idx][1] = shipnos[pl->p_no];
    buf[idx][2] = 0;
    return buf[idx++];
}
