/*	SCCS Id: @(#)decl.h	3.4	2001/12/10	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DECL_H
#define DECL_H

#define E extern

E int NDECL((*occupation));
E int NDECL((*afternmv));

E const char *hname;
E int hackpid;
#if defined(UNIX) || defined(VMS)
E int locknum;
#endif
#ifdef DEF_PAGER
E char *catmore;
#endif	/* DEF_PAGER */

E char SAVEF[];
#ifdef MICRO
E char SAVEP[];
#endif

E NEARDATA int bases[MAXOCLASSES];

E long last_clear_screen;

E NEARDATA int multi;
E char multi_txt[BUFSZ];
#if 0
E NEARDATA int warnlevel;
#endif
E NEARDATA int nroom;
E NEARDATA int nsubroom;
E NEARDATA int occtime;

#define WARNCOUNT 6			/* number of different warning levels */
E uchar warnsyms[WARNCOUNT];

E int x_maze_max, y_maze_max;
E int otg_temp;

#ifdef REDO
E NEARDATA int in_doagain;
#endif

E struct dgn_topology {		/* special dungeon levels for speed */
	/*Dungeons of Doom*/
    d_level	d_oracle_level;
    d_level	d_bigroom_level;	/* unused */
    d_level	d_bigroomb_level;	/* unused */
#ifdef REINCARNATION
    d_level	d_rogue_level;
#endif
	/*Medusa's Island*/
    d_level	d_medusa_level;
	/*Castle*/
	d_level	d_stronghold_level;
	/*Gehennom*/
    d_level	d_valley_level;
    d_level	d_stair1_level;
    d_level	d_stair2_level;
    d_level	d_stair3_level;
    d_level	d_wiz1_level;
    d_level	d_wiz2_level;
    d_level	d_wiz3_level;
//    d_level	d_juiblex_level;
//    d_level	d_orcus_level;
    d_level	d_hell1_level;
	int		hell1_variant;
#define BAEL_LEVEL		1
#define DISPATER_LEVEL	2
#define MAMMON_LEVEL	3
#define BELIAL_LEVEL	4
	d_level	d_hell2_level;
	int		hell2_variant;
#define LEVIATHAN_LEVEL	1
#define LILITH_LEVEL	2
#define BAALZEBUB_LEVEL	3
#define MEPHISTOPHELES_LEVEL	4
	d_level	d_hell3_level;
	d_level	d_abyss_level;
	int		abyss_variant;
#define JUIBLEX_LEVEL	1
#define JUIBLEX_RES		1
#define ZUGGTMOY_LEVEL	2
#define ZUGGTMOY_RES	2
#define YEENOGHU_LEVEL	3
#define BAPHOMET_LEVEL	4
#define NIGHT_LEVEL		5
#define NIGHT_RES		3
	d_level	d_abys2_level;
	int		abys2_variant;
#define ORCUS_LEVEL		1
#define MALCANTHET_LEVEL	2
#define GRAZ_ZT_LEVEL	3
#define LOLTH_LEVEL		4
	d_level	d_brine_level;
	int		brine_variant;
#define DEMOGORGON_LEVEL	1
#define DAGON_LEVEL		2
#define LAMASHTU_LEVEL		3
//    d_level	d_baalzebub_level;	/* unused */
//    d_level	d_asmodeus_level;	/* unused */
    d_level	d_portal_level;		/* only in goto_level() [do.c] */
    d_level	d_sanctum_level;
	/*Planes*/
    d_level	d_earth_level;
    d_level	d_water_level;
    d_level	d_fire_level;
    d_level	d_air_level;
    d_level	d_astral_level;
	/*Vlad's Tower*/
    xchar	d_tower_dnum;
	/*The Lost Tomb*/
    xchar	d_tomb_dnum;
	/*The Sunless Sea*/
    xchar	d_sea_dnum;
	int		sea_variant;
#define SUNLESS_SEA_LEVEL		1
#define PARADISE_ISLAND_LEVEL	2
#define SUNKEN_CITY_LEVEL		3
#define PEANUT_ISLAND_LEVEL		4
	/*The Temple of Moloch*/
    xchar	d_temple_dnum;
	/*Sokoban*/
    xchar	d_sokoban_dnum;
	/*Mines of the Gnomes of Zurich*/
    xchar	d_mines_dnum;
	/*Quest Levels*/
	xchar	d_quest_dnum;
    d_level	d_qstart_level, d_qlocate_level, d_nemesis_level;
	/*Neutral Quest*/
	xchar	d_neutral_dnum;
	xchar	d_rlyeh_dnum;
    d_level	d_gatetown_level, d_spire_level, d_sum_of_all_level, d_rlyeh_level;
	/*Chaos Quest*/
	xchar	d_chaos_dnum;
    d_level	d_chaosf_level, d_chaoss_level, d_chaost_level, d_chaosm_level,
		d_chaosfrh_level, d_chaosffh_level, d_chaossth_level,
		d_chaosvth_level, d_chaose_level;
	/*Law Quest*/
	xchar	d_law_dnum;
	d_level d_path1, d_path2, d_path3, d_illregrd, 
		d_arcadia1, d_arcadia2, d_arcadia3, d_arcward, d_arcfort,
		d_tower1, d_tower2, d_tower3, d_tower4, d_tower5,
		d_tower6, d_tower7, d_tower8, d_tower9, d_towertop;
	boolean alt_tower;
	/*Fort Knox*/
    d_level	d_knox_level;
	d_level d_minetown_level;
#ifdef RECORD_ACHIEVE
    d_level     d_mineend_level;
    d_level     d_sokoend_level;
#endif

} dungeon_topology;
/* macros for accesing the dungeon levels by their old names */
	/*Dungeons of Doom*/
