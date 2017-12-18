/*	SCCS Id: @(#)do.c	3.4	2003/12/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* Contains code for 'd', 'D' (drop), '>', '<' (up, down) */

#include "hack.h"
#include "lev.h"

#ifdef SINKS
# ifdef OVLB
STATIC_DCL void FDECL(trycall, (struct obj *));
# endif /* OVLB */
STATIC_DCL void FDECL(dosinkring, (struct obj *));
#endif /* SINKS */

STATIC_PTR int FDECL(drop, (struct obj *));
STATIC_PTR int NDECL(wipeoff);

#ifdef OVL0
STATIC_DCL int FDECL(menu_drop, (int));
#endif
#ifdef OVL2
STATIC_DCL int NDECL(currentlevel_rewrite);
STATIC_DCL void NDECL(final_level);
/* static boolean FDECL(badspot, (XCHAR_P,XCHAR_P)); */
#endif

#ifdef OVLB

static NEARDATA const char drop_types[] =
	{ ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, 0 };

/* 'd' command: drop one inventory item */
int
dodrop()
{
#ifndef GOLDOBJ
	int result, i = (invent || u.ugold) ? 0 : (SIZE(drop_types) - 1);
#else
	int result, i = (invent) ? 0 : (SIZE(drop_types) - 1);
#endif

	if (*u.ushops) sellobj_state(SELL_DELIBERATE);
	result = drop(getobj(&drop_types[i], "drop"));
	if (*u.ushops) sellobj_state(SELL_NORMAL);
	reset_occupations();

	return result;
}

#endif /* OVLB */
#ifdef OVL0

/* Called when a boulder is dropped, thrown, or pushed.  If it ends up
 * in a pool, it either fills the pool up or sinks away.  In either case,
 * it's gone for good...  If the destination is not a pool, returns FALSE.
 */
boolean
boulder_hits_pool(otmp, rx, ry, pushing)
struct obj *otmp;
register int rx, ry;
boolean pushing;
{
	if (!otmp || !is_boulder(otmp))
	    impossible("Not a boulder?");
	else if (!Is_waterlevel(&u.uz) && (is_pool(rx,ry, FALSE) || is_lava(rx,ry))) {
	    boolean lava = is_lava(rx,ry), cubewater = is_3dwater(rx, ry), fills_up;
	    const char *what = waterbody_name(rx,ry);
	    schar ltyp = levl[rx][ry].typ;
	    int chance = rn2(10);		/* water: 90%; lava: 10% */
	    fills_up = cubewater ? FALSE : lava ? chance == 0 : chance != 0;

	    if (fills_up) {
		struct trap *ttmp = t_at(rx, ry);

		if (ltyp == DRAWBRIDGE_UP) {
		    levl[rx][ry].drawbridgemask &= ~DB_UNDER; /* clear lava */
		    levl[rx][ry].drawbridgemask |= DB_FLOOR;
		} else
		    levl[rx][ry].typ = ROOM;

		if (ttmp) (void) delfloortrap(ttmp);
		bury_objs(rx, ry);
		
		newsym(rx,ry);
		if (pushing) {
		    You("push %s into the %s.", the(xname(otmp)), what);
		    if (flags.verbose && !Blind)
			pline("Now you can cross it!");
		    /* no splashing in this case */
		}
	    }
	    if (!fills_up || !pushing) {	/* splashing occurs */
		if (!u.uinwater) {
		    if (pushing ? !Blind : cansee(rx,ry)) {
			There("is a large splash as %s %s the %s.",
			      the(xname(otmp)), fills_up? "fills":"falls into",
			      what);
		    } else if (flags.soundok)
			You_hear("a%s splash.", lava ? " sizzling" : "");
		    wake_nearto_noisy(rx, ry, 40);
		}

		if (fills_up && u.uinwater && distu(rx,ry) == 0) {
		    u.uinwater = 0;
		    u.usubwater = 0;
		    docrt();
		    vision_full_recalc = 1;
		    You("find yourself on dry land again!");
		} else if (lava && distu(rx,ry) <= 2) {
		    You("are hit by molten lava%c",
			Fire_resistance ? '.' : '!');
			burn_away_slime();
		    losehp(d((Fire_resistance ? 1 : 3), 6),
			   "molten lava", KILLED_BY);
		} else if (!fills_up && flags.verbose &&
			   (pushing ? !Blind : cansee(rx,ry)))
		    pline("It sinks without a trace!");
	    }

	    /* boulder is now gone */
	    if (pushing) delobj(otmp);
	    else obfree(otmp, (struct obj *)0);
	    return TRUE;
	}
	return FALSE;
}

/* Used for objects which sometimes do special things when dropped; must be
 * called with the object not in any chain.  Returns TRUE if the object goes
 * away.
 */
boolean
flooreffects(obj,x,y,verb)
struct obj *obj;
int x,y;
const char *verb;
{
	struct trap *t;
	struct monst *mtmp;

	if (obj->where != OBJ_FREE)
	    panic("flooreffects: obj not free");

	/* make sure things like water_damage() have no pointers to follow */
	obj->nobj = obj->nexthere = (struct obj *)0;

	if (is_boulder(obj) && boulder_hits_pool(obj, x, y, FALSE))
		return TRUE;
	else if (is_boulder(obj) && (t = t_at(x,y)) != 0 &&
		 (t->ttyp==PIT || t->ttyp==SPIKED_PIT
			|| t->ttyp==TRAPDOOR || t->ttyp==HOLE)) {
		if (((mtmp = m_at(x, y)) && mtmp->mtrapped) ||
			(u.utrap && u.ux == x && u.uy == y)) {
		    if (*verb)
			pline_The("%s %s into the pit%s.",
				xname(obj),
				vtense((const char *)0, verb),
				(mtmp) ? "" : " with you");
		    if (mtmp) {
			if (!passes_walls(mtmp->data) &&
				!throws_rocks(mtmp->data)) {
			    if (hmon(mtmp, obj, TRUE) && !is_whirly(mtmp->data))
				return FALSE;	/* still alive */
			}
			mtmp->mtrapped = 0;
		    } else {
			if (!Passes_walls && !throws_rocks(youracedata) && !(u.sealsActive&SEAL_YMIR)) {
			    losehp(rnd(15), "squished under a heavy object",
				   NO_KILLER_PREFIX);
			    return FALSE;	/* player remains trapped */
			} else u.utrap = 0;
		    }
		}
		if (*verb) {
			if (Blind) {
				if ((x == u.ux) && (y == u.uy))
					You_hear("a CRASH! beneath you.");
				else
					You_hear("the %s %s.", xname(obj), verb);
			} else if (cansee(x, y)) {
				pline_The("%s %s%s.",
					xname(obj),
				    t->tseen ? "" : "triggers and ",
				    t->ttyp == TRAPDOOR ? "plugs a trap door" :
				    t->ttyp == HOLE ? "plugs a hole" :
				    "fills a pit");
			}
		}
		deltrap(t);
		bury_objs(x, y); //Crate handling: Bury everything here (inc boulder item) then free the boulder after
		if(obj->otyp == MASSIVE_STONE_CRATE){
			struct obj *item;
			if(Blind) pline("Click!");
			else pline("The crate pops open as it lands.");
			/* drop any objects contained inside the crate */
			while ((item = obj->cobj) != 0) {
				obj_extract_self(item);
				place_object(item, x, y);
			}
		}
		obfree(obj, (struct obj *)0);
		newsym(x,y);
		return TRUE;
	} else if (is_lava(x, y)) {
		return fire_damage(obj, FALSE, FALSE, x, y);
	} else if (is_pool(x, y, TRUE)) {
		/* Reasonably bulky objects (arbitrary) splash when dropped.
		 * If you're floating above the water even small things make noise.
		 * Stuff dropped near fountains always misses 
		 */
		if ((Blind || (Levitation || Flying)) && flags.soundok &&
		    ((x == u.ux) && (y == u.uy))) {
		    if (!Underwater) {
			if (weight(obj) > 9) {
				pline("Splash!");
		        } else if (Levitation || Flying) {
				pline("Plop!");
		        }
		    }
		    //map_background(x, y, 0);
		    newsym(x, y);
		}
		return water_damage(obj, FALSE, FALSE, FALSE, (struct monst *) 0);
	} else if (u.ux == x && u.uy == y &&
		(!u.utrap || u.utraptype != TT_PIT) &&
		(t = t_at(x,y)) != 0 && t->tseen &&
			(t->ttyp==PIT || t->ttyp==SPIKED_PIT)) {
		/* you escaped a pit and are standing on the precipice */
		if (Blind && flags.soundok)
			You_hear("%s %s downwards.",
				The(xname(obj)), otense(obj, "tumble"));
		else
			pline("%s %s into %s pit.",
				The(xname(obj)), otense(obj, "tumble"),
				the_your[t->madeby_u]);
	}
	if (is_lightsaber(obj) && litsaber(obj)) {
		if (cansee(x, y)) You("see %s deactivate.", an(xname(obj)));
		lightsaber_deactivate(obj, TRUE);
	}
	if (obj->oartifact == ART_HOLY_MOONLIGHT_SWORD && obj->lamplit) {
		if (cansee(x, y)) You("see %s go out.", an(xname(obj)));
		end_burn(obj, TRUE);
	}
	return FALSE;
}

#endif /* OVL0 */
#ifdef OVLB

void
doaltarobj(obj)  /* obj is an object dropped on an altar */
	register struct obj *obj;
{
	if (Blind)
		return;

	/* KMH, conduct */
	u.uconduct.gnostic++;

	if ((obj->blessed || obj->cursed) && obj->oclass != COIN_CLASS) {
		There("is %s flash as %s %s the altar.",
			an(hcolor(obj->blessed ? NH_AMBER : NH_BLACK)),
			doname(obj), otense(obj, "hit"));
		if (!Hallucination) obj->bknown = 1;
	} else {
		pline("%s %s on the altar.", Doname2(obj),
			otense(obj, "land"));
		obj->bknown = 1;
	}
}

#ifdef SINKS
STATIC_OVL
void
trycall(obj)
register struct obj *obj;
{
	if(!objects[obj->otyp].oc_name_known &&
	   !objects[obj->otyp].oc_uname)
	   docall(obj);
}

