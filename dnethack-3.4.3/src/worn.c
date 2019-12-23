/*	SCCS Id: @(#)worn.c	3.4	2003/01/08	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
void FDECL(mon_block_extrinsic, (struct monst *, struct obj *, int, boolean, boolean));
boolean FDECL(mon_gets_extrinsic, (struct monst *, int, struct obj *));
STATIC_DCL void FDECL(update_mon_intrinsic, (struct monst *,struct obj *,int,BOOLEAN_P,BOOLEAN_P));
STATIC_DCL void FDECL(m_lose_armor, (struct monst *,struct obj *));
STATIC_DCL void FDECL(m_dowear_type, (struct monst *,long, BOOLEAN_P, BOOLEAN_P));
STATIC_DCL int NDECL(def_beastmastery);
STATIC_DCL int NDECL(def_mountedCombat);

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
			!(Role_if(PM_WIZARD) || Race_if(PM_INCANTIFIER))) ? CLAIRVOYANT : 0)
		/* note: monsters don't have clairvoyance, so your role
		   has no significant effect on their use of w_blocks() */

/* 
 * Returns a pointer to a TEMPORARY array of all the properties (from prop.h) an item has
 * This pointer should be used immediately and discarded, since calling this function again will modify the contents of the array
 *
 * If called without an obj, uses otyp to give as much info as possible without knowing obj
 */
int *
item_property_list(obj, otyp)
struct obj* obj;
int otyp;
{
	static int property_list[LAST_PROP];	// the temporary list of properties
	int cur_prop, i;
	boolean got_prop;

	if (obj)
		otyp = obj->otyp;

	// bonus properties some items have that cannot fit into objclass->oc_oprop
	const static int NO_RES[] = { 0 };
	const static int ALCHEMY_RES[] = { ACID_RES, 0 };
	const static int ORANGE_RES[] = { SLEEP_RES, HALLUC_RES, 0 };
	const static int YELLOW_RES[] = { STONE_RES, 0 };
	const static int GREEN_RES[] = { SICK_RES, 0 };
	const static int BLUE_RES[] = { FAST, 0 };
	const static int BLACK_RES[] = { DRAIN_RES, 0 };
	const static int WHITE_RES[] = { MAGICAL_BREATHING, SWIMMING, 0 };
	const static int GRAY_RES[] = { HALF_SPDAM, 0 };
	const static int SHIM_RES[] = { SEE_INVIS, 0 };

	i = 0;
	for (cur_prop = 1; cur_prop < LAST_PROP; cur_prop++)
	{
		got_prop = FALSE;
		// from objclass
		if (objects[otyp].oc_oprop == cur_prop)
			got_prop = TRUE;
		// from object properties
		if (!got_prop && obj && obj->oproperties)
		{
			switch (cur_prop)
			{
			case FIRE_RES:
				if (obj->oproperties & OPROP_FIRE)
					got_prop = TRUE;
				break;
			case COLD_RES:
				if (obj->oproperties & OPROP_COLD)
					got_prop = TRUE;
				break;
			case SHOCK_RES:
				if (obj->oproperties & OPROP_ELEC)
					got_prop = TRUE;
				break;
			case ACID_RES:
				if (obj->oproperties & OPROP_ACID)
					got_prop = TRUE;
				break;
			case ANTIMAGIC:
				if (obj->oproperties & OPROP_MAGC)
					got_prop = TRUE;
				break;
			case REFLECTING:
				if (obj->oproperties & OPROP_REFL)
					got_prop = TRUE;
				break;
			case DISINT_RES:
				if (obj->oproperties & OPROP_DISN)
					got_prop = TRUE;
				break;
			}
		}
		// from object type that doesn't fit into objclass
		if (!got_prop)
		{
			// first, select the item's list of bonus properties
			const int * bonus_prop_list;
			switch (otyp)
			{
			case ALCHEMY_SMOCK:
				bonus_prop_list = (ALCHEMY_RES);
				break;
			case ORANGE_DRAGON_SCALES:
			case ORANGE_DRAGON_SCALE_MAIL:
			case ORANGE_DRAGON_SCALE_SHIELD:
				bonus_prop_list = (ORANGE_RES);
				break;
			case YELLOW_DRAGON_SCALES:
			case YELLOW_DRAGON_SCALE_MAIL:
			case YELLOW_DRAGON_SCALE_SHIELD:
				bonus_prop_list = (YELLOW_RES);
				break;
			case GREEN_DRAGON_SCALES:
			case GREEN_DRAGON_SCALE_MAIL:
			case GREEN_DRAGON_SCALE_SHIELD:
				bonus_prop_list = (GREEN_RES);
				break;
			case BLUE_DRAGON_SCALES:
			case BLUE_DRAGON_SCALE_MAIL:
			case BLUE_DRAGON_SCALE_SHIELD:
				bonus_prop_list = (BLUE_RES);
				break;
			case BLACK_DRAGON_SCALES:
			case BLACK_DRAGON_SCALE_MAIL:
			case BLACK_DRAGON_SCALE_SHIELD:
				bonus_prop_list = (BLACK_RES);
				break;
			case WHITE_DRAGON_SCALES:
			case WHITE_DRAGON_SCALE_MAIL:
			case WHITE_DRAGON_SCALE_SHIELD:
				bonus_prop_list = (WHITE_RES);
				break;
			case GRAY_DRAGON_SCALES:
			case GRAY_DRAGON_SCALE_MAIL:
			case GRAY_DRAGON_SCALE_SHIELD:
				bonus_prop_list = (GRAY_RES);
				break;
			case SHIMMERING_DRAGON_SCALES:
			case SHIMMERING_DRAGON_SCALE_MAIL:
			case SHIMMERING_DRAGON_SCALE_SHIELD:
				bonus_prop_list = (SHIM_RES);
				break;
			default:
				bonus_prop_list = (NO_RES);
				break;
			}
			// if it has one, then see if the current property is on the list
			if (bonus_prop_list != (NO_RES))
			{
				int j;
				for (j = 0; bonus_prop_list[j]; j++)
				if (bonus_prop_list[j] == cur_prop)
					got_prop = TRUE;
			}
		}
		// if we've got the property, add it to the array
		if (got_prop)
		{
			property_list[i] = cur_prop;
			i++;
		}
	}
	// add a terminator to the array
	property_list[i] = 0;

	// return the list
	return property_list;	
}

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
	} else if(obj && obj->oartifact == ART_HELM_OF_THE_ARCANE_ARCHER){
      if(P_UNSKILLED == OLD_P_SKILL(P_ATTACK_SPELL)) OLD_P_SKILL(P_ATTACK_SPELL) = P_BASIC;
      if(P_BASIC     == OLD_P_SKILL(P_ATTACK_SPELL)) OLD_P_SKILL(P_ATTACK_SPELL) = P_SKILLED;
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
		    if (u.twoweap && (oobj->owornmask & (W_WEP|W_SWAPWEP)) && !test_twoweapon())
				u.twoweap = 0;
		    oobj->owornmask &= ~wp->w_mask;
		    if (wp->w_mask & ~(W_SWAPWEP|W_QUIVER)) {
			/* leave as "x = x <op> y", here and below, for broken
			 * compilers */			
			int * property_list = item_property_list(oobj, oobj->otyp);
			p = 0;
			while (property_list[p] != 0)	{
				u.uprops[property_list[p]].extrinsic = u.uprops[property_list[p]].extrinsic & ~wp->w_mask;
				p++;
			}
			
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
				int * property_list = item_property_list(obj, obj->otyp);
				p = 0;
				while (property_list[p] != 0)	{
					u.uprops[property_list[p]].extrinsic = u.uprops[property_list[p]].extrinsic | wp->w_mask;
					p++;
				}
			    if ((p = w_blocks(obj, mask)) != 0)
				u.uprops[p].blocked |= wp->w_mask;
			}
			if (obj->oartifact)
			    set_artifact_intrinsic(obj, 1, mask);
		    }
		}
	    }
	}
	if(!restoring) {
		see_monsters(); //More objects than just artifacts grant warning now, and this is a convienient place to add a failsafe see_monsters check
		update_inventory();
	}
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
	if (obj->oartifact && obj->oartifact == ART_HELM_OF_THE_ARCANE_ARCHER){
      if(P_BASIC   == OLD_P_SKILL(P_ATTACK_SPELL)) OLD_P_SKILL(P_ATTACK_SPELL) = P_UNSKILLED;
      if(P_SKILLED == OLD_P_SKILL(P_ATTACK_SPELL)) OLD_P_SKILL(P_ATTACK_SPELL) = P_BASIC;
    }
	for(wp = worn; wp->w_mask; wp++)
	    if(obj == *(wp->w_obj)) {
		*(wp->w_obj) = 0;
		int * property_list = item_property_list(obj, obj->otyp);
		p = 0;
		while (property_list[p] != 0)	{
			u.uprops[property_list[p]].extrinsic = u.uprops[property_list[p]].extrinsic & ~wp->w_mask;
			p++;
		}
		
		if(obj->oartifact == ART_GAUNTLETS_OF_THE_BERSERKER){
//        adj_abon(uarmg, -uarmg->ovar1);
          uarmg->ovar1 = 0;
        }
		obj->owornmask &= ~wp->w_mask;
		if (obj->oartifact)
		    set_artifact_intrinsic(obj, 0, wp->w_mask);
		if ((p = w_blocks(obj,wp->w_mask)) != 0)
		    u.uprops[p].blocked &= ~wp->w_mask;
	    }
	update_inventory();
	if ((obj == uwep || obj == uswapwep) && !test_twoweapon()) u.twoweap = 0;
}

