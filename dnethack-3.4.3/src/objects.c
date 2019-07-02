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
# define OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,oc3,nut,color) \
	{obj}

NEARDATA struct objdescr obj_descr[] = {
#else
/* second pass -- object definitions */

# define BITS(nmkn,mrg,uskn,ctnr,mgc,chrg,uniq,nwsh,big,tuf,dir,sub,mtrl,shwmat) \
	nmkn,mrg,uskn,0,mgc,chrg,uniq,nwsh,big,tuf,dir,mtrl,shwmat,sub /* SCO ODT 1.1 cpp fodder */
# define OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,oc3,nut,color) \
	{0, 0, (char *)0, bits, prp, sym, dly, COLOR_FIELD(color) \
	 prob, wt, cost, sdam, ldam, oc1, oc2, oc3, nut}
# ifndef lint
#  define HARDGEM(n) (n >= 8)
# else
#  define HARDGEM(n) (0)
# endif

NEARDATA struct objclass objects[] = {
#endif
/* dummy object[0] -- description [2nd arg] *must* be NULL */
	OBJECT(OBJ("strange object",(char *)0), BITS(1,0,0,0,0,0,0,0,0,0,0,P_NONE,0,0),
			0, ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),

/* weapons ... */
#define WEAPON(name,app,kn,mg,bi,prob,wt,cost,sdam,ldam,hitbon,typ,sub,metal,shwmat,color) \
	OBJECT( \
		OBJ(name,app), BITS(kn,mg,1,0,0,1,0,0,bi,0,typ,sub,metal,shwmat), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, WP_GENERIC, 0, wt, color )
#define PROJECTILE(name,app,kn,prob,wt,cost,sdam,ldam,hitbon,metal,sub,color) \
	OBJECT( \
		OBJ(name,app), \
		BITS(kn,1,1,0,0,1,0,0,0,0,PIERCE,sub,metal,0), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, WP_GENERIC, 0, wt, color )
#define BOW(name, app, kn, bi, prob, wt, cost, hitbon, metal, sub, color) \
	OBJECT( \
		OBJ(name,app), BITS(kn,0,1,0,0,1,0,0,bi,0,0,sub,metal,0), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, 0, 0, hitbon, WP_GENERIC, 0, wt, color )
#define BULLET(name,app,kn,prob,wt,cost,sdam,ldam,hitbon,ammotyp,typ,metal,sub,color) \
	OBJECT( \
		OBJ(name,app), BITS(kn,1,1,0,0,1,0,0,0,0,typ,sub,metal,0), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, ammotyp, 0, wt, color )
#define GUN(name,app,kn,bi,prob,wt,cost,range,rof,hitbon,ammotyp,metal,sub,color) \
	OBJECT( \
		OBJ(name,app), BITS(kn,0,1,0,0,1,0,0,bi,0,0,sub,metal,0), 0, \
		WEAPON_CLASS, prob, 0, \
		wt, cost, range, rof, hitbon, ammotyp, 0, wt, color )

/* Note: for weapons that don't do an even die of damage (ex. 2-7 or 3-18)
 * the extra damage is added on in weapon.c, not here! */

#define P PIERCE
#define S SLASH
#define B WHACK
#define E EXPLOSION

/* missiles */
PROJECTILE("arrow", (char *)0,
		1, 50, 1, 2, 6, 6, 0, IRON, -P_BOW, HI_METAL),
PROJECTILE("elven arrow", "runed arrow",
		0, 18, 1, 2, 7, 5, 2, WOOD, -P_BOW, HI_WOOD),
PROJECTILE("orcish arrow", "crude arrow",
		0, 18, 1, 2, 5, 8, -1, IRON, -P_BOW, CLR_BLACK),
PROJECTILE("silver arrow", (char *)0,
		1, 12, 1, 5, 6, 6, 0, SILVER, -P_BOW, HI_SILVER),
PROJECTILE("golden arrow", (char *)0, /*Needs encyc entry*//*Needs tile*/
		1,  9, 2,10,13,13, 0, GOLD, -P_BOW, HI_GOLD),
PROJECTILE("ancient arrow", (char *)0, /*Needs encyc entry*//*Needs tile*/
		1,  0, 1,10,10,10, 0, METAL, -P_BOW, CLR_BLACK),
PROJECTILE("ya", "bamboo arrow",
		0, 15, 1, 4, 7, 7, 1, METAL, -P_BOW, HI_METAL),
PROJECTILE("crossbow bolt", (char *)0,
		1, 55, 1, 2, 4, 6, 0, IRON, -P_CROSSBOW, HI_METAL),
PROJECTILE("droven bolt", "crossbow bolt", /*Needs encyc entry*/
		0,  0, 1, 2, 9, 6, 2, OBSIDIAN_MT, -P_CROSSBOW, CLR_BLACK),

WEAPON("dart", (char *)0,
	1, 1, 0, 58,  1,  2,  3,  2, 0, P,   -P_DART, IRON, FALSE, HI_METAL),
WEAPON("shuriken", "throwing star",
	0, 1, 0, 33,  1,  5,  8,  6, 2, P|S,   -P_SHURIKEN, IRON, FALSE, HI_METAL),
WEAPON("boomerang", (char *)0,
	1, 1, 0, 13,  5, 20,  9,  9, 0, B,   -P_BOOMERANG, WOOD, FALSE, HI_WOOD),
WEAPON("chakram", "circular blade", /*Needs encyc entry*//*Needs tile*/
	1, 1, 0,  6,  5, 20,  9,  9, 0, S,   -P_BOOMERANG, SILVER, IDED|UNIDED, HI_SILVER),
WEAPON("spike", (char *)0, /*Needs encyc entry*/
	1, 1, 0,  0,  1,  2,  3,  1, 0, P,   -P_DART, BONE, IDED|UNIDED, CLR_WHITE),

/* spears */
WEAPON("spear", (char *)0,
	1, 1, 0, 50, 25,  3,  6,  8, 0, P,   P_SPEAR, IRON, FALSE, HI_METAL),
WEAPON("elven spear", "runed spear",
	0, 1, 0, 10,  2,  3,  7,  7, 2, P,   P_SPEAR, WOOD, FALSE, HI_WOOD),
WEAPON("droven spear", "long spear", /*Needs encyc entry*/
	0, 1, 1,  0, 15,  3, 12, 12, 2, P,   P_SPEAR, OBSIDIAN_MT, UNIDED, CLR_BLACK),
WEAPON("orcish spear", "crude spear",
	0, 1, 0, 13, 25,  3,  5,  10, -1, P,   P_SPEAR, IRON, FALSE, CLR_BLACK),
WEAPON("dwarvish spear", "stout spear",
	0, 1, 0, 12, 30,  3,  9,  9, 0, P,   P_SPEAR, IRON, FALSE, HI_METAL),
WEAPON("javelin", "throwing spear",
	0, 1, 0, 10, 20,  3,  6,  6, 0, P,   P_JAVELIN, IRON, FALSE, HI_METAL),

WEAPON("trident", (char *)0, /*Needs encyc entry*/
	1, 0, 0,  8, 25,  5,  6,  4, 0, P,   P_TRIDENT, IRON, FALSE, HI_METAL),
						/* +1 small, +2d4 large */

/* blades */
WEAPON("dagger", (char *)0,
	1, 1, 0, 24, 10,  4,  4,  3, 2, P,   P_DAGGER, IRON, FALSE, HI_METAL),
WEAPON("elven dagger", "runed dagger",
	0, 1, 0,  7,  1,  4,  5,  3, 4, P,   P_DAGGER, WOOD, FALSE, HI_WOOD),
WEAPON("droven dagger", "dagger", /*Needs encyc entry*/
	0, 1, 0,  0,  5,  12,  8,  6, 4, P,   P_DAGGER, OBSIDIAN_MT, UNIDED, CLR_BLACK),
WEAPON("orcish dagger", "crude dagger",
	0, 1, 0,  9, 10,  5,  3,  5, 1, P,   P_DAGGER, IRON, FALSE, CLR_BLACK),
WEAPON("athame", (char *)0,
	1, 1, 0,  0, 10,  4,  4,  4, 2, S,   P_DAGGER, IRON, FALSE, HI_METAL),
WEAPON("set of crow talons", "set of three feather-etched daggers",
	0, 0, 0,  0,  9,200,  4,  3, 2, S,   P_DAGGER, METAL, FALSE, HI_METAL),
WEAPON("scalpel", (char *)0,
	1, 1, 0,  0,  5,  6,  3,  1, 3, S,   P_KNIFE, METAL, FALSE, HI_METAL),
WEAPON("knife", (char *)0,
	1, 1, 0, 14,  5,  4,  3,  2, 2, P|S, P_KNIFE, IRON, FALSE, HI_METAL),
WEAPON("stiletto", (char *)0,
	1, 1, 0,  4,  5,  4,  6,  2, 1, P, P_KNIFE, IRON, FALSE, HI_METAL),
WEAPON("worm tooth", (char *)0,
	1, 0, 0,  0, 20,  2,  2,  2, 0, P,   P_KNIFE, MINERAL, FALSE, CLR_WHITE),
WEAPON("crysknife", (char *)0,
	1, 0, 0,  0, 20,100, 10, 10, 3, P,   P_KNIFE, MINERAL, FALSE, CLR_WHITE),

WEAPON("sickle", (char *)0, /* Vs plants: +6 to hit and double damage */
	1, 1, 0, 22, 20,  4,  4,  1, -2, S,   P_HARVEST, IRON, FALSE, HI_METAL),
WEAPON("elven sickle", "runed sickle", /* Vs plants: +6 to hit and double damage *//*Needs tile*/
	0, 1, 0,  0,  2,  4,  6,  3,  0, S,   P_HARVEST, WOOD, FALSE, HI_WOOD),

WEAPON("axe", (char *)0,
	1, 0, 0, 20, 60,  8,  6,  4, 0, S,   P_AXE, IRON, FALSE, HI_METAL),
WEAPON("battle-axe", "double-bitted axe",/* was "double-headed" ? */
	0, 0, 1, 16,120, 40,  8,  6, 0, S,   P_AXE, IRON, FALSE, HI_METAL), /*+1d4/+2d4*/
WEAPON("moon axe", "two-handed axe", /*Needs encyc entry*//*Needs tile*/
	0, 0, 1, 12,160, 40,  6,  6, 0, S,   P_AXE, SILVER, UNIDED, HI_SILVER),

/* swords */
WEAPON("short sword", (char *)0,
	1, 0, 0,  8, 30, 10,  6,  8, 0, P,   P_SHORT_SWORD, IRON, FALSE, HI_METAL),
WEAPON("vibroblade", "gray short sword", /*Needs encyc entry*//*Needs tile*/
	1, 0, 0,  0,  5,1000, 6,  8, 0, P,   P_SHORT_SWORD, PLASTIC, FALSE, CLR_GRAY),
WEAPON("elven short sword", "runed short sword",
	0, 0, 0,  2,  2, 10,  7,  7, 2, P,   P_SHORT_SWORD, WOOD, FALSE, HI_WOOD),
WEAPON("droven short sword", "short sword", /*Needs encyc entry*/
	0, 0, 0,  0, 15, 10,  9,  9, 2, P,   P_SHORT_SWORD, OBSIDIAN_MT, UNIDED, CLR_BLACK),
WEAPON("orcish short sword", "crude short sword",
	0, 0, 0,  3, 30, 10,  5,  10, -1, P,   P_SHORT_SWORD, IRON, FALSE, CLR_BLACK),
WEAPON("dwarvish short sword", "broad short sword",
	0, 0, 0,  2, 35, 10,  8,  7, 0, P|S,   P_SHORT_SWORD, IRON, FALSE, HI_METAL),
WEAPON("mirrorblade", "polished short sword",
	1, 0, 0,  0, 40,100,  6,  8, 0, P,   P_SHORT_SWORD, SILVER, FALSE, HI_SILVER),

WEAPON("scimitar", "curved sword",
	0, 0, 0, 14, 40, 15,  8,  8, 0, S,   P_SCIMITAR, IRON, FALSE, HI_METAL),
WEAPON("high-elven warsword", "runed curved sword",
	0, 0, 0,  1, 10,150, 10, 10, 2, S,   P_SCIMITAR, MITHRIL, UNIDED, HI_METAL),
WEAPON("rapier", (char *)0,
	1, 0, 0,  6, 28, 20,  6,  4, 2, P,   P_SABER, METAL, FALSE, HI_METAL),
WEAPON("saber", (char *)0,
	1, 0, 0,  6, 34, 75,  8,  8, 0, S,   P_SABER, SILVER, IDED|UNIDED, HI_SILVER),
WEAPON("crow quill", "feather-etched rapier",
	0, 0, 0,  0, 34,200,  8,  8, 2, P,   P_SABER, METAL, FALSE, HI_METAL),
WEAPON("rakuyo", "double-bladed saber",
	0, 0, 0,  0, 38,500,  8,  8, 2, P|S,   P_SABER, METAL, FALSE, HI_METAL),
WEAPON("rakuyo-saber", "latch-pommeled saber",
	0, 0, 0,  0, 28,400,  8,  8, 2, P|S,   P_SABER, METAL, FALSE, HI_METAL),
WEAPON("rakuyo-dagger", "latch-pommeled dagger",
	0, 0, 0,  0, 10,100,  4,  3, 2, P|S,   P_DAGGER, METAL, FALSE, HI_METAL),
WEAPON("broadsword", (char *)0,
	1, 0, 0,  8, 70, 10,  4,  6, 0, S,   P_BROAD_SWORD, IRON, FALSE, HI_METAL),
						/* +d4 small, +1 large */
WEAPON("elven broadsword", "runed broadsword",
	0, 0, 0,  4,  4, 10,  6,  6, 2, S,   P_BROAD_SWORD, WOOD, FALSE, HI_WOOD),
						/* +d4 small, +2 large */
WEAPON("long sword", (char *)0,
	1, 0, 0, 46, 40, 15,  8, 12, 0, S|P,   P_LONG_SWORD, IRON, FALSE, HI_METAL),
WEAPON("crystal sword", (char *)0, /*Needs encyc entry*//*Needs tile*/
	1, 0, 0, 2, 120,300,  8, 12, 0, S|P,   P_LONG_SWORD, GLASS, FALSE, CLR_CYAN),
WEAPON("two-handed sword", (char *)0,
	1, 0, 1, 22,150, 50, 12,  6, 0, S,   P_TWO_HANDED_SWORD, IRON, FALSE, HI_METAL),
						/* +2d6 large */
WEAPON("droven greatsword", "two-handed sword", /*Needs encyc entry*//*Needs tile*/
	0, 0, 1,  0,150, 50,  18, 30, 2, S,   P_TWO_HANDED_SWORD, OBSIDIAN_MT, UNIDED, CLR_BLACK),
WEAPON("katana", "samurai sword",
	0, 0, 0,  4, 40, 80, 10, 12, 1, S,   P_LONG_SWORD, IRON, FALSE, HI_METAL),
/* special swords set up for artifacts */
WEAPON("tsurugi", "long samurai sword",
	0, 0, 1,  0, 60,500, 16,  8, 2, S,   P_TWO_HANDED_SWORD, METAL, FALSE, HI_METAL),
						/* +2d6 large */
WEAPON("runesword", "runed black blade",
	0, 0, 0,  0, 40,300,  10,  10, 0, S,   P_BROAD_SWORD, IRON, FALSE, CLR_BLACK),
						/* +d4 small, +1 large */
						/* +5d2 +d8 from level drain */
WEAPON("white vibrosword", "white sword", /*Needs encyc entry*//*Needs tile*/
	0, 0, 0,  0,  7,8000, 10, 12, 1, P|S,   P_LONG_SWORD, PLASTIC, FALSE, CLR_WHITE),
WEAPON("gold-bladed vibrosword", "black and gold sword", /*Needs encyc entry*//*Needs tile*/
	0, 0, 0,  0, 80,8000, 10, 12, 1, P|S,   P_LONG_SWORD,    GOLD, FALSE, CLR_BLACK),
WEAPON("red-eyed vibrosword", "blue-glowing sword", /*Needs encyc entry*//*Needs tile*/
	0, 0, 0,  0, 80,8000, 10, 12, 1, P|S,   P_LONG_SWORD, PLASTIC, FALSE, CLR_GRAY),
WEAPON("white vibrozanbato", "curved white sword",
	0, 0, 1,  0, 60,50000, 16,  8, 2, S,   P_TWO_HANDED_SWORD, PLASTIC, FALSE, CLR_WHITE),
						    /* +2d6 large */
WEAPON("gold-bladed vibrozanbato", "curved black and gold sword",
	0, 0, 1,  0, 60,50000, 16,  8, 2, S,   P_TWO_HANDED_SWORD, GOLD, FALSE, CLR_BLACK),
						    /* +2d6 large */

/* polearms */
/* spear-type */
WEAPON("partisan", "vulgar polearm",
	0, 0, 1,  5, 80, 10,  6,  6, 0, P,   P_POLEARMS, IRON, FALSE, HI_METAL),
						/* +1 large */
WEAPON("ranseur", "hilted polearm",
	0, 0, 1,  5, 50,  6,  4,  4, 0, P,   P_POLEARMS, IRON, FALSE, HI_METAL),
						/* +d4 both */
WEAPON("spetum", "forked polearm",
	0, 0, 1,  5, 50,  5,  6,  6, 0, P,   P_POLEARMS, IRON, FALSE, HI_METAL),
						/* +1 small, +d6 large */
WEAPON("glaive", "single-edged polearm",
	0, 0, 1,  7, 75,  6,  6, 10, 0, S,   P_POLEARMS, IRON, FALSE, HI_METAL),
WEAPON("naginata", "samurai-sword polearm",
	0, 0, 1,  1, 75, 90,  8, 10, 1, S,   P_POLEARMS, IRON, FALSE, HI_METAL),
WEAPON("lance", (char *)0,
	1, 0, 0,  4,180, 10,  6,  8, 0, P,   P_LANCE, IRON, FALSE, HI_METAL),
WEAPON("force pike", "long gray spear",/*Needs tile*/
	0, 0, 0,  0, 30, 1000,  6,  8, 2, P|S,   P_LANCE, PLASTIC, FALSE, CLR_GRAY),
WEAPON("white vibrospear", "long white spear",/*Needs tile*/
	0, 0, 0,  0, 30, 1000,  6,  8, 2, P|S,   P_LANCE, PLASTIC, FALSE, CLR_WHITE),
WEAPON("gold-bladed vibrospear", "long black and gold spear",/*Needs tile*/
	0, 0, 0,  0, 30, 1000,  6,  8, 2, P|S,   P_LANCE, GOLD, FALSE, CLR_BLACK),
WEAPON("elven lance", "runed lance", /*Needs encyc entry*//*Needs tile*/
	0, 0, 0,  0, 11, 10,  8,  8, 2, P,   P_LANCE, WOOD, FALSE, HI_WOOD),
WEAPON("droven lance", "lance", /*Needs encyc entry*//*Needs tile*/
	0, 0, 1,  0,100, 10, 10, 10, 2, P,   P_LANCE, OBSIDIAN_MT, UNIDED, CLR_BLACK),
/* axe-type */
WEAPON("halberd", "angled poleaxe",
	0, 0, 1,  8,150, 10, 10,  6, 0, P|S, P_POLEARMS, IRON, FALSE, HI_METAL),
						/* +1d6 large */
WEAPON("bardiche", "long poleaxe",
	0, 0, 1,  4,120,  7,  4,  4, 0, S,   P_POLEARMS, IRON, FALSE, HI_METAL),
						/* +1d4 small, +2d4 large */
WEAPON("voulge", "pole cleaver",
	0, 0, 1,  4,125,  5,  4,  4, 0, S,   P_POLEARMS, IRON, FALSE, HI_METAL),
						/* +d4 both */
WEAPON("dwarvish mattock", "broad pick",
	0, 0, 1, 13,120, 50, 12,  8,-1, P|B,   P_PICK_AXE, IRON, FALSE, HI_METAL),

/* curved/hooked */
WEAPON("fauchard", "pole sickle",
	0, 0, 1,  5, 60,  5,  6,  8, 0, P|S, P_POLEARMS, IRON, FALSE, HI_METAL),
WEAPON("guisarme", "pruning hook",
	0, 0, 1,  5, 80,  5,  4,  8, 0, S,   P_POLEARMS, IRON, FALSE, HI_METAL),
						/* +1d4 small */
WEAPON("bill-guisarme", "hooked polearm",
	0, 0, 1,  3,120,  7,  4, 10, 0, P|S, P_POLEARMS, IRON, FALSE, HI_METAL),
						/* +1d4 small */
/* other */
WEAPON("lucern hammer", "pronged polearm",
	0, 0, 1,  4,150,  7,  4,  6, 0, B|P, P_POLEARMS, IRON, FALSE, HI_METAL),
						/* +1d4 small */
WEAPON("bec de corbin", "beaked polearm",
	0, 0, 1,  3,100,  8,  8,  6, 0, B|P, P_POLEARMS, IRON, FALSE, HI_METAL),

WEAPON("scythe", (char *)0, /* 2d4 damage. Vs plants: +6 to hit and double damage */
	1, 0, 1, 5, 100,  6,  4,  4, -2, S,   P_HARVEST, IRON, FALSE, HI_METAL),

/* bludgeons */
WEAPON("mace", (char *)0,
	1, 0, 0, 40, 30,  5,  6,  6, 0, B,   P_MACE, IRON, FALSE, HI_METAL),
						/* +1 small */
WEAPON("elven mace", "runed mace", /*Needs encyc entry*/
	0, 0, 0,  0,  2,  5,  7,  7, 2, B,   P_MACE, WOOD, FALSE, HI_WOOD),
						/* +1 small */
WEAPON("morning star", (char *)0,
	1, 0, 0, 12,120, 10,  4,  6, 0, B|P,   P_MORNING_STAR, IRON, FALSE, HI_METAL),
						/* +d4 small, +1 large */
WEAPON("war hammer", (char *)0,
	1, 0, 0, 15, 50,  5,  4,  4, 0, B,   P_HAMMER, IRON, FALSE, HI_METAL),
						/* +1 small */
WEAPON("club", (char *)0,
	1, 0, 0, 11, 30,  3,  6,  3, 0, B,   P_CLUB, WOOD, FALSE, HI_WOOD),
WEAPON("quarterstaff", "staff",
	0, 0, 1, 10, 40,  5,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, FALSE, HI_WOOD),
WEAPON("khakkhara", "monk's staff", /*Needs encyc entry*//*Needs tile*/
	0, 0, 1,  2, 120,  50,  6,  4, 0, B|P,   P_QUARTERSTAFF, SILVER, IDED, HI_SILVER),
WEAPON("kamerel vajra", "short mace", /*Needs encyc entry*/
	0, 0, 0,  0,  10,800, 6, 6, 1, S|E, P_MACE, GOLD, UNIDED, HI_GOLD),
WEAPON("bar", (char *)0,
	1, 0, 1, 0, 400,  10,  8,  6, -10, B,   P_QUARTERSTAFF, IRON, IDED|UNIDED, HI_METAL),
/* two-piece */
WEAPON("aklys", "thonged club",
	0, 0, 0,  8, 15,  4,  6,  3, 0, B,   P_CLUB, IRON, FALSE, HI_METAL),
WEAPON("flail", (char *)0,
	1, 0, 0, 35, 15,  4,  6,  4, 0, B,   P_FLAIL, IRON, FALSE, HI_METAL),
						/* +1 small, +1d4 large */
/* misc */
WEAPON("bullwhip", (char *)0,
	1, 0, 0,  5, 20,  4,  2,  1, 0, B,   P_WHIP, LEATHER, FALSE, CLR_BROWN),
WEAPON("viperwhip", (char *)0,
	1, 0, 0,  2,  3, 40,  4,  3, 2, P,   P_WHIP, SILVER, IDED|UNIDED, HI_SILVER),


/* Firearms */
//ifdef FIREARMS
 /*Needs encyc entry*/
GUN("flintlock", "broken hand-crossbow",			0,  0, 0,  10,   50,  8, -2,  0, WP_BULLET, IRON, P_FIREARM, HI_METAL), /*Needs tile*/

GUN("pistol", "broken hand-crossbow",	   	   		0,  0, 0,  12,  100, 15,  1,  0, WP_BULLET, IRON, P_FIREARM, HI_METAL), /*Needs tile*/
GUN("submachine gun", "strange broken crossbow",   	0,  0, 0,  25,  250, 10,  3, -1, WP_BULLET, IRON, P_FIREARM, HI_METAL), /*Needs tile*/
GUN("heavy machine gun", "strange broken crossbow",	0,  1, 0, 100, 2000, 20,  8, -4, WP_BULLET, IRON, P_FIREARM, HI_METAL), /*Needs tile*/
GUN("rifle", "broken crossbow",		   	   			0,  1, 0,  30,  150, 22, -1,  1, WP_BULLET, IRON, P_FIREARM, HI_METAL), /*Needs tile*/
GUN("assault rifle", "broken crossbow",	   			0,  0, 0,  40, 1000, 20,  5, -2, WP_BULLET, IRON, P_FIREARM, HI_METAL), /*Needs tile*/
GUN("sniper rifle", "broken crossbow",	   			0,  1, 0,  50, 4000, 25, -3,  0, WP_BULLET, IRON, P_FIREARM, HI_METAL), /*Needs tile*/
GUN("shotgun", "broken crossbow",	   	   			0,  0, 0,  35,  200,  3, -1,  3,  WP_SHELL, IRON, P_FIREARM, HI_METAL), /*Needs tile*/
GUN("auto shotgun", "strange broken crossbow",	   	0,  1, 0,  60, 1500,  3,  2,  0,  WP_SHELL, IRON, P_FIREARM, HI_METAL), /*Needs tile*/
GUN("rocket launcher", "metal tube",  				0,  1, 0, 100, 3500, 20, -5, -4, WP_ROCKET, IRON, P_FIREARM, HI_METAL), /*Needs tile*/
GUN("grenade launcher", "strange broken crossbow", 	0,  0, 0,  55, 1500,  6, -3, -3,WP_GRENADE, IRON, P_FIREARM, HI_METAL), /*Needs tile*/

GUN("BFG", "ovoid device", 			   		0,  1, 0, 100, 3000,100,  3,  0,WP_ROCKET|WP_GRENADE|WP_BULLET|WP_SHELL|WP_ROCKET,
																								SILVER, P_FIREARM, HI_SILVER), /*Needs tile*/
GUN("handgun", "unfamiliar handgun",			0,  0, 0,  12,  100, 15,  1,  0, WP_BULLET, IRON, P_FIREARM, HI_METAL),/*Needs tile*/
GUN("gun", "unfamiliar gun",					0,  0, 0,  25,  250, 10,  3, -1, WP_BULLET, IRON, P_FIREARM, HI_METAL),/*Needs tile*/
GUN("long gun", "unfamiliar long gun",			0,  1, 0,  30,  150, 22, -1,  1, WP_BULLET, IRON, P_FIREARM, HI_METAL),/*Needs tile*/
GUN("heavy gun", "unfamiliar heavy gun",		0,  1, 0, 100, 2000, 20,  8, -4, WP_BULLET, IRON, P_FIREARM, HI_METAL),/*Needs tile*/
 /*Needs encyc entry*/
GUN("hand blaster", "hard black handmirror",		0,  0, 0,   2, 1000, 10,  1,  0,WP_BLASTER, PLASTIC, P_FIREARM, CLR_BLACK), /*Needs tile*/
GUN("arm blaster",  "hard white bracer", 	   		0,  0, 0,   8, 4500, 15,  6,  0,WP_BLASTER, PLASTIC, P_FIREARM, CLR_WHITE), /*Needs tile*/
GUN("mass-shadow pistol",  "rectangular device",	0,  0, 0,   4, 4500, 10,  1,  0,WP_BLASTER, PLASTIC, P_FIREARM, CLR_GRAY), /*Needs tile*/
GUN("cutting laser","hard tan lozenge",    			0,  0, 0,   1, 1000,  3, -1,  3,WP_BLASTER, PLASTIC, P_FIREARM, CLR_YELLOW), /*Needs tile*/

GUN("raygun", "hard handle ending in glassy disks",	0,  0, 0,   8, 3000, 15,  1,  0,WP_BLASTER, PLASTIC, P_FIREARM, CLR_BRIGHT_CYAN), /*Needs tile*/
BULLET("bullet", "pellet",
	0,  0,   1,   5, 8, 6, 0, WP_BULLET,   P,   METAL, -P_FIREARM, HI_METAL),/*Needs tile*/
BULLET("silver bullet", "silver pellet",
	0,  0,   1,  15, 8, 6, 0, WP_BULLET,   P, SILVER, -P_FIREARM, HI_SILVER),/*Needs tile*/
BULLET("shotgun shell", "red tube",
	0,  0,   1,  10,12, 6,10,  WP_SHELL,   P,   METAL, -P_FIREARM, CLR_RED),/*Needs tile*/
BULLET("frag grenade", "green spheriod",
	0,  0,   5, 350, 2, 2, 0,  WP_GRENADE, B,   IRON, -P_FIREARM, CLR_GREEN),/*Needs tile*/
BULLET("gas grenade", "lime spheriod",
	0,  0,   2, 350, 2, 2, 0,  WP_GRENADE, B,   IRON, -P_FIREARM, CLR_BRIGHT_GREEN),/*Needs tile*/
BULLET("rocket", "firework",
	0,  0,  20, 450,12,20, 0, WP_ROCKET, P, SILVER, -P_FIREARM, CLR_BLUE),/*Needs tile*/
BULLET("stick of dynamite", "red stick",
	0,  0,  10, 150, 0, 0, 0,  WP_GENERIC,   B,   PLASTIC, P_NONE, CLR_RED),/*Needs tile*/

BULLET("blaster bolt", "ruby bolt",
		0,  0, 1, 0, 6, 8, 0, WP_BLASTER,   E,   METAL, -P_FIREARM, CLR_RED),/*Needs tile*/
BULLET("heavy blaster bolt", "scarlet bolt",
		0,  0, 1, 0,10,12, 0, WP_BLASTER,   E,   METAL, -P_FIREARM, CLR_ORANGE),/*Needs tile*/
BULLET("laser beam", "green bolt",
		0,  0, 1, 0, 1, 1, 0, WP_BLASTER,   S,   METAL, -P_FIREARM, CLR_BRIGHT_GREEN),/*Needs tile*/
//endif

/* bows */
BOW("bow", (char *)0,			1, 0, 24, 30, 60, 0, WOOD, P_BOW, HI_WOOD),
BOW("elven bow", "runed bow",	0, 0, 12, 20, 60, 2, WOOD, P_BOW, HI_WOOD),
BOW("orcish bow", "crude bow",	0, 0, 12, 30, 60, -2, WOOD, P_BOW, CLR_BLACK),
BOW("yumi", "long bow",			0,  0, 0, 30, 60, 0, WOOD, P_BOW, HI_WOOD),
BOW("sling", (char *)0,			1, 0, 40,  3, 20, -1, LEATHER, P_SLING, HI_LEATHER),
BOW("crossbow", (char *)0,		1, 0, 45, 50, 40, 1, WOOD, P_CROSSBOW, HI_WOOD),
BOW("droven crossbow", "spider-legged crossbow",	0, 0, 0, 50, 120, 4, SILVER, P_CROSSBOW, CLR_BLACK), /*Needs encyc entry*/

#undef P
#undef S
#undef B

#undef WEAPON
#undef PROJECTILE
#undef BOW
#undef BULLET
#undef GUN

/* armor ... */
/* IRON denotes ferrous metals, including steel.
 * Only IRON weapons and armor can rust.
 * Only COPPER (including brass) corrodes.
 * Some creatures are vulnerable to SILVER.
 */
#define ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,dr,can,sub,metal,c) \
	OBJECT( \
		OBJ(name,desc), BITS(kn,0,1,0,mgc,1,0,0,blk,0,0,sub,metal,0), power, \
		ARMOR_CLASS, prob, delay, wt, cost, \
		0, 0, 10 - ac, can, dr, wt, c )
#define ARMOR_MATSPEC(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,dr,can,sub,metal,shwmat,c) \
	OBJECT( \
		OBJ(name,desc), BITS(kn,0,1,0,mgc,1,0,0,blk,0,0,sub,metal,shwmat), power, \
		ARMOR_CLASS, prob, delay, wt, cost, \
		0, 0, 10 - ac, can, dr, wt, c )
#define HELM(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,dr,can,metal,c) \
	ARMOR(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,dr,can,ARM_HELM,metal,c)
#define CLOAK(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,dr,can,metal,c) \
	ARMOR(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,dr,can,ARM_CLOAK,metal,c)
#define SHIELD(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,dr,can,metal,c) \
	ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,dr,can,ARM_SHIELD,metal,c)
#define GLOVES(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,dr,can,metal,c) \
	ARMOR(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,dr,can,ARM_GLOVES,metal,c)
#define BOOTS(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,dr,can,metal,c) \
	ARMOR(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,dr,can,ARM_BOOTS,metal,c)

#define HELM_MATSPEC(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,dr,can,metal,shwmat,c) \
	ARMOR_MATSPEC(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,dr,can,ARM_HELM,metal,shwmat,c)
#define CLOAK_MATSPEC(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,dr,can,metal,shwmat,c) \
	ARMOR_MATSPEC(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,dr,can,ARM_CLOAK,metal,shwmat,c)
#define SHIELD_MATSPEC(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,dr,can,metal,shwmat,c) \
	ARMOR_MATSPEC(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,dr,can,ARM_SHIELD,metal,shwmat,c)
#define GLOVES_MATSPEC(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,dr,can,metal,shwmat,c) \
	ARMOR_MATSPEC(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,dr,can,ARM_GLOVES,metal,shwmat,c)
#define BOOTS_MATSPEC(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,dr,can,metal,shwmat,c) \
	ARMOR_MATSPEC(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,dr,can,ARM_BOOTS,metal,shwmat,c)

/* helmets */
HELM("sedge hat", "wide conical hat", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	3, 1,  3,   8,10, 1, 0, VEGGY, CLR_YELLOW),
HELM("leather helm", "leather hat",
		0, 0,  0,	5, 1,  3,   8,10, 1, 0, LEATHER, HI_LEATHER),
HELM("orcish helm", "skull cap",
		0, 0,  0,	5, 1, 30,  10, 9, 0, 0, IRON, CLR_BLACK),
HELM("dwarvish helm", "hard hat",
		0, 0,  0,	5, 1, 40,  20, 9, 1, 0, IRON, HI_METAL),
HELM("gnomish pointy hat", "conical hat",
		0, 0,  0,	0, 1, 10,  2, 10, 0, 0, CLOTH, CLR_RED),
HELM("fedora", (char *)0,
		1, 0,  0,	0, 0,  3,   1,10, 0, 0, CLOTH, CLR_BROWN),
HELM("cornuthaum", "conical hat",
		0, 1,  CLAIRVOYANT,
					3, 1,  4,  80,10, 0, 2, CLOTH, CLR_BLUE),
HELM("dunce cap", "conical hat",
		0, 1,  0,	3, 1,  4,   1, 10, 0, 0, CLOTH, CLR_BLUE),
HELM("war hat", "wide helm", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	2, 0, 100,  30, 8, 2, 2, IRON, HI_METAL),
HELM("flack helmet", "green bowl", /*Needs encyc entry*/
		0, 0,  0,	0, 0, 10,   50, 9, 2, 1, PLASTIC, CLR_GREEN),
HELM("bronze helm", "helmet",
		0, 0,  0,   0, 1, 30,  12, 9, 2, 0, COPPER, HI_COPPER),
HELM("harmonium helm", "red-lacquered spined helm",
		0, 0,  0,   0, 1, 45,   1, 9, 2, 0, METAL, CLR_RED),
HELM("elven helm", "runed helm", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	0, 1,  2,   5, 9, 1, 0, WOOD, HI_WOOD),
HELM_MATSPEC("high-elven helm", "runed helm", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	0, 1,  3,   5, 9, 2, 0, MITHRIL,UNIDED, HI_METAL),
HELM("droven helm", "spider-shaped helm", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	0, 1, 50,   5, 8, 2, 0, MINERAL, CLR_BLACK),
HELM("plasteel helm", "white skull helm", /*Needs encyc entry*//*Needs tile*/
		0, 1,  INFRAVISION,   0, 2, 25,  50, 8, 2, 2, PLASTIC, CLR_WHITE),
HELM_MATSPEC("crystal helm", "fish bowl", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,   0, 2,150, 300, 9, 0, 0, GLASS,UNIDED, CLR_CYAN),
HELM("pontiff's crown", "filigreed faceless helm", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,   0, 2,150, 300, 8, 3, 0, METAL, HI_GOLD),
/* With shuffled appearances... */
HELM("helmet", "plumed helmet", /* circlet */
		0, 0,  0,      8, 1, 30,  10, 9, 2, 0, IRON, CLR_RED),
HELM("helm of brilliance", "etched helmet", /* crown of cognizance */
		0, 1,  0,	6, 1, 50,  50, 9, 1, 0, IRON, CLR_BRIGHT_GREEN),
HELM("helm of opposite alignment", "crested helmet", /* tiara of treachery */
		0, 1,  0,	6, 1, 50,  50, 9, 1, 0, IRON, HI_METAL),
HELM("helm of telepathy", "visored helmet", /*tiara of telepathy*/ /*Note: 'visored' appearance gives +1 DR*/
		0, 1,  TELEPAT, 2, 1, 50,  50, 9, 1, 0, IRON, HI_METAL),
HELM("helm of drain resistance", "band", /*diadem of drain resistance*//*Needs tile*//*Note: 'band' appearance gives -1 DR*/
		0, 1,  DRAIN_RES, 2, 1, 50,  50, 9, 1, 0, IRON, HI_GOLD),

/* suits of armor */
/*
 * There is code in polyself.c that assumes (1) and (2).
 * There is code in obj.h, objnam.c, mon.c, read.c that assumes (2).
 *
 *	(1) The dragon scale mails and the dragon scales are together.
 *	(2) That the order of the dragon scale mail and dragon scales is the
 *	    the same defined in monst.c.
 */
#define DRGN_ARMR(name,mgc,power,cost,ac,dr,color) \
	ARMOR(name,(char *)0,1,mgc,1,power,0,5,150,cost,ac,dr,0,ARM_SUIT,DRAGON_HIDE,color)
/* 3.4.1: dragon scale mail reclassified as "magic" since magic is
   needed to create them */
DRGN_ARMR("gray dragon scale mail",   1, ANTIMAGIC,  1200, 5, 4, CLR_GRAY),
DRGN_ARMR("silver dragon scale mail", 1, REFLECTING, 1200, 5, 4, DRAGON_SILVER),
DRGN_ARMR("shimmering dragon scale mail", 1, DISPLACED, 1200, 5, 4, CLR_CYAN),
DRGN_ARMR("red dragon scale mail",    1, FIRE_RES,    900, 5, 4, CLR_RED),
DRGN_ARMR("white dragon scale mail",  1, COLD_RES,    900, 5, 4, CLR_WHITE),
DRGN_ARMR("orange dragon scale mail", 1, FREE_ACTION,   900, 5, 4, CLR_ORANGE),
DRGN_ARMR("black dragon scale mail",  1, DISINT_RES, 1200, 5, 4, CLR_BLACK),
DRGN_ARMR("blue dragon scale mail",   1, SHOCK_RES,   900, 5, 4, CLR_BLUE),
DRGN_ARMR("green dragon scale mail",  1, POISON_RES,  900, 5, 4, CLR_GREEN),
DRGN_ARMR("yellow dragon scale mail", 1, ACID_RES,    900, 5, 4, CLR_YELLOW),

/* For now, only dragons leave these. */
/* 3.4.1: dragon scales left classified as "non-magic"; they confer
   magical properties but are produced "naturally" */
DRGN_ARMR("gray dragon scales",   0, ANTIMAGIC,  700, 9, 2, CLR_GRAY),
DRGN_ARMR("silver dragon scales", 0, REFLECTING, 700, 9, 2, DRAGON_SILVER),
DRGN_ARMR("shimmering dragon scales", 0, DISPLACED,  700, 9, 2, CLR_CYAN),
DRGN_ARMR("red dragon scales",    0, FIRE_RES,   500, 9, 2, CLR_RED),
DRGN_ARMR("white dragon scales",  0, COLD_RES,   500, 9, 2, CLR_WHITE),
DRGN_ARMR("orange dragon scales", 0, FREE_ACTION,  500, 9, 2, CLR_ORANGE),
DRGN_ARMR("black dragon scales",  0, DISINT_RES, 700, 9, 2, CLR_BLACK),
DRGN_ARMR("blue dragon scales",   0, SHOCK_RES,  500, 9, 2, CLR_BLUE),
DRGN_ARMR("green dragon scales",  0, POISON_RES, 500, 9, 2, CLR_GREEN),
DRGN_ARMR("yellow dragon scales", 0, ACID_RES,   500, 9, 2, CLR_YELLOW),
#undef DRGN_ARMR

ARMOR("plate mail", (char *)0, /*Needs encyc entry*/
	1, 0, 1, 0,	44, 5, 225, 600,  5, 5, 3, ARM_SUIT, IRON, HI_METAL),
ARMOR("high-elven plate", "runed plate mail", /*Needs encyc entry*/
	0, 0, 1, 0,	0, 5, 60, 	1200,  5, 5, 3, ARM_SUIT, MITHRIL, HI_METAL),
ARMOR("droven plate mail", "crested black plate", /*Needs encyc entry*/
	0, 0, 1, 0,	0, 5, 50, 	2000,  5, 5, 1, ARM_SUIT, MINERAL, CLR_BLACK),
ARMOR("elven toga", (char *)0, /*Needs encyc entry*//*Needs tile*/
	1, 0, 0, 0,	 0, 5,	 5,  100,10, 1, 2, ARM_SUIT, CLOTH, CLR_GREEN),
ARMOR("noble's dress", "armored black dress", /*Needs encyc entry*/
	0, 0, 1, 0,	0, 5,100, 2000,  6, 4, 3, ARM_SUIT, MINERAL, CLR_BLACK),
ARMOR("black dress", (char *)0, /*Needs encyc entry*/
	1, 0, 1, 0,	0, 5, 50,  500, 10, 0, 2, ARM_SHIRT, CLOTH, CLR_BLACK),
ARMOR("consort's suit", "loud foppish suit", /*Needs encyc entry*//*Needs tile*/
	0, 0, 1, 0,	0, 5, 10, 	1000, 10, 1, 1, ARM_SUIT, CLOTH, CLR_BRIGHT_MAGENTA),
ARMOR("gentleman's suit", "expensive clothes", /*Needs encyc entry*/
	0, 0, 1, 0,	0, 5, 10, 1000,  10, 1, 2, ARM_SUIT, CLOTH, CLR_BLACK),
ARMOR("gentlewoman's dress", "expensive dress", /*Needs encyc entry*/
	0, 0, 1, 0,	0, 6,150, 1000,  10, 1, 3, ARM_SUIT, BONE, CLR_RED), /*Specifically, whale bone*/
ARMOR("crystal plate mail", (char *)0, /*Needs encyc entry*/
	1, 0, 1, 0,	10, 5, 250, 2000,  7, 3, 0, ARM_SUIT, GLASS, CLR_CYAN), /*Best armor, AC wise*/
#ifdef TOURIST
ARMOR("bronze plate mail", (char *)0, /*Needs encyc entry*/
	1, 0, 1, 0,	20, 5, 225, 400,  6, 4, 3, ARM_SUIT, COPPER, HI_COPPER),
#else
ARMOR("bronze plate mail", (char *)0,
	1, 0, 1, 0,	35, 5, 225, 400,  6, 4, 3, ARM_SUIT, COPPER, HI_COPPER),
#endif
ARMOR("harmonium plate", "red-lacquered bladed armor",
	0, 0, 1, 0,	 0, 5, 225,   1,  6, 4, 3, ARM_SUIT, METAL, CLR_RED),
ARMOR("harmonium scale mail", "red-lacquered spiked scale mail",
	0, 0, 1, 0,	 0, 5, 225,   1,  8, 2, 3, ARM_SUIT, METAL, CLR_RED),
ARMOR("plasteel armor", "hard white armor", /*Needs encyc entry*//*Needs tile*/
	0, 0, 1, 0,	 0, 5, 100,  500, 7, 3, 3, ARM_SUIT, PLASTIC, CLR_WHITE),
// ARMOR("force armor", "gemstone-adorned clothing",
	// 0, 0, 1, 0,	 0, 5,  50, 1000, 9, 3, ARM_SUIT, GEMSTONE, CLR_BRIGHT_GREEN),
ARMOR("splint mail", (char *)0,
	1, 0, 1, 0,	62, 5, 200,  80,  7, 3, 1, ARM_SUIT, IRON, HI_METAL),
ARMOR("banded mail", (char *)0,
	1, 0, 1, 0,	72, 5, 175,  90,  7, 3, 0, ARM_SUIT, IRON, HI_METAL),
ARMOR("dwarvish mithril-coat", (char *)0,
	1, 0, 0, 0,	10, 1, 40, 240,   7, 3, 3, ARM_SUIT, MITHRIL, HI_METAL),
ARMOR("elven mithril-coat", (char *)0,
	1, 0, 0, 0,	15, 1, 20, 240,  7, 2, 3, ARM_SUIT, MITHRIL, HI_METAL),
ARMOR("chain mail", (char *)0,
	1, 0, 0, 0,	72, 5, 150,  75,  8, 3, 1, ARM_SUIT, IRON, HI_METAL),
ARMOR("droven chain mail", "crested black mail", /*Needs encyc entry*/
	0, 0, 0, 0,	0, 5, 50,  1000,  8, 3, 2, ARM_SUIT, MINERAL, HI_METAL),
ARMOR("orcish chain mail", "crude chain mail",
	0, 0, 0, 0,	20, 5, 150,  75,  8, 2, 1, ARM_SUIT, IRON, CLR_BLACK),
ARMOR("scale mail", (char *)0,
	1, 0, 0, 0,	72, 5, 125,  45,  8, 2, 0, ARM_SUIT, IRON, HI_METAL),
ARMOR("studded leather armor", (char *)0,
	1, 0, 0, 0,	72, 3, 100,  15,  9, 2, 1, ARM_SUIT, LEATHER, HI_LEATHER),
ARMOR("ring mail", (char *)0,
	1, 0, 0, 0,	72, 5, 125, 100,  9, 2, 0, ARM_SUIT, IRON, HI_METAL),
ARMOR("orcish ring mail", "crude ring mail",
	0, 0, 0, 0,	20, 5, 125,  80,  9, 1, 1, ARM_SUIT, IRON, CLR_BLACK),
ARMOR("leather armor", (char *)0,
	1, 0, 0, 0,	82, 3, 75,   5, 10, 2, 0, ARM_SUIT, LEATHER, HI_LEATHER),
ARMOR("leather jacket", (char *)0,
	1, 0, 0, 0,	12, 0,	30,  10, 10, 1, 2, ARM_SUIT, LEATHER, CLR_BLACK),
ARMOR("straitjacket", "long-sleeved jacket", /*Needs encyc entry*//*Needs tile*/
	0, 0, 0, 0,	0, 0,	30,  10, 10, 1, 2, ARM_SUIT, CLOTH, CLR_WHITE),
ARMOR("healer uniform","clean white clothes", /*Needs encyc entry*//*Needs tile*/
	0, 0, 0, SICK_RES,	0, 0,	30,  10, 10, 1, 0, ARM_SUIT, CLOTH, CLR_WHITE),
#ifdef TOURIST
/* shirts */
/*ARMOR("Hawaiian shorts", "flowery shorts and lei",
	1, 0, 0, 0,	 0, 0,	 5,   3, 10, 0, ARM_SUIT, CLOTH, CLR_MAGENTA),
*/
ARMOR("Hawaiian shirt", "flowery shirt",
	0, 0, 0, 0,	 10, 0,	 5,   3, 10, 0, 0, ARM_SHIRT, CLOTH, CLR_MAGENTA),
ARMOR("T-shirt", (char *)0, /*Needs encyc entry*/
	1, 0, 0, 0,	 5, 0,	 5,   2, 10, 0, 0, ARM_SHIRT, CLOTH, CLR_WHITE),
# ifdef CONVICT
ARMOR("striped shirt", (char *)0, /*Needs encyc entry*/
	1, 0, 0, 0,	 0, 0,	 5,   2, 10, 0, 0, ARM_SHIRT, CLOTH, CLR_GRAY),
# endif /* CONVICT */
#endif
/*Ruffled shirts are little different from other shirts*/
ARMOR("ruffled shirt", (char *)0, /*Needs encyc entry*/
	1, 0, 0, 0,	 0, 0,	 5,   2, 10, 0, 0, ARM_SHIRT, CLOTH, CLR_WHITE),
/* victorian underwear, on the other hand, inflicts a penalty to AC but grants MC 3 */
ARMOR("victorian underwear", "white dress",
	0, 0, 1, 		  0,	 0, 5,	 5,   10, 10, 2, 3, ARM_SHIRT, CLOTH, CLR_WHITE),
ARMOR("jumpsuit", "silvery clothes",/*Needs encyc entry*//*Needs tile*/
	0, 0, 1, REFLECTING,	 0, 5,	 5, 1000, 10, 1, 3, ARM_SUIT, PLASTIC, HI_SILVER),
ARMOR("bodyglove", "tight black clothes", /*Needs encyc entry*//*Needs tile*/
	0, 0, 1, SICK_RES,	 0, 5,	 5, 1000, 10, 0, 3, ARM_SHIRT, PLASTIC, CLR_BLACK),
/* cloaks */
/*  'cope' is not a spelling mistake... leave it be */
CLOAK("mummy wrapping", (char *)0,
		1, 0,	0,	    0, 0,  3,  2, 10, 0, 1, CLOTH, CLR_GRAY),
CLOAK("elven cloak", "faded pall",
		0, 1,	STEALTH,    7, 0, 10, 60,  9, 0, 3, CLOTH, CLR_BLACK),
CLOAK("droven cloak", "cobwebbed cloak", /*Needs encyc entry*/
		0, 1,	0,      1, 0, 10, 60,  10, 0, 3, CLOTH, CLR_GRAY),
CLOAK("orcish cloak", "coarse mantelet",
		0, 0,	0,	    8, 0, 10, 40, 10, 0, 2, CLOTH, CLR_BLACK),
CLOAK("dwarvish cloak", "hooded cloak",
		0, 0,	0,	    8, 0, 10, 50,10, 1, 2, CLOTH, CLR_BLUE),
CLOAK("oilskin cloak", "slippery cloak",
		0, 0,	0,	    8, 0, 10, 50,  9, 0, 3, CLOTH, HI_CLOTH),
CLOAK("robe", (char *)0,
		1, 1,	0,	    3, 0, 15, 50, 10, 2, 3, CLOTH, CLR_RED),
CLOAK("white faceless robe", (char *)0,
		1, 1,	0,	    0, 2, 20, 50, 10, 1, 3, CLOTH, CLR_WHITE),
CLOAK("black faceless robe", (char *)0,
		1, 1,	COLD_RES,	    0, 2, 20, 50, 10, 2, 3, CLOTH, CLR_BLACK),
CLOAK("smoky violet faceless robe", (char *)0,
		1, 1,	COLD_RES,	    0, 2, 20,500, 10, 3, 3, CLOTH, CLR_MAGENTA),
CLOAK("alchemy smock", "apron",
		0, 1,	POISON_RES, 9, 0, 10, 50, 10, 1, 3, CLOTH, CLR_WHITE),
CLOAK("Leo Nemaeus hide", "lion skin",
		0, 1,	HALF_PHDAM,	    0, 10, 60, 1200, 10, 5, 0, DRAGON_HIDE, HI_GOLD),
CLOAK("leather cloak", (char *)0,
		1, 0,	0,	    8, 0, 15, 40, 10, 2, 1, LEATHER, CLR_BROWN),
/* With shuffled appearances... */
CLOAK("cloak of protection", "tattered cape",
		0, 1,	PROTECTION, 9, 0, 10, 50,  7, 3, 3, CLOTH, HI_CLOTH),
CLOAK("cloak of invisibility", "opera cloak",
		0, 1,	INVIS,	   10, 0, 10, 60,  9, 0, 2, CLOTH, CLR_BLACK),
CLOAK("cloak of magic resistance", "ornamental cope",
		0, 1,	ANTIMAGIC,  2, 0, 10, 60, 10, 0, 3, CLOTH, CLR_WHITE),
CLOAK("cloak of displacement", "piece of cloth",
		0, 1,	DISPLACED, 10, 0, 10, 50,  9, 0, 2, CLOTH, CLR_BRIGHT_MAGENTA),

/* shields */
SHIELD("buckler", (char *)0,
		1, 0, 0, 0,	     6, 0, 30,	3,  9, 0, 0, WOOD, HI_WOOD),
SHIELD("elven shield", "blue and green shield",
		0, 0, 0, 0,	     2, 0,  3,	7,  8, 0, 2, WOOD, CLR_GREEN),
SHIELD("Uruk-hai shield", "white-handed shield",
		0, 0, 0, 0,	     2, 0, 50,	7,  8, 0, 1, IRON, HI_METAL),
SHIELD("orcish shield", "red-eyed shield",
		0, 0, 0, 0,	     2, 0, 50,	7,  9, 0, 0, IRON, CLR_RED),
SHIELD("kite shield", (char *)0,
		1, 0, 1, 0,	     6, 0,100, 10,  8, 0, 1, IRON, HI_METAL),
SHIELD_MATSPEC("roundshield", "round shield",
		0, 0, 1, 0,	     1, 0,120,  7,  8, 0, 1, COPPER, IDED, HI_COPPER),
SHIELD("dwarvish roundshield", "round shield",
		0, 0, 0, 0,	     4, 0, 80, 10,  7, 0, 1, IRON, HI_METAL),
SHIELD_MATSPEC("crystal shield", "shield", /*Needs encyc entry*//*Needs tile*/
		0, 0, 0, 0,	     0, 0,300,150,  9, 0, 0, GLASS,UNIDED, CLR_CYAN),
SHIELD("shield of reflection", "polished shield",
		0, 1, 0, REFLECTING, 3, 0, 50, 50,  8, 0, 0, SILVER, HI_SILVER),
/*#define SHIELD(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,metal,c) \
     ARMOR(name,desc,kn,mgc,blk,power,prob,delay,wt,cost,ac,can,ARM_SHIELD,metal,c) */
#define DRGN_SHIELD(name,mgc,power,cost,ac,dr,color)						\
	ARMOR(name,(char *)0,1,mgc,1,power,0,0,75,cost,ac,dr,0,ARM_SHIELD,DRAGON_HIDE,color)
/* 3.4.1: dragon scale mail reclassified as "magic" since magic is
   needed to create them */
DRGN_SHIELD("gray dragon scale shield", 1, ANTIMAGIC,  1200, 7, 0, CLR_GRAY),
DRGN_SHIELD("silver dragon scale shield", 1, REFLECTING, 1200, 7, 0, DRAGON_SILVER),
DRGN_SHIELD("shimmering dragon scale shield", 1, DISPLACED, 1200, 7, 0, CLR_CYAN),
DRGN_SHIELD("red dragon scale shield", 1, FIRE_RES,    900, 7, 0, CLR_RED),
DRGN_SHIELD("white dragon scale shield", 1, COLD_RES,    900, 7, 0, CLR_WHITE),
DRGN_SHIELD("orange dragon scale shield", 1, FREE_ACTION,   900, 7, 0, CLR_ORANGE),
DRGN_SHIELD("black dragon scale shield", 1, DISINT_RES, 1200, 7, 0, CLR_BLACK),
DRGN_SHIELD("blue dragon scale shield", 1, SHOCK_RES,   900, 7, 0, CLR_BLUE),
DRGN_SHIELD("green dragon scale shield", 1, POISON_RES,  900, 7, 0, CLR_GREEN),
DRGN_SHIELD("yellow dragon scale shield", 1, ACID_RES,   900, 7, 0, CLR_YELLOW),

/* gloves */
/* these have their color but not material shuffled, so the IRON must stay
 * CLR_BROWN (== HI_LEATHER)
 */
//define GLOVES(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,dr,can,metal,c) \
	ARMOR(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,dr,can,ARM_GLOVES,metal,c)
GLOVES_MATSPEC("crystal gauntlets", "gauntlets", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	   0, 2, 50, 400,  9, 0, 0, GLASS,UNIDED, CLR_CYAN),
GLOVES_MATSPEC("gauntlets", (char *)0, /*Needs encyc entry*//*Needs tile*/
		1, 0,  0,	   4, 2, 25, 10, 8, 2, 0, IRON,IDED|UNIDED, HI_METAL),
GLOVES("bronze gauntlets", (char *)0, /*Needs encyc entry*//*Needs tile*/
		1, 0,  0,	   0, 2, 25, 10, 8, 2, 0, COPPER, HI_COPPER),
GLOVES("long gloves", (char *)0,
		1, 0,  0,	   0, 1, 15,  8, 10, 2, 1, CLOTH, CLR_WHITE),
GLOVES("harmonium gauntlets", "red-lacquered hooked gauntlets", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	   0, 2, 40,  1, 9, 2, 0, METAL, CLR_RED),
GLOVES("high-elven gauntlets", "runed gauntlets", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	   0, 2,  5, 50, 8, 2, 0, MITHRIL, HI_METAL),
GLOVES("plasteel gauntlets", "hard white gauntlets", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	   0, 2, 15, 50,  8, 2, 0, PLASTIC, CLR_WHITE),
GLOVES_MATSPEC("gloves", "old gloves",
		0, 0,  0,	   8, 1, 10,  8, 10, 1, 0, LEATHER,IDED, HI_LEATHER),
GLOVES("gauntlets of fumbling", "padded gloves", /*"padded" should give +1 DR*/
		0, 1,  FUMBLING,   7, 1, 10, 50, 10, 1, 0, LEATHER, HI_LEATHER),
GLOVES("gauntlets of power", "riding gloves",
		0, 1,  0,	   7, 1, 30, 50,  9, 1, 0, IRON, CLR_BROWN),
GLOVES("orihalcyon gauntlets", "fighting gloves",/*Needs tile*/
		0, 1,  ANTIMAGIC,  7, 1, 50, 50,  8, 2, 0, METAL, CLR_BROWN),
GLOVES("gauntlets of dexterity", "fencing gloves",
		0, 1,  0,	   7, 1, 10, 50,  8, 0, 0, LEATHER, HI_LEATHER),

/* boots */
//define BOOTS(name,desc,kn,mgc,power,prob,delay,wt,cost,ac,dr,can,metal,c) \
	ARMOR(name,desc,kn,mgc,0,power,prob,delay,wt,cost,ac,dr,can,ARM_BOOTS,metal,c)
BOOTS("low boots", "walking shoes",
		0, 0,  0,	  25, 2, 10,  8, 10, 1, 0, LEATHER, HI_LEATHER),
BOOTS_MATSPEC("shoes", "hard shoes",
		0, 0,  0,	   7, 2, 50, 16,  9, 1, 0, IRON,IDED, HI_METAL),
BOOTS_MATSPEC("armored boots", "hard boots",
		0, 0,  0,	   0, 1, 75, 16,  8, 2, 1, COPPER,IDED, HI_COPPER),
BOOTS("harmonium boots", "red-lacquered boots",
		0, 0,  0,	   0, 1, 95,  1,  8, 2, 1, METAL, CLR_RED),
BOOTS("plasteel boots", "hard white boots", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	   0, 2, 25, 32,  8, 2, 1, PLASTIC, CLR_WHITE),
BOOTS("stilettos", "high-heeled shoes", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	   0, 1, 10, 60, 10, 0, 0, METAL, HI_METAL),
BOOTS("high boots", "jackboots",
		0, 0,  0,	  15, 2, 20, 12, 10, 2, 0, LEATHER, HI_LEATHER),
BOOTS("heeled boots", "tall boots",
		1, 0,  0,	   0, 2, 20, 12, 10, 2, 0, LEATHER, CLR_BLACK),
BOOTS_MATSPEC("crystal boots", "boots", /*Needs encyc entry*//*Needs tile*/
		0, 0,  0,	   0, 2,150,300,   9, 0, 0, GLASS,UNIDED, CLR_CYAN),
/* With shuffled appearances... */
BOOTS("speed boots", "combat boots",
		0, 1,  FAST,	  12, 2, 20, 50,  9, 0, 0, LEATHER, HI_LEATHER),
BOOTS("water walking boots", "jungle boots",
		0, 1,  WWALKING,  12, 2, 20, 50, 10, 1, 0, LEATHER, HI_LEATHER),
BOOTS("jumping boots", "hiking boots",
		0, 1,  JUMPING,   12, 2, 20, 50,  9, 1, 0, LEATHER, HI_LEATHER),
BOOTS("elven boots", "mud boots",
		0, 1,  STEALTH,   12, 2,  1,  8,  9, 0, 0, WOOD, CLR_BROWN),
BOOTS("kicking boots", "buckled boots",
		0, 1,  0,         12, 2, 15,  8,  9, 2, 0, IRON, CLR_BROWN),
BOOTS("fumble boots", "riding boots",
		0, 1,  FUMBLING,  12, 2, 20, 30, 10, 1, 0, LEATHER, HI_LEATHER),
BOOTS("flying boots", "snow boots",
		0, 1,  FLYING,12, 2, 15, 30,  9, 1, 0, LEATHER, HI_LEATHER),
#undef HELM
#undef CLOAK
#undef SHIELD
#undef GLOVES
#undef BOOTS
#undef ARMOR

#undef HELM_MATSPEC
#undef CLOAK_MATSPEC
#undef SHIELD_MATSPEC
#undef GLOVES_MATSPEC
#undef BOOTS_MATSPEC
#undef ARMOR_MATSPEC

/* rings ... */
#define RING(name,power,stone,cost,mgc,spec,mohs,metal,color) OBJECT( \
		OBJ(name,stone), \
		BITS(0,0,spec,0,mgc,spec,0,0,0,HARDGEM(mohs),0,P_NONE,metal,0), \
		power, RING_CLASS, 0, 0, 3, cost, 0, 0, 0, 0, 0, 15, color )