STATIC_OVL
void
dosinkring(obj)  /* obj is a ring being dropped over a kitchen sink */
register struct obj *obj;
{
	register struct obj *otmp,*otmp2;
	register boolean ideed = TRUE;

	You("drop %s down the drain.", doname(obj));
	obj->in_use = TRUE;	/* block free identification via interrupt */
	switch(obj->otyp) {	/* effects that can be noticed without eyes */
	    case RIN_SEARCHING:
		You("thought your %s got lost in the sink, but there it is!",
			xname(obj));
		goto giveback;
	    case RIN_SLOW_DIGESTION:
		pline_The("ring is regurgitated!");
giveback:
		obj->in_use = FALSE;
		dropx(obj);
		trycall(obj);
		return;
	    case RIN_LEVITATION:
		pline_The("sink quivers upward for a moment.");
		break;
	    case RIN_POISON_RESISTANCE:
		You("smell rotten %s.", makeplural(fruitname(FALSE)));
		break;
	    case RIN_AGGRAVATE_MONSTER:
		pline("Several flies buzz angrily around the sink.");
		break;
	    case RIN_SHOCK_RESISTANCE:
		pline("Static electricity surrounds the sink.");
		break;
	    case RIN_CONFLICT:
		You_hear("loud noises coming from the drain.");
		break;
	    case RIN_ALACRITY:
		pline_The("water flow seems faster now.");
		break;
	    case RIN_SUSTAIN_ABILITY:	/* KMH */
		pline_The("water flow seems fixed.");
		break;
	    case RIN_GAIN_STRENGTH:
		pline_The("water flow seems %ser now.",
			(obj->spe<0) ? "weak" : "strong");
		break;
	    case RIN_GAIN_CONSTITUTION:
		pline_The("water flow seems %ser now.",
			(obj->spe<0) ? "less" : "great");
		break;
	    case RIN_INCREASE_ACCURACY:	/* KMH */
		pline_The("water flow %s the drain.",
			(obj->spe<0) ? "misses" : "hits");
		break;
	    case RIN_INCREASE_DAMAGE:
		pline_The("water's force seems %ser now.",
			(obj->spe<0) ? "small" : "great");
		break;
	    case RIN_HUNGER:
		ideed = FALSE;
		for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp2) {
		    otmp2 = otmp->nexthere;
		    if (otmp != uball && otmp != uchain &&
			    !obj_resists(otmp, 1, 99)) {
			if (!Blind) {
			    pline("Suddenly, %s %s from the sink!",
				  doname(otmp), otense(otmp, "vanish"));
			    ideed = TRUE;
			}
			delobj(otmp);
		    }
		}
		break;
	    case MEAT_RING:
		/* Not the same as aggravate monster; besides, it's obvious. */
		pline("Several flies buzz around the sink.");
		break;
	    default:
		ideed = FALSE;
		break;
	}
	if(!Blind && !ideed && obj->otyp != RIN_HUNGER) {
	    ideed = TRUE;
	    switch(obj->otyp) {		/* effects that need eyes */
		case RIN_ADORNMENT:
		    pline_The("faucets flash brightly for a moment.");
		    break;
		case RIN_REGENERATION:
		    pline_The("sink looks as good as new.");
		    break;
		case RIN_INVISIBILITY:
		    You("don't see anything happen to the sink.");
		    break;
		case RIN_FREE_ACTION:
		    You("see the ring slide right down the drain!");
		    break;
		case RIN_SEE_INVISIBLE:
		    You("see some air in the sink.");
		    break;
		case RIN_STEALTH:
		pline_The("sink seems to blend into the floor for a moment.");
		    break;
		case RIN_FIRE_RESISTANCE:
		pline_The("hot water faucet flashes brightly for a moment.");
		    break;
		case RIN_COLD_RESISTANCE:
		pline_The("cold water faucet flashes brightly for a moment.");
		    break;
		case RIN_PROTECTION_FROM_SHAPE_CHAN:
		    pline_The("sink looks nothing like a fountain.");
		    break;
		case RIN_PROTECTION:
		    pline_The("sink glows %s for a moment.",
			    hcolor((obj->spe<0) ? NH_BLACK : NH_SILVER));
		    break;
		case RIN_WARNING:
		    pline_The("sink glows %s for a moment.", hcolor(NH_WHITE));
		    break;
		case RIN_TELEPORTATION:
		    pline_The("sink momentarily vanishes.");
		    break;
		case RIN_TELEPORT_CONTROL:
	    pline_The("sink looks like it is being beamed aboard somewhere.");
		    break;
		case RIN_POLYMORPH:
		    pline_The("sink momentarily looks like a fountain.");
		    break;
		case RIN_POLYMORPH_CONTROL:
	pline_The("sink momentarily looks like a regularly erupting geyser.");
		    break;
	    }
	}
	if(ideed)
	    trycall(obj);
	else
	    You_hear("the ring bouncing down the drainpipe.");
	if (!rn2(20)) {
		pline_The("sink backs up, leaving %s.", doname(obj));
		obj->in_use = FALSE;
		dropx(obj);
	} else
		useup(obj);
}
#endif

#endif /* OVLB */
#ifdef OVL0

/* some common tests when trying to drop or throw items */
boolean
canletgo(obj,word)
register struct obj *obj;
register const char *word;
{
	if(obj->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)){
		if (*word)
			Norep("You cannot %s %s you are wearing.",word,
				something);
		return(FALSE);
	}
	if (obj->otyp == LOADSTONE && obj->cursed) {
		/* getobj() kludge sets corpsenm to user's specified count
		   when refusing to split a stack of cursed loadstones */
		if (*word) {
			/* getobj() ignores a count for throwing since that is
			   implicitly forced to be 1; replicate its kludge... */
			if (!strcmp(word, "throw") && obj->quan > 1L)
			    obj->corpsenm = 1;
			pline("For some reason, you cannot %s%s the stone%s!",
			      word, obj->corpsenm ? " any of" : "",
			      plur(obj->quan));
		}
		obj->corpsenm = 0;		/* reset */
		obj->bknown = 1;
		return(FALSE);
	}
	if (obj->otyp == LEASH && obj->leashmon != 0) {
		if (*word)
			pline_The("leash is tied around your %s.",
					body_part(HAND));
		return(FALSE);
	}
#ifdef STEED
	if (obj->owornmask & W_SADDLE) {
		if (*word)
			You("cannot %s %s you are sitting on.", word,
				something);
		return (FALSE);
	}
#endif
	return(TRUE);
}

STATIC_PTR
int
drop(obj)
register struct obj *obj;
{
	if(!obj) return(0);
	if(!canletgo(obj,"drop"))
		return(0);
	if(obj == uwep) {
		if(welded(uwep)) {
			weldmsg(obj);
			return(0);
		}
		setuwep((struct obj *)0);
	}
	if(obj == uquiver) {
		setuqwep((struct obj *)0);
	}
	if (obj == uswapwep) {
		setuswapwep((struct obj *)0);
	}

	if (u.uswallow) {
		/* barrier between you and the floor */
		if(flags.verbose)
		{
			char buf[BUFSZ];

			/* doname can call s_suffix, reusing its buffer */
			Strcpy(buf, s_suffix(mon_nam(u.ustuck)));
			You("drop %s into %s %s.", doname(obj), buf,
				mbodypart(u.ustuck, STOMACH));
		}
	} else {
#ifdef SINKS
	    if((obj->oclass == RING_CLASS || obj->otyp == MEAT_RING) &&
			IS_SINK(levl[u.ux][u.uy].typ)) {
		dosinkring(obj);
		return(1);
	    }
#endif
	    if (!can_reach_floor()) {
		if(flags.verbose) You("drop %s.", doname(obj));
#ifndef GOLDOBJ
		if (obj->oclass != COIN_CLASS || obj == invent) freeinv(obj);
#else
		/* Ensure update when we drop gold objects */
		if (obj->oclass == COIN_CLASS) flags.botl = 1;
		freeinv(obj);
#endif
		hitfloor(obj);
		return(1);
	    }
	    if (!IS_ALTAR(levl[u.ux][u.uy].typ) && flags.verbose)
		You("drop %s.", doname(obj));
	}
	dropx(obj);
	return(1);
}

/* Called in several places - may produce output */
/* eg ship_object() and dropy() -> sellobj() both produce output */
void
dropx(obj)
register struct obj *obj;
{
#ifndef GOLDOBJ
	if (obj->oclass != COIN_CLASS || obj == invent) freeinv(obj);
#else
        /* Ensure update when we drop gold objects */
        if (obj->oclass == COIN_CLASS) flags.botl = 1;
        freeinv(obj);
#endif
	if (!u.uswallow) {
	    if (ship_object(obj, u.ux, u.uy, FALSE)) return;
	    if (IS_ALTAR(levl[u.ux][u.uy].typ))
		doaltarobj(obj); /* set bknown */
	}
	dropy(obj);
}

void
dropy(obj)
register struct obj *obj;
{
	if (obj == uwep) setuwep((struct obj *)0);
	if (obj == uquiver) setuqwep((struct obj *)0);
	if (obj == uswapwep) setuswapwep((struct obj *)0);

	if (!u.uswallow && flooreffects(obj,u.ux,u.uy,"drop")) return;
	/* uswallow check done by GAN 01/29/87 */
	if(u.uswallow) {
	    boolean could_petrify = FALSE;
	    boolean could_poly = FALSE;
	    boolean could_slime = FALSE;
	    boolean could_grow = FALSE;
	    boolean could_heal = FALSE;

	    if (obj != uball) {		/* mon doesn't pick up ball */
		if (obj->otyp == CORPSE) {
		    could_petrify = touch_petrifies(&mons[obj->corpsenm]);
		    could_poly = polyfodder(obj) && !resists_poly(u.ustuck->data);
		    could_slime = (obj->corpsenm == PM_GREEN_SLIME || obj->corpsenm == PM_FLUX_SLIME) &&  !resists_poly(u.ustuck->data);
		    could_grow = (obj->corpsenm == PM_WRAITH);
		    could_heal = (obj->corpsenm == PM_NURSE);
		}
		(void) mpickobj(u.ustuck,obj);
		if (is_animal(u.ustuck->data)) {
		    if (could_poly || could_slime) {
			(void) newcham(u.ustuck,
				       could_poly ? (struct permonst *)0 :
				       &mons[PM_GREEN_SLIME],
				       FALSE, could_slime);
			delobj(obj);	/* corpse is digested */
		    } else if (could_petrify) {
			minstapetrify(u.ustuck, TRUE);
			/* Don't leave a cockatrice corpse in a statue */
			if (!u.uswallow) delobj(obj);
		    } else if (could_grow) {
			(void) grow_up(u.ustuck, (struct monst *)0);
			delobj(obj);	/* corpse is digested */
		    } else if (could_heal) {
			u.ustuck->mhp = u.ustuck->mhpmax;
			delobj(obj);	/* corpse is digested */
		    }
		}
	    }
	} else  {
	    place_object(obj, u.ux, u.uy);
	    if (obj == uball)
		drop_ball(u.ux,u.uy);
	    else
		sellobj(obj, u.ux, u.uy);
	    stackobj(obj);
	    if(Blind && Levitation)
		map_object(obj, 0);
	    newsym(u.ux,u.uy);	/* remap location under self */
	}
}

