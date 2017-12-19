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

extern const int monstr[];

#ifdef OVLB
STATIC_DCL boolean FDECL(isbig, (struct mkroom *));
STATIC_DCL boolean FDECL(isspacious, (struct mkroom *));
STATIC_DCL void NDECL(mkshop), FDECL(mkzoo,(int)), NDECL(mkswamp);
STATIC_DCL void NDECL(mktemple);
STATIC_DCL void NDECL(mkkamereltowers);
STATIC_DCL void NDECL(mkminorspire);
STATIC_DCL void NDECL(mkpluhomestead);
STATIC_DCL void NDECL(mkpluvillage);
STATIC_DCL void NDECL(mklolthsepulcher);
STATIC_DCL void NDECL(mkmivaultlolth);
STATIC_DCL void NDECL(mkvaultlolth);
STATIC_DCL void NDECL(mklolthgnoll);
STATIC_DCL void NDECL(mklolthgarden);
STATIC_DCL void NDECL(mklolthtroll);
STATIC_DCL void NDECL(mklolthdown);
STATIC_DCL void NDECL(mklolthup);
STATIC_DCL void FDECL(mkgarden, (struct mkroom *));
STATIC_DCL void FDECL(mklibrary, (struct mkroom *));
STATIC_DCL void FDECL(mkarmory, (struct mkroom *));
STATIC_DCL void NDECL(mkisland);
STATIC_DCL void NDECL(mkriver);
STATIC_DCL void NDECL(mkneuriver);
STATIC_DCL void FDECL(liquify, (xchar, xchar, boolean));
STATIC_DCL void FDECL(neuliquify, (xchar, xchar, boolean));
STATIC_DCL struct permonst * NDECL(morguemon);
STATIC_DCL struct permonst * NDECL(antholemon);
STATIC_DCL struct permonst * NDECL(squadmon);
STATIC_DCL struct permonst * NDECL(neu_squadmon);
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
		for(i=-1;i<2;i++) for(j=-1;j<2;j++) if(!isok(x+i,y+j) || levl[x+i][y+j].typ != STONE) okspot = FALSE;
		for(i=-2;i<3;i++) for(j=-2;j<3;j++) if(!isok(x+i,y+j) || levl[x+i][y+j].typ >= CORR) okspot = FALSE;
		if(okspot){
			good = TRUE;
			//Blank the inner area, to be safe
			for(i=-1;i<2;i++) for(j=-1;j<2;j++) levl[x+i][y+j].typ = STONE;
			//Make outer area walled
			levl[x+2][y+2].typ = BRCORNER;
			levl[x-2][y+2].typ = BLCORNER;
			for(i=-1;i<2;i++) levl[x+i][y+2].typ = HWALL;
			for(i=-1;i<2;i++) levl[x+i][y-2].typ = HWALL;
			for(i=-1;i<2;i++) levl[x+2][y+i].typ = VWALL;
			for(i=-1;i<2;i++) levl[x-2][y+i].typ = VWALL;
			levl[x+2][y-2].typ = TRCORNER;
			levl[x-2][y-2].typ = TLCORNER;
			//Make the inner area walled also
			levl[x+1][y+1].typ = BRCORNER;
			levl[x-1][y+1].typ = BLCORNER;
			levl[x+0][y+1].typ = HWALL;
			levl[x+0][y-1].typ = HWALL;
			levl[x+1][y+0].typ = VWALL;
			levl[x-1][y+0].typ = VWALL;
			levl[x+1][y-1].typ = TRCORNER;
			levl[x-1][y-1].typ = TLCORNER;
			//Make the center spot empty
			levl[x][y].typ = CORR;
			for(i=-2;i<3;i++) {
				for(j=-2;j<3;j++) {
					levl[x+i][y+j].wall_info |= W_NONDIGGABLE;
				}
			}
			for(i=-1;i<2;i++) {
				for(j=-1;j<2;j++) {
					levl[x+i][y+j].lit = 0;
					levl[x+i][y+j].wall_info |= W_NONDIGGABLE;
					levl[x+i][y+j].wall_info |= W_NONPASSWALL;
				}
			}
			makemon(&mons[PM_DREAD_SERAPH], x, y, 0);
		}
	}
}

STATIC_OVL
void
mklolthsepulcher()
{
	int x,y,tries=0;
	int i,j;
	boolean good=FALSE,okspot;
	while(!good && tries < 50){
		x = rn2(COLNO-7)+3;
		y = rn2(ROWNO-7)+3;
		tries++;
		okspot = TRUE;
		for(i=-2;i<3;i++) for(j=-2;j<3;j++) if(!isok(x+i,y+j) || !(levl[x+i][y+j].typ == ROOM || levl[x+i][y+j].typ == CLOUD)) okspot = FALSE;
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
			
			levl[x+1][y+1].typ = BRCORNER;
			levl[x-1][y+1].typ = BLCORNER;
			levl[x+0][y+1].typ = HWALL;
			levl[x+0][y-1].typ = HWALL;
			levl[x+1][y+0].typ = VWALL;
			levl[x-1][y+0].typ = VWALL;
			levl[x+1][y-1].typ = TRCORNER;
			levl[x-1][y-1].typ = TLCORNER;
			
			levl[x][y].typ = CORR;
			for(i=-2;i<3;i++) {
				for(j=-2;j<3;j++) {
					levl[x+i][y+j].lit = 0;
					levl[x+i][y+j].wall_info |= W_NONDIGGABLE;
				}
			}
			makemon(&mons[PM_DREAD_SERAPH], x, y, 0);
		}
	}
}

void
mkmivault()
{
	int x,y,tries=0;
	int i,j;
	struct obj *otmp;
	struct monst *mon;
	boolean good=FALSE,okspot;
	while(!good && tries < 50){
		x = rn2(COLNO-6)+1;
		y = rn2(ROWNO-6)+1;
		tries++;
		okspot = TRUE;
		for(i=0;i<6;i++) for(j=0;j<6;j++) if(!isok(x+i,y+j) || levl[x+i][y+j].typ != STONE) okspot = FALSE;
		if(okspot){
			good = TRUE;
			for(i=0;i<6;i++) for(j=0;j<6;j++) levl[x+i][y+j].typ = MOAT;
			for(i=0;i<6;i++){
				levl[x][y+i].edge = 1;
				levl[x+i][y].edge = 1;
				levl[x+5][y+i].edge = 1;
				levl[x+i][y+5].edge = 1;
			}
			for(i=2;i<4;i++) for(j=2;j<4;j++) levl[x+i][y+j].typ = ROOM;
			levl[x+4][y+4].typ = BRCORNER;
			levl[x+1][y+4].typ = BLCORNER;
			for(i=2;i<4;i++) levl[x+i][y+4].typ = HWALL;
			for(i=2;i<4;i++) levl[x+i][y+1].typ = HWALL;
			for(i=2;i<4;i++) levl[x+4][y+i].typ = VWALL;
			for(i=2;i<4;i++) levl[x+1][y+i].typ = VWALL;
			levl[x+4][y+1].typ = TRCORNER;
			levl[x+1][y+1].typ = TLCORNER;
			
			
			otmp = mksobj_at(CHEST, x+2, y+2, TRUE, FALSE);
			for(i = d(2,4);i>0;i--) mkmivaultitem(otmp);
			otmp = mksobj_at(CHEST, x+2, y+3, TRUE, FALSE);
			for(i = d(2,4);i>0;i--) mkmivaultitem(otmp);
			otmp = mksobj_at(CHEST, x+3, y+2, TRUE, FALSE);
			for(i = d(2,4);i>0;i--) mkmivaultitem(otmp);
			otmp = mksobj_at(CHEST, x+3, y+3, TRUE, FALSE);
			for(i = d(2,4);i>0;i--) mkmivaultitem(otmp);
			
			mon = makemon(mivaultmon(), x+rnd(2)+1, y+rnd(2)+1, 0);
			mon->mstrategy |= STRAT_WAITFORU;
			mon->mpeaceful = FALSE;
			set_malign(mon);
		}
	}
}

void
mkmivaultitem(container)
    struct obj *container;
{
	struct obj *otmp;
	int try_limit = 100;
	otmp = (struct obj *)0;
	if(!rn2(4)){
		do {
		if (otmp) delobj(otmp);
		otmp = mkobj(ARMOR_CLASS, FALSE);
		} while (--try_limit > 0 &&
		  !(objects[otmp->otyp].oc_magic || otmp->oartifact));
		add_to_container(container, otmp);
	} else {
		do {
			if(otmp && !Is_container(otmp)) delobj(otmp);
			otmp = mkobj(RANDOM_CLASS, TRUE);
			if(Is_container(otmp)){
				place_object(otmp, container->ox, container->oy);
			}
		} while (--try_limit > 0 &&
		  !(objects[otmp->otyp].oc_magic || otmp->oartifact));
		if(!Is_container(otmp)) add_to_container(container, otmp);
	}
}

STATIC_OVL
void
mkmivaultlolth()
{
	int x,y,tries=0;
	int i,j;
	struct obj *otmp;
	struct monst *mon;
	boolean good=FALSE,okspot;
	while(!good && tries < 50){
		x = rn2(COLNO-4)+1;
		y = rn2(ROWNO-4)+1;
		tries++;
		okspot = TRUE;
		for(i=0;i<4;i++) for(j=0;j<4;j++) if(!isok(x+i,y+j) || !(levl[x+i][y+j].typ == ROOM || levl[x+i][y+j].typ == CLOUD)) okspot = FALSE;
		if(okspot){
			good = TRUE;
			for(i=0;i<4;i++) for(j=0;j<4;j++) levl[x+i][y+j].typ = CORR;
			for(i=0;i<4;i++){
				levl[x][y+i].edge = 1;
				levl[x+i][y].edge = 1;
				levl[x+3][y+i].edge = 1;
				levl[x+i][y+3].edge = 1;
			}
			levl[x+3][y+3].typ = BRCORNER;
			levl[x][y+3].typ = BLCORNER;
			for(i=1;i<3;i++) levl[x+i][y+3].typ = HWALL;
			for(i=1;i<3;i++) levl[x+i][y].typ = HWALL;
			for(i=1;i<3;i++) levl[x+3][y+i].typ = VWALL;
			for(i=1;i<3;i++) levl[x][y+i].typ = VWALL;
			levl[x+3][y].typ = TRCORNER;
			levl[x][y].typ = TLCORNER;
			
			
			otmp = mksobj_at(CHEST, x+1, y+1, TRUE, FALSE);
			for(i = d(2,4);i>0;i--) mkmivaultitem(otmp);
			otmp = mksobj_at(CHEST, x+1, y+2, TRUE, FALSE);
			for(i = d(2,4);i>0;i--) mkmivaultitem(otmp);
			otmp = mksobj_at(CHEST, x+2, y+1, TRUE, FALSE);
			for(i = d(2,4);i>0;i--) mkmivaultitem(otmp);
			otmp = mksobj_at(CHEST, x+2, y+2, TRUE, FALSE);
			for(i = d(2,4);i>0;i--) mkmivaultitem(otmp);
			
			mon = makemon(mivaultmon(), x+rnd(2), y+rnd(2), 0);
			mon->mstrategy |= STRAT_WAITFORU;
			mon->mpeaceful = FALSE;
			set_malign(mon);
		}
	}
}

