/*	SCCS Id: @(#)dothrow.c	3.4	2003/12/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* Contains code for 't' (throw) */

#include "hack.h"
#include "edog.h"

STATIC_DCL void NDECL(autoquiver);
STATIC_DCL int FDECL(gem_accept, (struct monst *, struct obj *));
STATIC_DCL void FDECL(tmiss, (struct obj *, struct monst *));
STATIC_DCL int FDECL(throw_gold, (struct obj *));
STATIC_DCL void FDECL(check_shop_obj, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P));
STATIC_DCL void FDECL(breakobj, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P,BOOLEAN_P));
STATIC_DCL void FDECL(breakmsg, (struct obj *,BOOLEAN_P));
STATIC_DCL boolean FDECL(toss_up,(struct obj *, BOOLEAN_P));
STATIC_DCL boolean FDECL(throwing_weapon, (struct obj *));
STATIC_DCL void FDECL(sho_obj_return_to_u, (struct obj *obj, int, int));
STATIC_DCL boolean FDECL(mhurtle_step, (genericptr_t,int,int));
STATIC_DCL boolean FDECL(quest_arti_hits_leader, (struct obj *,struct monst *));


static NEARDATA const char toss_objs[] =
	{ ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, WEAPON_CLASS, 0 };
/* different default choices when wielding a sling (gold must be included) */
static NEARDATA const char bullets[] =
	{ ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, GEM_CLASS, 0 };

struct obj *thrownobj = 0;	/* tracks an object until it lands */

extern boolean notonhead;	/* for long worms */
//definition of an extern in you.h
boolean barage=FALSE;

/* Throw the selected object, asking for direction */
int
throw_obj(obj, shotlimit)
struct obj *obj;
int shotlimit;
{
	struct obj *otmp;
	int multishot = 1;
	schar skill;
	long wep_mask;
	boolean twoweap;

	/* ask "in what direction?" */
#ifndef GOLDOBJ
	if (!getdir((char *)0)) {
		if (obj->oclass == COIN_CLASS) {
		    u.ugold += obj->quan;
		    flags.botl = 1;
		    dealloc_obj(obj);
		}
		return(0);
	}

	if(obj->oclass == COIN_CLASS) return(throw_gold(obj));
#else
	if (!getdir((char *)0)) {
	    /* obj might need to be merged back into the singular gold object */
	    freeinv(obj);
	    addinv(obj);
	    return(0);
	}

        /*
	  Throwing money is usually for getting rid of it when
          a leprechaun approaches, or for bribing an oncoming 
          angry monster.  So throw the whole object.

          If the money is in quiver, throw one coin at a time,
          possibly using a sling.
        */
	if(obj->oclass == COIN_CLASS && obj != uquiver) return(throw_gold(obj));
#endif

	if(!canletgo(obj,"throw"))
		return(0);
	if ( (obj->oartifact == ART_MJOLLNIR || 
			obj->oartifact == ART_AXE_OF_THE_DWARVISH_LORDS) && 
			obj != uwep) {
	    pline("%s must be wielded before it can be thrown.",
		The(xname(obj)));
		return(0);
	}
	if ((obj->oartifact == ART_MJOLLNIR || 
			obj->oartifact == ART_AXE_OF_THE_DWARVISH_LORDS) && ACURR(A_STR) < STR19(25)
	   || (is_boulder(obj) && !throws_rocks(youmonst.data) && !(u.sealsActive&SEAL_YMIR))) {
		pline("It's too heavy.");
		return(1);
	}
	if(!u.dx && !u.dy && !u.dz) {
		You("cannot throw an object at yourself.");
		return(0);
	}
	
	if(obj->ostolen && u.sealsActive&SEAL_ANDROMALIUS) unbind(SEAL_ANDROMALIUS, TRUE);
	if((obj->oclass == POTION_CLASS || objects[obj->otyp].oc_material == GLASS) && u.sealsActive&SEAL_ASTAROTH) unbind(SEAL_ASTAROTH, TRUE);
	if((obj->otyp == EGG) && u.sealsActive&SEAL_ECHIDNA) unbind(SEAL_ECHIDNA, TRUE);
	
	u_wipe_engr(2);
	if (!uarmg && !Stone_resistance && (obj->otyp == CORPSE &&
		    touch_petrifies(&mons[obj->corpsenm]))) {
		You("throw the %s corpse with your bare %s.",
		    mons[obj->corpsenm].mname, body_part(HAND));
		Sprintf(killer_buf, "%s corpse", an(mons[obj->corpsenm].mname));
		instapetrify(killer_buf);
	}
	if (welded(obj)) {
		weldmsg(obj);
		return 1;
	}

	/* Multishot calculations
	 */
	skill = objects[obj->otyp].oc_skill;
	if (((ammo_and_launcher(obj, uwep) && skill != -P_CROSSBOW) || (skill == P_DAGGER && !Role_if(PM_WIZARD)) ||
			skill == -P_DART || skill == -P_SHURIKEN || skill == -P_BOOMERANG || obj->oartifact == ART_SICKLE_MOON ) &&
		!(Confusion || Stunned)) {
	    /* Bonus if the player is proficient in this weapon... */
	    switch (P_SKILL(weapon_type(obj))) {
	    default:	break; /* No bonus */
	    case P_SKILLED:	multishot++; break;
	    case P_EXPERT:	multishot += 2; break;
	    }
	    /* ...or is using the legendary Longbow... */
		if(ammo_and_launcher(obj, uwep)
			&& (uwep->oartifact == ART_LONGBOW_OF_DIANA || uwep->oartifact == ART_BELTHRONDING)
		) multishot++;
	    /* ...or is using a special weapon for their role... */
	    switch (Role_switch) {
	    case PM_RANGER:
		multishot++;
		if(ammo_and_launcher(obj, uwep) && uwep->oartifact == ART_LONGBOW_OF_DIANA) multishot++;//double bonus for Rangers
		break;
	    case PM_ROGUE:
		if (skill == P_DAGGER) multishot++;
		break;
	    case PM_SAMURAI:
		if (obj->otyp == YA && uwep && uwep->otyp == YUMI) multishot++;
		break;
	    default:
		break;	/* No bonus */
	    }
	    /* ...or using their race's special bow */
	    switch (Race_switch) {
	    case PM_ELF:
			if (obj->otyp == ELVEN_ARROW && uwep &&
					uwep->otyp == ELVEN_BOW) multishot++;
			if(obj->oartifact == ART_SICKLE_MOON) multishot++;
			if(ammo_and_launcher(obj, uwep) && uwep->oartifact == ART_BELTHRONDING) multishot++;//double bonus for Elves
		break;
	    case PM_DROW:
			if(obj->oartifact == ART_SICKLE_MOON) multishot++;
		break;
	    case PM_ORC:
			if (obj->otyp == ORCISH_ARROW && uwep &&
					uwep->otyp == ORCISH_BOW) multishot++;
		break;
	    // case PM_GNOME:
			// if (obj->otyp == CROSSBOW_BOLT && uwep &&
					// uwep->otyp == CROSSBOW) multishot++;
		// break;
	    default:
		break;	/* No bonus */
	    }
	}
	
	if(!barage) multishot = rnd(multishot);
	else multishot += u.ulevel/10+1; //state variable, we are doing a spirit power barage
	if(ammo_and_launcher(obj, uwep) && uwep->oartifact == ART_WRATHFUL_SPIDER) multishot += rn2(8);
	
	if ((long)multishot > obj->quan && obj->oartifact != ART_WINDRIDER 
		&& obj->oartifact != ART_SICKLE_MOON) multishot = (int)obj->quan;
	if (shotlimit > 0 && multishot > shotlimit) multishot = shotlimit;
	
	m_shot.s = ammo_and_launcher(obj,uwep) ? TRUE : FALSE;
	/* give a message if shooting more than one, or if player
	   attempted to specify a count */
	if (multishot > 1 || shotlimit > 0) {
	    /* "You shoot N arrows." or "You throw N daggers." */
		if(obj->quan > 1) You("%s %d %s.",
		m_shot.s ? "shoot" : "throw",
		multishot,	/* (might be 1 if player gave shotlimit) */
		(multishot == 1) ? singular(obj, xname) : obj->quan == 1 ? the(xname(obj)) : xname(obj));
		else if(multishot>1) You("%s %s %d times.",
		m_shot.s ? "shoot" : "throw",
		(multishot == 1) ? singular(obj, xname) : obj->quan == 1 ? the(xname(obj)) : xname(obj),
		multishot);	/* (might be 1 if player gave shotlimit) */
	}

	wep_mask = obj->owornmask;
	m_shot.o = obj->otyp;
	m_shot.n = multishot;
	for (m_shot.i = 1; m_shot.i <= m_shot.n; m_shot.i++) {
		if(!obj || obj->where != OBJ_INVENT) break; //Weapon lost
	    twoweap = u.twoweap;
	    /* split this object off from its slot if necessary */
	    if (obj->quan > 1L) {
		otmp = splitobj(obj, 1L);
	    } else {
		otmp = obj;
		if (otmp->owornmask)
		    remove_worn_item(otmp, FALSE);
	    }
	    freeinv(otmp);
	    throwit(otmp, wep_mask, twoweap);
	}
	m_shot.n = m_shot.i = 0;
	m_shot.o = STRANGE_OBJECT;
	m_shot.s = FALSE;

	return 1;
}


int
dothrow()
{
	register struct obj *obj;
	int shotlimit;

	/*
	 * Since some characters shoot multiple missiles at one time,
	 * allow user to specify a count prefix for 'f' or 't' to limit
	 * number of items thrown (to avoid possibly hitting something
	 * behind target after killing it, or perhaps to conserve ammo).
	 *
	 * Prior to 3.3.0, command ``3t'' meant ``t(shoot) t(shoot) t(shoot)''
	 * and took 3 turns.  Now it means ``t(shoot at most 3 missiles)''.
	 */
	/* kludge to work around parse()'s pre-decrement of `multi' */
	shotlimit = (multi || save_cm) ? multi + 1 : 0;
	multi = 0;		/* reset; it's been used up */

	if (notake(youmonst.data)) {
	    You("are physically incapable of throwing anything.");
	    return 0;
	}

	if(check_capacity((char *)0)) return(0);
	obj = getobj(uslinging() ? bullets : toss_objs, "throw");
	/* it is also possible to throw food */
	/* (or jewels, or iron balls... ) */

	if (!obj) return(0);
	return throw_obj(obj, shotlimit);
}


