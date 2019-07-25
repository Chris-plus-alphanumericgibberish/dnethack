/*	SCCS Id: @(#)mon.c	3.4	2003/12/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* If you're using precompiled headers, you don't want this either */
#ifdef MICROPORT_BUG
#define MKROOM_H
#endif

 //Note: math.h must be included before hack.h bc it contains a yn() macro that is incompatible with the one in hack
#include <math.h>

#include "hack.h"
#include "mfndpos.h"
#include "edog.h"
#include "artifact.h"
#include <ctype.h>
#include <stdlib.h>


STATIC_DCL boolean FDECL(restrap,(struct monst *));
STATIC_DCL int FDECL(scent_callback,(genericptr_t, int, int));
int scentgoalx, scentgoaly;

#ifdef OVL2
STATIC_DCL int NDECL(pick_animal);
STATIC_DCL int FDECL(select_newcham_form, (struct monst *));
STATIC_DCL void FDECL(kill_eggs, (struct obj *));
#endif

#ifdef REINCARNATION
#define LEVEL_SPECIFIC_NOCORPSE(mdat) \
	 ((Is_rogue_level(&u.uz) || \
	   (level.flags.graveyard && is_undead(mdat) && rn2(3))) \
	   && mdat!=&mons[PM_GARO] && mdat!=&mons[PM_GARO_MASTER])
#else
#define LEVEL_SPECIFIC_NOCORPSE(mdat) \
	   (level.flags.graveyard && is_undead(mdat) && rn2(3) \
	   && mdat!=&mons[PM_GARO] && mdat!=&mons[PM_GARO_MASTER])
#endif


#if 0
/* part of the original warning code which was replaced in 3.3.1 */
#ifdef OVL1
#define warnDelay 10
long lastwarntime;
int lastwarnlev;

const char *warnings[] = {
	"white", "pink", "red", "ruby", "purple", "black"
};

STATIC_DCL void NDECL(warn_effects);
#endif /* OVL1 */
#endif /* 0 */

#ifndef OVLB
STATIC_VAR int cham_to_pm[];
#else
STATIC_DCL struct obj *FDECL(make_corpse,(struct monst *));
STATIC_DCL void FDECL(m_detach, (struct monst *, struct permonst *));
STATIC_DCL void FDECL(lifesaved_monster, (struct monst *));

STATIC_DCL double FDECL(atanGerald, (double x));

STATIC_OVL double 
atanGerald(x)
double x;
{
	double temp1 = x >= 0 ? x : -x;
	double temp2 = temp1 <= 1.0 ? temp1 : (temp1 - 1) / (temp1 + 1);
	double sum = temp2;
	size_t i;
	for (i = 1; i != 6; ++i)
		sum += (i % 2 ? -1 : 1) * pow(temp2, (i << 1) + 1) / ((i << 1) + 1);

	if (temp1 > 1.0) sum += 0.785398;
	return x >= 0 ? sum : -sum;
}

void
removeMonster(x,y)
int x,y;
{
    if (level.monsters[x][y] &&
	opaque(level.monsters[x][y]->data) &&
	 (!level.monsters[x][y]->minvis || See_invisible(x,y)))
		unblock_point(x,y);
    level.monsters[x][y] = (struct monst *)0;
}

/* convert the monster index of an undead to its living counterpart */
int
undead_to_corpse(mndx)
int mndx;
{
	switch (mndx) {
	// case PM_KOBOLD_ZOMBIE:
	case PM_KOBOLD_MUMMY:	mndx = PM_KOBOLD;  break;
	// case PM_DWARF_ZOMBIE:
	case PM_DWARF_MUMMY:	mndx = PM_DWARF;  break;
	// case PM_GNOME_ZOMBIE:
	case PM_GNOME_MUMMY:	mndx = PM_GNOME;  break;
	// case PM_ORC_ZOMBIE:
	case PM_ORC_MUMMY:	mndx = PM_ORC;  break;
	// case PM_ELF_ZOMBIE:
	case PM_ELF_MUMMY:	mndx = PM_ELF;  break;
	case PM_VAMPIRE:
	case PM_VAMPIRE_LORD:
#if 0	/* DEFERRED */
	case PM_VAMPIRE_MAGE:
#endif
	// case PM_HUMAN_ZOMBIE:
	case PM_ZOMBIE:
	case PM_HUMAN_MUMMY:	mndx = PM_HUMAN;  break;
	case PM_HEDROW_ZOMBIE:	mndx = PM_DROW;  break;
	case PM_DROW_MUMMY:	mndx = PM_DROW_MATRON;  break;
	// case PM_GIANT_ZOMBIE:
	case PM_GIANT_MUMMY:	mndx = PM_GIANT;  break;
	// case PM_ETTIN_ZOMBIE:
	case PM_ETTIN_MUMMY:	mndx = PM_ETTIN;  break;
	case PM_ALABASTER_MUMMY: mndx = PM_ALABASTER_ELF_ELDER;  break;
	default:  break;
	}
	return mndx;
}

/* Convert the monster index of some monsters (such as quest guardians)
 * to their generic species type.
 *
 * Return associated character class monster, rather than species
 * if mode is 1.
 */
int
genus(mndx, mode)
int mndx, mode;
{
	switch (mndx) {
/* Quest guardians */
	case PM_STUDENT:     mndx = mode ? PM_ARCHEOLOGIST  : PM_HUMAN; break;
	case PM_CHIEFTAIN:   mndx = mode ? PM_BARBARIAN : PM_HUMAN; break;
#ifdef BARD
	case PM_RHYMER:      mndx = mode ? PM_BARD   : PM_HUMAN; break;
#endif
	case PM_NEANDERTHAL: mndx = mode ? PM_CAVEMAN   : PM_HUMAN; break;
	case PM_ATTENDANT:   mndx = mode ? PM_HEALER    : PM_HUMAN; break;
	case PM_PAGE:        mndx = mode ? PM_KNIGHT    : PM_HUMAN; break;
	case PM_ABBOT:       mndx = mode ? PM_MONK      : PM_HUMAN; break;
	case PM_ACOLYTE:     mndx = mode ? PM_PRIEST    : PM_HUMAN; break;
	case PM_HUNTER:      mndx = mode ? PM_RANGER    : PM_HUMAN; break;
	case PM_THUG:        mndx = mode ? PM_ROGUE     : PM_HUMAN; break;
	case PM_ROSHI:       mndx = mode ? PM_SAMURAI   : PM_HUMAN; break;
#ifdef TOURIST
	case PM_GUIDE:       mndx = mode ? PM_TOURIST   : PM_HUMAN; break;
#endif
	case PM_APPRENTICE:  mndx = mode ? PM_WIZARD    : PM_HUMAN; break;
	case PM_WARRIOR:     mndx = mode ? PM_VALKYRIE  : PM_HUMAN; break;
	default:
		if (mndx >= LOW_PM && mndx < NUMMONS) {
			struct permonst *ptr = &mons[mndx];
			if (is_human(ptr))      mndx = PM_HUMAN;
			else if (is_elf(ptr))   mndx = PM_ELF;
			else if (is_dwarf(ptr)) mndx = PM_DWARF;
			else if (is_gnome(ptr)) mndx = PM_GNOME;
			else if (is_orc(ptr))   mndx = PM_ORC;
		}
		break;
	}
	return mndx;
}

/* convert monster index to chameleon index */
int
pm_to_cham(mndx)
int mndx;
{
	int mcham;

	switch (mndx) {
	case PM_CHAMELEON:	mcham = CHAM_CHAMELEON; break;
	case PM_DOPPELGANGER:	mcham = CHAM_DOPPELGANGER; break;
	case PM_SANDESTIN:	mcham = CHAM_SANDESTIN; break;
	case PM_DREAM_QUASIELEMENTAL:	mcham = CHAM_DREAM; break;
	default: mcham = CHAM_ORDINARY; break;
	}
	return mcham;
}

/* convert chameleon index to monster index */
STATIC_VAR int cham_to_pm[] = {
		NON_PM,		/* placeholder for CHAM_ORDINARY */
		PM_CHAMELEON,
		PM_DOPPELGANGER,
		PM_SANDESTIN,
		PM_DREAM_QUASIELEMENTAL,
};

/* for deciding whether corpse or statue will carry along full monster data */
#define KEEPTRAITS(mon)	((mon)->isshk || (mon)->mtame ||		\
			 ((mon)->data->geno & G_UNIQ) ||		\
			 is_reviver((mon)->data) ||			\
			 ((mon)->mfaction == ZOMBIFIED) ||			\
			 ((mon)->mfaction == VAMPIRIC) ||			\
			 ((mon)->zombify) ||			\
			 ((mon)->data == &mons[PM_UNDEAD_KNIGHT]) ||			\
			 ((mon)->data == &mons[PM_WARRIOR_OF_SUNLIGHT]) ||			\
			 /* normally leader the will be unique, */	\
			 /* but he might have been polymorphed  */	\
			 (mon)->m_id == quest_status.leader_m_id ||	\
			 /* special cancellation handling for these */	\
			 (dmgtype((mon)->data, AD_SEDU) ||		\
			  dmgtype((mon)->data, AD_SSEX)))

			  
const int humanoid_eyes[] = {
	PM_HOBBIT,
	PM_DWARF,
	PM_ORC,
	PM_GNOME,
	PM_APE,
	PM_HUMAN,
	PM_ELF,
	PM_MYRKALFR,
	PM_HALF_DRAGON,
	PM_SHEEP,
	PM_HOUSECAT,
	PM_DOG,
	PM_HORSE
};
/* Creates a monster corpse, a "special" corpse, or nothing if it doesn't
 * leave corpses.  Monsters which leave "special" corpses should have
 * G_NOCORPSE set in order to prevent wishing for one, finding tins of one,
 * etc....
 */
