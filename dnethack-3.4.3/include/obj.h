/*	SCCS Id: @(#)obj.h	3.4	2002/01/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef TEXTCOLOR
# define COPY_OBJ_DESCR(o_dst,o_src) \
			o_dst.oc_descr_idx = o_src.oc_descr_idx,\
			o_dst.oc_color = o_src.oc_color
#else
# define COPY_OBJ_DESCR(o_dst,o_src) o_dst.oc_descr_idx = o_src.oc_descr_idx
#endif

#ifndef OBJ_H
#define OBJ_H

/* #define obj obj_nh */ /* uncomment for SCO UNIX, which has a conflicting
			  * typedef for "obj" in <sys/types.h> */

union vptrs {
	    struct obj *v_nexthere;	/* floor location lists */
	    struct obj *v_ocontainer;	/* point back to container */
	    struct monst *v_ocarry;	/* point back to carrying monst */
};

struct obj {
	struct obj *nobj;
	union vptrs v;
#define nexthere	v.v_nexthere
#define ocontainer	v.v_ocontainer
#define ocarry		v.v_ocarry

	struct obj *cobj;	/* contents list for containers */
	unsigned o_id;
	xchar ox,oy;
	int otyp;		/* object class number */
	unsigned owt;	/* object weight */
	long quan;		/* number of items */

	schar spe;		/* quality of weapon, armor or ring (+ or -)
				   number of charges for wand ( >= -1 )
				   marks your eggs, spinach tins
				   royal coffers for a court ( == 2)
				   tells which fruit a fruit is
				   special for uball and amulet
				   do not decay for corpses
				   historic and gender for statues */
#define STATUE_HISTORIC 0x01
#define STATUE_MALE     0x02
#define STATUE_FEMALE   0x04
	char	oclass;		/* object class */
	char	invlet;		/* designation in inventory */
	int		oartifact;	/* artifact array index */
	schar 	altmode; 	/* alternate modes - eg. SMG, double Lightsaber */
				/* WP_MODEs are in decreasing speed */
#define WP_MODE_AUTO	0	/* Max firing speed */
#define WP_MODE_BURST	1	/* 1/3 of max rate */
#define WP_MODE_SINGLE 	2	/* Single shot */

	xchar where;		/* where the object thinks it is */
#define OBJ_FREE	0		/* object not attached to anything */
#define OBJ_FLOOR	1		/* object on floor */
#define OBJ_CONTAINED	2		/* object in a container */
#define OBJ_INVENT	3		/* object in the hero's inventory */
#define OBJ_MINVENT	4		/* object in a monster inventory */
#define OBJ_MIGRATING	5		/* object sent off to another level */
#define OBJ_BURIED	6		/* object buried */
#define OBJ_ONBILL	7		/* object on shk bill */
#define OBJ_MAGIC_CHEST	8		/* object in shared magic chest */
#define NOBJ_STATES	9
	xchar timed;		/* # of fuses (timers) attached to this obj */

	Bitfield(cursed,1);
	Bitfield(blessed,1);
	Bitfield(unpaid,1);	/* on some bill */
	Bitfield(no_charge,1);	/* if shk shouldn't charge for this */
	Bitfield(known,1);	/* exact nature known */
	Bitfield(dknown,1);	/* color or text known */
	Bitfield(bknown,1);	/* blessing or curse known */
	Bitfield(rknown,1);	/* rustproof or not known */
	Bitfield(sknown,1);	/* stolen or not known */

	Bitfield(oeroded,2);	/* rusted/burnt/vaporized weapon/armor */
	Bitfield(oeroded2,2);	/* corroded/rotted/fractured weapon/armor */
#define greatest_erosion(otmp) (int)((otmp)->oeroded > (otmp)->oeroded2 ? (otmp)->oeroded : (otmp)->oeroded2)
#define MAX_ERODE 3
#define orotten oeroded		/* rotten food */
#define odiluted oeroded	/* diluted potions */
#define norevive oeroded2
	Bitfield(oerodeproof,1); /* erodeproof weapon/armor */
	Bitfield(olocked,1);	/* object is locked */
#define oarmed olocked
#define odrained olocked	/* drained corpse */
	Bitfield(obroken,1);	/* lock has been broken */
#define ohaluengr obroken	/* engraving on ring isn't a ward */
#define odebone obroken		/* corpse has been de-boned */
	Bitfield(otrapped,1);	/* container is trapped */
				/* or accidental tripped rolling boulder trap */

	Bitfield(recharged,3);	/* number of times it's been recharged */
#define ostriking recharged	/* extra whip heads striking (imposes cap of +7) */
	Bitfield(lamplit,1);	/* a light-source -- can be lit */
#ifdef INVISIBLE_OBJECTS
	Bitfield(oinvis,1);	/* invisible */
#endif
	Bitfield(greased,1);	/* covered with grease */
	Bitfield(oattached,2);	/* obj struct has special attachment */
#define OATTACHED_NOTHING 0
#define OATTACHED_MONST   1	/* monst struct in oextra */
#define OATTACHED_M_ID    2	/* monst id in oextra */
#define OATTACHED_UNUSED3 3

