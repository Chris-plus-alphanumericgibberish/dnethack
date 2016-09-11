/*	SCCS Id: @(#)cmd.c	3.4	2003/02/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "func_tab.h"
/* #define DEBUG */	/* uncomment for debugging */

/*
 * Some systems may have getchar() return EOF for various reasons, and
 * we should not quit before seeing at least NR_OF_EOFS consecutive EOFs.
 */
#if defined(SYSV) || defined(DGUX) || defined(HPUX)
#define NR_OF_EOFS	20
#endif

#define CMD_TRAVEL (char)0x90

#ifdef DEBUG
/*
 * only one "wiz_debug_cmd" routine should be available (in whatever
 * module you are trying to debug) or things are going to get rather
 * hard to link :-)
 */
extern int NDECL(wiz_debug_cmd);
#endif

#ifdef DUMB	/* stuff commented out in extern.h, but needed here */
extern int NDECL(doapply); /**/
extern int NDECL(dorub); /**/
extern int NDECL(dojump); /**/
extern int NDECL(doextlist); /**/
extern int NDECL(dodrop); /**/
extern int NDECL(doddrop); /**/
extern int NDECL(dodown); /**/
extern int NDECL(doup); /**/
extern int NDECL(donull); /**/
extern int NDECL(dowipe); /**/
extern int NDECL(do_mname); /**/
extern int NDECL(ddocall); /**/
extern int NDECL(dotakeoff); /**/
extern int NDECL(doremring); /**/
extern int NDECL(dowear); /**/
extern int NDECL(doputon); /**/
extern int NDECL(doddoremarm); /**/
extern int NDECL(dokick); /**/
extern int NDECL(dofire); /**/
extern int NDECL(dothrow); /**/
extern int NDECL(doeat); /**/
extern int NDECL(done2); /**/
extern int NDECL(doengward); /**/
extern int NDECL(doengrave); /**/
extern int NDECL(doward); /**/
extern int NDECL(dopickup); /**/
extern int NDECL(ddoinv); /**/
extern int NDECL(dotypeinv); /**/
extern int NDECL(dolook); /**/
extern int NDECL(doprgold); /**/
extern int NDECL(doprwep); /**/
extern int NDECL(doprarm); /**/
extern int NDECL(doprring); /**/
extern int NDECL(dopramulet); /**/
extern int NDECL(doprtool); /**/
extern int NDECL(dosuspend); /**/
extern int NDECL(doforce); /**/
extern int NDECL(doopen); /**/
extern int NDECL(doclose); /**/
extern int NDECL(dosh); /**/
extern int NDECL(dodiscovered); /**/
extern int NDECL(doset); /**/
extern int NDECL(dotogglepickup); /**/
extern int NDECL(dowhatis); /**/
extern int NDECL(doquickwhatis); /**/
extern int NDECL(dowhatdoes); /**/
extern int NDECL(dohelp); /**/
extern int NDECL(dohistory); /**/
extern int NDECL(doloot); /**/
extern int NDECL(dodrink); /**/
extern int NDECL(dodip); /**/
extern int NDECL(dosacrifice); /**/
extern int NDECL(dopray); /**/
extern int NDECL(doturn); /**/
extern int NDECL(doredraw); /**/
extern int NDECL(doread); /**/
extern int NDECL(dosave); /**/
extern int NDECL(dosearch); /**/
extern int NDECL(doidtrap); /**/
extern int NDECL(dopay); /**/
extern int NDECL(dosit); /**/
extern int NDECL(dodeepswim); /**/
extern int NDECL(dotalk); /**/
extern int NDECL(docast); /**/
extern int NDECL(dovspell); /**/
extern int NDECL(dotele); /**/
extern int NDECL(dountrap); /**/
extern int NDECL(doversion); /**/
extern int NDECL(doextversion); /**/
extern int NDECL(doswapweapon); /**/
extern int NDECL(dowield); /**/
extern int NDECL(dowieldquiver); /**/
extern int NDECL(dozap); /**/
extern int NDECL(doorganize); /**/
#endif /* DUMB */

#ifdef OVL1
static int NDECL((*timed_occ_fn));
#endif /* OVL1 */

STATIC_DCL int NDECL(use_reach_attack);
STATIC_PTR int NDECL(doprev_message);
STATIC_PTR int NDECL(timed_occupation);
STATIC_PTR int NDECL(doextcmd);
STATIC_PTR int NDECL(domonability);
STATIC_PTR int NDECL(dooverview_or_wiz_where);
# ifdef WIZARD
STATIC_PTR int NDECL(wiz_wish);
STATIC_PTR int NDECL(wiz_identify);
STATIC_PTR int NDECL(wiz_map);
STATIC_PTR int NDECL(wiz_genesis);
STATIC_PTR int NDECL(wiz_where);
STATIC_PTR int NDECL(wiz_detect);
STATIC_PTR int NDECL(wiz_panic);
STATIC_PTR int NDECL(wiz_polyself);
STATIC_PTR int NDECL(wiz_level_tele);
STATIC_PTR int NDECL(wiz_level_change);
STATIC_PTR int NDECL(wiz_show_seenv);
STATIC_PTR int NDECL(wiz_show_vision);
STATIC_PTR int NDECL(wiz_mon_polycontrol);
STATIC_PTR int NDECL(wiz_show_wmodes);
#if defined(__BORLANDC__) && !defined(_WIN32)
extern void FDECL(show_borlandc_stats, (winid));
#endif
#ifdef DEBUG_MIGRATING_MONS
STATIC_PTR int NDECL(wiz_migrate_mons);
#endif
STATIC_DCL void FDECL(count_obj, (struct obj *, long *, long *, BOOLEAN_P, BOOLEAN_P));
STATIC_DCL void FDECL(obj_chain, (winid, const char *, struct obj *, long *, long *));
STATIC_DCL void FDECL(magic_chest_obj_chain, (winid, const char *, long *, long *));
STATIC_DCL void FDECL(mon_invent_chain, (winid, const char *, struct monst *, long *, long *));
STATIC_DCL void FDECL(mon_chain, (winid, const char *, struct monst *, long *, long *));
STATIC_DCL void FDECL(contained, (winid, const char *, long *, long *));
STATIC_PTR int NDECL(wiz_show_stats);
#  ifdef PORT_DEBUG
STATIC_DCL int NDECL(wiz_port_debug);
#  endif
# endif
STATIC_PTR int NDECL(enter_explore_mode);
STATIC_PTR int NDECL(doattributes);
STATIC_PTR int NDECL(doconduct); /**/
STATIC_PTR boolean NDECL(minimal_enlightenment);
STATIC_PTR void NDECL(resistances_enlightenment);
STATIC_PTR void NDECL(signs_enlightenment);

#ifdef OVLB
STATIC_DCL void FDECL(enlght_line, (const char *,const char *,const char *));
STATIC_DCL char *FDECL(enlght_combatinc, (const char *,int,int,char *));
#ifdef UNIX
static void NDECL(end_of_input);
#endif
#endif /* OVLB */

static const char* readchar_queue="";

STATIC_DCL char *NDECL(parse);
STATIC_DCL boolean FDECL(help_dir, (CHAR_P,const char *));

#ifdef OVL1

STATIC_PTR int
doprev_message(VOID_ARGS)
{
    return nh_doprev_message();
}

/* Count down by decrementing multi */
STATIC_PTR int
timed_occupation(VOID_ARGS)
{
	(*timed_occ_fn)();
	if (multi > 0)
		multi--;
	return multi > 0;
}

/* If you have moved since initially setting some occupations, they
 * now shouldn't be able to restart.
 *
 * The basic rule is that if you are carrying it, you can continue
 * since it is with you.  If you are acting on something at a distance,
 * your orientation to it must have changed when you moved.
 *
 * The exception to this is taking off items, since they can be taken
 * off in a number of ways in the intervening time, screwing up ordering.
 *
 *	Currently:	Take off all armor.
 *			Picking Locks / Forcing Chests.
 *			Setting traps.
 */
void
reset_occupations()
{
	reset_remarm();
	reset_pick();
	reset_trapset();
}

/* If a time is given, use it to timeout this function, otherwise the
 * function times out by its own means.
 */
void
set_occupation(fn, txt, xtime)
int NDECL((*fn));
const char *txt;
int xtime;
{
	if (xtime) {
		occupation = timed_occupation;
		timed_occ_fn = fn;
	} else
		occupation = fn;
	occtxt = txt;
	occtime = 0;
	return;
}

#ifdef REDO

static char NDECL(popch);

/* Provide a means to redo the last command.  The flag `in_doagain' is set
 * to true while redoing the command.  This flag is tested in commands that
 * require additional input (like `throw' which requires a thing and a
 * direction), and the input prompt is not shown.  Also, while in_doagain is
 * TRUE, no keystrokes can be saved into the saveq.
 */
#define BSIZE 20
static char pushq[BSIZE], saveq[BSIZE];
static NEARDATA int phead, ptail, shead, stail;

static char
popch() {
	/* If occupied, return '\0', letting tgetch know a character should
	 * be read from the keyboard.  If the character read is not the
	 * ABORT character (as checked in pcmain.c), that character will be
	 * pushed back on the pushq.
	 */
	if (occupation) return '\0';
	if (in_doagain) return(char)((shead != stail) ? saveq[stail++] : '\0');
	else		return(char)((phead != ptail) ? pushq[ptail++] : '\0');
}

char
pgetchar() {		/* curtesy of aeb@cwi.nl */
	register int ch;

	if(!(ch = popch()))
		ch = nhgetch();
	return((char)ch);
}

/* A ch == 0 resets the pushq */
void
pushch(ch)
char ch;
{
	if (!ch)
		phead = ptail = 0;
	if (phead < BSIZE)
		pushq[phead++] = ch;
	return;
}

/* A ch == 0 resets the saveq.	Only save keystrokes when not
 * replaying a previous command.
 */
void
savech(ch)
char ch;
{
	if (!in_doagain) {
		if (!ch)
			phead = ptail = shead = stail = 0;
		else if (shead < BSIZE)
			saveq[shead++] = ch;
	}
	return;
}
#endif /* REDO */

#endif /* OVL1 */
#ifdef OVLB

STATIC_PTR int
doextcmd(VOID_ARGS)	/* here after # - now read a full-word command */
{
	int idx, retval;

	/* keep repeating until we don't run help or quit */
	do {
	    idx = get_ext_cmd();
	    if (idx < 0) return 0;	/* quit */

	    retval = (*extcmdlist[idx].ef_funct)();
	} while (extcmdlist[idx].ef_funct == doextlist);

	return retval;
}

int
doextlist(VOID_ARGS)	/* here after #? - now list all full-word commands */
{
	register const struct ext_func_tab *efp;
	char	 buf[BUFSZ];
	winid datawin;

	datawin = create_nhwindow(NHW_TEXT);
	putstr(datawin, 0, "");
	putstr(datawin, 0, "            Extended Commands List");
	putstr(datawin, 0, "");
	putstr(datawin, 0, "    Press '#', then type:");
	putstr(datawin, 0, "");

	for(efp = extcmdlist; efp->ef_txt; efp++) {
		Sprintf(buf, "    %-15s - %s.", efp->ef_txt, efp->ef_desc);
		putstr(datawin, 0, buf);
	}
	display_nhwindow(datawin, FALSE);
	destroy_nhwindow(datawin);
	return 0;
}

#ifdef TTY_GRAPHICS
#define MAX_EXT_CMD 40		/* Change if we ever have > 40 ext cmds */
/*
 * This is currently used only by the tty port and is
 * controlled via runtime option 'extmenu'
 */
int
extcmd_via_menu()	/* here after # - now show pick-list of possible commands */
{
    const struct ext_func_tab *efp;
    menu_item *pick_list = (menu_item *)0;
    winid win;
    anything any;
    const struct ext_func_tab *choices[MAX_EXT_CMD];
    char buf[BUFSZ];
    char cbuf[QBUFSZ], prompt[QBUFSZ], fmtstr[20];
    int i, n, nchoices, acount;
    int ret,  biggest;
    int accelerator, prevaccelerator;
    int  matchlevel = 0;

    ret = 0;
    cbuf[0] = '\0';
    biggest = 0;
    while (!ret) {
	    i = n = 0;
	    accelerator = 0;
	    any.a_void = 0;
	    /* populate choices */
	    for(efp = extcmdlist; efp->ef_txt; efp++) {
		if (!matchlevel || !strncmp(efp->ef_txt, cbuf, matchlevel)) {
			choices[i++] = efp;
			if ((int)strlen(efp->ef_desc) > biggest) {
				biggest = strlen(efp->ef_desc);
				Sprintf(fmtstr,"%%-%ds", biggest + 15);
			}
#ifdef DEBUG
			if (i >= MAX_EXT_CMD - 2) {
			    impossible("Exceeded %d extended commands in doextcmd() menu",
					MAX_EXT_CMD - 2);
			    return 0;
			}
#endif
		}
	    }
	    choices[i] = (struct ext_func_tab *)0;
	    nchoices = i;
	    /* if we're down to one, we have our selection so get out of here */
	    if (nchoices == 1) {
		for (i = 0; extcmdlist[i].ef_txt != (char *)0; i++)
			if (!strncmpi(extcmdlist[i].ef_txt, cbuf, matchlevel)) {
				ret = i;
				break;
			}
		break;
	    }

	    /* otherwise... */
	    win = create_nhwindow(NHW_MENU);
	    start_menu(win);
	    prevaccelerator = 0;
	    acount = 0;
	    for(i = 0; choices[i]; ++i) {
		accelerator = choices[i]->ef_txt[matchlevel];
		if (accelerator != prevaccelerator || nchoices < (ROWNO - 3)) {
		    if (acount) {
 			/* flush the extended commands for that letter already in buf */
			Sprintf(buf, fmtstr, prompt);
			any.a_char = prevaccelerator;
			add_menu(win, NO_GLYPH, &any, any.a_char, 0,
					ATR_NONE, buf, FALSE);
			acount = 0;
		    }
		}
		prevaccelerator = accelerator;
		if (!acount || nchoices < (ROWNO - 3)) {
		    Sprintf(prompt, "%s [%s]", choices[i]->ef_txt,
				choices[i]->ef_desc);
		} else if (acount == 1) {
		    Sprintf(prompt, "%s or %s", choices[i-1]->ef_txt,
				choices[i]->ef_txt);
		} else {
		    Strcat(prompt," or ");
		    Strcat(prompt, choices[i]->ef_txt);
		}
		++acount;
	    }
	    if (acount) {
		/* flush buf */
		Sprintf(buf, fmtstr, prompt);
		any.a_char = prevaccelerator;
		add_menu(win, NO_GLYPH, &any, any.a_char, 0, ATR_NONE, buf, FALSE);
	    }
	    Sprintf(prompt, "Extended Command: %s", cbuf);
	    end_menu(win, prompt);
	    n = select_menu(win, PICK_ONE, &pick_list);
	    destroy_nhwindow(win);
	    if (n==1) {
		if (matchlevel > (QBUFSZ - 2)) {
			free((genericptr_t)pick_list);
#ifdef DEBUG
			impossible("Too many characters (%d) entered in extcmd_via_menu()",
				matchlevel);
#endif
			ret = -1;
		} else {
			cbuf[matchlevel++] = pick_list[0].item.a_char;
			cbuf[matchlevel] = '\0';
			free((genericptr_t)pick_list);
		}
	    } else {
		if (matchlevel) {
			ret = 0;
			matchlevel = 0;
		} else
			ret = -1;
	    }
    }
    return ret;
}
#endif