RING("wishes", 0, "black",            500, 1, 1, 4, PLATINUM, CLR_BLACK),
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
RING("fire resistance", FIRE_RES, "iron",
											200, 1, 0, 5, IRON, HI_METAL),
RING("cold resistance", COLD_RES, "brass",
											150, 1, 0, 4, COPPER, HI_COPPER),
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
RING("invisibility", INVIS, "wire",  		150, 1, 0, 5, IRON, HI_METAL),
RING("see invisible", SEE_INVIS, "engagement",
											150, 1, 0, 5, GOLD, HI_METAL),
RING("alacrity", FAST, "shiny",				100, 1, 0, 5, METAL, CLR_BRIGHT_CYAN),/*Needs tile*/
RING("protection from shape changers", PROT_FROM_SHAPE_CHANGERS, "black signet",
											100, 1, 0, 5, MITHRIL, CLR_BLACK),
#undef RING

/* amulets ... - THE Amulet comes last because it is special */
#define AMULET(name,desc,power,prob) OBJECT( \
		OBJ(name,desc), BITS(0,0,0,0,1,0,0,0,0,0,0,P_NONE,IRON,0), power, \
		AMULET_CLASS, prob, 0, 20, 150, 0, 0, 0, 0, 0, 20, HI_METAL )

AMULET("amulet of drain resistance","warped",   DRAIN_RES,   60),
AMULET("amulet of ESP",           "circular",   TELEPAT,    130),
AMULET("amulet of life saving",   "spherical",  LIFESAVED,   70),
AMULET("amulet of strangulation", "oval",       STRANGLED,  100),
AMULET("amulet of restful sleep", "triangular", SLEEPING,   100),
AMULET("amulet versus poison",    "pyramidal",  POISON_RES, 140),
AMULET("amulet of change",        "square",     0,          120),
AMULET("amulet versus curses",    "convex",     0,           60),/*Needs tile*/
						/* POLYMORPH */
