/*	SCCS Id: @(#)eshk.h	3.4	1997/05/01	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ESHK_H
#define ESHK_H

#define REPAIR_DELAY	5	/* minimum delay between shop damage & repair */

#define BILLSZ	200

struct bill_x {
	unsigned bo_id;
	boolean useup;
	long price;		/* price per unit */
	long bquan;		/* amount used up */
};

struct eshk {
	long robbed;		/* amount stolen by most recent customer */
	long credit;		/* amount credited to customer */
	long debit;		/* amount of debt for using unpaid items */
	long loan;		/* shop-gold picked (part of debit) */
	int shoptype;		/* the value of rooms[shoproom].rtype */
	schar shoproom;		/* index in rooms; set by inshop() */
	schar unused;		/* to force alignment for stupid compilers */
	boolean following;	/* following customer since he owes us sth */
	boolean surcharge;	/* angry shk inflates prices */
	boolean pbanned;	/* player is banned from the shop */
	char signspotted;	/* max number of signs spotted by shopkeeper */
	coord shk;		/* usual position shopkeeper */
	coord shd;		/* position shop door */
	d_level shoplevel;	/* level (& dungeon) of his shop */
	int billct;		/* no. of entries of bill[] in use */
	struct bill_x bill[BILLSZ];
	struct bill_x *bill_p;
	int visitct;		/* nr of visits by most recent customer */
	char customer[PL_NSIZ]; /* most recent customer */
	char shknam[PL_NSIZ];
#ifdef OTHER_SERVICES
	long services;          /* Services offered */
#define SHK_ID_BASIC    00001L
#define SHK_ID_PREMIUM  00002L
#define SHK_UNCURSE     00010L
#define SHK_APPRAISE    00100L
#define SHK_SPECIAL_A   01000L
#define SHK_SPECIAL_B   02000L
#define SHK_SPECIAL_C   04000L
#endif
};

#define ESHK(mon)	((struct eshk *)&(mon)->mextra[0])

#define NOTANGRY(mon)	((mon)->mpeaceful)
#define ANGRY(mon)	(!NOTANGRY(mon))

#endif /* ESHK_H */