STATIC_OVL struct obj *
make_corpse(mtmp)
register struct monst *mtmp;
{
	register struct permonst *mdat = mtmp->data;
	int num;
	struct obj *obj = (struct obj *)0;
	int x = mtmp->mx, y = mtmp->my;
	int mndx = monsndx(mdat);
	struct obj *otmp;
	
	if(mtmp->mvanishes > -1){
		return (struct obj *)0;
	}
	
	if(u.specialSealsActive&SEAL_NUDZIRATH && !rn2(4)){
		(void) mksobj_at(MIRROR, x, y, TRUE, FALSE);
	}
	
	if(mtmp->mfaction == CRYSTALFIED){
		obj = mkcorpstat(STATUE, (struct monst *)0,
			mdat, x, y, FALSE);
		obj->obj_material = GLASS;
		fix_object(obj);
	} else switch(mndx) {
	    case PM_LICH__THE_FIEND_OF_EARTH:
			// if(mvitals[PM_GARLAND].died){
				// otmp = mksobj_at(CRYSTAL_BALL, x, y, FALSE, FALSE);
				// otmp = oname(otmp, artiname(ART_EARTH_CRYSTAL));		
				// curse(otmp);
				// otmp->oerodeproof = TRUE;
			// }
		goto default_1;
		break;
	    case PM_KARY__THE_FIEND_OF_FIRE:
			// if(mvitals[PM_GARLAND].died){
				// otmp = mksobj_at(CRYSTAL_BALL, x, y, FALSE, FALSE);
				// otmp = oname(otmp, artiname(ART_FIRE_CRYSTAL));		
				// curse(otmp);
				// otmp->oerodeproof = TRUE;
			// }
		goto default_1;
		break;
	    case PM_KRAKEN__THE_FIEND_OF_WATER:
			// if(mvitals[PM_GARLAND].died){
				// otmp = mksobj_at(CRYSTAL_BALL, x, y, FALSE, FALSE);
				// otmp = oname(otmp, artiname(ART_WATER_CRYSTAL));		
				// curse(otmp);
				// otmp->oerodeproof = TRUE;
			// }
		goto default_1;
		break;
	    case PM_TIAMAT__THE_FIEND_OF_WIND:
			// if(mvitals[PM_GARLAND].died){
				// otmp = mksobj_at(CRYSTAL_BALL, x, y, FALSE, FALSE);
				// otmp = oname(otmp, artiname(ART_AIR_CRYSTAL));		
				// curse(otmp);
				// otmp->oerodeproof = TRUE;
			// }
		goto default_1;
		break;
	    case PM_CHAOS:
			// otmp = mksobj_at(CRYSTAL_BALL, x, y, FALSE, FALSE);
			// otmp = oname(otmp, artiname(ART_BLACK_CRYSTAL));		
			// curse(otmp);
			// otmp->oerodeproof = TRUE;
		goto default_1;
		break;
	    case PM_GRAY_DRAGON:
	    case PM_SILVER_DRAGON:
	    case PM_SHIMMERING_DRAGON:
	    case PM_RED_DRAGON:
	    case PM_ORANGE_DRAGON:
	    case PM_WHITE_DRAGON:
	    case PM_BLACK_DRAGON:
	    case PM_BLUE_DRAGON:
	    case PM_GREEN_DRAGON:
	    case PM_YELLOW_DRAGON:
		/* Make dragon scales.  This assumes that the order of the */
		/* dragons is the same as the order of the scales.	   */
		if (!rn2(mtmp->mrevived ? 20 : 3)) {
		    num = GRAY_DRAGON_SCALES + monsndx(mdat) - PM_GRAY_DRAGON;
		    obj = mksobj_at(num, x, y, FALSE, FALSE);
		    obj->spe = 0;
		    obj->cursed = obj->blessed = FALSE;
		}
		goto default_1;
	    case PM_CHROMATIC_DRAGON:
		    obj = mksobj_at(BLACK_DRAGON_SCALES, x, y, FALSE, FALSE);
			obj = oname(obj, artiname(ART_CHROMATIC_DRAGON_SCALES));
		goto default_1;
	    case PM_PLATINUM_DRAGON:
		    obj = mksobj_at(SILVER_DRAGON_SCALE_MAIL, x, y, FALSE, FALSE);
			obj = oname(obj, artiname(ART_DRAGON_PLATE));
		goto default_1;
	    case PM_MANTICORE:
		if (mtmp->mrevived ? !rn2(6) : TRUE) {
			obj = mksobj_at(SPIKE, x, y, TRUE, FALSE);
			obj->blessed = 0;
			obj->cursed = 0;
			obj->quan = d(4,6);
			obj->spe = 0;
			obj->opoisoned = (OPOISON_PARAL);
		}
		goto default_1;
	    case PM_SON_OF_TYPHON:
		if (!rn2(mtmp->mrevived ? 20 : 3)) {
			obj = mksobj_at(LEO_NEMAEUS_HIDE, x, y, FALSE, FALSE);
		    obj->spe = 0;
		    obj->cursed = obj->blessed = FALSE;
		}
		goto default_1;

	    case PM_WATER_ELEMENTAL:
		if (levl[mtmp->mx][mtmp->my].typ == ROOM) {
		    levl[mtmp->mx][mtmp->my].typ = PUDDLE;
		    water_damage(level.objects[mtmp->mx][mtmp->my], FALSE, TRUE, level.flags.lethe, 0);
		}
		goto default_1;

	    case PM_SARA__THE_LAST_ORACLE:
		if (mtmp->mrevived) {
			if (canseemon(mtmp))
			   pline("%s recently returned eyes vanish once more.",
				s_suffix(Monnam(mtmp)));
		} else {
			if (canseemon(mtmp))
			   pline("%s eyes vanish.",
				s_suffix(Monnam(mtmp)));
		}
		goto default_1;
	    case PM_ORACLE:
		if (mtmp->mrevived) {
			if (canseemon(mtmp))
			   pline("%s recently regrown eyes crumble to dust.",
				s_suffix(Monnam(mtmp)));
		} else {
			if (canseemon(mtmp))
			   pline("%s eyes crumble to dust.",
				s_suffix(Monnam(mtmp)));
		}
		goto default_1;
	    case PM_WHITE_UNICORN:
	    case PM_GRAY_UNICORN:
	    case PM_BLACK_UNICORN:
		if (mtmp->mrevived && rn2(20)) {
			if (canseemon(mtmp))
			   pline("%s recently regrown horn crumbles to dust.",
				s_suffix(Monnam(mtmp)));
		} else
			(void) mksobj_at(UNICORN_HORN, x, y, TRUE, FALSE);
		goto default_1;
//		case PM_UNICORN_OF_AMBER:{
//				int spe2;
			    /* create special stuff; can't use mongets */
//			    otmp = mksobj(UNICORN_HORN, TRUE, FALSE);
//				otmp = oname(otmp, artiname(ART_AMBER_HORN));		
//			    curse(otmp);
//			    otmp->oerodeproof = TRUE;
//			    spe2 = rn2(4)-3;
//			    otmp->spe = spe2;
//				place_object(otmp, x, y);
//			}
//		goto default_1;
//		case PM_PINK_UNICORN:{
//			int spe2;
		    /* create special stuff; can't use mongets */
//		    otmp = mksobj(UNICORN_HORN, TRUE, FALSE);
//			otmp = oname(otmp, artiname(ART_WHITE_PINK_HORN));
//			
//		    curse(otmp);
//		    otmp->oerodeproof = TRUE;
//		    spe2 = rn2(4)-3;
//		    otmp->spe = spe2;
//			place_object(otmp, x, y);
//		  }
//		goto default_1;
		case PM_NIGHTMARE:
			{
			int spe2;
		    /* create special stuff; can't use mongets */
		    otmp = mksobj(UNICORN_HORN, TRUE, FALSE);
			otmp = oname(otmp, artiname(ART_NIGHTHORN));
		
		    curse(otmp);
		    otmp->oerodeproof = TRUE;
		    spe2 = rn2(4)-3;
		    otmp->spe = spe2;
			place_object(otmp, x, y);
			}
		goto default_1;
		case PM_VECNA:
			{
			if(!rn2(2)){
				pline("All that remains is a hand...");
				otmp = oname(mksobj(SEVERED_HAND, TRUE, FALSE),
						artiname(ART_HAND_OF_VECNA));
			} else {
				pline("All that remains is a single eye...");
				otmp = oname(mksobj(EYEBALL, TRUE, FALSE),
						artiname(ART_EYE_OF_VECNA));
			}
		    /* create special stuff; can't use mongets */
			
		    curse(otmp);
		    otmp->oerodeproof = TRUE;
			place_object(otmp, x, y);
			}
		goto default_1;
	    case PM_LONG_WORM:
			(void) mksobj_at(WORM_TOOTH, x, y, TRUE, FALSE);
		goto default_1;
	    case PM_VAMPIRE:
	    case PM_VAMPIRE_LORD:
		/* include mtmp in the mkcorpstat() call */
		num = undead_to_corpse(mndx);
		obj = mkcorpstat(CORPSE, mtmp, &mons[num], x, y, TRUE);
		obj->age -= 100;		/* this is an *OLD* corpse */
		break;
	    case PM_KOBOLD_MUMMY:
	    case PM_DWARF_MUMMY:
	    case PM_GNOME_MUMMY:
	    case PM_ORC_MUMMY:
	    case PM_ELF_MUMMY:
	    case PM_HUMAN_MUMMY:
	    case PM_DROW_MUMMY:
	    case PM_HALF_DRAGON_MUMMY:
	    case PM_GIANT_MUMMY:
	    case PM_ETTIN_MUMMY:
	    // case PM_KOBOLD_ZOMBIE:
	    // case PM_DWARF_ZOMBIE:
	    // case PM_GNOME_ZOMBIE:
	    // case PM_ORC_ZOMBIE:
	    // case PM_ELF_ZOMBIE:
	    // case PM_HUMAN_ZOMBIE:
	    case PM_ZOMBIE:
	    case PM_HEDROW_ZOMBIE:
	    // case PM_HALF_DRAGON_ZOMBIE:
	    // case PM_GIANT_ZOMBIE:
	    // case PM_ETTIN_ZOMBIE:
	    case PM_ALABASTER_MUMMY:
		if(is_alabaster_mummy(mtmp->data) && mtmp->mvar1 >= SYLLABLE_OF_STRENGTH__AESH && mtmp->mvar1 <= SYLLABLE_OF_SPIRIT__VAUL){
			mksobj_at(mtmp->mvar1, x, y, TRUE, FALSE);
			if(mtmp->mvar1 == SYLLABLE_OF_SPIRIT__VAUL)
				mtmp->mintrinsics[(DISPLACED-1)/32] &= ~(1 << (DISPLACED-1)%32);
			mtmp->mvar1 = 0; //Lose the bonus if resurrected
		}
		num = undead_to_corpse(mndx);
		obj = mkcorpstat(CORPSE, mtmp, &mons[num], x, y, TRUE);
		break;
	    case PM_ARSENAL:
			num = d(3,6);
			while(num--){
				obj = mksobj_at(PLATE_MAIL, x, y, TRUE, FALSE);
				obj->spe = 3;
				obj->obj_material = COPPER;
				fix_object(obj);
			}
			num = d(2,4);
			while(num--)
				obj = mksobj_at(HEAVY_IRON_BALL, x, y, TRUE, FALSE);
			mtmp->mnamelth = 0;
		    otmp = mksobj(MACE, TRUE, FALSE);
			otmp = oname(otmp, artiname(ART_FIELD_MARSHAL_S_BATON));
		    otmp->oerodeproof = TRUE;
		    otmp->spe = -3;
			place_object(otmp, x, y);
		break;
	    case PM_TINKER_GNOME:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(1,4);
			obj->owt = weight(obj);
			if(!mtmp->mrevived && !rn2(20)){
				obj = mksobj_at(UPGRADE_KIT, x, y, TRUE, FALSE);
			} else if(!mtmp->mrevived && !rn2(19)){
				obj = mksobj_at(TINNING_KIT, x, y, TRUE, FALSE);
			} else if(!mtmp->mrevived && !rn2(10)){
				obj = mksobj_at(CAN_OF_GREASE, x, y, TRUE, FALSE);
			}
			mtmp->mnamelth = 0;
		goto default_1;
	    case PM_CLOCKWORK_DWARF:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(1,4);
			obj->owt = weight(obj);
			mtmp->mnamelth = 0;
		break;
	    case PM_FABERGE_SPHERE:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(1,3);
			obj->owt = weight(obj);
			mtmp->mnamelth = 0;
		break;
	    case PM_FIREWORK_CART:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(1,4);
			obj->owt = weight(obj);
			mtmp->mnamelth = 0;
		break;
	    case PM_CLOCKWORK_SOLDIER:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(1,3);
			obj->owt = weight(obj);
			mtmp->mnamelth = 0;
		break;
	    case PM_GOLDEN_HEART:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(1,4);
			obj->owt = weight(obj);
			obj = mksobj_at(SUBETHAIC_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = 1;
			obj->owt = weight(obj);
			mtmp->mnamelth = 0;
		break;
	    case PM_JUGGERNAUT:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(3,4);
			obj->owt = weight(obj);
			if(!rn2(20)){
				obj = mksobj_at(TINNING_KIT, x, y, TRUE, FALSE);
			} else if(!rn2(10)){
				obj = mksobj_at(CAN_OF_GREASE, x, y, TRUE, FALSE);
			}
			mtmp->mnamelth = 0;
		break;
	    case PM_CLOCKWORK_FACTORY:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(3,4);
			obj->owt = weight(obj);
			if(!rn2(20)){
				obj = mksobj_at(UPGRADE_KIT, x, y, TRUE, FALSE);
			} else if(!rn2(19)){
				obj = mksobj_at(TINNING_KIT, x, y, TRUE, FALSE);
			} else if(!rn2(10)){
				obj = mksobj_at(CAN_OF_GREASE, x, y, TRUE, FALSE);
			}
			mtmp->mnamelth = 0;
		break;
	    case PM_ID_JUGGERNAUT:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(4,4);
			obj->owt = weight(obj);
			obj = mksobj_at(SUBETHAIC_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(1,4);
			obj->owt = weight(obj);
			mtmp->mnamelth = 0;
		break;
	    case PM_SCRAP_TITAN:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(4,4);
			obj->owt = weight(obj);
			mtmp->mnamelth = 0;
			num = d(2,4);
			while (num--){
				obj = mksobj_at(CHAIN, x, y, TRUE, FALSE);
				obj->oeroded = 3;
				obj = mksobj_at(CHAIN, x, y, TRUE, FALSE);
				obj->oeroded = 3;
				obj = mksobj_at(BAR, x, y, TRUE, FALSE);
				obj->oeroded = 3;
				obj = mksobj_at(SCRAP, x, y, TRUE, FALSE);
				obj->oeroded = 3;
				obj = mksobj_at(SCRAP, x, y, TRUE, FALSE);
				obj->oeroded = 3;
				obj = mksobj_at(SCRAP, x, y, TRUE, FALSE);
				obj->oeroded = 3;
			}
		break;
	    case PM_HELLFIRE_COLOSSUS:
			obj = mksobj_at(CLOCKWORK_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(4,4);
			obj->owt = weight(obj);
			obj = mksobj_at(HELLFIRE_COMPONENT, x, y, TRUE, FALSE);
			obj->quan = d(4,4);
			obj->owt = weight(obj);
			num = d(2,6);
			while (num--){
				obj = mksobj_at(CHAIN, x, y, TRUE, FALSE);
				obj = mksobj_at(CHAIN, x, y, TRUE, FALSE);
				obj = mksobj_at(BAR, x, y, TRUE, FALSE);
			}
			mtmp->mnamelth = 0;
		break;
	    case PM_ANDROID:
			obj = mksobj_at(BROKEN_ANDROID, x, y, FALSE, FALSE);
		break;
	    case PM_CRUCIFIED_ANDROID:
			obj = mksobj_at(BROKEN_ANDROID, x, y, FALSE, FALSE);
			obj = mksobj_at(BAR, x, y, FALSE, FALSE);
			obj->oeroded = 1;
			obj = mksobj_at(BAR, x, y, FALSE, FALSE);
			obj->oeroded = 1;
		break;
	    case PM_MUMMIFIED_ANDROID:
			obj = mksobj_at(BROKEN_ANDROID, x, y, FALSE, FALSE);
		break;
	    case PM_FLAYED_ANDROID:
			obj = mksobj_at(BROKEN_ANDROID, x, y, FALSE, FALSE);
		break;
	    case PM_PARASITIZED_ANDROID:
			obj = mksobj_at(BROKEN_ANDROID, x, y, FALSE, FALSE);
			obj = mksobj_at(CORPSE, x, y, FALSE, FALSE);
			obj->corpsenm = PM_PARASITIC_MIND_FLAYER;
			fix_object(obj);
		break;
	    case PM_GYNOID:
			obj = mksobj_at(BROKEN_GYNOID, x, y, FALSE, FALSE);
		break;
	    case PM_CRUCIFIED_GYNOID:
			obj = mksobj_at(BROKEN_GYNOID, x, y, FALSE, FALSE);
			obj = mksobj_at(BAR, x, y, FALSE, FALSE);
			obj->oeroded = 1;
			obj = mksobj_at(BAR, x, y, FALSE, FALSE);
			obj->oeroded = 1;
		break;
	    case PM_MUMMIFIED_GYNOID:
			obj = mksobj_at(BROKEN_GYNOID, x, y, FALSE, FALSE);
		break;
	    case PM_FLAYED_GYNOID:
			obj = mksobj_at(BROKEN_GYNOID, x, y, FALSE, FALSE);
		break;
	    case PM_PARASITIZED_GYNOID:
			obj = mksobj_at(BROKEN_GYNOID, x, y, FALSE, FALSE);
			obj = mksobj_at(CORPSE, x, y, FALSE, FALSE);
			obj->corpsenm = PM_PARASITIC_MIND_FLAYER;
			fix_object(obj);
		break;
	    case PM_DANCING_BLADE:
			obj = mksobj_at(TWO_HANDED_SWORD, x, y, FALSE, FALSE);
			obj->blessed = TRUE;
			obj->cursed = FALSE;
			obj->spe = 7;
			obj->objsize = MZ_LARGE;
			fix_object(obj);
		break;
	    case PM_IRON_GOLEM:
			num = d(2,6);
			while (num--){
				obj = mksobj_at(CHAIN, x, y, TRUE, FALSE);
				obj = mksobj_at(KITE_SHIELD, x, y, TRUE, FALSE);
				obj = mksobj_at(BAR, x, y, TRUE, FALSE);
			}
			mtmp->mnamelth = 0;
		break;
	    case PM_CHAIN_GOLEM:
			num = d(6,6);
			while (num--){
				obj = mksobj_at(CHAIN, x, y, TRUE, FALSE);
			}
			mtmp->mnamelth = 0;
		break;
	    case PM_ARGENTUM_GOLEM:
			num = d(1,3);
			while (num--){
				obj = mksobj_at(SILVER_SLINGSTONE, x, y, TRUE, FALSE);
				obj->quan = d(10,5);
				obj->owt = weight(obj);
			}
			mtmp->mnamelth = 0;
		break;
	    case PM_GLASS_GOLEM:
			num = d(2,4);   /* very low chance of creating all glass gems */
			while (num--)
				obj = mksobj_at((LAST_GEM + rnd(9)), x, y, TRUE, FALSE);
				mtmp->mnamelth = 0;
		break;
	    case PM_CLAY_GOLEM:
			obj = mksobj_at(ROCK, x, y, FALSE, FALSE);
			obj->quan = (long)(rn2(20) + 50);
			obj->owt = weight(obj);
			mtmp->mnamelth = 0;
		break;
	    case PM_STONE_GOLEM:
			obj = mkcorpstat(STATUE, (struct monst *)0,
				mdat, x, y, FALSE);
		break;
	    case PM_SENTINEL_OF_MITHARDIR:
			obj = mkcorpstat(STATUE, (struct monst *)0,
				mdat, x, y, FALSE);
			if(obj){
				obj->corpsenm = PM_ALABASTER_ELF;
				fix_object(obj);
			}
		break;
	    case PM_WOOD_GOLEM:
			num = d(2,4);
			while(num--) {
				obj = mksobj_at(QUARTERSTAFF, x, y, TRUE, FALSE);
			}
			mtmp->mnamelth = 0;
		break;
	    case PM_GROVE_GUARDIAN:
			num = d(3,4);
			while(num--) {
				obj = mksobj_at(QUARTERSTAFF, x, y, TRUE, FALSE);
				obj->spe = rnd(3);
			}
			mtmp->mnamelth = 0;
		break;
	    case PM_LIVING_LECTERN:
			num = d(2,3);
			while(num--) {
				obj = mksobj_at(CLUB, x, y, TRUE, FALSE);
			}
			obj = mkobj_at(SPBOOK_CLASS, x, y, FALSE);
			mtmp->mnamelth = 0;
		break;
	    case PM_LEATHER_GOLEM:
			num = d(2,4);
			while(num--)
				obj = mksobj_at(LEATHER_ARMOR, x, y, TRUE, FALSE);
			mtmp->mnamelth = 0;
		break;
	    case PM_GOLD_GOLEM:
			/* Good luck gives more coins */
			obj = mkgold((long)(200 - rnl(101)), x, y);
			mtmp->mnamelth = 0;
			break;
	    case PM_ARA_KAMEREL:
			/* Ara Kamerel are projecting their images into gold golems */
			obj = mkgold((long)(200 - rnl(101)), x, y);
			mtmp->mnamelth = 0;
			break;
		case PM_TREASURY_GOLEM:
			num = d(2,4); 
			while (num--)
				obj = mksobj_at((LAST_GEM - rnd(9)), x, y, TRUE, FALSE);
			mtmp->mnamelth = 0;
			/* Good luck gives more coins */
			obj = mkgold((long)(400 - rnl(101)), x, y);
			mtmp->mnamelth = 0;
		break;
		case PM_PAPER_GOLEM:
			num = rnd(4);
			while (num--)
				obj = mksobj_at(SCR_BLANK_PAPER, x, y, TRUE, FALSE);
			mtmp->mnamelth = 0;
		break;
	    case PM_STRAW_GOLEM:
		if(!rn2(10)) mksobj_at(SEDGE_HAT, x, y, FALSE, FALSE);
		obj = mksobj_at(SHEAF_OF_HAY, x, y, FALSE, FALSE);
		obj->quan = (long)(d(2,4));
		obj->owt = weight(obj);
		mtmp->mnamelth = 0;
		break;
		case PM_SPELL_GOLEM:{
			int scrnum = 0;
			int scrrng = SCR_STINKING_CLOUD-SCR_ENCHANT_ARMOR;
			num = rnd(8);
			while (num--){
				scrnum = d(1, scrrng)-1;
				obj = mksobj_at(scrnum+SCR_ENCHANT_ARMOR, x, y, TRUE, FALSE);
			}
			mtmp->mnamelth = 0;
		}
		break;
	    case PM_GARO:
			if(!rn2(100)){
				obj = mksobj_at(MASK, x, y, TRUE, FALSE);
				obj->corpsenm = PM_GARO;
			}
		goto default_1;
		break;
	    case PM_GARO_MASTER:
			obj = mksobj_at(MASK, x, y, TRUE, FALSE);
			obj->corpsenm = PM_GARO_MASTER;
		goto default_1;
		break;
	    case PM_CHANGED:
			flags.cth_attk=TRUE;//state machine stuff.
			create_gas_cloud(x, y, 4, rnd(3)+1);
			flags.cth_attk=FALSE;
			obj = mksobj_at(EYEBALL, x, y, FALSE, FALSE);
			obj->corpsenm = humanoid_eyes[rn2(SIZE(humanoid_eyes))];
			obj->quan = 2;
			obj->owt = weight(obj);
		goto default_1;
		break;
	    case PM_WARRIOR_CHANGED:
			flags.cth_attk=TRUE;//state machine stuff.
			create_gas_cloud(x, y, 5, rnd(3)+1);
			flags.cth_attk=FALSE;
			num = rn1(10,10);
			while (num--){
				obj = mksobj_at(EYEBALL, x, y, FALSE, FALSE);
				obj->corpsenm = humanoid_eyes[rn2(SIZE(humanoid_eyes))];
			}
		goto default_1;
		break;
	    case PM_TWITCHING_FOUR_ARMED_CHANGED:
			flags.cth_attk=TRUE;//state machine stuff.
			create_gas_cloud(x, y, 4, rnd(3)+1);
			flags.cth_attk=FALSE;
			obj = mksobj_at(EYEBALL, x, y, FALSE, FALSE);
			obj->corpsenm = PM_MYRKALFR;
			obj->quan = 2;
			obj = mksobj_at(EYEBALL, x, y, FALSE, FALSE);
			obj->corpsenm = PM_ELF;
			obj->quan = 2;
			num = rn1(10,10);
			while (num--){
				obj = mksobj_at(EYEBALL, x, y, FALSE, FALSE);
				obj->corpsenm = humanoid_eyes[rn2(SIZE(humanoid_eyes))];
			}
			num = rn1(10,10);
			while (num--){
				obj = mksobj_at(WORM_TOOTH, x, y, FALSE, FALSE);
				obj->oproperties = OPROP_LESSW|OPROP_FLAYW;
			}
		goto default_1;
		break;
	    case PM_CLAIRVOYANT_CHANGED:
			flags.cth_attk=TRUE;//state machine stuff.
			create_gas_cloud(x, y, 4, rnd(3)+1);
			flags.cth_attk=FALSE;
			obj = mksobj_at(EYEBALL, x, y, FALSE, FALSE);
			obj->corpsenm = PM_HUMAN;
			obj->quan = 2;
			obj->owt = weight(obj);
			obj->oartifact = ART_EYE_OF_THE_ORACLE;
			obj = mksobj_at(EYEBALL, x, y, FALSE, FALSE);
			obj->corpsenm = PM_HUMAN;
			obj->quan = 14;
			obj->owt = weight(obj);
			obj = mksobj_at(EYEBALL, x, y, FALSE, FALSE);
			obj->corpsenm = PM_HUMAN;
			obj->quan = 4;
			obj->owt = weight(obj);
		goto default_1;
		break;
	    case PM_EMBRACED_DROWESS:{
			struct monst *mon;
			mon = makemon(&mons[PM_DROW_CAPTAIN], x, y, MM_EDOG | MM_ADJACENTOK | NO_MINVENT | MM_NOCOUNTBIRTH);
			if (mon){
				initedog(mon);
				mon->mhpmax = (mon->m_lev * 8) - 4;
				mon->mhp = mon->mhpmax;
				mon->female = TRUE;
				mon->mtame = 10;
				mon->mpeaceful = 1;
				mon->mfaction = ZOMBIFIED;
			}
			obj = mkcorpstat(CORPSE, mon, (struct permonst *)0, x, y, FALSE);
			mongone(mon);
		}break;
	    case PM_PARASITIZED_EMBRACED_ALIDER:{
			struct monst *mon;
			mon = makemon(&mons[PM_ALIDER], x, y, MM_EDOG | MM_ADJACENTOK | NO_MINVENT | MM_NOCOUNTBIRTH);
			if (mon){
				initedog(mon);
				mon->mhpmax = (mon->m_lev * 8) - 4;
				mon->mhp = mon->mhpmax;
				mon->female = TRUE;
				mon->mtame = 10;
				mon->mpeaceful = 1;
				mon->mfaction = ZOMBIFIED;
			}
			mkcorpstat(CORPSE, mon, (struct permonst *)0, x, y, FALSE);
			mongone(mon);
			obj = mksobj_at(CORPSE, x, y, FALSE, FALSE);
			obj->corpsenm = PM_PARASITIC_MASTER_MIND_FLAYER;
			fix_object(obj);
		}break;
	    default_1:
	    default:
		if (mvitals[mndx].mvflags & G_NOCORPSE)
		    return (struct obj *)0;
		else if(u.sealsActive&SEAL_BERITH && !(mvitals[mndx].mvflags & G_UNIQ) 
				&& mtmp->m_lev > u.ulevel && !KEEPTRAITS(mtmp)
		){
			obj = mkgold((long)(mtmp->m_lev*25 - rnl(mtmp->m_lev*25/2+1)), x, y);
			mtmp->mnamelth = 0;
		} else{	/* preserve the unique traits of some creatures */
//			pline("preserving unique traits");
		    obj = mkcorpstat(CORPSE, KEEPTRAITS(mtmp) ? mtmp : 0,
				     mdat, x, y, TRUE);
		}
		break;
	}
	if(obj && obj->otyp == CORPSE && is_undead_mon(mtmp)){
		obj->age -= 100;		/* this is an *OLD* corpse */
	}
	/* All special cases should precede the G_NOCORPSE check */
	
	/* if polymorph or undead turning has killed this monster,
	   prevent the same attack beam from hitting its corpse */
	if (flags.bypasses) bypass_obj(obj);

	if (mtmp->mnamelth)
	    obj = oname(obj, NAME(mtmp));

	/* Avoid "It was hidden under a green mold corpse!" 
	 *  during Blind combat. An unseen monster referred to as "it"
	 *  could be killed and leave a corpse.  If a hider then hid
	 *  underneath it, you could be told the corpse type of a
	 *  monster that you never knew was there without this.
	 *  The code in hitmu() substitutes the word "something"
	 *  if the corpses obj->dknown is 0.
	 */
	if (Blind && !sensemon(mtmp)) obj->dknown = 0;

#ifdef INVISIBLE_OBJECTS
	/* Invisible monster ==> invisible corpse */
	obj->oinvis = mtmp->minvis;
#endif

	stackobj(obj);
	newsym(x, y);
	return obj;
}

#endif /* OVLB */
#ifdef OVL1

#if 0
/* part of the original warning code which was replaced in 3.3.1 */
STATIC_OVL void
warn_effects()
{
    if (warnlevel == 100) {
	if(!Blind && uwep &&
	    (warnlevel > lastwarnlev || moves > lastwarntime + warnDelay)) {
	    Your("%s %s!", aobjnam(uwep, "glow"),
		hcolor(NH_LIGHT_BLUE));
	    lastwarnlev = warnlevel;
	    lastwarntime = moves;
	}
	warnlevel = 0;
	return;
    }

    if (warnlevel >= SIZE(warnings))
	warnlevel = SIZE(warnings)-1;
    if (!Blind &&
	    (warnlevel > lastwarnlev || moves > lastwarntime + warnDelay)) {
	const char *which, *what, *how;
	long rings = (EWarning & (LEFT_RING|RIGHT_RING));

	if (rings) {
	    what = Hallucination ? "mood ring" : "ring";
	    how = "glows";	/* singular verb */
	    if (rings == LEFT_RING) {
		which = "left ";
	    } else if (rings == RIGHT_RING) {
		which = "right ";
	    } else {		/* both */
		which = "";
		what = (const char *) makeplural(what);
		how = "glow";	/* plural verb */
	    }
	    Your("%s%s %s %s!", which, what, how, hcolor(warnings[warnlevel]));
	} else {
	    if (Hallucination)
		Your("spider-sense is tingling...");
	    else
		You_feel("apprehensive as you sense a %s flash.",
		    warnings[warnlevel]);
	}

	lastwarntime = moves;
	lastwarnlev = warnlevel;
    }
}
#endif /* 0 */

/* check mtmp and water/lava for compatibility, 0 (survived), 1 (died) */
int
minliquid(mtmp)
register struct monst *mtmp;
{
	boolean inpool, inlava, infountain, inshallow;

	inpool = is_pool(mtmp->mx,mtmp->my, FALSE) &&
	     ((!mon_resistance(mtmp,FLYING) && !mon_resistance(mtmp,LEVITATION)) || is_3dwater(mtmp->mx,mtmp->my));
	inlava = is_lava(mtmp->mx,mtmp->my) &&
	     !mon_resistance(mtmp,FLYING) && !mon_resistance(mtmp,LEVITATION);
	infountain = IS_FOUNTAIN(levl[mtmp->mx][mtmp->my].typ);
	inshallow = IS_PUDDLE(levl[mtmp->mx][mtmp->my].typ) &&
	     (!mon_resistance(mtmp,FLYING) && !mon_resistance(mtmp,LEVITATION));

#ifdef STEED
	/* Flying and levitation keeps our steed out of the liquid */
	/* (but not water-walking or swimming) */
	if (mtmp == u.usteed && (Flying || Levitation) && !is_3dwater(mtmp->mx,mtmp->my))
		return (0);
#endif

    /* Gremlin multiplying won't go on forever since the hit points
     * keep going down, and when it gets to 1 hit point the clone
     * function will fail.
     */
    if (mtmp->data == &mons[PM_GREMLIN] && (inpool || infountain || inshallow) && rn2(3)) {
	if (split_mon(mtmp, (struct monst *)0))
	    dryup(mtmp->mx, mtmp->my, FALSE);
	if (inpool) water_damage(mtmp->minvent, FALSE, FALSE, level.flags.lethe, mtmp);
	return (0);
    } else if ((mtmp->data == &mons[PM_IRON_GOLEM] || mtmp->data == &mons[PM_CHAIN_GOLEM]) && ((inpool && !rn2(5)) || inshallow)) {
	/* rusting requires oxygen and water, so it's faster for shallow water */
	int dam = d(2,6);
	if (cansee(mtmp->mx,mtmp->my))
	    pline("%s rusts.", Monnam(mtmp));
	mtmp->mhp -= dam;
	if (mtmp->mhpmax > dam) mtmp->mhpmax -= dam;
	if (mtmp->mhp < 1) {
	    if (canseemon(mtmp)) pline("%s falls to pieces!", Monnam(mtmp));
	    mondead(mtmp);
	    if (mtmp->mhp < 1) {
			if (mtmp->mtame && !canseemon(mtmp))
				pline("May %s rust in peace.", mon_nam(mtmp));
			return (1);
		}
	}
	if(inshallow) water_damage(which_armor(mtmp, W_ARMF), FALSE, FALSE, level.flags.lethe, mtmp);
	else water_damage(mtmp->minvent, FALSE, FALSE, level.flags.lethe, mtmp);
	return (0);
    }

    if (inlava) {
	/*
	 * Lava effects much as water effects. Lava likers are able to
	 * protect their stuff. Fire resistant monsters can only protect
	 * themselves  --ALI
	 */
	if (!is_clinger(mtmp->data) && !likes_lava(mtmp->data)) {
	    if (!resists_fire(mtmp)) {
		if (cansee(mtmp->mx,mtmp->my))
		    pline("%s %s.", Monnam(mtmp),
			  mtmp->data == &mons[PM_WATER_ELEMENTAL] ?
			  "boils away" : "burns to a crisp");
		mondead(mtmp);
	    }
	    else {
		if (--mtmp->mhp < 1) {
		    if (cansee(mtmp->mx,mtmp->my))
			pline("%s surrenders to the fire.", Monnam(mtmp));
		    mondead(mtmp);
		}
		else if (cansee(mtmp->mx,mtmp->my))
		    pline("%s burns slightly.", Monnam(mtmp));
	    }
	    if (mtmp->mhp > 0) {
		(void) fire_damage(mtmp->minvent, FALSE, FALSE,
						mtmp->mx, mtmp->my);
		(void) rloc(mtmp, FALSE);
		return 0;
	    }
	    return (1);
	}
    } else if (inpool) {
	/* Most monsters drown in pools.  flooreffects() will take care of
	 * water damage to dead monsters' inventory, but survivors need to
	 * be handled here.  Swimmers are able to protect their stuff...
	 */
	if (!is_clinger(mtmp->data)
	    && !mon_resistance(mtmp,SWIMMING) && !amphibious_mon(mtmp)) {
	    if (cansee(mtmp->mx,mtmp->my)) {
		    if(mtmp->data == &mons[PM_ACID_PARAELEMENTAL]){
				int tx = mtmp->mx, ty = mtmp->my, dn = mtmp->m_lev;
				pline("%s explodes.", Monnam(mtmp));
				mondead(mtmp);
				explode(tx, ty, AD_ACID, MON_EXPLODE, d(dn, 10), EXPL_NOXIOUS, 1);
			} else pline("%s drowns.", Monnam(mtmp));
	    }
	    if (u.ustuck && u.uswallow && u.ustuck == mtmp) {
	    /* This can happen after a purple worm plucks you off a
		flying steed while you are over water. */
		pline("%s sinks as water rushes in and flushes you out.",
			Monnam(mtmp));
	    }
	    mondead(mtmp);
	    if (mtmp->mhp > 0) {
		(void) rloc(mtmp, FALSE);
		water_damage(mtmp->minvent, FALSE, FALSE, level.flags.lethe, mtmp);
		return 0;
	    }
	    return (1);
	}
    } else {
	/* but eels have a difficult time outside */
	if (mtmp->data->mlet == S_EEL && !Is_waterlevel(&u.uz)) {
		/* Puddles can sustain a tiny sea creature, or lessen the burdens of a larger one */
		if (!(inshallow && mtmp->data->msize == MZ_TINY))
		{
			if (mtmp->mhp > 1 && rn2(mtmp->data->msize)) mtmp->mhp--;
			monflee(mtmp, 2, FALSE, FALSE);
		}
	}
    }
    return (0);
}


int
mcalcmove(mon)
struct monst *mon;
{
    int mmove = mon->data->mmove;
	
	if(mon->mfaction == ZOMBIFIED && mmove > 6){
		mmove = mmove/2;
		if(mmove < 6) mmove = 6;
	}
	if(mon->mfaction == SKELIFIED && mmove > 6){
		mmove = mmove*3/4;
		if(mmove < 6) mmove = 6;
	}
	
	if(u.ustuck == mon && mmove < 12 && mon->data->mlet == S_VORTEX){
		mmove *= 2;
	}
	
	if(isdark(mon->mx, mon->my) && mon->data == &mons[PM_GRUE]){
		mmove *= 2;
	}
	
	if(mon->data == &mons[PM_PYTHON] && 
		dist2(mon->mx, mon->my, mon->mux, mon->muy) <= 8)
		mmove *= 4;
	
	if(uwep && uwep->oartifact == ART_SINGING_SWORD && !mindless_mon(mon) && !is_deaf(mon)){
		if(uwep->osinging == OSING_HASTE && mon->mtame)
			mmove += 2;
		else if(uwep->osinging == OSING_LETHARGY && !mon->mtame)
			mmove -= 2;
	}
	
	if(mon->data == &mons[PM_CHOKHMAH_SEPHIRAH])
		mmove += u.chokhmah;
	if(mon->data == &mons[PM_BANDERSNATCH] && mon->mflee)
		mmove += 12;
	if(mon->data == &mons[PM_UVUUDAUM] && mon->mpeaceful)
		mmove /= 4;
    /* Note: MSLOW's `+ 1' prevents slowed speed 1 getting reduced to 0;
     *	     MFAST's `+ 2' prevents hasted speed 1 from becoming a no-op;
     *	     both adjustments have negligible effect on higher speeds.
     */
    if (mon->mspeed == MSLOW)
	mmove = (2 * mmove + 1) / 3;
    else if (mon->mspeed == MFAST)
	mmove = (4 * mmove + 2) / 3;

#ifdef STEED
    if (mon == u.usteed) {
	if (u.ugallop && flags.mv) {
	    /* average movement is 1.50 times normal */
	    mmove = ((rn2(2) ? 4 : 5) * mmove) / 3;
	}
    }
#endif
	if(is_alabaster_mummy(mon->data) && mon->mvar1 == SYLLABLE_OF_GRACE__UUR)
		mmove += 6;
	
	if(u.sealsActive&SEAL_CHUPOCLOPS && distmin(mon->mx, mon->my, u.ux, u.uy) <= u.ulevel/5+1){
		mmove = max(mmove-(u.ulevel/10+1),1);
	}
	if(In_fog_cloud(mon)) mmove = max(mmove/3, 1);
	return mmove;
}

/* actions that happen once per ``turn'', regardless of each
   individual monster's metabolism; some of these might need to
   be reclassified to occur more in proportion with movement rate */
void
mcalcdistress()
{
    struct monst *mtmp;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	if (DEADMONSTER(mtmp)) continue;

	/* must check non-moving monsters once/turn in case
	 * they managed to end up in liquid */
	if (mtmp->data->mmove == 0) {
	    if (vision_full_recalc) vision_recalc(0);
	    if (minliquid(mtmp)) continue;
	}

	if(mtmp->data == &mons[PM_HEZROU] && !(mtmp->mtrapped && t_at(mtmp->mx, mtmp->my) && t_at(mtmp->mx, mtmp->my)->ttyp == VIVI_TRAP)){
		flags.cth_attk=TRUE;//state machine stuff.
		create_gas_cloud(mtmp->mx+rn2(3)-1, mtmp->my+rn2(3)-1, rnd(3), rnd(3)+1);
		flags.cth_attk=FALSE;
	}
	
	if(mtmp->data == &mons[PM_ANCIENT_OF_ICE]){
		struct monst *tmpm;
		int targets = 0, damage = 0;
		for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
			if(distmin(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= 4
				&& tmpm->mpeaceful != mtmp->mpeaceful
				&& tmpm->mtame != mtmp->mtame
				&& !resists_fire(tmpm)
				&& !DEADMONSTER(tmpm)
			) targets++;
		}
		if(distmin(u.ux,u.uy,mtmp->mx,mtmp->my) <= 4
			&& !mtmp->mpeaceful
			&& !mtmp->mtame
			&& !Fire_resistance
		) targets++;
		targets = rnd(targets);
		for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
			if(distmin(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= 4
				&& tmpm->mpeaceful != mtmp->mpeaceful
				&& tmpm->mtame != mtmp->mtame
				&& !resists_fire(tmpm)
				&& !DEADMONSTER(tmpm)
			) targets--;
			if(!targets) break;
		}
		if(tmpm){
			if(canseemon(tmpm) && canseemon(mtmp)){
				pline("Heat shimmer dances in the air above %s.", mon_nam(tmpm));
				pline("%s is covered in frost!", Monnam(tmpm));
				if(resists_cold(tmpm) && has_head(tmpm->data)) pline("%s looks very surprised!", Monnam(tmpm));
				pline("The shimmers are drawn into the open mouth of %s.", mon_nam(mtmp));
			} else if(canseemon(tmpm)){
				pline("Heat shimmer dances in the air above %s.", mon_nam(tmpm));
				pline("%s is covered in frost!", Monnam(tmpm));
				if(resists_cold(tmpm) && has_head(tmpm->data)) pline("%s looks very surprised!", Monnam(tmpm));
			} else if(canseemon(mtmp)){
				pline("Heat shimmers are drawn into the open mouth of %s.", mon_nam(mtmp));
			}
			damage = d(min(10, (mtmp->m_lev)/3), 8);
			tmpm->mhp -= damage;
			if(tmpm->mhp < 1){
				if (canspotmon(tmpm))
					pline("%s %s!", Monnam(tmpm),
					nonliving_mon(tmpm)
					? "is destroyed" : "dies");
				tmpm->mhp = 0;
				grow_up(mtmp,tmpm);
				mondied(tmpm);
			}
			mtmp->mhp += damage;
			if(mtmp->mhp > mtmp->mhpmax){
				mtmp->mhp = mtmp->mhpmax;
				// grow_up(mtmp,mtmp);
			}
			mtmp->mspec_used = 0;
			mtmp->mcan = 0;
		} else if(targets > 0
			&& distmin(u.ux,u.uy,mtmp->mx,mtmp->my) <= 4
			&& !mtmp->mpeaceful
			&& !mtmp->mtame
			&& !Fire_resistance
		){
			pline("Heat shimmer dances in the air above you.");
			pline("You are covered in frost!");
			if(canseemon(mtmp)){
				pline("The shimmers are drawn into the open mouth of %s.", mon_nam(mtmp));
			}
			damage = d(min(10, (mtmp->m_lev)/3), 8);
			losehp(damage, "heat drain", KILLED_BY);
			mtmp->mhp += damage;
			if(mtmp->mhp > mtmp->mhpmax){
				mtmp->mhp = mtmp->mhpmax;
				// grow_up(mtmp,mtmp);
			}
			mtmp->mspec_used = 0;
			mtmp->mcan = 0;
			mtmp->mux = u.ux;
			mtmp->muy = u.uy;
		}
		if(damage){
			struct attack mattk;
			mattk.aatyp = AT_BREA;
			mattk.adtyp = AD_COLD;
			mattk.damn = 8;
			mattk.damd = 8;
			
			if(mon_can_see_you(mtmp)){
				mtmp->mux = u.ux;
				mtmp->muy = u.uy;
			}
			
			if(!mtmp->mtame && !mtmp->mpeaceful && lined_up(mtmp)){
				flags.drgn_brth = 1;
				breamu(mtmp, &mattk);
				flags.drgn_brth = 0;
			} else {
				for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
					if(distmin(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= BOLT_LIM
						&& tmpm->mpeaceful != mtmp->mpeaceful
						&& tmpm->mtame != mtmp->mtame
						&& !resists_cold(tmpm)
						&& mlined_up(mtmp, tmpm, TRUE)
						&& !DEADMONSTER(tmpm)
					){
						flags.drgn_brth = 1;
						breamm(mtmp, tmpm, &mattk);
						flags.drgn_brth = 0;
						break;
					};
				}
			}
		}
	}
	
	if(mtmp->data == &mons[PM_ANCIENT_OF_DEATH]){
		struct monst *tmpm;
		int targets = 0, damage = 0;
		for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
			if(distmin(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= 4
				&& tmpm->mpeaceful != mtmp->mpeaceful
				&& tmpm->mtame != mtmp->mtame
				&& !nonliving_mon(tmpm)
				&& !DEADMONSTER(tmpm)
			) targets++;
		}
		if(distmin(u.ux,u.uy,mtmp->mx,mtmp->my) <= 4
			&& !mtmp->mpeaceful
			&& !mtmp->mtame
			&& !nonliving(youracedata)
		) targets++;
		targets = rnd(targets);
		for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
			if(distmin(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= 4
				&& tmpm->mpeaceful != mtmp->mpeaceful
				&& tmpm->mtame != mtmp->mtame
				&& !nonliving_mon(tmpm)
				&& !DEADMONSTER(tmpm)
			) targets--;
			if(!targets) break;
		}
		if(tmpm){
			if(canseemon(tmpm) && canseemon(mtmp)){
				pline("Motes of light dance in the air above %s.", mon_nam(tmpm));
				pline("%s suddenly seems weaker!", Monnam(tmpm));
				if(resists_drain(tmpm) && has_head(tmpm->data)) pline("%s looks very surprised!", Monnam(tmpm));
				pline("The motes are drawn into the %s of %s.", mtmp->data == &mons[PM_BAALPHEGOR] ? "open mouth" : "ghostly hood", mon_nam(mtmp));
			} else if(canseemon(tmpm)){
				pline("Motes of light dance in the air above %s.", mon_nam(tmpm));
				pline("%s suddenly seems weaker!", Monnam(tmpm));
				if(resists_drain(tmpm) && has_head(tmpm->data)) pline("%s looks very surprised!", Monnam(tmpm));
			} else if(canseemon(mtmp)){
				pline("Motes of light are drawn into the %s of %s.", mtmp->data == &mons[PM_BAALPHEGOR] ? "open mouth" : "ghostly hood", mon_nam(mtmp));
			}
			damage = d(min(10, (mtmp->m_lev)/3), 4);
			tmpm->mhp -= damage;
			if(tmpm->mhp < 1){
				if (canspotmon(tmpm))
					pline("%s %s!", Monnam(tmpm),
					nonliving_mon(tmpm)
					? "is destroyed" : "dies");
				tmpm->mhp = 0;
				grow_up(mtmp,tmpm);
				mondied(tmpm);
			}
			mtmp->mhp += damage;
			if(mtmp->mhp > mtmp->mhpmax){
				mtmp->mhp = mtmp->mhpmax;
				// grow_up(mtmp,mtmp);
			}
			mtmp->mspec_used = 0;
			mtmp->mcan = 0;
		} else if(targets > 0
			&& distmin(u.ux,u.uy,mtmp->mx,mtmp->my) <= 4
			&& !mtmp->mpeaceful
			&& !mtmp->mtame
			&& !nonliving(youracedata)
		){
			pline("Motes of light dance in the air above you.");
			pline("You suddenly feel weaker!");
			if(canseemon(mtmp)){
				pline("The motes are drawn into the %s of %s.", mtmp->data == &mons[PM_BAALPHEGOR] ? "open mouth" : "ghostly hood", mon_nam(mtmp));
			}
			damage = d(min(10, (mtmp->m_lev)/3), 4);
			losehp(damage, "life-force theft", KILLED_BY);
			mtmp->mhp += damage;
			if(mtmp->mhp > mtmp->mhpmax){
				mtmp->mhp = mtmp->mhpmax;
				// grow_up(mtmp,mtmp);
			}
			mtmp->mspec_used = 0;
			mtmp->mcan = 0;
			mtmp->mux = u.ux;
			mtmp->muy = u.uy;
		}
		if(damage){
			if(!mtmp->mtame && !mtmp->mpeaceful && distmin(u.ux,u.uy,mtmp->mx,mtmp->my) <= BOLT_LIM
				&& !(nonliving(youracedata) || is_demon(youracedata))
				&& !(ward_at(u.ux,u.uy) == CIRCLE_OF_ACHERON)
				&& !(u.sealsActive&SEAL_OSE || resists_death(&youmonst))
			){
				if(canseemon(mtmp)){
					pline("%s breathes out dark vapors.", Monnam(mtmp));
				}
				if(Hallucination){
					You("have an out of body experience.");
					losehp(8, "a bad trip", KILLED_BY); //you still take damage
				} else if (Antimagic){
					shieldeff(u.ux, u.uy);
					Your("%s flutters!", body_part(HEART));
					losehp(d(4,4), "the ancient breath of death", KILLED_BY); //you still take damage
				} else if(maybe_polyd((u.mh >= 100), (u.uhp >= 100))){
					Your("%s stops!  When it finally beats again, it is weak and thready.", body_part(HEART));
					losehp(d(8,8), "the ancient breath of death", KILLED_BY); //Same as death's touch attack, sans special effects
				} else {
					killer_format = KILLED_BY;
					killer = "the ancient breath of death";
					done(DIED);
				}
			} else {
				struct monst *targ = 0;
				for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
					if(distmin(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= BOLT_LIM
						&& tmpm->mpeaceful != mtmp->mpeaceful
						&& tmpm->mtame != mtmp->mtame
						&& !(nonliving_mon(tmpm) || is_demon(tmpm->data))
						&& !(ward_at(tmpm->mx,tmpm->my) == CIRCLE_OF_ACHERON)
						&& !(resists_death(tmpm))
						&& !DEADMONSTER(tmpm)
					){
						if(!targ || (distmin(tmpm->mx, tmpm->my, mtmp->mx, mtmp->my) < distmin(targ->mx, targ->my, mtmp->mx, mtmp->my))){
							targ = tmpm;
						}
					}
				}
				if(targ){
					if(canseemon(mtmp)){
						pline("%s breathes out dark vapors.", Monnam(mtmp));
					}
					if(resists_magm(targ)){
						targ->mhp -= 8;
						if(targ->mhp < 1){
							if (canspotmon(targ))
								pline("%s %s!", Monnam(targ),
								nonliving_mon(targ)
								? "is destroyed" : "dies");
							targ->mhp = 0;
							grow_up(mtmp,targ);
							mondied(targ);
						}
					} else if(targ->mhp >= 100){
						targ->mhp -= d(8,8);
						if(targ->mhp < 1){
							if (canspotmon(targ))
								pline("%s %s!", Monnam(targ),
								nonliving_mon(targ)
								? "is destroyed" : "dies");
							targ->mhp = 0;
							grow_up(mtmp,targ);
							mondied(targ);
						}
					} else {
						if (canspotmon(targ))
							pline("%s %s!", Monnam(targ),
							nonliving_mon(targ)
							? "is destroyed" : "dies");
						targ->mhp = 0;
						grow_up(mtmp,targ);
						mondied(targ);
					}
				}
			}
		}
	}
	
	if(mtmp->data == &mons[PM_BAALPHEGOR] && mtmp->mhp < mtmp->mhpmax/2){
		struct monst *tmpm;
		int targets = 0, damage = 0;
		for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
			if(distmin(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= 4
				&& (tmpm->mpeaceful != mtmp->mpeaceful || mtmp->mhp < mtmp->mhpmax/4)
				&& (tmpm->mtame != mtmp->mtame || mtmp->mhp < mtmp->mhpmax/4)
				&& tmpm->mfaction != CRYSTALFIED
				&& !is_demon(tmpm->data)
				&& !DEADMONSTER(tmpm)
			) targets++;
		}
		if(distmin(u.ux,u.uy,mtmp->mx,mtmp->my) <= 4
			&& !mtmp->mpeaceful
			&& !mtmp->mtame
			&& !is_demon(youracedata)
		) targets++;
		targets = rnd(targets);
		for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
			if(distmin(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= 4
				&& (tmpm->mpeaceful != mtmp->mpeaceful || mtmp->mhp < mtmp->mhpmax/4)
				&& (tmpm->mtame != mtmp->mtame || mtmp->mhp < mtmp->mhpmax/4)
				&& tmpm->mfaction != CRYSTALFIED
				&& !is_demon(tmpm->data)
				&& !DEADMONSTER(tmpm)
			) targets--;
			if(!targets) break;
		}
		if(tmpm){
			if(canseemon(tmpm) && canseemon(mtmp)){
				pline("Some unseen virtue is drawn from %s.", mon_nam(tmpm));
//				pline("%s suddenly seems weaker!", Monnam(tmpm));
				pline("The virtue is sucked into the open mouth of %s.", mon_nam(mtmp));
			} else if(canseemon(tmpm)){
				pline("Some unseen virtue is drawn from %s.", mon_nam(tmpm));
//				pline("%s suddenly seems weaker!", Monnam(tmpm));
				if(resists_drain(tmpm) && has_head(tmpm->data)) pline("%s looks very surprised!", Monnam(tmpm));
			} else if(canseemon(mtmp)){
				pline("Some unseen virtue is sucked into the open mouth of %s.", mon_nam(mtmp));
			}
			damage = d(min(10, (mtmp->m_lev)/3), 8);
			if(resists_cold(mtmp)) damage /= 2;
			if(damage >= tmpm->mhp){
				grow_up(mtmp,tmpm);
				if (canspotmon(tmpm))
					pline("%s turns to glass!", Monnam(tmpm));
				tmpm->mpeaceful = mtmp->mpeaceful;
				if(tmpm->mtame && tmpm->mtame != mtmp->mtame)
					tmpm->mtame = 0;
				tmpm->mfaction = CRYSTALFIED;
				newsym(tmpm->mx, tmpm->my);
				mtmp->mhp += tmpm->mhp;
			}
			else{
				tmpm->mhp -= damage;
				mtmp->mhp += damage;
			}
			
			if(mtmp->mhp > mtmp->mhpmax){
				mtmp->mhp = mtmp->mhpmax;
				// grow_up(mtmp,mtmp);
			}
			mtmp->mspec_used = 0;
			mtmp->mcan = 0;
		} else if(targets > 0
			&& distmin(u.ux,u.uy,mtmp->mx,mtmp->my) <= 4
			&& !mtmp->mpeaceful
			&& !mtmp->mtame
			&& !is_demon(youracedata)
		){
			pline("Some unseen virtue is drawn from you.");
			if(canseemon(mtmp)){
				pline("The virtue is sucked into the open mouth of %s.", mon_nam(mtmp));
			}
			damage = d(min(10, (mtmp->m_lev)/3), 8);
			if(Free_action) damage /= 2;
			if((HCold_resistance && ECold_resistance)
				|| (NCold_resistance)
			) damage /= 2;
			int temparise = u.ugrave_arise;
			mtmp->mhp += maybe_polyd(u.mh, u.uhp);
			u.ugrave_arise = PM_BAALPHEGOR;
			mdamageu(mtmp, damage);
			/*If the player surived the attack, restore the value of arise*/
			u.ugrave_arise = temparise;
			
			if(mtmp->mhp > mtmp->mhpmax){
				mtmp->mhp = mtmp->mhpmax;
				// grow_up(mtmp,mtmp);
			}
			mtmp->mspec_used = 0;
			mtmp->mcan = 0;
			mtmp->mux = u.ux;
			mtmp->muy = u.uy;
		}
		if(damage){
			struct	obj	*otmp;
			for(otmp = invent; otmp; otmp=otmp->nobj)
				if(otmp->oartifact == ART_HELPING_HAND)
					break;
			if(!mtmp->mtame && !mtmp->mpeaceful && distmin(u.ux,u.uy,mtmp->mx,mtmp->my) <= BOLT_LIM
				&& !(uamul && (uamul->otyp == AMULET_VERSUS_CURSES))
				&& !(uwep && (uwep->oartifact == ART_MAGICBANE) && rn2(20))
				&& !(uwep && (uwep->oartifact == ART_TENTACLE_ROD) && rn2(20))
				&& !(otmp && rn2(20))
				&& !(u.ukinghill && rn2(20))
			){
				if(canseemon(mtmp)){
					pline("%s breathes out static curses.", Monnam(mtmp));
				}
				if(
					(!Free_action || HFast) &&
					(!uarmh || uarmh->cursed) &&
					(!uarmc || uarmc->cursed) &&
					(!uarm || uarm->cursed) &&
					(!uarmu || uarmu->cursed) &&
					(!uarmg || uarmg->cursed) &&
					(!uarmf || uarmf->cursed)
				) {
					if(Free_action){
						HFast = 0L;
						if (!Very_fast)
							You_feel("yourself slowing down%s.",
										Fast ? " a bit" : "");
					} else done(GLASSED);
				} else {
					int nobj = 0, cnt, onum;
					for (otmp = invent; otmp; otmp = otmp->nobj) {
#ifdef GOLDOBJ
						/* gold isn't subject to being cursed or blessed */
						if (otmp->oclass == COIN_CLASS) continue;
#endif
						if(!otmp->cursed) nobj++;
					}
					if (nobj) {
						for (cnt = 8; cnt > 0; cnt--)  {
							onum = rnd(nobj);
							for (otmp = invent; otmp; otmp = otmp->nobj) {
#ifdef GOLDOBJ
								/* gold isn't subject to being cursed or blessed */
								if (otmp->oclass == COIN_CLASS) continue;
#endif
								if(!otmp->cursed) onum--;
								if(!onum) break;
							}
							if (!otmp || otmp->cursed) continue;	/* next target */
							if(otmp->oartifact && spec_ability(otmp, SPFX_INTEL) &&
							   rn2(10) < 8) {
								pline("%s!", Tobjnam(otmp, "resist"));
								continue;
							}

							if(otmp->blessed)
								unbless(otmp);
							else
								curse(otmp);
							update_inventory();
						}
					}
				}
			} else {
				struct monst *targ = 0;
				struct obj *otmp;
				for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
					if(distmin(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= BOLT_LIM
						&& tmpm->mpeaceful != mtmp->mpeaceful
						&& tmpm->mtame != mtmp->mtame
						&& tmpm->mfaction != CRYSTALFIED
						// && !(uamul && (uamul->otyp == AMULET_VERSUS_CURSES))
						&& !(MON_WEP(tmpm) && (MON_WEP(tmpm)->oartifact == ART_MAGICBANE) && rn2(20))
						&& !(MON_WEP(tmpm) && (MON_WEP(tmpm)->oartifact == ART_TENTACLE_ROD) && rn2(20))
						&& !DEADMONSTER(tmpm)
					){
						for(otmp = tmpm->minvent; otmp; otmp=otmp->nobj)
							if(otmp->oartifact == ART_TREASURY_OF_PROTEUS)
								break;
						if(otmp && rn2(20))
							continue;
						for(otmp = tmpm->minvent; otmp; otmp=otmp->nobj)
							if(otmp->oartifact == ART_HELPING_HAND)
								break;
						if(otmp && rn2(20))
							continue;
						if(!targ || (distmin(tmpm->mx, tmpm->my, mtmp->mx, mtmp->my) < distmin(targ->mx, targ->my, mtmp->mx, mtmp->my))){
							targ = tmpm;
						}
					}
				}
				if(targ){
					if(canseemon(mtmp)){
						pline("%s breathes out static curses.", Monnam(mtmp));
					}
					if(
						!(targ->misc_worn_check & W_ARMH && (otmp = which_armor(targ, W_ARMH)) && !otmp->cursed) &&
						!(targ->misc_worn_check & W_ARMC && (otmp = which_armor(targ, W_ARMC)) && !otmp->cursed) &&
						!(targ->misc_worn_check & W_ARM && (otmp = which_armor(targ, W_ARM)) && !otmp->cursed) &&
						!(targ->misc_worn_check & W_ARMU && (otmp = which_armor(targ, W_ARMU)) && !otmp->cursed) &&
						!(targ->misc_worn_check & W_ARMG && (otmp = which_armor(targ, W_ARMG)) && !otmp->cursed) &&
						!(targ->misc_worn_check & W_ARMF && (otmp = which_armor(targ, W_ARMF)) && !otmp->cursed)
					) {
						minstaglass(targ,FALSE);
					} else {
						int nobj = 0, cnt, onum;
						for (otmp = targ->minvent; otmp; otmp = otmp->nobj) {
#ifdef GOLDOBJ
							/* gold isn't subject to being cursed or blessed */
							if (otmp->oclass == COIN_CLASS) continue;
#endif
							if(!otmp->cursed) nobj++;
						}
						if (nobj) {
							for (cnt = 8; cnt > 0; cnt--)  {
								onum = rnd(nobj);
								for (otmp = targ->minvent; otmp; otmp = otmp->nobj) {
#ifdef GOLDOBJ
									/* gold isn't subject to being cursed or blessed */
									if (otmp->oclass == COIN_CLASS) continue;
#endif
									if(!otmp->cursed) onum--;
									if(!onum) break;
								}
								if (!otmp || otmp->cursed) continue;	/* next target */
								if(otmp->oartifact && spec_ability(otmp, SPFX_INTEL) &&
								   rn2(10) < 8) {
									continue;
								}

								if(otmp->blessed)
									unbless(otmp);
								else
									curse(otmp);
								// update_inventory();
							}
						}
					}
				}
			}
		}
	}
	
	/* regenerate hit points */
	mon_regen(mtmp, FALSE);
	
	timeout_problems(mtmp);
	
	/* FIXME: mtmp->mlstmv ought to be updated here */
    }
}

void
timeout_problems(mtmp)
struct monst *mtmp;
{
	if(bold(mtmp->data) && mtmp->mflee){
		if(mtmp->mfleetim > 4) mtmp->mfleetim /= 4;
		else {
			mtmp->mfleetim = 0;
			mtmp->mflee = 0;
		}
	}
	
	
	/* possibly polymorph shapechangers and lycanthropes */
	if (mtmp->cham && !rn2(6))
	    (void) newcham(mtmp, (struct permonst *)0, FALSE, FALSE);
	were_change(mtmp);
	
	if(!mtmp->mcansee && (mtmp->data == &mons[PM_SHOGGOTH] || mtmp->data == &mons[PM_PRIEST_OF_GHAUNADAUR])){
		if(canspotmon(mtmp)) pline("%s forms new eyes!",Monnam(mtmp));
		mtmp->mblinded = 1;
	}
	
	/* gradually time out temporary problems */
	if (mtmp->mblinded && !--mtmp->mblinded)
	    mtmp->mcansee = 1;
	if (mtmp->mdeafened && !--mtmp->mdeafened)
	    mtmp->mcanhear = 1;
	if (mtmp->mlaughing && !--mtmp->mlaughing)
	    mtmp->mnotlaugh = 1;
	if (mtmp->mfrozen && !--mtmp->mfrozen)
	    mtmp->mcanmove = 1;
	if (mtmp->mfleetim && !--mtmp->mfleetim)
	    mtmp->mflee = 0;
}


int
movemon()
{
    register struct monst *mtmp, *nmtmp;
    register boolean somebody_can_move = FALSE;
	int maxtheta=0, mintheta=0, indextheta, deltax, deltay, theta, arc;
	boolean thetafirst=TRUE;

#if 0
    /* part of the original warning code which was replaced in 3.3.1 */
    warnlevel = 0;
#endif

    /*
    Some of you may remember the former assertion here that
    because of deaths and other actions, a simple one-pass
    algorithm wasn't possible for movemon.  Deaths are no longer
    removed to the separate list fdmon; they are simply left in
    the chain with hit points <= 0, to be cleaned up at the end
    of the pass.

    The only other actions which cause monsters to be removed from
    the chain are level migrations and losedogs().  I believe losedogs()
    is a cleanup routine not associated with monster movements, and
    monsters can only affect level migrations on themselves, not others
    (hence the fetching of nmon before moving the monster).  Currently,
    monsters can jump into traps, read cursed scrolls of teleportation,
    and drink cursed potions of raise level to change levels.  These are
    all reflexive at this point.  Should one monster be able to level
    teleport another, this scheme would have problems.
	
	Uh oh, weeping 	angels have a levelport attack. Doesn't seem to 
	cause problems, though....
    */
	
	/////////////////////////////////////////////////////
	//Weeping angel handling////////////////////////////
	///////////////////////////////////////////////////
	/*
	 * 1) Angels generated randomly prior to you becoming a demigod are generated as individuals. 
	 *		Angels generated afterwards are generated as large groups.
	 *		Check if any angels on current level were generated before you became a demigod, and create a large group if so.
	 * 2) Angels move faster if there are more of them in your line of sight, and especially if they are widely separated.
	 *		Calculate the minimum vision arc that covers all angels in view
	 * 3) Once the current movement loop begins, scale the angel's movement by the value calculated in 2. Also, scale
	 *		the angel's AC based on the value from 2, slower speed equals higher AC (Quantum Lock).
	 */
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon){
		//Weeping angel step 1
		if(is_weeping(mtmp->data)){
			if(mtmp->mvar3 && u.uevent.invoked){
				mtmp->mvar3 = 0; //Quantum Lock status will be reset below.
				m_initgrp(mtmp, 0, 0, 10);
			}
		} else if(mtmp->data == &mons[PM_RAZORVINE] && 
				  mtmp->mhp == mtmp->mhpmax && 
				  !mtmp->mcan && 
				  !((monstermoves + mtmp->mnum) % (30-depth(&u.uz)/2)) && 
				  !rn2(4)
		){
			struct monst *sprout = (struct monst *) 0;
			sprout = makemon(mtmp->data,(mtmp->mx-1)+rn2(3),(mtmp->my-1)+rn2(3),MM_CHECK_GOODPOS|MM_NOCOUNTBIRTH|NO_MINVENT);
			if(sprout) sprout->mhp = In_hell(&u.uz) ? sprout->mhp*3/4 : sprout->mhp/2;
		}
	}
	
	//Weeping angel step 2
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if(is_weeping(mtmp->data) && canseemon(mtmp)){
			deltax = u.ux - mtmp->mx;
			deltay = u.uy - mtmp->my;
			theta = (int)(atanGerald((float)deltay/(float)deltax)*100);
			if(deltax<0&&deltay<0) theta+=314;
			else if(deltax<0) theta+=314;
			else if(deltay<=0) theta+=628;
			theta -= 314;
			if(thetafirst){
				indextheta = theta;
				thetafirst = FALSE;
			}else{
				theta = theta - indextheta;
				if(theta <= -314) theta+=628;
				else if(theta > 314) theta -= 628;
				if(theta<mintheta) mintheta = theta;
				else if(theta>maxtheta) maxtheta = theta;
			}
		}
	}
	arc = maxtheta - mintheta;
	
	//Current Movement Loop///////////////////////////////////////////////////
    for(mtmp = fmon; mtmp; mtmp = nmtmp) {
	nmtmp = mtmp->nmon;
	/* Find a monster that we have not treated yet.	 */
	if(DEADMONSTER(mtmp))
	    continue;
	if(mtmp->movement < NORMAL_SPEED)
	    continue;

	mtmp->movement -= NORMAL_SPEED;
	if (mtmp->movement >= NORMAL_SPEED)
	    somebody_can_move = TRUE;
	
	if(mtmp->mstdy > 0) mtmp->mstdy -= 1; //monster is moving, reduce studied level
	else if(mtmp->mstdy < 0) mtmp->mstdy += 1; //monster is moving, reduce protection level

	//Weeping angel step 3
	if(is_weeping(mtmp->data)){
		mtmp->mvar2 &= 0x1L; //clear higher order bits, first bit is whether it should generate a swarm when you return
		if(canseemon(mtmp)){
			mtmp->mvar1 +=  (long)(NORMAL_SPEED*arc/314);
			if(!arc){
				mtmp->mvar2 |= 0x4L; //Quantum Locked
			}
			else if(arc < 314/2){
				mtmp->mvar2 |= 0x2L; //Partial Quantum Lock
			}
			m_respond(mtmp);
			if(mtmp->mvar1 >= NORMAL_SPEED*2) mtmp->mvar1 -= NORMAL_SPEED*2;
			else continue;
		}
		//else no quant lock
		
	}
	
	if(mtmp->data == &mons[PM_METROID_QUEEN]){
		if(mtmp->mtame){
			struct monst *baby;
			int tnum = d(1,6);
			int i;
			mtmp->mtame = 0;
			mtmp->mpeaceful = 1;
			for(i = 0; i < 6; i++){
				baby = makemon(&mons[PM_METROID], mtmp->mx, mtmp->my, MM_ADJACENTOK);
				if(tnum-- > 0) tamedog(baby,(struct obj *) 0);
				else baby->mpeaceful = 1;
			}
		}
		if(!rn2(100)){
			struct obj *egg;
			egg = mksobj(EGG, FALSE, FALSE);
			egg->spe = 0; //not yours
			egg->quan = 1;
			egg->owt = weight(egg);
			egg->corpsenm = egg_type_from_parent(PM_METROID_QUEEN, FALSE);
			attach_egg_hatch_timeout(egg);
			if(canseemon(mtmp)){
				egg->known = egg->dknown = 1;
				pline("%s lays an egg.",Monnam(mtmp));
			}
			if (!flooreffects(egg, mtmp->mx, mtmp->my, "")) {
				place_object(egg, mtmp->mx, mtmp->my);
				stackobj(egg);
				newsym(mtmp->mx, mtmp->my);  /* map the rock */
			}
			stackobj(egg);
		}
	}
	
	if(u.uevent.uaxus_foe && 
		is_auton(mtmp->data) && 
		mtmp->mpeaceful
	){
		if(canseemon(mtmp)) pline("%s gets angry...", mon_nam(mtmp));
		mtmp->mpeaceful = 0;
		mtmp->mtame = 0;
	}
	if(mtmp->data == &mons[PM_UVUUDAUM]){
		if(u.uevent.invoked 
		|| (Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz))
		|| mtmp->mhp < mtmp->mhpmax
		|| mtmp->m_lev < 30
		|| mtmp->mspec_used > 0
		){ 
			if(mtmp->mpeaceful){
				pline("%s ceases its meditation...", Amonnam(mtmp));
				mtmp->mpeaceful = 0;
				set_malign(mtmp);
			}
		} else if(!mtmp->mpeaceful && mtmp->mhp == mtmp->mhpmax && mtmp->m_lev >= 30 && mtmp->mspec_used == 0){
			pline("%s resumes its meditation...", Amonnam(mtmp));
			mtmp->mpeaceful = 1;
			set_malign(mtmp);
		}
	}
	if(mtmp->data == &mons[PM_DREAD_SERAPH] && 
		mtmp->mhp == mtmp->mhpmax && 
		!(mtmp->mstrategy&STRAT_WAITMASK) && 
		!(u.uevent.invoked || (Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)))
			
	){
		//go back to sleep
		mtmp->mstrategy |= STRAT_WAITMASK;
	}
	
	if (vision_full_recalc) vision_recalc(0);	/* vision! */

	if (is_hider(mtmp->data)) {
	    /* unwatched mimics and piercers may hide again  [MRS] */
	    if(restrap(mtmp))   continue;
	    if(mtmp->m_ap_type == M_AP_FURNITURE ||
				mtmp->m_ap_type == M_AP_OBJECT)
		    continue;
	    if(mtmp->mundetected) continue;
	}

	if (minliquid(mtmp)) continue;

	/* continue if the monster died fighting */
	if (!mtmp->iswiz && !is_blind(mtmp)) {
	    /* Note:
	     *  Conflict does not take effect in the first round.
	     *  Therefore, A monster when stepping into the area will
	     *  get to swing at you.
	     *
	     *  The call to fightm() must be _last_.  The monster might
	     *  have died if it returns 1.
	     */
	    if (fightm(mtmp)) continue;	/* mon might have died */
	}
	if(dochugw(mtmp))		/* otherwise just move the monster */
	    continue;
    }