#define oracle_level		(dungeon_topology.d_oracle_level)
#define bigroom_level		(dungeon_topology.d_bigroom_level)
#ifdef REINCARNATION
#define rogue_level		(dungeon_topology.d_rogue_level)
#endif
#define medusa_level		(dungeon_topology.d_medusa_level)
#define stronghold_level	(dungeon_topology.d_stronghold_level)
	/*Gehennom*/
#define valley_level		(dungeon_topology.d_valley_level)
#define wiz1_level		(dungeon_topology.d_wiz1_level)
#define wiz2_level		(dungeon_topology.d_wiz2_level)
#define wiz3_level		(dungeon_topology.d_wiz3_level)
#define abyss1_level	(dungeon_topology.d_abyss_level)
#define abyss1_juiblex_level	(dungeon_topology.d_abyss_juiblex_level)
#define abyss2_level	(dungeon_topology.d_abys2_level)
#define orcus_level		(dungeon_topology.d_abys2_level)
#define abyss3_level	(dungeon_topology.d_brine_level)
#define hell1_level		(dungeon_topology.d_hell1_level)
#define hell2_level		(dungeon_topology.d_hell2_level)
#define hell3_level		(dungeon_topology.d_hell3_level)
////define juiblex_level		(dungeon_topology.d_juiblex_level)
//define orcus_level		(dungeon_topology.d_orcus_level)
//define demogoron_level		(dungeon_topology.d_demogorgon_level)
//define baalzebub_level		(dungeon_topology.d_baalzebub_level)
//define asmodeus_level		(dungeon_topology.d_asmodeus_level)
#define portal_level		(dungeon_topology.d_portal_level)
#define sanctum_level		(dungeon_topology.d_sanctum_level)
	/*Planes*/
#define earth_level		(dungeon_topology.d_earth_level)
#define water_level		(dungeon_topology.d_water_level)
#define fire_level		(dungeon_topology.d_fire_level)
#define air_level		(dungeon_topology.d_air_level)
#define astral_level		(dungeon_topology.d_astral_level)
	/*Vlad's Tower*/
#define tower_dnum		(dungeon_topology.d_tower_dnum)
	/*The Lost Tomb*/
#define tomb_dnum		(dungeon_topology.d_tomb_dnum)
	/*The Sunless Sea*/
#define sea_dnum		(dungeon_topology.d_sea_dnum)
	/*The Temple of Moloch*/
#define temple_dnum		(dungeon_topology.d_temple_dnum)
	/*Medusa's Island*/
