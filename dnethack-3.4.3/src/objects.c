/*	SCCS Id: @(#)objects.c	3.4	2002/07/31	*/
/* Copyright (c) Mike Threepoint, 1989.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OBJECTS_PASS_2_
/* first pass */
struct monst { struct monst *dummy; };	/* lint: struct obj's union */
#include "config.h"
#include "obj.h"
#include "objclass.h"
#include "prop.h"
#include "skills.h"

#else	/* !OBJECTS_PASS_2_ */
/* second pass */
#include "color.h"
#  define COLOR_FIELD(X) X,
#endif	/* !OBJECTS_PASS_2_ */


/* objects have symbols: ) [ = " ( % ! ? + / $ * ` 0 _ . */

/*
 *	Note:  OBJ() and BITS() macros are used to avoid exceeding argument
 *	limits imposed by some compilers.  The ctnr field of BITS currently
 *	does not map into struct objclass, and is ignored in the expansion.
 *	The 0 in the expansion corresponds to oc_pre_discovered, which is
 *	set at run-time during role-specific character initialization.
 */

#ifndef OBJECTS_PASS_2_
/* first pass -- object descriptive text */
# define OBJ(name,desc) name,desc
# define OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,nut,color) \
	{obj}

NEARDATA struct objdescr obj_descr[] = {
#else
/* second pass -- object definitions */

# define BITS(nmkn,mrg,uskn,ctnr,mgc,chrg,uniq,nwsh,big,tuf,dir,sub,mtrl) \
	nmkn,mrg,uskn,0,mgc,chrg,uniq,nwsh,big,tuf,dir,mtrl,sub /* SCO ODT 1.1 cpp fodder */
# define OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,nut,color) \
	{0, 0, (char *)0, bits, prp, sym, dly, COLOR_FIELD(color) \
	 prob, wt, cost, sdam, ldam, oc1, oc2, nut}
# ifndef lint
#  define HARDGEM(n) (n >= 8)
# else
#  define HARDGEM(n) (0)
# endif

NEARDATA struct objclass objects[] = {
#endif
/* dummy object[0] -- description [2nd arg] *must* be NULL */
	OBJECT(OBJ("strange object",(char *)0), BITS(1,0,0,0,0,0,0,0,0,0,0,P_NONE,0),
			0, ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),

/* weapons ... */
#define WEAPON(name,app,kn,mg,bi,prob,wt,cost,sdam,ldam,hitbon,typ,sub,metal,color) \
	OBJECT( \
		OBJ(name,app), BITS(kn,mg,1,0,0,1,0,0,bi,0,typ,sub,metal), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, 0, wt, color )
#define PROJECTILE(name,app,kn,prob,wt,cost,sdam,ldam,hitbon,metal,sub,color) \
	OBJECT( \
		OBJ(name,app), \
		BITS(kn,1,1,0,0,1,0,0,0,0,PIERCE,sub,metal), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, 0, wt, color )
#define BOW(name,app,kn,prob,wt,cost,hitbon,metal,sub,color) \
	OBJECT( \
		OBJ(name,app), BITS(kn,0,1,0,0,1,0,0,0,0,0,sub,metal), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, 2, 2, hitbon, 0, wt, color )

/* Note: for weapons that don't do an even die of damage (ex. 2-7 or 3-18)
 * the extra damage is added on in weapon.c, not here! */

#define P PIERCE
#define S SLASH
#define B WHACK

/* missiles */
PROJECTILE("arrow", (char *)0,
		1, 50, 1, 2, 6, 6, 0, IRON, -P_BOW, HI_METAL),
PROJECTILE("elven arrow", "runed arrow",
		0, 18, 1, 2, 7, 5, 2, WOOD, -P_BOW, HI_WOOD),
PROJECTILE("orcish arrow", "crude arrow",
		0, 18, 1, 2, 5, 8, -1, IRON, -P_BOW, CLR_BLACK),
PROJECTILE("silver arrow", (char *)0,
		1, 12, 1, 5, 6, 6, 0, SILVER, -P_BOW, HI_SILVER),
PROJECTILE("golden arrow", (char *)0,
		1,  9, 2,10,13,13, 0, GOLD, -P_BOW, HI_GOLD),
PROJECTILE("ancient arrow", (char *)0,
		1,  0, 1,10,10,10, 0, METAL, -P_BOW, CLR_BLACK),
PROJECTILE("ya", "bamboo arrow",
		0, 15, 1, 4, 7, 7, 1, METAL, -P_BOW, HI_METAL),
PROJECTILE("crossbow bolt", (char *)0,
		1, 55, 1, 2, 4, 6, 0, IRON, -P_CROSSBOW, HI_METAL),
PROJECTILE("droven bolt", "obsidian bolt",
		0,  0, 1, 2, 9, 6, 2, GLASS, -P_CROSSBOW, CLR_BLACK),

WEAPON("dart", (char *)0,
	1, 1, 0, 60,  1,  2,  3,  2, 0, P,   -P_DART, IRON, HI_METAL),
WEAPON("shuriken", "throwing star",
	0, 1, 0, 35,  1,  5,  8,  6, 2, P,   -P_SHURIKEN, IRON, HI_METAL),
WEAPON("boomerang", (char *)0,
	1, 1, 0, 15,  5, 20,  9,  9, 0, 0,   -P_BOOMERANG, WOOD, HI_WOOD),
WEAPON("spike", (char *)0,
	1, 1, 0,  0,  1,  2,  3,  2, 0, P,   -P_DART, BONE, CLR_WHITE),

/* spears */
WEAPON("spear", (char *)0,
	1, 1, 0, 48, 25,  3,  6,  8, 0, P,   P_SPEAR, IRON, HI_METAL),
WEAPON("elven spear", "runed spear",
	0, 1, 0, 10, 25,  3,  7,  7, 2, P,   P_SPEAR, WOOD, HI_WOOD),
WEAPON("droven spear", "long obsidian spear",
	0, 1, 1,  0, 25,  3, 12, 12, 2, P,   P_SPEAR, GLASS, CLR_BLACK),
WEAPON("orcish spear", "crude spear",
	0, 1, 0, 13, 25,  3,  5,  10, -1, P,   P_SPEAR, IRON, CLR_BLACK),
WEAPON("dwarvish spear", "stout spear",
	0, 1, 0, 12, 30,  3,  9,  9, 0, P,   P_SPEAR, IRON, HI_METAL),
WEAPON("silver spear", (char *)0,
	1, 1, 0,  2, 30, 40,  6,  8, 0, P,   P_SPEAR, SILVER, HI_SILVER),
WEAPON("javelin", "throwing spear",
	0, 1, 0, 10, 20,  3,  6,  6, 0, P,   P_JAVELIN, IRON, HI_METAL),

WEAPON("trident", (char *)0,
	1, 0, 0,  8, 25,  5,  6,  4, 0, P,   P_TRIDENT, IRON, HI_METAL),
						/* +1 small, +2d4 large */

/* blades */
WEAPON("dagger", (char *)0,
	1, 1, 0, 22, 10,  4,  4,  3, 2, P,   P_DAGGER, IRON, HI_METAL),
WEAPON("elven dagger", "runed dagger",
	0, 1, 0,  7, 10,  4,  5,  3, 4, P,   P_DAGGER, WOOD, HI_WOOD),
WEAPON("droven dagger", "obsidian dagger",
	0, 1, 0,  0, 10,  12,  8,  6, 4, P,   P_DAGGER, GLASS, CLR_BLACK),
WEAPON("orcish dagger", "crude dagger",
	0, 1, 0,  9, 10,  5,  3,  5, 1, P,   P_DAGGER, IRON, CLR_BLACK),
WEAPON("silver dagger", (char *)0,
	1, 1, 0,  2, 12, 40,  4,  3, 2, P,   P_DAGGER, SILVER, HI_SILVER),
WEAPON("athame", (char *)0,
	1, 1, 0,  0, 10,  4,  4,  4, 2, S,   P_DAGGER, IRON, HI_METAL),
WEAPON("scalpel", (char *)0,
	1, 1, 0,  0,  5,  6,  3,  1, 3, S,   P_KNIFE, METAL, HI_METAL),
WEAPON("knife", (char *)0,
	1, 1, 0, 14,  5,  4,  3,  2, 2, P|S, P_KNIFE, IRON, HI_METAL),
WEAPON("stiletto", (char *)0,
	1, 1, 0,  4,  5,  4,  6,  2, 1, P|S, P_KNIFE, IRON, HI_METAL),
WEAPON("worm tooth", (char *)0,
	1, 0, 0,  0, 20,  2,  2,  2, 0, 0,   P_KNIFE, 0, CLR_WHITE),
WEAPON("crysknife", (char *)0,
	1, 0, 0,  0, 20,100, 10, 10, 3, P,   P_KNIFE, MINERAL, CLR_WHITE),

WEAPON("sickle", (char *)0, /* Vs plants: +6 to hit and double damage */
	1, 1, 0, 22, 20,  4,  4,  1, -2, S,   P_HARVEST, IRON, HI_METAL),
WEAPON("elven sickle", (char *)0, /* Vs plants: +6 to hit and double damage */
	1, 1, 0,  0, 20,  4,  6,  3,  0, S,   P_HARVEST, WOOD, HI_WOOD),

WEAPON("axe", (char *)0,
	1, 0, 0, 36, 60,  8,  6,  4, 0, S,   P_AXE, IRON, HI_METAL),
WEAPON("battle-axe", "double-headed axe",/* "double-bitted" ? */
	0, 0, 1, 10,120, 40,  8,  6, 0, S,   P_AXE, IRON, HI_METAL), /*+1d4/+2d4*/
WEAPON("moon axe", "large silver axe",
	0, 0, 1,  2,160, 40,  6,  6, 0, S,   P_AXE, SILVER, HI_SILVER),

/* swords */
WEAPON("short sword", (char *)0,
	1, 0, 0,  8, 30, 10,  6,  8, 0, P,   P_SHORT_SWORD, IRON, HI_METAL),
WEAPON("elven short sword", "runed short sword",
	0, 0, 0,  2, 30, 10,  7,  7, 2, P,   P_SHORT_SWORD, WOOD, HI_WOOD),
WEAPON("droven short sword", "obsidian short sword",
	0, 0, 0,  0, 30, 10,  9,  9, 2, P,   P_SHORT_SWORD, GLASS, CLR_BLACK),

WEAPON("orcish short sword", "crude short sword",
	0, 0, 0,  3, 30, 10,  5,  10, -1, P,   P_SHORT_SWORD, IRON, CLR_BLACK),
WEAPON("dwarvish short sword", "broad short sword",
	0, 0, 0,  2, 30, 10,  8,  7, 0, P,   P_SHORT_SWORD, IRON, HI_METAL),
WEAPON("scimitar", "curved sword",
	0, 0, 0, 15, 40, 15,  8,  8, 0, S,   P_SCIMITAR, IRON, HI_METAL),
WEAPON("rapier", (char *)0,
	1, 0, 0,  6, 40, 20,  6,  4, 2, P,   P_SABER, METAL, HI_METAL),
WEAPON("silver saber", (char *)0,
	1, 0, 0,  6, 40, 75,  8,  8, 0, S,   P_SABER, SILVER, HI_SILVER),
WEAPON("broadsword", (char *)0,
	1, 0, 0,  8, 70, 10,  4,  6, 0, S,   P_BROAD_SWORD, IRON, HI_METAL),
						/* +d4 small, +1 large */
WEAPON("elven broadsword", "runed broadsword",
	0, 0, 0,  4, 70, 10,  6,  6, 2, S,   P_BROAD_SWORD, WOOD, HI_WOOD),
						/* +d4 small, +2 large */
WEAPON("long sword", (char *)0,
	1, 0, 0, 46, 40, 15,  8, 12, 0, S,   P_LONG_SWORD, IRON, HI_METAL),
WEAPON("crystal sword", (char *)0,
	1, 0, 0, 2, 120,300,  8, 12, 0, S,   P_LONG_SWORD, GLASS, CLR_CYAN),
WEAPON("two-handed sword", (char *)0,
	1, 0, 1, 22,150, 50, 12,  6, 0, S,   P_TWO_HANDED_SWORD, IRON, HI_METAL),
						/* +2d6 large */
WEAPON("droven greatsword", "large obsidian sword",
	0, 0, 1,  0,150, 50,  18, 30, 2, S,   P_TWO_HANDED_SWORD, GLASS, CLR_BLACK),
WEAPON("katana", "samurai sword",
	0, 0, 0,  4, 40, 80, 10, 12, 1, S,   P_LONG_SWORD, IRON, HI_METAL),
/* special swords set up for artifacts */
WEAPON("tsurugi", "long samurai sword",
	0, 0, 1,  0, 60,500, 16,  8, 2, S,   P_TWO_HANDED_SWORD, METAL, HI_METAL),
						/* +2d6 large */
WEAPON("runesword", "runed black blade",
	0, 0, 0,  0, 40,300,  10,  10, 0, S,   P_BROAD_SWORD, IRON, CLR_BLACK),
						/* +d4 small, +1 large */
						/* +5d2 +d8 from level drain */

/* polearms */
/* spear-type */
WEAPON("partisan", "vulgar polearm",
	0, 0, 1,  5, 80, 10,  6,  6, 0, P,   P_POLEARMS, IRON, HI_METAL),
						/* +1 large */
WEAPON("ranseur", "hilted polearm",
	0, 0, 1,  5, 50,  6,  4,  4, 0, P,   P_POLEARMS, IRON, HI_METAL),
						/* +d4 both */
WEAPON("spetum", "forked polearm",
	0, 0, 1,  5, 50,  5,  6,  6, 0, P,   P_POLEARMS, IRON, HI_METAL),
						/* +1 small, +d6 large */
WEAPON("glaive", "single-edged polearm",
	0, 0, 1,  8, 75,  6,  6, 10, 0, S,   P_POLEARMS, IRON, HI_METAL),
WEAPON("lance", (char *)0,
	1, 0, 0,  4,180, 10,  6,  8, 0, P,   P_LANCE, IRON, HI_METAL),
WEAPON("elven lance", (char *)0,
	1, 0, 0,  0,140, 10,  8,  8, 2, P,   P_LANCE, WOOD, HI_WOOD),
WEAPON("droven lance", "obsidian lance",
	0, 0, 1,  0,200, 10, 10, 10, 2, P,   P_LANCE, GLASS, CLR_BLACK),
/* axe-type */
WEAPON("halberd", "angled poleaxe",
	0, 0, 1,  8,150, 10, 10,  6, 0, P|S, P_POLEARMS, IRON, HI_METAL),
						/* +1d6 large */
WEAPON("bardiche", "long poleaxe",
	0, 0, 1,  4,120,  7,  4,  4, 0, S,   P_POLEARMS, IRON, HI_METAL),
						/* +1d4 small, +2d4 large */
WEAPON("voulge", "pole cleaver",
	0, 0, 1,  4,125,  5,  4,  4, 0, S,   P_POLEARMS, IRON, HI_METAL),
						/* +d4 both */
WEAPON("dwarvish mattock", "broad pick",
	0, 0, 1, 13,120, 50, 12,  8,-1, P|B,   P_PICK_AXE, IRON, HI_METAL),

/* curved/hooked */
WEAPON("fauchard", "pole sickle",
	0, 0, 1,  5, 60,  5,  6,  8, 0, P|S, P_POLEARMS, IRON, HI_METAL),
WEAPON("guisarme", "pruning hook",
	0, 0, 1,  5, 80,  5,  4,  8, 0, S,   P_POLEARMS, IRON, HI_METAL),
						/* +1d4 small */
WEAPON("bill-guisarme", "hooked polearm",
	0, 0, 1,  3,120,  7,  4, 10, 0, P|S, P_POLEARMS, IRON, HI_METAL),
						/* +1d4 small */
/* other */
WEAPON("lucern hammer", "pronged polearm",
	0, 0, 1,  4,150,  7,  4,  6, 0, B|P, P_POLEARMS, IRON, HI_METAL),
						/* +1d4 small */
WEAPON("bec de corbin", "beaked polearm",
	0, 0, 1,  3,100,  8,  8,  6, 0, B|P, P_POLEARMS, IRON, HI_METAL),

WEAPON("scythe", (char *)0, /* 2d4 damage. Vs plants: +6 to hit and double damage */
	1, 0, 1, 5, 100,  6,  4,  4, -2, S,   P_HARVEST, IRON, HI_METAL),

/* bludgeons */
WEAPON("mace", (char *)0,
	1, 0, 0, 40, 30,  5,  6,  6, 0, B,   P_MACE, IRON, HI_METAL),
						/* +1 small */
WEAPON("elven mace", (char *)0,
	1, 0, 0,  0, 30,  5,  7,  7, 2, B,   P_MACE, WOOD, HI_WOOD),
						/* +1 small */
WEAPON("morning star", (char *)0,
	1, 0, 0, 12,120, 10,  4,  6, 0, B,   P_MORNING_STAR, IRON, HI_METAL),
						/* +d4 small, +1 large */
WEAPON("war hammer", (char *)0,
	1, 0, 0, 15, 50,  5,  4,  4, 0, B,   P_HAMMER, IRON, HI_METAL),
						/* +1 small */
WEAPON("club", (char *)0,
	1, 0, 0, 11, 30,  3,  6,  3, 0, B,   P_CLUB, WOOD, HI_WOOD),
WEAPON("quarterstaff", "staff",
	0, 0, 1, 10, 40,  5,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD),
WEAPON("silver khakkhara", "silver monk's staff",
	0, 0, 1,  2, 120,  5,  6,  4, 0, B|P,   P_QUARTERSTAFF, SILVER, HI_SILVER),
WEAPON("iron bar", (char *)0,
	1, 1, 1, 0, 400,  10,  8,  6, -10, B,   P_QUARTERSTAFF, IRON, HI_METAL),
/* two-piece */
WEAPON("aklys", "thonged club",
	0, 0, 0,  8, 15,  4,  6,  3, 0, B,   P_CLUB, IRON, HI_METAL),
WEAPON("flail", (char *)0,
	1, 0, 0, 40, 15,  4,  6,  4, 0, B,   P_FLAIL, IRON, HI_METAL),
						/* +1 small, +1d4 large */
/* misc */
WEAPON("bullwhip", (char *)0,
	1, 0, 0,  2, 20,  4,  2,  1, 0, 0,   P_WHIP, LEATHER, CLR_BROWN),

/* bows */
BOW("bow", (char *)0,		1, 24, 30, 60, 0, WOOD, P_BOW, HI_WOOD),
BOW("elven bow", "runed bow",	0, 12, 30, 60, 2, WOOD, P_BOW, HI_WOOD),
BOW("orcish bow", "crude bow",	0, 12, 30, 60, -2, WOOD, P_BOW, CLR_BLACK),
BOW("yumi", "long bow",		0,  0, 30, 60, 0, WOOD, P_BOW, HI_WOOD),
BOW("sling", (char *)0,		1, 40,  3, 20, -1, LEATHER, P_SLING, HI_LEATHER),
BOW("crossbow", (char *)0,	1, 45, 50, 40, 1, WOOD, P_CROSSBOW, HI_WOOD),
BOW("droven crossbow", "spider-legged crossbow",	0, 0, 50, 120, 4, SILVER, P_CROSSBOW, CLR_BLACK),

#undef P
#undef S
#undef B

#undef WEAPON
#undef PROJECTILE
#undef BOW

/* armor ... */
/* IRON denotes ferrous metals, including steel.
 * Only IRON weapons and armor can rust.
 * Only COPPER (including brass) corrodes.
 * Some creatures are vulnerable to SILVER.
 */
#define ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,sub,metal,c) \
	OBJECT( \
		OBJ(name,desc), BITS(kn,0,1,0,mgc,1,0,0,blk,0,0,sub,metal), power, \
		ARMOR_CLASS, prob, delay, wt, cost, \
		0, 0, 10 - ac, can, wt, c )
#define HELM(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c) \
	ARMOR(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,can,ARM_HELM,metal,c)
#define CLOAK(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c) \
	ARMOR(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,can,ARM_CLOAK,metal,c)
#define SHIELD(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,metal,c) \
	ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,ARM_SHIELD,metal,c)
#define GLOVES(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c) \
	ARMOR(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,can,ARM_GLOVES,metal,c)
#define BOOTS(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c) \
	ARMOR(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,can,ARM_BOOTS,metal,c)

/* helmets */
HELM("sedge hat", "wide conical hat",
		0, 0,  0,	3, 1,  3,   8, 9, 0, VEGGY, CLR_YELLOW),
HELM("elven leather helm", "leather hat",
		0, 0,  0,	5, 1,  3,   8, 9, 0, LEATHER, HI_LEATHER),
HELM("orcish helm", "iron skull cap",
		0, 0,  0,	5, 1, 30,  10, 9, 0, IRON, CLR_BLACK),
HELM("dwarvish iron helm", "hard hat",
		0, 0,  0,	5, 1, 40,  20, 8, 0, IRON, HI_METAL),
HELM("gnomish pointy hat", "conical hat",
		0, 0,  0,	0, 1, 10,  2, 10, 0, CLOTH, CLR_RED),
HELM("fedora", (char *)0,
		1, 0,  0,	0, 0,  3,   1,10, 0, CLOTH, CLR_BROWN),
HELM("cornuthaum", "conical hat",
		0, 1,  CLAIRVOYANT,
				3, 1,  4,  80,10, 2, CLOTH, CLR_BLUE),
HELM("dunce cap", "conical hat",
		0, 1,  0,	3, 1,  4,   1,10, 0, CLOTH, CLR_BLUE),
HELM("flack helmet", "green bowel",
		1, 0,  0,	2, 0, 10,   20, 8, 1, PLASTIC, CLR_GREEN),
HELM("droven helm", "spider shaped helm",
		1, 0,  0,	0, 0, 50,   5, 7, 0, MINERAL, CLR_BLACK),
HELM("crystal helm", "fish bowl",
		0, 0,  0,   0, 1,150, 300, 6, 1, GLASS, CLR_CYAN),
/* With shuffled appearances... */
HELM("helmet", "plumed helmet", /* circlet */
		0, 0,  0,      8, 1, 30,  10, 9, 0, IRON, CLR_RED),
HELM("helm of brilliance", "etched helmet", /* crown of cognizance */
		0, 1,  0,	6, 1, 50,  50, 9, 0, IRON, CLR_BRIGHT_GREEN),
HELM("helm of opposite alignment", "crested helmet", /* tiara of treachery */
		0, 1,  0,	6, 1, 50,  50, 9, 0, IRON, HI_METAL),
HELM("helm of telepathy", "visored helmet", /*tiara of telepathy*/
		0, 1,  TELEPAT, 2, 1, 50,  50, 9, 0, IRON, HI_METAL),
HELM("helm of drain resistance", "gold circlet", /*diadem of drain resistance*/
		0, 1,  DRAIN_RES, 2, 1, 50,  50, 9, 0, GOLD, HI_GOLD),

/* suits of armor */
/*
 * There is code in polyself.c that assumes (1) and (2).
 * There is code in obj.h, objnam.c, mon.c, read.c that assumes (2).
 *
 *	(1) The dragon scale mails and the dragon scales are together.
 *	(2) That the order of the dragon scale mail and dragon scales is the
 *	    the same defined in monst.c.
 */
#define DRGN_ARMR(name,mgc,power,cost,ac,color) \
	ARMOR(name,(char *)0,1,mgc,1,power,0,5,150,cost,ac,0,ARM_SUIT,DRAGON_HIDE,color)
/* 3.4.1: dragon scale mail reclassified as "magic" since magic is
   needed to create them */
DRGN_ARMR("gray dragon scale mail",   1, ANTIMAGIC,  1200, 1, CLR_GRAY),
DRGN_ARMR("silver dragon scale mail", 1, REFLECTING, 1200, 1, DRAGON_SILVER),
DRGN_ARMR("mercurial dragon scale mail", 1, FAST, 1200, 1, CLR_GRAY),
DRGN_ARMR("shimmering dragon scale mail", 1, DISPLACED, 1200, 1, CLR_CYAN),
DRGN_ARMR("deep dragon scale mail",   1, DRAIN_RES,  1200, 1, CLR_MAGENTA),
DRGN_ARMR("red dragon scale mail",    1, FIRE_RES,    900, 1, CLR_RED),
DRGN_ARMR("white dragon scale mail",  1, COLD_RES,    900, 1, CLR_WHITE),
DRGN_ARMR("orange dragon scale mail", 1, FREE_ACTION,   900, 1, CLR_ORANGE),
DRGN_ARMR("black dragon scale mail",  1, DISINT_RES, 1200, 1, CLR_BLACK),
DRGN_ARMR("blue dragon scale mail",   1, SHOCK_RES,   900, 1, CLR_BLUE),
DRGN_ARMR("green dragon scale mail",  1, POISON_RES,  900, 1, CLR_GREEN),
DRGN_ARMR("yellow dragon scale mail", 1, ACID_RES,    900, 1, CLR_YELLOW),

/* For now, only dragons leave these. */
/* 3.4.1: dragon scales left classified as "non-magic"; they confer
   magical properties but are produced "naturally" */
DRGN_ARMR("gray dragon scales",   0, ANTIMAGIC,  700, 7, CLR_GRAY),
DRGN_ARMR("silver dragon scales", 0, REFLECTING, 700, 7, DRAGON_SILVER),
DRGN_ARMR("mercurial dragon scales", 0, FAST,  700, 7, CLR_GRAY),
DRGN_ARMR("shimmering dragon scales", 0, DISPLACED,  700, 7, CLR_CYAN),
DRGN_ARMR("deep dragon scales",   0, DRAIN_RES,  500, 7, CLR_MAGENTA),
DRGN_ARMR("red dragon scales",    0, FIRE_RES,   500, 7, CLR_RED),
DRGN_ARMR("white dragon scales",  0, COLD_RES,   500, 7, CLR_WHITE),
DRGN_ARMR("orange dragon scales", 0, FREE_ACTION,  500, 7, CLR_ORANGE),
DRGN_ARMR("black dragon scales",  0, DISINT_RES, 700, 7, CLR_BLACK),
DRGN_ARMR("blue dragon scales",   0, SHOCK_RES,  500, 7, CLR_BLUE),
DRGN_ARMR("green dragon scales",  0, POISON_RES, 500, 7, CLR_GREEN),
DRGN_ARMR("yellow dragon scales", 0, ACID_RES,   500, 7, CLR_YELLOW),
#undef DRGN_ARMR

ARMOR("plate mail", (char *)0,
	1, 0, 1, 0,	44, 5, 225, 600,  0, 3, ARM_SUIT, IRON, HI_METAL),
ARMOR("droven plate mail", "crested black plate",
	0, 0, 1, 0,	0, 5, 50, 	2000,  0, 1, ARM_SUIT, MINERAL, CLR_BLACK),
ARMOR("elven toga", (char *)0,
	1, 0, 0, 0,	 0, 5,	 5,  100, 9, 2, ARM_SUIT, CLOTH, CLR_GREEN),
ARMOR("noble's dress", "armored black dress",
	0, 0, 1, 0,	0, 5,100, 2000,  2, 3, ARM_SUIT, MINERAL, CLR_BLACK),
ARMOR("black dress", (char *)0,
	1, 0, 1, 0,	0, 5, 50,  500, 10, 2, ARM_SHIRT, CLOTH, CLR_BLACK),
ARMOR("consort's suit", "loud foppish suit",
	0, 0, 1, 0,	0, 5, 10, 	1000,  9, 1, ARM_SUIT, CLOTH, CLR_BRIGHT_MAGENTA),
ARMOR("crystal plate mail", (char *)0,
	1, 0, 1, 0,	10, 5, 250, 2000,  -6, 2, ARM_SUIT, GLASS, CLR_CYAN), /*Best armor, AC wise*/
#ifdef TOURIST
ARMOR("bronze plate mail", (char *)0,
	1, 0, 1, 0,	20, 5, 225, 400,  3, 3, ARM_SUIT, COPPER, HI_COPPER),
#else
ARMOR("bronze plate mail", (char *)0,
	1, 0, 1, 0,	35, 5, 225, 400,  3, 3, ARM_SUIT, COPPER, HI_COPPER),
#endif
ARMOR("splint mail", (char *)0,
	1, 0, 1, 0,	62, 5, 200,  80,  4, 1, ARM_SUIT, IRON, HI_METAL),
ARMOR("banded mail", (char *)0,
	1, 0, 1, 0,	72, 5, 175,  90,  4, 0, ARM_SUIT, IRON, HI_METAL),
ARMOR("dwarvish mithril-coat", (char *)0,
	1, 0, 0, 0,	10, 1, 40, 240,  4, 3, ARM_SUIT, MITHRIL, HI_METAL),
ARMOR("elven mithril-coat", (char *)0,
	1, 0, 0, 0,	15, 1, 20, 240,  5, 3, ARM_SUIT, MITHRIL, HI_METAL),
ARMOR("chain mail", (char *)0,
	1, 0, 0, 0,	72, 5, 150,  75,  5, 1, ARM_SUIT, IRON, HI_METAL),
ARMOR("droven chain mail", "crested black mail",
	0, 0, 0, 0,	0, 5, 50,  1000,  5, 2, ARM_SUIT, MINERAL, HI_METAL),
ARMOR("orcish chain mail", "crude chain mail",
	0, 0, 0, 0,	20, 5, 150,  75,  6, 1, ARM_SUIT, IRON, CLR_BLACK),
ARMOR("scale mail", (char *)0,
	1, 0, 0, 0,	72, 5, 125,  45,  6, 0, ARM_SUIT, IRON, HI_METAL),
ARMOR("studded leather armor", (char *)0,
	1, 0, 0, 0,	72, 3, 100,  15,  7, 1, ARM_SUIT, LEATHER, HI_LEATHER),
ARMOR("ring mail", (char *)0,
	1, 0, 0, 0,	72, 5, 125, 100,  7, 0, ARM_SUIT, IRON, HI_METAL),
ARMOR("orcish ring mail", "crude ring mail",
	0, 0, 0, 0,	20, 5, 125,  80,  8, 1, ARM_SUIT, IRON, CLR_BLACK),
ARMOR("leather armor", (char *)0,
	1, 0, 0, 0,	82, 3, 75,   5,  8, 0, ARM_SUIT, LEATHER, HI_LEATHER),
ARMOR("leather jacket", (char *)0,
	1, 0, 0, 0,	12, 0,	30,  10,  9, 0, ARM_SUIT, LEATHER, CLR_BLACK),
/*ARMOR("healer uniform", "clean white clothes",
	1, 0, 0, 0,	0, 0,	30,  10,  9, 0, ARM_SUIT, CLOTH, CLR_WHITE),
*/
#ifdef TOURIST
/* shirts */
/*ARMOR("Hawaiian shorts", "flowery shorts and lei",
	1, 0, 0, 0,	 0, 0,	 5,   3, 10, 0, ARM_SUIT, CLOTH, CLR_MAGENTA),
*/
ARMOR("Hawaiian shirt", "flowery shirt",
	0, 0, 0, 0,	 10, 0,	 5,   3, 10, 0, ARM_SHIRT, CLOTH, CLR_MAGENTA),
ARMOR("T-shirt", (char *)0,
	1, 0, 0, 0,	 5, 0,	 5,   2, 10, 0, ARM_SHIRT, CLOTH, CLR_WHITE),
# ifdef CONVICT
ARMOR("striped shirt", (char *)0,
	1, 0, 0, 0,	 0, 0,	 5,   2, 10, 0, ARM_SHIRT, CLOTH, CLR_GRAY),
# endif /* CONVICT */
#endif
/*Ruffled shirts are little different from other shirts*/
ARMOR("ruffled shirt", (char *)0,
	1, 0, 0, 0,	 0, 0,	 5,   2, 10, 0, ARM_SHIRT, CLOTH, CLR_WHITE),
/* victorian underwear, on the other hand, inflicts a penalty to AC but grants MC 3 */
ARMOR("victorian underwear", (char *)0,
	1, 0, 1, 0,	 0, 5,	 5,   2, 10, 3, ARM_SHIRT, CLOTH, CLR_WHITE),
/* cloaks */
/*  'cope' is not a spelling mistake... leave it be */
CLOAK("mummy wrapping", (char *)0,
		1, 0,	0,	    0, 0,  3,  2, 10, 1, CLOTH, CLR_GRAY),
CLOAK("elven cloak", "faded pall",
		0, 1,	STEALTH,    7, 0, 10, 60,  9, 3, CLOTH, CLR_BLACK),
CLOAK("droven cloak", "cobwebbed cloak",
		0, 1,	0,      1, 0, 10, 60,  10, 3, CLOTH, CLR_GRAY),
CLOAK("orcish cloak", "coarse mantelet",
		0, 0,	0,	    8, 0, 10, 40, 10, 2, CLOTH, CLR_BLACK),
CLOAK("dwarvish cloak", "hooded cloak",
		0, 0,	0,	    8, 0, 10, 50, 9, 2, CLOTH, CLR_BLUE),
CLOAK("oilskin cloak", "slippery cloak",
		0, 0,	0,	    8, 0, 10, 50,  9, 3, CLOTH, HI_CLOTH),
CLOAK("robe", (char *)0,
		1, 1,	0,	    3, 0, 15, 50,  8, 3, CLOTH, CLR_RED),
CLOAK("alchemy smock", "apron",
		0, 1,	POISON_RES, 9, 0, 10, 50,  9, 3, CLOTH, CLR_WHITE),
CLOAK("Leo Nemaeus hide", "lion skin",
		0, 1,	HALF_PHDAM,	    0, 10, 60, 1200,  7, 0, DRAGON_HIDE, HI_GOLD),
CLOAK("leather cloak", (char *)0,
		1, 0,	0,	    8, 0, 15, 40,  8, 1, LEATHER, CLR_BROWN),
/* With shuffled appearances... */
CLOAK("cloak of protection", "tattered cape",
		0, 1,	PROTECTION, 9, 0, 10, 50,  7, 3, CLOTH, HI_CLOTH),
CLOAK("cloak of invisibility", "opera cloak",
		0, 1,	INVIS,	   10, 0, 10, 60,  9, 2, CLOTH, CLR_BLACK),
CLOAK("cloak of magic resistance", "ornamental cope",
		0, 1,	ANTIMAGIC,  2, 0, 10, 60,  9, 3, CLOTH, CLR_WHITE),
CLOAK("cloak of displacement", "piece of cloth",
		0, 1,	DISPLACED, 10, 0, 10, 50,  9, 2, CLOTH, CLR_BRIGHT_MAGENTA),

/* shields */
SHIELD("small shield", (char *)0,
		1, 0, 0, 0,	     6, 0, 30,	3,  9, 0, WOOD, HI_WOOD),
SHIELD("elven shield", "blue and green shield",
		0, 0, 0, 0,	     2, 0, 40,	7,  8, 2, WOOD, CLR_GREEN),
SHIELD("Uruk-hai shield", "white-handed shield",
		0, 0, 0, 0,	     2, 0, 50,	7,  8, 1, IRON, HI_METAL),
SHIELD("orcish shield", "red-eyed shield",
		0, 0, 0, 0,	     2, 0, 50,	7,  9, 0, IRON, CLR_RED),
SHIELD("large shield", (char *)0,
		1, 0, 1, 0,	     7, 0,100, 10,  8, 1, IRON, HI_METAL),
SHIELD("dwarvish roundshield", "large round shield",
		0, 0, 0, 0,	     4, 0, 80, 10,  7, 1, IRON, HI_METAL),
SHIELD("crystal shield", "glass shield",
		0, 0, 0, 0,	     0, 0,300,150,  4, 1, GLASS, CLR_CYAN),
SHIELD("shield of reflection", "polished silver shield",
		0, 1, 0, REFLECTING, 3, 0, 50, 50,  8, 0, SILVER, HI_SILVER),
/*#define SHIELD(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,metal,c) \
     ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,ARM_SHIELD,metal,c) */
#define DRGN_SHIELD(name,mgc,power,cost,ac,color)						\
	ARMOR(name,(char *)0,1,mgc,1,power,0,0,75,cost,ac,0,ARM_SHIELD,DRAGON_HIDE,color)
/* 3.4.1: dragon scale mail reclassified as "magic" since magic is
   needed to create them */
DRGN_SHIELD("gray dragon scale shield", 1, ANTIMAGIC,  1200, 7, CLR_GRAY),
DRGN_SHIELD("silver dragon scale shield", 1, REFLECTING, 1200, 7, DRAGON_SILVER),
DRGN_SHIELD("mercurial dragon scale shield", 1, FAST, 1200, 7, CLR_GRAY),
DRGN_SHIELD("shimmering dragon scale shield", 1, DISPLACED, 1200, 7, CLR_CYAN),
DRGN_SHIELD("deep dragon scale shield", 1, DRAIN_RES, 1200, 7, CLR_MAGENTA),
DRGN_SHIELD("red dragon scale shield", 1, FIRE_RES,    900, 7, CLR_RED),
DRGN_SHIELD("white dragon scale shield", 1, COLD_RES,    900, 7, CLR_WHITE),
DRGN_SHIELD("orange dragon scale shield", 1, FREE_ACTION,   900, 7, CLR_ORANGE),
DRGN_SHIELD("black dragon scale shield", 1, DISINT_RES, 1200, 7, CLR_BLACK),
DRGN_SHIELD("blue dragon scale shield", 1, SHOCK_RES,   900, 7, CLR_BLUE),
DRGN_SHIELD("green dragon scale shield", 1, POISON_RES,  900, 7, CLR_GREEN),
DRGN_SHIELD("yellow dragon scale shield", 1, ACID_RES,   900, 7, CLR_YELLOW),

/* gloves */
/* these have their color but not material shuffled, so the IRON must stay
 * CLR_BROWN (== HI_LEATHER)
 */
GLOVES("leather gloves", "old gloves",
		0, 0,  0,	  12, 1, 10,  8,  9, 0, LEATHER, HI_LEATHER),
GLOVES("gauntlets of fumbling", "padded gloves",
		0, 1,  FUMBLING,   7, 1, 10, 50,  9, 0, LEATHER, HI_LEATHER),
GLOVES("gauntlets of power", "riding gloves",
		0, 1,  0,	   7, 1, 30, 50,  9, 0, IRON, CLR_BROWN),
GLOVES("orihalcyon gauntlets", "fighting gloves",
		0, 1,  ANTIMAGIC,  7, 1, 10, 50,  9, 0, METAL, CLR_BROWN),
GLOVES("gauntlets of dexterity", "fencing gloves",
		0, 1,  0,	   7, 1, 10, 50,  9, 0, LEATHER, HI_LEATHER),

/* boots */
BOOTS("low boots", "walking shoes",
		0, 0,  0,	  25, 2, 10,  8,  9, 0, LEATHER, HI_LEATHER),
BOOTS("iron shoes", "hard shoes",
		0, 0,  0,	   7, 2, 50, 16,  8, 0, IRON, HI_METAL),
BOOTS("high boots", "jackboots",
		0, 0,  0,	  15, 2, 20, 12,  8, 0, LEATHER, HI_LEATHER),
BOOTS("crystal boots", "glass boots",
		0, 0,  0,	   0, 2,150,300,  7, 0, GLASS, CLR_CYAN),
/* With shuffled appearances... */
BOOTS("speed boots", "combat boots",
		0, 1,  FAST,	  12, 2, 20, 50,  9, 0, LEATHER, HI_LEATHER),
BOOTS("water walking boots", "jungle boots",
		0, 1,  WWALKING,  12, 2, 20, 50,  9, 0, LEATHER, HI_LEATHER),
BOOTS("jumping boots", "hiking boots",
		0, 1,  JUMPING,   12, 2, 20, 50,  9, 0, LEATHER, HI_LEATHER),
BOOTS("elven boots", "mud boots",
		0, 1,  STEALTH,   12, 2, 15,  8,  9, 0, LEATHER, HI_LEATHER),
BOOTS("kicking boots", "buckled boots",
		0, 1,  0,         12, 2, 15,  8,  9, 0, IRON, CLR_BROWN),
BOOTS("fumble boots", "riding boots",
		0, 1,  FUMBLING,  12, 2, 20, 30,  9, 0, LEATHER, HI_LEATHER),
BOOTS("flying boots", "snow boots",
		0, 1,  FLYING,12, 2, 15, 30,  9, 0, LEATHER, HI_LEATHER),
#undef HELM
#undef CLOAK
#undef SHIELD
#undef GLOVES
#undef BOOTS
#undef ARMOR

/* rings ... */
#define RING(name,power,stone,cost,mgc,spec,mohs,metal,color) OBJECT( \
		OBJ(name,stone), \
		BITS(0,0,spec,0,mgc,spec,0,0,0,HARDGEM(mohs),0,P_NONE,metal), \
		power, RING_CLASS, 0, 0, 3, cost, 0, 0, 0, 0, 15, color )
