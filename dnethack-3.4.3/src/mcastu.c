/*	SCCS Id: @(#)mcastu.c	3.4	2003/01/08	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* monster mage spells */
#define FIRST_MGC		 0
#define MGC_PSI_BOLT	 0
#define MGC_CURE_SELF	 1
#define MGC_HASTE_SELF	 2
#define MGC_STUN_YOU	 3
#define MGC_DISAPPEAR	 4
#define MGC_WEAKEN_YOU	 5
#define MGC_DESTRY_ARMR	 6
#define MGC_CURSE_ITEMS	 7
#define MGC_AGGRAVATION	 8
#define MGC_SUMMON_MONS	 9
#define MGC_CLONE_WIZ	 10
#define MGC_DEATH_TOUCH	 11
#define LAST_MGC		 11

/* monster cleric spells */
#define FIRST_CLC		 12
#define CLC_OPEN_WOUNDS	 12
#define CLC_CURE_SELF	 13
#define CLC_CONFUSE_YOU	 14
#define CLC_PARALYZE	 15
#define CLC_BLIND_YOU	 16
#define CLC_INSECTS	 	 17
#define CLC_CURSE_ITEMS	 18
#define CLC_LIGHTNING	 19
#define CLC_FIRE_PILLAR	 20
#define CLC_GEYSER	 	 21
#define LAST_CLC	 	 21

STATIC_DCL void FDECL(cursetxt,(struct monst *,BOOLEAN_P));
STATIC_DCL int FDECL(choose_magic_spell, (int));
STATIC_DCL int FDECL(choose_clerical_spell, (int));
STATIC_DCL void FDECL(cast_wizard_spell,(struct monst *, int,int));
STATIC_DCL void FDECL(cast_cleric_spell,(struct monst *, int,int));
STATIC_DCL boolean FDECL(is_undirected_spell,(unsigned int,int));
STATIC_DCL boolean FDECL(spell_would_be_useless,(struct monst *,unsigned int,int));

#ifdef OVL0

extern const char * const flash_types[];	/* from zap.c */

/* feedback when frustrated monster couldn't cast a spell */
STATIC_OVL
void
cursetxt(mtmp, undirected)
struct monst *mtmp;
boolean undirected;
{
	if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
	    const char *point_msg;  /* spellcasting monsters are impolite */

	    if (undirected)
		point_msg = "all around, then curses";
	    else if ((Invis && !perceives(mtmp->data) &&
			(mtmp->mux != u.ux || mtmp->muy != u.uy)) ||
		    (youmonst.m_ap_type == M_AP_OBJECT &&
			youmonst.mappearance == STRANGE_OBJECT) ||
		    u.uundetected)
		point_msg = "and curses in your general direction";
	    else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
		point_msg = "and curses at your displaced image";
	    else
		point_msg = "at you, then curses";

	    pline("%s points %s.", Monnam(mtmp), point_msg);
	} else if ((!(moves % 4) || !rn2(4))) {
	    if (flags.soundok) Norep("You hear a mumbled curse.");
	}
}

#endif /* OVL0 */
#ifdef OVLB

/* convert a level based random selection into a specific mage spell;
   inappropriate choices will be screened out by spell_would_be_useless() */
STATIC_OVL int
choose_magic_spell(spellval)
int spellval;
{
    switch (spellval) {
    case 22:
    case 21:
    case 20:
	return MGC_DEATH_TOUCH;
    case 19:
    case 18:
	return MGC_CLONE_WIZ;
    case 17:
    case 16:
    case 15:
	return MGC_SUMMON_MONS;
    case 14:
    case 13:
	return MGC_AGGRAVATION;
    case 12:
    case 11:
    case 10:
	return MGC_CURSE_ITEMS;
    case 9:
    case 8:
	return MGC_DESTRY_ARMR;
    case 7:
    case 6:
	return MGC_WEAKEN_YOU;
    case 5:
    case 4:
	return MGC_DISAPPEAR;
    case 3:
	return MGC_STUN_YOU;
    case 2:
	return MGC_HASTE_SELF;
    case 1:
	return MGC_CURE_SELF;
    case 0:
    default:
	return MGC_PSI_BOLT;
    }
}

