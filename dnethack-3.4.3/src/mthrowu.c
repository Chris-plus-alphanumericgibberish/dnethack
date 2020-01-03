/*	SCCS Id: @(#)mthrowu.c	3.4	2003/05/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "mfndpos.h" /* ALLOW_M */

//STATIC_DCL int FDECL(drop_throw,(struct monst *, struct obj *,BOOLEAN_P,int,int));
extern char* FDECL(breathwep, (int));

#define URETREATING(x,y) (distmin(u.ux,u.uy,x,y) > distmin(u.ux0,u.uy0,x,y))

#define POLE_LIM 5	/* How far monsters can use pole-weapons */

static const int dirx[8] = {0, 1, 1,  1,  0, -1, -1, -1},
				 diry[8] = {1, 1, 0, -1, -1, -1,  0,  1};

/* 
 * replace the old char*breathwep[] array
 * not dependent on order anymore
 */
char *
breathwep(atype)
int atype;
{
	switch (atype)
	{
	case AD_MAGM: return "fragments";
	case AD_FIRE: return "fire";
	case AD_COLD: return "frost";
	case AD_SLEE: return "sleep gas";
	case AD_DISN: return "a disintegration blast";
	case AD_ELEC: return "lightning";
	case AD_DRST: return "poison gas";
	case AD_ACID: return "acid";
	case AD_DRLI: return "dark energy";
	case AD_GOLD: return "gold";
	default:
		impossible("unaccounted-for breath type in breathwep: %d", atype);
		return "404 BREATH NOT FOUND";
	}
}

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
		if (obj->timed) obj_stop_timers(obj);
		if (obj->owornmask) {
		    mon->misc_worn_check &= ~obj->owornmask;
		    update_mon_intrinsics(mon, obj, FALSE, FALSE);
		}
		obfree(obj, (struct obj*) 0);
	}
}

#endif /* OVLB */
#ifdef OVL1

extern int monstr[];

