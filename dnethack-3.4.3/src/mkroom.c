/*	SCCS Id: @(#)mkroom.c	3.4	2001/09/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Entry points:
 *	mkroom() -- make and stock a room of a given type
 *	nexttodoor() -- return TRUE if adjacent to a door
 *	has_dnstairs() -- return TRUE if given room has a down staircase
 *	has_upstairs() -- return TRUE if given room has an up staircase
 *	courtmon() -- generate a court monster
 *	save_rooms() -- save rooms into file fd
 *	rest_rooms() -- restore rooms from file fd
 */

#include "hack.h"

#ifdef OVLB
STATIC_DCL boolean FDECL(isbig, (struct mkroom *));
STATIC_DCL boolean FDECL(isspacious, (struct mkroom *));
STATIC_DCL void NDECL(mkshop), FDECL(mkzoo,(int)), NDECL(mkswamp);
STATIC_DCL void NDECL(mktemple);
STATIC_DCL void FDECL(mkgarden, (struct mkroom *));
STATIC_DCL void FDECL(mklibrary, (struct mkroom *));
STATIC_DCL void NDECL(mkisland);
STATIC_DCL void NDECL(mkriver);
STATIC_DCL void FDECL(liquify, (xchar, xchar, boolean));
STATIC_DCL coord * FDECL(shrine_pos, (int));
STATIC_DCL struct permonst * NDECL(morguemon);
STATIC_DCL struct permonst * NDECL(antholemon);
STATIC_DCL struct permonst * NDECL(squadmon);
STATIC_DCL void FDECL(save_room, (int,struct mkroom *));
STATIC_DCL void FDECL(rest_room, (int,struct mkroom *));
#endif /* OVLB */

#define sq(x) ((x)*(x))

extern const struct shclass shtypes[];	/* defined in shknam.c */

#ifdef OVLB

STATIC_OVL boolean
isbig(sroom)
register struct mkroom *sroom;
{
	register int area = (sroom->hx - sroom->lx + 1)
			   * (sroom->hy - sroom->ly + 1);
	return((boolean)( area > 20 ));
}

/* Returns true if room has both an X and Y size of at least five. */
STATIC_OVL boolean
isspacious(sroom)
register struct mkroom *sroom;
{
	return((boolean)( ((sroom->hx - sroom-> lx)+1) >= 5 &&
		(((sroom->hy - sroom->ly)+1) >=5 ) ));
}

void
mksepulcher()
{
	int x,y,tries=0;
	int i,j;
	boolean good=FALSE,okspot;
	while(!good && tries < 50){
		x = rn2(COLNO-7)+3;
		y = rn2(ROWNO-7)+3;
		tries++;
		okspot = TRUE;
		for(i=-1;i<2;i++) for(j=-1;j<2;j++) if(levl[x+i][y+j].typ != STONE) okspot = FALSE;
		for(i=-2;i<3;i++) for(j=-2;j<3;j++) if(levl[x+i][y+j].typ >= CORR) okspot = FALSE;
		if(okspot){
			good = TRUE;
			for(i=-1;i<2;i++) for(j=-1;j<2;j++) levl[x+i][y+j].typ = STONE;
			levl[x+2][y+2].typ = BRCORNER;
			levl[x-2][y+2].typ = BLCORNER;
			for(i=-1;i<2;i++) levl[x+i][y+2].typ = HWALL;
			for(i=-1;i<2;i++) levl[x+i][y-2].typ = HWALL;
			for(i=-1;i<2;i++) levl[x+2][y+i].typ = VWALL;
			for(i=-1;i<2;i++) levl[x-2][y+i].typ = VWALL;
			levl[x+2][y-2].typ = TRCORNER;
			levl[x-2][y-2].typ = TLCORNER;
			levl[x][y].typ = ROOM;
			for(i=-2;i<3;i++) {
				for(j=-2;j<3;j++) {
					levl[x][y].lit = 0;
					levl[x][y].wall_info |= W_NONDIGGABLE;
				}
			}
			makemon(&mons[PM_DREAD_SERAPH], x, y, 0);
		}
	}
}

void
mkroom(roomtype)
/* make and stock a room of a given type */
int	roomtype;
{
    if (roomtype >= SHOPBASE)
		mkshop();	/* someday, we should be able to specify shop type */
    else switch(roomtype) {
	case COURT:	mkzoo(COURT); break;
	case ZOO:	mkzoo(ZOO); break;
	case BEEHIVE:	mkzoo(BEEHIVE); break;
	case MORGUE:	mkzoo(MORGUE); break;
	case BARRACKS:	mkzoo(BARRACKS); break;
	case SWAMP:	mkswamp(); break;
	case GARDEN:	mkgarden((struct mkroom *)0); break;
	case LIBRARY:	mklibrary((struct mkroom *)0); break;
	case TEMPLE:	mktemple(); break;
	case LEPREHALL:	mkzoo(LEPREHALL); break;
	case COCKNEST:	mkzoo(COCKNEST); break;
	case ANTHOLE:	mkzoo(ANTHOLE); break;
	case ISLAND: mkisland(); break;
	case RIVER: mkriver(); break;	default:	impossible("Tried to make a room of type %d.", roomtype);
	case POOLROOM:	mkpoolroom(); break;
    }
}

