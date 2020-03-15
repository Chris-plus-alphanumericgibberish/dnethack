/*	SCCS Id: @(#)pray.c	3.4	2003/03/23	*/
/* Copyright (c) Benson I. Margulies, Mike Stephenson, Steve Linhart, 1989. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "epri.h"
#include "artifact.h"
#include "artilist.h"

extern const int monstr[];

STATIC_PTR int NDECL(prayer_done);
STATIC_DCL struct obj *NDECL(worst_cursed_item);
STATIC_DCL int NDECL(in_trouble);
STATIC_DCL void FDECL(fix_worst_trouble,(int));
STATIC_DCL void FDECL(at_your_feet, (const char *));
#ifdef ELBERETH
STATIC_DCL void NDECL(gcrownu);
#endif	/*ELBERETH*/
STATIC_DCL void FDECL(pleased,(ALIGNTYP_P));
STATIC_DCL void FDECL(godvoice,(ALIGNTYP_P,const char*));
STATIC_DCL void FDECL(god_zaps_you,(ALIGNTYP_P));
STATIC_DCL void FDECL(fry_by_god,(ALIGNTYP_P));
STATIC_DCL void FDECL(consume_offering,(struct obj *));
STATIC_DCL boolean FDECL(water_prayer,(BOOLEAN_P));
STATIC_DCL boolean FDECL(blocked_boulder,(int,int));
static void NDECL(lawful_god_gives_angel);
static void FDECL(god_gives_pet,(const char *,ALIGNTYP_P));
static void FDECL(god_gives_benefit,(ALIGNTYP_P));

/* simplify a few tests */
#define Cursed_obj(obj,typ) ((obj) && (obj)->otyp == (typ) && (obj)->cursed)

/*
 * Logic behind deities and altars and such:
 * + prayers are made to your god if not on an altar, and to the altar's god
 *   if you are on an altar
 * + If possible, your god answers all prayers, which is why bad things happen
 *   if you try to pray on another god's altar
 * + sacrifices work basically the same way, but the other god may decide to
 *   accept your allegiance, after which they are your god.  If rejected,
 *   your god takes over with your punishment.
 * + if you're in Gehennom, all messages come from Moloch
 */

/*
 *	Moloch, who dwells in Gehennom, is the "renegade" cruel god
 *	responsible for the theft of the Amulet from Marduk, the Creator.
 *	Moloch is unaligned.
 */
//definition of externs in you.h
const char	*Moloch = "Moloch";
const char	*Morgoth = "Melkor";
const char	*MolochLieutenant = "Moloch, lieutenant of Melkor";
const char	*Silence = "The Silence";
const char	*Chaos = "Chaos";
const char	*DeepChaos = "Chaos, with Cosmos in chains";
const char	*tVoid = "the void";
const char	*Demiurge = "Yaldabaoth";
const char	*Sophia = "Pistis Sophia";
const char	*Other = "an alien god";
const char	*BlackMother = "the Black Mother";
const char	*Nodens = "Nodens";
const char	*DreadFracture = "the Dread Fracture";
const char	*AllInOne = "Yog-Sothoth";

static const char *godvoices[] = {
    "booms out",
    "thunders",
    "rings out",
    "booms",
};

/* values calculated when prayer starts, and used when completed */
static aligntyp p_aligntyp;
static int p_trouble;
static int p_type; /* (-1)-3: (-1)=really naughty, 3=really good */

#define PIOUS 20
#define DEVOUT 14
#define FERVENT 9
#define STRIDENT 4
#define ALIGNED 3
#define HALTING 1
#define NOMINAL 0
#define STRAYED -3
#define SINNED -8

/*
 * The actual trouble priority is determined by the order of the
 * checks performed in in_trouble() rather than by these numeric
 * values, so keep that code and these values synchronized in
 * order to have the values be meaningful.
 */

#define TROUBLE_STONED			17
#define TROUBLE_FROZEN_AIR		16
#define TROUBLE_SLIMED			15
#define TROUBLE_STRANGLED		14
#define TROUBLE_LAVA			13
#define TROUBLE_SICK			12
#define TROUBLE_STARVING		11
#define TROUBLE_HIT			 	10
#define TROUBLE_WIMAGE		 	9
#define TROUBLE_MORGUL		 	8
#define TROUBLE_HPMOD		 	7
#define TROUBLE_LYCANTHROPE		 6
#define TROUBLE_COLLAPSING		 5
#define TROUBLE_STUCK_IN_WALL		 4
#define TROUBLE_CURSED_LEVITATION	 3
#define TROUBLE_UNUSEABLE_HANDS		 2
#define TROUBLE_CURSED_BLINDFOLD	 1

#define TROUBLE_PUNISHED	       (-1)
#define TROUBLE_FUMBLING	       (-2)
#define TROUBLE_CURSED_ITEMS	   (-3)
#define TROUBLE_SADDLE		       (-4)
#define TROUBLE_CARRY_CURSED	   (-5)
#define TROUBLE_TOHIT_CURSED	   (-6)
#define TROUBLE_AC_CURSED		   (-7)
#define TROUBLE_DAMAGE_CURSED	   (-8)
#define TROUBLE_BLIND		       (-9)
#define TROUBLE_POISONED	      (-10)
#define TROUBLE_WOUNDED_LEGS	  (-11)
#define TROUBLE_HUNGRY		      (-12)
#define TROUBLE_STUNNED		      (-13)
#define TROUBLE_CONFUSED	      (-14)
#define TROUBLE_HALLUCINATION	  (-15)

/* We could force rehumanize of polyselfed people, but we can't tell
   unintentional shape changes from the other kind. Oh well.
   3.4.2: make an exception if polymorphed into a form which lacks
   hands; that's a case where the ramifications override this doubt.
 */

/* Return 0 if nothing particular seems wrong, positive numbers for
   serious trouble, and negative numbers for comparative annoyances. This
   returns the worst problem. There may be others, and the gods may fix
   more than one.

This could get as bizarre as noting surrounding opponents, (or hostile dogs),
but that's really hard.
 */

#define ugod_is_angry() (u.ualign.record < 0)
#define on_altar()	(IS_ALTAR(levl[u.ux][u.uy].typ) || goat_mouth_at(u.ux, u.uy))
#define on_shrine()	((levl[u.ux][u.uy].altarmask & AM_SHRINE) != 0)

STATIC_OVL int
in_trouble()
{
	struct obj *otmp;
	int i, j, count=0;

/* Borrowed from eat.c */

#define SATIATED	0
#define NOT_HUNGRY	1
#define HUNGRY		2
#define WEAK		3
#define FAINTING	4
#define FAINTED		5
#define STARVED		6

	/*
	 * major troubles
	 */
	if(Stoned) return(TROUBLE_STONED);
	if(Golded) return(TROUBLE_STONED);
	if(FrozenAir) return(TROUBLE_FROZEN_AIR);
	if(Slimed) return(TROUBLE_SLIMED);
	if(Strangled) return(TROUBLE_STRANGLED);
	if(u.utrap && u.utraptype == TT_LAVA) return(TROUBLE_LAVA);
	if(Sick) return(TROUBLE_SICK);
	if(u.uhs >= WEAK && !Race_if(PM_INCANTIFIER)) return(TROUBLE_STARVING);
	if (Upolyd ? (u.mh <= 5 || u.mh*7 <= u.mhmax) :
		(u.uhp <= 5 || u.uhp*7 <= u.uhpmax)) return TROUBLE_HIT;
	if(u.wimage >= 10 && on_altar()) return(TROUBLE_WIMAGE);
	if(u.umorgul && on_altar()) return(TROUBLE_MORGUL);
	if(u.uhpmod < -18 && on_altar()) return(TROUBLE_HPMOD);
	if(u.ulycn >= LOW_PM) return(TROUBLE_LYCANTHROPE);
	if(near_capacity() >= EXT_ENCUMBER && AMAX(A_STR)-ABASE(A_STR) > 3)
		return(TROUBLE_COLLAPSING);

	for (i= -1; i<=1; i++) for(j= -1; j<=1; j++) {
		if (!i && !j) continue;
		if (!isok(u.ux+i, u.uy+j) || IS_ROCK(levl[u.ux+i][u.uy+j].typ)
		    || (blocked_boulder(i,j) && !throws_rocks(youracedata) && !(u.sealsActive&SEAL_YMIR)))
			count++;
	}
	if (count == 8 && !Passes_walls)
		return(TROUBLE_STUCK_IN_WALL);

	if (stuck_ring(uleft, RIN_LEVITATION) ||
		stuck_ring(uright, RIN_LEVITATION))
		return(TROUBLE_CURSED_LEVITATION);
	if (nohands(youracedata) || !freehand()) {
	    /* for bag/box access [cf use_container()]...
	       make sure it's a case that we know how to handle;
	       otherwise "fix all troubles" would get stuck in a loop */
	    if (welded(uwep)) return TROUBLE_UNUSEABLE_HANDS;
	    if (Upolyd && nohands(youracedata) && (!Unchanging ||
		    ((otmp = unchanger()) != 0 && otmp->cursed)))
		return TROUBLE_UNUSEABLE_HANDS;
	}
	if(Role_if(PM_BARD) && welded(uwep))
		return TROUBLE_UNUSEABLE_HANDS;
	if(Blindfolded && ublindf->cursed) return(TROUBLE_CURSED_BLINDFOLD);

	/*
	 * minor troubles
	 */
	if(Punished) return(TROUBLE_PUNISHED);
	if (Cursed_obj(uarmg, GAUNTLETS_OF_FUMBLING) ||
		Cursed_obj(uarmf, FUMBLE_BOOTS))
	    return TROUBLE_FUMBLING;
	if (worst_cursed_item()) return TROUBLE_CURSED_ITEMS;
#ifdef STEED
	if (u.usteed) {	/* can't voluntarily dismount from a cursed saddle */
	    otmp = which_armor(u.usteed, W_SADDLE);
	    if (Cursed_obj(otmp, SADDLE)) return TROUBLE_SADDLE;
	}
#endif

	if (u.ucarinc < 0) return(TROUBLE_CARRY_CURSED);
	if (u.uhitinc < 0) return(TROUBLE_TOHIT_CURSED);
	if (u.uacinc < 0) return(TROUBLE_AC_CURSED);
	if (u.udaminc < 0) return(TROUBLE_DAMAGE_CURSED);
	if (Blinded > 1 && haseyes(youracedata)) return(TROUBLE_BLIND);
	for(i=0; i<A_MAX; i++)
	    if(ABASE(i) < AMAX(i)) return(TROUBLE_POISONED);
	if(Wounded_legs
#ifdef STEED
		    && !u.usteed
#endif
				) return (TROUBLE_WOUNDED_LEGS);
	if(u.uhs >= HUNGRY && !Race_if(PM_INCANTIFIER)) return(TROUBLE_HUNGRY);
	if(HStun) return (TROUBLE_STUNNED);
	if(HConfusion) return (TROUBLE_CONFUSED);
	if(Hallucination) return(TROUBLE_HALLUCINATION);
	return(0);
}

/* select an item for TROUBLE_CURSED_ITEMS */
STATIC_OVL struct obj *
worst_cursed_item()
{
    register struct obj *otmp;

    /* if strained or worse, check for loadstone first */
    if (near_capacity() >= HVY_ENCUMBER) {
	for (otmp = invent; otmp; otmp = otmp->nobj)
	    if (Cursed_obj(otmp, LOADSTONE)) return otmp;
    }
    /* weapon takes precedence if it is interfering
       with taking off a ring or putting on a shield */
    if (welded(uwep) && (uright || bimanual(uwep,youracedata))) {	/* weapon */
	otmp = uwep;
    /* gloves come next, due to rings */
    } else if (uarmg && uarmg->cursed) {		/* gloves */
	otmp = uarmg;
    /* then shield due to two handed weapons and spells */
    } else if (uarms && uarms->cursed) {		/* shield */
	otmp = uarms;
    /* then cloak due to body armor */
    } else if (uarmc && uarmc->cursed) {		/* cloak */
	otmp = uarmc;
    } else if (uarm && uarm->cursed) {			/* suit */
	otmp = uarm;
    } else if (uarmh && uarmh->cursed) {		/* helmet */
	otmp = uarmh;
    } else if (uarmf && uarmf->cursed) {		/* boots */
	otmp = uarmf;
#ifdef TOURIST
    } else if (uarmu && uarmu->cursed) {		/* shirt */
	otmp = uarmu;
#endif
    } else if (uamul && uamul->cursed) {		/* amulet */
	otmp = uamul;
    } else if (uleft && uleft->cursed) {		/* left ring */
	otmp = uleft;
    } else if (uright && uright->cursed) {		/* right ring */
	otmp = uright;
    } else if (ublindf && ublindf->cursed) {		/* eyewear */
	otmp = ublindf;	/* must be non-blinding lenses */
    /* if weapon wasn't handled above, do it now */
    } else if (welded(uwep)) {				/* weapon */
	otmp = uwep;
    /* active secondary weapon even though it isn't welded */
    } else if (uswapwep && uswapwep->cursed && u.twoweap) {
	otmp = uswapwep;
    /* all worn items ought to be handled by now */
    } else {
	for (otmp = invent; otmp; otmp = otmp->nobj) {
	    if (!otmp->cursed) continue;
	    if (otmp->otyp == LOADSTONE || confers_luck(otmp))
		break;
	}
    }
    return otmp;
}

STATIC_OVL void
fix_worst_trouble(trouble)
register int trouble;
{
	int i;
	struct obj *otmp = 0;
	const char *what = (const char *)0;
	static NEARDATA const char leftglow[] = "left ring softly glows",
				   rightglow[] = "right ring softly glows";

	switch (trouble) {
	    case TROUBLE_STONED:
		    You_feel("more limber.");
		    Stoned = 0;
		    Golded = 0;
		    flags.botl = 1;
		    delayed_killer = 0;
		    break;
	    case TROUBLE_SLIMED:
		    pline_The("slime disappears.");
		    Slimed = 0;
		    flags.botl = 1;
		    delayed_killer = 0;
		    break;
	    case TROUBLE_FROZEN_AIR:
		    pline_The("frozen air vaporizes.");
		    FrozenAir = 0;
		    flags.botl = 1;
		    delayed_killer = 0;
		    break;
	    case TROUBLE_STRANGLED:
		    if (uamul && uamul->otyp == AMULET_OF_STRANGULATION) {
				Your("amulet vanishes!");
				useup(uamul);
		    }
			if(HStrangled){
				HStrangled = 0;
				Your("throat opens up!");
			}
			if(!Strangled) You("can breathe again.");
		    flags.botl = 1;
		    break;
	    case TROUBLE_LAVA:
		    You("are back on solid ground.");
		    /* teleport should always succeed, but if not,
		     * just untrap them.
		     */
		    if(!safe_teleds(FALSE))
			u.utrap = 0;
		    break;
	    case TROUBLE_STARVING:
		    losestr(-1);
		    /* fall into... */
	    case TROUBLE_HUNGRY:
		    Race_if(PM_CLOCKWORK_AUTOMATON) ? 
				Your("mainspring is miraculously wound!") :
				Your("%s feels content.", body_part(STOMACH));
		    reset_uhunger();
		    flags.botl = 1;
		    break;
	    case TROUBLE_SICK:
		    You_feel("better.");
		    make_sick(0L, (char *) 0, FALSE, SICK_ALL);
		    break;
	    case TROUBLE_HIT:
		    /* "fix all troubles" will keep trying if hero has
		       5 or less hit points, so make sure they're always
		       boosted to be more than that */
		    You_feel("much better.");
		    if (Upolyd) {
				if (u.mhmax <= 5){
					u.uhpmod = 5+1;
					calc_total_maxhp();
				}
			u.mh = u.mhmax;
		    }
		    else if (u.uhpmax < u.ulevel * 5 + 11){
				u.uhpmod += rnd(5);
				calc_total_maxhp();
			}
			if(u.uhpmax < 6){
				if(u.uhprolled < 6)
					u.uhprolled = 6;	/* arbitrary */
				calc_total_maxhp();
				if(u.uhpmax < 6 && u.uhpmod < 0){
					u.uhpmod = 0;	/* arbitrary */
					calc_total_maxhp();
				}
				if(u.uhpmax < 6 && u.uhpbonus < 0){
					u.uhpbonus = 0;	/* arbitrary */
					calc_total_maxhp();
				}
			}
		    u.uhp = u.uhpmax;
		    flags.botl = 1;
		    break;
	    case TROUBLE_COLLAPSING:
		    ABASE(A_STR) = AMAX(A_STR);
		    flags.botl = 1;
		    break;
	    case TROUBLE_STUCK_IN_WALL:
		    Your("surroundings change.");
		    /* no control, but works on no-teleport levels */
		    (void) safe_teleds(FALSE);
		    break;
	    case TROUBLE_CURSED_LEVITATION:
		    if ((otmp = stuck_ring(uleft,RIN_LEVITATION)) !=0) {
				if (otmp == uleft) what = leftglow;
		    } else if ((otmp = stuck_ring(uright,RIN_LEVITATION))!=0) {
				if (otmp == uright) what = rightglow;
		    }
		    goto decurse;
	    case TROUBLE_UNUSEABLE_HANDS:
			if(uarm && uarm->otyp == STRAITJACKET && uarm->cursed){
				otmp = uarm;
				goto decurse;
			}
		    else if (welded(uwep)) {
				otmp = uwep;
				goto decurse;
		    }
		    if (Upolyd && nohands(youracedata)) {
			if (!Unchanging) {
			    Your("shape becomes uncertain.");
			    rehumanize();  /* "You return to {normal} form." */
			} else if ((otmp = unchanger()) != 0 && otmp->cursed) {
			    /* otmp is an amulet of unchanging */
			    goto decurse;
			}
		    }
		    if (nohands(youracedata) || !freehand())
			impossible("fix_worst_trouble: couldn't cure hands.");
		    break;
	    case TROUBLE_CURSED_BLINDFOLD:
		    otmp = ublindf;
		    goto decurse;
	    case TROUBLE_WIMAGE:
			pline("The image of the weeping angel fades from your mind.");
		    u.wimage = 0;
		    break;
	    case TROUBLE_MORGUL:
			pline("The chill of death fades away.");
			//Destroy the blades.
		    u.umorgul = 0;
		    break;
	    case TROUBLE_HPMOD:
			You_feel("restored to health.");
		    u.uhpmod = max(u.uhpmod, 0);
		    calc_total_maxhp();
		    break;
	    case TROUBLE_LYCANTHROPE:
		    you_unwere(TRUE);
		    break;
	/*
	 */
	    case TROUBLE_PUNISHED:
		    Your("chain disappears.");
		    unpunish();
		    break;
	    case TROUBLE_FUMBLING:
		    if (Cursed_obj(uarmg, GAUNTLETS_OF_FUMBLING))
			otmp = uarmg;
		    else if (Cursed_obj(uarmf, FUMBLE_BOOTS))
			otmp = uarmf;
		    goto decurse;
		    /*NOTREACHED*/
		    break;
	    case TROUBLE_CURSED_ITEMS:
		    otmp = worst_cursed_item();
		    if (otmp == uright) what = rightglow;
		    else if (otmp == uleft) what = leftglow;
decurse:
		    if (!otmp) {
			impossible("fix_worst_trouble: nothing to uncurse.");
			return;
		    }
		    uncurse(otmp);
		    if (!Blind) {
			Your("%s %s.", what ? what :
				(const char *)aobjnam(otmp, "softly glow"),
			     hcolor(NH_AMBER));
			otmp->bknown = TRUE;
		    }
		    update_inventory();
		    break;
	    case TROUBLE_POISONED:
		    if (Hallucination)
			pline("There's a tiger in your tank.");
		    else
			You_feel("in good health again.");
		    for(i=0; i<A_MAX; i++) {
			if(ABASE(i) < AMAX(i)) {
				ABASE(i) = AMAX(i);
				flags.botl = 1;
			}
		    }
		    (void) encumber_msg();
		    break;
	    case TROUBLE_CARRY_CURSED:
			Your("pack feels lighter.");
		    u.ucarinc = 0;
			inv_weight();
	    break;
	    case TROUBLE_TOHIT_CURSED:
			Your("curse is lifted.");
		    u.uhitinc = 0;
	    break;
	    case TROUBLE_AC_CURSED:
			Your("curse is lifted.");
		    u.uacinc = 0;
	    break;
	    case TROUBLE_DAMAGE_CURSED:
			Your("curse is lifted.");
		    u.udaminc = 0;
	    break;
	    case TROUBLE_BLIND:
		{
		    int num_eyes = eyecount(youracedata);
		    const char *eye = body_part(EYE);

		    Your("%s feel%s better.",
			 (num_eyes == 1) ? eye : makeplural(eye),
			 (num_eyes == 1) ? "s" : "");
		    u.ucreamed = 0;
		    make_blinded(0L,FALSE);
		    break;
		}
	    case TROUBLE_WOUNDED_LEGS:
		    heal_legs();
		    break;
	    case TROUBLE_STUNNED:
		    make_stunned(0L,TRUE);
		    break;
	    case TROUBLE_CONFUSED:
		    make_confused(0L,TRUE);
		    break;
	    case TROUBLE_HALLUCINATION:
		    pline ("Looks like you are back in Kansas.");
		    (void) make_hallucinated(0L,FALSE,0L);
		    break;
#ifdef STEED
	    case TROUBLE_SADDLE:
		    otmp = which_armor(u.usteed, W_SADDLE);
		    uncurse(otmp);
		    if (!Blind) {
			pline("%s %s %s.",
			      s_suffix(upstart(y_monnam(u.usteed))),
			      aobjnam(otmp, "softly glow"),
			      hcolor(NH_AMBER));
			otmp->bknown = TRUE;
		    }
		    break;
#endif
	}
}

