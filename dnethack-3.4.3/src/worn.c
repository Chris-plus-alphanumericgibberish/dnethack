/*	SCCS Id: @(#)worn.c	3.4	2003/01/08	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_DCL void FDECL(m_lose_armor, (struct monst *,struct obj *));
STATIC_DCL void FDECL(m_dowear_type, (struct monst *,long, BOOLEAN_P, BOOLEAN_P));
STATIC_DCL int NDECL(def_beastmastery);

const static int ORANGE_RES[] = {SLEEP_RES};
const static int CHROMATIC_RES[] = {FIRE_RES, COLD_RES, DISINT_RES, SHOCK_RES, POISON_RES};
const static int EREBOR_RES[] = {FIRE_RES, COLD_RES};
const static int DURIN_RES[] = {FIRE_RES, ACID_RES, POISON_RES};
const static int REV_PROPS[] = {COLD_RES, REGENERATION, FIXED_ABIL, POISON_RES, SEE_INVIS};

const struct worn {
	long w_mask;
	struct obj **w_obj;
} worn[] = {
	{ W_ARM, &uarm },
	{ W_ARMC, &uarmc },
	{ W_ARMH, &uarmh },
	{ W_ARMS, &uarms },
	{ W_ARMG, &uarmg },
	{ W_ARMF, &uarmf },
#ifdef TOURIST
	{ W_ARMU, &uarmu },
#endif
	{ W_RINGL, &uleft },
	{ W_RINGR, &uright },
	{ W_WEP, &uwep },
	{ W_SWAPWEP, &uswapwep },
	{ W_QUIVER, &uquiver },
	{ W_AMUL, &uamul },
	{ W_TOOL, &ublindf },
	{ W_BALL, &uball },
	{ W_CHAIN, &uchain },
	{ 0, 0 }
};

/* This only allows for one blocking item per property */
#define w_blocks(o,m) \
		((o->otyp == MUMMY_WRAPPING && ((m) & W_ARMC)) ? INVIS : \
		 (o->otyp == CORNUTHAUM && ((m) & W_ARMH) && \
			!Role_if(PM_WIZARD)) ? CLAIRVOYANT : 0)
		/* note: monsters don't have clairvoyance, so your role
		   has no significant effect on their use of w_blocks() */


/* Updated to use the extrinsic and blocked fields. */
void
setworn(obj, mask)
register struct obj *obj;
long mask;
{
	register const struct worn *wp;
	register struct obj *oobj;
	register int p;
	
	/*Handle the pen of the void here*/
	if(obj && obj->oartifact == ART_PEN_OF_THE_VOID){
		if(obj->ovar1 && !Role_if(PM_EXILE)){
			long oldseals = u.sealsKnown;
			u.sealsKnown |= obj->ovar1;
			if(oldseals != u.sealsKnown) You("learned new seals.");
		}
		obj->ovar1 = u.spiritTineA|u.spiritTineB;
		if(u.voidChime){
			int i;
			for(i=0; i<u.sealCounts; i++){
				obj->ovar1 |= u.spirit[i];
			}
		}
	}
	
	if ((mask & (W_ARM|I_SPECIAL)) == (W_ARM|I_SPECIAL)) {
	    /* restoring saved game; no properties are conferred via skin */
	    uskin = obj;
	 /* assert( !uarm ); */
	} else {
	    for(wp = worn; wp->w_mask; wp++) if(wp->w_mask & mask) {
		oobj = *(wp->w_obj);
		if(oobj && !(oobj->owornmask & wp->w_mask))
			impossible("Setworn: mask = %ld.", wp->w_mask);
		if(oobj) {
		    if (u.twoweap && (oobj->owornmask & (W_WEP|W_SWAPWEP)))
			u.twoweap = 0;
		    oobj->owornmask &= ~wp->w_mask;
		    if (wp->w_mask & ~(W_SWAPWEP|W_QUIVER)) {
			/* leave as "x = x <op> y", here and below, for broken
			 * compilers */
			p = objects[oobj->otyp].oc_oprop;
			if(oobj->otyp == ORANGE_DRAGON_SCALES || oobj->otyp == ORANGE_DRAGON_SCALE_MAIL || oobj->otyp == ORANGE_DRAGON_SCALE_SHIELD){
				for(p = 0; p < 1; p++) u.uprops[ORANGE_RES[p]].extrinsic = u.uprops[ORANGE_RES[p]].extrinsic & ~wp->w_mask;
			}
			
			if(oobj->oartifact == ART_CHROMATIC_DRAGON_SCALES){
				for(p = 0; p < 5; p++) u.uprops[CHROMATIC_RES[p]].extrinsic = u.uprops[CHROMATIC_RES[p]].extrinsic & ~wp->w_mask;
			} else if(oobj->oartifact == ART_WAR_MASK_OF_DURIN){
				for(p = 0; p < 3; p++) u.uprops[DURIN_RES[p]].extrinsic = u.uprops[DURIN_RES[p]].extrinsic & ~wp->w_mask;
			} else if(oobj->oartifact == ART_ARMOR_OF_EREBOR){
				for(p = 0; p < 2; p++) u.uprops[EREBOR_RES[p]].extrinsic = u.uprops[EREBOR_RES[p]].extrinsic & ~wp->w_mask;
			} else if(oobj->oartifact == ART_CLAWS_OF_THE_REVENANCER){
				for(p = 0; p < 5; p++) u.uprops[REV_PROPS[p]].extrinsic = u.uprops[REV_PROPS[p]].extrinsic & ~wp->w_mask;
			} else u.uprops[p].extrinsic = u.uprops[p].extrinsic & ~wp->w_mask;
			if ((p = w_blocks(oobj,mask)) != 0)
			    u.uprops[p].blocked &= ~wp->w_mask;
			if (oobj->oartifact)
			    set_artifact_intrinsic(oobj, 0, mask);
		    }
		}
		*(wp->w_obj) = obj;
		if(obj) {
		    obj->owornmask |= wp->w_mask;
		    /* Prevent getting/blocking intrinsics from wielding
		     * potions, through the quiver, etc.
		     * Allow weapon-tools, too.
		     * wp_mask should be same as mask at this point.
		     */
		    if (wp->w_mask & ~(W_SWAPWEP|W_QUIVER)) {
			if (obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
					    mask != W_WEP) {
			    p = objects[obj->otyp].oc_oprop;
				if(obj->oartifact == ART_CHROMATIC_DRAGON_SCALES){
					for(p = 0; p < 5; p++) u.uprops[CHROMATIC_RES[p]].extrinsic = u.uprops[CHROMATIC_RES[p]].extrinsic | wp->w_mask;
				} else if(obj->oartifact == ART_WAR_MASK_OF_DURIN){
					for(p = 0; p < 3; p++) u.uprops[DURIN_RES[p]].extrinsic = u.uprops[DURIN_RES[p]].extrinsic | wp->w_mask;
				} else if(obj->oartifact == ART_ARMOR_OF_EREBOR){
					for(p = 0; p < 2; p++) u.uprops[EREBOR_RES[p]].extrinsic = u.uprops[EREBOR_RES[p]].extrinsic | wp->w_mask;
				} else if(obj->oartifact == ART_CLAWS_OF_THE_REVENANCER){
					for(p = 0; p < 5; p++) u.uprops[REV_PROPS[p]].extrinsic = u.uprops[REV_PROPS[p]].extrinsic | wp->w_mask;
				} else u.uprops[p].extrinsic = u.uprops[p].extrinsic | wp->w_mask;
			    if ((p = w_blocks(obj, mask)) != 0)
				u.uprops[p].blocked |= wp->w_mask;
			}
			if (obj->oartifact)
			    set_artifact_intrinsic(obj, 1, mask);
		    }
		}
	    }
	}
	if(!restoring) see_monsters(); //More objects than just artifacts grant warning now, and this is a convienient place to add a failsafe see_monsters check
	update_inventory();
}

