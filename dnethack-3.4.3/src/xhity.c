#include "hack.h"
#include "artifact.h"
#include "monflag.h"
#include "edog.h"
#include "xhity.h"

/* TODO LIST

MAJOR:

MINOR:
artifact hitmesages
Claws of the Revenancer w/ rings
*/

STATIC_DCL int FDECL(getvis, (struct monst *, struct monst *, int, int));
STATIC_DCL void FDECL(wildmiss, (struct monst *, struct attack *, struct obj *, boolean));
STATIC_DCL boolean FDECL(u_surprise, (struct monst *, boolean));
STATIC_DCL struct attack * FDECL(getnextspiritattack, (boolean));
STATIC_DCL int FDECL(destroy_item2, (struct monst *, int, int, boolean));
STATIC_DCL void FDECL(xswingsy, (struct monst *, struct monst *, struct obj *, boolean));
STATIC_DCL void FDECL(xyhitmsg, (struct monst *, struct monst *, struct attack *));
STATIC_DCL void FDECL(noises, (struct monst *, struct attack *));
STATIC_DCL void FDECL(xymissmsg, (struct monst *, struct monst *, struct attack *, int, boolean));
STATIC_DCL void FDECL(heal, (struct monst *, int));
STATIC_DCL int FDECL(xdamagey, (struct monst *, struct monst *, struct attack *, int, boolean));
STATIC_DCL int FDECL(xstoney, (struct monst *, struct monst *));
STATIC_DCL int FDECL(do_weapon_multistriking_effects, (struct monst *, struct monst *, struct attack *, struct obj *, int));
STATIC_DCL int FDECL(xmeleehurty, (struct monst *, struct monst *, struct attack *, struct obj *, boolean, int, int, int, boolean));
STATIC_DCL int FDECL(xcasty, (struct monst *, struct monst *, struct attack *, int));
STATIC_DCL int FDECL(xtinkery, (struct monst *, struct monst *, struct attack *, int));
STATIC_DCL int FDECL(xengulfhity, (struct monst *, struct monst *, struct attack *, int));
STATIC_DCL int FDECL(xengulfhurty, (struct monst *, struct monst *, struct attack *, int));
STATIC_DCL int FDECL(xexplodey, (struct monst *, struct monst *, struct attack *, int));
STATIC_DCL int FDECL(hmoncore, (struct monst *, struct monst *, struct attack *, struct obj *, struct obj *, int, int, int, boolean, int, boolean, int, boolean *));
STATIC_DCL int FDECL(shadow_strike, (struct monst *));
STATIC_DCL int FDECL(xpassivehity, (struct monst *, struct monst *, struct attack *, struct attack *, struct obj *, int, int, struct permonst *, boolean));

/* item destruction strings from zap.c */
extern const char * const destroy_strings[];

/* for long worms */
extern boolean notonhead;

/* Counterattack chance at skill level....  B:  S:  E:  */
static const int DjemSo_counterattack[] = {  5, 10, 20 };
static const int Shien_counterattack[]  = {  5, 10, 20 };
static const int Soresu_counterattack[] = { 10, 15, 25 };
/* Misc attacks */
static struct attack noattack = { 0, 0, 0, 0 };
static struct attack basicattack  = { AT_WEAP, AD_PHYS, 1, 4 };

/* getvis()
 * 
 * determines vis if needed.
 */
int
getvis(magr, mdef, tarx, tary)
struct monst * magr;
struct monst * mdef;
int tarx;
int tary;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	int vis;
	
	if (!tarx && !tary) {
		tarx = x(mdef);
		tary = y(mdef);
	}

	if (youagr || youdef || cansee(x(magr), y(magr)) || cansee(tarx, tary))
	{
		if (youagr || (cansee(x(magr), y(magr)) && canseemon(magr)))
			vis |= VIS_MAGR;
		if (youdef || (cansee(tarx, tary) && canseemon(mdef)))
			vis |= VIS_MDEF;
		if (youagr || youdef || canspotmon(magr) || canspotmon(mdef))
			vis |= VIS_NONE;
	}
	else
		vis = 0;
	return vis;
}

/* attack2()
 * 
 * The bump attack function!
 * Player-only; u.dx and u.dy must already be set.
 * Returns FALSE if we moved to where the monster was.
 * TODO: remove old attack() function, rename this one
 */
boolean
attack2(mdef)
struct monst * mdef;
{
	int result;
	int attack_type;	/* uses ATTACKCHECKS defines */
	register struct permonst *pd = mdef->data;

	/* This section of code provides protection against accidentally
	 * hitting peaceful (like '@') and tame (like 'd') monsters.
	 * Protection is provided as long as player is not: blind, confused,
	 * hallucinating or stunned.
	 * changes by wwp 5/16/85
	 * More changes 12/90, -dkh-. if its tame and safepet, (and protected
	 * 07/92) then we assume that you're not trying to attack. Instead,
	 * you'll usually just swap places if this is a movement command
	 */
	/* Intelligent chaotic weapons (Stormbringer) want blood */
	if (is_safepet(mdef) && !flags.forcefight) {
		if (!uwep || !spec_ability2(uwep, SPFX2_BLDTHRST)) {
			/* there are some additional considerations: this won't work
			* if in a shop or Punished or you miss a random roll or
			* if you can walk thru walls and your pet cannot (KAA) or
			* if your pet is a long worm (unless someone does better).
			* there's also a chance of displacing a "frozen" monster.
			* sleeping monsters might magically walk in their sleep.
			*/
			boolean foo = (Punished || !rn2(7) || is_longworm(mdef->data)),
				inshop = FALSE;
			char *p;

			for (p = in_rooms(x(mdef), y(mdef), SHOPBASE); *p; p++)
			if (tended_shop(&rooms[*p - ROOMOFFSET])) {
				inshop = TRUE;
				break;
			}

			if (inshop || foo ||
				(IS_ROCK(levl[u.ux][u.uy].typ) &&
				!mon_resistance(mdef, PASSES_WALLS))) {
				char buf[BUFSZ];

				monflee(mdef, rnd(6), FALSE, FALSE);
				Strcpy(buf, y_monnam(mdef));
				buf[0] = highc(buf[0]);
				You("stop.  %s is in the way!", buf);
				return(TRUE);
			}
			else if ((mdef->mfrozen || (!mdef->mcanmove)
				|| (mdef->data->mmove == 0)) && rn2(6)) {
				pline("%s doesn't seem to move!", Monnam(mdef));
				return(TRUE);
			}
			else return(FALSE);
		}
	}

	/* attack_checks returns a truthy value if we can attack */
	if (!(attack_type = attack_checks(mdef, uwep))) {
		return TRUE;
	}

	if (u.twoweap && !test_twoweapon())
		untwoweapon();

	if (unweapon) {
		unweapon = FALSE;
		if (flags.verbose) {
			if (uwep){
				if (uwep->oartifact == ART_LIECLEAVER)
					You("begin slashing monsters with your %s.", aobjnam(uwep, (char *)0));
				else You("begin bashing monsters with your %s.",
					aobjnam(uwep, (char *)0));
			}
			else if (!cantwield(youracedata)){
				if (u.specialSealsActive&SEAL_BLACK_WEB)
					You("begin slashing monsters with your shadow-blades.");
				else
					You("begin %sing monsters with your %s %s.",
					Role_if(PM_MONK) ? "strik" : "bash",
					uarmg ? "gloved" : "bare",	/* Del Lamb */
					makeplural(body_part(HAND)));
			}
		}
	}
	exercise(A_STR, TRUE);		/* you're exercising muscles */
	/* andrew@orca: prevent unlimited pick-axe attacks */
	u_wipe_engr(3);

	/* Is the "it died" check actually correct? */
	if (pd->mlet == S_LEPRECHAUN && !mdef->mfrozen && !mdef->msleeping &&
		!mdef->mconf && !is_blind(mdef) && !rn2(7) &&
		(m_move(mdef, 0) == 2 ||			    /* it died */
		x(mdef) != u.ux + u.dx || y(mdef) != u.uy + u.dy)) /* it moved */
	{
		return(FALSE);
	}

	check_caitiff(mdef);

	/* SCOPECREEP: remove this, replace with u.actioncost */
	if (uwep && fast_weapon(uwep) && uwep->spe >= 2){
		youmonst.movement += NORMAL_SPEED / 6;
	}
	if (uwep
		&& (uwep->otyp == RAKUYO || uwep->otyp == DOUBLE_FORCE_BLADE)
		&& !u.twoweap
		){
		youmonst.movement -= NORMAL_SPEED / 4;
	}
	
	if ((u.specialSealsActive&SEAL_BLACK_WEB) && u.spiritPColdowns[PWR_WEAVE_BLACK_WEB] > moves + 20)
		weave_black_web(mdef);
	
	/* Do the attacks */
	if (attack_type == ATTACKCHECK_BLDTHRST) {
		/* unintentional attacks only cause the one hit, no follow-ups */
		int vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
		/* assumes the bloodthirst is caused by your mainhand weapon */
		Your("bloodthirsty weapon attacks!");
		bhitpos.x = u.ux + u.dx; bhitpos.y = u.uy + u.dy;
		notonhead = (bhitpos.x != x(mdef) || bhitpos.y != y(mdef));
		result = xmeleehity(&youmonst, mdef, &basicattack, uwep, VIS_MAGR, 0, FALSE);
	}
	else {
		bhitpos.x = u.ux + u.dx; bhitpos.y = u.uy + u.dy;
		notonhead = (bhitpos.x != x(mdef) || bhitpos.y != y(mdef));
		result = xattacky(&youmonst, mdef, u.ux + u.dx, u.uy + u.dy);

		if (!DEADMONSTER(mdef) && u.sealsActive&SEAL_SHIRO){
			int i, dx, dy;
			struct obj *otmp;
			for (i = rnd(8); i>0; i--){
				dx = rn2(3) - 1;
				dy = rn2(3) - 1;
				otmp = mksobj(ROCK, TRUE, FALSE);
				otmp->blessed = 0;
				otmp->cursed = 0;
				if ((dx || dy) && !DEADMONSTER(mdef)){
					projectile(&youmonst, otmp, (struct obj *)0, TRUE, x(mdef) + dx, y(mdef) + dy, -dx, -dy, 0, 1, TRUE, FALSE, FALSE);
				}
				else {
					projectile(&youmonst, otmp, (struct obj *)0, TRUE, u.ux, u.uy, u.dx, u.dy, 0, 1, TRUE, FALSE, FALSE);
				}
			}
		}

		if (!DEADMONSTER(mdef) && u.sealsActive&SEAL_AHAZU){
			if (*hp(mdef) < .1*(*hpmax(mdef))){
#define MAXVALUE 24
				extern const int monstr[];
				int value = min(monstr[monsndx(pd)] + 1, MAXVALUE);
				pline("%s sinks into your deep black shadow!", Monnam(mdef));
				cprefx(monsndx(pd), TRUE, TRUE);
				cpostfx(monsndx(pd), FALSE, TRUE, FALSE);
				if (u.ugangr[Align2gangr(u.ualign.type)]) {
					u.ugangr[Align2gangr(u.ualign.type)] -= ((value * (u.ualign.type == A_CHAOTIC ? 2 : 3)) / MAXVALUE);
					if (u.ugangr[Align2gangr(u.ualign.type)] < 0) u.ugangr[Align2gangr(u.ualign.type)] = 0;
				}
				else if (u.ualign.record < 0) {
					if (value > MAXVALUE) value = MAXVALUE;
					if (value > -u.ualign.record) value = -u.ualign.record;
					adjalign(value);
				}
				else if (u.ublesscnt > 0) {
					u.ublesscnt -=
						((value * (u.ualign.type == A_CHAOTIC ? 500 : 300)) / MAXVALUE);
					if (u.ublesscnt < 0) u.ublesscnt = 0;
				}
				mongone(mdef);
			}
			else if (!rn2(5)){
				Your("deep black shadow pools under %s.", mon_nam(mdef));
				mdef->movement -= 12;
			}
		}
	}

atk_done:
	/* see comment in attack_checks() */
	/* we only need to check for this if we did an attack_checks()
	* and it returned non-0 (it's okay to attack), and the monster didn't
	* evade.
	*/
	if (flags.forcefight && mdef && mdef->mhp > 0 && !canspotmon(mdef) &&
		!glyph_is_invisible(levl[u.ux + u.dx][u.uy + u.dy].glyph) &&
		!(u.uswallow && mdef == u.ustuck))
		map_invisible(u.ux + u.dx, u.uy + u.dy);

	return(TRUE);
}

/* xattacky()
 * 
 * Harmonized function for when a creature attempts to attack another creature.
 * 
 * Assumes that the agressor (now) knows the defender is there.
 * 
 * If the player is the aggressor, assumes melee range to the defender.
 * The player's ranged attacks must be handled elsewhere.
 * 
 * Monster to monster attacks.  When a monster attacks another (mattackm),
 * any or all of the following can be returned.  See mattackm() for more
 * details.
 * MM_MISS		0x00	aggressor missed
 * MM_HIT		0x01	aggressor hit defender
 * MM_DEF_DIED	0x02	defender died
 * MM_AGR_DIED	0x04	aggressor died
 * MM_AGR_STOP	0x08	aggressor must stop attacking for some reason
 * MM_DEF_LSVD	0x10	defender died and then revived
 * 
 * Inside this function itself, determines if/which attacks are allowed and
 * then farms each out to an appropriate function.
 * 
 * The farmed out attack-specific function is responsible for calling
 * the correct damaging function. For example, AT_GAZE attacks have
 * different effects than AT_CLAW attacks, even when with the same adtype.
 */
int
xattacky(magr, mdef, tarx, tary)
struct monst * magr;
struct monst * mdef;
int tarx;
int tary;
{
	/* if attacker doesn't exist or is trying to attack something that doesn't exist -- must be checked right away */
	if (!magr || !mdef)
		return(MM_MISS);		/* mike@genat */

	int	indexnum = 0,	/* loop counter */
		tohitmod = 0,	/* flat accuracy modifier for a specific attack */
		strike = 0,	/* hit this attack (default to 0) */
		aatyp = 0,	/* aatyp of current attack; used for brevity */
		adtyp = 0,	/* adtyp of current attack; used for brevity */
		struck = 0,	/* hit at least once */
		marinum = 0,/* number of AT_MARI weapons used */
		subout = 0,	/* remembers what attack substitutions have been made for [magr]'s attack chain */
		res[3];		/* results of previous 2 attacks ([0] -> current attack, [1] -> 1 ago, [2] -> 2 ago) -- this is dynamic! */
	int attacklimit, attacksmade;
	struct attack *attk;
	struct attack prev_attk = noattack;
	struct obj *otmp;
	int vis = 0;
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	boolean missedyou = (!youagr && youdef && (tarx != u.ux || tary != u.uy));	/* monster tried to attack you, but it got your location wrong */
	boolean ranged = (distmin(x(magr), y(magr), tarx, tary) > 1);	/* is magr near its target? */
	boolean dopassive = FALSE;	/* whether or not to provoke a passive counterattack */
	/* if TRUE, don't make attacks that will be fatal to self (like touching a cockatrice) */
	boolean be_safe = (mdef && !(youagr ? (Confusion || Stunned || Hallucination || flags.forcefight || !canseemon(mdef)) :
		(magr->mconf || magr->mstun || magr->mcrazed || mindless_mon(magr) || (youdef && !mon_can_see_you(magr)) || (!youdef && !mon_can_see_mon(magr, mdef)))));

	/* set permonst pointers */
	struct permonst * pa = youagr ? youracedata : magr->data;
	struct permonst * pd = youdef ? youracedata : mdef->data;
	int result = 0;		/* result from current attack */
	int allres = 0;		/* cumulative results from all attacks; needed for passives */
	long slot;

	/* set notonhead */
	notonhead = (tarx != x(mdef) || tary != y(mdef));

	/*
	* Since some characters attack multiple times in one turn,
	* allow user to specify a count prefix for 'F' to limit
	* number of attack (to avoid possibly hitting a dangerous
	* monster multiple times in a row and triggering lots of
	* retalitory attacks).
	*/
	if (youagr) {
		/* kludge to work around parse()'s pre-decrement of `multi' */
		attacklimit = (multi || save_cm) ? multi + 1 : 0;
		multi = 0;		/* reset; it's been used up */
	}
	/* monsters always give their all */
	else 
	{
		attacklimit = 0;
	}

	/* cases where the agressor cannot make any attacks at all */
	/* player is invincible*/
	if(youdef && (u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves+20)) {
	    /* monsters won't attack you */
	    if(magr == u.ustuck)
			pline("%s loosens its grip slightly.", Monnam(magr));
	    else if(!ranged) {
		if (canseemon(magr) || sensemon(magr))
		    pline("%s starts to attack you, but pulls back.",
			  Monnam(magr));
		else
		    You_feel("%s move nearby.", something);
	    }
	    return MM_MISS;
	}

	/* agr can't attack */
	if (cantmove(magr))
		return MM_MISS;

	if (youagr && madness_cant_attack(mdef))
		return MM_MISS;

	/* some creatures are limited in *where* they can attack */
	/* Grid bugs and Bebeliths cannot attack at an angle. */
	if ((pa == &mons[PM_GRID_BUG] || pa == &mons[PM_BEBELITH])
		&& x(magr) != tarx && y(magr) != tary)
		return MM_MISS;

	/* limited attack angles (monster-agressor only) */
	if (!youagr && (
		pa == &mons[PM_CLOCKWORK_SOLDIER] || pa == &mons[PM_CLOCKWORK_DWARF] ||
		pa == &mons[PM_FABERGE_SPHERE] || pa == &mons[PM_FIREWORK_CART] ||
		pa == &mons[PM_JUGGERNAUT] || pa == &mons[PM_ID_JUGGERNAUT]))
	{
		if (x(magr) + xdir[(int)magr->mvar1] != tarx ||
			y(magr) + ydir[(int)magr->mvar1] != tary)
			return MM_MISS;
	}

	/* Monsters can't attack a player that's underwater unless the monster can swim; asymetric */
	if (youdef && Underwater && !mon_resistance(magr, SWIMMING))
		return MM_MISS;

	/* Monsters can't attack a player that's swallowed unless the monster *is* u.ustuck */
	if (youdef && u.uswallow) {
		if (magr != u.ustuck)
			return MM_MISS;
		/* they also know exactly where you are */
		u.ustuck->mux = u.ux;
		u.ustuck->muy = u.uy;
		/* if you're invulnerable, you're fine though */
		if (u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves + 20)
			return MM_MISS; /* stomachs can't hurt you! */
	}

	/* Set up the visibility of action */
	vis = getvis(magr, mdef, tarx, tary);

	/*	Set flag indicating monster has moved this turn.  Necessary since a
	 *	monster might get an attack out of sequence (i.e. before its move) in
	 *	some cases, in which case this still counts as its move for the round
	 *	and it shouldn't move again.
	 */
	if(!youagr) magr->mlstmv = monstermoves;

	/* Weeping angels (pre-invocation) can only attack once per global turn */
	if (!(u.uevent.invoked) && is_weeping(pa))
		magr->movement = 0;

	/* monsters may be in for a surprise if attacking a hidden player */
	if (youdef && !ranged && !missedyou && !u.uswallow) {
		if (u_surprise(magr, canseemon(magr)))
			return MM_MISS;	/* they can't attack this turn */
	}

	/*	Special demon/minion handling code */
	if (youdef && !magr->cham && (is_demon(pa) || is_minion(pa)) && !ranged
		&& pa != &mons[PM_OONA]
		&& pa != &mons[PM_BALROG]
		&& pa != &mons[PM_DURIN_S_BANE]
		&& pa != &mons[PM_SUCCUBUS]
		&& pa != &mons[PM_INCUBUS]
		) {
		if (!magr->mcan && !rn2(13))
			msummon(magr);
		return MM_MISS;
	}
	if (youagr && is_demon(youracedata) && !rn2(13) && !uwep
		&& u.umonnum != PM_SUCCUBUS
		&& u.umonnum != PM_INCUBUS
		&& u.umonnum != PM_BALROG) {
	    demonpet();
		return MM_MISS;
	}
	/*	Special lycanthrope handling code */
	if(youdef && !magr->cham && is_were(pa) && !ranged) {

	    if(is_human(pa)) {
			if(!rn2(5 - (night() * 2)) && !magr->mcan) new_were(magr);
	    } else if(!rn2(30) && !magr->mcan) new_were(magr);
	    pa = magr->data;

	    if(!rn2(10) && !magr->mcan) {
	    	int numseen, numhelp;
		char buf[BUFSZ], genericwere[BUFSZ];

		Strcpy(genericwere, "creature");
		numhelp = were_summon(pa, FALSE, &numseen, genericwere);
		if (vis&VIS_MAGR) {
			pline("%s summons help!", Monnam(magr));
			if (numhelp > 0) {
			    if (numseen == 0)
				You_feel("hemmed in.");
			} else pline("But none comes.");
		} else {
			const char *from_nowhere;

			if (flags.soundok) {
				pline("%s %s!", Something,
					makeplural(growl_sound(magr)));
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
				/* help came, that was their action */
				return MM_MISS;
			} /* else no help came; but you didn't know it tried */
		}
	    }
	}
	/* monsters may target your steed */
	if (youdef && u.usteed && !missedyou) {
		if (magr == u.usteed)
			/* Your steed won't attack you */
			return MM_MISS;
		/* Orcs like to steal and eat horses and the like */
		if (!rn2(is_orc(magr->data) ? 2 : 4) &&
			distu(x(magr), y(magr)) <= 2) {
			/* Attack your steed instead */
			result = xattacky(magr, u.usteed, u.ux, u.uy);
			if (result != MM_MISS) {	/* needs to have made at least 1 attack */
				if (result & MM_AGR_DIED)
					return MM_AGR_DIED;
				if ((result & MM_DEF_DIED) || u.umoved)
					return MM_AGR_STOP;
				/* Let your steed maybe retaliate */
				if (u.usteed->movement >= NORMAL_SPEED) {
					int res2 = xattacky(u.usteed, magr, x(magr), y(magr));
					if (res2) {
						if (res2 & MM_DEF_DIED)
							result |= MM_AGR_DIED;
						if (res2 & MM_AGR_DIED)
							result |= MM_AGR_STOP;
						u.usteed->movement -= NORMAL_SPEED;
					}
				}
			}
			return result;
		}
	}

	/* lillends (that aren't you) can use masks */
	if (pa == &mons[PM_LILLEND]
		&& !youagr
		&& !(magr->mfaction == ZOMBIFIED || magr->mfaction == SKELIFIED)
		&& rn2(2)){
		magr->mvar2 = monsndx(find_mask(magr));
		if (!Blind && pa != &mons[PM_LILLEND] && canseemon(magr))
			pline("%s uses a %s mask!", Monnam(magr), pa->mname);
	}
	/* zero out res[] */
	res[0] = MM_MISS;
	res[1] = MM_MISS;
	res[2] = MM_MISS;
	/* Now perform all attacks. */
	do {
		boolean dopassive_local = FALSE;
		/* reset pd (in case player-defender rehumanized) */
		pd = youdef ? youracedata : mdef->data;
		/* reset otmp */
		otmp = (struct obj *)0;
		/* reset tohitmod */
		tohitmod = 0;
		/* reset result */
		result = 0;
		/* cycle res[] -- it should be the results of the previous 2 attacks */
		res[2] = res[1];
		res[1] = res[0];
		res[0] = MM_MISS;
		/* get next attack */
		attk = getattk(magr, res, &indexnum, &prev_attk, FALSE, &subout, &tohitmod);
		/* set aatyp, adtyp */
		aatyp = attk->aatyp;
		adtyp = attk->adtyp;
		/* maybe end (mdef may have been forcibly moved!)*/
		if (((youdef || mdef==u.usteed) && !missedyou && (tarx != u.ux || tary != u.uy)) ||
			(!(youdef || mdef == u.usteed) && m_at(tarx, tary) != mdef)) {
			result = MM_AGR_STOP;
			continue;
		}
		
		/* if you are the target and are engulfed, you are only targettable by engulf attacks */
		if (youdef && u.uswallow && (aatyp != AT_ENGL && aatyp != AT_ILUR))
			continue;

		/* Some armor completely covers the face and prevents bite attacks*/
		if (aatyp == AT_BITE || aatyp == AT_LNCK || aatyp == AT_5SBT ||
			(aatyp == AT_ENGL && !(youdef && u.uswallow)) ||
			(aatyp == AT_TENT && is_mind_flayer(magr->data)))
		{
			struct obj * helm = youagr ? uarmh : which_armor(magr, W_ARMH);
			struct obj * cloak = youagr ? uarmc : which_armor(magr, W_ARMC);
			if ((helm && (
				helm->otyp == PLASTEEL_HELM ||
				helm->otyp == CRYSTAL_HELM ||
				helm->otyp == PONTIFF_S_CROWN)) ||
				(cloak && (
				cloak->otyp == WHITE_FACELESS_ROBE ||
				cloak->otyp == BLACK_FACELESS_ROBE ||
				cloak->otyp == SMOKY_VIOLET_FACELESS_ROBE))
				)
				continue;
		}

		/* Illurien can only engulf the player, and only if she is stuck to you */
		if (pa == &mons[PM_ILLURIEN_OF_THE_MYRIAD_GLIMPSES] && aatyp == AT_ENGL &&
			(!youdef || u.ustuck != magr)) {
			continue;
		}

		/* Rend attacks only happen if the previous two attacks hit */
		/* Hugs attacks are similar, but also happen if magr and mdef are stuck together */
		if ((res[1] == MM_MISS || res[2] == MM_MISS) && (
			(attk->aatyp == AT_REND) ||
			(attk->aatyp == AT_HUGS && !((youdef && u.ustuck == magr) || (youagr && u.ustuck == mdef)))
			)) {
			continue;
		}

		/* If the player was using 'k' to kick, they are only performing kick attacks */
		/* onlykicks is a state variable defined in dokick.c */
		if (youagr && onlykicks && attk->aatyp != AT_KICK)
			continue;

		/* based on the attack type... */
		switch (aatyp)
		{
//////////////////////////////////////////////////////////////
// NON-ATTACK
//////////////////////////////////////////////////////////////
			/* non attacks, do not occur here */
		case AT_NONE:	// retaliatory when hit
		case AT_BOOM:	// retaliatory explosion when killed
			continue;
//////////////////////////////////////////////////////////////
// WEAPON
//////////////////////////////////////////////////////////////
			/* various weapon attacks */
		case AT_WEAP:	// mainhand
		case AT_DEVA:	// mainhand; can attack many times
		case AT_XWEP:	// no ranged (is done concurrently with AT_WEAP for offhanded blasters); offhand
		case AT_MARI:	// no ranged; from inventory
		case AT_HODS:	// no ranged; opponent's mainhand weapon

			/* there are cases where we can't attack at all */
			/* 1: Wielding a weapon */
			if (!youagr &&												// monster attacking
				(aatyp == AT_WEAP || aatyp == AT_DEVA) &&				// using a primary weapon attack
				(magr->weapon_check == NEED_WEAPON || !MON_WEP(magr))	// needs a weapon
				){
				/* pick appropriate weapon based on range to target */
				if (dist2(x(magr), y(magr), tarx, tary) <= 8) {
					/* melee or polearm range */
					magr->combat_mode = HNDHND_MODE;
					magr->weapon_check = NEED_HTH_WEAPON;
				}
				else {
					/* long range */
					magr->combat_mode = RANGED_MODE;
					magr->weapon_check = NEED_RANGED_WEAPON;
				}
				if (mon_wield_item(magr) != 0)				// and try to wield something (did it take time?)
				{
					continue;								// it took time, don't attack using this action
				}
			}
			/* 2: Offhand attack when not allowed */
			if (aatyp == AT_XWEP && (						// offhand attack
					(youagr && !u.twoweap) ||				// player attacking and choosing not to twoweapon
					(!youagr && (which_armor(magr, W_ARMS))	// monster attacking and cannot twoweapon (wearing shield)
					))
				) {
				continue;									// not allowed, don't attack
			}
			/*3: ranged attack with incorrect aatyp */
			if (!youagr && ranged && (						// monster's ranged attack
				aatyp == AT_XWEP ||							// subject to change; may later allow monsters to fire guns from their offhand
				aatyp == AT_MARI ||
				aatyp == AT_HODS)
				){
				continue;									// not allowed, don't attack
			}

			/* get correct weapon */
			if (aatyp == AT_WEAP || aatyp == AT_DEVA) {
				/* mainhand */
				otmp = (youagr) ? uwep : MON_WEP(magr);
			}
			else if (aatyp == AT_XWEP) {
				/* offhand */
				otmp = (youagr) ? uswapwep : MON_SWEP(magr);
			}
			else if (aatyp == AT_MARI) {
				/* randomly(ish) from inventory */
				int wcount = 0;	// valid weapons so far
				// loop through attacker's inv to find next allowable weapon to hit with
				for (otmp = (youagr ? invent : magr->minvent); otmp; otmp = otmp->nobj){
					if ((otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
						|| (otmp->otyp == CHAIN && pa == &mons[PM_CATHEZAR])
						)																	// valid weapon
						//&& !otmp->oartifact												// ~~non-artifact~~
						&& (youagr || (otmp != MON_WEP(magr) && otmp != MON_SWEP(magr)))	// not wielded already (monster)
						&& (!youagr || (otmp != uwep && (!u.twoweap || otmp != uswapwep)))	// not wielded already (player)
						&& !(is_ammo(otmp) || is_pole(otmp) || throwing_weapon(otmp))		// not unsuitable for melee (ammo, throwable, polearm)
						&& !otmp->owornmask)												// not worn
					{
						/* we have a potential weapon */
						if (wcount == marinum) {
							// found the next weapon, exit loop
							marinum++;
							break;
						}
						else {
							// not the next weapon, continue looping
							wcount++;
						}
					}
				}
			}
			else if (aatyp == AT_HODS) {
				/* defender's mainhand weapon */
				otmp = (youdef) ? uwep : MON_WEP(mdef);
			}
			/* don't make self-fatal attacks -- being at a range implies safety */
			if (be_safe && !ranged && !safe_attack(magr, mdef, attk, otmp, pa, pd))
				continue;

			/* make the attack */
			/* melee -- if attacking an adjacent square or thrusting a polearm */
			if (!ranged ||
				(otmp && is_pole(otmp) && dist2(x(magr), y(magr), tarx, tary) < 8)) {
				/* check for wild misses */
				if (missedyou) {
					wildmiss(magr, attk, otmp, ranged);
					result |= MM_AGR_STOP;	/* it knows you aren't there */
					continue;
				}
				/* print message for magr swinging weapon (m only in function) */
				if (vis)
					xswingsy(magr, mdef, otmp, ranged);
				/* do{}while(); loop for AT_DEVA */
				boolean devaloop = (aatyp == AT_DEVA);
				do {
					bhitpos.x = tarx; bhitpos.y = tary;
					result = xmeleehity(magr, mdef, attk, otmp, vis, tohitmod, ranged);
					/* Marionette causes an additional weapon strike to a monster behind the original target */
					/* this can attack peaceful/tame creatures without warning */
					if (youagr && !ranged && u.sealsActive&SEAL_MARIONETTE && (result != MM_MISS))
					{
						/* try to find direction (u.dx and u.dy may be incorrect) */
						int dx = sgn(tarx - x(magr));
						int dy = sgn(tary - y(magr));
						if (isok(tarx + dx, tary + dy) &&
							isok(tarx - dx, tary - dy) &&
							u.ux == tarx - dx &&
							u.uy == tary - dy
							)
						{
							struct monst *mdef2 = m_at(tarx + dx, tary + dy);
							if (mdef2 && (mdef2 != mdef)) {
								int vis2 = (VIS_MAGR | VIS_NONE) | (canseemon(mdef2) ? VIS_MDEF : 0);
								bhitpos.x = tarx; bhitpos.y = tary;
								(void)xmeleehity(magr, mdef2, attk, otmp, vis2, tohitmod, TRUE);
								/* we aren't handling MM_AGR_DIED or MM_AGR_STOP; hopefully the attacker being a player covers those cases well enough */
							}
						}
					}
					/* AT_DEVA considerations -- either stop looping or decrease to-hit */
					if (devaloop) {
						if (result == MM_MISS || result&(MM_DEF_DIED|MM_DEF_LSVD|MM_AGR_DIED|MM_AGR_STOP))
							devaloop = FALSE;
						else
							tohitmod -= 4;	/* reduce accuracy on repeated AT_DEVA attacks */
					}
				} while (devaloop);

				if (!ranged)
					dopassive_local = TRUE;
				/* if the attack hits, or if the creature is able to notice it was attacked (but the attack missed) it wakes up */
				if (youdef ||(!(result&MM_DEF_DIED) && (result || (!mdef->msleeping && mdef->mcanmove))))
					wakeup2(mdef, youagr);
			}
			else {
				/* make ranged attack */
				/* players should not make ranged attacks here */
				if (youagr)
					continue;
				else if (m_online(magr, mdef, tarx, tary, (magr->mtame && !magr->mconf), TRUE) &&
					distmin(x(magr), y(magr), tarx, tary) <= BOLT_LIM) {
					if (mdofire(magr, mdef, tarx, tary)) {
						/* they did do a ranged attack */
						mon_ranged_gazeonly = FALSE;
						/* monsters figure out they don't know where you are */
						if (missedyou) {
							magr->mux = magr->muy = 0;
						}
						/* note: can't tell if mdef lifesaved */
						if (*hp(mdef) < 0)
							result |= MM_DEF_DIED;

						/* if the attack was made, defender can wake up (reduced chance vs melee) */
						if ((youdef || (!(result&MM_DEF_DIED) && result)) && !rn2(3))
							wakeup2(mdef, youagr);
					}
				}
			}
			break;
//////////////////////////////////////////////////////////////
// MELEE
//////////////////////////////////////////////////////////////
			/* basic hand-to-hand attacks, check covering for hitting *trices */
		case AT_CLAW:	// 
		case AT_KICK:	// 
		case AT_BITE:	// not used by vampires against creatures with deadly corpses
		case AT_STNG:	// 
		case AT_BUTT:	// 
		case AT_TENT:	// 
		case AT_WHIP:	// 
		case AT_WISP:	// 
		case AT_HITS:	// always hits
		case AT_TUCH:	// uses touch accuracy
		case AT_SRPR:	// uses touch accuracy
		case AT_REND:	// hits if previous 2 attacks hit
		case AT_HUGS:	// hits if previous 2 attacks hit, or if magr and mdef are stuck together
			/* not in range */
			if (ranged)
				continue;
			/* don't make self-fatal attacks */
			if (be_safe && !safe_attack(magr, mdef, attk, (struct obj *)0, pa, pd))
				continue;
			/* check for wild misses */
			if (missedyou) {
				wildmiss(magr, attk, otmp, ranged);
				result |= MM_AGR_STOP;	/* it knows you aren't there */
				continue;
			}
			/* make the attack */
			bhitpos.x = tarx; bhitpos.y = tary;
			result = xmeleehity(magr, mdef, attk, (struct obj *)0, vis, tohitmod, ranged);
			dopassive_local = TRUE;
			/* if the attack hits, or if the creature is able to notice it was attacked (but the attack missed) it wakes up */
			if (youdef || (!(result&MM_DEF_DIED) && (result || (!mdef->msleeping && mdef->mcanmove))))
				wakeup2(mdef, youagr);
			break;

			/* engulfing attacks */
		case AT_ENGL:
		case AT_ILUR:	/* deprecated */
			/* don't make self-fatal attacks */
			if (be_safe && !safe_attack(magr, mdef, attk, (struct obj *)0, pa, pd))
				continue;
			/* cannot swallow huge or larger */
			if (pd->msize >= MZ_HUGE)
				continue;
			/* check for wild misses */
			if (missedyou) {
				wildmiss(magr, attk, otmp, ranged);
				result |= MM_AGR_STOP;	/* it knows you aren't there */
				continue;
			}
			/* make the attack */
			result = xengulfhity(magr, mdef, attk, vis);
			break;

			/* explodes, killing itself */
		case AT_EXPL:
			/* not in range */
			if (ranged)
				continue;
			/* a monster exploding must not be cancelled */
			if (!youagr && magr->mcan)
				continue;
			/* explode -- this function handles wild misses */
			result = xexplodey(magr, mdef, attk, vis);
			break;

//////////////////////////////////////////////////////////////
// RANGED
//////////////////////////////////////////////////////////////

			/* ranged on-line attacks */
		case AT_BREA:	// breath attack, can be draconic
		case AT_BEAM:	// breath attack but a like a beam wand so it can't be reflected.
						// Maybe should be axed and rolled into AT_BREA; only uses AD_WET and AD_DRLI
		case AT_SPIT:	// spit venom
		case AT_ARRW:	// fire an arrow/boulder/etc; can be fired in close quarters (except for AD_SHDW bolts)
			/* these are all monster only -- players can activate these via #monster or 'f'iring with an empty quiver */
			if (youagr)
				continue;

			/* check line of fire to target -- this includes being on line, line of sight, and friendly fire */
			if (!m_online(magr, mdef, tarx, tary,
					(magr->mtame && !magr->mconf),		/* pets try to be safe with ranged attacks if they aren't confused */
					(aatyp == AT_BREA ? FALSE : TRUE)))	/* breath attacks overpenetrate targets */
				continue;
			
			switch (aatyp) {
			case AT_BREA:
				if (ranged && !magr->mspec_used && (distmin(x(magr), y(magr), tarx, tary) <= BOLT_LIM) && rn2(3)) {	// not in melee, 2/3 chance when ready
					if (result = xbreathey(magr, attk, tarx, tary)) {
						/* they did do a breath attack */
						mon_ranged_gazeonly = FALSE;
						/* monsters figure out they don't know where you are */
						if (missedyou) {
							magr->mux = magr->muy = 0;
						}
					}
				}
				break;
			case AT_BEAM:
				/* lowest effort ranged attack -- goes straight to melee damage */
				if (ranged && (distmin(x(magr), y(magr), tarx, tary) <= BOLT_LIM)) {
					mon_ranged_gazeonly = FALSE;
					result = xmeleehurty(magr, mdef, attk, (struct obj *)0, TRUE, -1, rn1(18, 2), vis, ranged);
				}
				break;
			case AT_SPIT:
				if (!magr->mspec_used && !rn2(BOLT_LIM - distmin(x(magr), y(magr), tarx, tary))) {	// distance in 8 chance when ready
					if (result = xspity(magr, attk, tarx, tary)) {
						/* they did spit */
						mon_ranged_gazeonly = FALSE;
						/* monsters figure out they don't know where you are */
						if (missedyou) {
							magr->mux = magr->muy = 0;
						}
					}
				}
				break;
			case AT_ARRW:
				if ((adtyp != AD_SHDW || ranged)) {	// can be used in melee range, except for shadow
					int n;
					/* message -- done outside of xfirey to only print 1 message for d(n,d) function calls */
					/* potential change: pass a domessage variable to xfirey */
					if (canseemon(magr)) {
						pline("%s shoots!", Monnam(magr));
					}
					/* fire d(n,d) projectiles */
					for (n = d(attk->damn, attk->damd); n > 0; n--)
						result |= xfirey(magr, attk, tarx, tary);
					if (result) {
						/* they did fire at least one projectile */
						mon_ranged_gazeonly = FALSE;
						/* monsters figure out they don't know where you are */
						if (missedyou) {
							magr->mux = magr->muy = 0;
						}
					}
				}
				break;
			}

			/* note: can't tell if mdef lifesaved */
			if (*hp(mdef) < 1)
				result |= MM_DEF_DIED;

			/* if the attack was made, defender can wake up (reduced chance vs melee) */
			if ((youdef || (!(result&MM_DEF_DIED) && result)) && !rn2(3))
				wakeup2(mdef, youagr);
			break;

			/* ranged maybe-not-on-line attacks */
		case AT_LNCK:	// range 2 bite
		case AT_5SBT:	// range 5 bite
		case AT_LRCH:	// range 2 touch
		case AT_5SQR:	// range 5 touch
			/* don't make attacks that will kill oneself */
			if (be_safe && !safe_attack(magr, mdef, attk, (struct obj *)0, pa, pd))
				continue;
			/* requires a clear path */
			if (!clear_path(x(magr), y(magr), tarx, tary))
				continue;
			/* must be in range */
			if (distmin(x(magr), y(magr), tarx, tary) > 
				((aatyp == AT_5SBT || aatyp == AT_5SQR) ? 5 : 2))
				continue;
			/* check for wild misses */
			if (missedyou) {
				wildmiss(magr, attk, otmp, ranged);
				result |= MM_AGR_STOP;	/* it knows you aren't there */
				continue;
			}
			/* make the attack */
			bhitpos.x = tarx; bhitpos.y = tary;
			result = xmeleehity(magr, mdef, attk, (struct obj *)0, vis, tohitmod, ranged);
			if (distmin(x(magr), y(magr), tarx, tary) == 1)
				dopassive_local = TRUE;
			/* if the attack hits, or if the creature is able to notice it was attacked (but the attack missed) it wakes up */
			if (youdef || (!(result&MM_DEF_DIED) && (result || (!mdef->msleeping && mdef->mcanmove))))
				wakeup2(mdef, youagr);
			break;

			/* targeted gazes */
		case AT_GAZE:
			/* Weeping angels gaze during movemon(). */
			if (is_weeping(pa)) {
				continue;
			}

			if (youagr) {
				/* not done as part of the player's attack chain, use #monster */
				continue;
			}
			else {
				result = xgazey(magr, mdef, attk, vis);
			}
			break;
			/* wide (passive) gaze */
		case AT_WDGZ:
			/* these happen every turn and are NOT done here; they're somewhere in dochug and allmain? */
			continue;
			/* tinkering */
		case AT_TNKR:
			/* players can use #monster to tinker (thanks, Demo!) -- does nothing in the player's attack chain */
			if (youagr)
				continue;
			/* must be done at a range */
			if (!ranged)
				continue;
			/* do the tinkering */
			result = xtinkery(magr, mdef, attk, vis);
			break;
			/* magic */
		case AT_MAGC:
		case AT_MMGC:
			/* the player cannot cast AT_MMGC (monster-only-magic) */
			if (youagr && aatyp == AT_MMGC)
				continue;
			/* Demogorgon casts spells less frequently in melee range */
			if (pa == &mons[PM_DEMOGORGON] && !ranged && rn2(6) && !(youagr || magr->mflee))
				continue;

			result = xcasty(magr, mdef, attk, vis);

			/* Asmodeus randomly stops his casting early? */
			if (pa == &mons[PM_ASMODEUS] && !rn2(3))
				result |= MM_AGR_STOP;
			break;
		}//switch (aatyp)

//////////////////////////////////////////////////////////////
// AFTER THE ATTACK IS DONE
//////////////////////////////////////////////////////////////

		//DEBUG: attack chain ended reasons
		//if (res[0] & MM_DEF_DIED)
		//	pline("def died");
		//if(res[0] & MM_DEF_LSVD)
		//	pline("def lifesaved");
		//if(res[0] & MM_AGR_DIED)
		//	pline("agr died");
		//if(res[0] & MM_AGR_STOP)
		//	pline("agr stopped");

		/* make per-attack counterattacks */
		if (dopassive_local) {
			dopassive = TRUE;
			result = xpassivey(magr, mdef, attk, otmp, vis, result, pd, FALSE);
		}

		/* double check MM_DEF_DIED */
		if (*hp(mdef) < 1)
			result |= MM_DEF_DIED;

		/* increment number of attacks made */
		attacksmade++;
		/* save result to res, allres */
		res[0] = result;
		allres |= result;

	}while (!(
		(youagr && attacklimit && attacksmade >= attacklimit) ||	/* player wants to only attack N times */
		(result & MM_DEF_DIED) ||									/* defender died */
		(result & MM_DEF_LSVD) ||									/* defender lifesaved */
		(result & MM_AGR_DIED) ||									/* attacker died */
		(result & MM_AGR_STOP) ||									/* attacker stopped for some other reason */
		is_null_attk(attk)											/* no more attacks */
		));

	/* make per-round counterattacks -- note that these cannot use otmp or attk, as those are per-attack */
	if (dopassive)
		result = xpassivey(magr, mdef, (struct attack *)0, (struct obj *)0, vis, allres, pd, TRUE);
	
	/* reset lillend mask usage */
	if (!youagr && pa == &mons[PM_LILLEND])
		magr->mvar2 = 0;

	if (attacksmade > 0)
		allres |= MM_HIT;		/* signifies that the attack action was indeed taken, even if no attacks hit */

	if(youdef && is_aquatic(magr->data) && roll_madness(MAD_THALASSOPHOBIA)){
		You("panic after being attacked by a sea monster!");
		nomul(-1*rnd(6), "panicking");
	}
	
	return allres;
}// xattacky

/*
 * wildmiss()
 *
 * Monster is trying to attack player (with a melee attack) but doesn't know exactly where they are.
 * Prints a message
 */
void
wildmiss(magr, attk, otmp, ranged)
struct monst * magr;
struct attack * attk;
struct obj * otmp;
boolean ranged;
{
	boolean compat = (attk->adtyp == AD_SEDU || attk->adtyp == AD_SSEX || attk->adtyp == AD_LSEX) && could_seduce(magr, &youmonst, (struct attack *)0);
	
	/* verbose message */
	if (!flags.verbose)
		return;

	/* you have to be able to see magr */
	if (!canseemon(magr))
		return;

	/* Print message */
	if (Displaced) {
		if (compat) {
			pline("%s smiles %s at your %sdisplaced image...",
				Monnam(magr),
				compat == 2 ? "engagingly" : "seductively",
				Invis ? "invisible " : "");
		}
		else {
			pline("%s strikes at your %sdisplaced image and misses you!",
				/* Note: if you're both invisible and displaced,
				 * only monsters which see invisible will attack your
				 * displaced image, since the displaced image is also
				 * invisible.
				 */
				Monnam(magr),
				Invis ? "invisible " : "");
		}
	}
	else if (Underwater) {
		/* monsters may miss especially on water level where
		bubbles shake the player here and there */
		if (compat) {
			pline("%s reaches towards your distorted image.", Monnam(magr));
		}
		else {
			pline("%s is fooled by water reflections and misses!", Monnam(magr));
		}
	}
	else if (magr->mcrazed) {
		pline("%s flails around randomly.", Monnam(magr));
	}
	else if (ranged && otmp) {
		pline("%s %s %s wildly.", Monnam(magr), otmp->otyp == AKLYS ? "throws" : "thrusts",
			obj_is_pname(otmp) ? the(xname(otmp)) : an(xname(otmp)));
	}
	else {
		const char *swings =
			attk->aatyp == AT_BITE || attk->aatyp == AT_LNCK || attk->aatyp == AT_5SBT ? "snaps" :
			attk->aatyp == AT_KICK ? "kicks" :
			(attk->aatyp == AT_STNG ||
			attk->aatyp == AT_BUTT ||
			nolimbs(magr->data)) ? "lunges" : "swings";

		if (compat) {
			pline("%s tries to touch you and misses!", Monnam(magr));
		}
		else {
			switch (rn2(3)) {
			case 0: pline("%s %s wildly and misses!", Monnam(magr),
				swings);
				break;
			case 1: pline("%s attacks a spot beside you.", Monnam(magr));
				break;
			case 2: pline("%s strikes at %s!", Monnam(magr),
				((int)levl[magr->mux][magr->muy].typ) == WATER
				? "empty water" : "thin air");
				break;
			default:pline("%s %s wildly!", Monnam(magr), swings);
				break;
			}
		}
	}
	return;
}


/*
 * u_surprise()
 *
 * A monster tried to move onto your square while you were hidden there! Surprise!
 */
boolean
u_surprise(mtmp, youseeit)
struct monst * mtmp;
boolean youseeit;
{
	if (u.uundetected) {
		u.uundetected = 0;
		if (is_hider(youracedata)) {
			coord cc; /* maybe we need a unexto() function? */
			struct obj *obj;

			You("fall from the %s!", ceiling(u.ux, u.uy));
			if (enexto(&cc, u.ux, u.uy, youracedata)) {
				remove_monster(mtmp->mx, mtmp->my);
				newsym(mtmp->mx, mtmp->my);
				place_monster(mtmp, u.ux, u.uy);
				if (mtmp->wormno) worm_move(mtmp);
				teleds(cc.x, cc.y, TRUE);
				set_apparxy(mtmp);
				newsym(u.ux, u.uy);
			}
			else {
				pline("%s is killed by a falling %s (you)!",
					Monnam(mtmp), youracedata->mname);
				killed(mtmp);
				newsym(u.ux, u.uy);
				if (mtmp->mhp > 0) return 0;
				else return 1;
			}
			if (youracedata->mlet != S_PIERCER)
				return(0);	/* trappers don't attack */

			obj = which_armor(mtmp, WORN_HELMET);
			if (obj && is_hard(obj)) {
				Your("blow glances off %s helmet.",
					s_suffix(mon_nam(mtmp)));
			}
			else {
				if (3 + find_mac(mtmp) <= rnd(20)) {
					pline("%s is hit by a falling piercer (you)!",
						Monnam(mtmp));
					if ((mtmp->mhp -= d(3, 6)) < 1)
						killed(mtmp);
				}
				else
					pline("%s is almost hit by a falling piercer (you)!",
					Monnam(mtmp));
			}
		}
		else {
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
					(youracedata->mlet == S_EEL && is_pool(u.ux, u.uy, FALSE))) {
					int save_spe = 0; /* suppress warning */
					if (obj) {
						save_spe = obj->spe;
						if (obj->otyp == EGG) obj->spe = 0;
					}
					if (youracedata->mlet == S_EEL)
						pline("Wait, %s!  There's a hidden %s named %s there!",
						m_monnam(mtmp), youracedata->mname, plname);
					else
						pline("Wait, %s!  There's a %s named %s hiding under %s!",
						m_monnam(mtmp), youracedata->mname, plname,
						doname(level.objects[u.ux][u.uy]));
					if (obj) obj->spe = save_spe;
				}
				else
					impossible("hiding under nothing?");
			}
			newsym(u.ux, u.uy);
		}
		return(0);
	}

	/* mimicry */
	if (youracedata->mlet == S_MIMIC && youmonst.m_ap_type) {
		if (!youseeit) pline("It gets stuck on you.");
		else pline("Wait, %s!  That's a %s named %s!",
			m_monnam(mtmp), youracedata->mname, plname);
		u.ustuck = mtmp;
		youmonst.m_ap_type = M_AP_NOTHING;
		youmonst.mappearance = 0;
		newsym(u.ux, u.uy);
		return(0);
	}

	/* player might be mimicking an object (without being a proper mimic) */
	if (youmonst.m_ap_type == M_AP_OBJECT) {
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
				Upolyd ? (const char *)an(youracedata->mname) :
				(const char *) "yourself");
			unmul(buf);	/* immediately stop mimicking */
		}
		return 0;
	}
}


/* getnextspiritattack()
 *
 * Returns the next spirit attack the player will use
 * Uses a static int to cycle through the attacks, which is reset to 0 when [fresh] is true
 */
struct attack *
getnextspiritattack(fresh)
boolean fresh;
{
#define ATTK_AMON		0
#define ATTK_CHUPOCLOPS	1
#define ATTK_IRIS		2
#define ATTK_NABERIUS	3
#define ATTK_OTIAX		4
#define ATTK_SIMURGH	5
#define ATTK_MISKA_ARM	6
#define ATTK_MISKA_WOLF	7
#define PASV_SHADOW_WEB	8
#define PASV_ECHIDNA	9
#define PASV_FAFNIR		10
#define PASV_EDEN		11
	static struct attack spiritattack[] =
	{
		{ AT_BUTT, AD_PHYS, 1, 9 },
		{ AT_BITE, AD_DRST, 2, 4 },
		{ AT_TENT, AD_IRIS, 1, 4 },
		{ AT_BITE, AD_NABERIUS, 1, 6 },
		{ AT_WISP, AD_OTIAX, 1, 5 },
		{ AT_CLAW, AD_SIMURGH, 1, 6 },
		{ AT_MARI, AD_PHYS, 1, 8 },
		{ AT_BITE, AD_DRST, 2, 4 },
		{ AT_NONE, AD_SHDW, 4, 8 },
		{ AT_NONE, AD_ACID, 1, 1 },	/* actually 1d(spiritDsize) */
		{ AT_NONE, AD_FIRE, 1, 1 }, /* actually 1d(spiritDsize) */
		{ AT_NONE, AD_SLVR, 1, 20 }
	};
	int i;					/* loop counter */
	static int indexnum;	/* which attack index to return -- kept between calls of this function */
	int curindex = 0;		/* which attack index has been reached */

	/* possibly reset indexnum */
	if (fresh)
		indexnum = 0;
	/* increment indexnum */
	indexnum++;

	/* Amon */
	if (u.sealsActive&SEAL_AMON) {
		if (++curindex == indexnum)
			return &spiritattack[ATTK_AMON];
	}
	/* Chupoclops */
	if (u.sealsActive&SEAL_CHUPOCLOPS) {
		if (++curindex == indexnum)
			return &spiritattack[ATTK_CHUPOCLOPS];
	}
	/* Naberius */
	if (u.sealsActive&SEAL_NABERIUS) {
		if (++curindex == indexnum)
			return &spiritattack[ATTK_NABERIUS];
	}
	/* Simurgh */
	if (u.sealsActive&SEAL_SIMURGH) {
		if (++curindex == indexnum)
			return &spiritattack[ATTK_SIMURGH];
	}
	/* Miska */
	if (u.specialSealsActive&SEAL_MISKA){
		/* poison bite */
		if (u.ulevel >= 10) {
			if (++curindex == indexnum)
				return &spiritattack[ATTK_MISKA_WOLF];
		}
		/* poison bite */
		if (u.ulevel >= 18) {
			if (++curindex == indexnum)
				return &spiritattack[ATTK_MISKA_WOLF];
		}
		/* 2x non-wielded weapon */
		if (u.ulevel >= 26) {
			for (i = 0; i < 2; i++)
			if (++curindex == indexnum)
				return &spiritattack[ATTK_MISKA_ARM];
		}
	}
	/* Iris */
	if (u.sealsActive&SEAL_IRIS){
		if (++curindex == indexnum)
			return &spiritattack[ATTK_IRIS];

		if (u.twoweap || (uwep && bimanual(uwep, youracedata))){
			if (++curindex == indexnum)
				return &spiritattack[ATTK_IRIS];
		}
		if (u.specialSealsActive&SEAL_MISKA && u.ulevel >= 26){
			/* +2 */
			for (i = 0; i < 2; i++)
			if (++curindex == indexnum)
				return &spiritattack[ATTK_IRIS];
		}
		if (youracedata == &mons[PM_MARILITH]){
			/* +4 */
			for (i = 0; i < 4; i++)
			if (++curindex == indexnum)
				return &spiritattack[ATTK_IRIS];

		}
	}
	/* Otiax */
	if (u.sealsActive&SEAL_OTIAX){
		static int tendrils;
		/* on the first call reaching here, determine how many tendrils to use */
		if (curindex + 1 == indexnum)
			tendrils = min(rnd(5), spiritDsize());

		/* make that many attacks */
		for (i = 0; i < tendrils; i++)
		if (++curindex == indexnum) {
			return &spiritattack[ATTK_OTIAX];
		}
	}
	/* Passives! */
	if (u.specialSealsActive&SEAL_BLACK_WEB && u.spiritPColdowns[PWR_WEAVE_BLACK_WEB] > moves + 20)
	{
		if (++curindex == indexnum)
			return &spiritattack[PASV_SHADOW_WEB];
	}
	if (u.sealsActive&SEAL_ECHIDNA)
	{
		spiritattack[PASV_ECHIDNA].damd = spiritDsize();
		if (++curindex == indexnum)
			return &spiritattack[PASV_ECHIDNA];
	}
	if (u.sealsActive&SEAL_FAFNIR)
	{
		spiritattack[PASV_FAFNIR].damd = spiritDsize();
		if (++curindex == indexnum)
			return &spiritattack[PASV_FAFNIR];
	}
	if (u.sealsActive&SEAL_EDEN)
	{
		if (++curindex == indexnum)
			return &spiritattack[PASV_EDEN];
	}
		
	/* Default case: no attack */
	return &noattack;
}

/* getattk()
 * 
 * Returns the [*indexnum] attack for [magr]
 * 
 * Uses a lot of int pointers because there is no guarantee that successive calls of getattk() will be made
 * by the same attacker -- counterattacks specifically cause this, which prevents the use of static variables
 * inside the function
 */
struct attack *
getattk(magr, prev_res, indexnum, prev_and_buf, by_the_book, subout, tohitmod)
struct monst * magr;			/* attacking monster */
int * prev_res;					/* results of previous attacks; ignored if by_the_book is true */
int * indexnum;					/* index number to use, incremented HERE (if actually pulling the attack from the monster's index) */
struct attack * prev_and_buf;	/* double-duty pointer: 1st, is the previous attack; 2nd, is a pointer to allocated memory */
boolean by_the_book;			/* if true, gives the "standard" attacks for [magr]. Useful for the pokedex. */
int * subout;					/* records what attacks have been subbed out */
#define SUBOUT_UNDEAD	0x0001	/* derived undead special attack */
#define SUBOUT_FIENDS	0x0002	/* Five Fiends of Chaos1 spellcasting */
#define SUBOUT_BAEL1	0x0004	/* Bael's Sword Archon attack chain */
#define SUBOUT_BAEL2	0x0008	/* Bael's marilith-hands attack chain */
#define SUBOUT_DEMO1	0x0010	/* Demogorgon's shredding rend */
#define SUBOUT_DEMO2	0x0020	/* Demogorgon's item steal */
#define SUBOUT_SPIRITS	0x0040	/* Player's bound spirits */
#define SUBOUT_BARB1	0x0080	/* 1st bit of barbarian bonus attacks */
#define SUBOUT_BARB2	0x0100	/* 2nd bit of barbarian bonus attacks, must directly precede the 1st bit */
#define SUBOUT_MAINWEPB	0x0200	/* Bonus attack caused by the wielded *mainhand* weapon */
#define SUBOUT_XWEP		0x1000	/* when giving additional attacks, whether or not to use AT_XWEP or AT_WEAP this call */
int * tohitmod;					/* some attacks are made with decreased accuracy */
{
	struct attack * attk;
	struct attack prev_attack = *prev_and_buf;
	boolean youagr = (magr == &youmonst);
	boolean fromlist;
	struct permonst * pa = youagr ? youracedata : magr->data;

	/* lillends are able to use the attacks of another monster */
	/* modify pa here, but only here (when getting attacks) */
	if (pa == &mons[PM_LILLEND] && magr->mvar2) {
		pa = &mons[magr->mvar2];
	}

	if (*indexnum < NATTK) {
		/* get the nth attack */
		attk = &(pa->mattk[*indexnum]);
		fromlist = TRUE;
		*prev_and_buf = *attk;
		attk = prev_and_buf;
	}
	else {
		/* possibly add additional attacks */
		attk = prev_and_buf;
		*attk = noattack;
		fromlist = FALSE;
	}

	/* unpolymorphed players get changes */
	if (youagr && !Upolyd && fromlist) {
		/* do NOT get permonst-inherent attacks, except for: */
		if (!is_null_attk(attk) && !(
			(*indexnum == 0) ||												/* first attack, HOPEFULLY a weapon attack! */
			(Race_if(PM_YUKI_ONNA) && (!uwep || attk->aatyp == AT_NONE)) ||	/* yuki-onna get their additional attacks when unarmed, and their passive always */
			(Race_if(PM_VAMPIRE)) ||
			(Race_if(PM_CHIROPTERAN))
			)){
			/* just get the next attack */
			*indexnum += 1;
			*prev_and_buf = prev_attack;
			fromlist = TRUE;
			return getattk(magr, prev_res, indexnum, prev_and_buf, by_the_book, subout, tohitmod);
		}

		/* if twoweaponing, make an xwep attack after each weap attack, if it isn't in the inherent attack chain */
		if (!by_the_book && *indexnum > 0 && (prev_res[1] != MM_MISS) && prev_attack.aatyp == AT_WEAP && attk->aatyp != AT_XWEP && u.twoweap) {
			fromlist = FALSE;
			attk->aatyp = AT_XWEP;
			attk->adtyp = AD_PHYS;
			attk->damn = 1;
			attk->damd = 4;
		}
	}

	/* players sub out monster claw attacks for weapon attacks */
	if (youagr && !cantwield(pa)) {
		if ((*indexnum == 0 || (*indexnum == 1 && (pa == &mons[PM_INCUBUS] || pa == &mons[PM_SUCCUBUS]))) &&
			(attk->aatyp == AT_CLAW || (attk->aatyp == AT_TUCH && pa->mlet == S_LICH && uwep)))
		{
			attk->aatyp = AT_WEAP;
			attk->adtyp = AD_PHYS;
			attk->damn = 1;	// unused
			attk->damd = 4;	// unused
		}
	}
	/* Grue does not make its later attacks if its square is lit */
	if (pa == &mons[PM_GRUE] &&
		!by_the_book &&
		*indexnum >= 2 &&
		!isdark(x(magr), y(magr))
		) {
			return &noattack;
	}

	/* Alabaster mummies:
	 * Spell glyphs result in spellcasting,
	 * Physical glyphs result in melee,
	 * Non-specific glyphs result in random choice
	 */
	if (!youagr && *indexnum == 1 && is_alabaster_mummy(pa) && magr->mvar1){
		switch (magr->mvar1){
		case SYLLABLE_OF_STRENGTH__AESH:
		case SYLLABLE_OF_GRACE__UUR:
			// no changes
			break;
		case SYLLABLE_OF_POWER__KRAU:
		case SYLLABLE_OF_THOUGHT__NAEN:
			attk->aatyp = AT_MAGC;
			attk->adtyp = AD_SPEL;
			attk->damn = 0;
			attk->damd = 4;
			break;
		default:
			if (magr->m_id % 2){
				attk->aatyp = AT_MAGC;
				attk->adtyp = AD_SPEL;
				attk->damn = 0;
				attk->damd = 4;
			}
			break;
		}
	}

	/* the Five Fiends spellcasting -- not shown in pokedex */
	if (!by_the_book && (
		(pa == &mons[PM_LICH__THE_FIEND_OF_EARTH]) ||
		(pa == &mons[PM_KARY__THE_FIEND_OF_FIRE]) ||
		(pa == &mons[PM_KRAKEN__THE_FIEND_OF_WATER]) ||
		(pa == &mons[PM_TIAMAT__THE_FIEND_OF_WIND]) ||
		(pa == &mons[PM_CHAOS]))
		){
		// first index -- determing if using the alternate attack set (solo spellcasting)
		if (*indexnum == 0){
			if (
				(pa == &mons[PM_LICH__THE_FIEND_OF_EARTH] && rn2(4)) ||
				(pa == &mons[PM_KARY__THE_FIEND_OF_FIRE] && rn2(100)<37) ||
				(pa == &mons[PM_KRAKEN__THE_FIEND_OF_WATER] && rn2(100)<52) ||
				(pa == &mons[PM_TIAMAT__THE_FIEND_OF_WIND] && !rn2(4)) ||
				(pa == &mons[PM_CHAOS] && rn2(3))
				){
				*subout |= SUBOUT_FIENDS;
				attk->aatyp = AT_MAGC;
				attk->adtyp = AD_SPEL;
				attk->damn = 0;
				attk->damd = 0;
			}
		}
		else if (*subout&SUBOUT_FIENDS){
			/* If spellcasting, stop after the first index */
			return &noattack;
		}
	}
	/* Bael has alternate attack routines -- not shown in pokedex */
	if (pa == &mons[PM_BAEL] && *indexnum < NATTK && !by_the_book) {
		static const struct attack marilithHands[6] = {
			{ AT_MARI, AD_PHYS, 1, 15 },
			{ AT_MARI, AD_PHYS, 1, 15 },
			{ AT_MARI, AD_PHYS, 1, 15 },
			{ AT_MARI, AD_PHYS, 1, 15 },
			{ AT_MARI, AD_PHYS, 1, 15 },
			{ AT_MARI, AD_PHYS, 1, 15 }
		};
		static const struct attack swordArchon[6] = {
			{ AT_CLAW, AD_ACFR, 3, 7 },
			{ AT_CLAW, AD_ACFR, 3, 7 },
			{ AT_REND, AD_DISN, 7, 1 },
			{ AT_BUTT, AD_EFIR, 9, 1 },
			{ AT_BITE, AD_POSN, 9, 1 },
			{ AT_GAZE, AD_STDY, 1, 9 }
		};
		// first index -- determine which attack form
		if (*indexnum == 0){
			if (!rn2(7)){		// 1/7 of sword archon
				*subout |= SUBOUT_BAEL1;
			}
			else if (!rn2(6)){	// 1/7 of marilith-hands
				*subout |= SUBOUT_BAEL2;
			}
			//else;				// 5/7 of normal
		}
		// If using marilith-hands or sword archon, sub out entire attack chain
		if (*subout & SUBOUT_BAEL1){
			*attk = swordArchon[*indexnum];
		}
		else if (*subout & SUBOUT_BAEL2){
			*attk = marilithHands[*indexnum];
		}
	}
	/* Demogorgon gets additional shred and steal attacks, which puts him over 6 attacks */
	if (pa == &mons[PM_DEMOGORGON]) {
		/* rend */
		if (*indexnum == 2 && fromlist && !(*subout&SUBOUT_DEMO1)) {
			*subout |= SUBOUT_DEMO1;
			attk->aatyp = AT_REND;
			attk->adtyp = AD_SHRD;
			attk->damn = 3;
			attk->damd = 12;
			fromlist = FALSE;
		}
		/* steal */
		if (*indexnum == 4 && fromlist && !(*subout&SUBOUT_DEMO2)) {
			*subout |= SUBOUT_DEMO2;
			attk->aatyp = AT_REND;
			attk->adtyp = AD_SEDU;
			attk->damn = 1;
			attk->damd = 1;
			fromlist = FALSE;
		}
	}
	/* Various weapons can cause an additional full attack to be made */
	/* This only works if it is wielded in the mainhand, and a weapon attack is being made with it (AT_WEAP, AT_DEVA) */
	/* not shown in pokedex */
	if ((attk->aatyp == AT_WEAP || attk->aatyp == AT_DEVA) && !(*subout&SUBOUT_MAINWEPB) && !by_the_book) {
		/* get weapon */
		struct obj * otmp = (youagr ? uwep : MON_WEP(magr));
		/* continue checking conditions */
		if (otmp && (
			(otmp->oartifact == ART_STAFF_OF_TWELVE_MIRRORS) ||
			(otmp->oartifact == ART_QUICKSILVER && mlev(magr) > 15) ||
			(youagr && martial_bonus() && otmp->otyp == QUARTERSTAFF && P_SKILL(P_QUARTERSTAFF) >= P_EXPERT && P_SKILL(P_BARE_HANDED_COMBAT) >= P_EXPERT)
			)){
			/* make additional attacks */
			attk->aatyp = (*subout&SUBOUT_XWEP) ? AT_XWEP : AT_WEAP;
			attk->adtyp = AD_PHYS;
			attk->damn = 1;
			attk->damd = 4;
			if (otmp->oartifact == ART_QUICKSILVER)
				*tohitmod = -15;
			fromlist = FALSE;
			if (*subout&SUBOUT_XWEP) {
				*subout |= SUBOUT_MAINWEPB;
				*subout &= ~SUBOUT_XWEP;
			}
			else {
				*subout |= SUBOUT_XWEP;
			}
		}
	}

	/* Unpolyed player Barbarians get up to 3 bonus attacks (and offhand attacks) */
	/* Would be shown in pokedex, if you could look up yourself in it. */
	if (youagr && !Upolyd && Role_if(PM_BARBARIAN) && (is_null_attk(attk) || prev_attack.aatyp == AT_WEAP || prev_attack.aatyp == AT_XWEP)) {
		int nattacks = (u.ulevel >= 11) + (u.ulevel >= 21) + (u.ulevel >= 30);
		/* note: this code assumes subout_barb1 and barb2 are sequential bits, as it uses them like a tiny int */
		int attacknum = (*subout&(SUBOUT_BARB1|SUBOUT_BARB2)) / SUBOUT_BARB1;

		if (attacknum < nattacks)
		{
			attk->aatyp = (*subout&SUBOUT_XWEP) ? AT_XWEP : AT_WEAP;
			attk->adtyp = AD_PHYS;
			attk->damn = 1;
			attk->damd = 4;
			*tohitmod = -10 * attacknum;
			fromlist = FALSE;
			if (*subout&SUBOUT_XWEP) {
				*subout |= ((SUBOUT_BARB1 & (*subout&SUBOUT_BARB1)) ? SUBOUT_BARB2 : 0);
				*subout ^= SUBOUT_BARB1;
				*subout &= ~SUBOUT_XWEP;
			}
			else {
				*subout |= SUBOUT_XWEP;
			}
		}
	}

	/* players with the Black Web Entity bound replace unarmed punches with shadow-blade attacks */
	if (youagr && u.specialSealsActive&SEAL_BLACK_WEB && !by_the_book) {
		if ((attk->aatyp == AT_WEAP && !uwep) ||
			(attk->aatyp == AT_XWEP && !uswapwep && u.twoweap)) {
			/* replace the attack */
			attk->aatyp = AT_SRPR;
			attk->adtyp = AD_SHDW;
			attk->damn = 4;
			attk->damd = 8;
			/* this is applied to all acceptable attacks; no subout marker is necessary */
		}	
	}

	/* players can get a whole host of spirit attacks */
	if (youagr && is_null_attk(attk) && !by_the_book) {
		/* this assumes that getattk() will not be interrupted with youagr when already called with youagr */
		if (!(*subout&SUBOUT_SPIRITS)) {
			attk = getnextspiritattack(TRUE);
			*subout |= SUBOUT_SPIRITS;
		}
		else {
			attk = getnextspiritattack(FALSE);
		}
		fromlist = FALSE;
	}

	/* derived undead skip some attacks */
	if (!youagr && (((
		magr->mfaction == ZOMBIFIED ||
		magr->mfaction == SKELIFIED ||
		magr->mfaction == CRYSTALFIED) && (
		attk->aatyp == AT_SPIT ||
		attk->aatyp == AT_BREA ||
		attk->aatyp == AT_GAZE ||
		attk->aatyp == AT_ARRW ||
		attk->aatyp == AT_MMGC ||
		attk->aatyp == AT_TNKR ||
		attk->aatyp == AT_SRPR ||
		attk->aatyp == AT_BEAM ||
		attk->aatyp == AT_MAGC ||
		(attk->aatyp == AT_TENT && magr->mfaction == SKELIFIED))
		) || ((
		magr->mfaction == FRACTURED) && (
		attk->aatyp == AT_GAZE ||
		attk->aatyp == AT_WDGZ)
		))) {
		/* just get the next attack */
		*indexnum += 1;
		*prev_and_buf = prev_attack;
		fromlist = TRUE;
		return getattk(magr, prev_res, indexnum, prev_and_buf, by_the_book, subout, tohitmod);
	}
	/* derived undead modify some attacks, and add attacks to the end of their attack chain */
	if (!youagr) {
#define ok_undead_subout (!(*subout&SUBOUT_UNDEAD) && (is_null_attk(attk) || (attk->aatyp > AT_HUGS && fromlist)))
		/* skeletons get a paralyzing touch */
		if (magr->mfaction == SKELIFIED && ok_undead_subout) {
			*subout |= SUBOUT_UNDEAD;
			fromlist = FALSE;
			attk->aatyp = AT_TUCH;
			attk->adtyp = AD_SLOW;
			attk->damn = 1;
			attk->damd = max(magr->data->msize * 2, 4);
		}
		/* vitreans get a cold touch */
		if (magr->mfaction == CRYSTALFIED && ok_undead_subout) {
			*subout |= SUBOUT_UNDEAD;
			fromlist = FALSE;
			attk->aatyp = AT_TUCH;
			attk->adtyp = AD_ECLD;
			attk->damn = min(10, magr->m_lev / 3);
			attk->damd = 8;
		}
		/* vampires' bites become vampiric */
		if (magr->mfaction == VAMPIRIC && (
			attk->aatyp == AT_BITE ||
			ok_undead_subout)
			){
			*subout |= SUBOUT_UNDEAD;
			if (attk->aatyp != AT_BITE)
				fromlist = FALSE;
			attk->aatyp = AT_BITE;
			attk->adtyp = AD_VAMP;
			attk->damn = max(1, attk->damn);
			attk->damd = max(4, max(magr->data->msize * 2, attk->damd));
		}
		/* fractured's claws become mirror-glass shards */
		if (magr->mfaction == FRACTURED && (
			(attk->aatyp == AT_CLAW && (
			attk->adtyp == AD_PHYS ||
			attk->adtyp == AD_SQUE ||
			attk->adtyp == AD_SAMU
			)) ||
			ok_undead_subout
			)) {
			*subout |= SUBOUT_UNDEAD;
			if (attk->aatyp != AT_CLAW)
				fromlist = FALSE;
			attk->aatyp = AT_CLAW;
			attk->adtyp = AD_GLSS;
			attk->damn = max(magr->m_lev / 10 + 1, attk->damn);
			attk->damd = max(magr->data->msize * 2, max(attk->damd, 4));
		}
		/* pseudonatural's bites become int-draining tentacles */
		if (magr->mfaction == PSEUDONATURAL && (
			attk->aatyp == AT_BITE ||
			ok_undead_subout)
			){
			*subout |= SUBOUT_UNDEAD;
			if (attk->aatyp != AT_BITE)
				fromlist = FALSE;
			attk->aatyp = AT_TENT;
			attk->adtyp = AD_DRIN;
			attk->damn = 1;
			attk->damd = 4;
		}
		/* some pseudonatural's claws become more-damaging tentacles */
		if (magr->mfaction == PSEUDONATURAL && (
			attk->aatyp == AT_CLAW && (magr->m_id + *indexnum) % 4 == 0)
			){
			attk->aatyp = AT_TENT;
			if (attk->damn < 3)
				attk->damd += 2;
			else
				attk->damn++;
		}
		/* tomb herd's attacks are generally stronger */
		if (magr->mfaction == TOMB_HERD && (
			!is_null_attk(attk))
			){
			if (attk->damn < 3)
				attk->damd += 2;
			else
				attk->damn++;
		}
		/* tomb herd's attacks are generally stronger */
		if (magr->mfaction == TOMB_HERD && (
			ok_undead_subout)
			){
			*subout |= SUBOUT_UNDEAD;
			fromlist = FALSE;
			attk->aatyp = AT_TUCH;
			attk->adtyp = AD_ABDC;
			attk->damn = 1;
			attk->damd = 1;
		}
		/* yith gain spellcasting */
		if (magr->mfaction == YITH && (
			ok_undead_subout)
			){
			*subout |= SUBOUT_UNDEAD;
			fromlist = FALSE;
			attk->aatyp = AT_MAGC;
			attk->adtyp = AD_SPEL;
			attk->damn = 2;
			attk->damd = 6;
		}
		/* cranium rats gain psionic spellcasting */
		if (magr->mfaction == CRANIUM_RAT && (
			ok_undead_subout)
			){
			*subout |= SUBOUT_UNDEAD;
			fromlist = FALSE;
			attk->aatyp = AT_MAGC;
			attk->adtyp = AD_PSON;
			attk->damn = 0;
			attk->damd = 15;
		}
#undef ok_undead_subout
	}

	/* Undead damage multipliers -- note that these must be after actual replacements are done */
	/* zombies deal double damage, and all undead deal double damage at midnight (the midnight multiplier is not shown in the pokedex) */
	if (!youagr && magr->mfaction == ZOMBIFIED && (is_undead_mon(magr) && midnight() && !by_the_book))
		attk->damn *= 3;
	else if (!youagr && magr->mfaction == ZOMBIFIED || (is_undead_mon(magr) && midnight() && !by_the_book))
		attk->damn *= 2;

	/* Bandersnatches become frumious instead of fleeing, dealing double damage -- not shown in the pokedex */
	if (!youagr && magr->mflee && pa == &mons[PM_BANDERSNATCH] && !by_the_book)
		attk->damn *= 2;

	/* twoweapon symmetry -- if the previous attack missed, do not make an offhand attack -- obviously not shown in the pokedex */
	if (!by_the_book && *indexnum > 0 && prev_res[1] == MM_MISS && attk->aatyp == AT_XWEP)
	{
		/* just get the next attack */
		if (fromlist) {
			*indexnum += 1;
		}
		*prev_and_buf = prev_attack;
		return getattk(magr, prev_res, indexnum, prev_and_buf, by_the_book, subout, tohitmod);
	}

	/* prevent a monster with two consecutive disease or hunger attacks
	from hitting with both of them on the same turn; if the first has
	already hit, switch to a stun attack for the second */
	if (!by_the_book && *indexnum > 0 && prev_res[1] &&
		(attk->adtyp == AD_DISE ||
		// attk->adtyp == AD_PEST ||
		// attk->adtyp == AD_FAMN ||
		pa == &mons[PM_ASTRAL_DEVA]) &&
		attk->adtyp == prev_attack.adtyp
		) {
		attk->adtyp = AD_STUN;
	}

	/* possibly increment indexnum, if we want to move on in the monster's attack list 
	 * this is most of the time, except for when we have inserted attacks into [magr]'s chain */
	if (fromlist) {
		*indexnum += 1;
	}
	return attk;
}
#undef SUBOUT_UNDEAD
#undef SUBOUT_FIENDS
#undef SUBOUT_BAEL1
#undef SUBOUT_BAEL2
#undef SUBOUT_DEMO1
#undef SUBOUT_DEMO2
#undef SUBOUT_SPIRITS
#undef SUBOUT_BARB1
#undef SUBOUT_BARB2
#undef SUBOUT_MAINWEPB
#undef SUBOUT_XWEP

/* destroy_item2()
 *
 * Called when item(s) are supposed to be destroyed in a defender's inventory
 *
 * Works for player and monster mtmp
 * Assumes both the defender is alive and existant when called
 *
 * Can return:
 * MM_MISS		0x00	no items destroyed
 * MM_HIT		0x01	item(s) destroyed
 * MM_DEF_DIED	0x02	defender died
 *
 * If allow_lethal is false, damage will still be dealt but is never fatal,
 * so prematurely killing a monster isn't a problem.
 */
int
destroy_item2(mtmp, osym, dmgtyp, allow_lethal)
struct monst * mtmp;
int osym;
int dmgtyp;
boolean allow_lethal;
{
	boolean youdef = mtmp == &youmonst;
	struct permonst * data = (youdef) ? youracedata : mtmp->data;
	int vis = (youdef) ? TRUE : canseemon(mtmp);
	boolean any_destroyed = FALSE;
	struct obj *obj, *obj2;
	int dmg, xresist, skip;
	long i, cnt, quan;
	int dindx;
	const char *mult;

	if (osym == RING_CLASS && dmgtyp == AD_ELEC)
		return MM_MISS; /*Rings aren't destroyed by electrical damage anymore*/

	for (obj = (youdef ? invent : mtmp->minvent); obj; obj = obj2) {
		obj2 = obj->nobj;
		if (obj->oclass != osym) continue; /* test only objs of type osym */
		if (obj->oartifact) continue; /* don't destroy artifacts */
		if (obj->in_use && obj->quan == 1) continue; /* not available */
		xresist = skip = 0;
		dmg = dindx = 0;
		quan = 0L;

		switch (dmgtyp) {
			/* Cold freezes potions */
		case AD_COLD:
			if (osym == POTION_CLASS && obj->otyp != POT_OIL) {
				quan = obj->quan;
				dindx = 0;
				dmg = 4;
			}
			else skip++;
			break;
			/* Fire boils potions, burns scrolls, burns spellbooks */
		case AD_FIRE:
			xresist = (Fire_res(mtmp) && obj->oclass != POTION_CLASS);

			if (osym == SCROLL_CLASS && obj->oartifact)
				skip++;
			if (obj->otyp == SCR_FIRE || obj->otyp == SCR_GOLD_SCROLL_OF_LAW || obj->otyp == SPE_FIREBALL)
				skip++;
			if (obj->otyp == SPE_BOOK_OF_THE_DEAD) {
				skip++;
				if (!Blind && vis)
					pline("%s glows a strange %s, but remains intact.",
					The(xname(obj)), hcolor("dark red"));
			}
			quan = obj->quan;
			switch (osym) {
			case POTION_CLASS:
				dindx = 1;
				dmg = 6;
				break;
			case SCROLL_CLASS:
				dindx = 2;
				dmg = 1;
				break;
			case SPBOOK_CLASS:
				dindx = 3;
				dmg = 6;
				break;
			default:
				skip++;
				break;
			}
			break;
			/* electricity sparks charges out of wands */
		case AD_ELEC:
			xresist = (Shock_res(mtmp));
			quan = obj->quan;
			if (osym == WAND_CLASS){
				if (obj->otyp == WAN_LIGHTNING)
					skip++;
				dindx = 5;
				dmg = 6;
			}
			else
				skip++;
			break;
			/* other damage types don't destroy items here */
		default:
			skip++;
			break;
		}
		/* destroy the item, if allowed */
		if (!skip) {
			if (obj->in_use) --quan; /* one will be used up elsewhere */
			int amt = (osym == WAND_CLASS) ? obj->spe : quan;
			/* approx 10% of items in the stack get destroyed */
			for (i = cnt = 0L; i < amt; i++) {
				if (!rn2(10)) cnt++;
			}
			/* No items destroyed? Skip */
			if (!cnt)
				continue;
			/* print message */
			if (cnt == quan || quan == 1)	mult = "";
			else if (cnt > 1)				mult = "Some of ";
			else							mult = "One of ";
			pline("%s%s %s %s!",
				mult,
				(youdef) ? ((mult) ? "your" : "Your") : ((mult) ? s_suffix(mon_nam(mtmp)) : s_suffix(Monnam(mtmp))),
				xname(obj),
				(cnt > 1L) ? destroy_strings[dindx * 3 + 1]
				: destroy_strings[dindx * 3]);

			/* potion vapors */
			if (osym == POTION_CLASS && dmgtyp != AD_COLD) {
				if (!breathless(data) || haseyes(data)) {
					if (youdef)
						potionbreathe(obj);
					else
						/* no function for monster breathing potions */;
				}
			}
			/* destroy item */
			if (osym == WAND_CLASS)
				obj->spe -= cnt;
			else {
				if (obj == current_wand) current_wand = 0;	/* destroyed */
				for (i = 0; i < cnt; i++) {
					/* use correct useup function */
					if (youdef) useup(obj);
					else m_useup(mtmp, obj);
				}
			}
			any_destroyed = TRUE;

			/* possibly deal damage */
			if (dmg) {
				/* you */
				if (youdef) {
					if (xresist)	You("aren't hurt!");
					else {
						const char *how = destroy_strings[dindx * 3 + 2];
						boolean one = (cnt == 1L);

						dmg = d(cnt, dmg);
						if (!allow_lethal && dmg > *hp(&youmonst))
							dmg = min(0, *hp(&youmonst) - 1);
						losehp(dmg, (one && osym != WAND_CLASS) ? how : (const char *)makeplural(how),
							one ? KILLED_BY_AN : KILLED_BY);
						exercise(A_STR, FALSE);
						/* Let's not worry about properly returning if that killed you. If it did, it's moot. I think. */
					}
				}
				/* monster */
				else {
					if (xresist);	// no message, reduce spam
					else {
						dmg = d(cnt, dmg);
						if (!allow_lethal && dmg > mtmp->mhp)
							dmg = min(0, mtmp->mhp - 1);

						mtmp->mhp -= dmg;
						if (mtmp->mhp < 0) {
							if(vis) pline("%s dies!", Monnam(mtmp));
							mondied(mtmp);
							return (MM_HIT|MM_DEF_DIED);
						}
					}
				}
			}
		}
	}
	/* return if anything was destroyed */
	return (any_destroyed ? MM_HIT : MM_MISS);
}

/* noises()
/* prints noises from mvm combat
 */
void
noises(magr, attk)
struct monst * magr;
struct attack * attk;
{
	/* static variable prevent message spam */
	static boolean far_noise = FALSE;
	static long noisetime = 0;

	boolean farq = (distu(x(magr), y(magr)) > 15);

	if (flags.soundok && (farq != far_noise || moves - noisetime > 10)) {
		far_noise = farq;
		noisetime = moves;
		You_hear("%s%s.",
			(attk->aatyp == AT_EXPL) ? "an explosion" : "some noises",
			farq ? " in the distance" : "");
	}
}

/* xymissmsg()
 *
 * Prints an appropriate basic miss message (You miss foo, foo misses, foo misses bar)
 * Also does "noises" for when mvm combat isn't visible at all
 *
 * Also stops occupation() for the player
 */
void
xymissmsg(magr, mdef, attk, vis, nearmiss)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
int vis;
boolean nearmiss;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	boolean compat = (could_seduce(magr, mdef, attk) && (youagr || !magr->mcan && !magr->mspec_used));

	if (youagr) {
		if (compat)
			You("pretend to be friendly to %s.", mon_nam(mdef));
		else if (flags.verbose)
			You("miss %s.", mon_nam(mdef));
		else
			You("miss it.");
	}
	else if (youdef) {
		if (!(vis&VIS_MAGR))
			map_invisible(x(magr), y(magr));

		if (compat)
			pline("%s pretends to be friendly.", Monnam(magr));
		else if (flags.verbose && nearmiss)
			pline("%s just misses!", Monnam(magr));
		else
			pline("%s misses.", Monnam(magr));
		/* interrupt player */
		stop_occupation();
	}
	else {
		if (vis) {
			if (!(vis&VIS_MAGR))
				map_invisible(x(magr), y(magr));
			if (!(vis&VIS_MDEF))
				map_invisible(x(mdef), y(mdef));

			if (mdef->m_ap_type) seemimic(mdef);
			if (magr->m_ap_type) seemimic(magr);

			pline("%s %s %s.",
				Monnam(magr),
				(compat ? "pretends to be friendly to" : "misses"),
				mon_nam_too(mdef, magr)
				);
		}
		else {
			noises(magr, attk);
		}
	}
	return;
}

/* xyhitmsg() 
 * 
 * Prints an appropriate basic hit message (foo hits bar, foo hits, you hit foo)
 * 
 * Do not call this if the attack shouldn't be visible!
 */
void
xyhitmsg(magr, mdef, attk)
struct monst *magr;
struct monst *mdef;
struct attack *attk;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	int compat;

	/* AT_NONE is silent */
	if (attk->aatyp == AT_NONE)
		return;

	/* Note: if opposite gender, "seductively" */
	/* If same gender, "engagingly" for nymph, normal msg for others */
	if ((compat = could_seduce(magr, mdef, attk))
		&& !magr->mcan && !magr->mspec_used) {
		pline("%s %s %s %s.",
			(youagr ? "You" : Monnam(magr)),
			(youagr ?
				((is_silent_mon(magr)) ? "smile at" :
				(is_blind(mdef)) ? "talk to" : "move near")
				:
				((is_silent_mon(magr)) ? "smiles at" :
				(is_blind(mdef)) ? "talks to" : "moves near")
				),
			((youdef && !youagr) ? "you" : mon_nam_too(mdef, magr)),
			compat == 2 ? "engagingly" : "seductively");
	}
	else {
		boolean specify_you = FALSE;
		const char * verb = (const char *)0;
		const char * ending = "!";
		/* You X foo!	(youagr)
		 * Foo Xs!		(youdef, !youagr)
		 * Foo Xs bar!	()
		 * 
		 * <agressor> X (maybe plural) <defender if not you> <rest of string>
		 */	

		switch (attk->aatyp) {
		case AT_BEAM:
			if (!verb) verb = "blast";
			// fall through
		case AT_LNCK:
		case AT_BITE:
		case AT_5SBT:
			if (!verb) verb = "bite";
			// fall through
		case AT_KICK:
			if (!verb) verb = "kick";
			if (attk->aatyp == AT_KICK &&
				(thick_skinned(mdef->data) || (youdef && u.sealsActive&SEAL_ECHIDNA)))
				ending = ".";
			// fall through
		case AT_STNG:
			if (!verb) verb = "sting";
			// fall through
		case AT_BUTT:
			if (!verb) verb = "butt";
			// fall through
		case AT_SRPR:
				if (!verb){
					verb = "slash";
					if ((attk->adtyp == AD_SHDW) || (attk->adtyp == AD_STAR) || (attk->adtyp == AD_BLUD)) {
						ending = (attk->adtyp == AD_SHDW) ? " with bladed shadows!" :
							(attk->adtyp == AD_STAR) ? " with a starlight rapier!" :
							(attk->adtyp == AD_MERC) ? " with a blade of mercury!" :
							(attk->adtyp == AD_BLUD) ? " with a blade of blood!" : "!";
					}
					if (youdef)
						specify_you = TRUE;
				}
		case AT_5SQR:
		case AT_TUCH:
			if (!verb) {
				verb = "touch";
				if (youdef)
					specify_you = TRUE;
			}
			/* print the message */
			pline("%s %s%s%s%s%s%s",
				(youagr ? "You" : Monnam(magr)),
				(is_weeping(pa) && !youagr ? "is " : ""),
				(youagr && !is_weeping(pa) ? verb : makeplural(verb)),
				(is_weeping(pa) && !youagr ? "ing" : ""),
				((youdef && !youagr && !specify_you) ? "" : " "),
				((youdef && !youagr && !specify_you) ? "" : mon_nam_too(mdef, magr)),
				ending
				);
			break;
			/* some more special cases */
		case AT_TENT:
			pline("%s tentacles suck %s!",
				(youagr ? "Your" : s_suffix(Monnam(magr))),
				((youdef && !youagr) ? "you" : mon_nam_too(mdef, magr))
				);
			break;
		case AT_WHIP:
			pline("%s barbed whips lash %s!",
				(youagr ? "Your" : s_suffix(Monnam(magr))),
				((youdef && !youagr) ? "you" : mon_nam_too(mdef, magr))
				);
			break;
		case AT_EXPL:
		case AT_BOOM:
			pline("%s %s!",
				(youagr ? "You" : Monnam(magr)),
				(youagr ? "explode" : makeplural("explode"))
				);
			break;
		case AT_NONE:
			break;
		default:
			pline("%s %s%s%s!",
				(youagr ? "You" : Monnam(magr)),
				(youagr ? "hit" : makeplural("hit")),
				((youdef && !youagr) ? "" : " "),
				((youdef && !youagr) ? "" : mon_nam_too(mdef, magr))
				);
			break;
		}
	}
	return;
}

void
xswingsy(magr, mdef, otmp, ranged)
struct monst *magr, *mdef;
struct obj *otmp;
boolean ranged;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	char buf[BUFSZ];

	/* don't make a "You swing" message */
	if (youagr)
		return;
	/* don't message if there is no weapon */
	if (!otmp)
		return;
	/* don't message if you're blind or can't see magr (Nero asks: why the blind check?) */
	if (Blind || !canseemon(magr))
		return;
	/* don't message if we are not being verbose about combat, unless it's a thrust polearm */
	if (!flags.verbose && !ranged)
		return;

	if (!ranged) {
		Sprintf(buf, "%s %s %s %s",
			Monnam(magr),
			(objects[otmp->otyp].oc_dir & PIERCE && weapon_type(otmp) != P_PICK_AXE) ? "thrusts" : "swings",
			mhis(magr),
			singular(otmp, xname)
			);
	}
	else {
		Sprintf(buf, "%s %s %s", Monnam(magr), otmp->otyp == AKLYS ? "throws" : "thrusts",
			obj_is_pname(otmp) ? the(xname(otmp)) : an(xname(otmp)));
	}

	if (youdef) {
		Sprintf(buf, "%s!", buf);
	}
	else {
		Sprintf(buf, "%s at %s.", buf, mon_nam(mdef));
	}

	pline("%s", buf);
	return;
}

/* slips_free() 
 * 
 * Checks whether slippery clothing protects from hug or wrap attack.
 * 
 * Prints messages.
 */
boolean
slips_free(magr, mdef, attk, vis)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
int vis;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct obj *obj = (struct obj *)0;

	if (vis == -1)
		vis = getvis(magr, mdef, 0, 0);

	static int mboots1 = 0;
	if (!mboots1) mboots1 = find_mboots();

	switch (attk->adtyp) {
		/* INT attacks always target heads */
	case AD_DRIN:
		obj = (youdef ? uarmh : which_armor(mdef, W_ARMH));
		break;
		/* this is assumed to be an attack to the hands, draining DEX? */
	case AD_DRDX:
		obj = (youdef ? uarmg : which_armor(mdef, W_ARMG));
		break;
		/* attack to the legs */
	case AD_LEGS:
		obj = (youdef ? uarmf : which_armor(mdef, W_ARMF));
		break;
		/* wrap attacks are specifically blocked by mud boots, in addition to body armors */
	case AD_WRAP:
		obj = (youdef ? uarmf : which_armor(mdef, W_ARMF));
		if (obj && obj->otyp == mboots1)
			break;
		else
			/*fall through to default (body armors)*/;
	default:
		if (!obj) obj = (youdef ? uarmc : which_armor(mdef, W_ARMC));
		if (!obj) obj = (youdef ? uarm : which_armor(mdef, W_ARM));
		if (!obj) obj = (youdef ? uarmu : which_armor(mdef, W_ARMU));
	}

	if (obj && (
		(obj->greased || obj->otyp == OILSKIN_CLOAK) ||		/* greased (or oilskin) armor */
		(attk->adtyp == AD_WRAP && obj->otyp == mboots1)	/* mud boots vs wrap attacks */
		)
		&&
		!(obj->cursed && !rn2(3))							/* 1/3 chance to fail when cursed */
		) {
		/* print appropriate message */
		if (vis&VIS_MDEF) {
			pline("%s %s%s %s %s %s %s!",
				(youagr ? "You" : Monnam(magr)),
				(attk->adtyp == AD_WRAP ? "slip" : "grab"),
				(youagr ? "" : "s"),
				(attk->adtyp == AD_WRAP ? "off of" : ", but cannot hold onto"),
				(youdef ? "your" : s_suffix(mon_nam(mdef))),
				(obj->greased ? "greased" : "slippery"),
				((obj->otyp == OILSKIN_CLOAK && !objects[obj->otyp].oc_name_known)
					? cloak_simple_name(obj) : obj->otyp == mboots1 ? "mud boots" : xname(obj))
				);
		}
		/* remove grease (50% odds) */
		if (obj->greased && !rn2(2)) {
			if (vis&VIS_MDEF) {
				pline_The("grease wears off.");
			}
			obj->greased = 0;
		}
		return TRUE;
	}
	return FALSE;
}


/* heal()
 * 
 * increases [mon]'s hp by [amnt], up to [mon]'s maxhp
 * sets flags.botl when applicable
 */
void
heal(mon, amnt)
struct monst * mon;
int amnt;
{
	*hp(mon) += amnt;
	if (*hp(mon) > *hpmax(mon))
		*hp(mon) = *hpmax(mon);
	if (mon == &youmonst)
		flags.botl = 1;
	return;
}


/* xdamagey()
 * 
 * Deals [dmg] damage to [mdef].
 *
 * Rewards [magr] as appropriate.
 * 
 * If called with killerset==TRUE, will use the currently set killer and killer_format if the player dies
 * 
 * Returns:
 * MM_HIT		0x01	(possible) aggressor hit defender (usually, except when defender lifesaved)
 * MM_DEF_DIED	0x02	(possible) defender died (defender died and was not lifesaved)
 * MM_AGR_DIED	0x04	(never) aggressor died
 * MM_AGR_STOP	0x08	(never) aggressor should stop attacking for some reason
 * 
 */
int
xdamagey(magr, mdef, attk, dmg, killerset)
struct monst * magr;	/* attacker (might not exist) */
struct monst * mdef;	/* defender */
struct attack * attk;	/* attack used to deal damage (might not exist) */
int dmg;				/* damage to deal */
boolean killerset;		/* for custom killer() for players dying */
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);

	/* if defender is already dead, avoid re-killing them; just note that they are dead */
	if (*hp(mdef) < 1) {
		return MM_DEF_DIED;
	}

	/* debug */
	if (wizard && youagr && ublindf && ublindf->otyp == LENSES)
		pline("(dmg = %d)", dmg);
	/* deal damage */
	*hp(mdef) -= dmg;
	if (*hp(mdef) > *hpmax(mdef))
		*hp(mdef) = *hpmax(mdef);

	/* mhitu */
	if (youdef) {
		stop_occupation();
		flags.botl = 1;
		if (dmg > 0 && magr)
			magr->mhurtu = TRUE;
		/* messages */
		if ((dmg > 0) && (*hp(mdef) > 0) && (*hp(mdef) * 10 < *hpmax(mdef)) && !(Upolyd && !Unchanging))
			maybe_wail();
		if (*hp(mdef) < 1) {
			if (Upolyd && !Unchanging)
				rehumanize();
			else if (magr && !killerset)
				done_in_by(magr);
			else {
				You("die...");
				done(DIED);
			}
			if (*hp(mdef) > 0)
				return MM_DEF_LSVD;	/* you lifesaved or rehumanized */
			else
				return MM_DEF_DIED;
		}
	}
	/* uhitm */
	else if (youagr) {
		/* abuse pets */
		if (mdef->mtame && (!mdef->mflee || mdef->mfleetim) && dmg > 0) {
			abuse_dog(mdef);
			monflee(mdef, 10 * rnd(dmg), FALSE, FALSE);
		}

		if (*hp(mdef) < 1) {
			/* killed a pet by accident */
			if (mdef->mtame && !cansee(mdef->mx, mdef->my)) {
				You_feel("embarrassed for a moment.");
				if (dmg) xkilled(mdef, 0); /* !dmg but hp<1: already killed */
			}
			/* killed monster */
			else {
				/* non-verbose */
				if (!flags.verbose) {
					You("destroy it!");
					if (dmg) xkilled(mdef, 0);
				}
				/* verbose */
				else {
					if (dmg) killed(mdef);
				}
			}
			if (*hp(mdef) > 0)
				return MM_DEF_LSVD; /* mdef lifesaved */
			else
				return MM_DEF_DIED;
		}
	}
	/* mhitm */
	else {
		if (*hp(mdef)< 1) {
			monkilled(mdef, "", attk ? (int)attk->adtyp : 0);
			if (*hp(mdef) > 0)
				return MM_DEF_LSVD; /* mdef lifesaved */
			else
				return (MM_HIT | MM_DEF_DIED | (!magr || grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
		}
	}
	return MM_HIT;
}

/* xstoney()
 * 
 * Attempts to turn [mdef] to stone.
 * 
 * Returns:
 * MM_MISS		0x00	(possible) aggressor missed (caused by lifesaved mondef)
 * MM_HIT		0x01	(possible) aggressor hit defender (defender poly'd when stoned OR youdef)
 * MM_DEF_DIED	0x02	(possible) defender died (mondef turned to stone OR died while unstoning)
 * MM_AGR_DIED	0x04	(possible) aggressor died (monagr got XP for the kill and leveled up into something genocided)
 * MM_AGR_STOP	0x08	(never) aggressor should stop attacking for some reason
 * 
 */
int
xstoney(magr, mdef)
struct monst * magr;
struct monst * mdef;
{

	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	struct permonst * pd = youdef ? youracedata : mdef->data;

	if (!Stone_res(mdef)
		&& !(youdef && Stoned))
	{
		/* vs player */
		if (youdef) {
			if (!(poly_when_stoned(youracedata) && polymon(PM_STONE_GOLEM))) {
				Stoned = 5;
				delayed_killer = pa->mname;
				if (pa->geno & G_UNIQ) {
					if (!type_is_pname(pa)) {
						static char kbuf[BUFSZ];
						/* "the" buffer may be reallocated */
						Strcpy(kbuf, the(delayed_killer));
						delayed_killer = kbuf;
					}
					killer_format = KILLED_BY;
				}
				else
					killer_format = KILLED_BY_AN;
			}
		}
		/* vs monsters */
		else {
			/* try to unstone itself */
			if (!munstone(mdef, youagr)) {
				minstapetrify(mdef, youagr);
			}
			/* some monsters poly when stoned */
			if (Stone_res(mdef)) {
				/* don't return MM_DEF_LSVD -- lifesaving didn't happen */
				return MM_HIT;
			}
			/* survived */
			if (*hp(mdef) > 0)
				return MM_DEF_LSVD;
			/* died */
			else {
				if (mdef->mtame && !youagr)
					You("have a peculiarly sad feeling for a moment, then it passes.");
				return (MM_HIT | (MM_DEF_DIED | (youagr || grow_up(magr, mdef) ? 0 : MM_AGR_DIED)));
			}
		}
	}
	return MM_HIT;
}

/* do_weapon_multistriking_effects()
 * 
 * Performs specific artifact-level special effects caused by multistriking weapons
 * 
 * This is performed ONCE per attack action, and only if [weapon]->ostriking != 0
 * This is in contrast to artifacthit, which is applied additionally for each [weapon]->ostriking
 *
 * Assumes defender is alive
 */
int
do_weapon_multistriking_effects(magr, mdef, attk, weapon, vis)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
struct obj * weapon;
int vis;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	struct permonst * pd = youdef ? youracedata : mdef->data;

	int result;

	if (arti_threeHead(weapon)) {
		/* only has special effects when all 3 heads hit */
		if (weapon->ostriking + 1 == 3) {
			/* message */
			if (youdef) {
				You("stagger!");
			}
			else if (canseemon(mdef)) {
				pline("%s %s!", 
					Monnam(mdef),
					makeplural(stagger(mdef, "stagger"))
					);
			}
			/* apply stun */
			if (youdef) {
				make_stunned((HStun)+d(3, 3), FALSE);
			}
			else {
				mdef->mconf = 1;
				mdef->mstun = 1;
			}
		}
	}

	if (arti_tentRod(weapon)) {
		if (weapon->ostriking + 1 >= 3)
		{
			switch (rn2(3))
			{
			case 0:/* Blind */
				if (youdef) {
					if(!Blind)
						You("go blind!");
					make_blinded(Blinded + d(3, 3), FALSE);
				}
				else {
					if (canseemon(mdef) && !is_blind(mdef))
						pline("%s goes blind!", Monnam(mdef));
					mdef->mcansee = 0;
					mdef->mblinded = d(3, 3);
				}
				break;
			case 1:/* Stun */
				if (youdef) {
					You("stagger!");
					make_stunned((HStun)+d(3, 3), FALSE);
				}
				else {
					if (canseemon(mdef))
						pline("%s is stunned!", Monnam(mdef));
					mdef->mstun = 1;
				}
				break;
			case 2:/* Conf */
				if (youdef) {
					You("are confused!");
					make_confused(HConfusion + d(3, 3), FALSE);
				}
				else {
					if (canseemon(mdef))
						pline("%s %s!", Monnam(mdef), makeplural(stagger(mdef, "stagger")));
					mdef->mconf = 1;
				}
				break;
			}
		}
		/* not elseif, this is additive */
		if (weapon->ostriking + 1 >= 6)
		{
			switch (rn2(3))
			{
			case 0:/* Slow */
				if (youdef) {
					u_slow_down();
				}
				else {
					if (canseemon(mdef))
						pline("%s slows down!", Monnam(mdef));
					mdef->mspeed = MSLOW;
					mdef->permspeed = MSLOW;
				}
				break;
			case 1:/* Paralyze */
				if (youdef) {
					You("can't move!");
					nomul(-1 * d(3, 3), "paralyzed by the Tentacle Rod.");
				}
				else {
					if (canseemon(mdef))
						pline("%s is frozen!", Monnam(mdef));
					mdef->mcanmove = 0;
					mdef->mfrozen = d(1, 6);
				}
				break;
			case 2:/* Craze */
				if (youdef) {
					You("go insane!");
					make_confused(10000, FALSE); //very large value representing insanity
				}
				else {
					if (canseemon(mdef))
						pline("%s goes insane!", Monnam(mdef));
					mdef->mcrazed = 1;
				}
				break;
			}
		}
		/* not elseif */
		if (weapon->ostriking + 1 == 7)
		{
			/* message */
			if (vis&VIS_MAGR) {
				pline_The("%s hits %s with a storm of energy!",
					xname(weapon),
					mon_nam(mdef));
			}
			/* 1d7 bonus damage per element, checking that mdef hasn't died partway through */
			if (!(result&(MM_DEF_DIED|MM_AGR_DIED|MM_DEF_LSVD)) && !Fire_res(mdef))		result |= xdamagey(magr, mdef, attk, d(1, 7), FALSE);
			if (!(result&(MM_DEF_DIED|MM_AGR_DIED|MM_DEF_LSVD)) && !Cold_res(mdef))		result |= xdamagey(magr, mdef, attk, d(1, 7), FALSE);
			if (!(result&(MM_DEF_DIED|MM_AGR_DIED|MM_DEF_LSVD)) && !Shock_res(mdef))	result |= xdamagey(magr, mdef, attk, d(1, 7), FALSE);
			if (!(result&(MM_DEF_DIED|MM_AGR_DIED|MM_DEF_LSVD)) && !Acid_res(mdef))		result |= xdamagey(magr, mdef, attk, d(1, 7), FALSE);
			if (!(result&(MM_DEF_DIED|MM_AGR_DIED|MM_DEF_LSVD)) && !Magic_res(mdef))	result |= xdamagey(magr, mdef, attk, d(1, 7), FALSE);
			if (!(result&(MM_DEF_DIED|MM_AGR_DIED|MM_DEF_LSVD)) && !Poison_res(mdef))	result |= xdamagey(magr, mdef, attk, d(1, 7), FALSE);
			if (!(result&(MM_DEF_DIED|MM_AGR_DIED|MM_DEF_LSVD)) && !Drain_res(mdef))	result |= xdamagey(magr, mdef, attk, d(1, 7), FALSE);
			if (result&(MM_DEF_DIED|MM_AGR_DIED|MM_DEF_LSVD)) {
				return result;
			}
		}
	}
	/* default return: no one died */
	return MM_HIT;
}

/*
 * tohitval()
 *
 * Very general to-hit-bonus finding function.
 * 
 * [magr] might not exist!
 *
 */
int
tohitval(magr, mdef, attk, weapon, launcher, thrown, flat_acc)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
struct obj * weapon;
struct obj * launcher;
int thrown;					/* 0: not thrown  1: thrown properly 2: thrown improperly (like arrows thrown by hand) */
int flat_acc;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = (magr ? (youagr ? youracedata : magr->data) : (struct permonst *)0);
	struct permonst * pd = youdef ? youracedata : mdef->data;
	struct obj * otmp;
	boolean fired = (weapon && thrown == 1);

	/* partial accuracy counters */
	int base_acc = 0;	/* accuracy from leveling up */
	int bons_acc = 0;	/* attacker's misc accuracy bonuses */
	int rang_acc = 0;	/* ranged-attack specific accuracy bonuses */
	int stdy_acc = 0;	/* defender's study amount */
	int vdef_acc = 0;	/* accuracy bonuses versus that specific defender */
	int wepn_acc = 0;	/* accuracy of the weapon -- dependent on attk */
	int defn_acc = 0;	/* defender's defense -- dependent on attk */
	int totl_acc = 0;	/* sum of above partial accuracy counters */

	/* base accuracy due to level -- requires attacker */
	if (magr) {
		/* Some player roles have better ranged accuracy with certain weapons than their BAB */
		if (youagr && thrown && weapon &&
			(Role_if(PM_RANGER) ||
			(u.sealsActive&SEAL_EVE) ||
			(weapon->otyp == DAGGER && Role_if(PM_ROGUE)) ||
			(weapon->otyp == DART && Role_if(PM_TOURIST)) ||
			(weapon->otyp == HEAVY_IRON_BALL && Role_if(PM_CONVICT))
			)) {
			base_acc = mlev(magr);
		}
		else {
			base_acc = mlev(magr) * (youagr ? BASE_ATTACK_BONUS : thrown ? 0.25 : 0.67);
		}
	}
	/* other attacker-related accuracy bonuses */
	if (magr) {
		/* player-only accuracy bonuses */
		if (youagr) {
			/* base +1/-1 for no reason */
			bons_acc += (thrown ? -1 : +1);
			/* Stat (DEX, STR) */
			if (!thrown || !launcher || objects[launcher->otyp].oc_skill == P_SLING){
				/* firing ranged attacks without a laucher (ex manticore tail spikes) can use STR */
				bons_acc += abon();
			}
			else {
				if (ACURR(A_DEX) < 4)			bons_acc -= 3;
				else if (ACURR(A_DEX) < 6)		bons_acc -= 2;
				else if (ACURR(A_DEX) < 8)		bons_acc -= 1;
				else if (ACURR(A_DEX) >= 14)	bons_acc += (ACURR(A_DEX) - 14);
			}
			/* Stat (INT) (from Dantalion vs telepathically sensed enemies) */
			if (u.sealsActive&SEAL_DANTALION && tp_sensemon(mdef))
				bons_acc += max(0, (ACURR(A_INT) - 10) / 2);
			/* intrinsic accuracy bonuses */
			bons_acc += u.uhitinc;
			/* Malphas' bonus accuracy from having nearby crows -- btw Chris this is horribly named */
			bons_acc += u.spiritAttk;
			/* Uur (active) */
			if (u.uuur_duration)
				bons_acc += 10;
			/* luck */
			if (Luck)
				bons_acc += sgn(Luck)*rnd(abs(Luck));
			/* Bard */
			bons_acc += u.uencouraged;
			/* Singing Sword */
			if (uwep && uwep->oartifact == ART_SINGING_SWORD){
				if (uwep->osinging == OSING_LIFE){
					bons_acc += uwep->spe + 1;
				}
			}
			/* carrying too much */
			if (near_capacity())
				bons_acc -= ((near_capacity() * 2) - 1);
			/* trapped */
			if (u.utrap)
				bons_acc -= 3;
			/* swallowed */
			if (u.uswallow && u.ustuck == mdef)
				bons_acc += 1000;
		}
		/* monster-only accuracy bonuses */
		else {
			/* high-rank foes are accurate */
			if (is_lord(pa))
				bons_acc += 2;
			if (is_prince(pa))
				bons_acc += 5;
			/* these guys are extra accurate */
			if (is_uvuudaum(pa) || pa == &mons[PM_CLAIRVOYANT_CHANGED])
				bons_acc += 20;
			if (pa == &mons[PM_DANCING_BLADE])
				bons_acc += 7;
			if (pa == &mons[PM_CHOKHMAH_SEPHIRAH])
				bons_acc += u.chokhmah;
			/* Your steed gets a skill-based boost */
			if (magr == u.usteed)
				bons_acc += mountedCombat();
			/* All of your pets get a skill-based boost */
			if (magr->mtame)
				bons_acc += beastmastery();
			/* Bard */
			bons_acc += magr->encouraged;
			/* Singing Sword */
			if (uwep && uwep->oartifact == ART_SINGING_SWORD && !mindless_mon(magr) && !is_deaf(magr)){
				if (uwep->osinging == OSING_DIRGE && !magr->mtame){
					bons_acc -= uwep->spe + 1;
				}
			}
			/* trapped */
			if (magr->mtrapped)
				bons_acc -= 2;
		}
	}

	/* ranged-specific accuracy modifiers */
	if (thrown)
	{
		if (magr) {
			/* accuracy is reduced by range (or increased, for sniper rifles) */
			int dist_penalty = max(-4, distmin(x(magr), y(magr), x(mdef), y(mdef)) - 3);
			if (launcher && launcher->otyp == SNIPER_RIFLE)
				rang_acc += dist_penalty;
			else
				rang_acc -= dist_penalty;
		}

		/* gloves are a hinderance to proper use of bows */
		if (magr && launcher && objects[launcher->otyp].oc_skill == P_BOW) {
			struct obj * gloves;
			gloves = (youagr ? uarmg : which_armor(magr, W_ARMG));
			if (gloves){
				switch (uarmg->otyp) {
				case ORIHALCYON_GAUNTLETS:    /* metal */
				case GAUNTLETS_OF_POWER:    /* metal */
				case GAUNTLETS:
				case CRYSTAL_GAUNTLETS:
					rang_acc -= 2;
					break;
				case GAUNTLETS_OF_FUMBLING:
					rang_acc -= 3;
					break;
				case ARCHAIC_GAUNTLETS:
				case PLASTEEL_GAUNTLETS:
					rang_acc -= 1;
					break;
				case LONG_GLOVES:
				case GLOVES:
				case GAUNTLETS_OF_DEXTERITY:
				case HIGH_ELVEN_GAUNTLETS:
					break;
				default:
					impossible("Unknown type of gloves (%d)", uarmg->otyp);
					break;
				}
			}
		}
		/* some objects are more likely to hit than others */
		switch (weapon->otyp) {
		case HEAVY_IRON_BALL:
			if (weapon != uball)
				rang_acc += 2;
			break;
		case BOULDER:
		case STATUE:
			if (is_boulder(weapon))
				rang_acc += 6;
			break;
		case BALL_OF_WEBBING:
			/* balls of webbing should always miss */
			rang_acc -= 2000;
			break;
		}
	}

	/* study */
	if (youdef)
		stdy_acc += u.ustdy;
	if (!youdef)
		stdy_acc += mdef->mstdy;
	if (youagr)
		stdy_acc += mdef->ustdym;

	/* Adjust vs defender state; do not modify */
	if (youdef ? Stunned : mdef->mstun)
		vdef_acc += 2;
	if (youdef ? u.usleep : mdef->msleeping)
		vdef_acc += 2;
	if (youdef ||
		!(pd == &mons[PM_GIANT_TURTLE] && mdef->mflee && !mdef->mcanmove)) {	// don't penalize enshelled turtles
		if (youdef ? FALSE : mdef->mflee)
			vdef_acc += 2;
		if (youdef ? multi < 0 : !mdef->mcanmove)
			vdef_acc += 4;
	}
	/* Elves hate orcs, and the devs like elves. */
	if (magr && is_orc(pd) && is_elf(pa))
		vdef_acc += 1;
	/* ranged attacks are affected by target size */
	if (thrown)
	{
		vdef_acc += (pd->msize - MZ_MEDIUM);
	}
	/* Smaug is vulnerable to stabbings */
	if (pd == &mons[PM_SMAUG] && fired && weapon && launcher &&
		is_stabbing(weapon) && is_ammo(weapon))
	{
		vdef_acc += 20;
	}

	/* weapon accuracy -- only applies for a weapon attack OR a properly-thrown object */
	if ((attk && (attk->aatyp == AT_WEAP || attk->aatyp == AT_XWEP || attk->aatyp == AT_MARI || attk->aatyp == AT_DEVA || attk->aatyp == AT_HODS))
		|| fired)
	{
		if (weapon) {
			/* specific bonuses of weapon vs mdef */
			wepn_acc += hitval(weapon, mdef);
			/* Houchou always hits when thrown */
			if (weapon->oartifact == ART_HOUCHOU && thrown)
				wepn_acc += 1000;
			/* -4 accuracy per weapon size too large (not for thrown objects) */
			if (!thrown && weapon->objsize - pa->msize > 0){
				wepn_acc += -4 * (weapon->objsize - pa->msize);
			}
			/* ranged attacks also get their launcher's accuracy */
			if (fired && launcher) {
				/* enchantment, erosion */
				wepn_acc += launcher->spe;
				wepn_acc -= greatest_erosion(launcher);
				/* artifact bonus */
				if (launcher->oartifact){
					wepn_acc += spec_abon(launcher, mdef);
					if (youagr && Role_if(PM_BARD)) //legend lore
						wepn_acc += 5;
				}
				/*
				 * Elves and Samurais are highly trained w/bows,
				 * especially their own special types of bow.
				 * Polymorphing won't make you a bow expert.
				 */
				if (youagr && (Race_if(PM_ELF) || Role_if(PM_SAMURAI)) &&
					(!Upolyd || your_race(youmonst.data)) &&
					objects[launcher->otyp].oc_skill == P_BOW) {
					wepn_acc++;
					if (Race_if(PM_ELF) && launcher->otyp == ELVEN_BOW)
						wepn_acc++;
					else if (Role_if(PM_SAMURAI) && launcher->otyp == YUMI)
						wepn_acc++;
				}
			}
			/* mis-used ammo */
			else if (thrown == 2) {
				wepn_acc -= 4;
			}
			/* other thrown things */
			else if (thrown == 1)
			{
				if (is_boomerang(weapon))			/* arbitrary */
					wepn_acc += 4;
				else if (throwing_weapon(weapon))	/* meant to be thrown */
					wepn_acc += 2;
				else								/* not meant to be thrown at all */
					wepn_acc -= 2;
			}

			/* Shii Cho lightsaber form is not meant for fighting other lightsaber users */
			if (youagr && is_lightsaber(weapon) && litsaber(weapon)){
				if (u.fightingForm == FFORM_SHII_CHO && MON_WEP(mdef) && is_lightsaber(MON_WEP(mdef)) && litsaber(MON_WEP(mdef))){
					wepn_acc -= 5;
				}
			}
		}
		/* skill bonus (player-only; applies without a weapon as well) */
		if (youagr) {
			if (fired && launcher)
				wepn_acc += weapon_hit_bonus(launcher);
			else if (!thrown || thrown == 1)
				wepn_acc += weapon_hit_bonus(weapon);
		}
		/* monk accuracy bonus/penalty (player-only) (melee) */
		if (youagr && !thrown && Role_if(PM_MONK) && !Upolyd) {
			static boolean armmessage = TRUE;
			if (uarm) {
				if (armmessage) Your("armor is rather cumbersome...");
				armmessage = FALSE;
				wepn_acc -= 20; /*flat -20 for monks in armor*/
			}
			else {
				if (!armmessage) armmessage = TRUE;
				if (!uwep && !uarms) {
					wepn_acc += (u.ulevel / 3) + 2;
				}
			}
		}
		/* nudist accuracy bonus/penalty (player-only) (melee) */
		if (youagr && u.umadness&MAD_NUDIST && u.usanity < 100){
			int delta = 100 - u.usanity;
			int discomfort = u_clothing_discomfort();
			static boolean clothmessage = TRUE;
			if (discomfort) {
				if (clothmessage) Your("clothing is rather uncomfortable...");
				clothmessage = FALSE;
				wepn_acc -= (discomfort * delta) / 10;
			}
			else {
				if (!clothmessage) clothmessage = TRUE;
				if (!uwep && !uarms) {
					wepn_acc += delta / 10;
				}
			}
		}
	}

	/* combat boots increase accuracy */
	if (magr) {
		static int cbootsa = 0;
		if (!cbootsa) cbootsa = find_cboots();
		otmp = (youagr ? uarmf : which_armor(magr, W_ARMF));
		if (otmp && otmp->otyp == cbootsa)
			wepn_acc++;
	}


	/* find defender's AC */
	/* ignore worn armor? */
	if ((youagr && u.sealsActive&SEAL_CHUPOCLOPS && !thrown) ||
		(weapon && arti_shining(weapon)) ||
		(!thrown && attk->aatyp == AT_TUCH) ||
		(!thrown && attk->aatyp == AT_SRPR)) {
		if (youdef) {
			defn_acc += AC_VALUE(base_uac() + u.uspellprot) + 10 - u.uspellprot;
		}
		else {
			defn_acc += base_mac(mdef);
		}
	}
	/* do not ignore worn armor */
	else {
		if (youdef){
			defn_acc += AC_VALUE(u.uac + u.uspellprot) + 10 - u.uspellprot;
		}
		else {
			defn_acc += find_mac(mdef);
		}
	}

	/* determine if the attack hits */
	totl_acc = base_acc
		+ rang_acc
		+ bons_acc
		+ stdy_acc
		+ vdef_acc
		+ wepn_acc
		+ defn_acc
		+ flat_acc;
	/*
	if (wizard && !youdef && ublindf && ublindf->otyp == LENSES) {
		pline("Accuracy = %d+%d+%d+%d+%d+%d+%d+%d=%d",
			base_acc,
			rang_acc,
			bons_acc,
			stdy_acc,
			vdef_acc,
			wepn_acc,
			defn_acc,
			flat_acc,
			totl_acc
			);
	}
	*/

	/* return our to-hit -- if this is greater than a d20, it hits */
	return totl_acc;
}

/* xmeleehity()
 * 
 * Called when a creature attempts to attack another creature with a specific melee attack.
 * 
 * Assumes both the attacker and defender are alive and existant when called
 * 
 * Inside the function itself, determines if the attacks hits
 * Performs counterattacks as well
 * 
 * Hit / miss messages are handled here and below
 *
 * Uses MM_ return values
 */
int
xmeleehity(magr, mdef, attk, weapon, vis, flat_acc, ranged)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
struct obj * weapon;
int vis;
int flat_acc;
boolean ranged;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct attack alt_attk = *attk;
	struct permonst * pa = youagr ? youracedata : magr->data;
	struct permonst * pd = youdef ? youracedata : mdef->data;
	struct obj * otmp;
	char buf[BUFSZ];
	int result;			/* did attack hit, miss, defender live, die, agressor die, stop? */

	boolean unarmed_punch = ((!weapon || martial_aid(weapon)) &&
		(attk->aatyp == AT_WEAP || attk->aatyp == AT_XWEP || attk->aatyp == AT_DEVA || attk->aatyp == AT_MARI || attk->aatyp == AT_HODS));

	int dieroll;				/* rolled accuracy */
	int accuracy;				/* accuracy of attack; if this is less than dieroll, the attack hits */
	boolean hit = FALSE;		/* whether or not the attack hit */
	boolean miss = FALSE;		/* counterpart to hit */
	boolean domissmsg = TRUE;	/* FALSE if a message has already been printed about a miss */

	/* if it is the player's pet attacking and it is in LoS, set flag to train Beast Mastery skill */
	if (!youagr && magr->mtame && canseemon(magr)) {
		u.petattacked = TRUE;
	}

	if (vis == -1)
		vis = getvis(magr, mdef, 0, 0);
	
	/* Some things cause immediate misses */
	/* monster displacement */
	if (!youdef &&
		mon_resistance(mdef, DISPLACED) &&
		!(youagr && u.ustuck && u.ustuck == mdef) &&
		!(youagr && u.uswallow) &&
		rn2(2)) {
		if (has_passthrough_displacement(pd)){
			if (vis&VIS_MAGR) {
				pline("%s attack passes harmlessly through %s!",
					(youagr ? "Your" : s_suffix(Monnam(magr))),
					the(mon_nam(mdef)));
			}
		}
		else {
			if (vis&VIS_MAGR) {
				pline("%s attack%s %s displaced image!",
					(youagr ? "You" : Monnam(magr)),
					(youagr ? "" : "s"),
					(youagr ? "a" : s_suffix(mon_nam(mdef)))
					);
			}
		}
		domissmsg = FALSE;
		miss = TRUE;
	}
	/* insubstantial (shade-type) immunity to being hit */
	if (!miss && !hits_insubstantial(magr, mdef, attk, weapon)) {
		/* Print message */
		if (vis&VIS_MAGR) {
			Sprintf(buf, "%s", ((!weapon || valid_weapon(weapon)) ? "attack" : cxname(weapon)));
			pline("%s %s %s harmlessly through %s.",
				(youagr ? "Your" : s_suffix(Monnam(magr))),
				buf,
				vtense(buf, "pass"),
				(youdef ? "you" : mon_nam(mdef))
				);
		}
		domissmsg = FALSE;
		miss = TRUE;
	}

	/* get accuracy of attack */
	if (miss)
		accuracy = 0;
	else
		accuracy = tohitval(magr, mdef, attk, weapon, (struct obj *)0, 0, flat_acc);

	/* roll to-hit die */
	dieroll = rnd(20);

	/* Diverge on aatyp */
	switch (attk->aatyp)
	{
//////////////////////////////////////////////////////////////
// WEAPON
//////////////////////////////////////////////////////////////
		/* various weapon attacks */
	case AT_WEAP:
	case AT_DEVA:
	case AT_XWEP:
	case AT_MARI:
	case AT_HODS:
		
		/* if the player is attacking while swallowed, guaranteed hit */
		if (youagr && u.uswallow && !miss) {
			hit = TRUE;
		}
		
		if ((accuracy > dieroll) && !miss) {
			hit = TRUE;
		}
		/* multistriking weapons need to determine how many hit, and set ostriking */
		if (weapon && multistriking(weapon) && !miss) {
			weapon->ostriking = 0;
			int attempts = rn2(multistriking(weapon));
			for (; attempts; attempts--) {
				if (accuracy > rnd(20))
					weapon->ostriking++;
			}
			/* if [dieroll] was not high enough to hit, reduce ostriking by 1 and make [hit] true */
			if (!hit && weapon->ostriking) {
				weapon->ostriking--;
				hit = TRUE;
			}
		}
		break;
//////////////////////////////////////////////////////////////
// MELEE
//////////////////////////////////////////////////////////////
		/* basic hand-to-hand attacks */
	case AT_CLAW:
	case AT_KICK:
	case AT_BITE:
	case AT_STNG:
	case AT_BUTT:
	case AT_TENT:
	case AT_WHIP:
	case AT_WISP:
	case AT_HITS:	// always hits
	case AT_TUCH:	// uses touch accuracy
	case AT_SRPR:	// uses touch accuracy
	/* ranged attack types that are also melee */
	case AT_LNCK:
	case AT_5SBT:
	case AT_LRCH:
	case AT_5SQR:
		/* determine if the attack hits */
		if (attk->aatyp == AT_HITS && !miss) {
			hit = TRUE;
		}
		if ((accuracy > dieroll) && !miss) {
			hit = TRUE;
		}
		break;
		/* hits if previous 2 attacks hit; always contacts */
	case AT_REND:
	case AT_HUGS:	// also a guaranteed hit if the player is involved and the player and other creature are stuck together
		/* xattacky() would have not called xmeleehity() if the conditions to make either aatyp weren't satisfied
		 * We do not need to check previous attacks
		 */
		if (!miss) {
			hit = TRUE;
		}
		break;
	default:
		/* complain */
		impossible("bad aatyp for xmeleehity (%d)", attk->aatyp);
		return 0;
	}//switch(aatyp)

	/* if we haven't confirmed a hit yet, we missed */
	if (!hit)
		miss = TRUE;

	/* AT_DEVA attacks shouldn't print a miss message if it is a subsequent attack that misses */
	/* Hack this in by knowing that repeated AT_DEVA attacks have a flat_acc penalty */
	if (attk->aatyp == AT_DEVA && flat_acc < 0)
		domissmsg = FALSE;
	/* print a "miss" message */
	if (miss && domissmsg) {
		xymissmsg(magr, mdef, attk, vis, (accuracy == dieroll));
	}

	/* misc after-attack things to deal with */

	/* show I for invisible attackers to the player */
	if (youdef && !canspotmon(magr))
		map_invisible(x(magr), y(magr));

	/* Nearby monsters may be awakened */
	if (!(is_silent_mon(magr) && (cantmove(mdef) || is_silent_mon(mdef)))	/* suppressed entirely if attacker and defender are silent */
		&& rn2(10)															/* suppressed entirely 1/10 times */
		&& !((youagr || youdef) && Stealth && !rn2(3)))						/* suppressed entirely 1/3 times if the player is attacking/defending and stealthy */
		wake_nearto(x(mdef), y(mdef),										/* call function to wake nearby monsters */
		combatNoise(pa) / (((youagr || youdef) && Stealth) ? 2 : 1));		/* range is reduced if the player is involved and stealthy */

	/* If the monster is undetected and hits you, you should know where the attack came from. */
	if (youdef && magr->mundetected && hit && (hides_under(pa) || pa->mlet == S_EEL)) {
		magr->mundetected = 0;
		if (!(Blind ? Blind_telepat : Unblind_telepat)) {
			struct obj *obj;
			const char *what;

			if ((obj = level.objects[x(magr)][y(magr)]) != 0) {
				if (Blind && !obj->dknown)
					what = something;
				else if (is_pool(x(magr), y(magr), TRUE) && !Underwater)
					what = "the water";
				else
					what = doname(obj);

				pline("%s was hidden under %s!", Amonnam(magr), what);
			}
			newsym(x(magr), y(magr));
		}
	}
	/* blast the player sometimes if they're wearing blasty artifact body armor */
	if (youdef && uarm && uarm->oartifact && !rn2(10))
		touch_artifact(uarm, &youmonst, FALSE);

	/* if we hit... */
	if (hit) {
		/* DEAL THE DAMAGE */
		result = xmeleehurty(magr, mdef, attk, weapon, TRUE, -1, dieroll, vis, ranged);
	}
	else {
		result = MM_MISS;
	}

	/* if the attack hits, or if the creature is able to notice it was attacked (but the attack missed) it wakes up */
	if (youdef || (hit || (!mdef->msleeping && mdef->mcanmove)))
		wakeup2(mdef, youagr);

	return result;
}

/* xmeleehurty()
 *
 * Called when a creature is to deal damage to another creature with a melee attack.
 * It is assumed that the attack will hit, only damage and effects are in play.
 *
 * Assumes both the attacker and defender are alive and existant when called
 *
 * Monster to monster attacks.  When a monster attacks another (mattackm),
 * any or all of the following can be returned.  See mattackm() for more
 * details.
 * MM_MISS		0x00	aggressor missed
 * MM_HIT		0x01	aggressor hit defender
 * MM_DEF_DIED	0x02	defender died
 * MM_AGR_DIED	0x04	aggressor died
 *
 * Inside the function itself, determines if the attacks hits and
 * what damage was dealt and who survived.
 */
int
xmeleehurty(magr, mdef, attk, weapon, dohitmsg, flatdmg, dieroll, vis, ranged)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
struct obj * weapon;
boolean dohitmsg;
int flatdmg;
int dieroll;
int vis;
boolean ranged;
{
	int dmg = 0,					// damage that will be dealt
		ptmp = 0,					// poison type
		result = 0,					// result from self-referential calls of xmeleehurty()
		armpro = 0;					// MC of defender's armor
	boolean uncancelled = TRUE,		// if the attack's specials are not cancelled (probably via armor)
			notmcan = TRUE,			// if the attacker is not cancelled (so it's the player attacking or an uncancelled monster)
			armuncancel = FALSE;	// if armor is responsible for cancelling attack specials
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	boolean wepgone = FALSE;
	boolean spec = FALSE;			// general-purpose special flag
	struct attack alt_attk = *attk;	// buffer space to modify attacks in 
	struct permonst * pa = youagr ? youracedata : magr->data;
	struct permonst * pd = youdef ? youracedata : mdef->data;
	struct obj * otmp;
	struct monst * mtmp;
	char buf[BUFSZ];

	/* First determine the base damage done */
	if (flatdmg >= 0) {
		dmg = flatdmg;
	}
	else if (attk->damn && attk->damd) {
		dmg = d((int)attk->damn, (int)attk->damd);
	}
	else {
		dmg = 0;
	}
	/* worms get increased damage on their bite if they are lined up with momentum */
	if(!youagr && pa == &mons[PM_LONG_WORM] && magr->wormno && attk->aatyp == AT_BITE){
		if(wormline(magr, bhitpos.x, bhitpos.y))
			dmg += d(2,4); //Add segment damage
	}
	/* madness can make the player take more damage */
	if (youdef) {
		if (u.umadness&MAD_SUICIDAL){
			dmg += ((100 - u.usanity)*u.ulevel) / 200;
		}

		if ((pa->mlet == S_SNAKE
			|| pa->mlet == S_NAGA
			|| pa == &mons[PM_COUATL]
			|| pa == &mons[PM_LILLEND]
			|| pa == &mons[PM_MEDUSA]
			|| pa == &mons[PM_MARILITH]
			|| pa == &mons[PM_MAMMON]
			|| pa == &mons[PM_SHAKTARI]
			|| pa == &mons[PM_DEMOGORGON]
			|| pa == &mons[PM_GIANT_EEL]
			|| pa == &mons[PM_ELECTRIC_EEL]
			|| pa == &mons[PM_KRAKEN]
			|| pa == &mons[PM_SALAMANDER]
			|| pa == &mons[PM_KARY__THE_FIEND_OF_FIRE]
			|| pa == &mons[PM_CATHEZAR]
			) && u.umadness&MAD_OPHIDIOPHOBIA && u.usanity < 100){
			dmg += (100 - u.usanity) / 5;
		}

		if ((pa->mlet == S_WORM
			|| attacktype(pa, AT_TENT)
			) && u.umadness&MAD_HELMINTHOPHOBIA && u.usanity < 100){
			dmg += (100 - u.usanity) / 5;
		}

		if (!magr->female && humanoid_upperbody(pa) && u.umadness&MAD_ARGENT_SHEEN && u.usanity < 100){
			dmg += (100 - u.usanity) / 5;
		}

		if ((is_insectoid(pa) || is_arachnid(pa)) && u.umadness&MAD_ENTOMOPHOBIA && u.usanity < 100){
			dmg += (100 - u.usanity) / 5;
		}

		if (is_aquatic(pa) && u.umadness&MAD_THALASSOPHOBIA && u.usanity < 100){
			dmg += (100 - u.usanity) / 5;
		}

		if ((is_spider(pa)
			|| pa == &mons[PM_SPROW]
			|| pa == &mons[PM_DRIDER]
			|| pa == &mons[PM_PRIESTESS_OF_GHAUNADAUR]
			|| pa == &mons[PM_AVATAR_OF_LOLTH]
			) && u.umadness&MAD_ARACHNOPHOBIA && u.usanity < 100){
			dmg += (100 - u.usanity) / 5;
		}

		if (magr->female && humanoid_upperbody(pa) && u.umadness&MAD_ARACHNOPHOBIA && u.usanity < 100){
			dmg += (100 - u.usanity) / 5;
		}
	}

	/*	Next a cancellation factor	*/
	/*	Use uncancelled when the cancellation factor takes into account certain
	 *	armor's special magic protection.  Otherwise just use !mtmp->mcan.
	 */
	armpro = magic_negation(mdef);
	armuncancel = ((rn2(3) >= armpro) || !rn2(50));
	/* hack: elemental gaze attacks call this function with their AT_GAZE; we want that to ignore armor cancellation */
	if (attk->aatyp == AT_GAZE || attk->aatyp == AT_WDGZ)
		armuncancel = TRUE;
	notmcan = (youagr || !magr->mcan);
	uncancelled = notmcan && armuncancel;
	

	/* Do stuff based on damage type 
	 *  
	 * Physical Damage
	 *   hitmsg
	 *   damage
	 *   modifiers
	 *   more modifiers
	 *   on-hit effects
	 *   RETURN
	 * 
	 * Elemental Damage
	 *   if called directly (AT != AT_NONE)
	 *      makes 0d0 physical attack (hitmsg)
	 *      maybe return
	 *      then makes AT_NONE elemental damage attack
	 *   if called indirectly (AT_NONE)
	 *      elemental effects
	 *      damage (not reduced by DR or halfphysdamage)
	 *      RETURN
	 *   
	 * Status Effects [includes teleportation]
	 *   physical attack (hitmsg)
	 *   maybe return
	 *   special effects
	 * 
	 * Modified Attacks [includes seduction]
	 *   hitmsg
	 *   special effects
	 *   physical attack (no hitmsg)
	 * 
	 * Misc
	 *   hitmsg
	 *   special effects
	 *   followed by physical attack (no hitmsg)
	 *   
	 */

	/* intercept attacks dealing elemental damage to split them apart */
	if (attk->aatyp != AT_NONE && attk->aatyp != AT_GAZE && attk->aatyp != AT_WDGZ) {
		switch (attk->adtyp)
		{
		case AD_MAGM:
		case AD_EFIR:
		case AD_FIRE:
		case AD_ECLD:
		case AD_COLD:
		case AD_EELC:
		case AD_ELEC:
		case AD_EACD:
		case AD_ACID:
		case AD_DISE:
		case AD_POSN:
			/* make a 0 damage physical attack 
			 * This prints hitmsg and applies on-hit effects of any weapon
			 */
			alt_attk.aatyp = attk->aatyp;
			alt_attk.adtyp = AD_PHYS;
			result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, 0, dieroll, vis, ranged);
			/* return early if cannot continue the attack */
			if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;
			/* then, make the elemental attack */
			alt_attk = *attk;
			alt_attk.aatyp = AT_NONE;
			return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);
		default:
			break;
		}
	}

	boolean weaponattk = (attk->aatyp == AT_WEAP ||
		attk->aatyp == AT_XWEP ||
		attk->aatyp == AT_DEVA ||
		attk->aatyp == AT_HODS ||
		attk->aatyp == AT_MARI);

	switch (attk->adtyp)
	{
//////////////////////////////////////////////////////////////
// BASIC DAMAGE AND SLIGHTLY MODIFIED DAMAGE TYPES
//////////////////////////////////////////////////////////////
		/* 
		 * What almost every damage type ends up being
		 */
	case AD_PHYS:
	case AD_HODS:	/* should be deprecated in favour of just physical damage */
	case AD_SHDW:	/* poisoned, phases (blade of shadow) */
	case AD_STAR:	/* silvered, phases (silver starlight rapier) */
	case AD_BLUD:	/* bloodied, phases (blade of blood) */
	case AD_MERC:	/* poisoned, cold, phases (blade of mercury) */
	case AD_GLSS:	/* silvered (mirror-shards) */
		
		/* The Tentacle Rod has a unique hitmessage which will replace the usual hitmsg */
		if (vis&VIS_MAGR && weapon && arti_tentRod(weapon) && dohitmsg) {
			int extrahit = weapon->ostriking + 1;
			pline("The Tentacle Rod hits %s %d time%s%s",
				(youdef ? "you" : mon_nam(mdef)),
				extrahit,
				extrahit > 1 ? "s" : "",
				extrahit < 3 ? "." : extrahit < 7 ? "!" : "!!");
			dohitmsg = FALSE;
		}
		/* hit with [weapon] */
		result = hmon2point0(magr, mdef, attk, weapon, (struct obj *)0, (weapon && ranged), 0, dmg, dohitmsg, dieroll, FALSE, vis, &wepgone);
		if (result&(MM_DEF_DIED|MM_DEF_LSVD|MM_AGR_DIED))
			return result;
		if (weapon && multistriking(weapon) && weapon->ostriking) {
			int i;
			for (i = 0; (i < weapon->ostriking); i++) {
				result = hmon2point0(magr, mdef, attk, weapon, (struct obj *)0, (weapon && ranged), 0, 0, FALSE, dieroll, TRUE, vis, &wepgone);
				if (result&(MM_DEF_DIED|MM_DEF_LSVD|MM_AGR_DIED))
					return result;
			}
			do_weapon_multistriking_effects(magr, mdef, attk, weapon, vis);
		}
		return result;
//////////////////////////////////////////////////////////////
// PURE ELEMENTAL DAMAGE
//////////////////////////////////////////////////////////////
		/* 
		 * The damage can be resisted usually in full.
		 * For capital-E "Elemental" damage, resistance tends
		 * to be parial.
		 */
		/* magic */
	case AD_MAGM:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* does it do anything? Nullmagic gives utter and total immunity. MR gives immunity to the damage. */
		if (uncancelled
			&& !(youdef && Nullmagic)
			&& !(!youdef && mon_resistance(mdef, NULLMAGIC))){

			/* message */
			if (vis) {
				pline("%s%s lanced by magic!",
					(youdef ? "You" : Monnam(mdef)),
					(youdef ? "'re " : " is "));
			}

			if (Magic_res(mdef))
			{
				/* full resistance */
				dmg = 0;
				/* messages for resisted damage */
				if (youdef) {
					pline_The("magic bounces off!");
				}
				else if (vis) {
					pline_The("magic bounces off %s!",
						mon_nam(mdef));
					shieldeff(mdef->mx, mdef->my);
				}
			}
		}
		else
			dmg = 0;
		return xdamagey(magr, mdef, attk, dmg, FALSE);
		/* fire */
	case AD_EFIR:	/* elemental version, partially ignores resistance */
	case AD_FIRE:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* does it do anything? */
		if (uncancelled || attk->adtyp == AD_EFIR)
		{
			int olddmg = dmg;	// hack to remember dmg before it is reduced by resistances, so golemeffects() gets a good amount
			/* someone's on fire -- print onfire message! */
			if (vis) {
				pline("%s%s%s!",
					(youdef ? "You" : Monnam(mdef)),
					(youdef ? "'re " : " is "),
					on_fire(pd, attk));
			}

			/* burn away slime (player-only) */
			if (youdef)
				burn_away_slime();

			/* destory items in inventory */
			/* damage can only kill the player, right now, but it will injure monsters */
			if (!InvFire_res(mdef)){
				if ((int)mlev(magr) > rn2(20))
					destroy_item2(mdef, SCROLL_CLASS, AD_FIRE, youdef);
				if ((int)mlev(magr) > rn2(20))
					destroy_item2(mdef, POTION_CLASS, AD_FIRE, youdef);
				if ((int)mlev(magr) > rn2(25))
					destroy_item2(mdef, SPBOOK_CLASS, AD_FIRE, youdef);
			}
			/* reduce damage via resistance OR instakill */
			if (Fire_res(mdef))
			{
				if (attk->adtyp == AD_EFIR)	{ /* elemental, is reduced by resistance */
					/* resistances */
					if (youdef)	{
						/* you get 1/2 for intrinsic/extrinsic, 1/4 for both */
						if (HFire_resistance) dmg /= 2;
						if (EFire_resistance) dmg /= 2;
					}
					else {
						/* monsters just get 1/2, unless their species resists (below) */
						dmg /= 2;
					}
					/* immunities */
					if ((species_resists_fire(mdef))
						|| (ward_at(x(mdef), y(mdef)) == SIGIL_OF_CTHUGHA)
						|| (youdef && ((Race_if(PM_HALF_DRAGON) && flags.HDbreath == AD_FIRE)))
						|| (!youdef && is_half_dragon(pd) && mdef->mvar1 == AD_FIRE)
						|| (youdef && u.sealsActive&SEAL_FAFNIR))
						dmg = 0;
				}
				else {	/* standard, immunity from any resistance */
					dmg = 0;
				}

				/* messages for resisted damage */
				if (dmg == 0) {
					if (youdef) {
						pline_The("fire doesn't feel hot!");
					}
					else if (vis) {
						pline_The("fire doesn't seem to burn %s!",
							mon_nam(mdef));
						shieldeff(mdef->mx, mdef->my);
					}
				}
				else if (attk->adtyp == AD_EFIR && youdef) {
					pline_The("fire still feels hot%s",
						(dmg > 2 ? "!" : "."));
				}
			}
			else {	/* potential instakills to some creatures */
				if (pd == &mons[PM_STRAW_GOLEM] ||
					pd == &mons[PM_PAPER_GOLEM] ||
					pd == &mons[PM_SPELL_GOLEM]) {
					if (youdef) {
						/* assumes the player was polyed and not in natural form */
						You("burn up!");
						rehumanize();
						break;
					}
					else {
						if (vis)
							pline("%s burns completely!", Monnam(mdef));
						if (youagr)
							killed(mdef);
						else
							mondied(mdef);
						if (mdef->mhp > 0)
							return MM_DEF_LSVD;	/* lifesaved? */
						else if (mdef->mtame && !vis)
							pline("May %s roast in peace.", mon_nam(mdef));
						return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));	/* grow_up might kill magr */
					}
				}
				else if (pd == &mons[PM_MIGO_WORKER]) {
					if (youdef) {
						/* assumes the player was polyed and not in natural form */
						You("burn up!");
						rehumanize();
						break;
					}
					else {
						if (vis) pline("%s brain melts!", s_suffix(Monnam(mdef)));
						if (youagr)
							killed(mdef);
						else
							mondied(mdef);
						if (mdef->mhp > 0)
							return MM_DEF_LSVD;	/* lifesaved? */
						else if (mdef->mtame && !vis)
							pline("May %s roast in peace.", mon_nam(mdef));
						return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));	/* grow_up might kill magr */
					}
				}
			}
			/* some golems heal from fire damage */
			if (youdef)
				ugolemeffects(AD_FIRE, olddmg);
			else
				golemeffects(mdef, AD_FIRE, olddmg);
		}
		else
			dmg = 0;
		return xdamagey(magr, mdef, attk, dmg, FALSE);
		/* cold */
	case AD_ECLD:
	case AD_COLD:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* does it do anything? */
		if (uncancelled || attk->adtyp == AD_ECLD)
		{
			int olddmg = dmg;	// hack to remember dmg before it is reduced by resistances, so golemeffects() gets a good amount

			/* someone's freezing -- print mesage message! */
			if (vis) {
				pline("%s%s covered in frost!",
					(youdef ? "You" : Monnam(mdef)),
					(youdef ? "'re" : " is"));
			}
			/* madness: cold */
			if (youdef)
				roll_frigophobia();
			/* destory items in inventory */
			/* damage can only kill the player, right now, but it will injure monsters */
			if (!InvCold_res(mdef)){
				if ((int)mlev(magr) > rn2(20))
					destroy_item2(mdef, POTION_CLASS, AD_COLD, youdef);
			}
			/* reduce damage via resistance */
			if (Cold_res(mdef))
			{
				if (attk->adtyp == AD_ECLD)	{ /* elemental, is reduced by resistance */
					/* resistances */
					if (youdef)	{
						if (HCold_resistance) dmg /= 2;
						if (ECold_resistance) dmg /= 2;
					}
					else {
						dmg /= 2;
					}
					/* immunities */
					if ((species_resists_cold(mdef))
						|| (ward_at(x(mdef), y(mdef)) == BRAND_OF_ITHAQUA)
						|| (youdef && ((Race_if(PM_HALF_DRAGON) && flags.HDbreath == AD_COLD)))
						|| (!youdef && is_half_dragon(pd) && mdef->mvar1 == AD_COLD)
						|| (youdef && u.sealsActive&SEAL_AMON))
						dmg = 0;
				}
				else {	/* standard, immunity from any resistance */
					dmg = 0;
				}

				/* messages for resisted damage */
				if (dmg == 0) {
					if (youdef) {
						pline_The("frost doesn't seem cold!");
					}
					else if (vis) {
						pline_The("frost doesn't seem to chill %s!",
							mon_nam(mdef));
						shieldeff(mdef->mx, mdef->my);
					}
				}
				else if (attk->adtyp == AD_ECLD && youdef) {
					pline_The("frost still feels cold%s",
						(dmg > 2 ? "!" : "."));
				}
			}
			/* some golems are affected by cold damage
			* it doesn't appear to be a thing for player-polyed golems? */
			if (youdef)
				ugolemeffects(AD_COLD, olddmg);
			else
				golemeffects(mdef, AD_COLD, olddmg);
		}
		else
			dmg = 0;
		return xdamagey(magr, mdef, attk, dmg, FALSE);
		/* shock */
	case AD_EELC:
	case AD_ELEC:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* does it do anything? */
		if (uncancelled || attk->adtyp == AD_EELC)
		{
			int olddmg = dmg;	// hack to remember dmg before it is reduced by resistances, so golemeffects() gets a good amount

			/* someone's shocked -- print mesage message! */
			if (vis) {
				pline("%s %s zapped!",
					(youdef ? "You" : Monnam(mdef)),
					(youdef ? "get" : "gets"));
			}

			/* destory items in inventory */
			/* damage can only kill the player, right now, but it will injure monsters */
			if (!InvShock_res(mdef)){
				if ((int)mlev(magr) > rn2(20))
					destroy_item2(mdef, WAND_CLASS, AD_ELEC, youdef);
			}
			/* reduce damage via resistance */
			if (Shock_res(mdef))
			{
				if (attk->adtyp == AD_EELC)	{ /* elemental, is reduced by resistance */
					/* resistances */
					if (youdef)	{
						if (HShock_resistance) dmg /= 2;
						if (EShock_resistance) dmg /= 2;
					}
					else {
						dmg /= 2;
					}
					/* immunities */
					if ((species_resists_elec(mdef))
						|| (ward_at(x(mdef), y(mdef)) == TRACERY_OF_KARAKAL)
						|| (youdef && ((Race_if(PM_HALF_DRAGON) && flags.HDbreath == AD_ELEC)))
						|| (!youdef && is_half_dragon(pd) && mdef->mvar1 == AD_ELEC)
						|| (youdef && u.sealsActive&SEAL_ASTAROTH))
						dmg = 0;
				}
				else {	/* standard, immunity from any resistance */
					dmg = 0;
				}

				/* messages for resisted damage */
				if (dmg == 0) {
					if (youdef) {
						pline_The("zap doesn't shock you!");
					}
					else if (vis) {
						pline_The("zap doesn't shock %s!",
							mon_nam(mdef));
						shieldeff(mdef->mx, mdef->my);
					}
				}
				else if (attk->adtyp == AD_EELC && youdef) {
					pline_The("zap still shocks you%s",
						(dmg > 2 ? "!" : "."));
				}
			}
			/* some golems are affected by elec damage
			* it doesn't appear to be a thing for player-polyed golems? */
			if (youdef)
				ugolemeffects(AD_ELEC, olddmg);
			else
				golemeffects(mdef, AD_ELEC, olddmg);
		}
		else
			dmg = 0;
		return xdamagey(magr, mdef, attk, dmg, FALSE);
		/* acid */
	case AD_EACD:
	case AD_ACID:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* active? */
		if ((notmcan && !rn2(3)) || attk->adtyp == AD_EACD) {
			/* someone's splashed -- print mesage message! */
			if (vis) {
				Sprintf(buf, "%s%s covered in acid",
					(youdef ? "You" : Monnam(mdef)),
					(youdef ? "'re" : " is"));
			}
			if (Acid_res(mdef)) {
				if (attk->adtyp == AD_EACD)
					dmg /= 2;
				else
					dmg = 0;
			}
			/* print message */
			if (youdef){
				if (dmg == 0) {
					Sprintf(buf, "%s, but it seems harmless.", buf);
				}
				else if (Acid_res(mdef)) {
					Sprintf(buf, "%s! It stings!", buf);
				}
				else {
					Sprintf(buf, "%s! It burns!", buf);
				}
			}
			else {
				if (dmg == 0) {
					Sprintf(buf, "%s, but it seems unharmed.", buf);
				}
				///* random flavour -- a monster nearly killed by acid will scream */
				//else if (dmg > mdef->mhp * 2/3 && !is_silent_mon(mdef)){
				//	Sprintf(buf, "%s! %s screams!", buf, Monnam(mdef));
				//	wake_nearto(x(mdef), y(mdef), combatNoise(pd));
				//}
				else
					Sprintf(buf, "%s!", buf);
			}
			if (youdef || vis) {
				pline("%s", buf);
				if (dmg == 0 && !youdef)
					shieldeff(mdef->mx, mdef->my);
			}

			/* erode armor, if inventory isn't protected */
			if (!InvAcid_res(mdef)) {
				if (!rn2(10) || attk->adtyp == AD_EACD) {
					erode_armor(mdef, TRUE);
				}
			}
			/* abuse STR */
			if (youdef && !Acid_res(mdef))
				exercise(A_STR, FALSE);
		}
		else
			dmg = 0;
		return xdamagey(magr, mdef, attk, dmg, FALSE);
		/* sickness damage */
	case AD_DISE:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* asymetric: diseasemu prints out messages, applies sickness to player*/
		if (youdef) {
			if (!diseasemu(pa))
				dmg = 0;
		}
		else {
			if (Sick_res(mdef)) {
				if (vis) {
					pline("%s seems unharmed by the disease.",
						Monnam(mdef));
					shieldeff(mdef->mx, mdef->my);
				}
				dmg = 0;
			}
			else {
				if (vis) {
					pline("%s is afflicted by disease!",
						Monnam(mdef));
				}
			}
		}
		return xdamagey(magr, mdef, attk, dmg, FALSE);
		/* Only poison damage
		 * Not to be confused with a poisonous attack that adds poison bonus damage to a physical strike */
	case AD_POSN:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}

		if (Poison_res(mdef)){
			if (youdef) {
				pline("You're covered in poison, but it seems harmless.");
			}
			else if (vis) {
				pline("%s is covered in poison, but it seems unharmed.",
					Monnam(mdef));
				shieldeff(mdef->mx, mdef->my);
			}
			dmg = 0;
		}
		else {
			if (youdef) {
				pline("You're covered in poison! You don't feel so good!");
				exercise(A_STR, FALSE);
				exercise(A_INT, FALSE);
				exercise(A_CON, FALSE);
				exercise(A_DEX, FALSE);
			}
			else if (vis) {
				pline("%s is covered in poison!",
					Monnam(mdef));
			}
		}
		return xdamagey(magr, mdef, attk, dmg, FALSE);

//////////////////////////////////////////////////////////////
// PHYSICAL DAMAGE BEFORE NON-LETHAL SPECIAL EFFECTS
//////////////////////////////////////////////////////////////
		/* These deal a physical attack with their given dice,
		 * then apply a non-lethal effect, often a staus effect using the same sized dice.
		 * The physical damage is often minor, or non-existant (thus only printing a hit message)
		 */

	case AD_BLND:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		if (can_blnd(magr, mdef, attk->aatyp, (struct obj*)0)) {
			if (youdef){
				if (!Blind) pline("%s blinds you!", Monnam(magr));
				make_blinded(Blinded + (long)dmg, FALSE);
				if (!Blind) Your1(vision_clears);
			}
			else {
				if (canseemon(mdef) && !is_blind(mdef))
					pline("%s is blinded!", Monnam(mdef));

				int rnd_tmp = d((int)attk->damn, (int)attk->damd);
				if ((rnd_tmp += mdef->mblinded) > 127) rnd_tmp = 127;
				mdef->mblinded = rnd_tmp;
				mdef->mcansee = 0;
				mdef->mstrategy &= ~STRAT_WAITFORU;
			}
		}
		return result;

	case AD_STUN:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		if (notmcan && !rn2(4)) {
			if (youdef) {
				make_stunned(HStun + dmg, TRUE);
			}
			else
			{
				if (vis&VIS_MDEF) {
					pline("%s %s for a moment.",
						Monnam(mdef),
						makeplural(stagger(mdef, "stagger")));
				}
				mdef->mstun = 1;
			}
		}
		return result;

	case AD_CONF:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		if (notmcan && !rn2(4) && (youagr || !magr->mspec_used)) {
			/* put on cooldown */
			if (!youagr)
				magr->mspec_used = magr->mspec_used + (dmg + rn2(6));

			if (youdef) {
				if (Confusion)
					You("are getting even more confused.");
				else
					You("are getting confused.");
				make_confused(HConfusion + dmg, FALSE);
			}
			else
			{
				if (vis&VIS_MDEF)
					pline("%s looks confused.", Monnam(mdef));

				mdef->mconf = 1;
				mdef->mstrategy &= ~STRAT_WAITFORU;
			}
		}
		return result;

	case AD_HALU:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		if (notmcan && !rn2(4) && (youagr || !magr->mspec_used)) {
			/* put on cooldown */
			if (!youagr)
				magr->mspec_used = magr->mspec_used + (dmg + rn2(6));

			if (youdef) {
				if (!hallucinogenic(pd)) {
					boolean chg;
					if (!Hallucination)
						Your("mind is filled with kaleidoscopic light!");
					chg = make_hallucinated(HHallucination + (long)dmg, FALSE, 0L);
					You("%s.", chg ? "are freaked out" : "seem unaffected");
				}
			}
			/* monsters get confused by AD_HALU */
			else {
				if (vis&VIS_MDEF)
					pline("%s looks confused.", Monnam(mdef));

				mdef->mconf = 1;
				mdef->mstrategy &= ~STRAT_WAITFORU;
			}
		}
		return result;

	case AD_SLOW:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		if (uncancelled && !rn2(4)) {
			/* player */
			if (youdef){
				if (HFast && !defends(AD_SLOW, uwep)) {
					u_slow_down();
				}
			}
			/* monsters */
			else {
				unsigned int oldspeed = mdef->mspeed;

				mon_adjust_speed(mdef, -1, (struct obj *)0);
				mdef->mstrategy &= ~STRAT_WAITFORU;
				if (mdef->mspeed != oldspeed && canseemon(mdef))
					pline("%s slows down.", Monnam(mdef));
			}
		}
		return result;

	case AD_SLEE:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		if (uncancelled
			&& !Sleep_res(mdef)
			&& !cantmove(mdef)
			&& !rn2(7 - mlev(magr))) {
			if (youdef) {
				fall_asleep(-rnd(10), TRUE);
				if (Blind) You("are put to sleep!");
				else You("are put to sleep by %s!", mon_nam(magr));
			}
			else {
				if (sleep_monst(mdef, rnd(10), -1)) {
					pline("%s falls asleep!",
						Monnam(mdef));
					mdef->mstrategy &= ~STRAT_WAITFORU;
					slept_monst(mdef);
				}
			}
		}
		return result;

	case AD_PLYS:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		if (uncancelled && !cantmove(mdef) && !rn2(3)) {
			if (youdef){
				if (Free_action) {
					You("momentarily stiffen.");
				}
				else {
					if (Blind) You("are frozen!");
					else You("are frozen by %s!", mon_nam(magr));
					nomovemsg = 0;	/* default: "you can move again" */
					nomul(-dmg, "paralyzed by a monster");
					exercise(A_DEX, FALSE);
				}
			}
			else {
				if (mon_resistance(mdef, FREE_ACTION)) {
					if (canseemon(mdef)) {
						pline("%s momentarily stiffens.", Monnam(mdef));
					}
				}
				else {
					if (canseemon(mdef)) {
						pline("%s freezes!", Monnam(mdef));
					}
					mdef->mcanmove = 0;
					mdef->mfrozen = rnd(10);
					mdef->mstrategy &= ~STRAT_WAITFORU;
				}
			}
		}
		return result;

	case AD_STCK:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		if (uncancelled
			&& !(result & MM_AGR_DIED)
			&& (youagr || youdef)					/* the player must be involved in a sticking situation (gameplay limitation) */
			&& !u.ustuck							/* can't already be stuck */
			&& !(sticks(pa) && sticks(pd)))			/* creatures can't grab other grabbers (gameplay limitation)  */
		{
			if (pa == &mons[PM_TOVE])
				pline("%s %s much too slithy to stick to %s!",
				(youagr ? "You" : Monnam(magr)),
				(youagr ? "are" : "is"),
				((youdef && !youagr) ? "you" : mon_nam_too(mdef, magr))
				);
			else if (pd == &mons[PM_TOVE])
				pline("%s %s much too slithy to get stuck!",
				(youagr ? "You" : Monnam(magr)),
				(youagr ? "are" : "is")
				);
			else
				u.ustuck = (youagr ? mdef : youdef ? magr : u.ustuck);
		}
		return result;

	case AD_WERE:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		/* sometimes infect */
		if (uncancelled && !rn2(4)){
			/* only vs players*/
			if (youdef) {
				if (u.ulycn == NON_PM
					&& !Protection_from_shape_changers
					&& !spec_ability2(uwep, SPFX2_NOWERE)
					&& !umechanoid
					) {
					You_feel("feverish.");
					exercise(A_CON, FALSE);
					u.ulycn = monsndx(pa);
				}
			}
			else
				/* not implemented vs monsters */;
		}
		return result;

	case AD_POLY:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;
		/* polymorph? */
		if (uncancelled
			&& !(youdef ? Unchanging : mon_resistance(mdef, UNCHANGING))
			&& !(is_rider(pd) || resists_poly(pd))
			){
			/* forced polyself */
			if (youdef) {
				Your("DNA has been altered!");
				polyself(FALSE);
				if (rnd(100) < 15){
					Your("cellular structure degenerates.");
					losexp("cellular degeneration", TRUE, TRUE, FALSE);
				}
			}
			/* monsters only polymorph; never system shock or degenerate */
			else {
				newcham(mdef, (struct permonst *) 0, FALSE, vis);
			}
		}
		return result;

	case AD_DREN:
		/* we do NOT want to make a physical attack with these damage dice -- they tend to be quite large! */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, 0, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		if ((uncancelled && !rn2(4)) || pa == &mons[PM_PALE_NIGHT]) {
			if (youdef)
				drain_en(dmg);
			else
				/* Missing an equivalent for monsters. Perhaps this damage type should be skipped vs monsters. */;
		}
		return result;

//////////////////////////////////////////////////////////////
// PHYSICAL DAMAGE AFTER SPECIAL EFFECTS
//////////////////////////////////////////////////////////////
		/* Prints the hitmessage first,
		 * Then does special effects
		 * If the special effects are outright lethal, returns early.
		 * Otherwise, continues on to deal physical damage (without hitmsg) and use its result.
		 */

	case AD_RUST:
		/* print hitmessage */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}

		/* no special effect if cancelled */
		if (notmcan){
			/* instakills */
			if (pd == &mons[PM_IRON_GOLEM] ||
				pd == &mons[PM_CHAIN_GOLEM]) {
				if (youdef) {
					You("rust!");
					/* KMH -- this is okay with unchanging */
					rehumanize();
					return 0;
				}
				else {
					if (vis)
						pline("%s falls to pieces!", Monnam(mdef));

					if (youagr)
						killed(mdef);
					else
						mondied(mdef);

					if (mdef->mhp > 0)
						return MM_DEF_LSVD;
					else if (mdef->mtame && !vis && !youagr)
						pline("May %s rust in peace.", mon_nam(mdef));
					return (MM_HIT | MM_DEF_DIED | (youagr || grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
				}
			}

			/* rust armor */
			if (youdef) {
				hurtarmor(AD_RUST);
			}
			else {
				hurtmarmor(mdef, AD_RUST);
				mdef->mstrategy &= ~STRAT_WAITFORU;
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_DCAY:
		/* print hitmessage */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}

		/* no special effect if cancelled */
		if (notmcan){
			/* instakills */
			if (pd == &mons[PM_WOOD_GOLEM] ||
				pd == &mons[PM_GROVE_GUARDIAN] ||
				pd == &mons[PM_LIVING_LECTERN] ||
				pd == &mons[PM_LEATHER_GOLEM])  {

				if (youdef) {
					You("rot!");
					/* KMH -- this is okay with unchanging */
					rehumanize();
					return 0;
				}
				else {
					if (vis)
						pline("%s falls to pieces!", Monnam(mdef));

					if (youagr)
						killed(mdef);
					else
						mondied(mdef);
					if (mdef->mhp > 0)
						return 0;
					else if (mdef->mtame && !vis && !youagr)
						pline("May %s rot in peace.", mon_nam(mdef));
					return (MM_HIT | MM_DEF_DIED | (youagr || grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
				}
			}

			/* rot armor */
			if (youdef) {
				hurtarmor(AD_DCAY);
			}
			else {
				hurtmarmor(mdef, AD_DCAY);
				mdef->mstrategy &= ~STRAT_WAITFORU;
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_CORR:
		/* print hitmessage */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* corrode armor */
		if (youdef) {
			hurtarmor(AD_CORR);
		}
		else {
			hurtmarmor(mdef, AD_CORR);
			mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_WET:
	case AD_LETHE:
		/* print hitmessage */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
			if (youdef)
				You("are soaking wet!");
			else if (vis&VIS_MDEF)
				pline("%s is soaking wet!", Monnam(mdef));
		}
		/* water/lethe damage */
		water_damage((youdef ? invent : mdef->minvent), FALSE, FALSE, (attk->adtyp == AD_LETHE ? TRUE : FALSE), mdef);
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_ENCH:
		/* print hitmessage */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* disenchant */
		if (uncancelled) {
			otmp = some_armor(mdef);

			if (drain_item(otmp)) {
				if (youdef) {
					Your("%s less effective.",
						aobjnam(otmp, "seem"));
				}
				else if (vis&VIS_MDEF) {
					pline("%s %s less effective.",
						s_suffix(Monnam(mdef)),
						aobjnam(otmp, "seem"));
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

		/* various poisons */
	case AD_DRST:
	case AD_DRDX:
	case AD_EDRC:
	case AD_DRCO:
		/* select poison type */
		switch (attk->adtyp)
		{
		case AD_DRST:	ptmp = A_STR; break;
		case AD_DRDX:	ptmp = A_DEX; break;
		case AD_EDRC:
		case AD_DRCO:	ptmp = A_CON; break;
		default:		ptmp = A_STR; break;
		}
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* only activates 1/8 times even when uncancelled */
		if (uncancelled && !rn2(8)) {
			/* NOTE: poison acts quite differently betwen player and monster!!! */
			/* vs Player  */
			if (youdef) {
				/* rely on poisoned(), from mon.c */
				Sprintf(buf, "%s %s",
					s_suffix(Monnam(magr)), mpoisons_subj(magr, attk));
				poisoned(buf, ptmp, pa->mname, 30, 0);
			}
			/* vs Monster */
			else {
				/* print message, maybe */
				if (vis) {
					pline("%s %s was poisoned!",
						(youagr ? "Your" : s_suffix(Monnam(magr))),
						mpoisons_subj(magr, attk));
				}
				/* resistance */
				if (Poison_res(mdef)) {
					if (vis)
						pline_The("poison doesn't seem to affect %s.",
						mon_nam(mdef));
				}
				else {
					/* 9/10 odds of small bonus damage */
					if (rn2(10))
						mdef->mhp -= rn1(10, 6);	/* note that this is BONUS damage */
					/* 1/10 of deadly */
					else {
						if (vis)
							pline_The("poison was deadly...");
						mdef->mhp = 0;
					}
					/* if the poison killed, deal with the maybe-dead monster and return early */
					if (mdef->mhp < 1) {
						if (youagr)
							killed(mdef);
						else
							monkilled(mdef, "", attk->adtyp);
						/* is it dead, or was it lifesaved? */
						if (mdef->mhp > 0)
							return MM_DEF_LSVD;	/* lifesaved */
						else
							return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));
					}
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_VAMP:
	case AD_DRLI:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}

		/* biting to drain blood */
		if (attk->aatyp == AT_BITE
			&& has_blood_mon(mdef)
			&& !(pa == &mons[PM_VAMPIRE_BAT] && !(youdef ? u.usleep : mdef->msleeping))	/* vampire bats need sleeping victims */
			) {
			/* message for a player being drained */
			if (youdef) {
				Your("blood is being drained!");
			}

			/* blood bloaters heal*/
			if (pa == &mons[PM_BLOOD_BLOATER]) {
				/* aggressor lifesteals by dmg dealt */
				heal(magr, min(dmg, *hp(mdef)));
			}

			/* tame vampires gain nutrition */
			if (uncancelled && !youagr && magr->mtame && !magr->isminion && is_vampire(pa))
				EDOG(magr)->hungrytime += ((int)((mdef->data)->cnutrit / 20) + 1);
		}
		/* level-draining effect doesn't actually need blood, it drains life force */
		if ((uncancelled || (attk->adtyp == AD_VAMP && notmcan))
			&& !Drain_res(mdef)
			&& !(pa == &mons[PM_VAMPIRE_BAT] && !(youdef ? u.usleep : mdef->msleeping))	/* vampire bats need sleeping victims */
			&& !rn2(3)
			) {
			/* blood bloaters split (but not the player) */
			if (!youagr && pa == &mons[PM_BLOOD_BLOATER]){
				(void)split_mon(magr, 0);
			}

			/* metroids gain life (but not the player) */
			if (!youagr && is_metroid(pa)) {
				*hpmax(magr) += d(1, 4);
				heal(magr, d(1, 6));
				/* tame metroids gain nutrition (does not stack with for-vampires above) */
				if (magr->mtame && !magr->isminion){
					EDOG(magr)->hungrytime += pd->cnutrit / 4;  //400/4 = human nut/4
				}
			}

			/* this line should NOT be displayed in addition to "your blood is being drained" */
			if (youdef && !(attk->aatyp == AT_BITE && has_blood_mon(mdef)))
				pline("%s feeds on your life force!", Monnam(magr));

			/* drain life! */
			if (youdef) {
				/* the player has a handy level-drain function */
				losexp("life force drain", TRUE, FALSE, FALSE);
			}
			else {
				/* print message first -- this should happen before the victim is drained/dies */
				if (vis&VIS_MDEF)
					pline("%s suddenly seems weaker!", Monnam(mdef));

				/* for monsters, we need to make something up -- drain 2d6 maxhp, 1 level */
				dmg = d(2, 6);

				/* kill if this will level-drain below 0 m_lev, or lifedrain below 1 maxhp */
				if (mlev(mdef) == 0 || *hpmax(mdef) <= dmg) {
					/* clean up the maybe-dead monster, return early */
					if (youagr)
						killed(mdef);
					else
						monkilled(mdef, "", attk->adtyp);
					/* is it dead, or was it lifesaved? */
					if (mdef->mhp > 0)
						return MM_DEF_LSVD;	/* lifesaved */
					else
						return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));
				}
				else {
					/* drain stats */
					mdef->m_lev--;
					mdef->mhpmax -= dmg;
				}
			}
		}

		/* wraithworms have poisonous vampiric bites */
		if ((pa == &mons[PM_WRAITHWORM]
			|| pa == &mons[PM_FIRST_WRAITHWORM])) {
			alt_attk.adtyp = AD_DRST;
		}
		else {
			alt_attk.adtyp = AD_PHYS;
		}

		/* make attack without hitmsg */
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);


	case AD_DESC:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		if (nonliving(pd) || is_anhydrous(pd)){
			if (vis) {
				shieldeff(x(mdef), y(mdef));
			}
			dmg = 0;
		}
		else {
			/* message */
			if (vis) {
				pline("%s %s dehydrated!",
					(youdef ? "You" : Monnam(mdef)),
					(youdef ? "are" : "is")
					);
			}
			/* 2x damage to watery creatures */
			if (is_watery(pd))
				dmg *= 2;
			/* aggressor lifesteals by dmg dealt */
			heal(magr, min(dmg, *hp(mdef)));
		}
		/* make attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_STON:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}

		/* 1/3 chance of special effects */
		if (!rn2(3)) {
			/* a cancelled *trice only coughs*/
			if (!notmcan) {
				/* note: since the player cannot be cancelled, there is no cough for them. */
				/* message */
				if (vis) {
					You_hear("a cough from %s!", mon_nam(magr));
				}
				else if (dist2(u.ux, u.uy, x(magr), y(magr)) < 25) {	/* utterly arbitrary distance to hear cough */
					You_hear("a nearby cough.");
				}
			}
			/* hissing cockatrice */
			else {
				/* message */
				if (youagr) {
					You("hiss!");
				}
				else if (dist2(u.ux, u.uy, x(magr), y(magr)) < 25) {	/* utterly arbitrary distance to hear hissing */
					if (vis&VIS_MAGR)
						You_hear("%s hissing!", s_suffix(mon_nam(magr)));
					else
						You_hear("nearby hissing.");
				}
				/* stoning */
				if (!rn2(10) ||
					(flags.moonphase == NEW_MOON && (youdef && !have_lizard()))
					){
					/* do stone */
					result = xstoney(magr, mdef);
					if (result&(MM_DEF_DIED | MM_DEF_LSVD | MM_AGR_DIED))
						return result;
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_SLIM:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		if (uncancelled) {
			/* flaming immunity to slime */
			if (flaming(pd)
				|| pd == &mons[PM_RED_DRAGON]
				|| ((otmp = (youdef ? uarm : which_armor(mdef, W_ARM))) && (otmp->otyp == RED_DRAGON_SCALES || otmp->otyp == RED_DRAGON_SCALE_MAIL))
				|| ((otmp = (youdef ? uarms : which_armor(mdef, W_ARMS))) && (otmp->otyp == RED_DRAGON_SCALE_SHIELD))
				) {
				/* message if visible */
				if (vis) {
					pline_The("slime burns away!");
				}
			}
			/* unchanging immunity to slime */
			else if (Change_res(mdef)
				|| pd == &mons[PM_GREEN_SLIME]
				|| pd == &mons[PM_FLUX_SLIME]
				|| is_rider(pd)
				|| resists_poly(pd)) {
				/* only message for the player defending */
				if (youdef) {
					You("are unaffected.");
				}
			}
			/* affected! */
			else {
				/* you are slimed */
				if (youdef) {
					if (!Slimed) {
						You("don't feel very well.");
						Slimed = 10L;
						flags.botl = 1;
						killer_format = KILLED_BY_AN;
						delayed_killer = magr->data->mname;
					}
					else {
						pline("Yuck!");
					}
				}
				/* you slimed it */
				else if (youagr) {
					You("turn %s into slime.", mon_nam(mdef));
					(void)newcham(mdef, &mons[PM_GREEN_SLIME], FALSE, FALSE);
				}
				/* monster slimed it */
				else {
					(void)newcham(mdef, &mons[PM_GREEN_SLIME], FALSE, vis);
					mdef->mstrategy &= ~STRAT_WAITFORU;
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_WISD:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* maybe drain WIS */
		if (uncancelled && !rn2(8)) {
			/* player-only */
			if (youdef) {
				pline("%s assaults your sanity!", Monnam(magr));
				if (u.sealsActive&SEAL_HUGINN_MUNINN){
					unbind(SEAL_HUGINN_MUNINN, TRUE);
				}
				else {
					(void)adjattrib(A_WIS, -dmg / 10 + 1, FALSE);
					forget(10);	/* lose 10% of memory */
					exercise(A_WIS, FALSE);
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_NPDC:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* maybe drain CON */
		if (uncancelled) {
			/* player-only */
			if (youdef && ACURR(A_CON) > 3) {
				(void)adjattrib(A_CON, -1, FALSE);
			}
			/* monsters take d10 damage */
			else {
				result = xdamagey(magr, mdef, attk, rnd(10), FALSE);
				/* return early if cannot continue the attack */
				if (result&(MM_DEF_DIED|MM_DEF_LSVD))
					return result;
				/* otherwise continue to finish the attack (with physical damage) */
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_CURS:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		if ((notmcan && !rn2(10) || pa == &mons[PM_PALE_NIGHT])
			&& !(pa == &mons[PM_GREMLIN] && !night())
			)
		{
			/* message */
			if (flags.soundok) {
				if (youagr)
				{
					You("chuckle.");
				}
				else if (vis) {
					if (Blind) You_hear("laughter.");
					else       pline("%s chuckles.", Monnam(magr));
				}
			}
			/* effect */
			if (youdef) {
				if (u.umonnum == PM_CLAY_GOLEM || u.umonnum == PM_SPELL_GOLEM) {
					pline("Some writing vanishes from your head!");
					/* KMH -- this is okay with unchanging */
					rehumanize();
					return MM_DEF_LSVD;	/* You died but didn't actually die. Lifesaved. */
				}
				attrcurse();
			}
			else {
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
					if (youagr)
						xkilled(mdef, 0);
					else
						mondied(mdef);

					if (mdef->mhp > 0)
						return MM_DEF_LSVD;
					else if (mdef->mtame && !vis && !youagr)
						You("have a strangely sad feeling for a moment, then it passes.");
					return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_VORP:
		/* do NOT immediately print a basic hit message -- vorpality can cause a miss */

		/* 1/20 chance of a vorpal strike (or 20/20 vs jabberwocks) */
		if (!rn2(20) || pd == &mons[PM_JABBERWOCK])
			spec = TRUE;

		if (spec &&
			(!has_head(pd))) {
			if (vis){
				pline("Somehow, %s %s %s wildly.",
					(youagr ? "you" : mon_nam(magr)),
					(youagr ? "miss" : "misses"),
					(youdef ? "you" : mon_nam(mdef))
					);
			}
			return MM_MISS;
		}
		else {
			/* print basic hit message */
			if (vis && dohitmsg) {
				xyhitmsg(magr, mdef, attk);
			}
		}

		/* spec is true if vorpal */
		if (spec){
			if (noncorporeal(pd) || amorphous(pd)) {
				if (vis) {
					pline("%s slice%s through %s %s.",
						(youagr ? "You" : Monnam(magr)),
						(youagr ? "" : "s"),
						(youdef ? "your" : s_suffix(mon_nam(mdef))),
						(youdef ? body_part(NECK): mbodypart(mdef, NECK))
						);
				}
			}
			else {
				/* find helmet */
				otmp = (youdef ? uarmh : which_armor(mdef, W_ARMH));

				/* no helmet... DECAPITATION! */
				if (!otmp) {
					/* message */
					if (vis) {
						pline("%s decapitate%s %s!",
							(youagr ? "You" : Monnam(magr)),
							(youagr ? "" : "s"),
							(youdef ? "you" : mon_nam(mdef))
							);
					}
					/* kill */
					return xdamagey(magr, mdef, attk, FATAL_DAMAGE_MODIFIER, FALSE);
				}
				else {
					/* destroy the helmet */
					if (!otmp->oartifact || (pa == &mons[PM_DEMOGORGON])){
						if (youdef)
							claws_destroy_arm(otmp);
						else
							claws_destroy_marm(mdef, otmp);
					}
					/* double damage! */
					dmg *= 2;
				}
			}
		}

		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_SHRD:
		/* get a piece of worn armor */
		otmp = some_armor(mdef);

		/* armor protects ye */
		if (otmp){
			/* print custom message */
			if (vis && dohitmsg) {
				switch (attk->aatyp)
				{
				case AT_LNCK:
				case AT_BITE:
				case AT_5SBT:
					pline("%s teeth catch on %s armor!",
						(youagr ? "Your" : s_suffix(Monnam(magr))),
						(youdef ? "your" : s_suffix(mon_nam(mdef)))
						);
					break;
				case AT_STNG:
					pline("%s stinger catches on %s armor!",
						(youagr ? "Your" : s_suffix(Monnam(magr))),
						(youdef ? "your" : s_suffix(mon_nam(mdef)))
						);
					break;
				case AT_BUTT:
					pline("%s %s catch%s on %s armor!",
						(youagr ? "Your" : s_suffix(Monnam(magr))),
						(num_horns(pa) == 0 ? "head" : num_horns(pa) == 1 ? "horn" : "horns"),
						(num_horns(pa) == 1 ? "es" : ""),
						(youdef ? "your" : s_suffix(mon_nam(mdef)))
						);
					break;
				case AT_TENT:
					pline("%s tentacles catch on %s armor!",
						(youagr ? "Your" : s_suffix(Monnam(magr))),
						(youdef ? "your" : s_suffix(mon_nam(mdef)))
						);
					break;
				case AT_EXPL:
				case AT_BOOM:
					pline("%s shrapnel hits %s armor!",
						(youagr ? "Your" : s_suffix(Monnam(magr))),
						(youdef ? "your" : s_suffix(mon_nam(mdef)))
						);
					break;
				case AT_WEAP:
				case AT_XWEP:
				case AT_MARI:
					pline("%s %s %s armor!",
						(youagr ? "Your" : s_suffix(Monnam(magr))),
						(weapon ? "weapon strikes" : "claws catch on"),
						(youdef ? "your" : s_suffix(mon_nam(mdef)))
						);
					break;
				default:
					pline("%s claws catch on %s armor!",
						(youagr ? "Your" : s_suffix(Monnam(magr))),
						(youdef ? "your" : s_suffix(mon_nam(mdef)))
						);
					break;
				}
			}

			int i = 1;
			if (pa == &mons[PM_DEMOGORGON])
				i += rnd(4);

			for (; i>0; i--){
				if (otmp->spe > -1 * objects[(otmp)->otyp].a_ac){
					damage_item(otmp);
				}
				else if (!otmp->oartifact || (pa == &mons[PM_DEMOGORGON] && rn2(10))){
					if (youdef)
						claws_destroy_arm(otmp);
					else
						claws_destroy_marm(mdef, otmp);
					/* exit armor-destroying loop*/
					break;
				}
			}
			/* make a physical attack without hitmsg */
			alt_attk.adtyp = AD_PHYS;
			return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);
		}
		/* no armor */
		else {
			/* Demogorgon tries to kill */
			if (pa == &mons[PM_DEMOGORGON]) {
				if (noncorporeal(pd) || amorphous(pd)) {
					/* custom hit message */
					if (vis && dohitmsg) {
						pline("%s %s to rip %s apart!",
							(youagr ? "You" : Monnam(magr)),
							(youagr ? "try" : "tries"),
							(youdef ? "you" : mon_nam(mdef))
							);
					}
					/* Double damage */
					dmg *= 2;
					/* make a physical attack without hitmsg */
					alt_attk.adtyp = AD_PHYS;
					return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);
				}
				else {
					/* custom hit message */
					if (vis && dohitmsg) {
						pline("%s rip%s %s apart!",
							(youagr ? "You" : Monnam(magr)),
							(youagr ? "" : "s"),
							(youdef ? "you" : mon_nam(mdef))
							);
					}
					/* kill */
					return xdamagey(magr, mdef, attk, FATAL_DAMAGE_MODIFIER, FALSE);
				}
			}
		}
		/* make physical attack WITH hitmsg, since none of the cases above applied */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);

	case AD_MALK:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* attempt to stick */
		if ((youagr || youdef)					/* the player must be involved in a sticking situation (gameplay limitation) */
			&& !u.ustuck						/* can't already be stuck */
			&& !(sticks(pa) && sticks(pd))		/* grabbers can't grab other grabbers (gameplay limitation)  */
			){
			if (pd == &mons[PM_TOVE])
			{
				/* note: assumes player is either agr or def, vis is true */
				pline("%s, %s %s much too slithy to grab!",
					(youagr ? "Unfortunately" : "Fortunately"),
					(youagr ? mon_nam(mdef) : "you"),
					(youagr ? "is" : "are")
					);
			}
			else
				u.ustuck = (youdef ? magr : youagr ? mdef : u.ustuck); /* it is now stuck to you, and you to it */
		}
		/* electric damage */
		if (uncancelled) {
			dmg = d(1, 4);
			int old_aatyp = alt_attk.aatyp;	/* save aatyp to put back after making ELEC attack */
			alt_attk.aatyp = AT_NONE;
			alt_attk.adtyp = AD_ELEC;
			result = xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);
			if (result&(MM_DEF_DIED | MM_DEF_LSVD | MM_AGR_DIED))
				return result;
			alt_attk.aatyp = old_aatyp;
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);


	case AD_TCKL:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* weeping angels are entirely immune to the special effects */
		if (!is_weeping(pd)) {
			/* very different behaviour between mhitu and xhitm */
			if (youdef) {
				if (armuncancel && multi >= 0) {
					if (Free_action)
						You_feel("horrible fingers probing your flesh!  But you are not distracted!");
					else {
						if (Blind) You("are mercilessly tickled!");
						else You("are mercilessly tickled by %s!", mon_nam(magr));
						nomovemsg = 0;	/* default: "you can move again" */
						if (!rn2(3)) nomul(-rnd(10), "being tickled to death");
						else nomul(-1, "being tickled to death");
						exercise(A_DEX, FALSE);
						exercise(A_CON, FALSE);
						if (uwep && !welded(uwep)){
							if (d(1, 100) - min(ACURR(A_DEX), ACURR(A_CON)) > 0){
								if (u.twoweap){
									You("lose hold of your weapons.");
									u.twoweap = FALSE;
									otmp = uswapwep;
									setuswapwep((struct obj *)0);
									freeinv(otmp);
									(void)dropy(otmp);
								}
								else You("lose hold of your weapon.");
								otmp = uwep;
								uwepgone();
								freeinv(otmp);
								(void)dropy(otmp);
								if (roll_madness(MAD_TALONS)){
									You("panic after having dropping your weapon!");
									nomul(-1 * rnd(6), "panic");
								}
							}
							else{
								You("keep a tight grip on your %s!", u.twoweap ? "weapons" : "weapon");
							}
						}
					}
				}
				else if (uarmc){
					You_feel("horrible fingers tug at your cloak.");
					if (d(1, 100) - ACURR(A_DEX) > 0){
						pline("The horrible fingers pull off your cloak!");
						otmp = uarmc;
						if (donning(otmp)) cancel_don();
						(void)Cloak_off();
						freeinv(otmp);
						(void)dropy(otmp);
						if (roll_madness(MAD_TALONS)){
							You("panic after having your cloak taken!");
							nomul(-1 * rnd(6), "panic");
						}
					}
				}
				else if (uwep && uwep->oartifact == ART_TENSA_ZANGETSU){
					You_feel("horrible fingers tug at your shihakusho.");
				}
				else{
					if (uswapwep && !u.twoweap){
						if (d(1, 100) - ACURR(A_DEX) > 0){
							You_feel("your %s being stealthily drawn out of your belt.", doname(uswapwep));
							otmp = uswapwep;
							setuswapwep((struct obj *)0);
							freeinv(otmp);
							(void)mpickobj(magr, otmp);
							if (roll_madness(MAD_TALONS)){
								You("panic after having your property stolen!");
								nomul(-1 * rnd(6), "panic");
							}
						}
					}
					else if (uarm){
						You_feel("horrible fingers work at your armor.");
						if (d(1, 100) - ACURR(A_DEX) > 0 && d(1, 4) == 4){
							pline("The horrible fingers unfasten your armor!");
							otmp = uarm;
							if (donning(otmp)) cancel_don();
							(void)Armor_gone();
							freeinv(otmp);
							(void)dropy(otmp);
							if (roll_madness(MAD_TALONS)){
								You("panic after having your armor taken!");
								nomul(-1 * rnd(6), "panic");
							}
						}
					}
				}
			}
			/* vs monsters, only tickles them mercilessly */
			else {
				if (mdef->mcanmove) {
					if (vis) {
						pline("%s mercilessly %s %s.",
							(youagr ? "You" : Monnam(magr)),
							(youagr ? "tickle" : "tickles"),
							(youdef ? "you" : mon_nam(mdef))
							);
					}
					mdef->mcanmove = 0;
					mdef->mfrozen = rnd(10);
					mdef->mstrategy &= ~STRAT_WAITFORU;
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_SUCK:
		/* does nothing at all to noncorporeal or amoprhous creatures */
		if (noncorporeal(pd) || amorphous(pd))
		{
			/* no hitmessage, either */
			return MM_MISS;
		}
		else {
			/* chance for vorpal-esque head removal */
			otmp = (youdef ? uarmh : which_armor(mdef, W_ARMH));
			if (has_head(pd)
				&& !otmp
				&& !rn2(20)
				&& (*hp(mdef)*10 < *hpmax(mdef))
				){
				/* message */
				if (vis) {
					pline("%s suck%s %s %s off!",
						(youagr ? "You" : Monnam(magr)),
						(youagr ? "" : "s"),
						(youdef ? "your" : s_suffix(mon_nam(mdef))),
						(youdef ? body_part(HEAD) : mbodypart(mdef, HEAD))
						);
				}
				/* kill */
				return xdamagey(magr, mdef, attk, FATAL_DAMAGE_MODIFIER, FALSE);
			}
			/* most commonly, this path is taken */
			else {
				/* special hit message */
				if (youdef && dohitmsg) {
					You_feel("%s trying to suck your extremities off!",
						mon_nam(magr));
				}
				else if (vis && dohitmsg) {
					xyhitmsg(magr, mdef, attk);
				}
				/* 1/10 chance to twist legs (player-only) */
				if (youdef && !rn2(10)) {
					Your("%s twist from the suction!", makeplural(body_part(LEG)));
					set_wounded_legs(RIGHT_SIDE, rnd(60 - ACURR(A_DEX)));
					set_wounded_legs(LEFT_SIDE, rnd(60 - ACURR(A_DEX)));
					exercise(A_STR, FALSE);
					exercise(A_DEX, FALSE);
				}
				/* 1/6 chance to disarm */
				otmp = (youdef ? uwep : MON_WEP(mdef));
				if (otmp && !rn2(6)){
					/* message for players */
					if (youdef) {
						You_feel("%s pull on your weapon!", mon_nam(magr));
					}
					/* 1d130 > STR */
					if (d(1, 130) > (youdef ? ACURR(A_STR) : mdef->mstr)){
						if (youdef) {
							Your("weapon is sucked out of your grasp!");
							uwepgone();
							freeinv(otmp);
							(void)mpickobj(magr, otmp);
						}
						else {
							if (vis) {
								pline("%s weapon is sucked out of %s grasp!",
									s_suffix(Monnam(mdef)),
									hisherits(mdef)
									);
							}
							obj_extract_self(otmp);
							possibly_unwield(mdef, FALSE);
							setmnotwielded(mdef, otmp);
							if (youagr) {
								(void)hold_another_object(otmp, "You drop %s!", doname(otmp), (const char *)0);
							}
							else {
								(void)mpickobj(magr, otmp);
							}
						}
					}
					else if (youdef) {
						You("keep a tight grip on your weapon!");
					}
				}
				/* 1/10 chance to suck off boots */
				static int bboots1 = 0;
				if (!bboots1) bboots1 = find_bboots();
				otmp = (youdef ? uarmf : which_armor(mdef, W_ARMF));
				if (otmp
					&& otmp->otyp != bboots1
					&& !rn2(10)
					) {
					if (youdef) {
						Your("boots are sucked off!");
						if (donning(otmp)) cancel_don();
						(void)Boots_off();
						freeinv(otmp);
						(void)mpickobj(magr, otmp);
					}
					else {
						if (vis) {
							pline("%s boots are sucked off!",
								Monnam(mdef));
						}
						obj_extract_self(otmp);
						otmp->owornmask = 0L;
						update_mon_intrinsics(mdef, otmp, FALSE, FALSE);
						if (youagr) {
							(void)hold_another_object(otmp, "You drop %s!", doname(otmp), (const char *)0);
						}
						else {
							(void)mpickobj(magr, otmp);
						}
					}
				}
				/* 1/6 chance to suck off gloves */
				otmp = (youdef ? uarmg : which_armor(mdef, W_ARMG));
				if (otmp
					&& !rn2(6)
					) {
					/* message for players */
					if (youdef) {
						You_feel("%s pull on your gloves!", mon_nam(magr));
					}
					/* 1d40 > STR */
					if (d(1, 40) > (youdef ? ACURR(A_STR) : mdef->mstr)){
						if (youdef) {
							Your("gloves are sucked off!");
							if (donning(otmp)) cancel_don();
							(void)Gloves_off();
							freeinv(otmp);
							(void)mpickobj(magr, otmp);
						}
						else {
							if (vis) {
								pline("%s gloves are sucked off!",
									s_suffix(Monnam(mdef)));
							}
							obj_extract_self(otmp);
							otmp->owornmask = 0L;
							update_mon_intrinsics(mdef, otmp, FALSE, FALSE);
							if (youagr) {
								(void)hold_another_object(otmp, "You drop %s!", doname(otmp), (const char *)0);
							}
							else {
								(void)mpickobj(magr, otmp);
							}
						}

					}
					else if (youdef) {
						You("keep your %s closed.", makeplural(body_part(HAND)));
					}
				}
				/* 1/8 chance of sucking off shields */
				otmp = (youdef ? uarms : which_armor(mdef, W_ARMS));
				if (otmp
					&& !rn2(8)
					) {
					/* message for players */
					if (youdef) {
						You_feel("%s pull on your shield!", mon_nam(magr));
					}
					/* 1d150 > STR */
					if (d(1, 150) > (youdef ? ACURR(A_STR) : mdef->mstr)){
						if (youdef) {
							Your("shield is sucked out of your grasp!");
							if (donning(otmp)) cancel_don();
							(void)Shield_off();
							freeinv(otmp);
							(void)mpickobj(magr, otmp);
						}
						else {
							if (vis) {
								pline("%s shield is sucked out of %s grasp!",
									s_suffix(Monnam(mdef)),
									hisherits(mdef)
									);
							}
							obj_extract_self(otmp);
							otmp->owornmask = 0L;
							update_mon_intrinsics(mdef, otmp, FALSE, FALSE);
							if (youagr) {
								(void)hold_another_object(otmp, "You drop %s!", doname(otmp), (const char *)0);
							}
							else {
								(void)mpickobj(magr, otmp);
							}
						}

					}
					else if (youdef) {
						You("keep a tight grip on your shield!");
					}
				}
				/* 1/4 chance to suck off helmets */
				otmp = (youdef ? uarmh : which_armor(mdef, W_ARMH));
				if (otmp
					&& !rn2(4)
					) {
					if (youdef) {
						Your("helmet is sucked off!");
						if (donning(otmp)) cancel_don();
						(void)Helmet_off();
						freeinv(otmp);
						(void)mpickobj(magr, otmp);
					}
					else {
						if (vis) {
							pline("%s helmet is sucked off!",
								Monnam(mdef));
						}
						obj_extract_self(otmp);
						otmp->owornmask = 0L;
						update_mon_intrinsics(mdef, otmp, FALSE, FALSE);
						if (youagr) {
							(void)hold_another_object(otmp, "You drop %s!", doname(otmp), (const char *)0);
						}
						else {
							(void)mpickobj(magr, otmp);
						}
					}
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_UVUU:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* chance for vorpal-esque headsmashing */
		if (!rn2(20)){
			/* lack of head is only a minor issue */
			if (!has_head(pd)) {
				/* 2x damage */
				dmg *= 2;
			}
			/* so is having a pass-through head */
			else if (noncorporeal(pd) || amorphous(pd)) {
				/* message */
				if (vis) {
					pline("%s %s through %s %s.",
						(youagr ? "You" : Monnam(magr)),
						(youagr ? "pass" : "passes"),
						(youdef ? "your" : hisherits(mdef)),
						(youdef ? body_part(HEAD) : mbodypart(mdef, HEAD))
						);
				}
				/* 2x damage */
				dmg *= 2;
			}
			/* smash! */
			else {
				otmp = (youdef ? uarmh : which_armor(mdef, W_ARMH));

				/* no helmet... SMASH! */
				if (!otmp) {
					/* message */
					if (vis) {
						pline("%s %s %s %s!",
							(youagr ? "You" : Monnam(magr)),
							(youagr ? "smash" : "smashes"),
							(youdef ? "your" : s_suffix(mon_nam(mdef))),
							(youdef ? body_part(HEAD) : mbodypart(mdef, HEAD))
							);
					}
					/* kill */
					return xdamagey(magr, mdef, attk, FATAL_DAMAGE_MODIFIER, FALSE);
				}
				else {
					/* helmet protected */
					/* this doesn't destroy the helmet? */
					if (vis) {
						pline("%s headspike hits %s %s!",
							(youagr ? "Your" : s_suffix(Monnam(magr))),
							(youdef ? "your" : s_suffix(mon_nam(mdef))),
							xname(otmp)
							);
					}
					/* no bonus effects */
				}
			}
		}
		/* con-draining (player only?) */
		if (youdef) {
			Sprintf(buf, "%s %s",
				s_suffix(Monnam(magr)), mpoisons_subj(magr, attk));
			poisoned(buf, A_CON, pa->mname, 60, 0);
		}
		/* wis-draining (player only) */
		if (youdef) {
			int wisdmg = (dmg / 6) + 1;

			if (Poison_resistance)
				wisdmg -= ACURR(A_CON) / 2;

			if (wisdmg > 0){
				if (u.sealsActive&SEAL_HUGINN_MUNINN){
					unbind(SEAL_HUGINN_MUNINN, TRUE);
				}
				else {
					while (ABASE(A_WIS) > ATTRMIN(A_WIS) && wisdmg > 0){
						wisdmg--;
						(void)adjattrib(A_WIS, -1, TRUE);
						forget(10);
						exercise(A_WIS, FALSE);
					}
					if (AMAX(A_WIS) > ATTRMIN(A_WIS) &&
						ABASE(A_WIS) < AMAX(A_WIS) / 2) AMAX(A_WIS) -= 1; //permanently drain wisdom
					if (wisdmg){
						boolean chg;
						chg = make_hallucinated(HHallucination + (long)(wisdmg * 5), FALSE, 0L);
					}
				}
			}
		}
		/* energy draining (player only) */
		if (youdef) {
			drain_en((int)(dmg / 2));
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_TENT:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* the tentacle attack is only implemented vs the player */
		if (youdef) {
			u.ustuck = magr; //can steal stickage from other monsters.
#ifdef SEDUCE
			dotent(magr, dmg);
#endif
			change_usanity(u_sanity_loss(magr));
		}
		/* Might be technically incorrect to make the player also take an AT_TENT AD_PHYS attack afterwards
		 * but it really simplifies the flow to use the standard behaviour of [special effects] -> [basic damage]
		 */
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_WEBS:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		if (TRUE) {
			struct trap *ttmp2 = maketrap(x(mdef), y(mdef), WEB);
			if (ttmp2) {
				if (youdef) {
					pline_The("webbing sticks to you. You're caught!");
					dotrap(ttmp2, NOWEBMSG);
#ifdef STEED
					if (u.usteed && u.utrap) {
						/* you, not steed, are trapped */
						dismount_steed(DISMOUNT_FELL);
					}
#endif
				}
				else {
					mintrap(mdef);
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_STDY:
		/* study before doing the attack */
		if (notmcan) {
			int * stdy = (youdef ? &(u.ustdy) : &(mdef->mstdy));
			/* only print message or increase study if it is an increase */
			if (dmg > *stdy){
				/* message */
				if (is_orc(pa)) {
					pline("%s %s and urges %s followers on.",
						(youagr ? "You" : Monnam(magr)),
						(youagr ? "curse" : "curses"),
						(youagr ? "your" : mhis(magr)));
				}
				else if (pa == &mons[PM_LEGION] || pa == &mons[PM_LEGIONNAIRE]); //no message
				else if (youagr || canseemon(magr)) {
					pline("%s %s %s with a level stare.",
						(youagr ? "You" : Monnam(magr)),
						(youagr ? "study" : "studies"),
						(youdef ? "you" : mon_nam(mdef))
						);
				}
				/* increase study */
				(*stdy) = dmg;
			}
		}
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);


//////////////////////////////////////////////////////////////
// SEDUCTION / ITEM-STEALING
//////////////////////////////////////////////////////////////
		/* except when aimed at the player, these 
		 * are identical */
	case AD_SITM:
	case AD_SEDU:
#ifdef SEDUCE
	case AD_SSEX:
#endif
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		if (youdef) {
			static int engagering1 = 0;
			static int engagering4 = 0;
			boolean engring = FALSE;

			/* spaghetti code alert: many paths of code in here return early */
			switch (attk->adtyp) {
			case AD_SITM:
				if (!(u.sealsActive&SEAL_ANDROMALIUS)
					&& notmcan){
					switch (steal(magr, buf, FALSE, TRUE))
					{
					case -1:
						return MM_AGR_DIED;
					case 0:
						m_dowear(magr, FALSE);
						return MM_HIT;
					default:
						if (*buf) {
							if (canseemon(magr))
								pline("%s tries to %s away with %s.",
								Monnam(magr),
								locomotion(magr, "run"),
								buf);
						}
						monflee(magr, 0, FALSE, FALSE);
						return MM_AGR_STOP;
					}
				}
				break;

			case AD_SEDU:
				if (!engagering4) engagering4 = find_engagement_ring();
				if ((uleft && uleft->otyp == engagering4) || (uright && uright->otyp == engagering4)) engring = TRUE;
				if (u.sealsActive&SEAL_ANDROMALIUS) break;
				//pline("test string!");
				if (pa == &mons[PM_DEMOGORGON]){
					buf[0] = '\0';
					steal(magr, buf, FALSE, FALSE);
					m_dowear(magr, FALSE);
					return MM_HIT;
				}
				if ((pa == &mons[PM_FIERNA] || pa == &mons[PM_PALE_NIGHT]) && rnd(20) < 15) return MM_HIT;
				if (((MON_WEP(magr)) && pa == &mons[PM_ALRUNES]) && !rn2(20)) return MM_HIT;
				if (dmgtype(youracedata, AD_SEDU)
#ifdef SEDUCE
					|| dmgtype(youracedata, AD_SSEX) || dmgtype(youracedata, AD_LSEX)
#endif
					) {
					pline("%s %s.", Monnam(magr), magr->minvent ?
						"brags about the goods some dungeon explorer provided" :
						"makes some remarks about how difficult theft is lately");
					if (!tele_restrict(magr)) (void)rloc(magr, FALSE);
					return MM_AGR_STOP;
				}
				else if (magr->mcan || engring) {
					if (!Blind) {
						pline("%s tries to %s you, but you seem %s.",
							Adjmonnam(magr, "plain"),
							(is_neuter(magr->data) || flags.female == magr->female) ? "charm" : "seduce",
							(is_neuter(magr->data) || flags.female == magr->female) ? "unaffected" : "uninterested");
					}
					if (rn2(3)) {
						if (!tele_restrict(magr)) (void)rloc(magr, FALSE);
						return MM_AGR_STOP;
					}
					break;
				}
				buf[0] = '\0';
				switch (steal(magr, buf, FALSE, FALSE)) {
				case -1:
					return MM_AGR_DIED;
				case 0:
					m_dowear(magr, FALSE);
					return MM_HIT;
				default:
					if (!tele_restrict(magr))
						(void)rloc(magr, FALSE);
					monflee(magr, 0, FALSE, FALSE);
					return MM_AGR_STOP;
				}
				break;

#ifdef SEDUCE
			case AD_SSEX:
				if (!engagering1) engagering1 = find_engagement_ring();
				if ((uleft && uleft->otyp == engagering1) || (uright && uright->otyp == engagering1))
					break;

				if (pa == &mons[PM_MOTHER_LILITH] && could_seduce(magr, &youmonst, attk) == 1){
					magr->mcan = 0;	/* Question for Chris: is this intentional? It's different from all others here. */
					if (!rn2(4)) return MM_HIT;
					if (dolilithseduce(magr)) return MM_AGR_STOP;
				}
				else if (pa == &mons[PM_BELIAL] && could_seduce(magr, &youmonst, attk) == 1){
					if (!rn2(4)) return MM_HIT;
					if (dobelialseduce(magr)) return MM_AGR_STOP;
				}
				//	else if(pa == &mons[PM_SHAMI_AMOURAE] && could_seduce(magr, &youmonst, attk) == 1 
				//		&& notmcan){
				//		if(dosflseduce(magr)) return MM_AGR_STOP;
				//	}
				//	else if(pa == &mons[PM_THE_DREAMER] && could_seduce(magr, &youmonst, attk) == 2 
				//		&& notmcan){
				//	}
				//	else if(pa == &mons[PM_XINIVRAE] && could_seduce(magr, &youmonst, attk) == 2 
				//		&& notmcan){
				//	}
				//	else if(pa == &mons[PM_LYNKHAB] && could_seduce(magr, &youmonst, attk) 
				//		&& notmcan){
				//	}
				else if (pa == &mons[PM_MALCANTHET] && could_seduce(magr, &youmonst, attk)
					&& notmcan){
					if (domlcseduce(magr)) return MM_AGR_STOP;
				}
				else if (pa == &mons[PM_GRAZ_ZT] && could_seduce(magr, &youmonst, attk)
					&& notmcan){
					if (dograzseduce(magr)) return MM_AGR_STOP;
				}
				else if (pa == &mons[PM_PALE_NIGHT] && could_seduce(magr, &youmonst, attk)
					&& notmcan){
					dopaleseduce(magr);
					return MM_AGR_STOP;
				}
				else if (pa == &mons[PM_AVATAR_OF_LOLTH] && could_seduce(magr, &youmonst, attk)
					&& notmcan){
					dololthseduce(magr);
					return MM_AGR_STOP;
				}
				else if (could_seduce(magr, &youmonst, attk) == 1
					&& notmcan){
					if (doseduce(magr))
						return MM_AGR_STOP;
				}
				break;
#endif /* SEDUCE */
			}
		}
		/* the player just steals an item from a monster */
		else if (youagr) {
			steal_it(mdef, attk);
		}
		/* monsters just steal items from each other */
		else {
			if (notmcan) {
				/* select item from defender's inventory */
				for (otmp = mdef->minvent; otmp; otmp = otmp->nobj)
				if (!magr->mtame || !otmp->cursed)
					break;

				if (otmp) {
					char onambuf[BUFSZ], mdefnambuf[BUFSZ];

					/* make a special x_monnam() call that never omits
					the saddle, and save it for later messages */
					Strcpy(mdefnambuf, x_monnam(mdef, ARTICLE_THE, (char *)0, 0, FALSE));

					otmp = otmp;
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
							setmnotwielded(mdef, otmp);
						otmp->owornmask = 0L;
						update_mon_intrinsics(mdef, otmp, FALSE, FALSE);
					}
					/* add_to_minv() might free otmp [if it merges] */
					if (vis)
						Strcpy(onambuf, doname(otmp));
					(void)add_to_minv(magr, otmp);
					if (vis) {
						Strcpy(buf, Monnam(magr));
						pline("%s steals %s from %s!", buf,
							onambuf, mdefnambuf);
					}
					possibly_unwield(mdef, FALSE);
					mdef->mstrategy &= ~STRAT_WAITFORU;
					mselftouch(mdef, (const char *)0, FALSE);
					if (mdef->mhp <= 0)
						return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));
					if (magr->data->mlet == S_NYMPH &&
						!tele_restrict(magr)) {
						(void)rloc(magr, FALSE);
						result |= MM_AGR_STOP;
						if (vis && !canspotmon(magr))
							pline("%s suddenly disappears!", buf);
					}
					m_dowear(magr, FALSE);
				}
			}
		}
		return result;

		/* steal gold is... similar in that it steals items? */
	case AD_SGLD:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		/* youdef vs youagr vs mvm are all separate */
		if(youdef){
			if(notmcan
				&& !(pd->mlet == pa->mlet)
				&& !(u.sealsActive&SEAL_ANDROMALIUS)
			){
				stealgold(magr);
			}
		}
		else if (youagr) {
#ifndef GOLDOBJ
			if (mdef->mgold) {
				u.ugold += mdef->mgold;
				mdef->mgold = 0;
				Your("purse feels heavier.");
			}
#else
			/* This you as a leprechaun, so steal
			real gold only, no lesser coins */
			{
				struct obj *mongold = findgold(mdef->minvent);
				if (mongold) {
					obj_extract_self(mongold);
					if (merge_choice(invent, mongold) || inv_cnt() < 52) {
						addinv(mongold);
						Your("purse feels heavier.");
					}
					else {
						You("grab %s's gold, but find no room in your knapsack.", mon_nam(mdef));
						dropy(mongold);
					}
				}
			}
#endif
			exercise(A_DEX, TRUE);
		}
		else {
			if (notmcan) {
#ifndef GOLDOBJ
				if (mdef->mgold) {
					/* technically incorrect; no check for stealing gold from
					 * between mdef's feet...
					 */
					magr->mgold += mdef->mgold;
					mdef->mgold = 0;
#else
					/* technically incorrect; no check for stealing gold from
					* between mdef's feet...
					*/
					struct obj *gold = findgold(mdef->minvent);
					if (gold) {
						obj_extract_self(gold);
						add_to_minv(magr, gold);
#endif
					mdef->mstrategy &= ~STRAT_WAITFORU;
					if (vis) {
						Strcpy(buf, Monnam(magr));
						pline("%s steals some gold from %s.", buf, mon_nam(mdef));
					}
					if (!tele_restrict(magr)) {
						(void)rloc(magr, FALSE);
						result |= MM_AGR_STOP;
						if (vis && !canspotmon(magr))
							pline("%s suddenly disappears!", Monnam(magr));
					}
				}
			}
		}
		return result;

	case AD_SAMU:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* only special vs the player */
		if (youdef) {
			/* when the Wiz hits, 1/20 steals the amulet */
			if (u.uhave.amulet ||
				u.uhave.bell || u.uhave.book || u.uhave.menorah
				) /* carrying the Quest Artifact */
			if (!rn2(20)) stealamulet(magr);
		}
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_SQUE:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* only special vs the player */
		if (youdef) {
			/* when the Nemeses hits, 1/10 steals the something special */
			if (u.uhave.amulet ||
				u.uhave.bell || u.uhave.book || u.uhave.menorah
				|| u.uhave.questart) /* carrying the Quest Artifact */
			if (!rn2(10)) stealquestart(magr);
		}
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_STTP:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* only implemented vs player */
		if (youdef) {
			otmp = some_armor(&youmonst);
			if (otmp)
				teleport_arm(otmp);
		}
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

//////////////////////////////////////////////////////////////
// TELEPORTING ATTACKS
//////////////////////////////////////////////////////////////
		/* Deal physical damage first.
		 * The defender needs to be alive to be teleported.
		 */

	case AD_TLPT:
	case AD_ABDC:
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		/* mhitu */
		if (youdef) {
			/* sadly needs a hack to determine if the player was teleported to a new location,
			 * which would interrupt the attacker's attack chain */
			int oldux = u.ux, olduy = u.uy;
			switch (attk->adtyp)
			{
			case AD_TLPT:
				if (uncancelled) {
					if (flags.verbose)
						Your("position suddenly seems very uncertain!");
					tele();
				}
				break;
			case AD_ABDC:
				if (armuncancel || (pa == &mons[PM_PALE_NIGHT] && !rn2(3))) {
					(void)safe_teleds(FALSE);
				}
				break;
			}
			/* Were you moved? If so, take note. */
			if (oldux != u.ux || olduy != u.uy)
				result |= MM_AGR_STOP;	/* defender moved */
		}
		/* uhitm */
		else if (youagr) {
			if (uncancelled) {
				char nambuf[BUFSZ];
				boolean u_saw_mon = canseemon(mdef) ||
					(u.uswallow && u.ustuck == mdef);
				/* record the name before losing sight of monster */
				Strcpy(nambuf, Monnam(mdef));
				if (u_teleport_mon(mdef, FALSE)) {
					result |= MM_AGR_STOP;	/* defender moved */
					if (u_saw_mon && !canseemon(mdef)) {
						pline("%s suddenly disappears!", nambuf);
					}
				}
			}
		}
		/* mhitm */
		else {
			if (uncancelled && !tele_restrict(mdef)) {
				char mdef_Monnam[BUFSZ];
				/* save the name before monster teleports, otherwise
				we'll get "it" in the suddenly disappears message */
				if (vis) Strcpy(mdef_Monnam, Monnam(mdef));
				mdef->mstrategy &= ~STRAT_WAITFORU;
				(void)rloc(mdef, FALSE);
				result |= MM_AGR_STOP;	/* defender moved */
				if (vis && !canspotmon(mdef)
#ifdef STEED
					&& mdef != u.usteed
#endif
					)
					pline("%s suddenly disappears!", mdef_Monnam);
			}
		}
		return result;

	case AD_LVLT:	/* only perform the levelport, no draining or other weeping effects */
	case AD_WEEP:	/* significantly different when targeting player */
		/* make physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		/* return early if cannot continue the attack */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD))
			return result;

		/* only works when uncancelled */
		if (uncancelled) {
			/* mhitu, AD_WEEP is different from AD_LVLT */
			if (youdef) {
				switch (attk->adtyp)
				{
				case AD_LVLT:
					level_tele();
					break;
				case AD_WEEP:
					/* before endgame, tries to levelport you to drain your potential */
					if (!u.uevent.invoked){
						/* tport control is as good protection as drain res here, it needs to teleport you to drain you. */
						if (Teleport_control || Drain_resistance) {
							if (flags.verbose) You("feel like you could have lost some potential.");
						}
						/* levelported and drained */
						else {
							int potentialLost = 0;
							level_tele();
							You("suddenly feel like you've lost some potential.");
							potentialLost = min(abs(u.uz.dlevel - u.utolev.dlevel), u.ulevel - 1) / 2 + 1;
							for (; potentialLost>0; potentialLost--) losexp("loss of potential", FALSE, TRUE, TRUE); /*not verbose, force drain, drain exp also*/
							dmg = 0;
							result |= MM_AGR_STOP; /*You teleported, monster should stop attacking.*/
						}
					}
					/* during endgame, just drains levels (since levelporting is a no-go?) */
					else{
						if (Drain_resistance) {
							if (flags.verbose) You("feel like you could have lost some potential.");
						}
						else {
							You("suddenly feel like you've lost some potential.");
							losexp("loss of potential", FALSE, TRUE, TRUE); /*not verbose, force drain, drain exp also*/
						}
					}
					break;
				}//switch(adtyp)
			}
			/* uhitm and mhitm are both almost identically AD_LVLT */
			else {
				/* grab the name of mdef while it's visible */
				char mdef_Monnam[BUFSZ];
				if (vis)
					Strcpy(mdef_Monnam, Monnam(mdef));
				mdef->mstrategy &= ~STRAT_WAITFORU;
				/* Once the player performs the Invocation, weeping angels (that aren't you) will
				 * be too interested in your potential to feed off the potential of monsters */
				if (!youagr && u.uevent.invoked && attk->adtyp == AD_WEEP) {
					if (vis && canspotmon(magr) && flags.verbose)
						pline("%s is glancing at you with a hungry stare.", Monnam(magr));
				}
				/* levelport target */
				else {
#ifdef STEED
					if (u.usteed == mdef) {
						pline("%s vanishes from underneath you.", Monnam(mdef));
						dismount_steed(DISMOUNT_VANISHED);
					}
					else {
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
					result |= MM_AGR_STOP;	/* defender gone */
				}
			}
		}
		return result;
//////////////////////////////////////////////////////////////
// MISC
//////////////////////////////////////////////////////////////
	case AD_LEGS:
		/* this really only makes sense vs the player, sadly */
		/* if not attacking player, try to make a simple physical attack instead */
		if (!youdef) {
			/* if cancelled, deal no damage */
			if (!notmcan) {
				alt_attk.damn = 0;
				alt_attk.damd = 0;
			}
			alt_attk.adtyp = AD_PHYS;

			/* do this new attack */
			return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		}
		else {
			long side = rn2(2) ? RIGHT_SIDE : LEFT_SIDE;
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
				pline("%s tries to reach your %s %s!", Monnam(magr),
					sidestr, body_part(LEG));
				return MM_MISS;
			}
			else if (magr->mcan) {
				pline("%s nuzzles against your %s %s!", Monnam(magr),
					sidestr, body_part(LEG));
				return MM_MISS;
			}
			else {
				if (uarmf) {
					static int jboots1 = 0;
					if (!jboots1) jboots1 = find_jboots();
					if (rn2(2) && (uarmf->otyp == LOW_BOOTS ||
						uarmf->otyp == SHOES))
						pline("%s pricks the exposed part of your %s %s!",
						Monnam(magr), sidestr, body_part(LEG));
					else if (uarmf->otyp != jboots1 && !rn2(5))
						pline("%s pricks through your %s boot!",
						Monnam(magr), sidestr);
					else {
						pline("%s scratches your %s boot!", Monnam(magr),
							sidestr);
						alt_attk.damn = 0;
						alt_attk.damd = 0;
					}
				}
				else pline("%s pricks your %s %s!", Monnam(magr),
					sidestr, body_part(LEG));
				set_wounded_legs(side, rnd(60 - ACURR(A_DEX)));
				exercise(A_STR, FALSE);
				exercise(A_DEX, FALSE);
			}
		}

		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_DISN:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* maybe print glowy message */
		if (!Blind && (youdef || canseemon(mdef))){
			const char * glow = ((pa == &mons[PM_SWORD_ARCHON] || pa == &mons[PM_BAEL]) ?
				"faintly blue" : "sickly green");
			if (youdef)
				You("glow %s!", glow);
			else
				pline("%s glows %s!", Monnam(mdef), glow);
		}
		/* disintegrate! */
		for (; dmg > 0; dmg--)
		{
			/* grab a random piece of equipped armor */
			otmp = some_armor(mdef);

			if (otmp){
				if (obj_resists(otmp, 0, 80)){
					break; //End loop
				}
				else {
					if (oresist_disintegration(otmp))
						continue;
					if (otmp->spe > -1 * objects[otmp->otyp].a_ac){
						damage_item(otmp);
					}
					else if (!otmp->oartifact){
						destroy_arm(otmp);
					}
				}
			}
			else {
				if (!Disint_res(mdef)){
					/* you are disintegrated */
					if (youdef) {
						You("disintegrate!");
						killer_format = KILLED_BY;
						killer = pa->mname;
						done(DISINTEGRATED);
						You("reintegrate!");//lifesaved
						return MM_DEF_LSVD;
					}
					/* monster is disintegrated */
					else {
						pline("%s disintegrates!", Monnam(mdef));
						if (youagr)
							xkilled(mdef, 2);
						else
							monkilled(mdef, "", AD_DISN);
						if (mdef->mhp > 0)
							return MM_DEF_LSVD;	/* lifesaved */
						else
							return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));	/* grow_up might kill magr */
					}
				}
			}
		}
		return MM_HIT;

	case AD_DRIN:
		/* Special case for Migo.
		 * Migo only scoop out brains some of the time (1/20)
		 * Otherwise, they do a basic physical attack */
		if ((pa == &mons[PM_MIGO_PHILOSOPHER] || pa == &mons[PM_MIGO_QUEEN])
			&& rn2(20)) {
			/* make physical attack */
			alt_attk.adtyp = AD_PHYS;
			return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		}
		else {
			/* do the AD_DRIN attack, noting that we aren't eating brains */
			spec = TRUE;
		}

		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}

		/* entirely unharmed */
		if (defends(AD_DRIN, weapon)
			|| !has_head(pd)
			|| (youdef && umechanoid)
			/*|| notonhead*/ //damnit what
			) {
			if (vis) {
				pline("%s %s entirely unharmed.",
					(youdef ? "You" : Monnam(mdef)),
					(youdef ? "seem" : "seems")
					);
			}
			if (youdef && attk->aatyp == AT_TENT && roll_madness(MAD_HELMINTHOPHOBIA)){
				You("panic anyway!");
				nomul(-1 * rnd(3), "panicking");
			}
			/* don't do any further damage or anything, but do trigger retaliation attacks */
			return MM_HIT;
		}

		if (slips_free(magr, mdef, attk, vis)) {
			/* don't do any further damage or anything, but do trigger retaliation attacks */
			return MM_HIT;
		}

		/* protected by helmets */
		otmp = (youdef ? uarmh : which_armor(mdef, W_ARMH));
		if (otmp && ((
			otmp->otyp == PLASTEEL_HELM ||
			otmp->otyp == CRYSTAL_HELM ||
			otmp->otyp == PONTIFF_S_CROWN
			) || (
			rn2(8)
			))) {
			if (youdef) {
				/* not body_part(HEAD) */
				Your("helmet blocks the attack to your head.");
			}
			else if (vis) {
				Strcpy(buf, s_suffix(Monnam(mdef)));
				pline("%s helmet blocks %s attack to %s head.",
					buf, s_suffix(mon_nam(magr)),
					mhis(mdef));
			}
			/* don't do any further damage or anything, but do trigger retaliation attacks */
			return MM_HIT;
		}

		/* reduce damage through half-phys-damage */
		if (youdef ? Half_physical_damage : mon_resistance(mdef, HALF_PHDAM))
			dmg = (dmg+1)/2;

		/* brains will be attacked now */
		/* mhitu */
		if (youdef) {
			if (u.sealsActive&SEAL_HUGINN_MUNINN){
				unbind(SEAL_HUGINN_MUNINN, TRUE);
			}
			else {
				(void)adjattrib(A_INT, -dmg, FALSE);
				while (dmg--){
					forget(10);	/* lose 10% of memory per point lost*/
					exercise(A_WIS, FALSE);
				}
			}
			/* dunce caps prevent brain-eating messages, and brainlessness death */
			if (!(uarmh && uarmh->otyp == DUNCE_CAP)) {
				if (spec)
					Your("brain has been removed!");
				else
					Your("brain is eaten!");
				/* No such thing as mindless players... */
				if (ABASE(A_INT) <= 3) {
					int lifesaved = 0;
					struct obj *wore_amulet = uamul;

					while (1) {
						/* avoid looping on "die(y/n)?" */
						if (lifesaved && (discover || wizard)) {
							if (wore_amulet && !uamul) {
								/* used up AMULET_OF_LIFE_SAVING; still
								subject to dying from brainlessness */
								wore_amulet = 0;
							}
							else {
								/* explicitly chose not to die;
								arbitrarily boost intelligence */
								ABASE(A_INT) = ATTRMIN(A_INT) + 2;
								You_feel("like a scarecrow.");
								/* break deathloop */
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
				if (youdef && attk->aatyp == AT_TENT && roll_madness(MAD_HELMINTHOPHOBIA)){
					You("panic from the burrowing tentacles!");
					nomul(-1 * rnd(6), "panicking");
				}
				/* if a migo scooped out your brain, it stops attacking */
				if (pa == &mons[PM_MIGO_PHILOSOPHER] || pa == &mons[PM_MIGO_QUEEN])
					return MM_AGR_STOP;
			}
		}
		/* uhitm */
		else if (youagr) {
			int intdrain = dmg;	/* if we are actually tracking monster INT stat, use this */
			dmg += d(dmg, 10); /* fakery, since monsters lack INT scores. NOTE: 1d10 PER POINT OF INT */

			/* message */
			You("%s %s brain!",
				(spec ? "remove" : "eat"),
				s_suffix(mon_nam(mdef))
				);
			/* don't check this part if we aren't eating the brain */
			if (!spec) {
				u.uconduct.food++;
				if (touch_petrifies(mdef->data) && !Stone_resistance && !Stoned) {
					Stoned = 5;
					killer_format = KILLED_BY_AN;
					delayed_killer = mdef->data->mname;
				}
				if (!vegan(mdef->data))
					u.uconduct.unvegan++;
				if (!vegetarian(mdef->data))
					violated_vegetarian();
			}
			/* if victim was mindless, return early */
			if (mindless_mon(mdef)) {
				pline("%s doesn't notice.", Monnam(mdef));
				return MM_HIT;
			}
			/* don't check this part if we aren't eating the brain */
			if (!spec) {
				morehungry(-rnd(dmg * 6)); /* cannot choke; aprox 30 points of hunger per point of 'int' drained */
				if (ABASE(A_INT) < AMAX(A_INT)) {
					ABASE(A_INT) += rnd(4);
					if (ABASE(A_INT) > AMAX(A_INT))
						ABASE(A_INT) = AMAX(A_INT);
					flags.botl = 1;
				}
				exercise(A_WIS, TRUE);
				exercise(A_INT, TRUE);
			}

			/* kills if pretend-brain-eating damage would reduce hp below zero */
			mdef->mhp -= dmg;
			if (mdef->mhp < 1) {
				if (vis) {
					pline("%s last thought fades away...",
						s_suffix(Monnam(mdef)));
				}
				/* use killed, player was responsible */
				killed(mdef);
				if (mdef->mhp > 0)
					return MM_DEF_LSVD;	/* lifesaved */
				else
					return MM_DEF_DIED;
			}
		}
		/* mhitm */
		else {
			int intdrain = dmg;	/* if we are actually tracking monster INT stat, use this */
			dmg += d(dmg, 10); /* fakery, since monsters lack INT scores. NOTE: 1d10 PER POINT OF INT */

			if (vis)
				pline("%s brain is eaten!", s_suffix(Monnam(mdef)));
			if (mindless_mon(mdef)) {
				if (vis)
					pline("%s doesn't notice.", Monnam(mdef));
				return MM_HIT;
			}
			/* tame creatures get nutrition */
			if (magr->mtame && !magr->isminion) {
				EDOG(magr)->hungrytime += dmg * 11; /*aprox 60 points of hunger per 1d10*/
				magr->mconf = 0;
			}
			/* kills if pretend-brain-eating damage would reduce hp below zero */
			mdef->mhp -= dmg;
			if (mdef->mhp < 1) {
				if (vis) {
					pline("%s last thought fades away...",
						s_suffix(Monnam(mdef)));
				}
				/* use monkilled, player was not responsible */
				monkilled(mdef, "", AD_DRIN);
				if (mdef->mhp > 0)
					return MM_DEF_LSVD;	/* lifesaved */
				else
					return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));	/* grow_up might kill magr */
			}
		}
		/* don't do other damage modifers, we're done here. */
		return MM_HIT;

	case AD_HEAL:
		/* for hostiles, deal damage, not heal */
		if (((youdef || youagr) && !magr->mpeaceful) ||
			(magr->mpeaceful != mdef->mpeaceful)) {
			/* make physical attack */
			alt_attk.adtyp = AD_PHYS;
			result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		}
		else {
			/* don't print a basic hitmessage */
			result = MM_HIT;
		}
		/* from this point on, assumes non-hostility */
		/* mhitu */
		if (youdef) {
			/* to operate on the player, the player must be disrobed */
			if (!uarm && !uarmu) {

				pline("%s hits!  (I hope you don't mind.)", Monnam(magr));

				/* heal,
				 * increase maxhp using healup()'s diminishing returns,
				 * cure sickness
				 */
				healup(dmg, dmg/4, TRUE, FALSE);

				/* sometimes exercise STR, CON */
				if (!rn2(3)) exercise(A_STR, TRUE);
				if (!rn2(3)) exercise(A_CON, TRUE);

				/* and sometimes go away */
				if (!rn2(33)) {
					/* run/teleport away */
					if (!tele_restrict(magr))
						(void)rloc(magr, FALSE);
					monflee(magr, d(3, 6), TRUE, FALSE);
					return MM_AGR_STOP;	/* maybe teleported away, definitely not continuing to attack */
				}
				else if (!rn2(333)) {
					/* poof out of existance*/
					mongone(magr);
					return MM_AGR_DIED;
				}
			}
			/* no healing for you! */
			else {
				/* complain to a fellow healer */
				if (flags.soundok && !(moves % 5)) {
					if (Role_if(PM_HEALER))
						verbalize("Doc, I can't help you unless you cooperate.");
				}
				/* give the player some space */
				monflee(magr, d(3, 6), TRUE, FALSE);
				/* and miss */
				return (MM_MISS);
			}
		}
		/* uhitm, mhitm */
		else {
			/* my guess: uhitm behaviour is likely rendered useless by other
			 * circumstances and effects, but oh well */
			/* heals patient by dmg, up to its maxhp */
			mdef->mhp += dmg;
			if (mdef->mhpmax < mdef->mhp)
				mdef->mhp = mdef->mhpmax;
			/* message if patient is seen */
			if (canseemon(mdef))
				pline("%s looks better!", mon_nam(mdef));
		}
		return result;

	case AD_WRAP:
		/* Monsters cannot grab each other
		 * If the player isn't involved,
		 * or if either creature could be the one doing the grabbing,
		 * substitute simple physical damage */
		if (!(youagr || youdef)
			|| (sticks(pd))){
			/* make physical attack */
			alt_attk.adtyp = AD_PHYS;
			return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		}
		else {
			/* figure out who the other creature is */
			if (youagr)
				mtmp = mdef;
			else if (youdef)
				mtmp = magr;
			else {
				impossible("who is the other one being stuck?");
				return MM_MISS;
			}
		}

		/* is the player stuck to the other creature? */
		if (notmcan || (u.ustuck == mtmp)) {
			/* if not attached to anything, attempt to attach to the other creature*/
			if (!u.ustuck && !rn2(10)) {
				if (slips_free(magr, mdef, attk, vis)) {
					/* message was printed (if visible) */
					/* do nothing */;
				}
				else {
					/* get stuck */
					/* print a message (vis is assumed, since the player is involved) */
					pline("%s swing%s %s around %s!",
						(youagr ? "You" : Monnam(mtmp)),
						(youagr ? "" : "s"),
						(youagr ? "yourself" : "itself"),
						(youagr ? mon_nam(mtmp) : "you")
						);

					if (pd == &mons[PM_TOVE])
					{
						pline("%s, %s %s much too slithy to grab!",
							(youagr ? "Unfortunately" : "Fortunately"),
							(youagr ? mon_nam(mtmp) : "you"),
							(youagr ? "is" : "are")
							);
					}
					else
						u.ustuck = mtmp; /* it is now stuck to you, and you to it */
				}
			}
			/* if you are attached to the other creature, do the thing! */
			else if (u.ustuck == mtmp) {
				/* drowning? */
				if (is_pool(x(magr), y(magr), FALSE)
					&& !(youdef ? Swimming : mon_resistance(mdef, SWIMMING))
					&& !(youdef ? Breathless : breathless_mon(mdef))
					&& !(amphibious(pd))	/* Odd, need to check the species for amphibious. Fixme later? */
					){
					int ltyp = levl[x(magr)][y(magr)].typ;
					boolean moat =
						(ltyp != POOL) &&
						(ltyp != WATER) &&
						!Is_medusa_level(&u.uz) &&
						!Is_waterlevel(&u.uz);

					/* water damage to drownee's inventory */
					water_damage((youdef ? invent : mdef->minvent), FALSE, FALSE, level.flags.lethe, mdef);

					if (youdef) {
						if (u.divetimer > 0){
							pline("%s pulls you into the %s!", Monnam(mtmp), moat ? "moat" : "pool of water");
							if (!uarm || is_light_armor(uarm)){
								pline("%s is crushing the breath out of you!", Monnam(mtmp));
								u.divetimer -= dmg;
							}
							else if (is_medium_armor(uarm)){
								pline("%s is squeezing the breath out of you!", Monnam(mtmp));
								u.divetimer -= dmg / 2 + 1;
							}
							else {
								u.divetimer -= dmg / 4 + 1;
							}
							if (u.divetimer < 0) u.divetimer = 0;
						}
						else { /*out of air*/
							pline("%s drowns you...", Monnam(mtmp));
							killer_format = KILLED_BY_AN;
							Sprintf(buf, "%s by %s",
								moat ? "moat" : "pool of water",
								an(mtmp->data->mname));
							killer = buf;
							done(DROWNING);
							return MM_DEF_DIED;
						}
					}
					else if (youagr) {
						You("drown %s...", mon_nam(mdef));
						/* kill */
						return xdamagey(magr, mdef, attk, FATAL_DAMAGE_MODIFIER, FALSE);
					}
					else {
						impossible("Player must be youagr or youdef for drowning.");
						return MM_HIT;
					}
				}
				/* crushing bearhug? */
				else if (attk->aatyp == AT_HUGS){
					pline("%s is %s.",
						Monnam(mtmp),
						(youagr ? "being crushed" : "crushing you")
						);
				}
				/* Gentle squeeze. Restore 1d3 points of sanity. Just kidding. */
				else {
					dmg = 0;
					pline("%s is %s.",
						Monnam(mtmp),
						(youagr ? "wrapped up in you" : "wrapped tight around you")
						);
				}
			}
			/* nothing really happens */
			else {
				if (flags.verbose) {
					if (youagr) {
						You("brush against %s %s.",
							s_suffix(mon_nam(mdef)),
							mbodypart(mdef, LEG));
					}
					else if (youdef) {
						pline("%s brushes against your %s.", Monnam(mtmp),
							body_part(LEG));
					}
				}
				/* return miss -- nothing happens */
				return MM_MISS;
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_CHKH:
		/* make modified physical attack */
		/* gains much more damage vs player */
		if (youdef) {
			dmg += u.chokhmah * 5;
		}
		else {
			dmg += u.chokhmah;
		}
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);


	case AD_CHRN:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* vs player, do special effects like as a unicorn horn */
		if (youdef) {
			long lcount = (long)rnd(100);

			switch (rn2(6)) {
			case 0:
				make_sick(Sick ? Sick / 3L + 1L : (long)rn1(ACURR(A_CON), 20),
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
				(void)adjattrib(rn2(A_MAX), -1, FALSE);
				break;
			case 5:
				(void)make_hallucinated(HHallucination + lcount, TRUE, 0L);
				break;
			}

			alt_attk.adtyp = AD_PHYS;
		}
		/* vs monsters, make an appropriate damage+status effect bundled attack */
		else {
			switch (rn2(6)) {
			case 0:
				/* technically incorrect; this is pure sickness damage */
				alt_attk.adtyp = AD_DISE;
				break;
			case 1:
				alt_attk.adtyp = AD_BLND;
				break;
			case 2: 
				alt_attk.adtyp = AD_CONF;
				break;
			case 3:
				alt_attk.adtyp = AD_STUN;
				break;
			case 4:
				/* no good substitute for random stat drain */
				alt_attk.adtyp = AD_PHYS;
				break;
			case 5:
				alt_attk.adtyp = AD_HALU;
				break;
			}
		}
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_TELE:
		/* hitter tries to teleport without making an attack */
		if (!youagr) {
			if (!tele_restrict(magr)) {
				(void)rloc(magr, FALSE);
				return MM_AGR_STOP;
			}
		}
		else {
			/* sadly needs a hack to determine if the player was teleported to a new location,
			* which would interrupt the attacker's attack chain */
			int oldux = u.ux, olduy = u.uy;
			tele();
			/* Were you moved? If so, stop attacking. */
			if (oldux != u.ux || olduy != u.uy)
				return MM_AGR_STOP;	/* defender moved */
		}
		/* if that failed, make a physical attack instead */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);

	case AD_AXUS:
		/* fancy hitmsg */
		if (vis && dohitmsg) {
			pline("%s reaches out with %s %s!  A corona of dancing energy surrounds the %s!",
				Monnam(magr), mhis(magr), mbodypart(magr, ARM), mbodypart(magr, HAND));
		}
		/* shock / fire / stun combo */
		alt_attk.aatyp = AT_NONE;

		alt_attk.adtyp = AD_ELEC;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);		/* elec damage */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;
		alt_attk.adtyp = AD_FIRE;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);		/* fire damage */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;
		alt_attk.adtyp = AD_STUN;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, 3, dieroll, vis, ranged);		/* 3 turn stun, minor physical damage */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;

		/* but wait, there's more! */
		if (vis && dohitmsg) {
			pline("%s reaches out with %s other %s!  A penumbra of shadows surrounds the %s!",
				Monnam(magr), mhis(magr), mbodypart(magr, ARM), mbodypart(magr, HAND));
		}
		/* cold / drain combo */
		alt_attk.aatyp = AT_NONE;
		alt_attk.adtyp = AD_COLD;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);		/* cold damage */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;
		alt_attk.adtyp = AD_DRLI;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, 0, dieroll, vis, ranged);		/* level drain */
		if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;

		return result;

	case AD_ACFR:
		/* make a physical attack */
		alt_attk.adtyp = AD_PHYS;
		result = xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;
		alt_attk.aatyp = AT_NONE;
		/* add fire damage, if not resistant */
		if (!Fire_res(mdef)) {
			alt_attk.adtyp = AD_FIRE;
			result = xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);
			if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;
		}
		/* add holy damage */
		if (hates_holy_mon(mdef)) {
			if (vis) {
				pline("The holy flames sear %s!",
					(youdef ? "your flesh" : mon_nam(mdef))
					);
			}
			result = xdamagey(magr, mdef, &alt_attk, dmg, FALSE);
			if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;
		}

		return result;

	case AD_FRWK:
		/* creates many explosions! */
		/* no hit message? */
		{
		int i = 1 + rnd(3);
		for (; i > 0; i--) {
			explode(
				x(mdef) + rn2(3) - 1,	/* x center coord */
				y(mdef) + rn2(3) - 1,	/* y center coord */
				AD_PHYS,				/* damage type  */
				-1,						/* object causing explosion (none) */
				dmg,					/* damage per explosion */
				rn2(EXPL_MAX),			/* color */
				1);						/* radius */
		}
		if (!youdef && DEADMONSTER(mdef))
			return MM_DEF_DIED;
		else
			return MM_HIT;
		}

//////////////////////////////////////////////////////////////
// JUST CHANGING THE DAMAGE TYPE
//////////////////////////////////////////////////////////////
	case AD_OONA:
		/* use correct damage type */
		alt_attk.adtyp = u.oonaenergy;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);

	case AD_HDRG:
		/* use halfdragon's breath type */
		if (youagr && Race_if(PM_HALF_DRAGON))
			alt_attk.adtyp = flags.HDbreath;
		else if (is_half_dragon(pa))
			alt_attk.adtyp = magr->mvar1;
		else
			alt_attk.adtyp = AD_COLD;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);

	case AD_RBRE:	/* should actually be breath-only */
	case AD_RETR:
		/* random between fire/cold/elec damage */
		switch (rn2(3))
		{
		case 0: alt_attk.adtyp = AD_FIRE; break;
		case 1: alt_attk.adtyp = AD_COLD; break;
		case 2: alt_attk.adtyp = AD_ELEC; break;
		}
		return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);

//////////////////////////////////////////////////////////////
// BINDER SPIRIT ATTACKS
//////////////////////////////////////////////////////////////
	case AD_IRIS:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* note when the player last made an Iris attack */
		if (youagr) {
			u.irisAttack = moves;
		}
		/* 1/20 chance of shriveling opponent */
		if (!rn2(20)){
			if (!nonliving(pd) &&
				!is_anhydrous(pd)){
				/* message */
				if (vis) {
					pline("%s %s at the touch of %s iridescent tentacles!",
						(youdef ? "You" : Monnam(mdef)),
						(youdef ? "shrivel" : "shrivels"),
						(youagr ? "your" : s_suffix(mon_nam(magr)))
						);
				}
				/* bonus damage for the player (5 spirit dice) */
				if (youagr) {
					dmg += d(5, spiritDsize());
				}
				/* aggressor lifesteals by dmg dealt */
				heal(magr, min(dmg, *hp(mdef)));
			}
			else {
				shieldeff(x(mdef), y(mdef));
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_NABERIUS:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		if (!youdef && mdef->mflee) {	/* what would be an acceptable condition for the player to be affected? */
			if ((*hp(magr) < *hpmax(magr)) && vis) {
				pline("%s crimson fangs run red with blood.",
					(youagr ? "Your" : s_suffix(Monnam(magr))));
			}
			/* healup by d8 */
			heal(magr, rnd(8));
		}
		if (!youdef && mdef->mpeaceful) {	/* what would be an acceptable condition for the player to be affected? */
			if (vis) {
				pline("%s tarnished fangs %s in the %s.", 
					(youagr ? "Your" : s_suffix(Monnam(magr))),
					(levl[x(magr)][y(magr)].lit ? "shine" : "glimmer"),
					(levl[x(magr)][y(magr)].lit ? "light" : "dark")
					);
				if (youdef) {
					long lcount = (long)rnd(8);
					make_confused(HConfusion + lcount, FALSE);
					make_stunned(HStun + lcount, FALSE);
				}
				else {
					mdef->mconf = 1;
					mdef->mstun = 1;
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_OTIAX:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}
		/* note when the player last made an Otiax attack */
		if (youagr) {
			u.otiaxAttack = moves;
		}

		/* only implemented for uhitm :( */
		if (youagr && !youdef) {
			/* 1/10 chance of stealing items */
			if (!rn2(10)){
				struct obj *otmp2, **minvent_ptr;
				long unwornmask;

				/* Don't steal worn items, and downweight wielded items */
				if ((otmp2 = mdef->minvent) != 0) {
					while (otmp2 &&
						otmp2->owornmask&W_ARMOR &&
						!((otmp2->owornmask&W_WEP) && !rn2(10))
						) otmp2 = otmp2->nobj;
				}
				/* Still has handling for worn items, incase that changes */
				if (otmp2 != 0){
					int dx, dy;
					/* take the object away from the monster */
					if (otmp2->quan > 1L){
						otmp2 = splitobj(otmp2, 1L);
						obj_extract_self(otmp2); //wornmask is cleared by splitobj
					}
					else{
						obj_extract_self(otmp2);
						if ((unwornmask = otmp2->owornmask) != 0L) {
							mdef->misc_worn_check &= ~unwornmask;
							if (otmp2->owornmask & W_WEP) {
								setmnotwielded(mdef, otmp2);
								MON_NOWEP(mdef);
							}
							if (otmp2->owornmask & W_SWAPWEP){
								setmnotwielded(mdef, otmp2);
								MON_NOSWEP(mdef);
							}
							otmp2->owornmask = 0L;
							update_mon_intrinsics(mdef, otmp2, FALSE, FALSE);
						}
					}
					Your("mist tendrils free %s.", doname(otmp2));
					mdrop_obj(mdef, otmp2, FALSE);
					/* more take-away handling, after theft message */
					if (unwornmask & W_WEP) {		/* stole wielded weapon */
						possibly_unwield(mdef, FALSE);
					}
					else if (unwornmask & W_ARMG) {	/* stole worn gloves */
						mselftouch(mdef, (const char *)0, TRUE);
						if (mdef->mhp <= 0)	/* it's now a statue */
							return 1; /* monster is dead */
					}
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_SIMURGH:
		/* print a basic hit message */
		if (vis && dohitmsg) {
			xyhitmsg(magr, mdef, attk);
		}

		/* 4/5 chance of just iron-hating damage */
		if (rn2(5)){
			/* only iron */
			if (hates_iron(pd)) {
				if (vis) {
					pline("%s cold iron quills brush %s.",
						(youagr ? "Your" : s_suffix(Monnam(magr))),
						(youdef ? "you" : mon_nam(mdef))
						);
					result = xdamagey(magr, mdef, attk, d(rnd(5), (mlev(mdef) + 1) / 2), FALSE);
					if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;
				}
			}
		}
		/* 1/5 chance of radiant feathers */
		else {
			/* message */
			if (vis) {
				pline("Radiant feathers slice through %s.",
					(youdef ? "you" : mon_nam(mdef)));
			}
			/* iron */
			if (hates_iron(pd)){
				if (vis) {
					pline("The cold iron rachises sear %s.",
						(youdef ? "you" : mon_nam(mdef)));
				}
				result = xdamagey(magr, mdef, attk, d(5, mlev(mdef)), FALSE);
				if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;
			}
			/* check random "resistances" */
			int i;
			boolean hurts, extrahurts;
			for (i = 0; i < 5; i++){
				hurts = FALSE;
				extrahurts = FALSE;
				switch (rn2(18))
				{
				case 0:		hurts = !Fire_res(mdef); extrahurts = Cold_res(mdef); break;
				case 1:		hurts = !Cold_res(mdef); extrahurts = Fire_res(mdef); break;
				case 2:		hurts = !Disint_res(mdef); break;
				case 3:		hurts = pm_invisible(pd); break;
				case 4:		hurts = !Shock_res(mdef); break;
				case 5:		hurts = !Poison_res(mdef); break;
				case 6:		hurts = !Acid_res(mdef); break;
				case 7:		hurts = !Stone_res(mdef); break;
				case 8:		hurts = !Drain_res(mdef); break;
				case 9:		hurts = !Sick_res(mdef); break;
				case 10:	hurts = is_undead_mon(mdef); break;
				case 11:	hurts = is_fungus(pd); break;
				case 12:	hurts = infravision(pd); break;
				case 13:	hurts = opaque(pd); break;
				case 14:	hurts = !breathless_mon(mdef); break;
				case 15:	hurts = !(insubstantial(pd) || noncorporeal(pd) || (youdef ? Passes_walls : mon_resistance(mdef, PASSES_WALLS))); break;
				case 16:	hurts = is_whirly(pd); break;
				case 17:	hurts = !mindless_mon(mdef); break;
				}
				if (hurts) {
					/* in this case, non-players arbitrarily get a spiritDsize of 5 */
					result = xdamagey(magr, mdef, attk, rnd((extrahurts ? 2 : 1) * (youagr ? spiritDsize() : 5)), FALSE);
					if (result&(MM_DEF_DIED|MM_DEF_LSVD)) return result;
				}
				else {
					if (vis) {
						shieldeff(x(mdef), y(mdef));
					}
				}
			}
			/* stunning radiance */
			if (!resists_blnd(mdef)) {
				if (youdef) {
					make_stunned(HStun + rnd(5), TRUE);
				}
				else {
					mdef->mstun = 1;
				}
			}
		}
		/* make physical attack without hitmsg */
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);
	
//////////////////////////////////////////////////////////////
// THE RIDERS' ATTACKS
//////////////////////////////////////////////////////////////
		/* riders' */
	case AD_DETH:
		/* fancy hitmsg */
		if (vis && dohitmsg) {
			pline("%s %s out with %s deadly touch.",
				(youagr ? "You" : Monnam(magr)),
				(youagr ? "reach" : "reaches"),
				(youagr ? "your" : mhis(magr))
				);
		}
		/* undead are immune to the special effect */
		if (is_undead_mon(mdef) || (youdef && u.sealsActive&SEAL_OSE)) {
			if (youdef) {
				pline("Was that the touch of death?");
			}
			else if (vis) {
				pline("%s looks no deader than before.", Monnam(mdef));
			}
		}
		else {
			/*  3/20 chance of instakill; falls through to lifedrain */
			/* 15/20 chance of lifedrain */
			/*  remainder (5/20) no effect */
			int chance = rn2(20);
			if (chance < 3) {
				/* instakill */
				if (youdef) {
					if (!Antimagic) {
						/* heal Death to full */
						heal(magr, *hpmax(magr));
						/* instakill */
						killer_format = KILLED_BY_AN;
						killer = "touch of death";
						done(DIED);
						return MM_DEF_LSVD;	/* must have lifesaved */
					}
				}
				else {
					if (!resists_magm(mdef) && !resist(mdef, 0, 0, 0)) {
						/* heal Death to full */
						heal(magr, *hpmax(magr));
						/* instakill */
						*hp(mdef) = 0;
						if (youagr)
							killed(mdef);
						else
							monkilled(mdef, "", AD_DETH);

						if (*hp(mdef) > 0)
							return MM_DEF_LSVD; /* mdef lifesaved */
						else
							return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));
					}
				}
			}
			if (chance < 15){
				/* message */
				if (youdef) {
					You_feel("your life force draining away...");
				}
				else if (vis) {
					pline("%s looks weaker!", Monnam(mdef));
				}
				/* aggressor lifesteals by damage dealt */
				heal(magr, min(dmg, *hp(mdef)));
				/* reduce defender's maxhp */
				int hpdrain = rn2(dmg / 2 + 1);
				/* vs player */
				if (youdef) {
					int minhp;
					/*
					* Apply some of the damage to permanent hit points:
					*	polymorphed	    100% against poly'd hpmax
					*	hpmax > 25*lvl	    100% against normal hpmax
					*	hpmax > 10*lvl	50..100%
					*	hpmax >  5*lvl	25..75%
					*	otherwise	 0..50%
					* Never reduces hpmax below 1 hit point per level.
					*/
					if (Upolyd || u.uhpmax > 25 * u.ulevel) hpdrain = dmg;
					else if (u.uhpmax > 10 * u.ulevel) hpdrain += dmg / 2;
					else if (u.uhpmax > 5 * u.ulevel) hpdrain += dmg / 4;

					if (Upolyd) {
						/* [can't use youmonst.m_lev] */
						minhp = min((int)youracedata->mlevel, u.ulevel);
					}
					else {
						minhp = u.ulevel;
					}
					/* reduce maxhp */
					*hpmax(mdef) -= hpdrain;
					if (*hpmax(mdef) < minhp)
						*hpmax(mdef) = minhp;
					flags.botl = 1;
				}
				/* vs monsters */
				else {
					/* check if that would kill */
					if (*hp(mdef) <= hpdrain || *hpmax(mdef) <= hpdrain)
					{
						*hp(mdef) = 0;
						if (youagr)
							killed(mdef);
						else
							monkilled(mdef, "", AD_DETH);
						if (*hp(mdef) > 0)
							return MM_DEF_LSVD; /* mdef lifesaved */
						else
							return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));
					}
					else {
						/* drain the hp */
						*hp(mdef) -= hpdrain;
						*hpmax(mdef) -= hpdrain;
						dmg -= hpdrain;	/* don't double up on the maxhpdrain damage */
					}
				}
			}
			else {
				/* shield effect */
				if (Magic_res(mdef))
					shieldeff(x(mdef), y(mdef));
				/* message */
				if (youdef) {
					pline("Lucky for you, it didn't work!");
				}
				else {
					if (vis)
						pline("That didn't work...");
				}
				/* deal no damage */
				return MM_HIT;
			}
		}
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_PEST:
		/* fancy hitmsg */
		if (vis && dohitmsg) {
			pline("%s %s out, and %s %s.",
				(youagr ? "You" : Monnam(magr)),
				(youagr ? "reach" : "reaches"),
				(youdef ? "you" : mon_nam(mdef)),
				(youdef ? "feel fever and chills" : "looks rather ill")
				);
		}
		/* very very different effect between mhitu and xhitm */
		if (youdef) {
			(void)diseasemu(pa); /* plus the normal damage */
			/* healup based on how sick you are? */
			if (!Sick_res(mdef)) {
				heal(magr, (*hpmax(magr) / Sick));
			}
		}
		else {
			/* arbitrarily reduce defender's hp (by 1/2 if not sickres, by 3/4 if sickres) */
			if (!resist(mdef, 0, 0, NOTELL)) {
				*hpmax(mdef) = min(*hpmax(mdef) * (Sick_res(mdef) ? 2 : 3) / 4, mlev(mdef));
				if (*hp(mdef) > *hpmax(mdef))
					*hp(mdef) = *hpmax(mdef);
			}
			/* healup */
			if (!Sick_res(mdef)) {
				heal(magr, (*hpmax(magr) / 10));
			}
		}
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_FAMN:
		/* fancy hitmsg */
		if (vis && dohitmsg) {
			pline("%s %s out, and %s body shrivels.",
				(youagr ? "You" : Monnam(magr)),
				(youagr ? "reach" : "reaches"),
				(youdef ? "your" : s_suffix(mon_nam(mdef)))
				);
		}
		if (youdef){
			exercise(A_CON, FALSE);
			if (!is_fainted()){
				int hungr = rn1(40, 40);
				morehungry(hungr);
				//heal by the amount of HP it would heal by resting for that nutr worth of turns
				heal(magr, (mlev(magr)*hungr / 30));
			}
		}
		else {
			/* pets can get hungry */
			if (mdef->mtame && !mdef->isminion){
				int hungr = rn1(120, 120);
				EDOG(mdef)->hungrytime -= hungr;
				//heal by the amount of HP it would heal by resting for that nutr worth of turns
				heal(magr, (mlev(magr)*hungr / 30));
			}
			/* other creatures don't have hungrytime */
			else {
				dmg += rnd(10);
				/* if the attack would kill, give a special message and heal Famine */
				if (*hp(mdef) <= dmg)
				{
					/* message */
					if (vis) {
						pline("%s starves.", Monnam(mdef));
					}
					/* fully heal Famine */
					heal(magr, *hpmax(magr));
					/* kill */
					*hp(mdef) = 0;
					if (youagr)
						xkilled(mdef, 0);
					else
						monkilled(mdef, "", AD_FAMN);
					if (*hp(mdef) > 0)
						return MM_DEF_LSVD; /* mdef lifesaved */
					else
						return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));
				}
				else {
					/* aggressor lifesteals by dmg dealt */
					heal(magr, min(dmg, *hp(mdef)));
				}
			}
		}
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);

	case AD_CNFT:
		{
		static boolean in_conflict = FALSE;

		if (in_conflict)
		{
			/* avoid infinite loops; make a basic melee attack */
			alt_attk.adtyp = AD_PHYS;
			return xmeleehurty(magr, mdef, &alt_attk, weapon, dohitmsg, dmg, dieroll, vis, ranged);
		}
		else
		{
			struct monst *tmpm, *nmon;

			in_conflict = TRUE;
			/* fancy hitmsg */
			if (vis && dohitmsg) {
				pline("%s %s out, and conflict surrounds %s.",
					(youagr ? "You" : Monnam(magr)),
					(youagr ? "reach" : "reaches"),
					(youdef ? "you" : mon_nam(mdef))
					);
			}
			/* player-only: abuse CHA */
			if (youdef)
				exercise(A_CHA, FALSE);
			/* all monsters on the level attempt to attack the target */
			for (tmpm = fmon; tmpm; tmpm = nmon){
				nmon = tmpm->nmon;
				if (!DEADMONSTER(tmpm) && tmpm != mdef && tmpm != magr)
					result = xattacky(tmpm, mdef, x(mdef), y(mdef));
				/* possibly return early if def died */
				if (result&(MM_DEF_DIED|MM_DEF_LSVD))
				{
					in_conflict = FALSE;
					return result;
				}
			}
			in_conflict = FALSE;
		}
		}// close scope for in_conflict
		alt_attk.adtyp = AD_PHYS;
		return xmeleehurty(magr, mdef, &alt_attk, weapon, FALSE, dmg, dieroll, vis, ranged);
		break;
//////////////////////////////////////////////////////////////
// NOT IMPLEMENTED FOR XMELEEHURTY
//////////////////////////////////////////////////////////////
		/* retaliatory only, AT_NONE */
	case AD_BARB:
	case AD_HLBD:
	case AD_UNKNWN:
		/* explosion only, AT_EXPL */
	case AD_FNEX:
		/* on-death only, AT_BOOM */
	case AD_SPNL:
	case AD_GARO:
	case AD_GARO_MASTER:
		/* on-death only, AT_NONE */
	case AD_GROW:
	case AD_SOUL:
	case AD_KAOS:
	case AD_MAND:
		/* engulf only, AT_ENGL */
	case AD_DGST:
	case AD_ILUR:
		/* spell only, AT_MAGC or AT_MMGC */
	case AD_CLRC:
	case AD_SPEL:
	case AD_PSON:
		/* gaze only, AT_GAZE or AT_WDGZ */
	case AD_LUCK:
	case AD_SSUN:
	case AD_SPOR:
	case AD_BLNK:
	case AD_WTCH:
	case AD_RGAZ:
	case AD_MIST:
	case AD_DEAD:
	case AD_CNCL:
		/* breath only, AT_BREA */
	case AD_GOLD:
		/* tinker only, AT_TNKR */
	case AD_TNKR:
		/* AT_ARRW specifiers */
	case AD_SLVR:
	case AD_BALL:
	case AD_BLDR:
	case AD_VBLD:
	case AD_SURY:
	case AD_LOAD:
	case AD_SOLR:
		/* missiles (completely unused) */
	case AD_CMSL:
	case AD_FMSL:
	case AD_EMSL:
	case AD_SMSL:
		/* other deprecated or unusued damage types */
	case AD_NTZC:
	case AD_LSEX:
	case AD_BIST:
	case AD_DUNSTAN:
	case AD_WMTG:
	case AD_JAILER:
		impossible("Attack type not coded for xmeleehurty (%d)!", attk->adtyp);
		break;
	default:
		impossible("Bad attack type (%d)!", attk->adtyp);
	}
	impossible("Reached end of xmeleehurty -- report this to Nero on ##nethack-variants.");
	return MM_MISS;
}

int
xcasty(magr, mdef, attk, vis)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
int vis;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	struct permonst * pd = youdef ? youracedata : mdef->data;
	boolean on_line;
	boolean can_target;
	boolean foundyou;
	int newres;
	int result = MM_MISS;
	int distance;
	int range;
	int tarx = (youdef ? magr->mux : x(mdef));
	int tary = (youdef ? magr->muy : y(mdef));
	int dx = sgn(tarx - x(magr));
	int dy = sgn(tary - y(magr));

	/* cancelled monsters can't cast spells -- return early */
	if (!youagr && magr->mcan) {
		cursetxt(magr, FALSE);
		return MM_MISS;
	}

	/* find distance between magr and mdef */
	distance = dist2(
		x(magr),
		y(magr),
		(youdef ? magr->mux : x(mdef)),
		(youdef ? magr->muy : y(mdef))
		);

	/* determine spellcasting range of attacker */
	if (is_orc(pa) ||
		pa == &mons[PM_HEDROW_WARRIOR] ||
		pa == &mons[PM_MINOTAUR_PRIESTESS]
		)
		range = (BOLT_LIM*BOLT_LIM) / 2;
	else if (pa == &mons[PM_CHROMATIC_DRAGON] || pa == &mons[PM_PLATINUM_DRAGON])
		range = 2;
	else
		range = (BOLT_LIM*BOLT_LIM);

	/* determine if magr can target mdef */
	if ((youdef ? couldsee(x(magr), y(magr)) : clear_path(x(magr), y(magr), x(mdef), y(mdef))) &&	/* clear line of sight */
		((youdef||youagr) ? TRUE : mon_can_see_mon(magr, mdef)) &&									/* can see def (ignored vs player?) */
		dist2(x(magr), y(magr), x(mdef), y(mdef)) <= range)											/* within range */
		can_target = TRUE;
	else
		can_target = FALSE;

	/* determine if magr and mdef are lined up (or magr thinks they are) */
	if (x(magr) == tarx ||
		y(magr) == tary	)
		on_line = TRUE;
	else
		on_line = FALSE;
	/* determine if monster is actually aiming at player, if youdef */
	if (youdef && tarx == u.ux && tary == u.uy)
		foundyou = TRUE;
	else
		foundyou = FALSE;

	/* Non-standard (and non-star???) spell types */
	if (attk->adtyp != AD_SPEL &&
		attk->adtyp != AD_CLRC &&
		attk->adtyp != AD_PSON &&
		attk->adtyp != AD_STAR)
	{
		/* needs a target */
		if (can_target) {
			if (distance <= 2) {
				/* melee magic */
				/* currently part of castum (and variants) */
				if (youdef)
					newres = castmu(magr, attk, TRUE, foundyou);
				else if (youagr)
					newres = castum(mdef, attk);
				else
					newres = castmm(magr, mdef, attk);
				/* confirm result */
				switch (newres)
				{
				case 0:	/* unsuccessful spell */
					result = MM_MISS;
					break;
				case 1:	/* successful spell */
					result = MM_HIT;
					break;
				case 2:	/* (m) target died */
					result = MM_DEF_DIED;
					break;
				}
			}
			else {
				/* ranged magic */
				/* requires being on a line */
				if (on_line) {
					/* ray spell */
					int dmn;
					int type = attk->adtyp;
					/* get number of damage dice */
					dmn = (min(MAX_BONUS_DICE, mlev(magr) / 3 + 1));
					if (attk->damn)
						dmn += attk->damn;
					if (dmn < 1)
						dmn = 1;
					/* swap damage type as appropriate */
					if (type == AD_RBRE || type == AD_RETR)
					{
						/* random between fire/cold/elec damage */
						switch (rn2(3))
						{
						case 0: type = AD_FIRE; break;
						case 1: type = AD_COLD; break;
						case 2: type = AD_ELEC; break;
						}
					}
					else if (type == AD_OONA) {
						/* Oona */
						type = u.oonaenergy;
					}
					/* message */
					if (youdef) {
						/* message */
						if (vis&VIS_MAGR) {
							pline("%s zaps you with a %s!", Monnam(magr),
								flash_type(type, SPBOOK_CLASS));
						}
						else {
							You("are zapped with a %s!",
								flash_type(type, SPBOOK_CLASS));
						}
					}
					else {
						if (vis&VIS_MAGR) {
							pline("%s %s %s with a %s!",
								(youagr ? "You" : Monnam(magr)),
								(youagr ? "zap" : "zaps"),
								mon_nam(mdef),
								flash_type(type, SPBOOK_CLASS)
								);
						}
					}
					/* interrupt player if zap will hit */
					if (youdef && foundyou)
						nomul(0, NULL);
					/* do the zap */
					/* FIXME: this prints a message even if you can't see magr or mdef */
					buzz(type, SPBOOK_CLASS, FALSE, dmn,
						x(magr), y(magr), dx, dy, 0, 0);
					/* figure out who died */
					/* unfortunately, cannot tell if either lifesaved */
					if (*hp(mdef) < 0)
						result |= MM_DEF_DIED;
					if (*hp(magr) < 0)
						result |= MM_AGR_DIED;

				}
				/* if not lined up, no casting */
			}
		}
		/* needs a target */
	}
	/* standard spell damage types: AD_MAGC, AD_CLRC, AD_PSON, and for some reason, AD_STAR */
	else {
		/* farm out to old existing spellcasting functions */
		if (youdef) {
			/* can cast without actually having you in range */
			newres = castmu(magr, attk, can_target, (foundyou&&can_target));
		}
		else if (youagr) {
			/* you need to be in range */
			if (can_target)
				newres = castum(mdef, attk);
			else
				newres = 0;
		}
		else {
			/* monsters need to be in range of each other */
			if (can_target)
				newres = castmm(magr, mdef, attk);
			else
				newres = 0;
		}
		/* confirm result */
		switch (newres)
		{
		case 0:	/* unsuccessful spell */
			result = MM_MISS;
			break;
		case 1:	/* successful spell */
			result = MM_HIT;
			break;
		case 2:	/* (m) target died */
			result = MM_DEF_DIED;
			break;
		}
	}

	return result;
}


/* tinker, mvu only right now */
int
xtinkery(magr, mdef, attk, vis)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
int vis;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	struct permonst * pd = youdef ? youracedata : mdef->data;
	int result = MM_MISS;
	
	/* mvm behaviour is undefined right now -- do nothing */
	if (!youagr && !youdef)
		return MM_MISS;

	int tarx = (youdef ? magr->mux : x(mdef));
	int tary = (youdef ? magr->muy : y(mdef));

	/* only mvu behaviour is implemented */
	if (youdef) {
		/* tinkering requires mspec available */
		if (!magr->mspec_used) {
			struct monst *mlocal;
			int dx = 0, dy = 0, i;
			int monid;

			/* get dx, dy */
			if		(tarx - x(magr) < 0) dx = -1;
			else if (tarx - x(magr) > 0) dx = +1;
			if		(tary - y(magr) < 0) dy = -1;
			else if (tary - y(magr) > 0) dy = +1;

			/* get monid */
			if (magr->data == &mons[PM_HOOLOOVOO]){
				if (rn2(4))
					monid = PM_GOLDEN_HEART;
				else
					monid = PM_ID_JUGGERNAUT;
			}
			else {
				if (mlev(mdef) > 10 && !rn2(10))
					monid = PM_FIREWORK_CART;
				else
					monid = PM_CLOCKWORK_SOLDIER + rn2(3);
			}
			/* make the monster */
			mlocal = makemon(&mons[monid],
				x(magr) + dx,
				y(magr) + dy,
				(MM_ADJACENTOK|MM_ADJACENTSTRICT|MM_NOCOUNTBIRTH));

			/* set the creation's direction */
			if (mlocal){
				for (i = 0; i<8; i++) if (xdir[i] == dx && ydir[i] == dy) break;
				mlocal->mvar1 = i;

				magr->mspec_used = rnd(6);
				result = MM_HIT;
			}
			else
				result = MM_MISS;
		}
		else
			result = MM_MISS;
		magr->mflee = 1;
	}
	return result;
}

int
xengulfhity(magr, mdef, attk, vis)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
int vis;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	struct permonst * pd = youdef ? youracedata : mdef->data;
	int result = MM_MISS;
	struct trap *trap = t_at(x(mdef), y(mdef));
	struct obj * otmp;
	int dmg = d((int)attk->damn, (int)attk->damd);

	if (youdef) {
		/* swallows you */
		if (!u.uswallow) {
			if ((trap && ((trap->ttyp == PIT) || (trap->ttyp == SPIKED_PIT))) &&
				boulder_at(u.ux, u.uy))
				return MM_MISS;

			/* remove ball and chain */
			if (Punished)
				unplacebc();
			/* move attacker */
			remove_monster(x(magr), y(magr));
			magr->mtrapped = 0;		/* no longer on old trap */
			place_monster(magr, u.ux, u.uy);
			newsym(x(magr), y(magr));
			u.ustuck = magr;
#ifdef STEED
			if (is_animal(pa) && u.usteed) {
				/* Too many quirks presently if hero and steed
				* are swallowed. Pretend purple worms don't
				* like horses for now :-)
				*/
				pline("%s lunges forward and plucks you off %s!",
					Monnam(magr), mon_nam(u.usteed));
				dismount_steed(DISMOUNT_ENGULFED);
			}
			else
#endif
				pline("%s engulfs you!", Monnam(magr));
			stop_occupation();
			reset_occupations();	/* behave as if you had moved */

			if (u.utrap) {
				You("are released from the %s!",
					u.utraptype == TT_WEB ? "web" : "trap");
				u.utrap = 0;
			}

			int i = number_leashed();
			if (i > 0) {
				const char *s = (i > 1) ? "leashes" : "leash";
				pline_The("%s %s loose.", s, vtense(s, "snap"));
				unleash_all();
			}

			if (touch_petrifies(youracedata) && !resists_ston(magr)) {
				minstapetrify(magr, TRUE);
				if (magr->mhp > 0)
					return MM_AGR_STOP;
				else
					return MM_AGR_DIED;
			}

			display_nhwindow(WIN_MESSAGE, FALSE);
			vision_recalc(2);	/* hero can't see anything */
			u.uswallow = 1;

			/* for digestion, shorter time is more dangerous;
			for other swallowings, longer time means more
			chances for the swallower to attack */
			int tim_tmp;
			if (attk->adtyp == AD_DGST) {
				tim_tmp = 25 - mlev(magr);
				if (tim_tmp > 0)
					tim_tmp = rnd(tim_tmp) / 2;
				else if (tim_tmp < 0)
					tim_tmp = -(rnd(-tim_tmp) / 2);
				/* having good armor & high constitution makes
				it take longer for you to be digested, but
				you'll end up trapped inside for longer too */
				tim_tmp += -u.uac + 10 + (ACURR(A_CON) / 3 - 1);
			}
			else {
				/* higher level attacker takes longer to eject hero */
				tim_tmp = rnd((int)mlev(magr) + 10 / 2);
			}
			/* u.uswldtim always set > 1 */
			u.uswldtim = (unsigned)((tim_tmp < 2) ? 2 : tim_tmp);
			swallowed(1);

			/* snuff player's carried lightsources */
			for (otmp = invent; otmp; otmp = otmp->nobj) {
				if (!is_whirly(u.ustuck->data)
					|| (otmp->otyp != TORCH && otmp->otyp != SHADOWLANDER_S_TORCH && otmp->otyp != SUNROD))
					(void)snuff_lit(otmp);
			}
		}
		/* player should be swallowed now */
		if (magr != u.ustuck)
			return MM_MISS;
		/* decrement swallow time remaining */
		if (u.uswldtim > 0)
			u.uswldtim -= 1;
		/* deal damage and other effects */
		result = xengulfhurty(magr, mdef, attk, vis);

		/* after engulfing, possibly regurgitate you */
		if (touch_petrifies(youracedata) && !resists_ston(magr)) {
			pline("%s very hurriedly %s you!", Monnam(magr),
				is_animal(pa) ? "regurgitates" : "expels");
			expels(magr, pa, FALSE);
		}
		else if (!u.uswldtim || youracedata->msize >= MZ_HUGE) {
			You("get %s!", is_animal(pa) ? "regurgitated" : "expelled");
			if (flags.verbose && (is_animal(pa) ||
				(dmgtype(pa, AD_DGST) && Slow_digestion)))
				pline("Obviously %s doesn't like your taste.", mon_nam(magr));
			expels(magr, pa, FALSE);
		}
	}
	else {
		/* message */
		if (youagr) {
			You("%s %s!",
				(is_animal(pa) ? "swallow" : "engulf"),
				mon_nam(mdef));
		}
		else if (vis) {
			pline("%s %s %s.",
				Monnam(magr),
				(is_animal(pa) ? "swallows" : "engulfs"),
				mon_nam(mdef));
		}
		/* snuff mdef's carried lightsources */
		for (otmp = mdef->minvent; otmp; otmp = otmp->nobj) {
			if (!is_whirly(pa)
				|| (otmp->otyp != TORCH && otmp->otyp != SHADOWLANDER_S_TORCH && otmp->otyp != SUNROD))
				(void)snuff_lit(otmp);
		}
		/* visually move the agressor over defender */
		if (youagr ? (!Invisible) : canspotmon(magr)) {
			map_location(x(magr), y(magr), TRUE);
			/* SCOPECREEP: get the correct glyph for pets/peacefuls/zombies/detected/etc */
			tmp_at(DISP_FLASH, mon_to_glyph(magr));
			tmp_at(x(mdef), y(mdef));
			delay_output();
			delay_output();
		}
		/* deal damage and other effects */
		result = xengulfhurty(magr, mdef, attk, vis);

		/* if defender died, move agressor to defender's coord */
		if (result&MM_DEF_DIED) {
			/* sanity check */
			if (*hp(mdef) > 0)
				impossible("dead engulfee still alive?");

			if (youagr) {
				remove_monster(x(mdef), y(mdef));
				teleds(x(mdef), y(mdef), FALSE);
			}
			else {
				remove_monster(x(magr), y(magr));
				remove_monster(x(mdef), y(mdef));
				place_monster(magr, x(mdef), y(mdef));
			}
		}
		/* remap the agressor's old location */
		if (youagr ? (!Invisible) : canspotmon(magr)) {
			tmp_at(DISP_END, 0);
			newsym(x(magr), y(magr));
		}
	}

	return result;
}

int
xengulfhurty(magr, mdef, attk, vis)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
int vis;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	struct permonst * pd = youdef ? youracedata : mdef->data;
	int result = MM_MISS;
	int dmg = d(attk->damn, attk->damd);
	int fulldmg = dmg;
	char buf[BUFSZ];
	struct obj * otmp;

	/* apply half-phys */
	if (Half_phys(mdef))
		dmg /= 2;

	switch (attk->adtyp)
	{
		/* the most important and most special case */
	case AD_DGST:
		if (youdef) {
			if (pa == &mons[PM_METROID_QUEEN] && !Drain_resistance) {
				losexp("life force drain", TRUE, FALSE, FALSE);
				magr->mhpmax += d(1, 4);
				magr->mhp += d(1, 6);
				if (magr->mhp > magr->mhpmax) magr->mhp = magr->mhpmax;
				if (magr->mtame){
					EDOG(magr)->hungrytime += 100;  //400/4 = human nut/4
				}
			}
			if (Slow_digestion) {
				/* Messages are handled in caller */
				u.uswldtim = 0;
				result = MM_HIT;
			}
			else if (u.uswldtim == 0) {
				pline("%s totally digests you!", Monnam(magr));
				result = xdamagey(magr, mdef, attk, FATAL_DAMAGE_MODIFIER, FALSE);
			}
			else {
				pline("%s%s digests you!", Monnam(magr),
					(u.uswldtim == 2) ? " thoroughly" :
					(u.uswldtim == 1) ? " utterly" : "");
				exercise(A_STR, FALSE);
				result = MM_HIT;
			}
		}
		else if (youagr) {
			/* eating a Rider or its corpse is fatal */
			if (is_rider(mdef->data)) {
				pline("Unfortunately, digesting any of it is fatal.");
				Sprintf(buf, "unwisely tried to eat %s",
					mdef->data->mname);
				killer = buf;
				killer_format = NO_KILLER_PREFIX;
				done(DIED);
				result = MM_AGR_STOP;		/* lifesaved */
			}
			else {
				if (Slow_digestion) {
					dmg = 0;
					break;
				}

				/* KMH, conduct */
				u.uconduct.food++;
				if (!vegan(pd))
					u.uconduct.unvegan++;
				if (!vegetarian(pd))
					violated_vegetarian();

				/* Use up amulet of life saving */
				if (!!(otmp = mlifesaver(mdef))) m_useup(mdef, otmp);

				newuhs(FALSE);
				xkilled(mdef, 2);
				if (*hp(mdef) > 0) { /* monster lifesaved */
					You("hurriedly regurgitate the sizzling in your %s.",
						body_part(STOMACH));
					result |= MM_DEF_LSVD;
				}
				else {
					result |= MM_DEF_DIED;
					int digest_time = 1 + (pd->cwt >> 8);
					if (corpse_chance(mdef, &youmonst, TRUE) &&
						!(mvitals[monsndx(pd)].mvflags &
						G_NOCORPSE || mdef->mvanishes)) {
						/* nutrition only if there can be a corpse */
						if (Race_if(PM_INCANTIFIER)) u.uen += mlev(mdef);
						else u.uhunger += (pd->cnutrit + 1) / 2;
					}
					else digest_time = 0;
					Sprintf(buf, "You totally digest %s.",
						mon_nam(mdef));
					if (digest_time != 0) {
						/* setting afternmv = end_engulf is tempting,
						* but will cause problems if the player is
						* attacked (which uses his real location) or
						* if his See_invisible wears off
						*/
						You("digest %s.", mon_nam(mdef));
						if (Slow_digestion) digest_time *= 2;
						nomul(-digest_time, "digesting a victim");
						nomovemsg = buf;
					}
					else pline("%s", buf);
					if (pd == &mons[PM_GREEN_SLIME] || pd == &mons[PM_FLUX_SLIME]) {
						Sprintf(buf, "%s isn't sitting well with you.",
							The(pd->mname));
						if (!Unchanging) {
							Slimed = 5L;
							flags.botl = 1;
						}
					}
					else
						exercise(A_CON, TRUE);
				}
			}
		}
		else {
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
				if (magr->mhp > 0) {
					result = MM_AGR_STOP;	/* lifesaved */
				}
				else {
					if (magr->mtame && !vis)
						You("have a queasy feeling for a moment, then it passes.");
					result = MM_AGR_DIED;
				}
			}
			else {
				if (flags.verbose && flags.soundok)
					verbalize("Burrrrp!");

				/* Use up amulet of life saving - no lifesaving allowed? */
				if (!!(otmp = mlifesaver(mdef))) m_useup(mdef, otmp);

				/* kill */
				result = xdamagey(magr, mdef, attk, FATAL_DAMAGE_MODIFIER, FALSE);

				/* if they survivied, by some miracle, end */
				if (result&MM_DEF_LSVD) {
					break;
				}

				/* Is a corpse for nutrition possible?  It may kill magr */
				if (!corpse_chance(mdef, magr, TRUE)) {
					break;
				}
				if (*hp(magr) < 1) {
					result |= MM_AGR_DIED;
					break;
				}
				/* Pets get nutrition from swallowing monster whole.
				* No nutrition from G_NOCORPSE monster, eg, undead.
				* DGST monsters don't die from undead corpses
				*/
				int num = monsndx(mdef->data);
				if (magr->mtame && !magr->isminion &&
					!(mvitals[num].mvflags & G_NOCORPSE || mdef->mvanishes)) {
					struct obj *virtualcorpse = mksobj(CORPSE, FALSE, FALSE);
					int nutrit;

					virtualcorpse->corpsenm = num;
					virtualcorpse->owt = weight(virtualcorpse);
					nutrit = dog_nutrition(magr, virtualcorpse);
					dealloc_obj(virtualcorpse);

					/* only 50% nutrition, 25% of normal eating time */
					if (magr->meating > 1) magr->meating = (magr->meating + 3) / 4;
					if (nutrit > 1) nutrit /= 2;
					EDOG(magr)->hungrytime += nutrit;
				}
			}
		}
		break;

	case AD_WET:
		/* print hitmessage */
		if (youdef)
			You("are soaking wet!");
		else if (vis&VIS_MDEF)
			pline("%s is soaking wet!", Monnam(mdef));
		/* water/lethe damage */
		water_damage((youdef ? invent : mdef->minvent), FALSE, FALSE, FALSE, mdef);
		result = MM_HIT;
		break;
	case AD_LETHE:
		/* very asymetric effects */
		if (youdef) {
			pline("The waters of the Lethe wash over you!");
			if (u.sealsActive&SEAL_HUGINN_MUNINN){
				unbind(SEAL_HUGINN_MUNINN, TRUE);
			}
			else {
				(void)adjattrib(A_INT, -dmg, FALSE);
				forget(25);
				water_damage(invent, FALSE, FALSE, TRUE, &youmonst);

				exercise(A_WIS, FALSE);
			}
			if (ABASE(A_INT) <= 3) {
				int lifesaved = 0;
				struct obj *wore_amulet = uamul;

				while (1) {
					/* avoid looping on "die(y/n)?" */
					if (lifesaved && (discover || wizard)) {
						if (wore_amulet && !uamul) {
							/* used up AMULET_OF_LIFE_SAVING; still
							subject to dying from brainlessness */
							wore_amulet = 0;
						}
						else {
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
					killer = "memory loss";
					killer_format = KILLED_BY;
					done(DIED);
					lifesaved++;
					result |= MM_DEF_LSVD;
				}
			}
		}
		else {
			if (vis&VIS_MDEF)
				pline("%s is soaking wet!", Monnam(mdef));
			water_damage(mdef->minvent, FALSE, FALSE, TRUE, mdef);
		}
		result |= MM_HIT;
		break;
	case AD_BLND:
		if (can_blnd(magr, mdef, attk->aatyp, (struct obj*)0)) {
			if (youdef) {
				if (!Blind) {
					You_cant("see in here!");
					make_blinded((long)dmg, FALSE);
					if (!Blind)
						Your1(vision_clears);
				}
				else
					/* keep player blind until disgorged */
					make_blinded(Blinded + 1, FALSE);
			}
			else {
				if (vis&VIS_MDEF && mdef->mcansee)
					pline("%s can't see in there!", Monnam(mdef));
				mdef->mcansee = 0;
				mdef->mblinded = min(127, dmg + mdef->mblinded);
			}
		}
		result = MM_HIT;
		break;
	case AD_STDY:
		if (youdef) {
			if (dmg > u.ustdy) {
				You("are studied from all directions!");
				u.ustdy = dmg;
			}
		}
		else {
			/* no message? */
			if (dmg > mdef->mstdy)
				mdef->mstdy = dmg;
		}
		break;
		/* Illurien's unique engulf attack */
	case AD_ILUR:
		/* undo half-phys-damage; this is quite magical */
		dmg = fulldmg;
		
		if (has_blood(pd) || !is_anhydrous(pd)) {
			dmg *= 2;
			pline("A mist of %s from %s %s and swept into the cloud!",
				(has_blood(pd) ? "blood is torn" : "water is drawn"),
				(youdef ? "your" : s_suffix(mon_nam(mdef))),
				(youdef ? body_part(BODY_SKIN) : mbodypart(mdef, BODY_SKIN))
				);
			if (youdef) {
				if (u.sealsActive&SEAL_HUGINN_MUNINN){
					unbind(SEAL_HUGINN_MUNINN, TRUE);
				}
				else {
					int perc = (*hpmax(mdef) - (*hp(mdef) - dmg)) * 10 / (*hpmax(mdef));
					if (perc > 10) perc = 10;
					if (perc > 0) forget(perc);
				}
			}
			result = xdamagey(magr, mdef, attk, dmg, FALSE);
		}
		else {
			if (pd == &mons[PM_IRON_GOLEM] || pd == &mons[PM_CHAIN_GOLEM]) {
				if (youdef) {
					You("are laden with moisture and rust away!");
					/* KMH -- this is okay with unchanging */
					rehumanize();
					result = MM_DEF_LSVD;
					magr->mhp = 1;
					expels(magr, pa, FALSE);
					pline("Rusting your iron body took a severe toll on the cloud!");
				}
				else {
					if (vis&VIS_MDEF) {
						pline("%s falls to pieces!",
							Monnam(mdef));
					}
					if (youagr)
						killed(mdef);
					else
						mondied(mdef);

					if (mdef->mhp > 0)
						result = MM_DEF_LSVD;
					else {
						if (mdef->mtame && !vis && !youagr)
							pline("May %s rust in peace.", mon_nam(mdef));
						result = (MM_HIT | MM_DEF_DIED | (youagr || grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
					}
				}
			}
			else {
				if (youdef) {
					You("are laden with moisture and %s",
						flaming(youracedata) ? "are smoldering out!" :
						Breathless ? "find it mildly uncomfortable." :
						amphibious(youracedata) ? "feel comforted." :
						"can barely breathe!");
					/* NB: Amphibious includes Breathless */
					if (Amphibious && !flaming(youracedata)) dmg = 0;
					else exercise(A_CON, FALSE);
				}
				else {
					if (vis&VIS_MDEF) {
						pline("%s is laden with moisture.",
							Monnam(mdef));
					}
					if (amphibious_mon(mdef) && !flaming(pd)) {
						dmg = 0;
						if (vis&VIS_MDEF)
							pline("%s seems unharmed.", Monnam(mdef));
					}
				}
				result = xdamagey(magr, mdef, attk, dmg, FALSE);
			}
		}
		/* rust armor */
		if (youdef) {
			hurtarmor(AD_RUST);
		}
		else {
			hurtmarmor(mdef, AD_RUST);
		}
		break;
		/* basic damage engulf types */
	case AD_PHYS:
		if (pa == &mons[PM_FOG_CLOUD]) {
			if (youdef) {
				You("are laden with moisture and %s",
					flaming(youracedata) ? "are smoldering out!" :
					Breathless ? "find it mildly uncomfortable." :
					amphibious(youracedata) ? "feel comforted." :
					"can barely breathe!");
				/* NB: Amphibious includes Breathless */
				if (Amphibious && !flaming(youracedata)) dmg = 0;
				else exercise(A_CON, FALSE);
			}
			else {
				if (vis&VIS_MDEF) {
					pline("%s is laden with moisture.",
						Monnam(mdef));
				}
				if (amphibious_mon(mdef) &&	!flaming(pd)) {
					dmg = 0;
					if (vis&VIS_MDEF)
						pline("%s seems unharmed.", Monnam(mdef));
				}
			}
		}
		else if (pa == &mons[PM_DREADBLOSSOM_SWARM]) {
			if (youdef) {
				You("are sliced by the whirling stems!");
				exercise(A_DEX, FALSE);
			}
			else if (vis&VIS_MDEF) {
				pline("%s is sliced by whirling stems!",
					Monnam(mdef));
			}
		}
		else {
			if (youdef) {
				You("are pummeled with debris!");
				exercise(A_STR, FALSE);
			}
			else if (vis&VIS_MDEF) {
				pline("%s is pummeled with debris!",
					Monnam(mdef));
			}
		}
		/* deal damage (which can be 0 gracefully) */
		result = xdamagey(magr, mdef, attk, dmg, FALSE);
		break;
	case AD_ACID:
	case AD_EACD:
		/* apply resistance */
		if (Acid_res(mdef)) {
			if (attk->adtyp == AD_EACD)
				dmg /= 2;
			else
				dmg = 0;
		}
		/* message, do special effects */
		if (dmg) {
			if (youdef) {
				if (Hallucination) pline("Ouch!  You've been slimed!");
				else You("are covered in acid! It burns!");
				exercise(A_STR, FALSE);
			}
			else if (vis&VIS_MDEF) {
				pline("%s is covered in acid!",
					Monnam(mdef));
			}
			if(!rn2(3))
				erode_armor(mdef, TRUE);
		}
		else {
			if (youdef) {
				You("are covered in seemingly harmless goo.");
			}
			else if (vis&VIS_MDEF) {
				pline("%s is covered in goo.",
					Monnam(mdef));
			}
		}
		/* deal damage */
		result = xdamagey(magr, mdef, attk, dmg, FALSE);
		break;
	case AD_ELEC:
	case AD_EELC:
		/* apply resistance */
		if (Shock_res(mdef)) {
			if (attk->adtyp == AD_EELC)
				dmg /= 2;
			else
				dmg = 0;
		}
		/* message, special effects */
		if (((attk->adtyp == AD_EELC) || (youagr || !magr->mcan)) &&
			(rn2(2) || !youdef || !u.uswallow))
		{
			/* message */
			if (vis&VIS_MDEF) {
				pline_The("air around %s crackles with electricity.",
					(youdef ? "you" : mon_nam(mdef))
					);
				if (!dmg) {
					pline("%s %s unhurt.",
						(youdef ? "You" : Monnam(mdef)),
						(youdef ? "seem" : "seems")
						);
				}
			}
			/* destroy items */
			if (!InvShock_res(mdef)){
				if (mlev(magr) > rn2(20))
					destroy_item2(mdef, WAND_CLASS, AD_ELEC, youdef);
				if (mlev(magr) > rn2(20))
					destroy_item2(mdef, RING_CLASS, AD_ELEC, youdef);
			}
			/* golem effects */
			if (youdef)
				ugolemeffects(AD_ELEC, fulldmg);
			else
				golemeffects(mdef, AD_ELEC, fulldmg);
		}
		else
			dmg = 0;
		/* deal damage */
		result = xdamagey(magr, mdef, attk, dmg, FALSE);
		break;
	case AD_COLD:
	case AD_ECLD:
		/* apply resistance */
		if (Cold_res(mdef)) {
			if (attk->adtyp == AD_ECLD)
				dmg /= 2;
			else
				dmg = 0;
		}
		/* message, special effects */
		if (((attk->adtyp == AD_ECLD) || (youagr || !magr->mcan)) &&
			(rn2(2) || !youdef || !u.uswallow))
		{
			/* message */
			if (vis&VIS_MDEF) {
				if (dmg) {
					pline("%s %s freezing to death!",
						(youdef ? "You" : Monnam(mdef)),
						(youdef ? "are" : "is")
						);
				}
				else {
					pline("%s %s mildly chilly.",
						(youdef ? "You" : Monnam(mdef)),
						(youdef ? "feel" : "seems")
						);
				}
			}
			/* destroy items */
			if (!InvCold_res(mdef)){
				if (mlev(magr) > rn2(20))
					destroy_item2(mdef, POTION_CLASS, AD_COLD, youdef);
			}
			/* golem effects */
			if (youdef)
				ugolemeffects(AD_COLD, fulldmg);
			else
				golemeffects(mdef, AD_COLD, fulldmg);
		}
		else
			dmg = 0;
		/* deal damage */
		result = xdamagey(magr, mdef, attk, dmg, FALSE);
		break;
	case AD_FIRE:
	case AD_EFIR:
	case AD_ACFR:
		{ boolean holysear = FALSE, firedmg = TRUE;
		/* apply resistance/vulnerability */
		if (attk->adtyp == AD_ACFR) {
			if (youdef ? (hates_holy(pd)) : (hates_holy_mon(mdef)))
			{
				dmg *= 2;
				holysear = TRUE;
			}
		}
		if (Fire_res(mdef)) {
			if (attk->adtyp == AD_EFIR)
				dmg /= 2;
			else if (attk->adtyp == AD_ACFR && holysear) {
				firedmg = FALSE;
				dmg /= 2;
			}
			else {
				firedmg = FALSE;
				dmg = 0;
			}
		}
		/* message, special effects */
		if (((attk->adtyp == AD_EFIR || attk->adtyp == AD_ACFR) || (youagr || !magr->mcan)) &&
			(rn2(2) || !youdef || !u.uswallow))
		{
			/* message */
			if (vis&VIS_MDEF) {
				if (dmg) {
					if (firedmg) {
						pline("%s %s burning to a crisp!",
							(youdef ? "You" : Monnam(mdef)),
							(youdef ? "are" : "is")
							);
					}
					if (holysear) {
						pline("%s %s seared by the holy flames!",
							(youdef ? "You" : Monnam(mdef)),
							(youdef ? "are" : "is")
							);
					}
				}
				else {
					pline("%s %s mildly hot.",
						(youdef ? "You" : Monnam(mdef)),
						(youdef ? "feel" : "seems")
						);
				}
			}
			/* destroy items */
			if (!InvFire_res(mdef)) {
				if (mlev(magr) > rn2(20))
					destroy_item2(mdef, SCROLL_CLASS, AD_FIRE, youdef);
				if (mlev(magr) > rn2(20))
					destroy_item2(mdef, POTION_CLASS, AD_FIRE, youdef);
				if (mlev(magr) > rn2(25))
					destroy_item2(mdef, SPBOOK_CLASS, AD_FIRE, youdef);
			}
			/* golem effects */
			if (youdef)
				ugolemeffects(AD_FIRE, fulldmg);
			else
				golemeffects(mdef, AD_FIRE, fulldmg);
			/* burn away slime */
			if (youdef)
				burn_away_slime();
		}
		else
			dmg = 0;
		/* deal damage */
		result = xdamagey(magr, mdef, attk, dmg, FALSE);
		}
		break;
	case AD_DESC:
		/* apply resistance/vulnerability */
		if (nonliving(pd) || is_anhydrous(pd)){
			shieldeff(x(mdef), y(mdef));
			dmg = 0;
		}
		if (is_watery(pd))
			dmg *= 2;
		/* message, special effects */
		if (dmg > 0) {
			/* message */
			if (vis&VIS_MDEF) {
				pline("%s %s dehydrated!",
					(youdef ? "You" : Monnam(mdef)),
					(youdef ? "are" : "is")
					);
			}
			/* heal from damage dealt*/
			heal(mdef, min(*hp(mdef), dmg));
		}
		/* deal damage */
		result = xdamagey(magr, mdef, attk, dmg, FALSE);
		break;
	case AD_DRST:
		/* unbreathing provides total immunity */
		if (youdef ? Breathless : breathless_mon(mdef))
			break;
		/* apply resistance */
		if (Poison_res(mdef)) {
			dmg = 0;
		}
		/* message */
		if (dmg) {
			if (youdef) {
				pline("%s is burning your %s!", Something, makeplural(body_part(LUNG)));
				You("cough and spit blood!");
			}
			else if (vis&VIS_MDEF) {
				pline("%s coughs!",
					Monnam(mdef));
			}
		}
		else {
			if (youdef) {
				You("cough!");
			}
		}
		/* blind defender (even if poison resistant) */
		if (youdef) {
			if (!Blind)
				make_blinded(1L, FALSE);
		}
		else {
			mdef->mcansee = 0;
			if (!mdef->mblinded)
				mdef->mblinded = 1;
		}
		/* deal damage */
		result = xdamagey(magr, mdef, attk, dmg, FALSE);
		break;
	case AD_DISE:	/* damage/effect ? */
		break;
	}
	return result;
}

int
xexplodey(magr, mdef, attk, vis)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
int vis;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	struct permonst * pd = youdef ? youracedata : mdef->data;
	int result = MM_MISS;

	int dmg = d((int)attk->damn, (int)attk->damd);
	int fulldmg = dmg;			/* original unreduced damage */

	/* message */
	if (vis)
		xyhitmsg(magr, mdef, attk);
	else
		noises(magr, attk);

	/* directed at player, wild miss */
	if (youdef && (magr->mux != u.ux || magr->muy != u.uy)) {
		/* give message regardless of vis -- it was targeting the player */
		pline("%s explodes at a spot in %s!",
			Monnam(magr),
			(((int)levl[magr->mux][magr->muy].typ) == WATER ? "empty water" : "thin air")
			);
		/* skip most effects. Go to killing magr */
	}
	else {
		switch (attk->adtyp)
		{
		case AD_FNEX:
			/* fern spores are extra special */
			/* need to die before their explosion, so that a new monster can be placed there */
			/* they also create real explosions */
			if(!youagr)
				mondead(magr);
			if (pa == &mons[PM_SWAMP_FERN_SPORE])
				explode(x(magr), y(magr), AD_DISE, MON_EXPLODE, dmg, EXPL_MAGICAL, 1);
			else if (pa == &mons[PM_BURNING_FERN_SPORE])
				explode(x(magr), y(magr), AD_PHYS, MON_EXPLODE, dmg, EXPL_YELLOW, 1);
			else
				explode(x(magr), y(magr), AD_ACID, MON_EXPLODE, dmg, EXPL_NOXIOUS, 1);
			/* players, on the other hand, shouldn't be rehumanized before the explosion (since it will hurt them too) */
			if (youagr && Upolyd)
				rehumanize();
			return (*hp(magr) > 0 ? MM_AGR_STOP : MM_AGR_DIED) | (*hp(mdef) > 0 ? MM_HIT : MM_DEF_DIED);

			/* special explosions */
		case AD_BLND:
			if (!resists_blnd(mdef)) {
				if (youdef) {
					/* sometimes you're affected even if it's invisible */
					if (mon_visible(magr) || (rnd(dmg /= 2) > u.ulevel)) {
						You("are blinded by a blast of light!");
						make_blinded((long)dmg, FALSE);
						if (!Blind) Your("%s", vision_clears);
					}
					else if (flags.verbose)
						You("get the impression it was not terribly bright.");
				}
				else {
					if (youagr && vis&VIS_MDEF) {
						pline("%s is blinded by your flash of light!", Monnam(mdef));
					}
					else if (vis&VIS_MDEF) {
						pline("%s is blinded.", Monnam(mdef));
					}
					mdef->mblinded = min((int)mdef->mblinded + dmg, 127);
					mdef->mcansee = 0;
				}
			}
			else {
				/* message */
				if (youdef) {
					You("seem unaffected.");
				}
				else if (vis&VIS_MDEF) {
					pline("%s seems unaffected.", Monnam(mdef));
				}
			}
			break;
		
		case AD_HALU:
			/* asymetric conditions for this, sadly */
			if (!hallucinogenic(pd) &&
				(youdef ? (!NoLightBlind) : (haseyes(pd) && mdef->mcansee))
				) {
				if (youdef) {
					boolean chg;
					if (!Hallucination)
						You("are caught in a blast of kaleidoscopic light!");
					chg = make_hallucinated(HHallucination + (long)dmg, FALSE, 0L);
					You("%s.", chg ? "are freaked out" : "seem unaffected");
					if (chg && Hallucination && magr->data == &mons[PM_DAUGHTER_OF_BEDLAM])
						u.umadness |= MAD_DELUSIONS;
				}
				else {
					if (youagr && vis&VIS_MDEF) {
						pline("%s is affected by your flash of light!", Monnam(mdef));
					}
					else if (vis&VIS_MDEF) {
						pline("%s looks confused.", Monnam(mdef));
					}
					mdef->mconf = 1;
					mdef->mstrategy &= ~STRAT_WAITFORU;
				}
			}
			else {
				/* message */
				if (youdef) {
					You("seem unaffected.");
				}
				else if (vis&VIS_MDEF) {
					pline("%s seems unaffected.", Monnam(mdef));
				}
			}
			break;

			/* basic damage types */
		case AD_COLD:
			if (Cold_res(mdef))
				dmg = 0;
			goto expl_common;
		case AD_ECLD:
			if (Cold_res(mdef))
				dmg /= 2;
			goto expl_common;
		case AD_FIRE:
			if (Fire_res(mdef))
				dmg = 0;
			goto expl_common;
		case AD_EFIR:
			if (Fire_res(mdef))
				dmg /= 2;
			goto expl_common;
		case AD_ACFR:
			if (Fire_res(mdef) && !(youdef ? hates_holy(pd) : hates_holy_mon(mdef)))
				dmg = 0;
			goto expl_common;
		case AD_ELEC:
			if (Shock_res(mdef))
				dmg = 0;
			goto expl_common;
		case AD_EELC:
			if (Shock_res(mdef))
				dmg /= 2;
			goto expl_common;
		case AD_DESC:
			if (is_anhydrous(pd) || is_undead_mon(mdef))
				dmg = 0;
			goto expl_common;
expl_common:
			/* dmg has been (maybe fully) reduced based on element */

			/* apply half phys damage */
			if (Half_phys(mdef))
				dmg = (dmg + 1) / 2;

			if (dmg > 0) {
				/* damage dealt */
				/* message */
				if (youdef) {
					/* player can "dodge" based on DEX */
					if (ACURR(A_DEX) > rnd(20)) {
						You("duck some of the blast.");
						dmg = (dmg + 1) / 2;
					}
					else {
						if (flags.verbose) You("get blasted!");
					}
				}
				else if (vis) {
					pline("%s gets blasted!", Monnam(mdef));
				}

				/* deal damage */
				result = xdamagey(magr, mdef, attk, dmg, FALSE);
			}
			else {
				/* damage resisted */
				/* message */
				if (youdef) {
					You("are unhurt.");
				}
				else if (vis&VIS_MDEF) {
					/* we can only tell damage wasn't dealt if we can see mdef */
					pline_The("blast doesn't seem to hurt %s.", mon_nam(mdef));
				}
				else if (vis) {
					/* intentionally duplicate the blast-hurt message -- we cannot see mdef */
					pline("%s gets blasted!", Monnam(mdef));
				}
				/* golem effects */
				if (is_golem(pd)) {
					int golem_ad = 0;
					switch (attk->adtyp) {
					case AD_COLD:
					case AD_ECLD:
						roll_frigophobia();
						golem_ad = AD_COLD;
						break;
					case AD_FIRE:
					case AD_EFIR:
					case AD_ACFR:
						golem_ad = AD_FIRE;
						break;
					case AD_ELEC:
					case AD_EELC:
						golem_ad = AD_ELEC;
						break;
					}
					if (golem_ad) {
						if (youdef)
							ugolemeffects(golem_ad, fulldmg);
						else
							golemeffects(mdef, golem_ad, fulldmg);
					}
				}
			}
			/* damage inventory */
			if (attk->adtyp == AD_FIRE || attk->adtyp == AD_EFIR || attk->adtyp == AD_ACFR){
				if (!InvFire_res(mdef)){
					if (mlev(magr) > rn2(20))
						destroy_item2(mdef, SCROLL_CLASS, AD_FIRE, youdef);
					if (mlev(magr) > rn2(20))
						destroy_item2(mdef, POTION_CLASS, AD_FIRE, youdef);
					if (mlev(magr) > rn2(25))
						destroy_item2(mdef, SPBOOK_CLASS, AD_FIRE, youdef);
				}
			}
			else if (attk->adtyp == AD_ELEC || attk->adtyp == AD_EELC){
				if (!InvShock_res(mdef)){
					if (mlev(magr) > rn2(20))
						destroy_item2(mdef, WAND_CLASS, AD_ELEC, youdef);
					if (mlev(magr) > rn2(20))
						destroy_item2(mdef, RING_CLASS, AD_ELEC, youdef);
				}
			}
			else if (attk->adtyp == AD_COLD || attk->adtyp == AD_ECLD){
				if (!InvCold_res(mdef)){
					if (mlev(magr) > rn2(20))
						destroy_item2(mdef, POTION_CLASS, AD_COLD, youdef);
				}
			}
			break;
		}
	}
	/* kill exploder */
	if (youagr) {
		rehumanize();
		result |= MM_AGR_STOP;
	}
	else {
		mondead(magr);
		if (*hp(magr) > 0)
			result |= MM_AGR_STOP;
		else
			result |= MM_AGR_DIED;

		/* give this message even if it was visible */
		if (magr->mtame)
			You("have a melancholy feeling for a moment, then it passes.");
	}
	/* wake nearby monsters -- it's a loud boom */
	wake_nearto_noisy(x(magr), y(magr), 7 * 7);

	return result;
}

boolean
umetgaze(mtmp)
struct monst *mtmp;
{
	return (canseemon_eyes(mtmp) && couldsee(mtmp->mx, mtmp->my) && !(ublindf && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD));
}

boolean
mmetgaze(looker, lookie)
struct monst *looker;
struct monst *lookie;
{
	return (mon_can_see_mon(looker, lookie) && !(is_blind(looker) || is_blind(lookie)) && !(looker->msleeping || lookie->msleeping));
}

/* xgazey()
 * 
 * A creature uses its gaze attack (either active or passive) on another.
 *
 * Returns MM_MISS if this failed and took no time (though a player attempting to #monster gaze still used their turn)
 *
 * Otherwise returns MM hitflags as usual.
 */
int
xgazey(magr, mdef, attk, vis)
struct monst * magr;
struct monst * mdef;
struct attack * attk;
int vis;
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = (youagr ? youracedata : magr->data);
	struct permonst * pd = (youdef ? youracedata : mdef->data);

	boolean needs_magr_eyes = TRUE;		/* when TRUE, mdef is protected if magr is blind */
	boolean needs_mdef_eyes = TRUE;		/* when TRUE, mdef is protected by being blind */
	boolean needs_uncancelled = TRUE;	/* when TRUE, attack cannot happen when cancelled */
	boolean maybe_not = (!youagr);		/* when TRUE, occasionally doesn't use gaze attack at all */

	static const int randomgazeattacks[] = { AD_DEAD, AD_CNCL, AD_PLYS, AD_DRLI, AD_ENCH, AD_STON, AD_LUCK,
		AD_CONF, AD_SLOW, AD_STUN, AD_BLND, AD_FIRE, AD_FIRE,
		AD_COLD, AD_COLD, AD_ELEC, AD_ELEC, AD_HALU, AD_SLEE };
	static const int elementalgazeattacks[] = { AD_FIRE, AD_COLD, AD_ELEC };

	char buf[BUFSZ];
	struct attack alt_attk;

	int result = MM_MISS;
	int adtyp = attk->adtyp;
	int dmg = d((int)attk->damn, (int)attk->damd);
	int fulldmg = dmg;			/* original unreduced damage */

	if (vis == -1)
		vis = getvis(magr, mdef, 0, 0);

	/* Hamsa ward protects from gazes */
	if (ward_at(x(mdef), y(mdef) == HAMSA))
		return MM_MISS;
	/* at the very least, all gaze attacks need a clear line of sight */
	if (!clear_path(x(magr), y(magr), x(mdef), y(mdef)))
		return MM_MISS;

	/* fix up adtyps for some gazes */
	switch (adtyp)
	{
	case AD_RGAZ:
		adtyp = randomgazeattacks[rn2(SIZE(randomgazeattacks))];
		break;
	case AD_RBRE:
	case AD_RETR:
		adtyp = elementalgazeattacks[rn2(SIZE(elementalgazeattacks))];
		break;
	case AD_WISD:
		if (!youdef)
			adtyp = AD_CONF;
		break;
	}
	/* figure out if gaze requires eye-contact or not */
	switch (adtyp)
	{
		/* meeting the gaze of the monster is dangerous */
	case AD_DEAD:
	case AD_PLYS:
	case AD_STON:
	case AD_LUCK:
	case AD_BLND:
	case AD_CONF:
	case AD_SLOW:
	case AD_STUN:
	case AD_HALU:
	case AD_SLEE:
	case AD_BLNK:
	case AD_SSEX:
	case AD_SEDU:
	case AD_VAMP:
	case AD_WISD:
		needs_magr_eyes = TRUE;
		needs_mdef_eyes = TRUE;
		break;
		/* the monster staring *at* something is dangerous */
	case AD_FIRE:
	case AD_COLD:
	case AD_ELEC:
	case AD_DRLI:
	case AD_CNCL:
	case AD_ENCH:
	case AD_SSUN:
	case AD_STDY:
	case AD_BLAS:
		needs_magr_eyes = TRUE;
		needs_mdef_eyes = FALSE;
		break;
		/* these adtyps are just using gaze as a convenient way of causing something non-gaze-y to happen */
	case AD_WTCH:
	case AD_MIST:
	case AD_SPOR:
		needs_magr_eyes = FALSE;
		needs_mdef_eyes = FALSE;
		needs_uncancelled = FALSE;
		/* these are just straight copy-pasted from originals at the moment, and only are coded for monster vs player */
		if (!youdef)
			return MM_MISS;
		break;
	default:
		impossible("unhandled gaze type %d", adtyp);
		break;
	}
	/* special cases */
	if (pa == &mons[PM_MEDUSA] && adtyp == AD_STON) {	// Medusa's petrification curse
		needs_magr_eyes = FALSE;
		maybe_not = FALSE;
	}
	/* actually, right now, all stoning gazes are a straight copy-paste, so do this for now. */
	if (adtyp == AD_STON) {
		needs_magr_eyes = needs_mdef_eyes = maybe_not = needs_uncancelled = FALSE;
	}
	if (is_angel(pa) && adtyp == AD_BLND) {				// Angels' blinding radiance
		needs_magr_eyes = FALSE;
		maybe_not = FALSE;
	}
	if (is_uvuudaum(pa) && adtyp == AD_CONF) {			// Uvuudaum's form 
		needs_magr_eyes = FALSE;
		needs_uncancelled = FALSE;
		maybe_not = FALSE;
	}
	if (pa == &mons[PM_DEMOGORGON]) {					// Demogorgon is special
		needs_mdef_eyes = TRUE;
		needs_uncancelled = FALSE;
		maybe_not = FALSE;
	}
	if (attk->adtyp == AD_WISD) {						// Obox-Ob, Great Cthulhu special
		needs_magr_eyes = FALSE;
		needs_uncancelled = FALSE;
		maybe_not = FALSE;
	}

	if ((needs_magr_eyes && (
		(youagr  && (Blind)) ||
		(!youagr && (is_blind(magr)))
		))
		||
		(needs_mdef_eyes && (
		(youdef  && (!umetgaze(magr) || multi >= 0)) ||
		(youagr  && (mon_can_see_you(mdef))) ||
		(!youagr && !youdef && (!mmetgaze(magr, mdef)))
		))){
		/* gaze fails because the appropriate gazer/gazee eye (contact?) is not available */
		return MM_MISS;
	}
	if (needs_uncancelled && !(
		youagr ||
		!magr->mcan
		)) {
		/* gaze fails because magr is cancelled */
		return MM_MISS;
	}
		
	/* Do the appropriate stuff -- function often returns in this switch statement */
	switch (adtyp)
	{
		/* elemental gazes */
	case AD_FIRE:
	case AD_COLD:
	case AD_ELEC:
		/* 4/5 chance to succeed */
		if (maybe_not && !rn2(5))
			return MM_MISS;
		/* message */
		if (vis&VIS_MAGR) {
			switch (adtyp) {
			case AD_FIRE:	Sprintf(buf, "fiery");		break;
			case AD_COLD:	Sprintf(buf, "icy");		break;
			case AD_ELEC:	Sprintf(buf, "shocking");	break;
			}
			pline("%s attack%s %s with a %s stare.",
				(youagr ? "You" : Monnam(magr)),
				(youagr ? "" : "s"),
				(youdef ? "you" : mon_nam(mdef)),
				buf
				);
		}
		/* re-use xmeleehurty -- it will print a "X is on fire"-esque message, deal damage, and all other good stuff */
		alt_attk = *attk;
		alt_attk.aatyp = AT_GAZE;
		alt_attk.adtyp = adtyp;
		result = xmeleehurty(magr, mdef, &alt_attk, (struct obj *)0, FALSE, dmg, 0, vis, FALSE);
		wakeup2(mdef, youagr);
		break;

		/* lifedrain gazes */
	case AD_VAMP:
	case AD_DRLI:
		/* Demogorgon's gaze is special, of course*/
		if (youdef && pa == &mons[PM_DEMOGORGON]){
			if (!Drain_resistance || !rn2(3)){
				You("meet the gaze of Hethradiah, right head of Demogorgon!");
				You("feel a primal darkness fall upon your soul!");
				losexp("primal darkness", FALSE, !rn2(3), FALSE);
				losexp("primal darkness", FALSE, !rn2(3), FALSE);
				losexp("primal darkness", TRUE, TRUE, FALSE);
				if (u.sealsActive&SEAL_HUGINN_MUNINN){
					unbind(SEAL_HUGINN_MUNINN, TRUE);
				}
				else {
					forget(13);
				}
			}
			else {
				You("avoid the gaze of the right head of Demogorgon!");
				return MM_MISS;
			}
		}
		else {
			/* 1/3 chance to succeed */
			if (maybe_not && rn2(3))
				return MM_MISS;

			/* Don't waste turns trying to drain the life from a resistant target */
			if (Drain_res(mdef))
				return MM_MISS;

			/* message */
			if (youdef) {
				if (vis&VIS_MAGR) {
					if (adtyp == AD_VAMP)
						pline("%s hungry eyes feed on your life force!", Monnam(magr));
					else
						You("feel your life force wither before the gaze of %s!", mon_nam(magr));
				}
				else
					You("feel your life force wither!");
			}
			else if (vis&VIS_MAGR && vis&VIS_MDEF) {
				pline("%s gaze %ss%s %s's life force!",
					(youagr ? "Your" : s_suffix(Monnam(magr))),
					(adtyp == AD_VAMP ? "feed" : "wither"),
					(adtyp == AD_VAMP ? " on" : ""),
					mon_nam(mdef)	/* note: cannot use two s_suffix() calls in same pline. Grr. */
					);
			}
			/* drain life! */
			if (youdef) {
				/* the player has a handy level-drain function */
				losexp("life force drain", TRUE, FALSE, FALSE);
			}
			else {
				/* print message first -- this should happen before the victim is drained/dies */
				if (vis&VIS_MDEF)
					pline("%s suddenly seems weaker!", Monnam(mdef));

				/* for monsters, we need to make something up -- drain 2d6 maxhp, 1 level */
				dmg = d(2, 6);

				/* kill if this will level-drain below 0 m_lev, or lifedrain below 1 maxhp */
				if (mlev(mdef) == 0 || *hpmax(mdef) <= dmg) {
					/* clean up the maybe-dead monster, return early */
					if (youagr)
						killed(mdef);
					else
						monkilled(mdef, "", attk->adtyp);
					/* is it dead, or was it lifesaved? */
					if (mdef->mhp > 0)
						return MM_DEF_LSVD;	/* lifesaved */
					else
						return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));
				}
				else {
					/* drain stats */
					mdef->m_lev--;
					mdef->mhpmax -= dmg;
				}
			}
		}
		if (*hp(mdef) > 0)
			wakeup2(mdef, youagr);
		break;
	case AD_SSUN:
		/* requires reflectable light */
		if (!levl[x(magr)][y(magr)].lit)
			return MM_MISS;

		/* message and blind */
		if (youdef) {
			pline("%s attacks you with a beam of reflected light!", Monnam(magr));
			stop_occupation();

			if (canseemon(magr) && !resists_blnd(&youmonst)) {
				You("are blinded by %s beam!", s_suffix(mon_nam(magr)));
				make_blinded((long)dmg, FALSE);
			}
			if (Fire_resistance) {
				pline_The("beam doesn't feel hot!");
				dmg = 0;
			}
			else if (Reflecting){
				if (canseemon(magr)) ureflects("%s beam is reflected by your %s.", s_suffix(Monnam(magr)));
				dmg = 0;
			}
		}
		else {
			if (vis&VIS_MAGR && vis&VIS_MDEF) {
				pline("%s attack%s %s with a beam of reflected light!",
					(youagr ? "You" : Monnam(magr)),
					(youagr ? "" : "s"),
					mon_nam(mdef)
					);
			}
			if (can_blnd(magr, mdef, attk->aatyp, (struct obj *)0)) {
				if (vis&VIS_MDEF)
					pline("%s is blinded!", Monnam(mdef));
				mdef->mblinded = dmg;
				mdef->mcansee = 0;
			}
			if (Fire_res(mdef))
				dmg = 0;
			else if (mon_reflects(mdef, "The beam is reflected by %s %s!"))
				dmg = 0;
		}
		/* damage inventory */
		if (!InvFire_res(mdef) && !(youdef ? Reflecting : mon_resistance(mdef, REFLECTING))) {
			if ((int)mlev(magr) > rn2(20))
				destroy_item2(mdef, SCROLL_CLASS, AD_FIRE, youdef);
			if ((int)mlev(magr) > rn2(20))
				destroy_item2(mdef, POTION_CLASS, AD_FIRE, youdef);
			if ((int)mlev(magr) > rn2(25))
				destroy_item2(mdef, SPBOOK_CLASS, AD_FIRE, youdef);
		}

		if (youdef)
			burn_away_slime();

		if (dmg)
			result = xdamagey(magr, mdef, attk, dmg, FALSE);

		if (*hp(mdef) > 0)
			wakeup2(mdef, youagr);
		break;
		/* deathgaze */
	case AD_DEAD:
		/* message */
		if (youdef) {
			pline("Oh no, you meet %s gaze of death!",
				s_suffix(mon_nam(magr)));
		}
		else if (vis&VIS_MDEF && vis&VIS_MAGR) {
			pline("%s meets %s gaze of death!",
				Monnam(mdef),
				(youagr ? "your" : s_suffix(mon_nam(magr)))
				);
		}
		/* effect */
		if (youdef) {
			if (nonliving(pd) || is_demon(pd)) {
				You("seem no deader than before.");
			}
			else if (Magic_res(mdef) || (u.sealsActive&SEAL_OSE)) {
				if (Magic_res(mdef))
					shieldeff(u.ux, u.uy);
				pline("Lucky for you, it didn't work!");
			}
			else {
				if (Hallucination) {
					You("have an out of body experience.");
				}
				else {
					killer_format = KILLED_BY_AN;
					killer = "gaze of death";
					done(DIED);

					if (*hp(mdef) > 0)
						return MM_DEF_LSVD;				/* you lifesaved */
					else
						return (MM_HIT | MM_DEF_DIED);	/* moot */
				}
			}
		}
		else {
			if (nonliving_mon(mdef) || is_demon(pd)) {
				if (vis&VIS_MDEF && vis&VIS_MAGR) {
					pline("%s seems no deader than before.",
						Monnam(mdef));
				}
			}
			else if (Magic_res(mdef)) {
				if (vis&VIS_MDEF && vis&VIS_MAGR) {
					pline("It didn't seem to work.");
				}
			}
			else {
				/* no hallucination protection for monsters */
				/* instakill */
				*hp(mdef) = 0;
				if (youagr)
					killed(mdef);
				else
					monkilled(mdef, "", AD_DETH);

				if (*hp(mdef) > 0)
					return MM_DEF_LSVD; /* mdef lifesaved */
				else
					return (MM_HIT | MM_DEF_DIED | ((youagr || grow_up(magr, mdef)) ? 0 : MM_AGR_DIED));
			}
		}
		break;
		/* stonegaze */
	case AD_STON:
		/* STRAIGHT COPY-PASTE FROM ORIGINAL. */
		if (youdef) {
			if (pa == &mons[PM_MEDUSA]){
				static boolean tamemedusa = FALSE;
				if (magr->mcan){
					if (!canseemon(magr)) break;	/* silently */
					pline("%s doesn't look all that ugly.", Monnam(magr));
					break;
				}
				else if (Reflecting && couldsee(magr->mx, magr->my)) {
					/* hero has line of sight to Medusa and she's not blind */
					boolean useeit = canseemon(magr);

					if (useeit){
						if (!(tamemedusa && magr->mtame))
							(void)ureflects("%s image is reflected by your %s.",
							s_suffix(Monnam(magr)));
					}
					if (mon_reflects(magr, (!useeit || tamemedusa) ? (char *)0 :
						"The image is reflected away by %s %s!")){
						if (magr->mtame) tamemedusa = TRUE;
						break;
					}
					if (!m_canseeu(magr) || is_blind(magr)) { /* probably you're invisible */
						if (useeit)
							pline(
							"%s doesn't seem to notice that %s image was reflected.",
							Monnam(magr), mhis(magr));
						break;
					}
					if (useeit)
						pline("%s is turned to stone!", Monnam(magr));
					stoned = TRUE;
					killed(magr);

					if (magr->mhp > 0) break;
					return 2;
				}
				if (umetgaze(magr) && !Stone_resistance) {
					You("see %s.", mon_nam(magr));
					stop_occupation();
					if (poly_when_stoned(youracedata) && polymon(PM_STONE_GOLEM)) break;
					You("turn to stone...");
					killer_format = KILLED_BY;
					killer = "Poseidon's curse";
					done(STONING);
				}
				tamemedusa = FALSE;
			}
			else if (magr->mcan || is_blind(magr)) {
				if (!canseemon(magr)) break;	/* silently */
				pline("%s gazes ineffectually.", Monnam(magr));
				break;
			}
			else if (pa == &mons[PM_PALE_NIGHT]){
				if (canseemon(magr)) pline("%s parts her shroud!", Monnam(magr));
				if (magr->mcan || Stone_resistance) {
					if (!canseemon(magr)) break;	/* silently */
					pline("%s %s.", Monnam(magr),
						"doesn't look all that ugly");
					break;
				}
				if (Reflecting && couldsee(magr->mx, magr->my)) {
					boolean useeit = canseemon(magr);
					if (useeit)
						(void)ureflects("%s image is reflected by your %s.",
						s_suffix(Monnam(magr)));
					if (mon_reflects(magr, !useeit ? (char *)0 :
						"The image is reflected away by %s %s!"))
						break;
					if (!m_canseeu(magr)) { /* probably you're invisible */
						if (useeit)
							pline(
							"%s doesn't seem to notice that %s image was reflected.",
							Monnam(magr), mhis(magr));
						break;
					}
					return 0;
				}
				if (couldsee(magr->mx, magr->my) &&
					!Stone_resistance) {
					if (ublindf && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) {
						Your("lenses block out your sight!");
						break;
					}
					You("see the truth behind the veil!");
					stop_occupation();
					if (poly_when_stoned(youracedata) && polymon(PM_STONE_GOLEM))
						break;
					You("turn to stone...");
					killer_format = KILLED_BY;
					killer = pa->mname;
					done(STONING);
				}
				return 0;
			}
			else if (pa == &mons[PM_BEHOLDER]){
				if (umetgaze(magr) && !Stone_resistance) {
					You("meet %s gaze.", s_suffix(mon_nam(magr)));
					stop_occupation();
					if (poly_when_stoned(youracedata) && polymon(PM_STONE_GOLEM)) break;
					Stoned = 5;
					delayed_killer = "a beholder's eye of petrification.";
					killer_format = KILLED_BY;
				}
			}
			else {
				if (umetgaze(magr) && !Stone_resistance) {
					You("meet %s gaze.", s_suffix(mon_nam(magr)));
					stop_occupation();
					if (poly_when_stoned(youracedata) && polymon(PM_STONE_GOLEM)) break;
					You("turn to stone...");
					killer_format = KILLED_BY;
					killer = pa->mname;
					done(STONING);
				}
			}
		}
		else if (!youagr) {
			if (magr->mcan || !mmetgaze(magr, mdef))
				return MM_MISS;
			if (canseemon(magr)){
				if (attk->aatyp == AT_GAZE){
					Sprintf(buf, "%s", Monnam(magr));
					pline("%s gazes at %s...", buf, mon_nam(mdef));
				}
				else if (attk->aatyp == AT_WDGZ){
					Sprintf(buf, "%s", Monnam(mdef));
					pline("%s can see %s...", buf, mon_nam(magr));
				}
			}
			/* may die from the acid if it eats a stone-curing corpse */
			if (!resists_ston(mdef) && munstone(mdef, FALSE))
				goto post_stone;
			if (poly_when_stoned(pd)) {
				mon_to_stone(mdef);
				break;
			}
			if (!resists_ston(mdef)) {
				if (vis)
					pline("%s turns to stone!", Monnam(mdef));
				monstone(mdef);
			post_stone:
				if (mdef->mhp > 0)
					return MM_HIT;
				else if (mdef->mtame && !vis)
					You("have a peculiarly sad feeling for a moment, then it passes.");

				return (MM_DEF_DIED | (grow_up(magr, mdef) ? 0 : MM_AGR_DIED));
			}
			break;
		}
		else {
			/* youagr */
			/* NOT IMPLEMENTED YET */;
		}
		break;
		/* confusion */
	case AD_CONF:
		/* 4/5 chance to succeed */
		if (maybe_not && !rn2(5))
			return MM_MISS;
		/* default confusion time: 3d4 */
		if (!dmg)
			dmg = d(3, 4);
		/* put on cooldown */
		if (!youagr && !(is_uvuudaum(pa) || attk->adtyp == AD_WISD)) {
			if (magr->mspec_used)
				return MM_MISS;
			else
				magr->mspec_used = (dmg + rn2(6));
		}

		if (youdef) {
			if (Confusion) {
				You("are getting more and more confused.");
				/* if magr is confusing us every turn, let's not stack confusion too high */
				if (!magr->mspec_used && (HConfusion > 0))
					dmg = min(dmg*dmg / HConfusion, dmg);
			}
			else {
				pline("%s %s confuses you!",
					s_suffix(Monnam(magr)),
					((is_uvuudaum(pa) || attk->adtyp == AD_WISD) ? "form" : "gaze")
					);
			}
			make_confused(HConfusion + dmg, FALSE);
			stop_occupation();
		}
		else
		{
			if (!mdef->mconf) {
				if (vis&VIS_MDEF && vis&VIS_MAGR) {
					pline("%s looks confused by %s %s.",
						Monnam(mdef),
						(youagr ? "your" : s_suffix(mon_nam(magr))),
						((is_uvuudaum(pa) || attk->adtyp == AD_WISD) ? "form" : "gaze")
						);
				}
				else if (vis&VIS_MDEF) {
					pline("%s looks confused.", Monnam(mdef));
				}
				mdef->mconf = 1;
				mdef->mstrategy &= ~STRAT_WAITFORU;
			}
		}
		break;
		/* paralysis */
	case AD_PLYS:
		/* Demogorgon's gaze is special, of course*/
		if (youdef && pa == &mons[PM_DEMOGORGON]){
			if ((!Free_action || rn2(2)) && (!Sleep_resistance || rn2(4))){
				You("meet the gaze of Aameul, left head of Demogorgon!");
				You("are mesmerized!");
				nomovemsg = 0;	/* default: "you can move again" */
				if (!Free_action && !Sleep_resistance) nomul(-rn1(5, 2), "mesmerized by Aameul");
				else if (!Free_action || !Sleep_resistance) nomul(-1, "mesmerized by Aameul");
				else youmonst.movement -= 6;
				exercise(A_DEX, FALSE);
			}
			else {
				You("avoid the gaze of the left head of Demogorgon!");
				return MM_MISS;
			}
		}
		else {
			/* 1/3 chance to succeed */
			if (maybe_not && rn2(3))
				return MM_MISS;

			/* calc max paralysis time, and default time is 1d10 */
			int maxdmg = attk->damn * attk->damd;
			if (!dmg) {
				dmg = rnd(10);
				maxdmg = 10;
			}
			/* put on cooldown */
			if (!youagr) {
				if (magr->mspec_used)
					return MM_MISS;
				else
					magr->mspec_used = maxdmg;
			}

			/* split between player and monster */
			if (youdef) {
				if (Free_action) {
					You("momentarily stiffen.");
				}
				else{
					You("are mesmerized by %s!", mon_nam(magr));
					nomovemsg = 0;	/* default: "you can move again" */
					nomul(-dmg, "mesmerized by a monster");
					exercise(A_DEX, FALSE);
				}
			}
			else {
				if (mon_resistance(mdef, FREE_ACTION)) {
					if (vis&VIS_MDEF) {
						pline("%s momentarily stiffens.", Monnam(mdef));
					}
				}
				else {
					if (vis&VIS_MDEF) {
						pline("%s freezes!", Monnam(mdef));
					}
					mdef->mcanmove = 0;
					mdef->mfrozen = dmg;
					mdef->mstrategy &= ~STRAT_WAITFORU;
				}
			}
		}
		break;

	case AD_ENCH:
		if (youdef && pa == &mons[PM_DEMOGORGON]){		/* has this been depricated? */
			struct obj *obj = some_armor(&youmonst);
			if (drain_item(obj)) {
				You("meet Demogorgon's gaze!");
				Your("%s less effective.", aobjnam(obj, "seem"));
			}
		}
		else {
			/* 1/4 chance to succeed */
			if (maybe_not && rn2(4))
				return MM_MISS;

			struct obj * otmp = some_armor(mdef);

			if (youagr)
				You("stare at %s.", mon_nam(mdef));

			if (drain_item(otmp)) {
				if (youdef) {
					if (vis&VIS_MAGR)
						pline("%s stares at you.", Monnam(magr));
					else
						You_feel("watched.");
					Your("%s less effective.",
						aobjnam(otmp, "seem"));
				}
				else if (vis&VIS_MDEF) {
					pline("%s %s less effective.",
						s_suffix(Monnam(mdef)),
						aobjnam(otmp, "seem"));
				}
			}
		}
		break;
		/* slow */
	case AD_SLOW:
		/* 4/5 chance to succeed */
		if (maybe_not && !rn2(5))
			return MM_MISS;
		/* set cooldown */
		if (!youagr) {
			if (magr->mspec_used)
				return MM_MISS;
			else
				magr->mspec_used = rn2(12);
		}

		if (youdef) {
			if (vis&VIS_MAGR)
				pline("%s stares piercingly at you!", Monnam(magr));
			else
				You_feel("watched.");
			u_slow_down();
			stop_occupation();
		}
		else {
			unsigned int oldspeed = mdef->mspeed;

			if (vis&VIS_MAGR && vis&VIS_MDEF)
				pline("%s stare%s piercingly at %s!",
				(youagr ? "You" : Monnam(magr)),
				(youagr ? "" : "s"),
				mon_nam(mdef)
				);

			mon_adjust_speed(mdef, -1, (struct obj *)0);
			mdef->mstrategy &= ~STRAT_WAITFORU;
			if (mdef->mspeed != oldspeed && vis&VIS_MDEF)
				pline("%s slows down.", Monnam(mdef));
		}
		break;
		/* stun */
	case AD_STUN:
		/* 4/5 chance to succeed */
		if (maybe_not && !rn2(5))
			return MM_MISS;
		/* default stun time: 2d6 */
		if (!dmg)
			dmg = d(2, 6);
		/* put on cooldown */
		if (!youagr) {
			if (magr->mspec_used)
				return MM_MISS;
			else
				magr->mspec_used = (dmg + rn2(6));
		}

		if (youdef) {
			if (vis&VIS_MAGR)
				pline("%s stares piercingly at you!", Monnam(magr));
			else
				You_feel("watched.");
			make_stunned(HStun + dmg, TRUE);
			stop_occupation();
		}
		else {
			if (vis&VIS_MAGR && vis&VIS_MDEF)
				pline("%s stare%s piercingly at %s!",
				(youagr ? "You" : Monnam(magr)),
				(youagr ? "" : "s"),
				mon_nam(mdef)
				);
			if (vis&VIS_MDEF) {
				pline("%s %s for a moment.",
					Monnam(mdef),
					makeplural(stagger(mdef, "stagger")));
			}
			mdef->mstun = 1;
		}
		break;
		/* blinding (radiance and gaze) */
	case AD_BLND:
		/* there is an existing can-blind check, yay! */
		if (!can_blnd(magr, mdef, attk->aatyp, (struct obj *)0))
			return MM_MISS;

		/* assumes that angels with AD_BLND have a blinding radiance, which is limited range and stunning */
		if (is_angel(pa)) {
			if (dist2(x(magr), y(magr), x(mdef), y(mdef)) > BOLT_LIM*BOLT_LIM)
				return MM_MISS;
			if (youdef) {
				You("are blinded by %s radiance!", s_suffix(mon_nam(magr)));
				make_blinded((long)dmg, FALSE);
				stop_occupation();
				make_stunned((long)d(1, 3), TRUE);
			}
			else {
				if (vis&VIS_MDEF && vis&VIS_MAGR) {
					pline("%s is blinded by %s radiance!",
						Monnam(mdef),
						(youagr ? "your" : s_suffix(mon_nam(magr)))
						);
				}


				if (vis&VIS_MDEF) {
					pline("%s %s for a moment.",
						Monnam(mdef),
						makeplural(stagger(mdef, "stagger")));
				}
				mdef->mstun = 1;
			}
		}
		/* any other blinding gazes */
		else {
			/* 4/5 chance to succeed */
			if (maybe_not && !rn2(5))
				return MM_MISS;
			/* default blind time: 2d6 */
			if (!dmg)
				dmg = d(2, 6);
			/* put on cooldown */
			if (!youagr) {
				if (magr->mspec_used)
					return MM_MISS;
				else
					magr->mspec_used = (dmg + rn2(6));
			}

			if (youdef)
			{
				You("are blinded by %s gaze!", s_suffix(mon_nam(magr)));
				make_blinded((long)dmg, FALSE);
				stop_occupation();
			}
			else {
				if (vis&VIS_MDEF && vis&VIS_MAGR) {
					pline("%s is blinded by %s gaze!",
						Monnam(mdef),
						(youagr ? "your" : s_suffix(mon_nam(magr)))
						);
				}
				mdef->mblinded = dmg;
				mdef->mcansee = 0;
			}
		}
		break;

		/* hallucination */
	case AD_HALU:
		/* 4/5 chance to succeed */
		if (maybe_not && !rn2(5))
			return MM_MISS;
		/* default hallu time: 1d12 */
		if (!dmg)
			dmg = rnd(12);
		/* put on cooldown */
		if (!youagr) {
			if (magr->mspec_used)
				return MM_MISS;
			else
				magr->mspec_used = (dmg + rn2(6));
		}

		if (youdef) {
			if (!hallucinogenic(pd)) {
				boolean chg;
				if (vis&VIS_MAGR)
					pline("%s attacks you with a kaleidoscopic gaze!", Monnam(magr));
				else if (!Hallucination)
					Your("mind is filled with kaleidoscopic light!");
				chg = make_hallucinated(HHallucination + (long)dmg, FALSE, 0L);
				You("%s.", chg ? "are freaked out" : "seem unaffected");
			}
		}
		/* monsters get confused by AD_HALU */
		else {
			if (vis&VIS_MDEF && vis&VIS_MAGR) {
				pline("%s looks confused by %s gaze.",
					Monnam(mdef),
					(youagr ? "your" : s_suffix(mon_nam(magr)))
					);
			}
			else if (vis&VIS_MDEF) {
				pline("%s looks confused.", Monnam(mdef));
			}

			mdef->mconf = 1;
			mdef->mstrategy &= ~STRAT_WAITFORU;
		}
		break;
		/* sleep */
	case AD_SLEE:
		/* 4/5 chance to succeed */
		if (maybe_not && !rn2(5))
			return MM_MISS;
		/* no effect on sleeping or immune targets */
		if (Sleep_res(mdef) || (youdef ? multi >= 0 : mdef->msleeping))
			return MM_MISS;
		/* default sleep time: 1d10 */
		if (!dmg)
			dmg = rnd(10);
		/* put on cooldown */
		if (!youagr) {
			if (magr->mspec_used)
				return MM_MISS;
			else
				magr->mspec_used = (dmg + rn2(6));
		}
		if (youdef) {
			pline("%s gaze makes you very sleepy...",
				s_suffix(Monnam(magr)));
			fall_asleep(-dmg, TRUE);
		}
		else if (sleep_monst(mdef, dmg, -1)) {
			if (vis&VIS_MAGR) {
				if (attk->aatyp == AT_GAZE)
					Sprintf(buf, "is put to sleep by %s gaze.",
					(youagr ? "your" : s_suffix(mon_nam(magr))));
				else if (attk->aatyp == AT_WDGZ)
					Sprintf(buf, "is put to sleep under %s gaze.",
					(youagr ? "your" : s_suffix(mon_nam(magr))));
			}
			else
				Sprintf(buf, "falls asleep!");
			if (vis&VIS_MDEF) {
				pline("%s %s", Monnam(mdef), buf);
			}
			mdef->mstrategy &= ~STRAT_WAITFORU;
			slept_monst(mdef);
		}
		break;

		/* cancellation */
	case AD_CNCL:
		if (cancel_monst(mdef, mksobj(SPE_CANCELLATION, FALSE, FALSE), FALSE, TRUE, FALSE, !rn2(4) ? rnd(mlev(magr)) : 0)) {
			if (youdef) {
				if (vis&VIS_MAGR)
				{
					pline("%s stares at you.", Monnam(magr));
					pline("Your magic fades.");
				}
				else
					You_feel("your magic fade.");
			}
			else {
				/* no message for monsters being cancelled??*/;
			}
		}
		break;

		/* study */
	case AD_STDY:
		if (!youagr) {
			int * study = (youdef ? &(u.ustdy) : &(mdef->mstdy));

			if (dmg > *study) {	// reduce message spam by only showing when study is actually increased
				if (is_orc(pa))
					pline("%s curses and urges %s followers on.", Monnam(magr), mhis(magr));
				else if (pa == &mons[PM_LEGION] || pa == &mons[PM_LEGIONNAIRE])
					/* no message */;
				else if (vis&VIS_MAGR) {
					pline("%s studies %s with a level stare.",
						Monnam(magr),
						(youdef ? "you" : mon_nam(mdef))
						);
				}
				//else no message
				/* add to study */
				*study = dmg;
			}
		}
		else if (vis&VIS_MDEF) {	/* how can you study something if you can't see it? */
			/* assumed that you studying a monster is *not* something that can or will be spammed all the time */
			You("study %s intently.", mon_nam(mdef));
			mdef->mstdy = max(mdef->mstdy, dmg);
		}
		else
			return MM_MISS;
		break;

		/* luck drain */
	case AD_LUCK:
		/* 4/5 chance to succeed */
		if (maybe_not && !rn2(5))
			return MM_MISS;
		/* no effect on monsters */
		if (!youdef)
			return MM_MISS;
		/* put on cooldown */
		if (!youagr) {
			if (magr->mspec_used)
				return MM_MISS;
			else
				magr->mspec_used = d(2, 6);
		}
		/* assumes you are defending */
		pline("%s glares ominously at you!", Monnam(magr));

		/* misc protections */
		if (uwep && uwep->otyp == MIRROR && uwep->blessed) {
			pline("%s sees its own glare in your mirror.",
				Monnam(magr));
			pline("%s is cancelled!", Monnam(magr));
			magr->mcan = 1;
			monflee(magr, 0, FALSE, TRUE);
		}
		else if ((uwep && !uwep->cursed && confers_luck(uwep)) ||
			(stone_luck(TRUE) > 0 && rn2(4))) {
			pline("Luckily, you are not affected.");
		}
		else {
			You_feel("your luck running out.");
			change_luck(-1 * dmg);
		}
		stop_occupation();
		break;

		/* weeping angel gaze */
	case AD_BLNK:
		/* special case: Weeping angels using their gaze attack on each other has unfortunate effects for both of them */
		if (is_weeping(pd)) {
			if (vis&VIS_MAGR && vis&VIS_MDEF) {
				pline("%s and %s are permanently quantum-locked!", Monnam(mdef), mon_nam(magr));
			}
			monstone(mdef);
			monstone(magr);
			return (MM_DEF_DIED | MM_AGR_DIED);
		}
		/* otherwise, weeping angels' gazes only affect the player */
		if (!youdef)
			return MM_MISS;
		/* 4/5 chance to succeed */
		if (maybe_not && !rn2(5))
			return MM_MISS;
		/* put on cooldown */
		/* In practice, this will be zeroed when a new movement ration is handed out, and acts to make sure Blink can only be used once per round. */
		if (!youagr) {
			if (magr->mspec_used)
				return MM_MISS;
			else
				magr->mspec_used = 10;
		}

		dmg = d(1, 4);
		if (!Reflecting) {
			pline("%s reflection in your mind weakens you.", s_suffix(Monnam(magr)));
			stop_occupation();
			exercise(A_INT, TRUE);
			exercise(A_WIS, FALSE);
		}
		else {
			static long lastverbed = 0L;
			if (flags.verbose && lastverbed + 10 < moves)
				/* Since this message means the player is unaffected, limit
				its occurence to preserve flavor but avoid message spam */
				pline("%s is covering its face.", Monnam(magr));
			lastverbed = moves;
			dmg = 0;
		}
		if (dmg) {
			int temparise = u.ugrave_arise;
			u.wimage += dmg;
			if (u.wimage > 10) u.wimage = 10;
			u.ugrave_arise = PM_WEEPING_ANGEL;
			int result = xdamagey(magr, mdef, attk, dmg, FALSE);
			/*If the player surived the gaze attack, restore the value of arise*/
			u.ugrave_arise = temparise;
			return result;
		}
		break;

	case AD_WISD:
		/* cancels if no damage? */
		if (!dmg)
			return MM_MISS;
		/* non-players should get hit by a basic confusion gaze */
		/* that swap should have happened already */
		if (!youdef) {
			impossible("AD_WISD gaze not being swapped out vs non-player?");
			return MM_MISS;
		}
		/* put on cooldown */
		/* In practice, this will be zeroed when a new movement ration is handed out, and acts to make sure this can only be used once per round. */
		if (!youagr) {
			if (magr->mspec_used)
				return MM_MISS;
			else
				magr->mspec_used = 4;
		}

		/* assumes only player defending now */
		pline("Blasphemous geometries assault your sanity!");
		if (u.sealsActive&SEAL_HUGINN_MUNINN){
			unbind(SEAL_HUGINN_MUNINN, TRUE);
		}
		else {
			while (!(ABASE(A_WIS) <= ATTRMIN(A_WIS)) && dmg > 0) {
				dmg--;
				(void)adjattrib(A_WIS, -1, TRUE);
				forget(10);	/* lose 10% of memory per point lost*/
				exercise(A_WIS, FALSE);
				/* Great Cthulhu permanently drains wisdom */
				if ((pa == &mons[PM_GREAT_CTHULHU]) && (AMAX(A_WIS) > ATTRMIN(A_WIS)))
					AMAX(A_WIS) -= 1;
			}
			if (dmg > 0) {
				You("tear at yourself in horror!"); //assume always able to damage self
				xdamagey(magr, mdef, attk, dmg*10, FALSE);
			}
		}
		break;

	case AD_SEDU:
		if (!youdef)
			return MM_MISS;
		/* STRAIGHT COPY-PASTE FROM ORIGINAL */
		else {
			static int engagering5 = 0;
			boolean engring = FALSE;
			if (!engagering5) engagering5 = find_engagement_ring();
			if ((uleft && uleft->otyp == engagering5) || (uright && uright->otyp == engagering5)) engring = TRUE;
			if (u.sealsActive&SEAL_ANDROMALIUS) break;
			if (distu(magr->mx, magr->my) > 1 ||
				magr->mcan ||
				!umetgaze(magr) ||
				is_blind(magr)
				) return MM_MISS;//fail
			//else
			if (pa == &mons[PM_DEMOGORGON]){
				buf[0] = '\0';
				steal(magr, buf, FALSE, FALSE);
				m_dowear(magr, FALSE);
				return MM_HIT;
			}
			if ((pa == &mons[PM_FIERNA] || pa == &mons[PM_PALE_NIGHT]) && rnd(20) < 15) return MM_HIT;
			if (pa == &mons[PM_ALRUNES]){
				if (MON_WEP(magr) && rn2(20)) return MM_HIT;
			}
			if (is_animal(pa)) {
				xyhitmsg(magr, mdef, attk);
				if (magr->mcan) break;
				/* Continue below */
			}
			else if (dmgtype(youracedata, AD_SEDU)
#ifdef SEDUCE
				|| dmgtype(youracedata, AD_SSEX) || dmgtype(youracedata, AD_LSEX)
#endif
				) {
				pline("%s %s.", Monnam(magr), magr->minvent ?
					"brags about the goods some dungeon explorer provided" :
					"makes some remarks about how difficult theft is lately");
				if (!tele_restrict(magr)) (void)rloc(magr, FALSE);
				return MM_AGR_STOP;
			}
			else if (magr->mcan || engring) {
				if (!Blind) {
					pline("%s tries to %s you, but you seem %s.",
						Adjmonnam(magr, "plain"),
						(is_neuter(pa) || flags.female == magr->female) ? "charm" : "seduce",
						(is_neuter(pa) || flags.female == magr->female) ? "unaffected" : "uninterested");
				}
				if (rn2(3)) {
					if (!tele_restrict(magr)) (void)rloc(magr, FALSE);
					return MM_AGR_STOP;
				}
				break;
			}
			buf[0] = '\0';
			switch (steal(magr, buf, FALSE, FALSE)) {
			case -1:
				return MM_AGR_DIED;
			case 0:
				break;
			default:
				if (!is_animal(pa) && !tele_restrict(magr))
					(void)rloc(magr, FALSE);
				if (is_animal(pa) && *buf) {
					if (canseemon(magr))
						pline("%s tries to %s away with %s.",
						Monnam(magr),
						locomotion(magr, "run"),
						buf);
				}
				monflee(magr, 0, FALSE, FALSE);
				return MM_AGR_STOP;
			}
			m_dowear(magr, FALSE);
		}
		break;
	case AD_SSEX:
		if (!youdef)
			return MM_MISS;
		/* STRAIGHT COPY-PASTE FROM ORIGINAL */
		else {
			static int engagering2 = 0;
			if (!engagering2) engagering2 = find_engagement_ring();
			if ((uleft && uleft->otyp == engagering2) || (uright && uright->otyp == engagering2))
				break;
			if (could_seduce(magr, &youmonst, attk) == 1
				&& !magr->mcan
				&& !is_blind(magr)
				&& umetgaze(magr)
				&& distu(magr->mx, magr->my) == 1)
			if (doseduce(magr))
				return MM_AGR_STOP;
		}
		break;

	case AD_BLAS:
		/* 4/5 chance to succeed */
		if (maybe_not && !rn2(5))
			return MM_MISS;
		/* put on cooldown */
		/* In practice, this will be zeroed when a new movement ration is handed out, and acts to make sure it can only be used once per round. */
		if (!youagr) {
			if (magr->mspec_used)
				return MM_MISS;
			else
				magr->mspec_used = 7;
		}
		/* only affects the player */
		if (!youdef)
			return MM_MISS;
		else {
			int angrygod = A_CHAOTIC + rn2(3); //Note: -1 to +1
			u.ualign.record -= rnd(20);
			u.ualign.sins++;
			u.hod += rnd(20);
			u.ugangr[Align2gangr(angrygod)]++;
			angrygods(angrygod);
		}
		break;

		/* MONSTER GENERATING GAZES, MVU ONLY */

	case AD_MIST:  // mi-go mist projector
		/* 4/5 chance to succeed */
		if (maybe_not && !rn2(5))
			return MM_MISS;
		else {
			int i = 0;
			int n = 0;
			if (pa == &mons[PM_MIGO_SOLDIER]){
				n = rn2(4);
				if (cansee(magr->mx, magr->my)) You("see fog billow out from around %s.", mon_nam(magr));
				for (i = 0; i < n; i++) makemon(&mons[PM_FOG_CLOUD], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT);
			}
			else if (pa == &mons[PM_MIGO_PHILOSOPHER]){
				n = rn2(4);
				if (cansee(magr->mx, magr->my)) You("see whirling snow swirl out from around %s.", mon_nam(magr));
				for (i = 0; i < n; i++) makemon(&mons[PM_ICE_VORTEX], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT);
			}
			else if (pa == &mons[PM_MIGO_QUEEN]){
				n = rn2(2);
				if (cansee(magr->mx, magr->my)) You("see scalding steam swirl out from around %s.", mon_nam(magr));
				for (i = 0; i < n; i++) makemon(&mons[PM_STEAM_VORTEX], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT);
			}
			else if (pa == &mons[PM_ANCIENT_TEMPEST]){
				switch (rnd(4)){
				case 1:
					if (cansee(magr->mx, magr->my)) You("see a whisp of cloud swirl out from %s.", mon_nam(magr));
					makemon(&mons[PM_AIR_ELEMENTAL], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT);
					break;
				case 2:
					if (cansee(magr->mx, magr->my)) You("see rain coalesce and stride out from %s.", mon_nam(magr));
					makemon(&mons[PM_WATER_ELEMENTAL], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT);
					break;
				case 3:
					if (cansee(magr->mx, magr->my)) You("see lightning coalesce and strike out from %s.", mon_nam(magr));
					makemon(&mons[PM_LIGHTNING_PARAELEMENTAL], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT);
					break;
				case 4:
					if (cansee(magr->mx, magr->my)) You("see hail coalesce and stride out from %s.", mon_nam(magr));
					makemon(&mons[PM_ICE_PARAELEMENTAL], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT);
					break;
				}
			}
			else{
				if (cansee(magr->mx, magr->my)) You("see fog billow out from around %s.", mon_nam(magr));
				makemon(&mons[PM_FOG_CLOUD], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT);
			}
			return MM_AGR_STOP; // if a mi-go fires a mist projector, it can take no further actions that turn
		}
		break;
	case AD_SPOR:
		/* release a spore if the player is nearby */
		if (is_fern(pa) && !magr->mcan && distu(magr->mx, magr->my) <= 96 &&
			!is_fern_sprout(pa) ? !rn2(2) : !rn2(4)) {
			coord mm;
			mm.x = magr->mx; mm.y = magr->my;
			enexto(&mm, mm.x, mm.y, &mons[PM_DUNGEON_FERN_SPORE]);
			if (pa == &mons[PM_DUNGEON_FERN] ||
				pa == &mons[PM_DUNGEON_FERN_SPROUT]
				) {
				makemon(&mons[PM_DUNGEON_FERN_SPORE], mm.x, mm.y, NO_MM_FLAGS);
			}
			else if (pa == &mons[PM_SWAMP_FERN] ||
				pa == &mons[PM_SWAMP_FERN_SPROUT]
				) {
				makemon(&mons[PM_SWAMP_FERN_SPORE], mm.x, mm.y, NO_MM_FLAGS);
			}
			else if (pa == &mons[PM_BURNING_FERN] ||
				pa == &mons[PM_BURNING_FERN_SPROUT]
				) {
				makemon(&mons[PM_BURNING_FERN_SPORE], mm.x, mm.y, NO_MM_FLAGS);
			}
			else { /* currently these should not be generated */
				makemon(&mons[PM_DUNGEON_FERN_SPORE], mm.x, mm.y, NO_MM_FLAGS);
			}
			if (canseemon(magr)) pline("%s releases a spore!", Monnam(magr));
		}
		break;
	case AD_WTCH:{
					 //Watcher in the Water's tentacle spawn and retreat behavior
					 int ltnt = 0, stnt = 0;
					 struct monst *tmon;
					 if (distmin(u.ux, u.uy, magr->mx, magr->my) <= 2 && !(magr->mflee)){
						 magr->mflee = 1;
						 magr->mfleetim = 2;
					 }
					 for (tmon = fmon; tmon; tmon = tmon->nmon){
						 if (pa == &mons[PM_WATCHER_IN_THE_WATER]){
							 if (tmon->data == &mons[PM_SWARM_OF_SNAKING_TENTACLES]) stnt++;
							 else if (tmon->data == &mons[PM_LONG_SINUOUS_TENTACLE]) ltnt++;
						 }
						 else if (pa == &mons[PM_KETO]){
							 if (tmon->data == &mons[PM_WIDE_CLUBBED_TENTACLE]) ltnt++;
						 }
					 }
					 if (pa == &mons[PM_WATCHER_IN_THE_WATER]){
						 if (stnt<6){
							 makemon(&mons[PM_SWARM_OF_SNAKING_TENTACLES], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT | MM_NOCOUNTBIRTH);
						 }
						 else if (ltnt<2){
							 makemon(&mons[PM_LONG_SINUOUS_TENTACLE], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT | MM_NOCOUNTBIRTH);
						 }
					 }
					 else if (pa == &mons[PM_KETO]){
						 if (ltnt<2){
							 makemon(&mons[PM_WIDE_CLUBBED_TENTACLE], magr->mx, magr->my, MM_ADJACENTOK | MM_ADJACENTSTRICT | MM_NOCOUNTBIRTH);
						 }
					 }
	}break;


		break;
	default:
		impossible("unhandled gaze type %d", adtyp);
	}
	/* if we got to the end, we didn't abort early, which means something should have happened */
	result |= MM_HIT;
	return result;
}

boolean
apply_hit_effects(magr, mdef, otmp, basedmg, dmgptr, dieroll, returnvalue, hittxt)
struct monst * magr;
struct monst * mdef;
struct obj * otmp;
int basedmg;
int * dmgptr;
int dieroll;
int * returnvalue;
boolean * hittxt;
{
	if (otmp->oartifact) {		// artifact
		int tmp = basedmg;
		*hittxt = artifact_hit(magr, mdef, otmp, &tmp, dieroll);
		if (*hp(mdef) <= 0)
			{*returnvalue = MM_DEF_DIED; return TRUE;}
		if (migrating_mons == mdef)
			{*returnvalue = MM_AGR_STOP; return TRUE;}
		if (tmp == 0)
			{*returnvalue = MM_MISS; return TRUE;}
		*dmgptr += (tmp - basedmg);
	}
	if (otmp->oproperties) {	// properties
		int tmp = basedmg;
		*hittxt |= oproperty_hit(magr, mdef, otmp, &tmp, dieroll);
		if (*hp(mdef) <= 0)
			{*returnvalue = MM_DEF_DIED; return TRUE;}
		if (migrating_mons == mdef)
			{*returnvalue = MM_AGR_STOP; return TRUE;}
		if (tmp == 0)
			{*returnvalue = MM_MISS; return TRUE;}
		*dmgptr += (tmp - basedmg);
	}
	if (spec_prop_otyp(otmp)) {	// otyp
		int tmp = basedmg;
		*hittxt |= otyp_hit(magr, mdef, otmp, &tmp, dieroll);
		if (*hp(mdef) <= 0)
			{*returnvalue = MM_DEF_DIED; return TRUE;}
		if (migrating_mons == mdef)
			{*returnvalue = MM_AGR_STOP; return TRUE;}
		if (tmp == 0)
			{*returnvalue = MM_MISS; return TRUE;}
		*dmgptr += (tmp - basedmg);
	}
	return FALSE;
}

/* hmon2point0
 * 
 * Like as it was in uhitm.c, this is a wrapper so that ghod_hitsu() and angry_guards()
 * are called after the player hits, while letting hmoncore have messy returns wherever it wants
 */
int
hmon2point0(magr, mdef, attk, weapon, launcher, thrown, flatbasedmg, monsdmg, dohitmsg, dieroll, recursed, vis, wepgone)
struct monst * magr;	/* attacker */
struct monst * mdef;	/* defender */
struct attack * attk;	/* attack structure to use -- if this does not exist, we MUST have a weapon */
struct obj * weapon;	/* weapon to hit with */
struct obj * launcher;	/* launcher weapon was fired with */
int thrown;				/* was [weapon] thrown or thrust? 0:No 1:thrown properly 2:thrown improperly*/
int flatbasedmg;		/* if >0, REPLACE basedmg with this value -- typically used for unusual weapon hits like throwing something upwards */
int monsdmg;			/* flat damage amount to add onto other effects -- for monster attacks */
boolean dohitmsg;		/* print hit message? */
int dieroll;			/* 1-20 accuracy dieroll, used for special effects */
boolean recursed;		/* True for all but one attacks when 1 object is hitting >1 times in 1 attack. If so, avoid duplicating some messages and effects. */
int vis;				/* True if action is at all visible to the player */
boolean * wepgone;		/* used to return an additional result: was [weapon] destroyed? */
{
	int result;
	boolean u_anger_guards;

	if (magr == &youmonst &&
		mdef->mpeaceful &&
		(mdef->ispriest || mdef->isshk ||
		mdef->data == &mons[PM_WATCHMAN] ||
		mdef->data == &mons[PM_WATCH_CAPTAIN])
		)
		u_anger_guards = TRUE;
	else
		u_anger_guards = FALSE;

	result = hmoncore(magr, mdef, attk, weapon, launcher, thrown, flatbasedmg, monsdmg, dohitmsg, dieroll, recursed, vis, wepgone);

	if (mdef->ispriest && !rn2(2))
		ghod_hitsu(mdef);
	if (u_anger_guards)
		(void)angry_guards(!flags.soundok);

	return result;
}

int
hmoncore(magr, mdef, attk, weapon, launcher, thrown, flatbasedmg, monsdmg, dohitmsg, dieroll, recursed, vis, wepgone)
struct monst * magr;	/* attacker */
struct monst * mdef;	/* defender */
struct attack * attk;	/* attack structure to use */
struct obj * weapon;	/* weapon to hit with */
struct obj * launcher;	/* launcher weapon was fired with */
int thrown;				/* was [weapon] thrown or thrust? 0:No 1:thrown properly 2:thrown improperly*/
int flatbasedmg;		/* if >0, REPLACE basedmg with this value -- currently unused. SCOPECREEP: use hmon for things like throwing an object upwards */
int monsdmg;			/* flat damage amount to add onto other effects -- for monster attacks */
boolean dohitmsg;		/* print hit message? */
int dieroll;			/* 1-20 accuracy dieroll, used for special effects */
boolean recursed;		/* True for all but one attacks when 1 object is hitting >1 times in 1 attack. If so, avoid duplicating some messages and effects. */
int vis;				/* True if action is at all visible to the player */
boolean * wepgone;		/* used to return an additional result: was [weapon] destroyed? */
{
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = (magr ? (youagr ? youracedata : magr->data) : (struct permonst *)0);
	struct permonst * pd = youdef ? youracedata : mdef->data;

	boolean staggering_strike = FALSE;
	boolean shattering_strike = FALSE;
	boolean disarming_strike = FALSE;
	int jousting = 0;		/* can be 1 (joust), 0 (ordinary hit), -1 (joust and lance breaks) */
	int sneak_dice = 0;
	int sneak_attack = 0;
#define SNEAK_HIDDEN	0x01
#define SNEAK_BEHIND	0x02
#define SNEAK_BLINDED	0x04
#define SNEAK_TRAPPED	0x08
#define SNEAK_HELPLESS	0x10
#define SNEAK_JUYO		0x20
	long silverobj = 0L,
		jadeobj = 0L,
		ironobj = 0L,
		holyobj = 0L,
		unholyobj = 0L,
		otherobj = 0L;
	int poisons_resisted = 0,
		poisons_minoreff = 0,
		poisons_majoreff = 0,
		poisons_wipedoff = 0;
	struct obj * poisonedobj;	/* object that is poisoned responsible for above poisons_X variables -- set once, should not be reset after */
	boolean swordofblood = FALSE;

	boolean resisted_weapon_attacks = FALSE;
	boolean resisted_attack_type = FALSE;
	static int warnedotyp = -1;
	static struct permonst *warnedptr = 0;

	char yourbuf[BUFSZ];

	boolean phase_armor = FALSE;
	boolean valid_weapon_attack = FALSE;
	boolean fake_valid_weapon_attack = FALSE;
	boolean invalid_weapon_attack = FALSE;
	boolean unarmed_punch = FALSE;
	boolean unarmed_kick = FALSE;		/* SCOPECREEP: eventually replace kickdmg() */
	boolean natural_strike = FALSE;
	boolean ulightsaberhit = FALSE;

	boolean destroy_one_magr_weapon = FALSE;	/* destroy one of magr's weapons */
	boolean destroy_all_magr_weapon = FALSE;	/* destroy all of magr's weapon */

	boolean real_attack = (attk && (attk->aatyp == AT_WEAP || attk->aatyp == AT_XWEP || attk->damn > 0 || attk->damd > 0));

	boolean hittxt = FALSE;
	boolean lethaldamage = FALSE;

	boolean fired = (weapon && (is_ammo(weapon) || launcher) && thrown == 1);	/* true if we are properly firing ammo (which may actually not use a launcher, eg monster AT_ARRW) */
	
	struct obj tempwep;	/* used to save the data of an object before it gets destroyed, for things like naming */
	struct obj * otmp;	/* generic object pointer -- variable */
	long slot = 0L;		/* slot, either the weapon pointer (W_WEP) or armor -- variable */
	long rslot = 0L;	/* slot, dedicated to rings (left and right) -- set at start, should not be reset */

	/* pick the most correct ring slot */
	rslot = (!attk || !magr || attk->aatyp == AT_MARI) ? 0L	/* no attack, or no attacker, or marilith: not a ring-hand */
		//: (attk->aatyp == AT_WEAP || attk->aatyp == AT_DEVA || attk->aatyp == AT_HODS) ? W_RINGR /* mainhand */
		//: (attk->aatyp == AT_XWEP) ? W_RINGL	/* offhand */
		: ((youagr ? uarms : which_armor(magr, W_ARMS)) || !rn2(2)) ? W_RINGR : W_RINGL;	/* either hand, but not offhand if wearing a shield */

	int precision_mult = 0;	/* damage multiplier for precision weapons */

	/* partial damage counters */
	int basedmg = 0;	/* base weapon/unarmed damage */
	int artidmg = 0;	/* bonus damage from ARTIFACT HIT block */
	int bonsdmg = 0;	/* various bonus damage */
	int snekdmg = 0;	/* sneak attack bonus damage */
	int jostdmg = 0;	/* jousting bonus damage */
	//  monsdmg			/* passed to this function */
	int subtotl = 0;	/* subtotal of above damages */
	int seardmg = 0;	/* X-hating */
	int poisdmg = 0;	/* poisons */
	int heatdmg = 0;	/* clockwork heat */
	int specdmg = 0;	/* sword of blood; sword of mercury */
	int totldmg = 0;	/* total of subtotal and below */

	int result;	/* value to return */

	/* FLOW MAP */
	/*
	 * Figure Out What Applies
	 *  - (in)valid weapon attack?
	 *  - unarmed punch or natural strike?
	 *  - find out if these apply:
	 *     - precision multiplier
	 *     - sneak attack (calc dmg)
	 *     - jousting (calc dmg)(uhitm)
	 *     - staggering strike (uhitm)
	 *     - shattering strike (uhitm)
	 *     - iron/silver/holy/unholy hating (calc dmg)
	 *     - poison (calc dmg vs monsters)
	 *     - clockwork heat (calc dmg)(uhitm)
	 *     - sword of blood (calc dmg)
	 *  - apply these with no message and note that they were applied:
	 *     - zombification (mhitm)
	 *
	 * Calculate Damage
	 *  - Physical Damage
	 *     - base damage
	 *  - Set Phasing
	 *  - ARTIFACT HIT - THIS PRINTS MESSAGES IN MANY CASES AND UNFORTUNATELY CAN'T BE SUPPRESSED
	 *     - add to base damage
	 *     - check if creature is still alive at this point
	 *  - Bonus Reducable Damage
	 *     - sneak attack
	 *     - jousting
	 *     - staggering
	 *     - shattering strike
	 *  - Reduce Damage
	 *     - insubstantial (note: we are already checking that this is hitting, but many things will only do X-hating damage -- catch this)
	 *     - weapon-resist
	 *     - half-phys damage  AND/OR  type-resistance
	 *     - DR
	 *  - Bonus Non-Reducable Damage
	 *     - iron/silver/holy/unholy hating (already calculated)
	 *     - poison (already calculated vs monsters, still don't call poisoned() since it will print messages)
	 *     - clockwork heat (already calculated)
	 *
	 * Hit Message
	 *  - suppress if any of the following apply:
	 *     - !dohitsmg
	 *     - artifacthit printed a message
	 *     - jousting (we always want the jousting message, and it to replace the hit message)
	 *     - staggering strike (we always want the staggering message, and it to replace the hit message)
	 *     - creature will be killed be calculated damage AND the player is attacking
	 *  - print hit message
	 *
	 * Additional Messages
	 *  - juyo sneak attack
	 *  - sneak attack (suppress if blow will kill)
	 *  - jousting
	 *  - shattering strike (suppress if blow will kill; don't shatter weapon if blow will kill)
	 *  - staggering strike (suppress if blow will kill)
	 *  - type-resistance (suppress if blow will kill)
	 *  - weapon-resistance (suppress if blow will kill)
	 *  - ~~~clockwork heat~~~ nevermind, this is silent
	 *  - iron/silver/holy/unholy hating
	 *  - poison (if vs player, NOW call poisoned() since it will print messages)
	 *
	 * Deal Damage
	 *  - possibly killing, with message
	 *
	 * Break Attacker's Weapon
	 *  - lances only
	 *
	 * Versus-living Effects
	 *  - confuse monster
	 *  - hurtle (staggering strike, jousting)
	 *  - pudding division
	 *  - uranium imp warping
	 *  - cutting worm tails
	 */

	/* FIGURE OUT WHAT APPLIES */
	/* what kind of attack is being made? */
	if (weapon) {
		if (/* valid weapon */
			valid_weapon(weapon) &&
			/* being used with an attack action, or no attack action (which implies an oddly-launched object, like a falling boulder or something) */
			(!attk || attk->aatyp == AT_WEAP || attk->aatyp == AT_XWEP || attk->aatyp == AT_DEVA || attk->aatyp == AT_MARI || attk->aatyp == AT_HODS) &&
			/* isn't a misused launcher */
			(!is_launcher(weapon) ||
			weapon->oartifact == ART_LIECLEAVER ||
			weapon->oartifact == ART_ROGUE_GEAR_SPIRITS) &&
			/* isn't a misused polearm */
			(!is_pole(weapon) ||
			thrown ||
			u.usteed ||
			is_vibropike(weapon) ||
			weapon->otyp == AKLYS ||
			weapon->otyp == NAGINATA ||
			weapon->oartifact == ART_WEBWEAVER_S_CROOK ||
			weapon->oartifact == ART_SILENCE_GLAIVE ||
			weapon->oartifact == ART_HEARTCLEAVER ||
			weapon->oartifact == ART_SOL_VALTIVA ||
			weapon->oartifact == ART_DEATH_SPEAR_OF_VHAERUN ||
			weapon->oartifact == ART_SHADOWLOCK ||
			weapon->oartifact == ART_PEN_OF_THE_VOID
			) &&
			/* isn't an unthrown missile */
			!((is_missile(weapon) || is_ammo(weapon)) && weapon->otyp != CHAKRAM && !thrown) &&
			/* isn't an unthrown Houchou */
			!(weapon->oartifact == ART_HOUCHOU && !thrown) &&
			/* isn't unthrowable ammo (ie, any ammo but rocks) being thrown but not fired*/
			!(thrown == 2)
			)
		{
			/* unlit lightsabers are martial arts aids, not weapons */
			if (martial_aid(weapon) && !thrown)
				unarmed_punch = TRUE;
			else
				valid_weapon_attack = TRUE;

			if (youagr && is_lightsaber(weapon) && litsaber(weapon) && !thrown)
				ulightsaberhit = TRUE;
		}
		else
			invalid_weapon_attack = TRUE;
	}
	else {
		if (/* being made with an attack action */
			(attk->aatyp == AT_WEAP || attk->aatyp == AT_XWEP || attk->aatyp == AT_DEVA || attk->aatyp == AT_MARI || attk->aatyp == AT_HODS) &&
			/* not thrown (how could this happen?) */
			!thrown)
			unarmed_punch = TRUE;
		else if (attk->aatyp == AT_KICK && !thrown)	/* monsdmg == 0 for a player's basic kick, monsdmg == -1 for a player's clumsy kick -- different from a horse's kick! */
			unarmed_kick = TRUE;
		else if (attk->aatyp == AT_TUCH && (attk->adtyp == AD_SHDW || attk->adtyp == AD_STAR || attk->adtyp == AD_BLUD || attk->adtyp == AD_MERC) && !thrown)
			fake_valid_weapon_attack = TRUE;
		else
			natural_strike = TRUE;
	}
	/* precision multiplier */
	if (fired && launcher &&								// Firing ammo from a launcher (fired implies thrown)
		(objects[launcher->otyp].oc_skill == P_CROSSBOW ||	// from a REAL crossbow (but not the Pen of the Void or the BFG, those would be brokenly strong)
		launcher->otyp == SNIPER_RIFLE)						// or a sniper rifle
		&& !(noncorporeal(pd) || amorphous(pd) || ((stationary(pd) || sessile(pd)) && (pd->mlet == S_FUNGUS || pd->mlet == S_PLANT)))	// versus vulnerable targets
		){
		precision_mult = 1;
		/* R/U/B/S/E : 1x/1x/1x/2x/3x multiplier */
		if (youagr) {
			precision_mult += max(P_SKILL(objects[launcher->otyp].oc_skill) - 2, 0);
		}
		/* gnomes get bonus +1 mult */
		if (magr && (youagr ? Race_if(PM_GNOME) : is_gnome(pa)))
			precision_mult += 1;
		/* drow get bonus +1 mult for droven crossbows */
		if (magr && (youagr ? Race_if(PM_DROW) : is_drow(pa)) &&
			(launcher->otyp == DROVEN_CROSSBOW))
			precision_mult += 1;

		/* Wrathful Spider is not very precise. */
		if (launcher->oartifact == ART_WRATHFUL_SPIDER)
			precision_mult = min(precision_mult, 2);
	}

	/* sneak attack -- attacker's number of dice */
	if (youagr) {
		/* player-only sources of sneak-attacking */
		if (Role_if(PM_ROGUE) && !Upolyd)
			sneak_dice++;
		if (Role_if(PM_ANACHRONONAUT) && Race_if(PM_MYRKALFR) && !Upolyd)
			sneak_dice++;
		if (Role_if(PM_CONVICT) && !Upolyd && weapon && weapon->owornmask && weapon->otyp == SPOON)
			sneak_dice++;
		if (u.sealsActive&SEAL_ANDROMALIUS)
			sneak_dice++;
		if (ulightsaberhit && u.fightingForm == FFORM_JUYO && (!uarm || is_light_armor(uarm)))
			sneak_dice++;
	}
	if (magr && is_backstabber(pa))
		sneak_dice++;
	if (weapon && weapon->owornmask && weapon->oartifact == ART_SPINESEEKER)
		sneak_dice++;
	if (weapon && weapon->owornmask && weapon->oartifact == ART_PEN_OF_THE_VOID && weapon->ovar1&SEAL_ANDROMALIUS)
		sneak_dice++;

	/* sneak attack -- defender's conditions must allow sneak attacking, and we must have sneak attack dice */
	if (sneak_dice && (
		!noncorporeal(pd) &&
		!amorphous(pd) &&
		!((stationary(pd) || sessile(pd)) && (pd->mlet == S_FUNGUS || pd->mlet == S_PLANT)) &&
		!(youagr && u.uswallow)
		))
	{
		if (youagr) {
			if (!mdef->mcanmove || !mdef->mnotlaugh ||
				mdef->mstun || mdef->mconf || mdef->msleeping)
				sneak_attack |= SNEAK_HELPLESS;
			if (is_blind(mdef))
				sneak_attack |= SNEAK_BLINDED;
			if (mdef->mtrapped)
				sneak_attack |= SNEAK_TRAPPED;
			if ((sgn(mdef->mx - u.ux) != sgn(mdef->mx - mdef->mux) && sgn(mdef->my - u.uy) != sgn(mdef->my - mdef->muy)) ||
				(mdef->mux == 0 && mdef->muy == 0) ||
				(mdef->mflee && pd != &mons[PM_BANDERSNATCH]))
				sneak_attack |= SNEAK_BEHIND;
			if (((mdef->mux != u.ux || mdef->muy != u.uy) &&
				((weapon && weapon == uwep && uwep->oartifact == ART_LIFEHUNT_SCYTHE && has_head(pd) && !is_unalive(pd))
				|| distmin(u.ux, u.uy, mdef->mx, mdef->my) > BOLT_LIM)))
				sneak_attack |= SNEAK_HIDDEN;
			if (ulightsaberhit && u.fightingForm == FFORM_JUYO && (!uarm || is_light_armor(uarm)))
				sneak_attack |= SNEAK_JUYO;	/* modifies a sneak attack; not sufficient on its own */
		}
		else if (youdef) {
			if (u.ustuck || u.utrap)
				sneak_attack |= SNEAK_TRAPPED;
			if (!canseemon(magr) && !sensemon(magr))
			{
				sneak_attack |= SNEAK_BEHIND;
				if (Blind)
					sneak_attack |= SNEAK_BLINDED;

			}
			if (multi || Stunned || Confusion || Sleeping)
				sneak_attack |= SNEAK_HELPLESS;
		}
		else {
			if (mdef->mflee && pd != &mons[PM_BANDERSNATCH])
				sneak_attack |= SNEAK_BEHIND;
			if (is_blind(mdef))
				sneak_attack |= SNEAK_BLINDED;
			if (mdef->mtrapped)
				sneak_attack |= SNEAK_TRAPPED;
			if (!mdef->mcanmove || !mdef->mnotlaugh ||
				mdef->mstun || mdef->mconf || mdef->msleeping)
				sneak_attack |= SNEAK_HELPLESS;
		}
	}
	if ((sneak_attack&~SNEAK_JUYO) && sneak_dice && !recursed)	/* do not multiply sneak damage in multihits */
	{
		int snekdie = mlev(magr);
		/* some things increase sneak attack die size */
		if ((weapon && weapon->oartifact == ART_SILVER_STARLIGHT) ||
			(pa == &mons[PM_CUPRILACH_RILMANI]))
			snekdie = snekdie * 3 / 2;

		/* some things increase the number of sneak dice even further */
		if (weapon && weapon->owornmask && weapon->oartifact == ART_SPINESEEKER && (sneak_attack&SNEAK_BEHIND))
			sneak_dice++;
		if (weapon && weapon->oartifact == ART_PEN_OF_THE_VOID && weapon->ovar1&SEAL_ANDROMALIUS && (mvitals[PM_ACERERAK].died > 0))
			sneak_dice++;

		/* some of the player's glyphs proc on sneak attacks */
		if (youagr) {
			if (active_glyph(CLAWMARK))
				snekdie = snekdie * 13 / 10;
			if (active_glyph(BLOOD_RAPTURE))
				heal(&youmonst, 30);
			if (active_glyph(WRITHE)){
				u.uen += 30;
				if (u.uen > u.uenmax)
					u.uen = u.uenmax;
			}
		}

		/* calculate snekdmg */
		snekdmg = d(sneak_dice, snekdie);
	}
	else {
		/* no sneak attack this time */
		sneak_attack = 0;
		sneak_dice = 0;
	}

	/* jousting */
#ifdef STEED
	if (youagr && !thrown && !recursed) {	/* do not joust in multihits */
		if (u.usteed && weapon &&
			(weapon_type(weapon) == P_LANCE ||
			(weapon->oartifact == ART_ROD_OF_SEVEN_PARTS) ||
			(weapon->oartifact == ART_PEN_OF_THE_VOID && weapon->ovar1&SEAL_BERITH)
			) &&
			mdef != u.ustuck &&
			!Fumbling &&
			!Stunned &&
			(weapon == uwep || (weapon == uswapwep && u.twoweap)))
		{
			/* if using two weapons, use worse of lance and two-weapon skills */
			jousting = 0;
			int joust_dieroll;
			int skill_rating = P_SKILL(weapon_type(weapon));	/* lance skill */
			if (u.twoweap && P_SKILL(P_TWO_WEAPON_COMBAT) < skill_rating)
				skill_rating = P_SKILL(P_TWO_WEAPON_COMBAT);
			if (skill_rating == P_ISRESTRICTED)
				skill_rating = P_UNSKILLED; /* 0=>1 */

			/* odds to joust are expert:80%, skilled:60%, basic:40%, unskilled:20% */
			if ((joust_dieroll = rn2(5)) < skill_rating) {
				if (!unsolid(pd) && !obj_resists(weapon, 0, 100)){
					if (weapon->otyp == DROVEN_LANCE && rnl(40) == (40 - 1))
						jousting = -1;	/* hit that breaks lance */
					else if (joust_dieroll == 0){ /* Droven lances are especially brittle */
						if (weapon->otyp == ELVEN_LANCE && rnl(75) == (75 - 1))
							jousting = -1;	/* hit that breaks lance */
						else if (rnl(50) == (50 - 1))
							jousting = -1;	/* hit that breaks lance */
					}
				}
				if (jousting != -1)
					jousting = 1;	/* successful joust */
			}
			else {
				jousting = 0;	/* did not joust */
			}
			/* exercise skill even for minimal damage hits */
			if (jousting) valid_weapon_attack = TRUE;
		}
	}
	if (jousting) {
		/* calculate bonus jousting damage */
		jostdmg += d(2, (weapon == uwep) ? 10 : 2);
	}
#endif
	/* staggering strike */
	if (youagr && !recursed) {
		if (
			// unarmed_punch (not two-weaponing)
			(unarmed_punch && !Upolyd && !bigmonst(pd) && !thick_skinned(pd) && !(u.twoweap) && 
			(dieroll < P_SKILL(P_BARE_HANDED_COMBAT)) &&
			((uarmg && uarmg->oartifact == ART_PREMIUM_HEART) || !rn2(5))) ||	// (odds reduced by 80% when not wearing Premium Heart)
			// Green Dragon Crescent Blade
			(weapon && weapon == uwep && uwep->oartifact == ART_GREEN_DRAGON_CRESCENT_BLAD &&
			(dieroll < P_SKILL(weapon_type(uwep)))) ||
			// Djem So
			(ulightsaberhit && u.fightingForm == FFORM_DJEM_SO && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm)) &&
			(dieroll < min(P_SKILL(FFORM_DJEM_SO), P_SKILL(weapon_type(uwep)))) &&
			(mdef->mattackedu || !rn2(5))) ||	// (odds reduced by 80% when not counterattacking)
			// Juyo 
			(ulightsaberhit && u.fightingForm == FFORM_JUYO && (!uarm || is_light_armor(uarm)) &&
			(dieroll < min(P_SKILL(FFORM_JUYO), P_SKILL(weapon_type(uwep)))) &&
			((sneak_attack&SNEAK_JUYO) || (rn2(5) < 2)))	// (odds reduced by 60% when not sneak attacking)
			)
		{
			staggering_strike = TRUE;
		}
	}
	/* shattering strike */
	/* note: does not consider defender's weapon here */
	if (youagr && !recursed) {
		if (
			// General Shattering Strike
			(
			(weapon && weapon == uwep) &&	// needs to be done with a mainhand weapon
			(dieroll <= (Role_if(PM_BARBARIAN) ? 4 : 2)) &&	// good roll
			(
			(weapon->oclass == WEAPON_CLASS && bimanual(weapon, youracedata)) ||	// twohanded weapon OR
			(Role_if(PM_SAMURAI) && weapon->otyp == KATANA && !uarms) ||			// samurai w/ a katana and no shield OR
			(weapon->oartifact == ART_PEN_OF_THE_VOID && weapon->ovar1&SEAL_BERITH)	// berith bound into the Pen
			) &&
			(weapon_type(weapon) != P_NONE) && (P_SKILL(weapon_type(weapon)) >= P_SKILLED) &&	// must be Skilled+
			(!u.twoweap)	// cannot be twoweaponing
			) ||
			// Artifacts
			(weapon && arti_shattering(weapon)) ||
			// Green Dragon Crescent Blade
			(weapon && weapon == uwep && weapon->oartifact == ART_GREEN_DRAGON_CRESCENT_BLAD && (dieroll <= 2 + P_SKILL(weapon_type(weapon))))
			)
		{
			shattering_strike = TRUE;
		}
	}
	/* disarming strike */
	/* note: does not consider defender's weapon here */
	if (youagr && valid_weapon_attack && !recursed) {
		if ((((weapon && weapon == uwep) &&	// needs to be done with a mainhand weapon
			(dieroll <= (1 + P_SKILL(weapon_type(weapon))))) &&	// good roll (B:10%  S:15%  E:20%)
			(weapon->otyp == RANSEUR) &&	// can only be done with a ranseur
			(weapon_type(weapon) != P_NONE) && (P_SKILL(weapon_type(weapon)) >= P_BASIC) && // at least Basic skill
			(!u.twoweap)	// not twoweaponing
			)
			||
			arti_disarm(weapon)
			)
		{
			/* this conflicts with shattering strike -- either we are stealing the weapon or breaking it, not both! */
			/* if both apply, 50/50 odds which will be attempted */
			if (!shattering_strike || rn2(2)) {
				shattering_strike = FALSE;
				disarming_strike = TRUE;
			}
		}
	}
	/* X-hating */
	/* note: setting holyobj/etc affects messages later, but damage happens regardless of whether holyobj/etc is set correctly here */
	if (weapon)
		slot = W_WEP;	// note: the pointer <weapon>, which is not necessarily magr's wielded weapon
	else
		slot = attk_equip_slot(attk->aatyp);

	switch (slot)
	{
	case W_ARMG:
		otmp = (youagr ? uarmg : which_armor(magr, slot));
		break;
	case W_ARMF:
		otmp = (youagr ? uarmf : which_armor(magr, slot));
		break;
	case W_ARMH:
		otmp = (youagr ? uarmh : which_armor(magr, slot));
		break;
	case W_WEP:
		otmp = weapon;
		break;
	default:
		otmp = (struct obj *)0;
		break;
	}
	/* fake weapons */
	if (attk && (
		attk->adtyp == AD_GLSS ||
		attk->adtyp == AD_STAR)) {
		if (hates_silver(pd) && !(youdef && u.sealsActive&SEAL_EDEN)) {
			silverobj |= W_SKIN;
			seardmg += rnd(20);
		}
	}
	/* lightsabers */
	else if (otmp && weapon && is_lightsaber(weapon) && litsaber(weapon))
	{
		/* do not do any hates-obj damage */;
	}
	/* weapons/armor */
	else if (otmp &&
		(otmp == weapon ||							// if using a weapon, only check that weapon (probably moot)
		hits_insubstantial(magr, mdef, attk, otmp))	// if armor harmlessly passes through mdef, skin/rings have an effect
		) {
		if (otmp->oartifact == ART_GLAMDRING &&
			(is_orc(pd) || is_demon(pd)))
			silverobj |= slot;
		if (hates_silver(pd) && !(youdef && u.sealsActive&SEAL_EDEN)) {
			if (obj_silver_searing(otmp))
				silverobj |= slot;
			if (obj_jade_searing(otmp))
				jadeobj |= slot;
		}
		if (hates_iron(pd) &&
			otmp->obj_material == IRON) {
			ironobj |= slot;
		}
		
		if (hates_holy_mon(mdef) &&
			(otmp->known && (otmp->oproperties&OPROP_HOLYW || otmp->oproperties&OPROP_HOLY)) && /* message requires a particularly holy object */
			otmp->blessed) {
			holyobj |= slot;
		}
		if (hates_unholy_mon(mdef) &&
			is_unholy(otmp)) {
			unholyobj |= slot;
		}
		/* unusual case: wooden objects carved with the Veioistafur stave deal bonus damage to sea creatures */
		if (otmp->obj_material == WOOD && otmp->otyp != MOON_AXE &&
			(otmp->oward & WARD_VEIOISTAFUR) && mdef->data->mlet == S_EEL) {
			otherobj |= slot;
		}
		/* calculate sear damage */
		seardmg += hatesobjdmg(mdef, otmp);
	}
	/* direct contact (includes rings) */
	else {
		/* skin/touch/etc */
		if (hates_silver(pd) && !(youdef && u.sealsActive&SEAL_EDEN)) {
			if (is_silver_mon(magr)) {
				silverobj |= W_SKIN;
				seardmg += rnd(20);
			}
			// no jade monsters (yet)
		}
		if (hates_iron(pd) &&
			is_iron_mon(magr)) {
			ironobj |= W_SKIN;
			seardmg += rnd(mlev(mdef));
		}
		if (hates_holy_mon(mdef) &&
			is_holy_mon(magr)) {
			holyobj |= W_SKIN;
			seardmg += d(3, 7);
		}
		if (hates_unholy_mon(mdef) &&
			is_unholy_mon(magr)) {
			unholyobj |= W_SKIN;
			seardmg += d(4, 9);
		}

		/* rings, for an unarmed punch/claw attack */
		if (unarmed_punch) {
			/* only the player wears rings */
			if (youagr) {
				/* get correct ring */
				otmp = (rslot == W_RINGL) ? uleft
					: (rslot == W_RINGR) ? uright
					: (struct obj *)0;

				/* find what applies */
				if (otmp) {
					if (hates_silver(pd) && !(youdef && u.sealsActive&SEAL_EDEN)) {
						if (obj_silver_searing(otmp))
							silverobj |= rslot;
						if (obj_jade_searing(otmp))
							jadeobj |= rslot;
					}
					if (hates_iron(pd) &&
						otmp->obj_material == IRON) {
						ironobj |= rslot;
					}
					if (hates_holy_mon(mdef) &&
						(otmp->known && (otmp->oproperties&OPROP_HOLYW || otmp->oproperties&OPROP_HOLY)) && /* message requires a particularly holy object */
						otmp->blessed) {
						holyobj |= rslot;
					}
					if (hates_unholy_mon(mdef) &&
						is_unholy(otmp)) {
						unholyobj |= rslot;
					}
					/* calculate sear damage */
					seardmg += hatesobjdmg(mdef, otmp);
				}
			}
		}
		/* Simurgh's iron claws, for the player attacking with bared hands */
		if (youagr && unarmed_punch && u.sealsActive&SEAL_SIMURGH && hates_iron(pd)) {
			ironobj |= W_SKIN;
			seardmg += rnd(mlev(mdef));
		}
	}
	/* Poison */
	poisonedobj = (struct obj *)0;
	if (valid_weapon_attack) {
		poisonedobj = weapon;
	}
	else if (unarmed_punch) {
		/* only the player wears rings */
		if (youagr && (!uarmg || !hits_insubstantial(magr, mdef, attk, uarmg))) {
			poisonedobj = (rslot == W_RINGL) ? uleft
				: (rslot == W_RINGR) ? uright
				: (struct obj *)0;
		}
	}

	if (poisonedobj && hits_insubstantial(magr, mdef, attk, poisonedobj))
	{
		int poisons = poisonedobj->opoisoned;
		int artipoisons = 0;
		if (arti_poisoned(poisonedobj))
			artipoisons |= OPOISON_BASIC;
		if (poisonedobj->oartifact == ART_WEBWEAVER_S_CROOK)
			artipoisons |= (OPOISON_SLEEP | OPOISON_BLIND | OPOISON_PARAL);
		if (poisonedobj->oartifact == ART_SUNBEAM)
			artipoisons |= OPOISON_FILTH;
		if (poisonedobj->oartifact == ART_MOONBEAM)
			artipoisons |= OPOISON_SLEEP;
		poisons |= artipoisons;

		/* Penalties for you using a poisoned weapon */
		if (poisons && youagr && !recursed)
		{
			if Role_if(PM_SAMURAI) {
				if (!(uarmh && uarmh->oartifact && uarmh->oartifact == ART_HELM_OF_THE_NINJA)){
					adjalign(-sgn(u.ualign.type) * 5); //stiffer penalty

					if (sgn(u.ualign.type) > 0) {
						You("dishonorably use a poisoned weapon!");
						u.ualign.sins++;
						u.hod++;
					}
				}
				else {
					You("dishonorably use a poisoned weapon!");
					adjalign(5);
				}
			}
			else if ((u.ualign.type == A_LAWFUL) && !Race_if(PM_ORC) &&
				!((Race_if(PM_DROW) && !flags.initgend &&
				(Role_if(PM_PRIEST) || Role_if(PM_ROGUE) || Role_if(PM_RANGER) || Role_if(PM_WIZARD))) ||
				((Race_if(PM_HUMAN) || Race_if(PM_INHERITOR) || Race_if(PM_INCANTIFIER) || Race_if(PM_HALF_DRAGON)) && (Pantheon_if(PM_RANGER) || Role_if(PM_RANGER)))
				) &&
				(u.ualign.record > -10)
				) {
				You_feel("like an evil coward for using a poisoned weapon.");
				adjalign(-2);//stiffer penalty
				if (rn2(2)) u.hod++;
			}
		}

		/* which poisons need resist messages, and which will take effect? */
		int i, n;
		for (n = 0; n < NUM_POISONS; n++)
		{
			boolean resists = FALSE;
			boolean majoreff = FALSE;
			i = (1 << n);

			if (!(poisons & i))
				continue;
			/* determine which resistance is being checked and calculate the damage the poison will do */
			switch (i)
			{
			case OPOISON_BASIC:
				resists = Poison_res(mdef);
				majoreff = !rn2(10);
				break;
			case OPOISON_FILTH:
				resists = Sick_res(mdef);
				majoreff = !rn2(10);
				break;
			case OPOISON_SLEEP:
				resists = Sleep_res(mdef);
				majoreff = !rn2(5) || poisonedobj->oartifact == ART_MOONBEAM;
				break;
			case OPOISON_BLIND:
				resists = (Poison_res(mdef) || !haseyes(pd));
				majoreff = !rn2(10);
				break;
			case OPOISON_PARAL:
				resists = FALSE;
				majoreff = !rn2(8);
				break;
			case OPOISON_AMNES:
				resists = mindless_mon(mdef);
				majoreff = !rn2(10);
				break;
			case OPOISON_ACID:
				resists = Acid_res(mdef);
				majoreff = TRUE;
				break;
			case OPOISON_SILVER:
				resists = !(hates_silver(pd) && !(youdef && u.sealsActive&SEAL_EDEN));
				majoreff = TRUE;
				break;
			}
			if (!rn2(20) && !(artipoisons&i))
				poisons_wipedoff |= i;

			if (resists)
				poisons_resisted |= i;
			else
			{
				if (majoreff)
					poisons_majoreff |= i;
				else
					poisons_minoreff |= i;
			}
		}
		/* poison-injecting rings only ever do major effects */
		if (poisonedobj->oclass == RING_CLASS) {
			poisons_resisted &= ~(poisons_majoreff);
			poisons_wipedoff = poisons_majoreff;
			poisons_minoreff = 0;
		}
		/* calculate poison damage (versus monsters only -- poisoned() is used vs player) */
		if (!youdef)
		{
			for (n = 0; n < NUM_POISONS; n++)
			{
				i = (1 << n);
				boolean major = (poisons_majoreff & i);
				boolean minor = (poisons_minoreff & i);
				if (!major && !minor)
					continue;
				/* calculate poison damage */
				switch (i)
				{
				case OPOISON_BASIC:
					poisdmg += (major) ? 9999 : rnd(6);
					break;
				case OPOISON_FILTH:
					poisdmg += (major) ? 9999 : rnd(12);
					break;
				case OPOISON_SLEEP:
					/* no damage */
					break;
				case OPOISON_BLIND:
					poisdmg += (major) ? 3 : rnd(3);
					break;
				case OPOISON_PARAL:
					poisdmg += (major) ? 6 : rnd(6);
					break;
				case OPOISON_AMNES:
					/* no damage */
					break;
				case OPOISON_ACID:
					poisdmg += rnd(10);
					break;
				case OPOISON_SILVER:
					poisdmg += rnd(20);
					break;
				}
			}
		}
	}

	/* Spidersilk adds sleep poison to unarmed punches */
	if (unarmed_punch) {
		otmp = (youagr ? uarm : which_armor(magr, W_ARM));	/* note: don't set poisonedobj; this is additional */
		if (otmp && otmp->oartifact == ART_SPIDERSILK && !rn2(5)) {
			if (Sleep_res(mdef))
				poisons_resisted |= OPOISON_SLEEP;
			else
				poisons_majoreff |= OPOISON_SLEEP;
		}
	}
	/* AD_SHDW attacks are poisoned as well */
	if (attk && attk->adtyp == AD_SHDW) {
		if (Poison_res(mdef))
			poisons_resisted |= OPOISON_BASIC;
		else if (rn2(10))
			poisons_minoreff |= OPOISON_BASIC;
		else
			poisons_majoreff |= OPOISON_BASIC;
	}

	/* Clockwork heat - player melee only */
	if (youagr && !thrown && !recursed) {
		if (uclockwork && !Fire_res(mdef) && u.utemp) {
			int heatdie = min(u.utemp, 20);
			int heatdice = (1 + (u.utemp >= MELTING) + (u.utemp >= MELTED));
			/* unarmed */
			if (unarmed_punch)
			{
				if (u.utemp >= BURNING_HOT && (!uarmg || is_metallic(uarmg))) {
					heatdmg += d(heatdice, heatdie);
				}
				else if (u.utemp >= HOT) {
					heatdmg += d(heatdice, heatdie / 2);
				}
			}
			/* using a metallic "weapon" -- a valid weapon attack isn't necessary */
			else if (weapon && is_metallic(weapon)) {
				heatdmg += d(heatdice, heatdie / 2);
			}
		}
	}

	/* set zombify resulting from melee mvm combat */
	if (!youagr && !youdef && !thrown && !recursed) {
		if ((magr->mfaction == ZOMBIFIED || (magr->mfaction == SKELIFIED && !rn2(20))) && can_undead_mon(mdef)){
			mdef->zombify = 1;
		}

		if ((magr->data == &mons[PM_UNDEAD_KNIGHT]
			|| magr->data == &mons[PM_WARRIOR_OF_SUNLIGHT]
			|| magr->data == &mons[PM_DREAD_SERAPH]
			) && can_undead_mon(mdef)){
			mdef->zombify = 1;
		}

		if (magr->mfaction == FRACTURED && is_kamerel(mdef->data)){
			mdef->zombify = 1;
		}
	}

	/* CALCULATE DAMAGE */
	/* Base Damage */
	/* case 1: valid melee weapon */
	/* case 2: invalid melee weapon */
	/* case 3: unarmed punch */
	/* case 4: unarmed kick */
	/* case 5: none of the above */
	if (valid_weapon_attack) {
		/* note: dmgval() includes enchantment and erosion of weapon */
		if ((weapon->oartifact == ART_PEN_OF_THE_VOID && weapon->ovar1&SEAL_MARIONETTE) ||
			(youagr && !thrown && u.sealsActive&SEAL_MARIONETTE && distmin(u.ux, u.uy, mdef->mx, mdef->my) > 1))
			basedmg = dmgval(weapon, mdef, SPEC_MARIONETTE);
		else
			basedmg = dmgval(weapon, mdef, 0);

		/* Liecleaver adds 1d10 damage to fired bolts */
		if (fired && launcher && launcher->oartifact == ART_LIECLEAVER)
			basedmg += rnd(10);
	}
	else if (invalid_weapon_attack) {
		long cnt = weapon->quan;

		switch (weapon->oclass)
		{
		case POTION_CLASS:
			/* potionhit() requires our potion to be free, so don't use destroy_one_magr_weapon */
			if (cnt > 1L)
				otmp = splitobj(weapon, 1L);
			else {
				otmp = weapon;
				weapon = (struct obj *)0;
			}
			if (otmp->where != OBJ_FREE) {
				if (youagr)
					freeinv(otmp);
				else
					m_freeinv(otmp);
			}

			if (!weapon) {
				/* remember stuff about the potion so weapon can be used for xname() and such
				 * after potionhit() is called */
				weapon = &tempwep;
				*weapon = *otmp;
			}

			/* do the potion effects */
			/* note: if player is defending, this assumes the potion was thrown */
			potionhit(mdef, otmp, youagr);
			*wepgone = TRUE;
			/* check if defender was killed */
			if (*hp(mdef) < 1)
				return MM_DEF_DIED;
			/* potionhit prints messages */
			hittxt = TRUE;
			/* in case potion effect causes transformation */
			pd = (youdef ? youracedata : mdef->data);
			/* set basedmg */
			basedmg = 1;
			break;

		case SPBOOK_CLASS:
			if (youagr && (u.sealsActive&SEAL_PAIMON))
				basedmg = rnd(spiritDsize()) + objects[weapon->otyp].oc_level;
			else
				basedmg = 1;	// assumes spellbooks weigh <200 aum
			break;

		default:
			switch (weapon->otyp)
			{
			case MIRROR:
				/* mirrors shatter */
				if (breaktest(weapon)) {
					/* if break the mirror, it affects your luck. And prints a message. */
					if (youagr) {
						if (u.specialSealsActive&SEAL_NUDZIRATH){
							You("break %s mirror.  You feel a deep satisfaction!",
								shk_your(yourbuf, weapon));
							change_luck(+2);
						}
						else {
							You("break %s mirror.  That's bad luck!",
								shk_your(yourbuf, weapon));
							change_luck(-2);
						}
						hittxt = TRUE;
					}
					destroy_one_magr_weapon = TRUE;
					real_attack = FALSE;
					/* set basedmg */
					basedmg = 1;
				}
				break;

			case EXPENSIVE_CAMERA:
				/* expensive cameras are destroyed */
				if (weapon->otyp == EXPENSIVE_CAMERA) {
					if (youagr) {
						You("succeed in destroying %s camera.  Congratulations!",
							shk_your(yourbuf, weapon));
					}
					destroy_one_magr_weapon = TRUE;
					real_attack = FALSE;
					basedmg = 1;
				}
				break;

			case BOOMERANG:
				/* boomerangs can break when used as melee weapons */
				if (youagr && !thrown &&
					rnl(4) == 4 - 1 && !weapon->oartifact) {
					if (dohitmsg) {
						pline("As you hit %s, %s%s %s breaks into splinters.",
							mon_nam(mdef), (cnt > 1L) ? "one of " : "",
							shk_your(yourbuf, weapon), xname(weapon));

						hittxt = TRUE;
					}
					destroy_one_magr_weapon = TRUE;
					/* slightly more damage */
					basedmg = rnd(2) + 1;
				}
				else
					basedmg = rnd(2);
				break;

			case CORPSE:
				/* some corpses petrify */
				if (touch_petrifies(&mons[weapon->corpsenm])) {
					static const char withwhat[] = "corpse";
					if (dohitmsg) {
						if (thrown && vis) {
							pline_The("%s corpse hits %s%s",
								mons[weapon->corpsenm].mname,
								(youdef ? "you" : mon_nam(mdef)),
								(youdef ? "!" : ".")
								);
							hittxt = TRUE;
						}
						else if (youagr) {
							You("hit %s with %s %s.",
								mon_nam(mdef),
								weapon->dknown ? the(mons[weapon->corpsenm].mname) : an(mons[weapon->corpsenm].mname),
								(weapon->quan > 1) ? makeplural(withwhat) : withwhat
								);
							hittxt = TRUE;
						}
						else if (vis) {
							pline("%s hits %s with %s %s %s%s",
								Monnam(magr),
								(youdef ? "you" : mon_nam(mdef)),
								hisherits(magr),
								mons[weapon->corpsenm].mname,
								((weapon->quan > 1) ? makeplural(withwhat) : withwhat),
								(youdef ? "!" : ".")
								);
							hittxt = TRUE;
						}
					}
					/* do stone */
					result = xstoney(magr, mdef);
					if (result&(MM_DEF_DIED | MM_DEF_LSVD | MM_AGR_DIED)) {
						return result;
					}
				}
				/* all corpses deal damage based on the size of the corpse */
				basedmg = (weapon->corpsenm >= LOW_PM ? mons[weapon->corpsenm].msize : 0) + 1;
				break;

			case EGG:
				basedmg = 1;/* nominal physical damage */
				real_attack = FALSE;

				/* luck penalty for killing your own eggs */
				if (youagr && weapon->spe && weapon->corpsenm >= LOW_PM) {
					if (cnt < 5)
						change_luck((schar)-(cnt));
					else
						change_luck(-5);
				}

				/* petrifying eggs */
				if (touch_petrifies(&mons[weapon->corpsenm])) {
					/*learn_egg_type(obj->corpsenm);*/
					if (vis) {
						pline("%s hit%s %s with %s %s egg%s!",
							(youagr ? "You" : Monnam(magr)),
							(youagr ? "" : "s"),
							(youdef ? "you" : mon_nam(mdef)),
							(weapon->known ? "the" : (cnt > 1L) ? "some" : "a"),
							(weapon->known ? mons[weapon->corpsenm].mname : "petrifying"),
							plur(cnt)
							);
						hittxt = TRUE;
					}
					if (vis)
						pline("Splat!");
					weapon->known = 1;	/* (not much point...) */
					destroy_all_magr_weapon = TRUE;

					result = xstoney(magr, mdef);
					/* clean up the eggs right now, if not thrown */
					/* if they were, thrown, the throwing function will clean them up */
					if (!thrown) {
						if (youagr)
							useupall(weapon);
						else for (; cnt; cnt--)
							m_useup(magr, weapon);
						if (wepgone)
							*wepgone = TRUE;
					}
					/* return if we had a significant result from xstoney */
					if (result&(MM_DEF_DIED | MM_DEF_LSVD | MM_AGR_DIED))
					{	
						return result;
					}
				}
				else {	/* ordinary egg(s) */
					const char *eggp =
						(weapon->corpsenm != NON_PM && weapon->known) ?
						the(mons[weapon->corpsenm].mname) :
						(cnt > 1L) ? "some" : "an";

					if (vis) {
						pline("%s hit%s %s with %s egg%s.",
							(youagr ? "You" : Monnam(magr)),
							(youagr ? "" : "s"),
							(youdef ? "you" : mon_nam(mdef)),
							eggp,
							plur(cnt)
							);
						hittxt = TRUE;
					}

					if (touch_petrifies(pd) && !stale_egg(weapon)) {
						if (vis) {
							pline_The("egg%s %s alive any more...",
								plur(cnt),
								(cnt == 1L) ? "isn't" : "aren't");
						}
						if (weapon->timed) obj_stop_timers(weapon);
						weapon = poly_obj(weapon, ROCK);
						weapon->oartifact = 0;
						weapon->spe = 0;
						weapon->known = weapon->dknown = weapon->bknown = 0;
						weapon->owt = weight(weapon);
						if (thrown) {
							place_object(weapon, x(mdef), y(mdef));
							if (wepgone)
								*wepgone = TRUE;
						}
					}
					else {
						if (vis) {
							pline("Splat!");
						}
						destroy_all_magr_weapon = TRUE;
						if (youagr) {
							exercise(A_WIS, FALSE);
						}
					}
				}
				break;

			case CLOVE_OF_GARLIC:
				if (!youdef && is_undead_mon(mdef)) {/* no effect against demons */
					monflee(mdef, d(2, 4), FALSE, TRUE);
				}
				basedmg = 1;
				break;

			case CREAM_PIE:
			case BLINDING_VENOM:
				if (youdef) {
					/*I don't think anything goes here?*/;
				}
				else {
					mdef->msleeping = 0;
				}

				basedmg = 0;
				real_attack = FALSE;

				if (can_blnd(magr, mdef, (uchar)
					(weapon->otyp == BLINDING_VENOM
					? AT_SPIT : AT_WEAP), weapon)) {

					if (youdef) {
						int blindinc = rnd(25);
						hittxt = TRUE;
						if (weapon->otyp == CREAM_PIE) {
							if (!Blind) pline("Yecch!  You've been creamed.");
							else pline("There's %s sticky all over your %s.",
								something,
								body_part(FACE));
						}
						else if (weapon->otyp == BLINDING_VENOM) {
							int num_eyes = eyecount(youracedata);
							/* venom in the eyes */
							if (!Blind) pline_The("venom blinds you.");
							else Your("%s sting%s.",
								(num_eyes == 1) ? body_part(EYE) :
								makeplural(body_part(EYE)),
								(num_eyes == 1) ? "s" : "");
						}
						u.ucreamed += blindinc;
						make_blinded(Blinded + (long)blindinc, FALSE);
						if (!Blind)
							Your1(vision_clears);
					}
					else {
						if (youagr && Blind) {
							pline("%s!",
								weapon->otyp == CREAM_PIE ? "Splat" : "Splash");
						}
						else if (canseemon(mdef)) {
							hittxt = TRUE;
							if (weapon->otyp == BLINDING_VENOM) {
								pline_The("venom blinds %s%s!", mon_nam(mdef),
									mdef->mcansee ? "" : " further");
							}
							else if (weapon->otyp == CREAM_PIE) {
								char *whom = mon_nam(mdef);
								char *what = The(xname(weapon));
								if (!thrown && weapon->quan > 1)
									what = An(singular(weapon, xname));
								/* note: s_suffix returns a modifiable buffer */
								if (haseyes(pd) && pd != &mons[PM_FLOATING_EYE])
									whom = strcat(strcat(s_suffix(whom), " "),
									mbodypart(mdef, FACE));
								pline("%s %s over %s!",
									what, vtense(what, "splash"), whom);
							}
						}

						if (youagr) {
							setmangry(mdef);
						}
						mdef->mcansee = 0;
						int tmp = rn1(25, 21);
						if (((int)mdef->mblinded + tmp) > 127)
							mdef->mblinded = 127;
						else
							mdef->mblinded += tmp;
					}
				}
				break;

			case ACID_VENOM:
				if (Acid_res(mdef)) {
					if (youdef || canseemon(mdef)) {
						hittxt = TRUE;
						if (youagr)
							Your("venom hits %s harmlessly.",
							mon_nam(mdef));
						else {
							pline("%s %s unaffected.",
								(youdef ? "You" : Monnam(mdef)),
								(youdef ? "are" : "is")
								);
						}
					}
					basedmg = 0;
				}
				else {
					if (youdef || canseemon(mdef)) {
						hittxt = TRUE;
						if (youagr)
							Your("venom burns %s!", mon_nam(mdef));
						else {
							pline("The venom burns%s%s!",
								(youdef ? "" : " "),
								(youdef ? "" : mon_nam(mdef))
								);
						}
					}
					basedmg = dmgval(weapon, mdef, 0);
				}
				/* projectile should take care of it */
				//destroy_all_magr_weapon = TRUE;
				real_attack = FALSE;
				break;

			case STILETTOS:
				basedmg = rnd(bigmonst(pd) ? 2 : 6) + weapon->spe + (youagr ? dbon(weapon) : 0);
				if (youagr && u.twoweap)
					basedmg += rnd(bigmonst(pd) ? 2 : 6) + weapon->spe;
				break;

			default:
				basedmg = weapon->owt / 100;
				if (basedmg < 1) basedmg = 1;
				else basedmg = rnd(basedmg);
				if (basedmg > 6) basedmg = 6;
				break;
			}
		}
	}
	else if (unarmed_punch) {
		struct weapon_dice unarmed_dice;
		int unarmedMult;
		/* initialize struct */
		dmgval_core(&unarmed_dice, bigmonst(pd), (struct obj *)0, 0);
		/* determine unarmedMult */
		if (youagr) {
			unarmedMult = Race_if(PM_HALF_DRAGON) ? 3 : (!uarmg && u.sealsActive&SEAL_ECHIDNA) ? 2 : 1;
		}
		else {
			unarmedMult = 1;
		}
		/* base unarmed dice */
		if (youagr && martial_bonus())
			unarmed_dice.oc.damd = 4 * unarmedMult;
		else
			unarmed_dice.oc.damd = 2 * unarmedMult;
		/* Eurynome causes exploding dice, sometimes larger dice */
		if (youagr && u.sealsActive&SEAL_EURYNOME) {
			unarmed_dice.oc.aatyp = AT_EXPL;
			unarmed_dice.oc.damd = max(unarmed_dice.oc.damd,
				2 * rnd(5) + (martial_bonus() ? 2 * unarmedMult : 0));
		}
		/* Grandmaster's robe causes exploding dice, 50% chance of doubled dice */
		otmp = (youagr ? uarmc : which_armor(magr, W_ARMC));
		if (otmp && otmp->oartifact == ART_GRANDMASTER_S_ROBE) {
			unarmed_dice.oc.aatyp = AT_EXPL;
			if (rn2(2)) {
				unarmed_dice.oc.damn *= 2;
			}
		}
		/* calculate dice and set basedmg */
		basedmg = weapon_dmg_roll(&(unarmed_dice.oc), FALSE);

		/* The Annulus is very stronk -- 2x base damage + 2x enchantment */
		/* yes, this can be redoubled by artifact gloves */
		/* it's so strong, its damage applies no matter which hand is punching */
		if (youagr) {	// only the player wears rings
			if ((otmp = uright) && otmp->oartifact == ART_ANNULUS ||
				(otmp = uleft) && otmp->oartifact == ART_ANNULUS)
			{
				basedmg += weapon_dmg_roll(&(unarmed_dice.oc), FALSE);
				basedmg += otmp->spe * 2;
			}
		}

		/* martial arts aids */
		if (weapon && martial_aid(weapon)) {
			/* if these were lit, it would have been a weapon attack, not an unarmed punch */
			if (weapon->oartifact == ART_INFINITY_S_MIRRORED_ARC)
				basedmg += rnd(6) + weapon->spe;
			else if (weapon->otyp == LIGHTSABER ||
				weapon->otyp == BEAMSWORD ||
				weapon->otyp == DOUBLE_LIGHTSABER)
				basedmg += rnd(4) + weapon->spe;
		}

		otmp = (youagr ? uarmg : which_armor(magr, W_ARMG));
		/* fighting gloves give bonus damage */
		static int tgloves = 0;
		if (!tgloves) tgloves = find_tgloves();
		if (otmp && otmp->otyp == tgloves)
			basedmg += ((youagr && martial_bonus()) ? 3 : 1);

		/* some artifact gloves give enchantment */
		if (otmp && (otmp->oartifact == ART_PREMIUM_HEART || otmp->oartifact == ART_GREAT_CLAWS_OF_URDLEN))
			basedmg += otmp->spe;
	}
	else if (unarmed_kick) {
		/* If a proper monster-style attack is being used, no stat-based damage! */
		if (monsdmg == 0) {
			/* Do monsters ever kick for 0 monsdmg? */
			if (youagr)
				basedmg = rnd((ACURRSTR + ACURR(A_DEX) + ACURR(A_CON)) / 15);
			else
				basedmg = 1;
			/* martial players are much better at kicking */
			if (youagr && (martial_bonus() || (youracedata == &mons[PM_SASQUATCH]) || (uarmf && uarmf->otyp == KICKING_BOOTS)))
				basedmg += rn2(ACURR(A_DEX)/2 + 1);
		}

		/* boots can increase kicking damage */
		otmp = (youagr ? uarmf : which_armor(magr, W_ARMF));
		if (otmp) {
			basedmg += otmp->spe;
			if (otmp->otyp == KICKING_BOOTS)
				basedmg += rnd(6) + rnd(5) + (bigmonst(pd) ? 0 : 1);
			if (otmp->otyp == STILETTOS || otmp->otyp == HEELED_BOOTS){
				basedmg += rnd(bigmonst(pd) ? 2 : 6);
			}

			basedmg = max(basedmg, 1);
		}


		/* we are using monsdmg == -1 to signify a 'clumsy' kick that deals half damage */
		if (monsdmg == -1)
			basedmg /= 2;
	}
	else {
		basedmg = 0;
	}
	/* multiply by precision_mult, if it applies */
	basedmg *= (precision_mult ? precision_mult : 1);

	/* fakewep: Sword of Blood bonus damage */
	if (attk->aatyp == AT_SRPR && attk->adtyp == AD_BLUD)
	{
		if (has_blood(pd)) {
			specdmg += mlev(mdef);
			swordofblood = TRUE;	/* must come before "Set Phasing" */
			//pline("The blade of rotted blood tears through your veins!");
		}
	}
	/* fakewep: Sword of Mercury bonus effects */
	if (attk && attk->adtyp == AD_MERC)
	{
		if (!Cold_res(mdef)) {
			specdmg += basedmg;
		}
		/* abuse player's stats */
		if (youdef) {
			exercise(A_INT, FALSE);
			exercise(A_WIS, FALSE);
			exercise(A_CHA, FALSE);
			if (!Poison_res(mdef)){
				exercise(A_INT, FALSE);
				exercise(A_WIS, FALSE);
				exercise(A_CHA, FALSE);
			}
		}
	}

	/* Set Phasing */
	phase_armor = (
		(weapon && arti_shining(weapon)) ||
		(youagr && u.sealsActive&SEAL_CHUPOCLOPS) ||
		(attk && attk->aatyp == AT_SRPR && attk->aatyp != AD_BLUD) ||
		(swordofblood) /* this touch adtyp is only conditionally phasing */
		);

	/* Get Other Bonus Damage */
	/* only applies once per attack */
	if (!recursed)
	{
		/* Study */
		if (youagr && mdef->ustdym){
			bonsdmg += rnd(mdef->ustdym);
		}

		if (youdef && u.ustdy) {
			bonsdmg += u.ustdy;
			u.ustdy /= 2;
		}
		if (!youdef && mdef->mstdy){/* note: negative mstdy is used for monster spell protection */
			bonsdmg += mdef->mstdy;
			if (mdef->mstdy > 0) mdef->mstdy /= 2;
			else mdef->mstdy += 1;
		}
		/* Bardic Encouragement */
		if (youagr)
			bonsdmg += u.uencouraged;
		else if (magr)
			bonsdmg += magr->encouraged;
		/* Singing Sword -- only works when the player is wielding it >_> */
		if (uwep && uwep->oartifact == ART_SINGING_SWORD) {
			if (uwep->osinging == OSING_LIFE){
				if (youagr || (magr->mtame && !mindless_mon(magr) && !is_deaf(magr)))
					bonsdmg += (uwep->spe + 1);
			}
			if (uwep->osinging == OSING_DIRGE) {
				if (!(youagr || (magr->mtame && !mindless_mon(magr) && !is_deaf(magr))))
					bonsdmg -= (uwep->spe + 1);
			}
		}
		/* Dahlver Nar gives bonus damage to unarmed punches */
		if (youagr && unarmed_punch && u.specialSealsActive&SEAL_DAHLVER_NAR) {
			bonsdmg += d(2, 6) + min(u.ulevel / 2, (u.uhpmax - u.uhp) / 10);
		}
		/* general damage bonus */
		if(real_attack){
			/* The player has by-far the most detailed attacks */
			if (youagr && (valid_weapon_attack || fake_valid_weapon_attack || unarmed_punch || unarmed_kick || natural_strike)) {
				int bon_damage = 0;

				bon_damage += u.udaminc;
				bon_damage += aeshbon();
				/* If you throw using a propellor, you don't get a strength
				* bonus but you do get an increase-damage bonus.
				*/
				if(natural_strike)
					bon_damage += dbon((struct obj *)0);
				else if (!thrown)
					bon_damage += dbon(weapon);
				else{ //thrown
					if (!fired)
						bon_damage += dbon(weapon); // thrown by hand, get strength bonus
					else if (launcher && objects[launcher->otyp].oc_skill == P_SLING)
						bon_damage += dbon(launcher); // fired by a sling, get strength bonus
					else if (launcher && launcher->otyp == ATLATL)
						bon_damage += dbon(launcher) * 2; // fired by an atlatl, get 2x strength bonus
					//else no bonus
				}
				bonsdmg += bon_damage;
			} else if(!youagr){
				int bon_damage = 0;

				/* 
				* Monsters don't actually have anything other than a str bonus, and then only from items.
				*/
				if (!thrown)
					bon_damage += m_dbon(magr, weapon);
				else{ //thrown
					if (!fired)
						bon_damage += m_dbon(magr, weapon); // thrown by hand, get strength bonus
					else if (launcher && objects[launcher->otyp].oc_skill == P_SLING)
						bon_damage += m_dbon(magr, launcher); // fired by a sling, get strength bonus
					else if (launcher && launcher->otyp == ATLATL)
						bon_damage += m_dbon(magr, launcher) * 2; // fired by an atlatl, get 2x strength bonus
					//else no bonus
				}
				bonsdmg += bon_damage;
			}
		}
		/* skill damage bonus */
		if(youagr && (valid_weapon_attack || fake_valid_weapon_attack || unarmed_punch)){
			/* note: unarmed kicks do not get skill bonus damage */
			int skill_damage = 0;
			int wtype;

			/* get simple weapon skill associated with the weapon */
			if (fired && launcher)
				wtype = weapon_type(launcher);
			else if (u.twoweap)
				wtype = P_TWO_WEAPON_COMBAT;
			else if (weapon && weapon->oartifact == ART_LIECLEAVER)
				wtype = P_SCIMITAR;
			else if (weapon && weapon->oartifact == ART_ROGUE_GEAR_SPIRITS)
				wtype = P_PICK_AXE;
			else if (weapon && weapon->otyp == KAMEREL_VAJRA && !litsaber(weapon))
				wtype = P_MACE;
			else
				wtype = weapon_type(weapon);
			
			/* ammo fired from a launcher */
			if (fired && launcher) {
				/* precision fired ammo gets skill bonuses, multiplied */
				if (is_ammo(weapon) && (precision_mult))
					skill_damage = weapon_dam_bonus(launcher) * precision_mult;
				/* spears fired from atlatls also get their skill bonus */
				else if (launcher->otyp == ATLATL)
					skill_damage = weapon_dam_bonus(launcher);
				/* other fired ammo does not get skill bonuses */
				else
					skill_damage = 0;
			}
			/* things thrown but not fired (ie no launcher) */
			else if (thrown && !fired) {
				/* ammo thrown without a launcher does not get skill bonuses*/
				if (is_ammo(weapon))
					skill_damage = 0;
				/* otherwise, they do get skill bonuses */
				else
					skill_damage = weapon_dam_bonus(weapon);
			}
			/* melee weapons */
			else {
				/* some weapons use contextually-specific skills */
				if (wtype != P_TWO_WEAPON_COMBAT && wtype != weapon_type(weapon))
					skill_damage = skill_dam_bonus(wtype);
				/* general case */
				else
					skill_damage = weapon_dam_bonus(weapon);
			}

			/* Wrathful Spider halves damage from skill for fired bolts */
			if (launcher && launcher->oartifact == ART_WRATHFUL_SPIDER)
				skill_damage /= 2;

			/* add to the bonsdmg counter */
			bonsdmg += skill_damage;

			/* now, train skills */
			use_skill(wtype, 1);

			if (!thrown && weapon && is_lightsaber(weapon) && litsaber(weapon) && P_SKILL(wtype) >= P_BASIC){
				use_skill(FFORM_SHII_CHO, 1);
				if (P_SKILL(FFORM_SHII_CHO) >= P_BASIC || weapon->oartifact == ART_INFINITY_S_MIRRORED_ARC){
					if ((u.fightingForm == FFORM_SHII_CHO ||
						(u.fightingForm == FFORM_MAKASHI && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm)))
						) &&
						!uarms && !u.twoweap && wtype == P_SABER
						) use_skill(FFORM_MAKASHI, 1);
					if ((u.fightingForm == FFORM_SHII_CHO ||
						(u.fightingForm == FFORM_ATARU && (!uarm || is_light_armor(uarm)))
						) &&
						u.lastmoved + 1 >= monstermoves
						) use_skill(FFORM_ATARU, 1);
					if ((u.fightingForm == FFORM_SHII_CHO ||
						(u.fightingForm == FFORM_DJEM_SO && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm)))
						) &&
						mdef->mattackedu
						) use_skill(FFORM_DJEM_SO, 1);
					if ((u.fightingForm == FFORM_SHII_CHO ||
						(u.fightingForm == FFORM_NIMAN && (!uarm || !is_metallic(uarm)))
						) &&
						u.lastcast >= monstermoves
						) use_skill(FFORM_NIMAN, 1);
				}
			}
		}
	}
	/* ARTIFACT HIT BLOCK */
	/* this must come after skills are trained, as this can kill the defender and cause a return */
	if (valid_weapon_attack || unarmed_punch || unarmed_kick)
	{
		int returnvalue = 0;
		/* use guidance glyph */
		if (youagr && !thrown && active_glyph(GUIDANCE))
			doguidance(mdef, basedmg);
		/* hits with a valid weapon proc effects of the weapon */
		if (valid_weapon_attack) {
			otmp = weapon;
			if (otmp && apply_hit_effects(magr, mdef, otmp, basedmg, &artidmg, dieroll, &returnvalue, &hittxt))
				return returnvalue;
		}
		/* ranged weapon attacks also proc effects of the launcher */
		if (thrown && fired && launcher && valid_weapon_attack) {
			otmp = launcher;
			if (otmp && apply_hit_effects(magr, mdef, otmp, basedmg, &artidmg, dieroll, &returnvalue, &hittxt))
				return returnvalue;
		}
		/* ranged weapon attacks also proc effects of The Helm of the Arcane Archer */
		if (thrown && fired && launcher && valid_weapon_attack &&
			((otmp = (youagr ? uarmh : which_armor(magr, W_ARMH))) &&
			otmp->oartifact == ART_HELM_OF_THE_ARCANE_ARCHER)) {
			if (otmp && apply_hit_effects(magr, mdef, otmp, basedmg, &artidmg, dieroll, &returnvalue, &hittxt))
				return returnvalue;
		}
		/* unarmed punches proc effects of worn gloves */
		if (unarmed_punch) {
			otmp = (youagr ? uarmg : which_armor(magr, W_ARMG));
			if (otmp && apply_hit_effects(magr, mdef, otmp, basedmg, &artidmg, dieroll, &returnvalue, &hittxt))
				return returnvalue;
		}
		/* unarmed kicks proc effects of worn boots */
		if (unarmed_kick) {
			otmp = (youagr ? uarmf : which_armor(magr, W_ARMF));
			if (otmp && apply_hit_effects(magr, mdef, otmp, basedmg, &artidmg, dieroll, &returnvalue, &hittxt))
				return returnvalue;
		}
	}

	/* Sum reduceable damage */
	subtotl = basedmg
		+ artidmg
		+ bonsdmg
		+ ((youagr && !natural_strike) ? 0 : monsdmg)	/* only add monsdmg for monsters or a player making a monster attack */
		+ snekdmg
		+ jostdmg;

	/* Reduce Incoming Damage */
	/* min 1 base damage (0 if real_attack is false) */
	if (subtotl < 1)
		subtotl = (real_attack ? 1 : 0);

	/* some attacks only deal searing damage to insubstantial creatures */
	if (hits_insubstantial(magr, mdef, attk, weapon) == 1) {
		subtotl = 0;
	}

	/* some creatures resist weapon attacks to the extreme */
	if (resist_attacks(pd) && (unarmed_punch || unarmed_kick || valid_weapon_attack || invalid_weapon_attack)) {
		if (subtotl > 0) {
			resisted_weapon_attacks = TRUE;
			subtotl = 0;
		}
	}
	/* other creatures resist specific types of attacks */
	else if (unarmed_punch || unarmed_kick || valid_weapon_attack || invalid_weapon_attack) {
		int attackmask = 0;
		int resistmask = 0;

		/* get attackmask */
		if (weapon && (valid_weapon_attack || invalid_weapon_attack)) {
			otmp = weapon;
			if (is_bludgeon(otmp)
				|| otmp->oartifact == ART_YORSHKA_S_SPEAR
				|| otmp->oartifact == ART_GREEN_DRAGON_CRESCENT_BLAD
				|| otmp->oartifact == ART_INFINITY_S_MIRRORED_ARC
				|| (otmp->otyp == KAMEREL_VAJRA && !litsaber(otmp))
				){
				attackmask |= WHACK;
			}
			if (is_stabbing(otmp)
				|| otmp->oartifact == ART_ROGUE_GEAR_SPIRITS
				|| (otmp->otyp == KAMEREL_VAJRA && !litsaber(otmp))
				){
				attackmask |= PIERCE;
			}
			if (is_slashing(otmp)
				|| otmp->oartifact == ART_LIECLEAVER
				|| otmp->oartifact == ART_INFINITY_S_MIRRORED_ARC
				){
				attackmask |= SLASH;
			}
			if (is_blasting(otmp)
				|| (otmp->oartifact == ART_HOLY_MOONLIGHT_SWORD && otmp->lamplit)
				|| otmp->oartifact == ART_FIRE_BRAND
				|| otmp->oartifact == ART_FROST_BRAND
				){
				attackmask |= EXPLOSION;
			}
			/* if it's not any of the above, we're just smacking things with it */
			if (!attackmask)
				attackmask = WHACK;
		}
		else if (unarmed_punch) {
			//Can always whack someone
			attackmask |= WHACK;

			/* gloves */
			otmp = (youagr ? uarmg : which_armor(magr, W_ARMG));

			if (otmp && (otmp->oartifact == ART_GREAT_CLAWS_OF_URDLEN || otmp->oartifact == ART_SHIELD_OF_THE_RESOLUTE_HEA || otmp->oartifact == ART_PREMIUM_HEART)){
				//Digging claws, or heart-shaped bit
				attackmask |= PIERCE;
			}
			if (otmp && (otmp->oartifact == ART_GREAT_CLAWS_OF_URDLEN || otmp->oartifact == ART_CLAWS_OF_THE_REVENANCER)){
				attackmask |= SLASH;
			}

			if (/* claw attacks are slashing (even while wearing gloves?) */
				(attk && attk->aatyp == AT_CLAW) ||
				/* and so are the player's punches, as a half-dragon or chiropteran */
				(youagr && !Upolyd && (Race_if(PM_HALF_DRAGON) || Race_if(PM_CHIROPTERAN)))
				)
				attackmask |= SLASH;
		}
		else if (unarmed_kick) {
			otmp = (youagr ? uarmf : which_armor(magr, W_ARMF));
			attackmask |= WHACK;
		}
		else {
			/* something odd -- maybe it was a weapon attack and the weapon was destroyed earlier than usual? */
			otmp = (struct obj *)0;
			/* lets just ignore resistances */
			attackmask |= (WHACK | SLASH | PIERCE);
		}

		/* get resistmask */
		if (resist_blunt(pd) || (!youdef && (mdef->mfaction == ZOMBIFIED))){
			resistmask |= WHACK;
		}
		if (resist_pierce(pd) || (!youdef && (mdef->mfaction == ZOMBIFIED || mdef->mfaction == SKELIFIED || mdef->mfaction == CRYSTALFIED))){
			resistmask |= PIERCE;
		}
		if (resist_slash(pd) || (!youdef && (mdef->mfaction == SKELIFIED || mdef->mfaction == CRYSTALFIED))){
			resistmask |= SLASH;
		}

		if ((thick_skinned(pd) || (youdef && u.sealsActive&SEAL_ECHIDNA)) && (
			(unarmed_kick && !(otmp && (otmp->otyp == STILETTOS || otmp->otyp == HEELED_BOOTS || otmp->otyp == KICKING_BOOTS))) || 
			(otmp && (valid_weapon_attack || invalid_weapon_attack) && (otmp->obj_material <= LEATHER) && !arti_shining(otmp)) ||
			(otmp && (valid_weapon_attack || invalid_weapon_attack) && (otmp->oproperties&OPROP_FLAYW))
			)){
			/* damage entirely mitigated */
			subtotl = 0;
			resisted_attack_type = TRUE;
		}
		if ((attackmask & ~(resistmask)) == 0L && !(otmp && narrow_spec_applies(otmp, mdef)) && (subtotl > 0)) {
			/* damage reduced by 75% */
			subtotl /= 4;
			resisted_attack_type = TRUE;
			/* can only reduce damage to 1 */
			if (subtotl < 1)
				subtotl = 1;
		}
		else if (subtotl > 0 && vulnerable_mask(resistmask) && !(attackmask & EXPLOSION)) {
			/* 2x damage for attacking a vulnerability */
			subtotl *= 2;
		}
	}

	/* Half Physical Damage -- does not stack with damage-type resistance */
	if (Half_phys(mdef) && !resisted_attack_type && (subtotl > 0)) {
		subtotl /= 2;
		/* can only reduce damage to 1 */
		if (subtotl < 1)
			subtotl = 1;
	}

	/* thalassophobic players can deal greatly reduced damage to sea creatures */
	if (youagr && is_aquatic(pd) && roll_madness(MAD_THALASSOPHOBIA)){
		subtotl = (subtotl + 9)/10;
	}
	
	/* Apply DR */
	if (subtotl > 0){
		if (phase_armor){
			subtotl -= (youdef ? (base_udr() + base_nat_udr()) : (base_mdr(mdef) + base_nat_mdr(mdef)));
		}
		else {
			subtotl -= (youdef ? roll_udr(magr) : roll_mdr(mdef, magr));
		}
		/* can only reduce damage to 1 */
		if (subtotl < 1)
			subtotl = 1;
	}
	/* hack to enhance mm_aggression(); we don't want purple
	worm's bite attack to kill a shrieker because then it
	won't swallow the corpse; but if the target survives,
	the subsequent engulf attack should accomplish that */
	if (!youdef && !youagr &&
		pa == &mons[PM_PURPLE_WORM] &&
		pd == &mons[PM_SHRIEKER] &&
		(subtotl > *hp(mdef)))
		subtotl = max(*hp(mdef) - 1, 1);

	/* Final sum of damage */
	totldmg = subtotl + seardmg + heatdmg + poisdmg + specdmg;
	lethaldamage = (totldmg >= *hp(mdef));

	if (wizard && ublindf && ublindf->otyp == LENSES) {
		pline("dmg = (%d + %d + %d + %d + %d - defense) = %d; + %d = %d",
			basedmg,
			artidmg,
			bonsdmg,
			((youagr && !natural_strike) ? 0 : monsdmg),	/* only add monsdmg for monsters or a player making a monster attack */
			snekdmg + jostdmg,
			subtotl,
			seardmg + heatdmg + poisdmg + specdmg,
			totldmg
			);
	}
	
	/* PRINT HIT MESSAGE. MAYBE. */
	if (dohitmsg && vis){
		if (thrown && !hittxt)
		{
			if (youdef) {
				if (!Blind) {
					pline("%s %s you!", The(mshot_xname(weapon)), vtense(mshot_xname(weapon), "hit"));
				}
				else {
					pline("You are hit!");
				}
			}
			else if ((!lethaldamage || (m_shot.n > 1 && m_shot.o == weapon->otyp))
				) {
				if ((!cansee(bhitpos.x, bhitpos.y) && !canspotmon(mdef) &&
					!(u.uswallow && mdef == u.ustuck))
					|| !flags.verbose)
					pline("%s %s it.", The(mshot_xname(weapon)), vtense(mshot_xname(weapon), "hit"));
				else pline("%s %s %s%s", The(mshot_xname(weapon)), vtense(mshot_xname(weapon), "hit"),
					mon_nam(mdef), exclam(totldmg));
			}
		}
		else {
			if (!hittxt &&
				!jousting &&
				!staggering_strike &&
				!(youagr && lethaldamage) &&
				!(youagr && sneak_attack))
			{
				xyhitmsg(magr, mdef, attk);
			}
		}
	}

	/* Print additional messages and do more effects */
	/*
	 * Additional Messages
	 *  - juyo sneak attack
	 *  - sneak attack (suppress if blow will kill)
	 *  - jousting
	 *  - shattering strike (suppress if blow will kill)
	 *  - staggering strike (suppress if blow will kill)
	 *  - type-resistance (suppress if blow will kill)
	 *  - weapon-resistance (suppress if blow will kill)
	 *  - clockwork heat
	 *  - iron/silver/holy/unholy hating
	 *  - poison (if vs player, NOW call poisoned() since it will print messages)
	 *  - sword of blood
	 */

	/* sneak attack messages only if the player is attacking */
	if (youagr) {
		if (sneak_attack & SNEAK_JUYO) {
			/* always message, because... */
			if (stationary(pd) || sessile(pd))		You("rain blows on the immobile %s%s", l_monnam(mdef), exclam(subtotl));
			else if (sneak_attack & SNEAK_BEHIND)	You("rain blows on %s from behind%s", mon_nam(mdef), exclam(subtotl));
			else if (sneak_attack & SNEAK_BLINDED)	You("rain blows on the blinded %s%s", l_monnam(mdef), exclam(subtotl));
			else if (sneak_attack & SNEAK_TRAPPED)	You("rain blows on the trapped %s%s", l_monnam(mdef), exclam(subtotl));
			else if (sneak_attack & SNEAK_HELPLESS) You("rain blows on the helpless %s%s", l_monnam(mdef), exclam(subtotl));
			else									You("rain blows on %s%s", mon_nam(mdef), exclam(subtotl));
			/* ...player gets bonus movement points! */
			switch (min(P_SKILL(FFORM_JUYO), P_SKILL(weapon_type(weapon)))){
			case P_BASIC:	youmonst.movement += NORMAL_SPEED / 4;	break;
			case P_SKILLED:	youmonst.movement += NORMAL_SPEED / 3;	break;
			case P_EXPERT:	youmonst.movement += NORMAL_SPEED / 2;	break;
			}
		}
		else if (sneak_attack) {
			/* don't message if the attack is lethal, or if the attack dealt no damage (excluding X-hating/poison/etc) */
			if (!lethaldamage && (subtotl > 0)) {
				if (sneak_attack & SNEAK_HIDDEN)		You("%s the flat-footed %s%s",
					(distmin(x(magr), y(magr), x(mdef), y(mdef)) > BOLT_LIM) ? "snipe" : "strike", l_monnam(mdef), exclam(subtotl));
				else if (sneak_attack & SNEAK_BEHIND)	You("strike %s from behind%s", mon_nam(mdef), exclam(subtotl));
				else if (sneak_attack & SNEAK_BLINDED)	You("strike the blinded %s%s", l_monnam(mdef), exclam(subtotl));
				else if (sneak_attack & SNEAK_TRAPPED)	You("strike the trapped %s%s", l_monnam(mdef), exclam(subtotl));
				else if (sneak_attack & SNEAK_HELPLESS)	You("strike the helpless %s%s", l_monnam(mdef), exclam(subtotl));
				else									You("strike %s%s", mon_nam(mdef), exclam(subtotl));
			}
		}
	}
	/* jousting -- message, break lance */
	if (jousting) {
		/* message */
		if (youagr) {
			You("joust %s%s",
				mon_nam(mdef), canseemon(mdef) ? exclam(subtotl) : ".");
			if (jousting < 0) {
				Your("%s shatters on impact!", xname(weapon));
				destroy_one_magr_weapon = TRUE;
				/* (must be either primary or secondary weapon to get here) */
				u.twoweap = FALSE;	/* untwoweapon() is too verbose here */
				if (weapon == uwep)
					uwepgone();		/* set unweapon */
				if (weapon == uswapwep)
					uswapwepgone();	/* set unweapon */

				/* useup() will be called later */
			}
		}
		else {
			impossible("Monsters jousting? Someone forgot to fully implement this...");
		}
	}
	/* shattering strike -- attempt to destroy the defender's weapon */
	if (shattering_strike && !lethaldamage) {
		if (youagr) {
			otmp = MON_WEP(mdef);
			if (otmp &&
				!is_flimsy(otmp) &&
				!obj_resists(otmp, 0, 100)
				) {
				setmnotwielded(mdef, otmp);
				MON_NOWEP(mdef);
				mdef->weapon_check = NEED_WEAPON;
				if (is_lightsaber(weapon)) Your("energy blade slices %s %s in two!",
					s_suffix(mon_nam(mdef)), xname(otmp));
				else pline("%s %s %s from the force of your blow!",
					s_suffix(Monnam(mdef)), xname(otmp),
					otense(otmp, "shatter"));
				m_useup(mdef, otmp);
				/* If someone just shattered MY weapon, I'd flee! */
				if (rn2(4) && !mindless_mon(mdef)) {
					monflee(mdef, d(2, 3), TRUE, TRUE);
				}
			}
		}
		else {
			impossible("Monsters using shattering strikes? Someone forgot to fully implement this...");
		}
	}
	/* staggering strike */
	if (staggering_strike && !lethaldamage) {
		if (youagr) {
			if (canspotmon(mdef)) {
				pline("%s %s from your powerful %s%s!",
					Monnam(mdef),
					makeplural(stagger(mdef, "stagger")),
					((weapon && weapon->oartifact == ART_GREEN_DRAGON_CRESCENT_BLAD) ? "blow" : "strike"),
					((sneak_attack & SNEAK_JUYO) ? "s" : "")
					);
			}
		}
		else {
			impossible("Monsters using staggering strikes? Someone forgot to fully implement this...");
		}
	}
	/* disarming strike */
	if (disarming_strike && !lethaldamage) {
		if (youagr) {
			otmp = MON_WEP(mdef);
			if (otmp &&
				otmp->oartifact != ART_GLAMDRING &&
				!(otmp->cursed && !is_weldproof_mon(mdef)))
			{
				if (otmp->quan > 1L){
					otmp = splitobj(otmp, 1L);
					obj_extract_self(otmp); //wornmask is cleared by splitobj
				}
				else{
					long unwornmask;
					if ((unwornmask = otmp->owornmask) != 0L) {
						mdef->misc_worn_check &= ~unwornmask;
					}
					setmnotwielded(mdef, otmp);
					MON_NOWEP(mdef);
					mdef->weapon_check = NEED_WEAPON;
					obj_extract_self(otmp);
					otmp->owornmask = 0L;
					update_mon_intrinsics(mdef, otmp, FALSE, FALSE);
				}
				pline("%s %s is snagged by your %s.",
					s_suffix(Monnam(mdef)), xname(otmp), xname(weapon));
				getdir((char *)0);
				if (u.dx || u.dy){
					You("toss it away.");
					projectile(&youmonst, otmp, (struct obj *)0, TRUE, u.ux, u.uy, u.dx, u.dy, 0, (int)((ACURRSTR) / 2 - otmp->owt / 40 + weapon->spe), FALSE, FALSE, FALSE);
				}
				else{
					You("drop it at your feet.");
					(void)dropy(otmp);
				}
				nomul(0, NULL);
				if (mdef->mhp <= 0) /* flung weapon killed monster */
					return MM_DEF_DIED;
			}
		}
		else {
			impossible("Monsters using disarming strikes? Someone forgot to fully implement this...");
		}
	}

	/* damage resistance -- only print messages for the player, and only set/reset once per turn */
	if (youagr)
	{
		static long lastwarning = 0;

		if (lastwarning < moves) {
			if (warnedotyp != (otmp ? otmp->otyp : 0) || warnedptr != pd) {
				lastwarning = moves;
				/* warn the player that their attacks are futile */
				if (resisted_weapon_attacks || resisted_attack_type) {
					if (resisted_weapon_attacks) {
						pline("%s is resistant to attacks.",
							Monnam(mdef));
					}
					else if (valid_weapon_attack || invalid_weapon_attack) {
						pline("%s %s ineffective against %s.",
							The(xname(otmp)),
							(otmp->quan > 1L ? "are" : "is"),
							mon_nam(mdef)
							);
					}
					else if (unarmed_punch) {
						pline("Your %s are ineffective against %s.",
							makeplural(body_part(HAND)),
							mon_nam(mdef)
							);
					}
					else if (unarmed_kick) {
						pline("Your %s is ineffective against %s.",
							body_part(FOOT),
							mon_nam(mdef)
							);
					}
					warnedotyp = (otmp ? otmp->otyp : 0);
					warnedptr = pd;
				}
				else {
					/* reset notification */
					warnedotyp = -1;
					warnedptr = 0;
				}
			}
		}
	}

	/* fakewep: sword of blood message */
	if (swordofblood) {
		if (youdef) {
			pline("The blade of rotted blood tears through your %s!", body_part(BLOOD));
		}
		else if (vis&VIS_MDEF) {
			pline("The blade of rotted blood tears through %s %s!", s_suffix(mon_nam(mdef)), mbodypart(mdef, BLOOD));
		}
	}
	/* fakewep: sword of mercury message and inventory damage */
	if (attk && attk->adtyp == AD_MERC)
	{
		/* message */
		if (youdef) {
			pline("The metallic blade is covered in frost!");
			if (Cold_res(mdef)) {
				You("aren't cold!");
			}
		}
		else if (vis&VIS_MDEF) {
			if (!Cold_res(mdef)) {
				pline("The metallic blade freezes %s!",
					mon_nam(mdef));
			}
		}
		/* inventory damage */
		if (!InvCold_res(mdef)) {
			if (mlev(magr) > rn2(20))
				destroy_item2(mdef, POTION_CLASS, AD_COLD, youdef);
		}
	}

	/* Searing messages */
	if ((silverobj || jadeobj || ironobj || holyobj || unholyobj || otherobj) && (youdef || canseemon(mdef)) && !recursed) {
		long active_slots = (silverobj | jadeobj | ironobj | holyobj | unholyobj | otherobj);
		char buf[BUFSZ];
		char * obuf;
		/* Examples:
		 * Your silver skin and blessed silver ring sear Foo's flesh!
		 * Bar's white-burning blade sears Foo!
		 * Foo's cold-iron long sword sears your flesh!
		 * Foo's cold-iron quarterstaff sears your flesh!
		 * Your cursed gloves sear Foo's flesh!
		 * Your silver small silver quarterstaff sears Foo's flesh! >_>
		 */
		/* Attacker */
		Sprintf(buf, "%s ", (magr ? (youagr ? "Your" : s_suffix(Monnam(magr))) : "The"));
		/* skin / simurgh claws */
		slot = W_SKIN;
		if (active_slots & slot) {
			if (holyobj & slot)
				Sprintf(buf, "%sglorious ", buf);
			if (unholyobj & slot)
				Sprintf(buf, "%scursed ", buf);
			/* special cases */
			if (attk && attk->adtyp == AD_STAR)
			{
				Sprintf(buf, "%sstarlight rapier", buf);
			}
			else if (attk && attk->adtyp == AD_GLSS)
			{
				Sprintf(buf, "%sshards of broken mirrors", buf);
			}
			else if (youagr && u.sealsActive&SEAL_SIMURGH && u.sealsActive&SEAL_EDEN)
			{
				Sprintf(buf, "%scold-iron claws and silver skin", buf);
			}
			else {
				if (silverobj & slot)
					Sprintf(buf, "%ssilver ", buf);
				if (jadeobj & slot)
					Sprintf(buf, "%sjade ", buf);
				if (ironobj & slot)
					Sprintf(buf, "%scold-iron ", buf);
				Sprintf(buf, "%s%s", buf,
					(youagr && u.sealsActive&SEAL_SIMURGH) ? "claws"
					: (youagr ? body_part(BODY_SKIN) : mbodypart(magr, BODY_SKIN)));
			}
		}
		/* weapon */
		slot = W_WEP;
		if (active_slots & slot) {
			otmp = weapon;
			obuf = xname(otmp);

			if (active_slots & W_SKIN)
				Sprintf(buf, "%s and ", buf);

			if (holyobj & slot)
				Sprintf(buf, "%s%s", buf,
				(otmp->known && (otmp->oproperties&OPROP_HOLYW || otmp->oproperties&OPROP_HOLY)) ? "holy " : "blessed "
				);
			if (unholyobj & slot)
				Sprintf(buf, "%s%s", buf,
				(otmp->known && (otmp->oproperties&OPROP_UNHYW || otmp->oproperties&OPROP_UNHY)) ? "unholy " : "cursed "
				);
			/* special cases */
			if (otmp->oartifact == ART_SUNSWORD && (silverobj&slot)) {
				Sprintf(buf, "%sburning-white blade", buf);
			}
			else if (otmp->oartifact == ART_GLAMDRING && (silverobj&slot)) {
				Sprintf(buf, "%swhite-burning blade", buf);
			}
			else {
				if (silverobj & slot){
					if (!strstri(obuf, "silver "))
						Sprintf(buf, "%ssilver%s ", buf, (otmp->obj_material != SILVER ? "ed" : ""));
				}
				if (jadeobj & slot) {
					if (!strstri(obuf, "jade "))
						Sprintf(buf, "%sjade ", buf);
				}
				if (ironobj & slot) {
					if (!strncmpi(obuf, "iron ", 5))
						Sprintf(buf, "%scold-", buf);
					else if (!strstri(obuf, "iron "))
						Sprintf(buf, "%scold-iron ", buf);
				}
				if (otherobj & slot) {
					if (otmp->obj_material == WOOD && otmp->otyp != MOON_AXE &&
						(otmp->oward&WARD_VEIOISTAFUR) && pd->mlet == S_EEL &&
						(u.wardsknown&WARD_VEIOISTAFUR))		/* you need to know the ward to recognize it */
					{
						/* only will specifically modify "carved" */
						if (!strncmpi(obuf, "carved ", 7))
							Sprintf(buf, "%sveioistafur-", buf);
					}
						
				}
				Sprintf(buf, "%s%s", buf, xname(otmp));
			}
		}
		/* rings -- don't use xname(); "ring" is fine. */
		slot = rslot;
		if (active_slots & slot) {
			if (active_slots & (W_SKIN|W_WEP))
				Sprintf(buf, "%s and ", buf);
			/* only the player wears rings */
			/* get correct ring */
			otmp = (rslot == W_RINGL) ? uleft
				: (rslot == W_RINGR) ? uright
				: (struct obj *)0;

			if (otmp)
			{
				if (holyobj & slot)
					Sprintf(buf, "%s%s", buf,
					(otmp->known && (otmp->oproperties&OPROP_HOLYW || otmp->oproperties&OPROP_HOLY)) ? "holy " : "blessed "
					);
				if (unholyobj & slot)
					Sprintf(buf, "%s%s", buf,
					(otmp->known && (otmp->oproperties&OPROP_UNHYW || otmp->oproperties&OPROP_UNHY)) ? "unholy " : "cursed "
					);
				if (silverobj & slot){
					Sprintf(buf, "%ssilver%s ", buf, ((jadeobj&slot) || (ironobj&slot) ? "ed" : ""));
				}
				if (jadeobj & slot) {
					Sprintf(buf, "%sjade ", buf);
				}
				if (ironobj & slot) {
					Sprintf(buf, "%scold-iron ", buf);
				}

				Sprintf(buf, "%sring", buf);
			}
			else {
				active_slots &= ~rslot;
			}
		}

		/* gloves/boots/helmet -- assumes only one of the three will be used. */
		slot = attk_equip_slot(attk ? attk->aatyp : 0L);
		switch (slot)
		{
		case W_ARMG:
			otmp = (youagr ? uarmg : which_armor(magr, slot));
			break;
		case W_ARMF:
			otmp = (youagr ? uarmf : which_armor(magr, slot));
			break;
		case W_ARMH:
			otmp = (youagr ? uarmh : which_armor(magr, slot));
			break;
		default:
			otmp = (struct obj *)0;
			break;
		}
		if (otmp && (active_slots & slot)) {
			obuf = xname(otmp);
			if (active_slots & (W_SKIN|W_WEP|rslot))
				Sprintf(buf, "%s and ", buf);

			if (holyobj & slot)
				Sprintf(buf, "%s%s", buf,
				(otmp->known && (otmp->oproperties&OPROP_HOLYW || otmp->oproperties&OPROP_HOLY)) ? "holy " : "blessed "
				);
			if (unholyobj & slot)
				Sprintf(buf, "%s%s", buf,
				(otmp->known && (otmp->oproperties&OPROP_UNHYW || otmp->oproperties&OPROP_UNHY)) ? "unholy " : "cursed "
				);
			if (silverobj & slot){
				if (!strstri(obuf, "silver "))
					Sprintf(buf, "%ssilver%s ", buf, (otmp->obj_material != SILVER ? "ed" : ""));
			}
			if (jadeobj & slot) {
				if (!strstri(obuf, "jade "))
					Sprintf(buf, "%sjade ", buf);
			}
			if (ironobj & slot) {
				if (!strncmpi(obuf, "iron ", 5))
					Sprintf(buf, "%scold-", buf);
				else if (!strstri(obuf, "iron "))
					Sprintf(buf, "%scold-iron ", buf);
			}
			Sprintf(buf, "%s%s", buf, obuf);
		}
		/* defender */
		char seared[BUFSZ];
		if (noncorporeal(pd)) {
			Sprintf(seared, "%s",
				youdef ? "you" : mon_nam(mdef));
		}
		else {
			Sprintf(seared, "%s %s",
				(youdef ? "your" : s_suffix(mon_nam(mdef))),
				(youdef ? body_part(BODY_FLESH) : mbodypart(mdef, BODY_FLESH))
				);
		}
		Sprintf(buf, "%s sears %s", buf, seared);

		pline("%s!", buf);
	}

	/* poison */
	if (poisons_resisted || poisons_majoreff || poisons_minoreff || poisons_wipedoff) {
		otmp = poisonedobj;
		if (youdef && attk) {/* SCOPECREEP: get below scopecreep working so that the player can be poisoned by a poisoned object that isn't from an attack hitting them */
			/* using poisoned() from mon.c */
			char buf[BUFSZ];
			Sprintf(buf, "%s %s", s_suffix(Monnam(magr)), mpoisons_subj(magr, attk));
			/* SCOPECREEP: the "fatal" field (30) is inconsistent. Preferrably, this should use the same major/minor effect system set up in this function */
			/* also, this spams messages when recursed */
			poisoned(buf, A_CON, pa->mname, 30, (poisons_majoreff | poisons_minoreff));
		}
		else {
			int i, n;
			/* poison resist messages -- should only appear once, as resistivity should be constant between hits */
			if (poisons_resisted && canseemon(mdef) && !recursed) {
				for (n = 0; n < NUM_POISONS; n++)
				{
					i = (1 << n);
					if (!(poisons_resisted & i))
						continue;
					switch (i)
					{
					case OPOISON_BASIC:
					case OPOISON_BLIND:
					case OPOISON_PARAL:
						pline_The("poison doesn't seem to affect %s.", mon_nam(mdef));
						break;
					case OPOISON_FILTH:
						pline_The("filth doesn't seem to affect %s.", mon_nam(mdef));
						break;
					case OPOISON_SLEEP:
						pline_The("drug doesn't seem to affect %s.", mon_nam(mdef));
						break;
					case OPOISON_AMNES:
						pline_The("lethe-rust doesn't seem to affect %s.", mon_nam(mdef));
						break;
					case OPOISON_ACID:
						pline_The("acid-coating doesn't seem to affect %s.", mon_nam(mdef));
						break;
					case OPOISON_SILVER:
						/* no message */
						break;
					}
				}
			}
			/* poison major effects and their messages -- can happen multiple times */
			for (n = 0; n < NUM_POISONS; n++)
			{
				i = (1 << n);
				if (!(poisons_majoreff & i))
					continue;
				switch (i)
				{
				case OPOISON_BASIC:
					if (canseemon(mdef) && lethaldamage)
						pline_The("poison was deadly...");
					break;
				case OPOISON_FILTH:
					if (canseemon(mdef) && lethaldamage)
						pline_The("tainted filth was deadly...");
					break;
				case OPOISON_SLEEP:
					if (sleep_monst(mdef, rnd(12), POTION_CLASS)) {
						if (canseemon(mdef))
							pline("%s falls asleep.", Monnam(mdef));
						slept_monst(mdef);
					}
					break;
				case OPOISON_BLIND:
					{
					if (canseemon(mdef) && !is_blind(mdef))
						pline("It seems %s has gone blind!", mon_nam(mdef));

					register int btmp = 64 + rn2(32) +
						rn2(32) * !resist(mdef, POTION_CLASS, 0, NOTELL);
					btmp += mdef->mblinded;
					mdef->mblinded = min(btmp, 127);
					mdef->mcansee = 0;
					}
					break;
				case OPOISON_PARAL:
					if (canseemon(mdef) && mdef->mcanmove)
						pline("%s stops moving!", Monnam(mdef));
					if (mdef->mcanmove) {
						mdef->mcanmove = 0;
						mdef->mfrozen = rnd(25);
					}
					break;
				case OPOISON_AMNES:
					if (canseemon(mdef) && (mdef->mtame || !mdef->mpeaceful))
						pline("%s looks around as if awakening from a dream.", Monnam(mdef));
					mdef->mtame = FALSE;
					mdef->mpeaceful = TRUE;
					break;
				case OPOISON_ACID:
				case OPOISON_SILVER:
					/* no message, no additional effects */
					break;
				}
			}
		}
		/* poisons wiped off */
		if (otmp && poisons_wipedoff) {
			/* rings subtract from corpsenm */
			if (otmp->oclass == RING_CLASS) {
				if (otmp->opoisonchrgs-- <= 0)
					otmp->opoisoned = OPOISON_NONE;
			}
			/* viperwhips also subtract from corpsenm, but do so with a message */
			else if (otmp->otyp == VIPERWHIP && otmp->opoisonchrgs) {
				if (youagr) {
					pline("Poison from the internal reservoir coats the fangs of your %s.", xname(otmp));
				}
				otmp->opoisonchrgs--;
			}
			/* normal poisoned object behaviour */
			else {
				if (otmp->quan > 1){
					struct obj *unpoisd = splitobj(otmp, 1L);
					unpoisd->opoisoned &= ~(poisons_wipedoff);
					if (youagr) {
						pline("The coating on your %s has worn off.", xname(unpoisd));
						obj_extract_self(unpoisd);	/* free from inv */
						/* shouldn't merge */
						unpoisd = hold_another_object(unpoisd, "You drop %s!",
							doname(unpoisd), (const char *)0);
					}
					else {
						obj_extract_self(unpoisd);
						mpickobj(magr, unpoisd);
					}
				}
				else {
					if (youagr) {
						pline("The coating on your %s has worn off.", xname(otmp));
					}
					otmp->opoisoned &= ~(poisons_wipedoff);
				}
			}
		}
	}

	/* silently handle destroyed weapons */
	if (destroy_one_magr_weapon || destroy_all_magr_weapon) {
		boolean deallocweapon = (weapon->quan == 1L || destroy_all_magr_weapon);

		if (youagr) {
			if (deallocweapon) {
				if (weapon == uwep)
					uwepgone();
				else if (weapon == uswapwep)
					uswapwepgone();
			}
			if (weapon->where == OBJ_FREE)
				obfree(weapon, (struct obj *)0);
			else if (weapon->where == OBJ_INVENT) {
				if (destroy_all_magr_weapon)
					useupall(weapon);
				else
					useup(weapon);
			}
			else
				impossible("used up weapon is not in player's inventory or free (%d)", weapon->where);
		}
		else {
			if (weapon->where == OBJ_FREE)
				obfree(weapon, (struct obj *)0);
			else if (weapon->where == OBJ_MINVENT)
			{
				if (destroy_all_magr_weapon)
				{
					m_freeinv(weapon);
					obfree(weapon, (struct obj *)0);
				}
				else
					m_useup(magr, weapon);
			}
			else
				impossible("used up weapon is not in monster's inventory or free (%d)", weapon->where);
		}
		if (deallocweapon) {
			if (wepgone)
				*wepgone = TRUE;
			weapon = (struct obj *)0;
		}
	}
	
	/* Deal Damage */
	/* this can possibly kill, returning immediately */
	result = xdamagey(magr, mdef, attk, totldmg, FALSE);
	if (result&(MM_DEF_DIED|MM_DEF_LSVD))
		return result;

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	/* ALL EFFECTS AFTER THIS POINT REQUIRE THE DEFENDER TO SURVIVE THE ATTACK (not including lifesaving) */
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* apply confuse monster (player only) */
	if (youagr && u.umconf && !mdef->mconf) {
		char *hands = makeplural(body_part(HAND));
		/* message, reduce your hand glowiness */
		if (u.umconf == 1) {
			if (Blind)
				Your("%s stop tingling.", hands);
			else
				Your("%s stop glowing %s.", hands, hcolor(NH_RED));
		}
		else {
			if (Blind)
				pline_The("tingling in your %s lessens.", hands);
			else
				Your("%s no longer glow so brightly %s.", hands,
				hcolor(NH_RED));
		}
		u.umconf--;
		/* apply the confusion */
		if (!resist(mdef, SPBOOK_CLASS, 0, NOTELL)) {
			mdef->mconf = 1;
			if (!mdef->mstun && mdef->mcanmove && !mdef->msleeping && canseemon(mdef))
				pline("%s appears confused.", Monnam(mdef));
		}
	}

	/* hurtle mdef (player-inflicted only for now, as long as staggering strikes and jousting are) */
	if (staggering_strike || jousting) {
		if (youdef) {
			hurtle(sgn(x(mdef)-x(magr)), sgn(y(mdef)-y(magr)), 1, TRUE);
			if (staggering_strike)
				make_stunned(HStun + rnd(10), TRUE);
		}
		else {
			mhurtle(mdef, sgn(x(mdef)-x(magr)), sgn(y(mdef)-y(magr)), 1);
			if (staggering_strike)
				mdef->mstun = TRUE;
			pd = mdef->data; /* in case of a polymorph trap */
		}
	}

	/* pudding division */
	if ((pd == &mons[PM_BLACK_PUDDING] || pd == &mons[PM_BROWN_PUDDING] || pd == &mons[PM_DARKNESS_GIVEN_HUNGER])
		&& weapon && (valid_weapon_attack || invalid_weapon_attack)
		&& weapon->obj_material == IRON
		&& !thrown && (youdef || !mdef->mcan)) {
		if (youdef) {
			if (totldmg > 1)
				exercise(A_STR, FALSE);
			if (cloneu())
				You("divide!");
		}
		else {
			if (clone_mon(mdef, 0, 0)) {
				if (vis&VIS_MDEF) {
					pline("%s divides!", Monnam(mdef));
				}
			}
		}
	}

	/* worm cutting -- assumes bhitpos is set! */
	if (isok(bhitpos.x, bhitpos.y) && m_at(bhitpos.x, bhitpos.y) == mdef) {
		cutworm(mdef, bhitpos.x, bhitpos.y, weapon);
	}

	/* uranium imp warping -- not implemented for the player defending */
	if (pd == &mons[PM_URANIUM_IMP] && !mdef->mcan && !youdef){
		/* avoid mysterious force message by not using tele_restrict() */
		if (canseemon(mdef)) {
			pline("%s %s reality!",
				Monnam(mdef),
				level.flags.noteleport ? "tries to warp" : "warps"
				);
		}
		if (!level.flags.noteleport) {
			coord cc;
			if (youdef) {
				tele();
			}
			else {
				rloc(magr, FALSE);
			}
			enexto(&cc, x(mdef), y(mdef), &mons[PM_URANIUM_IMP]);
			rloc_to(mdef, cc.x, cc.y);
			
			return MM_AGR_STOP;
		}
	}

	return result;
}

/* shadow_strike()
 *
 * The player attacks [mdef] with one AT_TUCH AD_SHDW attack.
 */
int
shadow_strike(mdef)
struct monst * mdef;
{
	static struct attack shadowblade = { AT_SRPR, AD_SHDW, 4, 8 };
	int tohitmod = 0;	/* necessary to call xmeleehity */

	if (mdef){
		return xmeleehity(&youmonst,
			mdef,
			&shadowblade,
			(struct obj *)0,
			((canseemon(mdef) ? VIS_MDEF : 0) | VIS_MAGR | VIS_NONE),
			tohitmod,
			FALSE);
	}
	else
		return MM_MISS;
}

/* weave_black_web()
 * 
 * The player makes a shadowblade attack against all adjacent enemies, except for mexclude
 */
void
weave_black_web(mexclude)
struct monst * mexclude;
{
	struct monst *mdef;	/* defender */
	int i;				/* loop counter */

	for (i = 0; i<8; i++){
		if (isok(u.ux + xdir[i], u.uy + ydir[i])){
			mdef = m_at(u.ux + xdir[i], u.uy + ydir[i]);
			if (mdef && !mdef->mpeaceful && mdef != mexclude){
				(void)shadow_strike(mdef);
			}
		}
	}
}

/* xpassivey()
 * 
 * [mdef] was hit by [magr] and is now counterattacking.
 * This may seem backwards in this function, but it keeps [magr] and [mdef] consistent
 *
 * Does not include AT_BOOM effects, which are handled in mon.c for monsters.
 * SCOPECREEP: handle AT_BOOM effects for players somewhere/somehow.
 */
int
xpassivey(magr, mdef, attk, weapon, vis, result, pd, endofchain)
struct monst * magr;		/* original attacker, being affected by the passive */
struct monst * mdef;		/* original defender, whose passive is being proced */
struct attack * attk;		/* attacker's attack */
struct obj * weapon;		/* attacker's weapon */
int vis;					/* visiblity of original attack */
int result;					/* if attack hit / def died / agr died / agr-def moved */
struct permonst * pd;		/* defender's pd; cannot use mdef->data when player rehumanizes */
boolean endofchain;			/* if the attacker has finished their attack chain */
{
	int i;					/* loop counter */
	int newres;
	int dmg;
	int indexnum = 0;
	int subout = 0;
	int tohitmod = 0;
	int res[3];
	long slot = 0L;
	struct obj * otmp;
	boolean usedmask = FALSE;		/* whether a message has been printed about a lillend using a mask to make passive attacks */
	struct attack * passive;
	struct attack alt_attk;
	struct attack prev_attk = noattack;
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	/* set permonst pointers */
	struct permonst * pa = youagr ? youracedata : magr->data;

	if (vis == -1)
		vis = getvis(magr, mdef, 0, 0);

	/* Lillends can use masks to counterattack (but only at the end of the chain) */
	if (!youdef && pd == &mons[PM_LILLEND] && !(mdef->mfaction == ZOMBIFIED || mdef->mfaction == SKELIFIED) && rn2(2) && endofchain){
		pd = find_mask(mdef);
		/* message moved to AFTER getting passive attack, to avoid printing a useless message 
		 * like "The lillend uses a rothe mask!" */
	}

	/* SIMILAR STRUCTURE TO XATTACKY */

	/* zero out res[] */
	res[0] = MM_MISS;
	res[1] = MM_MISS;
	res[2] = MM_MISS;
	do {
		/* cycle res[] -- it should be the results of the previous 2 attacks */
		res[2] = res[1];
		res[1] = res[0];
		res[0] = MM_MISS;
		/* get next attack */
		passive = getattk(mdef, res, &indexnum, &prev_attk, FALSE, &subout, &tohitmod);
		/* if we don't have a passive attack, continue */
		if (is_null_attk(passive) || passive->aatyp != AT_NONE)
			continue;

		/* Only make noise about the lillend mask if applicable */
		if (!youdef && mdef->data == &mons[PM_LILLEND] && passive != &noattack && pd != mdef->data && !Blind && canseemon(mdef) &&
			!usedmask) {
			usedmask = TRUE;
			pline("%s uses a %s mask!", Monnam(mdef), pd->mname);
		}
		/* do the passive */
		res[0] = xpassivehity(magr, mdef, passive, attk, weapon, vis, result, pd, endofchain);
		result |= res[0];

	} while (!(
		res[0] & MM_DEF_DIED ||		/* attacker died */
		res[0] & MM_DEF_LSVD ||		/* attacker lifesaved */
		res[0] & MM_AGR_DIED ||		/* defender died */
		res[0] & MM_AGR_STOP ||		/* defender stopped for some other reason */
		is_null_attk(passive)		/* no more attacks */
		));
	
	/* passives NOT from a creature's attacks */
	/* per-attack */
	if (!endofchain) {
		/* Iris unbinds on attacking a reflective creature */
		if (youagr && u.sealsActive&SEAL_IRIS &&
			!(result&MM_DEF_DIED) &&
			!Blind && canseemon(mdef) && !Invisible && !is_vampire(pa)
			&& mon_reflects(mdef, "You catch a glimpse of a stranger's reflection in %s %s.")
			){
			unbind(SEAL_IRIS, TRUE);
			result |= MM_AGR_STOP;	/* your attack was interrupted! */
		}
		/* Argent Sheen madness can make you stop at reflecting monsters */
		else if (youagr && !Blind && canseemon(mdef) && !Invisible
			&& !is_vampire(youracedata)
			&& roll_madness(MAD_ARGENT_SHEEN)
			&& mon_reflects(mdef, "You admire your reflection in %s %s.")
			){
			nomul(-1 * rnd(6), "posing in front of a mirror.");
		}
		/* Vampires biting a deadly creature die. */
		if ((result&MM_HIT) && attk->aatyp == AT_BITE && is_vampire(pa)) {
			if (youagr) {
				if (bite_monster(mdef))
					result |= MM_AGR_STOP;	/* attacker lifesaved */
			}
			else {
				if (is_deadly(pd)) {
					/* kill */
					newres = xdamagey(mdef, magr, &noattack, 9999, FALSE);
					if (newres&MM_DEF_DIED)
						result |= MM_AGR_DIED;	/* attacker died */
					if (newres&MM_DEF_LSVD)
						result |= MM_AGR_STOP;	/* attacker lifesaved */
				}
				else if (pd == &mons[PM_GREEN_SLIME] || pd == &mons[PM_FLUX_SLIME]) {
					/* slime it */
					if (!(Change_res(magr)
						|| pa == &mons[PM_GREEN_SLIME]
						|| pa == &mons[PM_FLUX_SLIME]
						|| is_rider(pa)
						|| resists_poly(pa))) {
						if (youdef) {
							Your("%s turns %s into slime.", body_part(BODY_FLESH), mon_nam(mdef));
							(void)newcham(mdef, &mons[PM_GREEN_SLIME], FALSE, FALSE);
						}
						else {
							(void)newcham(mdef, &mons[PM_GREEN_SLIME], FALSE, vis);
							mdef->mstrategy &= ~STRAT_WAITFORU;
						}
					}
				}
			}
		}

		/* lightsaber forms (per-attack) (player-only) */
		if (youdef && uwep && is_lightsaber(uwep) && litsaber(uwep) &&			/* player with a lightsaber in their main hand */
			(multi >= 0) &&														/* not paralyzed */
			distmin(x(magr), y(magr), x(mdef), y(mdef)) == 1 &&					/* in close quarters */
			!(result&MM_AGR_DIED) &&											/* attacker is still alive */
			(u.fightingForm == FFORM_DJEM_SO || u.fightingForm == FFORM_SORESU)	/* proper forms */
			){
			int chance = 0;

			/* determine chance of counterattacking */
			switch (u.fightingForm) {
			case FFORM_DJEM_SO:
				if (!uarm || is_light_armor(uarm) || is_medium_armor(uarm))
					chance = DjemSo_counterattack[(min(P_SKILL(FFORM_DJEM_SO), P_SKILL(weapon_type(uwep))) - 2)];
				break;
			case FFORM_SORESU:
				if (!uarm || is_light_armor(uarm) || is_medium_armor(uarm))
					chance = Soresu_counterattack[(min(P_SKILL(FFORM_SORESU), P_SKILL(weapon_type(uwep))) - 2)];
				break;
			}

			/* maybe make the counterattack */
			if (rn2(100) < chance) {
				int newvis = vis&VIS_NONE;
				if (vis&VIS_MAGR)
					newvis |= VIS_MDEF;
				if (vis&VIS_MDEF)
					newvis |= VIS_MAGR;

				You("counterattack!");
				use_skill(u.fightingForm, 1);
				
				newres = xmeleehity(mdef, magr, &basicattack, uwep, newvis, 0, FALSE);
				if (newres&MM_DEF_DIED)
					result |= MM_AGR_DIED;	/* attacker died */
				if (newres&MM_DEF_LSVD)
					result |= MM_AGR_STOP;	/* attacker lifesaved */
			}
		}
	}
	/* per round -- note cannot access attk or weapon */
	else {
		/* Gauntlets of the Divine Disciple's divine retribution -- player only */
		if (youdef && uarmg && uarmg->oartifact == ART_GAUNTLETS_OF_THE_DIVINE_DI && !rn2(4)){
			if (!Shock_res(magr)) {
				pline("A bolt of divine energy strikes %s from the heavens!", mon_nam(magr));
				newres = xdamagey(mdef, magr, passive, d(2, 12), FALSE);
				if (newres&MM_DEF_DIED)
					result |= MM_AGR_DIED;	/* attacker died */
				if (newres&MM_DEF_LSVD)
					result |= MM_AGR_STOP;	/* attacker lifesaved */
			}
		}
		/* Counterattacks */
		/* get defender's weapon */
		otmp = (youdef ? uwep : MON_WEP(mdef));
		if (otmp &&
			!(result&(MM_AGR_DIED | MM_DEF_DIED | MM_DEF_LSVD)) &&
			distmin(x(magr), y(magr), x(mdef), y(mdef)) == 1 &&
			!cantmove(mdef)) {
			int chance = 0;
			/* lightsaber forms (per-round) (player-only) */
			if (youdef &&
				is_lightsaber(otmp) && litsaber(otmp) &&
				u.fightingForm == FFORM_SHIEN &&
				(!uarm || is_light_armor(uarm)))
			{
				chance += Shien_counterattack[(min(P_SKILL(FFORM_SHIEN), P_SKILL(weapon_type(uwep))) - 2)];
			}
			/* Sansara Mirror */
			if (otmp->oartifact == ART_SANSARA_MIRROR) {
				chance += 50;
			}
			/* Pen of the Void */
			if (otmp->oartifact == ART_PEN_OF_THE_VOID &&
				otmp->ovar1&SEAL_EURYNOME) {
				chance += 10;
				if (quest_status.killed_nemesis && Role_if(PM_EXILE))
					chance += 10;
			}

			/* maybe make the counterattack */
			if (rn2(100) < chance) {
				struct attack * counter;
				int newvis = vis&VIS_NONE;
				if (vis&VIS_MAGR)
					newvis |= VIS_MDEF;
				if (vis&VIS_MDEF)
					newvis |= VIS_MAGR;

				/* get 1st weapon attack defender has */
				int subout2 = 0;
				int indexnum2 = 0;
				/* grab the first weapon attack mdef has, or else use a basic 1d4 attack */
				do {
					/* we'll ignore res[], tohitmod, and prev_attk, resusing them from earlier */
					counter = getattk(mdef, res, &indexnum2, &prev_attk, FALSE, &subout2, &tohitmod);
				} while (counter->aatyp != AT_WEAP && !is_null_attk(counter));
				if (is_null_attk(counter))
					counter = &basicattack;

				/* make the counterattack */
				if (youdef) {
					You("counterattack!");
				}
				else if (newvis&VIS_MAGR) {
					pline("%s counterattacks!", Monnam(mdef));
				}
				/* train lightsaber skill if applicable */
				if (youdef && u.fightingForm == FFORM_SHIEN)
					use_skill(u.fightingForm, 1);

				/* make the attack */
				newres = xmeleehity(mdef, magr, counter, otmp, newvis, 0, FALSE);
				if (newres&MM_DEF_DIED)
					result |= MM_AGR_DIED;	/* attacker died */
				if (newres&MM_DEF_LSVD)
					result |= MM_AGR_STOP;	/* attacker lifesaved */
			}
		}

		/* Eurynome (player-only) */
		if (youdef && u.sealsActive&SEAL_EURYNOME &&							/* player with Eurynome bound */
			(multi >= 0) &&														/* not paralyzed */
			distmin(x(magr), y(magr), x(mdef), y(mdef)) == 1 &&					/* in close quarters */
			!(result&MM_AGR_DIED)												/* attacker is still alive */
			){
			/* chance of counterattacking is 20% */
			/* maybe make the counterattack */
			if (rn2(100) < 20) {
				int newvis = vis&VIS_NONE;
				if (vis&VIS_MAGR)
					newvis |= VIS_MDEF;
				if (vis&VIS_MDEF)
					newvis |= VIS_MAGR;
				int i;

				You("counterattack!");

				/* counterattack with two unarmed strikes, regardless of free-hand-ed-ness */
				for (i = 0; i < 2; i++) {
					newres = xmeleehity(mdef, magr, &basicattack, (struct obj *)0, newvis, 0, FALSE);
					if (newres&MM_DEF_DIED)
						result |= MM_AGR_DIED;	/* attacker died */
					if (newres&MM_DEF_LSVD)
						result |= MM_AGR_STOP;	/* attacker lifesaved */
					/* if (original attacker) died, don't keep attacking */
					if (result&(MM_AGR_DIED|MM_AGR_STOP))
						break;
				}
			}
		}
	}
	return result;
}


int
xpassivehity(magr, mdef, passive, attk, weapon, vis, result, pd, endofchain)
struct monst * magr;		/* original attacker, being affected by the passive */
struct monst * mdef;		/* original defender, whose passive is being proced */
struct attack * passive;	/* defender's passive being used */
struct attack * attk;		/* attacker's attack */
struct obj * weapon;		/* attacker's weapon */
int vis;					/* visiblity of original attack */
int result;					/* if attack hit / def died / agr died / agr-def moved */
struct permonst * pd;		/* defender's pd; cannot use mdef->data when player rehumanizes */
boolean endofchain;			/* if the passive is occuring at the end of aggressor's attack chain */
{
	int newres;
	int dmg;
	long slot = 0L;
	struct monst * mtmp;
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	/* set permonst pointers */
	struct permonst * pa = youagr ? youracedata : magr->data;


	/* Get damage of passive */
	/* Note: not always used */
	if (passive->damn)
		dmg = d(passive->damn, passive->damd);
	else if (passive->damd)
		dmg = d(mlev(mdef) + 1, passive->damd);
	else
		dmg = 0;

	/* These passives specifically happen per attack */
	/* necessary for passives that interact with [attk] */
	/* Try not to have passives deal damage like this, because that adds up to a lot quickly. */
	if (!endofchain) {
		/* passives from statblock */
		if (passive != &noattack && result&MM_HIT) {
			switch (passive->adtyp)
			{
			case AD_STON:
				slot = attk_protection(attk->aatyp);
				/* Touching is fatal */
				if (touch_petrifies(pd) && !(Stone_res(magr))
					&& badtouch(magr, mdef, attk, weapon))
				{
					if (youagr) {
						if (poly_when_stoned(youracedata) && polymon(PM_STONE_GOLEM)) {
							/* polyd into a stone golem */
							result |= MM_AGR_STOP;
						}
						else {
							/* stoned */
							You("turn to stone...");
							done_in_by(mdef);
							result |= MM_AGR_DIED;
						}
					}
					else {
						if (poly_when_stoned(pa)) {
							mon_to_stone(magr);
							result |= MM_AGR_STOP;
						}
						else {
							if (vis&VIS_MAGR) {
								pline("%s turns to stone!", Monnam(magr));
							}
							stoned = 1;
							xkilled(magr, 0);
							if (*hp(magr) > 0)
								result |= MM_AGR_STOP;
							else
								result |= MM_AGR_DIED;
						}
					}
				}
				break;
			case AD_SLVR:
				/* Eden's silver body sears attackers */
				slot = attk_protection(attk->aatyp);
				if (hates_silver(pa)
					&& badtouch(magr, mdef, attk, weapon))
				{
					if (youdef) {
						Your("silver body sears %s!", mon_nam(magr));
					}
					else if (youagr || canseemon(magr)) {
						pline("%s %s seared by %s silver body!",
							(youagr ? "You" : Monnam(magr)),
							(youagr ? "are" : "is"),
							s_suffix(mon_nam(mdef))
							);
					}
					newres = xdamagey(mdef, magr, passive, dmg, FALSE);
					if (newres&MM_DEF_DIED)
						result |= MM_AGR_DIED;	/* attacker died */
					if (newres&MM_DEF_LSVD)
						result |= MM_AGR_STOP;	/* attacker lifesaved */
				}
				break;
			case AD_RUST:
				if (youdef || !mdef->mcan) {
					/* naiads have additional limitations */
					if (pd == &mons[PM_NAIAD]){
						if (((*hp(mdef) < *hpmax(mdef) / 2) && rn2(3)) ||
							result&MM_DEF_DIED) {
							if (youdef) {
								You("collapse into a puddle of water!");
								rehumanize();
							}
							else {
								if (canseemon(mdef))
									pline("%s collapses into a puddle of water!", Monnam(mdef));
								if (!(result&MM_DEF_DIED)) {
									newres = xdamagey(magr, mdef, passive, *hp(mdef), FALSE);
									if (newres&MM_DEF_DIED)
										result |= MM_DEF_DIED;	/* defender died */
									if (newres&MM_DEF_LSVD)
										result |= MM_AGR_STOP;	/* defender lifesaved */
								}
							}
							/* fall through, rusting weapon */
						}
						/* if the naiad didn't collapse into a puddle, no rusting happens */
						else break;
					}
					/* rust weapon */
					passive_obj2(magr, mdef, weapon, attk, passive);
				}
				break;

			case AD_CORR:
				if (youdef || !mdef->mcan) {
					/* corrode weapon */
					passive_obj2(magr, mdef, weapon, attk, passive);
				}
				break;

			case AD_ENCH:
				if (youdef || !mdef->mcan) {
					/* disenchant weapon */
					passive_obj2(magr, mdef, weapon, attk, passive);
				}
				break;
			case AD_EACD:
			case AD_ACID:
				/* possibly corrode weapon -- this is independent of the attacker itself being splashed with acid */
				if (!rn2(passive->adtyp == AD_EACD ? 2 : 6)) {
					/* call dedicated passive-affects-object function */
					passive_obj2(magr, mdef, weapon, attk, passive);
				}
				break;
			}
		}
	}
	/* these passives happen at the end of the attack round */
	/* cannot access attk or weapon */
	else {
		/* These passives happen, hit or miss, even if the defender died. */
		if (passive != &noattack) {
			switch (passive->adtyp)
			{
			case AD_MAGM:
				/* wrath of gods for attacking Oracle */
				if (Magic_res(magr)) {
					dmg = 0;
					if (youagr) {
						shieldeff(u.ux, u.uy);
						pline("A hail of magic missiles narrowly misses you!");
					}
					else if (canseemon(magr)) {
						shieldeff(x(magr), y(magr));
						pline("%s is missed by a hail of magic missiles.", Monnam(magr));
					}
				}
				else {
					if (youagr) {
						You("are hit by magic missiles appearing from thin air!");
					}
					else if (canseemon(magr)) {
						pline("%s is hit by a hail of magic missiles.", Monnam(magr));
					}
					newres = xdamagey(mdef, magr, passive, dmg, FALSE);
					if (newres&MM_DEF_DIED)
						result |= MM_AGR_DIED;	/* attacker died */
					if (newres&MM_DEF_LSVD)
						result |= MM_AGR_STOP;	/* attacker lifesaved */
				}
				break;

			case AD_SHDW:
				if (youagr) {
					pline("Its bladed shadow falls on you!");
				}
				newres = xdamagey(mdef, magr, passive, dmg, FALSE);
				if (newres&MM_DEF_DIED)
					result |= MM_AGR_DIED;	/* attacker died */
				if (newres&MM_DEF_LSVD)
					result |= MM_AGR_STOP;	/* attacker lifesaved */

				if (!rn2(8) && !result&(MM_AGR_DIED | MM_AGR_STOP))
				{
					if (youdef) {
						char buf[BUFSZ];
						Sprintf(buf, "%s shadow", s_suffix(Monnam(mdef)));
						poisoned(buf, A_STR, pd->mname, 30, 0);
					}
					else {
						if (!Poison_res(magr)) {
							newres = xdamagey(mdef, magr, passive, rn1(10, 6), FALSE);
							if (newres&MM_DEF_DIED)
								result |= MM_AGR_DIED;	/* attacker died */
							if (newres&MM_DEF_LSVD)
								result |= MM_AGR_STOP;	/* attacker lifesaved */
						}
					}
				}
				break;
			}
		}

		/* These passives happen even if the defender died, but require a hit */
		if ((result&MM_HIT) && passive != &noattack) {
			switch (passive->adtyp)
			{
			case AD_BARB:
				/* message */
				if (youagr) {
					if (pd == &mons[PM_RAZORVINE]) You("are hit by the springing vines!");
					else You("are hit by %s barbs!", s_suffix(mon_nam(mdef)));
				}
				else if (vis) {
					if (pd == &mons[PM_RAZORVINE]) {
						pline("%s is hit by %s springing vines!",
							Monnam(mdef),
							(youdef ? "your" : s_suffix(mon_nam(mdef)))
							);
					}
					else {
						pline("%s is hit by %s barbs!",
							Monnam(mdef),
							(youdef ? "your" : s_suffix(mon_nam(mdef)))
							);
					}
				}
				/* damage (reduced by DR, half-phys damage, min 1) */
				dmg -= (youagr ? roll_udr(mdef) : roll_mdr(magr, mdef));
				if (Half_phys(magr))
					dmg = (dmg + 1) / 2;
				if (dmg < 1)
					dmg = 1;

				newres = xdamagey(mdef, magr, &noattack, dmg, FALSE);
				if (newres&MM_DEF_DIED)
					result |= MM_AGR_DIED;	/* attacker died */
				if (newres&MM_DEF_LSVD)
					result |= MM_AGR_STOP;	/* attacker lifesaved */
				break;

			case AD_EACD:
			case AD_ACID:
				/* attacker is splashed 1/2 the time */
				if (rn2(2)) {
					/* maybe reduce damage (affects messages) */
					if (Acid_res(magr))
						dmg = (passive->adtyp == AD_EACD ? dmg / 2 : 0);
					/* message */
					if (youagr) {
						if (Blind || !flags.verbose) {
							You("are splashed%s",
								exclam(dmg)
								);
						}
						else {
							You("are %ssplashed by %s acid%s",
								(dmg == 0 ? "harmlessly " : ""),
								s_suffix(mon_nam(mdef)),
								exclam(dmg)
								);
						}
					}
					else if (vis) {
						pline("%s is %ssplashed by %s acid%s",
							Monnam(magr),
							(dmg == 0 ? "harmlessly " : ""),
							(youdef ? "your" : s_suffix(mon_nam(mdef))),
							exclam(dmg)
							);
					}
					/* damage */
					if (dmg) {
						newres = xdamagey(mdef, magr, passive, dmg, FALSE);
						if (newres&MM_DEF_DIED)
							result |= MM_AGR_DIED;	/* attacker died */
						if (newres&MM_DEF_LSVD)
							result |= MM_AGR_STOP;	/* attacker lifesaved */
					}
					/* potentially erode worn armor */
					if (!rn2(passive->adtyp == AD_EACD ? 6 : 30))
						erode_armor(magr, TRUE);
				}
				break;

			case AD_WEBS:
			{
				struct trap *ttmp2 = maketrap(x(mdef), y(mdef), WEB);
				if (ttmp2) {
					if (youdef) {
						pline_The("webbing sticks to you. You're caught!");
						dotrap(ttmp2, NOWEBMSG);
#ifdef STEED
						if (u.usteed && u.utrap) {
							/* you, not steed, are trapped */
							dismount_steed(DISMOUNT_FELL);
						}
#endif
					}
					else {
						mintrap(mdef);
					}
				}
			}
				break;
			case AD_HLBD:
				/* Legion gets many zombies and legionnaires */
				if (pd == &mons[PM_LEGION]) {
					int n = rnd(4);
					for (; n > 0; n--) {
						mtmp = (rn2(7) ? makemon(mkclass(S_ZOMBIE, G_NOHELL | G_HELL), x(mdef), y(mdef), NO_MINVENT | MM_ADJACENTOK | MM_ADJACENTSTRICT)
							: makemon(&mons[PM_LEGIONNAIRE], x(mdef), y(mdef), NO_MINVENT | MM_ADJACENTOK | MM_ADJACENTSTRICT));
						if (mtmp) {
							/* Legion's summons don't time out */
							/* Although this currently is impossible, we should handle tame/selfpolyd Legion */
							if (youdef || mdef->mtame) {
								initedog(mtmp);
							}
						}
					}
				}
				/* Others (Asmodeus, Verier) get Devils */
				else {
					if (*hp(mdef) > *hpmax(mdef) * 3 / 4)		mtmp = makemon(&mons[PM_LEMURE], x(mdef), y(mdef), MM_ADJACENTOK);
					else if (*hp(mdef) > *hpmax(mdef) * 2 / 4)	mtmp = makemon(&mons[PM_HORNED_DEVIL], x(mdef), y(mdef), MM_ADJACENTOK);
					else if (*hp(mdef) > *hpmax(mdef) * 1 / 4)	mtmp = makemon(&mons[PM_BARBED_DEVIL], x(mdef), y(mdef), MM_ADJACENTOK);
					else if (*hp(mdef) > *hpmax(mdef) * 0 / 4)	mtmp = makemon(&mons[PM_PIT_FIEND], x(mdef), y(mdef), MM_ADJACENTOK);
					if (mtmp) {
						/* Asmodeus's and Verier's summons don't time out */
						/* Although this currently is impossible, we should handle tame/selfpolyd Asmo/Verier */
						if (youdef || mdef->mtame) {
							initedog(mtmp);
						}
					}
				}
				break;
			case AD_OONA:
				/* */
				if (u.oonaenergy == AD_FIRE){
					if (rn2(2)) mtmp = makemon(&mons[PM_FLAMING_SPHERE], x(mdef), y(mdef), MM_ADJACENTOK);
					else		mtmp = makemon(&mons[PM_FIRE_VORTEX], x(mdef), y(mdef), MM_ADJACENTOK);
				}
				else if (u.oonaenergy == AD_COLD){
					if (rn2(2)) mtmp = makemon(&mons[PM_FREEZING_SPHERE], x(mdef), y(mdef), MM_ADJACENTOK);
					else		mtmp = makemon(&mons[PM_ICE_VORTEX], x(mdef), y(mdef), MM_ADJACENTOK);
				}
				else if (u.oonaenergy == AD_ELEC){
					if (rn2(2)) mtmp = makemon(&mons[PM_SHOCKING_SPHERE], x(mdef), y(mdef), MM_ADJACENTOK);
					else		mtmp = makemon(&mons[PM_ENERGY_VORTEX], x(mdef), y(mdef), MM_ADJACENTOK);
				}
				/* Oona's summons time out and vanish */
				if (mtmp) {
					mtmp->mvanishes = mlev(mdef) + rnd(mlev(mdef));
					/* can be tame */
					if (youdef || mdef->mtame) {
						initedog(mtmp);
					}
				}
				break;
			case AD_DISE:
				if (youdef || !mdef->mcan) {
					/* swamp nymphs have additional limitations, like naiads */
					if (pd == &mons[PM_SWAMP_NYMPH]){
						if (((*hp(mdef) < *hpmax(mdef) / 2) && rn2(3)) ||
							result&MM_DEF_DIED) {
							if (youdef) {
								You("collapse into a puddle of noxious fluid!");
								rehumanize();
							}
							else {
								if (canseemon(mdef))
									pline("%s collapses into a puddle of noxious fluid!", Monnam(mdef));
								if (!(result&MM_DEF_DIED)) {
									newres = xdamagey(magr, mdef, passive, *hp(mdef), FALSE);
									if (newres&MM_DEF_DIED)
										result |= MM_DEF_DIED;	/* defender died */
									if (newres&MM_DEF_LSVD)
										result |= MM_AGR_STOP;	/* defender lifesaved */
								}
							}
							/* fall through, diseasing attacker */
						}
						/* if the swamp nymph didn't collapse into a puddle, nothing happens */
						else break;
					}
					/* other creatures have other messages and conditions */
					else
					{
						if ((youdef && !rn2(4)) || !mdef->mspec_used) {
							if (!rn2(3) ||
								pd == &mons[PM_ZUGGTMOY]) {
								if (canseemon(mdef))
									pline("A cloud of spores is released!");
								if (!youdef)
									mdef->mspec_used = 1;
							}
							else break;
						}
					}
					/* if this point has been reached, do the disease */
					if (youagr) {
						diseasemu(pd);
					}
					else {
						if (!Sick_res(mdef))
						{
							if (canseemon(magr))
								pline("%s looks very sick!", Monnam(magr));
							*hp(magr) = (*hp(magr) + 1) / 2;
						}
					}
				}
				break;
			}

			/* These passives only happen if the defender lived, and also require a hit */
			if ((result&MM_HIT) && !(result&MM_DEF_DIED) && passive != &noattack &&
				rn2(3) && (youdef || !mdef->mcan)) {
				switch (passive->adtyp)
				{
				case AD_AXUS:
					/* reflection-piercing paralysis gaze */
					if (ward_at(x(magr), y(magr)) != HAMSA && !is_blind(mdef)) {
						if (youagr) {
							if (canseemon(mdef)) {
								if (Free_action)
									You("momentarily stiffen under %s gaze!",
									s_suffix(mon_nam(mdef)));
								else if (ureflects("%s gaze is only partially reflected by your %s!",
									s_suffix(Monnam(mdef)))){
									nomul(-dmg / 2, "frozen by the gaze of Axus");
								}
								else {
									You("are frozen by %s gaze!",
										s_suffix(mon_nam(mdef)));
									nomul(-dmg, "frozen by the gaze of Axus");
								}
							}
						}
						else {
							if (!is_blind(magr) && haseyes(pa) &&
								(mon_resistance(magr, SEE_INVIS) || !mdef->minvis)) {
								char buf[BUFSZ];
								Sprintf(buf, "%s gaze is only partially reflected by %%s %%s!",
									s_suffix(mon_nam(mdef)));
								if (mon_reflects(magr,
									canseemon(magr) ? buf : (char *)0)) {
									magr->mcanmove = 0;
									magr->mfrozen = dmg / 2;
								}
								else {
									if (canseemon(magr)) {
										pline("%s is frozen by %s gaze!",
											Monnam(magr), s_suffix(mon_nam(mdef)));
									}
									magr->mcanmove = 0;
									magr->mfrozen = dmg;
								}
							}
						}
					}
					/* anger autons */
					if (youagr) {
						struct monst * mtmp;
						int mndx;
						for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
							mndx = monsndx(mtmp->data);
							if (mndx <= PM_QUINON && mndx >= PM_MONOTON && mtmp->mpeaceful){
								pline("%s gets angry...", Amonnam(mtmp));
								mtmp->mpeaceful = 0;
								mtmp->mtame = 0;
							}
						}
					}
					break;
				case AD_PLYS:
					if (pd->mlet == S_EYE) {	/* assumed to be gaze */
						/* hamsa prevents gazes */
						if (ward_at(x(magr), y(magr)) == HAMSA)
							break;
						/* the eye can't be blinded */
						if (is_blind(mdef)) {
							if (youagr) {
								pline("%s cannot defend itself.",
									Adjmonnam(mdef, "blind"));
								if (!rn2(500))
									change_luck(-1);
							}
						}
						else {
							if (youagr) {
								if (canseemon(mdef)) {
									if (ureflects("%s gaze is reflected by your %s.",
										s_suffix(Monnam(mdef)))){
										/* ureflects() prints message */;
									}
									else if (Free_action) {
										You("momentarily stiffen under %s gaze!",
											s_suffix(mon_nam(mdef)));
									}
									else {
										You("are frozen by %s gaze!",
											s_suffix(mon_nam(mdef)));
										nomul(-dmg, "frozen by a monster's gaze");
									}
								}
							}
							else {
								if (!is_blind(magr) && haseyes(pa) &&
									(mon_resistance(magr, SEE_INVIS) || !mdef->minvis)) {
									char buf[BUFSZ];
									Sprintf(buf, "%s gaze is reflected by %%s %%s.",
										s_suffix(mon_nam(mdef)));
									if (mon_reflects(magr,
										canseemon(magr) ? buf : (char *)0)) {
										/* mon_reflects() prints message */;
									}
									else if (mon_resistance(magr, FREE_ACTION)) {
										if (canseemon(magr)) {
											pline("%s momentarily stiffens.",
												Monnam(magr));
										}
									}
									else {
										if (canseemon(magr)) {
											pline("%s is frozen by %s gaze!",
												Monnam(magr), s_suffix(mon_nam(mdef)));
										}
										magr->mcanmove = 0;
										magr->mfrozen = dmg;
									}
								}
							}
						}
					}
					/* not eyes/gazes, assumed to be touch */
					else {

						if (youagr) {
							if (Free_action) {
								You("momentarily stiffen.");
							}
							else {
								You("are frozen by %s!", mon_nam(mdef));
								nomovemsg = 0;	/* default: "you can move again" */
								nomul(-dmg, "frozen by a monster");
								exercise(A_DEX, FALSE);
							}
						}
						else {
							if (mon_resistance(magr, FREE_ACTION)) {
								if (canseemon(magr)) {
									pline("%s momentarily stiffens.",
										Monnam(magr));
								}
							}
							else {
								if (canseemon(magr)) {
									pline("%s is frozen by %s!",
										Monnam(magr), mon_nam(mdef));
								}
								magr->mcanmove = 0;
								magr->mfrozen = dmg;
							}
						}
					}
					break;
				case AD_STUN:
					if (youdef) {
						if (!Stunned)
							make_stunned((long)dmg, TRUE);
					}
					else {
						if (canseemon(magr)){
							pline("%s %s",
								Monnam(magr),
								makeplural(stagger(magr, "stagger"))
								);
						}
						magr->mstun = 1;
					}
					break;
				case AD_COLD:
					/* resistance */
					if (Cold_res(magr)) {
						if (youagr) {
							shieldeff(u.ux, u.uy);
							You_feel("a mild chill.");
							ugolemeffects(AD_COLD, dmg);
						}
						else {
							golemeffects(magr, AD_COLD, dmg);
						}
					}
					/* otherwise, damage */
					else {
						if (youagr) {
							You("are suddenly very cold!");
							roll_frigophobia();
						}
						else if canseemon(magr) {
							pline("%s is suddenly very cold!",
								Monnam(magr));
						}
						newres = xdamagey(mdef, magr, passive, dmg, FALSE);
						if (newres&MM_DEF_DIED)
							result |= MM_AGR_DIED;	/* attacker died */
						if (newres&MM_DEF_LSVD)
							result |= MM_AGR_STOP;	/* attacker lifesaved */

						/* some monsters get stronger, and split! */
						if (pd == &mons[PM_BROWN_MOLD] ||
							pd == &mons[PM_BLUE_JELLY] ||
							pd == &mons[PM_ASPECT_OF_THE_SILENCE]
							) {
							*hp(mdef) += dmg / 2;
							if (*hpmax(mdef) < *hp(mdef))
								*hpmax(mdef) = *hp(mdef);
							/* at a certain point, the monster will reproduce! */
							if (*hpmax(mdef) >((int)(mlev(mdef) + 1) * 8)){
								(void)split_mon(mdef, magr);
							}
						}
					}
					break;
				case AD_FIRE:
					/* resistance */
					if (Fire_res(magr)) {
						if (youagr) {
							shieldeff(u.ux, u.uy);
							You_feel("mildly warm.");
							ugolemeffects(AD_FIRE, dmg);
						}
						else {
							golemeffects(magr, AD_FIRE, dmg);
						}
					}
					/* otherwise, damage */
					else {
						if (youagr) {
							You("are suddenly very hot!");
						}
						else if canseemon(magr) {
							pline("%s is suddenly very hot!",
								Monnam(magr));
						}
						newres = xdamagey(mdef, magr, passive, dmg, FALSE);
						if (newres&MM_DEF_DIED)
							result |= MM_AGR_DIED;	/* attacker died */
						if (newres&MM_DEF_LSVD)
							result |= MM_AGR_STOP;	/* attacker lifesaved */
					}
					break;
				case AD_ELEC:
					/* resistance */
					if (Shock_res(magr)) {
						if (youagr) {
							shieldeff(u.ux, u.uy);
							You_feel("a mild tingle.");
							ugolemeffects(AD_ELEC, dmg);
						}
						else {
							golemeffects(magr, AD_ELEC, dmg);
						}
					}
					/* otherwise, damage */
					else {
						if (youagr) {
							You("are jolted with electricity!");
						}
						else if canseemon(magr) {
							pline("%s is jolted with electricity!",
								Monnam(magr));
						}
						newres = xdamagey(mdef, magr, passive, dmg, FALSE);
						if (newres&MM_DEF_DIED)
							result |= MM_AGR_DIED;	/* attacker died */
						if (newres&MM_DEF_LSVD)
							result |= MM_AGR_STOP;	/* attacker lifesaved */
					}
					break;
				}
			}
		}
	}
	return result;
}

void
passive_obj2(magr, mdef, otmp, attk, passive)
struct monst * magr;		/* creature who was attacking first */
struct monst * mdef;		/* creature whose passive attack is being used */
struct obj * otmp;			/* object that is being affected by the passive attack */
struct attack * attk;		/* attack aggressor used, which is now being counterattacked against */
struct attack * passive;	/* specific passive attack being used */
{
	int i;

	/* if not given, grab passive attack from permonst */
	if (!passive || is_null_attk(passive))
	{
		/* we need a defender to do this */
		if (!mdef)
			return;

		struct permonst * pd = ((mdef == &youmonst) ? youracedata : mdef->data);
		passive = &noattack;
		for (i = 0;; i++) {
			if (pd->mattk[i].aatyp == AT_NONE &&
				!is_null_attk(&(pd->mattk[i]))) {
				passive = &pd->mattk[i];
				break;
			}
		}
		/* if we failed to get a passive attack, nothing happens. */
		if (passive == &noattack)
			return;
	}
	/* if not given, get otmp based on attk and magr */
	if (!otmp) {
		/* we need both magr and attk to do this */
		if (!magr || !attk || is_null_attk(attk))
			return;
		boolean youagr = (magr == &youmonst);
		long slot = attk_equip_slot(attk->aatyp);

		switch (slot)
		{
		case W_ARMG:
			otmp = (youagr ? uarmg : which_armor(magr, slot));
			break;
		case W_ARMF:
			otmp = (youagr ? uarmf : which_armor(magr, slot));
			break;
		case W_ARMH:
			otmp = (youagr ? uarmh : which_armor(magr, slot));
			break;
		default:
			otmp = (struct obj *)0;
			break;
		}
		/* if we failed to get an object, nothing happens. */
		if (!otmp)
			return;
	}

	switch (passive->adtyp)
	{
	case AD_EACD:
	case AD_ACID:
		if (!rn2(attk->adtyp == AD_EACD ? 2 : 6)) {
			erode_obj(otmp, TRUE, FALSE);
		}
		break;
	case AD_RUST:
		if (!mdef || !mdef->mcan) {
			erode_obj(otmp, FALSE, FALSE);
		}
		break;
	case AD_CORR:
		if (!mdef || !mdef->mcan) {
			erode_obj(otmp, TRUE, FALSE);
		}
		break;
	case AD_ENCH:
		if (!mdef || !mdef->mcan) {
			if (drain_item(otmp) && carried(otmp) &&
				(otmp->known || otmp->oclass == ARMOR_CLASS)) {
				/* carried() already assured us the player is involved */
				Your("%s less effective.", aobjnam(otmp, "seem"));
			}
			break;
		}
	default:
		break;
	}

	if (carried(otmp))
		update_inventory();
	return;
}

void
wakeup2(mdef, your_fault)
struct monst * mdef;
boolean your_fault;
{
	if (mdef == &youmonst)
	{
		if (u.usleep && u.usleep < monstermoves && rn2(20) < ACURR(A_WIS)) {
			multi = -1;
			nomovemsg = "The combat suddenly awakens you.";
		}
	}
	else
	{
		wakeup(mdef, your_fault);
	}
	return;
}

/* android_combo()
 *
 * Perform's the player's android weapon combo dependent on their mainhand weapon
 *
 * Returns FALSE if this was cancelled before doing anything.
 */
boolean
android_combo()
{
	struct monst * mdef;
	int vis;

	static struct attack weaponhit =	{ AT_WEAP, AD_PHYS, 0, 0 };
	static struct attack kickattack =	{ AT_KICK, AD_PHYS, 1, 2 };
	static struct attack finisher =		{ AT_CLAW, AD_PHYS,16, 8 };

	/* unarmed */
	if (!uwep){
		if (!getdir((char *)0))
			return FALSE;
		if (u.ustuck && u.uswallow)
			mdef = u.ustuck;
		else
			mdef = m_at(u.ux + u.dx, u.uy + u.dy);

		if (!mdef)
			You("swing wildly!");
		else {
			vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
			xmeleehity(&youmonst, mdef, &weaponhit, (struct obj *)0, vis, 0, FALSE);
			xmeleehity(&youmonst, mdef, &weaponhit, (struct obj *)0, vis, 0, FALSE);
		}
		u.uen--;
		if (P_SKILL(P_BARE_HANDED_COMBAT) >= P_SKILLED && u.uen > 0){
			if (dokick()){
				u.uen--;
			}
			else return TRUE;
		}
		if (P_SKILL(P_BARE_HANDED_COMBAT) >= P_EXPERT && u.uen > 0){
			int j = jump(1);
			int k = dokick();
			if (j || k){
				u.uen--;
			}
			else return TRUE;
		}
		if (P_SKILL(P_BARE_HANDED_COMBAT) >= P_MASTER && u.uen > 0){
			int j = jump(1);
			int d = getdir((char *)0);
			if (!j && !d) return TRUE;
			u.uen--;
			if (d){
				if (u.ustuck && u.uswallow)
					mdef = u.ustuck;
				else
					mdef = m_at(u.ux + u.dx, u.uy + u.dy);
				if (!mdef)
					You("swing wildly!");
				else {
					vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
					xmeleehity(&youmonst, mdef, &weaponhit,  (struct obj *)0, vis, 0, FALSE);
					xmeleehity(&youmonst, mdef, &weaponhit,  (struct obj *)0, vis, 0, FALSE);
					xmeleehity(&youmonst, mdef, &kickattack, (struct obj *)0, vis, 0, FALSE);
					xmeleehity(&youmonst, mdef, &kickattack, (struct obj *)0, vis, 0, FALSE);
				}
			}
		}
		if(P_SKILL(P_BARE_HANDED_COMBAT) >= P_GRAND_MASTER && u.uen > 0){
			if(!getdir((char *)0)) return 1;
			u.uen--;
			if (u.ustuck && u.uswallow)
				mdef = u.ustuck;
			else
				mdef = m_at(u.ux + u.dx, u.uy + u.dy);
			if (!mdef)
				You("swing wildly!");
			else {
				xmeleehity(&youmonst, mdef, &finisher,   (struct obj *)0, vis, 0, FALSE);
			}
		}
		return TRUE;
	}
	else if (is_lightsaber(uwep) && litsaber(uwep)){ //!uwep handled above
		/* get direction of attack */
		if (!getdir((char *)0))
			return FALSE;
		/* fast weapons give you speed */
		if (fast_weapon(uwep))
			youmonst.movement += 2;
		/* get defender */
		if (u.ustuck && u.uswallow)
			mdef = u.ustuck;
		else
			mdef = m_at(u.ux + u.dx, u.uy + u.dy);

		/* attack (melee twice OR throw lightsaber) */
		if (!mdef) {
			projectile(&youmonst, uwep, (struct obj *)0, FALSE, u.ux, u.uy, u.dx, u.dy, u.dz, 10, FALSE, TRUE, FALSE);
		}
		else {
			vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
			xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
			xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
		}
		u.uen--;
		if (uwep && P_SKILL(objects[uwep->otyp].oc_skill) >= P_SKILLED && u.uen > 0){
			int a;
			int k;
			/* get direction */
			a = getdir((char *)0);
			if (a){
				/* get defender */
				if (u.ustuck && u.uswallow)
					mdef = u.ustuck;
				else
					mdef = m_at(u.ux + u.dx, u.uy + u.dy);
				/* attack (melee once OR throw lightsaber) */
				if (!mdef) {
					projectile(&youmonst, uwep, (struct obj *)0, FALSE, u.ux, u.uy, u.dx, u.dy, u.dz, 10, FALSE, TRUE, FALSE);
				}
				else {
					vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
					xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
				}
			}
			k = dokick();
			if (a || k){
				u.uen--;
			}
			else return TRUE;
		}
		if (uwep && P_SKILL(objects[uwep->otyp].oc_skill) >= P_EXPERT && u.uen > 0){
			int j = jump(1);
			int d = getdir((char *)0);
			if (!j && !d)
				return TRUE;
			u.uen--;
			if (d){
				/* get defender */
				if (u.ustuck && u.uswallow)
					mdef = u.ustuck;
				else
					mdef = m_at(u.ux + u.dx, u.uy + u.dy);
				/* attack (melee twice OR throw lightsaber) */
				if (!mdef) {
					projectile(&youmonst, uwep, (struct obj *)0, FALSE, u.ux, u.uy, u.dx, u.dy, u.dz, 10, FALSE, TRUE, FALSE);
				}
				else {
					vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
					xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
					xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
				}
			}
		}
		return TRUE;
	}
	else if (objects[uwep->otyp].oc_skill == P_SPEAR || objects[uwep->otyp].oc_skill == P_LANCE){ //!uwep handled above
		boolean attacked = FALSE;
		int n = 1;
		
		if (uwep && P_SKILL(objects[uwep->otyp].oc_skill) >= P_SKILLED)
			n++;
		if (uwep && P_SKILL(objects[uwep->otyp].oc_skill) >= P_EXPERT)
			n++;

		while (n > 0 && u.uen > 0){
			/* get direction of attack; if first time, cancelling will take no time */
			if (!getdir((char *)0))
				return attacked;
			/* things that only occur in the first 'attack' of the combo */
			if (!attacked) {
				if (fast_weapon(uwep))
					youmonst.movement += 2;
			}
			/* get defender */
			if (u.ustuck && u.uswallow)
				mdef = u.ustuck;
			else
				mdef = m_at(u.ux + u.dx, u.uy + u.dy);
			/* attack */
			if (!mdef)
				You("stab wildly!");
			else {
				vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
				xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
				xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
			}
			n--;
			u.uen--;
			attacked = TRUE;
		}
		return TRUE;
	}
	else if (objects[uwep->otyp].oc_skill == P_WHIP){ //!uwep handled above
		/* get direction of attack */
		if (!getdir((char *)0))
			return FALSE;
		/* fast weapons give you speed */
		if (fast_weapon(uwep))
			youmonst.movement += 2;
		/* get defender */
		if (u.ustuck && u.uswallow)
			mdef = u.ustuck;
		else
			mdef = m_at(u.ux + u.dx, u.uy + u.dy);
		/* attack (twice) */
		if (!mdef)
			You("swing wildly!");
		else {
			vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
			xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
			xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
		}
		u.uen--;

		if (uwep && P_SKILL(objects[uwep->otyp].oc_skill) >= P_SKILLED && u.uen > 0){
			/* get direction AND do whip things */
			if (!use_whip(uwep) || !uwep)
				return TRUE;
			/* get defender */
			if (u.ustuck && u.uswallow)
				mdef = u.ustuck;
			else
				mdef = m_at(u.ux + u.dx, u.uy + u.dy);
			/* attack (once) */
			if (!mdef)
				You("swing wildly!");
			else {
				vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
				xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
			}
			u.uen--;
		}


		if (uwep && P_SKILL(objects[uwep->otyp].oc_skill) >= P_EXPERT && u.uen > 0){
			/* get direction AND do whip things */
			if (!use_whip(uwep) || !uwep)
				return TRUE;
			if (uwep->otyp == FORCE_WHIP){
				/* turn it into a sword */
				use_force_sword(uwep);
				/* get defender */
				if (u.ustuck && u.uswallow)
					mdef = u.ustuck;
				else
					mdef = m_at(u.ux + u.dx, u.uy + u.dy);
				/* attack (twice) */
				if (!mdef)
					You("swing wildly!");
				else {
					vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
					xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
					xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
				}
			}
			else {
				/* get defender */
				if (u.ustuck && u.uswallow)
					mdef = u.ustuck;
				else
					mdef = m_at(u.ux + u.dx, u.uy + u.dy);
				/* attack (once) */
				if (!mdef)
					You("swing wildly!");
				else {
					vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
					xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
				}
			}
			u.uen--;
		}
		return TRUE;
	}
	else if (!bimanual(uwep, youracedata)){ //!uwep handled above
		/* get direction of attack */
		if (!getdir((char *)0))
			return FALSE;
		/* fast weapons give you speed */
		if (fast_weapon(uwep))
			youmonst.movement += 2;
		/* get defender */
		if (u.ustuck && u.uswallow)
			mdef = u.ustuck;
		else
			mdef = m_at(u.ux + u.dx, u.uy + u.dy);
		/* attack (twice) */
		if (!mdef)
			You("swing wildly!");
		else {
			vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
			xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
			xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
		}
		u.uen--;

		if (uwep && P_SKILL(objects[uwep->otyp].oc_skill) >= P_SKILLED && u.uen > 0){
			if (dofire()){
				u.uen--;
			}
			else return TRUE;
		}
		if (uwep && P_SKILL(objects[uwep->otyp].oc_skill) >= P_EXPERT && u.uen > 0){
			if (dofire()){
				u.uen--;
				if (uwep){
					/* get defender */
					if (u.ustuck && u.uswallow)
						mdef = u.ustuck;
					else
						mdef = m_at(u.ux + u.dx, u.uy + u.dy);
					/* attack (once) (no message for no defender) */
					if (mdef) {
						vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
						xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
					}
				}
			}
			else return TRUE;
		}
		return TRUE;
	}
	else if (bimanual(uwep, youracedata)){ //!uwep handled above
		int i, j;
		/* get direction of attack */
		if (!getdir((char *)0))
			return FALSE;
		/* fast weapons give you speed */
		if (fast_weapon(uwep))
			youmonst.movement += 2;
		
		if (u.dz) {
			/* if getdir() was gived u.dz != 0, we will just pick a random direction to start hitting */
			i = rn2(8);
		}
		else {
			/* get your targetted direction's index */
			for (i = 0; i < 8; i++)
			if (xdir[i] == u.dx && ydir[i] == u.dy)
				break;
		}
		/* attack counterclockwise, hitting first direction twice (first and last hits) */
		for (j = 8; j >= 0; j--){
			if (u.ustuck && u.uswallow)
				mdef = u.ustuck;
			else
				mdef = m_at(u.ux + xdir[(i + j) % 8], u.uy + ydir[(i + j) % 8]);
			/* isn't that nice, we don't attack pets (even when confused?) */
			if (mdef && !mdef->mtame){
				vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
				xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
			}
		}
		u.uen--;
		youmonst.movement -= 3;
		if (uwep && P_SKILL(objects[uwep->otyp].oc_skill) >= P_SKILLED && u.uen > 0){
			/* use a kick to get direction */
			if (!dokick())
				return TRUE;
			/* get your targetted direction's index */
			for (i = 0; i < 8; i++)
			if (xdir[i] == u.dx && ydir[i] == u.dy)
				break;
			/* attack counterclockwise, hitting first direction twice (first and last hits) */
			for (j = 8; j >= 0; j--){
				if (u.ustuck && u.uswallow)
					mdef = u.ustuck;
				else
					mdef = m_at(u.ux + xdir[(i + j) % 8], u.uy + ydir[(i + j) % 8]);
				/* isn't that nice, we don't attack pets (even when confused?) */
				if (mdef && !mdef->mtame){
					vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
					xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
				}
			}
			u.uen--;
			youmonst.movement -= 3;
		}
		if (uwep && P_SKILL(objects[uwep->otyp].oc_skill) >= P_EXPERT && u.uen > 0){
			if (dofire()){
				u.uen--;
				/* get defender */
				if (u.ustuck && u.uswallow)
					mdef = u.ustuck;
				else
					mdef = m_at(u.ux + u.dx, u.uy + u.dy);
				/* attack (once) */
				if (mdef) {
					vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
					xmeleehity(&youmonst, mdef, &weaponhit, uwep, vis, 0, FALSE);
				}
			}
			else return TRUE;
		}
		return TRUE;
	}
	/* This should never be reached */
	return FALSE;
}

/* u_pole_pound()
 * 
 * hits monster at bhitpos with your (mainhand) polearm
 */
int
u_pole_pound(mdef)
struct monst * mdef;
{
	int vis = (VIS_MAGR | VIS_NONE) | (canseemon(mdef) ? VIS_MDEF : 0);
	notonhead = (bhitpos.x != x(mdef) || bhitpos.y != y(mdef));
	return xmeleehity(&youmonst, mdef, &basicattack, uwep, vis, 0, TRUE);
}
