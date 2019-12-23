/*	SCCS Id: @(#)allmain.c	3.4	2003/04/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* various code that was replicated in *main.c */

#include "hack.h"
#include "edog.h"
#include "artifact.h"
#ifdef OVLB
#include "artilist.h"
#else
STATIC_DCL struct artifact artilist[];
#endif


#ifndef NO_SIGNAL
#include <signal.h>
#endif

#ifdef POSITIONBAR
STATIC_DCL void NDECL(do_positionbar);
#endif

STATIC_DCL void NDECL(androidUpkeep);
STATIC_DCL void NDECL(printMons);
STATIC_DCL void NDECL(printDPR);
STATIC_DCL int NDECL(do_inheritor_menu);
STATIC_DCL void FDECL(printAttacks, (char *,struct permonst *));
STATIC_DCL void FDECL(resFlags, (char *,unsigned int));

#ifdef OVL0

extern const int monstr[];

STATIC_OVL void
digcrater(mtmp)
	struct monst *mtmp;
{
	int x,y,i,j;
	struct trap *ttmp;
	for(i=-5;i<=5;i++){
		x = mtmp->mx+i;
		for(j=-5;j<=5;j++){
			y = mtmp->my+j;
			if(isok(x,y)&& !(x == u.ux && y == u.uy)){
				ttmp = t_at(x, y);
				if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
					if(dist2(x,y,mtmp->mx,mtmp->my) <= 26){
						levl[x][y].typ = CORR;
						if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
						if(ttmp) delfloortrap(ttmp);
						if(dist2(x,y,mtmp->mx,mtmp->my) <= 9) levl[x][y].typ = LAVAPOOL;
						else if(dist2(x,y,mtmp->mx,mtmp->my) <= 26) digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
					}
				}
			}
		}
	}
}

STATIC_OVL void
digcloudcrater(mtmp)
	struct monst *mtmp;
{
	int x,y,i,j;
	struct trap *ttmp;
	for(i=-2;i<=2;i++){
		x = mtmp->mx+i;
		for(j=-2;j<=2;j++){
			y = mtmp->my+j;
			if(isok(x,y) && dist2(x,y,mtmp->mx,mtmp->my) < 8){
				levl[x][y].typ = CLOUD;
			}
		}
	}
}

STATIC_OVL void
digXchasm(mtmp)
	struct monst *mtmp;
{
	int x,y;
	struct trap *ttmp;
	int inty = rn2(4)+2;
	int dy = rn2(2) ? 1 : -1;
	y = mtmp->my;
	x = mtmp->mx;
	if(isok(x,y)&& !(x == u.ux && y == u.uy)){
		ttmp = t_at(x, y);
		if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
			levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
			if(ttmp) delfloortrap(ttmp);
			digactualhole(x, y, mtmp, HOLE, TRUE, FALSE);
		}
	}
	y=y+1;
	if(isok(x,y)){
		ttmp = t_at(x, y);
		if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
			levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
			if(ttmp) delfloortrap(ttmp);
			digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
		}
	}
	y=y-1;
	y=y-1;
	if(isok(x,y)){
		ttmp = t_at(x, y);
		if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
			levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
			if(ttmp) delfloortrap(ttmp);
			digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
		}
	}
	y=y+1;
	for(x = mtmp->mx + 1; x < COLNO; x++){
		if(!(x%inty)) y += dy;
		if(isok(x,y) && !(x == u.ux && y == u.uy)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
				if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, HOLE, TRUE, FALSE);
			}
		}
		y=y+1;
		if(isok(x,y)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
				if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
			}
		}
		y=y-1;
		y=y-1;
		if(isok(x,y)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
			}
		}
		y=y+1;
	}
	y = mtmp->my;
	for(x = mtmp->mx - 1; x >= 0; x--){
		if(!(x%inty)) y -= dy;
		if(isok(x,y)&& !(x == u.ux && y == u.uy)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, HOLE, TRUE, FALSE);
			}
		}
		y=y+1;
		if(isok(x,y)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
			}
		}
		y=y-1;
		y=y-1;
		if(isok(x,y)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
			}
		}
		y=y+1;
	}
}

STATIC_OVL void
digYchasm(mtmp)
	struct monst *mtmp;
{
	int x,y;
	struct trap *ttmp;
	int intx = rn2(4)+2;
	int dx = rn2(2) ? 1 : -1;
	x = mtmp->mx;
	y = mtmp->my;
	if(isok(x,y)&& !(x == u.ux && y == u.uy)){
		ttmp = t_at(x, y);
		if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
			levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
			if(ttmp) delfloortrap(ttmp);
			digactualhole(x, y, mtmp, HOLE, TRUE, FALSE);
		}
	}
	x=x+1;
	if(isok(x,y)){
		ttmp = t_at(x, y);
		if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
			levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
			if(ttmp) delfloortrap(ttmp);
			digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
		}
	}
	x=x-1;
	x=x-1;
	if(isok(x,y)){
		ttmp = t_at(x, y);
		if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
			levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
			if(ttmp) delfloortrap(ttmp);
			digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
		}
	}
	x=x+1;
	for(y = mtmp->my + 1; y < COLNO; y++){
		if(!(y%intx)) x += dx;
		if(isok(x,y)&& !(x == u.ux && y == u.uy)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, HOLE, TRUE, FALSE);
			}
		}
		x=x+1;
		if(isok(x,y)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
			}
		}
		x=x-1;
		x=x-1;
		if(isok(x,y)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
			}
		}
		x=x+1;
	}
	x = mtmp->mx;
	for(y = mtmp->my - 1; y >= 0; y--){
		if(!(y%intx)) x -= dx;
		if(isok(x,y)&& !(x == u.ux && y == u.uy)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, HOLE, TRUE, FALSE);
			}
		}
		x=x+1;
		if(isok(x,y)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
			}
		}
		x=x-1;
		x=x-1;
		if(isok(x,y)){
			ttmp = t_at(x, y);
			if(levl[x][y].typ <= SCORR || levl[x][y].typ == CORR || levl[x][y].typ == ROOM){
				levl[x][y].typ = CORR;
			if(!does_block(x,y,&levl[x][y])) unblock_point(x,y);	/* vision:  can see through */
				if(ttmp) delfloortrap(ttmp);
				digactualhole(x, y, mtmp, PIT, TRUE, FALSE);
			}
		}
		x=x+1;
	}
}

STATIC_OVL
void
androidUpkeep()
{
	//Pay unusual upkeep here, possibly pass out
	if(uandroid && !u.usleep){
		int mult = HEALCYCLE/u.ulevel;
		//Possibly pass out if you begin this step with 0 energy.
		if(u.uen == 0 && !rn2(10+u.ulevel) && moves >= u.nextsleep){
			int t = rn1(u.uenmax*mult+40, u.uenmax*mult+40);
			You("pass out from exhaustion!");
			u.nextsleep = moves+rnz(350)+t;
			u.lastslept = moves;
			fall_asleep(-t, TRUE);
			nomul(-1*u.uenmax, "passed out from exhaustion");
		}
		if(u.phasengn){
			u.uen -= 10;
			if(u.uen <= 0){
				u.uen = 0;
				You("can no longer maintain phase!");
				u.phasengn = 0;
			}
		}
		if(u.ucspeed == HIGH_CLOCKSPEED){
			u.uen -= 1;
			if(u.uen <= 0){
				u.uen = 0;
				You("can no longer maintain emergency speed!");
				u.ucspeed = NORM_CLOCKSPEED;
			}
		}
		//Exceeding operational time
		static char fatigue_warning = 0;
		if(moves >= u.nextsleep+1400 && u.uen > 10 && !fatigue_warning){
			fatigue_warning = 1;
			You_feel("fatigued.");
		}
		static char e_fatigued = 0;
		if(moves >= u.nextsleep+1400 && u.uen > 1 && u.uen <= 10 && !e_fatigued){
			e_fatigued = 1;
			You_feel("extremely fatigued.");
		}
		static char pass_warning = 0;
		if(moves >= u.nextsleep && u.uen <= 1 && !pass_warning){
			pass_warning = 1;
			You_feel("like you're about to pass out.");
		}
		if(moves > u.nextsleep+1400 && u.uen > 0){
			if(!(moves%20)) u.uen -= 1;
		}
	}
}

/* perform 1 turn's worth of time-dependent hp modification, mostly silently */
/* NOTES: can rehumanize(), can print You("pass out from exertion!") if moving when overloaded at 1 hp */
void
you_regen_hp()
{
	int wtcap = near_capacity();
	int perX = 0;
	int * hpmax;
	int * hp;

	// set hp, maxhp pointers
	hp    = (Upolyd) ? (&u.mh)    : (&u.uhp);
	hpmax = (Upolyd) ? (&u.mhmax) : (&u.uhpmax);

	if (u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves + 20) {
		/* for the moment at least, you're in tiptop shape */
		/* do not regenerate any health */
		/* you also do not lose health (e.g. from being a fish out of water) */
		return;
	}

	// Sanity check
	if (Upolyd && (*hp < 1))
		rehumanize();

	//Androids regenerate from active Hoon, but not from other sources unless dormant
	if(u.uhoon_duration && (*hp) < (*hpmax)){
		flags.botl = 1;
		
		(*hp) += 10;

		if ((*hp) > (*hpmax))
			(*hp) = (*hpmax);
	}
	
	perX += u.uhoon;
	
	//Androids regenerate from active Hoon, but not from other sources unless dormant
	// Notably, no bonus from passive Hoon
	if(uandroid && !u.usleep)
		return;
	
	// fish out of water
	if (youracedata->mlet == S_EEL && !is_pool(u.ux, u.uy, youracedata->msize == MZ_TINY) && !Is_waterlevel(&u.uz)) {
		if (is_pool(u.ux, u.uy, TRUE))
			perX -= HEALCYCLE * (youracedata->msize - 1) / (youracedata->msize);
		else
			perX -= HEALCYCLE;
		u.regen_blocked++;
	}

	// invidiaks out of dark
	if (youracedata == &mons[PM_INVIDIAK] && !isdark(u.ux, u.uy)) {
		perX -= HEALCYCLE;
		u.regen_blocked++;
	}
	
	// regeneration 'trinsic
	if (Regeneration){
		perX += HEALCYCLE;
	}
	// Corruption thought
	if(active_glyph(CORRUPTION) && (*hp < (*hpmax)*.3)){
		perX += HEALCYCLE;
	}
	
	// "Natural" regeneration has stricter limitations
	if (u.regen_blocked > 0) u.regen_blocked--;		// not regen_blocked (NOTE: decremented here)
	else if (!(nonliving(youracedata) && !uandroid) &&	// not nonliving, however, androids auto-repair while asleep
		!Race_if(PM_INCANTIFIER) &&					// not incantifier (including while polymorphed)
		(wtcap < MOD_ENCUMBER || !u.umoved) &&		// not overloaded
		!(uwep && uwep->oartifact == ART_ATMA_WEAPON && uwep->lamplit && !Drain_resistance && rn2(4)) // 3 in 4 chance of being prevented by Atma Weapon
			// Question for Chris: what if instead, do "reglevel /= 4;" when atma weapon is active, placed either before or after the minimum reg 1 check?
		)
	{
		int reglevel = u.ulevel;

		// CON bonus (while in natural form)
		if (!Upolyd)
			reglevel += ((int)ACURR(A_CON) - 10) / 2;
		// minimum 1
		if (reglevel < 1)
			reglevel = 1;

		// healer role bonus
		if (Role_if(PM_HEALER) && !Upolyd)
			reglevel += 10;

		// penalty for being itchy
		reglevel -= u_healing_penalty();

		// minimum 1
		if (reglevel < 1)
			reglevel = 1;

		perX += reglevel;
	}

	// The Ring of Hygiene's Disciple
	if (!Upolyd &&	// Question for Chris: should this be enabled to also work while polymorphed?
		((uleft  && uleft->oartifact == ART_RING_OF_HYGIENE_S_DISCIPLE) ||
		(uright && uright->oartifact == ART_RING_OF_HYGIENE_S_DISCIPLE))
		){
		perX += HEALCYCLE * min(4, (*hpmax) / max((*hp), 1));
	}

	// Buer
	if (u.sealsActive&SEAL_BUER){
		int dsize = spiritDsize();

		if (uwep && uwep->oartifact == ART_ATMA_WEAPON && uwep->lamplit && !Drain_resistance)
			perX += dsize * 6 / 4;
		else
			perX += dsize * 6;
	}
	
	/* moving around while encumbered is hard work */
	if (wtcap > MOD_ENCUMBER && u.umoved) {
		if (*hp > 1)
		{
			if (wtcap < EXT_ENCUMBER)
				perX -= 1;
			else
				perX -= 3;
		}
		else
		{
			You("pass out from exertion!");
			exercise(A_CON, FALSE);
			fall_asleep(-10, FALSE);
		}
	}

	if (((perX > 0) && ((*hp) < (*hpmax))) ||			// if regenerating
		((perX < 0))									// or dying
		)
	{
		// update bottom line
		flags.botl = 1;

		// modify by 1/HEALCYCLEth of perX per turn:
		*hp += perX / HEALCYCLE;
		// Now deal with any remainder
		if (((moves)*(abs(perX) % HEALCYCLE)) / HEALCYCLE >((moves - 1)*(abs(perX) % HEALCYCLE)) / HEALCYCLE)
			*hp += 1 * sgn(perX);
		// cap at maxhp
		if ((*hp) > (*hpmax))
			(*hp) = (*hpmax);

		// check for rehumanization
		if (Upolyd && (*hp < 1))
			rehumanize();
	}
}

/* perform 1 turn's worth of time-dependent power modification, silently */
void
you_regen_pw()
{
	int wtcap = near_capacity();
	int perX = 0;

	// natural power regeneration
	if (wtcap < MOD_ENCUMBER &&		// not overly encumbered
		!Race_if(PM_INCANTIFIER)	// not an incantifier
		) {
		int reglevel = u.ulevel + (((int)ACURR(A_WIS)) - 10) / 2;
		// level + WISmod minimum 1
		if (reglevel < 1)
			reglevel = 1;

		// role bonuses
		if (Role_if(PM_WIZARD))   reglevel += 10;
		if (Role_if(PM_HEALER))   reglevel += 6;
		if (Role_if(PM_PRIEST))   reglevel += 6;
		if (Role_if(PM_VALKYRIE)) reglevel += 3;
		if (Role_if(PM_MONK))     reglevel += 3;

		// cornuthaum bonus for wizards (but not incantifiers, since they don't naturally regenerate power at all)
		if (u.uen < u.uenmax && (Role_if(PM_WIZARD)) && uarmh && uarmh->otyp == CORNUTHAUM){
			reglevel += uarmh->spe;
		}
		// penalty for being itchy
		reglevel -= u_healing_penalty();
		// penalty from spell protection interfering with natural pw regeneration
		if (u.uspellprot > 0)
			reglevel -= 10 + 2 * u.uspellprot;

		// minimum 1
		if (reglevel < 1)
			reglevel = 1;

		perX += reglevel;
	}

	// external power regeneration
	if (Energy_regeneration ||										// energy regeneration 'trinsic
		(u.umartial && !uarmf && IS_GRASS(levl[u.ux][u.uy].typ))	// or being a bare-foot martial-artist standing on grass
		)
	{
		perX += HEALCYCLE;
	}

	// Unknown God
	if (u.specialSealsActive&SEAL_UNKNOWN_GOD){
		perX += spiritDsize() * 6;
	}

	// power drain from maintained spells
	if (u.maintained_en_debt > 0)
	{
		int reglevel = u.maintained_en_debt / 3;
		int debtpaid = 0;

		if (perX > reglevel)
		{// can just subtract drain from pw regeneration and still have net positive
			perX -= reglevel;
			debtpaid += reglevel;
		}
		else
		{// either 0 pw regen or net drain
			// put the entirety of perX against the debt owed
			reglevel -= perX;
			debtpaid += perX;
			perX = 0;
			// drain further if the player has reserves to burn
			if (u.uen > 0 || Race_if(PM_INCANTIFIER))
			{
				debtpaid += reglevel;
				perX = -reglevel;
			}
		}
		// settle the payment
		u.maintained_en_debt -= debtpaid / HEALCYCLE;
		//Now deal with any remainder
		if (((moves)*(debtpaid % HEALCYCLE)) / HEALCYCLE >((moves - 1)*(debtpaid % HEALCYCLE)) / HEALCYCLE)
			u.maintained_en_debt -= 1;
		// minimum zero
		if (u.maintained_en_debt < 0)
			u.maintained_en_debt = 0;
	}

	//Naen fast energy regen applies to androids, too.
	if(u.unaen_duration && (u.uen < u.uenmax)){
		flags.botl = 1;
		u.uen += 10;
		if (u.uen > u.uenmax)
			u.uen = u.uenmax;
	}
	
	//Naen passive bonus only partially applies to androids and incantifiers
	if(Race_if(PM_INCANTIFIER)){
		if(perX < 0)
			perX += u.unaen;
		if(perX > 0)
			perX = 0;
	}
	else perX += u.unaen;
	
	//Androids only regen power while asleep, but allow their rate to offset spell maintenance
	if(uandroid && !u.usleep && perX > 0)
		return;

	if (((perX > 0) && (u.uen < u.uenmax)) ||							// if regenerating power
		((perX < 0) && ((u.uen > 0) || Race_if(PM_INCANTIFIER)))		// or draining power
		)
	{
		//update bottom line
		flags.botl = 1;

		// modify by 1/HEALCYCLEth of perX per turn:
		u.uen += perX / HEALCYCLE;
		// Now deal with any remainder
		if (((moves)*(abs(perX) % HEALCYCLE)) / HEALCYCLE >((moves - 1)*(abs(perX) % HEALCYCLE)) / HEALCYCLE)
			u.uen += 1 * sgn(perX);
		// cap at maxpw
		if (u.uen > u.uenmax)
			u.uen = u.uenmax;
		// and at 0 (for non-incantifiers)
		if (u.uen < 0 && !(Race_if(PM_INCANTIFIER)))
			u.uen = 0;
	}
}

