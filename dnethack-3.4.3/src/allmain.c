/*	SCCS Id: @(#)allmain.c	3.4	2003/04/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* various code that was replicated in *main.c */

#include "hack.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

#ifdef POSITIONBAR
STATIC_DCL void NDECL(do_positionbar);
#endif

#ifdef OVL0

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
	static boolean oldBlind = 0;

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
    if (wizard) add_debug_extended_commands();
#endif

    (void) encumber_msg(); /* in case they auto-picked up something */

    u.uz0.dlevel = u.uz.dlevel;
    youmonst.movement = NORMAL_SPEED;	/* give the hero some movement points */

    for(;;) {/////////////////////////MAIN LOOP/////////////////////////////////
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
			if (!oldBlind ^ !Blind) {  /* one or the other but not both */
				see_monsters();
				flags.botl = 1;
				vision_full_recalc = 1;	/* blindness just got toggled */
				if (Blind_telepat || Infravision) see_monsters();
				oldBlind = !!Blind;
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
			if(u.sealsActive&SEAL_FAFNIR && u.ugold < u.ulevel*100) unbind(SEAL_FAFNIR,TRUE);
#else
			if(u.sealsActive&SEAL_FAFNIR && money_cnt(invent) < u.ulevel*100) unbind(SEAL_FAFNIR,TRUE);
#endif
			if(u.sealsActive&SEAL_JACK && (Is_astralevel(&u.uz) || Inhell)) unbind(SEAL_JACK,TRUE);
			if(u.sealsActive&SEAL_ORTHOS && !(u.sealsActive&SEAL_AMON) 
				&&!(viz_array[u.uy][u.ux]&TEMP_LIT || levl[u.ux][u.uy].lit)
			){
				if(++u.orthocounts>5) unbind(SEAL_ORTHOS,TRUE);
				else if(Hallucination){
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
		    for (mtmp = fmon; mtmp; mtmp = nxtmon){
				nxtmon = mtmp->nmon;
				/* Possibly vanish */
				if(mtmp->mvanishes>-1){
					if(mtmp->mvanishes-- == 0){
						monvanished(mtmp);
						continue;
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
						if(sensemon(mtmp) || ((cansee(mtmp->mx,mtmp->my) || see_with_infrared(mtmp)) && canspotmon(mtmp) && !mtmp->mundetected)){
							pline("%s is laughing hysterically.", Monnam(mtmp));
						} else if(couldsee(mtmp->mx,mtmp->my)){
							You_hear("hysterical laughter.");
						} else {
							You_hear("laughter in the distance.");
						}
					} else pline("%s is trembling hysterically.", Monnam(mtmp));
				}
				if(mtmp->data == &mons[PM_GREAT_CTHULHU] || mtmp->data == &mons[PM_ZUGGTMOY] 
					|| mtmp->data == &mons[PM_SWAMP_FERN]) mtmp->mspec_used = 0;
				if(is_weeping(mtmp->data)) mtmp->mspec_used = 0;
				if(mtmp->data == &mons[PM_CLOCKWORK_SOLDIER] || mtmp->data == &mons[PM_CLOCKWORK_DWARF] || 
				   mtmp->data == &mons[PM_FABERGE_SPHERE] || mtmp->data == &mons[PM_FIREWORK_CART] ||
				   mtmp->data == &mons[PM_ID_JUGGERNAUT]
				) if(rn2(2)) mtmp->mextra[0] = ((int)mtmp->mextra[0] + rn2(3)-1)%8;
				if((mtmp->data == &mons[PM_JUGGERNAUT] || mtmp->data == &mons[PM_ID_JUGGERNAUT]) && !rn2(3)){
					int mdx=0, mdy=0, i;
					if(mtmp->mux - mtmp->mx < 0) mdx = -1;
					else if(mtmp->mux - mtmp->mx > 0) mdx = +1;
					if(mtmp->muy - mtmp->my < 0) mdy = -1;
					else if(mtmp->muy - mtmp->my > 0) mdy = +1;
					for(i=0;i<8;i++) if(xdir[i] == mdx && ydir[i] == mdy) break;
					if(mtmp->mextra[0] != i){
						if(sensemon(mtmp) || ((cansee(mtmp->mx,mtmp->my) || see_with_infrared(mtmp)) && canspotmon(mtmp) && !mtmp->mundetected)){
							pline("%s turns to a new heading.", Monnam(mtmp));
						} else if(couldsee(mtmp->mx,mtmp->my)){
							You_hear("a loud scraping noise.");
						} else {
							You_hear("scraping in the distance.");
						}
						mtmp->mextra[0] = i;
						mtmp->movement = -12;
					}
				}
			} /* movement rations */

		    if(!rn2(u.uevent.udemigod ? 25 :
			    (depth(&u.uz) > depth(&stronghold_level)) ? 50 : 70)
			){
				if (u.uevent.udemigod && xupstair && rn2(10)) {
					(void) makemon((struct permonst *)0, xupstair, yupstair, MM_ADJACENTOK);
				} //TEAM ATTACKS
				if(In_sokoban(&u.uz)){
					if(u.uz.dlevel != 1 && u.uz.dlevel != 4) makemon((struct permonst *)0, xupstair, yupstair, MM_ADJACENTSTRICT);
				}
				else (void) makemon((struct permonst *)0, 0, 0, NO_MM_FLAGS);
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
					morehungry(10);
				}
			}
		    } else
#endif
		    {
			moveamt = youmonst.data->mmove;
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
				if(!is_undead(youmonst.data) && u.ZangetsuSafe-- < 1){
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
			else if(u.ZangetsuSafe < u.ulevel && !(moves%10)) u.ZangetsuSafe++;
			
			if(!(moves%10)){
				if(u.eurycounts) u.eurycounts--;
				if(u.orthocounts) u.orthocounts--;
				if(u.wimage){
					exercise(A_INT, TRUE);
					exercise(A_WIS, FALSE);
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
			}
			
			if(u.petattacked){
				u.petattacked = FALSE;
				use_skill(P_BEAST_MASTERY, 1);
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
					u.protean = rnz(100)+d(3,10);
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
		    } else if (Upolyd && youmonst.data->mlet == S_EEL && !is_pool(u.ux,u.uy) && !Is_waterlevel(&u.uz)) {
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
			if(u.sealsActive&SEAL_BUER){
				if(Upolyd && u.mh < u.mhmax){
					if(!uwep || uwep->oartifact != ART_ATMA_WEAPON || !uwep->lamplit || Drain_resistance || !rn2(4)) u.mh++;
				} else if(u.uhp < u.uhpmax){
					if(!uwep || uwep->oartifact != ART_ATMA_WEAPON || !uwep->lamplit || Drain_resistance || !rn2(4)) u.uhp++;
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
			if(Teleportation && !rn2(85)) {
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
		u.ustdy /= 2;
		
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

	} /* actual time passed */

	/****************************************/
	/* once-per-player-input things go here */
	/****************************************/
	find_ac();
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
			com_pager(207);
#ifdef CONVICT
		} else if (Role_if(PM_CONVICT)) {
		    com_pager(199);
#endif /* CONVICT */
        // } else if(Race_if(PM_ELF)){
			// com_pager(201);
		} else if(Race_if(PM_ELF) && (Role_if(PM_PRIEST) || Role_if(PM_RANGER) || Role_if(PM_NOBLEMAN) || Role_if(PM_WIZARD))){
			com_pager(201);
		} else if(Race_if(PM_WORM_THAT_WALKS)){
			if(Role_if(PM_CONVICT)){
				com_pager(204);
			} else if(Race_if(PM_ELF) && (Role_if(PM_PRIEST) || Role_if(PM_RANGER) || Role_if(PM_NOBLEMAN) || Role_if(PM_WIZARD))){
				com_pager(203);
			} else{
				com_pager(202);
			}
			com_pager(205);
			com_pager(206);
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


#endif /* OVLB */

/*allmain.c*/
