/*--------------------------------------------------------------------------
NETREK II -- Paradise

Permission to use, copy, modify, and distribute this software and its
documentation for any NON-COMMERCIAL purpose (following the terms of
the GNU General Public License (read the file 'COPYING')) and without
fee is hereby granted, provided that this copyright notice appear in all
copies.  No representations are made about the suitability of this
software for any purpose.  This software is provided "as is" without
express or implied warranty.

    Xtrek Copyright 1986                            Chris Guthrie
    Netrek (Xtrek II) Copyright 1989                Kevin P. Smith
                                                    Scott Silvey
    Paradise II (Netrek II) Copyright 1993          Larry Denys
                                                    Kurt Olsen
                                                    Brandon Gillespie

    Comprehensive credits are available in the file "CREDITS"
--------------------------------------------------------------------------*/

#define NEED_TIME 

#include "config.h"
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include "defs.h"
#include "struct.h"
#include "data.h"
#include "shmem.h"
#include "net.h"

static int sendflag = 0;

extern int intrupt();
extern void start_interruptor();
extern int isClientDead();
int     reconnect();
extern int readFromClient();
extern void stop_interruptor();
extern int exitGame();
extern int closeUdpConn();
extern unsigned int sleep();
extern int connectToClient();
extern void (*r_signal()) ();

void 
setflag()
{
    sendflag = 1;
}


void 
input()
{
    fd_set  readfds;

    start_interruptor();

    /* Idea:  read from client often, send to client not so often */
    while (me->p_status == PALIVE ||
	   me->p_status == PEXPLODE ||
	   me->p_status == PDEAD ||
	   me->p_status == POBSERVE) {
	if (isClientDead()) {
	    if (!reconnect())
		exit(0);
	}
	FD_ZERO(&readfds);
	FD_SET(sock, &readfds);
	if (udpSock >= 0)
	    FD_SET(udpSock, &readfds);

	/*
	   blocks indefinitely unless client input or timerDelay SIGALRM
	   causes EINTR
	*/

	if (socketWait() > 0) {
	    readFromClient();
	}
	if (sendflag) {
	    intrupt();
	    sendflag = 0;
	}

	/* statements below are executed once per timerDelay */
	if (me->p_updates > delay) {
	    me->p_flags &= ~(PFWAR);
	}
	if (me->p_updates > rdelay) {
	    me->p_flags &= ~(PFREFITTING);
	}

	/*
	   this will ghostbust a player if no ping has been received in
	   ping_ghostbust_interval seconds
	*/

	if (configvals->ping_allow_ghostbust
	    && (me->p_status == PALIVE || me->p_status == POBSERVE)
	    && ping_ghostbust > configvals->ping_ghostbust_interval)
	    me->p_ghostbuster = 100000;	/* ghostbusted */
	else
	    /* Keep self from being nuked... */
	    me->p_ghostbuster = 0;
    }
    stop_interruptor();
}

int 
reconnect()
{
    int     i;

    if (noressurect)
	exitGame();
    stop_interruptor();

    r_signal(SIGIO, SIG_IGN);
#ifdef DEBUG
    printf("Ack!  The client went away!\n");
    printf("I will attempt to resurrect him!\n");
    fflush(stdout);
#endif
    commMode = COMM_TCP;
    if (udpSock >= 0)
	closeUdpConn();

    /* For next two minutes, we try to restore connection */
    shutdown(sock, 2);
    sock = -1;
    for (i = 0;; i++) {
	me->p_ghostbuster = 0;
	sleep(5);
	if (connectToClient(host, nextSocket))
	    break;
	if (i == 23) {
#ifdef DEBUG
	    printf("Oh well, maybe I'm getting rusty!\n");
	    fflush(stdout);
#endif
	    return 0;
	}
    }
    start_interruptor();
#ifdef DEBUG
    printf("A miracle!  He's alive!\n");
    fflush(stdout);
#endif
    return 1;
}
