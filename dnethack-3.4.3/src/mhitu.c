/*	SCCS Id: @(#)mhitu.c	3.4	2003/11/26	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#include "edog.h"

STATIC_VAR NEARDATA struct obj *otmp;

STATIC_DCL void FDECL(urustm, (struct monst *, struct obj *));
# ifdef OVL1
STATIC_DCL boolean FDECL(u_slip_free, (struct monst *,struct attack *));
STATIC_DCL int FDECL(passiveum, (struct permonst *,struct monst *,struct attack *));
# endif /* OVL1 */

#ifdef OVLB
# ifdef SEDUCE
STATIC_DCL void FDECL(mayberem, (struct obj *, const char *));
STATIC_DCL void FDECL(lrdmayberem, (struct obj *, const char *));
STATIC_DCL void FDECL(mlcmayberem, (struct obj *, const char *, boolean));
STATIC_DCL void FDECL(sflmayberem, (struct obj *, const char *, boolean));
STATIC_DCL void FDECL(palemayberem, (struct obj *, const char *, boolean));
# endif
#endif /* OVLB */

STATIC_DCL int FDECL(hitmu, (struct monst *,struct attack *));
STATIC_DCL int FDECL(gulpmu, (struct monst *,struct attack *));
STATIC_DCL int FDECL(explmu, (struct monst *,struct attack *,BOOLEAN_P));
STATIC_DCL void FDECL(missmu,(struct monst *,BOOLEAN_P,struct attack *));
STATIC_DCL void FDECL(mswings,(struct monst *,struct obj *));
STATIC_DCL void FDECL(wildmiss, (struct monst *,struct attack *));

STATIC_DCL void FDECL(hurtarmor,(int));
STATIC_DCL void FDECL(hitmsg,(struct monst *,struct attack *));

static const int gazeattacks[] = {AD_DEAD, AD_CNCL, AD_PLYS, AD_DRLI, AD_ENCH, AD_STON, AD_LUCK,
										AD_CONF, AD_SLOW, AD_STUN, AD_BLND, AD_FIRE, AD_FIRE,
										AD_COLD, AD_COLD, AD_ELEC, AD_ELEC, AD_HALU, AD_SLEE };
static const int elementalgaze[] = {AD_FIRE,AD_COLD,AD_ELEC};

/* See comment in mhitm.c.  If we use this a lot it probably should be */
/* changed to a parameter to mhitu. */
static int dieroll;
static const char tools[] = { TOOL_CLASS, 0 };

#ifdef OVL1


STATIC_OVL void
hitmsg(mtmp, mattk)
register struct monst *mtmp;
register struct attack *mattk;
{
	int compat;

	/* Note: if opposite gender, "seductively" */
	/* If same gender, "engagingly" for nymph, normal msg for others */
	if((compat = could_seduce(mtmp, &youmonst, mattk))
			&& !mtmp->mcan && !mtmp->mspec_used) {
		pline("%s %s you %s.", Monnam(mtmp),
			Blind ? "talks to" : "smiles at",
			compat == 2 ? "engagingly" : "seductively");
	} else
	    switch (mattk->aatyp) {
		case AT_BEAM:
			pline("%s blasts!", Monnam(mtmp));
			break;
		case AT_LNCK:
		case AT_BITE:
			pline("%s bites!", Monnam(mtmp));
			break;
		case AT_KICK:
			pline("%s kicks%c", Monnam(mtmp),
				    (thick_skinned(youmonst.data) || u.sealsActive&SEAL_ECHIDNA) ? '.' : '!');
			break;
		case AT_STNG:
			pline("%s stings!", Monnam(mtmp));
			break;
		case AT_BUTT:
			pline("%s butts!", Monnam(mtmp));
			break;
		case AT_TUCH:
			if(mtmp->data == &mons[PM_EDDERKOP]) pline("%s slashes you with bladed shadows!", Monnam(mtmp));
			else pline("%s touches you!", Monnam(mtmp));
			break;
		case AT_TENT:
			pline("%s tentacles suck you!",
				        s_suffix(Monnam(mtmp)));
		break;
		case AT_WHIP:
			pline("%s barbed whips lash you!",
				        s_suffix(Monnam(mtmp)));
		break;
		case AT_EXPL:
		case AT_BOOM:
			pline("%s explodes!", Monnam(mtmp));
			break;
		default:
			pline("%s hits!", Monnam(mtmp));
	    }
		if(mattk->adtyp == AD_WET){
			You("are soaking wet!");
		}
}

STATIC_OVL void
missmu(mtmp, nearmiss, mattk)		/* monster missed you */
register struct monst *mtmp;
register boolean nearmiss;
register struct attack *mattk;
{
	if (!canspotmon(mtmp))
	    map_invisible(mtmp->mx, mtmp->my);

	if(could_seduce(mtmp, &youmonst, mattk) && !mtmp->mcan)
	    pline("%s pretends to be friendly.", Monnam(mtmp));
	else {
	    if (!flags.verbose || !nearmiss)
		pline("%s misses.", Monnam(mtmp));
	    else
		pline("%s just misses!", Monnam(mtmp));
	}
	stop_occupation();
}

STATIC_OVL void
mswings(mtmp, otemp)		/* monster swings obj */
register struct monst *mtmp;
register struct obj *otemp;
{
	if (!flags.verbose || Blind || !mon_visible(mtmp))
		return;
	pline("%s %s %s %s.", Monnam(mtmp),
	      (objects[otemp->otyp].oc_dir & PIERCE) ? "thrusts" : "swings",
	      mhis(mtmp), singular(otemp, xname));
}

/* return how a poison attack was delivered */
const char *
mpoisons_subj(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
	if (mattk->aatyp == AT_WEAP) {
	    struct obj *mwep = (mtmp == &youmonst) ? uwep : MON_WEP(mtmp);
	    /* "Foo's attack was poisoned." is pretty lame, but at least
	       it's better than "sting" when not a stinging attack... */
	    return (!mwep || !mwep->opoisoned) ? "attack" : "weapon";
	} 
	else if(mattk->adtyp == AD_SHDW){
		return "shadow blade";
	} 
	else if(mattk->adtyp == AD_STAR){
		return "starlight rapier";
	} 
	else if(mattk->aatyp == AT_HODS){
	    struct obj *mwep = uwep;
	    /* "Foo's attack was poisoned." is pretty lame, but at least
	       it's better than "sting" when not a stinging attack... */
	    return (!mwep || !mwep->opoisoned) ? "attack" : "weapon";
	} else {
	    return (mattk->aatyp == AT_TUCH) ? "contact" :
		   (mattk->aatyp == AT_GAZE) ? "gaze" :
		   (mattk->aatyp == AT_ENGL) ? "vapor" :
		   (mattk->aatyp == AT_BITE || mattk->aatyp == AT_LNCK) ? "bite" : "sting";
	}
}

/* called when your intrinsic speed is taken away */
void
u_slow_down()
{
	HFast = 0L;
	if (!Fast)
	    You("slow down.");
	else	/* speed boots */
	    Your("quickness feels less natural.");
	exercise(A_DEX, FALSE);
}

#endif /* OVL1 */
#ifdef OVLB

STATIC_OVL void
wildmiss(mtmp, mattk)		/* monster attacked your displaced image */
	register struct monst *mtmp;
	register struct attack *mattk;
{
	int compat;

	/* no map_invisible() -- no way to tell where _this_ is coming from */

	if (!flags.verbose) return;
	if (!cansee(mtmp->mx, mtmp->my)) return;
		/* maybe it's attacking an image around the corner? */

	compat = (mattk->adtyp == AD_SEDU || mattk->adtyp == AD_SSEX || mattk->adtyp == AD_LSEX) &&
		 could_seduce(mtmp, &youmonst, (struct attack *)0);

	if (is_blind(mtmp) || (Invis && !perceives(mtmp->data))) {
	    const char *swings =
		mattk->aatyp == AT_BITE || mattk->aatyp == AT_LNCK ? "snaps" :
		mattk->aatyp == AT_KICK ? "kicks" :
		(mattk->aatyp == AT_STNG ||
		 mattk->aatyp == AT_BUTT ||
		 nolimbs(mtmp->data)) ? "lunges" : "swings";

	    if (compat)
		pline("%s tries to touch you and misses!", Monnam(mtmp));
	    else
		switch(rn2(3)) {
		case 0: pline("%s %s wildly and misses!", Monnam(mtmp),
			      swings);
		    break;
		case 1: pline("%s attacks a spot beside you.", Monnam(mtmp));
		    break;
		case 2: pline("%s strikes at %s!", Monnam(mtmp),
				levl[mtmp->mux][mtmp->muy].typ == WATER
				    ? "empty water" : "thin air");
		    break;
		default:pline("%s %s wildly!", Monnam(mtmp), swings);
		    break;
		}

	} else if (Displaced) {
	    if (compat)
		pline("%s smiles %s at your %sdisplaced image...",
			Monnam(mtmp),
			compat == 2 ? "engagingly" : "seductively",
			Invis ? "invisible " : "");
	    else
		pline("%s strikes at your %sdisplaced image and misses you!",
			/* Note: if you're both invisible and displaced,
			 * only monsters which see invisible will attack your
			 * displaced image, since the displaced image is also
			 * invisible.
			 */
			Monnam(mtmp),
			Invis ? "invisible " : "");

	} else if (Underwater) {
	    /* monsters may miss especially on water level where
	       bubbles shake the player here and there */
	    if (compat)
		pline("%s reaches towards your distorted image.",Monnam(mtmp));
	    else
		pline("%s is fooled by water reflections and misses!",Monnam(mtmp));

	} 
	else if(mtmp->mcrazed){
		pline("%s flails around randomly.",Monnam(mtmp));
	}
	else impossible("%s attacks you without knowing your location?",
		Monnam(mtmp));
}

void
expels(mtmp, mdat, message)
register struct monst *mtmp;
register struct permonst *mdat; /* if mtmp is polymorphed, mdat != mtmp->data */
boolean message;
{
	if (message) {
		if (is_animal(mdat))
			You("get regurgitated!");
		else {
			char blast[40];
			register int i;

			blast[0] = '\0';
			for(i = 0; i < NATTK; i++)
				if(mdat->mattk[i].aatyp == AT_ENGL)
					break;
			if (mdat->mattk[i].aatyp != AT_ENGL)
			      impossible("Swallower has no engulfing attack?");
			else {
				if (is_whirly(mdat)) {
					switch (mdat->mattk[i].adtyp) {
						case AD_ELEC:
							Strcpy(blast,
						      " in a shower of sparks");
							break;
						case AD_COLD:
							Strcpy(blast,
							" in a blast of frost");
							break;
					}
				} else
					Strcpy(blast, " with a squelch");
				You("get expelled from %s%s!",
				    mon_nam(mtmp), blast);
			}
		}
	}
	unstuck(mtmp);	/* ball&chain returned in unstuck() */
	mnexto(mtmp);
	newsym(u.ux,u.uy);
	spoteffects(TRUE);
	/* to cover for a case where mtmp is not in a next square */
	if(um_dist(mtmp->mx,mtmp->my,1))
		pline("Brrooaa...  You land hard at some distance.");
}

#endif /* OVLB */
#ifdef OVL0

/* select a monster's next attack, possibly substituting for its usual one */
struct attack *
getmattk(mptr, indx, prev_result, alt_attk_buf)
struct permonst *mptr;
int indx, prev_result[];
struct attack *alt_attk_buf;
{
    struct attack *attk = &mptr->mattk[indx];

    /* prevent a monster with two consecutive disease or hunger attacks
       from hitting with both of them on the same turn; if the first has
       already hit, switch to a stun attack for the second */
    if (indx > 0 && prev_result[indx - 1] > 0 &&
	    (attk->adtyp == AD_DISE ||
		attk->adtyp == AD_PEST ||
		attk->adtyp == AD_FAMN ||
		mptr == &mons[PM_ASTRAL_DEVA]) &&
	    attk->adtyp == mptr->mattk[indx - 1].adtyp
	) {
		*alt_attk_buf = *attk;
		attk = alt_attk_buf;
		attk->adtyp = AD_STUN;
    }
    if (indx > 0 && prev_result[indx - 1] > 0 &&
		(attk->adtyp == AD_SHRD) &&
	    attk->adtyp == mptr->mattk[indx - 1].adtyp
	) {
		*alt_attk_buf = *attk;
		attk = alt_attk_buf;
		attk->adtyp = AD_PHYS;
    }
    return attk;
}

/*
 * mattacku: monster attacks you
 *	returns 1 if monster dies (e.g. "yellow light"), 0 otherwise
 *	Note: if you're displaced or invisible the monster might attack the
 *		wrong position...
 *	Assumption: it's attacking you or an empty square; if there's another
 *		monster which it attacks by mistake, the caller had better
 *		take care of it...
 */
int
mattacku(mtmp)
	register struct monst *mtmp;
{
	struct	attack	*mattk, alt_attk;
	int	i, j, tmp, tchtmp, sum[NATTK];
	struct	permonst *mdat = mtmp->data;
	boolean ranged = (distu(mtmp->mx, mtmp->my) > 3);
		/* Is it near you?  Affects your actions */
	boolean range2 = !monnear(mtmp, mtmp->mux, mtmp->muy);
		/* Does it think it's near you?  Affects its actions */
	boolean foundyou = (mtmp->mux==u.ux && mtmp->muy==u.uy);
		/* Is it attacking you or your image? */
	boolean youseeit = canseemon(mtmp);
		/* Might be attacking your image around the corner, or
		 * invisible, or you might be blind....
		 */
	
	if(!ranged) nomul(0, NULL);
	if(mtmp->mhp <= 0 || (Underwater && !is_swimmer(mtmp->data)))
	    return(0);
	
	if(!(u.uevent.udemigod) && is_weeping(mtmp->data)) mtmp->movement = 0; /*Only attack once per turn*/
	
	/* If swallowed, can only be affected by u.ustuck */
	if(u.uswallow) {
	    if(mtmp != u.ustuck)
		return(0);
	    u.ustuck->mux = u.ux;
	    u.ustuck->muy = u.uy;
	    range2 = 0;
	    foundyou = 1;
	    if(u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves+20) return (0); /* stomachs can't hurt you! */
	}

#ifdef STEED
	else if (u.usteed) {
		if (mtmp == u.usteed)
			/* Your steed won't attack you */
			return (0);
		/* Orcs like to steal and eat horses and the like */
		if (!rn2(is_orc(mtmp->data) ? 2 : 4) &&
				distu(mtmp->mx, mtmp->my) <= 2) {
			/* Attack your steed instead */
			i = mattackm(mtmp, u.usteed);
			if ((i & MM_AGR_DIED))
				return (1);
			if (i & MM_DEF_DIED || u.umoved)
				return (0);
			/* Let your steed retaliate */
			return (!!(mattackm(u.usteed, mtmp) & MM_DEF_DIED));
		}
	}
#endif

	if (u.uundetected && !range2 && foundyou && !u.uswallow) {
		u.uundetected = 0;
		if (is_hider(youmonst.data)) {
		    coord cc; /* maybe we need a unexto() function? */
		    struct obj *obj;

		    You("fall from the %s!", ceiling(u.ux,u.uy));
		    if (enexto(&cc, u.ux, u.uy, youmonst.data)) {
			remove_monster(mtmp->mx, mtmp->my);
			newsym(mtmp->mx,mtmp->my);
			place_monster(mtmp, u.ux, u.uy);
			if(mtmp->wormno) worm_move(mtmp);
			teleds(cc.x, cc.y, TRUE);
			set_apparxy(mtmp);
			newsym(u.ux,u.uy);
		    } else {
			pline("%s is killed by a falling %s (you)!",
					Monnam(mtmp), youmonst.data->mname);
			killed(mtmp);
			newsym(u.ux,u.uy);
			if (mtmp->mhp > 0) return 0;
			else return 1;
		    }
		    if (youmonst.data->mlet != S_PIERCER)
			return(0);	/* trappers don't attack */

		    obj = which_armor(mtmp, WORN_HELMET);
		    if (obj && is_hard(obj)) {
			Your("blow glances off %s helmet.",
			               s_suffix(mon_nam(mtmp)));
		    } else {
			if (3 + find_mac(mtmp) <= rnd(20)) {
			    pline("%s is hit by a falling piercer (you)!",
								Monnam(mtmp));
			    if ((mtmp->mhp -= d(3,6)) < 1)
				killed(mtmp);
			} else
			  pline("%s is almost hit by a falling piercer (you)!",
								Monnam(mtmp));
		    }
		} else {
		    if (!youseeit)
			pline("It tries to move where you are hiding.");
		    else {
			/* Ugly kludge for eggs.  The message is phrased so as
			 * to be directed at the monster, not the player,
			 * which makes "laid by you" wrong.  For the
			 * parallelism to work, we can't rephrase it, so we
			 * zap the "laid by you" momentarily instead.
			 */
			struct obj *obj = level.objects[u.ux][u.uy];

			if (obj ||
			      (youmonst.data->mlet == S_EEL && is_pool(u.ux, u.uy))) {
			    int save_spe = 0; /* suppress warning */
			    if (obj) {
				save_spe = obj->spe;
				if (obj->otyp == EGG) obj->spe = 0;
			    }
			    if (youmonst.data->mlet == S_EEL)
		pline("Wait, %s!  There's a hidden %s named %s there!",
				m_monnam(mtmp), youmonst.data->mname, plname);
			    else
	     pline("Wait, %s!  There's a %s named %s hiding under %s!",
				m_monnam(mtmp), youmonst.data->mname, plname,
				doname(level.objects[u.ux][u.uy]));
			    if (obj) obj->spe = save_spe;
			} else
			    impossible("hiding under nothing?");
		    }
		    newsym(u.ux,u.uy);
		}
		return(0);
	}
	if (youmonst.data->mlet == S_MIMIC && youmonst.m_ap_type &&
		    !range2 && foundyou && !u.uswallow) {
		if (!youseeit) pline("It gets stuck on you.");
		else pline("Wait, %s!  That's a %s named %s!",
			   m_monnam(mtmp), youmonst.data->mname, plname);
		u.ustuck = mtmp;
		youmonst.m_ap_type = M_AP_NOTHING;
		youmonst.mappearance = 0;
		newsym(u.ux,u.uy);
		return(0);
	}

	/* player might be mimicking an object */
	if (youmonst.m_ap_type == M_AP_OBJECT && !range2 && foundyou && !u.uswallow) {
	    if (!youseeit)
		 pline("%s %s!", Something,
			(likes_gold(mtmp->data) && youmonst.mappearance == GOLD_PIECE) ?
			"tries to pick you up" : "disturbs you");
	    else pline("Wait, %s!  That %s is really %s named %s!",
			m_monnam(mtmp),
			mimic_obj_name(&youmonst),
			an(mons[u.umonnum].mname),
			plname);
	    if (multi < 0) {	/* this should always be the case */
		char buf[BUFSZ];
		Sprintf(buf, "You appear to be %s again.",
			Upolyd ? (const char *) an(youmonst.data->mname) :
			    (const char *) "yourself");
		unmul(buf);	/* immediately stop mimicking */
	    }
	    return 0;
	}

/*	Work out the armor class differential	*/
	tmp = AC_VALUE(u.uac) + 10;		/* tmp ~= 0 - 20 */
	tchtmp = AC_VALUE(base_uac()) + 10;
	tmp += mtmp->m_lev;
	tchtmp += mtmp->m_lev;
	tmp += u.ustdy;
	tchtmp += u.ustdy;
//ifdef BARD
	tmp += mtmp->encouraged;
	if (wizard && mtmp->encouraged)
		pline("[%s +%d]", Monnam(mtmp), mtmp->encouraged);
//endif
	if(mtmp->data == &mons[PM_UVUUDAUM]){
		tmp += 20;
		tchtmp += 20;
	}
	if(mtmp->data == &mons[PM_CHOKHMAH_SEPHIRAH]){
		tmp += u.chokhmah;
		tchtmp += u.chokhmah;
	}
	if(multi < 0){
		tmp += 4;
		tchtmp += 4;
	}
	if((Invis && !perceives(mdat)) || is_blind(mtmp)){
		tmp -= 2;
		tchtmp -= 2;
	}
	if(mtmp->mtrapped){
		tmp -= 2;
		tchtmp -= 2;
	}
	if(tmp <= 0) tmp = 1;
	if(tchtmp <= 0) tchtmp = 1;

	/* make eels visible the moment they hit/miss us */
	if(mdat->mlet == S_EEL && mtmp->minvis && cansee(mtmp->mx,mtmp->my)) {
		mtmp->minvis = 0;
		newsym(mtmp->mx,mtmp->my);
	}

/*	Special demon handling code */
	if(!mtmp->cham && is_demon(mdat) && !range2
	   && mtmp->data != &mons[PM_BALROG]
	   && mtmp->data != &mons[PM_SUCCUBUS]
	   && mtmp->data != &mons[PM_INCUBUS])
	    if(!mtmp->mcan && !rn2(13))	msummon(mtmp);

/*	Special lycanthrope handling code */
	if(!mtmp->cham && is_were(mdat) && !range2) {

	    if(is_human(mdat)) {
			if(!rn2(5 - (night() * 2)) && !mtmp->mcan) new_were(mtmp);
	    } else if(!rn2(30) && !mtmp->mcan) new_were(mtmp);
	    mdat = mtmp->data;

	    if(!rn2(10) && !mtmp->mcan) {
	    	int numseen, numhelp;
		char buf[BUFSZ], genericwere[BUFSZ];

		Strcpy(genericwere, "creature");
		numhelp = were_summon(mdat, FALSE, &numseen, genericwere);
		if (youseeit) {
			pline("%s summons help!", Monnam(mtmp));
			if (numhelp > 0) {
			    if (numseen == 0)
				You_feel("hemmed in.");
			} else pline("But none comes.");
		} else {
			const char *from_nowhere;

			if (flags.soundok) {
				pline("%s %s!", Something,
					makeplural(growl_sound(mtmp)));
				from_nowhere = "";
			} else from_nowhere = " from nowhere";
			if (numhelp > 0) {
			    if (numseen < 1) You_feel("hemmed in.");
			    else {
				if (numseen == 1)
			    		Sprintf(buf, "%s appears",
							an(genericwere));
			    	else
			    		Sprintf(buf, "%s appear",
							makeplural(genericwere));
				pline("%s%s!", upstart(buf), from_nowhere);
			    }
			} /* else no help came; but you didn't know it tried */
		}
	    }
	}

	if(mdat == &mons[PM_LILLEND] && rn2(2)){
		mdat = find_mask(mtmp);
		if(!Blind && mdat != &mons[PM_LILLEND]) pline("%s uses a %s mask!",Monnam(mtmp),mdat->mname);
	}
	if(u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves+20) {
	    /* monsters won't attack you */
	    if(mtmp == u.ustuck)
		pline("%s loosens its grip slightly.", Monnam(mtmp));
	    else if(!range2) {
		if (youseeit || sensemon(mtmp))
		    pline("%s starts to attack you, but pulls back.",
			  Monnam(mtmp));
		else
		    You_feel("%s move nearby.", something);
	    }
	    return (0);
	}

	/* Unlike defensive stuff, don't let them use item _and_ attack. */
	if(find_offensive(mtmp)) {
		int foo = use_offensive(mtmp);

		if (foo != 0) return(foo==1);
	}

	if(u.sealsActive&SEAL_OTIAX && !rn2(5)){
		tmp = 0;
		tchtmp = 0;
	}
	for(i = 0; i < NATTK; i++) {

	    sum[i] = 0;
	    mattk = getmattk(mdat, i, sum, &alt_attk);
	    if (u.uswallow && (mattk->aatyp != AT_ENGL && mattk->aatyp != AT_ILUR))
		continue;
		
		/*Plasteel helms cover the face and prevent bite attacks*/
		if((mtmp->misc_worn_check & W_ARMH) && 
			(((which_armor(mtmp, W_ARMH))->otyp) == PLASTEEL_HELM || ((which_armor(mtmp, W_ARMH))->otyp) == CRYSTAL_HELM) && 
			(mattk->aatyp == AT_BITE || mattk->aatyp == AT_LNCK || 
				(mattk->aatyp == AT_ENGL && !u.uswallow) ||
				(mattk->aatyp == AT_TENT && is_mind_flayer(mtmp->data))
			)
		) continue;
		
	    switch(mattk->aatyp) {
		case AT_CLAW:	/* "hand to hand" attacks */
		case AT_KICK:
		case AT_BITE:
		case AT_STNG:
		case AT_BUTT:
		case AT_TENT:
		case AT_WHIP:
			if(!range2 && (!MON_WEP(mtmp) || mtmp->mconf || Conflict ||
					!touch_petrifies(youmonst.data))) {
			    if (foundyou) {
				if(tmp > (j = rnd(20+i))) {
				    if (mattk->aatyp != AT_KICK ||
					    !(thick_skinned(youmonst.data) || u.sealsActive&SEAL_ECHIDNA))
					sum[i] = hitmu(mtmp, mattk);
				} else
				    missmu(mtmp, (tmp == j), mattk);
			    } else
				wildmiss(mtmp, mattk);
			}
			if(mdat == &mons[PM_DEMOGORGON] && sum[i]){
				mtmp->mvar2 = mtmp->mvar2+1;
				if(!range2 && mtmp->mvar2>=2){
					struct attack rend = {AT_HUGS, AD_SHRD, 3, 12};
					hitmu(mtmp, &rend);
					mtmp->mvar2=0;
				}
			}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_TUCH:
			if(!range2 && (!MON_WEP(mtmp) || mtmp->mconf || Conflict ||
					!touch_petrifies(youmonst.data))) {
			    if (foundyou) {
				if(tchtmp > (j = rnd(20+i))) {
				    if (mattk->aatyp != AT_KICK ||
					    !(thick_skinned(youmonst.data) || u.sealsActive&SEAL_ECHIDNA))
					sum[i] = hitmu(mtmp, mattk);
				} else
				    missmu(mtmp, (tchtmp == j), mattk);
			    } else
				wildmiss(mtmp, mattk);
			}
			if(mdat == &mons[PM_DEMOGORGON] && sum[i]){
				mtmp->mvar2 = mtmp->mvar2+1;
				if(!range2 && mtmp->mvar2>=2){
					struct attack rend = {AT_HUGS, AD_SHRD, 3, 12};
					hitmu(mtmp, &rend);
					mtmp->mvar2=0;
				}
			}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_SHDW:
			if(!range2 && (!MON_WEP(mtmp) || mtmp->mconf || Conflict)) {
			    if (foundyou) {
				if(tchtmp > (j = rnd(20+i))) {
				    if (mattk->aatyp != AT_KICK ||
					    !(thick_skinned(youmonst.data) || u.sealsActive&SEAL_ECHIDNA))
					sum[i] = hitmu(mtmp, mattk);
				} else
				    missmu(mtmp, (tchtmp == j), mattk);
			    } else
					wildmiss(mtmp, mattk);
			}
			if(mdat == &mons[PM_DEMOGORGON] && sum[i]){
				mtmp->mvar2 = mtmp->mvar2+1;
				if(!range2 && mtmp->mvar2>=2){
					struct attack rend = {AT_HUGS, AD_SHRD, 3, 12};
					hitmu(mtmp, &rend);
					mtmp->mvar2=0;
				}
			}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_HUGS:	/* automatic if prev two attacks succeed */
			/* Note: if displaced, prev attacks never succeeded */
			if((!range2 && i>=2 && sum[i-1] && sum[i-2])
							|| mtmp == u.ustuck)
				sum[i]= hitmu(mtmp, mattk);
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_GAZE:	/* can affect you either ranged or not */
			/* Medusa gaze already operated through m_respond in
			 * 	dochug();
			 * Weeping angels gaze during movemon().
			 * Don't gaze more than once per round.
			 */
			if (mdat != &mons[PM_MEDUSA] && !is_weeping(mdat))
				sum[i] = gazemu(mtmp, mattk);
			if(mdat == &mons[PM_DEMOGORGON] && sum[i]){
				mtmp->mvar1 = mtmp->mvar1+1;
				if(!range2 && mtmp->mvar1>=2){
					struct attack theft = {AT_CLAW, AD_SEDU, 1, 1};
					You("have met the twin gaze of Demogorgon, Prince of Demons!");
					You("feel his command within you!");
					hitmu(mtmp, &theft);
					mtmp->mvar1=0;
				}
			}
		break;

///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_EXPL:	/* automatic hit if next to, and aimed at you */
			if(!range2) sum[i] = explmu(mtmp, mattk, foundyou);
			if (is_fern_spore(mdat)) spore_dies(mtmp);
		break;

///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_ENGL:
			if (!range2) {
			    if(foundyou) {
				if((u.uswallow || tmp > (j = rnd(20+i))) && !(u.sealsActive&SEAL_AHAZU)) {
				    /* Force swallowing monster to be
				     * displayed even when player is
				     * moving away */
				    flush_screen(1);
				    sum[i] = gulpmu(mtmp, mattk);
				} else {
					if(u.uswallow) expels(mtmp, mtmp->data, TRUE);
				    else missmu(mtmp, (tmp == j), mattk);
				}
			    } else if (is_animal(mtmp->data)) {
				pline("%s gulps some air!", Monnam(mtmp));
			    } else {
				if (youseeit)
				    pline("%s lunges forward and recoils!",
					  Monnam(mtmp));
				else
				    You_hear("a %s nearby.",
					     is_whirly(mtmp->data) ?
						"rushing noise" : "splat");
			   }
			}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_ILUR:
			if (!range2) {
			    if(foundyou) {
				if(u.uswallow || tmp > (j = rnd(20+i))) {
				    /* Force swallowing monster to be
				     * displayed even when player is
				     * moving away */
				    flush_screen(1);
				    if(u.ustuck == mtmp) sum[i] = gulpmu(mtmp, mattk);
				    else sum[i] = hitmu(mtmp, mattk);
				} else {
					if(u.uswallow) expels(mtmp, mtmp->data, TRUE);
				    else missmu(mtmp, (tmp == j), mattk);
				}
			    } else if (is_animal(mtmp->data)) {
				pline("%s gulps some air!", Monnam(mtmp));
			    } else {
				if (youseeit)
				    pline("%s lunges forward and recoils!",
					  Monnam(mtmp));
				else
				    You_hear("a %s nearby.",
					     is_whirly(mtmp->data) ?
						"rushing noise" : "splat");
			   }
			}
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_BREA:
//			if( mdat == &mons[PM_UNMASKED_GOD_EMPEROR] ) mtmp->mspec_used = 0;
			if(is_dragon(mdat)) flags.drgn_brth = 1;
			if(range2) sum[i] = breamu(mtmp, mattk);
			flags.drgn_brth = 0;
			/* Note: breamu takes care of displacement */
//			if( mdat == &mons[PM_UNMASKED_GOD_EMPEROR] ) mtmp->mspec_used = 0;
			break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_SPIT:
			if(mdat == &mons[PM_MOTHER_LILITH])
				if(mtmp->mcan) mtmp->mcan=0;
			if(range2) sum[i] = spitmu(mtmp, mattk);
			/* Note: spitmu takes care of displacement */
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_ARRW:{
			int n;
			if((mattk->adtyp != AD_SHDW || range2) && lined_up(mtmp)){
				if (canseemon(mtmp)) pline("%s shoots at you!", Monnam(mtmp));
				for(n = d(mattk->damn, mattk->damd); n > 0; n--) sum[i] = firemu(mtmp, mattk);
			/* Note: firemu takes care of displacement */
			}
		} break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_TNKR:{
			if(range2 && !mtmp->mspec_used){
				struct monst *mlocal;
				int mdx=0, mdy=0, i;
				
				if(mtmp->mux - mtmp->mx < 0) mdx = -1;
				else if(mtmp->mux - mtmp->mx > 0) mdx = +1;
				if(mtmp->muy - mtmp->my < 0) mdy = -1;
				else if(mtmp->muy - mtmp->my > 0) mdy = +1;
				
				if(mtmp->data == &mons[PM_HOOLOOVOO]){
					if(rn2(4)) mlocal = makemon(&mons[PM_GOLDEN_HEART], mtmp->mx+mdx, mtmp->my+mdy, MM_ADJACENTOK|MM_ADJACENTSTRICT|MM_NOCOUNTBIRTH);
					else mlocal = makemon(&mons[PM_ID_JUGGERNAUT], mtmp->mx+mdx, mtmp->my+mdy, MM_ADJACENTOK|MM_ADJACENTSTRICT|MM_NOCOUNTBIRTH);
				} else {
					if(u.ulevel > 10 && !rn2(10)) mlocal = makemon(&mons[PM_FIREWORK_CART], mtmp->mx+mdx, mtmp->my+mdy, MM_ADJACENTOK|MM_ADJACENTSTRICT|MM_NOCOUNTBIRTH);
					else mlocal = makemon(&mons[PM_CLOCKWORK_SOLDIER+rn2(3)], mtmp->mx+mdx, mtmp->my+mdy, MM_ADJACENTOK|MM_ADJACENTSTRICT|MM_NOCOUNTBIRTH);
				}
				
				if(mlocal){
					for(i=0;i<8;i++) if(xdir[i] == mdx && ydir[i] == mdy) break;
					mlocal->mvar1 = i;
					
					mtmp->mspec_used = rnd(6);
				}
			}
			mtmp->mflee = 1;
		}break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_BEAM:
			if(lined_up(mtmp) && dist2(mtmp->mx,mtmp->my,mtmp->mux,mtmp->muy) <= BOLT_LIM*BOLT_LIM){
				if (foundyou) sum[i] = hitmu(mtmp, mattk);
				else wildmiss(mtmp, mattk);
			}
		case AT_LNCK:
		case AT_LRCH:{
			if(dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 8 && couldsee(mtmp->mx, mtmp->my)){
				if(mtmp->mconf || Conflict ||
					!touch_petrifies(youmonst.data)){
					if (foundyou){
						if(tmp > (j = rnd(20+i))) {
							sum[i] = hitmu(mtmp, mattk);
						} else
							missmu(mtmp, (tmp == j), mattk);
					} else
						wildmiss(mtmp, mattk);
				}
				if(mdat == &mons[PM_DEMOGORGON] && sum[i]){
					mtmp->mvar2 = mtmp->mvar2+1;
					if(!range2 && mtmp->mvar2>=2){
						struct attack rend = {AT_HUGS, AD_SHRD, 3, 12};
						hitmu(mtmp, &rend);
						mtmp->mvar2=0;
					}
				}
			}
		}break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AT_WEAP:
		case AT_DEVA:
			if(range2) {
#ifdef REINCARNATION
				if (!Is_rogue_level(&u.uz))
#endif
					thrwmu(mtmp);
			} else {
			    int hittmp = 0;

			    /* Rare but not impossible.  Normally the monster
			     * wields when 2 spaces away, but it can be
			     * teleported or whatever....
			     */
			    if (mtmp->weapon_check == NEED_WEAPON ||
							!MON_WEP(mtmp)) {
					mtmp->weapon_check = NEED_HTH_WEAPON;
					/* mon_wield_item resets weapon_check as
					 * appropriate */
					if (mon_wield_item(mtmp) != 0) break;
			    }
			    if (foundyou) {
					otmp = MON_WEP(mtmp);
					if(otmp) {
						hittmp = hitval(otmp, &youmonst);
						tmp += hittmp;
						mswings(mtmp, otmp);
					}
					if(tmp > (j = dieroll = rnd(20+i)))
						sum[i] = hitmu(mtmp, mattk);
					else
						missmu(mtmp, (tmp == j), mattk);
					/* KMH -- Don't accumulate to-hit bonuses */
					if (otmp)
						tmp -= hittmp;
			    } else
					wildmiss(mtmp, mattk);
			}
			break;
		case AT_HODS:
			if(!range2){
			    int hittmp = 0;

			    if (foundyou) {
					otmp = uwep;
					if(otmp) {
						hittmp = hitval(otmp, &youmonst);
						tmp += hittmp;
						mswings(mtmp, otmp);
					}
					if(tmp > (j = dieroll = rnd(20+i)))
						sum[i] = hitmu(mtmp, mattk);
					else
						missmu(mtmp, (tmp == j), mattk);
					/* KMH -- Don't accumulate to-hit bonuses */
					if (otmp)
						tmp -= hittmp;
			    } else
					wildmiss(mtmp, mattk);
			}
		break;
		case AT_MAGC:
		case AT_MMGC:{
			int temp=0;
			if( mdat == &mons[PM_ASMODEUS] ) mtmp->mspec_used = 0;
			else if( mdat == &mons[PM_DEMOGORGON] && rn2(3) ) mtmp->mspec_used = 0;
			else if( mdat == &mons[PM_LAMASHTU] && rn2(3) ) mtmp->mspec_used = 0;
			else if( mdat == &mons[PM_ELDER_PRIEST] && rn2(2) ) mtmp->mspec_used = 0;
			else if( mdat == &mons[PM_ALHOON] && rn2(2) ) mtmp->mspec_used = 0;
			else if( mdat == &mons[PM_EMBRACED_DROWESS]) mtmp->mspec_used = 0;
			else if( mdat == &mons[PM_ELDER_BRAIN]) mtmp->mspec_used = 0;
			else if( mdat == &mons[PM_AVATAR_OF_LOLTH]) mtmp->mspec_used = 0;
			else if( mdat == &mons[PM_HOOLOOVOO] && rn2(2) ) break;
//			else if( mdat == &mons[PM_UNMASKED_GOD_EMPEROR] ) mtmp->mspec_used = 0;
			
//			if( mdat == &mons[PM_ASMODEUS] && mattk->adtyp == AD_SPEL && rn2(2) ) return 0;
//			if( mdat == &mons[PM_ASMODEUS] && mattk->adtyp == AD_FIRE && rn2(2) ) return 0;
			if( mdat == &mons[PM_GRAZ_ZT]) temp = mtmp->mspec_used;
			if( mdat == &mons[PM_QUINON] ) {
				temp = mtmp->mspec_used;
				mtmp->mspec_used = 0;
			}
			if( mdat == &mons[PM_DEMOGORGON] && !range2 && !mtmp->mflee && rn2(6)) break; //cast spells more rarely if he's in melee range
			if (range2 && mattk->adtyp != AD_SPEL && mattk->adtyp != AD_CLRC)
			    sum[i] = buzzmu(mtmp, mattk, mtmp->m_lev);
			else {
				int range;
				if(is_orc(mtmp->data) || 
					mtmp->data == &mons[PM_HEDROW_WARRIOR] || 
					mtmp->data == &mons[PM_MINOTAUR_PRIESTESS]
				) range = (BOLT_LIM*BOLT_LIM)/2;
				else range = (BOLT_LIM*BOLT_LIM);
			    if (foundyou){
					if(couldsee(mtmp->mx, mtmp->my) && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= range)
						sum[i] = castmu(mtmp, mattk, TRUE, TRUE);
					else
						sum[i] = castmu(mtmp, mattk, FALSE, FALSE);
			    } else{
					if(couldsee(mtmp->mx, mtmp->my) && dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= range)
						sum[i] = castmu(mtmp, mattk, TRUE, FALSE);
					else
						sum[i] = castmu(mtmp, mattk, FALSE, FALSE);
				}
			}
			if( mdat == &mons[PM_ASMODEUS] && !rn2(3) ) return 3;
			if( mdat == &mons[PM_QUINON] ) {
				mtmp->mspec_used = temp;
			}
			if( mdat == &mons[PM_GRAZ_ZT] && temp == 0) mtmp->mspec_used = 4;
//			if( mdat == &mons[PM_UNMASKED_GOD_EMPEROR] ) mtmp->mspec_used = 0;
			break;
			}

		default:		/* no attack */
			break;
	    }
	    if(flags.botl) bot();
	/* give player a chance of waking up before dying -kaa */
	    if(sum[i] == 1) {	    /* successful attack */
		if (u.usleep && u.usleep < monstermoves && rn2(20) < ACURR(A_WIS)) {
		    multi = -1;
		    nomovemsg = "The combat suddenly awakens you.";
		}
	    }
	    if(sum[i] == 2) return 1;		/* attacker dead */
	    if(sum[i] == 3) break;  /* attacker teleported, no more attacks */
	    /* sum[i] == 0: unsuccessful attack */
	}
	if(mdat == &mons[PM_DEMOGORGON]){ 
		mtmp->mvar1 = 0;
		mtmp->mvar2 = 0;
	}
	return(0);
}