/* "I am sometimes shocked by...  the nuns who never take a bath without
 * wearing a bathrobe all the time.  When asked why, since no man can see them,
 * they reply 'Oh, but you forget the good God'.  Apparently they conceive of
 * the Deity as a Peeping Tom, whose omnipotence enables Him to see through
 * bathroom walls, but who is foiled by bathrobes." --Bertrand Russell, 1943
 * Divine wrath, dungeon walls, and armor follow the same principle.
 */
STATIC_OVL void
god_zaps_you(resp_god)
aligntyp resp_god;
{
	if (u.uswallow) {
	    pline("Suddenly a bolt of lightning comes down at you from the heavens!");
	    pline("It strikes %s!", mon_nam(u.ustuck));
	    if (!resists_elec(u.ustuck)) {
		pline("%s fries to a crisp!", Monnam(u.ustuck));
		/* Yup, you get experience.  It takes guts to successfully
		 * pull off this trick on your god, anyway.
		 */
		xkilled(u.ustuck, 0);
	    } else
		pline("%s seems unaffected.", Monnam(u.ustuck));
	} else {
	    pline("Suddenly, a bolt of lightning strikes you!");
	    if (Reflecting) {
		shieldeff(u.ux, u.uy);
		if (Blind)
		    pline("For some reason you're unaffected.");
		else
		    (void) ureflects("%s reflects from your %s.", "It");
	    } else if (Shock_resistance) {
		shieldeff(u.ux, u.uy);
		pline("It seems not to affect you.");
	    } else
		fry_by_god(resp_god);
	}

	pline("%s is not deterred...", align_gname(resp_god));
	if (u.uswallow) {
	    pline("A wide-angle disintegration beam aimed at you hits %s!",
			mon_nam(u.ustuck));
	    if (!resists_disint(u.ustuck)) {
		pline("%s fries to a crisp!", Monnam(u.ustuck));
		xkilled(u.ustuck, 2); /* no corpse */
	    } else
		pline("%s seems unaffected.", Monnam(u.ustuck));
	} else {
	    pline("A wide-angle disintegration beam hits you!");

	    /* disintegrate shield and body armor before disintegrating
	     * the impudent mortal, like black dragon breath -3.
	     */
	    if (uarms && !(EReflecting & W_ARMS) &&
	    		!(EDisint_resistance & W_ARMS))
		(void) destroy_arm(uarms);
	    if (uarmc && !(EReflecting & W_ARMC) &&
	    		!(EDisint_resistance & W_ARMC))
		(void) destroy_arm(uarmc);
	    if (uarm && !(EReflecting & W_ARM) &&
	    		!(EDisint_resistance & W_ARM) && !uarmc)
		(void) destroy_arm(uarm);
#ifdef TOURIST
	    if (uarmu && !uarm && !uarmc) (void) destroy_arm(uarmu);
#endif
	    if (!Disint_resistance)
		fry_by_god(resp_god);
	    else {
		You("bask in its %s glow for a minute...", NH_BLACK);
		godvoice(resp_god, "I believe it not!");
	    }
	    if (Is_astralevel(&u.uz) || Is_sanctum(&u.uz)) {
		/* one more try for high altars */
		verbalize("Thou cannot escape my wrath, mortal!");
		(void) summon_god_minion(align_gname_full(resp_god), resp_god, FALSE);
		(void) summon_god_minion(align_gname_full(resp_god), resp_god, FALSE);
		(void) summon_god_minion(align_gname_full(resp_god), resp_god, FALSE);
		(void) summon_god_minion(align_gname_full(resp_god), resp_god, FALSE);
		(void) summon_god_minion(align_gname_full(resp_god), resp_god, FALSE);
		verbalize("Destroy %s, my servants!", uhim());
	    }
	}
}

STATIC_OVL void
fry_by_god(resp_god)
aligntyp resp_god;
{
	char killerbuf[64];

	You("fry to a crisp.");
	killer_format = KILLED_BY;
	Sprintf(killerbuf, "the wrath of %s", align_gname(resp_god));
	killer = killerbuf;
	done(DIED);
}

void
angrygods(resp_god)
aligntyp resp_god;
{
	register int	maxanger;

	if(resp_god == A_VOID) return;
	
	if(Inhell && !(Race_if(PM_DROW) && (resp_god != A_LAWFUL || !flags.initgend))) resp_god = A_NONE;
	
	u.ublessed = 0;
	
	if(u.ualign.type == resp_god){
		u.lastprayed = moves;
		u.lastprayresult = PRAY_ANGER;
		u.reconciled = REC_NONE;
	}
	/* changed from tmp = u.ugangr + abs (u.uluck) -- rph */
	/* added test for alignment diff -dlc */
	if(resp_god != u.ualign.type)
	    maxanger =  3*u.ugangr[Align2gangr(resp_god)] +
		((Luck > 0 || u.ualign.record <= STRAYED) ? -Luck/3 : -Luck);
	else
	    maxanger =  3*u.ugangr[Align2gangr(resp_god)] +
		((Luck > 0 || u.ualign.record >= STRIDENT) ? -Luck/3 : -Luck);
	if (maxanger < 1) maxanger = 1; /* possible if bad align & good luck */
	else if (maxanger > 15) maxanger = 15;	/* be reasonable */
	
	if(Align2gangr(resp_god) == GA_MOTHER){
		struct monst *mtmp;
		for(mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon){
			if(mtmp->mux == u.uz.dnum && mtmp->muy == u.uz.dlevel && (mtmp->data == &mons[PM_BLESSED] || mtmp->data == &mons[PM_MOUTH_OF_THE_GOAT])){
				mtmp->mpeaceful = 0;
				mtmp->mtame = 0;
				set_malign(mtmp);
			}
		}
		u.ugoatblesscnt = rnz(300);
	}
	
	switch (rn2(maxanger)) {
	    case 0:
	    case 1:	You_feel("that %s is %s.", align_gname(resp_god),
			    Hallucination ? "bummed" : "displeased");
			break;
	    case 2:
	    case 3:
			godvoice(resp_god,(char *)0);
			pline("\"Thou %s, %s.\"",
			    (ugod_is_angry() && resp_god == u.ualign.type)
				? "hast strayed from the path" :
						"art arrogant",
			      youracedata->mlet == S_HUMAN ? "mortal" : "creature");
			verbalize("Thou must relearn thy lessons!");
			(void) adjattrib(A_WIS, -1, FALSE);
			losexp((char *)0,TRUE,FALSE,TRUE);
			break;
	    case 6:	if (!Punished) {
			    gods_angry(resp_god);
			    punish((struct obj *)0);
			    break;
			} /* else fall thru */
	    case 4:
	    case 5:	gods_angry(resp_god);
			if (!Blind && !Antimagic)
			    pline("%s glow surrounds you.",
				  An(hcolor(NH_BLACK)));
			rndcurse();
			break;
	    case 7:
	    case 8:	godvoice(resp_god,(char *)0);
			verbalize("Thou durst %s me?",
				  (on_altar() &&
				   (a_align(u.ux,u.uy) != resp_god)) ?
				  "scorn":"call upon");
			pline("\"Then die, %s!\"",
			      youracedata->mlet == S_HUMAN ? "mortal" : "creature");
			(void) summon_god_minion(align_gname_full(resp_god), resp_god, FALSE);
			break;

	    default:	gods_angry(resp_god);
			god_zaps_you(resp_god);
			break;
	}
	u.ublesscnt = rnz(300);
	return;
}

/* helper to print "str appears at your feet", or appropriate */
static void
at_your_feet(str)
	const char *str;
{
	if (Blind) str = Something;
	if (u.uswallow) {
	    /* barrier between you and the floor */
	    pline("%s %s into %s %s.", str, vtense(str, "drop"),
		  s_suffix(mon_nam(u.ustuck)), mbodypart(u.ustuck, STOMACH));
	} else {
	    pline("%s %s %s your %s!", str,
		  Blind ? "lands" : vtense(str, "appear"),
		  Levitation ? "beneath" : "at",
		  makeplural(body_part(FOOT)));
	}
}

