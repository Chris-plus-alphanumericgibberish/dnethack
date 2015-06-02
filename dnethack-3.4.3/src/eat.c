/*	SCCS Id: @(#)eat.c	3.4	2003/02/13	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
/* #define DEBUG */	/* uncomment to enable new eat code debugging */

#ifdef DEBUG
# ifdef WIZARD
#define debugpline	if (wizard) pline
# else
#define debugpline	pline
# endif
#endif

STATIC_PTR int NDECL(eatmdone);
STATIC_PTR int NDECL(eatfood);
STATIC_PTR void FDECL(costly_tin, (const char*));
STATIC_PTR int NDECL(opentin);
STATIC_PTR int NDECL(unfaint);
STATIC_PTR int NDECL(windclock);
STATIC_PTR int FDECL(clockwork_eat_menu, (boolean,boolean));

#ifdef OVLB
STATIC_DCL const char *FDECL(food_xname, (struct obj *,BOOLEAN_P));
STATIC_DCL void FDECL(choke, (struct obj *));
STATIC_DCL void NDECL(recalc_wt);
STATIC_DCL struct obj *FDECL(touchfood, (struct obj *));
STATIC_DCL void NDECL(do_reset_eat);
STATIC_DCL void FDECL(done_eating, (BOOLEAN_P));
STATIC_DCL int FDECL(intrinsic_possible, (int,struct permonst *));
STATIC_DCL void FDECL(givit, (int,struct permonst *, unsigned short, boolean));
STATIC_DCL void FDECL(start_tin, (struct obj *));
STATIC_DCL int FDECL(eatcorpse, (struct obj *));
STATIC_DCL void FDECL(start_eating, (struct obj *));
STATIC_DCL void FDECL(fprefx, (struct obj *));
STATIC_DCL void FDECL(accessory_has_effect, (struct obj *));
STATIC_DCL void FDECL(fpostfx, (struct obj *));
STATIC_DCL int NDECL(bite);
STATIC_DCL int FDECL(edibility_prompts, (struct obj *));
STATIC_DCL int FDECL(rottenfood, (struct obj *));
STATIC_DCL void NDECL(eatspecial);
STATIC_DCL void FDECL(eataccessory, (struct obj *));
STATIC_DCL const char *FDECL(foodword, (struct obj *));
STATIC_DCL boolean FDECL(maybe_cannibal, (int,BOOLEAN_P));

char msgbuf[BUFSZ];
int etype;			/* Clockwork's eat type */

#endif /* OVLB */

/* also used to see if you're allowed to eat cats and dogs */
#define CANNIBAL_ALLOWED() (Role_if(PM_CAVEMAN) || Race_if(PM_ORC) || \
		Race_if(PM_VAMPIRE))

#ifndef OVLB

STATIC_DCL NEARDATA const char comestibles[];
STATIC_DCL NEARDATA const char allobj[];
STATIC_DCL boolean force_save_hs;

#else

STATIC_OVL NEARDATA const char comestibles[] = { FOOD_CLASS, 0 };

/* Gold must come first for getobj(). */
STATIC_OVL NEARDATA const char allobj[] = {
	COIN_CLASS, WEAPON_CLASS, ARMOR_CLASS, POTION_CLASS, SCROLL_CLASS,
	WAND_CLASS, RING_CLASS, AMULET_CLASS, FOOD_CLASS, TOOL_CLASS,
	GEM_CLASS, ROCK_CLASS, BALL_CLASS, CHAIN_CLASS, SPBOOK_CLASS, /*BED_CLASS,*/ 0 };

STATIC_OVL boolean force_save_hs = FALSE;

const char *hu_stat[] = {
	"Satiated",
	"        ",
	"Hungry  ",
	"Weak    ",
	"Fainting",
	"Fainted ",
	"Starved "
};

const char *ca_hu_stat[] = {
	"OvrWound",
	"        ",
	"Waning  ",
	"Unwound ",
	"Slipping",
	"Slipped ",
	"Stopped "
};

#endif /* OVLB */
#ifdef OVL1

boolean
incantifier_edible(obj)
register struct obj *obj;
{
	/* protect invocation tools but not Rider corpses (handled elsewhere)*/
     /* if (obj->oclass != FOOD_CLASS && obj_resists(obj, 0, 0)) */
	 /* Repeating this check in the name of futureproofing */
	if (objects[obj->otyp].oc_unique || (objects[obj->otyp].oc_merge && 
										 obj->otyp != CORPSE && 
										 obj->otyp != TIN && 
										 obj->oclass != SCROLL_CLASS))
		return FALSE;
	
	if(obj->spe > 0){
		if(obj->oclass == WEAPON_CLASS) return TRUE;
		if(obj->oclass == ARMOR_CLASS) return TRUE;
		if(obj->oclass == TOOL_CLASS && is_weptool(obj)) return TRUE;
	}
	if(obj->oclass == SCROLL_CLASS && obj->otyp != SCR_BLANK_PAPER && obj->otyp != SCR_GOLD_SCROLL_OF_LAW && 
#ifdef MAIL
		obj->otyp != SCR_MAIL && 
#endif
		!obj->oartifact) return TRUE;
	if(obj->oclass == SPBOOK_CLASS && obj->otyp != SPE_BLANK_PAPER && !obj->oartifact) return TRUE;
	if(obj->oclass == AMULET_CLASS && obj->spe >= 0 && !obj->oartifact) return TRUE;
	if(obj->oclass == RING_CLASS && obj->spe >= 0 && !obj->oartifact) return TRUE;
	if(obj->oclass == WAND_CLASS && obj->spe > 0 && obj->otyp != WAN_NOTHING) return TRUE;
	if(obj->otyp == CORPSE && (obj->corpsenm == PM_AOA || obj->corpsenm == PM_AOA_DROPLET || obj->corpsenm == PM_NEWT)) return TRUE;
	if(obj->otyp == TIN && (!obj->known || obj->corpsenm == PM_AOA || obj->corpsenm == PM_AOA_DROPLET || obj->corpsenm == PM_NEWT)) return TRUE;
	
	return FALSE;
}

boolean
uclockwork_edible(obj)
struct obj *obj;
{
	/* protect invocation tools but not Rider corpses (handled elsewhere)*/
     /* if (obj->oclass != FOOD_CLASS && obj_resists(obj, 0, 0)) */
	 /* Repeating this check in the name of futureproofing */
	if (objects[obj->otyp].oc_unique)
		return FALSE;
	
	
	if(etype == SCRAP_MAW && is_metallic(obj) && is_corrodeable(obj) && !(obj->oartifact))
		return TRUE;
	
	if(etype == WOOD_STOVE && !(obj->oartifact)){
		if(obj->otyp == EUCALYPTUS_LEAF ||
		   obj->otyp == SPRIG_OF_WOLFSBANE ||
		   obj->otyp == FORTUNE_COOKIE ||
		   obj->otyp == LEMBAS_WAFER ||
		   obj->otyp == SHEAF_OF_HAY ||
		   obj->otyp == SEDGE_HAT ||
		   is_wood(obj) ||
		   (is_paper(obj) 
#ifdef MAIL
			&& obj->otyp != SCR_MAIL
#endif
		   )
		) return TRUE;
	}
	
	if(etype == HELLFIRE_FURNACE && !(obj->oartifact)){
		if(objects[obj->otyp].oc_material >= WAX && objects[obj->otyp].oc_material <= BONE 
#ifdef MAIL
			&& obj->otyp != SCR_MAIL
#endif
		)
			return TRUE;
	}
	
	if(etype == MAGIC_FURNACE && !(objects[obj->otyp].oc_merge && 
											  obj->otyp != CORPSE && 
											  obj->otyp != TIN && 
											  obj->oclass != SCROLL_CLASS)
	){
		if(obj->spe > 0){
			if(obj->oclass == WEAPON_CLASS) return TRUE;
			if(obj->oclass == ARMOR_CLASS) return TRUE;
			if(obj->oclass == TOOL_CLASS && is_weptool(obj)) return TRUE;
		}
		if(obj->oclass == SCROLL_CLASS && obj->otyp != SCR_BLANK_PAPER && obj->otyp != SCR_GOLD_SCROLL_OF_LAW && 
#ifdef MAIL
			obj->otyp != SCR_MAIL && 
#endif
			!obj->oartifact) return TRUE;
		if(obj->oclass == SPBOOK_CLASS && obj->otyp != SPE_BLANK_PAPER && !obj->oartifact) return TRUE;
		if(obj->oclass == AMULET_CLASS && obj->spe >= 0 && !obj->oartifact) return TRUE;
		if(obj->oclass == RING_CLASS && obj->spe >= 0 && !obj->oartifact) return TRUE;
		if(obj->oclass == WAND_CLASS && obj->spe > 0 && obj->otyp != WAN_NOTHING) return TRUE;
		if(obj->otyp == CORPSE && (obj->corpsenm == PM_AOA || obj->corpsenm == PM_AOA_DROPLET || obj->corpsenm == PM_NEWT)) return TRUE;
	}
	
	return FALSE;
}
/*
 * Decide whether a particular object can be eaten by the possibly
 * polymorphed character.  Not used for monster checks.
 */
boolean
is_edible(obj)
register struct obj *obj;
{
	/* protect invocation tools but not Rider corpses (handled elsewhere)*/
     /* if (obj->oclass != FOOD_CLASS && obj_resists(obj, 0, 0)) */
	if (objects[obj->otyp].oc_unique)
		return FALSE;
	/* above also prevents the Amulet from being eaten, so we must never
	   allow fake amulets to be eaten either [which is already the case] */

	if(Race_if(PM_INCANTIFIER)) return incantifier_edible(obj);
	if(uclockwork) return uclockwork_edible(obj);
	
	if (metallivorous(youmonst.data) && is_metallic(obj) &&
	    (youmonst.data != &mons[PM_RUST_MONSTER] || is_rustprone(obj)))
		return TRUE;
	if (u.umonnum == PM_GELATINOUS_CUBE && is_organic(obj) &&
		/* [g.cubes can eat containers and retain all contents
		    as engulfed items, but poly'd player can't do that] */
	    !Has_contents(obj))
		return TRUE;

	/* a sheaf of straw is VEGGY, but only edible for herbivorous animals */
	if ((obj->otyp == SHEAF_OF_HAY || obj->otyp == SEDGE_HAT) && herbivorous(youmonst.data))
		return !carnivorous(youmonst.data);
	if (herbivorous(youmonst.data) && is_veggy(obj))
		return TRUE;
		
	/* Ghouls only eat corpses */
	if (u.umonnum == PM_GHOUL)
	   	return (boolean)(obj->otyp == CORPSE);
	/* Vampires drink the blood of meaty corpses */
	/* [ALI] (fully) drained food is not presented as an option,
	 * but partly eaten food is (even though you can't drain it).
	 */
	if (maybe_polyd(is_vampire(youmonst.data), Race_if(PM_VAMPIRE)))
		return (boolean)(obj->otyp == CORPSE &&
		  has_blood(&mons[obj->corpsenm]) && (!obj->odrained ||
		  obj->oeaten > drainlevel(obj)));

     /* return((boolean)(!!index(comestibles, obj->oclass))); */
	return (boolean)(obj->oclass == FOOD_CLASS);
}

#endif /* OVL1 */
#ifdef OVLB

void
init_uhunger()
{
	if(Race_if(PM_INCANTIFIER)){
		u.uenmax += 900;
		u.uen = u.uenmax*.45;
	} else {
		u.uhungermax = DEFAULT_HMAX;
		u.uhunger = u.uhungermax*.45;
	}
	u.uhs = NOT_HUNGRY;
}

void
reset_uhunger()
{
	if(Race_if(PM_INCANTIFIER)){
		u.uen = u.uenmax*.45;
	} else {
		u.uhunger = u.uhungermax*.45;
	}
	u.uhs = NOT_HUNGRY;
}

static const struct { const char *txt; int nut; } tintxts[] = {
	{"deep fried",	 60},
	{"pickled",	 40},
	{"soup made from", 20},
	{"pureed",	500},
#define ROTTEN_TIN 4
	{"rotten",	-50},
#define HOMEMADE_TIN 5
	{"homemade",	 50},
	{"stir fried",   80},
	{"candied",      100},
	{"boiled",       50},
#define DRIED_TIN 9
	{"dried",        55},
	{"szechuan",     70},
#define FRENCH_FRIED_TIN 11
	{"french fried", 40},
	{"sauteed",      95},
	{"broiled",      80},
	{"smoked",       50},
	{"", 0}
};
#define TTSZ	SIZE(tintxts)

static NEARDATA struct {
	struct	obj *tin;
	int	usedtime, reqtime;
} tin;

static NEARDATA struct {
	struct	obj *piece;	/* the thing being eaten, or last thing that
				 * was partially eaten, unless that thing was
				 * a tin, which uses the tin structure above,
				 * in which case this should be 0 */
	/* doeat() initializes these when piece is valid */
	int	usedtime,	/* turns spent eating */
		reqtime;	/* turns required to eat */
	int	nmod;		/* coded nutrition per turn */
	struct	monst *mon; /* monster associated with item */
	Bitfield(canchoke,1);	/* was satiated at beginning */

	/* start_eating() initializes these */
	Bitfield(fullwarn,1);	/* have warned about being full */
	Bitfield(eating,1);	/* victual currently being eaten */
	Bitfield(doreset,1);	/* stop eating at end of turn */
} victual;

static char *eatmbuf = 0;	/* set by cpostfx() */

STATIC_PTR
int
eatmdone()		/* called after mimicing is over */
{
	/* release `eatmbuf' */
	if (eatmbuf) {
	    if (nomovemsg == eatmbuf) nomovemsg = 0;
	    free((genericptr_t)eatmbuf),  eatmbuf = 0;
	}
	/* update display */
	if (youmonst.m_ap_type) {
	    youmonst.m_ap_type = M_AP_NOTHING;
	    newsym(u.ux,u.uy);
	}
	return 0;
}

/* ``[the(] singular(food, xname) [)]'' with awareness of unique monsters */
STATIC_OVL const char *
food_xname(food, the_pfx)
struct obj *food;
boolean the_pfx;
{
	const char *result;
	int mnum = food->corpsenm;

	if (food->otyp == CORPSE && (mons[mnum].geno & G_UNIQ)) {
	    /* grab xname()'s modifiable return buffer for our own use */
	    char *bufp = xname(food);
	    Sprintf(bufp, "%s%s corpse",
		    (the_pfx && !type_is_pname(&mons[mnum])) ? "the " : "",
		    s_suffix(mons[mnum].mname));
	    result = bufp;
	} else {
	    /* the ordinary case */
	    result = singular(food, xname);
	    if (the_pfx) result = the(result);
	}
	return result;
}

/* Created by GAN 01/28/87
 * Amended by AKP 09/22/87: if not hard, don't choke, just vomit.
 * Amended by 3.  06/12/89: if not hard, sometimes choke anyway, to keep risk.
 *		  11/10/89: if hard, rarely vomit anyway, for slim chance.
 */
STATIC_OVL void
choke(food)	/* To a full belly all food is bad. (It.) */
	register struct obj *food;
{
	/* only happens if you were satiated */
	if (u.uhs != SATIATED) {
		if (!food || food->otyp != AMULET_OF_STRANGULATION)
			return;
	} else if (Role_if(PM_KNIGHT) && u.ualign.type == A_LAWFUL) {
			adjalign(-5);		/* gluttony is unchivalrous */ //stiffer penalty
			You_feel("like a glutton!");
	}

	exercise(A_CON, FALSE);

	if ((Breathless && !Race_if(PM_INCANTIFIER)) || (!Strangled && !rn2(20))) {
		/* choking by eating AoS doesn't involve stuffing yourself */
		if (food && food->otyp == AMULET_OF_STRANGULATION) {
			You("choke, but recover your composure.");
			return;
		}
		You("stuff yourself and then vomit voluminously.");
		morehungry(1000);	/* you just got *very* sick! */
		nomovemsg = 0;
		vomit();
	} else {
	 if(Race_if(PM_INCANTIFIER)){
		killer_format = KILLED_BY;
		You("absorb too much energy and explode.");
		killer = Hallucination ? "amateur-hour horseshit" : "absorbing too much energy and exploding"; //8-bit theater
		You("die...");
		done(DISINTEGRATED);
		explode(u.ux, u.uy, 0, u.uhpmax/2, MON_EXPLODE, EXPL_MAGICAL);
		u.uhp = u.uhpmax/2;
		pline("You reform!");
		morehungry(u.uenmax/2);	/* lifesaved */
	 } else{
		killer_format = KILLED_BY_AN;
		/*
		 * Note all "killer"s below read "Choked on %s" on the
		 * high score list & tombstone.  So plan accordingly.
		 */
		if(food) {
			You("choke over your %s.", foodword(food));
			if (food->oclass == COIN_CLASS) {
				killer = "a very rich meal";
			} else {
				killer = food_xname(food, FALSE);
				if (food->otyp == CORPSE &&
				    (mons[food->corpsenm].geno & G_UNIQ)) {
				    if (!type_is_pname(&mons[food->corpsenm]))
					killer = the(killer);
				    killer_format = KILLED_BY;
				}
			}
		} else {
			You("choke over it.");
			killer = "quick snack";
		}
		You("die...");
		done(CHOKING);
	 }
	}
}

/* modify object wt. depending on time spent consuming it */
STATIC_OVL void
recalc_wt()
{
	struct obj *piece = victual.piece;

#ifdef DEBUG
	debugpline("Old weight = %d", piece->owt);
	debugpline("Used time = %d, Req'd time = %d",
		victual.usedtime, victual.reqtime);
#endif
	piece->owt = weight(piece);
#ifdef DEBUG
	debugpline("New weight = %d", piece->owt);
#endif
}

void
reset_eat()		/* called when eating interrupted by an event */
{
    /* we only set a flag here - the actual reset process is done after
     * the round is spent eating.
     */
	if(victual.eating && !victual.doreset) {
#ifdef DEBUG
	    debugpline("reset_eat...");
#endif
	    victual.doreset = TRUE;
	}
	return;
}

STATIC_OVL struct obj *
touchfood(otmp)
register struct obj *otmp;
{
	struct obj *pseudo;
	if (otmp->quan > 1L) {
	    if(!carried(otmp))
		(void) splitobj(otmp, otmp->quan - 1L);
	    else
		otmp = splitobj(otmp, 1L);
#ifdef DEBUG
	    debugpline("split object,");
#endif
	}

	if (!otmp->oeaten) {
	    if(((!carried(otmp) && costly_spot(otmp->ox, otmp->oy) &&
		 !otmp->no_charge)
		 || otmp->unpaid)) {
		/* create a dummy duplicate to put on bill */
		verbalize("You bit it, you bought it!");
		bill_dummy_object(otmp);
	    }
		if((!(u.uconduct.unvegetarian) || Role_if(PM_MONK)) && (otmp->otyp == K_RATION || otmp->otyp == C_RATION)){
			You("find that this military ration has a meat serving.");
			if(yn("Eat the meat?") == 'n'){
				You("discard the meat.");
			    otmp->oeaten = objects[otmp->otyp].oc_nutrition / 4;
			}
			else{
				You("eat the meat.");
			    otmp->oeaten = objects[otmp->otyp].oc_nutrition;
			    u.uconduct.unvegan++;
			    violated_vegetarian();
			}
		}
		else if(otmp->otyp == CANDY_BAR){
			if(!rn2(100)){
				You("find this candy bar to be of rare quality!");
			    otmp->oeaten = 10 * objects[otmp->otyp].oc_nutrition;
				pluslvl(FALSE);/*first bite gives level up, also has a lot of nutriton*/

			    if(((!carried(otmp) && costly_spot(otmp->ox, otmp->oy) &&
				 !otmp->no_charge)
				 || otmp->unpaid)) {
					/* create a dummy duplicate to put on bill */
					verbalize("Hey, that was really rare!");
					pseudo = mksobj(POT_GAIN_LEVEL, FALSE, FALSE);
					pseudo->blessed = pseudo->cursed = 0;
					bill_dummy_object(pseudo);
					obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
				}
			}
			else otmp->oeaten = objects[otmp->otyp].oc_nutrition;
		}
	    else otmp->oeaten = (otmp->otyp == CORPSE ?
				mons[otmp->corpsenm].cnutrit :
				objects[otmp->otyp].oc_nutrition);
	}

	if (carried(otmp)) {
	    freeinv(otmp);
	    if (inv_cnt() >= 52) {
		sellobj_state(SELL_DONTSELL);
		dropy(otmp);
		sellobj_state(SELL_NORMAL);
	    } else {
		otmp->oxlth++;		/* hack to prevent merge */
		otmp = addinv(otmp);
		otmp->oxlth--;
	    }
	}
	return(otmp);
}

