/*	SCCS Id: @(#)lock.c	3.4	2000/02/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

STATIC_PTR int NDECL(picklock);
STATIC_PTR int NDECL(forcelock);
STATIC_PTR int NDECL(forcedoor);

	//Through me you pass into the city of woe;
	//Through me you pass into eternal pain;
	//Through me among the people lost for aye.
	//
	//Justice the founder of my fabric moved:
	//To rear me the task of power divine,
	//supremest wisdom, and primeval love.
	//
	//Before me only eternal things were made,
	//And I shall endure eternally.
	//All hope abandon, ye who enter here!
const char * const gates_of_hell[] = {
"",
"Through me you pass into the city of woe;\n\
Through me you pass into eternal pain;\n\
Through me among the people lost for aye.",
"Justice the founder of my fabric moved:\n\
To rear me the task of power divine,\n\
supremest wisdom, and primeval love.",
"Before me only eternal things were made,\n\
And I shall endure eternally.\n\
All hope abandon, ye who enter here!"
};
/* at most one of `door' and `box' should be non-null at any given time */
STATIC_VAR NEARDATA struct xlock_s {
	struct rm  *door;
	struct obj *box;
	int picktyp, chance, usedtime;
	/* ALI - Artifact doors from slash'em*/
	int key;			/* Key being used (doors only) */
	long mgclcknm;
} xlock;

#ifdef OVLB

STATIC_DCL const char *NDECL(lock_action);
STATIC_DCL boolean FDECL(obstructed,(int,int));
STATIC_DCL void FDECL(chest_shatter_msg, (struct obj *));

boolean
picking_lock(x, y)
	int *x, *y;
{
	if (occupation == picklock) {
	    *x = u.ux + u.dx;
	    *y = u.uy + u.dy;
	    return TRUE;
	} else {
	    *x = *y = 0;
	    return FALSE;
	}
}

boolean
picking_at(x, y)
int x, y;
{
	return (boolean)(occupation == picklock && xlock.door == &levl[x][y]);
}

/* produce an occupation string appropriate for the current activity */
STATIC_OVL const char *
lock_action()
{
	/* "unlocking"+2 == "locking" */
	static const char *actions[] = {
		/* [0] */	"unlocking the door",
		/* [1] */	"unlocking the chest",
		/* [2] */	"unlocking the box",
		/* [3] */	"picking the lock"
	};

	/* if the target is currently unlocked, we're trying to lock it now */
	if (xlock.door && !(xlock.door->doormask & D_LOCKED))
		return actions[0]+2;	/* "locking the door" */
	else if (xlock.box && !xlock.box->olocked && xlock.box->otyp != MAGIC_CHEST)
		return xlock.box->otyp == CHEST ? actions[1]+2 : actions[2]+2;
	/* otherwise we're trying to unlock it */
	else if (xlock.picktyp == LOCK_PICK)
		return actions[3];	/* "picking the lock" */
#ifdef TOURIST
	else if (xlock.picktyp == CREDIT_CARD)
		return actions[3];	/* same as lock_pick */
#endif
	else if (xlock.door)
		return actions[0];	/* "unlocking the door" */
	else
		return xlock.box->otyp == CHEST ? actions[1] : actions[2];
}

STATIC_PTR
int
picklock()	/* try to open/close a lock */
{

	if (xlock.box) {
	    if((xlock.box->ox != u.ux) || (xlock.box->oy != u.uy)) {
		return((xlock.usedtime = 0));		/* you or it moved */
	    }
	} else {		/* door */
	    if(xlock.door != &(levl[u.ux+u.dx][u.uy+u.dy])) {
		return((xlock.usedtime = 0));		/* you moved */
	    }
	    switch (xlock.door->doormask) {
		case D_NODOOR:
		    pline("This doorway has no door.");
		    return((xlock.usedtime = 0));
		case D_ISOPEN:
		    You("cannot lock an open door.");
		    return((xlock.usedtime = 0));
		case D_BROKEN:
		    pline("This door is broken.");
		    return((xlock.usedtime = 0));
	    }
	}

	if (xlock.usedtime++ >= 50 || nohands(youracedata) || !freehand()) {
	    You("give up your attempt at %s.", lock_action());
	    exercise(A_DEX, TRUE);	/* even if you don't succeed */
	    return((xlock.usedtime = 0));
	}

	if(rn2(100) >= xlock.chance) return(1);		/* still busy */

	You("succeed in %s.", lock_action());
	if (xlock.door) {
	    if(xlock.door->doormask & D_TRAPPED) {
		    b_trapped("door", FINGER);
		    xlock.door->doormask = D_NODOOR;
		    unblock_point(u.ux+u.dx, u.uy+u.dy);
		    if (*in_rooms(u.ux+u.dx, u.uy+u.dy, SHOPBASE))
			add_damage(u.ux+u.dx, u.uy+u.dy, 0L);
		    newsym(u.ux+u.dx, u.uy+u.dy);
	    } else if (xlock.door->doormask & D_LOCKED) xlock.door->doormask = D_CLOSED;
	    else{
			xlock.door->doormask = D_LOCKED;
			if(u.sealsActive&SEAL_OTIAX) unbind(SEAL_OTIAX,TRUE);
		}
	} else {
	    if(xlock.box->otyp == MAGIC_CHEST){
			xlock.box->olocked = 0;
			xlock.box->ovar1 = xlock.mgclcknm;
		} else xlock.box->olocked = !xlock.box->olocked;
	    if(xlock.box->otrapped)	
		(void) chest_trap(xlock.box, FINGER, FALSE);
	}
	exercise(A_DEX, TRUE);
	return((xlock.usedtime = 0));
}