AMULET("amulet of unchanging",    "concave",    UNCHANGING,	 45),
AMULET("amulet of nullify magic", "pentagonal",  NULLMAGIC,  45),/*Needs tile*/
AMULET("amulet of reflection",    "hexagonal",  REFLECTING,  70),
AMULET("amulet of magical breathing", "octagonal",      MAGICAL_BREATHING, 60),
OBJECT(OBJ("cheap plastic imitation of the Amulet of Yendor",
	"Amulet of Yendor"), BITS(0,0,1,0,0,0,0,0,0,0,0,0,PLASTIC,0), 0,
	AMULET_CLASS, 0, 0, 20,    0, 0, 0, 0, 0, 0,  1, HI_METAL),
OBJECT(OBJ("Amulet of Yendor",	/* note: description == name */
	"Amulet of Yendor"), BITS(0,0,1,0,1,0,1,1,0,0,0,0,MITHRIL,0), 0,
	AMULET_CLASS, 0, 0, 20, 30000, 0, 0, 0, 0, 0, 20, HI_METAL),
#undef AMULET

/* tools ... */
/* tools with weapon characteristics come last */
#define TOOLMASK(name,desc,prop,kn,mrg,mgc,chg,prob,wt,cost,mat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,mrg,chg,0,mgc,chg,0,0,0,0,0,P_NONE,mat,0), \
		prop, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, 0, wt, color )
