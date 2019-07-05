/*	SCCS Id: @(#)objnam.c	3.4	2003/12/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#ifdef OVLB
#include "artilist.h"
#else
STATIC_DCL struct artifact artilist[];
#endif

/* "an uncursed greased partly eaten guardian naga hatchling [corpse]" */
#define PREFIX	125	/* (56) */
#define SCHAR_LIM 255
#define NUMOBUF 12

STATIC_DCL char *FDECL(strprepend,(char *,const char *));
#ifdef OVLB
static boolean FDECL(wishymatch, (const char *,const char *,BOOLEAN_P));
#endif
static char *NDECL(nextobuf);
static void FDECL(add_erosion_words, (struct obj *, char *));
char * doxname(struct obj *, BOOLEAN_P, BOOLEAN_P, BOOLEAN_P);
#ifdef SORTLOOT
char * FDECL(xname2, (struct obj *,BOOLEAN_P));
#endif

struct Jitem {
	int item;
	const char *name;
};

/* true for gems/rocks that should have " stone" appended to their names */
#define GemStone(typ)	(typ == FLINT || \
			 (objects[typ].oc_material == GEMSTONE &&	\
			  (typ != DILITHIUM_CRYSTAL && typ != RUBY &&	\
			   typ != DIAMOND && typ != SAPPHIRE &&		\
			   typ != MAGICITE_CRYSTAL && 	\
			   typ != BLACK_OPAL && 	\
			   typ != EMERALD && typ != OPAL)))

#ifndef OVLB

STATIC_DCL struct Jitem Japanese_items[];
STATIC_DCL struct Jitem ObscureJapanese_items[];
STATIC_OVL struct Jitem Pirate_items[];

#else /* OVLB */

/*
"mirrored": Lightsabers reflect incident light.  A 'black' lightsaber
blade would therefore actually look like a cylindrical mirror.
"black": A lightsaber made with a piece of fossil dark absorbs all
but the brightest light and casts darkness
*/
NEARDATA struct colorTextClr LightsaberColor[] = {
	{"brilliant blue",CLR_BRIGHT_BLUE},	/*MAGICITE_CRYSTAL*/
	{"twisting rainbow",CLR_WHITE},		/*DILITHIUM_CRYSTAL*/
	{"coruscating white",CLR_WHITE},	/*DIAMOND*/
	{"star blue",CLR_BLUE},				/*STAR_SAPPHIRE*/
	{"ruby",CLR_RED},					/*RUBY*/
	{"sunset",CLR_ORANGE},				/*JACINTH*/
	{"sapphire",CLR_BLUE},				/*SAPPHIRE*/
	{"smoke",CLR_BLACK},				/*BLACK_OPAL*/
	{"emerald",CLR_GREEN},				/*EMERALD*/
	{"turquoise",CLR_GREEN},			/*TURQUOISE*/
	{"pink",CLR_ORANGE},				/*MORGANITE*/
	{"citrine",CLR_YELLOW},				/*CITRINE*/
	{"aquamarine",CLR_GREEN},			/*AQUAMARINE*/
	{"amber",CLR_BROWN},				/*AMBER*/
	{"brown",CLR_BROWN},				/*TOPAZ*/
	{"mirrored",CLR_BLACK},				/*JET*/
	{"rainbow",CLR_WHITE},				/*OPAL*/
	{"yellow",CLR_YELLOW},				/*CHRYSOBERYL*/
	{"deep red",CLR_RED},					/*GARNET*/
	{"amethyst",CLR_MAGENTA},			/*AMETHYST*/
	{"crimson",CLR_RED},				/*JASPER*/
	{"violet",CLR_MAGENTA},				/*FLUORITE*/
	{"blue",CLR_BLUE},					/*FLUORITE*/
	{"white",CLR_WHITE},				/*FLUORITE*/
	{"green",CLR_GREEN},				/*FLUORITE*/
	{"gray",CLR_GRAY},					/*OBSIDIAN*/
	{"orange",CLR_ORANGE},				/*AGATE*/
	{"jade",CLR_GREEN},					/*JADE*/
	{"red",CLR_RED},					/*white glass*/
	{"red",CLR_RED},					/*blue glass*/
	{"red",CLR_RED},					/*red glass*/
	{"red",CLR_RED},					/*yellowish brown glass*/
	{"red",CLR_RED},					/*orange glass*/
	{"red",CLR_RED},					/*yellow glass*/
	{"red",CLR_RED},					/*black glass*/
	{"red",CLR_RED},					/*green glass*/
	{"red",CLR_RED},					/*violet glass*/
	{"",CLR_RED},						/*luckstone*/
	{"",CLR_RED},						/*loadstone*/
	{"",CLR_RED},						/*touchstone*/
	{"",CLR_RED},						/*flint*/
	{"",CLR_RED},						/*mithril*/
	{"black",CLR_BLACK},				/*fossil dark*/
	{"",CLR_RED}						/*silver slingstone*/
};

STATIC_OVL char *SaberHilts[] = {
/*00*/"This %s has a curved hilt, making it particularly suited for use in duels.",
/*01*/"This %s has a classic grip and exposed emitter disk.",
/*02*/"This %s has a raised emitter with a lotus-bud shroud.",
/*03*/"This %s has a delicate-looking basket hilt.",
/*04*/"This %s projects a single quillon blade.",
/*05*/"This %s projects a pair of quillon blades.",
/*06*/"This %s has a smooth, flowing design.",
/*07*/"This %s has a smooth hilt with a recessed emitter disk.",
/*08*/"Six little claws grip the emitter disk of this %s.",
/*09*/"This %s's grip is composed of a hard, woody substance.",
/*10*/"This %s's grip is a set of vertical black ridges.",
/*11*/"This %s is fashioned to appear much like a branch of coral.",
/*12*/"This %s is a single long spiral.",
/*13*/"This %s's emitter disk is set into the jaws of a white tiger.",
/*14*/"This %s's emitter disk is held by the coils of a green dragon.",
/*15*/"A red bird holds this %s's emitter disk in its beak, its wings form a saber guard.",
/*16*/"This %s has a black tortoise shell grip, and its blade emerges from the beak of a black turtle head.",
/*17*/"This %s has an electrum grip and pommel.",
/*18*/"This %s has a black grip and silver details.",
/*19*/"This %s has a long, fabric-wrapped grip and jade buttons.",
/*20*/"This %s has a rough, worn-looking leather grip.",
/*21*/"This %s has a grip of tarnished silver ivy.",
/*22*/"This %s has a forest camouflage covering.",
/*23*/"This %s has a smooth diskguard.",
/*24*/"This %s has a ornate golden diskguard.",
/*25*/"The focusing chamber of this %s can be seen through a viewport.",
/*26*/"A thin red ribbon hangs from the pommel of this %s.",
/*27*/"This %s has a crude hilt fashioned from crystallized metal.",
/*28*/"There is a winged blade of light carved into the pommel of this %s.",
/*29*/"The Galactic Roundel is carved into this %s's pommel.",
/*30*/"A crimson starbird is set the pommel of this %s.",
/*31*/"A blazing claw is set the pommel of this %s.",
/*32*/"A red eclipse symbol is set the pommel of this %s.",
/*33*/"There is a ring of bronze sea-creatures above the grip of this %s.",
/*34*/"The pommel of this %s is carved from a japor snippet.",
/*35*/"Four lines of silver Qualith stand out from this basalt hilt.",
/*36*/"This %s is decorated with subtle swirls.",
/*37*/"This %s is decorated with non-euclidean curves and angles.", /*Given that a lightsaber handle is a curved surface....*/
/*38*/"This %s is wrapped with silver spiderwebs.",
/*39*/"This %s is quite intricate in its design, covered in delicate runes and inlaid with black markings.",
};

STATIC_OVL struct Jitem ObscureJapanese_items[] = {
	{ BATTLE_AXE, "ono" },
	{ BROADSWORD, "ninja-to" },
	{ BRONZE_PLATE_MAIL, "tanko" },
	{ CLUB, "jo" },
	{ CRYSTAL_PLATE_MAIL, "jade o-yoroi" },
	{ DAGGER, "kunai" },
	{ DART, "bo-shuriken" },
	{ DWARVISH_MATTOCK, "dwarvish zaghnal" },
	{ FLAIL, "nunchaku" },
	{ FOOD_RATION, "gunyoki" },
	{ SEDGE_HAT, "sugegasa" },
	{ WAR_HAT, "jingasa" },
	{ GAUNTLETS_OF_FUMBLING, "kote of fumbling" },
	{ GAUNTLETS_OF_POWER, "kote of power" },
	{ GUISARME, "kamayari" },
	{ HALBERD, "bisento" },
	{ HELMET, "kabuto" },
	{ KNIFE, "shito" },
	{ LANCE, "uma-yari" },
	{ GLOVES, "yugake" },
	{ LOCK_PICK, "osaku" },
	{ LONG_SWORD, "chokuto" },
	{ PLATE_MAIL, "o-yoroi" },
	{ POT_BOOZE, "sake" },
	{ QUARTERSTAFF, "bo" },
	{ SHORT_SWORD, "wakizashi" },
	{ SHURIKEN, "hira-shuriken" },
	{ SPEAR, "yari" },
	{ SPLINT_MAIL, "dou-maru" },
	{ TRIDENT, "magari yari" },
	{ TWO_HANDED_SWORD, "no-dachi" },
	{ WAR_HAMMER, "dai tsuchi" },
	{ WOODEN_HARP, "koto" },
	{0, "" }
};

STATIC_OVL struct Jitem Japanese_items[] = {
	{ BROADSWORD, "ninja-to" },
	{ CRYSTAL_PLATE_MAIL, "crystal tanko" },
	{ BRONZE_PLATE_MAIL, "bronze tanko" },
	{ PLATE_MAIL, "tanko" },
	{ DAGGER, "kunai" },
	{ DART, "bo-shuriken" },
	{ FLAIL, "nunchaku" },
	{ FOOD_RATION, "gunyoki" },
	{ SEDGE_HAT, "sugegasa" },
	{ WAR_HAT, "jingasa" },
	{ GAUNTLETS_OF_FUMBLING, "kote of fumbling" },
	{ GAUNTLETS_OF_POWER, "kote of power" },
	{ HELMET, "kabuto" },
	{ KNIFE, "shito" },
	{ GLOVES, "yugake" },
	{ LOCK_PICK, "osaku" },
	{ POT_BOOZE, "sake" },
	{ QUARTERSTAFF, "bo staff" },
	{ SHORT_SWORD, "wakizashi" },
	{ SHURIKEN, "hira-shuriken" },
	{ SPLINT_MAIL, "dou-maru" },
	{ WOODEN_HARP, "koto" },
	{0, "" }
};

STATIC_OVL struct Jitem Pirate_items[] = {
	{ POT_BOOZE, "rum" },
	{ CRAM_RATION, "sea biscuit" },
	{ SCIMITAR, "cutlass" },
	{ SACK, "ditty bag" },
	{ OILSKIN_SACK, "oilskin ditty bag" },
	{ BOX, "foot locker" },
	{ CLUB, "belaying pin" },
	{0, "" }
};
#endif /* OVLB */

STATIC_DCL const char *FDECL(Alternate_item_name,(int i, struct Jitem * ));

#ifdef OVL1

STATIC_OVL char *
strprepend(s,pref)
register char *s;
register const char *pref;
{
	register int i = (int)strlen(pref);

	if(i > PREFIX) {
		impossible("PREFIX too short (for %d).", i);
		return(s);
	}
	s -= i;
	(void) strncpy(s, pref, i);	/* do not copy trailing 0 */
	return(s);
}

#endif /* OVL1 */
#ifdef OVLB

/* manage a pool of BUFSZ buffers, so callers don't have to */
static char *
nextobuf()
{
	static char NEARDATA bufs[NUMOBUF][BUFSZ];
	static int bufidx = 0;

	bufidx = (bufidx + 1) % NUMOBUF;
	return bufs[bufidx];
}

const char *
lightsaber_colorText(otmp)
struct obj *otmp;
{
	if(otmp->oartifact) switch(otmp->oartifact){
		case ART_ANNULUS: return Hallucination ? hcolor(0) : "cerulean";
		case ART_INFINITY_S_MIRRORED_ARC: {
			xchar x, y;
			int dnm = 0;
			get_obj_location(otmp, &x, &y, 0);
			if(levl[x][y].lit && 
				!(viz_array[y][x]&TEMP_DRK3 && 
				 !(viz_array[y][x]&TEMP_LIT1)
				)
			) dnm += 2;
			if(viz_array[y][x]&TEMP_LIT1 && 
				!(viz_array[y][x]&TEMP_DRK3)
			) dnm += 1;
			if(Hallucination) return hcolor(0);
			if(dnm == 3){
				if(In_outdoors(&u.uz)) return "sky-blue bladed";
				else if(In_W_tower(u.ux, u.uy, &u.uz)) return "heliotrope bladed";
				else if(In_hell(&u.uz)) return "scarlet bladed";
				else if(In_cave(&u.uz)) return "honey bladed";
				return "sun-white bladed";
			} else if(dnm == 1){
				if(rn2(2)) return "faint-orange bladed";
				else return "faint-yellow bladed";
			} else {
				if(In_outdoors(&u.uz)) return "mottled-blue bladed";
				else if(In_W_tower(u.ux, u.uy, &u.uz)) return "mottled-magenta bladed";
				else if(In_hell(&u.uz)) return "mottled-crimson bladed";
				else if(In_cave(&u.uz)) return "mottled-brown bladed";
				return "mottled-white bladed";
			}
		} break;
		case ART_ARKENSTONE: return Hallucination ? hcolor(0) : "rainbow-glinting sparking white";
		case ART_FLUORITE_OCTAHEDRON: return Hallucination ? hcolor(0) : "burning cerulean";
		case ART_HEART_OF_AHRIMAN: return Hallucination ? hcolor(0) : "pulsing and shimmering ruby";
		case ART_GLITTERSTONE: return Hallucination ? hcolor(0) : "glittering gold";
		
		default: return Hallucination ? hcolor(0) : LightsaberColor[((int)otmp->cobj->otyp) - MAGICITE_CRYSTAL].colorText;
	}
	if(otmp->otyp == KAMEREL_VAJRA)
		return "lightning bladed";
	return Hallucination ? hcolor(0) : LightsaberColor[((int)otmp->cobj->otyp) - MAGICITE_CRYSTAL].colorText;
}

char *
lightsaber_hiltText(otmp)
struct obj *otmp;
{
	if(otmp->oartifact == ART_ANNULUS) return "This %s is just a hollow silver pipe.";
	return SaberHilts[(int)otmp->ovar1];
}

int
random_saber_hilt()
{
	return rn2(SIZE(SaberHilts));
}

char *
obj_typename(otyp)
register int otyp;
{
	char *buf = nextobuf();
	register struct objclass *ocl = &objects[otyp];
	register const char *actualn = OBJ_NAME(*ocl);
	register const char *dn = OBJ_DESCR(*ocl);
	register const char *un = ocl->oc_uname;
	register int nn = ocl->oc_name_known;

	if (Role_if(PM_SAMURAI) && iflags.role_obj_names && Alternate_item_name(otyp,Japanese_items))
		actualn = Alternate_item_name(otyp,Japanese_items);
	if (Role_if(PM_SAMURAI) && iflags.obscure_role_obj_names && Alternate_item_name(otyp,ObscureJapanese_items))
		actualn = Alternate_item_name(otyp,ObscureJapanese_items);
	if (Role_if(PM_PIRATE) && iflags.role_obj_names && Alternate_item_name(otyp,Pirate_items))
		actualn = Alternate_item_name(otyp,Pirate_items);
	switch(ocl->oc_class) {
	case COIN_CLASS:
		Strcpy(buf, "coin");
		break;
	case POTION_CLASS:
		Strcpy(buf, "potion");
		break;
	case SCROLL_CLASS:
		Strcpy(buf, "scroll");
		break;
	case TILE_CLASS:
		Strcpy(buf, "shard");
		break;
	case WAND_CLASS:
		Strcpy(buf, "wand");
		break;
	case SPBOOK_CLASS:
		Strcpy(buf, "spellbook");
		break;
	case RING_CLASS:
		Strcpy(buf, "ring");
		break;
	case AMULET_CLASS:
		if(nn)
			Strcpy(buf,actualn);
		else
			Strcpy(buf,"amulet");
		if(un)
			Sprintf(eos(buf)," called %s",un);
		if(dn)
			Sprintf(eos(buf)," (%s)",dn);
		return(buf);
	default:
		if(nn) {
			Strcpy(buf, actualn);
			if (GemStone(otyp))
				Strcat(buf, " stone");
			if(un)
				Sprintf(eos(buf), " called %s", un);
			if(dn)
				Sprintf(eos(buf), " (%s)", dn);
		} else {
			Strcpy(buf, dn ? dn : actualn);
			if(ocl->oc_class == GEM_CLASS)
				Strcat(buf, (ocl->oc_material == MITHRIL || 
							 ocl->oc_material == SILVER || 
							 ocl->oc_material == MINERAL) ?
						" stone" : " gem");
			if(un)
				Sprintf(eos(buf), " called %s", un);
		}
		return(buf);
	}
	/* here for ring/scroll/potion/wand */
	if(nn) {
	    if (ocl->oc_unique)
		Strcpy(buf, actualn); /* avoid spellbook of Book of the Dead */
	    else
		Sprintf(eos(buf), " of %s", actualn);
	}
	if(un)
		Sprintf(eos(buf), " called %s", un);
	if(dn)
		Sprintf(eos(buf), " (%s)", dn);
	return(buf);
}

/* less verbose result than obj_typename(); either the actual name
   or the description (but not both); user-assigned name is ignored */
char *
simple_typename(otyp)
int otyp;
{
    char *bufp, *pp, *save_uname = objects[otyp].oc_uname;

    objects[otyp].oc_uname = 0;		/* suppress any name given by user */
    bufp = obj_typename(otyp);
    objects[otyp].oc_uname = save_uname;
    if ((pp = strstri(bufp, " (")) != 0)
	*pp = '\0';		/* strip the appended description */
    return bufp;
}

boolean
obj_is_pname(obj)
register struct obj *obj;
{
    return((boolean)(obj->dknown && obj->known && obj->onamelth && obj->oartifact));
}

/* Give the name of an object seen at a distance.  Unlike xname/doname,
 * we don't want to set dknown if it's not set already.  The kludge used is
 * to temporarily set Blind so that xname() skips the dknown setting.  This
 * assumes that we don't want to do this too often; if this function becomes
 * frequently used, it'd probably be better to pass a parameter to xname()
 * or doname() instead.
 */
char *
distant_name(obj, func)
register struct obj *obj;
char *FDECL((*func), (OBJ_P));
{
	char *str;

	long save_Blinded = Blinded;
	Blinded = 1;
	str = (*func)(obj);
	Blinded = save_Blinded;
	return str;
}

/* convert player specified fruit name into corresponding fruit juice name
   ("slice of pizza" -> "pizza juice" rather than "slice of pizza juice") */
char *
fruitname(juice)
boolean juice;	/* whether or not to append " juice" to the name */
{
    char *buf = nextobuf();
    const char *fruit_nam = strstri(pl_fruit, " of ");

    if (fruit_nam)
	fruit_nam += 4;		/* skip past " of " */
    else
	fruit_nam = pl_fruit;	/* use it as is */

    Sprintf(buf, "%s%s", makesingular(fruit_nam), juice ? " juice" : "");
    return buf;
}

#endif /* OVLB */
#ifdef OVL1

char *
xname(obj)
register struct obj *obj;
{
	return doxname(obj, FALSE, FALSE, FALSE);
}

static void
add_determiner_words(obj, buf)
struct obj *obj;
char *buf;
{
	char tmpbuf[PREFIX + 1];

	if (obj->quan != 1L && !(obj->quan == 8 && obj->oartifact == ART_FLUORITE_OCTAHEDRON))
	{
		Sprintf(tmpbuf, "%ld ", obj->quan);
		Strcat(buf, tmpbuf);
		if (obj->otyp == CORPSE && (mons[obj->corpsenm].geno & G_UNIQ))
		{
			Strcat(buf, "of ");
			if (!type_is_pname(&mons[obj->corpsenm])) Strcat(buf, "the ");
			Sprintf(tmpbuf, "%s ", s_suffix(mons[obj->corpsenm].mname));
			Strcat(buf, tmpbuf);
		}
	}
	else if (obj->otyp == CORPSE && (mons[obj->corpsenm].geno & G_UNIQ)) {
		if (!type_is_pname(&mons[obj->corpsenm])) Strcat(buf, "the ");
		Sprintf(tmpbuf, "%s ", s_suffix(mons[obj->corpsenm].mname));
		Strcat(buf, tmpbuf);
	}
	else if ((obj_is_pname(obj) || the_unique_obj(obj)) && !(obj->quan == 1 && obj->oartifact == ART_FLUORITE_OCTAHEDRON)) {
		Strcat(buf, "the ");
	}
	else
		Strcat(buf, "a ");
}

static void
add_stolen_words(obj, buf)
struct obj *obj;
char *buf;
{
	if (obj->sknown && obj->ostolen)
		Strcat(buf, "stolen ");
}

static void
add_buc_words(obj, buf)
struct obj *obj;
char *buf;
{
	if (obj->bknown &&
		obj->oclass != COIN_CLASS &&
		(obj->otyp != POT_WATER || !objects[POT_WATER].oc_name_known
		|| (!obj->cursed && !obj->blessed))) {
		/* allow 'blessed clear potion' if we don't know it's holy water;
		* always allow "uncursed potion of water"
		*/
		if (obj->cursed)
			Strcat(buf, (obj->known && (obj->oproperties&OPROP_UNHYW || obj->oproperties&OPROP_UNHY)) ? "unholy " : "cursed ");
		else if (obj->blessed)
			Strcat(buf, (obj->known && (obj->oproperties&OPROP_HOLYW || obj->oproperties&OPROP_HOLY)) ? "holy " : "blessed ");
		else if (iflags.show_buc || ((!obj->known || !objects[obj->otyp].oc_charged ||
			(obj->oclass == ARMOR_CLASS ||
			obj->oclass == RING_CLASS))
			/* For most items with charges or +/-, if you know how many
			* charges are left or what the +/- is, then you must have
			* totally identified the item, so "uncursed" is unneccesary,
			* because an identified object not described as "blessed" or
			* "cursed" must be uncursed.
			*
			* If the charges or +/- is not known, "uncursed" must be
			* printed to avoid ambiguity between an item whose curse
			* status is unknown, and an item known to be uncursed.
			*/
#ifdef MAIL
			&& obj->otyp != SCR_MAIL
#endif
			&& obj->otyp != FAKE_AMULET_OF_YENDOR
			&& obj->otyp != AMULET_OF_YENDOR
			&& !Role_if(PM_PRIEST)))
			Strcat(buf, "uncursed ");
	}
}

static void
add_size_words(obj, buf)
struct obj *obj;
char *buf;
{
	if (obj->objsize != ((obj->oartifact && artilist[obj->oartifact].size && obj->known) ? artilist[obj->oartifact].size : MZ_MEDIUM))
	{
		switch (obj->objsize)
		{
		case MZ_TINY:     Strcat(buf, "tiny ");     break;
		case MZ_SMALL:    Strcat(buf, "small ");    break;
		case MZ_MEDIUM:   Strcat(buf, "medium ");   break;
		case MZ_LARGE:    Strcat(buf, "large ");    break;
		case MZ_HUGE:     Strcat(buf, "huge ");     break;
		case MZ_GIGANTIC: Strcat(buf, "gigantic "); break;
		}
	}
}