/* Find a target for a ranged attack. */
struct monst *
mfind_target(mtmp, force_linedup)
struct monst *mtmp;
int force_linedup;
{
    int dir, origdir = -1;
    int x, y, dx, dy;

    int i;

    struct monst *mat, *mret = (struct monst *)0, *oldmret = (struct monst *)0;
	struct monst *mtmp2;
	
    boolean conflicted = Conflict && couldsee(mtmp->mx,mtmp->my) && 
						(distu(mtmp->mx,mtmp->my) <= BOLT_LIM*BOLT_LIM) &&
						!resist(mtmp, RING_CLASS, 0, 0);
	
	struct obj *mrwep = select_rwep(mtmp);
	
	if(is_derived_undead_mon(mtmp)) return 0;
	
	
    if (is_covetous(mtmp->data) && !mtmp->mtame)
    {
        /* find our mark and let him have it, if possible! */
        register int gx = STRAT_GOALX(mtmp->mstrategy),
                     gy = STRAT_GOALY(mtmp->mstrategy);
        mtmp2 = m_at(gx, gy);
	if (mtmp2 && (!force_linedup || mlined_up(mtmp, mtmp2, FALSE)) && (
			   (attacktype(mtmp->data, AT_BREA) && !mtmp->mcan && mlined_up(mtmp, mtmp2, FALSE))
			|| (attacktype(mtmp->data, AT_SPIT) && mlined_up(mtmp, mtmp2, FALSE))
			|| (attacktype(mtmp->data, AT_TNKR) && mlined_up(mtmp, mtmp2, FALSE))
			|| (attacktype(mtmp->data, AT_ARRW) && mlined_up(mtmp, mtmp2, FALSE))
			|| (attacktype(mtmp->data, AT_BEAM) && mlined_up(mtmp, mtmp2, FALSE))
			|| (attacktype(mtmp->data, AT_MAGC) && !mtmp->mcan && (
			  ((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp <= AD_SPC2) ||
			  ((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp == AD_RBRE) ||
			  ((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp == AD_OONA) )
				&& mlined_up(mtmp, mtmp2, FALSE)
			  )
			|| (attacktype(mtmp->data, AT_MMGC) && !mtmp->mcan && (
			  ((attacktype_fordmg(mtmp->data, AT_MMGC, AD_ANY))->adtyp <= AD_SPC2) ||
			  ((attacktype_fordmg(mtmp->data, AT_MMGC, AD_ANY))->adtyp == AD_RBRE) ||
			  ((attacktype_fordmg(mtmp->data, AT_MMGC, AD_ANY))->adtyp == AD_OONA) )
				&& mlined_up(mtmp, mtmp2, FALSE)
			  )
			|| (is_commander(mtmp->data) && distmin(mtmp2->mx, mtmp2->my, mtmp->mx, mtmp->my) < BOLT_LIM)
			|| (attacktype(mtmp->data, AT_LRCH) && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) <= 2)
			|| (attacktype(mtmp->data, AT_LNCK) && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) <= 2)
			|| (attacktype(mtmp->data, AT_5SQR) && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) <= 5)
			|| (attacktype(mtmp->data, AT_5SBT) && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) <= 5)
			|| (attacktype(mtmp->data, AT_GAZE) && !mtmp->mcan && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MAGC, AD_SPEL) && !mtmp->mcan && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MAGC, AD_CLRC) && !mtmp->mcan && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MMGC, AD_SPEL) && !mtmp->mcan && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MMGC, AD_CLRC) && !mtmp->mcan && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_WEAP, AD_PHYS) && mrwep && (
				(!is_pole(mrwep) && mlined_up(mtmp, mtmp2, FALSE)) ||
				( is_pole(mrwep) && mlined_up(mtmp, mtmp2, FALSE) && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) <= 2 && (mtmp2->mx == mtmp->mx || mtmp2->my == mtmp->my))
			   ))
		)
	){
	    if(!(mtmp->data == &mons[PM_OONA] && resists_oona(mtmp2)) && mm_aggression(mtmp, mtmp2)) return mtmp2;
	}
	
#if 0
	if (!is_mplayer(mtmp->data)/* || !(mtmp->mstrategy & STRAT_NONE)*/)
	{
		return 0;
	}