RING("adornment", ADORNED, "wooden",        100, 1, 1, 2, WOOD, HI_WOOD),
RING("gain strength", 0, "granite",         150, 1, 1, 7, MINERAL, HI_MINERAL),
RING("gain constitution", 0, "opal",        150, 1, 1, 7, MINERAL,  HI_MINERAL),
RING("increase accuracy", 0, "clay",        150, 1, 1, 4, MINERAL, CLR_RED),
RING("increase damage", 0, "coral",         150, 1, 1, 4, MINERAL, CLR_ORANGE),
RING("protection", PROTECTION, "black onyx",100, 1, 1, 7, MINERAL, CLR_BLACK),
RING("regeneration", REGENERATION, "moonstone",
					    200, 1, 0, 6, MINERAL, HI_MINERAL),
RING("searching", SEARCHING, "tiger eye",   200, 1, 0, 6, GEMSTONE, CLR_BROWN),
RING("stealth", STEALTH, "jade",            100, 1, 0, 6, GEMSTONE, CLR_GREEN),
RING("sustain ability", FIXED_ABIL, "bronze",
					    100, 1, 0, 4, COPPER, HI_COPPER),
RING("levitation", LEVITATION, "agate",     200, 1, 0, 7, GEMSTONE, CLR_RED),
RING("hunger", HUNGER, "topaz",             100, 1, 0, 8, GEMSTONE, CLR_CYAN),
RING("aggravate monster", AGGRAVATE_MONSTER, "sapphire",
					    150, 1, 0, 9, GEMSTONE, CLR_BLUE),
