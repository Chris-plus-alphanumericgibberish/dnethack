/*	SCCS Id: @(#)exper.c	3.4	2002/11/20	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_DCL void NDECL(binderdown);
STATIC_DCL int FDECL(enermod, (int));

static long expUps[] = {
0,		//1
20,		//2
50,		//3
100,	//4
200,	//5
400,	//6
800,	//7
1600,	//8
3200,	//9
6400,	//10
10400,	//11
15600,	//12
22800,	//13
32500,	//14
45200,	//15
61900,	//16
83100,	//17
109300,	//18
141000,	//19
178700,	//20
223000,	//21
274500,	//22
333800,	//23
401500,	//24
478200,	//25
563900,	//26
659600,	//27
766300,	//28
885000,	//29
1000000	//30
};

long
newuexp(lev)
int lev;
{
	if (lev < 30) return expUps[lev];
	else return 100000L + lev*10000L;
	// if (lev < 10) return (10L * (1L << lev));
	// if (lev < 20) return (10000L * (1L << (lev - 10)));
	// return (10000000L * ((long)(lev - 19)));
}

int
experience(mtmp, nk)	/* return # of exp points for mtmp after nk killed */
	register struct	monst *mtmp;
	register int	nk;
#if defined(macintosh) && (defined(__SC__) || defined(__MRC__))
# pragma unused(nk)
#endif
{
	register struct permonst *ptr = mtmp->data;
	int	i, tmp, tmp2;
	
	if(mtmp->data == &mons[PM_DANCING_BLADE] || mtmp->data == &mons[PM_LONG_SINUOUS_TENTACLE] || mtmp->data == &mons[PM_SWARM_OF_SNAKING_TENTACLES] || mtmp->data == &mons[PM_WIDE_CLUBBED_TENTACLE]) return 0;
	
/*	Dungeon fern spores give no experience */
	if(is_fern_spore(mtmp->data)) return 0;

/*	zombies give no experience (resurecting and infecting)*/
	if(is_derived_undead_mon(mtmp) && mtmp->mclone) return 0;

	tmp = 1 + mtmp->m_lev * mtmp->m_lev;

/*	For higher ac values, give extra experience */
	if ((i = find_mac(mtmp)) < 3) tmp += (7 - i) * ((i < 0) ? 2 : 1);

/*	For very fast monsters, give extra experience */
	if (ptr->mmove > NORMAL_SPEED)
	    tmp += (ptr->mmove > (3*NORMAL_SPEED/2)) ? 5 : 3;

/*	For each "special" attack type give extra experience */
	for(i = 0; i < NATTK; i++) {

	    tmp2 = ptr->mattk[i].aatyp;
	    if(tmp2 > AT_BUTT) {

		if(tmp2 == AT_WEAP) tmp += 5;
		else if(tmp2 == AT_MAGC || tmp2 == AT_MMGC) tmp += 10;
		else tmp += 3;
	    }
	}

/*	For each "special" damage type give extra experience */
	for(i = 0; i < NATTK; i++) {
	    tmp2 = ptr->mattk[i].adtyp;
	    if(tmp2 > AD_PHYS && tmp2 < AD_BLND) tmp += 2*mtmp->m_lev;
	    else if((tmp2 == AD_DRLI) || (tmp2 == AD_STON) ||
	    		(tmp2 == AD_SLIM)) tmp += 50;
	    else if(tmp != AD_PHYS) tmp += mtmp->m_lev;
		/* extra heavy damage bonus */
	    if((int)(ptr->mattk[i].damd * ptr->mattk[i].damn) > 23)
		tmp += mtmp->m_lev;
	    if (tmp2 == AD_WRAP && ptr->mlet == S_EEL && !Amphibious)
		tmp += 1000;
	}

/*	For certain "extra nasty" monsters, give even more */
	if (extra_nasty(ptr)) tmp += (7 * mtmp->m_lev);

/*	For higher level monsters, an additional bonus is given */
	if(mtmp->m_lev > 8) tmp += 50;

/*	Dungeon fern spores give no experience */
	if(is_fern_spore(mtmp->data)) tmp = 0;

#ifdef MAIL
	/* Mail daemons put up no fight. */
	if(mtmp->data == &mons[PM_MAIL_DAEMON]) tmp = 1;
#endif

	return(tmp);
}

