/*	SCCS Id: @(#)artifact.h 3.4	1995/05/31	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ARTIFACT_H
#define ARTIFACT_H

#define SPFX_NONE   0x00000000L	/* no special effects, just a bonus */
#define SPFX_NOGEN  0x00000001L	/* item is special, bequeathed by gods */
#define SPFX_RESTR  0x00000002L	/* item is restricted - can't be named */
#define SPFX_INTEL  0x00000004L	/* item is self-willed - intelligent */
#define SPFX_SPEAK  0x00000008L	/* item can speak (not implemented) */
#define SPFX_SEEK   0x00000010L	/* item helps you search for things, ie, adds enhancement bonus to attempts */
#define SPFX_WARN   0x00000020L	/* item warns you of danger */
#define SPFX_ATTK   0x00000040L	/* item has a special attack (attk) */
#define SPFX_DEFN   0x00000080L	/* item has a special defence (defn) */
#define SPFX_DRLI   0x00000100L	/* drains a level from monsters */
#define SPFX_SEARCH 0x00000200L	/* automatic searching */
#define SPFX_BEHEAD 0x00000400L	/* beheads monsters */
#define SPFX_HALRES 0x00000800L	/* blocks hallucinations */
#define SPFX_ESP    0x00001000L	/* ESP (like amulet of ESP) */
#define SPFX_DISPL  0x00002000L	/* Displacement */
#define SPFX_REGEN  0x00004000L	/* Regeneration */
#define SPFX_EREGEN 0x00008000L	/* Energy Regeneration */
#define SPFX_HSPDAM 0x00010000L	/* 1/2 spell damage (on player) in combat */
#define SPFX_HPHDAM 0x00020000L	/* 1/2 physical damage (on player) in combat */
#define SPFX_TCTRL  0x00040000L	/* Teleportation Control */
#define SPFX_LUCK   0x00080000L	/* Increase Luck (like Luckstone) */
#define SPFX_CON_AND 0x00100000L	/* Affects only those monsters with ALL flags specified */
#define SPFX_CON_OR 0x00200000L	/* Affects any monster with ANY flag specified */
#define SPFX_DALIGN 0x00400000L	/* attack bonus on non-aligned monsters  */
#define SPFX_DBONUS (SPFX_DALIGN|SPFX_CON_OR|SPFX_CON_AND)	/* attack bonus mask */
#define SPFX_XRAY   0x00800000L	/* gives X-RAY vision to player */
#define SPFX_REFLECT 0x01000000L /* Reflection */
#define SPFX_CONFL  0x02000000L  /* Conflict */
#define SPFX_AGGRM  0x04000000L /* Aggravate monster */
#define SPFX_INHER  0x08000000L /* Can be Inherited */
/* expanded special effects*/
#define SPFX2_NONE		0x0000000L	/* nothing special */
#define SPFX2_DIG		0x0000001L	/* Can dig with artifact, as if it were a pick. Clarent Patch (GreyKnight)*/

#define SPFX2_BLIND		0x0000004L	/* causes blindness */

#define SPFX2_FIRE		0x0000010L	/* Fireball strike (1/4) */
#define SPFX2_COLD		0x0000020L	/* Cold blast strike (1/4 */
#define SPFX2_ELEC		0x0000040L	/* Lightning bolt strike (1/4) */
#define SPFX2_RAM		0x0000080L	/* Causes knockback (1/4) */
#define SPFX2_FIRE2		0x0000100L	/* Fireball strike (always) */
#define SPFX2_COLD2		0x0000200L	/* Cold blast strike (always) */
#define SPFX2_ELEC2		0x0000400L	/* Lightning bolt strike (always) */
#define SPFX2_RAM2		0x0000800L	/* Always causes knockback */
#define SPFX2_NOCALL	0x0001000L	/* Blocks Demon-minion-summoning when wielded */
#define SPFX2_NOWERE	0x0002000L	/* Protects agains lycathropy when wielded */
#define SPFX2_POISONED	0x0004000L	/* permanently poisoned weapon */

