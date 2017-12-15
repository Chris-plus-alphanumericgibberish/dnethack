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
STATIC_DCL struct permonst * FDECL(lookat, (int, int, char *, char *, char *));
STATIC_DCL void FDECL(checkfile,
		      (char *,struct permonst *,BOOLEAN_P,BOOLEAN_P));
STATIC_DCL int FDECL(do_look, (BOOLEAN_P));
STATIC_DCL boolean FDECL(help_menu, (int *));
#ifdef PORT_HELP
extern void NDECL(port_help);
#endif

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

/*
 * Return the name of the glyph found at (x,y).
 * If not hallucinating and the glyph is a monster, also monster data.
 */
STATIC_OVL struct permonst *
lookat(x, y, buf, monbuf, shapebuff)
    int x, y;
    char *buf, *monbuf, *shapebuff;
{
    register struct monst *mtmp = (struct monst *) 0;
    struct permonst *pm = (struct permonst *) 0;
    int glyph;

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
	if (mtmp != (struct monst *) 0) {
	    char *name, monnambuf[BUFSZ];
	    boolean accurate = !Hallucination;

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
		    (EDOG(mtmp)->friend ? "friendly " : "tame ") :
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
		if ((!mtmp->minvis || See_invisible) && see_with_infrared(mtmp))
		    ways_seen++;
		/* bloodsense */
		if ((!mtmp->minvis || See_invisible) && see_with_bloodsense(mtmp))
		    ways_seen++;
		/* lifesense */
		if ((!mtmp->minvis || See_invisible) && see_with_lifesense(mtmp))
		    ways_seen++;
		/* senseall */
		if ((!mtmp->minvis || See_invisible) && see_with_senseall(mtmp))
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
		    if ((!mtmp->minvis || See_invisible) &&
			    see_with_infrared(mtmp)) {
			Strcat(monbuf, "infravision");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
		    if ((!mtmp->minvis || See_invisible) &&
			    see_with_bloodsense(mtmp)) {
			Strcat(monbuf, "bloodsense");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
		    }
		    if ((!mtmp->minvis || See_invisible) &&
			    see_with_lifesense(mtmp)) {
			Strcat(monbuf, "lifesense");
			if (ways_seen-- > 1) Strcat(monbuf, ", ");
			}
		    if ((!mtmp->minvis || See_invisible) &&
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
			if (Hallucination){
				Strcat(monbuf, "paranoid delusion");
				if (ways_seen-- > 1) Strcat(monbuf, ", ");
			} else {
				//going to need more detail about how it is seen....
				ways_seen = 0;
				if(u.sealsActive&SEAL_PAIMON && is_magical((mtmp)->data)) ways_seen++;
				if(u.sealsActive&SEAL_ANDROMALIUS && is_thief((mtmp)->data)) ways_seen++;
				if(u.sealsActive&SEAL_TENEBROUS && !nonliving_mon(mtmp)) ways_seen++;
				if(u.specialSealsActive&SEAL_ACERERAK && is_undead_mon(mtmp)) ways_seen++;
				if(uwep && ((uwep->ovar1 & WARD_THJOFASTAFUR) && 
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
					if(uwep && (uwep->ovar1 & WARD_THJOFASTAFUR) && ((mtmp)->data->mlet == S_LEPRECHAUN || (mtmp)->data->mlet == S_NYMPH || is_thief((mtmp)->data))){
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
		if(mtmp->data->msize == MZ_TINY) Sprintf(shapebuff, "a tiny");
		else if(mtmp->data->msize == MZ_SMALL) Sprintf(shapebuff, "a small");
		else if(mtmp->data->msize == MZ_HUMAN) Sprintf(shapebuff, "a human-sized");
		else if(mtmp->data->msize == MZ_LARGE) Sprintf(shapebuff, "a large");
		else if(mtmp->data->msize == MZ_HUGE) Sprintf(shapebuff, "a huge");
		else if(mtmp->data->msize == MZ_GIGANTIC) Sprintf(shapebuff, "a gigantic");
		else Sprintf(shapebuff, "an odd-sized");
		
		if((mtmp->data->mflagsb&MB_HEADMODIMASK) == MB_LONGHEAD) Strcat(shapebuff, ", snouted");
		else if((mtmp->data->mflagsb&MB_HEADMODIMASK) == MB_LONGNECK) Strcat(shapebuff, ", long-necked");
		
		if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == MB_ANIMAL) Strcat(shapebuff, " animal");
		else if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == MB_SLITHY) Strcat(shapebuff, " ophidian");
		else if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == MB_HUMANOID) Strcat(shapebuff, " humanoid");
		else if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == (MB_HUMANOID|MB_ANIMAL)) Strcat(shapebuff, " centauroid");
		else if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == (MB_HUMANOID|MB_SLITHY)) Strcat(shapebuff, " snake-legged humanoid");
		else if((mtmp->data->mflagsb&MB_BODYTYPEMASK) == (MB_ANIMAL|MB_SLITHY)) Strcat(shapebuff, " snake-backed animal");
		else Strcat(shapebuff, " thing");
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
	Strcpy(buf, Is_airlevel(&u.uz) ? "cloudy area" : "fog/vapor cloud");
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

    return ((pm && !Hallucination) ? pm : (struct permonst *) 0);
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
 */
STATIC_OVL void
checkfile(inp, pm, user_typed_name, without_asking)
    char *inp;
    struct permonst *pm;
    boolean user_typed_name, without_asking;
{
    dlb *fp;
    char buf[BUFSZ], newstr[BUFSZ];
    char *ep, *dbase_str;
    long txt_offset;
    int chk_skip;
    boolean found_in_file = FALSE, skipping_entry = FALSE;

    fp = dlb_fopen(DATAFILE, "r");
    if (!fp) {
	pline("Cannot open data file!");
	return;
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
	    return;
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
		return;
	}

	if (user_typed_name || without_asking || yn("More info?") == 'y') {
	    winid datawin;

	    if (dlb_fseek(fp, txt_offset + entry_offset, SEEK_SET) < 0) {
		pline("? Seek error on 'data' file!");
		(void) dlb_fclose(fp);
		return;
	    }
	    datawin = create_nhwindow(NHW_MENU);
	    for (i = 0; i < entry_count; i++) {
		if (!dlb_fgets(buf, BUFSZ, fp)) goto bad_data_file;
		if ((ep = index(buf, '\n')) != 0) *ep = 0;
		if (index(buf+1, '\t') != 0) (void) tabexpand(buf+1);
		putstr(datawin, 0, buf+1);
	    }
	    display_nhwindow(datawin, FALSE);
	    destroy_nhwindow(datawin);
	}
    } else if (user_typed_name)
	pline("I don't have any information on those things.");

    (void) dlb_fclose(fp);
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
    int     i, ans = 0;
    int     sym;		/* typed symbol or converted glyph */
    int	    found;		/* count of matching syms found */
    coord   cc;			/* screen pos of unknown glyph */
    boolean save_verbose;	/* saved value of flags.verbose */
    boolean from_screen;	/* question from the screen */
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
	    checkfile(out_str, pm, TRUE, TRUE);
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
		    Sprintf(out_str, "%c       %s", sym, an(monexplain[i]));
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
	    !(Race_if(PM_HUMAN) || Race_if(PM_ELF) || Race_if(PM_DROW) || Race_if(PM_MYRKALFR)) && !Upolyd)
	    found += append_str(out_str, "you");	/* tack on "or you" */

	/*
	 * Special case: if identifying from the screen, and we're swallowed,
	 * and looking at something other than our own symbol, then just say
	 * "the interior of a monster".
	 */
	if (u.uswallow && from_screen && is_swallow_sym(sym)) {
	    if (!found) {
		Sprintf(out_str, "%c       %s", sym, mon_interior);
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
		    Sprintf(out_str, "%c       %s", sym, an(objexplain[i]));
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
		Sprintf(out_str, "%c       %s", sym, an(invisexplain));
		firstmatch = invisexplain;
		found++;
	    } else {
		found += append_str(out_str, an(invisexplain));
	    }
	}

#define is_cmap_trap(i) ((i) >= S_arrow_trap && (i) <= S_polymorph_trap)
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
			Sprintf(out_str, "%c       a trap", sym);
			hit_trap = TRUE;
		    } else if (level.flags.lethe && !strcmp(x_str, "water")) { //Lethe patch
			Sprintf(out_str, "%c       sparkling water", sym); //Lethe patch
		    } else {
			Sprintf(out_str, "%c       %s", sym,
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
				sym, def_warnsyms[i].explanation);
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
		Sprintf(out_str, "%c       %s", sym, an(x_str));
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
		Sprintf(out_str, "%c       %s", sym, an(firstmatch));
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

		pm = lookat(cc.x, cc.y, look_buf, monbuf, shapebuf);
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
		}
	}

	/* Finally, print out our explanation. */
	if (found) {
	    pline("%s", out_str);
	    /* check the data file for information about this thing */
	    if (found == 1 && ans != LOOK_QUICK && ans != LOOK_ONCE &&
			(ans == LOOK_VERBOSE || (flags.help && !quick))) {
		char temp_buf[BUFSZ];
		Strcpy(temp_buf, level.flags.lethe //lethe
					&& !strcmp(firstmatch, "water")?
				"lethe" : firstmatch);
		checkfile(temp_buf, pm, FALSE, (boolean)(ans == LOOK_VERBOSE));
	    }
	} else {
	    pline("I've never heard of such things.");
	}

    } while (from_screen && !quick && ans != LOOK_ONCE);

    flags.verbose = save_verbose;
    return 0;
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