RING("conflict", CONFLICT, "ruby",          300, 1, 0, 9, GEMSTONE, CLR_RED),
RING("warning", WARNING, "diamond",         100, 1, 0,10, GEMSTONE, CLR_WHITE),
RING("poison resistance", POISON_RES, "pearl",
					    150, 1, 0, 4, IRON, CLR_WHITE),
RING("fire resistance", FIRE_RES, "iron",   200, 1, 0, 5, IRON, HI_METAL),
RING("cold resistance", COLD_RES, "brass",  150, 1, 0, 4, COPPER, HI_COPPER),
RING("shock resistance", SHOCK_RES, "copper",
					    150, 1, 0, 3, COPPER, HI_COPPER),
RING("free action",     FREE_ACTION, "twisted",
					    200, 1, 0, 6, IRON, HI_METAL),
RING("slow digestion",  SLOW_DIGESTION, "steel",
					    200, 1, 0, 8, IRON, HI_METAL),
RING("teleportation", TELEPORT, "silver",   200, 1, 0, 3, SILVER, HI_SILVER),
RING("teleport control", TELEPORT_CONTROL, "gold",
					    300, 1, 0, 3, GOLD, HI_GOLD),
RING("polymorph", POLYMORPH, "ivory",       300, 1, 0, 4, BONE, CLR_WHITE),
RING("polymorph control", POLYMORPH_CONTROL, "emerald",
					    300, 1, 0, 8, GEMSTONE, CLR_BRIGHT_GREEN),