void
moveloop()
{
#if defined(MICRO) || defined(WIN32)
	char ch;
#endif
	int abort_lev, i, j;
    struct monst *mtmp, *nxtmon;
	struct obj *pobj;
    int moveamt = 0, wtcap = 0, change = 0;
    boolean didmove = FALSE, monscanmove = FALSE;
	int oldspiritAC=0;
	int tx,ty;
	int nmonsclose,nmonsnear,enkispeedlim;
	static boolean oldBlind = 0, oldLightBlind = 0;
	int healing_penalty = 0;
    int hpDiff;
	static int oldsanity = 100;

    flags.moonphase = phase_of_the_moon();
    if(flags.moonphase == FULL_MOON) {
	You("are lucky!  Full moon tonight.");
	change_luck(1);
    } else if(flags.moonphase == NEW_MOON) {
	pline("Be careful!  New moon tonight.");
    }
    flags.friday13 = friday_13th();
    if (flags.friday13) {
	pline("Watch out!  Bad things can happen on Friday the 13th.");
	change_luck(-1);
    }

    initrack();


    /* Note:  these initializers don't do anything except guarantee that
	    we're linked properly.
    */
    decl_init();
    monst_init();
    monstr_init();	/* monster strengths */
    objects_init();

#ifdef WIZARD
	// Called twice in dNAO?
    if (wizard) add_debug_extended_commands();
#endif

    (void) encumber_msg(); /* in case they auto-picked up something */

    u.uz0.dlevel = u.uz.dlevel;
    youmonst.movement = NORMAL_SPEED;	/* give the hero some movement points */
	flags.move = FALSE; /* From nethack 3.6.2 */

	if(u.ualignbase[A_ORIGINAL] == A_LAWFUL && flags.initalign != 0){
		flags.initalign = 0;
		impossible("Bad alignment initializer detected and fixed. Save and reload.");
	}
	if(u.ualignbase[A_ORIGINAL] == A_NEUTRAL && flags.initalign != 1){
		flags.initalign = 1;
		impossible("Bad alignment initializer detected and fixed. Save and reload.");
	}
	if(u.ualignbase[A_ORIGINAL] == A_CHAOTIC && flags.initalign != 2){
		flags.initalign = 2;
		impossible("Bad alignment initializer detected and fixed. Save and reload.");
	}
	printMons();
	printDPR();
    for(;;) {/////////////////////////MAIN LOOP/////////////////////////////////
    hpDiff = u.uhp;
	get_nh_event();
#ifdef POSITIONBAR
	do_positionbar();
#endif

	didmove = flags.move;
	if(didmove) {
	    /* actual time passed */
		if(flags.run != 0){
			if(uwep && uwep->oartifact == ART_TENSA_ZANGETSU){
				youmonst.movement -= 1;
			} else if(uwep && uwep->oartifact == ART_SODE_NO_SHIRAYUKI){
				youmonst.movement -= 3;
			} else if(uandroid && u.ucspeed == HIGH_CLOCKSPEED){
				youmonst.movement -= 3;
			} else if(uwep && uwep->oartifact == ART_TOBIUME){
				youmonst.movement -= 4;
			} else {
				youmonst.movement -= NORMAL_SPEED;
			}
		} else if(uandroid && u.ucspeed == HIGH_CLOCKSPEED){
			if(u.umoved){
				youmonst.movement -= 3;
			} else {
				u.ucspeed = NORMAL_SPEED;
				youmonst.movement -= NORMAL_SPEED;
			}
		} else youmonst.movement -= NORMAL_SPEED;
		
		  /**************************************************/
		 /*monsters that respond to the player turn go here*/
		/**************************************************/
		for (mtmp = fmon; mtmp; mtmp = nxtmon){
			nxtmon = mtmp->nmon;
			if (!DEADMONSTER(mtmp)
				&& attacktype(mtmp->data, AT_WDGZ)
				&& !(controlledwidegaze(mtmp->data) && (mtmp->mpeaceful || mtmp->mtame))
				&& !(hideablewidegaze(mtmp->data) && (rn2(3) < magic_negation(mtmp)))
				&& couldsee(mtmp->mx, mtmp->my)
			) m_respond(mtmp);
		}
		
	    do { /* hero can't move this turn loop */
		wtcap = encumber_msg();

		flags.mon_moving = TRUE;
		do { /* Monster turn loop */
		    monscanmove = movemon();
			  /****************************************/
			 /*once-per-monster-moving things go here*/
			/****************************************/
////////////////////////////////////////////////////////////////////////////////////////////////
			reset_mvmdieroll();
			reset_mvudieroll();
			if(u.specialSealsActive&SEAL_LIVING_CRYSTAL)
				average_dogs();
			for (mtmp = fmon; mtmp; mtmp = nxtmon){
				nxtmon = mtmp->nmon;
				if(mtmp->data == &mons[PM_HELLCAT]){
					if(!isdark(mtmp->mx,mtmp->my) && !mtmp->minvis){
						mtmp->minvis = TRUE;
						mtmp->perminvis = TRUE;
						newsym(mtmp->mx,mtmp->my);
					} else if(isdark(mtmp->mx,mtmp->my) && mtmp->minvis){
						mtmp->minvis = FALSE;
						mtmp->perminvis = FALSE;
						newsym(mtmp->mx,mtmp->my);
					}
				} else if(mtmp->data == &mons[PM_HUDOR_KAMEREL]){
					if(is_pool(mtmp->mx,mtmp->my, TRUE) && !mtmp->minvis){
						mtmp->minvis = TRUE;
						mtmp->perminvis = TRUE;
						newsym(mtmp->mx,mtmp->my);
					} else if(!is_pool(mtmp->mx,mtmp->my, TRUE) && mtmp->minvis){
						mtmp->minvis = FALSE;
						mtmp->perminvis = FALSE;
						newsym(mtmp->mx,mtmp->my);
					}
				} else if(mtmp->data == &mons[PM_GRUE] || mtmp->data == &mons[PM_INVIDIAK]){
					if(isdark(mtmp->mx,mtmp->my) && !mtmp->minvis){
						mtmp->minvis = TRUE;
						mtmp->perminvis = TRUE;
						newsym(mtmp->mx,mtmp->my);
					} else if(!isdark(mtmp->mx,mtmp->my) && mtmp->minvis){
						mtmp->minvis = FALSE;
						mtmp->perminvis = FALSE;
						newsym(mtmp->mx,mtmp->my);
					}
				} else if(mtmp->m_insight_level > u.uinsight){
					migrate_to_level(mtmp, ledger_no(&u.uz), MIGR_EXACT_XY, (coord *)0);
					continue;
				}
				if (mtmp->minvis){
					newsym(mtmp->mx, mtmp->my);
				}
				if (Blind && Bloodsense && has_blood_mon(mtmp)){
					newsym(mtmp->mx, mtmp->my);
				}
				if(uwep && uwep->oartifact == ART_SINGING_SWORD && !is_deaf(mtmp)){
					//quite noisy
					mtmp->mux = u.ux;
					mtmp->muy = u.uy;
				}
			}
////////////////////////////////////////////////////////////////////////////////////////////////
			if(Echolocation){
				for(i=1; i<COLNO; i++)
					for(j=0; j<ROWNO; j++)
						if(viz_array[j][i]&COULD_SEE)
							echo_location(i, j);
				see_monsters();
			}
			/*If anything a monster did caused us to get moved out of water, surface*/
			if(u.usubwater && !is_pool(u.ux, u.uy, FALSE)){
				u.usubwater = 0;
				vision_full_recalc = 1;
				vision_recalc(2);	/* unsee old position */
				doredraw();
			} else if(Is_waterlevel(&u.uz) && u.usubwater && !is_3dwater(u.ux, u.uy)){
				You("pop out into an airbubble!");
				u.usubwater = 0;
			} else if(Is_waterlevel(&u.uz) && !u.usubwater && is_3dwater(u.ux, u.uy)){
				Your("%s goes under water!", body_part(HEAD));
				if(!Breathless) You("can't breath.");
				u.usubwater = 1;
			}
////////////////////////////////////////////////////////////////////////////////////////////////
			switch (((u_healing_penalty() - healing_penalty) > 0) - ((u_healing_penalty() - healing_penalty) < 0))
			{
			case 0:
					break;
			case 1:
					if (!Hallucination) {
						You_feel("%s.", (healing_penalty) ? "itchier" : "itchy");
					}
					else {
						You_feel("%s.", (healing_penalty) ? "uglier" : "ugly");
					}
					healing_penalty = u_healing_penalty();
					break;
			case -1:
					if (!Hallucination) {
						You_feel("%s.", (u_healing_penalty()) ? "some relief" : "relieved");
					}
					else {
						You_feel("%s.", (u_healing_penalty()) ? "pretty" : "beautiful");
					}
					healing_penalty = u_healing_penalty();
					break;
			}
			/*
			if (healing_penalty != u_healing_penalty()) {
				if (!Hallucination){
					You_feel("%s.", (!healing_penalty) ? "itchy" : "relief");
				} else {
					You_feel("%s.", (!healing_penalty) ? (hates_silver(youracedata) ? "tarnished" :
						hates_iron(youracedata) ? "magnetic" : "like you are failing Organic Chemistry") : "like you are no longer failing Organic Chemistry");
				}
				
				
			}
			*/
////////////////////////////////////////////////////////////////////////////////////////////////
			if (!oldBlind ^ !Blind) {  /* one or the other but not both */
				see_monsters();
				flags.botl = 1;
				vision_full_recalc = 1;	/* blindness just got toggled */
				if (Blind_telepat || Infravision) see_monsters();
				oldBlind = !!Blind;
			}
////////////////////////////////////////////////////////////////////////////////////////////////
			if (!oldLightBlind ^ !LightBlind) {  /* one or the other but not both */
				see_monsters();
				flags.botl = 1;
				vision_full_recalc = 1;	/* blindness just got toggled */
				if (Blind_telepat || Infravision) see_monsters();
				oldLightBlind = !!LightBlind;
			}
////////////////////////////////////////////////////////////////////////////////////////////////
			calc_total_maxhp();
////////////////////////////////////////////////////////////////////////////////////////////////
			calc_total_maxen();
////////////////////////////////////////////////////////////////////////////////////////////////
			oldspiritAC = u.spiritAC;
			u.spiritAC = 0; /* reset temporary spirit AC bonuses. Do this once per monster turn */
			u.spiritAttk = 0;
			if(u.sealsActive){
				if(u.sealsActive&SEAL_MALPHAS){
					for (mtmp = fmon; mtmp; mtmp = mtmp->nmon){
						if(mtmp->data == &mons[PM_CROW] && mtmp->mtame && !um_dist(mtmp->mx,mtmp->my,1)){
							u.spiritAC++;
							u.spiritAttk++;
						}
					}
				}
				if(u.sealsActive&SEAL_ANDREALPHUS){
					if(isok(tx=u.ux-1,ty=u.uy-1) && IS_CORNER(levl[tx][ty].typ) 
						&& IS_WALL(levl[tx+1][ty].typ) && IS_WALL(levl[tx][ty+1].typ)) u.spiritAC += u.ulevel/2+1;
					else if(isok(tx=u.ux+1,ty=u.uy-1) && IS_CORNER(levl[tx][ty].typ) 
						&& IS_WALL(levl[tx-1][ty].typ) && IS_WALL(levl[tx][ty+1].typ)) u.spiritAC += u.ulevel/2+1;
					else if(isok(tx=u.ux+1,ty=u.uy+1) && IS_CORNER(levl[tx][ty].typ) 
						&& IS_WALL(levl[tx-1][ty].typ) && IS_WALL(levl[tx][ty-1].typ)) u.spiritAC += u.ulevel/2+1;
					else if(isok(tx=u.ux-1,ty=u.uy+1) && IS_CORNER(levl[tx][ty].typ) 
						&& IS_WALL(levl[tx+1][ty].typ) && IS_WALL(levl[tx][ty-1].typ)) u.spiritAC += u.ulevel/2+1;
				}
				if(u.sealsActive&SEAL_ENKI){
					for(tx=u.ux-1; tx<=u.ux+1;tx++){
						for(ty=u.uy-1; ty<=u.uy+1;ty++){
							if(isok(tx,ty) && (tx!=u.ux || ty!=u.uy) && IS_STWALL(levl[tx][ty].typ)) u.spiritAC += 1;
						}
					}
				}
				if(u.spiritAC!=oldspiritAC) flags.botl = 1;
			}
////////////////////////////////////////////////////////////////////////////////////////////////
			if(u.uhs == WEAK && u.sealsActive&SEAL_AHAZU) unbind(SEAL_AHAZU,TRUE);
#ifndef GOLDOBJ
			// if(u.sealsActive&SEAL_FAFNIR && u.ugold < u.ulevel*100) unbind(SEAL_FAFNIR,TRUE);
			if(u.sealsActive&SEAL_FAFNIR && u.ugold == 0) unbind(SEAL_FAFNIR,TRUE);
#else
			// if(u.sealsActive&SEAL_FAFNIR && money_cnt(invent) < u.ulevel*100) unbind(SEAL_FAFNIR,TRUE);
			if(u.sealsActive&SEAL_FAFNIR && money_cnt(invent) == 0) unbind(SEAL_FAFNIR,TRUE);
#endif
			if(u.sealsActive&SEAL_JACK && (Is_astralevel(&u.uz) || Inhell)) unbind(SEAL_JACK,TRUE);
			if(u.sealsActive&SEAL_ORTHOS && !(Darksight || Catsight || Extramission)
				&&!(
					(viz_array[u.uy][u.ux]&TEMP_LIT3 && !(viz_array[u.uy][u.ux]&TEMP_DRK3)) || 
					(levl[u.ux][u.uy].lit && !(viz_array[u.uy][u.ux]&TEMP_DRK3 && !(viz_array[u.uy][u.ux]&TEMP_LIT3)))
				   )
			){
				if(Elfsight){
					if(++u.orthocounts>(5*3)) unbind(SEAL_ORTHOS,TRUE);
				} else if(Lowlightsight){
					if(++u.orthocounts>(5*2)) unbind(SEAL_ORTHOS,TRUE);
				} else {
					if(++u.orthocounts>5) unbind(SEAL_ORTHOS,TRUE);
				}
				if(u.sealsActive&SEAL_ORTHOS && Hallucination){ /*Didn't just unbind it*/
					if(u.orthocounts == 1) pline("It is now pitch black. You are likely to be eaten by a grue.");
					else pline("You are likely to be eaten by a grue.");
				} else You_feel("increasingly panicked about being in the dark!");
			    if (multi >= 0) {
					if (occupation)
						stop_occupation();
					else
						nomul(0, NULL);
				}
			}
			if(u.sealsActive&SEAL_NABERIUS && u.udrunken < u.ulevel/3) unbind(SEAL_NABERIUS,TRUE);
			if(u.specialSealsActive&SEAL_NUMINA && u.ulevel<30) unbind(SEAL_SPECIAL|SEAL_NUMINA,TRUE);
			if(u.sealsActive&SEAL_SHIRO && uarmc && uarmc->otyp == MUMMY_WRAPPING){
				struct obj *otmp = uarmc;
				pline("Hey, people might notice me with that!");
				if (donning(otmp)) cancel_don();
				(void) Cloak_off();
				useup(otmp);
				unbind(SEAL_SHIRO,TRUE);
			}
////////////////////////////////////////////////////////////////////////////////////////////////
			//These artifacts may need to respond to what monsters have done.
			///If the player no longer meets the kusanagi's requirements (ie, they lost the amulet)
			///blast 'em and drop to the ground.
			if (uwep && uwep->oartifact == ART_KUSANAGI_NO_TSURUGI){
				if(!touch_artifact(uwep,&youmonst, FALSE)){
					dropx(uwep);
				}
			}
			///If the Atma weapon becomes cursed, it drains life from the wielder to stay lit.
			if(uwep && uwep->oartifact == ART_ATMA_WEAPON &&
				uwep->cursed && 
				uwep->lamplit
			){
				if (!Drain_resistance) {
					pline("%s for a moment, then %s brightly.",
					  Tobjnam(uwep, "flicker"), otense(uwep, "burn"));
					losexp("life force drain",TRUE,TRUE,TRUE);
					uwep->cursed = FALSE;
				}
			}
////////////////////////////////////////////////////////////////////////////////////////////////
		    if (youmonst.movement >= NORMAL_SPEED)
			break;	/* it's now your turn */
		} while (monscanmove);
		flags.mon_moving = FALSE;

		if (!monscanmove && youmonst.movement < NORMAL_SPEED) {
		    /* both you and the monsters are out of steam this round */
		    /* set up for a new turn */
		    mcalcdistress();	/* adjust monsters' trap, blind, etc */

		    /* reallocate movement rations to monsters */
			flags.goldka_level=0;
			flags.silence_level=0;
			flags.spore_level=0;
			flags.slime_level=0;
			flags.walky_level=0;
			flags.shade_level=0;
		    for (mtmp = fmon; mtmp; mtmp = nxtmon){
				nxtmon = mtmp->nmon;
				/* check for bad swap weapons */
				if (mtmp->msw) {
					struct obj *obj;

					for(obj = mtmp->minvent; obj; obj = obj->nobj)
						if (obj->owornmask & W_SWAPWEP) break;
					if (!obj || mtmp->msw != obj){
						MON_NOSWEP(mtmp);
						impossible("bad monster swap weapon detected (and fixed)");
					}
				}
				/* Some bugs cause mtame and mpeaceful to diverge, fix w/ a warning */
				if(mtmp->mtame && !mtmp->mpeaceful){
					impossible("Hostile+tame monster state detected (and fixed)");
					mtmp->mpeaceful = TRUE;
				}
				//Remove after testing (can cause "re-trapping" of untrapped monsters)
				if(!mtmp->mtrapped && t_at(mtmp->mx, mtmp->my) && t_at(mtmp->mx, mtmp->my)->ttyp == VIVI_TRAP && !DEADMONSTER(mtmp)){
					impossible("Re-trapping mon %s in vivi trap",noit_mon_nam(mtmp));
					mtmp->mtrapped = TRUE;
				}
				/* Possibly vanish */
				if(mtmp->mvanishes>-1){
					if(--mtmp->mvanishes == 0){
						monvanished(mtmp);
						continue;
					}
				}
				if(noactions(mtmp)){
					/* Monsters in a essence trap can't move */
					if(mtmp->mtrapped && t_at(mtmp->mx, mtmp->my) && t_at(mtmp->mx, mtmp->my)->ttyp == VIVI_TRAP){
						mtmp->mhp = 1;
						mtmp->movement = 0;
						continue;
					}
					if(mtmp->entangled){
						if(mtmp->entangled == SHACKLES){
							mtmp->mhp = min(mtmp->mhp, mtmp->mhpmax/2+1);
							mtmp->movement = 0;
							continue;
						}
						else if(!mbreak_entanglement(mtmp)){
							mtmp->movement = 0;
							mescape_entanglement(mtmp);
							continue;
						}
					}
				}
				/* Possibly wake up */
				if(mtmp->mtame && mtmp->mstrategy&STRAT_WAITMASK){
					mtmp->mstrategy &= ~STRAT_WAITMASK;
				}
				/* Possibly become hostile */
				if(mtmp->mpeacetime && !mtmp->mtame){
					mtmp->mpeacetime--;
					if(!mtmp->mpeacetime) mtmp->mpeaceful = FALSE;
				}
				/* Possibly change hostility */
				if(mtmp->data == &mons[PM_SURYA_DEVA]){
					struct monst *blade;
					for(blade = fmon; blade; blade = blade->nmon)
						if(blade->data == &mons[PM_DANCING_BLADE] && mtmp->m_id == blade->mvar1) break;
					if(blade){
						if(mtmp->mtame && !blade->mtame){
							if(blade == nxtmon) nxtmon = nxtmon->nmon;
							tamedog(blade, (struct obj *) 0);
						} else if(!mtmp->mtame && blade->mtame){
							blade->mtame = 0;
						}
						if(mtmp->mpeaceful != blade->mpeaceful){
							mtmp->mpeaceful = blade->mpeaceful;
						}
					}
				}
				
				/*Reset fracture flag*/
				if(mtmp->zombify && is_kamerel(mtmp->data)) mtmp->zombify = 0;
				
				if(mtmp->data == &mons[PM_ARA_KAMEREL]) flags.goldka_level=1;
				if(mtmp->data == &mons[PM_ASPECT_OF_THE_SILENCE]){
					flags.silence_level=1;
					u.uen -= 3;
					if(!Race_if(PM_INCANTIFIER))
						u.uen = max(u.uen, 0);
				}
				if(mtmp->data == &mons[PM_ZUGGTMOY]) flags.spore_level=1;
				if(mtmp->data == &mons[PM_JUIBLEX]) flags.slime_level=1;
				if(mtmp->data == &mons[PM_PALE_NIGHT] || mtmp->data == &mons[PM_DREAD_SERAPH] || mtmp->data == &mons[PM_LEGION]) flags.walky_level=1;
				if(mtmp->data == &mons[PM_ORCUS] || mtmp->data == &mons[PM_NAZGUL]) flags.shade_level=1;
				if(mtmp->data == &mons[PM_DREAD_SERAPH] && (mtmp->mstrategy & STRAT_WAITMASK) && (u.uevent.invoked || (Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)))){
					mtmp->mstrategy &= ~STRAT_WAITMASK;
					pline_The("entire %s is shaking around you!",
						   In_endgame(&u.uz) ? "plane" : "dungeon");
					do_earthquake(mtmp->mx, mtmp->my, min(((int)mtmp->m_lev - 1) / 3 + 1,24), 3, TRUE, mtmp);
					if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)){
						digcrater(mtmp);
					} else if(Is_lolth_level(&u.uz)){
						digcloudcrater(mtmp);
					} else if(rn2(2)){ //Do for x
						digXchasm(mtmp);
					} else { //Do for y
						digYchasm(mtmp);
					}
				}
				
				if(mtmp->data == &mons[PM_GOLD_GOLEM]){
					int golds = u.goldkamcount_tame + level.flags.goldkamcount_peace + level.flags.goldkamcount_hostile;
					if(golds > 0){
						if(canseemon_eyes(mtmp)){
							pline("%s blossoms into a swirl of mirrored arcs!", Monnam(mtmp));
							You("see the image of %s reflected in the golden mirrors!", an(mons[PM_ARA_KAMEREL].mname));
						}
						set_mon_data(mtmp, &mons[PM_ARA_KAMEREL], 0);
						mtmp->m_lev = 15;
						mtmp->mhpmax = d(15, 8);
						mtmp->mhp = mtmp->mhpmax;
						if(mtmp->mnamelth) mtmp = christen_monst(mtmp, ""); //Now a different entity
						mtmp->movement = 9;//Don't pause for a turn
						golds = rnd(golds);
						
						golds -= u.goldkamcount_tame;
						if(golds <= 0){
							u.goldkamcount_tame--;
							if(!mtmp->mtame)
								mtmp = tamedog(mtmp, (struct obj *) 0);
							newsym(mtmp->mx,mtmp->my);
							if(!mtmp)
								continue; //something went wrong, and now mtmp is no good
							goto karemade;
						}
						golds -= level.flags.goldkamcount_peace;
						if(golds <= 0){
							level.flags.goldkamcount_peace--;
							mtmp->mtame = 0;
							mtmp->mpeaceful = 1;
							newsym(mtmp->mx,mtmp->my);
							goto karemade;
						}
						level.flags.goldkamcount_hostile--;
						mtmp->mtame = 0;
						mtmp->mpeaceful = 0;
						newsym(mtmp->mx,mtmp->my);
karemade:						
						set_malign(mtmp);
					}
				}
				
				if(mtmp->data == &mons[PM_DREADBLOSSOM_SWARM]){
					if(canseemon(mtmp) || u.ustuck == mtmp) mtmp->movement += mcalcmove(mtmp);
					else {
						struct monst *tmpm;
						for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
							if(!(is_fey(tmpm->data) || is_plant(tmpm->data)) && mon_can_see_mon(tmpm,mtmp)){
								mtmp->movement += mcalcmove(mtmp);
								break;
							}
						}
					}
				} else mtmp->movement += mcalcmove(mtmp);
				
				if(mtmp->moccupation && !occupation){
					mtmp->moccupation = 0;
					mtmp->mcanmove = 1;
				}
				if(!mtmp->mnotlaugh){
					if(!is_silent_mon(mtmp)){
						wake_nearto_noisy(mtmp->mx, mtmp->my, combatNoise(mtmp->data));
						if(sensemon(mtmp) || (canseemon(mtmp) && !mtmp->mundetected)){
							pline("%s is laughing hysterically.", Monnam(mtmp));
						} else if(couldsee(mtmp->mx,mtmp->my)){
							You_hear("hysterical laughter.");
						} else {
							You_hear("laughter in the distance.");
						}
					} else if(sensemon(mtmp) || (canseemon(mtmp) && !mtmp->mundetected))
						pline("%s is trembling hysterically.", Monnam(mtmp));
				}
//ifdef BARD
				if (mtmp->encouraged && (!rn2(4))) {
					if(mtmp->encouraged > 0) mtmp->encouraged--;
					else mtmp->encouraged++;
					if (!(mtmp->encouraged) && canseemon(mtmp) && mtmp->mtame) 
						pline("%s looks calm again.", Monnam(mtmp));
				}
//endif
				if(mtmp->data == &mons[PM_GREAT_CTHULHU] || mtmp->data == &mons[PM_ZUGGTMOY] 
					|| mtmp->data == &mons[PM_SWAMP_FERN]) mtmp->mspec_used = 0;
				if(is_weeping(mtmp->data)) mtmp->mspec_used = 0;
				if(mtmp->data == &mons[PM_CLOCKWORK_SOLDIER] || mtmp->data == &mons[PM_CLOCKWORK_DWARF] || 
				   mtmp->data == &mons[PM_FABERGE_SPHERE] || mtmp->data == &mons[PM_FIREWORK_CART] ||
				   mtmp->data == &mons[PM_ID_JUGGERNAUT]
				) if(rn2(2)) mtmp->mvar1 = ((int)mtmp->mvar1 + rn2(3)-1)%8;
				if((mtmp->data == &mons[PM_JUGGERNAUT] || mtmp->data == &mons[PM_ID_JUGGERNAUT]) && !rn2(3)){
					int mdx=0, mdy=0, i;
					if(mtmp->mux == 0 && mtmp->muy == 0){
						i = rn2(8);
						mdx = xdir[i];
						mdy = ydir[i];
					} else {
						if(mtmp->mux - mtmp->mx < 0) mdx = -1;
						else if(mtmp->mux - mtmp->mx > 0) mdx = +1;
						if(mtmp->muy - mtmp->my < 0) mdy = -1;
						else if(mtmp->muy - mtmp->my > 0) mdy = +1;
						for(i=0;i<8;i++) if(xdir[i] == mdx && ydir[i] == mdy) break;
					}
					if(mtmp->mvar1 != i){
						if(sensemon(mtmp) || (canseemon(mtmp) && !mtmp->mundetected)){
							pline("%s turns to a new heading.", Monnam(mtmp));
						} else if(couldsee(mtmp->mx,mtmp->my)){
							You_hear("a loud scraping noise.");
						} else {
							You_hear("scraping in the distance.");
						}
						mtmp->mvar1 = i;
						mtmp->movement = -12;
					}
				}
			} /* movement rations */
			
			static boolean LBbreach = FALSE;
			static boolean LBproxim = FALSE;
			static boolean LBperim = FALSE;
			if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz) && Is_qstart(&u.uz) && !(quest_status.leader_is_dead)){
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) if(!mtmp->mpeaceful && mtmp->mx <= 23) break;
				if(mtmp && !(LBbreach && moves%5)) {
					verbalize("**EMERGENCY ALERT: hostile entities detected within Last Bastion**");
					LBbreach = TRUE;
					(void) makemon((struct permonst *)0, xdnstair, ydnstair, MM_ADJACENTOK);
				} else if(!mtmp) LBbreach = FALSE;
				
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) if(!mtmp->mpeaceful && mtmp->mx <= 26 && mtmp->mx > 23) break;
				if(mtmp && !LBbreach && !(LBproxim && moves%10)) {
					verbalize("*PROXIMITY ALERT: hostile entities detected outside Last Bastion*");
					LBproxim = TRUE;
				} else if(!mtmp) LBproxim = FALSE;
				
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) if(!mtmp->mpeaceful && mtmp->mx <= 55 && mtmp->mx > 26) break;
				if(mtmp && !LBperim) {
					if(mtmp) verbalize("*PERIMETER ALERT: hostile entities closing on Last Bastion*");
					LBperim = TRUE;
				} else if(!mtmp) LBperim = FALSE;
			}
			//Random monster generation block
			if(In_mithardir_terminus(&u.uz) &&
				mvitals[PM_ASPECT_OF_THE_SILENCE].born == 0 &&
				((u.ufirst_light && u.ufirst_sky)
				|| (u.ufirst_light && u.ufirst_life)
				|| (u.ufirst_sky && u.ufirst_life)
			)){
				makemon(&mons[PM_ASPECT_OF_THE_SILENCE], 0, 0, NO_MM_FLAGS);
			}
		    else if(!(Is_illregrd(&u.uz) && u.ualign.type == A_LAWFUL && !u.uevent.uaxus_foe) && /*Turn off random generation on axus's level if lawful*/
				!rn2(u.uevent.udemigod ? 25 :
				(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz) && !(Is_qstart(&u.uz) && !(quest_status.leader_is_dead))) ? 35 :
			    (depth(&u.uz) > depth(&stronghold_level)) ? 50 : 70)
			){
				if(Is_ford_level(&u.uz)){
					if(rn2(2)){
						int x, y, tries = 200;
						do x = rn2(COLNO/2) + COLNO/2 + 1, y =  rn2(ROWNO-2)+1;
						while((!isok(x,y) || !(levl[x][y].typ == SOIL || levl[x][y].typ == ROOM)) && tries-- > 0);
						if(tries >= 0)
							makemon(ford_montype(1), x, y, MM_ADJACENTOK);
					} else {
						int x, y, tries = 200;
						do x = rn2(COLNO/2) + 1, y =  rn2(ROWNO-2)+1;
						while((!isok(x,y) || !(levl[x][y].typ == SOIL || levl[x][y].typ == ROOM)) && tries-- > 0);
						if(tries >= 0)
							makemon(ford_montype(-1), x, y, MM_ADJACENTOK);
					}
				} else if(!(mvitals[PM_HOUND_OF_TINDALOS].mvflags&G_GONE && !In_quest(&u.uz)) && (level_difficulty()+u.ulevel)/2+5 > monstr[PM_HOUND_OF_TINDALOS] && u.uinsight > rn2(INSIGHT_RATE)){
					int x, y;
					for(x = 1; x < COLNO; x++)
						for(y = 0; y < ROWNO; y++){
							if(IS_CORNER(levl[x][y].typ) && couldsee(x, y) && rn2(45) < u.ulevel){
								flags.cth_attk=TRUE;//state machine stuff.
								create_gas_cloud(x, y, 4, 30);
								flags.cth_attk=FALSE;
								makemon(&mons[PM_HOUND_OF_TINDALOS], x, y, 0);
							}
						}
				} else {
					if (u.uevent.invoked && xupstair && rn2(10)) {
						(void) makemon((struct permonst *)0, xupstair, yupstair, MM_ADJACENTOK);
					} //TEAM ATTACKS
					if(In_sokoban(&u.uz)){
						if(u.uz.dlevel != 1 && u.uz.dlevel != 4) makemon((struct permonst *)0, xupstair, yupstair, MM_ADJACENTSTRICT|MM_ADJACENTOK);
					} else if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz) && Is_qstart(&u.uz) && !(quest_status.leader_is_dead)){
						(void) makemon((struct permonst *)0, xdnstair, ydnstair, MM_ADJACENTOK);
						(void) makemon((struct permonst *)0, xdnstair, ydnstair, MM_ADJACENTOK);
						(void) makemon((struct permonst *)0, xdnstair, ydnstair, MM_ADJACENTOK);
						(void) makemon((struct permonst *)0, xdnstair, ydnstair, MM_ADJACENTOK);
					}
					else (void) makemon((struct permonst *)0, 0, 0, NO_MM_FLAGS);
				}
			}
			if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz) && !(Is_qstart(&u.uz) && !Race_if(PM_ANDROID)) && !rn2(35)){
				struct monst* mtmp = makemon(&mons[PM_SEMBLANCE], rn1(COLNO-3,2), rn1(ROWNO-3,2), MM_ADJACENTOK);
				//"Where stray illuminations from the Far Realm leak onto another plane, matter stirs at the beckoning of inexplicable urges before burning to ash."
				if(mtmp && canseemon(mtmp)) pline("The base matter of the world stirs at the beckoning of inexplicable urges, dancing with a semblance of life.");
			}

		    /* reset summon monster block. */
			if(u.summonMonster) u.summonMonster = FALSE;

		    /* calculate how much time passed. */