/* called e.g. when obj is destroyed */
/* Updated to use the extrinsic and blocked fields. */
void
setnotworn(obj)
register struct obj *obj;
{
	register const struct worn *wp;
	register int p;

	if (!obj) return;
	if (obj == uwep || obj == uswapwep) u.twoweap = 0;
	for(wp = worn; wp->w_mask; wp++)
	    if(obj == *(wp->w_obj)) {
		*(wp->w_obj) = 0;
		p = objects[obj->otyp].oc_oprop;
		if(obj->otyp == ORANGE_DRAGON_SCALES || obj->otyp == ORANGE_DRAGON_SCALE_MAIL || obj->otyp == ORANGE_DRAGON_SCALE_SHIELD){
			for(p = 0; p < 1; p++) u.uprops[ORANGE_RES[p]].extrinsic = u.uprops[ORANGE_RES[p]].extrinsic & ~wp->w_mask;
		}
		
		if(obj->oartifact == ART_CHROMATIC_DRAGON_SCALES){
			for(p = 0; p < 5; p++) u.uprops[CHROMATIC_RES[p]].extrinsic = u.uprops[CHROMATIC_RES[p]].extrinsic & ~wp->w_mask;
		} else if(obj->oartifact == ART_WAR_MASK_OF_DURIN){
			for(p = 0; p < 3; p++) u.uprops[DURIN_RES[p]].extrinsic = u.uprops[DURIN_RES[p]].extrinsic & ~wp->w_mask;
		} else if(obj->oartifact == ART_ARMOR_OF_EREBOR){
			for(p = 0; p < 2; p++) u.uprops[EREBOR_RES[p]].extrinsic = u.uprops[EREBOR_RES[p]].extrinsic & ~wp->w_mask;
		} else if(obj->oartifact == ART_CLAWS_OF_THE_REVENANCER){
			for(p = 0; p < 5; p++) u.uprops[REV_PROPS[p]].extrinsic = u.uprops[REV_PROPS[p]].extrinsic & ~wp->w_mask;
		} else u.uprops[p].extrinsic = u.uprops[p].extrinsic & ~wp->w_mask;
		obj->owornmask &= ~wp->w_mask;
		if (obj->oartifact)
		    set_artifact_intrinsic(obj, 0, wp->w_mask);
		if ((p = w_blocks(obj,wp->w_mask)) != 0)
		    u.uprops[p].blocked &= ~wp->w_mask;
	    }
	update_inventory();
}

void
mon_set_minvis(mon)
struct monst *mon;
{
	mon->perminvis = 1;
	if (!mon->invis_blkd) {
	    mon->minvis = 1;
	    if (opaque(mon->data))
		unblock_point(mon->mx, mon->my);
	    newsym(mon->mx, mon->my);		/* make it disappear */
	    if (mon->wormno) see_wsegs(mon);	/* and any tail too */
	}
}

void
mon_adjust_speed(mon, adjust, obj)
struct monst *mon;
int adjust;	/* positive => increase speed, negative => decrease */
struct obj *obj;	/* item to make known if effect can be seen */
{
    struct obj *otmp;
    boolean give_msg = !in_mklev, petrify = FALSE;
    unsigned int oldspeed = mon->mspeed;

    switch (adjust) {
     case  2:
	mon->permspeed = MFAST;
	give_msg = FALSE;	/* special case monster creation */
	break;
     case  1:
	if (mon->permspeed == MSLOW) mon->permspeed = 0;
	else mon->permspeed = MFAST;
	break;
     case  0:			/* just check for worn speed boots */
	break;
     case -1:
	if (mon->permspeed == MFAST) mon->permspeed = 0;
	else mon->permspeed = MSLOW;
	break;
     case -2:
	mon->permspeed = MSLOW;
	give_msg = FALSE;	/* (not currently used) */
	break;
     case -3:			/* petrification */
	/* take away intrinsic speed but don't reduce normal speed */
	if (mon->permspeed == MFAST) mon->permspeed = 0;
	petrify = TRUE;
	break;
    }

