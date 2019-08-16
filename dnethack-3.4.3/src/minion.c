/*	SCCS Id: @(#)minion.c	3.4	2003/01/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "emin.h"
#include "epri.h"
#include "artifact.h"

extern const int monstr[];

//defined in pray.c
extern const char *Moloch, *Morgoth, *MolochLieutenant, *Silence, *Chaos, *DeepChaos, *tVoid, *Demiurge, *Sophia, *Other, *BlackMother, *DreadFracture, *AllInOne; /*defined in pray*/

void
msummon(mon)		/* mon summons a monster */
struct monst *mon;
{
	register struct permonst *ptr;
	register int dtype = NON_PM, cnt = 0;
	aligntyp atyp;
	struct monst *mtmp;

	/* Wielded Demonbane prevents demons from gating in others. From Sporkhack*/
	if (uwep && uwep->oartifact && spec_ability2(uwep, SPFX2_NOCALL) && is_demon(mon->data)) {
		pline("%s looks puzzled for a moment.",Monnam(mon));
		return;
	}

	if (mon) {
	    ptr = mon->data;
	    atyp = (ptr->maligntyp==A_NONE) ? A_NONE : sgn(ptr->maligntyp);
	    if (mon->ispriest || mon->data == &mons[PM_ALIGNED_PRIEST]
		|| mon->data == &mons[PM_ANGEL])
		atyp = EPRI(mon)->shralign;
	} else {
	    ptr = &mons[PM_WIZARD_OF_YENDOR];
	    atyp = (ptr->maligntyp==A_NONE) ? A_NONE : sgn(ptr->maligntyp);
	}
	if(ptr == &mons[PM_SHAKTARI]) {
	    dtype = PM_MARILITH;
		cnt = d(1,6);
	} else if(ptr == &mons[PM_BAALPHEGOR] && rn2(4)) {
	    dtype = rn2(4) ? PM_METAMORPHOSED_NUPPERIBO : PM_ANCIENT_NUPPERIBO;
		cnt = d(4,4);
	} else if(ptr == &mons[PM_ANCIENT_OF_ICE] || ptr == &mons[PM_ANCIENT_OF_DEATH]) {
	    dtype = rn2(4) ? PM_METAMORPHOSED_NUPPERIBO : PM_ANCIENT_NUPPERIBO;
		cnt = d(1,4);
	} else if (is_dprince(ptr) || (ptr == &mons[PM_WIZARD_OF_YENDOR])) {
	    dtype = (!rn2(20)) ? dprince(ptr, atyp) :
				 (!rn2(4)) ? dlord(ptr, atyp) : ndemon(atyp);
	    cnt = (!rn2(4) && is_ndemon(&mons[dtype])) ? 2 : 1;
	} else if (is_dlord(ptr)) {
	    dtype = (!rn2(50)) ? dprince(ptr, atyp) :
				 (!rn2(20)) ? dlord(ptr, atyp) : ndemon(atyp);
	    cnt = (!rn2(4) && is_ndemon(&mons[dtype])) ? 2 : 1;
	} else if (is_ndemon(ptr)) {
	    dtype = (!rn2(20) && Inhell) ? dlord(ptr, atyp) :
				 ((mons[monsndx(ptr)].geno & G_UNIQ) || !rn2(6)) ? ndemon(atyp) : monsndx(ptr);
	    cnt = 1;
	} else if (is_lminion(mon)) {
		if(is_keter(mon->data)){
			if(mon->data == &mons[PM_MALKUTH_SEPHIRAH] && !rn2(8)) return;
			dtype = PM_MALKUTH_SEPHIRAH;
			cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
		} else if(In_endgame(&u.uz)){
			dtype = (is_lord(ptr) && !rn2(20)) ? llord() :
				 (is_lord(ptr) || (mons[monsndx(ptr)].geno & G_UNIQ) || !rn2(6)) ? lminion() : monsndx(ptr);
			cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
		}
	} else if (is_nminion(mon) && In_endgame(&u.uz)) {
	    dtype = (is_lord(ptr) && !rn2(20)) ? nlord() :
		     (is_lord(ptr) || (mons[monsndx(ptr)].geno & G_UNIQ) || !rn2(6)) ? nminion() : monsndx(ptr);
	    cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
	} else if (is_cminion(mon) && In_endgame(&u.uz)) {
	    dtype = (is_lord(ptr) && !rn2(20)) ? clord() : cminion();
	    cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
	} else if (ptr == &mons[PM_ANGEL]) {
	    if (rn2(6)) {
			(void) summon_god_minion(align_gname_full(atyp), atyp, FALSE);
	    } else {
			dtype = PM_ANGEL;
	    }
	    cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
	}