#define SPFX2_BLDTHRST	0x0010000L	/* bloodthirsty weapon (primary wielded only) */
#define SPFX2_BRIGHT	0x0020000L	/* turns trolls to stone, dusts gremlins */
#define SPFX2_STEAL		0x0040000L  /* steals stuff */
#define SPFX2_DANCER	0x0080000L	/* extra movement points for attacking */
#define SPFX2_SILVERED	0x0100000L	/* weapon is silvered  */
#define SPFX2_STLTH		0x0200000L	/* gives stealth when wielded or worn */
#define SPFX2_SHATTER	0x0400000L	/* breaks opponents weapons */
#define SPFX2_DISARM	0x0800000L	/* disarms opponents */
#define SPFX2_SPELLUP	0x1000000L	/* increases spell power */
#define SPFX2_TENTROD	0x2000000L	/* various effects */
#define SPFX2_SHINING	0x4000000L	/* passes through armor */
#define SPFX2_THREEHEAD	0x8000000L	/* 2 extra attacks plus stunning if all three hit */

#define SPFX3_ENGRV		0x0000001L	/* Allows fast engraving (as for Athame) */
#define SPFX3_PCTRL		0x0000002L	/* grants polymorph control (caried or wielded) */
#define SPFX3_FEAR		0x0000004L  /* as dropped scroll of scare monster */
#define SPFX3_NOCNT		0x0000008L  /* doesn't count towards  created artifact total */
#define SPFX3_MANDALA	0x0000010L	/* (r)ead object to benifit as from unihorn ONLY CHECKED FOR SHIRTS */
#define SPFX3_LIGHT		0x0000040L	/* always lit lightsource */

#define	WSFX_LIGHTEN	0x0000001L	/* when worn, increases the weight you can haul. Currently only includes
										Shirts, Cloaks, and body armor.  See do_wear.c and hack.c */
#define	WSFX_WCATRIB	0x0000002L	/* when worn above armor, increases Cha.  Under armor, increases Wis. */
#define	WSFX_PLUSSEV	0x0000004L	/* Enchant to +7 max. */
#define	WSFX_FREEACT	0x0000008L	/* Grant free action when worn. */

///////////////////
//To be implemented
#define SPFX2_NINJA		0x0000008L	/* throws from 1-your skill level ninja stars after each attack */
#define SPFX3_CARCAP	0x0000020L	/* increases carrying capacity when carried */


/*Necronomicon flags*/
//	-Summon 
#define	S_BYAKHEE		0x00000001L
#define	S_NIGHTGAUNT	0x00000002L
#define	S_SHOGGOTH		0x00000004L
#define	S_OOZE			0x00000008L
//	-Spells
#define	SP_PROTECTION	0x00000010L
#define	SP_TURN_UNDEAD	0x00000020L

#define	SP_FORCE_BOLT	0x00000040L
#define	SP_DRAIN_LIFE	0x00000080L
#define	SP_DEATH		0x00000100L

#define	SP_DETECT_MNSTR	0x00000200L
#define	SP_CLAIRVOYANCE	0x00000400L
#define	SP_DETECT_UNSN	0x00000800L
#define	SP_IDENTIFY		0x00001000L

#define	SP_CONFUSE		0x00002000L
#define	SP_CAUSE_FEAR	0x00004000L

#define	SP_LEVITATION	0x00008000L

#define	SP_STONE_FLESH	0x00010000L

#define	SP_CANCELLATION	0x00020000L

#define	SP_COMBAT		0x00040000L
#define	SP_HEALTH		0x00080000L

#define	SUM_DEMON		0x00100000L
#define	S_DEVIL			0x00200000L

#define	R_Y_SIGN		0x00400000L	//yellow sign
#define	R_WARDS			0x00800000L	//eye, queen
#define	R_ELEMENTS		0x01000000L	//Cthuqha, Ithaqua, and Karakal
#define	R_NAMES_1		0x02000000L	//Half of the spirits (based on random order)
#define	R_NAMES_2		0x04000000L	//Other half of the spirits


#define	LAST_PAGE		0x40000000L

//	-Summon 
#define	SELECT_BYAKHEE		1
#define	SELECT_NIGHTGAUNT	SELECT_BYAKHEE + 1
#define	SELECT_SHOGGOTH		SELECT_NIGHTGAUNT + 1
#define	SELECT_OOZE			SELECT_SHOGGOTH + 1
#define	SELECT_DEMON		SELECT_OOZE + 1
#define	SELECT_DEVIL		SELECT_DEMON + 1
//	-Spells
#define	SELECTED_SPELL		SELECT_DEVIL + 1
#define	SELECT_PROTECTION	SELECTED_SPELL
#define	SELECT_TURN_UNDEAD	SELECT_PROTECTION + 1

