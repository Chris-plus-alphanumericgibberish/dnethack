/*	SCCS Id: @(#)mondata.c	3.4	2003/06/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "eshk.h"
#include "epri.h"

/*	These routines provide basic data for any type of monster. */

#ifdef OVLB

void
set_mon_data(mon, ptr, flag)
struct monst *mon;
struct permonst *ptr;
int flag;
{
    mon->data = ptr;
    if (flag == -1) return;		/* "don't care" */

    if (flag == 1)
		mon->mintrinsics |= (ptr->mresists & 0x03FF);
    else
		mon->mintrinsics = (ptr->mresists & 0x03FF);
	if(is_half_dragon(ptr) && (mon->mvar1 == 0 || flag != 1)){
		/*
			Store half dragon breath type in mvar1
		*/
		if(is_half_dragon(ptr)){
			switch(rnd(6)){
				case 1:
					mon->mvar1 = AD_COLD;
					mon->mintrinsics |= MR_COLD;
				break;
				case 2:
					mon->mvar1 = AD_FIRE;
					mon->mintrinsics |= MR_FIRE;
				break;
				case 3:
					mon->mvar1 = AD_SLEE;
					mon->mintrinsics |= MR_SLEEP;
				break;
				case 4:
					mon->mvar1 = AD_ELEC;
					mon->mintrinsics |= MR_ELEC;
				break;
				case 5:
					mon->mvar1 = AD_DRST;
					mon->mintrinsics |= MR_POISON;
				break;
				case 6:
					mon->mvar1 = AD_ACID;
					mon->mintrinsics |= MR_ACID;
				break;
			}
		}
	}
    return;
}

#endif /* OVLB */
#ifdef OVL0

struct attack *
attacktype_fordmg(ptr, atyp, dtyp)
struct permonst *ptr;
int atyp, dtyp;
{
    struct attack *a;

    for (a = &ptr->mattk[0]; a < &ptr->mattk[NATTK]; a++){
		if (a->aatyp == atyp && ( 
								  (dtyp == AD_ANY && atyp != AT_GAZE)
								 /*Some gazes are passive, not attacks. 
									It was leading monsters like weeping angels to try to stand off with their useless gazes*/
								||(dtyp == AD_ANY && a->adtyp != AD_BLNK && a->adtyp != AD_WISD && a->adtyp != AD_STON )
								|| a->adtyp == dtyp)
		) return a;
	}

    return (struct attack *)0;
}

boolean
attacktype(ptr, atyp)
struct permonst *ptr;
int atyp;
{
    return attacktype_fordmg(ptr, atyp, AD_ANY) ? TRUE : FALSE;
}

int
attackindex(ptr, atyp, dtyp)
struct permonst *ptr;
int atyp, dtyp;
{
	int i;
    for (i = 0; i < NATTK; i++)
	if ((&ptr->mattk[i])->aatyp == atyp && (
		dtyp == AD_ANY || (&ptr->mattk[i])->adtyp == dtyp))
	    return i;
    return -1;
}

#endif /* OVL0 */
#ifdef OVLB

boolean
poly_when_stoned(ptr)
    struct permonst *ptr;
{
    return((boolean)(is_golem(ptr) && ptr != &mons[PM_STONE_GOLEM] &&
	    !(mvitals[PM_STONE_GOLEM].mvflags & G_GENOD && !In_quest(&u.uz))));
	    /* allow G_EXTINCT */
}

boolean
resists_oona(mon)
	struct monst *mon;
{
	switch(u.oonaenergy){
		case AD_ELEC: return resists_elec(mon);
		case AD_FIRE: return resists_fire(mon);
		case AD_COLD: return resists_cold(mon);
		default: return FALSE;
	}
}

boolean
resists_fire(mon)
	struct monst *mon;
{
	return (((mon)->mintrinsics & MR_FIRE) != 0 || (mon == u.usteed && u.sealsActive&SEAL_BERITH && Fire_resistance));
}

boolean
resists_cold(mon)
	struct monst *mon;
{
	return (((mon)->mintrinsics & MR_COLD) != 0 || (mon == u.usteed && u.sealsActive&SEAL_BERITH && Cold_resistance));
}

boolean
resists_sleep(mon)
	struct monst *mon;
{
	return (((mon)->mintrinsics & MR_SLEEP) != 0 || ((mon) == u.usteed && u.sealsActive&SEAL_BERITH && Sleep_resistance) || (mon)->cham == CHAM_DREAM);
}

boolean
resists_disint(mon)
	struct monst *mon;
{
	return (((mon)->mintrinsics & MR_DISINT) != 0 || (mon == u.usteed && u.sealsActive&SEAL_BERITH && Disint_resistance));
}