STATIC_PTR
int
forcelock()	/* try to force a locked chest */
{

	register struct obj *otmp;

	if((xlock.box->ox != u.ux) || (xlock.box->oy != u.uy))
		return((xlock.usedtime = 0));		/* you or it moved */

	if (xlock.usedtime++ >= 50 || (!uwep && xlock.picktyp != 3) 
		|| nohands(youracedata) || !freehand()
	) {
	    You("give up your attempt to force the lock.");
	    if(xlock.usedtime >= 50)		/* you made the effort */
	      exercise((xlock.picktyp) ? A_DEX : A_STR, TRUE);
	    return((xlock.usedtime = 0));
	}
	
	
	if(xlock.picktyp == 1) {	/* blade */

	    if((uwep->obj_material == GLASS || uwep->obj_material == OBSIDIAN_MT || (rn2(1000-(int)uwep->spe) > (992-greatest_erosion(uwep)*10) &&
	       !uwep->cursed)) && !uwep->oartifact) {
		/* for a +0 weapon, probability that it survives an unsuccessful
		 * attempt to force the lock is (.992)^50 = .67
		 */
		pline("%sour %s broke!",
		      (uwep->quan > 1L) ? "One of y" : "Y", xname(uwep));
		useup(uwep);
		You("give up your attempt to force the lock.");
		exercise(A_DEX, TRUE);
		return((xlock.usedtime = 0));
	    }
	} else if(xlock.picktyp == 0)			/* blunt */
	    wake_nearby_noisy();	/* due to hammering on the container */

	if(xlock.picktyp == 3) u.otiaxAttack = moves;
	if(rn2(100) >= xlock.chance) return(1);		/* still busy */

	You("succeed in forcing the lock.");
	xlock.box->olocked = 0;
	if(!(u.sealsActive&SEAL_OTIAX)) xlock.box->obroken = 1;
	if(!xlock.picktyp && !(u.sealsActive&SEAL_OTIAX) && !rn2(3)) {
	    struct monst *shkp;
	    boolean costly;
	    long loss = 0L;

	    costly = (*u.ushops && costly_spot(u.ux, u.uy));
	    shkp = costly ? shop_keeper(*u.ushops) : 0;

	    pline("In fact, you've totally destroyed %s.",
		  the(xname(xlock.box)));

	    /* Put the contents on ground at the hero's feet. */
		if (xlock.box->spe == 1) {
			observe_quantum_cat(xlock.box, TRUE); //TRUE: use past tense
		}else if(xlock.box->spe == 4){
			open_coffin(xlock.box, TRUE); //TRUE: use past tense
		}else if(xlock.box->spe == 5){
			open_sarcophagus(xlock.box, TRUE); //TRUE: use past tense
		}
		while ((otmp = xlock.box->cobj) != 0) {
		obj_extract_self(otmp);
		if(!rn2(3) || otmp->oclass == POTION_CLASS) {
		    chest_shatter_msg(otmp);
		    if (costly)
		        loss += stolen_value(otmp, u.ux, u.uy,
					     (boolean)shkp->mpeaceful, TRUE);
		    if (otmp->quan == 1L) {
			obfree(otmp, (struct obj *) 0);
			continue;
		    }
		    useup(otmp);
		}
		if (xlock.box->otyp == ICE_BOX && otmp->otyp == CORPSE) {
		    otmp->age = monstermoves - otmp->age; /* actual age */
		    start_corpse_timeout(otmp);
		}
		place_object(otmp, u.ux, u.uy);
		stackobj(otmp);
	    }

	    if (costly)
		loss += stolen_value(xlock.box, u.ux, u.uy,
					     (boolean)shkp->mpeaceful, TRUE);
	    if(loss) You("owe %ld %s for objects destroyed.", loss, currency(loss));
	    delobj(xlock.box);
	}
	exercise((xlock.picktyp) ? A_DEX : A_STR, TRUE);
	return((xlock.usedtime = 0));
}