#define	SELECT_FORCE_BOLT	SELECT_TURN_UNDEAD + 1
#define	SELECT_DRAIN_LIFE	SELECT_FORCE_BOLT + 1
#define	SELECT_DEATH		SELECT_DRAIN_LIFE + 1

#define	SELECT_DETECT_MNSTR	SELECT_DEATH + 1
#define	SELECT_CLAIRVOYANCE	SELECT_DETECT_MNSTR + 1
#define	SELECT_DETECT_UNSN	SELECT_CLAIRVOYANCE + 1
#define	SELECT_IDENTIFY		SELECT_DETECT_UNSN + 1

#define	SELECT_CONFUSE		SELECT_IDENTIFY + 1
#define	SELECT_CAUSE_FEAR	SELECT_CONFUSE + 1

#define	SELECT_LEVITATION	SELECT_CAUSE_FEAR + 1

#define	SELECT_STONE_FLESH	SELECT_LEVITATION + 1

#define	SELECT_CANCELLATION	SELECT_STONE_FLESH + 1

#define	SELECTED_SPECIAL	SELECT_CANCELLATION + 1
#define	SELECT_COMBAT		SELECTED_SPECIAL
#define	SELECT_HEALTH		SELECT_COMBAT + 1
#define	SELECT_SIGN			SELECT_HEALTH + 1
#define	SELECT_WARDS		SELECT_SIGN + 1
#define	SELECT_ELEMENTS		SELECT_WARDS + 1
#define	SELECT_SPIRITS1		SELECT_ELEMENTS + 1
#define	SELECT_SPIRITS2		SELECT_SPIRITS1 + 1


#define SELECT_STUDY		SELECT_SPIRITS2 + 1

#define SELECT_WHISTLE		1
#define SELECT_LEASH		SELECT_WHISTLE + 1
#define SELECT_SADDLE		SELECT_LEASH + 1
#define SELECT_TRIPE		SELECT_SADDLE + 1
#define SELECT_APPLE		SELECT_TRIPE + 1
#define SELECT_BANANA		SELECT_APPLE + 1

#define COMMAND_RAPIER			 1
#define COMMAND_AXE				 2
#define COMMAND_MACE			 3
#define COMMAND_SPEAR			 4
#define COMMAND_LANCE			 5

#define COMMAND_D_GREAT			 6
#define COMMAND_MOON_AXE		 7
#define COMMAND_KHAKKHARA		 8
#define COMMAND_DROVEN_SPEAR	 9
#define COMMAND_D_LANCE			10

#define COMMAND_E_SWORD			11
#define COMMAND_E_SICKLE		12
#define COMMAND_E_MACE			13
#define COMMAND_E_SPEAR			14
#define COMMAND_E_LANCE			15

#define COMMAND_SCIMITAR		16
#define COMMAND_WHIP			17

#define COMMAND_ATLATL			18

#define COMMAND_SABER			19
#define COMMAND_RING			20
#define COMMAND_ARM				21
#define COMMAND_RAY				22
#define COMMAND_BFG				23
#define COMMAND_ANNULUS			24

#define COMMAND_LADDER			25
#define COMMAND_CLAIRVOYANCE	26
#define COMMAND_FEAR			27
#define COMMAND_LIFE			28
#define COMMAND_KNEEL			29

#define COMMAND_STRIKE			30

#define COMMAND_AMMO			31

#define COMMAND_BELL			32
#define COMMAND_BULLETS			33
#define COMMAND_ROCKETS			34
#define COMMAND_BEAM			35
#define COMMAND_ANNUL			36
#define COMMAND_CHARGE			37

#define COMMAND_POISON			38
#define COMMAND_DRUG  			39
#define COMMAND_STAIN 			40
#define COMMAND_ENVENOM			41
#define COMMAND_FILTH 			42

#define COMMAND_IMPROVE_WEP     43
#define COMMAND_IMPROVE_ARM     44

#define COMMAND_DEATH           45

