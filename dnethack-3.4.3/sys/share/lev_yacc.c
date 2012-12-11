#ifndef lint
static char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif
#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#define YYRECOVERING (yyerrflag!=0)
#define YYPREFIX "yy"
#line 2 "lev_comp.y"
/*	SCCS Id: @(#)lev_yacc.c	3.4	2000/01/17	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the Level Compiler code
 * It may handle special mazes & special room-levels
 */

/* In case we're using bison in AIX.  This definition must be
 * placed before any other C-language construct in the file
 * excluding comments and preprocessor directives (thanks IBM
 * for this wonderful feature...).
 *
 * Note: some cpps barf on this 'undefined control' (#pragma).
 * Addition of the leading space seems to prevent barfage for now,
 * and AIX will still see the directive.
 */
#ifdef _AIX
 #pragma alloca		/* keep leading space! */
#endif

#include "hack.h"
#include "sp_lev.h"

#define MAX_REGISTERS	10
#define ERR		(-1)
/* many types of things are put in chars for transference to NetHack.
 * since some systems will use signed chars, limit everybody to the
 * same number for portability.
 */
#define MAX_OF_TYPE	128

#define New(type)		\
	(type *) memset((genericptr_t)alloc(sizeof(type)), 0, sizeof(type))
#define NewTab(type, size)	(type **) alloc(sizeof(type *) * size)
#define Free(ptr)		free((genericptr_t)ptr)

extern void FDECL(yyerror, (const char *));
extern void FDECL(yywarning, (const char *));
extern int NDECL(yylex);
int NDECL(yyparse);

extern int FDECL(get_floor_type, (CHAR_P));
extern int FDECL(get_room_type, (char *));
extern int FDECL(get_trap_type, (char *));
extern int FDECL(get_monster_id, (char *,CHAR_P));
extern int FDECL(get_object_id, (char *,CHAR_P));
extern boolean FDECL(check_monster_char, (CHAR_P));
extern boolean FDECL(check_object_char, (CHAR_P));
extern char FDECL(what_map_char, (CHAR_P));
extern void FDECL(scan_map, (char *));
extern void NDECL(wallify_map);
extern boolean NDECL(check_subrooms);
extern void FDECL(check_coord, (int,int,const char *));
extern void NDECL(store_part);
extern void NDECL(store_room);
extern boolean FDECL(write_level_file, (char *,splev *,specialmaze *));
extern void FDECL(free_rooms, (splev *));

static struct reg {
	int x1, y1;
	int x2, y2;
}		current_region;

static struct coord {
	int x;
	int y;
}		current_coord, current_align;

static struct size {
	int height;
	int width;
}		current_size;

char tmpmessage[256];
digpos *tmppass[32];
char *tmpmap[ROWNO];

digpos *tmpdig[MAX_OF_TYPE];
region *tmpreg[MAX_OF_TYPE];
lev_region *tmplreg[MAX_OF_TYPE];
door *tmpdoor[MAX_OF_TYPE];
drawbridge *tmpdb[MAX_OF_TYPE];
walk *tmpwalk[MAX_OF_TYPE];

room_door *tmprdoor[MAX_OF_TYPE];
trap *tmptrap[MAX_OF_TYPE];
monster *tmpmonst[MAX_OF_TYPE];
object *tmpobj[MAX_OF_TYPE];
altar *tmpaltar[MAX_OF_TYPE];
lad *tmplad[MAX_OF_TYPE];
stair *tmpstair[MAX_OF_TYPE];
gold *tmpgold[MAX_OF_TYPE];
engraving *tmpengraving[MAX_OF_TYPE];
fountain *tmpfountain[MAX_OF_TYPE];
sink *tmpsink[MAX_OF_TYPE];
pool *tmppool[MAX_OF_TYPE];

mazepart *tmppart[10];
room *tmproom[MAXNROFROOMS*2];
corridor *tmpcor[MAX_OF_TYPE];

static specialmaze maze;
static splev special_lev;
static lev_init init_lev;

static char olist[MAX_REGISTERS], mlist[MAX_REGISTERS];
static struct coord plist[MAX_REGISTERS];

int n_olist = 0, n_mlist = 0, n_plist = 0;

unsigned int nlreg = 0, nreg = 0, ndoor = 0, ntrap = 0, nmons = 0, nobj = 0;
unsigned int ndb = 0, nwalk = 0, npart = 0, ndig = 0, nlad = 0, nstair = 0;
unsigned int naltar = 0, ncorridor = 0, nrooms = 0, ngold = 0, nengraving = 0;
unsigned int nfountain = 0, npool = 0, nsink = 0, npass = 0;

static int lev_flags = 0;

unsigned int max_x_map, max_y_map;

static xchar in_room;

extern int fatal_error;
extern int want_warnings;
extern const char *fname;