STATIC_PTR
int
forcedoor()      /* try to break/pry open a door */
{

	if(xlock.door != &(levl[u.ux+u.dx][u.uy+u.dy])) {
	    return((xlock.usedtime = 0));           /* you moved */
	} 
	switch (xlock.door->doormask) {
	    case D_NODOOR:
		pline("This doorway has no door.");
		return((xlock.usedtime = 0));
	    case D_ISOPEN:
		You("cannot force an open door.");
		return((xlock.usedtime = 0));
	    case D_BROKEN:
		pline("This door is broken.");
		return((xlock.usedtime = 0));
	}
	
	if (xlock.usedtime++ >= 50
		|| ((nohands(youracedata) || !freehand()) 
		&& !(u.sealsActive&SEAL_OTIAX))
	) {
	    You("give up your attempt at %s the door.",
	    	(xlock.picktyp == 2 ? "melting" : xlock.picktyp == 1 ? 
	    		"prying open" : "breaking down"));
	    exercise(A_STR, TRUE);      /* even if you don't succeed */
	    return((xlock.usedtime = 0));
	}
	
	if(xlock.picktyp == 3) u.otiaxAttack = moves;
	
	if(rn2(100) > xlock.chance) return(1);          /* still busy */

	You("succeed in %s the door.",
	    	(xlock.picktyp == 3 ? "picking the lock on" : xlock.picktyp == 2 ? "melting" : xlock.picktyp == 1 ? 
	    		"prying open" : "breaking down"));

	if(xlock.door->doormask & D_TRAPPED) {
	    b_trapped("door", 0);
	    xlock.door->doormask = D_NODOOR;
	} else if (xlock.picktyp == 3) {
	    xlock.door->doormask = D_ISOPEN;
	} else if (xlock.picktyp == 1)
	    xlock.door->doormask = D_BROKEN;
	else xlock.door->doormask = D_NODOOR;
		unblock_point(u.ux+u.dx, u.uy+u.dy);
	if (*in_rooms(u.ux+u.dx, u.uy+u.dy, SHOPBASE))
	    add_damage(u.ux+u.dx, u.uy+u.dy, 0L);
	newsym(u.ux+u.dx, u.uy+u.dy);
	
	exercise(A_STR, TRUE);
	return((xlock.usedtime = 0));
}

#endif /* OVLB */
#ifdef OVL0

void
reset_pick()
{
	xlock.usedtime = xlock.chance = xlock.picktyp = 0;
	xlock.door = 0;
	xlock.box = 0;
}

#endif /* OVL0 */
#ifdef OVLB

