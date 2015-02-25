/*	SCCS Id: @(#)artifact.h 3.4	1995/05/31	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ARTIFACT_H
#define ARTIFACT_H

#define SPFX_NONE   0x0000000L	/* no special effects, just a bonus */
#define SPFX_NOGEN  0x0000001L	/* item is special, bequeathed by gods */
#define SPFX_RESTR  0x0000002L	/* item is restricted - can't be named */
#define SPFX_INTEL  0x0000004L	/* item is self-willed - intelligent */
#define SPFX_SPEAK  0x0000008L	/* item can speak (not implemented) */
#define SPFX_SEEK   0x0000010L	/* item helps you search for things, ie, adds enhancement bonus to attempts */
#define SPFX_WARN   0x0000020L	/* item warns you of danger */
#define SPFX_ATTK   0x0000040L	/* item has a special attack (attk) */
#define SPFX_DEFN   0x0000080L	/* item has a special defence (defn) */
#define SPFX_DRLI   0x0000100L	/* drains a level from monsters */
#define SPFX_SEARCH 0x0000200L	/* automatic searching */
#define SPFX_BEHEAD 0x0000400L	/* beheads monsters */
#define SPFX_HALRES 0x0000800L	/* blocks hallucinations */
#define SPFX_ESP    0x0001000L	/* ESP (like amulet of ESP) */
#define SPFX_DISPL  0x0002000L	/* Displacement */
#define SPFX_REGEN  0x0004000L	/* Regeneration */
#define SPFX_EREGEN 0x0008000L	/* Energy Regeneration */
#define SPFX_HSPDAM 0x0010000L	/* 1/2 spell damage (on player) in combat */
#define SPFX_HPHDAM 0x0020000L	/* 1/2 physical damage (on player) in combat */
#define SPFX_TCTRL  0x0040000L	/* Teleportation Control */
#define SPFX_LUCK   0x0080000L	/* Increase Luck (like Luckstone) */
#define SPFX_DMONS  0x0100000L	/* attack bonus on one monster type */
#define SPFX_DCLAS  0x0200000L	/* attack bonus on monsters w/ symbol mtype */
#define SPFX_DFLAG1 0x0400000L	/* attack bonus on monsters w/ mflags1 flag */
#define SPFX_DFLAG2 0x0800000L	/* attack bonus on monsters w/ mflags2 flag */
#define SPFX_DALIGN 0x1000000L	/* attack bonus on non-aligned monsters  */
#define SPFX_DBONUS 0x1F00000L	/* attack bonus mask */
#define SPFX_XRAY   0x2000000L	/* gives X-RAY vision to player */
#define SPFX_REFLECT 0x4000000L /* Reflection */
#define SPFX_CONFL  0x8000000L  /* Conflict */
#define SPFX_AGGRM  0x10000000L /* Aggravate monster */
/* expanded special effects*/
#define SPFX2_NONE		0x0000000L	/* nothing special */
#define SPFX2_DIG		0x0000001L	/* Can dig with artifact, as if it were a pick. Clarent Patch (GreyKnight)*/
#define SPFX2_DEMIBANE  0x0000002L	/* +1d20 damage vs is_demihuman() mon types */
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

#define COMMAND_LADDER			16
#define COMMAND_CLAIRVOYANCE	17
#define COMMAND_FEAR			18
#define COMMAND_LIFE			19
#define COMMAND_KNEEL			20

#define COMMAND_SCIMITAR		21
#define COMMAND_WHIP			22
#define COMMAND_STRIKE			23


struct artifact {
	int	    otyp;
	const char  *name;
	unsigned long spfx;	/* special effect from wielding/wearing */
	unsigned long cspfx;	/* special effect just from carrying obj */
	unsigned long mtype;	/* monster type, symbol, or flag */
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

#define is_nameable_artifact(a) (\
			(a->spfx & (SPFX_NOGEN|SPFX_RESTR)) == 0\
			|| (a == &artilist[ART_SNICKERSNEE] && Role_if(PM_TOURIST) && (u.ulevel > 18 || u.uevent.qcompleted) )\
			|| (a == &artilist[ART_KUSANAGI_NO_TSURUGI] && Role_if(PM_SAMURAI) && u.ulevel >= 22 )\
			|| (a == &artilist[ART_GRANDMASTER_S_ROBE] && P_SKILL(P_MARTIAL_ARTS) >= P_GRAND_MASTER && u.ulevel >= 30 )\
			|| (a == &artilist[ART_STAFF_OF_THE_ARCHMAGI] && Role_if(PM_WIZARD) && u.ulevel == 30 )\
			|| (a == &artilist[ART_ROBE_OF_THE_ARCHMAGI] && Role_if(PM_WIZARD) && u.ulevel == 30 )\
			|| (a == &artilist[ART_HAT_OF_THE_ARCHMAGI] && Role_if(PM_WIZARD) && u.ulevel == 30 )\
			|| (a == &artilist[ART_FIRST_KEY_OF_CHAOS] && \
							(exist_artifact(SKELETON_KEY,artilist[ART_THIRD_KEY_OF_CHAOS].name) \
							||  u.ualign.type == A_CHAOTIC ) \
				)\
)

#endif /* ARTIFACT_H */
