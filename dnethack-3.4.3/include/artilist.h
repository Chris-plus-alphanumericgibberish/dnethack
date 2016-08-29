/*	SCCS Id: @(#)artilist.h 3.4	2003/02/12	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef MAKEDEFS_C
/* in makedefs.c, all we care about is the list of names */

#define A(nam,typ,s1,s2,mt, mfm, mft, mfb, mfg, mfr, mfv,atk,dfn,cry,inv,al,cl,rac,cost, s12, s22, ws) nam

static const char *artifact_names[] = {
#else
/* in artifact.c, set up the actual artifact list structure */

#define A(nam,typ,s1,s2,mt, mfm, mft, mfb, mfg, mfr, mfv,atk,dfn,cry,inv,al,cl,rac,cost, s12, s22, ws) \
 { typ, nam, s1, s2, mt, mfm, mft, mfb, mfg, mfr, mfv, atk, dfn, cry, inv, al, cl, rac, cost, s12, s22, ws }

#define     NO_ATTK	{0,0,0,0}		/* no attack */
#define     NO_DFNS	{0,0,0,0}		/* no defense */
#define     NO_CARY	{0,0,0,0}		/* no carry effects */
#define     DFNS(c)	{0,c,0,0}		/* DFNS(AD_MAGM) == grants magic resistance when wielded */
#define     CARY(c)	{0,c,0,0}		/* CARY(AD_MAGM) == grants magic resistance when in open inventory */
#define     PHYS(a,b)	{0,AD_PHYS,a,b}		/* physical */
#define     DRLI(a,b)	{0,AD_DRLI,a,b}		/* life drain */
#define     COLD(a,b)	{0,AD_COLD,a,b}
#define     FIRE(a,b)	{0,AD_FIRE,a,b}
#define     ELEC(a,b)	{0,AD_ELEC,a,b}		/* electrical shock */
#define     ACID(a,b)	{0,AD_ACID,a,b}
#define     STUN(a,b)	{0,AD_STUN,a,b}		/* magical attack */
#define     DRST(a,b)	{0,AD_DRST,a,b}		

STATIC_OVL NEARDATA struct artifact artilist[] = {
#endif	/* MAKEDEFS_C */

/* Artifact cost rationale:
 * 1.  The more useful the artifact, the better its cost.
 * 2.  Quest artifacts are highly valued.
 * 3.  Chaotic artifacts are inflated due to scarcity (and balance).
 */


/*  dummy element #0, so that all interesting indices are non-zero */
A("",				STRANGE_OBJECT,
	0, 0, 0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, 
	NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 0L, 0,0 ,0),

//////////////////////Crowning Gifts///////////////////////////////////////
/*Take Me Up/Cast Me Away*/
A("Excalibur",			LONG_SWORD,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_SEEK|SPFX_DEFN|SPFX_INTEL|SPFX_SEARCH),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(20,10),	DRLI(0,0),	NO_CARY,	//Excalibur is a very accurate weapon, a property that almost doesn't matter except for vs high level demons
	0, A_LAWFUL, PM_KNIGHT, NON_PM, 4000L, //Excalibur does extra blessed damage to demons and undead, +3d7 instead of 1d4
	0,0,0),
/* Clarent patch (Greyknight): Note that Clarent's SPFX2_DIG gives it another +2 to hit against thick-skinned
	monsters, as well as against wall-passers. Another special effect of Clarent
	is that it can be pulled out of a wall it is stuck in (by #untrapping towards
	it) if you are devoutly lawful. */
	/*Clarent has been modified to make it the Knight crowning-gift*/
A("Clarent",			LONG_SWORD, /*quote (sorta)*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_CON_OR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, MB_THICK_HIDE /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,20),	NO_DFNS,		NO_CARY,
	LEADERSHIP,	A_LAWFUL, PM_KNIGHT, NON_PM, 4000L, 
	SPFX2_DIG,0,0), 

/*
 *	Stormbringer only has a 2 because it can drain a level,
 *	providing 8 more.
 */

A("Stormbringer",		RUNESWORD,
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN|SPFX_INTEL|SPFX_DRLI), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	DRLI(5,2),	DRLI(0,0),	NO_CARY,	
	0, A_CHAOTIC, NON_PM, NON_PM, 8000L, 
	SPFX2_BLDTHRST,0,0),
A("Reaver",			SCIMITAR, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,8),	NO_DFNS,	NO_CARY,
	THEFT_TYPE, A_CHAOTIC, PM_PIRATE, NON_PM, 6000L,
	SPFX2_STEAL, 0, 0 ),

/*
 *	Two problems:  1) doesn't let trolls regenerate heads,
 *	2) doesn't give unusual message for 2-headed monsters (but
 *	allowing those at all causes more problems than worth the effort).
 */
A("Vorpal Blade",		LONG_SWORD,
	(SPFX_RESTR|SPFX_BEHEAD), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,1),	NO_DFNS,	NO_CARY, /*Special code in weapon.c throws an extra die, so 2d8+2 vs small, 2d12+2 vs large*/	
	0, A_NEUTRAL, NON_PM, NON_PM, 4000L, /*Vorpal Blade also uses exploading dice (roll again if maximum number is rolled)*/
	0,0,0),							 /*According to an article on 1d4Chan, the average of an exploading die is roughly that of a die one size larger*/
									 /*So vorpal sword is effectively 2d10+2/2d14+2*/
A("The Bow of Skadi",	BOW, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	COLD(1,24),	NO_DFNS,	CARY(AD_COLD),
	0, A_LAWFUL, PM_VALKYRIE, NON_PM, 4000L, /*Read to learn Cone of Cold (Skadi's Galdr) */
	SPFX2_COLD2,0,0),
A("The Crown of the Saint King",	HELMET, /*Actually gold circlet*/ /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	LEADERSHIP, A_LAWFUL, PM_NOBLEMAN, NON_PM, 4000L, /*Also causes pets to always follow you when worn*/
	0,0,WSFX_PLUSSEV),
A("The Helm of the Dark Lord",	HELMET, /*Actually visored helmet*/ /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	LEADERSHIP, A_CHAOTIC, PM_NOBLEMAN, NON_PM, 4000L, /*Also causes pets to always follow you when worn*/
	0,0,WSFX_PLUSSEV),
A("Sunbeam",	GOLDEN_ARROW, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,0),	NO_DFNS,	CARY(AD_DRLI),
	0, A_LAWFUL, PM_RANGER, NON_PM, 1000L,
	SPFX2_POISONED|SPFX2_BRIGHT,0,0),
A("Moonbeam",	SILVER_ARROW, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,0),	NO_DFNS,	CARY(AD_DRLI),
	0, A_CHAOTIC, PM_RANGER, NON_PM, 1000L,
	SPFX2_BRIGHT,0,0),
A("Veil of Latona",	CLOAK_OF_INVISIBILITY, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN|SPFX_REFLECT), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	DFNS(AD_MAGM),	CARY(AD_DRLI),
	0, A_NEUTRAL, PM_RANGER, NON_PM, 1000L,
	0,0,0),


A("The Marauder's Map", SCR_MAGIC_MAPPING, /*Needs encyc entry*/
	(SPFX_RESTR), 0, 
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	OBJECT_DET,	A_CHAOTIC, PM_PIRATE, NON_PM, 2000L,
	0,0,0),


/*//////////Namable Artifacts//////////*/
/*most namables can be twoweaponed, look in obj.h */

/*
 *	Orcrist and Sting have same alignment as elves.
 */
A("Orcrist",			ELVEN_BROADSWORD,
	(SPFX_CON_OR), SPFX_WARN, /* the old elves fought balrogs too. */
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_ORC|MA_DEMON) /*MA*/, 0 /*MV*/,
	PHYS(10,0),	NO_DFNS,	NO_CARY,	
	0, A_CHAOTIC, NON_PM, PM_ELF, 2000L, 
	0,0,0),

/*
 *	The combination of SPFX_WARN and M2_something on an artifact
 *	will trigger EWarn_of_mon for all monsters that have the appropriate
 *	M2_something flags.  In Sting's case it will trigger EWarn_of_mon
 *	for MA_ORC monsters.
 */
A("Sting",			ELVEN_DAGGER,
	(SPFX_CON_OR), SPFX_WARN,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_ORC|MA_ARACHNID /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	NO_CARY,	
	0, A_CHAOTIC, NON_PM, PM_ELF, 800L, 
	0,0,0),

A("Grimtooth",			ORCISH_DAGGER, /*Needs encyc entry*/
    (SPFX_CON_OR), SPFX_WARN,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_ELF|MA_HUMAN|MA_DWARF|MA_MINION) /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	NO_CARY,
	0, A_CHAOTIC, NON_PM, PM_ORC, 300L, 
	0,0,0),

A("Carnwennan",			DAGGER, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_NOGEN|SPFX_CON_OR), SPFX_WARN,
	0 /*Monster Symbol*/, 0 /*MM*/, MT_MAGIC /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_FEY /*MA*/, 0 /*MV*/,
	PHYS(5,10),	NO_DFNS,		NO_CARY,
	INVIS,	A_LAWFUL, PM_KNIGHT, NON_PM, 4000L, 
	SPFX2_STLTH,0,0), 

A("Slave to Armok",			DWARVISH_MATTOCK, /*two handed, so no twoweaponing.*/
	(SPFX_CON_OR), 0, /*DF Dwarves can be a nasty lot.*/
	0 /*Monster Symbol*/, 0 /*MM*/, MT_PEACEFUL /*MT*/, 0 /*MB*/, MG_LORD /*MG*/, (MA_ELF|MA_ORC) /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	NO_CARY,
	0, A_LAWFUL, NON_PM, PM_DWARF, 2500L, 
	SPFX2_BLDTHRST,0,0), /*attacks neutrals and pets*/

A("Claideamh",			LONG_SWORD, /* "Sword" */
	(SPFX_NOGEN|SPFX_CON_OR|SPFX_WARN), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_ELF|MA_FEY|MA_GIANT|MA_ELEMENTAL|MA_PRIMORDIAL) /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	NO_CARY,
	0, A_NONE, NON_PM, NON_PM, 5000L, 
	0,0,0),

/*//////////The Banes//////////*/
/*banes can be twoweaponed, look in obj.h*/

A("Dragonlance",			LANCE,
	(SPFX_RESTR|SPFX_CON_OR|SPFX_REFLECT), SPFX_WARN, /* also makes a handy weapon for knights, since it can't break */
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_DRAGON|MA_REPTILIAN /*MA*/, 0 /*MV*/,
	PHYS(10,20),	NO_DFNS,	NO_CARY,				/* plus, reflection */
	0, A_NONE, NON_PM, NON_PM, 5000L, 
	0,0,0),