int
ptrexperience(ptr)	/* return # of exp points for mtmp after nk killed */
	register struct permonst *ptr;
{
	int	i, tmp, tmp2;
	
	if(ptr == &mons[PM_DANCING_BLADE] || ptr == &mons[PM_LONG_SINUOUS_TENTACLE] || ptr == &mons[PM_SWARM_OF_SNAKING_TENTACLES] || ptr == &mons[PM_WIDE_CLUBBED_TENTACLE]) return 0;
	
/*	Dungeon fern spores give no experience */
	if(is_fern_spore(ptr)) return 0;

	tmp = 1 + ptr->mlevel * ptr->mlevel;

	tmp2 = 10-(ptr->nac+ptr->dac+ptr->pac);
/*	For higher ac values, give extra experience */
	if ((i = tmp2) < 3) tmp += (7 - i) * ((i < 0) ? 2 : 1);

/*	For very fast monsters, give extra experience */
	if (ptr->mmove > NORMAL_SPEED)
	    tmp += (ptr->mmove > (3*NORMAL_SPEED/2)) ? 5 : 3;

/*	For each "special" attack type give extra experience */
	for(i = 0; i < NATTK; i++) {

	    tmp2 = ptr->mattk[i].aatyp;
	    if(tmp2 > AT_BUTT) {

		if(tmp2 == AT_WEAP) tmp += 5;
		else if(tmp2 == AT_MAGC || tmp2 == AT_MMGC) tmp += 10;
		else tmp += 3;
	    }
	}

/*	For each "special" damage type give extra experience */
	for(i = 0; i < NATTK; i++) {
	    tmp2 = ptr->mattk[i].adtyp;
	    if(tmp2 > AD_PHYS && tmp2 < AD_BLND) tmp += 2*ptr->mlevel;
	    else if((tmp2 == AD_DRLI) || (tmp2 == AD_STON) ||
	    		(tmp2 == AD_SLIM)) tmp += 50;
	    else if(tmp != AD_PHYS) tmp += ptr->mlevel;
		/* extra heavy damage bonus */
	    if((int)(ptr->mattk[i].damd * ptr->mattk[i].damn) > 23)
		tmp += ptr->mlevel;
	    if (tmp2 == AD_WRAP && ptr->mlet == S_EEL && !Amphibious)
		tmp += 1000;
	}

/*	For certain "extra nasty" monsters, give even more */
	if (extra_nasty(ptr)) tmp += (7 * ptr->mlevel);

/*	For higher level monsters, an additional bonus is given */
	if(ptr->mlevel > 8) tmp += 50;

/*	Dungeon fern spores give no experience */
	if(is_fern_spore(ptr)) tmp = 0;

#ifdef MAIL
	/* Mail daemons put up no fight. */
	if(ptr == &mons[PM_MAIL_DAEMON]) tmp = 1;
#endif

	return(tmp);
}

void
more_experienced(exp, rexp)
	register int exp, rexp;
{
	if(active_glyph(MOON)){
		exp *= 1.3;
		rexp *= 1.3;
	}
	if(u.ulevel < u.ulevelmax){
		//if you have lost levels to level drain, gain XP at 5x rate.
		//if you are about to regain the last drained level, gain at least the base xp total
		if(newuexp(u.ulevelmax - 1) < u.uexp + exp*5){
			u.uexp += max(exp, newuexp(u.ulevelmax - 1) - u.uexp);
		} else {
			u.uexp += exp*5;
		}
	} else u.uexp += exp;
	u.urexp += 4*exp + rexp;
	if(exp
#ifdef SCORE_ON_BOTL
	   || flags.showscore
#endif
	   ) flags.botl = 1;
	if (u.urexp >= (Role_if(PM_WIZARD) ? 1000 : 2000))
		flags.beginner = 0;
}