int
pick_lock(pick) /* pick a lock with a given object */
	register struct	obj	*pick;
{
	int picktyp, c, ch;
	coord cc;
	int key;
	struct rm	*door;
	struct obj	*otmp;
	char qbuf[QBUFSZ];

	picktyp = pick->otyp;

	/* check whether we're resuming an interrupted previous attempt */
	if (xlock.usedtime && picktyp == xlock.picktyp) {
	    static char no_longer[] = "Unfortunately, you can no longer %s %s.";

	    if (nohands(youracedata) || !freehand()) {
		const char *what = (picktyp == LOCK_PICK) ? "pick" : "key";
#ifdef TOURIST
		if (picktyp == CREDIT_CARD) what = "card";
#endif
		pline(no_longer, "hold the", what);
		reset_pick();
		return 0;
	    } else if (xlock.box && !can_reach_floor()) {
		pline(no_longer, "reach the", "lock");
		reset_pick();
		return 0;
	    } else if (!xlock.door || xlock.key == pick->oartifact) {
		/* part of slash'em's artifact key code, I think -D_E */
		const char *action = lock_action();
		You("resume your attempt at %s.", action);
		set_occupation(picklock, action, 0);
		return(1);
	    }
	}

	if(nohands(youracedata)) {
		You_cant("hold %s -- you have no hands!", doname(pick));
		return(0);
	}
	if(!freehand()){
		You_cant("hold %s -- you have no free hands!", doname(pick));
		return(0);
	}

	if((picktyp != LOCK_PICK &&
#ifdef TOURIST
	    picktyp != CREDIT_CARD &&
#endif
	    picktyp != SKELETON_KEY &&
	    picktyp != UNIVERSAL_KEY)) {
		impossible("picking lock with object %d?", picktyp);
		return(0);
	}
	ch = 0;		/* lint suppression */

	if(!get_adjacent_loc((char *)0, "Invalid location!", u.ux, u.uy, &cc)) return 0;
	if (cc.x == u.ux && cc.y == u.uy) {	/* pick lock on a container */
	    const char *verb;
	    boolean it;
	    int count;

	    if (u.dz < 0) {
		There("isn't any sort of lock up %s.",
		      Levitation ? "here" : "there");
		return 0;
	    } else if (is_lava(u.ux, u.uy)) {
		pline("Doing that would probably melt your %s.",
		      xname(pick));
		return 0;
	    } else if (is_pool(u.ux, u.uy, FALSE) && !Underwater) {
		pline_The("water has no lock.");
		return 0;
	    }

	    count = 0;
	    c = 'n';			/* in case there are no boxes here */
	    for(otmp = level.objects[cc.x][cc.y]; otmp; otmp = otmp->nexthere)
		if (Is_box(otmp)) {
		    ++count;
		    if (!can_reach_floor()) {
			You_cant("reach %s from up here.", the(xname(otmp)));
			return 0;
		    }
		    it = 0;
		    if (otmp->obroken) verb = "fix";
		    else if (!otmp->olocked && otmp->otyp != MAGIC_CHEST) verb = "lock", it = 1;
		    else if (picktyp != LOCK_PICK) verb = "unlock", it = 1;
		    else verb = "pick";
		    Sprintf(qbuf, "There is %s here, %s %s?",
		    	    safe_qbuf("", sizeof("There is  here, unlock its lock?"),
			    	doname(otmp), an(simple_typename(otmp->otyp)), "a box"),
			    verb, it ? "it" : "its lock");

		    c = ynq(qbuf);
		    if(c == 'q') return(0);
		    if(c == 'n') continue;

		    if (otmp->obroken) {
				You_cant("fix its broken lock with %s.", doname(pick));
				return 0;
		    } else if(otmp->otyp == MAGIC_CHEST){
				char locknumber = 0;
				pline("What lock will you open (0-9)?");
				do locknumber = readchar();
				while(!(locknumber >= '0' && locknumber <= '9'));
				if(otmp->ovar1 == (long)(locknumber-'0') && !otmp->olocked){
					pline("That lock is already open.");
					return 0;
				}
				xlock.mgclcknm = (long)(locknumber-'0');
			}
#ifdef TOURIST
		    else if (picktyp == CREDIT_CARD && !otmp->olocked && otmp->otyp != MAGIC_CHEST) {
				/* credit cards are only good for unlocking */
				You_cant("do that with %s.", doname(pick));
				return 0;
		    }
#endif
		    switch(picktyp) {
#ifdef TOURIST
			case CREDIT_CARD:
			    ch = ACURR(A_DEX) + 20*Role_if(PM_ROGUE);
			    break;
#endif
			case LOCK_PICK:
			    ch = 4*ACURR(A_DEX) + 25*Role_if(PM_ROGUE);
			    break;
			case SKELETON_KEY:
			    ch = 75 + ACURR(A_DEX);
			    break;
			case UNIVERSAL_KEY:
			    ch = 85 + ACURR(A_DEX);
			    break;
			default:	ch = 0;
		    }
		    if(otmp->cursed) ch /= 2;

		    xlock.picktyp = picktyp;
		    xlock.box = otmp;
		    xlock.door = 0;
		    break;
		}
	    if (c != 'y') {
		if (!count)
		    There("doesn't seem to be any sort of lock here.");
		return(0);		/* decided against all boxes */
	    }
	} else {			/* pick the lock in a door */
	    struct monst *mtmp;

	    if (u.utrap && u.utraptype == TT_PIT) {
		You_cant("reach over the edge of the pit.");
		return(0);
	    }

	    door = &levl[cc.x][cc.y];
	    if ((mtmp = m_at(cc.x, cc.y)) && canseemon(mtmp)
			&& mtmp->m_ap_type != M_AP_FURNITURE
			&& mtmp->m_ap_type != M_AP_OBJECT) {
#ifdef TOURIST
		if (picktyp == CREDIT_CARD &&
		    (mtmp->isshk || mtmp->data == &mons[PM_ORACLE]))
		    verbalize("No checks, no credit, no problem.");
		else
#endif
		    pline("I don't think %s would appreciate that.", mon_nam(mtmp));
		return(0);
	    }
	    if(!IS_DOOR(door->typ)) {
		if (is_drawbridge_wall(cc.x,cc.y) >= 0)
		    You("%s no lock on the drawbridge.",
				Blind ? "feel" : "see");
		else
		    You("%s no door there.",
				Blind ? "feel" : "see");
		return(0);
		}
	    switch (door->doormask) {
		case D_NODOOR:
		    pline("This doorway has no door.");
		    return(0);
		case D_ISOPEN:
		    You("cannot lock an open door.");
		    return(0);
		case D_BROKEN:
		    pline("This door is broken.");
		    return(0);
		default:
#ifdef TOURIST
		    /* credit cards are only good for unlocking */
		    if(picktyp == CREDIT_CARD && !(door->doormask & D_LOCKED)) {
			You_cant("lock a door with a credit card.");
			return(0);
		    }
#endif
		    /* ALI - Artifact doors from slash'em */
		    key = artifact_door(cc.x, cc.y);

		    Sprintf(qbuf,"%sock it?",
			(door->doormask & D_LOCKED) ? "Unl" : "L" );

		    c = yn(qbuf);
		    if(c == 'n') return(0);

		    switch(picktyp) {
#ifdef TOURIST
			case CREDIT_CARD:
			    ch = 2*ACURR(A_DEX) + 20*Role_if(PM_ROGUE);
			    break;
#endif
			case LOCK_PICK:
			    ch = 3*ACURR(A_DEX) + 30*Role_if(PM_ROGUE);
			    break;
			case SKELETON_KEY:
			    ch = 70 + ACURR(A_DEX);
			    break;
			case UNIVERSAL_KEY:
			    ch = 80 + ACURR(A_DEX);
			    break;
			default:    ch = 0;
		    }
		    xlock.door = door;
		    xlock.box = 0;

			/* ALI - Artifact doors from slash'em*/
		    xlock.key = pick->oartifact;
			if (key){
				if(!In_quest(&u.uz) && xlock.key >= ART_FIRST_KEY_OF_LAW && xlock.key <= ART_THIRD_KEY_OF_NEUTRALITY) {
					register struct rm *here;
					here = &levl[cc.x][cc.y];
					here->typ = ROOM;
					useupall(pick);
					make_engr_at(cc.x, cc.y,
						 gates_of_hell[key%4], 0L, BURN); //mod 4 the array index so people can mess up the des file without causing problems
					unblock_point(cc.x,cc.y);
					newsym(cc.x,cc.y);
					return(0);
				} else if(In_quest(&u.uz) && urole.neminum == PM_BOLG && xlock.key == ART_KEY_OF_EREBOR){
					register struct rm *here;
					here = &levl[cc.x][cc.y];
					here->doormask = D_ISOPEN;
					unblock_point(cc.x,cc.y);
					newsym(cc.x,cc.y);
					return(1);
				} else if (picktyp == SKELETON_KEY || picktyp == UNIVERSAL_KEY) {
					Your("key doesn't seem to fit.");
					return(0);
				}
				else ch = -1;		/* -1 == 0% chance */
			}
	    }
	}
	flags.move = 0;
	xlock.chance = ch;
	xlock.picktyp = picktyp;
	xlock.usedtime = 0;
	set_occupation(picklock, lock_action(), 0);
	return(1);
}