/* things that must change when not held; recurse into containers.
   Called for both player and monsters */
void
obj_no_longer_held(obj)
struct obj *obj;
{
	if (!obj) {
	    return;
	} else if ((Is_container(obj) || obj->otyp == STATUE) && obj->cobj) {
	    struct obj *contents;
	    for(contents=obj->cobj; contents; contents=contents->nobj)
		obj_no_longer_held(contents);
	}
	switch(obj->otyp) {
	case CRYSKNIFE:
	    /* KMH -- Fixed crysknives have only 10% chance of reverting */
	    /* only changes when not held by player or monster */
	    if (!obj->oerodeproof || !rn2(10)) {
		obj->otyp = WORM_TOOTH;
		obj->oerodeproof = 0;
	    }
	    break;
	}
}

/* 'D' command: drop several things */
int
doddrop()
{
	int result = 0;

	add_valid_menu_class(0); /* clear any classes already there */
	if (*u.ushops) sellobj_state(SELL_DELIBERATE);
	if (flags.menu_style != MENU_TRADITIONAL ||
		(result = ggetobj("drop", drop, 0, FALSE, (unsigned *)0)) < -1)
	    result = menu_drop(result);
	if (*u.ushops) sellobj_state(SELL_NORMAL);
	reset_occupations();

	return result;
}

/* Drop things from the hero's inventory, using a menu. */
STATIC_OVL int
menu_drop(retry)
int retry;
{
    int n, i, n_dropped = 0;
    long cnt;
    struct obj *otmp, *otmp2;
#ifndef GOLDOBJ
    struct obj *u_gold = 0;
#endif
    menu_item *pick_list;
    boolean all_categories = TRUE;
    boolean drop_everything = FALSE;

#ifndef GOLDOBJ
    if (u.ugold) {
	/* Hack: gold is not in the inventory, so make a gold object
	   and put it at the head of the inventory list. */
	u_gold = mkgoldobj(u.ugold);	/* removes from u.ugold */
	u_gold->in_use = TRUE;
	u.ugold = u_gold->quan;		/* put the gold back */
	assigninvlet(u_gold);		/* might end up as NOINVSYM */
	u_gold->nobj = invent;
	invent = u_gold;
    }
#endif
    if (retry) {
	all_categories = (retry == -2);
    } else if (flags.menu_style == MENU_FULL) {
	all_categories = FALSE;
	n = query_category("Drop what type of items?",
			invent,
			UNPAID_TYPES | ALL_TYPES | CHOOSE_ALL |
			BUC_BLESSED | BUC_CURSED | BUC_UNCURSED | BUC_UNKNOWN,
			&pick_list, PICK_ANY);
	if (!n) goto drop_done;
	for (i = 0; i < n; i++) {
	    if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
		all_categories = TRUE;
	    else if (pick_list[i].item.a_int == 'A')
		drop_everything = TRUE;
	    else
		add_valid_menu_class(pick_list[i].item.a_int);
	}
	free((genericptr_t) pick_list);
    } else if (flags.menu_style == MENU_COMBINATION) {
	unsigned ggoresults = 0;
	all_categories = FALSE;
	/* Gather valid classes via traditional NetHack method */
	i = ggetobj("drop", drop, 0, TRUE, &ggoresults);
	if (i == -2) all_categories = TRUE;
	if (ggoresults & ALL_FINISHED) {
		n_dropped = i;
		goto drop_done;
	}
    }

    if (drop_everything) {
	for(otmp = invent; otmp; otmp = otmp2) {
	    otmp2 = otmp->nobj;
	    n_dropped += drop(otmp);
	}
    } else {
	/* should coordinate with perm invent, maybe not show worn items */
	n = query_objlist("What would you like to drop?", invent,
			USE_INVLET|INVORDER_SORT, &pick_list,
			PICK_ANY, all_categories ? allow_all : allow_category);
	if (n > 0) {
	    for (i = 0; i < n; i++) {
		otmp = pick_list[i].item.a_obj;
		cnt = pick_list[i].count;
		if (cnt < otmp->quan) {
		    if (welded(otmp)) {
			;	/* don't split */
		    } else if (otmp->otyp == LOADSTONE && otmp->cursed) {
			/* same kludge as getobj(), for canletgo()'s use */
			otmp->corpsenm = (int) cnt;	/* don't split */
		    } else {
#ifndef GOLDOBJ
			if (otmp->oclass == COIN_CLASS)
			    (void) splitobj(otmp, otmp->quan - cnt);
			else
#endif
			    otmp = splitobj(otmp, cnt);
		    }
		}
		n_dropped += drop(otmp);
	    }
	    free((genericptr_t) pick_list);
	}
    }

 drop_done:
#ifndef GOLDOBJ
    if (u_gold && invent && invent->oclass == COIN_CLASS) {
	/* didn't drop [all of] it */
	u_gold = invent;
	invent = u_gold->nobj;
	u_gold->in_use = FALSE;
	dealloc_obj(u_gold);
	update_inventory();
    }
#endif
    return n_dropped;
}

#endif /* OVL0 */
#ifdef OVL2

/* on a ladder, used in goto_level */
static NEARDATA boolean at_ladder = FALSE;

int
dodown()
{
	struct trap *trap = 0;
	boolean stairs_down = ((u.ux == xdnstair && u.uy == ydnstair) ||
		    (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up)),
		ladder_down = (u.ux == xdnladder && u.uy == ydnladder);

#ifdef STEED
	if (u.usteed && !u.usteed->mcanmove) {
		pline("%s won't move!", Monnam(u.usteed));
		return(0);
	} else if (u.usteed && u.usteed->meating) {
		pline("%s is still eating.", Monnam(u.usteed));
		return(0);
	} else
#endif
	if (Levitation) {
	    if ((HLevitation & I_SPECIAL) || (ELevitation & W_ARTI)) {
		/* end controlled levitation */
		if (ELevitation & W_ARTI) {
		    struct obj *obj;

		    for(obj = invent; obj; obj = obj->nobj) {
			if (obj->oartifact &&
					artifact_has_invprop(obj,LEVITATION)) {
			    if (obj->age < monstermoves)
				obj->age = monstermoves + rnz(100);
			    else
				obj->age += rnz(100);
			}
		    }
		}
		if (float_down(I_SPECIAL|TIMEOUT, W_ARTI))
		    return (1);   /* came down, so moved */
	    }
	    floating_above(stairs_down ? "stairs" : ladder_down ?
			   "ladder" : surface(u.ux, u.uy));
	    return (0);   /* didn't move */
	}
	if (!stairs_down && !ladder_down) {
		if (!(trap = t_at(u.ux,u.uy)) ||
			(trap->ttyp != TRAPDOOR && trap->ttyp != HOLE)
			|| !Can_fall_thru(&u.uz) || !trap->tseen) {

			if (flags.autodig && !flags.nopick &&
				uwep && (is_pick(uwep) || (is_lightsaber(uwep) && litsaber(uwep)) || (uwep->otyp == SEISMIC_HAMMER))) {
				return use_pick_axe2(uwep);
			} else if(uarmg && is_pick(uarmg)){
				return use_pick_axe2(uarmg);
			} else {
				if(levl[u.ux][u.uy].typ == STAIRS) pline("These stairs don't go down!");
				else You_cant("go down here.");
				return(0);
			}
		}
	}
	if(u.ustuck) {
		You("are %s, and cannot go down.",
			!u.uswallow ? "being held" : is_animal(u.ustuck->data) ?
			"swallowed" : "engulfed");
		return(1);
	}
	if (on_level(&valley_level, &u.uz) && !u.uevent.gehennom_entered) {
		You("are standing at the gate to Gehennom.");
		pline("Unspeakable cruelty and harm lurk down there.");
		if (yn("Are you sure you want to enter?") != 'y')
			return(0);
		else pline("So be it.");
		u.uevent.gehennom_entered = 1;	/* don't ask again */
#ifdef RECORD_ACHIEVE
		achieve.enter_gehennom = 1;
#endif
	}
	if(on_level(&spire_level,&u.uz)){
		u.uevent.sum_entered = 1; //entered sum of all
	}
	if(!next_to_u()) {
		You("are held back by your pet!");
		return(0);
	}

	if (trap)
	    You("%s %s.", locomotion(youracedata, "jump"),
		trap->ttyp == HOLE ? "down the hole" : "through the trap door");

	if (trap && Is_stronghold(&u.uz)) {
		goto_hell(FALSE, TRUE);
	} else {
		at_ladder = (boolean) (levl[u.ux][u.uy].typ == LADDER);
		next_level(!trap);
		at_ladder = FALSE;
	}
	return(1);
}

