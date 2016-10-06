/*	SCCS Id: @(#)track.c	3.4	87/08/08	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */
/* track.c - version 1.0.2 */

#include "hack.h"

#define UTSZ	200

STATIC_VAR NEARDATA int utcnt, utpnt;
STATIC_VAR NEARDATA coord utrack[UTSZ];

#ifdef OVLB

void
initrack()
{
	utcnt = utpnt = 0;
}

#endif /* OVLB */
#ifdef OVL1

/* add to track */
void
settrack()
{
	if(utcnt < UTSZ) utcnt++;
	if(utpnt == UTSZ) utpnt = 0;
	utrack[utpnt].x = u.ux;
	utrack[utpnt].y = u.uy;
	utpnt++;
}

#endif /* OVL1 */
#ifdef OVL0

coord *
gettrack(x, y)
register int x, y;
{
    int cnt, ndist;
    coord *tc;
    cnt = utcnt;
    for(tc = &utrack[utpnt]; cnt--; ){
		if(tc == utrack) tc = &utrack[UTSZ-1];
	
	
		else tc--;
		ndist = distmin(x,y,tc->x,tc->y);
	
		if(ndist <= 1)
		    return ndist ? tc : 0 ;
    }
    return (coord *)0;
}

#endif /* OVL0 */

/*track.c*/