A("Nodensfork",			TRIDENT,
	(SPFX_RESTR|SPFX_CON_OR), SPFX_WARN,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_PRIMORDIAL|MA_ET /*MA*/, MV_TELEPATHIC|MV_RLYEHIAN /*MV*/,
	PHYS(10,20),	NO_DFNS,	ELEC(0,0),				/* plus, reflection */
	0, A_NONE, NON_PM, NON_PM, 5000L, 
	SPFX2_SILVERED,0,0),

A("Gaia's Fate",			SICKLE,
	(SPFX_RESTR|SPFX_CON_OR), SPFX_WARN,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 
	MA_PLANT|MA_INSECTOID|MA_ARACHNID|MA_AVIAN|MA_REPTILIAN|MA_ANIMAL|MA_FEY|MA_ELF|MA_ELEMENTAL /*MA*/, 
	0 /*MV*/,
	PHYS(10,20),	NO_DFNS,	NO_CARY,				/* plus, reflection */
	0, A_NONE, NON_PM, NON_PM, 5000L, 
	SPFX2_SILVERED,0,0),

A("Demonbane",			SILVER_SABER,
	(SPFX_RESTR|SPFX_CON_OR), SPFX_WARN, /* blocks summoning and is a silver saber. */
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_DEMON /*MA*/, 0 /*MV*/,
	PHYS(10,20),	NO_DFNS,	NO_CARY,			/* Plus, demons are nasty. */
	0, A_LAWFUL, NON_PM, NON_PM, 2500L, 
	SPFX2_NOCALL,0,0),

A("Werebane",			SILVER_SABER, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_CON_OR), SPFX_WARN, /* protects against lycathropy and is silver */
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_WERE|MA_DEMIHUMAN /*MA*/, 0 /*MV*/,
	PHYS(10,20),	NO_DFNS,	NO_CARY,			/*works against many demihumans, including */
	0, A_NONE, NON_PM, NON_PM, 1500L,					/*a few late game enemies */
	(SPFX2_NOWERE),0,0),

A("Giantslayer",		AXE,
	(SPFX_RESTR|SPFX_CON_OR), SPFX_WARN, /* deducts move from hit giants. */
	0 /*Monster Symbol*/, 0 /*MM*/, MT_ROCKTHROW /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_GIANT /*MA*/, 0 /*MV*/,
	PHYS(10,20),	NO_DFNS,	NO_CARY, /* also works vs a few late game enemies, and the bonus damage aplies to all large monsters. */
	0, A_NONE, NON_PM, NON_PM, 2000L, 	 /* also gets bonus damage, +1d4/+2d4, for d6+d4/3d4 total. */
	0,0,0),

A("Vampire Killer",			BULLWHIP,
	(SPFX_RESTR|SPFX_CON_OR), 0, /* some standard castlevainia enemy types*/
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_UNDEAD|MA_DEMON|MA_WERE) /*MA*/, 0 /*MV*/,
	PHYS(10,20),	DRLI(0,0),	NO_CARY,	/*is given extra damage in weapon.c, since whip damage is so low*/
	BLESS, A_LAWFUL, NON_PM, NON_PM, 2500L, /*Bless: no timeout, bless weapon, set fixed, repair erosion,*/
	SPFX2_NOWERE,0,0),									/* and raise enchantment to +3. */

A("Kingslayer",		STILETTO,
	(SPFX_RESTR|SPFX_CON_OR), SPFX_WARN, /* works against just about all the late game badies */
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, (MG_LORD|MG_PRINCE) /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,20),	NO_DFNS,	NO_CARY,	
	0, A_CHAOTIC, NON_PM, NON_PM, 2500L, 
	SPFX2_POISONED,0,0),

A("Peace Keeper",		ATHAME, 
	(SPFX_RESTR|SPFX_CON_OR), SPFX_WARN, /* speaks for itself */
	0 /*Monster Symbol*/, 0 /*MM*/, MT_HOSTILE /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,20),	NO_DFNS,	NO_CARY,
	0, A_LAWFUL, NON_PM, NON_PM, 2500L, 
	SPFX2_SILVERED,0,0),

A("Ogresmasher",		WAR_HAMMER, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_CON_OR|SPFX_BEHEAD), 0, //BEHEAD code smashes ogres
	S_OGRE /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,20),	NO_DFNS,	NO_CARY,			//set str and con to 25, and smashing ogres excercises str and wis
	0, A_NONE, NON_PM, NON_PM, 2000L,
	0,0,0),

A("Trollsbane",			MORNING_STAR, //code early in attack petrifies trolls
	(SPFX_RESTR|SPFX_CON_OR), 0, //also gives bonus damage against monsters who pop in to ruin your day.
	S_TROLL /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, MG_REGEN /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,20),	NO_DFNS,	NO_CARY, /*Needs encyc entry*/
	0, A_NONE, NON_PM, NON_PM, 2000L, 
	SPFX2_SILVERED|SPFX2_BRIGHT,0,0),//Silvered

/*//////////First Gifts//////////*/
//first gifts can be twoweaponed by their associated classes, look in obj.h

/*
 *	Mjollnir will return to the hand of the wielder when thrown
 *	if the wielder is a Valkyrie wearing Gauntlets of Power.
 *	
 *	Now it will never strike the Valkyrie or fall to the ground,
 *	if both are in good condition.
 */
A("Mjollnir",			WAR_HAMMER,		/* Mjo:llnir */
	(SPFX_RESTR|SPFX_ATTK),  0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	ELEC(5,24),	NO_DFNS,	NO_CARY,	
	0, A_NEUTRAL, PM_VALKYRIE, NON_PM, 4000L, 
	SPFX2_ELEC,0,0),

A("the Pen of the Void",	ATHAME,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	NO_CARY,
	VOID_CHIME,	A_VOID, PM_EXILE, NON_PM, 2500L, 
	0,SPFX3_NOCNT,0),

#ifdef CONVICT
A("Luck Blade",			SHORT_SWORD, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_INTEL|SPFX_LUCK), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(7, 7),	NO_DFNS,	NO_CARY,	0, A_CHAOTIC, PM_CONVICT, NON_PM, 3000L,
	0,0,0 ),
#endif /* CONVICT */

A("Cleaver",			BATTLE_AXE,
	SPFX_RESTR, 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(3,0),	NO_DFNS,	NO_CARY,	
	0, A_NEUTRAL, PM_BARBARIAN, NON_PM, 1500L, 
	SPFX2_SHATTER,0,0),

/*	Need a way to convert era times to Japanese luni-solar months.*/
A("Kiku-ichimonji",		KATANA,
	SPFX_RESTR, 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(4,12),	NO_DFNS,	NO_CARY,
	0, A_LAWFUL, PM_SAMURAI, NON_PM, 1200L,
	0,0,0), 

A("Rhongomyniad",			LANCE, /*Needs encyc entry*/
	(SPFX_RESTR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(3,0),	NO_DFNS,		NO_CARY,
	0,	A_LAWFUL, PM_KNIGHT, NON_PM, 4000L, 
	0,0,0), 

A("The Rod of Lordly Might", MACE, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_DEFN), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(3,0),	NO_DFNS,	NO_CARY,
	LORDLY,	A_LAWFUL, PM_NOBLEMAN, NON_PM, 4000L, 
	0,SPFX3_ENGRV,0), 

/*
 *	Magicbane is a bit different!  Its magic fanfare
 *	unbalances victims in addition to doing some damage.
 */
A("Magicbane",			ATHAME, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	STUN(3,4),	DFNS(AD_MAGM),	NO_CARY,	
	0, A_NEUTRAL, PM_WIZARD, NON_PM, 3500L, 
	SPFX2_SILVERED,0,0),


/*//////////Double Damage Artifacts//////////*/

A("Grayswandir",		SILVER_SABER,
	(SPFX_RESTR|SPFX_HALRES|SPFX_WARN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1, 0),	NO_DFNS,	NO_CARY,	
	0, A_LAWFUL, NON_PM, NON_PM, 8000L, 
	0,0,0),

A("Frost Brand",		LONG_SWORD,
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	COLD(1,0),	COLD(0,0),	NO_CARY,	
	0, A_NONE, NON_PM, NON_PM, 3000L, 
	0,0,0),

A("Fire Brand",			LONG_SWORD,
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	FIRE(1,0),	FIRE(0,0),	NO_CARY,	
	0, A_NONE, NON_PM, NON_PM, 3000L, 
	0,0,0),

A("Gilded Sword of Y'ha-Talla",			SCIMITAR,
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	DRST(5,0),	DRST(0,0),	NO_CARY,
	LORDLY, A_NONE, NON_PM, NON_PM, 3000L, 
	SPFX2_POISONED,0,0),

A("Mirror Brand",	LONG_SWORD,	
	(SPFX_ATTK|SPFX_RESTR|SPFX_DALIGN|SPFX_REFLECT), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	STUN(1,0),	NO_DFNS,	NO_CARY,
	0,	A_NEUTRAL, NON_PM, NON_PM, 3000L, 
	SPFX2_SILVERED,0,0),

A("Sunsword",			LONG_SWORD,	
	(SPFX_RESTR|SPFX_CON_OR|SPFX_SEARCH), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_UNDEAD|MA_DEMON) /*MA*/, 0 /*MV*/,
	PHYS(1,0),	DFNS(AD_BLND),	NO_CARY,	/*also petrifies trolls, making this weapon strictly better than*/
	0, A_LAWFUL, NON_PM, NON_PM, 1500L, 	/*trollsbane.  But trollsbane can be twoweaponed.*/
	SPFX2_SHINING|SPFX2_SILVERED|SPFX2_BLIND|SPFX2_BRIGHT,0,0), 

A("The Axe of the Dwarvish Lords", BATTLE_AXE, /*can be thrown by dwarves*/
	(SPFX_RESTR|SPFX_TCTRL|SPFX_XRAY),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,	/*x-ray vision is for dwarves only.*/
	PHYS(1, 0),	NO_DFNS,	NO_CARY, 
	0, A_LAWFUL, NON_PM, PM_DWARF, 4000L, /*Needs encyc entry*/
	SPFX2_DIG,0,0),

A("Windrider",	BOOMERANG, /*returns to your hand.*/
	SPFX_RESTR, 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,0),	NO_DFNS,	NO_CARY, /*Needs encyc entry*/
	0,	A_NONE, NON_PM, NON_PM, 4000L, 
	0,0,0),

A("The Rod of the Ram",			MACE, /* Wolf, Ram, and Hart? Ram demon? */
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,0),	NO_DFNS,	NO_CARY, /*Needs encyc entry*/
	0, A_NEUTRAL, NON_PM, NON_PM, 3000L, 
	SPFX2_RAM2,0,0),

A("Atma Weapon", 		BEAMSWORD, /*Sword whose attack power is bound to its wielder's life force*/
	(SPFX_RESTR|SPFX_CON_OR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, MG_NASTY /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(6,6),	NO_DFNS,	NO_CARY,
	0, A_NONE, NON_PM, NON_PM, 6660L, 
	0,0,0),

A("Limited Moon", 		MOON_AXE, /*Axe whose attack power is bound to its wielder's magical energy*/
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,0),	NO_DFNS,	NO_CARY, /*Needs encyc entry*/
	0, A_CHAOTIC, NON_PM, NON_PM, 6660L, 
	0,0,0),