#endif /* OVL0 */
#ifdef OVLB

/*
 * helper function for some compilers that have trouble with hitmu
 */

STATIC_OVL void
hurtarmor(attk)
int attk;
{
	int	hurt;

	switch(attk) {
	    /* 0 is burning, which we should never be called with */
	    case AD_RUST: hurt = 1; break;
	    case AD_CORR: hurt = 3; break;
	    default: hurt = 2; break;
	}

	/* What the following code does: it keeps looping until it
	 * finds a target for the rust monster.
	 * Head, feet, etc... not covered by metal, or covered by
	 * rusty metal, are not targets.  However, your body always
	 * is, no matter what covers it.
	 */
	while (1) {
	    switch(rn2(5)) {
	    case 0:
		if (!uarmh || !rust_dmg(uarmh, xname(uarmh), hurt, FALSE, &youmonst))
			continue;
		break;
	    case 1:
		if (uarmc) {
		    (void)rust_dmg(uarmc, xname(uarmc), hurt, TRUE, &youmonst);
		    break;
		}
		/* Note the difference between break and continue;
		 * break means it was hit and didn't rust; continue
		 * means it wasn't a target and though it didn't rust
		 * something else did.
		 */
		if (uarm)
		    (void)rust_dmg(uarm, xname(uarm), hurt, TRUE, &youmonst);
#ifdef TOURIST
		else if (uarmu)
		    (void)rust_dmg(uarmu, xname(uarmu), hurt, TRUE, &youmonst);
#endif
		break;
	    case 2:
		if (!uarms || !rust_dmg(uarms, xname(uarms), hurt, FALSE, &youmonst))
		    continue;
		break;
	    case 3:
		if (!uarmg || !rust_dmg(uarmg, xname(uarmg), hurt, FALSE, &youmonst))
		    continue;
		break;
	    case 4:
		if (!uarmf || !rust_dmg(uarmf, xname(uarmf), hurt, FALSE, &youmonst))
		    continue;
		break;
	    }
	    break; /* Out of while loop */
	}
}

#endif /* OVLB */
#ifdef OVL1

boolean
diseasemu(mdat)
struct permonst *mdat;
{
	if (Sick_resistance) {
		You_feel("a slight illness.");
		return FALSE;
	} else {
		make_sick(Sick ? Sick/3L + 1L : (long)rn1(ACURR(A_CON), 20),
			mdat->mname, TRUE, SICK_NONVOMITABLE);
		return TRUE;
	}
}

/* check whether slippery clothing protects from hug or wrap attack */
STATIC_OVL boolean
u_slip_free(mtmp, mattk)
struct monst *mtmp;
struct attack *mattk;
{
	struct obj *obj = (uarmc ? uarmc : uarm);

#ifdef TOURIST
	if (!obj) obj = uarmu;
#endif
	if (mattk->adtyp == AD_DRIN) obj = uarmh;
	else if (mattk->adtyp == AD_DRDX) obj = uarmg;
	else if (mattk->adtyp == AD_LEGS) obj = uarmf;

	/* mud boots block wrap attacks 80% of the time */
    static int mboots = 0;
    if (!mboots) mboots = find_mboots();
    if(mattk->adtyp == AD_WRAP && 
		rn2(5) && 
		uarmf && uarmf->otyp == mboots
	){
		pline("%s slips off of your slippery mud boots!", Monnam(mtmp));
		return TRUE; 
	}
	
	/* if your cloak/armor is greased, monster slips off; this
	   protection might fail (33% chance) when the armor is cursed */
	if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK) &&
		(!obj->cursed || rn2(3))) {
	    pline("%s %s your %s %s!",
		  Monnam(mtmp),
		  (mattk->adtyp == AD_WRAP) ?
			"slips off of" : "grabs you, but cannot hold onto",
		  obj->greased ? "greased" : "slippery",
		  /* avoid "slippery slippery cloak"
		     for undiscovered oilskin cloak */
		  (obj->greased || objects[obj->otyp].oc_name_known) ?
			xname(obj) : cloak_simple_name(obj));

	    if (obj->greased && !rn2(2)) {
		pline_The("grease wears off.");
		obj->greased = 0;
		update_inventory();
	    }
	    return TRUE;
	}
	return FALSE;
}

/* armor that sufficiently covers the body might be able to block magic */
int
magic_negation(mon)
struct monst *mon;
{
	struct obj *armor;
	int armpro = 0;
	int cpro = 0;
	
	if(u.sealsActive&SEAL_PAIMON && mon == &youmonst) return 3;

	armor = (mon == &youmonst) ? uarm : which_armor(mon, W_ARM);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
	armor = (mon == &youmonst) ? uarmc : which_armor(mon, W_ARMC);
	if(armor){
		cpro = armor->otyp == DROVEN_CLOAK ? 
			objects[armor->otyp].a_can - armor->ovar1 :
			objects[armor->otyp].a_can;
		if(armpro < cpro) armpro = cpro;
	}
	armor = (mon == &youmonst) ? uarmh : which_armor(mon, W_ARMH);
	if(mon == &youmonst && !uarmc && 
		uwep && uwep->oartifact==ART_TENSA_ZANGETSU) armpro = max(armpro, 2); //magic cancelation for tensa zangetsu
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;

	/* armor types for shirt, gloves, shoes, and shield may not currently
	   provide any magic cancellation but we should be complete */
	armor = (mon == &youmonst) ? uarmu : which_armor(mon, W_ARMU);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
	armor = (mon == &youmonst) ? uarmg : which_armor(mon, W_ARMG);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
	armor = (mon == &youmonst) ? uarmf : which_armor(mon, W_ARMF);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
	armor = (mon == &youmonst) ? uarms : which_armor(mon, W_ARMS);
	if (armor && armpro < 3){
	    armpro = min(3, armpro+objects[armor->otyp].a_can);
	}

#ifdef STEED
	/* this one is really a stretch... */
	armor = (mon == &youmonst) ? 0 : which_armor(mon, W_SADDLE);
	if (armor && armpro < objects[armor->otyp].a_can)
	    armpro = objects[armor->otyp].a_can;
#endif

	return armpro;
}

/*
 * hitmu: monster hits you
 *	  returns 2 if monster dies (e.g. "yellow light"), 1 otherwise
 *	  3 if the monster lives but teleported/paralyzed, so it can't keep
 *	       attacking you
 */
STATIC_OVL int
hitmu(mtmp, mattk)
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	register struct permonst *mdat = mtmp->data;
	register int uncancelled, ptmp, armuncancel;
	struct obj *optr;
	int dmg, armpro, permdmg;
	char	 buf[BUFSZ];
	struct permonst *olduasmon = youmonst.data;
	int res;

	if (!canspotmon(mtmp))
	    map_invisible(mtmp->mx, mtmp->my);
	
	/* Nearby monsters may be awakened NOTE: no way to tell if a player that can't move is paralyized or engraving or something */
	if(!is_silent(mtmp->data) || !(u.usleep)) wake_nearto(u.ux, u.uy, combatNoise(mtmp->data));
/*	If the monster is undetected & hits you, you should know where
 *	the attack came from.
 */
	if(mtmp->mundetected && (hides_under(mdat) || mdat->mlet == S_EEL)) {
	    mtmp->mundetected = 0;
	    if (!(Blind ? Blind_telepat : Unblind_telepat)) {
		struct obj *obj;
		const char *what;

		if ((obj = level.objects[mtmp->mx][mtmp->my]) != 0) {
		    if (Blind && !obj->dknown)
			what = something;
		    else if (is_pool(mtmp->mx, mtmp->my) && !Underwater)
			what = "the water";
		    else
			what = doname(obj);

		    pline("%s was hidden under %s!", Amonnam(mtmp), what);
		}
		newsym(mtmp->mx, mtmp->my);
	    }
	}
	if(uarm && uarm->oartifact && !rn2(10)) touch_artifact(uarm, &youmonst);

/*	First determine the base damage done */
	if(mtmp->mflee && mdat == &mons[PM_BANDERSNATCH]) dmg = d((int)mattk->damn, 2*(int)mattk->damd);
	else dmg = d((int)mattk->damn, (int)mattk->damd);
	if(is_undead(mdat) && midnight())
		dmg += d((int)mattk->damn, (int)mattk->damd); /* extra damage */

/*	Next a cancellation factor	*/
/*	Use uncancelled when the cancellation factor takes into account certain
 *	armor's special magic protection.  Otherwise just use !mtmp->mcan.
 */
	armpro = magic_negation(&youmonst);
	armuncancel = ((rn2(3) >= armpro) || !rn2(50));
	uncancelled = !mtmp->mcan && armuncancel;
	//uncancelled = !mtmp->mcan && ((rn2(3) >= armpro) || !rn2(50));
	permdmg = 0;