	Bitfield(in_use,1);	/* for magic items before useup items */
	Bitfield(bypass,1);	/* mark this as an object to be skipped by bhito() */
	Bitfield(lifted,1); /* dipped in potion of levitation */
	Bitfield(lightened,1);/* dipped in potion of enlightenment */
	Bitfield(shopOwned,1);	/* owned by a shopkeeper */
	Bitfield(ostolen,1); 	/* was removed from a shop without being sold */
    Bitfield(was_thrown,1); /* for pickup_thrown */
	/* 0 free bits */
	Bitfield(fromsink,1);
	Bitfield(yours,1);	/* obj is yours (eg. thrown by you) */
	Bitfield(masters,1);	/* obj is given by a monster's master, it will not drop it */
	Bitfield(objsize,3);	/* 0-7 */
	Bitfield(obj_material,5);
//define LIQUID		1	/* currently only for venom */
//define WAX			2
//define VEGGY		3	/* foodstuffs */
//define FLESH		4	/*   ditto    */
//define PAPER		5
//define CLOTH		6
//define LEATHER		7
//define WOOD		8
//define BONE		9
//define DRAGON_HIDE	10	/* not leather! */
//define IRON		11	/* Fe - includes steel */
//define METAL		12	/* Sn, &c. */
//define COPPER		13	/* Cu - includes brass and bronze*/
//define SILVER		14	/* Ag */
//define GOLD		15	/* Au */
//define PLATINUM	16	/* Pt */
//define MITHRIL		17
//define PLASTIC		18
//define GLASS		19
//define GEMSTONE	20
//define MINERAL		21
	/* 21 free bits in this field, I think -CM */
	
	long bodytypeflag;	/* MB tag(s) this item goes with. */
	int	corpsenm;	/* type of corpse is mons[corpsenm] */
					/* Class of mask */
#define leashmon	corpsenm	/* gets m_id of attached pet */
#define spestudied	corpsenm	/* # of times a spellbook has been studied */
//define fromsink  corpsenm	/* a potion from a sink */
#define opoisonchrgs corpsenm	/* number of poison doses left */
#ifdef RECORD_ACHIEVE
#define record_achieve_special corpsenm
#endif
	
	int opoisoned; /* poisons smeared on the weapon*/
#define OPOISON_NONE	0x00
#define OPOISON_BASIC	0x01 /* Deadly Poison */
#define OPOISON_FILTH	0x02 /* Deadly Sickness */
#define OPOISON_SLEEP	0x04 /* Sleeping Poison */
#define OPOISON_BLIND	0x08 /* Blinding Poison */
#define OPOISON_PARAL	0x10 /* Paralysis Poison */
#define OPOISON_AMNES	0x20 /* Amnesia Poison */
#define OPOISON_ACID	0x40 /* Acid coating */
#define OPOISON_SILVER	0x80 /* Silver coating */

	long	oproperties;/* special properties */
#define OPROP_NONE		0x0000000000000000
#define OPROP_FIRE		0x0000000000000001
#define OPROP_COLD		0x0000000000000002
#define OPROP_WOOL		(OPROP_COLD|OPROP_FIRE)
#define OPROP_ELEC		0x0000000000000004
#define OPROP_ACID		0x0000000000000008
#define OPROP_MAGC		0x0000000000000010
#define OPROP_ANAR		0x0000000000000020
#define OPROP_CONC		0x0000000000000040
#define OPROP_AXIO		0x0000000000000080
#define OPROP_FIREW		0x0000000000000100
#define OPROP_COLDW		0x0000000000000200
#define OPROP_ELECW		0x0000000000000400
#define OPROP_ACIDW		0x0000000000000800
#define OPROP_MAGCW		0x0000000000001000
#define OPROP_ANARW		0x0000000000002000
#define OPROP_CONCW		0x0000000000004000
#define OPROP_AXIOW		0x0000000000008000
#define OPROP_LESSW		0x0000000000010000

	unsigned oeaten;	/* nutrition left in food, if partly eaten */
	long age;		/* creation date */

	uchar onamelth;		/* length of name (following oxlth) */
	short oxlth;		/* length of following data */
	/* in order to prevent alignment problems oextra should
	   be (or follow) a long int */
	long owornmask;
	long oward;
			/*Records the warding sign of spellbooks. */
			/*Records the warding sign of scrolls of ward. */
			/*Records the warding sign of rings. */
			/*Records runes for wooden weapons */
			