#if 0
    /* part of the original warning code which was replaced in 3.3.1 */
    if(warnlevel > 0)
	warn_effects();
#endif

    if (any_light_source())
	vision_full_recalc = 1;	/* in case a mon moved with a light source */
    dmonsfree();	/* remove all dead monsters */

    /* a monster may have levteleported player -dlc */
    if (u.utotype) {
	deferred_goto();
	/* changed levels, so these monsters are dormant */
	somebody_can_move = FALSE;
    }

    return somebody_can_move;
}

#endif /* OVL1 */
#ifdef OVLB

#define mstoning(obj)	(ofood(obj) && \
					(touch_petrifies(&mons[(obj)->corpsenm]) || \
					(obj)->corpsenm == PM_MEDUSA))

/*
 * Maybe eat a metallic object (not just gold).
 * Return value: 0 => nothing happened, 1 => monster ate something,
 * 2 => monster died (it must have grown into a genocided form, but
 * that can't happen at present because nothing which eats objects
 * has young and old forms).
 */
int
meatmetal(mtmp)
	register struct monst *mtmp;
{
	register struct obj *otmp;
	struct permonst *ptr;
	int poly, grow, heal, mstone;

	/* If a pet, eating is handled separately, in dog.c */
	if (mtmp->mtame) return 0;

	/* Eats topmost metal object if it is there */
	for (otmp = level.objects[mtmp->mx][mtmp->my];
						otmp; otmp = otmp->nexthere) {
	    if (mtmp->data == &mons[PM_RUST_MONSTER] && !is_rustprone(otmp))
		continue;
	    if (is_metallic(otmp) && !obj_resists(otmp, 5, 95) &&
		touch_artifact(otmp, mtmp, FALSE)) {
		if (mtmp->data == &mons[PM_RUST_MONSTER] && otmp->oerodeproof) {
		    if (canseemon(mtmp) && flags.verbose) {
			pline("%s eats %s!",
				Monnam(mtmp),
				distant_name(otmp,doname));
		    }
		    /* The object's rustproofing is gone now */
		    otmp->oerodeproof = 0;
		    mtmp->mstun = 1;
		    if (canseemon(mtmp) && flags.verbose) {
			pline("%s spits %s out in disgust!",
			      Monnam(mtmp), distant_name(otmp,doname));
		    }
		/* KMH -- Don't eat indigestible/choking objects */
		} else if (otmp->otyp != AMULET_OF_STRANGULATION &&
				otmp->otyp != RIN_SLOW_DIGESTION) {
		    if (cansee(mtmp->mx,mtmp->my) && flags.verbose)
			pline("%s eats %s!", Monnam(mtmp),
				distant_name(otmp,doname));
		    else if (flags.soundok && flags.verbose)
			You_hear("a crunching sound.");
		    mtmp->meating = otmp->owt/2 + 1;
		    /* Heal up to the object's weight in hp */
		    if (mtmp->mhp < mtmp->mhpmax) {
			mtmp->mhp += objects[otmp->otyp].oc_weight;
			if (mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;
		    }
		    if(otmp == uball) {
			unpunish();
			delobj(otmp);
		    } else if (otmp == uchain) {
			unpunish();	/* frees uchain */
		    } else {
			poly = polyfodder(otmp) && !resists_poly(mtmp->data);
			grow = mlevelgain(otmp);
			heal = mhealup(otmp);
			mstone = mstoning(otmp);
			delobj(otmp);
			ptr = mtmp->data;
			if (poly) {
			    if (newcham(mtmp, (struct permonst *)0,
					FALSE, FALSE))
				ptr = mtmp->data;
			} else if (grow) {
			    ptr = grow_up(mtmp, (struct monst *)0);
			} else if (mstone) {
			    if (poly_when_stoned(ptr)) {
				mon_to_stone(mtmp);
				ptr = mtmp->data;
			    } else if (!resists_ston(mtmp)) {
				if (canseemon(mtmp))
				    pline("%s turns to stone!", Monnam(mtmp));
				monstone(mtmp);
				ptr = (struct permonst *)0;
			    }
			} else if (heal) {
			    mtmp->mhp = mtmp->mhpmax;
			}
			if (!ptr) return 2;		 /* it died */
		    }
		    /* Left behind a pile? */
		    if (rnd(25) < 3)
			(void)mksobj_at(ROCK, mtmp->mx, mtmp->my, TRUE, FALSE);
		    newsym(mtmp->mx, mtmp->my);
		    return 1;
		}
	    }
	}
	return 0;
}

int
meatobj(mtmp)		/* for gelatinous cubes */
	register struct monst *mtmp;
{
	register struct obj *otmp, *otmp2;
	struct permonst *ptr;
	int poly, grow, heal, count = 0, ecount = 0;
	char buf[BUFSZ];

	buf[0] = '\0';
	/* If a pet, eating is handled separately, in dog.c */
	if (mtmp->mtame) return 0;

	/* Eats organic objects, including cloth and wood, if there */
	/* Engulfs others, except huge rocks and metal attached to player */
	for (otmp = level.objects[mtmp->mx][mtmp->my]; otmp; otmp = otmp2) {
	    otmp2 = otmp->nexthere;
	    if (is_organic(otmp) && !obj_resists(otmp, 5, 95) &&
		    touch_artifact(otmp, mtmp, FALSE)) {
		if (otmp->otyp == CORPSE && touch_petrifies(&mons[otmp->corpsenm]) &&
			!resists_ston(mtmp))
		    continue;
		if (otmp->otyp == AMULET_OF_STRANGULATION ||
				otmp->otyp == RIN_SLOW_DIGESTION)
		    continue;
		++count;
		if (cansee(mtmp->mx,mtmp->my) && flags.verbose)
		    pline("%s eats %s!", Monnam(mtmp),
			    distant_name(otmp, doname));
		else if (flags.soundok && flags.verbose)
		    You_hear("a slurping sound.");
		/* Heal up to the object's weight in hp */
		if (mtmp->mhp < mtmp->mhpmax) {
		    mtmp->mhp += objects[otmp->otyp].oc_weight;
		    if (mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;
		}
		if (Has_contents(otmp)) {
		    register struct obj *otmp3;
		    /* contents of eaten containers become engulfed; this
		       is arbitrary, but otherwise g.cubes are too powerful */
		    while ((otmp3 = otmp->cobj) != 0) {
			obj_extract_self(otmp3);
			if (otmp->otyp == ICE_BOX && otmp3->otyp == CORPSE) {
			    otmp3->age = monstermoves - otmp3->age;
			    start_corpse_timeout(otmp3);
			}
			(void) mpickobj(mtmp, otmp3);
		    }
		}
		poly = polyfodder(otmp) && !resists_poly(mtmp->data);
		grow = mlevelgain(otmp);
		heal = mhealup(otmp);
		delobj(otmp);		/* munch */
		ptr = mtmp->data;
		if (poly) {
		    if (newcham(mtmp, (struct permonst *)0, FALSE, FALSE))
			ptr = mtmp->data;
		} else if (grow) {
		    ptr = grow_up(mtmp, (struct monst *)0);
		} else if (heal) {
		    mtmp->mhp = mtmp->mhpmax;
		}
		/* in case it polymorphed or died */
		if (ptr != &mons[PM_GELATINOUS_CUBE])
		    return !ptr ? 2 : 1;
	    } else if (otmp->oclass != ROCK_CLASS &&
				    otmp != uball && otmp != uchain) {
		++ecount;
		if (ecount == 1) {
			Sprintf(buf, "%s engulfs %s.", Monnam(mtmp),
			    distant_name(otmp,doname));
		} else if (ecount == 2)
			Sprintf(buf, "%s engulfs several objects.", Monnam(mtmp));
		obj_extract_self(otmp);
		(void) mpickobj(mtmp, otmp);	/* slurp */
	    }
	    /* Engulf & devour is instant, so don't set meating */
	    if (mtmp->minvis) newsym(mtmp->mx, mtmp->my);
	}
	if (ecount > 0) {
	    if (cansee(mtmp->mx, mtmp->my) && flags.verbose && buf[0])
		pline("%s", buf);
	    else if (flags.soundok && flags.verbose)
	    	You_hear("%s slurping sound%s.",
			ecount == 1 ? "a" : "several",
			ecount == 1 ? "" : "s");
	}
	return ((count > 0) || (ecount > 0)) ? 1 : 0;
}

void
mpickgold(mtmp)
	register struct monst *mtmp;
{
    register struct obj *gold;
    int mat_idx;

    if ((gold = g_at(mtmp->mx, mtmp->my)) != 0) {
	mat_idx = gold->obj_material;
#ifndef GOLDOBJ
	mtmp->mgold += gold->quan;
	delobj(gold);
#else
        obj_extract_self(gold);
        add_to_minv(mtmp, gold);
#endif
	if (cansee(mtmp->mx, mtmp->my) ) {
	    if (flags.verbose && !mtmp->isgd)
		pline("%s picks up some %s.", Monnam(mtmp),
			mat_idx == GOLD ? "gold" : "money");
	    newsym(mtmp->mx, mtmp->my);
	}
    }
}
#endif /* OVLB */
#ifdef OVL2

boolean
mpickstuff(mtmp, str)
	register struct monst *mtmp;
	register const char *str;
{
	register struct obj *otmp, *otmp2;

/*	prevent shopkeepers from leaving the door of their shop */
	if(mtmp->isshk && inhishop(mtmp)) return FALSE;

	for(otmp = level.objects[mtmp->mx][mtmp->my]; otmp; otmp = otmp2) {
	    otmp2 = otmp->nexthere;
/*	Nymphs take everything.  Most monsters don't pick up corpses. */
	    if (!str ? searches_for_item(mtmp,otmp) :
		  !!(index(str, otmp->oclass))) {
		if (otmp->otyp == CORPSE && mtmp->data->mlet != S_NYMPH &&
			/* let a handful of corpse types thru to can_carry() */
			!touch_petrifies(&mons[otmp->corpsenm]) &&
			otmp->corpsenm != PM_LIZARD &&
			!acidic(&mons[otmp->corpsenm])) continue;
		if (!touch_artifact(otmp, mtmp, FALSE)) continue;
		if (!can_carry(mtmp,otmp)) continue;
		if (is_pool(mtmp->mx,mtmp->my, FALSE)) continue;
#ifdef INVISIBLE_OBJECTS
		if (otmp->oinvis && !mon_resistance(mtmp,SEE_INVIS)) continue;
#endif
		if (cansee(mtmp->mx,mtmp->my) && flags.verbose)
			pline("%s picks up %s.", Monnam(mtmp),
			      (distu(mtmp->mx, mtmp->my) <= 5) ?
				doname(otmp) : distant_name(otmp, doname));
		obj_extract_self(otmp);
		/* unblock point after extract, before pickup */
		if (is_boulder(otmp))
		    unblock_point(otmp->ox,otmp->oy);	/* vision */
		if(otmp) (void) mpickobj(mtmp, otmp);	/* may merge and free otmp */
		m_dowear(mtmp, FALSE);
		newsym(mtmp->mx, mtmp->my);
		return TRUE;			/* pick only one object */
	    }
	}
	return FALSE;
}

#endif /* OVL2 */
#ifdef OVL0

int
curr_mon_load(mtmp)
register struct monst *mtmp;
{
	register int curload = 0;
	register struct obj *obj;

	for(obj = mtmp->minvent; obj; obj = obj->nobj) {
		if(!is_boulder(obj) || !throws_rocks(mtmp->data))
			curload += obj->owt;
	}

	return curload;
}

int
max_mon_load(mtmp)
register struct monst *mtmp;
{
	long maxload = MAX_CARR_CAP;
	long carcap = 25L*(acurrstr((int)(mtmp->mstr)) + mtmp->mcon) + 50L;


	if (!mtmp->data->cwt){
		maxload = (maxload * (long)mtmp->data->msize) / MZ_HUMAN;
		carcap = (carcap * (long)mtmp->data->msize) / MZ_HUMAN;
	} else if (!strongmonst(mtmp->data)
		|| (strongmonst(mtmp->data) && (mtmp->data->cwt > WT_HUMAN))
	){
		maxload = (maxload * (long)mtmp->data->cwt) / WT_HUMAN;
		carcap = (carcap * (long)mtmp->data->cwt) / WT_HUMAN;
	}

	if (carcap > maxload) carcap = maxload;

	if (carcap < 1) carcap = 1;

	return (int) carcap;
}

boolean
mon_can_see_mon(looker, lookie)
	struct monst *looker;
	struct monst *lookie;
{
	boolean catsightdark = !(levl[looker->mx][looker->my].lit || (viz_array[looker->my][looker->mx]&TEMP_LIT1 && !(viz_array[looker->my][looker->mx]&TEMP_DRK1))) ||
							(!levl[looker->mx][looker->my].lit && !(viz_array[looker->my][looker->mx]&TEMP_DRK1 && !(viz_array[looker->my][looker->mx]&TEMP_LIT1)));
	
	if(looker->data == &mons[PM_DREADBLOSSOM_SWARM]){
		if(lookie->data == &mons[PM_DREADBLOSSOM_SWARM]) return FALSE;
		else return mon_can_see_mon(lookie, looker);
	}
	if(looker->data == &mons[PM_SWARM_OF_SNAKING_TENTACLES] || looker->data == &mons[PM_LONG_SINUOUS_TENTACLE]){
		struct monst *witw;
		for(witw = fmon; witw; witw = witw->nmon) if(witw->data == &mons[PM_WATCHER_IN_THE_WATER]) break;
		if(witw){
			looker->mux = witw->mux;
			looker->muy = witw->muy;
			if(mon_can_see_mon(witw, lookie)) return TRUE;
		}
		//may still be able to feel target adjacent
	}
	
	if(looker->data == &mons[PM_WIDE_CLUBBED_TENTACLE]){
		struct monst *keto;
		for(keto = fmon; keto; keto = keto->nmon) if(keto->data == &mons[PM_KETO]) break;
		if(keto){
			looker->mux = keto->mux;
			looker->muy = keto->muy;
			if(mon_can_see_mon(keto, lookie)) return TRUE;
		}
		//may still be able to feel target adjacent
	}
	
	if(looker->data == &mons[PM_DANCING_BLADE]){
		struct monst *surya;
		for(surya = fmon; surya; surya = surya->nmon) if(surya->m_id == looker->mvar1) break;
		if(surya){
			looker->mux = surya->mux;
			looker->muy = surya->muy;
			if(mon_can_see_mon(surya, lookie)) return TRUE;
		}
		return FALSE;
		//can't feel target adjacent
	}
	
	
	if(distmin(looker->mx,looker->my,lookie->mx,lookie->my) <= 1 && !rn2(8)) return TRUE;
	if((darksight(looker->data) || (catsight(looker->data) && catsightdark)) && !is_blind(looker)){
		if(distmin(looker->mx,looker->my,lookie->mx,lookie->my) <= 1) return TRUE;
		if(clear_path(looker->mx, looker->my, lookie->mx, lookie->my) && !(lookie->minvis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			if(levl[lookie->mx][lookie->my].lit){
				if(viz_array[lookie->my][lookie->mx]&TEMP_DRK1 && !(viz_array[lookie->my][lookie->mx]&TEMP_LIT1))
					return TRUE;
			} else {
				if(!(viz_array[lookie->my][lookie->mx]&TEMP_LIT1 && !(viz_array[lookie->my][lookie->mx]&TEMP_DRK1)))
					return TRUE;
			}
		}
	}
	if(lowlightsight3(looker->data) && !is_blind(looker)){
		if(clear_path(looker->mx, looker->my, lookie->mx, lookie->my) && !(lookie->minvis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			if(dist2(looker->mx,looker->my,lookie->mx,lookie->my) <= 3*3) return TRUE;
			else if(levl[lookie->mx][lookie->my].lit){
				if(!(viz_array[lookie->my][lookie->mx]&TEMP_DRK3 && !(viz_array[lookie->my][lookie->mx]&TEMP_LIT3)))
					return TRUE;
			} else {
				if(viz_array[lookie->my][lookie->mx]&TEMP_LIT3 && !(viz_array[lookie->my][lookie->mx]&TEMP_DRK1))
					return TRUE;
				else if(viz_array[lookie->my][lookie->mx]&TEMP_LIT2 && !(viz_array[lookie->my][lookie->mx]&TEMP_DRK2))
					return TRUE;
				else if(viz_array[lookie->my][lookie->mx]&TEMP_LIT1 && !(viz_array[lookie->my][lookie->mx]&TEMP_DRK3))
					return TRUE;
			}
		}
	}
	if(lowlightsight2(looker->data) && !is_blind(looker)){
		if(clear_path(looker->mx, looker->my, lookie->mx, lookie->my) && !(lookie->minvis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			if(dist2(looker->mx,looker->my,lookie->mx,lookie->my) <= 2*2) return TRUE;
			else if(levl[lookie->mx][lookie->my].lit){
				if(!(viz_array[lookie->my][lookie->mx]&TEMP_DRK2 && !(viz_array[lookie->my][lookie->mx]&TEMP_LIT2)) &&
					!(viz_array[lookie->my][lookie->mx]&TEMP_DRK3 && !(viz_array[lookie->my][lookie->mx]&TEMP_LIT1))
				)
					return TRUE;
			} else {
				if(viz_array[lookie->my][lookie->mx]&TEMP_LIT2 && !(viz_array[lookie->my][lookie->mx]&TEMP_DRK1))
					return TRUE;
				else if(viz_array[lookie->my][lookie->mx]&TEMP_LIT1 && !(viz_array[lookie->my][lookie->mx]&TEMP_DRK2))
					return TRUE;
			}
		}
	}
	if((normalvision(looker->data) || (catsight(looker->data) && !catsightdark)) && !is_blind(looker)){
		if(clear_path(looker->mx, looker->my, lookie->mx, lookie->my) && !(lookie->minvis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			if(distmin(looker->mx,looker->my,lookie->mx,lookie->my) <= 1) return TRUE;
			else if(levl[lookie->mx][lookie->my].lit){
				if(!(viz_array[lookie->my][lookie->mx]&TEMP_DRK1 && !(viz_array[lookie->my][lookie->mx]&TEMP_LIT1)) &&
					!(viz_array[lookie->my][lookie->mx]&TEMP_DRK2)
				)
					return TRUE;
			} else {
				if(viz_array[lookie->my][lookie->mx]&TEMP_LIT1 && !(viz_array[lookie->my][lookie->mx]&TEMP_DRK1))
					return TRUE;
			}
		}
	}
	if(echolocation(looker->data) && !is_deaf(looker) && !unsolid(lookie->data)){
		if(clear_path(looker->mx, looker->my, lookie->mx, lookie->my)){
			return TRUE;
		}
	}
	if(extramission(looker->data)){
		if(clear_path(looker->mx, looker->my, lookie->mx, lookie->my) && !(lookie->minvis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			return TRUE;
		}
	}
	if(infravision(looker->data) && infravisible(youracedata) && !is_blind(looker)){
		if((clear_path(looker->mx, looker->my, lookie->mx, lookie->my) || ominsense(looker->data)) && !(lookie->minvis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			return TRUE;
		}
	}
	if(bloodsense(looker->data) && has_blood(youracedata)){
		if((clear_path(looker->mx, looker->my, lookie->mx, lookie->my) || ominsense(looker->data)) && !(lookie->minvis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			return TRUE;
		}
	}
	if(lifesense(looker->data) && !nonliving(youracedata)){
		if((clear_path(looker->mx, looker->my, lookie->mx, lookie->my) || ominsense(looker->data)) && !(lookie->minvis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			return TRUE;
		}
	}
	if(senseall(looker->data)){
		if((clear_path(looker->mx, looker->my, lookie->mx, lookie->my) || ominsense(looker->data)) && !(lookie->minvis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			return TRUE;
		}
	}
	if(earthsense(looker->data) && !(mon_resistance(lookie,FLYING) || mon_resistance(lookie,LEVITATION) || unsolid(lookie->data))){
		return TRUE;
	}
	if(mon_resistance(looker,TELEPAT)){
		return TRUE;
	}
	if(goodsmeller(looker->data) && distmin(lookie->mx, lookie->my, looker->mx, looker->my) <= 6){
	/*sanity check: don't bother trying to path to it if it is farther than a path can possibly exist*/
		if(clear_path(lookie->mx, lookie->my, looker->mx, looker->my)){
		/*don't running a complicated path function if there is a straight line to you*/
			return TRUE;
		} else {
			boolean shouldsmell = FALSE;
			scentgoalx = lookie->mx;
			scentgoaly = lookie->my;
			xpathto(6, looker->mx, looker->my, scent_callback, (genericptr_t)&shouldsmell);
			if(shouldsmell){
				return TRUE;
			}
		}
	}
	return FALSE;
}

boolean
mon_can_see_you(looker)
	struct monst *looker;
{
	boolean catsightdark = !(levl[looker->mx][looker->my].lit || (viz_array[looker->my][looker->mx]&TEMP_LIT1 && !(viz_array[looker->my][looker->mx]&TEMP_DRK1)));
	
	
	if(looker->data == &mons[PM_DREADBLOSSOM_SWARM]){
		if(youracedata == &mons[PM_DREADBLOSSOM_SWARM]) return FALSE;
		else return canseemon(looker);
	}
	
	if(looker->data == &mons[PM_SWARM_OF_SNAKING_TENTACLES] || looker->data == &mons[PM_LONG_SINUOUS_TENTACLE]){
		struct monst *witw;
		for(witw = fmon; witw; witw = witw->nmon) if(witw->data == &mons[PM_WATCHER_IN_THE_WATER]) break;
		if(witw && mon_can_see_you(witw)) return TRUE;
		//may still be able to feel target adjacent
	}
	
	if(looker->data == &mons[PM_WIDE_CLUBBED_TENTACLE]){
		struct monst *keto;
		for(keto = fmon; keto; keto = keto->nmon) if(keto->data == &mons[PM_KETO]) break;
		if(keto && mon_can_see_you(keto)) return TRUE;
		//may still be able to feel target adjacent
	}
	
	if(looker->data == &mons[PM_DANCING_BLADE]){
		struct monst *surya;
		for(surya = fmon; surya; surya = surya->nmon) if(surya->m_id == looker->mvar1) break;
		if(surya && mon_can_see_you(surya)) return TRUE;
		return FALSE;
		//can't feel target adjacent
	}
	
	if(Aggravate_monster) return TRUE;
	
	if(distmin(looker->mx,looker->my,u.ux,u.uy) <= 1 && !rn2(8)) return TRUE;
	
	if((darksight(looker->data) || (catsight(looker->data) && catsightdark)) && !is_blind(looker)){
		if(couldsee(looker->mx, looker->my) && !(Invis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			if(levl[u.ux][u.uy].lit){
				if(viz_array[u.uy][u.ux]&TEMP_DRK1 && !(viz_array[u.uy][u.ux]&TEMP_LIT1))
					return TRUE;
			} else {
				if(!(viz_array[u.uy][u.ux]&TEMP_LIT1 && !(viz_array[u.uy][u.ux]&TEMP_DRK1)))
					return TRUE;
			}
		}
	}
	if(lowlightsight3(looker->data) && !is_blind(looker)){
		if(couldsee(looker->mx, looker->my) && !(Invis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			if(dist2(looker->mx,looker->my,u.ux,u.uy) <= 3*3) return TRUE;
			else if(levl[u.ux][u.uy].lit){
				if(!(viz_array[u.uy][u.ux]&TEMP_DRK3 && !(viz_array[u.uy][u.ux]&TEMP_LIT3)))
					return TRUE;
			} else {
				if(viz_array[u.uy][u.ux]&TEMP_LIT3 && !(viz_array[u.uy][u.ux]&TEMP_DRK1))
					return TRUE;
				else if(viz_array[u.uy][u.ux]&TEMP_LIT2 && !(viz_array[u.uy][u.ux]&TEMP_DRK2))
					return TRUE;
				else if(viz_array[u.uy][u.ux]&TEMP_LIT1 && !(viz_array[u.uy][u.ux]&TEMP_DRK3))
					return TRUE;
			}
		}
	}
	if(lowlightsight2(looker->data) && !is_blind(looker)){
		if(couldsee(looker->mx, looker->my) && !(Invis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			if(dist2(looker->mx,looker->my,u.ux,u.uy) <= 2*2) return TRUE;
			else if(levl[u.ux][u.uy].lit){
				if(!(viz_array[u.uy][u.ux]&TEMP_DRK2 && !(viz_array[u.uy][u.ux]&TEMP_LIT2)) &&
					!(viz_array[u.uy][u.ux]&TEMP_DRK3 && !(viz_array[u.uy][u.ux]&TEMP_LIT1))
				)
					return TRUE;
			} else {
				if(viz_array[u.uy][u.ux]&TEMP_LIT2 && !(viz_array[u.uy][u.ux]&TEMP_DRK1))
					return TRUE;
				else if(viz_array[u.uy][u.ux]&TEMP_LIT1 && !(viz_array[u.uy][u.ux]&TEMP_DRK2))
					return TRUE;
			}
		}
	}
	if((normalvision(looker->data) || (catsight(looker->data) && !catsightdark)) && !is_blind(looker)){
		if(couldsee(looker->mx, looker->my) && !(Invis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			if(distmin(looker->mx,looker->my,u.ux,u.uy) <= 1) return TRUE;
			else if(levl[u.ux][u.uy].lit){
				if(!(viz_array[u.uy][u.ux]&TEMP_DRK1 && !(viz_array[u.uy][u.ux]&TEMP_LIT1)) &&
					!(viz_array[u.uy][u.ux]&TEMP_DRK2)
				)
					return TRUE;
			} else {
				if(viz_array[u.uy][u.ux]&TEMP_LIT1 && !(viz_array[u.uy][u.ux]&TEMP_DRK1))
					return TRUE;
			}
		}
	}
	if(echolocation(looker->data) && !is_deaf(looker) && !unsolid(youracedata)){
		if(couldsee(looker->mx, looker->my)){
			return TRUE;
		}
	}
	if(extramission(looker->data)){
		if(couldsee(looker->mx, looker->my) && !(Invis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			return TRUE;
		}
	}
	if(infravision(looker->data) && infravisible(youracedata) && !is_blind(looker)){
		if((couldsee(looker->mx, looker->my) || ominsense(looker->data)) && !(Invis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			return TRUE;
		}
	}
	if(bloodsense(looker->data) && has_blood(youracedata)){
		if((couldsee(looker->mx, looker->my) || ominsense(looker->data)) && !(Invis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			return TRUE;
		}
	}
	if(lifesense(looker->data) && !nonliving(youracedata)){
		if((couldsee(looker->mx, looker->my) || ominsense(looker->data)) && !(Invis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			return TRUE;
		}
	}
	if(senseall(looker->data)){
		if((couldsee(looker->mx, looker->my) || ominsense(looker->data)) && !(Invis && !mon_resistance(looker,SEE_INVIS) && !can_track(looker->data) && rn2(11))){
			return TRUE;
		}
	}
	if(earthsense(looker->data) && !(Flying || Levitation || unsolid(youracedata) || Stealth)){
		return TRUE;
	}
	if(mon_resistance(looker,TELEPAT)){
		return TRUE;
	}
	if(goodsmeller(looker->data) && distmin(u.ux, u.uy, looker->mx, looker->my) <= 6){
	/*sanity check: don't bother trying to path to it if it is farther than a path can possibly exist*/
		if(clear_path(u.ux, u.uy, looker->mx, looker->my)){
		/*don't running a complicated path function if there is a straight line to you*/
			return TRUE;
		} else {
			boolean shouldsmell = FALSE;
			scentgoalx = u.ux;
			scentgoaly = u.uy;
			xpathto(6, looker->mx, looker->my, scent_callback, (genericptr_t)&shouldsmell);
			if(shouldsmell){
				return TRUE;
			}
		}
	}
	return FALSE;
}

STATIC_DCL int
scent_callback(data, x, y)
genericptr_t data;
int x, y;
{
    int is_accessible = ZAP_POS(levl[x][y].typ);
    boolean *shouldsmell = (boolean *)data;
    if (scentgoalx == x && scentgoaly == y) *shouldsmell = TRUE;
	
	if(!(*shouldsmell)) return !is_accessible;
	else return 1; /* Once a path to you is found, quickly end the xpath function */
}

/* for restricting monsters' object-pickup */
boolean
can_carry(mtmp,otmp)
struct monst *mtmp;
struct obj *otmp;
{
	int otyp = otmp->otyp, newload = otmp->owt;
	struct permonst *mdat = mtmp->data;

	if (notake(mdat)) return FALSE;		/* can't carry anything */

	if (otyp == CORPSE && touch_petrifies(&mons[otmp->corpsenm]) &&
		!(mtmp->misc_worn_check & W_ARMG) && !resists_ston(mtmp))
	    return FALSE;
	if (otyp == CORPSE && is_rider(&mons[otmp->corpsenm]))
	    return FALSE;
	if (otmp->obj_material == SILVER && hates_silver(mdat) &&
		(otyp != BELL_OF_OPENING || !is_covetous(mdat)))
	    return FALSE;
	if (otmp->obj_material == IRON && hates_iron(mdat))
	    return FALSE;

#ifdef STEED
	/* Steeds don't pick up stuff (to avoid shop abuse) */
	if (mtmp == u.usteed) return (FALSE);
#endif
	if (mtmp->isshk) return(TRUE); /* no limit */
	if ((mtmp->mpeaceful && mtmp->data != &mons[PM_MAID]) && !mtmp->mtame) return(FALSE);
	/* otherwise players might find themselves obligated to violate
	 * their alignment if the monster takes something they need
	 * 
	 * Maids, on the other hand, will vaccum up everything on the floor. 
	 * But if the player can't tame or assasinate one midlevel monster they
	 * deserve what's coming ;-)
	 */

	/* special--boulder throwers carry unlimited amounts of boulders */
	if (throws_rocks(mdat) && is_boulder(otmp))
		return(TRUE);

	/* nymphs deal in stolen merchandise, but not boulders or statues */
	if (mdat->mlet == S_NYMPH)
		return (boolean)(otmp->oclass != ROCK_CLASS);

	if (curr_mon_load(mtmp) + newload > max_mon_load(mtmp)) return FALSE;

	return(TRUE);
}

/* return number of acceptable neighbour positions */
int
mfndpos(mon, poss, info, flag)
	register struct monst *mon;
	coord *poss;	/* coord poss[9] */
	long *info;	/* long info[9] */
	long flag;
{
	struct permonst *mdat = mon->data;
	struct monst *witw = 0;
	struct monst *madjacent = 0;
	register xchar x,y,nx,ny;
	register int cnt = 0;
	register uchar ntyp;
	uchar nowtyp;
	boolean wantpool,wantpuddle,poolok,cubewaterok,lavaok,nodiag,quantumlock;
	boolean rockok = FALSE, treeok = FALSE, thrudoor;
	int maxx, maxy;
	
	if(mdat == &mons[PM_SWARM_OF_SNAKING_TENTACLES] || mdat == &mons[PM_LONG_SINUOUS_TENTACLE]){
		for(witw = fmon; witw; witw = witw->nmon) if(witw->data == &mons[PM_WATCHER_IN_THE_WATER]) break;
	}
	
	if(mdat == &mons[PM_WIDE_CLUBBED_TENTACLE]){
		for(witw = fmon; witw; witw = witw->nmon) if(witw->data == &mons[PM_KETO]) break;
	}
	
	if(mdat == &mons[PM_DANCING_BLADE]){
		for(madjacent = fmon; madjacent; madjacent = madjacent->nmon)
			if(madjacent->data == &mons[PM_SURYA_DEVA] && madjacent->m_id == mon->mvar1)
				break;
	}
	
	x = mon->mx;
	y = mon->my;
	nowtyp = levl[x][y].typ;

	nodiag = (mdat == &mons[PM_GRID_BUG]) || (mdat == &mons[PM_BEBELITH]);
	wantpool = mdat->mlet == S_EEL;
	wantpuddle = wantpool && mdat->msize == MZ_TINY;
	cubewaterok = (mon_resistance(mon,SWIMMING) || breathless_mon(mon) || amphibious_mon(mon));
	poolok = mon_resistance(mon,FLYING) || is_clinger(mdat) ||
		 (mon_resistance(mon,SWIMMING) && !wantpool);
	lavaok = mon_resistance(mon,FLYING) || is_clinger(mdat) || likes_lava(mdat);
	quantumlock = (is_weeping(mdat) && !u.uevent.invoked);
	thrudoor = ((flag & (ALLOW_WALL|BUSTDOOR)) != 0L);
	if (flag & ALLOW_DIG) {
	    struct obj *mw_tmp;

	    /* need to be specific about what can currently be dug */
	    if (!needspick(mdat)) {
		rockok = treeok = TRUE;
	    } else if ((mw_tmp = MON_WEP(mon)) && mw_tmp->cursed &&
		       mon->weapon_check == NO_WEAPON_WANTED) {
		rockok = is_pick(mw_tmp);
		treeok = is_axe(mw_tmp);
	    } else {
		rockok = (m_carrying(mon, PICK_AXE) ||
			  (m_carrying(mon, DWARVISH_MATTOCK) &&
			   !which_armor(mon, W_ARMS)));
		treeok = (m_carrying(mon, AXE) ||
			  (m_carrying(mon, BATTLE_AXE) &&
			   !which_armor(mon, W_ARMS)));
	    }
	    thrudoor |= rockok || treeok;
	}

nexttry:	/* eels prefer the water, but if there is no water nearby,
		   they will crawl over land */
	if(mon->mconf) {
		flag |= ALLOW_ALL;
		flag &= ~NOTONL;
	}
	if(is_blind(mon))
		flag |= ALLOW_SSM;
	maxx = min(x+1,COLNO-1);
	maxy = min(y+1,ROWNO-1);
	for(nx = max(1,x-1); nx <= maxx; nx++)
	  for(ny = max(0,y-1); ny <= maxy; ny++) {
	    if(nx == x && ny == y) continue;
	    if(IS_ROCK(ntyp = levl[nx][ny].typ) &&
	       !((flag & ALLOW_WALL) && may_passwall(nx,ny)) &&
	       !((IS_TREE(ntyp) ? treeok : rockok) && may_dig(nx,ny))) continue;
	    /* KMH -- Added iron bars */
	    if (ntyp == IRONBARS && !(flag & ALLOW_BARS)) continue;
	    /* ALI -- Artifact doors (no passage unless open/openable) from Slash'em*/
	    if (IS_DOOR(ntyp))
			if (artifact_door(nx, ny) ?
				(levl[nx][ny].doormask & D_CLOSED && !(flag & OPENDOOR))
				  || levl[nx][ny].doormask & D_LOCKED :
				!amorphous(mdat) &&
			   ((levl[nx][ny].doormask & D_CLOSED && !(flag & OPENDOOR)) ||
			(levl[nx][ny].doormask & D_LOCKED && !(flag & UNLOCKDOOR))) &&
			   !thrudoor) continue;
	    if(nx != x && ny != y && (nodiag || mdat == &mons[PM_LONG_WORM] ||
#ifdef REINCARNATION
	       ((IS_DOOR(nowtyp) &&
		 ((levl[x][y].doormask & ~D_BROKEN) || Is_rogue_level(&u.uz))) ||
		(IS_DOOR(ntyp) &&
		 ((levl[nx][ny].doormask & ~D_BROKEN) || Is_rogue_level(&u.uz))))
#else
	       ((IS_DOOR(nowtyp) && (levl[x][y].doormask & ~D_BROKEN)) ||
		(IS_DOOR(ntyp) && (levl[nx][ny].doormask & ~D_BROKEN)))
#endif
	       ))
			continue;
		if(mdat == &mons[PM_CLOCKWORK_SOLDIER] || mdat == &mons[PM_CLOCKWORK_DWARF] || 
		   mdat == &mons[PM_FABERGE_SPHERE] || mdat == &mons[PM_FIREWORK_CART] || 
		   mdat == &mons[PM_JUGGERNAUT] || mdat == &mons[PM_ID_JUGGERNAUT]
		) if(x + xdir[(int)mon->mvar1] != nx || 
			   y + ydir[(int)mon->mvar1] != ny 
			) continue;
		if((mdat == &mons[PM_GRUE]) && isdark(mon->mx, mon->my) && !isdark(nx, ny))
				continue;
		if((mdat == &mons[PM_WATCHER_IN_THE_WATER] || mdat == &mons[PM_KETO]) && 
			distmin(nx, ny, mon->mux, mon->muy) <= 3 && 
			dist2(nx, ny, mon->mux, mon->muy) <= dist2(mon->mx, mon->my, mon->mux, mon->muy)) continue;
		if((mdat == &mons[PM_WATCHER_IN_THE_WATER]) && 
			onlineu(nx, ny) && (lined_up(mon) || !rn2(4))) continue;
		if(witw && dist2(nx, ny, witw->mx, witw->my) > 32 && 
			dist2(nx, ny, witw->mx, witw->my) >= dist2(mon->mx, mon->my, witw->mx, witw->my)) continue;
		if(madjacent && distmin(nx, ny, madjacent->mx, madjacent->my) > 1 && 
			dist2(nx, ny, madjacent->mx, madjacent->my) >= dist2(mon->mx, mon->my, madjacent->mx, madjacent->my) &&
			!(m_at(nx, ny) && distmin(nx, ny, madjacent->mx, madjacent->my) <= 2)) continue;
		if(mdat == &mons[PM_HOOLOOVOO] && 
			IS_WALL(levl[mon->mx][mon->my].typ) &&
			!IS_WALL(levl[nx][ny].typ)
		) continue;
		//Weeping angels should avoid stepping into corredors, where they can be forced into a standoff.
		if(quantumlock && IS_ROOM(levl[mon->mx][mon->my].typ) && !IS_ROOM(ntyp) ) continue;
		
		if((is_pool(nx,ny, wantpuddle) == wantpool || poolok) &&
			(cubewaterok || !is_3dwater(nx,ny)) && 
			(lavaok || !is_lava(nx,ny))) {
		int dispx, dispy;
		boolean monseeu = (!is_blind(mon) && (!Invis || mon_resistance(mon,SEE_INVIS)));
		boolean checkobj = OBJ_AT(nx,ny);
		
		/* Displacement also displaces the Elbereth/scare monster,
		 * as long as you are visible.
		 */
		if(Displaced && monseeu && (mon->mux==nx) && (mon->muy==ny)) {
		    dispx = u.ux;
		    dispy = u.uy;
		} else {
		    dispx = nx;
		    dispy = ny;
		}

		info[cnt] = 0;
		if ((checkobj || Displaced) && onscary(dispx, dispy, mon)) {
		    if(!(flag & ALLOW_SSM)) continue;
		    info[cnt] |= ALLOW_SSM;
		}
		if((nx == u.ux && ny == u.uy) ||
		   (nx == mon->mux && ny == mon->muy)) {
			if (nx == u.ux && ny == u.uy) {
				/* If it's right next to you, it found you,
				 * displaced or no.  We must set mux and muy
				 * right now, so when we return we can tell
				 * that the ALLOW_U means to attack _you_ and
				 * not the image.
				 */
				mon->mux = u.ux;
				mon->muy = u.uy;
			}
			if(!(flag & ALLOW_U)) continue;
			info[cnt] |= ALLOW_U;
		} else {
			if(MON_AT(nx, ny)) {
				struct monst *mtmp2 = m_at(nx, ny);
				long mmflag = flag | mm_aggression(mon, mtmp2);

				if (!(mmflag & ALLOW_M)) continue;
				info[cnt] |= ALLOW_M;
				if (mtmp2->mtame) {
					if (!(mmflag & ALLOW_TM)) continue;
					info[cnt] |= ALLOW_TM;
				}
			}
			/* Note: ALLOW_SANCT only prevents movement, not */
			/* attack, into a temple. */
			if(level.flags.has_temple &&
			   *in_rooms(nx, ny, TEMPLE) &&
			   !*in_rooms(x, y, TEMPLE) &&
			   in_your_sanctuary((struct monst *)0, nx, ny)) {
				if(!(flag & ALLOW_SANCT)) continue;
				info[cnt] |= ALLOW_SANCT;
			}
		}
		if(checkobj && sobj_at(CLOVE_OF_GARLIC, nx, ny)) {
			if(flag & NOGARLIC) continue;
			info[cnt] |= NOGARLIC;
		}
		if(checkobj && boulder_at(nx, ny)) {
			if(!(flag & ALLOW_ROCK)) continue;
			info[cnt] |= ALLOW_ROCK;
		}
		if (monseeu && onlineu(nx,ny)) {
			if(flag & NOTONL) continue;
			info[cnt] |= NOTONL;
		}
		if (levl[nx][ny].typ == CLOUD && Is_lolth_level(&u.uz) && !(nonliving_mon(mon) || breathless_mon(mon) || resists_poison(mon))) {
			if(!(flag & ALLOW_TRAPS)) continue;
			info[cnt] |= ALLOW_TRAPS;
		}
		if (nx != x && ny != y && bad_rock(mon, x, ny)
			    && bad_rock(mon, nx, y)
			    && ((bigmonst(mdat) && !amorphous(mdat)) || (curr_mon_load(mon) > 600)))
			continue;
		/* The monster avoids a particular type of trap if it's familiar
		 * with the trap type.  Pets get ALLOW_TRAPS and checking is
		 * done in dogmove.c.  In either case, "harmless" traps are
		 * neither avoided nor marked in info[].
		 */
		{ register struct trap *ttmp = t_at(nx, ny);
		    if(ttmp) {
			if(ttmp->ttyp >= TRAPNUM || ttmp->ttyp == 0)  {
impossible("A monster looked at a very strange trap of type %d.", ttmp->ttyp);
			    continue;
			}
			struct obj *mwep;
			mwep = MON_WEP(mon);
			if ((ttmp->ttyp != RUST_TRAP
					|| mdat == &mons[PM_IRON_GOLEM]
					|| mdat == &mons[PM_CHAIN_GOLEM])
				&& ((ttmp->ttyp != PIT
				    && ttmp->ttyp != SPIKED_PIT
				    && ttmp->ttyp != TRAPDOOR
				    && ttmp->ttyp != HOLE)
				      || (!mon_resistance(mon,FLYING)
				    && !mon_resistance(mon,LEVITATION)
				    && !is_clinger(mdat))
				      || In_sokoban(&u.uz))
				&& (ttmp->ttyp != SLP_GAS_TRAP ||
				    !resists_sleep(mon))
				&& (ttmp->ttyp != BEAR_TRAP ||
				    (mdat->msize > MZ_SMALL &&
				     !amorphous(mdat) && !mon_resistance(mon,FLYING)))
				&& (ttmp->ttyp != FIRE_TRAP ||
				    !resists_fire(mon) || distmin(mon->mx, mon->my, mon->mux, mon->muy) > 2) /*Cuts down on plane of fire message spam*/
				&& (ttmp->ttyp != SQKY_BOARD || !mon_resistance(mon,FLYING))
				&& (ttmp->ttyp != WEB || (!amorphous(mdat) &&
				    !(webmaker(mdat) || (Is_lolth_level(&u.uz) && !mon->mpeaceful)) && !(
						 mdat->mlet == S_GIANT ||
						(mdat->mlet == S_DRAGON &&
						extra_nasty(mdat)) || /* excl. babies */
						(mon->wormno && count_wsegs(mon) > 5)
					) && 
					!(mwep && (mwep->oartifact == ART_STING || 
						mwep->oartifact == ART_LIECLEAVER))))
			) {
			    if (!(flag & ALLOW_TRAPS)) {
				if (mon->mtrapseen & (1L << (ttmp->ttyp - 1)) || mon_resistance(mon, SEARCHING))
				    continue;
			    }
			    info[cnt] |= ALLOW_TRAPS;
			}
		    }
		}
		poss[cnt].x = nx;
		poss[cnt].y = ny;
		cnt++;
	    }
	}
	if(!cnt && wantpool && !is_pool(x,y, wantpuddle)) {
		wantpool = FALSE;
		goto nexttry;
	}
	return(cnt);
}

#endif /* OVL0 */
#ifdef OVL1

/* Monster against monster special attacks; for the specified monster
   combinations, this allows one monster to attack another adjacent one
   in the absence of Conflict. Incorporates changes from grudge mod */
long
mm_aggression(magr, mdef)
struct monst *magr,	/* monster that is currently deciding where to move */
	     *mdef;	/* another monster which is next to it */
{
	struct permonst *ma,*md;

	ma = magr->data;
	md = mdef->data;
	
	// if(magr->mpeaceful && mdef->mpeaceful && (magr->mtame || mdef->mtame)) return 0L;
	
	if(magr->mtame && (mdef->mtame || mdef->moccupation)){
			return 0L;
	}
	
	if(mdef->mtrapped && t_at(mdef->mx, mdef->my) && t_at(mdef->mx, mdef->my)->ttyp == VIVI_TRAP){
			return 0L;
	}
	
	if(!mon_can_see_mon(magr, mdef)) return 0L;
	
	if(magr->mtame && mdef->mpeaceful && !u.uevent.uaxus_foe && md == &mons[PM_AXUS])
		return 0L;
	
	if(ma == &mons[PM_DREADBLOSSOM_SWARM]){
		if(!(is_fey(md) || is_plant(md))) return ALLOW_M|ALLOW_TM;
	}
	if(ma == &mons[PM_GRUE]){
		if(isdark(mdef->mx, mdef->my)) return ALLOW_M|ALLOW_TM;
	}
	
	if((ma == &mons[PM_OONA] || ma == &mons[PM_OONA])
		&& sgn(ma->maligntyp) == -1*sgn(md->maligntyp) //"Oona grudges on chaotics, but not on neutrals"
		&& !(magr->mtame || mdef->mtame) //Normal pet-vs-monster logic should take precedence over this
	){
		return ALLOW_M|ALLOW_TM;
	}
	
	
	/* In the anachrononaut quest, all peaceful monsters are at threat from all hostile monsters.
		The leader IS in serious danger */
	if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz) && Is_qstart(&u.uz)){
		if(magr->mpeaceful != mdef->mpeaceful) return ALLOW_M|ALLOW_TM;
		else return 0L;
	}
	
	if(magr->mberserk) return ALLOW_M|ALLOW_TM;
	
	if(touch_petrifies(md) && !resists_ston(magr) 
		&& distmin(magr->mx, magr->my, mdef->mx, mdef->my) < 3
		&& !MON_WEP(magr)
	)
		return 0L;
	
	if(md == &mons[PM_MANDRAKE]) return 0L;
	
	if(is_drow(ma) && is_drow(md) && (magr->mfaction == mdef->mfaction)) return 0L;
	
	if (u.sowdisc && !mdef->mtame && canseemon(magr) && canseemon(mdef))
	    return ALLOW_M|ALLOW_TM;
	/* supposedly purple worms are attracted to shrieking because they
	   like to eat shriekers, so attack the latter when feasible */
	if (ma == &mons[PM_PURPLE_WORM] &&
		md == &mons[PM_SHRIEKER])
			return ALLOW_M|ALLOW_TM;

#ifdef ATTACK_PETS
        /* pets attack hostile monsters */
	if (magr->mtame && !mdef->mpeaceful)
	    return ALLOW_M|ALLOW_TM;
	
        /* and vice versa */
	if (mdef->mtame && !magr->mpeaceful && !mdef->meating && mdef != u.usteed && !mdef->mflee)
	    return ALLOW_M|ALLOW_TM;
#endif /* ATTACK_PETS */

	/* Gugs and ghouls fight each other. 
		Gugs are afraid of
		Ghouls, so they won't attack first. */
	if (ma == &mons[PM_GHOUL] &&
		md == &mons[PM_GUG])
			return ALLOW_M|ALLOW_TM;

	/* Since the quest guardians are under siege, it makes sense to have 
       them fight hostiles.  (But we don't want the quest leader to be in danger.) */
	if( (ma->msound==MS_GUARDIAN 
		  || (Role_if(PM_NOBLEMAN) && (ma == &mons[PM_KNIGHT] || ma == &mons[PM_MAID] || ma == &mons[PM_PEASANT]) && magr->mpeaceful && In_quest(&u.uz))
		  || (Role_if(PM_KNIGHT) && ma == &mons[PM_KNIGHT] && magr->mpeaceful && In_quest(&u.uz))
		  || (Race_if(PM_DROW) && is_drow(ma) && magr->mfaction == u.uhouse)
		  || (Race_if(PM_GNOME) && (is_gnome(ma) && !is_undead_mon(magr)) && magr->mpeaceful)
		)
		&& !(Race_if(PM_DROW) && !(flags.stag || Role_if(PM_NOBLEMAN) || !is_drow(md)))
		&& !(Role_if(PM_EXILE))
	) {
		if(magr->mpeaceful==TRUE && mdef->mpeaceful==FALSE) return ALLOW_M|ALLOW_TM;
		if(magr->mpeaceful==TRUE && mdef->mtame==TRUE) return FALSE;
	}
	/* and vice versa */
	if( (md->msound == MS_GUARDIAN 
		  || (Role_if(PM_NOBLEMAN) && (md == &mons[PM_KNIGHT] || md == &mons[PM_MAID] || md == &mons[PM_PEASANT]) && mdef->mpeaceful && In_quest(&u.uz))
		  || (Role_if(PM_KNIGHT) && md == &mons[PM_KNIGHT] && mdef->mpeaceful && In_quest(&u.uz))
		  || (Race_if(PM_DROW) && is_drow(md) && mdef->mfaction == u.uhouse)
		  || (Race_if(PM_GNOME) && (is_gnome(md) && !is_undead_mon(mdef)) && mdef->mpeaceful)
		)
		&& !(Race_if(PM_DROW) && !(flags.stag || Role_if(PM_NOBLEMAN) || !is_drow(ma)))
		&& !(Role_if(PM_EXILE))
	){
		if(mdef->mpeaceful==TRUE && magr->mpeaceful==FALSE && rn2(2)) return ALLOW_M|ALLOW_TM;
		if(mdef->mpeaceful==TRUE && magr->mtame==TRUE) return FALSE;
	}

	/* elves vs. orcs */
	if(is_elf(ma) && (is_orc(md) || is_ogre(md) || is_undead_mon(mdef))
				&&	!(is_orc(ma) || is_ogre(ma) || is_undead_mon(magr)))
		return ALLOW_M|ALLOW_TM;
	/* and vice versa */
	if(is_elf(md) && (is_orc(ma) || is_ogre(ma) || is_undead_mon(magr))
				&&	!(is_orc(md) || is_ogre(md) || is_undead_mon(mdef)))
		return ALLOW_M|ALLOW_TM;

	/* dwarves vs. orcs */
	if(is_dwarf(ma) && (is_orc(md) || is_ogre(md) || is_troll(md))
					&&!(is_orc(ma) || is_ogre(ma) || is_troll(ma) || is_undead_mon(magr)))
		return ALLOW_M|ALLOW_TM;
	/* and vice versa */
	if(is_dwarf(md) && (is_orc(ma) || is_ogre(ma) || is_troll(ma))
					&&!(is_orc(md) || is_ogre(md) || is_troll(md) || is_undead_mon(mdef)))
		return ALLOW_M|ALLOW_TM;

	/* elves vs. drow */
	if(is_elf(ma) && is_drow(md) && mdef->mfaction != EILISTRAEE_SYMBOL)
		return ALLOW_M|ALLOW_TM;
	if(is_elf(md) && is_drow(ma) && magr->mfaction != EILISTRAEE_SYMBOL)
		return ALLOW_M|ALLOW_TM;

	/* undead vs civs */
	if(!(In_quest(&u.uz) || u.uz.dnum == temple_dnum || u.uz.dnum == tower_dnum || In_cha(&u.uz) || Is_rogue_level(&u.uz) || Inhell)){
		if(is_undead_mon(magr) && (!always_hostile_mon(mdef) && !is_undead_mon(mdef) && !(is_animal(md) && !is_domestic(md)) && !mindless_mon(mdef)))
			return ALLOW_M|ALLOW_TM;
		if((!always_hostile_mon(magr) && !is_undead_mon(magr) && !(is_animal(ma) && !is_domestic(ma)) && !mindless_mon(magr)) && is_undead_mon(mdef))
			return ALLOW_M|ALLOW_TM;
	}
	
	/* drow vs. other drow */
	/* Note that factions may be different than the displayed house name, 
		as faction is set during generation and displayed house name goes by equipment! */
	if( is_drow(ma) && is_drow(md) && 
		magr->mfaction != mdef->mfaction
	){
		int f1 = magr->mfaction, f2 = mdef->mfaction;
		boolean truce1 = FALSE, truce2 = FALSE;
				
		if((f1 >= FIRST_GODDESS && f1 <= LAST_GODDESS) ||
			(f1 >= FIRST_TOWER && f1 <= LAST_TOWER)
		) truce1 = TRUE;
		
		if((f2 >= FIRST_GODDESS && f2 <= LAST_GODDESS) ||
			(f2 >= FIRST_TOWER && f2 <= LAST_TOWER)
		) truce2 = TRUE;
		
		if((f1 == XAXOX || f1 == EDDER_SYMBOL) && (f2 == XAXOX || f2 == EDDER_SYMBOL));
		else if(!truce1 && !truce2) return ALLOW_M|ALLOW_TM;
		else if(truce1 && truce2);
		else if(truce1 && !(f2 <= LAST_HOUSE && f2 >= FIRST_HOUSE))
			return ALLOW_M|ALLOW_TM;
		else if(truce2 && !(f1 <= LAST_HOUSE && f1 >= FIRST_HOUSE))
			return ALLOW_M|ALLOW_TM;
	}

	/* drow vs. edderkops */
	if(is_drow(ma) && magr->mfaction != XAXOX && magr->mfaction != EDDER_SYMBOL && md == &mons[PM_EDDERKOP]){
		return ALLOW_M|ALLOW_TM;
	}
	/* and vice versa */
	if(is_drow(md) && mdef->mfaction != XAXOX && mdef->mfaction != EDDER_SYMBOL && ma == &mons[PM_EDDERKOP]){
		return ALLOW_M|ALLOW_TM;
	}
	
	/* Nazgul vs. hobbits */
	if(ma == &mons[PM_NAZGUL] && md == &mons[PM_HOBBIT])
		return ALLOW_M|ALLOW_TM;
	/* and vice versa */
	if(md == &mons[PM_NAZGUL] && ma == &mons[PM_HOBBIT])
		return ALLOW_M|ALLOW_TM;

	/* gnolls vs. minotaurs */
	if(is_gnoll(ma) && is_minotaur(md))
		return ALLOW_M|ALLOW_TM;
	/* and vice versa */
	if(is_gnoll(md) && is_minotaur(ma))
		return ALLOW_M|ALLOW_TM;

	/* angels vs. demons */
	if(is_angel(ma) && (is_demon(md) /*|| md == &mons[PM_FALLEN_ANGEL]*/))
		return ALLOW_M|ALLOW_TM;
	/* and vice versa */
	if(is_angel(md) && (is_demon(ma) /*|| ma == &mons[PM_FALLEN_ANGEL] || ma == &mons[PM_LUCIFER]*/))
		return ALLOW_M|ALLOW_TM;

	/* monadics vs. undead */
	if(ma == &mons[PM_MONADIC_DEVA] && (is_undead_mon(mdef)))
		return ALLOW_M|ALLOW_TM;
	/* and vice versa */
	if(md == &mons[PM_MONADIC_DEVA] && (is_undead_mon(magr)))
		return ALLOW_M|ALLOW_TM;

	/* woodchucks vs. The Oracle */
	if(ma == &mons[PM_WOODCHUCK] && md == &mons[PM_ORACLE])
		return ALLOW_M|ALLOW_TM;

	/* ravens like eyes */
	if(ma == &mons[PM_RAVEN] && md == &mons[PM_FLOATING_EYE])
		return ALLOW_M|ALLOW_TM;

	/* nurses heal adjacent alied monsters */
	if(ma == &mons[PM_NURSE] && mdef->mhp < mdef->mhpmax && magr->mpeaceful == mdef->mpeaceful)
		return ALLOW_M|ALLOW_TM;

	/* dungeon fern spores hate everything */
	if(is_fern_spore(ma) && !is_vegetation(md))
		return ALLOW_M|ALLOW_TM;
	/* and everything hates them */
	if(is_fern_spore(md) && !is_vegetation(ma))
		return ALLOW_M|ALLOW_TM;
	/* everything attacks razorvine */
	// if(md == &mons[PM_RAZORVINE] && !is_vegetation(ma))
		// return ALLOW_M|ALLOW_TM;

	else if (magr->data == &mons[PM_SKELETAL_PIRATE] &&
		mdef->data == &mons[PM_SOLDIER])
	    return ALLOW_M|ALLOW_TM;
	else if (mdef->data == &mons[PM_SKELETAL_PIRATE] &&
		magr->data == &mons[PM_SOLDIER])
	    return ALLOW_M|ALLOW_TM;
	else if (magr->data == &mons[PM_SKELETAL_PIRATE] &&
		mdef->data == &mons[PM_SERGEANT])
	    return ALLOW_M|ALLOW_TM;
	else if (mdef->data == &mons[PM_SKELETAL_PIRATE] &&
		magr->data == &mons[PM_SERGEANT])
	    return ALLOW_M|ALLOW_TM;

	else if (magr->data == &mons[PM_DAMNED_PIRATE] &&
		mdef->data == &mons[PM_SOLDIER])
	    return ALLOW_M|ALLOW_TM;
	else if (mdef->data == &mons[PM_DAMNED_PIRATE] &&
		magr->data == &mons[PM_SOLDIER])
	    return ALLOW_M|ALLOW_TM;
	else if (magr->data == &mons[PM_DAMNED_PIRATE] &&
		mdef->data == &mons[PM_SERGEANT])
	    return ALLOW_M|ALLOW_TM;
	else if (mdef->data == &mons[PM_DAMNED_PIRATE] &&
		magr->data == &mons[PM_SERGEANT])
	    return ALLOW_M|ALLOW_TM;

	else if (magr->data == &mons[PM_SKELETAL_PIRATE] &&
		u.ukinghill)
	    return ALLOW_M|ALLOW_TM;
	else if (magr->data == &mons[PM_DAMNED_PIRATE] &&
		u.ukinghill)
	    return ALLOW_M|ALLOW_TM;
	else if (magr->data == &mons[PM_GITHYANKI_PIRATE] &&
		u.ukinghill)
	    return ALLOW_M|ALLOW_TM;
	else if (mdef->data != &mons[PM_TINKER_GNOME] && mdef->data != &mons[PM_HOOLOOVOO] && 
			(magr->data == &mons[PM_CLOCKWORK_SOLDIER] || magr->data == &mons[PM_CLOCKWORK_DWARF] || 
			 magr->data == &mons[PM_FABERGE_SPHERE] || magr->data == &mons[PM_FIREWORK_CART] || 
			 magr->data == &mons[PM_JUGGERNAUT] || magr->data == &mons[PM_ID_JUGGERNAUT]))
	    return ALLOW_M|ALLOW_TM;
	/* Various other combinations such as dog vs cat, cat vs rat, and
	   elf vs orc have been suggested.  For the time being we don't
	   support those. */
	return 0L;
}

boolean
monnear(mon, x, y)
register struct monst *mon;
register int x,y;
/* Is the square close enough for the monster to move or attack into? */
{
	register int distance = dist2(mon->mx, mon->my, x, y);
	if (distance==2 && (mon->data==&mons[PM_GRID_BUG] || mon->data==&mons[PM_BEBELITH])) return 0;
	if(mon->data == &mons[PM_CLOCKWORK_SOLDIER] || mon->data == &mons[PM_CLOCKWORK_DWARF] || 
	   mon->data == &mons[PM_FABERGE_SPHERE]
	) if(mon->mx + xdir[(int)mon->mvar1] != x || 
		   mon->my + ydir[(int)mon->mvar1] != y 
		) return 0;
	if(mon->data == &mons[PM_FIREWORK_CART] || 
	   mon->data == &mons[PM_JUGGERNAUT] || 
	   mon->data == &mons[PM_ID_JUGGERNAUT]
	){
		if(mon->mx + xdir[(int)mon->mvar1] == x && 
		   mon->my + ydir[(int)mon->mvar1] == y 
		) return ((boolean)(distance < 3));
		else if(rn2(2) && mon->mx + xdir[((int)mon->mvar1 + 1)%8] == x && 
		   mon->my + ydir[((int)mon->mvar1 + 1)%8] == y 
		) return (!rn2(2) && (distance < 3));
		else if(mon->mx + xdir[((int)mon->mvar1 - 1)%8] == x && 
		   mon->my + ydir[((int)mon->mvar1 - 1)%8] == y 
		) return (!rn2(2) && (distance < 3));
		else return 0;
	}
	return((boolean)(distance < 3));
}

/* really free dead monsters */
void
dmonsfree()
{
    struct monst **mtmp;
    int count = 0;

    for (mtmp = &fmon; *mtmp;) {
	if ((*mtmp)->mhp <= 0) {
	    struct monst *freetmp = *mtmp;
	    *mtmp = (*mtmp)->nmon;
	    dealloc_monst(freetmp);
	    count++;
	} else
	    mtmp = &(*mtmp)->nmon;
    }

    if (count != iflags.purge_monsters)
	impossible("dmonsfree: %d removed doesn't match %d pending",
		   count, iflags.purge_monsters);
    iflags.purge_monsters = 0;
}

#endif /* OVL1 */
#ifdef OVLB

/* called when monster is moved to larger structure */
void
replmon(mtmp, mtmp2)
register struct monst *mtmp, *mtmp2;
{
    struct obj *otmp;

    /* transfer the monster's inventory */
    for (otmp = mtmp2->minvent; otmp; otmp = otmp->nobj) {
#ifdef DEBUG
	if (otmp->where != OBJ_MINVENT || otmp->ocarry != mtmp)
	    panic("replmon: minvent inconsistency");
#endif
	otmp->ocarry = mtmp2;
    }
    mtmp->minvent = 0;

    /* remove the old monster from the map and from `fmon' list */
    relmon(mtmp);

    /* finish adding its replacement */
#ifdef STEED
    if (mtmp == u.usteed) ; else	/* don't place steed onto the map */
#endif
    place_monster(mtmp2, mtmp2->mx, mtmp2->my);
    if (mtmp2->wormno)	    /* update level.monsters[wseg->wx][wseg->wy] */
	place_wsegs(mtmp2); /* locations to mtmp2 not mtmp. */
    if (emits_light(mtmp2->data)) {
	/* since this is so rare, we don't have any `mon_move_light_source' */
	new_light_source(mtmp2->mx, mtmp2->my,
			 emits_light(mtmp2->data),
			 LS_MONSTER, (genericptr_t)mtmp2);
	/* here we rely on the fact that `mtmp' hasn't actually been deleted */
	del_light_source(LS_MONSTER, (genericptr_t)mtmp, FALSE);
    }
    mtmp2->nmon = fmon;
    fmon = mtmp2;
    if (u.ustuck == mtmp) u.ustuck = mtmp2;
#ifdef STEED
    if (u.usteed == mtmp) u.usteed = mtmp2;
#endif
    if (mtmp2->isshk) replshk(mtmp,mtmp2);

    /* discard the old monster */
    dealloc_monst(mtmp);
}

/* release mon from display and monster list */
void
relmon(mon)
register struct monst *mon;
{
	register struct monst *mtmp;

	if (fmon == (struct monst *)0)  panic ("relmon: no fmon available.");

	removeMonster(mon->mx, mon->my);

	if(mon == fmon) fmon = fmon->nmon;
	else {
		for(mtmp = fmon; mtmp && mtmp->nmon != mon; mtmp = mtmp->nmon) ;
		if(mtmp)    mtmp->nmon = mon->nmon;
		else	    panic("relmon: mon not in list.");
	}
}

/* remove effects of mtmp from other data structures */
STATIC_OVL void
m_detach(mtmp, mptr)
struct monst *mtmp;
struct permonst *mptr;	/* reflects mtmp->data _prior_ to mtmp's death */
{
	if (mtmp->mleashed) m_unleash(mtmp, FALSE);
	    /* to prevent an infinite relobj-flooreffects-hmon-killed loop */
	mtmp->mtrapped = 0;
	mtmp->mhp = 0; /* simplify some tests: force mhp to 0 */
	relobj(mtmp, 0, FALSE);
	remove_monster(mtmp->mx, mtmp->my);
	if (emits_light(mptr))
	    del_light_source(LS_MONSTER, (genericptr_t)mtmp, FALSE);
	newsym(mtmp->mx,mtmp->my);
	unstuck(mtmp);
	fill_pit(mtmp->mx, mtmp->my);

	if(mtmp->isshk) shkgone(mtmp);
	if(mtmp->wormno) wormgone(mtmp);
	iflags.purge_monsters++;
}

/* find the worn amulet of life saving which will save a monster */
struct obj *
mlifesaver(mon)
struct monst *mon;
{
	if (!nonliving_mon(mon)) {
	    struct obj *otmp = which_armor(mon, W_AMUL);

	    if (otmp && otmp->otyp == AMULET_OF_LIFE_SAVING)
		return otmp;
	}
	return (struct obj *)0;
}

STATIC_OVL void
lifesaved_monster(mtmp)
struct monst *mtmp;
{
	struct obj *lifesave = mlifesaver(mtmp);

	if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz) && !(mtmp->mpeaceful) && !rn2(20)){
		if (cansee(mtmp->mx, mtmp->my)) {
			pline("But wait...");
			if (attacktype(mtmp->data, AT_EXPL)
			    || attacktype(mtmp->data, AT_BOOM))
				pline("%s reappears, looking much better!", Monnam(mtmp));
			else
				pline("%s flickers, then reappears looking much better!", Monnam(mtmp));
		}
		mtmp->mcanmove = 1;
		mtmp->mfrozen = 0;
		if (mtmp->mtame && !mtmp->isminion) {
			wary_dog(mtmp, FALSE);
		}
		if (mtmp->mhpmax <= 9) mtmp->mhpmax = 10;
		mtmp->mhp = mtmp->mhpmax;
		return;
	} else if(!rn2(20) && (mtmp->data == &mons[PM_ALABASTER_ELF]
		|| mtmp->data == &mons[PM_ALABASTER_ELF_ELDER]
		|| is_alabaster_mummy(mtmp->data)
	)){
		if (cansee(mtmp->mx, mtmp->my)) {
			pline("%s putrefies with impossible speed!",Monnam(mtmp));
			mtmp->mcanmove = 1;
			mtmp->mfrozen = 0;
			if (mtmp->mtame && !mtmp->isminion) {
				wary_dog(mtmp, FALSE);
			}
			mtmp->mhp = mtmp->mhpmax;
			mtmp->mspec_used = 0;
			if(is_alabaster_mummy(mtmp->data) && mtmp->mvar1 >= SYLLABLE_OF_STRENGTH__AESH && mtmp->mvar1 <= SYLLABLE_OF_SPIRIT__VAUL){
				mksobj_at(mtmp->mvar1, mtmp->mx, mtmp->my, TRUE, FALSE);
				if(mtmp->mvar1 == SYLLABLE_OF_SPIRIT__VAUL)
					mtmp->mintrinsics[(DISPLACED-1)/32] &= ~(1 << (DISPLACED-1)%32);
				mtmp->mvar1 = 0; //Lose the bonus if resurrected
			}
			newcham(mtmp, &mons[rn2(4) ? PM_ACID_BLOB : PM_BLACK_PUDDING], FALSE, FALSE);
			return;
		}
	} else if(mtmp->mspec_used == 0 && mtmp->data == &mons[PM_UVUUDAUM]){
		if (cansee(mtmp->mx, mtmp->my)) {
			pline("But wait...");
			pline("A glowing halo forms over %s!",
				mon_nam(mtmp));
			if (attacktype(mtmp->data, AT_EXPL)
			    || attacktype(mtmp->data, AT_BOOM))
				pline("%s reconstitutes!", Monnam(mtmp));
			else
				pline("%s looks much better!", Monnam(mtmp));
		}
		mtmp->mcanmove = 1;
		mtmp->mfrozen = 0;
		if (mtmp->mtame && !mtmp->isminion) {
			wary_dog(mtmp, FALSE);
		}
		if (mtmp->m_lev < 38) mtmp->m_lev = 38;
		if (mtmp->mhpmax <= 38*4.5) mtmp->mhpmax = (int)(38*4.5);
		mtmp->mhp = mtmp->mhpmax;
		mtmp->mspec_used = mtmp->mhpmax/5;
		return;
	} else if (lifesave) {
		/* not canseemon; amulets are on the head, so you don't want */
		/* to show this for a long worm with only a tail visible. */
		/* Nor do you check invisibility, because glowing and disinte- */
		/* grating amulets are always visible. */
		if (cansee(mtmp->mx, mtmp->my)) {
			pline("But wait...");
			pline("%s medallion begins to glow!",
				s_suffix(Monnam(mtmp)));
			makeknown(AMULET_OF_LIFE_SAVING);
			if (attacktype(mtmp->data, AT_EXPL)
			    || attacktype(mtmp->data, AT_BOOM))
				pline("%s reconstitutes!", Monnam(mtmp));
			else
				pline("%s looks much better!", Monnam(mtmp));
			pline_The("medallion crumbles to dust!");
		}
		m_useup(mtmp, lifesave);
		mtmp->mcanmove = 1;
		mtmp->mfrozen = 0;
		if (mtmp->mtame && !mtmp->isminion) {
			wary_dog(mtmp, FALSE);
		}
		if (mtmp->mhpmax <= 9) mtmp->mhpmax = 10;
		mtmp->mhp = mtmp->mhpmax;
		if (mvitals[monsndx(mtmp->data)].mvflags & G_GENOD && !In_quest(&u.uz)) {
			if (cansee(mtmp->mx, mtmp->my))
			    pline("Unfortunately %s is still genocided...",
				mon_nam(mtmp));
		} else
			return;
		/*Under this point, the only resurrection effects should be those affecting undead, or that the monster wouldn't WANT to trigger*/
	} else if(mtmp->mfaction == FRACTURED && !rn2(2)){
		if (couldsee(mtmp->mx, mtmp->my)) {
			pline("But wait...");
			if(canseemon(mtmp))
				pline("%s fractures further%s, but now looks uninjured!", Monnam(mtmp), !is_silent(mtmp->data) ? " with an unearthly scream" : "");
			else
				You_hear("something crack%s!", !is_silent(mtmp->data) ? " with an unearthly scream" : "");
		}
		mtmp->mcanmove = 1;
		mtmp->mfrozen = 0;
		mtmp->mtame = 0;
		mtmp->mpeaceful = 0;
		mtmp->m_lev += 4;
		mtmp->mhpmax = d(mtmp->m_lev, 8);
		mtmp->mhp = mtmp->mhpmax;
		return;
	} else if(mtmp->zombify && is_kamerel(mtmp->data)){
		if (couldsee(mtmp->mx, mtmp->my)) {
			pline("But wait...");
			if(canseemon(mtmp))
				pline("%s fractures%s, but now looks uninjured!", Monnam(mtmp), !is_silent(mtmp->data) ? " with an unearthly scream" : "");
			else
				You_hear("something crack%s!", !is_silent(mtmp->data) ? " with an unearthly scream" : "");
		}
		mtmp->mfaction = FRACTURED;
		mtmp->mcanmove = 1;
		mtmp->mfrozen = 0;
		mtmp->mtame = 0;
		mtmp->mpeaceful = 0;
		mtmp->m_lev += 4;
		mtmp->mhpmax = d(mtmp->m_lev, 8);
		mtmp->mhp = mtmp->mhpmax;
		return;
	}
	mtmp->mhp = 0;
}

void
mondead(mtmp)
register struct monst *mtmp;
{
	struct permonst *mptr;
	int tmp;

	/* cease occupation if the monster was associated */
	if(mtmp->moccupation) stop_occupation();
	
	if(mtmp->isgd) {
		/* if we're going to abort the death, it *must* be before
		 * the m_detach or there will be relmon problems later */
		if(!grddead(mtmp)) return;
	}
	lifesaved_monster(mtmp);
	if (mtmp->mhp > 0) return;

#ifdef STEED
	/* Player is thrown from his steed when it dies */
	if (mtmp == u.usteed)
		dismount_steed(DISMOUNT_GENERIC);
#endif

	mptr = mtmp->data;		/* save this for m_detach() */
	/* restore chameleon, lycanthropes to true form at death */
	if (mtmp->cham)
		set_mon_data(mtmp, &mons[cham_to_pm[mtmp->cham]], -1);
	else if (mtmp->data == &mons[PM_WEREJACKAL])
		set_mon_data(mtmp, &mons[PM_HUMAN_WEREJACKAL], -1);
	else if (mtmp->data == &mons[PM_WEREWOLF])
		set_mon_data(mtmp, &mons[PM_HUMAN_WEREWOLF], -1);
	else if (mtmp->data == &mons[PM_WERERAT])
		set_mon_data(mtmp, &mons[PM_HUMAN_WERERAT], -1);
	else if (mtmp->data == &mons[PM_ANUBAN_JACKAL])
		set_mon_data(mtmp, &mons[PM_ANUBITE], -1);

	/* if MAXMONNO monsters of a given type have died, and it
	 * can be done, extinguish that monster.
	 *
	 * mvitals[].died does double duty as total number of dead monsters
	 * and as experience factor for the player killing more monsters.
	 * this means that a dragon dying by other means reduces the
	 * experience the player gets for killing a dragon directly; this
	 * is probably not too bad, since the player likely finagled the
	 * first dead dragon via ring of conflict or pets, and extinguishing
	 * based on only player kills probably opens more avenues of abuse
	 * for rings of conflict and such.
	 */
	tmp = monsndx(mtmp->data);
	if (mvitals[tmp].died < 255) mvitals[tmp].died++;
	
	if (tmp == PM_NAZGUL){
			if(mvitals[tmp].born > 0) mvitals[tmp].born--;
			if(mvitals[tmp].mvflags&G_EXTINCT){
				mvitals[tmp].mvflags &= (~G_EXTINCT);
				reset_rndmonst(tmp);
			}
	}
	if(tmp == PM_ARA_KAMEREL && mtmp->mfaction != FRACTURED){
		if(mtmp->mtame)
			u.goldkamcount_tame++;
		else if(mtmp->mpeaceful)
			level.flags.goldkamcount_peace++;
		else
			level.flags.goldkamcount_hostile++;
	}
	/* if it's a (possibly polymorphed) quest leader, mark him as dead */
	if (mtmp->m_id == quest_status.leader_m_id)
		quest_status.leader_is_dead = TRUE;
#ifdef MAIL
	/* if the mail daemon dies, no more mail delivery.  -3. */
	if (tmp == PM_MAIL_DAEMON) mvitals[tmp].mvflags |= G_GENOD;
#endif

	if (mtmp->data->mlet == S_KETER) {
		/* Dead Keter may come back. */
		switch(rnd(5)) {
		case 1:	     /* returns near the stairs */
			(void) makemon(mtmp->data,xdnstair,ydnstair,NO_MM_FLAGS);
			break;
		case 2:	     /* returns near upstair */
			(void) makemon(mtmp->data,xupstair,yupstair,NO_MM_FLAGS);
			break;
		default:
			break;
		}
	}
	if(mtmp->iswiz) wizdead();
	if(mtmp->data->msound == MS_NEMESIS) nemdead();        
	//Asc items and crucial bookkeeping
	if(Race_if(PM_DROW) && !Role_if(PM_NOBLEMAN) && mtmp->data == &mons[urole.neminum] && !flags.made_bell){
		(void) mksobj_at(BELL_OF_OPENING, mtmp->mx, mtmp->my, TRUE, FALSE);
		flags.made_bell = TRUE;
	}
	if(mtmp->data == &mons[PM_OONA]){
		struct obj *obj;
		obj = mksobj_at(SKELETON_KEY, mtmp->mx, mtmp->my, FALSE, FALSE);
		obj = oname(obj, artiname(ART_THIRD_KEY_OF_LAW));
		obj->spe = 0;
		obj->cursed = obj->blessed = FALSE;
	}
	if(mtmp->data == &mons[PM_ASPECT_OF_THE_SILENCE]){
		int remaining = 0;
		if(!flags.made_first)
			remaining++;
		if(!flags.made_divide)
			remaining++;
		if(!flags.made_life)
			remaining++;
		if(remaining){
			remaining = rnd(remaining);
			if(!flags.made_first && !(--remaining))
				mksobj(FIRST_WORD, TRUE, FALSE);
			else if(!flags.made_divide && !(--remaining))
				mksobj(DIVIDING_WORD, TRUE, FALSE);
			else if(!flags.made_life && !(--remaining))
				mksobj(NURTURING_WORD, TRUE, FALSE);
		}
	}
	if(mtmp->data == &mons[PM_GARLAND]){
		int x = mtmp->mx, y = mtmp->my;
		struct obj *otmp;
		makemon(&mons[PM_CHAOS], mtmp->mx, mtmp->my, MM_ADJACENTOK);
	}
	if(mtmp->data == &mons[PM_ILLURIEN_OF_THE_MYRIAD_GLIMPSES] && !(u.uevent.ukilled_illurien)){
		u.uevent.ukilled_illurien = 1;
		u.ill_cnt = rn1(1000, 250);
	}
	if(mtmp->data == &mons[PM_ORCUS]){
		struct engr *oep = engr_at(mtmp->mx,mtmp->my);
		if(!oep){
			make_engr_at(mtmp->mx, mtmp->my,
			 "", 0L, DUST);
			oep = engr_at(mtmp->mx,mtmp->my);
		}
		oep->ward_id = TENEBROUS;
		oep->halu_ward = 0;
		oep->ward_type = BURN;
		oep->complete_wards = 1;
	}
	if(mtmp->data == &mons[PM_CHOKHMAH_SEPHIRAH]){
		u.chokhmah++;
		u.keter++;
	}
	if (mtmp->data == &mons[PM_CHAOS] && mvitals[PM_CHAOS].died == 1) {
	} else if(mtmp->data->geno & G_UNIQ && mvitals[monsndx(mtmp->data)].died == 1){
		char buf[BUFSZ];
		buf[0]='\0';
		if(nonliving(mtmp->data)) Sprintf(buf,"destroyed %s",noit_nohalu_mon_nam(mtmp));
		else Sprintf(buf,"killed %s",noit_nohalu_mon_nam(mtmp));
	}
	//Remove linked hungry dead
	if(mtmp->data == &mons[PM_BLOB_OF_PRESERVED_ORGANS]){
		struct monst *mon, *mtmp2;
		for (mon = fmon; mon; mon = mtmp2){
			mtmp2 = mon->nmon;
			if(mon->data == &mons[PM_HUNGRY_DEAD]){
				if(mtmp->mvar1 == (long)mon->m_id){
					if(mon->mhp > 0){
						mon->mhp = 0;
						mondied(mon);
					}
					break;
				}
			}
		}
		if(!mon){
			for (mon = migrating_mons; mon; mon = mtmp2){
				mtmp2 = mon->nmon;
				if(mon->data == &mons[PM_HUNGRY_DEAD]){
					if(mtmp->mvar1 == (long)mon->m_id){
						mon_arrive(mon, TRUE);
						if(mon->mhp > 0){
							mon->mhp = 0;
							mondied(mon);
						}
						break;
					}
				}
			}
		}
	}
	//Remove linked sword
	if(mtmp->data == &mons[PM_SURYA_DEVA]){
		struct monst *mon, *mtmp2;
		for (mon = fmon; mon; mon = mtmp2){
			mtmp2 = mon->nmon;
			if(mon->data == &mons[PM_DANCING_BLADE] && mon->mvar1 == mtmp->m_id){
				if (DEADMONSTER(mon)) continue;
				mon->mhp = -10;
				monkilled(mon,"",AD_DRLI);
			}
		}
	}
	//Remove linked tentacles
	if(mtmp->data == &mons[PM_WATCHER_IN_THE_WATER] || mtmp->data == &mons[PM_KETO]){
		struct monst *mon, *mtmp2;
		for (mon = fmon; mon; mon = mtmp2){
			mtmp2 = mon->nmon;
			if(mon->data == &mons[PM_SWARM_OF_SNAKING_TENTACLES] || mon->data == &mons[PM_LONG_SINUOUS_TENTACLE] || mon->data == &mons[PM_WIDE_CLUBBED_TENTACLE]){
				if (DEADMONSTER(mon)) continue;
				mon->mhp = -10;
				monkilled(mon,"",AD_DRLI);
			}
		}
	}
	
	//Quest flavor
	if(Role_if(PM_ANACHRONONAUT) && mtmp->mpeaceful && In_quest(&u.uz) && Is_qstart(&u.uz)){
		if(mtmp->data == &mons[PM_TROOPER]){
			verbalize("**ALERT: trooper %d vital signs terminated**", (int)(mtmp->m_id));
		} else if(mtmp->data == &mons[PM_MYRKALFAR_WARRIOR]){
			verbalize("**ALERT: warrior %d vital signs terminated**", (int)(mtmp->m_id));
		} else if(mtmp->data != &mons[PM_PHANTASM]){
			verbalize("**ALERT: citizen %d vital signs terminated**", (int)(mtmp->m_id));
		}
	}
#ifdef RECORD_ACHIEVE
	if(mtmp->data == &mons[PM_LUCIFER]){
		achieve.killed_lucifer = 1;
	}
	else if(mtmp->data == &mons[PM_ASMODEUS]){
		achieve.killed_asmodeus = 1;
	}
	else if(mtmp->data == &mons[PM_DEMOGORGON]){
		achieve.killed_demogorgon = 1;
	}
	else if (mtmp->data == &mons[PM_MEDUSA]
	|| mtmp->data == &mons[PM_GRUE]
	|| mtmp->data == &mons[PM_ECHO]
	|| mtmp->data == &mons[PM_SYNAISTHESIA]
	) {
		achieve.killed_challenge = 1;
	}
#endif
	if(glyph_is_invisible(levl[mtmp->mx][mtmp->my].glyph))
		unmap_object(mtmp->mx, mtmp->my);
	m_detach(mtmp, mptr);
}

/* TRUE if corpse might be dropped, magr may die if mon was swallowed */
boolean
corpse_chance(mon, magr, was_swallowed)
struct monst *mon;
struct monst *magr;			/* killer, if swallowed */
boolean was_swallowed;			/* digestion */
{
	struct permonst *mdat = mon->data;
	int i, tmp;
	if (mdat == &mons[PM_VLAD_THE_IMPALER]) {
		if(mvitals[PM_VLAD_THE_IMPALER].died == 1) livelog_write_string("destroyed Vlad the Impaler");
	    if (cansee(mon->mx, mon->my) && !was_swallowed)
		pline("%s body crumbles into dust.", s_suffix(Monnam(mon)));
	    return FALSE;
	}
	else if(mdat->mlet == S_VAMPIRE && mdat->geno & G_UNIQ){
		//Don't livelog Vlad's wives; livelog spam reduction
		pline("%s body crumbles into dust.", s_suffix(Monnam(mon)));
	    return FALSE;
	}
	else if (mdat->mlet == S_LICH && mdat != &mons[PM_LICH__THE_FIEND_OF_EARTH]) {
	    if (cansee(mon->mx, mon->my) && !was_swallowed)
			pline("%s body crumbles into dust.", s_suffix(Monnam(mon)));
	    if(mdat != &mons[PM_VECNA] && mdat != &mons[PM_LICH__THE_FIEND_OF_EARTH]) return FALSE; /*Vecna leaves his hand or eye*/
	}
	else if(mdat == &mons[PM_ECLAVDRA]) return FALSE;
	else if(mdat == &mons[PM_CHOKHMAH_SEPHIRAH]){
		if(mvitals[PM_CHOKHMAH_SEPHIRAH].died == 1) livelog_write_string("destroyed a chokhmah sephirah");
		return FALSE;
	}
	else if (mdat == &mons[PM_CHAOS] && mvitals[PM_CHAOS].died == 1) {
		if(Hallucination) livelog_write_string("perpetuated an asinine paradigm");
		else livelog_write_string("destroyed Chaos");
	} else if(mdat->geno & G_UNIQ && mvitals[monsndx(mdat)].died == 1){
		char buf[BUFSZ];
		buf[0]='\0';
		if(nonliving(mdat)) Sprintf(buf,"destroyed %s",noit_nohalu_mon_nam(mon));
		else Sprintf(buf,"killed %s",noit_nohalu_mon_nam(mon));
		livelog_write_string(buf);
	}
	//Must be done here for reasons that are obscure
	if(Role_if(PM_ANACHRONONAUT) && mon->mpeaceful && In_quest(&u.uz) && Is_qstart(&u.uz)){
		if(mdat == &mons[PM_TROOPER]){
			if(!cansee(mon->mx,mon->my)) map_invisible(mon->mx, mon->my);
		} else if(mdat == &mons[PM_MYRKALFAR_WARRIOR]){
			if(!cansee(mon->mx,mon->my)) map_invisible(mon->mx, mon->my);
		} else if(mdat != &mons[PM_PHANTASM]){
			if(!cansee(mon->mx,mon->my)) map_invisible(mon->mx, mon->my);
		}
	}
	if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && uwep->ovar1&SEAL_MALPHAS && rn2(20) <= (mvitals[PM_ACERERAK].died > 0 ? 4 : 1)){
		struct monst *mtmp;
		mtmp = makemon(&mons[PM_CROW], u.ux, u.uy, MM_EDOG|MM_ADJACENTOK);
		initedog(mtmp);
		mtmp->m_lev += uwep->spe;
		mtmp->mhpmax = (mtmp->m_lev * 8) - 4;
		mtmp->mhp =  mtmp->mhpmax;
	}
	
	/* Gas spores always explode upon death */
	for(i = 0; i < NATTK; i++) {
		if(mdat->mattk[i].aatyp == AT_NONE &&  mdat->mattk[i].adtyp == AD_OONA){
			mdat->mattk[i].aatyp = AT_BOOM;
			mdat->mattk[i].adtyp = u.oonaenergy;
		}
		if (mdat->mattk[i].aatyp == AT_BOOM  &&  mdat->mattk[i].adtyp != AD_HLBD && mdat->mattk[i].adtyp != AD_POSN) {
			if (mdat->mattk[i].damn)
				tmp = d((int)mdat->mattk[i].damn,
						(int)mdat->mattk[i].damd);
	    	else if(mdat->mattk[i].damd)
	    	    tmp = d((int)mdat->mlevel+1, (int)mdat->mattk[i].damd);
	    	else tmp = 0;
			if (was_swallowed && magr) {
				if (magr == &youmonst) {
					There("is an explosion in your %s!",
						  body_part(STOMACH));
					Sprintf(killer_buf, "%s explosion",
						s_suffix(mdat->mname));
					if (Half_physical_damage) tmp = (tmp+1) / 2;
					if(u.uvaul_duration) tmp = (tmp + 1) / 2;
					losehp(tmp, killer_buf, KILLED_BY_AN);
				} 
				else {
					if (flags.soundok) You_hear("an explosion.");
					magr->mhp -= tmp;
					if (magr->mhp < 1) mondied(magr);
					if (magr->mhp < 1) { /* maybe lifesaved */
						if (canspotmon(magr))
						pline("%s rips open!", Monnam(magr));
					} 
					else if (canseemon(magr)) pline("%s seems to have indigestion.", Monnam(magr));
				}
			
				return FALSE;
			}
			
	    	Sprintf(killer_buf, "%s explosion", s_suffix(mdat->mname));
	    	killer = killer_buf;
	    	killer_format = KILLED_BY_AN;
			if(mdat==&mons[PM_GAS_SPORE] || mdat==&mons[PM_DUNGEON_FERN_SPORE]){
	    	  explode(mon->mx, mon->my, AD_ACID, MON_EXPLODE, tmp, EXPL_NOXIOUS, 1);
			}
			else if(mdat==&mons[PM_SWAMP_FERN_SPORE]){
	    	  explode(mon->mx, mon->my, AD_DISE, MON_EXPLODE, tmp, EXPL_MAGICAL, 1);
			}
			else if(mdat==&mons[PM_BURNING_FERN_SPORE]){
	    	  explode(mon->mx, mon->my, AD_PHYS, MON_EXPLODE, tmp, EXPL_FIERY, 1);
			}
			else if(mdat->mattk[i].adtyp == AD_PHYS){
				if(mdat == &mons[PM_FABERGE_SPHERE]) explode(mon->mx, mon->my, AD_PHYS, MON_EXPLODE, tmp, rn2(7), 1);
				else explode(mon->mx, mon->my, AD_PHYS, MON_EXPLODE, tmp, EXPL_MUDDY, 1);
			} else if(mdat->mattk[i].adtyp == AD_FIRE){
				//mdat == &mons[PM_BALROG] || mdat == &mons[PM_MEPHISTOPHELES] || mdat == &mons[PM_FLAMING_SPHERE]){
				explode(mon->mx, mon->my, AD_FIRE, MON_EXPLODE, tmp, EXPL_FIERY, 1);
			}
			else if(mdat->mattk[i].adtyp == AD_JAILER){
				explode(mon->mx, mon->my, AD_FIRE, MON_EXPLODE, tmp, EXPL_FIERY, 1);
				u.uevent.ukilled_apollyon = 1;
			}
			else if(mdat->mattk[i].adtyp == AD_GARO){
				if(couldsee(mon->mx, mon->my)){
					pline("\"R-regrettable... Although my rival, you were spectacular.");
					pline("I shall take my bow by opening my heart and revealing my wisdom...");
					outrumor(rn2(2), BY_OTHER); //either true (3/4) or false (1/4), no mechanism specified.
					pline("Belief or disbelief rests with you.");
					pline("To die without leaving a corpse....\"");
					explode(mon->mx, mon->my, AD_PHYS, MON_EXPLODE, tmp, EXPL_MUDDY, 1);
					pline("\"That is the way of us Garo.\"");
				} else {
					explode(mon->mx, mon->my, AD_PHYS, MON_EXPLODE, tmp, EXPL_MUDDY, 1);
				}
			}
			else if(mdat->mattk[i].adtyp == AD_GARO_MASTER){
				if(couldsee(mon->mx, mon->my)){
					pline("\"To think thou couldst defeat me...");
					pline("Though my rival, thou were't spectacular.");
					pline("I shall take my bow by opening my heart and revealing my wisdom...");
					outgmaster(); //Gives out a major consultation. Does not set the consultation flags.
					pline("Do not forget these words...");
					pline("Die I shall, leaving no corpse.\"");
					explode(mon->mx, mon->my, AD_PHYS, MON_EXPLODE, tmp, EXPL_MUDDY, 1);
					pline("\"That is the law of us Garo.\"");
				} else {
					explode(mon->mx, mon->my, AD_PHYS, MON_EXPLODE, tmp, EXPL_MUDDY, 1);
				}
			}
			else if(mdat->mattk[i].adtyp == AD_COLD){
			//mdat == &mons[PM_BAALPHEGOR] || mdat == &mons[PM_ANCIENT_OF_ICE] || mdat == &mons[PM_FREEZING_SPHERE]){
			  explode(mon->mx, mon->my, AD_COLD, MON_EXPLODE, tmp, EXPL_FROSTY, 1);
			}
			else if(mdat->mattk[i].adtyp == AD_ELEC){//mdat == &mons[PM_SHOCKING_SPHERE]){
				explode(mon->mx, mon->my, AD_ELEC, MON_EXPLODE, tmp, EXPL_MAGICAL, 1);
			}
			else if(mdat->mattk[i].adtyp == AD_FRWK){
				int x, y, i;
				for(i = rn2(3)+2; i > 0; i--){
					x = rn2(7)-3;
					y = rn2(7)-3;
					explode(mon->mx+x, mon->my+y, AD_PHYS, -1, tmp, rn2(7), 1);
				}
				tmp=0;
			} else if(mdat->mattk[i].adtyp == AD_SPNL){
				explode(mon->mx, mon->my, AD_COLD, MON_EXPLODE, tmp, EXPL_WET, 1);
				makemon(rn2(2) ? &mons[PM_LEVIATHAN] : &mons[PM_LEVISTUS], mon->mx, mon->my, MM_ADJACENTOK);
			} else if(mdat == &mons[PM_ANCIENT_OF_DEATH]){
				if(!(u.sealsActive&SEAL_OSE)) explode(mon->mx, mon->my, AD_MAGM, MON_EXPLODE, tmp, EXPL_DARK, 1);
			} else if(mdat->mattk[i].adtyp == AD_MAND){
				struct monst *mtmp, *mtmp2;
				if(mon->mcan){
					char buf[BUFSZ];
					Sprintf(buf, "%s croaks out a horse shriek.", Monnam(mon)); //Monnam and mon_nam share a buffer and can't be used on the same line.
					pline("%s  It seems %s has a sore throat!", buf, mon_nam(mon));
					return FALSE;
				}
				else pline("%s lets out a terrible shriek!", Monnam(mon));
				for (mtmp = fmon; mtmp; mtmp = mtmp2){
					mtmp2 = mtmp->nmon;
					if(mtmp->data->geno & G_GENO && !nonliving_mon(mon) && !is_demon(mon->data) && !is_keter(mon->data) && mtmp->mhp <= 100){
						if (DEADMONSTER(mtmp)) continue;
						mtmp->mhp = -10;
						monkilled(mtmp,"",AD_DRLI);
					}
				}
				/* And a finger of death type attack on you */
				if (nonliving(youracedata) || is_demon(youracedata)) {
					You("seem no deader than before.");
				} else if ((Upolyd ? u.mh : u.uhp) <= 100 && !(u.sealsActive&SEAL_OSE)) {
					if (Hallucination) {
					You("have an out of body experience.");
					} else {
					killer_format = KILLED_BY_AN;
					killer = "mandrake's dying shriek";
					done(DIED);
					}
				} else shieldeff(u.ux,u.uy);
			}
			else{
			  explode(mon->mx, mon->my, AD_MAGM, MON_EXPLODE, tmp, EXPL_MAGICAL, 1);
			}
	    	if(mdat == &mons[PM_GARO_MASTER] || mdat == &mons[PM_GARO]) return (TRUE);
			else return (FALSE);
	    } //End AT_BOOM != AD_HLBD && != AD_POSN
		else if(mdat->mattk[i].adtyp == AD_HLBD && mdat == &mons[PM_ASMODEUS]){
			int i;
			for(i=0; i<2; i++) makemon(&mons[PM_NESSIAN_PIT_FIEND], mon->mx, mon->my, MM_ADJACENTOK);
			for(i=0; i<7; i++) makemon(&mons[PM_PIT_FIEND], mon->mx, mon->my, MM_ADJACENTOK);
			for(i = 0; i<12; i++) makemon(&mons[PM_BARBED_DEVIL], mon->mx, mon->my, MM_ADJACENTOK);
			for(i = 0; i<20; i++) makemon(&mons[PM_HORNED_DEVIL], mon->mx, mon->my, MM_ADJACENTOK);
			for(i = 0; i<30; i++) makemon(&mons[PM_LEMURE], mon->mx, mon->my, MM_ADJACENTOK);
	    	return (FALSE);
		}
		else if(mdat->mattk[i].adtyp == AD_HLBD && mdat == &mons[PM_VERIER]){
			int i;
			for(i=0; i<9; i++) makemon(&mons[PM_PIT_FIEND], mon->mx, mon->my, MM_ADJACENTOK);
			for(i = 0; i<12; i++) makemon(&mons[PM_BARBED_DEVIL], mon->mx, mon->my, MM_ADJACENTOK);
			for(i = 0; i<18; i++) makemon(&mons[PM_HORNED_DEVIL], mon->mx, mon->my, MM_ADJACENTOK);
			for(i = 0; i<30; i++) makemon(&mons[PM_LEMURE], mon->mx, mon->my, MM_ADJACENTOK);
	    	return (FALSE);
		}
  		else if(	( (mdat->mattk[i].aatyp == AT_NONE && mdat==&mons[PM_GREAT_CTHULHU])
					 || mdat->mattk[i].aatyp == AT_BOOM) 
				&& mdat->mattk[i].adtyp == AD_POSN){
	    	Sprintf(killer_buf, "%s explosion", s_suffix(mdat->mname));
	    	killer = killer_buf;
	    	killer_format = KILLED_BY_AN;
			explode(mon->mx, mon->my, AD_PHYS, MON_EXPLODE, d(8,8), EXPL_NOXIOUS, 1);
			if(mdat==&mons[PM_GREAT_CTHULHU]){
				flags.cth_attk=TRUE;//state machine stuff.
				create_gas_cloud(mon->mx, mon->my, 2, 30);
				flags.cth_attk=FALSE;
			}
		}
		else if(mdat->mattk[i].adtyp == AD_GROW && (mdat==&mons[PM_AXUS])){
			struct monst *mtmp;
			struct permonst *mdat1;
			int quin = 1, qua = 2, tre = 3, duo = 4;
			int mndx = 0;
//			int quincount = 0;
			for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
				mdat1 = mtmp->data;
//				if(mdat1==&mons[PM_QUINON]) quincount++;
				if(mdat1==&mons[PM_QUATON] && quin){
					set_mon_data(mtmp, &mons[PM_QUINON], 0);
					mtmp->m_lev += 1;
					mtmp->mhp += 4;
					mtmp->mhpmax += 4;
					newsym(mtmp->mx, mtmp->my);
					quin--;
//					quincount++;
				}
				else if(mdat1==&mons[PM_TRITON] && qua){
					set_mon_data(mtmp, &mons[PM_QUATON], 0);
					mtmp->m_lev += 1;
					mtmp->mhp += 4;
					mtmp->mhpmax += 4;
					newsym(mtmp->mx, mtmp->my);
					qua--;
				}
				else if(mdat1==&mons[PM_DUTON] && tre){
					set_mon_data(mtmp, &mons[PM_TRITON], 0);
					mtmp->m_lev += 1;
					mtmp->mhp += 4;
					mtmp->mhpmax += 4;
					newsym(mtmp->mx, mtmp->my);
					tre--;
				}
				else if(mdat1==&mons[PM_MONOTON] && duo){
					set_mon_data(mtmp, &mons[PM_DUTON], 0);
					mtmp->m_lev += 1;
					mtmp->mhp += 4;
					mtmp->mhpmax += 4;
					newsym(mtmp->mx, mtmp->my);
					duo--;
//					makemon(&mons[PM_MONOTON], mon->mx, mon->my,MM_ADJACENTOK|MM_ANGRY);
				}
			}
			u.uevent.uaxus_foe = 1;//enemy of the modrons
		  for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
			mndx = monsndx(mtmp->data);
			if(mndx <= PM_QUINON && mndx >= PM_MONOTON && mtmp->mpeaceful){
				if(canseemon(mtmp)) pline("%s gets angry...", mon_nam(mtmp));
				mtmp->mpeaceful = 0;
			}
		   }
//			The dungeon of ill regard, where Axus is found, now spawns only Modrons.  So this is uneeded
//			for(quincount;quincount<7;quincount++) makemon(&mons[PM_QUINON], mon->mx, mon->my,MM_ADJACENTOK|MM_ANGRY);
		}
		else if(mdat->mattk[i].adtyp == AD_GROW || (mdat==&mons[PM_QUINON] 
		        && mdat->mattk[i].adtyp == AD_STUN)){//horrid quinon kludge
			struct monst *mtmp;
			struct permonst *mdat1, **child, **growto;
			int i = 0;
			int chain = FALSE;
			int found = FALSE;
			int tontype = FALSE;
			if(mdat==&mons[PM_QUINON]){
				chain = TRUE;
				tontype = TRUE;
				child = (struct permonst **) malloc(sizeof(struct permonst)*5);
				growto = (struct permonst **) malloc(sizeof(struct permonst)*5);
				child[0]=&mons[PM_QUATON];
				growto[0]=&mons[PM_QUINON];
				child[1]=&mons[PM_TRITON];
				growto[1]=&mons[PM_QUATON];
				child[2]=&mons[PM_DUTON];
				growto[2]=&mons[PM_TRITON];
				child[3]=&mons[PM_MONOTON];
				growto[3]=&mons[PM_DUTON];
				child[4]=0;
				growto[4]=0;
			}
			else if(mdat==&mons[PM_QUATON]){
				chain = TRUE;
				tontype = TRUE;
				child = (struct permonst **) malloc(sizeof(struct permonst)*4);
				growto = (struct permonst **) malloc(sizeof(struct permonst)*4);
				child[0]=&mons[PM_TRITON];
				growto[0]=&mons[PM_QUATON];
				child[1]=&mons[PM_DUTON];
				growto[1]=&mons[PM_TRITON];
				child[2]=&mons[PM_MONOTON];
				growto[2]=&mons[PM_DUTON];
				child[3]=0;
				growto[3]=0;
			}
			else if(mdat==&mons[PM_TRITON]){
				chain = TRUE;
				tontype = TRUE;
				child = (struct permonst **) malloc(sizeof(struct permonst)*3);
				growto = (struct permonst **) malloc(sizeof(struct permonst)*3);
				child[0]=&mons[PM_DUTON];
				growto[0]=&mons[PM_TRITON];
				child[1]=&mons[PM_MONOTON];
				growto[1]=&mons[PM_DUTON];
				child[2]=0;
				growto[2]=0;
			}
			else if(mdat==&mons[PM_DUTON]){
				chain = TRUE;
				tontype = TRUE;
				child = (struct permonst **) malloc(sizeof(struct permonst)*2);
				growto = (struct permonst **) malloc(sizeof(struct permonst)*2);
				child[0]=&mons[PM_MONOTON];
				growto[0]=&mons[PM_DUTON];
				child[1]=0;
				growto[1]=0;
			}
			else if(mdat==&mons[PM_MONOTON]){
				child = (struct permonst **) malloc(sizeof(struct permonst)*1);
				growto = (struct permonst **) malloc(sizeof(struct permonst)*1);
				child[0]=0;
				growto[0]=0;
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
					mdat1 = mtmp->data;
					if(mdat1==&mons[PM_AXUS]){
						mtmp = makemon(&mons[PM_MONOTON], mtmp->mx, mtmp->my,MM_ADJACENTOK|MM_ANGRY|MM_NOCOUNTBIRTH);
						if(mtmp) mtmp->mclone = 1;
						break; //break special for loop
					}
				}
			}
			for (i=0; child[i] && growto[i]; i++){
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
					found = FALSE; //haven't found the child yet.
//				    mtmp2 = mtmp->nmon;
					mdat1 = mtmp->data;
					if(!DEADMONSTER(mtmp) && mdat1==child[i]){
						set_mon_data(mtmp, growto[i], 0);
						//Assumes Auton
						mtmp->m_lev += 1;
						mtmp->mhp += 4;
						mtmp->mhpmax += 4;
						
						newsym(mtmp->mx, mtmp->my);
						found=TRUE;
						if(child[i] == &mons[PM_MONOTON]){
							for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
								mdat1 = mtmp->data;
								if(mdat1==&mons[PM_AXUS]){
									mtmp = makemon(child[i], mtmp->mx, mtmp->my,MM_ADJACENTOK|MM_ANGRY);
									if(mtmp) mtmp->mclone = 1;
									break; //break special for loop
								}
							}
						}
						break;//exit inner for loop
					}
				}
				if(!found && tontype){
					for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
						mdat1 = mtmp->data;
						if(mdat1==&mons[PM_AXUS]){
							chain = FALSE;
							mtmp = makemon(growto[i], mtmp->mx, mtmp->my,MM_ADJACENTOK|MM_ANGRY);
							if(mtmp) mtmp->mclone = 1;
//							makemon(&mons[PM_MONOTON], mtmp->mx, mtmp->my,MM_ADJACENTOK|MM_ANGRY);
							break; //break special for loop
						}
					}
				}
				if(!found && chain) break;//exit outer loop if child[i] not found
											//AND the growth is chained.
			}
			free(growto);
			free(child);
		}//end AD_GROW basic
		else if(mdat->mattk[i].adtyp == AD_SOUL){
			struct monst *mtmp;
			struct permonst *mdat1;
			int hpgain = 0;
			int lvlgain = 0;
			int lvls = 0;
			if(mdat==&mons[PM_DEEP_ONE]){
				hpgain = 2;
			} else if(mdat==&mons[PM_DEEPER_ONE]){
				hpgain = 4;
			} else if(mdat==&mons[PM_DEEPEST_ONE]){
				hpgain = 8;
			} else { //arcadian avenger
				lvlgain = 1;
			}
			if(mdat==&mons[PM_DEEP_ONE] || mdat==&mons[PM_DEEPER_ONE] || mdat==&mons[PM_DEEPEST_ONE]){
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
					mdat1 = mtmp->data;
					if( mdat1==&mons[PM_DEEP_ONE] || 
						mdat1==&mons[PM_DEEPER_ONE] || 
						mdat1==&mons[PM_DEEPEST_ONE] ){
							if(mtmp->mhp > 0){
								if(lvlgain) for(lvls = lvlgain; lvls > 0; lvls--) grow_up(mtmp, 0);
								if(hpgain){
									if (mtmp->mhpmax < 300){
										mtmp->mhpmax += hpgain-1;
										mtmp->mhp += hpgain-1;
										grow_up(mtmp, mtmp); //gain last HP and grow up if needed
									}
									else {
										mtmp->mhpmax += 1;
										mtmp->mhp += 1;
										grow_up(mtmp, mtmp); //gain last HP and grow up if needed
									}
								}
							}
					}
				}
			} else { //arcadian avenger
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
					mdat1 = mtmp->data;
					if( mdat1==mdat ){
						if(mtmp->mhp > 0){ 
							if(lvlgain) for(lvls = lvlgain; lvls > 0; lvls--) grow_up(mtmp, 0);
							if(hpgain){
								mtmp->mhpmax += hpgain-1;
								mtmp->mhp += hpgain-1;
								grow_up(mtmp, mtmp); //gain last HP and grow up if needed
							}
						}
					}
				}
			}
		}//end of AD_SOUL
	} //end of for loop that views attacks.
	/* must duplicate this below check in xkilled() since it results in
	 * creating no objects as well as no corpse
	 */
	if (mon->mfaction == SKELIFIED)
		return FALSE;

	if (mon->mfaction == CRYSTALFIED)
		return TRUE;

	if (is_golem(mdat)
		   || is_mplayer(mdat)
		   || is_rider(mdat)
		   || mon->m_id == quest_status.leader_m_id
		   || mon->data->msound == MS_NEMESIS
		   || is_alabaster_mummy(mon->data)
		   || (uwep && uwep->oartifact == ART_SINGING_SWORD && uwep->osinging == OSING_LIFE && mon->mtame)
		   || mdat == &mons[PM_UNDEAD_KNIGHT]
		   || mdat == &mons[PM_WARRIOR_OF_SUNLIGHT]
		   || mdat == &mons[PM_CROW_WINGED_HALF_DRAGON]
		   || mdat == &mons[PM_SEYLL_AUZKOVYN]
		   || mdat == &mons[PM_DARUTH_XAXOX]
		   || mdat == &mons[PM_ORION]
		   || mdat == &mons[PM_VECNA]
//		   || mdat == &mons[PM_UNICORN_OF_AMBER]
		   || mdat == &mons[PM_NIGHTMARE]
		   || mdat == &mons[PM_CHROMATIC_DRAGON]
		   || mdat == &mons[PM_PLATINUM_DRAGON]
		   || mdat == &mons[PM_CHANGED]
		   || mdat == &mons[PM_WARRIOR_CHANGED]
		   || mdat == &mons[PM_TWITCHING_FOUR_ARMED_CHANGED]
		   || mdat == &mons[PM_CLAIRVOYANT_CHANGED]
//		   || mdat == &mons[PM_PINK_UNICORN]
		   )
		return TRUE;
		
	if (LEVEL_SPECIFIC_NOCORPSE(mdat))
		return FALSE;

	if(In_hell(&u.uz) || In_endgame(&u.uz)) //u.uevent.invoked || 
		return FALSE;
	
	if(bigmonst(mdat) || mdat == &mons[PM_LIZARD]) return TRUE;
	
	tmp = (int)(2 + ((int)(mdat->geno & G_FREQ)<2) + verysmall(mdat));
	return (u.sealsActive&SEAL_EVE) ? (!rn2(tmp)||!rn2(tmp)) : 
		  (uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && uwep->ovar1&SEAL_EVE) ?  (!rn2(tmp)||!rn2(2*tmp - 1)) :
		  !rn2(tmp);
}


void
spore_dies(mon)
struct monst *mon;
{
	if (mon->mhp <= 0) {
	    int sporetype;
	    coord mm; schar ltyp;
	    mm.x = mon->mx; mm.y = mon->my;
	    ltyp = levl[mm.x][mm.y].typ;
	    create_gas_cloud(mm.x, mm.y, rn1(2,1), rnd(8));
	    /* all fern spores have a 2/3 chance of creating nothing, except for
	       the generic fern spore, which guarantees a terrain-appropriate fern */
	    if (mon->data == &mons[PM_DUNGEON_FERN_SPORE]) {
		/* dungeon ferns cannot reproduce on ice, lava, or water; swamp is okay */
			if (!is_ice(mm.x, mm.y) && !is_lava(mm.x, mm.y) && !is_pool(mm.x, mm.y, FALSE))
				sporetype = 0;
			else return;
			if (rn2(3)) return;
	    } else if (mon->data == &mons[PM_SWAMP_FERN_SPORE]) {
			if (!is_ice(mm.x, mm.y) && !is_lava(mm.x, mm.y))
				sporetype = 2;
			else return;
			if (rn2(3)) return;
	    } else if (mon->data == &mons[PM_BURNING_FERN_SPORE]) {
			if (!is_ice(mm.x, mm.y) && !is_pool(mm.x, mm.y, TRUE))
				sporetype = 3;
			else return;
			if (rn2(3)) return;
		}
	    /* when creating a new fern, 5/6 chance of creating
	       a fern sprout and 1/6 chance of a fully-grown one */
	    switch (sporetype) {
		case 0:
		    if (!rn2(6)) makemon(&mons[PM_DUNGEON_FERN], mm.x, mm.y, NO_MM_FLAGS);
		    else makemon(&mons[PM_DUNGEON_FERN_SPROUT], mm.x, mm.y, NO_MM_FLAGS);
	    break;
		case 2:
		    if (!rn2(6)) makemon(&mons[PM_SWAMP_FERN], mm.x, mm.y, NO_MM_FLAGS);
		    else makemon(&mons[PM_SWAMP_FERN_SPROUT], mm.x, mm.y, NO_MM_FLAGS);
	    break;
		case 3:
		    if (!rn2(6)) makemon(&mons[PM_BURNING_FERN], mm.x, mm.y, NO_MM_FLAGS);
		    else makemon(&mons[PM_BURNING_FERN_SPROUT], mm.x, mm.y, NO_MM_FLAGS);
	    break;
		default:
		    pline("BUG: Unknown spore type: (%d)", sporetype);
	    break;
	    }

	}
}

/* drop (perhaps) a cadaver and remove monster */
void
mondied(mdef)
register struct monst *mdef;
{
	mondead(mdef);
	if (mdef->mhp > 0) return;	/* lifesaved */

	if (corpse_chance(mdef, (struct monst *)0, FALSE) &&
	    (accessible(mdef->mx, mdef->my) || is_pool(mdef->mx, mdef->my, FALSE)))
		(void) make_corpse(mdef);
}

/* monster disappears, not dies */
void
mongone(mdef)
register struct monst *mdef;
{
	mdef->mhp = 0;	/* can skip some inventory bookkeeping */
#ifdef STEED
	/* Player is thrown from his steed when it disappears */
	if (mdef == u.usteed)
		dismount_steed(DISMOUNT_GENERIC);
#endif
	/* cease occupation if the monster was associated */
	if(mdef->moccupation) stop_occupation();
	
	
	/* drop special items like the Amulet so that a dismissed Keter or nurse
	   can't remove them from the game */
	mdrop_special_objs(mdef);
	/* release rest of monster's inventory--it is removed from game */
	discard_minvent(mdef);
#ifndef GOLDOBJ
	mdef->mgold = 0L;
#endif
	m_detach(mdef, mdef->data);
}

/* monster vanishes, not dies, leaving inventory */
void
monvanished(mdef)
register struct monst *mdef;
{
	mdef->mhp = 0;	/* can skip some inventory bookkeeping */
#ifdef STEED
	/* Player is thrown from his steed when it disappears */
	if (mdef == u.usteed)
		dismount_steed(DISMOUNT_GENERIC);
#endif

	/* cease occupation if the monster was associated */
	if(mdef->moccupation) stop_occupation();
	
	/* drop special items like the Amulet so that a dismissed Keter or nurse
	   can't remove them from the game */
	mdrop_special_objs(mdef);
	/* release rest of monster's inventory--it is removed from game */
	// discard_minvent(mdef);
// #ifndef GOLDOBJ
	// mdef->mgold = 0L;
// #endif
	m_detach(mdef, mdef->data);
}

/* drop a statue or rock and remove monster */
void
monstone(mdef)
register struct monst *mdef;
{
	struct obj *otmp, *obj, *oldminvent;
	xchar x = mdef->mx, y = mdef->my;
	boolean wasinside = FALSE;

	/* we have to make the statue before calling mondead, to be able to
	 * put inventory in it, and we have to check for lifesaving before
	 * making the statue....
	 */
	lifesaved_monster(mdef);
	if (mdef->mhp > 0) return;

	mdef->mtrapped = 0;	/* (see m_detach) */

	if ((int)mdef->data->msize > MZ_TINY ||
		    !rn2(2 + ((int) (mdef->data->geno & G_FREQ) > 2))) {
		oldminvent = 0;
		/* some objects may end up outside the statue */
		while ((obj = mdef->minvent) != 0) {
		    obj_extract_self(obj);
		    if (obj->owornmask)
			update_mon_intrinsics(mdef, obj, FALSE, TRUE);
		    obj_no_longer_held(obj);
		    if (obj->owornmask & W_WEP){
				setmnotwielded(mdef,obj);
				MON_NOWEP(mdef);
			}
		    if (obj->owornmask & W_SWAPWEP){
				setmnotwielded(mdef,obj);
				MON_NOSWEP(mdef);
			}
		    obj->owornmask = 0L;
		    if (is_boulder(obj) ||
#if 0				/* monsters don't carry statues */
     (obj->otyp == STATUE && mons[obj->corpsenm].msize >= mdef->data->msize) ||
#endif
				obj_resists(obj, 0, 0)) {
			if (flooreffects(obj, x, y, "fall")) continue;
			place_object(obj, x, y);
		    } else {
			if (obj->lamplit) end_burn(obj, TRUE);
			obj->nobj = oldminvent;
			oldminvent = obj;
		    }
		}
		/* defer statue creation until after inventory removal
		   so that saved monster traits won't retain any stale
		   item-conferred attributes */
		otmp = mkcorpstat(STATUE, KEEPTRAITS(mdef) ? mdef : 0,
				  mdef->data, x, y, FALSE);
		if (mdef->mnamelth) otmp = oname(otmp, NAME(mdef));
		while ((obj = oldminvent) != 0) {
		    oldminvent = obj->nobj;
		    (void) add_to_container(otmp, obj);
		}
#ifndef GOLDOBJ
		if (mdef->mgold) {
			struct obj *au;
			au = mksobj(GOLD_PIECE, FALSE, FALSE);
			au->quan = mdef->mgold;
			au->owt = weight(au);
			(void) add_to_container(otmp, au);
			mdef->mgold = 0;
		}
#endif
		/* Archeologists should not break unique statues */
		if (mdef->data->geno & G_UNIQ)
			otmp->spe = 1;
		otmp->owt = weight(otmp);
	} else
		otmp = mksobj_at(ROCK, x, y, TRUE, FALSE);

	stackobj(otmp);
	/* mondead() already does this, but we must do it before the newsym */
	if(glyph_is_invisible(levl[x][y].glyph))
	    unmap_object(x, y);
	if (cansee(x, y)) newsym(x,y);
	/* We don't currently trap the hero in the statue in this case but we could */
	if (u.uswallow && u.ustuck == mdef) wasinside = TRUE;
	mondead(mdef);
	if (wasinside) {
		if (is_animal(mdef->data))
			You("%s through an opening in the new %s.",
				locomotion(&youmonst, "jump"),
				xname(otmp));
	}
}

/* drop a gold statue or rock and remove monster */
void
mongolded(mdef)
register struct monst *mdef;
{
	struct obj *otmp, *obj, *oldminvent;
	xchar x = mdef->mx, y = mdef->my;
	boolean wasinside = FALSE;

	/* we have to make the statue before calling mondead, to be able to
	 * put inventory in it, and we have to check for lifesaving before
	 * making the statue....
	 */
	lifesaved_monster(mdef);
	if (mdef->mhp > 0) return;

	mdef->mtrapped = 0;	/* (see m_detach) */

	if ((int)mdef->data->msize > MZ_TINY ||
		    !rn2(2 + ((int) (mdef->data->geno & G_FREQ) > 2))) {
		oldminvent = 0;
		/* some objects may end up outside the statue */
		while ((obj = mdef->minvent) != 0) {
		    obj_extract_self(obj);
		    if (obj->owornmask)
			update_mon_intrinsics(mdef, obj, FALSE, TRUE);
		    obj_no_longer_held(obj);
		    if (obj->owornmask & W_WEP){
				setmnotwielded(mdef,obj);
				MON_NOWEP(mdef);
			}
		    if (obj->owornmask & W_SWAPWEP){
				setmnotwielded(mdef,obj);
				MON_NOSWEP(mdef);
			}
		    obj->owornmask = 0L;
		    if (is_boulder(obj) ||
#if 0				/* monsters don't carry statues */
     (obj->otyp == STATUE && mons[obj->corpsenm].msize >= mdef->data->msize) ||
#endif
				obj_resists(obj, 0, 0)) {
			if (flooreffects(obj, x, y, "fall")) continue;
			place_object(obj, x, y);
		    } else {
			if (obj->lamplit) end_burn(obj, TRUE);
			obj->nobj = oldminvent;
			oldminvent = obj;
		    }
		}
		/* defer statue creation until after inventory removal
		   so that saved monster traits won't retain any stale
		   item-conferred attributes */
		otmp = mkcorpstat(STATUE, KEEPTRAITS(mdef) ? mdef : 0,
				  mdef->data, x, y, FALSE);
		set_material(otmp, GOLD);
		if (mdef->mnamelth) otmp = oname(otmp, NAME(mdef));
		while ((obj = oldminvent) != 0) {
		    oldminvent = obj->nobj;
			set_material(obj, GOLD);
		    (void) add_to_container(otmp, obj);
		}
#ifndef GOLDOBJ
		if (mdef->mgold) {
			struct obj *au;
			au = mksobj(GOLD_PIECE, FALSE, FALSE);
			au->quan = mdef->mgold;
			au->owt = weight(au);
			(void) add_to_container(otmp, au);
			mdef->mgold = 0;
		}
#endif
		/* Archeologists should not break unique statues */
		if (mdef->data->geno & G_UNIQ)
			otmp->spe = 1;
		otmp->owt = weight(otmp);
	} else {
		oldminvent = 0;
		/* some objects may end up outside the statue */
		while ((obj = mdef->minvent) != 0) {
		    obj_extract_self(obj);
		    if (obj->owornmask)
			update_mon_intrinsics(mdef, obj, FALSE, TRUE);
		    obj_no_longer_held(obj);
		    if (obj->owornmask & W_WEP){
				setmnotwielded(mdef,obj);
				MON_NOWEP(mdef);
			}
		    if (obj->owornmask & W_SWAPWEP){
				setmnotwielded(mdef,obj);
				MON_NOSWEP(mdef);
			}
		    obj->owornmask = 0L;
		    if (is_boulder(obj) ||
#if 0				/* monsters don't carry statues */
     (obj->otyp == STATUE && mons[obj->corpsenm].msize >= mdef->data->msize) ||
#endif
				obj_resists(obj, 0, 0)) {
			if (flooreffects(obj, x, y, "fall")) continue;
			place_object(obj, x, y);
		    } else {
			if (obj->lamplit) end_burn(obj, TRUE);
			obj->nobj = oldminvent;
			oldminvent = obj;
		    }
		}
		while ((obj = oldminvent) != 0) {
		    oldminvent = obj->nobj;
			set_material(obj, GOLD);
			place_object(obj, x, y);
			stackobj(obj);
		}
		otmp = mksobj_at(ROCK, x, y, TRUE, FALSE);
		set_material(otmp, GOLD);
		if (mdef->mnamelth) otmp = oname(otmp, NAME(mdef));
	}
	
	stackobj(otmp);
	/* mondead() already does this, but we must do it before the newsym */
	if(glyph_is_invisible(levl[x][y].glyph))
	    unmap_object(x, y);
	if (cansee(x, y)) newsym(x,y);
	/* We don't currently trap the hero in the statue in this case but we could */
	if (u.uswallow && u.ustuck == mdef) wasinside = TRUE;
	mondead(mdef);
	if (wasinside) {
		if (is_animal(mdef->data))
			You("%s through an opening in the new %s.",
				locomotion(&youmonst, "jump"),
				xname(otmp));
	}
}

/* drop a glass statue or rock and remove monster */
void
monglassed(mdef)
register struct monst *mdef;
{
	struct obj *otmp, *obj, *oldminvent;
	xchar x = mdef->mx, y = mdef->my;
	boolean wasinside = FALSE;

	/* we have to make the statue before calling mondead, to be able to
	 * put inventory in it, and we have to check for lifesaving before
	 * making the statue....
	 */
	lifesaved_monster(mdef);
	if (mdef->mhp > 0) return;

	mdef->mtrapped = 0;	/* (see m_detach) */

	if ((int)mdef->data->msize > MZ_TINY ||
		    !rn2(2 + ((int) (mdef->data->geno & G_FREQ) > 2))) {
		oldminvent = 0;
		/* some objects may end up outside the statue */
		while ((obj = mdef->minvent) != 0) {
		    obj_extract_self(obj);
		    if (obj->owornmask)
			update_mon_intrinsics(mdef, obj, FALSE, TRUE);
		    obj_no_longer_held(obj);
		    if (obj->owornmask & W_WEP){
				setmnotwielded(mdef,obj);
				MON_NOWEP(mdef);
			}
		    if (obj->owornmask & W_SWAPWEP){
				setmnotwielded(mdef,obj);
				MON_NOSWEP(mdef);
			}
		    obj->owornmask = 0L;
		    if (is_boulder(obj) ||
#if 0				/* monsters don't carry statues */
     (obj->otyp == STATUE && mons[obj->corpsenm].msize >= mdef->data->msize) ||
#endif
				obj_resists(obj, 0, 0)) {
			if (flooreffects(obj, x, y, "fall")) continue;
			place_object(obj, x, y);
		    } else {
			if (obj->lamplit) end_burn(obj, TRUE);
			obj->nobj = oldminvent;
			oldminvent = obj;
		    }
		}
		/* defer statue creation until after inventory removal
		   so that saved monster traits won't retain any stale
		   item-conferred attributes */
		otmp = mkcorpstat(STATUE, KEEPTRAITS(mdef) ? mdef : 0,
				  mdef->data, x, y, FALSE);
		set_material(otmp, GLASS);
		if (mdef->mnamelth) otmp = oname(otmp, NAME(mdef));
		while ((obj = oldminvent) != 0) {
		    oldminvent = obj->nobj;
		    (void) add_to_container(otmp, obj);
		}
#ifndef GOLDOBJ
		if (mdef->mgold) {
			struct obj *au;
			au = mksobj(GOLD_PIECE, FALSE, FALSE);
			au->quan = mdef->mgold;
			au->owt = weight(au);
			(void) add_to_container(otmp, au);
			mdef->mgold = 0;
		}
#endif
		/* Archeologists should not break unique statues */
		if (mdef->data->geno & G_UNIQ)
			otmp->spe = 1;
		otmp->owt = weight(otmp);
	} else {
		oldminvent = 0;
		/* some objects may end up outside the statue */
		while ((obj = mdef->minvent) != 0) {
		    obj_extract_self(obj);
		    if (obj->owornmask)
			update_mon_intrinsics(mdef, obj, FALSE, TRUE);
		    obj_no_longer_held(obj);
		    if (obj->owornmask & W_WEP){
				setmnotwielded(mdef,obj);
				MON_NOWEP(mdef);
			}
		    if (obj->owornmask & W_SWAPWEP){
				setmnotwielded(mdef,obj);
				MON_NOSWEP(mdef);
			}
		    obj->owornmask = 0L;
		    if (is_boulder(obj) ||
#if 0				/* monsters don't carry statues */
     (obj->otyp == STATUE && mons[obj->corpsenm].msize >= mdef->data->msize) ||
#endif
				obj_resists(obj, 0, 0)) {
			if (flooreffects(obj, x, y, "fall")) continue;
			place_object(obj, x, y);
		    } else {
			if (obj->lamplit) end_burn(obj, TRUE);
			obj->nobj = oldminvent;
			oldminvent = obj;
		    }
		}
		while ((obj = oldminvent) != 0) {
		    oldminvent = obj->nobj;
			place_object(obj, x, y);
			stackobj(obj);
		}
		otmp = mksobj_at(ROCK, x, y, TRUE, FALSE);
		set_material(otmp, GLASS);
		if (mdef->mnamelth) otmp = oname(otmp, NAME(mdef));
	}
	
	stackobj(otmp);
	/* mondead() already does this, but we must do it before the newsym */
	if(glyph_is_invisible(levl[x][y].glyph))
	    unmap_object(x, y);
	if (cansee(x, y)) newsym(x,y);
	/* We don't currently trap the hero in the statue in this case but we could */
	if (u.uswallow && u.ustuck == mdef) wasinside = TRUE;
	mondead(mdef);
	if (wasinside) {
		if (is_animal(mdef->data))
			You("%s through an opening in the new %s.",
				locomotion(&youmonst, "jump"),
				xname(otmp));
	}
}

/* another monster has killed the monster mdef */
void
monkilled(mdef, fltxt, how)
register struct monst *mdef;
const char *fltxt;
int how;
{
	boolean be_sad = FALSE;		/* true if unseen pet is killed */

	if ((mdef->wormno ? worm_known(mdef) : cansee(mdef->mx, mdef->my))
		&& fltxt)
	    pline("%s is %s%s%s!", Monnam(mdef),
			nonliving(mdef->data) ? "destroyed" : "killed",
		    *fltxt ? " by the " : "",
		    fltxt
		 );
	else
	    be_sad = (mdef->mtame != 0);

	/* no corpses if digested or disintegrated */
	if(how == AD_DGST || how == -AD_RBRE || how == AD_DISN)
	    mondead(mdef);
	else
	    mondied(mdef);

	if (be_sad && mdef->mhp <= 0)
	    You("have a sad feeling for a moment, then it passes.");
	if (is_fern_spore(mdef->data)) {
		spore_dies(mdef);
	}
}

void
unstuck(mtmp)
register struct monst *mtmp;
{
	if(u.ustuck == mtmp) {
		if(u.uswallow){
			u.ux = mtmp->mx;
			u.uy = mtmp->my;
			u.uswallow = 0;
			u.uswldtim = 0;
			if (Punished) placebc();
			vision_full_recalc = 1;
			docrt();
		}
		u.ustuck = 0;
	}
}

void
killed(mtmp)
register struct monst *mtmp;
{
	xkilled(mtmp, 1);
}

/* the player has killed the monster mtmp */
void
xkilled(mtmp, dest)
	register struct monst *mtmp;
/*
 * Dest=1, normal; dest=0, don't print message; dest=2, don't drop corpse
 * either; dest=3, message but no corpse
 */
	int	dest;
{
	register int tmp, x = mtmp->mx, y = mtmp->my;
	register struct permonst *mdat;
	int mndx;
	register struct obj *otmp;
	register struct trap *t;
	boolean redisp = FALSE, illalarm = FALSE;
	boolean wasinside = u.uswallow && (u.ustuck == mtmp);


	/* KMH, conduct */
	u.uconduct.killer++;
	if(mtmp->data == &mons[PM_CROW] && u.sealsActive&SEAL_MALPHAS) unbind(SEAL_MALPHAS,TRUE);

	if (dest & 1) {
	    const char *verb = nonliving_mon(mtmp) ? "destroy" : "kill";

	    if (!wasinside && !canspotmon(mtmp))
		You("%s it!", verb);
	    else {
		You("%s %s!", verb,
		    !mtmp->mtame ? mon_nam(mtmp) :
			x_monnam(mtmp,
				 mtmp->mnamelth ? ARTICLE_NONE : ARTICLE_THE,
				 "poor",
				 mtmp->mnamelth ? SUPPRESS_SADDLE : 0,
				 FALSE));
	    }
	}

	if(Is_illregrd(&u.uz)){
		if(mtmp->mtrapped && t_at(x, y) && t_at(x, y)->ttyp == VIVI_TRAP){
			illalarm = TRUE;
		}
	}
	if (mtmp->mtrapped && (t = t_at(x, y)) != 0 &&
		(t->ttyp == PIT || t->ttyp == SPIKED_PIT) &&
		boulder_at(x, y))
	    dest |= 2;     /*
			    * Prevent corpses/treasure being created "on top"
			    * of the boulder that is about to fall in. This is
			    * out of order, but cannot be helped unless this
			    * whole routine is rearranged.
			    */

	/* your pet knows who just killed it...watch out */
	if (mtmp->mtame && !mtmp->isminion) EDOG(mtmp)->killed_by_u = 1;

	/* dispose of monster and make cadaver */
	if(stoned) monstone(mtmp);
	else if(golded) mongolded(mtmp);
	else if(glassed) monglassed(mtmp);
	else mondead(mtmp);

	if (mtmp->mhp > 0) { /* monster lifesaved */
		/* Cannot put the non-visible lifesaving message in
		 * lifesaved_monster() since the message appears only when you
		 * kill it (as opposed to visible lifesaving which always
		 * appears).
		 */
		stoned = FALSE;
		golded = FALSE;
		glassed = FALSE;
		if (!cansee(x,y)) pline("Maybe not...");
		return;
	}

	mdat = mtmp->data; /* note: mondead can change mtmp->data */
	mndx = monsndx(mdat);

	if (is_fern_spore(mdat)) {
		spore_dies(mtmp);
	}
	if (stoned) {
		stoned = FALSE;
		goto cleanup;
	}

	if (golded) {
		golded = FALSE;
		goto cleanup;
	}

	if (glassed) {
		glassed = FALSE;
		goto cleanup;
	}

	if((dest & 2) || LEVEL_SPECIFIC_NOCORPSE(mdat))
		goto cleanup;

#ifdef MAIL
	if(mdat == &mons[PM_MAIL_DAEMON]) {
		stackobj(mksobj_at(SCR_MAIL, x, y, FALSE, FALSE));
		redisp = TRUE;
	}
#endif
	if((!accessible(x, y) && !is_pool(x, y, FALSE)) ||
	   (x == u.ux && y == u.uy)) {
	    /* might be mimic in wall or corpse in lava or on player's spot */
	    redisp = TRUE;
	    if(wasinside) spoteffects(TRUE);
	} else if(x != u.ux || y != u.uy) {
		/* might be here after swallowed */
		if (!rn2(6) && !(mvitals[mndx].mvflags & G_NOCORPSE)
					&& mdat->mlet != S_KETER
					&& mdat->mlet != S_PLANT
					&& !(mtmp->mvanishes >= 0)
					&& !(mtmp->mclone)
					&& !(is_derived_undead_mon(mtmp))
					&& !(is_auton(mtmp->data))
		) {
			int typ;

			/*Death Drop*/
			otmp = mkobj_at(RANDOM_CLASS, x, y, TRUE);
			if(In_quest(&u.uz) && !Role_if(PM_CONVICT)){
				if(otmp->oclass == WEAPON_CLASS || otmp->oclass == ARMOR_CLASS) otmp->objsize = (&mons[urace.malenum])->msize;
				if(otmp->oclass == ARMOR_CLASS){
					if(is_suit(otmp)) otmp->bodytypeflag = ((&mons[urace.malenum])->mflagsb&MB_BODYTYPEMASK);
					else if(is_helmet(otmp)) otmp->bodytypeflag = ((&mons[urace.malenum])->mflagsb&MB_HEADMODIMASK);
					else if(is_shirt(otmp)) otmp->bodytypeflag = ((&mons[urace.malenum])->mflagsb&MB_HUMANOID) ? MB_HUMANOID : ((&mons[urace.malenum])->mflagsb&MB_BODYTYPEMASK);
				}
			}
			
			/* Don't create large objects from small monsters */
			typ = otmp->otyp;
			if (mdat->msize < MZ_HUMAN && typ != FOOD_RATION
			    && typ != LEASH
			    && typ != FIGURINE
			    && (otmp->owt > 3 ||
				objects[typ].oc_big /*oc_bimanual/oc_bulky*/ ||
				is_spear(otmp) || (is_pole(otmp) && otmp->otyp != AKLYS) ||
				typ == MORNING_STAR)
				&& !is_divider(mdat)
			) {
			    delobj(otmp);
			} else redisp = TRUE;
		}
		/* Whether or not it always makes a corpse is, in theory,
		 * different from whether or not the corpse is "special";
		 * if we want both, we have to specify it explicitly.
		 */
		if (corpse_chance(mtmp, (struct monst *)0, FALSE))
			(void) make_corpse(mtmp);
	}
	if(redisp) newsym(x,y);
cleanup:
	/* punish bad behaviour */
	if(is_human(mdat) && !is_derived_undead_mon(mtmp) && 
	  !(u.sealsActive&SEAL_MALPHAS) && (!always_hostile_mon(mtmp) && mtmp->malign <= 0) &&
	   (mndx < PM_ARCHEOLOGIST || mndx > PM_WIZARD) &&
	   u.ualign.type != A_CHAOTIC) {
		HTelepat &= ~INTRINSIC;
		change_luck(-2);
		You("murderer!");
		if(u.ualign.type == A_LAWFUL){u.hod += 10; u.ualign.sins += 5;}
		else{u.hod += 5; u.ualign.sins += 2;}
		if (Blind && !Blind_telepat)
		    see_monsters(); /* Can't sense monsters any more. */
	}
	if((mtmp->mpeaceful && !rn2(2)) || mtmp->mtame)	change_luck(-1);
	if (is_unicorn(mdat) &&
	   !is_derived_undead_mon(mtmp) &&
		sgn(u.ualign.type) == sgn(mdat->maligntyp) && 
		u.ualign.type != A_VOID
	) {
		change_luck(-5);
		u.hod += 10;
		You_feel("guilty...");
	}
	/*Killing the specimens in the Dungeon of Ill-Regard angers the autons*/
	if(illalarm){
		u.uevent.uaxus_foe = 1;
		pline("An alarm sounds!");
		aggravate();
	}
	/* give experience points */
	tmp = experience(mtmp, (int)mvitals[mndx].died + 1);
	more_experienced(tmp, 0);
	newexplevel();		/* will decide if you go up */

	/* adjust alignment points */
	if (mtmp->m_id == quest_status.leader_m_id && !is_derived_undead_mon(mtmp)) {
		if(flags.leader_backstab){ /* They attacked you! */
			adjalign((int)(ALIGNLIM/4));
			// pline("That was %sa bad idea...",
					// u.uevent.qcompleted ? "probably " : "");
		} else {/* REAL BAD! */
			adjalign(-(u.ualign.record+(int)ALIGNLIM/2));
			u.hod += 30;
			pline("That was %sa bad idea...",
					u.uevent.qcompleted ? "probably " : "");
		}
	} else if (mdat->msound == MS_NEMESIS){	/* Real good! */
	    adjalign((int)(ALIGNLIM/4));
		u.hod = max(u.hod-10,0);
	} else if (mdat->msound == MS_GUARDIAN && mdat != &mons[PM_THUG] && !is_derived_undead_mon(mtmp)) {	/* Bad *//*nobody cares if you kill thugs*/
	    adjalign(-(int)(ALIGNLIM/8));											/*what's a little murder amongst rogues?*/
		u.hod += 10;
	    if (!Hallucination) pline("That was probably a bad idea...");
	    else pline("Whoopsie-daisy!");
	}else if (mtmp->ispriest && !is_derived_undead_mon(mtmp)) {
		adjalign((p_coaligned(mtmp)) ? -2 : 2);
		/* cancel divine protection for killing your priest */
		if (p_coaligned(mtmp)) u.ublessed = 0;
		if (mdat->maligntyp == A_NONE)
			adjalign((int)(ALIGNLIM / 4));		/* BIG bonus */
	} else if (mtmp->mtame && !Role_if(PM_EXILE)) {
		adjalign(-15);	/* bad!! */
		/* your god is mighty displeased... */
		if (!Hallucination) You_hear("the rumble of distant thunder...");
		else You_hear("the studio audience applaud!");
	} else if (mtmp->mpeaceful)
		adjalign(-5);
	
	if(((mtmp->mferal || mtmp->mtame)) && u.sealsActive&SEAL_BERITH) unbind(SEAL_BERITH,TRUE);

	/* malign was already adjusted for u.ualign.type and randomization */
	adjalign(mtmp->malign);
}

/* changes the monster into a stone monster of the same type */
/* this should only be called when poly_when_stoned() is true */
void
mon_to_stone(mtmp)
    register struct monst *mtmp;
{
    if(mtmp->data->mlet == S_GOLEM) {
	/* it's a golem, and not a stone golem */
	if(canseemon(mtmp))
	    pline("%s solidifies...", Monnam(mtmp));
	if (newcham(mtmp, &mons[PM_STONE_GOLEM], FALSE, FALSE)) {
	    if(canseemon(mtmp))
		pline("Now it's %s.", an(mtmp->data->mname));
	} else {
	    if(canseemon(mtmp))
		pline("... and returns to normal.");
	}
    } else
	impossible("Can't polystone %s!", a_monnam(mtmp));
}

void
mon_to_gold(mtmp)
    register struct monst *mtmp;
{
    if(mtmp->data->mlet == S_GOLEM) {
	/* it's a golem, and not a stone golem */
	if(canseemon(mtmp))
	    pline("%s turns shiny...", Monnam(mtmp));
	if (newcham(mtmp, &mons[PM_GOLD_GOLEM], FALSE, FALSE)) {
	    if(canseemon(mtmp))
		pline("Now it's %s.", an(mtmp->data->mname));
	} else {
	    if(canseemon(mtmp))
		pline("... and returns to normal.");
	}
    } else
	impossible("Can't polygold %s!", a_monnam(mtmp));
}

void
mnexto(mtmp)	/* Make monster mtmp next to you (if possible) */
	struct monst *mtmp;
{
	coord mm;

#ifdef STEED
	if (mtmp == u.usteed) {
		/* Keep your steed in sync with you instead */
		mtmp->mx = u.ux;
		mtmp->my = u.uy;
		return;
	}
#endif

	if(!enexto(&mm, u.ux, u.uy, mtmp->data)) return;
	rloc_to(mtmp, mm.x, mm.y);
	return;
}

void
monline(mtmp)	/* Make monster mtmp next to you (if possible) */
	struct monst *mtmp;
{
	coord mm;

#ifdef STEED
	if (mtmp == u.usteed) {
		/* Keep your steed in sync with you instead */
		mtmp->mx = u.ux;
		mtmp->my = u.uy;
		return;
	}
#endif

	if(!eonline(&mm, u.ux, u.uy, mtmp->data)) return;
	rloc_to(mtmp, mm.x, mm.y);
	return;
}

void
mofflin(mtmp)	/* Make monster mtmp next to you (if possible) */
	struct monst *mtmp;
{
	coord mm;

#ifdef STEED
	if (mtmp == u.usteed) {
		/* Keep your steed in sync with you instead */
		mtmp->mx = u.ux;
		mtmp->my = u.uy;
		return;
	}
#endif

	if(!eofflin(&mm, u.ux, u.uy, mtmp->data)) return;
	rloc_to(mtmp, mm.x, mm.y);
	return;
}

/* mnearto()
 * Put monster near (or at) location if possible.
 * Returns:
 *	1 - if a monster was moved from x, y to put mtmp at x, y.
 *	0 - in most cases.
 */
boolean
mnearto(mtmp,x,y,move_other)
register struct monst *mtmp;
xchar x, y;
boolean move_other;	/* make sure mtmp gets to x, y! so move m_at(x, y) */
{
	struct monst *othermon = (struct monst *)0;
	xchar newx, newy;
	coord mm;

	if ((mtmp->mx == x) && (mtmp->my == y)) return(FALSE);

	if (move_other && (othermon = m_at(x, y))) {
		if (othermon->wormno)
			remove_worm(othermon);
		else
			remove_monster(x, y);
	}

	newx = x;
	newy = y;

	if (!goodpos(newx, newy, mtmp, 0)) {
		/* actually we have real problems if enexto ever fails.
		 * migrating_mons that need to be placed will cause
		 * no end of trouble.
		 */
		if (!enexto(&mm, newx, newy, mtmp->data)) return(FALSE);
		newx = mm.x; newy = mm.y;
	}

	rloc_to(mtmp, newx, newy);

	if (move_other && othermon) {
	    othermon->mx = othermon->my = 0;
	    (void) mnearto(othermon, x, y, FALSE);
	    if ((othermon->mx != x) || (othermon->my != y))
		return(TRUE);
	}

	return(FALSE);
}


static const char *poiseff[] = {

	" feel weaker", "r brain is on fire",
	"r judgement is impaired", "r muscles won't obey you",
	" feel very sick", " break out in hives"
};

void
poisontell(typ)

	int	typ;
{
	pline("You%s.", poiseff[typ]);
}

void
poisoned(string, typ, pname, fatal, opoistype)
const char *string, *pname;
int  typ, fatal, opoistype;
{
	int i, plural, kprefix = KILLED_BY_AN;
	boolean thrown_weapon = (fatal < 0);

	if (thrown_weapon) fatal = -fatal;
	if(strcmp(string, "blast") && !thrown_weapon) {
	    /* 'blast' has already given a 'poison gas' message */
	    /* so have "poison arrow", "poison dart", etc... */
	    plural = (string[strlen(string) - 1] == 's')? 1 : 0;
	    /* avoid "The" Orcus's sting was poisoned... */
	    pline("%s%s %s poisoned!", isupper(*string) ? "" : "The ",
			string, plural ? "were" : "was");
	}
	
	if(!opoistype || (opoistype & OPOISON_BASIC)){
		if(Poison_resistance) {
			if(!strcmp(string, "blast")) shieldeff(u.ux, u.uy);
			pline_The("poison doesn't seem to affect you.");
		} else {
			/* suppress killer prefix if it already has one */
			if ((i = name_to_mon(pname)) >= LOW_PM && mons[i].geno & G_UNIQ) {
				kprefix = KILLED_BY;
				if (!type_is_pname(&mons[i])) pname = the(pname);
			} else if (!strncmpi(pname, "the ", 4) ||
				!strncmpi(pname, "an ", 3) ||
				!strncmpi(pname, "a ", 2)) {
				/*[ does this need a plural check too? ]*/
				kprefix = KILLED_BY;
			}
			i = rn2(fatal + 20*thrown_weapon);
			if(i == 0 && typ != A_CHA) {
				if (adjattrib(A_CON, typ==A_CON ? -2 : -rn1(3,3), 1))
					pline_The("poison was quite debilitating...");
			} else if(i <= 5) {
				/* Check that a stat change was made */
				if (adjattrib(typ, thrown_weapon ? -1 : -rn1(3,3), 1))
					pline("You%s!", poiseff[typ]);
			} else {
				i = thrown_weapon ? rnd(6) : rn1(10,6);
				if(Half_physical_damage) i = (i+1) / 2;
				if(u.uvaul_duration) i = (i + 1) / 2;
				losehp(i, pname, kprefix);
			}
			if(u.uhp < 1) {
				killer_format = kprefix;
				killer = pname;
				/* "Poisoned by a poisoned ___" is redundant */
				done(strstri(pname, "poison") ? DIED : POISONING);
			}
			(void) encumber_msg();
		}
	}
	if(opoistype & OPOISON_FILTH){
		if(Sick_resistance) {
			pline_The("tainted filth doesn't seem to affect you.");
		} else {
			long sick_time;

			sick_time = (long)rn1(ACURR(A_CON), 20);
			/* make sure new ill doesn't result in improvement */
			if (Sick && (sick_time > Sick))
				sick_time = (Sick > 2L) ? Sick/2L : 1L;
			make_sick(sick_time, string, TRUE, SICK_NONVOMITABLE);
		}
	}
	if(opoistype & OPOISON_SLEEP){
		if(Sleep_resistance) {
			pline_The("drug doesn't seem to affect you.");
		} else if(!rn2((fatal/3) + 20*thrown_weapon)){
			You("suddenly fall asleep!");
			fall_asleep(-rn1(2, 6), TRUE);
		}
	}
	if(opoistype & OPOISON_BLIND){
		if(Poison_resistance) {
			pline_The("poison doesn't seem to affect you.");
		} else if(!rn2((fatal/3) + 20*thrown_weapon)){
			i = thrown_weapon ? 3 : 8;
			if(Half_physical_damage) i = (i+1) / 2;
			if(u.uvaul_duration) i = (i + 1) / 2;
			losehp(i, pname, kprefix);
			make_blinded(rn1(20, 25),
					 (boolean)!Blind);
		} else{
			i = thrown_weapon ? rnd(3) : rn1(5,3);
			if(Half_physical_damage) i = (i+1) / 2;
			if(u.uvaul_duration) i = (i + 1) / 2;
			losehp(i, pname, kprefix);
		}
	}
	if(opoistype & OPOISON_PARAL){
		if(Free_action) {
			pline_The("poison doesn't seem to affect you.");
		} else if(!rn2((fatal) + 20*thrown_weapon)){
			i = thrown_weapon ? 6 : 16;
			if(Half_physical_damage) i = (i+1) / 2;
			if(u.uvaul_duration) i = (i + 1) / 2;
			losehp(i, pname, kprefix);
			nomul(-(rn1(5, 12 - 6)), "immobilized by paralysis venom");
		} else{
			i = thrown_weapon ? rnd(6) : rn1(10,6);
			if(Half_physical_damage) i = (i+1) / 2;
			if(u.uvaul_duration) i = (i + 1) / 2;
			losehp(i, pname, kprefix);
		}
	}
	if(opoistype & OPOISON_AMNES){
		if(u.sealsActive&SEAL_HUGINN_MUNINN){
			unbind(SEAL_HUGINN_MUNINN,TRUE);
		} else {
			forget(1);	/* lose 1% of memory per point lost*/
			forget_traps();		/* lose memory of all traps*/
		}
	}
	if(opoistype & OPOISON_ACID){
		if(Acid_resistance) {
			pline_The("acidic coating doesn't seem to affect you.");
		} else {
			/* suppress killer prefix if it already has one */
			if ((i = name_to_mon(pname)) >= LOW_PM && mons[i].geno & G_UNIQ) {
				kprefix = KILLED_BY;
				if (!type_is_pname(&mons[i])) pname = the(pname);
			} else if (!strncmpi(pname, "the ", 4) ||
				!strncmpi(pname, "an ", 3) ||
				!strncmpi(pname, "a ", 2)) {
				/*[ does this need a plural check too? ]*/
				kprefix = KILLED_BY;
			}
			i = thrown_weapon ? rnd(10) : rn1(10,10);
			losehp(i, pname, kprefix);
			if(u.uhp < 1) {
				killer_format = kprefix;
				killer = pname;
				done(DIED);
			}
		}
	}
}

/* monster responds to player action; not the same as a passive attack */
/* assumes reason for response has been tested, and response _must_ be made */
void
m_respond(mtmp)
register struct monst *mtmp;
{
	register int i;
    if(mtmp->data->msound == MS_SHRIEK) {
		if(flags.soundok) {
			pline("%s shrieks.", Monnam(mtmp));
			stop_occupation();
		}
		if (!rn2(10)) {
			if (!rn2(13))
			(void) makemon(&mons[PM_PURPLE_WORM], 0, 0, NO_MM_FLAGS);
			else
			(void) makemon((struct permonst *)0, 0, 0, NO_MM_FLAGS);

		}
		aggravate();
    } else if(!(mtmp->mspec_used) &&
		(
		mtmp->data->msound == MS_JUBJUB || mtmp->data->msound == MS_DREAD || 
		mtmp->data->msound == MS_SONG || mtmp->data->msound == MS_OONA ||
		mtmp->data->msound == MS_INTONE || mtmp->data->msound == MS_FLOWER ||
		mtmp->data->msound == MS_TRUMPET || mtmp->data == &mons[PM_RHYMER]
		)
	) {
		domonnoise(mtmp, FALSE);
    }
	for(i = 0; i < NATTK; i++)
		 if(mtmp->data->mattk[i].aatyp == AT_WDGZ) {
			 if (!(ublindf && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD))	// the Eyes of the Overworld protect you from whatever you might see
				(void) gazemu(mtmp, &mtmp->data->mattk[i]);
		 }
    if(is_weeping(mtmp->data)) {
		for(i = 0; i < NATTK; i++)
			 if(mtmp->data->mattk[i].aatyp == AT_GAZE) {
			 (void) gazemu(mtmp, &mtmp->data->mattk[i]);
			 break;
			 }
    }
}

#endif /* OVLB */
#ifdef OVL2

void
setmangry(mtmp)
register struct monst *mtmp;
{
	mtmp->mstrategy &= ~STRAT_WAITMASK;
	if(!mtmp->mpeaceful) return;
	if(mtmp->mtame) return;
	mtmp->mpeaceful = 0;
	newsym(mtmp->mx, mtmp->my);
	if(mtmp->ispriest) {
		if(p_coaligned(mtmp)) adjalign(-5); /* very bad */
		else adjalign(2);
	} else
		adjalign(-1);		/* attacking peaceful monsters is bad */
	
	if(mtmp->data == &mons[PM_DANCING_BLADE]){
		struct monst *surya;
		for(surya = fmon; surya; surya = surya->nmon) if(surya->m_id == mtmp->mvar1) break;
		if(surya) setmangry(surya);
	}
	if(mtmp->data == &mons[PM_SURYA_DEVA]){
		struct monst *blade;
		for(blade = fmon; blade; blade = blade->nmon) if(blade->data == &mons[PM_DANCING_BLADE] && mtmp->m_id == blade->mvar1) break;
		if(blade) setmangry(blade);
	}
	if (couldsee(mtmp->mx, mtmp->my)) {
		if (humanoid(mtmp->data) || mtmp->isshk || mtmp->isgd)
		    pline("%s gets angry!", Monnam(mtmp));
		else if (flags.verbose && flags.soundok) growl(mtmp);
		if(flags.stag && (mtmp->data == &mons[PM_DROW_MATRON_MOTHER] || mtmp->data == &mons[PM_ECLAVDRA])){
			struct monst *tm;
			for(tm = fmon; tm; tm = tm->nmon){
				if(tm->mfaction != EILISTRAEE_SYMBOL && 
					tm->mfaction != XAXOX && tm->mfaction != EDDER_SYMBOL && 
					is_drow(tm->data) && !tm->mtame 
				){
					tm->housealert = 1;
					tm->mpeaceful = 0;
					tm->mstrategy &= ~STRAT_WAITMASK;
					set_malign(tm);
				}
			}
		}
	}

	/* attacking your own quest leader will anger his or her guardians */
	if (!flags.mon_moving &&	/* should always be the case here */
		mtmp->m_id == quest_status.leader_m_id) {
		// mtmp->data == &mons[quest_info(MS_LEADER)]) {
	    struct monst *mon;
	    struct permonst *q_guardian = &mons[quest_info(MS_GUARDIAN)];
	    int got_mad = 0;

	    /* guardians will sense this attack even if they can't see it */
	    for (mon = fmon; mon; mon = mon->nmon)
		if (!DEADMONSTER(mon) && mon->data == q_guardian && mon->mpeaceful) {
		    mon->mpeaceful = 0;
		    if (canseemon(mon)) ++got_mad;
		}
	    if (got_mad && !Hallucination)
		pline_The("%s appear%s to be angry too...",
		      got_mad == 1 ? q_guardian->mname :
				    makeplural(q_guardian->mname),
		      got_mad == 1 ? "s" : "");
	}
}

void
wakeup(mtmp, anger)
register struct monst *mtmp;
int anger;
{
	mtmp->msleeping = 0;
	mtmp->meating = 0;	/* assume there's no salvagable food left */
	if(anger) setmangry(mtmp);
	if(mtmp->m_ap_type) seemimic(mtmp);
	else if (flags.forcefight && !flags.mon_moving && mtmp->mundetected) {
	    mtmp->mundetected = 0;
	    newsym(mtmp->mx, mtmp->my);
	}
}

/* Wake up nearby monsters. */
void
wake_nearby()
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (!DEADMONSTER(mtmp) && distu(mtmp->mx,mtmp->my) < u.ulevel*20) {
			mtmp->msleeping = 0;
	    }
	}
}

/* Wake up nearby monsters, deafening those with sensitive ears. */
void
wake_nearby_noisy(){
	register struct monst *mtmp;
	wake_nearby();
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (!DEADMONSTER(mtmp) && sensitive_ears(mtmp->data) && !is_deaf(mtmp) &&
				 distu(mtmp->mx,mtmp->my) < (u.ulevel*20)/3){
			mtmp->mstun = 1;
			mtmp->mconf = 1;
			mtmp->mcanhear = 0;
			mtmp->mdeafened = (u.ulevel*20)/3 - distu(mtmp->mx,mtmp->my);
		}
	}
}

/* Wake up monsters near some particular location. */
void
wake_nearto(x, y, distance)
register int x, y, distance;
{
	register struct monst *mtmp;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (!DEADMONSTER(mtmp) && mtmp->msleeping && !is_deaf(mtmp) && (distance == 0 ||
				 dist2(mtmp->mx, mtmp->my, x, y) < distance)
		){
			mtmp->msleeping = 0;
			if(mtmp->mux == 0 && mtmp->muy == 0){
				mtmp->mux = x;
				mtmp->muy = y;
			}
		}
	}
}

/* Wake up monsters near some particular location, deafening those with sensitive ears. */
void
wake_nearto_noisy(x, y, distance)
register int x, y, distance;
{
	register struct monst *mtmp;
	wake_nearto(x,y,distance);
	distance /= 3;
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (!DEADMONSTER(mtmp)){
			if(sensitive_ears(mtmp->data) && !is_deaf(mtmp) &&
			 dist2(mtmp->mx, mtmp->my, x, y) < distance
			){
				mtmp->mstun = 1;
				mtmp->mconf = 1;
				mtmp->mcanhear = 0;
				mtmp->mdeafened = distance - dist2(mtmp->mx, mtmp->my, x, y);
			}
			if(mtmp->data == &mons[PM_ECHO]){
				struct monst *tmpm;
				int targets = 0, damage = 0;
				for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
					if(distmin(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= 4
						&& tmpm->mpeaceful != mtmp->mpeaceful
						&& tmpm->mtame != mtmp->mtame
						&& !nonliving_mon(tmpm)
						&& !resists_drain(tmpm)
						&& !DEADMONSTER(tmpm)
					) targets++;
				}
				if(dist2(u.ux,u.uy,mtmp->mx,mtmp->my) <= distance
					&& !mtmp->mpeaceful
					&& !mtmp->mtame
					&& !nonliving(youracedata)
				) targets++;
				targets = rnd(targets);
				for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
					if(dist2(tmpm->mx,tmpm->my,mtmp->mx,mtmp->my) <= distance
						&& tmpm->mpeaceful != mtmp->mpeaceful
						&& tmpm->mtame != mtmp->mtame
						&& !nonliving_mon(tmpm)
						&& !resists_drain(tmpm)
						&& !DEADMONSTER(tmpm)
					) targets--;
					if(!targets) break;
				}
				if(tmpm){
					if(canseemon(tmpm)){
						pline("Powerful reverberations shake %s to %s soul!", mon_nam(tmpm), hisherits(tmpm));
					}
					tmpm->mconf = 1;
					tmpm->mcanhear = 0;
					tmpm->mdeafened = distance - dist2(tmpm->mx, tmpm->my, x, y);
					damage = tmpm->m_lev/2+1;
					if(damage > 0){
						tmpm->mhp -= 8*damage;
						tmpm->mhpmax -= 8*damage;
						tmpm->m_lev -= damage;
						if(tmpm->mhp < 1
						|| tmpm->mhpmax < 1
						|| tmpm->m_lev < 0
						){
							grow_up(mtmp,tmpm);
							mondied(tmpm);
						}
					}
				} else if(targets > 0
					&& dist2(u.ux,u.uy,mtmp->mx,mtmp->my) <= distance
					&& !mtmp->mpeaceful
					&& !mtmp->mtame
					&& !Drain_resistance
					&& !nonliving(youracedata)
				){
					pline("Powerful reverberations shake you to your soul!");
					damage = u.ulevel/2+1;
					while(--damage)
						losexp("soul echoes",FALSE,TRUE,TRUE);
					losexp("soul echoes",TRUE,TRUE,TRUE);
					losehp(8, "soul echoes", KILLED_BY); //might kill you before you hit level 0;
				}
			}
		}
	}
}

