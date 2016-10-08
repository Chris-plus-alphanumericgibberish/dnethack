/*	SCCS Id: @(#)mthrowu.c	3.4	2003/05/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "mfndpos.h" /* ALLOW_M */

STATIC_DCL int FDECL(drop_throw,(struct monst *, struct obj *,BOOLEAN_P,int,int));

#define URETREATING(x,y) (distmin(u.ux,u.uy,x,y) > distmin(u.ux0,u.uy0,x,y))

#define POLE_LIM 5	/* How far monsters can use pole-weapons */

#ifndef OVLB

STATIC_DCL const char *breathwep[];

#else /* OVLB */

/*
 * Keep consistent with breath weapons in zap.c, and AD_* in monattk.h.
 */
STATIC_OVL NEARDATA const char *breathwep[] = {
				"fragments",
				"fire",
				"frost",
				"sleep gas",
				"a disintegration blast",
				"lightning",
				"poison gas",
				"acid",
				"strange breath #8",
				"strange breath #9"
};

int destroy_thrown = 0; /*state variable, if nonzero drop_throw always destroys object.  This is necessary 
						 because the throw code doesn't report the identity of the thrown object, so it can only
						 be destroyed in the throw code itself */
int bypassDR=0;

/* hero is hit by something other than a monster */
int
thitu(tlev, dam, obj, name, burn)
int tlev, dam;
struct obj *obj;
const char *name;	/* if null, then format `obj' */
boolean burn;
{
	const char *onm, *knm;
	boolean is_acid;
	int kprefix = KILLED_BY_AN;
	char onmbuf[BUFSZ], knmbuf[BUFSZ];

	if (!name) {
	    if (!obj) panic("thitu: name & obj both null?");
	    name = strcpy(onmbuf,
			 (obj->quan > 1L) ? doname(obj) : mshot_xname(obj));
	    knm = strcpy(knmbuf, killer_xname(obj));
	    kprefix = KILLED_BY;  /* killer_name supplies "an" if warranted */
	} else {
	    knm = name;
	    /* [perhaps ought to check for plural here to] */
	    if (!strncmpi(name, "the ", 4) ||
		    !strncmpi(name, "an ", 3) ||
		    !strncmpi(name, "a ", 2)) kprefix = KILLED_BY;
	}
	onm = (obj && obj_is_pname(obj)) ? the(name) :
			    (obj && obj->quan > 1L) ? name : an(name);
	is_acid = (obj && obj->otyp == ACID_VENOM);

	if(uwep && is_lightsaber(uwep) && uwep->lamplit && P_SKILL(weapon_type(uwep)) >= P_BASIC){
		if(P_SKILL(FFORM_SHII_CHO) >= P_BASIC){
			if(u.fightingForm == FFORM_SHII_CHO || 
				(u.fightingForm == FFORM_SHIEN && (!uarm || is_light_armor(uarm)))
			) use_skill(FFORM_SHIEN,1);
		}
	}
	
	if(burn){
		You("burn %s out of the %s!", onm, (Underwater || Is_waterlevel(&u.uz)) ? "water" : "air");
		return 0;
	}
	
	if((bypassDR && (AC_VALUE(base_uac()) + tlev <= rnd(20))) || (!bypassDR && (AC_VALUE(u.uac) + tlev <= rnd(20)))) {
		if(Blind || !flags.verbose) pline("It misses.");
		else You("are almost hit by %s.", onm);
		return(0);
	} else {
		
		if(bypassDR && base_uac() < 0) dam -= AC_VALUE(base_uac());
		else if(!bypassDR && u.uac < 0) dam -= AC_VALUE(u.uac);
		
		if(dam < 1) dam = 1;
		
		if(Blind || !flags.verbose) You("are hit!");
		else You("are hit by %s%s", onm, exclam(dam));
		
		if(Reflecting && (obj->otyp == BLASTER_BOLT || obj->otyp == HEAVY_BLASTER_BOLT || obj->otyp == LASER_BEAM)){
			(void) ureflects("But %s reflects from your %s!", "it");
			return -1;
		}

		if (obj && (objects[obj->otyp].oc_material == SILVER || arti_silvered(obj)) &&
				!(is_lightsaber(obj) && obj->lamplit) &&
				!(u.sealsActive&SEAL_EDEN)
				&& hates_silver(youracedata)) {
			// dam += rnd(20);
			pline_The("silver sears your flesh!");
			exercise(A_CON, FALSE);
		}
		if (obj && (objects[obj->otyp].oc_material == IRON) &&
				!(is_lightsaber(obj) && obj->lamplit)
				&& hates_iron(youracedata)) {
			// dam += rnd(20);
			pline_The("cold-iron sears your flesh!");
			exercise(A_CON, FALSE);
		}
		if (obj && (objects[obj->otyp].oc_material == SILVER || arti_silvered(obj))
				&& hates_unholy(youracedata)) {
			// dam += rnd(20);
			pline_The("curse sears your flesh!");
			exercise(A_CON, FALSE);
		}
		if (is_acid && Acid_resistance)
			pline("It doesn't seem to hurt you.");
		else {
			if (is_acid) pline("It burns!");
			if (Half_physical_damage) dam = (dam+1) / 2;
			losehp(dam, knm, kprefix);
			exercise(A_STR, FALSE);
		}
		return(1);
	}
}

/* Be sure this corresponds with what happens to player-thrown objects in
 * dothrow.c (for consistency). --KAA
 * Returns 0 if object still exists (not destroyed).
 */

STATIC_OVL int
drop_throw(mon, obj, ohit, x, y)
register struct monst *mon;
register struct obj *obj;
boolean ohit;
int x,y;
{
	struct obj *mwep = (struct obj *) 0;
	
	int retvalu = 1;
	int create, autopickup = 0;
	struct monst *mtmp;
	struct trap *t;

	if (mon) mwep = MON_WEP(mon);

	if (breaks(obj, x, y)) return 1;
	if (destroy_thrown || (ohit && obj->oartifact == ART_HOUCHOU) //destroy_thrown is a state variable set in firemu
//#ifdef FIREARMS
		    /* WAC -- assume monsters don't throw without 
		    	using the right propellor */
                    || is_bullet(obj)
//#endif
	){
		if(ohit && x == u.ux && y == u.uy && obj->otyp == LOADSTONE && !rn2(3) ){
			create = 1;
			autopickup = 1;
		}
		else create = 0;
	}
	else if (ohit && (is_multigen(obj) || obj->otyp == ROCK))
		create = !rn2(3);
	else create = 1;

//#ifdef FIREARMS
	/* Detonate rockets */
	if (is_grenade(obj)) {
		if (!ohit) {
			create = 1; /* Don't destroy */
			arm_bomb(obj, FALSE);
		} else {
			grenade_explode(obj, bhitpos.x, bhitpos.y, FALSE, 0);
			obj = (struct obj *)0;
		}
	} else if (obj->otyp == BLASTER_BOLT) {
		explode(bhitpos.x, bhitpos.y, flags.mon_moving ? -8 : 8, d(3,6),
		    0, EXPL_RED);
	} else if (obj->otyp == HEAVY_BLASTER_BOLT) {
		explode(bhitpos.x, bhitpos.y, flags.mon_moving ? -8 : 8, d(3,10),
		    0, EXPL_FIERY);
	} else if (objects[obj->otyp].oc_dir & EXPLOSION) {
	    	if (cansee(bhitpos.x,bhitpos.y)) 
	    		pline("%s explodes in a ball of fire!", Doname2(obj));
		explode(bhitpos.x, bhitpos.y, flags.mon_moving ? -ZT_SPELL(ZT_FIRE) : ZT_SPELL(ZT_FIRE), d(3,8),
		    WEAPON_CLASS, EXPL_FIERY);
	}
//#endif
	// if (create && !((mtmp = m_at(x, y)) && (mtmp->mtrapped) &&
			// (t = t_at(x, y)) && ((t->ttyp == PIT) ||
			// (t->ttyp == SPIKED_PIT)))) {
	mtmp = m_at(x, y);
	t = t_at(x, y);
	if (create) {
		int objgone = 0;

		if (down_gate(x, y) != -1)
			objgone = ship_object(obj, x, y, FALSE);
		if (!objgone) {
			if (!flooreffects(obj,x,y,"fall")) { /* don't double-dip on damage */
			    place_object(obj, x, y);
			    if (!mtmp && x == u.ux && y == u.uy)
					mtmp = &youmonst;
			    if (mtmp && ohit)
					passive_obj(mtmp, obj, (struct attack *)0);
				if(mtmp == &youmonst && obj->otyp == LOADSTONE && (!rn2(3) || autopickup) ){
					pickup_object(obj,1,FALSE);
				}
				else stackobj(obj);
			    retvalu = 0;
			}
		}
	} else if (obj) obfree(obj, (struct obj*) 0);
	return retvalu;
}

#endif /* OVLB */
#ifdef OVL1

/* an object launched by someone/thing other than player attacks a monster;
   return 1 if the object has stopped moving (hit or its range used up) */
int
ohitmon(mon, mtmp, otmp, range, verbose)
struct monst *mon;  /* monster thrower (if applicable) */
struct monst *mtmp;	/* accidental target */
struct obj *otmp;	/* missile; might be destroyed by drop_throw */
int range;		/* how much farther will object travel if it misses */
			/* Use -1 to signify to keep going even after hit, */
			/* unless its gone (used for rolling_boulder_traps) */