A("The Black Arrow",		ANCIENT_ARROW, /*Needs encyc entry*/
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,0),	NO_DFNS,	NO_CARY,
	0, A_NONE, NON_PM, NON_PM, 4444L,
	0,0,0), 

A("Tensa Zangetsu",		TSURUGI,
	(SPFX_RESTR|SPFX_HSPDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*also has a haste effect when wielded, but massively increases hunger and damages the wielder*/
	PHYS(1,0),	NO_DFNS,	NO_CARY,
	SPEED_BANKAI, A_NEUTRAL, NON_PM, NON_PM, 4444L,/*The invoked attack is very powerful*/
	0,0,0), 

/*//////////Other Artifacts//////////*/

A("Sode no Shirayuki",		KATANA,
	(SPFX_RESTR|SPFX_ATTK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	COLD(1,0),	COLD(0,0),	NO_CARY,  /*Sort of intermediate between a double damage and a utility weapon,*/
	ICE_SHIKAI, A_LAWFUL, NON_PM, NON_PM, 8000L,/*Sode no Shirayuki gains x2 ice damage after using the third dance.*/
	SPFX2_SILVERED,0,0), /*however, it only keeps it for a few rounds, and the other dances are attack magic. */

A("Tobiume",		LONG_SWORD,
	(SPFX_RESTR|SPFX_ATTK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	FIRE(1,1),	FIRE(0,0),	NO_CARY,/*Tobiume is an awkward weapon.  It loses 3 damage vs large and 2 vs small*/
	FIRE_SHIKAI, A_CHAOTIC, NON_PM, NON_PM, 8000L,
	SPFX2_DISARM|SPFX2_FIRE2|SPFX2_RAM2,0,0),/*Ram and Fire blast only trigger if enemy is low hp*/

//A("Lancea Longini",			SILVER_SPEAR,
A("The Lance of Longinus",			SILVER_SPEAR,
	(SPFX_RESTR|SPFX_HSPDAM|SPFX_HPHDAM|SPFX_DEFN|SPFX_REFLECT),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	DFNS(AD_MAGM),	DRLI(0,0),
	WWALKING, A_LAWFUL, NON_PM, NON_PM, 1500L, 
	0,0,0),

// /* TODO aggrevate 'f' */
// A("The Pink Panther", DIAMOND,
	// (SPFX_NOGEN|SPFX_RESTR), 0, 0,
	// NO_ATTK,	NO_DFNS,	NO_CARY,
	// TRAP_DET, A_NONE, PM_ARCHEOLOGIST, NON_PM, 0L,
	// 0,0,0),

A("The Arkenstone", DIAMOND,
    SPFX_RESTR, SPFX_AGGRM,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
    NO_ATTK, NO_DFNS, NO_CARY,
	CONFLICT, A_CHAOTIC, NON_PM, NON_PM, 8000L, 
	0,SPFX3_LIGHT,0),

A("Release from Care",			SCYTHE, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_DEFN|SPFX_INTEL|SPFX_BEHEAD),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,10),	DRLI(0,0),	COLD(0,0),
	HEALING, A_NONE, NON_PM, NON_PM, 4000L, 
	0,0,0),

A("Lifehunt Scythe",			SCYTHE, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_DEFN|SPFX_BEHEAD),0, //Can only behead creatures that don't know where you are.
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(4,0),	DRLI(0,0),	COLD(0,0), //Actually only applies vs living or undead creatures
	INVIS, A_CHAOTIC, NON_PM, NON_PM, 4000L, 
	SPFX2_STLTH|SPFX2_BLDTHRST,0,0),

A("The Silence Glaive",		GLAIVE, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN|SPFX_DRLI),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	DRLI(1,1),	DRLI(0,0),	NO_CARY,
	SATURN, A_NONE, NON_PM, NON_PM, 8000L, 
	0,0,0),

A("The Garnet Rod",		UNIVERSAL_KEY, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_EREGEN|SPFX_REGEN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*also has a haste effect when wielded, but massively increases hunger*/
	NO_ATTK,	NO_ATTK,	NO_CARY,
	PLUTO, A_NONE, NON_PM, NON_PM, 8000L, 
	0,0,0),

A("Helping Hand",			GRAPPLING_HOOK, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_SEEK|SPFX_SEARCH|SPFX_WARN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	UNTRAP, A_LAWFUL, NON_PM, NON_PM, 2000L, 
	SPFX2_STLTH,SPFX3_ENGRV,0),

A("The Blade Singer's Spear",		SILVER_SPEAR, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(6,6),	NO_DFNS,	NO_CARY,
	DANCE_DAGGER, A_NONE, NON_PM, NON_PM, 1500L, 
	SPFX2_DANCER,0,0),

A("The Blade Dancer's Dagger",		SILVER_DAGGER, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(4,4),	NO_DFNS,	NO_CARY,
	SING_SPEAR, A_NONE, NON_PM, NON_PM, 1500L, 
	SPFX2_DANCER,SPFX3_NOCNT,0),

A("The Limb of the Black Tree",			CLUB, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	FIRE(4,1),	NO_DFNS,	FIRE(0,0),
	0, A_CHAOTIC, NON_PM, NON_PM, 3000L, 
	SPFX2_FIRE2,0,0),

A("Hellfire",			CROSSBOW,/*adapted from Slash'em*/
	(SPFX_RESTR|SPFX_ATTK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	FIRE(4,1),	FIRE(0,0),	NO_CARY,
	0, A_CHAOTIC, NON_PM, NON_PM, 3000L, 
	SPFX2_FIRE2,0,0),

A("The Lash of the Cold Waste",		BULLWHIP,
	(SPFX_RESTR|SPFX_ATTK|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	COLD(4,1),	NO_DFNS,	COLD(0,0),	
	0, A_CHAOTIC, NON_PM, NON_PM, 3000L, 
	SPFX2_COLD2,0,0),

A("Ramiel",			PARTISAN,
	(SPFX_RESTR|SPFX_ATTK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	ELEC(4,1),	NO_DFNS,	NO_CARY,	/*Ramiel's ranged attack is far more useful than the lash and the limb*/
	0, A_LAWFUL, NON_PM, NON_PM, 3000L, /*So it's your job to use it right!*/
	SPFX2_ELEC2,0,0),

A("Spineseeker",	SHORT_SWORD, /*Needs encyc entry*/
	SPFX_RESTR,0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,6),	NO_DFNS,	NO_CARY,
	0, A_CHAOTIC, NON_PM, NON_PM, 1200L,
	SPFX2_STLTH,0,0),

A("Quicksilver",	FLAIL, /*Needs encyc entry*/
	SPFX_RESTR,0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(4,8),	NO_DFNS,	NO_CARY,
	FAST, A_NONE, NON_PM, NON_PM, 1200L,
	SPFX2_SILVERED,0,0),

A("Sky Render",		KATANA, /*Needs encyc entry*/
	SPFX_RESTR, SPFX_DISPL,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,10),	NO_DFNS,	NO_CARY,
	0, A_LAWFUL, NON_PM, NON_PM, 1200L,
	0,0,0),

A("Fuma-itto no Ken",		BROADSWORD, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_DALIGN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,   
	PHYS(1,8),	NO_DFNS,	NO_CARY,
	CREATE_AMMO, A_CHAOTIC, NON_PM, NON_PM, 1200L,/*makes shuriken*/
	SPFX2_POISONED|SPFX2_NINJA,0,0), 
/*  The name is meant to translate to "Sword of the Fuma clan,"
 *  "Ken" being a word for "sword" (as in "Bokken," "wooden sword")
 *  "no" being "of", and "Fuma-itto" being "Fuma-clan" (or so I hope...),
 *  this being a clan of ninja from the Sengoku era
 *  (there should be an accent over the u of "Fuma" and the o of "-itto").
 *  Nethack Samurai call broadswords "Ninja-to," which is the steriotypical ninja sword.
 *  Aparently, there was no such thing as an actual Ninja-to, it's something Hollywood made up!
 */
A("Yoichi no yumi", YUMI, /*Needs encyc entry*/
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(20,0),	NO_DFNS,	NO_CARY,
	CREATE_AMMO, A_LAWFUL, NON_PM, NON_PM, 4000L, 
	0,0,0),

A("Fluorite Octahedron", BLUE_FLUORITE, /*Needs encyc entry*/
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NONE, NON_PM, NON_PM, 4000L, 
	0,0,0),

/*//////////Artifact Books///////////*/
A("The Book of Lost Names", SPE_SECRETS, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	SPIRITNAMES,	A_NONE, NON_PM, NON_PM, 5000L, 
	0,SPFX3_NOCNT,0),

A("The Book of Infinite Spells", SPE_SECRETS, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	INFINITESPELLS,	A_NONE, NON_PM, NON_PM, 5000L, 
	0,SPFX3_NOCNT,0),


/*//////////Artifact Armors//////////*/

#ifdef TOURIST
A("The Tie-Dye Shirt of Shambhala",	T_SHIRT, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	ENLIGHTENING, A_NEUTRAL, NON_PM, NON_PM, 4500L, 
	0,SPFX3_MANDALA,(WSFX_LIGHTEN|WSFX_WCATRIB|WSFX_PLUSSEV)),
#endif
A("The Grandmaster's Robe",	ROBE, /*double robe effect*/
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,/*martial arts attacks use exploding dice and get extra damage*/
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NEUTRAL, NON_PM, NON_PM, 4500L, 
	0,0,(WSFX_PLUSSEV)),

A("Beastmaster's Duster", LEATHER_JACKET, /*Needs encyc entry*/
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	PETMASTER, A_LAWFUL, NON_PM, NON_PM, 9000L, 
	0,0,WSFX_PLUSSEV),

A("Mirrorbright",	SHIELD_OF_REFLECTION,/*adapted from Slash'em*/
	(SPFX_RESTR|SPFX_HALRES),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	CONFLICT, A_CHAOTIC, NON_PM, NON_PM, 4000L, //needs message
	0,0,WSFX_PLUSSEV),

A("The Shield of the All-Seeing", ORCISH_SHIELD, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_SEEK|SPFX_SEARCH|SPFX_WARN), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_ELF /*MA*/, 0 /*MV*/,
     NO_ATTK, DFNS(AD_FIRE), NO_CARY,
	 PROT_FROM_SHAPE_CHANGERS, A_NONE, NON_PM, PM_ORC, 3000L,//needs message
	 0,0,WSFX_PLUSSEV),

