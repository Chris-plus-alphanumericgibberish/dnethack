#include "hack.h"
#include "xhity.h"

STATIC_DCL void FDECL(do_digging_projectile, (struct monst *, struct obj *, int, int));
STATIC_DCL void FDECL(destroy_projectile, (struct monst *, struct obj *));
STATIC_DCL void FDECL(end_projectile, (struct monst *, struct monst *, struct obj *, struct obj *, boolean, boolean, boolean *));
STATIC_DCL int FDECL(projectile_attack, (struct monst *, struct monst *, struct obj *, struct obj *, boolean, int*, int*, int*, int*, boolean, boolean*));
STATIC_DCL void FDECL(quest_art_swap, (struct obj *, struct monst *, boolean *));
STATIC_DCL void FDECL(sho_obj_return, (struct obj *, int, int));
STATIC_DCL void FDECL(return_thrownobj, (struct monst *, struct obj *));
STATIC_DCL boolean FDECL(toss_up2, (struct obj *));
STATIC_DCL int FDECL(calc_multishot, (struct monst *, struct obj *, struct obj *, int));
STATIC_DCL int FDECL(calc_range, (struct monst *, struct obj *, struct obj *, int *));
STATIC_DCL boolean FDECL(uthrow, (struct obj *, struct obj *, int, boolean));
STATIC_DCL boolean FDECL(misthrow, (struct monst *, struct obj *, struct obj *, boolean, int *, int *, int *));
STATIC_DCL int FDECL(mthrow, (struct monst *, struct obj *, struct obj *, int, int, boolean));
STATIC_DCL struct obj * FDECL(blaster_ammo, (struct obj *));

/* grab some functions from dothrow.c */
extern boolean FDECL(quest_arti_hits_leader, (struct obj *, struct monst *));
extern int FDECL(gem_accept, (struct monst *, struct obj *));
extern void FDECL(check_shop_obj, (struct obj *, XCHAR_P, XCHAR_P, BOOLEAN_P));
extern void FDECL(breakmsg, (struct obj *, BOOLEAN_P));
extern void FDECL(breakobj, (struct obj *, XCHAR_P, XCHAR_P, BOOLEAN_P, BOOLEAN_P));
extern int FDECL(throw_gold, (struct obj *));
extern void NDECL(autoquiver);
/* from mthrowu.c */
extern char* FDECL(breathwep, (int));

/* some damn global variables because passing these as parameters would be a lot to add for something so rarely used.
 * The player threw an object, these save what the player's state was just prior to throwing so it can be restored */
static boolean takenfromyourinv = FALSE;	/* tracks if a projectile originated in your pack. used to track MAD_TALONS */
static boolean u_was_twoweap;
static boolean u_was_swallowed;
static long old_wep_mask;

/* projectile()
 * 
 * Omnibus projectile firing function
 * 
 * returns MM_HIT/MM_DEF_DIED-etc values IF it hit a creature, otherwise returns MM_MISS (0).
 * 
 * Weightlessness/hurtling must be dealt with outside of this function;
 * it does not assume magr is actually firing the projectile themselves
 */
int
projectile(magr, ammo, launcher, fired, initx, inity, dx, dy, dz, initrange, forcedestroy, verbose, impaired)
struct monst * magr;			/* Creature responsible for the projectile. Can be non-existant. */
struct obj * ammo;				/* Projectile object. Must exist. May be in an inventory, or free, or anywhere. */
struct obj * launcher;			/* Launcher for the projectile. Can be non-existant. Implies "fired" is true. */
boolean fired;					/* Whether or not the projectile was fired (ex arrow from a bow). Fired without a launcher is possible (ex AT_ARRW). */
int initx;						/* x; Where the projectile originates from. Does not hit this location. */
int inity;						/* y; Where the projectile originates from. Does not hit this location. */
int dx;							/* x; Direction of projectile's movement */
int dy;							/* y; Direction of projectile's movement */
int dz;							/* z; Direction of projectile's movement */
int initrange;					/* Maximum range for projectile */
boolean forcedestroy;			/* TRUE if projectile should be forced to be destroyed at the end */
boolean verbose;				/* TRUE if messages should be printed even if the player can't see what happened */
boolean impaired;				/* TRUE if throwing/firing slipped OR magr is confused/stunned/etc */
{
	boolean youagr = (magr && magr == &youmonst);
	struct obj * thrownobj;				/* singular fired/thrown object */
	boolean onlyone;					/* if ammo only consists of thrownobj */
	boolean wepgone = FALSE;			/* TRUE if thrownobj is destroyed */
	boolean returning;					/* TRUE if projectile should magically return to magr (like Mjollnir) */
	struct monst * mdef = (struct monst *)0;
	int result = 0;
	int range = initrange;
	int boomerang_init;
	bhitpos.x = initx;
	bhitpos.y = inity;

	if (!ammo) {
		/* if we are out of ammo, can't fire */
		return MM_MISS;
	}
	if (launcher && !fired) {
		impossible("projectile() called with launcher but not fired?");
		return MM_MISS;
	}
	/* If player is doing the throwing, save some details like if they were twoweaponing and what slot the object was in */
	if (youagr) {
		u_was_twoweap = u.twoweap;
		u_was_swallowed = u.uswallow;
		old_wep_mask = ammo->owornmask;
	}

	/* First, create thrownobj */
	{
		int n_to_throw = 1;

		/* Fluorite Octahedron has obj->quan >1 possible */
		if (m_shot.o && (m_shot.o == ammo->otyp) && (ammo->oartifact == ART_FLUORITE_OCTAHEDRON) && !m_shot.s) {
			n_to_throw = m_shot.n;
			m_shot.n = 1;
		}

		/* Blasters create ammo as needed */
		/* we also need to leave the ammo object left behind so it can be cleaned up */
		if (launcher && is_blaster(launcher)) {
			if (launcher->ovar1 > 0) {
				launcher->ovar1--;
				ammo->quan++;
			}
			else {
				/* out of charge! */
				return MM_MISS;
			}
		}

		if (ammo->quan > n_to_throw) {
			/* split off 1 object */
			thrownobj = splitobj(ammo, n_to_throw);
			onlyone = FALSE;
		}
		else {
			/* there is just the one being thrown */
			thrownobj = ammo;
			onlyone = TRUE;
		}
	}

	/* clean up where it came from */
	switch (thrownobj->where)
	{
	case OBJ_FREE:		/* object not attached to anything */
		/* nothing required */
		break;
	case OBJ_FLOOR:		/* object on floor */
		obj_extract_self(thrownobj);
		break;
	case OBJ_INVENT:	/* object in the hero's inventory */
		takenfromyourinv = TRUE;
		if (onlyone) {
			if (thrownobj->owornmask)
				remove_worn_item(thrownobj, FALSE);
		}
		freeinv(thrownobj);
		break;
	case OBJ_MINVENT:	/* object in a monster inventory */
		if (!magr) {
			impossible("thrownobj in monster's inventory with no monster?");
			return 0;
		}
		if (onlyone) {
			if (MON_WEP(magr) == thrownobj) {
				setmnotwielded(magr, thrownobj);
				MON_NOWEP(magr);
			}
			if (MON_SWEP(magr) == thrownobj) {
				setmnotwielded(magr, thrownobj);
				MON_NOSWEP(magr);
			}
		}
		m_freeinv(thrownobj);
		break;
	default:			/* object elsewhere (migrating? contained?) */
		impossible("unhandled where of thrown item, %d", thrownobj->where);
		break;
	}
	thrownobj->owornmask &= ~(W_CHAIN|W_BALL);	/* balls and chains are still attached, other objects aren't*/
	/* set that it was thrown... if the player threw it */
	if (youagr) {
		thrownobj->was_thrown = TRUE;
	}

	/* determine if thrownobj should return (like Mjollnir) */
	if (magr && (
		(Race_if(PM_ANDROID) && !fired && youagr) ||	/* there's no android monster helper? */
		(thrownobj->oartifact == ART_MJOLLNIR && (youagr ? (Role_if(PM_VALKYRIE)) : magr ? (magr->data == &mons[PM_VALKYRIE]) : FALSE)) ||
		(thrownobj->oartifact == ART_AXE_OF_THE_DWARVISH_LORDS && (youagr ? (Race_if(PM_DWARF)) : magr ? (is_dwarf(magr->data)) : FALSE)) ||
		thrownobj->oartifact == ART_SICKLE_MOON ||
		thrownobj->oartifact == ART_ANNULUS ||
		thrownobj->oartifact == ART_KHAKKHARA_OF_THE_MONKEY ||
		thrownobj->oartifact == ART_DART_OF_THE_ASSASSIN ||
		thrownobj->oartifact == ART_WINDRIDER ||
		thrownobj->oartifact == ART_AMHIMITL
		)) {
		returning = TRUE;
	}
	else {
		returning = FALSE;
	}

	/* player exercises STR just be throwing heavy things */
	if (youagr && !fired && (
		thrownobj->otyp == BOULDER ||
		(thrownobj->otyp == STATUE && is_boulder(thrownobj)) ||
		thrownobj->otyp == HEAVY_IRON_BALL
		))
	{
		exercise(A_STR, TRUE);
	}

	/* if the player is swallowed, projectile is guaranteed to hit engulfer and no more */
	if (youagr && u.uswallow) {
		mdef = u.ustuck;
		bhitpos.x = x(mdef);
		bhitpos.y = y(mdef);
		result = projectile_attack(magr, mdef, thrownobj, launcher, fired, &dx, &dy, &range, &initrange, forcedestroy, &wepgone);
		end_projectile(magr, mdef, thrownobj, launcher, fired, forcedestroy, &wepgone);
		return result;
	}

	/* z direction */
	if (dz < 0) {
		/* up -- PLAYER ONLY */
		/* random slips can cause projectiles to go down, though */

		/* fired bullets should always disappear without a message */
		if (fired && is_bullet(thrownobj))
			return MM_MISS;

		/* if we are outdoors... */
		if (In_outdoors(&u.uz)) {
			/* some projectiles will explode mid-air */
			if (thrownobj->otyp == ROCKET || thrownobj->otyp == BLASTER_BOLT || thrownobj->otyp == HEAVY_BLASTER_BOLT) {
				pline("%s explodes harmlessly far up in the air.", Doname2(thrownobj));
				obfree(thrownobj, 0);
				return MM_MISS;
			}
			/* or go silently */
			else if (thrownobj->otyp == LASER_BEAM) {
				obfree(thrownobj, 0);
				return MM_MISS;
			}
		}
		/* or if we are indoors... */
		else {
			/* some projectiles will always hit the ceiling */
			if (thrownobj->otyp == ROCKET || thrownobj->otyp == BLASTER_BOLT || thrownobj->otyp == HEAVY_BLASTER_BOLT) {
				pline("%s the %s and explodes!", Tobjnam(thrownobj, "hit"), ceiling(bhitpos.x, bhitpos.y));
				destroy_projectile(magr, thrownobj);
				return MM_MISS;
			}
			/* or have special effects */
			else if (thrownobj->otyp == LASER_BEAM) {
				obfree(thrownobj, 0);
				/* loosen a rock from the ceiling */
				zap_dig(-1, -1, -1);	/* assumes player */
				return MM_MISS;
			}
		}

		/* weapon might return to your hand */
		if (returning && !impaired) {
			if (!In_outdoors(&u.uz))
				pline("%s the %s and returns to your hand!", Tobjnam(thrownobj, "hit"), ceiling(bhitpos.x, bhitpos.y));
			else 
				pline("%s to your hand!", Tobjnam(thrownobj, "return"));
			return_thrownobj(magr, thrownobj);
			return MM_MISS;
		}

		/* otherwise do the standard (player-only) toss upwards */
		if (!(Weightless || Underwater || Is_waterlevel(&u.uz))) {
			wepgone |= ~toss_up2(thrownobj);
		}
		end_projectile(magr, mdef, thrownobj, launcher, fired, forcedestroy, &wepgone);
		return MM_MISS;
	}
	else if (dz > 0) {
		/* down */
		/* random slips can cause projectiles to go down, though */

		/* fired bullets should specifically disappear without a message */
		if (fired && is_bullet(thrownobj))
			return MM_MISS;

		/* returning weapons don't return when thrown downwards */

		/* some projectiles have special effects */
		if (thrownobj->otyp == ROCKET || thrownobj->otyp == BLASTER_BOLT || thrownobj->otyp == HEAVY_BLASTER_BOLT) {
			if (youagr || couldsee(bhitpos.x, bhitpos.y))
				pline("%s the %s and explodes!", Tobjnam(thrownobj, "hit"), surface(bhitpos.x, bhitpos.y));
			destroy_projectile(magr, thrownobj);
			return MM_MISS;
		}
		/* or have special effects */
		else if (thrownobj->otyp == LASER_BEAM) {
			obfree(thrownobj, 0);
			/* no, you can't use this for quick escapes */
			if (youagr && !Weightless) {
				pline("The laser beam is absorbed by the %s.", surface(bhitpos.x, bhitpos.y));
			}
			return MM_MISS;
		}
		/* Projectile hits floor. This calls end_projectile() */
		hitfloor2(magr, thrownobj, launcher, fired, forcedestroy, &wepgone);
		return MM_MISS;
	}

	/* set projectile glyph to show */
	tmp_at(DISP_FLASH, obj_to_glyph(thrownobj));

	/* initialize boomerang thrown direction */
	if (is_boomerang(thrownobj)) {
		for (boomerang_init = 0; boomerang_init < 8; boomerang_init++)
		if (xdir[boomerang_init] == dx && ydir[boomerang_init] == dy)
			break;
	}

	/* move the projectile loop */
	/* always ends via break */
	while (TRUE)
	{
		/* boomerangs: change dx/dy to make signature circle */
		if (is_boomerang(thrownobj)) {
			/* assumes boomerangs always start with 10 range */
			/* don't worry about the math; it works */
			dx = xdir[((10-range) - (10-range+4)/5 + boomerang_init) % 8];
			dy = ydir[((10-range) - (10-range+4)/5 + boomerang_init) % 8];
			/* TODO: spinny glyphs for all possible boomerangs? */
			/* by not doing the spinny )()()( nonsense we can easily show a properly silver chakram */
		}

		/* priority order: */
		/* pickaxes in shops special case */
		/* space has monster and is not initxy */
		/* space has sink/wall and is not initxy */
		/* space ahead has wall and no monster */
		/* space ahead has iron bars and no monster */
		/* heavy iron ball specific checks */
		/* out of range */
		/* actually move the projectile */

		if ((is_pick(thrownobj)) &&
			(inside_shop(bhitpos.x, bhitpos.y)) &&
			(shkcatch(thrownobj, bhitpos.x, bhitpos.y))) {
			/* shopkeeper caught it */
			wepgone = TRUE;
			break;
		}

		/* projectile is on a creature */
		if ((range != initrange || initrange == 0) &&
			(mdef = creature_at(bhitpos.x, bhitpos.y)))
		{
			result = projectile_attack(magr, mdef, thrownobj, launcher, fired, &dx, &dy, &range, &initrange, forcedestroy, &wepgone);

			if (result)
			{
				break;
			}
		}

		/* projectile is on a sink (it "sinks" down) or is on a non-allowable square */
		if ((range != initrange || initrange == 0) &&
			(!ZAP_POS(levl[bhitpos.x][bhitpos.y].typ)) ||
			closed_door(bhitpos.x, bhitpos.y) ||
			(IS_SINK(levl[bhitpos.x][bhitpos.y].typ)))
		{
			range = 0;
		}

		/* space ahead has wall and no monster */
		if (!isok(bhitpos.x + dx, bhitpos.y + dy) ||
			((!ZAP_POS(levl[bhitpos.x + dx][bhitpos.y + dy].typ) || closed_door(bhitpos.x + dx, bhitpos.y + dy)) &&
			!m_at(bhitpos.x+dx, bhitpos.y+dy)))
		{
			do_digging_projectile(magr, thrownobj, dx, dy);
			range = 0;
		}
		/* space ahead has iron bars and no monster */
		/* 1/5 chance for 'small' objects (see hits_bars), unless if launched from right beside the iron bars */
		if (levl[bhitpos.x + dx][bhitpos.y + dy].typ == IRONBARS &&
			!m_at(bhitpos.x + dx, bhitpos.y + dy) && 
			hits_bars(
				/* object fired   */ &thrownobj,
				/* current coords */ bhitpos.x, bhitpos.y,
				/* force hit?     */ Is_illregrd(&u.uz) || ((bhitpos.x == initx && bhitpos.y == inity) ? 0 : !rn2(5)),
				/* player caused  */ (magr == &youmonst))
			) {
			do_digging_projectile(magr, thrownobj, dx, dy);
			range = 0;
		}

		/* limit range of iron balls so hero won't make an invalid move */
		if (range > 0 && thrownobj && thrownobj->otyp == HEAVY_IRON_BALL) {
			struct obj *bobj;
			struct trap *t;
			if ((bobj = boulder_at(bhitpos.x, bhitpos.y)) != 0) {
				if (cansee(bhitpos.x, bhitpos.y))
					pline("%s hits %s.",
					The(distant_name(thrownobj, xname)), an(xname(bobj)));
				range = 0;
			}
			else if (thrownobj == uball) {
				if (!test_move(bhitpos.x - dx, bhitpos.y - dy, dx, dy, TEST_MOVE)) {
					/* nb: it didn't hit anything directly */
					if (cansee(bhitpos.x, bhitpos.y)) {
						pline("%s jerks to an abrupt halt.",
							The(distant_name(thrownobj, xname))); /* lame */
					}
					range = 0;
				}
				else if (In_sokoban(&u.uz) && (t = t_at(bhitpos.x, bhitpos.y)) != 0 &&
					(t->ttyp == PIT || t->ttyp == SPIKED_PIT ||
					t->ttyp == HOLE || t->ttyp == TRAPDOOR)) {
					/* hero falls into the trap, so ball stops */
					range = 0;
				}
			}
		}

		/* no projectile -- something destroyed it */
		if (!thrownobj)
		{
			break;
		}

		/* got past everything */
		if (range < 1)
		{
			/* out of range, end loop */
			break;
		}
		else {
			/* otherwise move the projectile */
			range--;
			bhitpos.x += dx;
			bhitpos.y += dy;
			tmp_at(bhitpos.x, bhitpos.y);
			if (cansee(bhitpos.x, bhitpos.y))
				delay_output();
		}
	}
	/* end glyph showing */
	tmp_at(DISP_END, 0);

	/* some artifacts magically return to the location they were thrown from */
	if (returning && !wepgone) {
		/* show object's return flight */
		sho_obj_return(thrownobj, initx, inity);
		/* move it */
		bhitpos.x = initx;
		bhitpos.y = inity;
		/* check to see if magr is still there */
		if (magr && x(magr) == bhitpos.x && y(magr) == bhitpos.y)
		{
			/* attempt to catch it */
			if (!impaired) {
				/* success */
				if (youagr) {
					pline("%s to your hand!", Tobjnam(thrownobj, "return"));
				}
				return_thrownobj(magr, thrownobj);
			}
			else {
				/* failure */
				if (youagr) {
					/* straight copy of old failed-catch code */
					/* scope creep: use hmon2 */
					int dmg = rn2(2);
					if (!dmg) {
						pline(Blind ? "%s lands %s your %s." :
							"%s back to you, landing %s your %s.",
							Blind ? Something : Tobjnam(thrownobj, "return"),
							Levitation ? "beneath" : "at",
							makeplural(body_part(FOOT)));
					}
					else {
						dmg += rnd(3);
						pline(Blind ? "%s your %s!" :
							"%s back toward you, hitting your %s!",
							Tobjnam(thrownobj, Blind ? "hit" : "fly"),
							body_part(ARM));
						if (thrownobj->oartifact){
							(void)artifact_hit((struct monst *)0,
								&youmonst, thrownobj, &dmg, 0);
						}
						losehp(dmg, xname(thrownobj),
							obj_is_pname(thrownobj) ? KILLED_BY : KILLED_BY_AN);
					}
					/* end copy */
				}
				end_projectile(magr, mdef, thrownobj, launcher, fired, forcedestroy, &wepgone);
			}
			return result;
		}
		/* else just drop it on the floor */
	}

	/* end the projectile */
	end_projectile(magr, mdef, thrownobj, launcher, fired, forcedestroy, &wepgone);
	return result;
}