	long ovar1;		/* extra variable. Specifies: */
			/*Records the contents of Books of Secrets*/
			/*Records the tatteredness level of droven cloaks. */
			/*Records the cracked level of masks. */
			/*Records special features for weapons. */
			/* 	Records moon phase for moon axes */
			/* 	Records theft type for stealing artifacts (reaver (scimitar) and avarice (shortsword) */
			/* 	Records remaining ammo for blasters and force pikes */
			/* 	Records the hilt-type for lightsabers */
			/* 	Records the ema of damage taken for gloves of the berserker */
			
#define ECLIPSE_MOON	0
#define CRESCENT_MOON	1
#define HALF_MOON		2
#define GIBBOUS_MOON	3
#define FULL_MOON	 	4
			/* Rings: specifies engraving on certain rings */
			/* Cloaks: Droven: Tattered level.  */
			/* Acid venom: nonstandard damage amount */
			/* Corpses: rummor */
			/* Rocks: rummor */
			/* Masks: fracturing level */

	schar gifted; /*gifted is of type aligntyp.  For some reson aligntyp isn't being seen at compile*/
	
	struct mask_properties *mp;

	long oextra[1];		/* used for name of ordinary objects - length
				   is flexible; amount for tmp gold objects.  Must be last? */
};

//Useful items (ovar1 flags for planned cloak of useful items)
#define USE_DAGGER	0x0000001L
#define TWO_DAGGER	0x0000002L
#define USE_LANTERN	0x0000004L
#define TWO_LANTERN	0x0000008L
#define USE_MIRROR	0x0000010L
#define TWO_MIRROR	0x0000020L
#define USE_POLE	0x0000040L
#define USE_SACK	0x0000100L
#define USE_GOLD	0x0000400L
#define USE_CHEST	0x0001000L
#define USE_DOOR	0x0004000L
#define USE_GEMS	0x0010000L
#define USE_PONY	0x0040000L
#define USE_PIT		0x0100000L
#define USE_POTION	0x0400000L
#define USE_LAND	0x1000000L
#define USE_DOGS	0x4000000L
//Useful items 2
#define USE_SCROLL	0x0000001
#define USE_WINDOW	0x0000004
#define USE_RAM		0x0000010
#define USE_STARS	0x0000040
// #define USE_	0x0000100
// #define USE_	0x0000400
// #define USE_	0x0001000

#define SPEC_FIRE		0x0000001L
#define SPEC_COLD		0x0000002L
#define SPEC_ELEC		0x0000004L
#define SPEC_ACID		0x0000008L
#define SPEC_WILT		0x0000010L
#define SPEC_RADI		0x0000020L
#define SPEC_BONUS		0x0000040L
#define SPEC_DESTRUCTOR	0x0000080L
#define SPEC_MARIONETTE	0x0000100L

// flags for use with readobjnam()
#define WISH_QUIET      0x0000001L
#define WISH_WIZARD     0x0000002L
#define WISH_VERBOSE	0x0000004L
#define WISH_ARTALLOW	0x0000008L

#define WISH_NOTHING	0x0000100L
#define WISH_SUCCESS	0x0000200L
#define WISH_FAILURE	0x0000400L
#define WISH_DENIED		0x0000800L




#define newobj(xl)	(struct obj *)alloc((unsigned)(xl) + sizeof(struct obj))
#define ONAME(otmp)	(((char *)(otmp)->oextra) + (otmp)->oxlth)

/* Weapons and weapon-tools */
/* KMH -- now based on skill categories.  Formerly:
 *	#define is_sword(otmp)	(otmp->oclass == WEAPON_CLASS && \
 *			 objects[otmp->otyp].oc_wepcat == WEP_SWORD)
 *	#define is_blade(otmp)	(otmp->oclass == WEAPON_CLASS && \
 *			 (objects[otmp->otyp].oc_wepcat == WEP_BLADE || \
 *			  objects[otmp->otyp].oc_wepcat == WEP_SWORD))
 *	#define is_weptool(o)	((o)->oclass == TOOL_CLASS && \
 *			 objects[(o)->otyp].oc_weptool)
 *	#define is_multigen(otyp) (otyp <= SHURIKEN)
 *	#define is_poisonable(otyp) (otyp <= BEC_DE_CORBIN)
 */
#define artitypematch(a, o) (( (a)->otyp ) == BEAMSWORD ? ((o)->otyp==BROADSWORD) : \
							( (a)->otyp ) == UNIVERSAL_KEY ? ((o)->otyp==SKELETON_KEY) : \
							( (a)->otyp ) == ROUNDSHIELD ? ((o)->otyp==DWARVISH_ROUNDSHIELD) : \
							(a)->otyp == (o)->otyp)
#define is_blade(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= P_DAGGER && \
			 objects[otmp->otyp].oc_skill <= P_SABER)
#define is_knife(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 (objects[otmp->otyp].oc_skill == P_DAGGER || \
			 objects[otmp->otyp].oc_skill == P_KNIFE))
