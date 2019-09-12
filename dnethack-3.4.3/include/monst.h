/*	SCCS Id: @(#)monst.h	3.4	1999/01/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MONST_H
#define MONST_H

#include "prop.h"

/* The weapon_check flag is used two ways:
 * 1) When calling mon_wield_item, is 2-6 depending on what is desired.
 * 2) Between calls to mon_wield_item, is 0 or 1 depending on whether or not
 *    the weapon is known by the monster to be cursed (so it shouldn't bother
 *    trying for another weapon).
 * I originally planned to also use 0 if the monster already had its best
 * weapon, to avoid the overhead of a call to mon_wield_item, but it turns out
 * that there are enough situations which might make a monster change its
 * weapon that this is impractical.  --KAA
 */
# define NO_WEAPON_WANTED 0
# define NEED_WEAPON 1
# define NEED_RANGED_WEAPON 2
# define NEED_HTH_WEAPON 3
# define NEED_PICK_AXE 4
# define NEED_AXE 5
# define NEED_PICK_OR_AXE 6

/* The following flags are used for the second argument to display_minventory
 * in invent.c:
 *
 * MINV_NOLET  If set, don't display inventory letters on monster's inventory.
 * MINV_ALL    If set, display all items in monster's inventory, otherwise
 *	       just display wielded weapons and worn items.
 */
#define MINV_NOLET 0x01
#define MINV_ALL   0x02

#ifndef ALIGN_H
#include "align.h"
#endif

#define toostrong(monindx, lev) (monstr[monindx] > lev)
#define tooweak(monindx, lev)	(monstr[monindx] < lev)

struct monst {
	struct monst *nmon;
	struct permonst *data;
	unsigned m_id;
	int mnum;		/* permanent monster index number */
	short movement;		/* movement points (derived from permonst definition and added effects */
	uchar m_lev;		/* adjusted difficulty level of monster */
	aligntyp malign;	/* alignment of this monster, relative to the
				   player (positive = good to kill) */
	xchar mx, my;
	xchar mux, muy;		/* where the monster thinks you are */
#define MTSZ	4
#define no_upos(mon)	((mon)->mux == 0 && (mon)->muy == 0)
	coord mtrack[MTSZ];	/* monster track */
	int mhp, mhpmax;
	unsigned mappearance;	/* for undetected mimics and the wiz */
	uchar	 m_ap_type;	/* what mappearance is describing: */
#define M_AP_NOTHING	0	/* mappearance is unused -- monster appears
				   as itself */
#define M_AP_FURNITURE	1	/* stairs, a door, an altar, etc. */
#define M_AP_OBJECT	2	/* an object */
#define M_AP_MONSTER	3	/* a monster */

	int mtame;		/* level of tameness, implies peaceful */
	int mpeacetime;		/* level of peacefulness, implies peaceful, ignored if tame */
	unsigned long int mintrinsics[LAST_PROP/32+1];
	unsigned long int mextrinsics[LAST_PROP/32+1];
	int mspec_used;		/* monster's special ability attack timeout */
	int mstdy;		/* to record extra damage to be delt due to having been studied */
	int ustdym;		/* to record extra damage to be delt by you due to having been studied by you*/
	int m_san_level;/* to record sanity level at which this monster will be obscured */
	
	short mstr, mdex, mcon, mint, mwis, mcha; /* Monster ability scores */

	Bitfield(mferal,1);	/* was once tame */ /*1*/
	
	Bitfield(female,1);	/* is female */ /*2*/
	Bitfield(minvis,1);	/* currently invisible */ /*3*/
	Bitfield(invis_blkd,1); /* invisibility blocked */ /*4*/
	Bitfield(perminvis,1);	/* intrinsic minvis value */ /*5*/
	Bitfield(cham,4);	/* shape-changer */ /*9*/
/* note: lychanthropes are handled elsewhere */
#define CHAM_ORDINARY		0	/* not a shapechanger */
#define CHAM_CHAMELEON		1	/* animal */
#define CHAM_DOPPELGANGER	2	/* demi-human */
#define CHAM_SANDESTIN		3	/* demon */
#define CHAM_DREAM			4	/* anything */
#define CHAM_MAX_INDX		CHAM_DREAM
	Bitfield(mundetected,1);	/* not seen in present hiding place */ /*10*/
				/* implies one of MT_CONCEAL or MT_HIDE,
				 * but not mimic (that is, snake, spider,
				 * trapper, piercer, eel)
				 */