#define TOOL(name,desc,kn,mrg,mgc,chg,prob,wt,cost,mat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,mrg,chg,0,mgc,chg,0,0,0,0,0,P_NONE,mat,0), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, 0, wt, color )
#define TOOL_MATSPEC(name,desc,kn,mrg,mgc,chg,prob,wt,cost,mat, shwmat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,mrg,chg,0,mgc,chg,0,0,0,0,0,P_NONE,mat,shwmat), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, 0, wt, color )
#define CONTAINER(name,desc,kn,mgc,chg,prob,wt,cost,mat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,0,chg,1,mgc,chg,0,0,0,0,0,P_NONE,mat,0), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, 0, wt, color )
#define CONTAINER_MATSPEC(name,desc,kn,mgc,chg,prob,wt,cost,mat,shwmat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,0,chg,1,mgc,chg,0,0,0,0,0,P_NONE,mat,shwmat), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, 0, wt, color )
#define WEPTOOL(name,desc,kn,mgc,chg,bi,prob,wt,cost,sdam,ldam,hitbon,typ,sub,mat,clr) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,0,1,chg,mgc,1,0,0,bi,0,typ,sub,mat,0), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, WP_GENERIC, 0, wt, clr )
#define WEPTOOL_MATSPEC(name,desc,kn,mgc,chg,bi,prob,wt,cost,sdam,ldam,hitbon,typ,sub,mat,shwmat,clr) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,0,1,chg,mgc,1,0,0,bi,0,typ,sub,mat,shwmat), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, sdam, ldam, hitbon, WP_GENERIC, 0, wt, clr )
/* containers */
CONTAINER("box", (char *)0,       1, 0, 0,  30,350,   8, WOOD, HI_WOOD),
CONTAINER("massive stone crate", (char *)0,
										1, 0, 0,   0,6000,  80, MINERAL,HI_MINERAL),/*Needs tile*/