/* KMH -- Automatically fill quiver */
/* Suggested by Jeffrey Bay <jbay@convex.hp.com> */
static void
autoquiver()
{
	struct obj *otmp, *oammo = 0, *omissile = 0, *omisc = 0, *altammo = 0;

	if (uquiver)
	    return;

	/* Scan through the inventory */
	for (otmp = invent; otmp; otmp = otmp->nobj) {
	    if (otmp->owornmask || otmp->oartifact || !otmp->dknown) {
		;	/* Skip it */
	    } else if (otmp->otyp == ROCK ||
			/* seen rocks or known flint or known glass */
			(objects[otmp->otyp].oc_name_known &&
			 otmp->otyp == FLINT) ||
			(objects[otmp->otyp].oc_name_known &&
			 otmp->oclass == GEM_CLASS &&
			 objects[otmp->otyp].oc_material == GLASS)) {
		if (uslinging())
		    oammo = otmp;
		else if (ammo_and_launcher(otmp, uswapwep))
		    altammo = otmp;
		else if (!omisc)
		    omisc = otmp;
	    } else if (otmp->oclass == GEM_CLASS) {
		;	/* skip non-rock gems--they're ammo but
			   player has to select them explicitly */
	    } else if (is_ammo(otmp)) {
		if (ammo_and_launcher(otmp, uwep))
		    /* Ammo matched with launcher (bow and arrow, crossbow and bolt) */
		    oammo = otmp;
		else if (ammo_and_launcher(otmp, uswapwep))
		    altammo = otmp;
		else
		    /* Mismatched ammo (no better than an ordinary weapon) */
		    omisc = otmp;
	    } else if (is_missile(otmp)) {
		/* Missile (dart, shuriken, etc.) */
		omissile = otmp;
	    } else if (otmp->oclass == WEAPON_CLASS && throwing_weapon(otmp)) {
		/* Ordinary weapon */
		if (objects[otmp->otyp].oc_skill == P_DAGGER
			&& !omissile) 
		    omissile = otmp;
		else
		    omisc = otmp;
	    }
	}

	/* Pick the best choice */
	if (oammo)
	    setuqwep(oammo);
	else if (omissile)
	    setuqwep(omissile);
	else if (altammo)
	    setuqwep(altammo);
	else if (omisc)
	    setuqwep(omisc);

	return;
}


/* Throw from the quiver */
int
dofire()
{
	int shotlimit;

	if (notake(youmonst.data)) {
	    You("are physically incapable of doing that.");
	    return 0;
	}
	
	if(uwep && (!uquiver || (!is_ammo(uquiver) && !ammo_and_launcher(uquiver, uwep))) && uwep->oartifact && 
		(
		(uwep->oartifact == ART_MJOLLNIR && Role_if(PM_VALKYRIE) && ACURR(A_STR) == STR19(25)) ||
		(uwep->oartifact == ART_AXE_OF_THE_DWARVISH_LORDS && Race_if(PM_DWARF) && ACURR(A_STR) == STR19(25))
		// (uwep->oartifact == ART_SICKLE_MOON)
		)
	){
		/*See below for shotlimit*/
		shotlimit = (multi || save_cm) ? multi + 1 : 0;
		multi = 0;		/* reset; it's been used up */

		return throw_obj(uwep, shotlimit);
	}

	if(check_capacity((char *)0)) return(0);
	if (!uquiver) {
		if (!flags.autoquiver) {
			/* Don't automatically fill the quiver */
			You("have no ammunition readied!");
			if (iflags.quiver_fired)
			  dowieldquiver(); /* quiver_fired */
			if (!uquiver)
			  return(dothrow());
		} else {
		  autoquiver();
		  if (!uquiver) {
			You("have nothing appropriate for your quiver!");
			return(dothrow());
		  } else {
			You("fill your quiver:");
			prinv((char *)0, uquiver, 0L);
		  }
		}
	}

	/*
	 * Since some characters shoot multiple missiles at one time,
	 * allow user to specify a count prefix for 'f' or 't' to limit
	 * number of items thrown (to avoid possibly hitting something
	 * behind target after killing it, or perhaps to conserve ammo).
	 *
	 * The number specified can never increase the number of missiles.
	 * Using ``5f'' when the shooting skill (plus RNG) dictates launch
	 * of 3 projectiles will result in 3 being shot, not 5.
	 */
	/* kludge to work around parse()'s pre-decrement of `multi' */
	shotlimit = (multi || save_cm) ? multi + 1 : 0;
	multi = 0;		/* reset; it's been used up */

	return throw_obj(uquiver, shotlimit);
}


/*
 * Object hits floor at hero's feet.  Called from drop() and throwit().
 */
void
hitfloor(obj)
register struct obj *obj;
{
	if (IS_SOFT(levl[u.ux][u.uy].typ) || u.uinwater) {
		dropy(obj);
		return;
	}
	if (IS_ALTAR(levl[u.ux][u.uy].typ))
		doaltarobj(obj);
	else
		pline("%s hit%s the %s.", Doname2(obj),
		      (obj->quan == 1L) ? "s" : "", surface(u.ux,u.uy));

	if (hero_breaks(obj, u.ux, u.uy, TRUE)) return;
	if (ship_object(obj, u.ux, u.uy, FALSE)) return;
	dropy(obj);
	if (!u.uswallow) container_impact_dmg(obj);
}

/*
 * Walk a path from src_cc to dest_cc, calling a proc for each location
 * except the starting one.  If the proc returns FALSE, stop walking
 * and return FALSE.  If stopped early, dest_cc will be the location
 * before the failed callback.
 */
boolean
walk_path(src_cc, dest_cc, check_proc, arg)
    coord *src_cc;
    coord *dest_cc;
    boolean FDECL((*check_proc), (genericptr_t, int, int));
    genericptr_t arg;
{
    int x, y, dx, dy, x_change, y_change, err, i, prev_x, prev_y;
    boolean keep_going = TRUE;

    /* Use Bresenham's Line Algorithm to walk from src to dest */
    dx = dest_cc->x - src_cc->x;
    dy = dest_cc->y - src_cc->y;
    prev_x = x = src_cc->x;
    prev_y = y = src_cc->y;

    if (dx < 0) {
	x_change = -1;
	dx = -dx;
    } else
	x_change = 1;
    if (dy < 0) {
	y_change = -1;
	dy = -dy;
    } else
	y_change = 1;

    i = err = 0;
    if (dx < dy) {
	while (i++ < dy) {
	    prev_x = x;
	    prev_y = y;
	    y += y_change;
	    err += dx;
	    if (err >= dy) {
		x += x_change;
		err -= dy;
	    }
	/* check for early exit condition */
	if (!(keep_going = (*check_proc)(arg, x, y)))
	    break;
	}
    } else {
	while (i++ < dx) {
	    prev_x = x;
	    prev_y = y;
	    x += x_change;
	    err += dy;
	    if (err >= dx) {
		y += y_change;
		err -= dx;
	    }
	/* check for early exit condition */
	if (!(keep_going = (*check_proc)(arg, x, y)))
	    break;
	}
    }

    if (keep_going)
	return TRUE;	/* successful */

    dest_cc->x = prev_x;
    dest_cc->y = prev_y;
    return FALSE;
}

/*
 * Single step for the hero flying through the air from jumping, flying,
 * etc.  Called from hurtle() and jump() via walk_path().  We expect the
 * argument to be a pointer to an integer -- the range -- which is
 * used in the calculation of points off if we hit something.
 *
 * Bumping into monsters won't cause damage but will wake them and make
 * them angry.  Auto-pickup isn't done, since you don't have control over
 * your movements at the time.
 *
 * Possible additions/changes:
 *	o really attack monster if we hit one
 *	o set stunned if we hit a wall or door
 *	o reset nomul when we stop
 *	o creepy feeling if pass through monster (if ever implemented...)
 *	o bounce off walls
 *	o let jumps go over boulders
 */