STATIC_OVL void
mkshop()
{
	register struct mkroom *sroom;
	int i = -1;
#ifdef WIZARD
	char *ep = (char *)0;	/* (init == lint suppression) */

	/* first determine shoptype */
	if(wizard){
#ifndef MAC
		ep = nh_getenv("SHOPTYPE");
		if(ep){
			if(*ep == 'z' || *ep == 'Z'){
				mkzoo(ZOO);
				return;
			}
			if(*ep == 'm' || *ep == 'M'){
				mkzoo(MORGUE);
				return;
			}
			if(*ep == 'b' || *ep == 'B'){
				mkzoo(BEEHIVE);
				return;
			}
			if(*ep == 't' || *ep == 'T' || *ep == '\\'){
				mkzoo(COURT);
				return;
			}
			if(*ep == 's' || *ep == 'S'){
				mkzoo(BARRACKS);
				return;
			}
			if(*ep == 'a' || *ep == 'A'){
				mkzoo(ANTHOLE);
				return;
			}
			if(*ep == 'c' || *ep == 'C'){
				mkzoo(COCKNEST);
				return;
			}
			if(*ep == 'l' || *ep == 'L'){
				mkzoo(LEPREHALL);
				return;
			}
			if(*ep == 'o' || *ep == 'O'){
				mkpoolroom();
				return;
			}
			if(*ep == '_'){
				mktemple();
				return;
			}
			if(*ep == 'n'){
				mkgarden((struct mkroom *)0);
				return;
			}
			if(*ep == '\''){
				mklibrary((struct mkroom *)0);
				return;
			}
			if(*ep == '}'){
				mkswamp();
				return;
			}
			if(*ep == 'w' || *ep == 'W'){
				mkisland();
				return;
			}
			for(i=0; shtypes[i].name; i++)
				if(*ep == def_oc_syms[(int)shtypes[i].symb])
				    goto gottype;
			if(*ep == 'g' || *ep == 'G')
				i = 0;
			else
				i = -1;
		}
#endif
	}
#ifndef MAC
gottype:
#endif
#endif
	for(sroom = &rooms[0]; ; sroom++){
		if(sroom->hx < 0) return;
		if(sroom - rooms >= nroom) {
			pline("rooms not closed by -1?");
			return;
		}
		if(sroom->rtype != OROOM) continue;
		if(has_dnstairs(sroom) || has_upstairs(sroom))
			continue;
		if(
#ifdef WIZARD
		   (wizard && ep && sroom->doorct != 0) ||
#endif
			sroom->doorct == 1) break;
	}
	if (!sroom->rlit) {
		int x, y;

		for(x = sroom->lx - 1; x <= sroom->hx + 1; x++)
		for(y = sroom->ly - 1; y <= sroom->hy + 1; y++)
			levl[x][y].lit = 1;
		sroom->rlit = 1;
	}

	if(i < 0) {			/* shoptype not yet determined */
	    register int j;

	    /* pick a shop type at random */
	    for (j = rnd(100), i = 0; (j -= shtypes[i].prob) > 0; i++)
		continue;

	    /* big rooms cannot be wand or book shops,
	     * - so make them general stores
	     */
	    if(isbig(sroom) && (shtypes[i].symb == WAND_CLASS
				|| shtypes[i].symb == SPBOOK_CLASS)) i = 0;
	}
	sroom->rtype = SHOPBASE + i;

	/* set room bits before stocking the shop */
#ifdef SPECIALIZATION
	topologize(sroom, FALSE); /* doesn't matter - this is a special room */
#else
	topologize(sroom);
#endif

	/* stock the room with a shopkeeper and artifacts */
	stock_room(i, sroom);
}

struct mkroom *
pick_room(strict)
register boolean strict;
/* pick an unused room, preferably with only one door */
{
	register struct mkroom *sroom;
	register int i = nroom;

	for(sroom = &rooms[rn2(nroom)]; i--; sroom++) {
		if(sroom == &rooms[nroom])
			sroom = &rooms[0];
		if(sroom->hx < 0)
			return (struct mkroom *)0;
		if(sroom->rtype != OROOM)	continue;
		if(!strict) {
		    if(has_upstairs(sroom) || (has_dnstairs(sroom) && rn2(3)))
			continue;
		} else if(has_upstairs(sroom) || has_dnstairs(sroom))
			continue;
		if(sroom->doorct == 1 || !rn2(5)
#ifdef WIZARD
						|| wizard
#endif
							)
			return sroom;
	}
	return (struct mkroom *)0;
}

STATIC_OVL void
mkzoo(type)
int type;
{
	register struct mkroom *sroom;

	if ((sroom = pick_room(FALSE)) != 0) {
		sroom->rtype = type;
		fill_zoo(sroom);
	}
}

void
fill_zoo(sroom)
struct mkroom *sroom;
{
	struct monst *mon;
	register int sx,sy,i;
	int sh, tx, ty, goldlim, type = sroom->rtype;
	int rmno = (sroom - rooms) + ROOMOFFSET;
	coord mm;

#ifdef GCC_WARN
	tx = ty = goldlim = 0;
#endif