int
doup()
{
	if( (u.ux != xupstair || u.uy != yupstair)
	     && (!xupladder || u.ux != xupladder || u.uy != yupladder)
	     && (!sstairs.sx || u.ux != sstairs.sx || u.uy != sstairs.sy
			|| !sstairs.up)
		 && !(Role_if(PM_RANGER) && Race_if(PM_GNOME) && Is_qstart(&u.uz) && levl[u.ux][u.uy].ladder == LA_UP)
	) {
		if(uwep && uwep->oartifact == ART_ROD_OF_SEVEN_PARTS && u.RoSPflights > 0){
			struct obj *pseudo;
			pseudo = mksobj(SPE_LEVITATION, FALSE, FALSE);
			pseudo->blessed = pseudo->cursed = 0;
			pseudo->blessed = TRUE;
			pseudo->quan = 23L;			/* do not let useup get it */
			(void) peffects(pseudo);
			(void) peffects(pseudo);
			(void) peffects(pseudo);
			obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
			u.RoSPflights--;
		}
		else{
			if(levl[u.ux][u.uy].typ == STAIRS){
				pline("These stairs don't go up!");
			}
			else You_cant("go up here.");
		}
		return(0);
	}
#ifdef STEED
	if (u.usteed && !u.usteed->mcanmove) {
		pline("%s won't move!", Monnam(u.usteed));
		return(0);
	} else if (u.usteed && u.usteed->meating) {
		pline("%s is still eating.", Monnam(u.usteed));
		return(0);
	} else
#endif
	if(u.ustuck) {
		You("are %s, and cannot go up.",
			!u.uswallow ? "being held" : is_animal(u.ustuck->data) ?
			"swallowed" : "engulfed");
		return(1);
	}
	if(near_capacity() > SLT_ENCUMBER) {
		/* No levitation check; inv_weight() already allows for it */
		Your("load is too heavy to climb the %s.",
			levl[u.ux][u.uy].typ == STAIRS ? "stairs" : "ladder");
		return(1);
	}
	if(ledger_no(&u.uz) == 1) {
		if (yn("Beware, there will be no return! Still climb?") != 'y')
			return(0);
	}
	if(!next_to_u()) {
		You("are held back by your pet!");
		return(0);
	}
	if(Role_if(PM_RANGER) && Race_if(PM_GNOME) && Is_qstart(&u.uz) && levl[u.ux][u.uy].ladder == LA_UP){
		if(!u.uevent.qcompleted){
			pline("This staircase is partially collapsed.  It will be a tight squeeze.");
			if (bigmonst(youracedata) && !(u.sealsActive&SEAL_ANDREALPHUS) && !amorphous(youracedata)) {
				Your("body is too large to fit through.");
				return 1;
			}
			if (!invent || (inv_weight() + weight_cap() > 600) 
				|| (u.sealsActive&SEAL_ANDREALPHUS)
				|| (uarmc && (uarmc->otyp == OILSKIN_CLOAK || uarmc->greased)) 
				|| (uarm && uarm->greased) 
				|| (uarmu && uarmu->greased) 
			) {
				You("find you can't fit you gear past the rubble in this staircase.");
				return 1;
			}
			You("manage it, though.");
		}
		goto_level(&minetown_level,FALSE,FALSE,FALSE);
		return 1;
	}
	at_ladder = (boolean) (levl[u.ux][u.uy].typ == LADDER);
	prev_level(TRUE);
	at_ladder = FALSE;
	return(1);
}

d_level save_dlevel = {0, 0};

/* check that we can write out the current level */
STATIC_OVL int
currentlevel_rewrite()
{
	register int fd;
	char whynot[BUFSZ];

	/* since level change might be a bit slow, flush any buffered screen
	 *  output (like "you fall through a trap door") */
	mark_synch();

	fd = create_levelfile(ledger_no(&u.uz), whynot);
	if (fd < 0) {
		/*
		 * This is not quite impossible: e.g., we may have
		 * exceeded our quota. If that is the case then we
		 * cannot leave this level, and cannot save either.
		 * Another possibility is that the directory was not
		 * writable.
		 */
		pline("%s", whynot);
		return -1;
	}

#ifdef MFLOPPY
	if (!savelev(fd, ledger_no(&u.uz), COUNT_SAVE)) {
		(void) close(fd);
		delete_levelfile(ledger_no(&u.uz));
		pline("NetHack is out of disk space for making levels!");
		You("can save, quit, or continue playing.");
		return -1;
	}
#endif
	return fd;
}

#ifdef INSURANCE
void
save_currentstate()
{
	int fd;

	if (flags.ins_chkpt) {
		/* write out just-attained level, with pets and everything */
		fd = currentlevel_rewrite();
		if(fd < 0) return;
		bufon(fd);
		savelev(fd,ledger_no(&u.uz), WRITE_SAVE);
		bclose(fd);
	}

	/* write out non-level state */
	savestateinlock();
}
#endif

/*
static boolean
badspot(x, y)
register xchar x, y;
{
	return((levl[x][y].typ != ROOM && levl[x][y].typ != AIR &&
			 levl[x][y].typ != CORR) || MON_AT(x, y));
}
*/

