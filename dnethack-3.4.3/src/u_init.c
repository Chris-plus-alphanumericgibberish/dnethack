/*	SCCS Id: @(#)u_init.c	3.4	2002/10/22	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"

struct trobj {
	short trotyp;
	schar trspe;
	char trclass;
	Bitfield(trquan,8);
	Bitfield(trbless,2);
};

STATIC_DCL void FDECL(ini_inv, (struct trobj *));
STATIC_DCL void FDECL(knows_object,(int));
STATIC_DCL void FDECL(knows_class,(CHAR_P));
STATIC_DCL boolean FDECL(restricted_spell_discipline, (int));
STATIC_DCL void NDECL(scatter_weapons);

#define UNDEF_TYP	0
#define UNDEF_SPE	'\177'
#define UNDEF_BLESS	2

/*
 *	Initial inventory for the various roles.
 */

static struct trobj Archeologist[] = {
	/* if adventure has a name...  idea from tan@uvm-gen */
	{ BULLWHIP, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_JACKET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FEDORA, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ PICK_AXE, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ TINNING_KIT, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ TOUCHSTONE, 0, GEM_CLASS, 1, 0 },
	{ SACK, 0, TOOL_CLASS, 1, 0 },
	{ TORCH, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Anachrononaut_Hu[] = {
	{ FORCE_PIKE,  0, WEAPON_CLASS, 1, 0 },
	{ ARM_BLASTER, 0, WEAPON_CLASS, 1, 0 },
	{ HAND_BLASTER, 0, WEAPON_CLASS, 1, 0 },
	{ PLASTEEL_ARMOR, 0, ARMOR_CLASS, 1, 0 },
	{ BODYGLOVE, 0, ARMOR_CLASS, 1, 0 },
	{ PLASTEEL_HELM, 0, ARMOR_CLASS, 1, 0 },
	{ PLASTEEL_GAUNTLETS, 0, ARMOR_CLASS, 1, 0 },
	{ PLASTEEL_BOOTS, 0, ARMOR_CLASS, 1, 0 },
	{ CLOAK_OF_MAGIC_RESISTANCE, 0, ARMOR_CLASS, 1, 0 },
	{ POWER_PACK, 0, TOOL_CLASS, 5, 0 },
	{ PROTEIN_PILL, 0, FOOD_CLASS, 10, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Anachrononaut_Hlf[] = {
	{ AUTO_SHOTGUN,  3, WEAPON_CLASS, 1, 0 },
	{ SHOTGUN_SHELL, 3, WEAPON_CLASS, 100, 0 },
	{ SCALE_MAIL, 0, ARMOR_CLASS, 1, 0 },
	{ BRONZE_HELM, 0, ARMOR_CLASS, 1, 0 },
	{ GAUNTLETS, 0, ARMOR_CLASS, 1, 0 },
	{ ARMORED_BOOTS, 0, ARMOR_CLASS, 1, 0 },
	{ CLOAK_OF_PROTECTION, 3, ARMOR_CLASS, 1, 0 },
	{ BULLET_FABBER, 0, TOOL_CLASS, 1, 0 },
	{ CUTTING_LASER,  0, WEAPON_CLASS, 1, 0 },
	{ POWER_PACK, 0, TOOL_CLASS, 5, 0 },
	{ PROTEIN_PILL, 0, FOOD_CLASS, 10, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Anachrononaut_Dw[] = {
	{ HEAVY_MACHINE_GUN, 5, WEAPON_CLASS, 1, 0 },
	{ PISTOL, 1, WEAPON_CLASS, 1, 0 },
	{ SEISMIC_HAMMER, 0, WEAPON_CLASS, 1, 0 },
	{ STICK_OF_DYNAMITE, 0, TOOL_CLASS, 15, 0 },
	{ HIGH_ELVEN_PLATE, 1, ARMOR_CLASS, 1, 0 },
	{ BODYGLOVE, 0, ARMOR_CLASS, 1, 0 },
	{ HIGH_ELVEN_HELM, 1, ARMOR_CLASS, 1, 0 },
	{ HIGH_ELVEN_GAUNTLETS, 1, ARMOR_CLASS, 1, 0 },
	{ ELVEN_BOOTS, 1, ARMOR_CLASS, 1, 0 },
	{ AMULET_OF_NULLIFY_MAGIC, 0, AMULET_CLASS, 1, 0 },
	{ BULLET, 3, WEAPON_CLASS, 100, 0 },
	{ BULLET_FABBER, 0, TOOL_CLASS, 1, 0 },
	{ PROTEIN_PILL, 0, FOOD_CLASS, 10, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Anachrononaut_Inc[] = {
	{ LIGHTSABER,  3, WEAPON_CLASS, 1, 0 },
	{ ELVEN_TOGA, 1, ARMOR_CLASS, 1, 0 },
	{ BODYGLOVE, 0, ARMOR_CLASS, 1, 0 },
	{ GLOVES, 1, ARMOR_CLASS, 1, 0 },
	{ ROBE, 1, ARMOR_CLASS, 1, 0 },
	{ SEDGE_HAT, 1, ARMOR_CLASS, 1, 0 },
	{ HIGH_BOOTS, 1, ARMOR_CLASS, 1, 0 },
	{ POWER_PACK, 0, TOOL_CLASS, 5, 0 },
	{ SPE_DETECT_MONSTERS, 0, SPBOOK_CLASS, 1, 0 },
	{ SPE_JUMPING, 0, SPBOOK_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Anachrononaut_Vam[] = {
	{ SUBMACHINE_GUN, 4, WEAPON_CLASS, 1, 0 },
	{ SUBMACHINE_GUN, 4, WEAPON_CLASS, 1, 0 },
	{ VIBROBLADE,  0, WEAPON_CLASS, 1, 0 },
	{ CUTTING_LASER,  0, WEAPON_CLASS, 1, 0 },
	{ STUDDED_LEATHER_ARMOR, 1, ARMOR_CLASS, 1, 0 },
	{ BODYGLOVE, 0, ARMOR_CLASS, 1, 0 },
	{ GLOVES, 1, ARMOR_CLASS, 1, 0 },
	{ CLOAK_OF_DISPLACEMENT, 0, ARMOR_CLASS, 1, 0 },
	{ HIGH_BOOTS, 1, ARMOR_CLASS, 1, 0 },
	{ POWER_PACK, 0, TOOL_CLASS,  5, 0 },
	{ BULLET, 0, WEAPON_CLASS, 200, 0 },
	{ SILVER_BULLET, 0, WEAPON_CLASS, 50, 0 },
	{ BULLET_FABBER, 0, TOOL_CLASS, 1, 0 },
	{ FOOD_RATION, 0, TOOL_CLASS, 5, 0 },
	{ TINNING_KIT, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Anachrononaut_ShDro[] = {
	{ MASS_SHADOW_PISTOL, 4, WEAPON_CLASS, 1, 0 },
	{ BODYGLOVE, 1, ARMOR_CLASS, 1, 0 },
	{ DROVEN_CLOAK, 1, ARMOR_CLASS, 1, 0 },
	{ GAUNTLETS_OF_DEXTERITY, 1, ARMOR_CLASS, 1, 0 },
	{ ELVEN_BOOTS, 1, ARMOR_CLASS, 1, 0 },
	{ POWER_PACK, 0, TOOL_CLASS,  5, 0 },
	{ PROTEIN_PILL, 0, FOOD_CLASS, 10, 0 },
	{ SPE_FORCE_BOLT, 0, SPBOOK_CLASS, 1, 0 },
	{ SPE_SLOW_MONSTER, 0, SPBOOK_CLASS, 1, 0 },
	{ SPE_PROTECTION, 0, SPBOOK_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Anachrononaut_Dro[] = {
	{ SNIPER_RIFLE, 4, WEAPON_CLASS, 1, 0 },
	{ VIBROBLADE,  0, WEAPON_CLASS, 1, 0 },
	{ CUTTING_LASER,  0, WEAPON_CLASS, 1, 0 },
	{ PLASTEEL_ARMOR, 0, ARMOR_CLASS, 1, 0 },
	{ BODYGLOVE, 0, ARMOR_CLASS, 1, 0 },
	{ PLASTEEL_HELM, 0, ARMOR_CLASS, 1, 0 },
	{ ORIHALCYON_GAUNTLETS, 1, ARMOR_CLASS, 1, 0 },
	{ CLOAK_OF_INVISIBILITY, 0, ARMOR_CLASS, 1, 0 },
	{ ELVEN_BOOTS, 1, ARMOR_CLASS, 1, 0 },
	{ POWER_PACK, 0, TOOL_CLASS,  5, 0 },
	{ BULLET, 0, WEAPON_CLASS, 60, 0 },
	{ SILVER_BULLET, 0, WEAPON_CLASS, 30, 0 },
	{ BULLET_FABBER, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ PROTEIN_PILL, 0, FOOD_CLASS, 10, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Anachrononaut_Elf[] = {
	{ RAYGUN,  0, WEAPON_CLASS, 1, 0 },
	{ SENSOR_PACK,  25, WEAPON_CLASS, 1, 0 },
	{ JUMPSUIT, 2, ARMOR_CLASS, 1, 0 },
	{ BODYGLOVE, 0, ARMOR_CLASS, 1, 0 },
	{ JUMPING_BOOTS, 0, ARMOR_CLASS, 1, 0 },
	{ POWER_PACK, 0, TOOL_CLASS, 10, 0 },
	{ LEMBAS_WAFER, 0, FOOD_CLASS, 3, 0 },
	{ HYPOSPRAY, 0, FOOD_CLASS, 1, 0 },
	{ HYPOSPRAY_AMPULE, 15, TOOL_CLASS, 1, 0 },
	{ HYPOSPRAY_AMPULE, 10, TOOL_CLASS, 1, 0 },
	{ HYPOSPRAY_AMPULE,  5, TOOL_CLASS, 1, 0 },
	{ TINNING_KIT, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ TIN_OPENER, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Anachrononaut_Fem_Clk[] = {
	{ WHITE_VIBROSWORD, 0, WEAPON_CLASS, 1, 0 },
	{ BATTLE_AXE, 0, WEAPON_CLASS, 1, 0 },
	{ BLACK_DRESS, 0, ARMOR_CLASS, 1, 0 },
	{ LONG_GLOVES, 0, ARMOR_CLASS, 1, 0 },
	{ HEELED_BOOTS, 0, ARMOR_CLASS, 1, 0 },
	{ ANDROID_VISOR, 0, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Anachrononaut_Mal_Clk[] = {
	{ GOLD_BLADED_VIBROSWORD, 0, WEAPON_CLASS, 1, 0 },
	{ BATTLE_AXE, 0, WEAPON_CLASS, 1, 0 },
	{ LEATHER_JACKET, 0, ARMOR_CLASS, 1, 0 },
	{ GLOVES, 0, ARMOR_CLASS, 1, 0 },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, 0 },
	{ ANDROID_VISOR, 0, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Barbarian[] = {
#define B_MAJOR	0	/* two-handed sword or battle-axe  */
	{ TWO_HANDED_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
#define B_MINOR	1	/* matched with axe or short sword */
	{ AXE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ RING_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 1, 0 },
	{ TORCH, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef BARD
static struct trobj Bard[] = {
#define BARD_INSTR 0
	{ WOODEN_HARP, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
#define BARD_CLOAK 1
	{ LEATHER_CLOAK, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 3, 0 },
	{ ORANGE, 0, FOOD_CLASS, 3, 0 },
#define BARD_BOOZE 5
	{ POT_BOOZE, 0, POTION_CLASS, 1, UNDEF_BLESS },
#define BARD_WHISTLE 6
	{ TIN_WHISTLE, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
#endif
static struct trobj Binder[] = {
	{ SHEPHERD_S_CROOK, 0, WEAPON_CLASS, 1, 0 },
	{ KNIFE, 0, WEAPON_CLASS, 1, 0 },
	{ SICKLE, 0, WEAPON_CLASS, 1, 0 },
	{ ROCK, 0, GEM_CLASS, 5, 0 }, 
	{ FLINT, 0, GEM_CLASS, 1, 0 },
	{ LEATHER_CLOAK, 0, ARMOR_CLASS, 1, 0 },
	{ CRAM_RATION, 0, FOOD_CLASS, 1, 0 },
	{ APPLE, 0, FOOD_CLASS, 2, 0 },
	{ TRIPE_RATION, 0, FOOD_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};
//initialization of an extern in you.h
boolean forcesight = FALSE;
boolean forceblind = FALSE;
//definition of an extern in you.h
long sealKey[34] = {SEAL_AHAZU, SEAL_AMON, SEAL_ANDREALPHUS, SEAL_ANDROMALIUS, SEAL_ASTAROTH, SEAL_BALAM, 
				 SEAL_BERITH, SEAL_BUER, SEAL_CHUPOCLOPS, SEAL_DANTALION, SEAL_SHIRO, SEAL_ECHIDNA, SEAL_EDEN,
				 SEAL_ENKI, SEAL_EURYNOME, SEAL_EVE, SEAL_FAFNIR, SEAL_HUGINN_MUNINN, SEAL_IRIS, SEAL_JACK,
				 SEAL_MALPHAS, SEAL_MARIONETTE, SEAL_MOTHER, SEAL_NABERIUS, SEAL_ORTHOS, SEAL_OSE, SEAL_OTIAX,
				 SEAL_PAIMON, SEAL_SIMURGH, SEAL_TENEBROUS, SEAL_YMIR, SEAL_SPECIAL|SEAL_DAHLVER_NAR, SEAL_SPECIAL|SEAL_ACERERAK, SEAL_SPECIAL|SEAL_NUMINA
				};
static struct trobj Cave_man[] = {
	{ CLUB, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SLING, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
#define C_AMMO	2
	{ FLINT, 0, GEM_CLASS, 15, UNDEF_BLESS },	/* quan is variable */
	{ ROCK, 0, GEM_CLASS, 3, 0 },			/* yields 18..33 */
	{ LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ TORCH, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef CONVICT
static struct trobj Convict[] = {
	{ ROCK, 0, GEM_CLASS, 1, 0 },
	{ STRIPED_SHIRT, 0, ARMOR_CLASS, 1, 0 },
	{ SPOON, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif  /* CONVICT */
static struct trobj Healer[] = {
	{ SCALPEL, 0, WEAPON_CLASS, 1, 1 },
	{ QUARTERSTAFF, 0, WEAPON_CLASS, 1, 1 },
	{ GLOVES, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HEALER_UNIFORM, 0, ARMOR_CLASS, 1, 1 },
	{ LOW_BOOTS, 0, ARMOR_CLASS, 1, 1 },
	{ STETHOSCOPE, 0, TOOL_CLASS, 1, 0 },
	{ POT_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ POT_EXTRA_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ WAN_SLEEP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	/* always blessed, so it's guaranteed readable */
	{ SPE_HEALING, 0, SPBOOK_CLASS, 1, 1 },
	{ SPE_EXTRA_HEALING, 0, SPBOOK_CLASS, 1, 1 },
	{ SPE_STONE_TO_FLESH, 0, SPBOOK_CLASS, 1, 1 },
	{ APPLE, 0, FOOD_CLASS, 5, 0 },
	{ EUCALYPTUS_LEAF, 0, FOOD_CLASS, 5, 1 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Knight[] = {
	{ LONG_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LANCE, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ CHAIN_MAIL, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HELMET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ KITE_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 10, 0 },
	{ CARROT, 0, FOOD_CLASS, 10, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Monk[] = {
	{ GLOVES, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ ROBE, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ SEDGE_HAT, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
#define M_BOOK		3
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ POT_HEALING, 0, POTION_CLASS, 3, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ APPLE, 0, FOOD_CLASS, 5, UNDEF_BLESS },
	{ ORANGE, 0, FOOD_CLASS, 5, UNDEF_BLESS },
	/* Yes, we know fortune cookies aren't really from China.  They were
	 * invented by George Jung in Los Angeles, California, USA in 1916.
	 */
	{ FORTUNE_COOKIE, 0, FOOD_CLASS, 3, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Noble[] = {
	{ RAPIER, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
#define NOB_SHIRT	1
	{ RUFFLED_SHIRT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
#define NOB_SUIT	2
	{ GENTLEMAN_S_SUIT, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
#define NOB_SHOES	3
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_CLOAK, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 10, 0 },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ SUNROD, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj DNoble[] = {
	{ BULLWHIP, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DAGGER, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ NOBLE_S_DRESS, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ DROVEN_HELM, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ GLOVES, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_CLOAK, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ BOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
#define DNB_TWO_ARROWS	9
	{ ARROW, 2, WEAPON_CLASS, 50, UNDEF_BLESS },
#define DNB_ZERO_ARROWS	10
	{ ARROW, 0, WEAPON_CLASS, 30, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 10, 0 },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ CHUNK_OF_FOSSIL_DARK, 0, GEM_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj DwarfNoble[] = {
	{ BATTLE_AXE, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ CHAIN_MAIL, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ GLOVES, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_CLOAK, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ TRIPE_RATION, 0, FOOD_CLASS, 3, 0 },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ TORCH, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj HDNobleF[] = {
 	{ SPEAR,  1, WEAPON_CLASS, 1, 1 },
 	{ BELL, 0, TOOL_CLASS, 1, 1 },
	{ BUCKLER, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_HELM, 1, ARMOR_CLASS, 1, 1 },
	{ CHAIN_MAIL, 1, ARMOR_CLASS, 1, 1 },
	{ GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_CLOAK, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ SPE_HEALING, 0, SPBOOK_CLASS, 1, 1 },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ TORCH, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj HDNobleM[] = {
	{ BROADSWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BRONZE_PLATE_MAIL, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ BRONZE_HELM, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ ROUNDSHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ GLOVES, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ TORCH, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Pirate[] = {
	{ SCIMITAR, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ FLINTLOCK, 0, WEAPON_CLASS, 1, 0 },
#define PIR_KNIVES	2
	{ KNIFE, 0, WEAPON_CLASS, 1, 0 },
	{ CRAM_RATION, 0, FOOD_CLASS, 2, UNDEF_BLESS },
	{ LEATHER_JACKET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
#define PIR_SNACK 6
	{ BANANA, 0, FOOD_CLASS, 3, 0 },
	{ POT_BOOZE, 0, POTION_CLASS, 3, UNDEF_BLESS },
#define PIR_JEWELRY 8
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ GRAPPLING_HOOK, 0, TOOL_CLASS, 1, 0 },
	{ PICK_AXE, 0, TOOL_CLASS, 1, 0 },
	{ OILSKIN_SACK, 0, TOOL_CLASS, 1, 0 },
#define PIR_BULLETS 10
	{ BULLET,  0, WEAPON_CLASS, 20, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Priest[] = {
#define PRI_WEAPON	0
	{ MACE, 1, WEAPON_CLASS, 1, 1 },
	{ ROBE, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ BUCKLER, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_WATER, 0, POTION_CLASS, 4, 1 },	/* holy water */
	{ CLOVE_OF_GARLIC, 0, FOOD_CLASS, 1, 0 },
	{ SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 2, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj DPriest[] = {
#define PRI_WEAPON	0
	{ MACE, 1, WEAPON_CLASS, 1, 1 },
	{ BULLWHIP, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ROBE, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ BUCKLER, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_WATER, 0, POTION_CLASS, 4, 1 },	/* holy water */
	{ CLOVE_OF_GARLIC, 0, FOOD_CLASS, 1, 0 },
	{ SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 2, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Ranger[] = {
	{ DAGGER, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
#define RAN_BOW			1
	{ BOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
#define RAN_TWO_ARROWS	2
	{ ARROW, 2, WEAPON_CLASS, 50, UNDEF_BLESS },
#define RAN_ZERO_ARROWS	3
	{ ARROW, 0, WEAPON_CLASS, 30, UNDEF_BLESS },
	{ CLOAK_OF_DISPLACEMENT, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CRAM_RATION, 0, FOOD_CLASS, 4, 0 },
	{ TORCH, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Rogue[] = {
	{ SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
#define R_DAGGERS	1
	{ DAGGER, 0, WEAPON_CLASS, 10, 0 },	/* quan is variable */
	{ LEATHER_ARMOR, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LOW_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_SICKNESS, 0, POTION_CLASS, 1, 0 },
	{ LOCK_PICK, 9, TOOL_CLASS, 1, 0 },
	{ SACK, 0, TOOL_CLASS, 1, 0 },
	{ TORCH, 0, TOOL_CLASS, 2, 0 },
	{ SHADOWLANDER_S_TORCH, 0, TOOL_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Samurai[] = {
#define S_WEAPON	0
	{ KATANA, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
#define S_SECOND	1
	{ SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS }, /* wakizashi */
#define S_ARROWS	3
	{ YUMI, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ YA, 0, WEAPON_CLASS, 25, UNDEF_BLESS }, /* variable quan */
	{ SPLINT_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ ARMORED_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ GAUNTLETS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HELMET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ MASK, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
#ifdef TOURIST
static struct trobj Tourist[] = {
#define T_DARTS		0
	{ DART, 2, WEAPON_CLASS, 25, UNDEF_BLESS },	/* quan is variable */
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 10, 0 },
	{ POT_EXTRA_HEALING, 0, POTION_CLASS, 2, UNDEF_BLESS },
	{ SCR_MAGIC_MAPPING, 0, SCROLL_CLASS, 4, UNDEF_BLESS },
	{ HAWAIIAN_SHIRT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
/*	{ HAWAIIAN_SHORTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },*/
	{ LOW_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ EXPENSIVE_CAMERA, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ CREDIT_CARD, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif
static struct trobj Valkyrie[] = {
	{ SPEAR, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BOW, 	 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ARROW, 0, WEAPON_CLASS,15, UNDEF_BLESS },
	{ BUCKLER, 3, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Wizard[] = {
#define W_WEAPON	0
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 0 },
	{ ATHAME, -1, WEAPON_CLASS, 1, 0 },
#define W_MULTSTART	2
	{ CLOAK_OF_MAGIC_RESISTANCE, 0, ARMOR_CLASS, 1, 0 },
	{ LOW_BOOTS, 0, ARMOR_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 2, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, 0 },
	{ SPE_FORCE_BOLT, 0, SPBOOK_CLASS, 1, 0 },
#define W_MULTEND	6
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

/*
 *	Optional extra inventory items.
 */

static struct trobj Key[] = {
	{ SKELETON_KEY, 0, TOOL_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj HealingBook[] = {
	{ SPE_HEALING, 0, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj ForceBook[] = {
	{ SPE_FORCE_BOLT, 0, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj DrainBook[] = {
	{ SPE_DRAIN_LIFE, 0, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj FamBook[] = {
	{ SPE_CREATE_FAMILIAR, 0, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj MRGloves[] = {
	{ ORIHALCYON_GAUNTLETS, 0, ARMOR_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj DarkWand[] = {
	{ WAN_DARKNESS, UNDEF_SPE, WAND_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj Phial[] = {
	{ POT_STARLIGHT, UNDEF_SPE, POTION_CLASS, 1, 1 },
	{ SACK, UNDEF_SPE, TOOL_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj BlackTorches[] = {
	{ SHADOWLANDER_S_TORCH, 0, TOOL_CLASS, 3, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj ExtraBook[] = {
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj GnomishHat[] = {
	{ GNOMISH_POINTY_HAT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj TallowCandles[] = {
	{ TALLOW_CANDLE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

static struct trobj SleepPotions[] = {
	{ POT_SLEEPING, 0, POTION_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj DrovenHelm[] = {
	{ DROVEN_HELM, 0, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj DrovenCloak[] = {
	{ DROVEN_CLOAK, 0, ARMOR_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Tinopener[] = {
	{ TIN_OPENER, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Magicmarker[] = {
	{ MAGIC_MARKER, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Lamp[] = {
	{ OIL_LAMP, 1, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Blindfold[] = {
	{ BLINDFOLD, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Instrument[] = {
	{ WOODEN_FLUTE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj RandRing[] = {
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Xtra_food[] = {
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Orc_Brd_equip[] = {
	{ ORCISH_SHORT_SWORD, 0, WEAPON_CLASS, 1, 0 },
	{ ORCISH_DAGGER, 0, WEAPON_CLASS, 4, 0 },
	{ ORCISH_SHIELD, 0, WEAPON_CLASS, 1, 0 },
	{ ORCISH_HELM, 0, WEAPON_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef TOURIST
static struct trobj Leash[] = {
	{ LEASH, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Towel[] = {
	{ TOWEL, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif	/* TOURIST */
static struct trobj Wishing[] = {
	{ WAN_WISHING, 3, WAND_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef GOLDOBJ
static struct trobj Money[] = {
	{ GOLD_PIECE, 0 , COIN_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif

/* race-based substitutions for initial inventory;
   the weaker cloak for elven rangers is intentional--they shoot better */
static struct inv_sub { short race_pm, item_otyp, subs_otyp; } inv_subs[] = {
    // Elf substitutions
    { PM_ELF,	ATHAME,					ELVEN_DAGGER	      },
    { PM_ELF,	DAGGER,					ELVEN_DAGGER	      },
    { PM_ELF,	KNIFE,					ELVEN_DAGGER	      },
    { PM_ELF,	SPEAR,					ELVEN_SPEAR	      },
    { PM_ELF,	SHORT_SWORD,			ELVEN_SHORT_SWORD     },
    { PM_ELF,	BOW,					ELVEN_BOW	      },
    { PM_ELF,	ARROW,					ELVEN_ARROW	      },
    { PM_ELF,	HELMET,					ELVEN_HELM    },
	{ PM_ELF,	BUCKLER,				ELVEN_SHIELD	      },
	{ PM_ELF,	KITE_SHIELD,			ELVEN_SHIELD	      },
    { PM_ELF,	CLOAK_OF_DISPLACEMENT,	ELVEN_CLOAK	      },
    { PM_ELF,	LEATHER_CLOAK,			ELVEN_CLOAK	      },
    { PM_ELF,	CRAM_RATION,			LEMBAS_WAFER	      },
    { PM_ELF,	FOOD_RATION,			LEMBAS_WAFER	      },
    { PM_ELF,	GENTLEWOMAN_S_DRESS,	ELVEN_TOGA  },
    { PM_ELF,	GENTLEMAN_S_SUIT,		ELVEN_TOGA  },
    { PM_ELF,	VICTORIAN_UNDERWEAR,	ELVEN_SHIELD  },
    { PM_ELF,	RUFFLED_SHIRT,			ELVEN_SHIELD  },
    { PM_ELF,	RAPIER,					ELVEN_SPEAR  },
    { PM_ELF,	STILETTOS,				ELVEN_BOOTS  },
    // Orc substitutions
    { PM_ORC,	ATHAME,			ORCISH_DAGGER	      },
    { PM_ORC,	DAGGER,			ORCISH_DAGGER	      },
    { PM_ORC,	KNIFE,			ORCISH_DAGGER	      },
    { PM_ORC,	SPEAR,			ORCISH_SPEAR	      },
    { PM_ORC,	SHORT_SWORD,	ORCISH_SHORT_SWORD    },
    { PM_ORC,	BOW,			ORCISH_BOW	      },
    { PM_ORC,	ARROW,			ORCISH_ARROW	      },
    { PM_ORC,	HELMET,			ORCISH_HELM	      },
    { PM_ORC,	BUCKLER,		ORCISH_SHIELD	      },
    { PM_ORC,	KITE_SHIELD,	ORCISH_SHIELD	      },
    { PM_ORC,	RING_MAIL,		ORCISH_RING_MAIL      },
    { PM_ORC,	CHAIN_MAIL,		ORCISH_CHAIN_MAIL     },
    // Dwarf substitutions
    { PM_DWARF,	KNIFE,				DAGGER	      		  },
    { PM_DWARF, SPEAR,				DWARVISH_SPEAR	      },
    { PM_DWARF, SHORT_SWORD,		DWARVISH_SHORT_SWORD  },
    { PM_DWARF, HELMET,				DWARVISH_HELM    },
	{ PM_DWARF, BUCKLER,			DWARVISH_ROUNDSHIELD  }, 
	{ PM_DWARF, KITE_SHIELD,		DWARVISH_ROUNDSHIELD  }, 
    { PM_DWARF, PICK_AXE,			DWARVISH_MATTOCK      },
    { PM_DWARF, TWO_HANDED_SWORD,	BATTLE_AXE    },
    { PM_DWARF,	LEATHER_CLOAK,		DWARVISH_CLOAK  },
    { PM_GNOME, FEDORA,			GNOMISH_POINTY_HAT    },
    { PM_GNOME, BULLWHIP,		AKLYS    },
    { PM_GNOME, CLUB,			AKLYS    },
    { PM_GNOME, BOW,			CROSSBOW	      },
    { PM_GNOME, ARROW,			CROSSBOW_BOLT	      },
    // Drow substitutions
    { PM_DROW,	CLOAK_OF_DISPLACEMENT,		DROVEN_PLATE_MAIL  },
    { PM_DROW,	CLOAK_OF_MAGIC_RESISTANCE,	DROVEN_CHAIN_MAIL  },
    { PM_DROW,	LEATHER_ARMOR,				DROVEN_CHAIN_MAIL  },
    { PM_DROW,	ROBE,						DROVEN_PLATE_MAIL  },
    { PM_DROW,	ATHAME,						DROVEN_DAGGER	      },
    { PM_DROW,	DAGGER,						DROVEN_DAGGER	      },
    { PM_DROW,	KNIFE,						DROVEN_DAGGER	      },
    { PM_DROW,	SPEAR,						DROVEN_SHORT_SWORD	      },
    { PM_DROW,	SHORT_SWORD,				DROVEN_SHORT_SWORD     },
    { PM_DROW,	BOW,						DROVEN_CROSSBOW	      },
    { PM_DROW,	ARROW,						DROVEN_BOLT	      },
    { PM_DROW,	LEATHER_CLOAK,				DROVEN_CLOAK  },
    { PM_DROW,	MACE,						KHAKKHARA  },
    { PM_DROW,	BUCKLER,					DROVEN_HELM  },
    { PM_DROW,	GENTLEWOMAN_S_DRESS,		NOBLE_S_DRESS  },
    { PM_DROW,	GENTLEMAN_S_SUIT,			CONSORT_S_SUIT  },
    // { PM_DROW,	SCIMITAR,				DROVEN_SPEAR  },
    { PM_DROW,	APPLE,						TRIPE_RATION  },
    { PM_DROW,	TORCH,						SHADOWLANDER_S_TORCH  },
    { PM_DROW,	SUNROD,						SHADOWLANDER_S_TORCH  },
    // Half-dragon substitutions
    { PM_HALF_DRAGON,	APPLE,		TRIPE_RATION  },
    { PM_HALF_DRAGON,	CARROT,		TRIPE_RATION  },
    // Incantifier substitutions
    { PM_INCANTIFIER, CLOAK_OF_MAGIC_RESISTANCE,		ROBE  },
    { PM_INCANTIFIER, CLOAK_OF_DISPLACEMENT,			ROBE  },
    { PM_INCANTIFIER,	LEATHER_ARMOR,				ROBE  },
    { PM_INCANTIFIER,	LEATHER_JACKET,				ROBE  },
    { PM_INCANTIFIER,	RING_MAIL,					ROBE  },
    { PM_INCANTIFIER,	SPLINT_MAIL,				ROBE  },
    { PM_INCANTIFIER,	FOOD_RATION,				SCR_FOOD_DETECTION    	  },
    { PM_INCANTIFIER,	CRAM_RATION,				SCR_FOOD_DETECTION    	  },
    { PM_INCANTIFIER,	POT_FRUIT_JUICE,			SCR_FOOD_DETECTION	      },
    { PM_INCANTIFIER,	TRIPE_RATION,				SCR_FOOD_DETECTION    	  },
    { PM_INCANTIFIER,	BANANA,						SCR_FOOD_DETECTION    	  },
    { PM_INCANTIFIER,	ORANGE,						SCR_FOOD_DETECTION    	  },
    { PM_INCANTIFIER,	POT_BOOZE,					SCR_FOOD_DETECTION   	  },
    // Vampire substitutions
    { PM_VAMPIRE,	ATHAME,				DAGGER    	  },
    { PM_VAMPIRE,	FOOD_RATION,		POT_BLOOD    	  },
    { PM_VAMPIRE,	CRAM_RATION,		POT_BLOOD    	  },
    { PM_VAMPIRE,	POT_FRUIT_JUICE,	POT_BLOOD	      },
    { PM_VAMPIRE,	TRIPE_RATION,		POT_BLOOD    	  },
    { PM_VAMPIRE,	BANANA,				POT_BLOOD    	  },
    { PM_VAMPIRE,	ORANGE,				POT_BLOOD    	  },
    { PM_VAMPIRE,	POT_BOOZE,			POT_BLOOD    	  },
    // Clockwork automaton substitutions
    { PM_CLOCKWORK_AUTOMATON,	FOOD_RATION, 		POT_OIL },
    { PM_CLOCKWORK_AUTOMATON,	CRAM_RATION, 		POT_OIL },
    { PM_CLOCKWORK_AUTOMATON,	POT_FRUIT_JUICE,	POT_OIL	      },
    { PM_CLOCKWORK_AUTOMATON,	TRIPE_RATION,		POT_OIL    	  },
    { PM_CLOCKWORK_AUTOMATON,	BANANA,				POT_OIL    	  },
    { PM_CLOCKWORK_AUTOMATON,	ORANGE,				POT_OIL    	  },
    { PM_CLOCKWORK_AUTOMATON,	POT_BOOZE,			POT_OIL    	  },
    // Yuki-onna substitutions
    { PM_YUKI_ONNA,           RING_MAIL,   STUDDED_LEATHER_ARMOR  },
    { PM_YUKI_ONNA,           SPLINT_MAIL, STUDDED_LEATHER_ARMOR  },
    { PM_YUKI_ONNA,           HELMET,                  SEDGE_HAT  },
    { NON_PM,	STRANGE_OBJECT,		STRANGE_OBJECT	      }
};

static const struct def_skill Skill_A[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE,  P_BASIC },
    { P_PICK_AXE, P_EXPERT },		{ P_SHORT_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_SKILLED },		{ P_QUARTERSTAFF, P_SKILLED },
//#ifdef FIREARMS
    { P_FIREARM, P_SKILLED },
//#endif
    { P_SLING, P_SKILLED },		{ P_DART, P_BASIC },
    { P_BOOMERANG, P_EXPERT },		{ P_WHIP, P_EXPERT },
    { P_UNICORN_HORN, P_SKILLED },	{ P_HARVEST, P_BASIC },
    { P_ATTACK_SPELL, P_BASIC },	{ P_HEALING_SPELL, P_BASIC },
    { P_DIVINATION_SPELL, P_EXPERT},	{ P_MATTER_SPELL, P_BASIC},
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_WAND_POWER, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Ana[] = {
    { P_DAGGER, P_SKILLED },		{ P_KNIFE,  P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },{ P_LANCE,  P_EXPERT },
    { P_SABER, P_EXPERT },		{ P_LONG_SWORD,  P_BASIC },
    { P_CLUB, P_SKILLED },		{ P_QUARTERSTAFF, P_EXPERT },
	{ P_BROAD_SWORD, P_EXPERT },{ P_HAMMER, P_BASIC },
//#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
//#endif
    { P_DART, P_BASIC },		{ P_CROSSBOW, P_EXPERT },
    { P_WHIP, P_SKILLED },		 { P_BOOMERANG, P_SKILLED },
    { P_ATTACK_SPELL, P_SKILLED },	{ P_HEALING_SPELL, P_SKILLED },
    { P_DIVINATION_SPELL, P_SKILLED},	{ P_MATTER_SPELL, P_SKILLED},
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Droid_Ana[] = {
    { P_DAGGER, P_SKILLED },		{ P_KNIFE,  P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },{ P_LANCE,  P_EXPERT },
    { P_SABER, P_EXPERT },		{ P_LONG_SWORD,  P_EXPERT },
    { P_AXE, P_EXPERT },		{ P_TWO_HANDED_SWORD,  P_EXPERT },
    { P_CLUB, P_SKILLED },		{ P_QUARTERSTAFF, P_SKILLED },
	{ P_BROAD_SWORD, P_SKILLED },{ P_HAMMER, P_BASIC },
	{ P_SPEAR, P_EXPERT },
//#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
//#endif
    { P_DART, P_BASIC },		{ P_CROSSBOW, P_EXPERT },
    { P_WHIP, P_SKILLED },		 { P_BOOMERANG, P_SKILLED },
    { P_ATTACK_SPELL, P_SKILLED },	{ P_HEALING_SPELL, P_SKILLED },
    { P_DIVINATION_SPELL, P_SKILLED },	{ P_ENCHANTMENT_SPELL, P_SKILLED },
    { P_CLERIC_SPELL, P_SKILLED },	{ P_ESCAPE_SPELL, P_SKILLED },
    { P_MATTER_SPELL, P_SKILLED }, { P_WAND_POWER, P_SKILLED },
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_TWO_WEAPON_COMBAT, P_BASIC},
    { P_BARE_HANDED_COMBAT, P_GRAND_MASTER },
    { P_NONE, 0 }
};

// static const struct def_skill Skill_Neu_Ana[] = {
    // { FFORM_SHII_CHO, P_EXPERT },		{ FFORM_MAKASHI,  P_EXPERT },
    // { FFORM_SORESU, P_EXPERT },			{ FFORM_ATARU,  P_SKILLED },
    // { FFORM_DJEM_SO, P_EXPERT },		{ FFORM_SHIEN,  P_EXPERT },
    // { FFORM_NIMAN, P_EXPERT },			{ FFORM_JUYO,  P_BASIC },
    // { P_NONE, 0 }
// };

// static const struct def_skill Skill_Cha_Ana[] = {
    // { FFORM_SHII_CHO, P_EXPERT },		{ FFORM_MAKASHI,  P_EXPERT },
    // { FFORM_SORESU, P_SKILLED },		{ FFORM_ATARU,  P_EXPERT },
    // { FFORM_DJEM_SO, P_EXPERT },		{ FFORM_SHIEN,  P_EXPERT },
    // { FFORM_NIMAN, P_BASIC },			{ FFORM_JUYO,  P_EXPERT },
    // { P_NONE, 0 }
// };

static const struct def_skill Skill_All_Ana[] = {
    { FFORM_SHII_CHO, P_EXPERT },		{ FFORM_MAKASHI,  P_EXPERT },
    { FFORM_SORESU, P_EXPERT },			{ FFORM_ATARU,  P_EXPERT },
    { FFORM_DJEM_SO, P_EXPERT },		{ FFORM_SHIEN,  P_EXPERT },
    { FFORM_NIMAN, P_EXPERT },			{ FFORM_JUYO,  P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_B[] = {
    { P_DAGGER, P_SKILLED },		{ P_AXE, P_EXPERT },
    { P_PICK_AXE, P_SKILLED },	{ P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_EXPERT },	{ P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_EXPERT },	{ P_SCIMITAR, P_SKILLED },
    { P_SABER, P_SKILLED },		{ P_CLUB, P_SKILLED },
    { P_MACE, P_SKILLED },		{ P_MORNING_STAR, P_SKILLED },
    { P_FLAIL, P_BASIC },		{ P_HAMMER, P_EXPERT },
	{ P_HARVEST, P_SKILLED },
    { P_QUARTERSTAFF, P_BASIC },	{ P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_EXPERT },		{ P_BOW, P_SKILLED },
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_NONE, 0 }
};

#ifdef BARD
static struct def_skill Skill_Bard[] = {
    { P_DAGGER, P_SKILLED },		{ P_KNIFE,  P_BASIC },
    { P_SHORT_SWORD, P_BASIC },		{ P_SCIMITAR, P_BASIC },
    { P_CLUB, P_SKILLED },		{ P_FLAIL, P_BASIC },
    { P_QUARTERSTAFF, P_SKILLED },	{ P_POLEARMS, P_BASIC },
    { P_JAVELIN, P_SKILLED },		{ P_SPEAR, P_BASIC },
    { P_SLING, P_SKILLED },		{ P_DART, P_EXPERT },
    { P_UNICORN_HORN, P_BASIC },	{ P_CROSSBOW, P_SKILLED },
    { P_ENCHANTMENT_SPELL, P_EXPERT },	{ P_ESCAPE_SPELL, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_EXPERT }, { P_MUSICALIZE, P_EXPERT },
	{ P_BEAST_MASTERY, P_EXPERT },
    { P_NONE, 0 }
};
#endif

static const struct def_skill Skill_N[] = {
    { P_NONE, 0 }
};

static const struct def_skill Skill_C[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },		{ P_PICK_AXE, P_BASIC },
    { P_CLUB, P_EXPERT },		{ P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_BASIC },	{ P_FLAIL, P_SKILLED },
    { P_HAMMER, P_SKILLED },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },		{ P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_SKILLED },		{ P_TRIDENT, P_SKILLED },
    { P_BOW, P_SKILLED },		{ P_SLING, P_EXPERT },
    { P_ATTACK_SPELL, P_BASIC },	{ P_MATTER_SPELL, P_SKILLED },
    { P_BOOMERANG, P_EXPERT },		{ P_UNICORN_HORN, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_NONE, 0 }
};

#ifdef CONVICT
static const struct def_skill Skill_Con[] = {
    { P_DAGGER, P_SKILLED },		{ P_KNIFE,  P_EXPERT },
    { P_HAMMER, P_SKILLED },		{ P_PICK_AXE, P_EXPERT },
    { P_CLUB, P_EXPERT },		    { P_MACE, P_BASIC },
    { P_DART, P_SKILLED },		    { P_FLAIL, P_EXPERT },
    { P_SHORT_SWORD, P_BASIC },		{ P_SLING, P_SKILLED },
	{ P_HARVEST, P_EXPERT },
    { P_ATTACK_SPELL, P_BASIC },	{ P_ESCAPE_SPELL, P_EXPERT },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_BEAST_MASTERY, P_BASIC },
    { P_NONE, 0 }
};
#endif  /* CONVICT */

static const struct def_skill Skill_H[] = {
    { P_DAGGER, P_SKILLED },		{ P_KNIFE, P_EXPERT },
    { P_SHORT_SWORD, P_SKILLED },	{ P_SCIMITAR, P_BASIC },
    { P_SABER, P_BASIC },		{ P_CLUB, P_SKILLED },
    { P_MACE, P_BASIC },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_BASIC },		{ P_SPEAR, P_BASIC },
	{ P_HARVEST, P_EXPERT },
    { P_JAVELIN, P_BASIC },		{ P_TRIDENT, P_BASIC },
    { P_SLING, P_SKILLED },		{ P_DART, P_EXPERT },
    { P_SHURIKEN, P_SKILLED },		{ P_UNICORN_HORN, P_EXPERT },
    { P_HEALING_SPELL, P_EXPERT },
    { P_WAND_POWER, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_BEAST_MASTERY, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_I[] = {
    { P_ATTACK_SPELL, P_EXPERT },	{ P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_EXPERT },	{ P_ENCHANTMENT_SPELL, P_EXPERT },
    { P_CLERIC_SPELL, P_EXPERT },	{ P_ESCAPE_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
    { P_NONE, 0 }
};
#ifdef BARD
static const struct def_skill Skill_Elf_Music[] = {
    { P_MUSICALIZE, P_EXPERT },
    { P_NONE, 0 }
};
#endif
static const struct def_skill Skill_Elf_Ana[] = {
    { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Orc_Brd[] = {
    { P_DAGGER, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Drow_Unarmed[] = {
    { P_BARE_HANDED_COMBAT, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static const struct def_skill Skill_DP[] = {
    { P_ATTACK_SPELL, P_EXPERT },
    { P_BEAST_MASTERY, P_BASIC },
    { P_NONE, 0 }
};

static const struct def_skill Skill_DW[] = {
    { P_CLERIC_SPELL, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_HD_Female[] = {
    { P_HARVEST, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_G[] = {
    { P_PICK_AXE, P_EXPERT }, { P_CROSSBOW, P_EXPERT },
    { P_CLUB, P_EXPERT }, { P_NONE, 0 }
};

static const struct def_skill Skill_K[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE, P_BASIC },
    { P_AXE, P_SKILLED },		{ P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_SKILLED },	{ P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },	{ P_TWO_HANDED_SWORD, P_SKILLED },
    { P_SCIMITAR, P_BASIC },		{ P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },		{ P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_SKILLED },	{ P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },		{ P_POLEARMS, P_EXPERT },
    { P_SPEAR, P_SKILLED },		{ P_JAVELIN, P_SKILLED },
    { P_TRIDENT, P_BASIC },		{ P_LANCE, P_EXPERT },
    { P_BOW, P_BASIC },			{ P_CROSSBOW, P_SKILLED },
    { P_ATTACK_SPELL, P_SKILLED },	{ P_HEALING_SPELL, P_SKILLED },
    { P_CLERIC_SPELL, P_SKILLED },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_BEAST_MASTERY, P_SKILLED },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Mon[] = {
    { P_QUARTERSTAFF, P_EXPERT },    { P_SPEAR, P_BASIC },
    { P_JAVELIN, P_BASIC },		{ P_SHURIKEN, P_EXPERT },	
    { P_HARVEST, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_ATTACK_SPELL, P_BASIC },    { P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_BASIC },{ P_ENCHANTMENT_SPELL, P_BASIC },
    { P_CLERIC_SPELL, P_SKILLED },  { P_ESCAPE_SPELL, P_BASIC },
    { P_MATTER_SPELL, P_BASIC },
    { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_TWO_WEAPON_COMBAT, P_GRAND_MASTER },
#ifdef BARD
    { P_MUSICALIZE, P_BASIC },
#endif
    { P_NONE, 0 }
};

static const struct def_skill Skill_Nob[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE, P_EXPERT },
    { P_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_SKILLED },	{ P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_EXPERT },
    { P_CLUB, P_BASIC },		{ P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_BASIC },	{ P_FLAIL, P_BASIC },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },		{ P_LANCE, P_SKILLED },
    { P_BOW, P_SKILLED },			{ P_CROSSBOW, P_BASIC },
    { P_ATTACK_SPELL, P_SKILLED },	{ P_HEALING_SPELL, P_SKILLED },
    { P_ESCAPE_SPELL, P_SKILLED },	{ P_ENCHANTMENT_SPELL, P_SKILLED },
#ifdef STEED
    { P_RIDING, P_SKILLED },
#endif
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_BEAST_MASTERY, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_HD_Female_Nob[] = {
    { P_DAGGER, P_SKILLED },		{ P_KNIFE, P_BASIC },
    { P_AXE, P_EXPERT },
    { P_SHORT_SWORD, P_SKILLED },	{ P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },
    { P_SCIMITAR, P_SKILLED },		{ P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },		{ P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_BASIC },	{ P_FLAIL, P_BASIC },
    { P_POLEARMS, P_BASIC },	{ P_HARVEST, P_EXPERT },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },		{ P_LANCE, P_SKILLED },
    { P_BOW, P_SKILLED },			{ P_CROSSBOW, P_BASIC },
    { P_ATTACK_SPELL, P_SKILLED },	{ P_HEALING_SPELL, P_SKILLED },
    { P_ESCAPE_SPELL, P_SKILLED },	{ P_ENCHANTMENT_SPELL, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_BEAST_MASTERY, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_ENob[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE, P_EXPERT },
    { P_HARVEST, P_EXPERT },
    { P_SHORT_SWORD, P_SKILLED },	{ P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_EXPERT },
    { P_SCIMITAR, P_EXPERT },		{ P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },		{ P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_BASIC },	{ P_FLAIL, P_BASIC },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },		{ P_LANCE, P_SKILLED },
    { P_BOW, P_SKILLED },			{ P_CROSSBOW, P_BASIC },
    { P_ATTACK_SPELL, P_SKILLED },	{ P_HEALING_SPELL, P_SKILLED },
    { P_ESCAPE_SPELL, P_SKILLED },	{ P_ENCHANTMENT_SPELL, P_SKILLED },
#ifdef STEED
    { P_RIDING, P_SKILLED },
#endif
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_BEAST_MASTERY, P_SKILLED },
    { P_NONE, 0 }
};

static const struct def_skill Skill_DNob[] = {
    { P_DAGGER, P_SKILLED },		{ P_KNIFE, P_BASIC },
    { P_WHIP, P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_SKILLED },		{ P_SABER, P_BASIC },
    { P_CLUB, P_BASIC },		{ P_MACE, P_BASIC },
    { P_FLAIL, P_BASIC },
    { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },		{ P_LANCE, P_SKILLED },
    { P_QUARTERSTAFF, P_EXPERT },
    { P_BOW, P_BASIC },			{ P_CROSSBOW, P_SKILLED },
    { P_SLING, P_BASIC },
    { P_ATTACK_SPELL, P_SKILLED },	{ P_CLERIC_SPELL, P_SKILLED },
    { P_ESCAPE_SPELL, P_SKILLED },	{ P_ENCHANTMENT_SPELL, P_SKILLED },
#ifdef STEED
    { P_RIDING, P_SKILLED },
#endif
    { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_BEAST_MASTERY, P_SKILLED },
    { P_NONE, 0 }
};

static const struct def_skill Skill_DwaNob[] = {
    { P_DAGGER, P_SKILLED },		{ P_AXE, P_EXPERT },
    { P_PICK_AXE, P_EXPERT },	{ P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_SKILLED },	{ P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_SKILLED },	{ P_SCIMITAR, P_SKILLED },
    { P_SABER, P_SKILLED },		{ P_CLUB, P_SKILLED },
    { P_MACE, P_SKILLED },		{ P_MORNING_STAR, P_SKILLED },
    { P_FLAIL, P_BASIC },		{ P_HAMMER, P_EXPERT },
    { P_HARVEST, P_SKILLED },
    { P_QUARTERSTAFF, P_BASIC },	{ P_SPEAR, P_EXPERT },
    { P_TRIDENT, P_EXPERT },		{ P_BOW, P_SKILLED },
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_MASTER },
    { P_BEAST_MASTERY, P_BASIC },
    { P_NONE, 0 }
};

static const struct def_skill Skill_P[] = {
    { P_CLUB, P_EXPERT },		{ P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },		{ P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },		{ P_SPEAR, P_SKILLED },
    { P_JAVELIN, P_SKILLED },		{ P_TRIDENT, P_SKILLED },
    { P_LANCE, P_BASIC },		{ P_BOW, P_BASIC },
    { P_SLING, P_BASIC },		{ P_CROSSBOW, P_BASIC },
    { P_DART, P_BASIC },		{ P_SHURIKEN, P_BASIC },
    { P_BOOMERANG, P_BASIC },		{ P_UNICORN_HORN, P_SKILLED },
    { P_HEALING_SPELL, P_EXPERT },	{ P_DIVINATION_SPELL, P_EXPERT },
    { P_CLERIC_SPELL, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_BASIC },
#ifdef BARD
    { P_MUSICALIZE, P_BASIC },
#endif
    { P_NONE, 0 }
};

static const struct def_skill Skill_Dro_M_P[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE, P_EXPERT },
    { P_PICK_AXE, P_SKILLED },	{ P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_EXPERT },		{ P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_SKILLED },
    { P_POLEARMS, P_SKILLED },		{ P_SPEAR, P_SKILLED },
    { P_JAVELIN, P_SKILLED },		{ P_TRIDENT, P_SKILLED },
    { P_LANCE, P_BASIC },		{ P_BOW, P_BASIC },
    { P_SLING, P_BASIC },		{ P_CROSSBOW, P_BASIC },
    { P_DART, P_BASIC },		{ P_SHURIKEN, P_BASIC },
    { P_BOOMERANG, P_BASIC },		{ P_UNICORN_HORN, P_SKILLED },
    { P_HEALING_SPELL, P_EXPERT },	{ P_DIVINATION_SPELL, P_EXPERT },
    { P_CLERIC_SPELL, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Dro_F_P[] = {
    { P_WHIP, P_EXPERT },		{ P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_EXPERT },	{ P_FLAIL, P_EXPERT },
    { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },		{ P_SPEAR, P_SKILLED },
    { P_JAVELIN, P_SKILLED },		{ P_TRIDENT, P_SKILLED },
    { P_LANCE, P_BASIC },		{ P_BOW, P_BASIC },
    { P_SLING, P_BASIC },		{ P_CROSSBOW, P_SKILLED },
    { P_DART, P_BASIC },		{ P_SHURIKEN, P_BASIC },
    { P_UNICORN_HORN, P_SKILLED },
    { P_HEALING_SPELL, P_EXPERT },	{ P_DIVINATION_SPELL, P_EXPERT },
    { P_CLERIC_SPELL, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Pir[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_AXE, P_SKILLED },	    { P_SHORT_SWORD, P_BASIC },
    { P_BROAD_SWORD, P_EXPERT },{ P_LONG_SWORD, P_BASIC },
    { P_SCIMITAR, P_EXPERT },	{ P_SABER, P_EXPERT },
    { P_CLUB, P_EXPERT },		{ P_MORNING_STAR, P_SKILLED },
    { P_FLAIL, P_EXPERT },		{ P_SPEAR, P_BASIC },
    { P_JAVELIN, P_EXPERT },	{ P_TRIDENT, P_EXPERT },
    { P_CROSSBOW, P_EXPERT },   { P_DART, P_SKILLED },
    { P_WHIP, P_SKILLED },   	{ P_UNICORN_HORN, P_BASIC },
	{ P_PICK_AXE, P_SKILLED },
//#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
//#endif
    { P_ATTACK_SPELL, P_BASIC },{ P_DIVINATION_SPELL, P_BASIC },
    { P_ENCHANTMENT_SPELL, P_BASIC },{ P_ESCAPE_SPELL, P_SKILLED },
    { P_WAND_POWER, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_R[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_SKILLED },	{ P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },		{ P_SABER, P_SKILLED },
    { P_CLUB, P_SKILLED },		{ P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_BASIC },	{ P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },		{ P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },		{ P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },		{ P_SHURIKEN, P_SKILLED },
    { P_DIVINATION_SPELL, P_SKILLED },	{ P_ESCAPE_SPELL, P_SKILLED },
    { P_MATTER_SPELL, P_SKILLED },
    { P_WAND_POWER, P_EXPERT },
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_Ran[] = {
    { P_DAGGER, P_EXPERT },		 { P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },	 { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_BASIC },	 { P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_SKILLED },	 { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_EXPERT }, { P_POLEARMS, P_SKILLED },
    { P_HARVEST, P_SKILLED }, { P_CLUB, P_SKILLED },
    { P_SPEAR, P_SKILLED },	 { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_BASIC },	 { P_BOW, P_EXPERT },
    { P_SLING, P_EXPERT },	 { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },	 { P_SHURIKEN, P_SKILLED },
    { P_BOOMERANG, P_EXPERT },	 { P_WHIP, P_BASIC },
    { P_HEALING_SPELL, P_BASIC },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_ESCAPE_SPELL, P_BASIC },
    { P_WAND_POWER, P_BASIC },
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_BEAST_MASTERY, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_S[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },	{ P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },		{ P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_BASIC },		{ P_SABER, P_BASIC },
    { P_FLAIL, P_SKILLED },		{ P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_EXPERT },		{ P_SPEAR, P_BASIC },
    { P_JAVELIN, P_BASIC },		{ P_LANCE, P_SKILLED },
    { P_BOW, P_EXPERT },		{ P_SHURIKEN, P_EXPERT },
    { P_DART, P_EXPERT },
    { P_ATTACK_SPELL, P_SKILLED },	{ P_CLERIC_SPELL, P_SKILLED },
#ifdef STEED
    { P_RIDING, P_SKILLED },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },
    { P_MARTIAL_ARTS, P_MASTER },
    { P_NONE, 0 }
};

#ifdef TOURIST
static const struct def_skill Skill_T[] = {
    { P_DAGGER, P_BASIC },		{ P_KNIFE,  P_BASIC },
    { P_AXE, P_BASIC },			{ P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_BASIC },	{ P_BROAD_SWORD, P_BASIC },
    { P_LONG_SWORD, P_BASIC },		{ P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_BASIC },		{ P_SABER, P_EXPERT },
    { P_MACE, P_BASIC },		{ P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_BASIC },		{ P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_BASIC },	{ P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },		{ P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },		{ P_LANCE, P_BASIC },
    { P_BOW, P_BASIC },			{ P_SLING, P_BASIC },
//#ifdef FIREARMS
    { P_FIREARM, P_BASIC },
//#endif
    { P_CROSSBOW, P_BASIC },		{ P_DART, P_EXPERT },
    { P_SHURIKEN, P_BASIC },		{ P_BOOMERANG, P_EXPERT },
    { P_WHIP, P_BASIC },		{ P_HARVEST, P_SKILLED },
    { P_UNICORN_HORN, P_SKILLED },
    { P_DIVINATION_SPELL, P_SKILLED },	{ P_ENCHANTMENT_SPELL, P_BASIC },
    { P_ESCAPE_SPELL, P_SKILLED },
    { P_WAND_POWER, P_SKILLED },
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
#ifdef BARD
    { P_MUSICALIZE, P_SKILLED },
#endif
    { P_TWO_WEAPON_COMBAT, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_BEAST_MASTERY, P_SKILLED },
    { P_NONE, 0 }
};
#endif /* TOURIST */

static const struct def_skill Skill_V[] = {
    { P_DAGGER, P_BASIC },			{ P_AXE, P_EXPERT },
    { P_PICK_AXE, P_SKILLED },		{ P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_SKILLED },	{ P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_EXPERT },{ P_SCIMITAR, P_BASIC },
    { P_SABER, P_BASIC },			{ P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_BASIC },	{ P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_EXPERT },			{ P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },			{ P_LANCE, P_SKILLED },
    { P_SLING, P_BASIC },			{ P_BOW, P_EXPERT },
    { P_ATTACK_SPELL, P_EXPERT },	{ P_ESCAPE_SPELL, P_BASIC },
    { P_DIVINATION_SPELL, P_SKILLED },
    { P_WAND_POWER, P_BASIC },
#ifdef STEED
    { P_RIDING, P_SKILLED },
#endif
#ifdef BARD
    { P_MUSICALIZE, P_SKILLED },
#endif
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static const struct def_skill Skill_W[] = {
    { P_DAGGER, P_EXPERT },		{ P_KNIFE,  P_EXPERT },
    { P_AXE, P_BASIC },		{ P_SHORT_SWORD, P_BASIC },
    { P_CLUB, P_SKILLED },		{ P_MACE, P_BASIC },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_BASIC },		{ P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },		{ P_SLING, P_SKILLED },
    { P_DART, P_SKILLED },		{ P_SHURIKEN, P_BASIC },
    { P_CROSSBOW, P_SKILLED },
    { P_ATTACK_SPELL, P_EXPERT },	{ P_HEALING_SPELL, P_SKILLED },
    { P_DIVINATION_SPELL, P_EXPERT },	{ P_ENCHANTMENT_SPELL, P_SKILLED },
    { P_CLERIC_SPELL, P_SKILLED },	{ P_ESCAPE_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
    { P_WAND_POWER, P_SKILLED },
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_BEAST_MASTERY, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};

static const char *oseVowels[] = {"a","e","i","o","u","ae","oe","oo","y"};
static const char *oseConsonants[] = {"b","c","d","f","g","h","j","k","l","m","n","p","q","r","s","t","v","w","x","y","z","ch","ll","sh","th"};

STATIC_OVL void
knows_object(obj)
register int obj;
{
	discover_object(obj,TRUE,FALSE);
	objects[obj].oc_pre_discovered = 1;	/* not a "discovery" */
}

/* Know ordinary (non-magical) objects of a certain class,
 * like all gems except the loadstone and luckstone.
 */
STATIC_OVL void
knows_class(sym)
register char sym;
{
	register int ct;
	for (ct = 1; ct < NUM_OBJECTS; ct++)
		if (objects[ct].oc_class == sym && !objects[ct].oc_magic)
			knows_object(ct);
}

#ifdef BARD
/* Know 5 to 15 random magical objects (wands, potions, scrolls, ...)
   For now we decide that tools while possibly magical (bag of holding/tricks,
   magic lamp) are excempt because the Bard already knows all instruments
   which appear to be more than enough tools.
   We might also add GEM_CLASS with oc_material != GLASS 
*** Contributed by Johanna Ploog */
STATIC_OVL void
know_random_obj()
{
        register int obj, count, ct;

        count = rn1(11,5);
        for (ct = 500; ct > 0 && count > 0; ct--) {
           obj = rn2(NUM_OBJECTS);
           if (objects[obj].oc_magic &&

               /* We have to make an exception for those dummy
                  objects (wand and scroll) that exist to allow
                  for additional descriptions. */

               obj_descr[(objects[obj].oc_name_idx)].oc_name != 0 &&

              ((objects[obj].oc_class == ARMOR_CLASS &&

               /* Dragon scales and mails are considered magical,
                  but as they don't have different descriptions,
                  they don't appear in the discovery list,
                  so as not to rob the player of an opportunity... */

                !(obj > HELM_OF_TELEPATHY && obj < PLATE_MAIL)) ||

               objects[obj].oc_class == RING_CLASS ||
               objects[obj].oc_class == POTION_CLASS ||
               objects[obj].oc_class == SCROLL_CLASS ||
               objects[obj].oc_class == SPBOOK_CLASS ||
               objects[obj].oc_class == WAND_CLASS ||
               objects[obj].oc_class == AMULET_CLASS))
            {
              knows_object(obj);
              count--;
            }
        }
}
#endif

int randMeleeAttackTypes[] = 
						{AT_CLAW, 
						 AT_BITE, 
						 AT_KICK, 
						 AT_BUTT, 
						 AT_TUCH, 
						 AT_STNG, 
						 AT_TENT, 
						 AT_WHIP, 
						 AT_LNCK, 
						 AT_LRCH, 
						 AT_WEAP };

int randSpecialAttackTypes[] = 
						{AT_SPIT, 
						 AT_HUGS, 
						 AT_GAZE, 
						 AT_ENGL, 
						 AT_ARRW, 
						 AT_BREA, 
						 AT_BEAM, 
						 AT_5SQR, 
						 AT_DEVA, 
						 AT_MAGC, 
						 AT_HODS };

int randMeleeDamageTypes[] = 
						{AD_PHYS, 
						 AD_SHDW, 
						 AD_STAR, 
						 AD_BLUD, 
						 AD_FIRE, 
						 AD_COLD,
						 AD_SLEE,
						 AD_ELEC,
						 AD_DRST,
						 AD_ACID,
						 AD_BLND,
						 AD_STUN,
						 AD_SLOW,
						 AD_PLYS,
						 AD_DRLI,
						 AD_DREN,
						 AD_LEGS,
						 AD_STON,
						 AD_STCK,
						 AD_SGLD,
						 AD_SITM,
						 // AD_SEDU,
						 AD_TLPT,
						 AD_RUST,
						 AD_CONF,
						 // AD_DGST,
						 // AD_HEAL,
						 AD_WRAP,
						 // AD_WERE,
						 AD_DRDX,
						 AD_DRCO,
						 AD_DRIN,
						 AD_NPDC,
						 AD_DISE,
						 // AD_SSEX,
						 AD_HALU,
						 // AD_,DETH
						 // AD_FAMN,
						 // AD_PEST,
						 AD_SLIM,
						 AD_ENCH,
						 AD_CORR,
						 AD_POSN,
						 AD_WISD,
						 AD_VORP,
						 AD_SHRD,
						 AD_TCKL,
						 AD_WET,
						 // AD_LETHE,
						 AD_SUCK,
						 AD_MALK,
						 AD_UVUU,
						 AD_ABDC,
						 AD_TELE,
						 AD_CHRN,
						 AD_LVLT,
						 AD_WEEP };

int randBreathDamageTypes[] = 
						{AD_RBRE, 
						 AD_MAGM, 
						 AD_COLD, 
						 AD_DRST, 
						 AD_FIRE, 
						 AD_SLEE, 
						 AD_ELEC, 
						 AD_ACID, 
						 AD_DISN };

int randBeamDamageTypes[] = 
						{AD_MAGM, 
						 AD_COLD, 
						 AD_DRST, 
						 AD_FIRE, 
						 AD_SLEE, 
						 AD_ELEC, 
						 AD_ACID,
						 AD_BLND,
						 AD_STUN,
						 AD_PLYS,
						 AD_DRLI,
						 AD_DREN,
						 AD_STON,
						 AD_SGLD,
						 AD_SITM,
						 AD_TLPT,
						 AD_RUST,
						 AD_CONF,
						 AD_DISE,
						 AD_DCAY,
						 AD_HALU,
						 AD_SLIM,
						 AD_ENCH,
						 AD_CORR,
						 AD_POSN,
						 AD_SHRD,
						 AD_ABDC,
						 AD_TELE,
						 AD_POLY,
						 AD_CHRN,
						 AD_LVLT,
						 AD_VAMP,
						 AD_WEBS,
						 AD_SHDW,
						 AD_STTP };

int randSpitDamageTypes[] = 
						{AD_BLND, 
						 AD_ACID, 
						 AD_DRST };

int randGazeDamageTypes[] = 
						{AD_STON, 
						 AD_SITM, 
						 AD_DEAD, 
						 AD_CNCL, 
						 AD_PLYS, 
						 AD_DRLI, 
						 AD_ENCH, 
						 AD_CONF, 
						 AD_SLOW, 
						 AD_STUN, 
						 AD_BLND, 
						 AD_FIRE, 
						 AD_COLD, 
						 AD_ELEC, 
						 AD_HALU, 
						 AD_SLEE,
						 AD_LUCK,
						 AD_RGAZ,
						 AD_MIST };
 // AT_ARRW, 
 // AT_MAGC }
int randEngulfDamageTypes[] = 
						{AD_DISE, 
						 AD_ACID, 
						 AD_DGST, 
						 AD_PHYS, 
						 AD_BLND, 
						 AD_COLD, 
						 AD_ELEC, 
						 AD_FIRE };

int randArrowDamageTypes[] = 
						{AD_PHYS, /*Phys uses tracked arrows rather than generated ones*/
						 AD_LOAD, 
						 AD_VBLD, 
						 AD_BALL, 
						 AD_BLDR, 
						 AD_SHDW, 
						 AD_BLDR };

 // AT_MAGC }
int randMagicDamageTypes[] = 
						{AD_SPEL, 
						 AD_SPEL,
						 AD_SPEL,
						 AD_MAGM,
						 AD_FIRE,
						 AD_COLD,
						 AD_ELEC,
						 AD_CLRC,
						 AD_CLRC,
						 AD_CLRC };
int randCorpseWeights[] = 
						{WT_TINY,
						 WT_SMALL,
						 WT_MEDIUM,
						 WT_LARGE,
						 WT_HUGE,
						 0,
						 0,
						 WT_GIGANTIC };
int randCorpseNut[] = 
						{CN_TINY,
						 CN_SMALL,
						 CN_MEDIUM,
						 CN_LARGE,
						 CN_HUGE,
						 0,
						 0,
						 CN_GIGANTIC };
void
u_init()
{
	register int i;
	extern int monstr[];
	struct permonst * horrors[] = { &mons[PM_SHAMBLING_HORROR], &mons[PM_STUMBLING_HORROR], &mons[PM_WANDERING_HORROR] };
	struct attack* attkptr;
	int horrorattacks;

	flags.female = flags.initgend;
	flags.beginner = 1;

	/* zero u, including pointer values --
	 * necessary when aborting from a failed restore */
	(void) memset((genericptr_t)&u, 0, sizeof(u));
	u.ustuck = (struct monst *)0;

	u.summonMonster = FALSE;
	u.uleadamulet = FALSE;
	u.ZangetsuSafe = 1;
	u.ucspeed = NORM_CLOCKSPEED;
	u.voidChime = 0;
	u.regifted = 0;
	u.keter = 0;
	u.chokhmah = 0;
	u.binah = 0;
	u.gevurah = 0;
	u.hod = 0;
	u.daat = 0;
	u.wardsknown = 0;
	u.spells_maintained = 0;
	u.maintained_en_debt = 0;
	//u.wardsknown = ~0; //~0 should be all 1s, and is therefore debug mode.

#if 0	/* documentation of more zero values as desirable */
	u.usick_cause[0] = 0;
	u.uluck  = u.moreluck = 0;
# ifdef TOURIST
	uarmu = 0;
# endif
	uarm = uarmc = uarmh = uarms = uarmg = uarmf = 0;
	uwep = uball = uchain = uleft = uright = urope = 0;
	uswapwep = uquiver = 0;
	u.twoweap = 0;
	u.ublessed = 0;				/* not worthy yet */
	for(i=0;i<SIZE(u.ugangr);i++){
		u.ugangr[i] = 0;				/* gods not angry */
	}
	u.ugifts   = 0;				/* no divine gifts bestowed */
	u.ucarinc = 0;
	u.uacinc = 0;
// ifdef ELBERETH
	u.uevent.uhand_of_elbereth = 0;
// endif
	u.uevent.utook_castle = 0;
	u.uevent.uunknowngod = 0;
	u.uevent.uheard_tune = 0;
	u.uevent.uopened_dbridge = 0;
	u.uevent.udemigod = 0;		/* not a demi-god yet... */
	u.udg_cnt = 0;
	u.ill_cnt = 0;
	u.mh = u.mhmax = u.mtimedone = 0;
	u.uz.dnum = u.uz0.dnum = 0;
	u.utotype = 0;
#endif	/* 0 */

	u.uz.dlevel = 1;
	u.uz0.dlevel = 0;
	u.utolev = u.uz;

	u.umoved = FALSE;
	u.umortality = 0;
	u.ugrave_arise = Role_if(PM_PIRATE) ? PM_SKELETAL_PIRATE : 
					 Role_if(PM_EXILE) ? PM_BROKEN_SHADOW 
						: NON_PM;
	
	u.ukinghill = 0;
	u.protean = 0;
	u.divetimer = (ACURR(A_CON))/3;
	
	u.role_variant = 0;
	
	u.uhouse = 0;
	u.start_house = 0;
	
	u.uaesh = 0;
	u.uaesh_duration = 0;
	u.ukrau = 0;
	u.ukrau_duration = 0;
	u.uhoon = 0;
	u.uhoon_duration,
	u.uuur = 0;
	u.uuur_duration = 0;
	u.unaen = 0;
	u.unaen_duration = 0;
	u.uvaul = 0;
	u.uvaul_duration = 0;
	u.ufirst_light = FALSE;
	u.ufirst_light_timeout = 0;
	u.ufirst_sky = FALSE;
	u.ufirst_sky_timeout = 0;
	u.ufirst_life = FALSE;
	u.ufirst_life_timeout = 0;
	
	/*Randomize spirit order*/{
		int i,j,tmp;
		for(i=0;i<31;i++) u.sealorder[i]=i;
		for(i=0;i<31;i++){
			j=rn2(31);
			tmp = u.sealorder[i];
			u.sealorder[i] = u.sealorder[j];
			u.sealorder[j] = tmp;
		}
	}
	if(Role_if(PM_EXILE)){
		u.sealsKnown = sealKey[u.sealorder[0]] | sealKey[u.sealorder[1]] | sealKey[u.sealorder[2]];
	}
	else 	u.sealsKnown = 0L;
	
	u.specialSealsKnown = 0L;
	
	u.sealCounts = 0;
	u.sealsActive = 0;
	u.specialSealsActive = 0;

	u.sealTimeout[AHAZU-FIRST_SEAL] = u.sealTimeout[AMON-FIRST_SEAL] = u.sealTimeout[ANDREALPHUS-FIRST_SEAL] = u.sealTimeout[ANDROMALIUS-FIRST_SEAL] = u.sealTimeout[ASTAROTH-FIRST_SEAL] = u.sealTimeout[BALAM-FIRST_SEAL] = u.sealTimeout[BERITH-FIRST_SEAL] = u.sealTimeout[BUER-FIRST_SEAL] = u.sealTimeout[CHUPOCLOPS-FIRST_SEAL] = u.sealTimeout[DANTALION-FIRST_SEAL] = u.sealTimeout[SHIRO-FIRST_SEAL] = 0;
	u.sealTimeout[ECHIDNA-FIRST_SEAL] = u.sealTimeout[EDEN-FIRST_SEAL] = u.sealTimeout[ENKI-FIRST_SEAL] = u.sealTimeout[EURYNOME-FIRST_SEAL] = u.sealTimeout[EVE-FIRST_SEAL] = u.sealTimeout[FAFNIR-FIRST_SEAL] = u.sealTimeout[HUGINN_MUNINN-FIRST_SEAL] = u.sealTimeout[IRIS-FIRST_SEAL] = u.sealTimeout[JACK-FIRST_SEAL] = u.sealTimeout[MALPHAS-FIRST_SEAL] = u.sealTimeout[MARIONETTE-FIRST_SEAL] = u.sealTimeout[MOTHER-FIRST_SEAL] = 0;
	u.sealTimeout[NABERIUS-FIRST_SEAL] = u.sealTimeout[ORTHOS-FIRST_SEAL] = u.sealTimeout[OSE-FIRST_SEAL] = u.sealTimeout[OTIAX-FIRST_SEAL] = u.sealTimeout[PAIMON-FIRST_SEAL] = u.sealTimeout[SIMURGH-FIRST_SEAL] = u.sealTimeout[TENEBROUS-FIRST_SEAL] = u.sealTimeout[YMIR-FIRST_SEAL] = u.sealTimeout[DAHLVER_NAR-FIRST_SEAL] = u.sealTimeout[ACERERAK-FIRST_SEAL] = 0;
	
	u.osepro[0] = '\0';
	u.osegen[0] = '\0';
	
	u.wisSpirits = 0;
	u.intSpirits = 0;
	
	if(FALSE){
		if(flags.female) Sprintf(u.osepro,"he");
		else Sprintf(u.osepro,"she");
		
	if(rn2(20)){
			if(urace.individual.m){
				if(flags.female) Sprintf1(u.osegen,urace.individual.m);
				else Sprintf1(u.osegen,urace.individual.f);
			} else {
				Sprintf1(u.osegen,urace.noun);
			}
		} else {
			int rndI = randrace(flags.initrole);
			if(races[rndI].individual.m){
				if(flags.female) Sprintf1(u.osegen,races[rndI].individual.m);
				else Sprintf1(u.osegen,races[rndI].individual.f);
			} else {
				Sprintf1(u.osegen,races[rndI].noun);
			}
		}
	} else if(rn2(20)){
		if(!flags.female) Sprintf(u.osepro,"he");
		else Sprintf(u.osepro,"she");
		
		if(rn2(20)){
			if(urace.individual.m){
				if(flags.female) Sprintf1(u.osegen,urace.individual.f);
				else Sprintf1(u.osegen,urace.individual.m);
			} else {
				Sprintf1(u.osegen,urace.noun);
			}
		} else {
			int rndI = randrace(flags.initrole);
			if(races[rndI].individual.m){
				if(flags.female) Sprintf1(u.osegen,races[rndI].individual.f);
				else Sprintf1(u.osegen,races[rndI].individual.m);
			} else {
				Sprintf1(u.osegen,races[rndI].noun);
			}
		}
	} else{
		int i, lets = rnd(2) + rn2(2);
		Strcat(u.osepro, oseConsonants[rn2(SIZE(oseConsonants))]);
		for(i=0; i<lets;i++){
			if(i%2) Strcat(u.osepro, oseConsonants[rn2(SIZE(oseConsonants))]);
			else Strcat(u.osepro, oseVowels[rn2(SIZE(oseVowels))]);
		}
		lets = rnd(5);
		Strcat(u.osegen, oseConsonants[rn2(SIZE(oseConsonants))]);
		for(i=0; i<lets;i++){
			if(i%2) Strcat(u.osegen, oseConsonants[rn2(SIZE(oseConsonants))]);
			else Strcat(u.osegen, oseVowels[rn2(SIZE(oseVowels))]);
		}
	}
	
	u.irisAttack = u.otiaxAttack = 0;
	
	u.spirit[0] = u.spirit[1] = u.spirit[2] = u.spirit[3] = u.spirit[4] = u.spirit[5] = u.spirit[6] = u.spiritTineA = u.spiritTineB = 0;
	u.spiritT[0] = u.spiritT[1] = u.spiritT[2] = u.spiritT[3] = u.spiritT[4] = u.spiritT[5] = u.spiritT[6] = u.spiritTineTA = u.spiritTineTB = 0;
	
	for(i = 0; i<52; i++) u.spiritPOrder[i] = -1;
	for(i = 0; i<NUMBER_POWERS; i++) u.spiritPColdowns[i] = 0;
	
	u.umonnum = u.umonster = (flags.female &&
			urole.femalenum != NON_PM) ? urole.femalenum :
			urole.malenum;
	init_uasmon();

	u.ulevel = 0;	/* set up some of the initial attributes */
	u.uhp = u.uhpmax = newhp();
	u.uenmax = urole.enadv.infix + urace.enadv.infix;
	if (urole.enadv.inrnd > 0)
	    u.uenmax += rnd(urole.enadv.inrnd);
	if (urace.enadv.inrnd > 0)
	    u.uenmax += rnd(urace.enadv.inrnd);
	u.uen = u.uenmax;
	u.uspellprot = 0;
	u.sowdisc = 0;
	u.voidChime = 0;
	adjabil(0,1);
	u.ulevel = u.ulevelmax = 1;
	
	u.exerchkturn = 600L; /*Turn to first check for attribute gain from exercise*/
	
	init_uhunger();
	for (i = 0; i <= MAXSPELL; i++) spl_book[i].sp_id = NO_SPELL;
	u.ublesscnt = 300;			/* no prayers just yet */
	u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] = u.ualign.type = aligns[flags.initalign].value;
	u.ulycn = NON_PM;

#if defined(BSD) && !defined(POSIX_TYPES)
	(void) time((long *)&u.ubirthday);
#else
	(void) time(&u.ubirthday);
#endif

	/*
	 *  For now, everyone starts out with their xray range disabled.
	 */
	u.nv_range   =  urace.nv_range;
	u.xray_range = -1;


	/*** Role-specific initializations ***/
	switch (Role_switch) {
	/* rn2(100) > 50 necessary for some choices because some
	 * random number generators are bad enough to seriously
	 * skew the results if we use rn2(2)...  --KAA
	 */
	case PM_ARCHEOLOGIST:
		ini_inv(Archeologist);
		ini_inv(Tinopener);
		ini_inv(Lamp);
		// else if(!rn2(10)) ini_inv(Magicmarker);
		knows_object(SACK);
		knows_object(TOUCHSTONE);
		skill_init(Skill_A);
	break;
	case PM_ANACHRONONAUT:
		if(Race_if(PM_MYRKALFR) && !flags.female){
			ini_inv(Anachrononaut_Dro);
		} else if(Race_if(PM_MYRKALFR) && flags.female){
			ini_inv(Anachrononaut_ShDro);
			bindspirit(BLACK_WEB);
		} else if(Race_if(PM_ELF)) ini_inv(Anachrononaut_Elf);
		else if(Race_if(PM_INCANTIFIER)) ini_inv(Anachrononaut_Inc);
		else if(Race_if(PM_VAMPIRE)) ini_inv(Anachrononaut_Vam);
		else if(Race_if(PM_DWARF)) ini_inv(Anachrononaut_Dw);
		else if(Race_if(PM_HALF_DRAGON)) ini_inv(Anachrononaut_Hlf);
		else if(Race_if(PM_ANDROID)){
			if(!flags.female) ini_inv(Anachrononaut_Mal_Clk);
			else ini_inv(Anachrononaut_Fem_Clk);
			scatter_weapons();
		}
		else ini_inv(Anachrononaut_Hu);
		knows_object(FLINTLOCK);
		knows_object(PISTOL);
		knows_object(MASS_SHADOW_PISTOL);
		knows_object(SUBMACHINE_GUN);
		knows_object(HEAVY_MACHINE_GUN);
		knows_object(RIFLE);
		knows_object(ASSAULT_RIFLE);
		knows_object(SNIPER_RIFLE);
		knows_object(SHOTGUN);
		knows_object(AUTO_SHOTGUN);
		knows_object(ROCKET_LAUNCHER);
		knows_object(GRENADE_LAUNCHER);
		knows_object(BFG);
		knows_object(HAND_BLASTER);
		knows_object(ARM_BLASTER);
		knows_object(CUTTING_LASER);
		knows_object(RAYGUN);
		knows_object(BULLET);
		knows_object(SILVER_BULLET);
		knows_object(SHOTGUN_SHELL);
		knows_object(ROCKET);
		knows_object(FRAG_GRENADE);
		knows_object(GAS_GRENADE);
		knows_object(STICK_OF_DYNAMITE);
		knows_object(HEAVY_BLASTER_BOLT);
		knows_object(BLASTER_BOLT);
		knows_object(LASER_BEAM);
		knows_object(BULLET_FABBER);
		knows_object(SENSOR_PACK);
		knows_object(HYPOSPRAY);
		knows_object(HYPOSPRAY_AMPULE);
		knows_object(POWER_PACK);
		knows_object(PROTEIN_PILL);
		knows_object(FORCE_PIKE);
		knows_object(VIBROBLADE);
		knows_object(WHITE_VIBROSWORD);
		knows_object(GOLD_BLADED_VIBROSWORD);
		knows_object(RED_EYED_VIBROSWORD);
		knows_object(WHITE_VIBROSPEAR);
		knows_object(WHITE_VIBROZANBATO);
		knows_object(GOLD_BLADED_VIBROSPEAR);
		knows_object(GOLD_BLADED_VIBROZANBATO);
		knows_object(SEISMIC_HAMMER);
		knows_object(LIGHTSABER);
		knows_object(BEAMSWORD);
		knows_object(DOUBLE_LIGHTSABER);
		if(Race_if(PM_DWARF)){
			u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] = u.ualign.type = A_CHAOTIC;
			flags.initalign = 2; // 2 == chaotic
		}
		if(Race_if(PM_ANDROID)){
			skill_init(Skill_Droid_Ana);
			u.umartial = TRUE;
		} else {
			skill_init(Skill_Ana);
			skill_add(Skill_All_Ana);
		}
	break;
	case PM_BARBARIAN:
		u.role_variant = TWO_HANDED_SWORD;
		if (rn2(100) >= 50) {	/* see above comment */
			u.role_variant = BATTLE_AXE;
		    Barbarian[B_MAJOR].trotyp = BATTLE_AXE;
		    Barbarian[B_MINOR].trotyp = SHORT_SWORD;
		}
		ini_inv(Barbarian);
		// if(!rn2(6)) ini_inv(Lamp);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_B);
		break;
#ifdef BARD
	case PM_BARD:
		if (Race_if(PM_ORC)) Bard[BARD_INSTR].trotyp = (rn2(100) >= 50) ? BUGLE : TOOLED_HORN;
		else if (Race_if(PM_HALF_DRAGON)) Bard[BARD_INSTR].trotyp = LEATHER_DRUM;
		else if (rn2(100) >= 50) Bard[BARD_INSTR].trotyp = WOODEN_FLUTE;
		if (rn2(100) >= 85) Bard[BARD_WHISTLE].trotyp = BELL;
		if (Race_if(PM_DROW)) Bard[BARD_CLOAK].trotyp = DROVEN_CHAIN_MAIL;
		Bard[BARD_BOOZE].trquan = rn1(2, 5);
		ini_inv(Bard);
		if(Race_if(PM_DROW)){
			BlackTorches[0].trquan = 6;
			ini_inv(BlackTorches);
		}
		if(Race_if(PM_CLOCKWORK_AUTOMATON)){
			u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] = u.ualign.type = A_LAWFUL;
			flags.initalign = 0; // 0 == lawful
		}
		/* This depends on the order in objects.c */
		for (i = TIN_WHISTLE; i <= DRUM_OF_EARTHQUAKE; i++)
			knows_object(i);
		/* Bards know about the enchantment spellbooks, though they don't know
		   the spells */
		knows_object(SPE_SLEEP);
		knows_object(SPE_CONFUSE_MONSTER);
		knows_object(SPE_SLOW_MONSTER);
		knows_object(SPE_CAUSE_FEAR);
		knows_object(SPE_CHARM_MONSTER);
		/* Bards also know a lot about legendary & magical stuff. */
		know_random_obj();
		/* Bards also know all the basic wards. */
		u.wardsknown = WARD_ACHERON|WARD_HAMSA|WARD_ELDER_SIGN|WARD_EYE|WARD_QUEEN|WARD_CAT_LORD|WARD_GARUDA;
		u.wardsknown |= WARD_TOUSTEFNA;
		u.wardsknown |= WARD_DREPRUN;
		skill_init(Skill_Bard);
	break;
#endif
	case PM_EXILE:
		ini_inv(Binder);
		skill_init(Skill_N);
		knows_object(FLINT);
		knows_object(SHEPHERD_S_CROOK);
		if(Race_if(PM_INCANTIFIER)){
			knows_object(SPE_HEALING);
			knows_object(SPE_FORCE_BOLT);
		}
    	u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] =
			u.ualign.type = A_VOID; /* Override racial alignment */
		flags.initalign = 4; // 4 == VOID
		u.hod += 10;  /*One transgression is all it takes*/
		u.gevurah += 5; /*One resurection or two rehumanizations is all it takes*/
		u.daat += 8;
	break;
	case PM_CAVEMAN:
		Cave_man[C_AMMO].trquan = rn1(11, 10);	/* 10..20 */
		ini_inv(Cave_man);
		skill_init(Skill_C);
		break;
#ifdef CONVICT
	case PM_CONVICT:
        ini_inv(Convict);
        knows_object(SKELETON_KEY);
        knows_object(GRAPPLING_HOOK);
        skill_init(Skill_Con);
		u.hod += 6;
		u.ualign.sins += 16; /* You have sinned */
        /* On the verge of hungry */
		if(Race_if(PM_INCANTIFIER)){
			u.uen = 600;
		}
		else u.uhunger = 200;
    	u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] =
			u.ualign.type = A_CHAOTIC; /* Override racial alignment */
		flags.initalign = 2; // 2 == chaotic
        urace.hatemask |= urace.lovemask;   /* Hated by the race's allies */
        urace.lovemask = 0; /* Convicts are pariahs of their race */
        break;
#endif	/* CONVICT */
	case PM_HEALER:
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = rn1(1000, 1001);
#else
		u.umoney0 = rn1(1000, 1001);
#endif
		ini_inv(Healer);
		ini_inv(Lamp);
		knows_object(POT_FULL_HEALING);
		skill_init(Skill_H);
		break;
	case PM_KNIGHT:
		if(Race_if(PM_DWARF)) ini_inv(DwarfNoble);
		else ini_inv(Knight);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		/* give knights chess-like mobility
		 * -- idea from wooledge@skybridge.scl.cwru.edu */
		HJumping |= FROMOUTSIDE;
		if(Race_if(PM_DWARF)) skill_init(Skill_DwaNob);
		else skill_init(Skill_K);
		break;
	case PM_MONK:
		u.umartial = TRUE;
		switch (rn2(90) / 30) {
		case 0: Monk[M_BOOK].trotyp = SPE_HEALING; break;
		case 1: Monk[M_BOOK].trotyp = SPE_PROTECTION; break;
		case 2: Monk[M_BOOK].trotyp = SPE_SLEEP; break;
		}
		ini_inv(Monk);
		// if(!rn2(5)) ini_inv(Magicmarker);
		// else if(!rn2(10)) ini_inv(Lamp);
		// knows_class(ARMOR_CLASS);
		skill_init(Skill_Mon);
		break;
	case PM_NOBLEMAN:
		if(Race_if(PM_DWARF)) ini_inv(DwarfNoble);
		else if(Race_if(PM_HALF_DRAGON)){
			if(flags.initgend){
				ini_inv(HDNobleF);
			} else {
				ini_inv(HDNobleM);
			}
		}
		else if(Race_if(PM_DROW) && flags.female){
			DNoble[DNB_TWO_ARROWS].trquan = rn1(10, 50);
			DNoble[DNB_ZERO_ARROWS].trquan = rn1(10, 30);
			ini_inv(DNoble);
			ini_inv(DarkWand);
		} else{
			if(flags.female && !Race_if(PM_DROW)){
				Noble[NOB_SHIRT].trotyp = VICTORIAN_UNDERWEAR;
				Noble[NOB_SHIRT].trspe = 1;
				Noble[NOB_SUIT].trotyp = GENTLEWOMAN_S_DRESS;
				Noble[NOB_SUIT].trspe = 1;
				Noble[NOB_SHOES].trotyp = STILETTOS;
				Noble[NOB_SHOES].trspe = 1;
			} else if(!flags.female && Race_if(PM_DROW)){
				Noble[NOB_SHOES].trotyp = STILETTOS;
				Noble[NOB_SHOES].trspe = 1;
			}
			ini_inv(Noble);
		}
		
		if(Race_if(PM_ELF)){
			ini_inv(Phial);
		} else if(Race_if(PM_DROW) && !flags.female){
			ini_inv(BlackTorches);
		}
		// knows_class(ARMOR_CLASS);
		if(Race_if(PM_DROW) && flags.female) skill_init(Skill_DNob);
		else if(Race_if(PM_DWARF)) skill_init(Skill_DwaNob);
		else if(Race_if(PM_HALF_DRAGON) && Role_if(PM_NOBLEMAN) && flags.initgend) 
			skill_init(Skill_HD_Female_Nob);
		else if(Race_if(PM_ELF)) skill_init(Skill_ENob);
		else skill_init(Skill_Nob);
	break;
	case PM_PIRATE:
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = rnd(300);
#else
		u.umoney0 = rnd(300);
#endif
		if(Race_if(PM_DROW)) Pirate[PIR_KNIVES].trotyp = DROVEN_CROSSBOW;
		else Pirate[PIR_KNIVES].trquan = rn1(3, 4);
		if(!rn2(4)) Pirate[PIR_SNACK].trotyp = KELP_FROND;
		Pirate[PIR_SNACK].trquan += rn2(4);
		Pirate[PIR_BULLETS].trquan += rn2(Pirate[PIR_BULLETS].trquan/2);
		if(rn2(100)<50)	Pirate[PIR_JEWELRY].trotyp = RIN_ADORNMENT;
		ini_inv(Pirate);
		if(Race_if(PM_DROW)){
			struct obj *otmp;
			otmp = mksobj(CROSSBOW_BOLT, TRUE, FALSE);
			otmp->quan = rn1(12, 16);
			otmp->spe = otmp->cursed = otmp->blessed = 0;
			otmp->known = otmp->dknown = otmp->bknown = otmp->rknown = otmp->sknown = 1;
			addinv(otmp);
			ini_inv(BlackTorches);
		}
		knows_object(SACK);
		knows_object(OILSKIN_SACK);
		knows_object(OILSKIN_CLOAK);
		knows_object(GRAPPLING_HOOK);
		skill_init(Skill_Pir);
		break;
	case PM_PRIEST:
		if(!(flags.female) && Race_if(PM_DROW)){
			Priest[PRI_WEAPON].trotyp = DROVEN_GREATSWORD;
		}
		if(flags.female && Race_if(PM_DROW)) ini_inv(DPriest);
		else ini_inv(Priest);
		if(Race_if(PM_DROW)){
			if(flags.female){
				ini_inv(DrainBook);
				ini_inv(FamBook);
			}
			ini_inv(DarkWand);
		}
		// if(!rn2(10)) ini_inv(Magicmarker);
		// else if(!rn2(10)) ini_inv(Lamp);
		knows_object(POT_WATER);
		if(Race_if(PM_DROW)){
			if(!flags.female) skill_init(Skill_Dro_M_P);
			else skill_init(Skill_Dro_F_P);
		} else skill_init(Skill_P);
		if(Race_if(PM_DROW) && flags.female){
			skill_add(Skill_DP);
			ini_inv(ExtraBook);
		}
		/* KMH, conduct --
		 * Some may claim that this isn't agnostic, since they
		 * are literally "priests" and they have holy water.
		 * But we don't count it as such.  Purists can always
		 * avoid playing priests and/or confirm another player's
		 * role in their YAAP.
		 */
		break;
	case PM_RANGER:
		Ranger[RAN_TWO_ARROWS].trquan = rn1(10, 50);
		Ranger[RAN_ZERO_ARROWS].trquan = rn1(10, 30);
		ini_inv(Ranger);
		skill_init(Skill_Ran);
		if(Race_if(PM_DROW)){
			ini_inv(BlackTorches);
		}
		break;
	case PM_ROGUE:
		Rogue[R_DAGGERS].trquan = rn1(10, 6);
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = 0;
#else
		u.umoney0 = 0;
#endif
		ini_inv(Rogue);
		ini_inv(Blindfold);
		knows_object(SACK);
		skill_init(Skill_R);
		break;
	case PM_SAMURAI:
		u.umartial = TRUE;
		Samurai[S_ARROWS].trquan = rn1(20, 26);
		u.role_variant = KATANA;
		if(flags.female){
			Samurai[S_WEAPON].trotyp = NAGINATA;
			Samurai[S_SECOND].trotyp = KNIFE;
			u.role_variant = NAGINATA;
		}
		ini_inv(Samurai);
		ini_inv(Blindfold);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_S);
		break;
#ifdef TOURIST
	case PM_TOURIST:
		Tourist[T_DARTS].trquan = rn1(20, 21);
#ifndef GOLDOBJ
		u.ugold = u.ugold0 = rnd(1000);
#else
		u.umoney0 = rnd(1000);
#endif
		ini_inv(Tourist);
		ini_inv(Leash);
		ini_inv(Towel);
		// else if(!rn2(25)) ini_inv(Magicmarker);
		skill_init(Skill_T);
		break;
#endif
	case PM_VALKYRIE:
		ini_inv(Valkyrie);
		// if(!rn2(6)) ini_inv(Lamp);
		knows_class(WEAPON_CLASS);
		// knows_class(ARMOR_CLASS);
		skill_init(Skill_V);
		break;
	case PM_WIZARD:
		if(flags.female && Race_if(PM_DROW)){
			Wizard[W_WEAPON].trotyp = KHAKKHARA;
			ini_inv(MRGloves);
		}
		ini_inv(Wizard);
		if(Race_if(PM_DROW)){
			if(flags.female){
				ini_inv(ExtraBook);
			}
			ini_inv(DarkWand);
		}
		skill_init(Skill_W);
		if(Race_if(PM_DROW) && flags.female) skill_add(Skill_DW);
		break;

	default:	/* impossible */
		break;
	}


	/*** Race-specific initializations ***/
	switch (Race_switch) {
	case PM_HUMAN:
	    /* Nothing special */
	break;

	case PM_CLOCKWORK_AUTOMATON:
		ini_inv(Key);
    break;

	case PM_INCANTIFIER:
		skill_add(Skill_I);
	    if (!Role_if(PM_WIZARD)) ini_inv(ForceBook);
		else ini_inv(ExtraBook);
	    if (!Role_if(PM_HEALER) && (!Role_if(PM_MONK) || Monk[M_BOOK].trotyp != SPE_HEALING)) ini_inv(HealingBook);
		else ini_inv(ExtraBook);
		if(Role_if(PM_ANACHRONONAUT)){
			u.umartial = TRUE;
			HTelepat |= FROMOUTSIDE;
			skill_add(Skill_Elf_Ana);
		}
    break;

	case PM_ELF:
	    /*
	     * Elves are people of music and song, or they are warriors.
	     * Non-warriors get an instrument.  We use a kludge to
	     * get only non-magic instruments.
	     */
	    if (Role_if(PM_PRIEST) || Role_if(PM_WIZARD)) {
		static int trotyp[] = {
		    WOODEN_FLUTE, TOOLED_HORN, WOODEN_HARP,
		    BELL, BUGLE, LEATHER_DRUM
		};
		Instrument[0].trotyp = trotyp[rn2(SIZE(trotyp))];
		ini_inv(Instrument);
	    }

#ifdef BARD
		if(!Role_if(PM_BARD)) skill_add(Skill_Elf_Music);
#endif
		if(Role_if(PM_ANACHRONONAUT)){
			u.umartial = TRUE;
			HTelepat |= FROMOUTSIDE;
			skill_add(Skill_Elf_Ana);
		}
	    /* Elves can recognize all elvish objects */
	    knows_object(ELVEN_SHORT_SWORD);
	    knows_object(ELVEN_ARROW);
	    knows_object(ELVEN_BOW);
	    knows_object(ELVEN_SPEAR);
	    knows_object(ELVEN_DAGGER);
	    knows_object(ELVEN_BROADSWORD);
	    knows_object(ELVEN_MACE);
	    knows_object(ELVEN_LANCE);
	    knows_object(ELVEN_MITHRIL_COAT);
	    knows_object(ELVEN_HELM);
	    knows_object(ELVEN_SHIELD);
	    knows_object(ELVEN_BOOTS);
	    knows_object(ELVEN_CLOAK);
	break;

	case PM_MYRKALFR:
	case PM_DROW:{
		struct obj* pobj;
		
	    /*
	     * All drow get signet rings. Pirates and wizards get them through their
		 * class equipment
	     */
		ini_inv(SleepPotions);
	    if (!Role_if(PM_PIRATE) && !Role_if(PM_WIZARD)) {
			ini_inv(RandRing);
	    }
		/*Drow can put a lot of practice into using their rings*/
		skill_add(Skill_Drow_Unarmed);
		
		if(Role_if(PM_NOBLEMAN)){
			if(!flags.female){
				u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] =
					u.ualign.type = A_NEUTRAL; /* Males are neutral */
				flags.initalign = 1; // 1 == neutral
			}
		} else if(!Role_if(PM_EXILE) && !Role_if(PM_CONVICT)){
			ini_inv(DrovenCloak);
			if(!flags.female){
				u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] =
					u.ualign.type = A_NEUTRAL; /* Males are neutral */
				flags.initalign = 1; // 1 == neutral
			}
		} else if(Role_if(PM_ANACHRONONAUT)){
			u.umartial = TRUE;
		}
	    /* Drow can recognize all droven objects */
	    knows_object(DROVEN_SHORT_SWORD);
	    knows_object(DROVEN_BOLT);
	    knows_object(DROVEN_CROSSBOW);
	    knows_object(DROVEN_DAGGER);
	    knows_object(DROVEN_GREATSWORD);
	    knows_object(DROVEN_LANCE);
	    knows_object(DROVEN_SPEAR);
	    knows_object(DROVEN_CHAIN_MAIL);
	    knows_object(DROVEN_PLATE_MAIL);
	    knows_object(NOBLE_S_DRESS);
	    knows_object(CONSORT_S_SUIT);
	    knows_object(DROVEN_CLOAK);
	    knows_object(find_signet_ring());
		
		if(Role_if(PM_ANACHRONONAUT)) u.uhouse = LAST_BASTION_SYMBOL;
		else u.uhouse = !(Role_if(PM_EXILE) || (Role_if(PM_NOBLEMAN) && !flags.initgend) || Role_if(PM_CONVICT) || Role_if(PM_PIRATE)) ?
				rn2(LAST_HOUSE+1-FIRST_HOUSE)+FIRST_HOUSE :
				rn2(LAST_FALLEN_HOUSE+1-FIRST_FALLEN_HOUSE)+FIRST_FALLEN_HOUSE;
		u.start_house = u.uhouse;
		for(pobj = invent; pobj; pobj=pobj->nobj){
			if(pobj->otyp == DROVEN_CHAIN_MAIL ||
				pobj->otyp == CONSORT_S_SUIT ||
				pobj->otyp == DROVEN_PLATE_MAIL){
					pobj->ohaluengr = TRUE;
					pobj->oward = !flags.female ? 
						u.uhouse :
						LOLTH_SYMBOL;
			}
			else if(isSignetRing(pobj->otyp)){
				pobj->ohaluengr = TRUE;
				pobj->oward = u.uhouse;
				pobj->opoisoned = OPOISON_SLEEP;
				pobj->opoisonchrgs = 30;
			}
			else if(is_poisonable(pobj)){
				pobj->opoisoned = OPOISON_SLEEP;
				if(pobj->otyp == VIPERWHIP){
					pobj->opoisonchrgs = 7;
				}
			}
		}
    }break;

	case PM_DWARF:{
		struct obj* otmp;
	    /* Dwarves can recognize all dwarvish objects */
	    knows_object(DWARVISH_SPEAR);
	    knows_object(DWARVISH_SHORT_SWORD);
	    knows_object(DWARVISH_MATTOCK);
	    knows_object(DWARVISH_HELM);
	    knows_object(DWARVISH_MITHRIL_COAT);
	    knows_object(DWARVISH_CLOAK);
	    knows_object(DWARVISH_ROUNDSHIELD);
		otmp = mksobj(CLUB, TRUE, FALSE);
		otmp->spe = otmp->cursed = otmp->blessed = 0;
		if(Role_if(PM_EXILE)) otmp->dknown = otmp->rknown = otmp->sknown = 1;
		else otmp->known = otmp->dknown = otmp->bknown = otmp->rknown = otmp->sknown = 1;
		addinv(otmp);
		otmp = mksobj(KNIFE, TRUE, FALSE);
		otmp->spe = otmp->cursed = otmp->blessed = 0;
		if(Role_if(PM_EXILE)) otmp->dknown = otmp->rknown = otmp->sknown = 1;
		else otmp->known = otmp->dknown = otmp->bknown = otmp->rknown = otmp->sknown = 1;
		addinv(otmp);
		u.wardsknown |= WARD_TOUSTEFNA;
		u.wardsknown |= WARD_DREPRUN;
		u.wardsknown |= WARD_VEIOISTAFUR;
		u.wardsknown |= WARD_THJOFASTAFUR;
	}break;

	case PM_GNOME:
	    /* Gnomes can recognize common dwarvish objects */
	    if (!Role_if(PM_ARCHEOLOGIST)){
			ini_inv(GnomishHat);
		}
		skill_add(Skill_G);
		ini_inv(TallowCandles);
		knows_object(GNOMISH_POINTY_HAT);
	    knows_object(AKLYS);
	    knows_object(DWARVISH_HELM);
	    knows_object(DWARVISH_MATTOCK);
	    knows_object(DWARVISH_CLOAK);
    break;

	case PM_ORC:
	    /* compensate for generally inferior equipment */
	    if (!Role_if(PM_WIZARD)
#ifdef CONVICT
         && !Role_if(PM_CONVICT)
#endif /* CONVICT */
		) ini_inv(Xtra_food);
	    /* Orcs can recognize all orcish objects */
	    knows_object(ORCISH_SHORT_SWORD);
	    knows_object(ORCISH_ARROW);
	    knows_object(ORCISH_BOW);
	    knows_object(ORCISH_SPEAR);
	    knows_object(ORCISH_DAGGER);
	    knows_object(ORCISH_CHAIN_MAIL);
	    knows_object(ORCISH_RING_MAIL);
	    knows_object(ORCISH_HELM);
	    knows_object(ORCISH_SHIELD);
	    knows_object(URUK_HAI_SHIELD);
	    knows_object(ORCISH_CLOAK);
		if(Role_if(PM_BARD)){
			skill_add(Skill_Orc_Brd);
			ini_inv(Orc_Brd_equip);
		}
    break;
	case PM_HALF_DRAGON:
		if(Role_if(PM_BARD)){
			u.umartial = TRUE;
			u.uenmax += 30;
			u.uen = u.uenmax;
		} else if(u.uenmax < 15) {
			u.uen = u.uenmax = 15;
		}
		if(flags.initgend){
			skill_add(Skill_HD_Female);
		}
    break;
	case PM_VAMPIRE:
	    /* Vampires start off with gods not as pleased, luck penalty */
	    knows_object(POT_BLOOD);
	    adjalign(-5); 
		u.ualign.sins += 5;
	    change_luck(-1);
	    break;
	default:	/* impossible */
		break;
	}

	if (discover)
		ini_inv(Wishing);

#ifdef WIZARD
	if (wizard)
		read_wizkit();
#endif

#ifndef GOLDOBJ
	u.ugold0 += hidden_gold();	/* in case sack has gold in it */
#else
	if (u.umoney0) ini_inv(Money);
	u.umoney0 += hidden_gold();	/* in case sack has gold in it */
#endif

	if(Role_if(PM_EXILE)){
		if (Race_if(PM_ELF))
			init_attr(60);
		else
			init_attr(55);
	} else if(Race_if(PM_ORC)){
		init_attr(55);
	} else if (Race_if(PM_ANDROID)){
		init_attr(95);
	} else if (Role_if(PM_VALKYRIE)){
		init_attr(85);
	} else if (Race_if(PM_ELF)){
		init_attr(80);
	} else init_attr(75);	/* init attribute values */
	find_ac();				/* get initial ac value */
	max_rank_sz();			/* set max str size for class ranks */
/*
 *	Do we really need this?
 */
	for(i = 0; i < A_MAX; i++)
	    if(!rn2(20)) {
		register int xd = rn2(7) - 2;	/* biased variation */
		(void) adjattrib(i, xd, TRUE);
		if (ABASE(i) < AMAX(i)) AMAX(i) = ABASE(i);
	    }

	/* make sure you can carry all you have - especially for Tourists */
	while (inv_weight() > 0) {
		if (adjattrib(A_STR, 1, TRUE)) continue;
		if (adjattrib(A_CON, 1, TRUE)) continue;
		/* only get here when didn't boost strength or constitution */
		break;
	}

		int j;
	for (j = 0; j < SIZE(horrors); j++)
	{
		/* what a horrible night to have a curse */
		horrors[j]->mlevel = 1;							/* low starting level so difficulty is based on other things*/
		horrors[j]->mmove = rn2(7) * 2 + 6;				/* slow to very fast */
		horrors[j]->ac = rn2(21) + (rn2(3) ? -10 : -20);/* any AC */
		horrors[j]->mr = rn2(11) * 10;				/* varying amounts of MR */
		horrors[j]->maligntyp = d(2, 9) - 10;			/* any alignment */

		/* attacks...?  */
		horrorattacks = rnd(4);
		for (i = 0; i < horrorattacks; i++) {
			attkptr = &horrors[j]->mattk[i];
			/* restrict it to certain types of attacks */
			attkptr->aatyp = randMeleeAttackTypes[rn2(SIZE(randMeleeAttackTypes))];
			attkptr->adtyp = randMeleeDamageTypes[rn2(SIZE(randMeleeDamageTypes))];
			attkptr->damn = d(1, 3);			/* we're almost sure to get this wrong first time */
			attkptr->damd = rn2(4) * 2 + 6;		/* either too high or too low */
		}
		horrorattacks = horrorattacks + (rnd(9) / 3) - 1; //(0),(0),0,0,0,1,1,1,2
		for (; i < horrorattacks; i++){
			attkptr = &horrors[j]->mattk[i];
			/* restrict it to certain types of attacks */
			attkptr->aatyp = randSpecialAttackTypes[rn2(SIZE(randSpecialAttackTypes))];
			switch (attkptr->aatyp){
			case AT_SPIT:
				attkptr->adtyp = randSpitDamageTypes[rn2(SIZE(randSpitDamageTypes))];
			break;
			case AT_HUGS:
				attkptr->adtyp = AD_PHYS;
			break;
			case AT_GAZE:
				attkptr->adtyp = randGazeDamageTypes[rn2(SIZE(randGazeDamageTypes))];
			break;
			case AT_ENGL:
				attkptr->adtyp = randEngulfDamageTypes[rn2(SIZE(randEngulfDamageTypes))];
			break;
			case AT_ARRW:
				attkptr->adtyp = randArrowDamageTypes[rn2(SIZE(randArrowDamageTypes))];
			break;
			case AT_MAGC:
				attkptr->adtyp = randMagicDamageTypes[rn2(SIZE(randMagicDamageTypes))];
			break;
			case AT_BREA:
				attkptr->adtyp = randBreathDamageTypes[rn2(SIZE(randBreathDamageTypes))];
			break;
			case AT_HODS:
				attkptr->adtyp = AD_HODS;
				break;
			case AT_BEAM:
			case AT_DEVA:
			case AT_5SQR:
				attkptr->adtyp = randBeamDamageTypes[rn2(SIZE(randBeamDamageTypes))];
			break;
			default:
				attkptr->adtyp = AD_PHYS;
			break;
			}
			switch (attkptr->adtyp){
			case AD_VBLD:
				attkptr->damn = d(1, 3);
				attkptr->damd = rn2(3) + 1;
				break;
			case AD_LUCK:
				attkptr->damn = 1;
				attkptr->damd = 1;
				break;
			case AD_MIST:
			case AD_DEAD:
				attkptr->damn = 0;
				attkptr->damd = 0;
				break;
			default:
				attkptr->damn = d(3, 3);			/* we're almost sure to get this wrong first time */
				attkptr->damd = rn2(3) * 2 + 8;		/* either too high or too low */
			}
		}

	
		horrors[j]->msize = !rn2(6) ? MZ_GIGANTIC : rn2(MZ_HUGE + 1);			/* any size */
		horrors[j]->cwt = randCorpseWeights[horrors[j]->msize];					/* probably moot as it's flagged NOCORPSE */
		horrors[j]->cnutrit = randCorpseNut[horrors[j]->msize];					/* see above */
		horrors[j]->msound = rn2(MS_HUMANOID);								/* any but the specials */
		horrors[j]->mresists = 0;


		for (i = 0; i < rnd(6); i++) {
			horrors[j]->mresists |= (1 << rn2(10));		/* physical resistances... */
		}
		// for (i = 0; i < rnd(5); i++) {
		// horrors[j]->mresists |= (0x100 << rn2(7));	/* 'different' resistances, even clumsy */
		// }
		horrors[j]->mconveys = 0;					/* flagged NOCORPSE */

		/*
		 * now time for the random flags.  this will likely produce
		 * a number of complete trainwreck monsters at first, but
		 * every so often something will dial up nasty stuff
		 */
		horrors[j]->mflagsm = 0;
		horrors[j]->mflagst = 0;
		horrors[j]->mflagsb = 0;
		horrors[j]->mflagsg = 0;
		horrors[j]->mflagsa = 0;
		horrors[j]->mflagsv = 0;

		for (i = 0; i < rnd(17); i++) {
			horrors[j]->mflagsm |= (1 << rn2(33));		/* trainwreck this way :D */
		}
		for (i = 0; i < rnd(17); i++) {
			horrors[j]->mflagst |= (1 << rn2(33));
		}
		for (i = 0; i < rnd(17); i++) {
			horrors[j]->mflagsb |= (1 << rn2(33));
		}
		for (i = 0; i < rnd(17); i++) {
			horrors[j]->mflagsg |= (1 << rn2(33));
		}
		for (i = 0; i < rnd(17); i++) {
			horrors[j]->mflagsa |= (1 << rn2(33));
		}
		for (i = 0; i < rnd(17); i++) {
			horrors[j]->mflagsv |= (1 << rn2(33));
		}

		// horrors[j]->mflagsb &= ~MB_UNSOLID;			/* no ghosts */
		// horrors[j]->mflagsm &= ~MM_WALLWALK;			/* no wall-walkers */

		horrors[j]->mflagsg |= MG_NOPOLY;		/* Don't let the player be one of these yet. */

		horrors[j]->mflagst |= MT_HOSTILE;

		horrors[j]->mflagsg &= ~MG_MERC;				/* no guards */
		horrors[j]->mflagst &= ~MT_PEACEFUL;			/* no peacefuls */
		horrors[j]->mflagsa &= ~MA_WERE;				/* no lycanthropes */
		horrors[j]->mflagsg &= ~MG_PNAME;				/* not a proper name */

		/* some cleanup to reduce the trainwreck*/
		if (horrors[j]->mflagsm & MM_TUNNEL)
			horrors[j]->mflagsm &= ~MM_NEEDPICK;
		if (horrors[j]->mflagsm & MM_STATIONARY)
			horrors[j]->mflagsm &= ~(MM_FLEETFLEE | MM_TUNNEL | MM_NEEDPICK | MM_NOTONL);
		if (horrors[j]->mflagst & MT_NOTAKE)
			horrors[j]->mflagst &= ~MT_MAID;
		if (horrors[j]->mflagst & MT_MINDLESS)
			horrors[j]->mflagst &= ~MT_ANIMAL;
		if (horrors[j]->mflagst & MT_ANIMAL)
			horrors[j]->mflagst &= ~MT_MINDLESS;
		if (horrors[j]->mflagsb & MB_MALE)
			horrors[j]->mflagsb &= ~(MB_FEMALE | MB_NEUTER);
		if (horrors[j]->mflagsb & MB_FEMALE)
			horrors[j]->mflagsb &= ~(MB_MALE | MB_NEUTER);
		if (horrors[j]->mflagsb & MB_NEUTER)
			horrors[j]->mflagsb &= ~(MB_MALE | MB_FEMALE);
		if (horrors[j]->mflagsb & MB_NOHEAD)
			horrors[j]->mflagsb &= ~(MB_LONGHEAD | MB_LONGNECK);
		if (horrors[j]->mflagsv & MV_EXTRAMISSION)
			horrors[j]->mflagsv &= ~(MV_LOWLIGHT3 | MV_LOWLIGHT2 | MV_NORMAL | MV_CATSIGHT | MV_DARKSIGHT);
		if (horrors[j]->mflagsv & MV_CATSIGHT)
			horrors[j]->mflagsv &= ~(MV_LOWLIGHT3 | MV_LOWLIGHT2 | MV_NORMAL |  MV_DARKSIGHT);
		if (horrors[j]->mflagsv & MV_LOWLIGHT3)
			horrors[j]->mflagsv &= ~(MV_LOWLIGHT2 | MV_NORMAL | MV_DARKSIGHT);
		if (horrors[j]->mflagsv & MV_LOWLIGHT2)
			horrors[j]->mflagsv &= ~(MV_NORMAL | MV_DARKSIGHT);
		if (horrors[j]->mflagsv & MV_NORMAL)
			horrors[j]->mflagsv &= ~(MV_DARKSIGHT);
		if (!(attacktype(horrors[j], AT_MAGC) || attacktype(horrors[j], AT_MMGC)))
			horrors[j]->mflagsg &= ~MG_NOSPELLCOOLDOWN;
		if (horrors[j]->mflagsg & MG_PRINCE)
			horrors[j]->mflagsg &= ~MG_LORD;

		for (i = 0; i < 2; i++) /* adjust its level and difficulty upwards */
		{
			horrors[j]->mlevel = mstrength(horrors[j]);
			monstr[monsndx(horrors[j])] = mstrength(horrors[j]);
		}

		if (horrors[j]->mlevel<11)	// redo if we get too weak a monster
			j--;
	}
	// horror stuff that cannot be done in the loop easily
	if (!rn2(10)) u.shambin = 3;
	else if (!rn2(9)) u.shambin = 2;
	else if (rn2(8) > 1) u.shambin = 1;
	else u.shambin = 0;

	if (!rn2(10)) u.stumbin = 3;
	else if (!rn2(9)) u.stumbin = 2;
	else if (rn2(8) > 1) u.stumbin = 1;
	else u.stumbin = 0;

	if (!rn2(10)) u.wandein = 3;
	else if (!rn2(9)) u.wandein = 2;
	else if (rn2(8) > 1) u.wandein = 1;
	else u.wandein = 0;

	u.oonaenergy = !rn2(3) ? AD_FIRE : rn2(2) ? AD_COLD : AD_ELEC;
	dungeon_topology.alt_tower = !rn2(8);
	if(Race_if(PM_HALF_DRAGON) && Role_if(PM_NOBLEMAN)){
		if(flags.initgend)
			flags.HDbreath = AD_COLD;
		else flags.HDbreath = AD_FIRE;
	} else switch(rnd(6)){
		case 1:
			flags.HDbreath = AD_COLD;
		break;
		case 2:
			flags.HDbreath = AD_FIRE;
		break;
		case 3:
			flags.HDbreath = AD_SLEE;
		break;
		case 4:
			flags.HDbreath = AD_ELEC;
		break;
		case 5:
			flags.HDbreath = AD_DRST;
		break;
		case 6:
			flags.HDbreath = AD_ACID;
		break;
	}
	/* Fix up the alignment quest nemesi */
	mons[PM_OONA].mcolor = (u.oonaenergy == AD_FIRE) ? CLR_RED 
						 : (u.oonaenergy == AD_COLD) ? CLR_CYAN 
						 : (u.oonaenergy == AD_ELEC) ? HI_ZAP 
						 : CLR_WHITE;
	return;
}

/* skills aren't initialized, so we use the role-specific skill lists */
STATIC_OVL boolean
restricted_spell_discipline(otyp)
int otyp;
{
    const struct def_skill *skills;
    int this_skill = spell_skilltype(otyp);

    switch (Role_switch) {
     case PM_ARCHEOLOGIST:	skills = Skill_A; break;
     case PM_ANACHRONONAUT:	skills = Skill_Ana; break;
     case PM_BARBARIAN:		skills = Skill_B; break;
#ifdef BARD
     case PM_BARD:		skills = Skill_Bard; break;
#endif
     case PM_EXILE:			skills = Skill_N; break;
     case PM_CAVEMAN:		skills = Skill_C; break;
#ifdef CONVICT
     case PM_CONVICT:		skills = Skill_Con; break;
#endif  /* CONVICT */
     case PM_HEALER:		skills = Skill_H; break;
     case PM_KNIGHT:		skills = Skill_K; break;
     case PM_MONK:		skills = Skill_Mon; break;
	 case PM_PIRATE:		skills = Skill_Pir; break;
     case PM_PRIEST:		skills = Skill_P; break;
     case PM_RANGER:		skills = Skill_Ran; break;
     case PM_ROGUE:		skills = Skill_R; break;
     case PM_SAMURAI:		skills = Skill_S; break;
#ifdef TOURIST
     case PM_TOURIST:		skills = Skill_T; break;
#endif
     case PM_VALKYRIE:		skills = Skill_V; break;
     case PM_WIZARD:		skills = Skill_W; break;
     default:			skills = 0; break;	/* lint suppression */
    }

    while (skills->skill != P_NONE) {
	if (skills->skill == this_skill) return FALSE;
	++skills;
    }
    return TRUE;
}

STATIC_OVL void
ini_inv(trop)
register struct trobj *trop;
{
	struct obj *obj;
	int otyp, i;

	while (trop->trclass) {
		if (trop->trotyp != UNDEF_TYP) {
			otyp = (int)trop->trotyp;
			if (urace.malenum != PM_HUMAN) {
			    /* substitute specific items for generic ones */
			    for (i = 0; inv_subs[i].race_pm != NON_PM; ++i)
				if (inv_subs[i].race_pm == urace.malenum &&
					otyp == inv_subs[i].item_otyp) {
				    otyp = inv_subs[i].subs_otyp;
				    break;
				}
			}
			obj = mksobj(otyp, TRUE, FALSE);
			if(obj->otyp == POT_BLOOD) 
				obj->corpsenm = PM_HUMAN;
			if(obj->oclass == WEAPON_CLASS || obj->oclass == ARMOR_CLASS) obj->objsize = youracedata->msize;
			if(obj->oclass == ARMOR_CLASS){
				if(is_suit(obj)) obj->bodytypeflag = (youracedata->mflagsb&MB_BODYTYPEMASK);
				else if(is_helmet(obj)) obj->bodytypeflag = (youracedata->mflagsb&MB_HEADMODIMASK);
				else if(is_shirt(obj)) obj->bodytypeflag = (youracedata->mflagsb&MB_HUMANOID) ? MB_HUMANOID : (youracedata->mflagsb&MB_BODYTYPEMASK);
			}
			if(obj->otyp == BULLWHIP && Race_if(PM_DROW) && flags.initgend){
				obj->otyp = VIPERWHIP;
				obj->obj_material = SILVER;
				obj->ovar1 = 1;
				fix_object(obj);
			}
			if(obj->otyp == HEAVY_MACHINE_GUN && Role_if(PM_ANACHRONONAUT) && Race_if(PM_DWARF)){
				obj->obj_material = MITHRIL;
				fix_object(obj);
			}
			if(obj->otyp == BATTLE_AXE && Role_if(PM_ANACHRONONAUT) && Race_if(PM_ANDROID)){
				obj->objsize = MZ_LARGE;
				obj->oeroded = 1;
				fix_object(obj);
			}
			if(obj->otyp == SCALE_MAIL && Role_if(PM_ANACHRONONAUT)){
				obj->obj_material = COPPER;
				fix_object(obj);
			}
			if(obj->otyp == GAUNTLETS && Role_if(PM_ANACHRONONAUT)){
				obj->obj_material = COPPER;
				fix_object(obj);
			}
			if(obj->otyp == PISTOL && Role_if(PM_ANACHRONONAUT) && Race_if(PM_DWARF)){
				obj->obj_material = MITHRIL;
				fix_object(obj);
			}
			if(obj->otyp == SEISMIC_HAMMER && Role_if(PM_ANACHRONONAUT) && Race_if(PM_DWARF)){
				obj->obj_material = MITHRIL;
				fix_object(obj);
			}
			
			/* Don't start with +0 or negative rings */
			if (objects[obj->otyp].oc_charged && obj->spe <= 0)
				obj->spe = rne(3);
			fix_object(obj);
		} else {	/* UNDEF_TYP */
			static NEARDATA short nocreate = STRANGE_OBJECT;
			static NEARDATA short nocreate2 = STRANGE_OBJECT;
			static NEARDATA short nocreate3 = STRANGE_OBJECT;
			static NEARDATA short nocreate4 = STRANGE_OBJECT;
			static NEARDATA short nocreate5 = STRANGE_OBJECT;
			static NEARDATA short nocreate6 = STRANGE_OBJECT;
			static NEARDATA short nocreate7 = STRANGE_OBJECT;
			
			static NEARDATA short nocreateam1 = STRANGE_OBJECT;
			static NEARDATA short nocreateam2 = STRANGE_OBJECT;
		/*
		 * For random objects, do not create certain overly powerful
		 * items: wand of wishing, ring of levitation, or the
		 * polymorph/polymorph control combination.  Specific objects,
		 * i.e. the discovery wishing, are still OK.
		 * Also, don't get a couple of really useless items.  (Note:
		 * punishment isn't "useless".  Some players who start out with
		 * one will immediately read it and use the iron ball as a
		 * weapon.)
		 */
			if((Race_if(PM_DROW) || Race_if(PM_MYRKALFR)) && trop->trclass == RING_CLASS) obj = mksobj(find_signet_ring(),TRUE,FALSE);
			else obj = mkobj(trop->trclass, FALSE);
			otyp = obj->otyp;
			while (otyp == WAN_WISHING
				|| otyp == RIN_WISHES
				|| otyp == nocreate
				|| otyp == nocreate2
				|| otyp == nocreate3
				|| otyp == nocreate4
				|| otyp == nocreate5
				|| otyp == nocreate6
				|| otyp == nocreate7
				|| (obj->otyp == HYPOSPRAY_AMPULE && nocreateam1 == (short)obj->ovar1)
				|| (obj->otyp == HYPOSPRAY_AMPULE && nocreateam2 == (short)obj->ovar1)
				|| otyp == RIN_LEVITATION
				/* 'useless' items */
				|| otyp == POT_HALLUCINATION
				|| otyp == POT_ACID
				|| otyp == POT_AMNESIA
				|| otyp == SCR_AMNESIA
				|| otyp == SCR_FIRE
				|| otyp == SCR_BLANK_PAPER
				|| otyp == SPE_BLANK_PAPER
				|| otyp == RIN_AGGRAVATE_MONSTER
				|| otyp == RIN_HUNGER
				|| otyp == WAN_NOTHING
				/* Monks don't use weapons */
				|| (otyp == SCR_ENCHANT_WEAPON &&
				    Role_if(PM_MONK))
				/* wizard patch -- they already have one */
				|| (otyp == SPE_FORCE_BOLT &&
				    (Role_if(PM_WIZARD) || Race_if(PM_INCANTIFIER)))
				|| (otyp == SPE_HEALING &&
				    Race_if(PM_INCANTIFIER))
				|| ((otyp == SPE_HEALING || 
					 otyp == SPE_EXTRA_HEALING ||
					 otyp == SPE_STONE_TO_FLESH) &&
				    Role_if(PM_HEALER))
				/* powerful spells are either useless to
				   low level players or unbalancing; also
				   spells in restricted skill categories */
				|| (obj->oclass == SPBOOK_CLASS &&
				    (objects[otyp].oc_level > 3 ||
				    restricted_spell_discipline(otyp)))
				|| (hates_silver(youracedata) && obj->obj_material == SILVER)
				|| (hates_iron(youracedata) && obj->obj_material == IRON)
							) {
				dealloc_obj(obj);
				obj = mkobj(trop->trclass, FALSE);
				otyp = obj->otyp;
			}

			/* Don't start with +0 or negative rings */
			if (objects[otyp].oc_charged && obj->spe <= 0)
				obj->spe = rne(3);

			/* Heavily relies on the fact that 1) we create wands
			 * before rings, 2) that we create rings before
			 * spellbooks, and that 3) not more than 1 object of a
			 * particular symbol is to be prohibited.  (For more
			 * objects, we need more nocreate variables...)
			 */
			switch (otyp) {
			    case WAN_POLYMORPH:
			    case RIN_POLYMORPH:
			    case POT_POLYMORPH:
				nocreate = RIN_POLYMORPH_CONTROL;
				break;
			    case RIN_POLYMORPH_CONTROL:
				nocreate = RIN_POLYMORPH;
				nocreate2 = SPE_POLYMORPH;
				nocreate3 = POT_POLYMORPH;
			}
			/* Don't have 2 of the same ring or spellbook */
			if (obj->oclass == RING_CLASS ||
			    obj->oclass == SPBOOK_CLASS) {
				if(nocreate4 == STRANGE_OBJECT) nocreate4 = otyp;
				else if(nocreate5 == STRANGE_OBJECT) nocreate5 = otyp;
				else if(nocreate6 == STRANGE_OBJECT) nocreate6 = otyp;
				else if(nocreate7 == STRANGE_OBJECT) nocreate7 = otyp;
			}
			/* or ampule */
			if (obj->otyp == HYPOSPRAY_AMPULE){
				if(nocreateam1 == STRANGE_OBJECT) nocreateam1 = (short)obj->ovar1;
				else if(nocreateam2 == STRANGE_OBJECT) nocreateam2 = (short)obj->ovar1;
			}
		}

#ifdef GOLDOBJ
		if (trop->trclass == COIN_CLASS) {
			/* no "blessed" or "identified" money */
			obj->quan = u.umoney0;
		} else {
#endif
			if(Role_if(PM_EXILE)){
				obj->dknown = obj->rknown = obj->sknown = 1;
				if(obj->oclass == WEAPON_CLASS && !Race_if(PM_DWARF)){
					if(obj->obj_material != WOOD) obj->oeroded = 1;
					else obj->oeroded2 = 1;
				} else if(obj->oclass == FOOD_CLASS){
					obj->ostolen = TRUE;
				} else if(obj->otyp == FLINT) obj->known = 1;
			}else{
				obj->dknown = obj->bknown = obj->rknown = obj->sknown = 1;
				if (objects[otyp].oc_uses_known) obj->known = 1;
				if(Role_if(PM_PIRATE) && obj->obj_material == IRON) obj->oerodeproof = 1;
				if(Role_if(PM_SAMURAI) && obj->oclass == ARMOR_CLASS && obj->obj_material == IRON) obj->oerodeproof = 1;
				if(Role_if(PM_SAMURAI) && obj->otyp == MASK){
					obj->obj_material = IRON;
//					obj->oerodeproof = 1;
					obj->corpsenm = PM_HUMAN;
				}
			}
			obj->cursed = 0;
			if(obj->otyp == DROVEN_PLATE_MAIL  ||
				obj->otyp == NOBLE_S_DRESS 	  ||
				obj->otyp == DROVEN_CHAIN_MAIL ||
				obj->otyp == DROVEN_HELM
			) obj->oerodeproof = 1;
			if (obj->opoisoned && u.ualign.type != A_CHAOTIC)
			    obj->opoisoned = 0;
			if (obj->oward){
				if(obj->oclass == WEAPON_CLASS && (obj)->obj_material == WOOD) u.wardsknown |= obj->oward;
				else if(obj->oclass == RING_CLASS && isEngrRing((obj)->otyp) && !(obj->ohaluengr)) u.wardsknown |= get_wardID(obj->oward);
			}
			if (obj->oclass == WEAPON_CLASS ||
				obj->oclass == TOOL_CLASS) {
			    obj->quan = (long) trop->trquan;
			    trop->trquan = 1;
			} else if (obj->oclass == GEM_CLASS &&
				((is_graystone(obj) && obj->otyp != FLINT) ||
				  Role_if(PM_EXILE) )) {
			    obj->quan = 1L;
			}
#ifdef CONVICT
            if (obj->otyp == STRIPED_SHIRT ) {
                obj->cursed = TRUE;
            }
#endif /* CONVICT */
			if (trop->trspe != UNDEF_SPE)
			    obj->spe = trop->trspe;
			if (trop->trbless != UNDEF_BLESS)
			    obj->blessed = trop->trbless;
			
			if(hates_holy(youracedata)){
				if(obj->blessed){
					obj->blessed = 0;
					obj->cursed = 1;
				}
			}
			if(hates_silver(youracedata)){
				if(obj->obj_material == SILVER){
					obj->obj_material = GOLD;
					fix_object(obj);
				}
			}
			if(hates_iron(youracedata)){
				if(obj->obj_material == IRON){
					obj->obj_material = MITHRIL;
					fix_object(obj);
				}
			}
#ifdef GOLDOBJ
		}
#endif
		/* defined after setting otyp+quan + blessedness */
		obj->owt = weight(obj);
		obj = addinv(obj);

		/* Make the type known if necessary */
		if (OBJ_DESCR(objects[otyp]) && obj->known && !Role_if(PM_EXILE))
			discover_object(otyp, TRUE, FALSE);
		if (otyp == OIL_LAMP)
			discover_object(POT_OIL, TRUE, FALSE);

		if(obj->otyp == AMULET_OF_NULLIFY_MAGIC && Role_if(PM_ANACHRONONAUT) && !uamul){
			setworn(obj, W_AMUL);
		}
		
		if(obj->otyp == MASK && !ublindf){
			setworn(obj, W_TOOL);
		}
		
		if(obj->otyp == ANDROID_VISOR && !ublindf){
			setworn(obj, W_TOOL);
		}
		
		if(obj->oclass == ARMOR_CLASS){
			if (is_shield(obj) && !uarms) {
				setworn(obj, W_ARMS);
			} else if (is_helmet(obj) && !uarmh)
				setworn(obj, W_ARMH);
			else if (is_gloves(obj) && !uarmg)
				setworn(obj, W_ARMG);
#ifdef TOURIST
			else if (is_shirt(obj) && !uarmu)
				setworn(obj, W_ARMU);
#endif
			else if (is_cloak(obj) && !uarmc)
				setworn(obj, W_ARMC);
			else if (is_boots(obj) && !uarmf)
				setworn(obj, W_ARMF);
			else if (is_suit(obj) && !uarm)
				setworn(obj, W_ARM);
		}

		if (obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
			otyp == TIN_OPENER || otyp == BELL || otyp == FLINT || otyp == ROCK) {
		    if (is_ammo(obj) || is_missile(obj)) {
				if (!uquiver) setuqwep(obj);
		    } else if (!uwep) setuwep(obj);
		    else if (!uswapwep){
				setuswapwep(obj);
			}
		}
		if (obj->oclass == SPBOOK_CLASS &&
				obj->otyp != SPE_BLANK_PAPER){
		    initialspell(obj);
			initialward(obj);
		}

#if !defined(PYRAMID_BUG) && !defined(MAC)
		if(--trop->trquan) continue;	/* make a similar object */
#else
		if(trop->trquan) {		/* check if zero first */
			--trop->trquan;
			if(trop->trquan)
				continue;	/* make a similar object */
		}
#endif
		trop++;
	}
}

void
set_mask(){
}

void
scatter_weapons(){
	struct obj *obj;
	struct monst *mtmp;
	if(flags.initgend){
		// obj = mksobj(GOLD_BLADED_VIBROSWORD, TRUE, FALSE);
		// add_to_migration(obj);
		// obj->ox = stronghold_level.dnum;
		// obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
		int nlev;
		d_level flev;
		
		mtmp = makemon(&mons[PM_ANDROID], xdnstair, ydnstair, MM_ADJACENTOK|MM_EDOG);
		initedog(mtmp);
		EDOG(mtmp)->loyal = TRUE;
		nlev = rnd(stronghold_level.dlevel-10)+10;
		// pline("going to %d",nlev);
		get_level(&flev, nlev);
		migrate_to_level(mtmp, ledger_no(&flev), MIGR_RANDOM,
			(coord *)0);
	} else {
		// obj = mksobj(WHITE_VIBROSWORD, TRUE, FALSE);
		// add_to_migration(obj);
		// obj->ox = stronghold_level.dnum;
		// obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
		int nlev;
		d_level flev;
		
		mtmp = makemon(&mons[PM_GYNOID], xdnstair, ydnstair, MM_ADJACENTOK|MM_EDOG);
		initedog(mtmp);
		EDOG(mtmp)->loyal = TRUE;
		nlev = rnd(stronghold_level.dlevel-10)+10;
		// pline("going to %d",nlev);
		get_level(&flev, nlev);
		migrate_to_level(mtmp, ledger_no(&flev), MIGR_RANDOM,
			(coord *)0);
	}
	
	obj = mksobj(WHITE_VIBROSPEAR, TRUE, FALSE);
	fully_identify_obj(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(WHITE_VIBROZANBATO, TRUE, FALSE);
	fully_identify_obj(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(GOLD_BLADED_VIBROSPEAR, TRUE, FALSE);
	fully_identify_obj(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(GOLD_BLADED_VIBROZANBATO, TRUE, FALSE);
	fully_identify_obj(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(TWO_HANDED_SWORD, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->oeroded = 1;
	obj->objsize = MZ_GIGANTIC;
	fix_object(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(SHORT_SWORD, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->objsize = MZ_LARGE;
	obj->obj_material = SILVER;
	fix_object(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(TWO_HANDED_SWORD, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->objsize = MZ_LARGE;
	obj->obj_material = SILVER;
	fix_object(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(SPEAR, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->objsize = MZ_LARGE;
	obj->obj_material = SILVER;
	fix_object(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(RED_EYED_VIBROSWORD, TRUE, FALSE);
	fully_identify_obj(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(FORCE_PIKE, TRUE, FALSE);
	fully_identify_obj(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(TWO_HANDED_SWORD, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->objsize = MZ_LARGE;
	obj->obj_material = METAL;
	obj->oproperties = OPROP_ELECW;
	fix_object(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(SPEAR, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->obj_material = METAL;
	obj->oproperties = OPROP_ELECW;
	fix_object(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(LONG_SWORD, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->obj_material = METAL;
	obj->oproperties = OPROP_ELECW;
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(SABER, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->obj_material = METAL;
	fix_object(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(KATANA, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->obj_material = METAL;
	obj->oproperties = OPROP_AXIOW;
	fix_object(obj);
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(RAPIER, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->oproperties = OPROP_FLAYW;
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	obj = mksobj(GLAIVE, TRUE, FALSE);
	fully_identify_obj(obj);
	obj->oproperties = OPROP_FLAYW;
	add_to_migration(obj);
	obj->ox = stronghold_level.dnum;
	obj->oy = rnd(stronghold_level.dlevel-1)+1; //2->castle
	
	// if(flags.female){
		// add_to_migration(obj);
		// obj->ox = sanctum_level.dnum;
		// obj->oy = sanctum_level.dlevel - 1; /* vs level, bottom of the accesible part of hell */
	// }
}

/*u_init.c*/
