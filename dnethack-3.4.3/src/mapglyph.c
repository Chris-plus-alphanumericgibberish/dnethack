/*	SCCS Id: @(#)mapglyph.c	3.4	2003/01/08	*/
/* Copyright (c) David Cohrs, 1991				  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#if defined(TTY_GRAPHICS)
#include "wintty.h"	/* for prototype of has_color() only */
#endif
#include "color.h"
#define HI_DOMESTIC CLR_WHITE	/* monst.c */

int explcolors[] = {
	CLR_BLACK,	/* dark    */
	CLR_GREEN,	/* noxious */
	CLR_BROWN,	/* muddy   */
	CLR_BLUE,	/* wet     */
	CLR_MAGENTA,	/* magical */
	CLR_ORANGE,	/* fiery   */
	CLR_WHITE,	/* frosty  */
	CLR_GRAY,	/* gray  */
	CLR_BRIGHT_GREEN,	/* lime  */
	CLR_YELLOW,	/* yellow  */
	CLR_BRIGHT_BLUE,	/* bright blue */
	CLR_BRIGHT_MAGENTA,	/* magenta */
	CLR_RED,	/* red */
	CLR_BRIGHT_CYAN,	/* cyan */
};

#if !defined(TTY_GRAPHICS)
#define has_color(n)  TRUE
#endif

#ifdef TEXTCOLOR
#define zap_color(n)  color = iflags.use_color ? zapcolors[n] : NO_COLOR
#define cmap_color(n) color = iflags.use_color ? defsyms[n].color : NO_COLOR
#define obj_color(n)  color = iflags.use_color ? objects[n].oc_color : NO_COLOR
#define mon_color(n)  color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define invis_color(n) color = NO_COLOR
#define pet_color(n)  color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define zombie_color(n)  color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define peace_color(n)  color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define warn_color(n) color = iflags.use_color ? def_warnsyms[n].color : NO_COLOR
#define explode_color(n) color = iflags.use_color ? explcolors[n] : NO_COLOR
# if defined(REINCARNATION) && defined(ASCIIGRAPH)
#  define ROGUE_COLOR
# endif

#else	/* no text color */

#define zap_color(n)
#define cmap_color(n)
#define obj_color(n)
#define mon_color(n)
#define invis_color(n)
#define pet_color(c)
#define warn_color(n)
#define explode_color(n)
#endif

#ifdef ROGUE_COLOR
# if defined(USE_TILES) && defined(MSDOS)
#define HAS_ROGUE_IBM_GRAPHICS (iflags.IBMgraphics && !iflags.grmode && \
	Is_rogue_level(&u.uz))
# else
#define HAS_ROGUE_IBM_GRAPHICS (iflags.IBMgraphics && Is_rogue_level(&u.uz))
# endif
#endif

/*ARGSUSED*/
void
mapglyph(glyph, ochar, ocolor, ospecial, x, y)
int glyph, *ocolor, x, y;
int *ochar;
unsigned *ospecial;
{
	register int offset;
#if defined(TEXTCOLOR) || defined(ROGUE_COLOR)
	int color = NO_COLOR;
#endif
	uchar ch;
	unsigned special = 0;

    /*
     *  Map the glyph back to a character and color.
     *
     *  Warning:  For speed, this makes an assumption on the order of
     *		  offsets.  The order is set in display.h.
     */
    if ((offset = (glyph - GLYPH_WARNING_OFF)) >= 0) {	/* a warning flash */
    	ch = warnsyms[offset];
# ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS)
	    color = NO_COLOR;
	else
# endif
	    warn_color(offset);
    } else if ((offset = (glyph - GLYPH_SWALLOW_OFF)) >= 0) {	/* swallow */
	/* see swallow_to_glyph() in display.c */
	ch = (uchar) showsyms[S_sw_tl + (offset & 0x7)];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color)
	    color = NO_COLOR;
	else
#endif
	    mon_color(offset >> 3);
    } else if ((offset = (glyph - GLYPH_ZAP_OFF)) >= 0) {	/* zap beam */
	/* see zapdir_to_glyph() in display.c */
	ch = showsyms[S_vbeam + (offset & 0x3)];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color)
	    color = NO_COLOR;
	else
