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
#define ALIGNLIM	(10L + (moves/200L) - u.ualign.sins)

#define A_NONE	      (-128)	/* the value range of type */

#define A_CHAOTIC	(-1)
#define A_NEUTRAL	 0
#define A_LAWFUL	 1

#define A_VOID		 127 /* arbitrary, void alignment is an altar flag only */

#define A_COALIGNED	 1
#define A_OPALIGNED	(-1)

#define AM_NONE		 0
#define AM_CHAOTIC	 1
#define AM_NEUTRAL	 2
#define AM_LAWFUL	 4
#define AM_VOID		 8

#define AM_MASK		 15

#define AM_SPLEV_CO	 3
#define AM_SPLEV_NONCO	 7

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
						  (x)==A_VOID ? AM_VOID : A_NONE\
						)
#endif /* ALIGN_H */