boolean
resists_elec(mon)
	struct monst *mon;
{
	return (((mon)->mintrinsics & MR_ELEC) != 0 || (mon == u.usteed && u.sealsActive&SEAL_BERITH && Shock_resistance));
}

boolean
resists_poison(mon)
	struct monst *mon;
{
	return (((mon)->mintrinsics & MR_POISON) != 0 || (mon == u.usteed && u.sealsActive&SEAL_BERITH && Poison_resistance));
}

boolean
resists_acid(mon)
	struct monst *mon;
{
	return (((mon)->mintrinsics & MR_ACID) != 0 || (mon == u.usteed && u.sealsActive&SEAL_BERITH && Acid_resistance));
}

boolean
resists_ston(mon)
	struct monst *mon;
{
	return (((mon)->mintrinsics & MR_STONE) != 0 || (mon == u.usteed && u.sealsActive&SEAL_BERITH && Stone_resistance));
}

boolean
resists_drain(mon)
	struct monst *mon;
{
	return (((mon)->mintrinsics & MR_DRAIN) != 0 || (mon == u.usteed && u.sealsActive&SEAL_BERITH && Drain_resistance));
}

boolean
resists_sickness(mon)
	struct monst *mon;
{
	return (((mon)->mintrinsics & MR_SICK) != 0 || (mon == u.usteed && u.sealsActive&SEAL_BERITH && Sick_resistance));
}

boolean
resists_drli(mon)	/* returns TRUE if monster is drain-life resistant */
struct monst *mon;
{
	struct permonst *ptr = mon->data;
	struct obj *wep = ((mon == &youmonst) ? uwep : MON_WEP(mon));

	return (boolean)(is_undead(ptr) || is_demon(ptr) || is_were(ptr) ||
			 ptr == &mons[PM_DEATH] ||
			 mon->mintrinsics & MR_DRAIN ||
			 (wep && wep->oartifact && defends(AD_DRLI, wep))  || 
			 (mon == u.usteed && u.sealsActive&SEAL_BERITH && Drain_resistance));
}

boolean
resists_magm(mon)	/* TRUE if monster is magic-missile resistant */
struct monst *mon;
{
	struct permonst *ptr = mon->data;
	struct obj *o;
	
	if(mon == u.usteed && u.sealsActive&SEAL_BERITH && Antimagic) return TRUE;
	
	if(mon->data == &mons[PM_WATCHER_IN_THE_WATER] ||
		mon->data == &mons[PM_SWARM_OF_SNAKING_TENTACLES] || 
		mon->data == &mons[PM_LONG_SINUOUS_TENTACLE] ||
		mon->data == &mons[PM_KETO] ||
		mon->data == &mons[PM_WIDE_CLUBBED_TENTACLE] ||
		mon->data == &mons[PM_DARUTH_XAXOX]
	) return TRUE;
	
	/* as of 3.2.0:  gray dragons, Angels, Oracle, Yeenoghu */
	if (dmgtype(ptr, AD_MAGM) || ptr == &mons[PM_BABY_GRAY_DRAGON] ||
		(dmgtype(ptr, AD_RBRE) && ptr != &mons[PM_SHIMMERING_DRAGON]))	/* Chromatic Dragon, Platinum Dragon, mortai, flux slime, tulani */
	    return TRUE;
	/* check for magic resistance granted by wielded weapon */
	o = (mon == &youmonst) ? uwep : MON_WEP(mon);
	if (o && o->oartifact && defends(AD_MAGM, o))
	    return TRUE;
	/* check for magic resistance granted by worn or carried items */
	o = (mon == &youmonst) ? invent : mon->minvent;
	for ( ; o; o = o->nobj)
	    if ((o->owornmask && objects[o->otyp].oc_oprop == ANTIMAGIC) ||
		    (o->oartifact && protects(AD_MAGM, o)))
		return TRUE;
	return FALSE;
}

boolean
resists_death(mon)	/* TRUE if monster resists death magic */
struct monst *mon;
{
	struct permonst *ptr = mon->data;
	struct obj *o;
	
	if(mon == u.usteed && u.sealsActive&SEAL_BERITH && u.sealsActive&SEAL_OSE) return TRUE;
	
	return nonliving(mon->data) || is_demon(mon->data) || is_angel(mon->data) || is_keter(mon->data);
}

/* TRUE iff monster is resistant to light-induced blindness */
boolean
resists_blnd(mon)
struct monst *mon;
{
	struct permonst *ptr = mon->data;
	boolean is_you = (mon == &youmonst);
	struct obj *o;

