/*	SCCS Id: @(#)dungeon.h	3.4	1999/07/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DUNGEON_H
#define DUNGEON_H

typedef struct d_flags {	/* dungeon/level type flags */
	Bitfield(town, 1);	/* is this a town? (levels only) */
	Bitfield(hellish, 1);	/* is this part of hell? */
	Bitfield(maze_like, 1); /* is this a maze? */
	Bitfield(rogue_like, 1); /* is this an old-fashioned presentation? */
	Bitfield(align, 3);	/* dungeon alignment. */
	Bitfield(raise, 3);	/* corpse resurection type (current max 8) */
	Bitfield(unused, 1);	/* etc... */
} d_flags;

typedef struct d_level {	/* basic dungeon level element */
	int		dnum;		/* dungeon number */
	int		dlevel;		/* level number */
	d_flags flags;		/* type flags */
} d_level;

typedef struct s_level {	/* special dungeon level element */
	struct	s_level *next;
	d_level dlevel;		/* dungeon & level numbers */
	char	proto[15];	/* name of prototype file (eg. "tower") */
	char	boneid;		/* character to id level in bones files */
	uchar	rndlevs;	/* no. of randomly available similar levels */
	d_flags flags;		/* type flags */
} s_level;

typedef struct stairway {	/* basic stairway identifier */
	xchar	sx, sy;		/* x / y location of the stair */
	d_level tolev;		/* where does it go */
	char	up;		/* what type of stairway (up/down) */
} stairway;

/* level region types */
#define LR_DOWNSTAIR 0
#define LR_UPSTAIR 1
#define LR_PORTAL 2
#define LR_BRANCH 3
#define LR_TELE 4
#define LR_UPTELE 5
#define LR_DOWNTELE 6

typedef struct dest_area {	/* non-stairway level change indentifier */
	xchar	lx, ly;		/* "lower" left corner (near [0,0]) */
	xchar	hx, hy;		/* "upper" right corner (near [COLNO,ROWNO]) */
	xchar	nlx, nly;	/* outline of invalid area */
	xchar	nhx, nhy;	/* opposite corner of invalid area */
} dest_area;

typedef struct dungeon {	/* basic dungeon identifier */
	char	dname[24];	/* name of the dungeon (eg. "Hell") */
	char	proto[15];	/* name of prototype file (eg. "tower") */
	char	boneid;		/* character to id dungeon in bones files */
	d_flags flags;		/* dungeon flags */
	int	entry_lev;	/* entry level */
	int	num_dunlevs;	/* number of levels in this dungeon */
	int	dunlev_ureached; /* how deep you have been in this dungeon */
	int	ledger_start,	/* the starting depth in "real" terms */
		depth_start;	/* the starting depth in "logical" terms */
} dungeon;

/*
 * A branch structure defines the connection between two dungeons.  They
 * will be ordered by the dungeon number/level number of 'end1'.  Ties
 * are resolved by 'end2'.  'Type' uses 'end1' arbitrarily as the primary
 * point.
 */
typedef struct branch {
    struct branch *next;	/* next in the branch chain */
    int		  id;		/* branch identifier */
    int		  type;		/* type of branch */
    d_level	  end1;		/* "primary" end point */
    d_level	  end2;		/* other end point */
    boolean	  end1_up;	/* does end1 go up? */
} branch;

/* branch types */
#define BR_STAIR   0	/* "Regular" connection, 2 staircases. */
#define BR_NO_END1 1	/* "Regular" connection.  However, no stair from  */
			/*	end1 to end2.  There is a stair from end2 */
			/*	to end1.				  */
#define BR_NO_END2 2	/* "Regular" connection.  However, no stair from  */
			/*	end2 to end1.  There is a stair from end1 */
			/*	to end2.				  */
#define BR_PORTAL  3	/* Connection by magic portals (traps) */


/* A particular dungeon contains num_dunlevs d_levels with dlevel 1..
 * num_dunlevs.  Ledger_start and depth_start are bases that are added
 * to the dlevel of a particular d_level to get the effective ledger_no
 * and depth for that d_level.
 *
 * Ledger_no is a bookkeeping number that gives a unique identifier for a
 * particular d_level (for level.?? files, e.g.).
 *
 * Depth corresponds to the number of floors below the surface.
 */
#define Is_astralevel(x)	(on_level(x, &astral_level))
#define Is_earthlevel(x)	(on_level(x, &earth_level))
#define Is_waterlevel(x)	(on_level(x, &water_level))
#define Is_firelevel(x)		(on_level(x, &fire_level))
#define Is_airlevel(x)		(on_level(x, &air_level))