    for (otmp = mon->minvent; otmp; otmp = otmp->nobj)
	if (otmp->owornmask && objects[otmp->otyp].oc_oprop == FAST)
	    break;
    if (otmp)		/* speed boots */
	mon->mspeed = MFAST;
    else
	mon->mspeed = mon->permspeed;

    if (give_msg && (mon->mspeed != oldspeed || petrify) && canseemon(mon)) {
	/* fast to slow (skipping intermediate state) or vice versa */
	const char *howmuch = (mon->mspeed + oldspeed == MFAST + MSLOW) ?
				"much " : "";

	if (petrify) {
	    /* mimic the player's petrification countdown; "slowing down"
	       even if fast movement rate retained via worn speed boots */
	    if (flags.verbose) pline("%s is slowing down.", Monnam(mon));
	} else if (adjust > 0 || mon->mspeed == MFAST)
	    if (is_weeping(mon->data)) {
		pline("%s is suddenly changing positions %sfaster.", Monnam(mon), howmuch);
	    } else {
		pline("%s is suddenly moving %sfaster.", Monnam(mon), howmuch);
	    }
	else
	    if (is_weeping(mon->data)) {
		pline("%s is suddenly changing positions %sslower.", Monnam(mon), howmuch);
	    } else {
		pline("%s seems to be moving %sslower.", Monnam(mon), howmuch);
	    }

	/* might discover an object if we see the speed change happen, but
	   avoid making possibly forgotten book known when casting its spell */
	if (obj != 0 && obj->dknown &&
		objects[obj->otyp].oc_class != SPBOOK_CLASS)
	    makeknown(obj->otyp);
    }
}

update_mon_intrinsic(mon, obj, which, on, silently)
struct monst *mon;
struct obj *obj;
int which;
boolean on, silently;
{
    uchar mask;
    struct obj *otmp;
    if (on) {
	switch (which) {
	 case INVIS:
	    mon->minvis = !mon->invis_blkd;
	    break;
	 case FAST:
	  {
	    boolean save_in_mklev = in_mklev;
	    if (silently) in_mklev = TRUE;
	    mon_adjust_speed(mon, 0, obj);
	    in_mklev = save_in_mklev;
	    break;
	  }
	/* properties handled elsewhere */
	 case ANTIMAGIC:
	 case REFLECTING:
	    break;
	/* properties which have no effect for monsters */
	 case CLAIRVOYANT:
	 case STEALTH:
	 case TELEPAT:
	    break;
	/* properties which should have an effect but aren't implemented */
	 case LEVITATION:
	 case WWALKING:
	    break;
	/* properties which maybe should have an effect but don't */
	 case DISPLACED:
	 case FUMBLING:
	 case JUMPING:
	 case PROTECTION:
	    break;
	 default:
	    if (which <= 10) {	/* 1 thru 10 correspond to MR_xxx mask values */
		/* FIRE,COLD,SLEEP,DISINT,SHOCK,POISON,ACID,STONE */
		mask = (uchar) (1 << (which - 1));
		mon->mintrinsics |= (unsigned short) mask;
	    }
	    break;
	}
    } else {	    /* off */
	switch (which) {
	 case INVIS:
	    mon->minvis = mon->perminvis;
	    break;
	 case FAST:
	  {
	    boolean save_in_mklev = in_mklev;
	    if (silently) in_mklev = TRUE;
	    mon_adjust_speed(mon, 0, obj);
	    in_mklev = save_in_mklev;
	    break;
	  }
	 case FIRE_RES:
	 case COLD_RES:
	 case SLEEP_RES:
	 case DISINT_RES:
	 case SHOCK_RES:
	 case POISON_RES:
	 case ACID_RES:
	 case STONE_RES:
	 case DRAIN_RES:
	 case SICK_RES:
	    mask = (uchar) (1 << (which - 1));
	    /* If the monster doesn't have this resistance intrinsically,
	       check whether any other worn item confers it.  Note that
	       we don't currently check for anything conferred via simply
	       carrying an object. */
	    if (!(mon->data->mresists & mask)) {
		for (otmp = mon->minvent; otmp; otmp = otmp->nobj)
		    if (otmp->owornmask &&
			    (int) objects[otmp->otyp].oc_oprop == which)
			break;
		if (!otmp)
		    mon->mintrinsics &= ~((unsigned short) mask);
	    }
	    break;
	 default:
	    break;
	}
    }
}

/* armor put on or taken off; might be magical variety */
void
update_mon_intrinsics(mon, obj, on, silently)
struct monst *mon;
struct obj *obj;
boolean on, silently;
{
    int unseen;
    int which = (int) objects[obj->otyp].oc_oprop;
	
    unseen = !canseemon(mon);
    if (!which) goto maybe_blocks;
	
	if(obj->otyp == ORANGE_DRAGON_SCALES || obj->otyp == ORANGE_DRAGON_SCALE_MAIL || obj->otyp == ORANGE_DRAGON_SCALE_SHIELD){
		for(which = 0; which < 1; which++) update_mon_intrinsic(mon, obj, ORANGE_RES[which], on, silently);
	}
	
	if(obj->oartifact == ART_CHROMATIC_DRAGON_SCALES){
		for(which = 0; which < 5; which++) update_mon_intrinsic(mon, obj, CHROMATIC_RES[which], on, silently);
	} else if(obj->oartifact == ART_WAR_MASK_OF_DURIN){
		for(which = 0; which < 3; which++) update_mon_intrinsic(mon, obj, DURIN_RES[which], on, silently);
	} else if(obj->oartifact == ART_ARMOR_OF_EREBOR){
		for(which = 0; which < 2; which++) update_mon_intrinsic(mon, obj, EREBOR_RES[which], on, silently);
	} else if(obj->oartifact == ART_CLAWS_OF_THE_REVENANCER){
		for(which = 0; which < 5; which++) update_mon_intrinsic(mon, obj, REV_PROPS[which], on, silently);
	} else update_mon_intrinsic(mon, obj, which, on, silently);

 maybe_blocks:
    /* obj->owornmask has been cleared by this point, so we can't use it.
       However, since monsters don't wield armor, we don't have to guard
       against that and can get away with a blanket worn-mask value. */
    switch (w_blocks(obj,~0L)) {
     case INVIS:
	mon->invis_blkd = on ? 1 : 0;
	mon->minvis = on ? 0 : mon->perminvis;
	break;
     default:
	break;
    }

#ifdef STEED
	if (!on && mon == u.usteed && obj->otyp == SADDLE)
	    dismount_steed(DISMOUNT_FELL);
#endif

    /* if couldn't see it but now can, or vice versa, update display */
    if (!silently && (unseen ^ !canseemon(mon)))
	newsym(mon->mx, mon->my);
}