RING("invisibility", INVIS, "wire", 150, 1, 0, 5, IRON, HI_METAL),
RING("see invisible", SEE_INVIS, "engagement",
					    150, 1, 0, 5, IRON, HI_METAL),
RING("alacrity", FAST, "shiny", 100, 1, 0, 5, METAL, CLR_BRIGHT_CYAN),
RING("protection from shape changers", PROT_FROM_SHAPE_CHANGERS, "black signet",
					    100, 1, 0, 5, MITHRIL, CLR_BLACK),
#undef RING

/* amulets ... - THE Amulet comes last because it is special */
#define AMULET(name,desc,power,prob) OBJECT( \
		OBJ(name,desc), BITS(0,0,0,0,1,0,0,0,0,0,0,P_NONE,IRON), power, \
		AMULET_CLASS, prob, 0, 20, 150, 0, 0, 0, 0, 20, HI_METAL )

AMULET("amulet of drain resistance","warped",   DRAIN_RES,   60),
AMULET("amulet of ESP",           "circular",   TELEPAT,    140),
AMULET("amulet of life saving",   "spherical",  LIFESAVED,   70),
AMULET("amulet of strangulation", "oval",       STRANGLED,  135),
AMULET("amulet of restful sleep", "triangular", SLEEPING,   135),
AMULET("amulet versus poison",    "pyramidal",  POISON_RES, 155),
AMULET("amulet of change",        "square",     0,          130),
						/* POLYMORPH */