boolean
hurtle_step(arg, x, y)
    genericptr_t arg;
    int x, y;
{
    int ox, oy, *range = (int *)arg;
    struct obj *obj;
    struct monst *mon;
    boolean may_pass = TRUE;
    struct trap *ttmp;
    
    if (!isok(x,y)) {
	You_feel("the spirits holding you back.");
	return FALSE;
    } else if (!in_out_region(x, y)) {
	return FALSE;
    } else if (*range == 0) {
	return FALSE;			/* previous step wants to stop now */
    }

    if (!Passes_walls || !(may_pass = may_passwall(x, y))) {
	if (IS_ROCK(levl[x][y].typ) || closed_door(x,y)) {
	    const char *s;

	    pline("Ouch!");
	    if (IS_TREES(levl[x][y].typ))
		s = "bumping into a tree";
	    else if (IS_ROCK(levl[x][y].typ))
		s = "bumping into a wall";
	    else
		s = "bumping into a door";
	    losehp(rnd(2+*range), s, KILLED_BY);
	    return FALSE;
	}
	if (levl[x][y].typ == IRONBARS) {
	    You("crash into some iron bars.  Ouch!");
	    losehp(rnd(2+*range), "crashing into iron bars", KILLED_BY);
	    return FALSE;
	}
	if ((obj = boulder_at(x,y)) != 0) {
	    You("bump into a %s.  Ouch!", xname(obj));
	    losehp(rnd(2+*range), "bumping into a heavy object", KILLED_BY);
	    return FALSE;
	}
	if (!may_pass) {
	    /* did we hit a no-dig non-wall position? */
	    You("smack into something!");
	    losehp(rnd(2+*range), "touching the edge of the universe", KILLED_BY);
	    return FALSE;
	}
	if ((u.ux - x) && (u.uy - y) &&
		bad_rock(youmonst.data,u.ux,y) && bad_rock(youmonst.data,x,u.uy)) {
	    boolean too_much = (invent && (inv_weight() + weight_cap() > 600));
	    /* Move at a diagonal. */
	    if (bigmonst(youmonst.data) || too_much) {
		You("%sget forcefully wedged into a crevice.",
			too_much ? "and all your belongings " : "");
		losehp(rnd(2+*range), "wedging into a narrow crevice", KILLED_BY);
		return FALSE;
	    }
	}
    }

    if ((mon = m_at(x, y)) != 0) {
	You("bump into %s.", a_monnam(mon));
	wakeup(mon);
	return FALSE;
    }
    if ((u.ux - x) && (u.uy - y) &&
	bad_rock(youmonst.data,u.ux,y) && bad_rock(youmonst.data,x,u.uy)) {
	/* Move at a diagonal. */
	if (In_sokoban(&u.uz)) {
	    You("come to an abrupt halt!");
	    return FALSE;
	}
    }

    ox = u.ux;
    oy = u.uy;
    u.ux = x;
    u.uy = y;
    newsym(ox, oy);		/* update old position */
    vision_recalc(1);		/* update for new position */
    flush_screen(1);
    /* FIXME:
     * Each trap should really trigger on the recoil if
     * it would trigger during normal movement. However,
     * not all the possible side-effects of this are
     * tested [as of 3.4.0] so we trigger those that
     * we have tested, and offer a message for the
     * ones that we have not yet tested.
     */
    if ((ttmp = t_at(x, y)) != 0) {
    	if (ttmp->ttyp == MAGIC_PORTAL) {
    		dotrap(ttmp,0);
    		return FALSE;
	} else if (ttmp->ttyp == FIRE_TRAP) {
    		dotrap(ttmp,0);
	} else if ((ttmp->ttyp == PIT || ttmp->ttyp == SPIKED_PIT ||
		    ttmp->ttyp == HOLE || ttmp->ttyp == TRAPDOOR) &&
		   In_sokoban(&u.uz)) {
		/* Air currents overcome the recoil */
    		dotrap(ttmp,0);
		*range = 0;
		return TRUE;
    	} else {
		if (ttmp->tseen)
		    You("pass right over %s %s.",
		    	(ttmp->ttyp == ARROW_TRAP) ? "an" : "a",
		    	defsyms[trap_to_defsym(ttmp->ttyp)].explanation);
    	}
    }
    if (--*range < 0)		/* make sure our range never goes negative */
	*range = 0;
    if (*range != 0)
	delay_output();
    return TRUE;
}

STATIC_OVL boolean
mhurtle_step(arg, x, y)
    genericptr_t arg;
    int x, y;
{
	struct monst *mon = (struct monst *)arg;

	/* TODO: Treat walls, doors, iron bars, pools, lava, etc. specially
	 * rather than just stopping before.
	 */
	if (goodpos(x, y, mon, 0) && m_in_out_region(mon, x, y) && migrating_mons != mon) { /*If knockback causes monster to go fall through a hole, mon will be first migrating mon and hurtle should stop*/
	    remove_monster(mon->mx, mon->my);
	    newsym(mon->mx, mon->my);
	    place_monster(mon, x, y);
	    newsym(mon->mx, mon->my);
	    set_apparxy(mon);
	    (void) mintrap(mon);
	    return TRUE;
	}
	return FALSE;
}

/*
 * The player moves through the air for a few squares as a result of
 * throwing or kicking something.
 *
 * dx and dy should be the direction of the hurtle, not of the original
 * kick or throw and be only.
 */
void
hurtle(dx, dy, range, verbose)
    int dx, dy, range;
    boolean verbose;
{
    coord uc, cc;

    /* The chain is stretched vertically, so you shouldn't be able to move
     * very far diagonally.  The premise that you should be able to move one
     * spot leads to calculations that allow you to only move one spot away
     * from the ball, if you are levitating over the ball, or one spot
     * towards the ball, if you are at the end of the chain.  Rather than
     * bother with all of that, assume that there is no slack in the chain
     * for diagonal movement, give the player a message and return.
     */
    if(Punished && !carried(uball)) {
	You_feel("a tug from the iron ball.");
	nomul(0, NULL);
	return;
    } else if (u.utrap) {
	You("are anchored by the %s.",
	    u.utraptype == TT_WEB ? "web" : u.utraptype == TT_LAVA ? "lava" :
		u.utraptype == TT_INFLOOR ? surface(u.ux,u.uy) : "trap");
	nomul(0, NULL);
	return;
    }

    /* make sure dx and dy are [-1,0,1] */
    dx = sgn(dx);
    dy = sgn(dy);

    if(!range || (!dx && !dy) || u.ustuck) return; /* paranoia */

    nomul(-range, "moving through the air");
    if (verbose)
	You("%s in the opposite direction.", range > 1 ? "hurtle" : "float");
    /* if we're in the midst of shooting multiple projectiles, stop */
    if (m_shot.i < m_shot.n) {
	/* last message before hurtling was "you shoot N arrows" */
	You("stop %sing after the first %s.",
	    m_shot.s ? "shoot" : "throw", m_shot.s ? "shot" : "toss");
	m_shot.n = m_shot.i;	/* make current shot be the last */
    }
    if (In_sokoban(&u.uz))
	change_luck(-1);	/* Sokoban guilt */
    uc.x = u.ux;
    uc.y = u.uy;
    /* this setting of cc is only correct if dx and dy are [-1,0,1] only */
    cc.x = u.ux + (dx * range);
    cc.y = u.uy + (dy * range);
    (void) walk_path(&uc, &cc, hurtle_step, (genericptr_t)&range);
}

/* Move a monster through the air for a few squares.
 */
void
mhurtle(mon, dx, dy, range)
	struct monst *mon;
	int dx, dy, range;
{
    coord mc, cc;

	/* At the very least, debilitate the monster */
//	mon->movement = 0;
	mon->movement -= 10;
	mon->mstun = 1;

	/* Is the monster stuck or too heavy to push?
	 * (very large monsters have too much inertia, even floaters and flyers)
	 */
	if (mon->data->msize >= MZ_HUGE || mon == u.ustuck || mon->mtrapped)
	    return;

    /* Make sure dx and dy are [-1,0,1] */
    dx = sgn(dx);
    dy = sgn(dy);
    if(!range || (!dx && !dy)) return; /* paranoia */

	/* Send the monster along the path */
	mc.x = mon->mx;
	mc.y = mon->my;
	cc.x = mon->mx + (dx * range);
	cc.y = mon->my + (dy * range);
	(void) walk_path(&mc, &cc, mhurtle_step, (genericptr_t)mon);
	return;
}

STATIC_OVL void
check_shop_obj(obj, x, y, broken)
register struct obj *obj;
register xchar x, y;
register boolean broken;
{
	struct monst *shkp = shop_keeper(*u.ushops);

	if(!shkp) return;

	if(broken) {
		if (obj->unpaid) {
		    (void)stolen_value(obj, u.ux, u.uy,
				       (boolean)shkp->mpeaceful, FALSE);
		    subfrombill(obj, shkp);
		}
		obj->no_charge = 1;
		return;
	}

	if (!costly_spot(x, y) || *in_rooms(x, y, SHOPBASE) != *u.ushops) {
		/* thrown out of a shop or into a different shop */
		if (obj->unpaid) {
		    (void)stolen_value(obj, u.ux, u.uy,
				       (boolean)shkp->mpeaceful, FALSE);
		    subfrombill(obj, shkp);
		}
	} else {
		if (costly_spot(u.ux, u.uy) && costly_spot(x, y)) {
		    if(obj->unpaid) subfrombill(obj, shkp);
		    else if(!(x == shkp->mx && y == shkp->my))
			    sellobj(obj, x, y);
		}
	}
}

/*
 * Hero tosses an object upwards with appropriate consequences.
 *
 * Returns FALSE if the object is gone.
 */
STATIC_OVL boolean
toss_up(obj, hitsroof)
struct obj *obj;
boolean hitsroof;
{
    const char *almost;
    /* note: obj->quan == 1 */

    if (hitsroof) {
	if (breaktest(obj)) {
		pline("%s hits the %s.", Doname2(obj), ceiling(u.ux, u.uy));
		breakmsg(obj, !Blind);
		breakobj(obj, u.ux, u.uy, TRUE, TRUE);
		return FALSE;
	}
	almost = "";
    } else {
	almost = " almost";
    }
    pline("%s%s hits the %s, then falls back on top of your %s.",
	  Doname2(obj), almost, ceiling(u.ux,u.uy), body_part(HEAD));

    /* object now hits you */

    if (obj->oclass == POTION_CLASS) {
	potionhit(&youmonst, obj, TRUE);
    } else if (breaktest(obj)) {
	int otyp = obj->otyp, ocorpsenm = obj->corpsenm;
	int blindinc;

	/* need to check for blindness result prior to destroying obj */
	blindinc = (otyp == CREAM_PIE || otyp == BLINDING_VENOM) &&
		   /* AT_WEAP is ok here even if attack type was AT_SPIT */
		   can_blnd(&youmonst, &youmonst, AT_WEAP, obj) ? rnd(25) : 0;

	breakmsg(obj, !Blind);
	breakobj(obj, u.ux, u.uy, TRUE, TRUE);
	obj = 0;	/* it's now gone */
	switch (otyp) {
	case EGG:
		if (touch_petrifies(&mons[ocorpsenm]) &&
		    !uarmh && !Stone_resistance &&
		    !(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM)))
		goto petrify;
	case CREAM_PIE:
	case BLINDING_VENOM:
		pline("You've got it all over your %s!", body_part(FACE));
		if (blindinc) {
		    if (otyp == BLINDING_VENOM && !Blind)
			pline("It blinds you!");
		    u.ucreamed += blindinc;
		    make_blinded(Blinded + (long)blindinc, FALSE);
		    if (!Blind) Your(vision_clears);
		}
		break;
	default:
		break;
	}
	return FALSE;
    } else {		/* neither potion nor other breaking object */
	boolean less_damage = uarmh && is_metallic(uarmh), artimsg = FALSE;
	int dmg = dmgval(obj, &youmonst, 0);

	if (obj->oartifact)
	    /* need a fake die roll here; rn1(18,2) avoids 1 and 20 */
	    artimsg = artifact_hit((struct monst *)0, &youmonst,
				   obj, &dmg, rn1(18,2));

	if (!dmg) {	/* probably wasn't a weapon; base damage on weight */
	    dmg = (int) obj->owt / 100;
	    if (dmg < 1) dmg = 1;
	    else if (dmg > 6) dmg = 6;
	    if (youmonst.data->mlet == S_SHADE &&
		    objects[obj->otyp].oc_material != SILVER)
		dmg = 0;
	}
	if (dmg > 1 && less_damage) dmg = 1;
	if (dmg > 0) dmg += u.udaminc;
	if (dmg < 0) dmg = 0;	/* beware negative rings of increase damage */
	if (Half_physical_damage) dmg = (dmg + 1) / 2;

	if (uarmh) {
	    if (less_damage && dmg < (Upolyd ? u.mh : u.uhp)) {
		if (!artimsg && (is_metallic(uarmh) || uarmh->otyp == FLACK_HELMET || uarmh->otyp == DROVEN_HELM))
		    pline("Fortunately, you are wearing a hard helmet.");
	    } else if (flags.verbose &&
		    !(obj->otyp == CORPSE && touch_petrifies(&mons[obj->corpsenm])))
		Your("%s does not protect you.", xname(uarmh));
	} else if (obj->otyp == CORPSE && touch_petrifies(&mons[obj->corpsenm])) {
	    if (!Stone_resistance &&
		    !(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
 petrify:
		killer_format = KILLED_BY;
		killer = "elementary physics";	/* "what goes up..." */
		You("turn to stone.");
		if (obj) dropy(obj);	/* bypass most of hitfloor() */
		done(STONING);
		return obj ? TRUE : FALSE;
	    }
	}
	hitfloor(obj);
	losehp(dmg, "falling object", KILLED_BY_AN);
    }
    return TRUE;
}

