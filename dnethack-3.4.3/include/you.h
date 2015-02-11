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
#include "engrave.h"

/*** Substructures ***/

struct RoleName {
	char	*m;	/* name when character is male */
	char	*f;	/* when female; null if same as male */
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
	Bitfield(knoweddergud,1);		/* know the identity of the black-web god */

	Bitfield(invoked,1);		/* invoked Gate to the Sanctum level */
	Bitfield(gehennom_entered,1);	/* entered Gehennom via Valley */
	Bitfield(uhand_of_elbereth,5);	/* became Hand of Elbereth */
	Bitfield(udemigod,1);		/* killed the wiz */
	Bitfield(ukilled_apollyon,1);		/* killed the angel of the pit.  Lucifer should spawn on Astral */
	Bitfield(ukilled_illurien,1);		/* Harassment */
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
#define Pantheon_if(X)	(roles[flags.pantheon].malenum == (X))
#define Role_switch	(urole.malenum)

/* used during initialization for race, gender, and alignment
   as well as for character class */
#define ROLE_NONE	(-1)
#define ROLE_RANDOM	(-2)

struct mask_properties {
	int msklevel;
	int mskmonnum;
	int mskrolenum;
	Bitfield(mskfemale,1);
	struct attribs	mskacurr,
					mskaexe,
					mskamask;
	align	mskalign;
	schar mskluck;
	int mskhp,mskhpmax;
	int msken,mskenmax;
	int mskgangr[GA_NUM];
	long mskexp, mskrexp;
	int	mskweapon_slots;		/* unused skill slots */
	int	mskskills_advanced;		/* # of advances made so far */
	xchar	mksskill_record[P_SKILL_LIMIT];	/* skill advancements */
	struct skills mask_skills[P_NUM_SKILLS];
};

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
#define NORMALNIGHTVIS	1
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
	int ulevel, ulevel_real;		/* 1 to MAXULEV */
	int ulevelmax, ulevelmax_real;
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
	int	 uhungermax;/*  */
#define YouHunger	(Race_if(PM_INCANTIFIER) ? u.uen : u.uhunger)
#define DEFAULT_HMAX	2000
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
	long clockworkUpgrades;
	int uboiler;
#define	MAX_BOILER	2000
	int ustove;
	int utemp;
#define	WARM			 1
#define	HOT				 2
#define	BURNING_HOT		 5
#define	MELTING			10
#define	MELTED			20
/*Note: because clockwork_eat_menu uses ints instead of longs, all upgrades that change how you eat must fit in an int.*/
#define OIL_STOVE			0x000001L
#define WOOD_STOVE			0x000002L
#define FAST_SWITCH			0x000004L
#define EFFICIENT_SWITCH	0x000008L
#define ARMOR_PLATING		0x000010L
#define PHASE_ENGINE		0x000020L
#define MAGIC_FURNACE		0x000040L
#define HELLFIRE_FURNACE	0x000080L
#define SCRAP_MAW			0x000100L
#define HIGH_TENSION		0x000200L
	
	int slowclock;
	
	unsigned ucreamed;
	unsigned uswldtim;		/* time you have been swallowed */

	Bitfield(uswallow,1);		/* true if swallowed */
	Bitfield(uinwater,1);		/* if you're currently in water */
	Bitfield(usubwater,1);		/* if you're currently underwater */
	Bitfield(uundetected,1);	/* if you're a hiding monster/piercer */
	Bitfield(mfemale,1);		/* saved human value of flags.female */
	Bitfield(uinvulnerable,1);	/* you're invulnerable (praying) */
	Bitfield(uburied,1);		/* you're buried */
	Bitfield(uedibility,1);		/* blessed food detection; sense unsafe food */
	/* 0 free bits? (doesn't add up?) */
	Bitfield(shambin,2);		/* Whether the shambling horror has normal innards, undifferentiated innards, or solid/nonexistent innards */
	Bitfield(stumbin,2);		/* Whether the stumbling horror has normal innards, undifferentiated innards, or solid/nonexistent innards */
	Bitfield(wandein,2);		/* Whether the wandering horror has normal innards, undifferentiated innards, or solid/nonexistent innards */
	Bitfield(umartial,1);		/* blessed food detection; sense unsafe food */
	Bitfield(phasengn,1);		/* clockwork phase engine */
	/* 24 free bits */
	