	if (is_you ? (NoLightBlind || u.usleep) :
		(mon->mblinded || !mon->mcansee || !haseyes(ptr) ||
		    /* BUG: temporary sleep sets mfrozen, but since
			    paralysis does too, we can't check it */
		    mon->msleeping))
	    return TRUE;
	/* yellow light, Archon; !dust vortex, !cobra, !raven */
	if (dmgtype_fromattack(ptr, AD_BLND, AT_EXPL) ||
		dmgtype_fromattack(ptr, AD_BLND, AT_GAZE))
	    return TRUE;
	o = is_you ? uwep : MON_WEP(mon);
	if (o && o->oartifact && defends(AD_BLND, o))
	    return TRUE;
	o = is_you ? invent : mon->minvent;
	for ( ; o; o = o->nobj)
	    if ((o->owornmask && objects[o->otyp].oc_oprop == BLINDED) ||
		    (o->oartifact && protects(AD_BLND, o)))
		return TRUE;
	return FALSE;
}

/* TRUE iff monster can be blinded by the given attack */
/* Note: may return TRUE when mdef is blind (e.g. new cream-pie attack) */
boolean
can_blnd(magr, mdef, aatyp, obj)
struct monst *magr;		/* NULL == no specific aggressor */
struct monst *mdef;
uchar aatyp;
struct obj *obj;		/* aatyp == AT_WEAP, AT_SPIT */
{
	boolean is_you = (mdef == &youmonst);
	boolean check_visor = FALSE;
	struct obj *o;
	const char *s;

	/* no eyes protect against all attacks for now */
	if (!haseyes(mdef->data))
	    return FALSE;

	switch(aatyp) {
	case AT_EXPL: case AT_BOOM: case AT_GAZE: case AT_MAGC: case AT_MMGC:
	case AT_BREA: /* assumed to be lightning */
	    /* light-based attacks may be cancelled or resisted */
	    if (magr && magr->mcan)
		return FALSE;
	    return !resists_blnd(mdef);

	case AT_WEAP: case AT_SPIT: case AT_NONE:
	    /* an object is used (thrown/spit/other) */
	    if (obj && (obj->otyp == CREAM_PIE)) {
		if (is_you && Blindfolded)
		    return FALSE;
	    } else if (obj && (obj->otyp == BLINDING_VENOM)) {
		/* all ublindf, including LENSES, protect, cream-pies too */
		if (is_you && (ublindf || u.ucreamed))
		    return FALSE;
		check_visor = TRUE;
	    } else if (obj && (obj->otyp == POT_BLINDNESS)) {
		return TRUE;	/* no defense */
	    } else
		return FALSE;	/* other objects cannot cause blindness yet */
	    if ((magr == &youmonst) && u.uswallow)
		return FALSE;	/* can't affect eyes while inside monster */
	    break;

	case AT_ENGL:
	    if (is_you && (Blindfolded || u.usleep || u.ucreamed))
		return FALSE;
	    if (!is_you && mdef->msleeping)
		return FALSE;
	    break;

	case AT_CLAW:
	    /* e.g. raven: all ublindf, including LENSES, protect */
	    if (is_you && ublindf)
		return FALSE;
	    if ((magr == &youmonst) && u.uswallow)
		return FALSE;	/* can't affect eyes while inside monster */
	    check_visor = TRUE;
	    break;

	case AT_TUCH: case AT_STNG:
	    /* some physical, blind-inducing attacks can be cancelled */
	    if (magr && magr->mcan)
		return FALSE;
	    break;

	default:
	    break;
	}

	/* check if wearing a visor (only checked if visor might help) */
	if (check_visor) {
		static int vhelmsa = 0;
		if (!vhelmsa) vhelmsa = find_vhelm();
	    o = (mdef == &youmonst) ? invent : mdef->minvent;
	    for ( ; o; o = o->nobj)
		if ((o->owornmask & W_ARMH) &&
			(o->otyp == vhelmsa || o->otyp == CRYSTAL_HELM)
		) return FALSE;
	}
	return TRUE;
}

#endif /* OVLB */
#ifdef OVL0

boolean
ranged_attk(ptr)	/* returns TRUE if monster can attack at range */
struct permonst *ptr;
{
	register int i, atyp;
	long atk_mask = (1L << AT_BREA) | (1L << AT_SPIT) | (1L << AT_GAZE) | (1L << AT_LRCH) | (1L << AT_LNCK)
					| (1L << AT_MMGC) | (1L << AT_TNKR) | (1L << AT_ARRW) | (1L << AT_BEAM);

	/* was: (attacktype(ptr, AT_BREA) || attacktype(ptr, AT_WEAP) ||
		attacktype(ptr, AT_SPIT) || attacktype(ptr, AT_GAZE) ||
		attacktype(ptr, AT_MAGC));
	   but that's too slow -dlc
	 */
	for (i = 0; i < NATTK; i++) {
	    atyp = ptr->mattk[i].aatyp;
	    if (atyp >= AT_WEAP) return TRUE;
	 /* assert(atyp < 32); */
	    if ((atk_mask & (1L << atyp)) != 0L) return TRUE;
	}

