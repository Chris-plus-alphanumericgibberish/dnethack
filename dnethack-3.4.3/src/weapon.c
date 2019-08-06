/*	SCCS Id: @(#)weapon.c	3.4	2002/11/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *	This module contains code for calculation of "to hit" and damage
 *	bonuses for any given weapon used, as well as weapons selection
 *	code for monsters.
 */
#include "hack.h"

/* Categories whose names don't come from OBJ_NAME(objects[type])
 */
#define PN_BARE_HANDED			(-1)	/* includes martial arts */
#define PN_TWO_WEAPONS			(-2)
#define PN_RIDING				(-3)
#define PN_POLEARMS				(-4)
#define PN_SABER				(-5)
#define PN_HAMMER				(-6)
#define PN_WHIP					(-7)
#define PN_ATTACK_SPELL			(-8)
#define PN_HEALING_SPELL		(-9)
#define PN_DIVINATION_SPELL		(-10)
#define PN_ENCHANTMENT_SPELL	(-11)
#define PN_CLERIC_SPELL			(-12)
#define PN_ESCAPE_SPELL			(-13)
#define PN_MATTER_SPELL			(-14)
#define PN_HARVEST				(-15)
#define PN_BEAST_MASTERY		(-16)
#define PN_FIREARMS				(-17)
#ifdef BARD
#define PN_MUSICALIZE			(-18)
#endif
#define PN_SHII_CHO				(-19)
#define PN_MAKASHI				(-20)
#define PN_SORESU				(-21)
#define PN_ATARU				(-22)
#define PN_DJEM_SO				(-23)
#define PN_SHIEN				(-24)
#define PN_NIMAN				(-25)
#define PN_JUYO					(-26)
#define PN_WAND_DAMAGE			(-27)

#define holy_damage(mon)	((mon == &youmonst) ? \
							hates_holy(youracedata) :\
							hates_holy_mon(mon))


static void FDECL(mon_ignite_lightsaber, (struct obj *, struct monst *));

STATIC_DCL void FDECL(give_may_advance_msg, (int));

#ifndef OVLB

STATIC_DCL NEARDATA const short skill_names_indices[];
STATIC_DCL NEARDATA const char *odd_skill_names[];
STATIC_DCL NEARDATA const char *barehands_or_martial[];

#else	/* OVLB */

STATIC_VAR NEARDATA const short skill_names_indices[P_NUM_SKILLS] = {
	0,                DAGGER,         KNIFE,        AXE,
	PICK_AXE,         SHORT_SWORD,    BROADSWORD,   LONG_SWORD,
	TWO_HANDED_SWORD, SCIMITAR,       PN_SABER,     CLUB,
	MACE,             MORNING_STAR,   FLAIL,
	PN_HAMMER,        QUARTERSTAFF,   PN_POLEARMS,  SPEAR,
	TRIDENT,          LANCE,          BOW,          SLING,
	PN_FIREARMS,	  CROSSBOW,       DART,         SHURIKEN,
	BOOMERANG,        PN_WHIP,        PN_HARVEST,   UNICORN_HORN,
	PN_ATTACK_SPELL,     PN_HEALING_SPELL,
	PN_DIVINATION_SPELL, PN_ENCHANTMENT_SPELL,
	PN_CLERIC_SPELL,     PN_ESCAPE_SPELL,
	PN_MATTER_SPELL,
	PN_WAND_DAMAGE,
#ifdef BARD
	PN_MUSICALIZE,
#endif
	PN_BARE_HANDED,   PN_TWO_WEAPONS,
	PN_BEAST_MASTERY,
	PN_SHII_CHO, PN_MAKASHI, PN_SORESU, PN_ATARU,
	PN_DJEM_SO, PN_SHIEN, PN_NIMAN, PN_JUYO,
#ifdef STEED
	PN_RIDING
#endif
};

/* note: entry [0] isn't used */
STATIC_VAR NEARDATA const char * const odd_skill_names[] = {
    "no skill",
    "bare hands",		/* use barehands_or_martial[] instead */
    "two weapon combat",
    "riding",
    "polearms",
    "saber",
    "hammer",
    "whip",
    "attack spells",
    "healing spells",
    "divination spells",
    "enchantment spells",
    "clerical spells",
    "escape spells",
    "matter spells",
	"farm implements",
	"beast mastery",
    "firearms",
#ifdef BARD
	"musicalize spell",
#endif
    "form I: Shii-Cho",
    "form II: Makashi",
    "form III: Soresu",
    "form IV: Ataru",
    "form V: Djem So",
    "form V: Shien",
    "form VI: Niman",
    "form VII: Juyo",
    "wand damage",
};
/* indexed vis `is_martial() */
STATIC_VAR NEARDATA const char * const barehands_or_martial[] = {
    "bare handed combat", "martial arts"
};

STATIC_OVL void
give_may_advance_msg(skill)
int skill;
{
	You_feel("more confident in your %sskills.",
		skill == P_NONE ?
			"" :
		skill <= P_LAST_WEAPON ?
			"weapon " :
		skill <= P_LAST_SPELL ?
			"spell casting " :
		"fighting ");
}

#endif	/* OVLB */

STATIC_DCL boolean FDECL(can_advance, (int, BOOLEAN_P));
STATIC_DCL boolean FDECL(could_advance, (int));
STATIC_DCL boolean FDECL(peaked_skill, (int));
STATIC_DCL int FDECL(slots_required, (int));
STATIC_DCL int FDECL(pendamage, (struct obj *,struct monst *));

#ifdef OVL1

STATIC_DCL char *FDECL(skill_level_name, (int,char *));
STATIC_DCL char *FDECL(max_skill_level_name, (int,char *));
STATIC_DCL void FDECL(skill_advance, (int));

#endif	/* OVL1 */

#define P_NAME(type) ((skill_names_indices[type] > 0) ? \
		      OBJ_NAME(objects[skill_names_indices[type]]) : \
		      (type == P_BARE_HANDED_COMBAT) ? \
			barehands_or_martial[martial_bonus()] : \
			odd_skill_names[-skill_names_indices[type]])

#ifdef OVLB
static NEARDATA const char kebabable[] = {
	S_XORN, S_DRAGON, S_JABBERWOCK, S_NAGA, S_GIANT, '\0'
};

/*
 *	hitval returns an integer representing the "to hit" bonuses
 *	of "otmp" against the monster.
 */
int
hitval(otmp, mon)
struct obj *otmp;
struct monst *mon;
{
	int	tmp = 0;
	struct permonst *ptr = mon->data;
	boolean Is_weapon = (otmp->oclass == WEAPON_CLASS || is_weptool(otmp));
	
	if(mon == &youmonst)
		ptr = youracedata;
	
	if (Is_weapon || (otmp->otyp >= LUCKSTONE && otmp->otyp <= ROCK && otmp->ovar1 == -P_FIREARM)){
		if(Race_if(PM_ORC) && otmp == uwep){
			tmp += max((u.ulevel+2)/3, otmp->spe);
		} else {
			tmp += otmp->spe;
		}
	}

/*	Put weapon specific "to hit" bonuses in below:		*/
	if(otmp->otyp == VIPERWHIP){
		otmp->ostriking = min(7,rn2(otmp->ovar1));
		tmp += (1+otmp->ostriking) * objects[otmp->otyp].oc_hitbon;
	} else {
		tmp += objects[otmp->otyp].oc_hitbon;
	}
	
	if (is_lightsaber(otmp) && otmp->altmode) tmp += objects[otmp->otyp].oc_hitbon;
	//But DON'T double the to hit bonus from spe for double lightsabers in dual bladed mode. It gets harder to use, not easier.
	
/*	Put weapon vs. monster type "to hit" bonuses in below:	*/

	/* Blessed weapons used against undead or demons */
	if (Is_weapon && otmp->blessed && mon &&
	   (holy_damage(mon))){
		if(otmp->oartifact == ART_EXCALIBUR)
			tmp += 7; //Quite holy
		if(otmp->oartifact == ART_JINJA_NAGINATA)
			tmp += 5; //Quite holy
		else if(otmp->oartifact == ART_VAMPIRE_KILLER)
			tmp += 7; //Quite holy
		else if(otmp->oartifact == ART_HOLY_MOONLIGHT_SWORD && !otmp->lamplit)
			tmp += rnd(20); //Quite holy
		else tmp += 2;
	}
	if (is_spear(otmp) &&
	   index(kebabable, ptr->mlet)) tmp += (ptr == &mons[PM_SMAUG]) ? 20 : 2;

	if (is_farm(otmp) &&
	    ptr->mlet == S_PLANT) tmp += 6;

	/* trident is highly effective against swimmers */
	if (otmp->otyp == TRIDENT && species_swims(ptr)) {
	   if (is_pool(mon->mx, mon->my, FALSE)) tmp += 4;
	   else if (ptr->mlet == S_EEL || ptr->mlet == S_SNAKE) tmp += 2;
	}

	/* weapons with the veioistafur stave are highly effective against sea monsters */
	if(otmp->oclass == WEAPON_CLASS && otmp->obj_material == WOOD && otmp->otyp != MOON_AXE
		&& (otmp->oward & WARD_VEIOISTAFUR) && mon->data->mlet == S_EEL) tmp += 4;
	
	/* Picks used against xorns and earth elementals */
	if (is_pick(otmp) &&
	   (mon_resistance(mon,PASSES_WALLS) && thick_skinned(ptr))) tmp += 2;

#ifdef INVISIBLE_OBJECTS
	/* Invisible weapons against monsters who can't see invisible */
	if (otmp->oinvis && !mon_resistance(mon,SEE_INVIS)) tmp += 3;
#endif

	/* Check specially named weapon "to hit" bonuses */
	if (otmp->oartifact){
		tmp += spec_abon(otmp, mon);
		if(Role_if(PM_BARD)) //legend lore
			tmp += 5;
	}

	return tmp;
}

/* Historical note: The original versions of Hack used a range of damage
 * which was similar to, but not identical to the damage used in Advanced
 * Dungeons and Dragons.  I figured that since it was so close, I may as well
 * make it exactly the same as AD&D, adding some more weapons in the process.
 * This has the advantage that it is at least possible that the player would
 * already know the damage of at least some of the weapons.  This was circa
 * 1987 and I used the table from Unearthed Arcana until I got tired of typing
 * them in (leading to something of an imbalance towards weapons early in
 * alphabetical order).  The data structure still doesn't include fields that
 * fully allow the appropriate damage to be described (there's no way to say
 * 3d6 or 1d6+1) so we add on the extra damage in dmgval() if the weapon
 * doesn't do an exact die of damage.
 *
 * Of course new weapons were added later in the development of Nethack.  No
 * AD&D consistency was kept, but most of these don't exist in AD&D anyway.
 *
 * Second edition AD&D came out a few years later; luckily it used the same
 * table.  As of this writing (1999), third edition is in progress but not
 * released.  Let's see if the weapon table stays the same.  --KAA
 * October 2000: It didn't.  Oh, well.
 */

/* 
 * dmg_val_core modifies a "weapon_dice" pointer, which contains enough
 * info to (hopefully) perform 99% of the dnethack's weapon dice rolls
 * 
 * will ignore otyp, unless if it is called without an obj
 *
 * returns the enchantment multiplier for the weapon.
 * typically 1, but artifacts and lightsabers affect it
 */
