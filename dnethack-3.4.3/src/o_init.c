/*	SCCS Id: @(#)o_init.c	3.4	1999/12/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"	/* save & restore info */

STATIC_DCL void FDECL(setgemprobs, (d_level*));
STATIC_DCL void FDECL(shuffle,(int,int,BOOLEAN_P));
STATIC_DCL void NDECL(shuffle_all);
STATIC_DCL boolean FDECL(interesting_to_discover,(int));


static NEARDATA short disco[NUM_OBJECTS] = DUMMY;

#ifdef USE_TILES
STATIC_DCL void NDECL(shuffle_tiles);
extern short glyph2tile[];	/* from tile.c */

/* Shuffle tile assignments to match descriptions, so a red potion isn't
 * displayed with a blue tile and so on.
 *
 * Tile assignments are not saved, and shouldn't be so that a game can
 * be resumed on an otherwise identical non-tile-using binary, so we have
 * to reshuffle the assignments from oc_descr_idx information when a game
 * is restored.  So might as well do that the first time instead of writing
 * another routine.
 */
STATIC_OVL void
shuffle_tiles()
{
	int i;
	short tmp_tilemap[NUM_OBJECTS];

	for (i = 0; i < NUM_OBJECTS; i++)
		tmp_tilemap[i] =
			glyph2tile[objects[i].oc_descr_idx + GLYPH_OBJ_OFF];

	for (i = 0; i < NUM_OBJECTS; i++)
		glyph2tile[i + GLYPH_OBJ_OFF] = tmp_tilemap[i];
}
#endif	/* USE_TILES */

STATIC_OVL void
setgemprobs(dlev)
d_level *dlev;
{
	int j, first, lev, totalprob=0;

	if (dlev)
	    lev = (ledger_no(dlev) > maxledgerno())
				? maxledgerno() : ledger_no(dlev);
	else
	    lev = 0;
	first = bases[GEM_CLASS];

	for(j = 0; j < 11-lev/3; j++){
		totalprob += objects[first+j].oc_prob;
		objects[first+j].oc_prob = 0;
	}/*j=0;*/ //turned off gem limitation by dungeon level. Then turned it back on.
	first += j;
	if (first > LAST_GEM || objects[first].oc_class != GEM_CLASS ||
	    OBJ_NAME(objects[first]) == (char *)0) {
		raw_printf("Not enough gems? - level=%d first=%d j=%d LAST_GEM=%d",
			lev, first, j, LAST_GEM);
		wait_synch();
	    }
	//Note, 175 is the sum of the oc_probs for the true gems
	for (j = first; j <= LAST_GEM; j++)
		objects[j].oc_prob = (175+j-first)/(LAST_GEM+1-first);
}

/* shuffle descriptions on objects o_low to o_high */
STATIC_OVL void
shuffle(o_low, o_high, domaterial)
	int o_low, o_high;
	boolean domaterial;
{
	int i, j, num_to_shuffle;
	short sw;
	int color;

	for (num_to_shuffle = 0, j=o_low; j <= o_high; j++)
		if (!objects[j].oc_name_known) num_to_shuffle++;
	if (num_to_shuffle < 2) return;

	for (j=o_low; j <= o_high; j++) {
		if (objects[j].oc_name_known) continue;
		do
			i = j + rn2(o_high-j+1);
		while (objects[i].oc_name_known);
		sw = objects[j].oc_descr_idx;
		objects[j].oc_descr_idx = objects[i].oc_descr_idx;
		objects[i].oc_descr_idx = sw;
		sw = objects[j].oc_tough;
		objects[j].oc_tough = objects[i].oc_tough;
		objects[i].oc_tough = sw;
		color = objects[j].oc_color;
		objects[j].oc_color = objects[i].oc_color;
		objects[i].oc_color = color;

		/* shuffle material */
		if (domaterial) {
			sw = objects[j].oc_material;
			objects[j].oc_material = objects[i].oc_material;
			objects[i].oc_material = sw;
		}
	}
}