AMULET("amulet of unchanging",    "concave",    UNCHANGING,	 45),
AMULET("amulet of reflection",    "hexagonal",  REFLECTING,  70),
AMULET("amulet of magical breathing", "octagonal",      MAGICAL_BREATHING, 60),
OBJECT(OBJ("cheap plastic imitation of the Amulet of Yendor",
	"Amulet of Yendor"), BITS(0,0,1,0,0,0,0,0,0,0,0,0,PLASTIC), 0,
	AMULET_CLASS, 0, 0, 20,    0, 0, 0, 0, 0,  1, HI_METAL),
OBJECT(OBJ("Amulet of Yendor",	/* note: description == name */
	"Amulet of Yendor"), BITS(0,0,1,0,1,0,1,1,0,0,0,0,MITHRIL), 0,
	AMULET_CLASS, 0, 0, 20, 30000, 0, 0, 0, 0, 20, HI_METAL),
#undef AMULET

/* tools ... */
/* tools with weapon characteristics come last */
#define TOOLMASK(name,desc,prop,kn,mrg,mgc,chg,prob,wt,cost,mat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,mrg,chg,0,mgc,chg,0,0,0,0,0,P_NONE,mat), \
		prop, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, wt, color )
#define TOOL(name,desc,kn,mrg,mgc,chg,prob,wt,cost,mat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,mrg,chg,0,mgc,chg,0,0,0,0,0,P_NONE,mat), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, wt, color )
#define CONTAINER(name,desc,kn,mgc,chg,prob,wt,cost,mat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,0,chg,1,mgc,chg,0,0,0,0,0,P_NONE,mat), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, wt, color )
#define WEPTOOL(name,desc,kn,mgc,chg,bi,prob,wt,cost,sdam,ldam,hitbon,typ,sub,mat,clr) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,0,1,chg,mgc,1,0,0,bi,0,typ,sub,mat), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, 0, wt, clr )
/* containers */
CONTAINER("large box", (char *)0,       1, 0, 0,  40,350,   8, WOOD, HI_WOOD),
CONTAINER("chest", (char *)0,           1, 0, 0,  35,600,  16, WOOD, HI_WOOD),
CONTAINER("ice box", (char *)0,         1, 0, 0,   5,900,  42, PLASTIC, CLR_WHITE),
CONTAINER("sack", "bag",                0, 0, 0,  35, 15,   2, CLOTH, HI_CLOTH),
CONTAINER("oilskin sack", "bag",        0, 0, 0,   5, 15, 100, CLOTH, HI_CLOTH),
CONTAINER("bag of holding", "bag",      0, 1, 0,  20, 15, 100, CLOTH, HI_CLOTH),
CONTAINER("bag of tricks", "bag",       0, 1, 1,  20, 15, 100, CLOTH, HI_CLOTH),
#define HOSTAGE(name,desc,kn,mgc,chg,prob,ntrtn,wt,cost,mat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,0,chg,1,mgc,chg,0,0,0,0,0,P_NONE,mat), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, ntrtn, wt, color )

//HOSTAGE("distressed princess", (char *)0,           1, 0, 0,  0, 350,.9*1450,  1600, CLOTH, CLR_WHITE),
#undef HOSTAGE
#undef CONTAINER
//OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,nut,color) \
//	{0, 0, (char *)0, bits, prp, sym, dly, COLOR_FIELD(color) \
//	 prob, wt, cost, sdam, ldam, oc1, oc2, nut}

/* lock opening tools */
TOOL("skeleton key", "key",     0, 0, 0, 0,  80,  3,  10, IRON, HI_METAL),
TOOL("universal key", "key",     0, 0, 0, 0,  0,  3,  10, SILVER, HI_SILVER),
#ifdef TOURIST
TOOL("lock pick", (char *)0,    1, 0, 0, 0,  60,  4,  20, IRON, HI_METAL),
TOOL("credit card", (char *)0,  1, 0, 0, 0,  15,  1,  10, PLASTIC, CLR_WHITE),
#else
TOOL("lock pick", (char *)0,    1, 0, 0, 0,  75,  4,  20, IRON, HI_METAL),
#endif
/* light sources */
TOOL("tallow candle", "candle", 0, 1, 0, 0,  20,  2,  10, WAX, CLR_WHITE),
TOOL("wax candle", "candle",    0, 1, 0, 0,   5,  2,  20, WAX, CLR_WHITE),
TOOL("brass lantern", (char *)0,1, 0, 0, 0,  30, 30,  12, COPPER, CLR_YELLOW),
TOOL("oil lamp", "lamp",        0, 0, 0, 0,  45, 20,  10, COPPER, CLR_YELLOW),
TOOL("magic lamp", "lamp",      0, 0, 1, 0,  15, 20,  50, COPPER, CLR_YELLOW),
/* other tools */
#ifdef TOURIST
TOOL("expensive camera", (char *)0,
				1, 0, 0, 1,  15, 12, 200, PLASTIC, CLR_BLACK),
TOOL("mirror", "looking glass", 0, 0, 0, 0,  45, 13,  10, GLASS, HI_SILVER),
#else
TOOL("mirror", "looking glass", 0, 0, 0, 0,  60, 13,  10, GLASS, HI_SILVER),
#endif
TOOL("crystal ball", "glass orb",
								0, 0, 1, 1,  15,150, 200, GLASS, HI_GLASS),
TOOL("mask", (char *)0,			1, 0, 0, 0,  10, 10,  80, LEATHER, CLR_WHITE),
TOOLMASK("R'lyehian faceplate", "ebon pane", POISON_RES,
								0, 0, 1, 0,   0, 15, 200, GLASS, CLR_BLACK),
TOOL("lenses", (char *)0,		1, 0, 0, 0,   5,  3,  80, GLASS, HI_GLASS),
TOOL("blindfold", (char *)0,    1, 0, 0, 0,  50,  2,  20, CLOTH, CLR_GRAY),
TOOL("towel", (char *)0,        1, 0, 0, 0,  50,  2,  50, CLOTH, CLR_MAGENTA),
#ifdef STEED
TOOL("saddle", (char *)0,       1, 0, 0, 0,   5,200, 150, LEATHER, HI_LEATHER),
TOOL("leash", (char *)0,        1, 0, 0, 0,  65, 12,  20, LEATHER, HI_LEATHER),
#else
TOOL("leash", (char *)0,        1, 0, 0, 0,  70, 12,  20, LEATHER, HI_LEATHER),
#endif
TOOL("stethoscope", (char *)0,  1, 0, 0, 0,  25,  4,  75, IRON, HI_METAL),
TOOL("tinning kit", (char *)0,  1, 0, 0, 1,  15,100,  30, IRON, HI_METAL),
TOOL("upgrade kit", (char *)0,  1, 0, 0, 0,   0,100,  30, COPPER, HI_COPPER),
TOOL("tin opener", (char *)0,   1, 0, 0, 0,  35,  4,  30, IRON, HI_METAL),
TOOL("can of grease", (char *)0,1, 0, 0, 1,  15, 15,  20, IRON, HI_METAL),
TOOL("figurine", (char *)0,     1, 0, 1, 0,  25, 50,  80, MINERAL, HI_MINERAL),
TOOL("magic marker", (char *)0, 1, 0, 1, 1,  15,  2,  50, PLASTIC, CLR_RED),
/* traps */
TOOL("land mine",(char *)0,     1, 0, 0, 0,   0,300, 180, IRON, CLR_RED),
TOOL("beartrap", (char *)0,     1, 0, 0, 0,   0,200,  60, IRON, HI_METAL),
/* instruments */
/* some code in invent.c requires wooden flute .. drum of earthquake to be
   consecutive, with the wooden flute first and drum of earthquake last */
TOOL("tin whistle", "whistle",  0, 0, 0, 0,  90,  3,  10, METAL, HI_METAL),
TOOL("magic whistle", "whistle",0, 0, 1, 0,  30,  3,  10, METAL, HI_METAL),
/* "If tin whistles are made out of tin, what do they make foghorns out of?" */
TOOL("wooden flute", "flute",   0, 0, 0, 0,   4,  5,  12, WOOD, HI_WOOD),
TOOL("magic flute", "flute",    0, 0, 1, 1,   2,  5,  36, WOOD, HI_WOOD),
TOOL("tooled horn", "horn",     0, 0, 0, 0,   5, 18,  15, BONE, CLR_WHITE),
TOOL("frost horn", "horn",      0, 0, 1, 1,   2, 18,  50, BONE, CLR_WHITE),
TOOL("fire horn", "horn",       0, 0, 1, 1,   2, 18,  50, BONE, CLR_WHITE),
TOOL("horn of plenty", "horn",  0, 0, 1, 1,   2, 18,  50, BONE, CLR_WHITE),
TOOL("wooden harp", "harp",     0, 0, 0, 0,   4, 30,  50, WOOD, HI_WOOD),
TOOL("magic harp", "harp",      0, 0, 1, 1,   2, 30,  50, WOOD, HI_WOOD),
TOOL("bell", (char *)0,         1, 0, 0, 0,   2, 30,  50, COPPER, HI_COPPER),
TOOL("bugle", (char *)0,        1, 0, 0, 0,   4, 10,  15, COPPER, HI_COPPER),
TOOL("leather drum", "drum",    0, 0, 0, 0,   4, 25,  25, LEATHER, HI_LEATHER),
TOOL("drum of earthquake", "drum",
				0, 0, 1, 1,   2, 25,  25, LEATHER, HI_LEATHER),
/* tools useful as weapons */
WEPTOOL("pick-axe", (char *)0,
	1, 0, 0, 0, 20, 80,   50,  6,  3, 0, WHACK,  P_PICK_AXE, IRON, HI_METAL),
/* 
 * Lightsabers get 3x dice when lit 
 */
WEPTOOL("lightsaber", "silver sword hilt",
	0, 1, 1, 0,  0, 12, 500, 8,  8, -3, SLASH, P_SABER, SILVER, HI_SILVER),
WEPTOOL("beamsword",  "gold sword hilt",
	0, 1, 1, 0,  0, 20, 500, 10,10, -3, SLASH, P_BROAD_SWORD, GOLD, HI_GOLD),
WEPTOOL("double lightsaber",  "platinum grip",
	0, 1, 1, 1,  0, 24,1000, 10,10,  -6, SLASH, P_QUARTERSTAFF, PLATINUM, HI_METAL),
WEPTOOL("grappling hook", "iron hook",
	0, 0, 0, 0,  5,  30,  50,  2,  6, 0, WHACK,  P_FLAIL, IRON, HI_METAL),
/* 3.4.1: unicorn horn left classified as "magic" */
WEPTOOL("unicorn horn", (char *)0,
	1, 1, 0, 0,  0,  20, 100, 12, 12, 0, PIERCE, P_UNICORN_HORN, BONE, CLR_WHITE),
WEPTOOL("spoon", (char *)0,
	1, 0, 0, 0,  0,  1,    1,  1,  1, 0, PIERCE,  P_KNIFE, IRON, HI_METAL),