/* return true for weapon meant to be thrown; excludes ammo */
STATIC_OVL boolean
throwing_weapon(obj)
struct obj *obj;
{
	return (is_missile(obj) || is_spear(obj) ||
		/* daggers and knife (excludes scalpel) */
		(is_blade(obj) && !is_sword(obj) &&
		 (objects[obj->otyp].oc_dir & PIERCE)) ||
		/* special cases [might want to add AXE] */
		obj->otyp == WAR_HAMMER || obj->otyp == AKLYS);
}

/* the currently thrown object is returning to you (not for boomerangs) */
STATIC_OVL void
sho_obj_return_to_u(obj, destx, desty)
struct obj *obj;
int destx;
int desty;
{
    /* might already be our location (bounced off a wall) */
    if (bhitpos.x != destx || bhitpos.y != desty) {
	int x = bhitpos.x - u.dx, y = bhitpos.y - u.dy;

	tmp_at(DISP_FLASH, obj_to_glyph(obj));
	while(x != destx || y != desty) {
	    tmp_at(x, y);
	    delay_output();
	    x -= u.dx; y -= u.dy;
	}
	tmp_at(DISP_END, 0);
    }
}

void
throwit(obj, wep_mask, twoweap)
register struct obj *obj;
long wep_mask;	/* used to re-equip returning boomerang */
boolean twoweap; /* used to restore twoweapon mode if wielded weapon returns */
{
	register struct monst *mon;
	register int range, urange;
	boolean impaired = (Confusion || Stunned || Blind ||
			   Hallucination || Fumbling);
	int startX = u.ux, startY = u.uy;

       obj->was_thrown = 1;

	if ((obj->cursed || obj->greased) && (u.dx || u.dy) && !rn2(7)) {
	    boolean slipok = TRUE;
	    if (ammo_and_launcher(obj, uwep))
		pline("%s!", Tobjnam(obj, "misfire"));
	    else {
		/* only slip if it's greased or meant to be thrown */
		if (obj->greased || throwing_weapon(obj))
		    /* BUG: this message is grammatically incorrect if obj has
		       a plural name; greased gloves or boots for instance. */
		    pline("%s as you throw it!", Tobjnam(obj, "slip"));
		else slipok = FALSE;
	    }
	    if (slipok) {
		u.dx = rn2(3)-1;
		u.dy = rn2(3)-1;
		if (!u.dx && !u.dy) u.dz = 1;
		impaired = TRUE;
	    }
	}

	if ((u.dx || u.dy || (u.dz < 1)) &&
	    calc_capacity((int)obj->owt) > SLT_ENCUMBER &&
	    (Upolyd ? (u.mh < 5 && u.mh != u.mhmax)
	     : (u.uhp < 10 && u.uhp != u.uhpmax)) &&
	    obj->owt > (unsigned)((Upolyd ? u.mh : u.uhp) * 2) &&
	    !Is_airlevel(&u.uz)) {
	    You("have so little stamina, %s drops from your grasp.",
		the(xname(obj)));
	    exercise(A_CON, FALSE);
	    u.dx = u.dy = 0;
	    u.dz = 1;
	}

	thrownobj = obj;

	if(u.uswallow) {
		mon = u.ustuck;
		bhitpos.x = mon->mx;
		bhitpos.y = mon->my;
	} else if(u.dz) {
	    if (u.dz < 0 &&  ( 
				(obj->oartifact == ART_MJOLLNIR &&
				 Role_if(PM_VALKYRIE)) || 
				(obj->oartifact == ART_AXE_OF_THE_DWARVISH_LORDS && 
				 Race_if(PM_DWARF)) ||
				 obj->oartifact == ART_WINDRIDER ||
				 obj->oartifact == ART_SICKLE_MOON
			  ) && !impaired) {
		pline("%s the %s and returns to your hand!",
		      Tobjnam(obj, "hit"), ceiling(u.ux,u.uy));
		obj = addinv(obj);
		(void) encumber_msg();
		if(obj->oartifact == ART_WINDRIDER || obj->oartifact == ART_SICKLE_MOON){
			setuqwep(obj);
		} else{
			setuwep(obj);
			u.twoweap = twoweap;
		}
	    } else if (u.dz < 0 && !Is_airlevel(&u.uz) &&
		    !Underwater && !Is_waterlevel(&u.uz)) {
		(void) toss_up(obj, rn2(5));
	    } else {
		hitfloor(obj);
	    }
	    thrownobj = (struct obj*)0;
	    return;

	} else if(obj->otyp == BOOMERANG && !Underwater) {
		if(Is_airlevel(&u.uz) || Levitation)
		    hurtle(-u.dx, -u.dy, 1, TRUE);
		mon = boomhit(u.dx, u.dy);
		if(mon == &youmonst) {		/* the thing was caught */
			exercise(A_DEX, TRUE);
			obj = addinv(obj);
			(void) encumber_msg();
			if (wep_mask && !(obj->owornmask & wep_mask)) {
			    setworn(obj, wep_mask);
			    u.twoweap = twoweap;
			}
			thrownobj = (struct obj*)0;
			return;
		}
	} else {
		urange = (int)(ACURRSTR)/2;
		/* balls are easy to throw or at least roll */
		/* also, this insures the maximum range of a ball is greater
		 * than 1, so the effects from throwing attached balls are
		 * actually possible
		 */
		if (obj->otyp == HEAVY_IRON_BALL)
			range = urange - (int)(obj->owt/100);
		else
			range = urange - (int)(obj->owt/40);
		if (obj == uball) {
			if (u.ustuck) range = 1;
			else if (range >= 5) range = 5;
		}
		if (range < 1) range = 1;

		if (is_ammo(obj)) {
			if (ammo_and_launcher(obj, uwep)) //make range of longbow of diana effectively unlimited
				if(uwep->oartifact == ART_LONGBOW_OF_DIANA || 
					(uwep->oartifact == ART_PEN_OF_THE_VOID && uwep->ovar1&SEAL_EVE && mvitals[PM_ACERERAK].died > 0)
				) range = 1000;
				else range++;
		    else if (obj->oclass != GEM_CLASS)
			range /= 2;
		}

		if (Is_airlevel(&u.uz) || Levitation) {
		    /* action, reaction... */
		    urange -= range;
		    if(urange < 1) urange = 1;
		    range -= urange;
		    if(range < 1) range = 1;
		}

		if (is_boulder(obj))
		    range = 20;		/* you must be giant */
		else if (obj->oartifact == ART_MJOLLNIR) //But the axe of D Lords can be thrown
		    range = (range + 1) / 2;	/* it's heavy */
		else if (obj == uball && u.utrap && u.utraptype == TT_INFLOOR)
		    range = 1;

		if (Underwater) range = 1;

		boolean obj_destroyed = FALSE;
		mon = bhit(u.dx, u.dy, range, THROWN_WEAPON,
			   (int FDECL((*),(MONST_P,OBJ_P)))0,
			   (int FDECL((*),(OBJ_P,OBJ_P)))0,
			   obj, &obj_destroyed);

		/* have to do this after bhit() so u.ux & u.uy are correct */
		if(Is_airlevel(&u.uz) || Levitation)
		    hurtle(-u.dx, -u.dy, urange, TRUE);

		if (obj_destroyed) return; /* fixes C343-100 */
	}

	if (mon) {
		boolean obj_gone;

		if (mon->isshk &&
		    obj->where == OBJ_MINVENT && obj->ocarry == mon) {
		    thrownobj = (struct obj*)0;
		    return;		/* alert shk caught it */
		}
		(void) snuff_candle(obj);
		notonhead = (bhitpos.x != mon->mx || bhitpos.y != mon->my);
		obj_gone = thitmonst(mon, obj);
		/* Monster may have been tamed; this frees old mon */
		mon = m_at(bhitpos.x, bhitpos.y);

		/* [perhaps this should be moved into thitmonst or hmon] */
		if (mon && mon->isshk &&
			(!inside_shop(u.ux, u.uy) ||
			 !index(in_rooms(mon->mx, mon->my, SHOPBASE), *u.ushops)))
		    hot_pursuit(mon);

		if (obj_gone) return;
	}

	if (u.uswallow) {
		/* ball is not picked up by monster */
		if (obj != uball) (void) mpickobj(u.ustuck,obj);
	} else {
		/* the code following might become part of dropy() */
		if ( ( 
				(obj->oartifact == ART_MJOLLNIR &&
				 Role_if(PM_VALKYRIE)) || 
				(obj->oartifact == ART_AXE_OF_THE_DWARVISH_LORDS && 
				 Race_if(PM_DWARF)) ||
				 obj->oartifact == ART_SICKLE_MOON ||
				 obj->oartifact == ART_WINDRIDER
			  )
		) {
		    /* we must be wearing Gauntlets of Power to get here */
		    if(obj->oartifact != ART_WINDRIDER) sho_obj_return_to_u(obj, startX, startY);	    /* display its flight */
			
			if(obj->oartifact != ART_WINDRIDER && (u.ux != startX || u.uy != startY)){
				if(flooreffects(obj,startX,startY,"fall")) return;
				obj_no_longer_held(obj);
				if (mon && mon->isshk && is_pick(obj)) {
					if (cansee(startX, startY))
					pline("%s snatches up %s.",
						  Monnam(mon), the(xname(obj)));
					if(*u.ushops)
					check_shop_obj(obj, startX, startY, FALSE);
					(void) mpickobj(mon, obj);	/* may merge and free obj */
					thrownobj = (struct obj*)0;
					return;
				}
				(void) snuff_candle(obj);
				if (!mon && ship_object(obj, startX, startY, FALSE)) {
					thrownobj = (struct obj*)0;
					return;
				}
				thrownobj = (struct obj*)0;
				place_object(obj, startX, startY);
				if(*u.ushops && obj != uball)
					check_shop_obj(obj, startX, startY, FALSE);

				stackobj(obj);
				if (obj == uball)
					drop_ball(startX, startY);
				if (cansee(startX, startY))
					newsym(startX,startY);
				if (obj_sheds_light(obj))
					vision_full_recalc = 1;
				if (!IS_SOFT(levl[startX][startY].typ))
					container_impact_dmg(obj);
				return;
			}
			
		    if (!impaired && (obj->blessed || rn2(100))) {
			pline("%s to your hand!", Tobjnam(obj, "return"));
			obj = addinv(obj);
			(void) encumber_msg();
			if(obj->oartifact == ART_WINDRIDER || obj->oartifact == ART_SICKLE_MOON){
				setuqwep(obj);
			} else{
				setuwep(obj);
				u.twoweap = twoweap;
			}
			if(cansee(bhitpos.x, bhitpos.y))
			    newsym(bhitpos.x,bhitpos.y);
		    } else {
			int dmg = rn2(2);
			if (!dmg) {
			    pline(Blind ? "%s lands %s your %s." :
					"%s back to you, landing %s your %s.",
				  Blind ? Something : Tobjnam(obj, "return"),
				  Levitation ? "beneath" : "at",
				  makeplural(body_part(FOOT)));
			} else {
			    dmg += rnd(3);
			    pline(Blind ? "%s your %s!" :
					"%s back toward you, hitting your %s!",
				  Tobjnam(obj, Blind ? "hit" : "fly"),
				  body_part(ARM));
			    (void) artifact_hit((struct monst *)0,
						&youmonst, obj, &dmg, 0);
			    losehp(dmg, xname(obj),
				obj_is_pname(obj) ? KILLED_BY : KILLED_BY_AN);
			}
			if (ship_object(obj, u.ux, u.uy, FALSE)) {
		    	    thrownobj = (struct obj*)0;
			    return;
			}
			dropy(obj);
		    }
		    thrownobj = (struct obj*)0;
		    return;
		}

		if (!IS_SOFT(levl[bhitpos.x][bhitpos.y].typ) &&
			breaktest(obj)) {
			int dmg, dsize = spiritDsize(), sx, sy;
			struct monst *msmon;
			sx = bhitpos.x;
			sy = bhitpos.y;
			if(objects[obj->otyp].oc_material == GLASS && u.specialSealsActive&SEAL_NUDZIARTH){
				if(obj->otyp == MIRROR){
					if(u.spiritPColdowns[PWR_MIRROR_SHATTER] < monstermoves && !u.uswallow && uwep && uwep->otyp == MIRROR && !(uwep->oartifact)){
						useup(uwep);
						explode(u.ux,u.uy,8/*Phys*/, d(5,dsize), TOOL_CLASS, EXPL_DARK);
						explode(sx,sy,8/*Phys*/, d(5,dsize), TOOL_CLASS, EXPL_DARK);
						
						while(sx != u.ux && sy != u.uy){
							sx -= u.dx;
							sy -= u.dy;
							if(!isok(sx,sy)) break; //shouldn't need this, but....
							else {
								msmon = m_at(sx, sy);
								/* reveal/unreveal invisible msmonsters before tmp_at() */
								if (msmon && !canspotmon(msmon) && cansee(sx,sy))
									map_invisible(sx, sy);
								else if (!msmon && glyph_is_invisible(levl[sx][sy].glyph)) {
									unmap_object(sx, sy);
									newsym(sx, sy);
								}
								if (msmon) {
									if (resists_magm(msmon)) {	/* match effect on player */
										shieldeff(msmon->mx, msmon->my);
									} else {
										dmg = d(5,dsize);
										if(hates_silver(msmon->data)){
											dmg += rnd(20);
											pline("The flying shards of mirror sear %s!", mon_nam(msmon));
										} else {
											pline("The flying shards of mirror hit %s.", mon_nam(msmon));
											u_teleport_mon(msmon, TRUE);
										}
										msmon->mhp -= dmg;
										if (msmon->mhp <= 0){
											xkilled(msmon, 1);
										}
									}
								}
							}
						}
						u.spiritPColdowns[PWR_MIRROR_SHATTER] = monstermoves + 25;
					} else explode(sx,sy,8/*Phys*/, d(rnd(5),dsize), TOOL_CLASS, EXPL_DARK);
				} else if(obj->oclass == WEAPON_CLASS && obj->otyp != CRYSTAL_SWORD) explode(sx,sy,8/*Phys*/, d(rnd(5),dsize), WEAPON_CLASS, EXPL_DARK);
			}
		    tmp_at(DISP_FLASH, obj_to_glyph(obj));
		    tmp_at(bhitpos.x, bhitpos.y);
		    delay_output();
		    tmp_at(DISP_END, 0);
		    breakmsg(obj, cansee(bhitpos.x, bhitpos.y));
		    breakobj(obj, bhitpos.x, bhitpos.y, TRUE, TRUE);
		    return;
		}
		if(flooreffects(obj,bhitpos.x,bhitpos.y,"fall")) return;
		obj_no_longer_held(obj);
		if (mon && mon->isshk && is_pick(obj)) {
		    if (cansee(bhitpos.x, bhitpos.y))
			pline("%s snatches up %s.",
			      Monnam(mon), the(xname(obj)));
		    if(*u.ushops)
			check_shop_obj(obj, bhitpos.x, bhitpos.y, FALSE);
		    (void) mpickobj(mon, obj);	/* may merge and free obj */
		    thrownobj = (struct obj*)0;
		    return;
		}
		(void) snuff_candle(obj);
		if (!mon && ship_object(obj, bhitpos.x, bhitpos.y, FALSE)) {
		    thrownobj = (struct obj*)0;
		    return;
		}
		thrownobj = (struct obj*)0;
		place_object(obj, bhitpos.x, bhitpos.y);
		if(*u.ushops && obj != uball)
		    check_shop_obj(obj, bhitpos.x, bhitpos.y, FALSE);

		stackobj(obj);
		if (obj == uball)
		    drop_ball(bhitpos.x, bhitpos.y);
		if (cansee(bhitpos.x, bhitpos.y))
		    newsym(bhitpos.x,bhitpos.y);
		if (obj_sheds_light(obj))
		    vision_full_recalc = 1;
		if (!IS_SOFT(levl[bhitpos.x][bhitpos.y].typ))
		    container_impact_dmg(obj);
	}
}