int
dmgval_core(wdice, large, obj, otyp)
struct weapon_dice *wdice;
boolean large;
struct obj* obj;
int otyp;
{
	int dmod = 0;						/* die size modifier */
	int spe_mult = 1;					/* multiplier for enchantment value */

	/* use the otyp of the object called, if we have one */
	if (obj)
		otyp = obj->otyp;

	/* a number of definitions that will clean up the code*/
#define ocaa	(wdice->oc.aatyp)
#define ocad	(wdice->oc.adtyp)
#define ocn		(wdice->oc.damn)
#define ocd		(wdice->oc.damd)
#define bonaa	(wdice->bon.aatyp)
#define bonad	(wdice->bon.adtyp)
#define bonn	(wdice->bon.damn)
#define bond	(wdice->bon.damd)
#define flat	(wdice->flat)

	/* initialize wdice */
	ocaa = AT_NONE;
	ocad = AD_PHYS;
	ocn = 1;
	ocd = 2;
	bonaa = AT_NONE;
	bonad = AD_PHYS;
	bonn = 0;
	bond = 0;
	flat = 0;

	/* in case we are dealing with a complete lack of a weapon (!obj, !otyp)
		* just skip everything and only initialize wdice
		*/
	if (!otyp)
		return 0;

	/* grab ldie and sdie from the objclass definition */
	ocd = (large ? objects[otyp].oc_wldam : objects[otyp].oc_wsdam);

	/* set dmod, if possible*/
	if (obj){
		dmod = obj->objsize - MZ_MEDIUM;
		if (obj->oartifact == ART_FRIEDE_S_SCYTHE)
			dmod += 2;
		else if (obj->oartifact == ART_HOLY_MOONLIGHT_SWORD && obj->lamplit)
			dmod += 2;

		/* material-based dmod modifiers */
		if (obj->obj_material != objects[obj->otyp].oc_material)
		{
			/* if something is made of an especially effective material 
			 * and it normally isn't, it gets a dmod bonus 
			 */
			int mat;	/* material being contemplated */
			int mod;	/* mat modifier sign: -1 for base, +1 for current*/
			for (mod = -1; mod<2; mod+=2){
				if (mod == -1)
					mat = objects[obj->otyp].oc_material;
				else
					mat = obj->obj_material;
				switch (mat)
				{
				/* flimsy weapons are bad damage */
				case LIQUID:
				case WAX:
				case VEGGY:
				case FLESH:
				case PAPER:
				case CLOTH:
				case LEATHER:
					dmod -= mod;
					break;
				/* gold and platinum are heavy
				 * ...regardless that the elven mace is wooden, 
				 *   and is _better_ than a standard iron mace */
				case GOLD:
				case PLATINUM:
					if (is_bludgeon(obj))
						dmod += mod;
					break;
				/* glass and obsidian have sharp edges and points 
				 * shadowsteel ??? but gameplay-wise, droven weapons
				 *   made out of this troublesome-to-maintain material
				 *   shouldn't be weaker than their obsidian counterparts
				 */
				case GLASS:
				case OBSIDIAN_MT:
				case SHADOWSTEEL:
					if (is_slashing(obj) || is_stabbing(obj))
						dmod += mod;
					break;
				/* dragon teeth are good at piercing */
				case DRAGON_HIDE:
					if (is_stabbing(obj))
						dmod += mod;
					break;
				}
			}
		}
	}
	/* apply dmod to ocd */
	ocd = max(2, ocd + dmod * 2);

	/* SPECIAL CASES (beyond what is defined in objects.c) START HERE */

	/* Artifacts and other fun things that need obj to exist and apply for both small and large (mostly?) */
	if (obj)
	{
		if (obj->oartifact == ART_VORPAL_BLADE || obj->oartifact == ART_SNICKERSNEE)
		{
			ocaa = AT_EXPL + 1;				// exploding dice, +1 damage per roll
			ocn = 2;						// roll two oc dice
		}
		else if (obj->oartifact == ART_SCOURGE_OF_LOLTH)
		{
			ocaa = AT_EXPL;					// basic exploding dice
		}
		else if (obj->oartifact == ART_LUCK_BLADE)
		{
			ocad = AD_LUCK;					// luck-based dice
		}
		else if (obj->oartifact == ART_FLUORITE_OCTAHEDRON)
		{
			ocaa = AT_EXPL;					// exploding
			ocad = AD_LUCK;					// lucky
			ocn = obj->quan;				// 1 die per octahedron
			ocd = 8;						// They are eight-sided dice
		}
		else if (obj->oartifact == ART_GIANTSLAYER)
		{
			bonn = (large ? 2 : 1);
			bond = 4 + 2 * dmod;
		}
		else if (obj->oartifact == ART_MJOLLNIR)
		{
			bonn = 2;
			bond = 4 + 2 * dmod;
			if (!large)
				flat += 2;
		}
		else if (obj->oartifact == ART_DURIN_S_AXE)
		{
			ocn = 2;
		}
		else if (obj->oartifact == ART_REAVER)
		{
			bonn = 1;
			bond = 8 + 2 * dmod;
		}
		else if (obj->oartifact == ART_TOBIUME)
		{
			flat += (large ? -(3+dmod) : -(2+dmod));
		}
		else if (obj->oartifact == ART_VAMPIRE_KILLER)
		{
			if (large)
			{
				bonn = 1;
				bond = 10 + 2 * dmod;
			}
			else
			{
				flat += 10 + 2 * dmod;
			}
		}
		else if (obj->oartifact == ART_GREEN_DRAGON_CRESCENT_BLAD){
			int wt = (int)objects[NAGINATA].oc_weight;
			if ((int)obj->owt > wt) {
				bonn = 1;
				bond = 12 * ((int)obj->owt - wt) / wt;	// this appears to be a constant +1d12, since I can't find any way to change its weight.
			}
		}
		else if (obj->oartifact == ART_GOLDEN_SWORD_OF_Y_HA_TALLA && otyp == BULLWHIP)
		{
			ocn = 1;
			ocd = 2;
			bonn = 1;
			bond = 4;
		}
		else if (otyp == MOON_AXE)
		{
			ocn = 2;
			ocd = ocd + 2 * (obj->ovar1 - 1) + 2 * dmod;	// die size is based on axe's phase of moon (0 <= ovar1 <= 4)
			if (!large && !obj->ovar1)			// eclipse moon axe is surprisingly effective against small creatures (2d12)
				ocd = 12 + 2 * dmod;
		}
		else if (otyp == HEAVY_IRON_BALL) {
			int wt = (int)objects[HEAVY_IRON_BALL].oc_weight;

			if ((int)obj->owt > wt) {
				wt = ((int)obj->owt - wt) / 160;
				ocd += 4 * wt;
				if (ocd > 25) ocd = 25;	/* objects[].oc_wldam */
			}
		}
	}


#define plus_base(n,x)	bonn = (n); bond = (x)
#define plus(n,x)		plus_base((n), (x) + 2 * dmod)
#define pls(x)			plus(1, (x))
#define add(x)			flat += max(0, (x) + dmod)
#define chrgd			(!obj || obj->ovar1>0)
	/* bonus dice */
	switch (otyp)
	{
	case CROSSBOW_BOLT:			add(1); break;
	case DROVEN_BOLT:			add(1); break;
	case TRIDENT:				if(large){plus(2,4);} else {add(1);} break;
	case BATTLE_AXE:			if(large){plus(2,4);} else {pls(4);} break;
	case VIBROBLADE:			
	case WHITE_VIBROSWORD:
	case GOLD_BLADED_VIBROSWORD:
	case WHITE_VIBROZANBATO:
	case GOLD_BLADED_VIBROZANBATO:
	case WHITE_VIBROSPEAR:
	case GOLD_BLADED_VIBROSPEAR:
	case FORCE_PIKE:
	case DOUBLE_FORCE_BLADE:// external special case: wielded without twoweaponing
	case FORCE_BLADE:
	case FORCE_SWORD:
								if(chrgd){ocn++;flat+=ocd/2;} break;
	case FORCE_WHIP:
								if(chrgd){
									ocn++;
									flat+=ocd/2;
									if(large){
										plus(2,4);
										add(2);
									} else {
										add(2); //Base
										add(1); //Bonus (applies size mod to both)
									}
								} else {
									//As flail
									if(large){
										pls(4);
									} else {
										add(1); //Base
									}
								}
		break;
	case RED_EYED_VIBROSWORD:
								if(chrgd){ocn+=2;flat+=ocd;} break;
	case MIRRORBLADE:			break;	// external special case: depends on defender's weapon
	case RAPIER:				break;	// external special case: Silver Starlight vs plants
	case RAKUYO:				break;	// external special case: wielded without twoweaponing
	case BROADSWORD:			if(large){add(1);} else {pls(4);} break;
	case ELVEN_BROADSWORD:		if(large){add(2);} else {pls(4);} break;
	case CRYSTAL_SWORD:			if(large){pls(12);} else {pls(8);} break;
	case TWO_HANDED_SWORD:		if(large){plus(2,6);} else {;} break;
	case TSURUGI:				if(large){plus(2,6);} else {;} break;
	case RUNESWORD:				if(large){add(1);} else {pls(4);} break;
	case PARTISAN:				if(large){add(1);} else {;} break;
	case RANSEUR:				pls(4); break;
	case SPETUM:				if(large){pls(6);} else {add(1);} break;
	case HALBERD:				if(large){pls(6);} else {;} break;
	case BARDICHE:				if(large){plus(2,4);} else {pls(4);} break;
	case VOULGE:				pls(4); break;
	case DWARVISH_MATTOCK:		if(large){plus(2,6);} else {;} break;
	case GUISARME:				if(large){;} else {pls(4);} break;
	case BILL_GUISARME:			if(large){;} else {pls(4);} break;
	case LUCERN_HAMMER:			if(large){;} else {pls(4);} break;
	case BEC_DE_CORBIN:			if(large){;} else {;} break;
	case SCYTHE:				if(large){pls(4);} else {pls(4);} break;
	case MACE:					if(large){;} else {add(1);} break;
	case ELVEN_MACE:			if(large){;} else {add(1);} break;
	case MORNING_STAR:			if(large){add(1);} else {pls(4);} break;
	case WAR_HAMMER:			if(large){;} else {add(1);} break;
	case KAMEREL_VAJRA:			if(large){;} else {;} break;	// external special case: lightsaber forms
	case FLAIL:					if(large){pls(4);} else {add(1);} break;
	case VIPERWHIP:				if(large){;} else {;} break;	// external special case: number of heads striking
	case BULLET:				ocn++; flat += 4; break;
	case SILVER_BULLET:			ocn++; flat += 4; break;
	case SHOTGUN_SHELL:			ocn++; flat += 4; break;
	case ROCKET:				ocn++; flat += 4; break;
	case BLASTER_BOLT:			ocn += 2; flat += ocd; break;
	case HEAVY_BLASTER_BOLT:	ocn += 2; flat += ocd; break;
	case LASER_BEAM:			ocn += 2; flat += 10; break;
	case CHAIN:					add(1); break;
	case SEISMIC_HAMMER:		if (chrgd){ ocd *= 3; } break;
	case ACID_VENOM:			if (obj&&obj->ovar1){ ocn = 0; flat = obj->ovar1; } else{ add(6); } break;
	case LIGHTSABER:			spe_mult = 3; ocn += 2; if(obj&&obj->altmode){ plus(3,3); } break;		// external special case: lightsaber forms
	case BEAMSWORD:				spe_mult = 3; ocn += 2; if(obj&&obj->altmode){ plus(3,3); } break;		// external special case: Atma Weapon, lightsaber forms
	case DOUBLE_LIGHTSABER:		spe_mult = 3; ocn += 2; if(obj&&obj->altmode){ ocn+=3; } break;			// external special case: lightsaber forms
	}
#undef plus_base
#undef plus
#undef pls
#undef add
#undef chrgd
	/* more special cases that wouldn't really fit into the switch above */
	/* override for lightsaber dice if the lightsaber is turned off */
	if (obj && (otyp == LIGHTSABER || otyp == BEAMSWORD || otyp == DOUBLE_LIGHTSABER) && !litsaber(obj))
	{
		spe_mult = 1;
		ocn = 1;
		ocd = 2;
		bonn = 0;
		bond = 0;
	}
	/* kamerel vajra */
	if (obj && otyp == KAMEREL_VAJRA && litsaber(obj))
	{
		if (obj->where == OBJ_MINVENT && obj->ocarry->data == &mons[PM_ARA_KAMEREL])
		{
			spe_mult = 3;
			ocn *= 3;
			flat *= 3;
		}
		else
		{
			spe_mult = 2;
			ocn *= 2;
			flat *= 2;
		}
	}
	/* Infinity's Mirrored Arc */
	if (obj && obj->oartifact == ART_INFINITY_S_MIRRORED_ARC)
	{
		xchar x, y;
		ocn = 1;
		get_obj_location(obj, &x, &y, 0);
		if (levl[x][y].lit &&
			!(viz_array[y][x] & TEMP_DRK3 &&
			!(viz_array[y][x] & TEMP_LIT1)))
		{
			ocn += 2;
		}
		if (viz_array[y][x] & TEMP_LIT1 &&
			!(viz_array[y][x] & TEMP_DRK3))
		{
			ocn += 1;
		}

		if (obj->altmode)
			ocn *= 2;
		/* set spe_mult */
		spe_mult = ocn;
	}
	/* Atma Weapon */
	if (obj && obj->oartifact == ART_ATMA_WEAPON && litsaber(obj))
	{
		/* note: multiplied enchantment bonus damage has to be handled in dmgval() */
		if (obj == uwep &&	!Drain_resistance){
			spe_mult = 3;
			ocn = 3;
			bonn = 1;
			bond = u.ulevel;
		}
		else {
			spe_mult = 2;
			ocn = 2;
		}
	}
	/* the Tentacle Rod gets no damage from enchantment */
	if (obj && obj->oartifact == ART_TENTACLE_ROD)
		spe_mult = 0;
	/* if bonus dice exist, their minimum size is of a d2 */
	if (bonn && bond < 2)
		bond = 2;
	/* if bonus dice are identical in size & roll to oc dice, combine them */
	if (bonn && (ocd == bond && ocaa == bonaa && ocad == bonad))
	{
		ocn += bonn;
		bonn = 0;
		bond = 0;
	}
	/* undefine the helpers */
#undef ocaa	
#undef ocad	
#undef ocn		
#undef ocd		
#undef bonaa	
#undef bonad	
#undef bonn	
#undef bond	
#undef flat
	return spe_mult;
}

/* 
 * calculates the die roll from a given NdX with some function
 * as specified in the weapon_dice struct
 * 
 * data is received in the form of an attack struct
 */
int
weapon_dmg_roll(wdie, youdefend)
struct attack *wdie;
boolean youdefend;		// required for lucky dice
{
	int tmp = 0;

	/* verify there are appropriate dice to roll */
	if (!wdie->damn)
		return 0;
	if (wdie->damd < 1)
	{
		impossible("weapon_dmg_roll called with <1 die size and non-zero die number!");
		return 0;
	}

	/* determine function to use, and calculate the oc dice */
	if (wdie->aatyp == AT_NONE)
	{//normal dice
		if (wdie->adtyp == AD_PHYS)
		{//normal dice
			tmp += d(wdie->damn, wdie->damd);
		}
		else
		{//assumed to be lucky dice, which at this time don't have a nice NdX function
			int n = wdie->damn;
			for (n; n; n--)
			{
				tmp += youdefend ?
					(rnl(wdie->damd) + 1) :
					(wdie->damd - rnl(wdie->damd));
			}
		}
	}
	else
	{//assumed to be exploding dice
		int m = wdie->aatyp - AT_EXPL;

		if (wdie->adtyp == AD_PHYS)
		{//exploding dice
			tmp += exploding_d(wdie->damn, wdie->damd, m);
		}
		else
		{//lucky exploding dice
			tmp = youdefend ?
				unlucky_exploding_d(wdie->damn, wdie->damd, m) :
				lucky_exploding_d(wdie->damn, wdie->damd, m);
		}
	}
	return tmp;
}

/*
 *	dmgval returns an integer representing the damage bonuses
 *	of "otmp" against the monster.
 */
int
dmgval(otmp, mon, spec)
struct obj *otmp;
struct monst *mon;
int spec;
{
	int tmp = 0;				// running damage sum
	int otyp = otmp->otyp;		// obj's type
	int spe_mult = 1;			// enchantment spe_mult
	struct permonst *ptr;		// defender's permonst
	struct weapon_dice wdice;	// weapon dice of otmp
	boolean Is_weapon = (otmp->oclass == WEAPON_CLASS || is_weptool(otmp)), youdefend = mon == &youmonst;
	boolean add_dice = TRUE;	// should dmgval_core() be called to add damage? Overridden (to false) by some special cases.
	// if (!mon) ptr = &mons[NUMMONS];
	if (!mon) ptr = &mons[PM_HUMAN];
	else ptr = youdefend ? youracedata : mon->data;

	if (otyp == CREAM_PIE) return 0;
	
	if (otmp->oartifact == ART_HOUCHOU)
	        return 9999;

	/* grab the weapon dice from dmgval_core */
	spe_mult = dmgval_core(&wdice, bigmonst(ptr), otmp, otyp);

	/* increase die sizes by 2 if Marionette applies*/
	if (spec & SPEC_MARIONETTE)
	{
		wdice.oc.damd += 2;
		wdice.bon.damd += 2;
	}

	/* special cases of otyp not covered by dmgval_core:
	 *  - rakuyo					- add bonus damage
	 *  - double vibro blade		- double all damage
	 *  - viperwhips				- add ostriking
	 *  - mirrorblades				- total replacement
	 *  - crystal sword				- bonus enchantment damage
	 *  - seismic hammer			- damage die based on enchantment
	 *  - charged future weapons	- drain charge
	 * special cases of artifact not covered by dmgval_core:
	 *  - Silvered Starlight		- add bonus damage
	 *  - Atma Weapon				- multiply by health %
	 * other special cases not covered by dmgval_core:
	 *  - all lightsabers			- add lightsaber forms
	 */
	switch (otyp)
	{
	case RAKUYO:
		// deals bonus damage when not twoweaponing
		if ((otmp == uwep && !u.twoweap) || (mcarried(otmp) && otmp->owornmask&W_WEP))
		{
			/* modify wdice's bonus die and apply it */
			// bonus 1d4 vs small
			// bonus 1d3 vs large
			wdice.bon.damn = 1;
			wdice.bon.damd = max(2, ((bigmonst(ptr) ? 3 : 4) + 2 * (otmp->objsize - MZ_MEDIUM + !!(spec & SPEC_MARIONETTE))));
			// doubled enchantment
			spe_mult *= 2;
		}
		break;
	case VIPERWHIP:
		// extra heads means more base dice of damage
		if (otmp->ostriking > 0)
		{
			/* modify wdice's dice */
			// 1 additional die for every extra head striking
			wdice.oc.damn += otmp->ostriking;
			spe_mult += otmp->ostriking;
		}
		break;
	case MIRRORBLADE:{
		struct obj * otmp2 = (youdefend ? uwep : MON_WEP(mon));
		if (otmp2 && otmp2->otyp == MIRRORBLADE)
		{// clashing mirrorblades are quite deadly
			// 2 dice, exploding, with a flat explosion bonus of the average of attacker's and defender's weapons
			wdice.oc.aatyp = AT_EXPL + (otmp2->spe + otmp->spe) / 2;
			wdice.oc.damn = 2;
		}
		else
		{// if the defender's weapon would be stronger than the mirrorblade, use that instead
			/* calculate what the normal damage dice would be */
			int hyp = 0;
			hyp += weapon_dmg_roll(&(wdice.oc), youdefend);
			hyp += weapon_dmg_roll(&(wdice.bon), youdefend);
			hyp += wdice.flat;

			/* calculate what the mirrored damage dice would be */
			int mir = 0;
			struct weapon_dice mirdice;
			/* grab the weapon dice from dmgval_core */
			(void) dmgval_core(&mirdice, bigmonst(ptr), otmp2, 0);	//note: dmgval_core handles zero weapons gracefully
			if (spec & SPEC_MARIONETTE)
			{
				mirdice.oc.damd += 2;
				mirdice.bon.damd += 2;
			}
			/* find the damage from those dice */
			mir += weapon_dmg_roll(&(mirdice.oc), youdefend);
			mir += weapon_dmg_roll(&(mirdice.bon), youdefend);
			mir += mirdice.flat;

			/* use the better */
			tmp += max(hyp, mir);
			/* cannot be negative */
			if (tmp < 0)
				tmp = 0;
			/* signal to NOT add normal dice */
			add_dice = FALSE;
		}
	}break;
	case CRYSTAL_SWORD:
		// small bonus enchantment damage
		wdice.flat += otmp->spe/3;
		break;
	case LIGHTSABER:
	case BEAMSWORD:
	case DOUBLE_LIGHTSABER:
		// drain charge on lightsabers
		if (!((otmp->oartifact == ART_ATMA_WEAPON && otmp == uwep && !Drain_resistance) ||
			otmp->oartifact == ART_INFINITY_S_MIRRORED_ARC))
		{
			otmp->age -= 100;
			if (otmp->altmode){
				otmp->age -= 100;
			}
		}
		break;
	case SEISMIC_HAMMER:
		// damage die is increased by 3x the enchantment of the hammer when charged
		if (otmp->ovar1)
		{
			wdice.oc.damd += 3 * (otmp->spe);
			// drain charge on future-tech powered weapons
			otmp->ovar1--;
		}
		break;
	case VIBROBLADE:
	case WHITE_VIBROSWORD:
	case GOLD_BLADED_VIBROSWORD:
	case RED_EYED_VIBROSWORD:
	case WHITE_VIBROZANBATO:
	case GOLD_BLADED_VIBROZANBATO:
	case WHITE_VIBROSPEAR:
	case GOLD_BLADED_VIBROSPEAR:
	case FORCE_PIKE:
	case FORCE_BLADE:
	case FORCE_SWORD:
	case FORCE_WHIP:
		// drain charge on future-tech powered weapons
		if (otmp->ovar1)
			otmp->ovar1--;
		break;
	case DOUBLE_FORCE_BLADE:
		// deals bonus damage when not twoweaponing
		if((otmp == uwep && !u.twoweap) || (mcarried(otmp) && otmp->owornmask&W_WEP))
		{
			// doubled
			wdice.oc.damn *= 2;
			wdice.oc.damd *= 2;
			wdice.bon.damn *= 2;
			wdice.bon.damd *= 2;
			spe_mult *= 2;
	    }
		// drain charge on future-tech powered weapons
		if (otmp->ovar1)
			otmp->ovar1--;
		break;
	}

	switch (otmp->oartifact)
	{
	case ART_SILVER_STARLIGHT:
		// bonus damage to specific types of foes
		if (!(noncorporeal(ptr) || amorphous(ptr)
			|| ((stationary(ptr) || sessile(ptr)) && (ptr->mlet == S_FUNGUS || ptr->mlet == S_PLANT))
			))
		{
			/* modify wdice */
			// 1 additional main die
			// plus a 1d4 bonus die
			wdice.oc.damn += 1;
			wdice.bon.damn = 1;
			wdice.bon.damd = 4;
		}
		break;
	case ART_ATMA_WEAPON:
		/* damage is multiplied % of health remaining (currently only implemented for the player) */
		/* calculate damage normally */
		tmp += weapon_dmg_roll(&(wdice.oc), youdefend);
		tmp += weapon_dmg_roll(&(wdice.bon), youdefend);
		tmp += wdice.flat;
		/* apply the multiplier, if applicable */
		if (otmp == uwep &&	!Drain_resistance)
		{
			tmp *= Upolyd ?
				((float)u.mh) / u.mhmax :
				((float)u.uhp) / u.uhpmax;
		}
		/* cannot be negative */
		if (tmp < 0)
			tmp = 0;
		/* don't re-add the weapon dice */
		add_dice = FALSE;
		break;
	}

	/* add the main weapon dice */
	if (add_dice)
	{//true, unless overridden by a special case above (mirrorblades, Atma Weapon)
		/* find the damage from those dice */
		tmp += weapon_dmg_roll(&(wdice.oc), youdefend);
		tmp += weapon_dmg_roll(&(wdice.bon), youdefend);
		tmp += wdice.flat;
		/* cannot be negative */
		if (tmp < 0)
			tmp = 0;
	}

	/* lightsaber forms */
	if (is_lightsaber(otmp) && (otmp == uwep || (u.twoweap && otmp == uswapwep))){
		if (u.fightingForm == FFORM_MAKASHI && otmp == uwep && !u.twoweap && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm))){
			switch (min(P_SKILL(FFORM_MAKASHI), P_SKILL(weapon_type(otmp)))){
			case P_BASIC:
				if (mon->ustdym<5) mon->ustdym += 1;
				break;
			case P_SKILLED:
				if (mon->ustdym<10) mon->ustdym += 2;
				break;
			case P_EXPERT:
				if (mon->ustdym<20) mon->ustdym += 4;
				break;
			}
		}
		else if (u.fightingForm == FFORM_ATARU && u.lastmoved + 1 >= monstermoves && (!uarm || is_light_armor(uarm))){
			switch (min(P_SKILL(FFORM_ATARU), P_SKILL(weapon_type(otmp)))){
			case P_BASIC:
				tmp += d(2, wdice.oc.damd);
				if (otmp->altmode){ //Probably just the Annulus
					tmp += d(2, 3);
				}
				break;
			case P_SKILLED:
				tmp += d(4, wdice.oc.damd);
				if (otmp->altmode){ //Probably just the Annulus
					tmp += d(4, 3);
				}
				break;
			case P_EXPERT:
				tmp += d(6, wdice.oc.damd);
				if (otmp->altmode){ //Probably just the Annulus
					tmp += d(6, 3);
				}
				break;
			}
		}
		else if (u.fightingForm == FFORM_DJEM_SO && mon->mattackedu && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm))){
			int sbon = ACURR(A_STR);
			if (sbon >= STR19(19)) sbon -= 100; //remove percentile adjustment
			else if (sbon > 18) sbon = 18; //remove percentile adjustment
			//else it is fine as is.
			sbon = (sbon + 2) / 3; //1-9
			switch (min(P_SKILL(FFORM_DJEM_SO), P_SKILL(weapon_type(otmp)))){
			case P_BASIC:
				tmp += d(1, sbon);
				break;
			case P_SKILLED:
				tmp += d(2, sbon);
				break;
			case P_EXPERT:
				tmp += d(3, sbon);
				break;
			}
		}
		else if (u.fightingForm == FFORM_NIMAN && u.lastcast >= monstermoves && (!uarm || !is_metallic(uarm))){
			switch (min(P_SKILL(FFORM_NIMAN), P_SKILL(weapon_type(otmp)))){
			case P_BASIC:
				tmp -= 2;
				if (u.lastcast >= monstermoves) tmp += d(otmp->altmode ? 6 : 3, u.lastcast - monstermoves + 1);
				break;
			case P_SKILLED:
				tmp -= 1;
				if (u.lastcast >= monstermoves) tmp += d(otmp->altmode ? 12 : 6, u.lastcast - monstermoves + 1);
				break;
			case P_EXPERT:
				if (u.lastcast >= monstermoves) tmp += d(otmp->altmode ? 18 : 9, u.lastcast - monstermoves + 1);
				break;
			}
		}
	}

	/* enchantment damage */
	if ((otmp->oclass == WEAPON_CLASS) || is_weptool(otmp) || (otmp->otyp >= LUCKSTONE && otmp->otyp <= ROCK && otmp->ovar1 == -P_FIREARM))
	{
		int dambon = otmp->spe;
		/* player orcs can use their level as their weapon's enchantment */
		if (otmp->where == OBJ_INVENT && Race_if(PM_ORC))
			dambon = max((u.ulevel + 1) / 3, dambon);

		/* add damage */
		tmp += spe_mult * dambon;
		/* cannot reduce damage below 0 */
		if (tmp < 0)
			tmp = 0;
	}

	if (otmp->obj_material <= LEATHER && (thick_skinned(ptr) || (youdefend && u.sealsActive&SEAL_ECHIDNA)))
		/* thick skinned/scaled creatures don't feel it */
		tmp = 0;

	if (otmp->oproperties&OPROP_FLAYW && (thick_skinned(ptr) || (youdefend && u.sealsActive&SEAL_ECHIDNA) || (mon && some_armor(mon))))
		tmp = 1;
	
	if(is_stabbing(otmp) && ptr == &mons[PM_SMAUG]) tmp += rnd(20);
	if(is_farm(otmp) && ptr->mlet == S_PLANT){
		tmp *= 2;
		tmp += rnd(20);
	} if(u.sealsActive&SEAL_EVE && !youdefend && ptr->mlet == S_PLANT) tmp *= 2;