#endif
	    zap_color((offset >> 2));
    } else if ((offset = (glyph - GLYPH_EXPLODE_OFF)) >= 0) {	/* explosion */
		ch = showsyms[(offset % MAXEXPCHARS) + S_explode1];
		explode_color(offset / MAXEXPCHARS);
    } else if ((offset = (glyph - GLYPH_CMAP_OFF)) >= 0) {	/* cmap */
	ch = showsyms[offset];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color) {
	    if (offset >= S_vwall && offset <= S_hcdoor)
		color = CLR_BROWN;
	    else if (offset >= S_arrow_trap && offset <= S_polymorph_trap)
		color = CLR_MAGENTA;
	    else if (offset == S_corr || offset == S_litcorr)
		color = CLR_GRAY;
	    else if (offset >= S_drkroom && offset <= S_water)
		color = CLR_GREEN;
	    else
		color = NO_COLOR;
	} else
#endif
#ifdef TEXTCOLOR
	    /* provide a visible difference if normal and lit corridor
	     * use the same symbol */
	    if (iflags.use_color &&
		offset == S_litcorr && ch == showsyms[S_corr])
		color = CLR_WHITE;
	    else
#endif
		/* Special colours for special dungeon areas */
		if(iflags.use_color && iflags.dnethack_dungeon_colors){
			if(In_sokoban(&u.uz)){
				if(offset >= S_vwall && offset <= S_trwall){
					color = CLR_BRIGHT_BLUE;
				}
			} else if(In_endgame(&u.uz)){
				if(Is_earthlevel(&u.uz)){
					if(offset >= S_vwall && offset <= S_trwall){
						color = CLR_BROWN;
					}
					if(offset >= S_drkroom && offset <= S_dnladder){
						color = CLR_BROWN;
					}
				} else if(Is_waterlevel(&u.uz)){
					if(offset < S_arrow_trap){
						color = CLR_BRIGHT_BLUE;
					}
					if(offset == S_water){
						color = CLR_BLUE;
					}
				} else if(Is_firelevel(&u.uz)){
					if(offset >= S_vwall && offset <= S_trwall){
						color = CLR_BLACK;
					}
					if(offset >= S_drkroom && offset <= S_dnladder){
						color = offset == S_litroom ? CLR_GRAY : CLR_BLACK;
					}
				}
			} else if(Is_sunsea(&u.uz)){
				if(offset >= S_vwall && offset <= S_trwall){
					color = CLR_BROWN;
				}
				if(offset >= S_drkroom && offset <= S_dnladder){
					color = CLR_BROWN;
				}
			} else if(Is_paradise(&u.uz)){
				if(offset >= S_water){
					color = CLR_BRIGHT_BLUE;
				} else if(offset == S_drkroom || offset == S_corr || offset <= S_hcdoor){
					color = CLR_BROWN;
				} else if(offset >= S_litroom && offset <= S_dnladder){
					color = CLR_YELLOW;
				}
			} else if(Is_sunkcity(&u.uz)){
				if(offset >= S_drkroom && offset <= S_litcorr){
					color = CLR_BRIGHT_GREEN;
				}
			} else if(Is_peanut(&u.uz)){
				if(offset >= S_vwall && offset <= S_trwall){
					color = CLR_BROWN;
				} else if(offset >= S_water){
					color = CLR_BRIGHT_BLUE;
				} else if(offset == S_drkroom || offset == S_corr || offset <= S_hcdoor){
					color = CLR_BROWN;
				} else if(offset >= S_litroom && offset <= S_dnladder){
					color = CLR_YELLOW;
				}
			} else if(In_moloch_temple(&u.uz)){
				if(offset >= S_vwall && offset <= S_trwall){
					color = CLR_RED;
				}
				if(offset >= S_drkroom && offset <= S_dnladder){
					color = offset == S_litroom ? CLR_GRAY : CLR_BLACK;
				}
			} else if(In_cha(&u.uz)){
				if(offset >= S_vwall && offset <= S_trwall){
					color = CLR_BLACK;
				}
				if(offset >= S_drkroom && offset <= S_dnladder){
					color = offset == S_litroom ? CLR_GRAY : CLR_BLACK;
				}
				if(Is_lich_level(&u.uz)){
					if(offset >= S_vwall && offset <= S_trwall){
						color = CLR_BROWN;
					}
				} else if(Is_marilith_level(&u.uz)){
					if(offset >= S_vwall && offset <= S_trwall){
						color = CLR_ORANGE;
					}
				} else if(Is_kraken_level(&u.uz)){
					if(offset >= S_vwall && offset <= S_trwall){
						color = CLR_BRIGHT_BLUE;
					}
				} else if(Is_tiamat_level(&u.uz)){
					if(offset >= S_vwall && offset <= S_trwall){
						color = CLR_CYAN;
					}
				}
			} else if(In_neu(&u.uz)){
				if(u.uz.dnum == neutral_dnum && u.uz.dlevel <= sum_of_all_level.dlevel){
					if(offset >= S_vwall && offset <= S_trwall){
						color = CLR_BROWN;
					}
					if(offset == S_drkroom || offset == S_litroom){
						if(u.uz.dlevel < spire_level.dlevel &&
							u.uz.dlevel > gatetown_level.dlevel){
								if (*in_rooms(x,y, SHOPBASE)
									|| *in_rooms(x,y, TEMPLE)
									|| *in_rooms(x,y, BARRACKS)
									|| *in_rooms(x,y, COURT)
								) color = (offset == S_litroom) ? CLR_BROWN : CLR_BLACK;
								else color = (offset == S_litroom) ? CLR_BRIGHT_GREEN : CLR_GREEN;
							}
						else color = (offset == S_litroom) ? CLR_BROWN : CLR_BLACK;
					}
				}
				else if(Is_rlyeh(&u.uz)){
					if(offset >= S_vwall && offset <= S_trwall){
						color = CLR_BRIGHT_BLUE;
					}
					else if(offset >= S_drkroom && offset <= S_dnladder){
						color = CLR_BLUE;
					}
				} else {
					if(offset >= S_vwall && offset <= S_trwall){
						color = CLR_BLACK;
					}
					else if(offset >= S_drkroom && offset <= S_dnladder){
						color = offset == S_litroom ? CLR_GRAY : CLR_BLACK;
					}
				}
			} else if(In_law(&u.uz)){
				if(Is_path(&u.uz)){
					if(offset >= S_drkroom && offset <= S_dnladder){
						color = CLR_BROWN;
					}
				}
				if(Is_arcadia_woods(&u.uz)){
					if(x<69 || !Is_arcadia3(&u.uz)){
						if(offset >= S_vwall && offset <= S_trwall){
							color = CLR_BROWN;
						}
						else if(offset >= S_drkroom && offset <= S_dnladder){
							color = CLR_GREEN;
						}
					}
				}
			} else if(In_hell(&u.uz)){
				if(offset >= S_vwall && offset <= S_trwall){
					color = CLR_RED;
				}
				if(Is_valley(&u.uz)){
					if (iflags.use_color && Is_valley(&u.uz) && color != CLR_BLACK) {
						color = (color < NO_COLOR) ? CLR_GRAY : CLR_WHITE; /* The valley is drained of color */
					}
				}
				if(Is_abyss1(&u.uz)){
					if (Is_juiblex_level(&u.uz)){
						if (offset == S_pool || offset == S_water)
							color = CLR_GREEN;
						if(offset >= S_vwall && offset <= S_hcdoor)
							color = CLR_GREEN;
						if (offset == S_drkroom)
							color = CLR_GREEN;
						if (offset == S_litroom)
							color = CLR_BRIGHT_GREEN;
					}
					else if (Is_zuggtmoy_level(&u.uz)){
						if (offset == S_pool || offset == S_water)
							color = CLR_GREEN;
						if(offset >= S_vwall && offset <= S_hcdoor)
							color = CLR_GREEN;
						if (offset == S_drkroom || offset == S_litroom)
							color = CLR_MAGENTA;
						if (offset == S_tree)
							color = CLR_BRIGHT_MAGENTA;
					}
					else if (Is_yeenoghu_level(&u.uz)){
						if (offset == S_pool || offset == S_water)
							color = CLR_YELLOW;
						if(offset >= S_vwall && offset <= S_hcdoor)
							color = CLR_BROWN;
						if (offset == S_drkroom)
							color = CLR_BROWN;
						if (offset == S_litroom)
							color = CLR_YELLOW;
						if (offset == S_tree)
							color = CLR_BROWN;
					}
					else if (Is_night_level(&u.uz)){
						if(*in_rooms(x,y,MORGUE)){
							if(offset >= S_vwall && offset <= S_hcdoor)
								color = CLR_WHITE;
							if (offset == S_drkroom || offset == S_litroom)
								color = CLR_GRAY;
						}
					}
					// else if (Is_baphomet_level(&u.uz)){
					// }
				} else if(Is_abyss2(&u.uz)){
					if (Is_orcus_level(&u.uz)){
						if(offset >= S_vwall && offset <= S_hcdoor)
							color = CLR_BLACK;
						else if (offset == S_drkroom)
							color = CLR_BLACK;
						else if (offset == S_litroom)
							color = CLR_BROWN;
					}
				} else if(Is_abyss3(&u.uz)){
					if(Is_lamashtu_level(&u.uz) && 
						offset >= S_vwall && 
						offset <= S_hcdoor
					) color = CLR_BROWN;
					else if(offset >= S_vwall && offset <= S_hcdoor)
						color = CLR_GREEN;
				} else if(Is_hell3(&u.uz)){
						if (offset == S_drkroom) color = CLR_RED;
						else if(offset == S_litroom) color = CLR_ORANGE;
				}
			}
			if (offset >= S_vwall && offset <= S_hcdoor) {
				if (*in_rooms(x,y,BEEHIVE))
					color = CLR_YELLOW;
				else if (In_W_tower(x, y, &u.uz))
					color = CLR_MAGENTA;
				else if (In_mines_quest(&u.uz) && !Is_nemesis(&u.uz))
					color = CLR_BROWN;
				else if (Is_astralevel(&u.uz))
					color = CLR_WHITE;
			} else if (offset == S_drkroom || offset == S_litroom) {
				if (*in_rooms(x,y,BEEHIVE))
					color = (offset == S_drkroom) ? CLR_BROWN : CLR_YELLOW;
				else if(In_mines_quest(&u.uz) && !Is_nemesis(&u.uz)){
					color = (offset == S_drkroom) ? CLR_BLACK : CLR_BROWN;
				}
			} else if (offset == S_altar) {
				// if (Hallucination) color = rn2(CLR_MAX); Redraw cycle doesn't trigger unless something passes over square
				if (Is_astralevel(&u.uz)) color = CLR_BRIGHT_MAGENTA;
				else if(Is_sanctum(&u.uz)) color = CLR_MAGENTA;
				// else switch((aligntyp)Amask2align(levl[x][y].altarmask & AM_MASK)) { Commented out due to hallucination code.
					// case A_LAWFUL: color = CLR_WHITE; break; 
					// case A_NEUTRAL: color = CLR_GRAY; break; 
					// case A_CHAOTIC: color = CLR_BLACK; break; 
					// default: color = CLR_RED; break;
				// }
			}
			if(artifact_door(x, y)){
				color = CLR_MAGENTA;
			}
			if(u.uevent.found_square && invocation_pos(x, y) && !On_stairs(x, y)){
				color = CLR_MAGENTA;
				ch = '_';
			}
		}
		if (color == NO_COLOR) cmap_color(offset);
	} else if ((offset = (glyph - GLYPH_OBJ_OFF)) >= 0) {	/* object */

			if ((offset == BOULDER || offset == MASSIVE_STONE_CRATE) && iflags.bouldersym) ch = iflags.bouldersym;
			else ch = oc_syms[(int)objects[offset].oc_class];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color) {
	    switch(objects[offset].oc_class) {
		case COIN_CLASS: color = CLR_YELLOW; break;
		case FOOD_CLASS: color = CLR_RED; break;
		default: color = CLR_BRIGHT_BLUE; break;
	    }
	} else