int
doforce()		/* try to force a chest with your weapon */
{
	register struct obj *otmp;
	register int x, y, c, picktyp;
	struct rm       *door;
	char qbuf[QBUFSZ];

	if (!uwep && !(u.sealsActive&SEAL_OTIAX)) { /* Might want to make this so you use your shoulder */
	    You_cant("force anything without a weapon.");
	     return(0);
	}

	if (u.utrap && u.utraptype == TT_WEB) {
	    You("are entangled in a web!");
	    return(0);
	} else if (u.sealsActive&SEAL_OTIAX) {
		;
	} else if (uwep && is_lightsaber(uwep)) {
	    if (!litsaber(uwep)) {
		Your("lightsaber is deactivated!");
		return(0);
	    }
	} else if(uwep->otyp == LOCK_PICK ||
	    uwep->otyp == CREDIT_CARD ||
	    uwep->otyp == SKELETON_KEY) {
	    	return pick_lock(uwep);
	/* not a lightsaber or lockpicking device*/
	} else if(!uwep ||     /* proper type test */
	   (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep) &&
	    uwep->oclass != ROCK_CLASS) ||
	   (objects[uwep->otyp].oc_skill < P_DAGGER) ||
	   (objects[uwep->otyp].oc_skill > P_LANCE) ||
	   uwep->otyp == FLAIL || uwep->otyp == AKLYS
	  ) {
	    You_cant("force anything without a %sweapon.",
		  (uwep) ? "proper " : "");
	    return(0);
	}

	if (u.sealsActive&SEAL_OTIAX)
	    picktyp = 3;
	else if (is_lightsaber(uwep))
	    picktyp = 2;
	else
		picktyp = is_blade(uwep) ? 1 : 0;
	if(xlock.usedtime && picktyp == xlock.picktyp) {
	    if (xlock.box) {
		    You("resume your attempt to force the lock.");
		    set_occupation(forcelock, "forcing the lock", 0);
		    return(1);
	    } 
		else if (xlock.door) {
			You("resume your attempt to force the door.");
			set_occupation(forcedoor, "forcing the door", 0);
			return(1);
	    }
	}

	/* A lock is made only for the honest man, the thief will break it. */
	xlock.box = (struct obj *)0;

	if(!getdir((char *)0)) return(0);

	x = u.ux + u.dx;
	y = u.uy + u.dy;
	if (x == u.ux && y == u.uy && u.dz > -1) {
	for(otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere)
	    if(Is_box(otmp)) {
		if (otmp->obroken || !otmp->olocked) {
		    There("is %s here, but its lock is already %s.",
			  doname(otmp), otmp->obroken ? "broken" : "unlocked");
		    continue;
		}
		Sprintf(qbuf,"There is %s here, force its lock?",
			safe_qbuf("", sizeof("There is  here, force its lock?"),
				doname(otmp), an(simple_typename(otmp->otyp)),
				"a box"));

		c = ynq(qbuf);
		if(c == 'q') return(0);
		if(c == 'n') continue;

		if(picktyp == 3) {
		    You("insert your mist tendrils into the lock.");
			u.otiaxAttack = moves;
		} else if(picktyp == 2)
		    You("begin melting the lock with your %s.", xname(uwep));
		else if(picktyp)
		    You("force your %s into a crack and pry.", xname(uwep));
		else
		    You("start bashing it with your %s.", xname(uwep));
		xlock.box = otmp;
		if (picktyp == 3) //mist tendrils
			xlock.chance = spiritDsize() * 10;
		else if (picktyp == 2) //lightsaber
		    xlock.chance = uwep->spe * 2 + 75;
		else xlock.chance = objects[uwep->otyp].oc_wldam * 2;
		xlock.picktyp = picktyp;
		xlock.usedtime = 0;
		break;
	    }
	    if(xlock.box)   {
	    	xlock.door = 0;
	    	set_occupation(forcelock, "forcing the lock", 0);
	    	return(1);
	    }
	} else {		/* break down/open door */
	    struct monst *mtmp;

	    door = &levl[x][y];
	    if ((mtmp = m_at(x, y)) && canseemon(mtmp)
			&& mtmp->m_ap_type != M_AP_FURNITURE
			&& mtmp->m_ap_type != M_AP_OBJECT) {

		if (mtmp->isshk || mtmp->data == &mons[PM_ORACLE])		
		    verbalize("What do you think you are, a Jedi?"); /* Phantom Menace */
		else
		    pline("I don't think %s would appreciate that.", mon_nam(mtmp));
		return(0);
	    }
	    /* Lightsabers dig through doors and walls via dig.c */
	    if (picktyp != 3 && (is_pick(uwep) ||
		    is_lightsaber(uwep) ||
		    is_axe(uwep)))
	    	return use_pick_axe2(uwep);
		
	    if(!IS_DOOR(door->typ)) {
		if (is_drawbridge_wall(x,y) >= 0)
		    pline("The drawbridge is too solid to force open.");
		else
		    You("%s no door there.",
				Blind ? "feel" : "see");
		return(0);
	    }
	    /* ALI - artifact doors */
	    if (artifact_door(x, y)) {
		pline("This door is too solid to force open.");
		return 0;
	    }
	    switch (door->doormask) {
		case D_NODOOR:
		    pline("This doorway has no door.");
		    return(0);
		case D_ISOPEN:
		    You("cannot force an open door.");
		    return(0);
		case D_BROKEN:
		    pline("This door is broken.");
		    return(0);
		default:
		    if(picktyp == 3) c = yn("Force the door's lock?");
			else c = yn("Break down the door?");
		    if(c == 'n') return(0);

		    if(picktyp == 3){
				You("insert your mist tendrils into the door's lock.");
				u.otiaxAttack = moves;
		    } else if(picktyp == 1)
				You("force your %s into a crack and pry.", xname(uwep));
		    else
				You("start bashing it with your %s.", xname(uwep));
		    if (picktyp == 3)
				xlock.chance = spiritDsize() * 10;
		    else if (is_lightsaber(uwep))
				xlock.chance = uwep->spe + 38;
		    else
				xlock.chance = uwep->spe + objects[uwep->otyp].oc_wldam;
		    xlock.picktyp = picktyp;
		    xlock.usedtime = 0;    
		    xlock.door = door;
		    xlock.box = 0;
		    set_occupation(forcedoor, "forcing the door", 0);
	return(1);
	    }
	}
	You("decide not to force the issue.");
	return(0);
}