//ifdef ELBERETH
STATIC_OVL void
gcrownu()
{
    struct obj *obj;
    boolean already_exists, in_hand;
    short class_gift;
    int sp_no;
#define ok_wep(o) ((o) && ((o)->oclass == WEAPON_CLASS || is_weptool(o)))
#define ok_arm(o) ((o) && ((o)->oclass == ARMOR_CLASS))
	
	if(!Role_if(PM_EXILE)){
		HSee_invisible |= FROMOUTSIDE;
		HFire_resistance |= FROMOUTSIDE;
		HCold_resistance |= FROMOUTSIDE;
		HShock_resistance |= FROMOUTSIDE;
		HSleep_resistance |= FROMOUTSIDE;
		HPoison_resistance |= FROMOUTSIDE;
		u.wardsknown |= WARD_HEPTAGRAM;
		godvoice(u.ualign.type, (char *)0);
	}

    obj = ok_wep(uwep) ? uwep : 0;
    already_exists = in_hand = FALSE;	/* lint suppression */
	if( Pantheon_if(PM_PIRATE) || Role_if(PM_PIRATE) ){
		u.uevent.uhand_of_elbereth = 2; /* Alignment of P King is treated as neutral */
		in_hand = (uwep && uwep->oartifact == ART_REAVER);
		already_exists = exist_artifact(SCIMITAR, artiname(ART_REAVER));
		verbalize("Hurrah for our Pirate King!");
		livelog_write_string("became the Pirate King");
	} else if((Pantheon_if(PM_VALKYRIE) || Role_if(PM_VALKYRIE)) && flags.initgend){
		u.uevent.uhand_of_elbereth = 2; /* Alignment of Skadi is treated as neutral */
		in_hand = FALSE;
		already_exists = exist_artifact(BOW, artiname(ART_BOW_OF_SKADI));
		verbalize("I greet you, my daughter.");
		livelog_write_string("greeted as a daughter of Skadi");
	} else if(Race_if(PM_DWARF) && (urole.ldrnum == PM_THORIN_II_OAKENSHIELD || urole.ldrnum == PM_DAIN_II_IRONFOOT)){
		u.uevent.uhand_of_elbereth = 1; /* Alignment of Dwarf king is treated as lawful */
		if(urole.ldrnum == PM_THORIN_II_OAKENSHIELD){
			in_hand = FALSE;
			already_exists = exist_artifact(DIAMOND, artiname(ART_ARKENSTONE));
			verbalize("Hail, King under the Mountain!");
		} else if(urole.ldrnum == PM_DAIN_II_IRONFOOT){
			in_hand = FALSE;
			already_exists = exist_artifact(AXE, artiname(ART_DURIN_S_AXE));
			verbalize("Hail, Lord of Moria!");
		}
	} else if(Role_if(PM_EXILE)){
		u.uevent.uhand_of_elbereth = 2; /* Alignment of emissary is treated as neutral */
		You("suddenly perceive 15 pairs of star-like eyes, staring at you from within your head.");
		pline("<<We are the Council of Elements>>");
		pline("<<Guardians of the Material world>>");
		pline("<<You who straddle the line between our world and the void beyond,");
		pline("  you shall be our emissary to that which gave rise to us all>>");
		bindspirit(COUNCIL);
		livelog_write_string("became the Emissary of Elements");
		return;
	} else {
    switch (u.ualign.type) {
    case A_LAWFUL:
	if(Race_if(PM_DROW)){
		if(Role_if(PM_NOBLEMAN)){
			u.uevent.uhand_of_elbereth = 22;
			in_hand = FALSE;
			already_exists = exist_artifact(DROVEN_CROSSBOW, artiname(ART_LIECLEAVER));
			verbalize("I dub thee...  The Blade of Ver'tas!");
			livelog_write_string("became the Blade of Ver'tas");
		} else {
			if(flags.initgend){ /*Female*/
				u.uevent.uhand_of_elbereth = 16;
				in_hand = FALSE;
				already_exists = exist_artifact(SICKLE, artiname(ART_SICKLE_MOON));
				verbalize("I dub thee...  The Hand of Eilistraee!");
				livelog_write_string("became the Hand of Eilistraee");
			} else { /*male*/
				in_hand = FALSE;
				u.uevent.uhand_of_elbereth = 19;
				verbalize("As shadows define the light, so too do webs define the spider.");
				verbalize("You shall be my shepherd, to wrap the world in webs of shadow!");
				livelog_write_string("became the Shepherd of the Black Web");
				u.specialSealsKnown |= SEAL_BLACK_WEB;
				return;
			}
		}
	} else if(Race_if(PM_ELF)){
		u.uevent.uhand_of_elbereth = 13;
		in_hand = FALSE;
		already_exists = exist_artifact(LONG_SWORD, artiname(ART_ARCOR_KERYM));
		verbalize("I crown thee...  The Hand of Elbereth!");
		livelog_write_string("became the Hand of Elbereth");
	} else if((Race_if(PM_HUMAN) || Race_if(PM_INHERITOR) || Race_if(PM_INCANTIFIER) || Race_if(PM_HALF_DRAGON))
		&& (Pantheon_if(PM_RANGER) || Role_if(PM_RANGER))
	) {
		u.uevent.uhand_of_elbereth = 28;
		in_hand = FALSE;
		already_exists = exist_artifact(GOLDEN_ARROW, artiname(ART_SUNBEAM));
		verbalize("I anoint thee...  High %s of Apollo!", flags.female ? "Priestess" : "Priest");
		livelog_write_string("anointed by Apollo");
	} else if(Race_if(PM_GNOME) && Role_if(PM_RANGER)) {
		u.uevent.uhand_of_elbereth = 31;
		in_hand = FALSE;
		already_exists = exist_artifact(GRAY_DRAGON_SCALES, artiname(ART_STEEL_SCALES_OF_KURTULMAK));
		verbalize("I claim thee...  Great Slave-Vassal of Kurtulmak!");
		livelog_write_string("claimed by Kurtulmak");
	} else if(Pantheon_if(PM_KNIGHT) || Role_if(PM_KNIGHT)){
		u.uevent.uhand_of_elbereth = 1;
		in_hand = FALSE;
		already_exists = exist_artifact(LONG_SWORD, artiname(ART_CLARENT));
		verbalize("I crown thee...  King of the Angles!");
		livelog_write_string("crowned King of the Angles");
	} else if(Pantheon_if(PM_HEALER) || Role_if(PM_HEALER)){
		u.uevent.uhand_of_elbereth = 34;
		in_hand = FALSE;
		already_exists = exist_artifact(ROUNDSHIELD, artiname(ART_AEGIS));
		verbalize("I dub thee...  The Arm of Athena!");
		livelog_write_string("became the Arm of Athena");
	} else if(Pantheon_if(PM_MONK) || Role_if(PM_MONK)){
		u.uevent.uhand_of_elbereth = 4;
		in_hand = FALSE;
		already_exists = exist_artifact(ROBE, artiname(ART_GRANDMASTER_S_ROBE));
		verbalize("I dub thee...  The Sage of Law!");
		livelog_write_string("became the Sage of Law");
	} else if(Pantheon_if(PM_WIZARD) || Role_if(PM_WIZARD)){
		u.uevent.uhand_of_elbereth = 10;
		in_hand = FALSE;
		already_exists = exist_artifact(SPE_SECRETS, artiname(ART_NECRONOMICON));
		if(already_exists) already_exists = exist_artifact(SPE_SECRETS, artiname(ART_BOOK_OF_INFINITE_SPELLS));
		verbalize("I dub thee...  The Magister of Law!");
		livelog_write_string("became the Magister of Law");
	} else if(Pantheon_if(PM_NOBLEMAN) || Role_if(PM_NOBLEMAN)){
		in_hand = FALSE;
		if(Race_if(PM_VAMPIRE)) already_exists = exist_artifact(find_vhelm(), artiname(ART_HELM_OF_THE_DARK_LORD));
		else already_exists = exist_artifact(find_gcirclet(), artiname(ART_CROWN_OF_THE_SAINT_KING));
		if(Race_if(PM_VAMPIRE)){
			u.uevent.uhand_of_elbereth = 9;
			verbalize("I crown thee...  The Dark %s!", flags.female ? "Lady" : "Lord");
			livelog_write_string("recieved the helm of the Dark Lord");
		} else {
			u.uevent.uhand_of_elbereth = 7;
			verbalize("I crown thee...  The Saint %s!", flags.female ? "Queen" : "King");
			livelog_write_string("recieved the crown of the Saint King");
		}
	} else if(Pantheon_if(PM_ARCHEOLOGIST) || Role_if(PM_ARCHEOLOGIST)){
		u.uevent.uhand_of_elbereth = 37;
		in_hand = FALSE;
		already_exists = exist_artifact(AMULET_OF_MAGICAL_BREATHING, artiname(ART_EHECAILACOCOZCATL));
		verbalize("I proclaim thee...  The High Priest of Quetzalcoatl!");
	} else if(Pantheon_if(PM_SAMURAI) || Role_if(PM_SAMURAI)){
		char crown_msg[BUFSZ];
		u.uevent.uhand_of_elbereth = 1; /* Alignment of Nasu clan is treated as lawful */
		in_hand = FALSE;
		already_exists = exist_artifact(YUMI, artiname(ART_YOICHI_NO_YUMI));
		strcpy(crown_msg, "I proclame thee...  Nasu no ");
		strcat(crown_msg, plname);
		strcat(crown_msg, "!");
		verbalize1(crown_msg);
		strcpy(crown_msg, "became the Nasu no ");
		strcat(crown_msg, plname);
		strcat(crown_msg, "!");
		livelog_write_string(crown_msg);
	} else {
		u.uevent.uhand_of_elbereth = 1;
		verbalize("I dub thee...  The Arm of the Law!");
		livelog_write_string("became the Arm of the Law");
	}
	break;
    case A_NEUTRAL:
	if(Race_if(PM_DROW)){
		if(Role_if(PM_NOBLEMAN)){
			if(flags.initgend){ /*Female*/
				u.uevent.uhand_of_elbereth = 23;
				in_hand = FALSE;
				already_exists = exist_artifact(GAUNTLETS_OF_DEXTERITY, artiname(ART_CLAWS_OF_THE_REVENANCER));
				verbalize("I dub thee...  The Hand of Kiaransali!");
				livelog_write_string("became the Hand of Kiaransali");
			} else { /*male*/
				u.uevent.uhand_of_elbereth = 26;
				in_hand = FALSE;
				already_exists = exist_artifact(DROVEN_SHORT_SWORD, artiname(ART_LOLTH_S_FANG));
				verbalize("I dub thee...  The Hand of Keptolo!");
				livelog_write_string("became the Hand of Keptolo");
			}
		} else {
			if(flags.initgend){ /*Female*/
				u.uevent.uhand_of_elbereth = 17;
				in_hand = FALSE;
				already_exists = exist_artifact(GAUNTLETS_OF_DEXTERITY, artiname(ART_CLAWS_OF_THE_REVENANCER));
				verbalize("I dub thee...  The Hand of Kiaransali!");
				livelog_write_string("became the Hand of Kiaransali");
			} else { /*male*/
				u.uevent.uhand_of_elbereth = 20;
				in_hand = FALSE;
				already_exists = exist_artifact(DROVEN_SHORT_SWORD, artiname(ART_LOLTH_S_FANG));
				verbalize("I dub thee...  The Sword of Vhaeraun!");
				livelog_write_string("became the Sword of Vhaeraun");
			}
		}
	} else if(Race_if(PM_ELF)){
		u.uevent.uhand_of_elbereth = 14;
		in_hand = FALSE;
		already_exists = exist_artifact(RUNESWORD, artiname(ART_ARYFAERN_KERYM));
		verbalize("I dub thee...  The Doomspeaker of Vaire!");
		livelog_write_string("became the Doomspeaker of Vaire");
	} else if((Race_if(PM_HUMAN) || Race_if(PM_INHERITOR) || Race_if(PM_INCANTIFIER) || Race_if(PM_HALF_DRAGON))
		&& (Pantheon_if(PM_RANGER) || Role_if(PM_RANGER))
	) {
		u.uevent.uhand_of_elbereth = 29;
		in_hand = FALSE;
		already_exists = exist_artifact(CLOAK_OF_INVISIBILITY, artiname(ART_VEIL_OF_LATONA));
		verbalize("I anoint thee...  High %s of Latona!", flags.female ? "Priestess" : "Priest");
		livelog_write_string("anointed by Latona");
	} else if(Race_if(PM_GNOME) && Role_if(PM_RANGER)) {
		u.uevent.uhand_of_elbereth = 32;
		in_hand = FALSE;
		already_exists = exist_artifact(AMBER, artiname(ART_GLITTERSTONE));
		verbalize("I dub thee...  Thane of Garl Glittergold!");
		livelog_write_string("became the Thane of Garl Glittergold");
	} else if(Pantheon_if(PM_HEALER) || Role_if(PM_HEALER)){
		u.uevent.uhand_of_elbereth = 35;
		in_hand = FALSE;
		already_exists = exist_artifact(FLYING_BOOTS, artiname(ART_HERMES_S_SANDALS));
		verbalize("I dub thee... Messenger of Hermes!");
		livelog_write_string("became the Messenger of Hermes");
	} else if(Pantheon_if(PM_MONK) || Role_if(PM_MONK)){
		u.uevent.uhand_of_elbereth = 5;
		in_hand = FALSE;
		already_exists = exist_artifact(ROBE, artiname(ART_GRANDMASTER_S_ROBE));
		verbalize("Thou shalt be the Grandmaster of Balance!");
		livelog_write_string("became the Grandmaster of Balance");
	} else if(Pantheon_if(PM_WIZARD) || Role_if(PM_WIZARD)){
		u.uevent.uhand_of_elbereth = 11;
		in_hand = FALSE;
		already_exists = exist_artifact(SPE_SECRETS, artiname(ART_NECRONOMICON));
		if(already_exists) already_exists = exist_artifact(SPE_SECRETS, artiname(ART_BOOK_OF_INFINITE_SPELLS));
		verbalize("Thou shalt be the Wizard of Balance!");
		livelog_write_string("became the Wizard of Balance");
	} else if(Pantheon_if(PM_NOBLEMAN) || Role_if(PM_NOBLEMAN)){
		in_hand = FALSE;
		if(Race_if(PM_VAMPIRE)) already_exists = exist_artifact(find_vhelm(), artiname(ART_HELM_OF_THE_DARK_LORD));
		else already_exists = exist_artifact(find_gcirclet(), artiname(ART_CROWN_OF_THE_SAINT_KING));
		if(Race_if(PM_VAMPIRE)){
			u.uevent.uhand_of_elbereth = 9;
			verbalize("I crown thee...  Dark %s!", flags.female ? "Lady" : "Lord");
			livelog_write_string("recieved the helm of the Dark Lord");
		} else {
			u.uevent.uhand_of_elbereth = 8;
			verbalize("I dub thee...  The Grey Saint!");
			livelog_write_string("recieved the crown of the Saint King");
		}
	} else if(Pantheon_if(PM_ARCHEOLOGIST) || Role_if(PM_ARCHEOLOGIST)){
		u.uevent.uhand_of_elbereth = 38;
		in_hand = FALSE;
		already_exists = exist_artifact(JAVELIN, artiname(ART_AMHIMITL));
		verbalize("I proclaim thee... The Champion of Camaxtli!");
		livelog_write_string("became the Champion of Camaxtli!");
	} else {
		u.uevent.uhand_of_elbereth = 2;
		in_hand = (uwep && uwep->oartifact == ART_VORPAL_BLADE);
		already_exists = exist_artifact(LONG_SWORD, artiname(ART_VORPAL_BLADE));
		verbalize("Thou shalt be my Envoy of Balance!");
		livelog_write_string("became the Envoy of Balance");
	}
	break;
    case A_CHAOTIC:
	if(Race_if(PM_DROW)){
		if(Role_if(PM_NOBLEMAN)){
			if(flags.initgend){ /*Female*/
				u.uevent.uhand_of_elbereth = 24;
				in_hand = FALSE;
				already_exists = exist_artifact(ELVEN_MITHRIL_COAT, artiname(ART_WEB_OF_LOLTH));
				verbalize("I crown thee...  The Hand of Lolth!");
				livelog_write_string("became the Hand of Lolth");
			} else { /*male*/
				u.uevent.uhand_of_elbereth = 27;
				in_hand = FALSE;
				already_exists = exist_artifact(MORNING_STAR, artiname(ART_RUINOUS_DESCENT_OF_STARS));
				verbalize("I dub thee...  The Hammer of Ghaunadaur!");
				livelog_write_string("became the Hammer of Ghaunadaur");
			}
		} else {
			if(flags.initgend){ /*Female*/
				u.uevent.uhand_of_elbereth = 18;
				in_hand = FALSE;
				already_exists = exist_artifact(ELVEN_MITHRIL_COAT, artiname(ART_WEB_OF_LOLTH));
				verbalize("I crown thee...  The Hand of Lolth!");
				livelog_write_string("became the Hand of Lolth");
			} else { /*male*/
				u.uevent.uhand_of_elbereth = 21;
				in_hand = FALSE;
				already_exists = exist_artifact(DROVEN_SHORT_SWORD, artiname(ART_LOLTH_S_FANG));
				verbalize("I dub thee...  The Fang of Lolth!");
				livelog_write_string("became the Fang of Lolth");
			}
		}
	} else if(Race_if(PM_ELF)){
		u.uevent.uhand_of_elbereth = 15;
		in_hand = FALSE;
		already_exists = exist_artifact(ELVEN_BROADSWORD, artiname(ART_ARYVELAHR_KERYM));
		verbalize("I dub thee...  The Whisperer of Este!");
		livelog_write_string("became the Whisperer of Este");
	} else if((Race_if(PM_HUMAN) || Race_if(PM_INHERITOR) || Race_if(PM_INCANTIFIER) || Race_if(PM_HALF_DRAGON))
		&& (Pantheon_if(PM_RANGER) || Role_if(PM_RANGER))
	) {
		u.uevent.uhand_of_elbereth = 30;
		in_hand = FALSE;
		already_exists = exist_artifact(SILVER_ARROW, artiname(ART_MOONBEAM));
		verbalize("I anoint thee...  High %s of Diana!", flags.female ? "Priestess" : "Priest");
		livelog_write_string("anointed by Diana");
	} else if(Race_if(PM_GNOME) && Role_if(PM_RANGER)) {
		u.uevent.uhand_of_elbereth = 33;
		in_hand = FALSE;
		already_exists = exist_artifact(GAUNTLETS_OF_POWER, artiname(ART_GREAT_CLAWS_OF_URDLEN));
		verbalize("Thou art chosen to rend the Earth in My Name!");
		livelog_write_string("chosen by Urdlen");
	} else if(Pantheon_if(PM_HEALER) || Role_if(PM_HEALER)){
		u.uevent.uhand_of_elbereth = 36;
		in_hand = FALSE;
		already_exists = exist_artifact(TRIDENT, artiname(ART_POSEIDON_S_TRIDENT));
		verbalize("I dub thee... Glory of Poseidon!");
		livelog_write_string("became the Glory of Poseidon");
	} else if(Pantheon_if(PM_MONK) || Role_if(PM_MONK)){
		u.uevent.uhand_of_elbereth = 6;
		in_hand = FALSE;
		already_exists = exist_artifact(ROBE, artiname(ART_ROBE_OF_THE_ARCHMAGI));
		verbalize("Thou art chosen to cause dismay in My Name!");
		livelog_write_string("became the Glory of Eequor");
	} else if(Pantheon_if(PM_WIZARD) || Role_if(PM_WIZARD)){
		u.uevent.uhand_of_elbereth = 12;
		in_hand = FALSE;
		already_exists = exist_artifact(SPE_SECRETS, artiname(ART_NECRONOMICON));
		if(already_exists) already_exists = exist_artifact(SPE_SECRETS, artiname(ART_BOOK_OF_INFINITE_SPELLS));
		verbalize("Thou art chosen to take lives for My Glory!");
		livelog_write_string("became the Glory of Chardros");
	} else if(Pantheon_if(PM_NOBLEMAN) || Role_if(PM_NOBLEMAN)){
		u.uevent.uhand_of_elbereth = 9;
		in_hand = FALSE;
		already_exists = exist_artifact(find_vhelm(), artiname(ART_HELM_OF_THE_DARK_LORD));
		verbalize("I crown thee...  Dark %s!", flags.female ? "Lady" : "Lord");
		livelog_write_string("recieved the helm of the Dark Lord");
	} else if(Pantheon_if(PM_ARCHEOLOGIST) || Role_if(PM_ARCHEOLOGIST)){
		u.uevent.uhand_of_elbereth = 39;
		in_hand = FALSE;
		already_exists = exist_artifact(ROUNDSHIELD, artiname(ART_TECPATL_OF_HUHETOTL));
		verbalize("I dub thee... The Fire-bearer of Huhetotl!");
		livelog_write_string("became the Fire-bearer of Huhetotl");
	} else {
		u.uevent.uhand_of_elbereth = 3;
		in_hand = (uwep && uwep->oartifact == ART_STORMBRINGER);
		already_exists = exist_artifact(RUNESWORD, artiname(ART_STORMBRINGER));
		verbalize("Thou art chosen to %s for My Glory!",
			  already_exists && !in_hand ? "take lives" : "steal souls");
		livelog_write_string("became the Glory of Arioch");
	}
	break;
    }
	}

    class_gift = STRANGE_OBJECT;
    /* 3.3.[01] had this in the A_NEUTRAL case below,
       preventing chaotic wizards from receiving a spellbook */
	if(Race_if(PM_DROW) && (Role_if(PM_NOBLEMAN) || Role_if(PM_RANGER) || Role_if(PM_ROGUE) || Role_if(PM_PRIEST) || Role_if(PM_WIZARD))){
		if(flags.initgend){ /*Female*/
			if (class_gift != STRANGE_OBJECT) {
				;		/* already got bonus above for some reason */
			} else if (!already_exists) {
				if(u.ualign.type == A_CHAOTIC){
					obj = mksobj(ELVEN_MITHRIL_COAT, FALSE, FALSE);
					obj = oname(obj, artiname(ART_WEB_OF_LOLTH));
					discover_artifact(ART_WEB_OF_LOLTH);
				} else if(u.ualign.type == A_NEUTRAL){
					obj = mksobj(GAUNTLETS_OF_DEXTERITY, FALSE, FALSE);
					obj = oname(obj, artiname(ART_CLAWS_OF_THE_REVENANCER));
					discover_artifact(ART_CLAWS_OF_THE_REVENANCER);
				} else {
					if(Role_if(PM_NOBLEMAN)){
						obj = mksobj(DROVEN_CROSSBOW, FALSE, FALSE);
						obj = oname(obj, artiname(ART_LIECLEAVER));
						discover_artifact(ART_LIECLEAVER);
					} else {
						obj = mksobj(SICKLE, FALSE, FALSE);
						obj = oname(obj, artiname(ART_SICKLE_MOON));
						discover_artifact(ART_SICKLE_MOON);
					}
				}
				obj->spe = 1;
				at_your_feet("An object");
				dropy(obj);
				u.ugifts++;
			}
			if(u.ualign.type == A_CHAOTIC){
				//unrestrict_weapon_skill();
			} else if(u.ualign.type == A_NEUTRAL){
				gm_weapon_skill(P_BARE_HANDED_COMBAT);
				u.umartial = TRUE;
			} else if(u.ualign.type == A_LAWFUL){
				if(Role_if(PM_NOBLEMAN)){
					expert_weapon_skill(P_CROSSBOW);
					expert_weapon_skill(P_SCIMITAR);
				}
				else expert_weapon_skill(P_HARVEST);
			}
		} else { /*male*/
			if (class_gift != STRANGE_OBJECT) {
				;		/* already got bonus above for some reason */
			}
			else if(Role_if(PM_NOBLEMAN)){
				if (!already_exists){
					if(u.ualign.type == A_CHAOTIC){
						obj = mksobj(MORNING_STAR, FALSE, FALSE);
						obj = oname(obj, artiname(ART_RUINOUS_DESCENT_OF_STARS));
						discover_artifact(ART_RUINOUS_DESCENT_OF_STARS);
					} else if(u.ualign.type == A_NEUTRAL){
						obj = mksobj(GAUNTLETS_OF_DEXTERITY, FALSE, FALSE);
						obj = oname(obj, artiname(ART_CLAWS_OF_THE_REVENANCER));
						discover_artifact(ART_CLAWS_OF_THE_REVENANCER);
					} else if(u.ualign.type == A_LAWFUL){
						obj = mksobj(DROVEN_CROSSBOW, FALSE, FALSE);
						obj = oname(obj, artiname(ART_LIECLEAVER));
						discover_artifact(ART_LIECLEAVER);
					}
				}
			} else if (!already_exists) {
				obj = mksobj(DROVEN_SHORT_SWORD, FALSE, FALSE);
				obj = oname(obj, artiname(ART_LOLTH_S_FANG));
				discover_artifact(ART_LOLTH_S_FANG);
				obj->spe = 1;
				at_your_feet("A sword");
				dropy(obj);
				u.ugifts++;
			}
			if(Role_if(PM_NOBLEMAN)){
				if(u.ualign.type == A_CHAOTIC){
					expert_weapon_skill(P_MORNING_STAR);
				} else if(u.ualign.type == A_NEUTRAL){
					expert_weapon_skill(P_SHORT_SWORD);
				} else if(u.ualign.type == A_LAWFUL){
					expert_weapon_skill(P_CROSSBOW);
				}
			} else {
				expert_weapon_skill(P_SHORT_SWORD);
			}
		}
	} else if (Race_if(PM_DWARF) && (urole.ldrnum == PM_THORIN_II_OAKENSHIELD || urole.ldrnum == PM_DAIN_II_IRONFOOT)) {
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (!already_exists) {
			if(urole.ldrnum == PM_THORIN_II_OAKENSHIELD){
				obj = mksobj(DIAMOND, FALSE, FALSE);
				obj = oname(obj, artiname(ART_ARKENSTONE));
				at_your_feet("A shining diamond");
				discover_artifact(ART_DURIN_S_AXE);
			} else if(urole.ldrnum == PM_DAIN_II_IRONFOOT){
				obj = mksobj(AXE, FALSE, FALSE);
				obj = oname(obj, artiname(ART_DURIN_S_AXE));
				obj->spe = 1;
				at_your_feet("A silver axe");
				expert_weapon_skill(P_AXE);
				discover_artifact(ART_DURIN_S_AXE);
			}
			dropy(obj);
			u.ugifts++;
		}
	} else if (Race_if(PM_ELF) && !Role_if(PM_PIRATE)) {
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (!already_exists) {
			if(u.ualign.type == A_CHAOTIC){
				obj = mksobj(CRYSTAL_SWORD, FALSE, FALSE);
				obj = oname(obj, artiname(ART_ARYVELAHR_KERYM));
			} else if(u.ualign.type == A_NEUTRAL){
				obj = mksobj(RUNESWORD, FALSE, FALSE);
				obj = oname(obj, artiname(ART_ARYFAERN_KERYM));
			} else {
				obj = mksobj(LONG_SWORD, FALSE, FALSE);
				obj = oname(obj, artiname(ART_ARCOR_KERYM));
			}
			obj->spe = 1;
			at_your_feet("An Elfblade of Cormanthyr");
			dropy(obj);
			u.ugifts++;
		}
		if (obj && obj->oartifact == ART_ARYVELAHR_KERYM)
			discover_artifact(ART_ARYVELAHR_KERYM);
		else if(obj && obj->oartifact == ART_ARYFAERN_KERYM)
			discover_artifact(ART_ARYFAERN_KERYM);
		else if(obj && obj->oartifact == ART_ARCOR_KERYM)
			discover_artifact(ART_ARCOR_KERYM);
		
		if(u.ualign.type == A_CHAOTIC){
			expert_weapon_skill(P_BROAD_SWORD);
		} else if(u.ualign.type == A_NEUTRAL){
			expert_weapon_skill(P_BROAD_SWORD);
		} else if(u.ualign.type == A_LAWFUL){
			expert_weapon_skill(P_LONG_SWORD);
		}
	} else if( Pantheon_if(PM_PIRATE) || Role_if(PM_PIRATE) ){
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (in_hand) {
			Your("%s rings with the sound of waves!", xname(obj));
			obj->dknown = TRUE;
		} else if (!already_exists) {
			obj = mksobj(SCIMITAR, FALSE, FALSE);
			obj = oname(obj, artiname(ART_REAVER));
			obj->spe = 1;
			at_your_feet("A sword");
			dropy(obj);
			u.ugifts++;
		}
		/* acquire Reaver's skill regardless of weapon or gift, 
			although pirates are already good at using scimitars */
		expert_weapon_skill(P_SCIMITAR);
		if (obj && obj->oartifact == ART_REAVER)
			discover_artifact(ART_REAVER);
	} else if ((Pantheon_if(PM_VALKYRIE) || Role_if(PM_VALKYRIE)) && flags.initgend) {
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (!already_exists) {
			obj = mksobj(BOW, FALSE, FALSE);
			obj = oname(obj, artiname(ART_BOW_OF_SKADI));
			obj->spe = 1;
			at_your_feet("A bow");
			dropy(obj);
			u.ugifts++;
		}
		expert_weapon_skill(P_BOW);
		if (obj && obj->oartifact == ART_BOW_OF_SKADI)
			discover_artifact(ART_BOW_OF_SKADI);
	} else if((Race_if(PM_HUMAN) || Race_if(PM_INHERITOR) || Race_if(PM_INCANTIFIER) || Race_if(PM_HALF_DRAGON))
		&& (Pantheon_if(PM_RANGER) || Role_if(PM_RANGER))
	){
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (!already_exists) {
			if(u.ualign.type == A_CHAOTIC){
				obj = mksobj(SILVER_ARROW, FALSE, FALSE);
				obj = oname(obj, artiname(ART_MOONBEAM));
				obj->quan = 20;
				obj->spe = 6;
				at_your_feet("Silver arrows");
				dropy(obj);
				discover_artifact(ART_MOONBEAM);
				expert_weapon_skill(P_BOW);
			} else if(u.ualign.type == A_NEUTRAL){
				obj = mksobj(CLOAK_OF_INVISIBILITY, FALSE, FALSE);
				obj = oname(obj, artiname(ART_VEIL_OF_LATONA));
				obj->spe = 7;
				at_your_feet("A cloak");
				dropy(obj);
				discover_artifact(ART_VEIL_OF_LATONA);
			} else if(u.ualign.type == A_LAWFUL){
				obj = mksobj(GOLDEN_ARROW, FALSE, FALSE);
				obj = oname(obj, artiname(ART_SUNBEAM));
				obj->quan = 20;
				obj->spe = 7;
				at_your_feet("Golden arrows");
				dropy(obj);
				discover_artifact(ART_SUNBEAM);
				expert_weapon_skill(P_BOW);
			}
			u.ugifts++;
		}
	} else if(Race_if(PM_GNOME) && Role_if(PM_RANGER)){
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (!already_exists) {
			if(u.ualign.type == A_CHAOTIC){
				obj = mksobj(GAUNTLETS_OF_POWER, FALSE, FALSE);
				obj = oname(obj, artiname(ART_GREAT_CLAWS_OF_URDLEN));
				obj->spe = 1;
				obj->objsize = MZ_SMALL;
				fix_object(obj);
				at_your_feet("Clawed gauntlets");
				dropy(obj);
				discover_artifact(ART_GREAT_CLAWS_OF_URDLEN);
				gm_weapon_skill(P_BARE_HANDED_COMBAT);
				u.umartial = TRUE;
			} else if(u.ualign.type == A_NEUTRAL){
				obj = mksobj(AMBER, FALSE, FALSE);
				obj = oname(obj, artiname(ART_GLITTERSTONE));
				at_your_feet("A glittering gemstone");
				dropy(obj);
				discover_artifact(ART_GLITTERSTONE);
				expert_weapon_skill(P_ENCHANTMENT_SPELL);
				expert_weapon_skill(P_HEALING_SPELL);
				expert_weapon_skill(P_ESCAPE_SPELL);
			} else if(u.ualign.type == A_LAWFUL){
				obj = mksobj(GRAY_DRAGON_SCALES, FALSE, FALSE);
				obj = oname(obj, artiname(ART_STEEL_SCALES_OF_KURTULMAK));
				at_your_feet("Steel scales");
				dropy(obj);
				discover_artifact(ART_STEEL_SCALES_OF_KURTULMAK);
				expert_weapon_skill(P_RIDING);
			}
			u.ugifts++;
		}
	} else if (Pantheon_if(PM_KNIGHT) || Role_if(PM_KNIGHT)) {
		if(!already_exists){
			obj = mksobj(LONG_SWORD, FALSE, FALSE);
			obj = oname(obj, artiname(ART_CLARENT));
			obj->spe = 1;
			at_your_feet("A sword");
			discover_artifact(ART_CLARENT);
			dropy(obj);
			u.ugifts++;
		}
		expert_weapon_skill(P_LONG_SWORD);
		expert_weapon_skill(P_BEAST_MASTERY);
	} else if(Pantheon_if(PM_HEALER) || Role_if(PM_HEALER)){
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (!already_exists) {
			if(u.ualign.type == A_CHAOTIC){
				obj = mksobj(TRIDENT, FALSE, FALSE);
				obj = oname(obj, artiname(ART_POSEIDON_S_TRIDENT));
				discover_artifact(ART_POSEIDON_S_TRIDENT);
				expert_weapon_skill(P_TRIDENT);
				HSwimming |= INTRINSIC;
				obj->spe = 3;
				at_your_feet("A trident");
			} else if(u.ualign.type == A_NEUTRAL) {
				obj = mksobj(FLYING_BOOTS, FALSE, FALSE);
				obj = oname(obj, artiname(ART_HERMES_S_SANDALS));
				discover_artifact(ART_HERMES_S_SANDALS);
				expert_weapon_skill(P_LONG_SWORD);
				obj->spe = 1;
				at_your_feet("A pair of golden shoes");
			} else {
				obj = mksobj(ROUNDSHIELD, FALSE, FALSE);
				obj = oname(obj, artiname(ART_AEGIS));
				discover_artifact(ART_AEGIS);
				expert_weapon_skill(P_SPEAR);
				obj->spe = 1;
				at_your_feet("A shield");
			}
			dropy(obj);
			u.ugifts++;
		}
	} else if (Pantheon_if(PM_MONK) || Role_if(PM_MONK)) {
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (!already_exists) {
			obj = mksobj(ROBE, FALSE, FALSE);
			if(u.ualign.type != A_CHAOTIC){
				obj = oname(obj, artiname(ART_GRANDMASTER_S_ROBE));
			} else {
				obj = oname(obj, artiname(ART_ROBE_OF_THE_ARCHMAGI));
			}
			obj->spe = 1;
			at_your_feet("A robe");
			dropy(obj);
			u.ugifts++;
		}
		if (obj && obj->oartifact == ART_GRANDMASTER_S_ROBE){
			discover_artifact(ART_GRANDMASTER_S_ROBE);
		} else if(obj && obj->oartifact == ART_ROBE_OF_THE_ARCHMAGI)
			discover_artifact(ART_ROBE_OF_THE_ARCHMAGI);
		
		if(u.ualign.type != A_CHAOTIC){
			gm_weapon_skill(P_BARE_HANDED_COMBAT);
			u.umartial = TRUE;
		} else {
			expert_weapon_skill(P_ATTACK_SPELL);
			expert_weapon_skill(P_ENCHANTMENT_SPELL);
			expert_weapon_skill(P_ESCAPE_SPELL);
			expert_weapon_skill(P_MATTER_SPELL);
		}
	} else if (Pantheon_if(PM_WIZARD) || Role_if(PM_WIZARD)) {
		if(!already_exists){
			if (class_gift != STRANGE_OBJECT) {
				;		/* already got bonus above for some reason */
			} else if (!already_exists) {
				obj = mksobj(SPE_SECRETS, FALSE, FALSE);
				if(!exist_artifact(SPE_SECRETS, artiname(ART_NECRONOMICON))) obj = oname(obj, artiname(ART_NECRONOMICON));
				else obj = oname(obj, artiname(ART_BOOK_OF_INFINITE_SPELLS));
				obj->spe = 1;
				at_your_feet("A spellbook");
				dropy(obj);
				u.ugifts++;
			}
			if (obj && obj->oartifact == ART_NECRONOMICON){
				obj->ovar1 |= SP_DEATH;
				discover_artifact(ART_NECRONOMICON);
			} else if (obj && obj->oartifact == ART_BOOK_OF_INFINITE_SPELLS){
				obj->ovar1 = SPE_FINGER_OF_DEATH;
				discover_artifact(ART_BOOK_OF_INFINITE_SPELLS);
			}
		} else{
			for(obj = invent; obj; obj=obj->nobj)
				if(obj->oartifact == ART_NECRONOMICON) 
					obj->ovar1 |= SP_DEATH;
			if(!obj) for(obj = invent; obj; obj=obj->nobj)
				if(obj->oartifact == ART_BOOK_OF_INFINITE_SPELLS) 
					obj->ovar1 = SPE_FINGER_OF_DEATH;
		}
	} else if (Pantheon_if(PM_ARCHEOLOGIST) || Role_if(PM_ARCHEOLOGIST)) {
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (!already_exists) {
			if (u.uevent.uhand_of_elbereth == 37){
				obj = mksobj(AMULET_OF_MAGICAL_BREATHING, FALSE, FALSE);
				obj = oname(obj, artiname(ART_EHECAILACOCOZCATL));
				expert_weapon_skill(P_ATTACK_SPELL);
				
			} else if (u.uevent.uhand_of_elbereth == 38){
				obj = mksobj(JAVELIN, FALSE, FALSE);
				obj = oname(obj, artiname(ART_AMHIMITL));
				expert_weapon_skill(P_SPEAR);
			} else {
				obj = mksobj(TECPATL, FALSE, FALSE);
				obj = oname(obj, artiname(ART_TECPATL_OF_HUHETOTL));
				expert_weapon_skill(P_DAGGER);
				expert_weapon_skill(P_CLERIC_SPELL);
			}
			if(obj){
				obj->spe = 1;
				obj = hold_another_object(obj, 
					"A %s appears at your feet!", 
					u.uevent.uhand_of_elbereth == 37 ? "conch shell" : (u.uevent.uhand_of_elbereth == 38 ? "harpoon": "sacrificial dagger"),
					(const char *)0);
			}

			u.ugifts++;
		}
		if (obj && obj->oartifact == ART_EHECAILACOCOZCATL)
			discover_artifact(ART_EHECAILACOCOZCATL);
		else if(obj && obj->oartifact == ART_AMHIMITL)
			discover_artifact(ART_AMHIMITL);
		else if(obj && obj->oartifact == ART_TECPATL_OF_HUHETOTL)
			discover_artifact(ART_TECPATL_OF_HUHETOTL);
	} else if (Pantheon_if(PM_NOBLEMAN) || Role_if(PM_NOBLEMAN)) {
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (!already_exists) {
			if(u.uevent.uhand_of_elbereth != 9){
				obj = mksobj(find_gcirclet(), FALSE, FALSE);
				obj = oname(obj, artiname(ART_CROWN_OF_THE_SAINT_KING));
			} else {
				obj = mksobj(find_vhelm(), FALSE, FALSE);
				obj = oname(obj, artiname(ART_HELM_OF_THE_DARK_LORD));
			}
			if(obj){
				obj->spe = 1;
				obj = hold_another_object(obj, 
					"A %s appears at your feet", 
					u.uevent.uhand_of_elbereth != 9 ? "crown" : "helm",
					(const char *)0);
				if(carried(obj)){
					if(uarmh) remove_worn_item(uarmh, TRUE);
					setworn(obj, W_ARMH);
					Helmet_on();
				}
			}
			expert_weapon_skill(P_BEAST_MASTERY);
			u.ugifts++;
		}
		if (obj && obj->oartifact == ART_CROWN_OF_THE_SAINT_KING)
			discover_artifact(ART_CROWN_OF_THE_SAINT_KING);
		else if(obj && obj->oartifact == ART_HELM_OF_THE_DARK_LORD)
			discover_artifact(ART_HELM_OF_THE_DARK_LORD);
	} else if ((Pantheon_if(PM_SAMURAI) || Role_if(PM_SAMURAI)) && u.uevent.uhand_of_elbereth == 1) {
		if (class_gift != STRANGE_OBJECT) {
			;		/* already got bonus above for some reason */
		} else if (!already_exists) {
			obj = mksobj(YUMI, FALSE, FALSE);
			obj = oname(obj, artiname(ART_YOICHI_NO_YUMI));
			obj->spe = 1;
			at_your_feet("A bow");
			dropy(obj);
			u.ugifts++;
		}
		expert_weapon_skill(P_BOW);
		if (obj && obj->oartifact == ART_YOICHI_NO_YUMI)
			discover_artifact(ART_YOICHI_NO_YUMI);
    } else {
		switch (u.ualign.type) {
			case A_LAWFUL:
			if (class_gift != STRANGE_OBJECT) {
				;		/* already got bonus above */
			} else if (obj && obj->otyp == LONG_SWORD && !obj->oartifact) {
				if (!Blind) Your("sword shines brightly for a moment.");
				obj = oname(obj, artiname(ART_EXCALIBUR));
				if (obj && obj->oartifact == ART_EXCALIBUR) u.ugifts++;
			}
			/* acquire Excalibur's skill regardless of weapon or gift */
			expert_weapon_skill(P_LONG_SWORD);
			if (obj && obj->oartifact == ART_EXCALIBUR)
				discover_artifact(ART_EXCALIBUR);
			break;
			case A_NEUTRAL:
			if (class_gift != STRANGE_OBJECT) {
				;		/* already got bonus above */
			} else if (in_hand) {
				Your("%s goes snicker-snack!", xname(obj));
				obj->dknown = TRUE;
			} else if (!already_exists) {
					obj = mksobj(LONG_SWORD, FALSE, FALSE);
				obj = oname(obj, artiname(ART_VORPAL_BLADE));
				obj->spe = 1;
				at_your_feet("A sword");
				dropy(obj);
				u.ugifts++;
			}
			/* acquire Vorpal Blade's skill regardless of weapon or gift */
			expert_weapon_skill(P_LONG_SWORD);
			if (obj && obj->oartifact == ART_VORPAL_BLADE)
				discover_artifact(ART_VORPAL_BLADE);
			break;
			case A_CHAOTIC:
			  {
			char swordbuf[BUFSZ];

			Sprintf(swordbuf, "%s sword", hcolor(NH_BLACK));
			if (class_gift != STRANGE_OBJECT) {
				;		/* already got bonus above */
			} else if (in_hand) {
				Your("%s hums ominously!", swordbuf);
				obj->dknown = TRUE;
			} else if (!already_exists) {
				obj = mksobj(RUNESWORD, FALSE, FALSE);
				obj = oname(obj, artiname(ART_STORMBRINGER));
				at_your_feet(An(swordbuf));
				obj->spe = 1;
				dropy(obj);
				u.ugifts++;
			}
			/* acquire Stormbringer's skill regardless of weapon or gift */
			expert_weapon_skill(P_BROAD_SWORD);
			if (obj && obj->oartifact == ART_STORMBRINGER)
				discover_artifact(ART_STORMBRINGER);
			break;
			  }
			default:
			obj = 0;	/* lint */
			break;
		}
	}

    /* enhance weapon regardless of alignment or artifact status */
    if (ok_wep(obj) || ok_arm(obj)) {
		bless(obj);
		obj->oeroded = obj->oeroded2 = 0;
		obj->oerodeproof = TRUE;
		obj->bknown = obj->rknown = TRUE;
		if (obj->spe < 1) obj->spe = 1;
		/* acquire skill in this weapon */
		if (ok_wep(obj)) {
			unrestrict_weapon_skill(weapon_type(obj));
		}
    } else if (class_gift == STRANGE_OBJECT) {
		/* opportunity knocked, but there was nobody home... */
		You_feel("unworthy.");
    }
    update_inventory();
    return;
}
//endif	/*ELBERETH*/