/* an object may hit a monster; various factors adjust the chance of hitting */
int
omon_adj(mon, obj, mon_notices)
struct monst *mon;
struct obj *obj;
boolean mon_notices;
{
	int tmp = 0;

	/* size of target affects the chance of hitting */
	tmp += (mon->data->msize - MZ_MEDIUM);		/* -2..+5 */
	/* sleeping target is more likely to be hit */
	if (mon->msleeping) {
	    tmp += 2;
	    if (mon_notices) mon->msleeping = 0;
	}
	/* ditto for immobilized target */
	if (!mon->mcanmove || !mon->data->mmove) {
	    tmp += 4;
	    if (mon_notices && mon->data->mmove && !rn2(10)) {
		mon->mcanmove = 1;
		mon->mfrozen = 0;
	    }
	}
	/* some objects are more likely to hit than others */
	switch (obj->otyp) {
	case HEAVY_IRON_BALL:
	    if (obj != uball) tmp += 2;
    break;
	case BOULDER:
	    tmp += 6;
    break;
	case STATUE:
	    if(is_boulder(obj)) tmp += 6;
    break;
	default:
	    if (obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
		    obj->oclass == GEM_CLASS)
		tmp += hitval(obj, mon);
    break;
	}
	return tmp;
}

/* thrown object misses target monster */
STATIC_OVL void
tmiss(obj, mon)
struct obj *obj;
struct monst *mon;
{
    const char *missile = mshot_xname(obj);

    /* If the target can't be seen or doesn't look like a valid target,
       avoid "the arrow misses it," or worse, "the arrows misses the mimic."
       An attentive player will still notice that this is different from
       an arrow just landing short of any target (no message in that case),
       so will realize that there is a valid target here anyway. */
    if (!canseemon(mon) || (mon->m_ap_type && mon->m_ap_type != M_AP_MONSTER))
	pline("%s %s.", The(missile), otense(obj, "miss"));
    else
	miss(missile, mon);
    if (!rn2(3)) wakeup(mon);
    return;
}


