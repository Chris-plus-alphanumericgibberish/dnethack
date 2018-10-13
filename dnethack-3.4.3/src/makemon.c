/*	SCCS Id: @(#)makemon.c	3.4	2003/09/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "epri.h"
#include "emin.h"
#include "edog.h"
#ifdef REINCARNATION
#include <ctype.h>
#endif

struct monst zeromonst;

/* this assumes that a human quest leader or nemesis is an archetype
   of the corresponding role; that isn't so for some roles (tourist
   for instance) but is for the priests and monks we use it for... */
#define quest_mon_represents_role(mptr,role_pm) \
		(mptr->mlet == S_HUMAN && Role_if(role_pm) && \
		  (mptr == &mons[urole.ldrnum] || mptr->msound == MS_NEMESIS))

#ifdef OVL0
STATIC_DCL boolean FDECL(uncommon, (int));
STATIC_DCL int FDECL(align_shift, (struct permonst *));
#endif /* OVL0 */
STATIC_DCL struct permonst * NDECL(roguemonst);
STATIC_DCL boolean FDECL(wrong_elem_type, (struct permonst *));
STATIC_DCL void FDECL(m_initthrow,(struct monst *,int,int));
STATIC_DCL void FDECL(m_initweap,(struct monst *));
#ifdef OVL1
STATIC_DCL void FDECL(m_initinv,(struct monst *));
#endif /* OVL1 */

extern const int monstr[];
int curhouse = 0;
int undeadfaction = 0;
int zombiepm = -1;
int skeletpm = -1;

#define m_initsgrp(mtmp, x, y)	m_initgrp(mtmp, x, y, 3)
#define m_initlgrp(mtmp, x, y)	m_initgrp(mtmp, x, y, 10)

#ifdef OVLB
boolean
is_home_elemental(ptr)
register struct permonst *ptr;
{
	if (ptr->mlet == S_ELEMENTAL)
	    switch (monsndx(ptr)) {
		case PM_AIR_ELEMENTAL: return Is_airlevel(&u.uz);
		case PM_LIGHTNING_PARAELEMENTAL: return Is_airlevel(&u.uz);
		case PM_FIRE_ELEMENTAL: return Is_firelevel(&u.uz);
		case PM_POISON_PARAELEMENTAL: return Is_firelevel(&u.uz);
		case PM_EARTH_ELEMENTAL: return Is_earthlevel(&u.uz);
		case PM_ACID_PARAELEMENTAL: return Is_earthlevel(&u.uz);
		case PM_WATER_ELEMENTAL: return Is_waterlevel(&u.uz);
		case PM_ICE_PARAELEMENTAL: return Is_waterlevel(&u.uz);
		}
	return FALSE;
}

/*
 * Return true if the given monster cannot exist on this elemental level.
 */
STATIC_OVL boolean
wrong_elem_type(ptr)
    register struct permonst *ptr;
{
    if (ptr->mlet == S_ELEMENTAL) {
	return((boolean)(!is_home_elemental(ptr)));
    } else if (Is_earthlevel(&u.uz)) {
	/* no restrictions? */
    } else if (Is_waterlevel(&u.uz)) {
	/* just monsters that can swim */
	if(!is_swimmer(ptr)) return TRUE;
    } else if (Is_firelevel(&u.uz)) {
	if (!pm_resistance(ptr,MR_FIRE)) return TRUE;
    } else if (Is_airlevel(&u.uz)) {
	if(!(is_flyer(ptr) && ptr->mlet != S_TRAPPER) && !is_floater(ptr)
	   && !amorphous(ptr) && !noncorporeal(ptr) && !is_whirly(ptr))
	    return TRUE;
    }
    return FALSE;
}

void
m_initgrp(mtmp, x, y, n)	/* make a group just like mtmp */
register struct monst *mtmp;
register int x, y, n;
{
	coord mm;
	int mndx;
	register int cnt = rnd(n);
	struct monst *mon;
#if defined(__GNUC__) && (defined(HPUX) || defined(DGUX))
	/* There is an unresolved problem with several people finding that
	 * the game hangs eating CPU; if interrupted and restored, the level
	 * will be filled with monsters.  Of those reports giving system type,
	 * there were two DG/UX and two HP-UX, all using gcc as the compiler.
	 * hcroft@hpopb1.cern.ch, using gcc 2.6.3 on HP-UX, says that the
	 * problem went away for him and another reporter-to-newsgroup
	 * after adding this debugging code.  This has almost got to be a
	 * compiler bug, but until somebody tracks it down and gets it fixed,
	 * might as well go with the "but it went away when I tried to find
	 * it" code.
	 */
	int cnttmp,cntdiv;
	
	if(!mtmp) return; //called with bum monster, return;
	
	cnttmp = cnt;
# ifdef DEBUG
	pline("init group call x=%d,y=%d,n=%d,cnt=%d.", x, y, n, cnt);
# endif
	cntdiv = ((u.ulevel < 3) ? 4 : (u.ulevel < 5) ? 2 : 1);
#endif
	/* Tuning: cut down on swarming at low character levels [mrs] */
	cnt /= (u.ulevel < 3) ? 4 : (u.ulevel < 5) ? 2 : 1;
#if defined(__GNUC__) && (defined(HPUX) || defined(DGUX))
	if (cnt != (cnttmp/cntdiv)) {
		pline("cnt=%d using %d, cnttmp=%d, cntdiv=%d", cnt,
			(u.ulevel < 3) ? 4 : (u.ulevel < 5) ? 2 : 1,
			cnttmp, cntdiv);
	}
#endif
	if(!cnt) cnt++;
#if defined(__GNUC__) && (defined(HPUX) || defined(DGUX))
	if (cnt < 0) cnt = 1;
	if (cnt > 10) cnt = 10;
#endif

	mm.x = x;
	mm.y = y;
	mndx = monsndx(mtmp->data);
	if(!(u.uevent.uaxus_foe) || mndx > PM_QUINON || mndx < PM_MONOTON){
	 while(cnt--) {
		if (peace_minded(mtmp->data) && !is_derived_undead_mon(mtmp)) continue;
		/* Don't create groups of peaceful monsters since they'll get
		 * in our way.  If the monster has a percentage chance so some
		 * are peaceful and some are not, the result will just be a
		 * smaller group.
		 */
		 	/* if caller wants random locations, do one here */
		if(x == 0 && y == 0) {
			int tryct = 0;	/* careful with bigrooms */
			struct monst fakemon = {0};

			fakemon.data = mtmp->data;	/* set up for goodpos */
			do {
				mm.x = rn1(COLNO-3,2);
				mm.y = rn2(ROWNO);
			} while(!goodpos(mm.x, mm.y, &fakemon, NO_MM_FLAGS) &&
				(tryct++ < 200 && ((tryct < 100 && couldsee(mm.x, mm.y)) || (tryct < 150 && cansee(mm.x, mm.y)) || distmin(mm.x,mm.y,u.ux,u.uy) < BOLT_LIM)));
			if(!goodpos(mm.x, mm.y, &fakemon, NO_MM_FLAGS))
				return;
		}
		if (enexto(&mm, mm.x, mm.y, mtmp->data)) {
		    mon = makemon(mtmp->data, mm.x, mm.y, NO_MM_FLAGS);
		    if (mon) {
			    mon->mpeaceful = FALSE;
			    mon->mavenge = 0;
			    set_malign(mon);
		    }
		    /* Undo the second peace_minded() check in makemon(); if the
		     * monster turned out to be peaceful the first time we
		     * didn't create it at all; we don't want a second check.
		     */
		}
	 }
	}
	else{
	 while(cnt-- > 0) {
		 	/* if caller wants random locations, do one here */
		if(x == 0 && y == 0) {
			int tryct = 0;	/* careful with bigrooms */
			struct monst fakemon = {0};

			fakemon.data = mtmp->data;	/* set up for goodpos */
			do {
				mm.x = rn1(COLNO-3,2);
				mm.y = rn2(ROWNO);
			} while(!goodpos(mm.x, mm.y, &fakemon, NO_MM_FLAGS) ||
				(tryct++ < 150 && ((tryct < 50 && couldsee(mm.x, mm.y)) || (tryct < 100 && cansee(mm.x, mm.y)) || distmin(mm.x,mm.y,u.ux,u.uy) < BOLT_LIM)));
			if(!goodpos(mm.x, mm.y, &fakemon, NO_MM_FLAGS))
				return;
		}
		if (enexto(&mm, mm.x, mm.y, mtmp->data)) {
		    mon = makemon(mtmp->data, mm.x, mm.y, NO_MM_FLAGS);
		    mon->mpeaceful = 1;
		    mon->mavenge = 0;
		    set_malign(mon);
			mtmp->mpeaceful = 0;
		}
	 }
	}
}

STATIC_OVL
void
m_initthrow(mtmp,otyp,oquan)
struct monst *mtmp;
int otyp,oquan;
{
	register struct obj *otmp;

	otmp = mksobj(otyp, TRUE, FALSE);
	otmp->quan = (long) rn1(oquan, 3);
	otmp->owt = weight(otmp);
	otmp->objsize = mtmp->data->msize;
	if(otyp == ORCISH_ARROW) otmp->opoisoned = OPOISON_BASIC;
	else if(otyp == DROVEN_BOLT && is_drow(mtmp->data)) otmp->opoisoned = OPOISON_SLEEP;
	fix_object(otmp);
	(void) mpickobj(mtmp, otmp);
}

#endif /* OVLB */
#ifdef OVL2

static const int ANGELTWOHANDERCUT = 4;
static NEARDATA int angelwepsbase[] = {
	SABER, LONG_SWORD, LONG_SWORD, SABER,
	TWO_HANDED_SWORD, GLAIVE, LANCE,
	LUCERN_HAMMER,
};
static NEARDATA int angelweps[] = {
	ART_DEMONBANE, ART_SUNSWORD, ART_SWORD_OF_ERATHAOL, ART_SABER_OF_SABAOTH,
	ART_SWORD_OF_ONOEL, ART_GLAIVE_OF_SHAMSIEL, ART_LANCE_OF_URIEL,
	ART_HAMMER_OF_BARQUIEL,
};

