/*	SCCS Id: @(#)flag.h	3.4	2002/08/22	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* If you change the flag structure make sure you increment EDITLEVEL in   */
/* patchlevel.h if needed.  Changing the instance_flags structure does	   */
/* not require incrementing EDITLEVEL.					   */

#ifndef FLAG_H
#define FLAG_H

/*
 * Persistent flags that are saved and restored with the game.
 *
 */

struct flag {
#ifdef AMIFLUSH
	boolean  altmeta;	/* use ALT keys as META */
	boolean  amiflush;	/* kill typeahead */
#endif
#ifdef	MFLOPPY
	boolean  asksavedisk;
#endif
	boolean  autodig;       /* MRKR: Automatically dig */
	boolean  autoquiver;	/* Automatically fill quiver */
	boolean  beginner;
#ifdef MAIL
	boolean  biff;		/* enable checking for mail */
#endif
	boolean  botl;		/* partially redo status line */
	boolean  botlx;		/* print an entirely new bottom line */
	boolean  confirm;	/* confirm before hitting tame monsters */
	boolean  debug;		/* in debugging mode */
#define wizard	 flags.debug
	boolean  end_own;	/* list all own scores */
	boolean  explore;	/* in exploration mode */
#ifdef OPT_DISPMAP
	boolean  fast_map;	/* use optimized, less flexible map display */
#endif
#define discover flags.explore
	boolean  female;
	boolean  forcefight;
	boolean  friday13;	/* it's Friday the 13th */
	boolean  help;		/* look in data file for info about stuff */
	boolean  ignintr;	/* ignore interrupts */
#ifdef INSURANCE
	boolean  ins_chkpt;	/* checkpoint as appropriate */
#endif
	boolean  invlet_constant; /* let objects keep their inventory symbol */
	boolean  legacy;	/* print game entry "story" */
	boolean  lit_corridor;	/* show a dark corr as lit if it is in sight */
	boolean  made_amulet;
	boolean  mon_moving;	/* monsters' turn to move */
	boolean  cth_attk;		/* for use with stinking cloud, mark when 
							 'thulhu makes a cloud sos the comp knows he did it */
	boolean  drgn_brth;		/* for use with breath weapons, indicates that a dragon is breathing */
	boolean  mamn_brth;		/* for use with breath weapons, indicates that mammon is breathing */
	boolean  move;
	boolean  mv;
	boolean  bypasses;	/* bypass flag is set on at least one fobj */
	boolean  nap;		/* `timed_delay' option for display effects */
	boolean  nopick;	/* do not pickup objects (as when running) */
	boolean  null;		/* OK to send nulls to the terminal */
#ifdef MAC
	boolean  page_wait;	/* put up a --More-- after a page of messages */
#endif
	boolean  perm_invent;	/* keep full inventories up until dismissed */
	boolean  pickup;	/* whether you pickup or move and look */