	int oonaenergy;				/* Record the energy type used by Oona in your game. (Worm that Walks switches?) */
	unsigned udg_cnt;		/* timer for wizard intervention WRONG?:how long you have been demigod */
	unsigned ill_cnt;		/* timer for illurien intervention */
	struct u_event	uevent;		/* certain events have happened */
	struct u_have	uhave;		/* you're carrying special objects */
	struct u_conduct uconduct;	/* KMH, conduct */
	struct attribs	acurr,		/* your current attributes (eg. str)*/
			aexe,		/* for gain/loss via "exercise" */
			abon,		/* your bonus attributes (eg. str) */
			amax,		/* your max attributes (eg. str) */
			atemp,		/* used for temporary loss/gain */
			atime;		/* used for loss/gain countdown */
	long exerchkturn;	/* Stat Excercise: What turn is the next exerchk? */		
	align	ualign;			/* character alignment */
#define CONVERT		2
#define A_ORIGINAL	1
#define A_CURRENT	0
	aligntyp ualignbase[CONVERT];	/* for ualign conversion record */
	schar uluck, moreluck;		/* luck and luck bonus */
	int luckturn;
#define Luck	(u.uluck + u.moreluck)
#define LUCKADD		3	/* added value when carrying luck stone */
#define DIELUCK		4	/* subtracted from current luck on final death */
#define LUCKMAX		10	/* on moonlit nights 11 */
#define LUCKMIN		(-10)
	schar	uhitinc;		/* bonus to-hit chance */
	schar	udaminc;		/* bonus damage */
	int		ucarinc;		/* bonus carrying capacity */
	schar	uacinc;			/* bonus AC (not spell/divine) */
	schar	uac;
	uchar	uspellprot;		/* protection by SPE_PROTECTION */
	uchar	udrunken;		/* drunkeness level (based on total number of potions of booze drunk) */
	uchar	usptime;		/* #moves until uspellprot-- */
	uchar	uspmtime;		/* #moves between uspellprot-- */
	uchar	sowdisc;		/* sowing discord (spirit special attack) */
	int	uhp,uhpmax,uhpmax_real;
	int	uen, uenmax,uenmax_real;		/* magical energy - M. Stephenson */
	int ugangr[GA_NUM];			/* if the gods are angry at you */
	int ugifts;			/* number of artifacts bestowed */
	int ublessed, ublesscnt;	/* blessing/duration from #pray */
	long lastprayed;
	uchar lastprayresult, reconciled;
#define	PRAY_NONE	0
#define	PRAY_GOOD	1
#define	PRAY_BAD	2
#define	PRAY_GIFT	3
#define	PRAY_ANGER	4
#define	PRAY_CONV	5

#define	REC_NONE	0
#define	REC_REC		1
#define	REC_MOL		2

#ifndef GOLDOBJ
	long	ugold, ugold0;
#else
	long	umoney0;
#endif
	long	uexp, urexp;
	long	ucleansed;	/* to record moves when player was cleansed */
	long	usleep;		/* sleeping; monstermove you last started */
	int 	ustdy;		/* to record extra damage to be delt due to having been studied */
	uchar 	wimage;		/* to record if you have the image of a Weeping Angel in your mind */
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
	int divetimer;			/* how long you can stay under water */

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
#define WARD_TRANSIT		0x0010000L
#define WARD_STABILIZE		0x0020000L
#define WARD_TOUSTEFNA		0x0040000L
#define WARD_DREPRUN		0x0080000L
#define WARD_OTTASTAFUR		0x0100000L
#define WARD_KAUPALOKI		0x0200000L
#define WARD_VEIOISTAFUR	0x0400000L
#define WARD_THJOFASTAFUR	0x0800000L

	
	int sealorder[31];
	long	sealsKnown;
	long	specialSealsKnown;
#define SEAL_AHAZU					0x0000001L
#define SEAL_AMON					0x0000002L
#define SEAL_ANDREALPHUS			0x0000004L
#define SEAL_ANDROMALIUS			0x0000008L
#define SEAL_ASTAROTH				0x0000010L
#define SEAL_BALAM					0x0000020L
#define SEAL_BERITH					0x0000040L
#define SEAL_BUER					0x0000080L
#define SEAL_CHUPOCLOPS				0x0000100L
#define SEAL_DANTALION				0x0000200L
#define SEAL_ECHIDNA				0x0000400L
#define SEAL_EDEN					0x0000800L
#define SEAL_ENKI					0x0001000L
#define SEAL_EURYNOME				0x0002000L
	int		eurycounts;
#define SEAL_EVE					0x0004000L
#define SEAL_FAFNIR					0x0008000L
#define SEAL_HUGINN_MUNINN			0x0010000L
#define SEAL_IRIS					0x0020000L
	long	irisAttack;
#define SEAL_JACK					0x0040000L
#define SEAL_MALPHAS				0x0080000L
#define SEAL_MARIONETTE				0x0100000L
#define SEAL_MOTHER					0x0200000L
#define SEAL_NABERIUS				0x0400000L
#define SEAL_ORTHOS					0x0800000L
	int		orthocounts;
#define SEAL_OSE					0x1000000L
	char	osepro[5];
	char	osegen[9];
#define SEAL_OTIAX					0x2000000L
	long	otiaxAttack;
#define SEAL_PAIMON					0x4000000L
#define SEAL_SHIRO					0x8000000L
#define SEAL_SIMURGH				0x10000000L
#define SEAL_TENEBROUS				0x20000000L
#define SEAL_YMIR					0x40000000L
//Special flag for lookup tables, indicating that it is a quest spirit and should be treated as such
#define SEAL_SPECIAL				0x80000000L

//The remaining seals (Dahlver-Nar, Acererak, and the Numina) can't be learned in any way other than binder class features
#define SEAL_DAHLVER_NAR			0x00000001L
#define SEAL_ACERERAK				0x00000002L
#define SEAL_COUNCIL				0x00000004L
#define SEAL_COSMOS					0x00000008L
#define SEAL_MISKA					0x00000010L
#define SEAL_NUDZIARTH				0x00000020L
#define SEAL_ALIGNMENT_THING		0x00000040L
#define SEAL_UNKNOWN_GOD			0x00000080L
#define SEAL_BLACK_WEB				0x00000100L
#define SEAL_NUMINA					0x40000000L
//	long	numina;	//numina does not expire, and can be immediatly re-bound once 30th level is achived if the pact is broken.
	
