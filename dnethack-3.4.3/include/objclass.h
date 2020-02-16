/*	SCCS Id: @(#)objclass.h 3.4	1996/06/16	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OBJCLASS_H
#define OBJCLASS_H

/* definition of a class of objects */

struct objclass {
	short	oc_name_idx;		/* index of actual name */
	short	oc_descr_idx;		/* description when name unknown */
	char *	oc_uname;		/* called by user */
	Bitfield(oc_name_known,1);
	Bitfield(oc_merge,1);	/* merge otherwise equal objects */
	Bitfield(oc_uses_known,1); /* obj->known affects full description */
				/* otherwise, obj->dknown and obj->bknown */
				/* tell all, and obj->known should always */
				/* be set for proper merging behavior */
	Bitfield(oc_pre_discovered,1);	/* Already known at start of game; */
					/* won't be listed as a discovery. */
	Bitfield(oc_magic,1);	/* inherently magical object */
	Bitfield(oc_charged,1); /* may have +n or (n) charges */
	Bitfield(oc_unique,1);	/* special one-of-a-kind object */
	Bitfield(oc_nowish,1);	/* cannot wish for this object */

	Bitfield(oc_size, 3);	/* inherent size of the item; modifies size by about half */

	/* TODO: change all places oc_bimanual and oc_bulky are used */
#define oc_bimanual	oc_size==MZ_HUGE	/* for weapons & tools used as weapons */
#define oc_bulky	oc_size==MZ_HUGE	/* for armor */
	Bitfield(oc_tough,1);	/* hard gems/rings */

	Bitfield(oc_dir,4);
#define NODIR		1	/* for wands/spells: non-directional */
#define IMMEDIATE	2	/*		     directional */
#define RAY		3	/*		     zap beams */

#define WHACK		1
#define PIERCE		2	/* for weapons & tools used as weapons */
#define SLASH		4	/* (latter includes iron ball & chain) */
#define EXPLOSION	8	/* (rockets,  grenades) */

	/*Bitfield(oc_subtyp,3);*/	/* Now too big for a bitfield... see below */

	Bitfield(oc_material,5); //31 max
#define LIQUID		1	/* currently only for venom */
#define WAX			2
#define VEGGY		3	/* foodstuffs */
#define FLESH		4	/*   ditto    */
#define PAPER		5
#define CLOTH		6
#define LEATHER		7
#define WOOD		8
#define BONE		9
#define SHELL		10
#define DRAGON_HIDE	11	/* not leather! */
#define IRON		12	/* Fe - includes steel */
#define METAL		13	/* Sn, &c. */
#define COPPER		14	/* Cu - includes brass and bronze*/
#define SILVER		15	/* Ag */
#define GOLD		16	/* Au */
#define PLATINUM	17	/* Pt */
#define MITHRIL		18
#define PLASTIC		19
#define GLASS		20
#define GEMSTONE	21
#define MINERAL		22
#define OBSIDIAN_MT	23
#define SALT		24
#define SHADOWSTEEL	25
 //Note: 31 max, coordinate with obj.h
	Bitfield(oc_showmat,4);
#define UNIDED	1	/* always show material when base object type is unknown */
#define IDED	2	/* always show material when base object type is known */
#define NUNIDED 4	/* never show material when base object type is unknown */
#define NIDED	8	/* never show material when base object type is known */

#define is_organic(otmp)	((otmp)->obj_material <= WOOD)
#define is_metallic(otmp)	((otmp)->obj_material >= IRON && \
				 (otmp)->obj_material <= MITHRIL)
#define is_hard(otmp)	((otmp)->obj_material >= WOOD)

/* primary damage: fire/rust/--- */
/* is_flammable(otmp), is_rottable(otmp) in mkobj.c */
#define is_rustprone(otmp)	((otmp)->obj_material == IRON)

/* secondary damage: rot/acid/acid */
#define is_corrodeable(otmp)	((otmp)->obj_material == COPPER || (otmp)->obj_material == IRON)

#define is_evaporable(otmp)	((otmp)->obj_material == SHADOWSTEEL)

/* no partial damage available, but much the same */
#define is_shatterable(otmp) (((otmp)->obj_material == GLASS || (otmp)->obj_material == OBSIDIAN_MT) && otmp->oclass != GEM_CLASS)

#define is_damageable(otmp) (is_rustprone(otmp) || is_flammable(otmp) || \
				is_rottable(otmp) || is_corrodeable(otmp) || is_evaporable(otmp) || is_shatterable(otmp))

#define is_opaque(otmp) ((otmp)->obj_material != GLASS)

#define is_boomerang(otmp) (objects[(otmp)->otyp].oc_skill == -P_BOOMERANG)

	schar	oc_subtyp;
#define oc_skill	oc_subtyp   /* Skills of weapons, spellbooks, tools, gems */
#define oc_armcat	oc_subtyp   /* for armor */
#define ARM_SHIELD	1	/* needed for special wear function */
#define ARM_HELM	2
#define ARM_GLOVES	3
#define ARM_BOOTS	4
#define ARM_CLOAK	5
#define ARM_SHIRT	6
#define ARM_SUIT	0

	uchar	oc_oprop;		/* property (invis, &c.) conveyed */
	char	oc_class;		/* object class */
	schar	oc_delay;		/* delay when using such an object */
	uchar	oc_color;		/* color of the object */

	short	oc_prob;		/* probability, used in mkobj() */
	unsigned short	oc_weight;	/* encumbrance (1 cn = 0.1 lb.) */
	short	oc_cost;		/* base cost in shops */