/*	Put weapon vs. monster type damage bonuses in below:	*/
	if (Is_weapon || otmp->oclass == GEM_CLASS ||
		otmp->oclass == BALL_CLASS || otmp->oclass == CHAIN_CLASS) {
	    int bonus = 0;
		int resistmask = 0;
		int weaponmask = 0;
		static int warnedotyp = 0;
		static struct permonst *warnedptr = 0;
		
		if(otmp->oartifact == ART_PEN_OF_THE_VOID){
			tmp += pendamage(otmp, mon);
		}
		if(otmp->oartifact == ART_ROD_OF_SEVEN_PARTS 
			&& !otmp->blessed && !otmp->cursed
			&& mon
			&& (holy_damage(mon) || hates_unholy_mon(mon))
		){
			bonus += rnd(10);
		}
	    if (otmp->blessed && mon && (holy_damage(mon))){
			int dsize = 4;
			if(otmp->obj_material == GOLD)
				dsize = 20;
			
			if(otmp->oartifact == ART_EXCALIBUR) bonus += d(3,7); //Quite holy
			else if(otmp->oartifact == ART_JINJA_NAGINATA)
				bonus += rnd(12); //Quite holy
			else if(otmp->oartifact == ART_LANCE_OF_LONGINUS)
				bonus += d(3,7); //Quite holy
			else if(otmp->oartifact == ART_HOLY_MOONLIGHT_SWORD && !otmp->lamplit)
				bonus += rnd(10) + otmp->spe; //Quite holy
			else if(otmp->oartifact == ART_VAMPIRE_KILLER) 
					bonus += 7; //Quite holy
			else if(otmp->oartifact == ART_ROD_OF_SEVEN_PARTS)
				bonus += rnd(20); //Divinity
			else if(otmp->oartifact == ART_AMHIMITL)
				bonus += d(3,4);
			
			if(otmp->otyp == KHAKKHARA) bonus += d(rnd(3),dsize);
			else if(otmp->otyp == VIPERWHIP) bonus += d(otmp->ostriking+1,dsize);
			else bonus += rnd(dsize);
		}
	    if (is_axe(otmp) && is_wooden(ptr))
			bonus += rnd(4);
	    if ((otmp->obj_material == SILVER || arti_silvered(otmp))
			&& hates_silver(ptr)
			&& !(is_lightsaber(otmp) && litsaber(otmp))
			&& (!youdefend || !(u.sealsActive&SEAL_EDEN))
		){
			if(otyp == KHAKKHARA) bonus += d(rnd(3),20);
			else if(otmp->otyp == VIPERWHIP) bonus += d(otmp->ostriking+1,20);
			else if(otmp->oartifact == ART_PEN_OF_THE_VOID && mvitals[PM_ACERERAK].died > 0) bonus += d(2,20);
			else if(otmp->oartifact == ART_SILVER_STARLIGHT) bonus += d(2,20);
			else bonus += rnd(20);
		}
	    if (otmp->obj_material == IRON 
			&& hates_iron(ptr)
			&& !(is_lightsaber(otmp) && litsaber(otmp))
		){
			if(youdefend){
				if(otyp == KHAKKHARA) bonus += d(rnd(3),u.ulevel);
				else if(otmp->otyp == VIPERWHIP) bonus += d(otmp->ostriking+1,mon->m_lev);
				else bonus += rnd(u.ulevel);
			} else {
				if(otyp == KHAKKHARA) bonus += d(rnd(3),mon->m_lev);
				else if(otmp->otyp == VIPERWHIP) bonus += d(otmp->ostriking,mon->m_lev);
				else bonus += rnd(mon->m_lev);
			}
		}
	    if (otmp->oartifact == ART_GLAMDRING && (is_orc(ptr) || is_demon(ptr))){
			bonus += rnd(20);
		}
	    if (is_unholy(otmp)
			&& mon && hates_unholy_mon(mon)
		){
			int bdm = 1;
			if(otmp->obj_material == GOLD)
				bdm = 2;
			
			if(otmp->oartifact == ART_STORMBRINGER)
				bonus += d(4*bdm,9); //Extra unholy (4d9 vs excal's 3d7)
			else if(otmp->oartifact == ART_LANCE_OF_LONGINUS)
				bonus += d(3*bdm,9); //Quite unholy
			else if(otmp->oartifact == ART_SCEPTRE_OF_THE_FROZEN_FLOO)
				bonus += 8*bdm; //Extra unholy
			else if(otmp->oartifact == ART_ROD_OF_SEVEN_PARTS)
				bonus += d(bdm, 20); //Tyranny
			else if(otmp->oartifact == ART_AMHIMITL)
				bonus += d(3*bdm, 4); //Tyranny
			else if(otmp->oartifact == ART_TECPATL_OF_HUHETOTL)
				bonus += d(bdm,4); //Somewhat unholy
			else if(otyp == KHAKKHARA) bonus += d(rnd(3)*bdm,9);
			else if(otmp->otyp == VIPERWHIP) bonus += d(otmp->ostriking*bdm,9);
			else bonus += d(bdm, 9);
		}
		
		if (mon && hates_unholy_mon(mon) && otmp->oartifact == ART_TECPATL_OF_HUHETOTL)
			bonus += d(1, 4); // always counts as unholy regardless, but its' pretty weak
		
		if(mon && mon->isminion){
			if(otmp->oartifact == ART_LIFEHUNT_SCYTHE)
				bonus += d(4,4) + otmp->spe; //Occult
		}
		
		
		if(otmp->oclass == WEAPON_CLASS && otmp->obj_material == WOOD && otmp->otyp != MOON_AXE
			&& (otmp->oward & WARD_VEIOISTAFUR) && ptr->mlet == S_EEL) bonus += rnd(20);
		

	    /* if the weapon is going to get a double damage bonus, adjust
	       this bonus so that effectively it's added after the doubling */
	    if (bonus > 1){
			if(otmp->oartifact){
				int dbonus = spec_dbon(otmp, mon, 100);
				if(dbonus >= 100) bonus = (bonus + 1) / (1+dbonus/100);
			}
			if(otmp->oproperties){
				int dbonus = oproperty_dbon(mon, otmp, 100);
				if(dbonus >= 100) bonus = (bonus + 1) / (1+dbonus/100);
			}
		}
		
		if(mon && (((resists_all(ptr) && !narrow_spec_applies(otmp, mon))) || resist_attacks(ptr))){
			tmp /= 4;
			if(!flags.mon_moving && !youdefend && warnedptr != ptr){
				pline("Weapons are ineffective against %s.", mon_nam(mon));
				warnedptr = ptr;
			}
		} else {
			if(is_bludgeon(otmp) 
				|| otmp->oartifact == ART_YORSHKA_S_SPEAR 
				|| otmp->oartifact == ART_GREEN_DRAGON_CRESCENT_BLAD
			){
				weaponmask |= WHACK;
			}
			if(is_stabbing(otmp)){
				weaponmask |= PIERCE;
			}
			if(is_slashing(otmp)){
				weaponmask |= SLASH;
			}
			if(is_blasting(otmp) 
				|| (otmp->oartifact == ART_HOLY_MOONLIGHT_SWORD && otmp->lamplit)
				|| otmp->oartifact == ART_FIRE_BRAND
				|| otmp->oartifact == ART_FROST_BRAND
			){
				weaponmask |= EXPLOSION;
			}
			
			if(resist_blunt(ptr) || (!youdefend && mon && (mon->mfaction == ZOMBIFIED))){
				resistmask |= WHACK;
			}
			if(resist_pierce(ptr) || (!youdefend && mon && (mon->mfaction == ZOMBIFIED || mon->mfaction == SKELIFIED || mon->mfaction == CRYSTALFIED))){
				resistmask |= PIERCE;
			}
			if(resist_slash(ptr) || (!youdefend && mon && (mon->mfaction == SKELIFIED || mon->mfaction == CRYSTALFIED))){
				resistmask |= SLASH;
			}
			
			if(mon && (weaponmask & ~(resistmask)) == 0L && !narrow_spec_applies(otmp, mon)){
				tmp /= 4;
				if(!flags.mon_moving && !youdefend && (warnedotyp != otmp->otyp || warnedptr != ptr)){
					pline("%s is ineffective against %s.", The(xname(otmp)), mon_nam(mon));
					warnedotyp = otmp->otyp;
					warnedptr = ptr;
				}
			} else {
				if(!flags.mon_moving && !youdefend && (warnedotyp != otmp->otyp || warnedptr != ptr)){
					warnedotyp = 0;
					warnedptr = 0;
				}
			}
		}
		
	    tmp += bonus;
	}

	if (tmp > 0) {
		/* It's debateable whether a rusted blunt instrument
		   should do less damage than a pristine one, since
		   it will hit with essentially the same impact, but
		   there ought to some penalty for using damaged gear
		   so always subtract erosion even for blunt weapons. */
		/* Rust weapons may now shatter when used, so don't subtract
		   damage for blunt anymore */
		if(!is_bludgeon(otmp)) tmp -= greatest_erosion(otmp);
		if (tmp < 1) tmp = 1;
	}
	
	if (insubstantial(ptr)){
		if(!insubstantial_aware(mon, otmp, otmp == uwep || otmp == uswapwep || otmp == uarmf))
			tmp = 0;
		else
			tmp = insubstantial_damage(mon, otmp, tmp, otmp == uwep || otmp == uswapwep || otmp == uarmf);
	}
	
	return(tmp);
}

STATIC_OVL
int
pendamage(pen, mon)
struct obj *pen;
struct monst *mon;
{
	boolean youdef = mon == &youmonst;
	int dmg = 0;
	int dnum = (Role_if(PM_EXILE) && quest_status.killed_nemesis) ? 2 : 1;
	
	if(u.specialSealsActive&SEAL_COSMOS || u.specialSealsActive&SEAL_LIVING_CRYSTAL || u.specialSealsActive&SEAL_TWO_TREES){
		if(mon->data->maligntyp == A_CHAOTIC) dmg += d(2*dnum,4);
		else if(mon->data->maligntyp == A_NEUTRAL) dmg += d(dnum,4);
	} else if(u.specialSealsActive&SEAL_MISKA){
		if(mon->data->maligntyp == A_LAWFUL) dmg += d(2*dnum,4);
		else if(mon->data->maligntyp == A_NEUTRAL) dmg += d(dnum,4);
	} else if(u.specialSealsActive&SEAL_NUDZIRATH){
		if(mon->data->maligntyp != A_NEUTRAL) dmg += d(dnum,6);
	} else if(u.specialSealsActive&SEAL_ALIGNMENT_THING){
		if(rn2(3)) dmg += d(rnd(2)*dnum,4);
	} else if(u.specialSealsActive&SEAL_UNKNOWN_GOD){
		dmg -= pen->spe;
	}
	
	if(pen->ovar1&SEAL_AMON){
		if(youdef && !Fire_resistance) dmg += d(dnum,4);
		else if(!youdef && !resists_fire(mon)) dmg += d(resists_cold(mon) ? 2*dnum : dnum,4);
	}
	if(pen->ovar1&SEAL_ASTAROTH){
		if(youdef && !Shock_resistance) dmg += d(dnum,4);
		else if(!youdef && !resists_elec(mon)) dmg += d(dnum,4);
	}
	if(pen->ovar1&SEAL_BALAM){
		if(youdef && !Cold_resistance) dmg += d(dnum,4);
		else if(!youdef && !resists_cold(mon)) dmg += d(resists_fire(mon) ? 2*dnum : dnum,4);
	}
	if(pen->ovar1&SEAL_ECHIDNA){
		if(youdef && !Acid_resistance) dmg += d(dnum,4);
		else if(!youdef && !resists_acid(mon)) dmg += d(dnum,4);
	}
	if(pen->ovar1&SEAL_FAFNIR){
		if(youdef){
			if(is_golem(youracedata)) dmg += d(2*dnum,4);
			else if(nonliving(youracedata)) dmg += d(dnum,4);
		}
		else {
			if(is_golem(mon->data)) dmg += d(2*dnum,4);
			else if(nonliving_mon(mon)) dmg += d(dnum,4);
		}
	}
	if(pen->ovar1&SEAL_IRIS){
		if(youdef && !(nonliving(youracedata) || is_anhydrous(youracedata))) dmg += d(dnum,4);
		else if(!youdef && !(nonliving_mon(mon) || is_anhydrous(mon->data))) dmg += d(dnum,4);
	}
	if(pen->ovar1&SEAL_ENKI){
		if(youdef && !(nonliving(youracedata) || amphibious(youracedata))) dmg += d(dnum,4);
		else if(!youdef && !(nonliving_mon(mon) || amphibious_mon(mon))) dmg += d(dnum,4);
	}
	if(pen->ovar1&SEAL_OSE){
		if(youdef && (Blind_telepat || !rn2(5))) dmg += d(dnum,15);
		else if(!youdef && !mindless_mon(mon) && (mon_resistance(mon,TELEPAT) || !rn2(5))) dmg += d(dnum,15);
	}
	if(pen->ovar1&SEAL_NABERIUS){
		if(youdef && (Upolyd ? u.mh < .25*u.mhmax : u.uhp < .25*u.uhpmax)) dmg += d(dnum,4);
		else if(!youdef){
			if(mon->mflee) dmg += d(dnum,4);
			if(mon->mpeaceful) dmg += d(dnum,4);
		}
	}
	if(pen->ovar1&SEAL_HUGINN_MUNINN){
		if(youdef){
			if(!Blind){
				make_blinded(Blinded+1L,FALSE);
				dmg += d(dnum,4);
			}
		}
		else if(!youdef){
			if(mon->mcansee && haseyes(mon->data)){
				dmg += d(dnum,4);
				mon->mcansee = 0;
				mon->mblinded = 1;
			}
		}
	}
	return dmg;
}

#endif /* OVLB */
#ifdef OVL0

STATIC_DCL struct obj *FDECL(oselect, (struct monst *,int,int));
STATIC_DCL struct obj *FDECL(oselectBoulder, (struct monst *));
#define Oselect(x, spot) if ((otmp = oselect(mtmp, x, spot)) != 0) return(otmp);