/* convert a level based random selection into a specific cleric spell */
STATIC_OVL int
choose_clerical_spell(spellnum)
int spellnum;
{
    switch (spellnum) {
    case 13:
	return CLC_GEYSER;
    case 12:
	return CLC_FIRE_PILLAR;
    case 11:
	return CLC_LIGHTNING;
    case 10:
    case 9:
	return CLC_CURSE_ITEMS;
    case 8:
	return CLC_INSECTS;
    case 7:
    case 6:
	return CLC_BLIND_YOU;
    case 5:
    case 4:
	return CLC_PARALYZE;
    case 3:
    case 2:
	return CLC_CONFUSE_YOU;
    case 1:
	return CLC_CURE_SELF;
    case 0:
    default:
	return CLC_OPEN_WOUNDS;
    }
}

/* return values:
 * 1: successful spell
 * 0: unsuccessful spell
 */
int
castmu(mtmp, mattk, thinks_it_foundyou, foundyou)
	register struct monst *mtmp;
	register struct attack *mattk;
	boolean thinks_it_foundyou;
	boolean foundyou;
{
	int	dmg, ml = mtmp->m_lev;
	int ret;
	int spellnum = 0;

	/* Three cases:
	 * -- monster is attacking you.  Search for a useful spell.
	 * -- monster thinks it's attacking you.  Search for a useful spell,
	 *    without checking for undirected.  If the spell found is directed,
	 *    it fails with cursetxt() and loss of mspec_used.
	 * -- monster isn't trying to attack.  Select a spell once.  Don't keep
	 *    searching; if that spell is not useful (or if it's directed),
	 *    return and do something else. 
	 * Since most spells are directed, this means that a monster that isn't
	 * attacking casts spells only a small portion of the time that an
	 * attacking monster does.
	 */
	if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) && ml) {
	    int cnt = 40;

	    do {
		spellnum = rn2(min(ml,50));
		if (mattk->adtyp == AD_SPEL){
			if(mtmp->data==&mons[PM_SHOGGOTH]) if(!rn2(20)) spellnum=MGC_SUMMON_MONS; else return 0;
			else if(mtmp->data==&mons[PM_VERIER]) 
				if(!rn2(3))spellnum=MGC_WEAKEN_YOU;
				else spellnum=MGC_DESTRY_ARMR;
			else if(mtmp->data==&mons[PM_CRONE_LILITH])
				switch(rn2(6)){
					case 0:
					case 1:
					case 2:
						spellnum=MGC_CURSE_ITEMS;
					break;
					case 3:
						spellnum=MGC_WEAKEN_YOU;
					break;
					case 4:
						spellnum=MGC_CURE_SELF;
					break;
					case 5:
						spellnum=MGC_DEATH_TOUCH;
					break;
				}
			else if(mtmp->data==&mons[PM_PALE_NIGHT])
				switch(rn2(5)){
					case 0:
						spellnum=CLC_OPEN_WOUNDS;
					break;
					case 1:
						spellnum=MGC_WEAKEN_YOU;
					break;
					case 2:
						spellnum=MGC_PSI_BOLT;
					break;
					case 3:
						spellnum=MGC_CURSE_ITEMS;
					break;
					case 4:
						spellnum=MGC_DEATH_TOUCH;
					break;
				}
			else if(mtmp->data==&mons[PM_ASMODEUS])
				switch(rn2(9)){
					case 0:
						spellnum=MGC_CURE_SELF;
					break;
					case 1:
						spellnum=CLC_OPEN_WOUNDS;
					break;
					case 2:
						spellnum=CLC_CURSE_ITEMS;
					break;
					case 3:
						spellnum=CLC_LIGHTNING;
					break;
					case 4:
						spellnum=CLC_FIRE_PILLAR;
					break;
					case 5:
						spellnum=CLC_GEYSER;
					break;
					case 6:
						spellnum=MGC_SUMMON_MONS;
					break;
					case 7:
						spellnum=CLC_PARALYZE;
					break;
					case 8:
						spellnum=MGC_DEATH_TOUCH;
					break;
				}
/*			else if(mtmp->data==&mons[PM_CHAOS])
				switch(rn2(10)){
					case 0:
						spellnum=MGC_CURE_SELF;
					break;
					case 1:
					case 2:
					case 3:
						spellnum=CLC_LIGHTNING;
					break;
					case 4:
					case 5:
					case 6:
						spellnum=CLC_FIRE_PILLAR;
					break;
					case 7:
					case 8:
					case 9:
						spellnum=CLC_GEYSER;
					break;
				}
*/		    else spellnum = choose_magic_spell(spellnum);
		}
//		else if(mattk->adtyp == AD_PSON){
//			spellnum = choose_magic_spell(1);
//		}
		else{
			if(mtmp->data==&mons[PM_MINOTAUR_PRIESTESS]){
				switch (d(1,5)+8) {
					case 13:
					spellnum =  CLC_GEYSER;
					break;
					case 12:
					spellnum =  CLC_FIRE_PILLAR;
					break;
					case 11:
					spellnum =  CLC_LIGHTNING;
					break;
					case 10:
					case 9:
					spellnum =  CLC_CURSE_ITEMS;
					break;
					default:
					spellnum =  CLC_OPEN_WOUNDS;
					break;
				}
			}
			else if(mtmp->data==&mons[PM_GNOLL_MATRIARCH]){
				switch (d(1,10)-4) {
					case 6:
					spellnum =  CLC_BLIND_YOU;
					break;
					case 5:
					case 4:
					spellnum =  CLC_PARALYZE;
					break;
					case 3:
					case 2:
					spellnum =  CLC_CONFUSE_YOU;
					break;
					case 1:
					spellnum =  CLC_CURE_SELF;
					break;
					case 0:
					default:
					spellnum =  CLC_OPEN_WOUNDS;
					break;
				}
			}
			else spellnum = choose_clerical_spell(spellnum);
		}
		/* not trying to attack?  don't allow directed spells */
		if (!thinks_it_foundyou) {
		    if (!is_undirected_spell(mattk->adtyp, spellnum) ||
			spell_would_be_useless(mtmp, mattk->adtyp, spellnum)) {
			if (foundyou)
			    impossible("spellcasting monster found you and doesn't know it?");
			return 0;
		    }
		    break;
		}
	    } while(--cnt > 0 &&
		    spell_would_be_useless(mtmp, mattk->adtyp, spellnum));
	    if (cnt == 0) return 0;
	}

	/* monster unable to cast spells? */
	if(mtmp->mcan || mtmp->mspec_used || !ml) {
	    cursetxt(mtmp, is_undirected_spell(mattk->adtyp, spellnum));
	    return(0);
	}

	if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
	    mtmp->mspec_used = 10 - mtmp->m_lev;
	    if (mtmp->mspec_used < 2) mtmp->mspec_used = 2;
	}

	/* monster can cast spells, but is casting a directed spell at the
	   wrong place?  If so, give a message, and return.  Do this *after*
	   penalizing mspec_used. */
	if (!foundyou && thinks_it_foundyou &&
		!is_undirected_spell(mattk->adtyp, spellnum)) {
	    pline("%s casts a spell at %s!",
		canseemon(mtmp) ? Monnam(mtmp) : "Something",
		levl[mtmp->mux][mtmp->muy].typ == WATER
		    ? "empty water" : "thin air");
	    return(0);
	}

	nomul(0);
	if(rn2(ml*10) < (mtmp->mconf ? 100 : 20)) {	/* fumbled attack */
	    if (canseemon(mtmp) && flags.soundok)
		pline_The("air crackles around %s.", mon_nam(mtmp));
	    return(0);
	}
	if (canspotmon(mtmp) || !is_undirected_spell(mattk->adtyp, spellnum)) {
	    pline("%s casts a spell%s!",
		  canspotmon(mtmp) ? Monnam(mtmp) : "Something",
		  is_undirected_spell(mattk->adtyp, spellnum) ? "" :
		  (Invisible && !perceives(mtmp->data) && 
		   (mtmp->mux != u.ux || mtmp->muy != u.uy)) ?
		  " at a spot near you" :
		  (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy)) ?
		  " at your displaced image" :
		  " at you");
	}