STATIC_OVL
void
mkvaultlolth()
{
	int x,y,tries=0;
	int i,j;
	struct obj *otmp;
	struct monst *mon;
	boolean good=FALSE,okspot;
	while(!good && tries < 50){
		x = rn2(COLNO-4)+1;
		y = rn2(ROWNO-4)+1;
		tries++;
		okspot = TRUE;
		for(i=0;i<4;i++) for(j=0;j<4;j++) if(!isok(x+i,y+j) || !(levl[x+i][y+j].typ == ROOM || levl[x+i][y+j].typ == CLOUD)) okspot = FALSE;
		if(okspot){
			good = TRUE;
			for(i=0;i<4;i++) for(j=0;j<4;j++) levl[x+i][y+j].typ = CORR;
			for(i=0;i<4;i++){
				levl[x][y+i].edge = 1;
				levl[x+i][y].edge = 1;
				levl[x+3][y+i].edge = 1;
				levl[x+i][y+3].edge = 1;
			}
			levl[x+3][y+3].typ = BRCORNER;
			levl[x][y+3].typ = BLCORNER;
			for(i=1;i<3;i++) levl[x+i][y+3].typ = HWALL;
			for(i=1;i<3;i++) levl[x+i][y].typ = HWALL;
			for(i=1;i<3;i++) levl[x+3][y+i].typ = VWALL;
			for(i=1;i<3;i++) levl[x][y+i].typ = VWALL;
			levl[x+3][y].typ = TRCORNER;
			levl[x][y].typ = TLCORNER;
			
			mkgold((long) rn1((10+rnd(10)) * level_difficulty(),10), x+1, y+1);
			mkgold((long) rn1((10+rnd(10)) * level_difficulty(),10), x+2, y+1);
			mkgold((long) rn1((10+rnd(10)) * level_difficulty(),10), x+1, y+2);
			mkgold((long) rn1((10+rnd(10)) * level_difficulty(),10), x+2, y+2);
			
			switch(rn2(3)){
				case 0:
					makemon(&mons[PM_SKELETON], x+rnd(2), y+rnd(2), 0);
				break;
				case 1:
					makemon(&mons[PM_DROW_MUMMY], x+rnd(2), y+rnd(2), 0);
				break;
				case 2:
					makemon(&mons[PM_HEDROW_ZOMBIE], x+rnd(2), y+rnd(2), 0);
					if(rn2(4)) makemon(&mons[PM_HEDROW_ZOMBIE], x+rnd(2), y+rnd(2), 0);
					if(!rn2(3)) makemon(&mons[PM_HEDROW_ZOMBIE], x+rnd(2), y+rnd(2), 0);
				break;
			}
		}
	}
}