int 
base_mac(mon)
struct monst *mon;
{
	int base = mon->data->ac, armac = 0;
	
	if(mon->data == &mons[PM_ASMODEUS] && base < -9) base = -9 + AC_VALUE(base+9);
	else if(mon->data == &mons[PM_PALE_NIGHT] && base < -6) base = -6 + AC_VALUE(base+6);
	else if(mon->data == &mons[PM_CHOKHMAH_SEPHIRAH]){
		base -= u.chokhmah;
	}
	else if(is_weeping(mon->data)){
		if(mon->mextra[1] & 0x4L) base = -125; //Fully Quantum Locked
		if(mon->mextra[1] & 0x2L) base = -20; //Partial Quantum Lock
	}
	else if(mon->data == &mons[PM_MARILITH] || mon->data == &mons[PM_SHAKTARI]){
	    struct obj *mwep = (mon == &youmonst) ? uwep : MON_WEP(mon);
		if(mwep){
			base += base*10;
		}
	}
	
	if(mon->mtame) base -= rnd(def_beastmastery());
	
	return base;
}

int
find_mac(mon)
struct monst *mon;
{
	struct obj *obj;
	int base = mon->data->ac, armac = 0;
	long mwflags = mon->misc_worn_check;
	
	if(mon->data == &mons[PM_ASMODEUS] && base < -9) base = -9 + AC_VALUE(base+9);
	else if(mon->data == &mons[PM_PALE_NIGHT] && base < -6) base = -6 + AC_VALUE(base+6);
	else if(mon->data == &mons[PM_CHOKHMAH_SEPHIRAH]){
		base -= u.chokhmah;
	}
	else if(is_weeping(mon->data)){
		if(mon->mextra[1] & 0x4L) base = -125; //Fully Quantum Locked
		if(mon->mextra[1] & 0x2L) base = -20; //Partial Quantum Lock
	}
	else if(mon->data == &mons[PM_GIANT_TURTLE] && mon->mflee){
		base -= 15;
	}
	else if(mon->data == &mons[PM_MARILITH] || mon->data == &mons[PM_SHAKTARI]){
	    struct obj *mwep = (mon == &youmonst) ? uwep : MON_WEP(mon);
		if(mwep){
			base += base*10;
		}
	}
	
	if(mon->mtame) base -= rnd(def_beastmastery());
	
	if(mon->data == &mons[PM_HOD_SEPHIRAH]){
		if(uarm) armac += ARM_BONUS(uarm);
		if(uarmf) armac += ARM_BONUS(uarmf);
		if(uarmg) armac += ARM_BONUS(uarmg);
		if(uarmu) armac += ARM_BONUS(uarmu);
		if(uarms) armac += ARM_BONUS(uarms);
		if(uarmh) armac += ARM_BONUS(uarmh);
		if(uarmc) armac += ARM_BONUS(uarmc);
		
		if(armac < 0) armac *= -1;
	}
	else for (obj = mon->minvent; obj; obj = obj->nobj) {
	    if (obj->owornmask & mwflags)
		armac += ARM_BONUS(obj);
	}
	if(armac > 11) armac = rnd(armac-10) + 10; /* high armor ac values act like player ac values */

	base -= armac;
	/* since ARM_BONUS is positive, subtracting it increases AC */
	return base;
}

int
full_mac(mon)
struct monst *mon;
{
	struct obj *obj;
	int base = mon->data->ac, armac = 0;
	long mwflags = mon->misc_worn_check;
	
	if(mon->data == &mons[PM_CHOKHMAH_SEPHIRAH]){
		base -= u.chokhmah;
	}
	else if(is_weeping(mon->data)){
		if(mon->mextra[1] & 0x4L) base = -125; //Fully Quantum Locked
		if(mon->mextra[1] & 0x2L) base = -20; //Partial Quantum Lock
	}
	else if(mon->data == &mons[PM_GIANT_TURTLE] && mon->mflee){
		base -= 15;
	}
	else if(mon->data == &mons[PM_MARILITH] || mon->data == &mons[PM_SHAKTARI]){
	    struct obj *mwep = (mon == &youmonst) ? uwep : MON_WEP(mon);
		if(mwep){
			base += base*10;
		}
	}
	
	if(mon->mtame) base -= def_beastmastery();
	
	if(mon->data == &mons[PM_HOD_SEPHIRAH]){
		if(uarm) armac += ARM_BONUS(uarm);
		if(uarmf) armac += ARM_BONUS(uarmf);
		if(uarmg) armac += ARM_BONUS(uarmg);
		if(uarmu) armac += ARM_BONUS(uarmu);
		if(uarms) armac += ARM_BONUS(uarms);
		if(uarmh) armac += ARM_BONUS(uarmh);
		if(uarmc) armac += ARM_BONUS(uarmc);
		
		if(armac < 0) armac *= -1;
	}
	else for (obj = mon->minvent; obj; obj = obj->nobj) {
	    if (obj->owornmask & mwflags)
		armac += ARM_BONUS(obj);
	}

	base -= armac;
	/* since ARM_BONUS is positive, subtracting it increases AC */
	return base;
}

/* weapons are handled separately; rings and eyewear aren't used by monsters */

