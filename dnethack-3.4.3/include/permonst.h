/*	SCCS Id: @(#)permonst.h 3.4	1999/07/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef PERMONST_H
#define PERMONST_H

/*	This structure covers all attack forms.
 *	aatyp is the gross attack type (eg. claw, bite, breath, ...)
 *	adtyp is the damage type (eg. physical, fire, cold, spell, ...)
 *	damn is the number of hit dice of damage from the attack.
 *	damd is the number of sides on each die.
 *
 *	Some attacks can do no points of damage.  Additionally, some can
 *	have special effects *and* do damage as well.  If damn and damd
 *	are set, they may have a special meaning.  For example, if set
 *	for a blinding attack, they determine the amount of time blinded.
 */

struct attack {
	uchar		aatyp;
	uchar		adtyp, damn, damd;
};

/*	Max # of attacks for any given monster.
 */

#define NATTK		10

/*	Weight of a human body
 */

#define WT_BEE	 	 1
#define WT_DIMINUTIVE 10
#define WT_ANT	 	 WT_DIMINUTIVE
#define WT_TINY	 	 140
#define WT_HOBBIT	 500
#define WT_SMALL	 750
#define WT_ELF		 800
#define WT_DWARF	 900
#define WT_HUMAN	1450
#define WT_MEDIUM	1500
#define WT_LARGE   	3000
#define WT_HUGE		4500
#define WT_DRAGON 	4500
#define WT_GIGANTIC	9000

#define CN_BEE		 5
#define CN_DIMINUTIVE 10
#define CN_ANT		 CN_DIMINUTIVE
#define CN_TINY		 100
#define CN_GNOME	 100
#define CN_HOBBIT	 200
#define CN_SMALL	 200
#define CN_DWARF	 300
#define CN_HUMAN	 400
#define CN_MEDIUM	 400
#define CN_LARGE	 800
#define CN_HUGE		1000
#define CN_GIGANTIC	1500

#ifndef ALIGN_H
#include "align.h"
#endif
#include "monattk.h"
#include "monflag.h"

struct permonst {
	const char	*mname;			/* full name */
	char		mlet;			/* symbol */
	schar		mlevel,			/* base monster level */
			mmove,			/* move speed */
			nac,				/* Natural-armor AC */
			dac,			/* Dodge AC */
			pac,			/* Protection AC*/
			
			/* (base) dr values for head/body/gloves/legs/feet */
			hdr,
			bdr,
			gdr,
			ldr,
			fdr, 
			/* (special) dr values for head/body/gloves/legs/feet */
			spe_hdr,
			spe_bdr,
			spe_gdr,
			spe_ldr,
			spe_fdr, 
			mr;			/* (base) magic resistance */
	aligntyp	maligntyp;		/* basic monster alignment */
	unsigned short	geno;			/* creation/geno mask value */
	struct	attack	mattk[NATTK];		/* attacks matrix */
	unsigned short	cwt,			/* weight of corpse */
					cnutrit;		/* its nutritional value */
	short		pxlth;			/* length of extension */
	uchar		msound;			/* noise it makes (6 bits) */
	uchar		msize;			/* physical size (3 bits) */
/* Assumes that vocalizations from the monster account for some combat noise */
#define combatNoise(dat) (dat->msound ? dat->msize*2+1 : dat->msize+1)
	unsigned int	mresists;		/* resistances */
	unsigned int	mconveys;		/* conveyed by eating */
	unsigned long	mflagsm,		/* Monster Motility boolean bitflags */
					mflagst,		/* Monster Thoughts and behavior boolean bitflags */
					mflagsb,		/* Monster Body plan boolean bitflags */
					mflagsg,		/* Monster Game mechanics and bookkeeping boolean bitflags */
					mflagsa,		/* Monster rAce boolean bitflags */
					mflagsv;		/* Monster Vision boolean bitflags */
# ifdef TEXTCOLOR
	uchar		mcolor;			/* color to use */
# endif
};

extern NEARDATA struct permonst
		mons[];		/* the master list of monster types */

#define VERY_SLOW 3
#define SLOW_SPEED 9
#define NORMAL_SPEED 12 /* movement rates */
#define FAST_SPEED 15
#define VERY_FAST 24

#define NON_PM		PM_PLAYERMON		/* "not a monster" */
#define LOW_PM		(NON_PM+1)		/* first monster in mons[] */
#define SPECIAL_PM	PM_LONG_WORM_TAIL	/* [normal] < ~ < [special] */
	/* mons[SPECIAL_PM] through mons[NUMMONS-1], inclusive, are
	   never generated randomly and cannot be polymorphed into */

#endif /* PERMONST_H */