	Bitfield(mcan,1);	/* has been cancelled */ /*11*/
	Bitfield(mburied,1);	/* has been buried */ /*12*/
	Bitfield(mspeed,2);	/* current speed */ /*14*/
	Bitfield(permspeed,2);	/* intrinsic mspeed value */ /*16*/
	Bitfield(mrevived,1);	/* has been revived from the dead */ /*17*/
	Bitfield(mavenge,1);	/* did something to deserve retaliation */ /*18*/
	
	Bitfield(mflee,1);	/* fleeing */ /*19*/
	Bitfield(mfleetim,7);	/* timeout for mflee */ /*26*/
	
	Bitfield(mcrazed,1);	/* monster is crazed */ /*27*/
	
	Bitfield(mberserk,1);	/* monster is crazed */ /*28*/
	
	Bitfield(mcansee,1);	/* cansee 1, temp.blinded 0, blind 0 */ /*29*/
	Bitfield(mblinded,7);	/* cansee 0, temp.blinded n, blind 0 */ /*36*/

	Bitfield(mcanhear,1);	/* cansee 1, temp.blinded 0, blind 0 */ /*37*/
	Bitfield(mdeafened,7);	/* cansee 0, temp.blinded n, blind 0 */ /*44*/

	Bitfield(mcanmove,1);	/* paralysis, similar to mblinded */ /*45*/
	Bitfield(mfrozen,7); /*52*/
	
	Bitfield(mnotlaugh,1);	/* collapsed from laughter, similar to mblinded */ /*53*/
	Bitfield(mlaughing,7); /*60*/
	
	Bitfield(msleeping,1);	/* asleep until woken */ /*61*/
	Bitfield(mstun,1);	/* stunned (off balance) */ /*62*/
	Bitfield(mconf,1);	/* confused */ /*63*/
	Bitfield(mpeaceful,1);	/* does not attack unprovoked */ /*64*/
	Bitfield(mtrapped,1);	/* trapped in a pit, web or bear trap */ /*65*/
	Bitfield(mleashed,1);	/* monster is on a leash */ /*66*/
	Bitfield(mtraitor,1);	/* Former pet that turned traitor */ /*67*/
	Bitfield(isshk,1);	/* is shopkeeper */ /*68*/
	Bitfield(isminion,1);	/* is a minion */ /*69*/
	
	Bitfield(isgd,1);	/* is guard */ /*70*/
	Bitfield(ispriest,1);	/* is a priest */ /*71*/
	Bitfield(iswiz,1);	/* is the Wizard of Yendor */ /*72*/
	Bitfield(wormno,5);	/* at most 31 worms on any level */ /*77*/
#define MAX_NUM_WORMS	32	/* should be 2^(wormno bitfield size) */
	Bitfield(moccupation,1);/* linked to your occupation */ /*78*/
	Bitfield(mclone,1);		/* is a clone of another monster (don't make death drop) */ /*79*/
	Bitfield(uhurtm,1);		/* injured by you */ /*80*/
	Bitfield(mhurtu,1);		/* has injured you */ /*81*/
	Bitfield(mattackedu,1);	/* attacked you on it's last turn */ /*82*/
	Bitfield(housealert,1);	/* won't accept house-based pacification */ /*83*/
	Bitfield(mspiritual,1);	/* Created by spirit power, doesn't count towards pet limit */ /*84*/
	Bitfield(artnum,2);		/* Created from an artifact. */ /*86*/
#define ART_PYGMALION	1
#define ART_GALATEA		2
	Bitfield(notame,1);/* can't be tamed */ /*87*/
	Bitfield(zombify,1);/* should rise as zombie */ /*88*/
	long mwait;/* if tame, won't follow between levels (turn last ordered to wait on) */
	int encouraged;	/* affected by Encourage song */
#define BASE_DOG_ENCOURAGED_MAX		7
	
	int entangled;/* The monster is entangled, and in what? */
#define noactions(mon)	((mon)->entangled || ((mon)->mtrapped && t_at((mon)->mx, (mon)->my) && t_at((mon)->mx, (mon)->my)->ttyp == VIVI_TRAP))
#define helpless(mon) (mon->msleeping || !(mon->mcanmove) || !(mon->mnotlaugh) || noactions(mon))	
	long mstrategy;		/* for monsters with mflag3: current strategy */
#define STRAT_ARRIVE	0x40000000L	/* just arrived on current level */
#define STRAT_WAITFORU	0x20000000L
#define STRAT_CLOSE	0x10000000L
#define STRAT_WAITMASK	0x30000000L
#define STRAT_HEAL	0x08000000L
#define STRAT_GROUND	0x04000000L
#define STRAT_MONSTR	0x02000000L
#define STRAT_PLAYER	0x01000000L
#define STRAT_NONE	0x00000000L
#define STRAT_STRATMASK 0x0f000000L
#define STRAT_XMASK	0x00ff0000L
#define STRAT_YMASK	0x0000ff00L
#define STRAT_GOAL	0x000000ffL
#define STRAT_GOALX(s)	((xchar)((s & STRAT_XMASK) >> 16))
#define STRAT_GOALY(s)	((xchar)((s & STRAT_YMASK) >> 8))