CONTAINER("chest", (char *)0,           1, 0, 0,  30,600,  16, WOOD, HI_WOOD),
CONTAINER("magic chest", "big chest with 10 keyholes",
										0, 1, 0,  15,1001,7000, METAL, CLR_BRIGHT_MAGENTA),/*Needs tile*/
CONTAINER("ice box", (char *)0,         1, 0, 0,   5, 250,  42, PLASTIC, CLR_WHITE),
CONTAINER("sack", "bag",                0, 0, 0,  20,  15,   2, CLOTH, HI_CLOTH),
CONTAINER("oilskin sack", "bag",        0, 0, 0,  20,  15, 100, CLOTH, HI_CLOTH),
CONTAINER("bag of holding", "bag",      0, 1, 0,  20,  15, 100, CLOTH, HI_CLOTH),
CONTAINER("bag of tricks", "bag",       0, 1, 1,  20,  15, 100, CLOTH, HI_CLOTH),
#define HOSTAGE(name,desc,kn,mgc,chg,prob,ntrtn,wt,cost,mat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,0,chg,1,mgc,chg,0,0,0,0,0,P_NONE,mat), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, ntrtn, 0, wt, color )

//HOSTAGE("distressed princess", (char *)0,           1, 0, 0,  0, 350,.9*1450,  1600, CLOTH, CLR_WHITE),
#undef HOSTAGE
#undef CONTAINER
#undef CONTAINER_MATSPEC
//OBJECT(obj,bits,prp,sym,prob,dly,wt,cost,sdam,ldam,oc1,oc2,oc3,nut,color) \
//	{0, 0, (char *)0, bits, prp, sym, dly, COLOR_FIELD(color) \
//	 prob, wt, cost, sdam, ldam, oc1, oc2, oc3, nut}