	boolean  pushweapon;	/* When wielding, push old weapon into second slot */
	boolean  rest_on_space; /* space means rest */
	boolean  safe_dog;	/* give complete protection to the dog */
#ifdef EXP_ON_BOTL
	boolean  showexp;	/* show experience points */
#endif
#ifdef SCORE_ON_BOTL
	boolean  showscore;	/* show score */
#endif
	boolean  silent;	/* whether the bell rings or not */
	boolean  sortpack;	/* sorted inventory */
	boolean  soundok;	/* ok to tell about sounds heard */
	boolean  sparkle;	/* show "resisting" special FX (Scott Bigham) */
	boolean  standout;	/* use standout for --More-- */
	boolean  time;		/* display elapsed 'time' */
	boolean  timeoutOrder;	/* display spirits and powers in order of timeout */
	boolean  tombstone;	/* print tombstone */
	boolean  toptenwin;	/* ending list in window instead of stdout */
	boolean  verbose;	/* max battle info */
	boolean  prayconfirm;	/* confirm before praying */
	int	 end_top, end_around;	/* describe desired score list */
	unsigned ident;		/* social security number for each monster */
	unsigned moonphase;
	unsigned long suppress_alert;
#define NEW_MOON	0
#define FULL_MOON	4
	unsigned no_of_wizards; /* 0, 1 or 2 (wizard and his shadow) */
	boolean  travel;	/* find way automatically to u.tx,u.ty */
	unsigned run;		/* 0: h (etc), 1: H (etc), 2: fh (etc) */
				/* 3: FH, 4: ff+, 5: ff-, 6: FF+, 7: FF- */
				/* 8: travel */
	unsigned long warntypem; /* warn_of_mon monster type MM */
	unsigned long warntypet; /* warn_of_mon monster type MT */
	unsigned long warntypeb; /* warn_of_mon monster type MB */
	unsigned long warntypeg; /* warn_of_mon monster type MG */
	unsigned long warntypea; /* warn_of_mon monster type MA */
	unsigned long warntypev; /* warn_of_mon monster type MV */
	unsigned long long montype; /* warn_of_mon monster type bitshifted S_ */
	int	 warnlevel;
	int	 djinni_count, ghost_count;	/* potion effect tuning */
	int	 pickup_burden;		/* maximum burden before prompt */
	char	 inv_order[MAXOCLASSES];
	char	 pickup_types[MAXOCLASSES];
#define NUM_DISCLOSURE_OPTIONS		5
#define DISCLOSE_PROMPT_DEFAULT_YES	'y'
#define DISCLOSE_PROMPT_DEFAULT_NO	'n'
#define DISCLOSE_YES_WITHOUT_PROMPT	'+'
#define DISCLOSE_NO_WITHOUT_PROMPT	'-'
	char	 end_disclose[NUM_DISCLOSURE_OPTIONS + 1];  /* disclose various info
								upon exit */
	char	 menu_style;	/* User interface style setting */
#ifdef AMII_GRAPHICS
	int numcols;
	unsigned short amii_dripens[ 20 ]; /* DrawInfo Pens currently there are 13 in v39 */
	AMII_COLOR_TYPE amii_curmap[ AMII_MAXCOLORS ]; /* colormap */
#endif

	int	 questvar;	/* quest variant */
#define	questprogress	questvar
	boolean	 stag;	/* turned stag during the quest, re-used to track if the Anachrononaut has completed their extra task */
	boolean leader_backstab;		/* your leader is attacking you */
	boolean made_bell;		/* the bell of opening has been created */

	boolean spore_level;		/* the current level has a spore-spreading monster */
	boolean slime_level;		/* the current level has a slime-spreading monster */
	boolean walky_level;		/* the current level has a undead-raising monster */
	boolean shade_level;		/* the current level has a shade-casting monster */
	
	/* KMH, role patch -- Variables used during startup.
	 *
	 * If the user wishes to select a role, race, gender, and/or alignment
	 * during startup, the choices should be recorded here.  This
	 * might be specified through command-line options, environmental
	 * variables, a popup dialog box, menus, etc.
	 *
	 * These values are each an index into an array.  They are not
	 * characters or letters, because that limits us to 26 roles.
	 * They are not booleans, because someday someone may need a neuter
	 * gender.  Negative values are used to indicate that the user
	 * hasn't yet specified that particular value.	If you determine
	 * that the user wants a random choice, then you should set an
	 * appropriate random value; if you just left the negative value,
	 * the user would be asked again!
	 *
	 * These variables are stored here because the u structure is
	 * cleared during character initialization, and because the
	 * flags structure is restored for saved games.  Thus, we can
	 * use the same parameters to build the role entry for both
	 * new and restored games.
	 *
	 * These variables should not be referred to after the character
	 * is initialized or restored (specifically, after role_init()
	 * is called).
	 */
	int	 initrole;	/* starting role      (index into roles[])   */
	int	 initrace;	/* starting race      (index into races[])   */
	int	 initgend;	/* starting gender    (index into genders[]) */
	int	 initalign;	/* starting alignment (index into aligns[])  */
	int	 randomall;	/* randomly assign everything not specified */
	int	 pantheon;	/* deity selection for priest character */
	int	 racial_pantheon;	/* racial deity selection */
	int	 panLgod;	/* deity selection for binder character */
	int	 panNgod;	/* deity selection for binder character */
	int	 panCgod;	/* deity selection for binder character */
	int  HDbreath;	/* half-dragon breath weapon type*/
};

/*
 * Flags that are set each time the game is started.
 * These are not saved with the game.
 *
 */