#ifdef STEED
		    if (u.usteed && u.umoved) {
			/* your speed doesn't augment steed's speed */
			moveamt = mcalcmove(u.usteed);
			moveamt += P_SKILL(P_RIDING)-1; /* -1 to +3 */
			if(uclockwork){
				if(u.ucspeed == HIGH_CLOCKSPEED){
					/*You are still burning spring tension, even if it doesn't affect your speed!*/
					if(u.slowclock < 3) morehungry(3-u.slowclock);
					else if(!(moves%(u.slowclock - 2))) morehungry(1);
				}
				if(u.phasengn){
					//Phasing mount as well
					morehungry(10);
				}
			}
		    } else
#endif
		    {
			moveamt = youmonst.data->mmove;
			if(!Upolyd && Race_if(PM_HALF_DRAGON) && !(Role_if(PM_NOBLEMAN) && flags.initgend)) moveamt = (moveamt*2)/3;
			if(uarmf && uarmf->otyp == STILETTOS && !Flying && !Levitation) moveamt = (moveamt*5)/6;
			
			if(u.sealsActive&SEAL_EURYNOME && IS_PUDDLE_OR_POOL(levl[u.ux][u.uy].typ)){
				if (Very_fast) {	/* speed boots or potion */
					/* average movement is 1.78 times normal */
					moveamt += 2*NORMAL_SPEED / 3;
					if (rn2(3) == 0) moveamt += NORMAL_SPEED / 3;
				} else if (Fast) {
					/* average movement is 1.56 times normal */
					moveamt += NORMAL_SPEED / 3;
					if (rn2(3) != 0) moveamt += NORMAL_SPEED / 3;
				} else {
					/* average movement is 1.33 times normal */
					if (rn2(3) != 0) moveamt += NORMAL_SPEED / 2;
				}
			} else {
				if (Very_fast) {	/* speed boots or potion */
					/* average movement is 1.67 times normal */
					moveamt += NORMAL_SPEED / 2;
					if (rn2(3) == 0) moveamt += NORMAL_SPEED / 2;
				} else if (Fast) {
					/* average movement is 1.33 times normal */
					if (rn2(3) != 0) moveamt += NORMAL_SPEED / 2;
				}
			}
			
			if(uwep && uwep->oartifact == ART_SINGING_SWORD && uwep->osinging == OSING_HASTE){
				moveamt += 2;
			}
			
			if(u.specialSealsActive&SEAL_BLACK_WEB && u.utrap && u.utraptype == TT_WEB)
				moveamt += 8;
			
			if(u.sealsActive&SEAL_ENKI){
				nmonsclose = nmonsnear = 0;
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon){
					if(mtmp->mpeaceful) continue;
					if(distmin(u.ux, u.uy, mtmp->mx,mtmp->my) <= 1){
						nmonsclose++;
						nmonsnear++;
					} else if(distmin(u.ux, u.uy, mtmp->mx,mtmp->my) <= 2){
						nmonsnear++;
					}
				}
				enkispeedlim = u.ulevel/10+1;
				if(nmonsclose>1){
					moveamt += min(enkispeedlim,nmonsclose);
					nmonsnear -= min(enkispeedlim,nmonsclose);
				}
				if(nmonsnear>3) moveamt += min(enkispeedlim,nmonsnear-2);
			}
			if (uwep && uwep->oartifact == ART_GARNET_ROD) moveamt += NORMAL_SPEED / 2;
			if (uwep && uwep->oartifact == ART_TENSA_ZANGETSU){
				moveamt += NORMAL_SPEED;
				if(u.ZangetsuSafe-- < 1){
					if(ublindf && ublindf->otyp == MASK && is_undead(&mons[ublindf->corpsenm])){
						u.ZangetsuSafe = mons[ublindf->corpsenm].mlevel;
						if(ublindf->ovar1>=3){
							Your("mask shatters!");
							useup(ublindf);
						} else {
							Your("mask cracks.");
							ublindf->ovar1++;
						}
					} else {
						u.ZangetsuSafe = 0;
						losehp(5, "inadvisable haste", KILLED_BY);
						if (Upolyd) {
							if(u.mhmax > u.ulevel && moves % 2){
								u.uhpmod--;
								calc_total_maxhp();
							}
						}
						else{
							if(u.uhpmax > u.ulevel && moves % 2){
								u.uhpmod--;
								calc_total_maxhp();
							}
						}
	//					if( !(moves % 5) )
						You_feel("your %s %s!", 
							body_part(BONES), rnd(6) ? body_part(CREAK) : body_part(CRACK));
						exercise(A_CON, FALSE);
						exercise(A_STR, FALSE);
					}
				}
			}
			else if(u.ZangetsuSafe < u.ulevel && !(moves%10)) u.ZangetsuSafe++;
			
			if(!(moves%10)){
				if(u.eurycounts) u.eurycounts--;
				if(u.orthocounts){
					if(u.nv_range){
						u.orthocounts -= u.nv_range;
						if(u.orthocounts < 0) u.orthocounts = 0;
					} else {
						u.orthocounts--;
					}
				}
				if(u.wimage){
					if(u.wimage >= 10){
						exercise(A_INT, TRUE);
						exercise(A_WIS, FALSE);
					} else if(!(moves%10)) u.wimage--;
				}
				if(u.umorgul){
					int i = A_CON;
					int n = u.umorgul;
					if(ACURR(A_WIS)>=ACURR(i))
						i = A_WIS;
					if(ACURR(A_CHA)>=ACURR(i))
						i = A_CHA;
					while(n-- > 0){
						exercise(i, FALSE);
					}
				}
			}
			
			if(u.sealsActive&SEAL_ASTAROTH && u.uinwater){
				losehp(1, "rusting through", KILLED_BY);
			}
			
			if(uclockwork && u.ucspeed == HIGH_CLOCKSPEED){
				int hungerup;
				moveamt *= 2;
				hungerup = 2*moveamt/NORMAL_SPEED - 1;
				if(u.slowclock < hungerup) morehungry(hungerup-u.slowclock);
				else if(!(moves%(u.slowclock - hungerup + 1))) morehungry(1);
			}
			if(uandroid && u.ucspeed == HIGH_CLOCKSPEED){
				if (rn2(3) != 0) moveamt += NORMAL_SPEED / 2;
			}
			if(active_glyph(ANTI_CLOCKWISE_METAMORPHOSIS))
				moveamt += 3;
			if(u.uuur_duration)
				moveamt += 6;
			if(uwep && is_lightsaber(uwep) && litsaber(uwep) && u.fightingForm == FFORM_SORESU && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm))){
				switch(min(P_SKILL(FFORM_SORESU), P_SKILL(weapon_type(uwep)))){
					case P_BASIC:       moveamt = max(moveamt-6,1); break;
					case P_SKILLED:     moveamt = max(moveamt-4,1); break;
					case P_EXPERT:      moveamt = max(moveamt-3,1); break;
				}
			}
			if(youracedata->mmove){
				if(moveamt < 1) moveamt = 1;
			} else {
				if(moveamt < 0) moveamt = 0;
			}
			}
			
			if(uclockwork && u.phasengn){
				morehungry(10);
			}
			
			if(uclockwork && u.ucspeed == SLOW_CLOCKSPEED)
				moveamt /= 2; /*Even if you are mounted, a slow clockspeed affects how 
								fast you can issue commands to the mount*/
			
		    switch (wtcap) {
			case UNENCUMBERED: break;
			case SLT_ENCUMBER: moveamt -= (moveamt / 4); break;
			case MOD_ENCUMBER: moveamt -= (moveamt / 2); break;
			case HVY_ENCUMBER: moveamt -= ((moveamt * 3) / 4); break;
			case EXT_ENCUMBER: moveamt -= ((moveamt * 7) / 8); break;
			default: break;
		    }
			
		    if(In_fog_cloud(&youmonst)) moveamt = max(moveamt/3, 1);
		    youmonst.movement += moveamt;
			//floor how far into movement-debt you can fall.
		    if (youmonst.movement < -2*NORMAL_SPEED) youmonst.movement = -2*NORMAL_SPEED;
			
			if(u.uentangled){ //Note: the normal speed calculations include important hunger modifiers, so just calculate speed then 0 it out if needed.
				if(!ubreak_entanglement()){
					youmonst.movement = 0;
					if(u.uentangled == RAZOR_WIRE){
						int dmg = d(1,6);
						int beat;
						if(hates_silver(youracedata) && entangle_material(&youmonst, SILVER))
							dmg += rnd(20);
						if(hates_iron(youracedata) && entangle_material(&youmonst, SILVER))
							dmg += rnd(u.ulevel);
						beat = entangle_beatitude(&youmonst, -1);
						if(hates_unholy(youracedata) && beat)
							dmg += beat == 2 ? d(2,9) : rnd(9);
						beat = entangle_beatitude(&youmonst, 1);
						if(hates_holy(youracedata) && beat)
							dmg += beat == 2 ? rnd(20) : rnd(4);
						losehp(dmg, "being sliced to ribbons by razor wire", KILLED_BY);
					}
					uescape_entanglement();
				}
			}
			
		    settrack();

		    monstermoves++;
		    moves++;

		      /********************************/
		     /* once-per-turn things go here */
		    /********************************/
			/* Environment effects */
			dust_storm();
			/* Item attacks */
			mind_blast_items();
			if(uarm && 
				(uarm->otyp == LIVING_ARMOR || uarm->otyp == BARNACLE_ARMOR)
			) dosymbiotic();
			if(u.spiritPColdowns[PWR_PSEUDONATURAL_SURGE] >= moves+20)
				dopseudonatural();
			/* Clouds on Lolth's level deal damage */
			if(Is_lolth_level(&u.uz) && levl[u.ux][u.uy].typ == CLOUD){
				if (!(nonliving(youracedata) || Breathless)){
					if (!Blind)
						make_blinded((long)rnd(4), FALSE);
					if (!Poison_resistance) {
						pline("%s is burning your %s!", Something, makeplural(body_part(LUNG)));
						You("cough and spit blood!");
						losehp(d(3,8) + ((Amphibious && !flaming(youracedata)) ? 0 : rnd(6)), "stinking cloud", KILLED_BY_AN);
					} else if (!(Amphibious && !flaming(youracedata))){
						You("are laden with moisture and %s",
							flaming(youracedata) ? "are smoldering out!" :
							Breathless ? "find it mildly uncomfortable." :
							amphibious(youracedata) ? "feel comforted." :
							"can barely breathe!");
						/* NB: Amphibious includes Breathless */
						if (!(Amphibious && !flaming(youracedata))) losehp(rnd(6), "suffocating in a cloud", KILLED_BY);
					} else {
						You("cough!");
					}
				} else {
					You("are laden with moisture and %s",
						flaming(youracedata) ? "are smoldering out!" :
						Breathless ? "find it mildly uncomfortable." :
						amphibious(youracedata) ? "feel comforted." :
						"can barely breathe!");
					/* NB: Amphibious includes Breathless */
					if (!(Amphibious && !flaming(youracedata))) losehp(rnd(6), "suffocating in a cloud", KILLED_BY);
				}
			}
			/* If the player has too many pets, untame them untill that is no longer the case */
			{
				// finds weakest pet, and if there's more than 6 pets that count towards your limit
				// it sets the weakest one friendly - dog.c
				enough_dogs(0);
				
				// if there's a spiritual pet that isn't already marked for vanishing,
				// give it 5 turns before it disappears. - dog.c
				vanish_dogs();
			}
			
			if(u.petattacked){
				u.petattacked = FALSE;
				use_skill(P_BEAST_MASTERY, 1);
			}
			if(u.pethped){
				u.pethped = FALSE;
				more_experienced(u.ulevel,0);
				newexplevel();
			}
			if (u.uencouraged && (!rn2(4))) {
				if(u.uencouraged > 0) u.uencouraged--;
				else u.uencouraged++;
				if (!(u.uencouraged)) 
					You_feel("calm again.");
			}
			
		    if (flags.bypasses) clear_bypasses();
		    if(Glib) glibr();
		    nh_timeout();
		    run_regions();
		    run_maintained_spells();
			
			move_gliders();

		    if (u.ublesscnt)  u.ublesscnt--;
		    if(flags.time && !flags.run)
			flags.botl = 1;
			
			if(uclockwork){
				if(u.ustove){
					if(u.uboiler){
						int steam = min(10,min(u.ustove,u.uboiler));
						lesshungry(steam);
						u.ustove -= min(10,u.ustove);
						u.uboiler-=steam;
						flags.cth_attk=TRUE;//state machine stuff.
						create_gas_cloud(u.ux+rn2(3)-1, u.uy+rn2(3)-1, 1, rnd(3));
						flags.cth_attk=FALSE;
						if(u.utemp && moves%2) u.utemp--;
					} else {
						if(u.utemp<=WARM || 
							(u.utemp<=HOT && (moves%4)) ||
							(u.utemp>HOT && u.utemp<=BURNING_HOT && (moves%3)) ||
							(u.utemp>BURNING_HOT && u.utemp<=MELTING && (moves%2)) ||
							(u.utemp>MELTING && u.utemp<=MELTED && !(moves%3)) ||
							(u.utemp>MELTED && !(moves%4))
						) u.utemp++;
						flags.cth_attk=TRUE;//state machine stuff.
						create_gas_cloud(u.ux+rn2(3)-1, u.uy+rn2(3)-1, 1, rnd(6)); //Longer-lived smoke
						flags.cth_attk=FALSE;
						u.ustove -= min(10,u.ustove);
					}
				} else if(u.utemp) u.utemp--;
				if(u.utemp > BURNING_HOT){
					if((u.utemp-5)*2 > rnd(10)) destroy_item(SCROLL_CLASS, AD_FIRE);
					if((u.utemp-5)*2 > rnd(10)) destroy_item(POTION_CLASS, AD_FIRE);
					if((u.utemp-5)*2 > rnd(10)) destroy_item(SPBOOK_CLASS, AD_FIRE);
					
					if(u.utemp >= MELTING && !(HFire_resistance || u.sealsActive&SEAL_FAFNIR)){
						You("are melting!");
						losehp(u.ulevel, "melting from extreme heat", KILLED_BY);
						if(u.utemp >= MELTED){
							if(Upolyd) losehp(u.mhmax*2, "melting to slag", KILLED_BY);
							else { /* May have been rehumanized by previous damage. In that case, still die from left over bronze on your skin! */
								if(uclockwork) losehp((Upolyd ? u.mhmax : u.uhpmax)*2, "melting to slag", KILLED_BY);
								else if(!(HFire_resistance || u.sealsActive&SEAL_FAFNIR)) losehp((Upolyd ? u.mhmax : u.uhpmax)*2, "molten bronze", KILLED_BY);
							}
						}
					}
				}
			} else if(u.utemp) u.utemp = 0;
			
			if(u.uinwater && !u.umoved){
				if(uclockwork) u.uboiler = MAX_BOILER;
			}
			
			if(u.ukinghill){
				if(u.protean > 0) u.protean--;
				else{
					for(pobj = invent; pobj; pobj=pobj->nobj)
						if(pobj->oartifact == ART_TREASURY_OF_PROTEUS)
							break;
					if(!pobj){
						pline("Treasury not actually in inventory??");
						u.ukinghill = 0;
					}
					else if(pobj->cobj){
						arti_poly_contents(pobj);
					}
					u.protean = 100 + d(10,10);
					update_inventory();
				}
			}

			// Ymir's stat regeneration
				if(u.sealsActive&SEAL_YMIR && (wtcap < MOD_ENCUMBER || !u.umoved || Regeneration)){
					if((u.ulevel > 9 && !(moves % 3)) || 
						(u.ulevel <= 9 && !(moves % ((MAXULEV+12) / (u.ulevel+2) + 1)))
					){
						int val_limit, idx;
						for (idx = 0; idx < A_MAX; idx++) {
							val_limit = AMAX(idx);
							/* don't recover strength lost from hunger */
							if (idx == A_STR && u.uhs >= WEAK) val_limit--;
							
							if (val_limit > ABASE(idx)) ABASE(idx)++;
						}
					}
				}

		    /* One possible result of prayer is healing.  Whether or
		     * not you get healed depends on your current hit points.
		     * If you are allowed to regenerate during the prayer, the
		     * end-of-prayer calculation messes up on this.
		     * Another possible result is rehumanization, which requires
		     * that encumbrance and movement rate be recalculated.
		     */
			you_regen_hp();
			you_regen_pw();
			androidUpkeep();

		    if(!(u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves+20)) {
			if(Teleportation && !rn2(85) && !(
#ifdef WIZARD
				(
#endif
				 (u.uhave.amulet || On_W_tower_level(&u.uz)
#ifdef STEED
				  || (u.usteed && mon_has_amulet(u.usteed))
#endif
				 )
#ifdef WIZARD
				 && (!wizard) )
#endif
			)) {
			    xchar old_ux = u.ux, old_uy = u.uy;
			    tele();
			    if (u.ux != old_ux || u.uy != old_uy) {
				if (!next_to_u()) {
				    check_leash(old_ux, old_uy);
				}
#ifdef REDO
				/* clear doagain keystrokes */
				pushch(0);
				savech(0);
#endif
			    }
			}
			/* delayed change may not be valid anymore */
			if ((change == 1 && !Polymorph) ||
			    (change == 2 && u.ulycn == NON_PM))
			    change = 0;
			if(Polymorph && !rn2(100))
			    change = 1;
			else if (u.ulycn >= LOW_PM && !Upolyd &&
				 !umechanoid&&
				 !rn2(80 - (20 * night())))
			    change = 2;
			if (change && !Unchanging) {
			    if (multi >= 0) {
				if (occupation)
				    stop_occupation();
				else
				    nomul(0, NULL);
				if (change == 1) polyself(FALSE);
				else you_were();
				change = 0;
			    }
			}
		    }

		    if(Searching && multi >= 0) (void) dosearch0(1);
		    dosounds();
		    do_storms();
		    gethungry();
		    age_spells();
		    exerchk();
		    invault();
		    if (u.uhave.amulet) amulet();
		    if (!rn2(40+(int)(ACURR(A_DEX)*3)))
			u_wipe_engr(rnd(3));
		    if (u.uevent.udemigod && !(u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves+20)) {
				if (u.udg_cnt) u.udg_cnt--;
				if (!u.udg_cnt) {
					intervene();
					u.udg_cnt = rn1(200, 50);
				}
		    }
		    if (u.uevent.ukilled_illurien && !(u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves+20)) {
				if (u.ill_cnt) u.ill_cnt--;
				if (!u.ill_cnt) {
					illur_intervene();
					u.ill_cnt = rn1(1000, 250);
				}
		    }
		    restore_attrib();
		    /* underwater and waterlevel vision are done here */
		    if (Is_waterlevel(&u.uz))
			movebubbles();
		    else if (Underwater)
			under_water(0);
		    /* vision while buried done here */
		    else if (u.uburied) under_ground(0);

		    /* when immobile, count is in turns */
		    if(multi < 0) {
			if (++multi == 0) {	/* finished yet? */
			    unmul((char *)0);
			    /* if unmul caused a level change, take it now */
			    if (u.utotype) deferred_goto();
			}
		    }
		}
	    } while (youmonst.movement<NORMAL_SPEED); /* hero can't move loop */

	      /******************************************/
	     /* once-per-hero-took-time things go here */
	    /******************************************/
		reset_udieroll();
		
		if(u.ustdy > 0) u.ustdy -= 1;
		
		if(u.specialSealsActive&SEAL_LIVING_CRYSTAL)
			average_dogs();
		
		//Mithardir portals
		if(In_mithardir_desert(&u.uz)){
			struct trap *ttmp;
			for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
				if(ttmp->ttyp == MAGIC_PORTAL && distmin(u.ux, u.uy, ttmp->tx, ttmp->ty) < 3){
					if(!ttmp->tseen){
						ttmp->tseen = 1;
						newsym(ttmp->tx, ttmp->ty);
						if(cansee(ttmp->tx, ttmp->ty)){
							You("spot a swirling portal!");
						} else {
							You_hear("swirling dust!");
						}
					}
				}
			}
		}
		
		//Echolocation
		if(Echolocation){
			for(i=1; i<COLNO; i++)
				for(j=0; j<ROWNO; j++)
					if(viz_array[j][i]&COULD_SEE)
						echo_location(i, j);
			see_monsters();
		}
		if(u.utrap && u.utraptype == TT_LAVA) {
			if(!is_lava(u.ux,u.uy))
			    u.utrap = 0;
		    else if (!(u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves+20)) {
			    u.utrap -= 1<<8;
				if(u.utrap < 1<<8) {
					killer_format = KILLED_BY;
					killer = "molten lava";
					You("sink below the surface and die.");
					done(DISSOLVED);
				} else if(didmove && !u.umoved) {
					Norep("You sink deeper into the lava.");
					u.utrap += rnd(4);
				}
			}
	    }
		if(!Blind){
			int dx, dy;
			
			for(dx=-1; dx<2; dx++){
				for(dy=-1; dy<2; dy++){
					if(isok(u.ux+dx, u.uy+dy)){
						if((mtmp = m_at(u.ux+dx, u.uy+dy)) && !mtmp->mtame && canseemon(mtmp) && !(mvitals[monsndx(mtmp->data)].seen)){
							mvitals[monsndx(mtmp->data)].seen = 1;
							if(Role_if(PM_TOURIST)){
								more_experienced(experience(mtmp,0),0);
								newexplevel();
							}
							if(u.usanity > 0 && taxes_sanity(mtmp->data)){
								change_usanity(u_sanity_loss(mtmp));
							}
							if(yields_insight(mtmp->data)){
								change_uinsight(u_insight_gain(mtmp));
								change_usanity(u_sanity_gain(mtmp));
							}
						}
					}
				}
			}
		}
		if(u.uinwater){//Moving around will also call this, so your stuff degrades faster if you move
			water_damage(invent, FALSE, FALSE, level.flags.lethe, &youmonst);
		}

		hpDiff -= u.uhp;
		hpDiff = (hpDiff > 0) ? hpDiff : 0;
		if(uarmg && ART_GAUNTLETS_OF_THE_BERSERKER == uarmg->oartifact){
			float a = .1; /* closer to 1 -> discard older faster */
			long next = (long)(a * hpDiff + (1 - a) * uarmg->ovar1);
			next = (next > 10) ? 10 : next;
			long diff = next - uarmg->ovar1;
			uarmg->ovar1 = next;
			//if(diff) adj_abon(uarmg, diff);
		}
		didmove = FALSE;
	} /* actual time passed */

	/****************************************/
	/* once-per-player-input things go here */
	/****************************************/
	find_ac();
