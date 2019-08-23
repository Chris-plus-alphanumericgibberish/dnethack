/*	SCCS Id: @(#)spell.h	3.4	1995/06/01	*/
/* Copyright 1986, M. Stephenson				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SPELL_H
#define SPELL_H

/* spellmenu arguments; 0 thru n-1 used as spl_book[] index when swapping */
#define SPELLMENU_MAINTAIN (-4)
#define SPELLMENU_DESCRIBE (-3)
#define SPELLMENU_CAST (-2)
#define SPELLMENU_VIEW (-1)

struct spell {
    short	sp_id;			/* spell id (== object.otyp) */
    xchar	sp_lev;			/* power level */
    int		sp_know;		/* knowlege of spell */
};

#define decrnknow(spell)	spl_book[spell].sp_know--
#define spellid(spell)		spl_book[spell].sp_id
#define spellknow(spell)	spl_book[spell].sp_know

#define FIRST_LIGHT	MAXSPELL+1
#define PART_WATER	MAXSPELL+2
#define OVERGROW	MAXSPELL+3

#define MAX_BONUS_DICE	10

#endif /* SPELL_H */