	long mtrapseen;		/* bitmap of traps we've been trapped in */
	long mlstmv;		/* for catching up with lost time */
#ifndef GOLDOBJ
	long mgold;
#endif
	struct obj *minvent;

	struct obj *mw;
	struct obj *msw;
	long misc_worn_check;
	xchar weapon_check;
	xchar combat_mode;
#define	RANGED_MODE	0
#define HNDHND_MODE	1

	uchar mnamelth;		/* length of name (following mxlth) */
	short mxlth;		/* length of following data */
	int meating;		/* monster is eating timeout */
	int mfaction;		/* faction to which monster belongs */
/*Note: first drow house is 8*/
#define	FACTION_PADDING	1000
#define	ZOMBIFIED	FACTION_PADDING+1	/* zombie 'faction' */
#define	SKELIFIED	FACTION_PADDING+2	/* skeleton 'faction' */
#define	CRYSTALFIED	FACTION_PADDING+3	/* crystal dead 'faction' */
#define	FRACTURED	FACTION_PADDING+4	/* witness of the fracture 'faction' */
#define	VAMPIRIC	FACTION_PADDING+5	/* vampirified 'faction' */
#define	ILLUMINATED	FACTION_PADDING+6	/* illuminated 'faction' */
#define	INCUBUS_FACTION	FACTION_PADDING+7	/* male foocubus */
#define	SUCCUBUS_FACTION	FACTION_PADDING+8	/* female foocubus */
#define	PSEUDONATURAL	FACTION_PADDING+9	/* tentacled */
#define	TOMB_HERD	FACTION_PADDING+10	/* possessed statue */
#define	YITH		FACTION_PADDING+11	/* possessed by the great race of yith */
#define	CRANIUM_RAT	FACTION_PADDING+12	/* cranium rat */
//define	HALF_DEMON	FACTION_PADDING+1	/* half-demon */
//define	HALF_DEVIL	FACTION_PADDING+2	/* half-devil */
//define	HALF_DRAGON	FACTION_PADDING+3	/* half-dragon */
//define	LAMASHTU_FACTION	FACTION_PADDING+6	/* Lamashtu's faction */
	int mvanishes;
	//hatching situation
	//AT_LAYEGG, PM_HATCHLING_NAME, x, d
	//timetohatch
	//larvatime (larva effects default or by hatchling, time default or by hatchling)
	//	time:  AT_LARVA+x, where x=time, and AT_LARVA is large, 10000 or so.
	//utype = PM_HATCHLING_NAME
	//inventory
	long mvar1;
	long mvar2;
	long mvar3;
	/* in order to prevent alignment problems mextra should
	   be (or follow) a long int */
	long mextra[1]; /* monster dependent info */
};

/*
 * Note that mextra[] may correspond to any of a number of structures, which
 * are indicated by some of the other fields.
 *	isgd	 ->	struct egd
 *	ispriest ->	struct epri
 *	isshk	 ->	struct eshk
 *	isminion ->	struct emin
 *			(struct epri for roaming priests and angels, which is
 *			 compatible with emin for polymorph purposes)
 *	mtame	 ->	struct edog
 *			(struct epri for guardian angels, which do not eat
 *			 or do other doggy things)
 * Since at most one structure can be indicated in this manner, it is not
 * possible to tame any creatures using the other structures (the only
 * exception being the guardian angels which are tame on creation).
 */

#define newmonst(xl) (struct monst *)alloc((unsigned)(xl) + sizeof(struct monst))
#define dealloc_monst(mon) free((genericptr_t)(mon))

/* these are in mspeed */
#define MSLOW 1		/* slow monster */
#define MFAST 2		/* speeded monster */

#define NAME(mtmp)	(((char *)(mtmp)->mextra) + (mtmp)->mxlth)

#define MON_WEP(mon)	((mon)->mw)
#define MON_NOWEP(mon)	((mon)->mw = (struct obj *)0)
#define MON_SWEP(mon)	((mon)->msw)
#define MON_NOSWEP(mon)	((mon)->msw = (struct obj *)0)

#define DEADMONSTER(mon)	((mon)->mhp < 1)

#endif /* MONST_H */