#define medusa_level		(dungeon_topology.d_medusa_level)
	/*Sokoban*/
#define sokoban_dnum		(dungeon_topology.d_sokoban_dnum)
	/*Mines of the Gnomes of Zurich*/
#define mines_dnum		(dungeon_topology.d_mines_dnum)
	/*Quest Levels*/
#define quest_dnum		(dungeon_topology.d_quest_dnum)
#define qstart_level		(dungeon_topology.d_qstart_level)
#define qlocate_level		(dungeon_topology.d_qlocate_level)
#define nemesis_level		(dungeon_topology.d_nemesis_level)
	/*Neutral Quest*/
#define neutral_dnum		(dungeon_topology.d_neutral_dnum)
#define rlyeh_dnum		(dungeon_topology.d_rlyeh_dnum)
#define gatetown_level		(dungeon_topology.d_gatetown_level)
#define spire_level		(dungeon_topology.d_spire_level)
#define sum_of_all_level		(dungeon_topology.d_sum_of_all_level)
#define rlyeh_level		(dungeon_topology.d_rlyeh_level)
	/*Chaos Quest*/
#define chaos_dnum		(dungeon_topology.d_chaos_dnum)
#define chaosf_level		(dungeon_topology.d_chaosf_level)
#define chaoss_level		(dungeon_topology.d_chaoss_level)
#define chaost_level		(dungeon_topology.d_chaost_level)
#define chaosm_level		(dungeon_topology.d_chaosm_level)
#define chaosfrh_level		(dungeon_topology.d_chaosfrh_level)
#define chaosffh_level		(dungeon_topology.d_chaosffh_level)
#define chaossth_level		(dungeon_topology.d_chaossth_level)
#define chaosvth_level		(dungeon_topology.d_chaosvth_level)
#define chaose_level		(dungeon_topology.d_chaose_level)
	/*Law Quest*/
#define law_dnum		(dungeon_topology.d_law_dnum)
#define path1_level		(dungeon_topology.d_path1)
#define path2_level		(dungeon_topology.d_path2)
#define path3_level		(dungeon_topology.d_path3)
#define illregrd_level	(dungeon_topology.d_illregrd)
#define arcadia1_level	(dungeon_topology.d_arcadia1)
#define arcadia2_level	(dungeon_topology.d_arcadia2)
#define arcadia3_level	(dungeon_topology.d_arcadia3)
#define arcward_level	(dungeon_topology.d_arcward)
#define arcfort_level	(dungeon_topology.d_arcfort)
#define tower1_level	(dungeon_topology.d_tower1)
#define tower2_level	(dungeon_topology.d_tower2)
#define tower3_level	(dungeon_topology.d_tower3)
//define tower4_level	(dungeon_topology.d_tower4)
//define tower5_level	(dungeon_topology.d_tower5)
//define tower6_level	(dungeon_topology.d_tower6)
//define tower7_level	(dungeon_topology.d_tower7)
//define tower8_level	(dungeon_topology.d_tower8)
//define tower9_level	(dungeon_topology.d_tower9)
#define towertop_level	(dungeon_topology.d_towertop)
	/*Fort Knox*/
#define knox_level		(dungeon_topology.d_knox_level)
#define minetown_level           (dungeon_topology.d_minetown_level)
#ifdef RECORD_ACHIEVE
#define mineend_level           (dungeon_topology.d_mineend_level)
#define sokoend_level           (dungeon_topology.d_sokoend_level)
#endif

E NEARDATA stairway dnstair, upstair;		/* stairs up and down */
#define xdnstair	(dnstair.sx)
#define ydnstair	(dnstair.sy)
#define xupstair	(upstair.sx)
#define yupstair	(upstair.sy)

E NEARDATA stairway dnladder, upladder;		/* ladders up and down */
#define xdnladder	(dnladder.sx)
#define ydnladder	(dnladder.sy)
#define xupladder	(upladder.sx)
#define yupladder	(upladder.sy)

E NEARDATA stairway sstairs;

E NEARDATA dest_area updest, dndest;	/* level-change destination areas */