#define COMMAND_GRAY            46
#define COMMAND_SILVER          47
#define COMMAND_MERCURIAL       48
#define COMMAND_SHIMMERING      49
#define COMMAND_DEEP            50
#define COMMAND_RED             51
#define COMMAND_WHITE           52
#define COMMAND_ORANGE          53
#define COMMAND_BLACK           54
#define COMMAND_BLUE            55
#define COMMAND_GREEN           56
#define COMMAND_YELLOW          57

struct artifact {
	int	    otyp;
	const char  *name;
	int material;	/* default material */
	int size;		/* default size */
	unsigned long spfx;	/* special effect from wielding/wearing */
	unsigned long cspfx;	/* special effect just from carrying obj */
	unsigned long mtype;	/* monster symbol */
	unsigned long mflagsm;	/* Monster Motility boolean bitflags */
	unsigned long mflagst;	/* Monster Thoughts and behavior boolean bitflags */
	unsigned long mflagsb;	/* Monster Body plan boolean bitflags */
	unsigned long mflagsg;	/* Monster Game mechanics and bookkeeping boolean bitflags */
	unsigned long mflagsa;	/* Monster rAce boolean bitflags */
	unsigned long mflagsv;	/* Monster Vision boolean bitflags */
	struct attack attk, defn, cary;
	uchar	    inv_prop;	/* property obtained by invoking artifact */
	aligntyp    alignment;	/* alignment of bequeathing gods */
	short	    role;	/* character role associated with */
	short	    race;	/* character race associated with */
	long        cost;	/* price when sold to hero (default 100 x base cost) */
	unsigned long spfx2;	/* special effect from wielding/wearing (expanded list) */
	unsigned long cspfx3;	/* special effect just from carrying obj (expanded list)*/
	unsigned long wpfx;		/* special effect from wearing obj (expanded list)*/
};

/* invoked properties with special powers */
#define TAMING		(LAST_PROP+1)
#define HEALING		(LAST_PROP+2)
#define ENERGY_BOOST	(LAST_PROP+3)
#define UNTRAP		(LAST_PROP+4)
#define CHARGE_OBJ	(LAST_PROP+5)
#define LEV_TELE	(LAST_PROP+6)
#define CREATE_PORTAL	(LAST_PROP+7)
#define ENLIGHTENING	(LAST_PROP+8)
#define CREATE_AMMO	(LAST_PROP+9)
#define SMOKE_CLOUD    (LAST_PROP+10)
#define CANNONADE	(LAST_PROP+11)
#define MAGICALDEVICE	(LAST_PROP+12)
#define SHADOW_FLARE	(LAST_PROP+13)
#define BLIZAGA	(LAST_PROP+14)
#define FIRAGA	(LAST_PROP+15)
#define THUNDAGA	(LAST_PROP+16)
#define QUAKE	(LAST_PROP+17)
#define PETMASTER	(LAST_PROP+18)
#define LEADERSHIP	(LAST_PROP+19)//from GreyKnight's Clarent mod
#define BLESS	(LAST_PROP+20)
#define SEVENFOLD	(LAST_PROP+21)
#define SATURN	(LAST_PROP+22)
#define PLUTO	(LAST_PROP+23)
#define SPEED_BANKAI	(LAST_PROP+24)
#define ICE_SHIKAI	(LAST_PROP+25)
#define FIRE_SHIKAI	(LAST_PROP+26)
#define MANDALA	(LAST_PROP+27)
#define DRAIN_MEMORIES	(LAST_PROP+28)
#define SLAY_LIVING	(LAST_PROP+29)
#define NECRONOMICON	(LAST_PROP+30)
#define DANCE_DAGGER	(LAST_PROP+31)
#define SING_SPEAR	(LAST_PROP+32)
#define PHASING		(LAST_PROP+33)
#define OBJECT_DET	(LAST_PROP+34)
#define TELEPORT_SHOES	(LAST_PROP+35)
#define LORDLY	(LAST_PROP+36)
#define VOID_CHIME	(LAST_PROP+37)
#define INFINITESPELLS	(LAST_PROP+38)
#define SPIRITNAMES	(LAST_PROP+39)
#define RAISE_UNDEAD	(LAST_PROP+40)
#define FALLING_STARS	(LAST_PROP+41)
#define THEFT_TYPE		(LAST_PROP+42)
#define ANNUL			(LAST_PROP+43)
#define ARTI_REMOVE_CURSE	(LAST_PROP+44)
#define SUMMON_UNDEAD   (LAST_PROP+45)
#define DEATH_GAZE      (LAST_PROP+46)
#define SMITE           (LAST_PROP+47)
#define PROTECT         (LAST_PROP+48)
#define TRAP_DET        (LAST_PROP+49)
#define UNBIND_SEALS    (LAST_PROP+50)
#define HEAL_PETS       (LAST_PROP+51)
#define FREE_SPELL      (LAST_PROP+52)
#define BURN_WARD       (LAST_PROP+53)
#define FAST_TURNING    (LAST_PROP+54)
#define FIRE_BLAST      (LAST_PROP+55)
#define SELF_POISON     (LAST_PROP+56)
#define ADD_POISON      (LAST_PROP+57)
#define TOWEL_ITEMS     (LAST_PROP+58)
#define MAJ_RUMOR       (LAST_PROP+59)
#define ARTIFICE        (LAST_PROP+60)
#define SUMMON_PET      (LAST_PROP+61)
#define LIFE_DEATH      (LAST_PROP+62)
#define PRISMATIC       (LAST_PROP+63)
#define STEAL           (LAST_PROP+64)
#define SUMMON_VAMP     (LAST_PROP+65)
#define COLLECT_TAX     (LAST_PROP+66)
#define ALTMODE         (LAST_PROP+67)
#define AEGIS           (LAST_PROP+68)
#define WATER           (LAST_PROP+69)
#define SINGING         (LAST_PROP+70)
#define WIND_PETS		(LAST_PROP+71)
#define ORACLE          (LAST_PROP+72)


