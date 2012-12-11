/*	SCCS Id: @(#)engrave.h	3.4	1991/07/31	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ENGRAVE_H
#define ENGRAVE_H

struct engr {
	struct engr *nxt_engr;
	char *engr_txt;
	xchar engr_x, engr_y;
	unsigned engr_lth;	/* for save & restore; not length of text */
	long engr_time;		/* moment engraving was (will be) finished */
	xchar engr_type;
#define DUST	   1
#define ENGRAVE    2
#define BURN	   3
#define MARK	   4
#define ENGR_BLOOD 5
#define HEADSTONE  6
#define N_ENGRAVE  6
	xchar ward_type;
	int ward_id; /* id number of ward, or of the halucinatory ward if halu_ward is TRUE */
#define HEPTAGRAM					1
#define GORGONEION					2
#define CIRCLE_OF_ACHERON			3
#define PENTAGRAM					4
#define HEXAGRAM					5
#define HAMSA						6
#define ELDER_SIGN					7
#define ELDER_ELEMENTAL_EYE			8
#define SIGN_OF_THE_SCION_QUEEN		9
#define CARTOUCHE_OF_THE_CAT_LORD	10
#define WINGS_OF_GARUDA				11
#define NUMBER_OF_WARDS				11

/*Functional halucinatory wards, keep in sync with engrave.c!*/
#define CERULEAN_SIGN				2
	xchar halu_ward; /* If you draw while halucinating, you inscribe a halucinatory symbol */

/*One type of ward per square
Max of seven redundant copies (so you can fight on the ward)
Note: max of 3 Gorgoneia on a square
Note: max of 4 circles of acheron per square. Makes it a good emergency ward but a poor combat ward.
Note: max of 6 hamsa on the square. Hamsa after the first two must be drawn in pairs
Note: max of 6 elder signs per square. They form a more detailed, star shaped sign.
Note: elder elemental eye becomes more potent at 5 and 7 copies
*/
	xchar complete_wards; // functional
	xchar scuffed_wards; // 90%
	xchar degraded_wards;// 75%
	xchar partial_wards;// 50%
};

#define newengr(lth) (struct engr *)alloc((unsigned)(lth) + sizeof(struct engr))
#define dealloc_engr(engr) free((genericptr_t) (engr))

#endif /* ENGRAVE_H */