A("The Shield of Yggdrasil", ELVEN_SHIELD, /*Needs encyc entry*/
	(SPFX_RESTR|SPFX_REGEN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
     NO_ATTK, DFNS(AD_DRST), NO_CARY,
	 HEALING, A_NONE, NON_PM, PM_ELF, 3000L,
	 0,0,WSFX_PLUSSEV),

A("Whisperfeet", SPEED_BOOTS,/*adapted from Slash'em*/
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	NO_DFNS,	NO_CARY,
	INVIS, A_CHAOTIC, NON_PM, NON_PM, 4000L, 
	SPFX2_STLTH,0,WSFX_PLUSSEV),

A("Water Flowers", WATER_WALKING_BOOTS,
	(SPFX_RESTR|SPFX_DISPL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	NO_DFNS,	NO_CARY,
	TELEPORT_SHOES, A_CHAOTIC, NON_PM, NON_PM, 4000L, //needs message
	SPFX2_SILVERED,0,WSFX_PLUSSEV),

A("Hammerfeet", KICKING_BOOTS,
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	PHYS(1,0),	NO_DFNS,	NO_CARY,
	0, A_CHAOTIC, NON_PM, NON_PM, 4000L, 
	SPFX2_RAM2,0,WSFX_PLUSSEV),

A("The Shield of the Resolute Heart",		GAUNTLETS_OF_DEXTERITY,
	(SPFX_RESTR|SPFX_HPHDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	BLESS, A_NONE, NON_PM, NON_PM, 4000L, 
	0,0,WSFX_PLUSSEV),

A("The Gauntlets of Spell Power",		GAUNTLETS_OF_POWER, /*Note: it is quite deliberate that these cause */
	(SPFX_RESTR|SPFX_HSPDAM),0,							   /*		a spellcasting penalty. */
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY, /*Needs encyc entry*/
	0, A_NONE, NON_PM, NON_PM, 4000L, 
	SPFX2_SILVERED|SPFX2_SPELLUP,0,0),

A("Premium Heart",		GAUNTLETS_OF_POWER,
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,0),	NO_DFNS,	NO_CARY, /*Needs encyc entry*/
	0, A_NONE, NON_PM, NON_PM, 4000L, 
	0,0,WSFX_PLUSSEV),
	
A("Stormhelm",		HELM_OF_BRILLIANCE,
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	COLD(0,0),	ELEC(0,0), /*Needs encyc entry*/
	0, A_CHAOTIC, NON_PM, NON_PM, 3000L, 
	0,0,WSFX_PLUSSEV),

/*	Doesn't Work...
A("Hellrider's Saddle",			SADDLE,
	(SPFX_RESTR), SPFX_REFLECT, 0,
	NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	INVIS, A_NONE, NON_PM, NON_PM, 4000L, 
	0,0),
	*/
/*
 *	The artifacts for the quest dungeon, all self-willed.
 */

/*//////////Law Quest Artifacts//////////*/

A("The Rod of Seven Parts",	SILVER_SPEAR, /*From D&D*/
	(SPFX_CON_OR|SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DALIGN|SPFX_DEFN), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(7,20),	DRLI(0,0),	NO_CARY,
	SEVENFOLD,	A_LAWFUL, NON_PM, NON_PM, 7777L, 
	0,0,0),

A("The Field Marshal's Baton",	MACE,
	(SPFX_NOGEN|SPFX_RESTR), SPFX_WARN,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, MG_MERC /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	CANNONADE,	A_LAWFUL, NON_PM, NON_PM, 5000L, 
	0,SPFX3_NOCNT,0),

/*//////////Chaos Quest Artifacts//////////*/

A("Houchou",                SPOON, /*Needs encyc entry*/
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
        NO_ATTK,      NO_DFNS,        NO_CARY,        0, A_CHAOTIC, NON_PM, NON_PM, 50000L,0,0,0 ),

A("Werebuster",			LONG_SWORD, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_CON_OR), 0,/*should not be gifted or gened randomly*/
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_WERE /*MA*/, 0 /*MV*/,
	PHYS(10,20),	NO_DFNS,	NO_CARY,
	0, A_NONE, NON_PM, NON_PM, 1500L, /*does not protect agains lycathropy*/
	0,SPFX3_NOCNT,0), /*does not count against artifacts generated*/

A("Masamune",			TSURUGI, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,/*should not be gifted or gened randomly*/
	NO_ATTK,	NO_DFNS,	NO_CARY,
	BLESS, A_NONE, NON_PM, NON_PM, 7500L,
	SPFX2_SILVERED,SPFX3_NOCNT,0), /*does not count against artifacts generated*/

A("The Black Crystal", CRYSTAL_BALL, /*from Final Fantasy*/
        (SPFX_NOGEN|SPFX_RESTR|SPFX_DEFN|SPFX_DALIGN|SPFX_WARN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
        PHYS(3,0), NO_DFNS, CARY(AD_MAGM), 
		SHADOW_FLARE, A_CHAOTIC, NON_PM, NON_PM, 100L, 
		0,0,0), /*The crystals are generated together.  The Black Crystal counts, and the others don't.*/

A("The Water Crystal", CRYSTAL_BALL,
        (SPFX_NOGEN|SPFX_RESTR|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
        NO_ATTK, NO_DFNS, COLD(0,0), 
		BLIZAGA, A_NONE, NON_PM, NON_PM, 100L, 
		0,SPFX3_NOCNT,0), /*does not count against artifacts generated*/

A("The Fire Crystal", CRYSTAL_BALL,
        (SPFX_NOGEN|SPFX_RESTR|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
        NO_ATTK, NO_DFNS, FIRE(0,0), 
		FIRAGA, A_NONE, NON_PM, NON_PM, 100L, 
		0,SPFX3_NOCNT,0),

A("The Earth Crystal", CRYSTAL_BALL,
        (SPFX_NOGEN|SPFX_RESTR|SPFX_DEFN), SPFX_HPHDAM,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
        NO_ATTK, NO_DFNS, NO_CARY, 
		QUAKE, A_NONE, NON_PM, NON_PM, 100L, 
		0,SPFX3_NOCNT,0),

A("The Air Crystal", CRYSTAL_BALL,
        (SPFX_NOGEN|SPFX_RESTR|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
        NO_ATTK, NO_DFNS, ELEC(0,0), 
		THUNDAGA, A_NONE, NON_PM, NON_PM, 100L, 
		0,SPFX3_NOCNT,0),

A("Nighthorn",	UNICORN_HORN, /*from SLASH'EM, although modified from its original form*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK|SPFX_INTEL|SPFX_LUCK), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	FIRE(12,24),	FIRE(0,0),	NO_CARY,
	LEVITATION,	A_CHAOTIC, NON_PM, NON_PM, 5000L, 
	SPFX2_FIRE2,SPFX3_FEAR|SPFX3_NOCNT,0), /*Fire explosion and acts as a scroll of scare monster against non-chaotic monsters.*/


/*/Artifact Keys.  Must be grouped together.  Some code in lock.c, artifact.h, and invent.c depends on the order./*/

A("The First Key of Law", SKELETON_KEY, /*must be first*/
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,		A_LAWFUL, NON_PM, NON_PM, 1500L, 
	0,SPFX3_NOCNT,0),  /*None of the keys count against generated artifacts.*/

A("The Second Key of Law", SKELETON_KEY,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,		A_LAWFUL, NON_PM, NON_PM, 1500L, 
	0,SPFX3_NOCNT,0),

A("The Third Key of Law", SKELETON_KEY,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,		A_LAWFUL, NON_PM, NON_PM, 1500L, 
	0,SPFX3_NOCNT,0),

A("The First Key of Chaos", SKELETON_KEY,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,		A_CHAOTIC, NON_PM, NON_PM, 1500L, 
	0,SPFX3_NOCNT,0),

A("The Second Key of Chaos", SKELETON_KEY,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,		A_CHAOTIC, NON_PM, NON_PM, 1500L, 
	0,SPFX3_NOCNT,0),

A("The Third Key of Chaos", SKELETON_KEY,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,		A_CHAOTIC, NON_PM, NON_PM, 1500L, 
	0,SPFX3_NOCNT,0),

A("The First Key of Neutrality", SKELETON_KEY,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,		A_NEUTRAL, NON_PM, NON_PM, 1500L, 
	0,SPFX3_NOCNT,0),

A("The Second Key of Neutrality", SKELETON_KEY,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,		A_NEUTRAL, NON_PM, NON_PM, 1500L, 
	0,SPFX3_NOCNT,0),

A("The Third Key of Neutrality", SKELETON_KEY, /*must be last*/
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,		A_NEUTRAL, NON_PM, NON_PM, 1500L, 
	0,SPFX3_NOCNT,0),

/*//////////Neutral Quest Artifacts//////////*/

A("The Necronomicon", SPE_SECRETS, /*from the works of HP Lovecraft*/
	(SPFX_NOGEN|SPFX_RESTR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	NECRONOMICON,	A_NONE, NON_PM, NON_PM, 5000L, 
	0,SPFX3_NOCNT,0),

A("The Hand-Mirror of Cthylla", MIRROR, /*from the works of HP Lovecraft*/
	(SPFX_NOGEN|SPFX_RESTR), (SPFX_SEARCH|SPFX_TCTRL),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	0,	A_NONE, NON_PM, NON_PM, 5000L, 
	0,0,0),  /*polymorph control*/

A("The Silver Key", UNIVERSAL_KEY, /*from the works of HP Lovecraft*/
	(SPFX_NOGEN|SPFX_RESTR), (SPFX_EREGEN|SPFX_TCTRL),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	CREATE_PORTAL,	A_NEUTRAL, NON_PM, NON_PM, 5000L, 
	0,SPFX3_PCTRL,0),  /*polymorph control*/



/*//////////Role-specific Quest Artifacts//////////*/
/*
A("The Orb of Detection",	CRYSTAL_BALL,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_ESP|SPFX_HSPDAM), 0,
	NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	INVIS,		A_LAWFUL, PM_ARCHEOLOGIST, NON_PM, 2500L ),
*/
A("Itlachiayaque", SHIELD_OF_REFLECTION,/*From archeologist patch*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN), (SPFX_ESP|SPFX_HSPDAM),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
     NO_ATTK, DFNS(AD_FIRE), CARY(AD_MAGM),
	 SMOKE_CLOUD, A_LAWFUL, PM_ARCHEOLOGIST, NON_PM, 3000L, 
	0,0,0),

A("The Annulus", SILVER_CHAKRAM, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN), (SPFX_HSPDAM),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
     PHYS(5,1), NO_DFNS, CARY(AD_MAGM), /*Actually Phys(5,0) if not a lightsaber*/
	 ANNUL, A_CHAOTIC, PM_ANACHRONONAUT, NON_PM, 3000L, 
	0,0,0),

A("The Heart of Ahriman",	RUBY,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN), (SPFX_REFLECT|SPFX_HSPDAM),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	/* this stone does double damage if used as a projectile weapon */
	PHYS(10,0),	NO_DFNS,	CARY(AD_MAGM),
	ARTI_REMOVE_CURSE,	A_NEUTRAL, PM_BARBARIAN, NON_PM, 2500L, 
	0,0,0),

#ifdef BARD
A("The Lyre of Orpheus",	MAGIC_HARP,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SPEAK|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	TAMING,		A_NEUTRAL, PM_BARD, NON_PM, 5000L,
	0,0,0),
#endif

A("The Sceptre of Might",	MACE,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DALIGN|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	CARY(AD_MAGM),
	CONFLICT,	A_LAWFUL, PM_CAVEMAN, NON_PM, 2500L, 
	SPFX2_RAM,0,0),
#ifdef CONVICT
// A("The Iron Ball of Liberation", HEAVY_IRON_BALL,
	// (SPFX_NOGEN|SPFX_RESTR|SPFX_LUCK|SPFX_INTEL),
		// (SPFX_SEARCH|SPFX_SEEK|SPFX_WARN), 0,
	// NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	// PHASING,	A_CHAOTIC, PM_CONVICT, NON_PM, 5000L,
	// SPFX2_STLTH,0,0), /*Note: it had caried stealth before*/
A("The Iron Ball of Levitation", HEAVY_IRON_BALL, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_DALIGN|SPFX_LUCK|SPFX_INTEL), (SPFX_WARN),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,10),	NO_DFNS,	CARY(AD_DRLI),
	LEVITATION,	A_CHAOTIC, PM_CONVICT, NON_PM, 5000L,
	SPFX2_STLTH,0,0), /*Note: it had caried stealth before*/
A("The Iron Spoon of Liberation", SPOON, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_LUCK|SPFX_INTEL), (SPFX_SEARCH|SPFX_SEEK),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	CARY(AD_MAGM),
	PHASING,	A_CHAOTIC, PM_CONVICT, NON_PM, 5000L,
	SPFX2_STLTH|SPFX2_DIG,SPFX3_ENGRV,0), /*Note: it had caried stealth before*/
#endif	/* CONVICT */

A("Silver Starlight",		RAPIER, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(4,4),	NO_DFNS,	NO_CARY,
	CREATE_AMMO, A_NONE, NON_PM, PM_DROW, 5000L, /*Creates throwing stars. Makes throwing stars count as silver if wielded */
	SPFX2_SILVERED|SPFX2_SHINING,0,0),			/*Also can be (a)pplied as a magic flute.								  */

A("Wrathful Spider",		DROVEN_CROSSBOW, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	CREATE_AMMO, A_CHAOTIC, NON_PM, PM_DROW, 5000L,
	SPFX2_STLTH,0,0),

A("The Tentacle Rod",		FLAIL, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(7,1),	NO_DFNS,	NO_CARY,
	0, A_NONE, NON_PM, PM_DROW, 5000L,
	SPFX2_TENTROD,0,0),

A("The Crescent Blade",		SILVER_SABER, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK|SPFX_BEHEAD), SPFX_REFLECT,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	FIRE(4,0),	NO_DFNS,	NO_CARY,
	0, A_LAWFUL, NON_PM, PM_DROW, 5000L,
	SPFX2_SHINING,0,0),

A("The Darkweaver's Cloak",	DROVEN_CLOAK, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	DFNS(AD_MAGM),	NO_CARY, 
	0,	A_NONE,	 NON_PM, PM_DROW, 5000L,
	0,0,WSFX_PLUSSEV),

A("Spidersilk",	ELVEN_MITHRIL_COAT, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY, 
	0,	A_CHAOTIC,	 NON_PM, PM_DROW, 5000L,
	SPFX2_SPELLUP,0,WSFX_PLUSSEV), /*Adds sleep poison to unarmed attacks*/

A("Webweaver's Crook",	FAUCHARD, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,0),	NO_DFNS,	CARY(AD_MAGM), 
	0,	A_LAWFUL,	 NON_PM, PM_DROW, 5000L,
	SPFX2_POISONED,0,0), /*Adds poison to all attacks*/

A("Lolth's Fang",		DROVEN_SHORT_SWORD, /* Hedrow crowning gift, chaotic or neutral */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK|SPFX_DEFN|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	ACID(10,10),	DRLI(0,0),	NO_CARY,
	0, A_NONE, NON_PM, PM_DROW, 8000L, 
	SPFX2_SILVERED|SPFX2_POISONED,0,0),

A("The Web of Lolth",	ELVEN_MITHRIL_COAT, /* Drow crowning gift, chaotic */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_CON_OR|SPFX_INTEL|SPFX_WARN), 0, /*Needs encyc entry*/
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_ELF /*MA*/, 0 /*MV*/,
	NO_ATTK,	DRLI(0,0),	CARY(AD_MAGM),
	ENERGY_BOOST,	A_CHAOTIC, NON_PM, PM_DROW, 2000L, 
	SPFX2_SILVERED|SPFX2_SPELLUP,0,WSFX_PLUSSEV),

