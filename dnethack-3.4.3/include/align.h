/*	SCCS Id: @(#)align.h	3.4	1991/12/29	*/
/* Copyright (c) Mike Stephenson, Izchak Miller  1991.		  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ALIGN_H
#define ALIGN_H

typedef schar	aligntyp;	/* basic alignment type */

typedef struct	align {		/* alignment & record */
	aligntyp	type;
	int		record;
	int		sins; //records trasgressions that should lower the max record.
} align;

/* bounds for "record" -- respect initial alignments of 10 */
#define ALIGNLIM	(min(100L, 10L + (moves/200L) - u.ualign.sins))

#define A_NONE		(-128)	/* the value range of type */

#define A_CHAOTIC	(-1)
#define A_NEUTRAL	 0
#define A_LAWFUL	 1

#define A_VOID		 -3 /* chosen to line up with topten and aligns[] (aligns[] is defined in role.c) */

#define A_COALIGNED	 1
#define A_OPALIGNED	(-1)

#define AM_NONE		 0
#define AM_CHAOTIC	 1
#define AM_NEUTRAL	 2
#define AM_LAWFUL	 4
#define AM_VOID		 8
//AM_SHRINE			16 //Note: this is in another .h file, but must be kept in sync

#define AM_MASK		 15

#define AM_SPLEV_CO	 3
#define AM_SPLEV_NONCO	 7

#define GA_NONE		 0
#define GA_CHAOTIC	 1
#define GA_NEUTRAL	 2
#define GA_LAWFUL	 3
#define GA_VOID		 4
#define GA_SILENCE	 5
#define GA_CHAOS_FF	 6
#define GA_DEMIURGE	 7
#define GA_SOPHIA	 8
#define GA_OTHER	 9
#define GA_MOTHER	10
#define GA_NODENS	11
#define GA_FRACTURE	12
#define GA_SOTHOTH	13
#define GA_NUM		14

#define Amask2align(x)	((aligntyp) ( (x)==AM_NONE ? A_NONE : \
									  (x)==AM_CHAOTIC ? A_CHAOTIC :\
									  (x)==AM_NEUTRAL ? A_NEUTRAL :\
									  (x)==AM_LAWFUL ? A_LAWFUL :\
									  (x)==AM_VOID ? A_VOID : A_NONE\
									))
#define Align2amask(x)	( (x)==A_NONE ? AM_NONE : \
						  (x)==A_CHAOTIC ? AM_CHAOTIC :\
						  (x)==A_NEUTRAL ? AM_NEUTRAL :\
						  (x)==A_LAWFUL ? AM_LAWFUL :\
						  (x)==A_VOID ? AM_VOID : AM_NONE\
						)



#define Amask2gangr(x)	((aligntyp) ( (x)==AM_VOID ? GA_VOID :\
									  ((x)==AM_CHAOTIC && Role_if(PM_EXILE) && Is_astralevel(&u.uz)) ? GA_SOPHIA :\
									  (x)==AM_CHAOTIC ? GA_CHAOTIC :\
									  ((x)==AM_NEUTRAL && Role_if(PM_EXILE) && Is_astralevel(&u.uz)) ? GA_VOID :\
									  (x)==AM_NEUTRAL ? GA_NEUTRAL :\
									  ((x)==AM_LAWFUL && Role_if(PM_EXILE) && Is_astralevel(&u.uz)) ? GA_DEMIURGE :\
									  (x)==AM_LAWFUL ? GA_LAWFUL :\
									  In_mithardir_quest(&u.uz) ? GA_SILENCE :\
									  In_FF_quest(&u.uz) ? GA_CHAOS_FF :\
									  (Role_if(PM_EXILE) && In_quest(&u.uz)) ? GA_DEMIURGE :\
									  on_level(&lethe_temples,&u.uz) ? GA_MOTHER :\
									  on_level(&lethe_headwaters,&u.uz) ? GA_NODENS :\
									  In_outlands(&u.uz) ? GA_FRACTURE :\
									  on_level(&rlyeh_level,&u.uz) ? GA_SOTHOTH :\
									  In_lost_cities(&u.uz) ? GA_OTHER :\
									   GA_NONE\
									))
#define Align2gangr(x)	( (x)==A_VOID ? GA_VOID :\
						  ((x)==A_CHAOTIC && Role_if(PM_EXILE) && Is_astralevel(&u.uz)) ? GA_SOPHIA :\
						  (x)==A_CHAOTIC ? GA_CHAOTIC :\
						  ((x)==A_NEUTRAL && Role_if(PM_EXILE) && Is_astralevel(&u.uz)) ? GA_VOID :\
						  (x)==A_NEUTRAL ? GA_NEUTRAL :\
						  ((x)==A_LAWFUL && Role_if(PM_EXILE) && Is_astralevel(&u.uz)) ? GA_DEMIURGE :\
						  (x)==A_LAWFUL ? GA_LAWFUL :\
						  In_mithardir_quest(&u.uz) ? GA_SILENCE :\
						  In_FF_quest(&u.uz) ? GA_CHAOS_FF :\
						  (Role_if(PM_EXILE) && In_quest(&u.uz)) ? GA_DEMIURGE :\
						  on_level(&lethe_temples,&u.uz) ? GA_MOTHER :\
						  on_level(&lethe_headwaters,&u.uz) ? GA_NODENS :\
						  In_outlands(&u.uz) ? GA_FRACTURE :\
						  on_level(&rlyeh_level,&u.uz) ? GA_SOTHOTH :\
						  In_lost_cities(&u.uz) ? GA_OTHER :\
						  GA_NONE\
						)
#define Gangr2align(x)	((aligntyp) ( (x)==GA_NONE ? A_NONE : \
									  (x)==GA_CHAOTIC ? A_CHAOTIC :\
									  (x)==GA_NEUTRAL ? A_NEUTRAL :\
									  (x)==GA_LAWFUL ? A_LAWFUL :\
									  (x)==GA_VOID ? A_VOID : A_NONE\
									))
#define Gangr2amask(x)	( (x)==GA_NONE ? AM_NONE : \
						  (x)==GA_CHAOTIC ? AM_CHAOTIC :\
						  (x)==GA_NEUTRAL ? AM_NEUTRAL :\
						  (x)==GA_LAWFUL ? AM_LAWFUL :\
						  (x)==GA_VOID ? AM_VOID : AM_NONE\
						)
#endif /* ALIGN_H */