boolean verbose;  /* give message(s) even when you can't see what happened */
{
	int damage, tmp;
	boolean vis, ismimic;
	int objgone = 1;

	ismimic = mtmp->m_ap_type && mtmp->m_ap_type != M_AP_MONSTER;
	vis = cansee(bhitpos.x, bhitpos.y);
	
	if(bypassDR) tmp = 5 + base_mac(mtmp) + omon_adj(mtmp, otmp, FALSE);
	else tmp = 5 + find_mac(mtmp) + omon_adj(mtmp, otmp, FALSE);
	if (tmp < rnd(20)) {
	    if (!ismimic) {
		if (vis) miss(distant_name(otmp, mshot_xname), mtmp);
		else if (verbose) pline("It is missed.");
	    }
	    if (!range) { /* Last position; object drops */
	        if (is_pole(otmp)) return 1;

		(void) drop_throw(mon, otmp, 0, mtmp->mx, mtmp->my);
		return 1;
	    }
	} else if (otmp->oclass == POTION_CLASS) {
	    if (ismimic) seemimic(mtmp);
	    mtmp->msleeping = 0;
	    if (vis) otmp->dknown = 1;
	    potionhit(mtmp, otmp, FALSE);
	    return 1;
	} else {
	    damage = dmgval(otmp, mtmp, 0);
		if(mon == &youmonst && damage > 1){
			if(otmp->otyp >= LUCKSTONE && otmp->otyp <= ROCK && otmp->ovar1) use_skill((int)otmp->ovar1,1);
			else if(objects[otmp->otyp].oc_skill != P_NONE) use_skill(objects[otmp->otyp].oc_skill,1);
		}
	    if (otmp->otyp == ACID_VENOM && resists_acid(mtmp)) damage = 0;
	    if (ismimic) seemimic(mtmp);
	    mtmp->msleeping = 0;
	    if (vis) hit(distant_name(otmp,mshot_xname), mtmp, exclam(damage));
	    else if (verbose) pline("%s is hit%s", Monnam(mtmp), exclam(damage));

	    if ((otmp->opoisoned && is_poisonable(otmp)) || arti_poisoned(otmp) || 
			otmp->oartifact == ART_WEBWEAVER_S_CROOK || otmp->oartifact == ART_MOONBEAM
		) {
			if(otmp->opoisoned & OPOISON_BASIC || arti_poisoned(otmp)){
				if (resists_poison(mtmp)) {
					if (vis) pline_The("poison doesn't seem to affect %s.",
						   mon_nam(mtmp));
				} else {
					if (rn2(30)) {
						damage += rnd(6);
					} else {
					if (vis) pline_The("poison was deadly...");
						damage = mtmp->mhp;
					}
				}
				if(!rn2(20)) otmp->opoisoned &= ~OPOISON_BASIC;
			}
			if(otmp->opoisoned & OPOISON_FILTH || otmp->oartifact == ART_SUNBEAM){
				if (resists_sickness(mtmp)) {
					if (vis) pline_The("filth doesn't seem to affect %s.",
						   mon_nam(mtmp));
				} else {
					if (rn2(30)) {
						damage += rnd(12);
					} else {
					if (vis) pline_The("tainted filth was deadly...");
						damage = mtmp->mhp;
					}
				}
				if(!rn2(20)) otmp->opoisoned &= ~OPOISON_FILTH;
			}
			if(otmp->opoisoned & OPOISON_SLEEP || otmp->oartifact == ART_WEBWEAVER_S_CROOK || otmp->oartifact == ART_MOONBEAM){
				if (resists_poison(mtmp) || resists_sleep(mtmp) || (otmp->oartifact != ART_MOONBEAM && rn2(10))){
					if (vis) pline_The("drug doesn't seem to affect %s.",
						   mon_nam(mtmp));
				} else {
					if (sleep_monst(mtmp, rnd(12), POTION_CLASS)) {
						pline("%s falls asleep.", Monnam(mtmp));
						slept_monst(mtmp);
					}
				}
				if(!rn2(20)) otmp->opoisoned &= ~OPOISON_SLEEP;
			}
			if(otmp->opoisoned & OPOISON_BLIND || otmp->oartifact == ART_WEBWEAVER_S_CROOK){
				if (resists_poison(mtmp)) {
					if (vis) pline_The("poison doesn't seem to affect %s.",
						   mon_nam(mtmp));
				} else {
					if (rn2(10)) {
						damage += rnd(3);
					} else {
						if(haseyes(mtmp->data)) {
							if (vis) pline("It seems %s has gone blind!", mon_nam(mtmp));
							register int btmp = 64 + rn2(32) +
							rn2(32) * !resist(mtmp, POTION_CLASS, 0, NOTELL);
							btmp += mtmp->mblinded;
							mtmp->mblinded = min(btmp,127);
							mtmp->mcansee = 0;
						}
					}
				}
				if(!rn2(20)) otmp->opoisoned &= ~OPOISON_BLIND;
			}
			if(otmp->opoisoned & OPOISON_PARAL || otmp->oartifact == ART_WEBWEAVER_S_CROOK){
				if (resists_poison(mtmp)) {
					if (vis) pline_The("venom doesn't seem to affect %s.",
						   mon_nam(mtmp));
				} else {
					if (rn2(10)) {
						damage += rnd(6);
					} else {
						damage += 6;
						if (mtmp->mcanmove) {
							mtmp->mcanmove = 0;
							mtmp->mfrozen = rnd(25);
						}
					}
				}
				if(!rn2(20)) otmp->opoisoned &= ~OPOISON_PARAL;
			}
			if(otmp->opoisoned & OPOISON_AMNES){
				if (!mindless(mtmp->data) && !rn2(10)){
					if (vis) pline("%s looks around as if awakening from a dream.",
						   Monnam(mtmp));
					mtmp->mtame = FALSE;
					mtmp->mpeaceful = TRUE;
				}
				if(!rn2(20)) otmp->opoisoned &= ~OPOISON_AMNES;
			}
	    }
	    if ( (objects[otmp->otyp].oc_material == SILVER || arti_silvered(otmp)) &&
			!(is_lightsaber(otmp) && otmp->lamplit) &&
		    hates_silver(mtmp->data)
		) {
			if (vis) pline_The("silver sears %s flesh!",
					s_suffix(mon_nam(mtmp)));
			else if (verbose) pline("Its flesh is seared!");
	    }
	    if ( (objects[otmp->otyp].oc_material == IRON) &&
			!(is_lightsaber(otmp) && otmp->lamplit) &&
		    hates_iron(mtmp->data)
		) {
			if (vis) pline_The("cold-iron sears %s flesh!",
					s_suffix(mon_nam(mtmp)));
			else if (verbose) pline("Its flesh is seared!");
	    }
	    if ( (otmp->cursed) &&
		    hates_unholy(mtmp->data)
		) {
			if (vis) pline_The("curse sears %s flesh!",
					s_suffix(mon_nam(mtmp)));
			else if (verbose) pline("Its flesh is seared!");
	    }
	    if (otmp->otyp == ACID_VENOM && cansee(mtmp->mx,mtmp->my)) {
		if (resists_acid(mtmp)) {
		    if (vis || verbose)
			pline("%s is unaffected.", Monnam(mtmp));
		    damage = 0;
		} else {
		    if (vis) pline_The("acid burns %s!", mon_nam(mtmp));
		    else if (verbose) pline("It is burned!");
		}
	    }
	    mtmp->mhp -= damage;
	    if (mtmp->mhp < 1) {
		if (vis || verbose)
		    pline("%s is %s!", Monnam(mtmp),
			(nonliving(mtmp->data) || !canspotmon(mtmp))
			? "destroyed" : "killed");
		/* don't blame hero for unknown rolling boulder trap */
		if (!flags.mon_moving &&
		    (!is_boulder(otmp) || range >= 0 || !otmp->otrapped))
		    xkilled(mtmp,0);
		else mondied(mtmp);
	    }

	    if (can_blnd((struct monst*)0, mtmp,
		    (uchar)(otmp->otyp == BLINDING_VENOM ? AT_SPIT : AT_WEAP),
		    otmp)) {
		if (vis && mtmp->mcansee)
		    pline("%s is blinded by %s.", Monnam(mtmp), the(xname(otmp)));
		mtmp->mcansee = 0;
		tmp = (int)mtmp->mblinded + rnd(25) + 20;
		if (tmp > 127) tmp = 127;
		mtmp->mblinded = tmp;
	    }

	    if (is_pole(otmp))
	        return 1;

	    objgone = drop_throw(mon, otmp, 1, bhitpos.x, bhitpos.y);
	    if (!objgone && range == -1) {  /* special case */
		    obj_extract_self(otmp); /* free it for motion again */
		    return 0;
	    }
	    return 1;
	}
	return 0;
}