/* Wear the best object of each type that the monster has.  During creation,
 * the monster can put everything on at once; otherwise, wearing takes time.
 * This doesn't affect monster searching for objects--a monster may very well
 * search for objects it would not want to wear, because we don't want to
 * check which_armor() each round.
 *
 * We'll let monsters put on shirts and/or suits under worn cloaks, but
 * not shirts under worn suits.  This is somewhat arbitrary, but it's
 * too tedious to have them remove and later replace outer garments,
 * and preventing suits under cloaks makes it a little bit too easy for
 * players to influence what gets worn.  Putting on a shirt underneath
 * already worn body armor is too obviously buggy...
 */
void
m_dowear(mon, creation)
register struct monst *mon;
boolean creation;
{
#define RACE_EXCEPTION TRUE
	/* Note the restrictions here are the same as in dowear in do_wear.c
	 * except for the additional restriction on intelligence.  (Players
	 * are always intelligent, even if polymorphed).
	 */
	if (verysmall(mon->data) || nohands(mon->data) || is_animal(mon->data))
		return;
	/* give mummies a chance to wear their wrappings
	 * and let skeletons wear their initial armor */
	if (mindless(mon->data) && (!creation ||
	    (mon->data->mlet != S_MUMMY && mon->data != &mons[PM_SKELETON])))
		return;

	m_dowear_type(mon, W_AMUL, creation, FALSE);
#ifdef TOURIST
	/* can't put on shirt if already wearing suit */
	if (!cantweararm(mon->data) || (mon->misc_worn_check & W_ARM))
	    m_dowear_type(mon, W_ARMU, creation, FALSE);
#endif
	/* treating small as a special case allows
	   hobbits, gnomes, and kobolds to wear cloaks */
	if (!cantweararm(mon->data) || mon->data->msize == MZ_SMALL)
	    m_dowear_type(mon, W_ARMC, creation, FALSE);
	m_dowear_type(mon, W_ARMH, creation, FALSE);
	if (!MON_WEP(mon) || !bimanual(MON_WEP(mon)))
	    m_dowear_type(mon, W_ARMS, creation, FALSE);
	m_dowear_type(mon, W_ARMG, creation, FALSE);
	if (!slithy(mon->data) && mon->data->mlet != S_CENTAUR)
	    m_dowear_type(mon, W_ARMF, creation, FALSE);
	if (!cantweararm(mon->data))
	    m_dowear_type(mon, W_ARM, creation, FALSE);
	else
	    m_dowear_type(mon, W_ARM, creation, RACE_EXCEPTION);
}

STATIC_OVL void
m_dowear_type(mon, flag, creation, racialexception)
struct monst *mon;
long flag;
boolean creation;
boolean racialexception;
{
	struct obj *old, *best, *obj;
	int m_delay = 0;
	int unseen = !canseemon(mon);
	char nambuf[BUFSZ];

	if (mon->mfrozen) return; /* probably putting previous item on */

	/* Get a copy of monster's name before altering its visibility */
	Strcpy(nambuf, See_invisible ? Monnam(mon) : mon_nam(mon));

	old = which_armor(mon, flag);
	if (old && old->cursed) return;
	if (old && flag == W_AMUL) return; /* no such thing as better amulets */
	best = old;

	for(obj = mon->minvent; obj; obj = obj->nobj) {
	    switch(flag) {
		case W_AMUL:
		    if (obj->oclass != AMULET_CLASS ||
			    (obj->otyp != AMULET_OF_LIFE_SAVING &&
				obj->otyp != AMULET_OF_REFLECTION))
			continue;
		    best = obj;
		    goto outer_break; /* no such thing as better amulets */
#ifdef TOURIST
		case W_ARMU:
		    if (!is_shirt(obj)) continue;
		    break;
#endif
		case W_ARMC:
		    if (!is_cloak(obj)) continue;
		    break;
		case W_ARMH:
		    if (!is_helmet(obj)) continue;
		    /* (flimsy exception matches polyself handling) */
		    if (has_horns(mon->data) && !is_flimsy(obj)) continue;
		    break;
		case W_ARMS:
		    if (!is_shield(obj)) continue;
		    break;
		case W_ARMG:
		    if (!is_gloves(obj)) continue;
		    break;
		case W_ARMF:
		    if (!is_boots(obj)) continue;
		    break;
		case W_ARM:
		    if (!is_suit(obj)) continue;
		    if (racialexception && (racial_exception(mon, obj) < 1)) continue;
		    break;
	    }
	    if (obj->owornmask) continue;
	    /* I'd like to define a VISIBLE_ARM_BONUS which doesn't assume the
	     * monster knows obj->spe, but if I did that, a monster would keep
	     * switching forever between two -2 caps since when it took off one
	     * it would forget spe and once again think the object is better
	     * than what it already has.
	     */
	    if (best && (ARM_BONUS(best) + extra_pref(mon,best) >= ARM_BONUS(obj) + extra_pref(mon,obj)))
		continue;
	    best = obj;
	}
outer_break:
	if (!best || best == old) return;

	/* if wearing a cloak, account for the time spent removing
	   and re-wearing it when putting on a suit or shirt */
	if ((flag == W_ARM
#ifdef TOURIST
	  || flag == W_ARMU
#endif
			  ) && (mon->misc_worn_check & W_ARMC))
	    m_delay += 2;
	/* when upgrading a piece of armor, account for time spent
	   taking off current one */
	if (old)
	    m_delay += objects[old->otyp].oc_delay;

	if (old) /* do this first to avoid "(being worn)" */
	    old->owornmask = 0L;
	if (!creation) {
	    if (canseemon(mon)) {
		char buf[BUFSZ];

		if (old)
		    Sprintf(buf, " removes %s and", distant_name(old, doname));
		else
		    buf[0] = '\0';
		pline("%s%s puts on %s.", Monnam(mon),
		      buf, distant_name(best,doname));
	    } /* can see it */
	    m_delay += objects[best->otyp].oc_delay;
	    mon->mfrozen = m_delay;
	    if (mon->mfrozen) mon->mcanmove = 0;
	}
	if (old)
	    update_mon_intrinsics(mon, old, FALSE, creation);
	mon->misc_worn_check |= flag;
	best->owornmask |= flag;
	update_mon_intrinsics(mon, best, TRUE, creation);
	/* if couldn't see it but now can, or vice versa, */
	if (!creation && (unseen ^ !canseemon(mon))) {
		if (mon->minvis && !See_invisible) {
			pline("Suddenly you cannot see %s.", nambuf);
			makeknown(best->otyp);
		} /* else if (!mon->minvis) pline("%s suddenly appears!", Amonnam(mon)); */
	}
}
#undef RACE_EXCEPTION