/* #monster command - use special monster ability while polymorphed */
STATIC_PTR int
domonability(VOID_ARGS)
{
	winid tmpwin;
	int n, how;
	char buf[BUFSZ];
	char incntlet = 'a';
	menu_item *selected;
	anything any;
	boolean atleastone = FALSE;
	
	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */
	
	Sprintf(buf, "Attacks");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	if(uarm && uarms && 
		Is_dragon_armor(uarm) && Is_dragon_shield(uarms) && 
		Have_same_dragon_armor_and_shield &&
		uarm->age < monstermoves && uarms->age < monstermoves
	){
		Sprintf(buf, "Armor's Breath Weapon");
		any.a_int = MATTK_DSCALE;	/* must be non-zero */
		incntlet = 'a';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(is_were(youmonst.data)){
		Sprintf(buf, "Summon Aid");
		any.a_int = MATTK_SUMM;	/* must be non-zero */
		incntlet = 'A';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(can_breathe(youmonst.data) || Race_if(PM_HALF_DRAGON)){
		Sprintf(buf, "Breath Weapon");
		any.a_int = MATTK_BREATH;	/* must be non-zero */
		incntlet = 'b';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(youmonst.data == &mons[PM_TOVE]){
		Sprintf(buf, "Bore Hole");
		any.a_int = MATTK_HOLE;	/* must be non-zero */
		incntlet = 'B';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(uclockwork){
		Sprintf(buf, "Adjust Clock");
		any.a_int = MATTK_CLOCK;	/* must be non-zero */
		incntlet = 'c';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(Race_if(PM_HALF_DRAGON) && Role_if(PM_BARD) && u.ulevel >= 14){
		Sprintf(buf, "Sing an Elemental into being");
		any.a_int = MATTK_ELMENTAL;	/* must be non-zero */
		incntlet = 'E';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(attacktype(youmonst.data, AT_GAZE)){
		Sprintf(buf, "Gaze");
		any.a_int = MATTK_GAZE;	/* must be non-zero */
		incntlet = 'g';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(is_hider(youmonst.data)){
		Sprintf(buf, "Hide");
		any.a_int = MATTK_HIDE;	/* must be non-zero */
		incntlet = 'h';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(is_drow(youmonst.data)){
		Sprintf(buf, "Invoke Darkness");
		any.a_int = MATTK_DARK;	/* must be non-zero */
		incntlet = 'i';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(youmonst.data->mlet == S_NYMPH){
		Sprintf(buf, "Remove Iron Ball");
		any.a_int = MATTK_REMV;	/* must be non-zero */
		incntlet = 'I';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(is_mind_flayer(youmonst.data)){
		Sprintf(buf, "Mind Blast");
		any.a_int = MATTK_MIND;	/* must be non-zero */
		incntlet = 'm';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(attacktype(youmonst.data, AT_MAGC)){
		Sprintf(buf, "Monster Spells");
		any.a_int = MATTK_MAGIC;	/* must be non-zero */
		incntlet = 'M';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(youmonst.data == &mons[PM_BANDERSNATCH]){
		Sprintf(buf, "Reach Attack");
		any.a_int = MATTK_REACH;	/* must be non-zero */
		incntlet = 'r';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(u.umonnum == PM_GREMLIN){
		Sprintf(buf, "Replicate");
		any.a_int = MATTK_REPL;	/* must be non-zero */
		incntlet = 'R';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(attacktype(youmonst.data, AT_SPIT)){
		Sprintf(buf, "Spit");
		any.a_int = MATTK_SPIT;	/* must be non-zero */
		incntlet = 's';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(youmonst.data->msound == MS_SHRIEK){
		Sprintf(buf, "Shriek");
		any.a_int = MATTK_SHRIEK;	/* must be non-zero */
		incntlet = 'S';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(youmonst.data->msound == MS_JUBJUB){
		Sprintf(buf, "Scream");
		any.a_int = MATTK_SCREAM;	/* must be non-zero */
		incntlet = 'S';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(is_unicorn(youmonst.data)){
		Sprintf(buf, "Unicorn Horn");
		any.a_int = MATTK_UHORN;	/* must be non-zero */
		incntlet = 'u';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(webmaker(youmonst.data)){
		Sprintf(buf, "Make Web");
		any.a_int = MATTK_WEBS;	/* must be non-zero */
		incntlet = 'w';
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		atleastone = TRUE;
	}
	if(!atleastone){
		if(Upolyd) pline("Any special ability you may have is purely reflexive.");
		else You("don't have a special ability in your normal form!");
		return 0;
	}
	
	end_menu(tmpwin, "Choose which attack to use");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	
	if(n <= 0) return 0;
	
	switch (selected[0].item.a_int) {
	case MATTK_BREATH: return dobreathe(youmonst.data);
	case MATTK_DSCALE:{
		int res = dobreathe(Dragon_shield_to_pm(uarms));
		if(res){
			uarm->age = monstermoves + (long)(rnz(100)*(Role_if(PM_CAVEMAN) ? .8 : 1));
			uarms->age= monstermoves + (long)(rnz(100)*(Role_if(PM_CAVEMAN) ? .8 : 1));
		}
		return res;
	}
	case MATTK_SPIT: return dospit();
	case MATTK_MAGIC: return castum((struct monst *)0,
	                   &youmonst.data->mattk[attackindex(youmonst.data, 
			                         AT_MAGC,AD_ANY)]);
	case MATTK_REMV: return doremove();
	case MATTK_GAZE: return dogaze();
	case MATTK_SUMM: return dosummon();
	case MATTK_WEBS: return dospinweb();
	case MATTK_HIDE: return dohide();
	case MATTK_MIND: return domindblast();
	case MATTK_CLOCK: return doclockspeed();
	case MATTK_ELMENTAL: return doelementalbreath();
	case MATTK_DARK: return dodarken();
	case MATTK_REPL: {
	    if(IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
			if (split_mon(&youmonst, (struct monst *)0))
				dryup(u.ux, u.uy, TRUE);
			return 1;
	    } else {
			There("is no fountain here.");
			return 0;
		}
	}
	break;
	case MATTK_UHORN: {
	    use_unicorn_horn((struct obj *)0);
	    return 1;
	}
	break;
	case MATTK_SHRIEK: {
	    You("shriek.");
	    if(u.uburied) pline("Unfortunately sound does not carry well through rock.");
	    else aggravate();
		return 1;
	}
	break;
	case MATTK_SCREAM: {
	    You("scream high and shrill.");
	    if(u.uburied) pline("Unfortunately sound does not carry well through rock.");
	    else{
			struct monst *tmpm;
			for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
				if(tmpm->mtame && tmpm->mtame<20) tmpm->mtame++;
				if(d(1,tmpm->mhp) < Upolyd ? u.mh : u.uhp){
					tmpm->mflee = 1;
				}
			}
		}
		return 1;
	}
	break;
	case MATTK_HOLE: {
		struct trap *ttmp = t_at(u.ux, u.uy);
		struct rm *lev = &levl[u.ux][u.uy];
		schar typ;
		boolean nohole = !Can_dig_down(&u.uz);
		if (!(
			!isok(u.ux,u.uy) || 
			(ttmp && ttmp->ttyp == MAGIC_PORTAL) ||
		   /* ALI - artifact doors from slash'em */
			(IS_DOOR(levl[u.ux][u.uy].typ) && artifact_door(u.ux, u.uy)) ||
			(IS_ROCK(lev->typ) && lev->typ != SDOOR &&
			(lev->wall_info & W_NONDIGGABLE) != 0) ||
			(is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy)) ||
			(lev->typ == DRAWBRIDGE_DOWN ||
			   (is_drawbridge_wall(u.ux, u.uy) >= 0)) ||
			(boulder_at(u.ux, u.uy)) ||
			(IS_GRAVE(lev->typ)) ||
			(lev->typ == DRAWBRIDGE_UP) ||
			(IS_THRONE(lev->typ)) ||
			(IS_ALTAR(lev->typ))
		)){
			typ = fillholetyp(u.ux,u.uy);
			You("gyre and gimble into the %s.", surface(u.ux,u.uy));
			if (typ != ROOM) {
				lev->typ = typ;
				if (ttmp) (void) delfloortrap(ttmp);
				/* if any objects were frozen here, they're released now */
				unearth_objs(u.ux, u.uy);

					if(cansee(u.ux, u.uy))
						pline_The("hole fills with %s!",
						  typ == LAVAPOOL ? "lava" : "water");
				if (!Levitation && !Flying) {
					if (typ == LAVAPOOL)
					(void) lava_effects();
					else if (!Wwalking)
					(void) drown();
				}
			}
			if (nohole || !ttmp || (ttmp->ttyp != PIT && ttmp->ttyp != SPIKED_PIT && ttmp->ttyp != TRAPDOOR))
				digactualhole(u.ux, u.uy, &youmonst, PIT, FALSE, TRUE);
			else
				digactualhole(u.ux, u.uy, &youmonst, HOLE, FALSE, TRUE);
			return 1;
		} else {
			You("gyre and gimble, but the %s is too hard!", surface(u.ux,u.uy));
			return 1;
		}
	}
	break;
	case MATTK_REACH: return use_reach_attack();
	break;
	}
	return 0;
}

STATIC_OVL int
use_reach_attack()
{
	int res = 0, typ, max_range = 4, min_range = 1;
	coord cc;
	struct monst *mtmp;


	/* Are you allowed to use a reach attack? */
	if (u.uswallow) {
	    pline("There's not enough room here to use that.");
	    return (0);
	}
	/* Prompt for a location */
	pline("Where do you want to hit?");
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "the spot to hit") < 0)
	    return 0;	/* user pressed ESC */

	/* Calculate range */
	typ = P_BARE_HANDED_COMBAT;
	if (typ == P_NONE || P_SKILL(typ) <= P_BASIC) max_range = 4;
	else if ( P_SKILL(typ) == P_SKILLED) max_range = 5;
	else if ( P_SKILL(typ) == P_EXPERT) max_range = 8;
	else if ( P_SKILL(typ) == P_MASTER) max_range = 9;
	else max_range = 10;
	if (distu(cc.x, cc.y) > max_range) {
	    pline("Too far!");
	    return (res);
	} else if (distu(cc.x, cc.y) < min_range) {
	    pline("Too close!");
	    return (res);
	} else if (!cansee(cc.x, cc.y) &&
		   ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0 ||
		    !canseemon(mtmp))) {
	    You("won't hit anything if you can't see that spot.");
	    return (res);
	} else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
	    You("can't reach that spot from here.");
	    return res;
	}

	/* Attack the monster there */
	if ((mtmp = m_at(cc.x, cc.y)) != (struct monst *)0) {
		int tmp, tmpw, tmpt;

	    bhitpos = cc;
	    check_caitiff(mtmp);
		
		find_to_hit_rolls(mtmp, &tmp, &tmpw, &tmpt);
		
	    (void) hmonas(mtmp, youmonst.data, tmp, tmpw, tmpt);
	} else
	    /* Now you know that nothing is there... */
	    pline("%s", nothing_happens);
	return (1);
}

int
dofightingform()
{
	winid tmpwin;
	int n, how;
	char buf[BUFSZ];
	char incntlet = 'a';
	menu_item *selected;
	anything any;
	
	if(!(uwep && is_lightsaber(uwep))){
		pline("You don't know any special fighting styles for use in this situation.");
		return 0;
	}
	
	if(P_SKILL(weapon_type(uwep)) < P_BASIC){
		pline("You must have at least some basic skill in the use of your weapon before you can employ special fighting styles.");
		return 0;
	}

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */
	
	Sprintf(buf,	"Known Forms");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	if(P_SKILL(FFORM_SHII_CHO) >= P_BASIC){
		if(u.fightingForm == FFORM_SHII_CHO) {
			Sprintf(buf,	"Shii-Cho (active)");
		} else {
			Sprintf(buf,	"Shii-Cho");
		}
		any.a_int = FFORM_SHII_CHO;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(P_SKILL(FFORM_MAKASHI) >= P_BASIC){
		if(u.fightingForm == FFORM_MAKASHI) {
			if(uarm && !(is_light_armor(uarm) || is_medium_armor(uarm))){
				Sprintf(buf,	"Makashi (selected; blocked by armor)");
			} else {
				Sprintf(buf,	"Makashi (active)");
			}
		} else {
			if(uarm && !(is_light_armor(uarm) || is_medium_armor(uarm))){
				Sprintf(buf,	"Makashi (blocked by armor)");
			} else {
				Sprintf(buf,	"Makashi");
			}
		}
		any.a_int = FFORM_MAKASHI;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(P_SKILL(FFORM_SORESU) >= P_BASIC){
		if(u.fightingForm == FFORM_SORESU) {
			if(uarm && !(is_light_armor(uarm) || is_medium_armor(uarm))){
				Sprintf(buf,	"Soresu (selected; blocked by armor)");
			} else {
				Sprintf(buf,	"Soresu (active)");
			}
		} else {
			if(uarm && !(is_light_armor(uarm) || is_medium_armor(uarm))){
				Sprintf(buf,	"Soresu (blocked by armor)");
			} else {
				Sprintf(buf,	"Soresu");
			}
		}
		any.a_int = FFORM_SORESU;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(P_SKILL(FFORM_ATARU) >= P_BASIC){
		if(u.fightingForm == FFORM_ATARU) {
			if(uarm && !(is_light_armor(uarm))){
				Sprintf(buf,	"Ataru (selected; blocked by armor)");
			} else {
				Sprintf(buf,	"Ataru (active)");
			}
		} else {
			if(uarm && !(is_light_armor(uarm))){
				Sprintf(buf,	"Ataru (blocked by armor)");
			} else {
				Sprintf(buf,	"Ataru");
			}
		}
		any.a_int = FFORM_ATARU;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(P_SKILL(FFORM_SHIEN) >= P_BASIC){
		if(u.fightingForm == FFORM_SHIEN) {
			if(uarm && !(is_light_armor(uarm))){
				Sprintf(buf,	"Shien (selected; blocked by armor)");
			} else {
				Sprintf(buf,	"Shien (active)");
			}
		} else {
			if(uarm && !(is_light_armor(uarm))){
				Sprintf(buf,	"Shien (blocked by armor)");
			} else {
				Sprintf(buf,	"Shien");
			}
		}
		any.a_int = FFORM_SHIEN;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(P_SKILL(FFORM_DJEM_SO) >= P_BASIC){
		if(u.fightingForm == FFORM_DJEM_SO) {
			if(uarm && !(is_light_armor(uarm) || is_medium_armor(uarm))){
				Sprintf(buf,	"Djem So (selected; blocked by armor)");
			} else {
				Sprintf(buf,	"Djem So (active)");
			}
		} else {
			if(uarm && !(is_light_armor(uarm) || is_medium_armor(uarm))){
				Sprintf(buf,	"Djem So (blocked by armor)");
			} else {
				Sprintf(buf,	"Djem So");
			}
		}
		any.a_int = FFORM_DJEM_SO;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(P_SKILL(FFORM_NIMAN) >= P_BASIC){
		if(u.fightingForm == FFORM_NIMAN) {
			if(uarm && (is_metallic(uarm))){
				Sprintf(buf,	"Niman (selected; blocked by armor)");
			} else {
				Sprintf(buf,	"Niman (active)");
			}
		} else {
			if(uarm && (is_metallic(uarm))){
				Sprintf(buf,	"Niman (blocked by armor)");
			} else {
				Sprintf(buf,	"Niman");
			}
		}
		any.a_int = FFORM_NIMAN;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(P_SKILL(FFORM_JUYO) >= P_BASIC){
		if(u.fightingForm == FFORM_JUYO) {
			if(uarm && !(is_light_armor(uarm))){
				Sprintf(buf,	"Juyo (selected; blocked by armor)");
			} else {
				Sprintf(buf,	"Juyo (active)");
			}
		} else {
			if(uarm && !(is_light_armor(uarm))){
				Sprintf(buf,	"Juyo (blocked by armor)");
			} else {
				Sprintf(buf,	"Juyo");
			}
		}
		any.a_int = FFORM_JUYO;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	end_menu(tmpwin,	"Choose fighting style:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	
	if(n <= 0 || u.fightingForm == selected[0].item.a_int){
		return 0;
	} else {
		u.fightingForm = selected[0].item.a_int;
		return 0;
	}
}

STATIC_PTR int
enter_explore_mode(VOID_ARGS)
{
#ifdef PARANOID
	char buf[BUFSZ];
	int really_xplor = FALSE;
#endif
	pline("Explore mode is for local games, not public servers.");
	return 0;

	if(!discover && !wizard) {
		pline("Beware!  From explore mode there will be no return to normal game.");
#ifdef PARANOID
		if (iflags.paranoid_quit) {
		  getlin ("Do you want to enter explore mode? [yes/no]?",buf);
		  (void) lcase (buf);
		  if (!(strcmp (buf, "yes"))) really_xplor = TRUE;
		} else {
		  if (yn("Do you want to enter explore mode?") == 'y') {
		    really_xplor = TRUE;
		  }
		}
		if (really_xplor) {
#else
		if (yn("Do you want to enter explore mode?") == 'y') {
#endif
			clear_nhwindow(WIN_MESSAGE);
			You("are now in non-scoring explore mode.");
			discover = TRUE;
		}
		else {
			clear_nhwindow(WIN_MESSAGE);
			pline("Resuming normal game.");
		}
	}
	return 0;
}

STATIC_PTR int
dooverview_or_wiz_where()
{
#ifdef WIZARD
	if (wizard) return wiz_where();
	else
#endif
	dooverview();
	return 0;
}

#ifdef WIZARD

/* ^W command - wish for something */
STATIC_PTR int
wiz_wish(VOID_ARGS)	/* Unlimited wishes for debug mode by Paul Polderman */
{
	if (wizard) {
	    boolean save_verbose = flags.verbose;

	    flags.verbose = FALSE;
	    makewish();
	    flags.verbose = save_verbose;
	    (void) encumber_msg();
	} else
	    pline("Unavailable command '^W'.");
	return 0;
}

/* ^I command - identify hero's inventory */
STATIC_PTR int
wiz_identify(VOID_ARGS)
{
	if (wizard)	identify_pack(0);
	else		pline("Unavailable command '^I'.");
	return 0;
}

/* ^F command - reveal the level map and any traps on it */
STATIC_PTR int
wiz_map(VOID_ARGS)
{
	if (wizard) {
	    struct trap *t;
	    long save_Hconf = HConfusion,
		 save_Hhallu = HHallucination;

	    HConfusion = HHallucination = 0L;
	    for (t = ftrap; t != 0; t = t->ntrap) {
		t->tseen = 1;
		map_trap(t, TRUE);
	    }
	    do_mapping();
	    HConfusion = save_Hconf;
	    HHallucination = save_Hhallu;
	} else
	    pline("Unavailable command '^F'.");
	return 0;
}

/* ^G command - generate monster(s); a count prefix will be honored */
STATIC_PTR int
wiz_genesis(VOID_ARGS)
{
	if (wizard)	(void) create_particular();
	else		pline("Unavailable command '^G'.");
	return 0;
}

/* ^O command - display dungeon layout */
STATIC_PTR int
wiz_where(VOID_ARGS)
{
	if (wizard) (void) print_dungeon(FALSE, (schar *)0, (xchar *)0);
	else	    pline("Unavailable command '^O'.");
	return 0;
}

/* ^E command - detect unseen (secret doors, traps, hidden monsters) */
STATIC_PTR int
wiz_detect(VOID_ARGS)
{
	if(wizard)  (void) findit();
	else	    pline("Unavailable command '^E'.");
	return 0;
}

/* ^V command - level teleport */
STATIC_PTR int
wiz_level_tele(VOID_ARGS)
{
	if (wizard)	level_tele();
	else		pline("Unavailable command '^V'.");
	return 0;
}

/* #monpolycontrol command - choose new form for shapechangers, polymorphees */
STATIC_PTR int
wiz_mon_polycontrol(VOID_ARGS)
{
    iflags.mon_polycontrol = !iflags.mon_polycontrol;
    pline("Monster polymorph control is %s.",
	  iflags.mon_polycontrol ? "on" : "off");
    return 0;
}

/* #levelchange command - adjust hero's experience level */
STATIC_PTR int
wiz_level_change(VOID_ARGS)
{
    char buf[BUFSZ];
    int newlevel;
    int ret;

    getlin("To what experience level do you want to be set?", buf);
    (void)mungspaces(buf);
    if (buf[0] == '\033' || buf[0] == '\0') ret = 0;
    else ret = sscanf(buf, "%d", &newlevel);

    if (ret != 1) {
	pline1(Never_mind);
	return 0;
    }
    if (newlevel == u.ulevel) {
	You("are already that experienced.");
    } else if (newlevel < u.ulevel) {
	if (u.ulevel == 1) {
	    You("are already as inexperienced as you can get.");
	    return 0;
	}
	if (newlevel < 1) newlevel = 1;
	while (u.ulevel > newlevel)
	    losexp("#levelchange",TRUE,TRUE,TRUE);
    } else {
	if (u.ulevel >= MAXULEV) {
	    You("are already as experienced as you can get.");
	    return 0;
	}
	if (newlevel > MAXULEV) newlevel = MAXULEV;
	while (u.ulevel < newlevel)
	    pluslvl(FALSE);
    }
    u.ulevelmax = u.ulevel;
    return 0;
}

/* #panic command - test program's panic handling */
STATIC_PTR int
wiz_panic(VOID_ARGS)
{
	if (yn("Do you want to call panic() and end your game?") == 'y')
		panic("crash test.");
        return 0;
}

/* #polyself command - change hero's form */
STATIC_PTR int
wiz_polyself(VOID_ARGS)
{
        polyself(TRUE);
        return 0;
}

/* #seenv command */
STATIC_PTR int
wiz_show_seenv(VOID_ARGS)
{
	winid win;
	int x, y, v, startx, stopx, curx;
	char row[COLNO+1];

	win = create_nhwindow(NHW_TEXT);
	/*
	 * Each seenv description takes up 2 characters, so center
	 * the seenv display around the hero.
	 */
	startx = max(1, u.ux-(COLNO/4));
	stopx = min(startx+(COLNO/2), COLNO);
	/* can't have a line exactly 80 chars long */
	if (stopx - startx == COLNO/2) startx++;

	for (y = 0; y < ROWNO; y++) {
	    for (x = startx, curx = 0; x < stopx; x++, curx += 2) {
		if (x == u.ux && y == u.uy) {
		    row[curx] = row[curx+1] = '@';
		} else {
		    v = levl[x][y].seenv & 0xff;
		    if (v == 0)
			row[curx] = row[curx+1] = ' ';
		    else
			Sprintf(&row[curx], "%02x", v);
		}
	    }
	    /* remove trailing spaces */
	    for (x = curx-1; x >= 0; x--)
		if (row[x] != ' ') break;
	    row[x+1] = '\0';

	    putstr(win, 0, row);
	}
	display_nhwindow(win, TRUE);
	destroy_nhwindow(win);
	return 0;
}

/* #vision command */
STATIC_PTR int
wiz_show_vision(VOID_ARGS)
{
	winid win;
	int x, y, v;
	char row[COLNO+1];

	win = create_nhwindow(NHW_TEXT);
	Sprintf(row, "Flags: 0x%x could see, 0x%x in sight, 0x%x temp lit",
		COULD_SEE, IN_SIGHT, TEMP_LIT);
	putstr(win, 0, row);
	putstr(win, 0, "");
	for (y = 0; y < ROWNO; y++) {
	    for (x = 1; x < COLNO; x++) {
		if (x == u.ux && y == u.uy)
		    row[x] = '@';
		else {
		    v = viz_array[y][x]; /* data access should be hidden */
		    if (v == 0)
			row[x] = ' ';
		    else
			row[x] = '0' + viz_array[y][x];
		}
	    }
	    /* remove trailing spaces */
	    for (x = COLNO-1; x >= 1; x--)
		if (row[x] != ' ') break;
	    row[x+1] = '\0';

	    putstr(win, 0, &row[1]);
	}
	display_nhwindow(win, TRUE);
	destroy_nhwindow(win);
	return 0;
}

/* #wmode command */
STATIC_PTR int
wiz_show_wmodes(VOID_ARGS)
{
	winid win;
	int x,y;
	char row[COLNO+1];
	struct rm *lev;

	win = create_nhwindow(NHW_TEXT);
	for (y = 0; y < ROWNO; y++) {
	    for (x = 0; x < COLNO; x++) {
		lev = &levl[x][y];
		if (x == u.ux && y == u.uy)
		    row[x] = '@';
		else if (IS_WALL(lev->typ) || lev->typ == SDOOR)
		    row[x] = '0' + (lev->wall_info & WM_MASK);
		else if (lev->typ == CORR)
		    row[x] = '#';
		else if (IS_ROOM(lev->typ) || IS_DOOR(lev->typ))
		    row[x] = '.';
		else
		    row[x] = 'x';
	    }
	    row[COLNO] = '\0';
	    putstr(win, 0, row);
	}
	display_nhwindow(win, TRUE);
	destroy_nhwindow(win);
	return 0;
}

#endif /* WIZARD */


/* -enlightenment and conduct- */
static winid en_win;
static const char
	You_[] = "You ",
	are[]  = "are ",  were[]  = "were ",
	have[] = "have ", had[]   = "had ",
	can[]  = "can ",  could[] = "could ";
static const char
	have_been[]  = "have been ",
	have_never[] = "have never ", never[] = "never ";

#define enl_msg(prefix,present,past,suffix) \
			enlght_line(prefix, final ? past : present, suffix)
#define you_are(attr)	enl_msg(You_,are,were,attr)
#define you_have(attr)	enl_msg(You_,have,had,attr)
#define you_can(attr)	enl_msg(You_,can,could,attr)
#define you_have_been(goodthing) enl_msg(You_,have_been,were,goodthing)
#define you_have_never(badthing) enl_msg(You_,have_never,never,badthing)
#define you_have_X(something)	enl_msg(You_,have,(const char *)"",something)

static void
enlght_line(start, middle, end)
const char *start, *middle, *end;
{
	char buf[BUFSZ];

	Sprintf(buf, "%s%s%s.", start, middle, end);
	putstr(en_win, 0, buf);
}

/* format increased damage or chance to hit */
static char *
enlght_combatinc(inctyp, incamt, final, outbuf)
const char *inctyp;
int incamt, final;
char *outbuf;
{
	char numbuf[24];
	const char *modif, *bonus;

	if (final
#ifdef WIZARD
		|| wizard
#endif
	  ) {
	    Sprintf(numbuf, "%s%d",
		    (incamt > 0) ? "+" : "", incamt);
	    modif = (const char *) numbuf;
	} else {
	    int absamt = abs(incamt);

	    if (absamt <= 3) modif = "small";
	    else if (absamt <= 6) modif = "moderate";
	    else if (absamt <= 12) modif = "large";
	    else modif = "huge";
	}
	bonus = (incamt > 0) ? "bonus" : "penalty";
	/* "bonus to hit" vs "damage bonus" */
	if (!strcmp(inctyp, "damage")) {
	    const char *ctmp = inctyp;
	    inctyp = bonus;
	    bonus = ctmp;
	}
	Sprintf(outbuf, "%s %s %s", an(modif), bonus, inctyp);
	return outbuf;
}

void
enlightenment(final)
int final;	/* 0 => still in progress; 1 => over, survived; 2 => dead */
{
	int ltmp;
	char buf[BUFSZ];
	char prebuf[BUFSZ];

	en_win = create_nhwindow(NHW_MENU);
	putstr(en_win, 0, final ? "Final Attributes:" : "Current Attributes:");
	putstr(en_win, 0, "");

	if (u.uevent.uhand_of_elbereth) {
	    static const char * const hofe_titles[30] = {
				/* Default */
				"the Arm of the Law",		 /*01*/
				"the Envoy of Balance",		 /*02*/
				"the Glory of Arioch",		 /*03*/
				/* Monk */
				"the Sage of Law",			 /*04*/
				"the Grandmaster of Balance",/*05*/
				"the Glory of Eequor",		 /*06*/
				/* Noble (human, vampire, incant) */
				"the Saint %s",				 /*07*/
				"the Grey Saint",			 /*08*/
				"the Dark %s",				 /*09*/
				/* Wizard */
				"the Magister of Law",		 /*10*/
				"the Wizard of Balance",	 /*11*/
				"the Glory of Chardros",	 /*12*/
				/* Elf */
				"the Hand of Elbereth",		 /*13*/
				"the Doomspeaker of Vaire",	 /*14*/
				"the Whisperer of Este",	 /*15*/
				/* Drow */
				"the Hand of Eilistraee",	 /*16*/
				"the Hand of Kiaransali",	 /*17*/
				"the Hand of Lolth",		 /*18*/
				/* Hedrow */
				"the Shepherd of spiders",	 /*19*/
				"the Sword of Vhaeraun",	 /*20*/
				"the Fang of Lolth",		 /*21*/
				/* Drow Noble */
				"the Blade of Ver'tas",		 /*22*/
				"the Hand of Kiaransali",	 /*23*/
				"the Hand of Lolth",		 /*24*/
				/* Hedrow Noble */
				"the Sword of Selvetarm",	 /*25*/
				"the Hand of Keptolo",		 /*26*/
				"the Hammer of Ghaunadaur",	 /*27*/
				/* Ranger */
				"the High %s of Apollo",	 /*28*/
				"the High %s of Latona",	 /*29*/
				"the High %s of Diana",	 	 /*30*/
				
				/* uhand_of_elbereth max == 31 *AS IN, YOU'RE NOW OUT OF ROOM* */
	    };
		
	    if(Role_if(PM_EXILE)) you_are("the Emissary of Elements");
	    else if(Pantheon_if(PM_PIRATE) || Role_if(PM_PIRATE)) you_are("the Pirate King");
	    else if((Pantheon_if(PM_KNIGHT) || Role_if(PM_KNIGHT)) && u.uevent.uhand_of_elbereth == 1) you_are("the King of the Angles");
	    else if((Pantheon_if(PM_VALKYRIE) || Role_if(PM_VALKYRIE)) && flags.initgend) you_are("the Daughter of Skadi");
	    else if(Race_if(PM_DWARF) && (urole.ldrnum == PM_THORIN_II_OAKENSHIELD || urole.ldrnum == PM_DAIN_II_IRONFOOT)){
			if(urole.ldrnum == PM_THORIN_II_OAKENSHIELD) you_are("King under the Mountain");
			else if(urole.ldrnum == PM_DAIN_II_IRONFOOT) you_are("Lord of Moria");
	    } else if((Pantheon_if(PM_SAMURAI) || Role_if(PM_SAMURAI)) && u.uevent.uhand_of_elbereth == 1){
			strcpy(buf, "Nasu no ");
			strcat(buf, plname);
			you_are(buf);
		} else if(u.uevent.uhand_of_elbereth >= 28 && u.uevent.uhand_of_elbereth <= 30){
			Sprintf(buf, hofe_titles[u.uevent.uhand_of_elbereth - 1], flags.female ? "Priestess" : "Priest");
			enl_msg("You ", "are ", "were ", buf);
		} else if(Role_if(PM_NOBLEMAN) && !Race_if(PM_DROW)){
			if(u.uevent.uhand_of_elbereth == 9) Sprintf(buf, hofe_titles[u.uevent.uhand_of_elbereth - 1], flags.female ? "Lady" : "Lord");
			else if(u.uevent.uhand_of_elbereth == 7) Sprintf(buf, hofe_titles[u.uevent.uhand_of_elbereth - 1], flags.female ? "Queen" : "King");
			else Sprintf(buf, " %s", hofe_titles[u.uevent.uhand_of_elbereth - 1]);
			enl_msg("You ", "are ", "were ", buf);
		} else you_are(hofe_titles[u.uevent.uhand_of_elbereth - 1]);
	}
	
	if(u.lastprayed){
		Sprintf(buf, "You last %s %ld turns ago", u.lastprayresult==PRAY_GIFT ? "recieved a gift" :
												  u.lastprayresult==PRAY_ANGER ? "angered your god" : 
												  u.lastprayresult==PRAY_CONV ? "converted to a new god" : 
												  "prayed",
			moves - u.lastprayed);
		putstr(en_win, 0, buf);
		if(u.lastprayresult==PRAY_GOOD){
			Sprintf(buf, "That prayer was well recieved");
			putstr(en_win, 0, buf);
		} else if(u.lastprayresult==PRAY_BAD){
			Sprintf(buf, "That prayer was poorly recieved");
			putstr(en_win, 0, buf);
		}
		if(u.reconciled){
			if(u.reconciled==REC_REC){
				Sprintf(buf, " since reconciled with your god");
				enl_msg("You ", "have","had",buf);
			} else if(u.reconciled==REC_MOL){
				Sprintf(buf, " since mollified your god");
				enl_msg("You ", "have","had",buf);
			}
		}
	}
	
	/* note: piousness 20 matches MIN_QUEST_ALIGN (quest.h) */
	if (u.ualign.record >= 20)	you_are("piously aligned");
	else if (u.ualign.record > 13)	you_are("devoutly aligned");
	else if (u.ualign.record > 8)	you_are("fervently aligned");
	else if (u.ualign.record > 3)	you_are("stridently aligned");
	else if (u.ualign.record == 3)	you_are("aligned");
	else if (u.ualign.record > 0)	you_are("haltingly aligned");
	else if (u.ualign.record == 0)	you_are("nominally aligned");
	else if (u.ualign.record >= -3)	you_have("strayed");
	else if (u.ualign.record >= -8)	you_have("sinned");
	else you_have("transgressed");
#ifdef WIZARD
	if (wizard) {
		Sprintf(buf, " %d", u.ualign.record);
		enl_msg("Your alignment ", "is", "was", buf);
		Sprintf(buf, " %d sins", u.ualign.sins);
		enl_msg("You ", "carry", "carried", buf);
		Sprintf(buf, " %d", (int) ALIGNLIM);
		enl_msg("Your max alignment ", "is", "was", buf);
		if(flags.stag) enl_msg("You ", "have","had"," turned stag on your quest leader");
		else enl_msg("You ", "have","had"," stayed true to your quest");
		if(flags.leader_backstab) enl_msg("You ", "have","had"," been betrayed by your quest leader");
		Sprintf(buf, "a hod wantedness of %d", u.hod);
		you_have(buf);
		Sprintf(buf, "a gevurah wantedness of %d", u.gevurah);
		you_have(buf);
		Sprintf(buf, "a chokhmah wantedness of %d", u.keter);
		you_have(buf);
		Sprintf(buf, "%d chokhmah sephiroth ", u.chokhmah);
		enl_msg(buf, "are", "were", " deployed");
		Sprintf(buf, "%d weakness from being studied", u.ustdy);
		you_have(buf);
		Sprintf(buf, "spirits bound: %d", u.sealCounts);
		you_have(buf);
		Sprintf(buf, "seals active: %lx", u.sealsActive);
		you_have(buf);
		Sprintf(buf, "special seals active: %lx", u.specialSealsActive);
		you_have(buf);
	}
#endif
	
	if(u.sealsActive || u.specialSealsActive){
		int i,j,numBound,numFound=0;
		numBound = u.sealCounts;
		if(u.spirit[QUEST_SPIRIT]) numBound++;
		if(Role_if(PM_EXILE) && u.uevent.uhand_of_elbereth) numBound++;
		if(u.spirit[CROWN_SPIRIT]) numBound++;
		if(u.spirit[GPREM_SPIRIT]) numBound++;
		if(u.spirit[ALIGN_SPIRIT]) numBound++;
		if(u.spirit[OUTER_SPIRIT]) numBound++;
		Sprintf(prebuf, "Your soul ");
		Sprintf(buf, " bound to ");
		for(i=0;i<QUEST_SPIRIT;i++){
			if(u.spirit[i]) for(j=0;j<32;j++){
				if((u.spirit[i] >> j) == 1){
					Strcat(buf,sealNames[j]);
					numFound++;
					if(numBound==2 && numFound==1) Strcat(buf," and ");
					else if(numBound>=3){
						if(numFound<numBound-1) Strcat(buf,", ");
						if(numFound==numBound-1) Strcat(buf,", and ");
					}
					break;
				}
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_DAHLVER_NAR){
			Strcat(buf, sealNames[(DAHLVER_NAR) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_ACERERAK){
			Strcat(buf, sealNames[(ACERERAK) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_COUNCIL){
			Strcat(buf, sealNames[(COUNCIL) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.spirit[CROWN_SPIRIT]) for(j=0;j<32;j++){
			if((u.spirit[CROWN_SPIRIT] >> j) == 1){
				Strcat(buf,sealNames[j]);
				numFound++;
				if(numBound==2 && numFound==1) Strcat(buf," and ");
				else if(numBound>=3){
					if(numFound<numBound-1) Strcat(buf,", ");
					if(numFound==numBound-1) Strcat(buf,", and ");
				}
				break;
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_COSMOS){
			Strcat(buf, sealNames[(COSMOS) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_MISKA){
			Strcat(buf, sealNames[(MISKA) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_NUDZIARTH){
			Strcat(buf, sealNames[(NUDZIARTH) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_ALIGNMENT_THING){
			Strcat(buf, sealNames[(ALIGNMENT_THING) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_UNKNOWN_GOD){
			Strcat(buf, sealNames[(UNKNOWN_GOD) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_BLACK_WEB){
			Strcat(buf, sealNames[(BLACK_WEB) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_NUMINA){
			Strcat(buf, sealNames[(NUMINA) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		enl_msg(prebuf, "is", "was", buf);
	}
	if(u.sealsKnown || u.specialSealsKnown){
		int numSlots;
		if(Role_if(PM_EXILE)){
			if(u.ulevel <= 2) numSlots=1;
			else if(u.ulevel <= 9) numSlots=2;
			else if(u.ulevel <= 18) numSlots=3;
			else if(u.ulevel <= 25) numSlots=4;
			else numSlots=5;
		} else {
			numSlots=1;
		}
		if(!u.spirit[QUEST_SPIRIT] && u.specialSealsKnown&(SEAL_DAHLVER_NAR|SEAL_ACERERAK|SEAL_BLACK_WEB)){
			you_are("able to bind with a quest spirit");
		}
		if(!u.spirit[ALIGN_SPIRIT] && u.specialSealsKnown&(SEAL_COSMOS|SEAL_MISKA|SEAL_NUDZIARTH|SEAL_ALIGNMENT_THING|SEAL_UNKNOWN_GOD)){
			you_are("able to bind with an aligned spirit");
		}
		if(!u.spirit[OUTER_SPIRIT] && u.ulevel == 30 && Role_if(PM_EXILE)){
			you_are("able to bind with the Numina");
		}
		if(u.sealCounts < numSlots){
			Sprintf(prebuf, "You ");
			Sprintf(buf, " bind to %d more spirit%s", numSlots-u.sealCounts, (numSlots-u.sealCounts)>1 ? "s" : "");
			enl_msg(prebuf, "can", "could", buf);
		}
	}

	/*** Resistances to troubles ***/
	if (Acid_resistance) you_are("acid resistant");
	if (Cold_resistance) you_are("cold resistant");
	if (Disint_resistance) you_are("disintegration-resistant");
	if (Fire_resistance) you_are("fire resistant");
	if (Halluc_resistance)
		you_are("resistant to hallucinations");
	if (Invulnerable) you_are("invulnerable");
	if (Drain_resistance) you_are("level-drain resistant");
	if (Antimagic) you_are("magic-protected");
	if (Stone_resistance)
		you_are("petrification resistant");
	if (Poison_resistance) you_are("poison resistant");
	if (Shock_resistance) you_are("shock resistant");
	if (Sick_resistance) you_are("immune to sickness");
	if (Sleep_resistance) you_are("sleep resistant");
	if (u.uedibility || u.sealsActive&SEAL_BUER) you_can("recognize detrimental food");
	if ( (ublindf && ublindf->otyp == R_LYEHIAN_FACEPLATE && !ublindf->cursed) || 
		 (uarmc && uarmc->otyp == OILSKIN_CLOAK && !uarmc->cursed) ||
		 (u.sealsActive&SEAL_ENKI)
	) you_are("waterproof");

	/*** Troubles ***/
	if (final) {
		if (Hallucination) you_are("hallucinating");
		if (Stunned) you_are("stunned");
		if (Confusion) you_are("confused");
		if (Blinded) you_are("blinded");
		if (Sick) {
			if (u.usick_type & SICK_VOMITABLE)
				you_are("sick from food poisoning");
			if (u.usick_type & SICK_NONVOMITABLE)
				you_are("sick from illness");
		}
#ifdef CONVICT
        if (Punished) {
            you_are("punished");
        }
#endif /* CONVICT */
	}
	if (Stoned) you_are("turning to stone");
	if (Slimed) you_are("turning into slime");
	if (Strangled) you_are((u.uburied) ? "buried" : "being strangled");
	if (Glib) {
		Sprintf(buf, "slippery %s", makeplural(body_part(FINGER)));
		you_have(buf);
	}
	if (Fumbling) enl_msg("You fumble", "", "d", "");
	if (Wounded_legs
#ifdef STEED
	    && !u.usteed
#endif
			  ) {
		Sprintf(buf, "wounded %s", makeplural(body_part(LEG)));
		you_have(buf);
	}
#if defined(WIZARD) && defined(STEED)
	if (Wounded_legs && u.usteed && wizard) {
	    Strcpy(buf, x_monnam(u.usteed, ARTICLE_YOUR, (char *)0, 
		    SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION, FALSE));
	    *buf = highc(*buf);
	    enl_msg(buf, " has", " had", " wounded legs");
	}
#endif
	if (Sleeping) enl_msg("You ", "fall", "fell", " asleep");
	if (Hunger) enl_msg("You hunger", "", "ed", " rapidly");
	if(u.wimage >= 10){
		if(ACURR(A_WIS) < 6){
			Sprintf(buf, " filled with the image of a weeping angel");
			enl_msg("Your mind ", "is","was",buf);
		} else if(ACURR(A_WIS) < 9){
			Sprintf(buf, " ever on your mind");
			enl_msg("The image of a weeping angel ", "is","was",buf);
		} else if(ACURR(A_WIS) < 12){
			Sprintf(buf, " seem to shake the image of a weeping angel from your mind");
			enl_msg("You ", "can't","couldn't",buf);
		} else {
			Sprintf(buf, " in the back of your mind");
			enl_msg("The image of a weeping angel ", "lurks","lurked",buf);
		}
	}

	/*** Vision and senses ***/
	if (See_invisible) enl_msg(You_, "see", "saw", " invisible");
	if (Blind_telepat) you_are("telepathic");
	if (Warning) you_are("warned");
	if (Warn_of_mon && (flags.warntypem||flags.warntypet||flags.warntypeb||flags.warntypeg||flags.warntypea||flags.warntypev||flags.montype)) {
		if(flags.warntypea & MA_ANIMAL) you_are("aware of the presence of animals");
		if(flags.warntypea & MA_ARACHNID) you_are("aware of the presence of arachnids");
		if(flags.warntypea & MA_AVIAN) you_are("aware of the presence of birds");
		if(flags.warntypea & MA_DEMIHUMAN) you_are("aware of the presence of demihumans");
		if(flags.warntypea & MA_DEMON) you_are("aware of the presence of demons");
		if(flags.warntypea & MA_MINION) you_are("aware of the presence of divine beings");
		if(flags.warntypea & MA_DRAGON) you_are("aware of the presence of dragons");
		if(flags.warntypea & MA_DWARF) you_are("aware of the presence of dwarves");
		if(flags.warntypea & MA_ELEMENTAL) you_are("aware of the presence of elemental beings");
		if(flags.warntypea & MA_ELF) you_are("aware of the presence of elves");
		if(flags.warntypea & MA_FEY) you_are("aware of the presence of fair folk");
		if(flags.warntypea & MA_GIANT) you_are("aware of the presence of giants");
		if(flags.warntypea & MA_HUMAN) you_are("aware of the presence of humans");
		if(flags.warntypea & MA_INSECTOID) you_are("aware of the presence of insectoids");
		if(flags.warntypet & MT_MAGIC) you_are("aware of the presence of magic seekers");
		if(flags.warntypeg & MG_LORD) you_are("aware of the presence of nobles");
		if(flags.warntypea & MA_PRIMORDIAL) you_are("aware of the presence of old ones");
		if(flags.warntypea & MA_ORC) you_are("aware of the presence of orcs");
		if(flags.warntypea & MA_PLANT) you_are("aware of the presence of plants");
		if(flags.warntypea & MA_REPTILIAN) you_are("aware of the presence of reptiles");
		if(flags.warntypeg & MG_PRINCE) you_are("aware of the presence of rulers");
		if(flags.warntypea & MV_TELEPATHIC) you_are("aware of the presence of telepaths");
		if(flags.warntypea & MA_WERE) you_are("aware of the presence of werecreatures");
	}
	if (Undead_warning) you_are("warned of undead");
	if (Searching) you_have("automatic searching");
	if (Clairvoyant) you_are("clairvoyant");
	if (Infravision) you_have("infravision");
	if (Detect_monsters) you_are("sensing the presence of monsters");
	if (u.umconf) you_are("going to confuse monsters");

	/*** Appearance and behavior ***/
#ifdef WIZARD
	Sprintf(buf, "a carrying capacity of %d remaining", -1*inv_weight());
    you_have(buf);
	Sprintf(buf, "%d points of nutrition remaining", YouHunger);
    you_have(buf);
#endif
	if (Adornment) {
	    int adorn = 0;

	    if(uleft && uleft->otyp == RIN_ADORNMENT) adorn += uleft->spe;
	    if(uright && uright->otyp == RIN_ADORNMENT) adorn += uright->spe;
	    if (adorn < 0)
		you_are("poorly adorned");
	    else
		you_are("adorned");
	}
	if (Invisible) you_are("invisible");
	else if (Invis) you_are("invisible to others");
	/* ordinarily "visible" is redundant; this is a special case for
	   the situation when invisibility would be an expected attribute */
	else if ((HInvis || EInvis || pm_invisible(youmonst.data)) && BInvis)
	    you_are("visible");
	if (Displaced) you_are("displaced");
	if (Stealth) you_are("stealthy");
	if (Aggravate_monster) enl_msg("You aggravate", "", "d", " monsters");
	if (Conflict) enl_msg("You cause", "", "d", " conflict");

	/*** Transportation ***/
	if (Jumping) you_can("jump");
	if (Teleportation) you_can("teleport");
	if (Teleport_control) you_have("teleport control");
	if (Lev_at_will) you_are("levitating, at will");
	else if (Levitation) you_are("levitating");	/* without control */
	else if (Flying) you_can("fly");
	if (Wwalking) you_can("walk on water");
	if (Swimming) you_can("swim");
	if (Breathless) you_can("survive without air");
	else if (Amphibious) you_can("breathe water");
	if (Passes_walls) you_can("walk through walls");
#ifdef STEED
	/* If you die while dismounting, u.usteed is still set.  Since several
	 * places in the done() sequence depend on u.usteed, just detect this
	 * special case. */
	if (u.usteed && (final < 2 || strcmp(killer, "riding accident"))) {
	    Sprintf(buf, "riding %s", y_monnam(u.usteed));
	    you_are(buf);
	}
#endif
	if (u.uswallow) {
	    Sprintf(buf, "swallowed by %s", a_monnam(u.ustuck));
#ifdef WIZARD
	    if (wizard) Sprintf(eos(buf), " (%u)", u.uswldtim);
#endif
	    you_are(buf);
	} else if (u.ustuck) {
	    Sprintf(buf, "%s %s",
		    (Upolyd && sticks(youmonst.data)) ? "holding" : "held by",
		    a_monnam(u.ustuck));
	    you_are(buf);
	}

	/*** Physical attributes ***/
	if (uclockwork){
		if(u.ucspeed==HIGH_CLOCKSPEED) you_are("set to high clockspeed");
		if(u.ucspeed==NORM_CLOCKSPEED) you_are("set to normal clockspeed");
		if(u.ucspeed==SLOW_CLOCKSPEED) you_are("set to low clockspeed");
		if(u.phasengn) you_are("in phase mode");
	}
	if (u.uhitinc)
	    you_have(enlght_combatinc("to hit", u.uhitinc, final, buf));
	if (u.udaminc)
	    you_have(enlght_combatinc("damage", u.udaminc, final, buf));
	if (Slow_digestion) you_have("slower digestion");
	if (Regeneration) enl_msg("You regenerate", "", "d", "");
	if (u.uspellprot || Protection) {
	    int prot = 0;

	    if(uleft && uleft->otyp == RIN_PROTECTION) prot += uleft->spe;
	    if(uright && uright->otyp == RIN_PROTECTION) prot += uright->spe;
	    if (HProtection & INTRINSIC) prot += u.ublessed;
	    prot += u.uspellprot;

	    if (prot < 0)
		you_are("ineffectively protected");
	    else
		you_are("protected");
	}
	if (Protection_from_shape_changers)
		you_are("protected from shape changers");
	if (Polymorph) you_are("polymorphing");
	if (Polymorph_control) you_have("polymorph control");
	if (u.ulycn >= LOW_PM && !uclockwork) {
		Strcpy(buf, an(mons[u.ulycn].mname));
		you_are(buf);
	}
	if (Upolyd) {
	    if (u.umonnum == u.ulycn) Strcpy(buf, "in beast form");
	    else Sprintf(buf, "polymorphed into %s", an(youmonst.data->mname));
#ifdef WIZARD
	    if (wizard) Sprintf(eos(buf), " (%d)", u.mtimedone);
#endif
	    you_are(buf);
	}
	if (Unchanging) you_can("not change from your current form");
	if (Fast) you_are(Very_fast ? "very fast" : "fast");
	if (Reflecting) you_have("reflection");
	if (Reflecting && (
			(uwep && is_lightsaber(uwep) && uwep->lamplit && 
				((u.fightingForm == FFORM_SHIEN && (!uarm || is_light_armor(uarm))) || 
				 (u.fightingForm == FFORM_SORESU && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm)))
				)
			) ||
			(uarm && (uarm->otyp == SILVER_DRAGON_SCALE_MAIL || uarm->otyp == SILVER_DRAGON_SCALES || uarm->otyp == SILVER_DRAGON_SCALE_SHIELD)) ||
			(uwep && uwep->oartifact == ART_DRAGONLANCE)
	)) you_have("dragon-breath reflection");
	if (Free_action) you_have("free action");
	if (Fixed_abil) you_have("fixed abilities");
	if (Lifesaved)
		enl_msg("Your life ", "will be", "would have been", " saved");
	if (u.twoweap) you_are("wielding two weapons at once");

	/*** Miscellany ***/
	if (Spellboost) you_have("augmented spellcasting");
	if (Luck) {
	    ltmp = abs((int)Luck);
	    Sprintf(buf, "%s%slucky",
		    ltmp >= 10 ? "extremely " : ltmp >= 5 ? "very " : "",
		    Luck < 0 ? "un" : "");
#ifdef WIZARD
	    if (wizard) Sprintf(eos(buf), " (%d)", Luck);
#endif
	    you_are(buf);
	}
#ifdef WIZARD
	 else if (wizard) enl_msg("Your luck ", "is", "was", " zero");
#endif
	if (u.moreluck > 0) you_have("extra luck");
	else if (u.moreluck < 0) you_have("reduced luck");
	if (has_luckitem()) {
	    ltmp = stone_luck(FALSE);
	    if (ltmp <= 0)
		enl_msg("Bad luck ", "times", "timed", " out slowly for you");
	    if (ltmp >= 0)
		enl_msg("Good luck ", "times", "timed", " out slowly for you");
	}

	if (u.ugangr[Align2gangr(u.ualign.type)]) {
	    Sprintf(buf, " %sangry with you",
		    u.ugangr[Align2gangr(u.ualign.type)] > 6 ? "extremely " : u.ugangr[Align2gangr(u.ualign.type)] > 3 ? "very " : "");
#ifdef WIZARD
	    if (wizard) Sprintf(eos(buf), " (%d)", u.ugangr[Align2gangr(u.ualign.type)]);
#endif
	    enl_msg(u_gname(), " is", " was", buf);
	} else
	    /*
	     * We need to suppress this when the game is over, because death
	     * can change the value calculated by can_pray(), potentially
	     * resulting in a false claim that you could have prayed safely.
	     */
	  if (!final) {
#if 0
	    /* "can [not] safely pray" vs "could [not] have safely prayed" */
	    Sprintf(buf, "%s%ssafely pray%s", can_pray(FALSE) ? "" : "not ",
		    final ? "have " : "", final ? "ed" : "");
#else
	    Sprintf(buf, "%ssafely pray", can_pray(FALSE) ? "" : "not ");
#endif
#ifdef WIZARD
	    if (wizard) Sprintf(eos(buf), " (%d)", u.ublesscnt);
#endif
	    you_can(buf);
	}

    {
	const char *p;

	buf[0] = '\0';
	if (final < 2) {    /* still in progress, or quit/escaped/ascended */
	    p = "survived after being killed ";
	    switch (u.umortality) {
	    case 0:  p = !final ? (char *)0 : "survived";  break;
	    case 1:  Strcpy(buf, "once");  break;
	    case 2:  Strcpy(buf, "twice");  break;
	    case 3:  Strcpy(buf, "thrice");  break;
	    default: Sprintf(buf, "%d times", u.umortality);
		     break;
	    }
	} else {		/* game ended in character's death */
	    p = "are dead";
	    switch (u.umortality) {
	    case 0:  impossible("dead without dying?");
	    case 1:  break;			/* just "are dead" */
	    default: Sprintf(buf, " (%d%s time!)", u.umortality,
			     ordin(u.umortality));
		     break;
	    }
	}
	if (p) enl_msg(You_, "have been killed ", p, buf);
    }

	display_nhwindow(en_win, TRUE);
	destroy_nhwindow(en_win);
	return;
}

void
resistances_enlightenment()
{
	char buf[BUFSZ];

	en_win = create_nhwindow(NHW_MENU);
	putstr(en_win, 0, "Current Status:");
	putstr(en_win, 0, "");

	if (uclockwork){
		if(u.ucspeed==HIGH_CLOCKSPEED) putstr(en_win, 0, "Your clock is set to high speed.");
		if(u.ucspeed==NORM_CLOCKSPEED) putstr(en_win, 0, "Your clock is set to normal speed.");
		if(u.ucspeed==SLOW_CLOCKSPEED) putstr(en_win, 0, "Your clock is set to low speed.");
		if(u.phasengn) putstr(en_win, 0, "Your phase engine is activated.");
	}
	/*** Resistances to troubles ***/
	/* It is important to inform the player as to the status of any resistances that can expire */
	if (Fire_resistance && Cold_resistance) putstr(en_win, 0, "You feel comfortable.");
	else{
		if (Fire_resistance) putstr(en_win, 0, "You feel chilly.");
		if (Cold_resistance) putstr(en_win, 0, "You feel warm inside.");
	}
	if (Sleep_resistance) putstr(en_win, 0, "You feel wide awake.");
	if (Disint_resistance) putstr(en_win, 0, "You feel very firm.");
	if (Shock_resistance) putstr(en_win, 0, "You feel well grounded.");
	if (Poison_resistance) putstr(en_win, 0, "You feel healthy.");
	if (Acid_resistance) putstr(en_win, 0, "Your skin feels leathery.");
	if (Displaced) putstr(en_win, 0, "Your outline shimmers and shifts.");
	if (Drain_resistance) putstr(en_win, 0, "You feel especially energetic.");
	if (u.uinwater){
		if(ublindf && ublindf->otyp == R_LYEHIAN_FACEPLATE && !ublindf->cursed)
			putstr(en_win, 0, "Your faceplate wraps you in a waterproof field.");
		else if(uarmc && (uarmc->otyp == OILSKIN_CLOAK || uarmc->greased) && !uarmc->cursed)
			putstr(en_win, 0, "Your waterproof cloak protects your gear.");
		else if(u.sealsActive&SEAL_ENKI)
			putstr(en_win, 0, "YOU'RE soaked, but the water doesn't wet your gear.");
	}
/*
	if (Sick_resistance) you_are("immune to sickness");
	if (Antimagic) you_are("magic-protected");
	if (Stone_resistance)
		you_are("petrification resistant");
	if (Invulnerable) you_are("invulnerable");
	if (u.uedibility) you_can("recognize detrimental food");
	if (Halluc_resistance)
		enl_msg("You resist", "", "ed", " hallucinations");
*/
	/*** Troubles ***/
	if (Wounded_legs
#ifdef STEED
	    && !u.usteed
#endif
			  ) {
		Sprintf(buf, "You have wounded %s", makeplural(body_part(LEG)));
		putstr(en_win, 0, buf);
	}
	
	if(u.wimage >= 10){
		if(ACURR(A_WIS) < 6) Sprintf(buf, "Your mind is filled with the image of a weeping angel");
		else if(ACURR(A_WIS) < 9) Sprintf(buf, "The image of a weeping angel is ever on your mind");
		else if(ACURR(A_WIS) < 12) Sprintf(buf, "You can't seem to shake the image of a weeping angel from your mind");
		else Sprintf(buf, "The image of a weeping angel lurks in the back of your mind");
		putstr(en_win, 0, buf);
	}
	
	if(u.lastprayed){
		Sprintf(buf, "You last %s %ld turns ago", u.lastprayresult==PRAY_GIFT ? "recieved a gift" :
												  u.lastprayresult==PRAY_ANGER ? "angered your god" : 
												  u.lastprayresult==PRAY_CONV ? "converted to a new god" : 
												  "prayed",
			moves - u.lastprayed);
		putstr(en_win, 0, buf);
		if(u.lastprayresult==PRAY_GOOD){
			Sprintf(buf, "That prayer was well recieved");
			putstr(en_win, 0, buf);
		} else if(u.lastprayresult==PRAY_BAD){
			Sprintf(buf, "That prayer was poorly recieved");
			putstr(en_win, 0, buf);
		}
		if(u.reconciled){
			if(u.reconciled==REC_REC) Sprintf(buf, "You have since reconciled with your god");
			else if(u.reconciled==REC_MOL) Sprintf(buf, "You have since mollified your god");
			putstr(en_win, 0, buf);
		}
	}
	
	if(u.sealsActive || u.specialSealsActive){
		int i,j,numBound,numFound=0;
		numBound = u.sealCounts;
		if(u.spirit[QUEST_SPIRIT]) numBound++;
		if(Role_if(PM_EXILE) && u.uevent.uhand_of_elbereth) numBound++;
		if(u.spirit[CROWN_SPIRIT]) numBound++;
		if(u.spirit[GPREM_SPIRIT]) numBound++;
		if(u.spirit[ALIGN_SPIRIT]) numBound++;
		if(u.spirit[OUTER_SPIRIT]) numBound++;
		Sprintf(buf, "Your soul is bound to ");
		for(i=0;i<QUEST_SPIRIT;i++){
			if(u.spirit[i]) for(j=0;j<32;j++){
				if((u.spirit[i] >> j) == 1){
					Strcat(buf,sealNames[j]);
					numFound++;
					if(numBound==2 && numFound==1) Strcat(buf," and ");
					else if(numBound>=3){
						if(numFound<numBound-1) Strcat(buf,", ");
						if(numFound==numBound-1) Strcat(buf,", and ");
					}
					break;
				}
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_DAHLVER_NAR){
			Strcat(buf, sealNames[(DAHLVER_NAR) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_ACERERAK){
			Strcat(buf, sealNames[(ACERERAK) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_COUNCIL){
			Strcat(buf, sealNames[(COUNCIL) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.spirit[CROWN_SPIRIT]) for(j=0;j<32;j++){
			if((u.spirit[CROWN_SPIRIT] >> j) == 1){
				Strcat(buf,sealNames[j]);
				numFound++;
				if(numBound==2 && numFound==1) Strcat(buf," and ");
				else if(numBound>=3){
					if(numFound<numBound-1) Strcat(buf,", ");
					if(numFound==numBound-1) Strcat(buf,", and ");
				}
				break;
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_COSMOS){
			Strcat(buf, sealNames[(COSMOS) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_MISKA){
			Strcat(buf, sealNames[(MISKA) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_NUDZIARTH){
			Strcat(buf, sealNames[(NUDZIARTH) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_ALIGNMENT_THING){
			Strcat(buf, sealNames[(ALIGNMENT_THING) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_UNKNOWN_GOD){
			Strcat(buf, sealNames[(UNKNOWN_GOD) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_BLACK_WEB){
			Strcat(buf, sealNames[(BLACK_WEB) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		if(numFound < numBound && u.specialSealsActive&SEAL_NUMINA){
			Strcat(buf, sealNames[(NUMINA) - (FIRST_SEAL)]);
			numFound++;
			if(numBound==2 && numFound==1) Strcat(buf," and ");
			else if(numBound>=3){
				if(numFound<numBound-1) Strcat(buf,", ");
				if(numFound==numBound-1) Strcat(buf,", and ");
			}
		}
		Strcat(buf,".");
		putstr(en_win, 0, buf);
	}
	display_nhwindow(en_win, TRUE);
	destroy_nhwindow(en_win);
	return;
}

void
signs_enlightenment()
{
	boolean message = FALSE;

	en_win = create_nhwindow(NHW_MENU);
	putstr(en_win, 0, "Current Appearance:");
	putstr(en_win, 0, "");

	if(Invis){
		putstr(en_win, 0, "You are invisible.");
		message = TRUE;
	}
	
	// if(u.sealsActive&SEAL_AHAZU && !(ublindf && ublindf->otyp==MASK));
	if(u.sealsActive&SEAL_AMON && !Invis){
//		if(!(uarmh && is_metallic(uarmh))){
		putstr(en_win, 0, "You have a pair of large ram's horns.");
//		} else putstr(en_win, 0, "Your ram's horns have fused with your helm, taking on a metalic hue.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_ANDREALPHUS && !Invis && !(levl[u.ux][u.uy].lit == 0 && !(viz_array[u.uy][u.ux]&TEMP_LIT))){
		putstr(en_win, 0, "Up close, it is plain your shadow aspires to depth as well as width and height.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_ANDROMALIUS && !NoBInvis){
//		if((levl[u.ux][u.uy].lit == 0 && !(viz_array[u.uy][u.ux]&TEMP_LIT)))
			putstr(en_win, 0, "Your features have taken on the rigidity of a cheap disguise.");
		// else putstr(en_win, 0, "Your rigid features can't be seen in the dark.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_ASTAROTH && !Invis){
		if(!ublindf || ublindf->otyp == LENSES)
			putstr(en_win, 0, "A black liquid leaks from around your eyes.");
		else if(ublindf && (ublindf->otyp == MASK || ublindf->otyp == R_LYEHIAN_FACEPLATE))
			putstr(en_win, 0, "The black liquid leaking from your eyes is hidden by your mask.");
		else
			putstr(en_win, 0, "The black liquid leaking from your eyes is soaked up by your blindfold.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_BALAM && !Invis){
		if(uarmc || uarm)
			putstr(en_win, 0, "Freezing water leaks from a gash in you neck, but is hidden by your clothes.");
		else
			putstr(en_win, 0, "Freezing water leaks from a deep gash in you neck.");
		if(!uarmg)
			putstr(en_win, 0, "Freezing water leaks from deep holes in your wrists.");
		if(!uarmf)
			putstr(en_win, 0, "Freezing water leaks from deep holes in your ankles.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_BERITH && !Invis){
		if(u.usteed)
			putstr(en_win, 0, "Your steed is drenched in gore.");
		if(!(uarm && is_metallic(uarm) && uarmg && uarmf && uarmh))
			putstr(en_win, 0, "You are drenched in gore.");
		else
			putstr(en_win, 0, "Your armor is faced with crimson enamel.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_BUER && !Invis){
		if(!uarmf)
			putstr(en_win, 0, "Your legs bifurcate into twin pairs of cloven-hoved feet.");
		else
			putstr(en_win, 0, "Your strange feet are hidden in your shoes.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_CHUPOCLOPS){
		putstr(en_win, 0, "You feel something in your cheeks.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_DANTALION && !NoBInvis){
		if(!(uarmc || ((uarm && uarm->otyp != CRYSTAL_PLATE_MAIL) || uarmu))){
			switch(u.ulevel/10+1){
				case 1:
				putstr(en_win, 0, "There is an extra face growing on your chest.");
				break;
				case 2:
				putstr(en_win, 0, "There is a pair of faces growing on your chest.");
				break;
				case 3:
				putstr(en_win, 0, "Many extra faces grow on your chest.");
				break;
				case 4:
				putstr(en_win, 0, "There are legions of faces growing on your chest.");
				break;
			}
		}
		else{
			if(u.ulevel/10){
				putstr(en_win, 0, "Your extra faces are covered by your clothes.");
			} else
				putstr(en_win, 0, "Your extra face is covered by your clothes.");
		}
		message = TRUE;
	}
	// if(u.sealsActive&SEAL_SHIRO);
	if(u.sealsActive&SEAL_ECHIDNA && !Invis){
		if(!(uarmf && (uarmc || uarm)))
			putstr(en_win, 0, "Your hips give rise to twin serpent's tails instead of legs.");
		else
			putstr(en_win, 0, "Your serpentine legs are disguised by your clothes.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_EDEN){
		putstr(en_win, 0, "There is something rigid in the crown of your skull.");
		message = TRUE;
	} 
	if(u.sealsActive&SEAL_ENKI && !Invis){
		if(!(uarm || uarmc))
			putstr(en_win, 0, "Water runs off your body in steady rivulets.");
		else
			putstr(en_win, 0, "Your body's runoff is caught by your clothes.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_EURYNOME && !Invis){
		if(levl[u.ux][u.uy].lit != 0){
			putstr(en_win, 0, "Your shadow is that of a dancing nymph.");
			message = TRUE;
		} else if(viz_array[u.uy][u.ux]&TEMP_LIT){
			putstr(en_win, 0, "It's a bit hard to see, but your shadow is a dancing nymph.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_EVE && !NoBInvis){
		if(!uarm && !uarmc){
			putstr(en_win, 0, "There is a blood-caked wound on your stomach.");
			message = TRUE;
		}
		if(!uarmf){
			putstr(en_win, 0, "Your feet are torn by thorns and stones.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_FAFNIR && !NoBInvis){ 
		if(!(uright || uarmg)){
			putstr(en_win, 0, "There is a ring-shaped burn scar around your right ring-finger.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_HUGINN_MUNINN){
		putstr(en_win, 0, "There is something rustling around in your ear.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_IRIS && !NoBInvis){
		if(moves <= u.irisAttack+1){
			putstr(en_win, 0, "Waving, iridescent tentacles sprout from your forearms.");
			message = TRUE;
		} else if(!uarmc && moves <= u.irisAttack+5){
			putstr(en_win, 0, "There are iridescent tentacles wrapped around your forearms.");
			message = TRUE;
		} else if(!uarm){
			putstr(en_win, 0, "There are iridescent veins just under the skin of your forearms.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_JACK){
		putstr(en_win, 0, "There is something on your back.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_MALPHAS){
		putstr(en_win, 0, "You feel things pecking the inside of your mouth.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_MARIONETTE && !NoBInvis){
		if(!(uarm && is_metallic(uarm)))
			putstr(en_win, 0, "Metal wires protrude from your elbows, knees, and back.");
		else
			putstr(en_win, 0, "The metal wires protruding from your body have merged with your armor.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_MOTHER && !NoBInvis){
		if(!uarmg)
			putstr(en_win, 0, "The eyes on your fingers and palms stare back at you.");
		else
			putstr(en_win, 0, "The eyes on your fingers and palms are covered up.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_NABERIUS){
		putstr(en_win, 0, "Your tongue feels odd.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_ORTHOS && !NoBInvis){
		if(uarmc && uarmc->otyp != MUMMY_WRAPPING){
			putstr(en_win, 0, "Your cloak blows in a nonexistant wind.");
			message = TRUE;
		}
	}
	// // if(u.sealsActive&SEAL_OSE && !BClairvoyant && !(uarmh && is_metallic(uarmh) && uarmh->otyp != HELM_OF_TELEPATHY)) count++;
	if(u.sealsActive&SEAL_OTIAX && !Invis){
		if(moves <= u.otiaxAttack+5){
			putstr(en_win, 0, "The mist around you is formed into writhing tendrils.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_PAIMON && !Invis){
		putstr(en_win, 0, "There is a crown floating over your head.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_SIMURGH && !Invis){
		if(!uarmg)
			putstr(en_win, 0, "You have iron claws.");
		else
			putstr(en_win, 0, "Your iron claws seem to be part of your gloves.");
		putstr(en_win, 0, "There are prismatic feathers around your head.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_TENEBROUS && !Invis){
		if(!(levl[u.ux][u.uy].lit == 0 && !(viz_array[u.uy][u.ux]&TEMP_LIT))){
			putstr(en_win, 0, "Your shadow is deep black and pools unnaturally close to you.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_YMIR && !Invis){
		if(moves>5000 && moves <= 10000){
			if(!(uarm || uarmc))
				putstr(en_win, 0, "Your skin color is a bit off.");
			else
				putstr(en_win, 0, "Your skin is hidden under your clothes.");
			message = TRUE;
		} else if(moves>10000 && moves <= 20000){
			if(!(uarmc))
				putstr(en_win, 0, "Maggots burrow through your skin.");
			else
				putstr(en_win, 0, "Your rotting is hidden under your clothes.");
			message = TRUE;
		} else if(moves>20000 && moves <= 50000){
			if(!(uarmc && uarmg && uarmf))
				putstr(en_win, 0, "Your skin is rotting off.");
			else
				putstr(en_win, 0, "Your rotting is hidden under your clothes.");
			message = TRUE;
		} else if(moves>50000 && moves <= 100000){
			if(!(uarmc && uarmg && uarmf && uarm && uarmh))
				putstr(en_win, 0, "Your rotted body bristles with fungal sporangia and burrowing vermin.");
			else
				putstr(en_win, 0, "Your rotted form is hidden under your clothes.");
			message = TRUE;
		} else if(moves>100000){
			if(!(uarmc && uarmg && uarmf && uarm && uarmh && ublindf && ublindf->otyp==MASK))
				putstr(en_win, 0, "Your putrid body is a riot of fungal forms and saprophagous insects.");
			else
				putstr(en_win, 0, "Your putrid form is hidden under your clothes.");
			message = TRUE;
		}
	}
	if(u.specialSealsActive&SEAL_DAHLVER_NAR && !NoBInvis){
		if(dahlverNarVis())
			putstr(en_win, 0, "Your wounds are full of sharp teeth!");
		else
			putstr(en_win, 0, "You feel teeth beneath your skin.");
		message = TRUE;
	}
	if(u.specialSealsActive&SEAL_ACERERAK){
		putstr(en_win, 0, "Your eyes feel odd.");
		message = TRUE;
	}
	if(u.specialSealsActive&SEAL_BLACK_WEB){
		putstr(en_win, 0, "Your shadow is wrapped in the black web.");
		message = TRUE;
	}
	if(u.specialSealsActive&SEAL_COSMOS){
		putstr(en_win, 0, "You feel like something is behind you, but you can't see anything.");
		message = TRUE;
	}
	if(u.specialSealsActive&SEAL_MISKA){
		putstr(en_win, 0, "You have 4 arms, and a wolf head grows from each hip.");
		message = TRUE;
	}
	// if(u.specialSealsActive&SEAL_NUDZIARTH){
		// putstr(en_win, 0, "Your eyes feel odd.");
		// message = TRUE;
	// }
	if(u.specialSealsActive&SEAL_ALIGNMENT_THING){
		putstr(en_win, 0, "You feel like someone is staring at the back of your head.");
		putstr(en_win, 0, "You hear an argument raging in the distance.");
		message = TRUE;
	}
	if(u.specialSealsActive&SEAL_NUMINA){
		putstr(en_win, 0, "You are surounded by whispers.");
		message = TRUE;
	}
	
	if(!message){
		putstr(en_win, 0, "You think you look pretty normal.");
	}
	
	display_nhwindow(en_win, TRUE);
	destroy_nhwindow(en_win);
	return;
}

void
signs_mirror()
{
	boolean message = FALSE;

	en_win = create_nhwindow(NHW_MENU);
	putstr(en_win, 0, "Current Appearance:");
	putstr(en_win, 0, "");

	if(Invis){
		putstr(en_win, 0, "You are invisible.");
		message = TRUE;
	}
	
	if(u.sealsActive&SEAL_AHAZU && !NoBInvis){
		if(!(ublindf && ublindf->otyp==MASK)){
			putstr(en_win, 0, "There is a starry void in your throat.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_AMON && !Invis){
		if(!(uarmh && is_metallic(uarmh))){
			putstr(en_win, 0, "You have a pair of large ram's horns.");
		} else putstr(en_win, 0, "Your ram's horns have fused with your helm, taking on a metalic hue.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_ANDROMALIUS && !NoBInvis){
		if((levl[u.ux][u.uy].lit == 0 && !(viz_array[u.uy][u.ux]&TEMP_LIT)))
			putstr(en_win, 0, "Your features have taken on the rigidity of a cheap disguise.");
		else putstr(en_win, 0, "Your rigid features can't be seen in the dark.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_ASTAROTH && !Invis){
		if(!ublindf || ublindf->otyp == LENSES)
			putstr(en_win, 0, "A black liquid leaks from around your eyes.");
		else if(ublindf && (ublindf->otyp == MASK || ublindf->otyp == R_LYEHIAN_FACEPLATE))
			putstr(en_win, 0, "The black liquid leaking from your eyes is hidden by your mask.");
		else
			putstr(en_win, 0, "The black liquid leaking from your eyes is soaked up by your blindfold.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_BALAM && !Invis){
		if(uarmc || uarm)
			putstr(en_win, 0, "Freezing water leaks from a gash in you neck, but is hidden by your clothes.");
		else
			putstr(en_win, 0, "Freezing water leaks from a deep gash in you neck.");
		if(!uarmg)
			putstr(en_win, 0, "Freezing water leaks from deep holes in your wrists.");
		if(!uarmf)
			putstr(en_win, 0, "Freezing water leaks from deep holes in your ankles.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_BERITH && !Invis){
		if(!(uarm && is_metallic(uarm) && uarmg && uarmf && uarmh))
			putstr(en_win, 0, "You are drenched in gore.");
		else
			putstr(en_win, 0, "Your armor is faced with crimson enamel.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_BUER && !Invis){
		if(!uarmf)
			putstr(en_win, 0, "Your legs bifurcate into twin pairs of cloven-hoved feet.");
		else
			putstr(en_win, 0, "Your strange feet are hidden in your shoes.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_CHUPOCLOPS && !NoBInvis){
		if(!(ublindf && ublindf->otyp==MASK)){
			putstr(en_win, 0, "You see a pair of chelicerae in your mouth!");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_DANTALION && !NoBInvis){
		if(!(uarmc || ((uarm && uarm->otyp != CRYSTAL_PLATE_MAIL) || uarmu))){
			switch(u.ulevel/10+1){
				case 1:
				putstr(en_win, 0, "There is an extra face growing on your chest.");
				break;
				case 2:
				putstr(en_win, 0, "There is a pair of faces growing on your chest.");
				break;
				case 3:
				putstr(en_win, 0, "Many extra faces grow on your chest.");
				break;
				case 4:
				putstr(en_win, 0, "There are legions of faces growing on your chest.");
				break;
			}
		}
		else{
			if(u.ulevel/10){
				putstr(en_win, 0, "Your extra faces are covered by your clothes.");
			} else
				putstr(en_win, 0, "Your extra face is covered by your clothes.");
		}
		message = TRUE;
	}
	// if(u.sealsActive&SEAL_SHIRO);
	if(u.sealsActive&SEAL_ECHIDNA && !Invis){
		if(!(uarmf && (uarmc || uarm)))
			putstr(en_win, 0, "Your hips give rise to twin serpent's tails instead of legs.");
		else
			putstr(en_win, 0, "Your serpentine legs are disguised by your clothes.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_EDEN && !NoBInvis){
		putstr(en_win, 0, "You see a garden through the dome of cerulean crystal embedded in your head!");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_ENKI && !Invis){
		if(!(uarm || uarmc))
			putstr(en_win, 0, "Water runs off your body in steady rivulets.");
		else
			putstr(en_win, 0, "Your body's runoff is caught by your clothes.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_EVE && !NoBInvis){
		if(!uarm && !uarmc){
			putstr(en_win, 0, "There is a blood-caked wound on your stomach.");
			message = TRUE;
		}
		if(!uarmf){
			putstr(en_win, 0, "Your feet are torn by thorns and stones.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_FAFNIR && !NoBInvis){ 
		if(!(uright || uarmg)){
			putstr(en_win, 0, "There is a ring-shaped burn scar around your right ring-finger.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_HUGINN_MUNINN && !NoBInvis){
		if(!(ublindf && ublindf->otyp==MASK)){
			putstr(en_win, 0, "You find a raven nesting in each ear!");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_IRIS && !NoBInvis){
		if(moves <= u.irisAttack+1){
			putstr(en_win, 0, "Waving, iridescent tentacles sprout from your forearms.");
			message = TRUE;
		} else if(!uarmc && moves <= u.irisAttack+5){
			putstr(en_win, 0, "There are iridescent tentacles wrapped around your forearms.");
			message = TRUE;
		} else if(!uarm){
			putstr(en_win, 0, "There are iridescent veins just under the skin of your forearms.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_JACK && !NoBInvis){
		if(!uarmc){
			putstr(en_win, 0, "You see an old, old man on your back!");
		} else {
			putstr(en_win, 0, "You see a bump under your cloak on your back.");
		}
		message = TRUE;
	}
	if(u.sealsActive&SEAL_MALPHAS && !NoBInvis){
		if(!(ublindf && ublindf->otyp==MASK)){
			putstr(en_win, 0, "There is a whole flock's worth of crows peeking out of your throat!");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_MARIONETTE && !NoBInvis){
		if(!(uarm && is_metallic(uarm)))
			putstr(en_win, 0, "Metal wires protrude from your elbows, knees, and back.");
		else
			putstr(en_win, 0, "The metal wires protruding from your body have merged with your armor.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_MOTHER && !NoBInvis){
		if(!uarmg)
			putstr(en_win, 0, "The eyes on your fingers and palms stare back at you.");
		else
			putstr(en_win, 0, "The eyes on your fingers and palms are covered up.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_NABERIUS && !NoBInvis){
		if(!(ublindf && ublindf->otyp==MASK)){
			putstr(en_win, 0, "Your tongue is forked!");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_ORTHOS && !NoBInvis){
		if(uarmc && uarmc->otyp != MUMMY_WRAPPING){
			putstr(en_win, 0, "Your cloak blows in a nonexistant wind.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_OSE && !Blind && !BClairvoyant && !(uarmh && is_metallic(uarmh) && uarmh->otyp != HELM_OF_TELEPATHY)){
			putstr(en_win, 0, "You feel your gaze as a tangible force.");
			message = TRUE;
	}
	if(u.sealsActive&SEAL_OTIAX && !Invis){
		if(moves <= u.otiaxAttack+5){
			putstr(en_win, 0, "The mist around you is formed into writhing tendrils.");
			message = TRUE;
		}
	}
	if(u.sealsActive&SEAL_PAIMON && !Invis){ 
		if(!uarmh)
			putstr(en_win, 0, "There is a crown floating over your head.");
		else
			putstr(en_win, 0, "There is a crown sitting on your helm.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_SIMURGH && !Invis){
		if(!uarmg)
			putstr(en_win, 0, "You have iron claws.");
		else
			putstr(en_win, 0, "Your iron claws seem to be part of you gloves.");
		if(!uarmh)
			putstr(en_win, 0, "There is a pair of prismatic wings reaching around your head.");
		else
			putstr(en_win, 0, "Your helm has a crest of prismatic feathers.");
		message = TRUE;
	}
	if(u.sealsActive&SEAL_YMIR && !Invis){
		if(moves>5000 && moves <= 10000){
			if(!(uarm || uarmc))
				putstr(en_win, 0, "Your skin color is a bit off.");
			else
				putstr(en_win, 0, "Your skin is hidden under your clothes.");
			message = TRUE;
		} else if(moves>10000 && moves <= 20000){
			if(!(uarmc))
				putstr(en_win, 0, "Maggots burrow through your skin.");
			else
				putstr(en_win, 0, "Your rot is hidden under your clothes.");
			message = TRUE;
		} else if(moves>20000 && moves <= 50000){
			if(!(uarmc && uarmg && uarmf))
				putstr(en_win, 0, "Your skin is rotting off.");
			else
				putstr(en_win, 0, "Your rot is hidden under your clothes.");
			message = TRUE;
		} else if(moves>50000 && moves <= 100000){
			if(!(uarmc && uarmg && uarmf && uarm && uarmh))
				putstr(en_win, 0, "Your rotted body bristles with fungal sporangia and burrowing vermin.");
			else
				putstr(en_win, 0, "Your rotted form is hidden under your clothes.");
			message = TRUE;
		} else if(moves>100000){
			if(!(uarmc && uarmg && uarmf && uarm && uarmh && ublindf && ublindf->otyp==MASK))
				putstr(en_win, 0, "Your putrid body is a riot of fungal forms and saprophagous insects.");
			else
				putstr(en_win, 0, "Your putrid form is hidden under your clothes.");
			message = TRUE;
		}
	}
	if(u.specialSealsActive&SEAL_DAHLVER_NAR && !NoBInvis){
		if(dahlverNarVis()){
			putstr(en_win, 0, "Your wounds are full of sharp teeth!");
			message = TRUE;
		}
	}
	if(u.specialSealsActive&SEAL_ACERERAK && !NoBInvis && !(ublindf && ublindf->otyp != LENSES)){
		putstr(en_win, 0, "You have gemstones for eyes!");
		message = TRUE;
	}
	if(u.specialSealsActive&SEAL_COUNCIL && !NoBInvis){
		if(!ublindf || ublindf->otyp == LENSES)
			putstr(en_win, 0, "Your eyes shine like stars.");
		else if(ublindf && (ublindf->otyp == MASK))
			putstr(en_win, 0, "Your star-like eyes shine through your mask.");
		else if(ublindf && (ublindf->otyp == R_LYEHIAN_FACEPLATE))
			putstr(en_win, 0, "Your star-like eyes are covered by your opaque mask.");
		else
			putstr(en_win, 0, "Your star-like eyes are covered by your blindfold.");
		message = TRUE;
	}
	if(u.specialSealsActive&SEAL_COSMOS && !NoBInvis){
		putstr(en_win, 0, "A bright, crystaline aureola hangs behind you.");
		message = TRUE;
	}
	if(u.specialSealsActive&SEAL_MISKA && !NoBInvis){
		putstr(en_win, 0, "You have 4 arms, and a wolf's head grows from either hip.");
		message = TRUE;
	}
	if(u.specialSealsActive&SEAL_NUDZIARTH && !NoBInvis){
		putstr(en_win, 0, "A nimbus of tiny mirrored shards surrounds your head.");
		message = TRUE;
	}
	if(u.specialSealsActive&SEAL_ALIGNMENT_THING && !NoBInvis){
		putstr(en_win, 0, "You see a small black halo just behind your head. There is an eye in the center, staring at you!");
		message = TRUE;
	}
	
	if(!message){
		putstr(en_win, 0, "You think you look pretty normal.");
	}
	
	display_nhwindow(en_win, TRUE);
	destroy_nhwindow(en_win);
	return;
}

/*
 * Courtesy function for non-debug, non-explorer mode players
 * to help refresh them about who/what they are.
 * Returns FALSE if menu cancelled (dismissed with ESC), TRUE otherwise.
 */
STATIC_OVL boolean
minimal_enlightenment()
{
	winid tmpwin;
	menu_item *selected;
	anything any;
	int genidx, n;
	char buf[BUFSZ], buf2[BUFSZ];
	static const char untabbed_fmtstr[] = "%-15s: %-12s";
	static const char untabbed_deity_fmtstr[] = "%-17s%s";
	static const char tabbed_fmtstr[] = "%s:\t%-12s";
	static const char tabbed_deity_fmtstr[] = "%s\t%s";
	static const char *fmtstr;
	static const char *deity_fmtstr;

	fmtstr = iflags.menu_tab_sep ? tabbed_fmtstr : untabbed_fmtstr;
	deity_fmtstr = iflags.menu_tab_sep ?
			tabbed_deity_fmtstr : untabbed_deity_fmtstr; 
	any.a_void = 0;
	buf[0] = buf2[0] = '\0';
	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings, "Starting", FALSE);

	/* Starting name, race, role, gender */
	Sprintf(buf, fmtstr, "name", plname);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
	Sprintf(buf, fmtstr, "race", urace.noun);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
	Sprintf(buf, fmtstr, "role",
		(flags.initgend && urole.name.f) ? urole.name.f : urole.name.m);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
	Sprintf(buf, fmtstr, "gender", genders[flags.initgend].adj);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

	/* Starting alignment */
	Sprintf(buf, fmtstr, "alignment", align_str(u.ualignbase[A_ORIGINAL]));
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

	/* Current name, race, role, gender */
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "", FALSE);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings, "Current", FALSE);
	Sprintf(buf, fmtstr, "race", Upolyd ? youmonst.data->mname : urace.noun);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
	if (Upolyd) {
	    Sprintf(buf, fmtstr, "role (base)",
		(u.mfemale && urole.name.f) ? urole.name.f : urole.name.m);
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
	} else {
	    Sprintf(buf, fmtstr, "role",
		(flags.female && urole.name.f) ? urole.name.f : urole.name.m);
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
	}
	/* don't want poly_gender() here; it forces `2' for non-humanoids */
	genidx = is_neuter(youmonst.data) ? 2 : flags.female;
	Sprintf(buf, fmtstr, "gender", genders[genidx].adj);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
	if (Upolyd && (int)u.mfemale != genidx) {
	    Sprintf(buf, fmtstr, "gender (base)", genders[u.mfemale].adj);
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);
	}

	/* Current alignment */
	Sprintf(buf, fmtstr, "alignment", align_str(u.ualign.type));
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

	/* Deity list */
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, "", FALSE);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings, "Deities", FALSE);
	Sprintf(buf2, deity_fmtstr, align_gname(A_CHAOTIC),
	    (u.ualignbase[A_ORIGINAL] == u.ualign.type
		&& u.ualign.type == A_CHAOTIC) ? " (s,c)" :
	    (u.ualignbase[A_ORIGINAL] == A_CHAOTIC)       ? " (s)" :
	    (u.ualign.type   == A_CHAOTIC)       ? " (c)" : "");
	if(Role_if(PM_EXILE) && Is_astralevel(&u.uz)) Sprintf(buf, fmtstr, "Xaotic", buf2);
	else Sprintf(buf, fmtstr, "Chaotic", buf2);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

	Sprintf(buf2, deity_fmtstr, align_gname(A_NEUTRAL),
	    (u.ualignbase[A_ORIGINAL] == u.ualign.type
		&& u.ualign.type == A_NEUTRAL) ? " (s,c)" :
	    (u.ualignbase[A_ORIGINAL] == A_NEUTRAL)       ? " (s)" :
	    (u.ualign.type   == A_NEUTRAL)       ? " (c)" : "");
	if(Role_if(PM_EXILE) && Is_astralevel(&u.uz)) Sprintf(buf, fmtstr, "Gnostic", buf2);
	else Sprintf(buf, fmtstr, "Neutral", buf2);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

	Sprintf(buf2, deity_fmtstr, align_gname(A_LAWFUL),
	    (u.ualignbase[A_ORIGINAL] == u.ualign.type &&
		u.ualign.type == A_LAWFUL)  ? " (s,c)" :
	    (u.ualignbase[A_ORIGINAL] == A_LAWFUL)        ? " (s)" :
	    (u.ualign.type   == A_LAWFUL)        ? " (c)" : "");
	if(Role_if(PM_EXILE) && Is_astralevel(&u.uz)) Sprintf(buf, fmtstr, "Mundane", buf2);
	else Sprintf(buf, fmtstr, "Lawful", buf2);
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE, buf, FALSE);

	end_menu(tmpwin, "Base Attributes");
	n = select_menu(tmpwin, PICK_NONE, &selected);
	destroy_nhwindow(tmpwin);
	return (n != -1);
}

STATIC_PTR int
doattributes(VOID_ARGS)
{
	if (!minimal_enlightenment())
		return 0;
	if (wizard || discover)
		enlightenment(0);
	else resistances_enlightenment();
	if(u.sealsActive || u.specialSealsActive) signs_enlightenment();
	return 0;
}

/* KMH, #conduct
 * (shares enlightenment's tense handling)
 */
STATIC_PTR int
doconduct(VOID_ARGS)
{
	show_conduct(0);
	return 0;
}

void
show_conduct(final)
int final;
{
	char buf[BUFSZ];
	int ngenocided;

	/* Create the conduct window */
	en_win = create_nhwindow(NHW_MENU);
	putstr(en_win, 0, "Voluntary challenges:");
	putstr(en_win, 0, "");

	if (!u.uconduct.food)
	    enl_msg(You_, "have gone", "went", " without food");
	    /* But beverages are okay */
	else if (!u.uconduct.unvegan)
	    you_have_X("followed a strict vegan diet");
	else if (!u.uconduct.unvegetarian)
	    you_have_been("vegetarian");

	if (!u.uconduct.gnostic)
	    you_have_been("an atheist");

	if (!u.uconduct.weaphit)
	    you_have_never("hit with a wielded weapon");
#ifdef WIZARD
	else if (wizard) {
	    Sprintf(buf, "used a wielded weapon %ld time%s",
		    u.uconduct.weaphit, plur(u.uconduct.weaphit));
	    you_have_X(buf);
	}
#endif
	if (!u.uconduct.killer)
	    you_have_been("a pacifist");

	if (!u.uconduct.literate)
	    you_have_been("illiterate");
#ifdef WIZARD
	else if (wizard) {
	    Sprintf(buf, "read items or engraved %ld time%s",
		    u.uconduct.literate, plur(u.uconduct.literate));
	    you_have_X(buf);
	}
#endif

	ngenocided = num_genocides();
	if (ngenocided == 0) {
	    you_have_never("genocided any monsters");
	} else {
	    Sprintf(buf, "genocided %d type%s of monster%s",
		    ngenocided, plur(ngenocided), plur(ngenocided));
	    you_have_X(buf);
	}

	if (!u.uconduct.polypiles)
	    you_have_never("polymorphed an object");
#ifdef WIZARD
	else if (wizard) {
	    Sprintf(buf, "polymorphed %ld item%s",
		    u.uconduct.polypiles, plur(u.uconduct.polypiles));
	    you_have_X(buf);
	}
#endif

	if (!u.uconduct.polyselfs)
	    you_have_never("changed form");
#ifdef WIZARD
	else if (wizard) {
	    Sprintf(buf, "changed form %ld time%s",
		    u.uconduct.polyselfs, plur(u.uconduct.polyselfs));
	    you_have_X(buf);
	}
#endif

	if (!u.uconduct.wishes)
	    you_have_X("used no wishes");
	else {
	    Sprintf(buf, "used %ld wish%s",
		    u.uconduct.wishes, (u.uconduct.wishes > 1L) ? "es" : "");
	    you_have_X(buf);

	    if (!u.uconduct.wisharti)
		enl_msg(You_, "have not wished", "did not wish",
			" for any artifacts");
	}

	if (u.uconduct.shopID == 0) {
	    you_have_never("paid a shopkeeper to identify an item");
	} else {
	    Sprintf(buf, "paid to have %ld item%s identified",
		    u.uconduct.shopID, plur(u.uconduct.shopID));
	    you_have_X(buf);
	}
	if (u.uconduct.IDs == 0) {
	    you_have_never("magically identified an item");
	} else {
	    Sprintf(buf, "magically identified %ld item%s",
		    u.uconduct.IDs, plur(u.uconduct.shopID));
	    you_have_X(buf);
	}
	/* Pop up the window and wait for a key */
	display_nhwindow(en_win, TRUE);
	destroy_nhwindow(en_win);
}

#endif /* OVLB */
#ifdef OVL1

#ifndef M
# ifndef NHSTDC
#  define M(c)		(0x80 | (c))
# else
#  define M(c)		((c) - 128)
# endif /* NHSTDC */
#endif
#ifndef C
#define C(c)		(0x1f & (c))
#endif

static const struct func_tab cmdlist[] = {
	{C('d'), FALSE, dokick}, /* "D" is for door!...?  Msg is in dokick.c */
#ifdef WIZARD
	{C('g'), TRUE, wiz_genesis},
	{C('i'), TRUE, wiz_identify},
#endif
	{C('l'), TRUE, doredraw}, /* if number_pad is set */
	{C('n'), TRUE, donamelevel}, /* if number_pad is set */
	{C('o'), TRUE, dooverview_or_wiz_where}, /* depending on wizard status */
 	{C('p'), TRUE, doprev_message},
	{C('r'), TRUE, doredraw},
	{C('t'), TRUE, dotele},
#ifdef WIZARD
	{C('v'), TRUE, wiz_level_tele},
#endif
	{C('x'), TRUE, doattributes},
#ifdef SUSPEND
	{C('z'), TRUE, dosuspend},
#endif
	{'a', FALSE, doapply},
	{'A', FALSE, doddoremarm},
	{M('a'), TRUE, doorganize},
/*	'b', 'B' : go sw */
	{'c', FALSE, doclose},
	{'C', TRUE, do_mname},
	{M('c'), TRUE, dotalk},
	{'d', FALSE, dodrop},
	{'D', FALSE, doddrop},
	{M('d'), FALSE, dodip},
	{'e', FALSE, doeat},
/*	{'E', FALSE, doengward},*/
	{'E', FALSE, doengrave},
	{C('e'), TRUE, doseal},
	{M('e'), TRUE, enhance_weapon_skill},
	{'f', FALSE, dofire},
/*	'F' : fight (one time) */
	{M('f'), FALSE, doforce},
	{C('f'), TRUE, dospirit},
/*	'g', 'G' : multiple go */
/*	'h', 'H' : go west */
	{'h', TRUE, dohelp}, /* if number_pad is set */
	{'i', TRUE, ddoinv},
	{'I', TRUE, dotypeinv},		/* Robert Viduya */
	{M('i'), TRUE, doinvoke},
/*	'j', 'J', 'k', 'K', 'l', 'L', 'm', 'M', 'n', 'N' : move commands */
	{'j', FALSE, dojump}, /* if number_pad is on */
	{M('j'), FALSE, dojump},
	{'k', FALSE, dokick}, /* if number_pad is on */
	{'l', FALSE, doloot}, /* if number_pad is on */
	{M('l'), FALSE, doloot},
/*	'n' prefixes a count if number_pad is on */
	{M('m'), TRUE, domonability},
	{'N', TRUE, ddocall}, /* if number_pad is on */
	{M('n'), TRUE, ddocall},
	{M('N'), TRUE, ddocall},
	{'o', FALSE, doopen},
	{'O', TRUE, doset},
	{M('o'), FALSE, dosacrifice},
	{'p', FALSE, dopay},
	{'P', FALSE, doputon},
	{M('p'), TRUE, dopray},
	{'q', FALSE, dodrink},
	{'Q', FALSE, dowieldquiver},
	{C('q'), FALSE, doward},
	{M('q'), TRUE, done2},
	{'r', FALSE, doread},
	{'R', FALSE, doremring},
	{M('r'), FALSE, dorub},
	{'s', TRUE, dosearch, "searching"},
	{'S', TRUE, dosave},
//	{C('s'), FALSE, doseal}, 
	{M('s'), FALSE, dosit},
	{'t', FALSE, dothrow},
	{'T', FALSE, dotakeoff},
	{M('t'), TRUE, doturn},
/*	'u', 'U' : go ne */
	{'u', FALSE, dountrap}, /* if number_pad is on */
	{M('u'), FALSE, dountrap},
	{'v', TRUE, doversion},
	{'V', TRUE, dohistory},
	{M('v'), TRUE, doextversion},
	{'w', FALSE, dowield},
	{'W', FALSE, dowear},
	{C('w'), TRUE, doward},
	{M('w'), FALSE, dowipe},
	{'x', FALSE, doswapweapon},
/*	{'X', TRUE, enter_explore_mode},*/
	{'X', TRUE, dotwoweapon},
/*	'y', 'Y' : go nw */
	{'z', FALSE, dozap},
	{'Z', TRUE, docast},
	{'<', FALSE, doup},
	{'>', FALSE, dodown},
	{'/', TRUE, dowhatis},
	{'&', TRUE, dowhatdoes},
	{'?', TRUE, dohelp},
	{M('?'), TRUE, doextlist},
#ifdef SHELL
	{'!', TRUE, dosh},
#endif
	{'.', TRUE, donull, "waiting"},
	{' ', TRUE, donull, "waiting"},
	{',', FALSE, dopickup},
	{':', TRUE, dolook},
	{';', TRUE, doquickwhatis},
	{'^', TRUE, doidtrap},
	{'\\', TRUE, dodiscovered},		/* Robert Viduya */
	{'@', TRUE, dotogglepickup},
	{M('2'), FALSE, dotwoweapon},
	{WEAPON_SYM,  TRUE, doprwep},
	{ARMOR_SYM,  TRUE, doprarm},
	{RING_SYM,  TRUE, doprring},
	{AMULET_SYM, TRUE, dopramulet},
	{TOOL_SYM, TRUE, doprtool},
	{'*', TRUE, doprinuse},	/* inventory of all equipment in use */
	{GOLD_SYM, TRUE, doprgold},
	{SPBOOK_SYM, TRUE, dovspell},			/* Mike Stephenson */
	{'#', TRUE, doextcmd},
	{'_', TRUE, dotravel},
	{0,0,0,0}
};

struct ext_func_tab extcmdlist[] = {
	{"adjust", "adjust inventory letters", doorganize, TRUE},
	{"annotate", "name current level", donamelevel, TRUE},
	{"chat", "talk to someone", dotalk, TRUE},	/* converse? */
	{"conduct", "list which challenges you have adhered to", doconduct, TRUE},
	{"dip", "dip an object into something", dodip, FALSE},
	{"enhance", "advance or check weapons skills", enhance_weapon_skill,
							TRUE},
	{"equip", "give a pet an item", dopetequip, FALSE},
	{"force", "force a lock", doforce, FALSE},
	{"invoke", "invoke an object's powers", doinvoke, TRUE},
	{"jump", "jump to a location", dojump, FALSE},
	{"loot", "loot a box on the floor", doloot, FALSE},
	{"monster", "use a monster's special ability", domonability, TRUE},
	{"name", "name an item or type of object", ddocall, TRUE},
	{"offer", "offer a sacrifice to the gods", dosacrifice, FALSE},
	{"overview", "show an overview of the dungeon", dooverview, TRUE},
	{"pray", "pray to the gods for help", dopray, TRUE},
	{"quit", "exit without saving current game", done2, TRUE},
#ifdef STEED
	{"ride", "ride (or stop riding) a monster", doride, FALSE},
#endif
	{"rub", "rub a lamp or a stone", dorub, FALSE},
	{"sit", "sit down", dosit, FALSE},
	{"swim", "swim under water", dodeepswim, FALSE},
	{"style", "switch fighting style", dofightingform, TRUE},
	{"turn", "turn undead", doturn, TRUE},
	{"twoweapon", "toggle two-weapon combat", dotwoweapon, FALSE},
	{"untrap", "untrap something", dountrap, FALSE},
	{"version", "list compile time options for this version of NetHack",
		doextversion, TRUE},
	{"wipe", "wipe off your face", dowipe, FALSE},
	{"?", "get this list of extended commands", doextlist, TRUE},
#if defined(WIZARD)
	/*
	 * There must be a blank entry here for every entry in the table
	 * below.
	 */
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
#ifdef DEBUG_MIGRATING_MONS
	{(char *)0, (char *)0, donull, TRUE},
#endif
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
#ifdef PORT_DEBUG
	{(char *)0, (char *)0, donull, TRUE},
#endif
	{(char *)0, (char *)0, donull, TRUE},
    {(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
	{(char *)0, (char *)0, donull, TRUE},
#ifdef DEBUG
	{(char *)0, (char *)0, donull, TRUE},
#endif
	{(char *)0, (char *)0, donull, TRUE},
#endif
	{(char *)0, (char *)0, donull, TRUE}	/* sentinel */
};

#if defined(WIZARD)
static const struct ext_func_tab debug_extcmdlist[] = {
	{"levelchange", "change experience level", wiz_level_change, TRUE},
	{"lightsources", "show mobile light sources", wiz_light_sources, TRUE},
	{"detect", "do wizard detection", wiz_detect, TRUE},
	{"map", "map the current level", wiz_map, TRUE},
#ifdef DEBUG_MIGRATING_MONS
	{"migratemons", "migrate n random monsters", wiz_migrate_mons, TRUE},
#endif
	{"monpolycontrol", "control monster polymorphs", wiz_mon_polycontrol, TRUE},
	{"panic", "test panic routine (fatal to game)", wiz_panic, TRUE},
	{"polyself", "polymorph self", wiz_polyself, TRUE},
#ifdef PORT_DEBUG
	{"portdebug", "wizard port debug command", wiz_port_debug, TRUE},
#endif
	{"seenv", "show seen vectors", wiz_show_seenv, TRUE},
	{"stats", "show memory statistics", wiz_show_stats, TRUE},
	{"timeout", "look at timeout queue", wiz_timeout_queue, TRUE},
	{"vision", "show vision array", wiz_show_vision, TRUE},
	{"wish", "make a wizard wish", wiz_wish, TRUE},
#ifdef DEBUG
	{"wizdebug", "wizard debug command", wiz_debug_cmd, TRUE},
#endif
	{"wmode", "show wall modes", wiz_show_wmodes, TRUE},
	{(char *)0, (char *)0, donull, TRUE}
};

/*
 * Insert debug commands into the extended command list.  This function
 * assumes that the last entry will be the help entry.
 *
 * You must add entries in ext_func_tab every time you add one to the
 * debug_extcmdlist().
 */
void
add_debug_extended_commands()
{
	int i, j, k, n;

	/* count the # of help entries */
	for (n = 0; extcmdlist[n].ef_txt[0] != '?'; n++)
	    ;

	for (i = 0; debug_extcmdlist[i].ef_txt; i++) {
	    for (j = 0; j < n; j++)
		if (strcmp(debug_extcmdlist[i].ef_txt, extcmdlist[j].ef_txt) < 0) break;

	    /* insert i'th debug entry into extcmdlist[j], pushing down  */
	    for (k = n; k >= j; --k)
		extcmdlist[k+1] = extcmdlist[k];
	    extcmdlist[j] = debug_extcmdlist[i];
	    n++;	/* now an extra entry */
	}
}


static const char template[] = "%-18s %4ld  %6ld";
static const char count_str[] = "                   count  bytes";
static const char separator[] = "------------------ -----  ------";

STATIC_OVL void
count_obj(chain, total_count, total_size, top, recurse)
	struct obj *chain;
	long *total_count;
	long *total_size;
	boolean top;
	boolean recurse;
{
	long count, size;
	struct obj *obj;

	for (count = size = 0, obj = chain; obj; obj = obj->nobj) {
	    if (top) {
		count++;
		size += sizeof(struct obj) + obj->oxlth + obj->onamelth;
	    }
	    if (recurse && obj->cobj)
		count_obj(obj->cobj, total_count, total_size, TRUE, TRUE);
	}
	*total_count += count;
	*total_size += size;
}

STATIC_OVL void
obj_chain(win, src, chain, total_count, total_size)
	winid win;
	const char *src;
	struct obj *chain;
	long *total_count;
	long *total_size;
{
	char buf[BUFSZ];
	long count = 0, size = 0;

	count_obj(chain, &count, &size, TRUE, FALSE);
	*total_count += count;
	*total_size += size;
	Sprintf(buf, template, src, count, size);
	putstr(win, 0, buf);
}

STATIC_OVL void
magic_chest_obj_chain(win, src, total_count, total_size)
	winid win;
	const char *src;
	long *total_count;
	long *total_size;
{
	char buf[BUFSZ];
	long count = 0, size = 0;
	int i;

	for(i=0;i<10;i++)
		count_obj(magic_chest_objs[i], &count, &size, TRUE, FALSE);
	*total_count += count;
	*total_size += size;
	Sprintf(buf, template, src, count, size);
	putstr(win, 0, buf);
}

STATIC_OVL void
mon_invent_chain(win, src, chain, total_count, total_size)
	winid win;
	const char *src;
	struct monst *chain;
	long *total_count;
	long *total_size;
{
	char buf[BUFSZ];
	long count = 0, size = 0;
	struct monst *mon;

	for (mon = chain; mon; mon = mon->nmon)
	    count_obj(mon->minvent, &count, &size, TRUE, FALSE);
	*total_count += count;
	*total_size += size;
	Sprintf(buf, template, src, count, size);
	putstr(win, 0, buf);
}

STATIC_OVL void
contained(win, src, total_count, total_size)
	winid win;
	const char *src;
	long *total_count;
	long *total_size;
{
	char buf[BUFSZ];
	long count = 0, size = 0;
	struct monst *mon;
	int i;

	count_obj(invent, &count, &size, FALSE, TRUE);
	count_obj(fobj, &count, &size, FALSE, TRUE);
	count_obj(level.buriedobjlist, &count, &size, FALSE, TRUE);
	count_obj(migrating_objs, &count, &size, FALSE, TRUE);
	for (i = 0; i<10; i++)
	    count_obj(magic_chest_objs[i], &count, &size, FALSE, TRUE);
	/* DEADMONSTER check not required in this loop since they have no inventory */
	for (mon = fmon; mon; mon = mon->nmon)
	    count_obj(mon->minvent, &count, &size, FALSE, TRUE);
	for (mon = migrating_mons; mon; mon = mon->nmon)
	    count_obj(mon->minvent, &count, &size, FALSE, TRUE);

	*total_count += count; *total_size += size;

	Sprintf(buf, template, src, count, size);
	putstr(win, 0, buf);
}

STATIC_OVL void
mon_chain(win, src, chain, total_count, total_size)
	winid win;
	const char *src;
	struct monst *chain;
	long *total_count;
	long *total_size;
{
	char buf[BUFSZ];
	long count, size;
	struct monst *mon;

	for (count = size = 0, mon = chain; mon; mon = mon->nmon) {
	    count++;
	    size += sizeof(struct monst) + mon->mxlth + mon->mnamelth;
	}
	*total_count += count;
	*total_size += size;
	Sprintf(buf, template, src, count, size);
	putstr(win, 0, buf);
}

/*
 * Display memory usage of all monsters and objects on the level.
 */
static int
wiz_show_stats()
{
	char buf[BUFSZ];
	winid win;
	long total_obj_size = 0, total_obj_count = 0;
	long total_mon_size = 0, total_mon_count = 0;

	win = create_nhwindow(NHW_TEXT);
	putstr(win, 0, "Current memory statistics:");
	putstr(win, 0, "");
	Sprintf(buf, "Objects, size %d", (int) sizeof(struct obj));
	putstr(win, 0, buf);
	putstr(win, 0, "");
	putstr(win, 0, count_str);

	obj_chain(win, "invent", invent, &total_obj_count, &total_obj_size);
	obj_chain(win, "fobj", fobj, &total_obj_count, &total_obj_size);
	obj_chain(win, "buried", level.buriedobjlist,
				&total_obj_count, &total_obj_size);
	obj_chain(win, "migrating obj", migrating_objs,
				&total_obj_count, &total_obj_size);
	magic_chest_obj_chain(win, "magic chest obj",
				&total_obj_count,&total_obj_size);
	mon_invent_chain(win, "minvent", fmon,
				&total_obj_count,&total_obj_size);
	mon_invent_chain(win, "migrating minvent", migrating_mons,
				&total_obj_count, &total_obj_size);

	contained(win, "contained",
				&total_obj_count, &total_obj_size);

	putstr(win, 0, separator);
	Sprintf(buf, template, "Total", total_obj_count, total_obj_size);
	putstr(win, 0, buf);

	putstr(win, 0, "");
	putstr(win, 0, "");
	Sprintf(buf, "Monsters, size %d", (int) sizeof(struct monst));
	putstr(win, 0, buf);
	putstr(win, 0, "");

	mon_chain(win, "fmon", fmon,
				&total_mon_count, &total_mon_size);
	mon_chain(win, "migrating", migrating_mons,
				&total_mon_count, &total_mon_size);

	putstr(win, 0, separator);
	Sprintf(buf, template, "Total", total_mon_count, total_mon_size);
	putstr(win, 0, buf);

#if defined(__BORLANDC__) && !defined(_WIN32)
	show_borlandc_stats(win);
#endif

	display_nhwindow(win, FALSE);
	destroy_nhwindow(win);
	return 0;
}

void
sanity_check()
{
	obj_sanity_check();
	timer_sanity_check();
}

#ifdef DEBUG_MIGRATING_MONS
static int
wiz_migrate_mons()
{
	int mcount = 0;
	char inbuf[BUFSZ];
	struct permonst *ptr;
	struct monst *mtmp;
	d_level tolevel;
	getlin("How many random monsters to migrate? [0]", inbuf);
	if (*inbuf == '\033') return 0;
	mcount = atoi(inbuf);
	if (mcount < 0 || mcount > (COLNO * ROWNO) || Is_botlevel(&u.uz))
		return 0;
	while (mcount > 0) {
		if (Is_stronghold(&u.uz))
		    assign_level(&tolevel, &valley_level);
		else
		    get_level(&tolevel, depth(&u.uz) + 1);
		ptr = rndmonst();
		mtmp = makemon(ptr, 0, 0, NO_MM_FLAGS);
		if (mtmp) migrate_to_level(mtmp, ledger_no(&tolevel),
				MIGR_RANDOM, (coord *)0);
		mcount--;
	}
	return 0;
}
#endif

#endif /* WIZARD */

#define unctrl(c)	((c) <= C('z') ? (0x60 | (c)) : (c))
#define unmeta(c)	(0x7f & (c))


void
rhack(cmd)
register char *cmd;
{
	boolean do_walk, do_rush, prefix_seen, bad_command,
		firsttime = (cmd == 0);

	iflags.menu_requested = FALSE;
	if (firsttime) {
		flags.nopick = 0;
		cmd = parse();
	}
	if (*cmd == '\033') {
		flags.move = FALSE;
		return;
	}
#ifdef REDO
	if (*cmd == DOAGAIN && !in_doagain && saveq[0]) {
		in_doagain = TRUE;
		stail = 0;
		rhack((char *)0);	/* read and execute command */
		in_doagain = FALSE;
		return;
	}
	/* Special case of *cmd == ' ' handled better below */
	if(!*cmd || *cmd == (char)0377)
#else
	if(!*cmd || *cmd == (char)0377 || (!flags.rest_on_space && *cmd == ' '))
#endif
	{
		nhbell();
		flags.move = FALSE;
		return;		/* probably we just had an interrupt */
	}
	if (iflags.num_pad && iflags.num_pad_mode == 1) {
		/* This handles very old inconsistent DOS/Windows behaviour
		 * in a new way: earlier, the keyboard handler mapped these,
		 * which caused counts to be strange when entered from the
		 * number pad. Now do not map them until here. 
		 */
		switch (*cmd) {
		    case '5':       *cmd = 'g'; break;
		    case M('5'):    *cmd = 'G'; break;
		    case M('0'):    *cmd = 'I'; break;
        	}
        }
	/* handle most movement commands */
	do_walk = do_rush = prefix_seen = FALSE;
	flags.travel = iflags.travel1 = 0;
	switch (*cmd) {
	 case 'g':  if (movecmd(cmd[1])) {
			flags.run = 2;
			do_rush = TRUE;
		    } else
			prefix_seen = TRUE;
		    break;
	 case '5':  if (!iflags.num_pad) break;	/* else FALLTHRU */
	 case 'G':  if (movecmd(lowc(cmd[1]))) {
			flags.run = 3;
			do_rush = TRUE;
		    } else
			prefix_seen = TRUE;
		    break;
	 case '-':  if (!iflags.num_pad) break;	/* else FALLTHRU */
	/* Effects of movement commands and invisible monsters:
	 * m: always move onto space (even if 'I' remembered)
	 * F: always attack space (even if 'I' not remembered)
	 * normal movement: attack if 'I', move otherwise
	 */
	 case 'F':  if (movecmd(cmd[1])) {
			flags.forcefight = 1;
			do_walk = TRUE;
		    } else
			prefix_seen = TRUE;
		    break;
	 case 'm':  if (movecmd(cmd[1]) || u.dz) {
			flags.run = 0;
			flags.nopick = 1;
			if (!u.dz) do_walk = TRUE;
			else cmd[0] = cmd[1];	/* "m<" or "m>" */
		    } else
			prefix_seen = TRUE;
		    break;
	 case 'M':  if (movecmd(lowc(cmd[1]))) {
			flags.run = 1;
			flags.nopick = 1;
			do_rush = TRUE;
		    } else
			prefix_seen = TRUE;
		    break;
	 case '0':  if (!iflags.num_pad) break;
		    (void)ddoinv(); /* a convenience borrowed from the PC */
		    flags.move = FALSE;
		    multi = 0;
		    return;
	 case CMD_TRAVEL:
		    if (iflags.travelcmd) {
			    flags.travel = 1;
			    iflags.travel1 = 1;
			    flags.run = 8;
			    flags.nopick = 1;
			    do_rush = TRUE;
			    break;
		    }
		    /*FALLTHRU*/
	 default:   if (movecmd(*cmd)) {	/* ordinary movement */
			flags.run = 0;	/* only matters here if it was 8 */
			do_walk = TRUE;
		    } else if (movecmd(iflags.num_pad ?
				       unmeta(*cmd) : lowc(*cmd))) {
			flags.run = 1;
			do_rush = TRUE;
		    } else if (movecmd(unctrl(*cmd))) {
			flags.run = 3;
			do_rush = TRUE;
		    }
		    break;
	}

	/* some special prefix handling */
	/* overload 'm' prefix for ',' to mean "request a menu" */
	if (prefix_seen && cmd[1] == ',') {
		iflags.menu_requested = TRUE;
		++cmd;
	}

	if (do_walk) {
	    if (multi) flags.mv = TRUE;
	    domove();
	    flags.forcefight = 0;
	    return;
	} else if (do_rush) {
	    if (firsttime) {
		if (!multi) multi = max(COLNO,ROWNO);
		u.last_str_turn = 0;
	    }
	    flags.mv = TRUE;
	    domove();
	    return;
	} else if (prefix_seen && cmd[1] == '\033') {	/* <prefix><escape> */
	    /* don't report "unknown command" for change of heart... */
	    bad_command = FALSE;
	} else if (*cmd == ' ' && !flags.rest_on_space) {
	    bad_command = TRUE;		/* skip cmdlist[] loop */

	/* handle all other commands */
	} else {
	    register const struct func_tab *tlist;
	    int res, NDECL((*func));

	    for (tlist = cmdlist; tlist->f_char; tlist++) {
		if ((*cmd & 0xff) != (tlist->f_char & 0xff)) continue;

		if (u.uburied && !tlist->can_if_buried) {
		    You_cant("do that while you are buried!");
		    res = 0;
		} else {
		    /* we discard 'const' because some compilers seem to have
		       trouble with the pointer passed to set_occupation() */
		    func = ((struct func_tab *)tlist)->f_funct;
		    if (tlist->f_text && !occupation && multi)
			set_occupation(func, tlist->f_text, multi);
		    res = (*func)();		/* perform the command */
		}
		if (!res) {
		    flags.move = FALSE;
		    multi = 0;
		}
		return;
	    }
	    /* if we reach here, cmd wasn't found in cmdlist[] */
	    bad_command = TRUE;
	}

	if (bad_command) {
	    char expcmd[10];
	    register char *cp = expcmd;

	    while (*cmd && (int)(cp - expcmd) < (int)(sizeof expcmd - 3)) {
		if (*cmd >= 040 && *cmd < 0177) {
		    *cp++ = *cmd++;
		} else if (*cmd & 0200) {
			char c = *cmd++;
		    *cp++ = 'M';
		    *cp++ = '-';
		    *cp++ = c & ~0200;
		} else {
		    *cp++ = '^';
		    *cp++ = *cmd++ ^ 0100;
		}
	    }
	    *cp = '\0';
	    if (!prefix_seen || !iflags.cmdassist ||
		!help_dir(0, "Invalid direction key!"))
		Norep("Unknown command '%s'.", expcmd);
	}
	/* didn't move */
	flags.move = FALSE;
	multi = 0;
	return;
}

int
xytod(x, y)	/* convert an x,y pair into a direction code */
schar x, y;
{
	register int dd;

	for(dd = 0; dd < 8; dd++)
	    if(x == xdir[dd] && y == ydir[dd]) return dd;

	return -1;
}

void
dtoxy(cc,dd)	/* convert a direction code into an x,y pair */
coord *cc;
register int dd;
{
	cc->x = xdir[dd];
	cc->y = ydir[dd];
	return;
}

int
movecmd(sym)	/* also sets u.dz, but returns false for <> */
char sym;
{
	register const char *dp;
	register const char *sdp;
	if(iflags.num_pad) sdp = ndir; else sdp = sdir;	/* DICE workaround */

	u.dz = 0;
	if(!(dp = index(sdp, sym))) return 0;
	u.dx = xdir[dp-sdp];
	u.dy = ydir[dp-sdp];
	u.dz = zdir[dp-sdp];
	if (u.dx && u.dy && (u.umonnum == PM_GRID_BUG || u.umonnum == PM_BEBELITH)) {
		u.dx = u.dy = 0;
		return 0;
	}
	return !u.dz;
}

/*
 * uses getdir() but unlike getdir() it specifically
 * produces coordinates using the direction from getdir()
 * and verifies that those coordinates are ok.
 *
 * If the call to getdir() returns 0, Never_mind is displayed.
 * If the resulting coordinates are not okay, emsg is displayed.
 *
 * Returns non-zero if coordinates in cc are valid.
 */
int get_adjacent_loc(prompt,emsg,x,y,cc)
const char *prompt, *emsg;
xchar x,y;
coord *cc;
{
	xchar new_x, new_y;
	if (!getdir(prompt)) {
		pline(Never_mind);
		return 0;
	}
	new_x = x + u.dx;
	new_y = y + u.dy;
	if (cc && isok(new_x,new_y)) {
		cc->x = new_x;
		cc->y = new_y;
	} else {
		if (emsg) pline(emsg);
		return 0;
	}
	return 1;
}

int
getdir(s)
const char *s;
{
	char dirsym;

#ifdef REDO
	if(in_doagain || *readchar_queue)
	    dirsym = readchar();
	else
#endif
	    dirsym = yn_function ((s && *s != '^') ? s : "In what direction?",
					(char *)0, '\0');
#ifdef REDO
	savech(dirsym);
#endif
	if(dirsym == '.' || dirsym == 's')
		u.dx = u.dy = u.dz = 0;
	else if(!movecmd(dirsym) && !u.dz) {
		boolean did_help = FALSE;
		if(!index(quitchars, dirsym)) {
		    if (iflags.cmdassist) {
			did_help = help_dir((s && *s == '^') ? dirsym : 0,
					    "Invalid direction key!");
		    }
		    if (!did_help) pline("What a strange direction!");
		}
		return 0;
	}
	if(!u.dz && ((Stunned || (Confusion && !rn2(5))) && (u.udrunken < 3 || !rn2(u.udrunken/3)))) confdir();
	return 1;
}

STATIC_OVL boolean
help_dir(sym, msg)
char sym;
const char *msg;
{
	char ctrl;
	winid win;
	static const char wiz_only_list[] = "EFGIOVW";
	char buf[BUFSZ], buf2[BUFSZ], *expl;

	win = create_nhwindow(NHW_TEXT);
	if (!win) return FALSE;
	if (msg) {
		Sprintf(buf, "cmdassist: %s", msg);
		putstr(win, 0, buf);
		putstr(win, 0, "");
	}
	if (letter(sym)) { 
	    sym = highc(sym);
	    ctrl = (sym - 'A') + 1;
	    if ((expl = dowhatdoes_core(ctrl, buf2))
		&& (!index(wiz_only_list, sym)
#ifdef WIZARD
		    || wizard
#endif
	                     )) {
		Sprintf(buf, "Are you trying to use ^%c%s?", sym,
			index(wiz_only_list, sym) ? "" :
			" as specified in the Guidebook");
		putstr(win, 0, buf);
		putstr(win, 0, "");
		putstr(win, 0, expl);
		putstr(win, 0, "");
		putstr(win, 0, "To use that command, you press");
		Sprintf(buf,
			"the <Ctrl> key, and the <%c> key at the same time.", sym);
		putstr(win, 0, buf);
		putstr(win, 0, "");
	    }
	}
	if (iflags.num_pad && (u.umonnum == PM_GRID_BUG || u.umonnum == PM_BEBELITH)) {
	    putstr(win, 0, "Valid direction keys in your current form (with number_pad on) are:");
	    putstr(win, 0, "             8   ");
	    putstr(win, 0, "             |   ");
	    putstr(win, 0, "          4- . -6");
	    putstr(win, 0, "             |   ");
	    putstr(win, 0, "             2   ");
	} else if (u.umonnum == PM_GRID_BUG || u.umonnum == PM_BEBELITH) {
	    putstr(win, 0, "Valid direction keys in your current form are:");
	    putstr(win, 0, "             k   ");
	    putstr(win, 0, "             |   ");
	    putstr(win, 0, "          h- . -l");
	    putstr(win, 0, "             |   ");
	    putstr(win, 0, "             j   ");
	} else if (iflags.num_pad) {
	    putstr(win, 0, "Valid direction keys (with number_pad on) are:");
	    putstr(win, 0, "          7  8  9");
	    putstr(win, 0, "           \\ | / ");
	    putstr(win, 0, "          4- . -6");
	    putstr(win, 0, "           / | \\ ");
	    putstr(win, 0, "          1  2  3");
	} else {
	    putstr(win, 0, "Valid direction keys are:");
	    putstr(win, 0, "          y  k  u");
	    putstr(win, 0, "           \\ | / ");
	    putstr(win, 0, "          h- . -l");
	    putstr(win, 0, "           / | \\ ");
	    putstr(win, 0, "          b  j  n");
	};
	putstr(win, 0, "");
	putstr(win, 0, "          <  up");
	putstr(win, 0, "          >  down");
	putstr(win, 0, "          .  direct at yourself");
	putstr(win, 0, "");
	putstr(win, 0, "(Suppress this message with !cmdassist in config file.)");
	display_nhwindow(win, FALSE);
	destroy_nhwindow(win);
	return TRUE;
}

#endif /* OVL1 */
#ifdef OVLB

void
confdir()
{
	register int x = (u.umonnum == PM_GRID_BUG || u.umonnum == PM_BEBELITH) ? 2*rn2(4) : rn2(8);
	u.dx = xdir[x];
	u.dy = ydir[x];
	return;
}

#endif /* OVLB */
#ifdef OVL0

int
isok(x,y)
register int x, y;
{
	/* x corresponds to curx, so x==1 is the first column. Ach. %% */
	return x >= 1 && x <= COLNO-1 && y >= 0 && y <= ROWNO-1;
}

static NEARDATA int last_multi;

/*
 * convert a MAP window position into a movecmd
 */
const char *
click_to_cmd(x, y, mod)
    int x, y, mod;
{
    int dir;
    static char cmd[4];
    cmd[1]=0;

    x -= u.ux;
    y -= u.uy;

    if (iflags.travelcmd) {
        if (abs(x) <= 1 && abs(y) <= 1 ) {
            x = sgn(x), y = sgn(y);
        } else {
            u.tx = u.ux+x;
            u.ty = u.uy+y;
            cmd[0] = CMD_TRAVEL;
            return cmd;
        }

        if(x == 0 && y == 0) {
            /* here */
            if(IS_FOUNTAIN(levl[u.ux][u.uy].typ) || IS_SINK(levl[u.ux][u.uy].typ)) {
                cmd[0]=mod == CLICK_1 ? 'q' : M('d');
                return cmd;
            } else if(IS_THRONE(levl[u.ux][u.uy].typ)) {
                cmd[0]=M('s');
                return cmd;
            } else if((u.ux == xupstair && u.uy == yupstair)
                      || (u.ux == sstairs.sx && u.uy == sstairs.sy && sstairs.up)
                      || (u.ux == xupladder && u.uy == yupladder)) {
                return "<";
            } else if((u.ux == xdnstair && u.uy == ydnstair)
                      || (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up)
                      || (u.ux == xdnladder && u.uy == ydnladder)) {
                return ">";
            } else if(OBJ_AT(u.ux, u.uy)) {
                cmd[0] = Is_container(level.objects[u.ux][u.uy]) ? M('l') : ',';
                return cmd;
            } else {
                return "."; /* just rest */
            }
        }

        /* directional commands */

        dir = xytod(x, y);

	if (!m_at(u.ux+x, u.uy+y) && !test_move(u.ux, u.uy, x, y, TEST_MOVE)) {
            cmd[1] = (iflags.num_pad ? ndir[dir] : sdir[dir]);
            cmd[2] = 0;
            if (IS_DOOR(levl[u.ux+x][u.uy+y].typ)) {
                /* slight assistance to the player: choose kick/open for them */
                if (levl[u.ux+x][u.uy+y].doormask & D_LOCKED) {
                    cmd[0] = C('d');
                    return cmd;
                }
                if (levl[u.ux+x][u.uy+y].doormask & D_CLOSED) {
                    cmd[0] = 'o';
                    return cmd;
                }
            }
            if (levl[u.ux+x][u.uy+y].typ <= SCORR) {
                cmd[0] = 's';
                cmd[1] = 0;
                return cmd;
            }
        }
    } else {
        /* convert without using floating point, allowing sloppy clicking */
        if(x > 2*abs(y))
            x = 1, y = 0;
        else if(y > 2*abs(x))
            x = 0, y = 1;
        else if(x < -2*abs(y))
            x = -1, y = 0;
        else if(y < -2*abs(x))
            x = 0, y = -1;
        else
            x = sgn(x), y = sgn(y);

        if(x == 0 && y == 0)	/* map click on player to "rest" command */
            return ".";

        dir = xytod(x, y);
    }

    /* move, attack, etc. */
    cmd[1] = 0;
    if(mod == CLICK_1) {
	cmd[0] = (iflags.num_pad ? ndir[dir] : sdir[dir]);
    } else {
	cmd[0] = (iflags.num_pad ? M(ndir[dir]) :
		(sdir[dir] - 'a' + 'A')); /* run command */
    }

    return cmd;
}

STATIC_OVL char *
parse()
{
#ifdef LINT	/* static char in_line[COLNO]; */
	char in_line[COLNO];
#else
	static char in_line[COLNO];
#endif
	register int foo;
	boolean prezero = FALSE;

	multi = 0;
	flags.move = 1;
	flush_screen(1); /* Flush screen buffer. Put the cursor on the hero. */

	if (!iflags.num_pad || (foo = readchar()) == 'n')
	    for (;;) {
		foo = readchar();
		if (foo >= '0' && foo <= '9') {
		    multi = 10 * multi + foo - '0';
		    if (multi < 0 || multi >= LARGEST_INT) multi = LARGEST_INT;
		    if (multi > 9) {
			clear_nhwindow(WIN_MESSAGE);
			Sprintf(in_line, "Count: %d", multi);
			pline(in_line);
			mark_synch();
		    }
		    last_multi = multi;
		    if (!multi && foo == '0') prezero = TRUE;
		} else break;	/* not a digit */
	    }

	if (foo == '\033') {   /* esc cancels count (TH) */
	    clear_nhwindow(WIN_MESSAGE);
	    multi = last_multi = 0;
# ifdef REDO
	} else if (foo == DOAGAIN || in_doagain) {
	    multi = last_multi;
	} else {
	    last_multi = multi;
	    savech(0);	/* reset input queue */
	    savech((char)foo);
# endif
	}

	if (multi) {
	    multi--;
	    save_cm = in_line;
	} else {
	    save_cm = (char *)0;
	}
	in_line[0] = foo;
	in_line[1] = '\0';
	if (foo == 'g' || foo == 'G' || foo == 'm' || foo == 'M' ||
	    foo == 'F' || (iflags.num_pad && (foo == '5' || foo == '-'))) {
	    foo = readchar();
#ifdef REDO
	    savech((char)foo);
#endif
	    in_line[1] = foo;
	    in_line[2] = 0;
	}
	clear_nhwindow(WIN_MESSAGE);
	if (prezero) in_line[0] = '\033';
	return(in_line);
}

#endif /* OVL0 */
#ifdef OVLB

#ifdef UNIX
static
void
end_of_input()
{
#ifndef NOSAVEONHANGUP
	if (!program_state.done_hup++ && program_state.something_worth_saving)
	    (void) dosave0();
#endif
	exit_nhwindows((char *)0);
	clearlocks();
	terminate(EXIT_SUCCESS);
}
#endif

#endif /* OVLB */
#ifdef OVL0

char
readchar()
{
	register int sym;
	int x = u.ux, y = u.uy, mod = 0;

	if ( *readchar_queue )
	    sym = *readchar_queue++;
	else
#ifdef REDO
	    sym = in_doagain ? Getchar() : nh_poskey(&x, &y, &mod);
#else
	    sym = Getchar();
#endif

#ifdef UNIX
# ifdef NR_OF_EOFS
	if (sym == EOF) {
	    register int cnt = NR_OF_EOFS;
	  /*
	   * Some SYSV systems seem to return EOFs for various reasons
	   * (?like when one hits break or for interrupted systemcalls?),
	   * and we must see several before we quit.
	   */
	    do {
		clearerr(stdin);	/* omit if clearerr is undefined */
		sym = Getchar();
	    } while (--cnt && sym == EOF);
	}
# endif /* NR_OF_EOFS */
	if (sym == EOF)
	    end_of_input();
#endif /* UNIX */

	if(sym == 0) {
	    /* click event */
	    readchar_queue = click_to_cmd(x, y, mod);
	    sym = *readchar_queue++;
	}
	return((char) sym);
}

int
dotravel(VOID_ARGS)
{
	/* Keyboard travel command */
	static char cmd[2];
	coord cc;

	if (!iflags.travelcmd) return 0;
	cmd[1]=0;
	cc.x = iflags.travelcc.x;
	cc.y = iflags.travelcc.y;
	if (cc.x == -1 && cc.y == -1) {
	    /* No cached destination, start attempt from current position */
	    cc.x = u.ux;
	    cc.y = u.uy;
	}
	pline("Where do you want to travel to?");
	if (getpos(&cc, TRUE, "the desired destination") < 0) {
		/* user pressed ESC */
		return 0;
	}
	iflags.travelcc.x = u.tx = cc.x;
	iflags.travelcc.y = u.ty = cc.y;
	cmd[0] = CMD_TRAVEL;
	readchar_queue = cmd;
	return 0;
}

#ifdef PORT_DEBUG
# ifdef WIN32CON
extern void NDECL(win32con_debug_keystrokes);
extern void NDECL(win32con_handler_info);
# endif

int
wiz_port_debug()
{
	int n, k;
	winid win;
	anything any;
	int item = 'a';
	int num_menu_selections;
	struct menu_selection_struct {
		char *menutext;
		void NDECL((*fn));
	} menu_selections[] = {
#ifdef WIN32CON
		{"test win32 keystrokes", win32con_debug_keystrokes},
		{"show keystroke handler information", win32con_handler_info},
#endif
		{(char *)0, (void NDECL((*)))0}		/* array terminator */
	};

	num_menu_selections = SIZE(menu_selections) - 1;
	if (num_menu_selections > 0) {
		menu_item *pick_list;
		win = create_nhwindow(NHW_MENU);
		start_menu(win);
		for (k=0; k < num_menu_selections; ++k) {
			any.a_int = k+1;
			add_menu(win, NO_GLYPH, &any, item++, 0, ATR_NONE,
				menu_selections[k].menutext, MENU_UNSELECTED);
		}
		end_menu(win, "Which port debugging feature?");
		n = select_menu(win, PICK_ONE, &pick_list);
		destroy_nhwindow(win);
		if (n > 0) {
			n = pick_list[0].item.a_int - 1;
			free((genericptr_t) pick_list);
			/* execute the function */
			(*menu_selections[n].fn)();
		}
	} else
		pline("No port-specific debug capability defined.");
	return 0;
}
# endif /*PORT_DEBUG*/

#endif /* OVL0 */
#ifdef OVLB
/*
 *   Parameter validator for generic yes/no function to prevent
 *   the core from sending too long a prompt string to the
 *   window port causing a buffer overflow there.
 */
char
yn_function(plainquery,resp, def)
const char *plainquery,*resp;
char def;
{
	char qbuf[QBUFSZ];
	const char *query;
	unsigned truncspot, reduction = sizeof(" [N]  ?") + 1;
	
	/*Ben Collver's fixes*/
//	if(Role_if(PM_PIRATE)) query = piratesay(plainquery);
//	else query = plainquery;
	query = plainquery;
	
	if (resp) reduction += strlen(resp) + sizeof(" () ");
	if (strlen(query) < (QBUFSZ - reduction))
		return (*windowprocs.win_yn_function)(query, resp, def);
	paniclog("Query truncated: ", query);
	reduction += sizeof("...");
	truncspot = QBUFSZ - reduction;
	(void) strncpy(qbuf, query, (int)truncspot);
	qbuf[truncspot] = '\0';
	Strcat(qbuf,"...");
	return (*windowprocs.win_yn_function)(qbuf, resp, def);
}
#endif

/*cmd.c*/