	return FALSE;
}

/* true iff the type of monster pass through iron bars */
boolean
passes_bars(mptr)
struct permonst *mptr;
{
    return (boolean) (passes_walls(mptr) || amorphous(mptr) ||
		      is_whirly(mptr) || verysmall(mptr) ||
			  dmgtype(mptr, AD_CORR) || (dmgtype(mptr, AD_RUST) && mptr != &mons[PM_NAIAD] ) ||
		      (slithy(mptr) && !bigmonst(mptr)));
}

#endif /* OVL0 */
#ifdef OVL1

boolean
can_track(ptr)		/* returns TRUE if monster can track well */
	register struct permonst *ptr;
{
	if (uwep && (
		uwep->oartifact == ART_EXCALIBUR
		|| uwep->oartifact == ART_SLAVE_TO_ARMOK
		) ) return TRUE;
	else
		return((boolean)is_tracker(ptr));
}

#endif /* OVL1 */
#ifdef OVLB

#endif /* OVLB */
#ifdef OVL1

boolean
sticks(ptr)	/* creature sticks other creatures it hits */
	register struct permonst *ptr;
{
	return((boolean)(dmgtype(ptr,AD_STCK) || dmgtype(ptr,AD_WRAP) ||
		attacktype(ptr,AT_HUGS)));
}

/* number of horns this type of monster has on its head */
int
num_horns(ptr)
struct permonst *ptr;
{
    switch (monsndx(ptr)) {
    case PM_HORNED_DEVIL:	/* ? "more than one" */
    case PM_MINOTAUR:
    case PM_ASMODEUS:
    case PM_BALROG:
	return 2;
    case PM_WHITE_UNICORN:
    case PM_GRAY_UNICORN:
    case PM_BLACK_UNICORN:
    case PM_KI_RIN:
	return 1;
    default:
	break;
    }
    return 0;
}

struct attack *
dmgtype_fromattack(ptr, dtyp, atyp)
struct permonst *ptr;
int dtyp, atyp;
{
    struct attack *a;

    for (a = &ptr->mattk[0]; a < &ptr->mattk[NATTK]; a++)
	if (a->adtyp == dtyp && (atyp == AT_ANY || a->aatyp == atyp))
	    return a;

    return (struct attack *)0;
}

boolean
dmgtype(ptr, dtyp)
struct permonst *ptr;
int dtyp;
{
    return dmgtype_fromattack(ptr, dtyp, AT_ANY) ? TRUE : FALSE;
}

/* returns the maximum damage a defender can do to the attacker via
 * a passive defense */
int
max_passive_dmg(mdef, magr)
    register struct monst *mdef, *magr;
{
    int	i, dmg = 0;
    uchar adtyp;

    for(i = 0; i < NATTK; i++)
	if(mdef->data->mattk[i].aatyp == AT_NONE ||
		mdef->data->mattk[i].aatyp == AT_BOOM) {
	    adtyp = mdef->data->mattk[i].adtyp;
	    if ((adtyp == AD_ACID && !resists_acid(magr)) ||
		    (adtyp == AD_COLD && !resists_cold(magr)) ||
		    (adtyp == AD_FIRE && !resists_fire(magr)) ||
		    (adtyp == AD_ELEC && !resists_elec(magr)) ||
		    adtyp == AD_PHYS) {
		dmg = mdef->data->mattk[i].damn;
		if(!dmg) dmg = mdef->data->mlevel+1;
		dmg *= mdef->data->mattk[i].damd;
	    } else dmg = 0;

	    return dmg;
	}
    return 0;
}

#endif /* OVL1 */
#ifdef OVL0

int
monsndx(ptr)		/* return an index into the mons array */
	struct	permonst	*ptr;
{
	register int	i;

	if (ptr == &upermonst) return PM_PLAYERMON;

	i = (int)(ptr - &mons[0]);
	if (i < LOW_PM || i >= NUMMONS) {
		/* ought to switch this to use `fmt_ptr' */
	    panic("monsndx - could not index monster (%lx)",
		  (unsigned long)ptr);
	    return NON_PM;		/* will not get here */
	}

	return(i);
}

#endif /* OVL0 */
#ifdef OVL1


