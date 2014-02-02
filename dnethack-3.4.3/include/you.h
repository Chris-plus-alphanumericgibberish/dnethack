/*	SCCS Id: @(#)you.h	3.4	2000/05/21	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef YOU_H
#define YOU_H

#include "attrib.h"
#include "monst.h"
#ifndef PROP_H
#include "prop.h"		/* (needed here for util/makedefs.c) */
#endif
#include "skills.h"

/*** Substructures ***/

struct RoleName {
	const char	*m;	/* name when character is male */
	const char	*f;	/* when female; null if same as male */
};

struct RoleAdvance {
	/* "fix" is the fixed amount, "rnd" is the random amount */
	xchar infix, inrnd;	/* at character initialization */
	xchar lofix, lornd;	/* gained per level <  urole.xlev */
	xchar hifix, hirnd;	/* gained per level >= urole.xlev */
};

struct u_have {
	Bitfield(amulet,1);	/* carrying Amulet	*/
	Bitfield(bell,1);	/* carrying Bell	*/
	Bitfield(book,1);	/* carrying Book	*/
	Bitfield(menorah,1);	/* carrying Candelabrum */
	Bitfield(questart,1);	/* carrying the Quest Artifact */
	Bitfield(unused,3);
};

struct u_event {
	Bitfield(minor_oracle,1);	/* received at least 1 cheap oracle */
	Bitfield(major_oracle,1);	/*  "  expensive oracle */
	Bitfield(qcalled,1);		/* called by Quest leader to do task */
	Bitfield(qexpelled,1);		/* expelled from the Quest dungeon */
	Bitfield(qcompleted,1);		/* successfully completed Quest task */
	Bitfield(uheard_tune,2);	/* 1=know about, 2=heard passtune */
	Bitfield(uopened_dbridge,1);	/* opened the drawbridge */
	Bitfield(uread_necronomicon,1);		/* have read the necronomicon */

	Bitfield(invoked,1);		/* invoked Gate to the Sanctum level */
	Bitfield(gehennom_entered,1);	/* entered Gehennom via Valley */
	Bitfield(uhand_of_elbereth,2);	/* became Hand of Elbereth */
	Bitfield(udemigod,1);		/* killed the wiz */
	Bitfield(ukilled_apollyon,1);		/* killed the angel of the pit.  Lucifer should spawn on Astral */
	Bitfield(sum_entered,1);		/* entered Sum-of-All */
	Bitfield(uaxus_foe,1);		/* enemy of the modrons */
	Bitfield(ascended,1);		/* has offered the Amulet */
};

/* KMH, conduct --
 * These are voluntary challenges.  Each field denotes the number of
 * times a challenge has been violated.
 */
struct u_conduct {		/* number of times... */
	long	unvegetarian;	/* eaten any animal */
	long	unvegan;	/* ... or any animal byproduct */
	long	food;		/* ... or any comestible */
	long	ratseaten;	/* number of rats eaten */
	long	gnostic;	/* used prayer, priest, or altar */
	long	weaphit;	/* hit a monster with a weapon */
	long	killer;		/* killed a monster yourself */
	long	wardless;	/* drew a warding symbol */
	long	elbereth;	/* wrote elbereth */
	long	literate;	/* read something (other than BotD) */
	long	polypiles;	/* polymorphed an object */
	long	polyselfs;	/* transformed yourself */
	long	wishes;		/* used a wish */
	long	wisharti;	/* wished for an artifact */
	long	shopID;		/* number of items id by shopkeepers */
	long	IDs;		/* number of items id by magic or shopkeepers */
				/* genocides already listed at end of game */
};

/*** Unified structure containing role information ***/
struct Role {
	/*** Strings that name various things ***/
	struct RoleName name;	/* the role's name (from u_init.c) */
	struct RoleName rank[9]; /* names for experience levels (from botl.c) */
	const char *lgod, *ngod, *cgod; /* god names (from pray.c) */
	const char *filecode;	/* abbreviation for use in file names */
	const char *homebase;	/* quest leader's location (from questpgr.c) */
	const char *intermed;	/* quest intermediate goal (from questpgr.c) */

	/*** Indices of important monsters and objects ***/
	short malenum,		/* index (PM_) as a male (botl.c) */
	      femalenum,	/* ...or as a female (NON_PM == same) */
	      petnum,		/* PM_ of preferred pet (NON_PM == random) */
	      ldrnum,		/* PM_ of quest leader (questpgr.c) */
	      guardnum,		/* PM_ of quest guardians (questpgr.c) */
	      neminum,		/* PM_ of quest nemesis (questpgr.c) */
	      enemy1num,	/* specific quest enemies (NON_PM == random) */
	      enemy2num;
	char  enemy1sym,	/* quest enemies by class (S_) */
	      enemy2sym;
	short questarti;	/* index (ART_) of quest artifact (questpgr.c) */