STATIC_OVL void
pleased(g_align)
	aligntyp g_align;
{
	/* don't use p_trouble, worst trouble may get fixed while praying */
	int trouble = in_trouble();	/* what's your worst difficulty? */
	int pat_on_head = 0, kick_on_butt;

	You_feel("that %s is %s.", align_gname(g_align),
	    u.ualign.record >= DEVOUT ?
	    Hallucination ? "pleased as punch" : "well-pleased" :
	    u.ualign.record >= STRIDENT ?
	    Hallucination ? "ticklish" : "pleased" :
	    Hallucination ? "full" : "satisfied");

	/* not your deity */
	if (on_altar() && p_aligntyp != u.ualign.type) {
		adjalign(-1);
		return;
	} else if (u.ualign.record < 2 && trouble <= 0) adjalign(1);

	/* depending on your luck & align level, the god you prayed to will:
	   - fix your worst problem if it's major.
	   - fix all your major problems.
	   - fix your worst problem if it's minor.
	   - fix all of your problems.
	   - do you a gratuitous favor.

	   if you make it to the the last category, you roll randomly again
	   to see what they do for you.

	   If your luck is at least 0, then you are guaranteed rescued
	   from your worst major problem. */

	if (!trouble && u.ualign.record >= DEVOUT) {
	    /* if hero was in trouble, but got better, no special favor */
	    if (p_trouble == 0) pat_on_head = 1;
	} else {
	    int action = rn1(Luck + (on_altar() ? 3 + on_shrine() : 2), 1);
	    /* pleased Lawful gods often send you a helpful angel if you're
	       getting the crap beat out of you */
	    if ((u.uhp < 5 || (u.uhp*7 < u.uhpmax)) &&
		 u.ualign.type == A_LAWFUL && rn2(3)) lawful_god_gives_angel();

	    if (!on_altar()) action = min(action, 3);
	    if (u.ualign.record < STRIDENT)
		action = (u.ualign.record > 0 || !rnl(2)) ? 1 : 0;

	    switch(min(action,5)) {
	    case 5: pat_on_head = 1;
	    case 4: do fix_worst_trouble(trouble);
		    while ((trouble = in_trouble()) != 0);
		    break;

	    case 3: fix_worst_trouble(trouble);
	    case 2: while ((trouble = in_trouble()) > 0)
		    fix_worst_trouble(trouble);
		    break;

	    case 1: if (trouble > 0) fix_worst_trouble(trouble);
	    case 0: break; /* your god blows you off, too bad */
	    }
	}

    /* note: can't get pat_on_head unless all troubles have just been
       fixed or there were no troubles to begin with; hallucination
       won't be in effect so special handling for it is superfluous */
    if(pat_on_head){
		//Note: Luck > 10 means that you have a luckitem in open inventory.  To avoid crowning, just drop the luckitem.
	    if (Luck > 10 && u.ualign.record >= PIOUS && !u.uevent.uhand_of_elbereth && u.uevent.qcompleted) 
			gcrownu();
		else switch(rn2((Luck + 6)>>1)) {
		case 0:	break;
		case 1:
			if (uwep && (welded(uwep) || uwep->oclass == WEAPON_CLASS ||
				 is_weptool(uwep))) {
			char repair_buf[BUFSZ];

			*repair_buf = '\0';
			if (uwep->oeroded || uwep->oeroded2)
				Sprintf(repair_buf, " and %s now as good as new",
					otense(uwep, "are"));

			if (uwep->cursed) {
				uncurse(uwep);
				uwep->bknown = TRUE;
				if (!Blind)
				Your("%s %s%s.", aobjnam(uwep, "softly glow"),
					 hcolor(NH_AMBER), repair_buf);
				else You_feel("the power of %s over your %s.",
				u_gname(), xname(uwep));
				*repair_buf = '\0';
			} else if (!uwep->blessed) {
				bless(uwep);
				uwep->bknown = TRUE;
				if (!Blind)
				Your("%s with %s aura%s.",
					 aobjnam(uwep, "softly glow"),
					 an(hcolor(NH_LIGHT_BLUE)), repair_buf);
				else You_feel("the blessing of %s over your %s.",
				u_gname(), xname(uwep));
				*repair_buf = '\0';
			}

			/* fix any rust/burn/rot damage, but don't protect
			   against future damage */
			if (uwep->oeroded || uwep->oeroded2) {
				uwep->oeroded = uwep->oeroded2 = 0;
				/* only give this message if we didn't just bless
				   or uncurse (which has already given a message) */
				if (*repair_buf)
				Your("%s as good as new!",
					 aobjnam(uwep, Blind ? "feel" : "look"));
			}
			update_inventory();
			}
			break;
		case 3:
			/* takes 2 hints to get the music to enter the stronghold */
			if (!u.uevent.uopened_dbridge) {
			if (u.uevent.uheard_tune < 2) {
				godvoice(g_align,(char *)0);
				verbalize("Hark, %s!",
				  youracedata->mlet == S_HUMAN ? "mortal" : "creature");
				verbalize(
				"To enter the castle, thou must play the right tune!");
				You_hear("a divine music...");
				pline("It sounds like:  \"%s\".", tune);
				u.uevent.uheard_tune = 2;
				break;
			}
			}
			/* Otherwise, falls into next case */
		case 2:
			if (!Blind)
			You("are surrounded by %s glow.", an(hcolor(NH_GOLDEN)));
			/* if any levels have been lost (and not yet regained),
			   treat this effect like blessed full healing */
			if (u.ulevel < u.ulevelmax) {
			// u.ulevelmax -= 1;	/* see potion.c */
			pluslvl(FALSE);
			} else {
			u.uhpbonus += 5;
			calc_total_maxhp();
			}
			u.uhp = u.uhpmax;
			if (Upolyd) u.mh = u.mhmax;
			ABASE(A_STR) = AMAX(A_STR);
			if(Race_if(PM_INCANTIFIER)){
				if (u.uen < u.uenmax*.45) u.uen += 400;
				newuhs(TRUE);
			} else {
				if (u.uhunger < u.uhungermax*.45) u.uhunger = u.uhungermax*.45;
				u.uhs = NOT_HUNGRY;
			}
			if (u.uluck < 0) u.uluck = 0;
			make_blinded(0L,TRUE);
			flags.botl = 1;
			break;
		case 4: {
			register struct obj *otmp;
			int any = 0;

			if (Blind)
			You_feel("the power of %s.", u_gname());
			else You("are surrounded by %s aura.",
				 an(hcolor(NH_LIGHT_BLUE)));
			for(otmp=invent; otmp; otmp=otmp->nobj) {
			if (otmp->cursed) {
				uncurse(otmp);
				if (!Blind) {
				Your("%s %s.", aobjnam(otmp, "softly glow"),
					 hcolor(NH_AMBER));
				otmp->bknown = TRUE;
				++any;
				}
			}
			}
			if (any) update_inventory();
			if(u.sealsActive&SEAL_MARIONETTE) unbind(SEAL_MARIONETTE,TRUE);
			break;
		}
		case 5: {
			const char *msg="\"and thus I grant thee the gift of %s!\"";
			godvoice(u.ualign.type, "Thou hast pleased me with thy progress,");
			if (!(HTelepat & INTRINSIC))  {
			HTelepat |= FROMOUTSIDE;
			pline(msg, "Telepathy");
			if (Blind) see_monsters();
			} else if (!(HFast & INTRINSIC))  {
			HFast |= FROMOUTSIDE;
			pline(msg, "Speed");
			} else if (!(HStealth & INTRINSIC))  {
			HStealth |= FROMOUTSIDE;
			pline(msg, "Stealth");
			} else if(!(u.wardsknown & WARD_HAMSA)){
				u.wardsknown |= WARD_HAMSA;
				pline(msg, "the Hamsa ward");
			} else if(!(u.wardsknown & WARD_HEXAGRAM)){
				u.wardsknown |= WARD_HEXAGRAM;
				pline(msg, "the Hexagram ward");
			}else {
			if (!(HProtection & INTRINSIC))  {
				HProtection |= FROMOUTSIDE;
				if (!u.ublessed)  u.ublessed = rn1(3, 2);
			} else u.ublessed++;
			pline(msg, "my protection");
			}
			verbalize("Use it wisely in my name!");
			break;
		}
		case 7:
		case 8:
		case 9:		/* KMH -- can occur during full moons */
		case 6:	{
			struct obj *otmp;
			int sp_no, trycnt = u.ulevel + 1;

			at_your_feet("An object");
			/* not yet known spells given preference over already known ones */
			/* Also, try to grant a spell for which there is a skill slot */
			otmp = mkobj(SPBOOK_CLASS, TRUE);
			while (--trycnt > 0) {
			if (otmp->otyp != SPE_BLANK_PAPER) {
				for (sp_no = 0; sp_no < MAXSPELL; sp_no++)
				if (spl_book[sp_no].sp_id == otmp->otyp) break;
				if (sp_no == MAXSPELL &&
				!P_RESTRICTED(spell_skilltype(otmp->otyp)))
				break;	/* usable, but not yet known */
			} else {
				if (!objects[SPE_BLANK_PAPER].oc_name_known ||
					carrying(MAGIC_MARKER)) break;
			}
			otmp->otyp = rnd_class(bases[SPBOOK_CLASS], SPE_BLANK_PAPER);
			}
			bless(otmp);
			place_object(otmp, u.ux, u.uy);
			break;
		}
		default:	impossible("Confused deity!");
			break;
		}
	}
	
	/*Scare hostile monsters on level*/
	{
		struct monst *tmpm;
		for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
			if(!tmpm->mpeaceful){
				monflee(tmpm, 77, TRUE, TRUE);
			}
		}
	}
	
	u.ublesscnt = rnz(350);
	kick_on_butt = u.uevent.udemigod ? 1 : 0;
