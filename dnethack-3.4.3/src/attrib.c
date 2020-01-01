/*	SCCS Id: @(#)attrib.c	3.4	2002/10/07	*/
/*	Copyright 1988, 1989, 1990, 1992, M. Stephenson		  */
/* NetHack may be freely redistributed.  See license for details. */

/*  attribute modification routines. */

#include <limits.h>
#include "math.h"
#include "hack.h"

/* #define DEBUG */	/* uncomment for debugging info */

#ifdef OVLB

	/* part of the output on gain or loss of attribute */
static
const char	* const plusattr[] = {
	"strong", "smart", "wise", "agile", "tough", "charismatic"
},
		* const minusattr[] = {
	"weak", "stupid", "foolish", "clumsy", "fragile", "repulsive"
};


static
const struct innate {
	schar	ulevel;
	long	*ability;
	const char *gainstr, *losestr;
}
	arc_abil[] = { {	 1, &(HStealth), "", "" },
		     {   1, &(HFast), "", "" },
		     {  10, &(HSearching), "perceptive", "" },
		     {	 0, 0, 0, 0 } },

	ana_abil[] = { {   7, &(HFast), "quick", "slow" },
		     {	15, &(HWarning), "precognitive", "" },
		     {	 0, 0, 0, 0 } },

	bar_abil[] = { {	 1, &(HPoison_resistance), "", "" },
		     {   7, &(HFast), "quick", "slow" },
		     {  15, &(HStealth), "stealthy", "" },
		     {	 0, 0, 0, 0 } },
#ifdef BARD
	brd_abil[] = { {	5, &(HSleep_resistance), "awake", "tired" },
		     {	 10, &(HStealth), "stealthy", "noisy" },
		     {	 0, 0, 0, 0 } },
#endif
	bin_abil[] = {
		     {	 0, 0, 0, 0 } },

	cav_abil[] = { {	 7, &(HFast), "quick", "slow" },
		     {	15, &(HWarning), "sensitive", "" },
		     {	 0, 0, 0, 0 } },

#ifdef CONVICT
	con_abil[] = { {   1, &(HSick_resistance), "", "" },
	         {	 7, &(HPoison_resistance), "healthy", "" },
		     {  20, &(HSearching), "perceptive", "unaware" },
		     {	 0, 0, 0, 0 } },
#endif	/* CONVICT */

	hea_abil[] = { {	 1, &(HPoison_resistance), "", "" },
		     {	 1, &(HSick_resistance), "", "" },
		     {	15, &(HWarning), "sensitive", "" },
		     {	 0, 0, 0, 0 } },

	kni_abil[] = { {	 7, &(HFast), "quick", "slow" },
		     {	 0, 0, 0, 0 } },

	mon_abil[] = { {   1, &(HFast), "", "" },
		     {   1, &(HSleep_resistance), "", "" },
		     {   1, &(HSee_invisible), "", "" },
		     {   3, &(HPoison_resistance), "healthy", "sickly" },
		     {   5, &(HStealth), "stealthy", "noisy" },
		     {   7, &(HWarning), "sensitive", "insensitive" },
		     {   9, &(HSearching), "perceptive", "unaware" },
		     {  11, &(HFire_resistance), "cool", "warmer" },
		     {  13, &(HCold_resistance), "warm", "cooler" },
		     {  15, &(HShock_resistance), "insulated", "conductive" },
		     {  17, &(HTeleport_control), "controlled","uncontrolled" },
		     {  19, &(HAcid_resistance), "thick-skinned","soft-skinned" },
		     {  21, &(HWwalking), "light on your feet","heavy" },
		     {  23, &(HSick_resistance), "immunized","immunocompromised" },
		     {  25, &(HDisint_resistance), "firm","less firm" },
		     {  27, &(HStone_resistance), "limber","stiff" },
		     {  29, &(HAntimagic), "skeptical","credulous" },
		     {  30, &(HDrain_resistance), "above earthly concerns","not so above it all" },
		     {   0, 0, 0, 0 } },

	elnob_abil[] = { {	 7, &(HFast), "quick", "slow" },
			{	15, &(HWarning), "sensitive", "" },
		     {  20, &(HFire_resistance), "cool", "warmer" },
		     {	 0, 0, 0, 0 } },

	pir_abil[] = {	{1, &(HSwimming), "", ""  },
			 {	7, &(HStealth), "stealthy", ""  },	/* with cat-like tread ... */
		     {  11, &(HFast), "quick", "slow" },
		     {	 0, 0, 0, 0 } },

	pri_abil[] = { {	15, &(HWarning), "sensitive", "" },
		     {  20, &(HFire_resistance), "cool", "warmer" },
		     {	 0, 0, 0, 0 } },

	ran_abil[] = { {   1, &(HSearching), "", "" },
		     {	 7, &(HStealth), "stealthy", "" },
		     {	15, &(HSee_invisible), "", "" },
		     {	 0, 0, 0, 0 } },

	rog_abil[] = { {	 1, &(HStealth), "", ""  },
		     {  10, &(HSearching), "perceptive", "" },
		     {	 0, 0, 0, 0 } },

	sam_abil[] = { {	 1, &(HFast), "", "" },
		     {  15, &(HStealth), "stealthy", "" },
		     {	 0, 0, 0, 0 } },

	tou_abil[] = { {	10, &(HSearching), "perceptive", "" },
		     {	20, &(HPoison_resistance), "hardy", "" },
		     {	 0, 0, 0, 0 } },

	val_abil[] = { {	 1, &(HCold_resistance), "", "" },
		     {	 1, &(HStealth), "", "" },
		     {   7, &(HFast), "quick", "slow" },
		     {	 0, 0, 0, 0 } },

	wiz_abil[] = { {	15, &(HWarning), "sensitive", "" },
		     {  17, &(HTeleport_control), "controlled","uncontrolled" },
		     {	 0, 0, 0, 0 } },

	/* Intrinsics conferred by race */
	elf_abil[] = { {	4, &(HSleep_resistance), "awake", "tired" },
		     {	 0, 0, 0, 0 } },

	orc_abil[] = { {	1, &(HPoison_resistance), "", "" },
		     {  15, &(HAntimagic), "magic resistant","magic-sensitive" },
		     {	 0, 0, 0, 0 } },

	clk_abil[] = { {	1, &(HPoison_resistance), "", "" },
		     {	 1, &(HSick_resistance), "", "" },
		     {	 1, &(HStone_resistance), "", "" },
		     {	 5, &(HShock_resistance), "shock resistant", "less shock resistant" },
		     {	 10, &(HCold_resistance), "cold resistant", "less cold resistant" },
		     {	 15, &(HFire_resistance), "heat resistant", "less heat resistant" },
		     {	 0, 0, 0, 0 } },

	and_abil[] = { {	1, &(HPoison_resistance), "", "" },
		     {	 1, &(HSick_resistance), "", "" },
		     {	 1, &(HStone_resistance), "", "" },
		     {	 1, &(HCold_resistance), "", "" },
		     {   5, &(HStealth), ">Initiating short-range camouflage<", ">Short-range camouflage damaged<" },
		     {  10, &(HShock_resistance), ">Initiating ion-channel re-direction<", ">Ion-channel re-direction non-operational<" },
		     {  15, &(HFire_resistance), ">Activating asymmetrical heat sink<", ">Asymmetrical heat sink destroyed<" },
		     {	 0, 0, 0, 0 } },

	vam_abil[] = { {	1, &(HPoison_resistance), "", "" },
			 {	 1, &(HSleep_resistance), "", "" },
			 {	11, &(HCold_resistance), "the chill of the grave", "the warmth of life" },
		     {	 21, &(HPolymorph_control), "in control", "out of control" },
		     {	 0, 0, 0, 0 } },

	hlf_abil[] = { {	14, &(HFlying), "wings sprout from your back", "your wings shrivel and die" },
		     {	 0, 0, 0, 0 } },

	yki_abil[] = { {	1, &(HCold_resistance), "", "" },
		     {  11, &(HFire_resistance), "cool", "warmer" },
		     {	 0, 0, 0, 0 } },

	inc_abil[] = { {	1, &(HAntimagic), "", "" },
		     {	 0, 0, 0, 0 } };

