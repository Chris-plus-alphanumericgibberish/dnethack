/*	SCCS Id: @(#)dothrow.c	3.4	2003/12/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* Contains code for 't' (throw) */

#include "hack.h"
#include "edog.h"
#include "xhity.h"

extern void NDECL(autoquiver);
extern int FDECL(gem_accept, (struct monst *, struct obj *));
extern int FDECL(throw_gold, (struct obj *));
extern void FDECL(check_shop_obj, (struct obj *, XCHAR_P, XCHAR_P, BOOLEAN_P));
extern void FDECL(breakobj, (struct obj *, XCHAR_P, XCHAR_P, BOOLEAN_P, BOOLEAN_P));
extern void FDECL(breakmsg, (struct obj *, BOOLEAN_P));
STATIC_DCL boolean FDECL(mhurtle_step, (genericptr_t,int,int));
extern boolean FDECL(quest_arti_hits_leader, (struct obj *, struct monst *));


static NEARDATA const char toss_objs[] =
	{ ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, WEAPON_CLASS, 0 };
/* different default choices when wielding a sling (gold must be included) */
static NEARDATA const char bullets[] =
	{ ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, GEM_CLASS, 0 };


//definition of an extern in you.h
boolean barage=FALSE;
#define THROW_UWEP 	1
#define THROW_USWAPWEP 	2

STATIC_OVL const char * const Ronnie_ray_gun[] = {
	"When you can't make them see the light, make them feel the heat.",
	"Every man must be free to become whatever God intends he should become.",
	"There you go again.",
	"Before I refuse to take your questions, I have an opening statement.",
	"If you're explaining, you're losing.",
	"As government expands, liberty contracts.",
	"Each individual is accountable for his actions.",
	"Government's first duty is to protect the people, not run their lives.",
	"The ultimate determinate in the struggle now going on for the world will not be bombs and rockets but a test of wills and ideas.",
	"If we lose freedom here, there is no place to escape to.",
	"This is the last stand on Earth.",
	"You and I have a rendezvous with destiny.",
	"We will preserve for our children this, the last best hope of man on earth, or we will sentence them to take the first step into a thousand years of darkness.",
	"Trust, but verify.",
	"The bombing begins in five minutes.",
	"Some people wonder all their lives if they've made a difference. The Marines don't have that problem.",
	"History teaches that war begins when governments believe the price of aggression is cheap.",
	"Freedom is never more than one generation away from extinction.",
	"Our people look for a cause to believe in.",
	"Today we did what we had to do.",
	"They counted on us to be passive. They counted wrong.",
	"Of the four wars in my lifetime, none came about because we were too strong.",
	"If it moves, tax it. If it keeps moving, regulate it. And if it stops moving, subsidize it.",
	"Tear down this wall.",
	"Open this gate!",
	"Come here to this gate!",
	"There are no easy answers, but there are simple answers.",
	"We must have the courage to do what we know is morally right.",
	"We are never defeated unless we give up on God.",
	"We will always remember.",
	"We will always be proud.",
	"We will always be prepared, so we will always be free.",
	"It is a weapon our adversaries in today's world do not have.",
	"They say the world has become too complex for simple answers. They are wrong.",
	"Don't be afraid to see what you see.",
	"Concentrated power has always been the enemy of liberty."
};