A("The Claws of the Revenancer",		GAUNTLETS_OF_DEXTERITY, /* Drow crowning gift, neutral */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK|SPFX_EREGEN|SPFX_DRLI|SPFX_DEFN|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	DRLI(1,1),	DRLI(0,0),	NO_CARY,
	RAISE_UNDEAD, A_NEUTRAL, NON_PM, PM_DROW, 8000L, 
	SPFX2_SILVERED,0,0),

A("Liecleaver",		DROVEN_CROSSBOW, /* Drow noble crowning gift, lawful */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_SEEK|SPFX_DEFN|SPFX_INTEL|SPFX_SEARCH),SPFX_HALRES,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	PHYS(5,10),	DRLI(0,0),	NO_CARY,	
	CREATE_AMMO, A_LAWFUL, NON_PM, PM_DROW, 8000L, 
	0,0,0),

A("The Ruinous Descent of Stars",		MORNING_STAR, /* Herow noble crowning gift, Chaotic */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_DEFN|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	PHYS(1,0),	NO_DFNS,	CARY(AD_MAGM),	
	FALLING_STARS, A_CHAOTIC, NON_PM, PM_DROW, 8000L, 
	SPFX2_SILVERED,0,0),

/** lawful drow crowning gift */
A("Sickle Moon",	SICKLE, /*returns to your hand.*/
	SPFX_RESTR, 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	PHYS(1,0),	NO_DFNS,	NO_CARY,//needs quote
	0,	A_NONE, NON_PM, NON_PM, 4000L, 
	SPFX2_SILVERED,0,0),

/** Indwelling of the Black Web entity, lawful hedrow crowning gift */

A("Arcor Kerym",		LONG_SWORD, /* Lawful Elf crowning gift */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_DEFN|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	PHYS(5,0),	DRLI(0,0),	NO_CARY,
	HEALING, A_LAWFUL, NON_PM, PM_ELF, 8000L, 
	0,0,0),

A("Aryfaern Kerym",		RUNESWORD, /* Neutral Elf crowning gift */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK|SPFX_DEFN|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	ELEC(5,10),	ELEC(0,0),	NO_CARY,
	0, A_NEUTRAL, NON_PM, PM_ELF, 8000L, 
	SPFX2_SPELLUP,0,0),

A("Aryvelahr Kerym",		CRYSTAL_SWORD, /* Chaotic Elf crowning gift */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_DEFN|SPFX_INTEL|SPFX_REFLECT),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	PHYS(5,0),	DRLI(0,0),	NO_CARY,
	0, A_CHAOTIC, NON_PM, PM_ELF, 8000L, 
	SPFX2_SILVERED,0,0),

A("The Staff of Aesculapius",	QUARTERSTAFF,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK|SPFX_INTEL|SPFX_DRLI|SPFX_REGEN), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	DRLI(1,0),	DRLI(0,0),	NO_CARY,
	HEALING,	A_NEUTRAL, PM_HEALER, NON_PM, 5000L, 
	SPFX2_SILVERED,0,0), /*silver is associated with the healer's art*/

A("The Magic Mirror of Merlin", MIRROR,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SPEAK), SPFX_ESP,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	0,		A_LAWFUL, PM_KNIGHT, NON_PM, 1500L, 
	0,0,0),

A("The Eyes of the Overworld",	LENSES,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_XRAY),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	ENLIGHTENING,	A_NEUTRAL,	 PM_MONK, NON_PM, 2500L, 
	0,0,0),

A("The Mantle of Heaven",	LEATHER_CLOAK, /*Actually an ornamental cope in game*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_HSPDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	ELEC(0,0),	COLD(0,0), /* Plus double AC bonus */
	0,	A_LAWFUL,	 PM_NOBLEMAN, NON_PM, 2500L,
	0,0,0),

A("The Vestment of Hell",	LEATHER_CLOAK, /*Actually an opera cloak in game*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_HPHDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	ACID(0,0),	FIRE(0,0), /* Plus double AC bonus */
	0,	A_CHAOTIC,	 PM_NOBLEMAN, NON_PM, 2500L,
	0,0,0),

A("The Armor of Khazad-dum",	DWARVISH_MITHRIL_COAT, /*Moria dwarf noble first gift */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	DFNS(AD_MAGM),	NO_CARY,
	0,	A_LAWFUL,	 PM_NOBLEMAN, PM_DWARF, 2500L,
	0,0,WSFX_PLUSSEV),

A("The War-mask of Durin",	MASK, /*Moria dwarf noble */ /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_HSPDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*+5 attk and damage with axes*/
	NO_ATTK,	NO_DFNS,	NO_CARY, /* Gives Fire, Acid, and Poison resistance */
	0,	A_LAWFUL,	 PM_NOBLEMAN, PM_DWARF, 2500L,
	0,0,0),

A("Durin's Axe",			AXE, /*Moria dwarf noble crowning */ /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_SEEK|SPFX_DEFN|SPFX_INTEL|SPFX_SEARCH),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,10),	DRLI(0,0),	NO_CARY,	
	0, A_LAWFUL, PM_NOBLEMAN, PM_DWARF, 4000L, 
	SPFX2_DIG|SPFX2_SILVERED,0,0),

A("Glamdring",			ELVEN_BROADSWORD, /*Lonely Mountain dwarf noble first gift */
	(SPFX_NOGEN|SPFX_WARN|SPFX_RESTR), 0, /* the old elves fought balrogs too. */
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_ORC|MA_DEMON) /*MA*/, 0 /*MV*/,
	PHYS(10,10),	NO_DFNS,	NO_CARY, /*Needs encyc entry*/
	0, A_LAWFUL, PM_KNIGHT, NON_PM, 2000L, 
	0,0,0),

A("The Key of Erebor", SKELETON_KEY, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,		A_NONE, NON_PM, NON_PM, 1500L, 
	0,SPFX3_NOCNT,0),

A("The Armor of Erebor",	PLATE_MAIL, /*Lonely Mountain dwarf noble */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_HPHDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,  /*+10 AC*/
	NO_ATTK,	DFNS(AD_MAGM),	NO_CARY, /*Also gives Fire and Cold*/
	0,	A_LAWFUL,	 NON_PM, PM_DWARF, 2500L, /*Needs encyc entry*/
	0,0,0),