	sh = sroom->fdoor;
	switch(type) {
		case GARDEN:
			mkgarden(sroom);
			/* mkgarden() sets flags and we don't want other fillings */
		return; 
		case LIBRARY:
			mklibrary(sroom);
			/* mklibrary() sets flags and we don't want other fillings */
		return;
	    case COURT:
		if(level.flags.is_maze_lev) {
		    for(tx = sroom->lx; tx <= sroom->hx; tx++)
			for(ty = sroom->ly; ty <= sroom->hy; ty++)
			    if(IS_THRONE(levl[tx][ty].typ))
				goto throne_placed;
		}
		i = 100;
		do {	/* don't place throne on top of stairs */
			(void) somexy(sroom, &mm);
			tx = mm.x; ty = mm.y;
		} while (occupied((xchar)tx, (xchar)ty) && --i > 0);
	    throne_placed:
		/* TODO: try to ensure the enthroned monster is an M2_PRINCE */
		break;
	    case BEEHIVE:
		tx = sroom->lx + (sroom->hx - sroom->lx + 1)/2;
		ty = sroom->ly + (sroom->hy - sroom->ly + 1)/2;
		if(sroom->irregular) {
		    /* center might not be valid, so put queen elsewhere */
		    if ((int) levl[tx][ty].roomno != rmno ||
			    levl[tx][ty].edge) {
			(void) somexy(sroom, &mm);
			tx = mm.x; ty = mm.y;
		    }
		}
		break;
	    case ZOO:
	    case LEPREHALL:
		goldlim = 500 * level_difficulty();
		break;
	}
	for(sx = sroom->lx; sx <= sroom->hx; sx++)
	    for(sy = sroom->ly; sy <= sroom->hy; sy++) {
		if(sroom->irregular) {
		    if ((int) levl[sx][sy].roomno != rmno ||
			  levl[sx][sy].edge ||
			  (sroom->doorct &&
			   distmin(sx, sy, doors[sh].x, doors[sh].y) <= 1))
			continue;
		} else if(!SPACE_POS(levl[sx][sy].typ) ||
			  (sroom->doorct &&
			   ((sx == sroom->lx && doors[sh].x == sx-1) ||
			    (sx == sroom->hx && doors[sh].x == sx+1) ||
			    (sy == sroom->ly && doors[sh].y == sy-1) ||
			    (sy == sroom->hy && doors[sh].y == sy+1))))
		    continue;
		/* don't place monster on explicitly placed throne */
		if(type == COURT && IS_THRONE(levl[sx][sy].typ))
		    continue;
		if(!(Role_if(PM_NOBLEMAN) && In_quest(&u.uz) )){
		mon = makemon(
		    (type == COURT) ? courtmon() :
		    (type == BARRACKS) ? squadmon() :
		    (type == MORGUE) ? morguemon() :
		    (type == BEEHIVE) ?
			(sx == tx && sy == ty ? &mons[PM_QUEEN_BEE] :
			 &mons[PM_KILLER_BEE]) :
		    (type == LEPREHALL) ? &mons[PM_LEPRECHAUN] :
		    (type == COCKNEST) ? &mons[PM_COCKATRICE] :
		    (type == ANTHOLE) ? antholemon() :
		    (struct permonst *) 0,
		   sx, sy, NO_MM_FLAGS|MM_NOCOUNTBIRTH);//did add MM_NOCOUNTBIRTH.
		if(mon) {
			mon->msleeping = 1;
			if (type==COURT && mon->mpeaceful) {
				mon->mpeaceful = 0;
				set_malign(mon);
			}
		}
		}
		switch(type) {
		    case ZOO:
		    case LEPREHALL:
			if(sroom->doorct)
			{
			    int distval = dist2(sx,sy,doors[sh].x,doors[sh].y);
			    i = sq(distval);
			}
			else
			    i = goldlim;
			if(i >= goldlim) i = 5*level_difficulty();
			goldlim -= i;
			(void) mkgold((long) rn1(i, 10), sx, sy);
			break;
		    case MORGUE:
			if(!(Role_if(PM_NOBLEMAN) && In_quest(&u.uz) )){
			if(!rn2(5))
			    (void) mk_tt_object(CORPSE, sx, sy);
			if(!rn2(10))	/* lots of treasure buried with dead */
			    (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST,
					     sx, sy, TRUE, FALSE);
			if (!rn2(5))
			    make_grave(sx, sy, (char *)0);
			} else if(rn2(2)) make_grave(sx, sy, (char *)0);
			break;
		    case BEEHIVE:
			if(!rn2(3))
			    (void) mksobj_at(LUMP_OF_ROYAL_JELLY,
					     sx, sy, TRUE, FALSE);
			break;
		    case BARRACKS:
			if(!rn2(20))	/* the payroll and some loot */
			    (void) mksobj_at((rn2(3)) ? LARGE_BOX : CHEST,
					     sx, sy, TRUE, FALSE);
			break;
		    case COCKNEST:
			if(!rn2(3)) {
			    struct obj *sobj = mk_tt_object(STATUE, sx, sy);

			    if (sobj) {
				for (i = rn2(5); i; i--)
				    (void) add_to_container(sobj,
						mkobj(RANDOM_CLASS, FALSE));
				sobj->owt = weight(sobj);
			    }
			}
			break;
		    case ANTHOLE:
			if(!rn2(3))
			    (void) mkobj_at(FOOD_CLASS, sx, sy, FALSE);
			break;
		}
	    }
	switch (type) {
	      case COURT:
		{
		  struct obj *chest;
		  levl[tx][ty].typ = THRONE;
		  (void) somexy(sroom, &mm);
		  (void) mkgold((long) rn1(50 * level_difficulty(),10), mm.x, mm.y);
		  /* the royal coffers */
		  chest = mksobj_at(CHEST, mm.x, mm.y, TRUE, FALSE);
		  chest->spe = 2; /* so it can be found later */
		  level.flags.has_court = 1;
		  break;
		}
	      case BARRACKS:
		  level.flags.has_barracks = 1;
		  break;
	      case ZOO:
		  level.flags.has_zoo = 1;
		  break;
	      case MORGUE:
		  level.flags.has_morgue = 1;
		  break;
	      case SWAMP:
		  level.flags.has_swamp = 1;
		  break;
	      case BEEHIVE:
		  level.flags.has_beehive = 1;
		  break;
	}
}