/*	Now, adjust damages via resistances or specific attacks */
	switch(mattk->adtyp) {
///////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_HODS:
		 if(uwep){
			if (uwep->otyp == CORPSE
				&& touch_petrifies(&mons[uwep->corpsenm])) {
			    dmg = 1;
			    pline("%s hits you with the %s corpse.",
				Monnam(mtmp), mons[uwep->corpsenm].mname);
			    if (!Stoned)
				goto do_stone;
			}
			dmg += dmgval(uwep, &youmonst, 0);
			
			if (uwep->opoisoned){
				Sprintf(buf, "%s %s",
					s_suffix(Monnam(mtmp)), mpoisons_subj(mtmp, mattk));
				poisoned(buf, A_CON, mdat->mname, 30, uwep->opoisoned);
			}
			
			if (dmg <= 0) dmg = 1;
			if (!(uwep->oartifact &&
				artifact_hit(mtmp, &youmonst, uwep, &dmg,dieroll)))
			     hitmsg(mtmp, mattk);
			if (!dmg) break;
			if (u.mh > 1 && u.mh > ((u.uac>=0) ? dmg : dmg+AC_VALUE(u.uac)) &&
				   objects[uwep->otyp].oc_material == IRON &&
					(u.umonnum==PM_BLACK_PUDDING
					|| u.umonnum==PM_BROWN_PUDDING)) {
			    /* This redundancy necessary because you have to
			     * take the damage _before_ being cloned.
			     */
			    if (u.uac < 0) dmg += AC_VALUE(u.uac);
			    if (dmg < 1) dmg = 1;
			    if (dmg > 1) exercise(A_STR, FALSE);
			    u.mh -= dmg;
			    flags.botl = 1;
			    dmg = 0;
			    if(cloneu())
			    You("divide as %s hits you!",mon_nam(mtmp));
			}
			hitmsg(mtmp, mattk);
		 }
		break;
	    case AD_PHYS:
		if (mattk->aatyp == AT_HUGS && !sticks(youmonst.data)) {
		    if(!u.ustuck && rn2(2)) {
			if (u_slip_free(mtmp, mattk)) {
			    dmg = 0;
			} else {
				if(Upolyd && u.umonnum == PM_TOVE) You("are much too slithy to grab!");
				else {
					u.ustuck = mtmp;
					pline("%s grabs you!", Monnam(mtmp));
				}
			}
		    } else if(u.ustuck == mtmp) {
			exercise(A_STR, FALSE);
			You("are being %s.",
			      (mtmp->data == &mons[PM_ROPE_GOLEM])
			      ? "choked" : "crushed");
		    }
		} else {			  /* hand to hand weapon */
		    if(mattk->aatyp == AT_WEAP && otmp) {
			if (otmp->otyp == CORPSE &&
				touch_petrifies(&mons[otmp->corpsenm])) {
			    dmg = 1;
			    pline("%s hits you with the %s corpse.",
				Monnam(mtmp), mons[otmp->corpsenm].mname);
			    if (!Stoned) goto do_stone;
			}
			/* WAC -- Real weapon?
			 * Could be stuck with a cursed bow/polearm it wielded
			 */
			if (/* if you strike with a bow... */
				is_launcher(otmp) ||
				/* or strike with a missile in your hand... */
			    ((is_missile(otmp) || is_ammo(otmp)) &&
					!(otmp->otyp == SILVER_CHAKRAM)
				) ||
				/* lightsaber that isn't lit ;) */
				(is_lightsaber(otmp) && !otmp->lamplit) ||
				/* houchou not thrown */
				(otmp->oartifact == ART_HOUCHOU) ||
				/* WAC -- or using a pole at short range... */
				(is_pole(otmp) &&
					otmp->otyp != AKLYS && 
					otmp->otyp != FORCE_PIKE && 
					otmp->oartifact != ART_WEBWEAVER_S_CROOK && 
					otmp->oartifact != ART_HEARTCLEAVER && 
					otmp->oartifact != ART_SOL_VALTIVA && 
					otmp->oartifact != ART_PEN_OF_THE_VOID
				)) {
			    /* then do only 1-2 points of damage */
			    if (u.umonnum == PM_SHADE && otmp->otyp != SILVER_ARROW)
				dmg = 0;
				else if(otmp->oartifact == ART_LIECLEAVER)
				dmg = 2*(rnd(12) + rnd(10) + otmp->spe);
				else
				dmg = rnd(2);
			} else dmg += dmgval(otmp, &youmonst, 0);
			
			if (otmp && (objects[otmp->otyp].oc_material == SILVER || arti_silvered(otmp)) &&
				!(u.sealsActive&SEAL_EDEN) &&
				maybe_polyd(hates_silver(youmonst.data), Race_if(PM_VAMPIRE)) &&
				!(is_lightsaber(otmp) && otmp->lamplit && otmp->oartifact != ART_ANNULUS)
			) {
			    pline("The silver sears your flesh!");
			}
			
			if (otmp->opoisoned){
				Sprintf(buf, "%s %s",
					s_suffix(Monnam(mtmp)), mpoisons_subj(mtmp, mattk));
				poisoned(buf, A_CON, mdat->mname, 30, otmp->opoisoned);
			}
			if (dmg <= 0) dmg = 1;
			if (!(otmp->oartifact &&
				artifact_hit(mtmp, &youmonst, otmp, &dmg,dieroll)))
			     hitmsg(mtmp, mattk);
			if (!dmg) break;
			if (u.mh > 1 && u.mh > ((u.uac>=0) ? dmg : dmg+AC_VALUE(u.uac)) &&
				   objects[otmp->otyp].oc_material == IRON &&
					(u.umonnum==PM_BLACK_PUDDING
					|| u.umonnum==PM_BROWN_PUDDING)) {
			    /* This redundancy necessary because you have to
			     * take the damage _before_ being cloned.
			     */
			    if (u.uac < 0) dmg += AC_VALUE(u.uac);
			    if (dmg < 1) dmg = 1;
			    if (dmg > 1) exercise(A_STR, FALSE);
			    u.mh -= dmg;
			    flags.botl = 1;
			    dmg = 0;
			    if(cloneu())
			    You("divide as %s hits you!",mon_nam(mtmp));
			}
			urustm(mtmp, otmp);
		    } else if (mattk->aatyp != AT_TUCH || dmg != 0 ||
				mtmp != u.ustuck)
			hitmsg(mtmp, mattk);
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_STAR:
			if(maybe_polyd(hates_silver(youmonst.data), Race_if(PM_VAMPIRE))) {
            	dmg += rnd(20);
            	pline("The rapier of silver light sears your flesh!");
            }
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_SHDW:
			// dmg = d(rnd(8),rnd(5)+1);
			ptmp = A_STR;
			goto dopois;
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_CHKH:
			hitmsg(mtmp, mattk);
			dmg += u.chokhmah*5;
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_DISE:
		hitmsg(mtmp, mattk);
		if (!diseasemu(mdat)) dmg = 0;
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_CHRN:{
			long lcount = (long) rnd(100);

			switch (rn2(6)) {
				case 0:
					make_sick(Sick ? Sick/3L + 1L : (long)rn1(ACURR(A_CON),20),
					"cursed unicorn's horn", TRUE, SICK_NONVOMITABLE);
				break;
				case 1:
					make_blinded(Blinded + lcount, TRUE);
				break;
				case 2: if (!Confusion)
					You("suddenly feel %s.",
						Hallucination ? "trippy" : "confused");
					make_confused(HConfusion + lcount, TRUE);
				break;
				case 3:
					make_stunned(HStun + lcount, TRUE);
				break;
				case 4:
					(void) adjattrib(rn2(A_MAX), -1, FALSE);
				break;
				case 5: 
					(void) make_hallucinated(HHallucination + lcount, TRUE, 0L);
				break;
			}
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_FIRE:
		hitmsg(mtmp, mattk);
		if (uncancelled) {
		    pline("You're %s!", on_fire(youmonst.data, mattk));
		    if (youmonst.data == &mons[PM_STRAW_GOLEM] ||
		        youmonst.data == &mons[PM_PAPER_GOLEM] ||
		        youmonst.data == &mons[PM_SPELL_GOLEM]) {
			    You("burn up!");
				if((int) mtmp->m_lev > rn2(20))
				destroy_item(SCROLL_CLASS, AD_FIRE);
				if((int) mtmp->m_lev > rn2(20))
				destroy_item(POTION_CLASS, AD_FIRE);
				if((int) mtmp->m_lev > rn2(25))
				destroy_item(SPBOOK_CLASS, AD_FIRE);
			    /* KMH -- this is okay with unchanging */
			    rehumanize();
			    break;
		    }else if (youmonst.data == &mons[PM_MIGO_WORKER]) {
			    You("melt!");
				if((int) mtmp->m_lev > rn2(20))
				destroy_item(SCROLL_CLASS, AD_FIRE);
				if((int) mtmp->m_lev > rn2(20))
				destroy_item(POTION_CLASS, AD_FIRE);
				if((int) mtmp->m_lev > rn2(25))
				destroy_item(SPBOOK_CLASS, AD_FIRE);
			    /* KMH -- this is okay with unchanging */
			    rehumanize();
			    break;
		    } else if (Fire_resistance) {
				pline_The("fire doesn't feel hot!");
				dmg = 0;
		    } else {
				if((int) mtmp->m_lev > rn2(20))
				destroy_item(SCROLL_CLASS, AD_FIRE);
				if((int) mtmp->m_lev > rn2(20))
				destroy_item(POTION_CLASS, AD_FIRE);
				if((int) mtmp->m_lev > rn2(25))
				destroy_item(SPBOOK_CLASS, AD_FIRE);
			}
		    burn_away_slime();
		} else dmg = 0;
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_COLD:
		hitmsg(mtmp, mattk);
		if (uncancelled) {
		    pline("You're covered in frost!");
		    if (Cold_resistance) {
				pline_The("frost doesn't seem cold!");
				dmg = 0;
		    } else {
				if((int) mtmp->m_lev > rn2(20))
				destroy_item(POTION_CLASS, AD_COLD);
			}
		} else dmg = 0;
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_ELEC:
		hitmsg(mtmp, mattk);
		if (uncancelled) {
		    You("get zapped!");
		    if (Shock_resistance) {
				pline_The("zap doesn't shock you!");
				dmg = 0;
		    } else {
				if((int) mtmp->m_lev > rn2(20))
				destroy_item(WAND_CLASS, AD_ELEC);
				if((int) mtmp->m_lev > rn2(20))
				destroy_item(RING_CLASS, AD_ELEC);
			}
		} else dmg = 0;
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_AXUS: {
			pline("%s reaches out with %s %s!  A corona of dancing energy surrounds the %s!",
				mon_nam(mtmp), mhis(mtmp), mbodypart(mtmp, ARM), mbodypart(mtmp, HAND));
			if(Shock_resistance) {
			    shieldeff(u.ux, u.uy);
			    You_feel("a mild tingle.");
			    ugolemeffects(AD_ELEC, dmg);
			}
			else{
				You("are jolted with energy!");
				mdamageu(mtmp, dmg);
				if (!rn2(10)) (void) destroy_item(RING_CLASS, AD_ELEC);
				if (!rn2(10)) (void) destroy_item(WAND_CLASS, AD_ELEC);
				if(!Fire_resistance){
					pline("You're %s!", on_fire(youmonst.data, mattk));
					if (youmonst.data == &mons[PM_STRAW_GOLEM] ||
						youmonst.data == &mons[PM_PAPER_GOLEM] ||
						youmonst.data == &mons[PM_SPELL_GOLEM]) {
						You("burn up!");
						destroy_item(SCROLL_CLASS, AD_FIRE);
						destroy_item(POTION_CLASS, AD_FIRE);
						destroy_item(SPBOOK_CLASS, AD_FIRE);
						/* KMH -- this is okay with unchanging */
						rehumanize();
					} else if (youmonst.data == &mons[PM_MIGO_WORKER]) {
						You("melt!");
						/* KMH -- this is okay with unchanging */
						rehumanize();
						break;
					}
					if (!rn2(4)) (void) destroy_item(POTION_CLASS, AD_FIRE);
					if (!rn2(4)) (void) destroy_item(SCROLL_CLASS, AD_FIRE);
					if (!rn2(10)) (void) destroy_item(SPBOOK_CLASS, AD_FIRE);
				}
				if(!Stunned) make_stunned((long)dmg, TRUE);
			}
			pline("%s reaches out with %s other %s!  A penumbra of shadows surrounds the %s!",
				mon_nam(mtmp), mhis(mtmp), mbodypart(mtmp, ARM), mbodypart(mtmp, HAND));
		    if(Cold_resistance) {
				shieldeff(u.ux, u.uy);
				You_feel("a mild chill.");
				ugolemeffects(AD_COLD, dmg);
		    }
			else{
			    You("are suddenly very cold!");
			    mdamageu(mtmp, dmg);
				if (!rn2(4)) (void) destroy_item(POTION_CLASS, AD_COLD);
			}
			if (!Drain_resistance) {
			    losexp("life force drain",TRUE,FALSE,FALSE);
			}
			dmg = 0;
		} break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_SLEE:
		hitmsg(mtmp, mattk);
		if (uncancelled && multi >= 0 && !rn2(7-mtmp->m_lev)) {
		    if (Sleep_resistance) break;
				fall_asleep(-rnd(10), TRUE);
		    if (Blind) You("are put to sleep!");
		    else You("are put to sleep by %s!", mon_nam(mtmp));
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_BLND:
		if (can_blnd(mtmp, &youmonst, mattk->aatyp, (struct obj*)0)) {
		    if (!Blind) pline("%s blinds you!", Monnam(mtmp));
		    make_blinded(Blinded+(long)dmg,FALSE);
		    if (!Blind) Your(vision_clears);
		}
		dmg = 0;
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_DRST:
		ptmp = A_STR;
		goto dopois;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_DRDX:
		ptmp = A_DEX;
		goto dopois;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_DRCO:
		ptmp = A_CON;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
dopois:
		hitmsg(mtmp, mattk);
		if(mdat == &mons[PM_DEMOGORGON]){
			poisoned(buf, ptmp, mdat->mname, 6, 0);
			losexp("rotting alive",TRUE,FALSE,FALSE);
		}
		else if (uncancelled && !rn2(8)) {
		    Sprintf(buf, "%s %s",
			    s_suffix(Monnam(mtmp)), mpoisons_subj(mtmp, mattk));
		    poisoned(buf, ptmp, mdat->mname, 30, 0);
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
		case AD_POSN:
			hitmsg(mtmp, mattk);
			if(Poison_resistance){ 
				pline("You're covered in poison, but it seems harmless.");
				dmg = 0;
			} else {
				pline("You're covered in poison! You don't feel so good!");
				exercise(A_STR, FALSE);
				exercise(A_INT, FALSE);
				exercise(A_CON, FALSE);
				exercise(A_DEX, FALSE);
		    }
		break;
////////////////////////////////////////////////////////////////////////////////////////////////////
		case AD_WISD:
			if (uncancelled && !rn2(8)) {
				pline("%s assaults your sanity!", Monnam(mtmp));
				if(u.sealsActive&SEAL_HUGINN_MUNINN){
					unbind(SEAL_HUGINN_MUNINN,TRUE);
				} else {
					(void) adjattrib(A_WIS, -dmg/10+1, FALSE);
					forget_levels(10);	/* lose memory of 10% of levels*/
					forget_objects(10);	/* lose memory of 10% of objects*/
					exercise(A_WIS, FALSE);
				}
			}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////
		case AD_DRIN:
		if((mtmp->data == &mons[PM_MIGO_PHILOSOPHER] || mtmp->data == &mons[PM_MIGO_QUEEN])
			&& rn2(100) >= 4){	return 0;
		}
		hitmsg(mtmp, mattk);
		if (defends(AD_DRIN, uwep) || !has_head(youmonst.data) || uclockwork) {
		    You("don't seem harmed.");
		    /* Not clear what to do for green slimes */
		    break;
		}
		if (u_slip_free(mtmp,mattk)) break;

		if (uarmh && (uarmh->otyp == PLASTEEL_HELM || uarmh->otyp == CRYSTAL_HELM || rn2(8))) {
		    /* not body_part(HEAD) */
		    Your("helmet blocks the attack to your head.");
		    break;
		}
		if (Half_physical_damage) dmg = (dmg+1) / 2;
		mdamageu(mtmp, dmg);
		/* brain removal messages were here, but they should have been below */
		
		/* adjattrib gives dunce cap message when appropriate */
		//(void) adjattrib(A_INT, -rnd(2), FALSE);
		//forget_levels(25);	/* lose memory of 25% of levels */
		//forget_objects(25);	/* lose memory of 25% of objects */
		//exercise(A_WIS, FALSE);
		if(u.sealsActive&SEAL_HUGINN_MUNINN){
			unbind(SEAL_HUGINN_MUNINN,TRUE);
		} else {
			(void) adjattrib(A_INT, -dmg, FALSE);
			while(dmg--){
				forget_levels(10);	/* lose memory of 10% of levels per point lost*/
				forget_objects(10);	/* lose memory of 10% of objects per point lost*/
				exercise(A_WIS, FALSE);
			}
		}
		//begin moved brain removal messages
		if (!uarmh || uarmh->otyp != DUNCE_CAP) {
			if(mtmp->data == &mons[PM_MIGO_PHILOSOPHER]||mtmp->data == &mons[PM_MIGO_QUEEN])
				 Your("brain has been removed!");
		    else Your("brain is eaten!");
		    /* No such thing as mindless players... */
		    if (ABASE(A_INT) <= 3) {
				int lifesaved = 0;
				struct obj *wore_amulet = uamul;
				
				while(1) {
				    /* avoid looping on "die(y/n)?" */
				    if (lifesaved && (discover || wizard)) {
						if (wore_amulet && !uamul) {
						    /* used up AMULET_OF_LIFE_SAVING; still
						       subject to dying from brainlessness */
						    wore_amulet = 0;
						} else {
						    /* explicitly chose not to die;
						       arbitrarily boost intelligence */
						    ABASE(A_INT) = ATTRMIN(A_INT) + 2;
						    You_feel("like a scarecrow.");
						    break;
						}
					}
					if (lifesaved)
					pline("Unfortunately your brain is still gone.");
					else
					Your("last thought fades away.");
					killer = "brainlessness";
					killer_format = KILLED_BY;
					done(DIED);
					lifesaved++;
				}
		    }
		}
		//end moved brain removal msgs
		if(mtmp->data == &mons[PM_MIGO_PHILOSOPHER]||mtmp->data == &mons[PM_MIGO_QUEEN])
			return 3;
		break;
/////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_PLYS:
		hitmsg(mtmp, mattk);
		if (uncancelled && multi >= 0 && !rn2(3)) {
		    if (Free_action) {
				You("momentarily stiffen.");            
		    } else {
				if (Blind) You("are frozen!");
				else You("are frozen by %s!", mon_nam(mtmp));
				nomovemsg = 0;	/* default: "you can move again" */
				nomul(-dmg, "paralyzed by a monster");
				exercise(A_DEX, FALSE);
		    }
		}
		break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		case AD_TCKL:
		hitmsg(mtmp, mattk);
		if (armuncancel && multi >= 0) {
		    if (Free_action)
				You_feel("horrible fingers probing your flesh!  But you are not distracted!");
		    else {
				if (Blind) You("are mercilessly tickled!");
				else You("are mercilessly tickled by %s!", mon_nam(mtmp));
				nomovemsg = 0;	/* default: "you can move again" */
				if(!rn2(3)) nomul(-rnd(10), "being tickled to death");
				else nomul(-1, "being tickled to death");
				exercise(A_DEX, FALSE);
				exercise(A_CON, FALSE);
				if(uwep && !welded(uwep)){
					if( d(1,100) - min(ACURR(A_DEX),ACURR(A_CON)) > 0){
						if(u.twoweap){
							You("lose hold of your weapons.");
							u.twoweap = FALSE;
							optr = uswapwep;
							setuswapwep((struct obj *)0);
							freeinv(optr);
							(void) dropy(optr);
						}
						else You("lose hold of your weapon.");
						optr = uwep;
						uwepgone();
						freeinv(optr);
						(void) dropy(optr);
					 }
					 else{
						You("keep a tight grip on your %s!",u.twoweap?"weapons":"weapon");
					 }
				}
			}
		}
		else if(uarmc){
			You_feel("horrible fingers tug at your cloak.");
			if(d(1,100) - ACURR(A_DEX) > 0){
				pline("The horrible fingers pull off your cloak!");
				optr = uarmc;
				if (donning(optr)) cancel_don();
				(void) Cloak_off();
				freeinv(optr);
				(void) dropy(optr);
			}
		}
		else if(uwep && uwep->oartifact == ART_TENSA_ZANGETSU){
			You_feel("horrible fingers tug at your shihakusho.");
		}
		else{
			if(uswapwep && !u.twoweap){
				if(d(1,100) - ACURR(A_DEX) > 0){
					You_feel("your %s being stealthily drawn out of your belt.",doname(uswapwep));
					optr = uswapwep;
					setuswapwep((struct obj *)0);
					freeinv(optr);
					(void) mpickobj(mtmp,optr);
				}
			} else if(uarm){
				You_feel("horrible fingers work at your armor.");
				if(d(1,100) - ACURR(A_DEX) > 0 && d(1,4)==4){
					pline("The horrible fingers unfasten your armor!");
					optr = uarm;
					if (donning(optr)) cancel_don();
					(void) Armor_gone();
					freeinv(optr);
					(void) dropy(optr);
				}
			}
		}
		break;
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_VAMP:
			hitmsg(mtmp, mattk);
			/* if vampire biting (and also a pet) */
			if (mattk->aatyp == AT_BITE &&
				has_blood(youmonst.data) && !uclockwork
			) {
			   Your("blood is being drained!");
			   /* Get 1/20th of full corpse value
				* Therefore 4 bites == 1 drink
				*/
				if (mtmp->mtame && !mtmp->isminion)
					EDOG(mtmp)->hungrytime += ((int)((youmonst.data)->cnutrit / 20) + 1);
				if(mtmp->data == &mons[PM_BLOOD_BLOATER]){
					mtmp->mhp += dmg;
					if (mtmp->mhpmax < mtmp->mhp) mtmp->mhpmax = mtmp->mhp;
				}
			}
			if (!mtmp->mcan && !rn2(3) && !Drain_resistance) {
				if(!has_blood(youmonst.data) || uclockwork) pline("%s feeds on you life force!",Monnam(mtmp));
			    losexp("life force drain",TRUE,FALSE,FALSE);
				if(mtmp->data == &mons[PM_BLOOD_BLOATER]){
					(void)split_mon(mtmp, 0);
				}
			}
		break;
	    case AD_DRLI:
			hitmsg(mtmp, mattk);
			if (uncancelled  && !rn2(3) && !Drain_resistance) {
			    losexp("life force drain",TRUE,FALSE,FALSE);
				if(mdat == &mons[PM_METROID] || mdat == &mons[PM_ALPHA_METROID] || mdat == &mons[PM_GAMMA_METROID] 
					|| mdat == &mons[PM_ZETA_METROID] || mdat == &mons[PM_OMEGA_METROID] 
					|| mdat == &mons[PM_METROID_QUEEN]){
					mtmp->mhpmax += d(1,4);
					mtmp->mhp += d(1,6);
					if(mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;
					if(mtmp->mtame){
						EDOG(mtmp)->hungrytime += 100;  //400/4 = human nut/4
					}
				}
			}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_LEGS:
		{ register long side = rn2(2) ? RIGHT_SIDE : LEFT_SIDE;
		  const char *sidestr = (side == RIGHT_SIDE) ? "right" : "left";

		/* This case is too obvious to ignore, but Nethack is not in
		 * general very good at considering height--most short monsters
		 * still _can_ attack you when you're flying or mounted.
		 * [FIXME: why can't a flying attacker overcome this?]
		 */
		  if (
#ifdef STEED
			u.usteed ||
#endif
				    Levitation || Flying) {
		    pline("%s tries to reach your %s %s!", Monnam(mtmp),
			  sidestr, body_part(LEG));
		    dmg = 0;
		  } else if (mtmp->mcan) {
		    pline("%s nuzzles against your %s %s!", Monnam(mtmp),
			  sidestr, body_part(LEG));
		    dmg = 0;
		  } else {
		    if (uarmf) {
				static int jboots1 = 0;
				if (!jboots1) jboots1 = find_jboots();
				if (rn2(2) && (uarmf->otyp == LOW_BOOTS ||
							 uarmf->otyp == IRON_SHOES))
					pline("%s pricks the exposed part of your %s %s!",
					Monnam(mtmp), sidestr, body_part(LEG));
				else if (uarmf->otyp != jboots1 && !rn2(5))
					pline("%s pricks through your %s boot!",
					Monnam(mtmp), sidestr);
				else {
					pline("%s scratches your %s boot!", Monnam(mtmp),
					sidestr);
					dmg = 0;
					break;
				}
		    } else pline("%s pricks your %s %s!", Monnam(mtmp),
			  sidestr, body_part(LEG));
		    set_wounded_legs(side, rnd(60-ACURR(A_DEX)));
		    exercise(A_STR, FALSE);
		    exercise(A_DEX, FALSE);
		  }
		  break;
		}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	    case AD_STON:	/* cockatrice */
		hitmsg(mtmp, mattk);
		if(!rn2(3)) {
		    if (mtmp->mcan) {
			if (flags.soundok)
			    You_hear("a cough from %s!", mon_nam(mtmp));
		    } else {
			if (flags.soundok)
			    You_hear("%s hissing!", s_suffix(mon_nam(mtmp)));
			if(!rn2(10) ||
			    (flags.moonphase == NEW_MOON && !have_lizard())) {
 do_stone:
			    if (!Stoned && !Stone_resistance
				    && !(poly_when_stoned(youmonst.data) &&
					polymon(PM_STONE_GOLEM))) {
				Stoned = 5;
				delayed_killer = mtmp->data->mname;
				if (mtmp->data->geno & G_UNIQ) {
				    if (!type_is_pname(mtmp->data)) {
					static char kbuf[BUFSZ];

					/* "the" buffer may be reallocated */
					Strcpy(kbuf, the(delayed_killer));
					delayed_killer = kbuf;
				    }
				    killer_format = KILLED_BY;
				} else killer_format = KILLED_BY_AN;
				return(1);
				/* You("turn to stone..."); */
				/* done_in_by(mtmp); */
			    }
			}
		    }
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_STCK:
		hitmsg(mtmp, mattk);
		if (uncancelled && !u.ustuck && !sticks(youmonst.data)){
			if(Upolyd && u.umonnum == PM_TOVE) You("are much too slithy to get stuck!");
			else u.ustuck = mtmp;
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_WRAP:
		if ((!mtmp->mcan || u.ustuck == mtmp) && !sticks(youmonst.data)) {
		    if (!u.ustuck && !rn2(10)) {
			if (u_slip_free(mtmp, mattk)) {
			    dmg = 0;
			} else {
			    pline("%s swings itself around you!",
				  Monnam(mtmp));
				if(Upolyd && u.umonnum == PM_TOVE) pline("Fortunately, you are much too slithy to grab!");
				else u.ustuck = mtmp;
			}
		    } else if(u.ustuck == mtmp) {
				if (is_pool(mtmp->mx,mtmp->my) && !Swimming
					&& !Amphibious) {
					boolean moat =
					(levl[mtmp->mx][mtmp->my].typ != POOL) &&
					(levl[mtmp->mx][mtmp->my].typ != WATER) &&
					!Is_medusa_level(&u.uz) &&
					!Is_waterlevel(&u.uz);
					
					water_damage(invent, FALSE, FALSE, FALSE);
					
					pline("%s drowns you...", Monnam(mtmp));
					killer_format = KILLED_BY_AN;
					Sprintf(buf, "%s by %s",
						moat ? "moat" : "pool of water",
						an(mtmp->data->mname));
					killer = buf;
					done(DROWNING);
				} else if(mattk->aatyp == AT_HUGS){
					pline("%s is crushing you.", Monnam(mtmp));
				} else {
					dmg = 0;
					pline("%s is wrapped tight around you.", Monnam(mtmp));
				}
		    } else {
				dmg = 0;
				if(flags.verbose)
					pline("%s brushes against your %s.", Monnam(mtmp),
					   body_part(LEG));
		    }
		} else dmg = 0;
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_WERE:
		hitmsg(mtmp, mattk);
		if (uncancelled && !rn2(4) && u.ulycn == NON_PM &&
			!Protection_from_shape_changers &&
			!spec_ability2(uwep, SPFX2_NOWERE)) {
		    You_feel("feverish.");
		    exercise(A_CON, FALSE);
		    u.ulycn = monsndx(mdat);
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_SGLD:
		hitmsg(mtmp, mattk);
		if (youmonst.data->mlet == mdat->mlet || u.sealsActive&SEAL_ANDROMALIUS) break;
		if(!mtmp->mcan) stealgold(mtmp);
		break;

///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_SITM:	/* for now these are the same */
	    case AD_SEDU: //duplicated bellow, in the gaze section.
		{
			static int engagering4 = 0;
			boolean engring = FALSE;
			if (!engagering4) engagering4 = find_engagement_ring();
			if ( (uleft && uleft->otyp == engagering4) || (uright && uright->otyp == engagering4)) engring = TRUE;
			if(u.sealsActive&SEAL_ANDROMALIUS) break;
//pline("test string!");
			if(mdat == &mons[PM_DEMOGORGON]){
				buf[0] = '\0';
				steal(mtmp, buf,FALSE);
				m_dowear(mtmp, FALSE);
				return 1;
			}
			if ( (mdat == &mons[PM_FIERNA] || mdat == &mons[PM_PALE_NIGHT]) && rnd(20)<15) return 1;
			if (is_animal(mtmp->data)) {
				hitmsg(mtmp, mattk);
					if (mtmp->mcan) break;
				/* Continue below */
			} else if (dmgtype(youmonst.data, AD_SEDU)
#ifdef SEDUCE
				|| dmgtype(youmonst.data, AD_SSEX) || dmgtype(youmonst.data, AD_LSEX)
#endif
			  ) {
				pline("%s %s.", Monnam(mtmp), mtmp->minvent ?
			    "brags about the goods some dungeon explorer provided" :
			    "makes some remarks about how difficult theft is lately");
				if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
				return 3;
			} else if (mtmp->mcan || engring) {
			    if (!Blind) {
				pline("%s tries to %s you, but you seem %s.",
				    Adjmonnam(mtmp, "plain"),
				    flags.female ? "charm" : "seduce",
				    flags.female ? "unaffected" : "uninterested");
			    }
			    if(rn2(3)) {
					if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
					return 3;
			    }
			    break;
			}
			buf[0] = '\0';
		switch (steal(mtmp, buf, FALSE)) {
			  case -1:
				return 2;
			  case 0:
	 		  break;
			  default:
				if (!is_animal(mtmp->data) && !tele_restrict(mtmp))
				    (void) rloc(mtmp, FALSE);
				if (is_animal(mtmp->data) && *buf) {
				    if (canseemon(mtmp))
					pline("%s tries to %s away with %s.",
					      Monnam(mtmp),
					      locomotion(mtmp->data, "run"),
					      buf);
				}
				monflee(mtmp, 0, FALSE, FALSE);
				return 3;
			}
			m_dowear(mtmp, FALSE);
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
#ifdef SEDUCE
		case AD_SSEX:{
			static int engagering1 = 0;
			int mndx = monsndx(mdat);
			if (!engagering1) engagering1 = find_engagement_ring();
			if ( (uleft && uleft->otyp == engagering1) || (uright && uright->otyp == engagering1))
		break;
			if(mndx == PM_MOTHER_LILITH)
				if(mtmp->mcan) mtmp->mcan=0;
			if(mtmp->data == &mons[PM_MOTHER_LILITH] && !rn2(4)) return 1;
			if(mndx == PM_MOTHER_LILITH && could_seduce(mtmp, &youmonst, mattk) == 1){
				if(!rn2(4)) return 1;
				if(dolilithseduce(mtmp)) return 3;
			}
			else if(mndx == PM_BELIAL && could_seduce(mtmp, &youmonst, mattk) == 1){
				if(!rn2(4)) return 1;
				if(dobelialseduce(mtmp)) return 3;
			}
//			else if(mndx == PM_SHAMI_AMOURAE && could_seduce(mtmp, &youmonst, mattk) == 1 
//				&& !mtmp->mcan){
//				if(dosflseduce(mtmp)) return 3;
//			}
//			else if(mndx == PM_THE_DREAMER && could_seduce(mtmp, &youmonst, mattk) == 2 
//				&& !mtmp->mcan){
//			}
//			else if(mndx == PM_XINIVRAE && could_seduce(mtmp, &youmonst, mattk) == 2 
//				&& !mtmp->mcan){
//			}
//			else if(mndx == PM_LYNKHAB && could_seduce(mtmp, &youmonst, mattk) 
//				&& !mtmp->mcan){
//			}
			else if(mndx == PM_MALCANTHET && could_seduce(mtmp, &youmonst, mattk) 
				&& !mtmp->mcan){
				if(domlcseduce(mtmp)) return 3;
			}
			else if(mndx == PM_GRAZ_ZT && could_seduce(mtmp, &youmonst, mattk) 
				&& !mtmp->mcan){
				if(dograzseduce(mtmp)) return 3;
			}
			else if(mndx == PM_PALE_NIGHT && could_seduce(mtmp, &youmonst, mattk) 
				&& !mtmp->mcan){
				dopaleseduce(mtmp);
				return 3;
			}
			else if(mndx == PM_AVATAR_OF_LOLTH && could_seduce(mtmp, &youmonst, mattk) 
				&& !mtmp->mcan){
				dololthseduce(mtmp);
				return 3;
			}
			else if(could_seduce(mtmp, &youmonst, mattk) == 1 
				&& !mtmp->mcan){
			    if (doseduce(mtmp))
					return 3;
			}
		}
		break;
#endif
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_TENT:
			u.ustuck = mtmp; //can steal stickage from other monsters.
#ifdef SEDUCE
			dotent(mtmp,dmg);
#endif
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_SAMU:
		hitmsg(mtmp, mattk);
		/* when the Wiz hits, 1/20 steals the amulet */
		if (u.uhave.amulet ||
		     u.uhave.bell || u.uhave.book || u.uhave.menorah
		    ) /* carrying the Quest Artifact */
		    if (!rn2(20)) stealamulet(mtmp);
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_SQUE:
		hitmsg(mtmp, mattk);
		/* when the Nemeses hits, 1/10 steals the something special */
		if (u.uhave.amulet ||
		     u.uhave.bell || u.uhave.book || u.uhave.menorah
		     || u.uhave.questart) /* carrying the Quest Artifact */
		    if (!rn2(10)) stealquestart(mtmp);
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_TLPT:
		hitmsg(mtmp, mattk);
		if (uncancelled) {
		    if(flags.verbose)
				Your("position suddenly seems very uncertain!");
		    tele();
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_ABDC:
		hitmsg(mtmp, mattk);
		if ( armuncancel || (mtmp->data==&mons[PM_PALE_NIGHT] && !rn2(3)) ) {
			(void) safe_teleds(FALSE);
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_LVLT:
		hitmsg(mtmp, mattk);
		if (uncancelled) {
		    if(flags.verbose) {
				if (Teleport_control) {
					You("feel like you could have lost some potential.");
				} else {
					You("suddenly feel like you've lost some potential.");
				}
		    }
			level_tele();
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_WEEP: /*Weeping Angel's attack*/
		hitmsg(mtmp, mattk);
		if (uncancelled) {
			if(!u.uevent.udemigod){
				if (Teleport_control || Drain_resistance) {
					if(flags.verbose) You("feel like you could have lost some potential.");
				} else {
					int potentialLost = 0;
					level_tele();
					You("suddenly feel like you've lost some potential.");
					potentialLost = min(abs(u.uz.dlevel - u.utolev.dlevel),u.ulevel-1)/2 + 1;
					for(potentialLost; potentialLost>0; potentialLost--) losexp("loss of potential",FALSE,TRUE,TRUE); /*not verbose, force drain, drain exp also*/
					dmg = 0;
					return 3; /*You teleported, monster should stop attacking.*/
				}
			}
			else{/*The angels will try to drain you dry during the endgame*/
				if (Drain_resistance) {
					if(flags.verbose) You("feel like you could have lost some potential.");
				} else {
					You("suddenly feel like you've lost some potential.");
					losexp("loss of potential",FALSE,TRUE,TRUE); /*not verbose, force drain, drain exp also*/
				}
			}
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
		case AD_POLY:
			hitmsg(mtmp, mattk);
			if(!Unchanging && uncancelled){
				Your("DNA has been altered!");
				polyself(FALSE);
				if(rnd(100)<15){
					Your("cellular structure degenerates.");
					losexp("cellular degeneration",TRUE,TRUE,FALSE);
				}
			}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_RUST:
			hitmsg(mtmp, mattk);
			if (mtmp->mcan) break;
			if (u.umonnum == PM_IRON_GOLEM) {
				You("rust!");
			/* KMH -- this is okay with unchanging */
				rehumanize();
			break;
		}
		hurtarmor(AD_RUST);
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_CORR:
		hitmsg(mtmp, mattk);
		if (mtmp->mcan) break;
		hurtarmor(AD_CORR);
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_DCAY:
		hitmsg(mtmp, mattk);
		if (mtmp->mcan) break;
		if (u.umonnum == PM_WOOD_GOLEM ||
			u.umonnum == PM_GROVE_GUARDIAN ||
			u.umonnum == PM_LIVING_LECTURN ||
		    u.umonnum == PM_LEATHER_GOLEM) {
			You("rot!");
			/* KMH -- this is okay with unchanging */
			rehumanize();
			break;
		}
		hurtarmor(AD_DCAY);
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_HEAL:
//		/* a cancelled nurse was just an ordinary monster */
//		if (mtmp->mcan) {
//		    hitmsg(mtmp, mattk);
//		    break;
//		}
		if(mtmp->mpeaceful//!uwep
#ifdef TOURIST
		   && !uarmu
#endif
		   && !uarm && !uarmh && !uarms && !uarmg && !uarmc && !uarmf) {
		    boolean goaway = FALSE;
		    pline("%s hits!  (I hope you don't mind.)", Monnam(mtmp));
		    if (Upolyd) {
			u.mh += rnd(7);
			if (!rn2(7)) {
			    /* no upper limit necessary; effect is temporary */
			    u.mhmax++;
			    if (!rn2(13)) goaway = TRUE;
			}
			if (u.mh > u.mhmax) u.mh = u.mhmax;
		    } else {
			u.uhp += rnd(7);
			if (!rn2(7)) {
			    /* hard upper limit via nurse care: 25 * ulevel */
			    if (u.uhpmax < 5 * u.ulevel + d(2 * u.ulevel, 10))
				u.uhpmax++;
			    if (!rn2(13)) goaway = TRUE;
			}
			if (u.uhp > u.uhpmax) u.uhp = u.uhpmax;
		    }
		    if (!rn2(3)) exercise(A_STR, TRUE);
		    if (!rn2(3)) exercise(A_CON, TRUE);
		    if (Sick) make_sick(0L, (char *) 0, FALSE, SICK_ALL);
		    flags.botl = 1;
		    if (goaway) {
			mongone(mtmp);
			return 2;
		    } else if (!rn2(33)) {
			if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
			monflee(mtmp, d(3, 6), TRUE, FALSE);
			return 3;
		    }
		    dmg = 0;
		} else {
		    if (mtmp->mpeaceful) {
			if (flags.soundok && !(moves % 5))
		      if(Role_if(PM_HEALER)) verbalize("Doc, I can't help you unless you cooperate.");
			  else pline("%s changes %s mind.", Monnam(mtmp), mhis(mtmp));
			dmg = 0;
			monflee(mtmp, d(3, 6), TRUE, FALSE);
		    } else hitmsg(mtmp, mattk);
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_CURS:
		hitmsg(mtmp, mattk);
		if(!night() && mdat == &mons[PM_GREMLIN]) break;
		if((!mtmp->mcan && !rn2(10)) || mdat == &mons[PM_PALE_NIGHT]) {
		    if (flags.soundok) {
			if (Blind) You_hear("laughter.");
			else       pline("%s chuckles.", Monnam(mtmp));
		    }
		    if (u.umonnum == PM_CLAY_GOLEM || u.umonnum == PM_SPELL_GOLEM) {
			pline("Some writing vanishes from your head!");
			/* KMH -- this is okay with unchanging */
			rehumanize();
			break;
		    }
		    attrcurse();
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_STUN:
		hitmsg(mtmp, mattk);
		if(!mtmp->mcan && !rn2(4)) {
		    make_stunned(HStun + dmg, TRUE);
		    dmg /= 2;
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_ACID:
		hitmsg(mtmp, mattk);
		if (!mtmp->mcan && !rn2(3))
		    if (Acid_resistance) {
			pline("You're covered in acid, but it seems harmless.");
			dmg = 0;
		    } else {
			pline("You're covered in acid! It burns!");
			exercise(A_STR, FALSE);
		    }
		else		dmg = 0;
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_SLOW:
		hitmsg(mtmp, mattk);
		if (uncancelled && HFast &&
					!defends(AD_SLOW, uwep) && !rn2(4))
		    u_slow_down();
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_DREN:
		hitmsg(mtmp, mattk);
		if ((uncancelled && !rn2(4)) || mdat == &mons[PM_PALE_NIGHT])
		    drain_en(dmg);
		dmg = 0;
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_CONF:
		hitmsg(mtmp, mattk);
		if(!mtmp->mcan && !rn2(4) && !mtmp->mspec_used) {
		    mtmp->mspec_used = mtmp->mspec_used + (dmg + rn2(6));
		    if(Confusion)
			 You("are getting even more confused.");
		    else You("are getting confused.");
		    make_confused(HConfusion + dmg, FALSE);
		}
		dmg = 0;
		break;
///////////////////////////////////////////////////////////////////////////////////////////
		case AD_TELE:
			if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
			return 3;
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_DETH:
		pline("%s reaches out with its deadly touch.", Monnam(mtmp));
		if (is_undead(youmonst.data) || u.sealsActive&SEAL_OSE) {
		    /* Still does normal damage */
		    pline("Was that the touch of death?");
		    break;
		}
		switch (rn2(20)) {
		case 19: case 18: case 17:
		    if (!Antimagic) {
			killer_format = KILLED_BY_AN;
			killer = "touch of death";
			done(DIED);
			dmg = 0;
			break;
		    } /* else FALLTHRU */
		default: /* case 16: ... case 5: */
		    You_feel("your life force draining away...");
		    permdmg = 1;	/* actual damage done below */
		    break;
		case 4: case 3: case 2: case 1: case 0:
		    if (Antimagic) shieldeff(u.ux, u.uy);
		    pline("Lucky for you, it didn't work!");
		    dmg = 0;
		    break;
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_PEST:
		pline("%s reaches out, and you feel fever and chills.",
			Monnam(mtmp));
		(void) diseasemu(mdat); /* plus the normal damage */
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_FAMN:
		pline("%s reaches out, and your body shrivels.",
			Monnam(mtmp));
		exercise(A_CON, FALSE);
		if (!is_fainted()) morehungry(rn1(40,40));
		/* plus the normal damage */
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_SLIM:    
		hitmsg(mtmp, mattk);
		if (!uncancelled) break;
		if (flaming(youmonst.data)) {
		    pline_The("slime burns away!");
		    dmg = 0;
		} else if (Unchanging ||
				youmonst.data == &mons[PM_GREEN_SLIME] ||
				youmonst.data == &mons[PM_FLUX_SLIME]) {
		    You("are unaffected.");
		    dmg = 0;
		} else if (!Slimed) {
		    You("don't feel very well.");
		    Slimed = 10L;
		    flags.botl = 1;
		    killer_format = KILLED_BY_AN;
		    delayed_killer = mtmp->data->mname;
		} else
		    pline("Yuck!");
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_WEBS:{
			struct trap *ttmp2 = maketrap(u.ux, u.uy, WEB);
			if (ttmp2) {
				pline_The("webbing sticks to you. You're caught!");
				dotrap(ttmp2, NOWEBMSG);
#ifdef STEED
				if (u.usteed && u.utrap) {
				/* you, not steed, are trapped */
				dismount_steed(DISMOUNT_FELL);
				}
#endif
			}
		}break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
		hitmsg(mtmp, mattk);
		/* uncancelled is sufficient enough; please
		   don't make this attack less frequent */
		if (uncancelled) {
		    struct obj *obj = some_armor(&youmonst);

		    if (drain_item(obj)) {
				Your("%s less effective.", aobjnam(obj, "seem"));
		    }
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
		case AD_WET:
		hitmsg(mtmp, mattk);
		water_damage(invent, FALSE, FALSE, FALSE);
		break;
///////////////////////////////////////////////////////////////////////////////////////////
		case AD_SHRD:{
		    struct obj *obj = some_armor(&youmonst);
			hitmsg(mtmp, mattk);
			if(obj && mtmp->data != &mons[PM_RETRIEVER]){
			 int i = 0;
			 switch (mattk->aatyp) {
				case AT_LNCK:
				case AT_BITE:
					pline("%s's teeth catch on your armor!", Monnam(mtmp));
				break;
				case AT_STNG:
					pline("%s's stinger catches on your armor!", Monnam(mtmp));
				break;
				case AT_BUTT:
					pline("%s's horn catches on your armor!", Monnam(mtmp));
				break;
				case AT_TENT:
					pline("%s's tentacles catch on your armor!", Monnam(mtmp));
				break;
				case AT_EXPL:
				case AT_BOOM:
					pline("%s's shrapnel hits your armor!", Monnam(mtmp));
				break;
				case AT_WEAP:
					pline("%s's weapon strikes your armor!", Monnam(mtmp));
				break;
				default:
					pline("%s's claws catch on your armor!", Monnam(mtmp));
				break;
			 }
			 i = 1;
			 if(mtmp->data==&mons[PM_DEMOGORGON]) i += rnd(4);
			 for(i; i>0; i--){
				if(obj->spe > -1*objects[(obj)->otyp].a_ac){
					damage_item(obj);
					if(!i) Your("%s less effective.", aobjnam(obj, "seem"));
				}
				else if(!obj->oartifact || (mtmp->data==&mons[PM_DEMOGORGON] && rn2(10)) ){
				 claws_destroy_arm(obj);
				 i = 0;
				}
			 }
			}
			else if(mtmp->data==&mons[PM_DEMOGORGON]){
				if (noncorporeal(youmonst.data) || amorphous(youmonst.data)) {
					pline("%s tries to rip %s apart!",
				      mon_nam(mtmp), "you");
					dmg = (int)(1.5 * dmg);
				}
				dmg = 2 * (Upolyd ? u.mh : u.uhp)
					  + 400; //FATAL_DAMAGE_MODIFIER;
				pline("%s rips %s apart!",
				      mon_nam(mtmp), "you");
			}
		} break;
///////////////////////////////////////////////////////////////////////////////////////////
		case AD_STTP:{
		    struct obj *obj = some_armor(&youmonst);
			hitmsg(mtmp, mattk);
			teleport_arm(obj);
		} break;
///////////////////////////////////////////////////////////////////////////////////////////
		case AD_VORP:
		hitmsg(mtmp, mattk);
		if(!rn2(20) && mtmp->data != &mons[PM_LUCIFER]){//Lucifer nerf.
			if (!has_head(youmonst.data)) {
				pline("Somehow, %s misses you wildly.",
				      mon_nam(mtmp));
				dmg = 0;
			}
			if (noncorporeal(youmonst.data) || amorphous(youmonst.data)) {
				pline("%s slices through your %s.",
				      mon_nam(mtmp), body_part(NECK));
			}
			else {
				if(!uarmh){
					dmg = 2 * (Upolyd ? u.mh : u.uhp)
						+ 400; //FATAL_DAMAGE_MODIFIER;
					pline("%s decapitates %s!",
						mon_nam(mtmp), "you");
				}
				else{
					dmg *= 2;
					if(!uarmh->oartifact || (mtmp->data==&mons[PM_DEMOGORGON]) ){
						claws_destroy_arm(uarmh);
					}
				}
			}
		 }
		break;
///////////////////////////////////////////////////////////////////////////////////////////
		case AD_SUCK:
			if (noncorporeal(youmonst.data) || amorphous(youmonst.data)) dmg = 0;
			else{
				if( has_head(youmonst.data) && !uarmh && !rn2(20) && 
					((!Upolyd && u.uhp < .1*u.uhpmax) || (Upolyd && u.mh < .1*u.mhmax))
				){
					dmg = 2 * (Upolyd ? u.mh : u.uhp)
						  + 400; //FATAL_DAMAGE_MODIFIER;
					pline("%s sucks your %s off!",
					      Monnam(mtmp), body_part(HEAD));
				}
				else{
					You_feel("%s trying to suck your extremities off!",mon_nam(mtmp));
					if(youmonst.movement > -1 * NORMAL_SPEED) youmonst.movement -= NORMAL_SPEED / 2;
					if(!rn2(10)){
						Your("%s twist from the suction!", makeplural(body_part(LEG)));
					    set_wounded_legs(RIGHT_SIDE, rnd(60-ACURR(A_DEX)));
					    set_wounded_legs(LEFT_SIDE, rnd(60-ACURR(A_DEX)));
					    exercise(A_STR, FALSE);
					    exercise(A_DEX, FALSE);
					}
					if(uwep && !rn2(6)){
						You_feel("%s pull on your weapon!",mon_nam(mtmp));
						if( d(1,130) > ACURR(A_STR)){
							Your("weapon is sucked out of your grasp!");
							optr = uwep;
							uwepgone();
							freeinv(optr);
							(void) mpickobj(mtmp,optr);
						}
						else{
							You("keep a tight grip on your weapon!");
						}
					}
					static int bboots1 = 0;
					if (!bboots1) bboots1 = find_bboots();
					if(!rn2(10) && uarmf && uarmf->otyp != bboots1){
						Your("boots are sucked off!");
						optr = uarmf;
						if (donning(optr)) cancel_don();
						(void) Boots_off();
						freeinv(optr);
						(void) mpickobj(mtmp,optr);
					}
					if(!rn2(6) && uarmg && !uwep){
						You_feel("%s pull on your gloves!",mon_nam(mtmp));
						if( d(1,40) > ACURR(A_STR)){
							Your("gloves are sucked off!");
							optr = uarmg;
							if (donning(optr)) cancel_don();
							(void) Gloves_off();
							freeinv(optr);
							(void) mpickobj(mtmp,optr);
						}
						else You("keep your %s closed.", makeplural(body_part(HAND)));
					}
					if(!rn2(8) && uarms){
						You_feel("%s pull on your shield!",mon_nam(mtmp));
						if( d(1,150) > ACURR(A_STR)){
							Your("shield is sucked out of your grasp!");
							optr = uarms;
							if (donning(optr)) cancel_don();
							Shield_off();
							freeinv(optr);
							(void) mpickobj(mtmp,optr);
						 }
						 else{
							You("keep a tight grip on your shield!");
						 }
					}
					if(!rn2(4) && uarmh){
						Your("helmet is sucked off!");
						optr = uarmh;
						if (donning(optr)) cancel_don();
						(void) Helmet_off();
						freeinv(optr);
						(void) mpickobj(mtmp,optr);
					}
				}
			}
		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_MALK:
		hitmsg(mtmp, mattk);
		if (!rn2(4) && /*!u.ustuck &&*/ !sticks(youmonst.data))
			if(Upolyd && u.umonnum == PM_TOVE) You("are much too slithy to grab!");
			else u.ustuck = mtmp;
		if (uncancelled) {
		    You("get zapped!");
		    if (Shock_resistance) {
				pline_The("zap doesn't shock you!");
		    }
			else{
				dmg += d(1,4);
				if((int) mtmp->m_lev > rn2(30))
					destroy_item(WAND_CLASS, AD_ELEC);
				if((int) mtmp->m_lev > rn2(30))
					destroy_item(RING_CLASS, AD_ELEC);
			}
		}
 		break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_UVUU:{
		int wdmg = (int)(dmg/6) + 1;
		hitmsg(mtmp, mattk);
	    Sprintf(buf, "%s %s",
		    s_suffix(Monnam(mtmp)), mpoisons_subj(mtmp, mattk));
		poisoned(buf, A_CON, mdat->mname, 60, 0);
		if(Poison_resistance) wdmg -= ACURR(A_CON)/2;
		if(wdmg > 0){
		if(u.sealsActive&SEAL_HUGINN_MUNINN){
			unbind(SEAL_HUGINN_MUNINN,TRUE);
		} else {
			while( ABASE(A_WIS) > ATTRMIN(A_WIS) && wdmg > 0){
				wdmg--;
				(void) adjattrib(A_WIS, -1, TRUE);
				forget_levels(10);	/* lose memory of 10% of levels per point lost*/
				forget_objects(10);	/* lose memory of 10% of objects per point lost*/
				exercise(A_WIS, FALSE);
			}
			if(AMAX(A_WIS) > ATTRMIN(A_WIS) && 
				ABASE(A_WIS) < AMAX(A_WIS)/2) AMAX(A_WIS) -= 1; //permanently drain wisdom
			if(wdmg){
				boolean chg;
				chg = make_hallucinated(HHallucination + (long)(wdmg*5),FALSE,0L);
			}
		}
		}
		drain_en( (int)(dmg/2) );
		if(!rn2(20)){
			if (!has_head(youmonst.data)) {
				dmg *= 2;
			}
			else if (noncorporeal(youmonst.data) || amorphous(youmonst.data)) {
				pline("%s passes through your %s.",
				      mon_nam(mtmp), body_part(HEAD));
				dmg *= 2;
			}
			else {
				if(!uarmh){
					dmg = 2 * (Upolyd ? u.mh : u.uhp)
						+ 400; //FATAL_DAMAGE_MODIFIER;
					pline("%s smashes your %s!",
						mon_nam(mtmp), body_part(HEAD));
				} else pline("%s's headspike hits your %s!",
						mon_nam(mtmp), xname(uarmh) );
			}
		 }
 		}break;
///////////////////////////////////////////////////////////////////////////////////////////
	    case AD_ILUR: //Non-engulfed hit = damage and stick to you
		hitmsg(mtmp, mattk);
		if (!u.ustuck && !sticks(youmonst.data)){
			if(Upolyd && u.umonnum == PM_TOVE) You("are much too slithy to get stuck!");
			else u.ustuck = mtmp;
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AD_STDY:{
			if(mtmp->data == &mons[PM_ORC_CAPTAIN] || mtmp->data == &mons[PM_BOLG]) 
				pline("%s curses and urges %s followers on.", Monnam(mtmp), mhis(mtmp));
			else pline("%s studies you carefully.", Monnam(mtmp));
			u.ustdy = max(dmg,u.ustdy);
			dmg = 0;
		}break;
///////////////////////////////////////////////////////////////////////////////////////////
		case AD_FRWK:{
			int x,y,i = rn2(3)+2;
			for(i; i > 0; i--){
				x = rn2(3)-1;
				y = rn2(3)-1;
				explode(mtmp->mx+x, mtmp->my+y, 8, dmg, -1, rn2(7));		//-1 is unspecified source. 8 is physical
			}
			dmg=0;
		} break;
///////////////////////////////////////////////////////////////////////////////////////////
/*		case AD_VMSL:	//vorlon missile.  triple damage
			mondead(mtmp);
			explode(mtmp->mx, mtmp->my, 5, 2*dmg, -1, EXPL_WET);		//-1 is unspecified source. 5 is electrical
			explode(mtmp->mx, mtmp->my, 4, 1000, -1, EXPL_MAGICAL);	//-1 is unspecified source. 4 is disintegration
			explode(mtmp->mx, mtmp->my, 8, dmg, -1, EXPL_DARK);		//-1 is unspecified source. 8 is physical
			dmg=0;		//damage was done by explode

			if (mtmp->mhp > 0) return(0);
			return(2);	// it dies 
		break;
*/
///////////////////////////////////////////////////////////////////////////////////////////
	    default:	dmg = 0;
			break;
	}
	if(u.uhp < 1) done_in_by(mtmp);

	if(dmg && u.ustdy){
		dmg += u.ustdy;
		u.ustdy -= 1;
	}
/*	Negative armor class reduces damage done instead of fully protecting
 *	against hits.  The bladed shadows of the black web pierce your armor.
 */
	if (dmg && u.uac < 0 && mattk->adtyp != AD_SHDW && mattk->adtyp != AD_STAR) {
		dmg += AC_VALUE(u.uac);
		if (dmg < 1) dmg = 1;
	}

	if(dmg) {
	    if (Half_physical_damage
					/* Mitre of Holiness */
		|| (Role_if(PM_PRIEST) && uarmh && is_quest_artifact(uarmh) &&
		    (is_undead(mtmp->data) || is_demon(mtmp->data))))
		dmg = (dmg+1) / 2;

	    if (permdmg) {	/* Death's life force drain */
		int lowerlimit, *hpmax_p;
		/*
		 * Apply some of the damage to permanent hit points:
		 *	polymorphed	    100% against poly'd hpmax
		 *	hpmax > 25*lvl	    100% against normal hpmax
		 *	hpmax > 10*lvl	50..100%
		 *	hpmax >  5*lvl	25..75%
		 *	otherwise	 0..50%
		 * Never reduces hpmax below 1 hit point per level.
		 */
		permdmg = rn2(dmg / 2 + 1);
		if (Upolyd || u.uhpmax > 25 * u.ulevel) permdmg = dmg;
		else if (u.uhpmax > 10 * u.ulevel) permdmg += dmg / 2;
		else if (u.uhpmax > 5 * u.ulevel) permdmg += dmg / 4;

		if (Upolyd) {
		    hpmax_p = &u.mhmax;
		    /* [can't use youmonst.m_lev] */
		    lowerlimit = min((int)youmonst.data->mlevel, u.ulevel);
		} else {
		    hpmax_p = &u.uhpmax;
		    lowerlimit = u.ulevel;
		}
		if (*hpmax_p - permdmg > lowerlimit)
		    *hpmax_p -= permdmg;
		else if (*hpmax_p > lowerlimit)
		    *hpmax_p = lowerlimit;
		else	/* unlikely... */
		    ;	/* already at or below minimum threshold; do nothing */
		flags.botl = 1;
	    }

	    mdamageu(mtmp, dmg);
	}

	if (dmg)
	    res = passiveum(olduasmon, mtmp, mattk);
	else
	    res = 1;
	stop_occupation();
	return res;
}

#endif /* OVL1 */
#ifdef OVLB

STATIC_OVL int
gulpmu(mtmp, mattk)	/* monster swallows you, or damage if u.uswallow */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	struct trap *t = t_at(u.ux, u.uy);
	int	tmp = d((int)mattk->damn, (int)mattk->damd);
	int	tim_tmp;
	struct obj *otmp2;
	int	i;

	if (!u.uswallow) {	/* swallows you */
		if (youmonst.data->msize >= MZ_HUGE) return(0);
		if ((t && ((t->ttyp == PIT) || (t->ttyp == SPIKED_PIT))) &&
		    boulder_at(u.ux, u.uy))
			return(0);

		if (Punished) unplacebc();	/* ball&chain go away */
		remove_monster(mtmp->mx, mtmp->my);
		mtmp->mtrapped = 0;		/* no longer on old trap */
		place_monster(mtmp, u.ux, u.uy);
		u.ustuck = mtmp;
		newsym(mtmp->mx,mtmp->my);
#ifdef STEED
		if (is_animal(mtmp->data) && u.usteed) {
			char buf[BUFSZ];
			/* Too many quirks presently if hero and steed
			 * are swallowed. Pretend purple worms don't
			 * like horses for now :-)
			 */
			Strcpy(buf, mon_nam(u.usteed));
			pline ("%s lunges forward and plucks you off %s!",
				Monnam(mtmp), buf);
			dismount_steed(DISMOUNT_ENGULFED);
		} else
#endif
		pline("%s engulfs you!", Monnam(mtmp));
		stop_occupation();
		reset_occupations();	/* behave as if you had moved */

		if (u.utrap) {
			You("are released from the %s!",
				u.utraptype==TT_WEB ? "web" : "trap");
			u.utrap = 0;
		}

		i = number_leashed();
		if (i > 0) {
		    const char *s = (i > 1) ? "leashes" : "leash";
		    pline_The("%s %s loose.", s, vtense(s, "snap"));
		    unleash_all();
		}

		if (touch_petrifies(youmonst.data) && !resists_ston(mtmp)) {
			minstapetrify(mtmp, TRUE);
			if (mtmp->mhp > 0) return 0;
			else return 2;
		}

		display_nhwindow(WIN_MESSAGE, FALSE);
		vision_recalc(2);	/* hero can't see anything */
		u.uswallow = 1;
		/* u.uswldtim always set > 1 */
		tim_tmp = 25 - (int)mtmp->m_lev;
		if (tim_tmp > 0) tim_tmp = rnd(tim_tmp) / 2;
		else if (tim_tmp < 0) tim_tmp = -(rnd(-tim_tmp) / 2);
		tim_tmp += -u.uac + 10;
		u.uswldtim = (unsigned)((tim_tmp < 2) ? 2 : tim_tmp);
		swallowed(1);
		for (otmp2 = invent; otmp2; otmp2 = otmp2->nobj)
		    (void) snuff_lit(otmp2);
	}

	if (mtmp != u.ustuck) return(0);
	if (u.uswldtim > 0) u.uswldtim -= 1;

	switch(mattk->adtyp) {

		case AD_DGST:
			if (mtmp->data == &mons[PM_METROID_QUEEN] && !Drain_resistance) {
			    losexp("life force drain",TRUE,FALSE,FALSE);
				mtmp->mhpmax += d(1,4);
				mtmp->mhp += d(1,6);
				if(mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;
				if(mtmp->mtame){
					EDOG(mtmp)->hungrytime += 100;  //400/4 = human nut/4
				}
			}
		    if (Slow_digestion) {
			/* Messages are handled below */
			u.uswldtim = 0;
			tmp = 0;
		    } else if (u.uswldtim == 0) {
			pline("%s totally digests you!", Monnam(mtmp));
			tmp = u.uhp+300;//fatal damage modifier
			if (Half_physical_damage) tmp *= 2; /* sorry */
		    } else {
			pline("%s%s digests you!", Monnam(mtmp),
			      (u.uswldtim == 2) ? " thoroughly" :
			      (u.uswldtim == 1) ? " utterly" : "");
			exercise(A_STR, FALSE);
		    }
		break;
		case AD_PHYS:
		    if (mtmp->data == &mons[PM_FOG_CLOUD]) {
			You("are laden with moisture and %s",
			    flaming(youmonst.data) ? "are smoldering out!" :
			    Breathless ? "find it mildly uncomfortable." :
			    amphibious(youmonst.data) ? "feel comforted." :
			    "can barely breathe!");
			/* NB: Amphibious includes Breathless */
			if (Amphibious && !flaming(youmonst.data)) tmp = 0;
			else exercise(A_CON, FALSE);
		    } else if (mtmp->data == &mons[PM_DREADBLOSSOM_SWARM]) {
				You("are sliced by the whirling stems!");
				exercise(A_DEX, FALSE);
			} else {
				You("are pummeled with debris!");
				exercise(A_STR, FALSE);
		    }
		break;
		case AD_ACID:
		    if (Acid_resistance) {
			You("are covered with a seemingly harmless goo.");
			tmp = 0;
		    } else {
		      if (Hallucination) pline("Ouch!  You've been slimed!");
		      else You("are covered in slime!  It burns!");
		      exercise(A_STR, FALSE);
		    }
		break;
		case AD_BLND:
		    if (can_blnd(mtmp, &youmonst, mattk->aatyp, (struct obj*)0)) {
			if(!Blind) {
			    You_cant("see in here!");
			    make_blinded((long)tmp,FALSE);
			    if (!Blind) Your(vision_clears);
			} else
			    /* keep him blind until disgorged */
			    make_blinded(Blinded+1,FALSE);
		    }
		    tmp = 0;
		break;
		case AD_ELEC:
		    if(!mtmp->mcan && rn2(2)) {
			pline_The("air around you crackles with electricity.");
			if (Shock_resistance) {
				shieldeff(u.ux, u.uy);
				You("seem unhurt.");
				ugolemeffects(AD_ELEC,tmp);
				tmp = 0;
			}
		    } else tmp = 0;
		break;
		case AD_COLD:
		    if(!mtmp->mcan && rn2(2)) {
			if (Cold_resistance) {
				shieldeff(u.ux, u.uy);
				You_feel("mildly chilly.");
				ugolemeffects(AD_COLD,tmp);
				tmp = 0;
			} else You("are freezing to death!");
		    } else tmp = 0;
		break;
		case AD_FIRE:
		    if(!mtmp->mcan && rn2(2)) {
			  if (Fire_resistance) {
				shieldeff(u.ux, u.uy);
				You_feel("mildly hot.");
				ugolemeffects(AD_FIRE,tmp);
				tmp = 0;
			  } else You("are burning to a crisp!");
			burn_away_slime();
		    } else tmp = 0;
		break;
		case AD_DISE:
		    if (!diseasemu(mtmp->data)) tmp = 0;
		break;
		case AD_LETHE:
			pline("The waters of the Lethe wash over you!");
			if(u.sealsActive&SEAL_HUGINN_MUNINN){
				unbind(SEAL_HUGINN_MUNINN,TRUE);
			} else {
				(void) adjattrib(A_INT, -1, FALSE);
				forget_levels(25);	/* lose memory of 25% of levels*/
				forget_objects(25);	/* lose memory of 25% of objects*/
				water_damage(invent, FALSE, FALSE, TRUE);

				exercise(A_WIS, FALSE);
			}
			if (ABASE(A_INT) <= 3) {
				int lifesaved = 0;
				struct obj *wore_amulet = uamul;
				
				while(1) {
				    /* avoid looping on "die(y/n)?" */
				    if (lifesaved && (discover || wizard)) {
						if (wore_amulet && !uamul) {
						    /* used up AMULET_OF_LIFE_SAVING; still
						       subject to dying from brainlessness */
						    wore_amulet = 0;
						} else {
						    /* explicitly chose not to die;
						       arbitrarily boost intelligence */
						    ABASE(A_INT) = ATTRMIN(A_INT) + 2;
						    You_feel("like a scarecrow.");
						    break;
						}
					}
					if (lifesaved)
						pline("Unfortunately your mind is still gone.");
					else
						Your("last thought drifts away.");
					killer = "memmory loss";
					killer_format = KILLED_BY;
					done(DIED);
					lifesaved++;
				}
		    }
		break;
		case AD_ILUR:{
			int perc;
			if(has_blood(youmonst.data)){
				pline("A mist of blood is torn from your %s and swept into the cloud!", body_part(BODY_SKIN));
				tmp *= 2;
				if(u.sealsActive&SEAL_HUGINN_MUNINN){
					unbind(SEAL_HUGINN_MUNINN,TRUE);
				} else {
					if(Upolyd) perc = (u.mhmax - (u.mh - tmp))*10/u.mhmax;
					else perc = (u.uhpmax - (u.uhp - tmp))*10/u.uhpmax;
					if(perc > 10) perc = 10;
					if(perc >= 5) forget_levels(perc);
					if(perc > 1) forget_objects(perc);
				}
			} else if(!is_anhydrous(youmonst.data)){
				pline("A mist of water is drawn through your %s and swept into the cloud!", body_part(BODY_SKIN));
				if(u.sealsActive&SEAL_HUGINN_MUNINN){
					unbind(SEAL_HUGINN_MUNINN,TRUE);
				} else {
					if(Upolyd) perc = (u.mhmax - u.mh - tmp)*10/u.mhmax;
					else perc = (u.uhpmax - u.uhp - tmp)*10/u.uhpmax;
					if(perc > 10) perc = 10;
					if(perc >= 5) forget_levels(perc);
					if(perc > 1) forget_objects(perc);
				}
			} else {
				if (u.umonnum == PM_IRON_GOLEM) {
					You("are laden with moisture and rust away!");
				/* KMH -- this is okay with unchanging */
					rehumanize();
					mtmp->mhp = 1;
					expels(mtmp, mtmp->data,FALSE);
					pline("Rusting your iron body took a severe toll on the cloud!");
				}
				else {
					You("are laden with moisture and %s",
						flaming(youmonst.data) ? "are smoldering out!" :
						Breathless ? "find it mildly uncomfortable." :
						amphibious(youmonst.data) ? "feel comforted." :
						"can barely breathe!");
					/* NB: Amphibious includes Breathless */
					if (Amphibious && !flaming(youmonst.data)) tmp = 0;
				}
			}
			hurtarmor(AD_RUST);
			// No way to tell if rusting occurs!
			// if(mtmp->mhp > 1){
				// mtmp->mhp -= 10;
				// if(mtmp->mhp < 1) mtmp->mhp = 1;
			// }
			if(Half_physical_damage) tmp = (tmp*2)-1; /*reverse formula, cancel half damage*/
		}break;
		case AD_STDY:{
			You("are studied from all directions!");
			u.ustdy = max(tmp,u.ustdy);
			tmp = 0;
		}break;
		case AD_DRST:
			if (nonliving(youmonst.data) || Breathless){
				tmp = 0;
				break;
			}
			if (!Blind)
				make_blinded(1L, FALSE);
			if (!Poison_resistance) {
				pline("%s is burning your %s!", Something, makeplural(body_part(LUNG)));
				You("cough and spit blood!");
			} else {
				You("cough!");
				tmp = 0;
			}
		break;
		default:
		    tmp = 0;
		break;
	}

	if (Half_physical_damage) tmp = (tmp+1) / 2;

	mdamageu(mtmp, tmp);
	if (tmp) stop_occupation();

	if (touch_petrifies(youmonst.data) && !resists_ston(mtmp)) {
	    pline("%s very hurriedly %s you!", Monnam(mtmp),
		  is_animal(mtmp->data)? "regurgitates" : "expels");
	    expels(mtmp, mtmp->data, FALSE);
	} else if (!u.uswldtim || youmonst.data->msize >= MZ_HUGE) {
	    You("get %s!", is_animal(mtmp->data)? "regurgitated" : "expelled");
	    if (flags.verbose && (is_animal(mtmp->data) ||
		    (dmgtype(mtmp->data, AD_DGST) && Slow_digestion)))
		pline("Obviously %s doesn't like your taste.", mon_nam(mtmp));
	    expels(mtmp, mtmp->data, FALSE);
	}
	return(1);
}

STATIC_OVL int
explmu(mtmp, mattk, ufound)	/* monster explodes in your face */
register struct monst *mtmp;
register struct attack  *mattk;
boolean ufound;
{
//	explode(mtmp->mx, mtmp->my, 0, 5, 0, EXPL_NOXIOUS); //explode(x, y, type, dam, olet, expltype)
    if (mtmp->mcan) return(0);

    if (!ufound)
	pline("%s explodes at a spot in %s!",
	    canseemon(mtmp) ? Monnam(mtmp) : "It",
	    levl[mtmp->mux][mtmp->muy].typ == WATER
		? "empty water" : "thin air");
    else {
	register int tmp = d((int)mattk->damn, (int)mattk->damd);
	register boolean not_affected = defends((int)mattk->adtyp, uwep);

	hitmsg(mtmp, mattk);

	switch (mattk->adtyp) {
		case AD_FNEX:
			mondead(mtmp);
			if(mtmp->data==&mons[PM_SWAMP_FERN_SPORE]) explode(mtmp->mx, mtmp->my, 9, tmp, MON_EXPLODE, EXPL_MAGICAL);
			else explode(mtmp->mx, mtmp->my, 7, tmp, MON_EXPLODE, EXPL_NOXIOUS);
			return 2;
		break;
		
	    case AD_COLD:
			not_affected |= Cold_resistance;
			goto common;
	    case AD_FIRE:
			not_affected |= Fire_resistance;
			goto common;
	    case AD_ELEC:
			not_affected |= Shock_resistance;
common:

		if (!not_affected) {
		    if (ACURR(A_DEX) > rnd(20)) {
			You("duck some of the blast.");
			tmp = (tmp+1) / 2;
		    } else {
		        if (flags.verbose) You("get blasted!");
		    }
		    if (mattk->adtyp == AD_FIRE) burn_away_slime();
		    if (Half_physical_damage) tmp = (tmp+1) / 2;
		    mdamageu(mtmp, tmp);
		}
		break;

	    case AD_BLND:
		not_affected = resists_blnd(&youmonst);
		if (!not_affected) {
		    /* sometimes you're affected even if it's invisible */
		    if (mon_visible(mtmp) || (rnd(tmp /= 2) > u.ulevel)) {
			You("are blinded by a blast of light!");
			make_blinded((long)tmp, FALSE);
			if (!Blind) Your("%s", vision_clears);
		    } else if (flags.verbose)
			You("get the impression it was not terribly bright.");
		}
		break;

	    case AD_HALU:
		not_affected |= NoLightBlind ||
			(u.umonnum == PM_BLACK_LIGHT ||
			 u.umonnum == PM_VIOLET_FUNGUS ||
			 dmgtype(youmonst.data, AD_STUN));
		if (!not_affected) {
		    boolean chg;
		    if (!Hallucination)
			You("are caught in a blast of kaleidoscopic light!");
		    chg = make_hallucinated(HHallucination + (long)tmp,FALSE,0L);
		    You("%s.", chg ? "are freaked out" : "seem unaffected");
		}
		break;

	    default:
		break;
	}
	if (not_affected) {
	    You("seem unaffected by it.");
	    ugolemeffects((int)mattk->adtyp, tmp);
	}
    }
    mondead(mtmp);
    wake_nearto_noisy(mtmp->mx, mtmp->my, 7*7);
    if (mtmp->mhp > 0) return(0);
    return(2);	/* it dies */
}

int
gazemu(mtmp, mattk)	/* monster gazes at you */
	register struct monst *mtmp;
	register struct attack  *mattk;
{
	int succeeded = 0;
	int attack_type = mattk->adtyp;
	struct	permonst *mdat = mtmp->data;
	char buf[BUFSZ];
	if(mtmp->data->maligntyp < 0 && u.uz.dnum == law_dnum && on_level(&illregrd_level,&u.uz)) return 0;
	if(ward_at(u.ux,u.uy) == HAMSA) return 0;
	if(mattk->adtyp == AD_RGAZ){
		attack_type = gazeattacks[rn2(SIZE(gazeattacks))];	//flat random member of gazeattacks
	}
	else if(mattk->adtyp == AD_RETR){
		attack_type = elementalgaze[rn2(SIZE(elementalgaze))];	//flat random member of elementalgaze
	}
	switch(attack_type) {
//ifdef SEDUCE
		    case AD_SSEX:{

				static int engagering2 = 0;
				if (!engagering2) engagering2 = find_engagement_ring();
				if ( (uleft && uleft->otyp == engagering2) || (uright && uright->otyp == engagering2))
			break;
				if(could_seduce(mtmp, &youmonst, mattk) == 1
					&& !mtmp->mcan
					&& !Blind && !is_blind(mtmp) && canseemon(mtmp)
					&& 	distu(mtmp->mx,mtmp->my) == 1) 
						if (doseduce(mtmp))
								return 3;
			}break;
			case AD_SITM: //duplicate the whole nine yards, just cause
			{
				static int engagering5 = 0;
				boolean engring = FALSE;
				if (!engagering5) engagering5 = find_engagement_ring();
				if ( (uleft && uleft->otyp == engagering5) || (uright && uright->otyp == engagering5)) engring = TRUE;
				if(u.sealsActive&SEAL_ANDROMALIUS) break;
				if(distu(mtmp->mx,mtmp->my) > 1 ||
					mtmp->mcan ||
					Blind ||
					is_blind(mtmp) ||
					!canseemon(mtmp) 
				) return 0;//fail
				//else
				if(mdat == &mons[PM_DEMOGORGON]){
					buf[0] = '\0';
					steal(mtmp, buf,FALSE);
					m_dowear(mtmp, FALSE);
					return 1;
				}
				if ( (mdat == &mons[PM_FIERNA] || mdat == &mons[PM_PALE_NIGHT]) && rnd(20)<15) return 1;
				if (is_animal(mtmp->data)) {
					hitmsg(mtmp, mattk);
						if (mtmp->mcan) break;
					/* Continue below */
				} else if (dmgtype(youmonst.data, AD_SEDU)
#ifdef SEDUCE
					|| dmgtype(youmonst.data, AD_SSEX) || dmgtype(youmonst.data, AD_LSEX)
#endif
				  ) {
					pline("%s %s.", Monnam(mtmp), mtmp->minvent ?
				    "brags about the goods some dungeon explorer provided" :
				    "makes some remarks about how difficult theft is lately");
					if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
					return 3;
				} else if (mtmp->mcan || engring) {
				    if (!Blind) {
					pline("%s tries to %s you, but you seem %s.",
					    Adjmonnam(mtmp, "plain"),
					    flags.female ? "charm" : "seduce",
					    flags.female ? "unaffected" : "uninterested");
				    }
				    if(rn2(3)) {
						if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
						return 3;
				    }
				    break;
				}
				buf[0] = '\0';
				switch (steal(mtmp, buf,FALSE)) {
				  case -1:
					return 2;
				  case 0:
		 		  break;
				  default:
					if (!is_animal(mtmp->data) && !tele_restrict(mtmp))
					    (void) rloc(mtmp, FALSE);
					if (is_animal(mtmp->data) && *buf) {
					    if (canseemon(mtmp))
						pline("%s tries to %s away with %s.",
						      Monnam(mtmp),
						      locomotion(mtmp->data, "run"),
						      buf);
					}
					monflee(mtmp, 0, FALSE, FALSE);
					return 3;
				}
				m_dowear(mtmp, FALSE);
			}
			break;
			case AD_DEAD:
			   if(!Blind && !is_blind(mtmp) && canseemon(mtmp)){
				pline("Oh no, %s's using the gaze of death!", mhe(mtmp));
				if (nonliving(youmonst.data) || is_demon(youmonst.data)) {
				    You("seem no deader than before.");
				} else if (!Antimagic && !(u.sealsActive&SEAL_OSE)) {
				    if (Hallucination) {
						You("have an out of body experience.");
				    } else {
						killer_format = KILLED_BY_AN;
						killer = "gaze of death";
						succeeded = 1;
						done(DIED);
				    }
				} else {
				    if (Antimagic) shieldeff(u.ux, u.uy);
				    pline("Lucky for you, it didn't work!");
				}
			   }
			break;
		case AD_CNCL:
			if(!is_blind(mtmp) && couldsee(mtmp->mx, mtmp->my)){
				pline("Your magic fades.");
				(void) cancel_monst(&youmonst, mksobj(SPE_CANCELLATION, FALSE, FALSE), FALSE, TRUE, FALSE,!rn2(4) ? rnd(mtmp->m_lev) : 0);
				succeeded=1;
			}
		break;
	    case AD_PLYS:
		if(mtmp->data == &mons[PM_DEMOGORGON]){
		  if(!Blind && !is_blind(mtmp) && canseemon(mtmp)){
			if((!Free_action || !rn2(2)) && (!Sleep_resistance || rn2(4))){
				You("meet the gaze of Aameul, left head of Demogorgon!");
				You("are mesmerized!", mon_nam(mtmp));
				nomovemsg = 0;	/* default: "you can move again" */
				if(!Free_action && !Sleep_resistance) nomul(-rn1(5,2), "mesmerized by Aameul");
				else if(!Free_action || !Sleep_resistance) nomul(-1, "mesmerized by Aameul");
				else youmonst.movement -= 6;
				exercise(A_DEX, FALSE);
				succeeded=1;
			}
			else You("avoid the gaze of the left head of Demogorgon!");
		   }
		 //else succeeded = 0
		}
		else if (!mtmp->mcan && multi >= 0 && !rn2(3) && !is_blind(mtmp) && canseemon(mtmp)) {
		    if (Blind || Free_action ) {
				You("momentarily stiffen.");
				succeeded=0;
		    }
			else{
				You("are mesmerized by %s!", mon_nam(mtmp));
				nomovemsg = 0;	/* default: "you can move again" */
				nomul(-rnd(10), "mesmerized by a monster");
				exercise(A_DEX, FALSE);
				succeeded=1;
		    }
		}
		break;
		case AD_DRLI:
			if(mtmp->data == &mons[PM_DEMOGORGON]){
			  if(!Blind && !is_blind(mtmp) && canseemon(mtmp)){
				if(!Drain_resistance || !rn2(3)){
					You("meet the gaze of Hethradiah, right head of Demogorgon!");
					You("feel a primal darkness fall upon your soul!");
					losexp("primal darkness",FALSE,!rn2(3),FALSE);
					losexp("primal darkness",FALSE,!rn2(3),FALSE);
					losexp("primal darkness",TRUE,TRUE,FALSE);
					if(u.sealsActive&SEAL_HUGINN_MUNINN){
						unbind(SEAL_HUGINN_MUNINN,TRUE);
					} else {
						forget_levels(13);
						forget_objects(13);
					}
					succeeded=1;
				}
				else You("avoid the gaze of the right head of Demogorgon!");
			   }
			  //else succeeded = 0
			}
			else if (!mtmp->mcan && !rn2(3) && !Drain_resistance 
						&& !is_blind(mtmp) && canseemon(mtmp)) {
				You("feel your life force whither before the gaze of %s!", mon_nam(mtmp));
			    losexp("life force drain",TRUE,FALSE,FALSE);
//				forget_levels(10);
//				forget_objects(10);
				succeeded=1;
			}
		break;
		case AD_ENCH:
			if(mtmp->data == &mons[PM_DEMOGORGON]){
			 if(!Blind && !is_blind(mtmp) && canseemon(mtmp)){
				struct obj *obj = some_armor(&youmonst);
			    if (drain_item(obj)) {
					Your("%s less effective.", aobjnam(obj, "seem"));
			    }
				succeeded=1;
			 }
			} else if (!mtmp->mcan && !rn2(4) && !is_blind(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
				struct obj *obj = some_armor(&youmonst);
			    if (drain_item(obj)) {
					Your("%s less effective.", aobjnam(obj, "seem"));
			    }
				succeeded=1;
			}
		break;
	    case AD_STON:
		if (mtmp->mcan || is_blind(mtmp)) {
		    if (!canseemon(mtmp)) break;	/* silently */
		    pline("%s %s.", Monnam(mtmp),
			  (mtmp->data == &mons[PM_MEDUSA] && mtmp->mcan) ?
				"doesn't look all that ugly" :
				"gazes ineffectually");
		    break;
		}
		if (Reflecting && couldsee(mtmp->mx, mtmp->my) &&
			mtmp->data == &mons[PM_MEDUSA]) {
		    /* hero has line of sight to Medusa and she's not blind */
		    boolean useeit = canseemon(mtmp);

		    if (useeit)
			(void) ureflects("%s gaze is reflected by your %s.",
					 s_suffix(Monnam(mtmp)));
		    if (mon_reflects(mtmp, !useeit ? (char *)0 :
				     "The gaze is reflected away by %s %s!"))
			break;
		    if (!m_canseeu(mtmp)) { /* probably you're invisible */
			if (useeit)
			    pline(
		      "%s doesn't seem to notice that %s gaze was reflected.",
				  Monnam(mtmp), mhis(mtmp));
			break;
		    }
		    if (useeit)
			pline("%s is turned to stone!", Monnam(mtmp));
		    stoned = TRUE;
		    killed(mtmp);

		    if (mtmp->mhp > 0) break;
		    return 2;
		}
		if(mtmp->data == &mons[PM_PALE_NIGHT]){
			if(canseemon(mtmp)) pline("%s parts her shroud!", Monnam(mtmp));
			if (mtmp->mcan || Stone_resistance) {
				if (!canseemon(mtmp)) break;	/* silently */
				pline("%s %s.", Monnam(mtmp),
					"doesn't look all that ugly");
				break;
			}
			if (Reflecting && couldsee(mtmp->mx, mtmp->my)) {
				boolean useeit = canseemon(mtmp);
				if (useeit)
				(void) ureflects("%s image is reflected by your %s.",
						 s_suffix(Monnam(mtmp)));
				if (mon_reflects(mtmp, !useeit ? (char *)0 :
						 "The image is reflected away by %s %s!"))
				break;
				if (!m_canseeu(mtmp)) { /* probably you're invisible */
				if (useeit)
					pline(
				  "%s doesn't seem to notice that %s image was reflected.",
					  Monnam(mtmp), mhis(mtmp));
				break;
				}
				return 0;
			}
			if (couldsee(mtmp->mx, mtmp->my) &&
				!Stone_resistance) {
				You("see the truth behind the veil!", s_suffix(mon_nam(mtmp)));
				stop_occupation();
				if(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))
				break;
				You("turn to stone...");
				killer_format = KILLED_BY;
				killer = mtmp->data->mname;
				done(STONING);
			}
			return 0;
		}
		if (mtmp->data == &mons[PM_BEHOLDER] && canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) &&
		    !Stone_resistance) {
		    You("meet %s gaze.", s_suffix(mon_nam(mtmp)));
		    stop_occupation();
		    if(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM)) break;
			Stoned = 5;
			delayed_killer = "a beholder's eye of petrification.";
			killer_format = KILLED_BY;
		}
		else if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) &&
		    !Stone_resistance) {
		    You("meet %s gaze.", s_suffix(mon_nam(mtmp)));
		    stop_occupation();
		    if(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM)) break;
		    You("turn to stone...");
		    killer_format = KILLED_BY;
		    killer = mtmp->data->mname;
		    done(STONING);
		}
		break;
	    case AD_CONF:
		if(!mtmp->mcan && canseemon(mtmp) &&
		   couldsee(mtmp->mx, mtmp->my) &&
		   !is_blind(mtmp) && !mtmp->mspec_used && rn2(5)) {
		    int conf;
			if((int)mattk->damn == 0 || (int)mattk->damd == 0) 
				conf = d(3,4);
			else conf = d((int)mattk->damn, (int)mattk->damd);

		    mtmp->mspec_used = mtmp->mspec_used + (conf + rn2(6));
		    if(!Confusion)
			pline("%s gaze confuses you!",
			                  s_suffix(Monnam(mtmp)));
		    else
			You("are getting more and more confused.");
		    make_confused(HConfusion + conf, FALSE);
		    stop_occupation();
			succeeded=1;
		}
		break;
		case AD_SLOW:
		if(!mtmp->mcan && canseemon(mtmp) &&
		   couldsee(mtmp->mx, mtmp->my) &&
		   !is_blind(mtmp) && !mtmp->mspec_used && rn2(5)) {
			pline("%s stares piercingly at you!", Monnam(mtmp));
			u_slow_down();
			mtmp->mspec_used = mtmp->mspec_used + (rn2(12));
			stop_occupation();
			succeeded=1;
		}
		break;
	    case AD_STUN:
		if(!mtmp->mcan && canseemon(mtmp) &&
		   couldsee(mtmp->mx, mtmp->my) &&
		   !is_blind(mtmp) && !mtmp->mspec_used && rn2(5)) {
		    int stun = d(2,6);

		    mtmp->mspec_used = mtmp->mspec_used + (stun + rn2(6));
		    pline("%s stares piercingly at you!", Monnam(mtmp));
		    make_stunned(HStun + stun, TRUE);
		    stop_occupation();
			succeeded=1;
		}
		break;
	    case AD_BLND:
		if (!mtmp->mcan && canseemon(mtmp) && !resists_blnd(&youmonst)
			&& distu(mtmp->mx,mtmp->my) <= BOLT_LIM*BOLT_LIM) {
		    int blnd = d((int)mattk->damn, (int)mattk->damd);

		    You("are blinded by %s radiance!",
			              s_suffix(mon_nam(mtmp)));
		    make_blinded((long)blnd,FALSE);
		    stop_occupation();
		    /* not blind at this point implies you're wearing
		       the Eyes of the Overworld; make them block this
		       particular stun attack too */
		    if (!Blind) Your(vision_clears);
		    else make_stunned((long)d(1,3),TRUE);
			succeeded=1;
		}
		break;
	    case AD_SSUN:
		if (!mtmp->mcan && couldsee(mtmp->mx, mtmp->my) &&
			levl[mtmp->mx][mtmp->my].lit) {
			int dmg = d((int)mattk->damn, (int)mattk->damd);
		    pline("%s attacks you with a beam of reflected light!", Monnam(mtmp));
		    stop_occupation();
			if(canseemon(mtmp) && !resists_blnd(&youmonst)){
				int blnd = d((int)mattk->damn, (int)mattk->damd);
				You("are blinded by %s beam!",
							  s_suffix(mon_nam(mtmp)));
				make_blinded((long)blnd,FALSE);
				succeeded=1;
			}
		    if (Fire_resistance) {
				pline_The("beam doesn't feel hot!");
				dmg = 0;
		    } else if(Reflecting){
		    if (canseemon(mtmp)) ureflects("%s beam is reflected by your %s.",
					 s_suffix(Monnam(mtmp)));
				dmg = 0;
			} else {
				succeeded=1;
				if ((int) mtmp->m_lev > rn2(20))
				destroy_item(SCROLL_CLASS, AD_FIRE);
				if ((int) mtmp->m_lev > rn2(20))
				destroy_item(POTION_CLASS, AD_FIRE);
				if ((int) mtmp->m_lev > rn2(25))
				destroy_item(SPBOOK_CLASS, AD_FIRE);
			}
		    burn_away_slime();
			
		    if (dmg) mdamageu(mtmp, dmg);
		}
		break;
	    case AD_FIRE:
		if (!mtmp->mcan && canseemon(mtmp) &&
			couldsee(mtmp->mx, mtmp->my) &&
			!is_blind(mtmp) && !mtmp->mspec_used && rn2(5)) {
//		    int dmg = d(2,6);
			int dmg = d((int)mattk->damn, (int)mattk->damd);

		    pline("%s attacks you with a fiery glare!", Monnam(mtmp));
		    stop_occupation();
			succeeded=1;
		    if (Fire_resistance) {
				pline_The("fire doesn't feel hot!");
				dmg = 0;
				succeeded=0;
		    } else{
				if ((int) mtmp->m_lev > rn2(20))
				destroy_item(SCROLL_CLASS, AD_FIRE);
				if ((int) mtmp->m_lev > rn2(20))
				destroy_item(POTION_CLASS, AD_FIRE);
				if ((int) mtmp->m_lev > rn2(25))
				destroy_item(SPBOOK_CLASS, AD_FIRE);
				if (dmg) mdamageu(mtmp, dmg);
			}
		    burn_away_slime();
		}
		break;
		case AD_COLD:
		if (!mtmp->mcan && canseemon(mtmp) &&
			couldsee(mtmp->mx, mtmp->my) &&
			!is_blind(mtmp) && !mtmp->mspec_used && rn2(5)) {
//		    int dmg = d(2,6);
			int dmg = d((int)mattk->damn, (int)mattk->damd);

		    pline("%s attacks you with a icy stare!", Monnam(mtmp));
		    stop_occupation();
			succeeded=1;
		    if (Cold_resistance) {
				pline_The("frost doesn't feel cold!");
				dmg = 0;
				succeeded=0;
		    } else {
				if ((int) mtmp->m_lev > rn2(20))
					destroy_item(POTION_CLASS, AD_COLD);
			}
		    if (dmg) mdamageu(mtmp, dmg);
		}
		break;
		case AD_ELEC:
		if (!mtmp->mcan && canseemon(mtmp) &&
			couldsee(mtmp->mx, mtmp->my) &&
			!is_blind(mtmp) && !mtmp->mspec_used && rn2(5)) {
//		    int dmg = d(2,6);
			int dmg = d((int)mattk->damn, (int)mattk->damd);

		    pline("%s attacks you with a electrifying glance!", Monnam(mtmp));
		    stop_occupation();
			succeeded=1;
		    if (Shock_resistance) {
				You("aren't shocked!");
				dmg = 0;
				succeeded=0;
		    } else {
				if ((int) mtmp->m_lev > rn2(20))
				destroy_item(RING_CLASS, AD_ELEC);
				if ((int) mtmp->m_lev > rn2(20))
				destroy_item(WAND_CLASS, AD_ELEC);
			}
		    if (dmg) mdamageu(mtmp, dmg);
		}
		break;
	    case AD_BLNK:
		if (!mtmp->mcan && !u.wimage && canseemon(mtmp) &&
			couldsee(mtmp->mx, mtmp->my) &&
			!is_blind(mtmp) && !mtmp->mspec_used && rn2(5)
		) {
		    int dmg = d(1,4);
		    if (!Reflecting) {
				pline("%s reflection in your mind weakens you.", s_suffix(Monnam(mtmp)));
				stop_occupation();
	    		exercise(A_INT, TRUE);
	    		exercise(A_WIS, FALSE);
		    } else {
			if (flags.verbose)
			    /* Since this message means the player is unaffected, limit
			       its occurence to preserve flavor but avoid message spam */
			    if (!rn2(10)) pline("%s is covering its face.", Monnam(mtmp));
			dmg = 0;
		    }
		    if (dmg && !rn2(100)){
				int temparise = u.ugrave_arise;
				u.wimage = TRUE;
				u.ugrave_arise = PM_WEEPING_ANGEL;
				mdamageu(mtmp, dmg);
				/*If the player surived the gaze attack, restore the value of arise*/
				u.ugrave_arise = temparise;
			}
			mtmp->mspec_used = 10; //In practice, this will be zeroed when a new movement ration is handed out, and acts to make sure Blink can only be used once per round.
		}
		break;
		case AD_HALU:
		if (!mtmp->mcan && canseemon(mtmp) &&
			couldsee(mtmp->mx, mtmp->my) &&
			!is_blind(mtmp) && !mtmp->mspec_used && rn2(5)) {
			boolean not_affected=0;
			//int tmp = rn2(12);
			int tmp = d((int)mattk->damn, (int)mattk->damd);
		not_affected |= NoLightBlind ||
			(u.umonnum == PM_BLACK_LIGHT ||
			 u.umonnum == PM_VIOLET_FUNGUS ||
			 dmgtype(youmonst.data, AD_HALU));
		if (!not_affected) {
		    boolean chg;
		    if (!Hallucination)
				pline("%s attacks you with a kaleidoscopic gaze!", Monnam(mtmp));
		    chg = make_hallucinated(HHallucination + (long)tmp,FALSE,0L);
		    You("%s.", chg ? "are freaked out" : "seem unaffected");
			succeeded=1;
		}
		}
		break;
	    case AD_SLEE:
			if(!mtmp->mcan && canseemon(mtmp) &&
			   couldsee(mtmp->mx, mtmp->my) && !is_blind(mtmp) &&
			   multi >= 0 && !rn2(5) && !Sleep_resistance) {
	
			    fall_asleep(-rnd(10), TRUE);
			    pline("%s gaze makes you very sleepy...",
				  s_suffix(Monnam(mtmp)));
			}
		break;
		case AD_MIST:  // mi-go mist projector
			if(rn2(20) > 15){
				int i = 0;
				int n = 0;
				if(mtmp->data == &mons[PM_MIGO_SOLDIER]){
					n = rn2(4);
					if(cansee(mtmp->mx, mtmp->my)) You("see fog billow out from around %s.",mon_nam(mtmp));
					for(i=0;i<n;i++) makemon(&mons[PM_FOG_CLOUD], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_ADJACENTSTRICT);
				}
				else if(mtmp->data ==  &mons[PM_MIGO_PHILOSOPHER]){
					n = rn2(4);
					if(cansee(mtmp->mx, mtmp->my)) You("see whirling snow swirl out from around %s.",mon_nam(mtmp));
					for(i=0;i<n;i++) makemon(&mons[PM_ICE_VORTEX], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_ADJACENTSTRICT);
				}
				else if(mtmp->data == &mons[PM_MIGO_QUEEN]){
					n = rn2(2);
					if(cansee(mtmp->mx, mtmp->my)) You("see scalding steam swirl out from around %s.",mon_nam(mtmp));
					for(i=0;i<n;i++) makemon(&mons[PM_STEAM_VORTEX], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_ADJACENTSTRICT);
				}
				else{
					if(cansee(mtmp->mx, mtmp->my)) You("see fog billow out from around %s.",mon_nam(mtmp));
					makemon(&mons[PM_FOG_CLOUD], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_ADJACENTSTRICT);
				}
				return 3; // if a mi-go fires a mist projector, it can take no further actions that turn
			}
		break;
	    case AD_LUCK:
		if(!mtmp->mcan && canseemon(mtmp) && 
			couldsee(mtmp->mx, mtmp->my) && 
			!is_blind(mtmp) && !mtmp->mspec_used && rn2(5)) {
		    pline("%s glares ominously at you!", Monnam(mtmp));
		    mtmp->mspec_used = mtmp->mspec_used + d(2,6);

		    if (uwep && uwep->otyp == MIRROR && uwep->blessed) {
			pline("%s sees its own glare in your mirror.",
				Monnam(mtmp)); 
			pline("%s is cancelled!", Monnam(mtmp));
			mtmp->mcan = 1;
			monflee(mtmp, 0, FALSE, TRUE);
		    } else if((uwep && !uwep->cursed && confers_luck(uwep)) || 
			    (stone_luck(TRUE) > 0 && rn2(4))) {
			pline("Luckily, you are not affected.");
		    } else {
			You_feel("your luck running out.");
			change_luck(-1);
		    }
		    stop_occupation();
		}
		break;
	    case AD_SPOR:
		/* release a spore if the player is nearby */
		if (is_fern(mtmp->data) && !mtmp->mcan && distu(mtmp->mx, mtmp->my) <= 96 &&
			!is_fern_sprout(mtmp->data) ? !rn2(2) : !rn2(4)) {
		    coord mm;
		    mm.x = mtmp->mx; mm.y = mtmp->my;
		    enexto(&mm, mm.x, mm.y, &mons[PM_DUNGEON_FERN_SPORE]);
			if (mtmp->data == &mons[PM_DUNGEON_FERN] ||
				mtmp->data == &mons[PM_DUNGEON_FERN_SPROUT]) {
			makemon(&mons[PM_DUNGEON_FERN_SPORE], mm.x, mm.y, NO_MM_FLAGS);
		    } else if (mtmp->data == &mons[PM_SWAMP_FERN] ||
				mtmp->data == &mons[PM_SWAMP_FERN_SPROUT]) {
			makemon(&mons[PM_SWAMP_FERN_SPORE], mm.x, mm.y, NO_MM_FLAGS);
		    } else { /* currently these should not be generated */
			makemon(&mons[PM_DUNGEON_FERN_SPORE], mm.x, mm.y, NO_MM_FLAGS);
		    }
		    if (canseemon(mtmp)) pline("%s releases a spore!", Monnam(mtmp));
		}
		break;
	    case AD_WTCH:{
			//Watcher in the Water's tentacle spawn and retreat behavior
			int ltnt = 0, stnt = 0;
			struct monst *tmon;
			if (distu(mtmp->mx, mtmp->my) <= 8 && !(mtmp->mflee)){
				mtmp->mflee = 1;
				mtmp->mfleetim = 2;
			}
			for(tmon = fmon; tmon; tmon=tmon->nmon){
				if(tmon->data == &mons[PM_SWARM_OF_SNAKING_TENTACLES]) stnt++;
				else if(tmon->data == &mons[PM_LONG_SINUOUS_TENTACLE]) ltnt++;
			}
			if(stnt<6){
				makemon(&mons[PM_SWARM_OF_SNAKING_TENTACLES], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_ADJACENTSTRICT|MM_NOCOUNTBIRTH);
			} else if(ltnt<2){
				makemon(&mons[PM_LONG_SINUOUS_TENTACLE], mtmp->mx, mtmp->my, MM_ADJACENTOK|MM_ADJACENTSTRICT|MM_NOCOUNTBIRTH);
			}
		}break;
/*		case AD_VTGT:  // vorlon missile targeting vision
		{
			int i = 0;
			int maxn = mattk->damd;
			int n = 0;
			int p = mattk->damn;
			//pline("vorlon targeting gaze %d %d", p, maxn);
			if(!rn2(p)){
				n = rnd(maxn);
//				pline("%d vorlon missiles fired", n);
				for(i=0;i<n;i++) makemon(&mons[PM_VORLON_MISSILE], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				return 3; // if a vorlon fires missiles, it can take no further actions that turn
			}
			
		}	
		break;
*/
		case AD_WISD:{ //Cthulhu's attack
			if(canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my) && !mtmp->mspec_used) {
				int dmg = d((int)mattk->damn, (int)mattk->damd);
//				if(!couldsee(mtmp->mx, mtmp->my)) dmg /= 10;
				if(!dmg) break;
				pline("Blasphemous geometries assault your sanity!");
				if(u.sealsActive&SEAL_HUGINN_MUNINN){
					unbind(SEAL_HUGINN_MUNINN,TRUE);
				} else {
					while( !(ABASE(A_WIS) <= ATTRMIN(A_WIS)) && dmg > 0){
						dmg--;
						(void) adjattrib(A_WIS, -1, TRUE);
						forget_levels(10);	/* lose memory of 10% of levels per point lost*/
						forget_objects(10);	/* lose memory of 10% of objects per point lost*/
						exercise(A_WIS, FALSE);
						if(mtmp->data ==  &mons[PM_GREAT_CTHULHU] 
							&& AMAX(A_WIS) > ATTRMIN(A_WIS))		AMAX(A_WIS) -= 1; //permanently drain wisdom

					}
					if(dmg > 0){
						You("tear at yourself in horror!"); //assume always able to damage self
						mdamageu(mtmp, dmg*10);
					}
				}
				mtmp->mspec_used = 4; //In practice, this will be zeroed when a new movement ration is handed out, and acts to make sure GC can only use the gaze once per round.
				succeeded=1;
			}
		}
		break;
///////////////////////////////////////////////////////////////////////////////////////////////////////
		case AD_STDY:
			if (!mtmp->mcan && canseemon(mtmp) &&
				couldsee(mtmp->mx, mtmp->my) &&
				!is_blind(mtmp)
			) {
				int dmg = d((int)mattk->damn, (int)mattk->damd);
				if(mtmp->data == &mons[PM_ORC_CAPTAIN] || mtmp->data == &mons[PM_BOLG]) 
					pline("%s curses and urges %s followers on.", Monnam(mtmp), mhis(mtmp));
				else pline("%s studies you with a level stare.", Monnam(mtmp));
				u.ustdy = max(dmg,u.ustdy);
				dmg = 0;
			}
		break;
/*		case AD_SUMMON:
			int count;
			
			count = nasty(mtmp);	// summon something nasty
			if (mtmp->iswiz)
			    verbalize("Destroy the thief, my pet%s!", plur(count));
			else {
			    const char *mappear =
				(count == 1) ? "A monster appears" : "Monsters appear";
	
			    // messages not quite right if plural monsters created but
			    // only a single monster is seen 
			    if (Invisible && !perceives(mtmp->data) &&
					    (mtmp->mux != u.ux || mtmp->muy != u.uy))
				pline("%s around a spot near you!", mappear);
			    else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
				pline("%s around your displaced image!", mappear);
			    else
					pline("%s from nowhere!", mappear);
			}
		break;
*/
/*ifdef PM_BEHOLDER / work in progress /
	    case AD_SLEE:
		if(!mtmp->mcan && canseemon(mtmp) &&
		   couldsee(mtmp->mx, mtmp->my) && !is_blind(mtmp) &&
		   multi >= 0 && !rn2(5) && !Sleep_resistance) {

		    fall_asleep(-rnd(10), TRUE);
		    pline("%s gaze makes you very sleepy...",
			  s_suffix(Monnam(mtmp)));
		}
		break;
	    case AD_SLOW:
		if(!mtmp->mcan && canseemon(mtmp) && !is_blind(mtmp) &&
		   (HFast & (INTRINSIC|TIMEOUT)) &&
		   !defends(AD_SLOW, uwep) && !rn2(4))

		    u_slow_down();
		    stop_occupation();
		break;
endif*/
	    default: impossible("Gaze attack %d?", mattk->adtyp);
			succeeded=0;
		break;
	}
	return(succeeded);
}

#endif /* OVLB */
#ifdef OVL1

void
mdamageu(mtmp, n)	/* mtmp hits you for n points damage */
register struct monst *mtmp;
register int n;
{
	flags.botl = 1;
	if(n > 0) mtmp->mhurtu = TRUE;
//ifdef BARD
	n += mtmp->encouraged;
//endif
	if (Upolyd) {
		u.mh -= n;
		if (u.mh < 1) rehumanize();
	} else {
		u.uhp -= n;
		if(u.uhp < 1) done_in_by(mtmp);
	}
}

#endif /* OVL1 */
#ifdef OVLB

STATIC_OVL void
urustm(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	boolean vis;
	boolean is_acid;

	if (!mon || !obj) return; /* just in case */
	if (dmgtype(youmonst.data, AD_CORR))
	    is_acid = TRUE;
	else if (dmgtype(youmonst.data, AD_RUST))
	    is_acid = FALSE;
	else
	    return;

	vis = cansee(mon->mx, mon->my);

	if ((is_acid ? is_corrodeable(obj) : is_rustprone(obj)) &&
	    (is_acid ? obj->oeroded2 : obj->oeroded) < MAX_ERODE) {
		if (obj->greased || obj->oerodeproof || (obj->blessed && rn2(3))) {
		    if (vis)
			pline("Somehow, %s weapon is not affected.",
						s_suffix(mon_nam(mon)));
		    if (obj->greased && !rn2(2)) obj->greased = 0;
		} else {
		    if (vis)
			pline("%s %s%s!",
			        s_suffix(Monnam(mon)),
				aobjnam(obj, (is_acid ? "corrode" : "rust")),
			        (is_acid ? obj->oeroded2 : obj->oeroded)
				    ? " further" : "");
		    if (is_acid) obj->oeroded2++;
		    else obj->oeroded++;
		}
	}
}

#endif /* OVLB */
#ifdef OVL1

int
could_seduce(magr,mdef,mattk)
struct monst *magr, *mdef;
struct attack *mattk;
/* returns 0 if seduction impossible,
 *	   1 if fine,
 *	   2 if wrong gender for nymph */
{
	register struct permonst *pagr;
	boolean agrinvis, defperc;
	xchar genagr, gendef;

	static int engagering3 = 0;
	if (!engagering3) engagering3 = find_engagement_ring();
	if ( (uleft && uleft->otyp == engagering3) || (uright && uright->otyp == engagering3)) return 0;

	
	if (is_animal(magr->data)) return (0);
	if(magr == &youmonst) {
		pagr = youmonst.data;
		agrinvis = (Invis != 0);
		genagr = poly_gender();
	} else {
		pagr = magr->data;
		agrinvis = magr->minvis;
		genagr = gender(magr);
	}
	if(mdef == &youmonst) {
		defperc = (See_invisible != 0);
		gendef = poly_gender();
	} else {
		defperc = perceives(mdef->data);
		gendef = gender(mdef);
	}
	
	if(mattk && mattk->adtyp != AD_SSEX && mattk->adtyp != AD_LSEX && 
		mattk->adtyp != AD_SEDU && mattk->adtyp != AD_SITM
	) return 0;
	
	
	if(agrinvis && !defperc
#ifdef SEDUCE
		&& mattk && mattk->adtyp != AD_SSEX && mattk->adtyp != AD_LSEX
#endif
		)
		return 0;

	if(pagr->mlet == S_NYMPH || pagr == &mons[PM_INCUBUS] || pagr == &mons[PM_SUCCUBUS]
			|| pagr == &mons[PM_CARMILLA] || pagr == &mons[PM_VLAD_THE_IMPALER]){
		if(genagr == 1 - gendef)
			return 1;
		else
			return (pagr->mlet == S_NYMPH) ? 2 : 0;
	}
	else if(pagr == &mons[PM_MOTHER_LILITH] || pagr == &mons[PM_BELIAL]
		 /*|| pagr == &mons[PM_SHAMI_AMOURAE]*/){
		if(genagr == 1 - gendef) return 1;
		else return 0;
	}
	else if(pagr == &mons[PM_FIERNA]) return 2;
	else if(pagr == &mons[PM_MALCANTHET] || pagr == &mons[PM_GRAZ_ZT]
		 || pagr == &mons[PM_PALE_NIGHT] || pagr == &mons[PM_AVATAR_OF_LOLTH]) 
			return 1;
	else return 0;
}

#endif /* OVL1 */
#ifdef OVLB

#ifdef SEDUCE
/* Returns 1 if monster teleported */
int
doseduce(mon)
register struct monst *mon;
{
	register struct obj *ring, *nring;
	boolean fem = (mon->data == &mons[PM_SUCCUBUS]) || mon->data == &mons[PM_CARMILLA]; /* otherwise incubus */
	char qbuf[QBUFSZ];
	struct obj *key;
	int turns = 0;
	char class_list[MAXOCLASSES+2];
//	pline("starting ssex");
	if(TRUE){

		if (mon->mcan || mon->mspec_used) {
			pline("%s acts as though %s has got a %sheadache.",
				  Monnam(mon), mhe(mon),
				  mon->mcan ? "severe " : "");
			return 0;
		}

		if (unconscious()) {
			pline("%s seems dismayed at your lack of response.",
				  Monnam(mon));
			return 0;
		}

		if (Blind) pline("It caresses you...");
		else You_feel("very attracted to %s.", mon_nam(mon));

		for(ring = invent; ring; ring = nring) {
			nring = ring->nobj;
			if (ring->otyp != RIN_ADORNMENT) continue;
			if (fem) {
			if (rn2(20) < ACURR(A_CHA)) {
				Sprintf(qbuf, "\"That %s looks pretty.  May I have it?\"",
				safe_qbuf("",sizeof("\"That  looks pretty.  May I have it?\""),
				xname(ring), simple_typename(ring->otyp), "ring"));
				makeknown(RIN_ADORNMENT);
				if (yn(qbuf) == 'n') continue;
			} else pline("%s decides she'd like your %s, and takes it.",
				Blind ? "She" : Monnam(mon), xname(ring));
			makeknown(RIN_ADORNMENT);
			if (ring==uleft || ring==uright) Ring_gone(ring);
			if (ring==uwep) setuwep((struct obj *)0);
			if (ring==uswapwep) setuswapwep((struct obj *)0);
			if (ring==uquiver) setuqwep((struct obj *)0);
			freeinv(ring);
			(void) mpickobj(mon,ring);
			} else {
			char buf[BUFSZ];

			if (uleft && uright && uleft->otyp == RIN_ADORNMENT
					&& (uright->otyp==RIN_ADORNMENT || (uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)))
				break;
			if (ring==uleft || ring==uright) continue;
			if (rn2(20) < ACURR(A_CHA)) {
				Sprintf(qbuf,"\"That %s looks pretty.  Would you wear it for me?\"",
				safe_qbuf("",
					sizeof("\"That  looks pretty.  Would you wear it for me?\""),
					xname(ring), simple_typename(ring->otyp), "ring"));
				makeknown(RIN_ADORNMENT);
				if (yn(qbuf) == 'n') continue;
			} else {
				pline("%s decides you'd look prettier wearing your %s,",
				Blind ? "He" : Monnam(mon), xname(ring));
				pline("and puts it on your finger.");
			}
			makeknown(RIN_ADORNMENT);
			if (!uright && !(uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)) {
				pline("%s puts %s on your right %s.",
				Blind ? "He" : Monnam(mon), the(xname(ring)), body_part(HAND));
				setworn(ring, RIGHT_RING);
			} else if (!uleft) {
				pline("%s puts %s on your left %s.",
				Blind ? "He" : Monnam(mon), the(xname(ring)), body_part(HAND));
				setworn(ring, LEFT_RING);
			} else if (uright && uright->otyp != RIN_ADORNMENT && !(uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)) {
				Strcpy(buf, xname(uright));
				pline("%s replaces your %s with your %s.",
				Blind ? "He" : Monnam(mon), buf, xname(ring));
				Ring_gone(uright);
				setworn(ring, RIGHT_RING);
			} else if (uleft && uleft->otyp != RIN_ADORNMENT) {
				Strcpy(buf, xname(uleft));
				pline("%s replaces your %s with your %s.",
				Blind ? "He" : Monnam(mon), buf, xname(ring));
				Ring_gone(uleft);
				setworn(ring, LEFT_RING);
			} else impossible("ring replacement");
			Ring_on(ring);
			prinv((char *)0, ring, 0L);
			}
		}

		if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh
	#ifdef TOURIST
									&& !uarmu
	#endif
										)
			pline("%s murmurs sweet nothings into your ear.",
				Blind ? (fem ? "She" : "He") : Monnam(mon));
		else
			pline("%s murmurs in your ear, while helping you undress.",
				Blind ? (fem ? "She" : "He") : Monnam(mon));
		mayberem(uarmc, cloak_simple_name(uarmc));
		if(!uarmc)
			mayberem(uarm, "suit");
		mayberem(uarmf, "boots");
		if(!uwep || !welded(uwep))
			mayberem(uarmg, "gloves");
		mayberem(uarms, "shield");
		mayberem(uarmh, "helmet");
	#ifdef TOURIST
		if(!uarmc && !uarm)
			mayberem(uarmu, "shirt");
	#endif

		if (uarm || uarmc || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)) {
			verbalize("You're such a %s; I wish...",
					flags.female ? "sweet lady" : "nice guy");
			if(u.sealsActive&SEAL_ENKI) unbind(SEAL_ENKI,TRUE);
			if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
			return 1;
		}
		if (u.ualign.type == A_CHAOTIC)
			adjalign(1);

		/* by this point you have discovered mon's identity, blind or not... */
		if(!uclockwork){
			pline("Time stands still while you and %s lie in each other's arms...",
				noit_mon_nam(mon));
		}
		else{
			pline("You and %s lie down together...",
				noit_mon_nam(mon));
		}
		if (rn2(35) > ACURR(A_CHA) + ACURR(A_INT)) {
			/* Don't bother with mspec_used here... it didn't get tired! */
			if(!uclockwork){
				pline("%s seems to have enjoyed it more than you...",
					noit_Monnam(mon));
			} else{
				char buf[BUFSZ];
				pline("%s looks briefly confused...",
					noit_Monnam(mon));
				if(!rn2(5) && !Drain_resistance){
					pline("...then tries to suck out your soul with a kiss!");
					losexp("stolen soul",FALSE,FALSE,FALSE);
				}
				else {
					buf[0] = '\0';
					steal(mon, buf,FALSE);
				}
				goto pay;
			}
			switch (rn2(5)) {
				case 0: You_feel("drained of energy.");
					u.uen = 0;
					u.uenmax -= rnd(Half_physical_damage ? 5 : 10);
						exercise(A_CON, FALSE);
					if (u.uenmax < 0) u.uenmax = 0;
					break;
				case 1: You("are down in the dumps.");
					(void) adjattrib(A_CON, -1, TRUE);
						exercise(A_CON, FALSE);
					flags.botl = 1;
					break;
				case 2: Your("senses are dulled.");
					(void) adjattrib(A_WIS, -1, TRUE);
						exercise(A_WIS, FALSE);
					flags.botl = 1;
					break;
				case 3:
					if (!Drain_resistance) {
						You_feel("out of shape.");
						losexp("overexertion",TRUE,FALSE,FALSE);
					} else {
						You("have a curious feeling...");
					}
					break;
				case 4: {
					int tmp;
					You_feel("exhausted.");
						exercise(A_STR, FALSE);
					tmp = rn1(10, 6);
					if(Half_physical_damage) tmp = (tmp+1) / 2;
					losehp(tmp, "exhaustion", KILLED_BY);
					break;
				}
			}
		} else {
			mon->mspec_used = rnd(100); /* monster is worn out */
			if(!uclockwork){
				You("seem to have enjoyed it more than %s...",
					noit_mon_nam(mon));
			} else{
				pline("Time stands still while you and %s lie in each other's arms...",
					noit_mon_nam(mon));
				if(!rn2(5)){
					pline("That was a very educational experience.");
					pluslvl(FALSE);
					goto pay;
				} else if(u.uhunger < .5*u.uhungermax && !Race_if(PM_INCANTIFIER)){
					You("persuade %s to wind your clockwork.",
						noit_mon_nam(mon));
					struct obj *key;
					int turns = 0;
					
					Strcpy(class_list, tools);
					key = getobj(class_list, "wind with");
					if (!key){
						pline(Never_mind);
						goto pay;
					}
					turns = ask_turns(mon, 0, 0);
					if(!turns){
						pline(Never_mind);
						goto pay;
					}
					turns = (.8 + ((double)rn2(5))) * (turns);
					lesshungry(turns*10);
					You("notice %s wound your clockwork %d times.",noit_mon_nam(mon),turns);
					goto pay;
				} else {
					pline("%s looks happy, but confused.",
						noit_Monnam(mon));
					goto pay;
				}
			}
			switch (rn2(5)) {
			case 0: You_feel("raised to your full potential.");
				exercise(A_CON, TRUE);
				u.uen = (u.uenmax += rnd(5));
				break;
			case 1: You_feel("good enough to do it again.");
				(void) adjattrib(A_CON, 1, TRUE);
				exercise(A_CON, TRUE);
				flags.botl = 1;
				break;
			case 2: You("will always remember %s...", noit_mon_nam(mon));
				(void) adjattrib(A_WIS, 1, TRUE);
				exercise(A_WIS, TRUE);
				flags.botl = 1;
				break;
			case 3: pline("That was a very educational experience.");
				pluslvl(FALSE);
				exercise(A_WIS, TRUE);
				break;
			case 4: You_feel("restored to health!");
				u.uhp = u.uhpmax;
				if (Upolyd) u.mh = u.mhmax;
				exercise(A_STR, TRUE);
				flags.botl = 1;
				break;
			}
		}
pay:
		if (mon->mtame) /* don't charge */ ;
		else if (rn2(20) < ACURR(A_CHA)) {
			pline("%s demands that you pay %s, but you refuse...",
				noit_Monnam(mon),
				Blind ? (fem ? "her" : "him") : mhim(mon));
		} else if (u.umonnum == PM_LEPRECHAUN)
			pline("%s tries to take your money, but fails...",
					noit_Monnam(mon));
		else {
	#ifndef GOLDOBJ
			long cost;

			if (u.ugold > (long)LARGEST_INT - 10L)
				cost = (long) rnd(LARGEST_INT) + 500L;
			else
				cost = (long) rnd((int)u.ugold + 10) + 500L;
			if (mon->mpeaceful) {
				cost /= 5L;
				if (!cost) cost = 1L;
			}
			if (cost > u.ugold) cost = u.ugold;
			if (!cost) verbalize("It's on the house!");
			else {
				pline("%s takes %ld %s for services rendered!",
					noit_Monnam(mon), cost, currency(cost));
				u.ugold -= cost;
				mon->mgold += cost;
				flags.botl = 1;
			}
	#else
			long cost;
					long umoney = money_cnt(invent);

			if (umoney > (long)LARGEST_INT - 10L)
				cost = (long) rnd(LARGEST_INT) + 500L;
			else
				cost = (long) rnd((int)umoney + 10) + 500L;
			if (mon->mpeaceful) {
				cost /= 5L;
				if (!cost) cost = 1L;
			}
			if (cost > umoney) cost = umoney;
			if (!cost) verbalize("It's on the house!");
			else { 
				pline("%s takes %ld %s for services rendered!",
					noit_Monnam(mon), cost, currency(cost));
						money2mon(mon, cost);
				flags.botl = 1;
			}
	#endif
		}
	}
	if (!rn2(25)) mon->mcan = 1; /* monster is worn out */
	if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
	return 1;
}

int
dololthseduce(mon)
register struct monst *mon;
{
	register struct obj *ring, *nring;
	boolean fem = TRUE, helpless;
	char qbuf[QBUFSZ];
	struct obj *key;
	int turns = 0;
	char class_list[MAXOCLASSES+2];
//	pline("starting ssex");
	if(TRUE){
		if (unconscious()) {
			helpless = TRUE;
			pline("%s seems pleased at your lack of response.",
				  Monnam(mon));
		}

		if (Blind) pline("It caresses you...");
		else You_feel("very attracted to %s.", mon_nam(mon));

		for(ring = invent; ring; ring = nring) {
			nring = ring->nobj;
			if (ring->otyp != RIN_ADORNMENT) continue;
			{
				char buf[BUFSZ];

				if (uleft && uright && uleft->otyp == RIN_ADORNMENT
						&& (uright->otyp==RIN_ADORNMENT || (uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)))
					break;
				if (ring==uleft || ring==uright) continue;
				if (!helpless && rn2(40) < ACURR(A_CHA)) {
					Sprintf(qbuf,"\"That %s looks pretty.  Would you wear it for me?\"",
					safe_qbuf("",
						sizeof("\"That  looks pretty.  Would you wear it for me?\""),
						xname(ring), simple_typename(ring->otyp), "ring"));
					makeknown(RIN_ADORNMENT);
					if (yn(qbuf) == 'n') continue;
				} else {
					pline("%s decides you'd look prettier wearing your %s,",
					Blind ? "She" : Monnam(mon), xname(ring));
					pline("and puts it on your finger.");
				}
				makeknown(RIN_ADORNMENT);
				if (!uright && !(uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)) {
					pline("%s puts %s on your right %s.",
					Blind ? "She" : Monnam(mon), the(xname(ring)), body_part(HAND));
					setworn(ring, RIGHT_RING);
				} else if (!uleft) {
					pline("%s puts %s on your left %s.",
					Blind ? "She" : Monnam(mon), the(xname(ring)), body_part(HAND));
					setworn(ring, LEFT_RING);
				} else if (uright && uright->otyp != RIN_ADORNMENT && !(uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)) {
					Strcpy(buf, xname(uright));
					pline("%s replaces your %s with your %s.",
					Blind ? "She" : Monnam(mon), buf, xname(ring));
					Ring_gone(uright);
					setworn(ring, RIGHT_RING);
				} else if (uleft && uleft->otyp != RIN_ADORNMENT) {
					Strcpy(buf, xname(uleft));
					pline("%s replaces your %s with your %s.",
					Blind ? "She" : Monnam(mon), buf, xname(ring));
					Ring_gone(uleft);
					setworn(ring, LEFT_RING);
				} else impossible("ring replacement");
				Ring_on(ring);
				prinv((char *)0, ring, 0L);
			}
		}

		if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh
	#ifdef TOURIST
									&& !uarmu
	#endif
										)
			pline("%s murmurs sweet nothings into your ear.",
				Blind ? "She" : Monnam(mon));
		else
			pline("%s murmurs in your ear, while helping you undress.",
				Blind ? "She" : Monnam(mon));
		lrdmayberem(uarmc, cloak_simple_name(uarmc));
		if(!uarmc)
			lrdmayberem(uarm, "suit");
		lrdmayberem(uarmf, "boots");
		if(!uwep || !welded(uwep))
			lrdmayberem(uarmg, "gloves");
		lrdmayberem(uarms, "shield");
		lrdmayberem(uarmh, "helmet");
	#ifdef TOURIST
		if(!uarmc && !uarm)
			lrdmayberem(uarmu, "shirt");
	#endif

		if (u.ualign.type == A_CHAOTIC)
			adjalign(1);

		/* by this point you have discovered mon's identity, blind or not... */
		if (helpless || rn2(120) > ACURR(A_CHA) + ACURR(A_WIS)) {
			struct trap *ttmp2 = maketrap(u.ux, u.uy, WEB);
			/* Don't bother with mspec_used here... it didn't get tired! */
LolthAttacks:
			if(Blind) You("suddenly find yourself in the arms of a giant spider!");
			else pline("She suddenly becomes a giant spider and seizes you with her legs!");
			//Lolth bad
			if (ttmp2) {
				pline("She wraps you tight in her webs!");
				dotrap(ttmp2, NOWEBMSG);
#ifdef STEED
				if (u.usteed && u.utrap) {
				/* you, not steed, are trapped */
				dismount_steed(DISMOUNT_FELL);
				}
#endif
			}
			if(uclockwork){
				char buf[BUFSZ];
				pline("%s pauses in momentary confusion...",
					noit_Monnam(mon));
				if(rn2(5) && !Drain_resistance){
					pline("...then tries to suck out your soul with her fangs!");
					losexp("stolen soul",FALSE,FALSE,FALSE);
					losexp("stolen soul",FALSE,FALSE,FALSE);
				}
				else {
					buf[0] = '\0';
					pline("...then starts picking through your things!");
					steal(mon, buf,FALSE);
					steal(mon, buf,FALSE);
					steal(mon, buf,FALSE);
					steal(mon, buf,FALSE);
				}
			} else {
			int tmp;
			pline("After wrapping you up, she bites into your helpless form!");
				exercise(A_STR, FALSE);
			tmp = d(6, 8);
			if(Half_physical_damage) tmp = (tmp+1) / 2;
			losehp(tmp, "Lolth's bite", KILLED_BY);
			switch (rn2(2)) {
				case 0: 
				if (has_blood(youmonst.data)) {
				   Your("blood is being drained!");
				   /* Get 1/20th of full corpse value
					* Therefore 4 bites == 1 drink
					*/
					if (mon->mtame && !mon->isminion)
						EDOG(mon)->hungrytime += ((int)((youmonst.data)->cnutrit / 20) + 1);
				}
				if (!mon->mcan && !rn2(3) && !Drain_resistance) {
					losexp("life force drain",FALSE,FALSE,FALSE);
					losexp("life force drain",TRUE,FALSE,FALSE);
				}
				break;
				case 1:
					pline("She injects you with her poison!");
					if(Poison_resistance) pline_The("poison doesn't seem to affect you.");
					else {
					(void) adjattrib(A_CON, -4, TRUE);
						exercise(A_CON, FALSE);
					flags.botl = 1;
					if(!Upolyd || Unchanging){
						killer = "the poisoned kiss of Lolth's fangs";
						killer_format = KILLED_BY;
						done(DIED);
					} else rehumanize();
					}
				break;
			}
			}
		} else {
			//Lolth good
			if (uarm || uarmc || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)) {
				if(flags.female){
					verbalize("You're such a sweet lady, I wish you were more open to new things...",
							flags.female ? "sweet lady" : "nice guy");
					if(u.sealsActive&SEAL_ENKI) unbind(SEAL_ENKI,TRUE);
					if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
					return 1;
				} else {
					verbalize("How dare you refuse me!");
					goto LolthAttacks;
				}
			}
			mon->mspec_used = rnd(100); /* monster is worn out */
			if(Blind) pline("An elf-maid clasps herself to you!");
			else pline("She becomes a beautiful dark-skinned elf-maid!");
			if(!uclockwork){
				pline("Time stands still while you lie in each other's arms...");
			} else{
				pline("You and %s lie down together...",
					noit_mon_nam(mon));
				pline("Time stands still while you and %s lie in each other's arms...",
					noit_mon_nam(mon));
			}
			switch (rn2(4)) {
			case 0: 
				verbalize("Tell me your greatest desire!");
				makewish();
			break;
			case 1:
					verbalize("Go forth and slay thy enemies with my blessing!");
					u.udaminc += d(1,10);
					u.uhitinc += d(1,10);
			break;
			case 2: pline("That was a very educational experience.");
				pluslvl(FALSE);
				exercise(A_WIS, TRUE);
			break;
			case 3: You_feel("restored to health!");
				u.uhp = u.uhpmax;
				if (Upolyd) u.mh = u.mhmax;
				exercise(A_STR, TRUE);
				flags.botl = 1;
				break;
			}
		}
	}
	if (!rn2(25)) mon->mcan = 1; /* monster is worn out */
	if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
	return 1;
}

int
dolilithseduce(mon)
struct monst *mon;
{
	struct obj *ring, *nring;
	boolean fem = TRUE; /* Lilith */
	//char qbuf[QBUFSZ];
	char qbuf[QBUFSZ];
	struct obj *key;
	int turns = 0;
	char class_list[MAXOCLASSES+2];



	if (mon->mcan || mon->mspec_used) {
		pline("%s is uninterested in you.", Monnam(mon));
		return 0;
	}

	if (unconscious()) {
		pline("%s seems bored by your lack of response.",
			  Monnam(mon));
		return 0;
	}

	if (Blind) pline("It caresses you...");
	else You_feel("very attracted to %s.", mon_nam(mon));

	for(ring = invent; ring; ring = nring) {
		nring = ring->nobj;
		if (ring->otyp != RIN_ADORNMENT) continue;
		pline("%s decides she'd like your %s, and takes it.",
			Blind ? "She" : Monnam(mon), xname(ring));
		makeknown(RIN_ADORNMENT);
		if (ring==uleft || ring==uright) Ring_gone(ring);
		if (ring==uwep) setuwep((struct obj *)0);
		if (ring==uswapwep) setuswapwep((struct obj *)0);
		if (ring==uquiver) setuqwep((struct obj *)0);
		freeinv(ring);
		(void) mpickobj(mon,ring);
		
	}

	if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh
#ifdef TOURIST
								&& !uarmu
#endif
									)
		pline("She murmurs sweet nothings into your ear.");
	else
		pline("She murmurs in your ear, while helping you undress.");
	lrdmayberem(uarmc, cloak_simple_name(uarmc));
	if(!uarmc)
		lrdmayberem(uarm, "suit");
	lrdmayberem(uarmf, "boots");
	if(!uwep || !welded(uwep))
		lrdmayberem(uarmg, "gloves");
	lrdmayberem(uarms, "shield");
	lrdmayberem(uarmh, "helmet");
#ifdef TOURIST
	if(!uarmc && !uarm)
		lrdmayberem(uarmu, "shirt");
#endif

	if (uarm || uarmc || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)) {
		verbalize("You're such a %s; I wish...",
				flags.female ? "sweet lady" : "nice guy");
		if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
		return 1;
	}
	/* by this point you have discovered mon's identity, blind or not... */
	if(!uclockwork){
		pline("Time stands still while you and %s lie in each other's arms...",
			noit_mon_nam(mon));
	}
	else{
		pline("You and %s lie down together...",
			noit_mon_nam(mon));
	}
	if (rn2(139) > ACURR(A_CHA) + ACURR(A_INT)) {
		if(!uclockwork){
			pline("%s seems to have enjoyed it more than you...",
				noit_Monnam(mon));
		} else{
			char buf[BUFSZ];
			pline("%s looks briefly confused...",
				noit_Monnam(mon));
			if(!rn2(5) && !Drain_resistance){
				pline("...then tries to suck out your soul with a kiss!");
				losexp("stolen soul",FALSE,FALSE,FALSE);
				losexp("stolen soul",FALSE,FALSE,FALSE);
				losexp("stolen soul",TRUE,FALSE,FALSE);
			}
			else {
				buf[0] = '\0';
				steal(mon, buf,FALSE);
			}
			pline("Before you can get up, %s slips a knife into your gears!",
			noit_Monnam(mon));
			if(Half_physical_damage) losehp(rn1(5, 6), "knife to the ribs", KILLED_BY);
			else losehp(rn1(10, 6), "knife in the gears", KILLED_BY);
			goto pay;
		}
		switch (rn2(5)) {
			case 0: You_feel("drained of energy.");
				u.uen = 0;
				u.uenmax -= rnd(Half_physical_damage ? 45 : 90);
					exercise(A_CON, FALSE);
				if (u.uenmax < 0) u.uenmax = 0;
				break;
			case 1: You("are down in the dumps.");
				(void) adjattrib(A_CON, -6, TRUE);
				(void) adjattrib(A_WIS, -3, TRUE);
					exercise(A_CON, FALSE);
				flags.botl = 1;
				break;
			case 2: Your("senses are dulled.");
				(void) adjattrib(A_WIS, -9, TRUE);
					exercise(A_WIS, FALSE);
					exercise(A_WIS, FALSE);
					exercise(A_WIS, FALSE);
				flags.botl = 1;
				break;
			case 3:
				if (!Drain_resistance) {
					You_feel("out of shape.");
					losexp("overexertion",FALSE,FALSE,FALSE);
					losexp("overexertion",FALSE,FALSE,FALSE);
					losexp("overexertion",TRUE,FALSE,FALSE);
				} else {
					You("have a curious feeling...");
				}
				break;
			case 4: {
				int tmp;
				if (!Drain_resistance) {
					losexp("exhaustion",TRUE,FALSE,FALSE);
				}
				You_feel("exhausted.");
					exercise(A_STR, FALSE);
					exercise(A_STR, FALSE);
					exercise(A_STR, FALSE);
				tmp = rn1(20, 6);
				if(Half_physical_damage) tmp = (tmp+1) / 2;
				losehp(tmp, "exhaustion", KILLED_BY);
				break;
			}
		}
		if(ACURR(A_CHA)+rn1(4,3) < 24){
			pline("Before you can get up, %s slips a knife between your ribs!",
			noit_Monnam(mon));
			if(Half_physical_damage) losehp(rn1(5, 6), "knife to the ribs", KILLED_BY);
			else losehp(rn1(10, 6), "knife to the ribs", KILLED_BY);
		}
		else{
			pline("As you get up, %s tries to knife you, but is too distracted to do it properly!",
			noit_Monnam(mon));
			if(Half_physical_damage) losehp(rn1(1, 6), "knife to the ribs", KILLED_BY);
			else losehp(rn1(2, 6), "knife to the ribs", KILLED_BY);
		}
		mon->mspec_used = rnd(13)+3;
		return 0;
	} else {
		mon->mspec_used = rnd(39)+13;
		if(!uclockwork){
			You("seem to have enjoyed it more than %s...",
				noit_mon_nam(mon));
		} else{
			pline("Time stands still while you and %s lie in each other's arms...",
				noit_mon_nam(mon));
			if(!rn2(5)){
				pline("That was a very educational experience!");
				pluslvl(FALSE);
				pluslvl(FALSE);
				goto pay;
			} else if(u.uhunger < .5*u.uhungermax && !Race_if(PM_INCANTIFIER)){
				You("persuade %s to wind your clockwork.",
					noit_mon_nam(mon));
				struct obj *key;
				int turns = 0;
				
				Strcpy(class_list, tools);
				key = getobj(class_list, "wind with");
				if (!key){
					pline(Never_mind);
					goto pay;
				}
				turns = ask_turns(mon, 0, 0);
				if(!turns){
					pline(Never_mind);
					goto pay;
				}
				lesshungry(turns*10);
				You("notice %s wound your clockwork %d times.",noit_mon_nam(mon),turns);
				goto pay;
			} else {
				pline("%s looks happy, but confused.",
					noit_Monnam(mon));
				goto pay;
			}
		}
		switch (rn2(5)) {
		case 0: You_feel("raised to your full potential.");
			exercise(A_CON, TRUE);
			u.uen = (u.uenmax += rnd(10)+5);
			exercise(A_CON, TRUE);
			break;
		case 1: You_feel("good enough to do it again.");
			(void) adjattrib(A_CON, 2, TRUE);
			exercise(A_CON, TRUE);
			exercise(A_CON, TRUE);
			exercise(A_CON, TRUE);
			flags.botl = 1;
			break;
		case 2: You("will always remember %s...", noit_mon_nam(mon));
			(void) adjattrib(A_WIS, 2, TRUE);
			exercise(A_WIS, TRUE);
			exercise(A_WIS, TRUE);
			exercise(A_WIS, TRUE);
			flags.botl = 1;
			break;
		case 3: pline("That was a very educational experience.");
			pluslvl(FALSE);
			exercise(A_WIS, TRUE);
			pluslvl(FALSE);
			exercise(A_WIS, TRUE);
			break;
		case 4: You_feel("restored to health!");
			u.uhp = u.uhpmax;
			if (Upolyd) u.mh = u.mhmax;
			exercise(A_STR, TRUE);
			u.uen = (u.uenmax += rnd(5));
			flags.botl = 1;
			break;
		}
	}
pay:
	if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
	return 1;
}

int
dobelialseduce(mon)
struct monst *mon;
{
	struct obj *ring, *nring;
	boolean fem = FALSE; /* Belial */
	//char qbuf[QBUFSZ];
	char qbuf[QBUFSZ];
	struct obj *key;
	int turns = 0;
	char class_list[MAXOCLASSES+2];


	if (mon->mcan || mon->mspec_used) {
		pline("%s is uninterested in you.", Monnam(mon));
		return 0;
	}

	if (unconscious()) {
		pline("%s seems bored by your lack of response.",
			  Monnam(mon));
		return 0;
	}

	if (Blind) pline("It caresses you...");
	else You_feel("very attracted to %s.", mon_nam(mon));

	for(ring = invent; ring; ring = nring) {
		nring = ring->nobj;
		if (ring->otyp != RIN_ADORNMENT) continue;
		pline("%s decides he'd like your %s, and takes it.",
			Blind ? "He" : Monnam(mon), xname(ring));
		makeknown(RIN_ADORNMENT);
		if (ring==uleft || ring==uright) Ring_gone(ring);
		if (ring==uwep) setuwep((struct obj *)0);
		if (ring==uswapwep) setuswapwep((struct obj *)0);
		if (ring==uquiver) setuqwep((struct obj *)0);
		freeinv(ring);
		(void) mpickobj(mon,ring);
		
	}

	if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh
#ifdef TOURIST
								&& !uarmu
#endif
									)
		pline("He murmurs sweet nothings into your ear.");
	else
		pline("He murmurs in your ear, while helping you undress.");
	lrdmayberem(uarmc, cloak_simple_name(uarmc));
	if(!uarmc)
		lrdmayberem(uarm, "suit");
	lrdmayberem(uarmf, "boots");
	if(!uwep || !welded(uwep))
		lrdmayberem(uarmg, "gloves");
	lrdmayberem(uarms, "shield");
	lrdmayberem(uarmh, "helmet");
#ifdef TOURIST
	if(!uarmc && !uarm)
		lrdmayberem(uarmu, "shirt");
#endif

	if (uarm || uarmc || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)) {
		verbalize("You're such a %s; I wish...",
				flags.female ? "sweet lady" : "nice guy");
		if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
		return 1;
	}
	/* by this point you have discovered mon's identity, blind or not... */
	if(!uclockwork){
		pline("Time stands still while you and %s lie in each other's arms...",
			noit_mon_nam(mon));
	}
	else{
		pline("You and %s lie down together...",
			noit_mon_nam(mon));
	}
	if (rn2(139) > ACURR(A_CHA) + ACURR(A_INT)) {
		if(!uclockwork){
			pline("%s seems to have enjoyed it more than you...",
				noit_Monnam(mon));
		} else{
			char buf[BUFSZ];
			pline("%s looks briefly confused...",
				noit_Monnam(mon));
			if(!rn2(5) && !Drain_resistance){
				pline("...then tries to suck out your soul with a kiss!");
				losexp("stolen soul",FALSE,FALSE,FALSE);
				losexp("stolen soul",FALSE,FALSE,FALSE);
				losexp("stolen soul",FALSE,FALSE,FALSE);
			}
			else {
				buf[0] = '\0';
				steal(mon, buf,FALSE);
			}
			pline("Before you can get up, %s slips a knife between your ribs!",
			noit_Monnam(mon));
			if(Half_physical_damage) losehp(d(5, 6), "knife to the ribs", KILLED_BY);
			else losehp(d(10, 6), "knife to the ribs", KILLED_BY);
			goto pay;
		}
		switch (rn2(5)) {
			case 0: You_feel("drained of energy.");
				u.uen = 0;
				u.uenmax -= rnd(Half_physical_damage ? 45 : 90);
					exercise(A_CON, FALSE);
				if (u.uenmax < 0) u.uenmax = 0;
				break;
			case 1: You("are down in the dumps.");
				(void) adjattrib(A_CON, -6, TRUE);
				(void) adjattrib(A_WIS, -3, TRUE);
					exercise(A_CON, FALSE);
				flags.botl = 1;
				break;
			case 2: Your("senses are dulled.");
				(void) adjattrib(A_WIS, -9, TRUE);
					exercise(A_WIS, FALSE);
					exercise(A_WIS, FALSE);
					exercise(A_WIS, FALSE);
				flags.botl = 1;
				break;
			case 3:
				if (!Drain_resistance) {
					You_feel("out of shape.");
					losexp("overexertion",FALSE,FALSE,FALSE);
					losexp("overexertion",FALSE,FALSE,FALSE);
					losexp("overexertion",TRUE,FALSE,FALSE);
				} else {
					You("have a curious feeling...");
				}
				break;
			case 4: {
				int tmp;
				if (!Drain_resistance) {
					losexp("exhaustion",TRUE,FALSE,FALSE);
				}
				You_feel("exhausted.");
					exercise(A_STR, FALSE);
					exercise(A_STR, FALSE);
					exercise(A_STR, FALSE);
				tmp = d(20, 6);
				if(Half_physical_damage) tmp = (tmp+1) / 2;
				losehp(tmp, "exhaustion", KILLED_BY);
				break;
			}
		}
		if(ACURR(A_CHA)+d(4,3) < 24){
			pline("Before you can get up, %s slips a knife between your ribs!",
			noit_Monnam(mon));
			if(Half_physical_damage) losehp(d(5, 6), "knife to the ribs", KILLED_BY);
			else losehp(d(10, 6), "knife to the ribs", KILLED_BY);
		}
		else{
			pline("As you get up, %s tries to knife you, but is too distracted to do it properly!",
			noit_Monnam(mon));
			if(Half_physical_damage) losehp(d(1, 6), "knife to the ribs", KILLED_BY);
			else losehp(d(2, 6), "knife to the ribs", KILLED_BY);
		}
		mon->mspec_used = rnd(13)+3;
		return 0;
	} else {
		mon->mspec_used = rnd(39)+13;
		if(!uclockwork){
			You("seem to have enjoyed it more than %s...",
				noit_mon_nam(mon));
		} else{
			pline("Time stands still while you and %s lie in each other's arms...",
				noit_mon_nam(mon));
			if(!rn2(5)){
				pline("That was a very educational experience.");
				pluslvl(FALSE);
				pluslvl(FALSE);
				goto pay;
			} else if(u.uhunger < .5*u.uhungermax && !Race_if(PM_INCANTIFIER)){
				You("persuade %s to wind your clockwork.",
					noit_mon_nam(mon));
				struct obj *key;
				int turns = 0;
				
				Strcpy(class_list, tools);
				key = getobj(class_list, "wind with");
				if (!key){
					pline(Never_mind);
					goto pay;
				}
				turns = ask_turns(mon, 0, 0);
				if(!turns){
					pline(Never_mind);
					goto pay;
				}
				lesshungry(turns*10);
				You("notice %s wound your clockwork %d times.",noit_mon_nam(mon),turns);
				goto pay;
			} else {
				pline("%s looks happy, but confused.",
					noit_Monnam(mon));
				goto pay;
			}
		}
		switch (rn2(5)) {
		case 0: You_feel("raised to your full potential.");
			exercise(A_CON, TRUE);
			u.uen = (u.uenmax += rnd(10)+5);
			exercise(A_CON, TRUE);
			break;
		case 1: You_feel("good enough to do it again.");
			(void) adjattrib(A_CON, 2, TRUE);
			exercise(A_CON, TRUE);
			exercise(A_CON, TRUE);
			exercise(A_CON, TRUE);
			flags.botl = 1;
			break;
		case 2: You("will always remember %s...", noit_mon_nam(mon));
			(void) adjattrib(A_WIS, 2, TRUE);
			exercise(A_WIS, TRUE);
			exercise(A_WIS, TRUE);
			exercise(A_WIS, TRUE);
			flags.botl = 1;
			break;
		case 3: pline("That was a very educational experience.");
			pluslvl(FALSE);
			exercise(A_WIS, TRUE);
			pluslvl(FALSE);
			exercise(A_WIS, TRUE);
			break;
		case 4: You_feel("restored to health!");
			u.uhp = u.uhpmax;
			if (Upolyd) u.mh = u.mhmax;
			exercise(A_STR, TRUE);
			u.uen = (u.uenmax += rnd(5));
			flags.botl = 1;
			break;
		}
	}
pay:
	if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
	return 1;
}

int
domlcseduce(mon)
register struct monst *mon;
{
	register struct obj *ring, *nring;
	boolean fem = TRUE; /* otherwise incubus */
	boolean ufem=poly_gender();
	char qbuf[QBUFSZ];
	boolean helpless = FALSE;
//	pline("starting mlc seduce");
	if (mon->mcan || mon->mspec_used) {
		pline("%s is uninterested in you.", Monnam(mon));
		return 0;
	}

	if (unconscious()) {
//		pline("%s seems annoyed at your lack of response.",
//			  Monnam(mon));
			You("are having a strange dream.");
			helpless = TRUE;
	}

	if (Blind) pline("It caresses you...");
	else You_feel("very attracted to %s.", mon_nam(mon));

	for(ring = invent; ring; ring = nring) {
		nring = ring->nobj;
		if (ring->otyp != RIN_ADORNMENT) continue;
		if (ufem) {
		if (rn2(45) < ACURR(A_CHA)) {
			Sprintf(qbuf, "\"That %s looks pretty.  Give it to me.\"",
			safe_qbuf("",sizeof("\"That  looks pretty.  Give it to me.\""),
			xname(ring), simple_typename(ring->otyp), "ring"));
			makeknown(RIN_ADORNMENT);
			if (yn(qbuf) == 'n') continue;
		} else pline("%s decides she'd like your %s, and takes it.",
			Blind ? "She" : Monnam(mon), xname(ring));
		makeknown(RIN_ADORNMENT);
		if (ring==uleft || ring==uright) Ring_gone(ring);
		if (ring==uwep) setuwep((struct obj *)0);
		if (ring==uswapwep) setuswapwep((struct obj *)0);
		if (ring==uquiver) setuqwep((struct obj *)0);
		freeinv(ring);
		(void) mpickobj(mon,ring);
		} else {
		char buf[BUFSZ];
		if (uleft && uright && uleft->otyp == RIN_ADORNMENT
				&& (uright->otyp==RIN_ADORNMENT || (uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)))
			break;
		if (ring==uleft || ring==uright) continue;
		
		pline("%s decides you'd look more handsome wearing your %s,",
		Blind ? "She" : Monnam(mon), xname(ring));
		pline("and puts it on your finger.");
		
		makeknown(RIN_ADORNMENT);
		if (!uright && !(uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)) {
			pline("%s puts %s on your right %s.",
			Blind ? "She" : Monnam(mon), the(xname(ring)), body_part(HAND));
			setworn(ring, RIGHT_RING);
		} else if (!uleft) {
			pline("%s puts %s on your left %s.",
			Blind ? "She" : Monnam(mon), the(xname(ring)), body_part(HAND));
			setworn(ring, LEFT_RING);
		} else if (uright && uright->otyp != RIN_ADORNMENT && !(uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)) {
			Strcpy(buf, xname(uright));
			pline("%s replaces your %s with your %s.",
			Blind ? "She" : Monnam(mon), buf, xname(ring));
			Ring_gone(uright);
			setworn(ring, RIGHT_RING);
		} else if (uleft && uleft->otyp != RIN_ADORNMENT) {
			Strcpy(buf, xname(uleft));
			pline("%s replaces your %s with your %s.",
			Blind ? "She" : Monnam(mon), buf, xname(ring));
			Ring_gone(uleft);
			setworn(ring, LEFT_RING);
		} else impossible("ring replacement");
		Ring_on(ring);
		prinv((char *)0, ring, 0L);
		}
	}

	if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh
#ifdef TOURIST
								&& !uarmu
#endif
									)
		pline("%s caresses your body.",
			Blind ? "She" : Monnam(mon));
	else
		pline("%s starts undressing you.",
			Blind ? "She" : Monnam(mon));
	mlcmayberem(uarmc, cloak_simple_name(uarmc), helpless);
	if(!uarmc)
		mlcmayberem(uarm, "suit", helpless);
	mlcmayberem(uarmf, "boots", helpless);
	if(!uwep || !welded(uwep))
		mlcmayberem(uarmg, "gloves", helpless);
	mlcmayberem(uarms, "shield", helpless);
	mlcmayberem(uarmh, "helmet", helpless);
#ifdef TOURIST
	if(!uarmc && !uarm)
		mlcmayberem(uarmu, "shirt", helpless);
#endif

	if (uarm || uarmc || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)) {
		verbalize("How dare you refuse me!");
		pline("She claws at you!");
		losehp(d(4, 4), "a jilted paramour", KILLED_BY);
		if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
		return 1;
	}
	if (u.ualign.type == A_CHAOTIC)
		adjalign(1);
	/* by this point you have discovered mon's identity, blind or not... */
	if(!uclockwork){
		pline("Time stands still while you and %s lie in each other's arms...",
			noit_mon_nam(mon));
	}
	else{
		pline("You and %s lie down together...",
			noit_mon_nam(mon));
	}
	if (helpless || rn2(120) > ACURR(A_CHA) + ACURR(A_CON) + ACURR(A_INT)) {
		if(!uclockwork){
			pline("%s seems to have enjoyed it more than you...",
				noit_Monnam(mon));
		} else{
			pline("...but she becomes enraged when she discovers you're mechanical!");
			verbalize("How dare you trick me!");
			pline("She attacks your keyhole with her barbed tail!");
			losehp(d(4, 12), "an enraged demoness", KILLED_BY);
			morehungry(d(2,12)*10);
			pline("She claws your face!");
			losehp(d(4, 4), "an enraged demoness", KILLED_BY);
			(void) adjattrib(A_CHA, -1*d(2,4), TRUE);
			AMAX(A_CHA) = ABASE(A_CHA); //permanent drain!
			goto pay;
		}
		switch (rn2(8)) {
			case 0: You_feel("drained of energy.");
				u.uen = 0;
				u.uenmax -= Half_physical_damage ? 45 : 90;
					exercise(A_CON, FALSE);
				if (u.uenmax < 0) u.uenmax = 0;
				break;
			case 1: You("are down in the dumps.");
				u.uhpmax -= Half_physical_damage ? 25 : 50;
				if(u.uhpmax < 1) u.uhpmax = 1;
				if(u.uhp > u.uhpmax) u.uhp = u.uhpmax;
				(void) adjattrib(A_CON, -2, TRUE);
				(void) adjattrib(A_STR, -2, TRUE);
				if (diseasemu(mon->data)) You("seem to have caught a disease!"); 
					exercise(A_CON, FALSE);
				flags.botl = 1;
				break;
			case 2: Your("mind is dulled.");
				if(u.sealsActive&SEAL_HUGINN_MUNINN){
					unbind(SEAL_HUGINN_MUNINN,TRUE);
				} else {
					(void) adjattrib(A_INT, -3, TRUE);
					(void) adjattrib(A_WIS, -3, TRUE);
					forget_levels(30);
					forget_objects(30);
					exercise(A_WIS, FALSE);
					exercise(A_WIS, FALSE);
					exercise(A_WIS, FALSE);
				}
				flags.botl = 1;
				break;
			case 3:
				Your("pack feels heavier.");
				(void) adjattrib(A_STR, -2, TRUE);
				u.ucarinc -= 250;
			break;
			case 4: {
				int tmp;
				if (!Drain_resistance) {
					losexp("exhaustion",FALSE,FALSE,FALSE);
					losexp("exhaustion",TRUE,FALSE,FALSE);
				}
				You_feel("exhausted.");
					exercise(A_STR, FALSE);
				tmp = rn1(10, 6);
				if(Half_physical_damage) tmp = (tmp+1) / 2;
				losehp(tmp, "exhaustion", KILLED_BY);
				break;
			}
			case 5: {
				verbalize("If thou art as terrible a fighter as thou art a lover, death shall find you soon.");
				u.uacinc -= 10;
				u.udaminc -= 10;
				u.uhitinc -= 10;
			}
			break;
			case 6:
				You_feel("robbed... but your possessions are still here...?");
				attrcurse();
			break;
			case 7:
		    if (Levitation || Is_airlevel(&u.uz)||Is_waterlevel(&u.uz))
				You("are motionlessly suspended.");
#ifdef STEED
			else if (u.usteed)
				You("are frozen in place!");
#endif
			else
				You("are paralyzed!");
			pline("She has immobilized you with her magic!");
		    nomul(-(rn1(10, 25)), "immobilized by night-terrors");
		    nomovemsg = You_can_move_again;
		    exercise(A_DEX, FALSE);
			break;
		}
		return 1;
	} else {
		mon->mspec_used = rnd(39)+13;
		if(!uclockwork){
			You("seem to have enjoyed it more than %s...",
				noit_mon_nam(mon));
		} else{
			pline("She becomes very angry when she discovers your mechanical nature.");
			pline("She claws at you...");
			losehp(d(4, 4), "an angry paramour", KILLED_BY);
			pline("...but you manage to distract her before she does serious harm.");
			switch(rn2(4)){
				case 0:
						verbalize("Thou art wonderful! My favor shall protect you from harm!");
						/* Well, she's mixing thous and yous in these pronouncements, */
						/* But apparently she's ALSO overenthused enough to bless somebody who's fighting her, so... */
						u.uacinc += d(1,10);
				break;
				case 1:
						verbalize("I name you my champion. Go forth and slay thy enemies with my blessing!");
						u.udaminc += d(1,10);
						u.uhitinc += d(1,10);
				break;
				case 2:
						verbalize("Truly thou art as a fountain of life!");
						u.uhpmax = (int)(u.uhpmax*1.2);
						u.uenmax = (int)(u.uenmax*1.2);
				break;
				case 3:
						You_feel("as though you could lift mountains!");
						u.ucarinc += d(1,4)*50;
				break;
			}
			goto pay;
		}
		if(ufem && (ACURR(A_CHA) < rn2(35))){
			if(rn2(2) || uarmh){
				pline("She attacks you with her barbed tail!");
				losehp(d(4, 12), "a jealous demoness", KILLED_BY);
			}
			else{
				pline("She claws your face!");
				losehp(d(4, 4), "a jealous demoness", KILLED_BY);
				(void) adjattrib(A_CHA, -1*d(2,4), TRUE);
				AMAX(A_CHA) = ABASE(A_CHA); //permanent drain!
			}
		}
		else switch (rn2(8)) {
		case 0: 
			if(!(HPoison_resistance & FROMOUTSIDE)) {
				You_feel("healthy.");
				HPoison_resistance |= FROMOUTSIDE;
			}
			else pline("but that's about it.");
		break;
		case 1: You_feel("raised to your full potential.");
			(void) adjattrib(A_STR, 2, TRUE);
			(void) adjattrib(A_DEX, 2, TRUE);
			(void) adjattrib(A_CON, 2, TRUE);
			(void) adjattrib(A_INT, 2, TRUE);
			(void) adjattrib(A_WIS, 2, TRUE);
			(void) adjattrib(A_CHA, 2, TRUE);
			exercise(A_STR, TRUE);
			exercise(A_STR, TRUE);
			exercise(A_STR, TRUE);
			exercise(A_DEX, TRUE);
			exercise(A_DEX, TRUE);
			exercise(A_DEX, TRUE);
			exercise(A_CON, TRUE);
			exercise(A_CON, TRUE);
			exercise(A_CON, TRUE);
			exercise(A_INT, TRUE);
			exercise(A_INT, TRUE);
			exercise(A_INT, TRUE);
			exercise(A_WIS, TRUE);
			exercise(A_WIS, TRUE);
			exercise(A_WIS, TRUE);
			exercise(A_CHA, TRUE);
			exercise(A_CHA, TRUE);
			exercise(A_CHA, TRUE);
			flags.botl = 1;
			break;
		case 2:
			if(!(HAcid_resistance & FROMOUTSIDE)) {
				if(Hallucination) You("like you've gone back to the basics.");
				else Your("health seems insoluble.");
				HAcid_resistance |= FROMOUTSIDE;
			}
			else pline("but that's about it.");
		break;
		case 3:
			if(!(HSick_resistance & FROMOUTSIDE) && !rn2(4)) {
				You(Hallucination ? "feel alright." :
				    "feel healthier than you ever have before.");
				HSick_resistance |= FROMOUTSIDE;
			}
			else pline("but that's about it.");
		break;
		case 4:
				verbalize("Thou art wonderful! My favor shall protect you from harm!");
				/* Well, she's mixing thous and yous in these pronouncements, */
				/* But apparently she's ALSO overenthused enough to bless somebody who's fighting her, so... */
				u.uacinc += d(1,10);
		break;
		case 5:
				verbalize("I name you my champion. Go forth and slay thy enemies with my blessing!");
				u.udaminc += d(1,10);
				u.uhitinc += d(1,10);
		break;
		case 6:
				verbalize("Truly thou art as a fountain of life!");
				u.uhpmax = (int)(u.uhpmax*1.2);
				u.uenmax = (int)(u.uenmax*1.2);
		break;
		case 7:
				You_feel("as though you could lift mountains!");
				u.ucarinc += d(1,4)*50;
		break;
		}
	}
pay:
	if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
	return 1;
}

int
dograzseduce(mon)
register struct monst *mon;
{
	register struct obj *ring, *nring;
	boolean fem = FALSE;
	boolean ufem=poly_gender();
	char qbuf[QBUFSZ];
	char buf[QBUFSZ];
	boolean helpless = FALSE;
//	pline("starting mlc seduce");
	if (mon->mcan || mon->mspec_used) {
		pline("%s is uninterested in you.", Monnam(mon));
		return 0;
	}

	if (unconscious()) {
//		pline("%s seems annoyed at your lack of response.",
//			  Monnam(mon));
			You("are having a strange dream.");
			helpless = TRUE;
	}

	if (Blind) pline("It caresses you...");
	else You_feel("very attracted to %s.", mon_nam(mon));

	for(ring = invent; ring; ring = nring) {
		nring = ring->nobj;
		if (ring->otyp != RIN_ADORNMENT) continue;
		if (!ufem) {
		if (rn2(45) < ACURR(A_CHA)) {
			Sprintf(qbuf, "\"That %s looks pretty.  Give it to me.\"",
			safe_qbuf("",sizeof("\"That looks pretty.  Give it to me.\""),
			xname(ring), simple_typename(ring->otyp), "ring"));
			makeknown(RIN_ADORNMENT);
			if (yn(qbuf) == 'n') continue;
		} else pline("%s decides he'd like your %s, and takes it.",
			Blind ? "He" : Monnam(mon), xname(ring));
		makeknown(RIN_ADORNMENT);
		if (ring==uleft || ring==uright) Ring_gone(ring);
		if (ring==uwep) setuwep((struct obj *)0);
		if (ring==uswapwep) setuswapwep((struct obj *)0);
		if (ring==uquiver) setuqwep((struct obj *)0);
		freeinv(ring);
		(void) mpickobj(mon,ring);
		} else {
		char buf[BUFSZ];
		if (uleft && uright && uleft->otyp == RIN_ADORNMENT
				&& (uright->otyp==RIN_ADORNMENT || (uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)))
			break;
		if (ring==uleft || ring==uright) continue;
		
		pline("%s decides you'd look prettier wearing your %s,",
		Blind ? "He" : Monnam(mon), xname(ring));
		pline("and puts it on your finger.");
		
		makeknown(RIN_ADORNMENT);
		if (!uright && !(uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)) {
			pline("%s puts %s on your right %s.",
			Blind ? "He" : Monnam(mon), the(xname(ring)), body_part(HAND));
			setworn(ring, RIGHT_RING);
		} else if (!uleft) {
			pline("%s puts %s on your left %s.",
			Blind ? "He" : Monnam(mon), the(xname(ring)), body_part(HAND));
			setworn(ring, LEFT_RING);
		} else if (uright && uright->otyp != RIN_ADORNMENT && !(uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)) {
			Strcpy(buf, xname(uright));
			pline("%s replaces your %s with your %s.",
			Blind ? "He" : Monnam(mon), buf, xname(ring));
			Ring_gone(uright);
			setworn(ring, RIGHT_RING);
		} else if (uleft && uleft->otyp != RIN_ADORNMENT) {
			Strcpy(buf, xname(uleft));
			pline("%s replaces your %s with your %s.",
			Blind ? "He" : Monnam(mon), buf, xname(ring));
			Ring_gone(uleft);
			setworn(ring, LEFT_RING);
		} else impossible("ring replacement");
		Ring_on(ring);
		prinv((char *)0, ring, 0L);
		}
	}

	if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh
#ifdef TOURIST
								&& !uarmu
#endif
									)
		pline("%s caresses your body.",
			Blind ? "He" : Monnam(mon));
	else
		pline("%s starts undressing you.",
			Blind ? "He" : Monnam(mon));
	mlcmayberem(uarmc, cloak_simple_name(uarmc), helpless);
	if(!uarmc)
		mlcmayberem(uarm, "suit", helpless);
/*	mlcmayberem(uarmf, "boots", helpless); */
	if(!uwep || !welded(uwep))
		mlcmayberem(uarmg, "gloves", helpless);
	mlcmayberem(uarms, "shield", helpless);
/*	mlcmayberem(uarmh, "helmet", helpless); */
#ifdef TOURIST
	if(!uarmc && !uarm)
		mlcmayberem(uarmu, "shirt", helpless);
#endif

	if (uarm || uarmc || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)) {
		verbalize("How dare you refuse me!");
		pline("He punches you!");
		losehp(d(3, 8), "an enraged paramour", KILLED_BY);
		if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
		return 1;
	}
	if (u.ualign.type == A_CHAOTIC)
		adjalign(1);
	/* by this point you have discovered mon's identity, blind or not... */
	if(!uclockwork){
		pline("Time stands still while you and %s lie in each other's arms...",
			noit_mon_nam(mon));
	}
	else{
		pline("You and %s lie down together...",
			noit_mon_nam(mon));
	}
	if (helpless || rn2(120) > ACURR(A_CHA) + ACURR(A_CON) + ACURR(A_INT)) {
		struct obj *optr;
		if(!uclockwork){
			pline("%s seems to have enjoyed it more than you...",
				noit_Monnam(mon));
		} else{
			pline("...he becomes enraged when he discovers you're mechanical!");
			verbalize("How dare you trick me!");
			pline("He viciously bites you!");
			losehp(d(4, 8), "an enraged demon prince", KILLED_BY);
			(void) adjattrib(A_CHA, -1*d(1,8), TRUE);
			AMAX(A_CHA) = ABASE(A_CHA); //permanent drain!
			pline("He drips acid into your inner workings!");
			if(!HAcid_resistance){
				losehp(d(6, 8), "an enraged demon prince", KILLED_BY);
				morehungry(d(3,8));
			}
		}
		switch (rn2(6)) {
			case 0: verbalize("Surely you don't need all this junk?!");
				buf[0] = '\0';
				steal(mon, buf,FALSE);
				buf[0] = '\0';
				steal(mon, buf,FALSE);
				buf[0] = '\0';
				steal(mon, buf,FALSE);
				buf[0] = '\0';
				steal(mon, buf,FALSE);
				buf[0] = '\0';
				steal(mon, buf,FALSE);
				buf[0] = '\0';
				steal(mon, buf,FALSE);
			break;
			case 1:
				if(u.twoweap){
					verbalize("You're going to hurt yourself with those.");
					u.twoweap = FALSE;
					optr = uswapwep;
					setuswapwep((struct obj *)0);
					freeinv(optr);
					(void) mpickobj(mon,optr);

					optr = uwep;
					setuwep((struct obj *)0);
					freeinv(optr);
					(void) mpickobj(mon,optr);
				}
				else if(uwep && !(uwep->otyp != IRON_BAR)){
					verbalize("You're going to hurt yourself with that.");
					optr = uwep;
					setuwep((struct obj *)0);
					freeinv(optr);
					(void) mpickobj(mon,optr);
				}
				if(!uwep){
					buf[0] = '\0';
					steal(mon, buf,FALSE);
					optr = mksobj(IRON_BAR, TRUE, FALSE);		
					curse(optr);
					optr->spe = -6;
					verbalize("This will keep you out of trouble.");
					(void) hold_another_object(optr, u.uswallow ?
							   "Fortunately, you're out of reach! %s away." :
							   "Fortunately, you can't hold anything more! %s away.",
							   The(aobjnam(optr,
								 Is_airlevel(&u.uz) || u.uinwater ?
								   "slip" : "drop")),
							   (const char *)0);
					if(carried(optr)){
						setuwep(optr);
					}
				} else{
					verbalize("You're so helpless!");
						losexp("dark speach",FALSE,TRUE,FALSE);
				}
			break;
			case 2:
				if(uarmh && uarmh->otyp != DUNCE_CAP){
					Helmet_off();
				}
				if(!uarmh){
					verbalize("This should greatly improve your intellect.");
					buf[0] = '\0';
					steal(mon, buf,FALSE);
					optr = mksobj(DUNCE_CAP, TRUE, FALSE);		
					curse(optr);
					optr->spe = -6;
					(void) hold_another_object(optr, u.uswallow ?
							   "Fortunately, you're out of reach! %s away." :
							   "Fortunately, you can't hold anything more! %s away.",
							   The(aobjnam(optr,
								 Is_airlevel(&u.uz) || u.uinwater ?
								   "slip" : "drop")),
							   (const char *)0);
					if(carried(optr)){
						setworn(optr, W_ARMH);
						Helmet_on();
					}
				}
				else{
					verbalize("You're so stupid!");
					losexp("dark speach",FALSE,TRUE,FALSE);
				}
				
			break;
			case 3:
				if(uarmf && uarmf->otyp != FUMBLE_BOOTS){
					Boots_off();
				}
				if(!uarmf){
					verbalize("These boots will improve your looks.");
					buf[0] = '\0';
					steal(mon, buf,FALSE);
					optr = mksobj(FUMBLE_BOOTS, TRUE, FALSE);		
					curse(optr);
					optr->spe = -6;
					(void) hold_another_object(optr, u.uswallow ?
							   "Fortunately, you're out of reach! %s away." :
							   "Fortunately, you can't hold anything more! %s away.",
							   The(aobjnam(optr,
								 Is_airlevel(&u.uz) || u.uinwater ?
								   "slip" : "drop")),
							   (const char *)0);
					if(carried(optr)){
						setworn(optr, W_ARMF);
						Boots_on();
					}
				}
				else{
					verbalize("You're so clumsy!");
					losexp("dark speach",FALSE,TRUE,FALSE);
				}
			break;
			case 4:
				if(uamul && uamul->otyp != AMULET_OF_RESTFUL_SLEEP ){
					Amulet_off();
				}
				if(!uamul){
					verbalize("You need to take things more slowly.");
					buf[0] = '\0';
					steal(mon, buf,FALSE);
					optr = mksobj(AMULET_OF_RESTFUL_SLEEP, TRUE, FALSE);		
					curse(optr);
					(void) hold_another_object(optr, u.uswallow ?
							   "Fortunately, you're out of reach! %s away." :
							   "Fortunately, you can't hold anything more! %s away.",
							   The(aobjnam(optr,
								 Is_airlevel(&u.uz) || u.uinwater ?
								   "slip" : "drop")),
							   (const char *)0);
					if(carried(optr)){
						setworn(optr, W_AMUL);
						Amulet_on();
					}
				}
				else{
					verbalize("You're so lazy!");
					losexp("dark speach",FALSE,TRUE,FALSE);
				}
			break;
			case 5:
			    punish((struct obj *)0);
			    punish((struct obj *)0);
			    punish((struct obj *)0);
				verbalize("Stay here.");
			break;
		}
		return 1;
	} else {
		struct obj *optr;
		mon->mspec_used = rnd(39)+13;
		You("seem to have enjoyed it more than %s...",
			noit_mon_nam(mon));
		if(!ufem && (ACURR(A_CHA) < rn2(35))){
			if(rn2(2) || uarmh || HAcid_resistance){
				pline("He viciously bites you!");
				losehp(d(4, 8), "a jealous demon prince", KILLED_BY);
			}
			else{
				pline("He drips acid on your face!");
				losehp(d(6, 8), "a jealous demon prince", KILLED_BY);
				(void) adjattrib(A_CHA, -1*d(1,8), TRUE);
				AMAX(A_CHA) = ABASE(A_CHA); //permanent drain!
			}
		}
		else switch (rn2(6)) {
		case 0: 
			verbalize("Tell me your greatest desire!");
			makewish();
		break;
		case 1:
			verbalize("Tell me, whom shall I kill?");
			do_genocide(0);
		break;
		case 2:{
		   int i = 6;
			verbalize("I grant you six magics!");
		   for(i;i>0;i--){
				/* IMPROVEMENT NOTE: Randomize order, also, you will never
					reach this code while wearing armor or a cloak. */
				if(uwep && uwep->spe < 6) uwep->spe = 6;
				else if(uarm && uarm->spe < 6) uarm->spe = 6;
				else if(uarmc && uarmc->spe < 6) uarmc->spe = 6;
				else if(uarms && uarms->spe < 6) uarms->spe = 6;
				else if(uswapwep && uswapwep->spe < 6) uswapwep->spe = 6;
				else if(uarmh && uarmh->spe < 6) uarmh->spe = 6;
				else if(uarmg && uarmg->spe < 6) uarmg->spe = 6;
				else if(uarmf && uarmf->spe < 6) uarmf->spe = 6;
				else if(uquiver && uquiver->spe < 6) uquiver->spe = 6;
				else if(uarmu && uarmu->spe < 6) uarmu->spe = 6;
		   }
		}break;
		case 3:{
		   int i = 6;
			verbalize("I grant you six truths!");
		   for(i;i>0;i--){
			optr = mksobj(POT_ENLIGHTENMENT, TRUE, FALSE);		
			bless(optr);
			(void) hold_another_object(optr, u.uswallow ?
				       "Oops!  %s out of your reach!" :
				       (Is_airlevel(&u.uz) ||
					Is_waterlevel(&u.uz) ||
					levl[u.ux][u.uy].typ < IRONBARS ||
					levl[u.ux][u.uy].typ >= ICE) ?
				       "Oops!  %s away from you!" :
				       "Oops!  %s to the floor!",
				       The(aobjnam(optr,
					     Is_airlevel(&u.uz) || u.uinwater ?
						   "slip" : "drop")),
				       (const char *)0);
		   }
		}break;
		case 4:
			verbalize("I grant you life!");
			optr = mksobj(AMULET_OF_LIFE_SAVING, TRUE, FALSE);		
			bless(optr);
			(void) hold_another_object(optr, u.uswallow ?
				       "Oops!  %s out of your reach!" :
				       (Is_airlevel(&u.uz) ||
					Is_waterlevel(&u.uz) ||
					levl[u.ux][u.uy].typ < IRONBARS ||
					levl[u.ux][u.uy].typ >= ICE) ?
				       "Oops!  %s away from you!" :
				       "Oops!  %s to the floor!",
				       The(aobjnam(optr,
					     Is_airlevel(&u.uz) || u.uinwater ?
						   "slip" : "drop")),
				       (const char *)0);
			if(carried(optr)){
				if(!uamul){
					setworn(optr, W_AMUL);
					Amulet_on();
				}
			}
		break;
		case 5:{
		   int i = 6;
			verbalize("I grant you six followers!");
		   for(i;i>0;i--){
			optr = mksobj(FIGURINE, TRUE, FALSE);		
			bless(optr);
			(void) hold_another_object(optr, u.uswallow ?
				       "Oops!  %s out of your reach!" :
				       (Is_airlevel(&u.uz) ||
					Is_waterlevel(&u.uz) ||
					levl[u.ux][u.uy].typ < IRONBARS ||
					levl[u.ux][u.uy].typ >= ICE) ?
				       "Oops!  %s away from you!" :
				       "Oops!  %s to the floor!",
				       The(aobjnam(optr,
					     Is_airlevel(&u.uz) || u.uinwater ?
						   "slip" : "drop")),
				       (const char *)0);
		   }
		}
		break;
		}
	}
	if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
	return 1;
}

