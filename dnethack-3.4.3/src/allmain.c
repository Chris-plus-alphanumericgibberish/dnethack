/*	SCCS Id: @(#)allmain.c	3.4	2003/04/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* various code that was replicated in *main.c */

#include "hack.h"
#include "edog.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

#ifdef POSITIONBAR
STATIC_DCL void NDECL(do_positionbar);
#endif

STATIC_DCL void NDECL(printMons);
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

void
moveloop()
{
#if defined(MICRO) || defined(WIN32)
	char ch;
	int abort_lev;
#endif
    struct monst *mtmp, *nxtmon;
	struct obj *pobj;
    int moveamt = 0, wtcap = 0, change = 0;
    boolean didmove = FALSE, monscanmove = FALSE;
	int oldspiritAC=0;
	int tx,ty;
	int nmonsclose,nmonsnear,enkispeedlim;
	static boolean oldBlind = 0, oldLightBlind = 0;
	static int oldCon, oldWisBon;
    int hpDiff;

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

	oldCon = ACURR(A_CON);
	oldWisBon = ACURR(A_WIS)/4;
	// printMons();
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
			} else if(uwep && uwep->oartifact == ART_TOBIUME){
				youmonst.movement -= 4;
			} else {
				youmonst.movement -= NORMAL_SPEED;
			}
		} else {
			youmonst.movement -= NORMAL_SPEED;
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
	/*If anything a monster did caused us to get moved out of water, surface*/
	if(u.usubwater && !is_pool(u.ux, u.uy)){
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
			if (!oldCon != ACURR(A_CON)) {
				int condif = conplus(ACURR(A_CON)) - conplus(oldCon);
				if(condif != 0) u.uhpmax += u.ulevel*condif;
				if(u.uhpmax < 1) u.uhpmax = 1;
				if(u.uhp > u.uhpmax) u.uhp = u.uhpmax;
				oldCon = ACURR(A_CON);
			}
////////////////////////////////////////////////////////////////////////////////////////////////
			if (!oldWisBon != ACURR(A_WIS)/4) {
				u.uenmax += u.ulevel*(ACURR(A_WIS)/4 - oldWisBon);
				if(u.uenmax < 0) u.uenmax = 0;
				if(u.uen > u.uenmax) u.uen = u.uenmax;
				oldWisBon = ACURR(A_WIS)/4;
			}
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
			if(u.sealsActive&SEAL_ORTHOS && !(u.sealsActive&SEAL_AMON || Race_if(PM_DROW))
				&&!(viz_array[u.uy][u.ux]&TEMP_LIT || levl[u.ux][u.uy].lit)
			){
				if(!u.nv_range){
					if(++u.orthocounts>5) unbind(SEAL_ORTHOS,TRUE);
				} else {
					if(++u.orthocounts>5*u.nv_range) unbind(SEAL_ORTHOS,TRUE);
				}
				if(u.sealsActive&SEAL_ORTHOS && Hallucination){ /*Didn't just unbind it*/
					if(u.orthocounts == 1) pline("It is now pitch black. You are likely to be eaten by a grue.");
					else pline("You are likely to be eaten by a grue.");
				} else You_feel("increasingly panicked about being in the dark!");
			}
			if(u.sealsActive&SEAL_NABERIUS && u.udrunken<u.ulevel) unbind(SEAL_NABERIUS,TRUE);
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
				if(!touch_artifact(uwep,&youmonst)){
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
		    if (youmonst.movement > NORMAL_SPEED)
			break;	/* it's now your turn */
		} while (monscanmove);
		flags.mon_moving = FALSE;

		if (!monscanmove && youmonst.movement < NORMAL_SPEED) {
		    /* both you and the monsters are out of steam this round */
		    /* set up for a new turn */
		    mcalcdistress();	/* adjust monsters' trap, blind, etc */

		    /* reallocate movement rations to monsters */
			flags.spore_level=0;
			flags.slime_level=0;
			flags.walky_level=0;
			flags.shade_level=0;
		    for (mtmp = fmon; mtmp; mtmp = nxtmon){
				nxtmon = mtmp->nmon;
				/* Possibly vanish */
				if(mtmp->mvanishes>-1){
					if(mtmp->mvanishes-- == 0){
						monvanished(mtmp);
						continue;
					}
				}
				/* Possibly become hostile */
				if(mtmp->mpeacetime && !mtmp->mtame){
					mtmp->mpeacetime--;
					if(!mtmp->mpeacetime) mtmp->mpeaceful = FALSE;
				}
				if(mtmp->data == &mons[PM_ZUGGTMOY]) flags.spore_level=1;
				if(mtmp->data == &mons[PM_JUIBLEX]) flags.slime_level=1;
				if(mtmp->data == &mons[PM_PALE_NIGHT] || mtmp->data == &mons[PM_DREAD_SERAPH] || mtmp->data == &mons[PM_LEGION]) flags.walky_level=1;
				if(mtmp->data == &mons[PM_ORCUS] || mtmp->data == &mons[PM_NAZGUL]) flags.shade_level=1;
				if(mtmp->data == &mons[PM_DREAD_SERAPH] && (mtmp->mstrategy & STRAT_WAITMASK) && (u.uevent.udemigod || (Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)))){
					mtmp->mstrategy &= ~STRAT_WAITMASK;
					pline_The("entire %s is shaking around you!",
						   In_endgame(&u.uz) ? "plane" : "dungeon");
					do_earthquake(min(((int)mtmp->m_lev - 1) / 6 + 1,12), TRUE, mtmp);
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
				mtmp->movement += mcalcmove(mtmp);
				if(mtmp->moccupation && !occupation){
					mtmp->moccupation = 0;
					mtmp->mcanmove = 1;
				}
				if(!mtmp->mnotlaugh){
					if(!is_silent(mtmp->data)){
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
					if(mtmp->mux - mtmp->mx < 0) mdx = -1;
					else if(mtmp->mux - mtmp->mx > 0) mdx = +1;
					if(mtmp->muy - mtmp->my < 0) mdy = -1;
					else if(mtmp->muy - mtmp->my > 0) mdy = +1;
					for(i=0;i<8;i++) if(xdir[i] == mdx && ydir[i] == mdy) break;
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
			
		    if(!(Is_illregrd(&u.uz) && u.ualign.type == A_LAWFUL && !u.uevent.uaxus_foe) && /*Turn off random generation on axus's level if lawful*/
				!rn2(u.uevent.udemigod ? 25 :
				(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)) ? 35 :
			    (depth(&u.uz) > depth(&stronghold_level)) ? 50 : 70)
			){
				if (u.uevent.udemigod && xupstair && rn2(10)) {
					(void) makemon((struct permonst *)0, xupstair, yupstair, MM_ADJACENTOK);
				} //TEAM ATTACKS
				if(In_sokoban(&u.uz)){
					if(u.uz.dlevel != 1 && u.uz.dlevel != 4) makemon((struct permonst *)0, xupstair, yupstair, MM_ADJACENTSTRICT);
				} else if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz) && Is_qstart(&u.uz)){
					(void) makemon((struct permonst *)0, xdnstair, ydnstair, MM_ADJACENTOK);
				}
				else (void) makemon((struct permonst *)0, 0, 0, NO_MM_FLAGS);
			}
			if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz) && !Is_qstart(&u.uz) && !rn2(50)){
				struct monst* mtmp = makemon(&mons[PM_SEMBLANCE], rn1(COLNO-3,2), rn1(ROWNO-3,2), MM_ADJACENTSTRICT);
				//"Where stray illuminations from the Far Realm leak onto another plane, matter stirs at the beckoning of inexplicable urges before burining to ash."
				if(mtmp && canseemon(mtmp)) pline("The base matter of the world stirs at the beckoning of inexplicable urges, dancing with a semblance of life.");
			}

		    /* reset summon monster block. */
			if(u.summonMonster) u.summonMonster = FALSE;

		    /* calculate how much time passed. */
#ifdef STEED
		    if (u.usteed && u.umoved) {
			/* your speed doesn't augment steed's speed */
			moveamt = mcalcmove(u.usteed);
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
			if(Race_if(PM_HALF_DRAGON)) moveamt = (moveamt*2)/3;
			
			if(u.sealsActive&SEAL_EURYNOME && IS_POOL(levl[u.ux][u.uy].typ)){
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
			if(u.sealsActive&SEAL_ENKI){
				nmonsclose = nmonsnear = 0;
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon){
					if(mtmp->mpeaceful) continue;
					if(um_dist(u.ux,u.uy,1)){
						nmonsclose++;
						nmonsnear++;
					} else if(um_dist(u.ux,u.uy,2)){
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
							if(u.mhmax > u.ulevel && moves % 2) u.mhmax--;
							if (u.mh > u.mhmax) u.mh = u.mhmax;
						}
						else{
							if(u.uhpmax > u.ulevel && moves % 2) u.uhpmax--;
							if(u.uhp > u.uhpmax) u.uhp = u.uhpmax;
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
			if(uwep && is_lightsaber(uwep) && uwep->lamplit && u.fightingForm == FFORM_SORESU && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm))){
				switch(min(P_SKILL(FFORM_SORESU), P_SKILL(weapon_type(uwep)))){
					case P_BASIC:       moveamt = max(moveamt-6,1); break;
					case P_SKILLED:     moveamt = max(moveamt-4,1); break;
					case P_EXPERT:      moveamt = max(moveamt-3,1); break;
				}
			}
			if(youmonst.data->mmove){
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
			
		    youmonst.movement += moveamt;
			//floor how far into movement-debt you can fall.
		    if (youmonst.movement < -2*NORMAL_SPEED) youmonst.movement = -2*NORMAL_SPEED;
		    settrack();

		    monstermoves++;
		    moves++;

		      /********************************/
		     /* once-per-turn things go here */
		    /********************************/
			/* Clouds on Lolth's level deal damage */
			if(Is_lolth_level(&u.uz) && levl[u.ux][u.uy].typ == CLOUD){
				if (!(nonliving(youmonst.data) || Breathless)){
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
				struct monst *curmon, *weakdog;
				int numdogs;
				do {
					numdogs = 0;
					weakdog = (struct monst *)0;
					for(curmon = fmon; curmon; curmon = curmon->nmon){
						if(curmon->mtame && !(EDOG(curmon)->friend) && !(EDOG(curmon)->loyal) && 
							!is_suicidal(curmon->data) && !curmon->mspiritual && curmon->mvanishes < 0
						){
							numdogs++;
							if(!weakdog) weakdog = curmon;
							if(weakdog->m_lev > curmon->m_lev) weakdog = curmon;
							else if(weakdog->mtame > curmon->mtame) weakdog = curmon;
							else if(weakdog->mtame > curmon->mtame) weakdog = curmon;
							else if(weakdog->mtame > curmon->mtame) weakdog = curmon;
						}
					}
					if(weakdog && numdogs > (ACURR(A_CHA)/3) ) EDOG(weakdog)->friend = 1;
				} while(weakdog && numdogs > (ACURR(A_CHA)/3));
				
				do {
					weakdog = (struct monst *)0;
					numdogs = 0;
					for(curmon = fmon; curmon; curmon = curmon->nmon){
						if(curmon->mspiritual && curmon->mvanishes < 0){
							numdogs++;
							if(!weakdog) weakdog = curmon;
							if(weakdog->m_lev > curmon->m_lev) weakdog = curmon;
							else if(weakdog->mtame > curmon->mtame) weakdog = curmon;
							else if(weakdog->mtame > curmon->mtame) weakdog = curmon;
							else if(weakdog->mtame > curmon->mtame) weakdog = curmon;
						}
					}
					if(weakdog && numdogs > (ACURR(A_CHA)/3) ) weakdog->mvanishes = 5;
				} while(weakdog && numdogs > (ACURR(A_CHA)/3));
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
			
		    if (flags.bypasses) clear_bypasses();
		    if(Glib) glibr();
		    nh_timeout();
		    run_regions();
			
			move_gliders();

		    if (u.ublesscnt)  u.ublesscnt--;
		    if(flags.time && !flags.run)
			flags.botl = 1;
			
			if(uclockwork){
				if(u.ustove){
					if(u.uboiler){
						int steam = min(10,min(u.ustove,u.uboiler));
						lesshungry(steam);
						u.ustove-=steam;
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
								if(uclockwork) losehp(u.uhpmax*2, "melting to slag", KILLED_BY);
								else if(!(HFire_resistance || u.sealsActive&SEAL_FAFNIR)) losehp(u.uhpmax*2, "molten bronze", KILLED_BY);
							}
						}
					}
				}
			} else if(u.utemp) u.utemp = 0;
			
			if(u.uinwater) u.uboiler = MAX_BOILER;
			
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

		    /* One possible result of prayer is healing.  Whether or
		     * not you get healed depends on your current hit points.
		     * If you are allowed to regenerate during the prayer, the
		     * end-of-prayer calculation messes up on this.
		     * Another possible result is rehumanization, which requires
		     * that encumbrance and movement rate be recalculated.
		     */
		    if (u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves+20) {
				/* for the moment at least, you're in tiptop shape */
				wtcap = UNENCUMBERED;
		    } else {
				if (youracedata->mlet == S_EEL && !is_pool(u.ux,u.uy) && !Is_waterlevel(&u.uz)) {
				if (u.mh > 1) {
					u.mh--;
					flags.botl = 1;
				} else if (u.mh < 1)
					rehumanize();
				} else if (Upolyd && u.mh < u.mhmax) {
				if (u.mh < 1)
					rehumanize();
				else if (Regeneration ||
						(wtcap < MOD_ENCUMBER && !(moves%20))) {
					if(!uwep || uwep->oartifact != ART_ATMA_WEAPON || !uwep->lamplit || Drain_resistance || !rn2(4)) {
						flags.botl = 1;
						u.mh++;
					}
				}
				} else if (u.uhp < u.uhpmax &&
				 (wtcap < MOD_ENCUMBER || !u.umoved || Regeneration)) {
				if (u.ulevel > 9 && !(moves % 3) && 
					!(Race_if(PM_INCANTIFIER) || uclockwork || on_level(&valley_level, &u.uz))) {
					int heal, Con = (int) ACURR(A_CON);
					if(!uwep || uwep->oartifact != ART_ATMA_WEAPON || !uwep->lamplit || Drain_resistance || !rn2(4)) {
						if (Con < 12) {
						heal = 1;
						} else {
						heal = rnd(Con-10);
						if (heal > u.ulevel-9) heal = u.ulevel-9;
						}
						flags.botl = 1;
						u.uhp += heal;
						if(u.uhp > u.uhpmax)
						u.uhp = u.uhpmax;
					}
				} else if (Regeneration ||
					 (u.ulevel <= 9 && 
					 !(Race_if(PM_INCANTIFIER) || uclockwork || on_level(&valley_level, &u.uz)) &&
					  !(moves % ((MAXULEV+12) / (u.ulevel+2) + 1)))) {
					if(!uwep || uwep->oartifact != ART_ATMA_WEAPON || !uwep->lamplit || Drain_resistance || !rn2(4)){
						flags.botl = 1;
						u.uhp++;
					}
				}
				}
				if((uleft  && uleft->oartifact  == ART_RING_OF_HYGIENE_S_DISCIPLE)||
				   (uright && uright->oartifact == ART_RING_OF_HYGIENE_S_DISCIPLE)
				){
					if(u.uhp < u.uhpmax) u.uhp++;
					if(u.uhp < u.uhpmax / 2) u.uhp++;
					if(u.uhp < u.uhpmax / 3) u.uhp++;
					if(u.uhp < u.uhpmax / 4) u.uhp++;
				}
				if(u.sealsActive&SEAL_BUER){
					int dsize = spiritDsize(), regenrate = dsize/5, remainderrate = dsize%5;
					if(Upolyd && u.mh < u.mhmax){
						if(!uwep || uwep->oartifact != ART_ATMA_WEAPON || !uwep->lamplit || Drain_resistance){
							if(regenrate) u.mh+=regenrate;
							if(remainderrate && moves%5 < remainderrate) u.mh+=1;
						} else {
							if(regenrate && !(moves%4)) u.mh+=regenrate;
							if(remainderrate && moves%20 < remainderrate) u.mh+=1;
						}
						if(u.mh > u.mhmax) u.mh = u.mhmax;
					} else if(u.uhp < u.uhpmax){
						if(!uwep || uwep->oartifact != ART_ATMA_WEAPON || !uwep->lamplit || Drain_resistance){
							if(regenrate) u.uhp+=regenrate;
							if(remainderrate && moves%5 < remainderrate) u.uhp+=1;
						} else {
							if(regenrate && !(moves%4)) u.uhp+=regenrate;
							if(remainderrate && moves%20 < remainderrate) u.uhp+=1;
						}
						if(u.uhp > u.uhpmax) u.uhp = u.uhpmax;
					}
				}

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
			}
		    /* moving around while encumbered is hard work */
		    if (wtcap > MOD_ENCUMBER && u.umoved) {
			if(!(wtcap < EXT_ENCUMBER ? moves%30 : moves%10)) {
			    if (Upolyd && u.mh > 1) {
				u.mh--;
			    } else if (!Upolyd && u.uhp > 1) {
				u.uhp--;
			    } else {
				You("pass out from exertion!");
				exercise(A_CON, FALSE);
				fall_asleep(-10, FALSE);
			    }
			}
		    }

		    if ((u.uen < u.uenmax) && 
			((wtcap < MOD_ENCUMBER && !Race_if(PM_INCANTIFIER) &&
			  (!(moves%(((MAXULEV+2) - u.ulevel) *
				    (Role_if(PM_WIZARD) ? 3 : 4) / 6))))
			 || Energy_regeneration)) {
				u.uen += rn1((int)(ACURR(A_WIS) + ACURR(A_INT)) / 10 + 1,1);
				if (u.uen > u.uenmax)  u.uen = u.uenmax;
				flags.botl = 1;
		    }
			if(u.uen < u.uenmax && (Role_if(PM_WIZARD) || Race_if(PM_INCANTIFIER)) && uarmh && uarmh->otyp == CORNUTHAUM && uarmh->spe > 0){
				u.uen += rnd(uarmh->spe);
				if (u.uen > u.uenmax)  u.uen = u.uenmax;
				flags.botl = 1;
			}

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
				 !uclockwork &&
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
		if(u.ustdy > 0) u.ustdy -= 1;
		

		
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
						}
					}
				}
			}
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

	} /* actual time passed */

	/****************************************/
	/* once-per-player-input things go here */
	/****************************************/
	find_ac();
