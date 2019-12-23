/*	SCCS Id: @(#)pager.c	3.4	2003/08/13	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file contains the command routines dowhatis() and dohelp() and */
/* a few other help related facilities */

#include "hack.h"
#include "dlb.h"
#ifdef BARD
#include "edog.h"
#endif

STATIC_DCL boolean FDECL(is_swallow_sym, (int));
STATIC_DCL int FDECL(append_str, (char *, const char *));
STATIC_DCL struct monst * FDECL(lookat, (int, int, char *, char *, char *));
STATIC_DCL int FDECL(do_look, (BOOLEAN_P));
STATIC_DCL boolean FDECL(help_menu, (int *));
STATIC_DCL char * get_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_generation_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_weight_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_resistance_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_weakness_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_conveys_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_mm_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_mt_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_mb_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_ma_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_mv_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_mg_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_speed_description_of_monster_type(struct monst *, char *);
STATIC_DCL char * get_description_of_attack_type(uchar);
STATIC_DCL char * get_description_of_damage_type(uchar);
STATIC_DCL char * get_description_of_damage_prefix(uchar, uchar);
STATIC_DCL int generate_list_of_resistances(struct monst *, char *, int);
#ifdef PORT_HELP
extern void NDECL(port_help);
#endif

extern const int monstr[];

/* Returns "true" for characters that could represent a monster's stomach. */
STATIC_OVL boolean
is_swallow_sym(c)
int c;
{
    int i;
    for (i = S_sw_tl; i <= S_sw_br; i++)
	if ((int)showsyms[i] == c) return TRUE;
    return FALSE;
}

/*
 * Append new_str to the end of buf if new_str doesn't already exist as
 * a substring of buf.  Return 1 if the string was appended, 0 otherwise.
 * It is expected that buf is of size BUFSZ.
 */
STATIC_OVL int
append_str(buf, new_str)
    char *buf;
    const char *new_str;
{
    int space_left;	/* space remaining in buf */

    if (strstri(buf, new_str)) return 0;

    space_left = BUFSZ - strlen(buf) - 1;
    (void) strncat(buf, " or ", space_left);
    (void) strncat(buf, new_str, space_left - 4);
    return 1;
}

static const char * const sizeStr[] = {
	"fine",
	"gnat-sized",
	"diminutive",
	"tiny",
	"cat-sized",
	"bigger-than-a-breadbox",
	"small",
	"human-sized",
	"large",
	"gigantic",
	"colossal",
	"cosmically-huge"
};

static const char * const headStr[] = {
	"",
	"",
	"",
	"",
	"",
	" snouted",
	" flat-faced",
	" short-necked",
	" inverted-faced",
	" long-necked"
};

static const char * const bodyStr[] = {
	" animal",
	" ophidian",
	" squigglenoid",
	" insectoid",
	" humanoid",
	" snake-legged humanoid",
	" snake-backed animal",
	" centauroid"
};
/*
 * Return the name of the glyph found at (x,y).
 * If not hallucinating and the glyph is a monster, also monster data.
 */