#define	next_check u.exerchkturn
STATIC_DCL void NDECL(exerper);
STATIC_DCL void FDECL(postadjabil, (long *));

/* adjust an attribute; return TRUE if change is made, FALSE otherwise */
boolean
adjattrib(ndx, incr, msgflg)
	int	ndx, incr;
	int	msgflg;	    /* positive => no message, zero => message, and */
{			    /* negative => conditional (msg if change made) */
	if (Fixed_abil || !incr) return FALSE;

	if ((ndx == A_INT || ndx == A_WIS)
				&& uarmh && uarmh->otyp == DUNCE_CAP) {
		if (msgflg == 0)
		    Your("cap constricts briefly, then relaxes again.");
		return FALSE;
	}

	if (incr > 0) {
	    if ((AMAX(ndx) >= ATTRMAX(ndx)) && (ACURR(ndx) >= AMAX(ndx))) {
		if (msgflg == 0 && flags.verbose)
		    pline("You're already as %s as you can get.",
			  plusattr[ndx]);
		ABASE(ndx) = AMAX(ndx) = ATTRMAX(ndx); /* just in case */
		return FALSE;
	    }

	    ABASE(ndx) += incr;
	    if(ABASE(ndx) > AMAX(ndx)) {
		incr = ABASE(ndx) - AMAX(ndx);
		AMAX(ndx) += incr;
		if(AMAX(ndx) > ATTRMAX(ndx))
		    AMAX(ndx) = ATTRMAX(ndx);
		ABASE(ndx) = AMAX(ndx);
	    }
	} else {
	    if (ABASE(ndx) <= ATTRMIN(ndx)) {
			if(ndx == A_WIS && u.wimage >= 10){
				int temparise = u.ugrave_arise;
				pline("The image of the weeping angel is taking over your body!");
				u.ugrave_arise = PM_WEEPING_ANGEL;
				done(WEEPING);
				u.ugrave_arise = temparise;
			} else if(u.umorgul
				&& (ndx == A_WIS || ndx == A_CON || ndx == A_CHA)
				&& ABASE(A_WIS) <= ATTRMIN(A_WIS)
				&& ABASE(A_CON) <= ATTRMIN(A_CON)
				&& ABASE(A_CHA) <= ATTRMIN(A_CHA)
			){
				int temparise = u.ugrave_arise;
				You_feel("something cold pierce your %s!", body_part(HEART));
				u.ugrave_arise = PM_WRAITH;
				killer_format = KILLED_BY;
				killer = "a shard of a morgul-blade";
				done(DIED);
				u.ugrave_arise = temparise;
			} else {
				if (msgflg == 0 && flags.verbose)
					pline("You're already as %s as you can get.",
					  minusattr[ndx]);
			}
			ABASE(ndx) = ATTRMIN(ndx); /* just in case */
			return FALSE;
	    }

	    ABASE(ndx) += incr;
	    if(ABASE(ndx) < ATTRMIN(ndx)) {
		incr = ABASE(ndx) - ATTRMIN(ndx);
		ABASE(ndx) = ATTRMIN(ndx);
		AMAX(ndx) += incr;
		if(AMAX(ndx) < ATTRMIN(ndx))
		    AMAX(ndx) = ATTRMIN(ndx);
	    }
	}
	if (msgflg <= 0)
	    You_feel("%s%s!",
		  (incr > 1 || incr < -1) ? "very ": "",
		  (incr > 0) ? plusattr[ndx] : minusattr[ndx]);
	flags.botl = 1;
	if (moves > 1 && (ndx == A_STR || ndx == A_CON))
		(void)encumber_msg();
	return TRUE;
}

void
gainstr(otmp, incr)
	register struct obj *otmp;
	register int incr;
{
	int num = 1;

	if(incr) num = incr;
	else {
	    if(ABASE(A_STR) < 18) num = (rn2(4) ? 1 : rnd(6) );
	    else if (ABASE(A_STR) < STR18(85)) num = rnd(10);
	}
	(void) adjattrib(A_STR, (otmp && otmp->cursed) ? -num : num, TRUE);
}

void
losestr(num)	/* may kill you; cause may be poison or monster like 'a' */
	register int num;
{
	int ustr = ABASE(A_STR) - num;

	while(ustr < 3) {
	    ++ustr;
	    --num;
	    if (Upolyd) {
		u.mh -= 6;
	    } else {
		u.uhp -= 6;
	    }
		u.uhpmod -= 6;
	}
	calc_total_maxhp();
	(void) adjattrib(A_STR, -num, TRUE);
}

void
change_luck(n)
	register schar n;
{
	u.uluck += n;
	if (u.uluck < 0 && u.uluck < LUCKMIN)	u.uluck = LUCKMIN;
	if (u.uluck > 0 && u.uluck > LUCKMAX)	u.uluck = LUCKMAX;
}

int
stone_luck(parameter)
boolean parameter; /* So I can't think up of a good name.  So sue me. --KAA */
{
	register struct obj *otmp;
	register long bonchance = 0;

	for (otmp = invent; otmp; otmp = otmp->nobj)
	    if (confers_luck(otmp)) {
		if (otmp->cursed) bonchance -= otmp->quan;
		else if (otmp->blessed) bonchance += otmp->quan;
		else if (parameter) bonchance += otmp->quan;
	    }
// #ifdef CONVICT
	// if(Role_if(PM_CONVICT)) bonchance -= (u.ulevel-1)/10 + 1; /* a Convict's karmic burden becomes 
																// only more heavy as they level up */
// #endif	/* CONVICT */
	return (int)bonchance;
}

boolean
has_luckitem()
{
	register struct obj *otmp;

	for (otmp = invent; otmp; otmp = otmp->nobj)
	    if (confers_luck(otmp)) return TRUE;
	return FALSE;
}