void
goto_level(newlevel, at_stairs, falling, portal)
d_level *newlevel;
boolean at_stairs, falling, portal;
{
	int fd, l_idx;
	xchar new_ledger;
	boolean cant_go_back,
		up = (depth(newlevel) < depth(&u.uz)),
		newdungeon = (u.uz.dnum != newlevel->dnum),
		was_in_W_tower = In_W_tower(u.ux, u.uy, &u.uz),
		familiar = FALSE;
	boolean new = FALSE;	/* made a new level? */
	struct monst *mtmp;
	char whynot[BUFSZ];

	if (dunlev(newlevel) > dunlevs_in_dungeon(newlevel))
		newlevel->dlevel = dunlevs_in_dungeon(newlevel);
	if (newdungeon && In_endgame(newlevel)) { /* 1st Endgame Level !!! */
	    if (u.uhave.amulet) {
		    assign_level(newlevel, &earth_level);
		} else return;
	}
	new_ledger = ledger_no(newlevel);
	if (new_ledger <= 0)
		done(ESCAPED);	/* in fact < 0 is impossible */

	/* If you have the amulet and are trying to get out of Gehennom, going
	 * up a set of stairs sometimes does some very strange things!
	 * Biased against law and towards chaos, but not nearly as strongly
	 * as it used to be (prior to 3.2.0).
	 * Odds:	    old				    nethack				         dnethack
	 *	"up"    L      N      C		"up"    L      N      C		"up"    L      N      C 
	 *	 +1   75.0   75.0   75.0	 +1   75.0   75.0   75.0	 +1   66.66  66.66  66.6
	 *	  0    0.0   12.5   25.0	  0    6.25   8.33  12.5	  0    8.33  11.11  16.6
	 *	 -1    8.33   4.17   0.0	 -1    6.25   8.33  12.5	 -1    8.33  11.11  16.6
	 *	 -2    8.33   4.17   0.0	 -2    6.25   8.33   0.0	 -2    8.33  11.11   0.0
	 *	 -3    8.33   4.17   0.0	 -3    6.25   0.0    0.0	 -3    8.33   0.0    0.0
	 */
	if (Inhell && up && u.uhave.amulet && !newdungeon && !portal &&
				(dunlev(&u.uz) < dunlevs_in_dungeon(&u.uz)-3) &&
				(u.uz.dlevel < wiz1_level.dlevel) &&
				(u.uz.dlevel > valley_level.dlevel) ) {
		if (!rn2(3)) {
		    int odds = 3 + (int)u.ualign.type,		/* 2..4 */
			diff = odds <= 1 ? 0 : rn2(odds);	/* paranoia */

		    if (diff != 0) {
			assign_rnd_level(newlevel, &u.uz, diff);
			/* if inside the tower, stay inside */
			if (was_in_W_tower &&
			    !On_W_tower_level(newlevel)) diff = 0;
		    }
		    if (diff == 0)
			assign_level(newlevel, &u.uz);

		    new_ledger = ledger_no(newlevel);

		    pline("A mysterious force momentarily surrounds you...");
		    if (on_level(newlevel, &u.uz)) {
			(void) safe_teleds(FALSE);
			(void) next_to_u();
			return;
		    } else
			at_stairs = at_ladder = FALSE;
		}
	}

	/* Prevent the player from going past the first quest level unless
	 * (s)he has been given the go-ahead by the leader.
	 */
	if (on_level(&u.uz, &qstart_level) && !newdungeon && !ok_to_quest() && !flags.stag) {
		pline("A mysterious force prevents you from descending.");
		return;
	}
	// if (on_level(&u.uz, &nemesis_level) && !(quest_status.got_quest) && flags.stag) {
		// pline("A mysterious force prevents you from leaving.");
		// return;
	// }
	if (In_quest(&u.uz) && Race_if(PM_DWARF) &&  !up &&
		urole.neminum == PM_BOLG && Is_qlocate(&u.uz) && 
		!((mvitals[PM_SMAUG].mvflags & G_GENOD && !In_quest(&u.uz)) || mvitals[PM_SMAUG].died > 0)
	) {
		pline("A mysterious force prevents you from descending.");
		return;
	}

	if (on_level(newlevel, &u.uz)) return;		/* this can happen */

	fd = currentlevel_rewrite();
	if (fd < 0) return;

	if (falling) /* assuming this is only trap door or hole */
	    impact_drop((struct obj *)0, u.ux, u.uy, newlevel->dlevel);

	check_special_room(TRUE);		/* probably was a trap door */
	if (Punished) unplacebc();
	u.utrap = 0;				/* needed in level_tele */
	fill_pit(u.ux, u.uy);
	u.ustuck = 0;				/* idem */
	u.uinwater = 0;
	u.usubwater = 0;
	u.uundetected = 0;	/* not hidden, even if means are available */
	keepdogs(FALSE);
	if (u.uswallow)				/* idem */
		u.uswldtim = u.uswallow = 0;
	recalc_mapseen(); /* recalculate map overview before we leave the level */
	/*
	 *  We no longer see anything on the level.  Make sure that this
	 *  follows u.uswallow set to null since uswallow overrides all
	 *  normal vision.
	 */
	vision_recalc(2);

	/*
	 * Save the level we're leaving.  If we're entering the endgame,
	 * we can get rid of all existing levels because they cannot be
	 * reached any more.  We still need to use savelev()'s cleanup
	 * for the level being left, to recover dynamic memory in use and
	 * to avoid dangling timers and light sources.
	 */
	cant_go_back = (newdungeon && In_endgame(newlevel));
	if (!cant_go_back) {
	    update_mlstmv();	/* current monsters are becoming inactive */
	    bufon(fd);		/* use buffered output */
	}
	savelev(fd, ledger_no(&u.uz),
		cant_go_back ? FREE_SAVE : (WRITE_SAVE | FREE_SAVE));
	bclose(fd);
	if (cant_go_back) {
	    /* discard unreachable levels; keep #0 */
	    for (l_idx = maxledgerno(); l_idx > 0; --l_idx)
		delete_levelfile(l_idx);
	}

#ifdef REINCARNATION
	if (Is_rogue_level(newlevel) || Is_rogue_level(&u.uz))
		assign_rogue_graphics(Is_rogue_level(newlevel));
#endif
#ifdef USE_TILES
	substitute_tiles(newlevel);
#endif
	/* record this level transition as a potential seen branch unless using
	 * some non-standard means of transportation (level teleport).
	 */
	if ((at_stairs || falling || portal) && (u.uz.dnum != newlevel->dnum))
		recbranch_mapseen(&u.uz, newlevel);
	assign_level(&u.uz0, &u.uz);
	assign_level(&u.uz, newlevel);
	assign_level(&u.utolev, newlevel);
	u.utotype = 0;
	if (dunlev_reached(&u.uz) < dunlev(&u.uz))
		dunlev_reached(&u.uz) = dunlev(&u.uz);
	reset_rndmonst(NON_PM);   /* u.uz change affects monster generation */

	/* set default level change destination areas */
	/* the special level code may override these */
	(void) memset((genericptr_t) &updest, 0, sizeof updest);
	(void) memset((genericptr_t) &dndest, 0, sizeof dndest);

	if (!(level_info[new_ledger].flags & LFILE_EXISTS)) {
		/* entering this level for first time; make it now */
		if (level_info[new_ledger].flags & (FORGOTTEN|VISITED)) {
		    impossible("goto_level: returning to discarded level?");
		    level_info[new_ledger].flags &= ~(FORGOTTEN|VISITED);
		}
		mklev();
		new = TRUE;	/* made the level */
		if(Role_if(PM_TOURIST)){
			int dungeon_depth = 1;
			if (In_quest(&u.uz)) dungeon_depth = dunlev(&u.uz);
			else if (In_endgame(&u.uz) || Is_rlyeh(&u.uz) || Is_valley(&u.uz)) dungeon_depth = 100;
			else if (In_tower(&u.uz)) dungeon_depth = (5 - dunlev(&u.uz))*5;
			else if (In_law(&u.uz)) dungeon_depth = (path1_level.dlevel - u.uz.dlevel) + depth(&path1_level);
			else if (In_sokoban(&u.uz)) dungeon_depth = (5 - dunlev(&u.uz))*5;
			else dungeon_depth = depth(&u.uz) > 0 ? depth(&u.uz) : depth(&u.uz)-1;
			
			// more_experienced(u.ulevel*u.ulevel,0);
			more_experienced(u.ulevel*dungeon_depth,0);
			newexplevel();
		}
	} else {
		/* returning to previously visited level; reload it */
		fd = open_levelfile(new_ledger, whynot);
		if (fd < 0) {
			pline("%s", whynot);
			pline("Probably someone removed it.");
			killer = whynot;
			done(TRICKED);
			/* we'll reach here if running in wizard mode */
			error("Cannot continue this game.");
		}
		minit();	/* ZEROCOMP */
		getlev(fd, hackpid, new_ledger, FALSE);
		(void) close(fd);
	}
	/* do this prior to level-change pline messages */
	vision_reset();		/* clear old level's line-of-sight */
	vision_full_recalc = 0;	/* don't let that reenable vision yet */
	flush_screen(-1);	/* ensure all map flushes are postponed */

	if (portal && !In_endgame(&u.uz)) {
	    /* find the portal on the new level */
	    register struct trap *ttrap;
		int found=0;

		for (ttrap = ftrap; ttrap; ttrap = ttrap->ntrap){
			if (ttrap->ttyp == MAGIC_PORTAL && ttrap->dst.dlevel == u.uz0.dlevel){ //try to find a portal back to starting lev
				found = 1;
				break;
			}
		}
		if(!found){
			for (ttrap = ftrap; ttrap; ttrap = ttrap->ntrap) //otherwise just go with any portal
				if (ttrap->ttyp == MAGIC_PORTAL) break;
		}

	    if (!ttrap) panic("goto_level: no corresponding portal!");
	    seetrap(ttrap);
	    u_on_newpos(ttrap->tx, ttrap->ty);
	} else if (at_stairs && !In_endgame(&u.uz)) {
	    if (up) {
		if (at_ladder) {
		    u_on_newpos(xdnladder, ydnladder);
		} else {
		    if (newdungeon) {
			if (Is_stronghold(&u.uz)) {
			    register xchar x, y;

			    do {
				x = (COLNO - 2 - rnd(5));
				y = rn1(ROWNO - 4, 3);
			    } while(occupied(x, y) ||
				    IS_WALL(levl[x][y].typ));
			    u_on_newpos(x, y);
			} else u_on_sstairs();
		    } else u_on_dnstairs();
		}
		/* Remove bug which crashes with levitation/punishment  KAA */
		if (Punished && !Levitation) {
			pline("With great effort you climb the %s.",
				at_ladder ? "ladder" : "stairs");
		} else if (at_ladder)
		    You("climb up the ladder.");
	    } else {	/* down */
		if (at_ladder) {
		    u_on_newpos(xupladder, yupladder);
		} else {
		    if (newdungeon) u_on_sstairs();
		    else u_on_upstairs();
		}
		if (u.dz && Flying)
		    You("fly down along the %s.",
			at_ladder ? "ladder" : "stairs");
		else if (u.dz &&
#ifdef CONVICT
		    (near_capacity() > UNENCUMBERED || (Punished &&
		    ((uwep != uball) || ((P_SKILL(P_FLAIL) < P_BASIC))
            || !Role_if(PM_CONVICT)))
		     || Fumbling)
#else
		    (near_capacity() > UNENCUMBERED || Punished || Fumbling)
#endif /* CONVICT */
		) {
		    You("fall down the %s.", at_ladder ? "ladder" : "stairs");
		    if (Punished) {
			drag_down();
				if (carried(uball)) {
					if (uwep == uball)
					setuwep((struct obj *)0);
					if (uswapwep == uball)
					setuswapwep((struct obj *)0);
					if (uquiver == uball)
					setuqwep((struct obj *)0);
					freeinv(uball);
				}
		    }
			if(((uwep && is_lightsaber(uwep) && litsaber(uwep))) ||
				(uswapwep && is_lightsaber(uswapwep) && litsaber(uswapwep) && u.twoweap)
			){
				boolean mainsaber = (uwep && is_lightsaber(uwep) && litsaber(uwep));
				boolean secsaber = (uswapwep && is_lightsaber(uswapwep) && litsaber(uswapwep) && u.twoweap);
				if((mainsaber && uwep->oartifact == ART_INFINITY_S_MIRRORED_ARC)
					|| (secsaber && uswapwep->oartifact == ART_INFINITY_S_MIRRORED_ARC)
				){
					int lrole = rnl(20);
					if(lrole+5 < ACURR(A_DEX)){
						You("roll and dodge your tumbling energy sword%s.", (mainsaber && secsaber) ? "s" : "");
					} else {
						You("come into contact with your energy sword%s.", (mainsaber && secsaber && lrole >= ACURR(A_DEX)) ? "s" : "");
						if(mainsaber && (uwep->oartifact == ART_INFINITY_S_MIRRORED_ARC || lrole >= ACURR(A_DEX)))
							losehp(dmgval(uwep,&youmonst,0), "falling downstairs with a lit lightsaber", KILLED_BY);
						if(secsaber && (uswapwep->oartifact == ART_INFINITY_S_MIRRORED_ARC || lrole >= ACURR(A_DEX)))
							losehp(dmgval(uswapwep,&youmonst,0), "falling downstairs with a lit lightsaber", KILLED_BY);
					}
					if(mainsaber && uwep->oartifact != ART_INFINITY_S_MIRRORED_ARC)
						lightsaber_deactivate(uwep, TRUE);
					if(secsaber && uswapwep->oartifact != ART_INFINITY_S_MIRRORED_ARC)
						lightsaber_deactivate(uswapwep, TRUE);
				} else {
					if(rnl(20) < ACURR(A_DEX)){
						You("hurriedly deactivate your energy sword%s.", (mainsaber && secsaber) ? "s" : "");
					} else {
						You("come into contact with your energy sword%s.", (mainsaber && secsaber) ? "s" : "");
						if(mainsaber) losehp(dmgval(uwep,&youmonst,0), "falling downstairs with a lit lightsaber", KILLED_BY);
						if(secsaber) losehp(dmgval(uswapwep,&youmonst,0), "falling downstairs with a lit lightsaber", KILLED_BY);
					}
					if(mainsaber) lightsaber_deactivate(uwep, TRUE);
					if(secsaber) lightsaber_deactivate(uswapwep, TRUE);
				}
			}
#ifdef STEED
		    /* falling off steed has its own losehp() call */
		    if (u.usteed)
			dismount_steed(DISMOUNT_FELL);
		    else
#endif
			losehp(rnd(3), "falling downstairs", KILLED_BY);
		    selftouch("Falling, you");
		} else if (u.dz && at_ladder)
		    You("climb down the ladder.");
	    }
	} else {	/* trap door or level_tele or In_endgame */
	    if (was_in_W_tower && On_W_tower_level(&u.uz))
		/* Stay inside the Wizard's tower when feasible.	*/
		/* Note: up vs down doesn't really matter in this case. */
		place_lregion(dndest.nlx, dndest.nly,
				dndest.nhx, dndest.nhy,
				0,0, 0,0, LR_DOWNTELE, (d_level *) 0);
	    else if (up)
		place_lregion(updest.lx, updest.ly,
				updest.hx, updest.hy,
				updest.nlx, updest.nly,
				updest.nhx, updest.nhy,
				LR_UPTELE, (d_level *) 0);
	    else
		place_lregion(dndest.lx, dndest.ly,
				dndest.hx, dndest.hy,
				dndest.nlx, dndest.nly,
				dndest.nhx, dndest.nhy,
				LR_DOWNTELE, (d_level *) 0);
	    if (falling) {
		if (Punished) ballfall();
		selftouch("Falling, you");
	    }
	}

	if (Punished) placebc();
	obj_delivery();		/* before killing geno'd monsters' eggs */
	losedogs();
	kill_genocided_monsters();  /* for those wiped out while in limbo */
	/*
	 * Expire all timers that have gone off while away.  Must be
	 * after migrating monsters and objects are delivered
	 * (losedogs and obj_delivery).
	 */
	run_timers();

	initrack();

	if ((mtmp = m_at(u.ux, u.uy)) != 0
#ifdef STEED
		&& mtmp != u.usteed
#endif
		) {
	    /* There's a monster at your target destination; it might be one
	       which accompanied you--see mon_arrive(dogmove.c)--or perhaps
	       it was already here.  Randomly move you to an adjacent spot
	       or else the monster to any nearby location.  Prior to 3.3.0
	       the latter was done unconditionally. */
	    coord cc;

	    if (!rn2(2) &&
		    enexto(&cc, u.ux, u.uy, youracedata) &&
		    distu(cc.x, cc.y) <= 2)
		u_on_newpos(cc.x, cc.y);	/*[maybe give message here?]*/
	    else
		mnexto(mtmp);

	    if ((mtmp = m_at(u.ux, u.uy)) != 0) {
		impossible("mnexto failed (do.c)?");
		(void) rloc(mtmp, FALSE);
	    }
	}

	/* initial movement of bubbles just before vision_recalc */
	if (Is_waterlevel(&u.uz))
		movebubbles();

	if (level_info[new_ledger].flags & FORGOTTEN) {
	    forget_map(100);	/* forget the map */
	    forget_traps();		/* forget all traps too */
	    familiar = TRUE;
	    level_info[new_ledger].flags &= ~FORGOTTEN;
	}

	/* Reset the screen. */
	vision_reset();		/* reset the blockages */
	docrt();		/* does a full vision recalc */
	flush_screen(-1);

	/*
	 *  Move all plines beyond the screen reset.
	 */

	/* give room entrance message, if any */
	check_special_room(FALSE);

	/* Check whether we just entered Gehennom. */
	if (!In_hell(&u.uz0) && Inhell) {
	    if (Is_valley(&u.uz)) {
		You("arrive at the Valley of the Dead...");
		pline_The("odor of burnt flesh and decay pervades the air.");
#ifdef MICRO
		display_nhwindow(WIN_MESSAGE, FALSE);
#endif
		You_hear("groans and moans everywhere.");
	    } else {
			pline("It is hot here.  You smell smoke...");
#ifdef RECORD_ACHIEVE
			achieve.enter_gehennom = 1;
#endif
		}
	}

	if (familiar) {
	    static const char * const fam_msgs[4] = {
		"You have a sense of deja vu.",
		"You feel like you've been here before.",
		"This place %s familiar...",
		0	/* no message */
	    };
	    static const char * const halu_fam_msgs[4] = {
		"Whoa!  Everything %s different.",
		"You are surrounded by twisty little passages, all alike.",
		"Gee, this %s like uncle Conan's place...",
		0	/* no message */
	    };
	    const char *mesg;
	    char buf[BUFSZ];
	    int which = rn2(4);

	    if (Hallucination)
		mesg = halu_fam_msgs[which];
	    else
		mesg = fam_msgs[which];
	    if (mesg && index(mesg, '%')) {
		Sprintf(buf, mesg, !Blind ? "looks" : "seems");
		mesg = buf;
	    }
	    if (mesg) pline1(mesg);
	}

#ifdef REINCARNATION
	if (new && Is_rogue_level(&u.uz))
	    You("enter what seems to be an older, more primitive world.");
#endif
	/* Final confrontation */
	if (In_endgame(&u.uz) && newdungeon && u.uhave.amulet)
		resurrect();
	if (newdungeon && In_V_tower(&u.uz) && In_hell(&u.uz0))
		pline_The("heat and smoke are gone.");

	/* the message from your quest leader */
	if (!In_quest(&u.uz0) && at_dgn_entrance("The Quest") &&
		!(u.uevent.qexpelled || u.uevent.qcompleted || quest_status.leader_is_dead)) {
		if(Role_if(PM_EXILE)){
			You("sense something reaching out to you....");
		} else {
			if (u.uevent.qcalled) {
				com_pager(Role_if(PM_ROGUE) ? 4 : 3);
			} else {
				com_pager(2);
				u.uevent.qcalled = TRUE;
			}
		}
	}

	/* once Croesus is dead, his alarm doesn't work any more */
	if (Is_knox(&u.uz) && (new || !mvitals[PM_CROESUS].died)) {
		You("penetrated a high security area!");
		pline("An alarm sounds!");
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		    if (!DEADMONSTER(mtmp) && mtmp->msleeping) mtmp->msleeping = 0;
	}

	if (on_level(&u.uz, &astral_level))
	    final_level();
	else
	    onquest();
	assign_level(&u.uz0, &u.uz); /* reset u.uz0 */

#ifdef INSURANCE
	save_currentstate();
#endif

	/* assume this will always return TRUE when changing level */
	(void) in_out_region(u.ux, u.uy);
	(void) pickup(1);
	if(Is_waterlevel(&u.uz)) spoteffects(FALSE); /*Decided to be specific about this.  Dump character in water when coming out of portal*/
}