	/*** Bitmasks ***/
	short allow;		/* bit mask of allowed variations */
#define ROLE_RACEMASK	0x0ff8		/* allowable races */
#define ROLE_GENDMASK	0xf000		/* allowable genders */
#define ROLE_MALE	0x1000
#define ROLE_FEMALE	0x2000
#define ROLE_NEUTER	0x4000
#define ROLE_ALIGNMASK	AM_MASK		/* allowable alignments */
#define ROLE_LAWFUL	AM_LAWFUL
#define ROLE_NEUTRAL	AM_NEUTRAL
#define ROLE_CHAOTIC	AM_CHAOTIC

	/*** Attributes (from attrib.c and exper.c) ***/
	xchar attrbase[A_MAX];	/* lowest initial attributes */
	xchar attrdist[A_MAX];	/* distribution of initial attributes */
	struct RoleAdvance hpadv; /* hit point advancement */
	struct RoleAdvance enadv; /* energy advancement */
	xchar xlev;		/* cutoff experience level */
	xchar initrecord;	/* initial alignment record */

	/*** Spell statistics (from spell.c) ***/
	int spelbase;		/* base spellcasting penalty */
	int spelheal;		/* penalty (-bonus) for healing spells */
	int spelshld;		/* penalty for wearing any shield */
	int spelarmr;		/* penalty for wearing metal armour */
	int spelstat;		/* which stat (A_) is used */
	int spelspec;		/* spell (SPE_) the class excels at */
	int spelsbon;		/* penalty (-bonus) for that spell */

	/*** Properties in variable-length arrays ***/
	/* intrinsics (see attrib.c) */
	/* initial inventory (see u_init.c) */
	/* skills (see u_init.c) */

	/*** Don't forget to add... ***/
	/* quest leader, guardians, nemesis (monst.c) */
	/* quest artifact (artilist.h) */
	/* quest dungeon definition (dat/Xyz.dat) */
	/* quest text (dat/quest.txt) */
	/* dictionary entries (dat/data.bas) */
};

extern const struct Role roles[];	/* table of available roles */
extern struct Role urole;
#define Role_if(X)	(urole.malenum == (X))
#define Role_switch	(urole.malenum)

/* used during initialization for race, gender, and alignment
   as well as for character class */
#define ROLE_NONE	(-1)
#define ROLE_RANDOM	(-2)

/*** Unified structure specifying race information ***/

struct Race {
	/*** Strings that name various things ***/
	const char *noun;	/* noun ("human", "elf") */
	const char *adj;	/* adjective ("human", "elven") */
	const char *coll;	/* collective ("humanity", "elvenkind") */
	const char *filecode;	/* code for filenames */
	struct RoleName individual; /* individual as a noun ("man", "elf") */

	/*** Indices of important monsters and objects ***/
	short malenum,		/* PM_ as a male monster */
	      femalenum,	/* ...or as a female (NON_PM == same) */
	      mummynum,		/* PM_ as a mummy */
	      zombienum;	/* PM_ as a zombie */

	/*** Bitmasks ***/
	short allow;		/* bit mask of allowed variations */
	short selfmask,		/* your own race's bit mask */
	      lovemask,		/* bit mask of always peaceful */
	      hatemask;		/* bit mask of always hostile */

	/*** Attributes ***/
	xchar attrmin[A_MAX];	/* minimum allowable attribute */
	xchar attrmax[A_MAX];	/* maximum allowable attribute */
	struct RoleAdvance hpadv; /* hit point advancement */
	struct RoleAdvance enadv; /* energy advancement */
	int   nv_range;		/* night vision range */
#define NO_NIGHTVISION	0
#define NIGHTVISION2	2
#define NIGHTVISION3	3
#if 0	/* DEFERRED */
	int   xray_range;	/* X-ray vision range */
#endif

	/*** Properties in variable-length arrays ***/
	/* intrinsics (see attrib.c) */

	/*** Don't forget to add... ***/
	/* quest leader, guardians, nemesis (monst.c) */
	/* quest dungeon definition (dat/Xyz.dat) */
	/* quest text (dat/quest.txt) */
	/* dictionary entries (dat/data.bas) */
};

extern const struct Race races[];	/* Table of available races */
extern struct Race urace;
#define Race_if(X)	(urace.malenum == (X))
#define Race_switch	(urace.malenum)