////////////////////////////////////////////////////////////////////////////////////////////////
	for (mtmp = fmon; mtmp; mtmp = nxtmon){
		nxtmon = mtmp->nmon;
		if(mtmp->data == &mons[PM_HELLCAT]){
			if(!isdark(mtmp->mx,mtmp->my) && !mtmp->minvis){
				mtmp->minvis = TRUE;
				mtmp->perminvis = TRUE;
				newsym(mtmp->mx,mtmp->my);
			} else if(isdark(mtmp->mx,mtmp->my) && mtmp->minvis){
				mtmp->minvis = FALSE;
				mtmp->perminvis = FALSE;
				newsym(mtmp->mx,mtmp->my);
			}
		} else if(mtmp->data == &mons[PM_HUDOR_KAMEREL]){
			if(is_pool(mtmp->mx,mtmp->my, TRUE) && !mtmp->minvis){
				mtmp->minvis = TRUE;
				mtmp->perminvis = TRUE;
				newsym(mtmp->mx,mtmp->my);
			} else if(!is_pool(mtmp->mx,mtmp->my, TRUE) && mtmp->minvis){
				mtmp->minvis = FALSE;
				mtmp->perminvis = FALSE;
				newsym(mtmp->mx,mtmp->my);
			}
		} else if(mtmp->data == &mons[PM_GRUE] || mtmp->data == &mons[PM_INVIDIAK]){
			if(isdark(mtmp->mx,mtmp->my) && !mtmp->minvis){
				mtmp->minvis = TRUE;
				mtmp->perminvis = TRUE;
				newsym(mtmp->mx,mtmp->my);
			} else if(!isdark(mtmp->mx,mtmp->my) && mtmp->minvis){
				mtmp->minvis = FALSE;
				mtmp->perminvis = FALSE;
				newsym(mtmp->mx,mtmp->my);
			}
		} else if(mtmp->m_insight_level > u.uinsight){
			migrate_to_level(mtmp, ledger_no(&u.uz), MIGR_EXACT_XY, (coord *)0);
			continue;
		}
		if (mtmp->minvis){
			newsym(mtmp->mx, mtmp->my);
		}
		if (Blind && Bloodsense && has_blood_mon(mtmp)){
			newsym(mtmp->mx, mtmp->my);
		}
	}
