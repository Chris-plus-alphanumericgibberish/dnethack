/*	SCCS Id: @(#)mhitm.c	3.4	2003/01/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#include "edog.h"

extern boolean notonhead;
#define POLE_LIM 5	/* How far monsters can use pole-weapons */

#ifdef OVLB

static NEARDATA boolean vis, far_noise;
static NEARDATA long noisetime;
static NEARDATA struct obj *otmp;

static const char brief_feeling[] =
	"have a %s feeling for a moment, then it passes.";

STATIC_DCL char *FDECL(mon_nam_too, (char *,struct monst *,struct monst *));
STATIC_DCL void FDECL(mrustm, (struct monst *, struct monst *, struct obj *));
STATIC_DCL int FDECL(hitmm, (struct monst *,struct monst *,struct attack *));
STATIC_DCL int FDECL(gulpmm, (struct monst *,struct monst *,struct attack *));
STATIC_DCL int FDECL(explmm, (struct monst *,struct monst *,struct attack *));
STATIC_DCL int FDECL(mdamagem, (struct monst *,struct monst *,struct attack *));
STATIC_DCL void FDECL(mswingsm, (struct monst *, struct monst *, struct obj *));
STATIC_DCL void FDECL(noises,(struct monst *,struct attack *));
STATIC_DCL void FDECL(missmm,(struct monst *,struct monst *,struct attack *));
STATIC_DCL int FDECL(passivemm, (struct monst *, struct monst *, BOOLEAN_P, int, struct attack *));
STATIC_DCL int NDECL(beastmastery);
STATIC_DCL int NDECL(mountedCombat);

/* Needed for the special case of monsters wielding vorpal blades (rare).
 * If we use this a lot it should probably be a parameter to mdamagem()
 * instead of a global variable.
 */
static int dieroll;

/* returns mon_nam(mon) relative to other_mon; normal name unless they're
   the same, in which case the reference is to {him|her|it} self */
STATIC_OVL char *
mon_nam_too(outbuf, mon, other_mon)
char *outbuf;
struct monst *mon, *other_mon;
{
	Strcpy(outbuf, mon_nam(mon));
	if (mon == other_mon)
	    switch (pronoun_gender(mon)) {
	    case 0:	Strcpy(outbuf, "himself");  break;
	    case 1:	Strcpy(outbuf, "herself");  break;
	    default:	Strcpy(outbuf, "itself"); break;
	    }
	return outbuf;
}

STATIC_OVL void
noises(magr, mattk)
	register struct monst *magr;
	register struct	attack *mattk;
{
	boolean farq = (distu(magr->mx, magr->my) > 15);

	if(flags.soundok && (farq != far_noise || moves-noisetime > 10)) {
		far_noise = farq;
		noisetime = moves;
		You_hear("%s%s.",
			(mattk->aatyp == AT_EXPL) ? "an explosion" : "some noises",
			farq ? " in the distance" : "");
	}
}

STATIC_OVL
void
missmm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	struct attack *mattk;
{
	const char *fmt;
	char buf[BUFSZ], mdef_name[BUFSZ];

	if (vis) {
		if (!canspotmon(magr))
		    map_invisible(magr->mx, magr->my);
		if (!canspotmon(mdef))
		    map_invisible(mdef->mx, mdef->my);
		if (mdef->m_ap_type) seemimic(mdef);
		if (magr->m_ap_type) seemimic(magr);
		fmt = (could_seduce(magr,mdef,mattk) && !magr->mcan) ?
			"%s pretends to be friendly to" : "%s misses";
		Sprintf(buf, fmt, Monnam(magr));
		pline("%s %s.", buf, mon_nam_too(mdef_name, mdef, magr));
	} else  noises(magr, mattk);
}

/*
 *  fightm()  -- fight some other monster
 *
 *  Returns:
 *	0 - Monster did nothing.
 *	1 - If the monster made an attack.  The monster might have died.
 *
 *  There is an exception to the above.  If mtmp has the hero swallowed,
 *  then we report that the monster did nothing so it will continue to
 *  digest the hero.
 */
int
fightm(mtmp)		/* have monsters fight each other */
	register struct monst *mtmp;
{
	register struct monst *mon, *nmon;
	int result, has_u_swallowed;
	boolean conflict = Conflict && 
						couldsee(mtmp->mx,mtmp->my) && 
						(distu(mtmp->mx,mtmp->my) <= BOLT_LIM*BOLT_LIM) && 
						!resist(mtmp, RING_CLASS, 0, 0);
#ifdef LINT
	nmon = 0;
#endif
	/* perhaps the monster will resist Conflict */
	/* if(resist(mtmp, RING_CLASS, 0, 0))
	    return(0); */

	if ((mtmp->mtame || is_covetous(mtmp->data)) && !conflict)
	    return(0);

	if(u.ustuck == mtmp) {
	    /* perhaps we're holding it... */
	    if(itsstuck(mtmp))
		return(0);
	}
	has_u_swallowed = (u.uswallow && (mtmp == u.ustuck));

	for(mon = fmon; mon; mon = nmon) {
	    nmon = mon->nmon;
	    if(nmon == mtmp) nmon = mtmp->nmon;
	    /* Be careful to ignore monsters that are already dead, since we
	     * might be calling this before we've cleaned them up.  This can
	     * happen if the monster attacked a cockatrice bare-handedly, for
	     * instance.
	     */
	    if(mon != mtmp && !DEADMONSTER(mon)) {
		if(monnear(mtmp,mon->mx,mon->my)) {
		    if (!conflict && !mm_aggression(mtmp, mon))
		    	continue;
		    if(!u.uswallow && (mtmp == u.ustuck)) {
			if(!rn2(4)) {
			    pline("%s releases you!", Monnam(mtmp));
			    u.ustuck = 0;
			} else
			    break;
		    }

		    /* mtmp can be killed */
		    bhitpos.x = mon->mx;
		    bhitpos.y = mon->my;
		    notonhead = 0;
		    result = mattackm(mtmp,mon);

		    if (result & MM_AGR_DIED) return 1;	/* mtmp died */
		    /*
		     *  If mtmp has the hero swallowed, lie and say there
		     *  was no attack (this allows mtmp to digest the hero).
		     */
		    if (has_u_swallowed) return 0;

		    /* Allow attacked monsters a chance to hit back. Primarily
		     * to allow monsters that resist conflict to respond.
		     */
		    if ((result & MM_HIT) && !(result & MM_DEF_DIED) &&
			rn2(4) && mon->movement >= NORMAL_SPEED && mtmp->data != &mons[PM_NURSE]) {
				mon->movement -= NORMAL_SPEED;
				notonhead = 0;
				(void) mattackm(mon, mtmp);	/* return attack */
		    }
			
			//If was conflict and a miss, can continue to attack.  Otherwise, ignore you.
		    return (((result & MM_HIT) || !conflict) ? 1 : 0);
		}
	    }
	}
	return 0;
}

/*
 * mattackm() -- a monster attacks another monster.
 *
 * This function returns a result bitfield:
 *
 *	    --------- aggressor died
 *	   /  ------- defender died
 *	  /  /  ----- defender was hit
 *	 /  /  /
 *	x  x  x
 *
 *	0x4	MM_AGR_DIED
 *	0x2	MM_DEF_DIED
 *	0x1	MM_HIT
 *	0x0	MM_MISS
 *
 * Each successive attack has a lower probability of hitting.  Some rely on the
 * success of previous attacks.  ** this doen't seem to be implemented -dl **
 *
 * In the case of exploding monsters, the monster dies as well.
 */