/* lock opening tools */
//define TOOL(name,desc,kn,mrg,mgc,chg,prob,wt,cost,mat,color) \
	OBJECT( OBJ(name,desc), \
		BITS(kn,mrg,chg,0,mgc,chg,0,0,0,0,0,P_NONE,mat), \
		0, TOOL_CLASS, prob, 0, \
		wt, cost, 0, 0, 0, 0, 0, wt, color )
TOOL("skeleton key", "key",     0, 0, 0, 0,  80,  3,  10, IRON, HI_METAL),
TOOL_MATSPEC("universal key", "key",     0, 0, 0, 0,  0,  3,  10, SILVER,UNIDED, HI_SILVER),
#ifdef TOURIST
TOOL("lock pick", (char *)0,    1, 0, 0, 0,  60,  4,  20, IRON, HI_METAL),
TOOL("credit card", (char *)0,  1, 0, 0, 0,   0,  1,  10, PLASTIC, CLR_WHITE),
#else
TOOL("lock pick", (char *)0,    1, 0, 0, 0,  60,  4,  20, IRON, HI_METAL),
#endif
/* light sources */
TOOL("tallow candle", "candle", 0, 1, 0, 0,  15,  2,  10, WAX, CLR_WHITE),
TOOL("wax candle", "candle",    0, 1, 0, 0,   5,  2,  20, WAX, CLR_WHITE),
TOOL("candle of invocation", "runed candle", 
                                0, 0, 1, 0,  15,  2,  50, WAX, CLR_ORANGE),
TOOL("brass lantern", (char *)0,1, 0, 0, 0,  20, 30,  12, COPPER, CLR_YELLOW),
TOOL("oil lamp", "lamp",        0, 0, 0, 0,  30, 20,  10, COPPER, CLR_YELLOW),
TOOL("magic lamp", "lamp",      0, 0, 1, 0,  15, 20,  50, COPPER, CLR_YELLOW),
// TOOL("shadowlander's torch", "black torch",
								// 0, 0, 1, 0,  10, 20,  50, WOOD, CLR_BLACK),
/* other tools */
#ifdef TOURIST
TOOL("expensive camera", (char *)0,
				1, 0, 0, 1,  15, 12, 200, PLASTIC, CLR_BLACK),
TOOL("mirror", "looking glass", 0, 0, 0, 0,  45, 13,  10, GLASS, HI_SILVER),
#else
TOOL("mirror", "looking glass", 0, 0, 0, 0,  60, 13,  10, GLASS, HI_SILVER),
#endif
TOOL("crystal ball", "glass orb",
								0, 0, 1, 1,  15, 50, 200, GLASS, HI_GLASS),
TOOL("sensor pack", "rigid box", /*Needs encyc entry*//*Needs tile*/
								0, 0, 1, 1,   0, 15,2000, PLASTIC,CLR_WHITE),
TOOL("hypospray", "hammer-shaped device", /*Needs encyc entry*//*Needs tile*/
								0, 0, 1, 0,   0, 15, 500, PLASTIC,CLR_GRAY),
TOOL("hypospray ampule", "hard grey bottle", /*Needs encyc entry*//*Needs tile*/
								0, 0, 1, 0,   0,  1,  50, PLASTIC,CLR_GRAY),
TOOL("mask", (char *)0,			1, 0, 0, 0,  10, 10,  80, LEATHER, CLR_WHITE),
TOOLMASK("R'lyehian faceplate", "ebon pane", POISON_RES,/*Needs tile*/
								0, 0, 1, 0,   0, 15, 200, GLASS, CLR_BLACK),
TOOL("lenses", (char *)0,		1, 0, 0, 0,   5,  3,  80, GLASS, HI_GLASS), /*Needs encyc entry*/
TOOL("blindfold", (char *)0,    1, 0, 0, 0,  45,  2,  20, CLOTH, CLR_GRAY),
TOOL("android visor", "black blindfold",
								0, 0, 0, 0,   0,  2,  40, CLOTH, CLR_BLACK),
TOOL("towel", (char *)0,        1, 0, 0, 0,  45,  2,  50, CLOTH, CLR_MAGENTA),
#ifdef STEED
TOOL("saddle", (char *)0,       1, 0, 0, 0,   5,200, 150, LEATHER, HI_LEATHER),
TOOL("leash", (char *)0,        1, 0, 0, 0,  65, 12,  20, LEATHER, HI_LEATHER),
#else
TOOL("leash", (char *)0,        1, 0, 0, 0,  70, 12,  20, LEATHER, HI_LEATHER),
#endif
TOOL("stethoscope", (char *)0,  1, 0, 0, 0,  25,  4,  75, IRON, HI_METAL),
TOOL("tinning kit", (char *)0,  1, 0, 0, 1,  15,100,  30, IRON, HI_METAL),
TOOL("bullet fabber", "white box with a yellow fiddly bit",/*Needs tile*/
								0, 0, 1, 0,   0, 20,  30, PLASTIC, CLR_WHITE),
TOOL("upgrade kit", (char *)0,  1, 0, 0, 0,  40,100,  30, COPPER, HI_COPPER),/*Needs encyc entry*//*Needs tile*/
TOOL("power pack", "little white cube", /*Needs encyc entry*//*Needs tile*/
								0, 1, 1, 0,   0,  1,  300, PLASTIC, CLR_WHITE),
TOOL("tin opener", (char *)0,   1, 0, 0, 0,  20,  4,  30, IRON, HI_METAL),
TOOL("can of grease", (char *)0,1, 0, 0, 1,  15, 15,  20, IRON, HI_METAL),
TOOL("figurine", (char *)0,     1, 0, 1, 0,  20, 50,  80, MINERAL, HI_MINERAL),
TOOL("effigy",   (char *)0,     1, 1, 1, 0,  20,  5,  80, LEATHER, HI_LEATHER),
TOOL("magic marker", (char *)0, 1, 0, 1, 1,  15,  2,  50, PLASTIC, CLR_RED),
/* traps */
TOOL("land mine",(char *)0,     1, 0, 0, 0,   0,300, 180, IRON, CLR_RED),
TOOL("beartrap", (char *)0,     1, 0, 0, 0,   0,200,  60, IRON, HI_METAL),
/* instruments */
/* some code in invent.c and obj.h requires wooden flute .. drum of earthquake to be
   consecutive, with the wooden flute first and drum of earthquake last */
TOOL("tin whistle", "whistle",  0, 0, 0, 0,  60,  3,  10, METAL, HI_METAL),
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
	1, 0, 0, 0, 20, 80,   50,  6,  3, 0, PIERCE,  P_PICK_AXE, IRON, HI_METAL),
WEPTOOL("seismic hammer", "dull metallic hammer",
	0, 1, 1, 1,  0,150,  250, 12, 10,-5, WHACK,  P_HAMMER,  METAL, HI_METAL),
/*
 * Torches work as clubs
 */
WEPTOOL("torch", (char *)0,
	1, 0, 0, 0, 15, 50,  5,  6,  3, 0, WHACK,   P_CLUB, WOOD, HI_WOOD),
WEPTOOL("shadowlander's torch", "black torch",
	0, 0, 0, 0, 10, 50, 50,  6,  3, 0, WHACK,   P_CLUB, WOOD, CLR_BLACK),
WEPTOOL("sunrod", "gold rod",
	1, 0, 0, 0,  5, 50, 50,  6,  3, 0, WHACK,   P_MACE, GOLD, HI_GOLD),
/* 
 * Lightsabers get 3x dice when lit 
 */
WEPTOOL_MATSPEC("lightsaber", "sword hilt", /*Needs (better) encyc entry*/
	0, 1, 1, 0,  0, 10, 500, 8,  8, -3, SLASH|E, P_SABER, SILVER,IDED|UNIDED, HI_SILVER),
WEPTOOL_MATSPEC("beamsword",  "broadsword hilt", /*Needs encyc entry*/
	0, 1, 1, 0,  0, 20, 500, 10,10, -3, SLASH|E, P_BROAD_SWORD, GOLD,IDED|UNIDED, HI_GOLD),
WEPTOOL_MATSPEC("double lightsaber",  "long grip", /*Needs encyc entry*//*Needs tile*/
	0, 1, 1, 1,  0, 30,1000, 10,10, -6, SLASH|E, P_QUARTERSTAFF, PLATINUM,IDED|UNIDED, HI_SILVER),
WEPTOOL_MATSPEC("grappling hook", "hook",
	0, 0, 0, 0,  4,  30,  50,  2,  6, 0, WHACK,  P_FLAIL, IRON, UNIDED, HI_METAL),
WEPTOOL("shepherd's crook", "bent staff",
	0, 0, 0, 1,  1,  30,   5,  6,  4, 0, WHACK, P_QUARTERSTAFF, WOOD, HI_WOOD),
/* 3.4.1: unicorn horn left classified as "magic" */
WEPTOOL("unicorn horn", (char *)0,
	1, 1, 0, 0,  0,  20, 100, 12, 12, 0, PIERCE, P_UNICORN_HORN, BONE, CLR_WHITE),
WEPTOOL("spoon", (char *)0, /*Needs encyc entry*//*Needs tile*/
	1, 0, 0, 0,  0,  1,    1,  1,  1, 0, PIERCE,  P_KNIFE, IRON, HI_METAL),

/* two special unique artifact "tools" */
OBJECT(OBJ("Candelabrum of Invocation", "candelabrum"),
		BITS(0,0,1,0,1,0,1,1,0,0,0,P_NONE,GOLD,0), 0,
		TOOL_CLASS, 0, 0,10, 5000, 0, 0, 0, 0, 0, 200, HI_GOLD),
OBJECT(OBJ("Bell of Opening", "bell"),
		BITS(0,0,1,0,1,1,1,1,0,0,0,P_NONE,SILVER,UNIDED), 0,
		TOOL_CLASS, 0, 0,10, 5000, 0, 0, 0, 0, 0, 50, HI_SILVER),
#undef TOOL
#undef WEPTOOL
#undef TOOL_MATSPEC
#undef WEPTOOL_MATSPEC

/* Comestibles ... */
#define FOOD(name,prob,delay,wt,unk,tin,nutrition,color) OBJECT( \
		OBJ(name,(char *)0), BITS(1,1,unk,0,0,0,0,0,0,0,0,P_NONE,tin,0), 0, \
		FOOD_CLASS, prob, delay, \
		wt, nutrition/20 + 5, 0, 0, 0, 0, 0, nutrition, color )
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
FOOD("massive chunk of meat",   0,20,400, 0, FLESH,2000, CLR_BROWN),
/* special case because it's not mergable */
OBJECT(OBJ("meat ring", (char *)0),
    BITS(1,0,0,0,0,0,0,0,0,0,0,0,FLESH,0),
    0, FOOD_CLASS, 0, 1, 5, 1, 0, 0, 0, 0, 0, 5, CLR_BROWN),