	if (dtype == NON_PM) return;

	/* sanity checks */
	if (cnt > 1 && (mons[dtype].geno & G_UNIQ)) cnt = 1;
	/*
	 * If this daemon is unique and being re-summoned (the only way we
	 * could get this far with an extinct dtype), try another.
	 */
	if (mvitals[dtype].mvflags & G_GONE && !In_quest(&u.uz)) {
	    dtype = ndemon(atyp);
	    if (dtype == NON_PM) return;
	}
	
	while (cnt > 0) {
	    mtmp = makemon(&mons[dtype], u.ux, u.uy, NO_MM_FLAGS);
	    if (mtmp && (dtype == PM_ANGEL)) {
			/* alignment should match the summoner */
			if(mon->isminion) mtmp->isminion = TRUE;
			EPRI(mtmp)->shralign = atyp;
			mtmp->mpeaceful = mon->mpeaceful;
	    }
	    cnt--;
	}
}

struct monst *
summon_god_minion(gptr, alignment, talk)
const char *gptr;
aligntyp alignment;
boolean talk;
{
    const int *minions = god_minions(gptr);
    int mnum=NON_PM, mlev, num = 0, first, last;
	struct monst *mon;

	mlev = level_difficulty();
	
	for (first = 0; minions[first] != NON_PM; first++)
	    if (!(mvitals[minions[first]].mvflags & G_GONE && !In_quest(&u.uz)) && monstr[minions[first]] > mlev/2) break;
	if(minions[first] == NON_PM){ //All minions too weak, or no minions
		if(first == 0) return (struct monst *) 0;
		else mnum = minions[first-1];
	}
	else for (last = first; minions[last] != NON_PM; last++)
	    if (!(mvitals[minions[last]].mvflags & G_GONE && !In_quest(&u.uz))) {
			/* consider it */
			if(monstr[minions[last]] > mlev+5) break;
			num += min(1,mons[minions[last]].geno & G_FREQ);
	    }

	if(!num){ //All minions too strong, or gap between weak and strong minions
		if(first == 0) return (struct monst *) 0;
		else mnum = minions[first-1];
	}
/*	Assumption:	minions are presented in ascending order of strength. */
	else{
		for(num = rnd(num); num > 0; first++) if (!(mvitals[minions[first]].mvflags & G_GONE && !In_quest(&u.uz))) {
			/* skew towards lower value monsters at lower exp. levels */
			num -= min(1, mons[minions[first]].geno & G_FREQ);
			if (num && adj_lev(&mons[minions[first]]) > (u.ulevel*2)) {
				/* but not when multiple monsters are same level */
				if (mons[first].mlevel != mons[first+1].mlevel)
				num--;
			}
	    }
		first--; /* correct an off-by-one error */
		mnum = minions[first];
	}


    if (mnum == NON_PM) {
		mon = (struct monst *)0;
    } else if (mons[mnum].pxlth == 0) {
		struct permonst *pm = &mons[mnum];
		mon = makemon(pm, u.ux, u.uy, MM_EMIN);
		if (mon) {
			mon->isminion = TRUE;
			EMIN(mon)->min_align = alignment;
		}
    } else if (mnum == PM_ANGEL) {
		mon = makemon(&mons[mnum], u.ux, u.uy, NO_MM_FLAGS);
		if (mon) {
			mon->isminion = TRUE;
			EPRI(mon)->shralign = alignment;	/* always A_LAWFUL here */
		}
    } else
		mon = makemon(&mons[mnum], u.ux, u.uy, NO_MM_FLAGS);
    if (mon) {
		if (talk) {
			pline_The("voice of %s booms:", align_gname(alignment));
			verbalize("Thou shalt pay for thy indiscretion!");
			if (!Blind)
			pline("%s appears before you.", An(Hallucination ? rndmonnam() : mon->data->mname));
		}
		mon->mpeaceful = FALSE;
		/* don't call set_malign(); player was naughty */
		mon->msleeping = 0;
		mon->mcanmove = 1;
		
		/* fix house setting */
		if(is_drow(mon->data)){
			int faction = god_faction(gptr);
			struct obj *otmp;

			mon->mfaction = faction;
			
			for(otmp = mon->minvent; otmp; otmp = otmp->nobj){
				if(otmp->otyp == find_signet_ring() || otmp->otyp == DROVEN_CHAIN_MAIL || otmp->otyp == DROVEN_PLATE_MAIL || otmp->otyp == NOBLE_S_DRESS){
					otmp->oward = faction;
				}
			}
		}
		
		if(gptr == DreadFracture && mon->mfaction != FRACTURED){
			mon->mfaction = FRACTURED;
			mon->m_lev += 4;
			mon->mhpmax = d(mon->m_lev, 8);
			mon->mhp = mon->mhpmax;
			newsym(mon->mx,mon->my);
		}
    }
	
	return mon;
}

