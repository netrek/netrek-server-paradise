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

#define NEED_TIME 

#include "config.h"

#include <math.h>
#include <signal.h>
#include "defs.h"
#include "struct.h"
#include "data.h"
#include "shmem.h"

struct player *me;
struct ship *myship;
struct stats *mystats;



/*----------------------------VISIBLE FUNCTIONS---------------------------*/

/*-------------------------------ANGDIST----------------------------------*/
/*  This function provides the proper angular distance between two angles.
The angles are expressed as numbers from 0-255. */

int 
angdist(x, y)
    unsigned char x, y;
{
    register unsigned char res;	/* temp var */

    res = x - y;		/* get abs value of difference */
    if (res > 128)		/* if more than 180 degrees */
	return (256 - (int) res);	/* then choose to go other way around */
    return ((int) res);		/* else its just the difference */
}

/*-------------------------------------------------------------------------*/


/* this function checks to see if an occurrence is temporally spaced
   from the previous one.  This is useful in preventing the client
   from firing torps and missiles too quickly and to limit detting to a
   reasonable frequency (detting too fast burns fuel and increases
   wtemp without any benefit).
   */

int 
temporally_spaced(lasttime, gap)
    struct timeval *lasttime;
    int     gap;		/* microseconds */
{
    struct timeval curtp;

    gettimeofday(&curtp, (struct timezone *) 0);
    if ((curtp.tv_sec == lasttime->tv_sec &&
	 curtp.tv_usec < lasttime->tv_usec + gap)
	|| (curtp.tv_sec == lasttime->tv_sec + 1 &&
	    curtp.tv_usec + 1000000 < lasttime->tv_usec + gap))
	return 0;

    lasttime->tv_sec = curtp.tv_sec;
    lasttime->tv_usec = curtp.tv_usec;
    return 1;
}

/*
 *
 */

int 
check_fire_warp()
{
    if (configvals->fireduringwarp || !(me->p_flags & PFWARP))
	return 1;

    warning("Can not fire while in warp.");

    return 0;
}

int 
check_fire_warpprep()
{
    if (configvals->fireduringwarpprep || !me->p_warptime)
	return 1;

    warning("Can not fire while preparing for warp.");

    return 0;
}

int 
check_fire_docked()
{
    if (configvals->firewhiledocked || !(me->p_flags & PFDOCK))
	return 1;

    warning("It is unsafe to use weapons while docked.");

    return 0;
}


/*-------END OF FILE--------*/