/* make a swarm of undead around mm */
void
mkundead(mm, revive_corpses, mm_flags)
coord *mm;
boolean revive_corpses;
int mm_flags;
{
	int cnt = (level_difficulty() + 1)/10 + rnd(5);
	struct permonst *mdat;
	struct obj *otmp;
	coord cc;

	while (cnt--) {
	    mdat = morguemon();
	    if (enexto(&cc, mm->x, mm->y, mdat) &&
		    (!revive_corpses ||
		     !(otmp = sobj_at(CORPSE, cc.x, cc.y)) ||
		     !revive(otmp)))
		(void) makemon(mdat, cc.x, cc.y, mm_flags);
	}
	level.flags.graveyard = TRUE;	/* reduced chance for undead corpse */
}

STATIC_OVL struct permonst *
morguemon()
{
	register int i = rn2(100), hd = rn2(level_difficulty());

	if(hd > 10 && i < 10)
		return((Inhell || In_endgame(&u.uz)) ? mkclass(S_DEMON,G_NOHELL|G_HELL) :
							   mkclass(S_MUMMY, Inhell ? G_HELL : G_NOHELL));
//						       &mons[ndemon(A_NONE)]); Was this, but I think this was nullpointering
	if(hd > 8 && i > 85)
		return(mkclass(S_VAMPIRE, Inhell ? G_HELL : G_NOHELL));

	return((i < 20) ? &mons[PM_GHOST]
			: (i < 40) ? &mons[PM_WRAITH] : mkclass(S_ZOMBIE, Inhell ? G_HELL : G_NOHELL));
}

STATIC_OVL struct permonst *
antholemon()
{
	int mtyp;

	/* Same monsters within a level, different ones between levels */
	switch ((level_difficulty() + ((long)u.ubirthday)) % 3) {
	default:	mtyp = PM_GIANT_ANT; break;
	case 0:		mtyp = PM_SOLDIER_ANT; break;
	case 1:		mtyp = PM_FIRE_ANT; break;
	}
	return ((mvitals[mtyp].mvflags & G_GONE) ?
			(struct permonst *)0 : &mons[mtyp]);
}

/** Create a special room with trees, fountains and nymphs.
 * @author Pasi Kallinen
 */
STATIC_OVL void
mkgarden(croom)
struct mkroom *croom; /* NULL == choose random room */
{
    register int tryct = 0;
    boolean maderoom = FALSE;
    coord pos;
    register int i, tried;

    while ((tryct++ < 25) && !maderoom) {
	register struct mkroom *sroom = croom ? croom : &rooms[rn2(nroom)];
	
	if (sroom->hx < 0 || (!croom && (sroom->rtype != OROOM ||
	    !sroom->rlit || has_upstairs(sroom) || has_dnstairs(sroom))))
	    	continue;

	sroom->rtype = GARDEN;
	maderoom = TRUE;
	level.flags.has_garden = 1;

	tried = 0;
	if(rn2(2)){
		i = 1;
		while ((tried++ < 50) && (i > 0) && somexy(sroom, &pos)) {
			struct permonst *pmon;
			if (!MON_AT(pos.x, pos.y)) {
			struct monst *mtmp = makemon(&mons[PM_WEEPING_ANGEL], pos.x,pos.y, NO_MM_FLAGS);
			if (mtmp) mtmp->msleeping = 1;
			i--;
			}
		}
	}
	tried = 0;
	i = rnd(4);
	while ((tried++ < 50) && (i > 0) && somexy(sroom, &pos)) {
	    struct permonst *pmon;
	    if (!MON_AT(pos.x, pos.y) && (pmon = mkclass(S_NYMPH,G_NOHELL))) {
		struct monst *mtmp = makemon(pmon, pos.x,pos.y, NO_MM_FLAGS);
		if (mtmp) mtmp->msleeping = 1;
		i--;
	    }
	}
	tried = 0;
	i = rn1(5,3);
	while ((tried++ < 50) && (i > 0) && somexy(sroom, &pos)) {
	    struct permonst *pmon;
	    if (!MON_AT(pos.x, pos.y) && (pmon = mkclass(S_PLANT,G_HELL|G_NOHELL))) {
		struct monst *mtmp = makemon(pmon, pos.x,pos.y, NO_MM_FLAGS);
		if (mtmp) mtmp->msleeping = 1;
		i--;
	    }
	}
	tried = 0;
	i = rn1(3,3);
	while ((tried++ < 50) && (i > 0) && somexy(sroom, &pos)) {
	    if (levl[pos.x][pos.y].typ == ROOM && !MON_AT(pos.x,pos.y) &&
		!nexttodoor(pos.x,pos.y)) {
		if (rn2(3))
		  levl[pos.x][pos.y].typ = TREE;
		else {
		    levl[pos.x][pos.y].typ = FOUNTAIN;
		    level.flags.nfountains++;
		}
		i--;
	    }
	}
    }
}

STATIC_OVL void
mklibrary(croom)
struct mkroom *croom; /* NULL == choose random room */
{
    register int tryct = 0;
    boolean maderoom = FALSE;
    coord pos;
    register int i, tried;