#define MASTERY_ARTIFACT_LEVEL 20

#define has_named_mastery_artifact() (\
            exist_artifact(BULLWHIP, artilist[ART_TRUSTY_ADVENTURER_S_WHIP].name) ||\
            exist_artifact(SPE_BLANK_PAPER, artilist[ART_LOG_OF_THE_CURATOR].name) ||\
            exist_artifact(FORCE_PIKE, artilist[ART_FORCE_PIKE_OF_THE_RED_GUAR].name) ||\
            exist_artifact(GAUNTLETS, artilist[ART_GAUNTLETS_OF_THE_BERSERKER].name) ||\
            exist_artifact(SCR_REMOVE_CURSE, artilist[ART_DECLARATION_OF_THE_APOSTAT].name) ||\
            exist_artifact(LENSES, artilist[ART_SOUL_LENS].name) ||\
            exist_artifact(SCR_BLANK_PAPER, artilist[ART_SEAL_OF_THE_SPIRITS].name) ||\
            exist_artifact(WAN_FIRE, artilist[ART_TORCH_OF_ORIGINS].name) ||\
            exist_artifact(STRIPED_SHIRT, artilist[ART_STRIPED_SHIRT_OF_THE_MURDE].name) ||\
            exist_artifact(STRIPED_SHIRT, artilist[ART_STRIPED_SHIRT_OF_THE_THIEF].name) ||\
            exist_artifact(STRIPED_SHIRT, artilist[ART_STRIPED_SHIRT_OF_THE_FALSE].name) ||\
            exist_artifact(SCALPEL, artilist[ART_SCALPEL_OF_LIFE_AND_DEATH].name) ||\
            exist_artifact(GAUNTLETS_OF_DEXTERITY, artilist[ART_GAUNTLETS_OF_THE_HEALING_H].name) ||\
            exist_artifact(RIN_REGENERATION, artilist[ART_RING_OF_HYGIENE_S_DISCIPLE].name) ||\
            exist_artifact(ROBE, artilist[ART_COPE_OF_THE_ELDRITCH_KNIGH].name) ||\
            exist_artifact(KITE_SHIELD, artilist[ART_SHIELD_OF_THE_PALADIN].name) ||\
            exist_artifact(POT_BOOZE, artilist[ART_BOOZE_OF_THE_DRUNKEN_MASTE].name) ||\
            exist_artifact(GLOVES, artilist[ART_WRAPPINGS_OF_THE_SACRED_FI].name) ||\
            exist_artifact(KHAKKHARA, artilist[ART_KHAKKHARA_OF_THE_MONKEY].name) ||\
            exist_artifact(RIN_TELEPORTATION, artilist[ART_MARK_OF_THE_RIGHTFUL_SCION].name) ||\
            exist_artifact(ORIHALCYON_GAUNTLETS, artilist[ART_GAUNTLETS_OF_THE_DIVINE_DI].name) ||\
            exist_artifact(MACE, artilist[ART_MACE_OF_THE_EVANGELIST].name) ||\
            exist_artifact(DART, artilist[ART_DART_OF_THE_ASSASSIN].name) ||\
            exist_artifact(SHORT_SWORD, artilist[ART_SWORD_OF_THE_KLEPTOMANIAC].name) ||\
            exist_artifact(LEATHER_HELM, artilist[ART_HELM_OF_THE_ARCANE_ARCHER].name) ||\
            exist_artifact(FIGURINE, artilist[ART_FIGURINE_OF_PYGMALION].name) ||\
            exist_artifact(FIGURINE, artilist[ART_FIGURINE_OF_GALATEA].name) ||\
            exist_artifact(HELM_OF_OPPOSITE_ALIGNMENT, artilist[ART_HELM_OF_THE_NINJA].name) ||\
            exist_artifact(TOWEL, artilist[ART_TOWEL_OF_THE_INTERSTELLAR_].name) ||\
            exist_artifact(SPE_BLANK_PAPER, artilist[ART_ENCYCLOPEDIA_GALACTICA].name) ||\
            exist_artifact(WAN_TELEPORTATION, artilist[ART_TWIG_OF_YGGDRASIL].name) ||\
            exist_artifact(SADDLE, artilist[ART_SADDLE_OF_BRYNHILDR].name) ||\
            exist_artifact(QUARTERSTAFF, artilist[ART_STAFF_OF_WILD_MAGIC].name) ||\
            exist_artifact(ROBE, artilist[ART_ROBE_OF_THE_ARCHMAGI].name) ||\
            exist_artifact(WAR_HAMMER, artilist[ART_FORGE_HAMMER_OF_THE_ARTIFI].name) ||\
            exist_artifact(RIN_PROTECTION_FROM_SHAPE_CHAN, artilist[ART_RING_OF_LOLTH].name) ||\
            exist_artifact(DWARVISH_ROUNDSHIELD, artilist[ART_BULWARK_OF_THE_DWARVEN_DEF].name) ||\
            exist_artifact(RIN_PROTECTION_FROM_SHAPE_CHAN,artilist[ART_RING_OF_LOLTH].name) ||\
            exist_artifact(RIN_TELEPORT_CONTROL,artilist[ART_NARYA].name) ||\
            exist_artifact(RIN_TELEPORTATION,artilist[ART_NENYA].name) ||\
            exist_artifact(RIN_AGGRAVATE_MONSTER,artilist[ART_VILYA].name) ||\
            exist_artifact(GNOMISH_POINTY_HAT,artilist[ART_HAT_OF_THE_GIANT_KILLER].name) ||\
            exist_artifact(PLATE_MAIL,artilist[ART_PRISMATIC_DRAGON_PLATE].name) ||\
            exist_artifact(STAR_SAPPHIRE,artilist[ART_FOOTPRINTS_IN_THE_LABYRINT].name) ||\
            exist_artifact(AMULET_OF_RESTFUL_SLEEP,artilist[ART_TRAPPINGS_OF_THE_GRAVE].name)\
)