/*Arkenstone*/ /*Lonely Mountain dwarf noble crown*/

A("The Sceptre of Lolth", SILVER_KHAKKHARA, /* Drow noble first gift (hedrow get lordly might) */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_DEFN|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	PHYS(1,0),	NO_DFNS,	NO_CARY,
	LORDLY,	A_CHAOTIC, PM_NOBLEMAN, PM_DROW, 4000L, 
	SPFX2_SILVERED,SPFX3_ENGRV,0), 

A("The Web of the Chosen",	DROVEN_CLOAK, /* Drow noble quest */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_HSPDAM|SPFX_REFLECT),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	ACID(0,0),	ELEC(0,0), /* Plus double AC bonus */
	0,	A_CHAOTIC,	 PM_NOBLEMAN, PM_DROW, 2500L,
	SPFX2_SILVERED,0,0),

A("The Cloak of the Consort",	DROVEN_CLOAK, /* Hedrow noble quest */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_HPHDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	DRLI(0,0),	COLD(0,0), /* Plus double AC bonus */
	0,	A_NEUTRAL,	 PM_NOBLEMAN, PM_DROW, 2500L,
	0,0,0),

A("The Mitre of Holiness",	HELM_OF_BRILLIANCE,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_CON_OR|SPFX_INTEL), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, MA_UNDEAD /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	CARY(AD_FIRE),
	ENERGY_BOOST,	A_LAWFUL, PM_PRIEST, NON_PM, 2000L, 
	0,0,0),

A("The Treasury of Proteus",	CHEST,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_LUCK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	0,	A_CHAOTIC,	 PM_PIRATE, NON_PM, 2500L,
	0,0,0 ),

A("The Longbow of Diana", BOW,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_REFLECT), SPFX_ESP,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	NO_CARY,
	CREATE_AMMO, A_CHAOTIC, PM_RANGER, NON_PM, 4000L, 
	SPFX2_SILVERED,0,0), /*silver is the moon's metal... but bows don't enter this code...*/

A("The Rogue Gear-spirits", CROSSBOW,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SPEAK|SPFX_SEARCH|SPFX_SEEK), (SPFX_WARN|SPFX_ESP),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS, CARY(AD_FIRE),
	UNTRAP, A_NEUTRAL, PM_RANGER, PM_GNOME, 4000L, 
	SPFX2_DIG,0,0),

A("Glitterstone", AMBER,
    (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_EREGEN),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
    NO_ATTK, NO_DFNS, NO_CARY,
	CHARGE_OBJ, A_NEUTRAL, NON_PM, PM_GNOME, 8000L, 
	0,SPFX3_LIGHT,0),

A("Great Claws of Urdlen",		GAUNTLETS_OF_POWER,
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,0),	NO_DFNS,	NO_CARY, /*Needs encyc entry*/
	QUAKE, A_CHAOTIC, NON_PM, NON_PM, 4000L, 
	SPFX2_DIG,0,WSFX_PLUSSEV),
	
A("The Moonbow of Sehanine", ELVEN_BOW, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	NO_CARY,
	CREATE_AMMO, A_CHAOTIC, PM_RANGER, PM_ELF, 4000L, 
	SPFX2_SILVERED,0,0), /*silver is the moon's metal... but bows don't enter this code...*/

A("The Spellsword of Corellon", HIGH_ELVEN_WARSWORD, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,10),	NO_DFNS,	NO_CARY,
	0, A_CHAOTIC, NON_PM, PM_ELF, 4000L, 
	0,0,0),

A("The Warhammer of Vandria", WAR_HAMMER, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	NO_CARY,
	0, A_CHAOTIC, NON_PM, PM_ELF, 4000L, 
	0,0,0),

A("The Shield of Saint Cuthbert", SHIELD_OF_REFLECTION, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_HSPDAM|SPFX_HPHDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_LAWFUL, NON_PM, NON_PM, 4000L, 
	0,0,0),

A("The Palantir of Westernesse",	CRYSTAL_BALL, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),
		(SPFX_WARN|SPFX_ESP|SPFX_REFLECT|SPFX_XRAY),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	TAMING,		A_CHAOTIC, NON_PM , PM_ELF, 8000L,
	0,0,0),

A("Belthronding", ELVEN_BOW, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), SPFX_DISPL,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5,0),	NO_DFNS,	NO_CARY,
	CREATE_AMMO, A_CHAOTIC, NON_PM, PM_ELF, 4000L, 
	SPFX2_STLTH,0,0),

A("The Rod of the Elvish Lords", ELVEN_MACE, /* Elf noble first gift */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_DEFN|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	PHYS(3,0),	NO_DFNS,	NO_CARY,
	LORDLY,	A_CHAOTIC, PM_NOBLEMAN, PM_ELF, 4000L, 
	0,SPFX3_ENGRV,0), 

A("The Master Key of Thievery", SKELETON_KEY,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SPEAK),
		(SPFX_WARN|SPFX_TCTRL|SPFX_HPHDAM),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	UNTRAP,		A_CHAOTIC, PM_ROGUE, NON_PM, 3500L, 
	0,0,0),

A("The Tsurugi of Muramasa",	TSURUGI,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_SEEK|SPFX_INTEL|SPFX_BEHEAD|SPFX_LUCK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(2,0),	NO_DFNS,	NO_CARY,
	0,		A_LAWFUL, PM_SAMURAI, NON_PM, 4500L, 
	SPFX2_SHATTER|SPFX2_BLDTHRST,0,0),

#ifdef TOURIST
A("The Platinum Yendorian Express Card", CREDIT_CARD,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN),
		(SPFX_ESP|SPFX_HSPDAM),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	CHARGE_OBJ,	A_NEUTRAL, PM_TOURIST, NON_PM, 7000L, 
	SPFX2_SILVERED,0,0), /*decorative silver rim.  Probably doesn't work...*/
#endif

A("The Orb of Fate",		CRYSTAL_BALL,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_LUCK),
		(SPFX_WARN|SPFX_HSPDAM|SPFX_HPHDAM),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	LEV_TELE,	A_NEUTRAL, PM_VALKYRIE, NON_PM, 3500L, 
	0,0,0),

A("Sol Valtiva",			TWO_HANDED_SWORD,		/* The Sun of the Gods of the Dead */
	(SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	FIRE(5,24),	NO_DFNS,	NO_CARY,	
	0, A_CHAOTIC, NON_PM, PM_FIRE_GIANT, 4000L, 
	SPFX2_FIRE|SPFX2_BLIND|SPFX2_BRIGHT,0,0),

A("The Eye of the Aethiopica",	AMULET_OF_ESP,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_EREGEN|SPFX_HSPDAM),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	CREATE_PORTAL,	A_NEUTRAL, PM_WIZARD, NON_PM, 4000L, 
	0,0,0),


/*//////////Special High-Level Artifacts//////////*/

A("The Hat of the Archmagi",			CORNUTHAUM, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_SPEAK|SPFX_WARN|SPFX_XRAY|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	ENERGY_BOOST, A_NONE, PM_WIZARD, NON_PM, 9000L, 
	0,0,0),
/* 22+ Daimyo can name Kusanagi no Tsurugi from a longsword 
 * (should be a broadsword maybe, but that has been "translated" as a ninja-to).
 * only a level 30 (Shogun) samurai or one who is carying the amulet can wield the sword.
 */
A("The Kusanagi no Tsurugi",	LONG_SWORD, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_EREGEN|SPFX_BEHEAD|SPFX_LUCK), (SPFX_SEARCH|SPFX_SEEK),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(20,12),	NO_DFNS,	NO_CARY,
	0,		A_LAWFUL, PM_SAMURAI, NON_PM, 4500L, 
	0,0,0),

/*
 *	Ah, never shall I forget the cry,
 *		or the shriek that shrieked he,
 *	As I gnashed my teeth, and from my sheath
 *		I drew my Snickersnee!
 *			--Koko, Lord high executioner of Titipu
 *			  (From Sir W.S. Gilbert's "The Mikado")
 */

 A("Snickersnee",		KNIFE,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_BEHEAD),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /* currently nameable by advanced tourists */
	PHYS(3,10),	NO_DFNS,	NO_CARY,						/* Tourists and Samurai can twoweapon Snickersnee */
	0, A_NONE, PM_SAMURAI, NON_PM, 1200L, 
	0,0,0),

  /*/////////////Special Monster Artifacts//////////////*/
 /*//Few of these count, since they are boss treasure//*/
/*////////////////////////////////////////////////////*/

A("The Platinum Dragon Plate",	SILVER_DRAGON_SCALE_MAIL,/*heavier than normal, and causes spell penalties*/
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	DFNS(AD_MAGM),	NO_CARY,
	0, A_NONE, NON_PM, NON_PM, 9000L, 
	0,0,0),

A("The Chromatic Dragon Scales",	BLACK_DRAGON_SCALES,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NONE, NON_PM, NON_PM, 9000L, 
	0,0,0),

A("The Eye of Vecna", EYEBALL,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_HSPDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,        NO_DFNS,        NO_CARY,
	DEATH_GAZE,     A_CHAOTIC, NON_PM, NON_PM, 500L,
	0,0,0),

A("The Hand of Vecna",       SEVERED_HAND,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_HPHDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,        DRLI(0,0),      CARY(AD_COLD),
	SUMMON_UNDEAD,  A_CHAOTIC, NON_PM, NON_PM, 700L,
	0,0,0),

A("Genocide", TWO_HANDED_SWORD, /*Needs encyc entry, somehow*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	FIRE(10,20),	NO_DFNS,	NO_CARY,
	0,	A_LAWFUL, NON_PM, NON_PM, 9999L, 
	SPFX2_FIRE|SPFX2_BLDTHRST,SPFX3_NOCNT,0),

A("The Rod of Dis", MACE, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,8),	NO_DFNS,	NO_CARY,
	TAMING,	A_LAWFUL, NON_PM, NON_PM, 9999L, 
	SPFX2_RAM,SPFX3_NOCNT,0),

A("Avarice", SHORT_SWORD, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(10,1),	NO_DFNS,	NO_CARY,
	THEFT_TYPE,	A_LAWFUL, NON_PM, NON_PM, 9999L, 
	SPFX2_STEAL,SPFX3_NOCNT,0),

A("Fire of Heaven", TRIDENT, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	FIRE(1,0),	NO_DFNS,	NO_CARY,
	0,	A_LAWFUL, NON_PM, NON_PM, 9999L, 
	SPFX2_SILVERED|SPFX2_FIRE2|SPFX2_ELEC,SPFX3_NOCNT,0),

A("The Diadem of Amnesia", DUNCE_CAP, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	CONFLICT,	A_LAWFUL, NON_PM, NON_PM, 9999L, /*was DRAIN_MEMORIES*/ 
	0,SPFX3_NOCNT,0),