//ifdef FIREARMS
int
zap_raygun(raygun, shots, shotlimit)
struct obj *raygun;
int shots, shotlimit;
{
	int cost;
	boolean clicky = FALSE;
	
	if(raygun->altmode == AD_DISN) cost = 15;
	else if(raygun->altmode == AD_DEAD) cost = 10;
	else if(raygun->altmode == AD_FIRE) cost = 2;
	else cost = 1;
	
	if(raygun->ovar1 < shots*cost){
		shots = raygun->ovar1/cost;
		clicky = TRUE;
	}
	
	if(!u.dx && !u.dy){
		shotlimit = 1;
		if(u.dz > 0){
			raygun->ovar1 -= cost;
			if(raygun->altmode == AD_DISN){
				if (dighole(FALSE)){
					Your("raygun disintegrated the floor!");
					if(!Blind && !resists_blnd(&youmonst)) {
						You("are blinded by the flash!");
						make_blinded((long)rnd(50),FALSE);
						if (!Blind) Your1(vision_clears);
					}
					return 1;
				} else {
					struct engr *oep;
					if(!Blind){
						pline("A brilliant beam shoots from the raygun and burns into the %s!",surface(u.ux, u.uy));
						make_blinded((long)rnd(50),FALSE);
					}
					oep = engr_at(u.ux,u.uy);
					if(!oep){
						make_engr_at(u.ux, u.uy,	"", moves, DUST);
						oep = engr_at(u.ux,u.uy);
					}
					oep->ward_id = (!Hallucination || rn2(100)) ? 1 : randHaluWard();
					oep->halu_ward = TRUE;
					oep->ward_type = BURN;
					oep->complete_wards = 1;
				}
			} else if(raygun->altmode == AD_DEAD){
				if (!Blind) {
				   pline("The bugs on the %s stop moving!", surface(u.ux, u.uy));
				}
			} else if(raygun->altmode == AD_FIRE){
					struct engr *oep;
					if(!Blind){
						pline("A heat ray shoots from the raygun and melts into the %s!",surface(u.ux, u.uy));
					} else {
						pline("You feel the raygun heat up.");
					}
					oep = engr_at(u.ux,u.uy);
					if(!oep){
						make_engr_at(u.ux, u.uy,	"", moves, DUST);
						oep = engr_at(u.ux,u.uy);
					}
					oep->ward_id = (!Hallucination || rn2(100)) ? 1 : randHaluWard();
					oep->halu_ward = TRUE;
					oep->ward_type = BURN;
					oep->complete_wards = 1;
			} else {
				if (!Blind) {
				   pline("The bugs on the %s stop moving!", surface(u.ux, u.uy));
				}
			}
			return 1;
		} else {
			if(Hallucination) pline1(Ronnie_ray_gun[rn2(SIZE(Ronnie_ray_gun))]);
			raygun->ovar1 -= cost;
			buzz(raygun->altmode, WEAPON_CLASS, TRUE, 6, u.ux, u.uy, u.dx, u.dy, 1,0);
			return 1;
		}
	}
	
	if (shots > 0 && (shots > 1 || shotlimit > 0) && !Hallucination) {
		You("fire %d %s.",
		shots,	/* (might be 1 if player gave shotlimit) */
		(shots == 1) ? "ray" : "rays");
	}
	
	while(shots){
		if(Hallucination) pline1(Ronnie_ray_gun[rn2(SIZE(Ronnie_ray_gun))]);
		raygun->ovar1 -= cost;
		buzz(raygun->altmode, WEAPON_CLASS, TRUE, 6, u.ux, u.uy, u.dx, u.dy, objects[(raygun->otyp)].oc_range, 0);
		shots--;
	}
	
	if(clicky){
		You("push the firing stud, but nothing happens.");
	}
	return 1;
}