void
m_throw(mon, x, y, dx, dy, range, obj, verbose)
	register struct monst *mon;
	register int x,y,dx,dy,range;		/* direction and range */
	register struct obj *obj;
	register boolean verbose;
{
	register struct monst *mtmp;
	struct obj *singleobj;
	char sym = obj->oclass;
	int hitu, blindinc = 0;

	bhitpos.x = x;
	bhitpos.y = y;

	if (obj->quan == 1L) {
	    /*
	     * Remove object from minvent.  This cannot be done later on;
	     * what if the player dies before then, leaving the monster
	     * with 0 daggers?  (This caused the infamous 2^32-1 orcish
	     * dagger bug).
	     *
	     * VENOM is not in minvent - it should already be OBJ_FREE.
	     * The extract below does nothing.
	     */

	    /* not possibly_unwield, which checks the object's */
	    /* location, not its existence */
	    if (MON_WEP(mon) == obj) {
		    setmnotwielded(mon,obj);
		    MON_NOWEP(mon);
	    }
	    obj_extract_self(obj);
	    singleobj = obj;
	    obj = (struct obj *) 0;
	} else {
	    singleobj = splitobj(obj, 1L);
	    obj_extract_self(singleobj);
	}

	singleobj->owornmask = 0; /* threw one of multiple weapons in hand? */

	if ((singleobj->cursed || (mon != &youmonst && is_ammo(singleobj) && MON_WEP(mon) && MON_WEP(mon)->otyp == FLINTLOCK)) && (dx || dy) && !rn2(7)) {
		if(mon == &youmonst){
			if(is_ammo(singleobj))
				Your("weapon misfires!");
			else
				pline("%s as you throw it!",
				  Tobjnam(singleobj, "slip"));
		}
	    if(canseemon(mon) && flags.verbose) {
			if(is_ammo(singleobj))
				pline("%s misfires!", Monnam(mon));
			else
				pline("%s as %s throws it!",
				  Tobjnam(singleobj, "slip"), mon_nam(mon));
		}
	    dx = rn2(3)-1;
	    dy = rn2(3)-1;
	    /* check validity of new direction */
	    if (!dx && !dy) {
		(void) drop_throw(mon, singleobj, 0, bhitpos.x, bhitpos.y);
		return;
	    }
	}

	/* pre-check for doors, walls and boundaries.
	   Also need to pre-check for bars regardless of direction;
	   the random chance for small objects hitting bars is
	   skipped when reaching them at point blank range */
	if (!isok(bhitpos.x+dx,bhitpos.y+dy)
	    || IS_ROCK(levl[bhitpos.x+dx][bhitpos.y+dy].typ)
	    || closed_door(bhitpos.x+dx, bhitpos.y+dy)
	    || (levl[bhitpos.x + dx][bhitpos.y + dy].typ == IRONBARS &&
			(Is_illregrd(&u.uz) || hits_bars(&singleobj, bhitpos.x, bhitpos.y, 0, 0)))
	) {
		struct rm *room = &levl[bhitpos.x+dx][bhitpos.y+dy];
		boolean shopdoor=FALSE, shopwall=FALSE;
		if((closed_door(bhitpos.x+dx,bhitpos.y+dy) || room->typ == SDOOR) && 
			!artifact_door(bhitpos.x+dx,bhitpos.y+dy) && singleobj && 
			(singleobj->otyp == BLASTER_BOLT || singleobj->otyp == HEAVY_BLASTER_BOLT || singleobj->otyp == LASER_BEAM)
		) {
			if(cansee(bhitpos.x+dx,bhitpos.y+dy)) (singleobj->otyp == LASER_BEAM) ?
					pline("The %s cuts the door into chunks!", xname(singleobj)) : 
					pline("The door is blown to splinters by the impact!");
			if (*in_rooms(bhitpos.x+dx,bhitpos.y+dy,SHOPBASE)) {
				add_damage(bhitpos.x+dx,bhitpos.y+dy, !flags.mon_moving ? 400L : 0L);
				shopwall = TRUE;
			}
			if(!flags.mon_moving) watch_dig((struct monst *)0, bhitpos.x+dx,bhitpos.y+dy, TRUE);
			room->typ = DOOR;
			room->doormask = D_NODOOR;
			unblock_point(bhitpos.x+dx,bhitpos.y+dy); /* vision */
			doredraw();
		} else if(IS_WALL(room->typ) && may_dig(bhitpos.x+dx,bhitpos.y+dy) && singleobj && 
			((singleobj->otyp == LASER_BEAM) || (singleobj->otyp == BLASTER_BOLT && !rn2(20)) || (singleobj->otyp == HEAVY_BLASTER_BOLT && !rn2(5)))
		) {
			if(cansee(bhitpos.x+dx,bhitpos.y+dy)) (singleobj->otyp == LASER_BEAM) ?
					pline("The %s cuts the wall into chunks!", xname(singleobj)) : 
					pline("The wall blows apart from the impact!");
			if (*in_rooms(bhitpos.x+dx,bhitpos.y+dy,SHOPBASE)) {
				add_damage(bhitpos.x+dx,bhitpos.y+dy, !flags.mon_moving ? 200L : 0L);
				shopwall = TRUE;
			}
			if(!flags.mon_moving) watch_dig((struct monst *)0, bhitpos.x+dx,bhitpos.y+dy, TRUE);
			if (level.flags.is_cavernous_lev && !in_town(bhitpos.x+dx,bhitpos.y+dy)) {
				struct obj *otmp;
				room->typ = CORR;
				unblock_point(bhitpos.x+dx,bhitpos.y+dy); /* vision */
				otmp = mksobj_at(ROCK, bhitpos.x+dx,bhitpos.y+dy, TRUE, FALSE);
				otmp->quan = 20L+rnd(20);
				otmp->owt = weight(otmp);
				doredraw();
			} else {
				struct obj *otmp;
				room->typ = DOOR;
				room->doormask = D_NODOOR;
				unblock_point(bhitpos.x+dx,bhitpos.y+dy); /* vision */
				otmp = mksobj_at(ROCK, bhitpos.x+dx,bhitpos.y+dy, TRUE, FALSE);
				otmp->quan = 20L+rnd(20);
				otmp->owt = weight(otmp);
				doredraw();
			}
		} else if(isok(bhitpos.x+dx,bhitpos.y+dy) && IS_ROCK(room->typ) && may_dig(bhitpos.x+dx,bhitpos.y+dy) && singleobj && 
			(singleobj->otyp == LASER_BEAM)
		) {
			struct obj *otmp;
			if(cansee(bhitpos.x+dx,bhitpos.y+dy)) pline("The %s cuts the stone into chunks!", xname(singleobj));
			if (*in_rooms(bhitpos.x+dx,bhitpos.y+dy,SHOPBASE)) {
				add_damage(bhitpos.x+dx,bhitpos.y+dy, !flags.mon_moving ? 200L : 0L);
				shopwall = TRUE;
			}
			if(!flags.mon_moving) watch_dig((struct monst *)0, bhitpos.x+dx,bhitpos.y+dy, TRUE);
			room->typ = CORR;
			unblock_point(bhitpos.x+dx,bhitpos.y+dy); /* vision */
			otmp = mksobj_at(ROCK, bhitpos.x+dx,bhitpos.y+dy, TRUE, FALSE);
			otmp->quan = 20L+rnd(20);
			otmp->owt = weight(otmp);
			doredraw();
		} else if(isok(bhitpos.x+dx,bhitpos.y+dy) && (room->typ == IRONBARS) && singleobj && 
			(singleobj->otyp == LASER_BEAM)
		) {
			char numbars;
			struct obj *otmp;
			if(cansee(bhitpos.x+dx,bhitpos.y+dy)) pline("The %s cuts through the bars!", xname(singleobj));
			levl[bhitpos.x][bhitpos.y].typ = CORR;
			for(numbars = d(2,4)-1; numbars > 0; numbars--){
				otmp = mksobj_at(IRON_BAR, bhitpos.x, bhitpos.y, FALSE, FALSE);
				otmp->spe = 0;
				otmp->cursed = obj->blessed = FALSE;
			}
			newsym(bhitpos.x, bhitpos.y);
		}
		if(!flags.mon_moving && (shopdoor || shopwall)) (singleobj->otyp == LASER_BEAM) ?
					pay_for_damage("cut into", FALSE) : 
					pay_for_damage("blast into", FALSE);
	    if(singleobj) (void) drop_throw(mon, singleobj, 0, bhitpos.x, bhitpos.y); /*may have been broken by bars*/
	    return;
	}

	/* Note: drop_throw may destroy singleobj.  Since obj must be destroyed
	 * early to avoid the dagger bug, anyone who modifies this code should
	 * be careful not to use either one after it's been freed.
	 */
	if (sym) tmp_at(DISP_FLASH, obj_to_glyph(singleobj));
	while(range-- > 0) { /* Actually the loop is always exited by break */
		bhitpos.x += dx;
		bhitpos.y += dy;
		if ((mtmp = m_at(bhitpos.x, bhitpos.y)) != 0) {
			if((singleobj->otyp == LASER_BEAM || singleobj->otyp == BLASTER_BOLT || singleobj->otyp == HEAVY_BLASTER_BOLT) && mon_reflects(mtmp, (char *)0)){
				dx *= -1;
				dy *= -1;
		    } else if (ohitmon(mon, mtmp, singleobj, range, verbose))
				break;
		} else if (bhitpos.x == u.ux && bhitpos.y == u.uy) {
		    if (multi) nomul(0, NULL);
			
		    if (singleobj->oclass == GEM_CLASS &&
			    singleobj->otyp <= LAST_GEM+9 /* 9 glass colors */
			    && is_unicorn(youmonst.data)
			) {
				if (singleobj->otyp > LAST_GEM) {
					You("catch the %s.", xname(singleobj));
					You("are not interested in %s junk.",
					s_suffix(mon_nam(mon)));
					makeknown(singleobj->otyp);
					dropy(singleobj);
				} else {
					You("accept %s gift in the spirit in which it was intended.",
					s_suffix(mon_nam(mon)));
					(void)hold_another_object(singleobj,
					"You catch, but drop, %s.", xname(singleobj),
					"You catch:");
				}
				break;
		    }
		    if (singleobj->oclass == POTION_CLASS) {
				if (!Blind) singleobj->dknown = 1;
				potionhit(&youmonst, singleobj, FALSE);
				break;
		    }
		    switch(singleobj->otyp) {
			int dam, hitv;
			case EGG:
			    if (!touch_petrifies(&mons[singleobj->corpsenm])) {
				impossible("monster throwing egg type %d",
					singleobj->corpsenm);
				hitu = 0;
				break;
			    }
			    /* fall through */
			case CREAM_PIE:
			case BLINDING_VENOM:
				if(
				  !(singleobj->otyp == LASER_BEAM || singleobj->otyp == BLASTER_BOLT || singleobj->otyp == HEAVY_BLASTER_BOLT || singleobj->oartifact) && 
					uwep && is_lightsaber(uwep) && uwep->lamplit && 
						((u.fightingForm == FFORM_SHIEN && (!uarm || is_light_armor(uarm))) || 
						 (u.fightingForm == FFORM_SORESU && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm)))
						)
				){
					switch(min(P_SKILL(u.fightingForm), P_SKILL(weapon_type(uwep)))){
						case P_BASIC:
							if(rn2(100) < 33){
								thitu(4, 0, singleobj, (char *)0,TRUE);
								obfree(singleobj, (struct obj*) 0);
								delay_output();
								tmp_at(DISP_END, 0);
								return;
							}
						break;
						case P_SKILLED:
							if(rn2(100) < 66){
								thitu(4, 0, singleobj, (char *)0,TRUE);
								obfree(singleobj, (struct obj*) 0);
								delay_output();
								tmp_at(DISP_END, 0);
								return;
							}
						break;
						case P_EXPERT:
							thitu(4, 0, singleobj, (char *)0,TRUE);
							obfree(singleobj, (struct obj*) 0);
							delay_output();
							tmp_at(DISP_END, 0);
							return;
						break;
					}
				}
			    hitu = thitu(4+mon->m_lev, 0, singleobj, (char *)0, FALSE);
			    if(hitu>0) break;
			default:
			    dam = dmgval(singleobj, &youmonst, 0);
				if(!bypassDR && u.uac<0) dam += AC_VALUE(u.uac);
			    hitv = 3 - distmin(u.ux,u.uy, mon->mx,mon->my);
			    if (hitv < -4) hitv = (hitv+4)/2-4;
			    if (hitv < -8) hitv = (hitv+8)*2/3-8;
			    if (hitv < -12) hitv = (hitv+12)*3/4-12;
			    if (is_elf(mon->data) &&
				objects[singleobj->otyp].oc_skill == P_BOW) {
				hitv++;
					if (MON_WEP(mon) &&
					    MON_WEP(mon)->otyp == ELVEN_BOW)
					    hitv++;
					if(singleobj->otyp == ELVEN_ARROW) dam++;
				}
			    if (bigmonst(youmonst.data)) hitv++;
			    hitv += 4 + mon->m_lev + singleobj->spe;
			    if (dam < 1) dam = 1;
				/*FIXME:  incomplete initialization, monsters can't use artifact ranged weapons*/
//				if(/*hitu && */(singleobj->oartifact || ammo_and_launcher(singleobj, MON_WEP(mon))) ){
//					artifact_hit(mon, &youmonst, singleobj, &dam, d(1,20));
//				}//maybe this is ok.  It will give messages for hits that will ultimatly miss.
					//This is definitly a temporary setup.
				if(
				  !(singleobj->otyp == LASER_BEAM || singleobj->otyp == BLASTER_BOLT || singleobj->otyp == HEAVY_BLASTER_BOLT || singleobj->oartifact) && 
					uwep && is_lightsaber(uwep) && uwep->lamplit && 
						((u.fightingForm == FFORM_SHIEN && (!uarm || is_light_armor(uarm))) || 
						 (u.fightingForm == FFORM_SORESU && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm)))
						)
				){
					switch(min(P_SKILL(u.fightingForm), P_SKILL(weapon_type(uwep)))){
						case P_BASIC:
							if(rn2(100) < 33){
								thitu(4, 0, singleobj, (char *)0,TRUE);
								obfree(singleobj, (struct obj*) 0);
								delay_output();
								tmp_at(DISP_END, 0);
								return;
							}
						break;
						case P_SKILLED:
							if(rn2(100) < 66){
								thitu(4, 0, singleobj, (char *)0,TRUE);
								obfree(singleobj, (struct obj*) 0);
								delay_output();
								tmp_at(DISP_END, 0);
								return;
							}
						break;
						case P_EXPERT:
							thitu(4, 0, singleobj, (char *)0,TRUE);
							obfree(singleobj, (struct obj*) 0);
							delay_output();
							tmp_at(DISP_END, 0);
							return;
						break;
					}
				}
			    hitu = thitu(hitv, dam, singleobj, (char *)0, FALSE);
			}//close switch statement
			if(hitu < 0){
				boolean shienuse = FALSE;
				if(
					uwep && is_lightsaber(uwep) && uwep->lamplit && u.fightingForm == FFORM_SHIEN && (!uarm || is_light_armor(uarm))
				){
					switch(min(P_SKILL(u.fightingForm), P_SKILL(weapon_type(uwep)))){
						case P_BASIC:
							if(rn2(100) < 33){
								shienuse = TRUE;
							}
						break;
						case P_SKILLED:
							if(rn2(100) < 66){
								shienuse = TRUE;
							}
						break;
						case P_EXPERT:
							shienuse = TRUE;
						break;
					}
				}

				if(uwep && is_lightsaber(uwep) && uwep->lamplit && shienuse && getdir((char *)0) && (u.dx || u.dy)){
					dx = u.dx;
					dy = u.dy;
				} else {
					dx *= -1;
					dy *= -1;
				}
			}
		    if (hitu>0 && singleobj->opoisoned &&
				is_poisonable(singleobj)
			) {
				char onmbuf[BUFSZ], knmbuf[BUFSZ];

				Strcpy(onmbuf, xname(singleobj));
				Strcpy(knmbuf, killer_xname(singleobj));
				poisoned(onmbuf, A_STR, knmbuf, -10, singleobj->opoisoned);
			}
			if(hitu>0 &&
		       can_blnd((struct monst*)0, &youmonst,
				(uchar)(singleobj->otyp == BLINDING_VENOM ?
					AT_SPIT : AT_WEAP), singleobj)
			) {
				blindinc = rnd(25);
				if(singleobj->otyp == CREAM_PIE) {
					if(!Blind) pline("Yecch!  You've been creamed.");
					else pline("There's %s sticky all over your %s.",
						   something,
						   body_part(FACE));
				} else if(singleobj->otyp == BLINDING_VENOM) {
					int num_eyes = eyecount(youmonst.data);
					/* venom in the eyes */
					if(!Blind) pline_The("venom blinds you.");
					else Your("%s sting%s.",
						  (num_eyes == 1) ? body_part(EYE) :
							makeplural(body_part(EYE)),
						  (num_eyes == 1) ? "s" : "");
				}
		    }
		    if (hitu>0 && singleobj->otyp == EGG) {
				if (!Stone_resistance
					&& !(poly_when_stoned(youmonst.data) &&
					 polymon(PM_STONE_GOLEM))) {
					Stoned = 5;
					killer = (char *) 0;
				}
		    }
		    stop_occupation();
		    if (hitu>0 || !range) {
				(void) drop_throw(mon, singleobj, hitu, u.ux, u.uy);
				break;
		    }
		}
		if (!range	/* reached end of path */
			/* missile hits edge of screen */
			|| !isok(bhitpos.x+dx,bhitpos.y+dy)
			/* missile hits the wall */
			|| IS_ROCK(levl[bhitpos.x+dx][bhitpos.y+dy].typ)
			/* missile hit closed door */
			|| closed_door(bhitpos.x+dx, bhitpos.y+dy)
			/* missile might hit iron bars */
			|| (levl[bhitpos.x+dx][bhitpos.y+dy].typ == IRONBARS &&
				(Is_illregrd(&u.uz) || hits_bars(&singleobj, bhitpos.x, bhitpos.y, !rn2(5), 0)))
#ifdef SINKS
			/* Thrown objects "sink" */
			|| IS_SINK(levl[bhitpos.x][bhitpos.y].typ)
#endif
		) {
			struct rm *room = &levl[bhitpos.x+dx][bhitpos.y+dy];
			boolean shopdoor=FALSE, shopwall=FALSE;
			if(isok(bhitpos.x+dx,bhitpos.y+dy) && (closed_door(bhitpos.x+dx,bhitpos.y+dy) || room->typ == SDOOR) && 
				!artifact_door(bhitpos.x+dx,bhitpos.y+dy) && singleobj && 
				(singleobj->otyp == LASER_BEAM || singleobj->otyp == BLASTER_BOLT || singleobj->otyp == HEAVY_BLASTER_BOLT)
			) {
				if(cansee(bhitpos.x+dx,bhitpos.y+dy)) (singleobj->otyp == LASER_BEAM) ?
					pline("The %s cuts the door into chunks!", xname(singleobj)) : 
					pline("The door is blown to splinters by the impact!");
				if (*in_rooms(bhitpos.x+dx,bhitpos.y+dy,SHOPBASE)) {
					add_damage(bhitpos.x+dx,bhitpos.y+dy, !flags.mon_moving ? 400L : 0L);
					shopwall = TRUE;
				}
				if(!flags.mon_moving) watch_dig((struct monst *)0, bhitpos.x+dx,bhitpos.y+dy, TRUE);
				room->typ = DOOR;
				room->doormask = D_NODOOR;
				unblock_point(bhitpos.x+dx,bhitpos.y+dy); /* vision */
				doredraw();
			} else if(isok(bhitpos.x+dx,bhitpos.y+dy) && IS_WALL(room->typ) && may_dig(bhitpos.x+dx,bhitpos.y+dy) && singleobj && 
				((singleobj->otyp == LASER_BEAM) || (singleobj->otyp == BLASTER_BOLT && !rn2(20)) || (singleobj->otyp == HEAVY_BLASTER_BOLT && !rn2(5)))
			) {
				if(cansee(bhitpos.x+dx,bhitpos.y+dy)) (singleobj->otyp == LASER_BEAM) ?
					pline("The %s cuts the wall into chunks!", xname(singleobj)) : 
					pline("The wall blows apart from the impact!");
				if (*in_rooms(bhitpos.x+dx,bhitpos.y+dy,SHOPBASE)) {
					add_damage(bhitpos.x+dx,bhitpos.y+dy, !flags.mon_moving ? 200L : 0L);
					shopwall = TRUE;
				}
				if(!flags.mon_moving) watch_dig((struct monst *)0, bhitpos.x+dx,bhitpos.y+dy, TRUE);
				if (level.flags.is_cavernous_lev && !in_town(bhitpos.x+dx,bhitpos.y+dy)) {
					struct obj *otmp;
					room->typ = CORR;
					unblock_point(bhitpos.x+dx,bhitpos.y+dy); /* vision */
					otmp = mksobj_at(ROCK, bhitpos.x+dx,bhitpos.y+dy, TRUE, FALSE);
					otmp->quan = 20L+rnd(20);
					otmp->owt = weight(otmp);
					doredraw();
				} else {
					struct obj *otmp;
					room->typ = DOOR;
					room->doormask = D_NODOOR;
					unblock_point(bhitpos.x+dx,bhitpos.y+dy); /* vision */
					otmp = mksobj_at(ROCK, bhitpos.x+dx,bhitpos.y+dy, TRUE, FALSE);
					otmp->quan = 20L+rnd(20);
					otmp->owt = weight(otmp);
					doredraw();
				}
			} else if(isok(bhitpos.x+dx,bhitpos.y+dy) && IS_ROCK(room->typ) && may_dig(bhitpos.x+dx,bhitpos.y+dy) && singleobj && 
				(singleobj->otyp == LASER_BEAM)
			) {
				struct obj *otmp;
				if(cansee(bhitpos.x+dx,bhitpos.y+dy)) pline("The %s cuts the stone into chunks!", xname(singleobj));
				if (*in_rooms(bhitpos.x+dx,bhitpos.y+dy,SHOPBASE)) {
					add_damage(bhitpos.x+dx,bhitpos.y+dy, !flags.mon_moving ? 200L : 0L);
					shopwall = TRUE;
				}
				if(!flags.mon_moving) watch_dig((struct monst *)0, bhitpos.x+dx,bhitpos.y+dy, TRUE);
				room->typ = CORR;
				unblock_point(bhitpos.x+dx,bhitpos.y+dy); /* vision */
				otmp = mksobj_at(ROCK, bhitpos.x+dx,bhitpos.y+dy, TRUE, FALSE);
				otmp->quan = 20L+rnd(20);
				otmp->owt = weight(otmp);
				doredraw();
			} else if(isok(bhitpos.x+dx,bhitpos.y+dy) && (room->typ == IRONBARS) && singleobj && 
				(singleobj->otyp == LASER_BEAM)
			) {
				char numbars;
				struct obj *otmp;
				if(cansee(bhitpos.x+dx,bhitpos.y+dy)) pline("The %s cuts through the bars!", xname(singleobj));
				levl[bhitpos.x][bhitpos.y].typ = CORR;
				for(numbars = d(2,4)-1; numbars > 0; numbars--){
					otmp = mksobj_at(IRON_BAR, bhitpos.x, bhitpos.y, FALSE, FALSE);
					otmp->spe = 0;
					otmp->cursed = obj->blessed = FALSE;
				}
				newsym(bhitpos.x, bhitpos.y);
			}
			if(!flags.mon_moving && (shopdoor || shopwall)) (singleobj->otyp == LASER_BEAM) ?
				pay_for_damage("cut into", FALSE) : 
				pay_for_damage("blast into", FALSE);
		    if (singleobj) /* hits_bars might have destroyed it */
				(void) drop_throw(mon, singleobj, 0, bhitpos.x, bhitpos.y);
		    break;
		}
		tmp_at(bhitpos.x, bhitpos.y);
		delay_output();
	}
	tmp_at(bhitpos.x, bhitpos.y);
	delay_output();
	tmp_at(DISP_END, 0);

	if (blindinc) {
		u.ucreamed += blindinc;
		make_blinded(Blinded + (long)blindinc, FALSE);
		if (!Blind) Your1(vision_clears);
	}
}