/*
 * do_digging_projectile()
 *
 * Called when the projectile has been stopped by iron bars or some non-ZAP_POS.
 * Some projectiles perform a digging operation.
 */
void
do_digging_projectile(magr, thrownobj, dx, dy)
struct monst * magr;			/* Creature responsible for the projectile. Might not exist. */
struct obj * thrownobj;			/* Projectile object. Must be free. Will no longer exist after this function. */
int dx;							/* */
int dy;							/* */
{
	boolean youagr = (magr && (magr == &youmonst));
	int newx = bhitpos.x + dx;
	int newy = bhitpos.y + dy;
	struct rm *room = &levl[newx][newy];
	boolean shopdoor = FALSE, shopwall = FALSE;

	/* Projectile must be a digger */
	if (!((thrownobj->otyp == BLASTER_BOLT || thrownobj->otyp == HEAVY_BLASTER_BOLT || thrownobj->otyp == LASER_BEAM)))
		return;

	/* Doors (but not artifact doors) */
	if ((closed_door(newx, newy) || room->typ == SDOOR) &&
		!artifact_door(newx, newy)) 
	{
		/* message */
		if (cansee(newx, newy)) (thrownobj->otyp == LASER_BEAM) ?
			pline("The %s cuts the door into chunks!", xname(thrownobj)) :
			pline("The door is blown to splinters by the impact!");
		/* check shops */
		if (*in_rooms(newx, newy, SHOPBASE)) {
			add_damage(newx, newy, (youagr ? 400L : 0L));
			shopwall = TRUE;
		}
		/* anger watch */
		if (youagr)
			watch_dig((struct monst *)0, newx, newy, TRUE);
		/* create doorway */
		room->typ = DOOR;
		room->doormask = D_NODOOR;
		/* update vision */
		unblock_point(newx, newy);
		if (!Blind && cansee(newx, newy))
			newsym(newx, newy);
	}
	/* Walls (reduced chance for blasters) */
	else if (IS_WALL(room->typ) && may_dig(newx, newy) && (
		(thrownobj->otyp == LASER_BEAM) ||
		(thrownobj->otyp == BLASTER_BOLT && !rn2(20)) ||
		(thrownobj->otyp == HEAVY_BLASTER_BOLT && !rn2(5))
		)) {
		struct obj * otmp;	/* newly-created rocks */
		/* message */
		if (cansee(newx, newy)) (thrownobj->otyp == LASER_BEAM) ?
			pline("The %s cuts the wall into chunks!", xname(thrownobj)) :
			pline("The wall blows apart from the impact!");
		/* check shops */
		if (*in_rooms(newx, newy, SHOPBASE)) {
			add_damage(newx, newy, youagr ? 200L : 0L);
			shopwall = TRUE;
		}
		/* anger watch */
		if (youagr)
			watch_dig((struct monst *)0, newx, newy, TRUE);
		/* create opening */
		if (level.flags.is_cavernous_lev && !in_town(newx, newy)) {
			room->typ = CORR;
		}
		else {
			room->typ = DOOR;
			room->doormask = D_NODOOR;
		}
		/* create rocks */
		otmp = mksobj_at(ROCK, newx, newy, TRUE, FALSE);
		otmp->quan = 20L + rnd(20);
		otmp->owt = weight(otmp);
		/* update vision */
		unblock_point(newx, newy);
		if (!Blind && cansee(newx, newy))
			newsym(newx, newy);
	}
	/* Rock (laser cutter only) */
	else if (isok(newx, newy) && IS_ROCK(room->typ) && may_dig(newx, newy) &&
		(thrownobj->otyp == LASER_BEAM)
		) {
		struct obj *otmp;	/* newly-created rocks */
		/* message */
		if (cansee(newx, newy))
			pline("The %s cuts the stone into chunks!", xname(thrownobj));
		/* check shops */
		if (*in_rooms(newx, newy, SHOPBASE)) {
			add_damage(newx, newy, youagr ? 200L : 0L);
			shopwall = TRUE;
		}
		/* anger watch */
		if (youagr)
			watch_dig((struct monst *)0, newx, newy, TRUE);
		/* create opening */
		room->typ = CORR;
		/* create rocks */
		otmp = mksobj_at(ROCK, newx, newy, TRUE, FALSE);
		otmp->quan = 20L + rnd(20);
		otmp->owt = weight(otmp);
		/* update vision */
		unblock_point(newx, newy);
		if (!Blind && cansee(newx, newy))
			newsym(newx, newy);
	}
	/* Iron Bars (laser cutter only) */
	else if (isok(newx, newy) && (room->typ == IRONBARS) && 
		(thrownobj->otyp == LASER_BEAM)
		) {
		int numbars;
		struct obj *otmp;
		/* message */
		if (cansee(newx, newy))
			pline("The %s cuts through the bars!", xname(thrownobj));
		/* create opening */
		room->typ = CORR;
		/* create iron bars */
		for (numbars = d(2, 4) - 1; numbars > 0; numbars--){
			otmp = mksobj_at(BAR, newx, newy, FALSE, FALSE);
			otmp->obj_material = IRON;
			otmp->spe = 0;
			otmp->cursed = otmp->blessed = FALSE;
		}
		/* update vision */
		if (!Blind && cansee(newx, newy))
			newsym(newx, newy);
	}

	/* if you damaged a shop, add to bill */
	if (youagr && (shopdoor || shopwall)) (thrownobj->otyp == LASER_BEAM) ?
		pay_for_damage("cut into", FALSE) :
		pay_for_damage("blast into", FALSE);

	return;
}

/*
 * destroy_projectile()
 *
 * Call this when the projectile should immediately cease to exist. Possibly explosively.
 */
void
destroy_projectile(magr, thrownobj)
struct monst * magr;			/* Creature responsible for the projectile. Might not exist. */
struct obj * thrownobj;			/* Projectile object. Must be free. Will no longer exist after this function. */
{
	boolean youagr = (magr && (magr == &youmonst));

	switch (thrownobj->otyp)
	{
	case FRAG_GRENADE:
	case GAS_GRENADE:
		grenade_explode(thrownobj, bhitpos.x, bhitpos.y, youagr, 0);
		break;

	case ROCKET:
		if (cansee(bhitpos.x, bhitpos.y))
			pline("%s explodes in a ball of fire!", Doname2(thrownobj));
		else
			You_hear("an explosion");
		explode(bhitpos.x, bhitpos.y, AD_FIRE, WEAPON_CLASS,
			d(3, 8), EXPL_FIERY, youagr);
		obfree(thrownobj, (struct obj *)0);
		break;

	case BLASTER_BOLT:
	case HEAVY_BLASTER_BOLT:
		explode(bhitpos.x, bhitpos.y, AD_PHYS, 0,
			(thrownobj->otyp == HEAVY_BLASTER_BOLT ? (d(3, 10))   : (d(3, 6))),
			(thrownobj->otyp == HEAVY_BLASTER_BOLT ? (EXPL_FIERY) : (EXPL_RED)),
			youagr);
		obfree(thrownobj, (struct obj *)0);
		break;

	default:
		obfree(thrownobj, (struct obj *)0);
	}
	return;
}

/*
 * end_projectile()
 * 
 * The projectile may or may not have hit a monster, but it is ending.
 * If the projectile had hit a monster, that may have destroyed the projectile.
 * 
 * [magr] might not exist -- if it doesn't exist, we need to be careful what we call
 * [thrownobj] may have been destroyed already -- if so, we just return early, as there's nothing to do
 */