////////////////////////////////////////////////////////////////////////////////////////////////
	if(!flags.mv || Blind || oldBlind != (!!Blind)) {
	    /* redo monsters if hallu or wearing a helm of telepathy */
	    if (Hallucination) {	/* update screen randomly */
			see_monsters();
			see_objects();
			see_traps();
			if (u.uswallow) swallowed(0);
		} else if (Unblind_telepat || goodsmeller(youracedata) || Warning || Warn_of_mon || u.usanity < 100 || oldsanity < 100) {
	     	see_monsters();
	    }
		
		oldsanity = u.usanity;

		switch (((u_healing_penalty() - healing_penalty) > 0) - ((u_healing_penalty() - healing_penalty) < 0))
		{
		case 0:
			break;
		case 1:
			if (!Hallucination){
				You_feel("%s.", (healing_penalty) ? "itchier" : "itchy");
			}
			else {
				You_feel("%s.", (healing_penalty) ? "uglier" : "ugly");
			}
			healing_penalty = u_healing_penalty();
			break;
		case -1:
			if (!Hallucination) {
				You_feel("%s.", (u_healing_penalty()) ? "some relief" : "relieved");
			}
			else {
				You_feel("%s.", (u_healing_penalty()) ? "pretty" : "beautiful");
			}
			healing_penalty = u_healing_penalty();
			break;
		}
		
		if (!oldBlind ^ !Blind) {  /* one or the other but not both */
			see_monsters();
			flags.botl = 1;
			vision_full_recalc = 1;	/* blindness just got toggled */
			if (Blind_telepat || Infravision) see_monsters();
		}
		
	    if (vision_full_recalc) vision_recalc(0);	/* vision! */
	}
	oldBlind = !!Blind;
////////////////////////////////////////////////////////////////////////////////////////////////
	if(!flags.mv || LightBlind || oldLightBlind != (!!LightBlind)) {
	    /* redo monsters if hallu or wearing a helm of telepathy */
	    if (Hallucination) {	/* update screen randomly */
			see_monsters();
			see_objects();
			see_traps();
			if (u.uswallow) swallowed(0);
		} else if (Unblind_telepat || goodsmeller(youracedata) || Warning || Warn_of_mon || u.usanity < 100 || oldsanity < 100) {
	     	see_monsters();
	    }
		
		oldsanity = u.usanity;

		if (!oldLightBlind ^ !LightBlind) {  /* one or the other but not both */
			see_monsters();
			flags.botl = 1;
			vision_full_recalc = 1;	/* blindness just got toggled */
			if (Blind_telepat || Infravision) see_monsters();
		}
		
	    if (vision_full_recalc) vision_recalc(0);	/* vision! */
	}
	oldLightBlind = !!LightBlind;
////////////////////////////////////////////////////////////////////////////////////////////////
	calc_total_maxhp();
////////////////////////////////////////////////////////////////////////////////////////////////
	calc_total_maxen();