STATIC_OVL void
final_level()
{
	struct monst *mtmp;
	struct obj *otmp;
	coord mm;
	int i;

	/* reset monster hostility relative to player */
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if (!DEADMONSTER(mtmp)) reset_hostility(mtmp);

	/* create some player-monsters */
	create_mplayers(rn1(4, 3), TRUE);

	if(u.uevent.ukilled_apollyon){
		int host;
	    pline(
	     "A voice booms: \"The Angel of the Pit hast fallen!  We have returned!\"");
		(void) makemon(&mons[PM_LUCIFER], u.ux, u.uy, MM_ADJACENTOK);
		(void) makemon(&mons[PM_ANCIENT_OF_DEATH], u.ux, u.uy, MM_ADJACENTOK);
		(void) makemon(&mons[PM_ANCIENT_OF_ICE], u.ux, u.uy, MM_ADJACENTOK);
/*		for(host = 0; host < 10; host++ )*/ (void) makemon(&mons[PM_FALLEN_ANGEL], u.ux, u.uy, MM_ADJACENTOK);
	}
	/* create a guardian angel next to player, if worthy */
	if (Conflict) {
	    pline(
	     "A voice booms: \"Thy desire for conflict shall be fulfilled!\"");
	    for (i = rnd(4); i > 0; --i) {
		mm.x = u.ux;
		mm.y = u.uy;
		if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL]))
		    (void) mk_roamer(&mons[PM_ANGEL], u.ualign.type,
				     mm.x, mm.y, FALSE);
	    }

	} else if (u.ualign.record > 8) {	/* fervent */
	    pline("A voice whispers: \"Thou hast been worthy of me!\"");
	    mm.x = u.ux;
	    mm.y = u.uy;
	    if (enexto(&mm, mm.x, mm.y, &mons[PM_ANGEL])) {
		if ((mtmp = mk_roamer(&mons[PM_ANGEL], u.ualign.type,
				      mm.x, mm.y, TRUE)) != 0) {
		    if (!Blind)
			pline("An angel appears near you.");
		    else
			You_feel("the presence of a friendly angel near you.");
		    /* guardian angel -- the one case mtame doesn't
		     * imply an edog structure, so we don't want to
		     * call tamedog().
		     */
		    mtmp->mtame = 10;
		    /* make him strong enough vs. endgame foes */
		    mtmp->m_lev = rn1(8,15);
		    mtmp->mhp = mtmp->mhpmax = 8*mtmp->m_lev - rnd(7);
		    if ((otmp = select_hwep(mtmp)) == 0) {
			otmp = mksobj(SABER, FALSE, FALSE);
			if (mpickobj(mtmp, otmp))
			    panic("merged weapon?");
		    }
		    bless(otmp);
		    if (otmp->spe < 4) otmp->spe += rnd(4);
		    if ((otmp = which_armor(mtmp, W_ARMS)) == 0 ||
			    otmp->otyp != SHIELD_OF_REFLECTION) {
			(void) mongets(mtmp, AMULET_OF_REFLECTION);
			m_dowear(mtmp, TRUE);
			init_mon_wield_item(mtmp);
		    }
		}
	    }
	}
}

static char *dfr_pre_msg = 0,	/* pline() before level change */
	    *dfr_post_msg = 0;	/* pline() after level change */

/* change levels at the end of this turn, after monsters finish moving */
void
schedule_goto(tolev, at_stairs, falling, portal_flag, pre_msg, post_msg)
d_level *tolev;
boolean at_stairs, falling;
int portal_flag;
const char *pre_msg, *post_msg;
{
	int typmask = 0100;		/* non-zero triggers `deferred_goto' */

	/* destination flags (`goto_level' args) */
	if (at_stairs)	 typmask |= 1;
	if (falling)	 typmask |= 2;
	if (portal_flag) typmask |= 4;
	if (portal_flag < 0) typmask |= 0200;	/* flag for portal removal */
	u.utotype = typmask;
	/* destination level */
	assign_level(&u.utolev, tolev);

	if (pre_msg)
	    dfr_pre_msg = strcpy((char *)alloc(strlen(pre_msg) + 1), pre_msg);
	if (post_msg)
	    dfr_post_msg = strcpy((char *)alloc(strlen(post_msg)+1), post_msg);
}

/* handle something like portal ejection */
void
deferred_goto()
{
	if (!on_level(&u.uz, &u.utolev)) {
	    d_level dest;
	    int typmask = u.utotype; /* save it; goto_level zeroes u.utotype */

	    assign_level(&dest, &u.utolev);
	    if (dfr_pre_msg) pline1(dfr_pre_msg);
	    goto_level(&dest, !!(typmask&1), !!(typmask&2), !!(typmask&4));
	    if (typmask & 0200) {	/* remove portal */
		struct trap *t = t_at(u.ux, u.uy);

		if (t) {
		    deltrap(t);
		    newsym(u.ux, u.uy);
		}
	    }
	    if (dfr_post_msg) pline1(dfr_post_msg);
	}
	u.utotype = 0;		/* our caller keys off of this */
	if (dfr_pre_msg)
	    free((genericptr_t)dfr_pre_msg),  dfr_pre_msg = 0;
	if (dfr_post_msg)
	    free((genericptr_t)dfr_post_msg),  dfr_post_msg = 0;
}

#endif /* OVL2 */
#ifdef OVL3

/*
 * Return TRUE if we created a monster for the corpse.  If successful, the
 * corpse is gone.
 */

