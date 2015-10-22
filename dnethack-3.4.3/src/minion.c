/*	SCCS Id: @(#)minion.c	3.4	2003/01/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "emin.h"
#include "epri.h"
#include "artifact.h"

extern const int monstr[];

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
	} else if (is_dprince(ptr) || (ptr == &mons[PM_WIZARD_OF_YENDOR])) {
	    dtype = (!rn2(20)) ? dprince(atyp) :
				 (!rn2(4)) ? dlord(atyp) : ndemon(atyp);
	    cnt = (!rn2(4) && is_ndemon(&mons[dtype])) ? 2 : 1;
	} else if (is_dlord(ptr)) {
	    dtype = (!rn2(50)) ? dprince(atyp) :
				 (!rn2(20)) ? dlord(atyp) : ndemon(atyp);
	    cnt = (!rn2(4) && is_ndemon(&mons[dtype])) ? 2 : 1;
	} else if (is_ndemon(ptr)) {
	    dtype = (!rn2(20) && Inhell) ? dlord(atyp) :
				 ((mons[monsndx(ptr)].geno & G_UNIQ) || !rn2(6)) ? ndemon(atyp) : monsndx(ptr);
	    cnt = 1;
	} else if (is_lminion(mon)) {
	    dtype = (is_lord(ptr) && !rn2(20)) ? llord() :
		     (is_lord(ptr) || (mons[monsndx(ptr)].geno & G_UNIQ) || !rn2(6)) ? lminion() : monsndx(ptr);
	    cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
	} else if (is_nminion(mon)) {
	    dtype = (is_lord(ptr) && !rn2(20)) ? nlord() :
		     (is_lord(ptr) || (mons[monsndx(ptr)].geno & G_UNIQ) || !rn2(6)) ? nminion() : monsndx(ptr);
	    cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
	} else if (is_cminion(mon)) {
	    dtype = (is_lord(ptr) && !rn2(20)) ? clord() : cminion();
	    cnt = (!rn2(4) && !is_lord(&mons[dtype])) ? 2 : 1;
	} else if (ptr == &mons[PM_ANGEL]) {
	    /* non-lawful angels can also summon */
	    if (!rn2(6)) {
		switch (atyp) { /* see summon_minion */
		case A_NEUTRAL:
		    dtype = PM_AIR_ELEMENTAL + rn2(8);
		    break;
		case A_CHAOTIC:
		case A_NONE:
		    dtype = ndemon(atyp);
		    break;
		}
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
		EPRI(mtmp)->shralign = atyp;
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
    }
	
	if(is_drow(mon->data)){
		/* fix house setting */
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
	if (is_dprince(mtmp->data) && mtmp->minvis) {
	    mtmp->minvis = mtmp->perminvis = 0;
	    if (!Blind) pline("%s appears before you.", Amonnam(mtmp));
	    newsym(mtmp->mx,mtmp->my);
	}
	if (youmonst.data->mlet == S_DEMON) {	/* Won't blackmail their own. */
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

int
dprince(atyp)
aligntyp atyp;
{
	int tryct, pm;

	for (tryct = 0; tryct < 20; tryct++) {
	    pm = rn1(PM_DEMOGORGON + 1 - PM_ORCUS, PM_ORCUS);
	    if (!(mvitals[pm].mvflags & G_GONE && !In_quest(&u.uz)) &&
		    (atyp == A_NONE || sgn(mons[pm].maligntyp) == sgn(atyp)))
		return(pm);
	}
	return(dlord(atyp));	/* approximate */
}

int
dlord(atyp)
aligntyp atyp;
{
	int tryct, pm;

	for (tryct = 0; tryct < 20; tryct++) {
	    do pm = rn1(PM_YEENOGHU + 1 - PM_JUIBLEX, PM_JUIBLEX);
		while(pm == PM_SHAKTARI);
	    if (!(mvitals[pm].mvflags & G_GONE && !In_quest(&u.uz)) &&
		    (atyp == A_NONE || sgn(mons[pm].maligntyp) == sgn(atyp)))
		return(pm);
	}
	
	return(ndemon(atyp));	/* approximate */
}

/* create lawful (good) lord */
int
llord()
{
	if (!(mvitals[PM_ARCHON].mvflags & G_GONE && !In_quest(&u.uz)))
		return(PM_ARCHON);

	return(lminion());	/* approximate */
}

int
lminion()
{
	int	tryct;
	struct	permonst *ptr;

	for (tryct = 0; tryct < 20; tryct++) {
	    ptr = mkclass(S_ANGEL,G_NOHELL|G_HELL);
	    if (ptr && !is_lord(ptr))
		return(monsndx(ptr));
	}

	return NON_PM;
}

/* create neutral (good) lord */
int
nlord()
{
	if (!(mvitals[PM_MAHADEVA].mvflags & G_GONE && !In_quest(&u.uz)))
		return(PM_MAHADEVA);

	return(nminion());	/* approximate */
}

int
nminion()
{
	switch(rnd(3)){
		case 1: return PM_MOVANIC_DEVA;
		case 2: return PM_MONADIC_DEVA;
		case 3: return PM_ASTRAL_DEVA;
	}
}

/* create chaotic (good) lord */
int
clord()
{
	if (!(mvitals[PM_TULANI].mvflags & G_GONE && !In_quest(&u.uz)))
		return(PM_TULANI);

	return(cminion());	/* approximate */
}

int
cminion()
{
	switch(rnd(5)){
		case 1: return PM_NOVIERE;
		case 2: return PM_BRALANI;
		case 3: return PM_FIRRE;
		case 4: return PM_SHIERE;
		case 5: return PM_GHAELE;
	}
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