struct obj *
which_armor(mon, flag)
struct monst *mon;
long flag;
{
	register struct obj *obj;

	for(obj = mon->minvent; obj; obj = obj->nobj)
		if (obj->owornmask & flag) return obj;
	return((struct obj *)0);
}

/* remove an item of armor and then drop it */
STATIC_OVL void
m_lose_armor(mon, obj)
struct monst *mon;
struct obj *obj;
{
	mon->misc_worn_check &= ~obj->owornmask;
	if (obj->owornmask)
	    update_mon_intrinsics(mon, obj, FALSE, FALSE);
	obj->owornmask = 0L;

	obj_extract_self(obj);
	place_object(obj, mon->mx, mon->my);
	/* call stackobj() if we ever drop anything that can merge */
	newsym(mon->mx, mon->my);
}

/* all objects with their bypass bit set should now be reset to normal */
void
clear_bypasses()
{
	struct obj *otmp, *nobj;
	struct monst *mtmp;

	for (otmp = fobj; otmp; otmp = nobj) {
	    nobj = otmp->nobj;
	    if (otmp->bypass) {
		otmp->bypass = 0;
		/* bypass will have inhibited any stacking, but since it's
		   used for polymorph handling, the objects here probably
		   have been transformed and won't be stacked in the usual
		   manner afterwards; so don't bother with this */
#if 0
		if (objects[otmp->otyp].oc_merge) {
		    xchar ox, oy;

		    (void) get_obj_location(otmp, &ox, &oy, 0);
		    stack_object(otmp);
		    newsym(ox, oy);
		}
#endif	/*0*/
	    }
	}
	/* invent and mydogs chains shouldn't matter here */
	for (otmp = migrating_objs; otmp; otmp = otmp->nobj)
	    otmp->bypass = 0;
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
		otmp->bypass = 0;
	}
	for (mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon) {
	    for (otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
		otmp->bypass = 0;
	}
	flags.bypasses = FALSE;
}

void
bypass_obj(obj)
struct obj *obj;
{
	obj->bypass = 1;
	flags.bypasses = TRUE;
}

void
mon_break_armor(mon, polyspot)
struct monst *mon;
boolean polyspot;
{
	register struct obj *otmp;
	struct permonst *mdat = mon->data;
	boolean vis = cansee(mon->mx, mon->my);
	boolean handless_or_tiny = (nohands(mdat) || verysmall(mdat));
	const char *pronoun = mhim(mon),
			*ppronoun = mhis(mon);

	if (breakarm(mdat)) {
	    if ((otmp = which_armor(mon, W_ARM)) != 0) {
		if ((Is_dragon_scales(otmp) &&
			mdat == Dragon_scales_to_pm(otmp)) ||
		    (Is_dragon_mail(otmp) && mdat == Dragon_mail_to_pm(otmp)))
		    ;	/* no message here;
			   "the dragon merges with his scaly armor" is odd
			   and the monster's previous form is already gone */
		else if (vis)
		    pline("%s breaks out of %s armor!", Monnam(mon), ppronoun);
		else
		    You_hear("a cracking sound.");
		m_useup(mon, otmp);
	    }
	    if ((otmp = which_armor(mon, W_ARMC)) != 0) {
		if (otmp->oartifact) {
		    if (vis)
			pline("%s %s falls off!", s_suffix(Monnam(mon)),
				cloak_simple_name(otmp));
		    if (polyspot) bypass_obj(otmp);
		    m_lose_armor(mon, otmp);
		} else {
		    if (vis)
			pline("%s %s tears apart!", s_suffix(Monnam(mon)),
				cloak_simple_name(otmp));
		    else
			You_hear("a ripping sound.");
		    m_useup(mon, otmp);
		}
	    }
#ifdef TOURIST
	    if ((otmp = which_armor(mon, W_ARMU)) != 0) {
		if (vis)
		    pline("%s shirt rips to shreds!", s_suffix(Monnam(mon)));
		else
		    You_hear("a ripping sound.");
		m_useup(mon, otmp);
	    }
#endif
	} else if (sliparm(mdat)) {
	    if ((otmp = which_armor(mon, W_ARM)) != 0) {
		if (vis)
		    pline("%s armor falls around %s!",
				 s_suffix(Monnam(mon)), pronoun);
		else
		    You_hear("a thud.");
		if (polyspot) bypass_obj(otmp);
		m_lose_armor(mon, otmp);
	    }
	    if ((otmp = which_armor(mon, W_ARMC)) != 0) {
		if (vis) {
		    if (is_whirly(mon->data))
			pline("%s %s falls, unsupported!",
				     s_suffix(Monnam(mon)), cloak_simple_name(otmp));
		    else
			pline("%s shrinks out of %s %s!", Monnam(mon),
						ppronoun, cloak_simple_name(otmp));
		}
		if (polyspot) bypass_obj(otmp);
		m_lose_armor(mon, otmp);
	    }
#ifdef TOURIST
	    if ((otmp = which_armor(mon, W_ARMU)) != 0) {
		if (vis) {
		    if (sliparm(mon->data))
			pline("%s seeps right through %s shirt!",
					Monnam(mon), ppronoun);
		    else
			pline("%s becomes much too small for %s shirt!",
					Monnam(mon), ppronoun);
		}
		if (polyspot) bypass_obj(otmp);
		m_lose_armor(mon, otmp);
	    }
#endif
	}
	if (handless_or_tiny) {
	    /* [caller needs to handle weapon checks] */
	    if ((otmp = which_armor(mon, W_ARMG)) != 0) {
		if (vis)
		    pline("%s drops %s gloves%s!", Monnam(mon), ppronoun,
					MON_WEP(mon) ? " and weapon" : "");
		if (polyspot) bypass_obj(otmp);
		m_lose_armor(mon, otmp);
	    }
	    if ((otmp = which_armor(mon, W_ARMS)) != 0) {
		if (vis)
		    pline("%s can no longer hold %s shield!", Monnam(mon),
								ppronoun);
		else
		    You_hear("a clank.");
		if (polyspot) bypass_obj(otmp);
		m_lose_armor(mon, otmp);
	    }
	}
	if (handless_or_tiny || has_horns(mdat)) {
	    if ((otmp = which_armor(mon, W_ARMH)) != 0 &&
		    /* flimsy test for horns matches polyself handling */
		    (handless_or_tiny || !is_flimsy(otmp))) {
		if (vis)
		    pline("%s helmet falls to the %s!",
			  s_suffix(Monnam(mon)), surface(mon->mx, mon->my));
		else
		    You_hear("a clank.");
		if (polyspot) bypass_obj(otmp);
		m_lose_armor(mon, otmp);
	    }
	}
	if (handless_or_tiny || slithy(mdat) || mdat->mlet == S_CENTAUR) {
	    if ((otmp = which_armor(mon, W_ARMF)) != 0) {
		if (vis) {
		    if (is_whirly(mon->data))
			pline("%s boots fall away!",
				       s_suffix(Monnam(mon)));
		    else pline("%s boots %s off %s feet!",
			s_suffix(Monnam(mon)),
			verysmall(mdat) ? "slide" : "are pushed", ppronoun);
		}
		if (polyspot) bypass_obj(otmp);
		m_lose_armor(mon, otmp);
	    }
	}
#ifdef STEED
	if (!can_saddle(mon)) {
	    if ((otmp = which_armor(mon, W_SADDLE)) != 0) {
		if (polyspot) bypass_obj(otmp);
		m_lose_armor(mon, otmp);
		if (vis)
		    pline("%s saddle falls off.", s_suffix(Monnam(mon)));
	    }
	    if (mon == u.usteed)
		goto noride;
	} else if (mon == u.usteed && !can_ride(mon)) {
	noride:
	    You("can no longer ride %s.", mon_nam(mon));
	    if (touch_petrifies(u.usteed->data) &&
			!Stone_resistance && rnl(3)) {
		char buf[BUFSZ];

		You("touch %s.", mon_nam(u.usteed));
		Sprintf(buf, "falling off %s",
				an(u.usteed->data->mname));
		instapetrify(buf);
	    }
	    dismount_steed(DISMOUNT_FELL);
	}
#endif
	return;
}