/* two special unique artifact "tools" */
OBJECT(OBJ("Candelabrum of Invocation", "candelabrum"),
		BITS(0,0,1,0,1,0,1,1,0,0,0,P_NONE,GOLD), 0,
		TOOL_CLASS, 0, 0,10, 5000, 0, 0, 0, 0, 200, HI_GOLD),
OBJECT(OBJ("Bell of Opening", "silver bell"),
		BITS(0,0,1,0,1,1,1,1,0,0,0,P_NONE,SILVER), 0,
		TOOL_CLASS, 0, 0,10, 5000, 0, 0, 0, 0, 50, HI_SILVER),
#undef TOOL
#undef WEPTOOL

/* Comestibles ... */
#define FOOD(name,prob,delay,wt,unk,tin,nutrition,color) OBJECT( \
		OBJ(name,(char *)0), BITS(1,1,unk,0,0,0,0,0,0,0,0,P_NONE,tin), 0, \
		FOOD_CLASS, prob, delay, \
		wt, nutrition/20 + 5, 0, 0, 0, 0, nutrition, color )
/* all types of food (except tins & corpses) must have a delay of at least 1. */
/* delay on corpses is computed and is weight dependant */
/* dog eats foods 0-4 but prefers tripe rations above all others */
/* fortune cookies can be read */
/* carrots improve your vision */
/* +0 tins contain monster meat */
/* +1 tins (of spinach) make you stronger (like Popeye) */
/* food CORPSE is a cadaver of some type */
/* meatballs/sticks/rings are only created from objects via stone to flesh */

/* meat */
FOOD("tripe ration",       140, 2, 10, 0, FLESH, 200, CLR_BROWN),
FOOD("corpse",               0, 1,  0, 0, FLESH,   0, CLR_BROWN),
FOOD("egg",                 85, 1,  1, 1, FLESH,  80, CLR_WHITE),
FOOD("meatball",             0, 1,  1, 0, FLESH,   5, CLR_BROWN),
FOOD("meat stick",           0, 1,  1, 0, FLESH,   5, CLR_BROWN),
FOOD("huge chunk of meat",   0,20,400, 0, FLESH,2000, CLR_BROWN),
/* special case because it's not mergable */
OBJECT(OBJ("meat ring", (char *)0),
    BITS(1,0,0,0,0,0,0,0,0,0,0,0,FLESH),
    0, FOOD_CLASS, 0, 1, 5, 1, 0, 0, 0, 0, 5, CLR_BROWN),

/* fruits & veggies */
FOOD("kelp frond",           0, 1,  1, 0, VEGGY,  30, CLR_GREEN),
FOOD("eucalyptus leaf",      3, 1,  1, 0, VEGGY,  30, CLR_GREEN),
/*Forbidden by Eve starts here:*/
FOOD("apple",               15, 1,  2, 0, VEGGY,  50, CLR_RED),
FOOD("orange",              10, 1,  2, 0, VEGGY,  80, CLR_ORANGE),
FOOD("pear",                10, 1,  2, 0, VEGGY,  50, CLR_BRIGHT_GREEN),
FOOD("melon",               10, 1,  5, 0, VEGGY, 100, CLR_BRIGHT_GREEN),
FOOD("banana",              10, 1,  2, 0, VEGGY,  80, CLR_YELLOW),
FOOD("carrot",              15, 1,  2, 0, VEGGY,  50, CLR_ORANGE),
/*:and ends here*/
FOOD("sprig of wolfsbane",   7, 1,  1, 0, VEGGY,  40, CLR_GREEN),
FOOD("clove of garlic",      7, 1,  1, 0, VEGGY,  40, CLR_WHITE),
FOOD("slime mold",          75, 1,  5, 0, VEGGY, 250, HI_ORGANIC),

/* people food */
FOOD("lump of royal jelly",  0, 1,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("cream pie",           25, 1, 10, 0, VEGGY, 100, CLR_WHITE),
FOOD("candy bar",           13, 1,  2, 0, VEGGY, 100, CLR_BROWN),
FOOD("fortune cookie",      55, 1,  1, 0, VEGGY,  40, CLR_YELLOW),
FOOD("pancake",             25, 2,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("lembas wafer",        20, 2,  5, 0, VEGGY, 800, CLR_WHITE),
FOOD("cram ration",         20, 3, 15, 0, VEGGY, 600, HI_ORGANIC),
FOOD("food ration",        380, 5, 20, 0, VEGGY, 800, HI_ORGANIC),
FOOD("K-ration",             0, 1, 10, 0, VEGGY, 400, HI_ORGANIC),
FOOD("C-ration",             0, 1, 10, 0, VEGGY, 300, HI_ORGANIC),
FOOD("tin",                 75, 0, 10, 1, METAL,   0, HI_METAL),
#undef FOOD

/* potions ... */
#define POTION(name,desc,mgc,power,prob,cost,color) OBJECT( \
		OBJ(name,desc), BITS(0,1,0,0,mgc,0,0,0,0,0,0,P_NONE,GLASS), power, \
		POTION_CLASS, prob, 0, 20, cost, 0, 0, 0, 0, 10, color )
POTION("gain ability", "ruby",          1, 0,          40, 300, CLR_RED),
POTION("restore ability", "pink",       1, 0,          40, 100, CLR_BRIGHT_MAGENTA),
POTION("confusion", "orange",           1, CONFUSION,  40, 100, CLR_ORANGE),
POTION("blindness", "yellow",           1, BLINDED,    40, 150, CLR_YELLOW),
POTION("paralysis", "emerald",          1, 0,          40, 300, CLR_BRIGHT_GREEN),
POTION("speed", "dark green",           1, FAST,       40, 200, CLR_GREEN),
POTION("levitation", "cyan",            1, LEVITATION, 40, 200, CLR_CYAN),
POTION("hallucination", "sky blue",     1, HALLUC,     40, 100, CLR_CYAN),
POTION("invisibility", "brilliant blue",1, INVIS,      40, 150, CLR_BRIGHT_BLUE),
POTION("see invisible", "magenta",      1, SEE_INVIS,  40,  50, CLR_MAGENTA),
POTION("healing", "purple-red",         1, 0,          57, 100, CLR_MAGENTA),
POTION("extra healing", "puce",         1, 0,          47, 100, CLR_RED),
POTION("gain level", "milky",           1, 0,          20, 300, CLR_WHITE),
POTION("enlightenment", "swirly",       1, 0,          20, 200, CLR_BROWN),
POTION("monster detection", "bubbly",   1, 0,          40, 150, CLR_WHITE),
POTION("object detection", "smoky",     1, 0,          42, 150, CLR_GRAY),
POTION("gain energy", "cloudy",         1, 0,          40, 150, CLR_WHITE),
POTION("sleeping",  "effervescent",     1, 0,          40, 100, CLR_GRAY),
POTION("full healing",  "black",        1, 0,          10, 200, CLR_BLACK),
POTION("polymorph", "golden",           1, 0,          10, 200, CLR_YELLOW),
POTION("booze", "brown",                0, 0,          42,  50, CLR_BROWN),
POTION("sickness", "fizzy",             0, 0,          40,  50, CLR_CYAN),
POTION("fruit juice", "dark",           0, 0,          42,  50, CLR_BLACK),
POTION("acid", "white",                 0, 0,          10, 250, CLR_WHITE),
POTION("oil", "murky",                  0, 0,          30, 250, CLR_BROWN),
POTION("amnesia", "sparkling",          1, 0,          8, 100, CLR_CYAN),
POTION("water", "clear",                0, 0,          84, 100, CLR_CYAN),
POTION("blood", "blood-red",            0, 0,           18, 50, CLR_RED),
#undef POTION

/* scrolls ... */
#define SCROLL(name,text,mgc,prob,cost) OBJECT( \
		OBJ(name,text), BITS(0,1,0,0,mgc,0,0,0,0,0,0,P_NONE,PAPER), 0, \
		SCROLL_CLASS, prob, 0, 5, cost, 0, 0, 0, 0, 6, HI_PAPER )
	SCROLL("enchant armor",         "ZELGO MER",            1,  59,  80),
	SCROLL("destroy armor",         "JUYED AWK YACC",       1,  41, 100),
	SCROLL("confuse monster",       "NR 9",                 1,  49, 100),
	SCROLL("scare monster",         "XIXAXA XOXAXA XUXAXA", 1,  32, 100),
	SCROLL("remove curse",          "PRATYAVAYAH",          1,  60,  80),
	SCROLL("enchant weapon",        "DAIYEN FOOELS",        1,  74,  60),
	SCROLL("create monster",        "LEP GEX VEN ZEA",      1,  41, 200),
	SCROLL("taming",                "PRIRUTSENIE",          1,  14, 200),
	SCROLL("genocide",              "ELBIB YLOH",           1,  13, 300),
	SCROLL("light",                 "VERR YED HORRE",       1,  84,  50),
	SCROLL("teleportation",         "VENZAR BORGAVVE",      1,  51, 100),
	SCROLL("gold detection",        "THARR",                1,  30, 100),
	SCROLL("food detection",        "YUM YUM",              1,  23, 100),
	SCROLL("identify",              "KERNOD WEL",           1, 169,  20),
	SCROLL("magic mapping",         "ELAM EBOW",            1,  41, 100),
	SCROLL("amnesia",               "DUAM XNAHT",           1,  32, 200),
	SCROLL("fire",                  "ANDOVA BEGARIN",       1,  27, 100),
	SCROLL("earth",                 "KIRJE",                1,  16, 200),
	SCROLL("punishment",            "VE FORBRYDERNE",       1,  14, 300),
	SCROLL("charging",              "HACKEM MUCHE",         1,  14, 300),
	SCROLL("stinking cloud",		"VELOX NEB",            1,  13, 300),
	SCROLL("ward",					"TLASFO SENIL",         1,  65, 300),
	SCROLL("warding",				"RW NW PRT M HRW",      1,  13, 300),
	SCROLL((char *)0,		"FOOBIE BLETCH",        1,   0, 100),
	SCROLL((char *)0,		"TEMOV",                1,   0, 100),
	SCROLL((char *)0,		"GARVEN DEH",           1,   0, 100),
	SCROLL((char *)0,		"READ ME",              1,   0, 100),
	/* these must come last because they have special descriptions */
#ifdef MAIL
	SCROLL("mail",                  "stamped",          0,   0,   0),
#endif
	SCROLL("blank paper",           "unlabeled",        0,  25,  60),
#undef SCROLL
#define GOLD_SCROLL(name,text,mgc,prob,cost) OBJECT( \
		OBJ(name,text), BITS(0,1,0,0,mgc,0,0,0,0,0,0,P_NONE,GOLD), 0, \
		SCROLL_CLASS, prob, 0, 50, cost, 0, 0, 0, 0, 6, HI_GOLD )
	GOLD_SCROLL("Gold Scroll of Law", "golden",        0,  0,  10), /* Shopkeepers aren't interested in these */
#undef GOLD_SCROLL

/* spellbooks ... */
#define SPELL(name,desc,sub,prob,delay,level,mgc,dir,color) OBJECT( \
		OBJ(name,desc), BITS(0,0,0,0,mgc,0,0,0,0,0,dir,sub,PAPER), 0, \
		SPBOOK_CLASS, prob, delay, \
		50, level*100, 0, 0, 0, level, 20, color )