#define Is_challenge_level(x)	(on_level(x, &challenge_level))
#define Is_medusa_level(x)	(on_level(x, &challenge_level) && dungeon_topology.challenge_variant >= MEDUSA_LEVEL1 && dungeon_topology.challenge_variant <= MEDUSA_LEVEL4)
#define Is_grue_level(x)	(on_level(x, &challenge_level) && dungeon_topology.challenge_variant >= GRUE_LEVEL1 && dungeon_topology.challenge_variant <= GRUE_LEVEL2)

#define Is_oracle_level(x)	(on_level(x, &oracle_level))

//Law quest
#define Is_path(x)		(on_level(x, &path1_level) || on_level(x, &path2_level) || on_level(x, &path3_level))
#define Is_illregrd(x)	(on_level(x, &illregrd_level))
#define Is_arcadia(x)	(on_level(x, &arcadia1_level) ||\
						 on_level(x, &arcadia2_level) ||\
						 on_level(x, &arcadia3_level) ||\
						 on_level(x, &arcward_level) ||\
						 on_level(x, &tower1_level) ||\
						 on_level(x, &tower2_level) ||\
						 on_level(x, &tower3_level) ||\
						 on_level(x, &towertop_level) ||\
						 on_level(x, &arcfort_level)\
						)
#define Is_arcadia_woods(x)	(on_level(x, &arcadia1_level) ||\
						 on_level(x, &arcadia2_level) ||\
						 on_level(x, &arcadia3_level)\
						)
#define Is_arcadia3(x)	(on_level(x, &arcadia3_level))

#define Is_arcadiatower1(x)	(on_level(x, &tower1_level))
#define Is_arcadiatower2(x)	(on_level(x, &tower2_level))
#define Is_arcadiatower3(x)	(on_level(x, &tower3_level))
#define Is_arcadiadonjon(x)	(on_level(x, &towertop_level))

//Chaos quest 1
#define Is_lich_level(x)	(on_level(x, &chaosfrh_level))
#define Is_marilith_level(x)	(on_level(x, &chaosffh_level))
#define Is_kraken_level(x)	(on_level(x, &chaossth_level))
#define Is_tiamat_level(x)	(on_level(x, &chaosvth_level))
#define Is_chaos_level(x)	(on_level(x, &chaose_level))

//Chaos quest 2
#define Is_elshava(x)		(on_level(x,&elshava_level))
#define Is_last_spire(x)	(on_level(x,&lastspire_level))

//Chaos quest 3
#define Is_ford_level(x)	(on_level(x,&ford_level))
#define Is_spider_cave(x)	(on_level(x,&spider_level))

//Neutral quest
#define In_depths(x)		((x)->dnum == rlyeh_dnum)
#define Is_gatetown(x)		(on_level(x,&gatetown_level))
#define Is_bridge_temple(x)		(on_level(x,&bridge_temple))
#define Is_sumall(x)		(on_level(x,&sum_of_all_level))
#define Is_rlyeh(x)			(on_level(x, &rlyeh_level))

#define Is_valley(x)		(on_level(x, &valley_level))

#define Is_hell1(x)			(on_level(x, &hell1_level))
#define Is_bael_level(x)	(on_level(x, &hell1_level) && dungeon_topology.hell1_variant == BAEL_LEVEL)
#define Is_dis_level(x)		(on_level(x, &hell1_level) && dungeon_topology.hell1_variant == DISPATER_LEVEL)
#define Is_mammon_level(x)	(on_level(x, &hell1_level) && dungeon_topology.hell1_variant == MAMMON_LEVEL)
#define Is_belial_level(x)	(on_level(x, &hell1_level) && dungeon_topology.hell1_variant == BELIAL_LEVEL)
#define Is_hell2(x)			(on_level(x, &hell2_level))
#define Is_leviathan_level(x)	(on_level(x, &hell2_level) && dungeon_topology.hell2_variant == LEVIATHAN_LEVEL)
#define Is_lilith_level(x)		(on_level(x, &hell2_level) && dungeon_topology.hell2_variant == LILITH_LEVEL)
#define Is_baalzebub_level(x)	(on_level(x, &hell2_level) && dungeon_topology.hell2_variant == BAALZEBUB_LEVEL)
#define Is_mephisto_level(x)	(on_level(x, &hell2_level) && dungeon_topology.hell2_variant == MEPHISTOPHELES_LEVEL)
#define Is_hell3(x)			(on_level(x, &hell3_level))
#define Is_asmo_level(x)	(on_level(x, &hell3_level))