STATIC_OVL struct obj *
oselect(mtmp, x, spot)
struct monst *mtmp;
int x;
int spot;
{
	struct obj *otmp, *obest = 0;

	for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
	    if (otmp->otyp == x &&
		    /* never select non-cockatrice corpses */
		    !((x == CORPSE || x == EGG) &&
			!touch_petrifies(&mons[otmp->corpsenm])) &&
			/* never uncharged lightsabers */
            (!is_lightsaber(otmp) || otmp->age || otmp->oartifact == ART_INFINITY_S_MIRRORED_ARC || otmp->otyp == KAMEREL_VAJRA) &&
			/* never offhand artifacts (unless you are the Bastard) */
			(!otmp->oartifact || spot != W_SWAPWEP || mtmp->data == &mons[PM_BASTARD_OF_THE_BOREAL_VALLEY]) &&
			/* never untouchable artifacts */
		    (!otmp->oartifact || touch_artifact(otmp, mtmp, 0)) &&
			/* never unsuitable for mainhand wielding */
			(spot!=W_WEP || (!bimanual(otmp, mtmp->data) || ((mtmp->misc_worn_check & W_ARMS) == 0 && !MON_SWEP(mtmp) && strongmonst(mtmp->data)))) &&
			/* never unsuitable for offhand wielding */
			(spot!=W_SWAPWEP || (!(otmp->owornmask & (W_WEP)) && (!otmp->cursed || is_weldproof_mon(mtmp)) && !bimanual(otmp, mtmp->data) && (mtmp->misc_worn_check & W_ARMS) == 0 && 
				( (otmp->owt <= (30 + (mtmp->m_lev/5)*5)) 
				|| (otmp->otyp == CHAIN && mtmp->data == &mons[PM_CATHEZAR]) 
				|| (mtmp->data == &mons[PM_BASTARD_OF_THE_BOREAL_VALLEY])
				)
			)) &&
			/* never a hated weapon */
			(mtmp->misc_worn_check & W_ARMG || !hates_silver(mtmp->data) || otmp->obj_material != SILVER) &&
			(mtmp->misc_worn_check & W_ARMG || !hates_iron(mtmp->data)   || otmp->obj_material != IRON) &&
			(mtmp->misc_worn_check & W_ARMG || !hates_unholy_mon(mtmp) || !is_unholy(otmp))
		){
			if (!obest ||
				(dmgval(otmp, 0 /*zeromonst*/, 0) > dmgval(obest, 0 /*zeromonst*/,0))
				/*
				(is_bludgeon(otmp) ? 
					(otmp->spe - greatest_erosion(otmp) > obest->spe - greatest_erosion(obest)):
					(otmp->spe > obest->spe)
				)
				*/
			) obest = otmp;
		}
	}
	return obest;
}

STATIC_OVL struct obj *
oselectBoulder(mtmp)
struct monst *mtmp;
{
	struct obj *otmp, *obest = 0;

	for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj) {
	    if (is_boulder(otmp)  &&
		    (!otmp->oartifact || touch_artifact(otmp, mtmp, FALSE)))
            {
	        if (!obest ||
		    dmgval(otmp, 0 /*zeromonst*/, 0) > dmgval(obest, 0 /*zeromonst*/,0))
		    obest = otmp;
		}
	}
	return obest;
}

static NEARDATA const int rwep[] =
{	LOADSTONE/*1d30*/, 
// #ifdef FIREARMS
	FRAG_GRENADE, GAS_GRENADE, ROCKET, SILVER_BULLET, BULLET, SHOTGUN_SHELL,
// #endif
	DROVEN_BOLT/*1d9+1/1d6+1*/, 
	DWARVISH_SPEAR/*1d9/1d9*/, 
	SHURIKEN/*1d8/1d6*/, 
	DROVEN_DAGGER/*1d8/1d6*/, 
	YA,/*1d7/1d7*/ 
	ELVEN_ARROW/*1d7/1d5*/, 
	ELVEN_SPEAR/*1d7/1d7*/, 
	JAVELIN/*1d6/1d6*/, 
	SILVER_ARROW/*1d6/1d6*/, 
	SPEAR/*1d6/1d8*/, 
	ARROW/*1d6/1d6*/,
	FLINT/*1d6/1d6*/, 
	LUCKSTONE/*1d3/1d3*/, /*Because they think it's flint*/
	TOUCHSTONE/*1d3/1d3*/, 
	ORCISH_SPEAR/*1d5/1d10*/,
	CROSSBOW_BOLT/*1d4+1/1d6+1*/, 
	ORCISH_ARROW/*1d5/1d8*/, 
	ELVEN_DAGGER/*1d5/1d3*/, 
	DAGGER/*1d4/1d3*/,
	ORCISH_DAGGER/*1d3/1d5*/, 
	ROCK/*1d3/1d3*/, 
	DART/*1d3/1d2*/,
	KNIFE/*1d3/1d2*/, 
	/* BOOMERANG, */ 
	CREAM_PIE/*0*/
};

static NEARDATA const int pwep[] =
{	HALBERD, 
	BARDICHE, 
	SPETUM, 
	BILL_GUISARME, 
	VOULGE, 
	DROVEN_LANCE,
	RANSEUR, 
	GUISARME,
	NAGINATA, 
	GLAIVE, 
	LUCERN_HAMMER, 
	BEC_DE_CORBIN, 
	FAUCHARD, 
	ELVEN_LANCE,
	PARTISAN, 
	LANCE,
	AKLYS
};

boolean
would_prefer_rwep(mtmp, otmp)
struct monst *mtmp;
struct obj *otmp;
{
    struct obj *wep = select_rwep(mtmp);

    int i = 0;
    
    if (wep)
    {
        if (wep == otmp) return TRUE;
		
        if (wep->oartifact) return FALSE;

        if (throws_rocks(mtmp->data) &&  is_boulder(wep)) return FALSE;
        if (throws_rocks(mtmp->data) && is_boulder(otmp)) return TRUE;
		
		if(wep->otyp == otmp->otyp) return dmgval(otmp, 0 /*zeromonst*/, 0) > dmgval(wep, 0 /*zeromonst*/, 0);
    }
    
    if (((strongmonst(mtmp->data) && (mtmp->misc_worn_check & W_ARMS) == 0) || !bimanual(otmp,mtmp->data)) && 
		(mtmp->misc_worn_check & W_ARMG || otmp->obj_material != SILVER || !hates_silver(mtmp->data)) &&
		(mtmp->misc_worn_check & W_ARMG || otmp->obj_material != IRON	|| !hates_iron(mtmp->data)) &&
		(mtmp->misc_worn_check & W_ARMG || is_unholy(otmp)				|| !hates_unholy_mon(mtmp))
	){
        for (i = 0; i < SIZE(pwep); i++)
        {
            if ( wep &&
	         wep->otyp == pwep[i])
					return FALSE;
            if (otmp->otyp == pwep[i]) return TRUE;
        }
    }

    if (is_pole(otmp) && otmp->otyp != AKLYS) return FALSE; /* If we get this far,
                                        we failed the polearm strength check */

    for (i = 0; i < SIZE(rwep); i++)
    {
        if ( wep &&
             wep->otyp == rwep[i] &&
           !(otmp->otyp == rwep[i] &&
	     (dmgval(otmp, 0 /*zeromonst*/, 0) > dmgval(wep, 0 /*zeromonst*/, 0))))
	    return FALSE;
        if (otmp->otyp == rwep[i]) return TRUE;
    }

    return FALSE;
}

struct obj *propellor;

struct obj *
select_rwep(mtmp)	/* select a ranged weapon for the monster */
register struct monst *mtmp;
{
	register struct obj *otmp;
	int i;

	struct obj *tmpprop = &zeroobj;

	char mlet = mtmp->data->mlet;
	
	propellor = &zeroobj;
	Oselect(EGG, W_QUIVER); /* cockatrice egg */
	if(throws_rocks(mtmp->data))	/* ...boulders for giants */
	    oselectBoulder(mtmp);

	/* Select polearms first; they do more damage and aren't expendable */
	/* The limit of 13 here is based on the monster polearm range limit
	 * (defined as 5 in mthrowu.c).  5 corresponds to a distance of 2 in
	 * one direction and 1 in another; one space beyond that would be 3 in
	 * one direction and 2 in another; 3^2+2^2=13.
	 */
	/* This check is disabled, as it's targeted towards attacking you
	   and not any arbitrary target. */
	/* if (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 13 && couldsee(mtmp->mx, mtmp->my)) */
	{
	    for (i = 0; i < SIZE(pwep); i++) {
		if ((otmp = oselect(mtmp, pwep[i], W_WEP)) != 0) {
			propellor = otmp; /* force the monster to wield it */
			return otmp;
			}
	    }
	}
	//Check for charged arm blasters or hand blasters
	if(mtmp->data->msize == MZ_HUMAN && (propellor = m_carrying_charged(mtmp, ARM_BLASTER)) && !(
		((otmp = MON_WEP(mtmp)) && otmp->cursed && otmp != propellor && mtmp->weapon_check == NO_WEAPON_WANTED)// || (mtmp->combat_mode == HNDHND_MODE)
	)){
		return propellor;
	} else if(!bigmonst(mtmp->data) && (propellor = m_carrying_charged(mtmp, HAND_BLASTER)) && !(
		((otmp = MON_WEP(mtmp)) && otmp->cursed && otmp != propellor && mtmp->weapon_check == NO_WEAPON_WANTED)// || (mtmp->combat_mode == HNDHND_MODE)
	)){
		return propellor;
	}

	/*
	 * other than these two specific cases, always select the
	 * most potent ranged weapon to hand.
	 */
	for (i = 0; i < SIZE(rwep); i++) {
	    int prop;

	    /* shooting gems from slings; this goes just before the darts */
	    /* (shooting rocks is already handled via the rwep[] ordering) */
	    if (rwep[i] == DART && !likes_gems(mtmp->data) &&
		    m_carrying(mtmp, SLING)) {		/* propellor */
			for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
				if (otmp->oclass == GEM_CLASS &&
					(otmp->otyp != LOADSTONE || !otmp->cursed)) {
				propellor = m_carrying(mtmp, SLING);
				return otmp;
				}
	    }

		/* KMH -- This belongs here so darts will work */
		propellor = &zeroobj;

	    prop = (objects[rwep[i]]).oc_skill;
	    if (prop < 0) {
		switch (-prop) {
		case P_BOW:
		  propellor = (oselect(mtmp, YUMI, W_WEP));
		  if (!propellor) propellor = (oselect(mtmp, ELVEN_BOW, W_WEP));
		  if (!propellor) propellor = (oselect(mtmp, BOW, W_WEP));
		  if (!propellor) propellor = (oselect(mtmp, ORCISH_BOW, W_WEP));
		  break;
		case P_SLING:
		  propellor = (oselect(mtmp, SLING, W_WEP));
		  break;
//ifdef FIREARMS
		case P_FIREARM:
		  if ((objects[rwep[i]].w_ammotyp) == WP_BULLET) {
			propellor = (oselect(mtmp, BFG, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, HEAVY_MACHINE_GUN, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, ASSAULT_RIFLE, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, SUBMACHINE_GUN, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, SNIPER_RIFLE, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, RIFLE, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, PISTOL, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, FLINTLOCK, W_WEP));
		  } else if ((objects[rwep[i]].w_ammotyp) == WP_SHELL) {
			propellor = (oselect(mtmp, BFG, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, AUTO_SHOTGUN, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, SHOTGUN, W_WEP));
		  } else if ((objects[rwep[i]].w_ammotyp) == WP_ROCKET) {
			propellor = (oselect(mtmp, BFG, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, ROCKET_LAUNCHER, W_WEP));
		  } else if ((objects[rwep[i]].w_ammotyp) == WP_GRENADE) {
			propellor = (oselect(mtmp, BFG, W_WEP));
			if (!propellor) propellor = (oselect(mtmp, GRENADE_LAUNCHER, W_WEP));
			if (!propellor) propellor = &zeroobj;  /* can toss grenades */
		  }
		break;
//endif
		case P_CROSSBOW:
		  propellor = (oselect(mtmp, DROVEN_CROSSBOW, W_WEP));
		  if (!propellor) propellor = (oselect(mtmp, CROSSBOW, W_WEP));
		}
		if (!tmpprop) tmpprop = propellor;
		if (((otmp = MON_WEP(mtmp)) && otmp->cursed && otmp != propellor
				&& mtmp->weapon_check == NO_WEAPON_WANTED))// || (mtmp->combat_mode == HNDHND_MODE))
			propellor = 0;
	    }
	    /* propellor = obj, propellor to use
	     * propellor = &zeroobj, doesn't need a propellor
	     * propellor = 0, needed one and didn't have one
	     */
	    if (propellor != 0) {
			/* Note: cannot use m_carrying for loadstones, since it will
			 * always select the first object of a type, and maybe the
			 * monster is carrying two but only the first is unthrowable.
			 */
			if (rwep[i] != LOADSTONE) {
				/* Don't throw a cursed weapon-in-hand or an artifact */
				/* nor throw the last one (for stacks) of a wielded weapon */
				if ((otmp = oselect(mtmp, rwep[i], W_QUIVER))
					&& !otmp->oartifact
					&& (!otmp->cursed || otmp != MON_WEP(mtmp))
					&& !((otmp->quan == 1) && (otmp->owornmask & (W_WEP|W_SWAPWEP))))
					return(otmp);
			} else for(otmp=mtmp->minvent; otmp; otmp=otmp->nobj) {
				if (otmp->otyp == LOADSTONE && !otmp->cursed)
				return otmp;
			}
	    }
	}

	/* failure */
	if (tmpprop) propellor = tmpprop;
	return (struct obj *)0;
}

/* Weapons in order of preference */
static const NEARDATA short hwep[] = {
	  CORPSE,  /* cockatrice corpse */
	  KAMEREL_VAJRA /*quite a lot plus elect plus blindness*/,
	  GOLD_BLADED_VIBROZANBATO,/*2d16+8/2d8+4d6+10*/
	  WHITE_VIBROZANBATO,/*2d16+8/2d8+4d6+10*/
	  DOUBLE_FORCE_BLADE,/*6d6+12/6d4+8*/
	  DOUBLE_LIGHTSABER/*6d8*/, 
	  RED_EYED_VIBROSWORD,/*3d8+8/3d12+12*/
	  FORCE_SWORD,/*3d8+8/3d6+6*/
	  FORCE_WHIP,/*3d6+6+3/3d4+6+3d4+4*/
	  FORCE_PIKE,/*3d6+6/3d8+8*/
	  FORCE_BLADE,/*3d6+6/3d4+4*/
	  BEAMSWORD/*3d10*/,
	  GOLD_BLADED_VIBROSWORD,/*2d8+4/2d12+6*/
	  WHITE_VIBROSWORD,/*2d8+4/2d12+6*/
	  GOLD_BLADED_VIBROSPEAR,/*2d6+3/2d8+3*/
	  WHITE_VIBROSPEAR,/*2d6+3/2d8+3*/
	  LIGHTSABER/*3d8*/,
	  MIRRORBLADE/*your weapon is probably pretty darn good*/,
	  HEAVY_IRON_BALL,/*1d25/1d25*/
	  VIBROBLADE,/*2d6+3/2d8+4*/
	  CRYSTAL_SWORD/*2d8/2d12*/,
	  DROVEN_GREATSWORD/*1d18/1d30*/, 
	  TSURUGI/*1d16/1d8+2d6*/, 
	  MOON_AXE/*variable, 2d6 to 2d12*/,
	  RUNESWORD/*1d10+1d4/1d10+1*/, 
	  BATTLE_AXE/*1d8+1d4/1d6+2d4*/,
	  TWO_HANDED_SWORD/*1d12/3d6*/, 
	  DROVEN_SPEAR/*1d12/1d12*/,
	  UNICORN_HORN/*1d12/1d12*/,
	  DWARVISH_MATTOCK/*1d12/1d8*/, 
	  RAKUYO/*1d8+1d4/1d8+1d3*/, 
	  ELVEN_BROADSWORD/*1d6+1d4/1d6+2*/, 
	  KATANA/*1d10/1d12*/,
	  HIGH_ELVEN_WARSWORD/*1d10/1d10*/,
	  CRYSKNIFE/*1d10/1d10*/, 
	  VIPERWHIP/*2d4/2d3/poison*/,
	  DROVEN_SHORT_SWORD/*1d9/1d9*/, 
	  DWARVISH_SPEAR/*1d9/1d9*/, 
	  SCYTHE/*2d4*/, 
	  BROADSWORD/*2d4/1d6+1*/, 
	  MORNING_STAR/*2d4/1d6+1*/, 
	  RAKUYO_SABER/*1d8/1d8*/,
	  SABER/*1d8/1d8*/,
	  TRIDENT/*1d6+1/3d4*/, 
	  LONG_SWORD/*1d8/1d12*/,
	  FLAIL/*1d6+1/2d4*/, 
	  NAGINATA/*1d6+1/2d4*/, 
	  SCIMITAR/*1d8/1d8*/,
	  DWARVISH_SHORT_SWORD/*1d8/1d7*/, 
	  DROVEN_DAGGER/*1d8/1d6*/, 
	  MACE/*1d6+1/1d6*/, 
	  ELVEN_SHORT_SWORD/*1d7/1d7*/, 
	  ELVEN_MACE/*1d7/1d7*/, 
	  ELVEN_SPEAR/*1d7/1d7*/, 
	  SPEAR/*1d6/1d8*/,
	  SHORT_SWORD/*1d6/1d8*/,
	  RAPIER/*1d6/1d4*/, 
	  AXE/*1d6/1d4*/, 
	  ORCISH_SHORT_SWORD/*1d5/1d10*/, 
	  ORCISH_SPEAR/*1d5/1d10*/, 
	  KHAKKHARA/*1d6/1d4*/,
	  BULLWHIP/*1d2/1d1*/, 
	  QUARTERSTAFF/*1d6/1d6*/,
	  JAVELIN/*1d6/1d6*/, 
	  CHAIN/*1d6/1d6*/, 
	  WAR_HAMMER/*1d4+1/1d4*/, 
	  AKLYS/*1d6/1d3*/, 
	  SUNROD/*1d6/1d3*/, 
	  SHADOWLANDER_S_TORCH/*1d6/1d3*/, 
	  TORCH/*1d6/1d3*/, 
	  CLUB/*1d6/1d3*/, 
	  PICK_AXE/*1d6/1d3*/,
	  ELVEN_SICKLE/*1d6/1d3*/,
	  STILETTO/*1d6/1d2*/, 
	  ELVEN_DAGGER/*1d5/1d3*/, 
	  ATHAME/*1d4/1d4*/, 
	  RAKUYO_DAGGER/*1d4/1d3*/, 
	  DAGGER/*1d4/1d3*/, 
	  SICKLE/*1d4/1d1*/, 
	  ORCISH_DAGGER/*1d3/1d5*/,
	  KNIFE/*1d3/1d2*/, 
	  SCALPEL/*1d3/1d1*/, 
	  WORM_TOOTH/*1d2/1d2*/
	};

boolean
would_prefer_hwep(mtmp, otmp)
struct monst *mtmp;
struct obj *otmp;
{
    struct obj *wep = select_hwep(mtmp);

    int i = 0;
    
    if (wep)
    { 
        if (wep == otmp) return TRUE;
    
        if (wep->oartifact) return FALSE;
		
        if (wep->oproperties && !otmp->oproperties) return FALSE;

        if (is_giant(mtmp->data) &&  wep->otyp == CLUB) return FALSE;
        if (is_giant(mtmp->data) && otmp->otyp == CLUB) return TRUE;
    }
    