void
losexp(drainer,verbose,force,expdrain)		/* e.g., hit by drain life attack */
const char *drainer;	/* cause of death, if drain should be fatal */
boolean verbose; /* attack has custom notification */
boolean force; /* attack ignores drain resistance */
boolean expdrain; /* attack drains exp as well */
{
	register int num;

#ifdef WIZARD
	/* override life-drain resistance when handling an explicit
	   wizard mode request to reduce level; never fatal though */
	if (drainer && !strcmp(drainer, "#levelchange"))
	    drainer = 0;
	else
#endif
	    if (!force && Drain_resistance) return;

	if (u.ulevel > 1) {
		if(verbose) pline("%s level %d.", Goodbye(), u.ulevel);
		u.ulevel--;
		/* remove intrinsic abilities */
		adjabil(u.ulevel + 1, u.ulevel);
		reset_rndmonst(NON_PM);	/* new monster selection */
	} else {
		if (drainer) {
			killer_format = KILLED_BY;
			killer = drainer;
			done(DIED);
		}
		/* no drainer or lifesaved */
		u.uexp = 0;
	}
	num = newhp();
	u.uhprolled -= num;
	if (u.uhprolled < 1) u.uhprolled = 1;
	u.uhp -= num + conplus(ACURR(A_CON));
	calc_total_maxhp();
	
	if (u.uhp < 1) u.uhp = 1;

	
	num = newen();
	u.uenrolled -= num;
	calc_total_maxen();
	u.uen -= num;
	if (u.uen < 0) u.uen = 0;
	else if (u.uen > u.uenmax) u.uen = u.uenmax;

	if (u.uexp > 0){
		if(!expdrain) u.uexp = newuexp(u.ulevel) - 1;
		else if(u.ulevel > 1) u.uexp = (newuexp(u.ulevel) - newuexp(u.ulevel-1))/2 + newuexp(u.ulevel-1);
		else u.uexp = newuexp(1)/2;
	}
	if(Role_if(PM_EXILE)) binderdown();
	flags.botl = 1;
}

/*
 * Make experience gaining similar to AD&D(tm), whereby you can at most go
 * up by one level at a time, extra expr possibly helping you along.
 * After all, how much real experience does one get shooting a wand of death
 * at a dragon created with a wand of polymorph??
 */
void
newexplevel()
{
	if (u.ulevel < MAXULEV && u.uexp >= newuexp(u.ulevel))
	    pluslvl(TRUE);
}

/* Grant new spirits to binder */
/* It reaplies all spirts just for kicks */
void
binderup(){
	switch(u.ulevel){
	default:
	case 13:
		u.sealsKnown |= sealKey[u.sealorder[28]] | sealKey[u.sealorder[29]] | sealKey[u.sealorder[30]];
	case 12:
		u.sealsKnown |= sealKey[u.sealorder[26]] | sealKey[u.sealorder[27]];
	case 11:
		u.sealsKnown |= sealKey[u.sealorder[24]] | sealKey[u.sealorder[25]];
	case 10:
		u.sealsKnown |= sealKey[u.sealorder[21]] | sealKey[u.sealorder[22]] | sealKey[u.sealorder[23]];
	case 9:
		u.sealsKnown |= sealKey[u.sealorder[19]] | sealKey[u.sealorder[20]];
	case 8:
		u.sealsKnown |= sealKey[u.sealorder[17]] | sealKey[u.sealorder[18]];
	case 7:
		u.sealsKnown |= sealKey[u.sealorder[14]] | sealKey[u.sealorder[15]] | sealKey[u.sealorder[16]];
	case 6:
		u.sealsKnown |= sealKey[u.sealorder[12]] | sealKey[u.sealorder[13]];
	case 5:
		u.sealsKnown |= sealKey[u.sealorder[10]] | sealKey[u.sealorder[11]];
	case 4:
		u.sealsKnown |= sealKey[u.sealorder[7]] | sealKey[u.sealorder[8]] | sealKey[u.sealorder[9]];
	case 3:
		u.sealsKnown |= sealKey[u.sealorder[5]] | sealKey[u.sealorder[6]];
	case 2:
		u.sealsKnown |= sealKey[u.sealorder[3]] | sealKey[u.sealorder[4]];
	case 1:
		u.sealsKnown |= sealKey[u.sealorder[0]] | sealKey[u.sealorder[1]] | sealKey[u.sealorder[2]];
	break;
	}
}