A("Shadowlock", RAPIER, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_BEHEAD),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(20,0),	NO_DFNS,	NO_CARY,
	0,	A_LAWFUL, NON_PM, NON_PM, 9999L,
	SPFX2_SHINING,SPFX3_NOCNT,0),

A("Thunder's Voice", SILVER_DAGGER, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	ELEC(6,6),	NO_DFNS,	NO_CARY,
	0,	A_LAWFUL, NON_PM, NON_PM, 3333L, 
	SPFX2_ELEC,SPFX3_NOCNT,0),

A("Serpent's Tooth", ATHAME, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,	A_LAWFUL, NON_PM, NON_PM, 3333L, 
	SPFX2_POISONED,SPFX3_NOCNT,0),

A("Unblemished Soul", UNICORN_HORN, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_LUCK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,	A_LAWFUL, NON_PM, NON_PM, 3333L, 
	SPFX2_SILVERED,SPFX3_NOCNT,0),

A("Wrath of Heaven", LONG_SWORD, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	ELEC(1,0),	NO_DFNS,	NO_CARY,
	0,	A_LAWFUL, NON_PM, NON_PM, 9999L, 
	SPFX2_SILVERED|SPFX2_FIRE|SPFX2_ELEC2,SPFX3_NOCNT,0),

A("The All-seeing Eye of the Fly", HELM_OF_TELEPATHY, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	SHADOW_FLARE,	A_LAWFUL, NON_PM, NON_PM, 9999L, /* Was SLAY_LIVING */
	0,SPFX3_NOCNT,0),

A("Cold Soul", RANSEUR, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,	A_LAWFUL, NON_PM, NON_PM, 9999L, 
	SPFX2_ELEC|SPFX2_COLD|SPFX2_FIRE,SPFX3_NOCNT,0),

A("The Sceptre of the Frozen Floor of Hell", IRON_BAR, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	COLD(1,0),	NO_DFNS,	NO_CARY,
	BLIZAGA,	A_LAWFUL, NON_PM, NON_PM, 9999L, 
	SPFX2_COLD2,SPFX3_NOCNT,0),

A("Caress", BULLWHIP, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	ELEC(1,20),	ELEC(0,0),	NO_CARY,
	0, A_LAWFUL, NON_PM, NON_PM, 9999L, 
	0,SPFX3_NOCNT,0),

A("The Iconoclast", SILVER_SABER,  /*Weapon of Lixer, Prince of Hell, from Dicefreaks the Gates of Hell*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_CON_OR), 0, /*Needs encyc entry*/
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_HUMAN|MA_ELF|MA_DWARF|MA_GNOME) /*MA*/, 0 /*MV*/,
	PHYS(9,99),	DFNS(AD_MAGM),	NO_CARY, /*also does +9 damage to S_ANGELs*/
	0,	A_LAWFUL, NON_PM, NON_PM, 9999L, 
	0,SPFX3_NOCNT,0),

A("The Three-Headed Flail", FLAIL, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,	A_CHAOTIC, NON_PM, NON_PM, 6660L, 
	SPFX2_THREEHEAD,SPFX3_NOCNT,0),

A("Heartcleaver", HALBERD, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,0),	NO_DFNS,	NO_CARY,
	0,	A_CHAOTIC, NON_PM, NON_PM, 6660L, 
	0,SPFX3_NOCNT,0),

A("Wrathful Wind", CLUB, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	COLD(10,0),	NO_DFNS,	NO_CARY,
	0,	A_CHAOTIC, NON_PM, NON_PM, 6660L, 
	SPFX2_SILVERED|SPFX2_COLD,SPFX3_NOCNT,0),

A("The Sting of the Poison Queen", FLAIL, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(4,12),	DFNS(AD_MAGM),	NO_CARY,
	0,	A_CHAOTIC, NON_PM, NON_PM, 6660L, 
	SPFX2_POISONED,SPFX3_NOCNT,0),

A("The Scourge of Lolth", BULLWHIP, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(1,0),	NO_DFNS,	NO_CARY,
	0,	A_CHAOTIC, NON_PM, NON_PM, 6660L, 
	SPFX2_SILVERED,SPFX3_NOCNT,0),

A("Doomscreamer", TWO_HANDED_SWORD,  /*Weapon of Graz'zt, from Gord the Rogue*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK|SPFX_DEFN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, /*Needs encyc entry*/
	ACID(1,0),	ACID(0,0),	NO_CARY,
	0,	A_CHAOTIC, NON_PM, NON_PM, 6660L, 
	0,SPFX3_NOCNT,0),

A("The Wand of Orcus", WAN_DEATH, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_ATTK|SPFX_DRLI),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	DRLI(5,12),	NO_DFNS,	NO_CARY,
	0, A_CHAOTIC, NON_PM, NON_PM, 8000L, 
	SPFX2_BLDTHRST,SPFX3_NOCNT,0),

A("The Sword of Erathaol",			LONG_SWORD,	 /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SEARCH),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(7,10),	DFNS(AD_BLND),	NO_CARY,	/**/
	0, A_LAWFUL, NON_PM, NON_PM, 7777L, 	/**/
	SPFX2_SILVERED|SPFX2_BLIND|SPFX2_BRIGHT,SPFX3_NOCNT,0),

A("The Saber of Sabaoth",			SILVER_SABER, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SEARCH),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(7,10),	DFNS(AD_BLND),	NO_CARY,	/**/
	0, A_LAWFUL, NON_PM, NON_PM, 7777L, 	/**/
	SPFX2_BLIND|SPFX2_BRIGHT,SPFX3_NOCNT,0),

A("The Sword of Onoel",			TWO_HANDED_SWORD, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SEARCH),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(7,10),	DFNS(AD_BLND),	NO_CARY,	/**/
	0, A_LAWFUL, NON_PM, NON_PM, 7777L, 	/**/
	SPFX2_SILVERED|SPFX2_BLIND|SPFX2_BRIGHT,SPFX3_NOCNT,0),

A("The Glaive of Shamsiel",			GLAIVE,	 /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SEARCH),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(7,10),	DFNS(AD_BLND),	NO_CARY,	/**/
	0, A_LAWFUL, NON_PM, NON_PM, 7777L, 	/**/
	SPFX2_SILVERED|SPFX2_BLIND|SPFX2_BRIGHT,SPFX3_NOCNT,0),

A("The Lance of Uriel",			LANCE, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SEARCH),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(7,10),	DFNS(AD_BLND),	NO_CARY,	/**/
	0, A_LAWFUL, NON_PM, NON_PM, 7777L, 	/**/
	SPFX2_SILVERED|SPFX2_BLIND|SPFX2_BRIGHT,SPFX3_NOCNT,0),

A("The Hammer of Barquiel",			LUCERN_HAMMER, /*Needs encyc entry*/
	(SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SEARCH),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(7,10),	DFNS(AD_BLND),	NO_CARY,	/**/
	0, A_LAWFUL, NON_PM, NON_PM, 7777L, 	/**/
	SPFX2_SILVERED|SPFX2_BLIND|SPFX2_BRIGHT,SPFX3_NOCNT,0),

/* mastery artifacts */

/* Archeologist */
A("The Trusty Adventurer's Whip",	BULLWHIP,
	(SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(20,4),	NO_DFNS,	NO_CARY, //Always activates special whip effects
	0, A_NONE, PM_ARCHEOLOGIST, NON_PM, 0L,
	0,0,0),


/* TODO read */
A("The Log of the Curator", SPE_BLANK_PAPER,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	OBJECT_DET, A_NONE, PM_ARCHEOLOGIST, NON_PM, 0L,
	0,0,0),

/* TODO read */
A("The Fedora of the Investigator", FEDORA,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,//+7 ac, +7+ench to Int, Wis, and Cha
	NO_ATTK,	DFNS(AD_MAGM),	NO_CARY,//Detects traps
	OBJECT_DET, A_NONE, PM_ARCHEOLOGIST, NON_PM, 0L,
	0,0,0),

/* Anachrononaut */
A("The Force Pike of the Red Guard", FORCE_PIKE,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	STUN(1, 0),	NO_DFNS,	NO_CARY,
	0, A_NONE, PM_ANACHRONONAUT, NON_PM, 0L,
	0,0,0),

/* Barbarian */
/* TODO increase STR, DEX, CON by damage taken average */
/* TODO gaining CON increases hp with max hp */
A("The Gauntlets of the Berserker", LEATHER_GLOVES,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NONE, PM_BARBARIAN, NON_PM, 0L,
	0,0,0),

/* Binder */
A("The Declaration of the Apostate", SCR_REMOVE_CURSE,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	UNBIND_SEALS, A_NONE, PM_EXILE, NON_PM, 0L,
	0,0,0),

A("The Soul Lens", LENSES,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_WARN),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NONE, PM_EXILE, NON_PM, 0L,
	0,0,0),

A("The Seal of the Spirits", SCR_BLANK_PAPER,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NONE, PM_EXILE, NON_PM, 0L,
	0,0,0),

/* Caveman/Cavewoman */
/* TODO use club skill */
A("The Torch of Origins", WAN_FIRE,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	FIRE(0,5),	DFNS(AD_FIRE),	NO_CARY,
	0, A_NONE, PM_CAVEMAN, NON_PM, 0L,
	SPFX2_FIRE2,0,0),

/* Convict */
/* TODO */
A("The Striped Shirt of the Murderer", STRIPED_SHIRT,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NONE, PM_CONVICT, NON_PM, 0L,
	0,0,0),

/* TODO protect from theft */
/* TODO implement STEAL */
A("The Striped Shirt of the Thief", STRIPED_SHIRT,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	STEAL, A_NONE, PM_CONVICT, NON_PM, 0L,
	0,0,0),

/* TODO */
A("The Striped Shirt of the Falsely Accused", STRIPED_SHIRT,
	(SPFX_NOGEN|SPFX_RESTR), (SPFX_LUCK),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NONE, PM_CONVICT, NON_PM, 0L,
	0,0,0),