/* When food decays, in the middle of your meal, we don't want to dereference
 * any dangling pointers, so set it to null (which should still trigger
 * do_reset_eat() at the beginning of eatfood()) and check for null pointers
 * in do_reset_eat().
 */
void
food_disappears(obj)
register struct obj *obj;
{
	if (obj == victual.piece) victual.piece = (struct obj *)0;
	if (obj->timed) obj_stop_timers(obj);
}

/* renaming an object usually results in it having a different address;
   so the sequence start eating/opening, get interrupted, name the food,
   resume eating/opening would restart from scratch */
void
food_substitution(old_obj, new_obj)
struct obj *old_obj, *new_obj;
{
	if (old_obj == victual.piece) victual.piece = new_obj;
	if (old_obj == tin.tin) tin.tin = new_obj;
}

STATIC_OVL void
do_reset_eat()
{
#ifdef DEBUG
	debugpline("do_reset_eat...");
#endif
	if (victual.piece) {
		victual.piece = touchfood(victual.piece);
		recalc_wt();
	}
	victual.fullwarn = victual.eating = victual.doreset = FALSE;
	/* Do not set canchoke to FALSE; if we continue eating the same object
	 * we need to know if canchoke was set when they started eating it the
	 * previous time.  And if we don't continue eating the same object
	 * canchoke always gets recalculated anyway.
	 */
	stop_occupation();
	newuhs(FALSE);
}

STATIC_PTR
int
eatfood()		/* called each move during eating process */
{
	if(!victual.piece ||
	 (!carried(victual.piece) && !obj_here(victual.piece, u.ux, u.uy))) {
		/* maybe it was stolen? */
		do_reset_eat();
		return(0);
	}
	if (maybe_polyd(is_vampire(youmonst.data), 
			Race_if(PM_VAMPIRE)) != victual.piece->odrained) {
	    /* Polymorphed while eating/draining */
	    do_reset_eat();
	    return(0);
	}
	if(!victual.eating) return(0);

	if(++victual.usedtime <= victual.reqtime) {
	    if(bite()) return(0);
	    return(1);	/* still busy */
	} else {	/* done */
	    int crumbs = victual.piece->oeaten;		/* The last crumbs */
	    if (victual.piece->odrained) crumbs -= drainlevel(victual.piece);
	    if (crumbs > 0) {
			lesshungry(crumbs);
			victual.piece->oeaten -= crumbs;
	    }
	    done_eating(TRUE);
	    return(0);
	}
}

STATIC_OVL void
done_eating(message)
boolean message;
{
	victual.piece->in_use = TRUE;
	occupation = 0; /* do this early, so newuhs() knows we're done */
	newuhs(FALSE);
	if (nomovemsg) {
		if (message) pline(nomovemsg);
		nomovemsg = 0;
	} else if (message)
		You("finish %s %s.", victual.piece->odrained ? "draining" :
		  "eating", food_xname(victual.piece, TRUE));

	if(victual.piece->otyp == CORPSE) {
		cpostfx(victual.piece->corpsenm, FALSE, FALSE, victual.piece->odrained);
	} else
		fpostfx(victual.piece);

	if (victual.piece->odrained)
		victual.piece->in_use = FALSE;
	else
	if (carried(victual.piece)) useup(victual.piece);
	else useupf(victual.piece, 1L);
	victual.piece = (struct obj *) 0;
	victual.fullwarn = victual.eating = victual.doreset = FALSE;
}

STATIC_OVL boolean
maybe_cannibal(pm, allowmsg)
int pm;
boolean allowmsg;
{
	if(your_race(&mons[pm])){
		if (!CANNIBAL_ALLOWED()) {
			if (allowmsg) {
				if (Upolyd)
					You("have a bad feeling deep inside.");
				You("cannibal!  You will regret this!");
			}
			HAggravate_monster |= FROMOUTSIDE;
			change_luck(-rn1(4,2));		/* -5..-2 */
		} else if (Role_if(PM_CAVEMAN)) {
			adjalign(sgn(u.ualign.type));
			You("honour the dead.");
		} else {
			adjalign(-sgn(u.ualign.type));
			You_feel("evil and fiendish!");
		}
		return TRUE;
	}
	return FALSE;
}