int
name_to_mon(in_str)
const char *in_str;
{
	/* Be careful.  We must check the entire string in case it was
	 * something such as "ettin zombie corpse".  The calling routine
	 * doesn't know about the "corpse" until the monster name has
	 * already been taken off the front, so we have to be able to
	 * read the name with extraneous stuff such as "corpse" stuck on
	 * the end.
	 * This causes a problem for names which prefix other names such
	 * as "ettin" on "ettin zombie".  In this case we want the _longest_
	 * name which exists.
	 * This also permits plurals created by adding suffixes such as 's'
	 * or 'es'.  Other plurals must still be handled explicitly.
	 */
	register int i;
	register int mntmp = NON_PM;
	register char *s, *str, *term;
	char buf[BUFSZ];
	int len, slen;

	str = strcpy(buf, in_str);

	if (!strncmp(str, "a ", 2)) str += 2;
	else if (!strncmp(str, "an ", 3)) str += 3;

	slen = strlen(str);
	term = str + slen;

	if ((s = strstri(str, "vortices")) != 0)
	    Strcpy(s+4, "ex");
	/* be careful with "ies"; "priest", "zombies" */
	else if (slen > 3 && !strcmpi(term-3, "ies") &&
		    (slen < 7 || strcmpi(term-7, "zombies")))
	    Strcpy(term-3, "y");
	/* luckily no monster names end in fe or ve with ves plurals */
	else if (slen > 3 && !strcmpi(term-3, "ves"))
	    Strcpy(term-3, "f");

	slen = strlen(str); /* length possibly needs recomputing */

    {
	static const struct alt_spl { const char* name; short pm_val; }
	    names[] = {
	    /* Alternate spellings */
		{ "grey dragon",	PM_GRAY_DRAGON },
		{ "baby grey dragon",	PM_BABY_GRAY_DRAGON },
		{ "grey unicorn",	PM_GRAY_UNICORN },
		{ "grey ooze",		PM_GRAY_OOZE },
		{ "gray-elf",		PM_GREY_ELF },
	    /* Hyphenated names */
		{ "ki rin",		PM_KI_RIN },
		{ "uruk hai",		PM_URUK_HAI },
		{ "orc captain",	PM_ORC_CAPTAIN },
		{ "woodland elf",	PM_WOODLAND_ELF },
		{ "green elf",		PM_GREEN_ELF },
		{ "grey elf",		PM_GREY_ELF },
		{ "gray elf",		PM_GREY_ELF },
		{ "elf lord",		PM_ELF_LORD },
#if 0	/* OBSOLETE */
		{ "high elf",		PM_HIGH_ELF },
#endif
		{ "olog hai",		PM_OLOG_HAI },
		{ "arch lich",		PM_ARCH_LICH },
	    /* Some irregular plurals */
		{ "incubi",		PM_INCUBUS },
		{ "succubi",		PM_SUCCUBUS },
		{ "violet fungi",	PM_VIOLET_FUNGUS },
		{ "homunculi",		PM_HOMUNCULUS },
		{ "baluchitheria",	PM_BALUCHITHERIUM },
		{ "lurkers above",	PM_LURKER_ABOVE },
		{ "cavemen",		PM_CAVEMAN },
		{ "cavewomen",		PM_CAVEWOMAN },
		{ "djinn",		PM_DJINNI },
		{ "mumakil",		PM_MUMAK },
		{ "erinyes",		PM_ERINYS },
	    /* falsely caught by -ves check above */
		{ "master of thief",	PM_MASTER_OF_THIEVES },
	    /* human-form weres */
		{ "wererat (human)",	PM_HUMAN_WERERAT },
		{ "werejackal (human)",	PM_HUMAN_WEREJACKAL },
		{ "werewolf (human)",	PM_HUMAN_WEREWOLF },
	    /* end of list */
		{ 0, 0 }
	};
	register const struct alt_spl *namep;

	for (namep = names; namep->name; namep++)
	    if (!strncmpi(str, namep->name, (int)strlen(namep->name)))
		return namep->pm_val;
    }

	for (len = 0, i = LOW_PM; i < NUMMONS; i++) {
	    register int m_i_len = strlen(mons[i].mname);
	    if (m_i_len > len && !strncmpi(mons[i].mname, str, m_i_len)) {
		if (m_i_len == slen) return i;	/* exact match */
		else if (slen > m_i_len &&
			(str[m_i_len] == ' ' ||
			 !strcmpi(&str[m_i_len], "s") ||
			 !strncmpi(&str[m_i_len], "s ", 2) ||
			 !strcmpi(&str[m_i_len], "'") ||
			 !strncmpi(&str[m_i_len], "' ", 2) ||
			 !strcmpi(&str[m_i_len], "'s") ||
			 !strncmpi(&str[m_i_len], "'s ", 3) ||
			 !strcmpi(&str[m_i_len], "es") ||
			 !strncmpi(&str[m_i_len], "es ", 3))) {
		    mntmp = i;
		    len = m_i_len;
		}
	    }
	}
	if (mntmp == NON_PM) mntmp = title_to_mon(str, (int *)0, (int *)0);
	return mntmp;
}

#endif /* OVL1 */
#ifdef OVL2

/* returns 3 values (0=male, 1=female, 2=none) */
int
gender(mtmp)
register struct monst *mtmp;
{
	if (is_neuter(mtmp->data)) return 2;
	return mtmp->female;
}