int
dosflseduce(mon)
register struct monst *mon;
{
	boolean fem = TRUE;
	//char qbuf[QBUFSZ];
	boolean helpless = FALSE;
	if (mon->mcan || mon->mspec_used) {
		pline("%s is uninterested in you.", Monnam(mon));
		return 0;
	}

	if (unconscious()) {
			You("are having a horrible dream.");
			helpless = TRUE;
	}

	if (Blind) You_feel("Something grab you...");
	else pline("%s grabs you.", mon_nam(mon));


	if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh
#ifdef TOURIST
								&& !uarmu
#endif
									)
		pline("%s stares at you.",
			Blind ? (fem ? "She" : "He") : Monnam(mon));
	else
		pline("%s growels into your ear, while tearing at your clothing.",
			Blind ? (fem ? "She" : "He") : Monnam(mon));
	sflmayberem(uarmc, cloak_simple_name(uarmc), helpless);
	if(!uarmc)
		sflmayberem(uarm, "suit", helpless);
	sflmayberem(uarmf, "boots", helpless);
	if(!uwep || !welded(uwep))
		sflmayberem(uarmg, "gloves", helpless);
	sflmayberem(uarms, "shield", helpless);
	sflmayberem(uarmh, "helmet", helpless);
#ifdef TOURIST
	if(!uarmc && !uarm)
		sflmayberem(uarmu, "shirt", helpless);