/* there has just been an inventory change affecting a luck-granting item */
void
set_moreluck()
{
	int stoneluck = stone_luck(TRUE);
	if (!has_luckitem()) u.moreluck = 0;
	else if (stoneluck >= 0){
		if(Role_if(PM_CONVICT) && stoneluck < LUCKADD)
			u.moreluck = stoneluck;
		else u.moreluck = LUCKADD;
	} else u.moreluck = -LUCKADD;
}

#endif /* OVLB */
#ifdef OVL1

void
restore_attrib()
{
	int	i;

	for(i = 0; i < A_MAX; i++) {	/* all temporary losses/gains */

	   if(ATEMP(i) && ATIME(i)) {
		if(!(--(ATIME(i)))) { /* countdown for change */
		    ATEMP(i) += ATEMP(i) > 0 ? -1 : 1;

		    if(ATEMP(i)) /* reset timer */
			ATIME(i) = 100 / ACURR(A_CON);
		}
	   }
	}
	(void)encumber_msg();
}

#endif /* OVL1 */
#ifdef OVLB

#define AVAL	50		/* tune value for exercise gains */

void
exercise(i, inc_or_dec)
int	i;
boolean	inc_or_dec;
{
#ifdef DEBUG
	pline("Exercise:");
#endif
	if (i == A_CHA) return;	/* can't exercise cha */
	if(umechanoid) return; /* Mechanoids can't excercise abilities */
	if(u.sealsActive&SEAL_HUGINN_MUNINN && (i == A_INT || i == A_WIS)) return; /* don't excercise int or wis while artificially maxed */
	
	/* no physical exercise while polymorphed; the body's temporary */
	if (Upolyd && i != A_WIS && i != A_INT) return;

	if(abs(AEXE(i)) < AVAL) {
		/*
		 *	Law of diminishing returns (Part I):
		 *
		 *	Gain is harder at higher attribute values.
		 *	79% at "3" --> 0% at "18"
		 *	Loss is even at all levels (50%).
		 *
		 *	Note: *YES* ACURR is the right one to use.
		 */
		AEXE(i) += (inc_or_dec) ? (rn2(19) > ACURR(i)) : -rn2(2);
#ifdef DEBUG
		pline("%s, %s AEXE = %d",
			(i == A_STR) ? "Str" : (i == A_WIS) ? "Wis" :
			(i == A_DEX) ? "Dex" : (i == A_INT) ? "Int" : "Con",
			(inc_or_dec) ? "inc" : "dec", AEXE(i));
#endif
	}
	if (moves > 0 && (i == A_STR || i == A_CON)) (void)encumber_msg();
}

/* hunger values - from eat.c */
#define SATIATED	0
#define NOT_HUNGRY	1
#define HUNGRY		2
#define WEAK		3
#define FAINTING	4
#define FAINTED		5
#define STARVED		6

STATIC_OVL void
exerper()
{
	if(!(moves % 10)) {
		/* Hunger Checks */

		int hs = (YouHunger > (Race_if(PM_INCANTIFIER) ? max(u.uenmax/2,200) : 1000)) ? SATIATED :
			 (YouHunger > 150) ? NOT_HUNGRY :
			 (YouHunger > 50) ? HUNGRY :
			 (YouHunger > 0) ? WEAK : FAINTING;

#ifdef DEBUG
		pline("exerper: Hunger checks");
#endif
		switch (hs) {
		    case SATIATED:	if(!is_vampire(youracedata) && !Race_if(PM_INCANTIFIER))  /* undead/magic metabolism */
							exercise(A_DEX, FALSE);
					if (Role_if(PM_MONK))
					    exercise(A_WIS, FALSE);
					break;
		    case NOT_HUNGRY:	exercise(A_CON, TRUE); break;
		    case WEAK:		exercise(A_STR, FALSE);
					if (Role_if(PM_MONK))	/* fasting */
					    exercise(A_WIS, TRUE);
					break;
		    case FAINTING:
		    case FAINTED:	exercise(A_CON, FALSE); break;
		}

		/* Encumberance Checks */
#ifdef DEBUG
		pline("exerper: Encumber checks");
#endif
		switch (near_capacity()) {
		    case MOD_ENCUMBER:	exercise(A_STR, TRUE); break;
		    case HVY_ENCUMBER:	exercise(A_STR, TRUE);
					exercise(A_DEX, FALSE); break;
		    case EXT_ENCUMBER:	exercise(A_DEX, FALSE);
					exercise(A_CON, FALSE); break;
		}

	}

	/* status checks */
	if(!(moves % 5)) {
#ifdef DEBUG
		pline("exerper: Status checks");
#endif
		if ((HClairvoyant & (INTRINSIC|TIMEOUT)) &&
			!BClairvoyant)                      exercise(A_WIS, TRUE);
		if (HRegeneration)			exercise(A_STR, TRUE);

		if(Sick || Vomiting)     exercise(A_CON, FALSE);
		if(Confusion || Hallucination)		exercise(A_WIS, FALSE);
		if((Wounded_legs 
#ifdef STEED
		    && !u.usteed
#endif
			    ) || Fumbling || HStun)	exercise(A_DEX, FALSE);
	}
}