/* NOTE: we must check for mimicry before calling this routine */
void
seemimic(mtmp)
register struct monst *mtmp;
{
	unsigned old_app = mtmp->mappearance;
	uchar old_ap_type = mtmp->m_ap_type;

	mtmp->m_ap_type = M_AP_NOTHING;
	mtmp->mappearance = 0;

	/*
	 *  Discovered mimics don't block light.
	 */
	if (((old_ap_type == M_AP_FURNITURE &&
	      (old_app == S_hcdoor || old_app == S_vcdoor)) ||
	     (old_ap_type == M_AP_OBJECT && old_app == BOULDER)) &&
	    !does_block(mtmp->mx, mtmp->my, &levl[mtmp->mx][mtmp->my]))
	    unblock_point(mtmp->mx, mtmp->my);

	newsym(mtmp->mx,mtmp->my);
}

/* force all chameleons to become normal */
void
rescham()
{
	register struct monst *mtmp;
	int mcham;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		mcham = (int) mtmp->cham;
		if (mcham) {
			mtmp->cham = CHAM_ORDINARY;
			(void) newcham(mtmp, &mons[cham_to_pm[mcham]],
				       FALSE, FALSE);
		}
		if(is_were(mtmp->data) && mtmp->data->mlet != S_HUMAN)
			new_were(mtmp);
		if(mtmp->m_ap_type && cansee(mtmp->mx, mtmp->my)) {
			seemimic(mtmp);
			/* we pretend that the mimic doesn't */
			/* know that it has been unmasked.   */
			mtmp->msleeping = 1;
		}
	}
}

