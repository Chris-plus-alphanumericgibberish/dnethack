#include "hack.h"
//#include "artifact.h"
//#include "monflag.h"
#include "edog.h"

# ifdef SEDUCE
STATIC_DCL void FDECL(mayberem, (struct obj *, const char *));
STATIC_DCL void FDECL(lrdmayberem, (struct obj *, const char *));
STATIC_DCL void FDECL(mlcmayberem, (struct obj *, const char *, BOOLEAN_P));
STATIC_DCL void FDECL(sflmayberem, (struct obj *, const char *, BOOLEAN_P));
STATIC_DCL void FDECL(palemayberem, (struct obj *, const char *, BOOLEAN_P));
# endif

static const char tools[] = { TOOL_CLASS, 0 };

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
		pagr = youracedata;
		agrinvis = (Invis != 0);
		genagr = poly_gender();
	} else {
		pagr = magr->data;
		agrinvis = magr->minvis;
		genagr = gender(magr);
	}
	if(mdef == &youmonst) {
		defperc = (See_invisible(magr->mx,magr->my) != 0);
		gendef = poly_gender();
	} else {
		defperc = mon_resistance(mdef,SEE_INVIS);
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

	if(pagr == &mons[PM_SMALL_GOAT_SPAWN] || pagr == &mons[PM_GOAT_SPAWN] || pagr == &mons[PM_GIANT_GOAT_SPAWN])
		return 1;
	
	if(pagr->mlet == S_NYMPH || pagr == &mons[PM_INCUBUS] || pagr == &mons[PM_SUCCUBUS]
			|| pagr == &mons[PM_CARMILLA] || pagr == &mons[PM_VLAD_THE_IMPALER] || pagr == &mons[PM_LEVISTUS]){
		if(genagr == 1 - gendef)
			return 1;
		else
			return (pagr->mlet == S_NYMPH || pagr == &mons[PM_LEVISTUS]) ? 2 : 0;
	}
	else if(pagr == &mons[PM_MOTHER_LILITH] || pagr == &mons[PM_BELIAL]
		 /*|| pagr == &mons[PM_SHAMI_AMOURAE]*/){
		if(genagr == 1 - gendef) return 1;
		else return 0;
	}
	else if(pagr == &mons[PM_FIERNA]) return 2;
	else if(pagr == &mons[PM_ALRUNES]) return (genagr == gendef) ? 1 : 2;
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
	boolean fem = mon->female; /* otherwise incubus */
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
		if (Sterile || rn2(35) > ACURR(A_CHA) + ACURR(A_INT)) {
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
					steal(mon, buf, FALSE, FALSE);
				}
				goto pay;
			}
			switch (rn2(5)) {
				case 0: You_feel("drained of energy.");
					u.uen = 0;
					u.uenbonus -= rnd(Half_physical_damage ? 5 : 10);
						exercise(A_CON, FALSE);
					calc_total_maxen();
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
						pline1(Never_mind);
						goto pay;
					}
					turns = ask_turns(mon, 0, 0);
					if(!turns){
						pline1(Never_mind);
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
				u.uenbonus += rnd(10)+5;
				calc_total_maxen();
				u.uen = min(u.uen+400,u.uenmax);
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
					pline("...then starts picking through your things!");
					buf[0] = '\0';
					steal(mon, buf, FALSE, TRUE);
					buf[0] = '\0';
					steal(mon, buf, FALSE, TRUE);
					buf[0] = '\0';
					steal(mon, buf, FALSE, TRUE);
					buf[0] = '\0';
					steal(mon, buf, FALSE, TRUE);
					buf[0] = '\0';
					steal(mon, buf, FALSE, TRUE);
					buf[0] = '\0';
					steal(mon, buf, FALSE, TRUE);
					buf[0] = '\0';
					steal(mon, buf, FALSE, TRUE);
					buf[0] = '\0';
					steal(mon, buf, FALSE, TRUE);
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
				if (has_blood(youracedata)) {
				   Your("blood is being drained!");
				   /* Get 1/20th of full corpse value
					* Therefore 4 bites == 1 drink
					*/
					if (mon->mtame && !mon->isminion)
						EDOG(mon)->hungrytime += ((int)((youracedata)->cnutrit / 20) + 1);
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
					verbalize("You're such a sweet lady, I wish you were more open to new things...");
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
				makewish(WISH_VERBOSE);	// can not grant artifacts
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
	if (Sterile || rn2(139) > ACURR(A_CHA) + ACURR(A_INT)) {
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
				steal(mon, buf, FALSE, FALSE);
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
				u.uenbonus -= rnd(Half_physical_damage ? 45 : 90);
					exercise(A_CON, FALSE);
				calc_total_maxen();
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
					pline1(Never_mind);
					goto pay;
				}
				turns = ask_turns(mon, 0, 0);
				if(!turns){
					pline1(Never_mind);
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
			u.uenbonus += rnd(10)+5;
			calc_total_maxen();
			u.uen = min(u.uen+400,u.uenmax);
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
			u.uenbonus += rnd(10)+5;
			calc_total_maxen();
			u.uen = min(u.uen+400,u.uenmax);
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
	if (Sterile || rn2(139) > ACURR(A_CHA) + ACURR(A_INT)) {
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
				steal(mon, buf, FALSE, FALSE);
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
				u.uenbonus -= rnd(Half_physical_damage ? 45 : 90);
					exercise(A_CON, FALSE);
				calc_total_maxen();

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
					pline1(Never_mind);
					goto pay;
				}
				turns = ask_turns(mon, 0, 0);
				if(!turns){
					pline1(Never_mind);
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
			u.uenbonus += rnd(10)+5;
			calc_total_maxen();
			u.uen = min(u.uenmax, u.uen+400);
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
			u.uenbonus += rnd(10)+5;
			calc_total_maxen();
			u.uen = min(u.uen+400,u.uenmax);
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
	if (Sterile || helpless || rn2(120) > ACURR(A_CHA) + ACURR(A_CON) + ACURR(A_INT)) {
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
				u.uenbonus -= Half_physical_damage ? 45 : 90;
					exercise(A_CON, FALSE);
				calc_total_maxen();

				break;
			case 1: You("are down in the dumps.");
				u.uhpmod -= Half_physical_damage ? 25 : 50;
				calc_total_maxhp();
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
					forget(30);
					exercise(A_WIS, FALSE);
					exercise(A_WIS, FALSE);
					exercise(A_WIS, FALSE);
				}
				flags.botl = 1;
				break;
			case 3:
				Your("pack feels heavier.");
				(void) adjattrib(A_STR, -2, TRUE);
				u.ucarinc -= 100;
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
		    if (Levitation || Weightless||Is_waterlevel(&u.uz))
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
						u.uhpmultiplier += 2;
						u.uenmultiplier += 2;
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
				u.uhpmultiplier += 2;
				u.uenmultiplier += 2;
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
	if (Sterile || helpless || rn2(120) > ACURR(A_CHA) + ACURR(A_CON) + ACURR(A_INT)) {
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
				steal(mon, buf, FALSE, FALSE);
				buf[0] = '\0';
				steal(mon, buf, FALSE, FALSE);
				buf[0] = '\0';
				steal(mon, buf, FALSE, FALSE);
				buf[0] = '\0';
				steal(mon, buf, FALSE, FALSE);
				buf[0] = '\0';
				steal(mon, buf, FALSE, FALSE);
				buf[0] = '\0';
				steal(mon, buf, FALSE, FALSE);
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
				else if(uwep && !(uwep->otyp != BAR)){
					verbalize("You're going to hurt yourself with that.");
					optr = uwep;
					setuwep((struct obj *)0);
					freeinv(optr);
					(void) mpickobj(mon,optr);
				}
				if(!uwep){
					buf[0] = '\0';
					steal(mon, buf, FALSE, FALSE);
					optr = mksobj(BAR, TRUE, FALSE);		
					curse(optr);
					optr->spe = -6;
					verbalize("This will keep you out of trouble.");
					(void) hold_another_object(optr, u.uswallow ?
							   "Fortunately, you're out of reach! %s away." :
							   "Fortunately, you can't hold anything more! %s away.",
							   The(aobjnam(optr,
								 Weightless || u.uinwater ?
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
					steal(mon, buf, FALSE, FALSE);
					optr = mksobj(DUNCE_CAP, TRUE, FALSE);		
					curse(optr);
					optr->spe = -6;
					(void) hold_another_object(optr, u.uswallow ?
							   "Fortunately, you're out of reach! %s away." :
							   "Fortunately, you can't hold anything more! %s away.",
							   The(aobjnam(optr,
								 Weightless || u.uinwater ?
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
					steal(mon, buf, FALSE, FALSE);
					optr = mksobj(FUMBLE_BOOTS, TRUE, FALSE);		
					curse(optr);
					optr->spe = -6;
					(void) hold_another_object(optr, u.uswallow ?
							   "Fortunately, you're out of reach! %s away." :
							   "Fortunately, you can't hold anything more! %s away.",
							   The(aobjnam(optr,
								 Weightless || u.uinwater ?
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
					steal(mon, buf, FALSE, FALSE);
					optr = mksobj(AMULET_OF_RESTFUL_SLEEP, TRUE, FALSE);		
					curse(optr);
					(void) hold_another_object(optr, u.uswallow ?
							   "Fortunately, you're out of reach! %s away." :
							   "Fortunately, you can't hold anything more! %s away.",
							   The(aobjnam(optr,
								 Weightless || u.uinwater ?
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
			makewish(WISH_VERBOSE);	// can not grant artifacts
		break;
		case 1:
			verbalize("Tell me, whom shall I kill?");
			do_genocide(1);
		break;
		case 2:{
			int i;
			verbalize("I grant you six magics!");
			for (i = 6; i > 0; i--) {
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
			int i;
			verbalize("I grant you six truths!");
			for (i = 6; i > 0; i--) {
				optr = mksobj(POT_ENLIGHTENMENT, TRUE, FALSE);
				bless(optr);
				hold_another_object(optr, u.uswallow ?
				       "Oops!  %s out of your reach!" :
				       (Weightless ||
					Is_waterlevel(&u.uz) ||
					levl[u.ux][u.uy].typ < IRONBARS ||
					levl[u.ux][u.uy].typ >= ICE) ?
				       "Oops!  %s away from you!" :
				       "Oops!  %s to the floor!",
				       The(aobjnam(optr,
					     Weightless || u.uinwater ?
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
				       (Weightless ||
					Is_waterlevel(&u.uz) ||
					levl[u.ux][u.uy].typ < IRONBARS ||
					levl[u.ux][u.uy].typ >= ICE) ?
				       "Oops!  %s away from you!" :
				       "Oops!  %s to the floor!",
				       The(aobjnam(optr,
					     Weightless || u.uinwater ?
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
			int i;
			verbalize("I grant you six followers!");
			for (i = 6; i > 0; i--) {
				optr = mksobj(FIGURINE, TRUE, FALSE);
				bless(optr);
				hold_another_object(optr, u.uswallow ?
				       "Oops!  %s out of your reach!" :
				       (Weightless ||
					Is_waterlevel(&u.uz) ||
					levl[u.ux][u.uy].typ < IRONBARS ||
					levl[u.ux][u.uy].typ >= ICE) ?
				       "Oops!  %s away from you!" :
				       "Oops!  %s to the floor!",
				       The(aobjnam(optr,
					     Weightless || u.uinwater ?
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
	if (helpless || (25 + rn2(100)) > (ACURR(A_CHA) + ACURR(A_STR))) {
		int turns = d(1, 4);
		if(!helpless) You("are taken off guard!");
		nomul(-(turns), "knocked reeling by a violent lover");
		nomovemsg = You_can_move_again;
		mon->mspec_used = turns;
		return 0;
	} else {
		pline("But you gain the upper hand!");
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
		You("move to embrace %s, brushing aside the gossamer shroud hiding %s body from you.",
			noit_Monnam(mon), fem ? "her" : "his");
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
		if(rn2( (int)(ACURR(A_WIS)/2))){
			boolean loopingDeath = TRUE;
			while(loopingDeath) {
				boolean has_lifesaving = Lifesaved;
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
				if (lifesaved && (discover || wizard || has_lifesaving)) {
					if (has_lifesaving) {
						/* used up AMULET_OF_LIFE_SAVING; still
						   subject to dying from memory */
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
			forget(25);	/* lose 25% of memory */
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
	boolean fem = mon->female; /* otherwise incubus */
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
	if (Sterile || rn2(35) > (ACURR(A_CHA) + ACURR(A_INT))) {
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
				steal(mon, buf, FALSE, FALSE);
			}
			goto pay;
		}
		switch (rn2(5)) {
			case 0: You_feel("drained of energy.");
				u.uen = 0;
				u.uenbonus -= rnd(Half_physical_damage ? 5 : 10);
					exercise(A_CON, FALSE);
				calc_total_maxen();

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
					 dmgtype(youracedata, AD_HALU));
				if (!not_affected) {
				    boolean chg;
				    chg = make_hallucinated(HHallucination + (long)tmp,FALSE,0L);
				    You("%s.", chg ? "are freaked out" : "seem unaffected");
				}
				(void) adjattrib(A_INT, -1, FALSE);
				forget(5);
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
					pline1(Never_mind);
					goto pay;
				}
				turns = ask_turns(mon, 0, 0);
				if(!turns){
					pline1(Never_mind);
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
			u.uenbonus += rnd(10)+5;
			calc_total_maxen();
			u.uen = min(u.uen+400,u.uenmax);
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
				if(!slips_free(mon, &youmonst,  &bodyblow, -1)){
					You_feel("the tentacles squirm under your underclothes.");
					if( d(1,100) > 15){
						pline("The tentacles begin to tear at your underclothes!");
						 if(uarmu->spe > 1){
							for(i=rn2(4); i>=0; i--)
								drain_item(uarmu);
							Your("%s less effective.", aobjnam(uarmu, "seem"));
						 }
						 else{
							tent_destroy_arm(uarmu);
							if(!uarmu) change_usanity(u_sanity_loss(mon)/2); /*Forces a san check*/
						 }
					}
					else{
						pline("The tentacles pull your underclothes off!");
						otmp = uarmu;
						if (donning(otmp)) cancel_don();
						(void) Shirt_off();
						freeinv(otmp);
						(void) mpickobj(mon,otmp);
						change_usanity(u_sanity_loss(mon)/2); /*Forces a san check*/
						if(roll_madness(MAD_TALONS)){
							You("panic after having your underclothes pulled off!");
							nomul(-1*rnd(6),"panicking");
						}
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
			 if(!slips_free(mon, &youmonst,  &bodyblow, -1)){
				You_feel("the tentacles squirm under your armor.");
				if( d(1,100) > 25){
					pline("The tentacles begin to tear at your armor!");
					 if(uarm->spe > 1){
						for(i=rn2(4); i>=0; i--)
							drain_item(uarm);
						Your("%s less effective.", aobjnam(uarm, "seem"));
					 }
					 else{
						tent_destroy_arm(uarm);
						if(!uarm) change_usanity(u_sanity_loss(mon)/2); /*Forces a san check*/
					 }
				}
				else{
					pline("The tentacles shuck you out of your armor!");
					otmp = uarm;
					if (donning(otmp)) cancel_don();
					(void) Armor_gone();
					freeinv(otmp);
					(void) mpickobj(mon,otmp);
					change_usanity(u_sanity_loss(mon)/2); /*Forces a san check*/
					if(roll_madness(MAD_TALONS)){
						You("panic after having your armor removed!");
						nomul(-1*rnd(6),"panicking");
					}
				}
		  }
		 }
		}
		if(uarmc && n){
			n--;
			if(!slips_free(mon, &youmonst,  &bodyblow, -1)){
				You_feel("the tentacles work their way under your cloak.");
				if( d(1,100) > 66){
					pline("The tentacles begin to tear at the cloak!");
					 if(uarmc->spe > 1){
						for(i=rn2(4); i>=0; i--)
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
					if(roll_madness(MAD_TALONS)){
						You("panic after having your cloak taken!");
						nomul(-1*rnd(6),"panicking");
					}
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
				if(!slips_free(mon, &youmonst,  &legblast, -1)){
					You_feel("the tentacles squirm into your boots.");
					if( d(1,100) > 66){
						pline("The tentacles begin to tear at your boots!");
						 if(uarmf->spe > 1){
							for(i=rn2(4); i>=0; i--)
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
							if(roll_madness(MAD_TALONS)){
								You("panic after having your boots sucked off!");
								nomul(-1*rnd(6),"panicking");
							}
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
					if(roll_madness(MAD_TALONS)){
						You("panic after having your weapon taken!");
						nomul(-1*rnd(6),"panicking");
					}
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
				if(!slips_free(mon, &youmonst,  &handshit, -1)){
					You_feel("the tentacles squirm into your gloves.");
					if( d(1,40) <= ACURR(A_STR) || uwep){
						pline("The tentacles begin to tear at your gloves!");
						 if(uarmg->spe > 1){
							for(i=rn2(4); i>=0; i--)
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
						if(roll_madness(MAD_TALONS)){
							You("panic after having your gloves sucked off!");
							nomul(-1*rnd(6),"panicking");
						}
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
					if(roll_madness(MAD_TALONS)){
						You("panic after having your shield taken!");
						nomul(-1*rnd(6),"panicking");
					}
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
				if(!slips_free(mon, &youmonst,  &headshot, -1)){
					You_feel("the tentacles squirm under your helmet.");
					if( d(1,100) > 90){
						pline("The tentacles begin to tear at your helmet!");
						 if(uarmh->spe > 1){
							for(i=rn2(4); i>=0; i--)
								drain_item(uarmh);
							Your("%s less effective.", aobjnam(uarmh, "seem"));
						 }
						 else{
							tent_destroy_arm(uarmh);
							if(!uarmh) change_usanity(u_sanity_loss(mon)/2); /*Forces a san check*/
						 }
					}
					else{
						pline("The tentacles pull your helmet off!");
						otmp = uarmh;
						if (donning(otmp)) cancel_don();
						(void) Helmet_off();
						freeinv(otmp);
						(void) mpickobj(mon,otmp);
						change_usanity(u_sanity_loss(mon)/2); /*Forces a san check*/
						if(roll_madness(MAD_TALONS)){
							You("panic after having your helmet stolen!");
							nomul(-1*rnd(6),"panicking");
						}
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
				if(Half_physical_damage) u.uenbonus -= (int) max(.1*u.uenmax,5);
				else u.uenbonus -= (int) max(.2*u.uenmax, 10);
				calc_total_maxen();
				change_usanity(u_sanity_loss(mon)); /*Forces a san check*/
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
						forget(10);	/* lose 10% of memory per point lost*/
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
				change_usanity(u_sanity_loss(mon)); /*Forces a san check*/
				
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
					losexp("soul-shredding tentacles",FALSE,FALSE,FALSE);
					losexp("soul-shredding tentacles",FALSE,FALSE,FALSE);
					losexp("soul-shredding tentacles",FALSE,FALSE,FALSE);
					i = d(1,4);
					while(i-- > 0){
						losexp("soul-shredding tentacles",FALSE,FALSE,TRUE);
						exercise(A_WIS, FALSE);
						exercise(A_WIS, FALSE);
						exercise(A_WIS, FALSE);
					}
					(void) adjattrib(A_CON, -4, 1);
					You_feel("violated and very fragile. Your soul seems a thin and tattered thing.");
				} else {
					(void) adjattrib(A_CON, -2, 1);
					You_feel("a bit fragile, but strangely whole.");
				}
				losehp(Half_physical_damage ? dmg/4+1 : dmg/2+1, "drilling tentacles", KILLED_BY);
				change_usanity(u_sanity_loss(mon)); /*Forces a san check*/
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
				change_usanity(u_sanity_loss(mon)); /*Forces a san check*/
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
					steal(mon, buf,FALSE, FALSE);
					buf[0] = '\0';
					steal(mon, buf,FALSE, FALSE);
					buf[0] = '\0';
					steal(mon, buf,FALSE, FALSE);
					buf[0] = '\0';
					steal(mon, buf,FALSE, FALSE);
				}
			break;
		}
		}
	if(roll_madness(MAD_HELMINTHOPHOBIA)){
		You("panic from the squirming tentacles!");
		nomul(-1*rnd(6),"panicking");
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
register struct obj *obj;
const char *str;
boolean helpless;
{
	char qbuf[QBUFSZ];

	if (!obj || !obj->owornmask) return;

	if (!helpless && rn2(60) < ACURR(A_CHA)) {
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
	if (!helpless && her_strength < ACURR(A_STR)) {
		Sprintf(qbuf,"She tries to take off your %s, allow her?",
			str);
		if (yn(qbuf) == 'n') return;
	} else if(her_strength > ACURR(A_STR)*2){
		Sprintf(qbuf,"She tries to rip open your %s!",
			str);
		her_strength -= ACURR(A_STR);
		for(; her_strength >= 0; her_strength--){
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

/*mhitu.c*/