void
exerchk()
{
	int	i, mod_val;
	
	if(umechanoid) return; /* Mechanoids can't excercise abilities */
	/*	Check out the periodic accumulations */
	exerper();
	
#ifdef DEBUG
	if(moves >= next_check)
		pline("exerchk: ready to test. multi = %d.", multi);
#endif
	/*	Are we ready for a test?	*/
	if(moves >= next_check && !multi) {
#ifdef DEBUG
	    pline("exerchk: testing.");
#endif
	    /*
	     *	Law of diminishing returns (Part II):
	     *
	     *	The effects of "exercise" and "abuse" wear
	     *	off over time.  Even if you *don't* get an
	     *	increase/decrease, you lose some of the
	     *	accumulated effects.
	     */
	    for(i = 0; i < A_MAX; AEXE(i++) /= 2) {

#ifdef DEBUG
		pline("exerchk: testing %s (%d, max %d).",
			(i == A_STR) ? "Str" : (i == A_WIS) ? "Wis" : 
			(i == A_DEX) ? "Dex" : (i == A_INT) ? "Int" : 
			(i == A_CON) ? "Con" : (i == A_CHA) ? "Cha" : 
			"Err", AEXE(i), AMAX(i));
#endif

		if(ABASE(i) >= 18 || (!AEXE(i) && ABASE(i) >= AMAX(i))) continue;
		// if(i == A_CHA) continue;/* can't exercise cha */

#ifdef DEBUG
		pline("passed");
#endif

		/*
		 *	Law of diminishing returns (Part III):
		 *
		 *	You don't *always* gain by exercising.
		 */

		 //	[MRS 92/10/28 - Treat Wisdom specially for balance.]
		// if(rn2(AVAL) > ((i != A_WIS) ? abs(AEXE(i)*2/3) : abs(AEXE(i))))
		    // continue;
		if(!(ABASE(i) < AMAX(i) && !(i == A_STR && u.uhs >= 3) && AEXE(i) >= 0) && rn2(AVAL) > (abs(AEXE(i)*2/3)) )
		    continue;
		mod_val = sgn(AEXE(i));

#ifdef DEBUG
		pline("exerchk: changing %d.", i);
#endif
		if(adjattrib(i, mod_val, -1)) {
#ifdef DEBUG
		    pline("exerchk: changed %d.", i);
#endif
		    /* if you actually changed an attrib - zero accumulation */
		    AEXE(i) = 0;
		    /* then print an explanation */
		    switch(i) {
		    case A_STR: You((mod_val >0) ?
				    "must have been exercising." :
				    "must have been abusing your body.");
				if(mod_val < 0)	AMAX(i) += mod_val; /* permanent drain */
			break;
		    case A_DEX: You((mod_val >0) ?
				    "must have been working on your reflexes." :
				    "haven't been working on reflexes lately.");
				if(mod_val < 0)	AMAX(i) += mod_val; /* permanent drain */
			break;
		    case A_CON: You((mod_val >0) ?
				    "must be leading a healthy life-style." :
					u.umorgul ? "have the chill of death about you."
				    : "haven't been watching your health.");
				if(mod_val < 0)	AMAX(i) += mod_val; /* permanent drain */
			break;
		    case A_INT: You((mod_val >0) ?
					"must have been really concentrating lately." :
					"haven't been thinking things through.");
				if(mod_val < 0)	AMAX(i) += mod_val; /* permanent drain */
			break;
		    case A_WIS: You((mod_val >0) ?
				    "must have been very observant." :
				    u.wimage >= 10 ? "are being consumed by the image of the weeping angel!" 
					: u.umorgul ? "have the chill of death about you."
					: "haven't been paying attention.");
				if(mod_val < 0)	AMAX(i) += mod_val; /* permanent drain */
			break;
		    case A_CHA: You((mod_val >0) ?
				    "must have been very charming lately." :
					u.umorgul ? "have the chill of death about you."
				    : "haven't been watching behavior.");
				if(mod_val < 0)	AMAX(i) += mod_val; /* permanent drain */
			break;
		    }
		}
	    }
	    next_check += rn1(200,800);
#ifdef DEBUG
	    pline("exerchk: next check at %ld.", next_check);
#endif
	}
}

/* OBSOLETE: next_check will otherwise have its initial 600L after a game restore */
/* next_check is now stored in the you struct, and is persistent from session to session. */
void
reset_attribute_clock()
{
	/* if (moves > 600L) next_check = moves + rn1(50,800); */
}


void
init_attr(np)
	register int	np;
{
	register int	i, x, tryct;


	for(i = 0; i < A_MAX; i++) {
	    ABASE(i) = AMAX(i) = urole.attrbase[i];
		if(ABASE(i) > ATTRMAX(i)) ABASE(i) = AMAX(i) = ATTRMAX(i);
	    ATEMP(i) = ATIME(i) = 0;
	    np -= ABASE(i);
	}

	tryct = 0;
	while(np > 0 && tryct < 100) {

	    x = rn2(100);
	    for (i = 0; (i < A_MAX) && ((x -= urole.attrdist[i]) > 0); i++) ;
	    if(i >= A_MAX) continue; /* impossible */

	    if(ABASE(i) >= ATTRMAX(i)) {

		tryct++;
		continue;
	    }
	    tryct = 0;
	    ABASE(i)++;
	    AMAX(i)++;
	    np--;
	}

	tryct = 0;
	while(np < 0 && tryct < 100) {		/* for redistribution */

	    x = rn2(100);
	    for (i = 0; (i < A_MAX) && ((x -= urole.attrdist[i]) > 0); i++) ;
	    if(i >= A_MAX) continue; /* impossible */

	    if(ABASE(i) <= ATTRMIN(i)) {

		tryct++;
		continue;
	    }
	    tryct = 0;
	    ABASE(i)--;
	    AMAX(i)--;
	    np++;
	}
	if(Race_if(PM_INCANTIFIER)){
	    for (i = 0; (i < A_MAX); i++){
			if(ABASE(A_INT) < ABASE(i)){
				ABASE(A_INT) = min(18, ABASE(i));
				AMAX(A_INT) = min(18, AMAX(i));
			}
		}
	} else if(Race_if(PM_ELF)) {
		if(ABASE(A_DEX) < ABASE(A_STR)){
			int d = ABASE(A_DEX);
			ABASE(A_DEX) = ABASE(A_STR);
			ABASE(A_STR) = d;
			AMAX(A_DEX) = ABASE(A_DEX);
			AMAX(A_STR) = ABASE(A_STR);
		}
	}
}

void
init_mask_attr(np, mask)
	int	np;
	struct obj *mask;
{
	int	i, x, tryct;
	struct Role *mrole = pm2role(mask->mp->mskrolenum);

	for(i = 0; i < A_MAX; i++) {
	    ABASE(i) = AMAX(i) = urole.attrbase[i];
	    ATEMP(i) = ATIME(i) = 0;
	    np -= urole.attrbase[i];
	}

	tryct = 0;
	while(np > 0 && tryct < 100) {

	    x = rn2(100);
	    for (i = 0; (i < A_MAX) && ((x -= urole.attrdist[i]) > 0); i++) ;
	    if(i >= A_MAX) continue; /* impossible */

	    if(ABASE(i) >= ATTRMAX(i)) {

		tryct++;
		continue;
	    }
	    tryct = 0;
	    ABASE(i)++;
	    AMAX(i)++;
	    np--;
	}

	tryct = 0;
	while(np < 0 && tryct < 100) {		/* for redistribution */

	    x = rn2(100);
	    for (i = 0; (i < A_MAX) && ((x -= urole.attrdist[i]) > 0); i++) ;
	    if(i >= A_MAX) continue; /* impossible */

	    if(ABASE(i) <= ATTRMIN(i)) {

		tryct++;
		continue;
	    }
	    tryct = 0;
	    ABASE(i)--;
	    AMAX(i)--;
	    np++;
	}
}

void
redist_attr()
{
	register int i, tmp;

	for(i = 0; i < A_MAX; i++) {
	    if (i==A_INT || i==A_WIS) continue;
		/* Polymorphing doesn't change your mind */
	    tmp = AMAX(i);
	    AMAX(i) += (rn2(5)-2);
	    if (AMAX(i) > ATTRMAX(i)) AMAX(i) = ATTRMAX(i);
	    if (AMAX(i) < ATTRMIN(i)) AMAX(i) = ATTRMIN(i);
	    ABASE(i) = ABASE(i) * AMAX(i) / tmp;
	    /* ABASE(i) > ATTRMAX(i) is impossible */
	    if (ABASE(i) < ATTRMIN(i)) ABASE(i) = ATTRMIN(i);
	}
	(void)encumber_msg();
}

STATIC_OVL
void
postadjabil(ability)
long *ability;
{
	if (!ability) return;
	if (ability == &(HWarning) || ability == &(HSee_invisible))
		see_monsters();
}