E NEARDATA coord inv_pos;
E NEARDATA dungeon dungeons[];
E NEARDATA s_level *sp_levchn;
#define dunlev_reached(x)	(dungeons[(x)->dnum].dunlev_ureached)

#include "quest.h"
E struct q_score quest_status;

E NEARDATA char pl_character[PL_CSIZ];
E NEARDATA char pl_race;		/* character's race */

E NEARDATA char pl_fruit[PL_FSIZ];
E NEARDATA int current_fruit;
E NEARDATA struct fruit *ffruit;

E NEARDATA char tune[6];

#define MAXLINFO (MAXDUNGEON * MAXLEVEL)
E struct linfo level_info[MAXLINFO];

E NEARDATA struct sinfo {
	int gameover;		/* self explanatory? */
	int stopprint;		/* inhibit further end of game disclosure */
#if defined(UNIX) || defined(VMS) || defined (__EMX__) || defined(WIN32)
	int done_hup;		/* SIGHUP or moral equivalent received
				 * -- no more screen output */
#endif
	int something_worth_saving;	/* in case of panic */
	int panicking;		/* `panic' is in progress */
#if defined(VMS) || defined(WIN32)
	int exiting;		/* an exit handler is executing */
#endif
	int in_impossible;
#ifdef PANICLOG
	int in_paniclog;
#endif
} program_state;

E boolean restoring;

E const char quitchars[];
E const char vowels[];
E const char ynchars[];
E const char ynqchars[];
E const char ynaqchars[];
E const char ynNaqchars[];
E NEARDATA long yn_number;

E const char disclosure_options[];

E NEARDATA int smeq[];
E NEARDATA int doorindex;
E NEARDATA char *save_cm;
#define KILLED_BY_AN	 0
#define KILLED_BY	 1
#define NO_KILLER_PREFIX 2
E NEARDATA int killer_format;
E const char *killer;
E const char *delayed_killer;
#ifdef GOLDOBJ
E long done_money;
#endif
E char killer_buf[BUFSZ];
E const char *configfile;
E NEARDATA char plname[PL_NSIZ];
E NEARDATA char dogname[];
E NEARDATA char catname[];
E NEARDATA char horsename[];
E NEARDATA char lizardname[];
E NEARDATA char spidername[];
E NEARDATA char dragonname[];
E NEARDATA char parrotname[];
E NEARDATA char monkeyname[];
#ifdef CONVICT
E NEARDATA char ratname[];
#endif /* CONVICT */
E char preferred_pet;
E const char *occtxt;			/* defined when occupation != NULL */
E const char *nomovemsg;
E const char nul[];
E char lock[];

E const char sdir[], ndir[];
E const schar xdir[], ydir[], zdir[];

E NEARDATA schar tbx, tby;		/* set in mthrowu.c */

E NEARDATA struct multishot { int n, i; short o; boolean s; } m_shot;

E NEARDATA struct dig_info {		/* apply.c, hack.c */
	int	effort;
	d_level level;
	coord	pos;
	long lastdigtime;
	boolean down, chew, warned, quiet;
} digging;

E NEARDATA long moves, monstermoves;
E NEARDATA long wailmsg;

E NEARDATA boolean in_mklev;
E NEARDATA boolean stoned;
E NEARDATA boolean unweapon;
E NEARDATA boolean mrg_to_wielded;
E NEARDATA struct obj *current_wand;

E NEARDATA boolean in_steed_dismounting;

E const int shield_static[];

#include "spell.h"
E NEARDATA struct spell spl_book[];	/* sized in decl.c */

#include "color.h"
#ifdef TEXTCOLOR
E const int zapcolors[];
#endif

E const char def_oc_syms[MAXOCLASSES];	/* default class symbols */
E uchar oc_syms[MAXOCLASSES];		/* current class symbols */
E const char def_monsyms[MAXMCLASSES];	/* default class symbols */
E uchar monsyms[MAXMCLASSES];		/* current class symbols */

#include "obj.h"
E NEARDATA struct obj *magic_chest_objs[10];
E NEARDATA struct obj *invent,
	*uarm, *uarmc, *uarmh, *uarms, *uarmg, *uarmf,
#ifdef TOURIST
	*uarmu,				/* under-wear, so to speak */