/* KMH -- Automatically fill quiver */
/* Suggested by Jeffrey Bay <jbay@convex.hp.com> */
void
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
			 otmp->obj_material == GLASS)) {
		if (uslinging())
		    oammo = otmp;
		else if (ammo_and_launcher(otmp, uswapwep))
		    altammo = otmp;
		else if (!omisc)
		    omisc = otmp;
	    } else if (otmp->oclass == GEM_CLASS) {
		;	/* skip non-rock gems--they're ammo but
			   player has to select them explicitly */
	    } else if (ammo_and_launcher(otmp, uwep)) {
		    /* Ammo matched with launcher (bow and arrow, crossbow and bolt) */
		    oammo = otmp;
		} else if (ammo_and_launcher(otmp, uswapwep)) {
			/* Ammo matched with launcher in swapwep */
		    altammo = otmp;
		} else if (is_ammo(otmp)) {
		    /* Mismatched ammo (no better than an ordinary weapon) */
		    omisc = otmp;
	    } else if (is_missile(otmp)) {
		/* Missile (dart, shuriken, etc.) */
		omissile = otmp;
	    } else if (otmp->oclass == WEAPON_CLASS && throwing_weapon(otmp)) {
		/* Ordinary weapon */
		if ((objects[otmp->otyp].oc_skill == P_DAGGER ||
			objects[otmp->otyp].oc_skill == P_SPEAR)
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
		bad_rock(&youmonst,u.ux,y) && bad_rock(&youmonst,x,u.uy)) {
	    boolean too_much = (invent && (inv_weight() + weight_cap() > 600));
	    /* Move at a diagonal. */
	    if (bigmonst(youracedata) || too_much) {
		You("%sget forcefully wedged into a crevice.",
			too_much ? "and all your belongings " : "");
		losehp(rnd(2+*range), "wedging into a narrow crevice", KILLED_BY);
		return FALSE;
	    }
	}
    }

    if ((mon = m_at(x, y)) != 0) {
	You("bump into %s.", a_monnam(mon));
	wakeup(mon, FALSE);
	return FALSE;
    }
    if ((u.ux - x) && (u.uy - y) &&
	bad_rock(&youmonst,u.ux,y) && bad_rock(&youmonst,x,u.uy)) {
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

    nomul(-(range/4+1), "moving through the air");
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

void
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
  
int
gem_accept(mon, obj)
register struct monst *mon;
register struct obj *obj;
{
	char buf[BUFSZ];
	boolean is_buddy = sgn(mon->data->maligntyp) == sgn(u.ualign.type);
	boolean is_gem = obj->obj_material == GEMSTONE;
	int ret = 0;
	static NEARDATA const char nogood[] = " is not interested in your junk.";
	static NEARDATA const char acceptgift[] = " accepts your gift.";
	static NEARDATA const char maybeluck[] = " hesitatingly";
	static NEARDATA const char noluck[] = " graciously";
	static NEARDATA const char addluck[] = " gratefully";
	static NEARDATA const char isartifact[] = " drops it in shock!";

	Strcpy(buf,Monnam(mon));
	mon->mpeaceful = 1;
	mon->mavenge = 0;
	
	if(obj->oartifact){
		Strcat(buf,isartifact);
		goto nopick;
	}
	
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
void
breakobj(obj, x, y, hero_caused, from_invent)
struct obj *obj;
xchar x, y;		/* object location (ox, oy may not be right) */
boolean hero_caused;	/* is this the hero's fault? */
boolean from_invent;
{
	switch (obj->oclass == POTION_CLASS ? POT_WATER : obj->otyp) {
		case MIRROR:
			if (hero_caused){
			    if((u.specialSealsActive&SEAL_NUDZIRATH)) change_luck(+2);
				else change_luck(-2);
			}
			break;
		case POT_WATER:		/* really, all potions */
			if (obj->otyp == POT_OIL && obj->lamplit) {
			    splatter_burning_oil(x,y);
			} else if (distu(x,y) <= 2) {
			    if (!breathless(youracedata) || haseyes(youracedata)) {
				if (obj->otyp != POT_WATER) {
					if (!breathless(youracedata))
			    		     /* [what about "familiar odor" when known?] */
					    You("smell a peculiar odor...");
					else {
					    int numeyes = eyecount(youracedata);
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
	if (is_shatterable(obj) && !obj->oerodeproof)
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

void
breakmsg(obj, in_view)
struct obj *obj;
boolean in_view;
{
	const char *to_pieces;

	to_pieces = "";
	switch (obj->oclass == POTION_CLASS ? POT_WATER : obj->otyp) {
		default: /* misc glass item, including glass or crystal wand */
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

int
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
		if (u.dz < 0 && !Weightless &&
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