int
mattackm(magr, mdef)
    register struct monst *magr,*mdef;
{
    int		    i,		/* loop counter */
		    tmp,	/* amour class difference */
		    tchtmp,	/* touch amour class difference */
		    strike = 0,	/* hit this attack (default to 0) */
		    attk,	/* attack attempted this time */
		    struck = 0,	/* hit at least once */
		    res[NATTK];	/* results of all attacks */
    struct attack   *mattk, alt_attk;
    struct permonst *pa, *pd;
	struct obj *oarmor;
	boolean derundspec = 0;
	
    if (!magr || !mdef) return(MM_MISS);		/* mike@genat */
    if (!magr->mcanmove || magr->msleeping) return(MM_MISS);
    pa = magr->data;  pd = mdef->data;

    /* Grid bugs and Bebeliths cannot attack at an angle. */
    if ((pa == &mons[PM_GRID_BUG] || pa == &mons[PM_BEBELITH]) && magr->mx != mdef->mx
						&& magr->my != mdef->my) return(MM_MISS);
	
	if(pa == &mons[PM_CLOCKWORK_SOLDIER] || pa == &mons[PM_CLOCKWORK_DWARF] || 
	   pa == &mons[PM_FABERGE_SPHERE] || pa == &mons[PM_FIREWORK_CART] || 
	   pa == &mons[PM_JUGGERNAUT] || pa == &mons[PM_ID_JUGGERNAUT]
	) if(magr->mx + xdir[(int)magr->mvar1] != mdef->mx || 
		   magr->my + ydir[(int)magr->mvar1] != mdef->my 
		) return(MM_MISS);
	
    /* Calculate the armour class differential. */
    tmp = find_mac(mdef) + 2*magr->m_lev/3;
	tmp += mdef->mstdy;
	if(is_prince(magr->data)) tmp += 5;
	if(is_lord(magr->data)) tmp += 2;
    tchtmp = base_mac(mdef) + 2*magr->m_lev/3;
	tchtmp += mdef->mstdy;
	if(is_prince(magr->data)) tchtmp += 5;
	if(is_lord(magr->data)) tchtmp += 2;
	oarmor = which_armor(magr, W_ARMG);
	if(oarmor){
		if(oarmor->otyp == GAUNTLETS_OF_POWER){
			if(oarmor->oartifact == ART_PREMIUM_HEART){
				tmp += 14;
				tchtmp += 14;
			} else {
				tmp += 3;
				tchtmp += 3;
			}
		} else if(oarmor->otyp == GAUNTLETS_OF_DEXTERITY){
			tmp += oarmor->spe;
			tchtmp += oarmor->spe;
		}
	}
	if(magr->data == &mons[PM_UVUUDAUM]){
		tmp += 20;
		tchtmp += 20;
	}
	if(magr->data == &mons[PM_DANCING_BLADE])
		tmp += 7;
    if (mdef->mconf || !mdef->mcanmove || mdef->msleeping) {
		if(mdef->data != &mons[PM_GIANT_TURTLE] || !mdef->mflee){
			tmp += 4;
			tchtmp += 4;
		}
		mdef->msleeping = 0;
    }
	if(mdef->data == &mons[PM_GIANT_TURTLE] && mdef->mflee){
		tchtmp += 20;
	}
	
	if(magr->mtame && !mdef->mtame){
		tmp += beastmastery();
		tchtmp += beastmastery();
		if(u.specialSealsActive&SEAL_COSMOS){
			tmp += spiritDsize();
			tchtmp += spiritDsize();
		}
		if(u.usteed && magr == u.usteed){
			tmp += mountedCombat();
			tchtmp += mountedCombat();
		}
	}
	
	tmp += magr->encouraged;
	tchtmp += magr->encouraged;
	if (wizard && magr->encouraged)
		pline("[%s +%d]", Monnam(magr), magr->encouraged);
	
    /* undetect monsters become un-hidden if they are attacked */
    if (mdef->mundetected) {
	mdef->mundetected = 0;
	newsym(mdef->mx, mdef->my);
	if(canseemon(mdef) && !sensemon(mdef)) {
	    if (u.usleep) You("dream of %s.",
				(mdef->data->geno & G_UNIQ) ?
				a_monnam(mdef) : makeplural(m_monnam(mdef)));
	    else pline("Suddenly, you notice %s.", a_monnam(mdef));
	}
    }

    /* Elves hate orcs. */
    if (is_elf(pa) && is_orc(pd)){
		tmp++;
		tchtmp++;
	}

    /* Set up the visibility of action */
    vis = (cansee(magr->mx,magr->my) && cansee(mdef->mx,mdef->my) && (canspotmon(magr) || canspotmon(mdef)));

    /*	Set flag indicating monster has moved this turn.  Necessary since a
     *	monster might get an attack out of sequence (i.e. before its move) in
     *	some cases, in which case this still counts as its move for the round
     *	and it shouldn't move again.
     */
    magr->mlstmv = monstermoves;
	
	if(!(magr->mfaction == ZOMBIFIED || magr->mfaction == SKELIFIED) && pa == &mons[PM_LILLEND] && rn2(2)){
		pa = find_mask(magr);
		if(!Blind && pa != &mons[PM_LILLEND]) pline("%s uses a %s mask!",Monnam(magr), pa->mname);
	}
	
    /* Now perform all attacks for the monster. */
    for (i = 0; i < NATTK; i++) {
        int tmphp = mdef->mhp;
	res[i] = MM_MISS;
	mattk = getmattk(pa, i, res, &alt_attk);
	otmp = (struct obj *)0;
	attk = 1;
	
	if (magr->data == &mons[PM_GRUE] && (i >= 2) && !((!levl[magr->mx][magr->my].lit && !(viz_array[magr->my][magr->mx] & TEMP_LIT1 && !(viz_array[magr->my][magr->mx] & TEMP_DRK1)))
		|| (levl[magr->mx][magr->my].lit && (viz_array[magr->my][magr->mx] & TEMP_DRK1 && !(viz_array[magr->my][magr->mx] & TEMP_LIT1)))))
		continue;
	    
	if(magr->mfaction == ZOMBIFIED || magr->mfaction == SKELIFIED || magr->mfaction == CRYSTALFIED){
		if(mattk->aatyp == AT_SPIT 
			|| mattk->aatyp == AT_BREA 
			|| mattk->aatyp == AT_GAZE 
			|| mattk->aatyp == AT_ARRW 
			|| mattk->aatyp == AT_MMGC 
			|| mattk->aatyp == AT_TNKR 
			|| mattk->aatyp == AT_SHDW 
			|| mattk->aatyp == AT_BEAM 
			|| mattk->aatyp == AT_MAGC
			|| (mattk->aatyp == AT_TENT && magr->mfaction == SKELIFIED)
			|| (i == 0 && 
				(mattk->aatyp == AT_CLAW || mattk->aatyp == AT_WEAP || mattk->aatyp == AT_XWEP || mattk->aatyp == AT_MARI) && 
				mattk->adtyp == AD_PHYS && 
				mattk->damn*mattk->damd/2 < (magr->m_lev/10+1)*max(magr->data->msize*2, 4)/2
			   )
			|| (!derundspec && mattk->aatyp == 0 && mattk->adtyp == 0 && mattk->damn == 0 && mattk->damd == 0)
			|| (!derundspec && i == NATTK-1 && (magr->mfaction == CRYSTALFIED || magr->mfaction == SKELIFIED))
		){
			if(i == 0){
				alt_attk.aatyp = AT_CLAW;
				alt_attk.adtyp = AD_PHYS;
				alt_attk.damn = magr->m_lev/10+1 + (magr->mfaction != ZOMBIFIED ? 1 : 0);
				alt_attk.damd = max(magr->data->msize*2, 4);
				mattk = &alt_attk;
			}
			else if(!derundspec && magr->mfaction == SKELIFIED){
				derundspec = TRUE;
				alt_attk.aatyp = AT_TUCH;
				alt_attk.adtyp = AD_SLOW;
				alt_attk.damn = 1;
				alt_attk.damd = max(magr->data->msize*2, 4);
				mattk = &alt_attk;
			}
			else if(!derundspec && magr->mfaction == CRYSTALFIED){
				derundspec = TRUE;
				alt_attk.aatyp = AT_TUCH;
				alt_attk.adtyp = AD_ECLD;
				alt_attk.damn = min(10,magr->m_lev/3);
				alt_attk.damd = 8;
				mattk = &alt_attk;
			}
			else continue;
		}
	}
	if(magr->mfaction == FRACTURED){
			if(!derundspec && 
				mattk->aatyp == 0 && mattk->adtyp == 0 && mattk->damn == 0 && mattk->damd == 0
			){
				derundspec = TRUE;
				alt_attk.aatyp = AT_CLAW;
				alt_attk.adtyp = AD_GLSS;
				alt_attk.damn = max(magr->m_lev/10+1, mattk->damn);
				alt_attk.damd = max(magr->data->msize*2, max(mattk->damd, 4));
				mattk = &alt_attk;
			}
			if(mattk->aatyp == AT_CLAW && 
				(mattk->adtyp == AD_PHYS || mattk->adtyp == AD_SAMU || mattk->adtyp == AD_SQUE)
			){
				alt_attk.aatyp = AT_CLAW;
				alt_attk.adtyp = AD_GLSS;
				alt_attk.damn = mattk->damn;
				alt_attk.damd = mattk->damd;
				mattk = &alt_attk;
			}
			if(mattk->aatyp == AT_GAZE)
				continue;
	}
	
	/*Plasteel helms cover the face and prevent bite attacks*/
	if((magr->misc_worn_check & W_ARMH) && which_armor(magr, W_ARMH) &&
		(((which_armor(magr, W_ARMH))->otyp) == PLASTEEL_HELM || ((which_armor(magr, W_ARMH))->otyp) == CRYSTAL_HELM || ((which_armor(magr, W_ARMH))->otyp) == PONTIFF_S_CROWN) && 
		(mattk->aatyp == AT_BITE || mattk->aatyp == AT_LNCK || (mattk->aatyp == AT_TENT && is_mind_flayer(magr->data)))
	) continue;
	if((magr->misc_worn_check & W_ARMC) && which_armor(magr, W_ARMC) &&
		(((which_armor(magr, W_ARMC))->otyp) == WHITE_FACELESS_ROBE || ((which_armor(magr, W_ARMC))->otyp) == BLACK_FACELESS_ROBE || ((which_armor(magr, W_ARMC))->otyp) == SMOKY_VIOLET_FACELESS_ROBE) && 
		(mattk->aatyp == AT_BITE || mattk->aatyp == AT_LNCK || (mattk->aatyp == AT_TENT && is_mind_flayer(magr->data)))
	) continue;
	
	switch (mattk->aatyp) {
	    case AT_DEVA:
	    case AT_WEAP:
		case AT_XWEP:
		case AT_MARI: /* weapon attacks */
#define MAINHAND (mattk->aatyp != AT_XWEP && mattk->aatyp != AT_MARI)

		if (!mattk->aatyp == AT_XWEP && magr->misc_worn_check & W_ARMS) {
			// Offhand attacks cannot be made while wearing a shield
			break;
		}

#ifdef TAME_RANGED_ATTACKS
		if (dist2(magr->mx,magr->my,mdef->mx,mdef->my) > 2){
			if (MAINHAND) {
				thrwmm(magr, mdef);
				if (tmphp > mdef->mhp){
					res[i] = MM_HIT;
					if(magr->mtame && canseemon(magr)) u.petattacked = TRUE;
				} else res[i] = MM_MISS;
				if (mdef->mhp < 1) res[i] = MM_DEF_DIED;
				if (magr->mhp < 1) res[i] = MM_AGR_DIED;
				break;
			} else break;
		}
#endif /*TAME_RANGED_ATTACKS*/
		if(mattk->aatyp != AT_MARI){
			if (magr->weapon_check == NEED_WEAPON || (MAINHAND ? (!MON_WEP(magr)) : (!MON_SWEP(magr)))) {
				magr->weapon_check = NEED_HTH_WEAPON;
				if (mon_wield_item(magr) != 0) return 0;
			}
		}

#ifdef TAME_RANGED_ATTACKS
		if (MAINHAND) {
			if (!MON_WEP(magr) ||
				is_launcher(MON_WEP(magr))) {
					/* implies no melee weapon found */
				if(thrwmm(magr, mdef)){
					if (tmphp > mdef->mhp){
						res[i] = MM_HIT;
						if(magr->mtame && canseemon(magr)) u.petattacked = TRUE;
					} else res[i] = MM_MISS;
					if (mdef->mhp < 1) res[i] = MM_DEF_DIED;
					if (magr->mhp < 1) res[i] = MM_AGR_DIED;
					break;
				}
			}
		}
#endif
		possibly_unwield(magr, FALSE);
		if(MAINHAND){
			otmp = MON_WEP(magr);
		} else if(mattk->aatyp == AT_XWEP){
			otmp = MON_SWEP(magr);
		} else if(mattk->aatyp == AT_MARI){
			int wcount = 0;
			for(otmp = magr->minvent; otmp; otmp = otmp->nobj){
				if((otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
					|| (otmp->otyp == IRON_CHAIN && magr->data == &mons[PM_CATHEZAR])
					) && !otmp->oartifact
					&& otmp != MON_WEP(magr) && otmp != MON_SWEP(magr)
					&& !otmp->owornmask
				) wcount++;
			}
			wcount -= i;
			if(MON_WEP(magr))
				wcount++;
			if(MON_SWEP(magr))
				wcount++;
			for(otmp = magr->minvent; otmp; otmp = otmp->nobj){
				if((otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
					|| (otmp->otyp == IRON_CHAIN && magr->data == &mons[PM_CATHEZAR])
					) && !otmp->oartifact
					&& otmp != MON_WEP(magr) && otmp != MON_SWEP(magr)
					&& !otmp->owornmask
					&& --wcount <= 0
				) break;
			}
		} else {
			otmp = MON_WEP(magr);
		}
#undef MAINHAND
		if (otmp) {
		    if (vis) mswingsm(magr, mdef, otmp);
		    tmp += hitval(otmp, mdef);
		    tchtmp += hitval(otmp, mdef);
			if(otmp->objsize - magr->data->msize > 0){
				tmp += -4*(otmp->objsize - magr->data->msize);
				tchtmp += -2*(otmp->objsize - magr->data->msize);
			}
		}
		/* fall through */
	    case AT_CLAW:
	    case AT_KICK:
	    case AT_BITE:
	    case AT_STNG:
	    case AT_TUCH:
	    case AT_SHDW:
	    case AT_BUTT:
	    case AT_TENT:
		/* Nymph that teleported away on first attack? */
		if (dist2(magr->mx,magr->my,mdef->mx,mdef->my) > 2)
		{
#ifdef TAME_RANGED_ATTACKS
                    break; /* might have more ranged attacks */
#else
		    return MM_MISS;
#endif /* TAME_RANGED_ATTACKS */
		}
		else goto meleeattack;
		case AT_BEAM:
		if(magr->data->maligntyp < 0 && Is_illregrd(&u.uz)) break;
		if(!mlined_up(magr, mdef, FALSE) || dist2(magr->mx,magr->my,mdef->mx,mdef->my) > BOLT_LIM*BOLT_LIM){
#ifdef TAME_RANGED_ATTACKS
                    break; /* might have more ranged attacks */
#else
		    return MM_MISS;
#endif /* TAME_RANGED_ATTACKS */
		} goto meleeattack;
		case AT_5SQR:
		if (distmin(magr->mx,magr->my,mdef->mx,mdef->my) > 5)
		{
#ifdef TAME_RANGED_ATTACKS
                    break; /* might have more ranged attacks */
#else
		    return MM_MISS;
#endif /* TAME_RANGED_ATTACKS */
		} goto meleeattack;
		case AT_LRCH:
		case AT_LNCK:
		if (dist2(magr->mx,magr->my,mdef->mx,mdef->my) > 5)
		{
#ifdef TAME_RANGED_ATTACKS
                    break; /* might have more ranged attacks */
#else
		    return MM_MISS;
#endif /* TAME_RANGED_ATTACKS */
		}
meleeattack:
		/* Monsters won't attack cockatrices physically if they
		 * have a weapon instead.  This instinct doesn't work for
		 * players, or under conflict or confusion. 
		 */
		if (!magr->mconf && !Conflict && ((otmp &&
			mattk->aatyp != AT_WEAP && mattk->aatyp != AT_XWEP && mattk->aatyp != AT_MARI && 
			mattk->aatyp != AT_DEVA
			) || (
			mattk->aatyp != AT_BEAM && mattk->adtyp != AD_STAR
			&& mattk->adtyp != AD_BLUD && mattk->adtyp != AD_SHDW
			&& mattk->adtyp != AT_MAGC && mattk->adtyp != AT_MMGC
			)) && touch_petrifies(mdef->data)
		) {
		    strike = 0;
		    break;
		}
		dieroll = rnd(20 + i*2);
		if(mattk->aatyp == AT_TUCH || mattk->aatyp == AT_5SQR || mattk->aatyp == AT_SHDW){
			strike = (tchtmp > dieroll);
		} else {
			strike = (tmp > dieroll);
		}
		/* KMH -- don't accumulate to-hit bonuses */
		if (otmp){
		    tmp -= hitval(otmp, mdef);
		    tchtmp -= hitval(otmp, mdef);
			if(otmp->objsize - magr->data->msize > 0){
				tmp += 4*(otmp->objsize - magr->data->msize);
				tchtmp += 2*(otmp->objsize - magr->data->msize);
			}
		}
		if (strike) {
		    res[i] = hitmm(magr, mdef, mattk);
			if(res[i] && magr->mtame && canseemon(magr)) u.petattacked = TRUE;
		    if((mdef->data == &mons[PM_BLACK_PUDDING] || mdef->data == &mons[PM_BROWN_PUDDING])
		       && otmp && otmp->obj_material == IRON
		       && mdef->mhp > 1 && !mdef->mcan)
		    {
			if (clone_mon(mdef, 0, 0)) {
			    if (vis) {
				char buf[BUFSZ];

				Strcpy(buf, Monnam(mdef));
				pline("%s divides as %s hits it!", buf, mon_nam(magr));
			    }
			}
		    }
			if(mattk->aatyp == AT_DEVA && !DEADMONSTER(mdef)){
				int deva = 1;
				while(!DEADMONSTER(mdef) && tmp > (dieroll = rnd(20+i+(deva++)*2))) res[i] = hitmm(magr, mdef, mattk);
			}
		} else
		    missmm(magr, mdef, mattk);
		if(mattk->aatyp == AT_MARI && i == 5){
			if(tmp > rnd(20 + i*2)){
				struct attack rend = {AT_HUGS, AD_WRAP, magr->data == &mons[PM_SHAKTARI] ? 8 : 4, 6};
				// res[i] = hitmm(magr, mdef, &rend);
			}
		}
		break;

	    case AT_HUGS:	/* automatic if prev two attacks succeed */
		strike = (i >= 2 && res[i-1] == MM_HIT && res[i-2] == MM_HIT);
		if (strike){
		    res[i] = hitmm(magr, mdef, mattk);
//			if(magr->mtame && canseemon(magr)) u.petattacked = TRUE;
		}
		break;

#ifdef TAME_RANGED_ATTACKS
	    case AT_BREA:
			if(is_true_dragon(magr->data)
				|| (is_half_dragon(magr->data) && magr->m_lev >= 14)
				|| (magr->data == &mons[PM_CERBERUS])
			) flags.drgn_brth = 1;
			if(magr->data == &mons[PM_MAMMON]) flags.mamn_brth = 1;
	        breamm(magr, mdef, mattk);
			flags.drgn_brth = 0;
			flags.mamn_brth = 0;
		if (tmphp > mdef->mhp){
			res[i] = MM_HIT;
			if(magr->mtame && canseemon(magr)) u.petattacked = TRUE;
		} else res[i] = MM_MISS;
		if (mdef->mhp < 1) res[i] = MM_DEF_DIED;
		if (magr->mhp < 1) res[i] = MM_AGR_DIED;
		break;

	    case AT_SPIT:
	        spitmm(magr, mdef, mattk);
		if (tmphp > mdef->mhp){
			res[i] = MM_HIT;
			if(magr->mtame && canseemon(magr)) u.petattacked = TRUE;
		} else res[i] = MM_MISS;
		if (mdef->mhp < 1) res[i] = MM_DEF_DIED;
		if (magr->mhp < 1) res[i] = MM_AGR_DIED;
		break;

	    case AT_ARRW:{
			if(magr->data->maligntyp < 0 && Is_illregrd(&u.uz)) break;
			if((mattk->adtyp != AD_SHDW || dist2(magr->mx,magr->my,mdef->mx,mdef->my)>2) && mlined_up(magr, mdef, FALSE)){
				int n;
				if (canseemon(magr)) pline("%s shoots.", Monnam(magr));
				for(n = d(mattk->damn, mattk->damd); n > 0; n--) firemm(magr, mdef, mattk);
				if(res[i] == MM_MISS){
					if (tmphp > mdef->mhp){
						res[i] = MM_HIT;
						if(magr->mtame && canseemon(magr)) u.petattacked = TRUE;
					} else res[i] = MM_MISS;
				}
				if (mdef->mhp < 1){
					res[i] = MM_DEF_DIED;
					break;
				}
				if (magr->mhp < 1){
					res[i] = MM_AGR_DIED;
					break;
				}
			}
		}break;
		
	    case AT_TNKR:{
			if(!magr->mspec_used){
				struct monst *mlocal;
				int mdx=0, mdy=0, i;
				
				if(mdef->mx - magr->mx < 0) mdx = -1;
				else if(mdef->mx - magr->mx > 0) mdx = +1;
				if(mdef->my - magr->my < 0) mdy = -1;
				else if(mdef->my - magr->my > 0) mdy = +1;
				
				mlocal = makemon(&mons[PM_CLOCKWORK_SOLDIER+rn2(3)], magr->mx + mdx, magr->my + mdy, MM_ADJACENTOK|MM_ADJACENTSTRICT);
				
				if(mlocal){
					for(i=0;i<8;i++) if(xdir[i] == mdx && ydir[i] == mdy) break;
					mlocal->mvar1 = i;
					
					magr->mspec_used = rnd(6);
				}
			}
		}break;
#endif /* TAME_RANGED_ATTACKS */

	    case AT_WDGZ:
			strike = 0;	/* will not wake up a sleeper */
			res[i] = MM_MISS;
			continue;
		break;
	    case AT_GAZE:
		strike = 0;	/* will not wake up a sleeper */
		if(is_weeping(magr->data)){
			res[i] = MM_MISS;
			break;
		}
		if(!mon_can_see_mon(magr, mdef)){
			res[i] = MM_MISS;
			continue;
		}
		res[i] = gazemm(magr, mdef, mattk);
		if(res[i] && magr->mtame && canseemon(magr)) u.petattacked = TRUE;
		break;

	    case AT_EXPL:
		if (distmin(magr->mx,magr->my,mdef->mx,mdef->my) > 1) break;
		res[i] = explmm(magr, mdef, mattk);
		if(res[i] && magr->mtame && canseemon(magr)) u.petattacked = TRUE;
		if (is_fern_spore(magr->data)) spore_dies(magr);
		if (res[i] == MM_MISS) { /* cancelled--no attack */
		    strike = 0;
		    attk = 0;
		} else
		    strike = 1;	/* automatic hit */
		break;

	    case AT_ENGL:
		if (distmin(magr->mx,magr->my,mdef->mx,mdef->my) > 1) break;
#ifdef STEED
		if (u.usteed && (mdef == u.usteed)) {
		    strike = 0;
		    break;
		} 
#endif
		/* Engulfing attacks are directed at the hero if
		 * possible. -dlc
		 */
		if (u.uswallow && magr == u.ustuck)
		    strike = 0;
		else {
		    if ((strike = (tmp > rnd(20+i)))){
				res[i] = gulpmm(magr, mdef, mattk);
				if(magr->mtame && canseemon(magr)) u.petattacked = TRUE;
		    } else
				missmm(magr, mdef, mattk);
		}
		break;

#ifdef TAME_RANGED_ATTACKS
            case AT_MAGC:
            case AT_MMGC:{
				int temp=0;
				int range;
				if(magr->data->maligntyp < 0 && Is_illregrd(&u.uz)) break;
				
				if( magr->data == &mons[PM_DEMOGORGON] && distmin(magr->mx, magr->my, mdef->mx, mdef->my) > 1 && !magr->mflee && rn2(6)) 
					break; //cast spells more rarely if he's in melee range
				if(is_orc(magr->data) || 
					magr->data == &mons[PM_HEDROW_WARRIOR] || 
					magr->data == &mons[PM_MINOTAUR_PRIESTESS]
				) range = (BOLT_LIM*BOLT_LIM)/2;
				else if(magr->data == &mons[PM_CHROMATIC_DRAGON] || magr->data == &mons[PM_PLATINUM_DRAGON]) range = 2;
				else range = (BOLT_LIM*BOLT_LIM);
				
				if (dist2(magr->mx,magr->my,mdef->mx,mdef->my) > range) break;
				
				if (dist2(magr->mx,magr->my,mdef->mx,mdef->my) > 2 && mattk->adtyp != AD_SPEL && mattk->adtyp != AD_CLRC && mattk->adtyp != AD_STAR)
					res[i] = buzzmm(magr, mdef, mattk, magr->m_lev);
				else {
					res[i] = castmm(magr, mdef, mattk);
				}
				if(res[i] && magr->mtame && canseemon(magr)) u.petattacked = TRUE;
				if (res[i] & MM_DEF_DIED)
				if( pa == &mons[PM_ASMODEUS] && !rn2(3) ) return 3;
				return (MM_DEF_DIED |
					(grow_up(magr,mdef) ? 0 : MM_AGR_DIED));
			}break;
#endif /* TAME_RANGED_ATTACKS */

	    default:		/* no attack */
		strike = 0;
		attk = 0;
		break;
	}

	if (attk && !(res[i] & MM_AGR_DIED) &&
	    dist2(magr->mx, magr->my, mdef->mx, mdef->my) < 3)
	    res[i] = passivemm(magr, mdef, strike, res[i] & MM_DEF_DIED, mattk);

	if(res[i] && magr->mtame && canseemon(magr)) u.petattacked = TRUE;
	if (res[i] & MM_DEF_DIED) return res[i];

	/*
	 *  Wake up the defender.  NOTE:  this must follow the check
	 *  to see if the defender died.  We don't want to modify
	 *  unallocated monsters!
	 */
	if (strike) mdef->msleeping = 0;

	if (res[i] & MM_AGR_DIED)  return res[i];
	/* return if aggressor can no longer attack */
	if (!magr->mcanmove || magr->msleeping) return res[i];
	if (res[i] & MM_HIT) struck = 1;	/* at least one hit */
    }

	if (mdef->data == &mons[PM_URANIUM_IMP] && !mdef->mcan) {
	    /* avoid mysterious force message by not using tele_restrict() */
	    if (canseemon(mdef)) pline("%s %s reality!", Monnam(mdef),
		    level.flags.noteleport ? "tries to warp" : "warps");
	    if (!level.flags.noteleport) {
		coord mm;
		rloc(magr, FALSE);
		enexto(&mm, magr->mx, magr->my, &mons[PM_URANIUM_IMP]);
		rloc_to(mdef, mm.x, mm.y);
	    }
	return res[i];
	}
	
    return(struck ? MM_HIT : MM_MISS);
}