/* bias a monster's preferences towards armor that has special benefits. */
/* currently only does speed boots, but might be expanded if monsters get to
   use more armor abilities */
int
extra_pref(mon, obj)
struct monst *mon;
struct obj *obj;
{
    if (obj) {
	if (obj->otyp == SPEED_BOOTS && mon->permspeed != MFAST)
	    return 20;
    }
    return 0;
}

/*
 * Exceptions to things based on race. Correctly checks polymorphed player race.
 * Returns:
 *	 0 No exception, normal rules apply.
 * 	 1 If the race/object combination is acceptable.
 *	-1 If the race/object combination is unacceptable.
 */
int
racial_exception(mon, obj)
struct monst *mon;
struct obj *obj;
{
    const struct permonst *ptr = raceptr(mon);

    /* Acceptable Exceptions: */
    /* Allow hobbits to wear elven armor - LoTR */
    if (ptr == &mons[PM_HOBBIT] && is_elven_armor(obj))
	return 1;
    /* Unacceptable Exceptions: */
    /* Checks for object that certain races should never use go here */
    /*	return -1; */

    return 0;
}

void
light_damage(arg, timeout)
genericptr_t arg;
long timeout;
{
	struct obj *obj = (struct obj *) arg;;
 	xchar x = 0, y = 0;
	boolean on_floor = obj->where == OBJ_FLOOR,
		in_invent = obj->where == OBJ_INVENT;
	
	if(obj->shopOwned){
		start_timer(1, TIMER_OBJECT,
					LIGHT_DAMAGE, (genericptr_t)obj);
		return;
	}

//	pline("checking light damage");
	if (on_floor) {
	    x = obj->ox;
	    y = obj->oy;
		if(levl[x][y].lit == 0 && !(viz_array[y][x]&TEMP_LIT)){
			if(obj->oeroded && obj->oerodeproof) obj->oeroded--;
			start_timer(1, TIMER_OBJECT,
						LIGHT_DAMAGE, (genericptr_t)obj);
			return;
		}
		if(obj->oeroded < 2){
			obj->oeroded++;
			start_timer(1, TIMER_OBJECT,
						LIGHT_DAMAGE, (genericptr_t)obj);
			return;
		}else{
			if(obj->otyp == NOBLE_S_DRESS){
				obj = poly_obj(obj, BLACK_DRESS);
				obj->oeroded = 0;
			} else {
				obj_extract_self(obj);
				obfree(obj, (struct obj *)0);
			}
		}
	} else if (in_invent) {
//		pline("object in invent");
		int armpro = 0;
		boolean isarmor = obj == uarm || obj == uarmc || obj == uarms || obj == uarmh || 
					obj == uarmg || obj == uarmf || obj == uarms;
		if(uarmc){
			armpro = uarmc->otyp == DROVEN_CLOAK ? 
				objects[uarmc->otyp].a_can - uarmc->ovar1 :
				objects[uarmc->otyp].a_can;
		}
		if((levl[u.ux][u.uy].lit == 0 && !(viz_array[u.uy][u.ux]&TEMP_LIT)) || ((rn2(3) < armpro) && rn2(50))){
			if(obj->oeroded && obj->oerodeproof && 
				levl[u.ux][u.uy].lit == 0 && !(viz_array[u.uy][u.ux]&TEMP_LIT)) obj->oeroded--;
			start_timer(1, TIMER_OBJECT,
						LIGHT_DAMAGE, (genericptr_t)obj);
			return;
		}
		if(obj->oeroded < 2){
			obj->oeroded++;
			Your("%s degrades.",xname(obj));
			start_timer(1, TIMER_OBJECT,
						LIGHT_DAMAGE, (genericptr_t)obj);
			stop_occupation();
			if(flags.run) nomul(0, NULL);
			return;
		}
	    if (flags.verbose && !isarmor) {
			char *name = obj->otyp == CORPSE ? corpse_xname(obj, FALSE) : xname(obj);
			Your("%s%s%s %s away%c",
				 obj == uwep ? "wielded " : nul, name, obj->otyp == NOBLE_S_DRESS ? "'s armored plates" : "",
				 obj->otyp == NOBLE_S_DRESS ? "evaporate" : otense(obj, "evaporate"), obj == uwep ? '!' : '.');
	    }
	    if (obj == uwep) {
			uwepgone();	/* now bare handed */
			stop_occupation();
			if(flags.run) nomul(0, NULL);
			if(obj->otyp == NOBLE_S_DRESS){
				obj = poly_obj(obj, BLACK_DRESS);
				obj->oeroded = 0;
			} else {
				useupall(obj);
			}
	    } else if (obj == uswapwep) {
			uswapwepgone();
			stop_occupation();
			if(flags.run) nomul(0, NULL);
			if(obj->otyp == NOBLE_S_DRESS){
				obj = poly_obj(obj, BLACK_DRESS);
				obj->oeroded = 0;
			} else {
				useupall(obj);
				obj->oeroded = 0;
			}
	    } else if (obj == uquiver) {
			uqwepgone();
			stop_occupation();
			if(flags.run) nomul(0, NULL);
			if(obj->otyp == NOBLE_S_DRESS){
				obj = poly_obj(obj, BLACK_DRESS);
				obj->oeroded = 0;
			} else {
				useupall(obj);
			}
	    } else if (isarmor) {
			stop_occupation();
			if(flags.run) nomul(0, NULL);
			if(obj->otyp == NOBLE_S_DRESS){
				pline("The armored plates on your dress turn to dust and blow away.");
				remove_worn_item(obj, TRUE);
				obj = poly_obj(obj, BLACK_DRESS);
				obj->oeroded = 0;
				if(!uarmu){
					setworn(obj, W_ARMU);
					Shirt_on();
				} else {
					setworn(obj, W_ARM);
					Armor_on();
				}
			} else destroy_arm(obj);
	    } else{
			stop_occupation();
			if(flags.run) nomul(0, NULL);
			if(obj->otyp == NOBLE_S_DRESS){
				obj = poly_obj(obj, BLACK_DRESS);
				obj->oeroded = 0;
			} else {
				useupall(obj);
			}
		}
	} else if (obj->where == OBJ_MINVENT && obj->owornmask) {
		struct obj *armor = which_armor(obj->ocarry, W_ARMC);
		int armpro = 0;
		long unwornmask;
		struct monst *mtmp;
		if(armor){
			armpro = armor->otyp == DROVEN_CLOAK ? 
				objects[armor->otyp].a_can - armor->ovar1 :
				objects[armor->otyp].a_can;
		}
		if((levl[obj->ocarry->mx][obj->ocarry->my].lit == 0 && !(viz_array[obj->ocarry->my][obj->ocarry->mx]&TEMP_LIT))
			 || ((rn2(3) < armpro) && rn2(50))){
			if(obj->oeroded && obj->oerodeproof 
				&& levl[obj->ocarry->mx][obj->ocarry->my].lit == 0 && !(viz_array[obj->ocarry->my][obj->ocarry->mx]&TEMP_LIT)) 
					obj->oeroded--;
			start_timer(1, TIMER_OBJECT,
						LIGHT_DAMAGE, (genericptr_t)obj);
			return;
		}
		if(obj->oeroded < 2){
			obj->oeroded++;
			start_timer(1, TIMER_OBJECT,
						LIGHT_DAMAGE, (genericptr_t)obj);
			return;
		}
	    if (obj == MON_WEP(obj->ocarry)) {
			setmnotwielded(obj->ocarry,obj);
			MON_NOWEP(obj->ocarry);
			if(obj->otyp == NOBLE_S_DRESS){
				obj = poly_obj(obj, BLACK_DRESS);
				obj->oeroded = 0;
				place_object(obj, mtmp->mx, mtmp->my);
				/* call stackobj() if we ever drop anything that can merge */
				newsym(mtmp->mx, mtmp->my);
			} else {
				m_useup(obj->ocarry, obj);
			}
		}
		else if((unwornmask = obj->owornmask) != 0L){
			mtmp = obj->ocarry;
			obj_extract_self(obj);
			mtmp->misc_worn_check &= ~unwornmask;
			obj->owornmask = 0L;
			update_mon_intrinsics(mtmp, obj, FALSE, FALSE);
			if(obj->otyp == NOBLE_S_DRESS){
				obj = poly_obj(obj, BLACK_DRESS);
				obj->oeroded = 0;
				place_object(obj, mtmp->mx, mtmp->my);
				/* call stackobj() if we ever drop anything that can merge */
				newsym(mtmp->mx, mtmp->my);
			} else {
				m_useup(obj->ocarry, obj);
			}
		}
		else{
			obj_extract_self(obj);
			if(obj->otyp == NOBLE_S_DRESS){
				obj = poly_obj(obj, BLACK_DRESS);
				obj->oeroded = 0;
				place_object(obj, mtmp->mx, mtmp->my);
				/* call stackobj() if we ever drop anything that can merge */
				newsym(mtmp->mx, mtmp->my);
			} else {
				m_useup(obj->ocarry, obj);
			}
		}
	}
	if (on_floor) newsym(x, y);
	else if (in_invent) update_inventory();
}

STATIC_OVL int
def_beastmastery()
{
	switch (P_SKILL(P_BEAST_MASTERY)) {
		case P_ISRESTRICTED: return 0; break;
		case P_UNSKILLED:    return 0; break;
		case P_BASIC:        return 2; break;
		case P_SKILLED:      return 5; break;
		case P_EXPERT:       return 10; break;
	}
}

/*worn.c*/