#ifdef ELBERETH
	if (u.uevent.uhand_of_elbereth) kick_on_butt++;
#endif
	if (kick_on_butt) u.ublesscnt += kick_on_butt * rnz(1000);

	return;
}

/* either blesses or curses water on the altar,
 * returns true if it found any water here.
 */
STATIC_OVL boolean
water_prayer(bless_water)
    boolean bless_water;
{
    register struct obj* otmp;
    register long changed = 0;
	int non_water_changed = 0;
    boolean other = FALSE, bc_known = !(Blind || Hallucination);

    for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere) {
	/* turn water into (un)holy water */
	if (otmp->otyp == POT_WATER &&
		(bless_water ? !otmp->blessed : !otmp->cursed)) {
	    otmp->blessed = bless_water;
	    otmp->cursed = !bless_water;
	    otmp->bknown = bc_known;
	    changed += otmp->quan;
	} else if(otmp->oclass == POTION_CLASS)
	    other = TRUE;
	else if (otmp->oproperties & ((bless_water) ? OPROP_HOLYW : OPROP_UNHYW) && (bless_water ? !otmp->blessed : !otmp->cursed))
	{
		otmp->blessed = bless_water;
		otmp->cursed = !bless_water;
		otmp->bknown = bc_known;
		if (!Blind) {
			if (strncmpi(The(xname(otmp)), "The ", 4))
			{
				pline("On the altar, %s glow%s %s for a moment.",
					xname(otmp), (otmp->quan > 1L ? "" : "s"),
					(bless_water ? hcolor(NH_LIGHT_BLUE) : hcolor(NH_BLACK)));
			}
			else
			{
				pline("%s on the altar glow%s %s for a moment.",
					The(xname(otmp)), (otmp->quan > 1L ? "" : "s"),
					(bless_water ? hcolor(NH_LIGHT_BLUE) : hcolor(NH_BLACK)));
			}
		}
	}
    }
    if(!Blind && changed) {
	pline("%s potion%s on the altar glow%s %s for a moment.",
	      ((other && changed > 1L) ? "Some of the" :
					(other ? "One of the" : "The")),
	      ((other || changed > 1L) ? "s" : ""), (changed > 1L ? "" : "s"),
	      (bless_water ? hcolor(NH_LIGHT_BLUE) : hcolor(NH_BLACK)));
    }
    return((boolean)(changed > 0L));
}

STATIC_OVL void
godvoice(g_align, words)
    aligntyp g_align;
    const char *words;
{
    const char *quot = "";
    if(words)
	quot = "\"";
    else
	words = "";
	
	if(g_align != A_VOID){
		pline_The("voice of %s %s: %s%s%s", align_gname(g_align),
		  godvoices[rn2(SIZE(godvoices))], quot, words, quot);
	} else {
		You("think you hear a voice in the distance: %s%s%s", quot, words, quot);
	}
}

void
gods_angry(g_align)
    aligntyp g_align;
{
    godvoice(g_align, "Thou hast angered me.");
}

/* The g_align god is upset with you. */
void
gods_upset(g_align)
	aligntyp g_align;
{
	if(g_align == A_VOID) return;
	if(g_align == u.ualign.type) u.ugangr[Align2gangr(u.ualign.type)]++;
	else if(u.ugangr[Align2gangr(u.ualign.type)]){
		u.ugangr[Align2gangr(u.ualign.type)]--;
		u.ugangr[Align2gangr(g_align)]++;
	}
	else u.ugangr[Align2gangr(g_align)]++;
	angrygods(g_align);
}

static NEARDATA const char sacrifice_types[] = { FOOD_CLASS, AMULET_CLASS, 0 };

STATIC_OVL void
consume_offering(otmp)
register struct obj *otmp;
{
    if (Hallucination)
	switch (rn2(25)) {
	    case 0:
		Your("sacrifice sprouts wings and a propeller and roars away!");
		break;
	    case 1:
		Your("sacrifice puffs up, swelling bigger and bigger, and pops!");
		break;
	    case 2:
		Your("sacrifice collapses into a cloud of dancing particles and fades away!");
	    case 3:
		Your("sacrifice scarcifies!");
	    case 4:
		You("can't find your sacrifice.");
		You("must have misplaced it!");
		break;
	    case 5:
		Your("sacrifice is consumed with doubt!");
		break;
	    case 6:
		Your("sacrifice rots away!");
		break;
		case 7:
		godvoice(rn2(3)-1, "Hey! I ordered the chicken!");
		break;
		case 8:
		godvoice(rn2(3)-1, "Oh, gross! Honey, the pets left another dead critter on the doorstep!");
		break;
		case 9:
		Your("sacrifice is consumed in a flash!");
		break;
		case 10:
		Your("sacrifice is consumed by the altar!");
		break;
		case 11:
		You("consume the sacrifice!");
		break;
		case 12:
		Your("sacrifice is rejected!");
		break;
		case 13:
		Your("sacrifice is dejected!");
		break;
		case 14:
		godvoice(rn2(3)-1, "This better be kosher!");
		break;
		case 15:
		Your("sacrifice is consumed in a lash of fight!");
		break;
		case 16:
		Your("sacrifice is lame!");
		break;
		case 17:
		You("are consumed in a %s!",
	      u.ualign.type != A_LAWFUL ? "flash of light" : "burst of flame");
    	break;
	    case 18:
		Your("sacrifice is consumed with jealousy!");
		break;
	    case 19:
		Your("sacrifice is consumed with embarrassment!");
		break;
	    case 20:
		Your("sacrifice is consumed with self-hate!");
		break;
	    case 21:
		Your("sacrifice was stolen by fairies!");
		break;
	    case 22:
		Your("sacrifice is vanishes in a dash at night!");
	    case 23:
		u.ualign.type == A_LAWFUL ?
			Your("sacrifice is consumed in a flash of %s light!", hcolor(0)):
			Your("sacrifice is consumed in a burst of %s flame!", hcolor(0));
		break;
	    case 24:
		Your("sacrifice is consumed by trout!");
		break;
	}
    else if (Blind && u.ualign.type == A_LAWFUL)
	Your("sacrifice disappears!");
    else Your("sacrifice is consumed in a %s!",
	      u.ualign.type == A_LAWFUL ? "flash of light" : "burst of flame");
	if(u.sealsActive&SEAL_BALAM){
		struct permonst *ptr = &mons[otmp->corpsenm];
		if(!(is_animal(ptr) || nohands(ptr))) unbind(SEAL_BALAM,TRUE);
	}
	if(u.sealsActive&SEAL_YMIR){
		struct permonst *ptr = &mons[otmp->corpsenm];
		if(is_giant(ptr)) unbind(SEAL_YMIR,TRUE);
	}
    if (carried(otmp)) useup(otmp);
    else useupf(otmp, 1L);
    exercise(A_WIS, TRUE);
}

void
god_gives_pet(gptr, alignment)
const char *gptr;
aligntyp alignment;
{
/*
    register struct monst *mtmp2;
    register struct permonst *pm;
 */
    const int *minions = god_minions(gptr);
    int mnum=NON_PM, mlev, num = 0, first, last;
	struct monst *mon = (struct monst *)0;
	
	mlev = level_difficulty();
	
	for (first = 0; minions[first] != NON_PM; first++)
	    if (!(mvitals[minions[first]].mvflags & G_GONE && !In_quest(&u.uz)) && monstr[minions[first]] > mlev-5) break;
	if(minions[first] == NON_PM){ //All minions too weak, or no minions
		if(first == 0) return;
		else mnum = minions[first-1];
	}
	else for (last = first; minions[last] != NON_PM; last++)
	    if (!(mvitals[minions[last]].mvflags & G_GONE && !In_quest(&u.uz))) {
			/* consider it */
			if(monstr[minions[last]] > mlev*2) break;
			num += min(1,mons[minions[last]].geno & G_FREQ);
	    }

	if(!num){ //All minions too strong, or gap between weak and strong minions
		if(first == 0) return;
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
    }
	else mon = make_pet_minion(mnum,alignment);
	
    if (mon) {
	switch ((int)alignment) {
	   case A_LAWFUL:
		if (u.uhp > (u.uhpmax / 10)) godvoice(u.ualign.type, "My minion shall serve thee!");
		else godvoice(u.ualign.type, "My minion shall save thee!");
	   break;
	   case A_NEUTRAL:
	   case A_VOID:
		pline("%s", Blind ? "You hear the earth rumble..." :
		 "A cloud of gray smoke gathers around you!");
	   break;
	   case A_CHAOTIC:
	   case A_NONE:
		pline("%s", Blind ? "You hear an evil chuckle!" :
		 "A miasma of stinking vapors coalesces around you!");
	   break;
	}
	return;
    }
}

static void
lawful_god_gives_angel()
{
/*
    register struct monst *mtmp2;
    register struct permonst *pm;
*/
    int mnum;
    int mon;

    // mnum = lawful_minion(u.ulevel);
    // mon = make_pet_minion(mnum,A_LAWFUL);
    // pline("%s", Blind ? "You feel the presence of goodness." :
	 // "There is a puff of white fog!");
    // if (u.uhp > (u.uhpmax / 10)) godvoice(u.ualign.type, "My minion shall serve thee!");
    // else godvoice(u.ualign.type, "My minion shall save thee!");
	god_gives_pet(align_gname_full(A_LAWFUL),A_LAWFUL);
}