/*** Unified structure specifying gender information ***/
struct Gender {
	const char *adj;	/* male/female/neuter */
	const char *he;		/* he/she/it */
	const char *him;	/* him/her/it */
	const char *his;	/* his/her/its */
	const char *filecode;	/* file code */
	short allow;		/* equivalent ROLE_ mask */
};
#define ROLE_GENDERS	2	/* number of permitted player genders */
				/* increment to 3 if you allow neuter roles */

extern const struct Gender genders[];	/* table of available genders */
#define uhe()	(genders[flags.female ? 1 : 0].he)
#define uhim()	(genders[flags.female ? 1 : 0].him)
#define uhis()	(genders[flags.female ? 1 : 0].his)
#define mhe(mtmp)	(genders[pronoun_gender(mtmp)].he)
#define mhim(mtmp)	(genders[pronoun_gender(mtmp)].him)
#define mhis(mtmp)	(genders[pronoun_gender(mtmp)].his)


/*** Unified structure specifying alignment information ***/
struct Align {
	const char *noun;	/* law/balance/chaos */
	const char *adj;	/* lawful/neutral/chaotic */
	const char *filecode;	/* file code */
	short allow;		/* equivalent ROLE_ mask */
	aligntyp value;		/* equivalent A_ value */
};
#define ROLE_ALIGNS	3	/* number of permitted player alignments */

extern const struct Align aligns[];	/* table of available alignments */


/*** Information about the player ***/
struct you {
	xchar ux, uy;
	schar dx, dy, dz;	/* direction of move (or zap or ... ) */
	schar di;		/* direction of FF */
	xchar tx, ty;		/* destination of travel */
	xchar ux0, uy0;		/* initial position FF */
	d_level uz, uz0;	/* your level on this and the previous turn */
	d_level utolev;		/* level monster teleported you to, or uz */
	uchar utotype;		/* bitmask of goto_level() flags for utolev */
	boolean umoved;		/* changed map location (post-move) */
	int last_str_turn;	/* 0: none, 1: half turn, 2: full turn */
				/* +: turn right, -: turn left */
	int ulevel;		/* 1 to MAXULEV */
	int ulevelmax;
	unsigned utrap;		/* trap timeout */
	unsigned utraptype;	/* defined if utrap nonzero */
#define TT_BEARTRAP	0
#define TT_PIT		1
#define TT_WEB		2
#define TT_LAVA		3
#define TT_INFLOOR	4
	char	urooms[5];	/* rooms (roomno + 3) occupied now */
	char	urooms0[5];	/* ditto, for previous position */
	char	uentered[5];	/* rooms (roomno + 3) entered this turn */
	char	ushops[5];	/* shop rooms (roomno + 3) occupied now */
	char	ushops0[5];	/* ditto, for previous position */
	char	ushops_entered[5]; /* ditto, shops entered this turn */
	char	ushops_left[5]; /* ditto, shops exited this turn */

	int	 uhunger;	/* refd only in eat.c and shk.c */
#define YouHunger	(Race_if(PM_INCANTIFIER) ? u.uen : u.uhunger)
	unsigned uhs;		/* hunger state - see eat.c */

	boolean ukinghill; /* records if you are carying the pirate treasure (and are therefor king of the hill) */
	int protean; /* counter for the auto-polypiling power of the pirate treasure*/
	int uhouse; /* drow house info */
	struct prop uprops[LAST_PROP+1];

	unsigned umconf;
	char usick_cause[PL_PSIZ+20]; /* sizeof "unicorn horn named "+1 */
	Bitfield(usick_type,2);
#define SICK_VOMITABLE 0x01
#define SICK_NONVOMITABLE 0x02
#define SICK_ALL 0x03

	/* These ranges can never be more than MAX_RANGE (vision.h). */
	int nv_range;		/* current night vision range */
	int xray_range;		/* current xray vision range */

	/*
	 * These variables are valid globally only when punished and blind.
	 */
#define BC_BALL  0x01	/* bit mask for ball  in 'bc_felt' below */
#define BC_CHAIN 0x02	/* bit mask for chain in 'bc_felt' below */
	int bglyph;	/* glyph under the ball */
	int cglyph;	/* glyph under the chain */
	int bc_order;	/* ball & chain order [see bc_order() in ball.c] */
	int bc_felt;	/* mask for ball/chain being felt */

	int umonster;			/* hero's "real" monster num */
	int umonnum;			/* current monster number */