static void
add_grease_words(obj, buf)
struct obj *obj;
char *buf;
{
	if (obj->greased)
		Strcat(buf, "greased ");
}

static void
add_shape_words(obj, buf, dofull)
struct obj *obj;
char *buf;
boolean dofull;
{
	switch (obj->oclass)
	{
	case WEAPON_CLASS:
		if (obj->otyp == VIPERWHIP && obj->ovar1){
			switch (obj->ovar1){
			case 1: Strcat(buf, "one-headed "); break;
			case 2: Strcat(buf, "two-headed "); break;
			case 3: Strcat(buf, "three-headed "); break;
			case 4: Strcat(buf, "four-headed "); break;
			case 5: Strcat(buf, "five-headed "); break;
			case 6: Strcat(buf, "six-headed "); break;
			case 7: Strcat(buf, "seven-headed "); break;
			case 8: Strcat(buf, "eight-headed "); break;
			}
		}

		if ((obj)->obj_material == WOOD && obj->oward)
			Strcat(buf, "carved ");

		if (obj->otyp == MOON_AXE && objects[obj->otyp].oc_name_known){
			switch (obj->ovar1){
			case ECLIPSE_MOON:  Strcat(buf, "eclipse ");  break;
			case CRESCENT_MOON: Strcat(buf, "crescent "); break;
			case HALF_MOON:     Strcat(buf, "half ");     break;
			case GIBBOUS_MOON:  Strcat(buf, "gibbous ");  break;
			case FULL_MOON:     Strcat(buf, "full ");     break;
			}
		}
		break;
	case ARMOR_CLASS:
		if ((obj->bodytypeflag&MB_BODYTYPEMASK) != MB_HUMANOID){
			if ((obj->bodytypeflag&MB_BODYTYPEMASK) == MB_ANIMAL) Strcat(buf, "barded ");
			else if ((obj->bodytypeflag&MB_BODYTYPEMASK) == MB_SLITHY) is_shirt(obj) ? Strcat(buf, "tubular ") : Strcat(buf, "segmented ");
			else if ((obj->bodytypeflag&MB_BODYTYPEMASK) == (MB_HUMANOID | MB_ANIMAL)) Strcat(buf, "centaur ");
			else if ((obj->bodytypeflag&MB_BODYTYPEMASK) == (MB_HUMANOID | MB_SLITHY)) Strcat(buf, "snakeleg ");
			else if ((obj->bodytypeflag&MB_BODYTYPEMASK) == (MB_ANIMAL | MB_SLITHY)) Strcat(buf, "snakeback ");
		}
		if ((obj->bodytypeflag&MB_HEADMODIMASK) != 0){
			if ((obj->bodytypeflag&MB_HEADMODIMASK) == MB_LONGHEAD) Strcat(buf, "barded ");
			else if ((obj->bodytypeflag&MB_HEADMODIMASK) == MB_LONGNECK) Strcat(buf, "snakeneck ");
		}
		break;
	case TOOL_CLASS:
		if (dofull)
		{
			if (Role_if(PM_EXILE) && obj->otyp == BELL_OF_OPENING){
				Strcat(buf, "warped and cracked ");
			}
			if (
				obj->oartifact == ART_SECOND_KEY_OF_CHAOS ||
				obj->oartifact == ART_SECOND_KEY_OF_LAW ||
				obj->oartifact == ART_SECOND_KEY_OF_NEUTRALITY
				){
				Strcat(buf, "etched ");
			}
			if (Is_candle(obj) && obj->otyp != CANDLE_OF_INVOCATION &&
				obj->age < 20L * (long)objects[obj->otyp].oc_cost)
				Strcat(buf, "partly used ");
		}
		break;
	case RING_CLASS:
		if (dofull)
		{
			if (obj->oward && (isEngrRing(obj->otyp)))
				Strcat(buf, "engraved ");
		}
		break;
	case FOOD_CLASS:
		if (dofull)
		{
			if (obj->otyp == CORPSE && obj->odrained) {
#ifdef WIZARD
				if (wizard && obj->oeaten < drainlevel(obj))
					Strcat(buf, "over-drained ");
				else
#endif
				if (obj->oeaten > drainlevel(obj))
					Strcat(buf, "partly drained ");
				else
					Strcat(buf, "drained ");
			}
			else if (obj->oeaten)
				Strcat(buf, "partly eaten ");

#if 0	/* corpses don't tell if they're stale either */
			if (obj->otyp == EGG && obj->known && stale_egg(obj))
				Strcat(buf, "stale ");
#endif
		}

	}

}

static void
add_erosion_words(obj, buf)
struct obj *obj;
char *buf;
{
	boolean iscrys = (obj->otyp == CRYSKNIFE);
	if (!is_damageable(obj) && !iscrys) return;

	/* The only cases where any of these bits do double duty are for
	* rotted food and diluted potions, which are all not is_damageable().
	*/
	if (obj->oeroded && !iscrys) {
		switch (obj->oeroded) {
		case 2:	Strcat(buf, "very "); break;
		case 3:	Strcat(buf, "thoroughly "); break;
		}
		Strcat(buf, is_rustprone(obj) ? "rusty " :
			is_evaporable(obj) ? "tenuous " : "burnt ");
	}
	if (obj->oeroded2 && !iscrys) {
		switch (obj->oeroded2) {
		case 2:	Strcat(buf, "very "); break;
		case 3:	Strcat(buf, "thoroughly "); break;
		}
		Strcat(buf, is_corrodeable(obj) ? "corroded " :
			"rotted ");
	}
	if (obj->oeroded3) {
		switch (obj->oeroded3) {
		case 2:	Strcat(buf, "very "); break;
		case 3:	Strcat(buf, "thoroughly "); break;
		}
		Strcat(buf, "tattered ");
	}
	if (obj->ovar1 && obj->otyp == MASK) {
		switch (obj->ovar1) {
		case 1:	Strcat(buf, "cracked "); break;
		case 2:	Strcat(buf, "chipped "); break;
		case 3:	Strcat(buf, "fragmentary "); break;
		}
	}
	if (obj->rknown && obj->oerodeproof)
		Strcat(buf,
		(iscrys || is_evaporable(obj)) ? "fixed " :
		is_rustprone(obj) ? "rustproof " :
		is_corrodeable(obj) ? "corrodeproof " :	/* "stainless"? */
		is_flammable(obj) ? "fireproof " : "");
}

static void
add_properties_words(obj, buf, dofull)
struct obj *obj;
char *buf;
boolean dofull;
{
	if ((obj->oproperties&OPROP_WOOL) == OPROP_WOOL && !(obj->obj_material != objects[obj->otyp].oc_material && obj->obj_material == CLOTH)){
		if (obj->obj_material == CLOTH)
			Strcat(buf, "woolen ");
		else
			Strcat(buf, "wool-lined ");
	} else if(obj->oproperties && (obj->oartifact == 0 || dofull)){
		if(obj->oproperties&OPROP_ANAR && obj->known)
			Strcat(buf, "anarchic ");
		if(obj->oproperties&OPROP_CONC && obj->known)
			Strcat(buf, "concordant ");
		if(obj->oproperties&OPROP_AXIO && obj->known)
			Strcat(buf, "axiomatic ");
		if(obj->oproperties&OPROP_MAGC && obj->known)
			Strcat(buf, "magic-resistant ");
		if(obj->oproperties&OPROP_REFL)
			Strcat(buf, "reflective ");
		if(obj->oproperties&OPROP_FIRE && obj->known)
			Strcat(buf, "fireproof ");
		if(obj->oproperties&OPROP_COLD && obj->known)
			Strcat(buf, "coldproof ");
		if(obj->oproperties&OPROP_ELEC && obj->known)
			Strcat(buf, "voltproof ");
		if(obj->oproperties&OPROP_ACID && obj->known)
			Strcat(buf, "acidproof ");
	}

	if (obj->oproperties && (obj->oartifact == 0 || dofull)){
		if(obj->oproperties&OPROP_LESSW 
			&& obj->oproperties&OPROP_AXIOW 
			&& obj->oproperties&(OPROP_FIREW|OPROP_COLDW|OPROP_ELECW)
			&& obj->known
		){
			if (obj->oproperties&OPROP_FIREW)
				Strcat(buf, "forge-hot ");
			if (obj->oproperties&OPROP_COLDW)
				Strcat(buf, "crystalline ");
			if (obj->oproperties&OPROP_ELECW)
				Strcat(buf, "arcing ");
		}
		else {
			if(obj->oproperties&OPROP_LESSW && obj->known)
				Strcat(buf, "lesser ");
			if (obj->oproperties&OPROP_ANARW && obj->known)
				Strcat(buf, "anarchic ");
			if (obj->oproperties&OPROP_CONCW && obj->known)
				Strcat(buf, "concordant ");
			if (obj->oproperties&OPROP_AXIOW && obj->known)
				Strcat(buf, "axiomatic ");
			if (obj->oproperties&OPROP_PSIOW){
				if (obj->known) Strcat(buf, "psionic ");
				else if (Blind_telepat) Strcat(buf, "whispering ");
			}
			if (obj->oproperties&OPROP_DEEPW){
				if (Blind_telepat && obj->spe < 8) Strcat(buf, "mumbling ");
			}
			if (obj->oproperties&OPROP_PHSEW)
				Strcat(buf, "faded ");
			if (obj->oproperties&OPROP_WATRW)
				Strcat(buf, "misty ");
			if (obj->oproperties&OPROP_FIREW){
				if (obj->oproperties&OPROP_LESSW) Strcat(buf, "red-hot ");
				else Strcat(buf, "flaming ");
			}
			if (obj->oproperties&OPROP_COLDW)
				Strcat(buf, "freezing ");
			if (obj->oproperties&OPROP_ELECW)
				Strcat(buf, "shocking ");
			if (obj->oproperties&OPROP_ACIDW)
				Strcat(buf, "sizzling ");
			if (obj->oproperties&OPROP_MAGCW)
				Strcat(buf, "sparkling ");
			if (obj->oproperties&OPROP_VORPW && obj->known)
				Strcat(buf, "vorpal ");
			if (obj->oproperties&OPROP_MORGW && obj->known && obj->cursed)
				Strcat(buf, "morgul ");
			if (obj->oproperties&OPROP_FLAYW && obj->known)
				Strcat(buf, "flaying ");
			/* note: "holy" and "unholy" properties are shown in the BUC part of the name, as they replace "blessed" and "cursed". */
		}
	}
}

static void
add_enchantment_number(obj, buf)
struct obj *obj;
char *buf;
{
	if ((obj->oclass == WEAPON_CLASS) ||
		(obj->oclass == ARMOR_CLASS) ||
		(obj->oclass == TOOL_CLASS && is_weptool(obj)) ||
		(obj->oclass == RING_CLASS && objects[obj->otyp].oc_charged && obj->otyp != RIN_WISHES)
		)
		if (obj->known || Race_if(PM_INCANTIFIER)) {
			Strcat(buf, sitoa(obj->spe));
			Strcat(buf, " ");
		}
}

static void
add_poison_words(obj, buf)
struct obj *obj;
char *buf;
{
	if (obj->opoisoned){
		if (obj->opoisoned & OPOISON_BASIC) Strcat(buf, "poisoned ");
		if (obj->opoisoned & OPOISON_FILTH) Strcat(buf, "filth-crusted ");
		if (obj->opoisoned & OPOISON_SLEEP) Strcat(buf, "drug-coated ");
		if (obj->opoisoned & OPOISON_BLIND) Strcat(buf, "stained ");
		if (obj->opoisoned & OPOISON_PARAL) Strcat(buf, "envenomed ");
		if (obj->opoisoned & OPOISON_AMNES) Strcat(buf, "lethe-rusted ");
		if (obj->opoisoned & OPOISON_ACID)  Strcat(buf, "acid-coated ");
		if (obj->otyp == VIPERWHIP && obj->opoisonchrgs) Sprintf(eos(buf), "(%d coatings) ", (int)(obj->opoisonchrgs + 1));
	}
}

static void
add_colours_words(obj, buf)
struct obj *obj;
char *buf;
{
#ifdef INVISIBLE_OBJECTS
	if (obj->oinvis) Strcat(buf, "invisible ");
	else
#endif
	if (is_lightsaber(obj) && litsaber(obj)){
		Strcat(buf, lightsaber_colorText(obj));
		Strcat(buf, " ");
	}
}

static void
add_material_words(obj, buf)
struct obj *obj;
char *buf;
{
	/*To avoid an if statement with a massive condition, detect cases where the material should NOT be printed, and return out*/
	/*Avoid obviating randomized appearances*/
	if(id_for_material(obj) && !obj->known)
		return;
	/*Materials don't matter for lit lightsabers, and they should be described in terms of color*/
	if(is_lightsaber(obj) && litsaber(obj))
		return;
	if(obj->oartifact && obj->known){
		/*Known artifact is made from standard material*/
		if(artilist[obj->oartifact].material && obj->obj_material == artilist[obj->oartifact].material)
			return;
	} else {
		/*Non-artifact is made from standard material, and isn't of a type for which the material is always shown*/
		if(obj->known && !(objects[obj->otyp].oc_showmat&IDED) && obj->obj_material == objects[obj->otyp].oc_material)
			return;
		/*Unknown item is made from standard material, and isn't of a type for which the material is always shown*/
		if(!obj->known && !(objects[obj->otyp].oc_showmat&UNIDED) && obj->obj_material == objects[obj->otyp].oc_material)
			return;
	}
	if (obj->oartifact == ART_HOLY_MOONLIGHT_SWORD && obj->lamplit){
		Strcat(buf, "pale moonlight ");
	}
	else switch (obj->obj_material){
	case LIQUID: /*Wut?*/
		Strcat(buf, "liquid ");
		break;
	case WAX:
		Strcat(buf, "wax ");
		break;
	case VEGGY:
		Strcat(buf, "straw ");
		break;
	case FLESH:
		Strcat(buf, "flesh ");
		break;
	case PAPER:
		Strcat(buf, "paper ");
		break;
	case CLOTH:
		if ((obj->oproperties&OPROP_WOOL) == OPROP_WOOL)
			Strcat(buf, "woolen ");
		else
			Strcat(buf, "cloth ");
		break;
	case LEATHER:
		Strcat(buf, "leather ");
		break;
	case WOOD:
		Strcat(buf, "wooden ");
		break;
	case BONE:
		if (obj->oartifact == ART_WEBWEAVER_S_CROOK) Strcat(buf, "chitin ");
		else Strcat(buf, "bone ");
		break;
	case DRAGON_HIDE:
		if (objects[obj->otyp].oc_material > LEATHER && objects[obj->otyp].oc_material != DRAGON_HIDE)
			obj->oclass == WEAPON_CLASS ? Strcat(buf, "dragon-tooth ") : Strcat(buf, "dragon-bone ");
		else
			Strcat(buf, "dragon-scale ");
		break;
	case IRON:
		if (obj->oartifact == ART_STEEL_SCALES_OF_KURTULMAK) Strcat(buf, "steel ");
		else Strcat(buf, "iron ");
		break;
	case METAL:
		if(!(obj->oproperties&OPROP_LESSW 
			&& obj->oproperties&OPROP_AXIOW 
			&& obj->oproperties&(OPROP_FIREW|OPROP_COLDW|OPROP_ELECW)
			&& obj->known
		)) Strcat(buf, "metallic ");
		break;
	case COPPER:
		Strcat(buf, "bronze ");
		break;
	case SILVER:
		Strcat(buf, "silver ");
		break;
	case GOLD:
		Strcat(buf, "golden ");
		break;
	case PLATINUM:
		Strcat(buf, "platinum ");
		break;
	case MITHRIL:
		Strcat(buf, "mithril ");
		break;
	case PLASTIC:
		Strcat(buf, "plastic ");
		break;
	case GLASS:
		Strcat(buf, "glass ");
		break;
	case GEMSTONE:
		Strcat(buf, "gem ");
		break;
	case MINERAL:
		(obj->oclass == ARMOR_CLASS || obj->oclass == TILE_CLASS) ? Strcat(buf, "ceramic ") : Strcat(buf, "stone ");
		break;
	case OBSIDIAN_MT:
		Strcat(buf, "obsidian ");
		break;
	case SHADOWSTUFF:
		Strcat(buf, "black ");
		break;
	}
}

static void
add_type_words(obj, buf)
struct obj *obj;
char *buf;
{
	if (obj->otyp == MASK){
		Strcat(buf, mons[obj->corpsenm].mname);
		Strcat(buf, " ");
	}
	if (obj->otyp == CORPSE) {
		if (!(mons[obj->corpsenm].geno & G_UNIQ)) {
			Strcat(buf, mons[obj->corpsenm].mname);
			Strcat(buf, " ");
		}
	}
	if (obj->otyp == EGG && obj->corpsenm >= LOW_PM && (obj->known || mvitals[obj->corpsenm].mvflags & MV_KNOWS_EGG)) {
		Strcat(buf, mons[obj->corpsenm].mname);
		Strcat(buf, " ");
	}
}

/* Supposedly, the order of adjectives in English is: determiner, opinion, size, shape, age, colour, origin, material, purpose, noun
 * We will transfer that over to dNethack as:
 * quantity, stolen, BUC, size, shape, erosion, grease, enchantment, properties, poison, colour, material, object
 * "a stolen uncursed small three-headed rusted rustproof greased +2 flaming poisoned iron viperwhip"
 * "a cursed -1 lightning bladed kamerel vajra"
 */