STATIC_OVL struct monst *
lookat(x, y, buf, monbuf, shapebuff)
    int x, y;
    char *buf, *monbuf, *shapebuff;
{
    register struct monst *mtmp = (struct monst *) 0;
    struct permonst *pm = (struct permonst *) 0;
    int glyph;

	int do_halu = Hallucination;
	
    buf[0] = monbuf[0] = shapebuff[0] = 0;
    glyph = glyph_at(x,y);
    if (u.ux == x && u.uy == y && senseself()) {
	char race[QBUFSZ];

	/* if not polymorphed, show both the role and the race */
	race[0] = 0;
	if (!Upolyd) {
	    Sprintf(race, "%s ", urace.adj);
	}

	Sprintf(buf, "%s%s%s called %s",
		Invis ? "invisible " : "",
		race,
		mons[u.umonnum].mname,
		plname);
	/* file lookup can't distinguish between "gnomish wizard" monster
	   and correspondingly named player character, always picking the
	   former; force it to find the general "wizard" entry instead */
	if (Role_if(PM_WIZARD) && Race_if(PM_GNOME) && !Upolyd)
	    pm = &mons[PM_WIZARD];

#ifdef STEED
	if (u.usteed) {
	    char steedbuf[BUFSZ];

	    Sprintf(steedbuf, ", mounted on %s", y_monnam(u.usteed));
	    /* assert((sizeof buf >= strlen(buf)+strlen(steedbuf)+1); */
	    Strcat(buf, steedbuf);
	}
#endif
	/* When you see yourself normally, no explanation is appended
	   (even if you could also see yourself via other means).
	   Sensing self while blind or swallowed is treated as if it
	   were by normal vision (cf canseeself()). */
	if ((Invisible || u.uundetected) && !Blind && !u.uswallow) {
	    unsigned how = 0;

	    if (Infravision)	 how |= 1;
	    if (Unblind_telepat) how |= 2;
	    if (Detect_monsters) how |= 4;

	    if (how)
		Sprintf(eos(buf), " [seen: %s%s%s%s%s]",
			(how & 1) ? "infravision" : "",
			/* add comma if telep and infrav */
			((how & 3) > 2) ? ", " : "",
			(how & 2) ? "telepathy" : "",
			/* add comma if detect and (infrav or telep or both) */
			((how & 7) > 4) ? ", " : "",
			(how & 4) ? "monster detection" : "");
	}
    } else if (u.uswallow) {
	/* all locations when swallowed other than the hero are the monster */
	Sprintf(buf, "interior of %s",
				    Blind ? "a monster" : a_monnam(u.ustuck));
	pm = u.ustuck->data;
    } else if (glyph_is_monster(glyph)) {
	bhitpos.x = x;
	bhitpos.y = y;
	mtmp = m_at(x,y);
	do_halu = Hallucination || (mtmp && u.usanity < mtmp->m_san_level);
	if (mtmp != (struct monst *) 0) {
	    char *name, monnambuf[BUFSZ];
	    boolean accurate = !do_halu;

	    if (mtmp->data == &mons[PM_COYOTE] && accurate)
		name = coyotename(mtmp, monnambuf);
	    else
		name = distant_monnam(mtmp, ARTICLE_NONE, monnambuf);

	    pm = mtmp->data;
	    Sprintf(buf, "%s%s%s",
		    (mtmp->mx != x || mtmp->my != y) ?
			((mtmp->isshk && accurate)
				? "tail of " : "tail of a ") : "",
		    (mtmp->mtame && accurate) ? 
#ifdef BARD
		    (EDOG(mtmp)->friend ? "friendly " : (EDOG(mtmp)->loyal) ? "loyal " : "tame ") :
#else
		    "tame " :
#endif
		    (mtmp->mpeaceful && accurate) ? (mtmp->data==&mons[PM_UVUUDAUM]) ? "meditating " : "peaceful " : "",
		    name);
	    if (mtmp->data==&mons[PM_DREAD_SERAPH] && mtmp->mvar2)
		Strcat(buf, "praying ");
		
	    if (u.ustuck == mtmp)
		Strcat(buf, (sticks(youracedata)) ?
			", being held" : ", holding you");
	    if (mtmp->mleashed)
		Strcat(buf, ", leashed to you");

	    if (mtmp->mtrapped && cansee(mtmp->mx, mtmp->my)) {
		struct trap *t = t_at(mtmp->mx, mtmp->my);
		int tt = t ? t->ttyp : NO_TRAP;

		/* newsym lets you know of the trap, so mention it here */
		if (tt == BEAR_TRAP || tt == PIT ||
			tt == SPIKED_PIT || tt == WEB)
		    Sprintf(eos(buf), ", trapped in %s",
			    an(defsyms[trap_to_defsym(tt)].explanation));
	    }

		{
		int ways_seen = 0, normal = 0, xraydist;
		boolean useemon = (boolean) canseemon(mtmp);

		xraydist = (u.xray_range<0) ? -1 : u.xray_range * u.xray_range;
		/* normal vision */
		if ((mtmp->wormno ? worm_known(mtmp) : cansee(mtmp->mx, mtmp->my)) &&
			mon_visible(mtmp) && !mtmp->minvis) {
		    ways_seen++;
		    normal++;
		}
		/* see invisible */
		if (useemon && mtmp->minvis)
		    ways_seen++;
		/* infravision */
		if ((!mtmp->minvis || See_invisible(mtmp->mx,mtmp->my)) && see_with_infrared(mtmp))
		    ways_seen++;
		/* bloodsense */
		if ((!mtmp->minvis || See_invisible(mtmp->mx, mtmp->my)) && see_with_bloodsense(mtmp))
		    ways_seen++;
		/* lifesense */
		if ((!mtmp->minvis || See_invisible(mtmp->mx, mtmp->my)) && see_with_lifesense(mtmp))
		    ways_seen++;
		/* senseall */
		if ((!mtmp->minvis || See_invisible(mtmp->mx, mtmp->my)) && see_with_senseall(mtmp))
		    ways_seen++;
		/* earthsense */
		if (see_with_earthsense(mtmp))
		    ways_seen++;
		/* smell */
		if (sense_by_scent(mtmp))
		    ways_seen++;
		/* telepathy */
		if (tp_sensemon(mtmp))
		    ways_seen++;
		/* xray */
		if (useemon && xraydist > 0 &&
			distu(mtmp->mx, mtmp->my) <= xraydist)
		    ways_seen++;
		if (Detect_monsters)
		    ways_seen++;
		if (MATCH_WARN_OF_MON(mtmp))
		    ways_seen++;
		
		if (ways_seen > 1 || !normal) {
		    if (normal) {
			Strcat(monbuf, "normal vision");
			/* can't actually be 1 yet here */
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
		    if (useemon && mtmp->minvis) {
			Strcat(monbuf, "see invisible");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
			if ((!mtmp->minvis || See_invisible(mtmp->mx, mtmp->my)) &&
			    see_with_infrared(mtmp)) {
			Strcat(monbuf, "infravision");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
			if ((!mtmp->minvis || See_invisible(mtmp->mx, mtmp->my)) &&
			    see_with_bloodsense(mtmp)) {
			Strcat(monbuf, "bloodsense");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
			if ((!mtmp->minvis || See_invisible(mtmp->mx, mtmp->my)) &&
			    see_with_lifesense(mtmp)) {
			Strcat(monbuf, "lifesense");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
			}
			if ((!mtmp->minvis || See_invisible(mtmp->mx, mtmp->my)) &&
			    see_with_senseall(mtmp)) {
			Strcat(monbuf, "omnisense");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
			}
		    if (see_with_earthsense(mtmp)) {
			Strcat(monbuf, "earthsense");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
			}
		    if (sense_by_scent(mtmp)) {
			Strcat(monbuf, "scent");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
		    if (tp_sensemon(mtmp)) {
			Strcat(monbuf, "telepathy");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
		    if (useemon && xraydist > 0 &&
			    distu(mtmp->mx, mtmp->my) <= xraydist) {
			/* Eyes of the Overworld */
			Strcat(monbuf, "astral vision");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
		    if (Detect_monsters) {
			Strcat(monbuf, "monster detection");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
		    if (MATCH_WARN_OF_MON(mtmp)) {
		    	char wbuf[BUFSZ];
			if (do_halu){
				Strcat(monbuf, "paranoid delusion");
				if (ways_seen-- > 1) Strcat(monbuf, ", ");
			} else {
				//going to need more detail about how it is seen....
				ways_seen = 0;
				if(u.sealsActive&SEAL_PAIMON && is_magical((mtmp)->data)) ways_seen++;
				if(u.sealsActive&SEAL_ANDROMALIUS && is_thief((mtmp)->data)) ways_seen++;
				if(u.sealsActive&SEAL_TENEBROUS && !nonliving_mon(mtmp)) ways_seen++;
				if(u.specialSealsActive&SEAL_ACERERAK && is_undead_mon(mtmp)) ways_seen++;
				if(uwep && ((uwep->oward & WARD_THJOFASTAFUR) && 
					((mtmp)->data->mlet == S_LEPRECHAUN || (mtmp)->data->mlet == S_NYMPH || is_thief((mtmp)->data)))) ways_seen++;
				if(youracedata == &mons[PM_SHARK] && has_blood_mon(mtmp) &&
						(mtmp)->mhp < (mtmp)->mhpmax && is_pool(u.ux, u.uy, TRUE) && is_pool((mtmp)->mx, (mtmp)->my, TRUE)) ways_seen++;
				if(MATCH_WARN_OF_MON_STRICT(mtmp)){
					Sprintf(wbuf, "warned of %s",
						makeplural(mtmp->data->mname));
					Strcat(monbuf, wbuf);
				} else {
					if(u.sealsActive&SEAL_PAIMON && is_magical((mtmp)->data)){
					Sprintf(wbuf, "warned of magic users");
					Strcat(monbuf, wbuf);
					if (ways_seen-- > 1) Strcat(monbuf, ", ");
					}
					if(u.sealsActive&SEAL_ANDROMALIUS && is_thief((mtmp)->data)){
					Sprintf(wbuf, "warned of item thieves");
					Strcat(monbuf, wbuf);
					if (ways_seen-- > 1) Strcat(monbuf, ", ");
					}
					if(uwep && (uwep->oward & WARD_THJOFASTAFUR) && ((mtmp)->data->mlet == S_LEPRECHAUN || (mtmp)->data->mlet == S_NYMPH || is_thief((mtmp)->data))){
					Sprintf(wbuf, "warned of leprechauns, nymphs, and item thieves");
					Strcat(monbuf, wbuf);
					if (ways_seen-- > 1) Strcat(monbuf, ", ");
					}
					if(u.specialSealsActive&SEAL_ACERERAK && is_undead_mon(mtmp)){
					Sprintf(wbuf, "warned of the undead");
					Strcat(monbuf, wbuf);
					if (ways_seen-- > 1) Strcat(monbuf, ", ");
					}
					if(u.sealsActive&SEAL_TENEBROUS && !nonliving_mon(mtmp)){
					Sprintf(wbuf, "warned of living beings");
					Strcat(monbuf, wbuf);
					if (ways_seen-- > 1) Strcat(monbuf, ", ");
					}
					if(youracedata == &mons[PM_SHARK] && has_blood_mon(mtmp) &&
						(mtmp)->mhp < (mtmp)->mhpmax && is_pool(u.ux, u.uy, TRUE) && is_pool((mtmp)->mx, (mtmp)->my, TRUE)){
					Sprintf(wbuf, "smell blood in the water");
					Strcat(monbuf, wbuf);
					if (ways_seen-- > 1) Strcat(monbuf, ", ");
					}
				}
		    }
			}
		}
	    }
		if(!do_halu){
			if(mtmp->data->msize == MZ_TINY) Sprintf(shapebuff, "a tiny");
			else if(mtmp->data->msize == MZ_SMALL) Sprintf(shapebuff, "a small");
			else if(mtmp->data->msize == MZ_HUMAN) Sprintf(shapebuff, "a human-sized");
			else if(mtmp->data->msize == MZ_LARGE) Sprintf(shapebuff, "a large");
			else if(mtmp->data->msize == MZ_HUGE) Sprintf(shapebuff, "a huge");
			else if(mtmp->data->msize == MZ_GIGANTIC) Sprintf(shapebuff, "a gigantic");
			else Sprintf(shapebuff, "an odd-sized");
			
			if((mtmp->data->mflagsb&MB_HEADMODIMASK) == MB_LONGHEAD) Strcat(shapebuff, " snouted");
			else if((mtmp->data->mflagsb&MB_HEADMODIMASK) == MB_LONGNECK) Strcat(shapebuff, " long-necked");
			
			if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == MB_ANIMAL) Strcat(shapebuff, " animal");
			else if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == MB_SLITHY) Strcat(shapebuff, " ophidian");
			else if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == MB_HUMANOID) Strcat(shapebuff, " humanoid");
			else if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == (MB_HUMANOID|MB_ANIMAL)) Strcat(shapebuff, " centauroid");
			else if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == (MB_HUMANOID|MB_SLITHY)) Strcat(shapebuff, " snake-legged humanoid");
			else if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == (MB_ANIMAL|MB_SLITHY)) Strcat(shapebuff, " snake-backed animal");
			else Strcat(shapebuff, " thing");
		} else {
			Sprintf(shapebuff, "%s%s%s", an(sizeStr[rn2(SIZE(sizeStr))]), headStr[rn2(SIZE(headStr))], bodyStr[rn2(SIZE(bodyStr))]);
		}
	}
    }
    else if (glyph_is_object(glyph)) {
	struct obj *otmp = vobj_at(x,y);

	if (!otmp || otmp->otyp != glyph_to_obj(glyph)) {
	    if (glyph_to_obj(glyph) != STRANGE_OBJECT) {
		otmp = mksobj(glyph_to_obj(glyph), FALSE, FALSE);
		if (otmp->oclass == COIN_CLASS)
		    otmp->quan = 2L; /* to force pluralization */
		else if (otmp->otyp == SLIME_MOLD)
		    otmp->spe = current_fruit;	/* give the fruit a type */
		Strcpy(buf, distant_name(otmp, xname));
		dealloc_obj(otmp);
	    }
	} else
	    Strcpy(buf, distant_name(otmp, xname));

	if (levl[x][y].typ == STONE || levl[x][y].typ == SCORR)
	    Strcat(buf, " embedded in stone");
	else if (IS_WALL(levl[x][y].typ) || levl[x][y].typ == SDOOR)
	    Strcat(buf, " embedded in a wall");
	else if (closed_door(x,y))
	    Strcat(buf, " embedded in a door");
	else if (is_pool(x,y, FALSE))
	    Strcat(buf, " in water");
	else if (is_lava(x,y))
	    Strcat(buf, " in molten lava");	/* [can this ever happen?] */
    } else if (glyph_is_trap(glyph)) {
	int tnum = what_trap(glyph_to_trap(glyph));
	Strcpy(buf, defsyms[trap_to_defsym(tnum)].explanation);
    } else if(!glyph_is_cmap(glyph)) {
	Strcpy(buf,"dark part of a room");
    } else switch(glyph_to_cmap(glyph)) {
    case S_altar:
	if(!In_endgame(&u.uz))
	    Sprintf(buf, "%s altar",
		align_str(Amask2align(levl[x][y].altarmask & ~AM_SHRINE)));
	else Sprintf(buf, "aligned altar");
	break;
    case S_ndoor:
	if (is_drawbridge_wall(x, y) >= 0)
	    Strcpy(buf,"open drawbridge portcullis");
	else if ((levl[x][y].doormask & ~D_TRAPPED) == D_BROKEN)
	    Strcpy(buf,"broken door");
	else
	    Strcpy(buf,"doorway");
	break;
    case S_cloud:
    case S_fog:
	Strcpy(buf, Is_airlevel(&u.uz) ? "cloudy area" : "fog/vapor cloud");
	break;
    case S_dust:
	Strcpy(buf, "dust storm");
	break;
	//Lethe patch by way of Slashem
    case S_water:
    case S_pool:
	Strcpy(buf, level.flags.lethe? "sparkling water" : "water");
	break;
    default:
	Strcpy(buf,defsyms[glyph_to_cmap(glyph)].explanation);
	break;
    }

    return ((mtmp && !do_halu) ? mtmp : (struct monst *) 0);
}

/*
 * Look in the "data" file for more info.  Called if the user typed in the
 * whole name (user_typed_name == TRUE), or we've found a possible match
 * with a character/glyph and flags.help is TRUE.
 *
 * NOTE: when (user_typed_name == FALSE), inp is considered read-only and
 *	 must not be changed directly, e.g. via lcase(). We want to force
 *	 lcase() for data.base lookup so that we can have a clean key.
 *	 Therefore, we create a copy of inp _just_ for data.base lookup.
 * 
 * Returns TRUE if it found an entry and printed to the nhwindow
 */
boolean
checkfile(inp, pm, user_typed_name, without_asking, printwindow)
    char *inp;
    struct permonst *pm;
    boolean user_typed_name, without_asking;
	winid *printwindow;
{
    dlb *fp;
    char buf[BUFSZ], newstr[BUFSZ];
    char *ep, *dbase_str;
    long txt_offset;
    int chk_skip;
    boolean found_in_file = FALSE, skipping_entry = FALSE, wrote = FALSE;

    fp = dlb_fopen(DATAFILE, "r");
    if (!fp) {
	pline("Cannot open data file!");
	return FALSE;
    }

    /* To prevent the need for entries in data.base like *ngel to account
     * for Angel and angel, make the lookup string the same for both
     * user_typed_name and picked name.
     */
    if (pm != (struct permonst *) 0 && !user_typed_name)
	dbase_str = strcpy(newstr, pm->mname);
    else dbase_str = strcpy(newstr, inp);
    (void) lcase(dbase_str);

    if (!strncmp(dbase_str, "interior of ", 12))
	dbase_str += 12;
    if (!strncmp(dbase_str, "a ", 2))
	dbase_str += 2;
    else if (!strncmp(dbase_str, "an ", 3))
	dbase_str += 3;
    else if (!strncmp(dbase_str, "the ", 4))
	dbase_str += 4;
    if (!strncmp(dbase_str, "tame ", 5))
	dbase_str += 5;
    else if (!strncmp(dbase_str, "peaceful ", 9))
	dbase_str += 9;
    if (!strncmp(dbase_str, "invisible ", 10))
	dbase_str += 10;
    if (!strncmp(dbase_str, "statue of ", 10))
	dbase_str[6] = '\0';
    else if (!strncmp(dbase_str, "figurine of ", 12))
	dbase_str[8] = '\0';

    /* Make sure the name is non-empty. */
    if (*dbase_str) {
	/* adjust the input to remove "named " and convert to lower case */
	char *alt = 0;	/* alternate description */

	if ((ep = strstri(dbase_str, " named ")) != 0)
	    alt = ep + 7;
	else
	    ep = strstri(dbase_str, " called ");
	if (!ep) ep = strstri(dbase_str, ", ");
	if (ep && ep > dbase_str) *ep = '\0';

	/*
	 * If the object is named, then the name is the alternate description;
	 * otherwise, the result of makesingular() applied to the name is. This
	 * isn't strictly optimal, but named objects of interest to the user
	 * will usually be found under their name, rather than under their
	 * object type, so looking for a singular form is pointless.
	 */

	if (!alt)
	    alt = makesingular(dbase_str);
	else
	    if (user_typed_name)
		(void) lcase(alt);

	/* skip first record; read second */
	txt_offset = 0L;
	if (!dlb_fgets(buf, BUFSZ, fp) || !dlb_fgets(buf, BUFSZ, fp)) {
	    impossible("can't read 'data' file");
	    (void) dlb_fclose(fp);
	    return FALSE;
	} else if (sscanf(buf, "%8lx\n", &txt_offset) < 1 || txt_offset <= 0)
	    goto bad_data_file;

	/* look for the appropriate entry */
	while (dlb_fgets(buf,BUFSZ,fp)) {
	    if (*buf == '.') break;  /* we passed last entry without success */

	    if (digit(*buf)) {
		/* a number indicates the end of current entry */
		skipping_entry = FALSE;
	    } else if (!skipping_entry) {
		if (!(ep = index(buf, '\n'))) goto bad_data_file;
		*ep = 0;
		/* if we match a key that begins with "~", skip this entry */
		chk_skip = (*buf == '~') ? 1 : 0;
		if (pmatch(&buf[chk_skip], dbase_str) ||
			(alt && pmatch(&buf[chk_skip], alt))) {
		    if (chk_skip) {
			skipping_entry = TRUE;
			continue;
		    } else {
			found_in_file = TRUE;
			break;
		    }
		}
	    }
	}
    }

    if(found_in_file) {
	long entry_offset;
	int  entry_count;
	int  i;

	/* skip over other possible matches for the info */
	do {
	    if (!dlb_fgets(buf, BUFSZ, fp)) goto bad_data_file;
	} while (!digit(*buf));
	if (sscanf(buf, "%ld,%d\n", &entry_offset, &entry_count) < 2) {
bad_data_file:	impossible("'data' file in wrong format");
		(void) dlb_fclose(fp);
		return FALSE;
	}

	if (user_typed_name || without_asking || yn("More info?") == 'y') {

	    if (dlb_fseek(fp, txt_offset + entry_offset, SEEK_SET) < 0) {
		pline("? Seek error on 'data' file!");
		(void) dlb_fclose(fp);
		return FALSE;
	    }
		char *encyc_header = "Encyclopedia entry:";
		putstr(*printwindow, 0, "\n");
		putstr(*printwindow, 0, encyc_header);
		putstr(*printwindow, 0, "\n");
	    for (i = 0; i < entry_count; i++) {
		if (!dlb_fgets(buf, BUFSZ, fp)) goto bad_data_file;
		if ((ep = index(buf, '\n')) != 0) *ep = 0;
		if (index(buf+1, '\t') != 0) (void) tabexpand(buf+1);
		putstr(*printwindow, 0, buf + 1);
		wrote = TRUE;
	    }
	}
    } else if (user_typed_name)
	pline("I don't have any information on those things.");

    (void) dlb_fclose(fp);
	return wrote;
}

/* getpos() return values */
#define LOOK_TRADITIONAL	0	/* '.' -- ask about "more info?" */
#define LOOK_QUICK		1	/* ',' -- skip "more info?" */
#define LOOK_ONCE		2	/* ';' -- skip and stop looping */
#define LOOK_VERBOSE		3	/* ':' -- show more info w/o asking */

/* also used by getpos hack in do_name.c */
const char what_is_an_unknown_object[] = "an unknown object";

static const char * const bogusobjects[] = {
       /* Real */
	   "eleven arrow"
	   "ortish arrow",
       "ruined arrow",
       "vulgar arrow",
       "ja",
       "bamboozled sparrow",
       "crossbow colt",
	   "crossbow dolt",
	   "throwing fart",
       "shurunken",
	   "bomberang",
       "elven drear",
       "orcish dear",
       "dwarvish peer",
       "sliver spear",
       "throwing spar",
       "2x3dent", /*Homestuck*/
       "danger",
       "elven danger",
       "orcish danger",
       "sliver danger",
	   "allthame",
       "scalp",
       "stiletto",
	   "silletto",
       "criesknife",
       "old battle-axe",
       "length-challenged sword",
       "re-curved sword",
       "de-curved sword",
       "circular sword",
       "longer sword",
       "three-handed sword",
       "two-handled sword",
       "catana",
       "dunesword",
       "bi-partisan",
       "parisian",
       "sniptem",	/*Order of the Stick*/
       "fave",
       "rancor",
       "lancer",
       "halbeard",
       "snake-eyed bardiche",
       "vouge",
       "dwarvish hassock",
       "fauchard",
       "mysarme",
       "uisarme",
       "bob-guisarme",
       "lucifer's hammer",
       "bec de corwin",
       "yet another poorly-differentiated polearm",
       "cursed YAPDP",
       "can of mace",
       "evening star",
       "dawn star",
       "day star",
       "peace hammer",
       "wooden sandwich",
       "halfstaff",
       "thong club",
       "dire flail",
       "dire fail",
       "trollwhip",
       "partially-eaten yumi",
       "X-bow",
       "conical flat",
       "mithril-boat",
       "barded mail",
       "yet-another-substandard-mail-variant",
       "cursed YASMV",
       "scale-reinforced banded splint mail with chain joints",
       "leather studd armor",
       "white shirt",
       "red shirt",
       "pair of brown pants",
       "undershirt",
       "cope [sic]",
       "cope [with it]",
       "lion skin",
       "lion sin",
       "bear skin robe",
       "bear skin rug",
       "undersized shield",
       "shield of reflection",
       "shield of rarefaction",
       "wide-eyed shield",
       "pair of padding gloves",
       "pair of rideable gloves",
       "pair of feching gloves",
       "pair of wandering walking shoes",
       "pair of walkabout shoes",
       "pair of hardly shoes",
       "pair of jackboots",
       "pair of combat boats",
       "pair of hiking boots",
       "pair of wild hiking boots",
       "pair of muddy boots",
       "gnomerang",
       "gnagger",
       "hipospray ampule",
	   
	   "can of Greece",
	   "can of crease",
	   "can of Grease",
	   "can of greaser",
	   
	   "cursed boomerang axe named \"The Axe of Dungeon Collapse\"",
		
       "blessed greased +5 silly object of hilarity",
	   "kinda lame joke",
	   
	   "brazier of commanding fire elementals",
	   "brassiere of commanding fire elementals",
	   
	   "bowl of commanding water elementals",
	   "bowel of commanding water elementals",
	   
	   "censer of controlling air elementals",
	   "censure of controlling air elementals",
	   
	   "stone of controlling earth elementals",
	   "loan of controlling earth elementals",
	   
# ifdef TOURIST
       /* Modern */
       "polo mallet",
       "string vest",
       "applied theology textbook",        /* AFutD */
       "handbag",
       "onion ring",
       "tuxedo",
       "breath mint",
       "potion of antacid",
       "traffic cone",
       "chainsaw",
//	   "pair of high-heeled stilettos",    /* the *other* stiletto */
	   "high-heeled stiletto",
	   "comic book",
	   "lipstick",
       "dinner-jacket",
       "three-piece suit",

       /* Silly */
       "left-handed iron chain",
       "holy hand grenade",                /* Monty Python */
       "decoder ring",
       "amulet of huge gold chains",       /* Foo' */
       "rubber Marduk",
       "unicron horn",                     /* Transformers */
       "holy grail",                       /* Monty Python */
       "chainmail bikini",
	   "leather shorts",
       "first class one-way ticket to Albuquerque", /* Weird Al */
       "yellow spandex dragon scale mail", /* X-Men */

       /* Musical Instruments */
       "grand piano",
       "set of two slightly sampled electric eels", /* Oldfield */
       "kick drum",                        /* 303 */
       "tooled airhorn",

       /* Pop Culture */
       "flux capacitor",                   /* BTTF */
       "Walther PPK",                      /* Bond */
       "hanging chad",                     /* US Election 2000 */
       "99 red balloons",                  /* 80s */
       "pincers of peril",                 /* Goonies */
       "ring of schwartz",                 /* Spaceballs */
       "signed copy of Diaspora",          /* Greg Egan */
       "file containing the missing evidence in the Kelner case", /* Naked Gun */
       "blessed 9 helm of Des Lynam",     /* Bottom */
	   "oscillation overthruster",
	   "magic device",
	   
	   "Infinity Gauntlet",

        /* Geekery */
       "AAA chipset",                      /* Amiga */
       "thoroughly used copy of Nethack for Dummies",
       "named pipe",                       /* UNIX */
       "kernel trap",
       "copy of nethack 3.4.4",            /* recursion... */
       "YAFM", "YAAP", "YANI", "YAAD", "YASD", "YAFAP", "malevolent RNG", /* rgrn */
       "cursed smooth manifold",           /* Topology */
       "vi clone",
       "maximally subsentient emacs mode",
       "bongard diagram",                  /* Intelligence test */
	   
	   /* Movies etc. */
	   "overloading phaser", /* Star Trek */
	   "thermal detinator", /* Star Wars */
       "thing that is not tea",/*"no tea here!", "no tea, sadly",*/ /* HGttG */
		"potion almost, but not quite, entirely unlike tea",
		"potion of Pan-Galactic Gargle Blaster", "black scroll-case labeled DON'T PANIC", /* HGttG */
	   "ridiculously dangerous epaulet [it's armed]", /* Schlock Mercenary*/
	   "pokeball", /* Pokemon */
	   "scroll of oxygen-destroyer plans", /* Godzilla */
	   "hologram of Death Star plans",
	   "sonic screwdriver", /* Dr. Who */

	   /* British goodness */
       "bum bag",
       "blessed tin of marmite",
       "tesco value potion",
       "ringtone of drawbridge opening",
       "burberry cap",
       "potion of bitter",
       "cursed -2 bargain plane ticket to Ibiza",
       "black pudding corpse",
# endif
       /* Fantasy */
       "leaf of pipe weed",                /* LOTR */
       "knife missile",                    /* Iain M. Banks */
       "large gem",                        /* Valhalla */
       "ring of power",                    /* LOTR */
       "silmaril",                         /* LOTR */
       "pentagram of protection",          /* Quake */
	   "crown of swords",					/* Wheel of Time */
	   
	   /* Interwebs */
	   "memetic kill agent",				/* SCP Foundation */
	   "bottle of squid pro quo ink",		/* MSPA */
		"highly indulgent self-insert",
	   "cursed -1 phillips head",			/* xkcd nethack joke */
	   
	   /* Mythology */
	   "sampo",
	   "shamir",
	   "golden fleece",
	   "gold apple labeled \"For the Fairest\"",
	   "Holy Grail",
	   "Ark of the Covenant",
	   
	   /* Other Games */
	   "Chaos Emerald",
	   "bronze sphere",
	   "modron cube",
	   
	   /* Books */
       "monster manual",                   /* D&D */
       "monster book of monsters",         /* Harry Potter */
	   "Codex of the Infinite Planes",	   /* DnD */
	   "spellbook of Non-Conduit Transdimensional Fabric Fluxes and Real-Time Inter-dimensional Matrix Transformations", /* Maldin's Greyhawk */
		   "spellbook named the Codex of Betrayal",
		   "spellbook named the Demonomicon of Iggwilv",
		   "spellbook named the Book of Keeping",
		   "Black Scroll of Ahm",
	   "Elder Scroll", /*the Elder Scrolls*/
       "spellbook named The Ta'ge Fragments", /* Cthulhutech */
       "spellbook named Tome of Eternal Darkness", /* Eternal Darkness */
       "history book called A Chronicle of the Daevas", /* SCP Foundation */
       "spellbook named The Book of Sand",                     /* Jorge Luis Borges */
       "spellbook named Octavo",       	   /* Discworld */
		"spellbook called Necrotelicomnicon",
	   "lost copy of The Nice and Accurate Prophecies of Anges Nutter", /* Good Omens */
	   "spellbook of Addition Made Simple",
	   "spellbook of Noncommutative Hyperdimensional Geometry Made (Merely!) Complicated",
	   "heavily obfuscated spellbook",
	   "scroll of abstruse logic",
			"scroll of identity", /*deepy*/
			"scroll of cure blindness",
	   "spellbook of sub-formal introtransreductive logic [for Dummies!]",
	   "spellbook named A Brief History of Time",
	   "spellbook named The Book of Eibon", 					/* Clark Ashton Smith */
	   "playbook named The King in Yellow", 					/* Robert W. Chambers */
	   "playbook called the Scottish play", 					/* aka Macbeth */
	   "spellbook named De Vermis Mysteriis",					/* Robert Bloch */
	   "HANDBOOK FOR THE IMMINENTLY DECEASED named ~ATH",		/* Homestuck */
	   "spellbook named Necronomicon",							/* HP Lovecraft (yes, I know there is a game artifact of this name) */
	   "spellbook named Al Azif",								/* "Arabic" name for the Necronomicon, HP Lovecraft */
	   "spellbook named Unaussprechlichen Kulten",				/*  Robert E. Howard; Lovecraft and Derleth */
	   "spellbook called Nameless Cults",						/*  (the original name) */
	   "spellbook called Unspeakable Cults",					/*  One of the two things "Unaussprechlichen" translates to */
	   "spellbook called Unpronouncable Cults",					/*  The other thing "Unaussprechlichen" translates to */
	   "spellbook named The Diary of Drenicus the Wise",		/*  Dicefreaks, The Gates of Hell */
	   "spellbook named Clavicula Salomonis Regis",				/* ie, The Lesser Key of Solomon */
	   "copy of The Five Books of Moses",						/* aka the Torah */
	   "spellbook named The Six and Seventh Books of Moses",	/* 18th- or 19th-century magical text allegedly written by Moses */
	   "spellbook named The Book of Coming Forth by Day", "spellbook named The Book of emerging forth into the Light",
	   "spellbook named Sepher Ha-Razim",						/*  Book given to Noah by the angel Raziel */
	   "spellbook named Sefer Raziel HaMalakh", 				/* Book of Raziel the Angel, given to Adam */
	   "spellbook named The Testament of Solomon",
	   "spellbook named The Book of Enoch",
	   "spellbook named The Book of Inverted Darkness",
	   "spellbook named The Uruk Tablets",
	   "spellbook named The Book of the Law",
	   "spellbook named the Book of Shadows",
	   "spellbook named the Book of Mirrors",
	   "book called the Gospel of the Thricefold Exile",
       "dead sea scroll",
	   "mayan codex",
	   "simple textbook on the zoologically dubious",			/* Homestuck */
	   "spellbook named The Book of Night with Moon",			/* So you want to be a Wizard? */
	   "spellbook named The Grimmerie",			/* Wicked */
	   "book of horrors", /* MLP */
	   "book of Remembrance and Forgetting", /* the Old Kingdom */

       /* Historical */
	   "Amarna letter",
	   "lost ark of the covenant",
       "cat o'nine tails",
       "piece of eight",
       "codpiece",
       "straight-jacket",
       "bayonet",
       "iron maiden",
       "oubliette",
       "pestle and mortar",
       "plowshare",
	   "petard",
	   "caltrop",

       /* Mashups */
       "potion of rebigulation",           /* Simpsons */
       "potion of score doubling",
	   "potion of bad breath",
	   "potion of fire breathing",
	   "potion of intoxication",
	   "potion of immorality",
	   "potion of immoratlity",
	   "potion of gain divinity",
       "scroll labeled ED AWK YACC",      /* the standard scroll */
       "scroll labeled RTFM",
       "scroll labeled KLAATU BARADA NIKTO", /* Evil Dead 3 */
       "scroll of omniscience",
       "scroll of mash keyboard",
       "scroll of RNG taming",
       "scroll of fungicide",
	   "scroll of apocalypse",
	   "scroll of profits",
	   "scroll of stupidity",
	   "spellbook of firebear",
	   "spellbook of dip",
	   "spellbook of cone of cord",
       "helm of telemetry",
       "helm of head-butting",
       "pair of blue suede boots of charisma",
	   "pair of boots of rug-cutting",
	   "pair of cursed boots of mazing",
       "pair of wisdom boots",
       "cubic zirconium",
       "amulet of instadeath",
       "amulet of bad luck",
       "amulet of refraction",
       "amulet of deflection",
       "amulet of rarefaction",
       "amulet versus YASD",
       "O-ring",
       "ring named Frost Band",
       "expensive exact replica of the Amulet of Yendor",
       "giant beatle",
       "lodestone",
       "rubber chicken",                   /* c corpse */
       "figurine of a god",
       "tin of Player meat",
       "tin of whoop ass",
	   "ragnarok horn",
       "cursed -3 earring of adornment",
       "ornamental cape",
       "acid blob skeleton",
       "wand of washing",
	   "wand of intoxication",
	   "wand of vaporization",
	   "wand of disruption",
	   "wand of transubstantiation",
	   "wand of disintegration",
	   "wand of renewal",
	   
	   "little piece of home",

# ifdef MAIL
       "brand new, all time lowest introductory rate special offer",
       "tin of spam",
# endif
       "dirty rag"
};

/* Return a random bogus object name, for hallucination */
const char *
rndobjnam()
{
    int name;
	if(!rn2(3)){
		name = rn2(SIZE(bogusobjects));
		return bogusobjects[name];
//	} else if(!rn2(2)){
//		name = rn2(TIN);
//		return OBJ_DESCR(objects[name]);
	} else{
		name = rn2(TIN);
		return OBJ_NAME(objects[name]);
	}
}


STATIC_OVL int
do_look(quick)
    boolean quick;	/* use cursor && don't search for "more info" */
{
    char    out_str[BUFSZ], look_buf[BUFSZ];
    const char *x_str, *firstmatch = 0;
    struct permonst *pm = 0;
    struct monst *mtmp = 0;
    int     i, ans = 0;
    glyph_t sym;		/* typed symbol or converted glyph */
    int	    found;		/* count of matching syms found */
    coord   cc;			/* screen pos of unknown glyph */
    boolean save_verbose;	/* saved value of flags.verbose */
    boolean from_screen;	/* question from the screen */
    boolean force_defsyms;	/* force using glyphs from defsyms[].sym */
    boolean need_to_look;	/* need to get explan. from glyph */
    boolean hit_trap;		/* true if found trap explanation */
    int skipped_venom;		/* non-zero if we ignored "splash of venom" */
	int hallu_obj;		/* non-zero if found hallucinable object */
    static const char *mon_interior = "the interior of a monster";

    if (quick) {
	from_screen = TRUE;	/* yes, we want to use the cursor */
    } else {
	i = ynq("Specify unknown object by cursor?");
	if (i == 'q') return 0;
	from_screen = (i == 'y');
    }

    if (from_screen) {
	cc.x = u.ux;
	cc.y = u.uy;
	sym = 0;		/* gcc -Wall lint */
    } else {
	getlin("Specify what? (type the word)", out_str);
	if (out_str[0] == '\0' || out_str[0] == '\033')
	    return 0;

	if (out_str[1]) {	/* user typed in a complete string */
		winid datawin = create_nhwindow(NHW_MENU);
	    if(checkfile(out_str, pm, TRUE, TRUE, &datawin))
			display_nhwindow(datawin, TRUE);
		destroy_nhwindow(datawin);
	    return 0;
	}
	sym = out_str[0];
    }

    /* Save the verbose flag, we change it later. */
    save_verbose = flags.verbose;
    flags.verbose = flags.verbose && !quick;
    /*
     * The user typed one letter, or we're identifying from the screen.
     */
    do {
	/* Reset some variables. */
	need_to_look = FALSE;
	pm = (struct permonst *)0;
	skipped_venom = 0;
	hallu_obj = 0;
	found = 0;
	out_str[0] = '\0';

	if (from_screen) {
	    int glyph;	/* glyph at selected position */

	    if (flags.verbose)
		pline("Please move the cursor to %s.",
		       what_is_an_unknown_object);
	    else
		pline("Pick an object.");

	    ans = getpos(&cc, quick, what_is_an_unknown_object);
	    if (ans < 0 || cc.x < 0) {
		flags.verbose = save_verbose;
		return 0;	/* done */
	    }
	    flags.verbose = FALSE;	/* only print long question once */

	    /* Convert the glyph at the selected position to a symbol. */
	    glyph = glyph_at(cc.x,cc.y);
	    if (glyph_is_cmap(glyph)) {
		sym = showsyms[glyph_to_cmap(glyph)];
	    } else if (glyph_is_trap(glyph)) {
		sym = showsyms[trap_to_defsym(glyph_to_trap(glyph))];
	    } else if (glyph_is_object(glyph)) {
		sym = oc_syms[(int)objects[glyph_to_obj(glyph)].oc_class];
		if (sym == '`' && iflags.bouldersym && (int)glyph_to_obj(glyph) == BOULDER)
			sym = iflags.bouldersym;
	    } else if (glyph_is_monster(glyph)) {
		/* takes care of pets, detected, ridden, and regular mons */
		sym = monsyms[(int)mons[glyph_to_mon(glyph)].mlet];
	    } else if (glyph_is_swallow(glyph)) {
		sym = showsyms[glyph_to_swallow(glyph)+S_sw_tl];
	    } else if (glyph_is_invisible(glyph)) {
		sym = DEF_INVISIBLE;
	    } else if (glyph_is_warning(glyph)) {
		sym = glyph_to_warning(glyph);
	    	sym = warnsyms[sym];
	    } else {
		impossible("do_look:  bad glyph %d at (%d,%d)",
						glyph, (int)cc.x, (int)cc.y);
		sym = ' ';
	    }
	}

	/*
	 * Check all the possibilities, saving all explanations in a buffer.
	 * When all have been checked then the string is printed.
	 */

	/* Check for monsters */
	for (i = 0; i < MAXMCLASSES; i++) {
	    if (sym == (from_screen ? monsyms[i] : def_monsyms[i]) &&
		monexplain[i]) {
		need_to_look = TRUE;
		if (!found) {
		    Sprintf(out_str, "%c       %s", (uchar)sym, an(monexplain[i]));
		    firstmatch = monexplain[i];
		    found++;
		} else {
		    found += append_str(out_str, an(monexplain[i]));
		}
	    }
	}
	/* handle '@' as a special case if it refers to you and you're
	   playing a character which isn't normally displayed by that
	   symbol; firstmatch is assumed to already be set for '@' */
	if ((from_screen ?
		(sym == monsyms[S_HUMAN] && cc.x == u.ux && cc.y == u.uy) :
		(sym == def_monsyms[S_HUMAN] && !iflags.showrace)) &&
	    !(Race_if(PM_HUMAN) || Race_if(PM_INHERITOR) || Race_if(PM_ELF) || Race_if(PM_DROW) || Race_if(PM_MYRKALFR)) && !Upolyd)
	    found += append_str(out_str, "you");	/* tack on "or you" */

	/*
	 * Special case: if identifying from the screen, and we're swallowed,
	 * and looking at something other than our own symbol, then just say
	 * "the interior of a monster".
	 */
	if (u.uswallow && from_screen && is_swallow_sym(sym)) {
	    if (!found) {
		Sprintf(out_str, "%c       %s", (uchar)sym, mon_interior);
		firstmatch = mon_interior;
	    } else {
		found += append_str(out_str, mon_interior);
	    }
	    need_to_look = TRUE;
	}

	/* Now check for objects */
	for (i = 1; i < MAXOCLASSES; i++) {
	    if (sym == (from_screen ? oc_syms[i] : def_oc_syms[i])) {
		need_to_look = TRUE;
		if (from_screen && i == VENOM_CLASS) {
		    skipped_venom++;
		    continue;
		}
		if (!found) {
		    Sprintf(out_str, "%c       %s", (uchar)sym, an(objexplain[i]));
		    firstmatch = objexplain[i];
		    found++;
			hallu_obj++;
		} else {
		    found += append_str(out_str, an(objexplain[i]));
		}
	    }
	}

	if (sym == DEF_INVISIBLE) {
	    if (!found) {
		Sprintf(out_str, "%c       %s", (uchar)sym, an(invisexplain));
		firstmatch = invisexplain;
		found++;
	    } else {
		found += append_str(out_str, an(invisexplain));
	    }
	}

#define is_cmap_trap(i) ((i) >= S_arrow_trap && (i) <= S_mummy_trap)
#define is_cmap_drawbridge(i) ((i) >= S_vodbridge && (i) <= S_hcdbridge)

	/* Now check for graphics symbols */
	for (hit_trap = FALSE, i = 0; i < MAXPCHARS; i++) {
	    x_str = defsyms[i].explanation;
	    if (sym == (from_screen ? showsyms[i] : defsyms[i].sym) && *x_str) {
		/* avoid "an air", "a water", or "a floor of a room" */
		int article = (i == S_drkroom || i == S_litroom) ? 2 :		/* 2=>"the" */
			      !(strcmp(x_str, "air") == 0 ||	/* 1=>"an"  */
				strcmp(x_str, "shallow water") == 0 || /* 0=>(none)*/
				strcmp(x_str, "water") == 0);

		if (!found) {
		    if (is_cmap_trap(i)) {
			Sprintf(out_str, "%c       a trap", (uchar)sym);
			hit_trap = TRUE;
		    } else if (level.flags.lethe && !strcmp(x_str, "water")) { //Lethe patch
			Sprintf(out_str, "%c       sparkling water", (uchar)sym); //Lethe patch
		    } else {
			Sprintf(out_str, "%c       %s", (uchar)sym,
				article == 2 ? the(x_str) :
				article == 1 ? an(x_str) : x_str);
		    }
		    firstmatch = x_str;
		    found++;
		} else if (!u.uswallow && !(hit_trap && is_cmap_trap(i)) &&
			   !(found >= 3 && is_cmap_drawbridge(i))) {
		    if (level.flags.lethe && !strcmp(x_str, "water")) //lethe
			found += append_str(out_str, "sparkling water"); //lethe
		    else //lethe
		    	found += append_str(out_str,
					article == 2 ? the(x_str) :
					article == 1 ? an(x_str) : x_str);
		    if (is_cmap_trap(i)) hit_trap = TRUE;
		}

		if (i == S_altar || is_cmap_trap(i))
		    need_to_look = TRUE;
	    }
	}

	/* Now check for warning symbols */
	for (i = 1; i < WARNCOUNT; i++) {
	    x_str = def_warnsyms[i].explanation;
	    if (sym == (from_screen ? warnsyms[i] : def_warnsyms[i].sym)) {
		if (!found) {
			Sprintf(out_str, "%c       %s",
				(uchar)sym, def_warnsyms[i].explanation);
			firstmatch = def_warnsyms[i].explanation;
			found++;
		} else {
			found += append_str(out_str, def_warnsyms[i].explanation);
		}
		/* Kludge: warning trumps boulders on the display.
		   Reveal the boulder too or player can get confused */
		if (from_screen && boulder_at(cc.x, cc.y))
			Strcat(out_str, " co-located with a large object");
		break;	/* out of for loop*/
	    }
	}
    
	/* if we ignored venom and list turned out to be short, put it back */
	if (skipped_venom && found < 2) {
	    x_str = objexplain[VENOM_CLASS];
	    if (!found) {
		Sprintf(out_str, "%c       %s", (uchar)sym, an(x_str));
		firstmatch = x_str;
		found++;
	    } else {
		found += append_str(out_str, an(x_str));
	    }
	}

	/* handle optional boulder symbol as a special case */ 
	if (iflags.bouldersym && sym == iflags.bouldersym) {
	    if (!found) {
		firstmatch = "boulder";
		Sprintf(out_str, "%c       %s", (uchar)sym, an(firstmatch));
		found++;
	    } else {
		found += append_str(out_str, "boulder");
	    }
	}
	
	/*
	 * If we are looking at the screen, follow multiple possibilities or
	 * an ambiguous explanation by something more detailed.
	 */
	if (from_screen) {
	    if (hallu_obj && Hallucination) {
			char temp_buf[BUFSZ];
			Sprintf(temp_buf, " (%s)", rndobjnam());
			(void)strncat(out_str, temp_buf, BUFSZ-strlen(out_str)-1);
	    } else if (found > 1 || need_to_look) {

		char monbuf[BUFSZ];
		char shapebuf[BUFSZ];
		char temp_buf[BUFSZ];

		mtmp = lookat(cc.x, cc.y, look_buf, monbuf, shapebuf);
		firstmatch = look_buf;
		if (*firstmatch) {
			if(shapebuf[0]){
				Sprintf(temp_buf, " (%s", firstmatch);
				(void)strncat(out_str, temp_buf, BUFSZ-strlen(out_str)-1);
				Sprintf(temp_buf, ", %s)", shapebuf);
				(void)strncat(out_str, temp_buf, BUFSZ-strlen(out_str)-1);
				found = 1;	/* we have something to look up */
			}
			else {
				Sprintf(temp_buf, " (%s)", firstmatch);
				(void)strncat(out_str, temp_buf, BUFSZ-strlen(out_str)-1);
				found = 1;	/* we have something to look up */
			}
		}
		if (monbuf[0]) {
		    Sprintf(temp_buf, " [seen: %s]", monbuf);
		    (void)strncat(out_str, temp_buf, BUFSZ-strlen(out_str)-1);
		}
		if (mtmp != (struct monst *) 0) {
			strcat(out_str, "\n");
			temp_buf[0] = '\0';
			get_description_of_monster_type(mtmp, temp_buf);
			(void)strncat(out_str, temp_buf, BUFSZ - strlen(out_str) - 1);
		}
		}
	}

	/* Finally, print out our explanation. */
	if (found) {
		winid datawin = create_nhwindow(NHW_MENU);
		char * temp_print;
		temp_print = strtok(out_str, "\n");
		while (temp_print != NULL)
		{
			putstr(datawin, 0, temp_print);
			temp_print = strtok(NULL, "\n");
		}
	    /* check the data file for information about this thing */
	    if (found == 1 && ans != LOOK_QUICK && ans != LOOK_ONCE &&
			(ans == LOOK_VERBOSE || (flags.help && !quick))) {
		char temp_buf[BUFSZ];
		Strcpy(temp_buf, level.flags.lethe //lethe
					&& !strcmp(firstmatch, "water")?
				"lethe" : firstmatch);
		(void)checkfile(temp_buf, pm, FALSE, (boolean)(ans == LOOK_VERBOSE), &datawin);
	    }
		display_nhwindow(datawin, TRUE);
		destroy_nhwindow(datawin);
	} else {
	    pline("I've never heard of such things.");
	}

    } while (from_screen && !quick && ans != LOOK_ONCE);

    flags.verbose = save_verbose;
    return 0;
}


int
append(char * buf, int condition, char * text, boolean many)
{
	if (condition) {
		if (buf != NULL) {
			if (many) {
				(void)strcat(buf, ", ");
			}
			(void)strcat(buf, text);
		}
		return many + 1;
	}
	return many;
}
int
appendgroup(char * buf, int condition, char * primer, char * text, boolean many, boolean group)
{
	if (condition) {
		if (buf != NULL) {
			if (group) {
				(void)strcat(buf, "/");
			}
			else {
				if (many)
					(void)strcat(buf, ", ");
				(void)strcat(buf, primer);
				(void)strcat(buf, " ");
			}
			(void)strcat(buf, text);
		}
		return group + 1;
	}
	return group;
}

int
generate_list_of_resistances(struct monst * mtmp, char * temp_buf, int resists)
{
	unsigned int mr_flags;
	unsigned long mg_flags = mtmp->data->mflagsg;
	if (resists == 1){
		mr_flags = mtmp->data->mresists;
		if(mtmp->mfaction == ZOMBIFIED){
			mr_flags |= MR_COLD | MR_SLEEP | MR_POISON | MR_DRAIN;
			mg_flags |= MG_RPIERCE | MG_RBLUNT;
		}
		if(mtmp->mfaction == SKELIFIED){
			mr_flags |= MR_COLD | MR_SLEEP | MR_POISON | MR_DRAIN;
			mg_flags |= MG_RPIERCE | MG_RSLASH;
		}
		if(mtmp->mfaction == CRYSTALFIED){
			mr_flags |= MR_COLD | MR_SLEEP | MR_POISON | MR_DRAIN;
			mg_flags |= MG_RPIERCE | MG_RSLASH;
		}
		if(mtmp->mfaction == VAMPIRIC){
			mr_flags |= MR_SLEEP | MR_POISON | MR_DRAIN;
			if(mtmp->m_lev > 10) mr_flags |= MR_COLD;
		}
		if(mtmp->mfaction == PSEUDONATURAL){
			mr_flags |= MR_POISON;
		}
		if(mtmp->mfaction == TOMB_HERD){
			mr_flags |= MR_FIRE|MR_COLD|MR_SLEEP|MR_STONE|MR_DRAIN|MR_POISON|MR_SICK|MR_MAGIC;
		}
	}
	if (resists == 0)
		mr_flags = mtmp->data->mconveys;
	int many = 0;
	many = append(temp_buf, (mr_flags & MR_FIRE), "fire", many);
	many = append(temp_buf, (mr_flags & MR_COLD), "cold", many);
	many = append(temp_buf, (mr_flags & MR_SLEEP), "sleep", many);
	many = append(temp_buf, (mr_flags & MR_DISINT), "disintegration", many);
	many = append(temp_buf, (mr_flags & MR_ELEC), "electricity", many);
	many = append(temp_buf, (mr_flags & MR_POISON), "poison", many);
	many = append(temp_buf, (mr_flags & MR_ACID), "acid", many);
	many = append(temp_buf, (mr_flags & MR_STONE), "petrification", many);
	many = append(temp_buf, (mr_flags & MR_DRAIN), "level drain", many);
	many = append(temp_buf, (mr_flags & MR_SICK), "sickness", many);
	many = append(temp_buf, (mr_flags & MR_MAGIC), "magic", many);
	many = append(temp_buf, (((mg_flags & MG_WRESIST) != 0L) && resists == 1), "weapon attacks", many);
	many = append(temp_buf, (((mg_flags & MG_RBLUNT ) != 0L) && resists == 1), "blunt", many);
	many = append(temp_buf, (((mg_flags & MG_RSLASH ) != 0L) && resists == 1), "slashing", many);
	many = append(temp_buf, (((mg_flags & MG_RPIERCE) != 0L) && resists == 1), "piercing", many);
	//many = append(temp_buf, ((mg_flags & MG_RALL) == MG_RALL), "blunt & slashing & piercing", many);
	return many;
}

char *
get_generation_description_of_monster_type(struct monst * mtmp, char * temp_buf)
{
	struct permonst * ptr = mtmp->data;
	int many = 0;

	many = append(temp_buf, !(ptr->geno & G_NOGEN), "Normal generation", many);
	many = append(temp_buf, (ptr->geno & G_NOGEN), "Special generation", many);
	many = append(temp_buf, (ptr->geno & G_UNIQ), "unique", many);
	many = 0;
	many = append(temp_buf, (ptr->geno & G_SGROUP), " in groups", many);
	many = append(temp_buf, (ptr->geno & G_LGROUP), " in large groups", many);
	if ((ptr->geno & G_NOGEN) == 0) {
		char frequency[BUFSZ] = "";
		sprintf(frequency, ", with frequency %d.", (ptr->geno & G_FREQ));
		strcat(temp_buf, frequency);
	}
	else {
		strcat(temp_buf, ".");
	}
	return temp_buf;
}

char *
get_weight_description_of_monster_type(struct monst * mtmp, char * temp_buf)
{
	struct permonst * ptr = mtmp->data;

	sprintf(temp_buf, "Weight = %d. Nutrition = %d.", ptr->cwt, ptr->cnutrit);

	return temp_buf;
}

char *
get_resistance_description_of_monster_type(struct monst * mtmp, char * description)
{
	struct permonst * ptr = mtmp->data;
	char temp_buf[BUFSZ] = "";
	temp_buf[0] = '\0';
	int count = generate_list_of_resistances(mtmp, temp_buf, 1);

	if (species_reflects(mtmp))
		strcat(description, "Reflects. ");

	if (count == 0) {
		strcat(description, "No resistances.");
	}
	else {
		strcat(description, "Resists ");
		strcat(description, temp_buf);
		strcat(description, ".");
	}
	return description;
}

char *
get_weakness_description_of_monster_type(struct monst * mtmp, char * description)
{
	struct permonst * ptr = mtmp->data;
	char temp_buf[BUFSZ] = "";
	temp_buf[0] = '\0';
	int many = 0;

	many = append(temp_buf, hates_holy_mon(mtmp)	, "holy"		, many);
	many = append(temp_buf, hates_unholy_mon(mtmp)	, "unholy"		, many);
	many = append(temp_buf, hates_silver(ptr)		, "silver"		, many);
	many = append(temp_buf, hates_iron(ptr)			, "iron"		, many);

	if (many) {
		strcat(description, "Weak to ");
		strcat(description, temp_buf);
		strcat(description, ".");
	}
	return description;
}

char *
get_conveys_description_of_monster_type(struct monst * mtmp, char * description)
{
	struct permonst * ptr = mtmp->data;
	char temp_buf[BUFSZ] = "";
	temp_buf[0] = '\0';
	int count = generate_list_of_resistances(mtmp, temp_buf, 0);
	if ((ptr->geno & G_NOCORPSE) != 0 || mtmp->mfaction == SKELIFIED || mtmp->mfaction == CRYSTALFIED) {
		strcat(description, "Leaves no corpse. ");
	}
	else if (count == 0) {
		strcat(description, "Corpse conveys no resistances. ");
	}
	else {
		strcat(description, "Corpse conveys ");
		strcat(description, temp_buf);
		if (count == 1) {
			strcat(description, " resistance. ");
		}
		else {
			strcat(description, " resistances. ");
		}
	}

	return description;
}

char *
get_mm_description_of_monster_type(struct monst * mtmp, char * description)
{
	struct permonst * ptr = mtmp->data;
	strcat(description, "Movement: ");
	int many = 0;
	many = append(description, notonline(ptr)			, "avoids you"			, many);
	many = append(description, fleetflee(ptr)			, "flees"				, many);
	many = append(description, species_flies(ptr)	, "flies"				, many);
	many = append(description, species_floats(ptr)	, "floats"				, many);
	many = append(description, is_clinger(ptr)			, "clings to ceilings"	, many);
	many = append(description, species_swims(ptr)	, "swims"				, many);
	many = append(description, breathless_mon(mtmp)		, "is breathless"		, many);
	many = append(description, amphibious(ptr)			, "survives underwater"	, many);
	many = append(description, species_passes_walls(ptr), "phases"				, many);
	many = append(description, amorphous(ptr)			, "squeezes in gaps"	, many);
	many = append(description, tunnels(ptr)				, "tunnels"				, many);
	many = append(description, needspick(ptr)			, "digs"				, many);
	many = append(description, species_teleports(ptr), "teleports"			, many);
	many = append(description, species_controls_teleports(ptr)	, "controls teleports"	, many);
	many = append(description, mteleport(ptr)			, "teleports often"		, many);
	many = append(description, stationary(ptr)			, "stationary"			, many);
	many = append(description, (many==0)				, "moves normally"		, many);
	strcat(description, ". ");
	return description;
}

char *
get_mt_description_of_monster_type(struct monst * mtmp, char * description)
{
	struct permonst * ptr = mtmp->data;
	strcat(description, "Thinking: ");
	int many = 0;
	int eats = 0;
	int likes = 0;
	int wants = 0;

	many = append(description, bold(ptr)				, "fearless"					, many);
	many = append(description, hides_under(ptr)			, "hides"						, many);
	many = append(description, is_hider(ptr)			, "camoflauged"					, many);
	many = append(description, notake(ptr)				, "doesn't pick up items"		, many);
	many = append(description, mindless_mon(mtmp)		, "mindless"					, many);
	many = append(description, is_animal(ptr)			, "animal minded"				, many);
	eats = appendgroup(description, carnivorous(ptr)	, "eats",	"meat"				, many, eats);
	eats = appendgroup(description, herbivorous(ptr)	, "eats",	"veggies"			, many, eats);
	eats = appendgroup(description, metallivorous(ptr)	, "eats",	"metal"				, many, eats);
	many = appendgroup(description, magivorous(ptr)		, "eats",	"magic"				, many, eats) + many;
	many = append(description, is_domestic(ptr)			, "domestic"					, many);
	many = append(description, is_wanderer(ptr)			, "wanders"						, many);
	many = append(description, always_hostile_mon(mtmp)	, "usually hostile"				, many);
	many = append(description, always_peaceful(ptr)		, "usually peaceful"			, many);
	many = append(description, throws_rocks(ptr)		, "throws rocks"				, many);
	likes= appendgroup(description, likes_gold(ptr)		, "likes",	"gold"				, many, likes);
	likes= appendgroup(description, likes_gems(ptr)		, "likes",	"gems"				, many, likes);
	likes= appendgroup(description, likes_objs(ptr)		, "likes",	"equipment"			, many, likes);
	many = appendgroup(description, likes_magic(ptr)	, "likes",	"magic items"		, many, likes) + many;
	wants= appendgroup(description, wants_bell(ptr)		, "wants",	"the bell"				, many, wants);
	wants= appendgroup(description, wants_book(ptr)		, "wants",	"the book"				, many, wants);
	wants= appendgroup(description, wants_cand(ptr)		, "wants",	"the candalabrum"		, many, wants);
	wants= appendgroup(description, wants_qart(ptr)		, "wants",	"your quest artifact"	, many, wants);
	many = appendgroup(description, wants_amul(ptr)		, "wants",	"the amulet"			, many, wants) + many;
	many = append(description, can_betray(ptr)			, "traitorous"					, many);
	many = append(description, (many==0)				, "nothing special"				, many);
	strcat(description, ". ");
	return description;
}
char *
get_mb_description_of_monster_type(struct monst * mtmp, char * description)
{
	struct permonst * ptr = mtmp->data;
	strcat(description, "Body: ");
	int many = 0;
	many = append(description, your_race(ptr)			, "same race as you"		, many);
	many = append(description, !haseyes(ptr)			, "eyeless"					, many);
	many = append(description, sensitive_ears(ptr)		, "has sensitive ears"		, many);
	many = append(description, nohands(ptr)				, "handless"				, many);
	many = append(description, nolimbs(ptr)				, "limbless"				, many);
	many = append(description, !has_head(ptr)			, "headless"				, many);
	many = append(description, has_horns(ptr)			, "has horns"				, many);
	many = append(description, is_whirly(ptr)			, "whirly"					, many);
	many = append(description, flaming(ptr)				, "flaming"					, many);
	many = append(description, is_stone(ptr)			, "stony"					, many);
	many = append(description, is_anhydrous(ptr)		, "water-less"				, many);
	many = append(description, unsolid(ptr)				, "unsolid"					, many);
	many = append(description, slithy(ptr)				, "slithy"					, many);
	many = append(description, thick_skinned(ptr)		, "thick-skinned"			, many);
	many = append(description, lays_eggs(ptr)			, "oviparus"				, many);
	many = append(description, acidic(ptr)				, "acidic to eat"			, many);
	many = append(description, poisonous(ptr)			, "poisonous to eat"		, many);
	many = append(description, freezing(ptr)			, "freezing to eat"			, many);
	many = append(description, burning(ptr)				, "burning to eat"			, many);
	many = append(description, hallucinogenic(ptr)		, "hallucinogenic to eat"	, many);
	many = append(description, is_deadly(ptr)			, "deadly to eat"			, many);
	many = append(description, is_male(ptr)				, "always male"				, many);
	many = append(description, is_female(ptr)			, "always female"			, many);
	many = append(description, is_neuter(ptr)			, "neuter"					, many);
	many = append(description, strongmonst(ptr)			, "strong"					, many);
	many = append(description, infravisible(ptr)		, "infravisible"			, many);
	many = append(description, humanoid(ptr)			, "humanoid"				, many);
	many = append(description, animaloid(ptr)			, "animaloid"				, many);
	many = append(description, serpentine(ptr)			, "serpent"					, many);
	many = append(description, centauroid(ptr)			, "centauroid"				, many);
	many = append(description, snakemanoid(ptr)			, "human-serpent"			, many);
	many = append(description, leggedserpent(ptr)		, "animal-serpent"			, many);
	many = append(description, (many==0)				, "unknown thing"			, many);
	strcat(description, ". ");
	return description;
}

char *
get_ma_description_of_monster_type(struct monst * mtmp, char * description)
{
	struct permonst * ptr = mtmp->data;
	strcat(description, "Race: ");
	int many = 0;
	many = append(description, (is_undead_mon(mtmp))			, "undead"				, many);
	many = append(description, (ptr->mflagsa & MA_WERE)			, "lycanthrope"			, many);
	many = append(description, (ptr->mflagsa & MA_HUMAN)		, "human"				, many);
	many = append(description, (ptr->mflagsa & MA_ELF)			, "elf"					, many);
	many = append(description, (ptr->mflagsa & MA_DROW)			, "drow"				, many);
	many = append(description, (ptr->mflagsa & MA_DWARF)		, "dwarf"				, many);
	many = append(description, (ptr->mflagsa & MA_GNOME)		, "gnome"				, many);
	many = append(description, (ptr->mflagsa & MA_ORC)			, "orc"					, many);
	many = append(description, (ptr->mflagsa & MA_VAMPIRE)		, "vampire"				, many);
	many = append(description, (ptr->mflagsa & MA_CLOCK)		, "clockwork automaton"	, many);
	many = append(description, (ptr->mflagsa & MA_UNLIVING)		, "not alive"			, many);
	many = append(description, (ptr->mflagsa & MA_PLANT)		, "plant"				, many);
	many = append(description, (ptr->mflagsa & MA_GIANT)		, "giant"				, many);
	many = append(description, (ptr->mflagsa & MA_INSECTOID)	, "insectoid"			, many);
	many = append(description, (ptr->mflagsa & MA_ARACHNID)		, "arachind"			, many);
	many = append(description, (ptr->mflagsa & MA_AVIAN)		, "avian"				, many);
	many = append(description, (ptr->mflagsa & MA_REPTILIAN)	, "reptilian"			, many);
	many = append(description, (ptr->mflagsa & MA_ANIMAL)		, "mundane animal"		, many);
	many = append(description, (ptr->mflagsa & MA_AQUATIC)		, "water-dweller"		, many);
	many = append(description, (ptr->mflagsa & MA_DEMIHUMAN)	, "demihuman"			, many);
	many = append(description, (ptr->mflagsa & MA_FEY)			, "fey"					, many);
	many = append(description, (ptr->mflagsa & MA_ELEMENTAL)	, "elemental"			, many);
	many = append(description, (ptr->mflagsa & MA_DRAGON)		, "dragon"				, many);
	many = append(description, (ptr->mflagsa & MA_DEMON)		, "demon"				, many);
	many = append(description, (ptr->mflagsa & MA_MINION)		, "minion of a deity"	, many);
	many = append(description, (ptr->mflagsa & MA_PRIMORDIAL)	, "primordial"			, many);
	many = append(description, (ptr->mflagsa & MA_ET)			, "alien"				, many);
	strcat(description, ". ");
	return description;
}

char *
get_mv_description_of_monster_type(struct monst * mtmp, char * description)
{
	struct permonst * ptr = mtmp->data;
	strcat(description, "Vision: ");
	int many = 0;
	many = append(description, goodsmeller(ptr)			, "scent"					, many);
	many = append(description, species_perceives(ptr)	, "see invisible"			, many);
	many = append(description, species_is_telepathic(ptr)	, "telepathy"				, many);
	many = append(description, normalvision(ptr)		, "normal vision"			, many);
	many = append(description, darksight(ptr)			, "darksight"				, many);
	many = append(description, catsight(ptr)			, "catsight"				, many);
	many = append(description, lowlightsight2(ptr)		, "good low light vision"	, many);
	many = append(description, lowlightsight3(ptr)		, "great low light vision"	, many);
	many = append(description, echolocation(ptr)		, "echolocation"			, many);
	many = append(description, extramission(ptr)		, "extramission"			, many);
	many = append(description, rlyehiansight(ptr)		, "rlyehian sight"			, many);
	many = append(description, infravision(ptr)			, "infravision"				, many);
	many = append(description, bloodsense(ptr)			, "bloodsense"				, many);
	many = append(description, lifesense(ptr)			, "lifesense"				, many);
	many = append(description, earthsense(ptr)			, "earthsense"				, many);
	many = append(description, senseall(ptr)			, "senseall"				, many);
	many = append(description, (many==0)				, "blind"					, many);
	strcat(description, ". ");
	return description;
}
char * get_mg_description_of_monster_type(struct monst * mtmp, char * description)
{
	struct permonst * ptr = mtmp->data;
	strcat(description, "Mechanics: ");
	int many = 0;
	many = append(description, is_tracker(ptr)			, "tracks you"				, many);
	many = append(description, species_displaces(ptr), "displacing"				, many);
	many = append(description, polyok(ptr)				, "valid polymorph form"	, many);
	many = append(description, !polyok(ptr)				, "invalid polymorph form"	, many);
	many = append(description, is_untamable(ptr)		, "untamable"				, many);
	many = append(description, extra_nasty(ptr)			, "nasty"					, many);
	many = append(description, nospellcooldowns(ptr)	, "quick-caster"			, many);
	many = append(description, is_lord(ptr)				, "lord"					, many);
	many = append(description, is_prince(ptr)			, "prince"					, many);
	many = append(description, opaque(ptr)				, "opaque"					, many);
	many = append(description, species_regenerates(ptr)	, "regenerating"			, many);
	many = append(description, levl_follower(mtmp)		, "stalks you"				, many);
	many = append(description, (many==0)				, "normal"					, many);
	strcat(description, ". ");
	return description;
}

char *
get_speed_description_of_monster_type(struct monst * mtmp, char * description)
{
	int speed = mtmp->data->mmove;
	switch (mtmp->mfaction)
	{
	case ZOMBIFIED:
		speed = max(6, speed * 1/2);
		break;
	case SKELIFIED:
		speed = max(6, speed * 3/4);
		break;
	}
	if (speed > 35) {
		sprintf(description, "Extremely fast (%d). ", speed);
	}
	else if (speed > 19) {
		sprintf(description, "Very fast (%d). ", speed);
	}
	else if (speed > 12) {
		sprintf(description, "Fast (%d). ", speed);
	}
	else if (speed == 12) {
		sprintf(description, "Normal speed (%d). ", speed);
	}
	else if (speed > 8) {
		sprintf(description, "Slow (%d). ", speed);
	}
	else if (speed > 3) {
		sprintf(description, "Very slow (%d). ", speed);
	}
	else if (speed > 0) {
		sprintf(description, "Almost immobile (%d). ", speed);
	}
	else {
		sprintf(description, "Immobile (%d). ", speed);
	}

	if (stationary(mtmp->data)) sprintf(description, "Can't move around. Speed %d. ", speed);

	return description;
}

char *
get_description_of_attack_type(uchar id)
{
	switch (id){
	//case AT_ANY: return "fake attack; dmgtype_fromattack wildcard";
	case AT_NONE: return "passive";
	case AT_CLAW: return "claw";
	case AT_BITE: return "bite";
	case AT_KICK: return "kick";
	case AT_BUTT: return "head butt";
	case AT_TUCH: return "touch";
	case AT_STNG: return "sting";
	case AT_HUGS: return "crushing bearhug";
	case AT_SPIT: return "spit";
	case AT_ENGL: return "engulf";
	case AT_BREA: return "breath";
	case AT_EXPL: return "explosion";
	case AT_BOOM: return "on death";
	case AT_GAZE: return "targeted gaze";
	case AT_TENT: return "tentacles";
	case AT_ARRW: return "launch ammo";
	case AT_WHIP: return "whip";
	case AT_LRCH: return "reaching attack";
	case AT_HODS: return "mirror attack";
	case AT_LNCK: return "reaching bite";
	case AT_5SBT: return "long reach bite";
	case AT_MMGC: return "uses magic spell(s)";
	case AT_ILUR: return "swallow attack";
	case AT_HITS: return "automatic hit";
	case AT_WISP: return "mist wisps";
	case AT_TNKR: return "tinker";
	case AT_SHDW: return "phasing";
	case AT_BEAM: return "ranged beam";
	case AT_DEVA: return "million-arm weapon";
	case AT_5SQR: return "long reach touch";
	case AT_WDGZ: return "passive gaze";
	case AT_MARI: return "carried weapon";
	case AT_XWEP: return "offhand weapon";
	case AT_WEAP: return "weapon";
	case AT_MAGC: return "uses magic spell(s)";
	case AT_REND: return "extra effect for previous attacks";
	default:
			impossible("bug in get_description_of_attack_type(%d)", id);
			return "<MISSING DECRIPTION, THIS IS A BUG>";
	}
}

char *
get_description_of_damage_type(uchar id)
{
	switch (id){
	//case AD_ANY: return "fake damage; attacktype_fordmg wildcard";
	case AD_PHYS: return "physical";
	case AD_MAGM: return "magic missiles";
	case AD_FIRE: return "fire";
	case AD_COLD: return "frost";
	case AD_SLEE: return "sleep";
	case AD_DISN: return "disintegration";
	case AD_ELEC: return "shock";
	case AD_DRST: return "poison (STR)";
	case AD_ACID: return "acid";
	case AD_SPC1: return "for extension of buzz()";
	case AD_SPC2: return "for extension of buzz()";
	case AD_BLND: return "blinds";
	case AD_STUN: return "stuns";
	case AD_SLOW: return "slows";
	case AD_PLYS: return "paralyses";
	case AD_DRLI: return "drains life";
	case AD_DREN: return "drains energy";
	case AD_LEGS: return "damages legs";
	case AD_STON: return "petrifies";
	case AD_STCK: return "sticky";
	case AD_SGLD: return "steals gold";
	case AD_SITM: return "steals item";
	case AD_SEDU: return "seduces & steals multiple items";
	case AD_TLPT: return "teleports you";
	case AD_RUST: return "rusts armour";
	case AD_CONF: return "confuses";
	case AD_DGST: return "digests";
	case AD_HEAL: return "heals wounds";
	case AD_WRAP: return "crushes and drowns";
	case AD_WERE: return "confers lycanthropy";
	case AD_DRDX: return "poison (DEX)";
	case AD_DRCO: return "poison (CON)";
	case AD_DRIN: return "drains intelligence";
	case AD_DISE: return "confers diseases";
	case AD_DCAY: return "decays organics";
	case AD_SSEX: return "seduces";
	case AD_HALU: return "causes hallucination";
	case AD_DETH: return "drains life force";
	case AD_PEST: return "causes illness";
	case AD_FAMN: return "causes hunger";
	case AD_SLIM: return "slimes";
	case AD_ENCH: return "disenchants";
	case AD_CORR: return "corrodes armor";
	case AD_POSN: return "poison";
	case AD_WISD: return "drains wisdom";
	case AD_VORP: return "vorpal strike";
	case AD_SHRD: return "destroys armor";
	case AD_SLVR: return "silver arrows";
	case AD_BALL: return "iron balls";
	case AD_BLDR: return "boulders";
	case AD_VBLD: return "boulder volley";
	case AD_TCKL: return "tickle";
	case AD_WET: return "splash with water";
	case AD_LETHE: return "splash with lethe water";
	case AD_BIST: return "bisecting beak";
	case AD_CNCL: return "cancelation";
	case AD_DEAD: return "deadly gaze";
	case AD_SUCK: return "sucks you apart";
	case AD_MALK: return "immobilize and shock";
	case AD_UVUU: return "splatters your head";
	case AD_ABDC: return "abduction teleportation";
	case AD_KAOS: return "spawn Chaos";
	case AD_LSEX: return "seduces";
	case AD_HLBD: return "creates demons";
	case AD_SPNL: return "spawns Leviathan or Levistus";
	case AD_MIST: return "migo mist projector";
	case AD_TELE: return "monster teleports away";
	case AD_POLY: return "polymorphs you";
	case AD_PSON: return "psionic powers";
	case AD_GROW: return "grows brethren on death";
	case AD_SOUL: return "strengthens brethren on death";
	case AD_TENT: return "LarienTelrunya's bane";
	case AD_JAILER: return "sets Lucifer to apear and drops third key of law when killed";
	case AD_AXUS: return "fire-shock-drain-cold combo";
	case AD_UNKNWN: return "Priest of an unknown God";
	case AD_SOLR: return "silver arrows";
	case AD_CHKH: return "escalating damage";
	case AD_HODS: return "mirror attack";
	case AD_CHRN: return "cursed unicorn horn";
	case AD_LOAD: return "loadstones";
	case AD_GARO: return "physical damage + rumor";
	case AD_GARO_MASTER: return "physical damage + oracle";
	case AD_LVLT: return "level teleport";
	case AD_BLNK: return "mental invasion";
	case AD_WEEP: return "level teleport and drain";
	case AD_SPOR: return "generate spore";
	case AD_FNEX: return "fern spore explosion";
	case AD_SSUN: return "reflected sunlight";
	case AD_MAND: return "mandrake shriek";
	case AD_BARB: return "retaliatory physical";
	case AD_LUCK: return "drains luck";
	case AD_VAMP: return "drains blood";
	case AD_WEBS: return "spreads webbing";
	case AD_ILUR: return "drains memories";
	case AD_TNKR: return "tinkers";
	case AD_FRWK: return "firework explosions";
	case AD_STDY: return "studies you, making you vulnerable";
	case AD_OONA: return "Oona's energy type";
	case AD_NTZC: return "anti-equipment";
	case AD_WTCH: return "spawns tentacles";
	case AD_SHDW: return "shadow weapons";
	case AD_STTP: return "teleports your gear away";
	case AD_HDRG: return "half-dragon breath";
	case AD_STAR: return "silver starlight rapier";
	case AD_EELC: return "elemental electric";
	case AD_EFIR: return "elemental fire";
	case AD_EDRC: return "elemental poison (CON)";
	case AD_ECLD: return "elemental cold";
	case AD_EACD: return "elemental acid";
	case AD_CNFT: return "conflict-inducing touch";
	case AD_BLUD: return "Sword of Blood";
	case AD_SURY: return "Arrows of Slaying";
	case AD_NPDC: return "drains constitution";
	case AD_GLSS: return "silver mirror shards";
	case AD_MERC: return "mercury blade";
	case AD_GOLD: return "turns victim to gold";
	case AD_ACFR: return "holy fire";
	case AD_DESC: return "dessication";
	case AD_BLAS: return "blasphemy";
	case AD_DUNSTAN: return "stones throw themselves at target";
	case AD_IRIS: return "iridescent tentacles";
	case AD_NABERIUS: return "tarnished bloody fangs";
	case AD_OTIAX: return "mist tendrals";
	case AD_SIMURGH: return "thirty-colored feathers";
	case AD_CMSL: return "cold missile";
	case AD_FMSL: return "fire missile";
	case AD_EMSL: return "electric missile";
	case AD_SMSL: return "physical missile";
	case AD_WMTG: return "War Machine targeting gaze";
	case AD_CLRC: return "random clerical spell";
	case AD_SPEL: return "random magic spell";
	case AD_RBRE: return "random breath weapon";
	case AD_RGAZ: return "random gaze attack";
	case AD_RETR: return "elemental gaze attack";
	case AD_SAMU: return "steal Amulet";
	case AD_CURS: return "steal intrinsic";
	case AD_SQUE: return "steal Quest Artifact or Amulet";
	default:
			impossible("bug in get_description_of_damage_type(%d)", id);
			return "<MISSING DESCRIPTION, THIS IS A BUG>";
	}
}

char *
get_description_of_damage_prefix(uchar aatyp, uchar adtyp)
{
	switch (aatyp)
	{
	case AT_WEAP:
	case AT_XWEP:
	case AT_DEVA:
		switch (adtyp)
		{
		case AD_PHYS:
			return "";
		case AD_FIRE:
		case AD_COLD:
		case AD_ELEC:
		case AD_ACID:
			return "physical + 4d6 ";
		case AD_EFIR:
		case AD_ECLD:
		case AD_EELC:
		case AD_EACD:
			return "physical + 3d7 ";
		default:
			return "physical + ";
		}
		break;
	}
	return "";
}

char *
get_description_of_attack(struct attack *mattk, char * main_temp_buf)
{
	if (!(mattk->damn + mattk->damd + mattk->aatyp + mattk->adtyp)) {
		main_temp_buf[0] = '\0';
		return main_temp_buf;
	}

	char temp_buf[BUFSZ] = "";
	if (mattk->damn + mattk->damd) {
		sprintf(main_temp_buf, "%dd%d", mattk->damn, mattk->damd);
#ifndef USE_TILES
		strcat(main_temp_buf, ",");
#endif
		strcat(main_temp_buf, " ");
	}
	else {
		main_temp_buf[0] = '\0';
	}
#ifndef USE_TILES
	while (strlen(main_temp_buf) < 6) {
		strcat(main_temp_buf, " ");
	}
#endif
	sprintf(temp_buf, "%s - %s%s", get_description_of_attack_type(mattk->aatyp), get_description_of_damage_prefix(mattk->aatyp, mattk->adtyp), get_description_of_damage_type(mattk->adtyp));
	strcat(main_temp_buf, temp_buf);
#ifdef USE_TILES
	strcat(main_temp_buf, "; ");
#endif
	return main_temp_buf;
}

char *
get_description_of_monster_type(struct monst * mtmp, char * description)
{
	/*
	pline("%d<><><>", plined_length("12345678901234567890123456789012345678901234567890123456789012345678901234567890"));//0 passed
	pline("%d<><><>", plined_length("1234567890123456789012345678901234567890123456789012345678901234567890123456789"));
	*/
	char temp_buf[BUFSZ] = "";
	char main_temp_buf[BUFSZ] = "";
	struct permonst * ptr = mtmp->data;
	int monsternumber = monsndx(ptr);

	char name[BUFSZ] = "";
	Strcat(name, ptr->mname);
	if (mtmp->mfaction == ZOMBIFIED)		Strcat(name, " zombie");
	else if (mtmp->mfaction == SKELIFIED)	Strcat(name, " skeleton");
	else if (mtmp->mfaction == CRYSTALFIED) Strcat(name, " vitrean");
	else if (mtmp->mfaction == FRACTURED)	Strcat(name, " witness");
	else if (mtmp->mfaction == ILLUMINATED)	Strcat(name, " shining one");
	else if (mtmp->mfaction == VAMPIRIC)	Strcat(name, " vampire");
	else if (mtmp->mfaction == PSEUDONATURAL)	Strcat(name, " pseudonatural");
	else if (mtmp->mfaction == TOMB_HERD)	Strcat(name, " tomb herd");

	temp_buf[0] = '\0';
	if (iflags.pokedex) {
		sprintf(temp_buf, "Accessing Pokedex entry for %s... ", (!Upolyd || (monsternumber < NUMMONS)) ? name : "this weird creature");
		strcat(description, temp_buf);

		strcat(description, "\n");
		strcat(description, " ");
		strcat(description, "\n");
		if (iflags.pokedex & POKEDEX_SHOW_STATS){
			strcat(description, "Base statistics of this monster type:");
			strcat(description, "\n");
			int ac = 10-(ptr->nac+ptr->dac+ptr->pac);
			ac += (mtmp->mfaction == CRYSTALFIED ? -16 : mtmp->mfaction == SKELIFIED ? -6 : mtmp->mfaction == ZOMBIFIED ? -2 : 0);
			sprintf(temp_buf, "Base level = %d. Difficulty = %d. AC = %d. MR = %d. Alignment %d. ", ptr->mlevel, monstr[monsndx(ptr)], ac, ptr->mr, ptr->maligntyp);
			strcat(description, temp_buf);
			temp_buf[0] = '\0';
			strcat(description, get_speed_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_GENERATION){
			temp_buf[0] = '\0';
			strcat(description, "\n");
			strcat(description, get_generation_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_RESISTS){
			temp_buf[0] = '\0';
			strcat(description, "\n");
			strcat(description, get_resistance_description_of_monster_type(mtmp, temp_buf));
			temp_buf[0] = '\0';
			strcat(description, "\n");
			strcat(description, get_weakness_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_CONVEYS){
			temp_buf[0] = '\0';
			strcat(description, "\n");
			strcat(description, get_conveys_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_WEIGHT){
			temp_buf[0] = '\0';
			strcat(description, get_weight_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_MM){
			temp_buf[0] = '\0';
			strcat(description, "\n");
			strcat(description, get_mm_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_MT){
			temp_buf[0] = '\0';
			strcat(description, "\n");
			strcat(description, get_mt_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_MB){
			temp_buf[0] = '\0';
			strcat(description, "\n");
			strcat(description, get_mb_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_MV){
			temp_buf[0] = '\0';
			strcat(description, "\n");
			strcat(description, get_mv_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_MG){
			temp_buf[0] = '\0';
			strcat(description, "\n");
			strcat(description, get_mg_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_MA){
			temp_buf[0] = '\0';
			strcat(description, "\n");
			strcat(description, get_ma_description_of_monster_type(mtmp, temp_buf));
		}
		if (iflags.pokedex & POKEDEX_SHOW_ATTACKS){
			strcat(description, "\n");
			strcat(description, "Attacks:");
			strcat(description, "\n");
			struct attack *mattk;
			struct attack alt_attk;
			int sum[NATTK];
			int i;

			for (i = 0; i < NATTK; i++) {
				sum[i] = 1;
				mattk = getmattk(mtmp, ptr, i, sum, &alt_attk);

				main_temp_buf[0] = '\0';
				get_description_of_attack(mattk, temp_buf);
				if (temp_buf[0] == '\0') {
					if (i == 0) {
#ifndef USE_TILES
						strcat(description, "    ");
#endif
						strcat(description, "none");
						strcat(description, "\n");
					}
					continue;
				}
#ifndef USE_TILES
				strcat(main_temp_buf, "    ");
#endif
				strcat(main_temp_buf, temp_buf);
				strcat(main_temp_buf, "\n");
				strcat(description, main_temp_buf);
			}
		}
	}
	return description;
}

int
dowhatis()
{
	return do_look(FALSE);
}

int
doquickwhatis()
{
	return do_look(TRUE);
}

int
doidtrap()
{
	register struct trap *trap;
	int x, y, tt;

	if (!getdir("^")) return 0;
	x = u.ux + u.dx;
	y = u.uy + u.dy;
	for (trap = ftrap; trap; trap = trap->ntrap)
	    if (trap->tx == x && trap->ty == y) {
		if (!trap->tseen) break;
		tt = trap->ttyp;
		if (u.dz) {
		    if (u.dz < 0 ? (tt == TRAPDOOR || tt == HOLE) :
			    tt == ROCKTRAP) break;
		}
		tt = what_trap(tt);
		pline("That is %s%s%s.",
		      an(defsyms[trap_to_defsym(tt)].explanation),
		      !trap->madeby_u ? "" : (tt == WEB) ? " woven" :
			  /* trap doors & spiked pits can't be made by
			     player, and should be considered at least
			     as much "set" as "dug" anyway */
			  (tt == HOLE || tt == PIT) ? " dug" : " set",
		      !trap->madeby_u ? "" : " by you");
		return 0;
	    }
	pline("I can't see a trap there.");
	return 0;
}

char *
dowhatdoes_core(q, cbuf)
char q;
char *cbuf;
{
	dlb *fp;
	char bufr[BUFSZ];
	register char *buf = &bufr[6], *ep, ctrl, meta;

	fp = dlb_fopen(CMDHELPFILE, "r");
	if (!fp) {
		pline("Cannot open data file!");
		return 0;
	}

  	ctrl = ((q <= '\033') ? (q - 1 + 'A') : 0);
	meta = ((0x80 & q) ? (0x7f & q) : 0);
	while(dlb_fgets(buf,BUFSZ-6,fp)) {
	    if ((ctrl && *buf=='^' && *(buf+1)==ctrl) ||
		(meta && *buf=='M' && *(buf+1)=='-' && *(buf+2)==meta) ||
		*buf==q) {
		ep = index(buf, '\n');
		if(ep) *ep = 0;
		if (ctrl && buf[2] == '\t'){
			buf = bufr + 1;
			(void) strncpy(buf, "^?      ", 8);
			buf[1] = ctrl;
		} else if (meta && buf[3] == '\t'){
			buf = bufr + 2;
			(void) strncpy(buf, "M-?     ", 8);
			buf[2] = meta;
		} else if(buf[1] == '\t'){
			buf = bufr;
			buf[0] = q;
			(void) strncpy(buf+1, "       ", 7);
		}
		(void) dlb_fclose(fp);
		Strcpy(cbuf, buf);
		return cbuf;
	    }
	}
	(void) dlb_fclose(fp);
	return (char *)0;
}

int
dowhatdoes()
{
	char bufr[BUFSZ];
	char q, *reslt;

#if defined(UNIX) || defined(VMS)
	introff();
#endif
	q = yn_function("What command?", (char *)0, '\0');
#if defined(UNIX) || defined(VMS)
	intron();
#endif
	reslt = dowhatdoes_core(q, bufr);
	if (reslt)
		pline("%s", reslt);
	else
		pline("I've never heard of such commands.");
	return 0;
}

/* data for help_menu() */
static const char *help_menu_items[] = {
/* 0*/	"Long description of the game and commands.",
/* 1*/	"List of game commands.",
/* 2*/	"Concise history of NetHack.",
/* 3*/	"Info on a character in the game display.",
/* 4*/	"Info on what a given key does.",
/* 5*/	"List of game options.",
/* 6*/	"Longer explanation of game options.",
/* 7*/	"List of extended commands.",
/* 8*/	"The NetHack license.",
#ifdef PORT_HELP
	"%s-specific help and commands.",
#define PORT_HELP_ID 100
#define WIZHLP_SLOT 10
#else
#define WIZHLP_SLOT 9
#endif
#ifdef WIZARD
	"List of wizard-mode commands.",
#endif
	"",
	(char *)0
};

STATIC_OVL boolean
help_menu(sel)
	int *sel;
{
	winid tmpwin = create_nhwindow(NHW_MENU);
#ifdef PORT_HELP
	char helpbuf[QBUFSZ];
#endif
	int i, n;
	menu_item *selected;
	anything any;

	any.a_void = 0;		/* zero all bits */
	start_menu(tmpwin);
#ifdef WIZARD
	if (!wizard) help_menu_items[WIZHLP_SLOT] = "",
		     help_menu_items[WIZHLP_SLOT+1] = (char *)0;
#endif
	for (i = 0; help_menu_items[i]; i++)
#ifdef PORT_HELP
	    /* port-specific line has a %s in it for the PORT_ID */
	    if (help_menu_items[i][0] == '%') {
		Sprintf(helpbuf, help_menu_items[i], PORT_ID);
		any.a_int = PORT_HELP_ID + 1;
		add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 helpbuf, MENU_UNSELECTED);
	    } else
#endif
	    {
		any.a_int = (*help_menu_items[i]) ? i+1 : 0;
		add_menu(tmpwin, NO_GLYPH, &any, 0, 0,
			ATR_NONE, help_menu_items[i], MENU_UNSELECTED);
	    }
	end_menu(tmpwin, "Select one item:");
	n = select_menu(tmpwin, PICK_ONE, &selected);
	destroy_nhwindow(tmpwin);
	if (n > 0) {
	    *sel = selected[0].item.a_int - 1;
	    free((genericptr_t)selected);
	    return TRUE;
	}
	return FALSE;
}

int
dohelp()
{
	int sel = 0;

	if (help_menu(&sel)) {
		switch (sel) {
			case  0:  display_file(HELP, TRUE);  break;
			case  1:  display_file(SHELP, TRUE);  break;
			case  2:  (void) dohistory();  break;
			case  3:  (void) dowhatis();  break;
			case  4:  (void) dowhatdoes();  break;
			case  5:  option_help();  break;
			case  6:  display_file(OPTIONFILE, TRUE);  break;
			case  7:  (void) doextlist();  break;
			case  8:  display_file(LICENSE, TRUE);  break;
#ifdef WIZARD
			/* handle slot 9 or 10 */
			default: display_file(DEBUGHELP, TRUE);  break;
#endif
#ifdef PORT_HELP
			case PORT_HELP_ID:  port_help();  break;
#endif
		}
	}
	return 0;
}

int
dohistory()
{
	display_file(HISTORY, TRUE);
	return 0;
}

/*pager.c*/