/*
 *	As these are spells, the damage is related to the level
 *	of the monster casting the spell.
 */
	if (!foundyou) {
	    dmg = 0;
	    if (mattk->adtyp != AD_SPEL && mattk->adtyp != AD_CLRC) {
		impossible(
	      "%s casting non-hand-to-hand version of hand-to-hand spell %d?",
			   Monnam(mtmp), mattk->adtyp);
		return(0);
	    }
	} else if (mattk->damd)
	    dmg = d((int)((ml/2) + mattk->damn), (int)mattk->damd);
	else dmg = d((int)((ml/2) + 1), 6);
	if (Half_spell_damage) dmg = (dmg+1) / 2;

	ret = 1;

	switch (mattk->adtyp) {

	    case AD_ELEC:
		pline("Lightning crackles around you.");
		if(Shock_resistance) {
			shieldeff(u.ux, u.uy);
			pline("But you resist the effects.");
			dmg = 0;
			if(!rn2(10)) destroy_item(WAND_CLASS, AD_ELEC);
		}
		else{
			destroy_item(WAND_CLASS, AD_ELEC);
			if(!rn2(10)) destroy_item(RING_CLASS, AD_ELEC);
		}
		break;
	    case AD_FIRE:
		pline("You're enveloped in flames.");
		if(Fire_resistance) {
			shieldeff(u.ux, u.uy);
			pline("But you resist the effects.");
			dmg = 0;
			if(!rn2(6)) destroy_item(POTION_CLASS, AD_FIRE);
			if(!rn2(10)) destroy_item(SCROLL_CLASS, AD_FIRE);
		}
		else{
			destroy_item(POTION_CLASS, AD_FIRE);
			if(!rn2(6)) destroy_item(SCROLL_CLASS, AD_FIRE);
			if(!rn2(10)) destroy_item(SPBOOK_CLASS, AD_FIRE);
		}
		burn_away_slime();
		break;
	    case AD_COLD:
		pline("You're covered in frost.");
		if(Cold_resistance) {
			shieldeff(u.ux, u.uy);
			pline("But you resist the effects.");
			dmg = 0;
			if(!rn2(4)) destroy_item(POTION_CLASS, AD_COLD);
		}
		else destroy_item(POTION_CLASS, AD_COLD);
		break;
	    case AD_MAGM:
		You("are hit by a shower of missiles!");
		if(Antimagic) {
			shieldeff(u.ux, u.uy);
			pline_The("missiles bounce off!");
			dmg = 0;
		} else dmg = d((int)mtmp->m_lev/2 + 1,6);
		break;
	    case AD_SPEL:	/* wizard spell */
	    case AD_CLRC:   
			/* clerical spell */
	    {
		if (spellnum>=FIRST_MGC && spellnum<=LAST_MGC)
		    cast_wizard_spell(mtmp, dmg, spellnum);
		else if(spellnum>=FIRST_CLC && spellnum<=LAST_CLC)
		    cast_cleric_spell(mtmp, dmg, spellnum);
		dmg = 0; /* done by the spell casting functions */
		break;
	    }
	}
	if(dmg) mdamageu(mtmp, dmg);
	return(ret);
}