void
end_projectile(magr, mdef, thrownobj, launcher, fired, forcedestroy, wepgone)
struct monst * magr;			/* Creature responsible for the projectile. Might not exist. */
struct monst * mdef;			/* Creature hit by the projectile. Might not exist. */
struct obj * thrownobj;			/* Projectile object. Must be free. At this point, might not exist. */
struct obj * launcher;			/* Launcher for the projectile. Can be non-existant. Implies "fired" is true. */
boolean fired;					/* Whether or not the projectile was fired (ex arrow from a bow). Fired without a launcher is possible (ex AT_ARRW). */
boolean forcedestroy;			/* If TRUE, make sure the projectile is destroyed */
boolean * wepgone;				/* TRUE if projectile is already destroyed */
{
	boolean youagr = (magr && (magr == &youmonst));

	/* need to check that thrownobj hasn't been dealt with already */
	if (*wepgone)
	{
		return;
	}

	/* projectiles that never survive being fired; their special effects are handled in destroy_projectile() */
	if (fired && (
		thrownobj->otyp == ROCKET ||
		thrownobj->otyp == BLASTER_BOLT ||
		thrownobj->otyp == HEAVY_BLASTER_BOLT ||
		is_bullet(thrownobj)
		))
	{
		destroy_projectile(magr, thrownobj);
		*wepgone = TRUE;
		return;
	}

	/* projectiles that should always be destroyed, regardless of fired or whether or not they hit a "soft" surface */
	if (forcedestroy ||
		thrownobj->otyp == LASER_BEAM ||
		thrownobj->otyp == ACID_VENOM ||
		thrownobj->otyp == BLINDING_VENOM ||
		thrownobj->otyp == CREAM_PIE ||
		thrownobj->otyp == EGG
		)
	{
		/* prefer to destroy via breaktest, if possible */
		if (breaktest(thrownobj)) {
			tmp_at(DISP_FLASH, obj_to_glyph(thrownobj));
			tmp_at(bhitpos.x, bhitpos.y);
			if (cansee(bhitpos.x, bhitpos.y))
				delay_output();
			tmp_at(DISP_END, 0);
			breakmsg(thrownobj, cansee(bhitpos.x, bhitpos.y));
			breakobj(thrownobj, bhitpos.x, bhitpos.y, youagr, TRUE);
		}
		else {
			destroy_projectile(magr, thrownobj);
		}
		*wepgone = TRUE;
		return;
	}

	/* if the player was swallowed before throwing the object, special handling */
	if (youagr && u_was_swallowed) {
		if (u.uswallow) {
			/* you are still swallowed */
			pline("%s into %s %s.",
				Tobjnam(thrownobj, "vanish"), s_suffix(mon_nam(u.ustuck)),
				is_animal(u.ustuck->data) ? "entrails" : "currents");
			/* corpses are digested, other objects just stored */
			if (is_animal(u.ustuck->data) &&
				thrownobj->otyp == CORPSE) {
				obfree(thrownobj, 0);
				*wepgone = TRUE;
				return;
			}
			else {
				mpickobj(u.ustuck, thrownobj);
				*wepgone = TRUE;
				return;
			}
		}
		else {
			/* the thing you threw freed you (probably by killing the swallower) */
			/* we still want to remove a corpse thrown into the swallower -- hopefully mdef hasn't been cleaned up yet,
			 * otherwise we'll need to save mdef's data or something */
			if (mdef && is_animal(mdef->data) &&
				thrownobj->otyp == CORPSE) {
				obfree(thrownobj, 0);
				*wepgone = TRUE;
				return;
			}
			/* else go through the rest of the function */
		}
	}

	/* fragile objects shatter */
	if (!IS_SOFT(levl[bhitpos.x][bhitpos.y].typ) &&
		breaktest(thrownobj)) {

		/* Nudzirath effects */
		if (youagr && is_shatterable(thrownobj) && !thrownobj->oerodeproof && u.specialSealsActive&SEAL_NUDZIRATH){
			int dmg, dsize = spiritDsize(), sx, sy;
			struct monst *msmon;
			sx = bhitpos.x;
			sy = bhitpos.y;
			if (thrownobj->otyp == MIRROR){
				if (u.spiritPColdowns[PWR_MIRROR_SHATTER] < monstermoves && !u.uswallow && uwep && uwep->otyp == MIRROR && !(uwep->oartifact)){
					useup(uwep);
					explode(u.ux, u.uy, AD_PHYS, TOOL_CLASS, d(5, dsize), HI_SILVER, 1);
					explode(sx, sy, AD_PHYS, TOOL_CLASS, d(5, dsize), HI_SILVER, 1);

					while (sx != u.ux && sy != u.uy){
						sx -= u.dx;
						sy -= u.dy;
						if (!isok(sx, sy)) break; //shouldn't need this, but....
						else {
							msmon = m_at(sx, sy);
							/* reveal/unreveal invisible msmonsters before tmp_at() */
							if (msmon && !canspotmon(msmon) && cansee(sx, sy))
								map_invisible(sx, sy);
							else if (!msmon && glyph_is_invisible(levl[sx][sy].glyph)) {
								unmap_object(sx, sy);
								newsym(sx, sy);
							}
							if (msmon) {
								if (resists_magm(msmon)) {	/* match effect on player */
									shieldeff(msmon->mx, msmon->my);
								}
								else {
									dmg = d(5, dsize);
									if (hates_silver(msmon->data)){
										dmg += rnd(20);
										pline("The flying shards of mirror sear %s!", mon_nam(msmon));
									}
									else {
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
				}
				else explode(sx, sy, AD_PHYS, TOOL_CLASS, d(rnd(5), dsize), HI_SILVER, 1);
			}
			else if (thrownobj->obj_material == OBSIDIAN_MT) explode(sx, sy, AD_PHYS, WEAPON_CLASS, d(rnd(5), dsize), EXPL_DARK, 1);
		}

		tmp_at(DISP_FLASH, obj_to_glyph(thrownobj));
		tmp_at(bhitpos.x, bhitpos.y);
		if (cansee(bhitpos.x, bhitpos.y))
			delay_output();
		tmp_at(DISP_END, 0);
		breakmsg(thrownobj, cansee(bhitpos.x, bhitpos.y));
		breakobj(thrownobj, bhitpos.x, bhitpos.y, youagr, TRUE);
		*wepgone = TRUE;
		return;
	}

	/* do floor effects (like lava) -- if it returned true the item was destroyed */
	if (flooreffects(thrownobj, bhitpos.x, bhitpos.y, "fall")) {
		*wepgone = TRUE;
		return;
	}
	
	/* note that the item is no longer in anything's inventory */
	obj_no_longer_held(thrownobj);

	/* shopkeepers snatch up pickaxes */
	if (mdef && mdef->isshk && is_pick(thrownobj) && !DEADMONSTER(mdef)) {
		if (cansee(bhitpos.x, bhitpos.y))
			pline("%s snatches up %s.",
			Monnam(mdef), the(xname(thrownobj)));
		if (*u.ushops)
			check_shop_obj(thrownobj, bhitpos.x, bhitpos.y, FALSE);
		(void)mpickobj(mdef, thrownobj);	/* may merge and free obj */
		*wepgone = TRUE;
		return;
	}

	/* candles are snuffed */
	(void)snuff_candle(thrownobj);

	/* grenades are armed */
	if (is_grenade(thrownobj))
		arm_bomb(thrownobj, youagr);

	/* if there wasn't a creature at the projectile's hitspot, it might fall down the stairs */
	if (!mdef && ship_object(thrownobj, bhitpos.x, bhitpos.y, FALSE)) {
		*wepgone = TRUE;
		return;
	}

	/* now, place the object on the floor */
	place_object(thrownobj, bhitpos.x, bhitpos.y);

	/* possibly 'donate' it to a shop */
	if (*u.ushops && thrownobj != uball)
		check_shop_obj(thrownobj, bhitpos.x, bhitpos.y, FALSE);

	/* stack it */
	stackobj(thrownobj);

	/* ball-related stuff */
	if (thrownobj == uball) {
		thrownobj->owornmask = old_wep_mask;
		drop_ball(bhitpos.x, bhitpos.y);
	}

	/* update screen to see the new location of thrownobj */
	if (cansee(bhitpos.x, bhitpos.y))
		newsym(bhitpos.x, bhitpos.y);

	/* recalculate vision if we threw a lightsource */
	if (obj_sheds_light(thrownobj))
		vision_full_recalc = 1;

	/* damage the contents of a container */
	if (!IS_SOFT(levl[bhitpos.x][bhitpos.y].typ))
		container_impact_dmg(thrownobj);

	return;
}

/* hitfloor2()
 * 
 * 
 * 
 */
void
hitfloor2(magr, obj, launcher, fired, forcedestroy, wepgone)
struct monst * magr;
struct obj * obj;
struct obj * launcher;
boolean fired;
boolean forcedestroy;
boolean * wepgone;
{
	boolean youagr = (magr == &youmonst);

	/* specific effects before end_projectile, if thrownobj still exists */
	if (!(*wepgone) && youagr) {
		if (IS_ALTAR(levl[bhitpos.x][bhitpos.y].typ))
			doaltarobj(obj);
		else
			pline("%s hit%s the %s.", Doname2(obj),
			(obj->quan == 1L) ? "s" : "", surface(bhitpos.x, bhitpos.y));
	}
	/* end projectile*/
	end_projectile(magr, (struct monst *)0, obj, launcher, fired, forcedestroy, wepgone);
	return;
}


/* 
 * projectile_attack()
 * 
 * A projectile is going towards mdef
 * 
 * This should end the projectile unless MM_MISS is returned.
 *  
 */
int
projectile_attack(magr, mdef, thrownobj, launcher, fired, pdx, pdy, prange, prange2, forcedestroy, wepgone)
struct monst * magr;			/* Creature responsible for the projectile. Can be non-existant. */
struct monst * mdef;			/* Creature under fire. */
struct obj * thrownobj;			/* Projectile object. Must be free. */
struct obj * launcher;			/* Launcher for the projectile. Can be non-existant. Implies "fired" is true. */
boolean fired;					/* Whether or not the projectile was fired (ex arrow from a bow). Fired without a launcher is possible (ex AT_ARRW). */
int * pdx;						/* pointer to: x; Direction of projectile's movement */
int * pdy;						/* pointer to: y; Direction of projectile's movement */
int * prange;					/* pointer to: Remaining range for projectile */
int * prange2;					/* pointer to: Remaining 2x range for projectile */
boolean forcedestroy;			/* TRUE if projectile should be forced to be destroyed at the end */
boolean * wepgone;				/* pointer to: TRUE if projectile has been destroyed */
{
	int dx		= *pdx;
	int dy		= *pdy;
	int range	= *prange;
	int range2	= *prange2;
	boolean youagr = (magr == &youmonst);
	boolean youdef = (mdef == &youmonst);
	struct permonst * pa = magr ? (youagr ? youracedata : magr->data) : (struct permonst *)0;
	struct permonst * pd = youdef ? youracedata : mdef->data;
	boolean misfired = (!fired && is_ammo(thrownobj) && thrownobj->oclass != GEM_CLASS);
	int result;
	int accuracy;
	int dieroll;
	int vis;

	/* Set up the visibility of action */
	if (youagr || youdef || ((!magr || cansee(x(magr), y(magr))) && cansee(x(mdef), y(mdef))))
	{
		if (youagr || (magr && cansee(x(magr), y(magr)) && canseemon(magr)))
			vis |= VIS_MAGR;
		if (youdef || (cansee(x(mdef), y(mdef)) && canseemon(mdef)))
			vis |= VIS_MDEF;
		if (youagr || youdef || (magr && canspotmon(magr)) || canspotmon(mdef))
			vis |= VIS_NONE;
	}

	/* train player's Shien skill, if player is defending */
	if (youdef && uwep && is_lightsaber(uwep) && litsaber(uwep) && P_SKILL(weapon_type(uwep)) >= P_BASIC){
		if (P_SKILL(FFORM_SHII_CHO) >= P_BASIC){
			if (u.fightingForm == FFORM_SHII_CHO ||
				(u.fightingForm == FFORM_SHIEN && (!uarm || is_light_armor(uarm)))
				) use_skill(FFORM_SHIEN, 1);
		}
	}

	/* sometimes projectiles have an early return */
	/* you can catch your own boomerangs */
	if (youdef && youagr && is_boomerang(thrownobj)) {
		if (!(Fumbling || (!thrownobj->oartifact && rn2(18) >= ACURR(A_DEX)))) {
			/* we catch it */
			You("skillfully catch the %s.", xname(thrownobj));
			*wepgone = TRUE;
			exercise(A_DEX, TRUE);
			return_thrownobj(&youmonst, thrownobj);
			return MM_HIT;
		}
		/* else do the rest of the function, ouch! */
	}
	/* Sansara Mirror */
	if (rn2(2) && (
		(youdef && (
		(uwep && uwep->oartifact == ART_SANSARA_MIRROR) || (uswapwep && uswapwep->oartifact == ART_SANSARA_MIRROR)
		))
		||
		(!youdef && (
		(MON_WEP(mdef) && MON_WEP(mdef)->oartifact == ART_SANSARA_MIRROR) || (MON_SWEP(mdef) && MON_SWEP(mdef)->oartifact == ART_SANSARA_MIRROR)
		))
		))
	{
		/* message */
		if (vis) {
			pline("Sansara twists and reflects.");
		}
		/* return to sender */
		*pdx *= -1;
		*pdy *= -1;
		*prange = *prange2;
		return MM_MISS;
	}
	/* blaster bolts and laser beams are reflected by regular reflection */
	else if ((thrownobj->otyp == LASER_BEAM || thrownobj->otyp == BLASTER_BOLT || thrownobj->otyp == HEAVY_BLASTER_BOLT)
		&& (youdef ? Reflecting : mon_reflects(mdef, (char *)0)))
	{
		boolean shienuse = FALSE;
		/* if the player is using Shien lightsaber form, they can direct the reflection */
		if (youdef && uwep && is_lightsaber(uwep) && litsaber(uwep)
			&& u.fightingForm == FFORM_SHIEN && (!uarm || is_light_armor(uarm)
			&& rn2(3) < max((min(P_SKILL(u.fightingForm), P_SKILL(weapon_type(uwep)))) - 1, 1))) {
			You("reflect the %s with your lightsaber!", doname(thrownobj));
			if (getdir("Redirect it which way?"))
				shienuse = TRUE;
		}
		if (youdef && shienuse && (u.dx || u.dy)) {
			*pdx = u.dx;
			*pdy = u.dy;
			*prange = *prange2;
		}
		else if (youdef || mdef->mfaction != FRACTURED){
			*pdx *= -1;
			*pdy *= -1;
		}
		else {
			int i = rn2(8);
			*pdx = xdir[i];
			*pdy = ydir[i];
		}
		return MM_MISS;
	}
	/* the player has a chance to burn some projectiles (not blaster bolts or laser beams) out of the air with a lightsaber */
	else if (!(thrownobj->otyp == LASER_BEAM || thrownobj->otyp == BLASTER_BOLT || thrownobj->otyp == HEAVY_BLASTER_BOLT)
		&& youdef && uwep && is_lightsaber(uwep) && litsaber(uwep)
		&& ((u.fightingForm == FFORM_SHIEN && (!uarm || is_light_armor(uarm))) ||
		(u.fightingForm == FFORM_SORESU && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm)))
		&& 
		rn2(3) < max((min(P_SKILL(u.fightingForm), P_SKILL(weapon_type(uwep))))-1,1))
		)
	{
		You("burn %s out of the %s!", doname(thrownobj), (Underwater || Is_waterlevel(&u.uz)) ? "water" : "air");
		obfree(thrownobj, (struct obj*) 0);
		*wepgone = TRUE;
		return MM_HIT;
	}

	/* unicorns */
	if (thrownobj->oclass == GEM_CLASS && is_unicorn(pd)) {
		if (youdef)
		{
			if (thrownobj->otyp > LAST_GEM) {
				You("catch the %s.", xname(thrownobj));
				You("are not interested in %s junk.",
					(magr ? s_suffix(mon_nam(magr)) : "that"));
				makeknown(thrownobj->otyp);
				dropy(thrownobj);
			}
			else {
				You("accept %s gift in the spirit in which it was intended.",
					(magr ? s_suffix(mon_nam(magr)) : "the mysterious"));
				(void)hold_another_object(thrownobj,
					"You catch, but drop, %s.", xname(thrownobj),
					"You catch:");
			}
			*wepgone = TRUE;
			return MM_HIT;
		}
		else if(youagr)
		{
			if (vis && mdef->mtame) {
				pline("%s catches and drops %s.", Monnam(mdef), the(xname(thrownobj)));
			}
			else {
				if (thrownobj->oartifact && vis){ //All gem artifacts need to force a "the" here.
					pline("%s catches the %s.", Monnam(mdef), xname(thrownobj));
				}
				else if (vis) {
					pline("%s catches %s.", Monnam(mdef), the(xname(thrownobj)));
				}
				if (gem_accept(mdef, thrownobj)) {
					*wepgone = TRUE;
					return MM_HIT;
				}
					
			}
			/* not accepted -- drop it there */
			*prange = *prange2 = 0;
			return MM_MISS;
		}
		/* mvm unicorn gem throw-catch is not a thing. */
	}

	/* quest leader */
	if (youagr && quest_arti_hits_leader(thrownobj, mdef)) {
		/* not wakeup(), which angers non-tame monsters */
		mdef->msleeping = 0;
		mdef->mstrategy &= ~STRAT_WAITMASK;
		if (mdef->mcanmove) {
			quest_art_swap(thrownobj, mdef, wepgone);
			*wepgone = TRUE;
			return MM_HIT;
		}
		else {
			/* drop it there */
			*prange = *prange2 = 0;
			return MM_MISS;
		}
	}

	/* you throwing to a pet */
	if (youagr && mdef->mtame) {
		if (mdef->mcanmove &&
			(!is_animal(mdef->data)) &&
			(!mindless_mon(mdef) || (mdef->data == &mons[PM_CROW_WINGED_HALF_DRAGON] && thrownobj->oartifact == ART_YORSHKA_S_SPEAR)) &&
			!fired
			) {
			if (vis) {
				pline("%s catches %s.", Monnam(mdef), the(xname(thrownobj)));
			}
			obj_extract_self(thrownobj);
			(void)mpickobj(mdef, thrownobj);
			if ((attacktype(mdef->data, AT_WEAP) ||
				attacktype(mdef->data, AT_DEVA) ||
				attacktype(mdef->data, AT_XWEP)
				) &&
				mdef->weapon_check == NEED_WEAPON) {
				mdef->weapon_check = NEED_HTH_WEAPON;
				(void)mon_wield_item(mdef);
			}
			m_dowear(mdef, FALSE);
			newsym(x(mdef), y(mdef));
			*wepgone = TRUE;
			return MM_HIT;
		}
	}

	/* feeding domestic animals */
	if (befriend_with_obj(mdef->data, thrownobj) ||
		(mdef->mtame && dogfood(mdef, thrownobj) <= 2)) {	/* 2 <=> ACCFOOD */
		if (tamedog(mdef, thrownobj))
		{
			*wepgone = TRUE;
			return MM_HIT;           	/* obj is gone */
		}
		else {
			/* not tmiss(), which angers non-tame monsters */
			miss(xname(thrownobj), mdef);
			mdef->msleeping = 0;
			mdef->mstrategy &= ~STRAT_WAITMASK;
			return MM_MISS;
		}
	}

	/* Determine if the projectile hits */
	dieroll = rnd(20);
	struct attack dummy = { AT_WEAP, AD_PHYS, 0, 0 };
	accuracy = tohitval(magr, mdef, &dummy, thrownobj, launcher, (misfired ? 2 : 1), 0);

	if (accuracy > dieroll)
	{
		/* hit */
		/* (player-only) exercise dex */
		if (youagr)
			exercise(A_DEX, TRUE);
		/* call hmon to make the projectile hit */
		/* hmon will do hitmsg */
		result = hmon2point0(magr, mdef, &dummy, &dummy, thrownobj, launcher, (misfired ? 2 : 1), 0, 0, TRUE, dieroll, FALSE, vis, wepgone, FALSE);

		/* wake up defender */
		wakeup2(mdef, youagr);

		/* anger shopkeep */
		if (youagr && mdef && mdef->isshk &&
			(!inside_shop(u.ux, u.uy) ||
			!index(in_rooms(mdef->mx, mdef->my, SHOPBASE), *u.ushops)))
			hot_pursuit(mdef);

		/* deal with projectile */
		if (*wepgone) {
			/*hmon destroyed it, we're already done*/;
		}
		else 
		/* projectiles other than magic stones
		 * sometimes disappear when thrown
		 * WAC - Spoon always disappears after doing damage
		 */
		if ((objects[thrownobj->otyp].oc_skill < P_NONE &&
			objects[thrownobj->otyp].oc_skill > -P_BOOMERANG &&
			thrownobj->oclass != GEM_CLASS &&
			!objects[thrownobj->otyp].oc_magic) ||
			(thrownobj->oartifact == ART_HOUCHOU)
			) {
			/* we were breaking 2/3 of everything unconditionally.
			 * we still don't want anything to survive unconditionally,
			 * but we need ammo to stay around longer on average.
			 */
			boolean broken = FALSE;
			if (thrownobj->oartifact && thrownobj->oartifact != ART_HOUCHOU){
				broken = FALSE;
			}
			else if (forcedestroy ||
				(launcher && fired && (launcher->oartifact == ART_HELLFIRE || launcher->oartifact == ART_BOW_OF_SKADI)) ||
				(thrownobj->oartifact == ART_HOUCHOU) ||
				(fired && thrownobj->otyp == BULLET) || 
				(fired && thrownobj->otyp == SILVER_BULLET) || 
				(fired && thrownobj->otyp == SHOTGUN_SHELL) || 
				(fired && thrownobj->otyp == ROCKET) || 
				(fired && thrownobj->otyp == BLASTER_BOLT) ||
				(fired && thrownobj->otyp == HEAVY_BLASTER_BOLT) ||
				(fired && thrownobj->otyp == FRAG_GRENADE) || 
				(fired && thrownobj->otyp == GAS_GRENADE)) {
				broken = TRUE;
			}
			else {
				int break_chance = 3 + greatest_erosion(thrownobj) - thrownobj->spe;
				if (break_chance > 1)
					broken = rn2(break_chance);
				else
					broken = !rn2(4);
				if (thrownobj->blessed && rnl(100) < 25)
					broken = FALSE;
			}
			/* some projectiles should instead be transfered to the defender's inventory... if they lived */
			if (*hp(mdef)>0) {
				if (youdef && thrownobj->otyp == LOADSTONE && !rn2(3))
				{
					broken = FALSE;
					*wepgone = TRUE;
					pickup_object(thrownobj, 1, TRUE);
					result |= MM_HIT;
				}
			}

			if (broken) {
				if (*u.ushops) {
					check_shop_obj(thrownobj, bhitpos.x, bhitpos.y, TRUE);
				}
				//#ifdef FIREARMS
				destroy_projectile(magr, thrownobj);
				*wepgone = TRUE;

				/* check if explosions changed result */
				if (*hp(mdef) <= 0)
					result |= MM_DEF_DIED;
			}
		}
		else
		{
			/* possibly damage the projectile -- at this point it is surviving */
			passive_obj2(magr, mdef, thrownobj, (struct attack *)0, (struct attack *)0);
		}

		return result;
	}
	else
	{
		/* miss */
		/* print missmsg */
		if (vis) {
			/* If the target can't be seen or doesn't look like a valid target,
			 * avoid "the arrow misses it," or worse, "the arrows misses the mimic."
			 * An attentive player will still notice that this is different from
			 * an arrow just landing short of any target (no message in that case),
			 * so will realize that there is a valid target here anyway. 
			 */
			if (!canseemon(mdef) || (mdef->m_ap_type && mdef->m_ap_type != M_AP_MONSTER)) {
				pline("%s %s.", (The(mshot_xname(thrownobj))), (vtense(mshot_xname(thrownobj), "miss")));
			}
			else {
				pline("%s %s %s.",
					(The(mshot_xname(thrownobj))),
					(vtense(mshot_xname(thrownobj), "miss")),
					(((cansee(bhitpos.x, bhitpos.y) || canspotmon(mdef)) && flags.verbose) ? mon_nam(mdef) : "it")
					);
			}
		}
		/* possibly wake defender */
		if (!rn2(3))
		{
			wakeup2(mdef, youagr);
		}
		/* anger shopkeep */
		if (youagr && mdef && mdef->isshk &&
			(!inside_shop(u.ux, u.uy) ||
			!index(in_rooms(mdef->mx, mdef->my, SHOPBASE), *u.ushops)))
			hot_pursuit(mdef);
			
		/* cause projectile to fall onto the floor -- not for blaster bolts */
		if (thrownobj->otyp != BLASTER_BOLT &&
			thrownobj->otyp != HEAVY_BLASTER_BOLT &&
			thrownobj->otyp != LASER_BEAM)
			*prange = *prange2 = 0;
		return MM_MISS;
	}

	return MM_MISS;
}

void
quest_art_swap(obj, mon, swapped)
struct obj * obj;
struct monst * mon;
boolean * swapped;
{
	/* don't make game unwinnable if naive player throws artifact
	at leader.... */
	pline("%s catches %s.", Monnam(mon), the(xname(obj)));
	if (mon->mpeaceful) {
		if (Race_if(PM_ELF) && mon->data == &mons[PM_CELEBORN]){
			boolean next2u = monnear(mon, u.ux, u.uy);
			if (obj->oartifact == ART_PALANTIR_OF_WESTERNESSE &&
				yn("If you prefer, we can use the Palantir to secure the city, and you can use my bow in your travels.") == 'y'
				){
				*swapped = TRUE;
				obfree(obj, (struct obj *)0);
				obj = mksobj(ELVEN_BOW, TRUE, FALSE);
				obj = oname(obj, artiname(ART_BELTHRONDING));
				obj->oerodeproof = TRUE;
				obj->blessed = TRUE;
				obj->cursed = FALSE;
				obj->spe = rn2(8);
				pline("%s %s %s to you.", Monnam(mon),
					(next2u ? "hands" : "tosses"), the(xname(obj)));
				if (!next2u) sho_obj_return(obj, u.ux, u.uy);
				obj = addinv(obj);	/* into your inventory */
				(void)encumber_msg();
			}
			else {
				pline("%s %s %s back to you.", Monnam(mon),
					(next2u ? "hands" : "tosses"), the(xname(obj)));
				if (!next2u) sho_obj_return(obj, u.ux, u.uy);
				obj = addinv(obj);	/* back into your inventory */
				(void)encumber_msg();
			}
		}
		else if (Race_if(PM_DROW)){
			boolean next2u = monnear(mon, u.ux, u.uy);
			if (quest_status.got_thanks) finish_quest(obj);
			if (obj->oartifact == ART_SILVER_STARLIGHT && quest_status.got_thanks && mon->data == &mons[PM_ECLAVDRA] &&
				yn("Do you wish to take the Wrathful Spider, instead of this?") == 'y'
				){
				*swapped = TRUE;
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
				if (!next2u) sho_obj_return(obj, u.ux, u.uy);
				obj = addinv(obj);	/* into your inventory */
				(void)encumber_msg();
			}
			else if (obj->oartifact == ART_TENTACLE_ROD && quest_status.got_thanks && mon->data == &mons[PM_SEYLL_AUZKOVYN] &&
				yn("Do you wish to take the Crescent Blade, instead of this?") == 'y'
				){
				*swapped = TRUE;
				obfree(obj, (struct obj *)0);
				obj = mksobj(SABER, TRUE, FALSE);
				obj = oname(obj, artiname(ART_CRESCENT_BLADE));
				obj->oerodeproof = TRUE;
				obj->blessed = TRUE;
				obj->cursed = FALSE;
				obj->spe = rnd(7);
				finish_quest(obj);
				pline("%s %s %s to you.", Monnam(mon),
					(next2u ? "hands" : "tosses"), the(xname(obj)));
				if (!next2u) sho_obj_return(obj, u.ux, u.uy);
				obj = addinv(obj);	/* into your inventory */
				(void)encumber_msg();
			}
			else if (obj->oartifact == ART_DARKWEAVER_S_CLOAK && quest_status.got_thanks && mon->data == &mons[PM_ECLAVDRA] &&
				yn("Do you wish to take Spidersilk, instead of this?") == 'y'
				){
				*swapped = TRUE;
				obfree(obj, (struct obj *)0);
				obj = mksobj(DROVEN_CHAIN_MAIL, TRUE, FALSE);
				obj = oname(obj, artiname(ART_SPIDERSILK));
				obj->oerodeproof = TRUE;
				obj->blessed = TRUE;
				obj->cursed = FALSE;
				obj->spe = rn2(8);
				finish_quest(obj);
				pline("%s %s %s to you.", Monnam(mon),
					(next2u ? "hands" : "tosses"), the(xname(obj)));
				if (!next2u) sho_obj_return(obj, u.ux, u.uy);
				obj = addinv(obj);	/* into your inventory */
				(void)encumber_msg();
			}
			else if (obj->oartifact == ART_TENTACLE_ROD && quest_status.got_thanks && mon->data == &mons[PM_DARUTH_XAXOX] &&
				yn("Do you wish to take the Webweaver's Crook, instead of this?") == 'y'
				){
				*swapped = TRUE;
				obfree(obj, (struct obj *)0);
				obj = mksobj(FAUCHARD, TRUE, FALSE);
				obj = oname(obj, artiname(ART_WEBWEAVER_S_CROOK));
				obj->oerodeproof = TRUE;
				obj->blessed = TRUE;
				obj->cursed = FALSE;
				obj->spe = rnd(3) + rnd(4);
				finish_quest(obj);
				pline("%s %s %s to you.", Monnam(mon),
					(next2u ? "hands" : "tosses"), the(xname(obj)));
				if (!next2u) sho_obj_return(obj, u.ux, u.uy);
				obj = addinv(obj);	/* into your inventory */
				(void)encumber_msg();
			}
			else {
				if (!quest_status.got_thanks) finish_quest(obj);	/* acknowledge quest completion */
				pline("%s %s %s back to you.", Monnam(mon),
					(next2u ? "hands" : "tosses"), the(xname(obj)));
				if (!next2u) sho_obj_return(obj, u.ux, u.uy);
				obj = addinv(obj);	/* back into your inventory */
				(void)encumber_msg();
			}
		}
		else {
			boolean next2u = monnear(mon, u.ux, u.uy);

			finish_quest(obj);	/* acknowledge quest completion */
			pline("%s %s %s back to you.", Monnam(mon),
				(next2u ? "hands" : "tosses"), the(xname(obj)));
			if (!next2u) sho_obj_return(obj, u.ux, u.uy);
			obj = addinv(obj);	/* back into your inventory */
			(void)encumber_msg();
		}
	}
	else {
		/* angry leader caught it and isn't returning it */
		(void)mpickobj(mon, obj);
	}
	return;
}

/* show the currently thrown object returning to (destx, desty) in a straight line (not for boomerangs) */
void
sho_obj_return(thrownobj, destx, desty)
struct obj * thrownobj;
int destx;
int desty;
{
	int dx = destx - bhitpos.x;
	int dy = desty - bhitpos.y;

	/* check that it is okay to reduce dx and dy to small increments */
	if (dx && dy && (abs(dx) != abs(dy))) {
		return;
	}

	/* preference: don't show straight-line return for boomerangs */
	if (is_boomerang(thrownobj)) {
		return;
	}

	/* change dx and dy to be small increments */
	dx = sgn(dx);
	dy = sgn(dy);

	/* might already be our location (bounced off a wall) */
	if (bhitpos.x != destx || bhitpos.y != desty) {
		int x = bhitpos.x;
		int y = bhitpos.y;

		/* show its return */
		tmp_at(DISP_FLASH, obj_to_glyph(thrownobj));

		while (x != destx || y != desty) {
			tmp_at(x, y);
			if (cansee(x, y)) delay_output();
			x += dx;
			y += dy;
		}
		tmp_at(DISP_END, 0);
	}
	return;
}

/* 
 * return_thrownobj()
 * 
 * returns thrownobj to magr
 *
 * Relies on global variable "old_wep_mask"
 */
void
return_thrownobj(magr, thrownobj)
struct monst * magr;
struct obj * thrownobj;
{
	boolean youagr = (magr == &youmonst);
	struct obj * obj;

	/* check that thrownobj isn't already back in magr's possession */
	if (thrownobj->where != OBJ_FREE) {
		if (youagr && thrownobj->where == OBJ_INVENT)
			return;
		else if (!youagr && thrownobj->where == OBJ_MINVENT && thrownobj->ocarry == magr)
			return;
		else {
			impossible("thrownobj not free when returning?");
			return;
		}
	}

	if (youagr) {
		obj = addinv(thrownobj);
		(void)encumber_msg();
	}
	else {
		mpickobj(magr, thrownobj);
		obj = thrownobj;
	}
	if (obj->owornmask != old_wep_mask) {
		if (youagr) {
			rewield(obj, old_wep_mask);
			u.twoweap = u_was_twoweap;
		}
		else
			setmwielded(magr, obj, old_wep_mask);
	}
	return;
}

/*
 * toss_up()
 * 
 * Hero tosses an object upwards with appropriate consequences.
 *
 * Returns FALSE if the object is gone.
 *
 * SCOPECREEP: use hmon2point0 and clean up the old crap
 */
STATIC_OVL boolean
toss_up2(obj)
struct obj *obj;
{
	char buf[BUFSZ];
	boolean wepgone = FALSE;
	/* note: obj->quan == 1 */

	if (In_outdoors(&u.uz)) {
		Sprintf(buf, "flies upwards");
	}
	else {
		if (rn2(5)) {
			if (breaktest(obj)) {
				pline("%s hits the %s.", Doname2(obj), ceiling(u.ux, u.uy));
				breakmsg(obj, !Blind);
				breakobj(obj, u.ux, u.uy, TRUE, TRUE);
				return FALSE;
			}
			Sprintf(buf, "hits the %s", ceiling(u.ux, u.uy));
		}
		else {
			Sprintf(buf, "almost hits the %s", ceiling(u.ux, u.uy));
		}
	}
	if (!hits_insubstantial((struct monst *)0, &youmonst, (struct attack *)0, obj)) {
		pline("%s %s, then falls back down through you.", Doname2(obj), buf);
		hitfloor2(&youmonst, obj, (struct obj *)0, FALSE, FALSE, &wepgone);
	}
	else {
		pline("%s %s, then falls back on top of your %s.",
			Doname2(obj), buf, body_part(HEAD));

		/* object now hits you */

		if (obj->oclass == POTION_CLASS) {
			potionhit(&youmonst, obj, TRUE);
			return FALSE;
		}
		else if (breaktest(obj)) {
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
					!(poly_when_stoned(youracedata) && polymon(PM_STONE_GOLEM)))
					goto petrify;
			case CREAM_PIE:
			case BLINDING_VENOM:
				pline("You've got it all over your %s!", body_part(FACE));
				if (blindinc) {
					if (otyp == BLINDING_VENOM && !Blind)
						pline("It blinds you!");
					u.ucreamed += blindinc;
					make_blinded(Blinded + (long)blindinc, FALSE);
					if (!Blind) Your1(vision_clears);
				}
				break;
			default:
				break;
			}
			return FALSE;
		}
		else {		/* neither potion nor other breaking object */
			boolean less_damage = uarmh && is_hard(uarmh), artimsg = FALSE;
			int dmg = dmgval(obj, &youmonst, 0);
			int basedamage = dmg;
			int newdamage = dmg;
			int dieroll = rn1(18, 2);  /* need a fake die roll here; rn1(18,2) avoids 1 and 20 */

			if (obj->oartifact){
				artimsg = artifact_hit((struct monst *)0, &youmonst, obj, &newdamage, dieroll);
				dmg += (newdamage - basedamage);
				newdamage = basedamage;
			}
			if (obj->oproperties){
				artimsg |= oproperty_hit((struct monst *)0, &youmonst, obj, &newdamage, dieroll);
				dmg += (newdamage - basedamage);
				newdamage = basedamage;
			}
			if (spec_prop_otyp(obj)){
				artimsg |= otyp_hit((struct monst *)0, &youmonst, obj, &newdamage, dieroll);
				dmg += (newdamage - basedamage);
				newdamage = basedamage;
			}


			if (!dmg) {	/* probably wasn't a weapon; base damage on weight */
				dmg = (int)obj->owt / 100;
				if (dmg < 1) dmg = 1;
				else if (dmg > 6) dmg = 6;
				if (hits_insubstantial((struct monst *)0, &youmonst, (struct attack *)0, obj) != 2)	/* 2: full damage object (like sunsword) */
					dmg = 0;
			}
			if (resist_attacks(youracedata))
				dmg = 0;
			if (dmg > 1 && less_damage) dmg = 1;
			if (dmg > 0) dmg += aeshbon();
			if (dmg > 0) dmg += u.udaminc;
			if (dmg < 0) dmg = 0;	/* beware negative rings of increase damage */
			if (Half_physical_damage) dmg = (dmg + 1) / 2;
			if (u.uvaul_duration) dmg = (dmg + 1) / 2;

			if (uarmh) {
				if (less_damage && dmg < (Upolyd ? u.mh : u.uhp)) {
					if (!artimsg && (is_hard(uarmh)))
						pline("Fortunately, you are wearing a hard helmet.");
				}
				else if (flags.verbose &&
					!(obj->otyp == CORPSE && touch_petrifies(&mons[obj->corpsenm])))
					Your("%s does not protect you.", xname(uarmh));
			}
			else if (obj->otyp == CORPSE && touch_petrifies(&mons[obj->corpsenm])) {
				if (!Stone_resistance &&
					!(poly_when_stoned(youracedata) && polymon(PM_STONE_GOLEM))) {
				petrify:
					killer_format = KILLED_BY;
					killer = "elementary physics";	/* "what goes up..." */
					You("turn to stone.");
					if (obj) dropy(obj);	/* bypass most of hitfloor() */
					done(STONING);
					return obj ? TRUE : FALSE;
				}
			}
			hitfloor2(&youmonst, obj, (struct obj *)0, FALSE, FALSE, &wepgone);
			losehp(dmg, "falling object", KILLED_BY_AN);
		}
	}
	return TRUE;
}

/*
 * calc_multishot()
 *
 * Calculates the multishot to use
 *
 * Assumes launcher is an appropriate launcher of ammo
 */
int
calc_multishot(magr, ammo, launcher, shotlimit)
struct monst * magr;
struct obj * ammo;
struct obj * launcher;
int shotlimit;
{
	boolean youagr = (magr == &youmonst);
	int multishot = 1;
	int skill = objects[ammo->otyp].oc_skill;

	if (youagr ? (Confusion || Stunned) : (magr->mconf))
	{
		/* no skill bonuses to multishot when confused */
		multishot = 1;
	}
	else if (
		(ammo_and_launcher(ammo, launcher) && skill != -P_CROSSBOW) ||
		(skill == P_DAGGER) ||
		(skill == -P_DART) ||
		(skill == -P_SHURIKEN) ||
		(skill == -P_BOOMERANG) ||
		(ammo->oartifact == ART_SICKLE_MOON) || 
		(ammo->oartifact == ART_AMHIMITL)
		) {
		/* Skill based bonus */
		int magr_wepskill;
		if (youagr)
			magr_wepskill = P_SKILL(weapon_type(ammo));
		else
			magr_wepskill = is_prince(magr->data) ? P_EXPERT : is_lord(magr->data) ? P_SKILLED : P_BASIC;

		switch (magr_wepskill) {
		default:
			multishot = 1; break;
		case P_SKILLED:
			multishot = 2; break;
		case P_EXPERT:
			multishot = 3; break;
		}

		/* The legendary Longbow increases skilled RoF */
		if (launcher && (
			launcher->oartifact == ART_LONGBOW_OF_DIANA ||
			launcher->oartifact == ART_BELTHRONDING))
			multishot++;

		/* Heavy machine gun gets a large skill-based RoF bonus */
		if (launcher && launcher->otyp == HEAVY_MACHINE_GUN)
			multishot *= 2;

		/* Role-based RoF bonus */
		switch (youagr ? Role_switch : monsndx(magr->data)) {
		case PM_CAVEMAN:
			if (skill == -P_SLING) multishot++;
			break;
		case PM_RANGER:
			multishot++;
			if (launcher && launcher->oartifact == ART_LONGBOW_OF_DIANA) multishot++;//double bonus for Rangers
			break;
		case PM_ROGUE:
			if (skill == P_DAGGER) multishot++;
			break;
		case PM_NINJA:
		case PM_SAMURAI:
			if (ammo->otyp == YA && launcher && launcher->otyp == YUMI) multishot++;
			break;
		case PM_WIZARD:
			if (skill == P_DAGGER) multishot -= 3;	/* very bad at throwing daggers */
		default:
			break;	/* No bonus */
		}
		/* Race-based RoF bonus */
		if ((youagr ? Race_if(PM_ELF) : is_elf(magr->data)) && (
			(ammo->otyp == ELVEN_ARROW && launcher && launcher->otyp == ELVEN_BOW) ||
			(ammo->oartifact == ART_SICKLE_MOON) ||
			(launcher && launcher->oartifact == ART_BELTHRONDING) //double bonus for Elves
			))
			multishot++;
		if ((youagr ? (Race_if(PM_DROW) || Race_if(PM_MYRKALFR)) : is_drow(magr->data)) && (
			(ammo->oartifact == ART_SICKLE_MOON)
			))
			multishot++;
		if ((youagr ? Race_if(PM_ORC) : is_orc(magr->data)) && (
			(ammo->otyp == ORCISH_ARROW && launcher && launcher->otyp == ORCISH_BOW)
			))
			multishot++;
	}

	/* Okay. */
	if (youagr && barage) {
		/* Spirit power barage maximized multishot */
		multishot += u.ulevel / 10 + 1;
	}
	else {
		/* normal behaviour: randomize multishot */
		multishot = rnd(multishot);
	}

	/* The Sansara Mirror doubles your multishooting ability,
	 * as does the dual Pen of the Void (while Eve is bound) */
	if (youagr && (
		(uwep && uwep->oartifact == ART_SANSARA_MIRROR) ||
		(uswapwep && uswapwep->oartifact == ART_SANSARA_MIRROR) ||
		(launcher && launcher->oartifact == ART_PEN_OF_THE_VOID && (mvitals[PM_ACERERAK].died > 0))
		)) multishot *= 2;

	/* Crossbow artifact bonuses that are inconsistently applied after most other adjustments. Grr. */
	if (launcher && launcher->oartifact){
		if (launcher->oartifact == ART_WRATHFUL_SPIDER) multishot += rn2(8);
		else if (launcher->oartifact == ART_ROGUE_GEAR_SPIRITS) multishot = 2;
	}

	/* limit multishot by shotlimit */
	if (shotlimit > 0 && multishot > shotlimit)
		multishot = shotlimit;

	/* Firearms have a built-in rate of fire.
	 * Rate of fire is intrinsic to the weapon - cannot be user selected except via altmode
	 * (currently oc_rof conflicts with wsdam)
	 */
	if (launcher && is_firearm(launcher)) {
		if (launcher->otyp == BFG){
			if (objects[(ammo)->otyp].w_ammotyp == WP_BULLET)       multishot += 2 * (objects[(launcher->otyp)].oc_rof);
			else if (objects[(ammo)->otyp].w_ammotyp == WP_SHELL)   multishot += 1.5*(objects[(launcher->otyp)].oc_rof);
			else if (objects[(ammo)->otyp].w_ammotyp == WP_GRENADE) multishot += 1 * (objects[(launcher->otyp)].oc_rof);
			else if (objects[(ammo)->otyp].w_ammotyp == WP_ROCKET)  multishot += 0.5*(objects[(launcher->otyp)].oc_rof);
			else multishot += (objects[(launcher->otyp)].oc_rof);
		}
		else if (objects[(launcher->otyp)].oc_rof)
			multishot += (objects[(launcher->otyp)].oc_rof - 1);
		if (launcher->altmode == WP_MODE_SINGLE)
			/* weapons switchable b/w full/semi auto */
			multishot = 1;
		else if (launcher->altmode == WP_MODE_BURST)
			multishot = ((multishot > 5) ? (multishot / 3) : 1);
		/* else it is auto == no change */
	}

	/* For most things, limit multishot to ammo supply */
	if ((long)multishot > ammo->quan && !(
		ammo->oartifact == ART_WINDRIDER ||
		ammo->oartifact == ART_SICKLE_MOON ||
		ammo->oartifact == ART_ANNULUS ||
		ammo->oartifact == ART_AMHIMITL ||
		ammo->oartifact == ART_DART_OF_THE_ASSASSIN ||
		(launcher && is_blaster(launcher))
		))
		multishot = (int)ammo->quan;

	/* Blasters limit multishot to charge */
	if (launcher && is_blaster(launcher) &&
		multishot > launcher->ovar1)
		multishot = launcher->ovar1;

	/* minimum multishot of 1*/
	if (multishot < 1)
		multishot = 1;

	return multishot;
}

/*
 * calc_range()
 *
 * Calculates the maximum range of the projectile
 *
 * Assumes launcher is an appropriate launcher of ammo
 *
 * Is significantly stripped-down for non-player magr.
 */
int
calc_range(magr, ammo, launcher, hurtle_dist)
struct monst * magr;
struct obj * ammo;
struct obj * launcher;
int * hurtle_dist;
{
	boolean youagr = (magr && magr == &youmonst);
	int range = 1;
	int urange;
	
	/* The player's range is based on strength */
	/* Even for crossbows, for some reason. */
	if (youagr) {
		urange = (int)(ACURRSTR) / 2;
		int heaviness = (weight(ammo) / ammo->quan);	/* we want the weight of 1 projectile */
		/* balls are easy to throw or at least roll */
		/* also, this ensures the maximum range of a ball is greater
		* than 1, so the effects from throwing attached balls are
		* actually possible
		*/
		if (ammo->otyp == HEAVY_IRON_BALL)
			range = urange - (heaviness / 100);
		else
			range = urange - (heaviness / 40);
		if (ammo == uball) {
			if (u.ustuck)
				range = 1;
			else if (range >= 5)
				range = 5;
		}
		if (range < 1)
			range = 1;
	}
	else {
		/* monsters have a set base range and don't care about the weight of their ammo */
		urange = range = BOLT_LIM;
	}

	/* launched projectiles get increased range */
	if (launcher) {
		/* some things maximize range */
		if ((launcher->oartifact == ART_LONGBOW_OF_DIANA) ||
			(launcher->oartifact == ART_XIUHCOATL) ||
			(launcher->oartifact == ART_PEN_OF_THE_VOID && launcher->ovar1&SEAL_EVE && mvitals[PM_ACERERAK].died > 0)
			) {
			range = 1000;
		}
		else if (objects[(launcher->otyp)].oc_range) {
			/* some launchers specify range (firearms specifically) */
			range = objects[(launcher->otyp)].oc_range;
		}
		else {
			/* other launchers give a small range boost */
			range += 1;
		}
	}
	else if (is_ammo(ammo) && ammo->oclass != GEM_CLASS && !is_grenade(ammo)) {
		/* non-rock non-spear non-grenade ammo is poorly thrown */
		range /= 2;
	}

	/* Monsters don't hurtle */
	if (youagr && (Weightless || Levitation)) {
		/* action, reaction... */
		urange -= range;
		if (urange < 1) urange = 1;
		range -= urange;
		if (range < 1) range = 1;

		*hurtle_dist = urange;
	}
	else {
		*hurtle_dist = 0;
	}

	if (is_boulder(ammo))
		range = 20;		/* you must be giant */
	else if (is_boomerang(ammo))
		range = 10;		/* boomerangs must have range 10 to complete their arc */
	else if (ammo->oartifact == ART_MJOLLNIR) //But the axe of D Lords can be thrown
		range = (range + 1) / 2;	/* it's heavy */
	else if (ammo == uball && u.utrap && u.utraptype == TT_INFLOOR)
		range = 1;

	if (Underwater) {
		if (range > 1)
			range = 1;
		if (*hurtle_dist > 1)
			*hurtle_dist = 1;
	}

	return range;
}

/*
 * dothrow()
 *
 * Player's ('t') command
 * 
 * Asks for something to throw.
 */
int
dothrow()
{
	struct obj * ammo;
	struct obj * launcher;
	int oldmulti = multi, result, shotlimit;
	char *oldsave_cm = save_cm;

	if (notake(youracedata)) {
		You("are physically incapable of throwing anything.");
		return 0;
	}
	if (check_capacity((char *)0)) {
		return 0;
	}

	/* get object to throw */
	static NEARDATA const char toss_objs[] =
	{ ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, WEAPON_CLASS, 0 };
	/* different default choices when wielding a sling (gold must be included) */
	static NEARDATA const char bullets[] =
	{ ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, GEM_CLASS, 0 };

	ammo = getobj(uslinging() ? bullets : toss_objs, "throw");

	if (!ammo)
		return(0);

	/* kludge to work around parse()'s pre-decrement of 'multi' */
	shotlimit = (multi || save_cm) ? multi + 1 : 0;

	/* try to find a wielded launcher */
	if (ammo != uwep && ammo != uswapwep &&
		ammo_and_launcher(ammo, uwep))
		launcher = uwep;
	else if (ammo != uwep && ammo != uswapwep &&
		ammo_and_launcher(ammo, uswapwep))
		launcher = uswapwep;
	else
		launcher = (struct obj *)0;

	/* go to the next step */
	result = uthrow(ammo, launcher, shotlimit, FALSE);

	save_cm = oldsave_cm;
	return (result);
}


/*
 * dofire()
 *
 * Player's ('f') command
 */
int
dofire()
{
	int oldmulti = multi;
	int result = 0;
	int shotlimit = 0;
	char *oldsave_cm = save_cm;

	if (notake(youracedata)) {	/* this will need adjusting when 'f' automatically does AT_SPIT etc */
		You("are physically incapable of doing that.");
		return 0;
	}
	if (check_capacity((char *)0))
		return 0;

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
	shotlimit = (multi || save_cm) ? multi + 1 : 0;
	multi = 0;		/* reset; it's been used up */


	/* Fire loaded launchers and blasters */
	if ((uwep     && ((uquiver && ammo_and_launcher(uquiver, uwep    )) || is_blaster(uwep    ))) ||
		(uswapwep && ((uquiver && ammo_and_launcher(uquiver, uswapwep)) || is_blaster(uswapwep)))
		)
	{
		struct obj * launcher;
		int hand;

		/* verify twoweapon status */
		if (u.twoweap && !test_twoweapon())
			untwoweapon();

		/* do mainhand, then offhand (if twoweaponing) */
		for (hand = 0; hand < (u.twoweap ? 2 : 1); hand++) {
			launcher = (!hand ? uwep : uswapwep);
			if (!launcher || !((uquiver && ammo_and_launcher(uquiver, launcher)) || is_blaster(launcher)))
				continue;

			if (uquiver && ammo_and_launcher(uquiver, launcher)) {
				/* simply fire uquiver from the launcher */
				result += uthrow(uquiver, launcher, shotlimit, FALSE);
			}
			else if (is_blaster(launcher)) {
				/* blasters need to generate their ammo on the fly */
				struct obj * ammo = (struct obj *)0;

				/* do we have enough charge to fire? */
				if (!launcher->ovar1) {
					if (launcher->otyp == RAYGUN) You("push the firing stud, but nothing happens.");
					else pline("Nothing happens when you pull the trigger.");
					/* nothing else happens */
				}
				else {
					switch (launcher->otyp) {
					case CUTTING_LASER:
					case HAND_BLASTER:
					case ARM_BLASTER:
					case MASS_SHADOW_PISTOL:
						ammo = blaster_ammo(launcher);
						break;
					case RAYGUN:
						/* create fake ammo in order to calculate multishot correctly */
						ammo = blaster_ammo(launcher);
						if (getdir((char *)0))
							result += zap_raygun(launcher, calc_multishot(&youmonst, ammo, launcher, shotlimit), shotlimit);
						/* destroy ammo and don't go through uthrow */
						obfree(ammo, 0);
						ammo = (struct obj *)0;
						break;
					default:
						impossible("Unhandled blaster %d!", launcher->otyp);
						break;
					}
					/* always destroy ammo fired from a blaster */
					if (ammo) {
						result += uthrow(ammo, launcher, shotlimit, TRUE);
						/* and now delete the ammo object we created */
						obfree(ammo, 0);
					}
				}
			}
		}
		return (result > 0);
	}

	/* Throw quivered throwing weapons */
	if (uquiver && throwing_weapon(uquiver)) {
		return uthrow(uquiver, (struct obj *)0, shotlimit, FALSE);
	}

	/* Throw wielded weapon -- mainhand only */
	if ((uwep && (!uquiver || (is_ammo(uquiver) && !ammo_and_launcher(uquiver, uwep)))) && (
		(uwep->oartifact == ART_KHAKKHARA_OF_THE_MONKEY) ||
		(uwep->oartifact == ART_MJOLLNIR && Role_if(PM_VALKYRIE) && ACURR(A_STR) == STR19(25)) ||
		(uwep->oartifact == ART_ANNULUS && (uwep->otyp == CHAKRAM || uwep->otyp == LIGHTSABER)) ||
		(uwep->oartifact == ART_AXE_OF_THE_DWARVISH_LORDS && Race_if(PM_DWARF) && ACURR(A_STR) == STR19(25)) ||
		(!is_blaster(uwep) && Race_if(PM_ANDROID))
		// (uwep->oartifact == ART_SICKLE_MOON)
		)) {
		return uthrow(uwep, (struct obj *)0, shotlimit, FALSE);
	}

	/* Holy Moon Lightlight Sword's magic blast -- mainhand only */
	if (uwep && uwep->oartifact == ART_HOLY_MOONLIGHT_SWORD && uwep->lamplit && u.uen >= 25){
		int dmg = d(2, 12) + 2 * uwep->spe;
		int range = (Double_spell_size) ? 6 : 3;
		xchar lsx, lsy, sx, sy;
		struct monst *mon;
		sx = u.ux;
		sy = u.uy;
		if (!getdir((char *)0) || !(u.dx || u.dy)) return 0;
		u.uen -= 25;
		/* also swing in the direction chosen */
		flags.forcefight = 1;
		domove();
		flags.forcefight = 0;

		if (u.uswallow){
			explode(u.ux, u.uy, AD_MAGM, WAND_CLASS, (d(2, 12) + 2 * uwep->spe) * ((Double_spell_size) ? 3 : 2) / 2, EXPL_CYAN, 1 + !!Double_spell_size);
			return 1;
		}
		else {
			while (--range >= 0){
				lsx = sx; sx += u.dx;
				lsy = sy; sy += u.dy;
				if (isok(sx, sy) && isok(lsx, lsy) && !IS_STWALL(levl[sx][sy].typ)) {
					mon = m_at(sx, sy);
					if (mon){
						explode(sx, sy, AD_MAGM, WAND_CLASS, dmg * ((Double_spell_size) ? 3 : 2) / 2, EXPL_CYAN, 1 + !!Double_spell_size);
						break;//break loop
					}
					else {
						tmp_at(DISP_BEAM, cmap_to_glyph(S_digbeam));
						tmp_at(sx, sy);
						if (cansee(sx, sy)) delay_output();
						tmp_at(DISP_END, 0);
					}
				}
				else {
					explode(lsx, lsy, AD_MAGM, WAND_CLASS, dmg * ((Double_spell_size) ? 3 : 2) / 2, EXPL_CYAN, 1 + !!Double_spell_size);
					break;//break loop
				}
			}
			return 1;
		}
	}

	/* Rogue Gear Spirits' auto-generated ammo -- mainhand only */
	if (uwep && (!uquiver || (is_ammo(uquiver) && !ammo_and_launcher(uquiver, uwep))) && uwep->oartifact == ART_ROGUE_GEAR_SPIRITS){
		struct obj *bolt = mksobj(CROSSBOW_BOLT, FALSE, FALSE);
		bolt->spe = min(0, uwep->spe);
		bolt->blessed = uwep->blessed;
		bolt->cursed = uwep->cursed;
		bolt->objsize = MZ_SMALL;
		bolt->quan = 3;		/* Make more than enough so that we are always able to manually destroy the excess */
		fix_object(bolt);
		result = uthrow(bolt, uwep, shotlimit, TRUE);
		obfree(bolt, 0);
		return result;
	}

	/* TODO: intrinsic monster firing (like a manticore's spikes) */

	/* Throw any old garbage we have quivered */
	if (uquiver) {
		return uthrow(uquiver, (struct obj *)0, shotlimit, FALSE);
	}
	else {
		/* We don't have anything that should be done automatically at this point. */
		if (!flags.autoquiver) {
			/* Don't automatically fill the quiver */
			You("have no ammunition readied!");
			if (iflags.quiver_fired)
				dowieldquiver(); /* quiver_fired */
			if (!uquiver)
				return(dothrow());
		}
		else {
			autoquiver();
			if (!uquiver) {
				You("have nothing appropriate for your quiver!");
				return(dothrow());
			}
			else {
				You("fill your quiver:");
				prinv((char *)0, uquiver, 0L);
			}
		}
	}

	/* Fire now-loaded launchers. We can ignore blasters, because that would have been caught above and didn't care about uquiver */
	if ((uwep     && (uquiver && ammo_and_launcher(uquiver, uwep    ))) ||
		(uswapwep && (uquiver && ammo_and_launcher(uquiver, uswapwep)))
		)
	{
		struct obj * launcher;
		int hand;
		int result = 0;

		/* verify twoweapon status */
		if (u.twoweap && !test_twoweapon())
			untwoweapon();

		/* do mainhand, then offhand (if twoweaponing) */
		for (hand = 0; hand < (u.twoweap ? 2 : 1); hand++) {
			launcher = (!hand ? uwep : uswapwep);
			if (!launcher || !(uquiver && ammo_and_launcher(uquiver, launcher)))
				continue;

			if (uquiver && ammo_and_launcher(uquiver, launcher)) {
				/* simply fire uquiver from the launcher */
				result += uthrow(uquiver, launcher, shotlimit, FALSE);
			}
		}
		return result;
	}

	/* Throw whaterver it was we quivered */
	if (uquiver) {
		return uthrow(uquiver, (struct obj *)0, shotlimit, FALSE);
	}

	/* Fall through: we did nothing */
	return 0;
}

/* 
 * blaster_ammo()
 * 
 * Creates appropriate ammo for blaster
 */
struct obj *
blaster_ammo(blaster)
struct obj * blaster;
{
	struct obj * ammo = (struct obj *)0;

	switch (blaster->otyp) {
	case CUTTING_LASER:
		ammo = mksobj(LASER_BEAM, FALSE, FALSE);
		break;
	case HAND_BLASTER:
		ammo = mksobj(BLASTER_BOLT, FALSE, FALSE);
		break;
	case ARM_BLASTER:
		ammo = mksobj(HEAVY_BLASTER_BOLT, FALSE, FALSE);
		break;
	case MASS_SHADOW_PISTOL:
		/* note: does not copy cobj->oartifact */
		if (blaster->cobj)
			ammo = mksobj(blaster->cobj->otyp, FALSE, FALSE);
		else
			ammo = mksobj(ROCK, FALSE, FALSE);
		break;
	case RAYGUN:
		/* create fake ammo in order to calculate multishot correctly */
		ammo = mksobj(LASER_BEAM, FALSE, FALSE);
		break;
	default:
		impossible("Unhandled blaster %d!", blaster->otyp);
		break;
	}

	return ammo;
}


/* 
 * uthrow()
 * 
 * Get firing direction from player, calculate multishot,
 * call projectile().
 */
boolean
uthrow(ammo, launcher, shotlimit, forcedestroy)
struct obj * ammo;
struct obj * launcher;
int shotlimit;
boolean forcedestroy;
{
	int multishot;
	int hurtle_dist = 0;
	/* ask "in what direction?" */
	/* needs different code depending on if GOLDOBJ is enabled */

	if (!getdir((char *)0)) {
#ifndef GOLDOBJ
		if (ammo->oclass == COIN_CLASS) {
			u.ugold += ammo->quan;
			flags.botl = 1;
			dealloc_obj(ammo);
		}
#else
		freeinv(ammo);
		addinv(ammo);
#endif
		return(0);
	}

	/* if we are throwing gold, go to the function for doing that */
#ifndef GOLDOBJ
	if (ammo->oclass == COIN_CLASS)
#else
	if (ammo->oclass == COIN_CLASS && ammo != uquiver)
#endif
		return(throw_gold(ammo));


	/* reasons we can't throw ammo */
	if (!canletgo(ammo, "throw"))
		return(0);
	if ((ammo->oartifact == ART_MJOLLNIR ||
		ammo->oartifact == ART_AXE_OF_THE_DWARVISH_LORDS) &&
		ammo != uwep) {
		pline("%s must be wielded before it can be thrown.",
			The(xname(ammo)));
		return(0);
	}
	if (((ammo->oartifact == ART_MJOLLNIR ||
		ammo->oartifact == ART_AXE_OF_THE_DWARVISH_LORDS) && ACURR(A_STR) < STR19(25))
		|| (is_boulder(ammo) && !throws_rocks(youracedata) && !(u.sealsActive&SEAL_YMIR))) {
		pline("It's too heavy.");
		return(1);
	}
	if (!u.dx && !u.dy && !u.dz) {
		You("cannot throw an object at yourself.");
		return(0);
	}
	if (welded(ammo)) {
		weldmsg(ammo);
		return 1;
	}

	/* blasters */
	if (launcher && is_blaster(launcher)) {
		/* we're using charge */
		check_unpaid(launcher);
		/* unbind seals if we break taboos */
		if (launcher->ostolen && u.sealsActive&SEAL_ANDROMALIUS) unbind(SEAL_ANDROMALIUS, TRUE);
		/* degrade engravings on this spot (less) */
		u_wipe_engr(1);
	}
	/* non-blasters */
	else
	{
		/* unbind seals if we break taboos */
		if (ammo->ostolen && u.sealsActive&SEAL_ANDROMALIUS) unbind(SEAL_ANDROMALIUS, TRUE);
		if (breaktest(ammo) && u.sealsActive&SEAL_ASTAROTH) unbind(SEAL_ASTAROTH, TRUE);
		if ((ammo->otyp == EGG) && u.sealsActive&SEAL_ECHIDNA) unbind(SEAL_ECHIDNA, TRUE);
		/* degrade engravings on this spot */
		u_wipe_engr(2);
	}

	/* you touch the rubber chicken */
	if (!uarmg && !Stone_resistance && (ammo->otyp == CORPSE &&
		touch_petrifies(&mons[ammo->corpsenm]))) {
		You("throw the %s corpse with your bare %s.",
			mons[ammo->corpsenm].mname, body_part(HAND));
		Sprintf(killer_buf, "%s corpse", an(mons[ammo->corpsenm].mname));
		instapetrify(killer_buf);
	}

	/* get multishot calculation */
	multishot = calc_multishot(&youmonst, ammo, launcher, shotlimit);

	m_shot.s = (launcher) ? TRUE : FALSE;
	m_shot.o = ammo->otyp;
	m_shot.n = multishot;
	
	/* give a message if shooting more than one, or if player attempted to specify a count */
	if (ammo->oartifact == ART_FLUORITE_OCTAHEDRON){
		if (!m_shot.s){
			if (shotlimit && shotlimit < ammo->quan) You("throw %d Fluorite %s.", shotlimit, shotlimit > 1 ? "Octahedra" : "Octahedron");
			else if (ammo->quan == 8) You("throw the Fluorite Octet.");
			else You("throw %ld Fluorite %s.", ammo->quan, ammo->quan > 1 ? "Octahedra" : "Octahedron");
		}
		else if (multishot > 1 || shotlimit > 0) {
			if (ammo->quan > 1) You("shoot %d Fluorite %s.",
				multishot,	/* (might be 1 if player gave shotlimit) */
				(multishot == 1) ? "Octahedron" : "Octahedra");
		}
	}
	else if (multishot > 1 || shotlimit > 0) {
		/* "You shoot N arrows." or "You throw N daggers." */
		if (ammo->quan > 1) {
			You("%s %d %s.",
				m_shot.s ? "shoot" : "throw",
				multishot,	/* (might be 1 if player gave shotlimit) */
				(multishot == 1) ? singular(ammo, xname) : xname(ammo));
		}
		else if (multishot > 1 || shotlimit > 0) {
			You("%s %s %d time%s.",
				m_shot.s ? "shoot" : "throw",
				the(xname(ammo)),
				multishot,
				(multishot != 1 ? "s" : ""));	/* (might be 1 if player gave shotlimit) */
		}
	}

	/* get range calculation */
	int range = calc_range(&youmonst, ammo, launcher, &hurtle_dist);

	/* call projectile() to shoot n times */
	for (m_shot.i = 1; m_shot.i <= m_shot.n; m_shot.i++) {
		int dx = u.dx, dy = u.dy, dz = u.dz;
		boolean impaired = misthrow(&youmonst, ammo, launcher, m_shot.s, &dx, &dy, &dz);
		/* note: we actually don't care if the projectile hit anything */
		(void)projectile(&youmonst, ammo, launcher, m_shot.s, u.ux, u.uy, dx, dy, dz, range, forcedestroy, TRUE, impaired);
		if (Weightless || Levitation)
			hurtle(-u.dx, -u.dy, hurtle_dist, TRUE);
	}

	/* end */
	m_shot.n = m_shot.i = 0;
	m_shot.o = STRANGE_OBJECT;
	m_shot.s = FALSE;
	/* madness on losing an object */
	if (takenfromyourinv && roll_madness(MAD_TALONS)) {
		You("panic after throwing your property!");
		nomul(-1 * rnd(6), "panic");
	}
	return 1;	/* this took time */
}

/* misthrow()
 *
 * Sometimes, creatures don't throw their projectiles where they want
 * 
 * This function makes that happen by adjusting dx/dy/dz.
 *
 * returns TRUE if throw is impaired.
 */
boolean
misthrow(magr, ammo, launcher, fired, dx, dy, dz)
struct monst * magr;
struct obj * ammo;
struct obj * launcher;
boolean fired;
int * dx;
int * dy;
int * dz;
{
	boolean youagr = (magr == &youmonst);
	boolean impaired = FALSE;

	/* mis-throw/fire can change the direction of the projectile */
	if (/* needs a creature to be involved in throwing the projectile */
		magr && (
		/* cursed ammo */
		(ammo->cursed && !(youagr ? is_weldproof(youracedata) : is_weldproof_mon(magr)))
		||
		/* or flintlock */
		(fired && launcher && launcher->otyp == FLINTLOCK)
		||
		/* or greased */
		(ammo->greased)
		)
		&&
		/* at a 1/7 chance */
		(*dx || *dy) && !rn2(7)
		) {
		boolean slipok = TRUE;
		/* misfires */
		if (fired) {
			if (youagr)
				Your("weapon misfires!");
			else if (canseemon(magr) && flags.verbose)
				pline("%s misfires!", Monnam(magr));
		}
		/* slips */
		else {
			/* only greased items and proper throwing weapons can slip */
			/* only slip if it's greased or meant to be thrown */
			if (ammo->greased || throwing_weapon(ammo)) {
				/* BUG: this message is grammatically incorrect if obj has
				a plural name; greased gloves or boots for instance. */
				if (youagr)
					pline("%s as you throw it!", Tobjnam(ammo, "slip"));
				else if (canseemon(magr) && flags.verbose)
					pline("%s as %s throws it!", Tobjnam(ammo, "slip"), mon_nam(magr));
			}
			else {
				/* don't slip */
				slipok = FALSE;
			}
		}
		if (slipok) {
			/* new direction */
			*dx = rn2(3) - 1;
			*dy = rn2(3) - 1;
			if (!*dx && !*dy) {
				*dz = 1;
			}
			impaired = TRUE;
		}
	}
	/* you might be too weak to throw it */
	if (youagr &&
		(u.dx || u.dy || (u.dz < 1)) &&
		calc_capacity((int)ammo->owt) > SLT_ENCUMBER &&
		(*hp(magr) < (Upolyd ? 5 : 10) && *hp(magr) != *hpmax(magr)) &&
		ammo->owt > (*hp(magr) * 2) &&
		!Weightless) {
		You("have so little stamina, %s drops from your grasp.",
			the(xname(ammo)));
		exercise(A_CON, FALSE);
		*dx = *dy = 0;
		*dz = 1;
	}
	/* check other sources of impairment */
	if (magr) {
		if (youagr)
			impaired |= (Confusion || Stunned || Blind || Hallucination || Fumbling);
		else
			impaired |= (magr->mconf || magr->mstun || magr->mblinded);
	}

	return impaired;
}

/* 
 * onlinetohit()
 * 
 * returns TRUE if magr has a clear line of fire to (tarx,tary)
 * 
 * if called with SAFE, tries not to hit friendlies
 * if called with mdef and stoponhit, returns FALSE if a creature other than mdef will take the hit
 */
boolean
m_online(magr, mdef, tarx, tary, safe, stoponhit)
struct monst * magr;
struct monst * mdef;
int tarx;
int tary;
boolean safe;
boolean stoponhit;
{
	boolean youagr = (magr == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;

	/* First -- target must be on direct line with magr */
	if (!online2(x(magr), y(magr), tarx, tary))
		return FALSE;

	/* Second -- the points cannot be identical */
	if (x(magr) == tarx && y(magr) == tary)
		return FALSE;

	/* Third -- clear path; can cheat by using couldsee() if player is target or targeter */
	if ((youagr || (tarx == u.ux && tary == u.uy)) &&
		(!couldsee(x(magr), y(magr))))
		return FALSE;
	else if (!(clear_path(x(magr), y(magr), tarx, tary)))
		return FALSE;
	
	/* at this point, yes, it is on a line */

	if (safe || stoponhit) {
		/* try not to hit friendlies */
		int dx = sgn(tarx - x(magr)),
			dy = sgn(tary - y(magr));
		int x = x(magr);
		int y = y(magr);
		int range = BOLT_LIM; /* arbitrary */
		struct monst * mtmp;

		/* Check for creatures in the line of fire. */
		while(--range > 0)
		{
			x += dx;
			y += dy;
			if (!isok(x, y))
				break;

			/* pets don't hit player */
			if (x == u.ux && y == u.uy && magr->mtame && safe)
				return FALSE;

			/* monsters don't hit things of equal tameness (if trying to be safe) */
			mtmp = m_at(x, y);
			if (mtmp)
			{
				/* maybe we don't need to check beyond first target hit */
				if (stoponhit) {
					if (mdef && mtmp != mdef)
						return FALSE;
					else
						return TRUE;
				}
				/* Don't hit friendlies */
				if (safe && (
					(mtmp->mtame && magr->mtame) ||
					(always_peaceful(mtmp->data) && magr->mtame) ||
					(!mtmp->mtame && !magr->mtame)))
					return FALSE;
			}

		}
	}
	/* made it through, we're good to go */
	return TRUE;
}

/*
 * xbreathey() 
 * 
 * magr is breathing at (tarx, tary)
 * 
 * The player's version is fairly separate, and is in polyself.c
 * TODO: make #monster use this function; add additional functionality to make that possible
 */
boolean
xbreathey(magr, attk, tarx, tary)
struct monst * magr;
struct attack * attk;
int tarx;
int tary;
{
	boolean youagr = (magr == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	int typ = attk->adtyp;
	int mult = 1;
	static const int chromatic_dragon_breaths[] = { AD_FIRE, AD_COLD, AD_ELEC, AD_DRST, AD_DISN, AD_ACID };
	static const int platinum_dragon_breaths[] = { AD_FIRE, AD_DISN, AD_SLEE, AD_ELEC };
	static const int random_breaths[] = { AD_MAGM, AD_FIRE, AD_COLD, AD_SLEE, AD_DISN, AD_ELEC, AD_DRST, AD_ACID };
	int dx, dy, dz;

	if (tarx || tary) {
		dx = sgn(tarx - x(magr));
		dy = sgn(tary - y(magr));
		dz = 0;
	}
	else {
		dx = u.dx;
		dy = u.dy;
		dz = u.dz;
	}

	/* Random breath attacks */
	if (typ == AD_RBRE){
		if (pa == &mons[PM_CHROMATIC_DRAGON])
			typ = chromatic_dragon_breaths[rn2(SIZE(chromatic_dragon_breaths))];
		else if (pa == &mons[PM_PLATINUM_DRAGON])
			typ = platinum_dragon_breaths[rn2(SIZE(platinum_dragon_breaths))];
		else 
			typ = random_breaths[rn2(SIZE(random_breaths))];
	}
	/* Halfdragon breath attack */
	if (typ == AD_HDRG) {
		if (youagr && Race_if(PM_HALF_DRAGON))
			typ = flags.HDbreath;
		else if (is_half_dragon(pa))
			typ = magr->mvar1;
		else
			typ = AD_COLD;

		if (typ == AD_SLEE)
			mult = 4;	/* increased duration */
	}

	/* if cancelled, (or the player is strangled) can't use breath attack */
	if (youagr ? Strangled : magr->mcan) {
		if (youagr) {
			You_cant("breathe.  Sorry.");
			return FALSE;	/* took no time */
		}
		else if (flags.soundok) {
			if (canseemon(magr))
				pline("%s coughs.", Monnam(magr));
			else
				You_hear("a cough.");
			return TRUE;	/* did take time */
		}
	}

	/* message */
	if (youagr) {
		You("breathe %s!", breathwep(typ));
	}
	else if (canseemon(magr)) {
		pline("%s breathes %s!", Monnam(magr), breathwep(typ));
	}
	
	/* do the beam (sadly buzz doesn't do dz) */
	buzz(typ, FOOD_CLASS, FALSE, (int)attk->damn + min(MAX_BONUS_DICE, (mlev(magr) / 3)),
		x(magr), y(magr), dx, dy, 0, attk->damd ? (d((int)attk->damn + min(MAX_BONUS_DICE, (mlev(magr) / 3)), (int)attk->damd)*mult) : 0);
	
	/* interrupt player if they were targetted */
	if (tarx == u.ux && tary == u.uy)
		nomul(0, NULL);

	/* breath runs out sometimes. */ 
	if (!youagr) {
		if (!rn2(3))
			magr->mspec_used = 10 + rn2(20);

		/* if the player was targeted with sleep, greater cooldown */
		if ((tarx == u.ux && tary == u.uy) && typ == AD_SLEE && !Sleep_resistance)
			magr->mspec_used += rnd(20);
	}

	/* this took time */
	return TRUE;
}

/* 
 * xspity() 
 * 
 * magr is spitting at (tarx, tary)
 *
 * TODO: make 'f'ire use this function
 */
boolean
xspity(magr, attk, tarx, tary)
struct monst * magr;
struct attack * attk;
int tarx;
int tary;
{
	struct obj * otmp;
	boolean youagr = (magr == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	int typ = attk->adtyp;
	int dx, dy, dz;

	if (tarx || tary) {
		dx = sgn(tarx - x(magr));
		dy = sgn(tary - y(magr));
		dz = 0;
	}
	else {
		dx = u.dx;
		dy = u.dy;
		dz = u.dz;
	}

	/* cancelled monsters can't spit */
	if (!youagr && magr->mcan) {
		/* except for Zeta metroids, which uncancel themselves */
		if (magr->data == &mons[PM_ZETA_METROID]) //|| mtmp->data==&mons[PM_CRAZY_CHEMIST]) 
			magr->mcan = FALSE;
		else {
			if (flags.soundok)
				pline("A dry rattle comes from %s throat.",
				s_suffix(mon_nam(magr)));
			return TRUE;	/* took time */
		}
	}

	/* get otmp to spit */
	switch (typ) {
	case AD_WEBS:
		if (!youagr)
			magr->mspec_used = d(2, 6);
		otmp = mksobj(BALL_OF_WEBBING, TRUE, FALSE);
		break;
	case AD_BLND:
	case AD_DRST:
		otmp = mksobj(BLINDING_VENOM, TRUE, FALSE);
		break;
	default:
		impossible("bad attack type in xspity (%d)", typ);
		/* fall through to acid venom */
	case AD_ACID:
		otmp = mksobj(ACID_VENOM, TRUE, FALSE);
		if (attk->damn && attk->damd)
			otmp->ovar1 = d(attk->damn, attk->damd);
		break;
	}

	/* message */
	if (youagr) {
		You("spit %s!", typ == AD_WEBS ? "webbing" : "venom");
	}
	else if (canseemon(magr)) {
		pline("%s spits %s!", Monnam(magr), typ == AD_WEBS ? "webbing" : "venom");
	}
	/* there is an old convention to set spe to 1 to indicate that the venom/webbing is yours -- not sure if still needed */
	if (youagr)
		otmp->spe = 1;
	/* shoot otmp */
	projectile(magr, otmp, (struct obj *)0, FALSE,
		x(magr), y(magr), dx, dy, dz,
		BOLT_LIM, TRUE, youagr, FALSE);

	/* interrupt player if they were targetted */
	if (tarx == u.ux && tary == u.uy)
		nomul(0, NULL);

	/* this took time */
	return TRUE;
}

/* 
 * xfirey()
 *
 * magr is inherently shooting at (tarx, tary)
 *
 * TODO: make 'f'ire use this function
 */
boolean
xfirey(magr, attk, tarx, tary)		/* monster fires arrows at you */
struct monst * magr;
struct attack * attk;
int tarx;
int tary;
{
	struct obj * qvr = (struct obj *)0;				/* quiver of projectiles to use */
	boolean youagr = (magr == &youmonst);
	struct permonst * pa = youagr ? youracedata : magr->data;
	int typ = attk->adtyp;
	int xadj = 0;
	int yadj = 0;
	int rngmod = 0;
	boolean portal_projectile = FALSE;		/* if TRUE, teleports projectile directly to target */
	int ammo_type;
	int dx, dy, dz;

	if (tarx || tary) {
		dx = sgn(tarx - x(magr));
		dy = sgn(tary - y(magr));
		dz = 0;
	}
	else {
		dx = u.dx;
		dy = u.dy;
		dz = u.dz;
	}

	switch (typ) {
	case AD_SHDW:
		/* special: do not fire at warded squares */
		if (!youagr && onscary(tarx, tary, magr)) return FALSE; //Warded; did not fire
		ammo_type = SPIKE;
		qvr = mksobj(ammo_type, FALSE, FALSE);
		set_material(qvr, SHADOWSTEEL);
		qvr->quan = 1;
		qvr->spe = 8;
		qvr->opoisoned = (OPOISON_BASIC | OPOISON_BLIND);
		qvr->oproperties = (OPROP_PHSEW);
		portal_projectile = TRUE;
		/* also webs target hit. done in the function. */
		break;
	case AD_PEST:
		ammo_type = ARROW;
		qvr = mksobj(ammo_type, FALSE, FALSE);
		qvr->quan = 1;
		qvr->spe = d(7, 8) + 1; //same as touch
		qvr->opoisoned = OPOISON_FILTH;
		qvr->oproperties = OPROP_PHSEW;
		portal_projectile = TRUE;
		break;
	case AD_PLYS:
		ammo_type = SPIKE;
		qvr = mksobj(ammo_type, FALSE, FALSE);
		set_material(qvr, BONE);
		qvr->quan = 1;
		qvr->opoisoned = (OPOISON_PARAL);
		break;
	case AD_SOLR:
		ammo_type = SILVER_ARROW;
		qvr = mksobj(ammo_type, TRUE, FALSE);
		qvr->blessed = 1;
		qvr->quan = 1;
		qvr->spe = 7;
		qvr->oproperties = OPROP_PHSEW;
		//portal_projectile = TRUE;
		rngmod = 1000; /* Fly until it strikes something */
		break;
	case AD_SURY:
		ammo_type = SILVER_ARROW;
		qvr = mksobj(ammo_type, TRUE, FALSE);
		// qvr->oartifact = ART_ARROW_OF_SLAYING;
		qvr->blessed = 1;
		qvr->quan = 1;
		qvr->spe = 7 + 50; //Arrows of slaying actually just get +50 damage anyway :/
		qvr->oproperties = OPROP_PHSEW;
		//portal_projectile = TRUE;
		rngmod = 1000; /* Fly until it strikes something */
		break;
	case AD_SLVR:
		ammo_type = SILVER_ARROW;
		break;
	case AD_BALL:
		ammo_type = HEAVY_IRON_BALL;
		qvr = mksobj(ammo_type, FALSE, FALSE);
		rngmod = 8;
		break;
	case AD_LOAD:
		ammo_type = LOADSTONE;
		qvr = mksobj(ammo_type, FALSE, FALSE);
		qvr->cursed = 1;
		rngmod = 8;
		break;
	case AD_BLDR:
		ammo_type = BOULDER;
		qvr = mksobj(ammo_type, FALSE, FALSE);
		rngmod = 8;
		break;
	case AD_VBLD:
		ammo_type = HEAVY_IRON_BALL;
		qvr = mksobj(ammo_type, FALSE, FALSE);
		rngmod = 8;
		/* volley -- inaccurate */
		if		(tary == y(magr))
			yadj = d(1, 3) - 2;
		else if (tarx == x(magr))
			xadj = d(1, 3) - 2;
		else if (tarx - x(magr) == tary - y(magr)){
			xadj = d(1, 3) - 2;
			yadj = -1 * xadj;
		}
		else
			xadj = yadj = d(1, 3) - 2;
		break;
	default:
		ammo_type = ARROW;
		break;
	}

	if (!qvr){
		/* quiver from inventory */
		for (qvr = (youagr ? invent : magr->minvent); qvr; qvr = qvr->nobj){
			if (qvr->otyp == ammo_type) break;
		}
		/* No ammo of the right type found, nothing happened, took no time */
		if (!qvr)
			return FALSE; 
	}

	/* check that attacker is in range of target */
	if (BOLT_LIM + rngmod < distmin(x(magr), y(magr), tarx, tary))
		return FALSE;

	/* don't message here , because this function is often called several times :( */

	/* Fire the projectile */
	if (portal_projectile) {
		/* start the projectile adjacent to the target */
		projectile(magr, qvr, (struct obj *)0, TRUE,
			tarx-dx, tary-dy, dx, dy, dz,
			1, TRUE, youagr, FALSE);
	}
	else {
		/* start the projectile at magr's location, modified by xadj and yadj */
		projectile(magr, qvr, (struct obj *)0, TRUE,
			x(magr)+xadj, y(magr)+yadj, dx, dy, dz,
			BOLT_LIM+rngmod, TRUE, youagr, FALSE);
	}

	/* shadow bolts web the target hit */
	if (typ == AD_SHDW) {
		struct trap *ttmp2;
		ttmp2 = maketrap(bhitpos.x, bhitpos.y, WEB);
		if (bhitpos.x == u.ux && bhitpos.y == u.uy && ttmp2) {
			pline_The("webbing sticks to you. You're caught!");
			dotrap(ttmp2, NOWEBMSG);
#ifdef STEED
			if (u.usteed && u.utrap) {
				/* you, not steed, are trapped */
				dismount_steed(DISMOUNT_FELL);
			}
#endif
		}
		else if (ttmp2) {
			struct monst * mdef = m_at(bhitpos.x, bhitpos.y);
			if (mdef)
				mintrap(mdef);
		}
	}

	/* interrupt player if they were targetted */
	if (tarx == u.ux && tary == u.uy)
		nomul(0, NULL);
	/* this took time */
	return TRUE;
}

/* mdofire()
 *
 * Monster attepts a ranged weapon attack against either the player or a monster
 *
 * Includes guns, blasters, thrown and fired objects
 * NOT polearms.
 */ 
boolean
mdofire(magr, mdef, tarx, tary)
struct monst * magr;
struct monst * mdef;
int tarx;
int tary;
{
	boolean youdef = (mdef && mdef == &youmonst);
	struct obj * thrownobj;
	struct obj * launcher;
	int x = x(magr);
	int y = y(magr);
	int multishot;
	const char *onm;
	boolean mass_pistol = FALSE;
	int result = 0;

	/* AI: If target is you and you are coming toward the monster, the monster
	 * should try to soften you up with missiles.  If you are
	 * going away, you are probably hurt or running.  Give
	 * chase, but if you are getting too far away, throw.
	 * If we have already made a ranged attack this turn, we aren't chasing. Throw.
	 */
	if (youdef && (distmin(u.ux, u.uy, x, y) > distmin(u.ux0, u.uy0, x, y)) &&
		rn2(BOLT_LIM - distmin(x, y, tarx, tary)) && mon_ranged_gazeonly)
		return FALSE;	/* choose not to attack */
	/* Select ranged weapon to throw -- polearm, blaster, ammo, throwable */
	thrownobj = select_rwep(magr);
	if (!thrownobj)
		return FALSE;	/* didn't find an appropriate object */
	/* polearms are handled in xattacky as melee attacks -- if we got here, we were too far away to use it. */
	if (is_pole(thrownobj)) {
		return FALSE;
	}
	/* if we picked a blaster, fix up launcher/thrownobj confusion */
	if (is_blaster(thrownobj)) {
		launcher = thrownobj;
		thrownobj = blaster_ammo(launcher);
	}

	/* If we picked a blaster, or ammo for a wielded launcher, fire all */
	if ((MON_WEP(magr)  && ((ammo_and_launcher(thrownobj, MON_WEP(magr) )) || (MON_WEP(magr)  == thrownobj && is_blaster(thrownobj)))) ||
		(MON_SWEP(magr) && ((ammo_and_launcher(thrownobj, MON_SWEP(magr))) || (MON_SWEP(magr) == thrownobj && is_blaster(thrownobj))))
		)
	{
		struct obj * launcher;
		int hand;
		/* do mainhand, then offhand */
		for (hand = 0; hand < 2; hand++) {
			launcher = (!hand ? MON_WEP(magr) : MON_SWEP(magr));
			if (!launcher || !((ammo_and_launcher(thrownobj, launcher)) || (launcher == thrownobj && is_blaster(launcher))))
				continue;

			if (ammo_and_launcher(thrownobj, launcher)) {
				/* simply fire from the launcher */
				result |= mthrow(magr, thrownobj, launcher, tarx, tary, FALSE);
			}
			else if (is_blaster(launcher)) {
				/* blasters need to generate their ammo on the fly, but that was generated earlier, when fixing launcher/thrownobj confusion */

				/* do we have enough charge to fire? */
				if (!launcher->ovar1) {
					/* nothing happens */
					magr->weapon_check = NEED_WEAPON;	/* magr figures out it needs new weapons */
				}
				else {
					switch (launcher->otyp) {
					case CUTTING_LASER:
					case HAND_BLASTER:
					case ARM_BLASTER:
					case MASS_SHADOW_PISTOL:
						/* no special changes required */
						break;
					case RAYGUN:
						// TODO: monster raygun function
						//if (!getdir((char *)0))
						//	result |= zap_raygun(launcher, calc_multishot(&youmonst, ammo, launcher, shotlimit), shotlimit);
						obfree(thrownobj, 0);
						thrownobj = (struct obj *)0;
						break;
					default:
						impossible("Unhandled blaster %d!", launcher->otyp);
						break;
					}
					/* always destroy ammo fired from a blaster */
					if (thrownobj) {
						result |= mthrow(magr, thrownobj, launcher, tarx, tary, TRUE);
						/* and now delete thrownobj, which was generated from a blaster */
						obfree(thrownobj, 0);
					}
				}
			}
		}
	}
	/* Otherwise, we will throw thrownobj -- if it's not an arrow/bolt/bullet etc, which are dumb to throw */
	else if (!is_ammo(thrownobj) || thrownobj->oclass == GEM_CLASS || is_grenade(thrownobj))
	{
		result |= mthrow(magr, thrownobj, (struct obj *)0, tarx, tary, FALSE);
	}
	return (result > 0);
}

/* mthrow()
 *
 * Monster attepts a ranged weapon attack against either the player or a monster
 * Monster's equivalent to uthrow(), which is to say generally the same but without a lot of player interaction
 *
 * Actually returns MM_HIT/MISS etc markers.
 */
int
mthrow(magr, ammo, launcher, tarx, tary, forcedestroy)
struct monst * magr;
struct obj * ammo;
struct obj * launcher;
int tarx;
int tary;
boolean forcedestroy;
{
	int result;
	int multishot;
	int hurtle_dist = 0;	/* not used at the moment, but needed for range calculation */
	int odx, ody;

	/* figure out dx and dy */
	odx = sgn(tarx - x(magr));
	ody = sgn(tary - y(magr));

	/* get multishot calculation */
	multishot = calc_multishot(magr, ammo, launcher, 999);

	m_shot.s = (launcher) ? TRUE : FALSE;
	m_shot.o = ammo->otyp;
	m_shot.n = multishot;

	/* message (TODO: check if working correctly) */
	if (canseemon(magr)) {
		char onmbuf[BUFSZ];
		const char * onm;

		if (multishot > 1) {
			/* "N arrows"; multishot > 1 implies otmp->quan > 1, so
			xname()'s result will already be pluralized */
			Sprintf(onmbuf, "%d %s", multishot, xname(ammo));
			onm = onmbuf;
		}
		else {
			/* "an arrow" */
			onm = singular(ammo, xname);
			onm = obj_is_pname(ammo) ? the(onm) : an(onm);
		}
		pline("%s %s %s!", Monnam(magr),
			m_shot.s ? is_bullet(ammo) ? "fires" : "shoots" : "throws",
			onm);
	}
	else {
		m_shot.o = STRANGE_OBJECT;	/* don't give multishot feedback */
	}

	/* get range calculation */
	int range = calc_range(magr, ammo, launcher, &hurtle_dist);

	/* call projectile() to shoot n times */
	for (m_shot.i = 1; m_shot.i <= m_shot.n; m_shot.i++) {
		int dx = odx, dy = ody, dz = 0;
		boolean impaired = misthrow(magr, ammo, launcher, m_shot.s, &dx, &dy, &dz);
		/* note: we actually don't care if the projectile hit anything */
		result = projectile(magr, ammo, launcher, m_shot.s, x(magr), y(magr), dx, dy, dz, range, forcedestroy, FALSE, impaired);
		/* monsters don't hurtle like the player does at the moment */
	}

	/* end */
	m_shot.n = m_shot.i = 0;
	m_shot.o = STRANGE_OBJECT;
	m_shot.s = FALSE;
	return result;
}