STATIC_OVL
void
mklolthgnoll()
{
	int x,y,tries=0;
	int i,j, rmnumb = nroom+ROOMOFFSET, trycount, madedoor;
	struct obj *otmp;
	struct monst *mon;
	boolean good=FALSE,okspot;
	while(!good && tries < 50){
		x = rn2(COLNO-10)+1;
		y = rn2(ROWNO-10)+1;
		tries++;
		okspot = TRUE;
		for(i=0;i<10;i++) for(j=0;j<10;j++) if(!isok(x+i,y+j) || !(levl[x+i][y+j].typ == ROOM || levl[x+i][y+j].typ == CLOUD)) okspot = FALSE;
		if(okspot){
			good = TRUE;
			for(i=0;i<10;i++) for(j=0;j<10;j++){
				levl[x+i][y+j].typ = CORR;
				levl[x+i][y+j].roomno = rmnumb;
			}
			for(i=0;i<10;i++){
				levl[x][y+i].edge = 1;
				levl[x+i][y].edge = 1;
				levl[x+9][y+i].edge = 1;
				levl[x+i][y+9].edge = 1;
			}
			levl[x+9][y+9].typ = BRCORNER;
			levl[x][y+9].typ = BLCORNER;
			for(i=1;i<9;i++) levl[x+i][y+9].typ = HWALL;
			for(i=1;i<9;i++) levl[x+i][y].typ = HWALL;
			for(i=1;i<9;i++) levl[x+9][y+i].typ = VWALL;
			for(i=1;i<9;i++) levl[x][y+i].typ = VWALL;
			levl[x+9][y].typ = TRCORNER;
			levl[x][y].typ = TLCORNER;
			
			rooms[nroom].lx = x + 1;
			rooms[nroom].hx = x + 8;
			rooms[nroom].ly = y + 1;
			rooms[nroom].hy = y + 8;
			rooms[nroom].rtype = BARRACKS;
			rooms[nroom].doorct = 0;
			rooms[nroom].fdoor = 0;
			rooms[nroom].nsubrooms = 0;
			rooms[nroom].irregular = FALSE;
			nroom++;
			
			for(i = 2; i < 8; i++) for(j = 2; j < 8; j++){
				if(rn2(10)){
					mon = makemon(&mons[PM_GNOLL], x+i, y+j, 0);
					if(mon) mon->msleeping = 1;
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(8);
				if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
					madedoor++;
					levl[x+i][y].typ = DOOR;
					levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= 8 && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(8);
				if(isok(x+i,y+10) && levl[x+i][y+10].typ == ROOM){
					madedoor++;
					levl[x+i][y+9].typ = DOOR;
					levl[x+i][y+9].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= 8 && madedoor < 2; i++){
					if(isok(x+i,y+10) && levl[x+i][y+10].typ == ROOM){
						madedoor++;
						levl[x+i][y+9].typ = DOOR;
						levl[x+i][y+9].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(8);
				if(isok(x-1,y+i) && levl[x-1][y+i].typ == ROOM){
					madedoor++;
					levl[x][y+i].typ = DOOR;
					levl[x][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= 8 && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(8);
				if(isok(x+10,y+i) && levl[x+10][y+i].typ == ROOM){
					madedoor++;
					levl[x+9][y+i].typ = DOOR;
					levl[x+9][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= 8 && madedoor < 2; i++){
					if(isok(x+10,y+i) && levl[x+10][y+i].typ == ROOM){
						madedoor++;
						levl[x+9][y+i].typ = DOOR;
						levl[x+9][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
		}
	}
}


STATIC_OVL
void
mklolthgarden()
{
	int x,y,tries=0, width= rn1(4,5), height=rn1(4,5);
	int i,j, rmnumb = nroom+ROOMOFFSET, trycount, madedoor;
	struct obj *otmp;
	struct monst *mon;
	boolean good=FALSE,okspot;
	while(!good && tries < 50){
		x = rn2(COLNO-width)+1;
		y = rn2(ROWNO-height)+1;
		tries++;
		okspot = TRUE;
		for(i=0;i<width;i++) for(j=0;j<height;j++) if(!isok(x+i,y+j) || !(levl[x+i][y+j].typ == ROOM || levl[x+i][y+j].typ == CLOUD)) okspot = FALSE;
		if(okspot){
			good = TRUE;
			for(i=0;i<width;i++) for(j=0;j<height;j++){
				levl[x+i][y+j].typ = CORR;
			}
			levl[x+(width-1)][y+(height-1)].typ = BRCORNER;
			levl[x][y+(height-1)].typ = BLCORNER;
			for(i=1;i<(width-1);i++) levl[x+i][y+(height-1)].typ = HWALL;
			for(i=1;i<(width-1);i++) levl[x+i][y].typ = HWALL;
			for(i=1;i<(height-1);i++) levl[x+(width-1)][y+i].typ = VWALL;
			for(i=1;i<(height-1);i++) levl[x][y+i].typ = VWALL;
			levl[x+(width-1)][y].typ = TRCORNER;
			levl[x][y].typ = TLCORNER;
			
			rooms[nroom].lx = x + 1;
			rooms[nroom].hx = x + width - 2;
			rooms[nroom].ly = y + 1;
			rooms[nroom].hy = y + height - 2;
			rooms[nroom].rtype = GARDEN;
			rooms[nroom].doorct = 0;
			rooms[nroom].fdoor = 0;
			rooms[nroom].nsubrooms = 0;
			rooms[nroom].irregular = FALSE;
			mkgarden(&rooms[nroom]);
			nroom++;
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(width-2);
				if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
					madedoor++;
					levl[x+i][y].typ = DOOR;
					levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (width-2) && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(width-2);
				if(isok(x+i,y+height) && levl[x+i][y+height].typ == ROOM){
					madedoor++;
					levl[x+i][y+height-1].typ = DOOR;
					levl[x+i][y+height-1].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (width-2) && madedoor < 2; i++){
					if(isok(x+i,y+height) && levl[x+i][y+height].typ == ROOM){
						madedoor++;
						levl[x+i][y+height-1].typ = DOOR;
						levl[x+i][y+height-1].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(height-2);
				if(isok(x-1,y+i) && levl[x-1][y+i].typ == ROOM){
					madedoor++;
					levl[x][y+i].typ = DOOR;
					levl[x][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (height-2) && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(height-2);
				if(isok(x+width,y+i) && levl[x+width][y+i].typ == ROOM){
					madedoor++;
					levl[x+width-1][y+i].typ = DOOR;
					levl[x+width-1][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (height-2) && madedoor < 2; i++){
					if(isok(x+width,y+i) && levl[x+width][y+i].typ == ROOM){
						madedoor++;
						levl[x+width-1][y+i].typ = DOOR;
						levl[x+width-1][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
		}
	}
}


STATIC_OVL
void
mklolthtroll()
{
	int x,y,tries=0, width= 6, height=6;
	int i,j, rmnumb = nroom+ROOMOFFSET, trycount, madedoor;
	struct obj *otmp;
	struct monst *mon;
	boolean good=FALSE,okspot;
	while(!good && tries < 50){
		x = rn2(COLNO-width)+1;
		y = rn2(ROWNO-height)+1;
		tries++;
		okspot = TRUE;
		for(i=0;i<width;i++) for(j=0;j<height;j++) if(!isok(x+i,y+j) || !(levl[x+i][y+j].typ == ROOM || levl[x+i][y+j].typ == CLOUD)) okspot = FALSE;
		if(okspot){
			good = TRUE;
			for(i=0;i<width;i++) for(j=0;j<height;j++){
				levl[x+i][y+j].typ = CORR;
			}
			levl[x+(width-1)][y+(height-1)].typ = BRCORNER;
			levl[x][y+(height-1)].typ = BLCORNER;
			for(i=1;i<(width-1);i++) levl[x+i][y+(height-1)].typ = HWALL;
			for(i=1;i<(width-1);i++) levl[x+i][y].typ = HWALL;
			for(i=1;i<(height-1);i++) levl[x+(width-1)][y+i].typ = VWALL;
			for(i=1;i<(height-1);i++) levl[x][y+i].typ = VWALL;
			levl[x+(width-1)][y].typ = TRCORNER;
			levl[x][y].typ = TLCORNER;
			
			for(i = 0; i < 10; i++) makemon(mkclass(S_TROLL, G_HELL|G_NOHELL), x+3, y+3, MM_ADJACENTOK);
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(width-2);
				if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
					madedoor++;
					levl[x+i][y].typ = DOOR;
					levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (width-2) && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(width-2);
				if(isok(x+i,y+height) && levl[x+i][y+height].typ == ROOM){
					madedoor++;
					levl[x+i][y+height-1].typ = DOOR;
					levl[x+i][y+height-1].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (width-2) && madedoor < 2; i++){
					if(isok(x+i,y+height) && levl[x+i][y+height].typ == ROOM){
						madedoor++;
						levl[x+i][y+height-1].typ = DOOR;
						levl[x+i][y+height-1].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(height-2);
				if(isok(x-1,y+i) && levl[x-1][y+i].typ == ROOM){
					madedoor++;
					levl[x][y+i].typ = DOOR;
					levl[x][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (height-2) && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(height-2);
				if(isok(x+width,y+i) && levl[x+width][y+i].typ == ROOM){
					madedoor++;
					levl[x+width-1][y+i].typ = DOOR;
					levl[x+width-1][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (height-2) && madedoor < 2; i++){
					if(isok(x+width,y+i) && levl[x+width][y+i].typ == ROOM){
						madedoor++;
						levl[x+width-1][y+i].typ = DOOR;
						levl[x+width-1][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
		}
	}
}


STATIC_OVL
void
mklolthtreasure()
{
	int x,y,tries=0, width= 5, height=5;
	int i,j, rmnumb = nroom+ROOMOFFSET, trycount, madedoor;
	struct obj *otmp, *container;
	struct monst *mon;
	boolean good=FALSE,okspot;
	while(!good && tries < 50){
		x = rn2(COLNO-width)+1;
		y = rn2(ROWNO-height)+1;
		tries++;
		okspot = TRUE;
		for(i=0;i<width;i++) for(j=0;j<height;j++) if(!isok(x+i,y+j) || !(levl[x+i][y+j].typ == ROOM || levl[x+i][y+j].typ == CLOUD)) okspot = FALSE;
		if(okspot){
			good = TRUE;
			for(i=0;i<width;i++) for(j=0;j<height;j++){
				levl[x+i][y+j].typ = CORR;
			}
			levl[x+(width-1)][y+(height-1)].typ = BRCORNER;
			levl[x][y+(height-1)].typ = BLCORNER;
			for(i=1;i<(width-1);i++) levl[x+i][y+(height-1)].typ = HWALL;
			for(i=1;i<(width-1);i++) levl[x+i][y].typ = HWALL;
			for(i=1;i<(height-1);i++) levl[x+(width-1)][y+i].typ = VWALL;
			for(i=1;i<(height-1);i++) levl[x][y+i].typ = VWALL;
			levl[x+(width-1)][y].typ = TRCORNER;
			levl[x][y].typ = TLCORNER;
			
			makemon(&mons[PM_HEDROW_WIZARD], x+2, y+2, MM_ADJACENTOK);
			makemon(&mons[PM_DROW_MATRON], x+2, y+2, MM_ADJACENTOK);
			makemon(&mons[PM_DROW_MATRON], x+2, y+2, MM_ADJACENTOK);
			makemon(&mons[PM_DROW_MATRON], x+2, y+2, MM_ADJACENTOK);
			makemon(&mons[PM_DROW_MATRON], x+2, y+2, MM_ADJACENTOK);
			
			container = mksobj_at(CHEST, x+2, y+2, TRUE, FALSE);
			container->olocked = 1;
			container->otrapped = 1;
			otmp = mksobj(POT_FULL_HEALING, TRUE, FALSE);
			(void) add_to_container(container, otmp);
			otmp = mksobj(POT_FULL_HEALING, TRUE, FALSE);
			(void) add_to_container(container, otmp);
			for(i = d(2,8); i > 0; i--){
				otmp = mksobj(rn2(LAST_GEM-DILITHIUM_CRYSTAL)+DILITHIUM_CRYSTAL, TRUE, FALSE);
				(void) add_to_container(container, otmp);
			}
			for(i = d(2,4); i > 0; i--){
				otmp = mkgold((long)rn1(depth(&u.uz) * 20 + 100, 175), x+2, y+2);
				remove_object(otmp);
				(void) add_to_container(container, otmp);
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(width-2);
				if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
					madedoor++;
					levl[x+i][y].typ = DOOR;
					levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (width-2) && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(width-2);
				if(isok(x+i,y+height) && levl[x+i][y+height].typ == ROOM){
					madedoor++;
					levl[x+i][y+height-1].typ = DOOR;
					levl[x+i][y+height-1].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (width-2) && madedoor < 2; i++){
					if(isok(x+i,y+height) && levl[x+i][y+height].typ == ROOM){
						madedoor++;
						levl[x+i][y+height-1].typ = DOOR;
						levl[x+i][y+height-1].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(height-2);
				if(isok(x-1,y+i) && levl[x-1][y+i].typ == ROOM){
					madedoor++;
					levl[x][y+i].typ = DOOR;
					levl[x][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (height-2) && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(height-2);
				if(isok(x+width,y+i) && levl[x+width][y+i].typ == ROOM){
					madedoor++;
					levl[x+width-1][y+i].typ = DOOR;
					levl[x+width-1][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (height-2) && madedoor < 2; i++){
					if(isok(x+width,y+i) && levl[x+width][y+i].typ == ROOM){
						madedoor++;
						levl[x+width-1][y+i].typ = DOOR;
						levl[x+width-1][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
		}
	}
}


STATIC_OVL
void
mklolthup()
{
	int x,y,tries=0,madedoor,trycount;
	int i,j, width=5, height=5;
	struct obj *otmp;
	struct monst *mon;
	boolean good=FALSE,okspot;
	while(!good){
		x = rn2(COLNO-width)+1;
		y = rn2(ROWNO-height)+1;
		tries++;
		okspot = TRUE;
		for(i=0;i<width;i++) for(j=0;j<height;j++) if(!isok(x+i,y+j) || !(levl[x+i][y+j].typ == ROOM || levl[x+i][y+j].typ == CLOUD)) okspot = FALSE;
		if(okspot){
			good = TRUE;
			for(i=0;i<width;i++) for(j=0;j<height;j++){
				levl[x+i][y+j].typ = CORR;
			}
			levl[x+(width-1)][y+(height-1)].typ = BRCORNER;
			levl[x][y+(height-1)].typ = BLCORNER;
			for(i=1;i<(width-1);i++) levl[x+i][y+(height-1)].typ = HWALL;
			for(i=1;i<(width-1);i++) levl[x+i][y].typ = HWALL;
			for(i=1;i<(height-1);i++) levl[x+(width-1)][y+i].typ = VWALL;
			for(i=1;i<(height-1);i++) levl[x][y+i].typ = VWALL;
			levl[x+(width-1)][y].typ = TRCORNER;
			levl[x][y].typ = TLCORNER;
			
			xupstair = x+2;
			yupstair = y+2;
			
			levl[x+2][y+2].typ = STAIRS;
			levl[x+2][y+2].ladder = LA_UP;
			
			makemon(&mons[PM_VROCK], x+1, y+1, NO_MM_FLAGS);
			makemon(&mons[PM_VROCK], x+1, y+3, NO_MM_FLAGS);
			makemon(&mons[PM_VROCK], x+3, y+3, NO_MM_FLAGS);
			makemon(&mons[PM_VROCK], x+3, y+1, NO_MM_FLAGS);
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(width-2);
				if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
					madedoor++;
					levl[x+i][y].typ = DOOR;
					levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (width-2) && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(width-2);
				if(isok(x+i,y+height) && levl[x+i][y+height].typ == ROOM){
					madedoor++;
					levl[x+i][y+height-1].typ = DOOR;
					levl[x+i][y+height-1].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (width-2) && madedoor < 2; i++){
					if(isok(x+i,y+height) && levl[x+i][y+height].typ == ROOM){
						madedoor++;
						levl[x+i][y+height-1].typ = DOOR;
						levl[x+i][y+height-1].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(height-2);
				if(isok(x-1,y+i) && levl[x-1][y+i].typ == ROOM){
					madedoor++;
					levl[x][y+i].typ = DOOR;
					levl[x][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (height-2) && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(height-2);
				if(isok(x+width,y+i) && levl[x+width][y+i].typ == ROOM){
					madedoor++;
					levl[x+width-1][y+i].typ = DOOR;
					levl[x+width-1][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (height-2) && madedoor < 2; i++){
					if(isok(x+width,y+i) && levl[x+width][y+i].typ == ROOM){
						madedoor++;
						levl[x+width-1][y+i].typ = DOOR;
						levl[x+width-1][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
		}
	}
}

STATIC_OVL
void
mklolthdown()
{
	int x,y,tries=0, madedoor, trycount;
	int i,j, width=5, height=5;
	struct obj *otmp;
	struct monst *mon;
	boolean good=FALSE,okspot;
	while(!good){
		x = rn2(COLNO-width)+1;
		y = rn2(ROWNO-height)+1;
		tries++;
		okspot = TRUE;
		for(i=0;i<width;i++) for(j=0;j<height;j++) if(!isok(x+i,y+j) || !(levl[x+i][y+j].typ == ROOM || levl[x+i][y+j].typ == CLOUD)) okspot = FALSE;
		if(okspot){
			good = TRUE;
			for(i=0;i<width;i++) for(j=0;j<height;j++){
				levl[x+i][y+j].typ = CORR;
			}
			levl[x+(width-1)][y+(height-1)].typ = BRCORNER;
			levl[x][y+(height-1)].typ = BLCORNER;
			for(i=1;i<(width-1);i++) levl[x+i][y+(height-1)].typ = HWALL;
			for(i=1;i<(width-1);i++) levl[x+i][y].typ = HWALL;
			for(i=1;i<(height-1);i++) levl[x+(width-1)][y+i].typ = VWALL;
			for(i=1;i<(height-1);i++) levl[x][y+i].typ = VWALL;
			levl[x+(width-1)][y].typ = TRCORNER;
			levl[x][y].typ = TLCORNER;
			
			xdnstair = x+2;
			ydnstair = y+2;

			levl[x+2][y+2].typ = STAIRS;
			levl[x+2][y+2].ladder = LA_DOWN;
			
			makemon(&mons[PM_MARILITH], x+2, y+2, NO_MM_FLAGS);
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(width-2);
				if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
					madedoor++;
					levl[x+i][y].typ = DOOR;
					levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (width-2) && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(width-2);
				if(isok(x+i,y+height) && levl[x+i][y+height].typ == ROOM){
					madedoor++;
					levl[x+i][y+height-1].typ = DOOR;
					levl[x+i][y+height-1].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (width-2) && madedoor < 2; i++){
					if(isok(x+i,y+height) && levl[x+i][y+height].typ == ROOM){
						madedoor++;
						levl[x+i][y+height-1].typ = DOOR;
						levl[x+i][y+height-1].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(height-2);
				if(isok(x-1,y+i) && levl[x-1][y+i].typ == ROOM){
					madedoor++;
					levl[x][y+i].typ = DOOR;
					levl[x][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (height-2) && madedoor < 2; i++){
					if(isok(x+i,y-1) && levl[x+i][y-1].typ == ROOM){
						madedoor++;
						levl[x+i][y].typ = DOOR;
						levl[x+i][y].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
			
			madedoor = 0;
			trycount = 0;
			while(madedoor < 2 && trycount<50){
				i = rnd(height-2);
				if(isok(x+width,y+i) && levl[x+width][y+i].typ == ROOM){
					madedoor++;
					levl[x+width-1][y+i].typ = DOOR;
					levl[x+width-1][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				trycount++;
			}
			if(madedoor == 0){
				for(i = 1; i <= (height-2) && madedoor < 2; i++){
					if(isok(x+width,y+i) && levl[x+width][y+i].typ == ROOM){
						madedoor++;
						levl[x+width-1][y+i].typ = DOOR;
						levl[x+width-1][y+i].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
					}
				}
			}
		}
	}
}

STATIC_OVL
void
mkkamereltowers()
{
	int x=0,y=0,tx, ty, tries=0;
	int i,j, c, edge;
	boolean left = rn2(2), good=FALSE, okspot;
	int slant = rn2(3);
	struct obj *otmp;
	if(left){
		//Make shallow sea
		edge = rn1(20, 20);
		for(j = 0; j < ROWNO; j++){
			for(i = 0; i < edge; i++){
				if(isok(x+i,y+j)){
					if(levl[x+i][y+j].typ!=TREE || edge - i > rn2(6)) levl[x+i][y+j].typ = PUDDLE;
					levl[x+i][y+j].lit = 1;
				}
			}
			if(rn2(4)) edge += rn2(3)-slant;
		}
		//Build central tower
		while(!good && tries < 500){
			x=4+rnd(3)+rn2(3);
			y=6+rn2(10);
			
			tries++;
			okspot = TRUE;
			for(i=-3;i<=3;i++)
				for(j=-3;j<=3;j++){
					if(!isok(x+i,y+j) || levl[x+i][y+j].typ != PUDDLE)
						okspot = FALSE;
				}
			if(okspot){
				good = TRUE;
			} else continue;
			for(i=-3;i<=3;i++)
				for(j=-3;j<=3;j++){
					if(dist2(x+i,y+j,x,y)<=14){
						levl[x+i][y+j].typ = HWALL;
						levl[x+i][y+j].lit = 1;
					}
				}
			for(i=-2;i<=2;i++)
				for(j=-2;j<=2;j++){
					if(dist2(x+i,y+j,x,y)<=5){
						levl[x+i][y+j].typ = CORR;
						levl[x+i][y+j].lit = 0;
					}
				}
			for(i=-3;i<=3;i++)
				for(j=-3;j<=3;j++){
					if(levl[x+i][y+j].typ == HWALL){
						otmp = mksobj(MIRROR, FALSE, FALSE);
						otmp->objsize = MZ_GIGANTIC;
						fix_object(otmp);
						place_object(otmp, x+i, y+j);
					}
				}
			wallification(x-3, y-3, x+3, y+3);
			if(rn2(2)){
				otmp = mksobj(DOUBLE_LIGHTSABER, FALSE, FALSE);
				otmp = oname(otmp, artiname(ART_INFINITY_S_MIRRORED_ARC));
				otmp->spe = 0;
				otmp->cursed = 0;
				otmp->blessed = 0;
				place_object(otmp, x, y);
			} else {
				otmp = mksobj(KHAKKHARA, FALSE, FALSE);
				otmp = oname(otmp, artiname(ART_STAFF_OF_TWELVE_MIRRORS));
				otmp->spe = 0;
				otmp->cursed = 0;
				otmp->blessed = 0;
				place_object(otmp, x, y);
			}
			//record central tower location
			tx = x;
			ty = y;
		}
	} else {
		edge = COLNO - rn1(20, 20);
		for(j = 0; j < ROWNO; j++){
			for(i = COLNO; i > edge; i--){
				if(isok(x+i,y+j)){
					if(levl[x+i][y+j].typ!=TREE || i - edge > rn2(6)) levl[x+i][y+j].typ = PUDDLE;
					levl[x+i][y+j].lit = 1;
				}
			}
			if(rn2(4)) edge += rn2(3)-slant;
		}
		
		//Build central tower
		while(!good && tries < 500){
			x=COLNO - (4+rnd(3)+rn2(3));
			y=6+rn2(10);
			
			tries++;
			okspot = TRUE;
			for(i=-3;i<=3;i++)
				for(j=-3;j<=3;j++){
					if(!isok(x+i,y+j) || levl[x+i][y+j].typ != PUDDLE)
						okspot = FALSE;
				}
			if(okspot){
				good = TRUE;
			} else continue;
			for(i=-3;i<=3;i++)
				for(j=-3;j<=3;j++){
					if(dist2(x+i,y+j,x,y)<=14){
						levl[x+i][y+j].typ = HWALL;
						levl[x+i][y+j].lit = 1;
					}
				}
			for(i=-2;i<=2;i++)
				for(j=-2;j<=2;j++){
					if(dist2(x+i,y+j,x,y)<=5){
						levl[x+i][y+j].typ = CORR;
						levl[x+i][y+j].lit = 0;
					}
				}
			for(i=-3;i<=3;i++)
				for(j=-3;j<=3;j++){
					if(levl[x+i][y+j].typ == HWALL){
						otmp = mksobj(MIRROR, FALSE, FALSE);
						otmp->objsize = MZ_GIGANTIC;
						fix_object(otmp);
						place_object(otmp, x+i, y+j);
					}
				}
			wallification(x-3, y-3, x+3, y+3);
			if(rn2(2)){
				otmp = mksobj(DOUBLE_LIGHTSABER, FALSE, FALSE);
				otmp = oname(otmp, artiname(ART_INFINITY_S_MIRRORED_ARC));
				otmp->spe = 0;
				otmp->cursed = 0;
				otmp->blessed = 0;
				place_object(otmp, x, y);
			} else {
				otmp = mksobj(KHAKKHARA, FALSE, FALSE);
				otmp = oname(otmp, artiname(ART_STAFF_OF_TWELVE_MIRRORS));
				otmp->spe = 0;
				otmp->cursed = 0;
				otmp->blessed = 0;
				place_object(otmp, x, y);
			}
			//record central tower location
			tx = x;
			ty = y;
		}
	}
	
	//Build 3 square towers
	c = 1 + rn2(3);
	while(c > 0){
		good = FALSE;
		tries = 0;
		while(!good && tries < 50){
			x=tx+rn2(17)-8;
			y=ty+rn2(17)-8;
			
			tries++;
			okspot = TRUE;
			for(i=-3;i<=3;i++)
				for(j=-3;j<=3;j++){ //+/-3: walkable perimeter
					if(!isok(x+i,y+j) || levl[x+i][y+j].typ != PUDDLE)
						okspot = FALSE;
				}
			if(okspot){
				good = TRUE;
			} else continue;
			for(i=-2;i<=2;i++)
				for(j=-2;j<=2;j++){
					levl[x+i][y+j].typ = HWALL;
					levl[x+i][y+j].lit = 1;
				}
			for(i=-1;i<=1;i++)
				for(j=-1;j<=1;j++){
					levl[x+i][y+j].typ = CORR;
					levl[x+i][y+j].lit = 0;
				}
			for(i=-2;i<=2;i++)
				for(j=-2;j<=2;j++){
					if(levl[x+i][y+j].typ == HWALL){
						otmp = mksobj(MIRROR, FALSE, FALSE);
						otmp->objsize = MZ_GIGANTIC;
						fix_object(otmp);
						place_object(otmp, x+i, y+j);
					}
				}
			wallification(x-2, y-2, x+2, y+2);
		}
		c--;
	}
	//Build 1 square towers
	c = rnd(3) + rn2(3);
	while(c > 0){
		good = FALSE;
		tries = 0;
		while(!good && tries < 50){
			x=tx+rn2(21)-10;
			y=ty+rn2(21)-10;
			
			tries++;
			okspot = TRUE;
			for(i=-2;i<=2;i++)
				for(j=-2;j<=2;j++){ //+/-2: walkable perimeter
					if(!isok(x+i,y+j) || levl[x+i][y+j].typ != PUDDLE)
						okspot = FALSE;
				}
			if(okspot){
				good = TRUE;
			} else continue;
			for(i=-1;i<=1;i++)
				for(j=-1;j<=1;j++){
					levl[x+i][y+j].typ = HWALL;
					levl[x+i][y+j].lit = 1;
				}
			levl[x][y].typ = CORR;
			levl[x][y].lit = 0;
			for(i=-1;i<=1;i++)
				for(j=-1;j<=1;j++){
					if(levl[x+i][y+j].typ == HWALL){
						otmp = mksobj(MIRROR, FALSE, FALSE);
						otmp->objsize = MZ_GIGANTIC;
						fix_object(otmp);
						place_object(otmp, x+i, y+j);
					}
				}
			wallification(x-1, y-1, x+1, y+1);
		}
		c--;
	}
}

STATIC_OVL
void
mkminorspire()
{
	int x,y,ix, iy, tries=0;
	int i,j, c;
	boolean good=FALSE, okspot;
	struct obj *otmp;
	while(!good && tries < 50){
		x = rn2(COLNO-6)+3;
		y = rn2(ROWNO-5)+2;
		tries++;
		okspot = TRUE;
		for(i=0;i<3;i++)
			for(j=0;j<3;j++){
				if(!isok(x+i,y+j))
					okspot = FALSE;
			}
		if(okspot){
			good = TRUE;
		} else continue;
		
		ix = x;
		iy = y;
		for(i=-10;i<=10;i++){
			for(j=-10;j<=10;j++){
				if(isok(ix+i,iy+j) && dist2(ix, iy, ix+i, iy+j)<105){
					if(levl[ix+i][iy+j].typ == ROOM || (dist2(ix, iy, ix+i, iy+j)) < (rnd(8)*rnd(8)+36))
						levl[ix+i][iy+j].typ = PUDDLE;
					levl[ix+i][iy+j].lit = 1;
				}
			}
		}
		// for(c=rnd(8)+10; c > 0; c--){
			// ix = x+rn2(21)-10;
			// iy = y+rn2(21)-10;
			// for(i=-4;i<=4;i++){
				// for(j=-4;j<=4;j++){
					// if(isok(ix+i,iy+j) && dist2(ix, iy, ix+i, iy+j)<16){
						// levl[ix+i][iy+j].typ = PUDDLE;
						// levl[ix+i][iy+j].lit = 1;
					// }
				// }
			// }
		// }
		for(i=-1;i<=1;i++){
			for(j=-1;j<=1;j++){
				levl[x+i][y+j].typ = HWALL;
				levl[x+i][y+j].lit = 1;
			}
		}
		wallification(x-1, y-1, x+1, y+1);
		levl[x][y].lit = 0;
		
		mksobj_at(ROBE, x, y, TRUE, FALSE);
		switch(rn2(6)){
			case 4:
			    otmp = mksobj(LONG_SWORD, FALSE, FALSE);
				otmp = oname(otmp, artiname(ART_MIRROR_BRAND));
				if(!otmp->oartifact){
					otmp->obj_material = SILVER;
					fix_object(otmp);
					mksobj_at(SHIELD_OF_REFLECTION, x, y, TRUE, FALSE);
				}
				place_object(otmp, x, y);
			break;
			case 3:
			    otmp = mksobj(PLATE_MAIL, FALSE, FALSE);
				otmp = oname(otmp, artiname(ART_SOULMIRROR));
				if(!otmp->oartifact){
					otmp->obj_material = MITHRIL;
					fix_object(otmp);
					mksobj_at(AMULET_OF_REFLECTION, x, y, TRUE, FALSE);
				}
				place_object(otmp, x, y);
				if(find_sawant()){
					otmp = mksobj(find_sawant(), FALSE, FALSE);
					otmp->obj_material = SILVER;
					fix_object(otmp);
					place_object(otmp, x, y);
					
					otmp = mksobj(BUCKLER, FALSE, FALSE);
					otmp->obj_material = MITHRIL;
					fix_object(otmp);
					place_object(otmp, x, y);
				} else {
					mksobj_at(KHAKKHARA, x, y, TRUE, FALSE);
				}
			break;
			case 2:
			case 1:
				if(find_sawant()){
					otmp = mksobj(find_sawant(), FALSE, FALSE);
					otmp->obj_material = SILVER;
					fix_object(otmp);
					place_object(otmp, x, y);
					mksobj_at(SHIELD_OF_REFLECTION, x, y, TRUE, FALSE);
					break;
				}
			default:
				mksobj_at(KHAKKHARA, x, y, TRUE, FALSE);
				mksobj_at(AMULET_OF_REFLECTION, x, y, TRUE, FALSE);
			break;
		}
	}
}

STATIC_OVL
void
mkpluhomestead()
{
	int x,y,tries=0, roomnumb;
	int i,j, pathto = 0;
	boolean good=FALSE, okspot, accessible;
	while(!good && tries < 500){
		x = rn2(COLNO-6)+1;
		y = rn2(ROWNO-5);
		tries++;
		okspot = TRUE;
		accessible = FALSE;
		for(i=0;i<5;i++)
			for(j=0;j<5;j++){
				if(!isok(x+i,y+j) || t_at(x+i, y+j) || !(levl[x+i][y+j].typ == TREE))
					okspot = FALSE;
			}
		pathto = 0;
		if(isok(x+2,y-1) && levl[x+2][y-1].typ == ROOM) pathto++;
		if(isok(x+2,y+5) && levl[x+2][y+5].typ == ROOM) pathto++;
		if(isok(x+5,y+2) && levl[x+5][y+2].typ == ROOM) pathto++;
		if(isok(x-1,y+2) && levl[x-1][y+2].typ == ROOM) pathto++;
		if(pathto) accessible = TRUE;
		if(okspot && accessible){
			good = TRUE;
		} else continue;
		
		for(i=0;i<5;i++){
			for(j=0;j<5;j++){
				levl[x+i][y+j].typ = HWALL;
				if(m_at(x+i, y+j)) rloc(m_at(x+i, y+j), TRUE);
			}
		}
		for(i=1;i<4;i++){
			for(j=1;j<4;j++){
				levl[x+i][y+j].typ = CORR;
				if(rn2(9)) mkobj_at((rn2(2) ? WEAPON_CLASS : rn2(2) ? TOOL_CLASS : ARMOR_CLASS), x+i, y+j, FALSE);
			}
		}
		i = rnd(3)+rn2(2);
		for(i;i>0;i--){
			makemon(&mons[PM_PLUMACH_RILMANI], x+rnd(3), y+rnd(3), MM_ADJACENTOK);
		}
		
		wallification(x, y, x+4, y+4);
		
		pathto = rn2(pathto);
		if(isok(x+2,y-1) && levl[x+2][y-1].typ == ROOM && !(pathto--))
			levl[x+2][y+0].typ = DOOR, levl[x+2][y+0].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
		if(isok(x+2,y+5) && levl[x+2][y+5].typ == ROOM && !(pathto--))
			levl[x+2][y+4].typ = DOOR, levl[x+2][y+4].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
		if(isok(x+5,y+2) && levl[x+5][y+2].typ == ROOM && !(pathto--))
			levl[x+4][y+2].typ = DOOR, levl[x+4][y+2].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
		if(isok(x-1,y+2) && levl[x-1][y+2].typ == ROOM && !(pathto--))
			levl[x+0][y+2].typ = DOOR, levl[x+0][y+2].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
	}
}

STATIC_OVL
void
mkpluvillage()
{
	int x,y,tries=0, roomnumb;
	int i,j, n,ni;
	int nshacks, sizebig1, sizebig2, sizetot;
	struct obj *otmp;
	struct monst *mon;
	boolean good=FALSE, okspot, accessible, throne = 0;
	struct mkroom *croom;
	while(!good && tries < 50){
		nshacks = rnd(3) + rn2(3);
		// nshacks = rnd(5);
		if(rn2(2)){
			sizebig1 = 1+rnd(3)+2;
			sizebig2 = 2+rnd(3)+2;
		} else {
			sizebig1 = 2+rnd(3)+2;
			sizebig2 = 1+rnd(3)+2;
		}
		sizetot = sizebig1 + nshacks*5 + sizebig2 + 1;
		x = rn2(COLNO-sizetot)+1;
		y = rn2(ROWNO-11);
		tries++;
		okspot = TRUE;
		accessible = FALSE;
		for(i=0;i<sizetot+1;i++)
			for(j=0;j<12;j++){
				if(!isok(x+i,y+j) || t_at(x+i, y+j) || !(levl[x+i][y+j].typ == ROOM || levl[x+i][y+j].typ == TREE))
					okspot = FALSE;
				if(isok(x+i,y+j) && levl[x+i][y+j].typ == ROOM)
					accessible = TRUE;
			}
		if(okspot && accessible){
			good = TRUE;
			//Clear village green
			for(i=sizebig1;i<(sizetot-sizebig2);i++) 
				for(j=1;j<10;j++){
					levl[x+i][y+j].typ = ROOM;
					levl[x+i][y+j].lit = 1;
				}
			
			//Make left-hand big building
			roomnumb = nroom;
			
			levl[x+sizebig1-1][y+4+3].typ = BRCORNER;
			levl[x+sizebig1-1][y+4+3].lit = 1;
			levl[x][y+4+3].typ = BLCORNER;
			levl[x][y+4+3].lit = 1;
			for(i=1;i<sizebig1-1;i++){
				levl[x+i][y+4+3].typ = HWALL;
				levl[x+i][y+4+3].lit = 1;
				levl[x+i][y+3].typ = HWALL;
				levl[x+i][y+3].lit = 1;
			}
			for(i=1+3;i<4+3;i++){
				levl[x+sizebig1-1][y+i].typ = VWALL;
				levl[x+sizebig1-1][y+i].lit = 1;
				levl[x][y+i].typ = VWALL;
				levl[x][y+i].lit = 1;
			}
			levl[x+sizebig1-1][y+5].typ = DOOR;
			levl[x+sizebig1-1][y+5].doormask = rn2(3) ? D_CLOSED : D_LOCKED;

			levl[x+sizebig1-1][y+3].typ = TRCORNER;
			levl[x+sizebig1-1][y+3].lit = 1;
			levl[x][y+3].typ = TLCORNER;
			levl[x][y+3].lit = 1;
			
			switch(rn2(7)){
				case 0: //Random store
					for(i=1;i<sizebig1-1;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = ROOM;
							levl[x+i][y+j].lit = 1;
						}
					}
					flood_fill_rm(x+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					add_room(x+1, y+4, x+sizebig1-2, y+6, TRUE, SHOPBASE + rn2(UNIQUESHOP - SHOPBASE), TRUE);
				break;
				case 1: //Temple
					for(i=1;i<sizebig1-1;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = ROOM;
							levl[x+i][y+j].lit = 1;
						}
					}
					flood_fill_rm(x+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					levl[x+1][y+5].typ = ALTAR;
					levl[x+1][y+5].altarmask = Align2amask( A_NEUTRAL );
					add_room(x+1, y+4, x+sizebig1-2, y+6, TRUE, TEMPLE, TRUE);
					priestini(&u.uz, &rooms[nroom - 1], x+1, y+5, FALSE);
					levl[x+sizetot-2][y+5].altarmask |= AM_SHRINE;
					level.flags.has_temple = 1;
				break;
				case 2: //Garrison
					for(i=1;i<sizebig1-1;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = ROOM;
							levl[x+i][y+j].lit = 1;
						}
					}
					flood_fill_rm(x+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					add_room(x+1, y+4, x+sizebig1-2, y+6, TRUE, BARRACKS, TRUE);
				break;
				case 3: //Courtroom
					for(i=1;i<sizebig1-1;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = ROOM;
							levl[x+i][y+j].lit = 1;
						}
					}
					flood_fill_rm(x+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					throne = 1;
					add_room(x+1, y+4, x+sizebig1-2, y+6, TRUE, COURT, TRUE);
				break;
				case 4://Normal
					for(i=1;i<sizebig1-1;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = CORR;
							levl[x+i][y+j].lit = 1;
							if(rn2(2)) makemon(&mons[PM_PLUMACH_RILMANI], x+i, y+j, 0);
							if(rn2(2)) mkobj_at((rn2(2) ? WEAPON_CLASS : rn2(2) ? TOOL_CLASS : ARMOR_CLASS), x+i, y+j, FALSE);
						}
					}
					flood_fill_rm(x+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					add_room(x+1, y+4, x+sizebig1-2, y+6, TRUE, OROOM, TRUE);
				break;
				case 5: //Gold vault
					for(i=1;i<sizebig1-1;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = CORR;
							levl[x+i][y+j].lit = 1;
							if(rn2(2)) makemon(&mons[PM_GOLD_GOLEM], x+i, y+j, 0);
							mkgold((long) rn1((10+rnd(10)) * level_difficulty(),10), x+i, y+j);
						}
					}
					flood_fill_rm(x+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					add_room(x+1, y+4, x+sizebig1-2, y+6, TRUE, OROOM, TRUE);
				break;
				case 6://Tool shed
					for(i=1;i<sizebig1-1;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = CORR;
							levl[x+i][y+j].lit = 1;
							mkobj_at(TOOL_CLASS, x+i, y+j, FALSE);
						}
					}
					flood_fill_rm(x+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					add_room(x+1, y+4, x+sizebig1-2, y+6, TRUE, OROOM, TRUE);
				break;
			}
			add_door(x+sizebig1-1,y+5,&rooms[roomnumb]);
			fill_room(&rooms[roomnumb], FALSE);
			
			//Make north and south shacks
			for(n = 0; n<nshacks; n++){
				ni = sizebig1 + 1 + n*5;
				//make north shack
				levl[x+ni+3][y+3].typ = BRCORNER;
				levl[x+ni][y+3].typ = BLCORNER;
				for(i=ni+1;i<ni+3;i++){
					levl[x+i][y+3].typ = HWALL;
					levl[x+i][y].typ = HWALL;
				}
				if(rn2(2)){
					levl[x+ni+2][y+3].typ = DOOR;
					levl[x+ni+2][y+3].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				} else {
					levl[x+ni+1][y+3].typ = DOOR;
					levl[x+ni+1][y+3].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				for(i=ni+1;i<ni+3;i++){
					for(j=1;j<3;j++){
						levl[x+i][y+j].typ = CORR;
						levl[x+i][y+j].lit = 1;
						if(rn2(2)) makemon(&mons[PM_PLUMACH_RILMANI], x+i, y+j, 0);
						if(rn2(2)) mkobj_at((rn2(2) ? WEAPON_CLASS : rn2(2) ? TOOL_CLASS : ARMOR_CLASS), x+i, y+j, FALSE);
					}
				}
				for(i=1;i<3;i++){
					levl[x+ni+3][y+i].typ = VWALL;
					levl[x+ni][y+i].typ = VWALL;
				}
				levl[x+ni+3][y].typ = TRCORNER;
				levl[x+ni][y].typ = TLCORNER;
				
				//make south shack
				levl[x+ni+3][y+7+3].typ = BRCORNER;
				levl[x+ni][y+7+3].typ = BLCORNER;
				for(i=ni+1;i<ni+3;i++){
					levl[x+i][y+7+3].typ = HWALL;
					levl[x+i][y+7].typ = HWALL;
				}
				if(rn2(2)){
					levl[x+ni+2][y+7].typ = DOOR;
					levl[x+ni+2][y+7].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				} else {
					levl[x+ni+1][y+7].typ = DOOR;
					levl[x+ni+1][y+7].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
				}
				for(i=ni+1;i<ni+3;i++){
					for(j=1;j<3;j++){
						levl[x+i][y+7+j].typ = CORR;
						levl[x+i][y+j].lit = 1;
						if(rn2(2)) makemon(&mons[PM_PLUMACH_RILMANI], x+i, y+7+j, 0);
						if(rn2(2)) mkobj_at((rn2(2) ? WEAPON_CLASS : rn2(2) ? TOOL_CLASS : ARMOR_CLASS), x+i, y+7+j, FALSE);
					}
				}
				for(i=1;i<3;i++){
					levl[x+ni+3][y+7+i].typ = VWALL;
					levl[x+ni][y+7+i].typ = VWALL;
				}
				levl[x+ni+3][y+7].typ = TRCORNER;
				levl[x+ni][y+7].typ = TLCORNER;
			}
			
			//Make right big building
			roomnumb = nroom;
			
			levl[x+sizetot][y+4+3].typ = BRCORNER;
			levl[x+sizetot][y+4+3].lit = 1;
			levl[x+sizetot-sizebig2][y+4+3].typ = BLCORNER;
			levl[x+sizetot-sizebig2][y+4+3].lit = 1;
			for(i=sizetot-sizebig2+1;i<sizetot;i++){
				levl[x+i][y+4+3].typ = HWALL;
				levl[x+i][y+4+3].lit = 1;
				levl[x+i][y+3].typ = HWALL;
				levl[x+i][y+3].lit = 1;
			}
			for(i=1+3;i<4+3;i++){
				levl[x+sizetot][y+i].typ = VWALL;
				levl[x+sizetot][y+i].lit = 1;
				levl[x+sizetot-sizebig2][y+i].typ = VWALL;
				levl[x+sizetot-sizebig2][y+i].lit = 1;
			}
			
			levl[x+sizetot-sizebig2][y+5].typ = DOOR;
			levl[x+sizetot-sizebig2][y+5].doormask = rn2(3) ? D_CLOSED : D_LOCKED;
			
			levl[x+sizetot][y+3].typ = TRCORNER;
			levl[x+sizetot][y+3].lit = 1;
			levl[x+sizetot-sizebig2][y+3].typ = TLCORNER;
			levl[x+sizetot-sizebig2][y+3].lit = 1;		
			
			// switch(rn2(6)){
			switch(rn2(7)){
				case 0: //Shop
					for(i=sizetot-sizebig2+1;i<sizetot;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = ROOM;
							levl[x+i][y+j].lit = 1;
						}
					}
					flood_fill_rm(x+sizetot-1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					add_room(x+sizetot-sizebig2+1, y+4, x+sizetot-1, y+6, TRUE, SHOPBASE + rn2(UNIQUESHOP - SHOPBASE), TRUE);
				break;
				case 1: //Temple
					for(i=sizetot-sizebig2+1;i<sizetot;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = ROOM;
							levl[x+i][y+j].lit = 1;
						}
					}
					flood_fill_rm(x+sizetot-sizebig2+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					levl[x+sizetot-2][y+5].typ = ALTAR;
					levl[x+sizetot-2][y+5].altarmask = Align2amask( A_NEUTRAL );
					add_room(x+sizetot-sizebig2+1, y+4, x+sizetot-1, y+6, TRUE, TEMPLE, TRUE);
					priestini(&u.uz, &rooms[nroom - 1], x+sizetot-2, y+5, FALSE);
					levl[x+sizetot-2][y+5].altarmask |= AM_SHRINE;
					level.flags.has_temple = 1;
				break;
				case 2: //Garrison
					for(i=sizetot-sizebig2+1;i<sizetot;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = ROOM;
							levl[x+i][y+j].lit = 1;
						}
					}
					flood_fill_rm(x+sizetot-sizebig2+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					add_room(x+sizetot-sizebig2+1, y+4, x+sizetot-1, y+6, TRUE, BARRACKS, TRUE);
				break;
				case 3: //Courtroom
					if(!throne){
						for(i=sizetot-sizebig2+1;i<sizetot;i++){
							for(j=1+3;j<4+3;j++){
								levl[x+i][y+j].typ = ROOM;
								levl[x+i][y+j].lit = 1;
							}
						}
						flood_fill_rm(x+sizetot-sizebig2+1, y+5,
							  nroom+ROOMOFFSET, TRUE, TRUE);
						add_room(x+sizetot-sizebig2+1, y+4, x+sizetot-1, y+6, TRUE, COURT, TRUE);
				break;
					}
				case 4: //Normal
					for(i=sizetot-sizebig2+1;i<sizetot;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = CORR;
							levl[x+i][y+j].lit = 1;
							if(rn2(2)) makemon(&mons[PM_PLUMACH_RILMANI], x+i, y+j, 0);
							if(rn2(2)) mkobj_at((rn2(2) ? WEAPON_CLASS : rn2(2) ? TOOL_CLASS : ARMOR_CLASS), x+i, y+j, FALSE);
						}
					}
					flood_fill_rm(x+sizetot-sizebig2+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					add_room(x+sizetot-sizebig2+1, y+4, x+sizetot-1, y+6, TRUE, OROOM, TRUE);
				break;
				case 5: //Gold vault
					for(i=sizetot-sizebig2+1;i<sizetot;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = CORR;
							levl[x+i][y+j].lit = 1;
							if(rn2(2)) makemon(&mons[PM_GOLD_GOLEM], x+i, y+j, 0);
							mkgold((long) rn1((10+rnd(10)) * level_difficulty(),10), x+i, y+j);
						}
					}
					flood_fill_rm(x+sizetot-sizebig2+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					add_room(x+sizetot-sizebig2+1, y+4, x+sizetot-1, y+6, TRUE, OROOM, TRUE);
				break;
				case 6: //Tool shed
					for(i=sizetot-sizebig2+1;i<sizetot;i++){
						for(j=1+3;j<4+3;j++){
							levl[x+i][y+j].typ = CORR;
							levl[x+i][y+j].lit = 1;
							mkobj_at(TOOL_CLASS, x+i, y+j, FALSE);
						}
					}
					flood_fill_rm(x+sizetot-sizebig2+1, y+5,
						  nroom+ROOMOFFSET, TRUE, TRUE);
					add_room(x+sizetot-sizebig2+1, y+4, x+sizetot-1, y+6, TRUE, OROOM, TRUE);
				break;
			}
			add_door(x+sizetot-sizebig2,y+5,&rooms[roomnumb]);
			fill_room(&rooms[roomnumb], FALSE);
		}
	}
}

void
place_lolth_vaults()
{
	int num, i;
	//"portal room" upstairs with 4 vrocks
	mklolthup();
	//"portal room" downstairs with 1 marilith
	mklolthdown();
	//trollguards: 6x6 room (4x4 + walls) with 10 trolls.  Gold and gems
	if(!rn2(3))
		mklolthtroll();
	//Gnoll barracks
	else if(rn2(2))
		mklolthgnoll();
	else {
		mklolthtroll();
		mklolthgnoll();
	}
	//(leveled?) werewolf cage?
	//Garden
	if(rn2(3)){
		mklolthgarden();
	}
	if(!rn2(3)){
		mklolthgarden();
	}
	//4 drow clerics and 1 drow wizard Trapped chest, 4 full healing potions, gold and gems
	mklolthtreasure();
	//Co-alligned temple.  Priest transforms and becomes hostile if adjacent.
	//prison
	//sepulcher
	if(!rn2(4)){
		mklolthsepulcher();
	}
	//mivault
	num = rnd(3);
	for(i = 0; i < num; i++) mkmivaultlolth();
	//Vault
	num = rnd(6);
	for(i = 0; i < num; i++) mkvaultlolth();
}

void
place_neutral_features()
{
	if(!rn2(40)){
		mkkamereltowers();
	} else if(!rn2(20)){
		mkminorspire();
	} else if(!rn2(10)){
		mkneuriver();
	}
	
	
	if(!rn2(10)){
		mkpluvillage();
	// } else if(){
		// mkferufort();
	}
	
	if(!rn2(4)){
		int n = rnd(4) + rn2(4);
		for(n; n > 0; n--)
			mkpluhomestead();
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
	case ARMORY:	mkzoo(ARMORY); break;
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
	int ctype = 0, tries = 0;
	coord mm;
	int zlevel, minmlev, maxmlev;
	
	zlevel = level_difficulty();
	/* determine the level of the weakest monster to make. */
	minmlev = zlevel / 3;
	/* determine the level of the strongest monster to make. */
	maxmlev = (zlevel + u.ulevel) / 2;

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
		case ARMORY:
			mkarmory(sroom);
			/* mkarmory() sets flags and we don't want other fillings */
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
		
		levl[tx][ty].typ = THRONE;
		if(In_outlands(&u.uz)){
			ctype = PM_AURUMACH_RILMANI;
			mon = 0;
			if(!toostrong(ctype,maxmlev+5))
				mon = makemon(&mons[ctype], tx, ty, NO_MM_FLAGS|MM_NOCOUNTBIRTH);
			if(mon) {
				mon->msleeping = 1;
				if(ctype == PM_DROW_MATRON || ctype == PM_EMBRACED_DROWESS){
					set_curhouse(mon->mfaction);
				}
			}
		} else if(rn2(4)){
			/* recalculae minlev to be stricter about weak throne monsters */
			minmlev = zlevel * 2  / 3;
			/* allow mildly out-of-depth lords */
			maxmlev += 5;
			
			static const int kingnums[] = {
				PM_DWARF_KING,
				PM_DWARF_QUEEN,
				PM_KOBOLD_LORD,
				PM_ORC_CAPTAIN,
				PM_URUK_CAPTAIN,
				PM_ORC_OF_THE_AGES_OF_STARS,
				PM_GNOME_KING,
				PM_GNOME_QUEEN,
				PM_TITAN,
				PM_DEEPEST_ONE,
				PM_OGRE_KING,
				PM_VAMPIRE_LORD,
				PM_ELVENKING,
				PM_ELVENQUEEN,
				PM_DROW_MATRON,
				PM_EMBRACED_DROWESS
			};
			
			do ctype = kingnums[rn2(SIZE(kingnums))];
			while(tooweak(ctype, minmlev) || toostrong(ctype,maxmlev) || tries++ > 40);
			if(tries <= 40){
				mon = makemon(&mons[ctype], tx, ty, NO_MM_FLAGS|MM_NOCOUNTBIRTH);
				if(mon) {
					mon->msleeping = 1;
					if (type==COURT && mon->mpeaceful) {
						mon->mpeaceful = 0;
						set_malign(mon);
					}
					if(ctype == PM_DROW_MATRON || ctype == PM_EMBRACED_DROWESS){
						set_curhouse(mon->mfaction);
					}
				}
			}
			else ctype = 0;
		} else ctype = 0;
		
	    throne_placed:
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
		if(type == COURT){
			if(ctype == PM_KOBOLD_LORD || ctype == PM_VAMPIRE_LORD || ctype == PM_DROW_MATRON ||
				ctype == PM_EMBRACED_DROWESS || ctype == PM_DEEPEST_ONE || ctype == PM_ORC_OF_THE_AGES_OF_STARS
			){
				levl[tx][ty].lit = 0;
				sroom->rlit = 0;
			} else if(ctype != 0){
				levl[tx][ty].lit = 1;
				sroom->rlit = 1;
			}
		}
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
		/* don't place monster on throne */
		if(type == COURT && IS_THRONE(levl[sx][sy].typ))
		    continue;
		if(!(Role_if(PM_NOBLEMAN) && In_quest(&u.uz) )){
		mon = makemon(
		    (type == COURT) ? courtmon(ctype) :
		    (type == BARRACKS) ? (In_outlands(&u.uz) ? neu_squadmon() : squadmon()) :
		    (type == MORGUE) ? morguemon() :
		    (type == BEEHIVE) ?
			(sx == tx && sy == ty ? &mons[PM_QUEEN_BEE] :
			 &mons[PM_KILLER_BEE]) :
		    (type == LEPREHALL) ? &mons[PM_LEPRECHAUN] :
		    (type == COCKNEST) ? &mons[PM_COCKATRICE] :
		    (type == ANTHOLE) ? antholemon() :
		    (struct permonst *) 0,
		   sx, sy, NO_MM_FLAGS|MM_NOCOUNTBIRTH);
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
			    (void) mksobj_at((rn2(3)) ? BOX : CHEST,
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
			    (void) mksobj_at((rn2(3)) ? BOX : CHEST,
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
		  struct obj *chest, *gold;
		  (void) somexy(sroom, &mm);
		  gold = mksobj(GOLD_PIECE, TRUE, FALSE);
		  gold->quan = (long) rn1(50 * level_difficulty(), 10);
		  gold->owt = weight(gold);
		  /* the royal coffers */
		  chest = mksobj_at(CHEST, mm.x, mm.y, TRUE, FALSE);
		  chest->spe = 2; /* so it can be found later */
		  add_to_container(chest, gold);
		  chest->owt = weight(chest);
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
	set_curhouse(0);
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
	return ((mvitals[mtyp].mvflags & G_GONE && !In_quest(&u.uz)) ?
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
			mtmp->mvar3 = 1;
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
	    if ((levl[pos.x][pos.y].typ == ROOM || levl[pos.x][pos.y].typ == CORR) && !MON_AT(pos.x,pos.y) &&
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
			struct monst *mtmp = makemon(&mons[PM_LIVING_LECTERN], pos.x,pos.y, NO_MM_FLAGS);
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
mkarmory(croom)
struct mkroom *croom; /* NULL == choose random room */
{
    register int tryct = 0;
	register struct obj *otmp;
    boolean maderoom = FALSE;
    coord pos;
    register int i, tried;

    while ((tryct++ < 25) && !maderoom) {
	register struct mkroom *sroom = croom ? croom : &rooms[rn2(nroom)];
	
	if (sroom->hx < 0 || (!croom && (sroom->rtype != OROOM)))
	    	continue;

	sroom->rtype = ARMORY;
	maderoom = TRUE;
	level.flags.has_armory = 1;
	
	tried = 0;
	i = rnd(2);
	while ((tried++ < 50) && (i > 0) && somexy(sroom, &pos)) {
	    struct permonst *pmon;
	    if (!MON_AT(pos.x, pos.y)) {
			struct monst *mtmp = makemon(&mons[PM_RUST_MONSTER], pos.x,pos.y, NO_MM_FLAGS);
			if (mtmp) mtmp->msleeping = 1;
			i--;
	    }
	}
	tried = 0;
	i = rn2(3);
	while ((tried++ < 50) && (i > 0) && somexy(sroom, &pos)) {
	    struct permonst *pmon;
	    if (!MON_AT(pos.x, pos.y)) {
			struct monst *mtmp = makemon(&mons[PM_BROWN_PUDDING], pos.x,pos.y, NO_MM_FLAGS);
			if (mtmp) mtmp->mstrategy |= STRAT_WAITFORU;
			i--;
	    }
	}
	for(pos.x=sroom->lx; pos.x <= sroom->hx; pos.x++){
		for(pos.y=sroom->ly; pos.y <= sroom->hy; pos.y++){
			if(!rn2(8)){
				otmp = mkobj_at(WEAPON_CLASS, pos.x, pos.y, FALSE);
				if(rn2(3) && (is_rustprone(otmp) || is_flammable(otmp))){
					otmp->oeroded = rn2(4);
				} else if(is_corrodeable(otmp) || is_rottable(otmp)){
					otmp->oeroded2 = rn2(4);
				}
				if(rn2(2)){
					otmp->spe = rn2(3)+rn2(3);
				}
			}
			if(!rn2(8)){
				otmp = mkobj_at(ARMOR_CLASS, pos.x, pos.y, FALSE);
				if(rn2(3) && (is_rustprone(otmp) || is_flammable(otmp))){
					otmp->oeroded = rn2(4);
				} else if(is_corrodeable(otmp) || is_rottable(otmp)){
					otmp->oeroded2 = rn2(4);
				}
				if(rn2(2)){
					otmp->spe = rn2(3)+rn2(2);
				}
			}
			if(!rn2(4)){
				otmp = mkobj_at(WEAPON_CLASS, pos.x, pos.y, FALSE);
				if(rn2(3) && (is_rustprone(otmp) || is_flammable(otmp))){
					otmp->oeroded = rn2(4);
				} else if(is_corrodeable(otmp) || is_rottable(otmp)){
					otmp->oeroded2 = rn2(4);
				}
			}
			if(!rn2(4)){
				otmp = mkobj_at(ARMOR_CLASS, pos.x, pos.y, FALSE);
				if(rn2(3) && (is_rustprone(otmp) || is_flammable(otmp))){
					otmp->oeroded = rn2(4);
				} else if(is_corrodeable(otmp) || is_rottable(otmp)){
					otmp->oeroded2 = rn2(4);
				}
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
		    } else {
				levl[sx][sy].typ = PUDDLE;
				if(!rn2(4))	/* swamps tend to be moldy */
					(void) makemon(&mons[!rn2(4) ? PM_GREEN_MOLD : 
										 !rn2(3) ? PM_BROWN_MOLD: 
										 !rn2(2) ? PM_YELLOW_MOLD:
										PM_RED_MOLD],
							sx, sy, NO_MM_FLAGS);
				else if(!rn2(6)) /* swamp ferns like swamps */
					(void) makemon(&mons[PM_SWAMP_FERN],
							sx, sy, NO_MM_FLAGS);
				else if(!rn2(6)) /* many phantom fungi */
					(void) makemon(&mons[PM_PHANTOM_FUNGUS],
							sx, sy, NO_MM_FLAGS);
			}
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

STATIC_OVL void
mkneuriver()	/* John Harris */
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
				neuliquify(prog, fill, edge);
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
		for (prog = 0; prog<ROWNO; prog++) {
			edge = TRUE;
			for (fill=center-(width/2); fill<=center+(width/2) ; fill++) {
				neuliquify(fill, prog, edge);
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

STATIC_OVL void
neuliquify(x, y, edge)
register xchar x, y;
register boolean edge; /* Allows room walls to intrude slightly into river. */
{
	register int typ = levl[x][y].typ;
	register int monster = PM_JELLYFISH;
	/* Don't liquify shop walls */
	if (level.flags.has_shop && *in_rooms(x, y, SHOPBASE)) {return;}
	if (typ!=TREE || (!edge && rn2(6))){
		if(typ == TREE) levl[x][y].typ = POOL;
		else levl[x][y].typ = PUDDLE;
	}
	// else if ((typ == SCORR || typ == CORR || IS_DOOR(typ)
					// || typ == SDOOR) && !IS_WALL(typ)) {
			// levl[x][y].typ = ROOM;
	// }
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
	levl[x][y].lit = 1;
}

coord *
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
courtmon(kingnum)
	int kingnum;
{
	int i;
	switch(kingnum){
		case PM_DWARF:
		case PM_DWARF_KING:
		case PM_DWARF_QUEEN:
			i = rnd(100);
			if(i>95)
				return &mons[PM_DWARF_LORD];
			else if(i>90)
				return &mons[PM_DWARF_CLERIC];
			else if(i>50)
				return &mons[PM_DWARF];
			else if(i>20)
				return &mons[PM_GNOME];
			else if(i>17)
				return &mons[PM_GNOME_LORD];
			else if(i>14)
				return &mons[PM_GNOME_LADY];
			else if(i>10)
				return &mons[PM_GNOMISH_WIZARD];
			else if(i> 5)
				return &mons[PM_HOBBIT];
			else if(i> 0)
				return &mons[PM_LEPRECHAUN];
		break;
		
		case PM_KOBOLD:
		case PM_KOBOLD_LORD:
			i = rnd(100);
			if(i>95)
				return &mons[PM_KOBOLD_SHAMAN];
			else if(i>60)
				return &mons[PM_LARGE_KOBOLD];
			else if(i>10)
				return &mons[PM_KOBOLD];
			else if(i> 2)
				return &mons[PM_LEMURE];
			else if(i> 0)
				return &mons[PM_IMP];
		break;
		
		case PM_ORC:
		case PM_ORC_CAPTAIN:
			i = rnd(100);
			if(i>95)
				return &mons[PM_ORC_SHAMAN];
			else if(i>80)
				return &mons[PM_MORDOR_ORC];
			else if(i>60)
				return &mons[PM_HILL_ORC];
			else if(i>40)
				return &mons[PM_HOBGOBLIN];
			else if(i>10)
				return &mons[PM_GOBLIN];
			else if(i> 2)
				return &mons[PM_MANES];
			else if(i> 0)
				return &mons[PM_QUASIT];
		break;
		
		case PM_URUK_CAPTAIN:
			i = rnd(100);
			if(i>95)
				return &mons[PM_ORC_SHAMAN];
			else if(i>60)
				return &mons[PM_URUK_HAI];
			else if(i>40)
				return &mons[PM_HOBGOBLIN];
			else if(i>10)
				return &mons[PM_GOBLIN];
			else if(i> 2)
				return &mons[PM_LEMURE];
			else if(i> 0)
				return &mons[PM_IMP];
		break;
		
		case PM_ORC_OF_THE_AGES_OF_STARS:
			i = rnd(100);
			if(i>98)
				return &mons[PM_DEEP_WYRM];
			else if(i>96)
				return &mons[PM_RED_DRAGON];
			else if(i>94)
				return &mons[PM_WHITE_DRAGON];
			else if(i>92)
				return &mons[PM_BLACK_DRAGON];
			else if(i>90)
				return &mons[PM_BLUE_DRAGON];
			else if(i>60)
				return &mons[PM_ANGBAND_ORC];
			else if(i>40)
				return &mons[PM_HEDROW_BLADEMASTER];
			else if(i>36)
				return &mons[PM_SUCCUBUS];
			else if(i>32)
				return &mons[PM_INCUBUS];
			else if(i>20)
				return &mons[PM_OLOG_HAI];
			else if(i>10)
				return &mons[PM_SIEGE_OGRE];
			else if(i> 0)
				return &mons[PM_HELL_HOUND];
		break;
		
		case PM_GNOME:
		case PM_GNOME_KING:
		case PM_GNOME_QUEEN:
			i = rnd(100);
			if(i>95)
				return &mons[PM_GNOME_LORD];
			else if(i>90)
				return &mons[PM_GNOME_LADY];
			else if(i>50)
				return &mons[PM_GNOME];
			else if(i>45)
				return &mons[PM_GNOMISH_WIZARD];
			else if(i>40)
				return &mons[PM_TINKER_GNOME];
			else if(i>20)
				return &mons[PM_DWARF];
			else if(i>17)
				return &mons[PM_DWARF_LORD];
			else if(i>14)
				return &mons[PM_DWARF_CLERIC];
			else if(i>10)
				return &mons[PM_HOBBIT];
			else if(i> 5)
				return &mons[PM_LEPRECHAUN];
			else if(i> 0)
				return &mons[PM_HOMUNCULUS];
		break;
		
		case PM_GIANT:
		case PM_TITAN:
			i = rnd(100);
			if(i>90)
				return &mons[PM_STORM_GIANT];
			else if(i>75)
				return &mons[PM_FIRE_GIANT];
			else if(i>60)
				return &mons[PM_FROST_GIANT];
			else if(i>35)
				return &mons[PM_HILL_GIANT];
			else if(i>20)
				return &mons[PM_STONE_GIANT];
			else if(i>17)
				return &mons[PM_GIANT];
			else if(i>15)
				return &mons[PM_SON_OF_TYPHON];
			else if(i>11)
				return &mons[PM_DRYAD];
			else if(i> 7)
				return &mons[PM_NAIAD];
			else if(i> 3)
				return &mons[PM_OREAD];
			else if(i> 0)
				return &mons[PM_DEMINYMPH];
		break;
		
		case PM_DEEP_ONE:
		case PM_DEEPEST_ONE:
			i = rnd(100);
			if(i>80)
				return &mons[PM_DEEPER_ONE];
			else if(i>30)
				return &mons[PM_DEEP_ONE];
			else if(i>25)
				return &mons[PM_MIND_FLAYER];
			else if(i>20)
				return &mons[PM_DARK_YOUNG];
			else if(i>15)
				return &mons[PM_UMBER_HULK];
			else if(i>11)
				return &mons[PM_STUMBLING_HORROR];
			else if(i> 7)
				return &mons[PM_WANDERING_HORROR];
			else if(i> 3)
				return &mons[PM_SHAMBLING_HORROR];
			else if(i> 0)
				return &mons[PM_HUMAN];
		break;
		
		case PM_OGRE:
		case PM_OGRE_KING:
			i = rnd(100);
			if(i>95)
				return &mons[PM_OGRE_MAGE];
			else if(i>85)
				return &mons[PM_OGRE_LORD];
			else if(i>50)
				return &mons[PM_OGRE];
			else if(i>40)
				return &mons[PM_ORC_CAPTAIN];
			else if(i>20)
				return &mons[PM_MORDOR_ORC];
			else if(i>15)
				return &mons[PM_TENGU];
			else if(i> 5)
				return &mons[PM_TROLL];
			else if(i> 0)
				return(mkclass(S_GIANT, Inhell ? G_HELL : G_NOHELL));
		break;
		
		case PM_VAMPIRE:
		case PM_VAMPIRE_LORD:
			i = rnd(100);
			if(i>90)
				return &mons[PM_VAMPIRE];
			else if(i>70)
				return(mkclass(S_ZOMBIE, Inhell ? G_HELL : G_NOHELL));
			else if(i>65)
				return(mkclass(S_MUMMY, Inhell ? G_HELL : G_NOHELL));
			else if(i>60)
				return &mons[PM_FLESH_GOLEM];
			else if(i>55)
				return &mons[PM_WEREWOLF];
			else if(i>50)
				return &mons[PM_FLOATING_EYE];
			else if(i>45)
				return &mons[PM_GARGOYLE];
			else if(i>40)
				return &mons[PM_WINGED_GARGOYLE];
			else if(i>35)
				return &mons[PM_QUASIT];
			else if(i>30)
				return &mons[PM_BLACK_PUDDING];
			else if(i>25)
				return &mons[PM_WRAITH];
			else if(i>20)
				return &mons[PM_PHANTASM];
			else if(i>15)
				return &mons[PM_GHOST];
			else if(i>10)
				return &mons[PM_MAID];
			else if(i> 5)
				return &mons[PM_WEEPING_WILLOW];
			else if(i> 0)
				return &mons[PM_SWAMP_NYMPH];
		break;
		
		case PM_ELF:
		case PM_ELVENKING:
		case PM_ELVENQUEEN:
			i = rnd(100);
			if(i>95)
				return &mons[PM_ELF_LORD];
			else if(i>90)
				return &mons[PM_ELF_LADY];
			else if(i>75)
				return &mons[PM_GREY_ELF];
			else if(i>60)
				return &mons[PM_GREEN_ELF];
			else if(i>50)
				return &mons[PM_WOODLAND_ELF];
			else if(i>40)
				return &mons[PM_QUICKLING];
			else if(i>32)
				return &mons[PM_DRYAD];
			else if(i>30)
				return &mons[PM_NAIAD];
			else if(i>20)
				return &mons[PM_COURE_ELADRIN];
			else if(i>18)
				return &mons[PM_NOVIERE_ELADRIN];
			else if(i>16)
				return &mons[PM_BRALANI_ELADRIN];
			else if(i>13)
				return &mons[PM_FIRRE_ELADRIN];
			else if(i>10)
				return &mons[PM_SHIERE_ELADRIN];
			else if(i> 5)
				return &mons[PM_DOPPELGANGER];
			else if(i> 0)
				return &mons[PM_WOOD_TROLL];
		break;
		
		case PM_DROW:
		case PM_DROW_MATRON:
			i = rnd(100);
			if(i>90)
				return &mons[PM_UNEARTHLY_DROW];
			if(i>80)
				return &mons[PM_HEDROW_WIZARD];
			if(i>50)
				return &mons[PM_HEDROW_WARRIOR];
			if(i>40)
				return &mons[PM_DROW];
			if(i>30)
				return &mons[PM_SPROW];
			if(i>25)
				return &mons[PM_SWAMP_NYMPH];
			if(i>15)
				return &mons[PM_GIANT_SPIDER];
			if(i>10)
				return &mons[PM_CAVE_LIZARD];
			if(i> 5)
				return &mons[PM_LARGE_CAVE_LIZARD];
			if(i> 0)
				return &mons[PM_QUASIT];
		break;
		
		case PM_EMBRACED_DROWESS:
			i = rnd(100);
			if(i>95)
				return &mons[PM_DROW_ALIENIST];
			else if(i>90)
				return &mons[PM_HEDROW_BLADEMASTER];
			else if(i>70)
				return &mons[PM_EDDERKOP];
			else if(i>60)
				return &mons[PM_HEDROW_WIZARD];
			else if(i>30)
				return &mons[PM_HEDROW_WARRIOR];
			else if(i> 0)
				return &mons[PM_DROW_MUMMY];
		break;
		
		case PM_AURUMACH_RILMANI:
			i = rnd(100);
			if(i>99)
				return &mons[PM_ARGENACH_RILMANI];
			else if(i>95)
				return &mons[PM_ARGENTUM_GOLEM];
			else if(i>90)
				return &mons[PM_CUPRILACH_RILMANI];
			else if(i>80)
				return &mons[PM_FERRUMACH_RILMANI];
			else if(i>60)
				return &mons[PM_PLUMACH_RILMANI];
			else if(i>50)
				return &mons[PM_PLAINS_CENTAUR];
			else if(i>45)
				return &mons[PM_DRYAD];
			else if(i>40)
				return &mons[PM_STONE_GIANT];
			else if(i>35)
				return &mons[PM_HILL_GIANT];
			else if(i>30)
				return &mons[PM_FROST_GIANT];
			else if(i>25)
				return &mons[PM_STORM_GIANT];
			else if(i>20)
				return &mons[PM_QUANTUM_MECHANIC];
			else if(i>15)
				return &mons[PM_SERPENT_MAN_OF_YOTH];
			else if(i>10)
				return &mons[PM_DEEP_ONE];
			else if(i>7)
				return &mons[PM_DEEPER_ONE];
			else if(i>5)
				return &mons[PM_MIND_FLAYER];
			else if(i>1)
				return &mons[PM_HOOLOOVOO];
			else
				return &mons[PM_UVUUDAUM];
		break;
		
		default:
			i = rn2(60) + rn2(3*level_difficulty());
			if (i > 100)		return(mkclass(S_DRAGON, Inhell ? G_HELL : G_NOHELL));
			else if (i > 95)	return(mkclass(S_GIANT, Inhell ? G_HELL : G_NOHELL));
			else if (i > 85)	return(mkclass(S_TROLL, Inhell ? G_HELL : G_NOHELL));
			else if (i > 75)	return(mkclass(S_CENTAUR, Inhell ? G_HELL : G_NOHELL));
			else if (i > 60)	return(mkclass(S_ORC, Inhell ? G_HELL : G_NOHELL));
			else if (i > 45)	return(&mons[PM_BUGBEAR]);
			else if (i > 30)	return(&mons[PM_HOBGOBLIN]);
			else if (i > 15)	return(mkclass(S_GNOME, Inhell ? G_HELL : G_NOHELL));
			else			return(mkclass(S_KOBOLD, Inhell ? G_HELL : G_NOHELL));
		break;
	}
}

struct permonst *
mivaultmon()
{
	switch(rn2(22)){
		case 0:
			return(&mons[PM_SHOGGOTH]);
		break;
		case 1:
			return(&mons[PM_PRIEST_OF_GHAUNADAUR]);
		break;
		case 2:
			return(&mons[PM_EYE_OF_DOOM]);
		break;
		case 3:
			return(&mons[PM_SON_OF_TYPHON]);
		break;
		case 4:
			return(&mons[PM_NIGHTGAUNT]);
		break;
		case 5:
			return(&mons[PM_DARK_YOUNG]);
		break;
		case 6:
			return(&mons[PM_MIRKWOOD_ELDER]);
		break;
		case 7:
			return(&mons[PM_PURPLE_WORM]);
		break;
		case 8:
			return(&mons[PM_HUNTING_HORROR]);
		break;
		case 9:
			return(&mons[PM_BEBELITH]);
		break;
		case 10:
			return(&mons[PM_AMMIT]);
		break;
		case 11:
			return(&mons[PM_GIGANTIC_PSEUDODRAGON]);
		break;
		case 12:
			return(&mons[PM_JUGGERNAUT]);
		break;
		case 13:
			return(&mons[PM_ID_JUGGERNAUT]);
		break;
		case 14:
			return(&mons[PM_SCRAP_TITAN]);
		break;
		case 15:
			return(&mons[PM_HELLFIRE_COLOSSUS]);
		break;
		case 16:
			return(&mons[PM_TITAN]);
		break;
		case 17:
			return(&mons[PM_JABBERWOCK]);
		break;
		case 18:
			return(&mons[PM_GUARDIAN_NAGA]);
		break;
		case 19:
			return(&mons[PM_GROVE_GUARDIAN]);
		break;
		case 20:
			return(&mons[PM_IRON_GOLEM]);
		break;
		case 21:
			return(&mons[PM_BALROG]);
		break;
	}
}

#define NSTYPES (PM_CAPTAIN - PM_SOLDIER + 1)

static struct {
    unsigned	pm;
    unsigned	prob;
} squadprob[NSTYPES] = {
    {PM_SOLDIER, 80}, {PM_SERGEANT, 15}, {PM_LIEUTENANT, 4}, {PM_CAPTAIN, 1}
}, neu_squadprob[NSTYPES] = {
    {PM_FERRUMACH_RILMANI, 80}, {PM_IRON_GOLEM, 15}, {PM_ARGENTUM_GOLEM, 4}, {PM_ARGENACH_RILMANI, 1}
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
//	if (!(mvitals[mndx].mvflags & G_GONE && !In_quest(&u.uz))) return(&mons[mndx]);
	if (!(mvitals[mndx].mvflags & G_GENOD && !In_quest(&u.uz))) return(&mons[mndx]);//empty if genocided
	else			    return((struct permonst *) 0);
}

STATIC_OVL struct permonst *
neu_squadmon()		/* return soldier types. */
{
	int sel_prob, i, cpro, mndx;

	sel_prob = rnd(80+level_difficulty());

	cpro = 0;
	for (i = 0; i < NSTYPES; i++) {
	    cpro += neu_squadprob[i].prob;
	    if (cpro > sel_prob) {
		mndx = neu_squadprob[i].pm;
		goto gotone;
	    }
	}
	mndx = neu_squadprob[rn2(NSTYPES)].pm;
gotone:
//	if (!(mvitals[mndx].mvflags & G_GONE && !In_quest(&u.uz))) return(&mons[mndx]);
	if (!(mvitals[mndx].mvflags & G_GENOD && !In_quest(&u.uz))) return(&mons[mndx]);//empty if genocided
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