    for (i = 0; i < SIZE(hwep); i++)
    {
	if (hwep[i] == CORPSE && !(mtmp->misc_worn_check & W_ARMG))
	    continue;

        if ( wep &&
	     wep->otyp == hwep[i] &&
           !(otmp->otyp == hwep[i] &&
	     dmgval(otmp, 0 /*zeromonst*/, 0) > dmgval(wep, 0 /*zeromonst*/, 0)))
	    return FALSE;
        if (otmp->otyp == hwep[i]) return TRUE;
    }

    return FALSE;
}

struct obj *
select_hwep(mtmp)	/* select a hand to hand weapon for the monster */
register struct monst *mtmp;
{
	register struct obj *otmp;
	register int i;
	boolean strong = strongmonst(mtmp->data);
	boolean wearing_shield = (mtmp->misc_worn_check & W_ARMS) != 0;

	/* prefer artifacts to everything else */
	for(otmp=mtmp->minvent; otmp; otmp = otmp->nobj) {
		if (/* valid weapon */
			(otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
			|| otmp->otyp == CHAIN || otmp->otyp == HEAVY_IRON_BALL
			) &&
			/* an artifact or other special weapon*/
			(otmp->oartifact || otmp->oproperties) &&
			/* never uncharged lightsabers */
            (!is_lightsaber(otmp) || otmp->age
			 || otmp->oartifact == ART_INFINITY_S_MIRRORED_ARC
			 || otmp->otyp == KAMEREL_VAJRA
            ) &&
			/* never untouchable artifacts */
			(touch_artifact(otmp, mtmp, 0)) &&
			/* never too-large for available hands */
			(!bimanual(otmp, mtmp->data) || ((mtmp->misc_worn_check & W_ARMS) == 0 && strongmonst(mtmp->data))) &&
			/* never a hated weapon */
			(mtmp->misc_worn_check & W_ARMG || !hates_silver(mtmp->data) || otmp->obj_material != SILVER) &&
			(mtmp->misc_worn_check & W_ARMG || !hates_iron(mtmp->data) || otmp->obj_material != IRON) &&
			(mtmp->misc_worn_check & W_ARMG || !hates_unholy_mon(mtmp) || !is_unholy(otmp))
			) return otmp;
	}

	if(is_giant(mtmp->data))	/* giants just love to use clubs */
		Oselect(CLUB, W_WEP);

	for (i = 0; i < SIZE(hwep); i++) {
	    if (hwep[i] == CORPSE && !(mtmp->misc_worn_check & W_ARMG))
		continue;
		Oselect(hwep[i], W_WEP);
	}

	/* failure */
	return (struct obj *)0;
}

struct obj *
select_shwep(mtmp)	/* select an offhand hand to hand weapon for the monster */
register struct monst *mtmp;
{
	register struct obj *otmp;
	register int i;
	boolean strong = strongmonst(mtmp->data);
	boolean wearing_shield = (mtmp->misc_worn_check & W_ARMS) != 0;

	if(is_giant(mtmp->data))	/* giants just love to use clubs */
		Oselect(CLUB, W_SWAPWEP);

	for (i = 0; i < SIZE(hwep); i++) {
	    if (hwep[i] == CORPSE && !(mtmp->misc_worn_check & W_ARMG))
		continue;
		Oselect(hwep[i], W_SWAPWEP);
	}

	/* failure */
	return (struct obj *)0;
}
struct obj *
select_pick(mtmp)
struct monst *mtmp;
{
	struct obj * otmp;

	/* preference to any artifacts (and checks for arti_digs) */
	for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj){
		if (is_pick(otmp) && otmp->oartifact)
			Oselect(otmp->otyp, W_WEP);
	}

	Oselect(DWARVISH_MATTOCK, W_WEP);
	Oselect(PICK_AXE, W_WEP);
	/* failure */
	return (struct obj *)0;
}
struct obj *
select_axe(mtmp)
struct monst *mtmp;
{
	struct obj * otmp;

	/* preference to any artifacts */
	for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj){
		if (is_axe(otmp) && otmp->oartifact)
			Oselect(otmp->otyp, W_WEP);
	}

	Oselect(MOON_AXE, W_WEP);
	Oselect(BATTLE_AXE, W_WEP);
	Oselect(AXE, W_WEP);
	/* failure */
	return (struct obj *)0;
}

/* Called after polymorphing a monster, robbing it, etc....  Monsters
 * otherwise never unwield stuff on their own.  Might print message.
 */
void
possibly_unwield(mon, polyspot)
struct monst *mon;
boolean polyspot;
{
	struct obj *obj, *mw_tmp;
	struct obj *sobj, *msw_tmp;
	mw_tmp = MON_WEP(mon);
	msw_tmp = MON_SWEP(mon);
	if (!(mw_tmp || msw_tmp))
		return;
	for (obj = mon->minvent; obj; obj = obj->nobj)
		if (obj == mw_tmp) break;
	for (sobj = mon->minvent; sobj; sobj = sobj->nobj)
		if (sobj == msw_tmp) break;

	if (mw_tmp && !obj) { /* The mainhand weapon was stolen or destroyed */
		MON_NOWEP(mon);
		mon->weapon_check = NEED_WEAPON;
	}
	if (msw_tmp && !sobj) { /* The offhand weapon was stolen or destroyed */
		MON_NOSWEP(mon);
		mon->weapon_check = NEED_WEAPON;
	}

	/* monster can no longer wield any weapons */
	if (!attacktype(mon->data, AT_WEAP)) {
		if (mw_tmp) {
			setmnotwielded(mon, mw_tmp);
			MON_NOWEP(mon);
			if (obj)
				obj_extract_self(obj);
			if (cansee(mon->mx, mon->my)) {
				pline("%s drops %s.", Monnam(mon),
					distant_name(obj, doname));
				newsym(mon->mx, mon->my);
			}
			/* might be dropping object into water or lava */
			if (!flooreffects(obj, mon->mx, mon->my, "drop")) {
				if (polyspot) bypass_obj(obj);
				place_object(obj, mon->mx, mon->my);
				stackobj(obj);
			}
		}
		/* can't wield any weapons -> none wanted */
		mon->weapon_check = NO_WEAPON_WANTED;
	}
	/* monster can no longer twoweapon */
	if (!could_twoweap(mon->data))
	{
		if (msw_tmp) {
			setmnotwielded(mon, msw_tmp);
			MON_NOSWEP(mon);
			if (sobj)
				obj_extract_self(sobj);
			if (cansee(mon->mx, mon->my)) {
				pline("%s drops %s.", Monnam(mon),
					distant_name(sobj, doname));
				newsym(mon->mx, mon->my);
			}
			/* might be dropping object into water or lava */
			if (!flooreffects(sobj, mon->mx, mon->my, "drop")) {
				if (polyspot) bypass_obj(sobj);
				place_object(sobj, mon->mx, mon->my);
				stackobj(sobj);
			}
		}
		/* Do not set weapon_check */
		/* We may still want a mainhand weapon. Or not. */
	}
	/* The remaining case where there is a change is where a monster
	 * is polymorphed into a stronger/weaker monster with a different
	 * choice of weapons.  This has no parallel for players.  It can
	 * be handled by waiting until mon_wield_item is actually called.
	 * Though the monster still wields the wrong weapon until then,
	 * this is OK since the player can't see it.  (FIXME: Not okay since
	 * probing can reveal it.)
	 * Note that if there is no change, setting the check to NEED_WEAPON
	 * is harmless.
	 * Possible problem: big monster with big cursed weapon gets
	 * polymorphed into little monster.  But it's not quite clear how to
	 * handle this anyway....
	 */
	if (!(mw_tmp && (mw_tmp->cursed && !is_weldproof_mon(mon)) && mon->weapon_check == NO_WEAPON_WANTED))
	    mon->weapon_check = NEED_WEAPON;
	return;
}

/* Let a monster try to wield a weapon, based on mon->weapon_check.
 * Returns 1 if the monster took time to do it, 0 if it did not.
 */
int
mon_wield_item(mon)
register struct monst *mon;
{
	struct obj *obj = 0;
	struct obj *sobj = 0;
	struct obj *mw_tmp = MON_WEP(mon);
	struct obj *msw_tmp = MON_SWEP(mon);
	xchar old_weapon_check = mon->weapon_check;
	boolean time_taken = FALSE;
	boolean toreturn = FALSE;

	/* This case actually should never happen */
	if (mon->weapon_check == NO_WEAPON_WANTED) return 0;
	
	/* Most monsters are able to remember that they wielded a cursed weapon */
	if (mw_tmp && mw_tmp->cursed && mw_tmp->otyp != CORPSE && mon->m_lev > 1) {
		mon->weapon_check = NO_WEAPON_WANTED;
		return 0;
	}
	/* Pick weapon to attempt to wield */
	switch(mon->weapon_check) {
		case NEED_HTH_WEAPON:
			obj = select_hwep(mon);
			break;
		case NEED_RANGED_WEAPON:
			(void)select_rwep(mon);
			obj = propellor;
			break;
		case NEED_PICK_AXE:
			obj = select_pick(mon);
			break;
		case NEED_AXE:
			obj = select_axe(mon);
			break;
		case NEED_PICK_OR_AXE:
			/* prefer pick for fewer switches on most levels */
			obj = select_pick(mon);
			if (!obj)
				obj = select_axe(mon);
			break;
		default: impossible("weapon_check %d for %s?",
				mon->weapon_check, mon_nam(mon));
			return 0;
	}

	/* reset mw_tmp to be sure */
	mw_tmp = MON_WEP(mon);

	/* attempt to wield weapon */
	if (obj && obj != &zeroobj) {
		if (mw_tmp && mw_tmp->otyp == obj->otyp) {
			/* already wielding one similar to it */
			if (is_lightsaber(obj))
				mon_ignite_lightsaber(obj, mon);
			mon->weapon_check = NEED_WEAPON;
			toreturn = TRUE;
		}
		else {
			/* Actually, this isn't necessary--as soon as the monster
			* wields the weapon, the weapon welds itself, so the monster
			* can know it's cursed and needn't even bother trying.
			* Still....
			*/
			if (mw_tmp && mw_tmp->cursed && !is_weldproof_mon(mon) && mw_tmp->otyp != CORPSE) {
				/* the monster's weapon is visibly wielded to it's hand */
				if (canseemon(mon)) {
					char welded_buf[BUFSZ];
					const char *mon_hand = mbodypart(mon, HAND);

					if (bimanual(mw_tmp,mon->data)) mon_hand = makeplural(mon_hand);
					Sprintf(welded_buf, "%s welded to %s %s",
						otense(mw_tmp, "are"),
						mhis(mon), mon_hand);
					if (obj->otyp == PICK_AXE) {
						pline("Since %s weapon%s %s,",
							s_suffix(mon_nam(mon)),
							plur(mw_tmp->quan), welded_buf);
						pline("%s cannot wield that %s.",
							mon_nam(mon), xname(obj));
					}
					else {
						pline("%s tries to wield %s.", Monnam(mon),
							doname(obj));
						pline("%s %s %s!",
							s_suffix(Monnam(mon)),
							xname(mw_tmp), welded_buf);
					}
					mw_tmp->bknown = 1;
				}
				/* the monster will now not try that again */
				mon->weapon_check = NO_WEAPON_WANTED;
				/* that took time */
				time_taken = TRUE;
				toreturn = TRUE;
			}
			else
			{
				/* unwield old object */
				setmnotwielded(mon, mw_tmp);
				/* wield the new object*/
				mon->weapon_check = NEED_WEAPON;
				setmwielded(mon, obj, W_WEP);
				/* that took time */
				time_taken = TRUE;
				toreturn = TRUE;
			}
		}
	}
	/* update msw_tmp, since it could have been wielded in the mainhand (above) */
	msw_tmp = MON_SWEP(mon);

	/* possibly wield an off-hand weapon */
	if (old_weapon_check == NEED_HTH_WEAPON)
	{
		if (could_twoweap(mon->data) && !which_armor(mon, W_ARMS) && !bimanual(MON_WEP(mon), mon->data))
		{
			sobj = select_shwep(mon);
			if (sobj && sobj != &zeroobj) {
				if (msw_tmp && msw_tmp->otyp == sobj->otyp) {
					/* already wielding one similar to it */
					if (is_lightsaber(sobj))
						mon_ignite_lightsaber(sobj, mon);
					mon->weapon_check = NEED_WEAPON;
					toreturn = TRUE;
				}
				else {
					/* already-wielding-a-cursed-item check unnecessary here */
					/* unwield old object */
					setmnotwielded(mon, msw_tmp);
					/* wield the new object*/
					mon->weapon_check = NEED_WEAPON;
					setmwielded(mon, sobj, W_SWAPWEP);
					/* that took time */
					time_taken = TRUE;
					toreturn = TRUE;
				}
			}
		}
	}
	if (toreturn)
		return time_taken;

	mon->weapon_check = NEED_WEAPON;
	return 0;
}

void
setmwielded(mon, obj, slot)
register struct monst *mon;
register struct obj *obj;
long slot;
{
	if (!obj) return;
	/* silently unwield the weapon if it's already wielded */
	if (MON_WEP(mon) == obj)
		MON_NOWEP(mon);
	else if (MON_SWEP(mon) == obj)
		MON_NOSWEP(mon);
	if (obj->owornmask)
		obj->owornmask = 0;

	/* visible effects */
	if (canseemon(mon)) {
		pline("%s %s %s%s",
			Monnam(mon),
			(slot == W_WEP) ? "wields" : (slot == W_SWAPWEP) ? "offhands" : "telekinetically brandishes",
			doname(obj),
			mon->mtame ? "." : "!");
		if (obj->cursed && !is_weldproof_mon(mon) && obj->otyp != CORPSE) {
			pline("%s %s to %s %s!",
				Tobjnam(obj, "weld"),
				is_plural(obj) ? "themselves" : "itself",
				s_suffix(mon_nam(mon)), mbodypart(mon, HAND));
			obj->bknown = 1;
		}
	}
	if (artifact_light(obj) && !obj->lamplit) {
		begin_burn(obj, FALSE);
		if (canseemon(mon))
			pline("%s %s%s in %s %s!",
			Tobjnam(obj, (obj->blessed ? "shine" : "glow")),
			(obj->blessed ? " very" : ""),
			(obj->cursed ? "" : " brilliantly"),
			s_suffix(mon_nam(mon)), mbodypart(mon, HAND));
	}
	/* set pointers, masks, and do other backend */
	if (slot == W_WEP)
		MON_WEP(mon) = obj;
	else if (slot == W_SWAPWEP)
		MON_SWEP(mon) = obj;
	else
		impossible("bad slot for weapon (%s)[%d]!", xname(obj), (int)slot);
	obj->owornmask = slot;
	update_mon_intrinsics(mon, obj, TRUE, FALSE);
	/* ignite lightsabers*/
	if (is_lightsaber(obj))
		mon_ignite_lightsaber(obj, mon);
}

void
setmnotwielded(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	if (!obj) return;
	/* visible effects */
	if (artifact_light(obj) && obj->lamplit) {
		end_burn(obj, FALSE);
		if (canseemon(mon))
			pline("%s in %s %s %s glowing.", The(xname(obj)),
			s_suffix(mon_nam(mon)), mbodypart(mon, HAND),
			otense(obj, "stop"));
	}
	/* set pointers, masks, and do other backend */
	if (MON_WEP(mon) == obj)
		MON_NOWEP(mon);
	if (MON_SWEP(mon) == obj)
		MON_NOSWEP(mon);
	obj->owornmask &= ~W_WEP;
	obj->owornmask &= ~W_SWAPWEP;
	update_mon_intrinsics(mon, obj, FALSE, FALSE);
}

void
init_mon_wield_item(mon)
register struct monst *mon;
{
	struct obj *obj = 0;
	struct obj *sobj = 0;
	struct obj *mw_tmp = MON_WEP(mon);
	struct obj *msw_tmp = MON_SWEP(mon);
	int toreturn = 0;
	
	if(!attacktype(mon->data, AT_WEAP)) return;

	if(needspick(mon->data)){
		obj = m_carrying(mon, DWARVISH_MATTOCK);
		if (!obj || bimanual(obj, mon->data))
			obj = m_carrying(mon, PICK_AXE);
	}
	if(!obj){
		(void)select_rwep(mon);
		if(propellor != &zeroobj) obj = propellor;
	}
	if(!obj){
		obj = select_hwep(mon);
	}
	if (obj && obj != &zeroobj) {
		mon->mw = obj;		/* wield obj */
		setmnotwielded(mon, mw_tmp);
		mon->weapon_check = NEED_WEAPON;
		if (artifact_light(obj) && !obj->lamplit) {
		    begin_burn(obj, FALSE);
		}
		obj->owornmask = W_WEP;
		update_mon_intrinsics(mon, obj, TRUE, FALSE);
		if (is_lightsaber(obj))
		    mon_ignite_lightsaber(obj, mon);
		toreturn = 1;
	}
	if (could_twoweap(mon->data) && !which_armor(mon, W_ARMS) && !bimanual(obj, mon->data))
	{
		sobj = select_shwep(mon);
		if (sobj && sobj != &zeroobj) {
			mon->msw = sobj;		/* wield sobj */
			setmnotwielded(mon, msw_tmp);
			mon->weapon_check = NEED_WEAPON;
			if (artifact_light(sobj) && !sobj->lamplit) {
				begin_burn(sobj, FALSE);
			}
			sobj->owornmask = W_SWAPWEP;
			update_mon_intrinsics(mon, sobj, TRUE, FALSE);
			if (is_lightsaber(sobj))
				mon_ignite_lightsaber(sobj, mon);
			toreturn = 1;
		}
	}
	if (toreturn)
		return;
	mon->weapon_check = NEED_WEAPON;
	return;
}

static void
mon_ignite_lightsaber(obj, mon)
struct obj * obj;
struct monst * mon;
{
	/* No obj or not lightsaber or unlightable */
	if (!obj || !is_lightsaber(obj) || obj->oartifact == ART_INFINITY_S_MIRRORED_ARC || obj->otyp == KAMEREL_VAJRA) return;