////////////////////////////////////////////////////////////////////////////////////////////////
	/*If anything we did caused us to get moved out of water, surface*/
	if(u.usubwater && !is_pool(u.ux, u.uy, FALSE)){
		u.usubwater = 0;
		vision_full_recalc = 1;
		vision_recalc(2);	/* unsee old position */
		doredraw();
	} else if(Is_waterlevel(&u.uz) && u.usubwater && !is_3dwater(u.ux, u.uy)){
		You("pop out into an airbubble!");
		u.usubwater = 0;
	} else if(Is_waterlevel(&u.uz) && !u.usubwater && is_3dwater(u.ux, u.uy)){
		Your("%s goes under water!", body_part(HEAD));
		u.usubwater = 1;
	}
////////////////////////////////////////////////////////////////////////////////////////////////
	
	
#ifdef REALTIME_ON_BOTL
	if(iflags.showrealtime) {
		/* Update the bottom line if the number of minutes has
		 * changed */
		if(get_realtime() / 60 != realtime_data.last_displayed_time / 60)
			flags.botl = 1;
	}
#endif
  
	if(flags.botl || flags.botlx) bot();

	flags.move = 1;

	if(multi >= 0 && occupation) {
#if defined(MICRO) || defined(WIN32)
	    abort_lev = 0;
	    if (kbhit()) {
		if ((ch = Getchar()) == ABORT)
		    abort_lev++;
# ifdef REDO
		else
		    pushch(ch);
# endif /* REDO */
	    }
	    if (!abort_lev && (*occupation)() == 0)
#else
	    if ((*occupation)() == 0)
#endif
		occupation = 0;
	    if(
#if defined(MICRO) || defined(WIN32)
		   abort_lev ||
#endif
		   monster_nearby()) {
		stop_occupation();
		reset_eat();
	    }
#if defined(MICRO) || defined(WIN32)
	    if (!(++occtime % 7))
		display_nhwindow(WIN_MAP, FALSE);
#endif
	    continue;
	}

	if ((u.uhave.amulet || Clairvoyant) &&
	    !In_endgame(&u.uz) && !BClairvoyant &&
	    !(moves % 15) && !rn2(2))
		do_vicinity_map(u.ux,u.uy);

#ifdef WIZARD
	if (iflags.sanity_check)
	    sanity_check();
#endif

#ifdef CLIPPING
	/* just before rhack */
	cliparound(u.ux, u.uy);
#endif

	u.umoved = FALSE;

	if (multi > 0) {
	    lookaround();
	    if (!multi) {
		/* lookaround may clear multi */
		flags.move = 0;
		if (flags.time) flags.botl = 1;
		continue;
	    }
	    if (flags.mv) {
		if(multi < COLNO && !--multi)
		    flags.travel = iflags.travel1 = flags.mv = flags.run = 0;
		domove();
	    } else {
		--multi;
		rhack(save_cm);
	    }
	} else if (multi == 0) {
#ifdef MAIL
	    ckmailstatus();
#endif
	    rhack((char *)0);
	}
	if (u.utotype)		/* change dungeon level */
	    deferred_goto();	/* after rhack() */
	/* !flags.move here: multiple movement command stopped */
	else if (flags.time && (!flags.move || !flags.mv))
	    flags.botl = 1;

	if (vision_full_recalc) vision_recalc(0);	/* vision! */
	/* when running in non-tport mode, this gets done through domove() */
	if ((!flags.run || iflags.runmode == RUN_TPORT) &&
		(multi && (!flags.travel ? !(multi % 7) : !(moves % 7L)))) {
	    if (flags.time && flags.run) flags.botl = 1;
	    display_nhwindow(WIN_MAP, FALSE);
	}

	if (moves > last_clear_screen + 2000) doredraw();

    }
}

#endif /* OVL0 */
#ifdef OVL1

void
stop_occupation()
{
	struct monst *mtmp;
	if(occupation) {
		if (!maybe_finished_meal(TRUE))
		    You("stop %s.", occtxt);
		occupation = 0;
		flags.botl = 1; /* in case u.uhs changed */
		for (mtmp = fmon; mtmp; mtmp = mtmp->nmon){
			if(mtmp->moccupation && !occupation){
				mtmp->moccupation = 0;
				mtmp->mcanmove = 1;
			}
		}
/* fainting stops your occupation, there's no reason to sync.
		sync_hunger();
*/
#ifdef REDO
		nomul(0, NULL);
		pushch(0);
#endif
	}
}

#endif /* OVL1 */
#ifdef OVLB

void
display_gamewindows()
{
    WIN_MESSAGE = create_nhwindow(NHW_MESSAGE);
    WIN_STATUS = create_nhwindow(NHW_STATUS);
    WIN_MAP = create_nhwindow(NHW_MAP);
    WIN_INVEN = create_nhwindow(NHW_MENU);

#ifdef MAC
    /*
     * This _is_ the right place for this - maybe we will
     * have to split display_gamewindows into create_gamewindows
     * and show_gamewindows to get rid of this ifdef...
     */
	if ( ! strcmp ( windowprocs . name , "mac" ) ) {
	    SanePositions ( ) ;
	}
#endif

    /*
     * The mac port is not DEPENDENT on the order of these
     * displays, but it looks a lot better this way...
     */
    display_nhwindow(WIN_STATUS, FALSE);
    display_nhwindow(WIN_MESSAGE, FALSE);
    clear_glyph_buffer();
    display_nhwindow(WIN_MAP, FALSE);
}

void
newgame()
{
	int i;

#ifdef MFLOPPY
	gameDiskPrompt();
#endif

	flags.ident = 1;

	for (i = 0; i < NUMMONS; i++)
		mvitals[i].mvflags = mons[i].geno & G_NOCORPSE;

	init_objects();		/* must be before u_init() */
	
	flags.pantheon = -1;	/* role_init() will reset this */
	flags.panLgod = -1;	/* role_init() will reset this */
	flags.panNgod = -1;	/* role_init() will reset this */
	flags.panCgod = -1;	/* role_init() will reset this */
	role_init(TRUE);		/* must be before init_dungeons(), u_init(),
				 * and init_artifacts() */
	
	init_dungeons();	/* must be before u_init() to avoid rndmonst()
				 * creating odd monsters for any tins and eggs
				 * in hero's initial inventory */
	init_artifacts();	/* before u_init() in case $WIZKIT specifies
				 * any artifacts */
	u_init();
	
	hack_artifacts();	/* recall after u_init() to fix up role specific artifacts */

#ifndef NO_SIGNAL
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
#ifdef NEWS
	if(iflags.news) display_file(NEWS, FALSE);
#endif
	load_qtlist();	/* load up the quest text info */
/*	quest_init();*/	/* Now part of role_init() */

	mklev();
	u_on_upstairs();
	vision_reset();		/* set up internals for level (after mklev) */
	check_special_room(FALSE);

	flags.botlx = 1;

	/* Move the monster from under you or else
	 * makedog() will fail when it calls makemon().
	 *			- ucsfcgl!kneller
	 */
	if(MON_AT(u.ux, u.uy)) mnexto(m_at(u.ux, u.uy));
	(void) makedog();
	docrt();
#ifdef CONVICT
       if (Role_if(PM_CONVICT)) {
              setworn(mkobj(CHAIN_CLASS, TRUE), W_CHAIN);
              setworn(mkobj(BALL_CLASS, TRUE), W_BALL);
              uball->spe = 1;
              placebc();
              newsym(u.ux,u.uy);
       }
#endif /* CONVICT */

	if (flags.legacy) {
		flush_screen(1);
        if(Role_if(PM_EXILE)){
			com_pager(217);
#ifdef CONVICT
		} else if (Role_if(PM_CONVICT)) {
		    com_pager(199);
#endif /* CONVICT */
        // } else if(Race_if(PM_ELF)){
			// com_pager(211);
		} else if(Race_if(PM_ELF) && (Role_if(PM_PRIEST) || Role_if(PM_RANGER) || Role_if(PM_NOBLEMAN) || Role_if(PM_WIZARD))){
			com_pager(211);
		} else if(Role_if(PM_ANACHRONONAUT)){
			com_pager(218);
			if(Race_if(PM_ANDROID)){
				com_pager(222);
				com_pager(223);
				com_pager(224);
				com_pager(225);
			} else {
				com_pager(219);
				com_pager(220);
				com_pager(221);
			}
		} else if(Race_if(PM_WORM_THAT_WALKS)){
			if(Role_if(PM_CONVICT)){
				com_pager(214);
			} else if(Race_if(PM_ELF) && (Role_if(PM_PRIEST) || Role_if(PM_RANGER) || Role_if(PM_NOBLEMAN) || Role_if(PM_WIZARD))){
				com_pager(213);
			} else{
				com_pager(212);
			}
			com_pager(215);
			com_pager(216);
		} else {
			com_pager(1);
		}
	}

#ifdef INSURANCE
	save_currentstate();
#endif
	program_state.something_worth_saving++;	/* useful data now exists */

#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)

	/* Start the timer here */
	realtime_data.realtime = (time_t)0L;

#if defined(BSD) && !defined(POSIX_TYPES)
	(void) time((long *)&realtime_data.restoretime);
#else
	(void) time(&realtime_data.restoretime);
#endif

#endif /* RECORD_REALTIME || REALTIME_ON_BOTL */

	if(Darksight) litroom(FALSE,NULL);
	/* Success! */
	welcome(TRUE);
	if(Race_if(PM_INHERITOR)){
		int inherited;
		struct obj *otmp;
		do{inherited = do_inheritor_menu();}while(!inherited);
		otmp = mksobj((int)artilist[inherited].otyp, FALSE, FALSE);
	    otmp = oname(otmp, artilist[inherited].name);
		expert_weapon_skill(weapon_type(otmp));
		discover_artifact(inherited);
		fully_identify_obj(otmp);
	    otmp = hold_another_object(otmp, "Oops!  %s to the floor!",
				       The(aobjnam(otmp, "slip")), (const char *)0);
		if(otmp->oclass == WEAPON_CLASS)
			expert_weapon_skill(objects[otmp->otyp].oc_skill);
	    // otmp->oartifact = inherited;
	}
	return;
}

STATIC_OVL int
do_inheritor_menu()
{
	winid tmpwin;
	int n, how, i;
	char buf[BUFSZ];
	char incntlet = 'a';
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */

	for (i = 1; i<=NROFARTIFACTS; i++)
	{
		// if ((artilist[i].spfx2) && artilist[i].spfx && artilist[i].spfx)
		if(artilist[i].spfx&SPFX_INHER
		&& !Role_if(artilist[i].role)
		&& !Pantheon_if(artilist[i].role)
		&& (artilist[i].alignment == A_NONE
			|| artilist[i].alignment == u.ualign.type)
		){
			Sprintf(buf, "%s", artilist[i].name);
			any.a_int = i;	/* must be non-zero */
			add_menu(tmpwin, NO_GLYPH, &any,
				incntlet, 0, ATR_NONE, buf,
				MENU_UNSELECTED);
			incntlet = (incntlet == 'z') ? 'A' : (incntlet == 'Z') ? 'a' : (incntlet + 1);
		}
	}

	end_menu(tmpwin, "Which artifact did you inherit?");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return (n > 0) ? selected[0].item.a_int : 0;
}

/* show "welcome [back] to nethack" message at program startup */
void
welcome(new_game)
boolean new_game;	/* false => restoring an old game */
{
    char buf[BUFSZ];
    boolean currentgend = Upolyd ? u.mfemale : flags.female;

    /*
     * The "welcome back" message always describes your innate form
     * even when polymorphed or wearing a helm of opposite alignment.
     * Alignment is shown unconditionally for new games; for restores
     * it's only shown if it has changed from its original value.
     * Sex is shown for new games except when it is redundant; for
     * restores it's only shown if different from its original value.
     */
    *buf = '\0';
    if (new_game || u.ualignbase[A_ORIGINAL] != u.ualignbase[A_CURRENT])
	Sprintf(eos(buf), " %s", align_str(u.ualignbase[A_ORIGINAL]));
    if (!urole.name.f &&
	    (new_game ? (urole.allow & ROLE_GENDMASK) == (ROLE_MALE|ROLE_FEMALE) :
	     currentgend != flags.initgend))
	Sprintf(eos(buf), " %s", genders[currentgend].adj);

    pline(new_game ? "%s %s, welcome to dNetHack!  You are a%s %s %s."
		   : "%s %s, the%s %s %s, welcome back to dNetHack!",
	  Hello((struct monst *) 0), plname, buf, urace.adj,
	  (currentgend && urole.name.f) ? urole.name.f : urole.name.m);
	if(iflags.dnethack_start_text){
	pline("Press Ctrl^W or type #ward to engrave a warding sign.");
	if(Role_if(PM_PIRATE)) You("can swim! Type #swim while swimming on the surface to dive down to the bottom.");
	else if(Role_if(PM_EXILE)){
		pline("Press Ctrl^E or type #seal to engrave a seal of binding.");
		pline("#chat to a fresh seal to contact the spirit beyond.");
		pline("Press Ctrl^F or type #power to fire active spirit powers!");
	}
	if(Race_if(PM_DROW)){
		pline("Beware, droven armor evaporates in light!");
		pline("Use #monster to create a patch of darkness.");
	}
	if(Race_if(PM_ANDROID)){
		pline("Androids do not need to eat, but *do* need to sleep.");
		pline("Use #monster to access your innate abilities, including sleep.");
		pline("Use '.' to recover HP using magic energy.");
		pline("Sleep to recover magic energy.");
	}
	if(Race_if(PM_CLOCKWORK_AUTOMATON)){
		pline("Use #monster to adjust your clockspeed.");
		You("do not heal naturally. Use '.' to attempt repairs.");
	}
	if(Race_if(PM_INCANTIFIER)){
		pline("Incantifiers eat magic, not food, and do not heal naturally.");
	}
	}
}

#ifdef POSITIONBAR
STATIC_DCL void
do_positionbar()
{
	static char pbar[COLNO];
	char *p;
	
	p = pbar;
	/* up stairway */
	if (upstair.sx &&
	   (glyph_to_cmap(level.locations[upstair.sx][upstair.sy].glyph) ==
	    S_upstair ||
 	    glyph_to_cmap(level.locations[upstair.sx][upstair.sy].glyph) ==
	    S_upladder)) {
		*p++ = '<';
		*p++ = upstair.sx;
	}
	if (sstairs.sx &&
	   (glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
	    S_upstair ||
 	    glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
	    S_upladder)) {
		*p++ = '<';
		*p++ = sstairs.sx;
	}

	/* down stairway */
	if (dnstair.sx &&
	   (glyph_to_cmap(level.locations[dnstair.sx][dnstair.sy].glyph) ==
	    S_dnstair ||
 	    glyph_to_cmap(level.locations[dnstair.sx][dnstair.sy].glyph) ==
	    S_dnladder)) {
		*p++ = '>';
		*p++ = dnstair.sx;
	}
	if (sstairs.sx &&
	   (glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
	    S_dnstair ||
 	    glyph_to_cmap(level.locations[sstairs.sx][sstairs.sy].glyph) ==
	    S_dnladder)) {
		*p++ = '>';
		*p++ = sstairs.sx;
	}

	/* hero location */
	if (u.ux) {
		*p++ = '@';
		*p++ = u.ux;
	}
	/* fence post */
	*p = 0;

	update_positionbar(pbar);
}
#endif

#if defined(REALTIME_ON_BOTL) || defined (RECORD_REALTIME)
time_t
get_realtime(void)
{
    time_t curtime;

    /* Get current time */
#if defined(BSD) && !defined(POSIX_TYPES)
    (void) time((long *)&curtime);
#else
    (void) time(&curtime);
#endif

    /* Since the timer isn't set until the game starts, this prevents us
     * from displaying nonsense on the bottom line before it does. */
    if(realtime_data.restoretime == 0) {
        curtime = realtime_data.realtime;
    } else {
        curtime -= realtime_data.restoretime;
        curtime += realtime_data.realtime;
    }
 
    return curtime;
}
#endif /* REALTIME_ON_BOTL || RECORD_REALTIME */