    while ((tryct++ < 25) && !maderoom) {
	register struct mkroom *sroom = croom ? croom : &rooms[rn2(nroom)];
	
	if (sroom->hx < 0 || (!croom && (sroom->rtype != OROOM)))
	    	continue;

	sroom->rtype = LIBRARY;
	maderoom = TRUE;
	level.flags.has_library = 1;
	
	tried = 0;
	i = rnd(4);
	while ((tried++ < 50) && (i > 0) && somexy(sroom, &pos)) {
	    struct permonst *pmon;
	    if (!MON_AT(pos.x, pos.y)) {
			struct monst *mtmp = makemon(&mons[PM_LIVING_LECTURN], pos.x,pos.y, NO_MM_FLAGS);
			if (mtmp) mtmp->msleeping = 1;
			i--;
	    }
	}
	tried = 0;
	i = rn1(3,3);
	while ((tried++ < 50) && (i > 0) && somexy(sroom, &pos)) {
	    if (levl[pos.x][pos.y].typ == ROOM && !MON_AT(pos.x,pos.y) &&
		!nexttodoor(pos.x,pos.y)) {
		if (rn2(3))
		  levl[pos.x][pos.y].typ = POOL;
		else {
		    levl[pos.x][pos.y].typ = FOUNTAIN;
		    level.flags.nfountains++;
		}
		i--;
	    }
	}
	for(pos.x=sroom->lx-1; pos.x <= sroom->hx+1; pos.x++){
		for(pos.y=sroom->ly-1; pos.y <= sroom->hy+1; pos.y++){
			if(levl[pos.x][pos.y].typ >= STONE && levl[pos.x][pos.y].typ <= DBWALL){
				if(rn2(6)) mksobj_at(SPE_BLANK_PAPER, pos.x, pos.y, TRUE, FALSE);
				else mkobj_at(SPBOOK_CLASS, pos.x, pos.y, FALSE);
			}
		}
	}
	}
}

STATIC_OVL void
mkswamp()	/* Michiel Huisjes & Fred de Wilde */
{
	register struct mkroom *sroom;
	register int sx,sy,i,eelct = 0;

	for(i=0; i<5; i++) {		/* turn up to 5 rooms swampy */
		sroom = &rooms[rn2(nroom)];
		if(sroom->hx < 0 || sroom->rtype != OROOM ||
		   has_upstairs(sroom) || has_dnstairs(sroom))
			continue;

		/* satisfied; make a swamp */
		sroom->rtype = SWAMP;
		for(sx = sroom->lx; sx <= sroom->hx; sx++)
		for(sy = sroom->ly; sy <= sroom->hy; sy++)
		if(!OBJ_AT(sx, sy) &&
		   !MON_AT(sx, sy) && !t_at(sx,sy) && !nexttodoor(sx,sy)) {
		    if((sx+sy)%2) {
			levl[sx][sy].typ = POOL;
			if(!eelct || !rn2(4)) {
			    /* mkclass() won't do, as we might get kraken */
			    (void) makemon(rn2(5) ? &mons[PM_GIANT_EEL]
						  : rn2(2) ? &mons[PM_PIRANHA]
						  : &mons[PM_ELECTRIC_EEL],
						sx, sy, NO_MM_FLAGS);
			    eelct++;
			}
		    } else
			if(!rn2(4))	/* swamps tend to be moldy */
			    (void) makemon(mkclass(S_FUNGUS, Inhell ? G_HELL : G_NOHELL),
						sx, sy, NO_MM_FLAGS);
			else if(!rn2(6))
			    if (!rn2(2)) /* swamp ferns like swamps */
				(void) makemon(&mons[PM_SWAMP_FERN],
						sx, sy, NO_MM_FLAGS);
		}
		level.flags.has_swamp = 1;
	}
}

STATIC_OVL void
mkriver()	/* John Harris */
{
	register int center, width, prog, fill, edge;
	register int x, y;
	level.flags.has_river = 1;
	if (!rn2(4)) {      /* Horizontal river */
		center = rn2(ROWNO-12)+6;
		width = rn2(4)+4;
		for (prog = 1; prog<COLNO; prog++) {
			edge = TRUE;
			for (fill=center-(width/2); fill<=center+(width/2) ; fill++) {
				/* edge is true the first time through this loop and the last */
				liquify(prog, fill, edge);
				edge = (fill == (center+(width/2)-1));
			}
			if (!rn2(3)) {
				if (!rn2(2) && width >4) {width--;}
				else if (width <7) {width++;}
			}
			if (!rn2(3)) {
				if (!rn2(2) && (center-width/2) >1) {center--;}
				else if ((center+width/2) < ROWNO-1) {center++;}
			}
			/* Make sure river doesn't stray off map */
			if (center < 4) {center = 4;}
			if (center > (ROWNO-5)) {center = ROWNO-5;}
		}
	}
	else {      /* Vertical river */
		center = rn2(COLNO-14)+7;
		width = rn2(4)+5;
		for (prog = 1; prog<ROWNO; prog++) {
			edge = TRUE;
			for (fill=center-(width/2); fill<=center+(width/2) ; fill++) {
				liquify(fill, prog, edge);
				edge = (fill == (center+(width/2)-1));
			}
			if (!rn2(3)) {
				if (!rn2(2) && width >5) {width--;}
				else if (width <8) {width++;}
			}
			if (!rn2(3)) {
				if (!rn2(2) && (center-width/2) >1) {center--;}
				else if ((center+width/2) < ROWNO-1) {center++;}
			}
			/* Sanity checking */
			if (center < 5) {center = 5;}
			if (center > (COLNO-6)) {center = COLNO-6;}
		}
	}
}

/* This isn't currently used anywhere. It liquifies the whole level. */
STATIC_OVL void
mksea()	/* John Harris */
{
	register int x, y;
	/*level.flags.has_river = 1;*/
	for (x=1 ; x <= COLNO-1 ; x++) {
		for (y=1 ; y <= ROWNO-1 ; y++) {
			liquify(x,y, FALSE);
		};
	}
}