	/* WAC - Check lightsaber is on */
	if (!obj->lamplit) {
	    if (obj->cursed && !rn2(2)) {
		if (canseemon(mon)) pline("%s %s flickers and goes out.", 
			s_suffix(Monnam(mon)), xname(obj));

	    } else {
		if (canseemon(mon)) {
			makeknown(obj->otyp);
			pline("%s ignites %s.", Monnam(mon),
				an(xname(obj)));
		}	    	
		begin_burn(obj, FALSE);
	    }
	} else {
		/* Double Lightsaber in single mode? Ignite second blade */
		if (is_lightsaber(obj) && 
			(obj->otyp == DOUBLE_LIGHTSABER || obj->oartifact == ART_ANNULUS) && 
			!obj->altmode
		) {
		    /* Do we want to activate dual bladed mode? */
		    if (!obj->altmode && (!obj->cursed || rn2(4))) {
			if (canseemon(mon)) pline("%s ignites the second blade of %s.", 
				Monnam(mon), an(xname(obj)));
		    	obj->altmode = TRUE;
		    	return;
		    } else obj->altmode = FALSE;
		    lightsaber_deactivate(obj, TRUE);
		}
		return;
	}
}
int
abon()		/* attack bonus for strength & dexterity */
{
	int sbon;
	int str = ACURR(A_STR), dex = ACURR(A_DEX);

	if (Upolyd) return(adj_lev(&mons[u.umonnum]) - 3);
	if (str < 6) sbon = -2;
	else if (str < 8) sbon = -1;
	else if (str < 17) sbon = 0;
	else if (str <= STR18(50)) sbon = 1;	/* up to 18/50 */
	else if (str < STR18(100)) sbon = 2;
	else sbon = 3;

	if (dex < 4) return(sbon-3);
	else if (dex < 6) return(sbon-2);
	else if (dex < 8) return(sbon-1);
	else if (dex < 14) return(sbon);
	else return(sbon + dex-14);
}

#endif /* OVL0 */
#ifdef OVL1

int
dbon(otmp)		/* damage bonus for strength */
struct obj *otmp;
{
	int str = ACURR(A_STR);
	int bonus = 0;
	
	
	
	// if (Upolyd || otmp == uswapwep) return(0);
	if (uswapwep && otmp == uswapwep) return (str < 6) ? (-6+str) : 0;
	
	if (str < 6) bonus = -6+str;
	else if (str < 16) bonus = 0;
	else if (str < 18) bonus = 1;
	else if (str == 18) bonus = 2;		/* up to 18 */
	else if (str <= STR18(75)) bonus = 3;		/* up to 18/75 */
	else if (str <= STR18(90)) bonus = 4;		/* up to 18/90 */
	else if (str < STR18(100)) bonus = 5;		/* up to 18/99 */
	else if (str < STR19(22)) bonus = 6;
	else if (str < STR19(25)) bonus = 7;
	else /*  str ==25*/bonus = 8;
	
	if(otmp){
		if((bimanual(otmp,youracedata) ||
			(otmp->oartifact==ART_PEN_OF_THE_VOID && otmp->ovar1&SEAL_MARIONETTE && mvitals[PM_ACERERAK].died > 0)
		) && !uarms && !u.twoweap
		) bonus *= 2;
		else if(otmp->otyp == KATANA && !uarms && !u.twoweap)
			bonus *= 1.5;
		else if(is_vibrosword(otmp) && !uarms && !u.twoweap)
			bonus *= 1.5;
		else if(otmp->otyp == FORCE_SWORD && !uarms && !u.twoweap)
			bonus *= 2;
		
		if(otmp==uwep 
		&& (is_rapier(otmp)
			|| (otmp->otyp == LIGHTSABER && otmp->oartifact != ART_ANNULUS && otmp->ovar1 == 0)
			|| otmp->oartifact == ART_LIFEHUNT_SCYTHE
			|| otmp->oartifact == ART_FRIEDE_S_SCYTHE
		)){
			if(is_rakuyo(otmp))
				bonus = 0;
			else bonus /= 2; /*Half strength bonus/penalty*/
			
			if(ACURR(A_DEX) == 25) bonus += 8;
			else bonus += (ACURR(A_DEX)-10)/2;
			
			if(is_rakuyo(otmp))
				bonus *= 2;
		}
		
		if(otmp->oartifact == ART_YORSHKA_S_SPEAR){
			if(ACURR(A_WIS) == 25) bonus += 8;
			else bonus += (ACURR(A_WIS)-10)/2;
			if(ACURR(A_DEX) == 25) bonus += 8;
			else bonus += (ACURR(A_DEX)-10)/2;
		}
		
		if(otmp->oartifact == ART_FRIEDE_S_SCYTHE){
			if(ACURR(A_INT) == 25) bonus += 8;
			else bonus += (ACURR(A_INT)-10)/2;
		}
	}
	
	if(u.sealsActive&SEAL_DANTALION){
		if(ACURR(A_INT) == 25) bonus += 8;
		else bonus += max(0,(ACURR(A_INT)-10)/2);
	}
	return bonus;
}

int
dtypbon(otyp)		/* damage bonus for strength */
int otyp;
{
	int str = ACURR(A_STR);
	int bonus = 0;
	
	
	if (str < 6) bonus = -6+str;
	else if (str < 16) bonus = 0;
	else if (str < 18) bonus = 1;
	else if (str == 18) bonus = 2;		/* up to 18 */
	else if (str <= STR18(75)) bonus = 3;		/* up to 18/75 */
	else if (str <= STR18(90)) bonus = 4;		/* up to 18/90 */
	else if (str < STR18(100)) bonus = 5;		/* up to 18/99 */
	else if (str < STR19(22)) bonus = 6;
	else if (str < STR19(25)) bonus = 7;
	else /*  str ==25*/bonus = 8;
	
	if(otyp && objects[otyp].oc_bimanual) bonus *= 2;
	
	if(otyp==RAPIER){
		int dex = ACURR(A_DEX);
		bonus/=2; /*Half strength bonus/penalty*/
		
		bonus += (dex-11)/2;
	}
	
	if(u.sealsActive&SEAL_DANTALION) bonus += max(0,(ACURR(A_INT)-10)/2);
	return bonus;
}


/* copy the skill level name into the given buffer */
STATIC_OVL char *
skill_level_name(skill, buf)
int skill;
char *buf;
{
    const char *ptr;

    switch (P_SKILL(skill)) {
	case P_UNSKILLED:    ptr = "Unskilled"; break;
	case P_BASIC:	     ptr = "Basic";     break;
	case P_SKILLED:	     ptr = "Skilled";   break;
	case P_EXPERT:	     ptr = "Expert";    break;
	/* these are for unarmed combat/martial arts only */
	case P_MASTER:	     ptr = "Master";    break;
	case P_GRAND_MASTER: ptr = "Grand Master"; break;
	default:	     ptr = "Unknown";	break;
    }
    Strcpy(buf, ptr);
    return buf;
}

/* copy the skill level name into the given buffer */
STATIC_OVL char *
max_skill_level_name(skill, buf)
int skill;
char *buf;
{
    const char *ptr;

    switch (P_MAX_SKILL(skill)) {
	case P_UNSKILLED:    ptr = "Unskilled"; break;
	case P_BASIC:	     ptr = "Basic";     break;
	case P_SKILLED:	     ptr = "Skilled";   break;
	case P_EXPERT:	     ptr = "Expert";    break;
	/* these are for unarmed combat/martial arts only */
	case P_MASTER:	     ptr = "Master";    break;
	case P_GRAND_MASTER: ptr = "Grand Master"; break;
	default:	     ptr = "Unknown";	break;
    }
    Strcpy(buf, ptr);
    return buf;
}

/* return the # of slots required to advance the skill */
STATIC_OVL int
slots_required(skill)
int skill;
{
    int tmp = OLD_P_SKILL(skill);

    /* The more difficult the training, the more slots it takes.
     *	unskilled -> basic	1
     *	basic -> skilled	2
     *	skilled -> expert	3
     */
    if (skill != P_BARE_HANDED_COMBAT && 
		(skill != P_TWO_WEAPON_COMBAT || !Role_if(PM_MONK))  && 
		skill != P_MARTIAL_ARTS  && 
		skill != FFORM_NIMAN
	) return tmp;

    /* Fewer slots used up for unarmed or martial.
     *	unskilled -> basic	1
     *	basic -> skilled	1
     *	skilled -> expert	2
     *	expert -> master	2
     *	master -> grand master	3
     */
    return (tmp + 1) / 2;
}

/* return true if this skill can be advanced */
/*ARGSUSED*/
STATIC_OVL boolean
can_advance(skill, speedy)
int skill;
boolean speedy;
{
    return !P_RESTRICTED(skill)
	    && P_SKILL(skill) < P_MAX_SKILL(skill) && (
#ifdef WIZARD
	    (wizard && speedy) ||
#endif
	    (P_ADVANCE(skill) >=
		(unsigned) practice_needed_to_advance(OLD_P_SKILL(skill))
		&& practice_needed_to_advance(OLD_P_SKILL(skill))
	    && u.skills_advanced < P_SKILL_LIMIT
	    && u.weapon_slots >= slots_required(skill)));
}

/* return true if this skill could be advanced if more slots were available */
STATIC_OVL boolean
could_advance(skill)
int skill;
{
    return !P_RESTRICTED(skill)
	    && P_SKILL(skill) < P_MAX_SKILL(skill) && (
	    (P_ADVANCE(skill) >=
		(unsigned) practice_needed_to_advance(OLD_P_SKILL(skill))
		&& practice_needed_to_advance(OLD_P_SKILL(skill))
	    && u.skills_advanced < P_SKILL_LIMIT));
}

/* return true if this skill has reached its maximum and there's been enough
   practice to become eligible for the next step if that had been possible */
STATIC_OVL boolean
peaked_skill(skill)
int skill;
{
    return !P_RESTRICTED(skill)
	    && P_SKILL(skill) >= P_MAX_SKILL(skill) && (
	    (P_ADVANCE(skill) >=
		(unsigned) practice_needed_to_advance(OLD_P_SKILL(skill))));
}

STATIC_OVL void
skill_advance(skill)
int skill;
{
	u.weapon_slots -= slots_required(skill);
	OLD_P_SKILL(skill)++;
	u.skill_record[u.skills_advanced++] = skill;
	/* subtly change the advance message to indicate no more advancement */
	You("are now %s skilled in %s.",
	P_SKILL(skill) >= P_MAX_SKILL(skill) ? "most" : "more",
	P_NAME(skill));
}

const static struct skill_range {
	short first, last;
	const char *name;
} skill_ranges[] = {
    { P_FIRST_H_TO_H, P_LAST_H_TO_H, "Fighting Skills" },
    { P_FIRST_WEAPON, P_LAST_WEAPON, "Weapon Skills" },
    { P_FIRST_SPELL,  P_LAST_SPELL,  "Spellcasting Skills" },
};

/*
 * The `#enhance' extended command.  What we _really_ would like is
 * to keep being able to pick things to advance until we couldn't any
 * more.  This is currently not possible -- the menu code has no way
 * to call us back for instant action.  Even if it did, we would also need
 * to be able to update the menu since selecting one item could make
 * others unselectable.
 */
int
enhance_weapon_skill()
{
    int pass, i, n, len, longest,
	to_advance, eventually_advance, maxxed_cnt;
    char buf[BUFSZ], sklnambuf[BUFSZ], maxsklnambuf[BUFSZ];
    const char *prefix;
    menu_item *selected;
    anything any;
    winid win;
    boolean speedy = FALSE;

#ifdef WIZARD
	if (wizard && yn("Advance skills without practice?") == 'y')
	    speedy = TRUE;
#endif

	do {
	    /* find longest available skill name, count those that can advance */
	    to_advance = eventually_advance = maxxed_cnt = 0;
	    for (longest = 0, i = 0; i < P_NUM_SKILLS; i++) {
			if (P_RESTRICTED(i)) continue;
			if ((len = strlen(P_NAME(i))) > longest)
				longest = len;
			if (can_advance(i, speedy)) to_advance++;
			else if (could_advance(i)) eventually_advance++;
			else if (peaked_skill(i)) maxxed_cnt++;
	    }

	    win = create_nhwindow(NHW_MENU);
	    start_menu(win);

	    /* start with a legend if any entries will be annotated
	       with "*" or "#" below */
	    if (eventually_advance > 0 || maxxed_cnt > 0) {
		any.a_void = 0;
		if (eventually_advance > 0) {
		    Sprintf(buf,
			    "(Skill%s flagged by \"*\" may be enhanced %s.)",
			    plur(eventually_advance),
			    (u.ulevel < MAXULEV) ?
				"when you're more experienced" :
				"if skill slots become available");
		    add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE,
			     buf, MENU_UNSELECTED);
		}
		if (maxxed_cnt > 0) {
		    Sprintf(buf,
		  "(Skill%s flagged by \"#\" cannot be enhanced any further.)",
			    plur(maxxed_cnt));
		    add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE,
			     buf, MENU_UNSELECTED);
		}
		add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE,
			     "", MENU_UNSELECTED);
	    }

	    /* List the skills, making ones that could be advanced
	       selectable.  List the miscellaneous skills first.
	       Possible future enhancement:  list spell skills before
	       weapon skills for spellcaster roles. */
	  for (pass = 0; pass < SIZE(skill_ranges); pass++)
	    for (i = skill_ranges[pass].first;
		 i <= skill_ranges[pass].last; i++) {
		/* Print headings for skill types */
		any.a_void = 0;
		if (i == skill_ranges[pass].first)
		    add_menu(win, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
			     skill_ranges[pass].name, MENU_UNSELECTED);

		if (P_RESTRICTED(i)) continue;
		/*
		 * Sigh, this assumes a monospaced font unless
		 * iflags.menu_tab_sep is set in which case it puts
		 * tabs between columns.
		 * The 12 is the longest skill level name.
		 * The "    " is room for a selection letter and dash, "a - ".
		 */
		if (can_advance(i, speedy))
		    prefix = "";	/* will be preceded by menu choice */
		else if (could_advance(i))
		    prefix = "  * ";
		else if (peaked_skill(i))
		    prefix = "  # ";
		else
		    prefix = (to_advance + eventually_advance +
				maxxed_cnt > 0) ? "    " : "";
		(void) skill_level_name(i, sklnambuf);
		(void) max_skill_level_name(i, maxsklnambuf);
#ifdef WIZARD
		if (wizard) {
		    if (!iflags.menu_tab_sep)
			Sprintf(buf, " %s%-*s %-12s %5d(%4d)",
			    prefix, longest, P_NAME(i), sklnambuf,
			    P_ADVANCE(i),
			    practice_needed_to_advance(OLD_P_SKILL(i)));
		    else
			Sprintf(buf, " %s%s\t%s\t%5d(%4d)",
			    prefix, P_NAME(i), sklnambuf,
			    P_ADVANCE(i),
			    practice_needed_to_advance(OLD_P_SKILL(i)));
		 } else
#endif
		{
		    if (!iflags.menu_tab_sep)
			Sprintf(buf, " %s %-*s [%s / %s]",
			    prefix, longest, P_NAME(i), sklnambuf,maxsklnambuf);
		    else
			Sprintf(buf, " %s%s\t[%s / %s]",
			    prefix, P_NAME(i), sklnambuf,maxsklnambuf);
		}
		any.a_int = can_advance(i, speedy) ? i+1 : 0;
		add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
	    }

	    Strcpy(buf, (to_advance > 0) ? "Pick a skill to advance:" :
					   "Current skills:");
	    if (!speedy)
		Sprintf(eos(buf), "  (%d slot%s available)",
			u.weapon_slots, plur(u.weapon_slots));
	    end_menu(win, buf);
	    n = select_menu(win, to_advance ? PICK_ONE : PICK_NONE, &selected);
	    destroy_nhwindow(win);
	    if (n > 0) {
		n = selected[0].item.a_int - 1;	/* get item selected */
		free((genericptr_t)selected);
		skill_advance(n);
		/* check for more skills able to advance, if so then .. */
		for (n = i = 0; i < P_NUM_SKILLS; i++) {
		    if (can_advance(i, speedy)) {
			if (!speedy) You_feel("you could be more dangerous!");
			n++;
			break;
		    }
		}
	    }
	} while (speedy && n > 0);
	return 0;
}

/*
 * Change from restricted to unrestricted, allowing P_BASIC as max.  This
 * function may be called with with P_NONE.  Used in pray.c.
 */
void
unrestrict_weapon_skill(skill)
int skill;
{
    if (skill < P_NUM_SKILLS && OLD_P_RESTRICTED(skill)) {
		OLD_P_SKILL(skill) = P_UNSKILLED;
		OLD_P_MAX_SKILL(skill) = P_BASIC;
		P_ADVANCE(skill) = 0;
    }
}

/*
 * Change from restricted to unrestricted, allowing P_EXPERT as max.  This
 * function may be called with with P_NONE.  Used in pray.c.
 */
void
expert_weapon_skill(skill)
int skill;
{
    if (skill < P_NUM_SKILLS && OLD_P_MAX_SKILL(skill) < P_EXPERT) {
		if(OLD_P_SKILL(skill) == P_ISRESTRICTED) OLD_P_SKILL(skill) = P_UNSKILLED;
		OLD_P_MAX_SKILL(skill) = P_EXPERT;
		P_ADVANCE(skill) = practice_needed_to_advance(OLD_P_SKILL(skill)-1);
    }
}

/*
 * Change from restricted to unrestricted, allowing P_GRAND_MASTER as max.  This
 * function may be called with with P_NONE.  Used in pray.c.
 */
void
gm_weapon_skill(skill)
int skill;
{
    if (skill < P_NUM_SKILLS && OLD_P_MAX_SKILL(skill) < P_EXPERT) {
		if(OLD_P_SKILL(skill) == P_ISRESTRICTED) OLD_P_SKILL(skill) = P_UNSKILLED;
		OLD_P_MAX_SKILL(skill) = P_GRAND_MASTER;
		P_ADVANCE(skill) = practice_needed_to_advance(OLD_P_SKILL(skill)-1);
    }
}

#endif /* OVL1 */
#ifdef OVLB

void
use_skill(skill,degree)
int skill;
int degree;
{
    boolean advance_before;
	
	if(skill < 0) skill *= -1;
	
    if (skill != P_NONE && !P_RESTRICTED(skill)) {
	advance_before = can_advance(skill, FALSE);
	P_ADVANCE(skill)+=degree;
	if (!advance_before && can_advance(skill, FALSE))
	    give_may_advance_msg(skill);
    }
}