#define is_nameable_artifact(a) (\
			(a->spfx & (SPFX_RESTR)) == 0\
			|| (a == &artilist[ART_SNICKERSNEE] && Role_if(PM_TOURIST) && (u.ulevel > 18 || u.uevent.qcompleted) )\
			|| (a == &artilist[ART_KUSANAGI_NO_TSURUGI] && Role_if(PM_SAMURAI) && u.ulevel >= 22 )\
			|| (a == &artilist[ART_GRANDMASTER_S_ROBE] && P_SKILL(P_MARTIAL_ARTS) >= P_GRAND_MASTER && u.ulevel >= 30 )\
			|| (a == &artilist[ART_FIRST_KEY_OF_CHAOS] && \
							(exist_artifact(SKELETON_KEY,artilist[ART_THIRD_KEY_OF_CHAOS].name) \
							||  u.ualign.type == A_CHAOTIC ) \
				)\
			)
/* artifact has no specific material or size, eg "silver Grimtooth" */
#define is_malleable_artifact(a) (is_nameable_artifact((a)) || (a) == &artilist[ART_EXCALIBUR])

#define is_mastery_artifact_nameable(a) (\
            /* Mastery artifacts */\
            /* Archeologist */\
			((a) == &artilist[ART_TRUSTY_ADVENTURER_S_WHIP] && (Role_if(PM_ARCHEOLOGIST) || Pantheon_if(PM_ARCHEOLOGIST)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_LOG_OF_THE_CURATOR] && (Role_if(PM_ARCHEOLOGIST) || Pantheon_if(PM_ARCHEOLOGIST)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Anachrononaut */\
            || ((a) == &artilist[ART_FORCE_PIKE_OF_THE_RED_GUAR] && (Role_if(PM_ANACHRONONAUT) || Pantheon_if(PM_ANACHRONONAUT)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Barbarian */\
            || ((a) == &artilist[ART_GAUNTLETS_OF_THE_BERSERKER] && (Role_if(PM_BARBARIAN) || Pantheon_if(PM_BARBARIAN)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Binder */\
            || ((a) == &artilist[ART_DECLARATION_OF_THE_APOSTAT] && (Role_if(PM_EXILE) || Pantheon_if(PM_EXILE)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !u.sealCounts &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_SOUL_LENS] && (Role_if(PM_EXILE) || Pantheon_if(PM_EXILE)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_SEAL_OF_THE_SPIRITS] && (Role_if(PM_EXILE) || Pantheon_if(PM_EXILE)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Caveman/Cavewoman */\
            || ((a) == &artilist[ART_TORCH_OF_ORIGINS] && (Role_if(PM_CAVEMAN) || Pantheon_if(PM_CAVEMAN)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Convict */\
            || ((a) == &artilist[ART_STRIPED_SHIRT_OF_THE_MURDE] && (Role_if(PM_CONVICT) || Pantheon_if(PM_CONVICT)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_STRIPED_SHIRT_OF_THE_THIEF] && (Role_if(PM_CONVICT) || Pantheon_if(PM_CONVICT)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_STRIPED_SHIRT_OF_THE_FALSE] && (Role_if(PM_CONVICT) || Pantheon_if(PM_CONVICT)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Healer */\
            || ((a) == &artilist[ART_SCALPEL_OF_LIFE_AND_DEATH] && (Role_if(PM_HEALER) || Pantheon_if(PM_HEALER)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_GAUNTLETS_OF_THE_HEALING_H] && (Role_if(PM_HEALER) || Pantheon_if(PM_HEALER)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_RING_OF_HYGIENE_S_DISCIPLE] && (Role_if(PM_HEALER) || Pantheon_if(PM_HEALER)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Knight */\
            || ((a) == &artilist[ART_COPE_OF_THE_ELDRITCH_KNIGH] && (Role_if(PM_KNIGHT) || Pantheon_if(PM_KNIGHT)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
            /* Monk */\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_BOOZE_OF_THE_DRUNKEN_MASTE] && (Role_if(PM_MONK) || Pantheon_if(PM_MONK)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_WRAPPINGS_OF_THE_SACRED_FI] && (Role_if(PM_MONK) || Pantheon_if(PM_MONK)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_KHAKKHARA_OF_THE_MONKEY] && (Role_if(PM_MONK) || Pantheon_if(PM_MONK)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Nobleman/Noblewoman */\
            /* Priest/Priestess */\
            || ((a) == &artilist[ART_MACE_OF_THE_EVANGELIST] && (Role_if(PM_PRIEST) || Pantheon_if(PM_PRIEST)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !exist_artifact(ORIHALCYON_GAUNTLETS, artilist[ART_GAUNTLETS_OF_THE_DIVINE_DI].name))\
            || ((a) == &artilist[ART_GAUNTLETS_OF_THE_DIVINE_DI] && (Role_if(PM_PRIEST) || Pantheon_if(PM_PRIEST)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Pirate */\
            /* Rogue */\
            || ((a) == &artilist[ART_DART_OF_THE_ASSASSIN] && (Role_if(PM_ROGUE) || Pantheon_if(PM_ROGUE)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_SWORD_OF_THE_KLEPTOMANIAC] && (Role_if(PM_ROGUE) || Pantheon_if(PM_ROGUE)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Ranger */\
            || ((a) == &artilist[ART_HELM_OF_THE_ARCANE_ARCHER] && (Role_if(PM_RANGER) || Role_if(PM_BARD) || Pantheon_if(PM_RANGER)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_FIGURINE_OF_PYGMALION] && (Role_if(PM_RANGER) || Role_if(PM_BARD) || Pantheon_if(PM_RANGER)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_FIGURINE_OF_GALATEA] && (Role_if(PM_RANGER) || Role_if(PM_BARD) || Pantheon_if(PM_RANGER)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Samurai */\
            || ((a) == &artilist[ART_HELM_OF_THE_NINJA] && (Role_if(PM_SAMURAI) || Pantheon_if(PM_SAMURAI)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Tourist */\
            || ((a) == &artilist[ART_TOWEL_OF_THE_INTERSTELLAR_] && (Role_if(PM_TOURIST) || Pantheon_if(PM_TOURIST)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_ENCYCLOPEDIA_GALACTICA] && (Role_if(PM_TOURIST) || Pantheon_if(PM_TOURIST)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Troubadour */\
            /* Valkyrie */\
            || ((a) == &artilist[ART_TWIG_OF_YGGDRASIL] && (Role_if(PM_VALKYRIE) || Pantheon_if(PM_VALKYRIE)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_SADDLE_OF_BRYNHILDR] && (Role_if(PM_VALKYRIE) || Pantheon_if(PM_VALKYRIE)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Wizard */\
            || ((a) == &artilist[ART_STAFF_OF_WILD_MAGIC] && (Role_if(PM_WIZARD) || Pantheon_if(PM_WIZARD)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_ROBE_OF_THE_ARCHMAGI] && (Role_if(PM_WIZARD) || Pantheon_if(PM_WIZARD)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_FORGE_HAMMER_OF_THE_ARTIFI] && (Role_if(PM_WIZARD) || Pantheon_if(PM_WIZARD)) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Drow */\
            || ((a) == &artilist[ART_RING_OF_LOLTH] && Race_if(PM_DROW) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Dwarf */\
            || ((a) == &artilist[ART_BULWARK_OF_THE_DWARVEN_DEF] && Race_if(PM_DWARF) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Elf */\
            || ((a) == &artilist[ART_NARYA] && Race_if(PM_ELF) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_NENYA] && Race_if(PM_ELF) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            || ((a) == &artilist[ART_VILYA] && Race_if(PM_ELF) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Gnome */\
            || ((a) == &artilist[ART_HAT_OF_THE_GIANT_KILLER] && Race_if(PM_ELF) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Half-Dragon */\
            || ((a) == &artilist[ART_PRISMATIC_DRAGON_PLATE] && Race_if(PM_HALF_DRAGON) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Human */\
            /* Incantifier */\
            || ((a) == &artilist[ART_FOOTPRINTS_IN_THE_LABYRINT] && Race_if(PM_INCANTIFIER) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
            /* Orc */\
            /* Vampire */\
            || ((a) == &artilist[ART_TRAPPINGS_OF_THE_GRAVE] && Race_if(PM_VAMPIRE) && u.ulevel >= MASTERY_ARTIFACT_LEVEL &&\
                !has_named_mastery_artifact())\
)

#define is_monk_safe_artifact(m) (\
				(m) == ART_TIE_DYE_SHIRT_OF_SHAMBHALA ||\
				(m) == ART_CLOAK_OF_THE_UNHELD_ONE ||\
				(m) == ART_WHISPERFEET ||\
				(m) == ART_WATER_FLOWERS ||\
				(m) == ART_HAMMERFEET ||\
				(m) == ART_SHIELD_OF_THE_RESOLUTE_HEA ||\
				(m) == ART_GAUNTLETS_OF_SPELL_POWER ||\
				(m) == ART_PREMIUM_HEART ||\
				(m) == ART_STORMHELM ||\
				(m) == ART_HELPING_HAND ||\
				(m) == ART_GARNET_ROD ||\
				(m) == ART_ARKENSTONE ||\
				(m) == ART_FLUORITE_OCTAHEDRON ||\
				(m) == ART_MARAUDER_S_MAP\
			)

#define double_bonus_damage_artifact(m) (\
	(m) == ART_LIMITED_MOON ||\
	(m) == ART_STAFF_OF_TWELVE_MIRRORS ||\
	(m) == ART_POSEIDON_S_TRIDENT\
	)

#endif /* ARTIFACT_H */