	int sealTimeout[NUMINA-FIRST_SEAL]; //turn on which spirit will be again eligible for binding.
	
	int sealCounts;
	long sealsActive;
	long sealsUsed;
	long specialSealsActive;
	long specialSealsUsed;
	
	int wisSpirits, intSpirits;
	
#define	GATE_SPIRITS	5
#define	QUEST_SPIRIT	5
#define	GPREM_SPIRIT	6
#define	CROWN_SPIRIT	7
#define	ALIGN_SPIRIT	8
#define	OUTER_SPIRIT	9
#define	NUM_BIND_SPRITS	10
	//Spirits in order bound:
	long spirit[NUM_BIND_SPRITS];
	long spiritTineA,spiritTineB;
	//Corresponding timeouts (turn on which binding expires):
	long spiritT[NUM_BIND_SPRITS];
	long spiritTineTA,spiritTineTB;
	
	int spiritAC;
	int spiritAttk;
	
#define	PWR_ABDUCTION				 0
#define	PWR_FIRE_BREATH				 1
#define	PWR_TRANSDIMENSIONAL_RAY	 2
#define	PWR_TELEPORT				 3
#define	PWR_JESTER_S_MIRTH			 4
#define	PWR_THIEF_S_INSTINCTS		 5
#define	PWR_ASTAROTH_S_ASSEMBLY		 6
#define	PWR_ASTAROTH_S_SHARDS		 7
#define	PWR_ICY_GLARE				 8
#define	PWR_BALAM_S_ANOINTING		 9
#define	PWR_BLOOD_MERCENARY			10
#define	PWR_SOW_DISCORD				11
#define	PWR_GIFT_OF_HEALING			12
#define	PWR_GIFT_OF_HEALTH			13
#define	PWR_THROW_WEBBING			14
#define	PWR_THOUGHT_TRAVEL			15
#define	PWR_DREAD_OF_DANTALION		16
#define	PWR_EARTH_SWALLOW			17
#define	PWR_ECHIDNA_S_VENOM			18
#define	PWR_SUCKLE_MONSTER			19
#define	PWR_PURIFYING_BLAST			20
#define	PWR_RECALL_TO_EDEN			21
#define	PWR_STARGATE				22
#define	PWR_WALKER_OF_THRESHOLDS	23
#define	PWR_GEYSER					24
#define	PWR_VENGANCE				25
#define	PWR_SHAPE_THE_WIND			26
#define	PWR_THORNS_AND_STONES		27
#define	PWR_BARAGE					28
#define	PWR_BREATH_POISON			29
#define	PWR_RUINOUS_STRIKE			30
#define	PWR_RAVEN_S_TALONS			31
#define	PWR_HORRID_WILTING			32
#define	PWR_TURN_ANIMALS_AND_HUMANOIDS	33
#define	PWR_REFILL_LANTERN			34
#define	PWR_HELLFIRE				35
#define	PWR_CALL_MURDER				36
#define	PWR_ROOT_SHOUT				37
#define	PWR_PULL_WIRES				38
#define	PWR_DISGUSTED_GAZE			39
#define	PWR_BLOODY_TOUNGE			40
#define	PWR_SILVER_TOUNGE			41
#define	PWR_EXHALATION_OF_THE_RIFT	42
#define	PWR_QUERIENT_THOUGHTS		43
#define	PWR_GREAT_LEAP				44
#define	PWR_MASTER_OF_DOORWAYS		45
#define	PWR_READ_SPELL				46
#define	PWR_BOOK_TELEPATHY			47
#define	PWR_UNITE_THE_EARTH_AND_SKY	48
#define	PWR_HOOK_IN_THE_SKY			49
#define	PWR_ENLIGHTENMENT			50
#define	PWR_DAMNING_DARKNESS		51
#define	PWR_TOUCH_OF_THE_VOID		52
#define	PWR_ECHOS_OF_THE_LAST_WORD	53
#define	PWR_POISON_GAZE				54
#define	PWR_GAP_STEP				55
#define	PWR_MOAN					56
#define	PWR_SWALLOW_SOUL			57
#define	PWR_EMBASSY_OF_ELEMENTS		58
#define	PWR_SUMMON_MONSTER			59
#define	PWR_MIRROR_SHATTER			60
#define	PWR_FLOWING_FORMS			61
#define	PWR_PHASE_STEP				62
#define	PWR_BLACK_BOLT				63
#define	PWR_WEAVE_BLACK_WEB			64
#define	PWR_IDENTIFY_INVENTORY		65
#define	PWR_CLAIRVOYANCE			66
#define	PWR_FIND_PATH				67
#define	PWR_GNOSIS_PREMONITION		68
#define	NUMBER_POWERS				69