#define Is_abyss1(x)		(on_level(x, &abyss1_level))
#define Is_juiblex_level(x)	(on_level(x, &abyss1_level) && dungeon_topology.abyss_variant == JUIBLEX_LEVEL)
#define Is_zuggtmoy_level(x)	(on_level(x, &abyss1_level) && dungeon_topology.abyss_variant == ZUGGTMOY_LEVEL)
#define Is_yeenoghu_level(x)	(on_level(x, &abyss1_level) && dungeon_topology.abyss_variant == YEENOGHU_LEVEL)
#define Is_baphomet_level(x)	(on_level(x, &abyss1_level) && dungeon_topology.abyss_variant == BAPHOMET_LEVEL)
#define Is_night_level(x)	(on_level(x, &abyss1_level) && dungeon_topology.abyss_variant == NIGHT_LEVEL)
#define Is_abyss2(x)		(on_level(x, &abyss2_level))
#define Is_malcanthet_level(x)	(on_level(x, &abyss2_level) && dungeon_topology.abys2_variant == MALCANTHET_LEVEL)
#define Is_grazzt_level(x)	(on_level(x, &abyss2_level) && dungeon_topology.abys2_variant == GRAZ_ZT_LEVEL)
#define Is_orcus_level(x)	(on_level(x, &abyss2_level) && dungeon_topology.abys2_variant == ORCUS_LEVEL)
#define Is_lolth_level(x)	(on_level(x, &abyss2_level) && dungeon_topology.abys2_variant == LOLTH_LEVEL)
#define Is_abyss3(x)		(on_level(x, &abyss3_level))
#define Is_demogorgon_level(x)	(on_level(x, &abyss3_level) && dungeon_topology.brine_variant == DEMOGORGON_LEVEL)
#define Is_dagon_level(x)	(on_level(x, &abyss3_level) && dungeon_topology.brine_variant == DAGON_LEVEL)
#define Is_lamashtu_level(x)	(on_level(x, &abyss3_level) && dungeon_topology.brine_variant == LAMASHTU_LEVEL)

#define Is_wiz1_level(x)	(on_level(x, &wiz1_level))
#define Is_wiz2_level(x)	(on_level(x, &wiz2_level))
#define Is_wiz3_level(x)	(on_level(x, &wiz3_level))
#define Is_sanctum(x)		(on_level(x, &sanctum_level))
#define Is_portal_level(x)	(on_level(x, &portal_level))
#define Is_rogue_level(x)	(on_level(x, &rogue_level))
#define Is_stronghold(x)	(on_level(x, &stronghold_level))
#define Is_bigroom(x)		(on_level(x, &bigroom_level))
#define Is_qstart(x)		(on_level(x, &qstart_level))
#define Is_qlocate(x)		(on_level(x, &qlocate_level))
#define Is_nemesis(x)		(on_level(x, &nemesis_level))
#define Is_knox(x)		(on_level(x, &knox_level))
#define Is_minetown_level(x)     (on_level(x, &minetown_level))
#define Is_mineend_level(x)     (on_level(x, &mineend_level))
#define Is_sokoend_level(x)     (on_level(x, &sokoend_level))

#define In_sokoban(x)	((x)->dnum == sokoban_dnum)
#define In_tower(x)		((x)->dnum == tower_dnum)
#define In_sea(x)		((x)->dnum == sea_dnum)
#define Is_sunsea(x)	(In_sea(x) && dungeon_topology.sea_variant == SUNLESS_SEA_LEVEL)
#define Is_paradise(x)	(In_sea(x) && dungeon_topology.sea_variant == PARADISE_ISLAND_LEVEL)
#define Is_sunkcity(x)	(In_sea(x) && dungeon_topology.sea_variant == SUNKEN_CITY_LEVEL)
#define Is_peanut(x)	(In_sea(x) && dungeon_topology.sea_variant == PEANUT_ISLAND_LEVEL)
#define In_moloch_temple(x)	((x)->dnum == temple_dnum)
#define Inhell			In_hell(&u.uz)	/* now gehennom */
#define In_endgame(x)		((x)->dnum == astral_level.dnum)

#define within_bounded_area(X,Y,LX,LY,HX,HY) \
		((X) >= (LX) && (X) <= (HX) && (Y) >= (LY) && (Y) <= (HY))

/* monster and object migration codes */