int
doopen()		/* try to open a door */
{
	return doopen_indir(0, 0);
}

int
doopen_indir(x, y)
int x, y;
{
	coord cc;
	register struct rm *door;
	struct monst *mtmp;

	if (nohands(youracedata)) {
	    You_cant("open anything -- you have no hands!");
	    return 0;
	}
	if(!freehand()){
	    You_cant("open anything -- you have no free hands!");
		return(0);
	}

	if (u.utrap && u.utraptype == TT_PIT) {
	    You_cant("reach over the edge of the pit.");
	    return 0;
	}
	
	if(x > 0 && y > 0){
		cc.x = x;
		cc.y = y;
	} else if(!get_adjacent_loc((char *)0, (char *)0, u.ux, u.uy, &cc)) return(0);

	if((cc.x == u.ux) && (cc.y == u.uy)) return(0);

	if ((mtmp = m_at(cc.x,cc.y))			&&
		mtmp->m_ap_type == M_AP_FURNITURE	&&
		(mtmp->mappearance == S_hcdoor ||
			mtmp->mappearance == S_vcdoor)	&&
		!Protection_from_shape_changers)	 {

	    stumble_onto_mimic(mtmp);
	    return(1);
	}

	door = &levl[cc.x][cc.y];

	if(!IS_DOOR(door->typ)) {
		if (is_db_wall(cc.x,cc.y)) {
		    There("is no obvious way to open the drawbridge.");
		    return(0);
		}
		You("%s no door there.",
				Blind ? "feel" : "see");
		return(0);
	}

	if (!(door->doormask & D_CLOSED)) {
	    const char *mesg;

	    switch (door->doormask) {
	    case D_BROKEN: mesg = " is broken"; break;
	    case D_NODOOR: mesg = "way has no door"; break;
	    case D_ISOPEN: mesg = " is already open"; break;
	    default:	   mesg = " is locked"; break;
	    }
	    pline("This door%s.", mesg);
	    if (Blind) feel_location(cc.x,cc.y);
	    return(0);
	}

	if(verysmall(youracedata)) {
	    pline("You're too small to pull the door open.");
	    return(0);
	}

	/* door is known to be CLOSED */
	if (rnl(20) < (ACURRSTR+ACURR(A_DEX)+ACURR(A_CON))/3) {
	    pline_The("door opens.");
	    if(door->doormask & D_TRAPPED) {
		b_trapped("door", FINGER);
		door->doormask = D_NODOOR;
		if (*in_rooms(cc.x, cc.y, SHOPBASE)) add_damage(cc.x, cc.y, 0L);
	    } else
		door->doormask = D_ISOPEN;
	    if (Blind)
		feel_location(cc.x,cc.y);	/* the hero knows she opened it  */
	    else
		newsym(cc.x,cc.y);
	    unblock_point(cc.x,cc.y);		/* vision: new see through there */
	} else {
	    exercise(A_STR, TRUE);
	    pline_The("door resists!");
	}

	return(1);
}

STATIC_OVL
boolean
obstructed(x,y)
register int x, y;
{
	register struct monst *mtmp = m_at(x, y);

	if(mtmp && mtmp->m_ap_type != M_AP_FURNITURE) {
		if (mtmp->m_ap_type == M_AP_OBJECT) goto objhere;
		pline("%s stands in the way!", !canspotmon(mtmp) ?
			"Some creature" : Monnam(mtmp));
		if (!canspotmon(mtmp))
		    map_invisible(mtmp->mx, mtmp->my);
		return(TRUE);
	}
	if (OBJ_AT(x, y)) {
objhere:	pline("%s's in the way.", Something);
		return(TRUE);
	}
	return(FALSE);
}

