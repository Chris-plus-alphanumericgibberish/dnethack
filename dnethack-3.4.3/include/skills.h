/*	SCCS Id: @(#)skills.h	3.4	1999/10/27	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985-1999. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SKILLS_H
#define SKILLS_H

/* Much of this code was taken from you.h.  It is now
 * in a separate file so it can be included in objects.c.
 */


/* Code to denote that no skill is applicable */
#define P_NONE				0

/* Weapon Skills -- Stephen White
 * Order matters and are used in macros.
 * Positive values denote hand-to-hand weapons or launchers.
 * Negative values denote ammunition or missiles.
 * Update weapon.c if you ammend any skills.
 * Also used for oc_subtyp.
 */
#define P_DAGGER             1
#define P_KNIFE              2
#define P_AXE                3
#define P_PICK_AXE           4
#define P_SHORT_SWORD        5
#define P_BROAD_SWORD        6
#define P_LONG_SWORD         7
#define P_TWO_HANDED_SWORD   8
#define P_SCIMITAR           9
#define P_SABER             10
#define P_CLUB              11	/* Heavy-shafted bludgeon */
#define P_MACE              12	
#define P_MORNING_STAR      13	/* Spiked bludgeon */
#define P_FLAIL             14	/* Two pieces hinged or chained together */
#define P_HAMMER            15	/* Heavy head on the end */
#define P_QUARTERSTAFF      16	/* Long-shafted bludgeon */
#define P_POLEARMS          17
#define P_SPEAR             18  /* includes javelin */
#define P_TRIDENT           19
#define P_LANCE             20
#define P_BOW               21
#define P_SLING             22
//#ifdef FIREARMS
#define P_FIREARM			23	/* KMH */
//#endif
#define P_CROSSBOW          24
#define P_DART              25
#define P_SHURIKEN          26
#define P_BOOMERANG         27
#define P_WHIP              28
#define P_HARVEST 		    29
#define P_UNICORN_HORN      30	/* last weapon */
#define P_FIRST_WEAPON      P_DAGGER
#define P_LAST_WEAPON       P_UNICORN_HORN

/* Spell Skills added by Larry Stewart-Zerba */
#define P_ATTACK_SPELL		(P_LAST_WEAPON + 1)
#define P_HEALING_SPELL		(P_LAST_WEAPON + 2)
#define P_DIVINATION_SPELL	(P_LAST_WEAPON + 3)
#define P_ENCHANTMENT_SPELL	(P_LAST_WEAPON + 4)

#define P_CLERIC_SPELL      (P_LAST_WEAPON + 5)
#define P_ESCAPE_SPELL      (P_LAST_WEAPON + 6)
#define P_MATTER_SPELL      (P_LAST_WEAPON + 7)
#define P_WAND_POWER		(P_LAST_WEAPON + 8)
#define P_FIRST_SPELL		P_ATTACK_SPELL
#ifdef BARD
#define P_MUSICALIZE		(P_LAST_WEAPON + 9)	/* 'cast' spells as songs */
#define P_LAST_SPELL		P_MUSICALIZE
#else
#define P_LAST_SPELL		P_WAND_POWER
#endif

/* musicalize indices */
#define SNG_FEAR		1
#define SNG_SLEEP		2
#define SNG_HEAL		3
#define SNG_RLLY	    4
#define SNG_CONFUSION	5
#define SNG_HASTE		6
#define SNG_CNCL		7
#define SNG_SLOW		8
#define SNG_TAME		9
#define SNG_COURAGE	   10

/* Other types of combat */
#define P_BARE_HANDED_COMBAT	(P_LAST_SPELL + 1)
#define P_MARTIAL_ARTS		P_BARE_HANDED_COMBAT	/* Role distinguishes */
#define P_TWO_WEAPON_COMBAT	(P_LAST_SPELL + 2)	/* Finally implemented */
#define P_BEAST_MASTERY	(P_LAST_SPELL + 3)	/* Finally implemented */
#define FFORM_SHII_CHO	(P_LAST_SPELL + 4)
#define FFORM_MAKASHI	(P_LAST_SPELL + 5)
#define FFORM_SORESU	(P_LAST_SPELL + 6)
#define FFORM_ATARU		(P_LAST_SPELL + 7)
#define FFORM_DJEM_SO	(P_LAST_SPELL + 8)
#define FFORM_SHIEN		(P_LAST_SPELL + 9)
#define FFORM_NIMAN		(P_LAST_SPELL +10)
#define FFORM_JUYO		(P_LAST_SPELL +11)
#ifdef STEED
#define P_RIDING		(P_LAST_SPELL +12)	/* How well you control your steed */
#define P_LAST_H_TO_H		P_RIDING
#else
#define P_LAST_H_TO_H		P_TWO_WEAPON_COMBAT
#endif
#define P_FIRST_H_TO_H		P_BARE_HANDED_COMBAT

#define P_NUM_SKILLS		(P_LAST_H_TO_H+1)

/* These roles qualify for a martial arts bonus */
#define martial_bonus()	(u.umartial)


/*
 * These are the standard weapon skill levels.  It is important that
 * the lowest "valid" skill be be 1.  The code calculates the
 * previous amount to practice by calling  practice_needed_to_advance()
 * with the current skill-1.  To work out for the UNSKILLED case,
 * a value of 0 needed.
 */
#define P_ISRESTRICTED	0
#define P_UNSKILLED		1
#define P_BASIC			2
#define P_SKILLED		3
#define P_EXPERT		4
#define P_MASTER		5	/* Unarmed combat/martial arts only */
#define P_GRAND_MASTER	6	/* Unarmed combat/martial arts only */

#define practice_needed_to_advance(level) ((level)*(level)*20)

/* The hero's skill in various weapons. */
struct skills {
	xchar skill;
	xchar max_skill;
	unsigned short advance;
};

#define OLD_P_SKILL(type)		(u.weapon_skills[type].skill)
#define OLD_P_MAX_SKILL(type)	(u.weapon_skills[type].max_skill)
#define P_ADVANCE(type)		(u.weapon_skills[type].advance)
#define OLD_P_RESTRICTED(type)	(u.weapon_skills[type].skill == P_ISRESTRICTED)

#define P_SKILL_LIMIT 90	/* Max number of skill advancements (added +30 for human bonus, even though shouldnt need that many) */

/* Initial skill matrix structure; used in u_init.c and weapon.c */
struct def_skill {
	xchar skill;
	xchar skmax;
};

#endif  /* SKILLS_H */