struct monst *
summon_minion(alignment, talk, devils, angels)
aligntyp alignment;
boolean talk;
boolean devils;
boolean angels;
{
    register struct monst *mon;
    int mnum;

    switch ((int)alignment) {
	case A_LAWFUL:
	case A_VOID:
	    mnum = devils ? ndemon(alignment) : lminion();
	    break;
	case A_NEUTRAL:
	    mnum = angels ? nminion() : (PM_AIR_ELEMENTAL + rn2(8));
	    break;
	case A_CHAOTIC:
	    mnum = angels ? cminion() : ndemon(alignment);
	    break;
	case A_NONE:
	    mnum = angels ? PM_FALLEN_ANGEL : ndemon(alignment);
	    break;
	default:
//	    impossible("unaligned player?");
		pline("Odd alignment in minion summoning: %d",(int)alignment);
	    mnum = ndemon(A_NONE);
	    break;
    }
    if (mnum == NON_PM) {
		mon = 0;
    } else if (mons[mnum].pxlth == 0) {
		struct permonst *pm = &mons[mnum];
		mon = makemon(pm, u.ux, u.uy, MM_EMIN);
		if (mon) {
			mon->isminion = TRUE;
			EMIN(mon)->min_align = alignment;
		}
    } else if (mnum == PM_ANGEL) {
		mon = makemon(&mons[mnum], u.ux, u.uy, NO_MM_FLAGS);
		if (mon) {
			mon->isminion = TRUE;
			EPRI(mon)->shralign = alignment;	/* always A_LAWFUL here */
		}
    } else
		mon = makemon(&mons[mnum], u.ux, u.uy, NO_MM_FLAGS);
    if (mon) {
	if (talk) {
	    pline_The("voice of %s booms:", align_gname(alignment));
	    verbalize("Thou shalt pay for thy indiscretion!");
	    if (!Blind)
		pline("%s appears before you.", Amonnam(mon));
	}
	mon->mpeaceful = FALSE;
	/* don't call set_malign(); player was naughty */
    }
	return mon;
}

#define Athome	(Inhell && !mtmp->cham)