void
adjabil(oldlevel,newlevel)
int oldlevel, newlevel;
{
	register const struct innate *abil, *rabil;
	long mask = FROMEXPER;

	// set default values
	abil = 0;
	rabil = 0;

	switch (Role_switch) {
	case PM_ARCHEOLOGIST:   abil = arc_abil;	break;
	case PM_ANACHRONONAUT:    abil = ana_abil;	break;
	case PM_BARBARIAN:      abil = bar_abil;	break;
#ifdef BARD
	case PM_BARD:           abil = brd_abil;	break;
#endif
	case PM_EXILE:      	abil = bin_abil;	break;
	case PM_CAVEMAN:        abil = cav_abil;	break;
#ifdef CONVICT
	case PM_CONVICT:        abil = con_abil;	break;
#endif	/* CONVICT */
	case PM_HEALER:         abil = hea_abil;	break;
	case PM_KNIGHT:         abil = kni_abil;	break;
	case PM_MONK:           abil = mon_abil;	break;
	case PM_NOBLEMAN:
		switch (Race_switch)
		{
		case PM_HALF_DRAGON:
			abil = kni_abil;
			break;
		case PM_DWARF:
			abil = kni_abil;
			break;
		case PM_DROW:
			if (flags.initgend)
				abil = pri_abil;
			else
				abil = 0;
			break;
		case PM_ELF:
			abil = elnob_abil;
			break;
		default:
			abil = 0;
			break;
		}
	break;
	case PM_PIRATE:         abil = pir_abil;	break;
	case PM_PRIEST:         abil = pri_abil;	break;
	case PM_RANGER:         abil = ran_abil;	break;
	case PM_ROGUE:          abil = rog_abil;	break;
	case PM_SAMURAI:        abil = sam_abil;	break;
#ifdef TOURIST
	case PM_TOURIST:        abil = tou_abil;	break;
#endif
	case PM_VALKYRIE:       abil = val_abil;	break;
	case PM_WIZARD:         abil = wiz_abil;	break;
	default:                abil = 0;		break;
	}

	switch (Race_switch) {
	case PM_ELF:            rabil = elf_abil;	break;
	case PM_DROW:           rabil = elf_abil;	break;
	case PM_MYRKALFR:       rabil = elf_abil;	break;
	case PM_ORC:            rabil = orc_abil;	break;
	case PM_CLOCKWORK_AUTOMATON:rabil = clk_abil;	break;
	case PM_ANDROID:		rabil = and_abil;	break;
	case PM_INCANTIFIER:	rabil = inc_abil;	break;
	case PM_VAMPIRE:		rabil = vam_abil;	break;
	case PM_HALF_DRAGON:	rabil = hlf_abil;	break;
	case PM_YUKI_ONNA:		rabil = yki_abil;	break;
	case PM_HUMAN:
	case PM_DWARF:
	case PM_GNOME:
	default:                rabil = 0;		break;
	}

	while (abil || rabil) {
	    long prevabil;
	    /* Have we finished with the intrinsics list? */
	    if (!abil || !abil->ability) {
	    	/* Try the race intrinsics */
	    	if (!rabil || !rabil->ability) break;
	    	abil = rabil;
	    	rabil = 0;
	    	mask = FROMRACE;
	    }
		prevabil = *(abil->ability);
		if(oldlevel < abil->ulevel && newlevel >= abil->ulevel) {
			/* Abilities gained at level 1 can never be lost
			 * via level loss, only via means that remove _any_
			 * sort of ability.  A "gain" of such an ability from
			 * an outside source is devoid of meaning, so we set
			 * FROMOUTSIDE to avoid such gains.
			 */
			if (abil->ulevel == 1)
				*(abil->ability) |= (mask|FROMOUTSIDE);
			else
				*(abil->ability) |= mask;
			if(!(*(abil->ability) & INTRINSIC & ~mask)) {
			    if(*(abil->gainstr)){
					if(Race_if(PM_ANDROID) && mask == FROMRACE)
						pline("%s", abil->gainstr);
					else You_feel("%s!", abil->gainstr);
				}
			}
		} else if (oldlevel >= abil->ulevel && newlevel < abil->ulevel) {
			*(abil->ability) &= ~mask;
			if(!(*(abil->ability) & INTRINSIC)) {
			    if(*(abil->losestr)){
					if(Race_if(PM_ANDROID) && mask == FROMRACE)
						pline("%s", abil->losestr);
					else You_feel("%s!", abil->losestr);
			    } else if(*(abil->gainstr))
				You_feel("less %s!", abil->gainstr);
			}
		}
	    if (prevabil != *(abil->ability))	/* it changed */
		postadjabil(abil->ability);
	    abil++;
	}

	if (oldlevel > 0) {
		int skillslots;
	    if (newlevel > oldlevel){
			skillslots = newlevel - oldlevel;
			if(Race_if(PM_HUMAN) || Race_if(PM_INHERITOR) || Race_if(PM_ANDROID)){
				if(!(skillslots%2)) skillslots *= 1.5;
				else if(!(newlevel%2)) skillslots = skillslots*1.5 + 1;
				else skillslots *= 1.5;
			}
			add_weapon_skill(skillslots);
		}
	    else{
			skillslots = oldlevel - newlevel;
			if(Race_if(PM_HUMAN) || Race_if(PM_INHERITOR) || Race_if(PM_ANDROID)){
				if(!(skillslots%2)) skillslots *= 1.5;
				else if(!(oldlevel%2)) skillslots = skillslots*1.5 + 1;
				else skillslots *= 1.5;
			}
			lose_weapon_skill(skillslots);
		}
	}
}


int
newhp()
{
	int	hp;


	if (u.ulevel == 0) {
	    /* Initialize hit points */
	    hp = urole.hpadv.infix + urace.hpadv.infix;
	    if (urole.hpadv.inrnd > 1) hp += rnd(urole.hpadv.inrnd);
		else if(urole.hpadv.inrnd > 0) hp += rn2(urole.hpadv.inrnd);
	    if (urace.hpadv.inrnd > 1) hp += rnd(urace.hpadv.inrnd);
		else if(urace.hpadv.inrnd > 0) hp += rn2(urace.hpadv.inrnd);

	    /* Initialize alignment stuff */
	    u.ualign.type = aligns[flags.initalign].value;
	    u.ualign.record = urole.initrecord;

		return hp;
	} else {
	    if (u.ulevel < urole.xlev) {
	    	hp = urole.hpadv.lofix + urace.hpadv.lofix;
	    	if (urole.hpadv.lornd > 1) hp += rnd(urole.hpadv.lornd);
	    	else if (urole.hpadv.lornd > 0) hp += rn2(2); //0-1
	    	if (urace.hpadv.lornd > 1) hp += rnd(urace.hpadv.lornd);
	    	else if (urace.hpadv.lornd > 0) hp += rn2(2); //0-1
	    } else {
	    	hp = urole.hpadv.hifix + urace.hpadv.hifix;
	    	if (urole.hpadv.hirnd > 1) hp += rnd(urole.hpadv.hirnd);
	    	else if (urole.hpadv.hirnd > 0) hp += rn2(2); //0-1
	    	if (urace.hpadv.hirnd > 1) hp += rnd(urace.hpadv.hirnd);
	    	else if (urace.hpadv.hirnd > 0) hp += rn2(2); //0-1
	    }
	}
	
	return((hp <= 0) ? 1 : hp);
}