STATIC_OVL
boolean
quest_arti_hits_leader(obj,mon)
struct obj *obj;
struct monst *mon;
{
	if(obj->oartifact && is_quest_artifact(obj) && (mon->data == &mons[urole.ldrnum])) return TRUE;
	else if(Race_if(PM_ELF)){
		if((obj->oartifact == ART_PALANTIR_OF_WESTERNESSE || obj->oartifact == ART_BELTHRONDING) &&
			(mon->data == &mons[PM_CELEBORN] || mon->data == &mons[PM_GALADRIEL])
		) return TRUE;
	} else if(Race_if(PM_DROW)){
		if(((obj->oartifact == ART_SILVER_STARLIGHT || obj->oartifact == ART_WRATHFUL_SPIDER) &&
			(flags.initgend && mon->data == &mons[PM_ECLAVDRA])) || 
			((obj->oartifact == ART_DARKWEAVER_S_CLOAK || obj->oartifact == ART_SPIDERSILK) &&
			(!flags.initgend && mon->data == &mons[PM_ECLAVDRA])) ||
			
			((obj->oartifact == ART_TENTACLE_ROD || obj->oartifact == ART_CRESCENT_BLADE) &&
			(flags.initgend && mon->data == &mons[PM_SEYLL_AUZKOVYN])) ||
			((obj->oartifact == ART_TENTACLE_ROD || obj->oartifact == ART_WEBWEAVER_S_CROOK) &&
			(!flags.initgend && mon->data == &mons[PM_DARUTH_XAXOX]))
		) return TRUE;
	}
	return FALSE;
}
  
/*
 * Object thrown by player arrives at monster's location.
 * Return 1 if obj has disappeared or otherwise been taken care of,
 * 0 if caller must take care of it.
 */
int
thitmonst(mon, obj)
register struct monst *mon;
register struct obj   *obj;
{
	register int	tmp; /* Base chance to hit */
	register int	disttmp; /* distance modifier */
	int otyp = obj->otyp;
	boolean guaranteed_hit = (u.uswallow && mon == u.ustuck);
	int startX = u.ux, startY = u.uy;

	/* Differences from melee weapons:
	 *
	 * Dex still gives a bonus, but strength does not.
	 * Polymorphed players lacking attacks may still throw.
	 * There's a base -1 to hit.
	 * No bonuses for fleeing or stunned targets (they don't dodge
	 *    melee blows as readily, but dodging arrows is hard anyway).
	 * Not affected by traps, etc.
	 * Certain items which don't in themselves do damage ignore tmp.
	 * Distance and monster size affect chance to hit.
	 */
	tmp = -1 + Luck + find_mac(mon) + u.uhitinc + u.spiritAttk +
			maybe_polyd(youmonst.data->mlevel, u.ulevel);
	if (ACURR(A_DEX) < 4) tmp -= 3;
	else if (ACURR(A_DEX) < 6) tmp -= 2;
	else if (ACURR(A_DEX) < 8) tmp -= 1;
	else if (ACURR(A_DEX) >= 14) tmp += (ACURR(A_DEX) - 14);

	/* Modify to-hit depending on distance; but keep it sane.
	 * Polearms get a distance penalty even when wielded; it's
	 * hard to hit at a distance.
	 */
	disttmp = 3 - distmin(u.ux, u.uy, mon->mx, mon->my);
	if(disttmp < -4) disttmp = -4;
	tmp += disttmp;

	/* gloves are a hinderance to proper use of bows */
	if (uarmg && uwep && objects[uwep->otyp].oc_skill == P_BOW && ammo_and_launcher(obj, uwep)) {
	    switch (uarmg->otyp) {
	    case ORIHALCYON_GAUNTLETS:    /* metal */
	    case GAUNTLETS_OF_POWER:    /* metal */
		tmp -= 2;
		break;
	    case GAUNTLETS_OF_FUMBLING:
		tmp -= 3;
		break;
	    case LEATHER_GLOVES:
	    case GAUNTLETS_OF_DEXTERITY:
		break;
	    default:
		impossible("Unknown type of gloves (%d)", uarmg->otyp);
		break;
	    }
	}
	
	if(is_stabbing(obj) && is_ammo(obj) && (!uwep || ammo_and_launcher(obj, uwep)) && mon->data == &mons[PM_SMAUG]) tmp += 20;
	
	if(obj->otyp == BALL_OF_WEBBING) tmp -= 2000; //nasty hack :c
	
	tmp += omon_adj(mon, obj, TRUE);
	if (is_orc(mon->data) && maybe_polyd(is_elf(youmonst.data),
			Race_if(PM_ELF)))
	    tmp++;
	if (guaranteed_hit) {
	    tmp += 1000; /* Guaranteed hit */
	}

	if (obj->oclass == GEM_CLASS && is_unicorn(mon->data)) {
	    if (mon->mtame) {
		pline("%s catches and drops %s.", Monnam(mon), the(xname(obj)));
		return 0;
	    } else {
		pline("%s catches %s.", Monnam(mon), the(xname(obj)));
		return gem_accept(mon, obj);
	    }
	}

	/* don't make game unwinnable if naive player throws artifact
	   at leader.... */
	if (quest_arti_hits_leader(obj, mon)) {
	    /* not wakeup(), which angers non-tame monsters */
	    mon->msleeping = 0;
	    mon->mstrategy &= ~STRAT_WAITMASK;

	    if (mon->mcanmove) {
			pline("%s catches %s.", Monnam(mon), the(xname(obj)));
			if (mon->mpeaceful) {
				if(Race_if(PM_ELF) && mon->data == &mons[PM_CELEBORN]){
					boolean next2u = monnear(mon, u.ux, u.uy);
					if(obj->oartifact == ART_PALANTIR_OF_WESTERNESSE && 
						yn("If you prefer, we can use the Palantir to secure the city, and you can use my bow in your travels.") == 'y'
					){
						obfree(obj, (struct obj *)0);
						obj = mksobj(ELVEN_BOW, TRUE, FALSE);
						obj = oname(obj, artiname(ART_BELTHRONDING));
						obj->oerodeproof = TRUE;
						obj->blessed = TRUE;
						obj->cursed = FALSE;
						obj->spe = rn2(8);
						pline("%s %s %s to you.", Monnam(mon),
						  (next2u ? "hands" : "tosses"), the(xname(obj)));
						if (!next2u) sho_obj_return_to_u(obj, startX, startY);
						obj = addinv(obj);	/* into your inventory */
						(void) encumber_msg();
					} else {
						pline("%s %s %s back to you.", Monnam(mon),
						  (next2u ? "hands" : "tosses"), the(xname(obj)));
						if (!next2u) sho_obj_return_to_u(obj, startX, startY);
						obj = addinv(obj);	/* back into your inventory */
						(void) encumber_msg();
					}
				} else if(Race_if(PM_DROW)){
					boolean next2u = monnear(mon, u.ux, u.uy);
					if(quest_status.got_thanks) finish_quest(obj);
					if(obj->oartifact == ART_SILVER_STARLIGHT && quest_status.got_thanks && mon->data == &mons[PM_ECLAVDRA] && 
						yn("Do you wish to take the Wrathful Spider, instead of this?") == 'y'
					){
						obfree(obj, (struct obj *)0);
						obj = mksobj(DROVEN_CROSSBOW, TRUE, FALSE);
						obj = oname(obj, artiname(ART_WRATHFUL_SPIDER));
						obj->oerodeproof = TRUE;
						obj->blessed = TRUE;
						obj->cursed = FALSE;
						obj->spe = rn2(8);
						finish_quest(obj);
						pline("%s %s %s to you.", Monnam(mon),
						  (next2u ? "hands" : "tosses"), the(xname(obj)));
						if (!next2u) sho_obj_return_to_u(obj, startX, startY);
						obj = addinv(obj);	/* into your inventory */
						(void) encumber_msg();
					} else if(obj->oartifact == ART_TENTACLE_ROD && quest_status.got_thanks && mon->data == &mons[PM_SEYLL_AUZKOVYN] && 
						yn("Do you wish to take the Crescent Blade, instead of this?") == 'y'
					){
						obfree(obj, (struct obj *)0);
						obj = mksobj(SILVER_SABER, TRUE, FALSE);
						obj = oname(obj, artiname(ART_CRESCENT_BLADE));
						obj->oerodeproof = TRUE;
						obj->blessed = TRUE;
						obj->cursed = FALSE;
						obj->spe = rnd(7);
						finish_quest(obj);
						pline("%s %s %s to you.", Monnam(mon),
						  (next2u ? "hands" : "tosses"), the(xname(obj)));
						if (!next2u) sho_obj_return_to_u(obj, startX, startY);
						obj = addinv(obj);	/* into your inventory */
						(void) encumber_msg();
					} else if(obj->oartifact == ART_DARKWEAVER_S_CLOAK && quest_status.got_thanks && mon->data == &mons[PM_ECLAVDRA] && 
						yn("Do you wish to take Spidersilk, instead of this?") == 'y'
					){
						obfree(obj, (struct obj *)0);
						obj = mksobj(ELVEN_MITHRIL_COAT, TRUE, FALSE);
						obj = oname(obj, artiname(ART_SPIDERSILK));
						obj->oerodeproof = TRUE;
						obj->blessed = TRUE;
						obj->cursed = FALSE;
						obj->spe = rn2(8);
						finish_quest(obj);
						pline("%s %s %s to you.", Monnam(mon),
						  (next2u ? "hands" : "tosses"), the(xname(obj)));
						if (!next2u) sho_obj_return_to_u(obj, startX, startY);
						obj = addinv(obj);	/* into your inventory */
						(void) encumber_msg();
					} else if(obj->oartifact == ART_TENTACLE_ROD && quest_status.got_thanks && mon->data == &mons[PM_DARUTH_XAXOX] && 
						yn("Do you wish to take the Webweaver's Crook, instead of this?") == 'y'
					){
						obfree(obj, (struct obj *)0);
						obj = mksobj(FAUCHARD, TRUE, FALSE);
						obj = oname(obj, artiname(ART_WEBWEAVER_S_CROOK));
						obj->oerodeproof = TRUE;
						obj->blessed = TRUE;
						obj->cursed = FALSE;
						obj->spe = rnd(3)+rnd(4);
						finish_quest(obj);
						pline("%s %s %s to you.", Monnam(mon),
						  (next2u ? "hands" : "tosses"), the(xname(obj)));
						if (!next2u) sho_obj_return_to_u(obj, startX, startY);
						obj = addinv(obj);	/* into your inventory */
						(void) encumber_msg();
					} else {
						if(!quest_status.got_thanks) finish_quest(obj);	/* acknowledge quest completion */
						pline("%s %s %s back to you.", Monnam(mon),
						  (next2u ? "hands" : "tosses"), the(xname(obj)));
						if (!next2u) sho_obj_return_to_u(obj, startX, startY);
						obj = addinv(obj);	/* back into your inventory */
						(void) encumber_msg();
					}
				} else {
					boolean next2u = monnear(mon, u.ux, u.uy);
					
					finish_quest(obj);	/* acknowledge quest completion */
					pline("%s %s %s back to you.", Monnam(mon),
					  (next2u ? "hands" : "tosses"), the(xname(obj)));
					if (!next2u) sho_obj_return_to_u(obj, startX, startY);
					obj = addinv(obj);	/* back into your inventory */
					(void) encumber_msg();
				}
			} else {
				/* angry leader caught it and isn't returning it */
				(void) mpickobj(mon, obj);
			}
			return 1;		/* caller doesn't need to place it */
	    }
	    return(0);
	}
	else if (mon->mtame && mon->mcanmove &&
			(!is_animal(mon->data)) && (!mindless(mon->data)) &&
			!(uwep && ammo_and_launcher(obj, uwep))
	) {
		// if (could_use_item(mon, obj, TRUE)) {
			pline("%s catches %s.", Monnam(mon), the(xname(obj)));
			obj_extract_self(obj);
			(void) mpickobj(mon,obj);
			if (attacktype(mon->data, AT_WEAP) &&
				mon->weapon_check == NEED_WEAPON) {
				mon->weapon_check = NEED_HTH_WEAPON;
				(void) mon_wield_item(mon);
			}
			m_dowear(mon, FALSE);
			newsym(mon->mx, mon->my);
			return 1;
		// }
		// miss(xname(obj), mon);
	}
	else if (obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
		obj->oclass == GEM_CLASS) {
	    if (is_ammo(obj)) {
		if (!ammo_and_launcher(obj, uwep)) {
		    tmp -= 4;
		} else {
		    tmp += uwep->spe - greatest_erosion(uwep);
		    tmp += weapon_hit_bonus(uwep);
		    if (uwep->oartifact) tmp += spec_abon(uwep, mon);
		    /*
		     * Elves and Samurais are highly trained w/bows,
		     * especially their own special types of bow.
		     * Polymorphing won't make you a bow expert.
		     */
		    if ((Race_if(PM_ELF) || Role_if(PM_SAMURAI)) &&
				(!Upolyd || your_race(youmonst.data)) &&
				objects[uwep->otyp].oc_skill == P_BOW) {
			tmp++;
			if (Race_if(PM_ELF) && uwep->otyp == ELVEN_BOW)
			    tmp++;
			else if (Role_if(PM_SAMURAI) && uwep->otyp == YUMI)
			    tmp++;
		    }
		}
	    } else {
		if (otyp == BOOMERANG)		/* arbitrary */
		    tmp += 4;
		else if (throwing_weapon(obj))	/* meant to be thrown */
		    tmp += 2;
		else				/* not meant to be thrown */
		    tmp -= 2;
		/* we know we're dealing with a weapon or weptool handled
		   by WEAPON_SKILLS once ammo objects have been excluded */
		tmp += weapon_hit_bonus(obj);
	    }

	    if (tmp >= rnd(20)) {
		if (hmon(mon,obj,1)) {	/* mon still alive */
		    cutworm(mon, bhitpos.x, bhitpos.y, obj);
		}
		exercise(A_DEX, TRUE);
		/* projectiles other than magic stones
		 * sometimes disappear when thrown
		 * WAC - Spoon always disappears after doing damage
		 */
		if ((objects[otyp].oc_skill < P_NONE &&
			objects[otyp].oc_skill > -P_BOOMERANG &&
			obj->oclass != GEM_CLASS && 
			!objects[otyp].oc_magic) ||
			(obj->oartifact && obj->oartifact == ART_HOUCHOU)
		) {
		    /* we were breaking 2/3 of everything unconditionally.
		     * we still don't want anything to survive unconditionally,
		     * but we need ammo to stay around longer on average.
		     */
		    int broken, chance;
			if((uwep && ammo_and_launcher(obj, uwep) && 
				(uwep->oartifact==ART_HELLFIRE || uwep->oartifact==ART_BOW_OF_SKADI))
				|| obj->oartifact == ART_HOUCHOU
			){
				broken = 1;
			} else {
				chance = 3 + greatest_erosion(obj) - obj->spe;
				if (chance > 1)
				broken = rn2(chance);
				else
				broken = !rn2(4);
				if (obj->blessed && !rnl(4))
				broken = 0;
			}
		    if (broken) {
			if (*u.ushops)
			    check_shop_obj(obj, bhitpos.x,bhitpos.y, TRUE);
			obfree(obj, (struct obj *)0);
			return 1;
		    }
		}
		if(mon->data == &mons[PM_NAIAD]){
		  if((mon->mhp > 0 && mon->mhp < .5*mon->mhpmax
		   && rn2(3)) || mon->mhp-tmp <= 0){
			  pline("%s collapses into a puddle of water!", Monnam(mon));
			  passive_obj(mon, obj, (struct attack *)0);
			  if(mon->mhp-tmp > 0) killed(mon);
		  }
		} else passive_obj(mon, obj, (struct attack *)0);
	    } else {
		tmiss(obj, mon);
	    }

	} else if (otyp == HEAVY_IRON_BALL) {
	    exercise(A_STR, TRUE);
	    if (tmp >= rnd(20)) {
		int was_swallowed = guaranteed_hit;

		exercise(A_DEX, TRUE);
		if (!hmon(mon,obj,1)) {		/* mon killed */
		    if (was_swallowed && !u.uswallow && obj == uball)
			return 1;	/* already did placebc() */
		}
	    } else {
		tmiss(obj, mon);
	    }

	} else if (is_boulder(obj)) {
	    exercise(A_STR, TRUE);
	    if (tmp >= rnd(20)) {
		exercise(A_DEX, TRUE);
		(void) hmon(mon,obj,1);
	    } else {
		tmiss(obj, mon);
	    }

	} else if ((otyp == EGG || otyp == CREAM_PIE ||
		    otyp == BLINDING_VENOM || otyp == ACID_VENOM || otyp == BALL_OF_WEBBING) &&
		(guaranteed_hit || ACURR(A_DEX) > rnd(25))) {
	    (void) hmon(mon, obj, 1);
	    return 1;	/* hmon used it up */

	} else if (obj->oclass == POTION_CLASS &&
		(guaranteed_hit || ACURR(A_DEX) > rnd(25))) {
	    potionhit(mon, obj, TRUE);
	    return 1;

	} else if (befriend_with_obj(mon->data, obj) ||
		   (mon->mtame && dogfood(mon, obj) <= ACCFOOD)) {
	    if (tamedog(mon, obj))
		return 1;           	/* obj is gone */
	    else {
		/* not tmiss(), which angers non-tame monsters */
		miss(xname(obj), mon);
		mon->msleeping = 0;
		mon->mstrategy &= ~STRAT_WAITMASK;
	    }
	} else if (guaranteed_hit) {
	    /* this assumes that guaranteed_hit is due to swallowing */
	    wakeup(mon);
	    if (obj->otyp == CORPSE && touch_petrifies(&mons[obj->corpsenm])) {
		if (is_animal(u.ustuck->data)) {
			minstapetrify(u.ustuck, TRUE);
			/* Don't leave a cockatrice corpse available in a statue */
			if (!u.uswallow) {
				delobj(obj);
				return 1;
			}
	    	}
	    }
	    pline("%s into %s %s.",
		Tobjnam(obj, "vanish"), s_suffix(mon_nam(mon)),
		is_animal(u.ustuck->data) ? "entrails" : "currents");
	} else {
	    tmiss(obj, mon);
	}

	return 0;
}