STATIC_DCL
void
printDPR(){
	FILE *rfile;
	register int i, j, avdm, mdm;
	char pbuf[BUFSZ];
	struct permonst *ptr;
	struct attack *attk;
	rfile = fopen_datafile("MonDPR.tab", "w", SCOREPREFIX);
	if (rfile) {
		Sprintf(pbuf,"Number\tName\tclass\taverage\tmax\tspeed\talignment\tunique?\n");
		fprintf(rfile, pbuf);
		for(j=0;j<NUMMONS;j++){
			ptr = &mons[j];
			pbuf[0] = 0;
			avdm = 0;
			mdm = 0;
			for(i = 0; i<6; i++){
				attk = &ptr->mattk[i];
				if(attk->aatyp == 0 &&
					attk->adtyp == 0 &&
					attk->damn == 0 &&
					attk->damd == 0
				) break;
				else {
					avdm += attk->damn * (attk->damd + 1)/2;
					mdm += attk->damn * attk->damd;
				}
			}
			Sprintf(pbuf,"%d\t%s\t%d\t%d\t%d\t%d\t%d\t%s\n", j, mons[j].mname, mons[j].mlet,avdm, mdm, mons[j].mmove,ptr->maligntyp,(mons[j].geno&G_UNIQ) ? "unique":"");
			fprintf(rfile, pbuf);
		}
	}
}

STATIC_DCL
void
printMons(){
	FILE *rfile;
	register int i;
	char pbuf[BUFSZ*100];
	char attkbuf[BUFSZ];
	char sizebuf[BUFSZ];
	char resbuf[BUFSZ];
	char convbuf[BUFSZ];
	struct permonst *ptr;
	attkbuf[0]=0;
	sizebuf[0]=0;
	resbuf[0]=0;
	convbuf[0]=0;
	rfile = fopen_datafile("wikiMonsters.txt", "w", SCOREPREFIX);
	if (rfile) {
		char *colorstr;
		for(i=0;i<NUMMONS;i++){
			ptr = &mons[i];
			switch(mons[i].mcolor){
				case CLR_BLACK:
					colorstr = "black";
				break;
				case CLR_RED:
					colorstr = "red";
				break;
				case CLR_GREEN:
					colorstr = "green";
				break;
				case CLR_BROWN:
					colorstr = "brown";
				break;
				case CLR_BLUE:
					colorstr = "blue";
				break;
				case CLR_MAGENTA:
					colorstr = "magenta";
				break;
				case CLR_CYAN:
					colorstr = "cyan";
				break;
				case CLR_GRAY:
					colorstr = "gray";
				break;
				case CLR_ORANGE:
					colorstr = "orange";
				break;
				case CLR_BRIGHT_MAGENTA:
					colorstr = "brightmagenta";
				break;
				case CLR_BRIGHT_GREEN:
					colorstr = "brightgreen";
				break;
				case CLR_YELLOW:
					colorstr = "yellow";
				break;
				case CLR_BRIGHT_BLUE:
					colorstr = "brightblue";
				break;
				case CLR_BRIGHT_CYAN:
					colorstr = "brightcyan";
				break;
				case CLR_WHITE:
					colorstr = "white";
				break;
			}
			printAttacks(attkbuf,&mons[i]);
			switch(mons[i].msize){
				case MZ_TINY:
					Sprintf(sizebuf,"tiny");
				break;
				case MZ_SMALL:
					Sprintf(sizebuf,"small");
				break;
				case MZ_MEDIUM:
					Sprintf(sizebuf,"medium");
				break;
				case MZ_LARGE:
					Sprintf(sizebuf,"large");
				break;
				case MZ_HUGE:
					Sprintf(sizebuf,"huge");
				break;
				default:
					Sprintf(sizebuf,"gigantic");
				break;
			}
			resFlags(resbuf, mons[i].mresists);
			resFlags(convbuf, mons[i].mconveys);
			Sprintf(pbuf,"{{monster\n");
			Sprintf(eos(pbuf)," |name=%s\n", mons[i].mname);
			Sprintf(eos(pbuf)," |symbol={{%s|%c}}\n", colorstr, def_monsyms[(int)mons[i].mlet]);
			Sprintf(eos(pbuf)," |tile=\n");
			Sprintf(eos(pbuf)," |difficulty=%d\n", monstr[i]);
			Sprintf(eos(pbuf)," |level=%d\n", mons[i].mlevel);
			Sprintf(eos(pbuf)," |experience=%d\n", ptrexperience(ptr));
			Sprintf(eos(pbuf)," |speed=%d\n", mons[i].mmove);
			Sprintf(eos(pbuf)," |AC=%d\n", mons[i].ac);
			Sprintf(eos(pbuf)," |MR=%d\n", mons[i].mr);
			Sprintf(eos(pbuf)," |align=%d\n", mons[i].maligntyp);
			Sprintf(eos(pbuf)," |frequency=%d\n", (int)(mons[i].geno & G_FREQ));
			Sprintf(eos(pbuf)," |genocidable=%s\n", ((mons[i].geno & G_GENO) != 0) ? "Yes":"No");
			Sprintf(eos(pbuf)," |attacks=%s\n", attkbuf);
			Sprintf(eos(pbuf)," |weight=%d\n", (int) mons[i].cwt);
			Sprintf(eos(pbuf)," |nutr=%d\n", (int) mons[i].cnutrit);
			Sprintf(eos(pbuf)," |size=%s\n", sizebuf);
			Sprintf(eos(pbuf)," |resistances=%s\n", resbuf);
			Sprintf(eos(pbuf)," |resistances conveyed=%s\n", convbuf);
			Sprintf(eos(pbuf)," |attributes={{attributes\n");
				Sprintf(eos(pbuf)," |%s|=\n", Aptrnam(ptr));
			if(species_flies(&mons[i]))	Sprintf(eos(pbuf)," |fly=%s\n", species_flies(&mons[i]) ? "1":"");
			if(species_swims(&mons[i]))	Sprintf(eos(pbuf)," |swim=%s\n", species_swims(&mons[i]) ? "1":"");
			if(amorphous(ptr))	Sprintf(eos(pbuf)," |amorphous=%s\n", amorphous(ptr) ? "1":"");
			if(species_passes_walls(ptr))	Sprintf(eos(pbuf)," |wallwalk=%s\n", species_passes_walls(ptr) ? "1":"");
			if(is_clinger(ptr))	Sprintf(eos(pbuf)," |cling=%s\n", is_clinger(ptr) ? "1":"");
			if(tunnels(ptr))	Sprintf(eos(pbuf)," |tunnel=%s\n", tunnels(ptr) ? "1":"");
			if(needspick(ptr))	Sprintf(eos(pbuf)," |needpick=%s\n", needspick(ptr) ? "1":"");
			if(hides_under(ptr))	Sprintf(eos(pbuf)," |conceal=%s\n", hides_under(ptr) ? "1":"");
			if(is_hider(ptr))	Sprintf(eos(pbuf)," |hide=%s\n",  is_hider(ptr) ? "1":"");
			if( amphibious(ptr) )	Sprintf(eos(pbuf)," |amphibious=%s\n",  amphibious(ptr) ? "1":"");
			if(breathless(ptr))	Sprintf(eos(pbuf)," |breathless=%s\n",  breathless(ptr) ? "1":"");
			if(notake(ptr) )	Sprintf(eos(pbuf)," |notake=%s\n", notake(ptr) ? "1":"");
			if(!haseyes(ptr))	Sprintf(eos(pbuf)," |noeyes=%s\n", !haseyes(ptr) ? "1":"");
			if(nohands(ptr))	Sprintf(eos(pbuf)," |nohands=%s\n", nohands(ptr) ? "1":"");
			if(nolimbs(ptr))	Sprintf(eos(pbuf)," |nolimbs=%s\n", nolimbs(ptr) ? "1":"");
			if(!has_head(ptr))	Sprintf(eos(pbuf)," |nohead=%s\n", !has_head(ptr) ? "1":"");
			if(mindless(ptr))	Sprintf(eos(pbuf)," |mindless=%s\n", mindless(ptr) ? "1":"");
			if( humanoid(ptr))	Sprintf(eos(pbuf)," |humanoid=%s\n", humanoid(ptr) ? "1":"");
			if(is_animal(ptr))	Sprintf(eos(pbuf)," |animal=%s\n", is_animal(ptr) ? "1":"");
			if(slithy(ptr))	Sprintf(eos(pbuf)," |slithy=%s\n", slithy(ptr) ? "1":"");
			if(unsolid(ptr))	Sprintf(eos(pbuf)," |unsolid=%s\n", unsolid(ptr) ? "1":"");
			if(thick_skinned(ptr))	Sprintf(eos(pbuf)," |thick=%s\n", thick_skinned(ptr) ? "1":"");
			if(lays_eggs(ptr))	Sprintf(eos(pbuf)," |oviparous=%s\n", lays_eggs(ptr) ? "1":"");
			if(species_regenerates(ptr))	Sprintf(eos(pbuf)," |regen=%s\n", species_regenerates(ptr) ? "1":"");
			if(species_perceives(ptr))	Sprintf(eos(pbuf)," |seeinvis=%s\n", species_perceives(ptr) ? "1":"");
			if(species_teleports(ptr))	Sprintf(eos(pbuf)," |tport=%s\n", species_teleports(ptr) ? "1":"");
			if(species_controls_teleports(ptr))	Sprintf(eos(pbuf)," |tportcntrl=%s\n", species_controls_teleports(ptr) ? "1":"");
			if(acidic(ptr))	Sprintf(eos(pbuf)," |acid=%s\n", acidic(ptr) ? "1":"");
			if(poisonous(ptr) )	Sprintf(eos(pbuf)," |pois=%s\n", poisonous(ptr) ? "1":"");
			if(carnivorous(ptr)&&!herbivorous(ptr))	Sprintf(eos(pbuf)," |carnivore=%s\n", carnivorous(ptr)&&!herbivorous(ptr) ? "1":"");
			if(herbivorous(ptr)&&!carnivorous(ptr))	Sprintf(eos(pbuf)," |herbivore=%s\n", herbivorous(ptr)&&!carnivorous(ptr) ? "1":"");
			if(carnivorous(ptr)&&herbivorous(ptr))	Sprintf(eos(pbuf)," |omnivore=%s\n", carnivorous(ptr)&&herbivorous(ptr) ? "1":"");
			if(metallivorous(ptr))	Sprintf(eos(pbuf)," |metallivore=%s\n", metallivorous(ptr) ? "1":"");
			if(!polyok(ptr))	Sprintf(eos(pbuf)," |nopoly=%s\n", !polyok(ptr) ? "1":"");
			if(is_undead(ptr))	Sprintf(eos(pbuf)," |undead=%s\n", is_undead(ptr) ? "1":"");
			if(is_were(ptr))	Sprintf(eos(pbuf)," |were=%s\n", is_were(ptr) ? "1":"");
			if(is_human(ptr))	Sprintf(eos(pbuf)," |human=%s\n", is_human(ptr) ? "1":"");
			if(is_elf(ptr))	Sprintf(eos(pbuf)," |elf=%s\n", is_elf(ptr) ? "1":"");
			if(is_dwarf(ptr))	Sprintf(eos(pbuf)," |dwarf=%s\n", is_dwarf(ptr) ? "1":"");
			if(is_gnome(ptr))	Sprintf(eos(pbuf)," |gnome=%s\n", is_gnome(ptr) ? "1":"");
			if(is_orc(ptr))	Sprintf(eos(pbuf)," |orc=%s\n", is_orc(ptr) ? "1":"");
			if(is_demon(ptr))	Sprintf(eos(pbuf)," |demon=%s\n", is_demon(ptr) ? "1":"");
			if(is_mercenary(ptr))	Sprintf(eos(pbuf)," |merc=%s\n", is_mercenary(ptr) ? "1":"");
			if(is_lord(ptr))	Sprintf(eos(pbuf)," |lord=%s\n", is_lord(ptr) ? "1":"");
			if(is_prince(ptr))	Sprintf(eos(pbuf)," |prince=%s\n", is_prince(ptr) ? "1":"");
			if(is_giant(ptr) )	Sprintf(eos(pbuf)," |giant=%s\n", is_giant(ptr) ? "1":"");
			if(is_male(ptr))	Sprintf(eos(pbuf)," |male=%s\n", is_male(ptr) ? "1":"");
			if(is_female(ptr))	Sprintf(eos(pbuf)," |female=%s\n", is_female(ptr) ? "1":"");
			if(is_neuter(ptr))	Sprintf(eos(pbuf)," |neuter=%s\n", is_neuter(ptr) ? "1":"");
			if(always_hostile(ptr))	Sprintf(eos(pbuf)," |hostile=%s\n", always_hostile(ptr) ? "1":"");
			if(always_peaceful(ptr))	Sprintf(eos(pbuf)," |peaceful=%s\n", always_peaceful(ptr) ? "1":"");
			if(is_domestic(ptr))	Sprintf(eos(pbuf)," |domestic=%s\n", is_domestic(ptr) ? "1":"");
			if(is_wanderer(ptr))	Sprintf(eos(pbuf)," |wander=%s\n", is_wanderer(ptr) ? "1":"");
			if((((ptr)->mflagst & MT_STALK) != 0L))	Sprintf(eos(pbuf)," |stalk=%s\n", (((ptr)->mflagst & MT_STALK) != 0L) ? "1":"");
			if(extra_nasty(ptr))	Sprintf(eos(pbuf)," |nasty=%s\n", extra_nasty(ptr) ? "1":"");
			if(strongmonst(ptr) )	Sprintf(eos(pbuf)," |strong=%s\n", strongmonst(ptr) ? "1":"");
			if(throws_rocks(ptr))	Sprintf(eos(pbuf)," |rockthrow=%s\n", throws_rocks(ptr) ? "1":"");
			if(likes_gold(ptr))	Sprintf(eos(pbuf)," |greedy=%s\n", likes_gold(ptr) ? "1":"");
			if(likes_gems(ptr))	Sprintf(eos(pbuf)," |jewels=%s\n", likes_gems(ptr) ? "1":"");
			if(likes_objs(ptr))	Sprintf(eos(pbuf)," |collect=%s\n", likes_objs(ptr) ? "1":"");
			if(likes_magic(ptr))	Sprintf(eos(pbuf)," |magic=%s\n", likes_magic(ptr) ? "1":"");
			if(wants_amul(ptr))	Sprintf(eos(pbuf)," |wantsamul=%s\n", wants_amul(ptr) ? "1":"");
			if(wants_bell(ptr))	Sprintf(eos(pbuf)," |wantsbell=%s\n", wants_bell(ptr) ? "1":"");
			if(wants_book(ptr))	Sprintf(eos(pbuf)," |wantsbook=%s\n", wants_book(ptr) ? "1":"");
			if(wants_cand(ptr))	Sprintf(eos(pbuf)," |wantscand=%s\n", wants_cand(ptr) ? "1":"");
			if(wants_qart(ptr))	Sprintf(eos(pbuf)," |wantsarti=%s\n", wants_qart(ptr) ? "1":"");
			if(is_covetous(ptr))	Sprintf(eos(pbuf)," |wantsall=%s\n", is_covetous(ptr) ? "1":"");
			if((((ptr)->mflagst & MT_WAITFORU) != 0L))	Sprintf(eos(pbuf)," |waitsforu=%s\n", (((ptr)->mflagst & MT_WAITFORU) != 0L) ? "1":"");
			if((((ptr)->mflagst & MT_CLOSE) != 0L))	Sprintf(eos(pbuf)," |close=%s\n", (((ptr)->mflagst & MT_CLOSE) != 0L) ? "1":"");
			if(is_covetous(ptr))	Sprintf(eos(pbuf)," |covetous=%s\n", is_covetous(ptr) ? "1":"");
			if(infravision(ptr))	Sprintf(eos(pbuf)," |infravision=%s\n", infravision(ptr) ? "1":"");
			if(infravisible(ptr))	Sprintf(eos(pbuf)," |infravisible=%s\n", infravisible(ptr) ? "1":"");
			if((mons[i].geno & G_NOHELL))	Sprintf(eos(pbuf)," |nohell=%s\n", (mons[i].geno & G_NOHELL) ? "1":"");
			if(((mons[i].geno & G_HELL) != 0))	Sprintf(eos(pbuf)," |hell=%s\n", ((mons[i].geno & G_HELL) != 0) ? "1":"");
			// if(((mons[i].geno & G_PLANES) != 0))	Sprintf(eos(pbuf)," |hell=%s\n", ((mons[i].geno & G_PLANES) != 0) ? "1":"");
			// if()	Sprintf(eos(pbuf)," |moria=\n");
			if((mons[i].geno & G_SGROUP))	Sprintf(eos(pbuf)," |sgroup=%s\n", (mons[i].geno & G_SGROUP) ? "1":"");
			if((mons[i].geno & G_LGROUP))	Sprintf(eos(pbuf)," |lgroup=%s\n", (mons[i].geno & G_LGROUP) ? "1":"");
			// if()	Sprintf(eos(pbuf)," |vlgroup=\n");
			if((mons[i].geno & G_NOCORPSE))	Sprintf(eos(pbuf)," |nocorpse=%s\n", (mons[i].geno & G_NOCORPSE) ? "1":"");
			if((is_undead(ptr) && !(mons[i].geno & G_NOCORPSE)))	Sprintf(eos(pbuf)," |oldcorpse=%s\n", (is_undead(ptr) && !(mons[i].geno & G_NOCORPSE)) ? "1":"");
			if(emits_light(ptr))	Sprintf(eos(pbuf)," |light=%s\n", emits_light(ptr) ? "1":"");
			if((nonliving(ptr) || is_demon(ptr)))	Sprintf(eos(pbuf)," |death=%s\n", (nonliving(ptr) || is_demon(ptr)) ? "1":"");
			if((mons[i].mresists & MR_DRAIN))	Sprintf(eos(pbuf)," |drain=%s\n", (mons[i].mresists & MR_DRAIN) ? "1":"");
			// if()	Sprintf(eos(pbuf)," |plusone=\n");
			// if()	Sprintf(eos(pbuf)," |plustwo=\n");
			// if()	Sprintf(eos(pbuf)," |plusthree=\n");
			// if()	Sprintf(eos(pbuf)," |plusfour=\n");
			// if()	Sprintf(eos(pbuf)," |hitasone=\n");
			// if()	Sprintf(eos(pbuf)," |hitastwo=\n");
			// if()	Sprintf(eos(pbuf)," |hitasthree=\n");
			// if()	Sprintf(eos(pbuf)," |hitasfour=\n");
			if(is_vampire(ptr))	Sprintf(eos(pbuf)," |vampire=%s\n", is_vampire(ptr) ? "1":"");
			if(can_betray(ptr))	Sprintf(eos(pbuf)," |traitor=%s\n", can_betray(ptr) ? "1":"");
			if((!(is_covetous(ptr) || is_human(ptr))))	Sprintf(eos(pbuf)," |notame=%s\n", (!(is_covetous(ptr) || is_human(ptr))) ? "1":"");
			if((emits_light(ptr) == 1))	Sprintf(eos(pbuf)," |light1=%s\n", (emits_light(ptr) == 1) ? "1":"");
			if((emits_light(ptr) == 2))	Sprintf(eos(pbuf)," |light2=%s\n", (emits_light(ptr) == 2) ? "1":"");
			if((emits_light(ptr) == 3))	Sprintf(eos(pbuf)," |light3=%s\n", (emits_light(ptr) == 3) ? "1":"");
			// if()	Sprintf(eos(pbuf)," |groupattack=\n");
			// if()	Sprintf(eos(pbuf)," |blinker=\n");
			if((!species_regenerates(ptr) && nonliving(ptr)))	Sprintf(eos(pbuf)," |noregen=%s\n", (!species_regenerates(ptr) && nonliving(ptr)) ? "1":"");
			if(stationary(ptr))	Sprintf(eos(pbuf)," |stationary=%s\n", stationary(ptr) ? "1":"");
			Sprintf(eos(pbuf)," }}\n");
			Sprintf(eos(pbuf)," |refline=\n");
			Sprintf(eos(pbuf),"}}\n\n");
			fprintf(rfile, pbuf);
		}
		for(i=0;i<NUMMONS;i++){
			switch(mons[i].mcolor){
				case CLR_BLACK:
					colorstr = "black";
				break;
				case CLR_RED:
					colorstr = "red";
				break;
				case CLR_GREEN:
					colorstr = "green";
				break;
				case CLR_BROWN:
					colorstr = "brown";
				break;
				case CLR_BLUE:
					colorstr = "blue";
				break;
				case CLR_MAGENTA:
					colorstr = "magenta";
				break;
				case CLR_CYAN:
					colorstr = "cyan";
				break;
				case CLR_GRAY:
					colorstr = "gray";
				break;
				case CLR_ORANGE:
					colorstr = "orange";
				break;
				case CLR_BRIGHT_MAGENTA:
					colorstr = "brightmagenta";
				break;
				case CLR_BRIGHT_GREEN:
					colorstr = "brightgreen";
				break;
				case CLR_YELLOW:
					colorstr = "yellow";
				break;
				case CLR_BRIGHT_BLUE:
					colorstr = "brightblue";
				break;
				case CLR_BRIGHT_CYAN:
					colorstr = "brightcyan";
				break;
				case CLR_WHITE:
					colorstr = "white";
				break;
			}
			if((int)mons[i].mlet == S_SHADE) Sprintf(pbuf,"[[%s|{{%s|&nbsp;}}]]", mons[i].mname, colorstr);
			else Sprintf(pbuf,"[[%s|{{%s|%c}}]]", mons[i].mname, colorstr, def_monsyms[(int)mons[i].mlet]);
			fprintf(rfile, pbuf);
			if(((i+1)%40) == 0){
				Sprintf(pbuf,"<br/>\n", mons[i].mname, colorstr, def_monsyms[(int)mons[i].mlet]);
				fprintf(rfile, pbuf);
			}
		}
		while(((i+1)%40) != 0){
			Sprintf(pbuf,"{{black|&nbsp;}}");
			fprintf(rfile, pbuf);
			i++;
		}
	}
}