#define is_axe(otmp)	((otmp->oclass == WEAPON_CLASS || \
			 otmp->oclass == TOOL_CLASS) && \
			 objects[otmp->otyp].oc_skill == P_AXE)
#define is_pick(otmp)	(((otmp->oclass == WEAPON_CLASS || \
			 otmp->oclass == TOOL_CLASS) && \
			 (objects[otmp->otyp].oc_skill == P_PICK_AXE)) || \
			  arti_digs(otmp))
#define is_sword(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= P_SHORT_SWORD && \
			 objects[otmp->otyp].oc_skill <= P_SABER)
#define is_pole(otmp)	((otmp->oclass == WEAPON_CLASS || \
			otmp->oclass == TOOL_CLASS) && \
			 (objects[otmp->otyp].oc_skill == P_POLEARMS || \
			  objects[otmp->otyp].oc_skill == P_LANCE || \
			  otmp->otyp==AKLYS || \
			  otmp->oartifact==ART_SOL_VALTIVA || \
			  otmp->oartifact==ART_SHADOWLOCK || \
			  (otmp->oartifact==ART_PEN_OF_THE_VOID && otmp->ovar1&SEAL_MARIONETTE ) \
			 ))
#define is_spear(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= P_SPEAR && \
			 objects[otmp->otyp].oc_skill <= P_JAVELIN)
#define is_farm(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill == P_HARVEST)
#define is_launcher(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= P_BOW && \
			 objects[otmp->otyp].oc_skill <= P_CROSSBOW)
#define is_ammo(otmp)	((otmp->oclass == WEAPON_CLASS || \
			 otmp->oclass == GEM_CLASS) && \
			 objects[otmp->otyp].oc_skill >= -P_CROSSBOW && \
			 objects[otmp->otyp].oc_skill <= -P_BOW)
#define is_grenade(otmp)	(is_ammo(otmp) && \
			 	 objects[(otmp)->otyp].w_ammotyp == WP_GRENADE)
#define ammo_and_launcher(otmp,ltmp) \
			 (is_ammo(otmp) && (ltmp) && (\
			  (\
			   (ltmp->otyp == BFG) ||\
			   (ltmp->oartifact == ART_PEN_OF_THE_VOID && ltmp->ovar1&SEAL_EVE) ||\
			   (ltmp->otyp == MASS_SHADOW_PISTOL && (otmp->otyp == ltmp->cobj->otyp)) ||\
			   (otmp->objsize == (ltmp)->objsize || objects[(ltmp)->otyp].oc_skill == P_SLING) &&\
			    (objects[(otmp)->otyp].w_ammotyp & objects[(ltmp)->otyp].w_ammotyp) && \
			    (objects[(otmp)->otyp].oc_skill == -objects[(ltmp)->otyp].oc_skill))\
			   ))
#define is_missile(otmp)	((otmp->oclass == WEAPON_CLASS || \
			 otmp->oclass == TOOL_CLASS || otmp->oclass == GEM_CLASS) && \
			 ((objects[otmp->otyp].oc_skill >= -P_BOOMERANG && \
				 objects[otmp->otyp].oc_skill <= -P_DART) ||\
				 objects[otmp->otyp].oc_skill == -P_SLING)\
			 )
#define is_weptool(o)	((o)->oclass == TOOL_CLASS && \
			 objects[(o)->otyp].oc_skill != P_NONE)
#define is_instrument(o)	((o)->otyp >= WOODEN_FLUTE && \
			 (o)->otyp <= DRUM_OF_EARTHQUAKE)
#define bimanual(otmp,ptr)	(otmp && (otmp->oclass == WEAPON_CLASS || \
			 otmp->oclass == TOOL_CLASS) && \
			 ptr != &mons[PM_THRONE_ARCHON] && \
			 ptr != &mons[PM_BASTARD_OF_THE_BOREAL_VALLEY] && \
			 (otmp->oartifact == ART_HOLY_MOONLIGHT_SWORD && otmp->lamplit ?\
			  (objects[otmp->otyp].oc_bimanual ? \
				((ptr)->msize - otmp->objsize - 2 <  2):\
				((ptr)->msize - otmp->objsize - 2 < -1)) :\
			  (otmp->oartifact == ART_FRIEDE_S_SCYTHE ? \
				((ptr)->msize - otmp->objsize < -1) :\
				  (objects[otmp->otyp].oc_bimanual ? \
					((ptr)->msize - otmp->objsize <  2):\
					((ptr)->msize - otmp->objsize < -1))\
			   )\
			))
#define is_lightsaber(otmp) ((otmp)->otyp == LIGHTSABER || \
							 (otmp)->otyp == KAMEREL_VAJRA || \
							 (otmp)->otyp == BEAMSWORD || \
							 (otmp)->otyp == DOUBLE_LIGHTSABER)