STATIC_OVL int
gem_accept(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	char buf[BUFSZ];
	boolean is_buddy = sgn(mon->data->maligntyp) == sgn(u.ualign.type);
	boolean is_gem = objects[obj->otyp].oc_material == GEMSTONE;
	int ret = 0;
	static NEARDATA const char nogood[] = " is not interested in your junk.";
	static NEARDATA const char acceptgift[] = " accepts your gift.";
	static NEARDATA const char maybeluck[] = " hesitatingly";
	static NEARDATA const char noluck[] = " graciously";
	static NEARDATA const char addluck[] = " gratefully";

	Strcpy(buf,Monnam(mon));
	mon->mpeaceful = 1;
	mon->mavenge = 0;

	/* object properly identified */
	if(obj->dknown && objects[obj->otyp].oc_name_known) {
		if(is_gem) {
			if(is_buddy) {
				Strcat(buf,addluck);
				change_luck(5);
			} else {
				Strcat(buf,maybeluck);
				change_luck(rn2(7)-3);
			}
		} else {
			Strcat(buf,nogood);
			goto nopick;
		}
	/* making guesses */
	} else if(obj->onamelth || objects[obj->otyp].oc_uname) {
		if(is_gem) {
			if(is_buddy) {
				Strcat(buf,addluck);
				change_luck(2);
			} else {
				Strcat(buf,maybeluck);
				change_luck(rn2(3)-1);
			}
		} else {
			Strcat(buf,nogood);
			goto nopick;
		}
	/* value completely unknown to @ */
	} else {
		if(is_gem) {
			if(is_buddy) {
				Strcat(buf,addluck);
				change_luck(1);
			} else {
				Strcat(buf,maybeluck);
				change_luck(rn2(3)-1);
			}
		} else {
			Strcat(buf,noluck);
		}
	}
	Strcat(buf,acceptgift);
	if(*u.ushops) check_shop_obj(obj, mon->mx, mon->my, TRUE);
	(void) mpickobj(mon, obj);	/* may merge and free obj */
	ret = 1;

nopick:
	if(!Blind) pline("%s", buf);
	if (!tele_restrict(mon)) (void) rloc(mon, FALSE);
	return(ret);
}