	int mh, mhmax, mtimedone;	/* for polymorph-self */
	struct attribs	macurr,		/* for monster attribs */
			mamax;		/* for monster attribs */
	int ulycn;			/* lycanthrope type */
	short ucspeed;
#define	HIGH_CLOCKSPEED	1
#define	NORM_CLOCKSPEED	2
#define	SLOW_CLOCKSPEED	3
	unsigned ucreamed;
	unsigned uswldtim;		/* time you have been swallowed */

	Bitfield(uswallow,1);		/* true if swallowed */
	Bitfield(uinwater,1);		/* if you're currently in water (only
					   underwater possible currently) */
	Bitfield(uundetected,1);	/* if you're a hiding monster/piercer */
	Bitfield(mfemale,1);		/* saved human value of flags.female */
	Bitfield(uinvulnerable,1);	/* you're invulnerable (praying) */
	Bitfield(uburied,1);		/* you're buried */
	Bitfield(uedibility,1);		/* blessed food detection; sense unsafe food */
	/* 1 free bit! */

	unsigned udg_cnt;		/* how long you have been demigod */
	struct u_event	uevent;		/* certain events have happened */
	struct u_have	uhave;		/* you're carrying special objects */
	struct u_conduct uconduct;	/* KMH, conduct */
	struct attribs	acurr,		/* your current attributes (eg. str)*/
			aexe,		/* for gain/loss via "exercise" */
			abon,		/* your bonus attributes (eg. str) */
			amax,		/* your max attributes (eg. str) */
			atemp,		/* used for temporary loss/gain */
			atime;		/* used for loss/gain countdown */
	align	ualign;			/* character alignment */
#define CONVERT		2
#define A_ORIGINAL	1
#define A_CURRENT	0
	aligntyp ualignbase[CONVERT];	/* for ualign conversion record */
	schar uluck, moreluck;		/* luck and luck bonus */
	int luckturn;
#define Luck	(u.uluck + u.moreluck)
#define LUCKADD		3	/* added value when carrying luck stone */
#define LUCKMAX		10	/* on moonlit nights 11 */
#define LUCKMIN		(-10)
	schar	uhitinc;
	schar	udaminc;
	int		ucarinc;		/* bonus carrying capacity*/
	schar	uacinc;			/* bonus AC (not spell/divine) */
	schar	uac;
	uchar	uspellprot;		/* protection by SPE_PROTECTION */
	uchar	usptime;		/* #moves until uspellprot-- */
	uchar	uspmtime;		/* #moves between uspellprot-- */
	int	uhp,uhpmax;
	int	uen, uenmax;		/* magical energy - M. Stephenson */
	int ugangr;			/* if the gods are angry at you */
	int ugifts;			/* number of artifacts bestowed */
	int ublessed, ublesscnt;	/* blessing/duration from #pray */
#ifndef GOLDOBJ
	long	ugold, ugold0;
#else
	long	umoney0;
#endif
	long	uexp, urexp;
	long	ucleansed;	/* to record moves when player was cleansed */
	long	usleep;		/* sleeping; monstermove you last started */
	int uinvault;
	struct monst *ustuck;
#ifdef STEED
	struct monst *usteed;
	long ugallop;
	int urideturns;
#endif
	int	umortality;		/* how many times you died */
	int ugrave_arise; /* you die and become something aside from a ghost */
	time_t	ubirthday;		/* real world time when game began */

	int	weapon_slots;		/* unused skill slots */
	int	skills_advanced;		/* # of advances made so far */
	xchar	skill_record[P_SKILL_LIMIT];	/* skill advancements */
	struct skills weapon_skills[P_NUM_SKILLS];
	boolean twoweap;		/* KMH -- Using two-weapon combat */

	long	wardsknown;	/* known wards */
#define	WARD_ELBERETH		0x0000001L
#define WARD_HEPTAGRAM		0x0000002L
#define WARD_GORGONEION		0x0000004L
#define WARD_ACHERON		0x0000008L
#define WARD_PENTAGRAM		0x0000010L
#define WARD_HEXAGRAM		0x0000020L
#define WARD_HAMSA			0x0000040L
#define WARD_ELDER_SIGN		0x0000080L
#define WARD_EYE			0x0000100L
#define WARD_QUEEN			0x0000200L
#define WARD_CAT_LORD		0x0000400L
#define WARD_GARUDA			0x0000800L
#define WARD_CTHUGHA		0x0001000L
#define WARD_ITHAQUA		0x0002000L
#define WARD_KARAKAL		0x0004000L
#define WARD_YELLOW			0x0008000L
#define WARD_TOUSTEFNA		0x0010000L
#define WARD_DREPRUN		0x0020000L
#define WARD_OTTASTAFUR		0x0040000L
#define WARD_KAUPALOKI		0x0080000L
#define WARD_VEIOISTAFUR	0x0100000L
#define WARD_THJOFASTAFUR	0x0200000L