/* monster attempts ranged weapon attack against monster */
int
thrwmm(magr, mdef)
struct monst *magr;
struct monst *mdef;
{
	struct obj *otmp, *mwep;
	xchar x, y;
	schar skill;
    int multishot, mhp;
	const char *onm;

	if(magr->data->maligntyp < 0 && Is_illregrd(&u.uz)) return 0;
	
	/* Rearranged beginning so monsters can use polearms not in a line */
	if (magr->weapon_check == NEED_WEAPON || !MON_WEP(magr)) {
	    magr->weapon_check = NEED_RANGED_WEAPON;
	    /* mon_wield_item resets weapon_check as appropriate */
	if(mon_wield_item(magr) != 0) return MM_MISS;
	}

	/* Pick a weapon */
	otmp = select_rwep(magr);
    if (!otmp) return MM_MISS;

	if (is_pole(otmp)) {
		int dam, hitv, vis = canseemon(magr);

		if (dist2(magr->mx, magr->my, mdef->mx, mdef->my) > POLE_LIM ||
			!m_cansee(magr, mdef->mx, mdef->my))
	    	return MM_MISS;	/* Out of range, or intervening wall */

		if (vis) {
			onm = xname(otmp);
			pline("%s %s %s.", Monnam(magr), otmp->otyp == AKLYS ? "throws" : "thrusts",
			      obj_is_pname(otmp) ? the(onm) : an(onm));
	    }

	    return ohitmon(magr, mdef, otmp, 0, FALSE);

	}

	x = magr->mx;
	y = magr->my;
	
	/*
	 * Check for being lined up and for friendlies in the line
	 * of fire:
	 */
	if (!mlined_up(magr, mdef, FALSE))
	    return MM_MISS;

	skill = objects[otmp->otyp].oc_skill;
	mwep = MON_WEP(magr);		/* wielded weapon */

	/* Multishot calculations */
	multishot = 1;
	if ((ammo_and_launcher(otmp, mwep) || is_blaster(otmp) || skill == P_DAGGER ||
		skill == -P_DART || skill == -P_SHURIKEN) && !magr->mconf) {
	    /* Assumes lords are skilled, princes are expert */
	    if (is_prince(magr->data)) multishot += 2;
	    else if (is_lord(magr->data)) multishot++;

	    switch (monsndx(magr->data)) {
	    case PM_RANGER:
		    multishot++;
		    break;
	    case PM_ROGUE:
		    if (skill == P_DAGGER) multishot++;
		    break;
	    case PM_NINJA:
	    case PM_SAMURAI:
		    if (otmp->otyp == YA && mwep &&
			mwep->otyp == YUMI) multishot++;
		    break;
	    default:
		break;
	    }
	    /* racial bonus */
	    if ((is_elf(magr->data) &&
		    otmp->otyp == ELVEN_ARROW &&
		    mwep && mwep->otyp == ELVEN_BOW) ||
		(is_orc(magr->data) &&
		    otmp->otyp == ORCISH_ARROW &&
		    mwep && mwep->otyp == ORCISH_BOW))
		multishot++;

		if(is_blaster(otmp) && otmp == mwep){
			if((long)multishot > otmp->ovar1) multishot = (int)otmp->ovar1;
		} else if ((long)multishot > otmp->quan) multishot = (int)otmp->quan;
	    if (multishot < 1) multishot = 1;
	    else multishot = rnd(multishot);
// #ifdef FIREARMS
		// if (mwep && objects[mwep->otyp].oc_rof && is_launcher(mwep))
		    // multishot += objects[mwep->otyp].oc_rof;
		if (((is_blaster(otmp) && otmp == mwep) || ammo_and_launcher(otmp, mwep))
			&& objects[(mwep->otyp)].oc_rof && mwep->otyp != RAYGUN && mwep->altmode != WP_MODE_SINGLE
		) {
			if (mwep->otyp != RAYGUN && mwep->altmode == WP_MODE_BURST)
				multishot += objects[(mwep->otyp)].oc_rof / 3;
			/* else it is full auto */
			else multishot += (objects[(mwep->otyp)].oc_rof - 1);
		}
		/* single shot, don't add anything */
// #endif
		if(is_blaster(otmp) && otmp == mwep){
			if((long)multishot > otmp->ovar1) multishot = (int)otmp->ovar1;
		} else if ((long)multishot > otmp->quan) multishot = (int)otmp->quan;
		
	    if (multishot < 1) multishot = 1;
	}

	if(is_blaster(otmp) && otmp == mwep){
		otmp = mksobj(otmp->otyp == ARM_BLASTER ? HEAVY_BLASTER_BOLT : BLASTER_BOLT, TRUE, FALSE);
		otmp->blessed = mwep->blessed;
		otmp->cursed = mwep->cursed;
		otmp->spe = mwep->spe;
		otmp->quan = multishot;
		mwep->ovar1 -= multishot;
	}

	if (canseemon(magr)) {
	    char onmbuf[BUFSZ];

	    if (multishot > 1) {
		/* "N arrows"; multishot > 1 implies otmp->quan > 1, so
		   xname()'s result will already be pluralized */
		Sprintf(onmbuf, "%d %s", multishot, xname(otmp));
		onm = onmbuf;
	    } else {
		/* "an arrow" */
		onm = singular(otmp, xname);
		onm = obj_is_pname(otmp) ? the(onm) : an(onm);
	    }
	    m_shot.s = ammo_and_launcher(otmp,mwep) ? TRUE : FALSE;
	    pline("%s %s %s!", Monnam(magr),
#ifdef FIREARMS
	      m_shot.s ? is_bullet(otmp) ? "fires" : "shoots" : "throws",
	      onm);
#else
		  m_shot.s ? "shoots" : "throws", onm);
#endif
	    m_shot.o = otmp->otyp;
	} else {
	    m_shot.o = STRANGE_OBJECT;	/* don't give multishot feedback */
	}

    mhp = mdef->mhp;
	m_shot.n = multishot;
	for (m_shot.i = 1; m_shot.i <= m_shot.n; m_shot.i++)
	    if(m_shot.s && objects[(mwep->otyp)].oc_range) m_throw(magr, magr->mx, magr->my, sgn(tbx), sgn(tby),
		    objects[(mwep->otyp)].oc_range, otmp,
		    TRUE);
		else m_throw(magr, magr->mx, magr->my, sgn(tbx), sgn(tby),
		    distmin(magr->mx, magr->my, mdef->mx, mdef->my), otmp,
		    TRUE);
	m_shot.n = m_shot.i = 0;
	m_shot.o = STRANGE_OBJECT;
	m_shot.s = FALSE;

	//nomul(0, NULL); this is the monster function

    return (mdef->mhp < 1 ? MM_DEF_DIED : 0) | (mdef->mhp < mhp ? MM_HIT : 0) |
	   (magr->mhp < 1 ? MM_AGR_DIED : 0);
}

/* Returns the result of mdamagem(). */
STATIC_OVL int
hitmm(magr, mdef, mattk)
	register struct monst *magr,*mdef;
	struct	attack *mattk;
{
	/* Nearby monsters may be awakened */
	if(!is_silent_mon(magr) || !helpless(mdef)) wake_nearto(mdef->mx, mdef->my, combatNoise(magr->data));
	
	if(vis){
		int compat;
		char buf[BUFSZ], mdef_name[BUFSZ];

		if (!canspotmon(magr))
		    map_invisible(magr->mx, magr->my);
		if (!canspotmon(mdef))
		    map_invisible(mdef->mx, mdef->my);
		if(mdef->m_ap_type) seemimic(mdef);
		if(magr->m_ap_type) seemimic(magr);
		if((compat = could_seduce(magr,mdef,mattk)) && !magr->mcan) {
			Sprintf(buf, "%s %s", Monnam(magr),
				!is_blind(mdef) ? "smiles at" : "talks to");
			pline("%s %s %s.", buf, mon_nam(mdef),
				compat == 2 ?
					"engagingly" : "seductively");
		} else {
		    char magr_name[BUFSZ];

		    Strcpy(magr_name, Monnam(magr));
		    switch (mattk->aatyp) {
			case AT_BEAM:
				Sprintf(buf,"%s blasts", magr_name);
				break;
			case AT_LNCK:
			case AT_BITE:
				Sprintf(buf,"%s bites", magr_name);
				break;
			case AT_STNG:
				Sprintf(buf,"%s stings", magr_name);
				break;
			case AT_BUTT:
				Sprintf(buf,"%s butts", magr_name);
				break;
			case AT_5SQR:
			case AT_TUCH:
				if (is_weeping(magr->data)) {
					Sprintf(buf,"%s is touching", magr_name);
				} else if(mattk->adtyp == AD_SHDW || mattk->adtyp == AD_STAR || mattk->adtyp == AD_BLUD){
					if(magr->mcan) goto defaultmmhit;
					else Sprintf(buf,"%s slashes", magr_name);
				} else {
					Sprintf(buf,"%s touches", magr_name);
				}
				break;
			case AT_TENT:
				Sprintf(buf, "%s tentacles suck",
					s_suffix(magr_name));
				break;
			case AT_WHIP:
				pline("%s barbed whips lash", magr_name);
			break;
			case AT_HUGS:
				if (magr != u.ustuck) {
				    Sprintf(buf,"%s squeezes", magr_name);
				    break;
				}
			case AT_NONE:
				break;
			default:
defaultmmhit:
				if (is_weeping(magr->data)) {
					Sprintf(buf,"%s is hitting", magr_name);
				} else {
					Sprintf(buf,"%s hits", magr_name);
				}
				break;
		    }
		    pline("%s %s.", buf, mon_nam_too(mdef_name, mdef, magr));
		}
	} else  noises(magr, mattk);
	return(mdamagem(magr, mdef, mattk));
}