void
mon_set_minvis(mon)
struct monst *mon;
{
	mon->perminvis = 1;
	if (!mon->invis_blkd) {
	    mon->minvis = 1;
	    if (opaque(mon->data) && !See_invisible(mon->mx, mon->my))
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

    if (mon_extrinsic(mon, FAST))		/* speed boots or other extrinsic source */
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

/* update a blocked extrinsic
 * assumes single source of each blocked extrinsic
 */
void
mon_block_extrinsic(mon, obj, which, on, silently)
struct monst *mon;
struct obj *obj;
int which;
boolean on, silently;
{
	if (on) {
		switch (which)
		{
		case INVIS:
			if (mon->data != &mons[PM_HELLCAT]){
				mon->invis_blkd = TRUE;
				update_mon_intrinsic(mon, obj, which, !on, silently);
			}
			break;
		default:
			update_mon_intrinsic(mon, obj, which, !on, silently);
			break;
		}
	}
	else { /* off */
		switch (which)
		{
		case INVIS:
			if (mon->data != &mons[PM_HELLCAT]){
				mon->invis_blkd = FALSE;
				if (mon_gets_extrinsic(mon, which, obj))
					update_mon_intrinsic(mon, obj, which, !on, silently);
				if (mon->perminvis)
					mon->minvis = TRUE;
			}
			break;
		default:
			if (mon_gets_extrinsic(mon, which, obj))
				update_mon_intrinsic(mon, obj, which, !on, silently);
			break;
		}
	}
}

/* find out if a monster gets a certain extrinsic from its equipment 
 * if given an ignored_obj, does not consider it to give any extrinsics
 */
boolean
mon_gets_extrinsic(mon, which, ignored_obj)
struct monst *mon;
int which;
struct obj *ignored_obj;
{
	struct obj *otmp;			/* item in mon's inventory */
	boolean got_prop = FALSE;	/* property to find */
	int * tmp_property_list;	/* list of item/artifact properties */
	int i;						/* loop counter */

	for (otmp = mon->minvent; (otmp && !got_prop); otmp = otmp->nobj){
		/* ignore one object in particular */
		if (otmp == ignored_obj)
			continue;

		/* worn items */
		if (otmp->owornmask) {
			tmp_property_list = item_property_list(otmp, otmp->otyp);
			for (i = 0; tmp_property_list[i]; i++)
			{
				if (tmp_property_list[i] == which)
					got_prop = TRUE;
			}
		}
		/* worn artifacts */
		if (otmp->owornmask && otmp->oartifact){
			tmp_property_list = art_property_list(otmp->oartifact, FALSE);
			for (i = 0; tmp_property_list[i]; i++)
			{
				if (tmp_property_list[i] == which)
					got_prop = TRUE;
			}
		}
		/* carried artifacts */
		if (otmp->oartifact){
			tmp_property_list = art_property_list(otmp->oartifact, TRUE);
			for (i = 0; tmp_property_list[i]; i++)
			{
				if (tmp_property_list[i] == which)
					got_prop = TRUE;
			}
		}
	}
	return got_prop;
}

STATIC_OVL void
update_mon_intrinsic(mon, obj, which, on, silently)
struct monst *mon;
struct obj *obj;
int which;
boolean on, silently;
{
    uchar mask;
    struct obj *otmp;
	boolean oldprop = mon_resistance(mon, which);;

    if (on) {
		/* some properties need special handling */
		switch (which)
		{
		case INVIS:
			if (mon->data != &mons[PM_HELLCAT]){
				mon->mextrinsics[(which-1)/32] |= (1 << (which-1)%32);
				mon->minvis = !mon->invis_blkd;
			}
			break;
		case FAST:
			{
			mon->mextrinsics[(which-1)/32] |= (1 << (which-1)%32);
			boolean save_in_mklev = in_mklev;
			if (silently) in_mklev = TRUE;
			mon_adjust_speed(mon, 0, obj);
			in_mklev = save_in_mklev;
			
			break;
			}
		case LEVITATION:
		case FLYING:
			mon->mextrinsics[(which-1)/32] |= (1 << (which-1)%32);
			if (!oldprop && (mon_resistance(mon,LEVITATION) || mon_resistance(mon,FLYING))) {
				m_float_up(mon, silently);
			}
			break;
		case DISPLACED:
			mon->mextrinsics[(which-1)/32] |= (1 << (which-1)%32);
			if (!oldprop && mon_resistance(mon,DISPLACED) && !silently && canseemon(mon)) {
				pline("%s outline begins shimmering!", s_suffix(Monnam(mon)));
			}
			break;
		case SWIMMING:
			mon->mextrinsics[(which-1)/32] |= (1 << (which-1)%32);
			if (!oldprop && mon_resistance(mon,SWIMMING)) {
				minliquid(mon);
			}
			break;
		default:
			mon->mextrinsics[(which-1)/32] |= (1 << (which-1)%32);
			break;
		}
    }
	else { /* off */
		/* we need to check that this property isn't being granted by any other equipment */
		if (!mon_gets_extrinsic(mon, which, obj)) {
			/* again, some properties need special handling */
			switch (which)
			{
			case INVIS:
				mon->mextrinsics[(which-1)/32] &= ~(1 << (which-1)%32);
				mon->minvis = (mon->invis_blkd ? FALSE : mon->perminvis);
				break;
			case FAST:
				{
				mon->mextrinsics[(which-1)/32] &= ~(1 << (which-1)%32);
				boolean save_in_mklev = in_mklev;
				if (silently) in_mklev = TRUE;
				mon_adjust_speed(mon, 0, obj);
				in_mklev = save_in_mklev;
				break;
				}
			case LEVITATION:
			case FLYING:
				mon->mextrinsics[(which-1)/32] &= ~(1 << (which-1)%32);
				if (oldprop && !mon_resistance(mon,LEVITATION) && !mon_resistance(mon,FLYING)) {
					m_float_down(mon, silently);
				}
				break;
			case DISPLACED:
				mon->mextrinsics[(which-1)/32] &= ~(1 << (which-1)%32);
				if (oldprop && !mon_resistance(mon,DISPLACED) && !silently && canseemon(mon)) {
					pline("%s outline stops shimmering.", s_suffix(Monnam(mon)));
				}
				break;
			case SWIMMING:
				mon->mextrinsics[(which-1)/32] &= ~(1 << (which-1)%32);
				if (oldprop && !mon_resistance(mon,SWIMMING)) {
					minliquid(mon);
				}
				break;
			default:
				mon->mextrinsics[(which-1)/32] &= ~(1 << (which-1)%32);
				break;
			}
		}
    }
	return;
}

/* armor put on, taken off, grabbed, or dropped; might be magical variety */
void
update_mon_intrinsics(mon, obj, on, silently)
struct monst *mon;
struct obj *obj;
boolean on, silently;
{
	/* don't bother with dead monsters -- at best nothing will happen, at worst we get bad messages */
	if (DEADMONSTER(mon))
		return;

	int unseen = !canseemon(mon);
    int which;
    long all_worn = ~0L; /* clang lint */
	
	int * property_list = item_property_list(obj, obj->otyp);
	/* only turn on properties from this list if obj is worn */
	if (!on || obj->owornmask) {
		which = 0;
		while (property_list[which] != 0)	{
			update_mon_intrinsic(mon, obj, property_list[which], on, silently);
			which++;
		}
	}
	if (obj->oartifact)
	{
		/* only turn on properties from this list if obj is worn */
		if (!on || obj->owornmask) {
			property_list = art_property_list(obj->oartifact, FALSE);
			which = 0;
			while (property_list[which] != 0)	{
				update_mon_intrinsic(mon, obj, property_list[which], on, silently);
				which++;
			}
		}
		/* while-carried properties */
		property_list = art_property_list(obj->oartifact, TRUE);
		which = 0;
		while (property_list[which] != 0)	{
			update_mon_intrinsic(mon, obj, property_list[which], on, silently);
			which++;
		}
	}
	/* if the object blocks an extrinsic, recalculate if the monster should get that extrinsic */
	/* use all_worn because the owornmask may have been cleared already and monsters will not wield armor */
	if (which = w_blocks(obj, all_worn))
		mon_block_extrinsic(mon, obj, which, on, silently);

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
	int base = 10, armac = 0;
	
	base -= mon->data->nac;
	if(!helpless(mon))
		base -= mon->data->dac;
	if(!mon->mcan)
		base -= mon->data->pac;
	
	if(mon->data == &mons[PM_ASMODEUS] && base < -9) base = -9 + MONSTER_AC_VALUE(base+9);
	else if(mon->data == &mons[PM_PALE_NIGHT] && base < -6) base = -6 + MONSTER_AC_VALUE(base+6);
	else if(mon->data == &mons[PM_BAALPHEGOR] && base < -8) base = -8 + MONSTER_AC_VALUE(base+8);
	else if(mon->data == &mons[PM_ZAPHKIEL] && base < -8) base = -8 + MONSTER_AC_VALUE(base+8);
	else if(mon->data == &mons[PM_QUEEN_OF_STARS] && base < -6) base = -6 + MONSTER_AC_VALUE(base+6);
	else if(mon->data == &mons[PM_ETERNAL_LIGHT] && base < -6) base = -6 + MONSTER_AC_VALUE(base+6);
	else if(mon->data == &mons[PM_STRANGE_CORPSE] && base < -5) base = -5 + MONSTER_AC_VALUE(base+5);
	else if(mon->data == &mons[PM_ANCIENT_OF_DEATH] && base < -4) base = -4 + MONSTER_AC_VALUE(base+4);
	else if(mon->data == &mons[PM_CHOKHMAH_SEPHIRAH]){
		base -= u.chokhmah;
	}
	else if(is_weeping(mon->data)){
		if(mon->mvar2 & 0x4L) base = -125; //Fully Quantum Locked
		if(mon->mvar2 & 0x2L) base = -20; //Partial Quantum Lock
	}
	if(mon->mfaction == ZOMBIFIED) base += 2;
	if(mon->mfaction == SKELIFIED) base -= 2;
	if(mon->mfaction == CRYSTALFIED) base -= 6;
	if(mon->mfaction == CRANIUM_RAT) base -= 4;
	
	if(is_alabaster_mummy(mon->data) && mon->mvar1 == SYLLABLE_OF_GRACE__UUR)
		base -= 10;
	
	if(mon->mtame){
		base -= rnd(def_beastmastery());
		if(u.usteed && mon==u.usteed) base -= rnd(def_mountedCombat());
	}
	
	return base;
}

int
find_mac(mon)
struct monst *mon;
{
	struct obj *obj;
	int base, armac = 0;
	long mwflags = mon->misc_worn_check;
	
	base = base_mac(mon);
	
	if(mon->data == &mons[PM_GIANT_TURTLE] && mon->mflee){
		base -= 15;
	}
	//Block attack with weapon
	if(mon != &youmonst &&
		(mon->data == &mons[PM_MARILITH] 
		|| mon->data == &mons[PM_SHAKTARI]
		|| mon->data == &mons[PM_CATHEZAR]
	)){
		int wcount = 0;
		struct obj *otmp;
		for(otmp = mon->minvent; otmp; otmp = otmp->nobj){
			if((otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
				|| (otmp->otyp == CHAIN && mon->data == &mons[PM_CATHEZAR])
				) && !otmp->oartifact
				&& otmp != MON_WEP(mon) && otmp != MON_SWEP(mon)
				&& !otmp->owornmask
				&& ++wcount >= 4
			) break;
		}
		if(MON_WEP(mon))
			wcount++;
		if(MON_SWEP(mon))
			wcount++;
		if(rn2(6) < wcount){
			base -= rnd(20);
		}
	}
	
	if(mon->mfaction == ZOMBIFIED) base -= 4;
	if(mon->mfaction == CRYSTALFIED) base -= 10;
	if(mon->mfaction == TOMB_HERD) base -= 6;
	
	if(mon->mtame){
		if(u.specialSealsActive&SEAL_COSMOS) base -= spiritDsize();
	}
	
	//armor AC
	if(mon->data == &mons[PM_HOD_SEPHIRAH]){
		if(uarm) armac += arm_ac_bonus(uarm);
		if(uarmf) armac += arm_ac_bonus(uarmf);
		if(uarmg) armac += arm_ac_bonus(uarmg);
		if(uarmu) armac += arm_ac_bonus(uarmu);
		if(uarms) armac += arm_ac_bonus(uarms);
		if(uarmh) armac += arm_ac_bonus(uarmh);
		if(uarmc) armac += arm_ac_bonus(uarmc);
		if(armac < 0) armac *= -1;
	}
	else for (obj = mon->minvent; obj; obj = obj->nobj) {
	    if (obj->owornmask & mwflags){
			armac += arm_ac_bonus(obj);
			if(is_shield(obj)) armac += max(0, obj->objsize - mon->data->msize);
		}
	}
	if(armac > 11) armac = rnd(armac-10) + 10; /* high armor ac values act like player ac values */

	base -= armac;
	/* since arm_ac_bonus is positive, subtracting it increases AC */
	return base;
}

int
full_mac(mon)
struct monst *mon;
{
	struct obj *obj;
	int base = 10, armac = 0;
	long mwflags = mon->misc_worn_check;
	base -= mon->data->nac;
	if(!helpless(mon))
		base -= mon->data->dac;
	if(!mon->mcan)
		base -= mon->data->pac;
	
	if(mon->data == &mons[PM_CHOKHMAH_SEPHIRAH]){
		base -= u.chokhmah;
	}
	else if(is_weeping(mon->data)){
		if(mon->mvar2 & 0x4L) base = -125; //Fully Quantum Locked
		if(mon->mvar2 & 0x2L) base = -20; //Partial Quantum Lock
	}
	else if(mon->data == &mons[PM_GIANT_TURTLE] && mon->mflee){
		base -= 15;
	}
	else if(mon != &youmonst &&
		(mon->data == &mons[PM_MARILITH] 
		|| mon->data == &mons[PM_SHAKTARI]
		|| mon->data == &mons[PM_CATHEZAR]
	)){
		int wcount = 0;
		struct obj *otmp;
		for(otmp = mon->minvent; otmp; otmp = otmp->nobj){
			if(otmp->oclass == WEAPON_CLASS || is_weptool(otmp)
				|| (otmp->otyp == CHAIN && !otmp->owornmask && mon->data == &mons[PM_CATHEZAR])
			){
				base -= 20;
				break;
			}
		}
	}
	
	if(mon->mfaction == ZOMBIFIED) base -= 2;
	if(mon->mfaction == SKELIFIED) base -= 6;
	if(mon->mfaction == CRYSTALFIED) base -= 16;
	
	if(is_alabaster_mummy(mon->data) && mon->mvar1 == SYLLABLE_OF_GRACE__UUR)
		base -= 10;
	
	if(mon->mtame){
		base -= def_beastmastery();
		if(u.specialSealsActive&SEAL_COSMOS) base -= spiritDsize();
		if(u.usteed && mon==u.usteed) base -= def_mountedCombat();
	}
	
	if(mon->data == &mons[PM_HOD_SEPHIRAH]){
		if(uarm) armac += arm_ac_bonus(uarm);
		if(uarmf) armac += arm_ac_bonus(uarmf);
		if(uarmg) armac += arm_ac_bonus(uarmg);
		if(uarmu) armac += arm_ac_bonus(uarmu);
		if(uarms) armac += arm_ac_bonus(uarms);
		if(uarmh) armac += arm_ac_bonus(uarmh);
		if(uarmc) armac += arm_ac_bonus(uarmc);
		
		if(armac < 0) armac *= -1;
	}
	else for (obj = mon->minvent; obj; obj = obj->nobj) {
	    if (obj->owornmask & mwflags)
		armac += arm_ac_bonus(obj);
	}

	base -= armac;
	/* since arm_ac_bonus is positive, subtracting it increases AC */
	return base;
}

int
full_marmorac(mon)
struct monst *mon;
{
	struct obj *obj;
	int armac = 0;
	long mwflags = mon->misc_worn_check;
	
	if(mon->data == &mons[PM_GIANT_TURTLE] && mon->mflee){
		armac += 15;
	}
	
	if(mon->data == &mons[PM_DANCING_BLADE]){
		return -20;
	}
	
	if(mon->data == &mons[PM_HOD_SEPHIRAH]){
		if(uarm) armac += arm_ac_bonus(uarm);
		if(uarmf) armac += arm_ac_bonus(uarmf);
		if(uarmg) armac += arm_ac_bonus(uarmg);
		if(uarmu) armac += arm_ac_bonus(uarmu);
		if(uarms) armac += arm_ac_bonus(uarms);
		if(uarmh) armac += arm_ac_bonus(uarmh);
		if(uarmc) armac += arm_ac_bonus(uarmc);
		
		if(armac < 0) armac *= -1;
	}
	else for (obj = mon->minvent; obj; obj = obj->nobj) {
	    if (obj->owornmask & mwflags)
		armac += arm_ac_bonus(obj);
	}

	return 10 - armac;
}

int 
base_mdr(mon)
struct monst *mon;
{
	int base = 0, armac = 0;
	
	if(mon->data == &mons[PM_CHOKHMAH_SEPHIRAH]){
		base += u.chokhmah;
	}
	if(is_weeping(mon->data)){
		if(mon->mvar2 & 0x4L) base = +125; //Fully Quantum Locked
		if(mon->mvar2 & 0x2L) base = +5; //Partial Quantum Lock
	}
	else if(is_alabaster_mummy(mon->data) && mon->mvar1 == SYLLABLE_OF_SPIRIT__VAUL)
		base += 10;
	
	if(mon->mfaction == ZOMBIFIED) base += 2;
	if(mon->mfaction == CRYSTALFIED) base += 8;
	
	return base;
}

int
roll_mdr(mon, magr)
struct monst *mon;
struct monst *magr;
{
	struct obj *obj;
	int base, armac = 0, clkdr = 0;
	int agralign = 0;
	int agrmoral = 0;
	if(magr){
		agralign = (magr == &youmonst) ? sgn(u.ualign.type) : sgn(magr->data->maligntyp);
		
		if(magr == &youmonst){
			if(hates_holy(youracedata))
				agrmoral = -1;
			else if(hates_unholy(youracedata))
				agrmoral = 1;
		} else {
			if(hates_holy_mon(magr))
				agrmoral = -1;
			else if(hates_unholy_mon(magr))
				agrmoral = 1;
		}
	}
	
	base = base_mdr(mon);
	
	if(mon->data == &mons[PM_GIANT_TURTLE]){
		if(mon->mflee || rn2(2))
			base += 5;
	}
	if(is_uvuudaum(mon->data)){
		base += 10;//stoneskin
	}
	
	if(mon->mtame){
		if(u.specialSealsActive&SEAL_COSMOS) base += rnd(spiritDsize());
		if(active_glyph(IMPURITY)) base += 3;
	}
	
	//armor AC
	if(mon->data == &mons[PM_HOD_SEPHIRAH]){
		armac = roll_udr(magr);
		if(armac < 0) armac *= -1;
	} else {
		struct obj *curarm;
		int slot;
		
		if (which_armor(mon, W_ARMC)){
			curarm = which_armor(mon, W_ARMC);
			clkdr += arm_dr_bonus(curarm);
			if(magr) clkdr += properties_dr(curarm, agralign, agrmoral);
		} else if(MON_WEP(mon) && MON_WEP(mon)->oartifact == ART_TENSA_ZANGETSU){
			clkdr += max( 1 + (MON_WEP(mon)->spe+1)/2,0);
		}
		
		curarm = which_armor(mon, W_ARMU);
		if(curarm && curarm->otyp == BODYGLOVE){
			armac += arm_dr_bonus(curarm);
			if(magr) armac += properties_dr(curarm, agralign, agrmoral);
		}
		curarm = which_armor(mon, W_ARM);
		if(curarm && curarm->otyp == JUMPSUIT){
			armac += arm_dr_bonus(curarm);
			if(magr) armac += properties_dr(curarm, agralign, agrmoral);
		}
		//Note: Bias this somehow?
		slot = rn2(5);
		switch(slot){
			case UPPER_TORSO_DR:
mon_uppertorso:
				//Note: upper body (shirt plus torso armor)
				base += mon->data->bdr;
				if (which_armor(mon, W_ARMU)){
					curarm = which_armor(mon, W_ARMU);
					if(curarm->otyp != BODYGLOVE){
						armac += arm_dr_bonus(curarm);
						if(magr) armac += properties_dr(curarm, agralign, agrmoral);
					}
				}
			case LOWER_TORSO_DR:
mon_lowertorso:
				//Note: lower body (torso armor only)
				if (which_armor(mon, W_ARM)){
					curarm = which_armor(mon, W_ARM);
					if(curarm->otyp != JUMPSUIT){
						armac += arm_dr_bonus(curarm);
						if(magr) armac += properties_dr(curarm, agralign, agrmoral);
					}
				} else if(MON_WEP(mon) && MON_WEP(mon)->oartifact == ART_TENSA_ZANGETSU){
					armac += max( 1 + (MON_WEP(mon)->spe+1)/2,0);
				}
				armac += clkdr;
				//Lower body SPECIFIC modifiers
				if(slot == LOWER_TORSO_DR){
					base += mon->data->ldr;
					if (which_armor(mon, W_ARMU)){
						curarm = which_armor(mon, W_ARMU);
						if(curarm->otyp == BLACK_DRESS || curarm->otyp == VICTORIAN_UNDERWEAR){
							armac += arm_dr_bonus(curarm);
							if(magr) armac += properties_dr(curarm, agralign, agrmoral);
						}
					}
				}
			break;
			case HEAD_DR:
				if(!has_head_mon(mon)){
					slot = UPPER_TORSO_DR;
					goto mon_uppertorso;
				}
				base += mon->data->hdr;
				if (which_armor(mon, W_ARMH)){
					curarm = which_armor(mon, W_ARMH);
					armac += arm_dr_bonus(curarm);
					if(magr) armac += properties_dr(curarm, agralign, agrmoral);
				}
				armac += clkdr;
			break;
			case LEG_DR:
				if(!can_wear_boots(mon->data)){
					slot = LOWER_TORSO_DR;
					goto mon_lowertorso;
				}
				base += mon->data->fdr;
				if (which_armor(mon, W_ARMF)){
					curarm = which_armor(mon, W_ARMF);
					armac += arm_dr_bonus(curarm);
					if(magr) armac += properties_dr(curarm, agralign, agrmoral);
				} else if(MON_WEP(mon) && MON_WEP(mon)->oartifact == ART_TENSA_ZANGETSU){
					armac += max( 1 + (MON_WEP(mon)->spe+1)/2,0);
				}
				armac += clkdr;
			break;
			case ARM_DR:
				if(!can_wear_gloves(mon->data)){
					slot = UPPER_TORSO_DR;
					goto mon_uppertorso;
				}
				base += mon->data->gdr;
				if (which_armor(mon, W_ARMG)){
					curarm = which_armor(mon, W_ARMG);
					armac += arm_dr_bonus(curarm);
					if(magr) armac += properties_dr(curarm, agralign, agrmoral);
				} else if(MON_WEP(mon) && MON_WEP(mon)->oartifact == ART_TENSA_ZANGETSU){
					armac += max( 1 + (MON_WEP(mon)->spe+1)/2,0);
				}
			break;
		}
	}
	
	// if(armac && mon->mstdy){
		// armac -= mon->mstdy;
		// if(armac<0)
			// armac = 0;
	// }
	
	if(armac > 11) armac = rnd(armac-10) + 10; /* high armor dr values act like player ac values */

	base += armac;
	/* since arm_ac_bonus is positive, subtracting it increases AC */
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
	 *
	 * Give animals and mindless creatures a chance to wear their initial
	 * equipment.
	 */
	if ((is_animal(mon->data) || mindless_mon(mon)) && !creation)
		return;

	m_dowear_type(mon, W_AMUL, creation, FALSE);
#ifdef TOURIST
	/* can't put on shirt if already wearing suit */
	if (!(mon->misc_worn_check & W_ARM) || creation)
	    m_dowear_type(mon, W_ARMU, creation, FALSE);
#endif
	/* treating small as a special case allows
	   hobbits, gnomes, and kobolds to wear cloaks */
	m_dowear_type(mon, W_ARMC, creation, FALSE);
	m_dowear_type(mon, W_ARMH, creation, FALSE);
	if (!MON_WEP(mon) || !bimanual(MON_WEP(mon),mon->data))
	    m_dowear_type(mon, W_ARMS, creation, FALSE);
	m_dowear_type(mon, W_ARMG, creation, FALSE);
	m_dowear_type(mon, W_ARMF, creation, FALSE);
	m_dowear_type(mon, W_ARM, creation, FALSE);
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
	
	if(is_whirly(mon->data) || noncorporeal(mon->data)) return;

	/* Get a copy of monster's name before altering its visibility */
	Strcpy(nambuf, See_invisible(mon->mx,mon->my) ? Monnam(mon) : mon_nam(mon));

	old = which_armor(mon, flag);
	if (old && old->cursed) return;
	if (old && flag == W_AMUL) return; /* no such thing as better amulets */
	best = old;

	for(obj = mon->minvent; obj; obj = obj->nobj) {
	    switch(flag) {
		case W_AMUL:
		    if (obj->oclass != AMULET_CLASS ||
				!can_wear_amulet(mon->data) || 
			    (obj->otyp != AMULET_OF_LIFE_SAVING &&
				obj->otyp != AMULET_OF_REFLECTION))
			continue;
		    best = obj;
		    goto outer_break; /* no such thing as better amulets */
		case W_ARMU:
		    if (!is_shirt(obj) || obj->objsize != mon->data->msize || !shirt_match(mon->data,obj)) continue;
		    break;
		case W_ARMC:
			if(mon->data == &mons[PM_CATHEZAR] && obj->otyp == CHAIN)
				break;
		    if (!is_cloak(obj) || (abs(obj->objsize - mon->data->msize) > 1)) continue;
		    break;
		case W_ARMH:
			if(mon->data == &mons[PM_CATHEZAR] && obj->otyp == CHAIN)
				break;
		    if (!is_helmet(obj) || ((!helm_match(mon->data,obj) || !has_head(mon->data) || obj->objsize != mon->data->msize) && !is_flimsy(obj))) continue;
		    /* (flimsy exception matches polyself handling) */
		    if (has_horns(mon->data) && !is_flimsy(obj)) continue;
		    break;
		case W_ARMS:
		    if (cantwield(mon->data) || !is_shield(obj)) continue;
		    break;
		case W_ARMG:
			if((mon->data == &mons[PM_CATHEZAR] || mon->data == &mons[PM_WARDEN_ARIANNA]) && obj->otyp == CHAIN)
				break;
		    if (!is_gloves(obj) || obj->objsize != mon->data->msize || !can_wear_gloves(mon->data)) continue;
		    break;
		case W_ARMF:
			if((mon->data == &mons[PM_WARDEN_ARIANNA]) && obj->otyp == CHAIN)
				break;
		    if (!is_boots(obj) || obj->objsize != mon->data->msize || !can_wear_boots(mon->data)) continue;
		    break;
		case W_ARM:
			if((mon->data == &mons[PM_CATHEZAR] || mon->data == &mons[PM_WARDEN_ARIANNA]) && obj->otyp == CHAIN)
				break;
		    if (!is_suit(obj) || (!Is_dragon_scales(obj) && (!arm_match(mon->data, obj) || (obj->objsize != mon->data->msize &&
				!(is_elven_armor(obj) && abs(obj->objsize - mon->data->msize) <= 1))))
			) continue;
		    break;
	    }
	    if (obj->owornmask) continue;
	    /* I'd like to define a VISIBLE_arm_ac_bonus which doesn't assume the
	     * monster knows obj->spe, but if I did that, a monster would keep
	     * switching forever between two -2 caps since when it took off one
	     * it would forget spe and once again think the object is better
	     * than what it already has.
	     */
	    if (best && (arm_total_bonus(best) + extra_pref(mon,best) >= arm_total_bonus(obj) + extra_pref(mon,obj)))
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
		if (mon->minvis && !See_invisible(mon->mx,mon->my)) {
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
	int i;

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
	for(i=0;i<10;i++)
		for (otmp = magic_chest_objs[i]; otmp; otmp = otmp->nobj)
			otmp->bypass = 0;
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
	boolean handless_or_tiny = (nohands(mdat) || nolimbs(mdat) || verysmall(mdat));
	const char *pronoun = mhim(mon),
			*ppronoun = mhis(mon);

	if ((otmp = which_armor(mon, W_ARM)) != 0) {
		if ((Is_dragon_scales(otmp) &&
			mdat == Dragon_scales_to_pm(otmp)) ||
			(Is_dragon_mail(otmp) && mdat == Dragon_mail_to_pm(otmp)))
			m_useup(mon, otmp);	/* no message here;
			   "the dragon merges with his scaly armor" is odd
			   and the monster's previous form is already gone */
		else if((otmp->objsize != mon->data->msize && !(is_elven_armor(otmp) && abs(otmp->objsize - mon->data->msize) <= 1))
				|| !arm_match(mon->data,otmp) || is_whirly(mon->data) || noncorporeal(mon->data)
		){
			if (otmp->oartifact || otmp->objsize > mon->data->msize || is_whirly(mon->data) || noncorporeal(mon->data)) {
				if (vis)
					pline("%s armor falls around %s!",
						s_suffix(Monnam(mon)), pronoun);
				else
					You_hear("a thud.");
				if (polyspot) bypass_obj(otmp);
				m_lose_armor(mon, otmp);
			} else if (vis){
				pline("%s breaks out of %s armor!", Monnam(mon), ppronoun);
				m_useup(mon, otmp);
			} else {
				You_hear("a cracking sound.");
				m_useup(mon, otmp);
			}
		}
	}
	if ((otmp = which_armor(mon, W_ARMC)) != 0) {
		if(abs(otmp->objsize - mon->data->msize) > 1 || is_whirly(mon->data) || noncorporeal(mon->data)){
			if (otmp->oartifact || otmp->objsize > mon->data->msize || is_whirly(mon->data) || noncorporeal(mon->data)) {
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
	}
	if ((otmp = which_armor(mon, W_ARMU)) != 0) {
		if(otmp->objsize != mon->data->msize || !shirt_match(mon->data,otmp) || is_whirly(mon->data) || noncorporeal(mon->data)){
			if (otmp->oartifact || otmp->objsize > mon->data->msize || is_whirly(mon->data) || noncorporeal(mon->data)) {
				if (vis)
				pline("%s %s falls off!", s_suffix(Monnam(mon)),
					cloak_simple_name(otmp));
				if (polyspot) bypass_obj(otmp);
				m_lose_armor(mon, otmp);
			} else {
				if (vis)
					pline("%s shirt rips to shreds!", s_suffix(Monnam(mon)));
				else
					You_hear("a ripping sound.");
				m_useup(mon, otmp);
			}
		}
	}
	if ((otmp = which_armor(mon, W_ARMG)) != 0) {
		if(nohands(mon->data) || nolimbs(mon->data) || otmp->objsize != mon->data->msize || is_whirly(mon->data) || noncorporeal(mon->data)){
			if (vis)
				pline("%s drops %s gloves!", Monnam(mon), ppronoun);
			if (polyspot) bypass_obj(otmp);
			m_lose_armor(mon, otmp);
		}
	}
	if ((otmp = which_armor(mon, W_ARMS)) != 0) {
		if(nohands(mon->data) || nolimbs(mon->data) || bimanual(MON_WEP(mon),mon->data) || is_whirly(mon->data) || noncorporeal(mon->data)){
			if (vis)
				pline("%s can no longer hold %s shield!", Monnam(mon), ppronoun);
			else
				You_hear("a clank.");
			if (polyspot) bypass_obj(otmp);
			m_lose_armor(mon, otmp);
		}
	}
	if ((otmp = which_armor(mon, W_ARMH)) != 0 &&
		/* flimsy test for horns matches polyself handling */
		(!is_flimsy(otmp) || is_whirly(mon->data) || noncorporeal(mon->data))
	) {
		if(!has_head(mon->data) || mon->data->msize != otmp->objsize || !helm_match(mon->data,otmp) || has_horns(mon->data)
			 || is_whirly(mon->data) || noncorporeal(mon->data)
		){
			if (vis)
				pline("%s helmet falls to the %s!",
				  s_suffix(Monnam(mon)), surface(mon->mx, mon->my));
			else
				You_hear("a clank.");
			if (polyspot) bypass_obj(otmp);
			m_lose_armor(mon, otmp);
		}
	}
	if ((otmp = which_armor(mon, W_ARMF)) != 0) {
		if(noboots(mon->data) || !humanoid(mon->data) || mon->data->msize != otmp->objsize || is_whirly(mon->data) || noncorporeal(mon->data)){
			if (vis) {
				if (is_whirly(mon->data) || noncorporeal(mon->data))
					pline("%s %s falls, unsupported!",
							 s_suffix(Monnam(mon)), cloak_simple_name(otmp));
				else pline("%s boots %s off %s feet!",
				s_suffix(Monnam(mon)),
				mon->data->msize < otmp->objsize ? "slide" : "are pushed", ppronoun);
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
			!Stone_resistance && rnl(100) >= 33) {
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
int
extra_pref(mon, obj)
struct monst *mon;
struct obj *obj;
{
	if (!obj)
		return 0;

	/* specific item types that are more than their oc_oprop */
	switch (obj->otyp)
	{
		/* gloves */
	case GAUNTLETS_OF_POWER:
		return 2;
		break;
	case GAUNTLETS_OF_DEXTERITY:
		return (obj->spe / 2);
		break;
		/* cloaks */
	case ALCHEMY_SMOCK:
		if (!resists_acid(mon) || !resists_poison(mon))
			return 5;
		break;
	case MUMMY_WRAPPING:
		if (mon->data->mlet == S_MUMMY)
			return 30;
		else if (mon->mtame && mon->minvis && !See_invisible_old)
			return 10;
		else if (mon->minvis)
			return -5;
		break;
	}

	/* oc_oprop -- does not include extra properties
	 * such as the alchemy smock or object properties */
	switch (objects[obj->otyp].oc_oprop)
	{
	case ANTIMAGIC:
		if (!resists_magm(mon))
			return 20;
		break;
	case REFLECTING:
		if (!mon_reflects(mon, (char *)0))
			return 18;
		break;
	case FAST:
		if (mon->permspeed != MFAST)
			return 15;
		break;
	case FLYING:
		if (!mon_resistance(mon, FLYING))
			return 10;
		break;
	case DISPLACED:
		if (!mon_resistance(mon, DISPLACED))
			return 8;
		break;
	case STONE_RES:
		if (!resists_ston(mon))
			return 7;
		break;
	case SICK_RES:
		if (!resists_sickness(mon))
			return 5;
		break;
	case FIRE_RES:
		if (!resists_fire(mon))
			return 3;
	case COLD_RES:
		if (!resists_cold(mon))
			return 3;
	case SHOCK_RES:
		if (!resists_elec(mon))
			return 3;
	case ACID_RES:
		if (!resists_acid(mon))
			return 3;
	case POISON_RES:
		if (!resists_poison(mon))
			return 3;
	case SLEEP_RES:
		if (!resists_sleep(mon))
			return 3;
	case DRAIN_RES:
		if (!resists_drli(mon))
			return 3;
		break;
	case TELEPAT:
		if (!mon_resistance(mon, TELEPAT))
			return 1;
		break;
	case FUMBLING:
		return -20;
		break;
	/* pets prefer not to wear items that make themselves invisible to you */
	case INVIS:
		if (mon->mtame && !See_invisible_old)
			return -20;
		else if (!mon->minvis)
			return 5;
		break;
	}
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
		if((levl[x][y].lit == 0 && 
			!(viz_array[y][x]&TEMP_LIT1 && !(viz_array[y][x]&TEMP_DRK1)))
		   || (levl[x][y].lit && 
			(viz_array[y][x]&TEMP_DRK1 && !(viz_array[y][x]&TEMP_LIT1)))
		){
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
		
		if (Invis){
			start_timer(1, TIMER_OBJECT, LIGHT_DAMAGE, (genericptr_t)obj);
			return;
		}
		
		if((levl[u.ux][u.uy].lit == 0 && 
			!(viz_array[u.uy][u.ux]&TEMP_LIT1 && !(viz_array[u.uy][u.ux]&TEMP_DRK1)))
		  || (levl[u.ux][u.uy].lit && 
			(viz_array[u.uy][u.ux]&TEMP_DRK1 && !(viz_array[u.uy][u.ux]&TEMP_LIT1)))
		  || ((rn2(3) < armpro) && rn2(50))
		){
			if(obj->oeroded && obj->oerodeproof && 
				((levl[u.ux][u.uy].lit == 0 && 
					!(viz_array[u.uy][u.ux]&TEMP_LIT1 && !(viz_array[u.uy][u.ux]&TEMP_DRK1)))
				|| (levl[u.ux][u.uy].lit && 
					(viz_array[u.uy][u.ux]&TEMP_DRK1 && !(viz_array[u.uy][u.ux]&TEMP_LIT1)))
				)
			) obj->oeroded--;
			start_timer(1, TIMER_OBJECT,
						LIGHT_DAMAGE, (genericptr_t)obj);
			return;
		}
		if(obj->oeroded < 2){
			obj->oeroded++;
			Your("%s degrade%s.",xname(obj),(obj->quan > 1L ? "" : "s"));
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
		if (obj->ocarry->perminvis || obj->ocarry->minvis){
			start_timer(1, TIMER_OBJECT, LIGHT_DAMAGE, (genericptr_t)obj);
			return;
		}
		if((levl[obj->ocarry->mx][obj->ocarry->my].lit == 0 && 
				!(viz_array[obj->ocarry->my][obj->ocarry->mx]&TEMP_LIT1 && !(viz_array[obj->ocarry->my][obj->ocarry->mx]&TEMP_DRK1)))
			|| (levl[obj->ocarry->mx][obj->ocarry->my].lit && 
				(viz_array[obj->ocarry->my][obj->ocarry->mx]&TEMP_DRK1 && !(viz_array[obj->ocarry->my][obj->ocarry->mx]&TEMP_LIT1)))
			|| ((rn2(3) < armpro) && rn2(50))){
			if(obj->oeroded && obj->oerodeproof 
				&& ((levl[obj->ocarry->mx][obj->ocarry->my].lit == 0 && 
					!(viz_array[obj->ocarry->my][obj->ocarry->mx]&TEMP_LIT1 && !(viz_array[obj->ocarry->my][obj->ocarry->mx]&TEMP_DRK1))) 
				   || (levl[obj->ocarry->mx][obj->ocarry->my].lit && 
					(viz_array[obj->ocarry->my][obj->ocarry->mx]&TEMP_DRK1 && !(viz_array[obj->ocarry->my][obj->ocarry->mx]&TEMP_LIT1)))
				)
			) obj->oeroded--;
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
	    else if (obj == MON_SWEP(obj->ocarry)) {
			setmnotwielded(obj->ocarry,obj);
			MON_NOSWEP(obj->ocarry);
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
def_mountedCombat()
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

/*worn.c*/