/* monster wizard and cleric spellcasting functions */
/*
   If dmg is zero, then the monster is not casting at you.
   If the monster is intentionally not casting at you, we have previously
   called spell_would_be_useless() and spellnum should always be a valid
   undirected spell.
   If you modify either of these, be sure to change is_undirected_spell()
   and spell_would_be_useless().
 */
STATIC_OVL
void
cast_wizard_spell(mtmp, dmg, spellnum)
struct monst *mtmp;
int dmg;
int spellnum;
{
    if (dmg == 0 && !is_undirected_spell(AD_SPEL, spellnum)) {
	impossible("cast directed wizard spell (%d) with dmg=0?", spellnum);
	return;
    }

    switch (spellnum) {
    case MGC_DEATH_TOUCH:
	pline("Oh no, %s's using the touch of death!", mhe(mtmp));
	if (nonliving(youmonst.data) || is_demon(youmonst.data)) {
	    You("seem no deader than before.");
		dmg = 0; //you don't take damage
	} else if (!Antimagic && rn2(mtmp->m_lev) > 12) {
	    if (Hallucination) {
		You("have an out of body experience.");
	    } else {
		killer_format = KILLED_BY_AN;
		killer = "touch of death";
		dmg = 0; //no additional damage
		done(DIED);
	    }
	} else {
	    if (Antimagic) shieldeff(u.ux, u.uy);
//	    pline("Lucky for you, it didn't work!");
		Your("%s flutters!", body_part(HEART));
		dmg = rnd(mtmp->m_lev); //you still take damage
	}
	break;
    case MGC_CLONE_WIZ:
	if (mtmp->iswiz && flags.no_of_wizards == 1) {
	    pline("Double Trouble...");
	    clonewiz();
	    dmg = 0;
	} else
	    impossible("bad wizard cloning?");
	break;
    case MGC_SUMMON_MONS:
    {
	int count;
	if(u.summonMonster) goto psibolt;
	u.summonMonster = TRUE;
	count = nasty(mtmp);	/* summon something nasty */
	if (mtmp->iswiz)
	    verbalize("Destroy the thief, my pet%s!", plur(count));
	else {
	    const char *mappear =
		(count == 1) ? "A monster appears" : "Monsters appear";

	    /* messages not quite right if plural monsters created but
	       only a single monster is seen */
	    if (Invisible && !perceives(mtmp->data) &&
				    (mtmp->mux != u.ux || mtmp->muy != u.uy))
		pline("%s around a spot near you!", mappear);
	    else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
		pline("%s around your displaced image!", mappear);
	    else
		pline("%s from nowhere!", mappear);
	}
	dmg = 0;
	break;
    }
    case MGC_AGGRAVATION:
	You_feel("that monsters are aware of your presence.");
	aggravate();
	dmg = 0;
	break;
    case MGC_CURSE_ITEMS:
	You_feel("as if you need some help.");
	rndcurse();
	dmg =  rnd(mtmp->m_lev/2+1);//take damage from the curse
	break;
    case MGC_DESTRY_ARMR:
	if (Antimagic) {
	    shieldeff(u.ux, u.uy);
	    pline("A field of force surrounds you!");
		dmg = 0;//your antimagic cancelled the damage.
	} else if (!destroy_arm(some_armor(&youmonst))) {
	    Your("skin cracks!"); //take damage from the destructive energy
	}
	else dmg = 0;//your armor absorbed the damage
	break;
    case MGC_WEAKEN_YOU:		/* drain strength */
	if (Antimagic) {
	    shieldeff(u.ux, u.uy);
	    You_feel("momentarily weakened.");
	} else {
	    You("suddenly feel weaker!");
	    dmg = mtmp->m_lev - 6;
	    if (Half_spell_damage) dmg = (dmg + 1) / 2;
	    losestr(rnd(dmg));
	    if (u.uhp < 1)
		done_in_by(mtmp);
	}
	dmg = 0;
	break;
    case MGC_DISAPPEAR:		/* makes self invisible */
	if (!mtmp->minvis && !mtmp->invis_blkd) {
	    if (canseemon(mtmp))
		pline("%s suddenly %s!", Monnam(mtmp),
		      !See_invisible ? "disappears" : "becomes transparent");
	    mon_set_minvis(mtmp);
	    dmg = 0;
	} else
	    impossible("no reason for monster to cast disappear spell?");
	break;
    case MGC_STUN_YOU:
	if (Antimagic || Free_action) {
	    shieldeff(u.ux, u.uy);
	    if (!Stunned)
		You_feel("momentarily disoriented.");
	    make_stunned(1L, FALSE);
	} else {
	    You(Stunned ? "struggle to keep your balance." : "reel...");
	    dmg = d(ACURR(A_DEX) < 12 ? 6 : 4, 4);
	    if (Half_spell_damage) dmg = (dmg + 1) / 2;
	    make_stunned(HStun + dmg, FALSE);
	}
	dmg = 0;
	break;
    case MGC_HASTE_SELF:
	mon_adjust_speed(mtmp, 1, (struct obj *)0);
	dmg = 0;
	break;
    case MGC_CURE_SELF:
	if (mtmp->mhp < mtmp->mhpmax) {
	    if (canseemon(mtmp))
		pline("%s looks better.", Monnam(mtmp));
	    /* note: player healing does 6d4; this used to do 1d8; then it did 3d6 */
	    if ((mtmp->mhp += min(d(mtmp->m_lev/3+1,8), 10)) > mtmp->mhpmax)
			mtmp->mhp = mtmp->mhpmax;
	    dmg = 0;
	}
	break;
    case MGC_PSI_BOLT:
	psibolt:
	/* prior to 3.4.0 Antimagic was setting the damage to 1--this
	   made the spell virtually harmless to players with magic res. */
	if (Antimagic) {
	    shieldeff(u.ux, u.uy);
	    dmg = (dmg + 1) / 2;
		if(dmg > 25) dmg = 25;
	}
	if (dmg <= 5)
	    You("get a slight %sache.", body_part(HEAD));
	else if (dmg <= 10)
	    Your("brain is on fire!");
	else if (dmg <= 20)
	    Your("%s suddenly aches painfully!", body_part(HEAD));
	else{
	    Your("%s suddenly aches very painfully!", body_part(HEAD));
		if(dmg > 50) dmg = 50;
	}
	break;
    default:
	impossible("mcastu: invalid magic spell (%d)", spellnum);
	dmg = 0;
	break;
    }

    if (dmg) mdamageu(mtmp, dmg);
}