#endif

	if (uarm || uarmc || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)) {
		verbalize("You can't resist forever!");
		pline("She claws at you!");
		losehp(d(4, 10), "a jilted paramour", KILLED_BY);
		return 0;
	}
	if (u.ualign.type == A_CHAOTIC)
		adjalign(1);
	/* by this point you have discovered mon's identity, blind or not... */
	pline("Time stands still while you and %s lie in each other's arms...",
		noit_mon_nam(mon));
	pline("Suddenly, %s becomes violent!",
		noit_Monnam(mon));
	if (helpless || 25 + rn2(100) > ACURR(A_CHA) + ACURR(A_STR)) {
		int turns = d(1, 4);
		if(!helpless) You("are taken off guard!");
		nomul(-(turns), "knocked reeling by a violent lover");
		nomovemsg = You_can_move_again;
		mon->mspec_used = turns;
		return 0;
	} else {
		pline("But you gain the upper hand!",
			noit_Monnam(mon));
	    mon->mcanmove = 0;
	    mon->mfrozen = d(1,4)+1;
	    return 3;
	}
	return 0;
}

int
dopaleseduce(mon)
register struct monst *mon;
{
	register struct obj *ring, *nring;
	boolean fem = !poly_gender(); /* male = 0, fem = 1, neuter = 2 */
	boolean helpless = FALSE;
	char qbuf[QBUFSZ];
	
	if(!monnear(mon, u.ux,u.uy)) return 0;
	
	if((ward_at(u.ux,u.uy) == ELDER_SIGN && num_wards_at(u.ux, u.uy) == 6) || 
		(ward_at(u.ux,u.uy) == ELDER_ELEMENTAL_EYE && num_wards_at(u.ux, u.uy) == 7) || 
		ward_at(u.ux,u.uy) == PENTAGRAM || ward_at(u.ux,u.uy) == HEPTAGRAM ||
		ward_at(u.ux,u.uy) == HEXAGRAM || 
		(scaryElb(mon) && sengr_at("Elbereth", u.ux, u.uy)) || 
		(scaryLol(mon) && sengr_at("Lolth", u.ux, u.uy))
	) return 0;
	
	if (unconscious()) {/*Note: is probably not going to be possible to be unconscious and enter this function*/
		You("are having a horrible dream.");
		boolean helpless = TRUE;
	}

	if(mon->mvar1 == 1){
		if (Blind) You_feel("cloth against your %s...",body_part(BODY_SKIN));
		else{
			pline("The shroud dances as if in the wind. The %s figure beneath is almost exposed!", fem ? "shapely feminine" : "shapely masculine");
			You_feel("very attracted to %s.", mon_nam(mon));
		}
	}
	else{
		mon->mvar1 = 1;
		if (Blind) You_feel("the brush of cloth...");
		else{
			You("see a %s form behind the shroud. It beckons you forwards.", fem ? "lithe, feminine," : "toned, masculine,");
			if(rnd(10) + ACURR(A_WIS) - 10 > 6) You_feel("that it would be wise to stay away.");
		}
		return 0;
	}

	if (rn2(66) > 2*ACURR(A_WIS) - ACURR(A_INT) || helpless) {
		int lifesaved = 0;
		int wdmg = (int)(d(1,10)) + 1;
		palemayberem(uarmc, cloak_simple_name(uarmc), helpless);
		if(!uarmc)
			palemayberem(uarm, "suit", helpless);
		palemayberem(uarmf, "boots", helpless);
		palemayberem(uarmg, "gloves", helpless);
		palemayberem(uarms, "shield", helpless);
		palemayberem(uarmh, "helmet", helpless);
	#ifdef TOURIST
		if(!uarmc && !uarm)
			palemayberem(uarmu, "shirt", helpless);
	#endif
		You("move to embrace %s, brushing aside the gossamer shroud hiding %s body from you.",
			noit_Monnam(mon), fem ? "her" : "his");
		if(rn2( (int)(ACURR(A_WIS)/2))){
			boolean loopingDeath = TRUE;
			struct obj *wore_amulet = uamul;
			while(loopingDeath) {
				if (lifesaved){
					pline("There is something horrible lurking in your memory... the mere thought of it is consuming your mind from within!");
				}
				else{
					pline("As you pass through the shroud, your every sense goes mad.");
					Your("whole world becomes an unbearable symphony of agony.");
				}
				killer = "seeing something not meant for mortal eyes";
				killer_format = KILLED_BY;
				done(DIED);
				lifesaved++;
				/* avoid looping on "die(y/n)?" */
				if (lifesaved && (discover || wizard)) {
					if (wore_amulet && !uamul) {
						/* used up AMULET_OF_LIFE_SAVING; still
						   subject to dying from memory */
						wore_amulet = 0;
						if(rn2( (int)(ACURR(A_WIS)/2)) < 4) loopingDeath = FALSE;
					} else {
						/* explicitly chose not to die */
						loopingDeath = FALSE;
					}
				}
			}
		}
		You("find yourself staggering away from %s, with no memory of why.", fem ? "her" : "him");
		make_stunned(HStun + 12, TRUE);
		while( ABASE(A_WIS) > ATTRMIN(A_WIS) && wdmg > 0){
			wdmg--;
			(void) adjattrib(A_WIS, -1, TRUE);
			exercise(A_WIS, FALSE);
		}
		if(u.sealsActive&SEAL_HUGINN_MUNINN){
			unbind(SEAL_HUGINN_MUNINN,TRUE);
		} else {
			if(AMAX(A_WIS) > ABASE(A_WIS)) AMAX(A_WIS) = (int)((AMAX(A_WIS) - ABASE(A_WIS))/2 + 1); //permanently drain wisdom
			forget_levels(25);	/* lose memory of 25% of levels */
			forget_objects(25);	/* lose memory of 25% of objects */
		}
	} else {
		You("hang back from the %s form beneath the shroud. It poses enticingly.", fem ? "voluptuous feminine" : "muscular masculine");
	}
	return 1;
}