#define MIGR_NOWHERE	      (-1)	/* failure flag for down_gate() */
#define MIGR_RANDOM		0
#define MIGR_APPROX_XY		1	/* approximate coordinates */
#define MIGR_EXACT_XY		2	/* specific coordinates */
#define MIGR_STAIRS_UP		3
#define MIGR_STAIRS_DOWN	4
#define MIGR_LADDER_UP		5
#define MIGR_LADDER_DOWN	6
#define MIGR_SSTAIRS		7	/* dungeon branch */
#define MIGR_PORTAL		8	/* magic portal */
#define MIGR_NEAR_PLAYER	9	/* mon: followers; obj: trap door */

/* level information (saved via ledger number) */

struct linfo {
	unsigned char	flags;
#define VISITED		0x01	/* hero has visited this level */
#define FORGOTTEN	0x02	/* hero will forget this level when reached */
#define LFILE_EXISTS	0x04	/* a level file exists for this level */
/*
 * Note:  VISITED and LFILE_EXISTS are currently almost always set at the
 * same time.  However they _mean_ different things.
 */

#ifdef MFLOPPY
# define FROMPERM	 1	/* for ramdisk use */
# define TOPERM		 2	/* for ramdisk use */
# define ACTIVE		 1
# define SWAPPED	 2
	int	where;
	long	time;
	long	size;
#endif /* MFLOPPY */
};

/* types and structures for dungeon map recording
 *
 * It is designed to eliminate the need for an external notes file for some of
 * the more mundane dungeon elements.  "Where was the last altar I passed?" etc...
 * Presumably the character can remember this sort of thing even if, months
 * later in real time picking up an old save game, I can't.
 *
 * To be consistent, one can assume that this map is in the player's mind and
 * has no physical correspondence (eliminating illiteracy/blind/hands/hands free
 * concerns.) Therefore, this map is not exaustive nor detailed ("some fountains").
 * This makes it also subject to player conditions (amnesia).
 */

/* Because clearly Nethack needs more ways to specify alignment */
#define Amask2msa(x) ((x) == AM_LAWFUL ? MSA_LAWFUL : \
					  (x) == AM_NEUTRAL ? MSA_NEUTRAL : \
					  (x) == AM_CHAOTIC ? MSA_CHAOTIC : \
					  (x) == AM_NONE ? MSA_UNALI : \
					  (x) == AM_VOID ? MSA_VOID : \
					  MSA_MULTI)
#define Msa2amask(x) ((x) == MSA_LAWFUL ? AM_LAWFUL : \
					  (x) == MSA_NEUTRAL ? AM_NEUTRAL : \
					  (x) == MSA_CHAOTIC ? AM_CHAOTIC : \
					  (x) == MSA_UNALI ? AM_NONE : \
					  (x) == MSA_VOID ? AM_VOID : \
					  MSA_MULTI)
#define MSA_MULTI	0  /* multiple alignments */
#define MSA_LAWFUL  1
#define MSA_NEUTRAL 2
#define MSA_CHAOTIC 3
#define MSA_UNALI 	4
#define MSA_VOID 	5

typedef struct mapseen_feat {
	/* feature knowledge that must be calculated from levl array */
	Bitfield(nfount, 2);
	Bitfield(nsink, 2);
	Bitfield(naltar, 2);
	Bitfield(msalign, 3); /* corresponds to MSA_* above */
	Bitfield(ngrave, 2);
	Bitfield(nthrone, 2);
	Bitfield(ntree, 2);
	/* water, lava, ice are too verbose so commented out for now */
	/*
	Bitfield(water, 1);
	Bitfield(lava, 1);
	Bitfield(ice, 1);
	*/

	/* calculated from rooms array */
	Bitfield(nshop, 2);
	Bitfield(nmorgue, 2);
	Bitfield(ntemple, 2);
	Bitfield(shoptype, 5);

	Bitfield(forgot, 1); /* player has forgotten about this level? */
} mapseen_feat;

/* for mapseen->rooms */
#define MSR_SEEN		1

/* what the player knows about a single dungeon level */
/* initialized in mklev() */
typedef struct mapseen  {
	struct mapseen *next; /* next map in the chain */
	branch *br; /* knows about branch via taking it in goto_level */
	d_level lev; /* corresponding dungeon level */

	mapseen_feat feat;

	/* custom naming */
	char *custom;
	unsigned custom_lth;

	/* maybe this should just be in struct mkroom? */
	schar rooms[(MAXNROFROOMS+1)*2];
} mapseen;


#endif /* DUNGEON_H */