#endif
	*uskin, *uamul, *uleft, *uright, *ublindf,
	*uwep, *uswapwep, *uquiver;

E NEARDATA struct obj *urope;		/* defined only when entangled */
E NEARDATA struct obj *uchain;		/* defined only when punished */
E NEARDATA struct obj *uball;
E NEARDATA struct obj *migrating_objs;
E NEARDATA struct obj *billobjs;
E NEARDATA struct obj zeroobj;		/* init'd and defined in decl.c */

#include "you.h"
E NEARDATA struct you u;

#include "onames.h"
#ifndef PM_H		/* (pm.h has already been included via youprop.h) */
#include "pm.h"
#endif

E NEARDATA struct monst youmonst;	/* init'd and defined in decl.c */
E NEARDATA struct monst *mydogs, *migrating_mons;

E NEARDATA struct permonst upermonst;	/* init'd in decl.c, 
					 * defined in polyself.c 
					 */

E NEARDATA struct mvitals {
	uchar	born;
	uchar	died;
	uchar	mvflags;
	Bitfield(seen,1);
} mvitals[NUMMONS];

E NEARDATA struct c_color_names {
    const char	*const c_black, *const c_amber, *const c_golden,
		*const c_light_blue,*const c_red, *const c_green,
		*const c_silver, *const c_blue, *const c_purple,
		*const c_white;
} c_color_names;
#define NH_BLACK		c_color_names.c_black
#define NH_AMBER		c_color_names.c_amber
#define NH_GOLDEN		c_color_names.c_golden
#define NH_LIGHT_BLUE		c_color_names.c_light_blue
#define NH_RED			c_color_names.c_red
#define NH_GREEN		c_color_names.c_green
#define NH_SILVER		c_color_names.c_silver
#define NH_BLUE			c_color_names.c_blue
#define NH_PURPLE		c_color_names.c_purple
#define NH_WHITE		c_color_names.c_white

/* The names of the colors used for gems, etc. */
E const char *c_obj_colors[];

E struct c_common_strings {
    const char	*const c_nothing_happens, *const c_thats_enough_tries,
		*const c_silly_thing_to, *const c_shudder_for_moment,
		*const c_something, *const c_Something,
		*const c_You_can_move_again,
		*const c_Never_mind, *c_vision_clears,
		*const c_the_your[2];
} c_common_strings;
#define nothing_happens    c_common_strings.c_nothing_happens
#define thats_enough_tries c_common_strings.c_thats_enough_tries
#define silly_thing_to	   c_common_strings.c_silly_thing_to
#define shudder_for_moment c_common_strings.c_shudder_for_moment
#define something	   c_common_strings.c_something
#define Something	   c_common_strings.c_Something
#define You_can_move_again c_common_strings.c_You_can_move_again
#define Never_mind	   c_common_strings.c_Never_mind
#define vision_clears	   c_common_strings.c_vision_clears
#define the_your	   c_common_strings.c_the_your

/* material strings */
E const char *materialnm[];

/* Monster name articles */
#define ARTICLE_NONE	0
#define ARTICLE_THE	1
#define ARTICLE_A	2
#define ARTICLE_YOUR	3

/* Monster name suppress masks */
#define SUPPRESS_IT		0x01
#define SUPPRESS_INVISIBLE	0x02
#define SUPPRESS_HALLUCINATION  0x04
#define SUPPRESS_SADDLE		0x08
#define EXACT_NAME		0x0F

/* Vision */
E NEARDATA boolean vision_full_recalc;	/* TRUE if need vision recalc */
E NEARDATA char **viz_array;		/* could see/in sight row pointers */

/* Window system stuff */
E NEARDATA winid WIN_MESSAGE, WIN_STATUS;
E NEARDATA winid WIN_MAP, WIN_INVEN;

/* pline (et al) for a single string argument (suppress compiler warning) */
#define pline1(cstr) pline("%s", cstr)
#define Your1(cstr) Your("%s", cstr)
#define You1(cstr) You("%s", cstr)
#define verbalize1(cstr) verbalize("%s", cstr)
#define You_hear1(cstr) You_hear("%s", cstr)
#define Sprintf1(buf, cstr) Sprintf(buf, "%s", cstr)
#define panic1(cstr) panic("%s", cstr)