void
init_objects()
{
register int i, first, last, sum;
register char oclass;
	/* bug fix to prevent "initialization error" abort on Intel Xenix.
	 * reported by mikew@semike
	 */
	for (i = 0; i < MAXOCLASSES; i++)
		bases[i] = 0;
	/* initialize object descriptions */
	for (i = 0; i < NUM_OBJECTS; i++)
		objects[i].oc_name_idx = objects[i].oc_descr_idx = i;
	/* init base; if probs given check that they add up to 1000,
	   otherwise compute probs */
	first = 0;
	while( first < NUM_OBJECTS ) {
		oclass = objects[first].oc_class;
		last = first+1;
		while (last < NUM_OBJECTS && objects[last].oc_class == oclass) last++;
		bases[(int)oclass] = first;

		if (oclass == GEM_CLASS) {
			setgemprobs((d_level *)0);

			if (rn2(2)) { /* change turquoise from green to blue? */
			    COPY_OBJ_DESCR(objects[TURQUOISE],objects[SAPPHIRE]);
			}
			if (rn2(2)) { /* change aquamarine from green to blue? */
			    COPY_OBJ_DESCR(objects[AQUAMARINE],objects[SAPPHIRE]);
			}
			// switch (rn2(4)) { /* change fluorite from violet? */
			    // case 0:  break;
			    // case 1:	/* blue */
				// COPY_OBJ_DESCR(objects[FLUORITE],objects[SAPPHIRE]);
				// break;
			    // case 2:	/* white */
				// COPY_OBJ_DESCR(objects[FLUORITE],objects[DIAMOND]);
				// break;
			    // case 3:	/* green */
				// COPY_OBJ_DESCR(objects[FLUORITE],objects[EMERALD]);
				// break;
			// }
		}
	check:
		sum = 0;
		for(i = first; i < last; i++) sum += objects[i].oc_prob;
		if(sum == 0) {
			if(objects[first].oc_class == RING_CLASS)
				first++;//Ring of wishes should remain 0
			for(i = first; i < last; i++)
			    objects[i].oc_prob = (1000+i-first)/(last-first);
			goto check;
		}
		if(sum != 1000)
			error("init-prob error for class %d (%d%%)", oclass, sum);
		first = last;
	}
	/* shuffle descriptions */
	shuffle_all();
#ifdef USE_TILES
	shuffle_tiles();
#endif
}

STATIC_OVL void
shuffle_all()
{
	int first, last, oclass, signetring;

	for (oclass = 1; oclass < MAXOCLASSES; oclass++) {
		first = bases[oclass];
		last = first+1;
		while (last < NUM_OBJECTS && objects[last].oc_class == oclass)
			last++;

		if (OBJ_DESCR(objects[first]) != (char *)0 &&
				oclass != TOOL_CLASS &&
				oclass != WEAPON_CLASS &&
				oclass != ARMOR_CLASS &&
				oclass != GEM_CLASS) {
			int j = last-1;

			if (oclass == POTION_CLASS)
			    j -= 6;  /* goat's milk, space mead, water, starlight, amnesia, and blood have fixed descriptions */
			if (oclass == RING_CLASS)
				first += 1;	/* three wishes has a fixed description */
			if (oclass == WAND_CLASS)
			    first += 3;  /* light, darkness, and wishing have fixed descriptions */
			else if (oclass == AMULET_CLASS ||
				 oclass == TILE_CLASS ||
				 oclass == SPBOOK_CLASS
			){
			    while (!objects[j].oc_magic || objects[j].oc_unique)
				j--;
			}
			else if (oclass == SCROLL_CLASS){
				j=first;
			    while (objects[j].oc_magic) //Assumes blank scroll is the sentential
					j++;
				j--;
			}

			/* non-magical amulets, scrolls, and spellbooks
			 * (ex. imitation Amulets, blank, scrolls of mail)
			 * and one-of-a-kind magical artifacts at the end of
			 * their class in objects[] have fixed descriptions.
			 */
			shuffle(first, j, TRUE);
		}
	}
	
	/* check signet ring */
	
	signetring = find_signet_ring();
	while(signetring == RIN_LEVITATION ||
	   signetring== RIN_AGGRAVATE_MONSTER ||
	   signetring== RIN_HUNGER ||
	   signetring== RIN_POLYMORPH ||
	   signetring== RIN_POLYMORPH_CONTROL
	){
		shuffle(RIN_ADORNMENT, RIN_PROTECTION_FROM_SHAPE_CHAN, TRUE);
		signetring = find_signet_ring();
	}
	
	/* shuffle the helmets */
	shuffle(HELMET, HELM_OF_DRAIN_RESISTANCE, TRUE);
	
	/* shuffle the gloves */
	shuffle(GLOVES, GAUNTLETS_OF_DEXTERITY, FALSE);

	/* shuffle the cloaks */
	shuffle(CLOAK_OF_PROTECTION, CLOAK_OF_DISPLACEMENT, FALSE);

	/* shuffle the boots [if they change, update find_skates() below] */
	shuffle(SPEED_BOOTS, FLYING_BOOTS, FALSE);
}