////////////////////////////////////////////////////////////////////////////////////////////////
	if(!flags.mv || Blind || oldBlind != (!!Blind)) {
	    /* redo monsters if hallu or wearing a helm of telepathy */
	    if (Hallucination) {	/* update screen randomly */
			see_monsters();
			see_objects();
			see_traps();
			if (u.uswallow) swallowed(0);
		} else if (Unblind_telepat) {
			see_monsters();
	    } else if (Warning || Warn_of_mon)
	     	see_monsters();

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
		} else if (Unblind_telepat) {
			see_monsters();
	    } else if (Warning || Warn_of_mon)
	     	see_monsters();

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
	if (!oldCon != ACURR(A_CON)) {
		int condif = conplus(ACURR(A_CON)) - conplus(oldCon);
		if(condif != 0) u.uhpmax += u.ulevel*condif;
		if(u.uhpmax < 1) u.uhpmax = 1;
		if(u.uhp > u.uhpmax) u.uhp = u.uhpmax;
		oldCon = ACURR(A_CON);
	}
////////////////////////////////////////////////////////////////////////////////////////////////
	if (!oldWisBon != ACURR(A_WIS)/4) {
		u.uenmax += u.ulevel*(ACURR(A_WIS)/4 - oldWisBon);
		if(u.uenmax < 0) u.uenmax = 0;
		if(u.uen > u.uenmax) u.uen = u.uenmax;
		oldWisBon = ACURR(A_WIS)/4;
	}