/* Body parts.... eeeww */
FOOD("eyeball",              0, 1,  0, 0, FLESH,  10, CLR_WHITE),/*Needs tile*/
FOOD("severed hand",         0, 1,  0, 0, FLESH,  40, CLR_BROWN),/*Needs tile*/

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
FOOD("lump of royal jelly",	0, 1,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("lump of soldier's jelly",  
							0, 1,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("lump of dancer's jelly",
							0, 1,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("lump of philosopher's jelly",
							0, 1,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("lump of priestess's jelly",
							0, 1,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("lump of rhetor's jelly",
							0,  1,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("honeycomb",			0,  4, 10, 0, VEGGY, 800, CLR_YELLOW),
FOOD("cream pie",           25, 1, 10, 0, VEGGY, 100, CLR_WHITE),
FOOD("candy bar",           13, 1,  2, 0, VEGGY, 100, CLR_BROWN),
FOOD("fortune cookie",      55, 1,  1, 0, VEGGY,  40, CLR_YELLOW),
FOOD("pancake",             25, 2,  2, 0, VEGGY, 200, CLR_YELLOW),
FOOD("lembas wafer",        20, 2,  5, 0, VEGGY, 800, CLR_WHITE),
FOOD("cram ration",         20, 3, 15, 0, VEGGY, 600, HI_ORGANIC),
FOOD("food ration",        380, 5, 20, 0, VEGGY, 800, HI_ORGANIC),
FOOD("protein pill",         0, 1,  1, 0, VEGGY, 800, HI_ORGANIC), /*Needs encyc entry*//*Needs tile*/
FOOD("K-ration",             0, 1, 10, 0, VEGGY, 400, HI_ORGANIC),
FOOD("C-ration",             0, 1, 10, 0, VEGGY, 300, HI_ORGANIC),
FOOD("tin",                 75, 0, 10, 1, METAL,   0, HI_METAL),
#undef FOOD

/* potions ... */
#define POTION(name,desc,mgc,power,prob,cost,color) OBJECT( \
		OBJ(name,desc), BITS(0,1,0,0,mgc,0,0,0,0,0,0,P_NONE,GLASS,0), power, \
		POTION_CLASS, prob, 0, 20, cost, 0, 0, 0, 0, 0, 10, color )
POTION("gain ability", "ruby",          1, 0,          40, 300, CLR_RED),
POTION("restore ability", "pink",       1, 0,          40, 100, CLR_BRIGHT_MAGENTA),
POTION("confusion", "orange",           1, CONFUSION,  33, 100, CLR_ORANGE),
POTION("blindness", "yellow",           1, BLINDED,    33, 150, CLR_YELLOW),
POTION("paralysis", "emerald",          1, 0,          32, 300, CLR_BRIGHT_GREEN),
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
POTION("acid", "white",                 0, 0,          32, 250, CLR_WHITE),
POTION("oil", "murky",                  0, 0,          30, 250, CLR_BROWN),
POTION("amnesia", "sparkling",          1, 0,          8,  100, CLR_CYAN),
POTION("starlight", "dimly-shining",    1, 0,          4,  250, CLR_BRIGHT_CYAN),
POTION("water", "clear",                0, 0,          80, 100, CLR_CYAN),
POTION("blood", "blood-red",            0, 0,          18, 50,  CLR_RED),
#undef POTION

/* scrolls ... */
#define SCROLL(name,text,mgc,prob,cost) OBJECT( \
		OBJ(name,text), BITS(0,1,0,0,mgc,0,0,0,0,0,0,P_NONE,PAPER,0), 0, \
		SCROLL_CLASS, prob, 0, 5, cost, 0, 0, 0, 0, 0, 6, HI_PAPER )
	SCROLL("enchant armor",         "ZELGO MER",            1,  59,  80),
	SCROLL("destroy armor",         "JUYED AWK YACC",       1,  35, 100),
	SCROLL("confuse monster",       "NR 9",                 1,  41, 100),
	SCROLL("scare monster",         "XIXAXA XOXAXA XUXAXA", 1,  32, 100),
	SCROLL("remove curse",          "PRATYAVAYAH",          1,  60,  80),
	SCROLL("enchant weapon",        "DAIYEN FOOELS",        1,  74,  60),
	SCROLL("create monster",        "LEP GEX VEN ZEA",      1,  41, 200),
	SCROLL("taming",                "PRIRUTSENIE",          1,  14, 200),
	SCROLL("genocide",              "ELBIB YLOH",           1,  13, 300),
	SCROLL("light",                 "VERR YED HORRE",       1,  71,  50),
	SCROLL("teleportation",         "VENZAR BORGAVVE",      1,  51, 100),
	SCROLL("gold detection",        "THARR",                1,  30, 100),
	SCROLL("food detection",        "YUM YUM",              1,  23, 100),
	SCROLL("identify",              "KERNOD WEL",           1, 169,  20),
	SCROLL("magic mapping",         "ELAM EBOW",            1,  41, 100),
	SCROLL("amnesia",               "DUAM XNAHT",           1,  27, 200),
	SCROLL("fire",                  "ANDOVA BEGARIN",       1,  23, 100),
	SCROLL("earth",                 "KIRJE",                1,  16, 200),
	SCROLL("punishment",            "VE FORBRYDERNE",       1,  12, 300),
	SCROLL("charging",              "HACKEM MUCHE",         1,  14, 300),
	SCROLL("stinking cloud",		"VELOX NEB",            1,  13, 300),
	SCROLL("ward",					"TLASFO SENIL",         1,  55, 300),
	SCROLL("warding",				"RW NW PRT M HRW",      1,  13, 300),
	SCROLL("antimagic",				"KARSUS",     		    1,  18, 250),
	SCROLL("resistance",			"DESREVER TSEPMET",		1,  34, 250),
	SCROLL("consecration",			"HLY HLS",      		1,   0,3000),
	SCROLL((char *)0,		"FOOBIE BLETCH",        1,   0, 100),
	SCROLL((char *)0,		"TEMOV",                1,   0, 100),
	SCROLL((char *)0,		"GARVEN DEH",           1,   0, 100),
	SCROLL((char *)0,		"READ ME",              1,   0, 100),
	/* these must come last because they have special descriptions */
#ifdef MAIL
	SCROLL("mail",                  "stamped",          0,   0,   0),
#endif
	SCROLL("blank paper",           "unlabeled",        0,  21,  60),
#undef SCROLL
#define GOLD_SCROLL(name,text,mgc,prob,cost) OBJECT( \
		OBJ(name,text), BITS(0,1,0,0,mgc,0,0,0,0,0,0,P_NONE,GOLD,0), 0, \
		SCROLL_CLASS, prob, 0, 50, cost, 0, 0, 0, 0, 0, 6, HI_GOLD )
	GOLD_SCROLL("Gold Scroll of Law", "golden",        0,  0,  10), /* Shopkeepers aren't interested in these */
#undef GOLD_SCROLL

/* spellbooks ... */
#define SPELL(name,desc,sub,prob,delay,level,mgc,dir,color) OBJECT( \
		OBJ(name,desc), BITS(0,0,0,0,mgc,0,0,0,0,0,dir,sub,PAPER,0), 0, \
		SPBOOK_CLASS, prob, delay, \
		50, level*100, 0, 0, 0, level, 0, 20, color )
SPELL("dig",             "parchment",   P_MATTER_SPELL, 20,  6, 5, 1, RAY,       HI_PAPER),
SPELL("magic missile",   "vellum",      P_ATTACK_SPELL, 45,  2, 2, 1, RAY,       HI_PAPER),
SPELL("fireball",        "ragged",      P_ATTACK_SPELL, 15,  4, 4, 1, RAY,       HI_PAPER),
SPELL("cone of cold",    "dog eared",   P_ATTACK_SPELL, 10,  7, 4, 1, RAY,       HI_PAPER),
SPELL("sleep",           "mottled",     P_ENCHANTMENT_SPELL, 50,  1, 1, 1, RAY,  HI_PAPER),
SPELL("finger of death", "stained",     P_ATTACK_SPELL,  5, 10, 7, 1, RAY,       HI_PAPER),
SPELL("lightning bolt",  "storm-hued",  P_ATTACK_SPELL,  0,  8, 5, 1, RAY,       CLR_BLUE),
SPELL("poison spray",    "snakeskin",  	P_MATTER_SPELL,  0,  8, 4, 1, RAY,       CLR_GREEN),/*Needs tile*/
SPELL("acid splash",     "acid green", 	P_MATTER_SPELL, 10,  8, 4, 1, RAY,       CLR_BRIGHT_GREEN),/*Needs tile*/
SPELL("light",           "cloth",       P_DIVINATION_SPELL, 45,  1, 1, 1, NODIR,     HI_CLOTH),
SPELL("fire storm",		 "flame-red",   P_ATTACK_SPELL,  5,  8, 6, 1, NODIR,     CLR_RED),
SPELL("blizzard",        "snow white",  P_ATTACK_SPELL,  0,  8, 6, 1, NODIR,     CLR_WHITE),
SPELL("detect monsters", "leather",     P_DIVINATION_SPELL, 43,  2, 2, 1, NODIR,     HI_LEATHER),
SPELL("healing",         "white",       P_HEALING_SPELL, 40,  2, 1, 1, IMMEDIATE, CLR_WHITE),
SPELL("lightning storm", "ocean blue",  P_ATTACK_SPELL,  0,  9, 7, 1, NODIR,     CLR_BLUE),
SPELL("knock",           "pink",        P_MATTER_SPELL, 30,  1, 1, 1, IMMEDIATE, CLR_BRIGHT_MAGENTA),
SPELL("force bolt",      "red",         P_ATTACK_SPELL, 35,  2, 1, 1, IMMEDIATE, CLR_RED),
SPELL("confuse monster", "orange",      P_ENCHANTMENT_SPELL, 30,  2, 2, 1, IMMEDIATE, CLR_ORANGE),
SPELL("cure blindness",  "yellow",      P_HEALING_SPELL, 25,  2, 2, 1, IMMEDIATE, CLR_YELLOW),
SPELL("drain life",      "velvet",      P_ATTACK_SPELL, 10,  2, 2, 1, IMMEDIATE, CLR_MAGENTA),
SPELL("slow monster",    "light green", P_ENCHANTMENT_SPELL, 30,  2, 2, 1, IMMEDIATE, CLR_BRIGHT_GREEN),
SPELL("wizard lock",     "dark green",  P_MATTER_SPELL, 25,  3, 2, 1, IMMEDIATE, CLR_GREEN),
// SPELL("create monster",  "turquoise",   P_CLERIC_SPELL, 35,  3, 2, 1, NODIR,     CLR_BRIGHT_CYAN),
SPELL("turn undead",     "copper",      P_CLERIC_SPELL, 25,  3, 2, 1, IMMEDIATE, HI_COPPER),
SPELL("detect food",     "cyan",        P_DIVINATION_SPELL, 30,  3, 2, 1, NODIR,     CLR_CYAN),
SPELL("cause fear",      "light blue",  P_ENCHANTMENT_SPELL, 25,  3, 3, 1, NODIR,     CLR_BRIGHT_BLUE),
SPELL("clairvoyance",    "dark blue",   P_DIVINATION_SPELL, 15,  3, 3, 1, NODIR,     CLR_BLUE),
SPELL("cure sickness",   "indigo",      P_HEALING_SPELL, 32,  3, 3, 1, NODIR,     CLR_BLUE),
SPELL("pacify monster",  "fuchsia",		P_ENCHANTMENT_SPELL, 10, 3, 3, 1, IMMEDIATE, CLR_MAGENTA),
SPELL("charm monster",   "magenta",     P_ENCHANTMENT_SPELL, 10,  4, 5, 1, IMMEDIATE, CLR_MAGENTA),
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
SPELL("polymorph",       "silver",      P_MATTER_SPELL, 10,  8, 7, 1, IMMEDIATE, HI_SILVER),
SPELL("teleport away",   "gold",        P_ESCAPE_SPELL, 15,  6, 6, 1, IMMEDIATE, HI_GOLD),
SPELL("create familiar", "glittering",  P_CLERIC_SPELL, 10,  7, 6, 1, NODIR,     CLR_WHITE),
SPELL("cancellation",    "shining",     P_MATTER_SPELL, 15,  8, 6, 1, IMMEDIATE, CLR_WHITE),
SPELL("protection",	     "dull",        P_CLERIC_SPELL, 18,  3, 1, 1, NODIR,     HI_PAPER),
SPELL("antimagic shield","ebony",		P_CLERIC_SPELL, 10,  6, 5, 1, IMMEDIATE, CLR_BLACK),
SPELL("jumping",	     "thin",        P_ESCAPE_SPELL, 20,  3, 1, 1, IMMEDIATE, HI_PAPER),
SPELL("stone to flesh",	 "thick",       P_HEALING_SPELL, 15,  1, 3, 1, IMMEDIATE, HI_PAPER),
#if 0	/* DEFERRED */
SPELL("flame sphere",    "canvas",      P_MATTER_SPELL, 20,  2, 1, 1, NODIR, CLR_BROWN),
SPELL("freeze sphere",   "hardcover",   P_MATTER_SPELL, 20,  2, 1, 1, NODIR, CLR_BROWN),
#endif
/* blank spellbook must come last because it retains its description */
SPELL("blank paper",     "plain",       P_NONE, 18,  0, 0, 0, 0,         HI_PAPER),
/* a special, one of a kind, spellbook */
OBJECT(OBJ("Book of the Dead", "papyrus"), BITS(0,0,1,0,1,0,1,1,0,0,0,P_NONE,PAPER,0), 0,
	SPBOOK_CLASS, 0, 0,20, 10000, 0, 0, 0, 7, 0, 20, HI_PAPER),
OBJECT(OBJ("secrets", "ragged leather"), BITS(0,0,1,0,1,0,1,1,0,0,0,P_NONE,PAPER,0), 0,
	SPBOOK_CLASS, 0, 0,20, 10000, 0, 0, 0, 7, 0, 20, CLR_BROWN),
#undef SPELL

/* wands ... */
#define WAND(name,typ,prob,cost,mgc,dir,metal,color) OBJECT( \
		OBJ(name,typ), BITS(0,0,1,0,mgc,1,0,0,0,0,dir,P_NONE,metal,0), 0, \
		WAND_CLASS, prob, 0, 7, cost, 0, 0, 0, 0, 0, 30, color )
WAND("light",          "glass",    90, 100, 1, NODIR,     GLASS,    CLR_WHITE),/*Needs tile?*/
WAND("darkness",       "obsidian", 10, 100, 1, NODIR,     OBSIDIAN_MT,    CLR_BLACK),/*Needs tile*/
WAND("wishing",        "dragon-bone",
					0, 500, 1, NODIR,     BONE,     CLR_WHITE),	/* should not exist */
WAND("secret door detection", "balsa",
				   50, 150, 1, NODIR,	  WOOD,     HI_WOOD),
WAND("enlightenment",  "crystal",  18, 150, 1, NODIR,     GLASS,    HI_GLASS),
WAND("create monster", "maple",    42, 200, 1, NODIR,     WOOD,     HI_WOOD),
WAND("nothing",        "oak",      25, 100, 0, IMMEDIATE, WOOD,     HI_WOOD),
WAND("striking",       "ebony",    75, 150, 1, IMMEDIATE, WOOD,     HI_WOOD),
WAND("draining",       "ceramic",   5, 175, 1, IMMEDIATE, MINERAL,  HI_MINERAL),
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
WAND("death",          "long",      5, 500, 1, RAY,       IRON,     HI_METAL),
WAND("lightning",      "curved",   40, 175, 1, RAY,       IRON,     HI_METAL),
WAND((char *)0,        "pine",      0, 150, 1, 0,         WOOD,     HI_WOOD),
WAND((char *)0,        "forked",    0, 150, 1, 0,         WOOD,     HI_WOOD),
WAND((char *)0,        "spiked",    0, 150, 1, 0,         IRON,     HI_METAL),
WAND((char *)0,        "jeweled",   0, 150, 1, 0,         IRON,     HI_MINERAL),
#undef WAND

/* coins ... - so far, gold is all there is */
#define COIN(name,prob,metal,worth) OBJECT( \
		OBJ(name,(char *)0), BITS(0,1,0,0,0,0,0,0,0,0,0,P_NONE,metal,0), 0, \
		COIN_CLASS, prob, 0, 1, worth, 0, 0, 0, 0, 0, 0, HI_GOLD )
	COIN("gold piece",      1000, GOLD,1),