#endif /* OVL1 */
#ifdef OVLB

/* Remove an item from the monster's inventory and destroy it. */
void
m_useup(mon, obj)
struct monst *mon;
struct obj *obj;
{
	if (obj->quan > 1L) {
		obj->quan--;
		obj->owt = weight(obj);
	} else {
		obj_extract_self(obj);
		possibly_unwield(mon, FALSE);
		if (obj->owornmask) {
		    mon->misc_worn_check &= ~obj->owornmask;
		    update_mon_intrinsics(mon, obj, FALSE, FALSE);
		}
		obfree(obj, (struct obj*) 0);
	}
}

#endif /* OVLB */
#ifdef OVL1

/* monster attempts ranged weapon attack against player */
void
thrwmu(mtmp)
struct monst *mtmp;
{
	struct obj *otmp, *mwep;
	xchar x, y;
	schar skill;
	int multishot;
	const char *onm;
	boolean mass_pistol = FALSE;

	if(mtmp->data->maligntyp < 0 && Is_illregrd(&u.uz)) return;
	if(mtmp->mux == 0 && mtmp->muy == 0) return;
	
	/* Rearranged beginning so monsters can use polearms not in a line */
	if (mtmp->weapon_check == NEED_WEAPON || !MON_WEP(mtmp)) {
	    if (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 8) {
			mtmp->combat_mode = HNDHND_MODE;
	        mtmp->weapon_check = NEED_HTH_WEAPON;
	        if(mon_wield_item(mtmp) != 0) return;
	    } else {
			mtmp->combat_mode = RANGED_MODE;
			mtmp->weapon_check = NEED_RANGED_WEAPON;
			/* mon_wield_item resets weapon_check as appropriate */
			if(mon_wield_item(mtmp) != 0) return;
		}
	}
	/* Pick a weapon */
	otmp = select_rwep(mtmp);
	if (!otmp) return;