SPELL("dig",             "parchment",   P_MATTER_SPELL, 20,  6, 5, 1, RAY,       HI_PAPER),
SPELL("magic missile",   "vellum",      P_ATTACK_SPELL, 45,  2, 2, 1, RAY,       HI_PAPER),
SPELL("fireball",        "ragged",      P_ATTACK_SPELL, 20,  4, 4, 1, RAY,       HI_PAPER),
SPELL("cone of cold",    "dog eared",   P_ATTACK_SPELL, 10,  7, 4, 1, RAY,       HI_PAPER),
SPELL("sleep",           "mottled",     P_ENCHANTMENT_SPELL, 50,  1, 1, 1, RAY,  HI_PAPER),
SPELL("finger of death", "stained",     P_ATTACK_SPELL,  5, 10, 7, 1, RAY,       HI_PAPER),
SPELL("lightning bolt",  "storm-hued",  P_ATTACK_SPELL,  0,  8, 5, 1, RAY,       CLR_BLUE),
SPELL("poison spray",  "snakeskin",  	P_MATTER_SPELL,  0,  8, 4, 1, RAY,       CLR_GREEN),
SPELL("acid blast",  "acid green",  	P_MATTER_SPELL,  0,  8, 4, 1, RAY,       CLR_BRIGHT_GREEN),
SPELL("light",           "cloth",       P_DIVINATION_SPELL, 45,  1, 1, 1, NODIR,     HI_CLOTH),
SPELL("detect monsters", "leather",     P_DIVINATION_SPELL, 43,  1, 1, 1, NODIR,     HI_LEATHER),
SPELL("healing",         "white",       P_HEALING_SPELL, 40,  2, 1, 1, IMMEDIATE, CLR_WHITE),
SPELL("knock",           "pink",        P_MATTER_SPELL, 35,  1, 1, 1, IMMEDIATE, CLR_BRIGHT_MAGENTA),
SPELL("force bolt",      "red",         P_ATTACK_SPELL, 35,  2, 1, 1, IMMEDIATE, CLR_RED),
SPELL("confuse monster", "orange",      P_ENCHANTMENT_SPELL, 30,  2, 2, 1, IMMEDIATE, CLR_ORANGE),
SPELL("cure blindness",  "yellow",      P_HEALING_SPELL, 25,  2, 2, 1, IMMEDIATE, CLR_YELLOW),
SPELL("drain life",      "velvet",      P_ATTACK_SPELL, 10,  2, 2, 1, IMMEDIATE, CLR_MAGENTA),
SPELL("slow monster",    "light green", P_ENCHANTMENT_SPELL, 30,  2, 2, 1, IMMEDIATE, CLR_BRIGHT_GREEN),
SPELL("wizard lock",     "dark green",  P_MATTER_SPELL, 30,  3, 2, 1, IMMEDIATE, CLR_GREEN),
// SPELL("create monster",  "turquoise",   P_CLERIC_SPELL, 35,  3, 2, 1, NODIR,     CLR_BRIGHT_CYAN),
SPELL("turn undead",     "copper",      P_CLERIC_SPELL, 35,  3, 2, 1, IMMEDIATE, HI_COPPER),
SPELL("detect food",     "cyan",        P_DIVINATION_SPELL, 30,  3, 2, 1, NODIR,     CLR_CYAN),
SPELL("cause fear",      "light blue",  P_ENCHANTMENT_SPELL, 25,  3, 3, 1, NODIR,     CLR_BRIGHT_BLUE),
SPELL("clairvoyance",    "dark blue",   P_DIVINATION_SPELL, 15,  3, 3, 1, NODIR,     CLR_BLUE),
SPELL("cure sickness",   "indigo",      P_HEALING_SPELL, 32,  3, 3, 1, NODIR,     CLR_BLUE),
SPELL("charm monster",   "magenta",     P_ENCHANTMENT_SPELL, 20,  3, 3, 1, IMMEDIATE, CLR_MAGENTA),
SPELL("haste self",      "purple",      P_ESCAPE_SPELL, 33,  4, 3, 1, NODIR,     CLR_MAGENTA),
SPELL("detect unseen",   "violet",      P_DIVINATION_SPELL, 20,  4, 3, 1, NODIR,     CLR_MAGENTA),
SPELL("levitation",      "tan",         P_ESCAPE_SPELL, 20,  4, 4, 1, NODIR,     CLR_BROWN),
SPELL("extra healing",   "plaid",       P_HEALING_SPELL, 27,  5, 3, 1, IMMEDIATE, CLR_GREEN),
SPELL("restore ability", "light brown", P_HEALING_SPELL, 25,  5, 4, 1, NODIR,     CLR_BROWN),
SPELL("invisibility",    "dark brown",  P_ESCAPE_SPELL, 25,  5, 4, 1, NODIR,     CLR_BROWN),
SPELL("detect treasure", "gray",        P_DIVINATION_SPELL, 20,  5, 4, 1, NODIR,     CLR_GRAY),
SPELL("remove curse",    "wrinkled",    P_CLERIC_SPELL, 25,  5, 3, 1, NODIR,     HI_PAPER),
SPELL("magic mapping",   "dusty",       P_DIVINATION_SPELL, 18,  7, 5, 1, NODIR,     HI_PAPER),
SPELL("identify",        "bronze",      P_DIVINATION_SPELL, 20,  6, 3, 1, NODIR,     HI_COPPER),
// SPELL("turn undead",     "copper",      P_CLERIC_SPELL, 16,  8, 6, 1, IMMEDIATE, HI_COPPER),
SPELL("create monster",  "turquoise",   P_CLERIC_SPELL, 16,  8, 6, 1, NODIR,     CLR_BRIGHT_CYAN),
SPELL("polymorph",       "silver",      P_MATTER_SPELL, 10,  8, 6, 1, IMMEDIATE, HI_SILVER),
SPELL("teleport away",   "gold",        P_ESCAPE_SPELL, 15,  6, 6, 1, IMMEDIATE, HI_GOLD),
SPELL("create familiar", "glittering",  P_CLERIC_SPELL, 10,  7, 6, 1, NODIR,     CLR_WHITE),
SPELL("cancellation",    "shining",     P_MATTER_SPELL, 15,  8, 7, 1, IMMEDIATE, CLR_WHITE),
SPELL("protection",	     "dull",        P_CLERIC_SPELL, 18,  3, 1, 1, NODIR,     HI_PAPER),
SPELL("jumping",	     "thin",        P_ESCAPE_SPELL, 20,  3, 1, 1, IMMEDIATE, HI_PAPER),
SPELL("stone to flesh",	 "thick",       P_HEALING_SPELL, 15,  1, 3, 1, IMMEDIATE, HI_PAPER),
#if 0	/* DEFERRED */
SPELL("flame sphere",    "canvas",      P_MATTER_SPELL, 20,  2, 1, 1, NODIR, CLR_BROWN),
SPELL("freeze sphere",   "hardcover",   P_MATTER_SPELL, 20,  2, 1, 1, NODIR, CLR_BROWN),
#endif
/* blank spellbook must come last because it retains its description */
SPELL("blank paper",     "plain",       P_NONE, 18,  0, 0, 0, 0,         HI_PAPER),
/* a special, one of a kind, spellbook */
OBJECT(OBJ("Book of the Dead", "papyrus"), BITS(0,0,1,0,1,0,1,1,0,0,0,P_NONE,PAPER), 0,
	SPBOOK_CLASS, 0, 0,20, 10000, 0, 0, 0, 7, 20, HI_PAPER),
OBJECT(OBJ("secrets", "ragged leather"), BITS(0,0,1,0,1,0,1,1,0,0,0,P_NONE,PAPER), 0,
	SPBOOK_CLASS, 0, 0,20, 10000, 0, 0, 0, 7, 20, CLR_BROWN),
#undef SPELL

/* wands ... */
#define WAND(name,typ,prob,cost,mgc,dir,metal,color) OBJECT( \
		OBJ(name,typ), BITS(0,0,1,0,mgc,1,0,0,0,0,dir,P_NONE,metal), 0, \
		WAND_CLASS, prob, 0, 7, cost, 0, 0, 0, 0, 30, color )
WAND("light",          "glass",    90, 100, 1, NODIR,     GLASS,    CLR_WHITE),
WAND("darkness",       "obsidian", 10, 100, 1, NODIR,     GLASS,    CLR_BLACK),
WAND("wishing",        "pine",      5, 500, 1, NODIR,     WOOD,     HI_WOOD),
WAND("secret door detection", "balsa",
				   50, 150, 1, NODIR,	  WOOD,     HI_WOOD),
WAND("enlightenment",  "crystal",  17, 150, 1, NODIR,     GLASS,    HI_GLASS),
WAND("create monster", "maple",    42, 200, 1, NODIR,     WOOD,     HI_WOOD),
WAND("nothing",        "oak",      25, 100, 0, IMMEDIATE, WOOD,     HI_WOOD),
WAND("striking",       "ebony",    75, 150, 1, IMMEDIATE, WOOD,     HI_WOOD),
WAND("draining",       "ceramic",   3, 175, 1, IMMEDIATE, MINERAL,  HI_MINERAL),
WAND("make invisible", "marble",   42, 150, 1, IMMEDIATE, MINERAL,  HI_MINERAL),
WAND("slow monster",   "tin",      46, 150, 1, IMMEDIATE, METAL,    HI_METAL),
WAND("speed monster",  "brass",    50, 150, 1, IMMEDIATE, COPPER,   HI_COPPER),
WAND("undead turning", "copper",   50, 150, 1, IMMEDIATE, COPPER,   HI_COPPER),
WAND("polymorph",      "silver",   45, 200, 1, IMMEDIATE, SILVER,   HI_SILVER),
WAND("cancellation",   "platinum", 42, 200, 1, IMMEDIATE, PLATINUM, CLR_WHITE),
WAND("teleportation",  "iridium",  45, 200, 1, IMMEDIATE, METAL,    CLR_BRIGHT_CYAN),
WAND("opening",        "zinc",     30, 150, 1, IMMEDIATE, METAL,    HI_METAL),
WAND("locking",        "aluminum", 25, 150, 1, IMMEDIATE, METAL,    HI_METAL),
WAND("probing",        "uranium",  30, 150, 1, IMMEDIATE, METAL,    HI_METAL),
WAND("digging",        "iron",     55, 150, 1, RAY,       IRON,     HI_METAL),
WAND("magic missile",  "steel",    50, 150, 1, RAY,       IRON,     HI_METAL),
WAND("fire",           "hexagonal",40, 175, 1, RAY,       IRON,     HI_METAL),
WAND("cold",           "short",    40, 175, 1, RAY,       IRON,     HI_METAL),
WAND("sleep",          "runed",    50, 175, 1, RAY,       IRON,     HI_METAL),
WAND("death",          "long",      3, 500, 1, RAY,       IRON,     HI_METAL),
WAND("lightning",      "curved",   40, 175, 1, RAY,       IRON,     HI_METAL),
WAND((char *)0,        "forked",    0, 150, 1, 0,         WOOD,     HI_WOOD),
WAND((char *)0,        "spiked",    0, 150, 1, 0,         IRON,     HI_METAL),
WAND((char *)0,        "jeweled",   0, 150, 1, 0,         IRON,     HI_MINERAL),
#undef WAND