STATIC_OVL
void
cast_cleric_spell(mtmp, dmg, spellnum)
struct monst *mtmp;
int dmg;
int spellnum;
{
    if (dmg == 0 && !is_undirected_spell(AD_CLRC, spellnum)) {
	impossible("cast directed cleric spell (%d) with dmg=0?", spellnum);
	return;
    }

    switch (spellnum) {
    case CLC_GEYSER:
	/* this is physical damage, not magical damage */
	pline("A sudden geyser slams into you from nowhere!");
	dmg = d(8, 6);
	water_damage(invent, FALSE, FALSE, FALSE);
	if (Half_physical_damage) dmg = (dmg + 1) / 2;
	break;
    case CLC_FIRE_PILLAR:
	pline("A pillar of fire strikes all around you!");
	if (Fire_resistance) {
	    shieldeff(u.ux, u.uy);
	    dmg = 0;
	} else
	    dmg = d(8, 6);
	if (Half_spell_damage) dmg = (dmg + 1) / 2;
	burn_away_slime();
	(void) burnarmor(&youmonst);
	destroy_item(SCROLL_CLASS, AD_FIRE);
	destroy_item(POTION_CLASS, AD_FIRE);
	destroy_item(SPBOOK_CLASS, AD_FIRE);
	(void) burn_floor_paper(u.ux, u.uy, TRUE, FALSE);
	break;
    case CLC_LIGHTNING:
    {
	boolean reflects;

	pline("A bolt of lightning strikes down at you from above!");
	reflects = ureflects("It bounces off your %s%s.", "");
	if (reflects || Shock_resistance) {
	    shieldeff(u.ux, u.uy);
	    dmg = 0;
	    if (reflects)
		break;
	} else
	    dmg = d(8, 6);
	if (Half_spell_damage) dmg = (dmg + 1) / 2;
	destroy_item(WAND_CLASS, AD_ELEC);
	destroy_item(RING_CLASS, AD_ELEC);
	break;
    }
    case CLC_CURSE_ITEMS:
	You_feel("as if you need some help.");
	rndcurse();
	dmg = 0;
	break;
    case CLC_INSECTS:
      {
	/* Try for insects, and if there are none
	   left, go for (sticks to) snakes.  -3. */
	struct permonst *pm = mkclass(S_ANT,0);
	struct monst *mtmp2 = (struct monst *)0;
	char let = (pm ? S_ANT : S_SNAKE);
	boolean success;
	int i;
	coord bypos;
	int quan;

	quan = (mtmp->m_lev < 2) ? 1 : rnd((int)mtmp->m_lev / 2);
	if (quan < 3) quan = 3;
	success = pm ? TRUE : FALSE;
	for (i = 0; i <= quan; i++) {
	    if (!enexto(&bypos, mtmp->mux, mtmp->muy, mtmp->data))
		break;
	    if ((pm = mkclass(let,0)) != 0 &&
		    (mtmp2 = makemon(pm, bypos.x, bypos.y, NO_MM_FLAGS)) != 0) {
		success = TRUE;
		mtmp2->msleeping = mtmp2->mpeaceful = mtmp2->mtame = 0;
		set_malign(mtmp2);
	    }
	}
	/* Not quite right:
         * -- message doesn't always make sense for unseen caster (particularly
	 *    the first message)
         * -- message assumes plural monsters summoned (non-plural should be
         *    very rare, unlike in nasty())
         * -- message assumes plural monsters seen
         */
	if (!success)
	    pline("%s casts at a clump of sticks, but nothing happens.",
		Monnam(mtmp));
	else if (let == S_SNAKE)
	    pline("%s transforms a clump of sticks into snakes!",
		Monnam(mtmp));
	else if (Invisible && !perceives(mtmp->data) &&
				(mtmp->mux != u.ux || mtmp->muy != u.uy))
	    pline("%s summons insects around a spot near you!",
		Monnam(mtmp));
	else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
	    pline("%s summons insects around your displaced image!",
		Monnam(mtmp));
	else
	    pline("%s summons insects!", Monnam(mtmp));
	dmg = 0;
	break;
      }
    case CLC_BLIND_YOU:
	/* note: resists_blnd() doesn't apply here */
	if (!Blinded) {
	    int num_eyes = eyecount(youmonst.data);
	    pline("Scales cover your %s!",
		  (num_eyes == 1) ?
		  body_part(EYE) : makeplural(body_part(EYE)));
	    make_blinded(Half_spell_damage ? 100L : 200L, FALSE);
	    if (!Blind) Your(vision_clears);
	    dmg = 0;
	} else
	    impossible("no reason for monster to cast blindness spell?");
	break;
    case CLC_PARALYZE:
	if (Antimagic || Free_action) {
	    shieldeff(u.ux, u.uy);
	    if (multi >= 0)
		You("stiffen briefly.");
	    nomul(-1);
	} else {
	    if (multi >= 0)
		You("are frozen in place!");
	    dmg = 4 + (int)mtmp->m_lev;
	    if (Half_spell_damage) dmg = (dmg + 1) / 2;
	    nomul(-dmg);
	}
	dmg = 0;
	break;
    case CLC_CONFUSE_YOU:
	if (Antimagic) {
	    shieldeff(u.ux, u.uy);
	    You_feel("momentarily dizzy.");
	} else {
	    boolean oldprop = !!Confusion;

	    dmg = (int)mtmp->m_lev;
	    if (Half_spell_damage) dmg = (dmg + 1) / 2;
	    make_confused(HConfusion + dmg, TRUE);
	    if (Hallucination)
		You_feel("%s!", oldprop ? "trippier" : "trippy");
	    else
		You_feel("%sconfused!", oldprop ? "more " : "");
	}
	dmg = 0;
	break;
    case CLC_CURE_SELF:
	if (mtmp->mhp < mtmp->mhpmax) {
	    if (canseemon(mtmp))
		pline("%s looks better.", Monnam(mtmp));
	    /* note: player healing does 6d4; this used to do 1d8 */
	    if ((mtmp->mhp += d(3,6)) > mtmp->mhpmax)
		mtmp->mhp = mtmp->mhpmax;
	    dmg = 0;
	}
	break;
    case CLC_OPEN_WOUNDS:
	if (Antimagic) {
	    shieldeff(u.ux, u.uy);
	    dmg = (dmg + 1) / 2;
	}
	if (dmg <= 5)
	    Your("skin itches badly for a moment.");
	else if (dmg <= 10)
	    pline("Wounds appear on your body!");
	else if (dmg <= 20)
	    pline("Severe wounds appear on your body!");
	else
	    Your("body is covered with painful wounds!");
	break;
    default:
	impossible("mcastu: invalid clerical spell (%d)", spellnum);
	dmg = 0;
	break;
    }

    if (dmg) mdamageu(mtmp, dmg);
}