int
newen()
{
	int	en;


	if (u.ulevel == 0) {
	    /* Initialize energy points */
	    en = urole.enadv.infix + urace.enadv.infix;
	    if (urole.enadv.inrnd > 1) en += rnd(urole.enadv.inrnd);
		else if(urole.enadv.inrnd > 0) en += rn2(urole.enadv.inrnd);
	    if (urace.enadv.inrnd > 1) en += rnd(urace.enadv.inrnd);
		else if(urace.enadv.inrnd > 0) en += rn2(urace.enadv.inrnd);

		return en;
	} else {
	    if (u.ulevel < urole.xlev) {
	    	en = urole.enadv.lofix + urace.enadv.lofix;
	    	if (urole.enadv.lornd > 1) en += rnd(urole.enadv.lornd);
	    	else if (urole.enadv.lornd > 0) en += rn2(2); //0-1
	    	if (urace.enadv.lornd > 1) en += rnd(urace.enadv.lornd);
	    	else if (urace.enadv.lornd > 0) en += rn2(2); //0-1
	    } else {
	    	en = urole.enadv.hifix + urace.enadv.hifix;
	    	if (urole.enadv.hirnd > 1) en += rnd(urole.enadv.hirnd);
	    	else if (urole.enadv.hirnd > 0) en += rn2(2); //0-1
	    	if (urace.enadv.hirnd > 1) en += rnd(urace.enadv.hirnd);
	    	else if (urace.enadv.hirnd > 0) en += rn2(2); //0-1
	    }
	}
	
	return((en <= 0) ? 1 : en);
}

int
maxhp(incon)
int incon;
{
	int	hp;
	int perlevel;


	/* Initialize hit points */
	hp = urole.hpadv.infix + urace.hpadv.infix;
	
	hp += urole.hpadv.inrnd;
	hp += urace.hpadv.inrnd;

	if (u.ulevel <= urole.xlev) {
		perlevel = urole.hpadv.lofix + urace.hpadv.lofix;
		perlevel += urole.hpadv.lornd;
		perlevel += urace.hpadv.lornd;
		
		hp += (u.ulevel-1)*perlevel;
	} else {
		perlevel = urole.hpadv.lofix + urace.hpadv.lofix;
		perlevel += urole.hpadv.lornd;
		perlevel += urace.hpadv.lornd;
		
		hp += (urole.xlev-1)*perlevel; //Full bonus from below cutoff
		
		perlevel = urole.hpadv.hifix + urace.hpadv.hifix;
		perlevel += urole.hpadv.hirnd;
		perlevel += urace.hpadv.hirnd;
		
		hp += (u.ulevel-urole.xlev)*perlevel;
	}
	
	if(incon) hp += u.ulevel*conplus(ACURR(A_CON));
	
	return((hp <= u.ulevel) ? u.ulevel : hp);
}

int
maxen()
{
	int	en;
	int perlevel;


	/* Initialize hit points */
	en = urole.enadv.infix + urace.enadv.infix;
	
	en += urole.enadv.inrnd;
	en += urace.enadv.inrnd;

	if (u.ulevel <= urole.xlev) {
		perlevel = urole.enadv.lofix + urace.enadv.lofix;
		perlevel += urole.enadv.lornd;
		perlevel += urace.enadv.lornd;
		
		en += (u.ulevel-1)*perlevel;
	} else {
		perlevel = urole.enadv.lofix + urace.enadv.lofix;
		perlevel += urole.enadv.lornd;
		perlevel += urace.enadv.lornd;
		
		en += (urole.xlev-1)*perlevel; //Full bonus from below cutoff
		
		perlevel = urole.enadv.hifix + urace.enadv.hifix;
		perlevel += urole.enadv.hirnd;
		perlevel += urace.enadv.hirnd;
		
		en += (u.ulevel-urole.xlev)*perlevel;
	}
	
	return((en <= u.ulevel) ? u.ulevel : en);
}

int
conplus(con)
	int con;
{
	int conplus;
	
	if (con <= 3) conplus = -2;
	else if (con <= 6) conplus = -1;
	else if (con <= 14) conplus = 0;
	else if (con <= 16) conplus = 1;
	else if (con == 17) conplus = 2;
	else if (con == 18) conplus = 3;
	else if (con == 19) conplus = 4;
	else if (con <= 21) conplus = 5;
	else if (con <= 24) conplus = 6;
	else if (con == 25) conplus = 7;
	else conplus = 4;
	
	return conplus;
}

void
calc_total_maxhp()
{
	int ulev;
	int * hpmax;
	int * hp;
	int * hprolled;
	int hpcap;
	int rawmax;
	int maxbonus;
	int adjbonus;
	if (Upolyd) {
		ulev = (int)(mons[u.umonnum].mlevel);
		hp = &u.mh;
		hpmax = &u.mhmax;
		hprolled = &u.mhrolled;
		hpcap = 24 + 2*mons[u.umonnum].mlevel*8;
	} else {
		ulev = u.ulevel;
		hp = &u.uhp;
		hpmax = &u.uhpmax;
		hprolled = &u.uhprolled;
		hpcap = 24 + 2*maxhp(1);
	}
	
	if(u.uhpbonus > 0){
		rawmax = *hprolled + ulev*conplus(ACURR(A_CON));
		
		/*Calculate Metamorphosis *before* the max bonus is determined*/
		if(active_glyph(CLOCKWISE_METAMORPHOSIS)){
			rawmax *= 1.3;
			hpcap *= 1.3;
		}
		
		maxbonus = hpcap - rawmax;
		
		/*Calculate Malcanthet's multiplier *after* the max bonus is determined*/
		if(u.uhpmultiplier)
			rawmax = rawmax + (rawmax * u.uhpmultiplier / 10); /*Multiplier is in units of tenths*/
		
		if(maxbonus > 0){
			adjbonus = round(2.0*maxbonus/(1+exp(-(4.0/(2.0*maxbonus))*u.uhpbonus)) - maxbonus);
		}
		else adjbonus = 0;
		
		*hpmax = rawmax + adjbonus + u.uhpmod;
	} else {
		rawmax = *hprolled + ulev*conplus(ACURR(A_CON));
		
		if(active_glyph(CLOCKWISE_METAMORPHOSIS)){
			rawmax *= 1.3;
			hpcap *= 1.3;
		}
		
		if(u.uhpmultiplier)
			rawmax = rawmax + (rawmax * u.uhpmultiplier / 10); /*Multiplier is in units of tenths*/
		
		*hpmax = rawmax + u.uhpbonus + u.uhpmod;
	}
	
	if(*hpmax < 1) *hpmax = 1;
	// *hpmax += min(nxtra, max(0, 6*nxtra/5 - 6*nxtra*(*hpmax)*(*hpmax)/(5*hpcap*hpcap)));
	if(*hp > *hpmax)
		*hp = *hpmax;
	
	flags.botl = 1;
}