	if (is_pole(otmp)) {
	    int dam, hitv;

	    if (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) > POLE_LIM ||
		    !couldsee(mtmp->mx, mtmp->my))
		return;	/* Out of range, or intervening wall */
		
		if(mtmp->mux != u.ux || mtmp->muy != u.uy){
			if(canseemon(mtmp)){
				onm = xname(otmp);
				pline("%s %s %s wildly.", Monnam(mtmp), otmp->otyp == AKLYS ? "throws" : "thrusts",
					  obj_is_pname(otmp) ? the(onm) : an(onm));
			}
			//figures out you aren't where it thought you were.
			mtmp->mux = 0;
			mtmp->muy = 0;
			return;
		}
		
	    if (canseemon(mtmp)) {
			onm = xname(otmp);
			pline("%s %s %s.", Monnam(mtmp), otmp->otyp == AKLYS ? "throws" : "thrusts",
				  obj_is_pname(otmp) ? the(onm) : an(onm));
	    }

	    dam = dmgval(otmp, &youmonst, 0);
	    hitv = 3 - distmin(u.ux,u.uy, mtmp->mx,mtmp->my);
		if (hitv < -4) hitv = (hitv+4)/2-4;
		if (hitv < -8) hitv = (hitv+8)*2/3-8;
		if (hitv < -12) hitv = (hitv+12)*3/4-12;
	    if (bigmonst(youmonst.data)) hitv++;
	    hitv += 4 + mtmp->m_lev + otmp->spe;
	    if (dam < 1) dam = 1;

	    (void) thitu(hitv, dam, otmp, (char *)0,FALSE);
	    stop_occupation();
	    return;
	}

	x = mtmp->mx;
	y = mtmp->my;
	/* If you are coming toward the monster, the monster
	 * should try to soften you up with missiles.  If you are
	 * going away, you are probably hurt or running.  Give
	 * chase, but if you are getting too far away, throw.
	 */
	if (!lined_up(mtmp) ||
		(URETREATING(x,y) &&
			rn2(BOLT_LIM - distmin(x,y,mtmp->mux,mtmp->muy))))
	    return;

	skill = objects[otmp->otyp].oc_skill;
	mwep = MON_WEP(mtmp);		/* wielded weapon */

	/* Multishot calculations */
	multishot = 1;
	if ((ammo_and_launcher(otmp, mwep) || is_blaster(otmp) || skill == P_DAGGER ||
		skill == -P_DART || skill == -P_SHURIKEN) && !mtmp->mconf) {
	    /* Assumes lords are skilled, princes are expert */
	    if (is_prince(mtmp->data)) multishot += 2;
	    else if (is_lord(mtmp->data)) multishot++;

		/*Increase skill related rof for heavy machine gun*/
		if(mwep && mwep->otyp == HEAVY_MACHINE_GUN) multishot *= 2;
		
	    switch (monsndx(mtmp->data)) {
	    case PM_RANGER:
		    multishot++;
		    break;
	    case PM_ROGUE:
		    if (skill == P_DAGGER) multishot++;
		    break;
	    case PM_NINJA:
	    case PM_SAMURAI:
		    if (otmp->otyp == YA && mwep &&
			mwep->otyp == YUMI) multishot++;
		    break;
	    default:
		break;
	    }
	    /* racial bonus */
	    if ((is_elf(mtmp->data) &&
		    otmp->otyp == ELVEN_ARROW &&
		    mwep && mwep->otyp == ELVEN_BOW) ||
		(is_orc(mtmp->data) &&
		    otmp->otyp == ORCISH_ARROW &&
		    mwep && mwep->otyp == ORCISH_BOW))
		multishot++;
		
	    if (multishot < 1) multishot = 1;
	    else multishot = rnd(multishot);
//#ifdef FIREARMS
	    // if (mwep && objects[mwep->otyp].oc_rof && is_launcher(mwep))
		// multishot += objects[mwep->otyp].oc_rof;
		if (((is_blaster(otmp) && otmp == mwep) || ammo_and_launcher(otmp, mwep))
			&& objects[(mwep->otyp)].oc_rof && mwep->otyp != RAYGUN && mwep->altmode != WP_MODE_SINGLE
		) {
			if(mwep->otyp == BFG){
				if(objects[(otmp)->otyp].w_ammotyp == WP_BULLET) multishot += 2*(objects[(mwep->otyp)].oc_rof);
				else if(objects[(otmp)->otyp].w_ammotyp == WP_SHELL) multishot += 1.5*(objects[(mwep->otyp)].oc_rof);
				else if(objects[(otmp)->otyp].w_ammotyp == WP_GRENADE) multishot += 1*(objects[(mwep->otyp)].oc_rof);
				else if(objects[(otmp)->otyp].w_ammotyp == WP_ROCKET) multishot += .5*(objects[(mwep->otyp)].oc_rof);
				else multishot += (objects[(mwep->otyp)].oc_rof);
			} else if (objects[(mwep->otyp)].oc_rof)
				multishot += (objects[(mwep->otyp)].oc_rof - 1);
			if (mwep->altmode == WP_MODE_BURST)
				multishot = ((multishot > 5) ? (multishot / 3) : 1);
		}
		/* single shot, don't add anything */
//#endif
	    if(is_blaster(otmp) && otmp == mwep){
			if((long)multishot > otmp->ovar1) multishot = (int)otmp->ovar1;
		} else if ((long)multishot > otmp->quan) multishot = (int)otmp->quan;
	    if (multishot < 1) multishot = 1;
	}
	
	if(is_blaster(otmp) && otmp == mwep){
		if(otmp->otyp == MASS_SHADOW_PISTOL) mass_pistol = TRUE;
		otmp = mksobj(otmp->otyp == CUTTING_LASER ? LASER_BEAM : 
				otmp->otyp == ARM_BLASTER ? HEAVY_BLASTER_BOLT : 
				otmp->otyp == MASS_SHADOW_PISTOL ? otmp->cobj->otyp : 
				BLASTER_BOLT, TRUE, FALSE);
		otmp->blessed = mwep->blessed;
		otmp->cursed = mwep->cursed;
		otmp->spe = mwep->spe;
		otmp->quan = multishot;
		mwep->ovar1 -= multishot;
	}

	if (canseemon(mtmp)) {
	    char onmbuf[BUFSZ];

	    if (multishot > 1) {
		/* "N arrows"; multishot > 1 implies otmp->quan > 1, so
		   xname()'s result will already be pluralized */
		Sprintf(onmbuf, "%d %s", multishot, xname(otmp));
		onm = onmbuf;
	    } else {
		/* "an arrow" */
		onm = singular(otmp, xname);
		onm = obj_is_pname(otmp) ? the(onm) : an(onm);
	    }
	    m_shot.s = ammo_and_launcher(otmp,mwep) ? TRUE : FALSE;
	    pline("%s %s %s!", Monnam(mtmp),
//#ifdef FIREARMS
		  m_shot.s ? is_bullet(otmp) ? "fires" : "shoots" : "throws",
		  onm);
//#else
//		  m_shot.s ? "shoots" : "throws", onm);
//#endif
	    m_shot.o = otmp->otyp;
	} else {
	    m_shot.o = STRANGE_OBJECT;	/* don't give multishot feedback */
	}

	m_shot.n = multishot;
	
	if(mass_pistol) set_destroy_thrown(TRUE); /* state variable, always destroy thrown */
	for (m_shot.i = 1; m_shot.i <= m_shot.n; m_shot.i++)
	    if(m_shot.s && objects[(mwep->otyp)].oc_range) m_throw(mtmp, mtmp->mx, mtmp->my, sgn(tbx), sgn(tby),
		    objects[(mwep->otyp)].oc_range, otmp,
		    TRUE);
		else m_throw(mtmp, mtmp->mx, mtmp->my, sgn(tbx), sgn(tby),
		    distmin(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy), otmp,
		    TRUE);
	if(mass_pistol) set_destroy_thrown(FALSE); /* state variable, always destroy thrown */
	m_shot.n = m_shot.i = 0;
	m_shot.o = STRANGE_OBJECT;
	m_shot.s = FALSE;
	if(mtmp->mux != u.ux || mtmp->muy != u.uy){
		//figures out you aren't where it thought you were
		mtmp->mux = 0;
		mtmp->muy = 0;
	}
	nomul(0, NULL);
}

