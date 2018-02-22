/*	SCCS Id: @(#)prop.h	3.4	1999/07/07	*/
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef PROP_H
#define PROP_H

/*** What the properties are ***/
#define FIRE_RES		 1
#define COLD_RES		 2
#define SLEEP_RES		 3
#define DISINT_RES		 4
#define SHOCK_RES		 5
#define POISON_RES		 6
#define ACID_RES		 7
#define STONE_RES		 8
#define DRAIN_RES		 9
#define SICK_RES		10
/* note: for the first ten properties, MR_xxx == (1 << (xxx_RES - 1)) */
#define SEARCHING		11
#define SEE_INVIS		12
#define INVIS			13
#define TELEPORT_CONTROL	14
#define TELEPORT		15
#define POLYMORPH		16
#define POLYMORPH_CONTROL	17
#define LEVITATION		18
#define STEALTH			19
#define AGGRAVATE_MONSTER	20
#define CONFLICT		21
#define PROTECTION		22
#define PROT_FROM_SHAPE_CHANGERS 23
#define WARNING			24
#define TELEPAT			25
#define FAST			26
#define STUNNED			27
#define CONFUSION		28
#define SICK			29
#define BLINDED			30
#define SLEEPING		31
#define WOUNDED_LEGS		32
#define STONED			33
#define GOLDED			34
#define STRANGLED		35
#define HALLUC			36
#define HALLUC_RES		37
#define FUMBLING		38
#define JUMPING			39
#define WWALKING		40
#define HUNGER			41
#define GLIB			42
#define REFLECTING		43
#define LIFESAVED		44
#define ANTIMAGIC		45
#define DISPLACED		46
#define CLAIRVOYANT		47
#define VOMITING		48
#define ENERGY_REGENERATION	49
#define MAGICAL_BREATHING	50
#define HALF_SPDAM		51
#define HALF_PHDAM		52
#define ADORNED			53
#define REGENERATION	54
#define WARN_UNDEAD		55
#define INVULNERABLE	56
#define FREE_ACTION		57
#define SWIMMING		58
#define SLIMED			59
#define FIXED_ABIL		60
#define FLYING			61
#define UNCHANGING		62
#define PASSES_WALLS	63
#define SLOW_DIGESTION	64
#define INFRAVISION		65
#define NORMALVISION	INFRAVISION+1
#define LOWLIGHTSIGHT	NORMALVISION+1
#define ELFSIGHT		LOWLIGHTSIGHT+1
#define DARKSIGHT		ELFSIGHT+1
#define CATSIGHT		DARKSIGHT+1
#define EXTRAMISSION	CATSIGHT+1
#define WARN_OF_MON		EXTRAMISSION+1
#define DETECT_MONSTERS	WARN_OF_MON+1
#define BLOODSENSE		DETECT_MONSTERS+1
#define LIFESENSE		BLOODSENSE+1
#define SENSEALL		LIFESENSE+1
#define EARTHSENSE		SENSEALL+1
#define ECHOLOCATION	EARTHSENSE+1
#define SPELLBOOST		ECHOLOCATION+1
#define NECROSPELLS		SPELLBOOST+1
#define CARCAP			NECROSPELLS+1
#define WELDPROOF		CARCAP+1
#define NULLMAGIC		WELDPROOF+1
#define LAST_PROP		(NULLMAGIC)

/*** Where the properties come from ***/
/* Definitions were moved here from obj.h and you.h */
struct prop {
	/*** Properties conveyed by objects ***/
	long extrinsic;
	/* Armor */
#	define W_ARM	    0x00000001L /* Body armor */
#	define W_ARMC	    0x00000002L /* Cloak */
#	define W_ARMH	    0x00000004L /* Helmet/hat */
#	define W_ARMS	    0x00000008L /* Shield */
#	define W_ARMG	    0x00000010L /* Gloves/gauntlets */
#	define W_ARMF	    0x00000020L /* Footwear */
#ifdef TOURIST
#	define W_ARMU	    0x00000040L /* Undershirt */
#	define W_ARMOR	     (W_ARM | W_ARMC | W_ARMH | W_ARMS | W_ARMG | W_ARMF | W_ARMU)
#else
#	define W_ARMOR	     (W_ARM | W_ARMC | W_ARMH | W_ARMS | W_ARMG | W_ARMF)
#endif
	/* Weapons and artifacts */
#	define W_WEP	    0x00000100L /* Wielded weapon */
#	define W_QUIVER     0x00000200L /* Quiver for (f)iring ammo */
#	define W_SWAPWEP    0x00000400L /* Secondary weapon */
#	define W_ART	    0x00001000L /* Carrying artifact (not really worn) */
#	define W_ARTI	    0x00002000L /* Invoked artifact  (not really worn) */
	/* Amulets, rings, tools, and other items */
#	define W_AMUL	    0x00010000L /* Amulet */
#	define W_RINGL	    0x00020000L /* Left ring */
#	define W_RINGR	    0x00040000L /* Right ring */
#	define W_RING	    (W_RINGL | W_RINGR)
#	define W_TOOL	    0x00080000L /* Eyewear */
#ifdef STEED
#	define W_SADDLE     0x00100000L	/* KMH -- For riding monsters */
#endif
#	define W_BALL	    0x00200000L /* Punishment ball */
#	define W_CHAIN	    0x00400000L /* Punishment chain */

	/*** Property is blocked by an object ***/
	long blocked;					/* Same assignments as extrinsic */

	/*** Timeouts, permanent properties, and other flags ***/
	long intrinsic;
	/* Timed properties */
#	define TIMEOUT	    0x00ffffffL /* Up to 16 million turns */
	/* Permanent properties */
#	define FROMEXPER    0x01000000L /* Gain/lose with experience, for role */
#	define FROMRACE     0x02000000L /* Gain/lose with experience, for race */
#	define FROMOUTSIDE  0x04000000L /* By prayer, thrones, etc. */
#	define INTRINSIC    (FROMOUTSIDE|FROMRACE|FROMEXPER)
	/* Control flags */
#	define I_SPECIAL    0x10000000L /* Property is controllable */
};

/*** Definitions for backwards compatibility ***/
#define LEFT_RING	W_RINGL
#define RIGHT_RING	W_RINGR
#define LEFT_SIDE	LEFT_RING
#define RIGHT_SIDE	RIGHT_RING
#define BOTH_SIDES	(LEFT_SIDE | RIGHT_SIDE)
#define WORN_ARMOR	W_ARM
#define WORN_CLOAK	W_ARMC
#define WORN_HELMET	W_ARMH
#define WORN_SHIELD	W_ARMS
#define WORN_GLOVES	W_ARMG
#define WORN_BOOTS	W_ARMF
#define WORN_AMUL	W_AMUL
#define WORN_BLINDF	W_TOOL
#ifdef TOURIST
#define WORN_SHIRT	W_ARMU
#endif

#endif /* PROP_H */
