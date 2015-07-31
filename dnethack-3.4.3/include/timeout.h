/*	SCCS Id: @(#)timeout.h	3.4	1999/02/13	*/
/* Copyright 1994, Dean Luick					  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TIMEOUT_H
#define TIMEOUT_H

/* generic timeout function */
typedef void FDECL((*timeout_proc), (genericptr_t, long));

/* kind of timer */
#define TIMER_LEVEL	0	/* event specific to level */
#define TIMER_GLOBAL	1	/* event follows current play */
#define TIMER_OBJECT	2	/* event follows a object */
#define TIMER_MONSTER	3	/* event follows a monster */

/* save/restore timer ranges */
#define RANGE_LEVEL  0		/* save/restore timers staying on level */
#define RANGE_GLOBAL 1		/* save/restore timers following global play */

/* flags for revive type after timeout */
#define	REVIVE_MONSTER	0
#define	GROW_MOLD		1
#define	REVIVE_MOLD		2
#define	GROW_SLIME		3
#define	REVIVE_ZOMBIE	4
#define	REVIVE_SHADE	5
/*
 * Timeout functions.  Add a define here, then put it in the table
 * in timeout.c.  "One more level of indirection will fix everything."
 */
#define ROT_ORGANIC	0	/* for buried organics */
#define ROT_CORPSE	1
#define MOLDY_CORPSE	2
#define REVIVE_MON	3
#define BURN_OBJECT	4
#define HATCH_EGG	5
#define FIG_TRANSFORM	6
#define LIGHT_DAMAGE	7
#define SLIMY_CORPSE	8
#define ZOMBIE_CORPSE	9
#define SHADY_CORPSE	10
//# ifdef FIREARMS
#define BOMB_BLOW	11
#define NUM_TIME_FUNCS	12
//# else /* FIREARMS */
//#define NUM_TIME_FUNCS	11
//# endif /* FIREARMS */

/* used in timeout.c */
typedef struct fe {
    struct fe *next;		/* next item in chain */
    long timeout;		/* when we time out */
    unsigned long tid;		/* timer ID */
    short kind;			/* kind of use */
    short func_index;		/* what to call when we time out */
    genericptr_t arg;		/* pointer to timeout argument */
    Bitfield (needs_fixup,1);	/* does arg need to be patched? */
} timer_element;

#endif /* TIMEOUT_H */