char *
doxname(obj, dofull, ignore_oquan, with_price)
struct obj * obj;
boolean dofull;
boolean ignore_oquan;
boolean with_price;
{
	register char *buf;
	register int typ = obj->otyp;
	register struct objclass *ocl = &objects[typ];
	register int nn = ocl->oc_name_known;
	register const char *actualn = OBJ_NAME(*ocl);
	register const char *dn = OBJ_DESCR(*ocl);
	register const char *un = ocl->oc_uname;

	buf = nextobuf() + PREFIX;	/* leave room for "17 -3 " */
	if (Role_if(PM_SAMURAI) && iflags.role_obj_names && Alternate_item_name(typ, Japanese_items))
		actualn = Alternate_item_name(typ, Japanese_items);
	if (Role_if(PM_SAMURAI) && iflags.obscure_role_obj_names && Alternate_item_name(typ, ObscureJapanese_items))
		actualn = Alternate_item_name(typ, ObscureJapanese_items);
	if (Role_if(PM_PIRATE) && iflags.role_obj_names && Alternate_item_name(typ, Pirate_items))
		actualn = Alternate_item_name(typ, Pirate_items);

	buf[0] = '\0';
	/*
	* clean up known when it's tied to oc_name_known, eg after AD_DRIN
	* This is only required for unique objects since the article
	* printed for the object is tied to the combination of the two
	* and printing the wrong article gives away information.
	*/
	if (!nn && ocl->oc_uses_known && ocl->oc_unique) obj->known = 0;
	if (!Blind) obj->dknown = TRUE;
	if (Role_if(PM_PRIEST)) obj->bknown = TRUE;
	if (u.sealsActive&SEAL_ANDROMALIUS) obj->sknown = TRUE;
	//if (obj_is_pname(obj)) goto nameit;

	if (dofull) add_determiner_words(obj, buf);	// quantity or "a" or "the"
	/* general descriptors */
	if (dofull) add_stolen_words(obj, buf);
	if (dofull) add_buc_words(obj, buf);
	add_size_words(obj, buf);	// TODO - hide some artifact's sizes, currently shows all
	add_shape_words(obj, buf, dofull);		// Note: more verbose for a number of objects if dofull is true
	if (dofull) add_erosion_words(obj, buf);
	if (dofull) add_grease_words(obj, buf);
	if (dofull) add_enchantment_number(obj, buf);
	add_properties_words(obj, buf, dofull);	// Note: more verbose for artifacts if dofull is true
	add_poison_words(obj, buf);
	add_colours_words(obj, buf);
	add_material_words(obj, buf);	// TODO - show some artifact's materials, currently hides all
	if (dofull) add_type_words(obj, buf);

	/* finishing up xname stuff, which has a lot of special cases */
	if (!obj_is_pname(obj))
	{
		switch (obj->oclass) {
		case AMULET_CLASS:
			if (!obj->dknown)
				Strcat(buf, "amulet");
			else if (typ == AMULET_OF_YENDOR ||
				typ == FAKE_AMULET_OF_YENDOR)
				/* each must be identified individually */
				Strcat(buf, obj->known ? actualn : dn);
			else if (nn)
				Strcat(buf, actualn);
			else if (un)
				Sprintf(eos(buf), "amulet called %s", un);
			else
				Sprintf(eos(buf), "%s amulet", dn);
			break;
		case WEAPON_CLASS:
		case VENOM_CLASS:
		case TOOL_CLASS:
			if (typ == LENSES)
				Strcat(buf, "pair of ");
			if (typ == HYPOSPRAY_AMPULE){
				int ptyp = (int)(obj->ovar1);
				struct objclass *pcl = &objects[ptyp];
				// register int pnn = ocl->oc_name_known;
				register const char *pactualn = OBJ_NAME(*pcl);
				// register const char *pdn = OBJ_DESCR(*pcl);
				// register const char *pun = pcl->oc_uname;
				if (!obj->dknown); //add "ampule" below and finish
				else if (nn) {
					if (ptyp == POT_WATER &&
							obj->bknown && (obj->blessed || obj->cursed)
						) {
						Strcat(buf, obj->blessed ? "holy " : "unholy ");
					}
					Strcat(buf, pactualn);
					Strcat(buf, " ");
				}
				else if (un) {
					Strcat(buf, "ampule called ");
					Strcat(buf, un);
					break;
				}
				else {
					Strcat(buf, dn);
					break;
				}
				Strcat(buf, "ampule");
				Sprintf(eos(buf), " (%d doses)", (int)(obj->spe));
				break;
			}
			else if (!obj->dknown)
				Strcat(buf, dn ? dn : actualn);
			else if (nn)
				Strcat(buf, actualn);
			else if (un) {
				Strcat(buf, dn ? dn : actualn);
				Strcat(buf, " called ");
				Strcat(buf, un);
			}
			else Strcat(buf, dn ? dn : actualn);
			/* If we use an() here we'd have to remember never to use */
			/* it whenever calling doname() or xname(). */
			if (typ == FIGURINE)
				Sprintf(eos(buf), " of a%s %s",
				index(vowels, *(mons[obj->corpsenm].mname)) ? "n" : "",
				mons[obj->corpsenm].mname);
			else if (is_blaster(obj) && (obj->known || Race_if(PM_ANDROID)))
				Sprintf(eos(buf), " (%d:%d)", (int)obj->recharged, (int)obj->ovar1);
			else if (is_vibroweapon(obj) && (obj->known || Race_if(PM_ANDROID)))
				Sprintf(eos(buf), " (%d:%d)", (int)obj->recharged, (int)obj->ovar1);
			else if (obj->otyp == SEISMIC_HAMMER && (obj->known || Race_if(PM_ANDROID)))
				Sprintf(eos(buf), " (%d:%d)", (int)obj->recharged, (int)obj->ovar1);
			break;
		case ARMOR_CLASS:
			/* depends on order of the dragon scales objects */
			if (typ >= GRAY_DRAGON_SCALES && typ <= YELLOW_DRAGON_SCALES) {
				Sprintf(eos(buf), "set of %s", actualn);
				break;
			}
			if ((typ == VICTORIAN_UNDERWEAR && nn) || (typ == JUMPSUIT && !nn) || (typ == BODYGLOVE && !nn)) {
				Sprintf(eos(buf), "set of %s", actualn);
				break;
			}
			if (is_boots(obj) || is_gloves(obj)) Strcat(buf, "pair of ");

			if (obj->otyp >= ELVEN_SHIELD && obj->otyp <= ORCISH_SHIELD
				&& !obj->dknown) {
				Strcat(buf, "shield");
				break;
			}
			if (obj->otyp == SHIELD_OF_REFLECTION && !obj->dknown) {
				Strcat(buf, "smooth shield");
				break;
			}

			if (nn)	Strcat(buf, actualn);
			else if (un) {
				if (is_boots(obj))
					Strcat(buf, "boots");
				else if (is_gloves(obj))
					Strcat(buf, "gloves");
				else if (is_cloak(obj))
					Strcat(buf, "cloak");
				else if (is_helmet(obj))
					Strcat(buf, "helmet");
				else if (is_shield(obj))
					Strcat(buf, "shield");
				else
					Strcat(buf, "armor");
				Strcat(buf, " called ");
				Strcat(buf, un);
			}
			else	Strcat(buf, dn);
			break;
		case FOOD_CLASS:
			if (typ == SLIME_MOLD) {
				register struct fruit *f;

				for (f = ffruit; f; f = f->nextf) {
					if (f->fid == obj->spe) {
						Strcat(buf, f->fname);
						break;
					}
				}
				if (!f) impossible("Bad fruit #%d?", obj->spe);
				break;
			}

			if (typ == EYEBALL && obj->known) {
				if (obj->corpsenm != NON_PM)
					Sprintf(eos(buf), "%s ", mons[obj->corpsenm].mname);
			}

			Strcat(buf, actualn);
			if (typ == TIN && obj->known) {
				if (obj->spe > 0)
					Strcat(buf, " of spinach");
				else if (obj->corpsenm == NON_PM)
					Strcpy(buf, "empty tin");
				else if (vegetarian(&mons[obj->corpsenm]))
					Sprintf(eos(buf), " of %s", mons[obj->corpsenm].mname);
				else
					Sprintf(eos(buf), " of %s meat", mons[obj->corpsenm].mname);
			}
			break;
		case COIN_CLASS:
		case CHAIN_CLASS:
			Strcat(buf, actualn);
			if (obj->owornmask & W_ARM)
				Strcat(eos(buf), " (wrapped around chest)");
			else if (obj->owornmask & W_ARMC)
				Strcat(eos(buf), " (draped over shoulders)");
			else if (obj->owornmask & W_ARMH)
				Strcat(eos(buf), " (wrapped around head)");
			else if (obj->owornmask & W_ARMG)
				Strcat(eos(buf), " (wrapped around arms)");
			else if (obj->owornmask & W_ARMF)
				Strcat(eos(buf), " (wrapped around legs)");
			break;
		case ROCK_CLASS:
			if (typ == STATUE)
				Sprintf(eos(buf), "%s%s of %s%s",
				(Role_if(PM_ARCHEOLOGIST) && (obj->spe & STATUE_HISTORIC)) ? "historic " : "",
				actualn,
				type_is_pname(&mons[obj->corpsenm]) ? "" :
				(mons[obj->corpsenm].geno & G_UNIQ) ? "the " :
				(index(vowels, *(mons[obj->corpsenm].mname)) ?
				"an " : "a "),
				mons[obj->corpsenm].mname);
			else if (typ == FOSSIL)
				Sprintf(eos(buf), "%s %s",
				mons[obj->corpsenm].mname,
				actualn);
			else Strcat(buf, actualn);
			break;
		case BALL_CLASS:
			Sprintf(eos(buf), "%sheavy iron ball",
				(obj->owt > ocl->oc_weight) ? "very " : "");
			break;
		case POTION_CLASS:
			if (obj->dknown && obj->odiluted)
				Strcat(buf, "diluted ");
			if (typ == POT_BLOOD && (obj->known || is_vampire(youracedata))) {
				Strcat(buf, "potion");
				Sprintf(eos(buf), " of %s blood", mons[obj->corpsenm].mname);
			}
			else if (nn || un || !obj->dknown) {
				Strcat(buf, "potion");
				if (!obj->dknown) break;
				if (nn) {
					Strcat(buf, " of ");
					if (typ == POT_WATER &&
						obj->bknown && (obj->blessed || obj->cursed)) {
						Strcat(buf, obj->blessed ? "holy " : "unholy ");
					}
					Strcat(buf, actualn);
				}
				else {
					Strcat(buf, " called ");
					Strcat(buf, un);
				}
			}
			else {
				Strcat(buf, dn);
				Strcat(buf, " potion");
			}
			break;
		case SCROLL_CLASS:
		if(obj->otyp < SCR_BLANK_PAPER){
			if (obj->dknown && !un && !ocl->oc_magic)
			{
				Strcat(buf, dn);
				Strcat(buf, " ");
			}
			Strcat(buf, "scroll");
			if (!obj->dknown) break;
			if (nn) {
				Strcat(buf, " of ");
				if (obj->otyp != SCR_WARD) Strcat(buf, actualn);
				else Strcat(buf, wardDecode[obj->oward]);
			}
			else if (un) {
				Strcat(buf, " called ");
				Strcat(buf, un);
			}
			else if (ocl->oc_magic) {
				Strcat(buf, " labeled ");
				Strcat(buf, dn);
			}
			else {
				// "unlabeled scroll" should be the only case, and is already handled above.
				;
			}
		} else {
			if (!obj->dknown)
				Strcat(buf, dn ? dn : actualn);
			else if (nn)
				Strcat(buf, actualn);
			else if (un) {
				Strcat(buf, dn ? dn : actualn);
				Strcat(buf, " called ");
				Strcat(buf, un);
			}
			else Strcat(buf, dn ? dn : actualn);
		}
	break;
	case TILE_CLASS:
		if (obj->dknown && !un && !ocl->oc_magic){
			Strcat(buf, dn);
			Strcat(buf, " ");
		}
		if(ocl->oc_unique)
			Strcat(buf, "slab");
		else
			Strcat(buf, "shard");
		if (!obj->dknown) break;
		if (nn) {
			Strcat(buf, " bearing the ");
			Strcat(buf, actualn);
		}
		else if (un){
			Strcat(buf, " called ");
			Strcat(buf, un);
		}
		else if (ocl->oc_magic){
			Strcat(buf, " with a ");
			Strcat(buf, dn);
		}
		else {
		// "unlabeled scroll" should be the only case, and is already handled above.
		;
		}
			break;
		case WAND_CLASS:
			if (!obj->dknown)
				Strcat(buf, "wand");
			else if (nn)
				Sprintf(eos(buf), "wand of %s", actualn);
			else if (un)
				Sprintf(eos(buf), "wand called %s", un);
			else
				Sprintf(eos(buf), "%s wand", dn);
			break;
		case SPBOOK_CLASS:
			if (!obj->dknown) {
				Strcat(buf, "spellbook");
			}
			else if (nn) {
				if (typ != SPE_BOOK_OF_THE_DEAD)
					Strcat(buf, "spellbook of ");
				Strcat(buf, actualn);
			}
			else if (un) {
				Sprintf(eos(buf), "spellbook called %s", un);
			}
			else
				Sprintf(eos(buf), "%s spellbook", dn);
			break;
		case RING_CLASS:
			if (!obj->dknown)
				Strcat(buf, "ring");
			else if (nn)
				Sprintf(eos(buf), "ring of %s", actualn);
			else if (un)
				Sprintf(eos(buf), "ring called %s", un);
			else
				Sprintf(eos(buf), "%s ring", dn);
			break;
		case GEM_CLASS:
		{
						  const char *rock =
							  (ocl->oc_material == MINERAL ||
							  ocl->oc_material == MITHRIL ||
							  ocl->oc_material == SILVER
							  ) ? "stone" : "gem";
						  if (!obj->dknown) {
							  Strcat(buf, rock);
						  }
						  else if (!nn) {
							  if (un) Sprintf(eos(buf), "%s called %s", rock, un);
							  else Sprintf(eos(buf), "%s %s", dn, rock);
						  }
						  else {
							  Strcat(buf, actualn);
							  if (GemStone(typ)) Strcat(buf, " stone");
						  }
						  break;
		}
		default:
			Sprintf(eos(buf), "glorkum %d %d %d", obj->oclass, typ, obj->spe);
		}
#ifdef SORTLOOT
		if (!ignore_oquan)
#endif
		if (obj->quan != 1L) Strcpy(buf, makeplural(buf));
	}//endif !obj_is_pname(obj)

	if ((obj->onamelth && obj->dknown) || (obj_is_pname(obj))) {
		if (!obj_is_pname(obj) && obj->onamelth && obj->dknown) Strcat(buf, " named ");
		if (obj_is_pname(obj) && obj->known && (obj->oartifact == ART_FLUORITE_OCTAHEDRON)){
			if (obj->quan == 8) Strcat(buf, "Fluorite Octet");
			else if (obj->quan > 1) Strcat(buf, "Fluorite Octahedra");
			else Strcat(buf, "Fluorite Octahedron");
		}
		else if (obj_is_pname(obj) && obj->known && !strncmpi(ONAME(obj), "the ", 4))
			Strcat(buf, ONAME(obj)+4);
		else 
			Strcat(buf, ONAME(obj));
	}

	if (!dofull && !strncmpi(buf, "the ", 4)) buf += 4;

	/* Suffixes applied by dofull. From this point on, if dofull is not enabled, no changes should be made to buf */
	if (dofull)
	{
		const struct artifact *oart = 0;
		if (obj && obj->oartifact) oart = &artilist[(obj)->oartifact];

		switch (obj->oclass) {
		case AMULET_CLASS:
			if (obj->owornmask & W_AMUL)
				Strcat(buf, " (being worn)");
			break;
		case WEAPON_CLASS:
			if (obj->known && obj->oartifact &&
				(oart->inv_prop == LORDLY || oart->inv_prop == ANNUL)
				){
				Sprintf(eos(buf), " (%s)", OBJ_NAME(objects[obj->otyp]));
			}
			if (obj->oartifact == ART_SCALPEL_OF_LIFE_AND_DEATH){
				if (COMMAND_LIFE == obj->ovar1) Sprintf(eos(buf), " (life)");
				if (COMMAND_DEATH == obj->ovar1) Sprintf(eos(buf), " (death)");
			}
			if(obj->oartifact == ART_SINGING_SWORD && uwep == obj){
				switch(obj->osinging){
					default:
						Sprintf(eos(buf), " (tuneless song)");
					break;
					case OSING_FEAR:
						Sprintf(eos(buf), " (ominous chanting)");
					break;
					case OSING_HEALING:
						Sprintf(eos(buf), " (soothing humming)");
					break;
					case OSING_RALLY:
						Sprintf(eos(buf), " (soft whistling)");
					break;
					case OSING_CONFUSE:
						Sprintf(eos(buf), " (horrid cacophony)");
					break;
					case OSING_HASTE:
						Sprintf(eos(buf), " (marching song)");
					break;
					case OSING_LETHARGY:
						Sprintf(eos(buf), " (lethargic song)");
					break;
					case OSING_COURAGE:
						Sprintf(eos(buf), " (inspiring song)");
					break;
					case OSING_DIRGE:
						Sprintf(eos(buf), " (dismal dirge)");
					break;
					case OSING_FIRE:
						Sprintf(eos(buf), " (fiery song)");
					break;
					case OSING_FROST:
						Sprintf(eos(buf), " (chilling song)");
					break;
					case OSING_ELECT:
						Sprintf(eos(buf), " (electrifying song)");
					break;
					case OSING_QUAKE:
						Sprintf(eos(buf), " (earthshaking chant)");
					break;
					case OSING_OPEN:
						Sprintf(eos(buf), " (high ringing)");
					break;
					case OSING_DEATH:
						Sprintf(eos(buf), " (vast rushing)");
					break;
					case OSING_LIFE:
						Sprintf(eos(buf), " (heartbeat cadence)");
					break;
					case OSING_INSANE:
						Sprintf(eos(buf), " (monotonous whining)");
					break;
					case OSING_CANCEL:
						Sprintf(eos(buf), " (discordant notes)");
					break;
				}
			}
			//#ifdef FIREARMS
			if (obj->otyp == STICK_OF_DYNAMITE) {
				if (obj->lamplit) Strcat(buf, " (lit)");
#  ifdef DEBUG
				Sprintf(eos(buf), " (%d)", obj->age);
#  endif
			}
			else if (is_grenade(obj)){
				if (obj->oarmed) Strcat(buf, " (armed)");
#  ifdef DEBUG
				Sprintf(eos(buf), " (%d)", obj->age);
#  endif
			}
			//#endif	/* FIREARMS */
			if (obj->otyp == RAYGUN){
				if (obj->altmode == AD_SLEE) Strcat(buf, " (stun)");
				else if (obj->altmode == AD_FIRE) Strcat(buf, " (heat)");
				else if (obj->altmode == AD_DEAD) Strcat(buf, " (kill)");
				else if (obj->altmode == AD_DISN) Strcat(buf, " (disintegrate)");
			}
			else if (obj->otyp == ARM_BLASTER ||
				obj->otyp == ASSAULT_RIFLE ||
				obj->otyp == BFG ||
				obj->otyp == AUTO_SHOTGUN ||
				obj->otyp == SUBMACHINE_GUN
				){
				if (obj->altmode == WP_MODE_AUTO) Strcat(buf, " (auto)");
				else if (obj->altmode == WP_MODE_BURST) Strcat(buf, " (burst)");
				else if (obj->altmode == WP_MODE_SINGLE) Strcat(buf, " (single)");
			}
			else if (obj->oartifact == ART_HOLY_MOONLIGHT_SWORD && obj->lamplit) {
				Strcat(buf, " (lit)");
			}
			else if (is_lightsaber(obj)) {
				if (litsaber(obj)){
					if (obj->oartifact == ART_INFINITY_S_MIRRORED_ARC){
						xchar x, y;
						int dnm = 0;
						get_obj_location(obj, &x, &y, 0);
						if (levl[x][y].lit &&
							!(viz_array[y][x] & TEMP_DRK3 &&
							!(viz_array[y][x] & TEMP_LIT1)
							)
							) dnm += 2;
						if (viz_array[y][x] & TEMP_LIT1 &&
							!(viz_array[y][x] & TEMP_DRK3)
							) dnm += 1;
						if (obj->altmode){
							if (dnm > 1) Strcat(buf, " (two blades lit)");
							else Strcat(buf, " (two blades flickering)");
						}
						else if (dnm <= 1) Strcat(buf, " (flickering)");
					}
					else if (obj->otyp == KAMEREL_VAJRA){
						Strcat(buf, " (lit)");
					}
					else {
						if (obj->altmode){
							if (obj->age > 1000) Strcat(buf, " (two blades lit)");
							else Strcat(buf, " (two blades flickering)");
						}
						else {
							if (obj->age > 1000) Strcat(buf, " (lit)");
							else Strcat(buf, " (flickering)");
						}
					}
					// Sprintf(eos(buf), " (%d)", obj->age);
				}
#  ifdef DEBUG
				Sprintf(eos(buf), " (%d)", obj->age);
#  endif
			}
			else if (obj->otyp == SHADOWLANDER_S_TORCH
				|| obj->otyp == TORCH || obj->otyp == SUNROD
				) {
				if (obj->lamplit)
					Strcat(buf, " (lit)");
			}
			break;
		case ARMOR_CLASS:
			if (obj->owornmask & W_ARMOR)
				Strcat(buf, (obj == uskin) ? " (embedded in your skin)" :
				" (being worn)");
			if (obj->lamplit)
				Strcat(buf, " (lit)");
			if (obj->oartifact == ART_CHROMATIC_DRAGON_SCALES){
				if (Is_dragon_mail(obj)) Sprintf(eos(buf), " (mail)");
				if (Is_dragon_scales(obj)) Sprintf(eos(buf), " (scales)");
			}
			if (obj->oartifact == ART_PRISMATIC_DRAGON_PLATE){
				if (GRAY_DRAGON_SCALE_MAIL == obj->otyp)
					Sprintf(eos(buf), " (gray)");
				if (SILVER_DRAGON_SCALE_MAIL == obj->otyp)
					Sprintf(eos(buf), " (silver)");
				if (SHIMMERING_DRAGON_SCALE_MAIL == obj->otyp)
					Sprintf(eos(buf), " (shimmering)");
				if (RED_DRAGON_SCALE_MAIL == obj->otyp)
					Sprintf(eos(buf), " (red)");
				if (WHITE_DRAGON_SCALE_MAIL == obj->otyp)
					Sprintf(eos(buf), " (white)");
				if (ORANGE_DRAGON_SCALE_MAIL == obj->otyp)
					Sprintf(eos(buf), " (orange)");
				if (BLACK_DRAGON_SCALE_MAIL == obj->otyp)
					Sprintf(eos(buf), " (black)");
				if (BLUE_DRAGON_SCALE_MAIL == obj->otyp)
					Sprintf(eos(buf), " (blue)");
				if (GREEN_DRAGON_SCALE_MAIL == obj->otyp)
					Sprintf(eos(buf), " (green)");
				if (YELLOW_DRAGON_SCALE_MAIL == obj->otyp)
					Sprintf(eos(buf), " (yellow)");
			}
			break;
		case TOOL_CLASS:
			if (obj->owornmask & (W_TOOL /* blindfold */
#ifdef STEED
				| W_SADDLE
#endif
				)) {
				Strcat(buf, " (being worn)");
				break;
			}
			if (obj->otyp == LEASH && obj->leashmon != 0) {
				Strcat(buf, " (in use)");
				break;
			}
			if (obj->otyp == CANDELABRUM_OF_INVOCATION) {
				char tmpbuf[BUFSZ];

				if (!obj->spe)
					Strcpy(tmpbuf, "no");
				else
					Sprintf(tmpbuf, "%d", obj->spe);
				Sprintf(eos(buf), " (%s candle%s%s)",
					tmpbuf, plur(obj->spe),
					!obj->lamplit ? " attached" : ", lit");

				break;
			}
			else if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP
				|| obj->otyp == BRASS_LANTERN || Is_candle(obj) || obj->otyp == SHADOWLANDER_S_TORCH
				|| obj->otyp == TORCH || obj->otyp == SUNROD
				) {
				if (obj->lamplit)
					Strcat(buf, " (lit)");
				break;
			}
			if (objects[obj->otyp].oc_charged && !is_weptool(obj))
				goto charges;
			break;
		case WAND_CLASS:
		charges:
			if (obj->known || Race_if(PM_ANDROID))
				Sprintf(eos(buf), " (%d:%d)", (int)obj->recharged, obj->spe);
			else if (obj->spe <= 0 && Race_if(PM_INCANTIFIER))
				Sprintf(eos(buf), " (empty)");
			break;
		case POTION_CLASS:
			if (obj->otyp == POT_OIL && obj->lamplit)
				Strcat(buf, " (lit)");
			break;
		case RING_CLASS:
		ring:
			if (obj->otyp == RIN_WISHES && obj->known) Sprintf(eos(buf), " (%d remaining)", obj->spe);
			if (obj->otyp == RIN_WISHES && !obj->known && obj->spe > 0) Sprintf(eos(buf), " with %d star%s", obj->spe, plur(obj->spe));
			if (obj->owornmask & W_RINGR) Strcat(buf, " (on right ");
			if (obj->owornmask & W_RINGL) Strcat(buf, " (on left ");
			if (obj->owornmask & W_RING) {
				Strcat(buf, body_part(HAND));
				if (isSignetRing(obj->otyp)){
					if (obj->opoisoned & OPOISON_BASIC) Strcat(buf, ", poison injecting");
					if (obj->opoisoned & OPOISON_FILTH) Strcat(buf, ", filth injecting");
					if (obj->opoisoned & OPOISON_SLEEP) Strcat(buf, ", drug injecting");
					if (obj->opoisoned & OPOISON_BLIND) Strcat(buf, ", eyebite injecting");
					if (obj->opoisoned & OPOISON_PARAL) Strcat(buf, ", venom injecting");
					if (obj->opoisoned & OPOISON_AMNES) Strcat(buf, ", lethe injecting");
					if (obj->opoisoned & OPOISON_ACID) Strcat(buf, ", acid injecting");
				}
				Strcat(buf, ")");
			}
			else if (isSignetRing(obj->otyp)){
				if (obj->opoisoned & OPOISON_BASIC) Strcat(buf, " (poison injecting)");
				if (obj->opoisoned & OPOISON_FILTH) Strcat(buf, " (filth injecting)");
				if (obj->opoisoned & OPOISON_SLEEP) Strcat(buf, " (drug injecting)");
				if (obj->opoisoned & OPOISON_BLIND) Strcat(buf, " (eyebite injecting)");
				if (obj->opoisoned & OPOISON_PARAL) Strcat(buf, " (venom injecting)");
				if (obj->opoisoned & OPOISON_AMNES) Strcat(buf, " (lethe injecting)");
				if (obj->opoisoned & OPOISON_ACID)  Strcat(buf, " (acid injecting)");
			}
			break;
		case FOOD_CLASS:
			if (obj->otyp == EGG) {
				if (obj->corpsenm >= LOW_PM &&
					(obj->known ||
					mvitals[obj->corpsenm].mvflags & MV_KNOWS_EGG)) {
					if (obj->spe)
						Strcat(buf, " (laid by you)");
				}
			}
			if (obj->otyp == MEAT_RING) goto ring;
			break;
		case BALL_CLASS:
		case CHAIN_CLASS:
			if (obj->owornmask & W_BALL)
				Strcat(buf, " (chained to you)");
			break;
		}//end switch(oclass)

		if ((obj->owornmask & W_WEP) && !mrg_to_wielded) {
			if (obj->quan != 1L) {
				Strcat(buf, " (wielded)");
			}
			else {
				const char *hand_s = body_part(HAND);

				if ((bimanual(obj, youracedata)
					&& !(u.twoweap && (obj->oartifact == ART_PROFANED_GREATSCYTHE || obj->oartifact == ART_LIFEHUNT_SCYTHE)))
					|| (u.twoweap && obj->otyp == STILETTOS))
					hand_s = makeplural(hand_s);
				Sprintf(eos(buf), " (weapon in %s)", hand_s);
			}
		}
		if (obj->owornmask & W_SWAPWEP) {
			if (mcarried(obj) || (u.twoweap && !(uwep && uwep->otyp == STILETTOS)))	// Monsters twoweaponing stilettos would give a bad message, but they can't even wield stilettos right now...
				Sprintf(eos(buf), " (wielded in other %s)",
				body_part(HAND));
			else
				Strcat(buf, " (alternate weapon; not wielded)");
		}
		if (obj->owornmask & W_QUIVER) Strcat(buf, " (in quiver)");
		if (obj->unpaid) {
			xchar ox, oy;
			long quotedprice = unpaid_cost(obj);
			struct monst *shkp = (struct monst *)0;

			if (Has_contents(obj) &&
				get_obj_location(obj, &ox, &oy, BURIED_TOO | CONTAINED_TOO) &&
				costly_spot(ox, oy) &&
				(shkp = shop_keeper(*in_rooms(ox, oy, SHOPBASE))))
				quotedprice += contained_cost(obj, shkp, 0L, FALSE, TRUE);
			Sprintf(eos(buf), " (unpaid, %ld %s)",
				quotedprice, currency(quotedprice));
		}
		else if (with_price) {
			long price = get_cost_of_shop_item(obj);
			if (price > 0) {
				Sprintf(eos(buf), " (%ld %s)", price, currency(price));
			}
		}
		// fix "a" -> "an"
		if (!strncmp(buf, "a ", 2) &&
			index(vowels, *(buf + 2)) &&
			(strncmp(buf + 2, "uranium", 7) && strncmp(buf + 2, "unicorn", 7) && strncmp(buf + 2, "eucalyptus", 10)))
		{
			buf = strprepend(buf + 2, "an ");
		}
	}
	return buf;
}