#define is_multigen(otmp)	((otmp->oclass == WEAPON_CLASS && \
			 objects[otmp->otyp].oc_skill >= -P_SHURIKEN && \
			 objects[otmp->otyp].oc_skill <= -P_BOW))
// define is_poisonable(otmp)	(otmp->oclass == WEAPON_CLASS && \
			 // objects[otmp->otyp].oc_skill >= -P_SHURIKEN && \
			 // objects[otmp->otyp].oc_skill <= -P_BOW)
//#ifdef FIREARMS
#define is_unpoisonable_firearm_ammo(otmp)	\
			 (is_bullet(otmp) || (otmp)->otyp == STICK_OF_DYNAMITE)
//#else
//#define is_unpoisonable_firearm_ammo(otmp)	0
//#endif
#define is_poisonable(otmp)	((otmp->oclass == WEAPON_CLASS || is_weptool(otmp)) && \
			!is_launcher(otmp) &&\
			!is_unpoisonable_firearm_ammo(otmp) &&\
			objects[otmp->otyp].oc_dir &&\
			objects[otmp->otyp].oc_dir != WHACK)
#define uslinging()	(uwep && objects[uwep->otyp].oc_skill == P_SLING)
#define is_bludgeon(otmp)	(otmp->oclass == SPBOOK_CLASS || \
			otmp->oclass == WAND_CLASS || \
			(objects[otmp->otyp].oc_dir & WHACK)) //Whack == 1
#define is_stabbing(otmp)	(otmp->oclass != SPBOOK_CLASS && \
			otmp->oclass != WAND_CLASS && \
			(objects[otmp->otyp].oc_dir & PIERCE)) //Pierce == 2
#define is_slashing(otmp)	(otmp->oclass != SPBOOK_CLASS && \
			otmp->oclass != WAND_CLASS && \
			(objects[otmp->otyp].oc_dir & SLASH)) //Slash == 4
#define is_blasting(otmp)	(otmp->oclass != SPBOOK_CLASS && \
			otmp->oclass != WAND_CLASS && \
			(objects[otmp->otyp].oc_dir & EXPLOSION))
//#ifdef FIREARMS
#define is_blaster(otmp) \
			((otmp)->oclass == WEAPON_CLASS && \
			 objects[(otmp)->otyp].w_ammotyp == WP_BLASTER && \
			 objects[(otmp)->otyp].oc_skill == P_FIREARM)
#define is_firearm(otmp) \
			((otmp)->oclass == WEAPON_CLASS && \
			 objects[(otmp)->otyp].oc_skill == P_FIREARM)
#define is_bullet(otmp)	((otmp)->oclass == WEAPON_CLASS && \
			 objects[(otmp)->otyp].oc_skill == -P_FIREARM)
//#endif

/* Armor */
#define is_shield(otmp) (otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_SHIELD)
#define is_helmet(otmp) (otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_HELM)
#define is_boots(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_BOOTS)
#define is_gloves(otmp) (otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_GLOVES)
#define is_cloak(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_CLOAK)
#define is_shirt(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_SHIRT)
#define is_suit(otmp)	(otmp->oclass == ARMOR_CLASS && \
			 objects[otmp->otyp].oc_armcat == ARM_SUIT)
#define is_harmonium_armor(otmp)	((otmp)->otyp == HARMONIUM_HELM || (otmp)->otyp == HARMONIUM_PLATE\
								|| (otmp)->otyp == HARMONIUM_SCALE_MAIL || (otmp)->otyp == HARMONIUM_GAUNTLETS\
								|| (otmp)->otyp == HARMONIUM_BOOTS)
#define is_light_armor(otmp)	((otmp)->otyp == DWARVISH_MITHRIL_COAT || (otmp)->otyp == ELVEN_MITHRIL_COAT || \
			(otmp)->otyp == JUMPSUIT || (otmp)->otyp == LEATHER_JACKET || (otmp)->otyp == ELVEN_TOGA || \
			(otmp)->otyp == BLACK_DRESS)
#define is_medium_armor(otmp)	((otmp)->otyp == BRONZE_PLATE_MAIL || (otmp)->otyp == DROVEN_CHAIN_MAIL || \
			(otmp)->otyp == CHAIN_MAIL || (otmp)->otyp == SCALE_MAIL || (otmp)->otyp == STUDDED_LEATHER_ARMOR || \
			(otmp)->otyp == LEATHER_ARMOR || (otmp)->otyp == BANDED_MAIL || (otmp)->otyp == NOBLE_S_DRESS || \
			(otmp)->otyp == HARMONIUM_SCALE_MAIL || \
			(otmp)->otyp == PLASTEEL_ARMOR || (otmp)->otyp == HIGH_ELVEN_PLATE || Is_dragon_mail(otmp))
