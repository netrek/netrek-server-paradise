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

/* cutil.c - misc utility fuctions common to all the binaries */

#include "config.h"

#include <signal.h>
#include <sys/types.h>
#include <utime.h>
#include <math.h>

#include "data.h"

/* r_signal - reliable version of signal()
   the System-V signal() function provides the older, unreliable signal
   semantics.  So, this is an implementation of signal using sigaction. */

void    (*
	 r_signal(sig, func)) ()
    int     sig;
    void    (*func) ();
{
    struct sigaction act, oact;

    act.sa_handler = func;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
#ifdef SA_RESTART
#ifdef BAD_SVR4_HACKS
  if( sig != SIGALRM )
#endif /* BAD_SVR4_HACKS */
    act.sa_flags |= SA_RESTART;
#endif

    if (sigaction(sig, &act, &oact) < 0)
	return (SIG_ERR);

    return (oact.sa_handler);
}


int 
touch(char *file)
{
#ifdef HAVE_UTIME_NULL
    return(utime(file, NULL));
#else
    time_t  now[2];

    now[0] = now[1] = time(0);

    return(utime(file, (void *) now));
#endif
}

#ifndef HAVE_STRDUP
char *
strdup(char *str)
{
    char   *s;

    s = (char *) malloc(strlen(str) + 1);
    if(!s)
	return NULL;
    strcpy(s, str);
    return s;
}
#endif


/* dunno if any other systems have anyting like this */
#ifdef sys_hpux

int 
matherr(struct exception *x)
{
    char   *t;

    switch (x->type) {
    case DOMAIN:
	t = "domain";
	break;
    case SING:
	t = "singularity";
	break;
    case OVERFLOW:
	t = "overflow";
	break;
    case UNDERFLOW:
	t = "underflow";
	break;
    case TLOSS:
	t = "tloss";
	break;
    case PLOSS:
	t = "ploss";
	break;
    default:
	t = "buh??";
	break;
    }
    fprintf(stderr, "%s: %s error: %s(%f [, %f]) = %f\n",
	    argv0, t, x->name, x->arg1, x->arg2, x->retval);
}

#endif