struct instance_flags {
	boolean  cbreak;	/* in cbreak mode, rogue format */
	boolean  DECgraphics;	/* use DEC VT-xxx extended character set */
	boolean  echo;		/* 1 to echo characters */
	boolean  IBMgraphics;	/* use IBM extended character set */
	unsigned msg_history;	/* hint: # of top lines to save */
	boolean  num_pad;	/* use numbers for movement commands */
	boolean  news;		/* print news */
	boolean  window_inited; /* true if init_nhwindows() completed */
	boolean  vision_inited; /* true if vision is ready */
	boolean  menu_tab_sep;	/* Use tabs to separate option menu fields */
	boolean  menu_requested; /* Flag for overloaded use of 'm' prefix
				  * on some non-move commands */
	uchar num_pad_mode;
	int	menu_headings;	/* ATR for menu headings */
	int      purge_monsters;	/* # of dead monsters still on fmon list */
	int *opt_booldup;	/* for duplication of boolean opts in config file */
	int *opt_compdup;	/* for duplication of compound opts in config file */
	uchar	bouldersym;	/* symbol for boulder display */
	boolean travel1;	/* first travel step */
	coord	travelcc;	/* coordinates for travel_cache */
	boolean  door_opened;	/* set to true if door was opened during test_move */
#ifdef WIZARD
	boolean  sanity_check;	/* run sanity checks */
	boolean  mon_polycontrol;	/* debug: control monster polymorphs */
#endif
#ifdef TTY_GRAPHICS
	char prevmsg_window;	/* type of old message window to use */
	boolean  extmenu;	/* extended commands use menu interface */
#endif
#ifdef MFLOPPY
	boolean  checkspace;	/* check disk space before writing files */
				/* (in iflags to allow restore after moving
				 * to >2GB partition) */
#endif
#ifdef MICRO
	boolean  BIOS;		/* use IBM or ST BIOS calls when appropriate */
#endif
#if defined(MICRO) || defined(WIN32)
	boolean  rawio;		/* whether can use rawio (IOCTL call) */
#endif
#ifdef MAC_GRAPHICS_ENV
	boolean  MACgraphics;	/* use Macintosh extended character set, as
				   as defined in the special font HackFont */
	unsigned  use_stone;		/* use the stone ppats */
#endif
#if defined(MSDOS) || defined(WIN32)
	boolean hassound;	/* has a sound card */
	boolean usesound;	/* use the sound card */
	boolean usepcspeaker;	/* use the pc speaker */
	boolean tile_view;
	boolean over_view;
	boolean traditional_view;
#endif
#ifdef MSDOS
	boolean hasvga;		/* has a vga adapter */
	boolean usevga;		/* use the vga adapter */
	boolean grmode;		/* currently in graphics mode */
#endif
#ifdef LAN_FEATURES
	boolean lan_mail;	/* mail is initialized */
	boolean lan_mail_fetched; /* mail is awaiting display */
#endif
	boolean  autoopen;	/* open doors by walking into them */
	boolean  quiver_fired;
       boolean  pickup_thrown;
    boolean role_obj_names;
    boolean obscure_role_obj_names;
    boolean dnethack_start_text;
    boolean dnethack_dungeon_colors;
/*
 * Window capability support.
 */
	boolean wc_color;		/* use color graphics                  */
	boolean wc_hilite_pet;		/* hilight pets (blue)                    */
	boolean wc_hilite_peaceful;		/* hilight peaceful monsters (brown)   */
	boolean wc_hilite_zombies;		/* hilight pets  (green)               */
	boolean wc_zombie_z;		/* show zombies as Z of monster's color    */
	boolean wc_ascii_map;		/* show map using traditional ascii    */
	boolean wc_tiled_map;		/* show map using tiles                */
	boolean wc_preload_tiles;	/* preload tiles into memory           */
	int	wc_tile_width;		/* tile width                          */
	int	wc_tile_height;		/* tile height                         */
	char	*wc_tile_file;		/* name of tile file;overrides default */
	boolean wc_inverse;		/* use inverse video for some things   */
	int	wc_align_status;	/*  status win at top|bot|right|left   */
	int	wc_align_message;	/* message win at top|bot|right|left   */
	int     wc_vary_msgcount;	/* show more old messages at a time    */
	char    *wc_foregrnd_menu;	/* points to foregrnd color name for menu win   */
	char    *wc_backgrnd_menu;	/* points to backgrnd color name for menu win   */
	char    *wc_foregrnd_message;	/* points to foregrnd color name for msg win    */
	char    *wc_backgrnd_message;	/* points to backgrnd color name for msg win    */
	char    *wc_foregrnd_status;	/* points to foregrnd color name for status win */
	char    *wc_backgrnd_status;	/* points to backgrnd color name for status win */
	char    *wc_foregrnd_text;	/* points to foregrnd color name for text win   */
	char    *wc_backgrnd_text;	/* points to backgrnd color name for text win   */
	char    *wc_font_map;		/* points to font name for the map win */
	char    *wc_font_message;	/* points to font name for message win */
	char    *wc_font_status;	/* points to font name for status win  */
	char    *wc_font_menu;		/* points to font name for menu win    */
	char    *wc_font_text;		/* points to font name for text win    */
	int     wc_fontsiz_map;		/* font size for the map win           */
	int     wc_fontsiz_message;	/* font size for the message window    */
	int     wc_fontsiz_status;	/* font size for the status window     */
	int     wc_fontsiz_menu;	/* font size for the menu window       */
	int     wc_fontsiz_text;	/* font size for text windows          */
	int	wc_scroll_amount;	/* scroll this amount at scroll_margin */
	int	wc_scroll_margin;	/* scroll map when this far from
						the edge */
	int	wc_map_mode;		/* specify map viewing options, mostly
						for backward compatibility */
	int	wc_player_selection;	/* method of choosing character */
	boolean	wc_splash_screen;	/* display an opening splash screen or not */
	boolean	wc_popup_dialog;	/* put queries in pop up dialogs instead of
				   		in the message window */
	boolean wc_eight_bit_input;	/* allow eight bit input               */
	boolean wc_mouse_support;	/* allow mouse support */
	boolean wc2_fullscreen;		/* run fullscreen */
	boolean wc2_softkeyboard;	/* use software keyboard */
	boolean wc2_wraptext;		/* wrap text */