int
dosacrifice()
{
    register struct obj *otmp;
    int value = 0;
    int pm;
    aligntyp altaralign = a_align(u.ux,u.uy);
    if (!on_altar() || u.uswallow) {
	You("are not standing on an altar.");
	return 0;
    }

	if(Role_if(PM_ANACHRONONAUT) && flags.questprogress != 2 && u.uevent.qcompleted && u.uhave.questart && Is_astralevel(&u.uz)){
		You("worry that you have not yet completed your mission.");
	}
	
    if (In_endgame(&u.uz)) {
	if (!(otmp = getobj(sacrifice_types, "sacrifice"))) return 0;
    } else {
	if (!(otmp = floorfood("sacrifice", 1))) return 0;
    }
    /*
      Was based on nutritional value and aging behavior (< 50 moves).
      Sacrificing a food ration got you max luck instantly, making the
      gods as easy to please as an angry dog!

      Now only accepts corpses, based on the game's evaluation of their
      toughness.  Human and pet sacrifice, as well as sacrificing unicorns
      of your alignment, is strongly discouraged.
     */
	
	if(goat_mouth_at(u.ux, u.uy)){
		goat_eat(otmp);
		return 1;
	}
	
	if(Role_if(PM_ANACHRONONAUT) && otmp->otyp != AMULET_OF_YENDOR && flags.questprogress!=2){
		You("do not give offerings to the God of the future.");
		return 0;
	}
	
#define MAXVALUE 24 /* Highest corpse value (besides Wiz) */

    if (otmp->otyp == CORPSE) {
		register struct permonst *ptr = &mons[otmp->corpsenm];
		struct monst *mtmp;
		extern const int monstr[];

		/* KMH, conduct */
		u.uconduct.gnostic++;

		/* you're handling this corpse, even if it was killed upon the altar */
		feel_cockatrice(otmp, TRUE);

		if ((otmp->corpsenm == PM_ACID_BLOB
			|| (monstermoves <= peek_at_iced_corpse_age(otmp) + 50)
			) && mons[otmp->corpsenm].mlet != S_PLANT
		) {
			value = monstr[otmp->corpsenm] + 1;
			if (otmp->oeaten)
			value = eaten_stat(value, otmp);
		}

		if (your_race(ptr) && !is_animal(ptr) && !mindless(ptr) && u.ualign.type != A_VOID) {
			if (is_demon(youracedata)) {
				You("find the idea very satisfying.");
				exercise(A_WIS, TRUE);
			} else if (u.ualign.type != A_CHAOTIC || altaralign != A_CHAOTIC) {
				if((u.ualign.record >= 20 || ACURR(A_WIS) >= 20 || u.ualign.record >= rnd(20-ACURR(A_WIS))) && !roll_madness(MAD_CANNIBALISM)){
					char buf[BUFSZ];
					Sprintf(buf, "You feel a deep sense of kinship to %s!  Sacrifice %s anyway?",
						the(xname(otmp)), (otmp->quan == 1L) ? "it" : "one");
					if (yn_function(buf,ynchars,'n')=='n') return 0;
				}
				pline("You'll regret this infamous offense!");
				exercise(A_WIS, FALSE);
			}

			if (altaralign != A_CHAOTIC && altaralign != A_NONE) {
			/* curse the lawful/neutral altar */
			if(Race_if(PM_INCANTIFIER)) pline_The("altar is stained with human blood, the blood of your birth race.");
			else pline_The("altar is stained with %s blood.", urace.adj);
			if(!Is_astralevel(&u.uz))
				levl[u.ux][u.uy].altarmask = AM_CHAOTIC;
			angry_priest();
			} else {
			struct monst *dmon;
			const char *demonless_msg;

			/* Human sacrifice on a chaotic or unaligned altar */
			/* is equivalent to demon summoning */
			if (altaralign == A_CHAOTIC && u.ualign.type != A_CHAOTIC) {
				pline(
				 "The blood floods the altar, which vanishes in %s cloud!",
				  an(hcolor(NH_BLACK)));
				levl[u.ux][u.uy].typ = ROOM;
				levl[u.ux][u.uy].altarmask = 0;
				newsym(u.ux, u.uy);
				angry_priest();
				demonless_msg = "cloud dissipates";
			} else {
				/* either you're chaotic or altar is Moloch's or both */
				pline_The("blood covers the altar!");
				change_luck(altaralign == A_NONE ? -2 : 2);
				demonless_msg = "blood coagulates";
			}
			if ((pm = dlord((struct permonst *) 0, altaralign)) != NON_PM &&
				(dmon = makemon(&mons[pm], u.ux, u.uy, NO_MM_FLAGS))) {
				You("have summoned %s!", a_monnam(dmon));
				if (sgn(u.ualign.type) == sgn(dmon->data->maligntyp))
				dmon->mpeaceful = TRUE;
				You("are terrified, and unable to move.");
				nomul(-3, "being terrified of a demon");
			} else pline_The("%s.", demonless_msg);
			}

			if (u.ualign.type != A_CHAOTIC) {
			adjalign(-5);
			u.ugangr[Align2gangr(u.ualign.type)] += 3;
			(void) adjattrib(A_WIS, -1, TRUE);
			if (!Inhell) angrygods(u.ualign.type);
			change_luck(-5);
			} else adjalign(5);
			if (carried(otmp)) useup(otmp);
			else useupf(otmp, 1L);
			return(1);
		} else if (otmp->oxlth && otmp->oattached == OATTACHED_MONST
				&& ((mtmp = get_mtraits(otmp, FALSE)) != (struct monst *)0)
				&& mtmp->mtame) {
			/* mtmp is a temporary pointer to a tame monster's attributes,
			 * not a real monster */
			pline("So this is how you repay loyalty?");
			adjalign(-3);
			value = -1;
			HAggravate_monster |= FROMOUTSIDE;
		} else if (is_undead(ptr)) { /* Not demons--no demon corpses */
			if (u.ualign.type != A_CHAOTIC)
			value += 1;
		} else if (is_unicorn(ptr)) {
			int unicalign = sgn(ptr->maligntyp);

			/* If same as altar, always a very bad action. */
			if (unicalign == altaralign) {
			pline("Such an action is an insult to %s!",
				  (unicalign == A_CHAOTIC)
				  ? "chaos" : unicalign ? "law" : "balance");
			(void) adjattrib(A_WIS, -1, TRUE);
			value = -5;
			} else if (u.ualign.type == altaralign) {
			/* If different from altar, and altar is same as yours, */
			/* it's a very good action */
			if (u.ualign.record < ALIGNLIM)
				You_feel("appropriately %s.", align_str(u.ualign.type));
			else You_feel("you are thoroughly on the right path.");
			adjalign(5);
			value += 3;
			} else
			/* If sacrificing unicorn of your alignment to altar not of */
			/* your alignment, your god gets angry and it's a conversion */
			if (unicalign == u.ualign.type) {
				u.ualign.record = -1;
				value = 1;
			} else value += 3;
		}
    } /* corpse */

    if (otmp->otyp == AMULET_OF_YENDOR) {
		if (!Is_astralevel(&u.uz)) {
			if (Hallucination)
				You_feel("homesick.");
			else
				You_feel("an urge to return to the surface.");
			return 1;
		} else {
			/* The final Test.	Did you win? */
			if(uamul == otmp) Amulet_off();
			u.uevent.ascended = 1;
			if(carried(otmp)) useup(otmp); /* well, it's gone now */
			else useupf(otmp, 1L);
			You("offer the Amulet of Yendor to %s...", a_gname());
			if(!Role_if(PM_EXILE)){
				if (u.ualign.type != altaralign) {
					/* And the opposing team picks you up and
					   carries you off on their shoulders */
					adjalign(-99);
					pline("%s accepts your gift, and gains dominion over %s...",
						  a_gname(), u_gname());
					pline("%s is enraged...", u_gname());
					pline("Fortunately, %s permits you to live...", a_gname());
					pline("A cloud of %s smoke surrounds you...",
						  hcolor((const char *)"orange"));
					done(ESCAPED);
				} else { /* super big win */
					adjalign(10);
#ifdef RECORD_ACHIEVE
					achieve.ascended = 1;
#endif
					pline("An invisible choir sings, and you are bathed in radiance...");
					godvoice(altaralign, "Congratulations, mortal!");
					display_nhwindow(WIN_MESSAGE, FALSE);
					verbalize("In return for thy service, I grant thee the gift of Immortality!");
					You("ascend to the status of Demigod%s...",
						flags.female ? "dess" : "");
					done(ASCENDED);
				}
			} else {
				if (altaralign == A_LAWFUL) {
					/* And the opposing team picks you up and
					carries you off on their shoulders */
					adjalign(-99);
					pline("%s accepts your gift, and regains complete control over his creation.", a_gname());
					pline("In that instant, you loose all your powers as %s shuts the Gate.", a_gname());
					pline("Fortunately, %s permits you to live...", a_gname());
					pline("Occasionally, you may even be able to remember that you have forgoten something.");
					pline("A cloud of %s smoke surrounds you...",
						hcolor((const char *)"orange"));
					done(ESCAPED);
				} else if(altaralign == A_CHAOTIC) {
					/* And the opposing team picks you up and
					carries you off on their shoulders */
					adjalign(-99);
					pline("%s accepts your gift, and gains complete control over creation.", a_gname());
					pline("In the next instant, she destroys it.");
					pline("You are functionally dead, your soul shorn from its earthly husk...");
					pline("...as well as everything that made you YOU.");
					killer_format = KILLED_BY;
					killer = "the end of the world."; //8-bit theater
					done(DISINTEGRATED);
				} else { /* super big win */
					adjalign(10);
#ifdef RECORD_ACHIEVE
					achieve.ascended = 1;
#endif
					pline("From the threshold of the Gate, you look back at the world");
					pline("You don't know what awaits you in the Void,");
					pline("but whatever happens, the way shall remain open behind you,");
					pline("that others may make their own choice.");
					done(ASCENDED);
				}
			}
		}
    } /* real Amulet */

    if (otmp->otyp == FAKE_AMULET_OF_YENDOR) {
	    if (flags.soundok)
		You_hear("a nearby thunderclap.");
	    if (!otmp->known) {
		You("realize you have made a %s.",
		    Hallucination ? "boo-boo" : "mistake");
		otmp->known = TRUE;
		change_luck(-1);
		return 1;
	    } else {
		/* don't you dare try to fool the gods */
		if(u.ualign.type != A_VOID){
			change_luck(-3);
			adjalign(-1);
			u.ugangr[Align2gangr(u.ualign.type)] += 3;
			value = -3;
			u.lastprayresult = PRAY_ANGER;
			u.lastprayed = moves;
			u.reconciled = REC_NONE;
		}
	    }
    } /* fake Amulet */

    if (value == 0) {
	pline1(nothing_happens);
	return (1);
    }

    if (altaralign != u.ualign.type &&
	(Is_astralevel(&u.uz) || Is_sanctum(&u.uz))) {
	/*
	 * REAL BAD NEWS!!! High altars cannot be converted.  Even an attempt
	 * gets the god who owns it truely pissed off.
	 */
	You_feel("the air around you grow charged...");
	pline("Suddenly, you realize that %s has noticed you...", a_gname());
	godvoice(altaralign, "So, mortal!  You dare desecrate my High Temple!");
	/* Throw everything we have at the player */
	god_zaps_you(altaralign);
    } else if (value < 0) { /* I don't think the gods are gonna like this... */
	gods_upset(altaralign);
    } else {
	int saved_anger = u.ugangr[Align2gangr(u.ualign.type)];
	int saved_cnt = u.ublesscnt;
	int saved_luck = u.uluck;

	/* Sacrificing at an altar of a different alignment */
	if (u.ualign.type != altaralign) {
	    /* Is this a conversion ? */
	    /* An unaligned altar in Gehennom will always elicit rejection. */
	    if ((ugod_is_angry() && u.ualign.type != A_VOID) || (altaralign == A_NONE && Inhell)) {
		if(u.ualignbase[A_CURRENT] == u.ualignbase[A_ORIGINAL] &&
		   altaralign != A_NONE && altaralign != A_VOID && !Role_if(PM_EXILE)) {
		    You("have a strong feeling that %s is angry...", u_gname());
			if(otmp->otyp == CORPSE && is_rider(&mons[otmp->corpsenm])){
				pline("A pulse of darkness radiates from your sacrifice!");
				angrygods(altaralign);
				return 1;
			}
			consume_offering(otmp);
		    pline("%s accepts your allegiance.", a_gname());

		    /* The player wears a helm of opposite alignment? */
		    if (uarmh && uarmh->otyp == HELM_OF_OPPOSITE_ALIGNMENT)
			u.ualignbase[A_CURRENT] = altaralign;
		    else
			u.ualign.type = u.ualignbase[A_CURRENT] = altaralign;
		    u.ublessed = 0;
		    flags.botl = 1;

		    You("have a sudden sense of a new direction.");
		    /* Beware, Conversion is costly */
		    change_luck(-3);
		    u.ublesscnt += 300;
		    u.lastprayed = moves;
			u.reconciled = REC_NONE;
		    u.lastprayresult = PRAY_CONV;
		    adjalign((int)(u.ualignbase[A_ORIGINAL] * (ALIGNLIM / 2)));
			if(Race_if(PM_DROW)){
				switch(u.ualignbase[A_CURRENT]){
					case A_LAWFUL:
						if(Role_if(PM_NOBLEMAN)){
							if(flags.initgend){
								u.uhouse = VER_TAS_SYMBOL;
							} else {
								u.uhouse = VER_TAS_SYMBOL;
							}
						} else {
							if(flags.initgend){
								u.uhouse = EILISTRAEE_SYMBOL;
							} else {
								u.uhouse = EDDER_SYMBOL;
							}
						}
					break;
					case A_NEUTRAL:
						if(Role_if(PM_NOBLEMAN)){
							if(flags.initgend){
								u.uhouse = KIARANSALEE_SYMBOL;
							} else {
								u.uhouse = u.start_house; /*huh?*/
							}
						} else {
							if(flags.initgend){
								u.uhouse = KIARANSALEE_SYMBOL;
							} else {
								u.uhouse = u.start_house; /*huh?*/
							}
						}
					break;
					case A_CHAOTIC:
						if(Role_if(PM_NOBLEMAN)){
							if(flags.initgend){
								u.uhouse = u.start_house; /*huh?*/
							} else {
								u.uhouse = GHAUNADAUR_SYMBOL;
							}
						} else {
							if(flags.initgend){
								u.uhouse = u.start_house; /*huh?*/
							} else {
								u.uhouse = u.start_house; /*Converted to direct Lolth worship*/
							}
						}
					break;
				}
			}
		} else {
			if(altaralign == A_VOID){
				consume_offering(otmp);
				if (!Inhell){
					godvoice(u.ualign.type, "Suffer, infidel!");
					u.ugangr[Align2gangr(u.ualign.type)] += 3;
					adjalign(-5);
					u.lastprayed = moves;
					u.lastprayresult = PRAY_ANGER;
					u.reconciled = REC_NONE;
					change_luck(-5);
					(void) adjattrib(A_WIS, -2, TRUE);
					angrygods(u.ualign.type);
				} else {
					pline("Silence greets your offering.");
				}
			} else {
				u.ugangr[Align2gangr(u.ualign.type)] += 3;
				adjalign(-5);
				u.lastprayed = moves;
				u.lastprayresult = PRAY_ANGER;
				u.reconciled = REC_NONE;
				pline("%s rejects your sacrifice!", a_gname());
				godvoice(altaralign, "Suffer, infidel!");
				change_luck(-5);
				(void) adjattrib(A_WIS, -2, TRUE);
				if (!Inhell) angrygods(u.ualign.type);
			}
		}
		return(1);
	    } else {
		if(otmp->otyp == CORPSE && is_rider(&mons[otmp->corpsenm])){
			pline("A pulse of darkness radiates from your sacrifice!");
			angrygods(altaralign);
			return 1;
		}
		consume_offering(otmp);
		if(Role_if(PM_EXILE) && u.ualign.type != A_VOID && altaralign != A_VOID){
			You("sense a conference between %s and %s.",
				u_gname(), a_gname());
			pline("But nothing else occurs.");
		} else {
			You("sense a conflict between %s and %s.",
				u_gname(), a_gname());
			if (rn2(8 + u.ulevel) > 5) {
				struct monst *pri;
				You_feel("the power of %s increase.", u_gname());
				exercise(A_WIS, TRUE);
				change_luck(1);
				/* Yes, this is supposed to be &=, not |= */
				levl[u.ux][u.uy].altarmask &= AM_SHRINE;
				/* the following accommodates stupid compilers */
				levl[u.ux][u.uy].altarmask =
				levl[u.ux][u.uy].altarmask | (Align2amask(u.ualign.type));
				if (!Blind)
				pline_The("altar glows %s.",
					  hcolor(
					  u.ualign.type == A_LAWFUL ? NH_WHITE :
					  u.ualign.type ? NH_BLACK : (const char *)"gray"));
				if(Role_if(PM_EXILE) && In_quest(&u.uz) && u.uz.dlevel == nemesis_level.dlevel){
					int door = 0, ix, iy;
					if(altaralign == A_CHAOTIC) door = 1;
					else if(altaralign == A_NEUTRAL) door = 2;
					else if(altaralign == A_LAWFUL) door = 3;
					else if(!u.uevent.qcompleted && altaralign == A_NONE){
						pline("The muted tension that filled the wind fades away.");
						pline("You here again the thousand-tounged whisperers,");
						pline("though you cannot make out the name they repeat.");
						makemon(&mons[PM_ACERERAK],u.ux,u.uy,MM_ADJACENTOK);
						pline("Someone now stands beside you!");
					}
					if(door){
						for(ix = 1; ix < COLNO; ix++){
							for(iy = 0; iy < ROWNO; iy++){
								if(IS_DOOR(levl[ix][iy].typ) && artifact_door(ix,iy) == door){
									You_hear("a door open.");
									levl[ix][iy].typ = ROOM;
									unblock_point(ix,iy);
								}
							}
						}
					}
				}
				// if (rnl(u.ulevel) > 6 && u.ualign.record > 0 &&
				   // rnd(u.ualign.record) > (3*ALIGNLIM)/4)
				if(!Pantheon_if(PM_ELF)){
					if(u.ulevel > 20) summon_god_minion(align_gname_full(altaralign),altaralign, TRUE);
					if(u.ulevel > 10) summon_god_minion(align_gname_full(altaralign),altaralign, TRUE);
					(void) summon_god_minion(align_gname_full(altaralign),altaralign, TRUE);
				}
				/* anger priest; test handles bones files */
				if((pri = findpriest(temple_occupied(u.urooms))) &&
				   !p_coaligned(pri))
				angry_priest();
			} else {
				pline("Unluckily, you feel the power of %s decrease.",
				  u_gname());
				change_luck(-1);
				exercise(A_WIS, FALSE);
				if(!Pantheon_if(PM_ELF)){
					if(u.ulevel > 20) summon_god_minion(align_gname_full(altaralign),altaralign, TRUE);
					if(u.ulevel > 10) summon_god_minion(align_gname_full(altaralign),altaralign, TRUE);
					(void) summon_god_minion(align_gname_full(altaralign),altaralign, TRUE);
				}
			}
		}
		return(1);
	    }
	}

	if(otmp->otyp == CORPSE && is_rider(&mons[otmp->corpsenm])){
		pline("A pulse of darkness radiates from your sacrifice!");
		angrygods(altaralign);
		return 1;
	}
	consume_offering(otmp);
	/*if(altaralign == A_UNKNOWN) return(1);*/
	/* OK, you get brownie points. */
	if(u.ugangr[Align2gangr(u.ualign.type)]) {
	    u.ugangr[Align2gangr(u.ualign.type)] -=
		((value * (u.ualign.type == A_CHAOTIC ? 2 : 3)) / MAXVALUE);
	    if(u.ugangr[Align2gangr(u.ualign.type)] < 0) u.ugangr[Align2gangr(u.ualign.type)] = 0;
	    if(u.ugangr[Align2gangr(u.ualign.type)] != saved_anger) {
		if (u.ugangr[Align2gangr(u.ualign.type)]) {
		    pline("%s seems %s.", u_gname(),
			  Hallucination ? "groovy" : "slightly mollified");

		    if ((int)u.uluck < 0) change_luck(1);
		} else {
		    pline("%s seems %s.", u_gname(), Hallucination ?
			  "cosmic (not a new fact)" : "mollified");

		    if ((int)u.uluck < 0) u.uluck = 0;
		    u.reconciled = REC_MOL;
		}
	    } else { /* not satisfied yet */
		if (Hallucination)
		    pline_The("gods seem tall.");
		else You("have a feeling of inadequacy.");
	    }
	} else if(ugod_is_angry()) {
	    if(value > MAXVALUE) value = MAXVALUE;
	    if(value > -u.ualign.record) value = -u.ualign.record;
	    adjalign(value);
	    You_feel("partially absolved.");
	} else if (u.ublesscnt > 0) {
	    u.ublesscnt -=
		((value * (u.ualign.type == A_CHAOTIC ? 500 : 300)) / MAXVALUE);
	    if(u.ublesscnt < 0) u.ublesscnt = 0;
	    if(u.ublesscnt != saved_cnt) {
		if (u.ublesscnt) {
		    if (Hallucination)
			You("realize that the gods are not like you and I.");
		    else
			You("have a hopeful feeling.");
		    if ((int)u.uluck < 0) change_luck(1);
		} else {
		    if (Hallucination)
			pline("Overall, there is a smell of fried onions.");
		    else
			You("have a feeling of reconciliation.");
		    if ((int)u.uluck < 0) u.uluck = 0;
			
			u.reconciled = REC_REC;
		}
	    }
	} else {
	    int nartifacts = (int)(u.uconduct.wisharti + u.ugifts);
		//pline("looking into an artifact gift.  %d currently exist. %d gifts have been given, on level %d, and your luck %d.", nartifacts, (int)u.ugifts, u.ulevel, (int)u.uluck);
	    /* you were already in pretty good standing */
	    /* The player can gain an artifact */
	    /* The chance goes down as the number of artifacts goes up */
		/* Priests now only count gifts in this calculation, found artifacts are excluded */
	    if (u.ulevel > 2 && u.uluck >= 0 &&
			(
			Role_if(PM_PRIEST) ? !rn2(10 + (2 * u.ugifts * u.ugifts)) : !rn2(10 + (2 * u.ugifts * nartifacts)) 
			)
		) {
		otmp = mk_artifact((struct obj *)0, a_align(u.ux,u.uy));
		if (otmp) {
		    if (otmp->spe < 0) otmp->spe = 0;
		    if (otmp->cursed) uncurse(otmp);
		    otmp->oerodeproof = TRUE;
		    dropy(otmp);
		    at_your_feet("An object");
		    godvoice(u.ualign.type, "Use my gift wisely!");
			otmp->gifted = u.ualign.type;
			u.ugifts++;
		    u.ublesscnt = rnz(300 + (50 * nartifacts));
			u.lastprayed = moves;
			u.reconciled = REC_NONE;
			u.lastprayresult = PRAY_GIFT;
		    exercise(A_WIS, TRUE);
		    if (!flags.debug && otmp->oartifact) {
				char llog[BUFSZ+22];
				Sprintf(llog, "was given %s", the(artilist[otmp->oartifact].name));
				livelog_write_string(llog);
		    }
		    /* make sure we can use this weapon */
		    unrestrict_weapon_skill(weapon_type(otmp));
		    discover_artifact(otmp->oartifact);
			if(otmp->oartifact == ART_BLADE_SINGER_S_SPEAR || otmp->oartifact == ART_BLADE_DANCER_S_DAGGER){
			 if(otmp->oartifact == ART_BLADE_SINGER_S_SPEAR && !exist_artifact(DAGGER, "Blade Dancer's Dagger") ){
				otmp = mksobj(DAGGER, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_BLADE_DANCER_S_DAGGER));		
			 } else if(otmp->oartifact == ART_BLADE_DANCER_S_DAGGER && !exist_artifact(SPEAR, "Blade Singer's Spear") ){
				otmp = mksobj(SPEAR, TRUE, FALSE);
				otmp = oname(otmp, artiname(ART_BLADE_SINGER_S_SPEAR));		
			 }
				unrestrict_weapon_skill(P_SPEAR);
				unrestrict_weapon_skill(P_DAGGER);
				unrestrict_weapon_skill(P_TWO_WEAPON_COMBAT);
				if (otmp->spe < 0) otmp->spe = 0;
				if (otmp->cursed) uncurse(otmp);
				otmp->oerodeproof = TRUE;
				dropy(otmp);
			} else if(otmp->oartifact == ART_BEASTMASTER_S_DUSTER){
				unrestrict_weapon_skill(P_BEAST_MASTERY);
			} else if(otmp->oartifact == ART_GRANDMASTER_S_ROBE || otmp->oartifact == ART_PREMIUM_HEART){
				unrestrict_weapon_skill(P_BARE_HANDED_COMBAT);
				u.umartial = TRUE;
			} else if(otmp->oartifact == ART_RHONGOMYNIAD){
				unrestrict_weapon_skill(P_RIDING);
				mksobj_at(SADDLE, u.ux, u.uy, FALSE, FALSE);
				u.umartial = TRUE;
			}
			return(1);
		}
	    } else if (rnl((30 + u.ulevel)*10) < 10) {
			/* no artifact, but maybe a helpful pet? */
			/* WAC is now some generic benefit (includes pets) */
			god_gives_benefit(altaralign);
		    return(1);
	    }
	    change_luck((value * LUCKMAX) / (MAXVALUE * 2));
	    if ((int)u.uluck < 0) u.uluck = 0;
	    if (u.uluck != saved_luck) {
		if (Blind)
		    You("think %s brushed your %s.",something, body_part(FOOT));
		else You(Hallucination ?
		    "see crabgrass at your %s.  A funny thing in a dungeon." :
		    "glimpse a four-leaf clover at your %s.",
		    makeplural(body_part(FOOT)));
	    }
		u.reconciled = REC_REC;
	}
    }
    return(1);
}