/* Healer */
/* TODO 2x vs living */
A("Scalpel of Life and Death",	SCALPEL,
	(SPFX_NOGEN|SPFX_RESTR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	LIFE_DEATH,	A_NONE, PM_HEALER, NON_PM, 0L,
	0,0,WSFX_PLUSSEV),

A("The Gauntlets of the Healing Hand",	GAUNTLETS_OF_DEXTERITY,
	(SPFX_NOGEN|SPFX_RESTR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	HEAL_PETS,	A_NONE, PM_HEALER, NON_PM, 0L,
	0,0,WSFX_PLUSSEV),

A("The Ring of Hygiene's Disciple",	RIN_REGENERATION,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_REGEN), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	HEALING,	A_NONE, PM_HEALER, NON_PM, 0L,
	0,0,0),

/* Knight */
A("The Cope of the Eldritch Knight",	ROBE, /*double robe effect*/
	(SPFX_NOGEN|SPFX_RESTR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	FREE_SPELL,	A_NONE, PM_KNIGHT, NON_PM, 0L,
	0,0,WSFX_PLUSSEV),

A("The Shield of the Paladin",	KITE_SHIELD,
	(SPFX_NOGEN|SPFX_RESTR), SPFX_WARN,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_UNDEAD|MA_DEMON) /*MA*/, 0 /*MV*/,
	NO_ATTK,	DFNS(AD_MAGM),	NO_CARY,
	BURN_WARD,	A_NONE, PM_KNIGHT, NON_PM, 0L,
	0,0,WSFX_PLUSSEV),

/* Monk */
A("The Booze of the Drunken Master",	POT_BOOZE,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	FIRE_BLAST,	A_NONE, PM_MONK, NON_PM, 0L,
	0,0,0),

/* TODO 2x damage against undead/demons */
A("The Wrappings of the Sacred Fist",	LEATHER_GLOVES,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	FAST_TURNING,	A_NONE, PM_MONK, NON_PM, 0L,
	0,0,WSFX_PLUSSEV),

/* TODO jumping while wielded */
/* TODO staggering blows while wielded */
A("The Khakkhara of the Monkey",	SILVER_KHAKKHARA,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,	A_NONE, PM_MONK, NON_PM, 0L,
	SPFX2_SILVERED,0,0),

/* Nobleman/Noblewoman */
/* TODO */
A("The Ruffled Shirt of the Aristocrat",	RUFFLED_SHIRT,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,	A_NONE, PM_NOBLEMAN, NON_PM, 0L,
	0,0,0),

/* TODO */
A("The Victorian Underwear of the Aristocrat",	VICTORIAN_UNDERWEAR,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0,	A_NONE, PM_NOBLEMAN, NON_PM, 0L,
	0,0,0),

/* TODO name by appearance */
/* TODO implement LOOT_GOLD */
/* TODO PM_NOBLEWOMAN */
A("The Mark of the Rightful Scion", RIN_TELEPORT_CONTROL, /* gold */
	(SPFX_NOGEN|SPFX_RESTR), (SPFX_WARN),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_ORC) /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,	
	COLLECT_TAX, A_NONE, PM_NOBLEMAN, NON_PM, 0L, 
	0,0,0),

/* Priest/Priestess */
A("The Gauntlets of the Divine Disciple",	ORIHALCYON_GAUNTLETS,
	(SPFX_NOGEN|SPFX_RESTR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	PROTECT,	A_NONE, PM_PRIEST, NON_PM, 0L,
	0,0,WSFX_PLUSSEV),

A("The Mace of the Evangelist",	MACE,
	(SPFX_NOGEN|SPFX_RESTR), 0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5, 0),	NO_DFNS,	NO_CARY,
	SMITE,	A_NONE, PM_PRIEST, NON_PM, 0L,
	0,0,0),

/* Pirate */
/* Rogue */
/* TODO appearance when poisoned/drugged etc */
/* TODO multishot */
A("The Dart of the Assassin", DART,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	SELF_POISON, A_NONE, PM_ROGUE, NON_PM, 0L,
	0,0,0),

/* TODO only name w/ expert short sword */
A("The Sword of the Kleptomaniac", SHORT_SWORD,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5, 5),	DFNS(AD_MAGM),	NO_CARY,
	THEFT_TYPE, A_NONE, PM_ROGUE, NON_PM, 0L,
	SPFX2_STEAL,0,0),

/* Ranger */
/* TODO confirm proper handling of artifact_hit w/ Chris_ANG */
A("The Helm of the Arcane Archer", LEATHER_HELM,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NONE, PM_RANGER, NON_PM, 0L,
	SPFX2_COLD|SPFX2_FIRE|SPFX2_ELEC,0,0),

/* TODO naming gender */
/* TODO stoning resistance */
/* TODO apply */
A("The Figurine of Pygmalion", FIGURINE,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NONE, PM_RANGER, NON_PM, 0L,
	0,0,0),

/* TODO naming gender */
/* TODO sex resistance */
/* TODO apply */
A("The Figurine of Galatea", FIGURINE,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	SUMMON_PET, A_NONE, PM_RANGER, NON_PM, 0L,
	0,0,0),

/* Samurai */
A("The Helm of the Ninja", HELM_OF_OPPOSITE_ALIGNMENT,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	ADD_POISON, A_NONE, PM_SAMURAI, NON_PM, 0L,
	0,0,WSFX_PLUSSEV),

/* Tourist */
/* TODO towel wetting as in vanilla 3.6.0 */
/* TODO 2x damage while wet */
/* TODO +1 whip skill while wielded */
/* TODO create tinned biscuits instead of lichen */
A("The Towel of the Interstellar Hitchhiker", TOWEL,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	PHYS(5, 0),	DFNS(AD_COLD),	NO_CARY,
	TOWEL_ITEMS, A_NONE, PM_TOURIST, NON_PM, 0L,
	0,0,0),

A("The Encyclopedia Galactica", SPE_BLANK_PAPER,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_LUCK), (SPFX_WARN|SPFX_ESP),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	MAJ_RUMOR, A_NONE, PM_TOURIST, NON_PM, 0L,
	0,0,0),

/* Troubadour */
/* Valkyrie */
A("The Twig of Yggdrasil", WAN_TELEPORTATION,
	(SPFX_NOGEN|SPFX_RESTR), (SPFX_TCTRL),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	CREATE_PORTAL, A_NONE, PM_VALKYRIE, NON_PM, 0L,
	0,0,0),

/* TODO flying pets */
A("The Saddle of Brynhildr", SADDLE,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,
	0, A_NONE, PM_VALKYRIE, NON_PM, 0L,
	0,0,0),

/* Wizard */
A("The Staff of Wild Magic",			QUARTERSTAFF,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK|SPFX_SEARCH|SPFX_LUCK|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	STUN(20,4),	NO_DFNS,	NO_CARY,
	ENERGY_BOOST, A_NONE, PM_WIZARD, NON_PM, 0L, 
	SPFX2_DIG|SPFX2_FIRE|SPFX2_COLD|SPFX2_ELEC|SPFX2_RAM2,0,0),

/* TODO exploding spell dice */
/* TODO remove as crowning option */
A("The Robe of the Archmagi",			ROBE,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_WARN|SPFX_XRAY|SPFX_INTEL),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	DFNS(AD_MAGM),	NO_CARY,	
	ENERGY_BOOST, A_NONE, PM_WIZARD, NON_PM, 0L, 
	0,0,WSFX_PLUSSEV),

/* TODO 2x damage vs non-living */
A("The Forge Hammer of the Artificer",			WAR_HAMMER,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,	
	ARTIFICE, A_NONE, PM_WIZARD, NON_PM, 0L, 
	0,0,0),

/* Drow */
/* TODO name by appearance */
A("The Ring of Lolth", RIN_PROTECTION_FROM_SHAPE_CHAN, /* black signet */
	(SPFX_NOGEN|SPFX_RESTR), (SPFX_WARN),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_ORC) /*MA*/, 0 /*MV*/,
	NO_ATTK,	DFNS(AD_FIRE),	NO_CARY,	
	FIRE_BLAST, A_NONE, NON_PM, PM_DROW, 0L, 
	0,0,0),

/* Dwarf */
A("The Bulwark of the Dwarven Defender", DWARVISH_ROUNDSHIELD,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	DFNS(AD_MAGM),	NO_CARY,	
	BLESS, A_NONE, NON_PM, PM_DWARF, 0L, 
	0,0,WSFX_PLUSSEV),

/* Elf */
/* TODO name by appearance */
A("Narya", RIN_TELEPORT_CONTROL, /* gold */
	(SPFX_NOGEN|SPFX_RESTR), (SPFX_WARN),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_ORC) /*MA*/, 0 /*MV*/,
	NO_ATTK,	DFNS(AD_FIRE),	NO_CARY,	
	FIRE_BLAST, A_NONE, NON_PM, PM_ELF, 0L, 
	0,0,0),

/* TODO water walking */
/* TODO protect inventory from water damage */
/* TODO name by appearance */
A("Nenya", RIN_TELEPORTATION, /* silver */
	(SPFX_NOGEN|SPFX_RESTR), (SPFX_WARN),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_ORC)/*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,	
	0, A_NONE, NON_PM, PM_ELF, 0L, 
	0,0,0),

/* TODO name by appearance */
A("Vilya", RIN_AGGRAVATE_MONSTER, /* sapphire */
	(SPFX_NOGEN|SPFX_RESTR), (SPFX_WARN),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, (MA_ORC)/*MA*/, 0 /*MV*/,
	NO_ATTK,	DFNS(AD_ELEC),	NO_CARY,	
	HEALING, A_NONE, NON_PM, PM_ELF, 0L, 
	0,0,0),

/* Gnome */
/* TODO +1d5 bth against med+ */
/* TODO warn against medium+ */
A("The Hat of the Giant Killer", GNOMISH_POINTY_HAT,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_HPHDAM), (SPFX_WARN),
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,	
	0, A_NONE, NON_PM, PM_GNOME, 0L, 
	0,0,WSFX_PLUSSEV),

/* Half-Dragon */
/* TODO pet dragons +1 beast mastery */
A("The Prismatic Dragon Plate", PLATE_MAIL,
	(SPFX_NOGEN|SPFX_RESTR|SPFX_HPHDAM),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,	
	PRISMATIC, A_NONE, NON_PM, PM_HALF_DRAGON, 0L, 
	0,0,WSFX_PLUSSEV),

/* Human */
/* Incantifier */
/* TODO random clairvoyance */
A("Footprints in the Labyrinth", STAR_SAPPHIRE,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	NO_DFNS,	NO_CARY,	
	ENLIGHTENING, A_NONE, NON_PM, PM_INCANTIFIER, 0L, 
	SPFX2_SPELLUP,0,0),

/* Orc */
/* Vampire */
/* TODO grant darkvision */
/* TODO implement SUMMON_VAMP */
A("The Trappings of the Grave", AMULET_OF_RESTFUL_SLEEP,
	(SPFX_NOGEN|SPFX_RESTR),0,
	0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/,
	NO_ATTK,	DFNS(AD_MAGM),	NO_CARY,	
	SUMMON_VAMP, A_NONE, NON_PM, PM_VAMPIRE, 0L, 
	0,0,WSFX_PLUSSEV),

/*
 *  terminator; otyp must be zero
 */
A(0, 0, 0, 0, 0 /*Monster Symbol*/, 0 /*MM*/, 0 /*MT*/, 0 /*MB*/, 0 /*MG*/, 0 /*MA*/, 0 /*MV*/, 
	NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 0L, 0,0,0 )

};	/* artilist[] (or artifact_names[]) */

#undef	A

#ifndef MAKEDEFS_C
#undef	NO_ATTK
#undef	NO_DFNS
#undef	DFNS
#undef	PHYS
#undef	DRLI
#undef	COLD
#undef	FIRE
#undef	ELEC
#undef	STUN
#endif

/*artilist.h*/