int
doclose()		/* try to close a door */
{
	register int x, y;
	register struct rm *door;
	struct monst *mtmp;

	if (nohands(youracedata)) {
	    You_cant("close anything -- you have no hands!");
	    return 0;
	}
	if(!freehand()){
	    You_cant("close anything -- you have no free hands!");
		return(0);
	}

	if (u.utrap && u.utraptype == TT_PIT) {
	    You_cant("reach over the edge of the pit.");
	    return 0;
	}

	if(!getdir((char *)0)) return(0);

	x = u.ux + u.dx;
	y = u.uy + u.dy;
	if((x == u.ux) && (y == u.uy)) {
		You("are in the way!");
		return(1);
	}

	if ((mtmp = m_at(x,y))				&&
		mtmp->m_ap_type == M_AP_FURNITURE	&&
		(mtmp->mappearance == S_hcdoor ||
			mtmp->mappearance == S_vcdoor)	&&
		!Protection_from_shape_changers)	 {

	    stumble_onto_mimic(mtmp);
	    return(1);
	}

	door = &levl[x][y];

	if(!IS_DOOR(door->typ)) {
		if (door->typ == DRAWBRIDGE_DOWN)
		    There("is no obvious way to close the drawbridge.");
		else
		    You("%s no door there.",
				Blind ? "feel" : "see");
		return(0);
	}

	if(door->doormask == D_NODOOR) {
	    pline("This doorway has no door.");
	    return(0);
	}

	if(obstructed(x, y)) return(0);

	if(door->doormask == D_BROKEN) {
	    pline("This door is broken.");
	    return(0);
	}

	if(door->doormask & (D_CLOSED | D_LOCKED)) {
	    pline("This door is already closed.");
	    return(0);
	}

	if(door->doormask == D_ISOPEN) {
	    if(verysmall(youracedata)
#ifdef STEED
		&& !u.usteed
#endif
		) {
		 pline("You're too small to push the door closed.");
		 return(0);
	    }
	    if (
#ifdef STEED
		 u.usteed ||
#endif
		rn2(25) < (ACURRSTR+ACURR(A_DEX)+ACURR(A_CON))/3) {
		pline_The("door closes.");
		door->doormask = D_CLOSED;
		if (Blind)
		    feel_location(x,y);	/* the hero knows she closed it */
		else
		    newsym(x,y);
		block_point(x,y);	/* vision:  no longer see there */
	    }
	    else {
	        exercise(A_STR, TRUE);
	        pline_The("door resists!");
	    }
	}

	return(1);
}

boolean			/* box obj was hit with spell effect otmp */
boxlock(obj, otmp)	/* returns true if something happened */
register struct obj *obj, *otmp;	/* obj *is* a box */
{
	register boolean res = 0;

	switch(otmp->otyp) {
	case WAN_LOCKING:
	case SPE_WIZARD_LOCK:
	    if (!obj->olocked) {	/* lock it; fix if broken */
		pline("Klunk!");
		obj->olocked = 1;
		obj->obroken = 0;
		res = 1;
	    } /* else already closed and locked */
	    break;
	case WAN_OPENING:
	case SPE_KNOCK:
	    if (obj->olocked) {		/* unlock; couldn't be broken */
		pline("Klick!");
		obj->olocked = 0;
		res = 1;
	    } else			/* silently fix if broken */
		obj->obroken = 0;
	    break;
	case WAN_POLYMORPH:
	case SPE_POLYMORPH:
	default: /*default behavior is to rest picking and clear old context*/
	    /* maybe start unlocking chest, get interrupted, then zap it;
	       we must avoid any attempt to resume unlocking it */
	    if (xlock.box == obj)
		reset_pick();
	    break;
	}
	return res;
}