/* coins ... - so far, gold is all there is */
#define COIN(name,prob,metal,worth) OBJECT( \
		OBJ(name,(char *)0), BITS(0,1,0,0,0,0,0,0,0,0,0,P_NONE,metal), 0, \
		COIN_CLASS, prob, 0, 1, worth, 0, 0, 0, 0, 0, HI_GOLD )
	COIN("gold piece",      1000, GOLD,1),
#undef COIN

/* gems ... - includes stones and rocks but not boulders */
#define GEM(name,desc,prob,wt,gval,nutr,mohs,glass,color) OBJECT( \
	    OBJ(name,desc), \
	    BITS(0,1,0,0,0,0,0,0,0,HARDGEM(mohs),0,-P_SLING,glass), 0, \
	    GEM_CLASS, prob, 0, 1, gval, 3, 3, 0, 0, nutr, color )
#define ROCK(name,desc,kn,prob,wt,gval,sdam,ldam,mgc,nutr,mohs,glass,color) OBJECT( \
	    OBJ(name,desc), \
	    BITS(kn,1,0,0,mgc,0,0,0,0,HARDGEM(mohs),0,-P_SLING,glass), 0, \
	    GEM_CLASS, prob, 0, wt, gval, sdam, ldam, 0, 0, nutr, color )
GEM("magicite crystal","brilliant blue",0, 1, 5000, 15, 11, GEMSTONE, CLR_BRIGHT_BLUE),
GEM("dilithium crystal", "white",      2,  1, 4500, 15,  5, GEMSTONE, CLR_WHITE),
GEM("diamond", "white",                3,  1, 4000, 15, 10, GEMSTONE, CLR_WHITE),
GEM("star sapphire", "blue",           4,  1, 3750, 15,  9, GEMSTONE, CLR_BLUE),
GEM("ruby", "red",                     4,  1, 3500, 15,  9, GEMSTONE, CLR_RED),
GEM("jacinth", "orange",               3,  1, 3250, 15,  9, GEMSTONE, CLR_ORANGE),
GEM("sapphire", "blue",                4,  1, 3000, 15,  9, GEMSTONE, CLR_BLUE),
GEM("black opal", "black",             3,  1, 2500, 15,  8, GEMSTONE, CLR_BLACK),
GEM("emerald", "green",                5,  1, 2500, 15,  8, GEMSTONE, CLR_GREEN),
GEM("turquoise", "green",              6,  1, 2000, 15,  6, GEMSTONE, CLR_GREEN),
GEM("citrine", "yellow",               4,  1, 1500, 15,  6, GEMSTONE, CLR_YELLOW),
GEM("aquamarine", "green",             6,  1, 1500, 15,  8, GEMSTONE, CLR_GREEN),
GEM("amber", "yellowish brown",        8,  1, 1000, 15,  2, GEMSTONE, CLR_BROWN),
GEM("topaz", "yellowish brown",       10,  1,  900, 15,  8, GEMSTONE, CLR_BROWN),
GEM("jet", "black",                    6,  1,  850, 15,  7, GEMSTONE, CLR_BLACK),
GEM("opal", "white",                  12,  1,  800, 15,  6, GEMSTONE, CLR_WHITE),
GEM("chrysoberyl", "yellow",           8,  1,  700, 15,  5, GEMSTONE, CLR_YELLOW),
GEM("garnet", "red",                  12,  1,  700, 15,  7, GEMSTONE, CLR_RED),
GEM("amethyst", "violet",             14,  1,  600, 15,  7, GEMSTONE, CLR_MAGENTA),
GEM("jasper", "red",                  15,  1,  500, 15,  7, GEMSTONE, CLR_RED),
GEM("fluorite", "violet",             15,  1,  400, 15,  4, GEMSTONE, CLR_MAGENTA),
GEM("obsidian", "black",               9,  1,  200, 15,  6, GEMSTONE, CLR_BLACK),
GEM("agate", "orange",                12,  1,  200, 15,  6, GEMSTONE, CLR_ORANGE),
GEM("jade", "green",                  10,  1,  300, 15,  6, GEMSTONE, CLR_GREEN),
GEM("worthless piece of white glass", "white",   76, 1, 0, 6, 5, GLASS, CLR_WHITE),
GEM("worthless piece of blue glass", "blue",     76, 1, 0, 6, 5, GLASS, CLR_BLUE),
GEM("worthless piece of red glass", "red",       76, 1, 0, 6, 5, GLASS, CLR_RED),
GEM("worthless piece of yellowish brown glass", "yellowish brown",
						 77, 1, 0, 6, 5, GLASS, CLR_BROWN),
GEM("worthless piece of orange glass", "orange", 76, 1, 0, 6, 5, GLASS, CLR_ORANGE),
GEM("worthless piece of yellow glass", "yellow", 76, 1, 0, 6, 5, GLASS, CLR_YELLOW),
GEM("worthless piece of black glass",  "black",  76, 1, 0, 6, 5, GLASS, CLR_BLACK),
GEM("worthless piece of green glass", "green",   77, 1, 0, 6, 5, GLASS, CLR_GREEN),
GEM("worthless piece of violet glass", "violet", 77, 1, 0, 6, 5, GLASS, CLR_MAGENTA),

/* Placement note: there is a wishable subrange for
 * "gray stones" in the o_ranges[] array in objnam.c
 * that is currently everything between luckstones and flint (inclusive).
 */
ROCK("luckstone", "gray",	0, 10,  10, 60, 3, 3, 1, 10, 7, MINERAL, CLR_GRAY),
ROCK("loadstone", "gray",	0, 10, 500,  1, 30, 30, 1, 10, 6, MINERAL, CLR_GRAY),
ROCK("touchstone", "gray",	0,  8,  10, 45, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY),
ROCK("flint", "gray",		0, 10,  10,  1, 6, 6, 0, 10, 7, MINERAL, CLR_GRAY),
ROCK("small piece of unrefined mithril", "silvery metal", 0, 0, 1, 10000, 3, 3, 0, 0, 5, MITHRIL, HI_SILVER),
ROCK("silver bullet", "silver", 0, 0, 1, 10, 6, 6, 0, 0, 5, SILVER, HI_SILVER),
ROCK("rock", (char *)0,		1,100,  10,  0, 3, 3, 0, 10, 7, MINERAL, CLR_GRAY),
#undef GEM
#undef ROCK

/* miscellaneous ... */
/* Note: boulders and rocks are not normally created at random; the
 * probabilities only come into effect when you try to polymorph them.
 * Boulders weigh more than MAX_CARR_CAP; statues use corpsenm to take
 * on a specific type and may act as containers (both affect weight).
 */
OBJECT(OBJ("boulder",(char *)0), BITS(1,0,0,0,0,0,0,0,1,0,0,P_NONE,MINERAL), 0,
		ROCK_CLASS,   100, 0, 6000,  0, 20, 20, 0, 0, 2000, HI_MINERAL),
OBJECT(OBJ("statue", (char *)0), BITS(1,0,0,1,0,0,0,0,0,0,0,P_NONE,MINERAL), 0,
		ROCK_CLASS,   900, 0, 2500,  0, 20, 20, 0, 0, 2500, CLR_WHITE),
/*
OBJECT(OBJ("bed",(char *)0), BITS(1,0,0,0,0,0,0,0,1,0,0,P_NONE,WOOD), 0,
		BED_CLASS,   100, 0, 2000,  1000, 20, 20, 0, 0, 2000, HI_WOOD),
OBJECT(OBJ("knapsack",(char *)0), BITS(1,0,0,0,0,0,0,0,1,0,0,P_NONE,PLASTIC), 0,
		BED_CLASS,   900, 0, 60,  100, 20, 20, 0, 0, 2000, CLR_GREEN),
*/
#ifdef CONVICT
OBJECT(OBJ("heavy iron ball", (char *)0), BITS(1,0,0,0,0,0,0,0,0,0,WHACK,P_FLAIL,IRON), 0,
#else
OBJECT(OBJ("heavy iron ball", (char *)0), BITS(1,0,0,0,0,0,0,0,0,0,WHACK,P_NONE,IRON), 0,
#endif /* CONVICT */
		BALL_CLASS,  1000, 0,  480, 10, 25, 25, 0, 0,  200, HI_METAL),
						/* +d4 when "very heavy" */
OBJECT(OBJ("iron chain", (char *)0), BITS(1,0,0,0,0,0,0,0,0,0,WHACK,P_NONE,IRON), 0,
		CHAIN_CLASS, 1000, 0,  120,  0,  4,  4, 0, 0,  200, HI_METAL),
OBJECT(OBJ("sheaf of hay", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,0,P_NONE,VEGGY), 0,
		CHAIN_CLASS, 	0, 2, 	20,  0,  0,  0, 0, 0,  100, CLR_YELLOW),
OBJECT(OBJ("clockwork component", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,WHACK,P_NONE,COPPER), 0,
		CHAIN_CLASS,    0, 0,    1,  0,  1,  1, 0, 0,   20, HI_COPPER),
OBJECT(OBJ("subethaic component", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,WHACK,P_NONE,GLASS), 0,
		CHAIN_CLASS,    0, 0,    1,  0,  1,  1, 0, 0,   20, HI_GLASS),
OBJECT(OBJ("scrap", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,WHACK,P_NONE,IRON), 0,
		CHAIN_CLASS,    0, 0,    1,  0,  1,  1, 0, 0,   20, CLR_BROWN),
OBJECT(OBJ("hellfire component", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,WHACK,P_NONE,METAL), 0,
		CHAIN_CLASS,    0, 0,    1,  0,  1,  1, 0, 0,   20, CLR_ORANGE),
						/* +1 both l & s */

OBJECT(OBJ("blinding venom", "splash of venom"),
		BITS(0,1,0,0,0,0,0,1,0,0,0,P_NONE,LIQUID), 0,
		VENOM_CLASS,  500, 0,	 1,  0,  0,  0, 0, 0,	 0, CLR_BLACK),
OBJECT(OBJ("acid venom", "splash of venom"),
		BITS(0,1,0,0,0,0,0,1,0,0,0,P_NONE,LIQUID), 0,
		VENOM_CLASS,  500, 0,	 1,  0,  6,  6, 0, 0,	 0, CLR_BRIGHT_GREEN),
OBJECT(OBJ("ball of webbing", (char *)0),
		BITS(1,1,0,0,0,0,0,1,0,0,0,P_NONE,LIQUID), 0,
		VENOM_CLASS,  0, 0,	 1,  0,  6,  6, 0, 0,	 0, CLR_WHITE),
//OBJECT(OBJ("shoggoth venom", "splash of venom"),
//		BITS(0,1,0,0,0,0,0,1,0,0,0,P_NONE,LIQUID), 0,
//		VENOM_CLASS,  500, 0,	 1,  0,  0,  0, 0, 0,	 0, HI_ORGANIC),
		/* +d6 small or large */

/* fencepost, the deadly Array Terminator -- name [1st arg] *must* be NULL */
	OBJECT(OBJ((char *)0,(char *)0), BITS(0,0,0,0,0,0,0,0,0,0,0,P_NONE,0), 0,
		ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
}; /* objects[] */

#ifndef OBJECTS_PASS_2_

/* perform recursive compilation for second structure */
#  undef OBJ
#  undef OBJECT
#  define OBJECTS_PASS_2_
#include "objects.c"

void NDECL(objects_init);

/* dummy routine used to force linkage */
void
objects_init()
{
    return;
}

#endif	/* !OBJECTS_PASS_2_ */

/*objects.c*/