STATIC_OVL void
liquify(x, y, edge)
register xchar x, y;
register boolean edge; /* Allows room walls to intrude slightly into river. */
{
	register int typ = levl[x][y].typ;
	register int monster = PM_JELLYFISH;
	/* Don't liquify shop walls */
	if (level.flags.has_shop && *in_rooms(x, y, SHOPBASE)) {return;}
	if (typ == STONE || (IS_WALL(typ) && !edge && rn2(3)))
		{levl[x][y].typ = POOL;}
	else if ((typ == SCORR || typ == CORR || IS_DOOR(typ)
					|| typ == SDOOR) && !IS_WALL(typ)) {
			levl[x][y].typ = ROOM;
	}
	/* Leave boulders scattered around, dislodged by erosion.
		Also, because they are fun to push into water.  Plunk! */
	if (levl[x][y].typ == ROOM && !rn2(13))
		(void) mksobj_at(BOULDER, x, y, TRUE, FALSE);
	/* Sea monsters */
	if (levl[x][y].typ == POOL){
		if(!rn2(85-depth(&u.uz))) {
			if (depth(&u.uz) > 19 && !rn2(3)) {monster = PM_ELECTRIC_EEL;}
				else if (depth(&u.uz) > 15 && !rn2(3)) {monster = PM_GIANT_EEL;}
				else if (depth(&u.uz) > 11 && !rn2(2)) {monster = PM_SHARK;}
				else if (depth(&u.uz) > 7 && rn2(4)) {monster = PM_PIRANHA;}
			(void) makemon(&mons[monster], x, y, NO_MM_FLAGS);
		}
		if(!rn2(140-depth(&u.uz))){
			mkobj_at(RANDOM_CLASS, x, y, FALSE);
		}
		else if(!rn2(100-depth(&u.uz))){
		  (void) mkgold((long) rn1(10 * level_difficulty(),10), x, y);
		}
	}
	/* Underground rivers are relatively open spaces, so light them. */
	levl[x][y].lit = 1;
}

STATIC_OVL coord *
shrine_pos(roomno)
int roomno;
{
	static coord buf;
	struct mkroom *troom = &rooms[roomno - ROOMOFFSET];

	buf.x = troom->lx + ((troom->hx - troom->lx) / 2);
	buf.y = troom->ly + ((troom->hy - troom->ly) / 2);
	return(&buf);
}

STATIC_OVL void
mkisland() /* John Harris, modified from mktemple & mkshop,
				with ideas and aid from Pasi Kallinen.*/
{
	register struct mkroom *sroom;
	register struct rm *lev;
	register int x, y, dif, ptype, pxwidth, pywidth;
	register int txoff, tyoff, tspot, tdx, tdy;
	register int mx, my, montype;
	register struct obj *otmp;
	register struct obj *ogold;
	register int u_depth = depth(&u.uz);
	/*An island room has a row of water or lava along the inside of the
			wall of the room.  Because this can block progress, it has to be
			done carefully....
		In case someone wants to do something more with this, here's the
			rationale behind the choices I've made:
		Stairs inside the moat could prevent the player from leaving the level
			or room, so that should always be forbidden.
		A one-door room ensures this room isn't a necessary juncture between
			staircases.
		Deeper in the dungeon (past the mines, certainly) it becomes less
			likely that the player will be held back by a single room, so beyond
			level five the restrictions are relaxed a bit.
		In case the player teleports into the room or falls into it from above,
			we always generate a trap door inside it so the player can escape.
		Specifics of room placement & types:
		If we're on or before level four, island rooms can only be made
			in the same kinds of places as shops.  This prevents the player
			from being utterly blocked by the island, but it also makes them
			quite rare in that area.
		If we're at level five or below we're after the Mines, so a pick-axe
			is almost guarenteed, so we can place islands in more places.
		Starting with level six, if the room is big enough we may make some
			extra water.
		From level twelve down, we may use lava instead of water.
		Note: this code depends on the room being rectangular.
	*/
	for(sroom = &rooms[0]; ; sroom++){
		if(sroom->hx < 0) return;  /* from mkshop: Signifies out of rooms? */
		if(sroom - rooms >= nroom) {
			pline("rooms not closed by -1?");
			return;
		}

		if(sroom->rtype != OROOM || !isspacious(sroom) ||
			has_dnstairs(sroom) || has_upstairs(sroom))
				continue;
		if( sroom->doorct == 1 || (u_depth > 4 && sroom->doorct != 0)) break;
	}

	level.flags.has_island = 1;

	if (!rn2(3) && u_depth > 11) {ptype = LAVAPOOL;}
	else {ptype = POOL;};

	if ( u_depth > 5 ) {
		pxwidth = ((sroom->hx - sroom-> lx) > 6) ? 2 : 1;
		pywidth = ((sroom->hy - sroom-> ly) > 6) ? 2 : 1;
	}
	else {pxwidth = pywidth = 1;}
	/* Reveal secret doors and liquify borders */
	for(x = sroom->lx; x <= sroom->hx; x++) {
		if (levl[x][sroom->hy+1].typ == SDOOR)
			{levl[x][sroom->hy+1].typ = DOOR;}
		if (levl[x][sroom->ly-1].typ == SDOOR)
			{levl[x][sroom->ly-1].typ = DOOR;}
		levl[x][sroom->hy].typ = ptype;
		levl[x][sroom->ly].typ = ptype;
		if (pywidth == 2) {
			levl[x][sroom->hy-1].typ = ptype;
			levl[x][sroom->ly+1].typ = ptype;
		}
	}
	for(y = (sroom->ly); y <= (sroom->hy); y++) {
		if (levl[sroom->hx+1][y].typ == SDOOR)
			{levl[sroom->hx+1][y].typ = DOOR;}
		if (levl[sroom->lx-1][y].typ == SDOOR)
			{levl[sroom->lx-1][y].typ = DOOR;}
		levl[sroom->hx][y].typ = ptype;
		levl[sroom->lx][y].typ = ptype;
		if (pxwidth == 2) {
			levl[sroom->hx-1][y].typ = ptype;
			levl[sroom->lx+1][y].typ = ptype;
		}
	}
	/* Find the center of the room */
	x = (((sroom->hx - sroom->lx) / 2) + sroom->lx);
	y = (((sroom->hy - sroom->ly) / 2) + sroom->ly);
	/* Make the treasure, add gold to it, bury & mark it */
	otmp = mksobj_at(CHEST, x, y, TRUE, TRUE);
	ogold = mkgold((long)rn1(u_depth * 100 + 200, 250), x, y);
	remove_object(ogold);
	(void) add_to_container(otmp, ogold);
	bury_an_obj(otmp);
	make_engr_at(x,y,"X",0,ENGRAVE);

	/* Put a tree next to the spot
		(all desert islands have exactly one palm tree on them)*/
	tspot = rn2(8);
	if (tspot < 3) {tyoff = -1;}
	else if (tspot > 3 && tspot != 7) {tyoff = 1;}
	else tyoff = 0;
	if (tspot == 0 || tspot > 5) {txoff = -1;}
	else if (tspot > 1 && tspot < 5) {txoff = 1;}
	else txoff = 0;
	levl[x+txoff][y+tyoff].typ = TREE;
	/* Add a trap door in case the player gets stuck here. */
	do {
		tdx = rn2(sroom->hx - sroom->lx + 1 - pxwidth * 2) + sroom->lx + pxwidth;
		tdy = rn2(sroom->hy - sroom->ly + 1 - pywidth * 2) + sroom->ly + pywidth;
	} while (((tdx == x)&&(tdy == y)) ||
				((tdx == x + txoff) && (tdy == y + tyoff)));

	maketrap(tdx, tdy, TRAPDOOR);
	/* If level 9 or deeper, and the moat is water, maybe make a sea monster.
		(Eels earlier in the dungeon might be too hard and/or produce
		too many experience points.) */
	if (u_depth > 8 && ptype == POOL) {
		/* Make in the corners. */
		if (rn2(2)) {
			mx = (rn2(2)) ? sroom->hx : sroom->lx;
			my = (rn2(2)) ? sroom->hy : sroom->ly;
			(void) makemon(rn2(2) ? &mons[PM_GIANT_EEL]
				: &mons[PM_ELECTRIC_EEL], mx, my, NO_MM_FLAGS);
		}
	}
	level.flags.has_island = TRUE;
}