char *
xname2(obj, ignore_oquan)
register struct obj *obj;
boolean ignore_oquan;
{
	return doxname(obj, FALSE, ignore_oquan, FALSE);
}

/* xname() output augmented for multishot missile feedback */
char *
mshot_xname(obj)
struct obj *obj;
{
    char tmpbuf[BUFSZ];
    char *onm = xname(obj);

    if (m_shot.n > 1 && m_shot.o == obj->otyp) {
	/* copy xname's result so that we can reuse its return buffer */
	Strcpy(tmpbuf, onm);
	/* "the Nth arrow"; value will eventually be passed to an() or
	   The(), both of which correctly handle this "the " prefix */
	Sprintf(onm, "the %d%s %s", m_shot.i, ordin(m_shot.i), tmpbuf);
    }

    return onm;
}

#endif /* OVL1 */
#ifdef OVL0

/* used for naming "the unique_item" instead of "a unique_item" */
boolean
the_unique_obj(obj)
register struct obj *obj;
{
    if (!obj->dknown)
	return FALSE;
    else if (obj->otyp == FAKE_AMULET_OF_YENDOR && !obj->known)
	return TRUE;		/* lie */
    else
	return (boolean)(objects[obj->otyp].oc_unique &&
			 (obj->known || obj->otyp == AMULET_OF_YENDOR));
}

char *
doname_base(obj, with_price)
register struct obj *obj;
boolean with_price;
{
	return doxname(obj, TRUE, FALSE, with_price);
}

/** Wrapper function for vanilla behaviour. */
char *
doname(obj)
     register struct obj *obj;
{
    return doname_base(obj, FALSE);
}

/** Name of object including price. */
char *
doname_with_price(obj)
     register struct obj *obj;
{
    return doname_base(obj, iflags.show_shop_prices);
}


#endif /* OVL0 */
#ifdef OVLB

/* used from invent.c */
boolean
not_fully_identified(otmp)
register struct obj *otmp;
{
#ifdef GOLDOBJ
    /* gold doesn't have any interesting attributes [yet?] */
    if (otmp->oclass == COIN_CLASS) return FALSE;	/* always fully ID'd */
#endif
    /* check fundamental ID hallmarks first */
    if (!otmp->known || !otmp->dknown || !otmp->sknown || 
#ifdef MAIL
	    (!otmp->bknown && otmp->otyp != SCR_MAIL) ||
#else
	    !otmp->bknown ||
#endif
	    !objects[otmp->otyp].oc_name_known)	/* ?redundant? */
	return TRUE;
    if (otmp->oartifact && undiscovered_artifact(otmp->oartifact))
	return TRUE;
    /* otmp->rknown is the only item of interest if we reach here */
       /*
	*  Note:  if a revision ever allows scrolls to become fireproof or
	*  rings to become shockproof, this checking will need to be revised.
	*  `rknown' ID only matters if xname() will provide the info about it.
	*/
    if (otmp->rknown || (otmp->oclass != ARMOR_CLASS &&
			 otmp->oclass != WEAPON_CLASS &&
			 !is_weptool(otmp) &&		    /* (redunant) */
			 otmp->oclass != BALL_CLASS))	    /* (useless) */
	return FALSE;
    else	/* lack of `rknown' only matters for vulnerable objects */
	return (boolean)(is_rustprone(otmp) ||
			 is_corrodeable(otmp) ||
			 is_flammable(otmp));
}

char *
corpse_xname(otmp, ignore_oquan)
struct obj *otmp;
boolean ignore_oquan;	/* to force singular */
{
	char *nambuf = nextobuf();

	Sprintf(nambuf, "%s corpse", mons[otmp->corpsenm].mname);

	if (ignore_oquan || otmp->quan < 2)
	    return nambuf;
	else
	    return makeplural(nambuf);
}

/* xname, unless it's a corpse, then corpse_xname(obj, FALSE) */
char *
cxname(obj)
struct obj *obj;
{
	if (obj->otyp == CORPSE)
	    return corpse_xname(obj, FALSE);
	return xname(obj);
}
#ifdef SORTLOOT
char *
cxname2(obj)
struct obj *obj;
{
	if (obj->otyp == CORPSE)
	    return corpse_xname(obj, TRUE);
	return xname2(obj, TRUE);
}
#endif /* SORTLOOT */

/* treat an object as fully ID'd when it might be used as reason for death */
char *
killer_xname(obj)
struct obj *obj;
{
    struct obj save_obj;
    unsigned save_ocknown;
    char *buf, *save_ocuname;

    /* remember original settings for core of the object;
       oname and oattached extensions don't matter here--since they
       aren't modified they don't need to be saved and restored */
    save_obj = *obj;
    /* killer name should be more specific than general xname; however, exact
       info like blessed/cursed and rustproof makes things be too verbose */
    obj->known = obj->dknown = 1;
    obj->bknown = obj->rknown = obj->sknown = obj->greased = 0;
    /* if character is a priest[ess], bknown will get toggled back on */
    obj->blessed = obj->cursed = 0;
    /* "killed by poisoned <obj>" would be misleading when poison is
       not the cause of death and "poisoned by poisoned <obj>" would
       be redundant when it is, so suppress "poisoned" prefix */
    obj->opoisoned = 0;
    /* strip user-supplied name; artifacts keep theirs */
    if (!obj->oartifact) obj->onamelth = 0;
    /* temporarily identify the type of object */
    save_ocknown = objects[obj->otyp].oc_name_known;
    objects[obj->otyp].oc_name_known = 1;
    save_ocuname = objects[obj->otyp].oc_uname;
    objects[obj->otyp].oc_uname = 0;	/* avoid "foo called bar" */

    buf = xname(obj);
    if (obj->quan == 1L) buf = obj_is_pname(obj) ? the(buf) : an(buf);

    objects[obj->otyp].oc_name_known = save_ocknown;
    objects[obj->otyp].oc_uname = save_ocuname;
    *obj = save_obj;	/* restore object's core settings */

    return buf;
}

/*
 * Used if only one of a collection of objects is named (e.g. in eat.c).
 */
const char *
singular(otmp, func)
register struct obj *otmp;
char *FDECL((*func), (OBJ_P));
{
	long savequan;
	char *nam;

	/* Note: using xname for corpses will not give the monster type */
	if (otmp->otyp == CORPSE && func == xname)
		return corpse_xname(otmp, TRUE);

	savequan = otmp->quan;
	otmp->quan = 1L;
	nam = (*func)(otmp);
	otmp->quan = savequan;
	return nam;
}

char *
an(str)
register const char *str;
{
	char *buf = nextobuf();

	buf[0] = '\0';

	if (strncmpi(str, "the ", 4) &&
	    strcmp(str, "molten lava") &&
	    strcmp(str, "iron bars") &&
	    strcmp(str, "grass") &&
	    strcmp(str, "ice")) {
		if (index(vowels, *str) &&
		    strncmp(str, "one-", 4) &&
		    strncmp(str, "useful", 6) &&
		    strncmp(str, "unicorn", 7) &&
		    strncmp(str, "uranium", 7) &&
		    strncmp(str, "eucalyptus", 10))
			Strcpy(buf, "an ");
		else
			Strcpy(buf, "a ");
	}

	Strcat(buf, str);
	return buf;
}

char *
An(str)
const char *str;
{
	register char *tmp = an(str);
	*tmp = highc(*tmp);
	return tmp;
}

/*
 * Prepend "the" if necessary; assumes str is a subject derived from xname.
 * Use type_is_pname() for monster names, not the().  the() is idempotent.
 */
char *
the(str)
const char *str;
{
	char *buf = nextobuf();
	boolean insert_the = FALSE;

	if (!strncmpi(str, "the ", 4)) {
	    buf[0] = lowc(*str);
	    Strcpy(&buf[1], str+1);
	    return buf;
	} else if (*str < 'A' || *str > 'Z') {
	    /* not a proper name, needs an article */
	    insert_the = TRUE;
	} else {
	    /* Probably a proper name, might not need an article */
	    register char *tmp, *named, *called;
	    int l;

	    /* some objects have capitalized adjectives in their names */
	    if(((tmp = rindex(str, ' ')) || (tmp = rindex(str, '-'))) &&
	       (tmp[1] < 'A' || tmp[1] > 'Z'))
		insert_the = TRUE;
		else {
//	    else if (tmp && index(str, ' ') < tmp) {	/* has spaces */
//		/* it needs an article if the name contains "of" */
//		tmp = strstri(str, " of ");
//		named = strstri(str, " named ");
//		called = strstri(str, " called ");
//		if (called && (!named || called < named)) named = called;
//
//		if (tmp && (!named || tmp < named))	/* found an "of" */
//		    insert_the = TRUE;
//		/* stupid special case: lacks "of" but needs "the" */
//		else if (!named && (l = strlen(str)) >= 31 &&
//		      !strcmp(&str[l - 31], "Platinum Yendorian Express Card"))
//		    insert_the = TRUE;

			// This will catch all artifacts listed in artilist.h whose coded names begin with "The "
			int i;
			for (i = 1; i < NROFARTIFACTS && !insert_the; i++)
			{
				if (!strncmp(artilist[i].name, "The ", 4) &&
					((l = strlen(str)) >= strlen(artilist[i].name) - 4) &&
					!strcmp(&str[l - (strlen(artilist[i].name) - 4)], artilist[i].name + 4))
					insert_the = TRUE;
			}

			// This will have to catch any remaining items that should start with "the "
			// Notably, unique items fall in this category, as they are in Capital Case but are not in artilist.h
			if (!insert_the && (
				(strlen(str) >= 15 && (
				!strncmp(str, "Bell of Opening", 15)
				)) ||
				(strlen(str) >= 16 && (
				!strncmp(str, "Amulet of Yendor", 16) ||
				!strncmp(str, "Book of the Dead", 16)
				)) ||
				(strlen(str) >= 25 && (
				!strncmp(str, "Candelabrum of Invocation", 25)
				))
				))
				insert_the = TRUE;

	    }
	}
	if (insert_the)
	    Strcpy(buf, "the ");
	else
	    buf[0] = '\0';
	Strcat(buf, str);

	return buf;
}

char *
The(str)
const char *str;
{
    register char *tmp = the(str);
    *tmp = highc(*tmp);
    return tmp;
}

/* returns "count cxname(otmp)" or just cxname(otmp) if count == 1 */
char *
aobjnam(otmp,verb)
register struct obj *otmp;
register const char *verb;
{
	register char *bp = cxname(otmp);
	char prefix[PREFIX];

	if(otmp->quan != 1L) {
		Sprintf(prefix, "%ld ", otmp->quan);
		bp = strprepend(bp, prefix);
	}

	if(verb) {
	    Strcat(bp, " ");
	    Strcat(bp, otense(otmp, verb));
	}
	return(bp);
}

/* like aobjnam, but prepend "The", not count, and use xname */
char *
Tobjnam(otmp, verb)
register struct obj *otmp;
register const char *verb;
{
	char *bp = The(xname(otmp));

	if(verb) {
	    Strcat(bp, " ");
	    Strcat(bp, otense(otmp, verb));
	}
	return(bp);
}

/* return form of the verb (input plural) if xname(otmp) were the subject */
char *
otense(otmp, verb)
register struct obj *otmp;
register const char *verb;
{
	char *buf;

	/*
	 * verb is given in plural (without trailing s).  Return as input
	 * if the result of xname(otmp) would be plural.  Don't bother
	 * recomputing xname(otmp) at this time.
	 */
	if (!is_plural(otmp))
	    return vtense((char *)0, verb);

	buf = nextobuf();
	Strcpy(buf, verb);
	return buf;
}

/* various singular words that vtense would otherwise categorize as plural */
static const char * const special_subjs[] = {
	"erinys",
	"manes",		/* this one is ambiguous */
	"Cyclops",
	"Hippocrates",
	"Pelias",
	"lembas",
	"aklys",
	"amnesia",
	"paralysis",
	"dress",
	0
};

/* return form of the verb (input plural) for present tense 3rd person subj */
char *
vtense(subj, verb)
register const char *subj;
register const char *verb;
{
	char *buf = nextobuf();
	int len, ltmp;
	const char *sp, *spot;
	const char * const *spec;

	/*
	 * verb is given in plural (without trailing s).  Return as input
	 * if subj appears to be plural.  Add special cases as necessary.
	 * Many hard cases can already be handled by using otense() instead.
	 * If this gets much bigger, consider decomposing makeplural.
	 * Note: monster names are not expected here (except before corpse).
	 *
	 * special case: allow null sobj to get the singular 3rd person
	 * present tense form so we don't duplicate this code elsewhere.
	 */
	if(Role_if(PM_PIRATE) && !strcmp(verb,"are")) {
		Strcpy(buf,"be");
		return buf;
	}

	if (subj) {
	    if (!strncmpi(subj, "a ", 2) || !strncmpi(subj, "an ", 3))
		goto sing;
	    spot = (const char *)0;
	    for (sp = subj; (sp = index(sp, ' ')) != 0; ++sp) {
		if (!strncmp(sp, " of ", 4) ||
		    !strncmp(sp, " from ", 6) ||
		    !strncmp(sp, " called ", 8) ||
		    !strncmp(sp, " named ", 7) ||
		    !strncmp(sp, " labeled ", 9)) {
		    if (sp != subj) spot = sp - 1;
		    break;
		}
	    }
	    len = (int) strlen(subj);
	    if (!spot) spot = subj + len - 1;

	    /*
	     * plural: anything that ends in 's', but not '*us' or '*ss'.
	     * Guess at a few other special cases that makeplural creates.
	     */
	    if ((*spot == 's' && spot != subj &&
			(*(spot-1) != 'u' && *(spot-1) != 's') &&
			!((spot - subj) >= 5 && !strncmp(spot-4, "Chaos", 5))
		) ||
		((spot - subj) >= 4 && !strncmp(spot-3, "eeth", 4)) ||
		((spot - subj) >= 3 && !strncmp(spot-3, "feet", 4)) ||
		((spot - subj) >= 2 && !strncmp(spot-1, "ia", 2)) ||
		((spot - subj) >= 2 && !strncmp(spot-1, "ae", 2))) {
		/* check for special cases to avoid false matches */
		len = (int)(spot - subj) + 1;
		for (spec = special_subjs; *spec; spec++) {
		    ltmp = strlen(*spec);
		    if (len == ltmp && !strncmpi(*spec, subj, len)) goto sing;
		    /* also check for <prefix><space><special_subj>
		       to catch things like "the invisible erinys" */
		    if (len > ltmp && *(spot - ltmp) == ' ' &&
			   !strncmpi(*spec, spot - ltmp + 1, ltmp)) goto sing;
		}

		return strcpy(buf, verb);
	    }
	    /*
	     * 3rd person plural doesn't end in telltale 's';
	     * 2nd person singular behaves as if plural.
	     */
	    if (!strcmpi(subj, "they") || !strcmpi(subj, "you"))
		return strcpy(buf, verb);
	}

 sing:
	len = strlen(verb);
	spot = verb + len - 1;

	if (!strcmp(verb, "are"))
	    Strcpy(buf, "is");
	else if (!strcmp(verb, "have"))
	    Strcpy(buf, "has");
	else if (index("zxs", *spot) ||
		 (len >= 2 && *spot=='h' && index("cs", *(spot-1))) ||
		 (len == 2 && *spot == 'o')) {
	    /* Ends in z, x, s, ch, sh; add an "es" */
	    Strcpy(buf, verb);
	    Strcat(buf, "es");
	} else if (*spot == 'y' && (!index(vowels, *(spot-1)))) {
	    /* like "y" case in makeplural */
	    Strcpy(buf, verb);
	    Strcpy(buf + len - 1, "ies");
	} else {
	    Strcpy(buf, verb);
	    Strcat(buf, "s");
	}

	return buf;
}

/* capitalized variant of doname() */
char *
Doname2(obj)
register struct obj *obj;
{
	register char *s = doname(obj);

	*s = highc(*s);
	return(s);
}

/* returns "your xname(obj)" or "Foobar's xname(obj)" or "the xname(obj)" */
char *
yname(obj)
struct obj *obj;
{
	char *outbuf = nextobuf();
	char *s = shk_your(outbuf, obj);	/* assert( s == outbuf ); */
	int space_left = BUFSZ - strlen(s) - sizeof " ";

	return strncat(strcat(s, " "), cxname(obj), space_left);
}

/* capitalized variant of yname() */
char *
Yname2(obj)
struct obj *obj;
{
	char *s = yname(obj);

	*s = highc(*s);
	return s;
}

/* returns "your simple_typename(obj->otyp)"
 * or "Foobar's simple_typename(obj->otyp)"
 * or "the simple_typename(obj-otyp)"
 */
char *
ysimple_name(obj)
struct obj *obj;
{
	char *outbuf = nextobuf();
	char *s = shk_your(outbuf, obj);	/* assert( s == outbuf ); */
	int space_left = BUFSZ - strlen(s) - sizeof " ";

	return strncat(strcat(s, " "), simple_typename(obj->otyp), space_left);
}

/* capitalized variant of ysimple_name() */
char *
Ysimple_name2(obj)
struct obj *obj;
{
	char *s = ysimple_name(obj);

	*s = highc(*s);
	return s;
}

static const char *wrp[] = {
	"wand", "ring", "potion", "scroll", "shard", "gem", "amulet",
	"spellbook", "spell book",
	/* for non-specific wishes */
	"weapon", "armor", "armour", "tool", "food", "comestible",
};
static const char wrpsym[] = {
	WAND_CLASS, RING_CLASS, POTION_CLASS, SCROLL_CLASS, TILE_CLASS, GEM_CLASS,
	AMULET_CLASS, SPBOOK_CLASS, SPBOOK_CLASS,
	WEAPON_CLASS, ARMOR_CLASS, ARMOR_CLASS, TOOL_CLASS, FOOD_CLASS,
	FOOD_CLASS
};

#endif /* OVLB */
#ifdef OVL0

/* Plural routine; chiefly used for user-defined fruits.  We have to try to
 * account for everything reasonable the player has; something unreasonable
 * can still break the code.  However, it's still a lot more accurate than
 * "just add an s at the end", which Rogue uses...
 *
 * Also used for plural monster names ("Wiped out all homunculi.")
 * and body parts.
 *
 * Also misused by muse.c to convert 1st person present verbs to 2nd person.
 */
char *
makeplural(oldstr)
const char *oldstr;
{
	/* Note: cannot use strcmpi here -- it'd give MATZot, CAVEMeN,... */
	register char *spot;
	char *str = nextobuf();
	const char *excess = (char *)0;
	int len;

	while (*oldstr==' ') oldstr++;
	if (!oldstr || !*oldstr) {
		impossible("plural of null?");
		Strcpy(str, "s");
		return str;
	}
	Strcpy(str, oldstr);

	/*
	 * Skip changing "pair of" to "pairs of".  According to Webster, usual
	 * English usage is use pairs for humans, e.g. 3 pairs of dancers,
	 * and pair for objects and non-humans, e.g. 3 pair of boots.  We don't
	 * refer to pairs of humans in this game so just skip to the bottom.
	 */
	if (!strncmp(str, "pair of ", 8))
		goto bottom;

	/* Search for common compounds, ex. lump of royal jelly */
	for(spot=str; *spot; spot++) {
		if (!strncmp(spot, " of ", 4)
				|| !strncmp(spot, " labeled ", 9)
				|| !strncmp(spot, " called ", 8)
				|| !strncmp(spot, " named ", 7)
				|| !strcmp(spot, " above") /* lurkers above */
				|| !strncmp(spot, " versus ", 8)
				|| !strncmp(spot, " from ", 6)
				|| !strncmp(spot, " in ", 4)
				|| !strncmp(spot, " on ", 4)
				|| !strncmp(spot, " a la ", 6)
				|| !strncmp(spot, " with", 5)	/* " with "? */
				|| !strncmp(spot, " de ", 4)
				|| !strncmp(spot, " d'", 3)
				|| !strncmp(spot, " du ", 4)) {
			excess = oldstr + (int) (spot - str);
			*spot = 0;
			break;
		}
	}
	spot--;
	while (*spot==' ') spot--; /* Strip blanks from end */
	*(spot+1) = 0;
	/* Now spot is the last character of the string */

	len = strlen(str);

	/* Single letters */
	if (len==1 || !letter(*spot)) {
		Strcpy(spot+1, "'s");
		goto bottom;
	}

	/* Same singular and plural; mostly Japanese words except for "manes" */
	if ((len == 2 && !strcmp(str, "ya")) ||
	    (len >= 2 && !strcmp(spot-1, "ai")) || /* samurai, Uruk-hai */
	    (len >= 3 && !strcmp(spot-2, " ya")) ||
	    (len >= 4 && !strcmp(spot-3, "drow")) ||
	    (len >= 4 &&
	     (!strcmp(spot-3, "fish") || !strcmp(spot-3, "tuna") ||
	      !strcmp(spot-3, "deer") || !strcmp(spot-3, "yaki"))) ||
	    (len >= 5 && (!strcmp(spot-4, "sheep") ||
			!strcmp(spot-4, "ninja") ||
			!strcmp(spot-4, "ronin") ||
			!strcmp(spot-4, "shito") ||
			!strcmp(spot-7, "shuriken") ||
			!strcmp(spot-4, "tengu") ||
			!strcmp(spot-4, "manes"))) ||
	    (len >= 6 && !strcmp(spot-5, "ki-rin")) ||
	    (len >= 7 && !strcmp(spot-6, "gunyoki")))
		goto bottom;

	/* man/men ("Wiped out all cavemen.") */
	if (len >= 3 && !strcmp(spot-2, "man") &&
			(len<6 || strcmp(spot-5, "shaman")) &&
			(len<5 || strcmp(spot-4, "human"))) {
		*(spot-1) = 'e';
		goto bottom;
	}

	/* Keter (plural of sephirah is sephiroth) */
	if (len >= 8 && !strcmp(spot-7, "sephirah")) {
		Strcpy(spot-1, "oth");
		goto bottom;
	}

	/* Elves (plural of alfr is alfar) */
	if (len >= 4 && !strcmp(spot-3, "alfr")) {
		Strcpy(spot-3, "alfar");
		goto bottom;
	}

	/* tooth/teeth */
	if (len >= 5 && !strcmp(spot-4, "tooth")) {
		Strcpy(spot-3, "eeth");
		goto bottom;
	}

	/* knife/knives, etc... */
	if (!strcmp(spot-1, "fe")) {
		Strcpy(spot-1, "ves");
		goto bottom;
	} else if (*spot == 'f') {
		if (index("lr", *(spot-1)) || index(vowels, *(spot-1))) {
			Strcpy(spot, "ves");
			goto bottom;
		} else if (len >= 5 && !strncmp(spot-4, "staf", 4)) {
			Strcpy(spot-1, "ves");
			goto bottom;
		}
	}

	/* foot/feet (body part) */
	if (len >= 4 && !strcmp(spot-3, "foot")) {
		Strcpy(spot-2, "eet");
		goto bottom;
	}

	/* ium/ia (mycelia, baluchitheria) */
	if (len >= 3 && !strcmp(spot-2, "ium")) {
		*(spot--) = (char)0;
		*spot = 'a';
		goto bottom;
	}

	/* algae, larvae, hyphae (another fungus part) */
	if ((len >= 4 && !strcmp(spot-3, "alga")) ||
	    (len >= 5 &&
	     (!strcmp(spot-4, "hypha") || !strcmp(spot-4, "larva")))) {
		Strcpy(spot, "ae");
		goto bottom;
	}

	/* fungus/fungi, homunculus/homunculi, but buses, lotuses, wumpuses */
	if (len > 3 && !strcmp(spot-1, "us") &&
	    (len < 5 || (strcmp(spot-4, "lotus") &&
			 (len < 6 || strcmp(spot-5, "wumpus"))))) {
		*(spot--) = (char)0;
		*spot = 'i';
		goto bottom;
	}

	/* vortex/vortices */
	if (len >= 6 && !strcmp(spot-3, "rtex")) {
		Strcpy(spot-1, "ices");
		goto bottom;
	}

	/* djinni/djinn (note: also efreeti/efreet) */
	if (len >= 6 && !strcmp(spot-5, "djinni")) {
		*spot = (char)0;
		goto bottom;
	}

	/* mumak/mumakil */
	if (len >= 5 && !strcmp(spot-4, "mumak")) {
		Strcpy(spot+1, "il");
		goto bottom;
	}

	/* sis/ses (nemesis) */
	if (len >= 3 && !strcmp(spot-2, "sis")) {
		*(spot-1) = 'e';
		goto bottom;
	}

	/* erinys/erinyes */
	if (len >= 6 && !strcmp(spot-5, "erinys")) {
		Strcpy(spot, "es");
		goto bottom;
	}

	/* mouse/mice,louse/lice (not a monster, but possible in food names) */
	if (len >= 5 && !strcmp(spot-3, "ouse") && index("MmLl", *(spot-4))) {
		Strcpy(spot-3, "ice");
		goto bottom;
	}

	/* matzoh/matzot, possible food name */
	if (len >= 6 && (!strcmp(spot-5, "matzoh")
					|| !strcmp(spot-5, "matzah"))) {
		Strcpy(spot-1, "ot");
		goto bottom;
	}
	if (len >= 5 && (!strcmp(spot-4, "matzo")
					|| !strcmp(spot-5, "matza"))) {
		Strcpy(spot, "ot");
		goto bottom;
	}

	/* child/children (for wise guys who give their food funny names) */
	if (len >= 5 && !strcmp(spot-4, "child")) {
		Strcpy(spot, "dren");
		goto bottom;
	}

	if (len >= 5 && (!strcmp(spot-3, "dron"))) {
		Strcpy(spot-3, "dra");
		goto bottom;
	}
	
	/* note: -eau/-eaux (gateau, bordeau...) */
	/* note: ox/oxen, VAX/VAXen, goose/geese */

	/* Ends in z, x, s, ch, sh; add an "es" */
	if (index("zxs", *spot)
			|| (len >= 2 && *spot=='h' && index("cs", *(spot-1)))
	/* Kludge to get "tomatoes" and "potatoes" right */
			|| (len >= 4 && !strcmp(spot-2, "ato"))) {
		Strcpy(spot+1, "es");
		goto bottom;
	}

	/* Ends in y preceded by consonant (note: also "qu") change to "ies" */
	if (*spot == 'y' &&
	    (!index(vowels, *(spot-1)))) {
		Strcpy(spot, "ies");
		goto bottom;
	}

	/* Default: append an 's' */
	Strcpy(spot+1, "s");

bottom:	if (excess) Strcpy(eos(str), excess);
	return str;
}