#define is_elven_armor(otmp)	((otmp)->otyp == ELVEN_HELM\
				|| (otmp)->otyp == HIGH_ELVEN_HELM\
				|| (otmp)->otyp == HIGH_ELVEN_GAUNTLETS\
				|| (otmp)->otyp == HIGH_ELVEN_PLATE\
				|| (otmp)->otyp == ELVEN_MITHRIL_COAT\
				|| (otmp)->otyp == ELVEN_CLOAK\
				|| (otmp)->otyp == ELVEN_SHIELD\
				|| (otmp)->otyp == ELVEN_TOGA\
				|| (otmp)->otyp == ELVEN_BOOTS)
#define is_orcish_armor(otmp)	((otmp)->otyp == ORCISH_HELM\
				|| (otmp)->otyp == ORCISH_CHAIN_MAIL\
				|| (otmp)->otyp == ORCISH_RING_MAIL\
				|| (otmp)->otyp == ORCISH_CLOAK\
				|| (otmp)->otyp == URUK_HAI_SHIELD\
				|| (otmp)->otyp == ORCISH_SHIELD)
#define is_dwarvish_armor(otmp)	((otmp)->otyp == DWARVISH_HELM\
				|| (otmp)->otyp == DWARVISH_MITHRIL_COAT\
				|| (otmp)->otyp == DWARVISH_CLOAK\
				|| (otmp)->otyp == DWARVISH_ROUNDSHIELD)
#define is_gnomish_armor(otmp)	((otmp)->otyp == GNOMISH_POINTY_HAT)

#define is_twoweapable_artifact(otmp) ((otmp)->oartifact == ART_STING\
				|| (otmp)->oartifact == ART_ORCRIST\
				|| (otmp)->oartifact == ART_GRIMTOOTH\
				|| (otmp)->oartifact == ART_CARNWENNAN\
				|| (otmp)->oartifact == ART_CLAIDEAMH\
				|| (otmp)->oartifact == ART_DRAGONLANCE\
				|| (otmp)->oartifact == ART_DEMONBANE\
				|| (otmp)->oartifact == ART_NODENSFORK\
				|| (otmp)->oartifact == ART_GAIA_S_FATE\
				|| (otmp)->oartifact == ART_WEREBANE\
				|| (otmp)->oartifact == ART_GIANTSLAYER\
				|| (otmp)->oartifact == ART_VAMPIRE_KILLER\
				|| (otmp)->oartifact == ART_KINGSLAYER\
				|| (otmp)->oartifact == ART_PEACE_KEEPER\
				|| (otmp)->oartifact == ART_OGRESMASHER\
				|| (otmp)->oartifact == ART_TROLLSBANE\
				|| (otmp)->oartifact == ART_PEN_OF_THE_VOID\
				|| ((otmp)->oartifact == ART_CLARENT && uwep && uwep->oartifact==ART_EXCALIBUR)\
				|| ((otmp)->oartifact == ART_BLADE_DANCER_S_DAGGER && uwep && uwep->oartifact==ART_BLADE_SINGER_S_SPEAR)\
				|| ((otmp)->oartifact == ART_BLADE_DANCER_S_DAGGER && uwep && uwep->oartifact==ART_SODE_NO_SHIRAYUKI)\
				|| ((otmp)->oartifact == ART_FRIEDE_S_SCYTHE && uwep && uwep->oartifact==ART_PROFANED_GREATSCYTHE)\
				|| ((otmp)->oartifact == ART_FRIEDE_S_SCYTHE && uwep && uwep->oartifact==ART_LIFEHUNT_SCYTHE)\
				|| ((otmp)->oartifact == ART_MJOLLNIR && Role_if(PM_VALKYRIE))\
				|| ((otmp)->oartifact == ART_CLEAVER && Role_if(PM_BARBARIAN))\
				|| ((otmp)->oartifact == ART_KIKU_ICHIMONJI && Role_if(PM_SAMURAI))\
				|| ((otmp)->oartifact == ART_SNICKERSNEE && (Role_if(PM_SAMURAI) || Role_if(PM_TOURIST) ))\
				|| ((otmp)->oartifact == ART_MAGICBANE && Role_if(PM_WIZARD)))
				
/* Eggs and other food */
#define MAX_EGG_HATCH_TIME 200	/* longest an egg can remain unhatched */
#define stale_egg(egg)	((monstermoves - (egg)->age) > (2*MAX_EGG_HATCH_TIME))
#define ofood(o) ((o)->otyp == CORPSE || (o)->otyp == EGG || (o)->otyp == TIN)
#define polyfodder(obj) (ofood(obj) && \
			 pm_to_cham((obj)->corpsenm) != CHAM_ORDINARY)
#define mlevelgain(obj) (ofood(obj) && (obj)->corpsenm == PM_WRAITH)
#define mhealup(obj)	(ofood(obj) && (obj)->corpsenm == PM_NURSE)
#define drainlevel(corpse) (mons[(corpse)->corpsenm].cnutrit*4/5)