boolean
revive_corpse(corpse, different)
struct obj *corpse;
int different;
{
    struct monst *mtmp, *mcarry;
    boolean is_uwep, chewed;
    xchar where;
    char *cname, cname_buf[BUFSZ];
    struct obj *container = (struct obj *)0;
    int container_where = 0;
    where = corpse->where;
    is_uwep = corpse == uwep;
    cname = eos(strcpy(cname_buf, "bite-covered "));
    Strcpy(cname, corpse_xname(corpse, TRUE));
    mcarry = (where == OBJ_MINVENT) ? corpse->ocarry : 0;

    if (where == OBJ_CONTAINED) {
    	struct monst *mtmp2 = (struct monst *)0;
		container = corpse->ocontainer;
    	mtmp2 = get_container_location(container, &container_where, (int *)0);
		/* container_where is the outermost container's location even if nested */
		if (container_where == OBJ_MINVENT && mtmp2) mcarry = mtmp2;
    }
    mtmp = revive(corpse);      /* corpse is gone if successful && quan == 1 */

    if (mtmp) {
	/*
	 * [ALI] Override revive's HP calculation. The HP that a mold starts
	 * with do not depend on the HP of the monster whose corpse it grew on.
	 */
	if (different)
	    mtmp->mhp = mtmp->mhpmax;
	chewed = !different && (mtmp->mhp < mtmp->mhpmax);
	if (chewed) cname = cname_buf;	/* include "bite-covered" prefix */
	if(different==REVIVE_ZOMBIE){
		mtmp->mfaction = ZOMBIFIED;
		mtmp->zombify = 0;
	}
	switch (where) {
	    case OBJ_INVENT:
		if (is_uwep) {
		    if (different==GROW_MOLD) {
				Your("weapon goes moldy.");
				pline("%s writhes out of your grasp!", Monnam(mtmp));
		    }
		    else if (different==GROW_SLIME) {
				Your("weapon goes slimy.");
				pline("%s slips out of your grasp!", Monnam(mtmp));
		    }
		    else if (different==REVIVE_ZOMBIE) {
				pline_The("%s rises from the dead!", cname);
				pline("%s writhes out of your grasp!", Monnam(mtmp));
		    }
		    else{
				pline_The("%s writhes out of your grasp!", cname);
			}
		}
		else
		    You_feel("squirming in your backpack!");
		break;

	    case OBJ_FLOOR:
		if (cansee(mtmp->mx, mtmp->my)) {
		    if (different==GROW_MOLD)
				pline("%s grows on a moldy corpse!",
				  Amonnam(mtmp));
		    else if (different==REVIVE_MOLD)
				pline("%s regrows from its corpse!",
				  Amonnam(mtmp));
		    else if (different==GROW_SLIME)
				pline("%s leaks from a putrefying corpse!",
				  Amonnam(mtmp));
		    else if (different==REVIVE_ZOMBIE)
				pline("%s rises from the dead!",
				  Amonnam(mtmp));
		    else if (different==REVIVE_SHADE)
				pline("%s forms from a corpse!",
				  Amonnam(mtmp));
		    else if (mtmp->data==&mons[PM_DEATH])
				pline("Death cannot die.");
			else
				pline("%s rises from the dead!", chewed ?
					Adjmonnam(mtmp, "bite-covered") : Monnam(mtmp));
		}
		break;

	    case OBJ_MINVENT:		/* probably a nymph's */
		if (cansee(mtmp->mx, mtmp->my)) {
		    if (canseemon(mcarry))
			pline("Startled, %s drops %s as it %s!",
			      mon_nam(mcarry), different ? "a corpse" : an(cname),
			      different==GROW_MOLD ? "goes moldy" : 
			      different==GROW_SLIME ? "putrefies" : 
			      different==REVIVE_ZOMBIE ? "rises from the dead" : 
			      different==REVIVE_SHADE ? "dissolves into shadow" : 
				  "revives");
		    else
			pline("%s suddenly appears!", chewed ?
			      Adjmonnam(mtmp, "bite-covered") : Monnam(mtmp));
		}
		break;
	   case OBJ_CONTAINED:
	   	if (container_where == OBJ_MINVENT && cansee(mtmp->mx, mtmp->my) &&
		    mcarry && canseemon(mcarry) && container) {
		        char sackname[BUFSZ];
		        Sprintf(sackname, "%s %s", s_suffix(mon_nam(mcarry)),
				xname(container)); 
	   		pline("%s writhes out of %s!", Amonnam(mtmp), sackname);
	   	} else if (container_where == OBJ_INVENT && container) {
		        char sackname[BUFSZ];
		        Strcpy(sackname, an(xname(container)));
	   		pline("%s %s out of %s in your pack!",
	   			Blind ? Something : Amonnam(mtmp),
				locomotion(mtmp->data,"writhes"),
	   			sackname);
	   	} else if (container_where == OBJ_FLOOR && container &&
		            cansee(mtmp->mx, mtmp->my)) {
		        char sackname[BUFSZ];
		        Strcpy(sackname, an(xname(container)));
			pline("%s escapes from %s!", Amonnam(mtmp), sackname);
		}
		break;
	    default:
		/* we should be able to handle the other cases... */
		impossible("revive_corpse: lost corpse @ %d", where);
		break;
	}
	return TRUE;
    }
    return FALSE;
}

/* Revive the corpse via a timeout. */
/*ARGSUSED*/
void
revive_mon(arg, timeout)
genericptr_t arg;
long timeout;
{
    struct obj *body = (struct obj *) arg;

    /* if we succeed, the corpse is gone, otherwise, rot it away */
    if (!revive_corpse(body, 
					(is_fungus(&mons[body->corpsenm]) &&
					!is_migo(&mons[body->corpsenm])) ? 
						REVIVE_MOLD : 
						REVIVE_MONSTER)
	) {
		if (is_rider(&mons[body->corpsenm]))
			You_feel("less hassled.");
		(void) start_timer(250L - (monstermoves-body->age),
						TIMER_OBJECT, ROT_CORPSE, arg);
    }
}


static const int molds[] = 
{
	PM_BROWN_MOLD,
	PM_YELLOW_MOLD,
	PM_GREEN_MOLD,
	PM_RED_MOLD
};
static const int slimes[] = 
{
	PM_BROWN_PUDDING,
	PM_BLACK_PUDDING,
	PM_ACID_BLOB,
	PM_GELATINOUS_CUBE
};
static const int shades[] = 
{
	PM_SHADE,
	PM_SHADE,
	PM_SHADE,
	PM_PHANTASM
};
/* Revive the corpse as a mold via a timeout. */
/*ARGSUSED*/
void
moldy_corpse(arg, timeout)
genericptr_t arg;
long timeout;
{
	int pmtype, oldtyp, oldquan;
	struct obj *body = (struct obj *) arg;

	/* Turn the corpse into a mold corpse if molds are available */
	oldtyp = body->corpsenm;

	/* Weight towards non-motile fungi.
	 */
	//	fruitadd("slime mold");
	pmtype = molds[rn2(SIZE(molds))];

	/* [ALI] Molds don't grow in adverse conditions.  If it ever
	 * becomes possible for molds to grow in containers we should
	 * check for iceboxes here as well.
	 */
	if ((
			(body->where == OBJ_FLOOR || body->where==OBJ_BURIED) &&
			(is_pool(body->ox, body->oy, FALSE) || is_lava(body->ox, body->oy) ||
				is_ice(body->ox, body->oy))
		) || (
			(body->where == OBJ_CONTAINED && body->ocontainer->otyp == ICE_BOX)
		)
	) pmtype = -1;

	if (pmtype != -1) {
		/* We don't want special case revivals */
		if (cant_create(&pmtype, TRUE) || (body->oxlth &&
					(body->oattached == OATTACHED_MONST)))
			pmtype = -1; /* cantcreate might have changed it so change it back */
		else {
				body->corpsenm = pmtype;

			/* oeaten isn't used for hp calc here, and zeroing it 
			 * prevents eaten_stat() from worrying when you've eaten more
			 * from the corpse than the newly grown mold's nutrition
			 * value.
			 */
			body->oeaten = 0;

			/* [ALI] If we allow revive_corpse() to get rid of revived
			 * corpses from hero's inventory then we run into problems
			 * with unpaid corpses.
			 */
			if (body->where == OBJ_INVENT)
				body->quan++;
			oldquan = body->quan;
			if (revive_corpse(body, GROW_MOLD)) {
				if (oldquan != 1) {		/* Corpse still valid */
					body->corpsenm = oldtyp;
					if (body->where == OBJ_INVENT) {
						useup(body);
						oldquan--;
					}
				}
				if (oldquan == 1)
				body = (struct obj *)0;	/* Corpse gone */
			}
		}
	}

	/* If revive_corpse succeeds, it handles the reviving corpse.
	 * If there was more than one corpse, or the revive failed,
	 * set the remaining corpse(s) to rot away normally.
	 * Revive_corpse handles genocides
	 */
	if (body) {
		body->corpsenm = oldtyp; /* Fixup corpse after (attempted) revival */
		body->owt = weight(body);
		(void) start_timer(250L - (monstermoves-peek_at_iced_corpse_age(body)),
			TIMER_OBJECT, ROT_CORPSE, arg);
	}
}

/* Revive the corpse as a slime via a timeout. */
/*ARGSUSED*/
void
slimy_corpse(arg, timeout)
genericptr_t arg;
long timeout;
{
	int pmtype, oldtyp, oldquan;
	struct obj *body = (struct obj *) arg;

	/* Turn the corpse into a slimy corpse if slimes are available */
	oldtyp = body->corpsenm;

	pmtype = slimes[rn2(SIZE(slimes))];

	/* [ALI] Molds don't grow in adverse conditions.  If it ever
	 * becomes possible for molds to grow in containers we should
	 * check for iceboxes here as well.
	 */
	if ((body->where == OBJ_FLOOR || body->where==OBJ_BURIED) &&
	  (is_pool(body->ox, body->oy, FALSE) || is_lava(body->ox, body->oy) ||
	  is_ice(body->ox, body->oy)))
	pmtype = -1;

	if (pmtype != -1) {
	/* We don't want special case revivals */
		if (cant_create(&pmtype, TRUE) || (body->oxlth &&
					(body->oattached == OATTACHED_MONST)))
			pmtype = -1; /* cantcreate might have changed it so change it back */
		else {
			body->corpsenm = pmtype;

		/* oeaten isn't used for hp calc here, and zeroing it 
		 * prevents eaten_stat() from worrying when you've eaten more
		 * from the corpse than the newly grown mold's nutrition
		 * value.
		 */
		body->oeaten = 0;

		/* [ALI] If we allow revive_corpse() to get rid of revived
		 * corpses from hero's inventory then we run into problems
		 * with unpaid corpses.
		 */
		if (body->where == OBJ_INVENT)
			body->quan++;
		oldquan = body->quan;
			if (revive_corpse(body, GROW_SLIME)) {
			if (oldquan != 1) {		/* Corpse still valid */
			body->corpsenm = oldtyp;
			if (body->where == OBJ_INVENT) {
				useup(body);
				oldquan--;
			}
			}
			if (oldquan == 1)
			body = (struct obj *)0;	/* Corpse gone */
		}
		}
	}
}