STATIC_OVL void
m_initweap(mtmp)
register struct monst *mtmp;
{
	register struct permonst *ptr = mtmp->data;
	register int mm = monsndx(ptr);
	struct obj *otmp;
	int chance = 0;

#ifdef REINCARNATION
	if (Is_rogue_level(&u.uz)) return;
#endif
/*
 *	first a few special cases:
 *
 *		giants get a boulder to throw sometimes.
 *		ettins get clubs
 *		kobolds get darts to throw
 *		centaurs get some sort of bow & arrows or bolts
 *		soldiers get all sorts of things.
 */
	if(mm==PM_ARGENTUM_GOLEM){
	    struct obj *otmp = mksobj(SILVER_ARROW, TRUE, FALSE);
	    otmp->blessed = FALSE;
	    otmp->cursed = FALSE;
		otmp->quan += 18+rnd(8);
		(void) mpickobj(mtmp,otmp);
		switch(rnd(12)){
			case 1:
				mongets(mtmp,SABER);
			break;
			case 2:
				mongets(mtmp,TRIDENT);
			break;
			case 3:
				mongets(mtmp,AXE);
			break;
			case 4:
				mongets(mtmp,STILETTO);
			break;
			case 5:
				mongets(mtmp,ATHAME);
			break;
			case 6:
				mongets(mtmp,SHORT_SWORD);
			break;
			case 7:
				mongets(mtmp,SCIMITAR);
			break;
			case 8:
				mongets(mtmp,RAPIER);
			break;
			case 9:
				mongets(mtmp,LONG_SWORD);
			break;
			case 10:
				mongets(mtmp,TWO_HANDED_SWORD);
			break;
			case 11:
				mongets(mtmp,MACE);
			break;
			case 12:
				mongets(mtmp,MORNING_STAR);
			break;
		}
		return;
	}
	if(mm==PM_IRON_GOLEM){
		switch(rnd(3)){
			case 1:
				mongets(mtmp,GLAIVE);
				mongets(mtmp,BATTLE_AXE);
			break;
			case 2:
				mongets(mtmp,SPETUM);
				mongets(mtmp,DWARVISH_MATTOCK);
			break;
			case 3:
				mongets(mtmp,BEC_DE_CORBIN);
				mongets(mtmp,WAR_HAMMER);
			break;
		}
		return;
	}
	if(mm==PM_GROVE_GUARDIAN){
		switch(rnd(6)){
			case 1:
				otmp = mksobj(MOON_AXE, TRUE, FALSE);
				(void) mpickobj(mtmp,otmp);
			break;
			case 2:
				otmp = mksobj(KHAKKHARA, TRUE, FALSE);
				(void) mpickobj(mtmp,otmp);
			break;
			case 3:
				mongets(mtmp,HIGH_ELVEN_WARSWORD);
			break;
			case 4:
				mongets(mtmp,RAPIER);
			break;
			case 5:
				mongets(mtmp,ELVEN_BROADSWORD);
			break;
			case 6:
				mongets(mtmp,AKLYS);
			break;
		}
		return;
	}
	if(mm==PM_GLASS_GOLEM){
		switch(rnd(12)){
			case 1:
				mongets(mtmp,SABER);
			break;
			case 2:
				mongets(mtmp,SICKLE);
			break;
			case 3:
				mongets(mtmp,AXE);
			break;
			case 4:
				mongets(mtmp,STILETTO);
			break;
			case 5:
				mongets(mtmp,CRYSTAL_SWORD);
			break;
			case 6:
				mongets(mtmp,BROADSWORD);
			break;
			case 7:
				mongets(mtmp,SCIMITAR);
			break;
			case 8:
				mongets(mtmp,RAPIER);
			break;
			case 9:
				mongets(mtmp,LONG_SWORD);
			break;
			case 10:
				mongets(mtmp,TWO_HANDED_SWORD);
			break;
			case 11:
				mongets(mtmp,PARTISAN);
			break;
			case 12:
				mongets(mtmp,GLAIVE);
			break;
		}
		return;
	}
	if(mm==PM_TREASURY_GOLEM){
		switch(rnd(3)){
			case 1:
				mongets(mtmp,GLAIVE);
				mongets(mtmp,BATTLE_AXE);
			break;
			case 2:
				mongets(mtmp,SPETUM);
				mongets(mtmp,DWARVISH_MATTOCK);
			break;
			case 3:
				mongets(mtmp,BEC_DE_CORBIN);
				mongets(mtmp,WAR_HAMMER);
			break;
		}
		return;
	}
	if(mm==PM_GOLD_GOLEM){
		switch(rnd(6)){
			case 1:
				mongets(mtmp,STILETTO);
			break;
			case 2:
				mongets(mtmp,AXE);
			break;
			case 3:
				mongets(mtmp,SICKLE);
			break;
			case 4:
				mongets(mtmp,SHORT_SWORD);
			break;
			case 5:
				mongets(mtmp,SABER);
			break;
			case 6:
				mongets(mtmp,WAR_HAMMER);
			break;
		}
		return;
	}

/*	if(mm==PM_SERVANT_OF_THE_UNKNOWN_GOD){
		mongets(mtmp, CLUB);
		return;
	}
*/
	switch (ptr->mlet) {
	    case S_GIANT:
		if (rn2(2)) (void)mongets(mtmp, (mm != PM_ETTIN) ?
				    BOULDER : CLUB);
		if(mm == PM_LORD_SURTUR){
			otmp = mksobj(TWO_HANDED_SWORD, TRUE, FALSE);
			otmp = oname(otmp, artiname(ART_SOL_VALTIVA));
			otmp->oerodeproof = TRUE;
			otmp->blessed = FALSE;
			otmp->cursed = FALSE;
			(void) mpickobj(mtmp,otmp);
		}
		break;
		case S_IMP:{
		    int w1 = 0, w2 = 0;
			switch (mm){
			case PM_TENGU:
				if((Role_if(PM_SAMURAI) && In_quest(&u.uz)) || !rn2(20)){
					if(mtmp->female){
						otmp = mksobj(TWO_HANDED_SWORD, TRUE, FALSE);
						otmp = oname(otmp, artiname(ART_SOL_VALTIVA));
						otmp->oerodeproof = TRUE;
						otmp->blessed = FALSE;
						otmp->cursed = FALSE;
						(void) mpickobj(mtmp,otmp);
						(void)mongets(mtmp, KNIFE);
					} else {
						(void)mongets(mtmp, KATANA);
						(void)mongets(mtmp, SHORT_SWORD);
						(void)mongets(mtmp, YUMI);
						m_initthrow(mtmp, YA, 10);
					}
					(void)mongets(mtmp, BANDED_MAIL);
					(void)mongets(mtmp, HELMET);
					(void)mongets(mtmp, HIGH_BOOTS);
				}
			break;
			case PM_LEGION_DEVIL_GRUNT:
			      w1 = !rn2(4) ? PARTISAN : !rn2(3) ? HALBERD : rn2(2) ? BILL_GUISARME : BEC_DE_CORBIN;
			      w2 = rn2(2) ? DAGGER : KNIFE;
				  (void)mongets(mtmp, CROSSBOW);
				  m_initthrow(mtmp, CROSSBOW_BOLT, 5);
				  (void)mongets(mtmp, SCALE_MAIL);
			break;
			case PM_LEGION_DEVIL_SOLDIER:
			      w1 = !rn2(4) ? PARTISAN : !rn2(3) ? HALBERD : rn2(2) ? BILL_GUISARME : BEC_DE_CORBIN;
			      w2 = rn2(2) ? DAGGER : KNIFE;
				  (void)mongets(mtmp, CROSSBOW);
				  m_initthrow(mtmp, CROSSBOW_BOLT, 10);
				  (void)mongets(mtmp, rnd(WAN_LIGHTNING-WAN_CREATE_MONSTER)+WAN_CREATE_MONSTER);
				  (void)mongets(mtmp, SCALE_MAIL);
			break;
			case PM_LEGION_DEVIL_SERGEANT:
			      w1 = !rn2(4) ? PARTISAN : !rn2(3) ? HALBERD : rn2(2) ? BILL_GUISARME : BEC_DE_CORBIN;
			      w2 = rn2(2) ? DAGGER : KNIFE;
				  (void)mongets(mtmp, CROSSBOW);
				  m_initthrow(mtmp, CROSSBOW_BOLT, 20);
				  (void)mongets(mtmp, rnd_attack_wand(mtmp));
				  (void)mongets(mtmp, rnd(WAN_LIGHTNING-WAN_CREATE_MONSTER)+WAN_CREATE_MONSTER);
				  (void)mongets(mtmp, SCALE_MAIL);
			break;
			case PM_LEGION_DEVIL_CAPTAIN:
			      w1 = !rn2(4) ? PARTISAN : !rn2(3) ? HALBERD : rn2(2) ? BILL_GUISARME : BEC_DE_CORBIN;
			      w2 = rn2(2) ? DAGGER : KNIFE;
				  (void)mongets(mtmp, BOW);
				  m_initthrow(mtmp, ARROW, 30);
				  (void)mongets(mtmp, rnd_attack_wand(mtmp));
				  (void)mongets(mtmp, rnd_utility_wand(mtmp));
				  (void)mongets(mtmp, rnd(WAN_LIGHTNING-WAN_CREATE_MONSTER)+WAN_CREATE_MONSTER);
				  (void)mongets(mtmp, SCALE_MAIL);
			break;
			case PM_GLASYA:
				otmp = mksobj(BULLWHIP, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_CARESS));
				otmp->spe = 9;
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);
				mongets(mtmp, find_gcirclet());
				mongets(mtmp, GENTLEWOMAN_S_DRESS);
				mongets(mtmp, CRYSTAL_BOOTS);
				mongets(mtmp, CRYSTAL_GAUNTLETS);
				// mongets(mtmp, RAZOR_WIRE);
			break;
			}
		    if (w1) (void)mongets(mtmp, w1);
		    if (w2) (void)mongets(mtmp, w2);
		break;}
	    case S_HUMAN:
			if(mm == PM_ELDER_PRIEST){
				otmp = mksobj(QUARTERSTAFF, TRUE, FALSE);
				bless(otmp);
				otmp->spe = 3;
				(void) mpickobj(mtmp, otmp);
				otmp = mksobj(CLOAK_OF_MAGIC_RESISTANCE, TRUE, FALSE);
				bless(otmp);
				otmp->spe = 3;
				(void) mpickobj(mtmp, otmp);
				otmp = mksobj(POT_FULL_HEALING, TRUE, FALSE);
				bless(otmp);
				(void) mpickobj(mtmp, otmp);
				otmp = mksobj(POT_EXTRA_HEALING, TRUE, FALSE);
				bless(otmp);
				otmp->quan = 3;
				otmp->owt = weight(otmp);
				(void) mpickobj(mtmp, otmp);
				otmp = mksobj(LEATHER_DRUM, TRUE, FALSE);
				bless(otmp);
				(void) mpickobj(mtmp, otmp);
				otmp = mksobj(WOODEN_FLUTE, TRUE, FALSE);
				bless(otmp);
				(void) mpickobj(mtmp, otmp);
				return;//no random stuff!
			}
			else if(mm == PM_DAEMON){
				otmp = mksobj(SABER, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_ICONOCLAST));
				otmp->spe = 9;
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);

				(void)mongets(mtmp, ELVEN_MITHRIL_COAT);
				(void)mongets(mtmp, ORCISH_SHIELD);
				(void)mongets(mtmp, CLOAK_OF_MAGIC_RESISTANCE);
				(void)mongets(mtmp, DROVEN_HELM);
				(void)mongets(mtmp, ELVEN_BOOTS);
				(void)mongets(mtmp, ELVEN_DAGGER);
				(void)mongets(mtmp, ORCISH_DAGGER);
				(void)mongets(mtmp, DAGGER);
				(void)mongets(mtmp, STILETTO);

				otmp = mksobj(POT_FULL_HEALING, TRUE, FALSE);
				bless(otmp);
				(void) mpickobj(mtmp, otmp);
				// otmp = mksobj(POT_EXTRA_HEALING, TRUE, FALSE);
				// bless(otmp);
				// otmp->quan = 3;
				// (void) mpickobj(mtmp, otmp);
			}
			else if(mm == PM_BASTARD_OF_THE_BOREAL_VALLEY){
				otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
				otmp->obj_material = METAL;
				otmp->spe = 0;
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				fix_object(otmp);
				(void) mpickobj(mtmp,otmp);

				otmp = mksobj(GAUNTLETS, FALSE, FALSE);
				otmp->obj_material = METAL;
				otmp->spe = 0;
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				fix_object(otmp);
				(void) mpickobj(mtmp,otmp);
				
				otmp = mksobj(PLATE_MAIL, FALSE, FALSE);
				otmp->obj_material = METAL;
				otmp->spe = 0;
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				fix_object(otmp);
				(void) mpickobj(mtmp,otmp);
				
				otmp = mksobj(PONTIFF_S_CROWN, FALSE, FALSE);
				otmp->obj_material = GOLD;
				otmp->spe = 0;
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				fix_object(otmp);
				(void) mpickobj(mtmp,otmp);

				otmp = mksobj(SCYTHE, FALSE, FALSE);
				otmp = oname(otmp, artiname(ART_FRIEDE_S_SCYTHE));
				otmp->spe = 0;
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				fix_object(otmp);
				(void) mpickobj(mtmp,otmp);
				
				otmp = mksobj(SCYTHE, FALSE, FALSE);
				otmp = oname(otmp, artiname(ART_PROFANED_GREATSCYTHE));
				otmp->spe = 0;
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				fix_object(otmp);
				(void) mpickobj(mtmp,otmp);
			} else if(ptr == &mons[PM_SHATTERED_ZIGGURAT_CULTIST]) {
			    otmp = mksobj(TORCH, FALSE, FALSE);
				otmp->age = (long) rn1(500,1000);
			    (void) mpickobj(mtmp, otmp);
				begin_burn(otmp, FALSE);
				(void)mongets(mtmp, WHITE_FACELESS_ROBE);
				(void)mongets(mtmp, LOW_BOOTS);
			}
			else if(ptr == &mons[PM_SHATTERED_ZIGGURAT_KNIGHT]) {
			    otmp = mksobj(SHADOWLANDER_S_TORCH, FALSE, FALSE);
			    (void) mpickobj(mtmp, otmp);
				otmp->age = (long) rn1(500,1000);
				begin_burn(otmp, FALSE);
				(void)mongets(mtmp, HELMET);
				(void)mongets(mtmp, BLACK_FACELESS_ROBE);
				(void)mongets(mtmp, CHAIN_MAIL);
				(void)mongets(mtmp, GLOVES);
				(void)mongets(mtmp, HIGH_BOOTS);
			}
			else if(ptr == &mons[PM_SHATTERED_ZIGGURAT_WIZARD]) {
			    otmp = mksobj(SHADOWLANDER_S_TORCH, FALSE, FALSE);
				otmp->spe = rnd(3);
				otmp->age = (long) rn1(1000,2000);
			    (void) mpickobj(mtmp, otmp);
				begin_burn(otmp, FALSE);
				(void)mongets(mtmp, LEATHER_HELM);
				(void)mongets(mtmp, SMOKY_VIOLET_FACELESS_ROBE);
				(void)mongets(mtmp, LEATHER_ARMOR);
				(void)mongets(mtmp, GLOVES);
				(void)mongets(mtmp, HIGH_BOOTS);
			}
			else if(is_mercenary(ptr)) {
				int w1 = 0, w2 = 0;
				switch (mm) {
				case PM_WATCHMAN:
				case PM_SOLDIER:
				  if (!rn2(3)) {
					  w1 = !rn2(4) ? PARTISAN : !rn2(3) ? HALBERD : rn2(2) ? BILL_GUISARME : BEC_DE_CORBIN;
					  w2 = rn2(2) ? DAGGER : KNIFE;
				  } else if(!rn2(2)){
					  w1 = CROSSBOW;
					  m_initthrow(mtmp, CROSSBOW_BOLT, 24);
				  } else w1 = rn2(2) ? SPEAR : SHORT_SWORD;
				  break;
				case PM_SERGEANT:
				  w1 = rn2(2) ? FLAIL : MACE;
				  if(rn2(2)){
					  w2 = CROSSBOW;
					  m_initthrow(mtmp, CROSSBOW_BOLT, 36);
				  }
				  break;
				case PM_LIEUTENANT:
				  w1 = rn2(2) ? BROADSWORD : LONG_SWORD;
				  if(rn2(2)){
					  w2 = BOW;
					  m_initthrow(mtmp, ARROW, 36);
				  }
				  else{
					  w2 = SHORT_SWORD;
				  }
				  break;
#ifdef CONVICT
				case PM_PRISON_GUARD:
#endif /* CONVICT */
				case PM_CAPTAIN:
				case PM_WATCH_CAPTAIN:
				  w1 = rn2(2) ? LONG_SWORD : SABER;
				  if(rn2(2)){
					  w2 = BOW;
					  if(w1 != SABER) m_initthrow(mtmp, !rn2(4) ? GOLDEN_ARROW : ARROW, 48);
					  else m_initthrow(mtmp, ARROW, 48);
				  }
				  else {
					  if (!rn2(5))
						  w2 = LONG_SWORD;
					  else
						  w2 = SHORT_SWORD;
				  }
				  break;
				default:
				  if (!rn2(4)) w1 = DAGGER;
				  if (!rn2(7)) w2 = SPEAR;
				  break;
				}
				if (w1) (void)mongets(mtmp, w1);
				if (!w2 && w1 != DAGGER && !rn2(4)) w2 = KNIFE;
				if (w2) (void)mongets(mtmp, w2);
			} else if(is_drow(ptr)){
				mtmp->mspec_used = d(4,4);
				if(mm != PM_DROW_MATRON && mm != PM_DROW_MATRON_MOTHER && mm != PM_DROW_NOVICE 
					&& mm != PM_A_SALOM && mm != PM_SISTER && mm != PM_MOTHER && !is_yochlol(mtmp->data)
					&& mm != PM_ECLAVDRA && mm != PM_SEYLL_AUZKOVYN && mm != PM_STJARNA_ALFR
					&& mm != PM_DARUTH_XAXOX && mm != PM_DROW_ALIENIST && mm != PM_HEDROW_MASTER_WIZARD
				){
					otmp = mksobj(DROVEN_CHAIN_MAIL, TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					(void) mongets(mtmp, DROVEN_CLOAK);
					if (rn2(2)) (void)mongets(mtmp, DROVEN_HELM);
					else if (!rn2(4)) (void)mongets(mtmp, HIGH_BOOTS);
					if (rn2(2)) (void)mongets(mtmp, DROVEN_DAGGER);
					if (could_twoweap(ptr)) (void)mongets(mtmp, DROVEN_SHORT_SWORD);
					switch (rn2(3)) {
					case 0:
						if (!rn2(4)) (void)mongets(mtmp, BUCKLER);
						if (rn2(3)) (void)mongets(mtmp, DROVEN_SHORT_SWORD);
						(void)mongets(mtmp, DROVEN_CROSSBOW);
						m_initthrow(mtmp, DROVEN_BOLT, 24);
					break;
					case 1:
						(void)mongets(mtmp, DROVEN_SHORT_SWORD);
						if (rn2(2)) (void)mongets(mtmp, KITE_SHIELD);
						(void)mongets(mtmp, DROVEN_DAGGER);
						(void)mongets(mtmp, DROVEN_DAGGER);
					break;
					case 2:
						if (!rn2(4)) {
							(void)mongets(mtmp, DROVEN_SHORT_SWORD);
							(void)mongets(mtmp, BUCKLER);
						} else if(!rn2(3)){
							(void)mongets(mtmp, DROVEN_SPEAR);
						} else if(!rn2(2)){
							(void)mongets(mtmp, MORNING_STAR);
						} else {
							(void)mongets(mtmp, DROVEN_GREATSWORD);
						}
					break;
					}
					switch(rn2(5)){
						case 0:
							if(rn2(3)) (void)mongets(mtmp, CRAM_RATION);
						break;
						case 1:
							if(!rn2(3)) (void)mongets(mtmp, FOOD_RATION);
						break;
						case 2:
							otmp = mksobj(CORPSE,TRUE,FALSE);
							otmp->corpsenm = PM_LICHEN;
							(void) mpickobj(mtmp, otmp);
						break;
						case 3:
							otmp = mksobj(TIN, TRUE,FALSE);
							if(rnd(20) > 4) otmp->corpsenm = PM_LICHEN;
							else if(rnd(20) > 5) otmp->corpsenm = PM_SHRIEKER;
							else otmp->corpsenm = PM_VIOLET_FUNGUS;
							otmp->quan = rnd(8);
							otmp->owt = weight(otmp);
							(void) mpickobj(mtmp, otmp);
						break;
						case 4:
							otmp = mksobj(SLIME_MOLD, TRUE,FALSE);
							otmp->spe = fruitadd("mushroom cake");
							(void) mpickobj(mtmp, otmp);
						break;
					}
				} else if(mm == PM_DROW_NOVICE){
					otmp = mksobj(BLACK_DRESS, TRUE, FALSE);
					otmp->spe = 0;
					otmp->oeroded2 = 1;
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(MUMMY_WRAPPING, TRUE, FALSE);
					otmp->spe = 0;
					otmp->oeroded2 = 1;
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(CRAM_RATION, TRUE, FALSE);
					otmp->quan = d(2,3);
					otmp->owt = weight(otmp);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(CRAM_RATION, TRUE, FALSE);
					otmp->quan = 1;
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oeaten = 100;
					otmp->owt = weight(otmp);
					(void) mpickobj(mtmp, otmp);
				} else if(mm == PM_A_SALOM){
					otmp = mksobj(NOBLE_S_DRESS, TRUE, FALSE);
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					(void) mongets(mtmp, DROVEN_CLOAK);
					(void)mongets(mtmp, DROVEN_HELM);
					(void)mongets(mtmp, HIGH_BOOTS);
					(void)mongets(mtmp, DROVEN_DAGGER);
					(void)mongets(mtmp, DROVEN_DAGGER);
					otmp = mksobj(VIPERWHIP, TRUE, FALSE);
					otmp->spe = 4;
					otmp->opoisoned = OPOISON_FILTH;
					otmp->opoisonchrgs = 30;
					otmp->ovar1 = 3;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(BUCKLER, TRUE, FALSE);
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(WAN_DEATH, TRUE, FALSE);
					otmp->spe = rnd(4);
					otmp->recharged = 4;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(WAN_DIGGING, TRUE, FALSE);
					otmp->spe = rnd(6)+6;
					otmp->recharged = 2;
					(void) mpickobj(mtmp, otmp);
					/*amulet*/
					otmp = mksobj(AMULET_OF_LIFE_SAVING, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					(void) mpickobj(mtmp, otmp);
				} else if(mm == PM_ECLAVDRA){
					/*Plate Mail*/
					otmp = mksobj(CRYSTAL_PLATE_MAIL, TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = LOLTH_SYMBOL;
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->spe = 3;
					(void) mpickobj(mtmp, otmp);
					/*Dress*/
					otmp = mksobj(BLACK_DRESS, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 1;
					(void) mpickobj(mtmp, otmp);
					/*Ring*/
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					/*Cloak*/
					otmp = mksobj(CLOAK_OF_MAGIC_RESISTANCE, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*Helm*/
					otmp = mksobj(DROVEN_HELM, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*boots*/
					otmp = mksobj(IRON_SHOES, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
				} else if(mm == PM_SEYLL_AUZKOVYN){
					/*Plate Mail*/
					otmp = mksobj(ELVEN_MITHRIL_COAT, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*Dress*/
					otmp = mksobj(VICTORIAN_UNDERWEAR, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*Ring*/
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					/*Cloak*/
					otmp = mksobj(CLOAK_OF_MAGIC_RESISTANCE, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*Helm*/
					otmp = mksobj(HIGH_ELVEN_HELM, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*boots*/
					otmp = mksobj(ELVEN_BOOTS, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
				} else if(mm == PM_DARUTH_XAXOX){
					/*Body*/
					otmp = mksobj(LEATHER_JACKET, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 14;
					(void) mpickobj(mtmp, otmp);
					/*Shirt*/
					otmp = mksobj(RUFFLED_SHIRT, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
					/*Ring*/
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					/*Cloak*/
					otmp = mksobj(CLOAK_OF_PROTECTION, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
					/*Helm*/
					otmp = mksobj(CORNUTHAUM, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					/*boots*/
					otmp = mksobj(LOW_BOOTS, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
				} else if(mm == PM_DROW_ALIENIST){
					/*Plate Mail*/
					otmp = mksobj(DROVEN_CHAIN_MAIL, TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*Ring*/
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					/*Cloak*/
					otmp = mksobj(CLOAK_OF_MAGIC_RESISTANCE, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*Helm*/
					otmp = mksobj(DROVEN_HELM, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*boots*/
					otmp = mksobj(HIGH_BOOTS, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*weapon*/
					otmp = mksobj(KHAKKHARA, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
				} else if(is_yochlol(mtmp->data)){
					/*weapon*/
					otmp = mksobj(RAPIER, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 2;
					(void) mpickobj(mtmp, otmp);
					/*Plate Mail*/
					otmp = mksobj(DROVEN_CHAIN_MAIL, TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = LOLTH_SYMBOL;
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->spe = 2;
					(void) mpickobj(mtmp, otmp);
					/*Dress*/
					otmp = mksobj(BLACK_DRESS, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 2;
					(void) mpickobj(mtmp, otmp);
					/*Ring*/
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					/*Cloak*/
					otmp = mksobj(CLOAK_OF_MAGIC_RESISTANCE, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*Helm*/
					otmp = mksobj(DROVEN_HELM, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*boots*/
					otmp = mksobj(HIGH_BOOTS, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
				} else if(mm == PM_HEDROW_MASTER_WIZARD){
					otmp = mksobj(DROVEN_CHAIN_MAIL, TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					otmp->oerodeproof = TRUE;
					otmp->spe = 3;
					(void) mpickobj(mtmp, otmp);
					(void) mongets(mtmp, DROVEN_CLOAK);
					(void)mongets(mtmp, HIGH_BOOTS);
					(void)mongets(mtmp, DROVEN_DAGGER);
					(void)mongets(mtmp, BUCKLER);
					(void)mongets(mtmp, DROVEN_SHORT_SWORD);
				} else if(mm == PM_SISTER){
					otmp = mksobj(NOBLE_S_DRESS, TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					(void) mongets(mtmp, DROVEN_CLOAK);
					(void) mongets(mtmp, DROVEN_HELM);
					(void) mongets(mtmp, GLOVES);
					(void)mongets(mtmp, HIGH_BOOTS);
					otmp = mksobj(DAGGER, TRUE, FALSE);
					otmp->obj_material = SILVER;
					otmp->oerodeproof = TRUE;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					(void)mongets(mtmp, BUCKLER);
					otmp = mksobj(VIPERWHIP, TRUE, FALSE);
					otmp->spe = 2;
					otmp->opoisoned = rn2(4) ? OPOISON_BASIC : OPOISON_PARAL;
					otmp->opoisonchrgs = 1;
					otmp->ovar1 = rnd(3);
				} else if(mm == PM_MOTHER){
					otmp = mksobj(NOBLE_S_DRESS, TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					otmp->oerodeproof = TRUE;
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(DROVEN_CLOAK, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(DROVEN_HELM, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(GLOVES, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(HIGH_BOOTS, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(DAGGER, TRUE, FALSE);
					otmp->obj_material = SILVER;
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->quan = 6;
					otmp->spe = 5;
					otmp->owt = weight(otmp);
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(BUCKLER, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(SABER, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(VIPERWHIP, TRUE, FALSE);
					otmp->spe = 2;
					otmp->opoisoned = rn2(4) ? OPOISON_BASIC : OPOISON_PARAL;
					otmp->opoisonchrgs = 30;
					otmp->ovar1 = 5;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				} else if(mm == PM_STJARNA_ALFR){
					otmp = mksobj(DROVEN_PLATE_MAIL, TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = EILISTRAEE_SYMBOL;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					(void) mongets(mtmp, ELVEN_CLOAK);
					(void)mongets(mtmp, DROVEN_HELM);
					(void)mongets(mtmp, ELVEN_BOOTS);
					(void)mongets(mtmp, ELVEN_DAGGER);
					if(!rn2(20)){
						(void)mongets(mtmp, KHAKKHARA);
						(void)mongets(mtmp, ELVEN_DAGGER);
						(void)mongets(mtmp, ELVEN_DAGGER);
						(void)mongets(mtmp, ELVEN_DAGGER);
					} else {
						(void)mongets(mtmp, ELVEN_BROADSWORD);
						(void)mongets(mtmp, ELVEN_SHIELD);
					}
					(void)mongets(mtmp, ELVEN_BOW);
					m_initthrow(mtmp, ELVEN_ARROW, 24);
					if (!rn2(50)) (void)mongets(mtmp, CRYSTAL_BALL);
				} else {
					otmp = mksobj(DROVEN_PLATE_MAIL, TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = LOLTH_SYMBOL;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = curhouse;
					(void) mpickobj(mtmp, otmp);
					(void) mongets(mtmp, DROVEN_CLOAK);
					(void)mongets(mtmp, DROVEN_HELM);
					(void)mongets(mtmp, HIGH_BOOTS);
					(void)mongets(mtmp, DROVEN_DAGGER);
					if(!rn2(20)){
						otmp = mksobj(VIPERWHIP, TRUE, FALSE);
						otmp->spe = 2;
						otmp->opoisoned = rn2(4) ? OPOISON_BASIC : OPOISON_PARAL;
						otmp->opoisonchrgs = 1;
						otmp->ovar1 = 1+rnd(3);
						(void)mongets(mtmp, KHAKKHARA);
						(void)mongets(mtmp, DROVEN_DAGGER);
						(void)mongets(mtmp, DROVEN_DAGGER);
						(void)mongets(mtmp, DROVEN_DAGGER);
					} else {
						otmp = mksobj(VIPERWHIP, TRUE, FALSE);
						otmp->spe = 2;
						otmp->opoisoned = rn2(4) ? OPOISON_BASIC : OPOISON_PARAL;
						otmp->opoisonchrgs = 1;
						otmp->ovar1 = rnd(3);
						(void)mongets(mtmp, DROVEN_SHORT_SWORD);
						(void)mongets(mtmp, KITE_SHIELD);
					}
					(void)mongets(mtmp, DROVEN_CROSSBOW);
					m_initthrow(mtmp, DROVEN_BOLT, 24);
				}
			} else if (is_elf(ptr) && ptr != &mons[PM_HALF_ELF_RANGER]) {
				if(mm == PM_GALADRIEL){
					/*Dress*/
					otmp = mksobj(GENTLEWOMAN_S_DRESS, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(VICTORIAN_UNDERWEAR, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*Ring*/
					otmp = mksobj(find_silver_ring(), TRUE, FALSE);
					(void) mpickobj(mtmp, otmp);
					/*boots*/
					otmp = mksobj(ELVEN_BOOTS, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*phial*/
					otmp = mksobj(POT_STARLIGHT, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					(void) mpickobj(mtmp, otmp);
				} else if(mm == PM_CELEBORN){
					/*Plate Mail*/
					otmp = mksobj(HIGH_ELVEN_WARSWORD, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					/*Plate Mail*/
					otmp = mksobj(ELVEN_MITHRIL_COAT, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					/*Cloak*/
					otmp = mksobj(ELVEN_CLOAK, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 3;
					(void) mpickobj(mtmp, otmp);
					/*Helm*/
					otmp = mksobj(HIGH_ELVEN_HELM, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					/*boots*/
					otmp = mksobj(ELVEN_BOOTS, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
				} else if(mm == PM_MYRKALFAR_WARRIOR){
					otmp = mksobj(SNIPER_RIFLE, TRUE, FALSE);
					otmp->spe = 4;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(BULLET, FALSE, FALSE);
					otmp->spe = 0;
					otmp->quan += 60;
					otmp->owt = weight(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(SILVER_BULLET, FALSE, FALSE);
					otmp->spe = 3;
					otmp->quan += 30;
					otmp->owt = weight(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(VIBROBLADE, TRUE, FALSE);
					otmp->spe = rn1(3,3);
					otmp->ovar1 = 50 + d(5,10);
					otmp->recharged = rn1(3,3);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = LAST_BASTION_SYMBOL;
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, DROVEN_CLOAK);
					(void)mongets(mtmp, FLACK_HELMET);
					(void)mongets(mtmp, PLASTEEL_ARMOR);
					(void)mongets(mtmp, BODYGLOVE);
					(void)mongets(mtmp, ORIHALCYON_GAUNTLETS);
					(void)mongets(mtmp, ELVEN_BOOTS);
				} else if(mm == PM_MYRKALFAR_MATRON){
					otmp = mksobj(ARM_BLASTER, TRUE, FALSE);
					otmp->spe = 4;
					otmp->ovar1 = 50 + d(5,10);
					otmp->recharged = 4;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(LIGHTSABER, TRUE, FALSE);
					otmp->spe = 3;
					otmp->ovar1 = !rn2(4) ? 38L : !rn2(3) ? 18L : rn2(2) ? 10L : 26L;
					otmp->cobj->otyp = !rn2(4) ? MORGANITE : !rn2(3) ? RUBY : rn2(2) ? GARNET : JASPER;
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(find_signet_ring(), TRUE, FALSE);
					otmp->ohaluengr = TRUE;
					otmp->oward = LAST_BASTION_SYMBOL;
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, CLOAK_OF_MAGIC_RESISTANCE);
					(void)mongets(mtmp, BODYGLOVE);
					otmp = mksobj(GAUNTLETS_OF_DEXTERITY, TRUE, FALSE);
					otmp->spe = 4;
					(void) mpickobj(mtmp, otmp);
					(void)mongets(mtmp, ELVEN_BOOTS);
				} else if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz) && mm == PM_ELVENKING){ /* Give the elvenking in the quest a special setup */
					mtmp->m_lev += 7;
					mtmp->mhpmax = mtmp->mhp = d((int)mtmp->m_lev, 8);
					
					otmp = mksobj(LIGHTSABER, TRUE, FALSE);
					otmp->spe = 3;
					otmp->ovar1 = !rn2(4) ? 2L : !rn2(3) ? 9L : rn2(2) ? 21L : 22L;
					otmp->cobj->otyp = !rn2(3) ? EMERALD : rn2(2) ? GREEN_FLUORITE : JADE;
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HAND_BLASTER, TRUE, FALSE);
					otmp->spe = 3;
					otmp->ovar1 = 50 + d(5,10);
					otmp->recharged = rnd(4);
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, CLOAK_OF_MAGIC_RESISTANCE);
					(void)mongets(mtmp, ELVEN_MITHRIL_COAT);
					(void)mongets(mtmp, HIGH_ELVEN_HELM);
					(void)mongets(mtmp, BODYGLOVE);
					otmp = mksobj(GAUNTLETS_OF_DEXTERITY, TRUE, FALSE);
					otmp->spe = 3;
					(void) mpickobj(mtmp, otmp);
					(void)mongets(mtmp, ELVEN_BOOTS);
				} else {
					if (rn2(2))
					(void) mongets(mtmp,
						   rn2(2) ? ELVEN_MITHRIL_COAT : ELVEN_CLOAK);
					if (rn2(2)) (void)mongets(mtmp, (mm == PM_ELVENKING || mm == PM_ELF_LORD || mm == PM_ELVENQUEEN || mm == PM_ELF_LADY) ? HIGH_ELVEN_HELM : ELVEN_HELM);
					if (!rn2(4)) (void)mongets(mtmp, ELVEN_BOOTS);
					if (rn2(2)) (void)mongets(mtmp, ELVEN_DAGGER);
#ifdef BARD
					if ((mm == PM_ELVENKING || mm == PM_ELF_LORD || mm == PM_ELVENQUEEN || mm == PM_ELF_LADY) ? TRUE : !rn2(10))
						(void)mongets(mtmp, (rn2(2) ? WOODEN_FLUTE : WOODEN_HARP));
#endif
					if (rnd(100) < mtmp->m_lev)
						(void)mongets(mtmp, POT_STARLIGHT);
						
					switch (rn2(3)) {
					case 0:
						if (!rn2(4)) (void)mongets(mtmp, ELVEN_SHIELD);
						else if (could_twoweap(ptr)) (void)mongets(mtmp, ELVEN_DAGGER);
						if (rn2(3)) (void)mongets(mtmp, ELVEN_SHORT_SWORD);
						(void)mongets(mtmp, ELVEN_BOW);
						m_initthrow(mtmp, ELVEN_ARROW, 12);
						break;
					case 1:
						(void)mongets(mtmp, ELVEN_BROADSWORD);
						if (rn2(2)) (void)mongets(mtmp, ELVEN_SHIELD);
						else if (could_twoweap(ptr)) (void)mongets(mtmp, ELVEN_SHORT_SWORD);
						break;
					case 2:
						if (rn2(2)) {
						(void)mongets(mtmp, ELVEN_SPEAR);
						(void)mongets(mtmp, ELVEN_SHIELD);
						}
						break;
					}
					if (mm == PM_ELVENKING) {
					if (rn2(3) || (in_mklev && Is_earthlevel(&u.uz)))
						(void)mongets(mtmp, PICK_AXE);
					if (!rn2(50)) (void)mongets(mtmp, CRYSTAL_BALL);
					}
					if(In_quest(&u.uz) && Role_if(PM_KNIGHT)){
						if (mm == PM_ELF_LORD) {
							(void)mongets(mtmp, CRYSTAL_SWORD);
						}
					}
				}
			} else if (ptr->msound == MS_PRIEST ||
				quest_mon_represents_role(ptr,PM_PRIEST)) {
				otmp = mksobj(MACE, FALSE, FALSE);
				if(otmp) {
				otmp->spe = rnd(3);
				if(!rn2(2)) curse(otmp);
				(void) mpickobj(mtmp, otmp);
				}
			} else if(mm >= PM_STUDENT && mm <= PM_APPRENTICE){
				if(mm == PM_STUDENT){
					(void)mongets(mtmp, PICK_AXE);
					(void)mongets(mtmp, SCR_BLANK_PAPER);
					(void)mongets(mtmp, LEATHER_JACKET);
					(void)mongets(mtmp, LOW_BOOTS);
					(void)mongets(mtmp, FEDORA);
				} else if (mm == PM_TROOPER){
					otmp = mksobj(ARM_BLASTER, TRUE, FALSE);
					otmp->spe = 0;
					otmp->ovar1 = d(5,10);
					otmp->recharged = 4;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(VIBROBLADE, TRUE, FALSE);
					otmp->spe = rn1(3,3);
					otmp->ovar1 = 50 + d(5,10);
					otmp->recharged = rn1(3,3);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HAND_BLASTER, TRUE, FALSE);
					otmp->spe = 0;
					otmp->ovar1 = 50 + d(5,10);
					otmp->recharged = rn1(2,2);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(BUCKLER, TRUE, FALSE);
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, LEATHER_CLOAK);
					(void)mongets(mtmp, PLASTEEL_HELM);
					(void)mongets(mtmp, PLASTEEL_ARMOR);
					(void)mongets(mtmp, BODYGLOVE);
					(void)mongets(mtmp, PLASTEEL_GAUNTLETS);
					(void)mongets(mtmp, PLASTEEL_BOOTS);
				} else if (mm == PM_CHIEFTAIN){
					(void)mongets(mtmp, TWO_HANDED_SWORD);
					(void)mongets(mtmp, CHAIN_MAIL);
					(void)mongets(mtmp, HIGH_BOOTS);
				} else if (mm == PM_RHYMER){
					static int trotyp[] = {
						WOODEN_FLUTE, TOOLED_HORN, WOODEN_HARP,
						BELL, BUGLE, LEATHER_DRUM
					};
					(void)mongets(mtmp, trotyp[rn2(SIZE(trotyp))]);
					(void)mongets(mtmp, !rn2(10) ? ELVEN_MITHRIL_COAT : ELVEN_TOGA);
					(void)mongets(mtmp, LOW_BOOTS);
				} else if (mm == PM_NEANDERTHAL){
					(void)mongets(mtmp, CLUB);
					(void)mongets(mtmp, LEATHER_ARMOR);
// ifdef CONVICT
				} else if (mm == PM_INMATE){
					(void)mongets(mtmp, rn2(2) ? HEAVY_IRON_BALL : SPOON);
					(void)mongets(mtmp, STRIPED_SHIRT);
// endif
				} else if (mm == PM_ATTENDANT){
					(void) mongets(mtmp, SCALPEL);
					(void) mongets(mtmp, HEALER_UNIFORM);
					(void) mongets(mtmp, HIGH_BOOTS);
					(void) mongets(mtmp, GLOVES);
					(void) mongets(mtmp, POT_EXTRA_HEALING);
					(void) mongets(mtmp, POT_HEALING);
					(void)mongets(mtmp, POT_HEALING);
				} else if (mm == PM_PAGE){
					(void) mongets(mtmp, SHORT_SWORD);
					(void) mongets(mtmp, BUCKLER);
					(void) mongets(mtmp, CHAIN_MAIL);
					(void) mongets(mtmp, HIGH_BOOTS);
				} else if (mm == PM_ABBOT){
					(void) mongets(mtmp, LEATHER_CLOAK);
				} else if (mm == PM_SERVANT){
					if(mtmp->female){
						(void) mongets(mtmp, VICTORIAN_UNDERWEAR);
						(void) mongets(mtmp, STILETTOS);
					} else{
						(void) mongets(mtmp, RUFFLED_SHIRT);
						(void) mongets(mtmp, LOW_BOOTS);
					}
				} else if (mm == PM_ACOLYTE){
					(void) mongets(mtmp, !rn2(10) ? KHAKKHARA : MACE);
					(void) mongets(mtmp, LEATHER_HELM);
					(void) mongets(mtmp, LEATHER_CLOAK);
					(void) mongets(mtmp, LEATHER_ARMOR);
					(void) mongets(mtmp, HIGH_BOOTS);
				} else if (mm == PM_PIRATE_BROTHER){
					(void)mongets(mtmp, SCIMITAR);
					(void)mongets(mtmp, LEATHER_ARMOR);
					(void)mongets(mtmp, HIGH_BOOTS);
					(void)mongets(mtmp, FLINTLOCK);
					otmp = mksobj(BULLET, FALSE, FALSE);
					otmp->quan += 10;
					otmp->owt = weight(otmp);
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_HUNTER){
					(void)mongets(mtmp, SHORT_SWORD);
					(void)mongets(mtmp, LEATHER_ARMOR);
					(void)mongets(mtmp, LEATHER_HELM);
					(void)mongets(mtmp, HIGH_BOOTS);
					(void)mongets(mtmp, BOW);
					otmp = mksobj(ARROW, FALSE, FALSE);
					otmp->quan += 20;
					otmp->owt = weight(otmp);
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_THUG){
					(void)mongets(mtmp, CLUB);
					(void)mongets(mtmp, DAGGER);
					(void)mongets(mtmp, GLOVES);
					(void)mongets(mtmp, LEATHER_JACKET);
					(void)mongets(mtmp, LOW_BOOTS);
				} else if (mm == PM_NINJA){
					(void)mongets(mtmp, BROADSWORD);
					chance = d(1,100);
					if(chance > 95) (void)mongets(mtmp, SHURIKEN);
					else if(chance > 75) (void)mongets(mtmp, DART);
					else{ 
						(void) mongets(mtmp, DAGGER);
						(void) mongets(mtmp, DAGGER);
					}
				} else if (mm == PM_ROSHI){
					(void)mongets(mtmp, QUARTERSTAFF);
					(void)mongets(mtmp, SEDGE_HAT);
					(void)mongets(mtmp, LEATHER_CLOAK);
					(void)mongets(mtmp, LOW_BOOTS);
				} else if (mm == PM_GUIDE){
					(void)mongets(mtmp, LEATHER_CLOAK);
					(void)mongets(mtmp, LOW_BOOTS);
				} else if (mm == PM_WARRIOR){
					(void)mongets(mtmp, !rn2(10) ? LONG_SWORD : SHORT_SWORD);
					(void)mongets(mtmp, SPEAR);
					(void)mongets(mtmp, BUCKLER);
					(void)mongets(mtmp, LEATHER_HELM);
					(void)mongets(mtmp, LEATHER_ARMOR);
					(void)mongets(mtmp, HIGH_BOOTS);
				} else if (mm == PM_APPRENTICE){
					otmp = mksobj(rnd_attack_wand(mtmp), TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = FALSE;
					(void) mpickobj(mtmp, otmp);
					(void)mongets(mtmp, LEATHER_CLOAK);
					(void)mongets(mtmp, LOW_BOOTS);
				}
			} else if(mm >= PM_LORD_CARNARVON && mm <= PM_NEFERET_THE_GREEN){
				if(mm == PM_LORD_CARNARVON){
					otmp = mksobj(PISTOL, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(BULLET, FALSE, FALSE);
					otmp->quan += 20;
					bless(otmp);
					otmp->spe = 7;
					otmp->owt = weight(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(LEATHER_JACKET, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					otmp->owt = weight(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(FEDORA, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					otmp->owt = weight(otmp);
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, LOW_BOOTS);
				} else if (mm == PM_SARA__THE_LAST_ORACLE){
					otmp = mksobj(BLINDFOLD, TRUE, FALSE);
					otmp->spe = 0;
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(STRAITJACKET, TRUE, FALSE);
					otmp->spe = 5;
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_PELIAS){
					otmp = mksobj(TWO_HANDED_SWORD, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(ROBE, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HIGH_BOOTS, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_PINDAR){
					otmp = mksobj(ELVEN_MITHRIL_COAT, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(ROBE, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HIGH_BOOTS, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_SHAMAN_KARNOV){
					otmp = mksobj(CLUB, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(LEO_NEMAEUS_HIDE, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(rnd_class(GRAY_DRAGON_SCALE_MAIL, YELLOW_DRAGON_SCALE_MAIL), FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_ROBERT_THE_LIFER){
					otmp = mksobj(HEAVY_IRON_BALL, FALSE, FALSE);
					curse(otmp);
					otmp->spe = -5;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(STRIPED_SHIRT, FALSE, FALSE);
					curse(otmp);
					otmp->spe = -5;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_HIPPOCRATES){
					otmp = mksobj(SCALPEL, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HEALER_UNIFORM, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_KING_ARTHUR){
					otmp = mksobj(LONG_SWORD, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(KITE_SHIELD, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(PLATE_MAIL, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(find_gcirclet(), FALSE, FALSE);
					bless(otmp);
					otmp->spe = 3;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(GAUNTLETS, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
					otmp->obj_material = IRON;
					bless(otmp);
					otmp->spe = 0;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_GRAND_MASTER){
					otmp = mksobj(ROBE, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_OLD_GYPSY_WOMAN){
					otmp = mksobj(ATHAME, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(BLACK_DRESS, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_ARCH_PRIEST){
					otmp = mksobj(KHAKKHARA, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(ROBE, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HELM_OF_BRILLIANCE, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_MAYOR_CUMMERBUND){
					int spe2;
					otmp = mksobj(SCIMITAR, FALSE, FALSE);
					curse(otmp);
					otmp->oerodeproof = TRUE;
					spe2 = d(1,3);
					otmp->spe = max(otmp->spe, spe2);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(LEATHER_JACKET, FALSE, FALSE);
					otmp->oerodeproof = TRUE;
					spe2 = d(2,3);
					otmp->spe = max(otmp->spe, spe2);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(BUCKLER, FALSE, FALSE);
					otmp->oerodeproof = TRUE;
					spe2 = d(1,3);
					otmp->spe = max(otmp->spe, spe2);
					(void) mpickobj(mtmp, otmp);

					(void)mongets(mtmp, LEATHER_CLOAK);
					(void)mongets(mtmp, HIGH_BOOTS);
					(void)mongets(mtmp, GLOVES);
				} else if (mm == PM_ORION){
					otmp = mksobj(BOW, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(SILVER_ARROW, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					otmp->quan = 30;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(LEATHER_ARMOR, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(ELVEN_BOOTS, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_MASTER_OF_THIEVES){
					otmp = mksobj(STILETTO, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 4;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(DAGGER, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 4;
					otmp->quan = 10;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(GAUNTLETS_OF_DEXTERITY, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(LEATHER_ARMOR, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HIGH_BOOTS, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_LORD_SATO){
					otmp = mksobj(KATANA, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 4;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(SHORT_SWORD, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 4;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(SPLINT_MAIL, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 4;
					otmp->oerodeproof = 1;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(WAR_HAT, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HIGH_BOOTS, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_TWOFLOWER){
					otmp = mksobj(LENSES, FALSE, FALSE);
					bless(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HAWAIIAN_SHIRT, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(LOW_BOOTS, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 0;
					(void) mpickobj(mtmp, otmp);
				} else if (mm == PM_NORN){
					/* Nothing */
				} else if (mm == PM_NEFERET_THE_GREEN){
					otmp = mksobj(KHAKKHARA, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(ELVEN_BOW, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					otmp->obj_material = GOLD;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(GOLDEN_ARROW, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					otmp->quan = 20;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(ELVEN_MITHRIL_COAT, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 7;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(GLOVES, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					otmp->obj_material = SILVER;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(LOW_BOOTS, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					otmp->obj_material = SILVER;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(MUMMY_WRAPPING, FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					otmp->obj_material = GOLD;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(find_gcirclet() == HELM_OF_OPPOSITE_ALIGNMENT ? HELM_OF_BRILLIANCE : find_gcirclet(), FALSE, FALSE);
					bless(otmp);
					otmp->spe = 5;
					otmp->obj_material = GOLD;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(AMULET_OF_LIFE_SAVING, FALSE, FALSE);
					bless(otmp);
					otmp->obj_material = GOLD;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				}
// #ifdef CONVICT
			} else if (mm == PM_MINER) {
				(void)mongets(mtmp, PICK_AXE);
				otmp = mksobj(BRASS_LANTERN, TRUE, FALSE);
				(void) mpickobj(mtmp, otmp);
				begin_burn(otmp, FALSE);
// #endif /* CONVICT */
			} else {
				switch(mm){
				case PM_SIR_GARLAND:
					(void) mongets(mtmp, LONG_SWORD);
					(void) mongets(mtmp, LANCE);
					(void) mongets(mtmp, PLATE_MAIL);
				break;
				case PM_GARLAND:
					(void) mongets(mtmp, RUNESWORD);
					(void) mongets(mtmp, PLATE_MAIL);
				break;
				case PM_REBEL_RINGLEADER:
					(void) mongets(mtmp, SABER);
					(void) mongets(mtmp, GRAY_DRAGON_SCALE_MAIL);
					(void) mongets(mtmp, BUCKLER);
					(void) mongets(mtmp, HIGH_BOOTS);
				break;
				case PM_ADVENTURING_WIZARD:
					(void) mongets(mtmp, QUARTERSTAFF);
					(void) mongets(mtmp, LEATHER_ARMOR);
					(void) mongets(mtmp, ROBE);
					(void) mongets(mtmp, LOW_BOOTS);
				break;
				case PM_MILITANT_CLERIC:
					(void) mongets(mtmp, MACE);
					(void) mongets(mtmp, PLATE_MAIL);
					(void) mongets(mtmp, KITE_SHIELD);
					otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
					otmp->obj_material = IRON;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				break;
				case PM_HALF_ELF_RANGER:
					(void) mongets(mtmp, ELVEN_SHORT_SWORD);
					(void) mongets(mtmp, ELVEN_SHORT_SWORD);
					(void) mongets(mtmp, ELVEN_MITHRIL_COAT);
					(void) mongets(mtmp, ELVEN_HELM);
					(void) mongets(mtmp, ELVEN_CLOAK);
					(void) mongets(mtmp, HIGH_BOOTS);
					(void) mongets(mtmp, ELVEN_BOW);
					otmp = mksobj(SILVER_ARROW, TRUE, FALSE);
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->quan += 10;
					otmp->spe = 1;
					otmp->owt = weight(otmp);
					(void) mpickobj(mtmp,otmp);
					otmp = mksobj(ARROW, TRUE, FALSE);
					otmp->blessed = FALSE;
					otmp->cursed = FALSE;
					otmp->quan += 20;
					otmp->owt = weight(otmp);
					(void) mpickobj(mtmp,otmp);
				break;
				case PM_PEASANT:
					switch(rn2(6)){
						case 0:
							(void)mongets(mtmp, SICKLE);
						break;
						case 1:
							(void)mongets(mtmp, SCYTHE);
						break;
						case 2:
							(void)mongets(mtmp, KNIFE);
						break;
						case 3:
							(void)mongets(mtmp, CLUB);
						break;
						case 4:
							(void)mongets(mtmp, AXE);
						break;
						case 5:
							(void)mongets(mtmp, VOULGE);
						break;
					}
				break;
				case PM_NURSE:
					(void) mongets(mtmp, SCALPEL);
					(void) mongets(mtmp, HEALER_UNIFORM);
				return;
				break;
				case PM_MAID: //ninja maids
					chance = d(1,100);
					if(chance > 75) (void)mongets(mtmp, SHURIKEN);
					else if(chance > 50) (void)mongets(mtmp, DART);
					if(chance > 99){
						otmp = mksobj(DAGGER, TRUE, FALSE);
						otmp->obj_material = SILVER;
						fix_object(otmp);
						(void) mpickobj(mtmp, otmp);
					}
					(void) mongets(mtmp, DAGGER);
					(void) mongets(mtmp, DAGGER);
					(void) mongets(mtmp, DAGGER);
					(void) mongets(mtmp, DAGGER);
					chance = d(1,100);
					if(Role_if(PM_SAMURAI))
						if(chance > 95) (void)mongets(mtmp, NAGINATA);
						else if(chance > 75) (void)mongets(mtmp, KATANA);
						else if(chance > 50) (void)mongets(mtmp, BROADSWORD);
						else (void)mongets(mtmp, STILETTO);
					else if(chance > 70) (void)mongets(mtmp, KATANA);
					else (void)mongets(mtmp, STILETTO);
				break;
				}
			}
		break;

		break;
		case S_GHOST:{
			switch (mm){
			case PM_GARO:
				if(rnd(100) > 75) (void)mongets(mtmp, SHURIKEN);
				(void)mongets(mtmp, STILETTO);
				(void)mongets(mtmp, STILETTO);
				return;//no random stuff
			break;
			case PM_GARO_MASTER:
				(void)mongets(mtmp, SHURIKEN);
				otmp = mksobj(STILETTO, TRUE, FALSE);
				curse(otmp);
				otmp->spe = 4;
				(void) mpickobj(mtmp, otmp);
				otmp = mksobj(STILETTO, TRUE, FALSE);
				curse(otmp);
				otmp->spe = 4;
				(void) mpickobj(mtmp, otmp);
				return;//no random stuff
			break;
			case PM_BLACKBEARD_S_GHOST:{
				int spe2;
				otmp = mksobj(SCIMITAR, FALSE, FALSE);
				curse(otmp);
				otmp->oerodeproof = TRUE;
				otmp->oeroded = 1;
				spe2 = d(2,3);
				otmp->spe = max(otmp->spe, spe2);
				(void) mpickobj(mtmp, otmp);
			break;}
			}
		break;}
	    case S_NEU_OUTSIDER:{
			if(ptr == &mons[PM_PLUMACH_RILMANI]){
				if(!rn2(3)) otmp = mksobj(MACE, TRUE, FALSE);
				else otmp = mksobj(rn2(3) ? AXE : rn2(3) ? SICKLE : SCYTHE, TRUE, FALSE);
			    otmp->cursed = 0;
			    otmp->blessed = 0;
			    otmp->spe = -1;
				otmp->obj_material = METAL;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				if(otmp->otyp == MACE && !rn2(3)){
					otmp = mksobj(KITE_SHIELD, TRUE, FALSE);
					otmp->cursed = 0;
					otmp->blessed = 0;
					otmp->obj_material = METAL;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				}
			}
			if(ptr == &mons[PM_FERRUMACH_RILMANI]){
				otmp = mksobj(rn2(2) ? HALBERD : BATTLE_AXE, TRUE, FALSE);
				otmp->cursed = 0;
				otmp->blessed = 0;
				if(otmp->spe < 1) otmp->spe = 1;
				otmp->obj_material = IRON;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
			}
			if(ptr == &mons[PM_STANNUMACH_RILMANI]){
				otmp = mksobj(KHAKKHARA, TRUE, FALSE);
				otmp->cursed = 0;
				otmp->blessed = 0;
				if(otmp->spe < 1) otmp->spe = 1;
				otmp->obj_material = METAL;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
			}
			if(ptr == &mons[PM_CUPRILACH_RILMANI]){
				otmp = mksobj(SHORT_SWORD, TRUE, FALSE);
				otmp->cursed = 0;
				otmp->blessed = 0;
				if(otmp->spe < 2) otmp->spe = 2;
				otmp->obj_material = COPPER;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
				if(!rn2(3)){
					otmp = mksobj(BUCKLER, TRUE, FALSE);
					otmp->cursed = 0;
					otmp->blessed = 0;
					if(otmp->spe < 2) otmp->spe = 2;
					otmp->obj_material = COPPER;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				} else if(rn2(2)){
					otmp = mksobj(SHORT_SWORD, TRUE, FALSE);
					otmp->cursed = 0;
					otmp->blessed = 0;
					if(otmp->spe < 2) otmp->spe = 2;
					otmp->obj_material = COPPER;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				}
			}
			if(ptr == &mons[PM_ARGENACH_RILMANI]){
				otmp = mksobj(rn2(3) ? BROADSWORD : rn2(3) ? BATTLE_AXE : HALBERD, TRUE, FALSE);
			    otmp->cursed = 0;
			    otmp->blessed = 0;
				if(otmp->spe < 3) otmp->spe = 3;
				otmp->obj_material = SILVER;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				if(otmp->otyp == BROADSWORD){
					otmp = mksobj(rn2(6) ? KITE_SHIELD : SHIELD_OF_REFLECTION, TRUE, FALSE);
					otmp->cursed = 0;
					otmp->blessed = 0;
					if(otmp->spe < 3) otmp->spe = 3;
					otmp->obj_material = SILVER;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				}
			}
			if(ptr == &mons[PM_HYDRARGYRUMACH_RILMANI]){
				otmp = mksobj(VOULGE, TRUE, FALSE);
			    otmp->cursed = 0;
			    otmp->blessed = 0;
				otmp->obj_material = METAL;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				otmp = mksobj(LONG_SWORD, TRUE, FALSE);
			    otmp->cursed = 0;
			    otmp->blessed = 0;
				otmp->obj_material = METAL;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			}
			if(ptr == &mons[PM_AURUMACH_RILMANI]){
				otmp = mksobj(HALBERD, TRUE, FALSE);
			    otmp->cursed = 0;
			    otmp->blessed = 0;
				if(otmp->spe < 4) otmp->spe = 4;
				otmp->obj_material = GOLD;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				otmp = mksobj(TWO_HANDED_SWORD, TRUE, FALSE);
			    otmp->cursed = 0;
			    otmp->blessed = 0;
				if(otmp->spe < 4) otmp->spe = 4;
				otmp->obj_material = GOLD;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				if(!rn2(3)){
					otmp = mksobj(PLATE_MAIL, TRUE, FALSE);
					otmp->cursed = 0;
					otmp->blessed = 0;
					if(otmp->spe < 4) otmp->spe = 4;
					otmp->obj_material = GOLD;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				}
			}
			if(ptr == &mons[PM_AMM_KAMEREL]){
				if(rn2(10)){//Physical fighter, no magic
					mtmp->mcan = 1;
					if(rn2(10)){//Warrior
						mongets(mtmp, MIRRORBLADE);
						otmp = mksobj(ROUNDSHIELD, TRUE, FALSE);
						switch(rn2(3)){
							case 0:
								otmp->obj_material = COPPER;
							break;
							case 1:
								otmp->obj_material = GLASS;
							break;
							case 2:
								otmp->obj_material = SILVER;
							break;
						}
						otmp->objsize = MZ_SMALL;
						fix_object(otmp);
						(void) mpickobj(mtmp, otmp);
						
						otmp = mksobj(BANDED_MAIL, TRUE, FALSE);
						switch(rn2(3)){
							case 0:
								otmp->obj_material = COPPER;
							break;
							case 1:
								otmp->obj_material = GLASS;
							break;
							case 2:
								otmp->obj_material = SILVER;
							break;
						}
						otmp->objsize = MZ_SMALL;
						fix_object(otmp);
						(void) mpickobj(mtmp, otmp);
					} else { //Fighter driver
						mtmp->m_lev += 3;
						mtmp->mhpmax += d(3,8);
						mtmp->mhp = mtmp->mhpmax;
						mtmp->mspeed = MFAST;
						mtmp->permspeed = MFAST;
						
						otmp = mksobj(MIRRORBLADE, TRUE, FALSE);
						if(otmp->spe < 1) otmp->spe = 1;
						otmp->objsize = MZ_SMALL;
						fix_object(otmp);
						(void) mpickobj(mtmp, otmp);
						
						
						otmp = mksobj(ROUNDSHIELD, TRUE, FALSE);
						switch(rn2(3)){
							case 0:
								otmp->obj_material = COPPER;
							break;
							case 1:
								otmp->obj_material = GLASS;
							break;
							case 2:
								otmp->obj_material = SILVER;
							break;
						}
						if(otmp->spe < 1) otmp->spe = 1;
						otmp->objsize = MZ_SMALL;
						fix_object(otmp);
						(void) mpickobj(mtmp, otmp);
						
						otmp = mksobj(PLATE_MAIL, TRUE, FALSE);
						switch(rn2(3)){
							case 0:
								otmp->obj_material = COPPER;
							break;
							case 1:
								otmp->obj_material = GLASS;
							break;
							case 2:
								otmp->obj_material = SILVER;
							break;
						}
						otmp->objsize = MZ_SMALL;
						if(otmp->spe < 1) otmp->spe = 1;
						fix_object(otmp);
						(void) mpickobj(mtmp, otmp);
					}
				} else {
					if(rn2(10)){ //Wizard driver
						mtmp->m_lev += 3;
						mtmp->mhpmax += d(3,8);
						mtmp->mhp = mtmp->mhpmax;
						otmp = mksobj(MIRROR, FALSE, FALSE);
						otmp->objsize = MZ_SMALL;
						fix_object(otmp);
						(void) mpickobj(mtmp, otmp);
						switch(rn2(3)){
							case 0:
								(void) mongets(mtmp, ELVEN_CLOAK);
							break;
							case 1:
								(void) mongets(mtmp, DWARVISH_CLOAK);
							break;
							case 2:
								(void) mongets(mtmp, CLOAK_OF_INVISIBILITY);
							break;
						}
						
						otmp = mksobj(STILETTO, TRUE, FALSE);
						switch(rn2(3)){
							case 0:
								otmp->obj_material = COPPER;
							break;
							case 1:
								otmp->obj_material = GLASS;
							break;
							case 2:
								otmp->obj_material = SILVER;
							break;
						}
						if(otmp->spe < 1) otmp->spe = 1;
						otmp->objsize = MZ_SMALL;
						fix_object(otmp);
						(void) mpickobj(mtmp, otmp);
					} else { //Wizard leader
						mtmp->m_lev += 7;
						mtmp->mhpmax += d(7,8);
						mtmp->mhp = mtmp->mhpmax;
						otmp = mksobj(MIRROR, FALSE, FALSE);
						otmp->objsize = MZ_SMALL;
						fix_object(otmp);
						(void) mpickobj(mtmp, otmp);
						(void) mongets(mtmp, CLOAK_OF_PROTECTION);
						
						otmp = mksobj(STILETTO, TRUE, FALSE);
						switch(rn2(3)){
							case 0:
								otmp->obj_material = COPPER;
							break;
							case 1:
								otmp->obj_material = GLASS;
							break;
							case 2:
								otmp->obj_material = SILVER;
							break;
						}
						if(otmp->spe < 1) otmp->spe = 1;
						otmp->objsize = MZ_SMALL;
						fix_object(otmp);
						(void) mpickobj(mtmp, otmp);
					}
				}
			}
			if(ptr == &mons[PM_HUDOR_KAMEREL]){
				otmp = mksobj(MIRROR, FALSE, FALSE);
				otmp->obj_material = GLASS;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
			}
			if(ptr == &mons[PM_SHARAB_KAMEREL]){
				otmp = mksobj(MIRROR, FALSE, FALSE);
				otmp->obj_material = GLASS;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
			}
			if(ptr == &mons[PM_ARA_KAMEREL]){
				otmp = mksobj(KAMEREL_VAJRA, FALSE, FALSE);
				otmp->obj_material = GOLD;
				fix_object(otmp);
				otmp->spe = 1;
				(void) mpickobj(mtmp, otmp);
				otmp = mksobj(MIRROR, FALSE, FALSE);
				otmp->obj_material = GOLD;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
			}
		break;}
	    case S_LAW_ANGEL:
	    case S_NEU_ANGEL:
	    case S_CHA_ANGEL:
		{
		int spe2;
/*			if(ptr == &mons[PM_DESTROYER]){
				struct obj *otmp = mksobj(BROADSWORD, TRUE, FALSE);
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 10;
				(void) mpickobj(mtmp,otmp);

				otmp = mksobj(BROADSWORD, TRUE, FALSE);
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 10;
				(void) mpickobj(mtmp,otmp);

				otmp = mksobj(RIN_CONFLICT, TRUE, FALSE);
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				(void) mpickobj(mtmp,otmp);

				otmp = mksobj(RIN_INCREASE_DAMAGE, TRUE, FALSE);
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 19;
				(void) mpickobj(mtmp,otmp);

				otmp = mksobj(SKELETON_KEY, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_THIRD_KEY_OF_LAW));
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);
				return;

			}
			else if(ptr == &mons[PM_DANCER]){
				struct obj *otmp = mksobj(STILETTO, TRUE, FALSE);
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 19;
				(void) mpickobj(mtmp,otmp);

				otmp = mksobj(STILETTO, TRUE, FALSE);
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 19;
				(void) mpickobj(mtmp,otmp);

				otmp = mksobj(RIN_INCREASE_DAMAGE, TRUE, FALSE);
				otmp->blessed = TRUE;
				otmp->cursed = FALSE;
				otmp->spe = 28;
				(void) mpickobj(mtmp,otmp);
				return;

			}
*/
			if(ptr == &mons[PM_ARCADIAN_AVENGER]){
				(void)mongets(mtmp, SHORT_SWORD);
				(void)mongets(mtmp, SHORT_SWORD);
				return;//no random stuff
			} else if(ptr == &mons[PM_JUSTICE_ARCHON]){
				otmp = mksobj(GENTLEWOMAN_S_DRESS, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(HELMET, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(GAUNTLETS, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->obj_material = IRON;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(LONG_SWORD, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(KITE_SHIELD, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
			} else if(ptr == &mons[PM_SWORD_ARCHON]){
					//Nothing
			} else if(ptr == &mons[PM_SHIELD_ARCHON]){
				otmp = mksobj(SCALE_MAIL, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(HELMET, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(GAUNTLETS, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->obj_material = IRON;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(LANCE, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(WAR_HAMMER, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(KITE_SHIELD, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				otmp->spe = 3;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
			} else if(ptr == &mons[PM_TRUMPET_ARCHON]){
				otmp = mksobj(LEATHER_CLOAK, FALSE, FALSE);
			    bless(otmp);
				otmp->spe = 3;
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(GLOVES, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
			    bless(otmp);
				otmp->obj_material = SILVER;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(TWO_HANDED_SWORD, FALSE, FALSE);
			    bless(otmp);
				otmp->spe = 3;
				otmp->obj_material = SILVER;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
			} else if(ptr == &mons[PM_THRONE_ARCHON]){
				int artnum = rn2(8);
	
			    /* create minion stuff; can't use mongets */
			    otmp = mksobj(angelwepsbase[artnum], FALSE, FALSE);
	
			    /* make it special */
				otmp = oname(otmp, artiname(angelweps[artnum]));
			    bless(otmp);
			    otmp->spe = 7;
				otmp->objsize = MZ_LARGE;
				otmp->obj_material = SILVER;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
			    otmp = mksobj(SHIELD_OF_REFLECTION, FALSE, FALSE);
			    otmp->cursed = FALSE;
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
			    otmp->spe = 0;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(BRONZE_PLATE_MAIL, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(find_gcirclet(), FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				otmp->obj_material = COPPER;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(BRONZE_GAUNTLETS, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
			} else if(ptr == &mons[PM_LIGHT_ARCHON]){
				int artnum = rn2(8);
	
			    /* create minion stuff; can't use mongets */
			    otmp = mksobj(angelwepsbase[artnum], FALSE, FALSE);
	
			    /* make it special */
				otmp = oname(otmp, artiname(angelweps[artnum]));
			    bless(otmp);
			    otmp->spe = 7;
				otmp->objsize = MZ_LARGE;
				otmp->obj_material = SILVER;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
	
				if(artnum < ANGELTWOHANDERCUT){
					otmp = mksobj(SHIELD_OF_REFLECTION, FALSE, FALSE);
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->objsize = MZ_LARGE;
					otmp->obj_material = GLASS;
					otmp->spe = 0;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				} else {
					otmp = mksobj(AMULET_OF_REFLECTION, FALSE, FALSE);
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->objsize = MZ_LARGE;
					otmp->obj_material = GLASS;
					otmp->spe = 0;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				}
				
				otmp = mksobj(HELM_OF_BRILLIANCE, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				otmp->obj_material = GLASS;
				otmp->spe = 3;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(PLATE_MAIL, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				otmp->obj_material = GLASS;
				otmp->spe = 0;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
			} else if(ptr == &mons[PM_MONADIC_DEVA]){
				otmp = mksobj(TWO_HANDED_SWORD, FALSE, FALSE);
			    bless(otmp);
				spe2 = rn2(4);
			    otmp->oerodeproof = TRUE;
			    otmp->spe = max(otmp->spe, spe2);
			    (void) mpickobj(mtmp, otmp);
			} else if(ptr == &mons[PM_MOVANIC_DEVA]){
				otmp = mksobj(MORNING_STAR, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				spe2 = rn2(4);
			    otmp->spe = max(otmp->spe, spe2);
			    (void) mpickobj(mtmp, otmp);
			    otmp = mksobj(BUCKLER, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    otmp->spe = 0;
			    (void) mpickobj(mtmp, otmp);
				(void)mongets(mtmp, ROBE);
			} else if(ptr == &mons[PM_ASTRAL_DEVA]){
				otmp = mksobj(MACE, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    otmp->spe = 7;
			    (void) mpickobj(mtmp, otmp);
			    otmp = mksobj(CHAIN_MAIL, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    otmp->spe = 7;
			    (void) mpickobj(mtmp, otmp);
			} else if(ptr == &mons[PM_GRAHA_DEVA]){
				otmp = mksobj(TWO_HANDED_SWORD, FALSE, FALSE);
			    bless(otmp);
			    otmp->obj_material = COPPER;
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
			    otmp->spe = 9;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			    otmp = mksobj(BRONZE_PLATE_MAIL, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
			    otmp->spe = 7;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			    otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
			    otmp->spe = 1;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			    otmp = mksobj(BRONZE_GAUNTLETS, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
			    otmp->spe = 1;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			} else if(ptr == &mons[PM_SURYA_DEVA]){
				struct monst *dancer;
				dancer = makemon(&mons[PM_DANCING_BLADE], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH);
				dancer->mvar1 = (long)mtmp->m_id;
				dancer->mpeaceful = mtmp->mpeaceful;
				
			    otmp = mksobj(PLATE_MAIL, FALSE, FALSE);
			    bless(otmp);
			    otmp->obj_material = GOLD;
				otmp->objsize = MZ_LARGE;
			    otmp->spe = 7;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			    otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
			    bless(otmp);
			    otmp->obj_material = GOLD;
				otmp->objsize = MZ_LARGE;
			    otmp->spe = 1;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			    otmp = mksobj(GAUNTLETS_OF_POWER, FALSE, FALSE);
			    bless(otmp);
			    otmp->obj_material = GOLD;
				otmp->objsize = MZ_LARGE;
			    otmp->spe = 1;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			    otmp = mksobj(AMULET_OF_REFLECTION, FALSE, FALSE);
			    bless(otmp);
			    otmp->obj_material = GOLD;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			} else if(ptr == &mons[PM_MAHADEVA]){
				otmp = mksobj(SCIMITAR, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    otmp->spe = 7;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				otmp = mksobj(SCIMITAR, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    otmp->spe = 7;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				otmp = mksobj(SCIMITAR, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    otmp->spe = 7;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				otmp = mksobj(SCIMITAR, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    otmp->spe = 7;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			    otmp = mksobj(BRONZE_PLATE_MAIL, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
			    otmp->spe = 7;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			} else if(ptr == &mons[PM_COURE_ELADRIN]){
				(void)mongets(mtmp, GLOVES);
				(void)mongets(mtmp, LEATHER_JACKET);
				(void)mongets(mtmp, LOW_BOOTS);
				(void)mongets(mtmp, BOW);
				m_initthrow(mtmp, ARROW, d(4,4));
				switch(rnd(4)){
					case 1:
					(void)mongets(mtmp, RAPIER);
					break;
					case 2:
					(void)mongets(mtmp, SCIMITAR);
					(void)mongets(mtmp, KITE_SHIELD);
					break;
					case 3:
					(void)mongets(mtmp, SHORT_SWORD);
					(void)mongets(mtmp, BUCKLER);
					break;
					case 4:
					(void)mongets(mtmp, MOON_AXE);
					break;
				}
			} else if(ptr == &mons[PM_NOVIERE_ELADRIN]){
				(void)mongets(mtmp, GLOVES);
				(void)mongets(mtmp, LEATHER_JACKET);
				(void)mongets(mtmp, LOW_BOOTS);
				(void)mongets(mtmp, LEATHER_HELM);
				(void)mongets(mtmp, ELVEN_SPEAR);
				(void)mongets(mtmp, rn2(2) ? ELVEN_SICKLE : RAPIER);
			} else if(ptr == &mons[PM_BRALANI_ELADRIN]){
				(void)mongets(mtmp, CHAIN_MAIL);
				(void)mongets(mtmp, LEATHER_CLOAK);
				(void)mongets(mtmp, HIGH_BOOTS);
				(void)mongets(mtmp, HELMET);
				(void)mongets(mtmp, DWARVISH_SPEAR);
				(void)mongets(mtmp, DWARVISH_SPEAR);
				(void)mongets(mtmp, DWARVISH_SHORT_SWORD);
			} else if(ptr == &mons[PM_FIRRE_ELADRIN]){
				(void)mongets(mtmp, ELVEN_MITHRIL_COAT);
				(void)mongets(mtmp, ELVEN_SHIELD);
				(void)mongets(mtmp, ELVEN_CLOAK);
				(void)mongets(mtmp, ELVEN_BOOTS);
				(void)mongets(mtmp, ELVEN_HELM);
				(void)mongets(mtmp, ELVEN_SPEAR);
				(void)mongets(mtmp, ELVEN_BROADSWORD);
			} else if(ptr == &mons[PM_SHIERE_ELADRIN]){
				(void)mongets(mtmp, CRYSTAL_PLATE_MAIL);
				(void)mongets(mtmp, CRYSTAL_SHIELD);
				(void)mongets(mtmp, CRYSTAL_BOOTS);
				(void)mongets(mtmp, CRYSTAL_SWORD);
				(void)mongets(mtmp, WAN_STRIKING);
			} else if(ptr == &mons[PM_GHAELE_ELADRIN]){
				(void)mongets(mtmp, BRONZE_PLATE_MAIL);
				(void)mongets(mtmp, ROUNDSHIELD);
				(void)mongets(mtmp, ARMORED_BOOTS);
				(void)mongets(mtmp, BRONZE_HELM);
				(void)mongets(mtmp, LONG_SWORD);
				otmp = mksobj(LONG_SWORD, TRUE, FALSE);
				otmp->obj_material = COPPER;
			    (void) mpickobj(mtmp, otmp);
			} else if(ptr == &mons[PM_TULANI_ELADRIN]){
				(void)mongets(mtmp, CRYSTAL_PLATE_MAIL);
				(void)mongets(mtmp, ELVEN_CLOAK);
				(void)mongets(mtmp, CRYSTAL_BOOTS);
				(void)mongets(mtmp, CRYSTAL_GAUNTLETS);
				(void)mongets(mtmp, CRYSTAL_HELM);
			} else if(ptr == &mons[PM_GWYNHARWYF]){
				(void)mongets(mtmp, LEATHER_CLOAK);
				(void)mongets(mtmp, HIGH_BOOTS);
				(void)mongets(mtmp, SCIMITAR);
			} else if(ptr == &mons[PM_OONA]){
					//Note: Adjustments to how Oona's melee attacks were handled made her very weak without a weapon
					otmp = mksobj(LONG_SWORD, FALSE, FALSE);
					otmp->spe = 3;
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oproperties = OPROP_AXIOW|OPROP_LESSW;
					switch(u.oonaenergy){
						case AD_COLD:
							otmp->oproperties |= OPROP_ELECW; //superconducting?
						break;
						case AD_FIRE:
							otmp->oproperties |= OPROP_COLDW;
						break;
						case AD_ELEC:
							otmp->oproperties |= OPROP_FIREW;
						break;
					}
					otmp->obj_material = METAL;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(STILETTO, FALSE, FALSE);
					otmp->spe = 3;
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					otmp->oproperties = OPROP_AXIOW|OPROP_LESSW;
					switch(u.oonaenergy){
						case AD_COLD:
							otmp->oproperties |= OPROP_ELECW; //superconducting?
						break;
						case AD_FIRE:
							otmp->oproperties |= OPROP_COLDW;
						break;
						case AD_ELEC:
							otmp->oproperties |= OPROP_FIREW;
						break;
					}
					otmp->obj_material = METAL;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
			} else if(ptr == &mons[PM_LILLEND]){
				(void)mongets(mtmp, MASK);
				(void)mongets(mtmp, MASK);
				(void)mongets(mtmp, MASK);
				(void)mongets(mtmp, MASK);
				(void)mongets(mtmp, MASK);
				(void)mongets(mtmp, MASK);
				
				otmp = mksobj(ELVEN_BOW, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				spe2 = 3;
			    otmp->spe = max(otmp->spe, spe2);
			    (void) mpickobj(mtmp, otmp);
					m_initthrow(mtmp, ELVEN_ARROW, 12+rnd(30));
				(void)mongets(mtmp, WOODEN_HARP);
				otmp = mksobj(LONG_SWORD, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				spe2 = 3;
			    otmp->spe = max(otmp->spe, spe2);
			    (void) mpickobj(mtmp, otmp);
			} else {
				int artnum = rn2(8);
	
			    /* create minion stuff; can't use mongets */
			    otmp = mksobj(angelwepsbase[artnum], FALSE, FALSE);
	
			    /* maybe make it special */
			    if (!rn2(20) || is_lord(ptr))
				otmp = oname(otmp, artiname(angelweps[artnum]));
			    otmp->obj_material = SILVER;
			    bless(otmp);
			    if(is_lord(ptr)) spe2 = 7;
				else spe2 = rn2(4);
			    otmp->spe = max(otmp->spe, spe2);
			    (void) mpickobj(mtmp, otmp);
	
				if(artnum < ANGELTWOHANDERCUT){
					otmp = mksobj(!rn2(4) || is_lord(ptr) ?
						  SHIELD_OF_REFLECTION : KITE_SHIELD,
						  FALSE, FALSE);
					otmp->spe = 0;
					otmp->cursed = FALSE;
					otmp->obj_material = SILVER;
					(void) mpickobj(mtmp, otmp);
				} else if(is_lord(ptr) || !rn2(20)){
					otmp = mksobj(AMULET_OF_REFLECTION, FALSE, FALSE);
					otmp->cursed = FALSE;
					otmp->oerodeproof = TRUE;
					otmp->obj_material = SILVER;
					otmp->spe = 0;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				}
			}
		}
		break;
		
		case S_GNOME:
	      {
		int bias;

		bias = is_lord(ptr) + is_prince(ptr) * 2 + extra_nasty(ptr);
		switch(rnd(10 - (2 * bias))) {
		    case 1:
				m_initthrow(mtmp, DART, 12);
			break;
		    case 2:
			    (void) mongets(mtmp, CROSSBOW);
			    m_initthrow(mtmp, CROSSBOW_BOLT, 12);
			break;
		    case 3:
			    (void) mongets(mtmp, SLING);
			    m_initthrow(mtmp, ROCK, 12);
				(void) mongets(mtmp, AKLYS);
			break;
		    case 4:
				m_initthrow(mtmp, DAGGER, 3);
			break;
		    case 5:
				(void) mongets(mtmp, AKLYS);
			    (void) mongets(mtmp, SLING);
			    m_initthrow(mtmp, ROCK, 3);
			break;
		    default:
				(void) mongets(mtmp, AKLYS);
			break;
		}
		if (In_mines_quest(&u.uz)) {//note, gnomish wizards never have pointy hats.
			/* cm: Gnomes in dark mines have candles lit. */
			    otmp = mksobj(GNOMISH_POINTY_HAT, TRUE, FALSE);
			    (void) mpickobj(mtmp, otmp);
				if (!levl[mtmp->mx][mtmp->my].lit) {
					begin_burn(otmp, FALSE);
			    }	
		}
		else//Outside the mines, only one in 6 gnomes have hats.
		    if(!rn2(6)) (void)mongets(mtmp, GNOMISH_POINTY_HAT);
		  } //end case brackets

		break;

		case S_HUMANOID:
		if (mm == PM_HOBBIT) {
		    switch (rn2(3)) {
			case 0:
			    (void)mongets(mtmp, DAGGER);
			    break;
			case 1:
			    (void)mongets(mtmp, ELVEN_DAGGER);
			    break;
			case 2:
			    (void)mongets(mtmp, SLING);
			    m_initthrow(mtmp, ROCK, 6);
			    break;
		      }
		    if (!rn2(10)) (void)mongets(mtmp, ELVEN_MITHRIL_COAT);
		    if (!rn2(10)) (void)mongets(mtmp, DWARVISH_CLOAK);
		} else if(mm == PM_CHANGED) {
			if(!rn2(10)){
				otmp = mksobj(HAND_BLASTER, TRUE, FALSE);
				otmp->obj_material = BONE;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
			} else if(!rn2(9)){
				otmp = mksobj(PISTOL, TRUE, FALSE);
				otmp->obj_material = BONE;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(BULLET, TRUE, FALSE);
				otmp->obj_material = BONE;
				otmp->quan += rn1(20,20);
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
			} else {
				otmp = mksobj(STILETTO, TRUE, FALSE);
				otmp->obj_material = BONE;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
			}
		} else if(mm == PM_GNOLL) {
			switch(rnd(4)){
			case 1:
			(void)mongets(mtmp, FLAIL);
			if(rn2(2)) (void)mongets(mtmp, KNIFE);
			break;
			case 2:
			(void)mongets(mtmp, BOW);
			m_initthrow(mtmp, ARROW, d(8,4));
			break;
			case 3:
			(void)mongets(mtmp, BATTLE_AXE);
			break;
			case 4:
			(void)mongets(mtmp, SPEAR);
			(void)mongets(mtmp, SPEAR);
			(void)mongets(mtmp, SPEAR);
			break;
			}
			(void)mongets(mtmp, LEATHER_ARMOR);
			if(!rn2(2))(void)mongets(mtmp, GLOVES);
			if(!rn2(4))(void)mongets(mtmp, LEATHER_CLOAK);
		} else if(mm == PM_DEEP_ONE || mm == PM_DEEPER_ONE) {
		 if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)){
		    switch (rn2(3)) {
				case 0:
					(void)mongets(mtmp, LEATHER_JACKET);
				break;
				case 1:
					(void)mongets(mtmp, LEATHER_ARMOR);
				break;
				case 2:
					(void)mongets(mtmp, PLASTEEL_ARMOR);
				break;
			}
			(void)mongets(mtmp, FLACK_HELMET);
			(void)mongets(mtmp, GLOVES);
		    switch (rn2(6)) {
				case 0:
					otmp = mksobj(ASSAULT_RIFLE, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, BULLET);
					(void)mongets(mtmp, BULLET);
					(void)mongets(mtmp, BULLET);
					
					otmp = mksobj(KNIFE, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
				break;
				case 1:
					otmp = mksobj(PISTOL, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, BULLET);
					
					otmp = mksobj(SCIMITAR, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
				break;
				case 2:
					otmp = mksobj(HEAVY_MACHINE_GUN, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, BULLET);
					(void)mongets(mtmp, BULLET);
					(void)mongets(mtmp, BULLET);
					(void)mongets(mtmp, BULLET);
					
					otmp = mksobj(KNIFE, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
				break;
				case 3:
					otmp = mksobj(SUBMACHINE_GUN, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, BULLET);
					(void)mongets(mtmp, BULLET);
					(void)mongets(mtmp, BULLET);
					
					otmp = mksobj(SHORT_SWORD, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
				break;
				case 4:
					otmp = mksobj(SHOTGUN, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, SHOTGUN_SHELL);
					(void)mongets(mtmp, SHOTGUN_SHELL);
					
					otmp = mksobj(KNIFE, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
				break;
				case 5:
					otmp = mksobj(SNIPER_RIFLE, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
					
					(void)mongets(mtmp, BULLET);
					(void)mongets(mtmp, BULLET);
					(void)mongets(mtmp, BULLET);
					
					otmp = mksobj(SHORT_SWORD, TRUE, FALSE);
					otmp->oeroded = 3;
					(void) mpickobj(mtmp, otmp);
				break;
			}
			if(!rn2(6)){
				if(rn2(3)) (void)mongets(mtmp, FRAG_GRENADE);
				else if(rn2(2)) (void)mongets(mtmp, GAS_GRENADE);
				else {
					(void)mongets(mtmp, ROCKET_LAUNCHER);
					(void)mongets(mtmp, ROCKET);
				}
			}
		 } else if(!on_level(&rlyeh_level,&u.uz)){
		    switch (rn2(3)) {
				case 0:
					(void)mongets(mtmp, LEATHER_JACKET);
				break;
				case 1:
					(void)mongets(mtmp, LEATHER_ARMOR);
				break;
				case 2:
					otmp = mksobj(CHAIN_MAIL, TRUE, FALSE);
					otmp->oeroded = 2;
					(void) mpickobj(mtmp, otmp);
				break;
			}
			chance = d(4, 2) / 3;
			while (chance > 0){
				switch (rn2(6)) {
				case 0:
					otmp = mksobj(TWO_HANDED_SWORD, TRUE, FALSE);
					otmp->oeroded = 3;
					(void)mpickobj(mtmp, otmp);
				break;
				case 1:
					otmp = mksobj(SCIMITAR, TRUE, FALSE);
					otmp->oeroded = 3;
					(void)mpickobj(mtmp, otmp);
				break;
				case 2:
					otmp = mksobj(TRIDENT, TRUE, FALSE);
					otmp->oeroded = 3;
					(void)mpickobj(mtmp, otmp);
				break;
				case 3:
					otmp = mksobj(SHORT_SWORD, TRUE, FALSE);
					otmp->oeroded = 3;
					(void)mpickobj(mtmp, otmp);
				break;
				case 4:
					otmp = mksobj(DAGGER, TRUE, FALSE);
					otmp->oeroded = 3;
					(void)mpickobj(mtmp, otmp);
				break;
				case 5:
					otmp = mksobj(SPEAR, TRUE, FALSE);
					otmp->oeroded = 3;
					(void)mpickobj(mtmp, otmp);
				break;
				}
				chance--;
			}
		 }else{
			otmp = mksobj(TRIDENT, TRUE, FALSE);
			otmp->oerodeproof = 1;
			otmp->spe = mm==PM_DEEPER_ONE ? 6 : 3;
			otmp->blessed = FALSE;
			otmp->cursed = FALSE;
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(RANSEUR, TRUE, FALSE);
			otmp->oerodeproof = 1;
			otmp->spe = mm==PM_DEEPER_ONE ? 6 : 3;
			otmp->blessed = FALSE;
			otmp->cursed = FALSE;
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(JAVELIN, TRUE, FALSE);
			otmp->oeroded = 3;
			otmp->quan = mm==PM_DEEPER_ONE ? 6 : 3;
			otmp->owt = weight(otmp);
			otmp->spe = mm==PM_DEEPER_ONE ? 6 : 3;
			otmp->blessed = FALSE;
			otmp->cursed = FALSE;
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(BRONZE_PLATE_MAIL, TRUE, FALSE);
			otmp->spe = mm==PM_DEEPER_ONE ? 6 : 3;
			otmp->blessed = FALSE;
			otmp->cursed = TRUE;
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(OILSKIN_CLOAK, TRUE, FALSE);
			otmp->spe = mm==PM_DEEPER_ONE ? 6 : 3;
			otmp->blessed = FALSE;
			otmp->cursed = FALSE;
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(ORCISH_SHIELD, TRUE, FALSE);
			otmp->oeroded = 3;
			otmp->spe = mm==PM_DEEPER_ONE ? 6 : 3;
			otmp->blessed = FALSE;
			otmp->cursed = FALSE;
			(void) mpickobj(mtmp, otmp);
			
			(void) mongets(mtmp, rnd_attack_potion(mtmp));
			(void) mongets(mtmp, rnd_attack_potion(mtmp));
			if(mm == PM_DEEPER_ONE){
				otmp = mksobj(rnd_attack_wand(mtmp), TRUE, FALSE);
				otmp->spe = 1;
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp, otmp);
			}
		 }
		}else if((mm == PM_MIND_FLAYER || mm == PM_MASTER_MIND_FLAYER)){
			if(on_level(&rlyeh_level,&u.uz)){
				otmp = mksobj(BULLWHIP, TRUE, FALSE);
				otmp->oerodeproof = 1;
				otmp->spe = mm==PM_MASTER_MIND_FLAYER ? 4 : 0;
				otmp->blessed = FALSE;
				otmp->cursed = 1;
				otmp->objsize = mm==PM_MASTER_MIND_FLAYER ? MZ_LARGE : MZ_MEDIUM;
				otmp->oerodeproof = TRUE;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(SCALE_MAIL, TRUE, FALSE);
				otmp->spe = mm==PM_MASTER_MIND_FLAYER ? 4 : 1;
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->objsize = mm==PM_MASTER_MIND_FLAYER ? MZ_LARGE : MZ_MEDIUM;
				otmp->oerodeproof = TRUE;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(LEATHER_HELM, TRUE, FALSE);
				otmp->spe = mm==PM_MASTER_MIND_FLAYER ? 4 : 1;
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->objsize = mm==PM_MASTER_MIND_FLAYER ? MZ_LARGE : MZ_MEDIUM;
				otmp->oerodeproof = TRUE;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(GLOVES, TRUE, FALSE);
				otmp->spe = mm==PM_MASTER_MIND_FLAYER ? 4 : 1;
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->objsize = mm==PM_MASTER_MIND_FLAYER ? MZ_LARGE : MZ_MEDIUM;
				otmp->oerodeproof = TRUE;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(HIGH_BOOTS, TRUE, FALSE);
				otmp->spe = mm==PM_MASTER_MIND_FLAYER ? 4 : 1;
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->objsize = mm==PM_MASTER_MIND_FLAYER ? MZ_LARGE : MZ_MEDIUM;
				otmp->oerodeproof = TRUE;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(OILSKIN_CLOAK, TRUE, FALSE);
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->objsize = mm==PM_MASTER_MIND_FLAYER ? MZ_LARGE : MZ_MEDIUM;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
				
				(void) mongets(mtmp, rnd_attack_wand(mtmp));
				(void) mongets(mtmp, rnd_utility_wand(mtmp));
				
				(void) mongets(mtmp, rnd_utility_potion(mtmp));
				(void) mongets(mtmp, rnd_utility_potion(mtmp));
				
				(void) mongets(mtmp, rnd_attack_potion(mtmp));
				(void) mongets(mtmp, rnd_attack_potion(mtmp));
				(void) mongets(mtmp, rnd_attack_potion(mtmp));
				
				if(mm == PM_MASTER_MIND_FLAYER || !rn2(3)) mongets(mtmp, R_LYEHIAN_FACEPLATE);
			} else if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)){
				if(mm == PM_MASTER_MIND_FLAYER){
					otmp = mksobj(DOUBLE_LIGHTSABER, TRUE, FALSE);
					otmp->oerodeproof = 1;
					otmp->spe = 4;
					otmp->cobj->otyp = !rn2(3) ? RUBY : rn2(2) ? GREEN_FLUORITE : JADE+3/*Red glass*/;
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(ROBE, TRUE, FALSE);
					otmp->spe = 4;
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->objsize = MZ_LARGE;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HIGH_BOOTS, TRUE, FALSE);
					otmp->spe = 4;
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					otmp->objsize = MZ_LARGE;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				} else {
					otmp = mksobj(LIGHTSABER, TRUE, FALSE);
					otmp->oerodeproof = 1;
					otmp->spe = 1;
					otmp->ovar1 = !rn2(4) ? 6L : !rn2(3) ? 10L : rn2(2) ? 35L : 37L;
					otmp->cobj->otyp = !rn2(3) ? RUBY : rn2(2) ? GREEN_FLUORITE : JADE+3/*Red glass*/;
					otmp->blessed = TRUE;
					otmp->cursed = FALSE;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(ROBE, TRUE, FALSE);
					otmp->spe = 1;
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					(void) mpickobj(mtmp, otmp);
					
					otmp = mksobj(HIGH_BOOTS, TRUE, FALSE);
					otmp->spe = 1;
					otmp->blessed = FALSE;
					otmp->cursed = TRUE;
					(void) mpickobj(mtmp, otmp);
				}
			} else {
				mongets(mtmp, QUARTERSTAFF);
				mongets(mtmp, LEATHER_CLOAK);
				mongets(mtmp, GLOVES);
				mongets(mtmp, HIGH_BOOTS);
				mongets(mtmp, LEATHER_HELM);
				if(mm == PM_MASTER_MIND_FLAYER && !rn2(3)) mongets(mtmp, R_LYEHIAN_FACEPLATE);
				else if(mm == PM_MIND_FLAYER && !rn2(20)) mongets(mtmp, R_LYEHIAN_FACEPLATE);
			}
		} else if(mm == PM_GITHYANKI_PIRATE){
			otmp = mksobj(TWO_HANDED_SWORD, FALSE, FALSE);
			otmp->obj_material = SILVER;
			(void) mpickobj(mtmp, otmp);
			(void)mongets(mtmp, BRONZE_PLATE_MAIL);
			(void)mongets(mtmp, BRONZE_GAUNTLETS);
			(void)mongets(mtmp, ARMORED_BOOTS);
		} else if (is_dwarf(ptr)) { //slightly rearanged code so more dwarves get helms -D_E
		    if (rn2(7)) (void)mongets(mtmp, DWARVISH_CLOAK);
		    if (rn2(7)) (void)mongets(mtmp, IRON_SHOES);
			if (!rn2(4)) {
				(void)mongets(mtmp, DWARVISH_SHORT_SWORD);
			} else {
				if(!rn2(3)) (void)mongets(mtmp, DWARVISH_SPEAR);
				else (void)mongets(mtmp, DAGGER);
			}
			/* note: you can't use a mattock with a shield */
			if(!Is_minetown_level(&u.uz)){
				if (!rn2(3)) (void)mongets(mtmp, DWARVISH_MATTOCK);
				else {
					(void)mongets(mtmp, !rn2(3) ? PICK_AXE : AXE);
					if (!could_twoweap(ptr))
						(void)mongets(mtmp, DWARVISH_ROUNDSHIELD);
					else
						mongets(mtmp, DWARVISH_SHORT_SWORD);
				}
			}
			if (In_mines_quest(&u.uz) && !Is_minetown_level(&u.uz)) {
			/* MRKR: Dwarves in dark mines have their lamps on. */
			    otmp = mksobj(DWARVISH_HELM, TRUE, FALSE);
			    (void) mpickobj(mtmp, otmp);
				    if (!levl[mtmp->mx][mtmp->my].lit) {
					begin_burn(otmp, FALSE);
			    }
			}
			else {
			    (void)mongets(mtmp, DWARVISH_HELM);
				/* CM: Dwarves OUTSIDE the mines have booze. */
				mongets(mtmp, POT_BOOZE);
			}
 			if (!rn2(3)){
			    if(is_prince(ptr) || (is_lord(ptr) && !rn2(3))) (void)mongets(mtmp, DWARVISH_MITHRIL_COAT);
				else (void)mongets(mtmp, CHAIN_MAIL);
			}
		}
		break;
	    case S_KETER:
			if (mm == PM_CHOKHMAH_SEPHIRAH){
				otmp = mksobj(LONG_SWORD,
						  TRUE, rn2(13) ? FALSE : TRUE);
				otmp->spe = 7;
				otmp->oerodeproof = 1;
				(void) mpickobj(mtmp, otmp);
			}
		break;
	    case S_ORC:
		if(rn2(2)) (void)mongets(mtmp, ORCISH_HELM);
		switch (mm) {
		    case PM_ORC_OF_THE_AGES_OF_STARS:
				(void)mongets(mtmp, HIGH_ELVEN_WARSWORD);
				(void)mongets(mtmp, HIGH_ELVEN_WARSWORD);
				(void)mongets(mtmp, HIGH_ELVEN_HELM);
				(void)mongets(mtmp, HIGH_ELVEN_PLATE);
				(void)mongets(mtmp, ORCISH_CLOAK);
				(void)mongets(mtmp, HIGH_ELVEN_GAUNTLETS);
				(void)mongets(mtmp, ELVEN_BOOTS);
			    (void)mongets(mtmp, LEATHER_DRUM);
			break;
		    case PM_ANGBAND_ORC:
				(void)mongets(mtmp, SCIMITAR);
				if(rn2(2))
					(void)mongets(mtmp, SCIMITAR);
				else
					(void)mongets(mtmp, ORCISH_SHIELD);
				(void)mongets(mtmp, KNIFE);
				(void)mongets(mtmp, ORCISH_HELM);
				(void)mongets(mtmp, ORCISH_CHAIN_MAIL);
				(void)mongets(mtmp, ORCISH_CLOAK);
				(void)mongets(mtmp, GLOVES);
				(void)mongets(mtmp, HIGH_BOOTS);
			break;
		    case PM_ORC_CAPTAIN:
		    case PM_MORDOR_ORC:
			if(!rn2(3)) (void)mongets(mtmp, SCIMITAR);
			if(!rn2(3)) (void)mongets(mtmp, ORCISH_SHIELD);
			else if (could_twoweap(ptr))
						(void)mongets(mtmp, SCIMITAR);
			if(!rn2(3)) (void)mongets(mtmp, KNIFE);
			if(!rn2(3)) (void)mongets(mtmp, ORCISH_CHAIN_MAIL);
//ifdef BARD
			if (mm == PM_ORC_CAPTAIN ? !rn2(10) : !rn2(50)){
			    (void)mongets(mtmp, LEATHER_DRUM);
				if(!rn2(3)) {
					(void)mongets(mtmp, ORCISH_BOW);
					m_initthrow(mtmp, ORCISH_ARROW, 12);
				}
			}
//endif
			break;
		    case PM_URUK_CAPTAIN:
		    case PM_URUK_HAI:
			if(!rn2(3)) (void)mongets(mtmp, ORCISH_CLOAK);
			if(!rn2(3)) (void)mongets(mtmp, ORCISH_SHORT_SWORD);
			if(!rn2(3)) (void)mongets(mtmp, IRON_SHOES);
			if(!rn2(3)) (void)mongets(mtmp, URUK_HAI_SHIELD);
			else if (could_twoweap(ptr))
						(void)mongets(mtmp, ORCISH_SHORT_SWORD);
//ifdef BARD
			if (mm == PM_URUK_CAPTAIN ? !rn2(10) : !rn2(50)){
			    (void)mongets(mtmp, LEATHER_DRUM);
				(void)mongets(mtmp, CROSSBOW);
				m_initthrow(mtmp, CROSSBOW_BOLT, 12);
			} else {
				if(!rn2(3)) {
					(void)mongets(mtmp, CROSSBOW);
					m_initthrow(mtmp, CROSSBOW_BOLT, 12);
				}
			}
//endif
			break;
		    default:
			if (mm != PM_ORC_SHAMAN && rn2(2))
			  (void)mongets(mtmp, (mm == PM_GOBLIN || rn2(2) == 0)
						   ? ORCISH_DAGGER : SCIMITAR);
		}
		break;
	    case S_OGRE:
		if (!rn2(mm == PM_OGRE_KING ? 3 : mm == PM_OGRE_LORD ? 6 : 12))
		    (void) mongets(mtmp, BATTLE_AXE);
		else
		    (void) mongets(mtmp, CLUB);
		if(mm == PM_SIEGE_OGRE){
		    struct obj *otmp = mksobj(ARROW, TRUE, FALSE);
		    otmp->blessed = FALSE;
		    otmp->cursed = FALSE;
			otmp->quan = 240;
			otmp->owt = weight(otmp);
			(void) mpickobj(mtmp,otmp);
		}
		break;
	    case S_TROLL:
		if (!rn2(2)) switch (rn2(4)) {
		    case 0: (void)mongets(mtmp, RANSEUR); break;
		    case 1: (void)mongets(mtmp, PARTISAN); break;
		    case 2: (void)mongets(mtmp, GLAIVE); break;
		    case 3: (void)mongets(mtmp, SPETUM); break;
		}
		break;
	    case S_KOBOLD:
		if(ptr == &mons[PM_GREAT_HIGH_SHAMAN_OF_KURTULMAK]){
			(void)mongets(mtmp, CLOAK_OF_MAGIC_RESISTANCE);
			(void)mongets(mtmp, SHORT_SWORD);
			(void)mongets(mtmp, BUCKLER);
			m_initthrow(mtmp, CROSSBOW_BOLT, 36);
			m_initthrow(mtmp, DART, 36);
		} else {
			if (!rn2(4)) m_initthrow(mtmp, DART, 12);
			else if(!rn2(3)) (void)mongets(mtmp, SHORT_SWORD);
			else if(!rn2(2)) (void)mongets(mtmp, CLUB);
		}
		break;
	    case S_NYMPH:
			if(ptr == &mons[PM_THRIAE]){
				switch (rn2(6)) {
				/* MAJOR fall through ... */
				case 0: (void) mongets(mtmp, WAN_MAGIC_MISSILE);
				case 1: (void) mongets(mtmp, POT_EXTRA_HEALING);
				case 2: (void) mongets(mtmp, POT_HEALING);
					break;
				case 3: (void) mongets(mtmp, WAN_STRIKING);
				case 4: (void) mongets(mtmp, POT_HEALING);
				case 5: (void) mongets(mtmp, POT_EXTRA_HEALING);
				}
				(void)mongets(mtmp, (rn2(2) ? WOODEN_FLUTE : WOODEN_HARP));
			}
		break;
	    case S_CENTAUR:
		if(ptr == &mons[PM_DRIDER]){
			chance = rnd(10);
			if(chance >= 7) mongets(mtmp, DROVEN_PLATE_MAIL);
			else if(chance >= 6) mongets(mtmp, ELVEN_MITHRIL_COAT);
			else if(chance >= 4) mongets(mtmp, ORCISH_CHAIN_MAIL);
			(void)mongets(mtmp, DROVEN_CLOAK);
			(void)mongets(mtmp, DROVEN_LANCE);
			if(chance >= 5) (void)mongets(mtmp, DROVEN_SHORT_SWORD);
			(void)mongets(mtmp, DROVEN_CROSSBOW);
			m_initthrow(mtmp, DROVEN_BOLT, 20);
		} else if(ptr == &mons[PM_PRIESTESS_OF_GHAUNADAUR]){
			chance = rnd(10);
			if(chance >= 9) mongets(mtmp, CRYSTAL_PLATE_MAIL);
			else if(chance >= 6) mongets(mtmp, CLOAK_OF_PROTECTION);
			else if(chance == 5) mongets(mtmp, CONSORT_S_SUIT);
			(void)mongets(mtmp, CRYSTAL_SWORD);
		} else if(ptr == &mons[PM_ALIDER]){
			otmp = mksobj(FORCE_PIKE, TRUE, FALSE);
			otmp->spe = 8;
			otmp->ovar1 = 50 + d(5,10);
			otmp->recharged = rn1(3,3);
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(HAND_BLASTER, TRUE, FALSE);
			otmp->spe = 8;
			otmp->ovar1 = 50 + d(5,10);
			otmp->recharged = rn2(3);
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(find_signet_ring(), TRUE, FALSE);
			otmp->ohaluengr = TRUE;
			otmp->oward = LAST_BASTION_SYMBOL;
			(void) mpickobj(mtmp, otmp);
			
			(void)mongets(mtmp, CLOAK_OF_MAGIC_RESISTANCE);
			(void)mongets(mtmp, PLASTEEL_HELM);
			(void)mongets(mtmp, PLASTEEL_ARMOR);
			(void)mongets(mtmp, BODYGLOVE);
			(void)mongets(mtmp, GAUNTLETS_OF_POWER);
		}
		if (rn2(2)) {
		    if(ptr == &mons[PM_FOREST_CENTAUR] || ptr == &mons[PM_PLAINS_CENTAUR]) {
				(void)mongets(mtmp, BOW);
				m_initthrow(mtmp, ARROW, 12);
		    } else if(ptr == &mons[PM_MOUNTAIN_CENTAUR]) {
				(void)mongets(mtmp, CROSSBOW);
				m_initthrow(mtmp, CROSSBOW_BOLT, 12);
		    }
		}
		if(ptr == &mons[PM_MOUNTAIN_CENTAUR]){
			chance = rnd(10);
			if(chance == 10){
				mongets(mtmp, CHAIN_MAIL);
				mongets(mtmp, HELMET);
			} else if(chance >= 7){
				mongets(mtmp, SCALE_MAIL);
				mongets(mtmp, HELMET);
			} else if(chance >= 5){
				mongets(mtmp, STUDDED_LEATHER_ARMOR);
				mongets(mtmp, LEATHER_HELM);
			} else mongets(mtmp, LEATHER_ARMOR);
		}
		if(ptr == &mons[PM_FORMIAN_TASKMASTER]){
			switch (rn2(6)) {
			/* MAJOR fall through ... */
			case 0: (void) mongets(mtmp, WAN_MAGIC_MISSILE);
			case 1: (void) mongets(mtmp, POT_EXTRA_HEALING);
			case 2: (void) mongets(mtmp, POT_HEALING);
				break;
			case 3: (void) mongets(mtmp, WAN_STRIKING);
			case 4: (void) mongets(mtmp, POT_HEALING);
			case 5: (void) mongets(mtmp, POT_EXTRA_HEALING);
			}
			mongets(mtmp, BRONZE_PLATE_MAIL);
			mongets(mtmp, BRONZE_GAUNTLETS);
			otmp = mksobj(HELMET, TRUE, FALSE);
			otmp->obj_material = COPPER;
			fix_object(otmp);
			(void) mpickobj(mtmp, otmp);
			otmp = mksobj(LANCE, TRUE, FALSE);
			otmp->obj_material = COPPER;
			fix_object(otmp);
			(void) mpickobj(mtmp, otmp);
		}
		break;
	    case S_WRAITH:
		(void)mongets(mtmp, KNIFE);
		(void)mongets(mtmp, LONG_SWORD);
		break;
	    case S_ZOMBIE:
		if(mm == PM_UNDEAD_KNIGHT || mm == PM_WARRIOR_OF_SUNLIGHT){
			otmp = mksobj(LONG_SWORD, FALSE, FALSE);
			otmp->oeroded = 1;
			(void) mpickobj(mtmp, otmp);

			otmp = mksobj(KITE_SHIELD, FALSE, FALSE);
			otmp->oeroded = 1;
			(void) mpickobj(mtmp, otmp);

			otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
			otmp->obj_material = IRON;
			otmp->oeroded = 1;
			fix_object(otmp);
			(void) mpickobj(mtmp,otmp);

			otmp = mksobj(SCALE_MAIL, FALSE, FALSE);
			otmp->oeroded = 2;
			(void) mpickobj(mtmp, otmp);

			otmp = mksobj(GAUNTLETS, FALSE, FALSE);
			otmp->obj_material = IRON;
			otmp->oeroded = 1;
			fix_object(otmp);
			(void) mpickobj(mtmp,otmp);
				
		} else {
			if(mm == PM_SKELETAL_PIRATE){
				otmp = rn2(2) ? mksobj(SCIMITAR, FALSE, FALSE) : mksobj(KNIFE, FALSE, FALSE);
				// curse(otmp);
				otmp->oeroded = 1;
				(void) mpickobj(mtmp, otmp);
				
				otmp = rn2(2) ? mksobj(HIGH_BOOTS, FALSE, FALSE) : mksobj(LEATHER_JACKET, FALSE, FALSE);
				// curse(otmp);
				otmp->oeroded2 = 1;
				(void) mpickobj(mtmp, otmp);
				
				otmp = rn2(2) ? mksobj(FLINTLOCK, FALSE, FALSE) : mksobj(KNIFE, FALSE, FALSE);
				// curse(otmp);
				otmp->oeroded = 1;
				(void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(BULLET, FALSE, FALSE);
				otmp->quan += rnd(10);
				otmp->oeroded = 1;
				otmp->owt = weight(otmp);
				(void) mpickobj(mtmp, otmp);
				break;
			}
			if (!rn2(4)) (void)mongets(mtmp, LEATHER_ARMOR);
			if (!rn2(4))
				(void)mongets(mtmp, (rn2(3) ? KNIFE : SHORT_SWORD));
		}
		break;
	    case S_LIZARD:
		if (mm == PM_SALAMANDER)
			(void)mongets(mtmp, (rn2(7) ? SPEAR : rn2(3) ?
					     TRIDENT : STILETTO));
		break;
	    case S_DEMON:

		if(mm>PM_SHAYATEEN) return; //Lords handled above, no random cursed stuff!
		switch (mm) {
			case PM_DAMNED_PIRATE:
				otmp = mksobj(SCIMITAR, FALSE, FALSE);
				curse(otmp);
				(void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(LEATHER_ARMOR, FALSE, FALSE);
				curse(otmp);
				otmp->oeroded = 1;
				(void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(FLINTLOCK, FALSE, FALSE);
				curse(otmp);
				(void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(BULLET, FALSE, FALSE);
				otmp->quan += 10;
				otmp->owt = weight(otmp);
				(void) mpickobj(mtmp, otmp);
				
				return; //bypass general weapons
			break;
		    case PM_HORNED_DEVIL:
				(void)mongets(mtmp, rn2(4) ? TRIDENT : BULLWHIP);
			break;
		    case PM_ERINYS:{
				chance = rnd(10);
				if(chance >= 9){
					mongets(mtmp, HELMET);
					mongets(mtmp, PLATE_MAIL);
					mongets(mtmp, KITE_SHIELD);
					mongets(mtmp, GAUNTLETS);
					otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
					otmp->obj_material = IRON;
					(void) mpickobj(mtmp, otmp);
					mongets(mtmp, LONG_SWORD);
					// mongets(mtmp, IRON_BANDS);
				} else if(chance >= 6){
					mongets(mtmp, HELMET);
					mongets(mtmp, CHAIN_MAIL);
					mongets(mtmp, GLOVES);
					mongets(mtmp, HIGH_BOOTS);
					mongets(mtmp, TWO_HANDED_SWORD);
					// mongets(mtmp, ROPE_OF_ENTANGLING);
				} else if(chance >= 3){
					mongets(mtmp, LEATHER_HELM);
					mongets(mtmp, LEATHER_ARMOR);
					mongets(mtmp, GLOVES);
					mongets(mtmp, HIGH_BOOTS);
					mongets(mtmp, RAPIER);
					mongets(mtmp, STILETTO);
					mongets(mtmp, BOW);
					m_initthrow(mtmp, ARROW, 20);
					// mongets(mtmp, ROPE_OF_ENTANGLING);
				} else {
					mongets(mtmp, find_gcirclet());
					mongets(mtmp, GENTLEWOMAN_S_DRESS);
					mongets(mtmp, VICTORIAN_UNDERWEAR);
					otmp = mksobj(STILETTOS, TRUE, FALSE);
					otmp->obj_material = GOLD;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					mongets(mtmp, CRYSTAL_GAUNTLETS);
					mongets(mtmp, STILETTO);
					// mongets(mtmp, RAZOR_WIRE);
				}
			}break;
		    case PM_BONE_DEVIL:
				(void)mongets(mtmp, rn2(4) ? TRIDENT : WAR_HAMMER);
			break;
			case PM_ICE_DEVIL:
				if (!rn2(4)) {
					(void)mongets(mtmp, TRIDENT);
					if(!rn2(2))
						(void)mongets(mtmp, SHORT_SWORD);
				}
				else (void)mongets(mtmp, GLAIVE);
			break;
			case PM_NALFESHNEE:
				otmp = mksobj(KHAKKHARA, FALSE, FALSE);
				curse(otmp);
				otmp->oerodeproof = TRUE;
				otmp->obj_material = IRON;
				(void) mpickobj(mtmp, otmp);
			break;
		    case PM_MARILITH:{
				if(Inhell){
					chance = rnd(10);
					if(chance >= 9) mongets(mtmp, PLATE_MAIL);
					else if(chance >= 6) mongets(mtmp, CHAIN_MAIL);
					else if(chance >= 3) mongets(mtmp, STUDDED_LEATHER_ARMOR);
					else mongets(mtmp, LEATHER_ARMOR);					
					switch(rn2(3)){
						case 0:
							mongets(mtmp, TRIDENT);
						break;
						case 1:
							mongets(mtmp, STILETTO);
						break;
						case 2:
							mongets(mtmp, SICKLE);
						break;
						
					}
					switch(rn2(3)){
						case 0:
							mongets(mtmp, AXE);
						break;
						case 1:
							mongets(mtmp, SHORT_SWORD);
						break;
						case 2:
							mongets(mtmp, MACE);
						break;
						
					}
					switch(rn2(3)){
						case 0:
							mongets(mtmp, RAPIER);
						break;
						case 1:
							mongets(mtmp, RUNESWORD);
						break;
						case 2:
							mongets(mtmp, BROADSWORD);
						break;
						
					}
					switch(rn2(3)){
						case 0:
							mongets(mtmp, LONG_SWORD);
						break;
						case 1:
							mongets(mtmp, KATANA);
						break;
						case 2:
							mongets(mtmp, SABER);
						break;
						
					}
					switch(rn2(2)){
						case 0:
							mongets(mtmp, SCIMITAR);
						break;
						case 1:
							mongets(mtmp, MORNING_STAR);
						break;
						
					}
					switch(rn2(2)){
						case 0:
							mongets(mtmp, WAR_HAMMER);
						break;
						case 1:
							mongets(mtmp, FLAIL);
						break;
						
					}
				}
			}break;
		    case PM_PIT_FIEND:
				otmp = mksobj(TRIDENT, FALSE, FALSE);
				otmp->spe = rnd(9);
				(void) mpickobj(mtmp, otmp);
			break;
		    case PM_FALLEN_ANGEL:
				mongets(mtmp, LONG_SWORD);
			break;
		    case PM_ANCIENT_OF_ICE:
				otmp = mksobj(MACE, FALSE, FALSE);
				otmp->obj_material = METAL;
				fix_object(otmp);
				otmp->spe = rnd(8);
				(void) mpickobj(mtmp, otmp);
				(void) mongets(mtmp, CRYSTAL_HELM);
				(void) mongets(mtmp, CRYSTAL_PLATE_MAIL);
				(void) mongets(mtmp, CRYSTAL_GAUNTLETS);
				(void) mongets(mtmp, CRYSTAL_BOOTS);
			break;
		    case PM_ANCIENT_OF_DEATH:
				otmp = mksobj(SCYTHE, FALSE, FALSE);
				otmp->spe = 8;
				(void) mpickobj(mtmp, otmp);
				return; //bypass general weapons
			break;
		    case PM_BALROG:
				(void)mongets(mtmp, BULLWHIP);
				(void)mongets(mtmp, BROADSWORD);
				return; //bypass general weapons
			break;
		    case PM_NESSIAN_PIT_FIEND:
				otmp = mksobj(TRIDENT, FALSE, FALSE);
				otmp->spe = 9;
				(void) mpickobj(mtmp, otmp);
			break;
		}
		/* prevent djinnis and mail daemons from leaving objects when
		 * they vanish
		 */
		if (!is_demon(ptr) && mm != PM_DAMNED_PIRATE) break;
		/* fall thru */
/*
 *	Now the general case, Some chance of getting some type
 *	of weapon for "normal" monsters.  Certain special types
 *	of monsters will get a bonus chance or different selections.
 */
	    default:
	      {
		int bias;

		bias = is_lord(ptr) + is_prince(ptr) * 2 + extra_nasty(ptr);
		switch(rnd(14 - (2 * bias))) {
		    case 1:
			if(strongmonst(ptr)) (void) mongets(mtmp, BATTLE_AXE);
			else m_initthrow(mtmp, DART, 12);
			break;
		    case 2:
			if(strongmonst(ptr))
			    (void) mongets(mtmp, TWO_HANDED_SWORD);
			else {
			    (void) mongets(mtmp, CROSSBOW);
			    m_initthrow(mtmp, CROSSBOW_BOLT, 12);
			}
			break;
		    case 3:
			(void) mongets(mtmp, BOW);
			m_initthrow(mtmp, ARROW, 12);
			break;
		    case 4:
			if(strongmonst(ptr)) (void) mongets(mtmp, LONG_SWORD);
			else m_initthrow(mtmp, DAGGER, 3);
			break;
		    case 5:
			if(strongmonst(ptr))
			    (void) mongets(mtmp, LUCERN_HAMMER);
			else (void) mongets(mtmp, AKLYS);
			break;
		    default:
			break;
		}
	      }
	      break;
	}
	if ((int) mtmp->m_lev > rn2(75))
		(void) mongets(mtmp, rnd_offensive_item(mtmp));
}

#endif /* OVL2 */
#ifdef OVL1

#ifdef GOLDOBJ
/*
 *   Makes up money for monster's inventory.
 *   This will change with silver & copper coins
 */
void 
mkmonmoney(mtmp, amount)
struct monst *mtmp;
long amount;
{
    struct obj *gold = mksobj(GOLD_PIECE, FALSE, FALSE);
    gold->quan = amount;
    add_to_minv(mtmp, gold);
}
#endif

STATIC_OVL void
m_initinv(mtmp)
register struct	monst	*mtmp;
{
	register int cnt;
	int chance;
	register struct obj *otmp;
	register struct permonst *ptr = mtmp->data;
#ifdef REINCARNATION
	if (Is_rogue_level(&u.uz)) return;
#endif
/*
 *	Soldiers get armour & rations - armour approximates their ac.
 *	Nymphs may get mirror or potion of object detection.
 */
	switch(ptr->mlet) {

	    case S_HUMAN:
		if(is_mercenary(ptr)) {
		    register int mac;
			
		    switch(monsndx(ptr)) {
			case PM_GUARD: mac = -1; break;
#ifdef CONVICT
			case PM_PRISON_GUARD: mac = -2; break;
#endif /* CONVICT */
			case PM_SOLDIER: mac = 3; break;
			case PM_MYRMIDON_HOPLITE: mac = 3; break;
			case PM_SERGEANT: mac = 0; break;
			case PM_MYRMIDON_LOCHIAS: mac = 0; break;
			case PM_LIEUTENANT: mac = -2; break;
			case PM_MYRMIDON_YPOLOCHAGOS: mac = 0; break;
			case PM_CAPTAIN: mac = -3; break;
			case PM_MYRMIDON_LOCHAGOS: mac = -3; break;
			case PM_WATCHMAN: mac = 3; break;
			case PM_WATCH_CAPTAIN: mac = -2; break;
			default: impossible("odd mercenary %d?", monsndx(ptr));
				mac = 0;
				break;
		    }
			
			if(In_law(&u.uz) && is_army_pm(monsndx(ptr))){
				if(ptr == &mons[PM_CAPTAIN] || ptr == &mons[PM_LIEUTENANT]){
					mongets(mtmp, HARMONIUM_PLATE);
					mongets(mtmp, HARMONIUM_HELM);
					mongets(mtmp, HARMONIUM_GAUNTLETS);
					mongets(mtmp, HARMONIUM_BOOTS);
				} else {
					mongets(mtmp, HARMONIUM_SCALE_MAIL);
					mongets(mtmp, HARMONIUM_HELM);
					mongets(mtmp, HARMONIUM_GAUNTLETS);
					mongets(mtmp, HARMONIUM_BOOTS);
				}
			} else {
				if (mac < -1 && rn2(5))
				mac += objects[PLATE_MAIL].a_ac + mongets(mtmp, PLATE_MAIL);
				else if (mac < 3 && rn2(5))
				mac += objects[SPLINT_MAIL].a_ac + mongets(mtmp, (rn2(3)) ?
						   SPLINT_MAIL : BANDED_MAIL);
				else if (rn2(5))
				mac += objects[RING_MAIL].a_ac + mongets(mtmp, (rn2(3)) ?
						   RING_MAIL : STUDDED_LEATHER_ARMOR);
				else
				mac += objects[LEATHER_ARMOR].a_ac + mongets(mtmp, LEATHER_ARMOR);

				if (mac < 10 && rn2(3))
				mac += objects[HELMET].a_ac + mongets(mtmp, HELMET);
				else if (mac < 10 && rn2(2))
				mac += objects[LEATHER_HELM].a_ac + mongets(mtmp, LEATHER_HELM);
				else if (mac < 10 && !rn2(10))
				mac += objects[WAR_HAT].a_ac + mongets(mtmp, WAR_HAT);
				
				if (mac < 10 && rn2(3))
				mac += objects[BUCKLER].a_ac + mongets(mtmp, BUCKLER);
				else if (mac < 10 && rn2(2))
				mac += objects[KITE_SHIELD].a_ac + mongets(mtmp, KITE_SHIELD);
				else if (mac < 10 && rn2(2))
				mac += objects[ROUNDSHIELD].a_ac + mongets(mtmp, ROUNDSHIELD);
			
				if (mac < 10 && rn2(3))
				mac += objects[LOW_BOOTS].a_ac + mongets(mtmp, LOW_BOOTS);
				else if (mac < 10 && rn2(2))
				mac += objects[HIGH_BOOTS].a_ac + mongets(mtmp, HIGH_BOOTS);
				else if (mac < 10 && rn2(2))
				mac += objects[ARMORED_BOOTS].a_ac + mongets(mtmp, ARMORED_BOOTS);
			
				if (mac < 10 && rn2(3))
				mac += objects[GLOVES].a_ac + mongets(mtmp, GLOVES);
				else if (mac < 10 && rn2(2))
				mac += objects[GAUNTLETS].a_ac + mongets(mtmp, GAUNTLETS);
			
				if (mac < 10 && rn2(2))
				mac += objects[LEATHER_CLOAK].a_ac + mongets(mtmp, LEATHER_CLOAK);
			}
			if(ptr != &mons[PM_GUARD] &&
#ifdef CONVICT
			ptr != &mons[PM_PRISON_GUARD] &&
#endif /* CONVICT */
			ptr != &mons[PM_WATCHMAN] &&
			ptr != &mons[PM_WATCH_CAPTAIN]) {
			if(!(level.flags.has_barracks || In_law(&u.uz) || in_mklev || undeadfaction)){
				if (!rn2(3)) (void) mongets(mtmp, K_RATION);
				if (!rn2(2)) (void) mongets(mtmp, C_RATION);
			}
			if (ptr != &mons[PM_SOLDIER] && !rn2(3))
				(void) mongets(mtmp, BUGLE);
			} else
			   if (ptr == &mons[PM_WATCHMAN] && rn2(3))
				(void) mongets(mtmp, TIN_WHISTLE);
		} else if (ptr == &mons[PM_SHOPKEEPER]) {
		    (void) mongets(mtmp,SKELETON_KEY);
			if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)){
				(void) mongets(mtmp, SHOTGUN);
				(void) mongets(mtmp, SHOTGUN_SHELL);
				(void) mongets(mtmp, SHOTGUN_SHELL);
				(void) mongets(mtmp, POT_EXTRA_HEALING);
				(void) mongets(mtmp, POT_HEALING);
			} else switch (rn2(6)) {
				/* MAJOR fall through ... */
				case 0: (void) mongets(mtmp, WAN_MAGIC_MISSILE);
				case 1: (void) mongets(mtmp, POT_EXTRA_HEALING);
				case 2: (void) mongets(mtmp, POT_HEALING);
					break;
				case 3: (void) mongets(mtmp, WAN_STRIKING);
				case 4: (void) mongets(mtmp, POT_HEALING);
				case 5: (void) mongets(mtmp, POT_EXTRA_HEALING);
		    }
		} else if (ptr->msound == MS_PRIEST ||
			quest_mon_represents_role(ptr,PM_PRIEST)) {
		    (void) mongets(mtmp, rn2(7) ? ROBE :
					     rn2(3) ? CLOAK_OF_PROTECTION :
						 CLOAK_OF_MAGIC_RESISTANCE);
		    (void) mongets(mtmp, BUCKLER);
#ifndef GOLDOBJ
		    mtmp->mgold = (long)rn1(10,20);
#else
		    mkmonmoney(mtmp,(long)rn1(10,20));
#endif
		} else if (quest_mon_represents_role(ptr,PM_MONK)) {
		    (void) mongets(mtmp, rn2(11) ? ROBE :
					     CLOAK_OF_MAGIC_RESISTANCE);
		} else if(ptr == &mons[PM_MEDUSA]){
		 struct engr *oep = engr_at(mtmp->mx,mtmp->my);
		 int i;
		 for(i=0; i<3; i++){
			if(!oep){
				make_engr_at(mtmp->mx, mtmp->my,
			     "", 0L, DUST);
				oep = engr_at(mtmp->mx,mtmp->my);
			}
			oep->ward_id = GORGONEION;
			oep->halu_ward = 0;
			oep->ward_type = BURN;
			oep->complete_wards = 1;
			rloc_engr(oep);
			oep = engr_at(mtmp->mx,mtmp->my);
		 }
		} else if(ptr == &mons[PM_ILLURIEN_OF_THE_MYRIAD_GLIMPSES] && !(u.uevent.ukilled_illurien)){
			otmp = mksobj(SPE_SECRETS, TRUE, FALSE);
			if(!rn2(3)){
				otmp = oname(otmp, artiname(ART_BOOK_OF_LOST_NAMES));
			} else if(!rn2(2)){
				otmp = oname(otmp, artiname(ART_BOOK_OF_INFINITE_SPELLS));		
			} else {
				otmp = oname(otmp, artiname(ART_NECRONOMICON));		
			}
			otmp->blessed = FALSE;
			otmp->cursed = FALSE;
			(void) mpickobj(mtmp, otmp);
		} else if(is_drow(ptr)){
			if(ptr == &mons[PM_MINDLESS_THRALL]){
				struct obj *otmp;
				otmp = mksobj(DROVEN_CLOAK, TRUE, FALSE);
				otmp->oerodeproof = FALSE;
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = -d(1,3);
				otmp->ovar1 = 1;
				(void) mpickobj(mtmp,otmp);
			}
			else if( ptr == &mons[PM_A_GONE] ){
				struct obj *otmp;
				otmp = mksobj(DROVEN_CLOAK, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_WEB_OF_THE_CHOSEN));
				otmp->oerodeproof = FALSE;
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = -d(1,3);
				otmp->ovar1 = 3;
				(void) mpickobj(mtmp,otmp);
			}
		}
		break;
//		case S_IMP:
//			switch(monsndx(ptr)) {
//			case 
//			}
//		break;
		case S_ANT:
			if(In_law(&u.uz)){
				//Civilized ants
				if(mtmp->data == &mons[PM_SOLDIER_ANT]){
					chance = rnd(10);
					if(chance == 10) mongets(mtmp, PLATE_MAIL);
					else if(chance >= 8) mongets(mtmp, CHAIN_MAIL);
					else if(chance >= 5) mongets(mtmp, SCALE_MAIL);
					if(chance >= 5) mongets(mtmp, HELMET);
				} else if(mtmp->data == &mons[PM_KILLER_BEE]){
					if(!rn2(4)){
						mongets(mtmp, LEATHER_ARMOR);
					} else if(!rn2(3)){
						mongets(mtmp, BLACK_DRESS);
					}
				} else if(mtmp->data == &mons[PM_QUEEN_BEE]){
					chance = rnd(10);
					if(chance == 10) mongets(mtmp, PLATE_MAIL);
					else if(chance >= 8) mongets(mtmp, SCALE_MAIL);
					else if(chance >= 5) mongets(mtmp, GENTLEWOMAN_S_DRESS);
					if(chance >= 8) mongets(mtmp, HELMET);
				}
			}
			if(ptr == &mons[PM_VALAVI]){
				switch (rnd(2)) {
					case 1: (void) mongets(mtmp, POT_EXTRA_HEALING);
					case 2: (void) mongets(mtmp, POT_HEALING);
				}
				if(rn2(3)){
					mongets(mtmp, SHEPHERD_S_CROOK);
					mongets(mtmp, KNIFE);
					mongets(mtmp, KNIFE);
				} else {
					mongets(mtmp, SCIMITAR);
					switch (rnd(3)) {
						case 1:
							(void) mongets(mtmp, ROUNDSHIELD);
						break;
						case 2:
							(void) mongets(mtmp, BUCKLER);
						break;
						case 3:
							(void) mongets(mtmp, SCIMITAR);
						break;
					}
					mongets(mtmp, SCIMITAR);
					mongets(mtmp, SCIMITAR);
				}
		    }
		break;
		case S_DOG:
			//Escaped war-dog
			if(mtmp->data == &mons[PM_LARGE_DOG]){
				chance = rnd(100);
				if(chance == 100) mongets(mtmp, PLATE_MAIL);
				else if(chance >= 96) mongets(mtmp, SCALE_MAIL);
				else if(chance >= 90) mongets(mtmp, LEATHER_ARMOR);
				if(chance >= 96) mongets(mtmp, HELMET);
			//Escaped orcish mount
			} else if(mtmp->data == &mons[PM_WARG]){
				chance = rnd(10);
				if(chance == 10) mongets(mtmp, ORCISH_CHAIN_MAIL);
				else if(chance >= 8) mongets(mtmp, ORCISH_RING_MAIL);
				else if(chance >= 5) mongets(mtmp, LEATHER_ARMOR);
				if(chance >= 5) mongets(mtmp, ORCISH_HELM);
				if(!rn2(20)){
					otmp = mksobj(SADDLE, TRUE, FALSE);
					if (otmp) {
						if (mpickobj(mtmp, otmp)) break;//panic("merged saddle?");
						mtmp->misc_worn_check |= W_SADDLE;
						otmp->owornmask = W_SADDLE;
						otmp->leashmon = mtmp->m_id;
						update_mon_intrinsics(mtmp, otmp, TRUE, TRUE);
					}
				}
			}
		case S_QUADRUPED:
			//Escaped war-elephant
			if(mtmp->data == &mons[PM_MUMAK]){
				chance = rnd(100);
				if(chance == 100) mongets(mtmp, BRONZE_PLATE_MAIL);
				else if(chance >= 96) mongets(mtmp, ORCISH_CHAIN_MAIL);
				else if(chance >= 90) mongets(mtmp, ORCISH_RING_MAIL);
				if(chance == 100) mongets(mtmp, BRONZE_HELM);
				else if(chance >= 90) mongets(mtmp, ORCISH_HELM);
				if(chance >= 95){
					otmp = mksobj(SADDLE, TRUE, FALSE);
					if (otmp) {
						if (mpickobj(mtmp, otmp)) break;//panic("merged saddle?");
						mtmp->misc_worn_check |= W_SADDLE;
						otmp->owornmask = W_SADDLE;
						otmp->leashmon = mtmp->m_id;
						update_mon_intrinsics(mtmp, otmp, TRUE, TRUE);
					}
				}
			}
		break;
		case S_RODENT:
			//6) There is obviously no "underground kingdom beneath London, inhabited by huge, intelligent rodents."
			if(mtmp->data == &mons[PM_ENORMOUS_RAT]){
				chance = rnd(100);
				if(chance == 100){
					if(mtmp->female) mongets(mtmp, GENTLEWOMAN_S_DRESS);
					else mongets(mtmp, GENTLEMAN_S_SUIT);
					mongets(mtmp, FEDORA);
				}
				else if(chance >= 90) mongets(mtmp, LEATHER_JACKET);
			}
		break;
		case S_SPIDER:
			//Escaped drow pet
			if(mtmp->data == &mons[PM_GIANT_SPIDER]){
				chance = rnd(100);
				if(chance == 100) mongets(mtmp, DROVEN_PLATE_MAIL);
				else if(chance >= 96) mongets(mtmp, DROVEN_CHAIN_MAIL);
				else if(chance >= 90) mongets(mtmp, LEATHER_ARMOR);
				if(chance >= 96) mongets(mtmp, DROVEN_HELM);
				else if(chance >= 90) mongets(mtmp, LEATHER_HELM);
				if(chance >= 96) mongets(mtmp, DROVEN_CLOAK);
				if(!rn2(20)){
					otmp = mksobj(SADDLE, TRUE, FALSE);
					if (otmp) {
						if (mpickobj(mtmp, otmp)) break;//panic("merged saddle?");
						mtmp->misc_worn_check |= W_SADDLE;
						otmp->owornmask = W_SADDLE;
						otmp->leashmon = mtmp->m_id;
						update_mon_intrinsics(mtmp, otmp, TRUE, TRUE);
					}
				}
			}
		break;
		case S_UNICORN:
			//Escaped warhorse
			if(mtmp->data == &mons[PM_WARHORSE]){
				chance = rnd(10);
				if(chance == 10) mongets(mtmp, PLATE_MAIL);
				else if(chance >= 8) mongets(mtmp, CHAIN_MAIL);
				else if(chance >= 5) mongets(mtmp, SCALE_MAIL);
				if(chance >= 5) mongets(mtmp, HELMET);
				if(!rn2(4)){
					otmp = mksobj(SADDLE, TRUE, FALSE);
					if (otmp) {
						if (mpickobj(mtmp, otmp)) break;//panic("merged saddle?");
						mtmp->misc_worn_check |= W_SADDLE;
						otmp->owornmask = W_SADDLE;
						otmp->leashmon = mtmp->m_id;
						update_mon_intrinsics(mtmp, otmp, TRUE, TRUE);
					}
				}
			}
		break;
		case S_BAT:
			if(mtmp->data == &mons[PM_CHIROPTERAN]){
				chance = rnd(100);
				if(chance >= 75) mongets(mtmp, STUDDED_LEATHER_ARMOR);
				else mongets(mtmp, LEATHER_ARMOR);
				mongets(mtmp, LEATHER_HELM);
				chance = rnd(100);
				if(chance >= 75) mongets(mtmp, SCYTHE);
				else if(chance >= 50) mongets(mtmp, FAUCHARD);
				else mongets(mtmp, QUARTERSTAFF);
			}
		break;
		case S_LIZARD:
			//Escaped drow pet
			if(mtmp->data == &mons[PM_CAVE_LIZARD]){
				chance = rnd(100);
				if(chance == 100) mongets(mtmp, DROVEN_PLATE_MAIL);
				else if(chance >= 96) mongets(mtmp, DROVEN_CHAIN_MAIL);
				else if(chance >= 80) mongets(mtmp, LEATHER_ARMOR);
				if(chance >= 96) mongets(mtmp, DROVEN_HELM);
				else if(chance >= 80) mongets(mtmp, LEATHER_HELM);
				if(chance >= 96) mongets(mtmp, DROVEN_CLOAK);
				if(!rn2(20)){
					otmp = mksobj(SADDLE, TRUE, FALSE);
					if (otmp) {
						if (mpickobj(mtmp, otmp)) break;//panic("merged saddle?");
						mtmp->misc_worn_check |= W_SADDLE;
						otmp->owornmask = W_SADDLE;
						otmp->leashmon = mtmp->m_id;
						update_mon_intrinsics(mtmp, otmp, TRUE, TRUE);
					}
				}
			//Escaped drow mount
			} else if(mtmp->data == &mons[PM_LARGE_CAVE_LIZARD]){
				chance = rnd(100);
				if(chance == 98) mongets(mtmp, DROVEN_PLATE_MAIL);
				else if(chance >= 90) mongets(mtmp, DROVEN_CHAIN_MAIL);
				else if(chance >= 70) mongets(mtmp, LEATHER_ARMOR);
				if(chance >= 90) mongets(mtmp, DROVEN_HELM);
				else if(chance >= 70) mongets(mtmp, LEATHER_HELM);
				if(chance >= 90) mongets(mtmp, DROVEN_CLOAK);
				if(!rn2(4)){
					otmp = mksobj(SADDLE, TRUE, FALSE);
					if (otmp) {
						if (mpickobj(mtmp, otmp)) break;//panic("merged saddle?");
						mtmp->misc_worn_check |= W_SADDLE;
						otmp->owornmask = W_SADDLE;
						otmp->leashmon = mtmp->m_id;
						update_mon_intrinsics(mtmp, otmp, TRUE, TRUE);
					}
				}
			}
		break;
	    case S_NYMPH:
		if(!rn2(2)) (void) mongets(mtmp, MIRROR);
		if(!rn2(2)) (void) mongets(mtmp, POT_OBJECT_DETECTION);
		break;
		case S_CENTAUR:
 		break;
		case S_FUNGUS:
		{
			int i = 0;
			int n = 0;
			int id = 0;
			int rng = 0;
			switch(monsndx(ptr)){
				case PM_MIGO_WORKER:
					rng = LAST_GEM-DILITHIUM_CRYSTAL;
					n = rn2(8);
					for(i=0; i<n; i++) (void)mongets(mtmp, rn2(rng)+DILITHIUM_CRYSTAL);
					n = rn2(10);
					for(i=0; i<n; i++) (void)mongets(mtmp, FLINT);
					n = rn2(20);
					for(i=0; i<n; i++) (void)mongets(mtmp, ROCK);
				break;
				case PM_MIGO_SOLDIER:
					if(!rn2(4)){
						otmp = mksobj(WAN_LIGHTNING, TRUE, FALSE);
						bless(otmp);
						otmp->recharged=7;
						otmp->spe = 2;
						(void) mpickobj(mtmp, otmp);
					}
					n = rn2(200)+200;
					for(i=0; i<n; i++) (void)mongets(mtmp, GOLD_PIECE);
				break;
				case PM_MIGO_PHILOSOPHER:
					if(!rn2(6)){ 
						otmp = mksobj(WAN_LIGHTNING, TRUE, FALSE);
						bless(otmp);
						otmp->recharged=7;
						otmp->spe = 3;
						(void) mpickobj(mtmp, otmp);
					}
					rng = LAST_GEM-DILITHIUM_CRYSTAL;
					n = rn2(3);
					for(i=0; i<n; i++) (void)mongets(mtmp, rn2(rng)+DILITHIUM_CRYSTAL);
					rng = SCR_STINKING_CLOUD-SCR_ENCHANT_ARMOR;
					n = rn2(4)+3;
					for(i=0; i<n; i++) (void)mongets(mtmp, rn2(rng)+SCR_ENCHANT_ARMOR);
				break;
				case PM_MIGO_QUEEN:
					
					otmp = mksobj(WAN_LIGHTNING, TRUE, FALSE);
					bless(otmp);
					otmp->recharged=2;
					otmp->spe = 6;
					(void) mpickobj(mtmp, otmp);
					
					rng = LAST_GEM-DILITHIUM_CRYSTAL;
					n = rn2(10);
					for(i=0; i<n; i++) (void)mongets(mtmp, rn2(rng)+DILITHIUM_CRYSTAL);
					rng = SCR_STINKING_CLOUD-SCR_ENCHANT_ARMOR;
					n = rn2(4);
					for(i=0; i<n; i++) (void)mongets(mtmp, rn2(rng)+SCR_ENCHANT_ARMOR);
				break;
			}
		}
		break;
	    case S_GIANT:
		if (ptr == &mons[PM_MINOTAUR]) {
		    if (!rn2(3) || (in_mklev && Is_earthlevel(&u.uz)))
			(void) mongets(mtmp, WAN_DIGGING);
		} else if(monsndx(ptr) == PM_DEEPEST_ONE) {
		 if(!on_level(&rlyeh_level,&u.uz)){
		    switch (rn2(6)) {
				case 0:
					otmp = mksobj(TWO_HANDED_SWORD, TRUE, FALSE);
					otmp->oerodeproof = 1;
					otmp->spe = 3;
					(void) mpickobj(mtmp, otmp);
				break;
				case 1:
					otmp = mksobj(SCIMITAR, TRUE, FALSE);
					otmp->oerodeproof = 1;
					otmp->spe = 3;
					(void) mpickobj(mtmp, otmp);
				//Intentional fall-through for twoweaponing
				case 2:
					otmp = mksobj(SCIMITAR, TRUE, FALSE);
					otmp->oerodeproof = 1;
					otmp->spe = 3;
					(void)mpickobj(mtmp, otmp);
				break;
				case 3:
					otmp = mksobj(TRIDENT, TRUE, FALSE);
					otmp->oerodeproof = 1;
					otmp->spe = 3;
					(void) mpickobj(mtmp, otmp);
				//Intentional fall-through for twoweaponing
				case 4:
					otmp = mksobj(KNIFE, TRUE, FALSE);
					otmp->oerodeproof = 1;
					otmp->spe = 3;
					(void)mpickobj(mtmp, otmp);
				break;
			}
		 }else{
			otmp = mksobj(TRIDENT, TRUE, FALSE);
			otmp->oerodeproof = 1;
			otmp->spe = 9;
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(RANSEUR, TRUE, FALSE);
			otmp->oerodeproof = 1;
			otmp->spe = 9;
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(CROSSBOW, TRUE, FALSE);
			(void) mpickobj(mtmp, otmp);
			otmp = mksobj(CROSSBOW_BOLT, TRUE, FALSE);
			otmp->oerodeproof = 1;
			otmp->quan = 18;
			otmp->owt = weight(otmp);
			otmp->spe = 9;
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(SHIELD_OF_REFLECTION, TRUE, FALSE);
			otmp->oerodeproof = 1;
			otmp->spe = 9;
			(void) mpickobj(mtmp, otmp);
			
			(void) mongets(mtmp, rnd_attack_potion(mtmp));
			(void) mongets(mtmp, rnd_attack_potion(mtmp));
			
			otmp = mksobj(rnd_utility_potion(mtmp), TRUE, FALSE);
			otmp->blessed = FALSE;
			otmp->cursed = FALSE;
			(void) mpickobj(mtmp, otmp);
			
			otmp = mksobj(rnd_utility_potion(mtmp), TRUE, FALSE);
			otmp->blessed = FALSE;
			otmp->cursed = FALSE;
			(void) mpickobj(mtmp, otmp);
		 }
		} else if (is_giant(ptr)) {
		    for (cnt = rn2((int)(mtmp->m_lev / 2)); cnt; cnt--) {
			otmp = mksobj(rnd_class(DILITHIUM_CRYSTAL,LUCKSTONE-1),
				      FALSE, FALSE);
			otmp->quan = (long) rn1(2, 3);
			otmp->owt = weight(otmp);
			(void) mpickobj(mtmp, otmp);
		    }
			if(ptr == &mons[PM_GIANT] || ptr == &mons[PM_STONE_GIANT] || ptr == &mons[PM_HILL_GIANT]){
				if(!rn2(4)) mongets(mtmp, CLUB);
			} else if(ptr == &mons[PM_FIRE_GIANT]){
				mongets(mtmp, CLUB);
				mongets(mtmp, LEATHER_ARMOR);
			} else if(ptr == &mons[PM_FROST_GIANT]){
				mongets(mtmp, BROADSWORD);
				mongets(mtmp, HELMET);
				mongets(mtmp, SCALE_MAIL);
			} else if(ptr == &mons[PM_STORM_GIANT]){
				mongets(mtmp, TWO_HANDED_SWORD);
				mongets(mtmp, HELMET);
				mongets(mtmp, CHAIN_MAIL);
				mongets(mtmp, GLOVES);
				mongets(mtmp, IRON_SHOES);
			}
		} 
		break;
	    case S_NAGA:
			if(ptr == &mons[PM_ANCIENT_NAGA]){
				mongets(mtmp, LONG_SWORD);
				mongets(mtmp, LONG_SWORD);
				mongets(mtmp, BRONZE_PLATE_MAIL);
				mongets(mtmp, HELMET);
				mongets(mtmp, BRONZE_GAUNTLETS);
			} else if(ptr == &mons[PM_GUARDIAN_NAGA] || ptr == &mons[PM_GUARDIAN_NAGA_HATCHLING]){
				chance = rnd(10);
				if(chance >= 7){
					mongets(mtmp, BRONZE_PLATE_MAIL);
					mongets(mtmp, HELMET);
				}
			}
		break;
	    case S_WRAITH:
		if (ptr == &mons[PM_NAZGUL]) {
			otmp = mksobj(RIN_INVISIBILITY, FALSE, FALSE);
			curse(otmp);
			(void) mpickobj(mtmp, otmp);
		}
		break;
	    case S_LICH:
		if (ptr == &mons[PM_DEATH_KNIGHT]){
			(void)mongets(mtmp, RUNESWORD);
			(void)mongets(mtmp, PLATE_MAIL);
		} else if (ptr == &mons[PM_MASTER_LICH] && !rn2(13))
			(void)mongets(mtmp, (rn2(7) ? ATHAME : WAN_NOTHING));
		else if (ptr == &mons[PM_ARCH_LICH] && !rn2(3)) {
			otmp = mksobj(rn2(3) ? ATHAME : QUARTERSTAFF,
				      TRUE, rn2(13) ? FALSE : TRUE);
			if (otmp->spe < 2) otmp->spe = rnd(3);
			if (!rn2(4)) otmp->oerodeproof = 1;
			(void) mpickobj(mtmp, otmp);
		} else if(ptr == &mons[PM_ALHOON]){
			struct obj *otmp = mksobj(SKELETON_KEY, TRUE, FALSE);
			otmp = oname(otmp, artiname(ART_SECOND_KEY_OF_NEUTRALITY));
			if(!otmp->oartifact) otmp = oname(otmp, artiname(ART_THIRD_KEY_OF_NEUTRALITY));
			otmp->blessed = FALSE;
			otmp->cursed = FALSE;
			(void) mpickobj(mtmp,otmp);
		} else if(ptr == &mons[PM_ACERERAK]){
			struct obj *otmp = mksobj(ATHAME, TRUE, FALSE);
			otmp = oname(otmp, artiname(ART_PEN_OF_THE_VOID));
			otmp->blessed = FALSE;
			otmp->cursed = TRUE;
			otmp->spe = 5;
			(void) mpickobj(mtmp,otmp);
		} else if(ptr == &mons[PM_DOKKALFAR_ETERNAL_MATRIARCH]){
			/*Plate Mail*/
			otmp = mksobj(DROVEN_PLATE_MAIL, TRUE, FALSE);
			otmp->ohaluengr = TRUE;
			otmp->blessed = TRUE;
			otmp->cursed = FALSE;
			otmp->spe = 9;
			otmp->oward = curhouse;
			(void) mpickobj(mtmp, otmp);
			/*Dress*/
			otmp = mksobj(BLACK_DRESS, TRUE, FALSE);
			otmp->blessed = TRUE;
			otmp->cursed = FALSE;
			otmp->oerodeproof = TRUE;
			otmp->spe = 5;
			(void) mpickobj(mtmp, otmp);
			/*Ring*/
			otmp = mksobj(find_signet_ring(), TRUE, FALSE);
			otmp->ohaluengr = TRUE;
			otmp->oward = curhouse;
			(void) mpickobj(mtmp, otmp);
			/*Cloak*/
			otmp = mksobj(CLOAK_OF_MAGIC_RESISTANCE, TRUE, FALSE);
			otmp->blessed = TRUE;
			otmp->cursed = FALSE;
			otmp->oerodeproof = TRUE;
			otmp->spe = 5;
			(void) mpickobj(mtmp, otmp);
			/*Helm*/
			otmp = mksobj(DROVEN_HELM, TRUE, FALSE);
			otmp->blessed = TRUE;
			otmp->cursed = FALSE;
			otmp->oerodeproof = TRUE;
			otmp->spe = 5;
			(void) mpickobj(mtmp, otmp);
			/*boots*/
			otmp = mksobj(IRON_SHOES, TRUE, FALSE);
			otmp->blessed = TRUE;
			otmp->cursed = FALSE;
			otmp->oerodeproof = TRUE;
			otmp->spe = 3;
			(void) mpickobj(mtmp, otmp);
		}
		break;
	    case S_YETI:
			if(ptr == &mons[PM_GUG]){
				mongets(mtmp, CLUB);
			}
		break;
	    case S_MUMMY:
		(void)mongets(mtmp, ptr == &mons[PM_DROW_MUMMY] ? DROVEN_CLOAK : MUMMY_WRAPPING);
		if(ptr == &mons[PM_DROW_MUMMY]){
			if(!rn2(10)){
				otmp = mksobj(find_signet_ring(), FALSE, FALSE);
				otmp->ohaluengr = TRUE;
				otmp->oward = mtmp->mfaction;
				(void) mpickobj(mtmp, otmp);
			}
		}
		break;
		case S_ZOMBIE:
			if(ptr == &mons[PM_HEDROW_ZOMBIE] && !rn2(10)){
				otmp = mksobj(find_signet_ring(), FALSE, FALSE);
				otmp->ohaluengr = TRUE;
				otmp->oward = mtmp->mfaction;
				(void) mpickobj(mtmp, otmp);
			} else if(ptr == &mons[PM_HUNGRY_DEAD]){
				struct monst *blbtmp;
				blbtmp = makemon(&mons[PM_BLOB_OF_PRESERVED_ORGANS], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH);
				blbtmp->mvar1 = (long)mtmp->m_id;
			}
		break;
	    case S_QUANTMECH:
		if (!rn2(20)) {
			otmp = mksobj(BOX, FALSE, FALSE);
			otmp->spe = 1; /* flag for special box */
			otmp->owt = weight(otmp);
			(void) mpickobj(mtmp, otmp);
		}
		break;
	    case S_LEPRECHAUN:
#ifndef GOLDOBJ
		mtmp->mgold = (long) d(level_difficulty(), 30);
#else
		mkmonmoney(mtmp, (long) d(level_difficulty(), 30));
#endif
		break;
		case S_LAW_ANGEL:
		case S_NEU_ANGEL:
		case S_CHA_ANGEL:
			if(ptr == &mons[PM_WARDEN_ARCHON]){
				otmp = mksobj(PLATE_MAIL, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(HELMET, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(GAUNTLETS, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
				
				otmp = mksobj(IRON_SHOES, FALSE, FALSE);
			    bless(otmp);
			    otmp->oerodeproof = TRUE;
				otmp->objsize = MZ_LARGE;
				fix_object(otmp);
			    (void) mpickobj(mtmp, otmp);
			}
		break;
	    case S_DEMON:
	    	/* moved here from m_initweap() because these don't
		   have AT_WEAP so m_initweap() is not called for them */
			switch(monsndx(ptr)){
///////////////////////////////
		    case PM_KARY__THE_FIEND_OF_FIRE:
				mongets(mtmp, LEATHER_ARMOR);
				mongets(mtmp, LONG_SWORD);
				mongets(mtmp, SCIMITAR);
				mongets(mtmp, SCIMITAR);
				mongets(mtmp, SCIMITAR);
				mongets(mtmp, SCIMITAR);
				mongets(mtmp, SCIMITAR);
			break;
///////////////////////////////
		    case PM_CATHEZAR:
				mongets(mtmp, IRON_CHAIN);
				mongets(mtmp, IRON_CHAIN);
				mongets(mtmp, IRON_CHAIN);
				mongets(mtmp, IRON_CHAIN);
				
				mongets(mtmp, IRON_CHAIN);
				mongets(mtmp, IRON_CHAIN);
				mongets(mtmp, IRON_CHAIN);
				
				mongets(mtmp, IRON_CHAIN);
				mongets(mtmp, IRON_CHAIN);
				mongets(mtmp, IRON_CHAIN);
			break;
///////////////////////////////
			case PM_ALDINACH:
				otmp = mksobj(MASK, FALSE, FALSE);
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				otmp->corpsenm = PM_ALDINACH;
				(void) mpickobj(mtmp,otmp);
			break;
			case PM_KOSTCHTCHIE:
				otmp = mksobj(CLUB, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_WRATHFUL_WIND));
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);
				(void)mongets(mtmp, ORIHALCYON_GAUNTLETS);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_YEENOGHU:
				otmp = mksobj(FLAIL, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_THREE_HEADED_FLAIL));
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);
				(void)mongets(mtmp, GAUNTLETS_OF_DEXTERITY);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_BAPHOMET:
				otmp = mksobj(HALBERD, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_HEARTCLEAVER));
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);
				(void)mongets(mtmp, SPE_MAGIC_MAPPING);
				(void)mongets(mtmp, SCR_MAGIC_MAPPING);
				(void)mongets(mtmp, SCR_MAGIC_MAPPING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_JUIBLEX:
				// (void) mongets(mtmp, POT_FULL_HEALING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_ZUGGTMOY:
				// (void) mongets(mtmp, POT_FULL_HEALING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_ALRUNES:
				if(rn2(2)){
					(void) mongets(mtmp, HELMET);
					(void) mongets(mtmp, PLATE_MAIL);
					(void) mongets(mtmp, GAUNTLETS_OF_POWER);
					otmp = mksobj(ARMORED_BOOTS, FALSE, FALSE);
					otmp->obj_material = IRON;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					(void) mongets(mtmp, LONG_SWORD);
					(void) mongets(mtmp, KITE_SHIELD);
				} else {
					(void) mongets(mtmp, CRYSTAL_BOOTS);
					(void) mongets(mtmp, CRYSTAL_GAUNTLETS);
					(void) mongets(mtmp, CRYSTAL_PLATE_MAIL);
				}
			break;
///////////////////////////////
			case PM_ORCUS:
				otmp = mksobj(WAN_DEATH, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_WAND_OF_ORCUS));
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);
			break;
			case PM_MALCANTHET:
				(void) mongets(mtmp, BULLWHIP);
				(void) mongets(mtmp, AMULET_OF_DRAIN_RESISTANCE);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_AVATAR_OF_LOLTH:
				otmp = mksobj(VIPERWHIP, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_SCOURGE_OF_LOLTH));
				otmp->blessed = TRUE;
				otmp->cursed = FALSE;
				otmp->spe = 8;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
				/*Plate Mail*/
				otmp = mksobj(CRYSTAL_PLATE_MAIL, TRUE, FALSE);
				otmp->ohaluengr = TRUE;
				otmp->oward = LOLTH_SYMBOL;
				otmp->blessed = TRUE;
				otmp->cursed = FALSE;
				otmp->spe = 7;
				(void) mpickobj(mtmp, otmp);
				/*Gauntlets*/
				otmp = mksobj(ORIHALCYON_GAUNTLETS, TRUE, FALSE);
				otmp->blessed = TRUE;
				otmp->cursed = FALSE;
				otmp->oerodeproof = TRUE;
				otmp->spe = 5;
				(void) mpickobj(mtmp, otmp);
				/*Cloak*/
				otmp = mksobj(DROVEN_CLOAK, TRUE, FALSE);
				otmp->blessed = TRUE;
				otmp->cursed = FALSE;
				otmp->oerodeproof = TRUE;
				otmp->spe = 5;
				(void) mpickobj(mtmp, otmp);
				/*Helm*/
				otmp = mksobj(DROVEN_HELM, TRUE, FALSE);
				otmp->blessed = TRUE;
				otmp->cursed = FALSE;
				otmp->oerodeproof = TRUE;
				otmp->spe = 5;
				(void) mpickobj(mtmp, otmp);
				// (void) mongets(mtmp, POT_FULL_HEALING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_GRAZ_ZT:
				otmp = mksobj(TWO_HANDED_SWORD, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_DOOMSCREAMER));
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);
				(void) mongets(mtmp, AMULET_OF_REFLECTION);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
///////////////////////////////
			case PM_BAEL:
				otmp = mksobj(TWO_HANDED_SWORD, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_GENOCIDE));		
				otmp->spe = 9;
				curse(otmp);
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				(void) mongets(mtmp, TWO_HANDED_SWORD);
				(void) mongets(mtmp, GAUNTLETS_OF_POWER);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
		    case PM_DISPATER:
//				(void)mongets(mtmp, WAN_STRIKING);
//				(void) mongets(mtmp, MACE);
				otmp = mksobj(MACE, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_ROD_OF_DIS));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 2;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				(void) mongets(mtmp, SPE_CHARM_MONSTER);
				(void) mongets(mtmp, SCR_TAMING);
				(void) mongets(mtmp, SCR_TAMING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
		    case PM_MAMMON:
//				(void)mongets(mtmp, WAN_STRIKING);
				otmp = mksobj(SHORT_SWORD, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_AVARICE));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 3;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				(void) mongets(mtmp, POT_PARALYSIS);
				(void) mongets(mtmp, SPE_DETECT_TREASURE);
				(void) mongets(mtmp, SCR_GOLD_DETECTION);
				(void) mongets(mtmp, SCR_GOLD_DETECTION);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_FIERNA:
				(void)mongets(mtmp, WAN_DIGGING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_BELIAL:
				otmp = mksobj(TRIDENT, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_FIRE_OF_HEAVEN));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 4;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				(void)mongets(mtmp, WAN_DIGGING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
////////////////////////////////////////
			case PM_LEVIATHAN:
				otmp = mksobj(DUNCE_CAP, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_DIADEM_OF_AMNESIA));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 5;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				(void)mongets(mtmp, WAN_CANCELLATION);
				(void)mongets(mtmp, POT_AMNESIA);
				(void)mongets(mtmp, POT_AMNESIA);
				(void)mongets(mtmp, POT_AMNESIA);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_LEVISTUS:
				otmp = mksobj(RAPIER, FALSE, FALSE);
				otmp = oname(otmp, artiname(ART_SHADOWLOCK));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 4;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				(void)mongets(mtmp, RUFFLED_SHIRT);
				(void)mongets(mtmp, GENTLEMAN_S_SUIT);
				(void)mongets(mtmp, HIGH_BOOTS);
				(void)mongets(mtmp, GLOVES);
				(void)mongets(mtmp, CLOAK_OF_PROTECTION);
				(void)mongets(mtmp, WAN_CANCELLATION);
				(void)mongets(mtmp, POT_AMNESIA);
				(void)mongets(mtmp, POT_AMNESIA);
				(void)mongets(mtmp, POT_AMNESIA);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_DAUGHTER_LILITH:
				(void)mongets(mtmp, OILSKIN_CLOAK);
				otmp = mksobj(DAGGER, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_THUNDER_S_VOICE));
				otmp->obj_material = SILVER;
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 6;
				otmp->oerodeproof = TRUE;
				fix_object(otmp);
				(void) mpickobj(mtmp, otmp);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_MOTHER_LILITH:
				(void)mongets(mtmp, CLOAK_OF_MAGIC_RESISTANCE);
				(void)mongets(mtmp, OILSKIN_CLOAK);
				otmp = mksobj(ATHAME, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_SERPENT_S_TOOTH));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 6;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_CRONE_LILITH:
				(void)mongets(mtmp, BOULDER);
				(void)mongets(mtmp, BOULDER);
				(void)mongets(mtmp, BOULDER);
				(void)mongets(mtmp, QUARTERSTAFF);
				otmp = mksobj(UNICORN_HORN, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_UNBLEMISHED_SOUL));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 6;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				(void)mongets(mtmp, CRYSTAL_BALL);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_BAALZEBUB:
				otmp = mksobj(LONG_SWORD, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_WRATH_OF_HEAVEN));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 7;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);

				otmp = mksobj(HELM_OF_TELEPATHY, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_ALL_SEEING_EYE_OF_THE_FLY));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 7;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				(void)mongets(mtmp, RIN_FREE_ACTION);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_MEPHISTOPHELES:
				otmp = mksobj(RANSEUR, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_COLD_SOUL));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 8;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				(void)mongets(mtmp, HELM_OF_BRILLIANCE);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
			case PM_BAALPHEGOR:
				otmp = mksobj(QUARTERSTAFF, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_SCEPTRE_OF_THE_FROZEN_FLOO));		
				otmp->blessed = FALSE;
				otmp->cursed = TRUE;
				otmp->spe = 8;
				otmp->oerodeproof = TRUE;
				(void) mpickobj(mtmp, otmp);
				(void)mongets(mtmp, AMULET_OF_REFLECTION);
				// (void) mongets(mtmp, POT_FULL_HEALING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
/////////////////////////////////////////
			case PM_ASMODEUS:
//				(void)mongets(mtmp, WAN_COLD);
//				(void)mongets(mtmp, WAN_FIRE);
				(void)mongets(mtmp, ROBE);
				(void)mongets(mtmp, SPEED_BOOTS);
				(void)mongets(mtmp, SCR_CHARGING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
				// (void) mongets(mtmp, POT_FULL_HEALING);
			break;
////////////////////////////////////////
		    case PM_DURIN_S_BANE:
				(void)mongets(mtmp, BULLWHIP);
				otmp = mksobj(BULLWHIP, FALSE, FALSE);
				curse(otmp);
				otmp->spe = 9;
				(void) mpickobj(mtmp, otmp);
				(void)mongets(mtmp, POT_SPEED);
				(void)mongets(mtmp, POT_PARALYSIS);
				return; //bypass general weapons
			break;
////////////////////////////////////////
			case PM_CHAOS:
				mtmp->mvar2 = 0;
				{
				struct	monst *mlocal;
				/* create special stuff; can't use mongets */
				// otmp = mksobj(CRYSTAL_BALL, TRUE, FALSE);
				// otmp = oname(otmp, artiname(ART_BLACK_CRYSTAL));
				// bless(otmp);
				// otmp->oerodeproof = TRUE;
				// (void) mpickobj(mtmp, otmp);

				otmp = mksobj(SKELETON_KEY, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_THIRD_KEY_OF_CHAOS));
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);

				// mlocal = makemon(&mons[PM_KRAKEN], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				// otmp = mksobj(CRYSTAL_BALL, TRUE, FALSE);
				// otmp = oname(otmp, artiname(ART_WATER_CRYSTAL));		
				// bless(otmp);
				// otmp->oerodeproof = TRUE;
				// if(mlocal) (void) mpickobj(mlocal, otmp);

				// mlocal = makemon(&mons[PM_MARILITH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				// otmp = mksobj(CRYSTAL_BALL, TRUE, FALSE);
				// otmp = oname(otmp, artiname(ART_FIRE_CRYSTAL));		
				// bless(otmp);
				// otmp->oerodeproof = TRUE;
				// if(mlocal) (void) mpickobj(mlocal, otmp);

				// mlocal = makemon(&mons[PM_TIAMAT], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				// otmp = mksobj(CRYSTAL_BALL, TRUE, FALSE);
				// otmp = oname(otmp, artiname(ART_AIR_CRYSTAL));		
				// bless(otmp);
				// otmp->oerodeproof = TRUE;
				// if(mlocal) (void) mpickobj(mlocal, otmp);

				// mlocal = makemon(&mons[PM_LICH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				// otmp = mksobj(CRYSTAL_BALL, TRUE, FALSE);
				// otmp = oname(otmp, artiname(ART_EARTH_CRYSTAL));		
				// bless(otmp);
				// otmp->oerodeproof = TRUE;
				// if(mlocal) (void) mpickobj(mlocal, otmp);
				}
			break;
			case PM_GREAT_CTHULHU:
//				otmp = mksobj(UNIVERSAL_KEY, TRUE, FALSE);
//				otmp = oname(otmp, artiname(ART_SILVER_KEY));
//				otmp->blessed = FALSE;
//				otmp->cursed = FALSE;
//				(void) mpickobj(mtmp,otmp);
			break;

			}
		break;
	    case S_VAMPIRE:
		switch(rn2(6)) {
			case 1:
			(void)mongets(mtmp, POT_BLOOD);
			case 2:
			(void)mongets(mtmp, POT_BLOOD);
			case 3:
			(void)mongets(mtmp, POT_BLOOD);
			case 4:
			(void)mongets(mtmp, POT_BLOOD);
			default:
			break;
		}
		case S_EYE:
			if(ptr == &mons[PM_AXUS]){
				struct obj *otmp = mksobj(SKELETON_KEY, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_FIRST_KEY_OF_LAW));
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);
			}
		break;
		case S_GOLEM:
			if(ptr == &mons[PM_CENTER_OF_ALL]){
				struct obj *otmp = mksobj(BARDICHE, TRUE, FALSE);
				otmp->blessed = TRUE;
				otmp->cursed = FALSE;
				otmp->spe = 8;
				(void) mpickobj(mtmp,otmp);

				otmp = mksobj(SKELETON_KEY, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_FIRST_KEY_OF_NEUTRALITY));
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);
				return;
			}
			if(ptr == &mons[PM_ARSENAL]){
				struct obj *otmp = mksobj(SKELETON_KEY, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_SECOND_KEY_OF_LAW));
				otmp->blessed = FALSE;
				otmp->cursed = FALSE;
				(void) mpickobj(mtmp,otmp);
			}
		break;
	    default:
		break;
	}

	/* ordinary soldiers rarely have access to magic (or gold :-) */
	if (ptr == &mons[PM_SOLDIER] && rn2(13)) return;

	if ((int) mtmp->m_lev > rn2(50))
		(void) mongets(mtmp, rnd_defensive_item(mtmp));
	if ((int) mtmp->m_lev > rn2(100))
		(void) mongets(mtmp, rnd_misc_item(mtmp));
#ifndef GOLDOBJ
	if (likes_gold(ptr) && !mtmp->mgold && !rn2(5))
		mtmp->mgold =
		      (long) d(level_difficulty(), mtmp->minvent ? 5 : 10);
#else
	if (likes_gold(ptr) && !findgold(mtmp->minvent) && !rn2(5))
		mkmonmoney(mtmp, (long) d(level_difficulty(), mtmp->minvent ? 5 : 10));
#endif
}

/* Note: for long worms, always call cutworm (cutworm calls clone_mon) */
struct monst *
clone_mon(mon, x, y)
struct monst *mon;
xchar x, y;	/* clone's preferred location or 0 (near mon) */
{
	coord mm;
	struct monst *m2;

	/* may be too weak or have been extinguished for population control */
	if (mon->mhp <= 1 || (mvitals[monsndx(mon->data)].mvflags & G_EXTINCT && !In_quest(&u.uz)))
	    return (struct monst *)0;

	if (x == 0) {
	    mm.x = mon->mx;
	    mm.y = mon->my;
	    if (!enexto(&mm, mm.x, mm.y, mon->data) || MON_AT(mm.x, mm.y))
		return (struct monst *)0;
	} else if (!isok(x, y)) {
	    return (struct monst *)0;	/* paranoia */
	} else {
	    mm.x = x;
	    mm.y = y;
	    if (MON_AT(mm.x, mm.y)) {
		if (!enexto(&mm, mm.x, mm.y, mon->data) || MON_AT(mm.x, mm.y))
		    return (struct monst *)0;
	    }
	}
	m2 = newmonst(0);
	*m2 = *mon;			/* copy condition of old monster */
	m2->nmon = fmon;
	fmon = m2;
	m2->m_id = flags.ident++;
	if (!m2->m_id) m2->m_id = flags.ident++;	/* ident overflowed */
	m2->mx = mm.x;
	m2->my = mm.y;

	m2->minvent = (struct obj *) 0; /* objects don't clone */
	m2->mclone = TRUE; //no death drop
	m2->mleashed = FALSE;
#ifndef GOLDOBJ
	m2->mgold = 0L;
#endif
	/* Max HP the same, but current HP halved for both.  The caller
	 * might want to override this by halving the max HP also.
	 * When current HP is odd, the original keeps the extra point.
	 */
	m2->mhpmax = mon->mhpmax;
	m2->mhp = mon->mhp / 2;
	mon->mhp -= m2->mhp;

	/* since shopkeepers and guards will only be cloned if they've been
	 * polymorphed away from their original forms, the clone doesn't have
	 * room for the extra information.  we also don't want two shopkeepers
	 * around for the same shop.
	 */
	if (mon->isshk) m2->isshk = FALSE;
	if (mon->isgd) m2->isgd = FALSE;
	if (mon->ispriest) m2->ispriest = FALSE;
	m2->mxlth = 0;
	place_monster(m2, m2->mx, m2->my);
	if (emits_light(m2->data))
	    new_light_source(m2->mx, m2->my, emits_light(m2->data),
			     LS_MONSTER, (genericptr_t)m2);
	if (m2->mnamelth) {
	    m2->mnamelth = 0; /* or it won't get allocated */
	    m2 = christen_monst(m2, NAME(mon));
	} else if (mon->isshk) {
	    m2 = christen_monst(m2, shkname(mon));
	}

	/* not all clones caused by player are tame or peaceful */
	if (!flags.mon_moving) {
	    if (mon->mtame)
		m2->mtame = rn2(max(2 + u.uluck, 2)) ? mon->mtame : 0;
	    else if (mon->mpeaceful)
		m2->mpeaceful = rn2(max(2 + u.uluck, 2)) ? 1 : 0;
	}

	newsym(m2->mx,m2->my);	/* display the new monster */
	if (m2->mtame) {
	    struct monst *m3;

	    if (mon->isminion) {
		m3 = newmonst(sizeof(struct epri) + mon->mnamelth);
		*m3 = *m2;
		m3->mxlth = sizeof(struct epri);
		if (m2->mnamelth) Strcpy(NAME(m3), NAME(m2));
		*(EPRI(m3)) = *(EPRI(mon));
		replmon(m2, m3);
		m2 = m3;
	    } else {
		/* because m2 is a copy of mon it is tame but not init'ed.
		 * however, tamedog will not re-tame a tame dog, so m2
		 * must be made non-tame to get initialized properly.
		 */
		m2->mtame = 0;
		if ((m3 = tamedog(m2, (struct obj *)0)) != 0) {
		    m2 = m3;
		    *(EDOG(m2)) = *(EDOG(mon));
		}
	    }
	}
	set_malign(m2);

	return m2;
}

/*
 * Propagate a species
 *
 * Once a certain number of monsters are created, don't create any more
 * at random (i.e. make them extinct).  The previous (3.2) behavior was
 * to do this when a certain number had _died_, which didn't make
 * much sense.
 *
 * Returns FALSE propagation unsuccessful
 *         TRUE  propagation successful
 */
boolean
propagate(mndx, tally, ghostly)
int mndx;
boolean tally;
boolean ghostly;
{
	boolean result;
	uchar lim = mbirth_limit(mndx);
	boolean gone = (mvitals[mndx].mvflags & G_GONE && !In_quest(&u.uz)); /* genocided or extinct */

	result = (((int) mvitals[mndx].born < lim) && !gone) ? TRUE : FALSE;

	/* if it's unique, don't ever make it again */
	if (mons[mndx].geno & G_UNIQ) mvitals[mndx].mvflags |= G_EXTINCT;

	if (mvitals[mndx].born < 255 && tally && (!ghostly || (ghostly && result)))
		 mvitals[mndx].born++;
	if ((int) mvitals[mndx].born >= lim && !(mons[mndx].geno & G_NOGEN) &&
		!(mvitals[mndx].mvflags & G_EXTINCT && !In_quest(&u.uz))) {
#if defined(DEBUG) && defined(WIZARD)
		if (wizard) pline("Automatically extinguished %s.",
					makeplural(mons[mndx].mname));
#endif
		mvitals[mndx].mvflags |= G_EXTINCT;
		reset_rndmonst(mndx);
	}
	return result;
}

struct monst *
makeundead(ptr, x, y, mmflags, undeadtype)
register struct permonst *ptr;
register int	x, y;
register int	mmflags;
register int	undeadtype;
{
	struct monst *mtmp = 0;
	undeadfaction = undeadtype;
	mtmp = makemon(ptr, x, y, mmflags);
	undeadfaction = 0;
	return mtmp;
}
/*
 * called with [x,y] = coordinates;
 *	[0,0] means anyplace
 *	[u.ux,u.uy] means: near player (if !in_mklev)
 *
 *	In case we make a monster group, only return the one at [x,y].
 */
struct monst *
makemon(ptr, x, y, mmflags)
register struct permonst *ptr;
register int	x, y;
register int	mmflags;
{
	register struct monst *mtmp;
	int mndx, mcham, ct, mitem, xlth, num;
	boolean anymon = (!ptr);
	boolean byyou = (x == u.ux && y == u.uy);
	boolean allow_minvent = ((mmflags & NO_MINVENT) == 0);
	boolean countbirth = ((mmflags & MM_NOCOUNTBIRTH) == 0 && !In_quest(&u.uz));
	boolean randmonst = !ptr;
	unsigned gpflags = (mmflags & MM_IGNOREWATER) ? MM_IGNOREWATER : 0;
	boolean unsethouse = FALSE;
	
	if(Race_if(PM_DROW) && in_mklev && Is_qstart(&u.uz) && 
		(ptr == &mons[PM_SPROW] || ptr == &mons[PM_DRIDER] || ptr == &mons[PM_CAVE_LIZARD] || ptr == &mons[PM_LARGE_CAVE_LIZARD])
	)
		mmflags |= MM_EDOG;

	/* if caller both a random creature and a random location, try both at once first */
	if(!ptr && x == 0 && y == 0){
		int tryct = 0;	/* careful with bigrooms */
		struct monst fakemon = {0};
		do{
			x = rn1(COLNO-3,2);
			y = rn2(ROWNO);
			ptr = rndmonst();
			if(!ptr) {
	#ifdef DEBUG
				pline("Warning: no monster.");
	#endif
				return((struct monst *) 0);	/* no more monsters! */
			}
			fakemon.data = ptr;
			gpflags = (mmflags & MM_IGNOREWATER) ? MM_IGNOREWATER : 0;
		} while((!goodpos(x, y, &fakemon, gpflags) || (!in_mklev && cansee(x, y))) && tryct++ < 200);
		if(tryct >= 200){
			//That failed, return to the default way of handling things
			ptr = (struct permonst *)0;
			x = y = 0;
		} else if(PM_ARCHEOLOGIST <= monsndx(ptr) && monsndx(ptr) <= PM_WIZARD && !(mmflags & MM_EDOG)){
			return mk_mplayer(ptr, x, y, FALSE);
		}
	}
	
	/* if caller wants random location, do it here */
	if(x == 0 && y == 0) {
		int tryct = 0;	/* careful with bigrooms */
		struct monst fakemon = {0};

		fakemon.data = ptr;	/* set up for goodpos */
		do {
			x = rn1(COLNO-3,2);
			y = rn2(ROWNO);
		} while(!goodpos(x, y, ptr ? &fakemon : (struct monst *)0, Is_waterlevel(&u.uz) ? gpflags|MM_IGNOREWATER : gpflags) &&
			(tryct++ < 150 && ((tryct < 50 && couldsee(x, y)) || ((tryct < 100 && cansee(x, y))) || distmin(x,y,u.ux,u.uy) < BOLT_LIM)));
		if(tryct >= 150){
			return((struct monst *)0);
		}
		if(ptr && PM_ARCHEOLOGIST <= monsndx(ptr) && monsndx(ptr) <= PM_WIZARD && !(mmflags & MM_EDOG)){
			return mk_mplayer(ptr, x, y, FALSE);
		}
	} else if (byyou && !in_mklev) {
		coord bypos;

		if(enexto_core(&bypos, u.ux, u.uy, ptr, gpflags)) {
			x = bypos.x;
			y = bypos.y;
		} else
			return((struct monst *)0);
	}
	/* Does monster already exist at the position? */
	if(MON_AT(x, y)) {
		if ((mmflags & MM_ADJACENTOK) != 0) {
			coord bypos;
			if(enexto_core(&bypos, x, y, ptr, gpflags)) {
				if( !(mmflags & MM_ADJACENTSTRICT) || (
					bypos.x - x <= 1 && bypos.x - x >= -1 &&
					bypos.y - y <= 1 && bypos.y - y >= -1
				)){
					x = bypos.x;
					y = bypos.y;
				}
				else return((struct monst *) 0);
			} else
				return((struct monst *) 0);
		} else 
			return((struct monst *) 0);
	}
	
	if(ptr && mmflags & MM_CHECK_GOODPOS){
	 struct monst fakemon = {0};
	 fakemon.data = ptr;	/* set up for goodpos */
	 if(!goodpos(x, y, &fakemon, gpflags)){
		if ((mmflags & MM_ADJACENTOK) != 0) {
			coord bypos;
			if(enexto_core(&bypos, x, y, ptr, gpflags)) {
				if( !(mmflags & MM_ADJACENTSTRICT) || (
					bypos.x - x <= 1 && bypos.x - x >= -1 &&
					bypos.y - y <= 1 && bypos.y - y >= -1
				)){
					x = bypos.x;
					y = bypos.y;
				}
				else return((struct monst *) 0);
			} else
				return((struct monst *) 0);
		} else 
			return((struct monst *) 0);
	 }
	}
	
	if(ptr){
		mndx = monsndx(ptr);
		/* if you are to make a specific monster and it has
		   already been genocided, return */
		if (mvitals[mndx].mvflags & G_GENOD && !In_quest(&u.uz)) return((struct monst *) 0);
#if defined(WIZARD) && defined(DEBUG)
		if (wizard && (mvitals[mndx].mvflags & G_EXTINCT && !In_quest(&u.uz)))
		    pline("Explicitly creating extinct monster %s.",
			mons[mndx].mname);
#endif
	} else {
		/* make a random (common) monster that can survive here.
		 * (the special levels ask for random monsters at specific
		 * positions, causing mass drowning on the medusa level,
		 * for instance.)
		 */
		int tryct = 0;	/* maybe there are no good choices */
		struct monst fakemon = {0};
		do {
			if(!(ptr = rndmonst())) {
#ifdef DEBUG
			    pline("Warning: no monster.");
#endif
			    return((struct monst *) 0);	/* no more monsters! */
			}
			fakemon.data = ptr;	/* set up for goodpos */
		} while(!goodpos(x, y, &fakemon, gpflags) && tryct++ < 150);
		if(tryct >= 150){
			return((struct monst *) 0);	/* no more monsters! */
		}
		
		mndx = monsndx(ptr);
	}
	if(allow_minvent) allow_minvent = !(mons[mndx].maligntyp < 0 && Is_illregrd(&u.uz));
	(void) propagate(mndx, countbirth, FALSE);
	xlth = ptr->pxlth;
	if (mmflags & MM_EDOG) xlth += sizeof(struct edog);
	else if (mmflags & MM_EMIN) xlth += sizeof(struct emin);
	mtmp = newmonst(xlth);
	*mtmp = zeromonst;		/* clear all entries in structure */
	(void)memset((genericptr_t)mtmp->mextra, 0, xlth);
	mtmp->nmon = fmon;
	fmon = mtmp;
	mtmp->m_id = flags.ident++;
	if (!mtmp->m_id) mtmp->m_id = flags.ident++;	/* ident overflowed */
	mtmp->mcansee = mtmp->mcanhear = mtmp->mcanmove = mtmp->mnotlaugh = TRUE;
	mtmp->mblinded = mtmp->mfrozen = mtmp->mlaughing = 0;
	mtmp->mvar1 = mtmp->mvar2 = mtmp->mvar3 = 0;
	set_mon_data(mtmp, ptr, 0);
	
	mtmp->mstr = d(3,6);
	if(strongmonst(mtmp->data)) mtmp->mstr += 10;
	mtmp->mdex = d(3,6);
	if(is_elf(mtmp->data) && mtmp->mstr > mtmp->mdex){
		short swap = mtmp->mstr;
		mtmp->mstr = mtmp->mdex;
		mtmp->mdex = swap;
	}
	mtmp->mcon = d(3,6);
	if(is_animal(mtmp->data)) mtmp->mint = 3;
	else if(mindless_mon(mtmp)) mtmp->mint = 0;
	else if(is_magical(mtmp->data)) mtmp->mint = 13+rnd(5);
	else mtmp->mint = d(3,6);
	mtmp->mwis = d(3,6);
	mtmp->mcha = d(3,6);
	if(mtmp->data->mlet == S_NYMPH){
		if(mtmp->data == &mons[PM_DEMINYMPH]) mtmp->mcha = (mtmp->mcha + 25)/2;
		else mtmp->mcha = 25;
	}
	
	if(Race_if(PM_DROW) && in_mklev && Is_qstart(&u.uz) && (ptr == &mons[PM_SPROW] || ptr == &mons[PM_DRIDER] || ptr == &mons[PM_CAVE_LIZARD] || ptr == &mons[PM_LARGE_CAVE_LIZARD])){
		struct obj *otmp = mksobj(SADDLE, TRUE, FALSE);
		initedog(mtmp);
		EDOG(mtmp)->loyal = TRUE;
		if (otmp) {
			if (mpickobj(mtmp, otmp)) panic("merged saddle?");
			mtmp->misc_worn_check |= W_SADDLE;
			otmp->dknown = otmp->bknown = otmp->rknown = 1;
			otmp->owornmask = W_SADDLE;
			otmp->leashmon = mtmp->m_id;
			update_mon_intrinsics(mtmp, otmp, TRUE, TRUE);
		}
		if(ptr == &mons[PM_SPROW] || ptr == &mons[PM_DRIDER]){
			otmp = mksobj(DROVEN_PLATE_MAIL, TRUE, FALSE);
			otmp->oward = (long)u.start_house;
			otmp->oerodeproof = TRUE;
			otmp->blessed = FALSE;
			otmp->cursed = TRUE;
			otmp->spe = 3;
			(void) mpickobj(mtmp,otmp);
			otmp = mksobj(DROVEN_HELM, TRUE, FALSE);
			otmp->oerodeproof = TRUE;
			otmp->blessed = FALSE;
			otmp->cursed = TRUE;
			otmp->spe = 3;
			(void) mpickobj(mtmp,otmp);
			otmp = mksobj(DROVEN_CLOAK, TRUE, FALSE);
			otmp->oerodeproof = TRUE;
			otmp->blessed = FALSE;
			otmp->cursed = TRUE;
			otmp->spe = 3;
			(void) mpickobj(mtmp,otmp);
		}
	}
	
	if (ptr == &mons[urole.ldrnum])
	    quest_status.leader_m_id = mtmp->m_id;
	mtmp->mxlth = xlth;
	mtmp->mnum = mndx;
	mtmp->m_lev = adj_lev(ptr);
	if(mtmp->data == &mons[PM_CHOKHMAH_SEPHIRAH])
		mtmp->m_lev += u.chokhmah;
	if (is_golem(ptr)) {
	    mtmp->mhpmax = mtmp->mhp = golemhp(mndx);
	} else if (is_rider(ptr)) {
	    /* We want low HP, but a high mlevel so they can attack well */
	    mtmp->mhpmax = mtmp->mhp = d(10,8);
	} else if (ptr->mlevel > 49) {
	    /* "special" fixed hp monster
	     * the hit points are encoded in the mlevel in a somewhat strange
	     * way to fit in the 50..127 positive range of a signed character
	     * above the 1..49 that indicate "normal" monster levels */
//	    mtmp->mhpmax = mtmp->mhp = 2*(ptr->mlevel - 6);
	    mtmp->mhpmax = mtmp->mhp = 8*(ptr->mlevel);
	    // mtmp->m_lev = mtmp->mhp / 4;	/* approximation */
	} else if (ptr->mlet == S_DRAGON && mndx >= PM_GRAY_DRAGON) {
	    /* adult dragons */
	    mtmp->mhpmax = mtmp->mhp = (int) (In_endgame(&u.uz) ?
		(8 * mtmp->m_lev) : (4 * mtmp->m_lev + d((int)mtmp->m_lev, 4)));
	} else if (!mtmp->m_lev) {
	    mtmp->mhpmax = mtmp->mhp = rnd(4);
	} else {
	    mtmp->mhpmax = mtmp->mhp = d((int)mtmp->m_lev, 8);
	    if (is_home_elemental(ptr))
		mtmp->mhpmax = (mtmp->mhp *= 3);
	}

	if (is_female(ptr)) mtmp->female = TRUE;
	else if (is_male(ptr)) mtmp->female = FALSE;
	else mtmp->female = rn2(2);	/* ignored for neuters */

	// if (ptr == &mons[urole.ldrnum])		/* leader knows about portal */
	    // mtmp->mtrapseen |= (1L << (MAGIC_PORTAL-1));
	// if (ptr == &mons[PM_OONA])  /* don't trigger statue traps */
		// mtmp->mtrapseen |= (1L << (STATUE_TRAP-1));
	if (In_sokoban(&u.uz) && !mindless(ptr))  /* know about traps here */
	    mtmp->mtrapseen = (1L << (PIT - 1)) | (1L << (HOLE - 1));
	if (In_endgame(&u.uz))  /* know about fire traps here */
	    mtmp->mtrapseen = (1L << (PIT - 1)) | (1L << (FIRE_TRAP - 1) | (1L << (MAGIC_PORTAL-1)));
	if (ptr == &mons[urole.ldrnum] || ptr == &mons[urole.guardnum])		/* leader and guards knows about all traps */
	    mtmp->mtrapseen |= ~0;
	mtmp->mtrapseen |= (1L << (STATUE_TRAP-1)); /* all monsters should avoid statue traps */

	place_monster(mtmp, x, y);
	mtmp->mpeaceful  = FALSE;
	mtmp->mtraitor  = FALSE;
	mtmp->mferal  = FALSE;
	mtmp->mcrazed  = FALSE;
	mtmp->mclone  = FALSE;
	mtmp->mvanishes  = -1;

	mtmp->mspec_used = 3;
	mtmp->encouraged = 0;
	/* Ok, here's the deal: I'm using a global to coordinate the house emblems on the drow's armor. 
	   It needs to be set up here so that everyone created as part of the group gets the same emblem, 
	   and then unset after this creature's armor is created. */
	if(is_drow(ptr)){
		if(!curhouse){
			if((ptr == &mons[urole.ldrnum] && ptr != &mons[PM_ECLAVDRA]) || 
				(ptr == &mons[urole.guardnum] && ptr != &mons[PM_DROW_MATRON_MOTHER] && ptr != &mons[PM_HEDROW_MASTER_WIZARD])
			){
				if(Race_if(PM_DROW) && !Role_if(PM_EXILE)) curhouse = u.start_house;
				else curhouse = LOLTH_SYMBOL;
			} else if(Is_lolth_level(&u.uz)){
				curhouse = LOLTH_SYMBOL;
			} else if(ptr == &mons[PM_MINDLESS_THRALL] || ptr == &mons[PM_A_GONE] || ptr == &mons[PM_PEASANT]){
				curhouse = PEN_A_SYMBOL;
			} else if(ptr == &mons[PM_DOKKALFAR_ETERNAL_MATRIARCH]){
				curhouse = LOST_HOUSE;
			} else if(ptr == &mons[PM_ECLAVDRA] || ptr == &mons[PM_AVATAR_OF_LOLTH] || is_yochlol(ptr)){
				curhouse = LOLTH_SYMBOL;
			} else if(ptr == &mons[PM_DROW_MATRON_MOTHER]){
				if(Race_if(PM_DROW) && !Role_if(PM_EXILE)) curhouse = (Role_if(PM_NOBLEMAN) && !flags.initgend) ? (((u.start_house - FIRST_FALLEN_HOUSE)+FIRST_HOUSE)%(LAST_HOUSE-FIRST_HOUSE)) : LOLTH_SYMBOL;
				else curhouse = LOLTH_SYMBOL;
			} else if(ptr == &mons[PM_SEYLL_AUZKOVYN] || ptr == &mons[PM_STJARNA_ALFR]){
				curhouse = EILISTRAEE_SYMBOL;
			} else if(ptr == &mons[PM_PRIESTESS_OF_GHAUNADAUR]){
				curhouse = GHAUNADAUR_SYMBOL;
			} else if(ptr == &mons[PM_DARUTH_XAXOX] || ptr == &mons[PM_DROW_ALIENIST]){
				curhouse = XAXOX;
			} else if(ptr == &mons[PM_EMBRACED_DROWESS] || (ptr == &mons[PM_DROW_MUMMY] && In_quest(&u.uz) && !flags.initgend)){
				curhouse = EDDER_SYMBOL;
			} else if(ptr == &mons[PM_A_SALOM]){
				curhouse = VER_TAS_SYMBOL;
			} else if(ptr == &mons[PM_GROMPH]){
				curhouse = SORCERE;
			} else if(ptr == &mons[PM_DANTRAG]){
				curhouse = MAGTHERE;
			} else if(ptr == &mons[PM_HEDROW_BLADEMASTER]){
				curhouse = MAGTHERE;
			} else if(ptr == &mons[PM_HEDROW_MASTER_WIZARD]){
				curhouse = SORCERE;
			} else if(ptr->mlet != S_HUMAN && !((ptr == &mons[PM_SPROW] || ptr == &mons[PM_DRIDER]) && in_mklev && In_quest(&u.uz) && Is_qstart(&u.uz))){
				if(ptr == &mons[PM_DROW_MUMMY]){
					if(!(rn2(10))){
						if(!rn2(6)) curhouse = LOLTH_SYMBOL;
						else if(rn2(5) < 2) curhouse = EILISTRAEE_SYMBOL;
						else curhouse = KIARANSALEE_SYMBOL;
					} else if(!(rn2(4))) curhouse = rn2(LAST_HOUSE+1-FIRST_HOUSE)+FIRST_HOUSE;
					else curhouse = rn2(LAST_FALLEN_HOUSE+1-FIRST_FALLEN_HOUSE)+FIRST_FALLEN_HOUSE;
				}
				else if(ptr == &mons[PM_HEDROW_ZOMBIE]){
					if(!rn2(6)) curhouse = SORCERE;
					else if(!rn2(5)) curhouse = MAGTHERE;
					else if(!(rn2(4))) curhouse = rn2(LAST_HOUSE+1-FIRST_HOUSE)+FIRST_HOUSE;
					else curhouse = rn2(LAST_FALLEN_HOUSE+1-FIRST_FALLEN_HOUSE)+FIRST_FALLEN_HOUSE;
				}
				else curhouse = rn2(LAST_FALLEN_HOUSE+1-FIRST_FALLEN_HOUSE)+FIRST_FALLEN_HOUSE;
			} else if(In_quest(&u.uz)){
				if(Race_if(PM_DROW) && Role_if(PM_EXILE)){
					curhouse = PEN_A_SYMBOL;
				} else if(Role_if(PM_ANACHRONONAUT)){
					curhouse = LAST_BASTION_SYMBOL;
				} else if((Race_if(PM_DROW)) && (in_mklev || flags.stag || rn2(3))){
					if(Is_qstart(&u.uz)) curhouse = u.start_house;
					else if(Role_if(PM_NOBLEMAN)){
						if(flags.initgend) curhouse = u.start_house;
						else curhouse = (((u.start_house - FIRST_FALLEN_HOUSE)+FIRST_HOUSE)%(LAST_HOUSE-FIRST_HOUSE))+FIRST_HOUSE;
					} else if((&u.uz)->dlevel >= qlocate_level.dlevel){
						curhouse = rn2(2) ? u.start_house : flags.initgend ? EILISTRAEE_SYMBOL : EDDER_SYMBOL;
					} else {
						curhouse = flags.initgend ? EILISTRAEE_SYMBOL : EDDER_SYMBOL;
					}
				} else curhouse = rn2(LAST_HOUSE+1-FIRST_HOUSE)+FIRST_HOUSE;
			} else {
				curhouse = rn2(LAST_HOUSE+1-FIRST_HOUSE)+FIRST_HOUSE;
			}
			unsethouse = TRUE;
		}
		mtmp->mfaction = curhouse;
	} else if(!undeadfaction && ((zombiepm >=0 && &mons[zombiepm] == mtmp->data) || (skeletpm >=0 && &mons[skeletpm] == mtmp->data))){
		if(zombiepm >=0 && &mons[zombiepm] == mtmp->data){
			undeadfaction = ZOMBIFIED;
			unsethouse = TRUE;
			zombiepm = -1;
		} else {
			undeadfaction = SKELIFIED;
			unsethouse = TRUE;
			skeletpm = -1;
		}
	} else if(In_quest(&u.uz) && urole.neminum == PM_NECROMANCER && mtmp->data == &mons[PM_ELF]){
		undeadfaction = ZOMBIFIED;
		unsethouse = TRUE;
		m_initlgrp(mtmp, mtmp->mx, mtmp->my);
	} else if(mtmp->data == &mons[PM_ECHO]){
		undeadfaction = SKELIFIED;
		unsethouse = TRUE;
	} else if(!undeadfaction && (mtmp->data->geno&G_HELL) == 0 && Is_mephisto_level(&u.uz)){
		undeadfaction = CRYSTALFIED;
		unsethouse = TRUE;
	} else if(is_kamerel(mtmp->data)){
		if(level.flags.has_kamerel_towers && (mtmp->data != &mons[PM_ARA_KAMEREL] || rn2(2))){
			undeadfaction = FRACTURED;
			unsethouse = TRUE;
		} else if(!level.flags.has_minor_spire && mtmp->data != &mons[PM_ARA_KAMEREL]
			&& (mtmp->data != &mons[PM_HUDOR_KAMEREL] || rn2(2))
			&& (mtmp->data != &mons[PM_SHARAB_KAMEREL] || !rn2(4))
		){
			undeadfaction = FRACTURED;
			unsethouse = TRUE;
		}
	} else if(randmonst && !undeadfaction && can_undead_mon(mtmp)){
		if(In_mines(&u.uz)){
			if(Race_if(PM_GNOME) && Role_if(PM_RANGER) && rn2(10) <= 5){
				undeadfaction = ZOMBIFIED;
				unsethouse = TRUE;
				m_initlgrp(mtmp, mtmp->mx, mtmp->my);
			} else if(!rn2(10)){
				undeadfaction = ZOMBIFIED;
				unsethouse = TRUE;
				m_initlgrp(mtmp, mtmp->mx, mtmp->my);
			}
		} else if(!rn2(100)){
			undeadfaction = ZOMBIFIED;
			unsethouse = TRUE;
			m_initlgrp(mtmp, mtmp->mx, mtmp->my);
		}
	}
	if(undeadfaction){
		mtmp->mfaction = undeadfaction;
		if(undeadfaction == FRACTURED){
			mtmp->m_lev += 4;
			mtmp->mhpmax = d(mtmp->m_lev, 8);
			mtmp->mhp = mtmp->mhpmax;
		}
		newsym(mtmp->mx,mtmp->my);
		allow_minvent = rn2(2);
	}
	
	if(Race_if(PM_DROW) && in_mklev && Is_qstart(&u.uz) && 
		(ptr == &mons[PM_SPROW] || ptr == &mons[PM_DRIDER] || ptr == &mons[PM_CAVE_LIZARD] || ptr == &mons[PM_LARGE_CAVE_LIZARD])
	) mtmp->mpeaceful = TRUE;
	else mtmp->mpeaceful = (mmflags & MM_ANGRY) ? FALSE : (peace_minded(ptr) && !is_derived_undead_mon(mtmp));
	
	if(mndx == PM_CHAOS){
		mtmp->mhpmax = 15*mtmp->mhpmax;
		mtmp->mhp = mtmp->mhpmax;
	} else if(mndx == PM_KARY__THE_FIEND_OF_FIRE){
		mtmp->mhpmax = 10*mtmp->mhpmax;
		mtmp->mhp = mtmp->mhpmax;
	} else if(mndx == PM_LICH__THE_FIEND_OF_EARTH){
		mtmp->mhpmax = 10*mtmp->mhpmax;
		mtmp->mhp = mtmp->mhpmax;
	} else if(mndx == PM_KRAKEN__THE_FIEND_OF_WATER){
		mtmp->mhpmax = 10*mtmp->mhpmax;
		mtmp->mhp = mtmp->mhpmax;
	} else if(mndx == PM_TIAMAT__THE_FIEND_OF_WIND){
		mtmp->mhpmax = 10*mtmp->mhpmax;
		mtmp->mhp = mtmp->mhpmax;
	}
	
	switch(ptr->mlet) {
		case S_MIMIC:
			set_mimic_sym(mtmp);
			break;
		case S_QUADRUPED:
			if(mtmp->data == &mons[PM_DARK_YOUNG]) set_mimic_sym(mtmp);
			break;
		case S_SPIDER:
		case S_SNAKE:
			if(in_mklev)
			    if(x && y)
				(void) mkobj_at(0, x, y, TRUE);
			if(hides_under(ptr) && OBJ_AT(x, y))
			    mtmp->mundetected = TRUE;
		break;
		case S_LIGHT:
		case S_ELEMENTAL:
			if (mndx == PM_STALKER || mndx == PM_BLACK_LIGHT) {
			    mtmp->perminvis = TRUE;
			    mtmp->minvis = TRUE;
			}
		break;
		case S_EYE:
			if(!(mmflags & MM_EDOG)){
			if (mndx == PM_QUINON){
				mtmp->movement = d(1,6);
				if(anymon){
					makemon(&mons[PM_TRITON], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH);
					for(num = rnd(6); num >= 0; num--) makemon(&mons[PM_DUTON], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH);
					m_initlgrp(makemon(&mons[PM_MONOTON], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH), mtmp->mx, mtmp->my);
				}
				makemon(&mons[PM_QUATON], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH);
			} else if (mndx == PM_QUATON){
				mtmp->movement = d(1,7);
				if(anymon){
					for(num = rn1(6,5); num >= 0; num--) makemon(&mons[PM_MONOTON], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH);
				}
				makemon(&mons[PM_TRITON], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH);
			} else if (mndx == PM_TRITON){
				mtmp->movement = d(1,8);
				if(anymon) m_initlgrp(makemon(&mons[PM_MONOTON], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH), mtmp->mx, mtmp->my);
				makemon(&mons[PM_DUTON], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH);
			} else if (mndx == PM_DUTON){
				mtmp->movement = d(1,9);
				if(anymon && rn2(2)) m_initsgrp(makemon(&mons[PM_MONOTON], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH), mtmp->mx, mtmp->my);
				else makemon(&mons[PM_MONOTON], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH);
			} else if (mndx == PM_MONOTON){
				mtmp->movement = d(1,10);
			} else if (mndx == PM_BEHOLDER){
				if(anymon) m_initsgrp(makemon(&mons[PM_GAS_SPORE], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH), mtmp->mx, mtmp->my);
			}
			}
/*			if(mndx == PM_VORLON_MISSILE){
				mtmp->mhpmax = 3;
				mtmp->mhp = 3;
			}
*/		break;
		case S_IMP:
			if(!(mmflags & MM_EDOG)){
			if (anymon && mndx == PM_LEGION_DEVIL_SERGEANT){
				for(num = 10; num > 0; num--) makemon(&mons[PM_LEGION_DEVIL_GRUNT], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				for(num = 3; num > 0; num--) makemon(&mons[PM_LEGION_DEVIL_SOLDIER], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			} else if (anymon && mndx == PM_LEGION_DEVIL_CAPTAIN){
				for(num = 20; num > 0; num--) makemon(&mons[PM_LEGION_DEVIL_GRUNT], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				for(num = 8; num > 0; num--) makemon(&mons[PM_LEGION_DEVIL_SOLDIER], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				for(num = 2; num > 0; num--) makemon(&mons[PM_LEGION_DEVIL_SERGEANT], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			}
			}
		break;
		case S_LAW_ANGEL:
		case S_NEU_ANGEL:
		case S_CHA_ANGEL:
			if(is_weeping(mtmp->data)){
				mtmp->mvar1 = 0;
				mtmp->mvar2 = 0;
				mtmp->mvar3 = 0;
				if (anymon){
					if(u.uevent.udemigod) m_initlgrp(mtmp, 0, 0);
					else mtmp->mvar3 = 1; //Set to 1 to initiallize
				}
			} else if(mtmp->data == &mons[PM_ARCADIAN_AVENGER]){
				if(anymon) m_initsgrp(makemon(&mons[PM_ARCADIAN_AVENGER], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_NOCOUNTBIRTH), mtmp->mx, mtmp->my);
			} else if(mndx == PM_KETO){ 
				mtmp->mhpmax = 3*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
		break;
	    case S_GIANT:
			if(!(mmflags & MM_EDOG)){
			if (anymon && mndx == PM_DEEPEST_ONE){
				for(num = rn1(3,3); num >= 0; num--) makemon(&mons[PM_DEEPER_ONE], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				for(num = rn1(10,10); num >= 0; num--) makemon(&mons[PM_DEEP_ONE], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			}
			}
		break;
		case S_HUMAN:
			if(!(mmflags & MM_EDOG)){
			if(anymon){
				if (mndx == PM_DROW_MATRON){
					m_initlgrp(makemon(&mons[PM_HEDROW_WARRIOR], mtmp->mx, mtmp->my, MM_ADJACENTOK), mtmp->mx, mtmp->my);
				} else if (mndx == PM_DOKKALFAR_ETERNAL_MATRIARCH){
					m_initsgrp(makemon(&mons[PM_DROW_MATRON], mtmp->mx, mtmp->my, MM_ADJACENTOK), mtmp->mx, mtmp->my);
					m_initlgrp(makemon(&mons[PM_HEDROW_WARRIOR], mtmp->mx, mtmp->my, MM_ADJACENTOK), mtmp->mx, mtmp->my);
					m_initlgrp(makemon(&mons[PM_DROW_MUMMY], mtmp->mx, mtmp->my, MM_ADJACENTOK), mtmp->mx, mtmp->my);
					m_initlgrp(makemon(&mons[PM_HEDROW_ZOMBIE], mtmp->mx, mtmp->my, MM_ADJACENTOK), mtmp->mx, mtmp->my);
				} else if (mndx == PM_ELVENKING || mndx == PM_ELVENQUEEN){
					for(num = rnd(2); num >= 0; num--) makemon(&mons[rn2(2) ? PM_ELF_LORD : PM_ELF_LADY], mtmp->mx, mtmp->my, MM_ADJACENTOK);
					for(num = rn1(6,3); num >= 0; num--) makemon(&mons[PM_GREY_ELF], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				} else if (mndx == PM_CHIROPTERAN){
					if(!rn2(3)) m_initlgrp(makemon(&mons[PM_WARBAT], mtmp->mx, mtmp->my, MM_ADJACENTOK), mtmp->mx, mtmp->my);
					m_initlgrp(makemon(&mons[PM_BATTLE_BAT], mtmp->mx, mtmp->my, MM_ADJACENTOK), mtmp->mx, mtmp->my);
				}
			}
			}
		break;
		case S_HUMANOID:
			if(!(mmflags & MM_EDOG)){
			if(anymon){
				if (mndx == PM_DEEPER_ONE){
					for(num = rn1(10,3); num >= 0; num--) makemon(&mons[PM_DEEP_ONE], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				}
			}
			}
		break;
		case S_FUNGUS:
			if(!(mmflags & MM_EDOG)){
			if (anymon && mndx == PM_MIGO_QUEEN){
				for(num = rn2(2)+1; num >= 0; num--) makemon(&mons[PM_MIGO_PHILOSOPHER], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				for(num = rn2(3)+3; num >= 0; num--) makemon(&mons[PM_MIGO_SOLDIER], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				for(num = rn2(5)+5; num >= 0; num--) makemon(&mons[PM_MIGO_WORKER], mtmp->mx, mtmp->my, MM_ADJACENTOK);

			}
			}
			if (mndx == PM_PHANTOM_FUNGUS) {
			    mtmp->perminvis = TRUE;
			    mtmp->minvis = TRUE;
			}
		break;
		case S_GNOME:
			if(mndx == PM_CLOCKWORK_SOLDIER || mndx == PM_CLOCKWORK_DWARF || 
			   mndx == PM_FABERGE_SPHERE || mndx == PM_FIREWORK_CART || 
			   mndx == PM_JUGGERNAUT || mndx == PM_ID_JUGGERNAUT
			) mtmp->mvar1 = rn2(8);
			
			if(mndx == PM_ID_JUGGERNAUT) {
				mtmp->perminvis = TRUE;
				mtmp->minvis = TRUE;
			}
		break;
		case S_WRAITH:
			if (mndx == PM_PHANTASM){
			    mtmp->mhpmax = mtmp->mhp = d(1,10);
				if(rn2(2)) {
				    mtmp->perminvis = TRUE;
				    mtmp->minvis = TRUE;
				}
				if(!rn2(3)){
					mtmp->mspeed = MFAST;
					mtmp->permspeed = MFAST;
				}
				else if(rn2(2)){
					mtmp->mspeed = MSLOW;
					mtmp->permspeed = MSLOW;
				}
				else{
					mtmp->mspeed = 0;
					mtmp->permspeed = 0;
				}
				if(rn2(2)){
					mtmp->mflee = 1;
					mtmp->mfleetim = d(1,10);
				}
				if(rn2(2)){
					if(rn2(4)){
						mtmp->mcansee = 0;
						mtmp->mblinded = rn2(30);
					}
					else{
						mtmp->mcansee = 0;
						mtmp->mblinded = 0;
					}
				}
				if(rn2(2)){
					if(rn2(2)){
						mtmp->mconf = 1;
					}else if(rn2(2)){
						mtmp->mstun = 1;
					}else{
						mtmp->mcrazed = 1;
					}
				}
				if(rn2(2)){
					if(rn2(4)){
						mtmp->mcanmove = 0;
						mtmp->mfrozen = d(1,10);
					}else{
						mtmp->mcanmove = 0;
						mtmp->mfrozen = 0;
					}
				}
				if(!rn2(9)){
					mtmp->mnotlaugh = 0;
					// You_hear("soft laughter.");
					mtmp->mlaughing = d(2,4);
				}
				if(!rn2(8)){
					mtmp->msleeping = 1;
				}
				if(!rn2(8)){
					mtmp->mpeaceful = 1;
				}
			}
		break;
		case S_TRAPPER:
			if(!(mmflags & MM_EDOG)){
				if(mndx==PM_METROID_QUEEN) 
					if(anymon) for(num = 6; num >= 0; num--) makemon(&mons[PM_METROID], mtmp->mx, mtmp->my, MM_ADJACENTOK);
					// if(anymon) for(num = 6; num >= 0; num--) makemon(&mons[PM_BABY_METROID], mtmp->mx, mtmp->my, MM_ADJACENTOK);
					// else for(num = 6; num >= 0; num--) makemon(&mons[PM_METROID], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			}
		break;
//		case S_VAMPIRE:
//			{
//				if(mndx == PM_STAR_VAMPIRE){
//				    mtmp->minvis = TRUE;
//				    mtmp->perminvis = TRUE;
//				}
//			}
//		break;
		case S_BLOB:
			if (mndx == PM_SHOGGOTH || mndx == PM_PRIEST_OF_GHAUNADAUR){
				mtmp->mhpmax = 3*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
		break;
	    case S_KETER:
			if (mndx == PM_CHOKHMAH_SEPHIRAH){
				mtmp->mhpmax = 1+u.chokhmah*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
			if(!(mmflags & MM_EDOG)){
			if(mndx != PM_MALKUTH_SEPHIRAH && mndx != PM_DAAT_SEPHIRAH && mndx != PM_BINAH_SEPHIRAH){
				coord mm;
				mm.x = xdnstair;
				mm.y = ydnstair;
				makeketer(&mm);
				
				mm.x = xupstair;
				mm.y = yupstair;
				makeketer(&mm);
			}
			}
		break;
		case S_EEL:
			if (is_pool(x, y, FALSE))
			    mtmp->mundetected = TRUE;
			if(mndx == PM_WATCHER_IN_THE_WATER){ 
				mtmp->mhpmax = 3*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
				mtmp->mcanmove = 0;
				mtmp->mfrozen = 3;
			}
		break;
		case S_LEPRECHAUN:
			mtmp->msleeping = 1;
			break;
		case S_JABBERWOCK:
		case S_NYMPH:
			if (rn2(5) && !u.uhave.amulet 
				&& mndx != PM_NIMUNE && mndx != PM_INTONER && mndx != PM_AGLAOPE 
				&& !(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz))
			){
				mtmp->msleeping = 1;
			}
		break;
		case S_ORC:
			if (Race_if(PM_ELF)) mtmp->mpeaceful = FALSE;
			else if(mndx == PM_BOLG){
				mtmp->mhpmax = 3*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
			if(!(mmflags & MM_EDOG)){
			if (anymon && mndx == PM_ORC_CAPTAIN){
				for(num = rn1(10,3); num >= 0; num--) makemon(rn2(3) ? &mons[PM_HILL_ORC] : &mons[PM_MORDOR_ORC], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			}
			if (anymon && mndx == PM_URUK_CAPTAIN){
				for(num = rn1(10,3); num >= 0; num--) makemon(&mons[PM_URUK_HAI], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			}
			if (anymon && mndx == PM_ORC_SHAMAN){
				for(num = rnd(3); num >= 0; num--) makemon(&mons[PM_HILL_ORC], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			}
			if (anymon && mndx == PM_ORC_OF_THE_AGES_OF_STARS){
				for(num = rn1(10,3); num >= 0; num--) makemon(&mons[PM_ANGBAND_ORC], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			}
			}
		break;
		case S_UNICORN:
			if (is_unicorn(ptr) && u.ualign.type != A_VOID &&
					sgn(u.ualign.type) == sgn(ptr->maligntyp))
				mtmp->mpeaceful = TRUE;
//			if(mndx == PM_PINK_UNICORN){
//			    mtmp->minvis = TRUE;
//			    mtmp->perminvis = TRUE;
//			}
		break;
		case S_UMBER:
			if (mndx == PM_UVUUDAUM){
				mtmp->mhpmax = 3*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
		break;
		case S_BAT:
			if (Inhell && is_bat(ptr))
			    mon_adjust_speed(mtmp, 2, (struct obj *)0);
		break;
		case S_GOLEM:
			if(mndx == PM_CENTER_OF_ALL){
			    mtmp->minvis = TRUE;
			    mtmp->perminvis = TRUE;
			}
			if(mndx == PM_GROVE_GUARDIAN){
				if(!(mmflags & MM_EDOG)){
					if (anymon){
						if(!rn2(10)) makemon(&mons[rn2(2) ? PM_ELVENKING : PM_ELVENQUEEN], mtmp->mx, mtmp->my, MM_ADJACENTOK);
						if(rn2(4)) makemon(&mons[rn2(2) ? PM_ELF_LORD : PM_ELF_LADY], mtmp->mx, mtmp->my, MM_ADJACENTOK);
						for(num = rn2(5); num >= 0; num--) makemon(&mons[PM_GREY_ELF], mtmp->mx, mtmp->my, MM_ADJACENTOK);
						for(num = d(2,4); num >= 0; num--) makemon(&mons[PM_WOODLAND_ELF], mtmp->mx, mtmp->my, MM_ADJACENTOK);
					}
				}
			}
		break;
		case S_NEU_OUTSIDER:
			if(mndx == PM_SHARAB_KAMEREL){
				set_mimic_sym(mtmp);
			}
			if(mndx == PM_HUDOR_KAMEREL){
				if(is_pool(mtmp->mx,mtmp->my, TRUE)){
					mtmp->minvis = TRUE;
					mtmp->perminvis = TRUE;
				}
			}
		break;
		case S_PUDDING:
			if(mndx == PM_DARKNESS_GIVEN_HUNGER){
				mtmp->mhpmax = 3*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			    mtmp->minvis = TRUE;
			    mtmp->perminvis = TRUE;
			}
		break;
		case S_DRAGON:
			if(mndx == PM_CHROMATIC_DRAGON){
				mtmp->mhpmax = 3*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
		break;
		case S_PLANT:
			if(mndx == PM_RAZORVINE){
				mtmp->mhpmax = .5*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
		break;
		case S_ZOMBIE:
			if (mndx == PM_DREAD_SERAPH){
				mtmp->m_lev = max(mtmp->m_lev,30);
				mtmp->mhpmax = 4*8*mtmp->m_lev;
				mtmp->mhp = mtmp->mhpmax;
			}
		break;
		case S_DEMON:
//			pline("%d\n",mtmp->mhpmax);
			if(mndx == PM_JUIBLEX){
				mtmp->mhpmax = 4*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
			else if(mndx == PM_ZUGGTMOY){
				mtmp->mhpmax = 3*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
			else if(mndx == PM_ASMODEUS){
				mtmp->mhpmax = 2*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
			else if(mndx == PM_DEMOGORGON){
				mtmp->mhpmax = 2*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
				pline("Demogorgon, Prince of Demons, is near!");
				com_pager(200);
			}
			else if(mndx == PM_LAMASHTU){
				mtmp->mhpmax = 2*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
				pline("Lamashtu, the Demon Queen, is near!");
				com_pager(201);
			}
			else if(mndx == PM_DURIN_S_BANE){
				mtmp->mhpmax = 3*mtmp->mhpmax;
				mtmp->mhp = mtmp->mhpmax;
			}
			else if(mndx == PM_PALE_NIGHT){
				mtmp->mvar1 = 0;
			}
			if(mndx == PM_ANCIENT_OF_DEATH){
			    mtmp->minvis = TRUE;
			    mtmp->perminvis = TRUE;
			}
			if(mndx == PM_HELLCAT){
				if(!isdark(mtmp->mx,mtmp->my)){
					mtmp->minvis = TRUE;
					mtmp->perminvis = TRUE;
				}
			    mtmp->invis_blkd = TRUE;
			}
			if(mndx == PM_GRUE){
				if(isdark(mtmp->mx,mtmp->my)){
					mtmp->minvis = TRUE;
					mtmp->perminvis = TRUE;
				}
			    mtmp->invis_blkd = TRUE;
			}
//			pline("%d\n",mtmp->mhpmax);
		break;
	}
	if ((ct = emits_light(mtmp->data)) > 0)
		new_light_source(mtmp->mx, mtmp->my, ct,
				 LS_MONSTER, (genericptr_t)mtmp);
	mitem = 0;	/* extra inventory item for this monster */

	if ((mcham = pm_to_cham(mndx)) != CHAM_ORDINARY) {
		/* If you're protected with a ring, don't create
		 * any shape-changing chameleons -dgk
		 */
		if (Protection_from_shape_changers)
			mtmp->cham = CHAM_ORDINARY;
		else {
			mtmp->cham = mcham;
			(void) newcham(mtmp, rndmonst(), FALSE, FALSE);
		}
	} else if (mndx == PM_WIZARD_OF_YENDOR) {
		mtmp->iswiz = TRUE;
		mtmp->mspec_used = 0; /*Wizard can cast spells right off the bat*/
		flags.no_of_wizards++;
		if (flags.no_of_wizards == 1 && Is_earthlevel(&u.uz))
			mitem = SPE_DIG;
	} else if (mndx == PM_DJINNI) {
		flags.djinni_count++;
	} else if (mndx == PM_GHOST) {
		flags.ghost_count++;
		if (!(mmflags & MM_NONAME))
			mtmp = christen_monst(mtmp, rndghostname());
	} else if (mndx == PM_VLAD_THE_IMPALER) {
		mitem = CANDELABRUM_OF_INVOCATION;
	} else if (mndx == PM_CROESUS) {
		mitem = TWO_HANDED_SWORD;
	} else if (ptr->msound == MS_NEMESIS && !(Race_if(PM_DROW) && !Role_if(PM_NOBLEMAN)) ) {
		flags.made_bell = TRUE;
		mitem = BELL_OF_OPENING;
	} else if (mndx == PM_ECLAVDRA) {
		struct obj *otmp;
		otmp = mksobj(FLAIL, TRUE, FALSE);
		otmp = oname(otmp, artiname(ART_TENTACLE_ROD));
		otmp->oerodeproof = TRUE;
		otmp->blessed = FALSE;
		otmp->cursed = TRUE;
		otmp->spe = d(2,3);
		(void) mpickobj(mtmp,otmp);
	} else if (mndx == PM_DEATH) {
		if(Role_if(PM_EXILE)){
			makemon(&mons[PM_BINAH_SEPHIRAH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			makemon(&mons[PM_BINAH_SEPHIRAH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			makemon(&mons[PM_BINAH_SEPHIRAH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
		}
	} else if (mndx == PM_FAMINE) {
		if(Role_if(PM_EXILE)){
			makemon(&mons[PM_BINAH_SEPHIRAH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			makemon(&mons[PM_BINAH_SEPHIRAH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			makemon(&mons[PM_BINAH_SEPHIRAH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
		}
	} else if (mndx == PM_PESTILENCE) {
		mitem = POT_SICKNESS;
		if(Role_if(PM_EXILE)){
			makemon(&mons[PM_BINAH_SEPHIRAH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			makemon(&mons[PM_BINAH_SEPHIRAH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
			makemon(&mons[PM_BINAH_SEPHIRAH], mtmp->mx, mtmp->my, MM_ADJACENTOK);
		}
	}
	if (mitem && allow_minvent) (void) mongets(mtmp, mitem);
	
	if(in_mklev) {
		if(((is_ndemon(ptr)) ||
		    (mndx == PM_WUMPUS) ||
		    (mndx == PM_LONG_WORM) ||
		    (mndx == PM_GIANT_EEL)) && !u.uhave.amulet && rn2(5))
			mtmp->msleeping = TRUE;
	} else {
		if(byyou) {
			newsym(mtmp->mx,mtmp->my);
			set_apparxy(mtmp);
		}
	}
	if((is_dprince(ptr) || is_dlord(ptr)) && ptr->msound == MS_BRIBE) {
	    mtmp->mpeaceful = mtmp->minvis = mtmp->perminvis = 1;
	    mtmp->mavenge = 0;
	    if (uwep && (
			uwep->oartifact == ART_EXCALIBUR
			|| uwep->oartifact == ART_LANCE_OF_LONGINUS
		) ) mtmp->mpeaceful = mtmp->mtame = FALSE;
	}
#ifndef DCC30_BUG
	if((mndx == PM_LONG_WORM || mndx == PM_HUNTING_HORROR) && 
		(mtmp->wormno = get_wormno()) != 0)
#else
	/* DICE 3.0 doesn't like assigning and comparing mtmp->wormno in the
	 * same expression.
	 */
	if ((mndx == PM_LONG_WORM || mndx == PM_HUNTING_HORROR) &&
		(mtmp->wormno = get_wormno(), mtmp->wormno != 0))
#endif
	{
	    /* we can now create worms with tails - 11/91 */
	    initworm(mtmp, mndx == PM_HUNTING_HORROR ? 2 : rn2(5));
	    if (count_wsegs(mtmp)) place_worm_tail_randomly(mtmp, x, y);
	}
	set_malign(mtmp);		/* having finished peaceful changes */
	if(u.uevent.uaxus_foe && (mndx <= PM_QUINON && mndx >= PM_MONOTON)){
		mtmp->mpeaceful = mtmp->mtame = FALSE;
	}
	if(anymon) {
	    if ((ptr->geno & G_SGROUP) && rn2(2)) {
			m_initsgrp(mtmp, mtmp->mx, mtmp->my);
	    } else if (ptr->geno & G_LGROUP) {
			if(mndx != PM_MALKUTH_SEPHIRAH){ /* Malkuths are tough, but are generated in large numbers by shopkeeper code */
				if(rn2(3))  m_initlgrp(mtmp, mtmp->mx, mtmp->my);
				else	    m_initsgrp(mtmp, mtmp->mx, mtmp->my);
			}else{
				if(!rn2(3)) m_initsgrp(mtmp, mtmp->mx, mtmp->my);
			}
	    }
	}
	
	if (allow_minvent) {
	    if(is_armed(ptr))
			m_initweap(mtmp);	/* equip with weapons / armor */
	    m_initinv(mtmp);  /* add on a few special items incl. more armor */
	    m_dowear(mtmp, TRUE);
		init_mon_wield_item(mtmp);
	} else {
	    /* no initial inventory is allowed */
	    if (mtmp->minvent) discard_minvent(mtmp);
	    mtmp->minvent = (struct obj *)0;    /* caller expects this */
	}
	if(zombiepm >= 0){
		//wasn't used
		zombiepm = -1;
	}
	if(skeletpm >= 0){
		//wasn't used
		skeletpm = -1;
	}
	if(unsethouse){
		/*At this point, we have FINALLY created the inventory for the initial creature and all its associates, so the global should be unset now.*/
		curhouse = 0;
		undeadfaction = 0;
	}
	if ((ptr->mflagst & MT_WAITMASK) && !(mmflags & MM_NOWAIT) && !u.uevent.udemigod) {
		if (ptr->mflagst & MT_WAITFORU)
			mtmp->mstrategy |= STRAT_WAITFORU;
		if (ptr->mflagst & MT_CLOSE)
			mtmp->mstrategy |= STRAT_CLOSE;
	}

	if (!in_mklev)
	    newsym(mtmp->mx,mtmp->my);	/* make sure the mon shows up */

	return(mtmp);
}

void
set_curhouse(house)
	int house;
{
	curhouse = house;
}

int
mbirth_limit(mndx)
int mndx;
{
	/* assert(MAXMONNO < 255); */
	return (mndx == PM_NAZGUL ? 9 : /*mndx == PM_ERINYS ? 3 :*/ mndx == PM_GARO_MASTER ? 1 : mndx == PM_METROID ? 21
		: mndx == PM_ALPHA_METROID ? 45 : mndx == PM_GAMMA_METROID ? 48 : mndx == PM_ZETA_METROID ? 9 
		: mndx == PM_OMEGA_METROID ? 12 : mndx == PM_METROID_QUEEN ? 3 : mndx == PM_ARGENTUM_GOLEM ? 8 
		: mndx == PM_ALHOON ? 2 : mndx == PM_CENTER_OF_ALL ? 1 : mndx == PM_SOLDIER ? 250
		: mndx == PM_ANCIENT_OF_ICE ? 8 : mndx == PM_ANCIENT_OF_DEATH ? 4
		: mndx == PM_SOLDIER_ANT ? 250 : MAXMONNO); 
}

/* used for wand/scroll/spell of create monster */
/* returns TRUE iff you know monsters have been created */
boolean
create_critters(cnt, mptr)
int cnt;
struct permonst *mptr;		/* usually null; used for confused reading */
{
	coord c;
	int x, y;
	struct monst *mon;
	boolean known = FALSE;
#ifdef WIZARD
	boolean ask = wizard;
#endif

	while (cnt--) {
#ifdef WIZARD
	    if (ask) {
		if (create_particular()) {
		    known = TRUE;
		    continue;
		}
		else ask = FALSE;	/* ESC will shut off prompting */
	    }
#endif
	    x = u.ux,  y = u.uy;
	    /* if in water, try to encourage an aquatic monster
	       by finding and then specifying another wet location */
	    if (!mptr && u.uinwater && enexto(&c, x, y, &mons[PM_GIANT_EEL]))
		x = c.x,  y = c.y;

	    mon = makemon(mptr, x, y, NO_MM_FLAGS);
	    if (mon && canspotmon(mon)) known = TRUE;
	}
	return known;
}

#endif /* OVL1 */
#ifdef OVL0

STATIC_OVL boolean
uncommon(mndx)
int mndx;
{
	if (mons[mndx].geno & (G_NOGEN | G_UNIQ)) return TRUE;
	if (mvitals[mndx].mvflags & G_GONE && !In_quest(&u.uz)) return TRUE;
	if (Inhell)
		return(mons[mndx].maligntyp > A_NEUTRAL);
	else
		return((mons[mndx].geno & G_HELL) != 0);
}

/*
 *	shift the probability of a monster's generation by
 *	comparing the dungeon alignment and monster alignment.
 *	return an integer in the range of 0-5.
 */
STATIC_OVL int
align_shift(ptr)
register struct permonst *ptr;
{
    static NEARDATA long oldmoves = 0L;	/* != 1, starting value of moves */
    static NEARDATA s_level *lev;
    register int alshift;

    if(oldmoves != moves) {
	lev = Is_special(&u.uz);
	oldmoves = moves;
    }
    switch((lev) ? lev->flags.align : dungeons[u.uz.dnum].flags.align) {
    default:	/* just in case */
    case AM_NONE:	alshift = 0;
			break;
    case AM_LAWFUL:	alshift = (ptr->maligntyp+20)/(2*ALIGNWEIGHT);
			break;
    case AM_NEUTRAL:	alshift = (20 - abs(ptr->maligntyp))/ALIGNWEIGHT;
			break;
    case AM_CHAOTIC:	alshift = (-(ptr->maligntyp-20))/(2*ALIGNWEIGHT);
			break;
    }
    return alshift;
}

static NEARDATA struct {
	int choice_count;
	int mchoices[SPECIAL_PM];	/* not at ALL sure the monsters fit into a char, and in this age trying to save such a
								   small amount of memory seems outdated */
} rndmonst_state = { -1, {0} };

static int roguemons[] = {
	/*A*/
	PM_BAT,
	PM_PLAINS_CENTAUR,
	PM_RED_DRAGON,
	PM_STALKER,
	PM_LICHEN,
	PM_GNOME,
	/*H*/
	/*I(stalker, above)*/
	PM_JABBERWOCK,
	/*K*/
	/*L*/
	/*M*/
	/*N*/
	PM_OGRE,
	/*P*/
	/*Q*/
	PM_RUST_MONSTER,
	PM_PIT_VIPER,
	PM_TROLL,
	PM_UMBER_HULK,
	PM_VAMPIRE,
	PM_WRAITH,
	PM_XORN,
	PM_YETI,
	PM_ZOMBIE
};

STATIC_OVL
struct permonst *
roguemonst()
{
	int mn, tries=0;
	int zlevel, minmlev, maxmlev;
	
	zlevel = level_difficulty();
	/* determine the level of the weakest monster to make. */
	if(u.uevent.udemigod) minmlev = zlevel / 3;
	else minmlev = zlevel / 6;
	/* determine the level of the strongest monster to make. */
	maxmlev = (zlevel + u.ulevel) / 2;
	
	do mn = roguemons[rn2(SIZE(roguemons))];
	while(tooweak(mn, minmlev) || toostrong(mn,maxmlev) || tries++ > 40);
	return &mons[mn];
}

/* select a random monster type */
struct permonst *
rndmonst()
{
	register struct permonst *ptr;
	register int mndx, ct;
	int zlevel, minmlev, maxmlev;
	
	zlevel = level_difficulty();
	/* determine the level of the weakest monster to make. */
	if(u.uevent.udemigod) minmlev = zlevel / 3;
	else minmlev = zlevel / 6;
	/* determine the level of the strongest monster to make. */
	maxmlev = (zlevel + u.ulevel) / 2;

	if(u.ukinghill){ /* You have pirate quest artifact in open inventory */
		if(rnd(100)>80){
			if(In_endgame(&u.uz)) return &mons[PM_GITHYANKI_PIRATE];
			else if(Inhell) return &mons[PM_DAMNED_PIRATE];
			else return &mons[PM_SKELETAL_PIRATE];
		}
	}

	if((u.uevent.sum_entered || !rn2(100)) && !(mvitals[PM_CENTER_OF_ALL].mvflags & G_EXTINCT) && !rn2(100)){
	    return &mons[PM_CENTER_OF_ALL]; /*center of all may be created at any time, but is much more likely after
												entering sum of all */
	}

	if (u.uz.dnum == quest_dnum && !undeadfaction && (ptr = qt_montype()) != 0){
		if(ptr == &mons[PM_LONG_WORM_TAIL]) return (struct permonst *) 0;
	    else if(Role_if(PM_ANACHRONONAUT) || rn2(7)) return ptr;
		//else continue to random generation
	}
	else if (In_neu(&u.uz) && 
		(
			Is_rlyeh(&u.uz) ||  Is_sumall(&u.uz) || Is_gatetown(&u.uz) || 
			(rn2(10) && (In_outlands(&u.uz)))
		)
	){
	    if(!in_mklev) return neutral_montype();
		else return (struct permonst *)0;/*NOTE: ugly method to stop monster generation during level creation, since I can't find a better way*/
	}
	else if (In_cha(&u.uz)){
	    return chaos_montype();
	}
	else if (In_law(&u.uz)){
	    return law_montype();
	}
	else if (In_mines(&u.uz)){
		int roll = d(1,10);
		if(Race_if(PM_GNOME) && Role_if(PM_RANGER) && rn2(2)){
			switch(roll){
				case 1:	case 2: case 3: case 4: return mkclass(S_KOBOLD, G_NOHELL); break;
				case 5:	case 6: return rn2(6) ? &mons[PM_GNOME] : &mons[PM_DWARF]; break;
				case 7: return &mons[PM_IMP]; break;
				default: break; //proceed with normal generation
			}
		} else {
			switch(roll){
				case 1:	case 2: case 3: case 4: return mkclass(S_GNOME, G_NOHELL); break;
				case 5:	case 6: return &mons[PM_DWARF]; break;
				case 7: return rn2(6) ? &mons[PM_GNOME] : &mons[PM_DWARF]; break;
				default: break; //proceed with normal generation
			}
		}
	}
	else if(Is_juiblex_level(&u.uz)){
		if(rn2(2)) return rn2(2) ? mkclass(S_BLOB, G_NOHELL|G_HELL) : mkclass(S_PUDDING, G_NOHELL|G_HELL);
		//else default
	}
	else if(Is_zuggtmoy_level(&u.uz)){
		if(rn2(2)) return rn2(3) ? mkclass(S_FUNGUS, G_NOHELL|G_HELL) : mkclass(S_PLANT, G_NOHELL|G_HELL);
		//else default
	}
	else if(Is_yeenoghu_level(&u.uz)){
		int roll = d(1,20);
		switch(roll){
			case 1:	case 2: case 3: case 4: 
			case 5:	case 6: case 7: case 8: return &mons[PM_GNOLL]; break;
			case 9:	return &mons[PM_ANUBITE]; break;
			case 10: case 11: return &mons[PM_GNOLL_GHOUL]; break;
			case 12: return &mons[PM_GNOLL_MATRIARCH]; break;
			default: break; //proceed with normal generation
		}
	}
	else if(Is_baphomet_level(&u.uz)){
		int roll = d(1,10);
		switch(roll){
			case 1:	case 2: return &mons[PM_MINOTAUR]; break;
			case 3: return &mons[PM_MINOTAUR_PRIESTESS]; break;
			default: break; //proceed with normal generation
		}
	}
	else if(Is_night_level(&u.uz)){
		if(rn2(2)) return rn2(2) ? mkclass(S_ZOMBIE, G_NOHELL|G_HELL) : &mons[PM_SKELETON];
		if(!rn2(20)) return mkclass(S_LICH, G_NOHELL|G_HELL);
		//else default
	}
	else if(Is_malcanthet_level(&u.uz)){
		if(!rn2(6)) return rn2(3) ? &mons[PM_SUCCUBUS] : &mons[PM_INCUBUS];
		//else default
	}
	else if(Is_grazzt_level(&u.uz)){
		if(!rn2(6)) return !rn2(3) ? &mons[PM_SUCCUBUS] : &mons[PM_INCUBUS];
		//else default
	}
	else if(Is_orcus_level(&u.uz)){
		if(rn2(2)) return !rn2(3) ? mkclass(S_WRAITH, G_NOHELL|G_HELL) : &mons[PM_SHADE];
		if(!rn2(20)) return mkclass(S_LICH, G_NOHELL|G_HELL);
		//else default
	}
	else if(Is_lolth_level(&u.uz)){
		int roll = d(1,10);
		if(roll == 10) return &mons[PM_DROW_MATRON];
		if(roll > 4) return mkclass(S_SPIDER, G_NOHELL|G_HELL);
	}
	else if(Is_demogorgon_level(&u.uz)){
		if(rn2(3)) return mkclass(S_DEMON, G_NOHELL|G_HELL) ;
		//else default
	}
	else if(Is_dagon_level(&u.uz)){
		if(rn2(2)){
			return rn2(3) ? &mons[PM_KRAKEN] : rn2(2) ? &mons[PM_SHARK] : &mons[PM_ELECTRIC_EEL];
		}
		//else default
	}

	if(u.hod && !rn2(10) && rn2(40+u.hod) > 50){
		u.hod-=10;
		if(u.hod<0) u.hod = 0;
		if(!tooweak(PM_HOD_SEPHIRAH, minmlev)){
			return &mons[PM_HOD_SEPHIRAH];
		}
		else u.keter++;
	}
	if(u.gevurah && !rn2(20) && rn2(u.gevurah + 94) > 100){
		/* Notes on frequency: cheating death via lifesaving counts as +4
			cheating death via rehumanization counts as +1*/
		if(!tooweak(PM_GEVURAH_SEPHIRAH, minmlev)){
			return &mons[PM_GEVURAH_SEPHIRAH];
		}
		else{
			u.gevurah -= 4;
			if(u.gevurah<0) u.gevurah = 0;
			u.keter++;
			return &mons[PM_CHOKHMAH_SEPHIRAH];
		}
	}
	if(u.keter && !rn2(100) && rn2(u.keter+10) > 10){
		u.chokhmah++;
		return &mons[PM_CHOKHMAH_SEPHIRAH];
	}
	if (u.uz.dnum == tower_dnum)
		switch(rn2(10)){
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
				return mkclass(S_ZOMBIE, G_NOHELL);
			break;
			case 5:
			case 6:
				return mkclass(S_IMP, G_NOHELL);
			break;
			case 7:
				return mkclass(S_DOG,G_NOHELL);
			break;
			case 8:
				return mkclass(S_VAMPIRE, G_NOHELL);
			break;
			case 9:
				return mkclass(S_DEMON, G_HELL);
			break;
		}
	if (u.uz.dnum == tomb_dnum)
		return rn2(2) ? mkclass(S_ZOMBIE, G_NOHELL) : mkclass(S_MUMMY, G_NOHELL);

	if (u.uz.dnum == temple_dnum)
		return rn2(4) ? mkclass(S_ZOMBIE, G_NOHELL) : mkclass(S_BLOB, G_NOHELL);
	
	if(In_sea(&u.uz)){
		if (Is_sunsea(&u.uz))
			return rn2(3) ? &mons[PM_JELLYFISH] : rn2(2) ? &mons[PM_SHARK] : &mons[PM_GIANT_EEL];
		else if(Is_paradise(&u.uz)){
			switch(rn2(10)){
				case 0:
				case 1:
				case 2:
				case 3:
					return mkclass(S_EEL, G_NOHELL);
				break;
				case 4:
				case 5:
					return &mons[PM_PARROT];
				break;
				case 6:
					return mkclass(S_SNAKE, G_NOHELL);
				break;
				case 7:
					return &mons[PM_MONKEY];
				break;
				case 8:
					return mkclass(S_LIZARD,G_NOHELL);
				break;
				case 9:
					return mkclass(S_SPIDER,G_NOHELL);
				break;
			}
		}
		else if(Is_sunkcity(&u.uz)){
			switch(rn2(10)){
				case 0:
				case 1:
				case 2:
				case 3:
					return mkclass(S_EEL, G_NOHELL);
				break;
				case 4:
				case 5:
					return &mons[PM_DEEP_ONE];
				break;
				case 6:
					return &mons[PM_DEEPER_ONE];
				break;
				case 7:
				case 8:
					return &mons[PM_SOLDIER];
				break;
				case 9:
					return mkclass(S_DOG,G_NOHELL);
				break;
			}
		}
		else if(Is_peanut(&u.uz)){
			switch(rn2(10)){
				case 0:
				case 1:
					return mkclass(S_EEL, G_NOHELL);
				break;
				case 2:
				case 3:
				case 4:
					return &mons[PM_PARROT];
				break;
				case 5:
				case 6:
				case 7:
					return &mons[PM_SKELETAL_PIRATE];
				break;
				case 8:
					return &mons[PM_MONKEY];
				break;
				case 9:
					return mkclass(S_LIZARD,G_NOHELL);
				break;
			}
		}
	}
	if(Is_rogue_level(&u.uz))
		return roguemonst();

	if (rndmonst_state.choice_count < 0) {	/* need to recalculate */
	    int zlevel, minmlev, maxmlev;
	    boolean elemlevel;

	    rndmonst_state.choice_count = 0;
	    /* look for first common monster */
	    for (mndx = LOW_PM; mndx < SPECIAL_PM; mndx++) {
		if (!uncommon(mndx)) break;
		rndmonst_state.mchoices[mndx] = 0;
	    }		
	    if (mndx == SPECIAL_PM) {
		/* evidently they've all been exterminated */
#ifdef DEBUG
		pline("rndmonst: no common mons!");
#endif
		return (struct permonst *)0;
	    } /* else `mndx' now ready for use below */
	    zlevel = level_difficulty();
	    /* determine the level of the weakest monster to make. */
	    minmlev = zlevel / 6;
	    /* determine the level of the strongest monster to make. */
	    maxmlev = (zlevel + u.ulevel) / 2;
	    elemlevel = In_endgame(&u.uz) && !Is_astralevel(&u.uz);

/*
 *	Find out how many monsters exist in the range we have selected.
 */
	    /* (`mndx' initialized above) */
	    for ( ; mndx < SPECIAL_PM; mndx++) {
		ptr = &mons[mndx];
		rndmonst_state.mchoices[mndx] = 0;
		if (tooweak(mndx, minmlev) || toostrong(mndx, maxmlev))
		    continue;
		if (elemlevel && wrong_elem_type(ptr)) continue;
		if (uncommon(mndx)) continue;
		if (Inhell && (ptr->geno & G_NOHELL)) continue;
		if (!In_endgame(&u.uz) && ((ptr->geno & (G_PLANES|G_HELL|G_NOHELL)) == G_PLANES)) continue;
		ct = (int)(ptr->geno & G_FREQ) + align_shift(ptr);
		if (ct < 0 || ct > 127)
		    panic("rndmonst: bad count [#%d: %d]", mndx, ct);
		rndmonst_state.choice_count += ct;
		rndmonst_state.mchoices[mndx] = (char)ct;
	    }
/*
 *	    Possible modification:  if choice_count is "too low",
 *	    expand minmlev..maxmlev range and try again.
 */
	} /* choice_count+mchoices[] recalc */

	if (rndmonst_state.choice_count <= 0) {
	    /* maybe no common mons left, or all are too weak or too strong */
#ifdef DEBUG
	    Norep("rndmonst: choice_count=%d", rndmonst_state.choice_count);
#endif
	    return (struct permonst *)0;
	}

/*
 *	Now, select a monster at random.
 */
	ct = rnd(rndmonst_state.choice_count);
	for (mndx = LOW_PM; mndx < SPECIAL_PM; mndx++)
	    if ((ct -= (int)rndmonst_state.mchoices[mndx]) <= 0) break;

	if (mndx == SPECIAL_PM || uncommon(mndx)) {	/* shouldn't happen */
	    impossible("rndmonst: bad `mndx' [#%d]", mndx);
	    return (struct permonst *)0;
	}
	return &mons[mndx];
}

/* select a random monster type for a shapeshifter to turn into */
int
rndshape()
{
	register struct permonst *ptr;
	register int mndx, ct;
	int zlevel, minmlev, maxmlev;
	int choice_count;
	int mchoices[SPECIAL_PM];
	boolean elemlevel;
	
	zlevel = level_difficulty();
	
	/*increase difficulty to allow mildly out of depth monsters */
	zlevel += 12;
	
	/* determine the level of the weakest monster to make. */
	minmlev = zlevel / 6;
	/* determine the level of the strongest monster to make. */
	maxmlev = (zlevel + u.ulevel) / 2;

#ifdef REINCARNATION
	boolean upper;
#endif

	choice_count = 0;
	/* look for first common monster */
	for (mndx = LOW_PM; mndx < SPECIAL_PM; mndx++) {
		if (!uncommon(mndx)) break;
		mchoices[mndx] = 0;
	}		
	if (mndx == SPECIAL_PM) {
	/* evidently they've all been exterminated */
#ifdef DEBUG
	pline("rndmonst: no common mons!");
#endif
	return NON_PM;
	} /* else `mndx' now ready for use below */
#ifdef REINCARNATION
	upper = Is_rogue_level(&u.uz);
#endif
	elemlevel = In_endgame(&u.uz) && !Is_astralevel(&u.uz);

/*
*	Find out how many monsters exist in the range we have selected.
*/
	/* (`mndx' initialized above) */
	for ( ; mndx < SPECIAL_PM; mndx++) {
	ptr = &mons[mndx];
	mchoices[mndx] = 0;
	if (tooweak(mndx, minmlev) || toostrong(mndx, maxmlev))
		continue;
#ifdef REINCARNATION
	if (upper && !isupper(def_monsyms[(int)(ptr->mlet)])) continue;
#endif
	if (elemlevel && wrong_elem_type(ptr)) continue;
	if (uncommon(mndx)) continue;
	ct = (int)(ptr->geno & G_FREQ)/2 + align_shift(ptr);
	if (ct < 0 || ct > 127)
		panic("rndmonst: bad count [#%d: %d]", mndx, ct);
	choice_count += ct;
	mchoices[mndx] = (char)ct;
	}
/*
 *	    Possible modification:  if choice_count is "too low",
 *	    expand minmlev..maxmlev range and try again.
 */

	if (choice_count <= 0) {
	    /* maybe no common mons left, or all are too weak or too strong */
#ifdef DEBUG
	    Norep("rndmonst: choice_count=%d", choice_count);
#endif
	    return NON_PM;
	}

/*
 *	Now, select a monster at random.
 */
	ct = rnd(choice_count);
	for (mndx = LOW_PM; mndx < SPECIAL_PM; mndx++)
	    if ((ct -= (int)mchoices[mndx]) <= 0) break;

	if (mndx == SPECIAL_PM || uncommon(mndx)) {	/* shouldn't happen */
	    impossible("rndmonst: bad `mndx' [#%d]", mndx);
	    return NON_PM;
	}
	return mndx;
}



/* called when you change level (experience or dungeon depth) or when
   monster species can no longer be created (genocide or extinction) */
void
reset_rndmonst(mndx)
int mndx;	/* particular species that can no longer be created */
{
	/* cached selection info is out of date */
	if (mndx == NON_PM) {
	    rndmonst_state.choice_count = -1;	/* full recalc needed */
	} else if (mndx < SPECIAL_PM) {
	    rndmonst_state.choice_count -= rndmonst_state.mchoices[mndx];
	    rndmonst_state.mchoices[mndx] = 0;
	} /* note: safe to ignore extinction of unique monsters */
}

#endif /* OVL0 */
#ifdef OVL1

/*	The routine below is used to make one of the multiple types
 *	of a given monster class.  The second parameter specifies a
 *	special casing bit mask to allow the normal genesis
 *	masks to be deactivated.  Returns 0 if no monsters
 *	in that class can be made.
 */

struct permonst *
mkclass(class,spc)
char	class;
int	spc;
{
	register int	first, last, num = 0;
	int maxmlev, mask = (G_PLANES | G_NOHELL | G_HELL | G_NOGEN | G_UNIQ) & ~spc;

	maxmlev = level_difficulty() >> 1;
	if(class < 1 || class >= MAXMCLASSES) {
	    impossible("mkclass called with bad class!");
	    return((struct permonst *) 0);
	}
	if(Race_if(PM_DROW) && (Role_if(PM_PRIEST) || Role_if(PM_ROGUE) || Role_if(PM_RANGER) || Role_if(PM_WIZARD)) &&
		!flags.initgend && flags.stag == 0 && In_quest(&u.uz) && class == S_MUMMY && !(mvitals[PM_DROW_MUMMY].mvflags & G_GENOD && !In_quest(&u.uz))
	) return &mons[PM_DROW_MUMMY];
	if(Race_if(PM_DROW) && (Role_if(PM_PRIEST) || Role_if(PM_ROGUE) || Role_if(PM_RANGER) || Role_if(PM_WIZARD)) &&
		!flags.initgend && flags.stag == 0 && In_quest(&u.uz) && class == S_ZOMBIE && !(mvitals[PM_HEDROW_ZOMBIE].mvflags & G_GENOD && !In_quest(&u.uz))
	) return &mons[PM_HEDROW_ZOMBIE];
	if(class == S_ZOMBIE)
		return mkzombie();
/*	Assumption #1:	monsters of a given class are contiguous in the
 *			mons[] array.
 */
	for (first = LOW_PM; first < SPECIAL_PM; first++)
	    if (mons[first].mlet == class && !(mons[first].geno & mask)) break;
	if (first == SPECIAL_PM) return (struct permonst *) 0;

	for (last = first;
		last < SPECIAL_PM && mons[last].mlet == class; last++)
	    if (!(mvitals[last].mvflags & G_GONE && !In_quest(&u.uz)) && !(mons[last].geno & mask)
					&& !is_placeholder(&mons[last])) {
		/* consider it */
		if(num && toostrong(last, maxmlev) &&
		   monstr[last] != monstr[last-1] && (rn2(2) || monstr[last] > maxmlev+5)
		) break;
		num += mons[last].geno & G_FREQ;
	    }

	if(!num) return((struct permonst *) 0);

/*	Assumption #2:	monsters of a given class are presented in ascending
 *			order of strength.
 */
	for(num = rnd(num); num > 0; first++)
	    if (!(mvitals[first].mvflags & G_GONE && !In_quest(&u.uz)) && !(mons[first].geno & mask)
					&& !is_placeholder(&mons[first])) {
		/* skew towards lower value monsters at lower exp. levels */
		num -= mons[first].geno & G_FREQ;
		if (num && adj_lev(&mons[first]) > (u.ulevel*2)) {
		    /* but not when multiple monsters are same level */
		    if (mons[first].mlevel != mons[first+1].mlevel)
			num--;
		}
	    }
	first--; /* correct an off-by-one error */

	return(&mons[first]);
}

static const int standardZombies[] = {
					  PM_KOBOLD, //1
					  PM_HUMAN, //2
					  PM_GNOME, //3
					  PM_ORC, //3
					  PM_DWARF, //4
					  PM_DOG, //?
					  PM_URUK_HAI, //5
					  PM_ELF, //6
					  PM_OGRE, //7
					  PM_GIANT, //8
					  PM_ETTIN, //13
					  PM_HALF_DRAGON, //16
					  PM_MASTODON //23
					};

static const int orcusZombies[] = {
					  PM_WEREWOLF,
					  PM_MANTICORE,
					  PM_TITANOTHERE,
					  PM_BALUCHITHERIUM,
					  PM_MASTODON,
					  PM_LONG_WORM,
					  PM_PURPLE_WORM,
					  PM_JABBERWOCK
					};

static const int orcusSkeletons[] = {
					  PM_WINGED_KOBOLD,
					  PM_SOLDIER,
					  PM_HILL_ORC,
					  PM_DWARF,
					  PM_BUGBEAR,
					  PM_DOG,
					  PM_URUK_HAI,
					  PM_ELF,
					  PM_OGRE,
					  PM_GIANT,
					  PM_ETTIN,
					  PM_HALF_DRAGON
					};

static const int granfaloonZombies[] = {
						// PM_NOBLEMAN,
						// PM_NOBLEWOMAN,
						// PM_PRIEST,
						// PM_PRIESTESS,
						PM_CAPTAIN,
						PM_DWARF_KING,
						PM_DWARF_QUEEN,
						PM_GNOME_KING,
						PM_GNOME_QUEEN,
						PM_ELVENKING,
						PM_ELVENQUEEN
						// PM_DROW_MATRON
					};
static const int elfZombies[] = {
						PM_WOODLAND_ELF,
						PM_GREEN_ELF,
						PM_GREY_ELF,
						PM_ELF_LORD,
						PM_ELF_LADY,
						PM_HILL_ORC,
						PM_MORDOR_ORC,
						PM_ANGBAND_ORC
					};

struct permonst *
mkzombie()
{
	register int	first, last, num = 0;
	int maxmlev;

	maxmlev = level_difficulty() >> 1;
	if(Is_orcus_level(&u.uz)){
		if(!rn2(2)){
			skeletpm = orcusSkeletons[rn2(SIZE(orcusSkeletons))];
			return &mons[skeletpm];
		} else {
			zombiepm = orcusZombies[rn2(SIZE(orcusZombies))];
			return &mons[zombiepm];
		}
	}
	if(In_quest(&u.uz) && Role_if(PM_EXILE)){
		(urace.zombienum != NON_PM) ? (zombiepm = urace.zombienum) : (zombiepm = PM_HUMAN);
		return &mons[zombiepm];
	}
	if(In_quest(&u.uz) && Role_if(PM_PIRATE)){
		zombiepm = PM_HUMAN;
		return &mons[zombiepm];
	}
	if(In_quest(&u.uz) && Role_if(PM_PRIEST) && urole.neminum == PM_LEGION && mvitals[PM_LEGION].died == 0){
		zombiepm = granfaloonZombies[rn2(SIZE(granfaloonZombies))];
		return &mons[zombiepm];
	}
	if(In_quest(&u.uz) && urole.neminum == PM_NECROMANCER){
		zombiepm = elfZombies[rn2(SIZE(elfZombies))];
		return &mons[zombiepm];
	}
	if(u.uz.dnum == tower_dnum){
		zombiepm = rn2(3) ? PM_PEASANT : PM_WOLF;
		return &mons[zombiepm];
	}
	// if(Race_if(PM_DROW) && (Role_if(PM_PRIEST) || Role_if(PM_ROGUE) || Role_if(PM_RANGER) || Role_if(PM_WIZARD)) &&
		// !flags.initgend && flags.stag == 0 && In_quest(&u.uz) && class == S_ZOMBIE && !(mvitals[PM_HEDROW_ZOMBIE].mvflags & G_GENOD && !In_quest(&u.uz))
	// ) return &mons[PM_HEDROW_ZOMBIE];
	// if(class == S_ZOMBIE)
		// return mkzombie();

	first = 0;

	for (last = first; last < SIZE(standardZombies); last++)
	    if (!(mvitals[standardZombies[last]].mvflags & G_GENOD && !In_quest(&u.uz)) ) {
		/* consider it */
		if(num && toostrong(standardZombies[last], maxmlev) &&
		   monstr[standardZombies[last]] != monstr[standardZombies[last-1]] && (rn2(2) || monstr[standardZombies[last]] > maxmlev+5)
		) break;
		num += mons[standardZombies[last]].geno & G_FREQ;
	    }

	if(!num) return((struct permonst *) 0);

/*	Assumption #2:	monsters of a given class are presented in ascending
 *			order of strength.
 */
	for(num = rnd(num); num > 0; first++)
	    if (!(mvitals[standardZombies[first]].mvflags & G_GENOD && !In_quest(&u.uz)) ) {
			num -= mons[standardZombies[first]].geno & G_FREQ;
	    }
	first--; /* correct an off-by-one error */
	
	zombiepm = standardZombies[first];
	 
	return(&mons[standardZombies[first]]);
}

int
adj_lev(ptr)	/* adjust strength of monsters based on u.uz and u.ulevel */
register struct permonst *ptr;
{
	int	tmp, tmp2;

	if (ptr == &mons[PM_WIZARD_OF_YENDOR]) {
		/* does not depend on other strengths, but does get stronger
		 * every time he is killed
		 */
		tmp = ptr->mlevel + mvitals[PM_WIZARD_OF_YENDOR].died;
		if (tmp > 49) tmp = 49;
		return tmp;
	}

	if((tmp = ptr->mlevel) > 49){
//		pline("HD: %d", ptr->mlevel);
		return tmp; /* "special" demons/devils */
	}
	tmp2 = (level_difficulty() - tmp);
	if(tmp2 < 0) tmp--;		/* if mlevel > u.uz decrement tmp */
	else tmp += (tmp2 / 5);		/* else increment 1 per five diff */

	tmp2 = (u.ulevel - ptr->mlevel);	/* adjust vs. the player */
	if(tmp2 > 0) tmp += (tmp2 / 4);		/* level as well */

	tmp2 = (3 * ((int) ptr->mlevel))/ 2;	/* crude upper limit */
	if (tmp2 > 49) tmp2 = 49;		/* hard upper limit */
	return((tmp > tmp2) ? tmp2 : (tmp > 0 ? tmp : 0)); /* 0 lower limit */
}

#endif /* OVL1 */
#ifdef OVLB

struct permonst *
grow_up(mtmp, victim)	/* `mtmp' might "grow up" into a bigger version */
struct monst *mtmp, *victim;
{
	int oldtype, newtype, max_increase, cur_increase,
	    lev_limit, hp_threshold;
	struct permonst *ptr = mtmp->data;
	struct monst *bardmon;

	/* monster died after killing enemy but before calling this function */
	/* currently possible if killing a gas spore */
	if (mtmp->mhp <= 0)
	    return ((struct permonst *)0);

	if(mtmp->m_lev > 50 || ptr == &mons[PM_CHAOS]) return ((struct permonst *)0);
	/* note:  none of the monsters with special hit point calculations
	   have both little and big forms */
	oldtype = monsndx(ptr);
	newtype = little_to_big(oldtype, (boolean)mtmp->female);

	/* growth limits differ depending on method of advancement */
	if (victim) {		/* killed a monster */
	    /*
	     * The HP threshold is the maximum number of hit points for the
	     * current level; once exceeded, a level will be gained.
	     * Possible bug: if somehow the hit points are already higher
	     * than that, monster will gain a level without any increase in HP.
	     */
	    hp_threshold = mtmp->m_lev * 8;		/* normal limit */
	    if (!mtmp->m_lev)
		hp_threshold = 4;
	    else if (is_golem(ptr))	/* strange creatures */
		hp_threshold = ((mtmp->mhpmax / 10) + 1) * 10 - 1;
	    else if (is_home_elemental(ptr) || 
			ptr == &mons[PM_DARKNESS_GIVEN_HUNGER] ||
			ptr == &mons[PM_WATCHER_IN_THE_WATER] ||
			ptr == &mons[PM_KETO] ||
			ptr == &mons[PM_DURIN_S_BANE] ||
			ptr == &mons[PM_CHROMATIC_DRAGON] ||
			ptr == &mons[PM_BOLG] ||
			ptr == &mons[PM_UVUUDAUM] ||
			ptr == &mons[PM_PRIEST_OF_GHAUNADAUR] ||
			ptr == &mons[PM_SHOGGOTH]
		) hp_threshold *= 3;
	    else if (ptr == &mons[PM_RAZORVINE]) hp_threshold *= .5;
		else if(ptr == &mons[PM_CHAOS]) hp_threshold *= 15;
		else if(ptr == &mons[PM_KARY__THE_FIEND_OF_FIRE]) hp_threshold *= 10;
		else if(ptr == &mons[PM_LICH__THE_FIEND_OF_EARTH]) hp_threshold *= 10;
		else if(ptr == &mons[PM_KRAKEN__THE_FIEND_OF_WATER]) hp_threshold *= 10;
		else if(ptr == &mons[PM_TIAMAT__THE_FIEND_OF_WIND]) hp_threshold *= 10;
		else if(ptr == &mons[PM_CHOKHMAH_SEPHIRAH]) hp_threshold *= u.chokhmah;
	    lev_limit = 3 * (int)ptr->mlevel / 2;	/* same as adj_lev() */
	    /* If they can grow up, be sure the level is high enough for that */
	    if (oldtype != newtype && mons[newtype].mlevel > lev_limit)
		lev_limit = (int)mons[newtype].mlevel;
	    // /* number of hit points to gain; unlike for the player, we put
	       // the limit at the bottom of the next level rather than the top */
	    // max_increase = rnd((int)victim->m_lev + 1);
	    // if (mtmp->mhpmax + max_increase > hp_threshold + 1)
			// max_increase = max((hp_threshold + 1) - mtmp->mhpmax, 0);
	    // cur_increase = (max_increase > 0) ? rn2(max_increase)+1 : 0;
		if(mtmp->mhp < hp_threshold-8 || mtmp->m_lev < victim->m_lev + d(2,5)){ /*allow monsters to quickly gain hp up to around their HP limit*/
			max_increase = 1;
			cur_increase = 1;
			if(Role_if(PM_BARD) && mtmp->mtame && canseemon(mtmp)){
				u.pethped = TRUE;
			}
			for(bardmon = fmon; bardmon; bardmon = bardmon->nmon){
				if(is_bardmon(bardmon->data) 
					&& !is_bardmon(mtmp->data) 
					&& ((bardmon->mtame > 0) == (mtmp->mtame > 0)) && bardmon->mpeaceful == mtmp->mpeaceful
					&& mon_can_see_mon(bardmon,mtmp)
				) grow_up(bardmon, mtmp);
			}
		} else {
			max_increase = 0;
			cur_increase = 0;
		}
	} else {
	    /* a gain level potion or wraith corpse; always go up a level
	       unless already at maximum (49 is hard upper limit except
	       for demon lords, who start at 50 and can't go any higher) */
	    max_increase = cur_increase = rnd(8);
	    hp_threshold = 0;	/* smaller than `mhpmax + max_increase' */
	    lev_limit = 50;		/* recalc below */
	}

	mtmp->mhpmax += max_increase;
	mtmp->mhp += cur_increase;
	if (mtmp->mhpmax <= hp_threshold)
	    return ptr;		/* doesn't gain a level */

	if (is_mplayer(ptr) || ptr == &mons[PM_BYAKHEE] || ptr == &mons[PM_LILLEND] || ptr == &mons[PM_MAID]
	|| ptr == &mons[PM_CROW_WINGED_HALF_DRAGON] || ptr == &mons[PM_BASTARD_OF_THE_BOREAL_VALLEY]
	|| ptr == &mons[PM_UNDEAD_KNIGHT] || ptr == &mons[PM_WARRIOR_OF_SUNLIGHT]
	|| ptr == &mons[PM_FORMIAN_CRUSHER]
	) lev_limit = 30;	/* same as player */
	else if (is_eladrin(ptr) && ptr->mlevel <= 20) lev_limit = 30;
	else if (ptr == &mons[PM_ANCIENT_OF_ICE] || ptr == &mons[PM_ANCIENT_OF_DEATH]) lev_limit = 45;
	else if (lev_limit < 5) lev_limit = 5;	/* arbitrary */
	else if (lev_limit > 49) lev_limit = (ptr->mlevel > 49 ? ptr->mlevel : 49);

	if ((int)++mtmp->m_lev >= mons[newtype].mlevel && newtype != oldtype) {
	    ptr = &mons[newtype];
	    if (mvitals[newtype].mvflags & G_GENOD && !In_quest(&u.uz)) {	/* allow G_EXTINCT */
		if (sensemon(mtmp))
		    pline("As %s grows up into %s, %s %s!", mon_nam(mtmp),
			an(ptr->mname), mhe(mtmp),
			nonliving_mon(mtmp) ? "expires" : "dies");
		set_mon_data(mtmp, ptr, -1);	/* keep mvitals[] accurate */
		mondied(mtmp);
		return (struct permonst *)0;
	    }
	    set_mon_data(mtmp, ptr, 1);		/* preserve intrinsics */
	    newsym(mtmp->mx, mtmp->my);		/* color may change */
	    lev_limit = (int)mtmp->m_lev;	/* never undo increment */
		if(newtype == PM_METROID_QUEEN && mtmp->mtame){
			struct monst *baby;
			int tnum = d(1,6);
			int i;
			mtmp->mtame = 0;
			mtmp->mpeaceful = 1;
			for(i = 0; i < 6; i++){
				baby = makemon(&mons[PM_METROID], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				if(tnum-->0) tamedog(baby,(struct obj *) 0);
			}
		}
	}
	/* sanity checks */
	if ((int)mtmp->m_lev > lev_limit) {
	    mtmp->m_lev--;	/* undo increment */
	    /* HP might have been allowed to grow when it shouldn't */
	    if (mtmp->mhpmax >= hp_threshold + 1) mtmp->mhpmax--;
	}
//	if( !(monsndx(mtmp)<PM_DJINNI && monsndx(mtmp)>PM_BALROG) ){
		// Some High level stuff is higher than this.
		// if (mtmp->mhpmax > 50*8) mtmp->mhpmax = 50*8;	  /* absolute limit */
		// if (mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;
//	}
	return ptr;
}

#endif /* OVLB */
#ifdef OVL1

int
mongets(mtmp, otyp)
register struct monst *mtmp;
register int otyp;
{
	register struct obj *otmp;
	int spe;

	if (undeadfaction && mtmp->mfaction == undeadfaction && rn2(2))
		return 0;
	if (!otyp) return 0;
	otmp = mksobj(otyp, TRUE, FALSE);
	if (otmp) {
		/*Size and shape it to owner*/
		if((otmp->oclass == ARMOR_CLASS || otmp->oclass == WEAPON_CLASS) && mtmp->data != &mons[PM_HOBBIT])
			otmp->objsize = mtmp->data->msize;
		if(otmp->otyp == BULLWHIP && is_drow(mtmp->data) && mtmp->female)
			otmp->obj_material = SILVER;
		if(otmp->oclass == ARMOR_CLASS && !Is_dragon_scales(otmp)){
			if(is_suit(otmp)) otmp->bodytypeflag = (mtmp->data->mflagsb&MB_BODYTYPEMASK);
			else if(is_helmet(otmp)) otmp->bodytypeflag = (mtmp->data->mflagsb&MB_HEADMODIMASK);
			else if(is_shirt(otmp)) otmp->bodytypeflag = (mtmp->data->mflagsb&MB_HUMANOID) ? MB_HUMANOID : (mtmp->data->mflagsb&MB_BODYTYPEMASK);
		}
		
		if(mtmp->data->mlet == S_GOLEM){
			if(otmp->oclass == WEAPON_CLASS || otmp->oclass == ARMOR_CLASS){
				if(mtmp->data == &mons[PM_GOLD_GOLEM]){
					otmp->obj_material = GOLD;
				} else if(mtmp->data == &mons[PM_TREASURY_GOLEM]){
					otmp->obj_material = GOLD;
				} else if(mtmp->data == &mons[PM_GLASS_GOLEM]){
					otmp->obj_material = GLASS;
				} else if(mtmp->data == &mons[PM_GROVE_GUARDIAN]){
					otmp->obj_material = SILVER;
				} else if(mtmp->data == &mons[PM_IRON_GOLEM]){
					otmp->obj_material = IRON;
				} else if(mtmp->data == &mons[PM_ARGENTUM_GOLEM]){
					otmp->obj_material = SILVER;
				}
			}
		}
		if (is_demon(mtmp->data)) {
			/* demons never get blessed objects */
			if (otmp->blessed) curse(otmp);
	    }
		if(is_lminion(mtmp) || is_nminion(mtmp) || is_cminion(mtmp)) {
			/* lawful minions don't get cursed, bad, or rusting objects */
			otmp->cursed = FALSE;
			otmp->blessed = TRUE;
			if(otmp->spe < 0) otmp->spe *= -1;
			otmp->oerodeproof = TRUE;
	    }
		if(is_mplayer(mtmp->data) && is_sword(otmp)) {
			otmp->spe = (3 + rn2(4));
	    }
		fix_object(otmp);

	    if(otmp->otyp == CANDELABRUM_OF_INVOCATION) {
			otmp->spe = 0;
			otmp->age = 0L;
			otmp->lamplit = FALSE;
			otmp->blessed = otmp->cursed = FALSE;
	    } else if (otmp->otyp == BELL_OF_OPENING) {
			otmp->blessed = otmp->cursed = FALSE;
	    } else if (otmp->otyp == SPE_BOOK_OF_THE_DEAD) {
			otmp->blessed = FALSE;
			otmp->cursed = TRUE;
	    }

	    /* leaders don't tolerate inferior quality battle gear */
	    if (is_prince(mtmp->data)) {
			if (otmp->oclass == WEAPON_CLASS && otmp->spe < 1)
				otmp->spe = 1;
			else if (otmp->oclass == ARMOR_CLASS && otmp->spe < 0)
				otmp->spe = 0;
	    }

	    spe = otmp->spe;
	    (void) mpickobj(mtmp, otmp);	/* might free otmp */
	    return(spe);
	} else return(0);
}

#endif /* OVL1 */
#ifdef OVLB

int
golemhp(type)
int type;
{
	switch(type) {
		case PM_STRAW_GOLEM: return 20;
		case PM_PAPER_GOLEM: return 20;
		case PM_ROPE_GOLEM: return 30;
		case PM_LEATHER_GOLEM: return 40;
		case PM_GOLD_GOLEM: return 40;
		case PM_WOOD_GOLEM: return 50;
		case PM_LIVING_LECTERN: return 50;
		case PM_GROVE_GUARDIAN: return 60;
		case PM_FLESH_GOLEM: return 40;
		case PM_BRAIN_GOLEM: return 40;
		case PM_DANCING_BLADE: return 49;
		case PM_SPELL_GOLEM: return 20;
//		case PM_SAURON_THE_IMPRISONED: return 45;
		case PM_CLAY_GOLEM: return 50;
		case PM_CHAIN_GOLEM: return 50;
		case PM_TREASURY_GOLEM: return 60;
		case PM_STONE_GOLEM: return 60;
		case PM_GLASS_GOLEM: return 60;
		case PM_ARGENTUM_GOLEM: return 70;
		case PM_IRON_GOLEM: return 80;
		case PM_SEMBLANCE: return 80;
		case PM_ARSENAL: return 88;
		case PM_CENTER_OF_ALL: return 88;
		case PM_RETRIEVER: return 120;
//		case PM_HEAD_OF_THE_UNKNOWN_GOD: return 65;
//		case PM_BODY_OF_THE_UNKNOWN_GOD: return 65;
//		case PM_LEGS_OF_THE_UNKNOWN_GOD: return 65;
//		case PM_EYE_OF_THE_UNKNOWN_GOD: return 65;
//		case PM_SERVANT_OF_THE_UNKNOWN_GOD: return 65;
//		case PM_SCOURGE_OF_THE_UNKNOWN_GOD: return 65;
		default: return 0;
	}
}

#endif /* OVLB */
#ifdef OVL1

/*
 *	Alignment vs. yours determines monster's attitude to you.
 *	( some "animal" types are co-aligned, but also hungry )
 */
boolean
peace_minded(ptr)
register struct permonst *ptr;
{
	int mndx = monsndx(ptr);
	aligntyp mal = ptr->maligntyp, ual = u.ualign.type;
	
	if(Race_if(PM_CLOCKWORK_AUTOMATON) && (mndx == PM_TINKER_GNOME || mndx == PM_HOOLOOVOO) ) return TRUE;
	
	if(mndx == PM_CENTER_OF_ALL && !u.uevent.sum_entered ) return TRUE;
	
	if(Race_if(PM_DROW) && 
		((ual == A_CHAOTIC && (!Role_if(PM_NOBLEMAN) || flags.initgend)) || (ual == A_NEUTRAL && !flags.initgend)) && /*Males can be neutral or chaotic, but a chaotic male nobleman converted to a different god*/
		mndx == PM_AVATAR_OF_LOLTH && 
		strcmp(urole.cgod,"_Lolth") &&
		u.ualign.record >= 20
	) return TRUE;
	
	if(curhouse && 
		(curhouse == u.uhouse || allied_faction(curhouse,u.uhouse))
	) return TRUE;
	
	if(u.uhouse &&
		u.uhouse == EILISTRAEE_SYMBOL
		&& is_elf(ptr) && !is_drow(ptr)
	) return TRUE;
	
	if(u.uhouse &&
		(u.uhouse == XAXOX || u.uhouse == EDDER_SYMBOL)
		&& ptr == &mons[PM_EDDERKOP]
	) return TRUE;
	
	if(u.uhouse &&
		u.uhouse == GHAUNADAUR_SYMBOL
		&& (ptr->mlet == S_PUDDING || ptr->mlet == S_BLOB || ptr->mlet == S_JELLY)
		&& mindless(ptr)
	) return TRUE;
	
	if(u.uhouse &&
		u.uhouse == GHAUNADAUR_SYMBOL
		&& (ptr == &mons[PM_SHOGGOTH] || ptr == &mons[PM_PRIEST_OF_GHAUNADAUR] || ptr == &mons[PM_PRIESTESS_OF_GHAUNADAUR])
	) return TRUE;
	
	if (ptr == &mons[urole.ldrnum] || ptr->msound == MS_GUARDIAN)
		return TRUE;
	if (ptr->msound == MS_NEMESIS)	return FALSE;
	
	if (always_peaceful(ptr)) return TRUE;
	if(!u.uevent.udemigod && mndx==PM_UVUUDAUM && !(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz))) return TRUE;
	
	if(ual == A_VOID) return FALSE;
	
	//Law quest uniques
	if (((mndx <= PM_QUINON && mndx >= PM_MONOTON) || mndx == PM_AXUS) && sgn(mal) == sgn(ual)){
		if(!(u.uevent.uaxus_foe) && u.ualign.record >= 10){
			return TRUE;
		} else return FALSE;
	}
	if (mndx==PM_APOLLYON && u.ualign.record >= 0 && sgn(mal) == sgn(ual)) return TRUE;
	if (mndx==PM_OONA && u.ualign.record >= 20 && u.ualign.sins < 10 && sgn(mal) == sgn(ual)) return TRUE;
	
	//Always hostility, with exception for vampireness and law quest insects
	if (always_hostile(ptr) && 
		(u.uz.dnum != law_dnum || !(is_social_insect(ptr) || is_mercenary(ptr))
		|| (!on_level(&arcadia1_level,&u.uz) && !on_level(&arcadia2_level,&u.uz) && !on_level(&arcadia3_level,&u.uz))
		) && (!is_vampire(ptr) || !is_vampire(youracedata))
		) return FALSE;

	if(Role_if(PM_VALKYRIE) && (mndx==PM_CROW || mndx==PM_RAVEN)) return TRUE;
	
	if (race_peaceful(ptr)) return TRUE;
	if (race_hostile(ptr)) return FALSE;

	/* the monster is hostile if its alignment is different from the
	 * player's */
	if (sgn(mal) != sgn(ual)) return FALSE;

	/* Negative monster hostile to player with Amulet. */
	if (mal < A_NEUTRAL && u.uhave.amulet) return FALSE;

	/* minions are hostile to players that have strayed at all */
	if (is_minion(ptr)) return((boolean)(u.ualign.record >= 0));

	/* Last case:  a chance of a co-aligned monster being
	 * hostile.  This chance is greater if the player has strayed
	 * (u.ualign.record negative) or the monster is not strongly aligned.
	 */
	return((boolean)(!!rn2(16 + (u.ualign.record < -15 ? -15 : u.ualign.record)) &&
		!!rn2(2 + abs(mal))));
}

/* Set malign to have the proper effect on player alignment if monster is
 * killed.  Negative numbers mean it's bad to kill this monster; positive
 * numbers mean it's good.  Since there are more hostile monsters than
 * peaceful monsters, the penalty for killing a peaceful monster should be
 * greater than the bonus for killing a hostile monster to maintain balance.
 * Rules:
 *   it's bad to kill peaceful monsters, potentially worse to kill always-
 *	peaceful monsters
 *   it's never bad to kill a hostile monster, although it may not be good
 */
void
set_malign(mtmp)
struct monst *mtmp;
{
	schar mal = mtmp->data->maligntyp;
	boolean coaligned;

	if (mtmp->ispriest || mtmp->isminion) {
		/* some monsters have individual alignments; check them */
		if (mtmp->ispriest)
			mal = EPRI(mtmp)->shralign;
		else if (mtmp->isminion)
			mal = EMIN(mtmp)->min_align;
		/* unless alignment is none, set mal to -5,0,5 */
		/* (see align.h for valid aligntyp values)     */
		if(mal != A_NONE)
			mal *= 5;
	}

	coaligned = (sgn(mal) == sgn(u.ualign.type));
	if (mtmp->data == &mons[urole.ldrnum]) {
		mtmp->malign = -20;
	} else if (mal == A_NONE) {
		if (mtmp->mpeaceful)
			mtmp->malign = 0;
		else
			mtmp->malign = 20;	/* really hostile */
	} else if (always_peaceful(mtmp->data)) {
		int absmal = abs(mal);
		if (mtmp->mpeaceful)
			mtmp->malign = -3*max(5,absmal);
		else
			mtmp->malign = 3*max(5,absmal); /* renegade */
	} else if (always_hostile(mtmp->data)) {
		int absmal = abs(mal);
		if (coaligned)
			mtmp->malign = 0;
		else
			mtmp->malign = max(5,absmal);
	} else if (coaligned) {
		int absmal = abs(mal);
		if (mtmp->mpeaceful)
			mtmp->malign = -3*max(3,absmal);
		else	/* renegade */
			mtmp->malign = max(3,absmal);
	} else	/* not coaligned and therefore hostile */
		mtmp->malign = abs(mal);
}

#endif /* OVL1 */
#ifdef OVLB

static NEARDATA char syms[] = {
	MAXOCLASSES, MAXOCLASSES+1, RING_CLASS, WAND_CLASS, WEAPON_CLASS,
	FOOD_CLASS, COIN_CLASS, SCROLL_CLASS, POTION_CLASS, ARMOR_CLASS,
	AMULET_CLASS, TOOL_CLASS, ROCK_CLASS, GEM_CLASS, SPBOOK_CLASS,
	S_MIMIC_DEF, S_MIMIC_DEF, S_MIMIC_DEF,
};

void
set_mimic_sym(mtmp)		/* KAA, modified by ERS */
register struct monst *mtmp;
{
	int typ, roomno, rt;
	unsigned appear, ap_type;
	int s_sym;
	struct obj *otmp;
	int mx, my;

	if (!mtmp) return;
	
	mx = mtmp->mx; my = mtmp->my;
	typ = levl[mx][my].typ;
					/* only valid for INSIDE of room */
	roomno = levl[mx][my].roomno - ROOMOFFSET;
	if (roomno >= 0)
		rt = rooms[roomno].rtype;
#ifdef SPECIALIZATION
	else if (IS_ROOM(typ))
		rt = OROOM,  roomno = 0;
#endif
	else	rt = 0;	/* roomno < 0 case for GCC_WARN */

	if (mtmp->data == &mons[PM_DARK_YOUNG]) {
		ap_type = M_AP_FURNITURE;
		appear = S_deadtree;
	} else if (mtmp->data == &mons[PM_SHARAB_KAMEREL]) {
		ap_type = M_AP_FURNITURE;
		appear = S_puddle;
	} else if (OBJ_AT(mx, my)) {
		ap_type = M_AP_OBJECT;
		appear = level.objects[mx][my]->otyp;
	} else if (IS_DOOR(typ) || IS_WALL(typ) ||
		   typ == SDOOR || typ == SCORR) {
		ap_type = M_AP_FURNITURE;
		/*
		 *  If there is a wall to the left that connects to this
		 *  location, then the mimic mimics a horizontal closed door.
		 *  This does not allow doors to be in corners of rooms.
		 */
		if (mx != 0 &&
			(levl[mx-1][my].typ == HWALL    ||
			 levl[mx-1][my].typ == TLCORNER ||
			 levl[mx-1][my].typ == TRWALL   ||
			 levl[mx-1][my].typ == BLCORNER ||
			 levl[mx-1][my].typ == TDWALL   ||
			 levl[mx-1][my].typ == CROSSWALL||
			 levl[mx-1][my].typ == TUWALL    ))
		    appear = S_hcdoor;
		else
		    appear = S_vcdoor;

		if(!mtmp->minvis || See_invisible(mtmp->mx,mtmp->my))
		    block_point(mx,my);	/* vision */
	} else if (level.flags.is_maze_lev && rn2(2)) {
		ap_type = M_AP_OBJECT;
		appear = STATUE;
	} else if (roomno < 0) {
		ap_type = M_AP_OBJECT;
		appear = BOULDER;
		if(!mtmp->minvis || See_invisible(mtmp->mx,mtmp->my))
		    block_point(mx,my);	/* vision */
	} else if (rt == ZOO || rt == VAULT) {
		ap_type = M_AP_OBJECT;
		appear = GOLD_PIECE;
	} else if (rt == DELPHI) {
		if (rn2(2)) {
			ap_type = M_AP_OBJECT;
			appear = STATUE;
		} else {
			ap_type = M_AP_FURNITURE;
			appear = S_fountain;
		}
	} else if (rt == TEMPLE) {
		ap_type = M_AP_FURNITURE;
		appear = S_altar;
	/*
	 * We won't bother with beehives, morgues, barracks, throne rooms
	 * since they shouldn't contain too many mimics anyway...
	 */
	} else if (rt >= SHOPBASE) {
		s_sym = get_shop_item(rt - SHOPBASE);
		if (s_sym < 0) {
			ap_type = M_AP_OBJECT;
			appear = -s_sym;
		} else {
			if (s_sym == RANDOM_CLASS)
				s_sym = syms[rn2((int)sizeof(syms)-2) + 2];
			goto assign_sym;
		}
	} else {
		s_sym = syms[rn2((int)sizeof(syms))];
assign_sym:
		if (s_sym >= MAXOCLASSES) {
			ap_type = M_AP_FURNITURE;
			appear = s_sym == MAXOCLASSES ? S_upstair : S_dnstair;
		} else if (s_sym == COIN_CLASS) {
			ap_type = M_AP_OBJECT;
			appear = GOLD_PIECE;
		} else {
			ap_type = M_AP_OBJECT;
			if (s_sym == S_MIMIC_DEF) {
				appear = STRANGE_OBJECT;
			} else {
				otmp = mkobj( (char) s_sym, FALSE );
				appear = otmp->otyp;
				/* make sure container contents are free'ed */
				obfree(otmp, (struct obj *) 0);
			}
		}
	}
	mtmp->m_ap_type = ap_type;
	mtmp->mappearance = appear;
}

/* release a monster from a bag of tricks */
void
bagotricks(bag)
struct obj *bag;
{
    if (!bag || bag->otyp != BAG_OF_TRICKS) {
	impossible("bad bag o' tricks");
    } else if (bag->spe < 1) {
	pline1(nothing_happens);
    } else {
	boolean gotone = FALSE;
	int cnt = 1;

	consume_obj_charge(bag, TRUE);

	if (!rn2(23)) cnt += rn1(7, 1);
	while (cnt-- > 0) {
	    if (makemon((struct permonst *)0, u.ux, u.uy, NO_MM_FLAGS))
		gotone = TRUE;
	}
	if (gotone) makeknown(BAG_OF_TRICKS);
    }
}

#endif /* OVLB */

/*makemon.c*/