/* Let the chameleons change again -dgk */
void
restartcham()
{
	register struct monst *mtmp;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		mtmp->cham = pm_to_cham(monsndx(mtmp->data));
		if (mtmp->data->mlet == S_MIMIC && mtmp->msleeping &&
				cansee(mtmp->mx, mtmp->my)) {
			set_mimic_sym(mtmp);
			newsym(mtmp->mx,mtmp->my);
		}
	}
}

/* called when restoring a monster from a saved level; protection
   against shape-changing might be different now than it was at the
   time the level was saved. */
void
restore_cham(mon)
struct monst *mon;
{
	int mcham;

	if (Protection_from_shape_changers) {
	    mcham = (int) mon->cham;
	    if (mcham) {
		mon->cham = CHAM_ORDINARY;
		(void) newcham(mon, &mons[cham_to_pm[mcham]], FALSE, FALSE);
	    } else if (is_were(mon->data) && !is_human(mon->data)) {
		new_were(mon);
	    }
	} else if (mon->cham == CHAM_ORDINARY) {
	    mon->cham = pm_to_cham(monsndx(mon->data));
	}
}

/* unwatched hiders may hide again; if so, a 1 is returned.  */
STATIC_OVL boolean
restrap(mtmp)
register struct monst *mtmp;
{
	if(mtmp->cham || mtmp->mcan || mtmp->m_ap_type ||
	   cansee(mtmp->mx, mtmp->my) || rn2(3) || (mtmp == u.ustuck) ||
	   (sensemon(mtmp) && distu(mtmp->mx, mtmp->my) <= 2))
		return(FALSE);

	if(mtmp->data->mlet == S_MIMIC) {
		set_mimic_sym(mtmp);
		return(TRUE);
	} else
	    if(levl[mtmp->mx][mtmp->my].typ == ROOM)  {
		mtmp->mundetected = 1;
		return(TRUE);
	    }

	return(FALSE);
}