#endif
	    obj_color(offset);
    } else if ((offset = (glyph - GLYPH_RIDDEN_OFF)) >= 0) {	/* mon ridden */
	ch = monsyms[(int)mons[offset].mlet];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS)
	    /* This currently implies that the hero is here -- monsters */
	    /* don't ride (yet...).  Should we set it to yellow like in */
	    /* the monster case below?  There is no equivalent in rogue. */
	    color = NO_COLOR;	/* no need to check iflags.use_color */
	else
#endif
	    mon_color(offset);
	    special |= MG_RIDDEN;
    } else if ((offset = (glyph - GLYPH_BODY_OFF)) >= 0) {	/* a corpse */
	ch = oc_syms[(int)objects[CORPSE].oc_class];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color)
	    color = CLR_RED;
	else
#endif
	    mon_color(offset);
	    special |= MG_CORPSE;
    } else if ((offset = (glyph - GLYPH_DETECT_OFF)) >= 0) {	/* mon detect */
	ch = monsyms[(int)mons[offset].mlet];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS)
	    color = NO_COLOR;	/* no need to check iflags.use_color */
	else
#endif
	    mon_color(offset);
	/* Disabled for now; anyone want to get reverse video to work? */
	/* is_reverse = TRUE; */
	    special |= MG_DETECT;
    } else if ((offset = (glyph - GLYPH_INVIS_OFF)) >= 0) {	/* invisible */
	ch = DEF_INVISIBLE;
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS)
	    color = NO_COLOR;	/* no need to check iflags.use_color */
	else