STATIC_DCL
boolean
is_undirected_spell(adtyp, spellnum)
unsigned int adtyp;
int spellnum;
{
    if (adtyp == AD_SPEL) {
	switch (spellnum) {
	case MGC_CLONE_WIZ:
	case MGC_SUMMON_MONS:
	case MGC_AGGRAVATION:
	case MGC_DISAPPEAR:
	case MGC_HASTE_SELF:
	case MGC_CURE_SELF:
	    return TRUE;
	default:
	    break;
	}
    } else if (adtyp == AD_CLRC) {
	switch (spellnum) {
	case CLC_INSECTS:
	case CLC_CURE_SELF:
	    return TRUE;
	default:
	    break;
	}
    }
    return FALSE;
}

/* Some spells are useless under some circumstances. */
STATIC_DCL
boolean
spell_would_be_useless(mtmp, adtyp, spellnum)
struct monst *mtmp;
unsigned int adtyp;
int spellnum;
{
    /* Some spells don't require the player to really be there and can be cast
     * by the monster when you're invisible, yet still shouldn't be cast when
     * the monster doesn't even think you're there.
     * This check isn't quite right because it always uses your real position.
     * We really want something like "if the monster could see mux, muy".
     */
    boolean mcouldseeu = couldsee(mtmp->mx, mtmp->my);

    if (adtyp == AD_SPEL) {
	/* aggravate monsters, etc. won't be cast by peaceful monsters */
	if (mtmp->mpeaceful && (spellnum == MGC_AGGRAVATION ||
		spellnum == MGC_SUMMON_MONS || spellnum == MGC_CLONE_WIZ))
	    return TRUE;
	/* haste self when already fast */
	if (mtmp->permspeed == MFAST && spellnum == MGC_HASTE_SELF)
	    return TRUE;
	/* invisibility when already invisible */
	if ((mtmp->minvis || mtmp->invis_blkd) && spellnum == MGC_DISAPPEAR)
	    return TRUE;
	/* peaceful monster won't cast invisibility if you can't see invisible,
	   same as when monsters drink potions of invisibility.  This doesn't
	   really make a lot of sense, but lets the player avoid hitting
	   peaceful monsters by mistake */
	if (mtmp->mpeaceful && !See_invisible && spellnum == MGC_DISAPPEAR)
	    return TRUE;
	/* healing when already healed */
	if (mtmp->mhp == mtmp->mhpmax && spellnum == MGC_CURE_SELF)
	    return TRUE;
	/* don't summon monsters if it doesn't think you're around */
	if (!mcouldseeu && (spellnum == MGC_SUMMON_MONS ||
		(!mtmp->iswiz && spellnum == MGC_CLONE_WIZ)))
	    return TRUE;
	if ((!mtmp->iswiz || flags.no_of_wizards > 1)
						&& spellnum == MGC_CLONE_WIZ)
	    return TRUE;
    } else if (adtyp == AD_CLRC) {
	/* summon insects/sticks to snakes won't be cast by peaceful monsters */
	if (mtmp->mpeaceful && spellnum == CLC_INSECTS)
	    return TRUE;
	/* healing when already healed */
	if (mtmp->mhp == mtmp->mhpmax && spellnum == CLC_CURE_SELF)
	    return TRUE;
	/* don't summon insects if it doesn't think you're around */
	if (!mcouldseeu && spellnum == CLC_INSECTS)
	    return TRUE;
	/* blindness spell on blinded player */
	if (Blinded && spellnum == CLC_BLIND_YOU)
	    return TRUE;
    }
    return FALSE;
}

#endif /* OVLB */
#ifdef OVL0

/* convert 1..10 to 0..9; add 10 for second group (spell casting) */
#define ad_to_typ(k) (10 + (int)k - 1)

int
buzzmu(mtmp, mattk)		/* monster uses spell (ranged) */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	/* don't print constant stream of curse messages for 'normal'
	   spellcasting monsters at range */
	if (mattk->adtyp > AD_SPC2)
	    return(0);

	if (mtmp->mcan) {
	    cursetxt(mtmp, FALSE);
	    return(0);
	}
	if(lined_up(mtmp) && rn2(3)) {
	    nomul(0);
	    if(mattk->adtyp && (mattk->adtyp < 11)) { /* no cf unsigned >0 */
		if(canseemon(mtmp))
		    pline("%s zaps you with a %s!", Monnam(mtmp),
			  flash_types[ad_to_typ(mattk->adtyp)]);
		buzz(-ad_to_typ(mattk->adtyp), (int)mattk->damn,
		     mtmp->mx, mtmp->my, sgn(tbx), sgn(tby),0);
	    } else impossible("Monster spell %d cast", mattk->adtyp-1);
	}
	return(1);
}

#endif /* OVL0 */

/*mcastu.c*/