	int spiritPOrder[52]; //# of letters in alphabet, capital and lowercase
//	char spiritPLetters[NUMBER_POWERS];
	long spiritPColdowns[NUMBER_POWERS];
	
	
	/* 	variable that keeps track of summoning in your vicinity.
		Only allow 1 per turn, to help reduce summoning cascades. */
	boolean summonMonster;
	/* 	Variable that checks if the Wizard has increased the weight of the amulet */
	boolean uleadamulet;
	/*Ugly extra artifact variables workaround.  Spaghetti code alert!*/
	long SnSd1, SnSd2, SnSd3, SnSd3duration;
	int ZangetsuSafe;
	int voidChime;
	short RoSPkills, RoSPflights;
	/*Keter counters*/
	int keter, chokhmah, binah, gevurah, hod, daat, netzach;
	int regifted; /*keeps track of how many artifacts the player has given to the unknown god*/
};	/* end of `struct you' */
#define uclockwork ((Race_if(PM_CLOCKWORK_AUTOMATON) && !Upolyd) || (Upolyd && youmonst.data == &mons[PM_CLOCKWORK_AUTOMATON]))
#define BASE_ATTACK_BONUS	((Role_if(PM_BARBARIAN) || Role_if(PM_CONVICT) || Role_if(PM_KNIGHT) || (!uwep && Role_if(PM_MONK)) || Role_if(PM_PIRATE) || Role_if(PM_SAMURAI) || Role_if(PM_VALKYRIE)) ? 1.00 :\
							 (Role_if(PM_ARCHEOLOGIST) || Role_if(PM_EXILE) || Role_if(PM_CAVEMAN) || Role_if(PM_MONK) || Role_if(PM_NOBLEMAN) || Role_if(PM_PRIEST) || Role_if(PM_ROGUE) || Role_if(PM_RANGER)) ? 0.75 :\
							 (Role_if(PM_HEALER) || Role_if(PM_TOURIST) || Role_if(PM_WIZARD)) ? 0.50:\
							  .5) /* Failsafe */

extern long sealKey[34]; /*Defined in u_init.c*/
extern boolean forcesight; /*Defined in u_init.c*/
extern char *wardDecode[26]; /*Defined in spell.c*/
extern int wardMax[18]; /*Defined in engrave.c*/
extern char *sealNames[]; /*Defined in engrave.c*/
extern char *sealTitles[]; /*Defined in engrave.c*/
extern char *andromaliusItems[18]; /*Defined in sounds.c*/
extern long int_spirits; /*Defined in sounds.c*/
extern long wis_spirits; /*Defined in sounds.c*/
extern boolean barage; /*Defined in dothrow.c*/
#define Upolyd (u.umonnum != u.umonster)

#endif	/* YOU_H */