#endif
	    invis_color(offset);
	    special |= MG_INVIS;
    } else if ((offset = (glyph - GLYPH_ZOMBIE_OFF)) >= 0) {	/* a zombie */
	ch = monsyms[(int)mons[offset].mlet];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS)
	    color = NO_COLOR;	/* no need to check iflags.use_color */
	else
#endif
	    zombie_color(offset);
	    special |= MG_ZOMBIE;
    } else if ((offset = (glyph - GLYPH_PEACE_OFF)) >= 0) {	/* a peaceful monster */
	ch = monsyms[(int)mons[offset].mlet];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS)
	    color = NO_COLOR;	/* no need to check iflags.use_color */
	else
#endif
	    peace_color(offset);
	    special |= MG_PEACE;
    } else if ((offset = (glyph - GLYPH_PET_OFF)) >= 0) {	/* a pet */
	ch = monsyms[(int)mons[offset].mlet];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS)
	    color = NO_COLOR;	/* no need to check iflags.use_color */
	else
#endif
	    pet_color(offset);
	    special |= MG_PET;
    } else {							/* a monster */
	ch = monsyms[(int)mons[glyph].mlet];
#ifdef ROGUE_COLOR
	if (HAS_ROGUE_IBM_GRAPHICS && iflags.use_color) {
	    if (x == u.ux && y == u.uy)
		/* actually player should be yellow-on-gray if in a corridor */
		color = CLR_YELLOW;
	    else
		color = NO_COLOR;
	} else
#endif
	{
	    mon_color(glyph);
	    /* special case the hero for `showrace' option */
#ifdef TEXTCOLOR
	    if (iflags.use_color && x == u.ux && y == u.uy &&
		    iflags.showrace && !Upolyd)
		color = HI_DOMESTIC;
#endif
		}
    }

#ifdef TEXTCOLOR
    /* Turn off color if no color defined, or rogue level w/o PC graphics. */
# ifdef REINCARNATION
#  ifdef ASCIIGRAPH
    if (!has_color(color) || (Is_rogue_level(&u.uz) && !HAS_ROGUE_IBM_GRAPHICS))
#  else
    if (!has_color(color) || Is_rogue_level(&u.uz))
#  endif
# else
    if (!has_color(color))
# endif
	color = NO_COLOR;
#endif

    *ochar = (int)ch;
    *ospecial = special;
#ifdef TEXTCOLOR
    *ocolor = color;
#endif
    return;
}

/*mapglyph.c*/