void
binderdown(){
	switch(u.ulevel){
	default:
	case 12:
		u.sealsKnown &= ~(sealKey[u.sealorder[28]] | sealKey[u.sealorder[29]] | sealKey[u.sealorder[30]]);
	break;
	case 11:
		u.sealsKnown &= ~(sealKey[u.sealorder[26]] | sealKey[u.sealorder[27]]);
	break;
	case 10:
		u.sealsKnown &= ~(sealKey[u.sealorder[24]] | sealKey[u.sealorder[25]]);
	break;
	case 9:
		u.sealsKnown &= ~(sealKey[u.sealorder[21]] | sealKey[u.sealorder[22]] | sealKey[u.sealorder[23]]);
	break;
	case 8:
		u.sealsKnown &= ~(sealKey[u.sealorder[19]] | sealKey[u.sealorder[20]]);
	break;
	case 7:
		u.sealsKnown &= ~(sealKey[u.sealorder[17]] | sealKey[u.sealorder[18]]);
	break;
	case 6:
		u.sealsKnown &= ~(sealKey[u.sealorder[14]] | sealKey[u.sealorder[15]] | sealKey[u.sealorder[16]]);
	break;
	case 5:
		u.sealsKnown &= ~(sealKey[u.sealorder[12]] | sealKey[u.sealorder[13]]);
	break;
	case 4:
		u.sealsKnown &= ~(sealKey[u.sealorder[10]] | sealKey[u.sealorder[11]]);
	break;
	case 3:
		u.sealsKnown &= ~(sealKey[u.sealorder[7]] | sealKey[u.sealorder[8]] | sealKey[u.sealorder[9]]);
	break;
	case 2:
		u.sealsKnown &= ~(sealKey[u.sealorder[5]] | sealKey[u.sealorder[6]]);
	break;
	case 1:
		u.sealsKnown &= ~(sealKey[u.sealorder[3]] | sealKey[u.sealorder[4]]);
	break;
	case 0:
		u.sealsKnown &= ~(sealKey[u.sealorder[0]] | sealKey[u.sealorder[1]] | sealKey[u.sealorder[2]]);
	break;
	}
}

void
pluslvl(incr)
boolean incr;	/* true iff via incremental experience growth */
{		/*	(false for potion of gain level)      */
	register int num;

	if (!incr) You_feel("more experienced.");
	num = newhp();
	u.uhprolled += num;
	calc_total_maxhp();
	u.uhp += num + conplus(ACURR(A_CON));
	if (Upolyd) {
	    num = rnd(8);
	    u.mhrolled += num;
		calc_total_maxhp();
	    u.mh += num + conplus(ACURR(A_CON));
	}
	num = newen();
	u.uenrolled += num;
	calc_total_maxen();
	u.uen += num + ACURR(A_WIS)/4;
	if (u.ulevel < MAXULEV) {
	    if (incr) {
		long tmp = newuexp(u.ulevel + 1);
		if (u.uexp >= tmp) u.uexp = tmp - 1;
	    } else {
		u.uexp = newuexp(u.ulevel);
	    }
	    ++u.ulevel;
	    if (u.ulevelmax < u.ulevel) u.ulevelmax = u.ulevel;
	    pline("Welcome to experience level %d.", u.ulevel);
	    adjabil(u.ulevel - 1, u.ulevel);	/* give new intrinsics */
	    reset_rndmonst(NON_PM);		/* new monster selection */
	}
	if(Role_if(PM_EXILE)) binderup();
	flags.botl = 1;
}

/* compute a random amount of experience points suitable for the hero's
   experience level:  base number of points needed to reach the current
   level plus a random portion of what it takes to get to the next level */
long
rndexp(gaining)
boolean gaining;	/* gaining XP via potion vs setting XP for polyself */
{
	long minexp, maxexp, diff, factor, result;

	minexp = (u.ulevel == 1) ? 0L : newuexp(u.ulevel - 1);
	maxexp = newuexp(u.ulevel);
	diff = maxexp - minexp,  factor = 1L;
	/* make sure that `diff' is an argument which rn2() can handle */
	while (diff >= (long)LARGEST_INT)
	    diff /= 2L,  factor *= 2L;
	result = minexp + factor * (long)rn2((int)diff);
	/* 3.4.1:  if already at level 30, add to current experience
	   points rather than to threshold needed to reach the current
	   level; otherwise blessed potions of gain level can result
	   in lowering the experience points instead of raising them */
	if (u.ulevel == MAXULEV && gaining) {
	    result += (u.uexp - minexp);
	    /* avoid wrapping (over 400 blessed potions needed for that...) */
	    if (result < u.uexp) result = u.uexp;
	}
	return result;
}

/*exper.c*/
