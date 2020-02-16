/*	SCCS Id: @(#)prop.h	3.4	1999/07/07	*/
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef PROP_H
#define PROP_H

/*** What the properties are ***/
#define NO_PROP						0
#define FIRE_RES					1 + NO_PROP
#define COLD_RES					1 + FIRE_RES
#define SLEEP_RES					1 + COLD_RES
#define DISINT_RES					1 + SLEEP_RES
#define SHOCK_RES					1 + DISINT_RES
#define POISON_RES					1 + SHOCK_RES
#define ACID_RES					1 + POISON_RES
#define STONE_RES					1 + ACID_RES
#define DRAIN_RES					1 + STONE_RES
#define SICK_RES					1 + DRAIN_RES
#define ANTIMAGIC					1 + SICK_RES
#define REFLECTING					1 + ANTIMAGIC
/* note: for the first 12 properties, MR_xxx == (1 << (xxx_RES - 1)) */
#define DISPLACED					1 + REFLECTING
#define SEARCHING					1 + DISPLACED
#define SEE_INVIS					1 + SEARCHING					
#define INVIS						1 + SEE_INVIS					
#define TELEPORT_CONTROL			1 + INVIS						
#define TELEPORT					1 + TELEPORT_CONTROL			
#define POLYMORPH					1 + TELEPORT					
#define POLYMORPH_CONTROL			1 + POLYMORPH					
#define LEVITATION					1 + POLYMORPH_CONTROL			
#define STEALTH						1 + LEVITATION					
#define AGGRAVATE_MONSTER			1 + STEALTH						
#define CONFLICT					1 + AGGRAVATE_MONSTER			
#define PROTECTION					1 + CONFLICT					
#define PROT_FROM_SHAPE_CHANGERS	1 + PROTECTION					
#define WARNING						1 + PROT_FROM_SHAPE_CHANGERS	
#define TELEPAT						1 + WARNING						
#define FAST						1 + TELEPAT						
#define STUNNED						1 + FAST						
#define CONFUSION					1 + STUNNED						
#define SICK						1 + CONFUSION					
#define BLINDED						1 + SICK						
#define SLEEPING					1 + BLINDED						
#define WOUNDED_LEGS				1 + SLEEPING					
#define STONED						1 + WOUNDED_LEGS				
#define GOLDED						1 + STONED						
#define STRANGLED					1 + GOLDED						
#define FROZEN_AIR					1 + STRANGLED						
#define HALLUC						1 + FROZEN_AIR					
#define HALLUC_RES					1 + HALLUC						
#define FUMBLING					1 + HALLUC_RES					
#define JUMPING						1 + FUMBLING					
#define WWALKING					1 + JUMPING						
#define HUNGER						1 + WWALKING					
#define GLIB						1 + HUNGER						
#define LIFESAVED					1 + GLIB						
#define CLAIRVOYANT					1 + LIFESAVED					
#define VOMITING					1 + CLAIRVOYANT					
#define ENERGY_REGENERATION			1 + VOMITING					
#define MAGICAL_BREATHING			1 + ENERGY_REGENERATION			
#define HALF_SPDAM					1 + MAGICAL_BREATHING			
#define HALF_PHDAM					1 + HALF_SPDAM					
#define ADORNED						1 + HALF_PHDAM					
#define REGENERATION				1 + ADORNED						
#define WARN_UNDEAD					1 + REGENERATION				
#define INVULNERABLE				1 + WARN_UNDEAD					
#define FREE_ACTION					1 + INVULNERABLE				
#define SWIMMING					1 + FREE_ACTION					
#define SLIMED						1 + SWIMMING					
#define FIXED_ABIL					1 + SLIMED						
#define FLYING						1 + FIXED_ABIL					
#define UNCHANGING					1 + FLYING						
#define PASSES_WALLS				1 + UNCHANGING					
#define SLOW_DIGESTION				1 + PASSES_WALLS	
/* vision */
#define INFRAVISION					1 + SLOW_DIGESTION				
#define NORMALVISION				1 + INFRAVISION
#define LOWLIGHTSIGHT				1 + NORMALVISION
#define ELFSIGHT					1 + LOWLIGHTSIGHT
#define DARKSIGHT					1 + ELFSIGHT
#define CATSIGHT					1 + DARKSIGHT
#define EXTRAMISSION				1 + CATSIGHT
#define WARN_OF_MON					1 + EXTRAMISSION
#define DETECT_MONSTERS				1 + WARN_OF_MON
#define BLOODSENSE					1 + DETECT_MONSTERS
#define LIFESENSE					1 + BLOODSENSE
#define SENSEALL					1 + LIFESENSE
#define EARTHSENSE					1 + SENSEALL
#define ECHOLOCATION				1 + EARTHSENSE
#define SPELLBOOST					1 + ECHOLOCATION
#define NECROSPELLS					1 + SPELLBOOST
#define CARCAP						1 + NECROSPELLS
#define WELDPROOF					1 + CARCAP
#define NULLMAGIC					1 + WELDPROOF
#define STERILE						1 + NULLMAGIC
#define CHASTITY					1 + STERILE
#define CLEAVING					1 + CHASTITY
#define GOOD_HEALTH					1 + CLEAVING
#define RAPID_HEALING				1 + GOOD_HEALTH
#define DESTRUCTION					1 + RAPID_HEALING
#define PRESERVATION				1 + DESTRUCTION
#define QUICK_DRAW					1 + PRESERVATION
#define CLEAR_THOUGHTS				1 + QUICK_DRAW
#define LAST_PROP		(CLEAR_THOUGHTS)

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
#	define W_SKIN		I_SPECIAL	/* merged into skin */

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