/* Containers */
#define carried(o)	((o)->where == OBJ_INVENT)
#define mcarried(o)	((o)->where == OBJ_MINVENT)
#define Has_contents(o) (/* (Is_container(o) || (o)->otyp == STATUE) && */ \
			 (o)->cobj != (struct obj *)0)
#define Is_container(o) ((o)->otyp >= BOX && (o)->otyp <= BAG_OF_TRICKS/*DISTRESSED_PRINCESS*/)
#define Is_box(otmp)	(otmp->otyp == BOX || otmp->otyp == CHEST || otmp->otyp == MAGIC_CHEST)
#define Is_mbag(otmp)	(otmp->otyp == BAG_OF_HOLDING || \
			 otmp->otyp == BAG_OF_TRICKS)

/* dragon gear */
#define Is_dragon_scales(obj)	((obj)->otyp >= GRAY_DRAGON_SCALES && \
				 (obj)->otyp <= YELLOW_DRAGON_SCALES)
#define Is_dragon_mail(obj)	((obj)->otyp >= GRAY_DRAGON_SCALE_MAIL && \
				 (obj)->otyp <= YELLOW_DRAGON_SCALE_MAIL)
#define Is_dragon_shield(obj) ((obj)->otyp >= GRAY_DRAGON_SCALE_SHIELD && \
							   (obj)->otyp <= YELLOW_DRAGON_SCALE_SHIELD)
#define Is_dragon_armor(obj)	(Is_dragon_scales(obj) || Is_dragon_mail(obj) || Is_dragon_shield(obj))
#define Dragon_scales_to_pm(obj) &mons[PM_GRAY_DRAGON + (obj)->otyp \
				       - GRAY_DRAGON_SCALES]
#define Dragon_mail_to_pm(obj)	&mons[PM_GRAY_DRAGON + (obj)->otyp \
				      - GRAY_DRAGON_SCALE_MAIL]
#define Dragon_shield_to_pm(obj)	&mons[PM_GRAY_DRAGON + (obj)->otyp \
				      - GRAY_DRAGON_SCALE_SHIELD]
#define Dragon_to_scales(pm)	(GRAY_DRAGON_SCALES + (pm - mons))
#define Have_same_dragon_armor_and_shield	(Is_dragon_shield(uarms) && ((Is_dragon_scales(uarm) && Dragon_scales_to_pm(uarm) == Dragon_shield_to_pm(uarms)) ||\
													 (Is_dragon_mail(uarm) && Dragon_mail_to_pm(uarm) == Dragon_shield_to_pm(uarms)) ||\
												(uarm->oartifact == ART_DRAGON_PLATE && (\
													Dragon_shield_to_pm(uarms) == &mons[PM_SILVER_DRAGON] ||\
													Dragon_shield_to_pm(uarms) == &mons[PM_BLACK_DRAGON] ||\
													Dragon_shield_to_pm(uarms) == &mons[PM_BLUE_DRAGON] ||\
													Dragon_shield_to_pm(uarms) == &mons[PM_RED_DRAGON] ||\
													Dragon_shield_to_pm(uarms) == &mons[PM_GRAY_DRAGON] ||\
													Dragon_shield_to_pm(uarms) == &mons[PM_ORANGE_DRAGON]\
												)) ||\
												(uarm->oartifact == ART_CHROMATIC_DRAGON_SCALES && (\
													Dragon_shield_to_pm(uarms) == &mons[PM_BLACK_DRAGON] ||\
													Dragon_shield_to_pm(uarms) == &mons[PM_RED_DRAGON] ||\
													Dragon_shield_to_pm(uarms) == &mons[PM_BLUE_DRAGON] ||\
													Dragon_shield_to_pm(uarms) == &mons[PM_WHITE_DRAGON] ||\
													Dragon_shield_to_pm(uarms) == &mons[PM_YELLOW_DRAGON] ||\
													Dragon_shield_to_pm(uarms) == &mons[PM_GREEN_DRAGON]\
												))\
											))

/* Elven gear */
#define is_elven_weapon(otmp)	((otmp)->otyp == ELVEN_ARROW\
				|| (otmp)->otyp == ELVEN_SPEAR\
				|| (otmp)->otyp == ELVEN_DAGGER\
				|| (otmp)->otyp == ELVEN_SHORT_SWORD\
				|| (otmp)->otyp == ELVEN_BROADSWORD\
				|| (otmp)->otyp == ELVEN_BOW)
#define is_elven_obj(otmp)	(is_elven_armor(otmp) || is_elven_weapon(otmp))

/* Orcish gear */
#define is_orcish_obj(otmp)	(is_orcish_armor(otmp)\
				|| (otmp)->otyp == ORCISH_ARROW\
				|| (otmp)->otyp == ORCISH_SPEAR\
				|| (otmp)->otyp == ORCISH_DAGGER\
				|| (otmp)->otyp == ORCISH_SHORT_SWORD\
				|| (otmp)->otyp == ORCISH_BOW)