int
dotemplateseduce(mon)
register struct monst *mon;
{
	register struct obj *ring, *nring;
	boolean fem = (mon->data == &mons[PM_SUCCUBUS]) || mon->data == &mons[PM_CARMILLA]; /* otherwise incubus */
	char qbuf[QBUFSZ];
	struct obj *key;
	int turns = 0;
	char class_list[MAXOCLASSES+2];
//	pline("starting ssex");
	if (mon->mcan || mon->mspec_used) {
		pline("%s acts as though %s has got a %sheadache.",
			  Monnam(mon), mhe(mon),
			  mon->mcan ? "severe " : "");
		return 0;
	}

	if (unconscious()) {
		pline("%s seems dismayed at your lack of response.",
			  Monnam(mon));
		return 0;
	}

	if (Blind) pline("It caresses you...");
	else You_feel("very attracted to %s.", mon_nam(mon));

	for(ring = invent; ring; ring = nring) {
		nring = ring->nobj;
		if (ring->otyp != RIN_ADORNMENT) continue;
		if (fem) {
		if (rn2(20) < ACURR(A_CHA)) {
			Sprintf(qbuf, "\"That %s looks pretty.  May I have it?\"",
			safe_qbuf("",sizeof("\"That  looks pretty.  May I have it?\""),
			xname(ring), simple_typename(ring->otyp), "ring"));
			makeknown(RIN_ADORNMENT);
			if (yn(qbuf) == 'n') continue;
		} else pline("%s decides she'd like your %s, and takes it.",
			Blind ? "She" : Monnam(mon), xname(ring));
		makeknown(RIN_ADORNMENT);
		if (ring==uleft || ring==uright) Ring_gone(ring);
		if (ring==uwep) setuwep((struct obj *)0);
		if (ring==uswapwep) setuswapwep((struct obj *)0);
		if (ring==uquiver) setuqwep((struct obj *)0);
		freeinv(ring);
		(void) mpickobj(mon,ring);
		} else {
		char buf[BUFSZ];

		if (uleft && uright && uleft->otyp == RIN_ADORNMENT
				&& (uright->otyp==RIN_ADORNMENT || (uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)))
			break;
		if (ring==uleft || ring==uright) continue;
		if (rn2(20) < ACURR(A_CHA)) {
			Sprintf(qbuf,"\"That %s looks pretty.  Would you wear it for me?\"",
			safe_qbuf("",
				sizeof("\"That  looks pretty.  Would you wear it for me?\""),
				xname(ring), simple_typename(ring->otyp), "ring"));
			makeknown(RIN_ADORNMENT);
			if (yn(qbuf) == 'n') continue;
		} else {
			pline("%s decides you'd look prettier wearing your %s,",
			Blind ? "He" : Monnam(mon), xname(ring));
			pline("and puts it on your finger.");
		}
		makeknown(RIN_ADORNMENT);
		if (!uright && !(uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)) {
			pline("%s puts %s on your right %s.",
			Blind ? "He" : Monnam(mon), the(xname(ring)), body_part(HAND));
			setworn(ring, RIGHT_RING);
		} else if (!uleft) {
			pline("%s puts %s on your left %s.",
			Blind ? "He" : Monnam(mon), the(xname(ring)), body_part(HAND));
			setworn(ring, LEFT_RING);
		} else if (uright && uright->otyp != RIN_ADORNMENT && !(uarmg && uarmg->oartifact == ART_CLAWS_OF_THE_REVENANCER)) {
			Strcpy(buf, xname(uright));
			pline("%s replaces your %s with your %s.",
			Blind ? "He" : Monnam(mon), buf, xname(ring));
			Ring_gone(uright);
			setworn(ring, RIGHT_RING);
		} else if (uleft && uleft->otyp != RIN_ADORNMENT) {
			Strcpy(buf, xname(uleft));
			pline("%s replaces your %s with your %s.",
			Blind ? "He" : Monnam(mon), buf, xname(ring));
			Ring_gone(uleft);
			setworn(ring, LEFT_RING);
		} else impossible("ring replacement");
		Ring_on(ring);
		prinv((char *)0, ring, 0L);
		}
	}

	if (!uarmc && !uarmf && !uarmg && !uarms && !uarmh
#ifdef TOURIST
								&& !uarmu
#endif
									)
		pline("%s murmurs sweet nothings into your ear.",
			Blind ? (fem ? "She" : "He") : Monnam(mon));
	else
		pline("%s murmurs in your ear, while helping you undress.",
			Blind ? (fem ? "She" : "He") : Monnam(mon));
	mayberem(uarmc, cloak_simple_name(uarmc));
	if(!uarmc)
		mayberem(uarm, "suit");
	mayberem(uarmf, "boots");
	if(!uwep || !welded(uwep))
		mayberem(uarmg, "gloves");
	mayberem(uarms, "shield");
	mayberem(uarmh, "helmet");
#ifdef TOURIST
	if(!uarmc && !uarm)
		mayberem(uarmu, "shirt");
#endif

	if (uarm || uarmc) {
		verbalize("You're such a %s; I wish...",
				flags.female ? "sweet lady" : "nice guy");
		if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
		return 1;
	}
	if (u.ualign.type == A_CHAOTIC)
		adjalign(1);

	/* by this point you have discovered mon's identity, blind or not... */
	if(!uclockwork){
		pline("Time stands still while you and %s lie in each other's arms...",
			noit_mon_nam(mon));
	}
	else{
		pline("You and %s lie down together...",
			noit_mon_nam(mon));
	}
	if (rn2(35) > ACURR(A_CHA) + ACURR(A_INT)) {
		/* Don't bother with mspec_used here... it didn't get tired! */
		if(!uclockwork){
			pline("%s seems to have enjoyed it more than you...",
				noit_Monnam(mon));
		} else{
			char buf[BUFSZ];
			pline("%s looks briefly confused...",
				noit_Monnam(mon));
			if(!rn2(5) && !Drain_resistance){
				pline("...then tries to suck out your soul with a kiss!");
				losexp("stolen soul",FALSE,FALSE,FALSE);
			}
			else {
				buf[0] = '\0';
				steal(mon, buf,FALSE);
			}
			goto pay;
		}
		switch (rn2(5)) {
			case 0: You_feel("drained of energy.");
				u.uen = 0;
				u.uenmax -= rnd(Half_physical_damage ? 5 : 10);
					exercise(A_CON, FALSE);
				if (u.uenmax < 0) u.uenmax = 0;
				break;
			case 1: You("are down in the dumps.");
				(void) adjattrib(A_CON, -1, TRUE);
					exercise(A_CON, FALSE);
				flags.botl = 1;
				break;
			case 2: Your("senses are dulled.");
				(void) adjattrib(A_WIS, -1, TRUE);
					exercise(A_WIS, FALSE);
				flags.botl = 1;
				break;
			case 3:
				if (!Drain_resistance) {
					You_feel("out of shape.");
					losexp("overexertion",TRUE,FALSE,FALSE);
				} else {
					You("have a curious feeling...");
				}
				break;
			case 4: {
				int tmp;
				You_feel("exhausted.");
					exercise(A_STR, FALSE);
				tmp = rn1(10, 6);
				if(Half_physical_damage) tmp = (tmp+1) / 2;
				losehp(tmp, "exhaustion", KILLED_BY);
				break;
			}
/*				boolean not_affected=0;
				int tmp = d(6,6);
				You_feel("as though you are loosing your mind!");
				not_affected |= Blind ||
					(u.umonnum == PM_BLACK_LIGHT ||
					 u.umonnum == PM_VIOLET_FUNGUS ||
					 dmgtype(youmonst.data, AD_HALU));
				if (!not_affected) {
				    boolean chg;
				    chg = make_hallucinated(HHallucination + (long)tmp,FALSE,0L);
				    You("%s.", chg ? "are freaked out" : "seem unaffected");
				}
				(void) adjattrib(A_INT, -1, FALSE);
				forget_levels(5);
				forget_objects(5);
				exercise(A_WIS, FALSE);
				exercise(A_WIS, FALSE);
				exercise(A_INT, FALSE);
				exercise(A_INT, FALSE);
*/		}
	} else {
		mon->mspec_used = rnd(100); /* monster is worn out */
		if(!uclockwork){
			You("seem to have enjoyed it more than %s...",
				noit_mon_nam(mon));
		} else{
			pline("Time stands still while you and %s lie in each other's arms...",
				noit_mon_nam(mon));
			if(!rn2(5)){
				pline("That was a very educational experience.");
				pluslvl(FALSE);
				goto pay;
			} else if(u.uhunger < .5*u.uhungermax && !Race_if(PM_INCANTIFIER)){
				You("persuade %s to wind your clockwork.",
					noit_mon_nam(mon));
				struct obj *key;
				int turns = 0;
				
				Strcpy(class_list, tools);
				key = getobj(class_list, "wind with");
				if (!key){
					pline(Never_mind);
					goto pay;
				}
				turns = ask_turns(mon, 0, 0);
				if(!turns){
					pline(Never_mind);
					goto pay;
				}
				turns = (.8 + ((double)rn2(5))) * (turns);
				lesshungry(turns*10);
				You("notice %s wound your clockwork %d times.",noit_mon_nam(mon),turns);
				goto pay;
			} else {
				pline("%s looks happy, but confused.",
					noit_Monnam(mon));
				goto pay;
			}
		}
		switch (rn2(5)) {
		case 0: You_feel("raised to your full potential.");
			exercise(A_CON, TRUE);
			u.uen = (u.uenmax += rnd(5));
			break;
		case 1: You_feel("good enough to do it again.");
			(void) adjattrib(A_CON, 1, TRUE);
			exercise(A_CON, TRUE);
			flags.botl = 1;
			break;
		case 2: You("will always remember %s...", noit_mon_nam(mon));
			(void) adjattrib(A_WIS, 1, TRUE);
			exercise(A_WIS, TRUE);
			flags.botl = 1;
			break;
		case 3: pline("That was a very educational experience.");
			pluslvl(FALSE);
			exercise(A_WIS, TRUE);
			break;
		case 4: You_feel("restored to health!");
			u.uhp = u.uhpmax;
			if (Upolyd) u.mh = u.mhmax;
			exercise(A_STR, TRUE);
			flags.botl = 1;
			break;
		}
	}
pay:
	return 1;
}