#line 131 "lev_comp.y"
typedef union
{
	int	i;
	char*	map;
	struct {
		xchar room;
		xchar wall;
		xchar door;
	} corpos;
} YYSTYPE;
#line 151 "lev_comp.tab.c"
#define CHAR 257
#define INTEGER 258
#define BOOLEAN 259
#define PERCENT 260
#define MESSAGE_ID 261
#define MAZE_ID 262
#define LEVEL_ID 263
#define LEV_INIT_ID 264
#define GEOMETRY_ID 265
#define NOMAP_ID 266
#define OBJECT_ID 267
#define COBJECT_ID 268
#define MONSTER_ID 269
#define TRAP_ID 270
#define DOOR_ID 271
#define DRAWBRIDGE_ID 272
#define MAZEWALK_ID 273
#define WALLIFY_ID 274
#define REGION_ID 275
#define FILLING 276
#define RANDOM_OBJECTS_ID 277
#define RANDOM_MONSTERS_ID 278
#define RANDOM_PLACES_ID 279
#define ALTAR_ID 280
#define LADDER_ID 281
#define STAIR_ID 282
#define NON_DIGGABLE_ID 283
#define NON_PASSWALL_ID 284
#define ROOM_ID 285
#define PORTAL_ID 286
#define TELEPRT_ID 287
#define BRANCH_ID 288
#define LEV 289
#define CHANCE_ID 290
#define CORRIDOR_ID 291
#define GOLD_ID 292
#define ENGRAVING_ID 293
#define FOUNTAIN_ID 294
#define POOL_ID 295
#define SINK_ID 296
#define NONE 297
#define RAND_CORRIDOR_ID 298
#define DOOR_STATE 299
#define LIGHT_STATE 300
#define CURSE_TYPE 301
#define ENGRAVING_TYPE 302
#define DIRECTION 303
#define RANDOM_TYPE 304
#define O_REGISTER 305
#define M_REGISTER 306
#define P_REGISTER 307
#define A_REGISTER 308
#define ALIGNMENT 309
#define LEFT_OR_RIGHT 310
#define CENTER 311
#define TOP_OR_BOT 312
#define ALTAR_TYPE 313
#define UP_OR_DOWN 314
#define SUBROOM_ID 315
#define NAME_ID 316
#define FLAGS_ID 317
#define FLAG_TYPE 318
#define MON_ATTITUDE 319
#define MON_ALERTNESS 320
#define MON_APPEARANCE 321
#define CONTAINED 322
#define STRING 323
#define MAP_ID 324
#define YYERRCODE 256
short yylhs[] = {                                        -1,
    0,    0,   36,   36,   37,   37,   38,   39,   32,   23,
   23,   14,   14,   19,   19,   20,   20,   40,   40,   45,
   42,   42,   46,   46,   43,   43,   49,   49,   44,   44,
   51,   52,   52,   53,   53,   35,   50,   50,   56,   54,
   10,   10,   59,   59,   57,   57,   60,   60,   58,   58,
   55,   55,   61,   61,   61,   61,   61,   61,   61,   61,
   61,   61,   61,   61,   61,   62,   63,   76,   64,   75,
   75,   77,   15,   15,   13,   13,   12,   12,   31,   11,
   11,   41,   41,   78,   79,   79,   82,    1,    1,    2,
    2,   80,   80,   83,   83,   83,   47,   47,   48,   48,
   84,   86,   84,   81,   81,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,   87,   87,   87,   87,   87,
   87,   87,   87,   87,   87,  102,   65,  101,  101,  103,
  103,  103,  103,  103,   66,   66,  105,  104,  106,  106,
  107,  107,  107,  107,  108,  108,  109,  110,  110,  111,
  111,  111,  113,   88,  112,  112,  114,   67,   89,   95,
   96,   97,   74,  115,   91,  116,   92,  117,  119,   93,
  120,   94,  118,  118,   22,   22,   69,   70,   71,   98,
   99,   90,   68,   72,   73,   25,   25,   25,   28,   28,
   28,   33,   33,   34,   34,    3,    3,    4,    4,   21,
   21,   21,  100,  100,  100,    5,    5,    6,    6,    7,
    7,    7,    8,    8,  123,   29,   26,    9,   85,   24,
   27,   30,   16,   16,   17,   17,   18,   18,  122,  121,
};
short yylen[] = {                                         2,
    0,    1,    1,    2,    1,    1,    5,    7,    3,    0,
   13,    1,    1,    0,    3,    3,    1,    0,    2,    3,
    0,    2,    3,    3,    0,    1,    1,    2,    1,    1,
    1,    0,    2,    5,    5,    7,    2,    2,   12,   12,
    0,    2,    5,    1,    5,    1,    5,    1,    5,    1,
    0,    2,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    3,    3,    0,   11,    0,
    2,    2,    1,    1,    1,    1,    1,    1,    5,    1,
    1,    1,    2,    3,    1,    2,    5,    1,    1,    1,
    1,    0,    2,    3,    3,    3,    1,    3,    1,    3,
    1,    0,    4,    0,    2,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    0,   10,    0,    2,    2,
    2,    2,    2,    3,    2,    2,    0,    9,    1,    1,
    0,    7,    5,    5,    1,    1,    1,    1,    1,    0,
    2,    2,    0,    7,    0,    2,    2,    6,    7,    5,
    1,    5,    5,    0,    8,    0,    8,    0,    0,    8,
    0,    6,    0,    2,    1,   10,    3,    3,    3,    3,
    3,    8,    7,    5,    7,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    0,
    2,    4,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    4,    4,    4,    4,    1,    1,
    1,    1,    1,    1,    0,    1,    1,    1,    5,    9,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    2,    0,    5,    6,    0,
    0,    0,    0,    0,    4,  222,    0,    9,    0,    0,
    0,    0,    0,    0,   15,    0,    0,    0,    0,   21,
   80,   81,   79,    0,    0,    0,    0,   85,    7,    0,
   92,    0,   19,    0,   16,    0,   20,    0,   83,    0,
   86,    0,    0,    0,    0,    0,   22,   26,    0,   51,
   51,    0,   88,   89,    0,    0,    0,    0,    0,   93,
    0,    0,    0,    0,   31,    8,   29,    0,   28,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  161,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  106,  107,  109,  116,
  117,  122,  123,  121,  105,  108,  110,  111,  112,  113,
  114,  115,  118,  119,  120,  124,  125,  221,    0,   23,
  220,    0,   24,  199,    0,  198,    0,    0,   33,    0,
    0,    0,    0,    0,    0,   52,   53,   54,   55,   56,
   57,   58,   59,   60,   61,   62,   63,   64,   65,    0,
   91,   90,   87,   94,   96,    0,   95,    0,  219,  226,
    0,  135,  136,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  206,  207,    0,
  205,    0,    0,  203,  204,    0,    0,    0,    0,    0,
    0,    0,  164,    0,  175,  180,  181,  166,  168,  171,
  223,  224,    0,    0,  177,   98,  100,  208,  209,    0,
    0,    0,    0,   73,   74,    0,   67,  179,  178,   66,
    0,    0,    0,  190,    0,  189,    0,  191,  187,    0,
  186,    0,  188,  197,    0,  196,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  103,    0,    0,    0,    0,    0,  153,    0,    0,  160,
    0,    0,  212,    0,  210,    0,  211,  162,    0,    0,
    0,  163,    0,    0,    0,  184,  227,  228,    0,   44,
    0,    0,   46,    0,    0,    0,   35,   34,    0,    0,
  229,    0,  195,  194,  137,    0,  193,  192,    0,  158,
  155,  215,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  169,  172,    0,    0,    0,    0,    0,    0,    0,
    0,  216,    0,  217,    0,    0,  159,    0,    0,    0,
  214,  213,  183,    0,    0,    0,    0,  185,    0,   48,
    0,    0,    0,   50,    0,    0,    0,   75,   76,    0,
   12,   13,   11,    0,  126,    0,  156,    0,    0,  182,
  218,    0,  165,  167,    0,  170,    0,    0,    0,    0,
    0,    0,   77,   78,    0,    0,  140,  139,    0,  128,
  157,    0,    0,    0,  174,   43,    0,    0,   45,    0,
    0,   36,   68,    0,  138,    0,    0,    0,    0,    0,
    0,   40,    0,   39,   70,  146,  145,  147,    0,    0,
    0,  129,  230,  202,    0,   47,   42,   49,    0,    0,
    0,  131,  132,    0,  133,  130,  176,    0,   71,  149,
  148,    0,    0,    0,  134,   72,    0,    0,  143,  144,
    0,  151,  152,  142,
};
short yydgoto[] = {                                       3,
   65,  163,  265,  135,  210,  240,  306,  373,  307,  442,
   33,  415,  390,  393,  246,  233,  171,  319,   13,   25,
  400,  223,   21,  132,  262,  263,  129,  257,  258,  136,
    4,    5,  339,  335,  243,    6,    7,    8,    9,   28,
   39,   44,   56,   76,   29,   57,  130,  133,   58,   59,
   77,   78,  139,   60,   80,   61,  325,  386,  322,  382,
  146,  147,  148,  149,  150,  151,  152,  153,  154,  155,
  156,  157,  158,  159,  459,  445,  469,   40,   41,   50,
   69,   42,   70,  167,  168,  204,  115,  116,  117,  118,
  119,  120,  121,  122,  123,  124,  125,  126,  127,  224,
  436,  420,  452,  172,  363,  419,  435,  449,  450,  473,
  479,  366,  341,  397,  277,  279,  280,  406,  377,  281,
  225,  214,  215,
};
short yysindex[] = {                                   -134,
  -24,    2,    0, -251, -251,    0, -134,    0,    0, -242,
 -242,   36, -131, -131,    0,    0,   87,    0, -162,   84,
  -82,  -82, -234,  145,    0,  -67,  134,  -90,  -82,    0,
    0,    0,    0, -162,  149, -129,  137,    0,    0,  -90,
    0, -126,    0, -223,    0,  -58,    0, -133,    0, -124,
    0,  142,  144,  146,  147,  -95,    0,    0, -257,    0,
    0,  162,    0,    0,  163,  150,  151,  152, -123,    0,
  -46,  -45, -260, -260,    0,    0,    0,  -78,    0, -221,
 -221,  -44, -130,  -46,  -45,  174,  -43,  -43,  -43,  -43,
  158,  160,  161,    0,  164,  166,  167,  168,  169,  170,
  171,  173,  175,  176,  178,  179,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  177,    0,
    0,  188,    0,    0,  194,    0,  195,  182,    0,  183,
  184,  185,  186,  187,  189,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  202,
    0,    0,    0,    0,    0,   -9,    0,    0,    0,    0,
  190,    0,    0,  192,  193, -193,   39,   39,  214,   39,
   39,  -37,  214,  214,  -36,  -36,  -36, -227,   39,   39,
  -46,  -45, -248, -248,  215, -237,   39,   -2,   39,   39,
 -242,  -39,  213,  216, -219, -224, -240,    0,    0,  217,
    0,  172,  218,    0,    0,  220,    1,  221,  263,  303,
  309,  105,    0,  355,    0,    0,    0,    0,    0,    0,
    0,    0,  357,  369,    0,    0,    0,    0,    0,  371,
  372,  197,  375,    0,    0,  376,    0,    0,    0,    0,
  383,  203,  174,    0,  334,    0,  384,    0,    0,  338,
    0,  386,    0,    0,  394,    0,   39,  205,  165,  181,
  396, -248, -169,  -56,  206,  400,  401,  132,  408,  409,
  423,   39, -187,  -38,  -20,  425,  -35, -193, -248,  436,
    0,  208, -203,  224, -200,   39,    0,  390,  435,    0,
  227,  442,    0,  397,    0,  445,    0,    0,  446,  233,
  -36,    0,  -36,  -36,  -36,    0,    0,    0,  448,    0,
  241,  449,    0,  272,  487,  229,    0,    0,  490,  491,
    0,  443,    0,    0,    0,  444,    0,    0,  494,    0,
    0,    0, -193,  495, -260,  282, -199,  283,   86,  498,
  499,    0,    0, -242,  500,   -1,  501,   28,  502, -120,
 -214,    0,  503,    0,   39,  504,    0,  291,  507,  459,
    0,    0,    0,  509,  240, -242,  511,    0,  298,    0,
 -133,  527,  326,    0,  327,  546, -217,    0,    0,  547,
    0,    0,    0,   38,    0,  336,    0,  551,  320,    0,
    0,  339,    0,    0,  284,    0,  558,  556,   28,  566,
  564, -242,    0,    0,  568, -217,    0,    0,  571,    0,
    0,  358,  575,  576,    0,    0, -130,  577,    0,  364,
  577,    0,    0, -272,    0,  579,  583,  370,  374,  589,
  377,    0,  593,    0,    0,    0,    0,    0,  595,  596,
 -211,    0,    0,    0,  601,    0,    0,    0,  605, -239,
 -216,    0,    0, -242,    0,    0,    0,  379,    0,    0,
    0,  606,  607,  607,    0,    0, -216, -270,    0,    0,
  607,    0,    0,    0,
};
short yyrindex[] = {                                    633,
    0,    0,    0, -140,  350,    0,  645,    0,    0,    0,
    0,    0,  -99,  391,    0,    0,    0,    0,    0,    0,
  -80,  422,    0,  340,    0,    0,    0,    0,  381,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,   99,
    0,    0,    0,   57,    0,    0,    0,    0,    0,  525,
    0,    0,    0,    0,    0,   89,    0,    0,  116,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   91,    0,
    0,    0,    0,    0,    0,    0,    0,  143,    0,  196,
  424,    0,    0,    0,    0,    0,  598,  598,  598,  598,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  235,    0,
    0,  295,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  480,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  559,    0,    0,  592,    0,
    0,    0,    0,    0,    0,    0,  626,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    6,    0,
    0,    0,  660,    0,    0,    0,    0,  141,    0,    0,
  141,    0,    0,    0,    0,   43,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  180,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  102,  102,    0,    0,    0,    0,    0,    0,
  102,    0,    0,    0,
};
short yygindex[] = {                                      0,
  276,  234,    0,  -63, -267, -176,  211,    0,    0,  232,
    0,  248,    0,    0,    0,    0,   98,    0,  662,  636,
    0, -177,  657,  467,    0,    0,  469,    0,    0,  -10,
    0,    0,    0,    0,  388,  670,    0,    0,    0,    8,
  638,    0,    0,    0,    0,    0,  -72,  -70,  621,    0,
    0,    0,    0,    0,  620,    0,    0,  274,    0,    0,
    0,    0,    0,    0,  619,  622,  623,  624,  625,    0,
    0,  628,  629,  632,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  437,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -164,
    0,    0,    0,  614,    0,    0,    0,    0,  243, -437,
 -374,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -11,  -79,    0,
};
#define YYTABLESIZE 954
short yytable[] = {                                      17,
   18,  321,  222,  217,  242,  141,  169,  228,  229,  230,
  137,  164,  213,  216,  165,  219,  220,  241,  470,  324,
  329,  244,   31,  474,  234,  235,  482,   54,  446,   30,
  231,  447,  131,   10,  248,  249,   43,  128,  381,  481,
  413,  470,  127,  134,  391,   87,   88,   89,   90,  140,
  448,  238,  483,   52,   53,  239,   25,   55,   96,   11,
  141,   54,   16,  264,  471,   12,  245,  385,  142,   32,
  104,  105,  106,  143,  144,  367,  232,  166,  166,  259,
   16,  260,   16,  448,  254,  255,  414,  471,   32,  392,
   84,   55,  303,   19,  145,  302,  304,  305,   82,  480,
  333,  150,  297,  337,  371,  208,  484,  462,  463,  464,
  209,   16,  330,  372,  317,   27,  318,  316,  236,   16,
   14,  237,   16,   14,   14,   14,  331,    1,    2,  368,
   23,  340,   20,  350,  303,  351,  352,  353,  304,  305,
   41,   26,   30,   87,   88,   89,   90,   91,   92,   93,
   94,   95,   66,   67,   68,   24,   96,   97,   98,   99,
  100,   10,  101,  102,  103,   10,   10,  218,  104,  105,
  106,  226,  227,  169,   37,   38,   63,   64,   27,   69,
  161,  162,  388,  389,   18,   18,  174,  175,   34,   35,
  250,   36,   46,   47,   48,   37,  266,   51,   62,   71,
  395,   72,   75,   73,   74,   82,   83,   84,   85,   86,
  128,  131,  138,  166,  160,  176,  170,  177,  178,  251,
  191,  179,  327,  180,  181,  182,  183,  184,  185,  418,
  186,  192,  187,  188,   97,  189,  190,  193,  194,  195,
  196,  197,  198,  199,  200,  202,  201,  205,  203,  206,
  207,  221,  221,  217,  242,  247,  252,  308,  271,  253,
  267,  269,  268,  270,  272,  320,  211,  141,  141,  212,
  141,  141,  141,  141,  141,  141,  141,  141,  141,  141,
  141,  369,  334,  323,  338,  141,  141,  141,  141,  141,
  141,  141,  141,  141,   99,  141,  141,  141,  141,  141,
  141,  141,  380,  141,  127,  127,  273,  127,  127,  127,
  127,  127,  127,  127,  127,  127,  127,  127,   25,   25,
  141,  141,  127,  127,  127,  127,  127,  127,  127,  127,
  127,  384,  127,  127,  127,  127,  127,  127,  127,   17,
  127,  211,  211,  378,  212,  212,  274,   25,  275,   14,
   32,   32,   84,   84,   25,   84,   84,  127,  127,  417,
   82,   82,  276,  150,  150,  404,  150,  150,  150,  150,
  150,  150,  150,  150,  150,  150,  150,   27,   27,   32,
   18,  150,  150,  150,  150,  150,  150,  150,  150,  150,
   10,  150,  150,  150,  150,  150,  150,  150,  278,  150,
  282,  431,   41,   41,   30,   30,   27,   41,   41,   41,
   41,   41,  283,   27,  284,  285,  150,  150,  287,  288,
   41,   18,   41,   38,  292,   41,  289,  293,  294,  295,
   41,   41,   41,   41,   41,   41,   41,  296,   41,  301,
  466,   69,   69,  310,  311,  312,   69,   69,   69,   69,
   69,  313,  314,  475,  286,   41,   41,   37,   37,   69,
  290,   69,  298,  309,   69,  332,  315,  299,  326,   69,
   69,   69,   69,   69,   69,   69,  331,   69,  343,  101,
   37,  336,  342,  300,  344,  345,   37,  346,  347,  348,
  349,  354,  356,   37,   69,   69,   97,   97,  355,   97,
   97,   97,   97,   97,   97,   97,   97,   97,   97,   97,
   37,   97,   97,   97,   97,   97,   97,   97,   97,   97,
   97,   97,   97,  102,  104,   97,   97,   97,   97,  357,
  358,  359,   97,  360,  361,  362,  364,  365,  368,  370,
  374,  375,  376,  379,  383,  387,  394,  396,  398,   97,
  399,  401,  402,  403,  405,  407,   99,   99,  154,   99,
   99,   99,   99,   99,   99,   99,   99,   99,   99,   99,
  409,   99,   99,   99,   99,   99,   99,   99,   99,   99,
   99,   99,   99,  410,  411,   99,   99,   99,   99,  412,
  416,  200,   99,  421,  422,  423,  424,  425,  426,  427,
   17,   17,   17,   17,   17,   17,  429,  430,  432,   99,
   14,   14,   14,   14,  434,  437,   17,   17,  438,  439,
  441,  443,  451,  453,   17,  173,   14,   14,  454,  456,
   17,  455,    1,  458,   14,  457,  476,   17,  460,  461,
   14,  467,   18,   18,    3,   18,   18,   14,  468,  477,
  478,   10,   10,   10,   17,  225,  408,   18,   18,  201,
  440,  465,  444,  433,   14,   18,   14,   10,   10,   45,
   22,   18,  261,  256,  328,   10,   15,   49,   18,   79,
   81,   10,  428,   18,   18,   38,   38,  107,   10,  291,
  108,  109,  110,  111,    0,   18,  112,  113,   18,   18,
  114,  173,  472,    0,    0,   10,   18,    0,   38,    0,
    0,    0,   18,    0,   38,    0,    0,    0,    0,   18,
    0,   38,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   18,    0,   38,    0,
    0,  101,  101,    0,  101,  101,  101,  101,  101,  101,
  101,  101,  101,  101,  101,    0,  101,  101,  101,  101,
  101,  101,  101,  101,    0,  101,  101,  101,    0,    0,
    0,  101,  101,  101,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  104,  104,    0,  104,
  104,  104,  104,  104,  104,  104,  104,  104,  104,  104,
    0,    0,    0,    0,  104,  104,  104,  104,  104,    0,
  104,  104,  104,    0,    0,    0,  104,  104,  104,    0,
  154,  154,    0,  154,  154,  154,  154,  154,  154,  154,
  154,  154,  154,  154,    0,    0,    0,    0,  154,  154,
  154,  154,  154,    0,  154,  154,  154,    0,    0,    0,
  154,  154,  154,  200,  200,    0,  200,  200,  200,  200,
  200,  200,  200,  200,  200,  200,  200,    0,    0,    0,
    0,  200,  200,  200,  200,  200,    0,  200,  200,  200,
    0,    0,    0,  200,  200,  200,    0,  173,  173,    0,
  173,  173,  173,  173,  173,  173,  173,  173,  173,  173,
  173,    0,    0,    0,    0,  173,  173,  173,  173,  173,
    0,  173,  173,  173,    0,    0,    0,  173,  173,  173,
    0,  201,  201,    0,  201,  201,  201,  201,  201,  201,
  201,  201,  201,  201,  201,    0,    0,    0,    0,  201,
  201,  201,  201,  201,    0,  201,  201,  201,    0,    0,
    0,  201,  201,  201,
};
short yycheck[] = {                                      10,
   11,   40,   40,   40,   40,    0,   86,  185,  186,  187,
   74,   84,  177,  178,   85,  180,  181,  194,  258,   40,
  288,  259,  257,  461,  189,  190,  297,  285,  301,   22,
  258,  304,  257,   58,  199,  200,   29,  257,   40,  477,
  258,  258,    0,  304,  259,  267,  268,  269,  270,  271,
  323,  300,  323,  277,  278,  304,    0,  315,  280,   58,
  282,  285,  323,  304,  304,  317,  304,   40,  290,  304,
  292,  293,  294,  295,  296,  343,  304,   40,   40,  304,
  323,  306,  323,  323,  304,  305,  304,  304,    0,  304,
    0,  315,  304,   58,  316,  272,  308,  309,    0,  474,
  304,    0,  267,  304,  304,  299,  481,  319,  320,  321,
  304,  323,  289,  313,  302,    0,  304,  282,  191,  323,
  261,  192,  323,  264,  265,  266,   41,  262,  263,   44,
   44,  296,  264,  311,  304,  313,  314,  315,  308,  309,
    0,   58,    0,  267,  268,  269,  270,  271,  272,  273,
  274,  275,  277,  278,  279,  318,  280,  281,  282,  283,
  284,  261,  286,  287,  288,  265,  266,  179,  292,  293,
  294,  183,  184,  253,  265,  266,  310,  311,  261,    0,
  311,  312,  303,  304,  265,  266,   89,   90,   44,  257,
  201,   58,   44,  323,   58,    0,  207,  324,  257,   58,
  365,   58,  298,   58,   58,   44,   44,   58,   58,   58,
  257,  257,  291,   40,  259,   58,  260,   58,   58,  259,
   44,   58,  258,   58,   58,   58,   58,   58,   58,  394,
   58,   44,   58,   58,    0,   58,   58,   44,   44,   58,
   58,   58,   58,   58,   58,   44,   58,   58,  258,   58,
   58,  289,  289,   40,   40,  258,   44,  314,  258,   44,
   44,   44,   91,   44,   44,  304,  304,  262,  263,  307,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,  345,  293,  304,  295,  280,  281,  282,  283,  284,
  285,  286,  287,  288,    0,  290,  291,  292,  293,  294,
  295,  296,  304,  298,  262,  263,   44,  265,  266,  267,
  268,  269,  270,  271,  272,  273,  274,  275,  262,  263,
  315,  316,  280,  281,  282,  283,  284,  285,  286,  287,
  288,  304,  290,  291,  292,  293,  294,  295,  296,    0,
  298,  304,  304,  354,  307,  307,   44,  291,   40,    0,
  262,  263,  262,  263,  298,  265,  266,  315,  316,  322,
  262,  263,  258,  262,  263,  376,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,  262,  263,  291,
    0,  280,  281,  282,  283,  284,  285,  286,  287,  288,
    0,  290,  291,  292,  293,  294,  295,  296,   44,  298,
   44,  412,  262,  263,  262,  263,  291,  267,  268,  269,
  270,  271,   44,  298,   44,   44,  315,  316,   44,   44,
  280,    0,  282,    0,   91,  285,   44,   44,   91,   44,
  290,  291,  292,  293,  294,  295,  296,   44,  298,   44,
  451,  262,  263,   44,   44,  314,  267,  268,  269,  270,
  271,   44,   44,  464,  258,  315,  316,  262,  263,  280,
  258,  282,  258,  258,  285,  258,   44,  303,   44,  290,
  291,  292,  293,  294,  295,  296,   41,  298,   44,    0,
  285,  258,   93,  303,  258,   44,  291,   91,   44,   44,
  258,   44,   44,  298,  315,  316,  262,  263,  258,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
  315,  277,  278,  279,  280,  281,  282,  283,  284,  285,
  286,  287,  288,   44,    0,  291,  292,  293,  294,  258,
   44,  303,  298,   44,   44,   93,   93,   44,   44,  258,
  258,   44,   44,   44,   44,   44,   44,   44,  258,  315,
   44,   93,   44,  314,   44,  258,  262,  263,    0,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
   44,  277,  278,  279,  280,  281,  282,  283,  284,  285,
  286,  287,  288,  258,  258,  291,  292,  293,  294,   44,
   44,    0,  298,  258,   44,  276,  258,  314,   41,   44,
  261,  262,  263,  264,  265,  266,   41,   44,   41,  315,
  261,  262,  263,  264,   44,  258,  277,  278,   44,   44,
   44,  258,   44,   41,  285,    0,  277,  278,  259,   41,
  291,  258,    0,   41,  285,  259,  258,  298,   44,   44,
  291,   41,  262,  263,    0,  265,  266,  298,   44,   44,
   44,  261,  262,  263,  315,   58,  381,  277,  278,    0,
  427,  451,  431,  416,  315,  285,    5,  277,  278,   34,
   14,  291,  206,  205,  287,  285,    7,   40,  298,   59,
   61,  291,  409,  262,  263,  262,  263,   69,  298,  253,
   69,   69,   69,   69,   -1,  315,   69,   69,  277,  278,
   69,   88,  460,   -1,   -1,  315,  285,   -1,  285,   -1,
   -1,   -1,  291,   -1,  291,   -1,   -1,   -1,   -1,  298,
   -1,  298,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  315,   -1,  315,   -1,
   -1,  262,  263,   -1,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,   -1,  277,  278,  279,  280,
  281,  282,  283,  284,   -1,  286,  287,  288,   -1,   -1,
   -1,  292,  293,  294,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  262,  263,   -1,  265,
  266,  267,  268,  269,  270,  271,  272,  273,  274,  275,
   -1,   -1,   -1,   -1,  280,  281,  282,  283,  284,   -1,
  286,  287,  288,   -1,   -1,   -1,  292,  293,  294,   -1,
  262,  263,   -1,  265,  266,  267,  268,  269,  270,  271,
  272,  273,  274,  275,   -1,   -1,   -1,   -1,  280,  281,
  282,  283,  284,   -1,  286,  287,  288,   -1,   -1,   -1,
  292,  293,  294,  262,  263,   -1,  265,  266,  267,  268,
  269,  270,  271,  272,  273,  274,  275,   -1,   -1,   -1,
   -1,  280,  281,  282,  283,  284,   -1,  286,  287,  288,
   -1,   -1,   -1,  292,  293,  294,   -1,  262,  263,   -1,
  265,  266,  267,  268,  269,  270,  271,  272,  273,  274,
  275,   -1,   -1,   -1,   -1,  280,  281,  282,  283,  284,
   -1,  286,  287,  288,   -1,   -1,   -1,  292,  293,  294,
   -1,  262,  263,   -1,  265,  266,  267,  268,  269,  270,
  271,  272,  273,  274,  275,   -1,   -1,   -1,   -1,  280,
  281,  282,  283,  284,   -1,  286,  287,  288,   -1,   -1,
   -1,  292,  293,  294,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 324
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,0,"','",0,0,0,0,0,0,0,0,0,0,0,0,0,"':'",0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'['",0,"']'",0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"CHAR",
"INTEGER","BOOLEAN","PERCENT","MESSAGE_ID","MAZE_ID","LEVEL_ID","LEV_INIT_ID",
"GEOMETRY_ID","NOMAP_ID","OBJECT_ID","COBJECT_ID","MONSTER_ID","TRAP_ID",
"DOOR_ID","DRAWBRIDGE_ID","MAZEWALK_ID","WALLIFY_ID","REGION_ID","FILLING",
"RANDOM_OBJECTS_ID","RANDOM_MONSTERS_ID","RANDOM_PLACES_ID","ALTAR_ID",
"LADDER_ID","STAIR_ID","NON_DIGGABLE_ID","NON_PASSWALL_ID","ROOM_ID",
"PORTAL_ID","TELEPRT_ID","BRANCH_ID","LEV","CHANCE_ID","CORRIDOR_ID","GOLD_ID",
"ENGRAVING_ID","FOUNTAIN_ID","POOL_ID","SINK_ID","NONE","RAND_CORRIDOR_ID",
"DOOR_STATE","LIGHT_STATE","CURSE_TYPE","ENGRAVING_TYPE","DIRECTION",
"RANDOM_TYPE","O_REGISTER","M_REGISTER","P_REGISTER","A_REGISTER","ALIGNMENT",
"LEFT_OR_RIGHT","CENTER","TOP_OR_BOT","ALTAR_TYPE","UP_OR_DOWN","SUBROOM_ID",
"NAME_ID","FLAGS_ID","FLAG_TYPE","MON_ATTITUDE","MON_ALERTNESS",
"MON_APPEARANCE","CONTAINED","STRING","MAP_ID",
};
char *yyrule[] = {
"$accept : file",
"file :",
"file : levels",
"levels : level",
"levels : level levels",
"level : maze_level",
"level : room_level",
"maze_level : maze_def flags lev_init messages regions",
"room_level : level_def flags lev_init messages rreg_init rooms corridors_def",
"level_def : LEVEL_ID ':' string",
"lev_init :",
"lev_init : LEV_INIT_ID ':' CHAR ',' CHAR ',' BOOLEAN ',' BOOLEAN ',' light_state ',' walled",
"walled : BOOLEAN",
"walled : RANDOM_TYPE",
"flags :",
"flags : FLAGS_ID ':' flag_list",
"flag_list : FLAG_TYPE ',' flag_list",
"flag_list : FLAG_TYPE",
"messages :",
"messages : message messages",
"message : MESSAGE_ID ':' STRING",
"rreg_init :",
"rreg_init : rreg_init init_rreg",
"init_rreg : RANDOM_OBJECTS_ID ':' object_list",
"init_rreg : RANDOM_MONSTERS_ID ':' monster_list",
"rooms :",
"rooms : roomlist",
"roomlist : aroom",
"roomlist : aroom roomlist",
"corridors_def : random_corridors",
"corridors_def : corridors",
"random_corridors : RAND_CORRIDOR_ID",
"corridors :",
"corridors : corridors corridor",
"corridor : CORRIDOR_ID ':' corr_spec ',' corr_spec",
"corridor : CORRIDOR_ID ':' corr_spec ',' INTEGER",
"corr_spec : '(' INTEGER ',' DIRECTION ',' door_pos ')'",
"aroom : room_def room_details",
"aroom : subroom_def room_details",
"subroom_def : SUBROOM_ID ':' room_type ',' light_state ',' subroom_pos ',' room_size ',' string roomfill",
"room_def : ROOM_ID ':' room_type ',' light_state ',' room_pos ',' room_align ',' room_size roomfill",
"roomfill :",
"roomfill : ',' BOOLEAN",
"room_pos : '(' INTEGER ',' INTEGER ')'",
"room_pos : RANDOM_TYPE",
"subroom_pos : '(' INTEGER ',' INTEGER ')'",
"subroom_pos : RANDOM_TYPE",
"room_align : '(' h_justif ',' v_justif ')'",
"room_align : RANDOM_TYPE",
"room_size : '(' INTEGER ',' INTEGER ')'",
"room_size : RANDOM_TYPE",
"room_details :",
"room_details : room_details room_detail",
"room_detail : room_name",
"room_detail : room_chance",
"room_detail : room_door",
"room_detail : monster_detail",
"room_detail : object_detail",
"room_detail : trap_detail",
"room_detail : altar_detail",
"room_detail : fountain_detail",
"room_detail : sink_detail",
"room_detail : pool_detail",
"room_detail : gold_detail",
"room_detail : engraving_detail",
"room_detail : stair_detail",
"room_name : NAME_ID ':' string",
"room_chance : CHANCE_ID ':' INTEGER",
"$$1 :",
"room_door : DOOR_ID ':' secret ',' door_state ',' door_wall ',' door_pos $$1 room_door_infos",
"room_door_infos :",
"room_door_infos : room_door_infos room_door_info",
"room_door_info : ',' INTEGER",
"secret : BOOLEAN",
"secret : RANDOM_TYPE",
"door_wall : DIRECTION",
"door_wall : RANDOM_TYPE",
"door_pos : INTEGER",
"door_pos : RANDOM_TYPE",
"maze_def : MAZE_ID ':' string ',' filling",
"filling : CHAR",
"filling : RANDOM_TYPE",
"regions : aregion",
"regions : aregion regions",
"aregion : map_definition reg_init map_details",
"map_definition : NOMAP_ID",
"map_definition : map_geometry MAP_ID",
"map_geometry : GEOMETRY_ID ':' h_justif ',' v_justif",
"h_justif : LEFT_OR_RIGHT",
"h_justif : CENTER",
"v_justif : TOP_OR_BOT",
"v_justif : CENTER",
"reg_init :",
"reg_init : reg_init init_reg",
"init_reg : RANDOM_OBJECTS_ID ':' object_list",
"init_reg : RANDOM_PLACES_ID ':' place_list",
"init_reg : RANDOM_MONSTERS_ID ':' monster_list",
"object_list : object",
"object_list : object ',' object_list",
"monster_list : monster",
"monster_list : monster ',' monster_list",
"place_list : place",
"$$2 :",
"place_list : place $$2 ',' place_list",
"map_details :",
"map_details : map_details map_detail",
"map_detail : monster_detail",
"map_detail : object_detail",
"map_detail : door_detail",
"map_detail : trap_detail",
"map_detail : drawbridge_detail",
"map_detail : region_detail",
"map_detail : stair_region",
"map_detail : portal_region",
"map_detail : teleprt_region",
"map_detail : branch_region",
"map_detail : altar_detail",
"map_detail : fountain_detail",
"map_detail : mazewalk_detail",
"map_detail : wallify_detail",
"map_detail : ladder_detail",
"map_detail : stair_detail",
"map_detail : gold_detail",
"map_detail : engraving_detail",
"map_detail : diggable_detail",
"map_detail : passwall_detail",
"$$3 :",
"monster_detail : MONSTER_ID chance ':' monster_c ',' m_name ',' coordinate $$3 monster_infos",
"monster_infos :",
"monster_infos : monster_infos monster_info",
"monster_info : ',' string",
"monster_info : ',' MON_ATTITUDE",
"monster_info : ',' MON_ALERTNESS",
"monster_info : ',' alignment",
"monster_info : ',' MON_APPEARANCE string",
"object_detail : OBJECT_ID object_desc",
"object_detail : COBJECT_ID object_desc",
"$$4 :",
"object_desc : chance ':' object_c ',' o_name $$4 ',' object_where object_infos",
"object_where : coordinate",
"object_where : CONTAINED",
"object_infos :",
"object_infos : ',' curse_state ',' monster_id ',' enchantment optional_name",
"object_infos : ',' curse_state ',' enchantment optional_name",
"object_infos : ',' monster_id ',' enchantment optional_name",
"curse_state : RANDOM_TYPE",
"curse_state : CURSE_TYPE",
"monster_id : STRING",
"enchantment : RANDOM_TYPE",
"enchantment : INTEGER",
"optional_name :",
"optional_name : ',' NONE",
"optional_name : ',' STRING",
"$$5 :",
"door_detail : DOOR_ID ':' door_state ',' coordinate $$5 door_infos",
"door_infos :",
"door_infos : door_infos door_info",
"door_info : ',' INTEGER",
"trap_detail : TRAP_ID chance ':' trap_name ',' coordinate",
"drawbridge_detail : DRAWBRIDGE_ID ':' coordinate ',' DIRECTION ',' door_state",
"mazewalk_detail : MAZEWALK_ID ':' coordinate ',' DIRECTION",
"wallify_detail : WALLIFY_ID",
"ladder_detail : LADDER_ID ':' coordinate ',' UP_OR_DOWN",
"stair_detail : STAIR_ID ':' coordinate ',' UP_OR_DOWN",
"$$6 :",
"stair_region : STAIR_ID ':' lev_region $$6 ',' lev_region ',' UP_OR_DOWN",
"$$7 :",
"portal_region : PORTAL_ID ':' lev_region $$7 ',' lev_region ',' string",
"$$8 :",
"$$9 :",
"teleprt_region : TELEPRT_ID ':' lev_region $$8 ',' lev_region $$9 teleprt_detail",
"$$10 :",
"branch_region : BRANCH_ID ':' lev_region $$10 ',' lev_region",
"teleprt_detail :",
"teleprt_detail : ',' UP_OR_DOWN",
"lev_region : region",
"lev_region : LEV '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'",
"fountain_detail : FOUNTAIN_ID ':' coordinate",
"sink_detail : SINK_ID ':' coordinate",
"pool_detail : POOL_ID ':' coordinate",
"diggable_detail : NON_DIGGABLE_ID ':' region",
"passwall_detail : NON_PASSWALL_ID ':' region",
"region_detail : REGION_ID ':' region ',' light_state ',' room_type prefilled",
"altar_detail : ALTAR_ID ':' coordinate ',' alignment ',' altar_type",
"gold_detail : GOLD_ID ':' amount ',' coordinate",
"engraving_detail : ENGRAVING_ID ':' coordinate ',' engraving_type ',' string",
"monster_c : monster",
"monster_c : RANDOM_TYPE",
"monster_c : m_register",
"object_c : object",
"object_c : RANDOM_TYPE",
"object_c : o_register",
"m_name : string",
"m_name : RANDOM_TYPE",
"o_name : string",
"o_name : RANDOM_TYPE",
"trap_name : string",
"trap_name : RANDOM_TYPE",
"room_type : string",
"room_type : RANDOM_TYPE",
"prefilled :",
"prefilled : ',' FILLING",
"prefilled : ',' FILLING ',' BOOLEAN",
"coordinate : coord",
"coordinate : p_register",
"coordinate : RANDOM_TYPE",
"door_state : DOOR_STATE",
"door_state : RANDOM_TYPE",
"light_state : LIGHT_STATE",
"light_state : RANDOM_TYPE",
"alignment : ALIGNMENT",
"alignment : a_register",
"alignment : RANDOM_TYPE",
"altar_type : ALTAR_TYPE",
"altar_type : RANDOM_TYPE",
"p_register : P_REGISTER '[' INTEGER ']'",
"o_register : O_REGISTER '[' INTEGER ']'",
"m_register : M_REGISTER '[' INTEGER ']'",
"a_register : A_REGISTER '[' INTEGER ']'",
"place : coord",
"monster : CHAR",
"object : CHAR",
"string : STRING",
"amount : INTEGER",
"amount : RANDOM_TYPE",
"chance :",
"chance : PERCENT",
"engraving_type : ENGRAVING_TYPE",
"engraving_type : RANDOM_TYPE",
"coord : '(' INTEGER ',' INTEGER ')'",
"region : '(' INTEGER ',' INTEGER ',' INTEGER ',' INTEGER ')'",
};
#endif
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH 500
#endif
#endif
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#define yystacksize YYSTACKSIZE
#line 1713 "lev_comp.y"