/* Returns the same values as mdamagem(). */
int
gazemm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	struct attack *mattk;
{
	char buf[BUFSZ];

	if(magr->data->maligntyp < 0 && Is_illregrd(&u.uz)) return MM_MISS;
	
	if(vis) {
		/* the gaze attack of weeping (arch)angels isn't active like others */
		if (is_weeping(magr->data)) {
			if (mon_reflects(mdef, (char *)0)) {
				return (MM_MISS);
			} else {
				Sprintf(buf,"%s is staring at", Monnam(magr));
				pline("%s %s.", buf, mon_nam(mdef));
			}
		}
	}

	if (magr->mcan || is_blind(magr) ||
	    (magr->minvis && !perceives(mdef->data)) ||
	    is_blind(mdef) || mdef->msleeping) {
	    // if(vis && !is_weeping(magr->data)) pline("but nothing happens.");
	    return(MM_MISS);
	}
	/* call mon_reflects 2x, first test, then, if visible, print message */
	if (magr->data == &mons[PM_MEDUSA] && mon_reflects(mdef, (char *)0)) {
	    if (canseemon(mdef))
		(void) mon_reflects(mdef,
				    "The gaze is reflected away by %s %s.");
//	    if (!is_blind(mdef)) {
		if(mdef->mfaction != FRACTURED || !rn2(8)){
			if (mon_reflects(magr, (char *)0)) {
				if (canseemon(magr))
				(void) mon_reflects(magr,
						"The gaze is reflected away by %s %s.");
				return (MM_MISS);
			}
			if (mdef->minvis && !perceives(magr->data)) {
				if (canseemon(magr)) {
				pline("%s doesn't seem to notice that %s gaze was reflected.",
					  Monnam(magr), mhis(magr));
				}
				return (MM_MISS);
			}
			if (canseemon(magr))
				pline("%s is turned to stone!", Monnam(magr));
			monstone(magr);
			if (magr->mhp > 0) return (MM_MISS);
			return (MM_AGR_DIED);
	    }
	}

	return(mdamagem(magr, mdef, mattk));
}

/* Returns the same values as mattackm(). */
STATIC_OVL int
gulpmm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	register struct	attack *mattk;
{
	xchar	ax, ay, dx, dy;
	int	status;
	char buf[BUFSZ];
	struct obj *obj;

	if (mdef->data->msize >= MZ_HUGE) return MM_MISS;

	if (vis) {
		Sprintf(buf,"%s swallows", Monnam(magr));
		pline("%s %s.", buf, mon_nam(mdef));
	}
	for (obj = mdef->minvent; obj; obj = obj->nobj)
	    (void) snuff_lit(obj);

	/*
	 *  All of this maniuplation is needed to keep the display correct.
	 *  There is a flush at the next pline().
	 */
	ax = magr->mx;
	ay = magr->my;
	dx = mdef->mx;
	dy = mdef->my;
	/*
	 *  Leave the defender in the monster chain at it's current position,
	 *  but don't leave it on the screen.  Move the agressor to the def-
	 *  ender's position.
	 */
	remove_monster(ax, ay);
	place_monster(magr, dx, dy);
	newsym(ax,ay);			/* erase old position */
	newsym(dx,dy);			/* update new position */

	status = mdamagem(magr, mdef, mattk);

	if ((status & MM_AGR_DIED) && (status & MM_DEF_DIED)) {
	    ;					/* both died -- do nothing  */
	}
	else if (status & MM_DEF_DIED) {	/* defender died */
	    /*
	     *  Note:  remove_monster() was called in relmon(), wiping out
	     *  magr from level.monsters[mdef->mx][mdef->my].  We need to
	     *  put it back and display it.	-kd
	     */
	    place_monster(magr, dx, dy);
	    newsym(dx, dy);
	}
	else if (status & MM_AGR_DIED) {	/* agressor died */
	    place_monster(mdef, dx, dy);
	    newsym(dx, dy);
	}
	else {					/* both alive, put them back */
	    if (cansee(dx, dy))
		pline("%s is regurgitated!", Monnam(mdef));

	    place_monster(magr, ax, ay);
	    place_monster(mdef, dx, dy);
	    newsym(ax, ay);
	    newsym(dx, dy);
	}

	return status;
}

STATIC_OVL int
explmm(magr, mdef, mattk)
	register struct monst *magr, *mdef;
	register struct	attack *mattk;
{
	int result;

	if (magr->mcan)
	    return MM_MISS;

	if(cansee(magr->mx, magr->my))
		pline("%s explodes!", Monnam(magr));
	else	noises(magr, mattk);
	
	if(!is_fern_spore(magr->data)) result = mdamagem(magr, mdef, mattk);
	else{
		mondead(magr);
		if(magr->data==&mons[PM_SWAMP_FERN_SPORE]) explode(magr->mx, magr->my, 9, d((int)mattk->damn, (int)mattk->damd), MON_EXPLODE, EXPL_MAGICAL);
		else if(magr->data==&mons[PM_BURNING_FERN_SPORE])
			explode(magr->mx, magr->my, 8, d((int)mattk->damn, (int)mattk->damd), MON_EXPLODE, EXPL_YELLOW);
		else explode(magr->mx, magr->my, 7, d((int)mattk->damn, (int)mattk->damd), MON_EXPLODE, EXPL_NOXIOUS);
		if (magr->mhp > 0) return result;
		else return result | MM_AGR_DIED;
	}

	/* Kill off agressor if it didn't die. */
	if (!(result & MM_AGR_DIED)) {
	    mondead(magr);
	    if (magr->mhp > 0) return result;	/* life saved */
	    result |= MM_AGR_DIED;
	}
	if (magr->mtame)	/* give this one even if it was visible */
	    You(brief_feeling, "melancholy");

	return result;
}

/*
 *  See comment at top of mattackm(), for return values.
 */