/* Revive the corpse as a shade via a timeout. */
/*ARGSUSED*/
void
shady_corpse(arg, timeout)
genericptr_t arg;
long timeout;
{
	int pmtype, oldtyp, oldquan;
	struct obj *body = (struct obj *) arg;

	/* Turn the corpse into a slimy corpse if shades are available */
	oldtyp = body->corpsenm;

	pmtype = shades[rn2(SIZE(shades))];
	if(!rn2(100)) pmtype = PM_WRAITH;

	/* [ALI] Molds don't grow in adverse conditions.  If it ever
	 * becomes possible for molds to grow in containers we should
	 * check for iceboxes here as well.
	 */
	if ((body->where == OBJ_FLOOR || body->where==OBJ_BURIED) &&
	  (is_pool(body->ox, body->oy, FALSE) || is_lava(body->ox, body->oy) ||
	  is_ice(body->ox, body->oy)))
	pmtype = -1;

	if (pmtype != -1) {
	/* We don't want special case revivals */
		if (cant_create(&pmtype, TRUE) || (body->oxlth &&
					(body->oattached == OATTACHED_MONST)))
			pmtype = -1; /* cantcreate might have changed it so change it back */
		else {
			body->corpsenm = pmtype;

		/* oeaten isn't used for hp calc here, and zeroing it 
		 * prevents eaten_stat() from worrying when you've eaten more
		 * from the corpse than the newly grown mold's nutrition
		 * value.
		 */
		body->oeaten = 0;

		/* [ALI] If we allow revive_corpse() to get rid of revived
		 * corpses from hero's inventory then we run into problems
		 * with unpaid corpses.
		 */
		if (body->where == OBJ_INVENT)
			body->quan++;
		oldquan = body->quan;
			if (revive_corpse(body, REVIVE_SHADE)) {
			if (oldquan != 1) {		/* Corpse still valid */
			body->corpsenm = oldtyp;
			if (body->where == OBJ_INVENT) {
				useup(body);
				oldquan--;
			}
			}
			if (oldquan == 1)
			body = (struct obj *)0;	/* Corpse gone */
		}
		}
	}

	/* If revive_corpse succeeds, it handles the reviving corpse.
	 * If there was more than one corpse, or the revive failed,
	 * set the remaining corpse(s) to rot away normally.
	 * Revive_corpse handles genocides
	 */
	if (body) {
		body->corpsenm = oldtyp; /* Fixup corpse after (attempted) revival */
		body->owt = weight(body);
		(void) start_timer(250L - (monstermoves-peek_at_iced_corpse_age(body)),
			TIMER_OBJECT, ROT_CORPSE, arg);
	}
}

/* Revive the corpse as a zombie via a timeout. */
/*ARGSUSED*/
void
zombie_corpse(arg, timeout)
genericptr_t arg;
long timeout;
{
	int pmtype, oldtyp, oldquan;
	struct obj *body = (struct obj *) arg;
	
	pmtype = body->corpsenm;
	
	/* [ALI] Molds don't grow in adverse conditions.  If it ever
	 * becomes possible for molds to grow in containers we should
	 * check for iceboxes here as well.
	 */
	if ((body->where == OBJ_FLOOR || body->where==OBJ_BURIED) &&
	  (is_pool(body->ox, body->oy, FALSE) || is_lava(body->ox, body->oy) ||
	  is_ice(body->ox, body->oy)))
	pmtype = -1;
	
	if (pmtype != -1) {
		/* We don't want special case revivals */
		if (cant_create(&pmtype, TRUE) || (body->oxlth && !(((struct monst *)body->oextra)->zombify) &&
					(body->oattached == OATTACHED_MONST)))
			pmtype = -1; /* cantcreate might have changed it so change it back */
		else {
			body->corpsenm = pmtype;

		/* oeaten isn't used for hp calc here, and zeroing it 
		 * prevents eaten_stat() from worrying when you've eaten more
		 * from the corpse than the newly grown mold's nutrition
		 * value.
		 */
		body->oeaten = 0;

		/* [ALI] If we allow revive_corpse() to get rid of revived
		 * corpses from hero's inventory then we run into problems
		 * with unpaid corpses.
		 */
		if (body->where == OBJ_INVENT)
			body->quan++;
		oldquan = body->quan;
			if (revive_corpse(body, REVIVE_ZOMBIE)) {
			if (oldquan != 1) {		/* Corpse still valid */
			if (body->where == OBJ_INVENT) {
				useup(body);
				oldquan--;
			}
			}
			if (oldquan == 1)
			body = (struct obj *)0;	/* Corpse gone */
		}
		}
	}

	/* If revive_corpse succeeds, it handles the reviving corpse.
	 * If there was more than one corpse, or the revive failed,
	 * set the remaining corpse(s) to rot away normally.
	 * Revive_corpse handles genocides
	 */
	if (body) {
		body->owt = weight(body);
		(void) start_timer(250L - (monstermoves-peek_at_iced_corpse_age(body)),
			TIMER_OBJECT, ROT_CORPSE, arg);
	}
}

int
donull()
{
	static long lastreped = -13;//hacky way to tell if the player has recently tried repairing themselves
	if(uclockwork){
		if(!Upolyd && u.uhp<u.uhpmax){
			if(lastreped < monstermoves-13) You("attempt to make repairs.");
			if(!rn2(15-u.ulevel/2)){
				u.uhp += rnd(10);
				flags.botl = 1;
			}
			if(u.uhp > u.uhpmax) u.uhp = u.uhpmax;
			lastreped = monstermoves;
			if(u.uhp == u.uhpmax){
				You("complete your repairs.");
				lastreped = -13;
				stop_occupation();
				occupation = 0; /*redundant failsafe? why doesn't stop_occupation work?*/
			}
		} else if(Upolyd && u.mh<u.mhmax){
			if(lastreped < monstermoves-100) You("attempt to make repairs.");
			if(!rn2(15-u.ulevel/2)){
				u.mh += rnd(10);
				flags.botl = 1;
			}
			if(u.mh > u.mhmax) u.mh = u.mhmax;
			lastreped = monstermoves;
			if(u.mh == u.mhmax){
				You("complete your repairs.");
				lastreped = -13;
				stop_occupation();
				occupation = 0; /*redundant failsafe? why doesn't stop_occupation work?*/
			}
		} else if(u.sealsActive&SEAL_EURYNOME && ++u.eurycounts>5) unbind(SEAL_EURYNOME,TRUE);
	} else if(u.sealsActive&SEAL_EURYNOME && ++u.eurycounts>5) unbind(SEAL_EURYNOME,TRUE);
	return(1);	/* Do nothing, but let other things happen */
}

#endif /* OVL3 */
#ifdef OVLB

STATIC_PTR int
wipeoff()
{
	if(u.ucreamed < 4)	u.ucreamed = 0;
	else			u.ucreamed -= 4;
	if (Blinded < 4)	Blinded = 0;
	else			Blinded -= 4;
	if (!Blinded) {
		pline("You've got the glop off.");
		u.ucreamed = 0;
		Blinded = 1;
		make_blinded(0L,TRUE);
		return(0);
	} else if (!u.ucreamed) {
		Your("%s feels clean now.", body_part(FACE));
		return(0);
	}
	return(1);		/* still busy */
}

int
dowipe()
{
	if(u.ucreamed)  {
		static NEARDATA char buf[39];

		Sprintf(buf, "wiping off your %s", body_part(FACE));
		set_occupation(wipeoff, buf, 0);
		/* Not totally correct; what if they change back after now
		 * but before they're finished wiping?
		 */
		return(1);
	}
	Your("%s is already clean.", body_part(FACE));
	return(1);
}

void
set_wounded_legs(side, timex)
register long side;
register int timex;
{
	/* KMH -- STEED
	 * If you are riding, your steed gets the wounded legs instead.
	 * You still call this function, but don't lose hp.
	 * Caller is also responsible for adjusting messages.
	 */

	if(!Wounded_legs) {
		ATEMP(A_DEX)--;
		flags.botl = 1;
	}

	if(!Wounded_legs || (HWounded_legs & TIMEOUT))
		HWounded_legs = timex;
	EWounded_legs = side;
	(void)encumber_msg();
}

void
heal_legs()
{
	if(Wounded_legs) {
		if (ATEMP(A_DEX) < 0) {
			ATEMP(A_DEX)++;
			flags.botl = 1;
		}

#ifdef STEED
		if (!u.usteed)
#endif
		{
			/* KMH, intrinsics patch */
			if((EWounded_legs & BOTH_SIDES) == BOTH_SIDES) {
			Your("%s feel somewhat better.",
				makeplural(body_part(LEG)));
		} else {
			Your("%s feels somewhat better.",
				body_part(LEG));
		}
		}
		HWounded_legs = EWounded_legs = 0;
	}
	(void)encumber_msg();
}

int
dowait()
{
	struct monst *mtmp;
	if (!getdir("Indicate pet that should wait, or '.' for all.")) return(0);
	if(!(u.dx || u.dy)){
		You("order all your pets to wait for your return.");
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
			if(mtmp->mtame) mtmp->mwait = monstermoves;
		}
	}
	else if(isok(u.ux+u.dx, u.uy+u.dy)) {
		mtmp = m_at(u.ux+u.dx, u.uy+u.dy);
		if(!mtmp){
			pline("There is no target there.");
			return 0;
		}
		if(mtmp->mtame){
			mtmp->mwait = monstermoves;
			You("order %s to wait for your return.", mon_nam(mtmp));
		}
	} else pline("There is no target there.");
	return 0;
}

int
docome()
{
	struct monst *mtmp;
	if (!getdir("Indicate pet that should come with you, or '.' for all.")) return(0);
	if(!(u.dx || u.dy)){
		You("order all your pets to follow you.");
		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
			if(mtmp->mtame) mtmp->mwait = 0;
		}
	}
	else if(isok(u.ux+u.dx, u.uy+u.dy)) {
		mtmp = m_at(u.ux+u.dx, u.uy+u.dy);
		if(!mtmp){
			pline("There is no target there.");
			return 0;
		}
		if(mtmp->mtame){
			mtmp->mwait = 0;
			You("order %s to follow you.", mon_nam(mtmp));
		}
	} else pline("There is no target there.");
	return 0;
}

#endif /* OVLB */

/*do.c*/