extern int monstr[];

/* Find a target for a ranged attack. */
struct monst *
mfind_target(mtmp)
struct monst *mtmp;
{
    int dirx[8] = {0, 1, 1,  1,  0, -1, -1, -1},
        diry[8] = {1, 1, 0, -1, -1, -1,  0,  1};

    int dir, origdir = -1;
    int x, y, dx, dy;

    int i;

    struct monst *mat, *mret = (struct monst *)0, *oldmret = (struct monst *)0;

    boolean conflicted = Conflict && couldsee(mtmp->mx,mtmp->my) && 
						(distu(mtmp->mx,mtmp->my) <= BOLT_LIM*BOLT_LIM) &&
						!resist(mtmp, RING_CLASS, 0, 0);

    if (is_covetous(mtmp->data) && !mtmp->mtame)
    {
        /* find our mark and let him have it, if possible! */
        register int gx = STRAT_GOALX(mtmp->mstrategy),
                     gy = STRAT_GOALY(mtmp->mstrategy);
        register struct monst *mtmp2 = m_at(gx, gy);
	if (mtmp2 && 
		(mlined_up(mtmp, mtmp2, FALSE) ||
			attacktype(mtmp->data, AT_GAZE) ||
			attacktype(mtmp->data, AT_LRCH) ||
			attacktype(mtmp->data, AT_LNCK)
		)
	){
	    if(!(mtmp->data == &mons[PM_OONA] && resists_oona(mtmp2))) return mtmp2;
	}
	
#if 0
	if (!is_mplayer(mtmp->data)/* || !(mtmp->mstrategy & STRAT_NONE)*/)
	{
		return 0;
	}
#endif
    	if (!mtmp->mpeaceful && !conflicted &&
	   ((mtmp->mstrategy & STRAT_STRATMASK) == STRAT_NONE) &&
	    (lined_up(mtmp) ||
			attacktype(mtmp->data, AT_GAZE) ||
			attacktype(mtmp->data, AT_LRCH) ||
			attacktype(mtmp->data, AT_LNCK) )
		) {
        	if(!(mtmp->data == &mons[PM_OONA] && Oona_resistance)) return &youmonst;  /* kludge - attack the player first
				      if possible */
		}

	for (dir = 0; dir < 8; dir++)
		if (dirx[dir] == sgn(gx-mtmp->mx) &&
		    diry[dir] == sgn(gy-mtmp->my))
		    	break;

	if (dir == 8) {
	    tbx = tby = 0;
	    return 0;
	}

	origdir = -1;
    } else {
    	dir = rn2(8);
		origdir = -1;

    	if (!mtmp->mpeaceful && !conflicted && lined_up(mtmp)) {
        	if(!(mtmp->data == &mons[PM_OONA] && Oona_resistance)) return &youmonst;  /* kludge - attack the player first
				      if possible */
		}
    }

    for (; dir != origdir; dir = ((dir + 1) % 8))
    {
        if (origdir < 0) origdir = dir;

	mret = (struct monst *)0;

	x = mtmp->mx;
	y = mtmp->my;
	dx = dirx[dir];
	dy = diry[dir];
	for(i = 0; i < BOLT_LIM; i++)
	{
	    x += dx;
	    y += dy;

	    if (!isok(x, y) || !ZAP_POS(levl[x][y].typ) || closed_door(x, y))
	        break; /* off the map or otherwise bad */

	    if (!conflicted &&
	        ((mtmp->mpeaceful && (x == mtmp->mux && y == mtmp->muy)) ||
	        (mtmp->mtame && x == u.ux && y == u.uy)))
	    {
	        mret = oldmret;
	        break; /* don't attack you if peaceful */
	    }

	    if ((mat = m_at(x, y)))
	    {
	        /* i > 0 ensures this is not a close range attack */
	        if (mtmp->mtame && !mat->mtame &&
				acceptable_pet_target(mtmp, mat, TRUE) && i > 0
			) {
				if (((!oldmret) ||
					(monstr[monsndx(mat->data)] >
					monstr[monsndx(oldmret->data)])
					) && !(mtmp->data == &mons[PM_OONA] && resists_oona(mat))
				) mret = mat;
			}
			else if ((mm_aggression(mtmp, mat) & ALLOW_M)
				|| conflicted)
			{
				if (mtmp->mtame && !conflicted &&
					!acceptable_pet_target(mtmp, mat, TRUE))
				{
					mret = oldmret;
					break; /* not willing to attack in that direction */
				}

				/* Can't make some pairs work together
				   if they hate each other on principle. */
				if ((conflicted ||
					(!(mtmp->mtame && mat->mtame) || !rn2(5))) &&
				i > 0) {
					if (((!oldmret) ||
						(monstr[monsndx(mat->data)] >
						monstr[monsndx(oldmret->data)])
						) && !(mtmp->data == &mons[PM_OONA] && resists_oona(mat))
					)
						mret = mat;
				}
			}
			if (mtmp->mtame && mat->mtame)
			{
				mret = oldmret;
				break;  /* Not going to hit friendlies unless they
						   already hate them, as above. */
			}
		}
	}
	oldmret = mret;
    }
	
    if (mret != (struct monst *)0) {
	tbx = (mret->mx - mtmp->mx);
	tby = (mret->my - mtmp->my);
        return mret; /* should be the strongest monster that's not behind
	                a friendly */
    }

    /* Nothing lined up? */
    tbx = tby = 0;
    return (struct monst *)0;
}

#endif /* OVL1 */
#ifdef OVLB

int
spitmu(mtmp, mattk)		/* monster spits substance at you */
register struct monst *mtmp;
register struct attack *mattk;
{
	register struct obj *otmp;

	if(mtmp->data->maligntyp < 0 && Is_illregrd(&u.uz)) return;
	if(mtmp->mux == 0 && mtmp->muy == 0) return 0;

	if(mtmp->mcan) {
		if(mtmp->data==&mons[PM_ZETA_METROID]) //|| mtmp->data==&mons[PM_CRAZY_CHEMIST]) 
			mtmp->mcan=FALSE;
		else {
			if(flags.soundok)
			pline("A dry rattle comes from %s throat.",
			                      s_suffix(mon_nam(mtmp)));
		    return 0;
		}
	}
    if(mtmp->mspec_used){
		return 0;
	}
	if(lined_up(mtmp)) {
		switch (mattk->adtyp) {
		    case AD_WEBS:
				mtmp->mspec_used = d(2,6);
				otmp = mksobj(BALL_OF_WEBBING, TRUE, FALSE);
			break;
		    case AD_BLND:
		    case AD_DRST:
				otmp = mksobj(BLINDING_VENOM, TRUE, FALSE);
			break;
		    default:
			impossible("bad attack type in spitmu");
				/* fall through */
		    case AD_ACID:
				otmp = mksobj(ACID_VENOM, TRUE, FALSE);
				if(mattk->damn && mattk->damd) otmp->ovar1 = d(mattk->damn,mattk->damd);
			break;
		}
		if(!rn2(BOLT_LIM-distmin(mtmp->mx,mtmp->my,mtmp->mux,mtmp->muy))) {
//		    if (canseemon(mtmp))
//			if(mtmp->data==&mons[PM_CRAZY_CHEMIST])
//				pline("%s splashes venom!", Monnam(mtmp));
//			else
//				pline("%s spits venom!", Monnam(mtmp));
		    if (canseemon(mtmp)) pline("%s spits %s!", Monnam(mtmp), mattk->adtyp == AD_WEBS ? "webbing" : "venom");
			m_throw(mtmp, mtmp->mx, mtmp->my, sgn(tbx), sgn(tby),
			distmin(mtmp->mx,mtmp->my,mtmp->mux,mtmp->muy), otmp,
			TRUE);
			if(mtmp->mux != u.ux || mtmp->muy != u.uy){
				//figures out you aren't where it thought you were.
				mtmp->mux = 0;
				mtmp->muy = 0;
			}
		    nomul(0, NULL);
		    return 0;
		}
	}
	return 0;
}

int set_destroy_thrown(value)
int value;
{
	destroy_thrown = value;
	return destroy_thrown;
}

int set_bypassDR(value)
int value;
{
	bypassDR = value;
	return bypassDR;
}