/*lev_comp.y*/
#line 937 "lev_comp.tab.c"
#define YYABORT goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
#ifdef lint
    goto yynewerror;
#endif
yynewerror:
    yyerror("syntax error");
#ifdef lint
    goto yyerrlab;
#endif
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 7:
#line 182 "lev_comp.y"
{
			unsigned i;

			if (fatal_error > 0) {
				(void) fprintf(stderr,
				"%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
				maze.flags = yyvsp[-3].i;
				(void) memcpy((genericptr_t)&(maze.init_lev),
						(genericptr_t)&(init_lev),
						sizeof(lev_init));
				maze.numpart = npart;
				maze.parts = NewTab(mazepart, npart);
				for(i=0;i<npart;i++)
				    maze.parts[i] = tmppart[i];
				if (!write_level_file(yyvsp[-4].map, (splev *)0, &maze)) {
					yyerror("Can't write output file!!");
					exit(EXIT_FAILURE);
				}
				npart = 0;
			}
			Free(yyvsp[-4].map);
		  }
break;
case 8:
#line 209 "lev_comp.y"
{
			unsigned i;

			if (fatal_error > 0) {
			    (void) fprintf(stderr,
			      "%s : %d errors detected. No output created!\n",
					fname, fatal_error);
			} else {
				special_lev.flags = (long) yyvsp[-5].i;
				(void) memcpy(
					(genericptr_t)&(special_lev.init_lev),
					(genericptr_t)&(init_lev),
					sizeof(lev_init));
				special_lev.nroom = nrooms;
				special_lev.rooms = NewTab(room, nrooms);
				for(i=0; i<nrooms; i++)
				    special_lev.rooms[i] = tmproom[i];
				special_lev.ncorr = ncorridor;
				special_lev.corrs = NewTab(corridor, ncorridor);
				for(i=0; i<ncorridor; i++)
				    special_lev.corrs[i] = tmpcor[i];
				if (check_subrooms()) {
				    if (!write_level_file(yyvsp[-6].map, &special_lev,
							  (specialmaze *)0)) {
					yyerror("Can't write output file!!");
					exit(EXIT_FAILURE);
				    }
				}
				free_rooms(&special_lev);
				nrooms = 0;
				ncorridor = 0;
			}
			Free(yyvsp[-6].map);
		  }
break;
case 9:
#line 246 "lev_comp.y"
{
			if (index(yyvsp[0].map, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if ((int) strlen(yyvsp[0].map) > 8)
			    yyerror("Level names limited to 8 characters.");
			yyval.map = yyvsp[0].map;
			special_lev.nrmonst = special_lev.nrobjects = 0;
			n_mlist = n_olist = 0;
		  }
break;
case 10:
#line 258 "lev_comp.y"
{
			/* in case we're processing multiple files,
			   explicitly clear any stale settings */
			(void) memset((genericptr_t) &init_lev, 0,
					sizeof init_lev);
			init_lev.init_present = FALSE;
			yyval.i = 0;
		  }
break;
case 11:
#line 267 "lev_comp.y"
{
			init_lev.init_present = TRUE;
			init_lev.fg = what_map_char((char) yyvsp[-10].i);
			if (init_lev.fg == INVALID_TYPE)
			    yyerror("Invalid foreground type.");
			init_lev.bg = what_map_char((char) yyvsp[-8].i);
			if (init_lev.bg == INVALID_TYPE)
			    yyerror("Invalid background type.");
			init_lev.smoothed = yyvsp[-6].i;
			init_lev.joined = yyvsp[-4].i;
			if (init_lev.joined &&
			    init_lev.fg != CORR && init_lev.fg != ROOM)
			    yyerror("Invalid foreground type for joined map.");
			init_lev.lit = yyvsp[-2].i;
			init_lev.walled = yyvsp[0].i;
			yyval.i = 1;
		  }
break;
case 14:
#line 291 "lev_comp.y"
{
			yyval.i = 0;
		  }
break;
case 15:
#line 295 "lev_comp.y"
{
			yyval.i = lev_flags;
			lev_flags = 0;	/* clear for next user */
		  }
break;
case 16:
#line 302 "lev_comp.y"
{
			lev_flags |= yyvsp[-2].i;
		  }
break;
case 17:
#line 306 "lev_comp.y"
{
			lev_flags |= yyvsp[0].i;
		  }
break;
case 20:
#line 316 "lev_comp.y"
{
			int i, j;

			i = (int) strlen(yyvsp[0].map) + 1;
			j = (int) strlen(tmpmessage);
			if (i + j > 255) {
			   yyerror("Message string too long (>256 characters)");
			} else {
			    if (j) tmpmessage[j++] = '\n';
			    (void) strncpy(tmpmessage+j, yyvsp[0].map, i - 1);
			    tmpmessage[j + i - 1] = 0;
			}
			Free(yyvsp[0].map);
		  }
break;
case 23:
#line 337 "lev_comp.y"
{
			if(special_lev.nrobjects) {
			    yyerror("Object registers already initialized!");
			} else {
			    special_lev.nrobjects = n_olist;
			    special_lev.robjects = (char *) alloc(n_olist);
			    (void) memcpy((genericptr_t)special_lev.robjects,
					  (genericptr_t)olist, n_olist);
			}
		  }
break;
case 24:
#line 348 "lev_comp.y"
{
			if(special_lev.nrmonst) {
			    yyerror("Monster registers already initialized!");
			} else {
			    special_lev.nrmonst = n_mlist;
			    special_lev.rmonst = (char *) alloc(n_mlist);
			    (void) memcpy((genericptr_t)special_lev.rmonst,
					  (genericptr_t)mlist, n_mlist);
			  }
		  }
break;
case 25:
#line 361 "lev_comp.y"
{
			tmproom[nrooms] = New(room);
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->parent = (char *) 0;
			tmproom[nrooms]->rtype = 0;
			tmproom[nrooms]->rlit = 0;
			tmproom[nrooms]->xalign = ERR;
			tmproom[nrooms]->yalign = ERR;
			tmproom[nrooms]->x = 0;
			tmproom[nrooms]->y = 0;
			tmproom[nrooms]->w = 2;
			tmproom[nrooms]->h = 2;
			in_room = 1;
		  }
break;
case 31:
#line 387 "lev_comp.y"
{
			tmpcor[0] = New(corridor);
			tmpcor[0]->src.room = -1;
			ncorridor = 1;
		  }
break;
case 34:
#line 399 "lev_comp.y"
{
			tmpcor[ncorridor] = New(corridor);
			tmpcor[ncorridor]->src.room = yyvsp[-2].corpos.room;
			tmpcor[ncorridor]->src.wall = yyvsp[-2].corpos.wall;
			tmpcor[ncorridor]->src.door = yyvsp[-2].corpos.door;
			tmpcor[ncorridor]->dest.room = yyvsp[0].corpos.room;
			tmpcor[ncorridor]->dest.wall = yyvsp[0].corpos.wall;
			tmpcor[ncorridor]->dest.door = yyvsp[0].corpos.door;
			ncorridor++;
			if (ncorridor >= MAX_OF_TYPE) {
				yyerror("Too many corridors in level!");
				ncorridor--;
			}
		  }
break;
case 35:
#line 414 "lev_comp.y"
{
			tmpcor[ncorridor] = New(corridor);
			tmpcor[ncorridor]->src.room = yyvsp[-2].corpos.room;
			tmpcor[ncorridor]->src.wall = yyvsp[-2].corpos.wall;
			tmpcor[ncorridor]->src.door = yyvsp[-2].corpos.door;
			tmpcor[ncorridor]->dest.room = -1;
			tmpcor[ncorridor]->dest.wall = yyvsp[0].i;
			ncorridor++;
			if (ncorridor >= MAX_OF_TYPE) {
				yyerror("Too many corridors in level!");
				ncorridor--;
			}
		  }
break;
case 36:
#line 430 "lev_comp.y"
{
			if ((unsigned) yyvsp[-5].i >= nrooms)
			    yyerror("Wrong room number!");
			yyval.corpos.room = yyvsp[-5].i;
			yyval.corpos.wall = yyvsp[-3].i;
			yyval.corpos.door = yyvsp[-1].i;
		  }
break;
case 37:
#line 440 "lev_comp.y"
{
			store_room();
		  }
break;
case 38:
#line 444 "lev_comp.y"
{
			store_room();
		  }
break;
case 39:
#line 450 "lev_comp.y"
{
			tmproom[nrooms] = New(room);
			tmproom[nrooms]->parent = yyvsp[-1].map;
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->rtype = yyvsp[-9].i;
			tmproom[nrooms]->rlit = yyvsp[-7].i;
			tmproom[nrooms]->filled = yyvsp[0].i;
			tmproom[nrooms]->xalign = ERR;
			tmproom[nrooms]->yalign = ERR;
			tmproom[nrooms]->x = current_coord.x;
			tmproom[nrooms]->y = current_coord.y;
			tmproom[nrooms]->w = current_size.width;
			tmproom[nrooms]->h = current_size.height;
			in_room = 1;
		  }
break;
case 40:
#line 468 "lev_comp.y"
{
			tmproom[nrooms] = New(room);
			tmproom[nrooms]->name = (char *) 0;
			tmproom[nrooms]->parent = (char *) 0;
			tmproom[nrooms]->rtype = yyvsp[-9].i;
			tmproom[nrooms]->rlit = yyvsp[-7].i;
			tmproom[nrooms]->filled = yyvsp[0].i;
			tmproom[nrooms]->xalign = current_align.x;
			tmproom[nrooms]->yalign = current_align.y;
			tmproom[nrooms]->x = current_coord.x;
			tmproom[nrooms]->y = current_coord.y;
			tmproom[nrooms]->w = current_size.width;
			tmproom[nrooms]->h = current_size.height;
			in_room = 1;
		  }
break;
case 41:
#line 486 "lev_comp.y"
{
			yyval.i = 1;
		  }
break;
case 42:
#line 490 "lev_comp.y"
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 43:
#line 496 "lev_comp.y"
{
			if ( yyvsp[-3].i < 1 || yyvsp[-3].i > 5 ||
			    yyvsp[-1].i < 1 || yyvsp[-1].i > 5 ) {
			    yyerror("Room position should be between 1 & 5!");
			} else {
			    current_coord.x = yyvsp[-3].i;
			    current_coord.y = yyvsp[-1].i;
			}
		  }
break;
case 44:
#line 506 "lev_comp.y"
{
			current_coord.x = current_coord.y = ERR;
		  }
break;
case 45:
#line 512 "lev_comp.y"
{
			if ( yyvsp[-3].i < 0 || yyvsp[-1].i < 0) {
			    yyerror("Invalid subroom position !");
			} else {
			    current_coord.x = yyvsp[-3].i;
			    current_coord.y = yyvsp[-1].i;
			}
		  }
break;
case 46:
#line 521 "lev_comp.y"
{
			current_coord.x = current_coord.y = ERR;
		  }
break;
case 47:
#line 527 "lev_comp.y"
{
			current_align.x = yyvsp[-3].i;
			current_align.y = yyvsp[-1].i;
		  }
break;
case 48:
#line 532 "lev_comp.y"
{
			current_align.x = current_align.y = ERR;
		  }
break;
case 49:
#line 538 "lev_comp.y"
{
			current_size.width = yyvsp[-3].i;
			current_size.height = yyvsp[-1].i;
		  }
break;
case 50:
#line 543 "lev_comp.y"
{
			current_size.height = current_size.width = ERR;
		  }
break;
case 66:
#line 568 "lev_comp.y"
{
			if (tmproom[nrooms]->name)
			    yyerror("This room already has a name!");
			else
			    tmproom[nrooms]->name = yyvsp[0].map;
		  }
break;
case 67:
#line 577 "lev_comp.y"
{
			if (tmproom[nrooms]->chance)
			    yyerror("This room already assigned a chance!");
			else if (tmproom[nrooms]->rtype == OROOM)
			    yyerror("Only typed rooms can have a chance!");
			else if (yyvsp[0].i < 1 || yyvsp[0].i > 99)
			    yyerror("The chance is supposed to be percentile.");
			else
			    tmproom[nrooms]->chance = yyvsp[0].i;
		   }
break;
case 68:
#line 590 "lev_comp.y"
{
			/* ERR means random here */
			if (yyvsp[-2].i == ERR && yyvsp[0].i != ERR) {
		     yyerror("If the door wall is random, so must be its pos!");
			    tmprdoor[ndoor] = 0;
			} else {
			    tmprdoor[ndoor] = New(room_door);
			    tmprdoor[ndoor]->secret = yyvsp[-6].i;
			    tmprdoor[ndoor]->mask = yyvsp[-4].i;
			    tmprdoor[ndoor]->wall = yyvsp[-2].i;
			    tmprdoor[ndoor]->pos = yyvsp[0].i;
			    tmprdoor[ndoor]->arti_text = 0;
			}
		  }
break;
case 69:
#line 605 "lev_comp.y"
{
			if (tmprdoor[ndoor]) {
			    ndoor++;
			    if (ndoor >= MAX_OF_TYPE) {
				    yyerror("Too many doors in room!");
				    ndoor--;
			    }
			}
		  }
break;
case 72:
#line 621 "lev_comp.y"
{

			if (tmprdoor[ndoor])
			    tmprdoor[ndoor]->arti_text = yyvsp[0].i;
		  }
break;
case 79:
#line 642 "lev_comp.y"
{
			maze.filling = (schar) yyvsp[0].i;
			if (index(yyvsp[-2].map, '.'))
			    yyerror("Invalid dot ('.') in level name.");
			if ((int) strlen(yyvsp[-2].map) > 8)
			    yyerror("Level names limited to 8 characters.");
			yyval.map = yyvsp[-2].map;
			in_room = 0;
			n_plist = n_mlist = n_olist = 0;
		  }
break;
case 80:
#line 655 "lev_comp.y"
{
			yyval.i = get_floor_type((char)yyvsp[0].i);
		  }
break;
case 81:
#line 659 "lev_comp.y"
{
			yyval.i = -1;
		  }
break;
case 84:
#line 669 "lev_comp.y"
{
			store_part();
		  }
break;
case 85:
#line 675 "lev_comp.y"
{
			tmppart[npart] = New(mazepart);
			tmppart[npart]->halign = 1;
			tmppart[npart]->valign = 1;
			tmppart[npart]->nrobjects = 0;
			tmppart[npart]->nloc = 0;
			tmppart[npart]->nrmonst = 0;
			tmppart[npart]->xsize = 1;
			tmppart[npart]->ysize = 1;
			tmppart[npart]->map = (char **) alloc(sizeof(char *));
			tmppart[npart]->map[0] = (char *) alloc(1);
			tmppart[npart]->map[0][0] = STONE;
			max_x_map = COLNO-1;
			max_y_map = ROWNO;
		  }
break;
case 86:
#line 691 "lev_comp.y"
{
			tmppart[npart] = New(mazepart);
			tmppart[npart]->halign = yyvsp[-1].i % 10;
			tmppart[npart]->valign = yyvsp[-1].i / 10;
			tmppart[npart]->nrobjects = 0;
			tmppart[npart]->nloc = 0;
			tmppart[npart]->nrmonst = 0;
			scan_map(yyvsp[0].map);
			Free(yyvsp[0].map);
		  }
break;
case 87:
#line 704 "lev_comp.y"
{
			yyval.i = yyvsp[-2].i + (yyvsp[0].i * 10);
		  }
break;
case 94:
#line 722 "lev_comp.y"
{
			if (tmppart[npart]->nrobjects) {
			    yyerror("Object registers already initialized!");
			} else {
			    tmppart[npart]->robjects = (char *)alloc(n_olist);
			    (void) memcpy((genericptr_t)tmppart[npart]->robjects,
					  (genericptr_t)olist, n_olist);
			    tmppart[npart]->nrobjects = n_olist;
			}
		  }
break;
case 95:
#line 733 "lev_comp.y"
{
			if (tmppart[npart]->nloc) {
			    yyerror("Location registers already initialized!");
			} else {
			    register int i;
			    tmppart[npart]->rloc_x = (char *) alloc(n_plist);
			    tmppart[npart]->rloc_y = (char *) alloc(n_plist);
			    for(i=0;i<n_plist;i++) {
				tmppart[npart]->rloc_x[i] = plist[i].x;
				tmppart[npart]->rloc_y[i] = plist[i].y;
			    }
			    tmppart[npart]->nloc = n_plist;
			}
		  }
break;
case 96:
#line 748 "lev_comp.y"
{
			if (tmppart[npart]->nrmonst) {
			    yyerror("Monster registers already initialized!");
			} else {
			    tmppart[npart]->rmonst = (char *) alloc(n_mlist);
			    (void) memcpy((genericptr_t)tmppart[npart]->rmonst,
					  (genericptr_t)mlist, n_mlist);
			    tmppart[npart]->nrmonst = n_mlist;
			}
		  }
break;
case 97:
#line 761 "lev_comp.y"
{
			if (n_olist < MAX_REGISTERS)
			    olist[n_olist++] = yyvsp[0].i;
			else
			    yyerror("Object list too long!");
		  }
break;
case 98:
#line 768 "lev_comp.y"
{
			if (n_olist < MAX_REGISTERS)
			    olist[n_olist++] = yyvsp[-2].i;
			else
			    yyerror("Object list too long!");
		  }
break;
case 99:
#line 777 "lev_comp.y"
{
			if (n_mlist < MAX_REGISTERS)
			    mlist[n_mlist++] = yyvsp[0].i;
			else
			    yyerror("Monster list too long!");
		  }
break;
case 100:
#line 784 "lev_comp.y"
{
			if (n_mlist < MAX_REGISTERS)
			    mlist[n_mlist++] = yyvsp[-2].i;
			else
			    yyerror("Monster list too long!");
		  }
break;
case 101:
#line 793 "lev_comp.y"
{
			if (n_plist < MAX_REGISTERS)
			    plist[n_plist++] = current_coord;
			else
			    yyerror("Location list too long!");
		  }
break;
case 102:
#line 800 "lev_comp.y"
{
			if (n_plist < MAX_REGISTERS)
			    plist[n_plist++] = current_coord;
			else
			    yyerror("Location list too long!");
		  }
break;
case 126:
#line 836 "lev_comp.y"
{
			tmpmonst[nmons] = New(monster);
			tmpmonst[nmons]->x = current_coord.x;
			tmpmonst[nmons]->y = current_coord.y;
			tmpmonst[nmons]->class = yyvsp[-4].i;
			tmpmonst[nmons]->peaceful = -1; /* no override */
			tmpmonst[nmons]->asleep = -1;
			tmpmonst[nmons]->align = - MAX_REGISTERS - 2;
			tmpmonst[nmons]->name.str = 0;
			tmpmonst[nmons]->appear = 0;
			tmpmonst[nmons]->appear_as.str = 0;
			tmpmonst[nmons]->chance = yyvsp[-6].i;
			tmpmonst[nmons]->id = NON_PM;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Monster");
			if (yyvsp[-2].map) {
			    int token = get_monster_id(yyvsp[-2].map, (char) yyvsp[-4].i);
			    if (token == ERR)
				yywarning(
			      "Invalid monster name!  Making random monster.");
			    else
				tmpmonst[nmons]->id = token;
			    Free(yyvsp[-2].map);
			}
		  }
break;
case 127:
#line 863 "lev_comp.y"
{
			if (++nmons >= MAX_OF_TYPE) {
			    yyerror("Too many monsters in room or mazepart!");
			    nmons--;
			}
		  }
break;
case 130:
#line 876 "lev_comp.y"
{
			tmpmonst[nmons]->name.str = yyvsp[0].map;
		  }
break;
case 131:
#line 880 "lev_comp.y"
{
			tmpmonst[nmons]->peaceful = yyvsp[0].i;
		  }
break;
case 132:
#line 884 "lev_comp.y"
{
			tmpmonst[nmons]->asleep = yyvsp[0].i;
		  }
break;
case 133:
#line 888 "lev_comp.y"
{
			tmpmonst[nmons]->align = yyvsp[0].i;
		  }
break;
case 134:
#line 892 "lev_comp.y"
{
			tmpmonst[nmons]->appear = yyvsp[-1].i;
			tmpmonst[nmons]->appear_as.str = yyvsp[0].map;
		  }
break;
case 135:
#line 899 "lev_comp.y"
{
		  }
break;
case 136:
#line 902 "lev_comp.y"
{
			/* 1: is contents of preceeding object with 2 */
			/* 2: is a container */
			/* 0: neither */
			tmpobj[nobj-1]->containment = 2;
		  }
break;
case 137:
#line 911 "lev_comp.y"
{
			tmpobj[nobj] = New(object);
			tmpobj[nobj]->class = yyvsp[-2].i;
			tmpobj[nobj]->corpsenm = NON_PM;
			tmpobj[nobj]->curse_state = -1;
			tmpobj[nobj]->name.str = 0;
			tmpobj[nobj]->chance = yyvsp[-4].i;
			tmpobj[nobj]->id = -1;
			if (yyvsp[0].map) {
			    int token = get_object_id(yyvsp[0].map, yyvsp[-2].i);
			    if (token == ERR)
				yywarning(
				"Illegal object name!  Making random object.");
			     else
				tmpobj[nobj]->id = token;
			    Free(yyvsp[0].map);
			}
		  }
break;
case 138:
#line 930 "lev_comp.y"
{
			if (++nobj >= MAX_OF_TYPE) {
			    yyerror("Too many objects in room or mazepart!");
			    nobj--;
			}
		  }
break;
case 139:
#line 939 "lev_comp.y"
{
			tmpobj[nobj]->containment = 0;
			tmpobj[nobj]->x = current_coord.x;
			tmpobj[nobj]->y = current_coord.y;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Object");
		  }
break;
case 140:
#line 948 "lev_comp.y"
{
			tmpobj[nobj]->containment = 1;
			/* random coordinate, will be overridden anyway */
			tmpobj[nobj]->x = -MAX_REGISTERS-1;
			tmpobj[nobj]->y = -MAX_REGISTERS-1;
		  }
break;
case 141:
#line 957 "lev_comp.y"
{
			tmpobj[nobj]->spe = -127;
	/* Note below: we're trying to make as many of these optional as
	 * possible.  We clearly can't make curse_state, enchantment, and
	 * monster_id _all_ optional, since ",random" would be ambiguous.
	 * We can't even just make enchantment mandatory, since if we do that
	 * alone, ",random" requires too much lookahead to parse.
	 */
		  }
break;
case 142:
#line 967 "lev_comp.y"
{
		  }
break;
case 143:
#line 970 "lev_comp.y"
{
		  }
break;
case 144:
#line 973 "lev_comp.y"
{
		  }
break;
case 145:
#line 978 "lev_comp.y"
{
			tmpobj[nobj]->curse_state = -1;
		  }
break;
case 146:
#line 982 "lev_comp.y"
{
			tmpobj[nobj]->curse_state = yyvsp[0].i;
		  }
break;
case 147:
#line 988 "lev_comp.y"
{
			int token = get_monster_id(yyvsp[0].map, (char)0);
			if (token == ERR)	/* "random" */
			    tmpobj[nobj]->corpsenm = NON_PM - 1;
			else
			    tmpobj[nobj]->corpsenm = token;
			Free(yyvsp[0].map);
		  }
break;
case 148:
#line 999 "lev_comp.y"
{
			tmpobj[nobj]->spe = -127;
		  }
break;
case 149:
#line 1003 "lev_comp.y"
{
			tmpobj[nobj]->spe = yyvsp[0].i;
		  }
break;
case 151:
#line 1010 "lev_comp.y"
{
		  }
break;
case 152:
#line 1013 "lev_comp.y"
{
			tmpobj[nobj]->name.str = yyvsp[0].map;
		  }
break;
case 153:
#line 1019 "lev_comp.y"
{
			tmpdoor[ndoor] = New(door);
			tmpdoor[ndoor]->x = current_coord.x;
			tmpdoor[ndoor]->y = current_coord.y;
			tmpdoor[ndoor]->mask = yyvsp[-2].i;
			tmpdoor[ndoor]->arti_text = 0;
			if(current_coord.x >= 0 && current_coord.y >= 0 &&
			   tmpmap[current_coord.y][current_coord.x] != DOOR &&
			   tmpmap[current_coord.y][current_coord.x] != SDOOR)
			    yyerror("Door decl doesn't match the map");
		  }
break;
case 154:
#line 1031 "lev_comp.y"
{
			if (++ndoor >= MAX_OF_TYPE) {
			    yyerror("Too many doors in mazepart!");
			    ndoor--;
			}
		  }
break;
case 157:
#line 1044 "lev_comp.y"
{
			tmpdoor[ndoor]->arti_text = yyvsp[0].i;
		  }
break;
case 158:
#line 1050 "lev_comp.y"
{
			tmptrap[ntrap] = New(trap);
			tmptrap[ntrap]->x = current_coord.x;
			tmptrap[ntrap]->y = current_coord.y;
			tmptrap[ntrap]->type = yyvsp[-2].i;
			tmptrap[ntrap]->chance = yyvsp[-4].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Trap");
			if (++ntrap >= MAX_OF_TYPE) {
				yyerror("Too many traps in room or mazepart!");
				ntrap--;
			}
		  }
break;
case 159:
#line 1067 "lev_comp.y"
{
		        int x, y, dir;

			tmpdb[ndb] = New(drawbridge);
			x = tmpdb[ndb]->x = current_coord.x;
			y = tmpdb[ndb]->y = current_coord.y;
			/* convert dir from a DIRECTION to a DB_DIR */
			dir = yyvsp[-2].i;
			switch(dir) {
			case W_NORTH: dir = DB_NORTH; y--; break;
			case W_SOUTH: dir = DB_SOUTH; y++; break;
			case W_EAST:  dir = DB_EAST;  x++; break;
			case W_WEST:  dir = DB_WEST;  x--; break;
			default:
			    yyerror("Invalid drawbridge direction");
			    break;
			}
			tmpdb[ndb]->dir = dir;
			if (current_coord.x >= 0 && current_coord.y >= 0 &&
			    !IS_WALL(tmpmap[y][x])) {
			    char ebuf[60];
			    Sprintf(ebuf,
				    "Wall needed for drawbridge (%02d, %02d)",
				    current_coord.x, current_coord.y);
			    yyerror(ebuf);
			}

			if ( yyvsp[0].i == D_ISOPEN )
			    tmpdb[ndb]->db_open = 1;
			else if ( yyvsp[0].i == D_CLOSED )
			    tmpdb[ndb]->db_open = 0;
			else
			    yyerror("A drawbridge can only be open or closed!");
			ndb++;
			if (ndb >= MAX_OF_TYPE) {
				yyerror("Too many drawbridges in mazepart!");
				ndb--;
			}
		   }
break;
case 160:
#line 1109 "lev_comp.y"
{
			tmpwalk[nwalk] = New(walk);
			tmpwalk[nwalk]->x = current_coord.x;
			tmpwalk[nwalk]->y = current_coord.y;
			tmpwalk[nwalk]->dir = yyvsp[0].i;
			nwalk++;
			if (nwalk >= MAX_OF_TYPE) {
				yyerror("Too many mazewalks in mazepart!");
				nwalk--;
			}
		  }
break;
case 161:
#line 1123 "lev_comp.y"
{
			wallify_map();
		  }
break;
case 162:
#line 1129 "lev_comp.y"
{
			tmplad[nlad] = New(lad);
			tmplad[nlad]->x = current_coord.x;
			tmplad[nlad]->y = current_coord.y;
			tmplad[nlad]->up = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Ladder");
			nlad++;
			if (nlad >= MAX_OF_TYPE) {
				yyerror("Too many ladders in mazepart!");
				nlad--;
			}
		  }
break;
case 163:
#line 1146 "lev_comp.y"
{
			tmpstair[nstair] = New(stair);
			tmpstair[nstair]->x = current_coord.x;
			tmpstair[nstair]->y = current_coord.y;
			tmpstair[nstair]->up = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Stairway");
			nstair++;
			if (nstair >= MAX_OF_TYPE) {
				yyerror("Too many stairs in room or mazepart!");
				nstair--;
			}
		  }
break;
case 164:
#line 1163 "lev_comp.y"
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 165:
#line 1172 "lev_comp.y"
{
			tmplreg[nlreg]->del_islev = yyvsp[-2].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			if(yyvsp[0].i)
			    tmplreg[nlreg]->rtype = LR_UPSTAIR;
			else
			    tmplreg[nlreg]->rtype = LR_DOWNSTAIR;
			tmplreg[nlreg]->rname.str = 0;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 166:
#line 1192 "lev_comp.y"
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 167:
#line 1201 "lev_comp.y"
{
			tmplreg[nlreg]->del_islev = yyvsp[-2].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			tmplreg[nlreg]->rtype = LR_PORTAL;
			tmplreg[nlreg]->rname.str = yyvsp[0].map;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 168:
#line 1218 "lev_comp.y"
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 169:
#line 1227 "lev_comp.y"
{
			tmplreg[nlreg]->del_islev = yyvsp[0].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
		  }
break;
case 170:
#line 1235 "lev_comp.y"
{
			switch(yyvsp[0].i) {
			case -1: tmplreg[nlreg]->rtype = LR_TELE; break;
			case 0: tmplreg[nlreg]->rtype = LR_DOWNTELE; break;
			case 1: tmplreg[nlreg]->rtype = LR_UPTELE; break;
			}
			tmplreg[nlreg]->rname.str = 0;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 171:
#line 1251 "lev_comp.y"
{
			tmplreg[nlreg] = New(lev_region);
			tmplreg[nlreg]->in_islev = yyvsp[0].i;
			tmplreg[nlreg]->inarea.x1 = current_region.x1;
			tmplreg[nlreg]->inarea.y1 = current_region.y1;
			tmplreg[nlreg]->inarea.x2 = current_region.x2;
			tmplreg[nlreg]->inarea.y2 = current_region.y2;
		  }
break;
case 172:
#line 1260 "lev_comp.y"
{
			tmplreg[nlreg]->del_islev = yyvsp[0].i;
			tmplreg[nlreg]->delarea.x1 = current_region.x1;
			tmplreg[nlreg]->delarea.y1 = current_region.y1;
			tmplreg[nlreg]->delarea.x2 = current_region.x2;
			tmplreg[nlreg]->delarea.y2 = current_region.y2;
			tmplreg[nlreg]->rtype = LR_BRANCH;
			tmplreg[nlreg]->rname.str = 0;
			nlreg++;
			if (nlreg >= MAX_OF_TYPE) {
				yyerror("Too many levregions in mazepart!");
				nlreg--;
			}
		  }
break;
case 173:
#line 1277 "lev_comp.y"
{
			yyval.i = -1;
		  }
break;
case 174:
#line 1281 "lev_comp.y"
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 175:
#line 1287 "lev_comp.y"
{
			yyval.i = 0;
		  }
break;
case 176:
#line 1291 "lev_comp.y"
{
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if (yyvsp[-7].i <= 0 || yyvsp[-7].i >= COLNO)
				yyerror("Region out of level range!");
			else if (yyvsp[-5].i < 0 || yyvsp[-5].i >= ROWNO)
				yyerror("Region out of level range!");
			else if (yyvsp[-3].i <= 0 || yyvsp[-3].i >= COLNO)
				yyerror("Region out of level range!");
			else if (yyvsp[-1].i < 0 || yyvsp[-1].i >= ROWNO)
				yyerror("Region out of level range!");
			current_region.x1 = yyvsp[-7].i;
			current_region.y1 = yyvsp[-5].i;
			current_region.x2 = yyvsp[-3].i;
			current_region.y2 = yyvsp[-1].i;
			yyval.i = 1;
		  }
break;
case 177:
#line 1311 "lev_comp.y"
{
			tmpfountain[nfountain] = New(fountain);
			tmpfountain[nfountain]->x = current_coord.x;
			tmpfountain[nfountain]->y = current_coord.y;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Fountain");
			nfountain++;
			if (nfountain >= MAX_OF_TYPE) {
			    yyerror("Too many fountains in room or mazepart!");
			    nfountain--;
			}
		  }
break;
case 178:
#line 1327 "lev_comp.y"
{
			tmpsink[nsink] = New(sink);
			tmpsink[nsink]->x = current_coord.x;
			tmpsink[nsink]->y = current_coord.y;
			nsink++;
			if (nsink >= MAX_OF_TYPE) {
				yyerror("Too many sinks in room!");
				nsink--;
			}
		  }
break;
case 179:
#line 1340 "lev_comp.y"
{
			tmppool[npool] = New(pool);
			tmppool[npool]->x = current_coord.x;
			tmppool[npool]->y = current_coord.y;
			npool++;
			if (npool >= MAX_OF_TYPE) {
				yyerror("Too many pools in room!");
				npool--;
			}
		  }
break;
case 180:
#line 1353 "lev_comp.y"
{
			tmpdig[ndig] = New(digpos);
			tmpdig[ndig]->x1 = current_region.x1;
			tmpdig[ndig]->y1 = current_region.y1;
			tmpdig[ndig]->x2 = current_region.x2;
			tmpdig[ndig]->y2 = current_region.y2;
			ndig++;
			if (ndig >= MAX_OF_TYPE) {
				yyerror("Too many diggables in mazepart!");
				ndig--;
			}
		  }
break;
case 181:
#line 1368 "lev_comp.y"
{
			tmppass[npass] = New(digpos);
			tmppass[npass]->x1 = current_region.x1;
			tmppass[npass]->y1 = current_region.y1;
			tmppass[npass]->x2 = current_region.x2;
			tmppass[npass]->y2 = current_region.y2;
			npass++;
			if (npass >= 32) {
				yyerror("Too many passwalls in mazepart!");
				npass--;
			}
		  }
break;
case 182:
#line 1383 "lev_comp.y"
{
			tmpreg[nreg] = New(region);
			tmpreg[nreg]->x1 = current_region.x1;
			tmpreg[nreg]->y1 = current_region.y1;
			tmpreg[nreg]->x2 = current_region.x2;
			tmpreg[nreg]->y2 = current_region.y2;
			tmpreg[nreg]->rlit = yyvsp[-3].i;
			tmpreg[nreg]->rtype = yyvsp[-1].i;
			if(yyvsp[0].i & 1) tmpreg[nreg]->rtype += MAXRTYPE+1;
			tmpreg[nreg]->rirreg = ((yyvsp[0].i & 2) != 0);
			if(current_region.x1 > current_region.x2 ||
			   current_region.y1 > current_region.y2)
			   yyerror("Region start > end!");
			if(tmpreg[nreg]->rtype == VAULT &&
			   (tmpreg[nreg]->rirreg ||
			    (tmpreg[nreg]->x2 - tmpreg[nreg]->x1 != 1) ||
			    (tmpreg[nreg]->y2 - tmpreg[nreg]->y1 != 1)))
				yyerror("Vaults must be exactly 2x2!");
			if(want_warnings && !tmpreg[nreg]->rirreg &&
			   current_region.x1 > 0 && current_region.y1 > 0 &&
			   current_region.x2 < (int)max_x_map &&
			   current_region.y2 < (int)max_y_map) {
			    /* check for walls in the room */
			    char ebuf[60];
			    register int x, y, nrock = 0;

			    for(y=current_region.y1; y<=current_region.y2; y++)
				for(x=current_region.x1;
				    x<=current_region.x2; x++)
				    if(IS_ROCK(tmpmap[y][x]) ||
				       IS_DOOR(tmpmap[y][x])) nrock++;
			    if(nrock) {
				Sprintf(ebuf,
					"Rock in room (%02d,%02d,%02d,%02d)?!",
					current_region.x1, current_region.y1,
					current_region.x2, current_region.y2);
				yywarning(ebuf);
			    }
			    if (
		!IS_ROCK(tmpmap[current_region.y1-1][current_region.x1-1]) ||
		!IS_ROCK(tmpmap[current_region.y2+1][current_region.x1-1]) ||
		!IS_ROCK(tmpmap[current_region.y1-1][current_region.x2+1]) ||
		!IS_ROCK(tmpmap[current_region.y2+1][current_region.x2+1])) {
				Sprintf(ebuf,
				"NonRock edge in room (%02d,%02d,%02d,%02d)?!",
					current_region.x1, current_region.y1,
					current_region.x2, current_region.y2);
				yywarning(ebuf);
			    }
			} else if(tmpreg[nreg]->rirreg &&
		!IS_ROOM(tmpmap[current_region.y1][current_region.x1])) {
			    char ebuf[60];
			    Sprintf(ebuf,
				    "Rock in irregular room (%02d,%02d)?!",
				    current_region.x1, current_region.y1);
			    yyerror(ebuf);
			}
			nreg++;
			if (nreg >= MAX_OF_TYPE) {
				yyerror("Too many regions in mazepart!");
				nreg--;
			}
		  }
break;
case 183:
#line 1449 "lev_comp.y"
{
			tmpaltar[naltar] = New(altar);
			tmpaltar[naltar]->x = current_coord.x;
			tmpaltar[naltar]->y = current_coord.y;
			tmpaltar[naltar]->align = yyvsp[-2].i;
			tmpaltar[naltar]->shrine = yyvsp[0].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Altar");
			naltar++;
			if (naltar >= MAX_OF_TYPE) {
				yyerror("Too many altars in room or mazepart!");
				naltar--;
			}
		  }
break;
case 184:
#line 1467 "lev_comp.y"
{
			tmpgold[ngold] = New(gold);
			tmpgold[ngold]->x = current_coord.x;
			tmpgold[ngold]->y = current_coord.y;
			tmpgold[ngold]->amount = yyvsp[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Gold");
			ngold++;
			if (ngold >= MAX_OF_TYPE) {
				yyerror("Too many golds in room or mazepart!");
				ngold--;
			}
		  }
break;
case 185:
#line 1484 "lev_comp.y"
{
			tmpengraving[nengraving] = New(engraving);
			tmpengraving[nengraving]->x = current_coord.x;
			tmpengraving[nengraving]->y = current_coord.y;
			tmpengraving[nengraving]->engr.str = yyvsp[0].map;
			tmpengraving[nengraving]->etype = yyvsp[-2].i;
			if (!in_room)
			    check_coord(current_coord.x, current_coord.y,
					"Engraving");
			nengraving++;
			if (nengraving >= MAX_OF_TYPE) {
			    yyerror("Too many engravings in room or mazepart!");
			    nengraving--;
			}
		  }
break;
case 187:
#line 1503 "lev_comp.y"
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 190:
#line 1511 "lev_comp.y"
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 193:
#line 1519 "lev_comp.y"
{
			yyval.map = (char *) 0;
		  }
break;
case 195:
#line 1526 "lev_comp.y"
{
			yyval.map = (char *) 0;
		  }
break;
case 196:
#line 1532 "lev_comp.y"
{
			int token = get_trap_type(yyvsp[0].map);
			if (token == ERR)
				yyerror("Unknown trap type!");
			yyval.i = token;
			Free(yyvsp[0].map);
		  }
break;
case 198:
#line 1543 "lev_comp.y"
{
			int token = get_room_type(yyvsp[0].map);
			if (token == ERR) {
				yywarning("Unknown room type!  Making ordinary room...");
				yyval.i = OROOM;
			} else
				yyval.i = token;
			Free(yyvsp[0].map);
		  }
break;
case 200:
#line 1556 "lev_comp.y"
{
			yyval.i = 0;
		  }
break;
case 201:
#line 1560 "lev_comp.y"
{
			yyval.i = yyvsp[0].i;
		  }
break;
case 202:
#line 1564 "lev_comp.y"
{
			yyval.i = yyvsp[-2].i + (yyvsp[0].i << 1);
		  }
break;
case 205:
#line 1572 "lev_comp.y"
{
			current_coord.x = current_coord.y = -MAX_REGISTERS-1;
		  }
break;
case 212:
#line 1588 "lev_comp.y"
{
			yyval.i = - MAX_REGISTERS - 1;
		  }
break;
case 215:
#line 1598 "lev_comp.y"
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				current_coord.x = current_coord.y = - yyvsp[-1].i - 1;
		  }
break;
case 216:
#line 1607 "lev_comp.y"
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 217:
#line 1616 "lev_comp.y"
{
			if ( yyvsp[-1].i >= MAX_REGISTERS )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 218:
#line 1625 "lev_comp.y"
{
			if ( yyvsp[-1].i >= 3 )
				yyerror("Register Index overflow!");
			else
				yyval.i = - yyvsp[-1].i - 1;
		  }
break;
case 220:
#line 1637 "lev_comp.y"
{
			if (check_monster_char((char) yyvsp[0].i))
				yyval.i = yyvsp[0].i ;
			else {
				yyerror("Unknown monster class!");
				yyval.i = ERR;
			}
		  }
break;
case 221:
#line 1648 "lev_comp.y"
{
			char c = yyvsp[0].i;
			if (check_object_char(c))
				yyval.i = c;
			else {
				yyerror("Unknown char class!");
				yyval.i = ERR;
			}
		  }
break;
case 225:
#line 1667 "lev_comp.y"
{
			yyval.i = 100;	/* default is 100% */
		  }
break;
case 226:
#line 1671 "lev_comp.y"
{
			if (yyvsp[0].i <= 0 || yyvsp[0].i > 100)
			    yyerror("Expected percentile chance.");
			yyval.i = yyvsp[0].i;
		  }
break;
case 229:
#line 1683 "lev_comp.y"
{
			if (!in_room && !init_lev.init_present &&
			    (yyvsp[-3].i < 0 || yyvsp[-3].i > (int)max_x_map ||
			     yyvsp[-1].i < 0 || yyvsp[-1].i > (int)max_y_map))
			    yyerror("Coordinates out of map range!");
			current_coord.x = yyvsp[-3].i;
			current_coord.y = yyvsp[-1].i;
		  }
break;
case 230:
#line 1694 "lev_comp.y"
{
/* This series of if statements is a hack for MSC 5.1.  It seems that its
   tiny little brain cannot compile if these are all one big if statement. */
			if (yyvsp[-7].i < 0 || yyvsp[-7].i > (int)max_x_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-5].i < 0 || yyvsp[-5].i > (int)max_y_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-3].i < 0 || yyvsp[-3].i > (int)max_x_map)
				yyerror("Region out of map range!");
			else if (yyvsp[-1].i < 0 || yyvsp[-1].i > (int)max_y_map)
				yyerror("Region out of map range!");
			current_region.x1 = yyvsp[-7].i;
			current_region.y1 = yyvsp[-5].i;
			current_region.x2 = yyvsp[-3].i;
			current_region.y2 = yyvsp[-1].i;
		  }
break;
#line 2546 "lev_comp.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