void
add_weapon_skill(n)
int n;	/* number of slots to gain; normally one */
{
    int i, before, after;

    for (i = 0, before = 0; i < P_NUM_SKILLS; i++)
	if (can_advance(i, FALSE)) before++;
    u.weapon_slots += n;
    for (i = 0, after = 0; i < P_NUM_SKILLS; i++)
	if (can_advance(i, FALSE)) after++;
    if (before < after)
	give_may_advance_msg(P_NONE);
}

void
lose_weapon_skill(n)
int n;	/* number of slots to lose; normally one */
{
    int skill;

    while (--n >= 0) {
	/* deduct first from unused slots, then from last placed slot, if any */
	if (u.weapon_slots) {
	    u.weapon_slots--;
	} else if (u.skills_advanced) {
	    skill = u.skill_record[--u.skills_advanced];
	    if (OLD_P_SKILL(skill) <= P_UNSKILLED)
		panic("lose_weapon_skill (%d)", skill);
	    OLD_P_SKILL(skill)--;	/* drop skill one level */
	    /* Lost skill might have taken more than one slot; refund rest. */
	    u.weapon_slots = slots_required(skill) - 1;
	    /* It might now be possible to advance some other pending
	       skill by using the refunded slots, but giving a message
	       to that effect would seem pretty confusing.... */
	}
    }
}

int
weapon_type(obj)
struct obj *obj;
{
	/* KMH -- now uses the object table */
	int type;

	if (!obj)
		/* Not using a weapon */
		return (P_BARE_HANDED_COMBAT);
#ifdef CONVICT
    if ((obj->otyp == HEAVY_IRON_BALL) && (Role_if(PM_CONVICT) || u.sealsActive&SEAL_AHAZU))
        return objects[obj->otyp].oc_skill;
#endif /* CONVICT */
    if ((obj->otyp == CHAIN) && (Role_if(PM_CONVICT) || u.sealsActive&SEAL_AHAZU))
        return objects[obj->otyp].oc_skill;
	if (obj->oclass != WEAPON_CLASS && obj->oclass != TOOL_CLASS &&
	    obj->oclass != GEM_CLASS)
		/* Not a weapon, weapon-tool, or ammo */
		return (P_NONE);
	if(obj){
		if(obj->oartifact == ART_SUNSWORD){
			if(P_SKILL(P_LONG_SWORD) > P_SKILL(P_SHORT_SWORD))
				type = P_LONG_SWORD;
			else if(P_MAX_SKILL(P_LONG_SWORD) > P_MAX_SKILL(P_SHORT_SWORD))
				type = P_LONG_SWORD;
			else type = P_SHORT_SWORD;
		}
		else if(obj->oartifact == ART_YORSHKA_S_SPEAR){
			if(P_SKILL(P_HAMMER) > P_SKILL(P_SPEAR))
				type = P_HAMMER;
			else if(P_MAX_SKILL(P_HAMMER) > P_MAX_SKILL(P_SPEAR))
				type = P_HAMMER;
			else type = P_SPEAR;
		}
		else if(obj->otyp == DOUBLE_LIGHTSABER && !obj->altmode){
			if(P_SKILL(P_TWO_HANDED_SWORD) > P_SKILL(P_QUARTERSTAFF))
				type = P_TWO_HANDED_SWORD;
			else if(P_MAX_SKILL(P_TWO_HANDED_SWORD) > P_MAX_SKILL(P_QUARTERSTAFF))
				type = P_TWO_HANDED_SWORD;
			else type = P_QUARTERSTAFF;
		}
		else if(obj->oartifact == ART_TORCH_OF_ORIGINS){
			type = P_CLUB;
		}
		else if(obj->oartifact == ART_SINGING_SWORD){
			type = P_MUSICALIZE;
		}
		else if(obj->otyp >= LUCKSTONE && obj->otyp <= ROCK && obj->ovar1){
			type = (int)obj->ovar1;
		}
		else type = objects[obj->otyp].oc_skill;
	} else type = objects[obj->otyp].oc_skill;
	return ((type < 0) ? -type : type);
}

int
uwep_skill_type()
{
	if (u.twoweap)
		return P_TWO_WEAPON_COMBAT;
	return weapon_type(uwep);
}

/*
 * Return hit bonus/penalty based on skill of weapon.
 * Treat restricted weapons as unskilled.
 */
int
weapon_hit_bonus(weapon)
struct obj *weapon;
{
    int type, wep_type, skill, bonus = 0;
	unsigned int maxweight = 0;
    static const char bad_skill[] = "weapon_hit_bonus: bad skill %d";
	static boolean twowepwarn = TRUE;
	static boolean makashiwarn = TRUE;

    wep_type = weapon_type(weapon);
    /* use two weapon skill only if attacking with one of the wielded weapons */
	if((u.twoweap && (weapon == uwep || weapon == uswapwep))
		// && !(uwep && uswapwep && uswapwep->oartifact == ART_FRIEDE_S_SCYTHE)
	) type = P_TWO_WEAPON_COMBAT;
	else type = wep_type;
	
    if (type == P_NONE) {
		bonus = 0;
    } else if (type <= P_LAST_WEAPON) {
		switch (P_SKILL(type)) {
			default: impossible(bad_skill, P_SKILL(type)); /* fall through */
			case P_ISRESTRICTED:
			case P_UNSKILLED:   bonus = -4; break;
			case P_BASIC:       bonus =  0; break;
			case P_SKILLED:     bonus =  2; break;
			case P_EXPERT:      bonus =  5; break;
			//For use with martial-arts
			case P_MASTER:		bonus =  7; break;
			case P_GRAND_MASTER: bonus = 9; break;
		}
    } else if (type == P_TWO_WEAPON_COMBAT) {
		skill = P_SKILL(P_TWO_WEAPON_COMBAT);
		if (P_SKILL(wep_type) < skill) skill = P_SKILL(wep_type);
		switch (skill) {
			default: impossible(bad_skill, skill); /* fall through */
			case P_ISRESTRICTED:
			case P_UNSKILLED:   bonus = -10; break;
			case P_BASIC:	bonus = -8; break;
			case P_SKILLED:	bonus = -5; break;
			case P_EXPERT:	bonus = -2; break;
			//For use with martial-arts
			case P_MASTER:	bonus =  0; break;
			case P_GRAND_MASTER:	bonus = +2; break;
		}
    } else if (type == P_BARE_HANDED_COMBAT) {
	/*
	 *	       b.h.  m.a.
	 *	unskl:	+1    +2
	 *	basic:	+1    +3
	 *	skild:	+2    +4
	 *	exprt:	+2    +5
	 *	mastr:	+3    +6
	 *	grand:	+3    +7
	 */
	// bonus = P_SKILL(type);
	// bonus = max(bonus,P_UNSKILLED) - 1;	/* unskilled => 0 */
	// bonus = ((bonus + 1) * (martial_bonus() ? 3 : 1)) / 2;
		if(martial_bonus()){
			skill = P_SKILL(type);
			switch(skill){
				default: impossible(bad_skill, skill); /* fall through */
				case P_ISRESTRICTED:	bonus =  0; break;
				case P_UNSKILLED:   	bonus = +2; break;
				case P_BASIC:			bonus = +3; break;
				case P_SKILLED:			bonus = +4; break;
				case P_EXPERT:			bonus = +5; break;
				case P_MASTER:			bonus = +7; break;
				case P_GRAND_MASTER:	bonus = +9; break;
			}
		} else {
			skill = P_SKILL(type);
			switch(skill){
				default: impossible(bad_skill, skill); /* fall through */
				case P_ISRESTRICTED:	bonus = -2; break;
				case P_UNSKILLED:   	bonus =  0; break;
				case P_BASIC:			bonus = +1; break;
				case P_SKILLED:			bonus = +2; break;
				case P_EXPERT:			bonus = +3; break;
				case P_MASTER:			bonus = +4; break;
				case P_GRAND_MASTER:	bonus = +5; break;
			}
		}
    } else { //fallback for weapons that use non-weapon skills (like Singing Sword)
		switch (P_SKILL(type)) {
			default: impossible(bad_skill, P_SKILL(type)); /* fall through */
			case P_ISRESTRICTED:
			case P_UNSKILLED:   bonus = -4; break;
			case P_BASIC:       bonus =  0; break;
			case P_SKILLED:     bonus =  2; break;
			case P_EXPERT:      bonus =  5; break;
			//For use with martial-arts, should the need arise
			case P_MASTER:		bonus =  7; break;
			case P_GRAND_MASTER: bonus = 9; break;
		}
	}
	
	if(type == P_TWO_WEAPON_COMBAT){
		/* Sporkhack:
		 * Heavy things are hard to use in your offhand unless you're
		 * very good at what you're doing.
		 *
		 * No real need to restrict unskilled here since knives and such
		 * are very hard to find and people who are restricted can't
		 * #twoweapon even at unskilled...
		 */
		switch (P_SKILL(P_TWO_WEAPON_COMBAT)) {
			default: impossible(bad_skill, P_SKILL(P_TWO_WEAPON_COMBAT));
			case P_ISRESTRICTED:
			case P_UNSKILLED:	 maxweight = 10; break;	 /* not silver daggers */
			case P_BASIC:		 maxweight = 20; break;	 /* daggers, crysknife, sickle, aklys, flail, bullwhip, unicorn horn */
			case P_SKILLED:	 	 maxweight = 30; break;	 /* shortswords and spears (inc silver), mace, club, lightsaber, grappling hook */
			case P_EXPERT:		 maxweight = 40; break;	 /* sabers and long swords, axe weighs 60, war hammer 50, pickaxe 80, beamsword */
			case P_MASTER:			maxweight = 50; break;
			case P_GRAND_MASTER:	maxweight = 60; break;
		}
		if (uswapwep && !(uwep && (uwep->otyp == STILETTOS)) && uswapwep->owt > maxweight
		&& uswapwep->oartifact != ART_BLADE_DANCER_S_DAGGER && uswapwep->oartifact != ART_FRIEDE_S_SCYTHE
		) {
			if(twowepwarn) pline("Your %s seem%s very unwieldy.",xname(uswapwep),uswapwep->quan == 1 ? "s" : "");
			twowepwarn = FALSE;
			bonus += -20;
		} else if(!twowepwarn) twowepwarn = TRUE;
	} else if(!twowepwarn) twowepwarn = TRUE;
#ifdef STEED
	/* KMH -- It's harder to hit while you are riding */
	if (u.usteed) {
		switch (P_SKILL(P_RIDING)) {
		    case P_ISRESTRICTED:bonus -= 5; break;
		    case P_UNSKILLED:   bonus -= 2; break;
		    case P_BASIC:       bonus -= 1; break;
		    case P_SKILLED:     break;
		    case P_EXPERT:      bonus += 2; break;//but an expert can use the added momentum
		}
		if (u.twoweap) bonus -= 2;
	}
#endif
	//Do to-hit bonuses for lightsaber forms here.  May do other fighting styles at some point.
	if(weapon && is_lightsaber(weapon) && litsaber(weapon) && uwep == weapon){
		if(u.fightingForm < FFORM_SHII_CHO || u.fightingForm > FFORM_JUYO) u.fightingForm = FFORM_SHII_CHO;
		if(P_SKILL(u.fightingForm) < P_BASIC){
			if(weapon->oartifact == ART_INFINITY_S_MIRRORED_ARC)
				u.fightingForm = FFORM_NIMAN;
			else u.fightingForm = FFORM_SHII_CHO;
		}
		if(u.fightingForm == FFORM_MAKASHI && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm))){
			if(wep_type != P_SABER){
				if(makashiwarn) pline("Your %s seem%s very unwieldy.",xname(uwep),uwep->quan == 1 ? "s" : "");
				makashiwarn = FALSE;
				bonus += -20;
			} else if(!makashiwarn) makashiwarn = TRUE;
		} else if(!makashiwarn) makashiwarn = TRUE;

		switch(u.fightingForm){
			case FFORM_SHII_CHO:
				switch(min(P_SKILL(FFORM_SHII_CHO), P_SKILL(wep_type))){
					case P_ISRESTRICTED:bonus -= 5; break;
					case P_UNSKILLED:   bonus -= 2; break;
					case P_BASIC:       break;
					case P_SKILLED:     bonus += 2; break;
					case P_EXPERT:      bonus += 5; break;
				}
			break;
			case FFORM_MAKASHI:
				if(!uarm || is_light_armor(uarm) || is_medium_armor(uarm)){
					int sx, sy, mcount = -1;
					for(sx = u.ux-1; sx<=u.ux+1; sx++){
						for(sy = u.uy-1; sy<=u.uy+1; sy++){
							if(isok(sx,sy) && m_at(sx,sy)) mcount++;
						}
					}
					if(wep_type != P_SABER) bonus += -20;
					switch(min(P_SKILL(FFORM_MAKASHI), P_SKILL(wep_type))){
						case P_BASIC:
							if(wep_type == P_SABER) bonus += ((ACURR(A_DEX)+3)/3 - 4);
							if(mcount) bonus -= (mcount-1) * 5;
						break;
						case P_SKILLED:
							if(wep_type == P_SABER) bonus += 2*((ACURR(A_DEX)+3)/3 - 4);
							if(mcount) bonus -= (mcount-1) * 2;
						break;
						case P_EXPERT:
							if(wep_type == P_SABER) bonus += 3*((ACURR(A_DEX)+3)/3 - 4);
							if(mcount) bonus -= (mcount-1);
						break;
					}
				}
			break;
			case FFORM_SORESU:
				if(!uarm || is_light_armor(uarm) || is_medium_armor(uarm)){
					if(flags.mon_moving){
						switch(min(P_SKILL(FFORM_SORESU), P_SKILL(wep_type))){
							case P_BASIC:
								bonus += 1;
							break;
							case P_SKILLED:
								bonus += 2;
							break;
							case P_EXPERT:
								bonus += 5;
							break;
						}
					} else {
						switch(min(P_SKILL(FFORM_SORESU), P_SKILL(wep_type))){
							case P_BASIC:
								bonus -= 10;
							break;
							case P_SKILLED:
								bonus -= 5;
							break;
							case P_EXPERT:
								bonus -= 2;
							break;
						}
					}
				}
			break;
			case FFORM_ATARU:
				if(!uarm || is_light_armor(uarm)){
					switch(min(P_SKILL(FFORM_ATARU), P_SKILL(wep_type))){
						case P_BASIC:
							bonus -= 2;
						break;
						case P_SKILLED:
							bonus -= 1;
						break;
						case P_EXPERT:
						break;
					}
				}
			break;
			case FFORM_DJEM_SO:{
				int sbon;
				int str = ACURR(A_STR);
				if (str < 6) sbon = -2;
				else if (str < 8) sbon = -1;
				else if (str < 17) sbon = 0;
				else if (str <= STR18(50)) sbon = 1;	/* up to 18/50 */
				else if (str < STR18(100)) sbon = 2;
				else sbon = 3;
				if(flags.mon_moving){
					switch(min(P_SKILL(FFORM_DJEM_SO), P_SKILL(wep_type))){
						case P_BASIC:
							bonus += 1 + sbon;
						break;
						case P_SKILLED:
							bonus += 2 + sbon;
						break;
						case P_EXPERT:
							bonus += 5 + sbon;
						break;
					}
				} else {
					bonus += sbon;
				}
			} break;
			case FFORM_SHIEN:
				if(flags.mon_moving){
					switch(min(P_SKILL(FFORM_SHIEN), P_SKILL(wep_type))){
						case P_BASIC:
							// bonus
						break;
						case P_SKILLED:
							bonus += 1;
						break;
						case P_EXPERT:
							bonus += 2;
						break;
					}
				} else {
					switch(min(P_SKILL(FFORM_SHIEN), P_SKILL(wep_type))){
						case P_BASIC:
							bonus -= 5;
						break;
						case P_SKILLED:
							bonus -= 2;
						break;
						case P_EXPERT:
							bonus -= 1;
						break;
					}
				}
			break;
			// case FFORM_NIMAN:
				// //no bonus or penalty
			// break;
			case FFORM_JUYO:
				switch(min(P_SKILL(FFORM_JUYO), P_SKILL(wep_type))){
					case P_BASIC:
						bonus -= 2;
					break;
					case P_SKILLED:
						bonus -= 1;
					break;
					case P_EXPERT:
					break;
				}
			break;
		}
	}
	
	if(wep_type == P_AXE && Race_if(PM_DWARF) && ublindf && ublindf->oartifact == ART_WAR_MASK_OF_DURIN) bonus += 5;
	if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && type != P_TWO_WEAPON_COMBAT) bonus = max(bonus,0);
	
    return bonus;
}

/*
 * Return damage bonus/penalty based on skill of weapon.
 * Treat restricted weapons as unskilled.
 */
int
weapon_dam_bonus(weapon)
struct obj *weapon;
{
    int type, wep_type, skill, bonus = 0;
	unsigned int maxweight = 0;

    wep_type = weapon_type(weapon);
    /* use two weapon skill only if attacking with one of the wielded weapons */
	if((u.twoweap && (weapon == uwep || weapon == uswapwep))
		&& !(uwep && uswapwep && uswapwep->oartifact == ART_FRIEDE_S_SCYTHE)
	) type = P_TWO_WEAPON_COMBAT;
	else type = wep_type;
	