E char toplines[];
#ifndef TCAP_H
E struct tc_gbl_data {	/* also declared in tcap.h */
    char *tc_AS, *tc_AE;	/* graphics start and end (tty font swapping) */
    int   tc_LI,  tc_CO;	/* lines and columns */
} tc_gbl_data;
#define AS tc_gbl_data.tc_AS
#define AE tc_gbl_data.tc_AE
#define LI tc_gbl_data.tc_LI
#define CO tc_gbl_data.tc_CO
#endif

/* xxxexplain[] is in drawing.c */
E const char * const monexplain[], invisexplain[], * const objexplain[], * const oclass_names[];

/* Some systems want to use full pathnames for some subsets of file names,
 * rather than assuming that they're all in the current directory.  This
 * provides all the subclasses that seem reasonable, and sets up for all
 * prefixes being null.  Port code can set those that it wants.
 */
#define HACKPREFIX	0
#define LEVELPREFIX	1
#define SAVEPREFIX	2
#define BONESPREFIX	3
#define DATAPREFIX	4	/* this one must match hardcoded value in dlb.c */
#define SCOREPREFIX	5
#define LOCKPREFIX	6
#define CONFIGPREFIX	7
#define TROUBLEPREFIX	8
#define PREFIX_COUNT	9
/* used in files.c; xxconf.h can override if needed */
# ifndef FQN_MAX_FILENAME
#define FQN_MAX_FILENAME 512
# endif

#if defined(NOCWD_ASSUMPTIONS) || defined(VAR_PLAYGROUND)
/* the bare-bones stuff is unconditional above to simplify coding; for
 * ports that actually use prefixes, add some more localized things
 */
#define PREFIXES_IN_USE
#endif

E char *fqn_prefix[PREFIX_COUNT];
#ifdef PREFIXES_IN_USE
E char *fqn_prefix_names[PREFIX_COUNT];
#endif

#ifdef AUTOPICKUP_EXCEPTIONS
struct autopickup_exception {
	char *pattern;
	boolean grab;
	struct autopickup_exception *next;
};
#endif /* AUTOPICKUP_EXCEPTIONS */

#ifdef RECORD_ACHIEVE
struct u_achieve {
        Bitfield(get_keys,9);        /* the alignment keys */
        Bitfield(get_bell,1);        /* You have obtained the bell of 
                                      * opening */
        Bitfield(get_skey,1);        /* You have obtained the silver key */
        Bitfield(get_candelabrum,1); /* You have obtained the candelabrum */
        Bitfield(get_book,1);        /* You have obtained the book of 
                                      * the dead */
        Bitfield(enter_gehennom,1);  /* Entered Gehennom (including the 
                                      * Valley) by any means */
        Bitfield(perform_invocation,1); /* You have performed the invocation
                                         * ritual */
        Bitfield(get_amulet,1);      /* You have obtained the amulet
                                      * of Yendor */
        Bitfield(ascended,1);        /* You ascended to demigod[dess]hood.
                                      * Not quite the same as 
                                      * u.uevent.ascended. */
        Bitfield(get_luckstone,1);   /* You obtained the luckstone at the
                                      * end of the mines. */
        Bitfield(finish_sokoban,1);  /* You obtained the sokoban prize. */
        Bitfield(killed_medusa,1);   /* You defeated Medusa. */
		Bitfield(killed_lucifer,1);		/* Bragging rights */
		Bitfield(killed_asmodeus,1);		/* Bragging rights */
		Bitfield(killed_demogorgon,1);		/* Bragging rights */
};

E struct u_achieve achieve;
#endif

#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
E struct realtime_data {
  time_t realtime;    /* Amount of actual playing time up until the last time
                       * the game was restored. */
  time_t restoretime; /* The time that the game was started or restored. */
  time_t last_displayed_time; /* Last time displayed on the status line */
} realtime_data;
#endif /* RECORD_REALTIME || REALTIME_ON_BOTL */

#undef E

#endif /* DECL_H */