#endif /* OVL0 */

struct o_range {
	const char *name, oclass;
	int  f_o_range, l_o_range;
};

#ifndef OVLB

STATIC_DCL const struct o_range o_ranges[];

#else /* OVLB */

/* wishable subranges of objects */
STATIC_OVL NEARDATA const struct o_range o_ranges[] = {
	{ "bag",	TOOL_CLASS,   SACK,	      BAG_OF_TRICKS },
	{ "lamp",	TOOL_CLASS,   OIL_LAMP,	      MAGIC_LAMP },
	{ "candle",	TOOL_CLASS,   TALLOW_CANDLE,  WAX_CANDLE },
	{ "horn",	TOOL_CLASS,   TOOLED_HORN,    HORN_OF_PLENTY },
	{ "shield",	ARMOR_CLASS,  BUCKLER,   SHIELD_OF_REFLECTION },
	{ "helm",	ARMOR_CLASS,  LEATHER_HELM, HELM_OF_TELEPATHY },
	{ "gauntlets",	ARMOR_CLASS,  GLOVES, GAUNTLETS_OF_DEXTERITY },
	{ "boots",	ARMOR_CLASS,  LOW_BOOTS,      FLYING_BOOTS },
	{ "shoes",	ARMOR_CLASS,  LOW_BOOTS,      SHOES },
	{ "cloak",	ARMOR_CLASS,  MUMMY_WRAPPING, CLOAK_OF_DISPLACEMENT },
#ifdef TOURIST
	{ "shirt",	ARMOR_CLASS,  HAWAIIAN_SHIRT, RUFFLED_SHIRT },
#endif
	{ "dragon scales",
			ARMOR_CLASS,  GRAY_DRAGON_SCALES, YELLOW_DRAGON_SCALES },
	{ "dragon scale shield",
			ARMOR_CLASS,  GRAY_DRAGON_SCALE_SHIELD, YELLOW_DRAGON_SCALE_SHIELD },
	{ "scale shield",
			ARMOR_CLASS,  KITE_SHIELD, KITE_SHIELD },
	{ "dragon scale mail",
			ARMOR_CLASS,  GRAY_DRAGON_SCALE_MAIL, YELLOW_DRAGON_SCALE_MAIL },
	{ "sword",	WEAPON_CLASS, SHORT_SWORD,    KATANA },
//#ifdef FIREARMS
	{ "firearm", 	WEAPON_CLASS, PISTOL, AUTO_SHOTGUN },
	{ "gun", 	WEAPON_CLASS, PISTOL, AUTO_SHOTGUN },
	{ "grenade", 	WEAPON_CLASS, FRAG_GRENADE, GAS_GRENADE },
//#endif
#ifdef WIZARD
	{ "venom",	VENOM_CLASS,  BLINDING_VENOM, ACID_VENOM },
#endif
	{ "gray stone",	GEM_CLASS,    LUCKSTONE,      FLINT },
	{ "grey stone",	GEM_CLASS,    LUCKSTONE,      FLINT },
};

#define BSTRCMP(base,ptr,string) ((ptr) < base || strcmp((ptr),string))
#define BSTRCMPI(base,ptr,string) ((ptr) < base || strcmpi((ptr),string))
#define BSTRNCMP(base,ptr,string,num) ((ptr)<base || strncmp((ptr),string,num))
#define BSTRNCMPI(base,ptr,string,num) ((ptr)<base||strncmpi((ptr),string,num))

/*
 * Singularize a string the user typed in; this helps reduce the complexity
 * of readobjnam, and is also used in pager.c to singularize the string
 * for which help is sought.
 */
char *
makesingular(oldstr)
const char *oldstr;
{
	register char *p, *bp;
	char *str = nextobuf();

	if (!oldstr || !*oldstr) {
		impossible("singular of null?");
		str[0] = 0;
		return str;
	}
	Strcpy(str, oldstr);
	bp = str;

	while (*bp == ' ') bp++;
	/* find "cloves of garlic", "worthless pieces of blue glass" */
	if ((p = strstri(bp, "s of ")) != 0) {
	    /* but don't singularize "gauntlets", "boots", "Eyes of the.." */
	    if (BSTRNCMPI(bp, p-3, "Eye", 3) &&
		BSTRNCMP(bp, p-4, "boot", 4) &&
		BSTRNCMP(bp, p-8, "gauntlet", 8) &&
		BSTRNCMPI(bp, p-11, "Steel Scale", 11) &&
		BSTRNCMP(bp, p-8, "Gauntlet", 8))
		while ((*p = *(p+1)) != 0) p++;
	    return bp;
	}

	/* remove -s or -es (boxes) or -ies (rubies) */
	p = eos(bp);
	if (p >= bp+1 && p[-1] == 's') {
		if (p >= bp+2 && p[-2] == 'e') {
			if (p >= bp+3 && p[-3] == 'i') {
				if(!BSTRCMP(bp, p-7, "cookies") ||
				   !BSTRCMP(bp, p-4, "pies"))
					goto mins;
				Strcpy(p-3, "y");
				return bp;
			}

			/* note: cloves / knives from clove / knife */
			if(!BSTRCMP(bp, p-6, "knives")) {
				Strcpy(p-3, "fe");
				return bp;
			}
			if(!BSTRCMP(bp, p-6, "staves")) {
				Strcpy(p-3, "ff");
				return bp;
			}
			if (!BSTRCMPI(bp, p-6, "leaves")) {
				Strcpy(p-3, "f");
				return bp;
			}
			if (!BSTRCMP(bp, p-8, "vortices")) {
				Strcpy(p-4, "ex");
				return bp;
			}

			/* note: nurses, axes but boxes */
			if (!BSTRCMP(bp, p-5, "boxes") ||
			    !BSTRCMP(bp, p-4, "ches")) {
				p[-2] = '\0';
				return bp;
			}

			if (!BSTRCMP(bp, p-6, "gloves") ||
			    !BSTRCMP(bp, p-6, "lenses") ||
			    !BSTRCMP(bp, p-5, "shoes") ||
				!BSTRCMPI(bp, p-13, "versus curses") ||
			    !BSTRCMP(bp, p-6, "scales") ||
				!BSTRCMP(bp, p-6, "wishes") ||	/* ring */
				!BSTRCMPI(bp, p-10, "Lost Names")) /* book */
				return bp;

		} else if (!BSTRCMPI(bp, p-5, "boots") ||
			   !BSTRCMPI(bp, p-9, "gauntlets") ||
			   !BSTRCMPI(bp, p-6, "tricks") ||
			   !BSTRCMPI(bp, p-9, "paralysis") ||
			   !BSTRCMPI(bp, p-5, "glass") ||
			   !BSTRCMPI(bp, p-5, "grass") ||
			   !BSTRCMP(bp, p-4, "ness") ||
			   !BSTRCMPI(bp, p-14, "shape changers") ||
			   !BSTRCMPI(bp, p-15, "detect monsters") ||
			   !BSTRCMPI(bp, p-5, "Chaos") ||
			   !BSTRCMPI(bp, p-13, "Wand of Orcus") || /* wand */
			   !BSTRCMPI(bp, p-12, "Gear-spirits") || /* crossbow*/
			   !BSTRCMPI(bp, p-10, "Rod of Dis") || /* mace */
			   !BSTRCMPI(bp, p-6, "Caress") || /* whip */
			   !BSTRCMPI(bp, p-7, "Proteus") || /* chest */
			   !BSTRCMPI(bp, p-11, "Aesculapius") || /* staff */
			   !BSTRCMPI(bp, p-7, "Orpheus") || /* lyre */
			   !BSTRCMPI(bp, p-7, "Annulus") || /* Ring */
			   !BSTRCMPI(bp, p-5, "Aegis") || 	/* shield */
			   !BSTRCMPI(bp, p-8, "Longinus") || 	/* spear */
			   !BSTRCMPI(bp, p-14, "Twelve Mirrors") || 	/* staff */
			   !BSTRCMPI(bp, p-13, "Water Flowers") || 	/* boots */
			   !BSTRCMPI(bp, p-14, "Dwarvish Lords") || /* axe */
			   !BSTRCMPI(bp, p-12, "Elvish Lords") || /* mace */
			   !BSTRCMPI(bp, p-11, "Seven Parts") || /* spear */
			   !BSTRCMPI(bp, p-10, "Lost Names") || /* book */
			   !BSTRCMPI(bp, p-15, "Infinite Spells") || /* book */
			   !BSTRCMPI(bp, p-10, "eucalyptus") ||
#ifdef WIZARD
			   !BSTRCMPI(bp, p-9, "iron bars") ||
#endif
			   !BSTRCMPI(bp, p-5, "aklys") ||
			   !BSTRCMPI(bp, p-6, "lembas") ||
			   !BSTRCMPI(bp, p-5, "dress") ||
			   !BSTRCMPI(bp, p-16, "descent of stars") ||
			   !BSTRCMPI(bp, p-13, "dragon scales") ||
			   !BSTRCMPI(bp, p-6, "fungus"))
				return bp;
	mins:
		p[-1] = '\0';

	} else {

		if(!BSTRCMP(bp, p-5, "teeth")) {
			Strcpy(p-5, "tooth");
			return bp;
		}

		if (!BSTRCMP(bp, p-5, "fungi")) {
			Strcpy(p-5, "fungus");
			return bp;
		}

		/* here we cannot find the plural suffix */
	}
	return bp;
}

/* compare user string against object name string using fuzzy matching */
static boolean
wishymatch(u_str, o_str, retry_inverted)
const char *u_str;	/* from user, so might be variant spelling */
const char *o_str;	/* from objects[], so is in canonical form */
boolean retry_inverted;	/* optional extra "of" handling */
{
	/* special case: wizards can wish for traps.  The object is "beartrap"
	 * and the trap is "bear trap", so to let wizards wish for both we
	 * must not fuzzymatch.
	 */
#ifdef WIZARD
	if (wizard && !strcmp(o_str, "beartrap"))
	    return !strncmpi(o_str, u_str, 8);
#endif

	/* ignore spaces & hyphens and upper/lower case when comparing */
	if (fuzzymatch(u_str, o_str, " -", TRUE)) return TRUE;

	if (retry_inverted) {
	    const char *u_of, *o_of;
	    char *p, buf[BUFSZ];

	    /* when just one of the strings is in the form "foo of bar",
	       convert it into "bar foo" and perform another comparison */
	    u_of = strstri(u_str, " of ");
	    o_of = strstri(o_str, " of ");
	    if (u_of && !o_of) {
		Strcpy(buf, u_of + 4);
		p = eos(strcat(buf, " "));
		while (u_str < u_of) *p++ = *u_str++;
		*p = '\0';
		return fuzzymatch(buf, o_str, " -", TRUE);
	    } else if (o_of && !u_of) {
		Strcpy(buf, o_of + 4);
		p = eos(strcat(buf, " "));
		while (o_str < o_of) *p++ = *o_str++;
		*p = '\0';
		return fuzzymatch(u_str, buf, " -", TRUE);
	    }
	}

	/* [note: if something like "elven speed boots" ever gets added, these
	   special cases should be changed to call wishymatch() recursively in
	   order to get the "of" inversion handling] */
	if (!strncmp(o_str, "dwarvish ", 9)) {
	    if (!strncmpi(u_str, "dwarven ", 8))
		return fuzzymatch(u_str + 8, o_str + 9, " -", TRUE);
	} else if (!strncmp(o_str, "elven ", 6)) {
	    if (!strncmpi(u_str, "elvish ", 7))
		return fuzzymatch(u_str + 7, o_str + 6, " -", TRUE);
	    else if (!strncmpi(u_str, "elfin ", 6))
		return fuzzymatch(u_str + 6, o_str + 6, " -", TRUE);
	} else if (!strcmp(o_str, "aluminum")) {
		/* this special case doesn't really fit anywhere else... */
		/* (note that " wand" will have been stripped off by now) */
	    if (!strcmpi(u_str, "aluminium"))
		return fuzzymatch(u_str + 9, o_str + 8, " -", TRUE);
	}

	return FALSE;
}

/* alternate spellings; if the difference is only the presence or
   absence of spaces and/or hyphens (such as "pickaxe" vs "pick axe"
   vs "pick-axe") then there is no need for inclusion in this list;
   likewise for ``"of" inversions'' ("boots of speed" vs "speed boots") */
struct alt_spellings {
	const char *sp;
	int ob;
} spellings[] = {
	{ "pickax", PICK_AXE },
	{ "whip", BULLWHIP },
	{ "saber", SABER },
	{ "sabre", SABER },
	{ "smooth shield", SHIELD_OF_REFLECTION },
	{ "grey dragon scale shield", GRAY_DRAGON_SCALE_SHIELD },
	{ "grey dragon scale mail", GRAY_DRAGON_SCALE_MAIL },
	{ "grey dragon scales", GRAY_DRAGON_SCALES },
	{ "enchant armour", SCR_ENCHANT_ARMOR },
	{ "destroy armour", SCR_DESTROY_ARMOR },
	{ "scroll of enchant armour", SCR_ENCHANT_ARMOR },
	{ "scroll of destroy armour", SCR_DESTROY_ARMOR },
	{ "leather armour", LEATHER_ARMOR },
	{ "leather gloves", GLOVES },
	{ "studded leather armour", STUDDED_LEATHER_ARMOR },
	{ "chain", CHAIN },
	{ "iron chain", CHAIN },
	{ "iron ball", HEAVY_IRON_BALL },
	{ "lantern", BRASS_LANTERN },
	{ "mattock", DWARVISH_MATTOCK },
	{ "amulet of poison resistance", AMULET_VERSUS_POISON },
	{ "stone", ROCK },
#ifdef TOURIST
	{ "camera", EXPENSIVE_CAMERA },
	{ "tee shirt", T_SHIRT },
#endif
	{ "lembas", LEMBAS_WAFER },
	{ "can", TIN },
	{ "can opener", TIN_OPENER },
	{ "kelp", KELP_FROND },
	{ "eucalyptus", EUCALYPTUS_LEAF },
	{ "grapple", GRAPPLING_HOOK },
//#ifdef FIREARMS
	{ "handgun", PISTOL },
	{ "hand-gun", PISTOL },
	{ "hand gun", PISTOL },
	{ "revolver", PISTOL },
	{ "bazooka", ROCKET_LAUNCHER },
	{ "hand grenade", FRAG_GRENADE },
	{ "dynamite", STICK_OF_DYNAMITE },
	{ "ampule", HYPOSPRAY_AMPULE },
//#endif
	{ "rum", POT_BOOZE },
	{ "sea biscuit", CRAM_RATION },
	{ "cutlass", SCIMITAR },
	{ "buccaneer's ditty bag", OILSKIN_SACK },
	{ "ditty bag", SACK },
	{ "foot locker", BOX },
	{ "belaying pin", CLUB },
	{ "ono", BATTLE_AXE },
	{ "ninja-to", BROADSWORD },
	{ "tanko", BRONZE_PLATE_MAIL },
	{ "jo", CLUB },
	{ "jade o-yoroi", CRYSTAL_PLATE_MAIL },
	{ "kunai", DAGGER },
	{ "bo-shuriken", DART },
	{ "dwarvish zaghnal", DWARVISH_MATTOCK },
	{ "nunchaku", FLAIL },
	{ "gunyoki", FOOD_RATION },
	{ "kote of fumbling", GAUNTLETS_OF_FUMBLING },
	{ "kote of power", GAUNTLETS_OF_POWER },
	{ "kamayari", GUISARME },
	{ "bisento", HALBERD },
	{ "kabuto", HELMET },
	{ "shito", KNIFE },
	{ "sugegasa", SEDGE_HAT },
	{ "jingasa", WAR_HAT },
	{ "uma-yari", LANCE },
	{ "yugake", GLOVES },
	{ "osaku", LOCK_PICK },
	{ "chokuto", LONG_SWORD },
	{ "o-yoroi", PLATE_MAIL },
	{ "sake", POT_BOOZE },
	{ "bo", QUARTERSTAFF },
	{ "wakizashi", SHORT_SWORD },
	{ "hira-shuriken", SHURIKEN },
	{ "yari", SPEAR },
	{ "dou-maru", SPLINT_MAIL },
	{ "magari yari", TRIDENT },
	{ "no-dachi", TWO_HANDED_SWORD },
	{ "dai tsuchi", WAR_HAMMER },
	{ "koto", WOODEN_HARP },
	{ "helmet", HELMET },
	{ "circlet", HELMET },
	{ "helm of brilliance", HELM_OF_BRILLIANCE },
	{ "crown of cognizance", HELM_OF_BRILLIANCE },
	{ "helm of opposite alignment", HELM_OF_OPPOSITE_ALIGNMENT },
	{ "tiara of treachery", HELM_OF_OPPOSITE_ALIGNMENT },
	{ "helm of telepathy", HELM_OF_TELEPATHY },
	{ "tiara of telepathy", HELM_OF_TELEPATHY },
	{ "helm of drain resistance", HELM_OF_DRAIN_RESISTANCE },
	{ "diadem of drain resistance", HELM_OF_DRAIN_RESISTANCE },
	{ "mirror shield", SHIELD_OF_REFLECTION },
	{ "black dress", BLACK_DRESS },
	{ "dress", BLACK_DRESS },
	{ "noble's dress", NOBLE_S_DRESS },
	{ "armored dress", NOBLE_S_DRESS },
	{ "armored black dress", NOBLE_S_DRESS },
	{ "droven dress", NOBLE_S_DRESS },
	{ "armored boots", ARMORED_BOOTS },
	{ "fossil dark", CHUNK_OF_FOSSIL_DARK },
	{ "aesh", SYLLABLE_OF_STRENGTH__AESH },
	{ "strength syllable", SYLLABLE_OF_STRENGTH__AESH },
	{ "krau", SYLLABLE_OF_POWER__KRAU },
	{ "power syllable", SYLLABLE_OF_POWER__KRAU },
	{ "hoon", SYLLABLE_OF_LIFE__HOON },
	{ "life syllable", SYLLABLE_OF_LIFE__HOON },
	{ "uur", SYLLABLE_OF_GRACE__UUR },
	{ "grace syllable", SYLLABLE_OF_GRACE__UUR },
	{ "naen", SYLLABLE_OF_THOUGHT__NAEN },
	{ "thought syllable", SYLLABLE_OF_THOUGHT__NAEN },
	{ "vaul", SYLLABLE_OF_SPIRIT__VAUL },
	{ "spirit syllable", SYLLABLE_OF_SPIRIT__VAUL },
	{ (const char *)0, 0 },
};

/*
 * Return something wished for.  Specifying a null pointer for
 * the user request string results in a random object.  Otherwise,
 * if asking explicitly for "nothing" (or "nil") return no_wish;
 * if not an object return &zeroobj; if an error (no matching object),
 * return null.
 * If from_user is false, we're reading from the wizkit, nothing was typed in.
 */