STATIC_OVL int
mdamagem(magr, mdef, mattk)
	register struct monst	*magr, *mdef;
	register struct attack	*mattk;
{
	struct obj *obj;
	char buf[BUFSZ];
	struct permonst *pa = magr->data, *pd = mdef->data;
	int armpro, num, tmp;
	boolean cancelled;
	boolean phasearmor = FALSE;
	boolean weaponhit = (mattk->aatyp == AT_WEAP || mattk->aatyp == AT_XWEP || mattk->aatyp == AT_DEVA || mattk->aatyp == AT_MARI);
	struct attack alt_attk;
	
	if(weaponhit && mattk->adtyp != AD_PHYS) tmp = 0;
	else if(magr->mflee && pa == &mons[PM_BANDERSNATCH]) tmp = d((int)mattk->damn, 2*(int)mattk->damd);
	else  tmp = d((int)mattk->damn, (int)mattk->damd);

//ifdef BARD
	if(tmp > 0){
		tmp += magr->encouraged;
		if(tmp < 0) tmp = 0;
	} else {
		tmp -= magr->encouraged;
		if(tmp > 0) tmp = 0;
	}
//endif

	if (touch_petrifies(pd) && !resists_ston(magr) &&
		mattk->adtyp != AD_STAR && mattk->adtyp != AD_BLUD && mattk->adtyp != AD_SHDW
	) {
	    long protector = attk_protection((int)mattk->aatyp),
		 wornitems = magr->misc_worn_check;

	    /* wielded weapon gives same protection as gloves here */
	    if (otmp != 0) wornitems |= W_ARMG;

	    if (protector == 0L ||
		  (protector != ~0L && 
		  (wornitems & protector) != protector)
		) {
		if (poly_when_stoned(pa)) {
		    mon_to_stone(magr);
		    return MM_HIT; /* no damage during the polymorph */
		}
		if (vis) pline("%s turns to stone!", Monnam(magr));
		monstone(magr);
		if (magr->mhp > 0) return 0;
		else if (magr->mtame && !vis)
		    You(brief_feeling, "peculiarly sad");
		return MM_AGR_DIED;
	    }
	}

	/* cancellation factor is the same as when attacking the hero */
	armpro = magic_negation(mdef);
	cancelled = magr->mcan || !((rn2(3) >= armpro) || !rn2(50));

	switch (weaponhit ? AD_PHYS : mattk->adtyp) {
	    case AD_DGST:
		/* eating a Rider or its corpse is fatal */
		if (is_rider(mdef->data)) {
		    if (vis)
			pline("%s %s!", Monnam(magr),
			      mdef->data == &mons[PM_FAMINE] ?
				"belches feebly, shrivels up and dies" :
			      mdef->data == &mons[PM_PESTILENCE] ?
				"coughs spasmodically and collapses" :
				"vomits violently and drops dead");
		    mondied(magr);
		    if (magr->mhp > 0) return 0;	/* lifesaved */
		    else if (magr->mtame && !vis)
			You(brief_feeling, "queasy");
		    return MM_AGR_DIED;
		}
		if(flags.verbose && flags.soundok) verbalize("Burrrrp!");
		tmp = mdef->mhp;
		/* Use up amulet of life saving */
		if (!!(obj = mlifesaver(mdef))) m_useup(mdef, obj);

		/* Is a corpse for nutrition possible?  It may kill magr */
		if (!corpse_chance(mdef, magr, TRUE) || magr->mhp < 1)
		    break;

		/* Pets get nutrition from swallowing monster whole.
		 * No nutrition from G_NOCORPSE monster, eg, undead.
		 * DGST monsters don't die from undead corpses
		 */
		num = monsndx(mdef->data);
		if (magr->mtame && !magr->isminion &&
		    !(mvitals[num].mvflags & G_NOCORPSE || mdef->mvanishes)) {
		    struct obj *virtualcorpse = mksobj(CORPSE, FALSE, FALSE);
		    int nutrit;

		    virtualcorpse->corpsenm = num;
		    virtualcorpse->owt = weight(virtualcorpse);
		    nutrit = dog_nutrition(magr, virtualcorpse);
		    dealloc_obj(virtualcorpse);

		    /* only 50% nutrition, 25% of normal eating time */
		    if (magr->meating > 1) magr->meating = (magr->meating+3)/4;
		    if (nutrit > 1) nutrit /= 2;
		    EDOG(magr)->hungrytime += nutrit;
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_STUN:
		if (magr->mcan) break;
		if(mattk->aatyp == AT_GAZE){
			if(canseemon(magr)){
				Sprintf(buf,"%s gazes at", Monnam(magr));
				pline("%s %s...", buf, mon_nam(mdef));
			}
			tmp = 0;
		}
		if (canseemon(mdef))
		    pline("%s %s for a moment.", Monnam(mdef),
			  makeplural(stagger(mdef->data, "stagger")));
		mdef->mstun = 1;
		goto physical;
	    case AD_LEGS:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
		goto physical;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_HEAL:
			if(mdef->mpeaceful == magr->mpeaceful){
				mdef->mhp += d(magr->mhpmax/8,1);
				if(mdef->mhpmax < mdef->mhp) mdef->mhp = mdef->mhpmax;
				if(canseemon(mdef)) pline("%s looks better!", mon_nam(mdef));
				return MM_HIT;
			}
			else goto physical;
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_CHKH:
			tmp += u.chokhmah;
	    case AD_WERE:
	    case AD_PHYS:
			//defered special attacks
	    case AD_VORP:
	    case AD_TENT:
		case AD_SHRD:
	    case AD_HODS:
	    case AD_CHRN:
	    case AD_SUCK:
	    case AD_MALK:
	    case AD_UVUU:
	    case AD_TELE:
physical:{
		struct obj *oarm;
		oarm = which_armor(magr, W_ARMG);
		if (mattk->aatyp == AT_KICK && thick_skinned(pd)) {
		    tmp = 0;
		} else if(weaponhit) {
		    if(otmp) {
				if (otmp->otyp == CORPSE &&
					touch_petrifies(&mons[otmp->corpsenm]))
					goto do_stone;
				/* WAC -- Real weapon?
				 * Could be stuck with a cursed bow/polearm it wielded
				 */
				if (/* if you strike with a bow... */
					is_launcher(otmp) ||
					/* or strike with a missile in your hand... */
					((is_missile(otmp) || is_ammo(otmp)) &&
						!(otmp->otyp == CHAKRAM)
					) ||
					/* houchou not thrown */
					(otmp->oartifact == ART_HOUCHOU) ||
					/* lightsaber that isn't lit ;) */
					(is_lightsaber(otmp) && !litsaber(otmp)) ||
					/* WAC -- or using a pole at short range... */
					(is_pole(otmp) &&
						otmp->otyp != AKLYS && 
						otmp->otyp != FORCE_PIKE && 
						otmp->otyp != NAGINATA && 
						otmp->oartifact != ART_WEBWEAVER_S_CROOK && 
						otmp->oartifact != ART_SILENCE_GLAIVE && 
						otmp->oartifact != ART_HEARTCLEAVER && 
						otmp->oartifact != ART_SOL_VALTIVA && 
						otmp->oartifact != ART_SHADOWLOCK && 
						otmp->oartifact != ART_PEN_OF_THE_VOID
				)) {
					/* then do only 1-2 points of damage */
					if (insubstantial(mdef->data) && !insubstantial_aware(mdef, otmp, FALSE)) tmp = 0;
					else if(otmp->oartifact == ART_LIECLEAVER)
						tmp = 2*(rnd(12) + rnd(10) + otmp->spe);
					else if(otmp->oartifact == ART_ROGUE_GEAR_SPIRITS)
						tmp = 2*(rnd(bigmonst(mdef->data) ? 2 : 5) + otmp->spe);
					else tmp = rnd(2);
					
					if(otmp && (otmp->obj_material == SILVER || arti_silvered(otmp)) && hates_silver(pd) &&
						!(is_lightsaber(otmp) && litsaber(otmp))
					)
						tmp += rnd(20);
					if(otmp && (otmp->obj_material == IRON) && hates_iron(pd) &&
						!(is_lightsaber(otmp) && litsaber(otmp))
					)
						tmp += rnd(mdef->m_lev);
					if(otmp && is_unholy(otmp) && hates_unholy(pd))
						tmp += rnd(9);
				} else {
					tmp += dmgval(otmp, mdef, 0);
					if(otmp && ((is_lightsaber(otmp) && litsaber(otmp)) || arti_shining(otmp))) phasearmor = TRUE;
				}
				
				if(magr->data == &mons[PM_DANCING_BLADE])
					tmp += 7;
				
				if(resist_attacks(mdef->data))
					tmp = 0;
				/* WAC Weres get seared */
				if(otmp && (otmp->obj_material == SILVER || arti_silvered(otmp)) && hates_silver(pd) &&
					!(is_lightsaber(otmp) && litsaber(otmp))
				) {
					if (vis) pline("The silver sears %s!", mon_nam(mdef));
				}
				if(otmp && (otmp->obj_material == IRON) && hates_iron(pd) &&
					!(is_lightsaber(otmp) && litsaber(otmp))
				) {
					if (vis) pline("The cold-iron sears %s!", mon_nam(mdef));
				}
				if(otmp && is_unholy(otmp) && hates_unholy(pd)) {
					if (vis) pline("The curse sears %s!", mon_nam(mdef));
				}
				if(oarm && tmp && oarm->otyp == GAUNTLETS_OF_POWER){
					tmp += 8;
					if(otmp && bimanual(otmp,magr->data)) tmp += 4;
				}
				if (otmp && otmp->oartifact) {
					(void)artifact_hit(magr,mdef, otmp, &tmp, dieroll);
					if (mdef->mhp <= 0 || migrating_mons == mdef)
					return (MM_DEF_DIED |
						(grow_up(magr,mdef) ? 0 : MM_AGR_DIED));
				}
				if (otmp && tmp)
					mrustm(magr, mdef, otmp);
		    } else if(magr->data == &mons[PM_FORMIAN_CRUSHER]){
				tmp += d(3*((int)mattk->damn), (int)mattk->damd);
			}
			// tack on bonus elemental damage, if applicable
			if (mattk->adtyp != AD_PHYS){
				alt_attk.aatyp = AT_NONE;
				if(mattk->adtyp == AD_OONA)
					alt_attk.adtyp = u.oonaenergy;
				else if(mattk->adtyp == AD_HDRG)
					alt_attk.adtyp = magr->mvar1;
				else if(mattk->adtyp == AD_RBRE){
					switch(rn2(3)){
						case 0:
							alt_attk.adtyp = AD_FIRE;
						break;
						case 1:
							alt_attk.adtyp = AD_COLD;
						break;
						case 2:
							alt_attk.adtyp = AD_ELEC;
						break;
					}
				} else alt_attk.adtyp = mattk->adtyp;
				alt_attk.damn = mattk->damn;
				alt_attk.damd = mattk->damd;
				// switch (alt_attk.adtyp)
				// {
				// case AD_FIRE:
				// case AD_COLD:
				// case AD_ELEC:
				// case AD_ACID:
					// alt_attk.damn = 4;
					// alt_attk.damd = 6;
					// break;
				// case AD_EFIR:
				// case AD_ECLD:
				// case AD_EELC:
				// case AD_EACD:
					// alt_attk.damn = 3;
					// alt_attk.damd = 7;
					// break;
				// case AD_STUN:
					// alt_attk.damn = 1;
					// alt_attk.damd = 4;
					// break;
				// default:
					// alt_attk.damn = 0;
					// alt_attk.damd = 0;
					// break;
				// }
				mdamagem(magr, mdef, &alt_attk);
				if (DEADMONSTER(mdef))
					return (MM_DEF_DIED | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
			}
		} else if (magr->data == &mons[PM_PURPLE_WORM] &&
			    mdef->data == &mons[PM_SHRIEKER]) {
		    /* hack to enhance mm_aggression(); we don't want purple
		       worm's bite attack to kill a shrieker because then it
		       won't swallow the corpse; but if the target survives,
		       the subsequent engulf attack should accomplish that */
		    if (tmp >= mdef->mhp) tmp = mdef->mhp - 1;
		} else {
			if(oarm && tmp && oarm->otyp == GAUNTLETS_OF_POWER) tmp += 8;
			if(resist_attacks(mdef->data))
				tmp = 0;
		}
		if(pa == &mons[PM_BABY_METROID]){
				magr->mhpmax += 1;
				magr->mhp += 1;
				if(magr->mtame){
					EDOG(magr)->hungrytime += pd->cnutrit/10;  //400/10 = human nut/10
				}
		}
		}break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_FIRE:
		if (cancelled) {
		    tmp = 0;
		    break;
		}
		if(canseemon(magr) && mattk->aatyp == AT_GAZE){
			Sprintf(buf,"%s gazes at", Monnam(magr));
			pline("%s %s...", buf, mon_nam(mdef));
		}
		if (vis)
		    pline("%s is %s!", Monnam(mdef),
			  on_fire(mdef->data, mattk));
		if (pd == &mons[PM_STRAW_GOLEM] ||
		    pd == &mons[PM_PAPER_GOLEM] ||
		    pd == &mons[PM_SPELL_GOLEM]) {
				if (vis) pline("%s burns completely!", Monnam(mdef));
				mondied(mdef);
				if (mdef->mhp > 0) return 0;
				else if (mdef->mtame && !vis)
				    pline("May %s roast in peace.", mon_nam(mdef));
				return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}else if (pd == &mons[PM_MIGO_WORKER]) {
				if (vis) pline("%s\'s brain melts!", Monnam(mdef));
				mondied(mdef);
				if (mdef->mhp > 0) return 0;
				else if (mdef->mtame && !vis)
				    pline("May %s roast in peace.", mon_nam(mdef));
				return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}
		tmp += destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
		tmp += destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
		if (resists_fire(mdef)) {
		    if (vis)
			pline_The("fire doesn't seem to burn %s!",
								mon_nam(mdef));
		    shieldeff(mdef->mx, mdef->my);
		    golemeffects(mdef, AD_FIRE, tmp);
		    tmp = 0;
		}
		/* only potions damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
		break;
/////////////////////////////////////////////////
		case AD_STDY:
			if(canseemon(magr) && mattk->aatyp == AT_GAZE){
				Sprintf(buf,"%s studies", Monnam(magr));
				pline("%s %s intently.", buf, mon_nam(mdef));
			}
			if (!magr->mcan && !is_blind(magr)) {
				mdef->mstdy = max(tmp,mdef->mstdy);
			}
			tmp = 0;
		break;
/////////////////////////////////////////////////
	    case AD_COLD:
	    case AD_ECLD:
		if (cancelled) {
		    tmp = 0;
		    break;
		}
		if(canseemon(magr) && mattk->aatyp == AT_GAZE){
			Sprintf(buf,"%s gazes at", Monnam(magr));
			pline("%s %s...", buf, mon_nam(mdef));
		}
		if (vis) pline("%s is covered in frost!", Monnam(mdef));
		if (resists_cold(mdef)) {
		    if (vis)
			pline_The("frost doesn't seem to chill %s!",
								mon_nam(mdef));
		    shieldeff(mdef->mx, mdef->my);
		    golemeffects(mdef, AD_COLD, tmp);
		    tmp = 0;
		}
		tmp += destroy_mitem(mdef, POTION_CLASS, AD_COLD);
		break;
/////////////////////////////////////////////////
	    case AD_ELEC:
		if (cancelled) {
		    tmp = 0;
		    break;
		}
		if(canseemon(magr) && mattk->aatyp == AT_GAZE){
			Sprintf(buf,"%s gazes at", Monnam(magr));
			pline("%s %s...", buf, mon_nam(mdef));
		}
		if (vis) pline("%s gets zapped!", Monnam(mdef));
		tmp += destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
		if (resists_elec(mdef)) {
		    if (vis) pline_The("zap doesn't shock %s!", mon_nam(mdef));
		    shieldeff(mdef->mx, mdef->my);
		    golemeffects(mdef, AD_ELEC, tmp);
		    tmp = 0;
		}
		/* only rings damage resistant players in destroy_item */
		tmp += destroy_mitem(mdef, RING_CLASS, AD_ELEC);
		break;
/////////////////////////////////////////////////
	    case AD_AXUS:{
			int multiple = 0;
			int tmp2 = 0;
			if (vis) pline("%s gets zapped!", Monnam(mdef));
			if(resists_elec(mdef)) {
				if (vis) pline_The("zap doesn't shock %s!", mon_nam(mdef));
				shieldeff(mdef->mx, mdef->my);
			    golemeffects(mdef, AD_ELEC, tmp);
			}
			else{
				multiple++;
				(void) destroy_mitem(mdef, RING_CLASS, AD_ELEC);
				(void) destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
				if(!resists_fire(mdef)){
					if (vis)
						pline("%s is %s!", Monnam(mdef),
						  on_fire(mdef->data, mattk));
					if (pd == &mons[PM_STRAW_GOLEM] ||
						pd == &mons[PM_PAPER_GOLEM] ||
						pd == &mons[PM_SPELL_GOLEM]) {
							(void) destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
							(void) destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
							if (vis) pline("%s burns completely!", Monnam(mdef));
							mondied(mdef);
							if (mdef->mhp > 0) return 0;
							else if (mdef->mtame && !vis)
								pline("May %s roast in peace.", mon_nam(mdef));
							return (MM_DEF_DIED | (grow_up(magr,mdef) ?
										0 : MM_AGR_DIED));
					} else if (pd == &mons[PM_MIGO_WORKER]) {
							if (vis) pline("%s\'s brain melts!", Monnam(mdef));
							mondied(mdef);
							if (mdef->mhp > 0) return 0;
							else if (mdef->mtame && !vis)
								pline("May %s roast in peace.", mon_nam(mdef));
							return (MM_DEF_DIED | (grow_up(magr,mdef) ?
										0 : MM_AGR_DIED));
					}
					(void) destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
					(void) destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
					(void) destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
				}
				mdef->mconf = 1;
				mdef->mstun = 1;
			}
			if (vis) pline("%s is covered in frost!", Monnam(mdef));
			if (resists_cold(mdef)) {
				if (vis)
				pline_The("frost doesn't seem to chill %s!",
									mon_nam(mdef));
				shieldeff(mdef->mx, mdef->my);
				golemeffects(mdef, AD_COLD, tmp);
			} else {
				multiple++;
				(void) destroy_mitem(mdef, POTION_CLASS, AD_COLD);
			}
			if (!resists_drli(mdef)) {
				tmp2 = d(2,6);
				if (vis)
					pline("%s suddenly seems weaker!", Monnam(mdef));
				mdef->mhpmax -= tmp2;
				if (mdef->m_lev == 0){
					tmp2 = mdef->mhp;
				}
				else mdef->m_lev--;
				/* Automatic kill if drained past level 0 */
			}
			tmp *= multiple;
			tmp += tmp2;
		} break;
/////////////////////////////////////////////////
	    case AD_ACID:
		if (magr->mcan) {
		    tmp = 0;
		    break;
		}
		if(canseemon(magr) && mattk->aatyp == AT_GAZE){
			Sprintf(buf,"%s gazes at", Monnam(magr));
			pline("%s %s...", buf, mon_nam(mdef));
		}
		if (resists_acid(mdef)) {
		    if (vis)
			pline("%s is covered in acid, but it seems harmless.",
			      Monnam(mdef));
		    tmp = 0;
		} else if (vis) {
		    pline("%s is covered in acid!", Monnam(mdef));
		    pline("It burns %s!", mon_nam(mdef));
		}
		if (!rn2(30)) erode_armor(mdef, TRUE);
		if (!rn2(6)) erode_obj(MON_WEP(mdef), TRUE, TRUE);
		break;
	    case AD_RUST:
		if (magr->mcan) break;
		if (pd == &mons[PM_IRON_GOLEM] || pd == &mons[PM_CHAIN_GOLEM]) {
			if (vis) pline("%s falls to pieces!", Monnam(mdef));
			mondied(mdef);
			if (mdef->mhp > 0) return 0;
			else if (mdef->mtame && !vis)
			    pline("May %s rust in peace.", mon_nam(mdef));
			return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}
		hurtmarmor(mdef, AD_RUST);
		mdef->mstrategy &= ~STRAT_WAITFORU;
		tmp = 0;
		break;
	    case AD_CORR:
		if (magr->mcan) break;
		hurtmarmor(mdef, AD_CORR);
		mdef->mstrategy &= ~STRAT_WAITFORU;
		tmp = 0;
		break;
	    case AD_DCAY:
		if (magr->mcan) break;
		if (pd == &mons[PM_WOOD_GOLEM] ||
			pd == &mons[PM_GROVE_GUARDIAN] ||
			pd == &mons[PM_LIVING_LECTERN] ||
		    pd == &mons[PM_LEATHER_GOLEM]) {
			if (vis) pline("%s falls to pieces!", Monnam(mdef));
			mondied(mdef);
			if (mdef->mhp > 0) return 0;
			else if (mdef->mtame && !vis)
			    pline("May %s rot in peace.", mon_nam(mdef));
			return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}
		hurtmarmor(mdef, AD_DCAY);
		mdef->mstrategy &= ~STRAT_WAITFORU;
		tmp = 0;
		break;
	    case AD_STON:
		if (magr->mcan) break;
 do_stone:
		/* may die from the acid if it eats a stone-curing corpse */
		if (munstone(mdef, FALSE)) goto post_stone;
		if (poly_when_stoned(pd)) {
			mon_to_stone(mdef);
			tmp = 0;
			break;
		}
		if (!resists_ston(mdef)) {
			if (vis) pline("%s turns to stone!", Monnam(mdef));
			monstone(mdef);
 post_stone:		if (mdef->mhp > 0) return 0;
			else if (mdef->mtame && !vis)
			    You(brief_feeling, "peculiarly sad");
			return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		}
		tmp = (mattk->adtyp == AD_STON ? 0 : 1);
		break;
	    case AD_ABDC:
	    case AD_TLPT:
		if (!cancelled && tmp < mdef->mhp && !tele_restrict(mdef)) {
		    char mdef_Monnam[BUFSZ];
		    /* save the name before monster teleports, otherwise
		       we'll get "it" in the suddenly disappears message */
		    if (vis) Strcpy(mdef_Monnam, Monnam(mdef));
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    (void) rloc(mdef, FALSE);
		    if (vis && !canspotmon(mdef)
#ifdef STEED
		    	&& mdef != u.usteed
#endif
		    	)
			pline("%s suddenly disappears!", mdef_Monnam);
		}
		break;
	    case AD_WEEP:
	    case AD_LVLT:
		if (!cancelled && tmp < mdef->mhp) {
		    char mdef_Monnam[BUFSZ];
		    if (vis) Strcpy(mdef_Monnam, Monnam(mdef));
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    if (u.uevent.udemigod) {
		    /* Once the player kills Rodney or performs the Invocation, weeping angels will 
		       be too interested in your potential to feed off the potential of monsters */
			if (vis && canspotmon(magr) && flags.verbose)
			    pline("%s is glancing at you with a hungry stare.", Monnam(magr));
		    } else {
#ifdef STEED
			if (u.usteed == mdef) {
			    pline("%s vanishes from underneath you.", Monnam(mdef));
			    dismount_steed(DISMOUNT_VANISHED);
			} else {
#endif
			    if (vis && canspotmon(mdef) && flags.verbose)
				pline("%s vanishes before your eyes.", Monnam(mdef));
#ifdef STEED
			}
#endif
			    int nlev;
			    d_level flev;
			    nlev = random_teleport_level();
			    get_level(&flev, nlev); 
			    migrate_to_level(mdef, ledger_no(&flev), MIGR_RANDOM,
				    (coord *)0);
		    }
		}
		break;
	    case AD_SLEE:
		if (!cancelled && !mdef->msleeping &&
			sleep_monst(mdef, rnd(10), -1)) {
			if(mattk->aatyp == AT_GAZE){
				if(canseemon(magr)){
					Sprintf(buf,"%s gazes at", Monnam(magr));
					pline("%s %s...", buf, mon_nam(mdef));
				}
				tmp = 0;
			}
			else if(mattk->aatyp == AT_WDGZ){
				tmp = 0;
			}
		    if (vis) {
			Strcpy(buf, Monnam(mdef));
			pline("%s is put to sleep by %s.", buf, mon_nam(magr));
		    }
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    slept_monst(mdef);
		}
		break;
	    case AD_PLYS:
		if (is_weeping(mdef->data)) {
		    tmp = 0;
		    break;
		}
		if(mattk->aatyp == AT_GAZE){
			if(canseemon(magr)){
				Sprintf(buf,"%s gazes at", Monnam(magr));
				pline("%s %s...", buf, mon_nam(mdef));
			}
			tmp = 0;
		}
		else if(mattk->aatyp == AT_WDGZ){
			tmp = 0;
		}
		if(!cancelled && mdef->mcanmove) {
		    if (vis) {
			Strcpy(buf, Monnam(mdef));
			pline("%s is frozen by %s.", buf, mon_nam(magr));
		    }
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		break;
	    case AD_TCKL:
		if (is_weeping(mdef->data)) {
		    tmp = 0;
		    break;
		}
		if(mdef->mcanmove) {
		    if (vis) {
			Strcpy(buf, Monnam(magr));
			pline("%s mercilessly tickles %s.", buf, mon_nam(mdef));
		    }
		    mdef->mcanmove = 0;
		    mdef->mfrozen = rnd(10);
		    mdef->mstrategy &= ~STRAT_WAITFORU;
  		}
		break;
	    case AD_SLOW:
		if (is_weeping(mdef->data)) {
		    tmp = 0;
		    break;
		}
		if(mattk->aatyp == AT_GAZE){
			if(canseemon(magr)){
				Sprintf(buf,"%s gazes at", Monnam(magr));
				pline("%s %s...", buf, mon_nam(mdef));
			}
			tmp = 0;
		}
		else if(mattk->aatyp == AT_WDGZ){
			tmp = 0;
		}
		if (!cancelled && mdef->mspeed != MSLOW) {
		    unsigned int oldspeed = mdef->mspeed;

		    mon_adjust_speed(mdef, -1, (struct obj *)0);
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    if (mdef->mspeed != oldspeed && vis)
			pline("%s slows down.", Monnam(mdef));
		}
		break;
	    case AD_LUCK: 
		/* Luck drain only makes sense for the player, so let's make 
		 * the monster confused instead */
	    case AD_CONF:
		/* Since confusing another monster doesn't have a real time
		 * limit, setting spec_used would not really be right (though
		 * we still should check for it).
		 */
		if(mattk->aatyp == AT_GAZE){
			if(canseemon(magr)){
				Sprintf(buf,"%s gazes at", Monnam(magr));
				pline("%s %s...", buf, mon_nam(mdef));
			}
			tmp = 0;
		}
		else if(mattk->aatyp == AT_WDGZ){
			tmp = 0;
		}
		if (!magr->mcan && !mdef->mconf && !magr->mspec_used) {
		    if (vis) pline("%s looks confused.", Monnam(mdef));
		    mdef->mconf = 1;
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		break;
	    case AD_BLND:
		if(mattk->aatyp == AT_GAZE){
			if(canseemon(magr)){
				Sprintf(buf,"%s gazes at", Monnam(magr));
				pline("%s %s...", buf, mon_nam(mdef));
			}
			tmp = 0;
		}
		else if(mattk->aatyp == AT_WDGZ){
			tmp = 0;
		}
		if (can_blnd(magr, mdef, mattk->aatyp, (struct obj*)0)) {
		    register unsigned rnd_tmp;

		    if (vis && !is_blind(mdef))
			pline("%s is blinded.", Monnam(mdef));
		    rnd_tmp = d((int)mattk->damn, (int)mattk->damd);
		    if ((rnd_tmp += mdef->mblinded) > 127) rnd_tmp = 127;
		    mdef->mblinded = rnd_tmp;
		    mdef->mcansee = 0;
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		tmp = 0;
		break;
	    case AD_BLNK:
		/* Weeping angels using their gaze attack on each
		   other has unfortunate effects for both of them */
		if (is_weeping(pd) && !is_blind(mdef) && !is_blind(magr)) {
		    if (vis) {
			Strcpy(buf, Monnam(mdef));
			pline("%s and %s are permanently quantum-locked!", buf, mon_nam(magr));
		    }
		    monstone(mdef);
		    monstone(magr);
		    return (MM_DEF_DIED | MM_AGR_DIED);
		}
		break;
	    case AD_HALU:
		if (!magr->mcan && haseyes(pd) && !is_blind(mdef)) {
			if(mattk->aatyp == AT_GAZE){
				if(canseemon(magr)){
					Sprintf(buf,"%s gazes at", Monnam(magr));
					pline("%s %s...", buf, mon_nam(mdef));
				}
				tmp = 0;
			}
		    if (vis) pline("%s looks %sconfused.",
				    Monnam(mdef), mdef->mconf ? "more " : "");
		    mdef->mconf = 1;
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		tmp = 0;
		break;
	    case AD_CURS:
		if (!night() && (pa == &mons[PM_GREMLIN])) break;
		if (!magr->mcan && !rn2(10)) {
		    mdef->mcan = 1;	/* cancelled regardless of lifesave */
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    if (is_were(pd) && pd->mlet != S_HUMAN)
			were_change(mdef);
		    if (pd == &mons[PM_CLAY_GOLEM] || pd == &mons[PM_SPELL_GOLEM]) {
			    if (vis) {
				pline("Some writing vanishes from %s head!",
				    s_suffix(mon_nam(mdef)));
				pline("%s is destroyed!", Monnam(mdef));
			    }
			    mondied(mdef);
			    if (mdef->mhp > 0) return 0;
			    else if (mdef->mtame && !vis)
				You(brief_feeling, "strangely sad");
			    return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
		    }
		    if (flags.soundok) {
			    if (!vis) You_hear("laughter.");
			    else pline("%s chuckles.", Monnam(magr));
		    }
		}
		break;
	    case AD_SGLD:
		tmp = 0;
#ifndef GOLDOBJ
		if (magr->mcan || !mdef->mgold) break;
		/* technically incorrect; no check for stealing gold from
		 * between mdef's feet...
		 */
		magr->mgold += mdef->mgold;
		mdef->mgold = 0;
#else
                if (magr->mcan) break;
		/* technically incorrect; no check for stealing gold from
		 * between mdef's feet...
		 */
                {
		    struct obj *gold = findgold(mdef->minvent);
		    if (!gold) break;
                    obj_extract_self(gold);
		    add_to_minv(magr, gold);
                }
#endif
		mdef->mstrategy &= ~STRAT_WAITFORU;
		if (vis) {
		    Strcpy(buf, Monnam(magr));
		    pline("%s steals some gold from %s.", buf, mon_nam(mdef));
		}
		if (!tele_restrict(magr)) {
		    (void) rloc(magr, FALSE);
		    if (vis && !canspotmon(magr))
			pline("%s suddenly disappears!", buf);
		}
		break;
	    case AD_VAMP:
	    case AD_DRLI:
		if (!cancelled && magr->mtame && !magr->isminion &&
			is_vampire(pa) && mattk->aatyp == AT_BITE &&
			has_blood_mon(mdef)
		) EDOG(magr)->hungrytime += ((int)((mdef->data)->cnutrit / 20) + 1);
		
		if(has_blood_mon(mdef) && magr->data == &mons[PM_BLOOD_BLOATER]){
			magr->mhp += tmp;
			if (magr->mhpmax < magr->mhp) magr->mhpmax = magr->mhp;
		}
		if (!cancelled && rn2(2) && !resists_drli(mdef)
			&& (magr->data != &mons[PM_VAMPIRE_BAT] || mdef->msleeping)
		) {
			tmp = d(2,6);
			if (vis)
			    pline("%s suddenly seems weaker!", Monnam(mdef));
			mdef->mhpmax -= tmp;
			if (mdef->m_lev == 0)
				tmp = mdef->mhp;
			else mdef->m_lev--;
			/* Automatic kill if drained past level 0 */
			if(pa == &mons[PM_METROID] || pa == &mons[PM_ALPHA_METROID] || pa == &mons[PM_GAMMA_METROID]
				|| pa == &mons[PM_ZETA_METROID] || pa == &mons[PM_OMEGA_METROID]
				|| pa == &mons[PM_METROID_QUEEN]){
					magr->mhpmax += d(1,4);
					magr->mhp += d(1,6);
					if(magr->mhp > magr->mhpmax) magr->mhp = magr->mhpmax;
					if(magr->mtame){
						EDOG(magr)->hungrytime += pd->cnutrit/4;  //400/4 = human nut/4
					}
			}
			if(magr->data == &mons[PM_BLOOD_BLOATER]){
				(void)split_mon(magr, 0);
			}
		}
		break;
#ifdef SEDUCE
	    case AD_SSEX:
#endif
	    case AD_SITM:	/* for now these are the same */
	    case AD_SEDU:
		if (magr->mcan || dist2(magr->mx,magr->my,mdef->mx,mdef->my) > 2) break;
		/* find an object to steal, non-cursed if magr is tame */
		for (obj = mdef->minvent; obj; obj = obj->nobj)
		    if (!magr->mtame || !obj->cursed)
			break;

		if (obj) {
			char onambuf[BUFSZ], mdefnambuf[BUFSZ];

			/* make a special x_monnam() call that never omits
			   the saddle, and save it for later messages */
			Strcpy(mdefnambuf, x_monnam(mdef, ARTICLE_THE, (char *)0, 0, FALSE));

			otmp = obj;
#ifdef STEED
			if (u.usteed == mdef &&
					otmp == which_armor(mdef, W_SADDLE))
				/* "You can no longer ride <steed>." */
				dismount_steed(DISMOUNT_POLY);
#endif
			obj_extract_self(otmp);
			if (otmp->owornmask) {
				mdef->misc_worn_check &= ~otmp->owornmask;
				if (otmp->owornmask & W_WEP)
				    setmnotwielded(mdef,otmp);
				otmp->owornmask = 0L;
				update_mon_intrinsics(mdef, otmp, FALSE, FALSE);
			}
			/* add_to_minv() might free otmp [if it merges] */
			if (vis)
				Strcpy(onambuf, doname(otmp));
			(void) add_to_minv(magr, otmp);
			if (vis) {
				Strcpy(buf, Monnam(magr));
				pline("%s steals %s from %s!", buf,
				    onambuf, mdefnambuf);
			}
			possibly_unwield(mdef, FALSE);
			mdef->mstrategy &= ~STRAT_WAITFORU;
			mselftouch(mdef, (const char *)0, FALSE);
			if (mdef->mhp <= 0)
				return (MM_DEF_DIED | (grow_up(magr,mdef) ?
							0 : MM_AGR_DIED));
			if (magr->data->mlet == S_NYMPH &&
			    !tele_restrict(magr)) {
			    (void) rloc(magr, FALSE);
			    if (vis && !canspotmon(magr))
				pline("%s suddenly disappears!", buf);
			}
			m_dowear(magr, FALSE);
		}
		tmp = 0;
		break;
	    case AD_STAR:
			if(hates_silver(pd)) {
            	tmp += rnd(20);
            	if (vis) pline("The rapier of silver light sears %s!", mon_nam(mdef));
            }
		break;
		case AD_GLSS:
			if(hates_silver(pd)) {
            	tmp += rnd(20);
            	if (vis) pline("%s's fractured claws sear %s!", Monnam(magr), mon_nam(mdef));
            }
		break;
	    case AD_BLUD:
			if(has_blood_mon(mdef) || (has_blood(pd) && mdef->mfaction == ZOMBIFIED)) {
            	tmp += mdef->m_lev;
            	if (vis) pline("The blade of rotted blood tears through the veins of %s!", mon_nam(mdef));
            }
		break;
	    case AD_SHDW:
			// tmp = d(rnd(8),rnd(5)+1);
	    case AD_DRST:
	    case AD_DRDX:
	    case AD_DRCO:
		if (!cancelled && !rn2(8)) {
		    if (vis)
			pline("%s %s was poisoned!", s_suffix(Monnam(magr)),
			      mpoisons_subj(magr, mattk));
		    if (resists_poison(mdef)) {
			if (vis)
			    pline_The("poison doesn't seem to affect %s.",
				mon_nam(mdef));
		    } else {
			if (rn2(10)) tmp += rn1(10,6);
			else {
			    if (vis) pline_The("poison was deadly...");
			    tmp = mdef->mhp;
			}
		    }
		}
		break;
		case AD_NPDC:
			if(!cancelled) tmp += rnd(10);
		break;
	    case AD_DRIN:
		if (notonhead || !has_head(pd)) {
		    if (vis) pline("%s doesn't seem harmed.", Monnam(mdef));
		    /* Not clear what to do for green slimes */
		    tmp = 0;
		    break;
		}
		if ((mdef->misc_worn_check & W_ARMH) && which_armor(mdef, W_ARMH) && /*Armor going missing? slight performance hit worth not crashing*/
			(rn2(8) || ((which_armor(mdef, W_ARMH))->otyp) == PLASTEEL_HELM || ((which_armor(mdef, W_ARMH))->otyp) == CRYSTAL_HELM || ((which_armor(mdef, W_ARMH))->otyp) == PONTIFF_S_CROWN )
		) {
		    if (vis) {
			Strcpy(buf, s_suffix(Monnam(mdef)));
			pline("%s helmet blocks %s attack to %s head.",
				buf, s_suffix(mon_nam(magr)),
				mhis(mdef));
		    }
		    break;
		}
		if (vis) pline("%s brain is eaten!", s_suffix(Monnam(mdef)));
		if (mindless_mon(mdef)) {
		    if (vis) pline("%s doesn't notice.", Monnam(mdef));
		    break;
		}
		tmp += d(tmp,10); /* fakery, since monsters lack INT scores. NOTE: 1d10 PER POINT OF INT */
		if (magr->mtame && !magr->isminion) {
		    EDOG(magr)->hungrytime += tmp*11; /*aprox 60 points of hunger per 1d10*/
		    magr->mconf = 0;
		}
		if (tmp >= mdef->mhp && vis)
		    pline("%s last thought fades away...",
			          s_suffix(Monnam(mdef)));
		break;
	    case AD_DETH:
	        if (vis)
		    pline("%s reaches out with its deadly touch.",
		          Monnam(magr));
		if (is_undead_mon(mdef)) {
		    /* Still does normal damage */
	            if (vis)
		        pline("%s looks no deader than before.", Monnam(mdef));
		    break;
		}
		switch (rn2(20)) {
		case 19: case 18: case 17:
		    if (!resists_magm(mdef) && !resist(mdef, 0, 0, 0)) {
			mdef->mhp = 0;
		        monkilled(mdef, "", AD_DETH);
			tmp = 0;
			magr->mhp = magr->mhpmax;
			break;
		    } /* else FALLTHRU */
		default: /* case 16: ... case 5: */
		    if (vis)
		        pline("%s looks weaker!", Monnam(mdef));
		    mdef->mhpmax -= rn2(tmp / 2 + 1); /* mhp will then  */
		                                      /* still be less than  */
											  /* this value */
			magr->mhp = min(magr->mhp+tmp, magr->mhpmax);
		    break;
		case 4: case 3: case 2: case 1: case 0:
		    if (resists_magm(mdef)) shieldeff(mdef->mx, mdef->my);
	            if (vis)
		        pline("That didn't work...");
		    tmp = 0;
		    break;
		}
		break;
	    case AD_PEST:
			Strcpy(buf, mon_nam(mdef));
			if (vis)
				pline("%s reaches out, and %s looks rather ill.",
					Monnam(magr), buf);
			if((mdef->mhpmax > 3) && !resist(mdef, 0, 0, NOTELL)){
				magr->mhp = min(magr->mhp+mdef->mhpmax/2, magr->mhpmax);
				mdef->mhpmax /= 2;
			}
			if((mdef->mhp > 2) && !resist(mdef, 0, 0, NOTELL)){
				magr->mhp = min(magr->mhp+mdef->mhp/2, magr->mhpmax);
				mdef->mhp /= 2;
			}
			if (mdef->mhp > mdef->mhpmax){
				magr->mhp = min(magr->mhp+(mdef->mhp - mdef->mhpmax), magr->mhpmax);
				mdef->mhp = mdef->mhpmax;
			}
		break;
	    case AD_FAMN:
		Strcpy(buf, s_suffix(mon_nam(mdef)));
	        if (vis)
		    pline("%s reaches out, and %s body shrivels.",
			    Monnam(magr), buf);
		if (mdef->mtame && !mdef->isminion){
			int hngr = rn1(120, 120);
		    EDOG(mdef)->hungrytime -= hngr;
			magr->mhp = min(magr->mhp+magr->m_lev*hngr/30, magr->mhpmax); //ie, normal healing rate for that amount of nutrition
		} else
		{
		    tmp += rnd(10); /* lacks a food rating */
		    if (tmp >= mdef->mhp && vis){
		        pline("%s starves.", Monnam(mdef));
				magr->mhp = magr->mhpmax;
			} else magr->mhp = min(magr->mhp+tmp, magr->mhpmax);
		}
		/* plus the normal damage */
		break;
	    case AD_SLIM:
		if (cancelled) break;	/* physical damage only */
		{
		struct obj *armor = which_armor(mdef, W_ARM);
		struct obj *shield = which_armor(mdef, W_ARMS);
		if (!rn2(4) && !flaming(mdef->data)
				&& mdef->data != &mons[PM_RED_DRAGON]
				&& !(
					(armor && (armor->otyp == RED_DRAGON_SCALES || armor->otyp == RED_DRAGON_SCALE_MAIL)
					) || (shield && shield->otyp == RED_DRAGON_SCALE_SHIELD
					)
				) &&
				mdef->data != &mons[PM_GREEN_SLIME] && mdef->data != &mons[PM_FLUX_SLIME] && 
				!is_rider(mdef->data) && !resists_poly(magr->data)
		) {
		    (void) newcham(mdef, &mons[PM_GREEN_SLIME], FALSE, vis);
		    mdef->mstrategy &= ~STRAT_WAITFORU;
		    tmp = 0;
		}
		}
		break;
	    case AD_STCK:
		if (cancelled) tmp = 0;
		break;
	    case AD_WRAP: /* monsters cannot grab one another, it's too hard */
		if (magr->mcan) tmp = 0;
		break;
	    case AD_WEBS:{
			struct trap *ttmp2 = maketrap(mdef->mx, mdef->my, WEB);
			if (ttmp2) mintrap(mdef);
		}break;
	    case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
	    if (!cancelled){
		    struct obj *obj = some_armor(mdef);

		    if (drain_item(obj) && vis) {
				pline("%s's %s less effective.", Monnam(mdef), aobjnam(obj, "seem"));
		    }
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
		case AD_WET:
			water_damage(mdef->minvent, FALSE, FALSE, FALSE, mdef);
		break;
		case AD_FRWK:{
			int x, y, i;
			for (i = rn2(3)+2; i > 0; i--) {
				x = rn2(3)-1;
				y = rn2(3)-1;
				explode(magr->mx+x, magr->my+y, 8, tmp, -1, rn2(EXPL_MAX));		//-1 is unspecified source. 8 is physical
			}
			if(DEADMONSTER(magr))
				return MM_AGR_DIED;
			tmp=0;
		} break;
/*		case AD_VMSL:	//vorlon missile.  triple damage
			magr->mhp = -1;
			mondead(magr);
			explode(magr->mx, magr->my, 5, tmp, -1, EXPL_WET);		//-1 is unspecified source. 5 is electrical
			explode(magr->mx, magr->my, 4, tmp, -1, EXPL_MAGICAL);	//-1 is unspecified source. 4 is disintegration
			explode(magr->mx, magr->my, 8, tmp, -1, EXPL_DARK);		//-1 is unspecified source. 8 is physical
			tmp=0;		//damage was done by explode

//			if (magr->mhp > 0) return(0);
//			return(2);	// it dies
		break;
*/
	    default:	tmp = 0;
			break;
	}
	if(magr->mfaction == ZOMBIFIED){
		tmp *= 2;
	}
	
	if ( is_backstabber(magr->data) &&
		!(noncorporeal(mdef->data) || amorphous(mdef->data) || 
			((stationary(mdef->data) || sessile(mdef->data)) && (mdef->data->mlet == S_FUNGUS || mdef->data->mlet == S_PLANT))
		 ) && (
			(mdef->mflee && mdef->data != &mons[PM_BANDERSNATCH]) || is_blind(mdef) || !mdef->mcanmove || !mdef->mnotlaugh || 
				mdef->mstun || mdef->mconf || mdef->mtrapped || mdef->msleeping
		 )
	){
		if(magr->data == &mons[PM_CUPRILACH_RILMANI])
			tmp += rnd((int)(magr->m_lev*1.5));
		else tmp += rnd((int)(magr->m_lev));
	}
	
	
	if(magr->data == &mons[PM_LONG_WORM] && magr->wormno && mattk->aatyp == AT_BITE){
		if(wormline(magr, mdef->mx, mdef->my))
			tmp *= 2;
	}
	
   if(mdef->data == &mons[PM_GIANT_TURTLE] && mdef->mflee) tmp=tmp/2; 
	if(!tmp) return(MM_MISS);
	
	if(mdef->mstdy){
		tmp += mdef->mstdy;
		if(mdef->mstdy > 0) mdef->mstdy -= 1;
		else mdef->mstdy += 1;
	}
	
	if(tmp && magr->mtame && !mdef->mtame){
		tmp += beastmastery();
		if(u.specialSealsActive&SEAL_COSMOS) tmp += spiritDsize();
		if(u.usteed && magr == u.usteed) tmp += mountedCombat();
	}
	
	if(attacktype_fordmg(mdef->data, AT_NONE, AD_STAR)){
		if(otmp && otmp == MON_WEP(magr) && !otmp->oartifact && otmp->spe <= 0) tmp = 0;
		else if(!otmp || otmp != MON_WEP(magr)) tmp /= 2;
	}
	
	if(tmp && mattk->adtyp != AD_SHDW && mattk->adtyp != AD_STAR && !phasearmor){
		int mac = full_marmorac(mdef);
		if(mac < 0){
			tmp += MONSTER_AC_VALUE(mac);
			if(tmp < 1) tmp = 1;
		}
	}
	
	if((magr->mfaction == ZOMBIFIED || (magr->mfaction == SKELIFIED && !rn2(20))) && can_undead_mon(mdef)){
		mdef->zombify = 1;
	}
	
	if((magr->data == &mons[PM_UNDEAD_KNIGHT]
		|| magr->data == &mons[PM_WARRIOR_OF_SUNLIGHT]
		|| magr->data == &mons[PM_DREAD_SERAPH]
	) && can_undead_mon(mdef)){
		mdef->zombify = 1;
	}
	
	if(magr->mfaction == FRACTURED && is_kamerel(mdef->data)){
		mdef->zombify = 1;
	}
	
	if((mdef->mhp -= tmp) < 1) {
	    if (m_at(mdef->mx, mdef->my) == magr) {  /* see gulpmm() */
			remove_monster(mdef->mx, mdef->my);
			mdef->mhp = 1;	/* otherwise place_monster will complain */
			place_monster(mdef, mdef->mx, mdef->my);
			mdef->mhp = 0;
	    }
	    monkilled(mdef, "", (int)mattk->adtyp);
	    if (mdef->mhp > 0) return 0; /* mdef lifesaved */

	    if (mattk->adtyp == AD_DGST) {
			/* various checks similar to dog_eat and meatobj.
			 * after monkilled() to provide better message ordering */
			if (mdef->cham != CHAM_ORDINARY && !resists_poly(magr->data)) {
				(void) newcham(magr, (struct permonst *)0, FALSE, TRUE);
			} else if ((mdef->data == &mons[PM_GREEN_SLIME] || mdef->data == &mons[PM_FLUX_SLIME]) && !resists_poly(magr->data)) {
				(void) newcham(magr, &mons[PM_GREEN_SLIME], FALSE, TRUE);
			} else if (mdef->data == &mons[PM_WRAITH]) {
				(void) grow_up(magr, (struct monst *)0);
				/* don't grow up twice */
				return (MM_DEF_DIED | (magr->mhp > 0 ? 0 : MM_AGR_DIED));
			} else if (mdef->data == &mons[PM_NURSE]) {
				magr->mhp = magr->mhpmax;
			}
	    }

	    return (MM_DEF_DIED | (grow_up(magr,mdef) ? 0 : MM_AGR_DIED));
	}
	return(MM_HIT);
}

#endif /* OVLB */


#ifdef OVL0

int
noattacks(ptr)			/* returns 1 if monster doesn't attack */
	struct	permonst *ptr;
{
	int i;

	for(i = 0; i < NATTK; i++)
		if(ptr->mattk[i].aatyp) return(0);

	return(1);
}

/* `mon' is hit by a sleep attack; return 1 if it's affected, 0 otherwise */
int
sleep_monst(mon, amt, how)
struct monst *mon;
int amt, how;
{
	if (resists_sleep(mon) ||
		(how >= 0 && resist(mon, (char)how, 0, NOTELL))) {
	    shieldeff(mon->mx, mon->my);
	} else if (mon->mcanmove) {
	    amt += (int) mon->mfrozen;
	    if (amt > 0) {	/* sleep for N turns */
		mon->mcanmove = 0;
		mon->mfrozen = min(amt, 127);
	    } else {		/* sleep until awakened */
		mon->msleeping = 1;
	    }
	    return 1;
	}
	return 0;
}

/* sleeping grabber releases, engulfer doesn't; don't use for paralysis! */
void
slept_monst(mon)
struct monst *mon;
{
	if ((mon->msleeping || !mon->mcanmove) && mon == u.ustuck &&
		!sticks(youracedata) && !u.uswallow) {
	    pline("%s grip relaxes.", s_suffix(Monnam(mon)));
	    unstuck(mon);
	}
}

#endif /* OVL0 */
#ifdef OVLB

STATIC_OVL void
mrustm(magr, mdef, obj)
register struct monst *magr, *mdef;
register struct obj *obj;
{
	boolean is_acid;

	if (!magr || !mdef || !obj) return; /* just in case */

	if (dmgtype(mdef->data, AD_CORR))
	    is_acid = TRUE;
	else if (dmgtype(mdef->data, AD_RUST))
	    is_acid = FALSE;
	else
	    return;

	if (!mdef->mcan &&
	    (is_acid ? is_corrodeable(obj) : is_rustprone(obj)) &&
	    (is_acid ? obj->oeroded2 : obj->oeroded) < MAX_ERODE) {
		if (obj->greased || obj->oerodeproof || (obj->blessed && rn2(3))) {
		    if (cansee(mdef->mx, mdef->my) && flags.verbose)
			pline("%s weapon is not affected.",
			                 s_suffix(Monnam(magr)));
		    if (obj->greased && !rn2(2)) obj->greased = 0;
		} else {
		    if (cansee(mdef->mx, mdef->my)) {
			pline("%s %s%s!", s_suffix(Monnam(magr)),
			    aobjnam(obj, (is_acid ? "corrode" : "rust")),
			    (is_acid ? obj->oeroded2 : obj->oeroded)
				? " further" : "");
		    }
		    if (is_acid) obj->oeroded2++;
		    else obj->oeroded++;
		}
	}
}

STATIC_OVL void
mswingsm(magr, mdef, otemp)
register struct monst *magr, *mdef;
register struct obj *otemp;
{
	char buf[BUFSZ];
	if (!flags.verbose || Blind || !mon_visible(magr)) return;
	Strcpy(buf, mon_nam(mdef));
	pline("%s %s %s %s at %s.", Monnam(magr),
	      (objects[otemp->otyp].oc_dir & PIERCE && weapon_type(otemp) != P_PICK_AXE) ? "thrusts" : "swings",
	      mhis(magr), singular(otemp, xname), buf);
}

/*
 * Passive responses by defenders.  Does not replicate responses already
 * handled above.  Returns same values as mattackm.
 */
STATIC_OVL int
passivemm(magr,mdef,mhit,mdead,mattk)
register struct monst *magr, *mdef;
boolean mhit;
int mdead;
struct attack *mattk;
{
	register struct permonst *mddat = mdef->data;
	register struct permonst *madat = magr->data;
	char buf[BUFSZ];
	int i, tmp;

	for(i = 0; ; i++) {
	    if(i >= NATTK) return (mdead | mhit); /* no passive attacks */
	    if(mddat->mattk[i].aatyp == AT_NONE) break;
	}
	if (mddat->mattk[i].damn)
	    tmp = d((int)mddat->mattk[i].damn,
				    (int)mddat->mattk[i].damd);
	else if(mddat->mattk[i].damd)
	    tmp = d((int)mddat->mlevel+1, (int)mddat->mattk[i].damd);
	else
	    tmp = 0;

	if (mdef->data == &mons[PM_GRUE] && !((!levl[mdef->mx][mdef->my].lit && !(viz_array[mdef->my][mdef->mx] & TEMP_LIT1 && !(viz_array[mdef->my][mdef->mx] & TEMP_DRK1)))
		|| (levl[mdef->mx][mdef->my].lit && (viz_array[mdef->my][mdef->mx] & TEMP_DRK1 && !(viz_array[mdef->my][mdef->mx] & TEMP_LIT1)))))
		return (mdead | mhit);
	
	/* These affect the enemy even if defender killed */
	switch(mddat->mattk[i].adtyp) {
		case AD_BARB:
			Strcpy(buf, Monnam(magr));
			if(canseemon(magr)){
				if(mdef->data == &mons[PM_RAZORVINE]) 
					pline("%s is hit by the springing vines!", buf);
				else pline("%s is hit by %s barbs!",
					  buf, s_suffix(mon_nam(mdef)));
			}
			goto assess_dmg;
		break;
	    case AD_ACID:
		if (mhit && !rn2(2)) {
		    Strcpy(buf, Monnam(magr));
		    if(canseemon(magr))
			pline("%s is splashed by %s acid!",
			      buf, s_suffix(mon_nam(mdef)));
		    if (resists_acid(magr)) {
			if(canseemon(magr))
			    pline("%s is not affected.", Monnam(magr));
			tmp = 0;
		    }
		} else tmp = 0;
		goto assess_dmg;
		case AD_MAGM:
	    /* wrath of gods for attacking Oracle */
	    if(resists_magm(magr)) {
		if(canseemon(magr)) {
		shieldeff(magr->mx, magr->my);
		pline("A hail of magic missiles narrowly misses %s!",mon_nam(magr));
		}
	    } else {
		if (canseemon(magr)) {
		    if (magr->data == &mons[PM_WOODCHUCK])
			pline("ZOT!");
		    else
			pline("%s is hit by magic missiles appearing from thin air!", Monnam(magr));
		}
		goto assess_dmg;
	    }
	    break;
	    case AD_DRST:
	    case AD_DRDX:
	    case AD_DRCO:
		if (!rn2(8)) {
		    if (vis)
			pline("%s shadow was poisoned!", s_suffix(Monnam(mdef)));
		    if (resists_poison(magr)) {
			if (vis)
			    pline_The("poison doesn't seem to affect %s.",
				mon_nam(mdef));
		    } else {
			if (rn2(10)) tmp += rn1(10,6);
			else {
			    if (vis) pline_The("poison was deadly...");
			    tmp = mdef->mhp;
			}
		    }
		}
		break;
	    case AD_WEBS:{	/* KMH -- remove enchantment (disenchanter) */
			struct trap *ttmp2 = maketrap(magr->mx, magr->my, WEB);
			if (ttmp2) mintrap(magr);
		}break;
	    case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
		if (mhit && !mdef->mcan && otmp) {
		    (void) drain_item(otmp);
		    /* No message */
		}
		break;
	    case AD_OONA:{
			struct monst *omon;
//			pline("hp: %d x: %d y: %d", (mon->mhp*100)/mon->mhpmax, mon->mx, mon->my);
			if(!mhit) break; //didn't draw blood, forget it.
			
			if(u.oonaenergy == AD_FIRE){
				if(rn2(2)) omon = makemon(&mons[PM_FLAMING_SPHERE], mdef->mx, mdef->my, mdef->mtame ? MM_ADJACENTOK|MM_EDOG : MM_ADJACENTOK);
				else omon = makemon(&mons[PM_FIRE_VORTEX], mdef->mx, mdef->my, mdef->mtame ? MM_ADJACENTOK|MM_EDOG : MM_ADJACENTOK);
			} else if(u.oonaenergy == AD_COLD){
				if(rn2(2)) omon = makemon(&mons[PM_FREEZING_SPHERE], mdef->mx, mdef->my, mdef->mtame ? MM_ADJACENTOK|MM_EDOG : MM_ADJACENTOK);
				else omon = makemon(&mons[PM_ICE_VORTEX], mdef->mx, mdef->my, mdef->mtame ? MM_ADJACENTOK|MM_EDOG : MM_ADJACENTOK);
			} else if(u.oonaenergy == AD_ELEC){
				if(rn2(2)) omon = makemon(&mons[PM_SHOCKING_SPHERE], mdef->mx, mdef->my, mdef->mtame ? MM_ADJACENTOK|MM_EDOG : MM_ADJACENTOK);
				else omon = makemon(&mons[PM_ENERGY_VORTEX], mdef->mx, mdef->my, mdef->mtame ? MM_ADJACENTOK|MM_EDOG : MM_ADJACENTOK);
			}
			if(mdef->mtame) initedog(omon);
		} break;
	    case AD_STAR:
			if((otmp && otmp == MON_WEP(magr)) || !hates_silver(magr->data)) {
            	tmp = 0;
            } else if (vis) pline("The dress of silver stars sears %s!", mon_nam(magr));
			if(otmp){
				if(!otmp->oartifact){
					if(otmp->where == OBJ_MINVENT && otmp == MON_WEP(otmp->ocarry)){
						if(otmp->spe > 0) (void) drain_item(otmp);
						else{
							setmnotwielded(otmp->ocarry,otmp);
							MON_NOWEP(otmp->ocarry);
							m_useup(otmp->ocarry, otmp);
						}
					}
				}
			}
		break;
	    case AD_HLBD:
//		pline("hp: %d x: %d y: %d", (mon->mhp*100)/mon->mhpmax, mon->mx, mon->my);
	    if(!mhit) break; //didn't draw blood, forget it.
		
		if(mdef->data == &mons[PM_LEGION]){
			int n = rnd(4);
			for(n; n>0; n--) rn2(7) ? makemon(mkclass(S_ZOMBIE, G_NOHELL|G_HELL), mdef->mx, mdef->my, NO_MINVENT|MM_ADJACENTOK|MM_ADJACENTSTRICT): 
									  makemon(&mons[PM_LEGIONNAIRE], mdef->mx, mdef->my, NO_MINVENT|MM_ADJACENTOK|MM_ADJACENTSTRICT);
		} else {
			if(mdef->mhp > .75*mdef->mhpmax) makemon(&mons[PM_LEMURE], mdef->mx, mdef->my, MM_ADJACENTOK);
			else if(mdef->mhp > .50*mdef->mhpmax) makemon(&mons[PM_HORNED_DEVIL], mdef->mx, mdef->my, MM_ADJACENTOK);
			else if(mdef->mhp > .25*mdef->mhpmax) makemon(&mons[PM_BARBED_DEVIL], mdef->mx, mdef->my, MM_ADJACENTOK);
			else if(mdef->mhp > .00*mdef->mhpmax) makemon(&mons[PM_PIT_FIEND], mdef->mx, mdef->my, MM_ADJACENTOK);
		}
		break;
	    default:
		break;
	}
	if (mdead || mdef->mcan) return (mdead|mhit);

	/* These affect the enemy only if defender is still alive */
	if (rn2(3)) switch(mddat->mattk[i].adtyp) {
	    case AD_PLYS: /* Floating eye */
		if (tmp > 127) tmp = 127;
		if (mddat == &mons[PM_FLOATING_EYE]) {
		    if (!rn2(4)) tmp = 127;
		    if (!is_blind(magr) && haseyes(madat) && !is_blind(mdef) &&
			(perceives(madat) || !mdef->minvis)) {
			Sprintf(buf, "%s gaze is reflected by %%s %%s.",
				s_suffix(mon_nam(mdef)));
			if (mon_reflects(magr,
					 canseemon(magr) ? buf : (char *)0))
				return(mdead|mhit);
			Strcpy(buf, Monnam(magr));
			if(canseemon(magr))
			    pline("%s is frozen by %s gaze!",
				  buf, s_suffix(mon_nam(mdef)));
			magr->mcanmove = 0;
			magr->mfrozen = tmp;
			return (mdead|mhit);
		    }
		} else { /* gelatinous cube */
		    Strcpy(buf, Monnam(magr));
		    if(canseemon(magr))
			pline("%s is frozen by %s.", buf, mon_nam(mdef));
		    magr->mcanmove = 0;
		    magr->mfrozen = tmp;
		    return (mdead|mhit);
		}
		return 1;
	    case AD_COLD:
		if (resists_cold(magr)) {
		    if (canseemon(magr)) {
			pline("%s is mildly chilly.", Monnam(magr));
			golemeffects(magr, AD_COLD, tmp);
		    }
		    tmp = 0;
		    break;
		}
		if(canseemon(magr))
		    pline("%s is suddenly very cold!", Monnam(magr));
		mdef->mhp += tmp / 2;
		if (mdef->mhpmax < mdef->mhp) mdef->mhpmax = mdef->mhp;
		if (mdef->mhpmax > ((int) (mdef->m_lev+1) * 8) && (mdef->data == &mons[PM_BROWN_MOLD] || mdef->data == &mons[PM_BLUE_JELLY]))
		    (void)split_mon(mdef, magr);
		break;
	    case AD_STUN:
		if (!magr->mstun) {
		    magr->mstun = 1;
		    if (canseemon(magr))
			pline("%s %s...", Monnam(magr),
			      makeplural(stagger(magr->data, "stagger")));
		}
		tmp = 0;
		break;
	    case AD_FIRE:
	    case AD_EFIR:
		if (resists_fire(magr)) {
		    if (canseemon(magr)) {
			pline("%s is mildly warmed.", Monnam(magr));
			golemeffects(magr, AD_FIRE, tmp);
		    }
		    tmp = 0;
		    break;
		}
		if(canseemon(magr))
		    pline("%s is suddenly very hot!", Monnam(magr));
		break;
	    case AD_ELEC:
	    case AD_EELC:
		if (resists_elec(magr)) {
		    if (canseemon(magr)) {
			pline("%s is mildly tingled.", Monnam(magr));
			golemeffects(magr, AD_ELEC, tmp);
		    }
		    tmp = 0;
		    break;
		}
		if(canseemon(magr))
		    pline("%s is jolted with electricity!", Monnam(magr));
		break;
	    default: tmp = 0;
		break;
	}
	else tmp = 0;

    assess_dmg:
	if((magr->mhp -= tmp) <= 0) {
		monkilled(magr, "", (int)mddat->mattk[i].adtyp);
		return (mdead | mhit | MM_AGR_DIED);
	}
	return (mdead | mhit);
}

/* "aggressive defense"; what type of armor prevents specified attack
   from touching its target? */
long
attk_protection(aatyp)
int aatyp;
{
    long w_mask = 0L;

    switch (aatyp) {
    case AT_NONE:
    case AT_SPIT:
    case AT_EXPL:
    case AT_BOOM:
    case AT_GAZE:
    case AT_WDGZ:
    case AT_BREA:
    case AT_MAGC:
    case AT_MMGC:
    case AT_BEAM:
    case AT_SHDW:
    case AT_WISP:
	w_mask = ~0L;		/* special case; no defense needed */
	break;
    case AT_CLAW:
    case AT_LRCH:
    case AT_TUCH:
    case AT_5SQR:
    case AT_WEAP:
	case AT_XWEP:
	case AT_MARI:
    case AT_DEVA:
	w_mask = W_ARMG;	/* caller needs to check for weapon */
	break;
    case AT_KICK:
	w_mask = W_ARMF;
	break;
    case AT_BUTT:
	w_mask = W_ARMH;
	break;
    case AT_HUGS:
	w_mask = (W_ARMC|W_ARMG); /* attacker needs both to be protected */
	break;
    case AT_BITE:
    case AT_LNCK:
    case AT_STNG:
    case AT_ENGL:
    case AT_TENT:
    default:
	w_mask = 0L;		/* no defense available */
	break;
    }
    return w_mask;
}

STATIC_OVL int
beastmastery()
{
	int bm;
	switch (P_SKILL(P_BEAST_MASTERY)) {
		case P_ISRESTRICTED: bm =  0; break;
		case P_UNSKILLED:    bm =  0; break;
		case P_BASIC:        bm =  2; break;
		case P_SKILLED:      bm =  5; break;
		case P_EXPERT:       bm = 10; break;
	}
	if((uwep && uwep->oartifact == ART_CLARENT) || (uswapwep && uswapwep->oartifact == ART_CLARENT))
		bm *= 2;
	return bm;
}

STATIC_OVL int
mountedCombat()
{
	int bm;
	switch (P_SKILL(P_RIDING)) {
		case P_ISRESTRICTED: bm =  0; break;
		case P_UNSKILLED:    bm =  0; break;
		case P_BASIC:        bm =  2; break;
		case P_SKILLED:      bm =  5; break;
		case P_EXPERT:       bm = 10; break;
	}
	return bm;
}


#endif /* OVLB */

/*mhitm.c*/