	boolean  cmdassist;	/* provide detailed assistance for some commands */
	boolean	 obsolete;	/* obsolete options can point at this, it isn't used */
	/* Items which belong in flags, but are here to allow save compatibility */
	boolean  lootabc;	/* use "a/b/c" rather than "o/i/b" when looting */
	boolean  showrace;	/* show hero glyph by race rather than by role */
	boolean  travelcmd;	/* allow travel command */
	int	 runmode;	/* update screen display during run moves */
#ifdef AUTOPICKUP_EXCEPTIONS
	struct autopickup_exception *autopickup_exceptions[2];
#define AP_LEAVE 0
#define AP_GRAB	 1
#endif
#ifdef WIN32CON
#define MAX_ALTKEYHANDLER 25
	char	 altkeyhandler[MAX_ALTKEYHANDLER];
#endif
#ifdef REALTIME_ON_BOTL
  boolean  showrealtime; /* show actual elapsed time */
#endif
};

/*
 * Old deprecated names
 */
#ifdef TTY_GRAPHICS
#define eight_bit_tty wc_eight_bit_input
#endif
#ifdef TEXTCOLOR
#define use_color wc_color
#endif
#define hilite_pet wc_hilite_pet
#define hilite_peaceful wc_hilite_peaceful
#define hilite_zombies wc_hilite_zombies
#define zombie_z wc_zombie_z
#define use_inverse wc_inverse
#ifdef MAC_GRAPHICS_ENV
#define large_font obsolete
#endif
#ifdef MAC
#define popup_dialog wc_popup_dialog
#endif
#define preload_tiles wc_preload_tiles

extern NEARDATA struct flag flags;
extern NEARDATA struct instance_flags iflags;

/* runmode options */
#define RUN_TPORT	0	/* don't update display until movement stops */
#define RUN_LEAP	1	/* update display every 7 steps */
#define RUN_STEP	2	/* update display every single step */
#define RUN_CRAWL	3	/* walk w/ extra delay after each update */

#endif /* FLAG_H */