int
dotent(mon,dmg)
register struct monst *mon;
int dmg;
{
	char buf[BUFSZ];
	register struct obj *otmp;
	int i; //multipurpose local variable
	int n, ln, trycount; //loop control variables for attacks;
	int allreadydone = 0; //don't repeat the same special case;
	struct attack bodyblow = {AT_TENT, AD_WRAP, 2, 10};
	struct attack headshot = {AT_TENT, AD_DRIN, 2, 10};
	struct attack handshit = {AT_TENT, AD_DRDX, 2, 10};
	struct attack legblast = {AT_TENT, AD_LEGS, 2, 10};

	n = 4; //4 actions
	ln = n;
	trycount = 0;

/* First it makes one attempt to remove body armor.  It starts with the cloak,
 * followed by body armor and then the shirt.  It can only remove one per round.
 * After attempting to remove armor, it chooses random targets for the rest of its attacks.
 * These attacks are likely to be useless untill it gets rid of some armor.
 */
	#ifdef TOURIST
		if(!uarmc && !uarm && (!uwep || uwep->oartifact!=ART_TENSA_ZANGETSU))
			if(uarmu && n){
				n--;
				if(!u_slip_free(mon, &bodyblow)){
					You_feel("the tentacles squirm under your shirt.");
					if( d(1,100) > 15){
						pline("The tentacles begin to tear at your shirt!");
						 if(uarmu->spe > 1){
							int i = rn2(4);
							for(i; i>=0; i--)
								drain_item(uarmu);
							Your("%s less effective.", aobjnam(uarmu, "seem"));
						 }
						 else{
							tent_destroy_arm(uarmu);
						 }
					}
					else{
						pline("The tentacles pull your shirt off!");
						otmp = uarmu;
						if (donning(otmp)) cancel_don();
						(void) Shirt_off();
						freeinv(otmp);
						(void) mpickobj(mon,otmp);
					}
				}
			}
			
#endif
		if(!uarmc){
		 if(uwep && uwep->oartifact==ART_TENSA_ZANGETSU){
			You_feel("the tentacles tear uselessly at your regenerating shihakusho.");
		 }
		 else if(uarm && n){
			 n--;
			 if(!u_slip_free(mon, &bodyblow)){
				You_feel("the tentacles squirm under your armor.");
				if( d(1,100) > 25){
					pline("The tentacles begin to tear at your armor!");
					 if(uarm->spe > 1){
						int i = rn2(4);
						for(i; i>=0; i--)
							drain_item(uarm);
						Your("%s less effective.", aobjnam(uarm, "seem"));
					 }
					 else{
						tent_destroy_arm(uarm);
					 }
				}
				else{
					pline("The tentacles shuck you out of your armor!");
					otmp = uarm;
					if (donning(otmp)) cancel_don();
					(void) Armor_gone();
					freeinv(otmp);
					(void) mpickobj(mon,otmp);
				}
		  }
		 }
		}
		if(uarmc && n){
			n--;
			if(!u_slip_free(mon, &bodyblow)){
				You_feel("the tentacles work their way under your cloak.");
				if( d(1,100) > 66){
					pline("The tentacles begin to tear at the cloak!");
					 if(uarmc->spe > 1){
						int i = rn2(4);
						for(i; i>=0; i--)
							drain_item(uarmc);
						Your("%s less effective.", aobjnam(uarmc, "seem"));
					 }
					 else{
						tent_destroy_arm(uarmc);
					 }
				}
				else{
					pline("The tentacles strip off your cloak!");
					otmp = uarmc;
					if (donning(otmp)) cancel_don();
					(void) Cloak_off();
					freeinv(otmp);
					(void) mpickobj(mon,otmp);
				}
			}
		}
	  while(n > 0 && trycount++ < 50){
		   if(n < ln && (d(1,100) > 85)){ //it's useless to struggle, but...
			   yn("Struggle against the tentacles' grasp?");
			   ln = n;
		   }
		   switch(d(1,12)){
			case 1:
			if(allreadydone&(0x1<<1)) break;
			allreadydone |= 0x1<<1;
			if(uarmf){
				n--;
				if(!u_slip_free(mon, &legblast)){
					You_feel("the tentacles squirm into your boots.");
					if( d(1,100) > 66){
						pline("The tentacles begin to tear at your boots!");
						 if(uarmf->spe > 1){
							int i = rn2(4);
							for(i; i>=0; i--)
								drain_item(uarmf);
							Your("%s less effective.", aobjnam(uarmf, "seem"));
						 }
						 else{
							tent_destroy_arm(uarmf);
						 }
					}
					else{
						static int bboots2 = 0;
						if (!bboots2) bboots2 = find_bboots();
						if(uarmf->otyp != bboots2){
							pline("The tentacles suck off your boots!");
							otmp = uarmf;
							if (donning(otmp)) cancel_don();
							(void) Boots_off();
							freeinv(otmp);
							(void) mpickobj(mon,otmp);
						}
					}
				}
			}
			break;
			case 2:
			if(allreadydone&(0x1<<2)) break;
			allreadydone |= 0x1<<2;
			if(uwep){
				n--;
				You_feel("the tentacles wrap around your weapon.");
				if( d(1,130) > ACURR(A_STR)){
					pline("The tentacles yank your weapon out of your grasp!");
					otmp = uwep;
					uwepgone();
					freeinv(otmp);
					(void) mpickobj(mon,otmp);
				 }
				 else{
					You("keep a tight grip on your weapon!");
				 }
			}
			break;
			case 3:
			if(allreadydone&(0x1<<3)) break;
			allreadydone |= 0x1<<3;
			if(uarmg){
				n--;
				if(!u_slip_free(mon, &handshit)){
					You_feel("the tentacles squirm into your gloves.");
					if( d(1,40) <= ACURR(A_STR) || uwep){
						pline("The tentacles begin to tear at your gloves!");
						 if(uarmg->spe > 1){
							int i = rn2(4);
							for(i; i>=0; i--)
								drain_item(uarmg);
							Your("%s less effective.", aobjnam(uarmg, "seem"));
						 }
						 else{
							tent_destroy_arm(uarmg);
						 }
					}
					else{
						pline("The tentacles suck your gloves off!");
						otmp = uarmg;
						if (donning(otmp)) cancel_don();
						(void) Gloves_off();
						freeinv(otmp);
						(void) mpickobj(mon,otmp);
					}
				}
			}
			break;
			case 4:
			if(allreadydone&(0x1<<4)) break;
			allreadydone |= 0x1<<4;
			if(uarms){
				n--;
				You_feel("the tentacles wrap around your shield.");
				if( d(1,150) > ACURR(A_STR)){
					pline("The tentacles pull your shield out of your grasp!");
					otmp = uarms;
					if (donning(otmp)) cancel_don();
					Shield_off();
					freeinv(otmp);
					(void) mpickobj(mon,otmp);
				 }
				 else{
					You("keep a tight grip on your shield!");
				 }
			}
			break;
			case 5:
			if(allreadydone&(0x1<<5)) break;
			allreadydone |= 0x1<<5;
			if(uarmh){
				n--;
				if(!u_slip_free(mon, &headshot)){
					You_feel("the tentacles squirm under your helmet.");
					if( d(1,100) > 90){
						pline("The tentacles begin to tear at your helmet!");
						 if(uarmh->spe > 1){
							int i = rn2(4);
							for(i; i>=0; i--)
								drain_item(uarmh);
							Your("%s less effective.", aobjnam(uarmh, "seem"));
						 }
						 else{
							tent_destroy_arm(uarmh);
						 }
					}
					else{
						pline("The tentacles pull your helmet off!");
						otmp = uarmh;
						if (donning(otmp)) cancel_don();
						(void) Helmet_off();
						freeinv(otmp);
						(void) mpickobj(mon,otmp);
					}
				}
			}
			break;
			case 6:
				if(u.uenmax == 0) 
			break;
				if(allreadydone&(0x1<<6)) break;
				allreadydone |= 0x1<<6;
				n--; //else commit to the attack.
				if(uarmc || uarm || uarmu || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)){
					You_feel("the tentacles sucking on your %s", uarm ? "armor" : "clothes");
			break;  //blocked
				} //else
				You_feel("little mouths sucking on your exposed %s.",body_part(BODY_SKIN));
				u.uen = 0;
				if(Half_physical_damage) u.uenmax -= (int) max(.1*u.uenmax,5);
				else u.uenmax -= (int) max(.2*u.uenmax, 10);
				if (u.uenmax < 0) u.uenmax = 0;
			break;
			case 7:
				if(allreadydone&(0x1<<7)) break;
				allreadydone |= 0x1<<7;
				n--;
				if(uarmh){
					You_feel("the tentacles squirm over your helmet");
			break; //blocked
				} //else
				You_feel("the tentacles bore into your skull!");
				i = d(1,6);
				if(u.sealsActive&SEAL_HUGINN_MUNINN){
					unbind(SEAL_HUGINN_MUNINN,TRUE);
				} else {
					(void) adjattrib(A_INT, -i, 1);
					while(i-- > 0){
						if(i%2) losexp("brain damage",FALSE,TRUE,FALSE);
						forget_levels(10);	/* lose memory of 10% of levels per point lost*/
						forget_objects(10);	/* lose memory of 10% of objects per point lost*/
						exercise(A_WIS, FALSE);
					}
				}
				//begin moved brain removal messages
				Your("brain is cored like an apple!");
				if (ABASE(A_INT) <= 3) {
					int lifesaved = 0;
					struct obj *wore_amulet = uamul;
					while(1) {
							/* avoid looping on "die(y/n)?" */
							if (lifesaved && (discover || wizard)) {
								if (wore_amulet && !uamul) {
									/* used up AMULET_OF_LIFE_SAVING; still
									   subject to dying from brainlessness */
									wore_amulet = 0;
								} else {
									/* explicitly chose not to die;
									   arbitrarily boost intelligence */
									ABASE(A_INT) = ATTRMIN(A_INT) + 2;
									You_feel("like a scarecrow.");
									break;
								}
							}
						if (lifesaved)
							pline("Unfortunately your brain is still gone.");
						else
							Your("last thought fades away.");
						killer = "destruction of the brain and spinal cord";
						killer_format = KILLED_BY;
						done(DIED);
						lifesaved++;
					}
				}
				losehp(Half_physical_damage ? dmg/2 + 1 : dmg, "head trauma", KILLED_BY);
				
			break;
			case 8:
				if(allreadydone&(0x1<<8)) break;
				allreadydone |= 0x1<<8;
				n--;
				if(uarmc || uarm || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)){
					You_feel("a tentacle squirm over your %s.", uarmc ? "cloak" : (uwep && uwep->oartifact==ART_TENSA_ZANGETSU) ? "shihakusho" : "armor");
			break;  //blocked
				} //else
				You_feel("the tentacles drill through your unprotected %s and into your soul!",body_part(BODY_FLESH));
				if (!Drain_resistance) {
					losexp("soul-shreding tentacles",FALSE,FALSE,FALSE);
					losexp("soul-shreding tentacles",FALSE,FALSE,FALSE);
					losexp("soul-shreding tentacles",FALSE,FALSE,FALSE);
					i = d(1,4);
					while(i-- > 0){
						losexp("soul-shreding tentacles",FALSE,FALSE,TRUE);
						exercise(A_WIS, FALSE);
						exercise(A_WIS, FALSE);
						exercise(A_WIS, FALSE);
					}
					(void) adjattrib(A_CON, -4, 1);
					You_feel("violated and very fragile. Your soul seems a thin and tattered thing.");
				} else {
					(void) adjattrib(A_CON, -2, 1);
					You_feel("a bit fragile, but strangly whole.");
				}
				losehp(Half_physical_damage ? dmg/4+1 : dmg/2+1, "drilling tentacles", KILLED_BY);
			break;
			case 9:
				if(allreadydone&(0x1<<9)) break;
				allreadydone |= 0x1<<9;
				n--;
				if(uarmc || uarm  || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)){
					You_feel("the tentacles press into your %s", uarmc ? "cloak" : (uwep && uwep->oartifact==ART_TENSA_ZANGETSU) ? "shihakusho" : "armor");
			break; //blocked
				} //else
				You_feel("the tentacles spear into your unarmored body!");
				losehp(Half_physical_damage ? dmg : 4*dmg, "impaled by tentacles", NO_KILLER_PREFIX);
				(void) adjattrib(A_STR, -6, 1);
				(void) adjattrib(A_CON, -3, 1);
				You_feel("weak and helpless in their grip!");
			break;
			case 10:
				if(allreadydone&(0x1<<10)) break;
				allreadydone |= 0x1<<10;
			case 11:
				if(allreadydone&(0x1<<11)) break;
				allreadydone |= 0x1<<11;
			case 12:
				// if(allreadydone&(0x1<<12)) break; //earth any remaining attempts
				// allreadydone |= 0x1<<12;
				if(uarmc || (uwep && uwep->oartifact==ART_TENSA_ZANGETSU)) {
					n--;//while you have your cloak, this burns attacks at a high rate.
					You_feel("the tentacles writhe over your %s.", uarmc ? "cloak" : "shihakusho");
			break;
				} //else
				if(invent && !uarmc && !uarm && !uarmu && !uarmf && !uarmg && !uarms && !uarmh && !uwep
					){ //only steal if you have at least one item and everything else of interest is already gone.
					n = 0;
					You_feel("the tentacles pick through your remaining possessions.");
					buf[0] = '\0';
					steal(mon, buf,FALSE);
					buf[0] = '\0';
					steal(mon, buf,FALSE);
					buf[0] = '\0';
					steal(mon, buf,FALSE);
					buf[0] = '\0';
					steal(mon, buf,FALSE);
				}
			break;
		}
		}
	return 1;
}