int
demon_talk(mtmp)		/* returns 1 if it won't attack. */
register struct monst *mtmp;
{
	long cash, demand, offer;

	if (uwep && (
			   uwep->oartifact == ART_EXCALIBUR 
			|| uwep->oartifact == ART_LANCE_OF_LONGINUS
		) ) {
	    pline("%s looks very angry.", Amonnam(mtmp));
	    mtmp->mpeaceful = mtmp->mtame = 0;
	    set_malign(mtmp);
	    newsym(mtmp->mx, mtmp->my);
	    return 0;
	}

	/* Slight advantage given. */
	if ((is_dprince(mtmp->data) || is_dlord(mtmp->data)) && mtmp->minvis) {
	    mtmp->minvis = mtmp->perminvis = 0;
	    if (!Blind) pline("%s appears before you.", Amonnam(mtmp));
	    newsym(mtmp->mx,mtmp->my);
	}
	if (youracedata->mlet == S_DEMON) {	/* Won't blackmail their own. */
	    pline("%s says, \"Good hunting, %s.\"",
		  Amonnam(mtmp), flags.female ? "Sister" : "Brother");
	    if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
	    return(1);
	}
#ifndef GOLDOBJ
	cash = u.ugold;
#else
	cash = money_cnt(invent);
#endif
	demand = (cash * (rnd(80) + 20 * Athome)) /
	    (100 * (1 + (sgn(u.ualign.type) == sgn(mtmp->data->maligntyp))));

	if (!demand) {		/* you have no gold */
	    mtmp->mpeaceful = 0;
	    set_malign(mtmp);
	    return 0;
	} else {
		if(mtmp->data == &mons[PM_ASMODEUS] && demand < 9000) demand = 9000 + rn2(1000);
		else if(demand < 2000) demand = max(1000+rnd(1000), demand); //demons can't be bribed with chump change.
	    /* make sure that the demand is unmeetable if the monster
	       has the Amulet, preventing monster from being satisified
	       and removed from the game (along with said Amulet...) */
	    if (mon_has_amulet(mtmp))
		demand = cash + (long)rn1(1000,40);

	    pline("%s demands %ld %s for safe passage.",
		  Amonnam(mtmp), demand, currency(demand));

	    if ((offer = bribe(mtmp)) >= demand) {
		pline("%s vanishes, laughing about cowardly mortals.",
		      Amonnam(mtmp));
	    } else if (offer > 0L && (long)rnd(40) > (demand - offer)) {
		pline("%s scowls at you menacingly, then vanishes.",
		      Amonnam(mtmp));
	    } else {
		pline("%s gets angry...", Amonnam(mtmp));
		mtmp->mpeaceful = 0;
		set_malign(mtmp);
		return 0;
	    }
	}
	mongone(mtmp);
	return(1);
}

long
bribe(mtmp)
struct monst *mtmp;
{
	char buf[BUFSZ];
	long offer;
#ifdef GOLDOBJ
	long umoney = money_cnt(invent);
#endif

	getlin("How much will you offer?", buf);
	if (sscanf(buf, "%ld", &offer) != 1) offer = 0L;

	/*Michael Paddon -- fix for negative offer to monster*/
	/*JAR880815 - */
	if (offer < 0L) {
		You("try to shortchange %s, but fumble.",
			mon_nam(mtmp));
		return 0L;
	} else if (offer == 0L) {
		You("refuse.");
		return 0L;
#ifndef GOLDOBJ
	} else if (offer >= u.ugold) {
		You("give %s all your gold.", mon_nam(mtmp));
		offer = u.ugold;
	} else {
		You("give %s %ld %s.", mon_nam(mtmp), offer, currency(offer));
	}
	u.ugold -= offer;
	mtmp->mgold += offer;
#else
	} else if (offer >= umoney) {
		You("give %s all your gold.", mon_nam(mtmp));
		offer = umoney;
	} else {
		You("give %s %ld %s.", mon_nam(mtmp), offer, currency(offer));
	}
	(void) money2mon(mtmp, offer);
#endif
	flags.botl = 1;
	return(offer);
}

int demonPrinces[] = {
	PM_DEMOGORGON,
	PM_LAMASHTU,
	PM_OBOX_OB,
	PM_DAGON,
	PM_PALE_NIGHT,
	PM_ORCUS,
	PM_GRAZ_ZT,
	PM_MALCANTHET
};