	short sealorder[31];
	long	sealsKnown;
#define SEAL_AHAZU					0x0000001L
	long	ahazu;	//turn on which spirit will be again eligible for binding.
#define SEAL_AMON					0x0000002L
	long	amon;
#define SEAL_ANDREALPHUS			0x0000004L
	long	andrealphus;
#define SEAL_ANDROMALIUS			0x0000008L
	long	andromalius;
#define SEAL_ASTAROTH				0x0000010L
	long	astaroth;
#define SEAL_BALAM					0x0000020L
	long	balam;
#define SEAL_BERITH					0x0000040L
	long	berith;
#define SEAL_BUER					0x0000080L
	long	buer;
#define SEAL_CHUPOCLOPS				0x0000100L
	long	chupoclops;
#define SEAL_DANTALION				0x0000200L
	long	dantalion;
#define SEAL_SHIRO				0x0000400L
	long	shiro;
#define SEAL_ECHIDNA				0x0000800L
	long	echidna;
#define SEAL_EDEN					0x0001000L
	long	eden;
#define SEAL_ERIDU					0x0002000L
	long	eridu;
#define SEAL_EURYNOME				0x0004000L
	long	eurynome;
#define SEAL_EVE					0x0008000L
	long	eve;
#define SEAL_FAFNIR					0x0010000L
	long	fafnir;
#define SEAL_HUGINN_MUNINN			0x0020000L
	long	huginn_muninn;
#define SEAL_IRIS					0x0040000L
	long	iris;
#define SEAL_JACK					0x0080000L
	long	jack;
#define SEAL_MALPHAS				0x0100000L
	long	malphas;
#define SEAL_MARIONETTE				0x0200000L
	long	marionette;
#define SEAL_MOTHER					0x0400000L
	long	mother;
#define SEAL_NABERIUS				0x0800000L
	long	naberius;
#define SEAL_ORTHOS					0x1000000L
	long	orthos;
#define SEAL_OSE					0x2000000L
	long	ose;
#define SEAL_OTIAX					0x4000000L
	long	otiax;
#define SEAL_PAIMON					0x8000000L
	long	paimon;
#define SEAL_SIMURGH				0x10000000L
	long	simurgh;
#define SEAL_TENEBROUS				0x20000000L
	long	tenebrous;
#define SEAL_YMIR					0x40000000L
	long	ymir;

//The remaining seals (Dahlver-Nar, Acererak, and the Numina) can't be learned in any way other than binder class features
#define SEAL_DAHLVER_NAR			0x0000001L
	long	dahlver_nar;
#define SEAL_ACERERAK				0x0000002L
	long	acererak;
#define SEAL_NUMINA					0x0000004L
//	long	numina;	//numina does not expire, and can be immediatly re-bound once 30th level is achived if the pact is broken.
	
	char sealCounts;
	long sealsActive;
	long specialSealsActive;
	
	//Spirits in order bound:
	long spirit[5];
	long spiritQuest,spiritTineA,spiritTineB;
	//Corresponding timeouts (turn on which binding expires):
	long spiritT[5];
	long spiritQuestT,spiritTineTA,spiritTineTB;

	/* 	variable that keeps track of summoning in your vicinity.
		Only allow 1 per turn, to help reduce summoning cascades. */
	boolean summonMonster;
	/*Ugly extra artifact variables workaround.  Spaghetti code alert!*/
	long SnSd1, SnSd2, SnSd3, SnSd3duration;
	int ZangetsuSafe;
	short RoSPkills, RoSPflights;
	/*Keter counters*/
	int keter, chokhmah, gevurah, hod;
	int regifted; /*keeps track of how many artifacts the player has given to the unknown god*/
};	/* end of `struct you' */
#define uclockwork ((Race_if(PM_CLOCKWORK_AUTOMATON) && !Upolyd) || (Upolyd && youmonst.data == &mons[PM_CLOCKWORK_AUTOMATON]))

extern long sealKey[31]; /*Defined in */
extern char *wardDecode[26]; /*Defined in spell.c*/
extern int wardMax[16]; /*Defined in engrave.c*/
extern char *andromaliusItems[18]; /*Defined in */
#define Upolyd (u.umonnum != u.umonster)

#endif	/* YOU_H */