////////////////////////////////////////////////////////////////////////////////////////////////
	/*If anything we did caused us to get moved out of water, surface*/
	if(u.usubwater && !is_pool(u.ux, u.uy)){
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
	role_init();		/* must be before init_dungeons(), u_init(),
				 * and init_artifacts() */
	
	init_dungeons();	/* must be before u_init() to avoid rndmonst()
				 * creating odd monsters for any tins and eggs
				 * in hero's initial inventory */
	init_artifacts();	/* before u_init() in case $WIZKIT specifies
				 * any artifacts */
	u_init();

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
			com_pager(219);
			com_pager(220);
			com_pager(221);
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

	if(Race_if(PM_DROW)) litroom(FALSE,NULL);
	/* Success! */
	welcome(TRUE);
	return;
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
	else if(Race_if(PM_CLOCKWORK_AUTOMATON)){
		pline("Use #monster to adjust your clockspeed.");
		You("do not heal naturally. Use '.' to attempt repairs.");
	}
	else if(Race_if(PM_INCANTIFIER)){
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
			if(is_flyer(&mons[i]))	Sprintf(eos(pbuf)," |fly=%s\n", is_flyer(&mons[i]) ? "1":"");
			if(is_swimmer(&mons[i]))	Sprintf(eos(pbuf)," |swim=%s\n", is_swimmer(&mons[i]) ? "1":"");
			if(amorphous(ptr))	Sprintf(eos(pbuf)," |amorphous=%s\n", amorphous(ptr) ? "1":"");
			if(passes_walls(ptr))	Sprintf(eos(pbuf)," |wallwalk=%s\n", passes_walls(ptr) ? "1":"");
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
			if(regenerates(ptr))	Sprintf(eos(pbuf)," |regen=%s\n", regenerates(ptr) ? "1":"");
			if(perceives(ptr))	Sprintf(eos(pbuf)," |seeinvis=%s\n", perceives(ptr) ? "1":"");
			if(can_teleport(ptr))	Sprintf(eos(pbuf)," |tport=%s\n", can_teleport(ptr) ? "1":"");
			if(control_teleport(ptr))	Sprintf(eos(pbuf)," |tportcntrl=%s\n", control_teleport(ptr) ? "1":"");
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
			if((!regenerates(ptr) && nonliving(ptr)))	Sprintf(eos(pbuf)," |noregen=%s\n", (!regenerates(ptr) && nonliving(ptr)) ? "1":"");
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
			Sprintf(pbuf,"[[%s|{{%s|%c}}]]", mons[i].mname, colorstr, def_monsyms[(int)mons[i].mlet]);
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
		"Gaze attack",	/*15*/
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
		"Deva Arms"	/*29*/
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
		"[[ahazu abduction]]",	/*109*/
		"[[stone choir]]",		/*110*/
		"[[water vampire]]",	/*111*/
		"[[bloody fangs]]",		/*112*/
		"[[item freeing]]",		/*113*/
		"[[rainbow feathers]]",	/*114*/
		"[[Vorlon explosion]]",	/*115*/
		"[[cold explosion]]",	/*116*/
		"[[fire explosion]]",	/*117*/
		"[[shock explosion]]",	/*118*/
		"[[physical explosion]]",/*119*/
		"[[Vorlon missile]]",	/*120*/
		"[[Warmachine missile]]",/*121*/
		"[[clerical spell]]",	/*122*/
		"[[mage spell]]",		/*123*/
		"[[random breath type]]",/*124*/
		"[[random gaze type]]",	/*125*/
		"[[random elemental gaze]]",/*126*/
		"[[Amulet theft]]",		/*127*/
		"[[Intrinsic theft]]",	/*128*/
		"[[Quest Artifact theft]]"/*129*/
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
				attk->aatyp == AT_MAGC ? "Cast" :
				attackKey[((int)attk->aatyp)],
				attk->damn,
				attk->damd,
				damageKey[((int)attk->adtyp)]
			);
		} else {
			Sprintf(eos(buf), ", %s %dd%d %s",
				attk->aatyp == AT_WEAP ? "Weapon" :
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