/* Check the AD&D rules!  The FIRST is small monster damage. */
/* for weapons, and tools, rocks, and gems useful as weapons */
	schar	oc_wsdam, oc_wldam;	/* max small/large monster damage */
#define oc_range	oc_wsdam	/* for strength independant ranged weapons */
#define oc_rof		oc_wldam	/* rate of fire bonus for ranged weapons */
	
	schar	oc_oc1, oc_oc2, oc_oc3;
#define oc_hitbon	oc_oc1		/* weapons: "to hit" bonus */
#define w_ammotyp	oc_oc2		/* type of ammo taken by ranged weapon */
#define WP_GENERIC	1
#define WP_BULLET	2
#define WP_SHELL	4
#define WP_ROCKET	8
#define WP_GRENADE	16
#define WP_BLASTER	32

#define a_ac		oc_oc1	/* armor class, used in arm_ac_bonus */
#define a_can		oc_oc2		/* armor: used in mhitu.c */
#define a_dr		oc_oc3		/* armor damage reduction, used in arm_dr_bonus */
#define oc_level	oc_oc2		/* books: spell level */

	unsigned short	oc_nutrition;	/* food value */
};

struct objdescr {
	const char *oc_name;		/* actual name */
	const char *oc_descr;		/* description when name unknown */
};

struct colorTextClr {
	const char *colorText;	/* text name of color */
	const uchar colorClr;	/* displayed color */
};

struct material {
	const int id;				/* the #define'd id that this material is for */
	const int color;			/* default material color */
	const int density;			/* density (from old dnh) */
	const int cost;				/* cost multiplier (from xnethack, unused) */
	const int defense;			/* defense modifier (from xnethack, unused) */
};

extern NEARDATA struct objclass objects[];
extern NEARDATA struct objdescr obj_descr[];
extern NEARDATA struct colorTextClr LightsaberColor[];

/*
 * All objects have a class. Make sure that all classes have a corresponding
 * symbol below.
 */
#define RANDOM_CLASS	 0	/* used for generating random objects */
#define ILLOBJ_CLASS	 1
#define WEAPON_CLASS	 2
#define ARMOR_CLASS	 3
#define RING_CLASS	 4
#define AMULET_CLASS	 5
#define TOOL_CLASS	 6
#define FOOD_CLASS	 7
#define POTION_CLASS	 8
#define SCROLL_CLASS	 9
#define SPBOOK_CLASS	10	/* actually SPELL-book */
#define WAND_CLASS	11
#define COIN_CLASS	12
#define GEM_CLASS	13
#define ROCK_CLASS	14
#define BALL_CLASS	15
#define CHAIN_CLASS	16
#define VENOM_CLASS	17
#define TILE_CLASS	18
/*define BED_CLASS	19*/
#define MAXOCLASSES	19

#define ALLOW_COUNT	(MAXOCLASSES+1) /* Can be used in the object class */
#define ALL_CLASSES	(MAXOCLASSES+2) /* input to getobj().		   */
#define ALLOW_NONE	(MAXOCLASSES+3) /*				   */

#define BURNING_OIL	(MAXOCLASSES+1) /* Can be used as input to explode. */
#define MON_EXPLODE	(MAXOCLASSES+2) /* Exploding monster (e.g. gas spore) */

#if 0	/* moved to decl.h so that makedefs.c won't see them */
extern const char def_oc_syms[MAXOCLASSES];	/* default class symbols */
extern uchar oc_syms[MAXOCLASSES];		/* current class symbols */
#endif

/* Default definitions of all object-symbols (must match classes above). */

#define ILLOBJ_SYM	']'	/* also used for mimics */
#define WEAPON_SYM	')'
#define ARMOR_SYM	'['
#define RING_SYM	'='
#define AMULET_SYM	'"'
#define TOOL_SYM	'('
#define FOOD_SYM	'%'
#define POTION_SYM	'!'
#define SCROLL_SYM	'?'
#define SPBOOK_SYM	'+'
#define WAND_SYM	'/'
#define GOLD_SYM	'$'
#define GEM_SYM		'*'
#define ROCK_SYM	'`'
#define BALL_SYM	'0'
#define CHAIN_SYM	'_'
#define VENOM_SYM	'.'
#define TILE_SYM	'-'
#define BED_SYM		'#'

struct fruit {
	char fname[PL_FSIZ];
	int fid;
	struct fruit *nextf;
};
#define newfruit() (struct fruit *)alloc(sizeof(struct fruit))
#define dealloc_fruit(rind) free((genericptr_t) (rind))

#define OBJ_NAME(obj)  (obj_descr[(obj).oc_name_idx].oc_name)
#define OBJ_DESCR(obj) (obj_descr[(obj).oc_descr_idx].oc_descr)
#endif /* OBJCLASS_H */