int
dprince(ptr, atyp)
struct permonst *ptr;
aligntyp atyp;
{
	int tryct, pm;
	
	/*Specific aliances go here*/
	if(ptr == &mons[PM_BAPHOMET] && !(mvitals[PM_PALE_NIGHT].mvflags & G_GONE))
		return PM_PALE_NIGHT;
	if(ptr == &mons[PM_PALE_NIGHT] && !(mvitals[PM_ASCODEL].mvflags & G_GONE) && !rn2(4))
		return PM_ASCODEL;
	if(ptr == &mons[PM_PALE_NIGHT] && !(mvitals[PM_GRAZ_ZT].mvflags & G_GONE))
		return PM_GRAZ_ZT;
	if(ptr == &mons[PM_DEMOGORGON] && !(mvitals[PM_DAGON].mvflags & G_GONE))
		return PM_DAGON;

	if(atyp == A_NONE) atyp = !rn2(3) ? A_LAWFUL : A_CHAOTIC;
	
	if(atyp == A_LAWFUL){
		if(!(mvitals[PM_ASMODEUS].mvflags & G_GONE)) return PM_ASMODEUS;
		else return dlord(ptr, atyp);
	} else if(atyp == A_CHAOTIC) {
		for (tryct = 0; tryct < 20; tryct++) {
			pm = demonPrinces[rn2(SIZE(demonPrinces))];
			if (!(mvitals[pm].mvflags & G_GONE
				|| (ptr == &mons[PM_GRAZ_ZT] && pm == PM_MALCANTHET)
				|| (ptr == &mons[PM_MALCANTHET] && pm == PM_GRAZ_ZT)
				|| (ptr == &mons[PM_OBOX_OB] && pm == PM_DEMOGORGON)
				|| (ptr == &mons[PM_DEMOGORGON] && pm == PM_OBOX_OB)
				|| (ptr == &mons[PM_LAMASHTU] && pm == PM_DEMOGORGON)
				|| (ptr == &mons[PM_DEMOGORGON] && pm == PM_LAMASHTU)
			))
				return(pm);
		}
	}
	return(dlord(ptr, atyp));	/* approximate */
}

int demonLords[] = {
	PM_YEENOGHU,
	PM_BAPHOMET,
	PM_KOSTCHTCHIE,
	PM_ZUGGTMOY,
	PM_JUIBLEX,
	PM_ALRUNES,
	PM_ALDINACH
};

int lordsOfTheNine[] = {
	PM_MEPHISTOPHELES,
	PM_BAALZEBUB,
	PM_CRONE_LILITH,
	PM_CREATURE_IN_THE_ICE,
	PM_BELIAL,
	PM_MAMMON,
	PM_DISPATER,
	PM_BAEL
};

int
dlord(ptr, atyp)
struct permonst *ptr;
aligntyp atyp;
{
	int tryct, pm;
	
	/*Specific aliances go here*/
	if(ptr == &mons[PM_MEPHISTOPHELES] && !(mvitals[PM_BAALPHEGOR].mvflags & G_GONE))
		return PM_BAALPHEGOR;
	if(ptr == &mons[PM_CRONE_LILITH] && !(mvitals[PM_MOTHER_LILITH].mvflags & G_GONE))
		return PM_MOTHER_LILITH;
	if(ptr == &mons[PM_CRONE_LILITH] && !(mvitals[PM_DAUGHTER_LILITH].mvflags & G_GONE))
		return PM_DAUGHTER_LILITH;
	if(ptr == &mons[PM_MOTHER_LILITH] && !(mvitals[PM_DAUGHTER_LILITH].mvflags & G_GONE))
		return PM_DAUGHTER_LILITH;
	if(ptr == &mons[PM_BELIAL] && !(mvitals[PM_FIERNA].mvflags & G_GONE))
		return PM_FIERNA;
	if(ptr == &mons[PM_MAMMON] && !(mvitals[PM_GLASYA].mvflags & G_GONE) && !rn2(20))
		return PM_GLASYA;
	
	if(ptr == &mons[PM_PALE_NIGHT] && !(mvitals[PM_BAPHOMET].mvflags & G_GONE))
		return PM_BAPHOMET;
	if(ptr == &mons[PM_OBOX_OB] && !(mvitals[PM_ALDINACH].mvflags & G_GONE))
		return PM_ALDINACH;

	if(atyp == A_NONE) atyp = rn2(2) ? A_LAWFUL : A_CHAOTIC;

	if(atyp == A_LAWFUL){
		for (tryct = 0; tryct < 20; tryct++) {
			pm = lordsOfTheNine[rn2(SIZE(lordsOfTheNine))];
			if(pm == PM_CRONE_LILITH) pm = !rn2(3) ? PM_CRONE_LILITH : !rn2(2) ? PM_MOTHER_LILITH : PM_DAUGHTER_LILITH;
			if (!(mvitals[pm].mvflags & G_GONE
				|| (ptr == &mons[PM_MEPHISTOPHELES] && pm == PM_BAALZEBUB)
				|| (ptr == &mons[PM_BAALZEBUB] && pm == PM_MEPHISTOPHELES)
			)){
				if(pm == PM_CREATURE_IN_THE_ICE){
					mvitals[pm].mvflags |= G_GONE;
					pm = rn2(2) ? PM_LEVISTUS : PM_LEVIATHAN;
				}
				return(pm);
			}
		}
	} else if(atyp == A_CHAOTIC) {
		for (tryct = 0; tryct < 20; tryct++) {
			pm = demonLords[rn2(SIZE(demonLords))];
			if (!(mvitals[pm].mvflags & G_GONE
				|| (ptr == &mons[PM_BAPHOMET] && pm == PM_YEENOGHU)
				|| (ptr == &mons[PM_YEENOGHU] && pm == PM_BAPHOMET)
				|| (ptr == &mons[PM_ZUGGTMOY] && pm == PM_JUIBLEX)
				|| (ptr == &mons[PM_JUIBLEX] && pm == PM_ZUGGTMOY)
			))
				return(pm);
		}
	}
	
	return(ndemon(atyp));	/* approximate */
}