/*
 * Comments about the restructuring of the old breaks() routine.
 *
 * There are now three distinct phases to object breaking:
 *     breaktest() - which makes the check/decision about whether the
 *                   object is going to break.
 *     breakmsg()  - which outputs a message about the breakage,
 *                   appropriate for that particular object. Should
 *                   only be called after a positve breaktest().
 *                   on the object and, if it going to be called,
 *                   it must be called before calling breakobj().
 *                   Calling breakmsg() is optional.
 *     breakobj()  - which actually does the breakage and the side-effects
 *                   of breaking that particular object. This should
 *                   only be called after a positive breaktest() on the
 *                   object.
 *
 * Each of the above routines is currently static to this source module.
 * There are two routines callable from outside this source module which
 * perform the routines above in the correct sequence.
 *
 *   hero_breaks() - called when an object is to be broken as a result
 *                   of something that the hero has done. (throwing it,
 *                   kicking it, etc.)
 *   breaks()      - called when an object is to be broken for some
 *                   reason other than the hero doing something to it.
 */

/*
 * The hero causes breakage of an object (throwing, dropping it, etc.)
 * Return 0 if the object didn't break, 1 if the object broke.
 */
int
hero_breaks(obj, x, y, from_invent)
struct obj *obj;
xchar x, y;		/* object location (ox, oy may not be right) */
boolean from_invent;	/* thrown or dropped by player; maybe on shop bill */
{
	boolean in_view = !Blind;
	if (!breaktest(obj)) return 0;
	breakmsg(obj, in_view);
	breakobj(obj, x, y, TRUE, from_invent);
	return 1;
}

/*
 * The object is going to break for a reason other than the hero doing
 * something to it.
 * Return 0 if the object doesn't break, 1 if the object broke.
 */
int
breaks(obj, x, y)
struct obj *obj;
xchar x, y;		/* object location (ox, oy may not be right) */
{
	boolean in_view = Blind ? FALSE : cansee(x, y);

	if (!breaktest(obj)) return 0;
	breakmsg(obj, in_view);
	breakobj(obj, x, y, FALSE, FALSE);
	return 1;
}

/*
 * Unconditionally break an object. Assumes all resistance checks
 * and break messages have been delivered prior to getting here.
 */
STATIC_OVL void
breakobj(obj, x, y, hero_caused, from_invent)
struct obj *obj;
xchar x, y;		/* object location (ox, oy may not be right) */
boolean hero_caused;	/* is this the hero's fault? */
boolean from_invent;
{
	switch (obj->oclass == POTION_CLASS ? POT_WATER : obj->otyp) {
		case MIRROR:
			if (hero_caused){
			    if((u.specialSealsActive&SEAL_NUDZIARTH)) change_luck(+2);
				else change_luck(-2);
			}
			break;
		case POT_WATER:		/* really, all potions */
			if (obj->otyp == POT_OIL && obj->lamplit) {
			    splatter_burning_oil(x,y);
			} else if (distu(x,y) <= 2) {
			    if (!breathless(youmonst.data) || haseyes(youmonst.data)) {
				if (obj->otyp != POT_WATER) {
					if (!breathless(youmonst.data))
			    		     /* [what about "familiar odor" when known?] */
					    You("smell a peculiar odor...");
					else {
					    int numeyes = eyecount(youmonst.data);
					    Your("%s water%s.",
						 (numeyes == 1) ? body_part(EYE) :
							makeplural(body_part(EYE)),
						 (numeyes == 1) ? "s" : "");
					}
				}
				potionbreathe(obj);
			    }
			}
			/* monster breathing isn't handled... [yet?] */
			break;
		case EGG:
			/* breaking your own eggs is bad luck */
			if (hero_caused && obj->spe && obj->corpsenm >= LOW_PM)
			    change_luck((schar) -min(obj->quan, 5L));
			break;
		case BALL_OF_WEBBING:
			dowebgush(x,y, obj->ovar1 ? obj->ovar1 : 2);
		break;
	}
	if (hero_caused) {
	    if (from_invent) {
		if (*u.ushops)
			check_shop_obj(obj, x, y, TRUE);
	    } else if (!obj->no_charge && costly_spot(x, y)) {
		/* it is assumed that the obj is a floor-object */
		char *o_shop = in_rooms(x, y, SHOPBASE);
		struct monst *shkp = shop_keeper(*o_shop);

		if (shkp) {		/* (implies *o_shop != '\0') */
		    static NEARDATA long lastmovetime = 0L;
		    static NEARDATA boolean peaceful_shk = FALSE;
		    /*  We want to base shk actions on her peacefulness
			at start of this turn, so that "simultaneous"
			multiple breakage isn't drastically worse than
			single breakage.  (ought to be done via ESHK)  */
		    if (moves != lastmovetime)
			peaceful_shk = shkp->mpeaceful;
		    if (stolen_value(obj, x, y, peaceful_shk, FALSE) > 0L &&
			(*o_shop != u.ushops[0] || !inside_shop(u.ux, u.uy)) &&
			moves != lastmovetime) make_angry_shk(shkp, x, y);
		    lastmovetime = moves;
		}
	    }
	}
	delobj(obj);
}

/*
 * Check to see if obj is going to break, but don't actually break it.
 * Return 0 if the object isn't going to break, 1 if it is.
 */
boolean
breaktest(obj)
struct obj *obj;
{
	if (obj_resists(obj, 0, 100)) return 0;
	if (objects[obj->otyp].oc_material == GLASS &&
		obj->oclass != GEM_CLASS)
	    return 1;
	switch (obj->oclass == POTION_CLASS ? POT_WATER : obj->otyp) {
#ifdef TOURIST
		case EXPENSIVE_CAMERA:
#endif
		case POT_WATER:		/* really, all potions */
		case EGG:
		case CREAM_PIE:
		case MELON:
		case ACID_VENOM:
		case BLINDING_VENOM:
		case BALL_OF_WEBBING:
			return 1;
		default:
			return 0;
	}
}

STATIC_OVL void
breakmsg(obj, in_view)
struct obj *obj;
boolean in_view;
{
	const char *to_pieces;

	to_pieces = "";
	switch (obj->oclass == POTION_CLASS ? POT_WATER : obj->otyp) {
		default: /* glass or crystal wand */
		    if (obj->oclass != WAND_CLASS)
			impossible("breaking odd object?");
		case CRYSTAL_HELM:
		case CRYSTAL_PLATE_MAIL:
		case CRYSTAL_SWORD:
		case CRYSTAL_SHIELD:
		case CRYSTAL_BOOTS:
		case DROVEN_BOLT:
		case DROVEN_DAGGER:
		case DROVEN_SHORT_SWORD:
		case DROVEN_LANCE:
		case DROVEN_GREATSWORD:
		case DROVEN_SPEAR:
		case LENSES:
		case R_LYEHIAN_FACEPLATE:
		case MIRROR:
		case CRYSTAL_BALL:
		case SUBETHAIC_COMPONENT:
#ifdef TOURIST
		case EXPENSIVE_CAMERA:
#endif
			to_pieces = " into a thousand pieces";
			/*FALLTHRU*/
		case POT_WATER:		/* really, all potions */
			if (!in_view)
			    You_hear("%s shatter!", something);
			else
			    pline("%s shatter%s%s!", Doname2(obj),
				(obj->quan==1) ? "s" : "", to_pieces);
			break;
		case EGG:
		case MELON:
		case BALL_OF_WEBBING:
			pline("Splat!");
		break;
		case CREAM_PIE:
			if (in_view) pline("What a mess!");
		break;
		case ACID_VENOM:
		case BLINDING_VENOM:
			pline("Splash!");
		break;
	}
}

STATIC_OVL int
throw_gold(obj)
struct obj *obj;
{
	int range, odx, ody;
#ifndef GOLDOBJ
	long zorks = obj->quan;
#endif
	register struct monst *mon;

	if(!u.dx && !u.dy && !u.dz) {
#ifndef GOLDOBJ
		u.ugold += obj->quan;
		flags.botl = 1;
		dealloc_obj(obj);
#endif
		You("cannot throw gold at yourself.");
		return(0);
	}
#ifdef GOLDOBJ
        freeinv(obj);
#endif
	if(u.uswallow) {
		pline(is_animal(u.ustuck->data) ?
			"%s in the %s's entrails." : "%s into %s.",
#ifndef GOLDOBJ
			"The gold disappears", mon_nam(u.ustuck));
		u.ustuck->mgold += zorks;
		dealloc_obj(obj);
#else
			"The money disappears", mon_nam(u.ustuck));
		add_to_minv(u.ustuck, obj);
#endif
		return(1);
	}

	if(u.dz) {
		if (u.dz < 0 && !Is_airlevel(&u.uz) &&
					!Underwater && !Is_waterlevel(&u.uz)) {
	pline_The("gold hits the %s, then falls back on top of your %s.",
		    ceiling(u.ux,u.uy), body_part(HEAD));
		    /* some self damage? */
		    if(uarmh) pline("Fortunately, you are wearing a helmet!");
		}
		bhitpos.x = u.ux;
		bhitpos.y = u.uy;
	} else {
		/* consistent with range for normal objects */
		range = (int)((ACURRSTR)/2 - obj->owt/40);

		/* see if the gold has a place to move into */
		odx = u.ux + u.dx;
		ody = u.uy + u.dy;
		if(!ZAP_POS(levl[odx][ody].typ) || closed_door(odx, ody)) {
			bhitpos.x = u.ux;
			bhitpos.y = u.uy;
		} else {
			mon = bhit(u.dx, u.dy, range, THROWN_WEAPON,
				   (int FDECL((*),(MONST_P,OBJ_P)))0,
				   (int FDECL((*),(OBJ_P,OBJ_P)))0,
				   obj, NULL);
			if(mon) {
			    if (ghitm(mon, obj))	/* was it caught? */
				return 1;
			} else {
			    if(ship_object(obj, bhitpos.x, bhitpos.y, FALSE))
				return 1;
			}
		}
	}

	if(flooreffects(obj,bhitpos.x,bhitpos.y,"fall")) return(1);
	if(u.dz > 0)
		pline_The("gold hits the %s.", surface(bhitpos.x,bhitpos.y));
	place_object(obj,bhitpos.x,bhitpos.y);
	if(*u.ushops) sellobj(obj, bhitpos.x, bhitpos.y);
	stackobj(obj);
	newsym(bhitpos.x,bhitpos.y);
	return(1);
}

/*dothrow.c*/