/* Dwarvish gear */
#define is_dwarvish_obj(otmp)	(is_dwarvish_armor(otmp)\
				|| (otmp)->otyp == DWARVISH_SPEAR\
				|| (otmp)->otyp == DWARVISH_SHORT_SWORD\
				|| (otmp)->otyp == DWARVISH_MATTOCK)

/* Gnomish gear */
#define is_gnomish_obj(otmp)	(is_gnomish_armor(otmp))

/* Light sources */
#define Is_candle(otmp) (otmp->otyp == TALLOW_CANDLE || \
			 otmp->otyp == WAX_CANDLE || \
			 otmp->otyp == CANDLE_OF_INVOCATION)
#define MAX_OIL_IN_FLASK 400	/* maximum amount of oil in a potion of oil */
#define Is_darklight_source(otmp) ((otmp)->otyp == SHADOWLANDER_S_TORCH || \
			 (otmp)->otyp == CHUNK_OF_FOSSIL_DARK ||\
			 (is_lightsaber(otmp) && otmp->cobj && otmp->cobj->otyp == CHUNK_OF_FOSSIL_DARK))

/* MAGIC_LAMP intentionally excluded below */
/* age field of this is relative age rather than absolute */
#define age_is_relative(otmp)	((otmp)->otyp == BRASS_LANTERN\
				|| (otmp)->otyp == OIL_LAMP\
				|| (otmp)->otyp == DWARVISH_HELM\
				|| (otmp)->otyp == LIGHTSABER\
				|| (otmp)->otyp == BEAMSWORD\
				|| (otmp)->otyp == DOUBLE_LIGHTSABER\
				|| (otmp)->otyp == GNOMISH_POINTY_HAT\
				|| (otmp)->otyp == CANDELABRUM_OF_INVOCATION\
				|| (otmp)->otyp == TALLOW_CANDLE\
				|| (otmp)->otyp == WAX_CANDLE\
				|| (otmp)->otyp == POT_OIL)
/* object can be ignited */
#define ignitable(otmp)	((otmp)->otyp == BRASS_LANTERN\
				|| (otmp)->otyp == OIL_LAMP\
 				|| (otmp)->otyp == DWARVISH_HELM\
 				|| (otmp)->otyp == GNOMISH_POINTY_HAT\
				|| (otmp)->otyp == CANDELABRUM_OF_INVOCATION\
				|| (otmp)->otyp == TALLOW_CANDLE\
				|| (otmp)->otyp == WAX_CANDLE\
				|| (otmp)->otyp == POT_OIL)

/* special stones */
#define is_graystone(obj)	((obj)->otyp == LUCKSTONE || \
				 (obj)->otyp == LOADSTONE || \
				 (obj)->otyp == FLINT     || \
				 (obj)->otyp == TOUCHSTONE)

/* spirit related */
#define is_berithable(otmp)	(otmp->otyp == SADDLE\
				|| otmp->otyp == SABER\
				|| otmp->otyp == LONG_SWORD\
				|| otmp->otyp == BOW\
				|| otmp->otyp == LANCE\
				|| (OBJ_DESCR(objects[otmp->otyp]) != (char *)0 && !strncmp(OBJ_DESCR(objects[otmp->otyp]), "riding ", 7))\
				)
#define is_chupodible(otmp) (your_race(&mons[otmp->corpsenm]))

#define is_unholy(otmp)		((otmp)->oartifact == ART_STORMBRINGER || (otmp)->cursed)

/* material */
#define is_flimsy(otmp)		((otmp)->obj_material <= LEATHER)
#define is_wood(otmp)		((otmp)->obj_material == WOOD)
#define is_veggy(otmp)		((otmp)->obj_material == VEGGY)
#define is_flesh(otmp)		((otmp)->obj_material == FLESH)
#define is_paper(otmp)		((otmp)->obj_material == PAPER)
#define is_metal(otmp)		((otmp)->obj_material == METAL || \
							 (otmp)->obj_material == COPPER || \
							 (otmp)->obj_material == SILVER || \
							 (otmp)->obj_material == IRON)

/* misc */
#define is_boulder(otmp)		((otmp)->otyp == BOULDER || (otmp)->otyp == MASSIVE_STONE_CRATE || ((otmp)->otyp == STATUE && opaque(&mons[(otmp)->corpsenm])))

/* helpers, simple enough to be macros */
#define is_plural(o)	((o)->quan > 1 || \
			 (o)->oartifact == ART_EYES_OF_THE_OVERWORLD)

/* Flags for get_obj_location(). */
#define CONTAINED_TOO	0x1
#define BURIED_TOO	0x2

#endif /* OBJ_H */