void
calc_total_maxen()
{
	int en;
	en = u.uenrolled + (u.ulevel*ACURR(A_CON))/4;
	
	if(active_glyph(FORMLESS_VOICE))
		en *= 1.3;
	
	if(u.uenmultiplier)
		en = en + (en * u.uhpmultiplier / 10); /*Multiplier is in units of tenths*/
	
	u.uenmax = en + u.uenbonus;
	
	if(u.uenmax < 0) u.uenmax = 0;
	// *hpmax += min(nxtra, max(0, 6*nxtra/5 - 6*nxtra*(*hpmax)*(*hpmax)/(5*hpcap*hpcap)));
	if(u.uen > u.uenmax)
		u.uen = u.uenmax;
	
	flags.botl = 1;
}

#endif /* OVLB */
#ifdef OVL0

schar
acurr(x)
int x;
{
	register int tmp = (u.abon.a[x] + u.atemp.a[x] + u.acurr.a[x]);

	if(u.ufirst_light)
		tmp++;
	if(u.ufirst_sky)
		tmp++;
	if(u.ufirst_life)
		tmp++;
	if((uright && uright->oartifact == ART_SHARD_FROM_MORGOTH_S_CROWN) || (uleft && uleft->oartifact == ART_SHARD_FROM_MORGOTH_S_CROWN)){
		tmp += 6;
	}
	
	if(x ==A_CHA && uwep && uwep->oartifact == ART_SODE_NO_SHIRAYUKI){
		tmp += uwep->spe;
	}

	if(x == A_WIS && uarm && arti_chawis(uarm) && uarmc){
		tmp += uarm->spe;
	}
#ifdef TOURIST
	if(x == A_WIS && uarmu && arti_chawis(uarmu) && (uarmc || uarm)){
		tmp += uarmu->spe;
	}
#endif	/*TOURIST*/

	if(x == A_CHA && uarmc && arti_chawis(uarmc)){
		tmp += uarmc->spe;
	}
	if(x == A_CHA && uarm && arti_chawis(uarm) && !uarmc){
		tmp += uarm->spe;
	}
#ifdef TOURIST
	if(x == A_CHA && uarmu && arti_chawis(uarmu) && !uarmc && !uarm){
		tmp += uarmu->spe;
	}
#endif	/*TOURIST*/

	if (x == A_STR) {
		if(Race_if(PM_ORC)){
			tmp += u.ulevel/3;
			if(tmp > 18) tmp = STR19(tmp);
		}
		if ((uarmg && uarmg->otyp == GAUNTLETS_OF_POWER) || 
			(uwep && uwep->oartifact == ART_SCEPTRE_OF_MIGHT) || 
			(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && uwep->ovar1&SEAL_YMIR && mvitals[PM_ACERERAK].died > 0) ||
			(uwep && uwep->oartifact == ART_STORMBRINGER) ||
			// (uswapwep && uswapwep->oartifact == ART_STORMBRINGER) ||
			(uwep && uwep->oartifact == ART_OGRESMASHER) ||
			(uswapwep && uswapwep->oartifact == ART_OGRESMASHER)
		) return(125);
#ifdef WIN32_BUG
		else return(x=((tmp >= 125) ? 125 : (tmp <= 3) ? 3 : tmp));
#else
		else return((schar)((tmp >= 125) ? 125 : (tmp <= 3) ? 3 : tmp));
#endif
	} else if (x == A_CON) {
		if (
			(uwep && uwep->oartifact == ART_OGRESMASHER) ||
			(uswapwep && uswapwep->oartifact == ART_OGRESMASHER) ||
			(uwep && uwep->oartifact == ART_STORMBRINGER) ||
			// (uswapwep && uswapwep->oartifact == ART_STORMBRINGER) ||
			(uarmg && uarmg->oartifact == ART_GREAT_CLAWS_OF_URDLEN)
		) return(25);
		if(Race_if(PM_ORC)){
			tmp += u.ulevel/3;
		}
	} else if (x == A_DEX) {
		if (
			(uarmg && uarmg->oartifact == ART_PREMIUM_HEART)
		) return(25);
		if(Race_if(PM_ORC)){
			tmp += u.ulevel/3;
		}
	} else if (x == A_CHA) {
		if (tmp < 18 && youracedata && (youracedata->mlet == S_NYMPH ||
		    u.umonnum==PM_SUCCUBUS || u.umonnum == PM_INCUBUS))
		    return 18;
	} else if (x == A_INT || x == A_WIS) {
		/* yes, this may raise int/wis if player is sufficiently
		 * stupid.  there are lower levels of cognition than "dunce".
		 */
		if (uarmh && uarmh->otyp == DUNCE_CAP) return(6);
		else if(u.sealsActive&SEAL_HUGINN_MUNINN) return 25;
	}
	
#ifdef WIN32_BUG
	return(x=((tmp >= 25) ? 25 : (tmp <= 3) ? 3 : tmp));
#else
	return((schar)((tmp >= 25) ? 25 : (tmp <= 3) ? 3 : tmp));
#endif
}

void
change_usanity(delta)
int delta;
{
	pline("Sanity change: %d + %d", u.usanity, delta);
	u.usanity += delta;
	if(u.usanity < 0)
		u.usanity = 0;
	if(u.usanity > 100)
		u.usanity = 100;
	pline("= %d", u.usanity);
}

void
change_uinsight(delta)
int delta;
{
	pline("Insight change: %d + %d", u.uinsight, delta);
	u.uinsight += delta;
	if(u.uinsight < 0)
		u.uinsight = 0;
	if(u.uinsight > INSIGHT_RATE/20)
		u.uinsight = INSIGHT_RATE/20;
	pline("= %d", u.uinsight);
}

void
doguidance(mdef, dmg)
struct monst *mdef;
int dmg;
{
	if (mdef && mdef->mattackedu) {
		int life = (int)(dmg*0.2+1);
		healup(life, 0, FALSE, FALSE);
	}
}

int
glyph_sanity(thought)
long int thought;
{
	int sanlevel = 1000;
	switch(thought){
		case ANTI_CLOCKWISE_METAMORPHOSIS:
		break;
		case CLOCKWISE_METAMORPHOSIS:
		break;
		case ARCANE_BULWARK:
		break;
		case DISSIPATING_BULWARK:
		break;
		case SMOLDERING_BULWARK:
		break;
		case FROSTED_BULWARK:
		break;
		case BLOOD_RAPTURE:
			sanlevel = 90;
		break;
		case CLAWMARK:
			sanlevel = 90;
		break;
		case CLEAR_DEEPS:
		break;
		case DEEP_SEA:
		break;
		case COMMUNION:
		break;
		case CORRUPTION:
			sanlevel = 80;
		break;
		case EYE_THOUGHT:
		break;
		case FORMLESS_VOICE:
		break;
		case GUIDANCE:
		break;
		case IMPURITY:
			sanlevel = 80;
		break;
		case MOON:
		break;
		case WRITHE:
			sanlevel = 90;
		break;
		case RADIANCE:
		break;
		default:
			impossible("bad glyph %s in active_glyph!", thought);
			return 0;
		break;
	}
	return sanlevel;
}