/* determine prayer results in advance; also used for enlightenment */
boolean
can_pray(praying)
boolean praying;	/* false means no messages should be given */
{
    int alignment;

    p_aligntyp = on_altar() ? a_align(u.ux,u.uy) : u.ualign.type;
    p_trouble = in_trouble();

    if (is_demon(youracedata) && (p_aligntyp != A_CHAOTIC)) {
	if (praying)
	    pline_The("very idea of praying to a %s god is repugnant to you.",
		  p_aligntyp ? "lawful" : "neutral");
	return FALSE;
    }

    if (praying)
	You("begin praying to %s.", align_gname(p_aligntyp));

    if (u.ualign.type && u.ualign.type == -p_aligntyp)
	alignment = -u.ualign.record;		/* Opposite alignment altar */
    else if (u.ualign.type != p_aligntyp)
	alignment = u.ualign.record / 2;	/* Different alignment altar */
    else alignment = u.ualign.record;

    if ((int)Luck < 0 || u.ugangr[Align2gangr(u.ualign.type)] || alignment < 0)
        p_type = 0;             /* too naughty... */
    else if ((p_trouble > 0) ? (u.ublesscnt > 200) : /* big trouble */
	(p_trouble < 0) ? (u.ublesscnt > 100) : /* minor difficulties */
	(u.ublesscnt > 0))			/* not in trouble */
	p_type = 1;		/* too soon... */
    else /* alignment >= 0 */ {
	if(on_altar() && u.ualign.type != p_aligntyp)
	    p_type = 2;
	else
	    p_type = 3;
    }

    if (is_undead(youracedata) && !Inhell &&
	(p_aligntyp == A_LAWFUL || (p_aligntyp == A_NEUTRAL && !rn2(10))))
	p_type = -1;
    /* Note:  when !praying, the random factor for neutrals makes the
       return value a non-deterministic approximation for enlightenment.
       This case should be uncommon enough to live with... */

    return !praying ? (boolean)(p_type == 3 && !(Inhell && u.ualign.type != A_VOID)) : TRUE;
}

int
dopray()
{
    /* Confirm accidental slips of Alt-P */
	if(Role_if(PM_ANACHRONONAUT) && flags.questprogress!=2){
		pline("There is but one God in the future.");
		pline("And to It, you do not pray.");
		return 0;
	}
	
    if (flags.prayconfirm)
	if (yn("Are you sure you want to pray?") == 'n')
	    return 0;

	if(u.sealsActive&SEAL_AMON) unbind(SEAL_AMON,TRUE);
    u.uconduct.gnostic++;
    /* Praying implies that the hero is conscious and since we have
       no deafness attribute this implies that all verbalized messages
       can be heard.  So, in case the player has used the 'O' command
       to toggle this accessible flag off, force it to be on. */
    flags.soundok = 1;

    /* set up p_type and p_alignment */
    if (!can_pray(TRUE)) return 0;
	
	u.lastprayed = moves;
	u.reconciled = REC_NONE;
#ifdef WIZARD
    if (wizard && p_type >= 0) {
	if (yn("Force the gods to be pleased?") == 'y') {
	    u.ublesscnt = 0;
	    if (u.uluck < 0) u.uluck = 0;
	    if (u.ualign.record <= 0) u.ualign.record = 1;
	    u.ugangr[Align2gangr(u.ualign.type)] = 0;
	    if(p_type < 2) p_type = 3;
	}
    }
#endif
    nomul(-3, "praying");
    nomovemsg = "You finish your prayer.";
    afternmv = prayer_done;

    if((p_type == 3 && !(Inhell && u.ualign.type != A_VOID)) || (uwep && uwep->oartifact == ART_LANCE_OF_LONGINUS)) {
	/* if you've been true to your god you can't die while you pray */
	if (!Blind)
	    You("are surrounded by a shimmering light.");
	u.uinvulnerable = TRUE;
    }

    return(1);
}

STATIC_PTR int
prayer_done()		/* M. Stephenson (1.0.3b) */
{
    aligntyp alignment = p_aligntyp;

    u.uinvulnerable = FALSE;
	u.lastprayresult = PRAY_GOOD;
    if(p_type == -1) {
		godvoice(alignment,
			 alignment == A_LAWFUL ?
			 "Vile creature, thou durst call upon me?" :
			 "Walk no more, perversion of nature!");
		You_feel("like you are falling apart.");
	/* KMH -- Gods have mastery over unchanging */
	if (!Race_if(PM_VAMPIRE)) {
		u.lastprayresult = PRAY_GOOD;
		rehumanize();
		losehp(rnd(20), "residual undead turning effect", KILLED_BY_AN);
	} else {
		u.lastprayresult = PRAY_BAD;
	   /* Starting vampires are inherently vampiric */
	   losehp(rnd(20), "undead turning effect", KILLED_BY_AN);
	   pline("You get the idea that %s will be of %s help to you.",
	      align_gname(alignment),
			 alignment == A_LAWFUL ?
			 "little" :
			 "at best sporadic");
	}
	exercise(A_CON, FALSE);
	if(on_altar()){
		(void) water_prayer(FALSE);
		change_luck(-3);
		gods_upset(alignment);
	}
	return(1);
    }
    if (Inhell && u.ualign.type != A_VOID && !(alignment == A_CHAOTIC && strcmp(urole.cgod,"Lolth"))) {
	pline("Since you are in Gehennom, %s won't help you.",
	      align_gname(alignment));
	/* haltingly aligned is least likely to anger */
	if (u.ualign.record <= 0 || rnl(u.ualign.record))
	    angrygods(u.ualign.type);
	return(0);
    }

    if (p_type == 0) {
        if(on_altar() && u.ualign.type != alignment)
            (void) water_prayer(FALSE);
        angrygods(u.ualign.type);       /* naughty */
    } else if (p_type == 1) {
		if(on_altar() && u.ualign.type != alignment)
			(void) water_prayer(FALSE);
		if(u.ualign.type != A_VOID){
			u.ublesscnt += rnz(250);
			change_luck(-3);
			gods_upset(u.ualign.type);
		}
    } else if(p_type == 2) {
		if(water_prayer(FALSE)) {
			/* attempted water prayer on a non-coaligned altar */
			u.ublesscnt += rnz(250);
			change_luck(-3);
			if(u.ualign.type != A_VOID) gods_upset(u.ualign.type);
		} else pleased(alignment);
    } else {
	/* coaligned */
	if(on_altar())
	    (void) water_prayer(TRUE);
	pleased(alignment); /* nice */
    }
    return(1);
}

int
doturn()
{	/* Knights & Priest(esse)s only please */
	struct monst *mtmp, *mtmp2;
	int once, range, xlev;
	short fast = 0;

	if (!Role_if(PM_PRIEST) && !Role_if(PM_KNIGHT) && !Race_if(PM_VAMPIRE) && !(Role_if(PM_NOBLEMAN) && Race_if(PM_ELF))){
		/* Try to use turn undead spell. */
		if (objects[SPE_TURN_UNDEAD].oc_name_known) {
		    register int sp_no;
		    for (sp_no = 0; sp_no < MAXSPELL &&
			 spl_book[sp_no].sp_id != NO_SPELL &&
			 spl_book[sp_no].sp_id != SPE_TURN_UNDEAD; sp_no++);

		    if (sp_no < MAXSPELL &&
			spl_book[sp_no].sp_id == SPE_TURN_UNDEAD)
			    return spelleffects(sp_no, TRUE, 0);
		}

		You("don't know how to turn undead!");
		return(0);
	}
	if(!Race_if(PM_VAMPIRE)) u.uconduct.gnostic++;

	if(!Race_if(PM_VAMPIRE) && u.uen >= 30 && yn("Use abbreviated liturgy?") == 'y'){
		fast = 1;
	}
	
	if ((u.ualign.type != A_CHAOTIC && !Race_if(PM_VAMPIRE) &&
		    (is_demon(youracedata) || is_undead(youracedata))) ||
				u.ugangr[Align2gangr(u.ualign.type)] > 6 /* "Die, mortal!" */) {

		pline("For some reason, %s seems to ignore you.", u_gname());
		aggravate();
		exercise(A_WIS, FALSE);
		return(0);
	}

	if (Inhell && !Race_if(PM_VAMPIRE)) {
	    pline("Since you are in Gehennom, %s won't help you.", u_gname());
	    aggravate();
	    return(0);
	}
	if(!Race_if(PM_VAMPIRE)) pline("Calling upon %s, you chant holy scripture.", u_gname());
	else You("focus your vampiric aura!");
	exercise(A_WIS, TRUE);

	/* note: does not perform unturn_dead() on victims' inventories */
	range = BOLT_LIM + (u.ulevel / 5);	/* 5 to 11 */
	range *= range;
	once = 0;
	for(mtmp = fmon; mtmp; mtmp = mtmp2) {
	    mtmp2 = mtmp->nmon;

	    if (DEADMONSTER(mtmp)) continue;
	    if (!cansee(mtmp->mx,mtmp->my) ||
		distu(mtmp->mx,mtmp->my) > range) continue;

	    if (!mtmp->mpeaceful && (is_undead_mon(mtmp) ||
		   (is_demon(mtmp->data) && (u.ulevel > (MAXULEV/2))))) {

		    mtmp->msleeping = 0;
		    if (Confusion) {
			if (!once++){
			    if(!Race_if(PM_VAMPIRE)) pline("Unfortunately, your voice falters.");
			    else pline("Unfortunately, your concentration falters.");
			}
			if(mtmp->data != &mons[PM_BANDERSNATCH]) mtmp->mflee = 0;
			mtmp->mfrozen = 0;
			mtmp->mcanmove = 1;
		    } else if (!resist(mtmp, '\0', 0, TELL)) {
			xlev = 6;
			switch (mtmp->data->mlet) {
			    /* this is intentional, lichs are tougher
			       than zombies. */
			case S_LICH:    xlev += 2;  /*FALLTHRU*/
			case S_SHADE:   xlev += 2;  /*FALLTHRU*/
			case S_GHOST:   xlev += 2;  /*FALLTHRU*/
			case S_BAT: //Asumes undead bats are vampires
			case S_VAMPIRE: xlev += 2;  /*FALLTHRU*/
			case S_WRAITH:  xlev += 2;  /*FALLTHRU*/
			case S_MUMMY:   xlev += 2;  /*FALLTHRU*/
			case S_ZOMBIE:
			default:
			    if (u.ulevel >= xlev &&
				    !resist(mtmp, '\0', 0, NOTELL)) {
				if (u.ualign.type == A_CHAOTIC || Race_if(PM_VAMPIRE)){
				    mtmp->mpeaceful = 1;
				    set_malign(mtmp);
					if(PM_VAMPIRE) tamedog(mtmp, (struct obj *)0);
				} else { /* damn them */
				    killed(mtmp);
				}
				break;
			    } /* else flee */
			    /*FALLTHRU*/
			    monflee(mtmp, 0, FALSE, TRUE);
			    break;
			}
		    }
	    }
	}
	//Altered turn undead to consume energy if possible, otherwise take full time.
	if(fast){
		losepw(30);
		nomul(-1, "trying to turn the undead");
	}
	else
		nomul(-5, "trying to turn the undead");
	return(1);
}

const char *
a_gname()
{
    return(a_gname_at(u.ux, u.uy));
}

const char *
a_gname_at(x,y)     /* returns the name of an altar's deity */
xchar x, y;
{
    if(!IS_ALTAR(levl[x][y].typ)) return((char *)0);

    return align_gname(a_align(x,y));
}

const char *
u_gname()  /* returns the name of the player's deity */
{
    return align_gname(u.ualign.type);
}

const char * const hallu_gods[] = {
	"goodness",
	
	"the Yggdrasil Entity",
	"the Unbounded Blue",
	"the Silencer in the stars",
	"the Drowned",
	
	//TV tropes
	"the angels",
	"the devils",
	"the squid",
	
	//Games
	"Armok",
	"_Hylia",
	"_the three golden goddesses",
	"_the Lady of Pain",
	"the Outsider",
	"Yevon",
	"Bhunivelze",
	"_Etro",
	"the Transcendent One",
	"_the Mana Tree",
	"Golden Silver",
	"_Luna",
	"Arceus",
	"the Composer",
	"the Conductor",
	"Chakravartin",
	"Chattur'gha",
	"Ulyaoth",
	"_Xel'lo'tath",
	"Mantorok",
	"_Martel",
	"Gwyn, Lord of Sunlight",
	
	//Literature
	"Mahasamatman",
	"Nyarlathotep",
	"Azathoth",
	"Ubbo-Sathla",
	"Galactus",
	"Sauron",
	"Morgoth",
	"Eru Iluvatar",
	"the Powers that Be",
	"the Lone Power",
	"Om",
	"small gods",
	"Orannis",
	
	"The Water Phoenix King",
	"_Ailari, Goddess of Safe Journeys",
	"_Yuenki, Dark Lady of Apotheosis",
	"_Zeth Kahl, Dark Empress of The Gift Given to Bind",
	
	//Anime
	"the Data Overmind",
	"the Sky Canopy Domain",
	
	"the Truth and the Gate",
	
	"Ceiling Cat", /* Lolcats */
	"Zoamelgustar", /* Slayers */
	"Brohm", /* Ultima */
	"Xenu", /* Scientology */
	"the God of Cabbage", /* K-On! */
	"Bill Cipher", /* Gravity Falls */
	"Gades", "Amon", "_Erim", "Daos", /* Lufia series */
        "_Beatrice", /* Umineko no Naku Koro ni */
	"Spongebob Squarepants", /* should be obvious, right? :) */
	"Chiyo-chichi", /* Azumanga Daioh */
	"The Wondrous Mambo god", /* Xenogears */
	
	//Internet
	"SCP-343",
	"the Slender Man",
	"the Powers What Is",
	
	//atheism
	"the universe",
	"the Flying Spaghetti Monster",
	"_the Invisible Pink Unicorn",
	"last thursday",
	
	//Economic systems
	"Capitalism",
	"Communism",

	//Ok, so I have no idea what this is....
	//<Muad> same thing as the UN
	//<Muad> if that fits the context
	"the U.N.O.",
	
	//Vague
	"something",
	"someone",
	"whatever",
	"thing",
	"thingy",
	"whatchamacallit",
	"whosamawhatsit",
	"some guy",
	"_some gal",
	
	//nethack
	"the gnome with the wand of death",
	"the DevTeam",
	"Dion Nicolaas",
	"marvin",
	
	"stth the first demigod",
	"stth the wizard",
	"_stth the valkyrie",
	"stth the barbarian",
	"stth the tourist",
	"stth the healer",
	"stth the pirate", /* <stth> LOL RIGHT... FIRST DNETHACK STREAK
						  <stth> tiny 2-game streak, but it's a streak lol 
					   */
	"allihaveismymind the second demigod",
	"allihaveismymind the vanilla valkyrie",
	"Khor the third demigod",
	"Khor the noble",
	"Khor the priest",
	"Khor the ranger",
	"Khor the monk",
	"Khor the gnomish ranger",
	"ChrisANG the fourth demigod",
	"ChrisANG the binder",
	"FIQ the fifth demigod",
	"Tariru the elven noble",
	"Tariru the dwarf knight",
	"Tariru the samurai",
	"Tariru the archeologist",
	"Tariru the droven noble",
	"VoiceOfReason the knight",
	"VoiceOfReason the dwarven noble",
	"HBane the anachrononaut",
	
	"Dudley",
	"the RNG"
};

const char *
align_gname(alignment)
aligntyp alignment;
{
    const char *gnam;

	if (Hallucination) {
		gnam = hallu_gods[rn2(SIZE(hallu_gods))];
		if (*gnam == '_') ++gnam;
		return gnam;
	}
    switch (alignment) {
     case A_NONE:
		if(In_FF_quest(&u.uz)){
			if(on_level(&chaose_level,&u.uz)) gnam = DeepChaos;
			else gnam = Chaos;
		} else if(In_mithardir_quest(&u.uz)){
			gnam = Silence;
		} else if(In_mordor_quest(&u.uz)){
			if(on_level(&u.uz, &borehole_4_level)) gnam = MolochLieutenant;
			else gnam = Moloch;
		}
		else if(Role_if(PM_EXILE) && In_quest(&u.uz)) gnam = Demiurge;
		else if(In_lost_cities(&u.uz)){
			if(on_level(&rlyeh_level,&u.uz)) gnam = AllInOne;
			else if(on_level(&lethe_headwaters,&u.uz))
				gnam = Nodens;
			else if(on_level(&lethe_temples,&u.uz))
				gnam = BlackMother;
			else gnam = Other;
		}
		else if(In_outlands(&u.uz)){
			gnam = DreadFracture;
		}
		else gnam = Moloch;
	 break;
     case A_LAWFUL:
		if(Role_if(PM_EXILE) && Is_astralevel(&u.uz)){
						gnam = Demiurge; break;
		} else if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz) && !quest_status.got_quest){
			gnam = ""; break;
		} else {
						gnam = urole.lgod; break;
		}
     case A_NEUTRAL:	
		if(Role_if(PM_EXILE) && Is_astralevel(&u.uz)){
						gnam = tVoid; break;
		} else if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)){
			gnam = ""; break;
		} else {
						gnam = urole.ngod; break;
		}
     case A_CHAOTIC:
		if(Role_if(PM_EXILE) && Is_astralevel(&u.uz)){
						gnam = Sophia; break;
		} else if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)){
			gnam = ""; break;
		} else {
						gnam = urole.cgod; break;
		}
     case A_VOID:		gnam = tVoid; break;
     default:		impossible("unknown alignment.");
			gnam = "someone"; break;
    }
    if (*gnam == '_') ++gnam;
    return gnam;
}