/* find the object index for silver attack wand */
int
find_sawant()
{
    static int i = -1;
    register const char *s;
	
	if(i != -1) return i;
	
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "silver")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "hexagonal")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "short")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "runed")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "long")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "curved")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "forked")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "spiked")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
	i = 0;
    return i;
}

/* find the object index for gold attack wand */
int
find_gawant()
{
    static int i = -1;
    register const char *s;
	
	if(i != -1) return i;
	
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "gold")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "runed")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "long")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "forked")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "spiked")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "hexagonal")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "short")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "curved")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
	i = 0;
    return i;
}

/* find the object index for platinum attack wand */
int
find_pawant()
{
    static int i = -1;
    register const char *s;
	
	if(i != -1) return i;
	
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "platinum")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "runed")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "long")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "forked")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "spiked")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "hexagonal")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "short")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
    for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "curved")
			)
		){
			if(is_attack_wand(i))
				return i;
			else break;
		}
	i = 0;
    return i;
}

/* find the object index for iron wand */
int
find_riwant()
{
    int i = -1;
    int j;
    register const char *s;
	static char *candidateWands[] = {
		"iron",
		"hexagonal",
		"short",
		"runed",
		"long",
		"curved",
		"forked",
		"spiked"
	};
	int offset = rn2(SIZE(candidateWands));
	
	for(j = 0; j < SIZE(candidateWands); j++){
		for (i = WAN_LIGHT; i <= WAN_LIGHTNING; i++)
			if ((s = OBJ_DESCR(objects[i])) != 0 && (
				!strcmp(s, candidateWands[(j+offset)%SIZE(candidateWands)])
				)
			){
				// if(is_attack_wand(i))
				return i;
				// else break;
			}
	}
	i = 0;
    return i;
}