void
cprefx(pm,bld,nobadeffects)
int pm;
BOOLEAN_P bld, nobadeffects;
{
	if(!nobadeffects) maybe_cannibal(pm,TRUE);
	if (!nobadeffects && (touch_petrifies(&mons[pm]) || pm == PM_MEDUSA)) {
	    if (!Stone_resistance &&
		!(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
		Sprintf(killer_buf, "tasting %s %s", mons[pm].mname, bld ? "blood" : "meat");
		killer_format = KILLED_BY;
		killer = killer_buf;
		You("turn to stone.");
		done(STONING);
		if (victual.piece)
		    victual.eating = FALSE;
		return; /* lifesaved */
	    }
	}

	switch(pm) {
	    case PM_LITTLE_DOG:
	    case PM_DOG:
	    case PM_LARGE_DOG:
	    case PM_KITTEN:
	    case PM_HOUSECAT:
	    case PM_LARGE_CAT:
		if (!nobadeffects && !CANNIBAL_ALLOWED()) {
		    You_feel("that %s the %s%s%s was a bad idea.",
		      victual.eating ? "eating" : bld ? "drinking" : "biting",
		      occupation == opentin ? "tinned " : "", mons[pm].mname,
			  bld ? " blood" : "");
		    HAggravate_monster |= FROMOUTSIDE;
		}
		break;
	    case PM_LIZARD:
	    case PM_SMALL_CAVE_LIZARD:
	    case PM_CAVE_LIZARD:
	    case PM_LARGE_CAVE_LIZARD:
			if (Stoned) fix_petrification();
		break;
	    case PM_MANDRAKE:
			if(!nobadeffects){
				pline ("Oh wow!  Great stuff!");
				make_hallucinated(HHallucination + 200,FALSE,0L);
			}
			if (Stoned) fix_petrification();
			make_sick(0L, (char *) 0, TRUE, SICK_ALL);
		break;
		/*Note: these three imply corpse*/
	    case PM_GREAT_CTHULHU:
	    case PM_DEATH:
	    case PM_PESTILENCE:
	    case PM_FAMINE:
		{ char buf[BUFSZ];
			if(!nobadeffects){
				pline("Eating that is instantly fatal.");
				Sprintf(buf, "unwisely ate the body of %s",
					mons[pm].mname);
				killer = buf;
				killer_format = NO_KILLER_PREFIX;
				done(DIED);
			}
		    /* It so happens that since we know these monsters */
		    /* cannot appear in tins, victual.piece will always */
		    /* be what we want, which is not generally true. */
		    if (revive_corpse(victual.piece, REVIVE_MONSTER))
			victual.piece = (struct obj *)0;
		    return;
		}
		case PM_AXUS:
		case PM_NAZGUL:
		case PM_ELDER_PRIEST:
		case PM_PRIEST_OF_AN_UNKNOWN_GOD:
		    /* It so happens that since we know these monsters */
		    /* cannot appear in tins, victual.piece will always */
		    /* be what we want, which is not generally true. */
		    if (revive_corpse(victual.piece, REVIVE_MONSTER))
			victual.piece = (struct obj *)0;
		    return;
		break;
	    case PM_GREEN_SLIME:
	    case PM_FLUX_SLIME:
		if (!nobadeffects && !Slimed && !Unchanging && !flaming(youmonst.data) &&
			youmonst.data != &mons[PM_GREEN_SLIME]) {
		    You("don't feel very well.");
		    Slimed = 10L;
		    flags.botl = 1;
		}
		/* Fall through */
	    default:
		if (acidic(&mons[pm]) && Stoned)
		    fix_petrification();
		break;
	}
}

/*
 * Called when a vampire bites a monster.
 * Returns TRUE if hero died and was lifesaved.
 */

boolean
bite_monster(mon)
struct monst *mon;
{
    switch(monsndx(mon->data)) {
	case PM_LIZARD:
	case PM_SMALL_CAVE_LIZARD:
	case PM_CAVE_LIZARD:
	case PM_LARGE_CAVE_LIZARD:
	    if (Stoned) fix_petrification();
	    break;
	case PM_DEATH:
	case PM_PESTILENCE:
	case PM_FAMINE:
	    pline("Unfortunately, eating any of it is fatal.");
	    done_in_by(mon);
	    return TRUE;		/* lifesaved */

	case PM_GREEN_SLIME:
	case PM_FLUX_SLIME:
	    if (!Unchanging && youmonst.data != &mons[PM_FIRE_VORTEX] &&
			    youmonst.data != &mons[PM_FIRE_ELEMENTAL] &&
			    youmonst.data != &mons[PM_GREEN_SLIME]) {
		You("don't feel very well.");
		Slimed = 10L;
	    }
	    /* Fall through */
	default:
	    if (acidic(mon->data) && Stoned)
		fix_petrification();
	    break;
    }
    return FALSE;
}

void
fix_petrification()
{
	Stoned = 0;
	delayed_killer = 0;
	if (Hallucination)
	    pline("What a pity - you just ruined a future piece of %sart!",
		  ACURR(A_CHA) > 15 ? "fine " : "");
	else
	    You_feel("limber!");
}

/*
 * If you add an intrinsic that can be gotten by eating a monster, add it
 * to intrinsic_possible() and givit().  (It must already be in prop.h to
 * be an intrinsic property.)
 * It would be very easy to make the intrinsics not try to give you one
 * that you already had by checking to see if you have it in
 * intrinsic_possible() instead of givit().
 */

/* intrinsic_possible() returns TRUE iff a monster can give an intrinsic. */
STATIC_OVL int
intrinsic_possible(type, ptr)
int type;
register struct permonst *ptr;
{
	switch (type) {
	    case FIRE_RES:
#ifdef DEBUG
		if (ptr->mconveys & MR_FIRE) {
			debugpline("can get fire resistance");
			return(TRUE);
		} else  return(FALSE);
#else
		return(ptr->mconveys & MR_FIRE);
#endif
	    case SLEEP_RES:
#ifdef DEBUG
		if (ptr->mconveys & MR_SLEEP) {
			debugpline("can get sleep resistance");
			return(TRUE);
		} else  return(FALSE);
#else
		return(ptr->mconveys & MR_SLEEP);
#endif
	    case COLD_RES:
#ifdef DEBUG
		if (ptr->mconveys & MR_COLD) {
			debugpline("can get cold resistance");
			return(TRUE);
		} else  return(FALSE);
#else
		return(ptr->mconveys & MR_COLD);
#endif
	    case ACID_RES:
#ifdef DEBUG
		if (ptr->mconveys & MR_ACID) {
			debugpline("can get acid resistance");
			return(TRUE);
		} else  return(FALSE);
#else
		return(ptr->mconveys & MR_ACID);
#endif
	    case DISINT_RES:
#ifdef DEBUG
		if (ptr->mconveys & MR_DISINT) {
			debugpline("can get disintegration resistance");
			return(TRUE);
		} else  return(FALSE);
#else
		return(ptr->mconveys & MR_DISINT);
#endif
	    case SHOCK_RES:	/* shock (electricity) resistance */
#ifdef DEBUG
		if (ptr->mconveys & MR_ELEC) {
			debugpline("can get shock resistance");
			return(TRUE);
		} else  return(FALSE);
#else
		return(ptr->mconveys & MR_ELEC);
#endif
	    case POISON_RES:
#ifdef DEBUG
		if (ptr->mconveys & MR_POISON) {
			debugpline("can get poison resistance");
			return(TRUE);
		} else  return(FALSE);
#else
		return(ptr->mconveys & MR_POISON);
#endif
	    case TELEPORT:
#ifdef DEBUG
		if (can_teleport(ptr)) {
			debugpline("can get teleport");
			return(TRUE);
		} else  return(FALSE);
#else
		return(can_teleport(ptr));
#endif
	    case DISPLACED:
#ifdef DEBUG
		if (is_displacer(ptr)) {
			debugpline("can displacement");
			return(TRUE);
		} else  return(FALSE);
#else
		return(is_displacer(ptr));
#endif
	    case TELEPORT_CONTROL:
#ifdef DEBUG
		if (control_teleport(ptr)) {
			debugpline("can get teleport control");
			return(TRUE);
		} else  return(FALSE);
#else
		return(control_teleport(ptr));
#endif
	    case TELEPAT:
#ifdef DEBUG
		if (telepathic(ptr)) {
			debugpline("can get telepathy");
			return(TRUE);
		} else  return(FALSE);
#else
		return(telepathic(ptr));
#endif
	    default:
		return(FALSE);
	}
	/*NOTREACHED*/
}

/* givit() tries to give you an intrinsic based on the monster's level
 * and what type of intrinsic it is trying to give you.
 * Energy resistence intrinsics time out now - CM
 */
STATIC_OVL void
givit(type, ptr, nutval, drained)
int type;
register struct permonst *ptr;
unsigned short nutval;
boolean drained;
{
	int chance = 0; //starts at 0. Changing it indicates a non-energy resistence
	int multiplier = 1;
	char permanent = 0;

#ifdef DEBUG
	debugpline("Attempting to give intrinsic %d", type);
#endif
	/* some intrinsics are easier to get than others */
	switch (type) {
		case FIRE_RES:
		case SLEEP_RES:
		case COLD_RES:
		case SHOCK_RES:
		case ACID_RES:
			chance = 0;//skip roll, give atribute.
		break;
		case POISON_RES:
			if ((ptr == &mons[PM_KILLER_BEE] ||
					ptr == &mons[PM_SCORPION]) && !rn2(4))
				chance = 1;
			else
				chance = 15;
			break;
		case DISPLACED:
			chance = ptr == &mons[PM_SHIMMERING_DRAGON] ? 0 : 10;
			break;
		case TELEPORT:
			chance = 10;
			break;
		case TELEPORT_CONTROL:
			chance = 12;
			break;
		case TELEPAT:
			chance = 1;
			break;
		case DISINT_RES:
			chance = 15;
			break;
		default:
			chance = 15;
			break;
	}
	
	if(chance && drained && rn2(5))
		return;		/* drain chance */
	
	if (chance && ptr->mlevel <= rn2(chance))
		return;		/* failed die roll */

	if(ptr->mlevel < 5) multiplier = 2;
	else if (ptr->mlevel < 10) multiplier = 4;
	else if (ptr->mlevel < 15) multiplier = 6;
	else multiplier = 10;
	if(ptr->geno & G_UNIQ) multiplier = 20;
	if(ptr->geno & G_UNIQ && ptr->mlevel > 14) permanent = 1;
	switch (type) {
	    case FIRE_RES:
#ifdef DEBUG
		debugpline("Trying to give fire resistance");
#endif
		if( !(HFire_resistance) ) {
			You(Hallucination ? "be chillin'." :
			    "feel a momentary chill.");
		}
		if( (HFire_resistance & TIMEOUT) + (long)(nutval*multiplier) < TIMEOUT) {
			long timer = max((HFire_resistance & TIMEOUT), (long)(nutval*multiplier));
			HFire_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HFire_resistance |= timer; //set new timer
		}
		else{
			HFire_resistance |= TIMEOUT; //set timer to max value
		}
		if(permanent){
			HFire_resistance |= FROMOUTSIDE;
		}
		break;
	    case SLEEP_RES:
#ifdef DEBUG
		debugpline("Trying to give sleep resistance");
#endif
		if( !(HSleep_resistance) ) {
			You_feel("wide awake.");
		}
		if( (HSleep_resistance & TIMEOUT) + (long)(nutval*multiplier) < TIMEOUT) {
			long timer = max((HSleep_resistance & TIMEOUT), (long)(nutval*multiplier));
			HSleep_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HSleep_resistance |= timer; //set new timer
		}
		else{
			HSleep_resistance |= TIMEOUT; //set timer to max value
		}
		if(permanent){
			HSleep_resistance |= FROMOUTSIDE;
		}
		break;
	    case COLD_RES:
#ifdef DEBUG
		debugpline("Trying to give cold resistance");
#endif
		if( !(HCold_resistance) ) {
			You_feel("full of hot air.");
		}
		if( (HCold_resistance & TIMEOUT) + (long)(nutval*multiplier) < TIMEOUT) {
			long timer = max((HCold_resistance & TIMEOUT), (long)(nutval*multiplier));
			HCold_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HCold_resistance |= timer; //set new timer
		}
		else{
			HCold_resistance |= TIMEOUT; //set timer to max value
		}
		if(permanent){
			HCold_resistance |= FROMOUTSIDE;
		}
		break;
	    case DISINT_RES:
#ifdef DEBUG
		debugpline("Trying to give disintegration resistance");
#endif
		if(!(HDisint_resistance & FROMOUTSIDE)) {
			You_feel(Hallucination ?
			    "totally together, man." :
			    "very firm.");
			HDisint_resistance |= FROMOUTSIDE;
		}
		break;
	    case SHOCK_RES:	/* shock (electricity) resistance */
#ifdef DEBUG
		debugpline("Trying to give shock resistance");
#endif
		if( !(HShock_resistance) ) {
			if (Hallucination)
				rn2(2) ? You_feel("grounded in reality.") : Your("health currently feels amplified!");
			else
				You_feel("well grounded.");
		}
		if( (HShock_resistance & TIMEOUT) + (long)(nutval*multiplier) < TIMEOUT) {
			long timer = max((HShock_resistance & TIMEOUT), (long)(nutval*multiplier));
			HShock_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HShock_resistance |= timer; //set new timer
		}
		else{
			HShock_resistance |= TIMEOUT; //set timer to max value
		}
		if(permanent){
			HShock_resistance |= FROMOUTSIDE;
		}
		break;
	    case ACID_RES:	/* acid resistance */
#ifdef DEBUG
		debugpline("Trying to give acid resistance");
#endif
		if( !(HAcid_resistance) ) {
			if (Hallucination)
				rn2(2) ? You_feel("like you've really gotten back to basics!") : You_feel("insoluble.");
			else
				Your("skin feels leathery.");
		}
		if( (HAcid_resistance & TIMEOUT) + (long)(nutval*multiplier) < TIMEOUT) {
			long timer = max((HAcid_resistance & TIMEOUT), (long)(nutval*multiplier));
			HAcid_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HAcid_resistance |= timer; //set new timer
		}
		else{
			HAcid_resistance |= TIMEOUT; //set timer to max value
		}
		if(permanent){
			HAcid_resistance |= FROMOUTSIDE;
		}
		break;
	    case POISON_RES:
#ifdef DEBUG
		debugpline("Trying to give poison resistance");
#endif
		if(!(HPoison_resistance & FROMOUTSIDE)) {
			You_feel(Poison_resistance ?
				 "especially healthy." : "healthy.");
			HPoison_resistance |= FROMOUTSIDE;
		}
		break;
	    case DISPLACED:	/* displacement resistance */
		//Note that intrinsic displacemnt disregards the timeout multiplier and permanence setting.
#ifdef DEBUG
		debugpline("Trying to give intrinsic displacement");
#endif
		if( !(HDisplaced) ) {
			if (Hallucination)
				You_feel("quite beside yourself!");
			else
				Your("outline shimmers and shifts.");
		}
		if( (HDisplaced & TIMEOUT) + (long)(nutval) < TIMEOUT) {
			long timer = (HDisplaced & TIMEOUT) + (long)(nutval);
			HDisplaced &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HDisplaced |= timer; //set new timer
		}
		else{
			HDisplaced |= TIMEOUT; //set timer to max value
		}
		// if(permanent){
			// HDisplaced |= FROMOUTSIDE;
		// }
		break;
	    case TELEPORT:
#ifdef DEBUG
		debugpline("Trying to give teleport");
#endif
		if(!(HTeleportation & FROMOUTSIDE)) {
			You_feel(Hallucination ? "diffuse." :
			    "very jumpy.");
			HTeleportation |= FROMOUTSIDE;
		}
		break;
	    case TELEPORT_CONTROL:
#ifdef DEBUG
		debugpline("Trying to give teleport control");
#endif
		if(!(HTeleport_control & FROMOUTSIDE)) {
			You_feel(Hallucination ?
			    "centered in your personal space." :
			    "in control of yourself.");
			HTeleport_control |= FROMOUTSIDE;
		}
		break;
	    case TELEPAT:
#ifdef DEBUG
		debugpline("Trying to give telepathy");
#endif
		if(!(HTelepat & FROMOUTSIDE)) {
			You_feel(Hallucination ?
			    "in touch with the cosmos." :
			    "a strange mental acuity.");
			HTelepat |= FROMOUTSIDE;
			/* If blind, make sure monsters show up. */
			if (Blind) see_monsters();
		}
		break;
	    default:
#ifdef DEBUG
		debugpline("Tried to give an impossible intrinsic");
#endif
		break;
	}
}

void
cpostfx(pm, tin, nobadeffects, drained)		/* called after completely consuming a corpse */
register int pm;
BOOLEAN_P tin, nobadeffects, drained;
{
	register int tmp = 0;
	boolean catch_lycanthropy = FALSE;

	/* in case `afternmv' didn't get called for previously mimicking
	   gold, clean up now to avoid `eatmbuf' memory leak */
	if (eatmbuf) (void)eatmdone();

	switch(pm) {
	    case PM_NEWT:
		/* MRKR: "eye of newt" may give small magical energy boost */
		if(!drained || !rn2(5)){
			int old_uen = u.uen;
			u.uen += d(2,10);
			if (u.uen > u.uenmax){
				u.uenmax++;
				u.uen = u.uenmax;
			}
			if (old_uen != u.uen){
				You_feel("a mild buzz.");
				flags.botl = 1;
			}
		}
		break;
	    case PM_AOA_DROPLET: //Aoas are drops of pure magic
		if(!drained || !rn2(5)) if (rn2(3) || 3 * u.uen <= 2 * u.uenmax) {
		    int old_uen = u.uen;
		    u.uen += d(4,10);
		    if (u.uen > u.uenmax) {
				u.uenmax+=4;
				u.uen = u.uenmax;
		    }
		    if (old_uen != u.uen) {
			    You_feel("a strong buzzing sensation.");
			    flags.botl = 1;
		    }
		}
		break;
	    case PM_AOA: //Aoas are drops of pure magic
		if(!drained || !rn2(5)) if (rn2(3) || 3 * u.uen <= 2 * u.uenmax) {
		    int old_uen = u.uen;
			int bonus = d(6,10);
			u.uen = u.uen + 10 > (u.uenmax - 40) ? u.uen + 10 : (u.uenmax - 40);
		    u.uen += bonus;
		    if (u.uen > u.uenmax) {
				u.uenmax+=10;
				u.uen = u.uenmax;
		    }
		    if (old_uen != u.uen) {
			    You_feel("a great jolt run through your mind!");
			    flags.botl = 1;
		    }
		}
		break;
	    case PM_WRAITH:
			if(!drained || !rn2(5)) pluslvl(FALSE);
		break;
	    case PM_DEEP_DRAGON:
			if(!drained || !rn2(5)) pluslvl(FALSE);
		break;
	    case PM_HUMAN_WERERAT:
			if(!nobadeffects){
				catch_lycanthropy = TRUE;
				u.ulycn = PM_WERERAT;
			}
		break;
	    case PM_HUMAN_WEREJACKAL:
			if(!nobadeffects){
				catch_lycanthropy = TRUE;
				u.ulycn = PM_WEREJACKAL;
			}
		break;
	    case PM_HUMAN_WEREWOLF:
			if(!nobadeffects){
				catch_lycanthropy = TRUE;
				u.ulycn = PM_WEREWOLF;
			}
		break;
	    case PM_ANUBITE:
			if(!nobadeffects){
				catch_lycanthropy = TRUE;
				u.ulycn = PM_WEREJACKAL;
			}
		break;
	    case PM_NURSE:
			if (Upolyd) u.mh = u.mhmax;
			else u.uhp = u.uhpmax;
			flags.botl = 1;
		break;
		case PM_SEWER_RAT:
			if(!nobadeffects && !drained){
				if(d(1,10) > 9 && !Sick_resistance) make_vomiting(Vomiting+d(10,4), TRUE);
			}
		break;
		case PM_RABID_RAT:
			if(!nobadeffects){
			if(!rn2(20)){
				if (!Sick_resistance) {
					char buf[BUFSZ];
					long sick_time;
	
					sick_time = (long) d(10, 100)+1000;
					/* make sure new illness doesn't result in improvement */
					if (Sick && (sick_time > Sick))
						sick_time = (Sick > 1L) ? Sick - 1L : 1L;
						Sprintf(buf, "%s%s diseased corpse",
							!type_is_pname(&mons[pm]) ? "the " : "",
							s_suffix(mons[pm].mname));
						make_sick(sick_time, buf, FALSE, SICK_NONVOMITABLE);
				}
			}
			}
		break;
	    case PM_STALKER:
			if(!drained || !rn2(5)) if(!nobadeffects){
				if(!Invis) {
					set_itimeout(&HInvis, (long)rn1(100, 50));
					if (!Blind && !BInvis) self_invis_message();
				} else {
					if (!(HInvis & INTRINSIC)) You_feel("hidden!");
					HInvis |= FROMOUTSIDE;
					HSee_invisible |= FROMOUTSIDE;
				}
				newsym(u.ux, u.uy);
			}
		/* fall into next case */
	    case PM_YELLOW_LIGHT:
		/* fall into next case */
	    case PM_GIANT_BAT:
			if(!nobadeffects) make_stunned(HStun + 30,FALSE);
		/* fall into next case */
	    case PM_BAT:
			if(!nobadeffects) make_stunned(HStun + 30,FALSE);
		break;
	    case PM_GIANT_MIMIC:
		tmp += 10;
		/* fall into next case */
	    case PM_LARGE_MIMIC:
		tmp += 20;
		/* fall into next case */
	    case PM_SMALL_MIMIC:
		tmp += 20;
		if (!nobadeffects && youmonst.data->mlet != S_MIMIC && !Unchanging) {
		    char buf[BUFSZ];
		    You_cant("resist the temptation to mimic %s.",
			Hallucination ? "an orange" : "a pile of gold");
#ifdef STEED
                    /* A pile of gold can't ride. */
		    if (u.usteed) dismount_steed(DISMOUNT_FELL);
#endif
		    nomul(-tmp, "pretending to be a pile of gold");
		    Sprintf(buf, Hallucination ?
			"You suddenly dread being peeled and mimic %s again!" :
			"You now prefer mimicking %s again.",
			an(Upolyd ? youmonst.data->mname : urace.noun));
		    eatmbuf = strcpy((char *) alloc(strlen(buf) + 1), buf);
		    nomovemsg = eatmbuf;
		    afternmv = eatmdone;
		    /* ??? what if this was set before? */
		    youmonst.m_ap_type = M_AP_OBJECT;
		    youmonst.mappearance = Hallucination ? ORANGE : GOLD_PIECE;
		    newsym(u.ux,u.uy);
		    curs_on_u();
		    /* make gold symbol show up now */
		    display_nhwindow(WIN_MAP, TRUE);
		}
		break;
	    case PM_MERCURIAL_DRAGON:
		    if (!(HFast & INTRINSIC)) {
				if (!Fast)
					You("speed up.");
				else
					Your("quickness feels more natural.");
				HFast |= FROMOUTSIDE;
			}
			exercise(A_DEX, TRUE);
		break;
	    case PM_QUANTUM_MECHANIC:
		if (HFast & INTRINSIC) {
			if(!nobadeffects){
				HFast &= ~INTRINSIC;
				Your("velocity suddenly seems very uncertain!");
				You("seem slower.");
			}
		} else {
			HFast |= FROMOUTSIDE;
			Your("velocity suddenly seems very uncertain!");
			You("seem faster.");
		}
		break;
		case PM_GUG:
			if(!drained || !rn2(5)){
				gainstr((struct obj *)0, 0);
			}
		break;
	    case PM_MANDRAKE:
	    case PM_LIZARD:
	    case PM_SMALL_CAVE_LIZARD:
	    case PM_CAVE_LIZARD:
	    case PM_LARGE_CAVE_LIZARD:
			if (HStun > 2)  make_stunned(2L,FALSE);
			if (HConfusion > 2)  make_confused(2L,FALSE);
		break;
		case PM_BEHOLDER:
			if(!drained || !rn2(5)) {
				You(Hallucination ? "can see your own insides!"
					: "feel as though you can see the world from a whole new angle!");
				if (!BClairvoyant)
					do_vicinity_map(u.ux,u.uy);
				/* at present, only one thing blocks clairvoyance */
				else if (uarmh && uarmh->otyp == CORNUTHAUM)
					You("sense a pointy hat on top of your %s.",
					body_part(HEAD));
				incr_itimeout(&HClairvoyant, rn1(500,500));
			}
		break;
	    case PM_CHAMELEON:
	    case PM_DOPPELGANGER:
	 /* case PM_SANDESTIN: */
		if (!Unchanging && !nobadeffects) {
		    You_feel("a change coming over you.");
		    polyself(FALSE);
		}
		break;
	    case PM_MIND_FLAYER:
	    case PM_MASTER_MIND_FLAYER:
		if(!drained || !rn2(5)) {
			if (ABASE(A_INT) < ATTRMAX(A_INT)) {
				if (!rn2(2)) {
					pline("Yum! That was real brain food!");
					(void) adjattrib(A_INT, 1, FALSE);
					break;	/* don't give them telepathy, too */
				}
			}
			else {
				pline("For some reason, that tasted bland.");
			}
		}
		/* fall through to default case */
	    default: {
		register struct permonst *ptr = &mons[pm];
		int i, count;

		if (!nobadeffects && (dmgtype(ptr, AD_STUN) || dmgtype(ptr, AD_HALU) ||
		    pm == PM_VIOLET_FUNGUS)) {
			pline ("Oh wow!  Great stuff!");
			make_hallucinated(HHallucination + 200,FALSE,0L);
		}
		
		if(!drained || !rn2(5)) if(is_giant(ptr)) gainstr((struct obj *)0, 0);

		/* Check the monster for all of the intrinsics.  If this
		 * monster can give more than one, pick one to try to give
		 * from among all it can give.
		 *
		 * If a monster can give 4 intrinsics then you have
		 * a 1/1 * 1/2 * 2/3 * 3/4 = 1/4 chance of getting the first,
		 * a 1/2 * 2/3 * 3/4 = 1/4 chance of getting the second,
		 * a 1/3 * 3/4 = 1/4 chance of getting the third,
		 * and a 1/4 chance of getting the fourth.
		 *
		 * And now a proof by induction:
		 * it works for 1 intrinsic (1 in 1 of getting it)
		 * for 2 you have a 1 in 2 chance of getting the second,
		 *	otherwise you keep the first
		 * for 3 you have a 1 in 3 chance of getting the third,
		 *	otherwise you keep the first or the second
		 * for n+1 you have a 1 in n+1 chance of getting the (n+1)st,
		 *	otherwise you keep the previous one.
		 * Elliott Kleinrock, October 5, 1990
		 */

		 count = 0;	/* number of possible intrinsics */
		 tmp = 0;	/* which one we will try to give */
		 for (i = 1; i <= LAST_PROP; i++) {
			if (intrinsic_possible(i, ptr)) {
				count++;
					if(u.sealsActive&SEAL_AHAZU) givit(i, ptr, (tin && ptr->cnutrit > 45) ? 45 : ptr->cnutrit*0.9, drained);
					else givit(i, ptr, (tin && ptr->cnutrit > 50) ? 50 : ptr->cnutrit, drained);
			}
		 }

		 /* if any found try to give them one */
	    }
	    break;
	}

	if (catch_lycanthropy && spec_ability2(uwep, SPFX2_NOWERE)) {
	    if (!touch_artifact(uwep, &youmonst)) {
			dropx(uwep);
			uwepgone();
	    }
	}

	return;
}

void
violated_vegetarian()
{
    u.uconduct.unvegetarian++;
    if (Role_if(PM_MONK)) {
	You_feel("guilty.");
	adjalign(-1);
    }
    return;
}

/* common code to check and possibly charge for 1 context.tin.tin,
 * will split() context.tin.tin if necessary */
STATIC_PTR
void
costly_tin(verb)
	const char* verb;		/* if 0, the verb is "open" */
{
	if(((!carried(tin.tin) &&
	     costly_spot(tin.tin->ox, tin.tin->oy) &&
	     !tin.tin->no_charge)
	    || tin.tin->unpaid)) {
	    verbalize("You %s it, you bought it!", verb ? verb : "open");
	    if(tin.tin->quan > 1L) tin.tin = splitobj(tin.tin, 1L);
	    bill_dummy_object(tin.tin);
	}
}

STATIC_PTR
int
opentin()		/* called during each move whilst opening a tin */
{
	register int r;
	const char *what;
	int which;

	if(!carried(tin.tin) && !obj_here(tin.tin, u.ux, u.uy))
					/* perhaps it was stolen? */
		return(0);		/* %% probably we should use tinoid */
	if(tin.usedtime++ >= 50) {
		You("give up your attempt to open the tin.");
		return(0);
	}
	if(tin.usedtime < tin.reqtime)
		return(1);		/* still busy */
	if(tin.tin->otrapped ||
	   (tin.tin->cursed && tin.tin->spe != -1 && !rn2(8))) {
		b_trapped("tin", 0);
		costly_tin("destroyed");
		goto use_me;
	}
	You("succeed in opening the tin.");
	if(tin.tin->spe != 1) {
	    if (tin.tin->corpsenm == NON_PM) {
		pline("It turns out to be empty.");
		tin.tin->dknown = tin.tin->known = TRUE;
		costly_tin((const char*)0);
		goto use_me;
	    }
	    r = tin.tin->cursed ? ROTTEN_TIN :	/* always rotten if cursed */
		    (tin.tin->spe == -1) ? HOMEMADE_TIN :  /* player made it */
			rn2(TTSZ-1);		/* else take your pick */
	    if (r == ROTTEN_TIN && (tin.tin->corpsenm == PM_LIZARD || tin.tin->corpsenm == PM_SMALL_CAVE_LIZARD ||
			tin.tin->corpsenm == PM_CAVE_LIZARD || tin.tin->corpsenm == PM_LARGE_CAVE_LIZARD ||
			tin.tin->corpsenm == PM_LICHEN || tin.tin->corpsenm == PM_BEHOLDER))
		r = DRIED_TIN;		/* lizards don't rot */
	    else if (tin.tin->spe == -1 && !tin.tin->blessed && !rn2(7))
		r = ROTTEN_TIN;			/* some homemade tins go bad */
	    which = 0;	/* 0=>plural, 1=>as-is, 2=>"the" prefix */
	    if (Hallucination) {
		what = rndmonnam();
	    } else {
		what = mons[tin.tin->corpsenm].mname;
		if (mons[tin.tin->corpsenm].geno & G_UNIQ)
		    which = type_is_pname(&mons[tin.tin->corpsenm]) ? 1 : 2;
	    }
	    if (which == 0) what = makeplural(what);
	    pline("It smells like %s%s.", (which == 2) ? "the " : "", what);
        if (uclockwork){
			int eatHow;
			if(Hallucination && u.clockworkUpgrades&(WOOD_STOVE|MAGIC_FURNACE|HELLFIRE_FURNACE|SCRAP_MAW)){
				You("try to put it in your intake hopper, but you hallucinate and dump it instead!");
				costly_tin((const char*)0);
				goto use_me;
			} else if((eatHow = clockwork_eat_menu(r == DRIED_TIN || is_burnable(&mons[tin.tin->corpsenm]),	
						u.clockworkUpgrades&MAGIC_FURNACE && 
						(tin.tin->corpsenm == PM_NEWT || 
						 tin.tin->corpsenm == PM_AOA_DROPLET || 
						 tin.tin->corpsenm == PM_AOA))
			)){
				if(eatHow == WOOD_STOVE){
					/* KMH, conduct */
					u.uconduct.food++;
					if (!vegan(&mons[tin.tin->corpsenm]))
					u.uconduct.unvegan++;
					if (!vegetarian(&mons[tin.tin->corpsenm]))
					violated_vegetarian();
					
					tin.tin->dknown = tin.tin->known = TRUE;
					
					u.ustove += tintxts[r].nut;
					costly_tin((const char*)0);
					goto use_me;
				} else if(eatHow == MAGIC_FURNACE){
					/* KMH, conduct */
					u.uconduct.food++;
					if (!vegan(&mons[tin.tin->corpsenm]))
					u.uconduct.unvegan++;
					if (!vegetarian(&mons[tin.tin->corpsenm]))
					violated_vegetarian();
					
					tin.tin->dknown = tin.tin->known = TRUE;
					
					cpostfx(tin.tin->corpsenm, FALSE, FALSE, FALSE);
					costly_tin((const char*)0);
					goto use_me;
				} else if(eatHow == HELLFIRE_FURNACE){
					/* KMH, conduct */
					u.uconduct.food++;
					if (!vegan(&mons[tin.tin->corpsenm]))
					u.uconduct.unvegan++;
					if (!vegetarian(&mons[tin.tin->corpsenm]))
					violated_vegetarian();
					
					tin.tin->dknown = tin.tin->known = TRUE;
					
					u.ustove += (tintxts[r].nut)/10;
					costly_tin((const char*)0);
					goto use_me;
				} else if(eatHow == SCRAP_MAW){
					You("crunch up the tin can");
					if(!Upolyd){
						u.uhp += 5;
						if (u.uhp > u.uhpmax) u.uhp = u.uhpmax;
					} else {
						u.mh += 5;
						if (u.mh > u.mhmax) u.mh = u.mhmax;
					}
					tin.tin->dknown = tin.tin->known = TRUE;
					costly_tin((const char*)0);
					goto use_me;
				}
			} else {
				You("discard the tin.");
				tin.tin->dknown = tin.tin->known = TRUE;
				costly_tin((const char*)0);
				goto use_me;
			}
        } else if(Race_if(PM_INCANTIFIER) && 
			((tin.tin->corpsenm != PM_NEWT && 
			tin.tin->corpsenm != PM_AOA_DROPLET && 
			tin.tin->corpsenm != PM_AOA) ||
			Hallucination) 
		){
				You("would not derive any nutrition from eating that, so you discard it instead.");
				costly_tin((const char*)0);
				goto use_me;
		}
	    if (yn("Eat it?") == 'n') {
			if (!Hallucination) tin.tin->dknown = tin.tin->known = TRUE;
			if (flags.verbose) You("discard the open tin.");
			costly_tin((const char*)0);
			goto use_me;
	    }
		if(Race_if(PM_INCANTIFIER)){
			u.uconduct.food++;
			You("drain energy from %s %s.", tintxts[r].txt,
				mons[tin.tin->corpsenm].mname);
			cprefx(tin.tin->corpsenm, TRUE, FALSE);
			cpostfx(tin.tin->corpsenm, FALSE, FALSE, FALSE);
			costly_tin((const char*)0);
			goto use_me;
		}
	    /* in case stop_occupation() was called on previous meal */
	    victual.piece = (struct obj *)0;
	    victual.fullwarn = victual.eating = victual.doreset = FALSE;
		
		if(tin.tin->corpsenm == PM_JUBJUB_BIRD && tin.tin->blessed){
			You("consume symmetrical %s", mons[tin.tin->corpsenm].mname);
		} else {
			You("consume %s %s.", tintxts[r].txt,
				mons[tin.tin->corpsenm].mname);
		}

	    /* KMH, conduct */
	    u.uconduct.food++;
	    if (!vegan(&mons[tin.tin->corpsenm]))
		u.uconduct.unvegan++;
	    if (!vegetarian(&mons[tin.tin->corpsenm]))
		violated_vegetarian();

	    tin.tin->dknown = tin.tin->known = TRUE;
	    cprefx(tin.tin->corpsenm, FALSE, FALSE);
		cpostfx(tin.tin->corpsenm, TRUE, FALSE, FALSE);

	    /* charge for one at pre-eating cost */
	    costly_tin((const char*)0);

	    /* check for vomiting added by GAN 01/16/87 */
	    if(tintxts[r].nut < 0) make_vomiting((long)rn1(15,10), FALSE);
		else if(tin.tin->corpsenm == PM_JUBJUB_BIRD && tin.tin->blessed){
			lesshungry(600);
			use_unicorn_horn(tin.tin);
		} else lesshungry(tintxts[r].nut);

	    if(r == 0 || r == FRENCH_FRIED_TIN) {
	        /* Assume !Glib, because you can't open tins when Glib. */
		incr_itimeout(&Glib, rnd(15));
		pline("Eating deep fried food made your %s very slippery.",
		      makeplural(body_part(FINGER)));
	    }
	} else {
	    if (tin.tin->cursed)
		pline("It contains some decaying%s%s substance.",
			Blind ? "" : " ", Blind ? "" : hcolor(NH_GREEN));
	    else
		pline("It contains spinach.");
        if (uclockwork){
			if(u.clockworkUpgrades&HELLFIRE_FURNACE &&
				yn("Burn it in your hellfire furnace?") == 'y'
			){
				/* KMH, conduct */
				u.uconduct.food++;
				tin.tin->dknown = tin.tin->known = TRUE;
				u.ustove += 60;
				costly_tin((const char*)0);
				goto use_me;
			} else if(u.clockworkUpgrades&SCRAP_MAW &&
				yn("Feed it to your scrap maw?") == 'y'
			){
				You("crunch up the tin can");
				if(!Upolyd){
					u.uhp += 5;
					if (u.uhp > u.uhpmax) u.uhp = u.uhpmax;
				} else {
					u.mh += 5;
					if (u.mh > u.mhmax) u.mh = u.mhmax;
				}
				tin.tin->dknown = tin.tin->known = TRUE;
				costly_tin((const char*)0);
				goto use_me;
			} else if(u.clockworkUpgrades&WOOD_STOVE && (r == DRIED_TIN || is_burnable(&mons[tin.tin->corpsenm]) )){
				/* KMH, conduct */
				pline("Sadly, the spinach is much too damp to burn.");
				You("reluctantly discard the spinach");
				tin.tin->dknown = tin.tin->known = TRUE;
				costly_tin((const char*)0);
				goto use_me;
			} else {
				You("have no way to eat%s.", 
					u.clockworkUpgrades&(WOOD_STOVE|OIL_STOVE|MAGIC_FURNACE|HELLFIRE_FURNACE|SCRAP_MAW) ? " that" : "");
				You("reluctantly discard the spinach");
				tin.tin->dknown = tin.tin->known = TRUE;
				costly_tin((const char*)0);
				goto use_me;
			}
        } else if(Race_if(PM_INCANTIFIER) ){
				pline("Sadly, you would not derive any nutrition from eating it.");
				You("reluctantly discard the spinach");
				costly_tin((const char*)0);
				goto use_me;
		}
	    if (yn("Eat it?") == 'n') {
		if (!Hallucination && !tin.tin->cursed)
		    tin.tin->dknown = tin.tin->known = TRUE;
		if (flags.verbose)
		    You("discard the open tin.");
			costly_tin((const char*)0);
			goto use_me;
	    }

	    tin.tin->dknown = tin.tin->known = TRUE;
	    costly_tin((const char*)0);

	    if (!tin.tin->cursed)
		pline("This makes you feel like %s!",
		      Hallucination ? "Swee'pea" : "Popeye");
	    lesshungry(600);
	    gainstr(tin.tin, 0);
	    u.uconduct.food++;
	}
use_me:
	if (carried(tin.tin)) useup(tin.tin);
	else useupf(tin.tin, 1L);
	tin.tin = (struct obj *) 0;
	return(0);
}

STATIC_OVL void
start_tin(otmp)		/* called when starting to open a tin */
	register struct obj *otmp;
{
	register int tmp;

	if (metallivorous(youmonst.data)) {
		You("bite right into the metal tin...");
		tmp = 1;
	} else if (Upolyd && u.umonnum == PM_TOVE) {
		You("gyre, gimbling right into the metal tin.");
		tmp = 1;
	} else if (nolimbs(youmonst.data)) {
		You("cannot handle the tin properly to open it.");
		return;
	} else if (otmp->blessed) {
		pline_The("tin opens like magic!");
		tmp = 1;
	} else if(uwep) {
		switch(uwep->otyp) {
		case TIN_OPENER:
			tmp = 1;
			break;
		case DAGGER:
		case SILVER_DAGGER:
		case ELVEN_DAGGER:
		case ORCISH_DAGGER:
		case ATHAME:
		case CRYSKNIFE:
			tmp = 3;
			break;
		case PICK_AXE:
		case AXE:
			tmp = 6;
			break;
		default:
			goto no_opener;
		}
		pline("Using your %s you try to open the tin.",
			aobjnam(uwep, (char *)0));
	} else {
no_opener:
		pline("It is not so easy to open this tin.");
		if(Glib) {
			pline_The("tin slips from your %s.",
			      makeplural(body_part(FINGER)));
			if(otmp->quan > 1L) {
			    otmp = splitobj(otmp, 1L);
			}
			if (carried(otmp)) dropx(otmp);
			else stackobj(otmp);
			return;
		}
		tmp = rn1(1 + 500/((int)(ACURR(A_DEX) + ACURRSTR)), 10);
	}
	tin.reqtime = tmp;
	tin.usedtime = 0;
	tin.tin = otmp;
	set_occupation(opentin, "opening the tin", 0);
	return;
}

int
Hear_again(VOID_ARGS)		/* called when waking up after fainting */
{
	flags.soundok = 1;
	return 0;
}

/* called on the "first bite" of rotten food */
STATIC_OVL int
rottenfood(obj)
struct obj *obj;
{
	pline("Blecch!  Rotten %s!",
		maybe_polyd(is_vampire(youmonst.data),
			Race_if(PM_VAMPIRE)) ? "blood" : foodword(obj));
	if(!rn2(4)) {
		if (Hallucination) You_feel("rather trippy.");
		else You_feel("rather %s.", body_part(LIGHT_HEADED));
		make_confused(HConfusion + d(2,4),FALSE);
	} else if(!rn2(4) && !Blind) {
		pline("Everything suddenly goes dark.");
		make_blinded((long)d(2,10),FALSE);
		if (!Blind) Your("%s", vision_clears);
	} else if(!rn2(3)) {
		const char *what, *where;
		if (!Blind)
		    what = "goes",  where = "dark";
		else if (Levitation || Is_airlevel(&u.uz) ||
			 Is_waterlevel(&u.uz))
		    what = "you lose control of",  where = "yourself";
		else
		    what = "you slap against the", where =
#ifdef STEED
			   (u.usteed) ? "saddle" :
#endif
			   surface(u.ux,u.uy);
		pline_The("world spins and %s %s.", what, where);
		flags.soundok = 0;
		nomul(-rnd(10), "unconscious from rotten food");
		nomovemsg = "You are conscious again.";
		afternmv = Hear_again;
		return(1);
	}
	return(0);
}

static char *eatrat[] = { 
	"This tastes delicious!",
	"This tastes quite good!",
	"This tastes quite good (could use some ketchup, though).",
	"This tastes ... good (but really needs some ketchup).",
	"...You'd pay double for ketchup.",
	"...What you wouldn't give for some ketchup....",
};

STATIC_OVL int
eatcorpse(otmp)		/* called when a corpse is selected as food */
	register struct obj *otmp;
{
	int tp = 0, mnum = otmp->corpsenm;
	long rotted = 0L;
	boolean uniq = !!(mons[mnum].geno & G_UNIQ);
	int retcode = 0;
	boolean stoneable = (touch_petrifies(&mons[mnum]) && !Stone_resistance &&
				!poly_when_stoned(youmonst.data));

	/* KMH, conduct */
	if (!vegan(&mons[mnum])) u.uconduct.unvegan++;
	if (!vegetarian(&mons[mnum])) violated_vegetarian();

	if (mnum != PM_LIZARD && mnum != PM_SMALL_CAVE_LIZARD && mnum != PM_CAVE_LIZARD 
		&& mnum != PM_LARGE_CAVE_LIZARD && mnum != PM_LICHEN && mnum != PM_BEHOLDER
	) {
		long age = peek_at_iced_corpse_age(otmp);

		rotted = (monstermoves - age)/(10L + rn2(20));
		if (otmp->cursed) rotted += 2L;
		else if (otmp->blessed) rotted -= 2L;
	}

	/* Vampires only drink the blood of very young, meaty corpses 
	 * is_edible only allows meaty corpses here
	 * Blood is assumed to be 1/5 of the nutrition
	 * Thus happens before the conduct checks intentionally - should it be after?
	 * Blood is assumed to be meat and flesh.
	 */
	if (maybe_polyd(is_vampire(youmonst.data), Race_if(PM_VAMPIRE))) {
	    /* oeaten is set up by touchfood */
	    if (otmp->odrained ? otmp->oeaten <= drainlevel(otmp) :
	      otmp->oeaten < mons[otmp->corpsenm].cnutrit) {
	    	pline("There is no blood left in this corpse!");
	    	return 3;
	    } else if (rotted <= 0 &&
	      (peek_at_iced_corpse_age(otmp) + 5) >= monstermoves) {
		char buf[BUFSZ];

		/* Generate the name for the corpse */
		if (!uniq || Hallucination)
		    Sprintf(buf, "%s", the(corpse_xname(otmp,TRUE)));
		else
		    Sprintf(buf, "%s%s corpse",
			    !type_is_pname(&mons[mnum]) ? "the " : "",
			    s_suffix(mons[mnum].mname));

	    	pline("You drain the blood from %s.", buf);
		otmp->odrained = 1;
	    } else {
	    	pline("The blood in this corpse has coagulated!");
	    	return 3;
	    }
	}
	else
	    otmp->odrained = 0;

	/* Very rotten corpse will make you sick unless you are a ghoul or a ghast or bound to Chupoclops */
	if (mnum != PM_ACID_BLOB && !stoneable && rotted > 5L) {
		boolean cannibal = maybe_cannibal(mnum, FALSE);
	    if (u.umonnum == PM_GHOUL) {
	    	pline("Yum - that %s was well aged%s!",
		      mons[mnum].mlet == S_PLANT ? "vegetation" :
		      mons[mnum].mlet == S_FUNGUS ? "fungoid vegetation" :
		      !vegetarian(&mons[mnum]) ? "meat" : "protoplasm",
		      cannibal ? ", cannibal" : "");
	    } else if(u.sealsActive&SEAL_CHUPOCLOPS){
		boolean cannibal = maybe_cannibal(mnum, FALSE);
			pline("The corpse liquefies into a putrid broth, and you slurp it down%s!",
			cannibal ? ", cannibal" : "");
		} else {
		pline("Ulch - that %s was tainted%s!",
		      mons[mnum].mlet == S_FUNGUS ? "fungoid vegetation" :
		      !vegetarian(&mons[mnum]) ? "meat" : "protoplasm",
		      cannibal ? ", cannibal" : "");
		if (Sick_resistance) {
			pline("It doesn't seem at all sickening, though...");
		} else {
			char buf[BUFSZ];
			long sick_time;

			sick_time = (long) rn1(10, 10);
			/* make sure new ill doesn't result in improvement */
			if (Sick && (sick_time > Sick))
			    sick_time = (Sick > 1L) ? Sick - 1L : 1L;
			if (!uniq)
			    Sprintf(buf, "rotted %s", corpse_xname(otmp,TRUE));
			else
			    Sprintf(buf, "%s%s rotted corpse",
				    !type_is_pname(&mons[mnum]) ? "the " : "",
				    s_suffix(mons[mnum].mname));
			make_sick(sick_time, buf, TRUE, SICK_VOMITABLE);
		}
		if (carried(otmp)) useup(otmp);
		else useupf(otmp, 1L);
		return(2);
	    }
	} else if (youmonst.data == &mons[PM_GHOUL]) {
		pline ("This corpse is too fresh!");
		return 3;
	} else if (acidic(&mons[mnum]) && !Acid_resistance) {
		tp++;
		You("have a very bad case of stomach acid."); /* not body_part() */
		losehp(rnd(15), "acidic corpse", KILLED_BY_AN);
	} if (freezing(&mons[mnum]) && !Cold_resistance) {
		tp++;
		You("feel your stomach freeze!"); /* not body_part() */
		losehp(rnd(12) + rnd(12), "cryonic corpse", KILLED_BY_AN);
	} if (burning(&mons[mnum]) && !Fire_resistance) {
		tp++;
		You("feel your stomach boil!"); /* not body_part() */
		morehungry(200);
		nomovemsg = 0;
		vomit();
		losehp(rnd(20), "boiling hot corpse", KILLED_BY_AN);
	} if (poisonous(&mons[mnum]) && rn2(5)) {
		tp++;
		pline("Ecch - that must have been poisonous!");
		if(!Poison_resistance) {
			losestr(rnd(4));
			losehp(rnd(15), "poisonous corpse", KILLED_BY_AN);
		} else	You("seem unaffected by the poison.");
	/* now any corpse left too long will make you mildly ill */
	} if ( !(poisonous(&mons[mnum]) || burning(&mons[mnum]) ||
			 freezing(&mons[mnum])  || acidic(&mons[mnum])) &&
			(rotted > 5L || (rotted > 3L && rn2(5)))
					&& !(Sick_resistance || u.sealsActive&SEAL_CHUPOCLOPS)) {
		tp++;
		You_feel("%ssick.", (Sick) ? "very " : "");
		losehp(rnd(8), "cadaver", KILLED_BY_AN);
	}

	/* delay is weight dependent */
	victual.reqtime = (u.sealsActive&SEAL_AHAZU) ? 1 : (3 + (mons[mnum].cwt >> 6));
	if (otmp->odrained) victual.reqtime = (u.sealsActive&SEAL_AHAZU) ? 1 : rounddiv(victual.reqtime, 5);

	if (!tp && mnum != PM_LIZARD && mnum != PM_SMALL_CAVE_LIZARD && mnum != PM_CAVE_LIZARD 
		&& mnum != PM_LARGE_CAVE_LIZARD && mnum != PM_LICHEN && mnum != PM_BEHOLDER &&
			(otmp->orotten || !rn2(7))) {
	    if ( (monstermoves - peek_at_iced_corpse_age(otmp)) > 5 && rottenfood(otmp)) {
		otmp->orotten = TRUE;
		(void)touchfood(otmp);
		retcode = 1;
	    }

	    if (!mons[otmp->corpsenm].cnutrit) {
			/* no nutrution: rots away, no message if you passed out */
			if (!retcode) pline_The("corpse rots away completely.");
			if (carried(otmp)) useup(otmp);
			else useupf(otmp, 1L);
			retcode = 2;
	    } else {
			if (!retcode) consume_oeaten(otmp, 2);	/* oeaten >>= 2 */
			if (otmp->odrained && otmp->oeaten < drainlevel(otmp))
				otmp->oeaten = drainlevel(otmp);
		}
	} else if (maybe_polyd(!is_vampire(youmonst.data), !Race_if(PM_VAMPIRE))) {
		if(is_rat(&mons[mnum]) && Race_if(PM_DWARF)){
			if(u.uconduct.ratseaten<SIZE(eatrat)) pline("%s", eatrat[u.uconduct.ratseaten++]);
			else{
				u.uconduct.ratseaten++;
				pline("%s", eatrat[SIZE(eatrat)-1]);
			}
		} else if(mnum == PM_LONG_WORM){
			pline("This tastes spicy!");
		} else if(mnum == PM_CROW){
			You("eat crow!");
		} else {
		    pline("%s%s %s!",
			  !uniq ? "This " : !type_is_pname(&mons[mnum]) ? "The " : "",
			  food_xname(otmp, FALSE),
			  (vegan(&mons[mnum]) ?
			   (!carnivorous(youmonst.data) && herbivorous(youmonst.data)) :
			   (carnivorous(youmonst.data) && !herbivorous(youmonst.data)))
			  ? "is delicious" : "tastes terrible");
		}
	}

	return(retcode);
}

STATIC_OVL void
start_eating(otmp)		/* called as you start to eat */
	register struct obj *otmp;
{
#ifdef DEBUG
	debugpline("start_eating: %lx (victual = %lx)", otmp, victual.piece);
	debugpline("reqtime = %d", victual.reqtime);
	debugpline("(original reqtime = %d)", objects[otmp->otyp].oc_delay);
	debugpline("nmod = %d", victual.nmod);
	debugpline("oeaten = %d", otmp->oeaten);
#endif
	victual.fullwarn = victual.doreset = FALSE;
	victual.eating = TRUE;

	if (otmp->otyp == CORPSE) {
	    cprefx(victual.piece->corpsenm,FALSE,FALSE);
	    if (!victual.piece || !victual.eating) {
		/* rider revived, or died and lifesaved */
		return;
	    }
	}

	if (bite()) return;

	if (++victual.usedtime >= victual.reqtime) {
	    /* print "finish eating" message if they just resumed -dlc */
	    done_eating(victual.reqtime > 1 ? TRUE : FALSE);
	    return;
	}

	Sprintf(msgbuf, "%s %s", otmp->odrained ? "draining" : "eating",
	  food_xname(otmp, TRUE));
	set_occupation(eatfood, msgbuf, 0);
}


/*
 * called on "first bite" of (non-corpse) food.
 * used for non-rotten non-tin non-corpse food
 */
STATIC_OVL void
fprefx(otmp)
struct obj *otmp;
{
	switch(otmp->otyp) {
	    case FOOD_RATION:
		if(YouHunger <= 200)
		    pline(Hallucination ? "Oh wow, like, superior, man!" :
			  "That food really hit the spot!");
		else if(YouHunger <= u.uhungermax/2 - 300) pline("That satiated your %s!",
						body_part(STOMACH));
		break;
	    case TRIPE_RATION:
		if (carnivorous(youmonst.data) && (!humanoid(youmonst.data)) || 
			u.ulycn != NON_PM && carnivorous(&mons[u.ulycn]) && 
			!humanoid(&mons[u.ulycn]))
		    /* Symptom of lycanthropy is starting to like your
		     * alternative form's food! 
		     */
		    pline("That tripe ration was surprisingly good!");
		else if (maybe_polyd(is_orc(youmonst.data), Race_if(PM_ORC)))
		    pline(Hallucination ? "Tastes great! Less filling!" :
			  "Mmm, tripe... not bad!");
		else {
		    pline("Yak - dog food!");
#ifdef CONVICT
		    if (Role_if(PM_CONVICT))
			pline("At least it's not prison food.");
#endif /* CONVICT */
		    more_experienced(1,0);
		    newexplevel();
		    /* not cannibalism, but we use similar criteria
		       for deciding whether to be sickened by this meal */
		    if (rn2(2) && !CANNIBAL_ALLOWED())
#ifdef CONVICT
		    if (!Role_if(PM_CONVICT))
#endif /* CONVICT */
			make_vomiting((long)rn1(victual.reqtime, 14), FALSE);
		}
		break;
	    case MEATBALL:
	    case MEAT_STICK:
	    case HUGE_CHUNK_OF_MEAT:
	    case MEAT_RING:
		goto give_feedback;
	     /* break; */
	    case CLOVE_OF_GARLIC:
		if (is_undead(youmonst.data)) {
			make_vomiting((long)rn1(victual.reqtime, 5), FALSE);
			break;
		}
		/* Fall through otherwise */
	    default:
		if (otmp->otyp==SLIME_MOLD && !otmp->cursed
			&& otmp->spe == current_fruit)
		    pline("My, that was a %s %s!",
			  Hallucination ? "primo" : "yummy",
			  singular(otmp, xname));
		else
#ifdef UNIX
		if (otmp->otyp == APPLE || otmp->otyp == PEAR) {
		    if (!Hallucination) pline("Core dumped.");
		    else {
/* This is based on an old Usenet joke, a fake a.out manual page */
			int x = rnd(100);
			if (x <= 75)
			    pline("Segmentation fault -- core dumped.");
			else if (x <= 99)
			    pline("Bus error -- core dumped.");
			else pline("Yo' mama -- core dumped.");
		    }
		} else
#endif
#ifdef MAC	/* KMH -- Why should Unix have all the fun? */
		if (otmp->otyp == APPLE) {
			pline("Delicious!  Must be a Macintosh!");
		} else
#endif
		if (otmp->otyp == EGG && stale_egg(otmp)) {
		    pline("Ugh.  Rotten egg.");	/* perhaps others like it */
#ifdef CONVICT
		if (Role_if(PM_CONVICT) && (rn2(8) > u.ulevel)) {
		    You_feel("a slight stomach ache.");	/* prisoners are used to bad food */
		} else
#endif /* CONVICT */
		    make_vomiting(Vomiting+d(10,4), TRUE);
		} else
 give_feedback:
		    pline("This %s is %s", singular(otmp, xname),
		      otmp->cursed ? (Hallucination ? "grody!" : "terrible!") :
		      (otmp->otyp == CRAM_RATION
		      || otmp->otyp == K_RATION
		      || otmp->otyp == C_RATION)
		      ? "bland." :
		      Hallucination ? "gnarly!" : "delicious!");
		break;
	}
}

STATIC_OVL void
accessory_has_effect(otmp)
struct obj *otmp;
{
	pline("Magic spreads through your body as you digest the %s.",
	    otmp->oclass == RING_CLASS ? "ring" : "amulet");
}

STATIC_OVL void
eataccessory(otmp)
struct obj *otmp;
{
	int typ = otmp->otyp;
	long oldprop;

	/* Note: rings are not so common that this is unbalancing. */
	/* (How often do you even _find_ 3 rings of polymorph in a game?) */
	oldprop = u.uprops[objects[typ].oc_oprop].intrinsic;
	if (otmp == uleft || otmp == uright) {
	    Ring_gone(otmp);
	    if (u.uhp <= 0) return; /* died from sink fall */
	}
	otmp->known = otmp->dknown = 1; /* by taste */
	if (otmp->otyp == AMULET_OF_DRAIN_RESISTANCE || !rn2(otmp->oclass == RING_CLASS ? 3 : 5)) {
	  switch (otmp->otyp) {
	    default:
	        if (!objects[typ].oc_oprop) break; /* should never happen */

		if (!(u.uprops[objects[typ].oc_oprop].intrinsic & FROMOUTSIDE))
		    accessory_has_effect(otmp);

		u.uprops[objects[typ].oc_oprop].intrinsic |= FROMOUTSIDE;

		switch (typ) {
		  case RIN_SEE_INVISIBLE:
		    set_mimic_blocking();
		    see_monsters();
		    if (Invis && !oldprop && !ESee_invisible &&
				!perceives(youmonst.data) && !Blind) {
			newsym(u.ux,u.uy);
			pline("Suddenly you can see yourself.");
			makeknown(typ);
		    }
		    break;
		  case RIN_INVISIBILITY:
		    if (!oldprop && !EInvis && !BInvis && !Blind) {
				if(See_invisible){
					Your("body takes on a %s transparency...",
						Hallucination ? "normal" : "strange");
					makeknown(typ);
				} else {
					newsym(u.ux,u.uy);
					Your("body fades out of view.");
					makeknown(typ);
				}
		    }
		    break;
		  case RIN_PROTECTION_FROM_SHAPE_CHAN:
		    rescham();
		    break;
		  case RIN_LEVITATION:
		    /* undo the `.intrinsic |= FROMOUTSIDE' done above */
		    u.uprops[LEVITATION].intrinsic = oldprop;
		    if (!Levitation) {
			float_up();
			incr_itimeout(&HLevitation, d(10,20));
			makeknown(typ);
		    }
		    break;
		}
		break;
	    case RIN_ADORNMENT:
		accessory_has_effect(otmp);
		if (adjattrib(A_CHA, otmp->spe, -1))
		    makeknown(typ);
		break;
	    case RIN_GAIN_STRENGTH:
		accessory_has_effect(otmp);
		if (adjattrib(A_STR, otmp->spe, -1))
		    makeknown(typ);
		break;
	    case RIN_GAIN_CONSTITUTION:
		accessory_has_effect(otmp);
		if (adjattrib(A_CON, otmp->spe, -1))
		    makeknown(typ);
		break;
	    case RIN_INCREASE_ACCURACY:
		accessory_has_effect(otmp);
		u.uhitinc += otmp->spe;
		break;
	    case RIN_INCREASE_DAMAGE:
		accessory_has_effect(otmp);
		u.udaminc += otmp->spe;
		break;
	    case RIN_PROTECTION:
		accessory_has_effect(otmp);
		HProtection |= FROMOUTSIDE;
		u.ublessed += otmp->spe;
		flags.botl = 1;
		break;
	    case RIN_FREE_ACTION:
		/* Give sleep resistance instead */
		if (!(HSleep_resistance & FROMOUTSIDE))
		    accessory_has_effect(otmp);
		if (!Sleep_resistance)
		    You_feel("wide awake.");
		HSleep_resistance |= FROMOUTSIDE;
		break;
	    case AMULET_OF_CHANGE:
		accessory_has_effect(otmp);
		makeknown(typ);
		change_sex();
		You("are suddenly very %s!",
		    flags.female ? "feminine" : "masculine");
		flags.botl = 1;
		break;
	    case AMULET_OF_UNCHANGING:
		/* un-change: it's a pun */
		if (!Unchanging && Upolyd) {
		    accessory_has_effect(otmp);
		    makeknown(typ);
		    rehumanize();
		}
		break;
	    case AMULET_OF_STRANGULATION: /* bad idea! */
		/* no message--this gives no permanent effect */
		if(!Race_if(PM_INCANTIFIER)) choke(otmp);
		break;
	    case AMULET_OF_RESTFUL_SLEEP: /* another bad idea! */
		if (!(HSleeping & FROMOUTSIDE))
		    accessory_has_effect(otmp);
		HSleeping = FROMOUTSIDE | rnd(100);
		break;
	    case AMULET_OF_DRAIN_RESISTANCE:
#ifdef DEBUG
			debugpline("Trying to give drain resistance");
#endif
			if( !(HDrain_resistance) ) {
				You(Hallucination ? "are bouncing off the walls!" :
					"feel especially energetic.");
			}
			if( (HDrain_resistance & TIMEOUT) + 1000 < TIMEOUT) {
				long timer = (HDrain_resistance & TIMEOUT)+1000;
				HDrain_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
				HDrain_resistance |= timer; //set new timer
			}
			else{
				HDrain_resistance |= TIMEOUT; //set timer to max value
			}
		break;
	    case RIN_SUSTAIN_ABILITY:
	    case AMULET_OF_LIFE_SAVING:
	    case AMULET_OF_REFLECTION: /* nice try */
	    /* can't eat Amulet of Yendor or fakes,
	     * and no oc_prop even if you could -3.
	     */
		break;
	  }
	}
}

STATIC_OVL void
eatspecial() /* called after eating non-food */
{
	register struct obj *otmp = victual.piece;

	/* lesshungry wants an occupation to handle choke messages correctly */
	set_occupation(eatfood, "eating non-food", 0);
	lesshungry(victual.nmod);
	occupation = 0;
	victual.piece = (struct obj *)0;
	victual.eating = 0;
	if (otmp->oclass == COIN_CLASS) {
#ifdef GOLDOBJ
		if (carried(otmp))
		    useupall(otmp);
#else
		if (otmp->where == OBJ_FREE)
		    dealloc_obj(otmp);
#endif
		else
		    useupf(otmp, otmp->quan);
		return;
	}
	if (otmp->oclass == POTION_CLASS) {
		otmp->quan++; /* dopotion() does a useup() */
		(void)dopotion(otmp);
	}
	if (otmp->oclass == RING_CLASS || otmp->oclass == AMULET_CLASS)
		eataccessory(otmp);
	else if (otmp->otyp == LEASH && otmp->leashmon)
		o_unleash(otmp);

	/* KMH -- idea by "Tommy the Terrorist" */
	if ((otmp->otyp == TRIDENT) && !otmp->cursed)
	{
		pline(Hallucination ? "Four out of five dentists agree." :
				"That was pure chewing satisfaction!");
		exercise(A_WIS, TRUE);
	}
	if ((otmp->otyp == FLINT) && !otmp->cursed)
	{
		pline("Yabba-dabba delicious!");
		exercise(A_CON, TRUE);
	}

	if (otmp == uwep && otmp->quan == 1L) uwepgone();
	if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
	if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();

	if (otmp == uball) unpunish();
	if (otmp == uchain) unpunish(); /* but no useup() */
	else if (carried(otmp)) useup(otmp);
	else useupf(otmp, 1L);
}

/* NOTE: the order of these words exactly corresponds to the
   order of oc_material values #define'd in objclass.h. */
static const char *foodwords[] = {
	"meal", "liquid", "wax", "food", "meat",
	"paper", "cloth", "leather", "wood", "bone", "scale",
	"metal", "metal", "metal", "silver", "gold", "platinum", "mithril",
	"plastic", "glass", "rich food", "stone"
};

STATIC_OVL const char *
foodword(otmp)
register struct obj *otmp;
{
	if (otmp->oclass == FOOD_CLASS) return "food";
	if (otmp->oclass == GEM_CLASS &&
	    objects[otmp->otyp].oc_material == GLASS &&
	    otmp->dknown)
		makeknown(otmp->otyp);
	return foodwords[objects[otmp->otyp].oc_material];
}

STATIC_OVL void
fpostfx(otmp)		/* called after consuming (non-corpse) food */
register struct obj *otmp;
{
	if(u.sealsActive&SEAL_ENKI && u.uhp < u.uhpmax && otmp->otyp >= CREAM_PIE && otmp->otyp <= TIN){
		pline("The fruits of civilization give you strength!");
		if(!Upolyd){
			u.uhp += objects[otmp->otyp].oc_nutrition;
			if (u.uhp > u.uhpmax) u.uhp = u.uhpmax;
		} else {
			u.mh += objects[otmp->otyp].oc_nutrition;
			if (u.mh > u.mhmax) u.mh = u.mhmax;
		}
	}
	switch(otmp->otyp) {
	    case SPRIG_OF_WOLFSBANE:
		if (u.ulycn >= LOW_PM || is_were(youmonst.data))
		    you_unwere(TRUE);
		break;
	    case CARROT:
		make_blinded((long)u.ucreamed,TRUE);
		break;
	    case FORTUNE_COOKIE:
	   	if (yn("Read the fortune?") == 'y') {
			outrumor(bcsign(otmp), BY_COOKIE);
			if (!Blind) u.uconduct.literate++;
		}
		break;
	    case LUMP_OF_ROYAL_JELLY:
		/* This stuff seems to be VERY healthy! */
		gainstr(otmp, 1);
		if (Upolyd) {
		    u.mh += otmp->cursed ? -rnd(20) : rnd(20);
		    if (u.mh > u.mhmax) {
			if (!rn2(17)) u.mhmax++;
			u.mh = u.mhmax;
		    } else if (u.mh <= 0) {
			rehumanize();
		    }
		} else {
		    u.uhp += otmp->cursed ? -rnd(20) : rnd(20);
		    if (u.uhp > u.uhpmax) {
			if(!rn2(17)) u.uhpmax++;
			u.uhp = u.uhpmax;
		    } else if (u.uhp <= 0) {
			killer_format = KILLED_BY_AN;
			killer = "rotten lump of royal jelly";
			done(POISONING);
		    }
		}
		if(!otmp->cursed) heal_legs();
		break;
	    case EGG:
		if (touch_petrifies(&mons[otmp->corpsenm])) {
		    if (!Stone_resistance &&
			!(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
			if (!Stoned) Stoned = 5;
			killer_format = KILLED_BY_AN;
			Sprintf(killer_buf, "%s egg", mons[otmp->corpsenm].mname);
			delayed_killer = killer_buf;
		    }
		}
		break;
	    case EUCALYPTUS_LEAF:
		if (Sick && !otmp->cursed)
		    make_sick(0L, (char *)0, TRUE, SICK_ALL);
		if (Vomiting && !otmp->cursed)
		    make_vomiting(0L, TRUE);
		break;
	}
	return;
}

/*
 * return 0 if the food was not dangerous.
 * return 1 if the food was dangerous and you chose to stop.
 * return 2 if the food was dangerous and you chose to eat it anyway.
 */
STATIC_OVL int
edibility_prompts(otmp)
struct obj *otmp;
{
	/* blessed food detection granted you a one-use
	   ability to detect food that is unfit for consumption
	   or dangerous and avoid it. */

	char buf[BUFSZ], foodsmell[BUFSZ],
	     it_or_they[QBUFSZ], eat_it_anyway[QBUFSZ];
	boolean cadaver = (otmp->otyp == CORPSE),
		stoneorslime = FALSE;
	int material = objects[otmp->otyp].oc_material,
	    mnum = otmp->corpsenm;
	long rotted = 0L;

	Strcpy(foodsmell, Tobjnam(otmp, "smell"));
	Strcpy(it_or_they, (otmp->quan == 1L) ? "it" : "they");
	Sprintf(eat_it_anyway, "Eat %s anyway?",
		(otmp->quan == 1L) ? "it" : "one");

	if (cadaver || otmp->otyp == EGG || otmp->otyp == TIN) {
		/* These checks must match those in eatcorpse() */
		stoneorslime = (touch_petrifies(&mons[mnum]) &&
				!Stone_resistance &&
				!poly_when_stoned(youmonst.data));

		if (mnum == PM_GREEN_SLIME || mnum == PM_FLUX_SLIME)
		    stoneorslime = (!Unchanging && !flaming(youmonst.data) &&
			youmonst.data != &mons[PM_GREEN_SLIME]);

		if (cadaver && mnum != PM_LIZARD && mnum != PM_SMALL_CAVE_LIZARD && mnum != PM_CAVE_LIZARD 
		&& mnum != PM_LARGE_CAVE_LIZARD && mnum != PM_LICHEN && mnum != PM_BEHOLDER ) {
			long age = peek_at_iced_corpse_age(otmp);
			/* worst case rather than random
			   in this calculation to force prompt */
			rotted = (monstermoves - age)/(10L + 0 /* was rn2(20) */);
			if (otmp->cursed) rotted += 2L;
			else if (otmp->blessed) rotted -= 2L;
		}
	}

	/*
	 * These problems with food should be checked in
	 * order from most detrimental to least detrimental.
	 */

	if (cadaver && mnum != PM_ACID_BLOB && rotted > 5L && !(Sick_resistance || u.sealsActive&SEAL_CHUPOCLOPS)) {
		/* Tainted meat */
		Sprintf(buf, "%s like %s could be tainted! %s",
			foodsmell, it_or_they, eat_it_anyway);
		if (yn_function(buf,ynchars,'n')=='n') return 1;
		else return 2;
	}
	if (stoneorslime) {
		Sprintf(buf, "%s like %s could be something very dangerous! %s",
			foodsmell, it_or_they, eat_it_anyway);
		if (yn_function(buf,ynchars,'n')=='n') return 1;
		else return 2;
	}
	if (otmp->orotten || (cadaver && rotted > 3L)) {
		/* Rotten */
		Sprintf(buf, "%s like %s could be rotten! %s",
			foodsmell, it_or_they, eat_it_anyway);
		if (yn_function(buf,ynchars,'n')=='n') return 1;
		else return 2;
	}
	if (cadaver && poisonous(&mons[mnum]) && !Poison_resistance) {
		/* poisonous */
		Sprintf(buf, "%s like %s might be poisonous! %s",
			foodsmell, it_or_they, eat_it_anyway);
		if (yn_function(buf,ynchars,'n')=='n') return 1;
		else return 2;
	}
	if (cadaver && !vegetarian(&mons[mnum]) &&
	    !u.uconduct.unvegetarian && Role_if(PM_MONK)) {
		Sprintf(buf, "%s unhealthy. %s",
			foodsmell, eat_it_anyway);
		if (yn_function(buf,ynchars,'n')=='n') return 1;
		else return 2;
	}
	if (cadaver && acidic(&mons[mnum]) && !Acid_resistance) {
		Sprintf(buf, "%s rather acidic. %s",
			foodsmell, eat_it_anyway);
		if (yn_function(buf,ynchars,'n')=='n') return 1;
		else return 2;
	}
	if (Upolyd && u.umonnum == PM_RUST_MONSTER &&
	    is_metallic(otmp) && otmp->oerodeproof) {
		Sprintf(buf, "%s disgusting to you right now. %s",
			foodsmell, eat_it_anyway);
		if (yn_function(buf,ynchars,'n')=='n') return 1;
		else return 2;
	}

	/*
	 * Breaks conduct, but otherwise safe.
	 */
	 
	if (!u.uconduct.unvegan && !Race_if(PM_INCANTIFIER) &&
	    ((material == LEATHER || material == BONE ||
	      material == DRAGON_HIDE || material == WAX) ||
	     (cadaver && !vegan(&mons[mnum])))) {
		Sprintf(buf, "%s foul and unfamiliar to you. %s",
			foodsmell, eat_it_anyway);
		if (yn_function(buf,ynchars,'n')=='n') return 1;
		else return 2;
	}
	if (!u.uconduct.unvegetarian && !Race_if(PM_INCANTIFIER) &&
	    ((material == LEATHER || material == BONE ||
	      material == DRAGON_HIDE) ||
	     (cadaver && !vegetarian(&mons[mnum])))) {
		Sprintf(buf, "%s unfamiliar to you. %s",
			foodsmell, eat_it_anyway);
		if (yn_function(buf,ynchars,'n')=='n') return 1;
		else return 2;
	}

	if (cadaver && mnum != PM_ACID_BLOB && rotted > 5L && (Sick_resistance || u.sealsActive&SEAL_CHUPOCLOPS)) {
		/* Tainted meat with Sick_resistance */
		Sprintf(buf, "%s like %s could be tainted! %s",
			foodsmell, it_or_they, eat_it_anyway);
		if (yn_function(buf,ynchars,'n')=='n') return 1;
		else return 2;
	}
	return 0;
}

int
doeat()		/* generic "eat" command funtion (see cmd.c) */
{
	register struct obj *otmp;
	int basenutrit;			/* nutrition of full item */
	int nutrit;			/* nutrition available */
	etype = 0;
	char qbuf[QBUFSZ];
	char c;
	
	boolean dont_start = FALSE;
	
	if (Strangled) {
		pline("If you can't breathe air, how can you consume solids?");
		return 0;
	}

	if(uclockwork){
		long uUpgrades = (u.clockworkUpgrades&(WOOD_STOVE|MAGIC_FURNACE|HELLFIRE_FURNACE|SCRAP_MAW));
		if(!uUpgrades){
			pline("You are metal and springs, not flesh and blood. You cannot eat.");
			return 0;
		} else if(uUpgrades == WOOD_STOVE || 
				uUpgrades == MAGIC_FURNACE || 
				uUpgrades == HELLFIRE_FURNACE || 
				uUpgrades == SCRAP_MAW
		) etype = uUpgrades;
		else etype = clockwork_eat_menu(TRUE,TRUE);
	}
	if (!(otmp = floorfood("eat", 0))) return 0;
	if (check_capacity((char *)0)) return 0;

	if (u.uedibility || u.sealsActive&SEAL_BUER) {
		int res = edibility_prompts(otmp);
		if (res) {
			if(!u.sealsActive&SEAL_BUER){
				Your("%s stops tingling and your sense of smell returns to normal.",
					body_part(NOSE));
				u.uedibility = 0;
			}
		    if (res == 1) return 0;
		}
	}

	/* We have to make non-foods take 1 move to eat, unless we want to
	 * do ridiculous amounts of coding to deal with partly eaten plate
	 * mails, players who polymorph back to human in the middle of their
	 * metallic meal, etc....
	 */
	if (!is_edible(otmp)) {
	    You("cannot eat that!");
	    return 0;
	} else if ((otmp->owornmask & (W_ARMOR|W_TOOL|W_AMUL
#ifdef STEED
			|W_SADDLE
#endif
			)) != 0) {
	    /* let them eat rings */
	    You_cant("eat %s you're wearing.", something);
	    return 0;
	}
	if (is_metallic(otmp) &&
	    u.umonnum == PM_RUST_MONSTER && otmp->oerodeproof) {
	    	otmp->rknown = TRUE;
		if (otmp->quan > 1L) {
		    if(!carried(otmp))
			(void) splitobj(otmp, otmp->quan - 1L);
		    else
			otmp = splitobj(otmp, 1L);
		}
		pline("Ulch - That %s was rustproofed!", xname(otmp));
		/* The regurgitated object's rustproofing is gone now */
		otmp->oerodeproof = 0;
		make_stunned(HStun + rn2(10), TRUE);
		You("spit %s out onto the %s.", the(xname(otmp)),
			surface(u.ux, u.uy));
		if (carried(otmp)) {
			freeinv(otmp);
			dropy(otmp);
		}
		stackobj(otmp);
		return 1;
	}
	if(Race_if(PM_INCANTIFIER)){
		int curspe;
		if(objects[otmp->otyp].oc_unique) return 1;//redundant check against unique
		if(Race_if(PM_INCANTIFIER) && u.uen >= u.uenmax * 3/4 && yn("You feel overcharged. Continue eating?") != 'y'){
			return 0;
		}
		if (otmp->quan > 1L) {
		    if(!carried(otmp))
			(void) splitobj(otmp, otmp->quan - 1L);
		    else
			otmp = splitobj(otmp, 1L);
		if (carried(otmp)) {
			freeinv(otmp);
			if (inv_cnt() >= 52) {
			sellobj_state(SELL_DONTSELL);
			dropy(otmp);
			sellobj_state(SELL_NORMAL);
			} else {
			otmp->oxlth++;		/* hack to prevent merge */
			otmp = addinv(otmp);
			otmp->oxlth--;
			}
		}
		}
		switch(otmp->oclass){
			case WEAPON_CLASS:
				u.uconduct.food++;
				curspe = otmp->spe;
	    	    (void) drain_item(otmp);
				if(curspe > otmp->spe){
					You("drain the %s%s.", xname(otmp),otmp->spe!=0 ? "":" dry");
					lesshungry(50);
				} else {
					pline("The %s resists your attempt to drain its magic.", xname(otmp));
				}
			break;
			case RING_CLASS:
				if(otmp->oartifact) break; //redundant check
				u.uconduct.food++;
				pline("The %s turns to dust as you drain it dry.", xname(otmp));
				eataccessory(otmp);
				if (otmp == uwep && otmp->quan == 1L) uwepgone();
				if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
				if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();

				if (carried(otmp)) useup(otmp);
				else useupf(otmp, 1L);
				lesshungry(50);
			break;
			case AMULET_CLASS:
				if(otmp->oartifact || objects[otmp->otyp].oc_unique) break; //redundant check
				u.uconduct.food++;
				pline("The %s crumbles to dust as you drain it dry.", xname(otmp));
				eataccessory(otmp);
				if (otmp == uwep && otmp->quan == 1L) uwepgone();
				if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
				if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();

				if (carried(otmp)) useup(otmp);
				else useupf(otmp, 1L);
				lesshungry(50);
			break;
			case ARMOR_CLASS:
				u.uconduct.food++;
				curspe = otmp->spe;
	    	    (void) drain_item(otmp);
				if(curspe > otmp->spe){
					You("drain the %s%s.", xname(otmp),otmp->spe!=0 ? "":" dry");
					lesshungry(50);
				} else {
					pline("The %s resists your attempt to drain its magic.", xname(otmp));
				}
			break;
			case TOOL_CLASS:
				u.uconduct.food++;
				curspe = otmp->spe;
	    	    (void) drain_item(otmp);
				if(curspe > otmp->spe){
					You("drain the %s%s.", xname(otmp),otmp->spe!=0 ? "":" dry");
					lesshungry(50);
				} else {
					pline("The %s resists your attempt to drain its magic.", xname(otmp));
				}
			break;
			case SCROLL_CLASS:
				if(otmp->oartifact) break; //redundant check
				u.uconduct.food++;
				You("drain the ink from the %s.", xname(otmp));
				costly_cancel(otmp);
	    	    otmp->otyp = SCR_BLANK_PAPER;
	    	    otmp->spe = 0;
	    	    otmp->ovar1 = 0;
				lesshungry(50);
			break;
			case SPBOOK_CLASS:
				if(otmp->oartifact) break; //redundant check
				u.uconduct.food++;
				You("drain the magic from the %s.", xname(otmp));
				otmp->spestudied++;
				costly_cancel(otmp);
	    	    if(otmp->spestudied > MAX_SPELL_STUDY) otmp->otyp = SPE_BLANK_PAPER;
				lesshungry(50);
			break;
			case WAND_CLASS:
				u.uconduct.food++;
				curspe = otmp->spe;
	    	    (void) drain_item(otmp);
				if(curspe > otmp->spe){
					You("drain the %s%s.", xname(otmp),otmp->spe!=0 ? "":" dry");
					lesshungry(10);
				} else {
					pline("The %s resists your attempt to drain its magic.", xname(otmp));
				}
			break;
			case FOOD_CLASS:
				if(otmp->otyp == CORPSE){
					u.uconduct.food++;
					You("drain the %s.", xname(otmp));
					cprefx(otmp->corpsenm, TRUE, FALSE);
					cpostfx(otmp->corpsenm, FALSE, FALSE, otmp->odrained);
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				}
				if(otmp->otyp == TIN) {
					start_tin(otmp);
					return(1);
				}
			break;
		}
		return 1;
	}
	if(uclockwork){
		if(etype == MAGIC_FURNACE && !otmp->oartifact){
			int curspe;
			if(objects[otmp->otyp].oc_unique) return 1;//redundant check against unique
			You("place the %s in your magic furnace.", xname(otmp));
			if (otmp->quan > 1L) {
				if(!carried(otmp))
				(void) splitobj(otmp, otmp->quan - 1L);
				else
				otmp = splitobj(otmp, 1L);
			if (carried(otmp)) {
				freeinv(otmp);
				if (inv_cnt() >= 52) {
				sellobj_state(SELL_DONTSELL);
				dropy(otmp);
				sellobj_state(SELL_NORMAL);
				} else {
				otmp->oxlth++;		/* hack to prevent merge */
				otmp = addinv(otmp);
				otmp->oxlth--;
				}
			}
			}
			switch(otmp->oclass){
				case WEAPON_CLASS:
					u.uconduct.food++;
					u.uen = min(u.uenmax, u.uen + otmp->spe*50);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case RING_CLASS:
					u.uconduct.food++;
					u.uen = min(u.uenmax, u.uen + 50);
					eataccessory(otmp);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case AMULET_CLASS:
					u.uconduct.food++;
					u.uen = min(u.uenmax, u.uen + 50);
					eataccessory(otmp);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case ARMOR_CLASS:
					u.uconduct.food++;
					u.uen = min(u.uenmax, u.uen + otmp->spe*50);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case TOOL_CLASS:
					u.uconduct.food++;
					u.uen = min(u.uenmax, u.uen + otmp->spe*50);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case SCROLL_CLASS:
					u.uconduct.food++;
					u.uen = min(u.uenmax, u.uen + 50);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case SPBOOK_CLASS:
					u.uconduct.food++;
					u.uen = min(u.uenmax, u.uen + 150);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case WAND_CLASS:
					u.uconduct.food++;
					u.uen = min(u.uenmax, u.uen + otmp->spe*10);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case FOOD_CLASS:
					if(otmp->otyp == CORPSE){
						u.uconduct.food++;
						cpostfx(otmp->corpsenm, FALSE, FALSE, otmp->odrained);
						if (otmp == uwep && otmp->quan == 1L) uwepgone();
						if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
						if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
						if (carried(otmp)) useup(otmp);
						else useupf(otmp, 1L);
					}
				break;
			}
			return 1;
		}
		if(etype == WOOD_STOVE && !otmp->oartifact){
			if(objects[otmp->otyp].oc_unique) return 1;//redundant check against unique
			You("place the %s in your wood stove.", xname(otmp));
			if (otmp->quan > 1L) {
				if(!carried(otmp))
					(void) splitobj(otmp, otmp->quan - 1L);
				else
					otmp = splitobj(otmp, 1L);
				if (carried(otmp)) {
					freeinv(otmp);
					if (inv_cnt() >= 52) {
						sellobj_state(SELL_DONTSELL);
						dropy(otmp);
						sellobj_state(SELL_NORMAL);
					} else {
						otmp->oxlth++;		/* hack to prevent merge */
						otmp = addinv(otmp);
						otmp->oxlth--;
					}
				}
			}
			switch(otmp->oclass){
				case WEAPON_CLASS:
					u.uconduct.food++;
					u.ustove += objects[otmp->otyp].oc_nutrition;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case RING_CLASS:
					u.uconduct.food++;
					u.ustove += objects[otmp->otyp].oc_nutrition;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case AMULET_CLASS:
					u.uconduct.food++;
					u.ustove += objects[otmp->otyp].oc_nutrition;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case ARMOR_CLASS:
					u.uconduct.food++;
					u.ustove += objects[otmp->otyp].oc_nutrition;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case TOOL_CLASS:
					u.uconduct.food++;
					u.ustove += objects[otmp->otyp].oc_nutrition;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case SCROLL_CLASS:
					u.uconduct.food++;
					u.ustove += objects[otmp->otyp].oc_nutrition;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case SPBOOK_CLASS:
					u.uconduct.food++;
					u.ustove += objects[otmp->otyp].oc_nutrition;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case WAND_CLASS:
					u.uconduct.food++;
					u.ustove += objects[otmp->otyp].oc_nutrition;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case FOOD_CLASS:
					if(otmp->otyp == CORPSE){
						u.uconduct.food++;
						if (!vegan(&mons[otmp->corpsenm])) u.uconduct.unvegan++;
						if (!vegetarian(&mons[otmp->corpsenm])) violated_vegetarian();
						u.ustove += otmp->oeaten ? otmp->oeaten : mons[otmp->corpsenm].cnutrit;
						if (otmp == uwep && otmp->quan == 1L) uwepgone();
						if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
						if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
						if (carried(otmp)) useup(otmp);
						else useupf(otmp, 1L);
					} else {
						u.uconduct.food++;
						u.ustove += objects[otmp->otyp].oc_nutrition;
						if (otmp == uwep && otmp->quan == 1L) uwepgone();
						if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
						if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
						if (carried(otmp)) useup(otmp);
						else useupf(otmp, 1L);
					}
				break;
			}
			return 1;
		}
		if(etype == HELLFIRE_FURNACE && !otmp->oartifact){
			if(objects[otmp->otyp].oc_unique) return 1;//redundant check against unique
			You("place the %s in your hellfire furnace.", xname(otmp));
			if (otmp->quan > 1L) {
				if(!carried(otmp))
					(void) splitobj(otmp, otmp->quan - 1L);
				else
					otmp = splitobj(otmp, 1L);
				if (carried(otmp)) {
					freeinv(otmp);
					if (inv_cnt() >= 52) {
						sellobj_state(SELL_DONTSELL);
						dropy(otmp);
						sellobj_state(SELL_NORMAL);
					} else {
						otmp->oxlth++;		/* hack to prevent merge */
						otmp = addinv(otmp);
						otmp->oxlth--;
					}
				}
			}
			switch(otmp->oclass){
				case WEAPON_CLASS:
					u.uconduct.food++;
					u.ustove += (objects[otmp->otyp].oc_nutrition)/10+1;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case RING_CLASS:
					u.uconduct.food++;
					u.ustove += (objects[otmp->otyp].oc_nutrition)/10+1;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case AMULET_CLASS:
					u.uconduct.food++;
					u.ustove += (objects[otmp->otyp].oc_nutrition)/10+1;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case ARMOR_CLASS:
					u.uconduct.food++;
					u.ustove += (objects[otmp->otyp].oc_nutrition)/10+1;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case TOOL_CLASS:
					u.uconduct.food++;
					u.ustove += (objects[otmp->otyp].oc_nutrition)/10+1;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case SCROLL_CLASS:
					u.uconduct.food++;
					u.ustove += (objects[otmp->otyp].oc_nutrition)/10+1;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case SPBOOK_CLASS:
					u.uconduct.food++;
					u.ustove += (objects[otmp->otyp].oc_nutrition)/10+1;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case WAND_CLASS:
					u.uconduct.food++;
					u.ustove += (objects[otmp->otyp].oc_nutrition)/10+1;
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case FOOD_CLASS:
					if(otmp->otyp == CORPSE){
						u.uconduct.food++;
						if (!vegan(&mons[otmp->corpsenm])) u.uconduct.unvegan++;
						if (!vegetarian(&mons[otmp->corpsenm])) violated_vegetarian();
						u.ustove += otmp->oeaten ? ((otmp->oeaten)/10+1) : ((mons[otmp->corpsenm].cnutrit)/10+1);
						if (otmp == uwep && otmp->quan == 1L) uwepgone();
						if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
						if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
						if (carried(otmp)) useup(otmp);
						else useupf(otmp, 1L);
					} else {
						u.uconduct.food++;
						u.ustove += (objects[otmp->otyp].oc_nutrition)/10+1;
						if (otmp == uwep && otmp->quan == 1L) uwepgone();
						if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
						if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
						if (carried(otmp)) useup(otmp);
						else useupf(otmp, 1L);
					}
				break;
			}
			return 1;
		}
		if(etype == SCRAP_MAW && !otmp->oartifact){
			if(objects[otmp->otyp].oc_unique) return 1;//redundant check against unique
			You("crunch up the %s with your scrap maw.", xname(otmp));
			if (otmp->quan > 1L) {
				if(!carried(otmp))
					(void) splitobj(otmp, otmp->quan - 1L);
				else
					otmp = splitobj(otmp, 1L);
				if (carried(otmp)) {
					freeinv(otmp);
					if (inv_cnt() >= 52) {
						sellobj_state(SELL_DONTSELL);
						dropy(otmp);
						sellobj_state(SELL_NORMAL);
					} else {
						otmp->oxlth++;		/* hack to prevent merge */
						otmp = addinv(otmp);
						otmp->oxlth--;
					}
				}
			}
			switch(otmp->oclass){
				case WEAPON_CLASS:
					u.uconduct.food++;
					if(Upolyd) u.mh = min(u.mhmax, u.mh+objects[otmp->otyp].oc_nutrition);
					else u.uhp = min(u.uhpmax, u.uhp+objects[otmp->otyp].oc_nutrition);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case RING_CLASS:
					u.uconduct.food++;
					if(Upolyd) u.mh = min(u.mhmax, u.mh+objects[otmp->otyp].oc_nutrition);
					else u.uhp = min(u.uhpmax, u.uhp+objects[otmp->otyp].oc_nutrition);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case AMULET_CLASS:
					u.uconduct.food++;
					if(Upolyd) u.mh = min(u.mhmax, u.mh+objects[otmp->otyp].oc_nutrition);
					else u.uhp = min(u.uhpmax, u.uhp+objects[otmp->otyp].oc_nutrition);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case ARMOR_CLASS:
					u.uconduct.food++;
					if(Upolyd) u.mh = min(u.mhmax, u.mh+objects[otmp->otyp].oc_nutrition);
					else u.uhp = min(u.uhpmax, u.uhp+objects[otmp->otyp].oc_nutrition);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case TOOL_CLASS:
					u.uconduct.food++;
					if(Upolyd) u.mh = min(u.mhmax, u.mh+objects[otmp->otyp].oc_nutrition);
					else u.uhp = min(u.uhpmax, u.uhp+objects[otmp->otyp].oc_nutrition);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case SCROLL_CLASS:
					u.uconduct.food++;
					if(Upolyd) u.mh = min(u.mhmax, u.mh+objects[otmp->otyp].oc_nutrition);
					else u.uhp = min(u.uhpmax, u.uhp+objects[otmp->otyp].oc_nutrition);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case SPBOOK_CLASS:
					u.uconduct.food++;
					if(Upolyd) u.mh = min(u.mhmax, u.mh+objects[otmp->otyp].oc_nutrition);
					else u.uhp = min(u.uhpmax, u.uhp+objects[otmp->otyp].oc_nutrition);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case WAND_CLASS:
					u.uconduct.food++;
					if(Upolyd) u.mh = min(u.mhmax, u.mh+objects[otmp->otyp].oc_nutrition);
					else u.uhp = min(u.uhpmax, u.uhp+objects[otmp->otyp].oc_nutrition);
					if (otmp == uwep && otmp->quan == 1L) uwepgone();
					if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
					if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
					if (carried(otmp)) useup(otmp);
					else useupf(otmp, 1L);
				break;
				case FOOD_CLASS:
					if(otmp->otyp == CORPSE){
						u.uconduct.food++;
						if (!vegan(&mons[otmp->corpsenm])) u.uconduct.unvegan++;
						if (!vegetarian(&mons[otmp->corpsenm])) violated_vegetarian();
						if(Upolyd) u.mh += otmp->oeaten ? ((otmp->oeaten)/10+1) : ((mons[otmp->corpsenm].cnutrit)/10+1);
						else u.uhp += otmp->oeaten ? ((otmp->oeaten)/10+1) : ((mons[otmp->corpsenm].cnutrit)/10+1);
						if(Upolyd) u.mh = min(u.mhmax, u.mh);
						else u.uhp = min(u.uhpmax, u.uhp);
						if (otmp == uwep && otmp->quan == 1L) uwepgone();
						if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
						if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
						if (carried(otmp)) useup(otmp);
						else useupf(otmp, 1L);
					} else {
						u.uconduct.food++;
						if(Upolyd) u.mh = min(u.mhmax, u.mh+objects[otmp->otyp].oc_nutrition);
						else u.uhp = min(u.uhpmax, u.uhp+objects[otmp->otyp].oc_nutrition);
						if (otmp == uwep && otmp->quan == 1L) uwepgone();
						if (otmp == uquiver && otmp->quan == 1L) uqwepgone();
						if (otmp == uswapwep && otmp->quan == 1L) uswapwepgone();
						if (carried(otmp)) useup(otmp);
						else useupf(otmp, 1L);
					}
				break;
			}
			return 1;
		}
	}
	/* KMH -- Slow digestion is... indigestible */
	if (otmp->otyp == RIN_SLOW_DIGESTION) {
		pline("This ring is indigestible!");
		(void) rottenfood(otmp);
		if (otmp->dknown && !objects[otmp->otyp].oc_name_known
				&& !objects[otmp->otyp].oc_uname)
			docall(otmp);
		return (1);
	}
	if (otmp->oclass != FOOD_CLASS) {
	    int material;
	    victual.reqtime = 1;
	    victual.piece = otmp;
		/* Don't split it, we don't need to if it's 1 move */
	    victual.usedtime = 0;
	    victual.canchoke = (u.uhs == SATIATED);
		/* Note: gold weighs 1 pt. for each 1000 pieces (see */
		/* pickup.c) so gold and non-gold is consistent. */
	    if (otmp->oclass == COIN_CLASS)
		basenutrit = ((otmp->quan > ((long)u.uhungermax)*100L) ? u.uhungermax
			: (int)(otmp->quan/100L));
	    else if(otmp->oclass == BALL_CLASS || (otmp->oclass == CHAIN_CLASS && weight(otmp) > objects[otmp->otyp].oc_nutrition))
		basenutrit = weight(otmp);
	    /* oc_nutrition is usually weight anyway */
	    else basenutrit = objects[otmp->otyp].oc_nutrition;
	    victual.nmod = basenutrit;
	    victual.eating = TRUE; /* needed for lesshungry() */

	    material = objects[otmp->otyp].oc_material;
	    if (material == LEATHER ||
		material == BONE || material == DRAGON_HIDE) {
		u.uconduct.unvegan++;
		violated_vegetarian();
	    } else if (material == WAX)
			u.uconduct.unvegan++;
	    u.uconduct.food++;
	    
	    if (otmp->cursed)
		(void) rottenfood(otmp);

	    if (otmp->oclass == WEAPON_CLASS && otmp->opoisoned) {
			if(otmp->opoisoned & OPOISON_BASIC){
				pline("Ecch - that must have been poisonous!");
				if(!Poison_resistance) {
					losestr(rnd(4));
					losehp(rnd(15), xname(otmp), KILLED_BY_AN);
				} else
					You("seem unaffected by the poison.");
			}
			else if(otmp->opoisoned & OPOISON_FILTH){
				pline("Ulch - that was tainted with filth!");
				if (Sick_resistance) {
					pline("It doesn't seem at all sickening, though...");
				} else {
					char buf[BUFSZ];
					long sick_time;

					sick_time = (long) rn1(10, 10);
					/* make sure new ill doesn't result in improvement */
					if (Sick && (sick_time > Sick))
						sick_time = (Sick > 1L) ? Sick - 1L : 1L;
					Sprintf(buf, "filth-crusted %s", xname(otmp));
					make_sick(sick_time, buf, TRUE, SICK_VOMITABLE);
				}
			}
			else if(otmp->opoisoned & OPOISON_SLEEP){
				pline("Ecch - that must have been drugged!");
				if(Poison_resistance || Sleep_resistance) {
					You("suddenly fall asleep!");
					fall_asleep(-rn1(10, 25 - 12*bcsign(otmp)), TRUE);
				} else
					You("seem unaffected by the drugs.");
			}
			else if(otmp->opoisoned & OPOISON_BLIND){
				pline("Ecch - that must have been poisoned!");
				make_blinded(rn1(200, 250 - 125 * bcsign(otmp)),
						 (boolean)!Blind);
			}
			else if(otmp->opoisoned & OPOISON_PARAL){
				if (Free_action)
					You("stiffen momentarily.");
				else {
					if (Levitation || Is_airlevel(&u.uz)||Is_waterlevel(&u.uz))
					You("are motionlessly suspended.");
#ifdef STEED
					else if (u.usteed)
					You("are frozen in place!");
#endif
					else
					Your("%s are frozen to the %s!",
						 makeplural(body_part(FOOT)), surface(u.ux, u.uy));
					nomul(-(rn1(10, 25 - 12*bcsign(otmp))), "paralyzed from eating an envenomed weapon");
					nomovemsg = You_can_move_again;
					exercise(A_DEX, FALSE);
				}
			}
			else if(otmp->opoisoned & OPOISON_AMNES){
				forget((!otmp->blessed? ALL_SPELLS : 0) | ALL_MAP);
				if (Hallucination)
					pline("Hakuna matata!");
				else
					You_feel("your memories dissolve.");

				/* Blessed amnesia makes you forget lycanthropy, sickness */
				if (otmp->blessed) {
					if (u.ulycn >= LOW_PM && !Race_if(PM_HUMAN_WEREWOLF)) {
					You("forget your affinity to %s!",
							makeplural(mons[u.ulycn].mname));
					if (youmonst.data == &mons[u.ulycn])
						you_unwere(FALSE);
					u.ulycn = NON_PM;	/* cure lycanthropy */
					}
					make_sick(0L, (char *) 0, TRUE, SICK_ALL);

					/* You feel refreshed */
					if(Race_if(PM_INCANTIFIER)) u.uen += 50 + rnd(50);
					else u.uhunger += 50 + rnd(50);
					newuhs(FALSE);
				} else
					exercise(A_WIS, FALSE);
			}
		} else if (!otmp->cursed)
		pline("This %s is delicious!",
		      otmp->oclass == COIN_CLASS ? foodword(otmp) :
		      singular(otmp, xname));

	    eatspecial();
	    return 1;
	}

	/* [ALI] Hero polymorphed in the meantime.
	 */
	if (otmp == victual.piece &&
	  maybe_polyd(is_vampire(youmonst.data), 
	  Race_if(PM_VAMPIRE)) != otmp->odrained)
	    victual.piece = (struct obj *)0;	/* Can't resume */

	/* [ALI] Blood can coagulate during the interruption
	 *       but not during the draining process.
	 */
	if(otmp == victual.piece && otmp->odrained &&
	  (peek_at_iced_corpse_age(otmp) + victual.usedtime + 5) < monstermoves)
	    victual.piece = (struct obj *)0;	/* Can't resume */

	if(otmp == victual.piece) {
	/* If they weren't able to choke, they don't suddenly become able to
	 * choke just because they were interrupted.  On the other hand, if
	 * they were able to choke before, if they lost food it's possible
	 * they shouldn't be able to choke now.
	 */
	    if (u.uhs != SATIATED) victual.canchoke = FALSE;
	    victual.piece = touchfood(otmp);
	    You("resume your meal.");
	    start_eating(victual.piece);
	    return(1);
	}

	/* nothing in progress - so try to find something. */
	/* tins are a special case */
	/* tins must also check conduct separately in case they're discarded */
	if(otmp->otyp == TIN) {
	    start_tin(otmp);
	    return(1);
	}

	/* KMH, conduct */
	u.uconduct.food++;
	
	if(otmp->otyp == EGG && u.sealsActive&SEAL_ECHIDNA) unbind(SEAL_ECHIDNA,TRUE);
	if(otmp->otyp >= APPLE && otmp->otyp <= BANANA && u.sealsActive&SEAL_EVE) unbind(SEAL_EVE,TRUE);
	if(is_giant(&mons[otmp->corpsenm]) && u.sealsActive&SEAL_YMIR) unbind(SEAL_YMIR,TRUE);

	victual.piece = otmp = touchfood(otmp);
	victual.usedtime = 0;

	/* Now we need to calculate delay and nutritional info.
	 * The base nutrition calculated here and in eatcorpse() accounts
	 * for normal vs. rotten food.  The reqtime and nutrit values are
	 * then adjusted in accordance with the amount of food left.
	 */
	if(otmp->otyp == CORPSE) {
	    int tmp = eatcorpse(otmp);
	    if (tmp == 3) {
		/* inedible */
		victual.piece = (struct obj *)0;
		/*
		 * The combination of odrained == TRUE and oeaten == cnutrit
		 * represents the case of starting to drain a corpse but not
		 * getting any further (eg., loosing consciousness due to
		 * rotten food). We must preserve this case to avoid corpses
		 * changing appearance after a failed attempt to eat.
		 */
		if (!otmp->odrained &&
			otmp->oeaten == mons[otmp->corpsenm].cnutrit)
		    otmp->oeaten = 0;
		/* ALI, conduct: didn't eat it after all */
		u.uconduct.food--;
		return 0;
	    } else if (tmp == 2) {
		/* used up */
		victual.piece = (struct obj *)0;
		return(1);
	    } else if (tmp)
		dont_start = TRUE;
	    /* if not used up, eatcorpse sets up reqtime and may modify
	     * oeaten */
	} else {
	    /* No checks for WAX, LEATHER, BONE, DRAGON_HIDE.  These are
	     * all handled in the != FOOD_CLASS case, above */
	    switch (objects[otmp->otyp].oc_material) {
	    case FLESH:
		u.uconduct.unvegan++;
		if (otmp->otyp != EGG) {
		    violated_vegetarian();
		}
		break;

	    default:
		if (otmp->otyp == PANCAKE ||
		    otmp->otyp == FORTUNE_COOKIE || /* eggs */
		    otmp->otyp == CREAM_PIE ||
		    otmp->otyp == CANDY_BAR || /* milk */
		    otmp->otyp == LUMP_OF_ROYAL_JELLY)
		    u.uconduct.unvegan++;
		break;
	    }

	    victual.reqtime = (u.sealsActive&SEAL_AHAZU) ? 1 : objects[otmp->otyp].oc_delay;
	    if (otmp->otyp != FORTUNE_COOKIE &&
		(otmp->cursed ||
		 (((monstermoves - otmp->age) > (int) otmp->blessed ? 50:30) &&
		(otmp->orotten || !rn2(7))))) {

		if (rottenfood(otmp)) {
		    otmp->orotten = TRUE;
		    dont_start = TRUE;
		}
		if (otmp->oeaten < 2) {
		    victual.piece = (struct obj *)0;
		    if (carried(otmp)) useup(otmp);
		    else useupf(otmp, 1L);
		    return 1;
		} else
		consume_oeaten(otmp, 1);	/* oeaten >>= 1 */
	    } else fprefx(otmp);
	}

	/* re-calc the nutrition */
	if (otmp->otyp == CORPSE) basenutrit = mons[otmp->corpsenm].cnutrit;
	else basenutrit = objects[otmp->otyp].oc_nutrition;
	nutrit = otmp->oeaten;
	if (otmp->otyp == CORPSE && otmp->odrained) {
	    nutrit -= drainlevel(otmp);
	    basenutrit -= drainlevel(otmp);
	}

#ifdef DEBUG
	debugpline("before rounddiv: victual.reqtime == %d", victual.reqtime);
	debugpline("oeaten == %d, basenutrit == %d", otmp->oeaten, basenutrit);
	debugpline("nutrit == %d, cnutrit == %d", nutrit, otmp->otyp == CORPSE ?
	  mons[otmp->corpsenm].cnutrit : objects[otmp->otyp].oc_nutrition);
#endif
	victual.reqtime = (basenutrit == 0 ? 0 : (u.sealsActive&SEAL_AHAZU) ? 1 : 
		rounddiv(victual.reqtime * (long)nutrit, basenutrit));
#ifdef DEBUG
	debugpline("after rounddiv: victual.reqtime == %d", victual.reqtime);
#endif
	/* calculate the modulo value (nutrit. units per round eating)
	 * [ALI] Note: although this is not exact, the remainder is
	 *       now dealt with in done_eating().
	 */
	if (victual.reqtime == 0 || nutrit == 0)
	    /* possible if most has been eaten before */
	    victual.nmod = 0;
	else if (nutrit >= victual.reqtime)
	    victual.nmod = -(nutrit / victual.reqtime);
	else
	    victual.nmod = victual.reqtime % nutrit;
	victual.canchoke = (u.uhs == SATIATED);

	if (!dont_start) start_eating(otmp);
	return(1);
}

/* Take a single bite from a piece of food, checking for choking and
 * modifying usedtime.  Returns 1 if they choked and survived, 0 otherwise.
 */
STATIC_OVL int
bite()
{
	if(victual.canchoke && 
		((Race_if(PM_INCANTIFIER) && u.uen >= u.uenmax) ||
		 (!Race_if(PM_INCANTIFIER)&& u.uhunger >= u.uhungermax) )
		) {
		choke(victual.piece);
		return 1;
	}
	if (victual.doreset) {
		do_reset_eat();
		return 0;
	}
	force_save_hs = TRUE;
	if(victual.nmod < 0) {
		lesshungry(-victual.nmod);
		consume_oeaten(victual.piece, victual.nmod); /* -= -nmod */
	} else if(victual.nmod > 0 && (victual.usedtime % victual.nmod)) {
		lesshungry(1);
		consume_oeaten(victual.piece, -1);		  /* -= 1 */
	}
	force_save_hs = FALSE;
	recalc_wt();
	return 0;
}

#endif /* OVLB */
#ifdef OVL0

void
gethungry()	/* as time goes by - called by moveloop() and domove() */
{
	if (u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves+20) return;	/* you don't feel hungrier */
	
	if ((!u.usleep || !rn2(10))	/* slow metabolic rate while asleep */
		&& (carnivorous(youmonst.data) 
			|| herbivorous(youmonst.data) 
			|| maybe_polyd(is_vampire(youmonst.data), 
							Race_if(PM_VAMPIRE)))
#ifdef CONVICT
        /* Convicts can last twice as long at hungry and below */
        && (!Role_if(PM_CONVICT) || (u.uhs < HUNGRY) || (moves % 2 && u.uhs == HUNGRY) || (moves % 5))
#endif /* CONVICT */
        && (!Race_if(PM_VAMPIRE) || !(moves % (u.ulevel/10 + 1)))
		&& !( (Slow_digestion && !Race_if(PM_INCANTIFIER)) ||
				(Race_if(PM_INCANTIFIER) && moves%10) || 
				(uclockwork) ))
			(Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);		/* ordinary food consumption */
	if(uwep && (
		uwep->oartifact == ART_GARNET_ROD || (uwep->oartifact == ART_TENSA_ZANGETSU && !is_undead(youmonst.data)))
	){
		if(Race_if(PM_INCANTIFIER)) u.uen -= 9;
		else u.uhunger -= 9;
	}
	if(uclockwork){
		if(u.ucspeed == SLOW_CLOCKSPEED && !(moves%(10+u.slowclock))) (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
		else{
			if(u.slowclock < 10){
				if(moves%10 >= u.slowclock) (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
			} else if(!(moves%u.slowclock)) (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
		}
	}
	if (moves % 2) {	/* odd turns */
	    /* Regeneration uses up food, unless due to an artifact */
	    if ( (HRegeneration && !maybe_polyd(is_vampire(youmonst.data), Race_if(PM_VAMPIRE))) || ((ERegeneration & (~W_ART)) &&
				(ERegeneration != W_WEP || !uwep->oartifact) &&
				(ERegeneration != W_ARMS || !uarms->oartifact) 
				))
			(Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
	    if (near_capacity() > SLT_ENCUMBER) (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
	} else {		/* even turns */
	    if (Hunger) (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
	    if (u.sealsActive&SEAL_AHAZU) (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
	    /* Conflict uses up food too */
	    if (HConflict || (EConflict & (~W_ARTI))) (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
	    /* Alacrity uses up food too */
	    if (EFast & (W_RINGL|W_RINGR)) (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
	    /* +0 charged rings don't do anything, so don't affect hunger */
	    /* Slow digestion still uses ring hunger */
	    switch ((int)(moves % 20)) {	/* note: use even cases only */
	     case  4: if (uleft &&
			  (uleft->spe || !objects[uleft->otyp].oc_charged))
			    (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
		    break;
	     case  8: if (uamul) (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
		    break;
	     case 12: if (uright &&
			  (uright->spe || !objects[uright->otyp].oc_charged))
			    (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
		    break;
	     case 16: if (u.uhave.amulet) (Race_if(PM_INCANTIFIER) ? u.uen-- : u.uhunger--);
		    break;
	     default: break;
	    }
	}
	newuhs(TRUE);
}

#endif /* OVL0 */
#ifdef OVLB

void
morehungry(num)	/* called after vomiting and after performing feats of magic */
register int num;
{
	if(Race_if(PM_INCANTIFIER)) u.uen -= num;
	else u.uhunger -= num;
	newuhs(TRUE);
}


void
lesshungry(num)	/* called after eating (and after drinking fruit juice) */
register int num;
{
	/* See comments in newuhs() for discussion on force_save_hs */
	boolean iseating = (occupation == eatfood) || force_save_hs;
#ifdef DEBUG
	debugpline("lesshungry(%d)", num);
#endif
	if(Race_if(PM_INCANTIFIER)) u.uen += num;
	else u.uhunger += num;
	if(((Race_if(PM_INCANTIFIER) && u.uen >= u.uenmax) ||
		 (!Race_if(PM_INCANTIFIER)&& u.uhunger >= u.uhungermax) )
		) {
	    if (!iseating || victual.canchoke) {
		if (iseating) {
		    choke(victual.piece);
		    reset_eat();
		} else
		    if(!uclockwork) choke(occupation == opentin ? tin.tin : (struct obj *)0);
			else{
				Your("mainspring is wound too tight!");
				Your("clockwork breaks apart!");
				killer_format = KILLED_BY;
				killer = "overwinding";
				done(OVERWOUND);
				victual.piece = 0;
				victual.mon = 0;
				return;
			}
		/* no reset_eat() */
	    }
	} else {
	    /* Have lesshungry() report when you're nearly full so all eating
	     * warns when you're about to choke.
	     */
	    if ((Race_if(PM_INCANTIFIER) && u.uen >= u.uenmax * 3/4) ||
			(!Race_if(PM_INCANTIFIER) && u.uhunger >= u.uhungermax * 3/4)) {
			if (!victual.eating || (victual.eating && !victual.fullwarn)) {
				if(!uclockwork){
					pline("You're having a hard time getting all of it down.");
					nomovemsg = "You're finally finished.";
				}
				else{
					Your("mainspring is being overwound!");
					nomovemsg = "The winding finally stopped.";
				}
				if (!victual.eating)
					multi = -2;
				else {
					victual.fullwarn = TRUE;
					if (victual.canchoke && victual.reqtime > 1) {
						/* a one-gulp food will not survive a stop */
						if (yn_function("Stop eating?",ynchars,'y')=='y') {
						reset_eat();
						nomovemsg = (char *)0;
						}
					}
				}
			}
	    }
	}
	newuhs(FALSE);
}

STATIC_PTR
int
unfaint(VOID_ARGS)
{
	(void) Hear_again();
	if(u.uhs > FAINTING)
		u.uhs = FAINTING;
	stop_occupation();
	flags.botl = 1;
	return 0;
}

int
ask_turns(mon, flatrate, perturn)
struct monst *mon;
int flatrate;
int perturn;
{
	int ret, turns;
	long price;
	char buf[BUFSZ], qbuf[QBUFSZ];
	boolean pay;
	
	Sprintf(qbuf, "How many turns?");
	getlin(qbuf, buf);
	(void)mungspaces(buf);
	if (buf[0] == '\033' || buf[0] == '\0') ret = 0;
	else ret = sscanf(buf, "%d", &turns);
	
	if (ret != 1 || turns <= 0){
		return 0;
	}
	
	if(turns > 2000){
		pline("\"That sounds really boring.\"");
		return 0;
	}
	
	price = (long)(turns*perturn + flatrate);
	
	if(price < 0){
		You("couldn't have enough %s to pay for that!", currency(price));
		return 0;
	}
	
	if(price){
	Sprintf(qbuf, "That will be %ld %s. (Pay?)", price, currency(price));
	if(yn(qbuf)=='y'){
#ifndef GOLDOBJ
		if (price >= u.ugold) {
			You("don't have enough %s!", currency(price));
			return 0;
		} else {
			You("give %s %ld %s.", mon_nam(mon), price, currency(price));
		}
		u.ugold -= price;
		mon->mgold += price;
#else
		if (price >= umoney) {
			You("don't have enough %s!", currency(price));
			return 0;
		} else {
			You("give %s %ld %s.", mon_nam(mon), price, currency(price));
		}
		(void) money2mon(mon, price);
#endif
	return turns;
	} else{
		return 0;
	}
	}
	else return turns;
}

int
ask_cp(mon, flatrate)
struct monst *mon;
int flatrate;
{
	int ret, howmany;
	long price;
	char buf[BUFSZ], qbuf[QBUFSZ];
	boolean pay;
	
	Sprintf(qbuf, "How many components?");
	getlin(qbuf, buf);
	(void)mungspaces(buf);
	if (buf[0] == '\033' || buf[0] == '\0') ret = 0;
	else ret = sscanf(buf, "%d", &howmany);
	
	if (ret != 1 || howmany <= 0){
		return 0;
	}
	
	if(howmany > 100){
		pline("\"That's too many all at once.\"");
		return 0;
	}
	
	price = (long)(howmany*flatrate);
	
	if(price < 0){
		You("couldn't have enough %s to pay for that!", currency(price));
		return 0;
	}
	
	if(price){
	Sprintf(qbuf, "That will be %ld %s. (Pay?)", price, currency(price));
	if(yn(qbuf)=='y'){
#ifndef GOLDOBJ
		if (price >= u.ugold) {
			You("don't have enough %s!", currency(price));
			return 0;
		} else {
			You("give %s %ld %s.", mon_nam(mon), price, currency(price));
		}
		u.ugold -= price;
		mon->mgold += price;
#else
		if (price >= umoney) {
			You("don't have enough %s!", currency(price));
			return 0;
		} else {
			You("give %s %ld %s.", mon_nam(mon), price, currency(price));
		}
		(void) money2mon(mon, price);
#endif
	return howmany;
	} else{
		return 0;
	}
	}
	else return howmany;
}

int
start_clockwinding(key, mon, turns)
struct obj * key;
struct monst *mon;
int turns;
{
  // int ret;
  // char buf[BUFSZ], qbuf[QBUFSZ];
  // Sprintf(qbuf, "How many turns?");
  // getlin(qbuf, buf);
  // (void)mungspaces(buf);
  // if (buf[0] == '\033' || buf[0] == '\0') ret = 0;
  // else ret = sscanf(buf, "%d", &turns);

  // if (ret != 1 || turns <= 0){
    // pline(Never_mind);
    // return 0;
  // }

  if (key->otyp != SKELETON_KEY)
    return 0;
  pline("%s uses the key to wind up your clockwork.", Monnam(mon));
  victual.piece = key;
  victual.canchoke = TRUE;
  victual.usedtime = 0;
  victual.fullwarn = FALSE;
  victual.reqtime = turns;
  victual.mon = mon;
  mon->moccupation = 1;
  mon->mcanmove = 0;
  Sprintf(msgbuf, "being wound");
  set_occupation(windclock, msgbuf, 0);
  return 1;
}



STATIC_PTR
int
windclock()
{
  if (victual.reqtime == victual.usedtime){
    occupation = 0;
	pline("%s finishes winding up your clockwork.", Monnam(victual.mon));
	victual.mon->moccupation = 0;
	victual.mon->mcanmove = 1;
    newuhs(FALSE);
    victual.piece = 0;
    victual.mon = 0;
  }else if (!carried(victual.piece)){
    newuhs(FALSE);
    stop_occupation();
    victual.piece = 0;
    victual.mon = 0;
    return 0;
  }else if(victual.canchoke && u.uhunger >= u.uhungermax && !Race_if(PM_INCANTIFIER)) {
    Your("mainspring is wound too tight!");
    Your("clockwork breaks apart!");
    killer_format = KILLED_BY;
    killer = "overwinding";
    done(OVERWOUND);
    victual.piece = 0;
    victual.mon = 0;
    return 0;
  }
  else if (u.uhunger >= u.uhungermax * 3/4 && !victual.fullwarn && !Race_if(PM_INCANTIFIER)) {
    pline("%s is having a hard time cranking the key.",Monnam(victual.mon));
    victual.fullwarn = TRUE;
  }
  else {
	if(victual.reqtime - victual.usedtime >= 10){
		if(!Race_if(PM_INCANTIFIER)) u.uhunger += 100;
		victual.usedtime += 10;
	} else {
		if(!Race_if(PM_INCANTIFIER)) u.uhunger += (victual.reqtime - victual.usedtime) * 10;
		victual.usedtime += (victual.reqtime - victual.usedtime);
	}
  }
  flags.botl = 1;
  return 1;
}
#endif /* OVLB */
#ifdef OVL0

boolean
is_fainted()
{
	return((boolean)(u.uhs == FAINTED));
}

void
reset_faint()	/* call when a faint must be prematurely terminated */
{
	if(is_fainted()) nomul(0, NULL);
}

#if 0
void
sync_hunger()
{

	if(is_fainted()) {
		flags.soundok = 0;
		if(Role_if(PM_CONVICT)) nomul(-1+( YouHunger/20), "fainted from lack of food");
		else nomul(-10+( YouHunger/10), "fainted from lack of food");
		nomovemsg = "You regain consciousness.";
		afternmv = unfaint;
	}
}
#endif

void
newuhs(incr)		/* compute and comment on your (new?) hunger status */
boolean incr;
{
	unsigned newhs;
	static unsigned save_hs;
	static boolean saved_hs = FALSE;
	int h = YouHunger;
     boolean clockwork = uclockwork;

	newhs = (h > (Race_if(PM_INCANTIFIER) ? max(u.uenmax/2,200) : u.uhungermax/2) ) ? SATIATED :
		(h > 150) ? NOT_HUNGRY :
		(h > 50) ? HUNGRY :
		(h > 0) ? WEAK : FAINTING;

	/* While you're eating, you may pass from WEAK to HUNGRY to NOT_HUNGRY.
	 * This should not produce the message "you only feel hungry now";
	 * that message should only appear if HUNGRY is an endpoint.  Therefore
	 * we check to see if we're in the middle of eating.  If so, we save
	 * the first hunger status, and at the end of eating we decide what
	 * message to print based on the _entire_ meal, not on each little bit.
	 */
	/* It is normally possible to check if you are in the middle of a meal
	 * by checking occupation == eatfood, but there is one special case:
	 * start_eating() can call bite() for your first bite before it
	 * sets the occupation.
	 * Anyone who wants to get that case to work _without_ an ugly static
	 * force_save_hs variable, feel free.
	 */
	/* Note: If you become a certain hunger status in the middle of the
	 * meal, and still have that same status at the end of the meal,
	 * this will incorrectly print the associated message at the end of
	 * the meal instead of the middle.  Such a case is currently
	 * impossible, but could become possible if a message for SATIATED
	 * were added or if HUNGRY and WEAK were separated by a big enough
	 * gap to fit two bites.
	 */
	if (occupation == eatfood || force_save_hs) {
		if (!saved_hs) {
			save_hs = u.uhs;
			saved_hs = TRUE;
		}
		u.uhs = newhs;
		return;
	} else {
		if (saved_hs) {
			u.uhs = save_hs;
			saved_hs = FALSE;
		}
	}

	if(newhs == FAINTING) {
		if(is_fainted()) newhs = FAINTED;
		if(u.uhs <= WEAK || rn2(20-YouHunger/10) >= 19) {
			if(!is_fainted() && multi >= 0 /* %% */) {
				/* stop what you're doing, then faint */
				stop_occupation();
	            if (clockwork){
	              Your("clockwork slips.");
	              flags.soundok=0;
				  nomul(-2, "immobilized by slipping gears.");
	              nomovemsg = "Your clockwork catches again.";
	            } else {
					You("faint from lack of food.");
					flags.soundok = 0;
					if(Role_if(PM_CONVICT)) nomul(-1+YouHunger/20, "fainted from lack of food");
					else nomul(-10+YouHunger/10, "fainted from lack of food");
					nomovemsg = "You regain consciousness.";
					afternmv = unfaint;
            	}
				newhs = FAINTED;
			}
		} else
		if(YouHunger < -(int)(200 + 20*ACURR(A_CON))) {
            if (clockwork){
              Your("clockwork comes to a complete stop.");
			  killer_format = KILLED_BY_AN;
			  killer = "unwound spring";
			  done(DIED);
            } else {
				u.uhs = STARVED;
				flags.botl = 1;
				bot();
				You("die from starvation.");
				killer_format = KILLED_BY;
				killer = "starvation";
				done(STARVING);
			}
			/* if we return, we lifesaved, and that calls newuhs */
			return;
		}
	}

	if(newhs != u.uhs) {
		if(newhs >= WEAK && u.uhs < WEAK)
			losestr(1);	/* this may kill you -- see below */
		else if(newhs < WEAK && u.uhs >= WEAK)
			losestr(-1);
		switch(newhs){
		case HUNGRY:
	        if (clockwork){
	          if (Hallucination)
	            Your((!incr)? "cuckoo only feels hungry now.":
	                "cuckoo is feeling hungry.");
	          else
	          You_feel((!incr) ? "your mainspring tightening." :
	             "the power of your mainspring waning.");
	          if (incr && occupation && occupation != windclock)
	            stop_occupation();
		break;
	        }
			if (Hallucination) {
			    You((!incr) ?
				"now have a lesser case of the munchies." :
				"are getting the munchies.");
			} else
			    You((!incr) ? "only feel hungry now." :
				  (YouHunger < 145) ? "feel hungry." :
				   "are beginning to feel hungry.");
			if (incr && occupation &&
			    (occupation != eatfood && occupation != opentin))
			    stop_occupation();
		break;
		case WEAK:
	        if (clockwork){
	          You_feel("your mainspring %s and your gears %s.",
	              (Hallucination)?"sprunging":"unwinding",
	              (!incr)?"still slipping":
	              (YouHunger < 45 ) ? "slipping":
	              "starting to slip");  
	          if (incr && occupation && occupation != windclock)
	            stop_occupation();
			  if(u.ucspeed == HIGH_CLOCKSPEED){
				pline("There is no longer sufficient tension in your mainspring to maintain a high clock-speed");
				u.ucspeed = NORM_CLOCKSPEED;
			  }
		break;
	        }
			if (Hallucination)
			    pline((!incr) ?
				  "You still have the munchies." :
      "The munchies are interfering with your motor capabilities.");
			else if (incr &&
				(Role_if(PM_WIZARD) || Race_if(PM_ELF) ||
				 Role_if(PM_VALKYRIE)))
			    pline("%s needs food, badly!",
				  (Role_if(PM_WIZARD) || Role_if(PM_VALKYRIE)) ?
				  urole.name.m : "Elf");
			else
			    You((!incr) ? "feel weak now." :
				  (YouHunger < 45) ? "feel weak." :
				   "are beginning to feel weak.");
			if (incr && occupation &&
			    (occupation != eatfood && occupation != opentin))
			    stop_occupation();
		break;
		case SATIATED:
	        if (clockwork){
			  if(u.ucspeed == SLOW_CLOCKSPEED){
				pline("There is now too much tension in your mainspring to maintain a slow clock-speed");
				u.ucspeed = NORM_CLOCKSPEED;
			  }
			}
		break;
		case FAINTING:
	        if (clockwork){
			  if(u.ucspeed == HIGH_CLOCKSPEED){
				pline("There is no longer sufficient tension in your mainspring to maintain a high clock-speed");
				u.ucspeed = NORM_CLOCKSPEED;
			  }
			}
		break;
		}
		u.uhs = newhs;
		flags.botl = 1;
		bot();
		if ((Upolyd ? u.mh : u.uhp) < 1) {
			You("die from hunger and exhaustion.");
			killer_format = KILLED_BY;
			killer = "exhaustion";
			done(STARVING);
			return;
		}
	}
}

#endif /* OVL0 */
#ifdef OVLB

/* Returns an object representing food.  Object may be either on floor or
 * in inventory.
 */
struct obj *
floorfood(verb,corpsecheck)	/* get food from floor or pack */
	const char *verb;
	int corpsecheck; /* 0, no check, 1, corpses, 2, tinnable corpses */
{
	register struct obj *otmp;
	char qbuf[QBUFSZ];
	char c;
	boolean feeding = (!strcmp(verb, "eat"));

	/* if we can't touch floor objects then use invent food only */
	if (!can_reach_floor() ||
#ifdef STEED
		(feeding && u.usteed) || /* can't eat off floor while riding */
#endif
		((is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy)) &&
		    (Wwalking || is_clinger(youmonst.data) ||
			(Flying && !Breathless))))
	    goto skipfloor;

	if (feeding && (metallivorous(youmonst.data) || (uclockwork && u.clockworkUpgrades&SCRAP_MAW))) {
	    struct obj *gold;
	    struct trap *ttmp = t_at(u.ux, u.uy);

	    if (ttmp && ttmp->tseen && ttmp->ttyp == BEAR_TRAP) {
		/* If not already stuck in the trap, perhaps there should
		   be a chance to becoming trapped?  Probably not, because
		   then the trap would just get eaten on the _next_ turn... */
		Sprintf(qbuf, "There is a bear trap here (%s); eat it?",
			(u.utrap && u.utraptype == TT_BEARTRAP) ?
				"holding you" : "armed");
		if ((c = yn_function(qbuf, ynqchars, 'n')) == 'y') {
		    u.utrap = u.utraptype = 0;
		    deltrap(ttmp);
		    return mksobj(BEARTRAP, TRUE, FALSE);
		} else if (c == 'q') {
		    return (struct obj *)0;
		}
	    }

	    if (youmonst.data != &mons[PM_RUST_MONSTER] &&
			!(uclockwork && u.clockworkUpgrades&SCRAP_MAW) &&
		(gold = g_at(u.ux, u.uy)) != 0) {
		if (gold->quan == 1L)
		    Sprintf(qbuf, "There is 1 gold piece here; eat it?");
		else
		    Sprintf(qbuf, "There are %ld gold pieces here; eat them?",
			    gold->quan);
		if ((c = yn_function(qbuf, ynqchars, 'n')) == 'y') {
		    return gold;
		} else if (c == 'q') {
		    return (struct obj *)0;
		}
	    }
	}

	/* Is there some food (probably a heavy corpse) here on the ground? */
	for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere) {
		if(corpsecheck ?
		(otmp->otyp==CORPSE && (corpsecheck == 1 || tinnable(otmp))) :
		    feeding ? (otmp->oclass != COIN_CLASS && is_edible(otmp)) :
						otmp->oclass==FOOD_CLASS) {
			Sprintf(qbuf, "There %s %s here; %s %s?",
				otense(otmp, "are"),
				doname(otmp), verb,
				(otmp->quan == 1L) ? "it" : "one");
			if((c = yn_function(qbuf,ynqchars,'n')) == 'y')
				return(otmp);
			else if(c == 'q')
				return((struct obj *) 0);
		}
	}

 skipfloor:
	/* We cannot use ALL_CLASSES since that causes getobj() to skip its
	 * "ugly checks" and we need to check for inedible items.
	 */
	otmp = getobj(feeding ? (const char *)allobj :
				(const char *)comestibles, verb);
	if (corpsecheck && otmp)
	    if (otmp->otyp != CORPSE || (corpsecheck == 2 && !tinnable(otmp))) {
		You_cant("%s that!", verb);
		return (struct obj *)0;
	    }
	return otmp;
}

/* Side effects of vomiting */
/* added nomul (MRS) - it makes sense, you're too busy being sick! */
void
vomit()		/* A good idea from David Neves */
{
	make_sick(0L, (char *) 0, TRUE, SICK_VOMITABLE);
	nomul(-2, "vomiting");
}

int
eaten_stat(base, obj)
register int base;
register struct obj *obj;
{
	long uneaten_amt, full_amount;

	uneaten_amt = (long)obj->oeaten;
	full_amount = (obj->otyp == CORPSE) ? (long)mons[obj->corpsenm].cnutrit
					: (long)objects[obj->otyp].oc_nutrition;
	if (uneaten_amt > full_amount) {
	    impossible(
	  "partly eaten food (%ld) more nutritious than untouched food (%ld)",
		       uneaten_amt, full_amount);
	    uneaten_amt = full_amount;
	}

	base = (int)(full_amount ? (long)base * uneaten_amt / full_amount : 0L);
	return (base < 1) ? 1 : base;
}

/* reduce obj's oeaten field, making sure it never hits or passes 0 */
void
consume_oeaten(obj, amt)
struct obj *obj;
int amt;
{
    /*
     * This is a hack to try to squelch several long standing mystery
     * food bugs.  A better solution would be to rewrite the entire
     * victual handling mechanism from scratch using a less complex
     * model.  Alternatively, this routine could call done_eating()
     * or food_disappears() but its callers would need revisions to
     * cope with victual.piece unexpectedly going away.
     *
     * Multi-turn eating operates by setting the food's oeaten field
     * to its full nutritional value and then running a counter which
     * independently keeps track of whether there is any food left.
     * The oeaten field can reach exactly zero on the last turn, and
     * the object isn't removed from inventory until the next turn
     * when the "you finish eating" message gets delivered, so the
     * food would be restored to the status of untouched during that
     * interval.  This resulted in unexpected encumbrance messages
     * at the end of a meal (if near enough to a threshold) and would
     * yield full food if there was an interruption on the critical
     * turn.  Also, there have been reports over the years of food
     * becoming massively heavy or producing unlimited satiation;
     * this would occur if reducing oeaten via subtraction attempted
     * to drop it below 0 since its unsigned type would produce a
     * huge positive value instead.  So far, no one has figured out
     * _why_ that inappropriate subtraction might sometimes happen.
     */

    if (amt > 0) {
	/* bit shift to divide the remaining amount of food */
	obj->oeaten >>= amt;
    } else {
	/* simple decrement; value is negative so we actually add it */
	if ((int) obj->oeaten > -amt)
	    obj->oeaten += amt;
	else
	    obj->oeaten = 0;
    }

    if (obj->oeaten == 0) {
	if (obj == victual.piece)	/* always true unless wishing... */
	    victual.reqtime = victual.usedtime;	/* no bites left */
	obj->oeaten = 1;	/* smallest possible positive value */
    }
}

int
clockwork_eat_menu(dry,mgc)
	boolean dry;
	boolean mgc;
{
	winid tmpwin;
	int n, how;
	char buf[BUFSZ];
	char incntlet = 'a';
	menu_item *selected;
	anything any;
	
	if(!(dry||mgc||u.clockworkUpgrades&(SCRAP_MAW|HELLFIRE_FURNACE))) return 0;
	
	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */
	
	Sprintf(buf, "Installed Upgrades");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	if(u.clockworkUpgrades & WOOD_STOVE && dry){
		Sprintf(buf, "wood stove");
		any.a_int = (int) WOOD_STOVE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.clockworkUpgrades & HELLFIRE_FURNACE){
		Sprintf(buf, "hellfire furnace");
		any.a_int = (int) HELLFIRE_FURNACE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.clockworkUpgrades & MAGIC_FURNACE && mgc){
		Sprintf(buf, "magic furnace");
		any.a_int = (int) MAGIC_FURNACE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.clockworkUpgrades & SCRAP_MAW){
		Sprintf(buf, "scrap maw");
		any.a_int = (int) SCRAP_MAW;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	end_menu(tmpwin, "Choose upgrade:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return ( n > 0 ) ? selected[0].item.a_int : 0;
}

#endif /* OVLB */
#ifdef OVL1

/* called when eatfood occupation has been interrupted,
   or in the case of theft, is about to be interrupted */
boolean
maybe_finished_meal(stopping)
boolean stopping;
{
	/* in case consume_oeaten() has decided that the food is all gone */
	if (occupation == eatfood && victual.usedtime >= victual.reqtime) {
	    if (stopping) occupation = 0;	/* for do_reset_eat */
	    (void) eatfood(); /* calls done_eating() to use up victual.piece */
	    return TRUE;
	}
	return FALSE;
}

#endif /* OVL1 */

/*eat.c*/