short *animal_list = 0;		/* list of PM values for animal monsters */
int animal_list_count;

void
mon_animal_list(construct)
boolean construct;
{
	if (construct) {
	    short animal_temp[SPECIAL_PM];
	    int i, n;

	 /* if (animal_list) impossible("animal_list already exists"); */

	    for (n = 0, i = LOW_PM; i < SPECIAL_PM; i++)
		if (is_animal(&mons[i])) animal_temp[n++] = i;
	 /* if (n == 0) animal_temp[n++] = NON_PM; */

	    animal_list = (short *)alloc(n * sizeof *animal_list);
	    (void) memcpy((genericptr_t)animal_list,
			  (genericptr_t)animal_temp,
			  n * sizeof *animal_list);
	    animal_list_count = n;
	} else {	/* release */
	    if (animal_list) free((genericptr_t)animal_list), animal_list = 0;
	    animal_list_count = 0;
	}
}

STATIC_OVL int
pick_animal()
{
	if (!animal_list) mon_animal_list(TRUE);

	return animal_list[rn2(animal_list_count)];
}

STATIC_OVL int
select_newcham_form(mon)
struct monst *mon;
{
	int mndx = NON_PM;

	switch (mon->cham) {
	    case CHAM_SANDESTIN:
		if (rn2(7)) mndx = pick_nasty();
		break;
	    case CHAM_DOPPELGANGER:
		if (!rn2(7)) mndx = pick_nasty();
		else if (rn2(3)) mndx = rn1(PM_WIZARD - PM_ARCHEOLOGIST + 1,
					    PM_ARCHEOLOGIST);
		break;
	    case CHAM_CHAMELEON:
		if (!rn2(3)) mndx = pick_animal();
		else mndx = rndshape();//try to get an in-depth monster of any kind
		break;
	    case CHAM_DREAM:
		if(rn2(2)) mndx = rndshape();//try to get an in-depth monster of any kind
		else mndx = PM_DREAM_QUASIELEMENTAL;
		break;
	    case CHAM_ORDINARY:
	      {
		struct obj *m_armr = which_armor(mon, W_ARM);

		if (m_armr && Is_dragon_scales(m_armr))
		    mndx = Dragon_scales_to_pm(m_armr) - mons;
		else if (m_armr && Is_dragon_mail(m_armr))
		    mndx = Dragon_mail_to_pm(m_armr) - mons;
	      }
		break;
	}
#ifdef WIZARD
	/* For debugging only: allow control of polymorphed monster; not saved */
	if (wizard && iflags.mon_polycontrol) {
		char pprompt[BUFSZ], buf[BUFSZ];
		int tries = 0;
		do {
			Sprintf(pprompt,
				"Change %s into what kind of monster? [type the name]",
				mon_nam(mon));
			getlin(pprompt,buf);
			mndx = name_to_mon(buf);
			if (mndx < LOW_PM)
				You("cannot polymorph %s into that.", mon_nam(mon));
			else break;
		} while(++tries < 5);
		if (tries==5) pline1(thats_enough_tries);
	}
#endif /*WIZARD*/
	if (mndx == NON_PM) mndx = rndshape();//first try to get an in-depth monster
	if (mndx == NON_PM) mndx = rn1(SPECIAL_PM - LOW_PM, LOW_PM);//double check in case no monst was returned
	return mndx;
}