STATIC_OVL void
mayberem(obj, str)
register struct obj *obj;
const char *str;
{
	char qbuf[QBUFSZ];

	if (!obj || !obj->owornmask) return;

	if (rn2(20) < ACURR(A_CHA)) {
		Sprintf(qbuf,"\"Shall I remove your %s, %s?\"",
			str,
			(!rn2(2) ? "lover" : !rn2(2) ? "dear" : "sweetheart"));
		if (yn(qbuf) == 'n') return;
	} else {
		char hairbuf[BUFSZ];

		Sprintf(hairbuf, "let me run my fingers through your %s",
			body_part(HAIR));
		verbalize("Take off your %s; %s.", str,
			(obj == uarm)  ? "let's get a little closer" :
			(obj == uarmc || obj == uarms) ? "it's in the way" :
			(obj == uarmf) ? "let me rub your feet" :
			(obj == uarmg) ? "they're too clumsy" :
#ifdef TOURIST
			(obj == uarmu) ? "let me massage you" :
#endif
			/* obj == uarmh */
			hairbuf);
	}
	remove_worn_item(obj, TRUE);
}
STATIC_OVL void
lrdmayberem(obj, str)
register struct obj *obj;
const char *str;
{
	char qbuf[QBUFSZ];

	if (!obj || !obj->owornmask) return;

	if (rn2(60) < ACURR(A_CHA)) {
		Sprintf(qbuf,"\"Shall I remove your %s, %s?\"",
			str,
			(!rn2(2) ? "lover" : !rn2(2) ? "dear" : "sweetheart"));
		if (yn(qbuf) == 'n') return;
	} else {
		char hairbuf[BUFSZ];

		Sprintf(hairbuf, "let me run my fingers through your %s",
			body_part(HAIR));
		verbalize("Take off your %s; %s.", str,
			(obj == uarm)  ? "let's get a little closer" :
			(obj == uarmc || obj == uarms) ? "it's in the way" :
			(obj == uarmf) ? "let me rub your feet" :
			(obj == uarmg) ? "they're too clumsy" :
#ifdef TOURIST
			(obj == uarmu) ? "let me massage you" :
#endif
			/* obj == uarmh */
			hairbuf);
	}
	remove_worn_item(obj, TRUE);
}
//#endif  /* SEDUCE */

//#endif /* OVLB */

STATIC_OVL void
mlcmayberem(obj, str, helpless)
boolean helpless;
register struct obj *obj;
const char *str;
{
	char qbuf[QBUFSZ];

	if (!obj || !obj->owornmask) return;

	if (helpless || rn2(60) < ACURR(A_CHA)) {
		Sprintf(qbuf,"\"Shall I remove your %s, %s?\"",
			str,
			(!rn2(2) ? "lover" : !rn2(2) ? "dear" : "sweetheart"));
		if (yn(qbuf) == 'n') return;
	} else {
		char hairbuf[BUFSZ];

		Sprintf(hairbuf, "let me run my fingers through your %s",
			body_part(HAIR));
		verbalize("Take off your %s; %s.", str,
			(obj == uarm)  ? "let's get a little closer" :
			(obj == uarmc || obj == uarms) ? "it's in the way" :
			(obj == uarmf) ? "let me rub your feet" :
			(obj == uarmg) ? "they're too clumsy" :
#ifdef TOURIST
			(obj == uarmu) ? "let me massage you" :
#endif
			/* obj == uarmh */
			hairbuf);
	}
	remove_worn_item(obj, TRUE);
}

STATIC_OVL void
sflmayberem(obj, str, helpless)
boolean helpless;
register struct obj *obj;
const char *str;
{
	char qbuf[QBUFSZ];
	int her_strength;

	if (!obj || !obj->owornmask) return;
	
	her_strength = 25 + rn2(100);
	if (helpless || her_strength < ACURR(A_STR)) {
		Sprintf(qbuf,"She tries to take off your %s, allow her?",
			str);
		if (yn(qbuf) == 'n') return;
	} else if(her_strength > ACURR(A_STR)*2){
		Sprintf(qbuf,"She tries to rip open your %s!",
			str);
		her_strength -= ACURR(A_STR);
		for(her_strength; her_strength >= 0; her_strength--){
			if(obj->spe > -1*objects[(obj)->otyp].a_ac){
				damage_item(obj);
//				Your("%s less effective.", aobjnam(obj, "seem"));
			}
			else if(!obj->oartifact){
				claws_destroy_arm(obj);
			}
			else{
				remove_worn_item(obj, TRUE);
			}
		}
		return;
	}
	remove_worn_item(obj, TRUE);
	Sprintf(qbuf,"She removes your %s!",
		str);
}
STATIC_OVL void
palemayberem(obj, str, helpless)
register struct obj *obj;
const char *str;
boolean helpless;
{
	char qbuf[QBUFSZ];
	int its_cha;

	if (!obj || !obj->owornmask) return;
	
	its_cha = rn2(40);
	if (helpless || its_cha >= ACURR(A_CHA)) {
		if(!obj->oartifact || !rn2(10)){
			destroy_arm(obj);
		}
	}
}
#endif  /* SEDUCE */

#endif /* OVLB */

#ifdef OVL1

STATIC_OVL int
passiveum(olduasmon,mtmp,mattk)
struct permonst *olduasmon;
register struct monst *mtmp;
register struct attack *mattk;
{
	int i, tmp;

	for (i = 0; ; i++) {
	    if (i >= NATTK) return 1;
	    if (olduasmon->mattk[i].aatyp == AT_NONE ||
	    		olduasmon->mattk[i].aatyp == AT_BOOM) break;
	}
	if (olduasmon->mattk[i].damn)
	    tmp = d((int)olduasmon->mattk[i].damn,
				    (int)olduasmon->mattk[i].damd);
	else if(olduasmon->mattk[i].damd)
	    tmp = d((int)olduasmon->mlevel+1, (int)olduasmon->mattk[i].damd);
	else
	    tmp = 0;

	/* These affect the enemy even if you were "killed" (rehumanized) */
	switch(olduasmon->mattk[i].adtyp) {
	    case AD_ACID:
		if (!rn2(2)) {
		    pline("%s is splashed by your acid!", Monnam(mtmp));
		    if (resists_acid(mtmp)) {
			pline("%s is not affected.", Monnam(mtmp));
			tmp = 0;
		    }
		} else tmp = 0;
		if (!rn2(30)) erode_armor(mtmp, TRUE);
		if (!rn2(6)) erode_obj(MON_WEP(mtmp), TRUE, TRUE);
		goto assess_dmg;
	    case AD_STON: /* cockatrice */
	    {
		long protector = attk_protection((int)mattk->aatyp),
		     wornitems = mtmp->misc_worn_check;

		/* wielded weapon gives same protection as gloves here */
		if (MON_WEP(mtmp) != 0) wornitems |= W_ARMG;

		if (!resists_ston(mtmp) && 
			(protector == 0L || (protector != ~0L &&
			    (wornitems & protector) != protector))
		) {
		    if (poly_when_stoned(mtmp->data)) {
			mon_to_stone(mtmp);
			return (1);
		    }
		    pline("%s turns to stone!", Monnam(mtmp));
		    stoned = 1;
		    xkilled(mtmp, 0);
		    if (mtmp->mhp > 0) return 1;
		    return 2;
		}
		return 1;
	    }
		case AD_DRST:
		case AD_DRDX:
		case AD_DRCO:
			if (!rn2(8)) {
				Your("shadow was poisoned!");
				if (resists_poison(mtmp)){
					pline_The("poison doesn't seem to affect %s.",
						mon_nam(mtmp));
				} else {
					if (!rn2(10)) {
						Your("poison was deadly...");
						tmp = mtmp->mhp;
					} else tmp += rn1(10,6);
				}
			}
		break;
	    case AD_WEBS:{
			struct trap *ttmp2 = maketrap(mtmp->mx, mtmp->my, WEB);
			if (ttmp2) mintrap(mtmp);
		}break;
	    case AD_ENCH:	/* KMH -- remove enchantment (disenchanter) */
	    	if (otmp) {
	    	    (void) drain_item(otmp);
	    	    /* No message */
	    	}
	    	return (1);
	    default:
		break;
	}
//	if (!Upolyd) return 1; //Removed to enable spirit pasive attacks

	/* These affect the enemy only if you are still a monster */
	if (rn2(3)) switch(youmonst.data->mattk[i].adtyp) {
	    case AD_PHYS:
	    	if (youmonst.data->mattk[i].aatyp == AT_BOOM) {
	    	    You("explode!");
	    	    /* KMH, balance patch -- this is okay with unchanging */
	    	    rehumanize();
	    	    goto assess_dmg;
	    	}
	    	break;
	    case AD_PLYS: /* Floating eye */
		if (tmp > 127) tmp = 127;
		if (u.umonnum == PM_FLOATING_EYE) {
		    if (!rn2(4)) tmp = 127;
		    if (!is_blind(mtmp) && haseyes(mtmp->data) && rn2(3) &&
				(perceives(mtmp->data) || !Invis)) {
			if (Blind)
			    pline("As a blind %s, you cannot defend yourself.",
							youmonst.data->mname);
		        else {
			    if (mon_reflects(mtmp,
					    "Your gaze is reflected by %s %s."))
				return 1;
			    pline("%s is frozen by your gaze!", Monnam(mtmp));
			    mtmp->mcanmove = 0;
			    mtmp->mfrozen = tmp;
			    return 3;
			}
		    }
		} else { /* gelatinous cube */
		    pline("%s is frozen by you.", Monnam(mtmp));
		    mtmp->mcanmove = 0;
		    mtmp->mfrozen = tmp;
		    return 3;
		}
		return 1;
	    case AD_COLD: /* Brown mold or blue jelly */
		if (resists_cold(mtmp)) {
		    shieldeff(mtmp->mx, mtmp->my);
		    pline("%s is mildly chilly.", Monnam(mtmp));
		    golemeffects(mtmp, AD_COLD, tmp);
		    tmp = 0;
		    break;
		}
		pline("%s is suddenly very cold!", Monnam(mtmp));
		u.mh += tmp / 2;
		if (u.mhmax < u.mh) u.mhmax = u.mh;
		if (u.mhmax > ((youmonst.data->mlevel+1) * 8))
		    (void)split_mon(&youmonst, mtmp);
		break;
	    case AD_STUN: /* Yellow mold */
		if (!mtmp->mstun) {
		    mtmp->mstun = 1;
		    pline("%s %s.", Monnam(mtmp),
			  makeplural(stagger(mtmp->data, "stagger")));
		}
		tmp = 0;
		break;
	    case AD_FIRE: /* Red mold */
		if (resists_fire(mtmp)) {
		    shieldeff(mtmp->mx, mtmp->my);
		    pline("%s is mildly warm.", Monnam(mtmp));
		    golemeffects(mtmp, AD_FIRE, tmp);
		    tmp = 0;
		    break;
		}
		pline("%s is suddenly very hot!", Monnam(mtmp));
		break;
	    case AD_ELEC:
		if (resists_elec(mtmp)) {
		    shieldeff(mtmp->mx, mtmp->my);
		    pline("%s is slightly tingled.", Monnam(mtmp));
		    golemeffects(mtmp, AD_ELEC, tmp);
		    tmp = 0;
		    break;
		}
		pline("%s is jolted with your electricity!", Monnam(mtmp));
		break;
	    default: tmp = 0;
		break;
	}
	else tmp = 0;
	
	if(u.sealsActive){
		if(u.sealsActive&SEAL_ECHIDNA){
			pline("%s is splashed by your acidic blood!", Monnam(mtmp));
			if (resists_acid(mtmp)) {
				pline("%s is not affected.", Monnam(mtmp));
			} else {
				tmp += rnd(spiritDsize());
			}
		}
		if(u.sealsActive&SEAL_FAFNIR){
			pline("%s is burned by your halo of flames!", Monnam(mtmp));
			if (resists_fire(mtmp)) {
				pline("%s is not affected.", Monnam(mtmp));
			} else {
				tmp += rnd(spiritDsize());
				if(resists_cold(mtmp)) tmp += rnd(spiritDsize());
			}
		}
		if(u.sealsActive&SEAL_EDEN){
			if (hates_silver(mtmp->data)) {
				Your("silver body sears %s!", mon_nam(mtmp));
				tmp += rnd(20);
			}
		}
		if(u.sealsActive&SEAL_EURYNOME && !rn2(5)){
			You("counterattack!");
			attack(mtmp);
			if(DEADMONSTER(mtmp)) return 2;
		}
	}

	if( uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && 
		uwep->ovar1&SEAL_EURYNOME && 
		!rn2((quest_status.killed_nemesis && Role_if(PM_EXILE)) ? 10 : 20)
	){
		You("counterattack!");
		attack(mtmp);
		if(DEADMONSTER(mtmp)) return 2;
	}
	
    assess_dmg:
	if((mtmp->mhp -= tmp) <= 0) {
		pline("%s dies!", Monnam(mtmp));
		xkilled(mtmp,0);
		if (mtmp->mhp > 0) return 1;
		return 2;
	}
	return 1;
}

#endif /* OVL1 */
#ifdef OVLB

#include "edog.h"
struct monst *
cloneu()
{
	register struct monst *mon;
	int mndx = monsndx(youmonst.data);

	if (u.mh <= 1) return(struct monst *)0;
	if (mvitals[mndx].mvflags & G_EXTINCT && !In_quest(&u.uz)) return(struct monst *)0;
	mon = makemon(youmonst.data, u.ux, u.uy, NO_MINVENT|MM_EDOG);
	if (mon) {
		mon = christen_monst(mon, plname);
		initedog(mon);
		mon->m_lev = youmonst.data->mlevel;
		mon->mhpmax = u.mhmax;
		mon->mhp = u.mh / 2;
		mon->mclone = 1;
		u.mh -= mon->mhp;
		flags.botl = 1;
	}
	return(mon);
}

#endif /* OVLB */

/*mhitu.c*/