const char *
align_gname_full(alignment)
aligntyp alignment;
{
    const char *gnam;

    switch (alignment) {
     case A_NONE:
		if(In_FF_quest(&u.uz)){
			if(on_level(&chaose_level,&u.uz)) gnam = DeepChaos;
			else gnam = Chaos;
		} else if(In_mithardir_quest(&u.uz)){
			gnam = Silence;
		} else if(In_mordor_quest(&u.uz)){
			if(on_level(&u.uz, &borehole_4_level)) gnam = MolochLieutenant;
			else gnam = Moloch;
		}
		else if(Role_if(PM_EXILE) && In_quest(&u.uz)) gnam = Demiurge;
		else if(In_outlands(&u.uz)) gnam = DreadFracture;
		else if(In_lost_cities(&u.uz)){
			if(on_level(&rlyeh_level,&u.uz)) gnam = AllInOne;
			else if(on_level(&lethe_headwaters,&u.uz))
				gnam = Nodens;
			else if(on_level(&lethe_temples,&u.uz))
				gnam = BlackMother;
			else gnam = Other;
		}
		else gnam = Moloch;
	 break;
     case A_LAWFUL:
		if(!Role_if(PM_EXILE) || !Is_astralevel(&u.uz)){ 
						gnam = urole.lgod; break;
		} else {
						gnam = Demiurge; break;
		}
     case A_NEUTRAL:	
		if(!Role_if(PM_EXILE) || !Is_astralevel(&u.uz)){ 
						gnam = urole.ngod; break;
		} else {
						gnam = tVoid; break;
		}
     case A_CHAOTIC:	
		if(!Role_if(PM_EXILE) || !Is_astralevel(&u.uz)){ 
						gnam = urole.cgod; break;
		} else {
						gnam = Sophia; break;
		}
     case A_VOID:		gnam = tVoid; break;
     default:		impossible("unknown alignment.");
			gnam = "someone"; break;
    }
    return gnam;
}

/* hallucination handling for priest/minion names: select a random god
   iff character is hallucinating */
const char *
halu_gname(alignment)
aligntyp alignment;
{
    const char *gnam;
    int which;

    if (!Hallucination) return align_gname(alignment);

    which = randrole(0);
    switch (rn2(3)) {
     case 0:	gnam = roles[which].lgod; break;
     case 1:	gnam = roles[which].ngod; break;
     case 2:	gnam = roles[which].cgod; break;
     default:	gnam = 0; break;		/* lint suppression */
    }
    if (!gnam) gnam = Moloch;
    if (*gnam == '_') ++gnam;
    return gnam;
}

/* deity's title */
const char *
align_gtitle(alignment)
aligntyp alignment;
{
    const char *gnam, *result = "god";

    switch (alignment) {
     case A_LAWFUL:	gnam = urole.lgod; break;
     case A_NEUTRAL:	gnam = urole.ngod; break;
     case A_CHAOTIC:	gnam = urole.cgod; break;
     default:		gnam = 0; break;
    }
    if (gnam && *gnam == '_') result = "goddess";
    return result;
}

void
altar_wrath(x, y)
register int x, y;
{
    aligntyp altaralign = a_align(x,y);

    if(!strcmp(align_gname(altaralign), u_gname())) {
	godvoice(altaralign, "How darest thou desecrate my altar!");
	(void) adjattrib(A_WIS, -1, FALSE);
    } else {
	pline("A voice (could it be %s?) whispers:",
	      align_gname(altaralign));
	verbalize("Thou shalt pay, infidel!");
	change_luck(-1);
    }
}

/* assumes isok() at one space away, but not necessarily at two */
STATIC_OVL boolean
blocked_boulder(dx,dy)
int dx,dy;
{
    register struct obj *otmp;
    long count = 0L;

    for(otmp = level.objects[u.ux+dx][u.uy+dy]; otmp; otmp = otmp->nexthere) {
	if(is_boulder(otmp))
	    count += otmp->quan;
    }

    switch(count) {
	case 0: return FALSE; /* no boulders--not blocked */
	case 1: break; /* possibly blocked depending on if it's pushable */
	default: return TRUE; /* >1 boulder--blocked after they push the top
	    one; don't force them to push it first to find out */
    }

    if (!isok(u.ux+2*dx, u.uy+2*dy))
	return TRUE;
    if (IS_ROCK(levl[u.ux+2*dx][u.uy+2*dy].typ))
	return TRUE;
    if (boulder_at(u.ux+2*dx, u.uy+2*dy))
	return TRUE;

    return FALSE;
}


/*
 * A candle on a coaligned altar burns brightly or dimly
 * depending on your prayer status.
 */
 
int
candle_on_altar(candle) 
struct obj *candle;
{
  if (candle->where != OBJ_FLOOR 
     || !IS_ALTAR(levl[candle->ox][candle->oy].typ)
     ||  a_align(candle->ox, candle->oy) != u.ualign.type) {

     return 0;
  }

  can_pray(FALSE);

  /* Return a value indicating the chances of successful prayer */
 
  return (p_type > 2 ? 1 : p_type - 1);
}

/* Give away something */
void
god_gives_benefit(alignment)
aligntyp alignment;
{
	register struct obj *otmp;
	const char *what = (const char *)0;
	
	if (rnl((30 + u.ulevel)*10) < 10) god_gives_pet(align_gname_full(alignment),alignment);
	else {
		switch (rnl(5)) {
			case 0: /* randomly charge an object */
			case 1: /* increase weapon bonus */
				if(uwep && uwep->spe < 7 && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep))){
					uwep->spe++;
				}
			case 2: /* randomly identify items in the backpack */
			case 3: /* do magic mapping */
			case 4: /* give some food */
			case 5: /* randomly bless items */
		    /* weapon takes precedence if it interferes
		       with taking off a ring or shield */

		    if (uwep && !uwep->blessed) /* weapon */
			    otmp = uwep;
		    else if (uswapwep && !uswapwep->blessed) /* secondary weapon */
			    otmp = uswapwep;
		    /* gloves come next, due to rings */
		    else if (uarmg && !uarmg->blessed)    /* gloves */
			    otmp = uarmg;
		    /* then shield due to two handed weapons and spells */
		    else if (uarms && !uarms->blessed)    /* shield */
			    otmp = uarms;
		    /* then cloak due to body armor */
		    else if (uarmc && !uarmc->blessed)    /* cloak */
			    otmp = uarmc;
		    else if (uarm && !uarm->blessed)      /* armor */
			    otmp = uarm;
		    else if (uarmh && !uarmh->blessed)    /* helmet */
			    otmp = uarmh;
		    else if (uarmf && !uarmf->blessed)    /* boots */
			    otmp = uarmf;
//ifdef TOURIST
		    else if (uarmu && !uarmu->blessed)    /* shirt */
			    otmp = uarmu;
//endif
		    /* (perhaps amulet should take precedence over rings?) */
		    else if (uleft && !uleft->blessed)
			    otmp = uleft;
		    else if (uright && !uright->blessed)
			    otmp = uright;
		    else if (uamul && !uamul->blessed) /* amulet */
			    otmp = uamul;
		    else {
			    for(otmp=invent; otmp; otmp=otmp->nobj)
				if (!otmp->blessed)
					break;
			    return; /* Nothing to do! */
		    }
		    bless(otmp);
		    otmp->bknown = TRUE;
		    if (!Blind)
			    Your("%s %s.",
				 what ? what :
				 (const char *)aobjnam (otmp, "softly glow"),
				 hcolor(NH_AMBER));
			break;
		}
	}
}

STATIC_OVL int
goat_resurrect(otmp, yours)
struct obj *otmp;
boolean yours;
{
	struct monst *revived = 0;
	if(goat_monster(&mons[otmp->corpsenm])){
		pline("%s twitches.", The(xname(otmp)));
		revived = revive(otmp, FALSE);
		if(yours)
			angrygods(A_NONE);
	}
	if(revived)
		return TRUE;
	return FALSE;
}

STATIC_OVL int
goat_rider(otmp, yours)
struct obj *otmp;
boolean yours;
{
	int cn = otmp->corpsenm;
	struct monst *revived = 0;
	if(is_rider(&mons[otmp->corpsenm])){
		pline("A pulse of darkness radiates %s!", The(xname(otmp)));
		revived = revive(otmp, FALSE);
		if(yours)
			angrygods(A_NONE);
	}
	if(revived)
		return TRUE;
	return FALSE;
}

STATIC_OVL void
goat_gives_benefit()
{
	struct obj *optr;
	if (rnl((30 + u.ulevel)*10) < 10) god_gives_pet(align_gname_full(A_NONE),A_NONE);
	else switch(rnd(7)){
		case 1:
			if (Hallucination)
				You_feel("in touch with the Universal Oneness.");
			else
				You_feel("like someone is helping you.");
			for (optr = invent; optr; optr = optr->nobj) {
				uncurse(optr);
			}
			if(Punished) unpunish();
		break;
		case 2:
			change_luck(2*LUCKMAX);
		break;
		case 3:
			if(uwep && !uwep->oartifact && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)) && !(uwep->oproperties&OPROP_ACIDW)){
				if(!Blind) pline("Acid drips from your weapon!");
				uwep->oproperties |= OPROP_ACIDW;
				uwep->oeroded = 0;
				uwep->oeroded2 = 0;
				uwep->oerodeproof = 1;
			}
		break;
		case 4:
			if(HSterile){
				You_feel("fertile.");
				HSterile = 0L;
			}
		break;
		case 5:
		case 6:
		case 7:
			optr = mksobj(POT_GOAT_S_MILK, FALSE, FALSE);
			optr->quan = rnd(8);
			optr->owt = weight(optr);
			dropy(optr);
			optr->quan > 1 ? at_your_feet("Some objects") : at_your_feet("An object");
		break;
	}
	return;
}

boolean
goat_mouth_at(x, y)
int x, y;
{
	struct monst *mtmp;
	for(mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon){
		if(mtmp->mux == u.uz.dnum && mtmp->muy == u.uz.dlevel && mtmp->data == &mons[PM_MOUTH_OF_THE_GOAT]){
			xchar xlocale, ylocale, xyloc;
			xyloc	= mtmp->mtrack[0].x;
			xlocale = mtmp->mtrack[1].x;
			ylocale = mtmp->mtrack[1].y;
			if(xyloc == MIGR_EXACT_XY && xlocale == x && ylocale == y)
				return TRUE;
		}
	}
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
		if(mtmp->data == &mons[PM_MOUTH_OF_THE_GOAT] && distu(mtmp->mx,mtmp->my) == 1){
			return TRUE;
		}
	}
	return FALSE;
}

void
goat_eat(otmp)
struct obj *otmp;
{
    int value = 0;
    aligntyp altaralign = a_align(u.ux,u.uy);
	register struct permonst *ptr = &mons[otmp->corpsenm];
	struct monst *mtmp;
	extern const int monstr[];
	boolean yourinvent = FALSE;
	
	yourinvent = carried(otmp);
	
	if(goat_resurrect(otmp, yourinvent)){
		//otmp is now gone, and resurrect may have printed messages
		return;
	}
	
	if(goat_rider(otmp, yourinvent)){
		//otmp is now gone, and rider may have printed messages
		return;
	}
	

	if ((otmp->corpsenm == PM_ACID_BLOB
		|| (monstermoves <= peek_at_iced_corpse_age(otmp) + 50)
		) && mons[otmp->corpsenm].mlet != S_PLANT
	) {
		value = monstr[otmp->corpsenm] + 1;
		if (otmp->oeaten)
		value = eaten_stat(value, otmp);
	} else {
		//minimum, but still eat.
		value = 1;
	}

	if (yourinvent && your_race(ptr) && !is_animal(ptr) && !mindless(ptr) && u.ualign.type != A_VOID && !Role_if(PM_ANACHRONONAUT)) {
	//No demon summoning.  Your god just smites you, and sac continues.
		if (u.ualign.type != A_CHAOTIC) {
			adjalign(-5);
			u.ugangr[Align2gangr(u.ualign.type)] += 3;
			(void) adjattrib(A_WIS, -1, TRUE);
			if (!Inhell) angrygods(u.ualign.type);
			change_luck(-5);
		} else adjalign(5);
		if (carried(otmp)) useup(otmp);
		else useupf(otmp, 1L);
	//Pets are just eaten like anything else.  Your god doesn't know you did it, and the goat doesn't care.
	} else if (is_undead(ptr)) { /* Not demons--no demon corpses */
		if (u.ualign.type != A_CHAOTIC)
			value += 1;
	//Unicorns are resurrected.
	}
    /* corpse */
	//Value can't be 0
	//Value can't be -1
    {
	int saved_anger = u.ugangr[GA_MOTHER];
	int saved_cnt = u.ugoatblesscnt;
	int saved_luck = u.uluck;

	/* Sacrificing at an altar of a different alignment */
	/* Never a conversion */
	/* never an altar conversion*/
	
	/* Rider handled */
	consume_offering(otmp);
	if(yourinvent && u.ualign.type != A_CHAOTIC && u.ualign.type != A_VOID && !Role_if(PM_ANACHRONONAUT)) {
		adjalign(-value);
		u.ugangr[Align2gangr(u.ualign.type)] += 1;
		(void) adjattrib(A_WIS, -1, TRUE);
		if (!Inhell) angrygods(u.ualign.type);
		change_luck(-1);
	}
	/*if(altaralign == A_UNKNOWN) return;*/
	/* OK, you get brownie points. */
	if(u.ugangr[GA_MOTHER]) {
	    u.ugangr[GA_MOTHER] -=
		((value * (u.ualign.type == A_CHAOTIC ? 2 : 3)) / MAXVALUE);
	    if(u.ugangr[GA_MOTHER] < 0) u.ugangr[GA_MOTHER] = 0;
	    if(u.ugangr[GA_MOTHER] != saved_anger) {
		if (u.ugangr[GA_MOTHER]) {
		    pline("%s seems %s.", u_gname(),
			  Hallucination ? "groovy" : "slightly mollified");

		    if ((int)u.uluck < 0) change_luck(1);
		} else {
		    pline("%s seems %s.", u_gname(), Hallucination ?
			  "cosmic (not a new fact)" : "mollified");

		    if ((int)u.uluck < 0) u.uluck = 0;
		    u.reconciled = REC_MOL;
		}
	    } else { /* not satisfied yet */
		if (Hallucination)
		    pline_The("gods seem tall.");
		else You("have a feeling of inadequacy.");
	    }
	//No alignment record for the goat
	} else if (u.ugoatblesscnt > 0) {
	    u.ugoatblesscnt -=
		((value * (u.ualign.type == A_CHAOTIC ? 500 : 300)) / MAXVALUE);
	    if(u.ugoatblesscnt < 0) u.ugoatblesscnt = 0;
	    if(u.ugoatblesscnt != saved_cnt) {
		if (u.ugoatblesscnt) {
		    if (Hallucination)
			You("realize that the gods are not like you and I.");
		    else
			You("have a hopeful feeling.");
		    if ((int)u.uluck < 0) change_luck(1);
		} else {
		    if (Hallucination)
			pline("Overall, there is a smell of fried onions.");
		    else
			You("have a feeling of reconciliation.");
		    if ((int)u.uluck < 0) u.uluck = 0;
			
			u.reconciled = REC_REC;
		}
	    }
	} else if(yourinvent){
	    int nartifacts = (int)(u.uconduct.wisharti + u.ugifts);
		//The Black Goat is pleased
		struct monst *mtmp;
		for(mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon){
			if(mtmp->mux == u.uz.dnum && mtmp->muy == u.uz.dlevel && (mtmp->data == &mons[PM_BLESSED] || mtmp->data == &mons[PM_MOUTH_OF_THE_GOAT])){
				mtmp->mpeaceful = 1;
				set_malign(mtmp);
			}
		}
		for(mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon){
			if(mtmp->mux == u.uz.dnum && mtmp->muy == u.uz.dlevel && goat_monster(mtmp->data)){
				mtmp->mpeaceful = 1;
				set_malign(mtmp);
			}
		}
		//Character needs a holy symbol
		if(!has_object_type(invent, HOLY_SYMBOL_OF_THE_BLACK_MOTHE)){
			struct obj *otmp;
			if(!rn2(10+u.ugifts)){
				otmp = mksobj(HOLY_SYMBOL_OF_THE_BLACK_MOTHE, FALSE, FALSE);
				dropy(otmp);
				at_your_feet("An object");
				u.ugifts++;
			}
			return;
		}
		// pline("looking into goat gift.  %d currently exist. %d gifts have been given, on level %d, and your luck %d.", nartifacts, (int)u.ugifts, u.ulevel, (int)u.uluck);
	    /* you were already in pretty good standing */
	    /* The player can gain an artifact */
	    /* The chance goes down as the number of artifacts goes up */
		/* Priests now only count gifts in this calculation, found artifacts are excluded */
	    if(u.ulevel > 2 && u.uluck >= 0 
		    && (!flags.made_know || 
			 (uwep && (uwep->oartifact || uwep->oproperties || spec_prop_otyp(uwep)) && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)) && !(uwep->oproperties&OPROP_ACIDW))
		    ) && (
			 Role_if(PM_PRIEST) ? !rn2(10 + (2 * u.ugifts * u.ugifts)) : !rn2(10 + (2 * u.ugifts * nartifacts)) 
			)
		){
			if(uwep && (uwep->oartifact || uwep->oproperties || spec_prop_otyp(uwep)) && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)) && !(uwep->oproperties&OPROP_ACIDW)){
				if(!Blind) pline("Acid drips from your weapon!");
				uwep->oproperties |= OPROP_ACIDW;
				uwep->oeroded = 0;
				uwep->oeroded2 = 0;
				uwep->oerodeproof = 1;
				u.ugifts++;
			}
			else if(!flags.made_know){
				struct obj *otmp;
				otmp = mksobj(WORD_OF_KNOWLEDGE, FALSE, FALSE);
				dropy(otmp);
				at_your_feet("An object");
				u.ugifts++;
			}
			//Note: bugs in the above blocks were making ugifts go up without giving a benefit.
			//  I think the bugs are squashed, but keep the increment tightly associated with actual gifts.
			return;
	    } else if (rnl((30 + u.ulevel)*10) < 10) {
			/* no artifact, but maybe a helpful pet? */
			/* WAC is now some generic benefit (includes pets) */
			goat_gives_benefit();
		    return;
	    }
	    change_luck((value * LUCKMAX) / (MAXVALUE * 2));
	    if ((int)u.uluck < 0) u.uluck = 0;
	    if (u.uluck != saved_luck) {
		if (Blind)
		    You("think %s brushed your %s.",something, body_part(FOOT));
		else You(Hallucination ?
		    "see crabgrass at your %s.  A funny thing in a dungeon." :
		    "glimpse a four-leaf clover at your %s.",
		    makeplural(body_part(FOOT)));
	    }
		u.reconciled = REC_REC;
	}
    }
}


/*pray.c*/