STATIC_OVL void
mktemple()
{
	register struct mkroom *sroom;
	coord *shrine_spot;
	register struct rm *lev;

	if(!(sroom = pick_room(TRUE))) return;

	/* set up Priest and shrine */
	sroom->rtype = TEMPLE;
	/*
	 * In temples, shrines are blessed altars
	 * located in the center of the room
	 */
	shrine_spot = shrine_pos((sroom - rooms) + ROOMOFFSET);
	lev = &levl[shrine_spot->x][shrine_spot->y];
	lev->typ = ALTAR;
	lev->altarmask = induced_align(80);
	priestini(&u.uz, sroom, shrine_spot->x, shrine_spot->y, FALSE);
	lev->altarmask |= AM_SHRINE;
	level.flags.has_temple = 1;
}

boolean
nexttodoor(sx,sy)
register int sx, sy;
{
	register int dx, dy;
	register struct rm *lev;
	for(dx = -1; dx <= 1; dx++) for(dy = -1; dy <= 1; dy++) {
		if(!isok(sx+dx, sy+dy)) continue;
		if(IS_DOOR((lev = &levl[sx+dx][sy+dy])->typ) ||
		    lev->typ == SDOOR)
			return(TRUE);
	}
	return(FALSE);
}

boolean
has_dnstairs(sroom)
register struct mkroom *sroom;
{
	if (sroom == dnstairs_room)
		return TRUE;
	if (sstairs.sx && !sstairs.up)
		return((boolean)(sroom == sstairs_room));
	return FALSE;
}

boolean
has_upstairs(sroom)
register struct mkroom *sroom;
{
	if (sroom == upstairs_room)
		return TRUE;
	if (sstairs.sx && sstairs.up)
		return((boolean)(sroom == sstairs_room));
	return FALSE;
}

#endif /* OVLB */
#ifdef OVL0

int
somex(croom)
register struct mkroom *croom;
{
	return rn2(croom->hx-croom->lx+1) + croom->lx;
}

int
somey(croom)
register struct mkroom *croom;
{
	return rn2(croom->hy-croom->ly+1) + croom->ly;
}

boolean
inside_room(croom, x, y)
struct mkroom *croom;
xchar x, y;
{
	return((boolean)(x >= croom->lx-1 && x <= croom->hx+1 &&
		y >= croom->ly-1 && y <= croom->hy+1));
}

boolean
somexy(croom, c)
struct mkroom *croom;
coord *c;
{
	int try_cnt = 0;
	int i;

	if (croom->irregular) {
	    i = (croom - rooms) + ROOMOFFSET;

	    while(try_cnt++ < 100) {
		c->x = somex(croom);
		c->y = somey(croom);
		if (!levl[c->x][c->y].edge &&
			(int) levl[c->x][c->y].roomno == i)
		    return TRUE;
	    }
	    /* try harder; exhaustively search until one is found */
	    for(c->x = croom->lx; c->x <= croom->hx; c->x++)
		for(c->y = croom->ly; c->y <= croom->hy; c->y++)
		    if (!levl[c->x][c->y].edge &&
			    (int) levl[c->x][c->y].roomno == i)
			return TRUE;
	    return FALSE;
	}

	if (!croom->nsubrooms) {
		c->x = somex(croom);
		c->y = somey(croom);
		return TRUE;
	}

	/* Check that coords doesn't fall into a subroom or into a wall */

	while(try_cnt++ < 100) {
		c->x = somex(croom);
		c->y = somey(croom);
		if (IS_WALL(levl[c->x][c->y].typ))
		    continue;
		for(i=0 ; i<croom->nsubrooms;i++)
		    if(inside_room(croom->sbrooms[i], c->x, c->y))
			goto you_lose;
		break;
you_lose:	;
	}
	if (try_cnt >= 100)
	    return FALSE;
	return TRUE;
}