STATIC_DCL
void
resFlags(buf, rflags)
	char *buf;
	unsigned int rflags;
{
	int i;
	boolean b = FALSE;
	static char *mrKey[] = {
		"[[fire]]",
		"[[cold]]",
		"[[sleep]]",
		"[[disintegration]]",
		"[[shock]]",
		"[[poison]]",
		"[[acid]]",
		"[[petrification]]",
		"[[level drain]]",
		"[[disease]]"
	};
	// Sprintf(buf,""); //What was this for?
	for(i = 0; i<10; i++){
		if(rflags & (1 << i)){
			if(!b){
				b = TRUE;
				Sprintf(buf, "%s", mrKey[i]);
			} else {
				Sprintf(eos(buf), ", %s", mrKey[i]);
			}
		}
	}
}

STATIC_DCL
void
printAttacks(buf, ptr)
	char *buf;
	struct permonst *ptr;
{
	int i;
	struct attack *attk;
	static char *attackKey[] = {
		"Passive",	/*0*/
		"Claw",		/*1*/
		"Bite",		/*2*/
		"Kick",		/*3*/
		"Butt",		/*4*/
		"Touch",	/*5*/
		"Sting",	/*6*/
		"Bearhug",	/*7*/
		"NA",		/*8*/
		"NA",		/*9*/
		"Spit",		/*10*/
		"Engulf",	/*11*/
		"Breath weapon",	/*12*/
		"Suicidal explosion",	/*13*/
		"Explode on death",	/*14*/
		"Single-target (active) gaze attack",	/*15*/
		"Tentacle",	/*16*/
		"Arrow",	/*17*/
		"Whip",	/*18*/
		"Reach",	/*19*/
		"Your weapon",	/*20*/
		"Reach",	/*21*/
		"Magic",	/*22*/
		"Engulf",	/*23*/
		"Automatic hit",	/*24*/
		"Mist tendrils",	/*25*/
		"Tinker",	/*26*/
		"Shadow blades",	/*27*/
		"Beam",	/*28*/
		"Deva Arms",	/*29*/
		"Five-square-reach touch",	/*30*/
		"Wide-angle (passive) gaze"	/*31*/
		"Rend"	/*32*/
	};
	static char *damageKey[] = {
		"physical",				/*0*/
		"[[magic missile]]s",	/*1*/
		"[[fire]]",				/*2*/
		"[[cold]]",				/*3*/
		"[[sleep]]",			/*4*/
		"[[disintegration]]",	/*5*/
		"[[shock]]",			/*6*/
		"[[poison]] (strength)",/*7*/
		"[[acid]]",				/*8*/
		"Unused 1",				/*9*/
		"Unused 2",				/*10*/
		"[[blind]]ing",			/*11*/
		"[[stun]]ning",			/*12*/
		"[[slow]]ing",			/*13*/
		"[[paralysis]]",		/*14*/
		"[[life drain]]",		/*15*/
		"[[energy drain]]",		/*16*/
		"[[leg wounding]]",		/*17*/
		"[[petrifcation]]",		/*18*/
		"[[sticky]]",			/*19*/
		"[[steal gold]]",		/*20*/
		"[[steal item]]",		/*21*/
		"[[steal item|seduction]]",/*22*/
		"[[teleportation]]",	/*23*/
		"[[rust]]",				/*24*/
		"[[confusion]]",		/*25*/
		"[[digestion]]",		/*26*/
		"[[healing]]",			/*27*/
		"[[drowning]]",			/*28*/
		"[[lycanthropy]]",		/*29*/
		"[[poison]] (dexterity)",/*30*/
		"[[poison]] (constitution)",/*31*/
		"[[int drain]]",		/*32*/
		"[[disease]]",			/*33*/
		"[[rotting]]",			/*34*/
		"[[seduction]]",		/*35*/
		"[[hallucination]]",	/*36*/
		"[[Death's touch]]",	/*37*/
		"[[Pestilence]]",		/*38*/
		"[[Famine]]",			/*39*/
		"[[sliming]]",			/*40*/
		"[[disenchant]]",		/*41*/
		"[[corrosion]]",		/*42*/
		"[[poison]] (HP damage)",/*43*/
		"[[wis drain]]",		/*44*/
		"[[vorpal]]",			/*45*/
		"[[armor shredding]]",	/*46*/
		"[[silver]]",			/*47*/
		"[[cannon ball]]",		/*48*/
		"[[boulder]]",			/*49*/
		"[[random boulder]]",	/*50*/
		"[[tickling]]",			/*51*/
		"[[soaking]]",			/*52*/
		"[[lethe]]",			/*53*/
		"[[bisection]]",		/*54*/
		"NA",					/*55*/
		"NA",					/*56*/
		"NA",					/*57*/
		"NA",					/*58*/
		"NA",					/*59*/
		"[[cancellation]]",		/*60*/
		"[[deadly]]",			/*61*/
		"[[suction]]",			/*62*/
		"[[malkuth]]",			/*63*/
		"[[uvuudaum brainspike]]",/*64*/
		"[[abduction]]",		/*65*/
		"[[spawn Chaos]]",		/*66*/
		"[[seduction]]",		/*67*/
		"[[hellblood]]",		/*68*/
		"[[spawn Leviathan]]",	/*69*/
		"[[mist projection]]",	/*70*/
		"[[teleport away]]",	/*71*/
		"[[baleful polymorph]]",/*72*/
		"[[psionic]]",			/*73*/
		"[[promotion]]",		/*74*/
		"[[shared soul]]",		/*75*/
		"[[intrusion]]",		/*76*/
		"[[jailer]]",			/*77*/
		"[[special]]",			/*78*/
		"[[take artifact]]",	/*79*/
		"[[silver]]",			/*80*/
		"[[special]]",			/*81*/
		"[[your weapon]]",		/*82*/
		"[[cursed unicorn horn]]",/*83*/
		"[[loadstone]]",		/*84*/
		"[[garo report]]",		/*85*/
		"[[garo report]]",		/*86*/
		"[[level teleport]]",	/*87*/
		"[[blink]]",			/*88*/
		"[[angel's touch]]",	/*89*/
		"[[spore]]",			/*90*/
		"[[explosive spore]]",	/*91*/
		"[[sunlight]]",			/*92*/
		"[[deadly shriek]]",	/*93*/
		"[[barbs]]",			/*94*/
		"[[luck drain]]",		/*95*/
		"[[vampiric]]",			/*96*/
		"[[webbing]]",			/*97*/
		"[[special]]",			/*98*/
		"[[spawn gizmos]]",		/*99*/
		"[[fireworks]]",		/*100*/
		"[[study]]",			/*101*/
		"[[fire]], [[cold]], or [[shock]]",/*102*/
		"[[netzach]]",			/*103*/
		"[[special]]",			/*104*/
		"[[shadow]]",			/*105*/
		"[[armor teleportation]]",/*106*/
		"[[half-dragon breath]]",/*107*/
		"[[silver rapier]]",	/*108*/
		"elemental [[shock]]",	/*109*/
		"elemental [[fire]]",	/*110*/
		"elemental [[poison]]",	/*111*/
		"elemental [[cold]]",	/*112*/
		"elemental [[acid]]",	/*113*/
		"conflict",				/*114*/
		"blood blade",			/*115*/
		"Surya Deva arrow",		/*116*/
		"[[constitution]] drain",/*117*/
		"Silver mirror shards", /*118*/
		"Mercury blade",		/*119*/
		"Gold transmutation", 	/*120*/
		"Holy fire", 			/*121*/
		// "[[ahazu abduction]]",	/**/
		"[[stone choir]]",		/*122*/
		"[[water vampire]]",	/*123*/
		"[[bloody fangs]]",		/*124*/
		"[[item freeing]]",		/*125*/
		"[[rainbow feathers]]",	/*126*/
		"[[Vorlon explosion]]",	/*127*/
		"[[cold explosion]]",	/*128*/
		"[[fire explosion]]",	/*129*/
		"[[shock explosion]]",	/*130*/
		"[[physical explosion]]",/*131*/
		"[[Vorlon missile]]",	/*132*/
		"[[Warmachine missile]]",/*133*/
		"[[clerical spell]]",	/*134*/
		"[[mage spell]]",		/*135*/
		"[[random breath type]]",/*136*/
		"[[random gaze type]]",	/*137*/
		"[[random elemental gaze]]",/*138*/
		"[[Amulet theft]]",		/*139*/
		"[[Intrinsic theft]]",	/*140*/
		"[[Quest Artifact theft]]"/*141*/
	};
	for(i = 0; i<6; i++){
		attk = &ptr->mattk[i];
		if(attk->aatyp == 0 &&
			attk->adtyp == 0 &&
			attk->damn == 0 &&
			attk->damd == 0
		) return;
		if(!i){
			Sprintf(buf, "%s %dd%d %s",
				attk->aatyp == AT_WEAP ? "Weapon" :
				attk->aatyp == AT_XWEP ? "Offhand Weapon" :
				attk->aatyp == AT_MARI ? "Multiarm Weapon" :
				attk->aatyp == AT_MAGC ? "Cast" :
				attackKey[((int)attk->aatyp)],
				attk->damn,
				attk->damd,
				damageKey[((int)attk->adtyp)]
			);
		} else {
			Sprintf(eos(buf), ", %s %dd%d %s",
				attk->aatyp == AT_WEAP ? "Weapon" :
				attk->aatyp == AT_XWEP ? "Offhand Weapon" :
				attk->aatyp == AT_MARI ? "Multiarm Weapon" :
				attk->aatyp == AT_MAGC ? "Cast" :
				attackKey[((int)attk->aatyp)],
				attk->damn,
				attk->damd,
				damageKey[((int)attk->adtyp)]
			);
		}
	}
	return;
}

#endif /* OVLB */

/*allmain.c*/