int
glyph_insight(thought)
long int thought;
{
	int insightlevel = 0;
	switch(thought){
		case ANTI_CLOCKWISE_METAMORPHOSIS:
			insightlevel = 20;
		break;
		case CLOCKWISE_METAMORPHOSIS:
			insightlevel = 20;
		break;
		case ARCANE_BULWARK:
			insightlevel = 18;
		break;
		case DISSIPATING_BULWARK:
			insightlevel = 16;
		break;
		case SMOLDERING_BULWARK:
			insightlevel = 11;
		break;
		case FROSTED_BULWARK:
			insightlevel = 12;
		break;
		case BLOOD_RAPTURE:
			insightlevel = 14;
		break;
		case CLAWMARK:
			insightlevel = 16;
		break;
		case CLEAR_DEEPS:
			insightlevel = 10;
		break;
		case DEEP_SEA:
			insightlevel = 22;
		break;
		case COMMUNION:
			insightlevel = 25;
		break;
		case CORRUPTION:
			insightlevel = 15;
		break;
		case EYE_THOUGHT:
			insightlevel = 17;
		break;
		case FORMLESS_VOICE:
			insightlevel = 19;
		break;
		case GUIDANCE:
			insightlevel = 13;
		break;
		case IMPURITY:
			insightlevel = 5;
		break;
		case MOON:
			insightlevel = 10;
		break;
		case WRITHE:
			insightlevel = 14;
		break;
		case RADIANCE:
			insightlevel = 12;
		break;
		default:
			impossible("bad glyph %s in active_glyph!", thought);
			return 0;
		break;
	}
	return insightlevel;
}

int
active_glyph(thought)
long int thought;
{
	int insightlevel = 0, sanlevel = 1000;
	if(!(u.thoughts&thought))
		return 0;
	insightlevel = glyph_insight(thought);
	// sanlevel = glyph_sanity(thought);
	if(u.uinsight >= insightlevel && u.usanity <= sanlevel)
		return 1;
	else return 0;
	return 0;
}

int
roll_madness(madness)
long int madness;
{
	int sanlevel;
	if(!(u.umadness&madness))
		return 0;
	
	sanlevel = (int)(((float)rand()/(float)(RAND_MAX)) * ((float)rand()/(float)(RAND_MAX)) * 100);
	
	if(u.usanity < sanlevel)
		return 1;
	return 0;
}

int
mad_turn(madness)
long int madness;
{
	int sanlevel;
	unsigned long hashed = hash((unsigned long) (moves + madness)); //Offset the different madnesses before hashing
	if(!(u.umadness&madness))
		return 0;
	
	sanlevel = max_ints(1,(int)(((float)hashed/ULONG_MAX) * ((float)hashed/ULONG_MAX) * 100));
	
	if(u.usanity < sanlevel)
		return 1;
	return 0;
}

void
roll_frigophobia()
{
	if(roll_madness(MAD_FRIGOPHOBIA)){
		if(!Cold_resistance){
			You("panic from the cold!");
			nomul(-1*rnd(6), "panicking");
		}
		else if(roll_madness(MAD_FRIGOPHOBIA)){//Very scared of ice
			You("panic from the chill!");
			nomul(-1*rnd(3), "panicking");
		}
	}
}

int
give_madness(mon)
struct monst *mon;
{
	int mm = monsndx(mon->data);
	switch(mm){
		case PM_ALDINACH:
			u.umadness |= MAD_REAL_DELUSIONS;
		break;
		case PM_ALRUNES:
		case PM_HATEFUL_WHISPERS:
			u.umadness |= MAD_SANCTITY;
		break;
		case PM_JUIBLEX:
			u.umadness |= MAD_GLUTTONY;
		break;
		case PM_ZUGGTMOY:
			u.umadness |= MAD_SPORES;
		break;
		case PM_KOSTCHTCHIE:
			u.umadness |= MAD_FRIGOPHOBIA;
		break;
		case PM_BAPHOMET:
			u.umadness |= MAD_RAGE;
		break;
		case PM_YEENOGHU:
			u.umadness |= MAD_CANNIBALISM;
		break;
		case PM_MALCANTHET:
			u.umadness |= MAD_NUDIST;
		break;
		case PM_SHAKTARI:
			u.umadness |= MAD_OPHIDIOPHOBIA;
		break;
		case PM_AVATAR_OF_LOLTH:
			u.umadness |= MAD_ARACHNOPHOBIA;
		break;
		case PM_GRAZ_ZT:
			u.umadness |= MAD_ARGENT_SHEEN;
		break;
		case PM_ORCUS:
			u.umadness |= MAD_SUICIDAL;
		break;
		case PM_DAGON:
			u.umadness |= MAD_THALASSOPHOBIA;
		break;
		case PM_OBOX_OB:
			u.umadness |= MAD_ENTOMOPHOBIA;
		break;
		case PM_LAMASHTU:
			u.umadness |= MAD_TALONS;
		break;
		case PM_DEMOGORGON:
			u.umadness |= MAD_PARANOIA;
		break;
		case PM_ELDER_PRIEST:
			u.umadness |= MAD_HELMINTHOPHOBIA;
		break;
		case PM_GREAT_CTHULHU:
			// u.umadness |= ;
		break;
	}
}

/* condense clumsy ACURR(A_STR) value into value that fits into game formulas
 */
schar
acurrstr(str)
	int str;
{
	if (str <= 18) return((schar)str);
	if (str <= 121) return((schar)(19 + str / 50)); /* map to 19-21 */
	else return((schar)(str - 100));
}

#endif /* OVL0 */
#ifdef OVL2

/* avoid possible problems with alignment overflow, and provide a centralized
 * location for any future alignment limits
 */
void
adjalign(n)
register int n;
{
	register int newalign = u.ualign.record + n;

	if(n < 0) {
		if(newalign < u.ualign.record)
			u.ualign.record = newalign;
			if(u.ualign.record > ALIGNLIM)
				u.ualign.record = ALIGNLIM;
	} else
		if(newalign > u.ualign.record) {
			u.ualign.record = newalign;
			if(u.ualign.record > ALIGNLIM)
				u.ualign.record = ALIGNLIM;
		}
}

#endif /* OVL2 */

/** Returns the hitpoints of your current form. */
int
uhp()
{
    return (Upolyd ? u.mh : u.uhp);
}

/** Returns the maximal hitpoints of your current form. */
int
uhpmax()
{
    return (Upolyd ? u.mhmax : u.uhpmax);
}


/*attrib.c*/