/* Like gender(), but lower animals and such are still "it". */
/* This is the one we want to use when printing messages. */
int
pronoun_gender(mtmp)
register struct monst *mtmp;
{
	if (is_neuter(mtmp->data) || !canspotmon(mtmp)) return 2;
	return (humanoid(mtmp->data) || (mtmp->data->geno & G_UNIQ) ||
		type_is_pname(mtmp->data)) ? (int)mtmp->female : 2;
}

#endif /* OVL2 */
#ifdef OVLB

/* used for nearby monsters when you go to another level */
boolean
levl_follower(mtmp)
struct monst *mtmp;
{
	/* monsters with the Amulet--even pets--won't follow across levels */
	if (mon_has_amulet(mtmp)) return FALSE;

	/* some monsters will follow even while intending to flee from you */
	if (mtmp->mtame || mtmp->iswiz || is_fshk(mtmp)) return TRUE;

	/* stalking types follow, but won't when fleeing unless you hold
	   the Amulet */
	return (boolean)((mtmp->data->mflagst & MT_STALK) &&
				(!mtmp->mflee || mtmp->data == &mons[PM_BANDERSNATCH] || u.uhave.amulet));
}

static const short grownups[][2] = {
	{PM_CHICKATRICE, PM_COCKATRICE},
	{PM_LITTLE_DOG, PM_DOG}, {PM_DOG, PM_LARGE_DOG},
	{PM_HELL_HOUND_PUP, PM_HELL_HOUND},
	{PM_WINTER_WOLF_CUB, PM_WINTER_WOLF},
	{PM_KITTEN, PM_HOUSECAT}, {PM_HOUSECAT, PM_LARGE_CAT},
	{PM_TINY_PSEUDODRAGON, PM_PSEUDODRAGON}, {PM_PSEUDODRAGON, PM_RIDING_PSEUDODRAGON}, {PM_RIDING_PSEUDODRAGON, PM_LARGE_PSEUDODRAGON}, 
		{PM_LARGE_PSEUDODRAGON, PM_WINGED_PSEUDODRAGON}, {PM_WINGED_PSEUDODRAGON, PM_HUGE_PSEUDODRAGON}, {PM_HUGE_PSEUDODRAGON, PM_GIGANTIC_PSEUDODRAGON},
	{PM_PONY, PM_HORSE}, {PM_HORSE, PM_WARHORSE},
	{PM_KOBOLD, PM_LARGE_KOBOLD}, {PM_LARGE_KOBOLD, PM_KOBOLD_LORD},
	{PM_GNOME, PM_GNOME_LORD}, {PM_GNOME_LORD, PM_GNOME_KING},
	{PM_GNOME, PM_GNOME_LADY}, {PM_GNOME_LADY, PM_GNOME_QUEEN},
	{PM_DWARF, PM_DWARF_LORD}, {PM_DWARF_LORD, PM_DWARF_KING},
	{PM_DWARF, PM_DWARF_CLERIC}, {PM_DWARF_CLERIC, PM_DWARF_QUEEN},
	{PM_MIND_FLAYER, PM_MASTER_MIND_FLAYER},
	{PM_DEEP_ONE, PM_DEEPER_ONE}, {PM_DEEPER_ONE, PM_DEEPEST_ONE},
	{PM_ORC, PM_ORC_CAPTAIN}, {PM_HILL_ORC, PM_ORC_CAPTAIN},
	{PM_MORDOR_ORC, PM_ORC_CAPTAIN}, {PM_URUK_HAI, PM_ORC_CAPTAIN},
	{PM_SEWER_RAT, PM_GIANT_RAT},
#ifdef CONVICT
	{PM_GIANT_RAT, PM_ENORMOUS_RAT},
	{PM_ENORMOUS_RAT, PM_RODENT_OF_UNUSUAL_SIZE},
#endif	/* CONVICT */
	{PM_CAVE_SPIDER, PM_GIANT_SPIDER}, {PM_GIANT_SPIDER, PM_MIRKWOOD_SPIDER}, {PM_MIRKWOOD_SPIDER, PM_MIRKWOOD_ELDER},
	{PM_OGRE, PM_OGRE_LORD}, {PM_OGRE_LORD, PM_OGRE_KING},
	{PM_ELF, PM_ELF_LORD}, {PM_WOODLAND_ELF, PM_ELF_LORD},
	{PM_GREEN_ELF, PM_ELF_LORD}, {PM_GREY_ELF, PM_ELF_LORD},
	{PM_ELF_LORD, PM_ELVENKING},
	{PM_ELF, PM_ELF_LADY}, {PM_WOODLAND_ELF, PM_ELF_LADY},
	{PM_GREEN_ELF, PM_ELF_LADY}, {PM_GREY_ELF, PM_ELF_LADY},
	{PM_ELF_LADY, PM_ELVENQUEEN},
	{PM_LICH, PM_DEMILICH}, {PM_DEMILICH, PM_MASTER_LICH},
	{PM_MASTER_LICH, PM_ARCH_LICH},
	{PM_BABY_METROID, PM_METROID},{PM_METROID, PM_ALPHA_METROID}, {PM_ALPHA_METROID, PM_GAMMA_METROID},
	{PM_GAMMA_METROID, PM_ZETA_METROID}, {PM_ZETA_METROID, PM_OMEGA_METROID}, 
	{PM_OMEGA_METROID, PM_METROID_QUEEN},
	{PM_VAMPIRE, PM_VAMPIRE_LORD}, {PM_BAT, PM_GIANT_BAT},
	{PM_BABY_GRAY_DRAGON, PM_GRAY_DRAGON},
	{PM_BABY_SILVER_DRAGON, PM_SILVER_DRAGON},
	{PM_BABY_MERCURIAL_DRAGON, PM_MERCURIAL_DRAGON},
	{PM_BABY_DEEP_DRAGON, PM_DEEP_DRAGON},
	{PM_BABY_SHIMMERING_DRAGON, PM_SHIMMERING_DRAGON},
	{PM_BABY_RED_DRAGON, PM_RED_DRAGON},
	{PM_BABY_WHITE_DRAGON, PM_WHITE_DRAGON},
	{PM_BABY_ORANGE_DRAGON, PM_ORANGE_DRAGON},
	{PM_BABY_BLACK_DRAGON, PM_BLACK_DRAGON},
	{PM_BABY_BLUE_DRAGON, PM_BLUE_DRAGON},
	{PM_BABY_GREEN_DRAGON, PM_GREEN_DRAGON},
	{PM_BABY_YELLOW_DRAGON, PM_YELLOW_DRAGON},
	{PM_RED_NAGA_HATCHLING, PM_RED_NAGA},
	{PM_BLACK_NAGA_HATCHLING, PM_BLACK_NAGA},
	{PM_GOLDEN_NAGA_HATCHLING, PM_GOLDEN_NAGA},
	{PM_GUARDIAN_NAGA_HATCHLING, PM_GUARDIAN_NAGA},
	{PM_SMALL_MIMIC, PM_LARGE_MIMIC}, {PM_LARGE_MIMIC, PM_GIANT_MIMIC},
	{PM_BABY_LONG_WORM, PM_LONG_WORM},
	{PM_BABY_PURPLE_WORM, PM_PURPLE_WORM},
	{PM_BABY_CROCODILE, PM_CROCODILE},
	{PM_BABY_CAVE_LIZARD,PM_SMALL_CAVE_LIZARD}, {PM_SMALL_CAVE_LIZARD, PM_CAVE_LIZARD}, {PM_CAVE_LIZARD, PM_LARGE_CAVE_LIZARD},
	{PM_SOLDIER, PM_SERGEANT},
	{PM_SERGEANT, PM_LIEUTENANT},
	{PM_LIEUTENANT, PM_CAPTAIN},
	{PM_WATCHMAN, PM_WATCH_CAPTAIN},
	{PM_ALIGNED_PRIEST, PM_HIGH_PRIEST},
	{PM_STUDENT, PM_ARCHEOLOGIST},
	{PM_RHYMER, PM_BARD},
	{PM_HEDROW_WIZARD, PM_HEDROW_MASTER_WIZARD},
	{PM_ATTENDANT, PM_HEALER},
	{PM_PAGE, PM_KNIGHT},
	{PM_ACOLYTE, PM_PRIEST},
	{PM_ACOLYTE, PM_PRIESTESS},
	{PM_APPRENTICE, PM_WIZARD},
	{PM_DUNGEON_FERN_SPROUT, PM_DUNGEON_FERN},
	{PM_SWAMP_FERN_SPROUT, PM_SWAMP_FERN},
	{PM_BURNING_FERN_SPROUT, PM_BURNING_FERN},
	{NON_PM,NON_PM}

};