int
spitmm(mtmp, mdef, mattk)	/* monster spits substance at monster */
register struct monst *mtmp;
register struct monst *mdef;
register struct attack *mattk;
{
	register struct obj *otmp;

	if(mtmp->mcan) {

	    if(flags.soundok)
		pline("A dry rattle comes from %s throat.",
		                      s_suffix(mon_nam(mtmp)));
	    return 0;
	}
    if(mtmp->mspec_used){
		return 0;
	}
	if(mlined_up(mtmp, mdef, FALSE)) {
		switch (mattk->adtyp) {
		    case AD_WEBS:
				mtmp->mspec_used = d(2,6);
				otmp = mksobj(BALL_OF_WEBBING, TRUE, FALSE);
			break;
		    case AD_BLND:
		    case AD_DRST:
				otmp = mksobj(BLINDING_VENOM, TRUE, FALSE);
			break;
		    default:
			impossible("bad attack type in spitmu");
				/* fall through */
		    case AD_ACID:
			otmp = mksobj(ACID_VENOM, TRUE, FALSE);
			if(mattk->damn && mattk->damd) otmp->ovar1 = d(mattk->damn,mattk->damd);
			break;
		}
		if(!rn2(BOLT_LIM-distmin(mtmp->mx,mtmp->my,mdef->mx,mdef->my))) {
		    if (canseemon(mtmp)) {
			pline("%s spits %s!",  Monnam(mtmp), mattk->adtyp == AD_WEBS ? "webbing" : "venom");
		    nomul(0, NULL);
		    }
			destroy_thrown = 1; //state variable referenced in drop_throw
				m_throw(mtmp, mtmp->mx, mtmp->my, sgn(tbx), sgn(tby),
					distmin(mtmp->mx,mtmp->my,mdef->mx,mdef->my), otmp,
					FALSE);
			destroy_thrown = 0;  //state variable referenced in drop_throw
		    return 0;
		}
	}
	return 0;
}

int
firemu(mtmp, mattk)		/* monster fires arrows at you */
register struct monst *mtmp;
register struct attack *mattk;
{
	register struct obj *qvr = NULL;
	int ammo_type, autodestroy = 1;
	

	if(mtmp->data->maligntyp < 0 && Is_illregrd(&u.uz)) return;
	if(mtmp->mux == 0 && mtmp->muy == 0) return 0;
	
	if(lined_up(mtmp)) {
		int yadj, xadj, rngmod;
		yadj = xadj = 0;
		rngmod = 0;
		bypassDR = 0;
		switch (mattk->adtyp) {
		    case AD_SHDW:
				if(onscary(mtmp->mux,mtmp->muy,mtmp)) return 0; //Warded
				ammo_type = DROVEN_BOLT;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 0;
			    qvr->quan = 1;
			    qvr->spe = 8;
				qvr->opoisoned = (OPOISON_BASIC|OPOISON_BLIND);
				bypassDR = 1;
			break;
		    case AD_PLYS:
				ammo_type = SPIKE;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 0;
			    qvr->quan = 1;
				qvr->opoisoned = (OPOISON_PARAL);
			break;
		    case AD_SOLR:
				ammo_type = SILVER_ARROW;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 1;
			    qvr->cursed = 0;
			    qvr->quan = 1;
			    qvr->spe = 7;
				rngmod = 1000; /* Fly until it strikes something */
			break;
			case AD_SLVR:
				ammo_type = SILVER_ARROW;
			break;
			case AD_BALL:
ironball:
				ammo_type = HEAVY_IRON_BALL;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 0;
				rngmod = 8;
			break;
			case AD_LOAD:
				if(near_capacity()>UNENCUMBERED) goto ironball;
				ammo_type = LOADSTONE;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 1;
				rngmod = 8;
			break;
			case AD_BLDR:
				ammo_type = BOULDER;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 0;
				rngmod = 8;
				autodestroy = 0;
			break;
			case AD_VBLD:
				ammo_type = HEAVY_IRON_BALL;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 0;
				rngmod = 8;
				if(mtmp->muy == mtmp->my) yadj = d(1,3)-2;
				else if(mtmp->mux == mtmp->mx) xadj = d(1,3)-2;
				else if(mtmp->mux - mtmp->mx == mtmp->muy - mtmp->my){
					xadj = d(1,3)-2;
					yadj = -1*xadj;
				}
				else xadj = yadj = d(1,3)-2;
			break;
		    default:
				ammo_type = ARROW;
			break;
		}
		if(!qvr){
			for(qvr = mtmp->minvent; qvr; qvr=qvr->nobj){
					if(qvr->otyp==ammo_type) break;
			}
		}
		if(!qvr){
			return 0; //no ammo of the right type found.
		}
		if(BOLT_LIM + rngmod >= distmin(mtmp->mx,mtmp->my,mtmp->mux,mtmp->muy)) {
			destroy_thrown = autodestroy; //state variable referenced in drop_throw
				if(mattk->adtyp == AD_SHDW){
					struct trap *ttmp2;
					m_throw(mtmp, mtmp->mux + (-sgn(tbx)) + xadj, mtmp->muy + (-sgn(tby)) + yadj, sgn(tbx), sgn(tby),
						1, qvr,TRUE);
					ttmp2 = maketrap(mtmp->mux, mtmp->muy, WEB);
					if (mtmp->mux == u.ux && mtmp->muy == u.uy && ttmp2) {
						pline_The("webbing sticks to you. You're caught!");
						dotrap(ttmp2, NOWEBMSG);
#ifdef STEED
						if (u.usteed && u.utrap) {
						/* you, not steed, are trapped */
						dismount_steed(DISMOUNT_FELL);
						}
#endif
					}
				} else {
					m_throw(mtmp, mtmp->mx + xadj, mtmp->my + yadj, sgn(tbx), sgn(tby),
						BOLT_LIM + rngmod, qvr,TRUE);
				}
				if(mtmp->mux != u.ux || mtmp->muy != u.uy){
					//figures out you aren't where it thought you were
					mtmp->mux = 0;
					mtmp->muy = 0;
				}
			    nomul(0, NULL);
			destroy_thrown = 0;  //state variable referenced in drop_throw
		}
		bypassDR = 0;
	}
	return 0;
}

int
firemm(mtmp, mdef, mattk)		/* monster fires arrows at you */
register struct monst *mtmp, *mdef;
register struct attack *mattk;
{
	register struct obj *qvr = NULL;
	int ammo_type, autodestroy = 1;

	if(mlined_up(mtmp, mdef, FALSE)) {
		int yadj, xadj, rngmod;
		yadj = xadj = 0;
		rngmod = 0;
		bypassDR = 0;
		switch (mattk->adtyp) {
		    case AD_SHDW:
				if(onscary(mdef->mx,mdef->my,mtmp)) return 0; //Warded
				ammo_type = DROVEN_BOLT;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 0;
			    qvr->quan = 1;
			    qvr->spe = 8;
				qvr->opoisoned = (OPOISON_BASIC|OPOISON_BLIND);
				bypassDR = 1;
			break;
		    case AD_SOLR:
				ammo_type = SILVER_ARROW;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 1;
			    qvr->cursed = 0;
			    qvr->quan = 1;
			    qvr->spe = 7;
				rngmod = 1000; /* Fly until it strikes something */
			break;
			case AD_SLVR:
				ammo_type = SILVER_ARROW;
			break;
			case AD_BALL:
				ammo_type = HEAVY_IRON_BALL;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 0;
				rngmod = 8;
			break;
			case AD_LOAD:
				ammo_type = LOADSTONE;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 1;
				rngmod = 8;
			break;
			case AD_BLDR:
				ammo_type = BOULDER;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 0;
				rngmod = 8;
				autodestroy = 0;
			break;
			case AD_VBLD:
				ammo_type = HEAVY_IRON_BALL;
				qvr = mksobj(ammo_type, TRUE, FALSE);
			    qvr->blessed = 0;
			    qvr->cursed = 0;
				rngmod = 8;
				if(mdef->my == mtmp->my) yadj = d(1,3)-2;
				else if(mdef->mx == mtmp->mx) xadj = d(1,3)-2;
				else if(mdef->mx - mtmp->mx == mdef->my - mtmp->my){
					xadj = d(1,3)-2;
					yadj = -1*xadj;
				}
				else xadj = yadj = d(1,3)-2;
			break;
		    default:
				ammo_type = ARROW;
			break;
		}
		if(!qvr){
			for(qvr = mtmp->minvent; qvr; qvr=qvr->nobj){
					if(qvr->otyp==ammo_type) break;
			}
		}
		if(!qvr){
			return 0; //no ammo of the right type found.
		}
		if(BOLT_LIM + rngmod >= distmin(mtmp->mx,mtmp->my,mdef->mx,mdef->my)) {
			destroy_thrown = autodestroy; //state variable referenced in drop_throw
				if(mattk->adtyp == AD_SHDW){
					struct trap *ttmp2;
					m_throw(mtmp, mdef->mx + (-sgn(tbx)) + xadj, mdef->my + (-sgn(tby)) + yadj, sgn(tbx), sgn(tby),
						1, qvr,TRUE);
					ttmp2 = maketrap(mdef->mx, mdef->my, WEB);
					if (ttmp2) mintrap(mdef);
				} else {
					m_throw(mtmp, mtmp->mx + xadj, mtmp->my + yadj, sgn(tbx), sgn(tby),
						BOLT_LIM + rngmod, qvr,TRUE);
				}
			    nomul(0, NULL);
			destroy_thrown = 0;  //state variable referenced in drop_throw
		}
		bypassDR = 0;
	}
	return 0;
}
#endif /* OVLB */
#ifdef OVL1