#undef COIN

/* gems ... - includes stones and rocks but not boulders */
#define GEM(name,desc,prob,wt,gval,nutr,mohs,glass,color) OBJECT( \
	    OBJ(name,desc), \
	    BITS(0,1,0,0,0,0,0,0,0,HARDGEM(mohs),PIERCE,-P_SLING,glass,0), 0, \
	    GEM_CLASS, prob, 0, 1, gval, 3, 3, 0, WP_GENERIC, 0, nutr, color )
#define ROCK(name,desc,kn,prob,wt,gval,sdam,ldam,hitbon,mgc,nutr,mohs,glass,color) OBJECT( \
	    OBJ(name,desc), \
	    BITS(kn,1,0,0,mgc,0,0,0,0,HARDGEM(mohs),WHACK,-P_SLING,glass,0), 0, \
	    GEM_CLASS, prob, 0, wt, gval, sdam, ldam, hitbon, WP_GENERIC, 0, nutr, color )
#define FLINT(name,desc,kn,prob,wt,gval,sdam,ldam,hitbon,mgc,nutr,mohs,glass,color) OBJECT( \
	    OBJ(name,desc), \
	    BITS(kn,1,0,0,mgc,0,0,0,0,HARDGEM(mohs),SLASH,-P_SLING,glass,0), 0, \
	    GEM_CLASS, prob, 0, wt, gval, sdam, ldam, hitbon, WP_GENERIC, 0, nutr, color )
GEM("magicite crystal","brilliant blue",1, 1, 9999, 15, 11, GEMSTONE, CLR_BRIGHT_BLUE),/*Needs tile*/
GEM("dilithium crystal", "white",      2,  1, 4500, 15,  5, GEMSTONE, CLR_WHITE),
GEM("diamond", "white",                3,  1, 4000, 15, 10, GEMSTONE, CLR_WHITE),
//3
GEM("star sapphire", "blue",           2,  1, 3750, 15,  9, GEMSTONE, CLR_BLUE),/*Needs tile*/
GEM("ruby", "red",                     4,  1, 3500, 15,  9, GEMSTONE, CLR_RED),
GEM("jacinth", "orange",               3,  1, 3250, 15,  9, GEMSTONE, CLR_ORANGE),
//6
GEM("sapphire", "blue",                4,  1, 3000, 15,  9, GEMSTONE, CLR_BLUE),
GEM("black opal", "black",             3,  1, 2500, 15,  8, GEMSTONE, CLR_BLACK),
GEM("emerald", "green",                3,  1, 2500, 15,  8, GEMSTONE, CLR_GREEN),
//9
GEM("turquoise", "green",              4,  1, 2000, 15,  6, GEMSTONE, CLR_GREEN),
GEM("morganite", "pink",               4,  1, 2000, 15,  6, GEMSTONE, CLR_ORANGE),
//11 Note: Only first 11 gems are affected by dungeon level
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
GEM("violet fluorite", "violet",       4,  1,  400, 15,  4, GEMSTONE, CLR_MAGENTA),/*Needs tile*/
GEM("blue fluorite", "blue",           4,  1,  400, 15,  4, GEMSTONE, CLR_BLUE),/*Needs tile*/
GEM("white fluorite", "white",         4,  1,  400, 15,  4, GEMSTONE, CLR_WHITE),/*Needs tile*/
GEM("green fluorite", "green",         4,  1,  400, 15,  4, GEMSTONE, CLR_GREEN),/*Needs tile*/
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
ROCK("luckstone", "gray",	0, 10,   1, 60, 6, 6, 20, 1, 10, 7, MINERAL, CLR_GRAY),
ROCK("loadstone", "gray",	0, 10, 500,  1,30,30, -5, 1, 10, 6, MINERAL, CLR_GRAY),
ROCK("touchstone", "gray",	0,  8,   1, 45, 6, 6,  0, 1, 10, 6, MINERAL, CLR_GRAY),
FLINT("flint", "gray",		0, 10,   1,  1, 6, 6,  2, 0, 10, 7, MINERAL, CLR_GRAY),
ROCK("chunk of unrefined mithril", "silvery metal", 
							0,  0,   1, 10000,3,3, 3, 0, 0, 10, MITHRIL, HI_SILVER),/*Needs tile*/
ROCK("chunk of fossil dark", "black",
							0,  0,  25, 	500, 8, 8, 4, 1, 0, 1, MINERAL, CLR_BLACK),/*Needs tile*/
ROCK("silver slingstone", "silver", 
							0,  0,   1, 10, 6, 6, 2, 0,  0, 5, SILVER, HI_SILVER),/*Needs tile*/
ROCK("rock", (char *)0,		1,100,   1,  0, 6, 6, 0, 0, 10, 7, MINERAL, CLR_GRAY),
#undef GEM
#undef ROCK

/* miscellaneous ... */
/* Note: boulders and rocks are not normally created at random; the
 * probabilities only come into effect when you try to polymorph them.
 * Boulders weigh more than MAX_CARR_CAP; statues use corpsenm to take
 * on a specific type and may act as containers (both affect weight).
 */
								//BITS(nmkn,mrg,uskn,ctnr,mgc,chrg,uniq,nwsh,big,tuf,dir,sub,mtrl,shwmat)
OBJECT(OBJ("boulder",(char *)0), BITS(1,0,0,0,0,0,0,0,1,0,0,P_NONE,MINERAL,0), 0,
		ROCK_CLASS,   100, 0, 6000,  0, 20, 20, 0, 0, 0, 2000, HI_MINERAL),
OBJECT(OBJ("statue", (char *)0), BITS(1,0,0,1,0,0,0,0,0,0,0,P_NONE,MINERAL,0), 0,
		ROCK_CLASS,   800, 0, 2500,  0, 20, 20, 0, 0, 0, 2500, CLR_WHITE),
OBJECT(OBJ("fossil", (char *)0), BITS(1,0,0,0,0,0,0,0,0,0,0,P_NONE,MINERAL,0), 0,
		ROCK_CLASS,   100, 0, 2500,  0, 20, 20, 0, 0, 0, 2500, CLR_BROWN),
/*
OBJECT(OBJ("bed",(char *)0), BITS(1,0,0,0,0,0,0,0,1,0,0,P_NONE,WOOD,0), 0,
		BED_CLASS,   900, 0, 2000,  1000, 20, 20, 0, 0, 0, 2000, HI_WOOD),
OBJECT(OBJ("knapsack",(char *)0), BITS(1,0,0,0,0,0,0,0,1,0,0,P_NONE,PLASTIC,0), 0,
		BED_CLASS,   100, 0, 60,  100, 20, 20, 0, 0, 0, 2000, CLR_GREEN),
OBJECT(OBJ("gurney",(char *)0), BITS(1,0,0,0,0,0,0,0,1,0,0,P_NONE,METAL,0), 0,
		BED_CLASS,     0, 0, 60,  100, 20, 20, 0, 0, 0, 2000, CLR_WHITE),
*/
#ifdef CONVICT
OBJECT(OBJ("heavy iron ball", (char *)0), BITS(1,0,0,0,0,0,0,0,0,0,WHACK,P_FLAIL,IRON,0), 0,
#else
OBJECT(OBJ("heavy iron ball", (char *)0), BITS(1,0,0,0,0,0,0,0,0,0,WHACK,P_NONE,IRON,0), 0,
#endif /* CONVICT */
		BALL_CLASS,  1000, 0,  480, 10, 25, 25, 0, 0,  0, 200, HI_METAL),
						/* +d4 when "very heavy" */
OBJECT(OBJ("chain", (char *)0), BITS(1,0,0,0,0,0,0,0,0,0,WHACK,P_FLAIL,IRON,1), 0,
		CHAIN_CLASS, 1000, 0,  120,  0,  4,  4, 4, 0,  0, 200, HI_METAL),
OBJECT(OBJ("sheaf of hay", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,0,P_NONE,VEGGY,0), 0, /*Needs encyc entry*//*Needs tile*/
		CHAIN_CLASS, 	0, 2, 	20,  0,  0,  0, 0, 0,  0, 100, CLR_YELLOW),
OBJECT(OBJ("clockwork component", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,WHACK,P_NONE,COPPER,0), 0, /*Needs encyc entry*//*Needs tile*/
		CHAIN_CLASS,    0, 0,    1,  0,  1,  1, 0, 0,   0, 20, HI_COPPER),
OBJECT(OBJ("subethaic component", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,WHACK,P_NONE,GLASS,0), 0, /*Needs encyc entry*//*Needs tile*/
		CHAIN_CLASS,    0, 0,    1,  0,  1,  1, 0, 0,   0, 20, HI_GLASS),
OBJECT(OBJ("scrap", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,WHACK,P_NONE,IRON,0), 0, /*Needs encyc entry*//*Needs tile*/
		CHAIN_CLASS,    0, 0,    1,  0,  1,  1, 0, 0,   0, 20, CLR_BROWN),
OBJECT(OBJ("hellfire component", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,WHACK,P_NONE,METAL,0), 0, /*Needs encyc entry*//*Needs tile*/
		CHAIN_CLASS,    0, 0,    1,  0,  1,  1, 0, 0,   0, 20, CLR_ORANGE),
						/* +1 both l & s */
OBJECT(OBJ("broken android", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,WHACK,P_NONE,METAL,0), 0, /*Needs encyc entry*//*Needs tile*/
		CHAIN_CLASS,    0, 0, 3000,  0,  1,  1, 0, 0,   0, 20, CLR_WHITE),
OBJECT(OBJ("broken gynoid", (char *)0), BITS(1,1,0,0,0,0,0,0,0,0,WHACK,P_NONE,METAL,0), 0, /*Needs encyc entry*//*Needs tile*/
		CHAIN_CLASS,    0, 0, 3000,  0,  1,  1, 0, 0,   0, 20, CLR_WHITE),

OBJECT(OBJ("rope of entangling", (char *)0), BITS(1,0,0,0,0,0,0,0,0,0,WHACK,P_NONE,VEGGY,0), 0,
		CHAIN_CLASS,	0, 0,  120,  0,  4,  4, 0, 0,  0, 200, CLR_BROWN),
OBJECT(OBJ("iron bands", (char *)0), BITS(1,0,0,0,0,0,0,0,0,0,WHACK,P_NONE,IRON,0), 0,
		CHAIN_CLASS,	0, 0,  120,  0,  4,  4, 0, 0,  0, 200, HI_METAL),
OBJECT(OBJ("razor wire", (char *)0), BITS(1,0,0,0,0,0,0,0,0,0,SLASH,P_NONE,IRON,0), 0,
		CHAIN_CLASS,	0, 0,  120,  0,  4,  4, 0, 0,  0, 200, HI_METAL),

OBJECT(OBJ("blinding venom", "splash of venom"),
		BITS(0,1,0,0,0,0,0,1,0,0,0,P_NONE,LIQUID,0), 0,
		VENOM_CLASS,  500, 0,	 1,  0,  0,  0, 0, 0,	 0, 0, CLR_BLACK),
OBJECT(OBJ("acid venom", "splash of venom"),
		BITS(0,1,0,0,0,0,0,1,0,0,0,P_NONE,LIQUID,0), 0,
		VENOM_CLASS,  500, 0,	 1,  0,  6,  6, 0, 0,	 0, 0, CLR_BRIGHT_GREEN),
OBJECT(OBJ("ball of webbing", (char *)0),/*Needs tile*/
		BITS(1,1,0,0,0,0,0,1,0,0,0,P_NONE,LIQUID,0), 0,
		VENOM_CLASS,  0, 0,	 1,  0,  6,  6, 0, 0,	 0, 0, CLR_WHITE),
//OBJECT(OBJ("shoggoth venom", "splash of venom"),
//		BITS(0,1,0,0,0,0,0,1,0,0,0,P_NONE,LIQUID,0), 0,
//		VENOM_CLASS,  500, 0,	 1,  0,  0,  0, 0, 0,	 0, 0, HI_ORGANIC),
		/* +d6 small or large */

/* fencepost, the deadly Array Terminator -- name [1st arg] *must* be NULL */
	OBJECT(OBJ((char *)0,(char *)0), BITS(0,0,0,0,0,0,0,0,0,0,0,P_NONE,0,0), 0,
		ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
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