int
little_to_big(montype, female)
int montype;
boolean female;
{
#ifndef AIXPS2_BUG
	register int i;

	for (i = 0; grownups[i][0] >= LOW_PM; i++)
		if(montype == grownups[i][0] && (
			!((mons[grownups[i][1]].mflagsb)&(MB_FEMALE|MB_MALE)) ||
			(female && ((mons[grownups[i][1]].mflagsb)&(MB_FEMALE))) ||
			(!female && ((mons[grownups[i][1]].mflagsb)&(MB_MALE)))
		)) return grownups[i][1];
	return montype;
#else
/* AIX PS/2 C-compiler 1.1.1 optimizer does not like the above for loop,
 * and causes segmentation faults at runtime.  (The problem does not
 * occur if -O is not used.)
 * lehtonen@cs.Helsinki.FI (Tapio Lehtonen) 28031990
 */
	int i;
	int monvalue;

	monvalue = montype;
	for (i = 0; grownups[i][0] >= LOW_PM; i++)
		if(montype == grownups[i][0] && (
			!((mons[grownups[i][1]].mflagsb)&(MB_FEMALE|MB_MALE)) ||
			(female && ((mons[grownups[i][1]].mflagsb)&(MB_FEMALE))) ||
			(!female && ((mons[grownups[i][1]].mflagsb)&(MB_MALE)))
		)) monvalue = grownups[i][1];

	return monvalue;
#endif
}