/*
 * Search for a special room given its type (zoo, court, etc...)
 *	Special values :
 *		- ANY_SHOP
 *		- ANY_TYPE
 */

struct mkroom *
search_special(type)
schar type;
{
	register struct mkroom *croom;

	for(croom = &rooms[0]; croom->hx >= 0; croom++)
	    if((type == ANY_TYPE && croom->rtype != OROOM) ||
	       (type == ANY_SHOP && croom->rtype >= SHOPBASE) ||
	       croom->rtype == type)
		return croom;
	for(croom = &subrooms[0]; croom->hx >= 0; croom++)
	    if((type == ANY_TYPE && croom->rtype != OROOM) ||
	       (type == ANY_SHOP && croom->rtype >= SHOPBASE) ||
	       croom->rtype == type)
		return croom;
	return (struct mkroom *) 0;
}

#endif /* OVL0 */
#ifdef OVLB

struct permonst *
courtmon()
{
	int     i = rn2(60) + rn2(3*level_difficulty());
	if (i > 100)		return(mkclass(S_DRAGON, Inhell ? G_HELL : G_NOHELL));
	else if (i > 95)	return(mkclass(S_GIANT, Inhell ? G_HELL : G_NOHELL));
	else if (i > 85)	return(mkclass(S_TROLL, Inhell ? G_HELL : G_NOHELL));
	else if (i > 75)	return(mkclass(S_CENTAUR, Inhell ? G_HELL : G_NOHELL));
	else if (i > 60)	return(mkclass(S_ORC, Inhell ? G_HELL : G_NOHELL));
	else if (i > 45)	return(&mons[PM_BUGBEAR]);
	else if (i > 30)	return(&mons[PM_HOBGOBLIN]);
	else if (i > 15)	return(mkclass(S_GNOME, Inhell ? G_HELL : G_NOHELL));
	else			return(mkclass(S_KOBOLD, Inhell ? G_HELL : G_NOHELL));
}

#define NSTYPES (PM_CAPTAIN - PM_SOLDIER + 1)

static struct {
    unsigned	pm;
    unsigned	prob;
} squadprob[NSTYPES] = {
    {PM_SOLDIER, 80}, {PM_SERGEANT, 15}, {PM_LIEUTENANT, 4}, {PM_CAPTAIN, 1}
};

STATIC_OVL struct permonst *
squadmon()		/* return soldier types. */
{
	int sel_prob, i, cpro, mndx;

	sel_prob = rnd(80+level_difficulty());

	cpro = 0;
	for (i = 0; i < NSTYPES; i++) {
	    cpro += squadprob[i].prob;
	    if (cpro > sel_prob) {
		mndx = squadprob[i].pm;
		goto gotone;
	    }
	}
	mndx = squadprob[rn2(NSTYPES)].pm;
gotone:
//	if (!(mvitals[mndx].mvflags & G_GONE)) return(&mons[mndx]);
	if (!(mvitals[mndx].mvflags & G_GENOD)) return(&mons[mndx]);//empty if genocided
	else			    return((struct permonst *) 0);
}

/*
 * save_room : A recursive function that saves a room and its subrooms
 * (if any).
 */

STATIC_OVL void
save_room(fd, r)
int	fd;
struct mkroom *r;
{
	short i;
	/*
	 * Well, I really should write only useful information instead
	 * of writing the whole structure. That is I should not write
	 * the subrooms pointers, but who cares ?
	 */
	bwrite(fd, (genericptr_t) r, sizeof(struct mkroom));
	for(i=0; i<r->nsubrooms; i++)
	    save_room(fd, r->sbrooms[i]);
}

/*
 * save_rooms : Save all the rooms on disk!
 */

void
save_rooms(fd)
int fd;
{
	short i;

	/* First, write the number of rooms */
	bwrite(fd, (genericptr_t) &nroom, sizeof(nroom));
	for(i=0; i<nroom; i++)
	    save_room(fd, &rooms[i]);
}

STATIC_OVL void
rest_room(fd, r)
int fd;
struct mkroom *r;
{
	short i;

	mread(fd, (genericptr_t) r, sizeof(struct mkroom));
	for(i=0; i<r->nsubrooms; i++) {
		r->sbrooms[i] = &subrooms[nsubroom];
		rest_room(fd, &subrooms[nsubroom]);
		subrooms[nsubroom++].resident = (struct monst *)0;
	}
}

/*
 * rest_rooms : That's for restoring rooms. Read the rooms structure from
 * the disk.
 */

void
rest_rooms(fd)
int	fd;
{
	short i;

	mread(fd, (genericptr_t) &nroom, sizeof(nroom));
	nsubroom = 0;
	for(i = 0; i<nroom; i++) {
	    rest_room(fd, &rooms[i]);
	    rooms[i].resident = (struct monst *)0;
	}
	rooms[nroom].hx = -1;		/* restore ending flags */
	subrooms[nsubroom].hx = -1;
}
#endif /* OVLB */

/*mkroom.c*/