int
breamu(mtmp, mattk)			/* monster breathes at you (ranged) */
	register struct monst *mtmp;
	register struct attack  *mattk;
{

	if(mtmp->data->maligntyp < 0 && Is_illregrd(&u.uz)) return 0;
	if(mtmp->mux == 0 && mtmp->muy == 0) return 0;
	
	/* if new breath types are added, change AD_ACID to max type */
	int typ = (mattk->adtyp == AD_RBRE) ? rnd(AD_ACID) : mattk->adtyp, mult = 1;
	if(typ == AD_HDRG){
		typ = mtmp->mvar1;
		if(typ == AD_SLEE) mult = 4;
	}

	if(lined_up(mtmp)) {

	    if(mtmp->mcan) {
		if(flags.soundok) {
		    if(canseemon(mtmp))
			pline("%s coughs.", Monnam(mtmp));
		    else
			You_hear("a cough.");
		}
		return(0);
	    }
	    if(!mtmp->mspec_used && rn2(3)) {

		if((typ >= AD_MAGM) && (typ <= AD_ACID)) {

		    if(canseemon(mtmp))
			pline("%s breathes %s!", Monnam(mtmp),
			      breathwep[typ-1]);
		    buzz((int) (-20 - (typ-1)), (int)mattk->damn + (mtmp->m_lev/2),
			 mtmp->mx, mtmp->my, sgn(tbx), sgn(tby),0,mattk->damd ? (d((int)mattk->damn + (mtmp->m_lev/2), (int)mattk->damd)*mult) : 0);
			if(mtmp->mux != u.ux || mtmp->muy != u.uy){
				//figures out you aren't where it thought you were
				mtmp->mux = 0;
				mtmp->muy = 0;
			}
		    nomul(0, NULL);
		    /* breath runs out sometimes. Also, give monster some
		     * cunning; don't breath if the player fell asleep.
		     */
		    if(!rn2(3))
			mtmp->mspec_used = 10+rn2(20);
		    if(typ == AD_SLEE && !Sleep_resistance)
			mtmp->mspec_used += rnd(20);
		} else impossible("Breath weapon %d used", typ-1);
	    }
	}
	return(1);
}

int
breamm(mtmp, mdef, mattk)		/* monster breathes at monst (ranged) */
	register struct monst *mtmp;
	register struct monst *mdef;
	register struct attack  *mattk;
{
	/* if new breath types are added, change AD_ACID to max type */
	int typ = mattk->adtyp, mult = 1;
	if(typ == AD_HDRG){
		typ = mtmp->mvar1;
		if(typ == AD_SLEE) mult = 4;
	}
	
	if(typ == AD_RBRE){
		if(mtmp->data == &mons[PM_CHROMATIC_DRAGON]){
			switch(rnd(6)){
				case 1:
					typ = AD_FIRE;
				break;
				case 2:
					typ = AD_COLD;
				break;
				case 3:
					typ = AD_ELEC;
				break;
				case 4:
					typ = AD_DRST;
				break;
				case 5:
					typ = AD_DISN;
				break;
				case 6:
					typ = AD_ACID;
				break;
			}
		} else if(mtmp->data == &mons[PM_PLATINUM_DRAGON]){
			switch(rnd(4)){
				case 1:
					typ = AD_FIRE;
				break;
				case 2:
					typ = AD_DISN;
				break;
				case 3:
					typ = AD_SLEE;
				break;
				case 4:
					typ = AD_ELEC;
				break;
			}
		} else rnd(AD_ACID);
	}

	if(mtmp->data->maligntyp < 0 && Is_illregrd(&u.uz)) return 0;
	
	if (distmin(mtmp->mx, mtmp->my, mdef->mx, mdef->my) < 3)
	    return 0;  /* not at close range */

	if(mlined_up(mtmp, mdef, TRUE)) {

	    if(mtmp->mcan) {
		if(flags.soundok) {
		    if(canseemon(mtmp))
			pline("%s coughs.", Monnam(mtmp));
		    else
			You_hear("a cough.");
		}
		return(0);
	    }
	    if(!mtmp->mspec_used && rn2(3)) {

		if((typ >= AD_MAGM) && (typ <= AD_ACID)) {

		    if(canseemon(mtmp))
		    {
			pline("%s breathes %s!", Monnam(mtmp),
			      breathwep[typ-1]);
		    nomul(0, NULL);
	            }
		    buzz((int) (-20 - (typ-1)), (int)mattk->damn + (mtmp->m_lev/2),
			 mtmp->mx, mtmp->my, sgn(tbx), sgn(tby),0,mattk->damd ? (d((int)mattk->damn + (mtmp->m_lev/2), (int)mattk->damd)*mult) : 0);
		    /* breath runs out sometimes. Also, give monster some
		     * cunning; don't breath if the player fell asleep.
		     */
		    if(!rn2(3))
			mtmp->mspec_used = 10+rn2(20);
		    if(typ == AD_SLEE && !Sleep_resistance)
			mtmp->mspec_used += rnd(20);
		} else impossible("Breath weapon %d used", typ-1);
	    }
	}
	return(1);
}

boolean
linedup(ax, ay, bx, by)
register xchar ax, ay, bx, by;
{
	tbx = ax - bx;	/* These two values are set for use */
	tby = ay - by;	/* after successful return.	    */

	/* sometimes displacement makes a monster think that you're at its
	   own location; prevent it from throwing and zapping in that case */
	if (!tbx && !tby) return FALSE;

	if((!tbx || !tby || abs(tbx) == abs(tby)) /* straight line or diagonal */
	   && distmin(tbx, tby, 0, 0) < BOLT_LIM) {
	    if(ax == u.ux && ay == u.uy) return((boolean)(couldsee(bx,by)));
	    else if(clear_path(ax,ay,bx,by)) return TRUE;
	}
	return FALSE;
}

boolean
lined_up(mtmp)		/* is mtmp in position to use ranged attack? */
	register struct monst *mtmp;
{
	return(linedup(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my));
}

boolean
mlined_up(mtmp, mdef, breath)	/* is mtmp in position to use ranged attack? */
	register struct monst *mtmp;
	register struct monst *mdef;
	register boolean breath;
{
	struct monst *mat;

        boolean lined_up = linedup(mdef->mx,mdef->my,mtmp->mx,mtmp->my);

	int dx = sgn(mdef->mx - mtmp->mx),
	    dy = sgn(mdef->my - mtmp->my);

	int x = mtmp->mx, y = mtmp->my;

	int i = 10; /* arbitrary */

        /* No special checks if confused - can't tell friend from foe */
	if (!lined_up || mtmp->mconf || !mtmp->mtame) return lined_up;

        /* Check for friendlies in the line of fire. */
	for (; !breath || i > 0; --i)
	{
	    x += dx;
	    y += dy;
	    if (!isok(x, y)) break;
		
            if (x == u.ux && y == u.uy) 
	        return FALSE;

	    mat = m_at(x, y);
	    if (mat)
	    {
	        if (!breath && mat == mdef) return lined_up;

		/* Don't hit friendlies: */
		if (mat->mtame) return FALSE;
	    }
	}

	return lined_up;
}

#endif /* OVL1 */
#ifdef OVL0

/* Check if a monster is carrying a particular item.
 */
struct obj *
m_carrying(mtmp, type)
struct monst *mtmp;
int type;
{
	register struct obj *otmp;

	for(otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == type)
			return(otmp);
	return((struct obj *) 0);
}

/* Check if a monster is carrying a particular charged (ovar1>0) item.
 */
struct obj *
m_carrying_charged(mtmp, type)
struct monst *mtmp;
int type;
{
	register struct obj *otmp;

	for(otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == type && otmp->ovar1 > 0)
			return(otmp);
	return((struct obj *) 0);
}

/* TRUE iff thrown/kicked/rolled object doesn't pass through iron bars */
boolean
hits_bars(obj_p, x, y, always_hit, whodidit)
struct obj **obj_p;	/* *obj_p will be set to NULL if object breaks */
int x, y;
int always_hit;	/* caller can force a hit for items which would fit through */
int whodidit;	/* 1==hero, 0=other, -1==just check whether it'll pass thru */
{
    struct obj *otmp = *obj_p;
    int obj_type = otmp->otyp;
    boolean hits = always_hit;

    if (!hits)
	switch (otmp->oclass) {
	case WEAPON_CLASS:
	    {
		int oskill = objects[obj_type].oc_skill;

		hits = (oskill != -P_BOW  && oskill != -P_CROSSBOW &&
			oskill != -P_DART && oskill != -P_SHURIKEN &&
//#ifdef FIREARMS
			(oskill != -P_FIREARM || obj_type == ROCKET) &&
//#endif
			oskill != P_SPEAR && oskill != P_JAVELIN &&
			oskill != P_KNIFE);	/* but not dagger */
		break;
	    }
	case ARMOR_CLASS:
		hits = (objects[obj_type].oc_armcat != ARM_GLOVES);
		break;
	case TOOL_CLASS:
		hits = (obj_type != SKELETON_KEY &&
			obj_type != UNIVERSAL_KEY &&
			obj_type != LOCK_PICK &&
#ifdef TOURIST
			obj_type != CREDIT_CARD &&
#endif
			obj_type != TALLOW_CANDLE &&
			obj_type != WAX_CANDLE &&
			obj_type != LENSES &&
			obj_type != R_LYEHIAN_FACEPLATE &&
			obj_type != TIN_WHISTLE &&
			obj_type != MAGIC_WHISTLE);
		break;
	case ROCK_CLASS:	/* includes boulder */
		if (obj_type != STATUE ||
			mons[otmp->corpsenm].msize > MZ_TINY) hits = TRUE;
		break;
	case FOOD_CLASS:
		if (obj_type == CORPSE &&
			mons[otmp->corpsenm].msize > MZ_TINY) hits = TRUE;
		else
		    hits = (obj_type == MEAT_STICK ||
			    obj_type == MASSIVE_CHUNK_OF_MEAT);
		break;
	case SPBOOK_CLASS:
	case WAND_CLASS:
	case BALL_CLASS:
	case CHAIN_CLASS:
/*	case BED_CLASS:*/
		hits = TRUE;
		break;
	default:
		break;
	}

    if (hits && whodidit != -1) {
	if (whodidit ? hero_breaks(otmp, x, y, FALSE) : breaks(otmp, x, y))
	    *obj_p = otmp = 0;		/* object is now gone */
	    /* breakage makes its own noises */
	else if (obj_type == BOULDER || obj_type == STATUE || obj_type == HEAVY_IRON_BALL)
	    pline("Whang!");
	else if (otmp->oclass == COIN_CLASS ||
		objects[obj_type].oc_material == GOLD ||
		objects[obj_type].oc_material == SILVER)
	    pline("Clink!");
	else
	    pline("Clonk!");
    }

    return hits;
}

#endif /* OVL0 */

/*mthrowu.c*/
