/*	SCCS Id: @(#)trap.h	3.4	2000/08/30	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* note for 3.1.0 and later: no longer manipulated by 'makedefs' */

#ifndef TRAP_H
#define TRAP_H

union vlaunchinfo {
	/* ??? bits - max(<sizeof(pointer)>, 16bits). */
	struct obj* v_ammo;	/* quivered object associated with this trap - rocks/darts/arrows/beartraps/landmines. */
	/* rolling boulder */
	short v_launch_otyp;	/* type of object to be triggered */
	coord v_launch2;	/* secondary launch point (for boulders) */
	/* statue */
	unsigned int v_statue_oid;	/* object id of original statue trap on square to activate */
};

struct trap {
	struct trap *ntrap;
	xchar tx,ty;
	d_level dst;	/* destination for portals */
	coord launch;
	Bitfield(ttyp,5);
	Bitfield(tseen,1);
	Bitfield(once,1);
	Bitfield(madeby_u,1); /* So monsters may take offence when you trap
				 them.	Recognizing who made the trap isn't
				 completely unreasonable, everybody has
				 their own style.  This flag is also needed
				 when you untrap a monster.  It would be too
				 easy to make a monster peaceful if you could
				 set a trap for it and then untrap it. */
	union vlaunchinfo vl;
#define launch_ammo    vl.v_ammo
#define launch_otyp	vl.v_launch_otyp
#define launch2		vl.v_launch2
#define statueid       vl.v_statue_oid
};

extern struct trap *ftrap;
#define newtrap()	(struct trap *) alloc(sizeof(struct trap))
#define dealloc_trap(trap) free((genericptr_t) (trap))

/* what vl to use */
#define trapv_ammo(ttyp)	((ttyp) == DART_TRAP || (ttyp) == ARROW_TRAP || (ttyp) == ROCKTRAP ||\
								(ttyp) == BEAR_TRAP || (ttyp) == LANDMINE || (ttyp) == FIRE_TRAP)
#define trapv_launch(ttyp)	((ttyp) == ROLLING_BOULDER_TRAP)
#define trapv_statue(ttyp)	((ttyp) == STATUE_TRAP)

/* reasons for statue animation */
#define ANIMATE_NORMAL	0
#define ANIMATE_SHATTER 1
#define ANIMATE_SPELL	2

/* reasons for animate_statue's failure */
#define AS_OK		 0	/* didn't fail */
#define AS_NO_MON	 1	/* makemon failed */
#define AS_MON_IS_UNIQUE 2	/* statue monster is unique */

/* Note: if adding/removing a trap, adjust trap_engravings[] in mklev.c */

/* unconditional traps */
#define NO_TRAP		0
#define ARROW_TRAP	1
#define DART_TRAP	2
#define ROCKTRAP	3
#define SQKY_BOARD	4
#define BEAR_TRAP	5
#define LANDMINE	6
#define ROLLING_BOULDER_TRAP	7
#define SLP_GAS_TRAP	8
#define RUST_TRAP	9
#define FIRE_TRAP	10
#define PIT		11
#define SPIKED_PIT	12
#define HOLE		13
#define TRAPDOOR	14
#define TELEP_TRAP	15
#define LEVEL_TELEP	16
#define MAGIC_PORTAL	17
#define WEB		18
#define STATUE_TRAP	19
#define MAGIC_TRAP	20
#define ANTI_MAGIC	21
#define POLY_TRAP	22
#define VIVI_TRAP	23
#define MUMMY_TRAP	24
#define TRAPNUM 25

#endif /* TRAP_H */