int
big_to_little(montype)
int montype;
{
	register int i;

	for (i = 0; grownups[i][0] >= LOW_PM; i++)
		if(montype == grownups[i][1]) return big_to_little(grownups[i][0]);
	return montype;
}

/*
 * Return the permonst ptr for the race of the monster.
 * Returns correct pointer for non-polymorphed and polymorphed
 * player.  It does not return a pointer to player role character.
 */
const struct permonst *
raceptr(mtmp)
struct monst *mtmp;
{
    if (mtmp == &youmonst && !Upolyd) return(&mons[urace.malenum]);
    else return(mtmp->data);
}

static const char *levitate[4]	= { "float", "Float", "wobble", "Wobble" };
static const char *flys[4]	= { "fly", "Fly", "flutter", "Flutter" };
static const char *flyl[4]	= { "fly", "Fly", "stagger", "Stagger" };
static const char *slither[4]	= { "slither", "Slither", "falter", "Falter" };
static const char *ooze[4]	= { "ooze", "Ooze", "tremble", "Tremble" };
static const char *immobile[4]	= { "wiggle", "Wiggle", "pulsate", "Pulsate" };
static const char *crawl[4]	= { "crawl", "Crawl", "falter", "Falter" };

const char *
locomotion(ptr, def)
const struct permonst *ptr;
const char *def;
{
	int capitalize = (*def == highc(*def));

	return (
		is_floater(ptr) ? levitate[capitalize] :
		(is_flyer(ptr) && ptr->msize <= MZ_SMALL) ? flys[capitalize] :
		(is_flyer(ptr) && ptr->msize > MZ_SMALL)  ? flyl[capitalize] :
		slithy(ptr)     ? slither[capitalize] :
		amorphous(ptr)  ? ooze[capitalize] :
		!ptr->mmove	? immobile[capitalize] :
		nolimbs(ptr)    ? crawl[capitalize] :
		def
	       );

}

const char *
stagger(ptr, def)
const struct permonst *ptr;
const char *def;
{
	int capitalize = 2 + (*def == highc(*def));

	return (
		is_floater(ptr) ? levitate[capitalize] :
		(is_flyer(ptr) && ptr->msize <= MZ_SMALL) ? flys[capitalize] :
		(is_flyer(ptr) && ptr->msize > MZ_SMALL)  ? flyl[capitalize] :
		slithy(ptr)     ? slither[capitalize] :
		amorphous(ptr)  ? ooze[capitalize] :
		!ptr->mmove	? immobile[capitalize] :
		nolimbs(ptr)    ? crawl[capitalize] :
		def
	       );

}

/* return a phrase describing the effect of fire attack on a type of monster */
const char *
on_fire(mptr, mattk)
struct permonst *mptr;
struct attack *mattk;
{
    const char *what;

    switch (monsndx(mptr)) {
    case PM_FLAMING_SPHERE:
    case PM_FIRE_VORTEX:
    case PM_FIRE_ELEMENTAL:
    case PM_LIGHTNING_PARAELEMENTAL:
    case PM_SALAMANDER:
	what = "already on fire";
	break;
    case PM_WATER_ELEMENTAL:
    case PM_FOG_CLOUD:
    case PM_STEAM_VORTEX:
    case PM_ACID_PARAELEMENTAL:
	what = "boiling";
	break;
    case PM_ICE_VORTEX:
    case PM_ICE_PARAELEMENTAL:
    case PM_GLASS_GOLEM:
	what = "melting";
	break;
    case PM_STONE_GOLEM:
    case PM_CLAY_GOLEM:
    case PM_GOLD_GOLEM:
    case PM_AIR_ELEMENTAL:
    case PM_POISON_PARAELEMENTAL:
    case PM_EARTH_ELEMENTAL:
    case PM_DUST_VORTEX:
    case PM_ENERGY_VORTEX:
	what = "heating up";
	break;
    default:
	what = (mattk->aatyp == AT_HUGS) ? "being roasted" : "on fire";
	break;
    }
    return what;
}

#endif /* OVLB */

/*mondata.c*/