#endif
    	if (!mtmp->mpeaceful && !conflicted &&
			((mtmp->mstrategy & STRAT_STRATMASK) == STRAT_NONE) && (
			   (attacktype(mtmp->data, AT_BREA) && !mtmp->mcan && lined_up(mtmp))
			|| (attacktype(mtmp->data, AT_SPIT) && lined_up(mtmp))
			|| (attacktype(mtmp->data, AT_TNKR) && lined_up(mtmp))
			|| (attacktype(mtmp->data, AT_ARRW) && lined_up(mtmp))
			|| (attacktype(mtmp->data, AT_BEAM) && lined_up(mtmp))
			|| (attacktype(mtmp->data, AT_MAGC) && !mtmp->mcan && (
			  ((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp <= AD_SPC2) ||
			  ((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp == AD_RBRE) ||
			  ((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp == AD_OONA) )
				&& lined_up(mtmp)
			  )
			|| (attacktype(mtmp->data, AT_MMGC) && !mtmp->mcan && (
			  ((attacktype_fordmg(mtmp->data, AT_MMGC, AD_ANY))->adtyp <= AD_SPC2) ||
			  ((attacktype_fordmg(mtmp->data, AT_MMGC, AD_ANY))->adtyp == AD_RBRE) ||
			  ((attacktype_fordmg(mtmp->data, AT_MMGC, AD_ANY))->adtyp == AD_OONA) )
				&& lined_up(mtmp)
			  )
			|| (is_commander(mtmp->data) && distmin(mtmp->mux, mtmp->muy, mtmp->mx, mtmp->my) < BOLT_LIM)
			|| (attacktype(mtmp->data, AT_LRCH) && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) <= 2)
			|| (attacktype(mtmp->data, AT_LNCK) && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) <= 2)
			|| (attacktype(mtmp->data, AT_5SQR) && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) <= 5)
			|| (attacktype(mtmp->data, AT_5SBT) && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) <= 5)
			|| (attacktype(mtmp->data, AT_GAZE) && !mtmp->mcan && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MAGC, AD_SPEL) && !mtmp->mcan && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MAGC, AD_CLRC) && !mtmp->mcan && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MMGC, AD_SPEL) && !mtmp->mcan && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MMGC, AD_CLRC) && !mtmp->mcan && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_WEAP, AD_PHYS) && mrwep && (
				(!is_pole(mrwep) && lined_up(mtmp)) ||
				( is_pole(mrwep) && lined_up(mtmp) && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) <= 2 && (mtmp->mux == mtmp->mx || mtmp->muy == mtmp->my))
			   ))
		)) {
        	if(!(mtmp->data == &mons[PM_OONA] && Oona_resistance)) return &youmonst;  /* kludge - attack the player first
				      if possible */
		}
		if(gx != 0 || gy != 0){
			for (dir = 0; dir < 8; dir++)
				if (dirx[dir] == sgn(gx-mtmp->mx) &&
					diry[dir] == sgn(gy-mtmp->my))
						break;

			if (dir == 8) {
				tbx = tby = 0;
				return 0;
			}
		} else {
			dir = rn2(8);
		}

		origdir = -1;
    } else {
		int i, j;
		struct monst *tmpm;
    	dir = rn2(8);
		origdir = -1;

		//Don't make ranged attacks if in melee
		for(i=-1;i<2;i++){
			for(j=-1;j<2;j++){
				if(!i && !j)
					continue;
				if(isok(mtmp->mx+i, mtmp->my+j)){
					tmpm = m_at(mtmp->mx+i, mtmp->my+j);
					if(tmpm && mm_aggression(mtmp, tmpm)){
						return (struct monst *) 0;
					}
				}
			}
		}
	
    	if (!mtmp->mpeaceful && (!force_linedup || lined_up(mtmp)) && !conflicted && (
			   (attacktype(mtmp->data, AT_BREA) && !mtmp->mcan && lined_up(mtmp))
			|| (attacktype(mtmp->data, AT_SPIT) && lined_up(mtmp))
			|| (attacktype(mtmp->data, AT_TNKR) && lined_up(mtmp))
			|| (attacktype(mtmp->data, AT_ARRW) && lined_up(mtmp))
			|| (attacktype(mtmp->data, AT_BEAM) && lined_up(mtmp))
			|| (attacktype(mtmp->data, AT_MAGC) && !mtmp->mcan && (
			  ((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp <= AD_SPC2) ||
			  ((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp == AD_RBRE) ||
			  ((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp == AD_OONA) )
				&& lined_up(mtmp)
			  )
			|| (attacktype(mtmp->data, AT_MMGC) && !mtmp->mcan && (
			  ((attacktype_fordmg(mtmp->data, AT_MMGC, AD_ANY))->adtyp <= AD_SPC2) ||
			  ((attacktype_fordmg(mtmp->data, AT_MMGC, AD_ANY))->adtyp == AD_RBRE) ||
			  ((attacktype_fordmg(mtmp->data, AT_MMGC, AD_ANY))->adtyp == AD_OONA) )
				&& lined_up(mtmp)
			  )
			|| (is_commander(mtmp->data) && distmin(mtmp->mux, mtmp->muy, mtmp->mx, mtmp->my) < BOLT_LIM)
			|| (attacktype(mtmp->data, AT_LRCH) && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) <= 2)
			|| (attacktype(mtmp->data, AT_LNCK) && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) <= 2)
			|| (attacktype(mtmp->data, AT_5SQR) && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) <= 5)
			|| (attacktype(mtmp->data, AT_5SBT) && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) <= 5)
			|| (attacktype(mtmp->data, AT_GAZE) && !mtmp->mcan && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MAGC, AD_SPEL) && !mtmp->mcan && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MAGC, AD_CLRC) && !mtmp->mcan && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MMGC, AD_SPEL) && !mtmp->mcan && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MMGC, AD_CLRC) && !mtmp->mcan && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_WEAP, AD_PHYS) && mrwep && (
				(!is_pole(mrwep) && lined_up(mtmp)) ||
				( is_pole(mrwep) && lined_up(mtmp) && distmin(mtmp->mux,mtmp->muy,mtmp->mx,mtmp->my) <= 2 && (mtmp->mux == mtmp->mx || mtmp->muy == mtmp->my))
			   ))
		)) {
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
	        if (mtmp->mtame && !mat->mtame && i > 0) {
				if (((!oldmret) ||
					(monstr[monsndx(mat->data)] >
					monstr[monsndx(oldmret->data)])
					) && !(mtmp->data == &mons[PM_OONA] && resists_oona(mat))
					 && mm_aggression(mtmp, mat)
				) mret = mat;
			}
			else if ((mm_aggression(mtmp, mat) & ALLOW_M)
				|| conflicted)
			{
				if (mtmp->mtame && !conflicted){
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
						 && mm_aggression(mtmp, mat)
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
	
	if(!mret && !force_linedup) for(mtmp2 = fmon; mtmp2; mtmp2 = mtmp2->nmon){
		if(mtmp == mtmp2) continue;
    	if ((!!mtmp->mtame != !!mtmp2->mtame || conflicted || (mm_aggression(mtmp, mtmp2) & ALLOW_M)) &&
			!mlined_up(mtmp, mtmp2, FALSE) && //Note: must be something we don't want to hit in the way.
			(
			   (is_commander(mtmp->data) && distmin(mtmp2->mx, mtmp2->my, mtmp->mx, mtmp->my) < BOLT_LIM)
			|| (attacktype(mtmp->data, AT_LRCH) && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) <= 2)
			|| (attacktype(mtmp->data, AT_LNCK) && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) <= 2)
			|| (attacktype(mtmp->data, AT_5SQR) && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) <= 5)
			|| (attacktype(mtmp->data, AT_5SBT) && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) <= 5)
			|| (attacktype(mtmp->data, AT_GAZE) && !mtmp->mcan && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MAGC, AD_SPEL) && !mtmp->mcan && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MAGC, AD_CLRC) && !mtmp->mcan && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MMGC, AD_SPEL) && !mtmp->mcan && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) < BOLT_LIM)
			|| (attacktype_fordmg(mtmp->data, AT_MMGC, AD_CLRC) && !mtmp->mcan && distmin(mtmp2->mx,mtmp2->my,mtmp->mx,mtmp->my) < BOLT_LIM)
		)) {
			if (((!mret) ||
				(monstr[monsndx(mtmp2->data)] >
				monstr[monsndx(mret->data)])
				) && !(mtmp->data == &mons[PM_OONA] && resists_oona(mtmp2))
				 && mm_aggression(mtmp, mtmp2)
			){
				mret = mtmp2;
			}
		}
	}
	
    if (mret != (struct monst *)0) {
		if(!mlined_up(mtmp, mret, FALSE)){
			tbx = tby = 0;
		}
        return mret; /* should be the strongest monster that's not behind
	                a friendly */
    }

    /* Nothing lined up? */
    tbx = tby = 0;
    return (struct monst *)0;
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
			oskill != P_SPEAR && oskill != P_KNIFE);	/* but not dagger */
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
			obj_type != LIVING_MASK &&
			obj_type != R_LYEHIAN_FACEPLATE &&
			obj_type != WHISTLE &&
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
		otmp->obj_material == GOLD ||
		otmp->obj_material == SILVER)
	    pline("Clink!");
	else
	    pline("Clonk!");
    }

    return hits;
}

#endif /* OVL0 */

/*mthrowu.c*/