boolean			/* Door/secret door was hit with spell effect otmp */
doorlock(otmp,x,y)	/* returns true if something happened */
struct obj *otmp;
int x, y;
{
	register struct rm *door = &levl[x][y];
	boolean res = TRUE;
	int loudness = 0;
	const char *msg = (const char *)0;
	const char *dustcloud = "A cloud of dust";
	const char *quickly_dissipates = "quickly dissipates";
	int key = artifact_door(x, y);		/* ALI - Artifact doors from slash'em */
	
	if (door->typ == SDOOR) {
	    switch (otmp->otyp) {
	    case WAN_OPENING:
	    case SPE_KNOCK:
	    case WAN_STRIKING:
	    case SPE_FORCE_BOLT:
		if (key)	/* Artifact doors are revealed only (slash'em)*/
		    cvt_sdoor_to_door(door);
		else{
			door->typ = DOOR;
			door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
		}
		newsym(x,y);
		if (cansee(x,y)) pline("A door appears in the wall!");
		if (otmp->otyp == WAN_OPENING || otmp->otyp == SPE_KNOCK)
		    return TRUE;
		break;		/* striking: continue door handling below */
	    case WAN_LOCKING:
	    case SPE_WIZARD_LOCK:
	    default:
		return FALSE;
	    }
	}

	switch(otmp->otyp) {
	case WAN_LOCKING:
	case SPE_WIZARD_LOCK:
#ifdef REINCARNATION
	    if (Is_rogue_level(&u.uz)) {
	    	boolean vis = cansee(x,y);
		/* Can't have real locking in Rogue, so just hide doorway */
		if (vis) pline("%s springs up in the older, more primitive doorway.",
			dustcloud);
		else
			You_hear("a swoosh.");
		if (obstructed(x,y)) {
			if (vis) pline_The("cloud %s.",quickly_dissipates);
			return FALSE;
		}
		block_point(x, y);
		door->typ = SDOOR;
		if (vis) pline_The("doorway vanishes!");
		newsym(x,y);
		return TRUE;
	    }
#endif
	    if (obstructed(x,y)) return FALSE;
	    /* Don't allow doors to close over traps.  This is for pits */
	    /* & trap doors, but is it ever OK for anything else? */
	    if (t_at(x,y)) {
		/* maketrap() clears doormask, so it should be NODOOR */
		pline(
		"%s springs up in the doorway, but %s.",
		dustcloud, quickly_dissipates);
		return FALSE;
	    }

	    switch (door->doormask & ~D_TRAPPED) {
	    case D_CLOSED:
		if (key)
		    msg = "The door closes!";
		else{
			msg = "The door locks!";
			if(u.sealsActive&SEAL_OTIAX) unbind(SEAL_OTIAX,TRUE);
		}break;
	    case D_ISOPEN:
		if (key)
		    msg = "The door swings shut!";
		else{
			msg = "The door swings shut, and locks!";
			if(u.sealsActive&SEAL_OTIAX) unbind(SEAL_OTIAX,TRUE);
		}break;
	    case D_BROKEN:
			msg = "The broken door reassembles and locks!";
			if(u.sealsActive&SEAL_OTIAX) unbind(SEAL_OTIAX,TRUE);
		break;
	    case D_NODOOR:
		if (key)
		    msg = "The broken door reassembles!";
		else{
			msg = "The broken door reassembles and locks!";
			if(u.sealsActive&SEAL_OTIAX) unbind(SEAL_OTIAX,TRUE);
		}break;
		break;
	    default:
		res = FALSE;
		break;
	    }
	    block_point(x, y);
	    if (key)
			door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
	    else
		    door->doormask = D_LOCKED | (door->doormask & D_TRAPPED);
	    newsym(x,y);
	    break;
	case WAN_OPENING:
	case SPE_KNOCK:
	    if (!key && door->doormask & D_LOCKED) {
		msg = "The door unlocks!";
		door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
	    } else res = FALSE;
	break;
	case WAN_STRIKING:
	case SPE_FORCE_BOLT:
	    if (!key && door->doormask & (D_LOCKED | D_CLOSED)) {
		if (door->doormask & D_TRAPPED) {
		    if (MON_AT(x, y))
			(void) mb_trapped(m_at(x,y));
		    else if (flags.verbose) {
			if (cansee(x,y))
			    pline("KABOOM!!  You see a door explode.");
			else if (flags.soundok)
			    You_hear("a distant explosion.");
		    }
		    door->doormask = D_NODOOR;
		    unblock_point(x,y);
		    newsym(x,y);
		    loudness = 40;
		    break;
		}
		door->doormask = D_BROKEN;
		if (flags.verbose) {
		    if (cansee(x,y))
			pline_The("door crashes open!");
		    else if (flags.soundok)
			You_hear("a crashing sound.");
		}
		unblock_point(x,y);
		newsym(x,y);
		/* force vision recalc before printing more messages */
		if (vision_full_recalc) vision_recalc(0);
		loudness = 20;
	    } else res = FALSE;
	    break;
	default: impossible("magic (%d) attempted on door.", otmp->otyp);
	    break;
	}
	if (msg && cansee(x,y)) pline1(msg);
	if (loudness > 0) {
	    /* door was destroyed */
	    wake_nearto_noisy(x, y, loudness);
	    if (*in_rooms(x, y, SHOPBASE)) add_damage(x, y, 0L);
	}

	if (res && picking_at(x, y)) {
	    /* maybe unseen monster zaps door you're unlocking */
	    stop_occupation();
	    reset_pick();
	}
	return res;
}

STATIC_OVL void
chest_shatter_msg(otmp)
struct obj *otmp;
{
	const char *disposition;
	const char *thing;
	long save_Blinded;

	if (otmp->oclass == POTION_CLASS) {
		You("%s %s shatter!", Blind ? "hear" : "see", an(bottlename()));
		if (!breathless(youracedata) || haseyes(youracedata))
			potionbreathe(otmp);
		return;
	}
	/* We have functions for distant and singular names, but not one */
	/* which does _both_... */
	save_Blinded = Blinded;
	Blinded = 1;
	thing = singular(otmp, xname);
	Blinded = save_Blinded;
	switch (otmp->obj_material) {
	case PAPER:	disposition = "is torn to shreds";
		break;
	case WAX:	disposition = "is crushed";
		break;
	case VEGGY:	disposition = "is pulped";
		break;
	case FLESH:	disposition = "is mashed";
		break;
	case GLASS:	disposition = "shatters";
		break;
	case OBSIDIAN_MT:	disposition = "shatters";
		break;
	case WOOD:	disposition = "splinters to fragments";
		break;
	default:	disposition = "is destroyed";
		break;
	}
	pline("%s %s!", An(thing), disposition);
}

/* ALI - Kevin Hugo's artifact doors.
 * Return the artifact which unlocks the door at (x, y), or
 * zero if it is an ordinary door.
 * Note: Not all doors are listed in the doors array (eg., doors
 * dynamically converted from secret doors). Since only trapped
 * and artifact doors are needed this isn't a problem. If we ever
 * implement trapped secret doors we will have to extend this.
 */

int
artifact_door(x, y)
int x, y;
{
    int i;

    for(i = 0; i < doorindex; i++) {
		if (x == doors[i].x && y == doors[i].y){
			return doors[i].arti_text;
		}
    }
    return 0;
}

#endif /* OVLB */

/*lock.c*/