/* make a chameleon look like a new monster; returns 1 if it actually changed */
int
newcham(mtmp, mdat, polyspot, msg)
struct monst *mtmp;
struct permonst *mdat;
boolean polyspot;	/* change is the result of wand or spell of polymorph */
boolean msg;		/* "The oldmon turns into a newmon!" */
{
	int mhp, hpn, hpd;
	int mndx, tryct;
	struct permonst *olddata = mtmp->data;
	char oldname[BUFSZ];
	
	if (msg) {
	    /* like Monnam() but never mention saddle */
	    Strcpy(oldname, x_monnam(mtmp, ARTICLE_THE, (char *)0,
				     SUPPRESS_SADDLE, FALSE));
	    oldname[0] = highc(oldname[0]);
	}

	/* mdat = 0 -> caller wants a random monster shape */
	tryct = 0;
	if (mdat == 0) {
	    while (++tryct <= 100) {
		mndx = select_newcham_form(mtmp);
		mdat = &mons[mndx];
		if ((mvitals[mndx].mvflags & G_GENOD && !In_quest(&u.uz)) != 0 ||
			is_placeholder(mdat)) continue;
		/* polyok rules out all MG_PNAME and MA_WERE's;
		   select_newcham_form might deliberately pick a player
		   character type, so we can't arbitrarily rule out all
		   human forms any more */
		if (is_mplayer(mdat) || (!is_human(mdat) && polyok(mdat)))
		    break;
	    }
	    if (tryct > 100) return 0;	/* Should never happen */
	} else if (mvitals[monsndx(mdat)].mvflags & G_GENOD && !In_quest(&u.uz))
	    return(0);	/* passed in mdat is genocided */

	if(is_male(mdat)) {
		if(mtmp->female) mtmp->female = FALSE;
	} else if (is_female(mdat)) {
		if(!mtmp->female) mtmp->female = TRUE;
	} else if (!is_neuter(mdat)) {
		if(!rn2(10)) mtmp->female = !mtmp->female;
	}

	if (In_endgame(&u.uz) && is_mplayer(olddata)) {
		/* mplayers start out as "Foo the Bar", but some of the
		 * titles are inappropriate when polymorphed, particularly
		 * into the opposite sex.  players don't use ranks when
		 * polymorphed, so dropping the rank for mplayers seems
		 * reasonable.
		 */
		char *p = index(NAME(mtmp), ' ');
		if (p) {
			*p = '\0';
			mtmp->mnamelth = p - NAME(mtmp) + 1;
		}
	}

	if(mdat == mtmp->data) return(0);	/* still the same monster */

	if(mtmp->wormno) {			/* throw tail away */
		wormgone(mtmp);
		place_monster(mtmp, mtmp->mx, mtmp->my);
	}
	
	/* Possibly Unblock  */
	if(!opaque(mdat) && opaque(mtmp->data)) unblock_point(mtmp->mx, mtmp->my);
	/* Possibly Block  */
	if(opaque(mdat) && !opaque(mtmp->data)) block_point(mtmp->mx, mtmp->my);
	
	if(mtmp->cham != CHAM_DREAM){
		hpn = mtmp->mhp;
		hpd = (mtmp->m_lev < 50) ? ((int)mtmp->m_lev)*8 : mdat->mlevel;
		if(!hpd) hpd = 4;

		mtmp->m_lev = adj_lev(mdat);		/* new monster level */

		mhp = (mtmp->m_lev < 50) ? ((int)mtmp->m_lev)*8 : mdat->mlevel;
		if(!mhp) mhp = 4;

		/* new hp: same fraction of max as before */
#ifndef LINT
		mtmp->mhp = (int)(((long)hpn*(long)mhp)/(long)hpd);
#endif
		if(mtmp->mhp < 0) mtmp->mhp = hpn;	/* overflow */
	/* Unlikely but not impossible; a 1HD creature with 1HP that changes into a
	   0HD creature will require this statement */
		if (!mtmp->mhp) mtmp->mhp = 1;

	/* and the same for maximum hit points */
		hpn = mtmp->mhpmax;
#ifndef LINT
		mtmp->mhpmax = (int)(((long)hpn*(long)mhp)/(long)hpd);
#endif
		if(mtmp->mhpmax < 0) mtmp->mhpmax = hpn;	/* overflow */
		if (!mtmp->mhpmax) mtmp->mhpmax = 1;
	} //else just take on new form I think....
	/* take on the new form... */
	set_mon_data(mtmp, mdat, 0);

	if (emits_light(olddata) != emits_light(mtmp->data)) {
	    /* used to give light, now doesn't, or vice versa,
	       or light's range has changed */
	    if (emits_light(olddata))
		del_light_source(LS_MONSTER, (genericptr_t)mtmp, FALSE);
	    if (emits_light(mtmp->data))
		new_light_source(mtmp->mx, mtmp->my, emits_light(mtmp->data),
				 LS_MONSTER, (genericptr_t)mtmp);
	}
	if (!mtmp->perminvis || pm_invisible(olddata))
	    mtmp->perminvis = pm_invisible(mdat);
	mtmp->minvis = mtmp->invis_blkd ? 0 : mtmp->perminvis;
	if (!(hides_under(mdat) && OBJ_AT(mtmp->mx, mtmp->my)) &&
			!(mdat->mlet == S_EEL && is_pool(mtmp->mx, mtmp->my, FALSE)))
		mtmp->mundetected = 0;
	if (u.ustuck == mtmp) {
		if(u.uswallow) {
			if(!attacktype(mdat,AT_ENGL)) {
				/* Does mdat care? */
				if (!noncorporeal(mdat) && !amorphous(mdat) &&
				    !is_whirly(mdat) &&
				    (mdat != &mons[PM_YELLOW_LIGHT])) {
					You("break out of %s%s!", mon_nam(mtmp),
					    (is_animal(mdat)?
					    "'s stomach" : ""));
					mtmp->mhp = 1;  /* almost dead */
				}
				expels(mtmp, olddata, FALSE);
			} else {
				/* update swallow glyphs for new monster */
				swallowed(0);
			}
		} else if (!sticks(mdat) && !sticks(youracedata))
			unstuck(mtmp);
	}

#ifndef DCC30_BUG
	if ((mdat == &mons[PM_LONG_WORM] || mndx == PM_HUNTING_HORROR) && 
		(mtmp->wormno = get_wormno()) != 0) {
#else
	/* DICE 3.0 doesn't like assigning and comparing mtmp->wormno in the
	 * same expression.
	 */
	if ((mdat == &mons[PM_LONG_WORM] || mndx == PM_HUNTING_HORROR) &&
		(mtmp->wormno = get_wormno(), mtmp->wormno != 0)) {
#endif
	    /* we can now create worms with tails - 11/91 */
	    initworm(mtmp, mndx == PM_HUNTING_HORROR ? 2 : rn2(5));
	    if (count_wsegs(mtmp))
		place_worm_tail_randomly(mtmp, mtmp->mx, mtmp->my);
	}

	newsym(mtmp->mx,mtmp->my);

	if (msg) {
	    uchar save_mnamelth = mtmp->mnamelth;
	    mtmp->mnamelth = 0;
	    pline("%s turns into %s!", oldname,
		  mdat == &mons[PM_GREEN_SLIME] ? "slime" :
		  x_monnam(mtmp, ARTICLE_A, (char*)0, SUPPRESS_SADDLE, FALSE));
	    mtmp->mnamelth = save_mnamelth;
	}

	possibly_unwield(mtmp, polyspot);	/* might lose use of weapon */
	mon_break_armor(mtmp, polyspot);
	if (!(mtmp->misc_worn_check & W_ARMG))
	    mselftouch(mtmp, "No longer petrify-resistant, ",
			!flags.mon_moving);
	m_dowear(mtmp, FALSE);

	/* This ought to re-test can_carry() on each item in the inventory
	 * rather than just checking ex-giants & boulders, but that'd be
	 * pretty expensive to perform.  If implemented, then perhaps
	 * minvent should be sorted in order to drop heaviest items first.
	 */
	/* former giants can't continue carrying boulders */
	if (mtmp->minvent && !throws_rocks(mdat)) {
	    register struct obj *otmp, *otmp2;

	    for (otmp = mtmp->minvent; otmp; otmp = otmp2) {
		otmp2 = otmp->nobj;
		if (is_boulder(otmp)) {
		    /* this keeps otmp from being polymorphed in the
		       same zap that the monster that held it is polymorphed */
		    if (polyspot) bypass_obj(otmp);
		    obj_extract_self(otmp);
		    /* probably ought to give some "drop" message here */
		    if (flooreffects(otmp, mtmp->mx, mtmp->my, "")) continue;
		    place_object(otmp, mtmp->mx, mtmp->my);
		}
	    }
	}

	return(1);
}

/* sometimes an egg will be special */
#define BREEDER_EGG (!rn2(77))

/*
 * Determine if the given monster number can be hatched from an egg.
 * Return the monster number to use as the egg's corpsenm.  Return
 * NON_PM if the given monster can't be hatched.
 */
int
can_be_hatched(mnum)
int mnum;
{
    /* ranger quest nemesis has the oviparous bit set, making it
       be possible to wish for eggs of that unique monster; turn
       such into ordinary eggs rather than forbidding them outright */
    if (mnum == PM_SCORPIUS) mnum = PM_SCORPION;
	else if(mnum == PM_ANCIENT_NAGA) mnum = rn2(PM_GUARDIAN_NAGA_HATCHLING - PM_RED_NAGA_HATCHLING + 1) + PM_RED_NAGA_HATCHLING;
	else if(mnum == PM_SERPENT_MAN_OF_YOTH) mnum = rn2(PM_COBRA - PM_GARTER_SNAKE + 1) + PM_GARTER_SNAKE;
	else if(mnum == PM_HUNTING_HORROR) mnum = PM_BABY_LONG_WORM;
	else if(mnum == PM_SMAUG) mnum = PM_BABY_RED_DRAGON;
	
    mnum = little_to_big(mnum, (boolean)rn2(2));
    /*
     * Queen bees lay killer bee eggs (usually), but killer bees don't
     * grow into queen bees.  Ditto for [winged-]gargoyles.
     */
    if (mnum == PM_KILLER_BEE || mnum == PM_GARGOYLE ||
	    (lays_eggs(&mons[mnum]) && (BREEDER_EGG ||
		(mnum != PM_QUEEN_BEE && mnum != PM_WINGED_GARGOYLE))))
	return mnum;
    return NON_PM;
}

/* type of egg laid by #sit; usually matches parent */
int
egg_type_from_parent(mnum, force_ordinary)
int mnum;	/* parent monster; caller must handle lays_eggs() check */
boolean force_ordinary;
{
    if (force_ordinary || !BREEDER_EGG) {
	if (mnum == PM_QUEEN_BEE) mnum = PM_KILLER_BEE;
	else if (mnum == PM_WINGED_GARGOYLE) mnum = PM_GARGOYLE;
    }
    return mnum;
}

/* decide whether an egg of the indicated monster type is viable; */
/* also used to determine whether an egg or tin can be created... */
boolean
dead_species(m_idx, egg)
int m_idx;
boolean egg;
{
	/*
	 * For monsters with both baby and adult forms, genociding either
	 * form kills all eggs of that monster.  Monsters with more than
	 * two forms (small->large->giant mimics) are more or less ignored;
	 * fortunately, none of them have eggs.  Species extinction due to
	 * overpopulation does not kill eggs.
	 */
	return (boolean)
		(m_idx >= LOW_PM &&
		 ((mvitals[m_idx].mvflags & G_GENOD && !In_quest(&u.uz)) != 0 ||
		  (egg &&
		   (mvitals[big_to_little(m_idx)].mvflags & G_GENOD && !In_quest(&u.uz)) != 0)));
}

/* kill off any eggs of genocided monsters */
STATIC_OVL void
kill_eggs(obj_list)
struct obj *obj_list;
{
	struct obj *otmp;

	for (otmp = obj_list; otmp; otmp = otmp->nobj)
	    if (otmp->otyp == EGG) {
		if (dead_species(otmp->corpsenm, TRUE)) {
		    /*
		     * It seems we could also just catch this when
		     * it attempted to hatch, so we wouldn't have to
		     * search all of the objlists.. or stop all
		     * hatch timers based on a corpsenm.
		     */
		    kill_egg(otmp);
		}
#if 0	/* not used */
	    } else if (otmp->otyp == TIN) {
		if (dead_species(otmp->corpsenm, FALSE))
		    otmp->corpsenm = NON_PM;	/* empty tin */
	    } else if (otmp->otyp == CORPSE) {
		if (dead_species(otmp->corpsenm, FALSE))
		    ;		/* not yet implemented... */
#endif
	    } else if (Has_contents(otmp)) {
		kill_eggs(otmp->cobj);
	    }
}

/* kill all members of genocided species */
void
kill_genocided_monsters()
{
	struct monst *mtmp, *mtmp2;
	boolean kill_cham[CHAM_MAX_INDX+1];
	int mndx;

	kill_cham[CHAM_ORDINARY] = FALSE;	/* (this is mndx==0) */
	for (mndx = 1; mndx <= CHAM_MAX_INDX; mndx++)
	  kill_cham[mndx] = (mvitals[cham_to_pm[mndx]].mvflags & G_GENOD && !In_quest(&u.uz)) != 0;
	/*
	 * Called during genocide, and again upon level change.  The latter
	 * catches up with any migrating monsters as they finally arrive at
	 * their intended destinations, so possessions get deposited there.
	 *
	 * Chameleon handling:
	 *	1) if chameleons have been genocided, destroy them
	 *	   regardless of current form;
	 *	2) otherwise, force every chameleon which is imitating
	 *	   any genocided species to take on a new form.
	 */
	for (mtmp = fmon; mtmp; mtmp = mtmp2) {
	    mtmp2 = mtmp->nmon;
	    if (DEADMONSTER(mtmp)) continue;
	    mndx = monsndx(mtmp->data);
	    if ((mvitals[mndx].mvflags & G_GENOD && !In_quest(&u.uz)) || kill_cham[mtmp->cham]) {
		if (mtmp->cham && !kill_cham[mtmp->cham])
		    (void) newcham(mtmp, (struct permonst *)0, FALSE, FALSE);
		else
		    mondead(mtmp);
	    }
	    if (mtmp->minvent) kill_eggs(mtmp->minvent);
	}

	kill_eggs(invent);
	kill_eggs(fobj);
	kill_eggs(level.buriedobjlist);
}

#endif /* OVL2 */
#ifdef OVLB

void
golemeffects(mon, damtype, dam)
register struct monst *mon;
int damtype, dam;
{
    int heal = 0, slow = 0;

    if (mon->data == &mons[PM_FLESH_GOLEM]) {
	if (damtype == AD_ELEC) heal = dam / 6;
	else if (damtype == AD_FIRE || damtype == AD_COLD) slow = 1;
    } else if (mon->data == &mons[PM_IRON_GOLEM] || mon->data == &mons[PM_CHAIN_GOLEM] || mon->data == &mons[PM_ARGENTUM_GOLEM] || mon->data == &mons[PM_CENTER_OF_ALL]) {
	if (damtype == AD_ELEC) slow = 1;
	else if (damtype == AD_FIRE) heal = dam;
    } else {
	return;
    }
    if (slow) {
	if (mon->mspeed != MSLOW)
	    mon_adjust_speed(mon, -1, (struct obj *)0);
    }
    if (heal) {
	if (mon->mhp < mon->mhpmax) {
	    mon->mhp += dam;
	    if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
	    if (cansee(mon->mx, mon->my))
		pline("%s seems healthier.", Monnam(mon));
	}
    }
}

boolean
angry_guards(silent)
register boolean silent;
{
	register struct monst *mtmp;
	register int ct = 0, nct = 0, sct = 0, slct = 0;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		if((mtmp->data == &mons[PM_WATCHMAN] ||
			       mtmp->data == &mons[PM_WATCH_CAPTAIN])
					&& mtmp->mpeaceful && !mtmp->mtame) {
			ct++;
			if(cansee(mtmp->mx, mtmp->my) && mtmp->mcanmove) {
				if (distu(mtmp->mx, mtmp->my) == 2) nct++;
				else sct++;
			}
			if (mtmp->msleeping || mtmp->mfrozen) {
				slct++;
				mtmp->msleeping = mtmp->mfrozen = 0;
			}
			mtmp->mpeaceful = 0;
		}
	}
	if(ct) {
	    if(!silent) { /* do we want pline msgs? */
		if(slct) pline_The("guard%s wake%s up!",
				 slct > 1 ? "s" : "", slct == 1 ? "s" : "");
		if(nct || sct) {
			if(nct) pline_The("guard%s get%s angry!",
				nct == 1 ? "" : "s", nct == 1 ? "s" : "");
			else if(!Blind)
				You("see %sangry guard%s approaching!",
				  sct == 1 ? "an " : "", sct > 1 ? "s" : "");
		} else if(flags.soundok)
			You_hear("the shrill sound of a guard's whistle.");
	    }
	    return(TRUE);
	}
	return(FALSE);
}

void
pacify_guards()
{
	register struct monst *mtmp;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if (mtmp->data == &mons[PM_WATCHMAN] ||
		mtmp->data == &mons[PM_WATCH_CAPTAIN])
	    mtmp->mpeaceful = 1;
	}
}

void
mimic_hit_msg(mtmp, otyp)
struct monst *mtmp;
short otyp;
{
	short ap = mtmp->mappearance;

	switch(mtmp->m_ap_type) {
	    case M_AP_NOTHING:			
	    case M_AP_FURNITURE:
	    case M_AP_MONSTER:
		break;
	    case M_AP_OBJECT:
		if (otyp == SPE_HEALING || otyp == SPE_EXTRA_HEALING) {
		    pline("%s seems a more vivid %s than before.",
				The(simple_typename(ap)),
				c_obj_colors[objects[ap].oc_color]);
		}
		break;
	}
}
#endif /* OVLB */

/*mon.c*/