    if (type == P_NONE) {
		bonus = 0;
    } else if (type <= P_LAST_WEAPON) {
		switch (P_SKILL(type)) {
			default: impossible("weapon_dam_bonus: bad skill %d",P_SKILL(type));
				 /* fall through */
			case P_ISRESTRICTED:	bonus = -5; break;
			case P_UNSKILLED:	bonus = -2; break;
			case P_BASIC:	bonus =  0; break;
			case P_SKILLED:	bonus =  2; break;
			case P_EXPERT:	bonus =  5; break;
			//For use with martial-arts
			case P_MASTER:		bonus =  7; break;
			case P_GRAND_MASTER: bonus = 9; break;
		}
	} else if (type == P_TWO_WEAPON_COMBAT) {
		skill = P_SKILL(P_TWO_WEAPON_COMBAT);
		if (P_SKILL(wep_type) < skill) skill = P_SKILL(wep_type);
		if(wep_type == P_BARE_HANDED_COMBAT){
			if(martial_bonus()){
				skill = P_SKILL(type);
				switch(skill){
					default: impossible("weapon_dam_bonus: bad skill %d",P_SKILL(type)); /* fall through */
					case P_ISRESTRICTED:	bonus = -5; break;
					case P_UNSKILLED:   	bonus = -3; break;
					case P_BASIC:			bonus = -1; break;
					case P_SKILLED:			bonus = +1; break;
					case P_EXPERT:			bonus = +3; break;
					case P_MASTER:			bonus = +5; break;
					case P_GRAND_MASTER:	bonus = +7; break;
				}
			} else {
				skill = P_SKILL(type);
				switch(skill){
					default: impossible("weapon_dam_bonus: bad skill %d",P_SKILL(type)); /* fall through */
					case P_ISRESTRICTED:	bonus = -6; break;
					case P_UNSKILLED:   	bonus = -4; break;
					case P_BASIC:			bonus = -2; break;
					case P_SKILLED:			bonus =  0; break;
					case P_EXPERT:			bonus = +1; break;
					case P_MASTER:			bonus = +2; break;
					case P_GRAND_MASTER:	bonus = +3; break;
				}
			}
		} else {
			switch (skill) {
				default:
				case P_ISRESTRICTED:
				case P_UNSKILLED:	bonus = -5; break;
				case P_BASIC:	bonus = -3; break;
				case P_SKILLED:	bonus = -1; break;
				case P_EXPERT:	bonus =  0; break;
			}
		}
    } else if (type == P_BARE_HANDED_COMBAT) {
	// /*
	 // *	       b.h.  m.a.
	 // *	unskl:	 0   n/a
	 // *	basic:	+1    +3
	 // *	skild:	+1    +4
	 // *	exprt:	+2    +6
	 // *	mastr:	+2    +7
	 // *	grand:	+3    +9
	 // */
	// bonus = P_SKILL(type);
	// bonus = max(bonus,P_UNSKILLED) - 1;	/* unskilled => 0 */
	// bonus = ((bonus + 1) * (martial_bonus() ? 3 : 1)) / 2;
		if(martial_bonus()){
			skill = P_SKILL(type);
			switch(skill){
				default: impossible("weapon_dam_bonus: bad skill %d",P_SKILL(type)); /* fall through */
				case P_ISRESTRICTED:	bonus = -2; break;
				case P_UNSKILLED:   	bonus = +1; break;
				case P_BASIC:			bonus = +3; break;
				case P_SKILLED:			bonus = +4; break;
				case P_EXPERT:			bonus = +5; break;
				case P_MASTER:			bonus = +7; break;
				case P_GRAND_MASTER:	bonus = +9; break;
			}
		} else {
			skill = P_SKILL(type);
			switch(skill){
				default: impossible("weapon_dam_bonus: bad skill %d",P_SKILL(type)); /* fall through */
				case P_ISRESTRICTED:	bonus = -4; break;
				case P_UNSKILLED:   	bonus = -2; break;
				case P_BASIC:			bonus =  0; break;
				case P_SKILLED:			bonus = +1; break;
				case P_EXPERT:			bonus = +2; break;
				case P_MASTER:			bonus = +3; break;
				case P_GRAND_MASTER:	bonus = +5; break;
			}
		}
    } else { //fallback for weapons that use non-weapon skills (like Singing Sword)
		switch (P_SKILL(type)) {
			default: impossible("weapon_dam_bonus: bad skill %d",P_SKILL(type));
				 /* fall through */
			case P_ISRESTRICTED:	bonus = -5; break;
			case P_UNSKILLED:	bonus = -2; break;
			case P_BASIC:	bonus =  0; break;
			case P_SKILLED:	bonus =  2; break;
			case P_EXPERT:	bonus =  5; break;
			//For use with martial-arts
			case P_MASTER:		bonus =  7; break;
			case P_GRAND_MASTER: bonus = 9; break;
		}
    }
	
	if(type == P_TWO_WEAPON_COMBAT){
		/* Sporkhack:
		 * Heavy things are hard to use in your offhand unless you're
		 * very good at what you're doing.
		 *
		 * No real need to restrict unskilled here since knives and such
		 * are very hard to find and people who are restricted can't
		 * #twoweapon even at unskilled...
		 */
		switch (P_SKILL(P_TWO_WEAPON_COMBAT)) {
			default:
			case P_ISRESTRICTED:
			case P_UNSKILLED:	 maxweight = 10; break;	 /* not silver daggers */
			case P_BASIC:		 maxweight = 20; break;	 /* daggers, crysknife, sickle, aklys, flail, bullwhip, unicorn horn */
			case P_SKILLED:	 	 maxweight = 30; break;	 /* shortswords and spears (inc silver), mace, club, lightsaber, grappling hook */
			case P_EXPERT:		 maxweight = 40; break;	 /* sabers and long swords, axe weighs 60, war hammer 50, pickaxe 80, beamsword */
			case P_MASTER:		 maxweight = 50; break;	 /* war hammer */
			case P_GRAND_MASTER: maxweight = 60; break;	 /* axe */
		}
		if (uswapwep && !(uwep && (uwep->otyp == STILETTOS)) && uswapwep->owt > maxweight
		&& uswapwep->oartifact != ART_BLADE_DANCER_S_DAGGER && uswapwep->oartifact != ART_FRIEDE_S_SCYTHE
		) {
			bonus += max(-20, -5 * (uswapwep->owt-maxweight)/maxweight);
		}
	}

#ifdef STEED
	/* KMH -- Riding gives some thrusting damage */
	if (u.usteed && type != P_TWO_WEAPON_COMBAT) {
		switch (P_SKILL(P_RIDING)) {
		    case P_ISRESTRICTED:
		    case P_UNSKILLED:   break;
		    case P_BASIC:       break;
		    case P_SKILLED:     bonus += 2; break;
		    case P_EXPERT:      bonus += 5; break;
		}
	}
#endif

	//Do damage bonuses for lightsaber forms here.  May do other fighting styles at some point.
	if(weapon && is_lightsaber(weapon) && litsaber(weapon) && uwep == weapon){
		if(u.fightingForm < FFORM_SHII_CHO || u.fightingForm > FFORM_JUYO) u.fightingForm = FFORM_SHII_CHO;
		if(P_SKILL(u.fightingForm) < P_BASIC) u.fightingForm = FFORM_SHII_CHO;
		switch(u.fightingForm){
			// case FFORM_SHII_CHO:
				// //no bonus
			// break;
			case FFORM_MAKASHI:
				if(!uarms && !u.twoweap && wep_type == P_SABER) switch(min(P_SKILL(FFORM_MAKASHI), P_SKILL(wep_type))){
					case P_BASIC:
						bonus += 2 + ((ACURR(A_DEX)+3)/3 - 4);
					break;
					case P_SKILLED:
						bonus += (2*(ACURR(A_DEX)+3))/3 - 3;
					break;
					case P_EXPERT:
						bonus += 1 + ACURR(A_DEX);
					break;
				}
			break;
			// case FFORM_SORESU:
				// //No bonus
			// break;
			// case FFORM_ATARU:
				// //No bonus
			// break;
			// case FFORM_DJEM_SO:
				// //No bonus
			// break;
			case FFORM_SHIEN:{
				int sx, sy, mcount = -1;
				for(sx = u.ux-1; sx<=u.ux+1; sx++){
					for(sy = u.uy-1; sy<=u.uy+1; sy++){
						if(isok(sx,sy) && m_at(sx,sy)) mcount++;
					}
				}
				if(mcount > 1){
					int sbon = ACURR(A_STR);
					if(sbon >= STR19(19)) sbon -= 100; //remove percentile adjustment
					else if(sbon > 18) sbon = 18; //remove percentile adjustment
					//else it is fine as is.
					sbon = (sbon+2)/3; //1-9
					switch(min(P_SKILL(FFORM_SHIEN), P_SKILL(wep_type))){
						case P_BASIC:
							bonus += d(1,sbon+mcount); //1d17 max
						break;
						case P_SKILLED:
							bonus += d(1,sbon+mcount*2); //1d25 max
						break;
						case P_EXPERT:
							bonus += d(1,sbon+mcount*3); //1d33 max
						break;
					}
				}
			} break;
			// case FFORM_NIMAN:
				// //no bonus
			// break;
			// case FFORM_JUYO:
				// //no bonus
			// break;
		}
	}
	
	if(wep_type == P_AXE && Race_if(PM_DWARF) && ublindf && ublindf->oartifact == ART_WAR_MASK_OF_DURIN) bonus += 5;
	if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && type != P_TWO_WEAPON_COMBAT) bonus = max(bonus,0);
	
	return bonus;
}

/*
 * Return damage bonus/penalty based on skill.
 * Treat restricted weapons as unskilled.
 */
int
skill_dam_bonus(type)
int type;
{
    int bonus = 0;

    /* use two weapon skill only if attacking with one of the wielded weapons */
	
    if (type == P_NONE) {
		bonus = 0;
    } else if (type <= P_LAST_WEAPON) {
		switch (P_SKILL(type)) {
			default: impossible("weapon_dam_bonus: bad skill %d",P_SKILL(type));
				 /* fall through */
			case P_ISRESTRICTED:	bonus = -5; break;
			case P_UNSKILLED:	bonus = -2; break;
			case P_BASIC:	bonus =  0; break;
			case P_SKILLED:	bonus =  2; break;
			case P_EXPERT:	bonus =  5; break;
		}
	} else if (type == P_TWO_WEAPON_COMBAT) {
		switch (P_SKILL(type)) {
			default:
			case P_ISRESTRICTED:
			case P_UNSKILLED:	bonus = -5; break;
			case P_BASIC:	bonus = -3; break;
			case P_SKILLED:	bonus = -1; break;
			case P_EXPERT:	bonus =  0; break;
		}
    } else if (type == P_BARE_HANDED_COMBAT) {
		if(martial_bonus()){
			switch(P_SKILL(type)){
				default: impossible("weapon_dam_bonus: bad skill %d",P_SKILL(type)); /* fall through */
				case P_ISRESTRICTED:	bonus = -2; break;
				case P_UNSKILLED:   	bonus = +1; break;
				case P_BASIC:			bonus = +3; break;
				case P_SKILLED:			bonus = +4; break;
				case P_EXPERT:			bonus = +5; break;
				case P_MASTER:			bonus = +7; break;
				case P_GRAND_MASTER:	bonus = +9; break;
			}
		} else {
			switch(P_SKILL(type)){
				default: impossible("weapon_dam_bonus: bad skill %d",P_SKILL(type)); /* fall through */
				case P_ISRESTRICTED:	bonus = -4; break;
				case P_UNSKILLED:   	bonus = -2; break;
				case P_BASIC:			bonus =  0; break;
				case P_SKILLED:			bonus = +1; break;
				case P_EXPERT:			bonus = +2; break;
				case P_MASTER:			bonus = +3; break;
				case P_GRAND_MASTER:	bonus = +5; break;
			}
		}
    }
	
	if(type == P_TWO_WEAPON_COMBAT){
		int maxweight;
		/* Sporkhack:
		 * Heavy things are hard to use in your offhand unless you're
		 * very good at what you're doing.
		 *
		 * No real need to restrict unskilled here since knives and such
		 * are very hard to find and people who are restricted can't
		 * #twoweapon even at unskilled...
		 */
		switch (P_SKILL(P_TWO_WEAPON_COMBAT)) {
			default:
			case P_ISRESTRICTED:
			case P_UNSKILLED:	 maxweight = 10; break;	 /* not silver daggers */
			case P_BASIC:		 maxweight = 20; break;	 /* daggers, crysknife, sickle, aklys, flail, bullwhip, unicorn horn */
			case P_SKILLED:	 	 maxweight = 30; break;	 /* shortswords and spears (inc silver), mace, club, lightsaber, grappling hook */
			case P_EXPERT:		 maxweight = 40; break;	 /* sabers and long swords, axe weighs 60, war hammer 50, pickaxe 80, beamsword */
			case P_MASTER:		 maxweight = 50; break;	 /* war hammer */
			case P_GRAND_MASTER: maxweight = 60; break;	 /* axe */
		}
		if (uswapwep && !(uwep && (uwep->otyp == STILETTOS)) && uswapwep->owt > maxweight
		&& uswapwep->oartifact != ART_BLADE_DANCER_S_DAGGER && uswapwep->oartifact != ART_FRIEDE_S_SCYTHE
		) {
			bonus += max(-20, -5 * (uswapwep->owt-maxweight)/maxweight);
		}
	}

#ifdef STEED
	/* KMH -- Riding gives some thrusting damage */
	if (u.usteed && type != P_TWO_WEAPON_COMBAT) {
		switch (P_SKILL(P_RIDING)) {
		    case P_ISRESTRICTED:
		    case P_UNSKILLED:   break;
		    case P_BASIC:       break;
		    case P_SKILLED:     bonus += 2; break;
		    case P_EXPERT:      bonus += 5; break;
		}
	}
#endif

	if(type == P_AXE && Race_if(PM_DWARF) && ublindf && ublindf->oartifact == ART_WAR_MASK_OF_DURIN) bonus += 5;
	
	return bonus;
}

/*
 * Initialize weapon skill array for the game.  Start by setting all
 * skills to restricted, then set the skill for every weapon the
 * hero is holding, finally reading the given array that sets
 * maximums.
 */
void
skill_add(class_skill)
const struct def_skill *class_skill;
{
	int skmax, skill;
	/* walk through array to set skill maximums */
	for (; class_skill->skill != P_NONE; class_skill++) {
	    skmax = class_skill->skmax;
	    skill = class_skill->skill;

	    OLD_P_MAX_SKILL(skill) = skmax;
	    if (OLD_P_SKILL(skill) == P_ISRESTRICTED)	/* skill pre-set */
			OLD_P_SKILL(skill) = P_UNSKILLED;
	}

	/* High potential fighters already know how to use their hands. */
	if (OLD_P_MAX_SKILL(P_BARE_HANDED_COMBAT) > P_EXPERT)
	    OLD_P_SKILL(P_BARE_HANDED_COMBAT) = P_BASIC;
	
	/*
	 * Make sure we haven't missed setting the max on a skill
	 * & set advance
	 */
	for (skill = 0; skill < P_NUM_SKILLS; skill++) {
	    if (!OLD_P_RESTRICTED(skill)) {
		if (OLD_P_MAX_SKILL(skill) < OLD_P_SKILL(skill)) {
		    impossible("skill_init: curr > max: %s", P_NAME(skill));
		    OLD_P_MAX_SKILL(skill) = OLD_P_SKILL(skill);
		}
		P_ADVANCE(skill) = practice_needed_to_advance(OLD_P_SKILL(skill)-1);
	    }
	}
}
void
skill_init(class_skill)
const struct def_skill *class_skill;
{
	struct obj *obj;
	int skmax, skill;

	/* initialize skill array; by default, everything is restricted */
	for (skill = 0; skill < P_NUM_SKILLS; skill++) {
		OLD_P_SKILL(skill) = P_ISRESTRICTED;
		OLD_P_MAX_SKILL(skill) = P_ISRESTRICTED;
		P_ADVANCE(skill) = 0;
	}

	/* Set skill for all weapons in inventory to be basic */
	if(!Role_if(PM_EXILE)) for (obj = invent; obj; obj = obj->nobj) {
	    skill = weapon_type(obj);
	    if (skill != P_NONE)
			OLD_P_SKILL(skill) = Role_if(PM_PIRATE) ? P_SKILLED : P_BASIC;
	}

	/* set skills for magic */
	if (Role_if(PM_HEALER) || Role_if(PM_MONK)) {
		OLD_P_SKILL(P_HEALING_SPELL) = P_BASIC;
	} else if (Role_if(PM_PRIEST)) {
		OLD_P_SKILL(P_CLERIC_SPELL) = P_BASIC;
	} else if (Role_if(PM_WIZARD)) {
		OLD_P_SKILL(P_ATTACK_SPELL) = P_BASIC;
		OLD_P_SKILL(P_ENCHANTMENT_SPELL) = P_BASIC;
	}
#ifdef BARD
	if (Role_if(PM_BARD)) {
	  OLD_P_SKILL(P_MUSICALIZE) = P_BASIC;
	  OLD_P_SKILL(P_BARE_HANDED_COMBAT) = P_BASIC;
	  OLD_P_SKILL(P_BEAST_MASTERY) = P_BASIC;
	  OLD_P_SKILL(P_DART) = P_BASIC;
	  OLD_P_SKILL(P_DAGGER) = P_BASIC;
	}
#endif
	if (u.specialSealsActive&SEAL_BLACK_WEB) {
	  OLD_P_SKILL(P_CROSSBOW) = P_BASIC;
	}

	/* walk through array to set skill maximums */
	for (; class_skill->skill != P_NONE; class_skill++) {
		if(!(Race_if(PM_VAMPIRE) && !Role_if(PM_ANACHRONONAUT)) || class_skill->skill != P_TWO_WEAPON_COMBAT){
			skmax = class_skill->skmax;
			skill = class_skill->skill;

			OLD_P_MAX_SKILL(skill) = skmax;
			if (OLD_P_SKILL(skill) == P_ISRESTRICTED)	/* skill pre-set */
				OLD_P_SKILL(skill) = P_UNSKILLED;
		}
	}

	/* High potential fighters already know how to use their hands. */
	if (OLD_P_MAX_SKILL(P_BARE_HANDED_COMBAT) > P_EXPERT)
	    OLD_P_SKILL(P_BARE_HANDED_COMBAT) = P_BASIC;

	/* Roles that start with a horse know how to ride it */
#ifdef STEED
	if (urole.petnum == PM_PONY)
	    OLD_P_SKILL(P_RIDING) = P_BASIC;
#endif

	/*
	 * Make sure we haven't missed setting the max on a skill
	 * & set advance
	 */
	for (skill = 0; skill < P_NUM_SKILLS; skill++) {
	    if (!OLD_P_RESTRICTED(skill)) {
		if (OLD_P_MAX_SKILL(skill) < OLD_P_SKILL(skill)) {
		    impossible("skill_init: curr > max: %s", P_NAME(skill));
		    OLD_P_MAX_SKILL(skill) = OLD_P_SKILL(skill);
		}
		P_ADVANCE(skill) = practice_needed_to_advance(OLD_P_SKILL(skill)-1);
	    }
	}
}

int
aeshbon()
{
	int bonus = 0;
	if(u.uaesh_duration)
		bonus += 10;
	if(u.uaesh){
		bonus += u.uaesh/3;
		//remainder is probabilistic
		if(rn2(3) < u.uaesh%3)
			bonus++;
	}
	return bonus;
}

#endif /* OVLB */

/*weapon.c*/