struct obj *
readobjnam(bp, wishreturn, wishflags)
register char *bp;
int *wishreturn;
int wishflags;
{
	register char *p;
	register int i;
	register struct obj *otmp;
	int cnt, spe, spesgn, typ, very, rechrg;
	int blessed, uncursed, iscursed, ispoisoned, isgreased, isdrained, stolen;
	int moonphase = -1, viperheads = -1, ampule = -1, mat = 0;
	int eroded, eroded2, eroded3, erodeproof;
#ifdef INVISIBLE_OBJECTS
	int isinvisible;
#endif
	boolean from_user = !(wishflags & WISH_QUIET);
	boolean wizwish = !!(wishflags & WISH_WIZARD);
	boolean allow_artifact = !!(wishflags & WISH_ARTALLOW);
	int halfeaten, halfdrained, mntmp, contents;
	int islit, unlabeled, ishistoric, isdiluted;
	struct fruit *f;
	int ftype = current_fruit;
	char fruitbuf[BUFSZ];
	/* Fruits may not mess up the ability to wish for real objects (since
	 * you can leave a fruit in a bones file and it will be added to
	 * another person's game), so they must be checked for last, after
	 * stripping all the possible prefixes and seeing if there's a real
	 * name in there.  So we have to save the full original name.  However,
	 * it's still possible to do things like "uncursed burnt Alaska",
	 * or worse yet, "2 burned 5 course meals", so we need to loop to
	 * strip off the prefixes again, this time stripping only the ones
	 * possible on food.
	 * We could get even more detailed so as to allow food names with
	 * prefixes that _are_ possible on food, so you could wish for
	 * "2 3 alarm chilis".  Currently this isn't allowed; options.c
	 * automatically sticks 'candied' in front of such names.
	 */

    short dummyshort;
	boolean heptagram = FALSE,
		gorgoneion = FALSE,
		acheron = FALSE,
		pentagram = FALSE,
		hexagram = FALSE,
		hamsa = FALSE,
		sign = FALSE,
		eye = FALSE,
		queen = FALSE,
		cartouche = FALSE,
		garuda = FALSE,
		toustefna = FALSE,
		dreprun = FALSE,
		veioistafur = FALSE,
		thjofastafur = FALSE,
		sizewished = FALSE;
	int objsize = youracedata->msize;
	long bodytype = 0L;
	long long int oproperties = 0L;
	char oclass;
	char *un, *dn, *actualn;
	const char *name=0;
	boolean isartifact = FALSE;
	
	cnt = spe = spesgn = typ = very = rechrg =
		blessed = uncursed = iscursed = stolen = 
		isdrained = halfdrained =
#ifdef INVISIBLE_OBJECTS
		isinvisible =
#endif
		ispoisoned = isgreased = eroded = eroded2 = eroded3 = erodeproof =
		halfeaten = islit = unlabeled = ishistoric = isdiluted = 0;
	mntmp = NON_PM;
#define UNDEFINED 0
#define EMPTY 1
#define SPINACH 2
	contents = UNDEFINED;
	oclass = 0;
	actualn = dn = un = 0;

	if (!bp) goto any;
	/* first, remove extra whitespace they may have typed */
	(void)mungspaces(bp);
	/* allow wishing for "nothing" to preserve wishless conduct...
	   [now requires "wand of nothing" if that's what was really wanted] */
	if (!strcmpi(bp, "nothing") || !strcmpi(bp, "nil") ||
		!strcmpi(bp, "none"))
	{
		*wishreturn = WISH_NOTHING;
		return &zeroobj;
	}
	/* save the [nearly] unmodified choice string */
	Strcpy(fruitbuf, bp);

	for(;;) {
		register int l;

		if (!bp || !*bp) goto any;
		if (!strncmpi(bp, "an ", l=3) ||
		    !strncmpi(bp, "a ", l=2)) {
			cnt = 1;
		} else if (!strncmpi(bp, "the ", l=4)) {
			;	/* just increment `bp' by `l' below */
		} else if (!cnt && digit(*bp) && strcmp(bp, "0")) {
			cnt = atoi(bp);
			while(digit(*bp)) bp++;
			while(*bp == ' ') bp++;
			l = 0;
		} else if (*bp == '+' || *bp == '-') {
			spesgn = (*bp++ == '+') ? 1 : -1;
			spe = atoi(bp);
			while(digit(*bp)) bp++;
			while(*bp == ' ') bp++;
			l = 0;
		} else if (!strncmpi(bp, "stolen ", l=7)) {
			stolen = 1;
		} else if(!strncmpi(bp, "heptagram ", l=10)){
			heptagram = TRUE;
		} else if(!strncmpi(bp, "gorgoneion ", l=10)){
			gorgoneion = TRUE;
		} else if(!strncmpi(bp, "circle of acheron ", l=18) ||
					!strncmpi(bp, "circle ", l=7) ||
					!strncmpi(bp, "acheron ", l=8)){
			acheron = TRUE;
		} else if(!strncmpi(bp, "pentagram ", l=10)){
			pentagram = TRUE;
		} else if(!strncmpi(bp, "hexagram ", l=9)){
			hexagram = TRUE;
		} else if(!strncmpi(bp, "hamsa mark ", l=11) ||
					!strncmpi(bp, "hamsa ", l=6)){
			hamsa = TRUE;
		} else if(!strncmpi(bp, "elder sign ", l=11)){
			sign = TRUE;
		} else if(!strncmpi(bp, "elder elemental eye ", l=20) ||
					!strncmpi(bp, "elder eye ", l=10) ||
					!strncmpi(bp, "elemental eye ", l=14)){
			eye = TRUE;
		} else if(!strncmpi(bp, "sign of the scion queen mother ", l=31) ||
					!strncmpi(bp, "scion queen mother ", l=19) ||
					!strncmpi(bp, "queen mother ", l=13) ||
					!strncmpi(bp, "mother ", l=7)){
			queen = TRUE;
		} else if(!strncmpi(bp, "cartouche of the cat lord ", l=26) ||
					!strncmpi(bp, "cat lord ", l=9) ||
					!strncmpi(bp, "cartouche ", l=10)){
			cartouche = TRUE;
		} else if(!strncmpi(bp, "wings of garuda ", l=16) ||
					!strncmpi(bp, "garuda ", l=7) ||
					!strncmpi(bp, "wings ", l=6)){
			garuda = TRUE;
		} else if(!strncmpi(bp, "toustefna ", l=10)){
			toustefna = TRUE;
		} else if(!strncmpi(bp, "dreprun ", l=8)){
			dreprun = TRUE;
		} else if(!strncmpi(bp, "veioistafur ", l=12)){
			veioistafur = TRUE;
		} else if(!strncmpi(bp, "thjofastafur ", l=13)){
			thjofastafur = TRUE;
		} else if(!strncmpi(bp, "engraved ", l=9)){
			/*This modifier does nothing, really, but people should be allowed to write it.*/;
		} else if(!strncmpi(bp, "carved ", l=7)){
			/*This modifier does nothing, really, but people should be allowed to write it.*/;
		} else if(!sizewished && !strncmpi(bp, "tiny ", l=5)){
			objsize = MZ_TINY;
			sizewished = TRUE;
		} else if(!sizewished && !strncmpi(bp, "small ", l=6)){
			objsize = MZ_SMALL;
			sizewished = TRUE;
		} else if(!sizewished && !strncmpi(bp, "medium ", l=7)){
			objsize = MZ_MEDIUM;
			sizewished = TRUE;
		} else if(!sizewished && !strncmpi(bp, "large ", l=6)){
			objsize = MZ_LARGE;
			sizewished = TRUE;
		} else if(!sizewished && !strncmpi(bp, "huge ", l=5)){
			objsize = MZ_HUGE;
			sizewished = TRUE;
		} else if(!sizewished && !strncmpi(bp, "gigantic ", l=9)){
			objsize = MZ_GIGANTIC;
			sizewished = TRUE;
		} else if(!strncmpi(bp, "humanoid ", l=9)){
			bodytype = MB_HUMANOID;
		} else if(!strncmpi(bp, "human ", l=6)){
			bodytype = MB_HUMANOID;
		} else if(!strncmpi(bp, "barded ", l=7)){
			bodytype = MB_ANIMAL|MB_LONGHEAD;
		} else if(!strncmpi(bp, "barding ", l=6)){
			bodytype = MB_ANIMAL|MB_LONGHEAD;
		} else if(!strncmpi(bp, "segmented ", l=10)){
			bodytype = MB_SLITHY;
		} else if(!strncmpi(bp, "tubular ", l=8)){
			bodytype = MB_SLITHY;
		} else if(!strncmpi(bp, "centaur ", l=8)){
			bodytype = (MB_HUMANOID|MB_ANIMAL);
		} else if(!strncmpi(bp, "snakeleg ", l=9)){
			bodytype = (MB_HUMANOID|MB_SLITHY);
		} else if(!strncmpi(bp, "snakeback ", l=10)){
			bodytype = (MB_ANIMAL|MB_SLITHY);
		} else if(!strncmpi(bp, "snakeneck ", l=10)){
			bodytype = MB_LONGNECK;
		} else if (!strncmpi(bp, "blessed ", l=8)){
			blessed = 1;
		} else if (!strncmpi(bp, "cursed ", l=7)){
			iscursed = 1;
		} else if (!strncmpi(bp, "uncursed ", l=9)) {
			uncursed = 1;
#ifdef INVISIBLE_OBJECTS
		} else if (!strncmpi(bp, "invisible ", l=10)) {
			isinvisible = 1;
#endif
		} else if (!strncmpi(bp, "rustproof ", l=10) ||
			   !strncmpi(bp, "erodeproof ", l=11) ||
			   !strncmpi(bp, "corrodeproof ", l=13) ||
			   !strncmpi(bp, "fixed ", l=6) ||
			   !strncmpi(bp, "fireproof ", l=10) ||
			   !strncmpi(bp, "rotproof ", l=9)) {
			erodeproof = 1;
		} else if (!strncmpi(bp,"lit ", l=4) ||
			   !strncmpi(bp,"burning ", l=8)) {
			islit = 1;
		} else if (!strncmpi(bp,"unlit ", l=6) ||
			   !strncmpi(bp,"extinguished ", l=13)) {
			islit = 0;
		/* "unlabeled" and "blank" are synonymous */
		} else if (!strncmpi(bp,"unlabeled ", l=10) ||
			   !strncmpi(bp,"unlabelled ", l=11) ||
			   !strncmpi(bp,"blank ", l=6)) {
			unlabeled = 1;
		} else if(!strncmpi(bp, "poisoned ",l=9) || (wizwish && !strncmpi(bp, "trapped ",l=8))
			) {
			ispoisoned=OPOISON_BASIC;
		} else if(!strncmpi(bp, "filth-crusted ",l=14) || !strncmpi(bp, "filthy ",l=7)) {
			ispoisoned=OPOISON_FILTH;
		} else if(!strncmpi(bp, "drug-coated ",l=12) || !strncmpi(bp, "drugged ",l=8)) {
			ispoisoned=OPOISON_SLEEP;
		} else if(!strncmpi(bp, "stained ",l=8)) {
			ispoisoned=OPOISON_BLIND;
		} else if(!strncmpi(bp, "envenomed ",l=10)) {
			ispoisoned=OPOISON_PARAL;
		} else if(!strncmpi(bp, "lethe-rusted ",l=13)) {
			ispoisoned=OPOISON_AMNES;
		} else if(!strncmpi(bp, "acid-coated ",l=12)) {
			ispoisoned=OPOISON_ACID;
		} else if(!strncmpi(bp, "greased ",l=8)) {
			isgreased=1;
		} else if (!strncmpi(bp, "very ", l=5)) {
			/* very rusted very heavy iron ball */
			very = 1;
		} else if (!strncmpi(bp, "thoroughly ", l=11)) {
			very = 2;
		} else if (!strncmpi(bp, "rusty ", l=6) ||
			   !strncmpi(bp, "rusted ", l=7) ||
			   !strncmpi(bp, "burnt ", l=6) ||
			   !strncmpi(bp, "burned ", l=7)) {
			eroded = 1 + very;
			very = 0;
		} else if (!strncmpi(bp, "corroded ", l=9) ||
			   !strncmpi(bp, "rotted ", l=7)) {
			eroded2 = 1 + very;
			very = 0;
		} else if (!strncmpi(bp, "tattered ", l=9)) {
			eroded3 = 1 + very;
			very = 0;
		} else if (!strncmpi(bp, "partly drained ", l=15)) {
			isdrained = 1;
			halfdrained = 1;
		} else if (!strncmpi(bp, "drained ", l=8)) {
			isdrained = 1;
			halfdrained = 0;
		} else if (!strncmpi(bp, "partly eaten ", l=13)) {
			halfeaten = 1;
		} else if (!strncmpi(bp, "historic ", l=9)) {
			ishistoric = 1;
		} else if (!strncmpi(bp, "diluted ", l=8)) {
			isdiluted = 1;
		} else if(!strncmpi(bp, "empty ", l=6)) {
			contents = EMPTY;
		} else if (!strncmpi(bp, "1-headed ", l=9) || !strncmpi(bp, "one-headed ", l=11)) {
			viperheads = 1;
		} else if (!strncmpi(bp, "2-headed ", l=9) || !strncmpi(bp, "two-headed ", l=11)) {
			viperheads = 2;
		} else if (!strncmpi(bp, "3-headed ", l=9) || !strncmpi(bp, "three-headed ", l=13)) {
			viperheads = 3;
		} else if (!strncmpi(bp, "4-headed ", l=9) || !strncmpi(bp, "four-headed ", l=12)) {
			viperheads = 4;
		} else if (!strncmpi(bp, "5-headed ", l=9) || !strncmpi(bp, "five-headed ", l=12)) {
			viperheads = 5;
		} else if (!strncmpi(bp, "6-headed ", l=9) || !strncmpi(bp, "six-headed ", l=11)) {
			viperheads = 6;
		} else if (!strncmpi(bp, "7-headed ", l=9) || !strncmpi(bp, "seven-headed ", l=13)) {
			viperheads = 7;
		} else if (!strncmpi(bp, "8-headed ", l=9) || !strncmpi(bp, "eight-headed ", l=13)) {
			viperheads = 8;
		} else if (!strncmpi(bp, "eclipse ", l=8)) {
			moonphase = ECLIPSE_MOON;
		} else if (!strncmpi(bp, "crescent ", l=9)) {
			moonphase = CRESCENT_MOON;
		} else if (!strncmpi(bp, "half ", l=5)) {
			moonphase = HALF_MOON;
		} else if (!strncmpi(bp, "gibbous ", l=8)) {
			moonphase = GIBBOUS_MOON;
		} else if (!strncmpi(bp, "full ", l=5) && strncmpi(bp, "full healing", 12)) {
			moonphase = FULL_MOON;
		} else if (!strncmpi(bp, "gain ability ", l=13) && strstri(bp, " ampule")) {
			ampule = POT_GAIN_ABILITY;
		} else if (!strncmpi(bp, "restore ability ", l=16) && strstri(bp, " ampule")) {
			ampule = POT_RESTORE_ABILITY;
		} else if (!strncmpi(bp, "blindness ", l=10) && strstri(bp, " ampule")) {
			ampule = POT_BLINDNESS;
		} else if (!strncmpi(bp, "confusion ", l=10) && strstri(bp, " ampule")) {
			ampule = POT_CONFUSION;
		} else if (!strncmpi(bp, "paralysis ", l=10) && strstri(bp, " ampule")) {
			ampule = POT_PARALYSIS;
		} else if (!strncmpi(bp, "speed ", l=6) && strstri(bp, " ampule")) {
			ampule = POT_SPEED;
		} else if (!strncmpi(bp, "hallucination ", l=14) && strstri(bp, " ampule")) {
			ampule = POT_HALLUCINATION;
		} else if (!strncmpi(bp, "healing ", l=8) && strstri(bp, " ampule")) {
			ampule = POT_HEALING;
		} else if (!strncmpi(bp, "extra healing ", l=14) && strstri(bp, " ampule")) {
			ampule = POT_EXTRA_HEALING;
		} else if (!strncmpi(bp, "full healing ", l=13) && strstri(bp, " ampule")) {
			ampule = POT_FULL_HEALING;
		} else if (!strncmpi(bp, "gain energy ", l=11) && strstri(bp, " ampule")) {
			ampule = POT_GAIN_ENERGY;
		} else if (!strncmpi(bp, "sleeping ", l=9) && strstri(bp, " ampule")) {
			ampule = POT_SLEEPING;
		} else if (!strncmpi(bp, "polymorph ", l=10) && strstri(bp, " ampule")) {
			ampule = POT_POLYMORPH;
		} else if (!strncmpi(bp, "amnesia ", l=8) && strstri(bp, " ampule")) {
			ampule = POT_AMNESIA;
		} else if (!strncmpi(bp, "wax ", l=4) && strncmpi(bp, "wax candle", 10)
			) {
			mat = WAX;
		} else if ((!strncmpi(bp, "veggy ", l=6) || !strncmpi(bp, "organic ", l=8))
			) {
			mat = VEGGY;
		} else if (!strncmpi(bp, "flesh ", l=6) && strncmpi(bp, "flesh golem", 11)
			) {
			mat = FLESH;
		} else if (!strncmpi(bp, "paper ", l=6) && strncmpi(bp, "paper golem", 11)
			) {
			mat = PAPER;
		} else if (!strncmpi(bp, "cloth ", l=6) && strncmpi(bp, "cloth spellbook", 15)
			) {
			mat = CLOTH;
		} else if (!strncmpi(bp, "leather ", l=8) && strncmpi(bp, "leather spellbook", 17)
			&& strncmpi(bp, "leather armor", 13) && strncmpi(bp, "leather gloves", 14)
			&& strncmpi(bp, "leather jacket", 14) && strncmpi(bp, "leather armor", 13)
			&& strncmpi(bp, "leather helm", 12) && strncmpi(bp, "leather hat", 11)
			&& strncmpi(bp, "leather cloak", 13) && strncmpi(bp, "leather drum", 12)
			) {
			mat = LEATHER;
		} else if ((!strncmpi(bp, "wood ", l=5) || !strncmpi(bp, "wooden ", 7))
			&& strncmpi(bp, "wooden ring", 12) && strncmpi(bp, "wooden flute", 13)
			&& strncmpi(bp, "wooden harp", 12) && strncmpi(bp, "wood golem", 11)
			) {
			mat = WOOD;
		} else if ((!strncmpi(bp, "dragonhide ", l=11) || !strncmpi(bp, "dragon-hide ", l=12) || !strncmpi(bp, "dragon hide ", l=12)
			|| !strncmpi(bp, "dragonscale ", l=12) || !strncmpi(bp, "dragon-scale ", l=13) || !strncmpi(bp, "dragon scale ", l=13)
			|| !strncmpi(bp, "dragonbone ", l=11) || !strncmpi(bp, "dragon-bone ", l=12) || !strncmpi(bp, "dragon bone ", l=12)
			|| !strncmpi(bp, "dragontooth ", l=12) || !strncmpi(bp, "dragon-tooth ", l=13) || !strncmpi(bp, "dragon tooth ", l=13))
			&& strncmpi(bp, "dragon scale mail", 17) && strncmpi(bp, "dragon-bone wand", 16)
			) {
			mat = DRAGON_HIDE;
		} else if (!strncmpi(bp, "iron ", l=5) && strncmpi(bp, "iron skull cap", 14)
			&& strncmpi(bp, "iron shoes", 10) && strncmpi(bp, "iron golem", 10)
			&& strncmpi(bp, "iron ring", 9) && strncmpi(bp, "iron hook", 9) && strncmpi(bp, "iron wand", 9)
			&& strncmpi(bp, "iron wand", 9) && strncmpi(bp, "iron bands", 10)
			&& strncmpi(bp, "Iron Ball of Levitation", 23) && strncmpi(bp, "Iron Spoon of Liberation", 24)
			) {
			mat = IRON;
		} else if ((!strncmpi(bp, "metal ", l=6) || !strncmpi(bp, "metallic ", l=9))
			&& strncmpi(bp, "metal tube", 10) && strncmpi(bp, "metal gauntlets", 15)
			&& strncmpi(bp, "metal tube", 10)
			) {
			mat = METAL;
		} else if (!strncmpi(bp, "bronze ", l=7)
			&& strncmpi(bp, "bronze helm", 11) && strncmpi(bp, "bronze plate mail", 17)
			&& strncmpi(bp, "bronze roundshield", 18) && strncmpi(bp, "bronze gauntlets", 16)
			&& strncmpi(bp, "bronze ring", 11)
			&& strncmpi(bp, "bronze spellbook", 16)
		) {
			mat = COPPER;
		} else if (!strncmpi(bp, "silver ", l=7)
			&& strncmpi(bp, "silver arrow", 12) && strncmpi(bp, "silver bullet", 13)
			&& strncmpi(bp, "silver pellet", 13) && strncmpi(bp, "silver dragon", 13)
			&& strncmpi(bp, "silver clothes", 14) && strncmpi(bp, "silver ring", 11)
			&& strncmpi(bp, "silver bell", 11) && strncmpi(bp, "silver spellbook", 16)
			&& strncmpi(bp, "silver wand", 11) && strncmpi(bp, "silver slingstone", 17)
			&& strncmpi(bp, "silver stone", 12) && strncmpi(bp, "Silver Key", 10)
			&& strncmpi(bp, "Silver Starlight", 16)
		) {
			mat = SILVER;
		} else if ((!strncmpi(bp, "golden ", l=7) || !strncmpi(bp, "gold ", l=5))
			&& strncmpi(bp, "golden arrow", 12) && strncmpi(bp, "gold ring", 9)
			&& strncmpi(bp, "golden slingstone", 17) && strncmpi(bp, "gold detection", 14)
			&& strncmpi(bp, "golden scroll", 13) && strncmpi(bp, "Gold Scroll of Law", 18)
			&& strncmpi(bp, "gold wand", 9) && strncmpi(bp, "gold piece", 10)
			&& strncmpi(bp, "gold coin", 9) && strncmpi(bp, "Golden Sword of Y'ha-Talla", 26)
			&& strncmpi(bp, "gold golem", 10)
		) {
			mat = GOLD;
		} else if (!strncmpi(bp, "platinum ", l=9)
			&& strncmpi(bp, "platinum wand", 13) && strncmpi(bp, "Platinum Yendorian", 18)
			&& strncmpi(bp, "Platinum Dragon", 15) && strncmpi(bp, "Platinum Dragon Plate", 21)
		) {
			mat = PLATINUM;
		} else if (!strncmpi(bp, "mithril ", l=8)) {
			mat = MITHRIL;
		} else if (!strncmpi(bp, "plastic ", l=8)
			) {
			mat = PLASTIC;
		} else if (!strncmpi(bp, "glass ", l=6)
			&& strncmpi(bp, "glass shield", 12) && strncmpi(bp, "glass gauntlets", 15) 
			&& strncmpi(bp, "glass boots", 11) && strncmpi(bp, "glass orb", 9)  
			&& strncmpi(bp, "glass golem", 11)  
			) {
			mat = GLASS;
		} else if ((!strncmpi(bp, "gemstone ", l=9) || !strncmpi(bp, "gem ", l=4))
			) {
			mat = GEMSTONE;
		} else if ((!strncmpi(bp, "stone ", l=6) || !strncmpi(bp, "ceramic ", l=8))
			) {
			mat = MINERAL;
		} else if (!strncmpi(bp, "obsidian ", l=9)
			&& strncmpi(bp, "obsidian stone", 14) && strncmpi(bp, "obsidian gem", 12)
			) {
			mat = OBSIDIAN_MT;
		} else if (!strncmpi(bp, "woolen ", l=7) || !strncmpi(bp, "wool-lined ", l=11)
			) {
			oproperties |= OPROP_WOOL;
		} else if (!strncmpi(bp, "lesser ", l=7)
			) {
			oproperties |= OPROP_LESSW;
		} else if (!strncmpi(bp, "flaming ", l=8)
			) {
			oproperties |= OPROP_FIREW;
		} else if (!strncmpi(bp, "freezing ", l=9)
			) {
			oproperties |= OPROP_COLDW;
		} else if (!strncmpi(bp, "reflective ", l=11)
			) {
			oproperties |= OPROP_REFL;
		} else if (!strncmpi(bp, "misty ", l=6)
			) {
			oproperties |= OPROP_WATRW;
		} else if (!strncmpi(bp, "faded ", l=6)
			) {
			oproperties |= OPROP_PHSEW;
		} else if (!strncmpi(bp, "psionic ", l=8) || !strncmpi(bp, "whispering ", l=11)
			) {
			oproperties |= OPROP_PSIOW;
		} else if (!strncmpi(bp, "deep ", l=5) || !strncmpi(bp, "mumbling ", l=9)
			) {
			oproperties |= OPROP_DEEPW;
		} else if (!strncmpi(bp, "shocking ", l=9)
			) {
			oproperties |= OPROP_ELECW;
		} else if (!strncmpi(bp, "sizzling ", l=9)
			) {
			oproperties |= OPROP_ACIDW;
		} else if (!strncmpi(bp, "sparkling ", l=10) && strncmpi(bp, "sparkling potion", 16)
			) {
			oproperties |= OPROP_MAGCW;
		} else if (!strncmpi(bp, "anarchic ", l=9)
			) {
			oproperties |= OPROP_ANARW;
		} else if (!strncmpi(bp, "concordant ", l=11)
			) {
			oproperties |= OPROP_CONCW;
		} else if (!strncmpi(bp, "axiomatic ", l=10)
			) {
			oproperties |= OPROP_AXIOW;
		} else if (!strncmpi(bp, "holy ", l=5) && strncmpi(bp, "holy moonlight sword", 20)
			) {
			blessed = !(uncursed + iscursed);
			oproperties |= OPROP_HOLYW;
		} else if (!strncmpi(bp, "unholy ", l=7)
			) {
			iscursed = !(uncursed + blessed);
			oproperties |= OPROP_UNHYW;
		} else if (!strncmpi(bp, "vorpal ", l=7) && strncmpi(bp, "Vorpal Blade", 12)
			) {
			oproperties |= OPROP_VORPW;
		} else if (!strncmpi(bp, "morgul ", l=7)
			) {
			oproperties |= OPROP_MORGW;
		} else if (!strncmpi(bp, "flaying ", l=8)
			) {
			oproperties |= OPROP_FLAYW;
		} else
			break;
		bp += l;
	}
	if(!cnt) cnt = 1;		/* %% what with "gems" etc. ? */
	if (strlen(bp) > 1) {
	    if ((p = rindex(bp, '(')) != 0) {
		if (p > bp && p[-1] == ' ') p[-1] = 0;
		else *p = 0;
		p++;
		if (!strcmpi(p, "lit)")) {
		    islit = 1;
		} else {
		    spe = atoi(p);
		    while (digit(*p)) p++;
		    if (*p == ':') {
			p++;
			rechrg = spe;
			spe = atoi(p);
			while (digit(*p)) p++;
		    }
		    if (*p != ')') {
			spe = rechrg = 0;
		    } else {
			spesgn = 1;
			p++;
			if (*p) Strcat(bp, p);
		    }
		}
	    }
	}
/*
   otmp->spe is type schar; so we don't want spe to be any bigger or smaller.
   also, spe should always be positive  -- some cheaters may try to confuse
   atoi()
*/
	if (spe < 0) {
		spesgn = -1;	/* cheaters get what they deserve */
		spe = abs(spe);
	}
	if (spe > SCHAR_LIM)
		spe = SCHAR_LIM;
	if (rechrg < 0 || rechrg > 7) rechrg = 7;	/* recharge_limit */

	/* now we have the actual name, as delivered by xname, say
		green potions called whisky
		scrolls labeled "QWERTY"
		egg
		fortune cookies
		very heavy iron ball named hoei
		wand of wishing
		elven cloak
	*/
	if ((p = strstri(bp, " named ")) != 0) {
		*p = 0;
		name = p+7;
	}
	if ((p = strstri(bp, " called ")) != 0) {
		*p = 0;
		un = p+8;
		/* "helmet called telepathy" is not "helmet" (a specific type)
		 * "shield called reflection" is not "shield" (a general type)
		 */
		for(i = 0; i < SIZE(o_ranges); i++)
		    if(!strcmpi(bp, o_ranges[i].name)) {
			oclass = o_ranges[i].oclass;
			goto srch;
		    }
	}
	if ((p = strstri(bp, " labeled ")) != 0) {
		*p = 0;
		dn = p+9;
	} else if ((p = strstri(bp, " labelled ")) != 0) {
		*p = 0;
		dn = p+10;
	}
	if ((p = strstri(bp, " of spinach")) != 0) {
		*p = 0;
		contents = SPINACH;
	}

	/*
	Skip over "pair of ", "pairs of", "set of" and "sets of".

	Accept "3 pair of boots" as well as "3 pairs of boots". It is valid
	English either way.  See makeplural() for more on pair/pairs.

	We should only double count if the object in question is not
	refered to as a "pair of".  E.g. We should double if the player
	types "pair of spears", but not if the player types "pair of
	lenses".  Luckily (?) all objects that are refered to as pairs
	-- boots, gloves, and lenses -- are also not mergable, so cnt is
	ignored anyway.
	*/
	if(!strncmpi(bp, "pair of ",8)) {
		bp += 8;
		cnt *= 2;
	} else if(cnt > 1 && !strncmpi(bp, "pairs of ",9)) {
		bp += 9;
		cnt *= 2;
	} else if (!strncmpi(bp, "set of ",7)) {
		bp += 7;
	} else if (!strncmpi(bp, "sets of ",8)) {
		bp += 8;
	}

	/*
	 * Find corpse type using "of" (figurine of an orc, tin of orc meat)
	 * Don't check if it's a wand or spellbook.
	 * (avoid "wand/finger of death" confusion).
	 */
	if (!strstri(bp, "wand ")
	 && !strstri(bp, "spellbook ")
	 && !strstri(bp, "book ")
	 && !strstri(bp, "rod ")
	 && !strstri(bp, "finger ")) {
	    if ((p = strstri(bp, " of ")) != 0
		&& (mntmp = name_to_mon(p+4)) >= LOW_PM)
		*p = 0;
	}
	/* Find corpse type w/o "of" (red dragon scale mail, yeti corpse) */
	if (strncmpi(bp, "samurai sword", 13)) /* not the "samurai" monster! */
	if (strncmpi(bp, "wizard lock", 11)) /* not the "wizard" monster! */
	if (strncmpi(bp, "vampire killer", 14)) /* not the "vampire" monster! */
	if (strncmpi(bp, "ninja-to", 8)) /* not the "ninja" rank */
	if (strncmpi(bp, "rogue gear-spirits", 18)) /* not the "rogue" monster */
	if (strncmpi(bp, "master key", 10)) /* not the "master" rank */
	if (strncmpi(bp, "scroll of stinking cloud", 10)) /* not the "stinking cloud" monster */
	if (strncmpi(bp, "rod of lordly might", 19)) /* not the "lord" rank */
	if (strncmpi(bp, "magenta", 7)) /* not the "mage" rank */
	if (strncmpi(bp, "chromatic dragon scales", 23)) /* not a "dragon" */
	if (strncmpi(bp, "platinum dragon plate", 22)) /* not a "dragon" */
	if (mntmp < LOW_PM && strlen(bp) > 2 &&
	    (mntmp = name_to_mon(bp)) >= LOW_PM) {
		int mntmptoo, mntmplen;	/* double check for rank title */
		char *obp = bp;
		mntmptoo = title_to_mon(bp, (int *)0, &mntmplen);
		bp += mntmp != mntmptoo ? (int)strlen(mons[mntmp].mname) : mntmplen;
		if (*bp == ' ') bp++;
		else if (!strncmpi(bp, "s ", 2)) bp += 2;
		else if (!strncmpi(bp, "es ", 3)) bp += 3;
		else if (!*bp && !actualn && !dn && !un && !oclass) {
		    /* no referent; they don't really mean a monster type */
		    bp = obp;
		    mntmp = NON_PM;
		}
	}

	/* first change to singular if necessary */
	if (*bp) {
		char *sng = makesingular(bp);
		if (strcmp(bp, sng)) {
			if (cnt == 1) cnt = 2;
			Strcpy(bp, sng);
		}
	}
	
	/* Alternate spellings (pick-ax, silver sabre, &c) */
    {
	struct alt_spellings *as = spellings;

	while (as->sp) {
		if (fuzzymatch(bp, as->sp, " -", TRUE)) {
			typ = as->ob;
			goto typfnd;
		}
		as++;
	}
	/* can't use spellings list for this one due to shuffling */
	if (!strncmpi(bp, "grey spell", 10))
		*(bp + 2) = 'a';
    }

	/* dragon scales - assumes order of dragons */
	if(!strcmpi(bp, "scales") &&
			mntmp >= PM_GRAY_DRAGON && mntmp <= PM_YELLOW_DRAGON) {
		typ = GRAY_DRAGON_SCALES + mntmp - PM_GRAY_DRAGON;
		mntmp = NON_PM;	/* no monster */
		goto typfnd;
	}

	p = eos(bp);
	if(!BSTRCMPI(bp, p-10, "holy water")) {
		typ = POT_WATER;
		if ((p-bp) >= 12 && *(p-12) == 'u')
			iscursed = 1; /* unholy water */
		else blessed = 1;
		goto typfnd;
	}
	if(unlabeled && !BSTRCMPI(bp, p-6, "scroll")) {
		typ = SCR_BLANK_PAPER;
		goto typfnd;
	}
	if(unlabeled && !BSTRCMPI(bp, p-9, "spellbook")) {
		typ = SPE_BLANK_PAPER;
		goto typfnd;
	}
	/*
	 * NOTE: Gold pieces are handled as objects nowadays, and therefore
	 * this section should probably be reconsidered as well as the entire
	 * gold/money concept.  Maybe we want to add other monetary units as
	 * well in the future. (TH)
	 */
	if(!BSTRCMPI(bp, p-10, "gold piece") || !BSTRCMPI(bp, p-7, "zorkmid") ||
	   !strcmpi(bp, "gold") || !strcmpi(bp, "money") ||
	   !strcmpi(bp, "coin") || *bp == GOLD_SYM) {
			if (cnt > 5000 && !wizwish) cnt=5000;
		if (cnt < 1) cnt=1;
#ifndef GOLDOBJ
		if (from_user)
		    pline("%d gold piece%s.", cnt, plur(cnt));
		u.ugold += cnt;
		flags.botl=1;
		*wishreturn = WISH_SUCCESS;
		return (&zeroobj);
#else
                otmp = mksobj(GOLD_PIECE, FALSE, FALSE);
		otmp->quan = cnt;
                otmp->owt = weight(otmp);
		flags.botl=1;
		*wishreturn = WISH_SUCCESS;
		return (otmp);
#endif
	}
	if (strlen(bp) == 1 &&
	   (i = def_char_to_objclass(*bp)) < MAXOCLASSES && i > ILLOBJ_CLASS
	    && (wizwish || i != VENOM_CLASS)
	    && i != VENOM_CLASS
	    ) {
		oclass = i;
		goto any;
	}
	
	/* Search for class names: XXXXX potion, scroll of XXXXX.  Avoid */
	/* false hits on, e.g., rings for "ring mail". */
	if(strncmpi(bp, "enchant ", 8) &&
	   strncmpi(bp, "destroy ", 8) &&
	   strncmpi(bp, "food detection", 14) &&
	   strncmpi(bp, "ring mail", 9) &&
	   strncmpi(bp, "studded leather arm", 19) &&
	   strncmpi(bp, "leather arm", 11) &&
	   strncmpi(bp, "tooled horn", 11) &&
	   strncmpi(bp, "food ration", 11) &&
	   strncmpi(bp, "meat ring", 9) && 
	   strncmpi(bp, "rod of lordly might", 19) && 
	   strncmpi(bp, "rod of the elvish lords", 23) && 
	   strncmpi(bp, "glamdring", 9) && 
	   strncmpi(bp, "black dress", 11) && 
	   strncmpi(bp, "noble's dress", 13) &&
	   strncmpi(bp, "sceptre of lolth", 16) && 
	   strncmpi(bp, "atma weapon", 11) &&
	   strncmpi(bp, "wand of orcus", 13)
	)
	for (i = 0; i < (int)(sizeof wrpsym); i++) {
		register int j = strlen(wrp[i]);
		if(!strncmpi(bp, wrp[i], j)){
			oclass = wrpsym[i];
			if(oclass != AMULET_CLASS) {
			    bp += j;
			    if(!strncmpi(bp, " of ", 4)) actualn = bp+4;
			    /* else if(*bp) ?? */
			} else
			    actualn = bp;
			goto srch;
		}
		if(!BSTRCMPI(bp, p-j, wrp[i])){
			oclass = wrpsym[i];
			p -= j;
			*p = 0;
			if(p > bp && p[-1] == ' ') p[-1] = 0;
			actualn = dn = bp;
			goto srch;
		}
	}

	/* "grey stone" check must be before general "stone" */
	for (i = 0; i < SIZE(o_ranges); i++)
	    if(!strcmpi(bp, o_ranges[i].name)) {
		typ = rnd_class(o_ranges[i].f_o_range, o_ranges[i].l_o_range);
		goto typfnd;
	    }

	if (!BSTRCMPI(bp, p-6, " stone")) {
		p[-6] = 0;
		oclass = GEM_CLASS;
		dn = actualn = bp;
		goto srch;
	} else if (!strcmpi(bp, "looking glass")) {
		;	/* avoid false hit on "* glass" */
	} else if (!BSTRCMPI(bp, p-6, " glass") || !strcmpi(bp, "glass")) {
		register char *g = bp;
		if (strstri(g, "broken"))
		{
			*wishreturn = WISH_FAILURE;
			return &zeroobj;
		}
		if (!strncmpi(g, "worthless ", 10)) g += 10;
		if (!strncmpi(g, "piece of ", 9)) g += 9;
		if (!strncmpi(g, "colored ", 8)) g += 8;
		else if (!strncmpi(g, "coloured ", 9)) g += 9;
		if (!strcmpi(g, "glass")) {	/* choose random color */
			/* 9 different kinds */
			typ = LAST_GEM + rnd(9);
			if (objects[typ].oc_class == GEM_CLASS) goto typfnd;
			else typ = 0;	/* somebody changed objects[]? punt */
		} else {		/* try to construct canonical form */
			char tbuf[BUFSZ];
			Strcpy(tbuf, "worthless piece of ");
			Strcat(tbuf, g);  /* assume it starts with the color */
			Strcpy(bp, tbuf);
		}
	}

	actualn = bp;
	if (!dn) dn = actualn; /* ex. "skull cap" */
srch:
	/* check real names of gems first */
	if(!oclass && actualn) {
	    for(i = bases[GEM_CLASS]; i <= LAST_GEM; i++) {
		register const char *zn;

		if((zn = OBJ_NAME(objects[i])) && !strcmpi(actualn, zn)) {
		    typ = i;
		    goto typfnd;
		}
	    }
	}
	i = oclass ? bases[(int)oclass] : 1;
	while(i < NUM_OBJECTS && (!oclass || objects[i].oc_class == oclass)){
		register const char *zn;

		if (actualn && (zn = OBJ_NAME(objects[i])) != 0 &&
			    wishymatch(actualn, zn, TRUE)) {
			typ = i;
			goto typfnd;
		}
		if (dn && (zn = OBJ_DESCR(objects[i])) != 0 &&
			    wishymatch(dn, zn, FALSE)) {
			/* don't match extra descriptions (w/o real name) */
			if (!OBJ_NAME(objects[i]))
			{
				*wishreturn = WISH_FAILURE;
				return &zeroobj;
			}
			typ = i;
			goto typfnd;
		}
		if (un && (zn = objects[i].oc_uname) != 0 &&
			    wishymatch(un, zn, FALSE)) {
			typ = i;
			goto typfnd;
		}
		i++;
	}
	if (actualn) {
		struct Jitem *j[] = {Japanese_items,ObscureJapanese_items,Pirate_items};
		for(i=0;i<sizeof(j)/sizeof(j[0]);i++)
		{
		while(j[i]->item) {
			if (actualn && !strcmpi(actualn, j[i]->name)) {
				typ = j[i]->item;
				goto typfnd;
			}
			j[i]++;
		}
		}
	}
	if (!strcmpi(bp, "spinach")) {
		contents = SPINACH;
		typ = TIN;
		goto typfnd;
	}
	/*This is meant to catch "scrolls of X"*/
	if(actualn){
	if(!strncmpi(actualn, "heptagram", 9)){
		heptagram = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	if(!strncmpi(actualn, "gorgoneion", 9)){
		gorgoneion = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	if(!strncmpi(actualn, "acheron", 7) ||
		!strncmpi(actualn, "circle", 6) ||
		!strncmpi(actualn, "circle of acheron", 17)){
		acheron = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	if(!strncmpi(actualn, "pentagram", 9)){
		pentagram = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	if(!strncmpi(actualn, "hexagram", 8)){
		hexagram = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	if(!strncmpi(actualn, "hamsa mark", 10) || 
		!strncmpi(actualn, "hamsa", 5)){
		hamsa = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	if(!strncmpi(actualn, "elder sign", 10)){
		sign = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	if(!strncmpi(actualn, "elder elemental eye", 19)
		||!strncmpi(actualn, "elder eye", 9)
		||!strncmpi(actualn, "elemental eye", 13)){
		eye = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	if(!strncmpi(actualn, "sign of the scion queen mother", 30)\
		||!strncmpi(actualn, "scion queen mother", 18)
		||!strncmpi(actualn, "queen mother", 12)
		||!strncmpi(actualn, "mother", 6)){
		queen = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	if(!strncmpi(actualn, "cartouche of the cat lord", 25)
		||!strncmpi(actualn, "cat lord", 8)
		||!strncmpi(actualn, "cartouche", 9)){
		cartouche = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	if(!strncmpi(actualn, "wings of garuda", 15)
		||!strncmpi(actualn, "wings", 5)
		||!strncmpi(actualn, "garuda", 6)){
		garuda = TRUE;
		typ = SCR_WARD;
		goto typfnd;
	}
	}
	/* Note: not strncmpi.  2 fruits, one capital, one not, are possible. */
	{
	    char *fp;
	    int l, cntf;
	    int blessedf, iscursedf, uncursedf, halfeatenf;

	    blessedf = iscursedf = uncursedf = halfeatenf = 0;
	    cntf = 0;

	    fp = fruitbuf;
	    for(;;) {
		if (!fp || !*fp) break;
		if (!strncmpi(fp, "an ", l=3) ||
		    !strncmpi(fp, "a ", l=2)) {
			cntf = 1;
		} else if (!cntf && digit(*fp)) {
			cntf = atoi(fp);
			while(digit(*fp)) fp++;
			while(*fp == ' ') fp++;
			l = 0;
		} else if (!strncmpi(fp, "blessed ", l=8)) {
			blessedf = 1;
		} else if (!strncmpi(fp, "cursed ", l=7)) {
			iscursedf = 1;
		} else if (!strncmpi(fp, "uncursed ", l=9)) {
			uncursedf = 1;
		} else if (!strncmpi(fp, "partly eaten ", l=13)) {
			halfeatenf = 1;
		} else break;
		fp += l;
	    }

	    for(f=ffruit; f; f = f->nextf) {
		char *f1 = f->fname, *f2 = makeplural(f->fname);

		if(!strncmp(fp, f1, strlen(f1)) ||
					!strncmp(fp, f2, strlen(f2))) {
			typ = SLIME_MOLD;
			blessed = blessedf;
			iscursed = iscursedf;
			uncursed = uncursedf;
			halfeaten = halfeatenf;
			cnt = cntf;
			ftype = f->fid;
			goto typfnd;
		}
	    }
	}

	if(!oclass && actualn) {
	    short objtyp;

	    /* Perhaps it's an artifact specified by name, not type */
	    name = artifact_name(actualn, &objtyp);
	    if(name) {
			isartifact = TRUE;
			typ = objtyp;
			goto typfnd;
	    }
	}
	/* Let wizards wish for traps --KAA */
	/* must come after objects check so wizards can still wish for
	 * trap objects like beartraps
	 */
	if (wizwish && from_user) {
		int trap;

		for (trap = NO_TRAP+1; trap < TRAPNUM; trap++) {
			const char *tname;

			tname = defsyms[trap_to_defsym(trap)].explanation;
			if (!strncmpi(tname, bp, strlen(tname))) {
				/* avoid stupid mistakes */
				if((trap == TRAPDOOR || trap == HOLE)
				      && !Can_fall_thru(&u.uz)) trap = ROCKTRAP;
				(void) maketrap(u.ux, u.uy, trap);
				pline("%s.", An(tname));
				*wishreturn = WISH_SUCCESS;
				return(&zeroobj);
			}
		}
		/* or some other dungeon features -dlc */
		p = eos(bp);
		if(!BSTRCMP(bp, p-8, "fountain")) {
			levl[u.ux][u.uy].typ = FOUNTAIN;
			level.flags.nfountains++;
			if(!strncmpi(bp, "magic ", 6))
				levl[u.ux][u.uy].blessedftn = 1;
			pline("A %sfountain.",
			      levl[u.ux][u.uy].blessedftn ? "magic " : "");
			newsym(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
			return(&zeroobj);
		}
		if(!BSTRCMP(bp, p-6, "throne")) {
			levl[u.ux][u.uy].typ = THRONE;
			pline("A throne.");
			newsym(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
			return(&zeroobj);
		}
# ifdef SINKS
		if(!BSTRCMP(bp, p-4, "sink")) {
			levl[u.ux][u.uy].typ = SINK;
			level.flags.nsinks++;
			pline("A sink.");
			newsym(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
			return &zeroobj;
		}
# endif
		if(!BSTRCMP(bp, p-4, "pool")) {
			levl[u.ux][u.uy].typ = POOL;
			del_engr_ward_at(u.ux, u.uy);
			pline("A pool.");
			/* Must manually make kelp! */
			water_damage(level.objects[u.ux][u.uy], FALSE, TRUE, level.flags.lethe, (struct monst *) 0);
			newsym(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
			return &zeroobj;
		}
		if(!BSTRCMP(bp, p-13, "shallow water")) {
			levl[u.ux][u.uy].typ = PUDDLE;
			del_engr_at(u.ux, u.uy);
			pline("Shallow water.");
			water_damage(level.objects[u.ux][u.uy], FALSE, TRUE, level.flags.lethe, (struct monst *) 0);
			newsym(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
			return &zeroobj;
		}
		if (!BSTRCMP(bp, p-4, "lava")) {  /* also matches "molten lava" */
			levl[u.ux][u.uy].typ = LAVAPOOL;
			del_engr_ward_at(u.ux, u.uy);
			pline("A pool of molten lava.");
			if (!(Levitation || Flying)) (void) lava_effects();
			newsym(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
			return &zeroobj;
		}
		if (!BSTRCMP(bp, p-5, "grass")) {  /* also matches "wild grass" */
			levl[u.ux][u.uy].typ = GRASS;
			pline("A patch of grass.");
			newsym(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
			return &zeroobj;
		}

		if(!BSTRCMP(bp, p-5, "altar")) {
		    aligntyp al;

		    levl[u.ux][u.uy].typ = ALTAR;
		    if(!strncmpi(bp, "chaotic ", 8))
			al = A_CHAOTIC;
		    else if(!strncmpi(bp, "neutral ", 8))
			al = A_NEUTRAL;
		    else if(!strncmpi(bp, "lawful ", 7))
			al = A_LAWFUL;
		    else if(!strncmpi(bp, "unaligned ", 10))
			al = A_NONE;
		    else if(!strncmpi(bp, "void ", 8))
			al = A_VOID;
		    else if(!strncmpi(bp, "non-aligned ", 8))
			al = A_VOID;
		    else if(!strncmpi(bp, "gnostic ", 8))
			al = A_VOID;
		    else /* -1 - A_CHAOTIC, 0 - A_NEUTRAL, 1 - A_LAWFUL */
			al = (!rn2(6)) ? A_NONE : rn2((int)A_LAWFUL+2) - 1;
		    levl[u.ux][u.uy].altarmask = Align2amask( al );
		    pline("%s altar.", An(align_str(al)));
		    newsym(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
		    return(&zeroobj);
		}

		if(!BSTRCMP(bp, p-5, "grave") || !BSTRCMP(bp, p-9, "headstone")) {
		    make_grave(u.ux, u.uy, (char *) 0);
		    pline("A grave.");
		    newsym(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
		    return(&zeroobj);
		}

		if(!BSTRCMP(bp, p-9, "dead tree")) {
		    levl[u.ux][u.uy].typ = DEADTREE;
		    pline("A dead tree.");
		    newsym(u.ux, u.uy);
		    block_point(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
		    return &zeroobj;
		}

		if(!BSTRCMP(bp, p-4, "tree")) {
		    levl[u.ux][u.uy].typ = TREE;
		    pline("A tree.");
		    newsym(u.ux, u.uy);
		    block_point(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
		    return &zeroobj;
		}

		if(!BSTRCMP(bp, p-4, "bars")) {
		    levl[u.ux][u.uy].typ = IRONBARS;
		    pline("Iron bars.");
		    newsym(u.ux, u.uy);
			*wishreturn = WISH_SUCCESS;
		    return &zeroobj;
		}
	}
	if (!oclass)
	{
		*wishreturn = WISH_FAILURE;
		return &zeroobj;
	}
any:
	if(!oclass) oclass = wrpsym[rn2((int)sizeof(wrpsym))];
typfnd:
	if (typ) oclass = objects[typ].oc_class;

	/* some objects are only allowed for tourists (or if it's an artifact) */
	if (typ && !wizwish && !Role_if(PM_TOURIST) && !isartifact && (
		typ == LIGHTSABER ||
		typ == BEAMSWORD ||
		typ == DOUBLE_LIGHTSABER ||
		typ == VIBROBLADE ||
		typ == WHITE_VIBROSWORD ||
		typ == GOLD_BLADED_VIBROSWORD ||
		typ == WHITE_VIBROZANBATO ||
		typ == GOLD_BLADED_VIBROZANBATO ||
		typ == RED_EYED_VIBROSWORD ||
		typ == SEISMIC_HAMMER ||
		typ == FORCE_PIKE ||
		typ == WHITE_VIBROSPEAR ||
		typ == GOLD_BLADED_VIBROSPEAR ||
		(typ >= PISTOL && typ <= RAYGUN) ||
		(typ >= SHOTGUN_SHELL && typ <= LASER_BEAM) ||
		typ == FLACK_HELMET ||
		typ == PLASTEEL_HELM ||
		typ == PLASTEEL_ARMOR ||
		typ == JUMPSUIT ||
		typ == BODYGLOVE ||
		typ == PLASTEEL_GAUNTLETS ||
		typ == PLASTEEL_BOOTS ||
		(typ >= SENSOR_PACK && typ <= HYPOSPRAY_AMPULE) ||
		typ == BULLET_FABBER ||
		typ == PROTEIN_PILL
		))
	{
		*wishreturn = WISH_DENIED;
		return &zeroobj;
	}

	/* some objects are never allowed */
	if (typ && !wizwish && (
		objects[typ].oc_unique ||
		objects[typ].oc_nowish ||
		typ == MAGIC_LAMP ||
		typ == CANDLE_OF_INVOCATION ||
		typ == WAN_WISHING ||
		typ == RIN_WISHES ||
		typ == SPE_LIGHTNING_BOLT ||
		typ == SPE_POISON_SPRAY ||
		typ == SPE_LIGHTNING_STORM ||
		typ == SCR_CONSECRATION
		))
	{
		*wishreturn = WISH_DENIED;
		return &zeroobj;
	}
	
	if(typ) {
		otmp = mksobj(typ, TRUE, FALSE);
	} else {
		otmp = mkobj(oclass, FALSE);
		if (otmp) typ = otmp->otyp;
	}

	if (islit &&
		(typ == OIL_LAMP || typ == MAGIC_LAMP || typ == BRASS_LANTERN ||
		 Is_candle(otmp) || typ == POT_OIL)) {
	    place_object(otmp, u.ux, u.uy);  /* make it viable light source */
	    begin_burn(otmp, FALSE);
	    obj_extract_self(otmp);	 /* now release it for caller's use */
	}

	if(cnt > 0 && objects[typ].oc_merge && oclass != SPBOOK_CLASS &&
		(cnt < rnd(6) || wizwish ||
		 (cnt <= 7 && Is_candle(otmp)) ||
		 (cnt <= 20 &&
		  ((oclass == WEAPON_CLASS && is_ammo(otmp))
				|| typ == ROCK || is_missile(otmp)))))
			otmp->quan = (long) cnt;

	if (oclass == VENOM_CLASS) otmp->spe = 1;

	if (spesgn == 0) spe = otmp->spe;
	else if (wizwish) /* no alteration to spe */;
	else if (oclass == ARMOR_CLASS || oclass == WEAPON_CLASS ||
		 is_weptool(otmp) ||
			(oclass==RING_CLASS && objects[typ].oc_charged)) {
		if(spe > rnd(5) && spe > otmp->spe) spe = 0;
		if(spe > 2 && Luck < 0) spesgn = -1;
	} else {
		if (oclass == WAND_CLASS) {
			if (spe > 1 && spesgn == -1) spe = 1;
		} else {
			if (spe > 0 && spesgn == -1) spe = 0;
		}
		if (spe > otmp->spe) spe = otmp->spe;
	}

	if (spesgn == -1) spe = -spe;

	/* set otmp->spe.  This may, or may not, use spe... */
	switch (typ) {
		case TIN: if (contents==EMPTY) {
				otmp->corpsenm = NON_PM;
				otmp->spe = 0;
			} else if (contents==SPINACH) {
				otmp->corpsenm = NON_PM;
				otmp->spe = 1;
			}
			break;
		case SLIME_MOLD: otmp->spe = ftype;
			/* Fall through */
		case SKELETON_KEY: case UNIVERSAL_KEY: case CHEST: case BOX:
		case HEAVY_IRON_BALL: case CHAIN: case STATUE:
			/* otmp->cobj already done in mksobj() */
				break;
#ifdef MAIL
		case SCR_MAIL: otmp->spe = 1; break;
#endif
		default: otmp->spe = spe;
	}

	/* set otmp->corpsenm or dragon scale [mail] */
	if (mntmp >= LOW_PM) {
		if (mntmp == PM_LONG_WORM_TAIL) mntmp = PM_LONG_WORM;
		if (mntmp == PM_HUNTING_HORROR_TAIL) mntmp = PM_HUNTING_HORROR;

		switch (typ) {
		case TIN:
			otmp->spe = 0; /* No spinach */
		case POT_BLOOD:
			if (dead_species(mntmp, FALSE)) {
			    otmp->corpsenm = NON_PM;	/* it's empty */
			} else if (!(mons[mntmp].geno & G_UNIQ) &&
				   !(mvitals[mntmp].mvflags & G_NOCORPSE) &&
				   mons[mntmp].cnutrit != 0 &&
				   !(typ==POT_BLOOD && !has_blood(&mons[mntmp]))) {
			    otmp->corpsenm = mntmp;
			}
			break;
		case MASK:
		    otmp->corpsenm = mntmp;
		break;
		case CORPSE:
			if (!(mons[mntmp].geno & G_UNIQ) &&
				   !(mvitals[mntmp].mvflags & G_NOCORPSE)) {
			    /* beware of random troll or lizard corpse,
			       or of ordinary one being forced to such */
			    if (otmp->timed) obj_stop_timers(otmp);
			    if (mons[mntmp].msound == MS_GUARDIAN)
			    	otmp->corpsenm = genus(mntmp,1);
			    else
				otmp->corpsenm = mntmp;
			    start_corpse_timeout(otmp);
			}
			break;
		case FIGURINE:
			if (!(mons[mntmp].geno & G_UNIQ)
			    && !is_unwishable(&mons[mntmp])
			) otmp->corpsenm = mntmp;
			break;
		case EGG:
			mntmp = can_be_hatched(mntmp);
			if (mntmp != NON_PM) {
			    otmp->corpsenm = mntmp;
			    if (!dead_species(mntmp, TRUE))
				attach_egg_hatch_timeout(otmp);
			    else
				kill_egg(otmp);
			}
			break;
		case STATUE: otmp->corpsenm = mntmp;
			if (Has_contents(otmp) && verysmall(&mons[mntmp]))
			    delete_contents(otmp);	/* no spellbook */
			otmp->spe = ishistoric ? STATUE_HISTORIC : 0;
			break;
		case FOSSIL: otmp->corpsenm = mntmp;
			break;
		case SCALE_MAIL:
			/* Dragon mail - depends on the order of objects */
			/*		 & dragons.			 */
			if (mntmp >= PM_GRAY_DRAGON &&
				mntmp <= PM_YELLOW_DRAGON
			){
			    otmp->otyp = GRAY_DRAGON_SCALE_MAIL +
						    mntmp - PM_GRAY_DRAGON;
				otmp->obj_material = objects[otmp->otyp].oc_material;
			}
			break;
		case KITE_SHIELD:
			/* Dragon shield - depends on the order of objects */
			/*		 & dragons.			 */
			if (mntmp >= PM_GRAY_DRAGON &&
						mntmp <= PM_YELLOW_DRAGON)
			    otmp->otyp = GRAY_DRAGON_SCALE_SHIELD +
						    mntmp - PM_GRAY_DRAGON;
				otmp->obj_material = objects[otmp->otyp].oc_material;
			break;
		}
	}

	if(stolen){
		otmp->ostolen = 1;
		otmp->sknown = 1;
	}
	
	if(otmp->oclass == WEAPON_CLASS || (otmp->oclass == ARMOR_CLASS && !Is_dragon_scales(otmp))){
		otmp->objsize = objsize;
	}
	
	if(otmp->oclass == ARMOR_CLASS && !Is_dragon_scales(otmp)){
		if(bodytype == 0L){
			if(is_suit(otmp)) otmp->bodytypeflag = (youracedata->mflagsb&MB_BODYTYPEMASK);
			else if(is_helmet(otmp)) otmp->bodytypeflag = (youracedata->mflagsb&MB_HEADMODIMASK);
			else if(is_shirt(otmp)) otmp->bodytypeflag = (youracedata->mflagsb&MB_HUMANOID) ? MB_HUMANOID : (youracedata->mflagsb&MB_BODYTYPEMASK);
		} else {
			if(is_suit(otmp)){
				if((bodytype&MB_BODYTYPEMASK) != 0L) otmp->bodytypeflag = (bodytype&MB_BODYTYPEMASK);
				else otmp->bodytypeflag = (youracedata->mflagsb&MB_BODYTYPEMASK);
			} else if(is_helmet(otmp)){
				if((bodytype&MB_HEADMODIMASK) != 0L) otmp->bodytypeflag = (bodytype&MB_HEADMODIMASK);
				else if((bodytype&MB_HUMANOID) != 0L) otmp->bodytypeflag = 0L; //Humanoid heads have no special modifier
				else otmp->bodytypeflag = (youracedata->mflagsb&MB_HEADMODIMASK);
			} else if(is_shirt(otmp)){
				if((bodytype&MB_BODYTYPEMASK) != 0L) otmp->bodytypeflag = (bodytype&MB_HUMANOID) ? MB_HUMANOID : (bodytype&MB_BODYTYPEMASK);
				else otmp->bodytypeflag = (youracedata->mflagsb&MB_HUMANOID) ? MB_HUMANOID : (youracedata->mflagsb&MB_BODYTYPEMASK);
			}
		}
		
	}
	
	if(otmp->oclass == RING_CLASS && isEngrRing((otmp)->otyp) && (wizwish || (otmp->oward && !(otmp->ohaluengr)))){
		if(heptagram && wizwish)			otmp->oward = HEPTAGRAM;  /*can't be wished for*/
		else if(gorgoneion && wizwish)		otmp->oward = GORGONEION;/*can't be wished for*/
		else if(acheron)					otmp->oward = CIRCLE_OF_ACHERON;
		else if(pentagram)					otmp->oward = PENTAGRAM; /*not found randomly, but can be wished for*/
		else if(hexagram && wizwish) 		otmp->oward = HEXAGRAM;/*can't be wished for*/
		else if(hamsa)						otmp->oward = HAMSA;
		else if(sign)						otmp->oward = ELDER_SIGN;
		else if(eye)						otmp->oward = ELDER_ELEMENTAL_EYE;
		else if(queen)						otmp->oward = SIGN_OF_THE_SCION_QUEEN;
		else if(cartouche)					otmp->oward = CARTOUCHE_OF_THE_CAT_LORD;
		else if(garuda)						otmp->oward = WINGS_OF_GARUDA;
		else if(toustefna && wizwish)		otmp->oward = TOUSTEFNA;/*can't be wished for*/
		else if(dreprun && wizwish)			otmp->oward = DREPRUN;/*can't be wished for*/
		else if(veioistafur && wizwish)		otmp->oward = VEIOISTAFUR;/*can't be wished for*/
		else if(thjofastafur && wizwish)	otmp->oward = THJOFASTAFUR; /*can't be wished for*/
	}

	
	if(otmp->otyp == SCR_WARD){
		/* Can wish for a scroll of any ward, including heptagram. You are spending a wish, after all.*/
		if(heptagram) otmp->oward = HEPTAGRAM;
		else if(gorgoneion) otmp->oward = GORGONEION;
		else if(acheron) otmp->oward = CIRCLE_OF_ACHERON;
		else if(pentagram) otmp->oward = PENTAGRAM;
		else if(hexagram) otmp->oward = HEXAGRAM;
		else if(hamsa) otmp->oward = HAMSA;
		else if(sign) otmp->oward = ELDER_SIGN;
		else if(eye) otmp->oward = ELDER_ELEMENTAL_EYE;
		else if(queen) otmp->oward = SIGN_OF_THE_SCION_QUEEN;
		else if(cartouche) otmp->oward = CARTOUCHE_OF_THE_CAT_LORD;
		else if(garuda) otmp->oward = WINGS_OF_GARUDA;
	}
	
	/*You're spending a wish, you can get whatever stave you ask for*/
	if(otmp->oclass == WEAPON_CLASS && objects[(otmp)->otyp].oc_material == WOOD){
		if(toustefna) otmp->oward = WARD_TOUSTEFNA; 
		else if(dreprun) otmp->oward = WARD_DREPRUN;
		else if(veioistafur) otmp->oward = WARD_VEIOISTAFUR;
		else if(thjofastafur) otmp->oward = WARD_THJOFASTAFUR;
	}
	/* set blessed/cursed -- setting the fields directly is safe
	 * since weight() is called below and addinv() will take care
	 * of luck */
	if (iscursed) {
		curse(otmp);
	} else if (uncursed) {
		otmp->blessed = 0;
		otmp->cursed = (Luck < 0 && !wizwish);
	} else if (blessed) {
		otmp->blessed = (Luck >= 0 || wizwish);
		otmp->cursed = (Luck < 0 && !wizwish);
	} else if (spesgn < 0) {
		curse(otmp);
	}

#ifdef INVISIBLE_OBJECTS
	if (isinvisible) otmp->oinvis = 1;
#endif

	/* set eroded */
	if (is_damageable(otmp) || otmp->otyp == CRYSKNIFE) {
	    if (eroded && (is_flammable(otmp) || is_rustprone(otmp)))
		    otmp->oeroded = eroded;
	    if (eroded2 && (is_corrodeable(otmp) || is_rottable(otmp)))
		    otmp->oeroded2 = eroded2;
	    if (eroded3 && otmp->otyp == DROVEN_CLOAK)
		    otmp->oeroded3 = eroded3;

	    /* set erodeproof */
	    if (erodeproof && !eroded && !eroded2)
			otmp->oerodeproof = (Luck >= 0 || wizwish);
	}

	/* set otmp->recharged */
	if (oclass == WAND_CLASS) {
	    /* prevent wishing abuse */
		if (otmp->otyp == WAN_WISHING && !wizwish)
			rechrg = 1;
	    otmp->recharged = (unsigned)rechrg;
	}

	/* set poisoned */
	if (ispoisoned) {
	    if (is_poisonable(otmp))
		otmp->opoisoned = (Luck >= 0) ? ispoisoned : 0;
	    else if (Is_box(otmp) || typ == TIN)
		otmp->otrapped = 1;
	    else if (oclass == FOOD_CLASS)
		/* try to taint by making it as old as possible */
		otmp->age = 1L;
	}

	if (isgreased) otmp->greased = 1;

	if (isdiluted && otmp->oclass == POTION_CLASS &&
			otmp->otyp != POT_WATER)
		otmp->odiluted = 1;

	/* set material */
	if(mat){
		if (wizwish) {
			otmp->obj_material = mat;
		}
		else {
			if(otmp->oclass == WEAPON_CLASS && !otmp->oartifact){
				if(		// flexible materials
						((otmp->obj_material == CLOTH
						|| otmp->obj_material == LEATHER
						|| otmp->obj_material == PLASTIC)
						&&(mat == CLOTH
						|| mat == LEATHER)
					)
					||	// rigid materials
						(((otmp->obj_material >= DRAGON_HIDE && otmp->obj_material <= MITHRIL)
						|| otmp->obj_material == GLASS
						|| otmp->obj_material == BONE
						|| otmp->obj_material == WOOD
						|| otmp->obj_material == OBSIDIAN_MT
						|| otmp->obj_material == GEMSTONE
						|| otmp->obj_material == MINERAL)
						&&((mat >= DRAGON_HIDE && mat <= MITHRIL)
						|| mat == GLASS
						|| mat == OBSIDIAN_MT
						|| mat == MINERAL)
						)
					){
					set_material(otmp, mat);
				}
			}
		}
	}
	
	/* set object properties */
	if (oproperties && wizwish) // wishing for object properties is wizard-mode only
	{
		if (wizwish)		// wizard mode will give you what you ask for, even if it breaks things
			otmp->oproperties = oproperties;
		else
		{	// limit granted properties to what is realistic for the item class
			switch (otmp->oclass)
			{
			case TOOL_CLASS:
				if (is_weptool(otmp))
					;	// fall through to weapon_class
				else
					break;
			case WEAPON_CLASS:
				oproperties &= (OPROP_FIREW | OPROP_COLDW | OPROP_PSIOW | OPROP_DEEPW | OPROP_WATRW | OPROP_ELECW | OPROP_ACIDW | OPROP_MAGCW 
								| OPROP_ANARW | OPROP_CONCW | OPROP_AXIOW | OPROP_HOLYW | OPROP_UNHYW | OPROP_VORPW | OPROP_MORGW | OPROP_FLAYW
								| OPROP_PHSEW | OPROP_LESSW);
				break;
			case ARMOR_CLASS:
				oproperties &= (OPROP_FIRE | OPROP_COLD | OPROP_ELEC | OPROP_ACID | OPROP_MAGC | OPROP_ANAR | OPROP_CONC | OPROP_AXIO | OPROP_REFL);
				break;
			default:
				oproperties = 0;
			}
			otmp->oproperties = oproperties;
		}
	}
		


	if (name) {
		const char *aname;
		short objtyp;

		/* an artifact name might need capitalization fixing */
		aname = artifact_name(name, &objtyp);
		if (aname && objtyp == otmp->otyp){
			isartifact = TRUE;
			name = aname;
		}

		otmp = oname(otmp, name);
		if (otmp->oartifact) {
			u.uconduct.wisharti++;	/* KMH, conduct */
		}
	}

	/* set ampule type */
	if(ampule != -1 && otmp->otyp == HYPOSPRAY_AMPULE){
		otmp->ovar1 = ampule;
	}

	/* set viper heads, probability of getting what you wished for copied loosely from setting weapon/armor spe, but the minimum is 1, not 0. */
	if(viperheads != -1 && otmp->otyp == VIPERWHIP){
		otmp->ovar1 = (viperheads > rnd(5) && viperheads > otmp->ovar1 && !wizwish) ? 1 : viperheads;
	}
	
	/* set moon phase */
	if(moonphase != -1 && otmp->otyp == MOON_AXE){
		otmp->ovar1 = moonphase;
	}
	
	/* more wishing abuse: don't allow wishing for certain artifacts */
	/* and make them pay; charge them for the wish anyway! */
	if (otmp->oartifact && !wizwish &&
		(is_quest_artifact(otmp) //redundant failsafe.  You can't wish for ANY quest artifacts
		 || otmp->oartifact >= ART_ROD_OF_SEVEN_PARTS //No wishing for quest artifacts, unique monster artifacts, etc.
		 || !touch_artifact(otmp, &youmonst, TRUE) //Auto-fail a wish for an artifact you wouldn't be able to touch (mercy rule)
		 || !allow_artifact								// pre-determined if any artifact wish is allowed
		 // depreciated criteria:
		 // (otmp->oartifact && rn2((int)(u.uconduct.wisharti)) > 1) //Limit artifact wishes per game
		 // (otmp->oartifact >= ART_ITLACHIAYAQUE && otmp->oartifact <= ART_EYE_OF_THE_AETHIOPICA) || //no wishing for quest artifacts
		 // (otmp->oartifact >= ART_ROD_OF_SEVEN_PARTS && otmp->oartifact <= ART_SILVER_KEY) || //no wishing for alignment quest artifacts
		 // (otmp->oartifact >= ART_SWORD_OF_ERATHAOL && otmp->oartifact <= ART_HAMMER_OF_BARQUIEL) || //no wishing for angel artifacts
		 // (otmp->oartifact >= ART_GENOCIDE && otmp->oartifact <= ART_DOOMSCREAMER) || //no wishing for demon artifacts
		 // (otmp->oartifact >= ART_STAFF_OF_THE_ARCHMAGI && otmp->oartifact <= ART_SNICKERSNEE)
	    )) {
	    artifact_exists(otmp, ONAME(otmp), FALSE);	// Is this necessary?
		u.uconduct.wisharti--;
	    obfree(otmp, (struct obj *) 0);		// Is this necessary?
	    otmp = &zeroobj;					// Is this necessary?

		*wishreturn = WISH_DENIED;
		return &zeroobj;
	}
	
	if (halfeaten && otmp->oclass == FOOD_CLASS) {
		if (otmp->otyp == CORPSE)
			otmp->oeaten = mons[otmp->corpsenm].cnutrit;
		else otmp->oeaten = objects[otmp->otyp].oc_nutrition;
		/* (do this adjustment before setting up object's weight) */
		consume_oeaten(otmp, 1);
	}
	if (isdrained && otmp->otyp == CORPSE) {
		int amt;
		otmp->odrained = 1;
		amt = mons[otmp->corpsenm].cnutrit - drainlevel(otmp);
		if (halfdrained) {
		    amt /= 2;
		    if (amt == 0)
			amt++;
		}
		/* (do this adjustment before setting up object's weight) */
		consume_oeaten(otmp, -amt);
	}
	otmp->owt = weight(otmp);
	if (very && otmp->otyp == HEAVY_IRON_BALL) otmp->owt += 160;

	*wishreturn = WISH_SUCCESS;
	return(otmp);
}

int
rnd_class(first,last)
int first,last;
{
	int i, x, sum=0;

	if (first == last)
	    return (first);
	for(i=first; i<=last; i++)
		sum += objects[i].oc_prob;
	if (!sum) /* all zero */
		return first + rn2(last-first+1);
	x = rnd(sum);
	for(i=first; i<=last; i++)
		if (objects[i].oc_prob && (x -= objects[i].oc_prob) <= 0)
			return i;
	return 0;
}

STATIC_OVL const char *
Alternate_item_name(i,alternate_items)
int i;
struct Jitem *alternate_items;
{
	while(alternate_items->item) {
		if (i == alternate_items->item)
			return alternate_items->name;
		alternate_items++;
	}
	return (const char *)0;
}

const char *
cloak_simple_name(cloak)
struct obj *cloak;
{
    if (cloak) {
	switch (cloak->otyp) {
	case ROBE:
	    return "robe";
	case MUMMY_WRAPPING:
	    return "wrapping";
	case ALCHEMY_SMOCK:
	    return (objects[cloak->otyp].oc_name_known &&
			cloak->dknown) ? "smock" : "apron";
	default:
	    break;
	}
    }
    return "cloak";
}

const char *
mimic_obj_name(mtmp)
struct monst *mtmp;
{
	if (mtmp->m_ap_type == M_AP_OBJECT && mtmp->mappearance != STRANGE_OBJECT) {
		int idx = objects[mtmp->mappearance].oc_descr_idx;
		if (mtmp->mappearance == GOLD_PIECE) return "gold";
		return obj_descr[idx].oc_name;
	}
	return "whatcha-may-callit";
}
#endif /* OVLB */

/*objnam.c*/