/* create lawful (good) lord */
int
llord()
{
	switch(rnd(2)){
	case 1:
		if (!(mvitals[PM_THRONE_ARCHON].mvflags & G_GONE && !In_quest(&u.uz)))
			return(PM_THRONE_ARCHON);
	break;
	case 2:
		if (!(mvitals[PM_LIGHT_ARCHON].mvflags & G_GONE && !In_quest(&u.uz)))
			return(PM_LIGHT_ARCHON);
	break;
	}
	
	return(lminion());	/* approximate */
}

int
lminion()
{
	int	tryct;
	struct	permonst *ptr;

	for (tryct = 0; tryct < 20; tryct++) {
	    ptr = mkclass(S_LAW_ANGEL,G_NOHELL|G_HELL|G_PLANES);
	    if (ptr && !is_lord(ptr) && is_angel(ptr))
			return(monsndx(ptr));
	}

	return NON_PM;
}

/* create neutral (good) lord */
int
nlord()
{
	switch(rnd(4)){
	case 1:
		if (!(mvitals[PM_MAHADEVA].mvflags & G_GONE && !In_quest(&u.uz)))
			return(PM_MAHADEVA);
	break;
	case 2:
	case 3:
	case 4:
		if (!(mvitals[PM_SURYA_DEVA].mvflags & G_GONE && !In_quest(&u.uz)))
			return(PM_SURYA_DEVA);
	break;
	}
	
	return(nminion());	/* approximate */
}

int
nminion()
{
	int	tryct;
	struct	permonst *ptr;

	for (tryct = 0; tryct < 20; tryct++) {
	    ptr = mkclass(S_NEU_ANGEL,G_NOHELL|G_HELL|G_PLANES);
	    if (ptr && !is_lord(ptr) && is_angel(ptr))
			return(monsndx(ptr));
	}

	return NON_PM;
}

/* create chaotic (good) lord */
int
clord()
{
	if (!(mvitals[PM_TULANI_ELADRIN].mvflags & G_GONE && !In_quest(&u.uz)))
		return(PM_TULANI_ELADRIN);

	return(cminion());	/* approximate */
}

int
cminion()
{
	int	tryct;
	struct	permonst *ptr;

	for (tryct = 0; tryct < 20; tryct++) {
	    ptr = mkclass(S_CHA_ANGEL,G_NOHELL|G_HELL|G_PLANES);
	    if (ptr && !is_lord(ptr) && is_angel(ptr))
			return(monsndx(ptr));
	}

	return NON_PM;
}

int
ndemon(atyp)
aligntyp atyp;
{
	int	tryct;
	struct	permonst *ptr;
	
	for (tryct = 0; tryct < 20; tryct++) {
	    ptr = mkclass(S_DEMON, G_NOHELL|G_HELL);
	    if (ptr && is_ndemon(ptr) &&
		    (atyp == A_NONE || sgn(ptr->maligntyp) == sgn(atyp)))
		return(monsndx(ptr));
	}

	return NON_PM;
}

/*minion.c*/