/* find the object index for circlet */
int
find_gcirclet()
{
    register int i;
    register const char *s;

    for (i = HELMET; i <= HELM_OF_DRAIN_RESISTANCE; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "band")
		)) return i;

    impossible("band not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for silver ring */
int
find_sring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "silver")
		)) return i;

    impossible("silver ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for iron ring */
int
find_iring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "iron")
		)) return i;

    impossible("iron ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for a non-hunger iron ring */
int
find_good_iring()
{
    register const char *s;
    register int i = find_iring();
	
	if(i != RIN_POLYMORPH)
		return i;
	
    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && (
			!strcmp(s, "twisted")
		)) return i;

    impossible("twisted ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for visored helmet */
int
find_vhelm()
{
    register int i;
    register const char *s;

    for (i = HELMET; i <= HELM_OF_DRAIN_RESISTANCE; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && 
			!strcmp(s, "visored helmet")
		) return i;

    impossible("visored helmet not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for snow boots; used [once] by slippery ice code */
int
find_skates()
{
    register int i;
    register const char *s;

    for (i = SPEED_BOOTS; i <= FLYING_BOOTS; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "snow boots"))
	    return i;

    impossible("snow boots not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for combat boots */
int
find_cboots()
{
    register int i;
    register const char *s;

    for (i = SPEED_BOOTS; i <= FLYING_BOOTS; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "combat boots"))
	    return i;

    impossible("combat boots not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for mud boots */
int
find_mboots()
{
    register int i;
    register const char *s;

    for (i = SPEED_BOOTS; i <= FLYING_BOOTS; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "mud boots"))
	    return i;

    impossible("mud boots not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for hiking boots */
int
find_hboots()
{
    register int i;
    register const char *s;

    for (i = SPEED_BOOTS; i <= FLYING_BOOTS; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "hiking boots"))
	    return i;

    impossible("hiking boots not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for buckled boots */
int
find_bboots()
{
    register int i;
    register const char *s;

    for (i = SPEED_BOOTS; i <= FLYING_BOOTS; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "buckled boots"))
	    return i;

    impossible("buckled boots not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for jungle boots */
int
find_jboots()
{
    register int i;
    register const char *s;

    for (i = SPEED_BOOTS; i <= FLYING_BOOTS; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "jungle boots"))
	    return i;

    impossible("jungle boots not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for the ornamental cope */
int
find_cope()
{
    register int i;
    register const char *s;

    for (i = CLOAK_OF_PROTECTION; i <= CLOAK_OF_DISPLACEMENT; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "ornamental cope"))
			return i;

    impossible("ornamental cope not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for the opera cloak */
int
find_opera_cloak()
{
    register int i;
    register const char *s;

    for (i = CLOAK_OF_PROTECTION; i <= CLOAK_OF_DISPLACEMENT; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "opera cloak"))
			return i;

    impossible("opera cloak not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for the signet ring */
int
find_signet_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "black signet"))
			return i;

    impossible("signet ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for the engagement ring */
int
find_engagement_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
		if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "engagement"))
			return i;

    impossible("engagement ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for the gold ring */
int
find_gold_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "gold"))
	    return i;

    impossible("gold ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_silver_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "silver"))
	    return i;

    impossible("silver ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_opal_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "opal"))
	    return i;

    impossible("opal ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_clay_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "clay"))
	    return i;

    impossible("clay ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_coral_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "coral"))
	    return i;

    impossible("coral ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_onyx_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "black onyx"))
	    return i;

    impossible("black onyx ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_moonstone_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "moonstone"))
	    return i;

    impossible("moonstone ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_jade_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "jade"))
	    return i;

    impossible("jade ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_agate_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "agate"))
	    return i;

    impossible("agate ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_topaz_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "topaz"))
	    return i;

    impossible("topaz ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_sapphire_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "sapphire"))
	    return i;

    impossible("sapphire ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_ruby_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "ruby"))
	    return i;

    impossible("ruby ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_pearl_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "pearl"))
	    return i;

    impossible("pearl ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_ivory_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "ivory"))
	    return i;

    impossible("ivory ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_emerald_ring()
{
    register int i;
    register const char *s;

    for (i = RIN_ADORNMENT; i <= RIN_PROTECTION_FROM_SHAPE_CHAN; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "emerald"))
	    return i;

    impossible("emerald ring not found?");
    return -1;	/* not 0, or caller would try again each move */
}

boolean
isSignetRing(otyp)
int otyp;
{
	static int sigring = 0;
	if(!sigring) sigring = find_signet_ring();
	return sigring == otyp;
}

/* test if a ring is an engravable ring */
boolean
isEngrRing(otyp)
int otyp;
{
    register int i;
	
	static int egrings[13] = {0,0,0, 0,0,0, 0,0,0, 0,0,0, 0}; 
	if(!egrings[0]){ 
		egrings[0] = find_opal_ring(); 
		egrings[1] = find_clay_ring(); 
		egrings[2] = find_coral_ring();
		
		egrings[3] = find_onyx_ring();
		egrings[4] = find_moonstone_ring();
		egrings[5] = find_jade_ring();
		
		egrings[6] = find_agate_ring();
		egrings[7] = find_topaz_ring();
		egrings[8] = find_sapphire_ring();
		
		egrings[9] = find_ruby_ring();
		egrings[10] = find_pearl_ring(); /* diamond is too hard */
		egrings[11] = find_ivory_ring();
	
		egrings[12] = find_emerald_ring();
	}
	
    for (i = 0; i < 13; i++)
		if (egrings[i] == otyp) return TRUE;

    return FALSE;
}

int
find_golden_potion()
{
    register int i;
    register const char *s;

    for (i = POT_GAIN_ABILITY; i <= POT_BLOOD; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "golden"))
	    return i;

    impossible("golden potion not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_cloth_book()
{
    register int i;
    register const char *s;

    for (i = SPE_DIG; i <= SPE_BLANK_PAPER; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "cloth"))
	    return i;

    impossible("cloth spellbook not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_leather_book()
{
    register int i;
    register const char *s;

    for (i = SPE_DIG; i <= SPE_BLANK_PAPER; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "leather"))
	    return i;

    impossible("leather spellbook not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_bronze_book()
{
    register int i;
    register const char *s;

    for (i = SPE_DIG; i <= SPE_BLANK_PAPER; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "bronze"))
	    return i;

    impossible("bronze spellbook not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_silver_book()
{
    register int i;
    register const char *s;

    for (i = SPE_DIG; i <= SPE_BLANK_PAPER; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "silver"))
	    return i;

    impossible("silver spellbook not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_gold_book()
{
    register int i;
    register const char *s;

    for (i = SPE_DIG; i <= SPE_BLANK_PAPER; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "gold"))
	    return i;

    impossible("gold spellbook not found?");
    return -1;	/* not 0, or caller would try again each move */
}

int
find_hexagonal_wand()
{
    register int i;
    register const char *s;

    for (i = WAN_SECRET_DOOR_DETECTION; i <= WAN_LIGHTNING; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "hexagonal"))
	    return i;

    // impossible("wand not found?"); //not all wands used
    return -1;	/* not 0, or caller would try again each move */
}

int
find_short_wand()
{
    register int i;
    register const char *s;

    for (i = WAN_SECRET_DOOR_DETECTION; i <= WAN_LIGHTNING; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "short"))
	    return i;

    // impossible("wand not found?"); //not all wands used
    return -1;	/* not 0, or caller would try again each move */
}

int
find_runed_wand()
{
    register int i;
    register const char *s;

    for (i = WAN_SECRET_DOOR_DETECTION; i <= WAN_LIGHTNING; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "runed"))
	    return i;

    // impossible("wand not found?"); //not all wands used
    return -1;	/* not 0, or caller would try again each move */
}

int
find_long_wand()
{
    register int i;
    register const char *s;

    for (i = WAN_SECRET_DOOR_DETECTION; i <= WAN_LIGHTNING; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "long"))
	    return i;

    // impossible("wand not found?"); //not all wands used
    return -1;	/* not 0, or caller would try again each move */
}

int
find_curved_wand()
{
    register int i;
    register const char *s;

    for (i = WAN_SECRET_DOOR_DETECTION; i <= WAN_LIGHTNING; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "curved"))
	    return i;

    // impossible("wand not found?"); //not all wands used
    return -1;	/* not 0, or caller would try again each move */
}

int
find_forked_wand()
{
    register int i;
    register const char *s;

    for (i = WAN_SECRET_DOOR_DETECTION; i <= WAN_LIGHTNING; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "forked"))
	    return i;

    // impossible("wand not found?"); //not all wands used
    return -1;	/* not 0, or caller would try again each move */
}

int
find_spiked_wand()
{
    register int i;
    register const char *s;

    for (i = WAN_SECRET_DOOR_DETECTION; i <= WAN_LIGHTNING; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "spiked"))
	    return i;

    // impossible("wand not found?"); //not all wands used
    return -1;	/* not 0, or caller would try again each move */
}

int
find_jeweled_wand()
{
    register int i;
    register const char *s;

    for (i = WAN_SECRET_DOOR_DETECTION; i <= WAN_LIGHTNING; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "jeweled"))
	    return i;

    // impossible("wand not found?"); //not all wands used
    return -1;	/* not 0, or caller would try again each move */
}

/* find the first wand starting from otype that is compatable with a given material*/
int
matWand(otyp, mat)
int otyp;
int mat;
{
    int i, j, ctyp;
	
	static int matNeutralWands[7] = {0,0,0, 0,0,0, 0}; 
	if(!matNeutralWands[0]){ 
		matNeutralWands[0] = find_hexagonal_wand(); 
		matNeutralWands[1] = find_short_wand(); 
		matNeutralWands[2] = find_runed_wand();
		
		matNeutralWands[3] = find_long_wand();
		matNeutralWands[4] = find_curved_wand();
		matNeutralWands[5] = find_spiked_wand();
		
		matNeutralWands[6] = find_jeweled_wand();
	}
	
	for(i = WAN_SECRET_DOOR_DETECTION; i <= WAN_LIGHTNING; i++){
		ctyp = WAN_SECRET_DOOR_DETECTION + (((otyp+i)-WAN_SECRET_DOOR_DETECTION)%(WAN_LIGHTNING-WAN_SECRET_DOOR_DETECTION));
		if(objects[ctyp].oc_material == mat)
			return ctyp;
		for(j = 0; j < 7; j++)
			if(matNeutralWands[j] == ctyp)
				return ctyp;
	}
	
	//should never reach this
    return otyp;
}

/* find the object index for old gloves */
int
find_ogloves()
{
    register int i;
    register const char *s;

    for (i = GLOVES; i <= GAUNTLETS_OF_DEXTERITY; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "old gloves"))
	    return i;

    impossible("old gloves not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for fighting gloves */
int
find_tgloves()
{
    register int i;
    register const char *s;

    for (i = GLOVES; i <= GAUNTLETS_OF_DEXTERITY; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "fighting gloves"))
	    return i;

    impossible("fighting gloves not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for padded gloves */
int
find_pgloves()
{
    register int i;
    register const char *s;

    for (i = GLOVES; i <= GAUNTLETS_OF_DEXTERITY; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "padded gloves"))
	    return i;

    impossible("padded gloves not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for fencing gloves */
int
find_fgloves()
{
    register int i;
    register const char *s;

    for (i = GLOVES; i <= GAUNTLETS_OF_DEXTERITY; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "fencing gloves"))
	    return i;

    impossible("fencing gloves not found?");
    return -1;	/* not 0, or caller would try again each move */
}

/* find the object index for piece of cloth robe */
int
find_pcloth()
{
    register int i;
    register const char *s;

    for (i = CLOAK_OF_PROTECTION; i <= CLOAK_OF_DISPLACEMENT; i++)
	if ((s = OBJ_DESCR(objects[i])) != 0 && !strcmp(s, "piece of cloth"))
	    return i;

    impossible("piece of cloth not found?");
    return -1;	/* not 0, or caller would try again each move */
}

void
oinit()			/* level dependent initialization */
{
	setgemprobs(&u.uz);
}

void
savenames(fd, mode)
int fd, mode;
{
	register int i;
	unsigned int len;

	if (perform_bwrite(mode)) {
	    bwrite(fd, (genericptr_t)bases, sizeof bases);
	    bwrite(fd, (genericptr_t)disco, sizeof disco);
	    bwrite(fd, (genericptr_t)objects,
		   sizeof(struct objclass) * NUM_OBJECTS);
	}
	/* as long as we use only one version of Hack we
	   need not save oc_name and oc_descr, but we must save
	   oc_uname for all objects */
	for (i = 0; i < NUM_OBJECTS; i++)
	    if (objects[i].oc_uname) {
		if (perform_bwrite(mode)) {
		    len = strlen(objects[i].oc_uname)+1;
		    bwrite(fd, (genericptr_t)&len, sizeof len);
		    bwrite(fd, (genericptr_t)objects[i].oc_uname, len);
		}
		if (release_data(mode)) {
		    free((genericptr_t)objects[i].oc_uname);
		    objects[i].oc_uname = 0;
		}
	    }
}

void
restnames(fd)
register int fd;
{
	register int i;
	unsigned int len;

	mread(fd, (genericptr_t) bases, sizeof bases);
	mread(fd, (genericptr_t) disco, sizeof disco);
	mread(fd, (genericptr_t) objects, sizeof(struct objclass) * NUM_OBJECTS);
	for (i = 0; i < NUM_OBJECTS; i++)
	    if (objects[i].oc_uname) {
		mread(fd, (genericptr_t) &len, sizeof len);
		objects[i].oc_uname = (char *) alloc(len);
		mread(fd, (genericptr_t)objects[i].oc_uname, len);
	    }
#ifdef USE_TILES
	shuffle_tiles();
#endif
}

void
discover_object(oindx, mark_as_known, credit_hero)
register int oindx;
boolean mark_as_known;
boolean credit_hero;
{
    if (!objects[oindx].oc_name_known) {
	register int dindx, acls = objects[oindx].oc_class;

	/* Loop thru disco[] 'til we find the target (which may have been
	   uname'd) or the next open slot; one or the other will be found
	   before we reach the next class...
	 */
	for (dindx = bases[acls]; disco[dindx] != 0; dindx++)
	    if (disco[dindx] == oindx) break;
	disco[dindx] = oindx;

	if (mark_as_known) {
	    objects[oindx].oc_name_known = 1;
	    if (credit_hero) exercise(A_WIS, TRUE);
	}
	if (moves > 1L) update_inventory();
    }
}

/* if a class name has been cleared, we may need to purge it from disco[] */
void
undiscover_object(oindx)
register int oindx;
{
    if (!objects[oindx].oc_name_known) {
	register int dindx, acls = objects[oindx].oc_class;
	register boolean found = FALSE;

	/* find the object; shift those behind it forward one slot */
	for (dindx = bases[acls];
	      dindx < NUM_OBJECTS && disco[dindx] != 0
		&& objects[dindx].oc_class == acls; dindx++)
	    if (found)
		disco[dindx-1] = disco[dindx];
	    else if (disco[dindx] == oindx)
		found = TRUE;

	/* clear last slot */
	if (found) disco[dindx-1] = 0;
	/*else impossible("named object not in disco"); Papering over a bug: something about repeatedly using the amnesia code
		seems to be tripping this. However, the bug seems harmless so I'm removing the error message. Famous last words. */
	update_inventory();
    }
}

STATIC_OVL boolean
interesting_to_discover(i)
register int i;
{
	/* Pre-discovered objects are now printed with a '*' */
    return((boolean)(objects[i].oc_uname != (char *)0 ||
	    (objects[i].oc_name_known && OBJ_DESCR(objects[i]) != (char *)0)));
}

/* items that should stand out once they're known */
static short uniq_objs[] = {
	AMULET_OF_YENDOR,
	SPE_BOOK_OF_THE_DEAD,
	CANDELABRUM_OF_INVOCATION,
	BELL_OF_OPENING,
};

int
dodiscovered()				/* free after Robert Viduya */
{
    register int i, dis;
    int	ct = 0;
    char *s, oclass, prev_class, classes[MAXOCLASSES];
    winid tmpwin;
	char buf[BUFSZ];

    tmpwin = create_nhwindow(NHW_MENU);
    putstr(tmpwin, 0, "Discoveries");
    putstr(tmpwin, 0, "");

    /* gather "unique objects" into a pseudo-class; note that they'll
       also be displayed individually within their regular class */
    for (i = dis = 0; i < SIZE(uniq_objs); i++)
	if (objects[uniq_objs[i]].oc_name_known) {
	    if (!dis++)
		putstr(tmpwin, iflags.menu_headings, "Unique Items");
		Sprintf(buf, "  %s", OBJ_NAME(objects[uniq_objs[i]]));
	    putstr(tmpwin, 0, buf);
	    ++ct;
	}
    /* display any known artifacts as another pseudo-class */
    ct += disp_artifact_discoveries(tmpwin);

    /* several classes are omitted from packorder; one is of interest here */
    Strcpy(classes, flags.inv_order);
    if (!index(classes, VENOM_CLASS)) {
	s = eos(classes);
	*s++ = VENOM_CLASS;
	*s = '\0';
    }

    for (s = classes; *s; s++) {
	oclass = *s;
	prev_class = oclass + 1;	/* forced different from oclass */
	for (i = bases[(int)oclass];
	     i < NUM_OBJECTS && objects[i].oc_class == oclass; i++) {
	    if ((dis = disco[i]) && interesting_to_discover(dis)) {
		ct++;
		if (oclass != prev_class) {
		    putstr(tmpwin, iflags.menu_headings, let_to_name(oclass, FALSE, FALSE));
		    prev_class = oclass;
		}
		Sprintf(buf, "%s %s",(objects[dis].oc_pre_discovered ? "*" : " "),
				obj_typename(dis));
		putstr(tmpwin, 0, buf);
	    }
	}
    }
    if (ct == 0) {
	You("haven't discovered anything yet...");
    } else
	display_nhwindow(tmpwin, TRUE);
    destroy_nhwindow(tmpwin);

    return 0;
}

int
object_color(otmp)
struct obj *otmp;
{
	const char * s;

	/* should never happen */
	if (!otmp)
	{
		impossible("object_color called with no object");
		return 0;
	}
	/* fake mimic objects use the default color 
	 * they also have unset data in too many places
	 */
	if (otmp->oclass == STRANGE_OBJECT)
	{
		return objects[otmp->otyp].oc_color;
	}


	/* artifacts with set colors */
	switch (otmp->oartifact)
	{
	case ART_BLACK_CRYSTAL:		return mvitals[PM_CHAOS].died ? CLR_BLACK : HI_GLASS;
	case ART_EARTH_CRYSTAL:		return mvitals[PM_LICH__THE_FIEND_OF_EARTH].died ? 
										CLR_YELLOW : CLR_BROWN;
	case ART_FIRE_CRYSTAL:		return  mvitals[PM_KARY__THE_FIEND_OF_FIRE].died ? 
										CLR_ORANGE : CLR_RED;
	case ART_WATER_CRYSTAL:		return  mvitals[PM_KRAKEN__THE_FIEND_OF_WATER].died ? 
										CLR_BRIGHT_BLUE : CLR_BLUE;
	case ART_AIR_CRYSTAL:		return  mvitals[PM_TIAMAT__THE_FIEND_OF_WIND].died ? 
										CLR_BRIGHT_GREEN : CLR_GREEN;
	case ART_PRAYER_WARDED_WRAPPINGS_OF:		return CLR_BLACK;
	}
	/* gold pieces are yellow */
	if (otmp->otyp == GOLD_PIECE)
		return CLR_YELLOW;

	/* plumed helmets and etched helmets get fancy colors, but only if their material is boring (iron/metal) */
	if ((otmp->obj_material == IRON || otmp->obj_material == METAL)
		&& ((s = OBJ_DESCR(objects[otmp->otyp])) != (char *)0 && !strncmpi(s, "plumed", 6)))
		return CLR_RED;
	if ((otmp->obj_material == IRON || otmp->obj_material == METAL)
		&& ((s = OBJ_DESCR(objects[otmp->otyp])) != (char *)0 && !strncmpi(s, "etched", 6)))
		return CLR_BRIGHT_GREEN;

	/* objects with non-standard materials whose base color is that of their material */
	if (otmp->obj_material != objects[otmp->otyp].oc_material
		&& materials[objects[otmp->otyp].oc_material].color == objects[otmp->otyp].oc_color)
	{
		/* Fancy gem colors */
		if (otmp->obj_material == GEMSTONE && otmp->ovar1 && !obj_type_uses_ovar1(otmp) && !obj_art_uses_ovar1(otmp))
			return objects[otmp->ovar1].oc_color;
		/* Dragon hide/bone discrepancy -- dragonhide should be leather colored, not bone colored.
		 * We are neglecting dragon scales, since there currently no circumstances where that happens
		 * as it requires the base material to be dragonhide, and this code-block applies to modified mats
		 */
		if (otmp->obj_material == DRAGON_HIDE && !strncmpi(material_name(otmp, TRUE), "dragonhide", 10))
				return HI_LEATHER;
		
		return materials[otmp->obj_material].color;
	}
	/* default color */
	return objects[otmp->otyp].oc_color;
}

void
fix_object(otmp)
	struct obj *otmp;
{
	otmp->owt = weight(otmp);
}

/*o_init.c*/
