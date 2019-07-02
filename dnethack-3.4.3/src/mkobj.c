/*	SCCS Id: @(#)mkobj.c	3.4	2002/10/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "prop.h"

STATIC_DCL void FDECL(mkbox_cnts,(struct obj *));
STATIC_DCL void FDECL(obj_timer_checks,(struct obj *, XCHAR_P, XCHAR_P, int));
#ifdef OVL1
STATIC_DCL void FDECL(container_weight, (struct obj *));
STATIC_DCL struct obj *FDECL(save_mtraits, (struct obj *, struct monst *));
#ifdef WIZARD
STATIC_DCL const char *FDECL(where_name, (int));
STATIC_DCL void FDECL(check_contained, (struct obj *,const char *));
STATIC_DCL int FDECL(maid_clean, (struct monst *, struct obj *));
#endif
#endif /* OVL1 */

extern struct obj *thrownobj;		/* defined in dothrow.c */

/*#define DEBUG_EFFECTS*/	/* show some messages for debugging */

struct icp {
    int  iprob;		/* probability of an item type */
    char iclass;	/* item class */
};

#ifdef OVL1

const struct icp mkobjprobs[] = {
{10, WEAPON_CLASS},
{10, ARMOR_CLASS},
{20, FOOD_CLASS},
{ 8, TOOL_CLASS},
{ 8, GEM_CLASS},
{16, POTION_CLASS},
{16, SCROLL_CLASS},
{ 4, SPBOOK_CLASS},
{ 4, WAND_CLASS},
{ 3, RING_CLASS},
{ 1, AMULET_CLASS}
};

const struct icp boxiprobs[] = {
{18, GEM_CLASS},
{15, FOOD_CLASS},
{18, POTION_CLASS},
{18, SCROLL_CLASS},
{12, SPBOOK_CLASS},
{ 7, COIN_CLASS},
{ 6, WAND_CLASS},
{ 5, RING_CLASS},
{ 1, AMULET_CLASS}
};

#ifdef REINCARNATION
const struct icp rogueprobs[] = {
{12, WEAPON_CLASS},
{12, ARMOR_CLASS},
{22, FOOD_CLASS},
{22, POTION_CLASS},
{22, SCROLL_CLASS},
{ 5, WAND_CLASS},
{ 5, RING_CLASS}
};
#endif

const struct icp hellprobs[] = {
{20, WEAPON_CLASS},
{20, ARMOR_CLASS},
{16, FOOD_CLASS},
{12, TOOL_CLASS},
{10, GEM_CLASS},
{ 1, POTION_CLASS},
{ 1, SCROLL_CLASS},
{ 8, WAND_CLASS},
{ 8, RING_CLASS},
{ 4, AMULET_CLASS}
};

struct obj *
mkobj_at(let, x, y, artif)
char let;
int x, y;
boolean artif;
{
	struct obj *otmp;

	otmp = mkobj(let, artif);
	
	if(In_quest(&u.uz) && !Role_if(PM_CONVICT) && in_mklev){
		if(otmp->oclass == WEAPON_CLASS || otmp->oclass == ARMOR_CLASS) otmp->objsize = (&mons[urace.malenum])->msize;
		if(otmp->oclass == ARMOR_CLASS){
			if(is_suit(otmp)) otmp->bodytypeflag = ((&mons[urace.malenum])->mflagsb&MB_BODYTYPEMASK);
			else if(is_helmet(otmp)) otmp->bodytypeflag = ((&mons[urace.malenum])->mflagsb&MB_HEADMODIMASK);
			else if(is_shirt(otmp)) otmp->bodytypeflag = ((&mons[urace.malenum])->mflagsb&MB_HUMANOID) ? MB_HUMANOID : ((&mons[urace.malenum])->mflagsb&MB_BODYTYPEMASK);
		}
	}
	
	place_object(otmp, x, y);
	
	return(otmp);
}

struct obj *
mksobj_at(otyp, x, y, init, artif)
int otyp, x, y;
boolean init, artif;
{
	struct obj *otmp;

	otmp = mksobj(otyp, init, artif);
	
	if(In_quest(&u.uz) && !Role_if(PM_CONVICT) && in_mklev){
		if(otmp->oclass == WEAPON_CLASS || otmp->oclass == ARMOR_CLASS) otmp->objsize = (&mons[urace.malenum])->msize;
		if(otmp->oclass == ARMOR_CLASS){
			if(is_suit(otmp)) otmp->bodytypeflag = ((&mons[urace.malenum])->mflagsb&MB_BODYTYPEMASK);
			else if(is_helmet(otmp)) otmp->bodytypeflag = ((&mons[urace.malenum])->mflagsb&MB_HEADMODIMASK);
			else if(is_shirt(otmp)) otmp->bodytypeflag = ((&mons[urace.malenum])->mflagsb&MB_HUMANOID) ? MB_HUMANOID : ((&mons[urace.malenum])->mflagsb&MB_BODYTYPEMASK);
		}
	}
	
	place_object(otmp, x, y);
	
	return(otmp);
}

struct obj *
mkobj(oclass, artif)
char oclass;
boolean artif;
{
	int tprob, i, prob = rnd(1000);

	if(oclass == RANDOM_CLASS) {
		const struct icp *iprobs =
#ifdef REINCARNATION
				    (Is_rogue_level(&u.uz)) ?
				    (const struct icp *)rogueprobs :
#endif
				    Inhell ? (const struct icp *)hellprobs :
				    (const struct icp *)mkobjprobs;

		for(tprob = rnd(100);
		    (tprob -= iprobs->iprob) > 0;
		    iprobs++);
		oclass = iprobs->iclass;
	}

	i = bases[(int)oclass];
	while((prob -= objects[i].oc_prob) > 0) i++;

	if(objects[i].oc_class != oclass || !OBJ_NAME(objects[i]))
		panic("probtype error, oclass=%d i=%d", (int) oclass, i);

	return(mksobj(i, TRUE, artif));
}

STATIC_OVL void
mkbox_cnts(box)
struct obj *box;
{
	register int n;
	register int min = 0;
	register struct obj *otmp;

	box->cobj = (struct obj *) 0;

	switch (box->otyp) {
	case ICE_BOX:		n = 20; break;
	case CHEST:		n = 5; break;
	case BOX:		n = 3; break;
	case MASSIVE_STONE_CRATE: min=1;
	case SACK:
	case OILSKIN_SACK:
				/* initial inventory: sack starts out empty */
				if (moves <= 1 && !in_mklev) { n = 0; break; }
				/*else FALLTHRU*/
	case BAG_OF_HOLDING:	n = 1; break;
	default:		n = 0; break;
	}

	for (n = max_ints(rn2(n+1),min); n > 0; n--) {
	    if (box->otyp == ICE_BOX) {
			if (!(otmp = mksobj(CORPSE, TRUE, TRUE))) continue;
			/* Note: setting age to 0 is correct.  Age has a different
			 * from usual meaning for objects stored in ice boxes. -KAA
			 */
			otmp->age = 0L;
			if (otmp->timed) {
				(void) stop_timer(ROT_CORPSE, (genericptr_t)otmp);
				(void) stop_timer(REVIVE_MON, (genericptr_t)otmp);
			}
	    } else if(box->otyp == MASSIVE_STONE_CRATE){
			if (!(otmp = mkobj(FOOD_CLASS, TRUE))) continue;
	    } else {
		register int tprob;
		const struct icp *iprobs = boxiprobs;

		for (tprob = rnd(100); (tprob -= iprobs->iprob) > 0; iprobs++)
		    ;
		if (!(otmp = mkobj(iprobs->iclass, TRUE))) continue;

		/* handle a couple of special cases */
		if (otmp->oclass == COIN_CLASS) {
		    /* 2.5 x level's usual amount; weight adjusted below */
		    otmp->quan = (long)(rnd(level_difficulty()+2) * rnd(75));
		    otmp->owt = weight(otmp);
		} else while (otmp->otyp == ROCK) {
		    otmp->otyp = rnd_class(DILITHIUM_CRYSTAL, LOADSTONE);
			otmp->obj_material = objects[otmp->otyp].oc_material;
		    if (otmp->quan > 2L) otmp->quan = 1L;
		    otmp->owt = weight(otmp);
		}
		if (box->otyp == BAG_OF_HOLDING) {
		    if (Is_mbag(otmp)) {
			otmp->otyp = SACK;
			otmp->spe = 0;
			otmp->owt = weight(otmp);
		    } else while (otmp->otyp == WAN_CANCELLATION)
			    otmp->otyp = rnd_class(WAN_LIGHT, WAN_LIGHTNING);
		}
	    }
	    (void) add_to_container(box, otmp);
	}
}

int
rndmonnum()	/* select a random, common monster type */
{
	register struct permonst *ptr;
	register int	i;

	/* Plan A: get a level-appropriate common monster */
	ptr = rndmonst();
	if (ptr) return(monsndx(ptr));

	/* Plan B: get any common monster */
	do {
	    i = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
	    ptr = &mons[i];
	} while((ptr->geno & G_NOGEN) || (!Inhell && (ptr->geno & G_HELL)));

	return(i);
}

/*
 * Split obj so that it gets size gets reduced by num. The quantity num is
 * put in the object structure delivered by this call.  The returned object
 * has its wornmask cleared and is positioned just following the original
 * in the nobj chain (and nexthere chain when on the floor).
 */
struct obj *
splitobj(obj, num)
struct obj *obj;
long num;
{
	struct obj *otmp;

	if (obj->cobj || num <= 0L || obj->quan <= num)
	    panic("splitobj");	/* can't split containers */
	otmp = newobj(obj->oxlth + obj->onamelth);
	*otmp = *obj;		/* copies whole structure */
	otmp->o_id = flags.ident++;
	if (!otmp->o_id) otmp->o_id = flags.ident++;	/* ident overflowed */
	otmp->timed = 0;	/* not timed, yet */
	otmp->lamplit = 0;	/* ditto */
	otmp->owornmask = 0L;	/* new object isn't worn */
	obj->quan -= num;
	obj->owt = weight(obj);
	otmp->quan = num;
	otmp->owt = weight(otmp);	/* -= obj->owt ? */
	obj->nobj = otmp;
	/* Only set nexthere when on the floor, nexthere is also used */
	/* as a back pointer to the container object when contained. */
	if (obj->where == OBJ_FLOOR)
	    obj->nexthere = otmp;
	if (obj->oxlth)
	    (void)memcpy((genericptr_t)otmp->oextra, (genericptr_t)obj->oextra,
			obj->oxlth);
	if (obj->onamelth)
	    (void)strncpy(ONAME(otmp), ONAME(obj), (int)obj->onamelth);
	if (obj->unpaid) splitbill(obj,otmp);
	if (obj->timed) obj_split_timers(obj, otmp);
	if (obj_sheds_light(obj)) obj_split_light_source(obj, otmp);
	return otmp;
}

/*
 * Insert otmp right after obj in whatever chain(s) it is on.  Then extract
 * obj from the chain(s).  This function does a literal swap.  It is up to
 * the caller to provide a valid context for the swap.  When done, obj will
 * still exist, but not on any chain.
 *
 * Note:  Don't use use obj_extract_self() -- we are doing an in-place swap,
 * not actually moving something.
 */
void
replace_object(obj, otmp)
struct obj *obj;
struct obj *otmp;
{
    otmp->where = obj->where;
    switch (obj->where) {
    case OBJ_FREE:
	/* do nothing */
	break;
    case OBJ_INVENT:
	otmp->nobj = obj->nobj;
	obj->nobj = otmp;
	extract_nobj(obj, &invent);
	break;
    case OBJ_CONTAINED:
	otmp->nobj = obj->nobj;
	otmp->ocontainer = obj->ocontainer;
	obj->nobj = otmp;
	extract_nobj(obj, &obj->ocontainer->cobj);
	break;
    case OBJ_MINVENT:
	otmp->nobj = obj->nobj;
	otmp->ocarry =  obj->ocarry;
	obj->nobj = otmp;
	extract_nobj(obj, &obj->ocarry->minvent);
	break;
    case OBJ_FLOOR:
	otmp->nobj = obj->nobj;
	otmp->nexthere = obj->nexthere;
	otmp->ox = obj->ox;
	otmp->oy = obj->oy;
	obj->nobj = otmp;
	obj->nexthere = otmp;
	extract_nobj(obj, &fobj);
	extract_nexthere(obj, &level.objects[obj->ox][obj->oy]);
	break;
    default:
	panic("replace_object: obj position");
	break;
    }
}

/*
 * Create a dummy duplicate to put on shop bill.  The duplicate exists
 * only in the billobjs chain.  This function is used when a shop object
 * is being altered, and a copy of the original is needed for billing
 * purposes.  For example, when eating, where an interruption will yield
 * an object which is different from what it started out as; the "I x"
 * command needs to display the original object.
 *
 * The caller is responsible for checking otmp->unpaid and
 * costly_spot(u.ux, u.uy).  This function will make otmp no charge.
 *
 * Note that check_unpaid_usage() should be used instead for partial
 * usage of an object.
 */
void
bill_dummy_object(otmp)
register struct obj *otmp;
{
	register struct obj *dummy;

	if (otmp->unpaid)
	    subfrombill(otmp, shop_keeper(*u.ushops));
	dummy = newobj(otmp->oxlth + otmp->onamelth);
	*dummy = *otmp;
	dummy->where = OBJ_FREE;
	dummy->o_id = flags.ident++;
	if (!dummy->o_id) dummy->o_id = flags.ident++;	/* ident overflowed */
	dummy->timed = 0;
	if (otmp->oxlth)
	    (void)memcpy((genericptr_t)dummy->oextra,
			(genericptr_t)otmp->oextra, otmp->oxlth);
	if (otmp->onamelth)
	    (void)strncpy(ONAME(dummy), ONAME(otmp), (int)otmp->onamelth);
	if (Is_candle(dummy)) dummy->lamplit = 0;
	addtobill(dummy, FALSE, TRUE, TRUE);
	otmp->no_charge = 1;
	otmp->unpaid = 0;
	return;
}

#endif /* OVL1 */
#ifdef OVLB

static const char dknowns[] = {
		WAND_CLASS, RING_CLASS, POTION_CLASS, SCROLL_CLASS,
		GEM_CLASS, SPBOOK_CLASS, WEAPON_CLASS, TOOL_CLASS, 0
};

struct obj *
mksobj(otyp, init, artif)
int otyp;
boolean init;
boolean artif;
{
	int mndx, tryct;
	struct obj *otmp;
	char let = objects[otyp].oc_class;

	otmp = newobj(0);
	*otmp = zeroobj;
	otmp->age = monstermoves;
	otmp->o_id = flags.ident++;
	if (!otmp->o_id) otmp->o_id = flags.ident++;	/* ident overflowed */
	otmp->quan = 1L;
	otmp->oclass = let;
	otmp->otyp = otyp;
	otmp->where = OBJ_FREE;
	otmp->dknown = index(dknowns, let) ? 0 : 1;
	otmp->corpsenm = 0; /* BUGFIX: Where does this get set? shouldn't it be given a default during initialization? */
	otmp->objsize = MZ_MEDIUM;
	otmp->bodytypeflag = MB_HUMANOID;
	otmp->ovar1 = 0;
	otmp->oward = 0;
	otmp->oproperties = 0;
	otmp->gifted = A_NONE;
	otmp->lifted = 0;
	otmp->shopOwned = 0;
	otmp->sknown = 0;
	otmp->ostolen = 0;
	otmp->lightened = 0;
	otmp->obroken = 0; /* BUGFIX: shouldn't this be set to 0 initially? */
	otmp->opoisoned = 0;
	otmp->fromsink = 0;
	otmp->mp = (struct mask_properties *) 0;
	
	if(otyp == find_gcirclet()) otmp->obj_material = GOLD;
	else if(otyp == SPEAR && !rn2(25)) otmp->obj_material = SILVER;
	else if(otyp == DAGGER && !rn2(12)) otmp->obj_material = SILVER;
	else if(otyp == STILETTOS && !rn2(12)) otmp->obj_material = SILVER;
	else otmp->obj_material = objects[otyp].oc_material;
	
	if(otyp == VIPERWHIP) otmp->ovar1 = rn2(2) ? 1 : rn2(5) ? rnd(2) : rnd(5);
	
	fix_object(otmp);
	
	if ((otmp->otyp >= ELVEN_SHIELD && otmp->otyp <= ORCISH_SHIELD) ||
			otmp->otyp == SHIELD_OF_REFLECTION)
		otmp->dknown = 0;
	if (!objects[otmp->otyp].oc_uses_known && otmp->otyp!=POT_BLOOD)
		otmp->known = 1;
#ifdef INVISIBLE_OBJECTS
	otmp->oinvis = !rn2(1250);
#endif
	otmp->quan = is_multigen(otmp) ? (long) rn1(6,6) : 1L;
	if (init) switch (let) {
	case WEAPON_CLASS:
		if(!rn2(11)) {
			otmp->spe = rne(3);
			otmp->blessed = rn2(2);
		} else if(!rn2(10)) {
			curse(otmp);
			otmp->spe = -rne(3);
		} else	blessorcurse(otmp, 10);
		
		if(otmp->otyp == WHITE_VIBROSWORD
		 || otmp->otyp == WHITE_VIBROSPEAR
		 || otmp->otyp == WHITE_VIBROZANBATO
		)
			otmp->oproperties |= OPROP_HOLYW;
		if(otmp->otyp == GOLD_BLADED_VIBROSWORD
		 || otmp->otyp == GOLD_BLADED_VIBROSPEAR
		 || otmp->otyp == GOLD_BLADED_VIBROZANBATO
		)
			otmp->oproperties |= OPROP_UNHYW;
		
		if(is_vibroweapon(otmp)){
			otmp->ovar1 = 80L + rnd(20);
		}
		else if(otmp->otyp == RAYGUN){
			otmp->ovar1 = (8 + rnd(8))*10L;
			otmp->altmode = AD_SLEE;
		}
		else if(otmp->otyp == MASS_SHADOW_PISTOL){
			struct obj *stone = mksobj(ROCK, TRUE, FALSE);
			otmp->ovar1 = 800L + rnd(200);
			stone->quan = 1;
			stone->owt = weight(stone);
			add_to_container(otmp, stone);
			container_weight(otmp);
		}
		else if(is_blaster(otmp)){ //Rayguns and mass-shadow pistols are also blasters, so this has to go under that case
			otmp->ovar1 = 80L + rnd(20);
			if(otmp->otyp == ARM_BLASTER) otmp->altmode = WP_MODE_SINGLE;
			if(otmp->otyp == RAYGUN) otmp->altmode = AD_FIRE;	// I think this is never reached?
		}
		else if(otmp->otyp == MOON_AXE){
			switch(phase_of_the_moon()){
				case 0:
					otmp->ovar1 = ECLIPSE_MOON;
				break;
				case 1:
				case 7:
					otmp->ovar1 = CRESCENT_MOON;
				break;
				case 2:
				case 6:
					otmp->ovar1 = HALF_MOON;
				break;
				case 3:
				case 5:
					otmp->ovar1 = GIBBOUS_MOON;
				break;
				case 4:
					otmp->ovar1 = FULL_MOON;
				break;
			}
		}
		if (artif && !rn2(Role_if(PM_PIRATE) ? 5 : 20))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
//#ifdef FIREARMS
		if (otmp->otyp == STICK_OF_DYNAMITE) {
			otmp->age = (otmp->cursed ? rn2(15) + 2 : 
					(otmp->blessed ? 15 : rn2(10) + 10));
		}
//#endif
		if (is_poisonable(otmp) && ((is_ammo(otmp) && !rn2(100)) || !rn2(1000) )){
			if(!rn2(100)) otmp->opoisoned = OPOISON_FILTH; /* Once a game or once every few games */
			else otmp->opoisoned = OPOISON_BASIC;
		} else if((otmp)->obj_material == WOOD 
			&& otmp->oartifact != ART_BOW_OF_SKADI
			&& !rn2(100)
		){
			switch(d(1,4)){
				case 1: otmp->oward = WARD_TOUSTEFNA; break;
				case 2: otmp->oward = WARD_DREPRUN; break;
				case 3: otmp->oward = WARD_VEIOISTAFUR; break;
				case 4: otmp->oward = WARD_THJOFASTAFUR; break;
			}
		}
		break;
	case FOOD_CLASS:
	    otmp->odrained = 0;
	    otmp->oeaten = 0;
	    switch(otmp->otyp) {
	    case CORPSE:
		/* possibly overridden by mkcorpstat() */
		tryct = 50;
		do otmp->corpsenm = undead_to_corpse(rndmonnum());
		while ((mvitals[otmp->corpsenm].mvflags & G_NOCORPSE) && (--tryct > 0));
		if (tryct == 0) {
		/* perhaps rndmonnum() only wants to make G_NOCORPSE monsters on
		   this level; let's create an adventurer's corpse instead, then */
			otmp->corpsenm = PM_HUMAN;
		}
		/* timer set below */
		break;
	    case EGG:
		otmp->corpsenm = NON_PM;	/* generic egg */
		
		if(In_sokoban(&u.uz) || Is_gatetown(&u.uz)) break; /*Some levels shouldn't have mosnters spawning from eggs*/
		
		if (!rn2(3)) for (tryct = 200; tryct > 0; --tryct) {
		    mndx = can_be_hatched(rndmonnum());
		    if (mndx != NON_PM && !dead_species(mndx, TRUE)) {
			otmp->corpsenm = mndx;		/* typed egg */
			attach_egg_hatch_timeout(otmp);
			break;
		    }
		}
		break;
	    case EYEBALL:
			otmp->corpsenm = PM_HUMAN;
		break;
	    case TIN:
		otmp->corpsenm = NON_PM;	/* empty (so far) */
		if (!rn2(6))
		    otmp->spe = 1;		/* spinach */
		else for (tryct = 200; tryct > 0; --tryct) {
		    mndx = undead_to_corpse(rndmonnum());
		    if (mons[mndx].cnutrit &&
			    !(mvitals[mndx].mvflags & G_NOCORPSE)) {
			otmp->corpsenm = mndx;
			break;
		    }
		}
		blessorcurse(otmp, 10);
		break;
	    case SLIME_MOLD:
		otmp->spe = current_fruit;
		break;
	    case KELP_FROND:
		otmp->quan = (long) rnd(2);
		break;
		case PROTEIN_PILL:
			otmp->quan = rnd(5) + 5;
			otmp->owt = weight(otmp);
		break;
	    }
	    if (otmp->otyp == CORPSE || otmp->otyp == MEAT_RING ||
		otmp->otyp == KELP_FROND) break;
	    /* fall into next case */

	case GEM_CLASS:
		if (otmp->otyp == LOADSTONE) curse(otmp);
		else if (otmp->otyp == ROCK) otmp->quan = (long) rn1(6,6);
		else if (otmp->otyp != LUCKSTONE && !rn2(6)) otmp->quan = 2L;
		else otmp->quan = 1L;
		if (otmp->otyp == CHUNK_OF_FOSSIL_DARK){
			place_object(otmp, u.ux, u.uy);  /* make it viable light source */
			begin_burn(otmp, FALSE);
			obj_extract_self(otmp);	 /* now release it for caller's use */
		}
		if (artif && !rn2(Role_if(PM_PIRATE) ? 5 : 20))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
	break;
	case TOOL_CLASS:
	    switch(otmp->otyp) {
		case TALLOW_CANDLE:
		case WAX_CANDLE:	otmp->spe = 1;
					otmp->age = 20L * /* 400 or 200 */
					      (long)objects[otmp->otyp].oc_cost;
					otmp->lamplit = 0;
					otmp->quan = 1L + ((long)(rn2(2) && !Is_grue_level(&u.uz)) ? rn2(7) : 0);
					blessorcurse(otmp, 5);
					break;
		case BRASS_LANTERN:
		case OIL_LAMP:		otmp->spe = 1;
					otmp->age = (long) rn1(500,1000);
					otmp->lamplit = 0;
					blessorcurse(otmp, 5);
					break;
		case TORCH:
					otmp->age = (long) rn1(500,1000);
					otmp->lamplit = 0;
					blessorcurse(otmp, 5);
					break;
		case SUNROD:
					otmp->age = (long) rn1(500,1000);
					otmp->lamplit = 0;
					blessorcurse(otmp, 5);
					break;
		case SHADOWLANDER_S_TORCH:
					otmp->age = (long) rn1(500,1000);
					otmp->lamplit = 0;
					blessorcurse(otmp, 5);
					break;
		case CANDLE_OF_INVOCATION:
		case MAGIC_LAMP:	otmp->spe = 1;
					otmp->lamplit = 0;
					blessorcurse(otmp, 2);
					break;
		case SEISMIC_HAMMER:
			otmp->ovar1 = 80L + rnd(20);
		break;
		case DOUBLE_LIGHTSABER:
		case LIGHTSABER:
		case BEAMSWORD:
					otmp->altmode = FALSE;
					otmp->lamplit = 0;
					otmp->age = (long) rn1(50000,100000);
					blessorcurse(otmp, 2);
					{
						struct obj *gem = mksobj(rn2(6) ? BLUE_FLUORITE : GREEN_FLUORITE, TRUE, FALSE);
						gem->quan = 1;
						gem->owt = weight(gem);
						add_to_container(otmp, gem);
						container_weight(otmp);
					}
					otmp->ovar1 = random_saber_hilt();
					break;
		case CHEST:
		case BOX:
			if(Is_stronghold(&u.uz) && in_mklev){
				otmp->olocked = 1;
				otmp->otrapped = 0;
			} else {
				otmp->olocked = !!(rn2(5));
				otmp->otrapped = !(rn2(10));
			}
		case ICE_BOX:
		case SACK:
		case OILSKIN_SACK:
		case MASSIVE_STONE_CRATE:
		case BAG_OF_HOLDING:	mkbox_cnts(otmp);
					break;
		case MAGIC_CHEST:
			otmp->olocked = 1;
		break;
#ifdef TOURIST
		case EXPENSIVE_CAMERA:
#endif
		case TINNING_KIT:
		case MAGIC_MARKER:	otmp->spe = rn1(70,30);
					break;
		case CAN_OF_GREASE:	otmp->spe = rnd(25);
					blessorcurse(otmp, 10);
		break;
		case CRYSTAL_BALL:	otmp->spe = rnd(5);
					blessorcurse(otmp, 2);
		break;
		case POWER_PACK:
			otmp->quan = rnd(5) + 5;
			otmp->owt = weight(otmp);
		break;
		case SENSOR_PACK:
			otmp->spe = rnd(5) + 20;
		break;
		case HYPOSPRAY_AMPULE:{
			int pick;
			switch(rn2(14)){
				case 0:
					pick = POT_GAIN_ABILITY;
				break;
				case 1:
					pick = POT_RESTORE_ABILITY;
				break;
				case 2:
					pick = POT_BLINDNESS;
				break;
				case 3:
					pick = POT_CONFUSION;
				break;
				case 4:
					pick = POT_PARALYSIS;
				break;
				case 5:
					pick = POT_SPEED;
				break;
				case 6:
					pick = POT_HALLUCINATION;
				break;
				case 7:
					pick = POT_HEALING;
				break;
				case 8:
					pick = POT_EXTRA_HEALING;
				break;
				case 9:
					pick = POT_GAIN_ENERGY;
				break;
				case 10:
					pick = POT_SLEEPING;
				break;
				case 11:
					pick = POT_FULL_HEALING;
				break;
				case 12:
					pick = POT_POLYMORPH;
				break;
				case 13:
					pick = POT_AMNESIA;
				break;
			}
			// otmp->ovar1 = (long)(rn2(POT_POLYMORPH - POT_GAIN_ABILITY + 1) + POT_GAIN_ABILITY);
			otmp->ovar1 = (long)(pick);
			otmp->spe = rn1(6,6);
		}break;
		case HORN_OF_PLENTY:
		case BAG_OF_TRICKS:	otmp->spe = rnd(20);
					break;
		case FIGURINE:	{
					if(Is_paradise(&u.uz)){
						switch(rn2(3)){
							case 0:
								otmp->corpsenm = PM_SHADE;
							break;
							case 1:
								otmp->corpsenm = PM_DARKNESS_GIVEN_HUNGER;
							break;
							case 2:
								otmp->corpsenm = PM_NIGHTGAUNT;
							break;
						}
					} else if(Is_sunkcity(&u.uz)){
						switch(rn2(3)){
							case 0:
								otmp->corpsenm = PM_DEEPEST_ONE;
							break;
							case 1:
								otmp->corpsenm = PM_MASTER_MIND_FLAYER;
							break;
							case 2:
								otmp->corpsenm = PM_SHOGGOTH;
							break;
						}
					} else {
						int tryct2 = 0;
						do
							otmp->corpsenm = rndmonnum();
						while(is_human(&mons[otmp->corpsenm])
							&& tryct2++ < 30);
					}
					blessorcurse(otmp, 4);
				break;
				}
		case BELL_OF_OPENING:   otmp->spe = 3;
					break;
		case MAGIC_FLUTE:
		case MAGIC_HARP:
		case FROST_HORN:
		case FIRE_HORN:
		case DRUM_OF_EARTHQUAKE:
					otmp->spe = rn1(5,4);
		break;
	    case MASK:
			if(rn2(4)){
				int tryct2 = 0;
				do otmp->corpsenm = rndmonnum();
				while(is_human(&mons[otmp->corpsenm])
					&& tryct2++ < 30);
			} else if(rn2(10)){
				do otmp->corpsenm = rn2(PM_LONG_WORM_TAIL);
				while(mons[otmp->corpsenm].geno & G_UNIQ);
			} else {
				do otmp->corpsenm = PM_ARCHEOLOGIST + rn2(PM_WIZARD - PM_ARCHEOLOGIST);
				while(otmp->corpsenm == PM_WORM_THAT_WALKS);
			}
			doMaskStats(otmp);
		break;
		}
		if (artif && !rn2(Role_if(PM_PIRATE) ? 10 : 40))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
	break;
	case AMULET_CLASS:
		if (otmp->otyp == AMULET_OF_YENDOR) flags.made_amulet = TRUE;
		if(rn2(10) && (otmp->otyp == AMULET_OF_STRANGULATION ||
		   otmp->otyp == AMULET_OF_CHANGE ||
		   otmp->otyp == AMULET_OF_RESTFUL_SLEEP)) {
			curse(otmp);
		} else	blessorcurse(otmp, 10);
		if (artif && !rn2(Role_if(PM_PIRATE) ? 5 : 20))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
	case VENOM_CLASS:
	case CHAIN_CLASS:
	case BALL_CLASS:
		break;
	case POTION_CLASS:
		if(otmp->otyp == POT_BLOOD){
			otmp->corpsenm = PM_HUMAN;	/* default value */
			for (tryct = 200; tryct > 0; --tryct) {
				mndx = undead_to_corpse(rndmonnum());
				if (mons[mndx].cnutrit &&
					!(mvitals[mndx].mvflags & G_NOCORPSE)
					&& has_blood(&mons[mndx]) ) {
				otmp->corpsenm = mndx;
				break;
				}
			}
			blessorcurse(otmp, 10);
		}
		if (otmp->otyp == POT_OIL)
		    otmp->age = MAX_OIL_IN_FLASK;	/* amount of oil */
		if (otmp->otyp == POT_STARLIGHT){
			place_object(otmp, u.ux, u.uy);  /* make it viable light source */
			begin_burn(otmp, FALSE);
			obj_extract_self(otmp);	 /* now release it for caller's use */
		}
		/* fall through */
	case SCROLL_CLASS:
#ifdef MAIL
		if (otmp->otyp != SCR_MAIL)
#endif
			blessorcurse(otmp, 4);
		if(otmp->otyp == SCR_WARD){
			int prob = rn2(73);
			/*The circle of acheron is so common and so easy to draw that noone makes ward scrolls of it*/
			if(prob < 10) otmp->oward = WINGS_OF_GARUDA;
			else if(prob < 20) otmp->oward = CARTOUCHE_OF_THE_CAT_LORD;
			else if(prob < 30) otmp->oward = SIGN_OF_THE_SCION_QUEEN;
			else if(prob < 40) otmp->oward = ELDER_ELEMENTAL_EYE;
			else if(prob < 50) otmp->oward = ELDER_SIGN;
			else if(prob < 60) otmp->oward = HAMSA;
			else if(prob < 70) otmp->oward = PENTAGRAM;
			else otmp->oward = HEXAGRAM;
		}
		if (artif && !rn2(Role_if(PM_PIRATE) ? 5 : 20))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
	break;
	case SPBOOK_CLASS:
		blessorcurse(otmp, 17);
		// WARD_ACHERON			0x0000008L
		// WARD_QUEEN			0x0000200L
		// WARD_GARUDA			0x0000800L

		// WARD_ELDER_SIGN		0x0000080L
		// WARD_EYE				0x0000100L
		// WARD_CAT_LORD		0x0000400L

		// WARD_HEXAGRAM		0x0000020L
		// WARD_PENTAGRAM		0x0000010L
		// WARD_HAMSA			0x0000040L

		// WARD_HEPTAGRAM		0x0000002L

		/*Spellbooks are warded to help contain the magic.
		  Some of the wards contain usable symbols*/
		switch (objects[otmp->otyp].oc_level) {
		 case 0:
		 break;
		 case 1:
			if( (rn2(3)) ) otmp->oward = WARD_ACHERON;
			else if( !(rn2(3)) ){
				if( rn2(2) ) otmp->oward = WARD_QUEEN;
				else otmp->oward = WARD_GARUDA;
			}
		 break;
		 case 2:
			if( rn2(2) ){
				if( !(rn2(8)) ) otmp->oward = WARD_EYE;
				else if( rn2(2) ) otmp->oward = WARD_QUEEN;
				else otmp->oward = WARD_GARUDA;
			}
			else if( rn2(3) ) otmp->oward = WARD_ACHERON;
		 break;
		 case 3:
			if( !(rn2(3)) ){
				if( !(rn2(5)) ) otmp->oward = WARD_EYE;
				else if( !(rn2(4)) ) otmp->oward = WARD_QUEEN;
				else if( !(rn2(3)) )otmp->oward = WARD_GARUDA;
				else if(   rn2(2) )otmp->oward = WARD_ELDER_SIGN;
				else otmp->oward = WARD_CAT_LORD;
			}
			else if(rn2(2)){
				if( !(rn2(4)) ) otmp->oward = WARD_TOUSTEFNA;
				else if( !(rn2(3)) )otmp->oward = WARD_DREPRUN;
				else if(  (rn2(2)) )otmp->oward = WARD_VEIOISTAFUR;
				else otmp->oward = WARD_THJOFASTAFUR;
			}
			else if( rn2(3) ) otmp->oward = WARD_ACHERON;
		 break;
		 case 4:
			if( rn2(4) ){
				if( !(rn2(9)) ) otmp->oward = WARD_EYE;
				else if( !rn2(8) ) otmp->oward = WARD_QUEEN;
				else if( !rn2(7) )otmp->oward = WARD_GARUDA;
				else if( !rn2(6) )otmp->oward = WARD_ELDER_SIGN;
				else if( !rn2(5) )otmp->oward = WARD_CAT_LORD;
				else if( !rn2(4) ) otmp->oward = WARD_TOUSTEFNA;
				else if( !rn2(3) )otmp->oward = WARD_DREPRUN;
				else if(  rn2(2) )otmp->oward = WARD_VEIOISTAFUR;
				else otmp->oward = WARD_THJOFASTAFUR;
			}
			else otmp->oward = WARD_ACHERON;
		 break;
		 case 5:
			if( !(rn2(4)) ){
				if( !(rn2(2)) ) otmp->oward = WARD_PENTAGRAM;
				else otmp->oward = WARD_HAMSA;
			}
			else if( (rn2(3)) ){
				if( !(rn2(8)) ) otmp->oward = WARD_QUEEN;
				else if( !rn2(7) )otmp->oward = WARD_GARUDA;
				else if( !rn2(6) )otmp->oward = WARD_ELDER_SIGN;
				else if( !rn2(5) )otmp->oward = WARD_CAT_LORD;
				else if( !rn2(4) ) otmp->oward = WARD_TOUSTEFNA;
				else if( !rn2(3) )otmp->oward = WARD_DREPRUN;
				else if(  rn2(2) )otmp->oward = WARD_VEIOISTAFUR;
				else otmp->oward = WARD_THJOFASTAFUR;
			}
			else otmp->oward = WARD_EYE;
		 break;
		 case 6:
			if( !(rn2(3)) ){
				if( !(rn2(3)) ) otmp->oward = WARD_PENTAGRAM;
				else if( !rn2(2) )otmp->oward = WARD_HEXAGRAM;
				else otmp->oward = WARD_HAMSA;
			}
			else if( (rn2(6)) ){
				if( !(rn2(8)) ) otmp->oward = WARD_QUEEN;
				else if( !rn2(7) )otmp->oward = WARD_GARUDA;
				else if( !rn2(6) )otmp->oward = WARD_ELDER_SIGN;
				else if( !rn2(5) )otmp->oward = WARD_CAT_LORD;
				else if( !rn2(4) ) otmp->oward = WARD_TOUSTEFNA;
				else if( !rn2(3) )otmp->oward = WARD_DREPRUN;
				else if(  rn2(2) )otmp->oward = WARD_VEIOISTAFUR;
				else otmp->oward = WARD_THJOFASTAFUR;
			}
			else otmp->oward = WARD_EYE;
		 break;
		 case 7:
			if( !(rn2(4)) ){
				if( !(rn2(3)) ) otmp->oward = WARD_EYE;
				else if( !rn2(2) )otmp->oward = WARD_ELDER_SIGN;
				else otmp->oward = WARD_CAT_LORD;
			}
			else if( !(rn2(3)) ){
				if( !(rn2(3)) ) otmp->oward = WARD_ACHERON;
				else if( !rn2(2) )otmp->oward = WARD_QUEEN;
				else otmp->oward = WARD_GARUDA;
			}
			else if( !(rn2(2)) ){
				if( !(rn2(3)) ) otmp->oward = WARD_HEXAGRAM;
				else if( !rn2(2) )otmp->oward = WARD_PENTAGRAM;
				else otmp->oward = WARD_HAMSA;
			}
			else{
				otmp->oward = WARD_HEPTAGRAM;
			}
		 break;
		 default:
			impossible("Unknown spellbook level %d, book %d;",
				objects[otmp->otyp].oc_level, otmp->otyp);
			return 0;
		}
		if (artif && !rn2(Role_if(PM_PIRATE) ? 5 : 20))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
	break;
	case ARMOR_CLASS:
		if(rn2(10) && (otmp->otyp == FUMBLE_BOOTS ||
		   otmp->otyp == HELM_OF_OPPOSITE_ALIGNMENT ||
		   otmp->otyp == GAUNTLETS_OF_FUMBLING ||
		   !rn2(11))) {
			curse(otmp);
			otmp->spe = -rne(3);
		} else if(!rn2(10)) {
			otmp->blessed = rn2(2);
			otmp->spe = rne(3);
		} else	blessorcurse(otmp, 10);
		if (artif && !rn2(Role_if(PM_PIRATE) ? 10 : 40))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
		/* simulate lacquered armor for samurai */
		if (Role_if(PM_SAMURAI) && otmp->otyp == SPLINT_MAIL &&
		    (moves <= 1 || In_quest(&u.uz))) {
#ifdef UNIXPC
			/* optimizer bitfield bug */
			otmp->oerodeproof = 1;
			otmp->rknown = 1;
#else
			otmp->oerodeproof = otmp->rknown = 1;
#endif
		}
		if(is_evaporable(otmp)){
			start_timer(1, TIMER_OBJECT,
					LIGHT_DAMAGE, (genericptr_t)otmp);
		}

		
		/* MRKR: Mining helmets have lamps */
		if (otmp->otyp == DWARVISH_HELM) {
		    otmp->age = (long) rn1(300,300);//Many fewer turns than brass lanterns, as there are so many.
		    otmp->lamplit = 0;
		}
		/* CM: gnomish hats have candles */
		if (otmp->otyp == GNOMISH_POINTY_HAT) {
		    otmp->age = (long) rn1(900,900);//Last longer than dwarvish helms, since the radius is smaller
		    otmp->lamplit = 0;
		}
		if(otmp->otyp == DROVEN_PLATE_MAIL || otmp->otyp == DROVEN_CHAIN_MAIL || otmp->otyp == CONSORT_S_SUIT){
			otmp->ohaluengr = TRUE;
			if(Race_if(PM_DROW) && Is_qstart(&u.uz)) otmp->oward = u.start_house;
			else if(!(rn2(10))) otmp->oward = rn2(EDDER_SYMBOL+1-LOLTH_SYMBOL)+LOLTH_SYMBOL;
			else if(!(rn2(4))) otmp->oward = rn2(LAST_HOUSE+1-FIRST_HOUSE)+FIRST_HOUSE;
			else otmp->oward = rn2(LAST_FALLEN_HOUSE+1-FIRST_FALLEN_HOUSE)+FIRST_FALLEN_HOUSE;
		}

	break;
	case WAND_CLASS:
		if(otmp->otyp == WAN_WISHING) otmp->spe = rnd(3); else
		otmp->spe = rn1(5,
			(objects[otmp->otyp].oc_dir == NODIR) ? 11 : 4);
		blessorcurse(otmp, 17);
		if (otmp->otyp == WAN_WISHING)
			otmp->recharged = 1;
		else
			otmp->recharged = 0; /* used to control recharging */
		if (artif && !rn2(Role_if(PM_PIRATE) ? 5 : 20))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
	break;
	case RING_CLASS:
		if(isEngrRing(otmp->otyp) && !rn2(3) ){
			if(rn2(4)){
				otmp->ohaluengr = TRUE;
				otmp->oward = (long)random_haluIndex();
			}
			else{
				otmp->ohaluengr = FALSE;
				otmp->oward = rn2(4) ? CIRCLE_OF_ACHERON :
								!rn2(6) ? HAMSA : 
								!rn2(5) ? ELDER_SIGN : 
								!rn2(4) ? WINGS_OF_GARUDA : 
								!rn2(3) ? ELDER_ELEMENTAL_EYE : 
								!rn2(2) ? SIGN_OF_THE_SCION_QUEEN : 
								          CARTOUCHE_OF_THE_CAT_LORD ;
			}
		}
		if(isSignetRing(otmp->otyp)){
			otmp->ohaluengr = TRUE;
			if(!(rn2(100))) otmp->oward = rn2(EDDER_SYMBOL+1-LOLTH_SYMBOL)+LOLTH_SYMBOL;
			else if(!(rn2(4))) otmp->oward = rn2(LAST_HOUSE+1-FIRST_HOUSE)+FIRST_HOUSE;
			else otmp->oward = rn2(LAST_FALLEN_HOUSE+1-FIRST_FALLEN_HOUSE)+FIRST_FALLEN_HOUSE;
		}
		if(otmp->otyp == RIN_WISHES){
			otmp->spe = rnd(3);
		}
		if(objects[otmp->otyp].oc_charged && otmp->otyp != RIN_WISHES) {
		    blessorcurse(otmp, 3);
		    if(rn2(10)) {
			if(rn2(10) && bcsign(otmp))
			    otmp->spe = bcsign(otmp) * rne(3);
			else otmp->spe = rn2(2) ? rne(3) : -rne(3);
		    }
		    /* make useless +0 rings much less common */
		    if (otmp->spe == 0) otmp->spe = rn2(4) - rn2(3);
		    /* negative rings are usually cursed */
		    if (otmp->spe < 0 && rn2(5)) curse(otmp);
		} else if(rn2(10) && (otmp->otyp == RIN_TELEPORTATION ||
			  otmp->otyp == RIN_POLYMORPH ||
			  otmp->otyp == RIN_AGGRAVATE_MONSTER ||
			  otmp->otyp == RIN_HUNGER || !rn2(9))) {
			curse(otmp);
		}
		if (artif && !rn2(Role_if(PM_PIRATE) ? 5 : 20))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
	break;
	case ROCK_CLASS:
		switch (otmp->otyp) {
		    case STATUE:
			/* possibly overridden by mkcorpstat() */
			otmp->corpsenm = rndmonnum();
			if (!verysmall(&mons[otmp->corpsenm]) &&
				rn2(level_difficulty()/2 + 10) > 10)
			    (void) add_to_container(otmp,
						    mkobj(SPBOOK_CLASS,FALSE));
			break;
			case FOSSIL:
				switch(rnd(12)){
					case 1:
						otmp->corpsenm = PM_SABER_TOOTHED_CAT;
					break;
					case 2:
						otmp->corpsenm = PM_TYRANNOSAURUS;
					break;
					case 3:
						otmp->corpsenm = PM_TRICERATOPS;
					break;
					case 4:
						otmp->corpsenm = PM_DIPLODOCUS;
					break;
					case 5:
					case 6:
					case 7:
					case 8:
						otmp->corpsenm = PM_TRILOBITE;
					break;
					case 9:
						otmp->corpsenm = PM_TITANOTHERE;
					break;
					case 10:
						otmp->corpsenm = PM_BALUCHITHERIUM;
					break;
					case 11:
						otmp->corpsenm = PM_MASTODON;
					break;
					case 12:
						otmp->corpsenm = PM_CENTIPEDE;
					break;
				}
		}
		if (artif && !rn2(Role_if(PM_PIRATE) ? 5 : 20))
		    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
	break;
	case COIN_CLASS:
	// case BED_CLASS:
	case TILE_CLASS:
		break;	/* do nothing */
	default:
		impossible("impossible mkobj %d, sym '%c'.", otmp->otyp,
						objects[otmp->otyp].oc_class);
		return (struct obj *)0;
	}

	/* Some things must get done (timers) even if init = 0 */
	switch (otmp->otyp) {
	    case CORPSE:
		start_corpse_timeout(otmp);
		break;
	}

	/* unique objects may have an associated artifact entry */
	if (objects[otyp].oc_unique && !otmp->oartifact)
	    otmp = mk_artifact(otmp, (aligntyp)A_NONE);
	otmp->owt = weight(otmp);
	return(otmp);
}

void
doMaskStats(mask)
	struct obj *mask;
{
	mask->mp = malloc(sizeof(struct mask_properties));
	if(is_mplayer(&mons[mask->corpsenm])){
		mask->mp->msklevel = rnd(10);
//		mask->mp->mskmonnum = 
		mask->mp->mskrolenum = mask->corpsenm;
//		mask->mp->mskfemale = 
////		mask->mp->mskacurr = malloc(sizeof(struct attribs));
////		mask->mp->mskaexe = malloc(sizeof(struct attribs));
////		mask->mp->mskamask = malloc(sizeof(struct attribs));
//		mask->mp->mskalign = 
		mask->mp->mskluck = rn1(9,-4);
//		mask->mp->mskhp = 
//		mask->mp->mskhpmax = mask->mp->mskhp;
//		mask->mp->msken = 
//		mask->mp->mskenmax = mask->mp->msken;
		mask->mp->mskgangr[0] = 0;
		mask->mp->mskgangr[1] = 0;
		mask->mp->mskgangr[2] = 0;
		mask->mp->mskgangr[3] = 0;
		mask->mp->mskgangr[4] = 0;
//		mask->mp->mskexp = 
		mask->mp->mskrexp = mask->mp->msklevel-1;
		mask->mp->mskweapon_slots = 0;
		mask->mp->mskskills_advanced = 0;
		// mask->mp->mskskill_record;
		// mask->mp->mskweapon_skills;
//	} else {
	}
}

/*
 * Start a corpse decay or revive timer.
 * This takes the age of the corpse into consideration as of 3.4.0.
 */
void
start_corpse_timeout(body)
	struct obj *body;
{
	long when; 		/* rot away when this old */
	long corpse_age;	/* age of corpse          */
	int rot_adjust;
	short action;
	long age;
	int chance;
	struct monst *attchmon = 0;

#define TAINT_AGE (50L)		/* age when corpses go bad */
#define TROLL_REVIVE_CHANCE 37	/* 1/37 chance for 50 turns ~ 75% chance */
#define MOLD_REVIVE_CHANCE 23	/*  1/23 chance for 50 turns ~ 90% chance */
#define BASE_MOLDY_CHANCE 19900	/*  1/19900 chance for 200 turns ~ 1% chance */
#define HALF_MOLDY_CHANCE 290	/*  1/290 chance for 200 turns ~ 50% chance */
#define FULL_MOLDY_CHANCE 87	/*  1/87 chance for 200 turns ~ 90% chance */
#define ROT_AGE (250L)		/* age when corpses rot away */

	/* lizards, beholders, and lichen don't rot or revive */
	if (body->corpsenm == PM_LIZARD || body->corpsenm == PM_LICHEN || body->corpsenm == PM_CROW_WINGED_HALF_DRAGON || body->corpsenm == PM_BEHOLDER || body->spe) return;
	
	if(body->oattached == OATTACHED_MONST) attchmon = (struct monst *)body->oextra;

	action = ROT_CORPSE;		/* default action: rot away */
	rot_adjust = in_mklev ? 25 : 10;	/* give some variation */
	corpse_age = monstermoves - body->age;
	if (corpse_age > ROT_AGE)
		when = rot_adjust;
	else
		when = ROT_AGE - corpse_age;
	when += (long)(rnz(rot_adjust) - rot_adjust);
	
//	pline("corpse type: %d, %c",mons[body->corpsenm].mlet,def_monsyms[mons[body->corpsenm].mlet]);
	if(is_migo(&mons[body->corpsenm])){
		when = when/10 + 1;
	}

	if (is_rider(&mons[body->corpsenm])
		|| (uwep && uwep->oartifact == ART_SINGING_SWORD && uwep->osinging == OSING_LIFE && attchmon && attchmon->mtame)
		) {
		/*
		 * Riders always revive.  They have a 1/3 chance per turn
		 * of reviving after 12 turns.  Always revive by 500.
		 */
//		pline("setting up rider revival for %s", xname(body));
		action = REVIVE_MON;
		for (when = 12L; when < 500L; when++)
		    if (!rn2(3)) break;

	} else if (attchmon && 
		(body->corpsenm == PM_UNDEAD_KNIGHT
		|| body->corpsenm == PM_WARRIOR_OF_SUNLIGHT
		)
		&& !body->norevive
	) {
//		pline("setting up undead revival for %s", xname(body));
		attchmon->mclone = 1;
		action = REVIVE_MON;
		when = rn2(TAINT_AGE)+2;
	} else if (attchmon
	 && (attchmon->mfaction == ZOMBIFIED)
	 && !body->norevive
	) {
//		pline("setting up zombie revival for %s", xname(body));
		attchmon->mclone = 1;
		for (age = 2; age <= TAINT_AGE; age++)
		    if (!rn2(HALF_MOLDY_CHANCE)) {	/* zombie revives */
			action = REVIVE_MON;
			when = age;
			break;
		    }
	} else if (mons[body->corpsenm].mlet == S_TROLL && !body->norevive) {
//		pline("setting up troll revival for %s", xname(body));
		for (age = 2; age <= TAINT_AGE; age++)
		    if (!rn2(TROLL_REVIVE_CHANCE)) {	/* troll revives */
			action = REVIVE_MON;
			when = age;
			break;
		    }
	} else if (is_fungus(&mons[body->corpsenm]) && 
			  !is_migo(&mons[body->corpsenm])) {
		/* Fungi come back with a vengeance - if you don't eat it or
		 * destroy it,  any live cells will quickly use the dead ones
		 * as food and come back.
		 */
//		pline("setting up fungus revival for %s", xname(body));
		for (age = 2; age <= TAINT_AGE; age++)
		    if (!rn2(MOLD_REVIVE_CHANCE)) {    /* mold revives */
			action = REVIVE_MON;
			when = age;
			break;
		    }
	}
	
	if (action == ROT_CORPSE && !acidic(&mons[body->corpsenm])){
		/* Corpses get moldy
		 */
		chance = (Is_zuggtmoy_level(&u.uz) && flags.spore_level) ? FULL_MOLDY_CHANCE : 
				 (Is_zuggtmoy_level(&u.uz) || flags.spore_level) ? HALF_MOLDY_CHANCE : 
				 BASE_MOLDY_CHANCE;
		for (age = TAINT_AGE + 1; age <= ROT_AGE; age++)
			if (!rn2(chance)) {    /* "revives" as a random s_fungus */
				action = MOLDY_CORPSE;
				when = age;
				break;
			}
	}
	chance = (Is_juiblex_level(&u.uz) && flags.slime_level) ? FULL_MOLDY_CHANCE : 
			 (Is_juiblex_level(&u.uz) || flags.slime_level) ? HALF_MOLDY_CHANCE : 
			 0;
	if(action == ROT_CORPSE && chance){
		for (age = TAINT_AGE + 1; age <= ROT_AGE; age++)
			if (!rn2(chance)) {    /* "revives" as a random s_fungus */
				action = SLIMY_CORPSE;
				when = age;
				break;
			}
	}
	chance = (flags.walky_level) ? TROLL_REVIVE_CHANCE : 
			 (attchmon && attchmon->zombify) ? FULL_MOLDY_CHANCE : 
			 (Is_night_level(&u.uz)) ? HALF_MOLDY_CHANCE : 
			 0;
	if(action == ROT_CORPSE && chance){
		for (age = TAINT_AGE + 1; age <= ROT_AGE; age++)
			if (!rn2(chance)) {    /* "revives" as a random s_fungus */
				action = ZOMBIE_CORPSE;
				when = age;
				break;
			}
	}
	chance = (Is_orcus_level(&u.uz) && flags.shade_level) ? FULL_MOLDY_CHANCE : 
			 (Is_orcus_level(&u.uz) || flags.shade_level) ? HALF_MOLDY_CHANCE : 
			 0;
	if(action == ROT_CORPSE && chance) {
		chance = FULL_MOLDY_CHANCE;
		for (age = TAINT_AGE + 1; age <= ROT_AGE; age++)
			if (!rn2(chance)) {    /* "revives" as a random s_fungus */
				action = SHADY_CORPSE;
				when = age;
				break;
			}
	}
	
	if (body->norevive) body->norevive = 0;
//	pline("Starting timer %d on %s", action, xname(body));
	(void) start_timer(when, TIMER_OBJECT, action, (genericptr_t)body);
}

void
bless(otmp)
register struct obj *otmp;
{
#ifdef GOLDOBJ
	if (otmp->oclass == COIN_CLASS) return;
#endif
	otmp->cursed = 0;
	otmp->blessed = 1;
	if (carried(otmp) && confers_luck(otmp))
	    set_moreluck();
	else if (otmp->otyp == BAG_OF_HOLDING)
	    otmp->owt = weight(otmp);
	else if (artifact_light(otmp) && otmp->lamplit)
		begin_burn(otmp, FALSE);
	else if (otmp->otyp == FIGURINE && otmp->timed)
		(void) stop_timer(FIG_TRANSFORM, (genericptr_t) otmp);
	return;
}

void
unbless(otmp)
register struct obj *otmp;
{
	otmp->blessed = 0;
	if (carried(otmp) && confers_luck(otmp))
	    set_moreluck();
	else if (otmp->otyp == BAG_OF_HOLDING)
	    otmp->owt = weight(otmp);
	else if (artifact_light(otmp) && otmp->lamplit)
		begin_burn(otmp, FALSE);

}

void
curse(otmp)
register struct obj *otmp;
{
#ifdef GOLDOBJ
	if (otmp->oclass == COIN_CLASS) return;
#endif
	otmp->blessed = 0;
	otmp->cursed = 1;
	/* welded two-handed weapon interferes with some armor removal */
	if (otmp == uwep && bimanual(uwep,youracedata)) reset_remarm();
	/* rules at top of wield.c state that twoweapon cannot be done
	   with cursed alternate weapon */
	if (otmp == uswapwep && u.twoweap)
	    drop_uswapwep();
	if (otmp == uarm && otmp->otyp == STRAITJACKET){
		struct obj *o;
		reset_remarm();
		if(u.twoweap && uswapwep) drop_uswapwep();
		if(uwep){
			o = uwep;
			setuwep((struct obj *)0);
			dropx(o);
		}
	}
	/* some cursed items need immediate updating */
	if (carried(otmp) && confers_luck(otmp))
	    set_moreluck();
	else if (otmp->otyp == BAG_OF_HOLDING)
	    otmp->owt = weight(otmp);
	else if (artifact_light(otmp) && otmp->lamplit) 
		begin_burn(otmp, FALSE);
	else if (otmp->otyp == FIGURINE) {
		if (otmp->corpsenm != NON_PM
		    && !dead_species(otmp->corpsenm,TRUE)
		    && (carried(otmp) || mcarried(otmp)))
			attach_fig_transform_timeout(otmp);
	}
	return;
}

void
uncurse(otmp)
register struct obj *otmp;
{
	otmp->cursed = 0;
	if (carried(otmp) && confers_luck(otmp))
	    set_moreluck();
	else if (otmp->otyp == BAG_OF_HOLDING)
	    otmp->owt = weight(otmp);
	else if (otmp->otyp == FIGURINE && otmp->timed)
	    (void) stop_timer(FIG_TRANSFORM, (genericptr_t) otmp);
	else if (artifact_light(otmp) && otmp->lamplit)
		begin_burn(otmp, FALSE);

	return;
}

#endif /* OVLB */
#ifdef OVL1

void
blessorcurse(otmp, chance)
register struct obj *otmp;
register int chance;
{
	if(otmp->blessed || otmp->cursed) return;

	if(!rn2(chance)) {
	    if(!rn2(2)) {
		curse(otmp);
	    } else {
		bless(otmp);
	    }
	}
	return;
}

#endif /* OVL1 */
#ifdef OVLB

int
bcsign(otmp)
register struct obj *otmp;
{
	return(!!otmp->blessed - !!otmp->cursed);
}

#endif /* OVLB */
#ifdef OVL0

void
set_material(obj, mat)
struct obj *obj;
int mat;
{
	struct monst *owner = 0;
	long mask = 0;
	
	if(mat == obj->obj_material) return; //Already done!
	
	if(obj->where == OBJ_INVENT || obj->where == OBJ_MINVENT){
		owner = obj->ocarry;
		mask = obj->owornmask;
	}
	if(owner == &youmonst && mask){
		setnotworn(obj);
	} else if(owner && mask){
		update_mon_intrinsics(owner, obj, FALSE, TRUE);
	}
	switch(obj->otyp){
		case ARROW:
			if(mat == GOLD) obj->otyp = GOLDEN_ARROW;
			else if(mat == SILVER) obj->otyp = SILVER_ARROW;
		break;
		case GOLDEN_ARROW:
			if(mat == SILVER) obj->otyp = SILVER_ARROW;
			else obj->otyp = ARROW;
		break;
		case SILVER_ARROW:
			if(mat == GOLD) obj->otyp = GOLDEN_ARROW;
			else obj->otyp = ARROW;
		break;
		case BULLET:
			if(mat == SILVER) obj->otyp = SILVER_BULLET;
		break;
		case SILVER_BULLET:
			obj->otyp = BULLET;
		break;
		case LEATHER_HELM:
			if(mat == COPPER) obj->otyp = BRONZE_HELM;
			else obj->otyp = HELMET;
		break;
		case BRONZE_HELM:
			if(mat == LEATHER) obj->otyp = LEATHER_HELM;
			else obj->otyp = HELMET;
		break;
		case HELMET:
			if(mat == LEATHER) obj->otyp = LEATHER_HELM;
			else if(mat == COPPER) obj->otyp = BRONZE_HELM;
		break;
		case DWARVISH_MITHRIL_COAT:
			obj->otyp = CHAIN_MAIL;
		break;
		case ELVEN_MITHRIL_COAT:
			obj->otyp = CHAIN_MAIL;
		break;
		case STUDDED_LEATHER_ARMOR:
			obj->otyp = SCALE_MAIL;
		break;
		case LEATHER_ARMOR:
			obj->otyp = PLATE_MAIL;
		break;
		// case LEATHER_CLOAK:
			// obj->otyp = ;
		// break;
		// case ROUNDSHIELD:
			// obj->otyp = ;
		// break;
		//Tin whistle, wood harp
		case GAUNTLETS:
			if(mat == COPPER) obj->otyp = BRONZE_GAUNTLETS;
		break;
		case BRONZE_GAUNTLETS:
			obj->otyp = GAUNTLETS;
		break;
		case ORIHALCYON_GAUNTLETS:
			if(mat == COPPER) obj->otyp = BRONZE_GAUNTLETS;
			else obj->otyp = GAUNTLETS;
		break;
		case LOW_BOOTS:
			if(mat == IRON) obj->otyp = SHOES;
		break;
		case SHOES:
			obj->otyp = LOW_BOOTS;
		break;
		case BRONZE_PLATE_MAIL:
			obj->otyp = PLATE_MAIL;
		break;
		case PLATE_MAIL:
			if(mat == COPPER) obj->otyp = BRONZE_PLATE_MAIL;
		break;
		case CRYSTAL_PLATE_MAIL:
			if(mat == COPPER) obj->otyp = BRONZE_PLATE_MAIL;
			else obj->otyp = PLATE_MAIL;
			//BUT, turning plate mail to glass results in glass plate mail.  The magic is lost.
		break;
		// case CRYSTAL_HELM:
			//????
		// break;
		case CRYSTAL_SWORD:
			if(mat != OBSIDIAN_MT && mat != GLASS && mat != GEMSTONE) obj->otyp = LONG_SWORD;
			//Reversing material transformation does not recover a crystal sword.  The magic is lost.
		break;
		// case MASSIVE_STONE_CRATE:
			// obj->otyp = ;
		// break;
		// case TALLOW_CANDLE:
			// obj->otyp = ;
		// break;
		// case WAX_CANDLE:
			// obj->otyp = ;
		// break;
		// case CRYSTAL_BALL:
			// obj->otyp = ;
		// break;
		case ROCK:
			if(mat == SILVER) obj->otyp = SILVER_SLINGSTONE;
			if(mat == GLASS){
				obj->otyp = LAST_GEM + rnd(9);
			}
			if(mat == GEMSTONE){
				obj->otyp = MAGICITE_CRYSTAL + rn2(LAST_GEM - MAGICITE_CRYSTAL + 1);
			}
		break;
		case SILVER_SLINGSTONE:
			obj->otyp = ROCK;
			if(mat == GLASS){
				obj->otyp = LAST_GEM + rnd(9);
			}
			if(mat == GEMSTONE){
				obj->otyp = MAGICITE_CRYSTAL + rn2(LAST_GEM - MAGICITE_CRYSTAL + 1);
			}
		break;
		// case HEAVY_IRON_BALL:
			// obj->otyp = ;
		// break;
		// case CHAIN:
			// obj->otyp = ;
		// break;
		// case IRON_BANDS:
			// obj->otyp = ;
		// break;
	}
	switch(objects[obj->otyp].oc_class){
		case POTION_CLASS:
			switch(mat){
				case GOLD:{
					static int goldpotion = 0; 
					if(!goldpotion) goldpotion = find_golden_potion();
					if(goldpotion > -1){
						obj->otyp = goldpotion;
					}
				}break;
			}
		break;
		case SPBOOK_CLASS:
			switch(mat){
				case CLOTH:{
					static int clothbook = 0; 
					if(!clothbook) clothbook = find_cloth_book();
					if(clothbook > -1){
						obj->otyp = clothbook;
					}
				}break;
				case LEATHER:{
					static int leatherbook = 0; 
					if(!leatherbook) leatherbook = find_leather_book();
					if(leatherbook > -1){
						obj->otyp = leatherbook;
					}
				}break;
				case COPPER:{
					static int bronzebook = 0; 
					if(!bronzebook) bronzebook = find_bronze_book();
					if(bronzebook > -1){
						obj->otyp = bronzebook;
					}
				}break;
				case SILVER:{
					static int silverbook = 0; 
					if(!silverbook) silverbook = find_silver_book();
					if(silverbook > -1){
						obj->otyp = silverbook;
					}
				}break;
				case GOLD:{
					static int goldbook = 0; 
					if(!goldbook) goldbook = find_gold_book();
					if(goldbook > -1){
						obj->otyp = goldbook;
					}
				}break;
			}
		break;
		case WAND_CLASS:
			obj->otyp = matWand(obj->otyp, mat);
			if(obj->otyp == WAN_WISHING)
				obj->spe /= 5;
			if(!(obj->recharged)) obj->recharged = 1;
			obj->obj_material = mat;
		break;
		case GEM_CLASS:
			if(mat != GLASS && mat != GEMSTONE){
				if(mat == SILVER) obj->otyp = SILVER_SLINGSTONE;
				else obj->otyp = ROCK;
			}
			obj->obj_material = mat;
		break;
		default:
			obj->obj_material = mat;
		break;
	}
	//Silver bell should resist
	
	fix_object(obj);
	
	if(owner == &youmonst){
		setworn(obj, W_ARM);
	} else if(owner && mask){
		owner->misc_worn_check |= mask;
		obj->owornmask |= mask;
		update_mon_intrinsics(owner, obj, TRUE, TRUE);
	}
}


/*
 *  Calculate the weight of the given object.  This will recursively follow
 *  and calculate the weight of any containers.
 *
 *  Note:  It is possible to end up with an incorrect weight if some part
 *	   of the code messes with a contained object and doesn't update the
 *	   container's weight.
 */
int
weight(obj)
register struct obj *obj;
{
	int wt = objects[obj->otyp].oc_weight;
	if(obj->oartifact == ART_ROD_OF_LORDLY_MIGHT) wt = objects[MACE].oc_weight;
	else if(obj->oartifact == ART_ANNULUS) wt = objects[BELL_OF_OPENING].oc_weight;
	else if(obj->oartifact == ART_SCEPTRE_OF_LOLTH) wt = 3*objects[MACE].oc_weight;
	else if(obj->oartifact == ART_ROD_OF_THE_ELVISH_LORDS) wt = objects[ELVEN_MACE].oc_weight;
	else if(obj->oartifact == ART_VAMPIRE_KILLER) wt = 2*objects[BULLWHIP].oc_weight;
	else if(obj->oartifact == ART_GOLDEN_SWORD_OF_Y_HA_TALLA) wt = 2*objects[SCIMITAR].oc_weight;
	else if(obj->oartifact == ART_AEGIS) wt = objects[LEATHER_CLOAK].oc_weight;
	else if(obj->oartifact == ART_HERMES_S_SANDALS) wt = objects[FLYING_BOOTS].oc_weight;
	else if(obj->oartifact == ART_EARTH_CRYSTAL){
		wt = 160;
	}
	else if(obj->oartifact == ART_WATER_CRYSTAL){
		wt = 120;
	}
	else if(obj->oartifact == ART_FIRE_CRYSTAL){
		wt = 40;
	}
	else if(obj->oartifact == ART_AIR_CRYSTAL){
		wt = 80;
	}
	else if(obj->oartifact == ART_BLACK_CRYSTAL){
		wt = 100;
	} //500 total
	else if(obj->oartifact == ART_DRAGON_PLATE || obj->oartifact == ART_CHROMATIC_DRAGON_SCALES){
		wt =  (int)(wt * 1.5); //225
	} else if(obj->oartifact == ART_TREASURY_OF_PROTEUS){
		wt =  50; /* Same as a crystal ball (ie, the Orb of Weight) */
	} else if(obj->obj_material != objects[obj->otyp].oc_material){
	//ie, for normal objects and non-special weight artifacts
	static const double matDensityLookup[] = {
//	FENCEPOST
		0.5,
//  LIQUID
		1.0,
//  WAX
		0.9,
//  VEGGY very aprox Alfalfa leaf?
		0.33, 
//  FLESH
		1.1,
//  PAPER
		1.2,
//  CLOTH cotton fibre
		1.5,
//  LEATHER
		0.9,
//  WOOD
		0.5,
//  BONE
		1.7,
//  DRAGON_HIDE
		3.4,
//  IRON
		7.9,
//  METAL		12	/* Sn, &c. */
		7.7,
//  COPPER		13	/* Cu - includes brass and bronze*/
		8.9,
//  SILVER		14	/* Ag */
		10.5,
//  GOLD		15	/* Au */
		19.3,
//  PLATINUM	16	/* Pt */
		21.4,
//  MITHRIL		17 alumninum
		2.7,
//  PLASTIC		18 High end estimate for density of old credit card plastic, http://dwb5.unl.edu/chem/smallscale/SmallScale-069.html
		1.3,
//  GLASS		19
		2.4,
//  GEMSTONE	20 Very rough aprox.
		3.6,
//  MINERAL		21 Very rough aprox.
		2.7,
//  OBSIDIAN_MT	22
		2.6,
//  SHADOWSTUFF	23
		1,
	};
		wt = wt*matDensityLookup[obj->obj_material]/matDensityLookup[objects[obj->otyp].oc_material];
	}
	
	if(obj->otyp == MOON_AXE && obj->oartifact != ART_SCEPTRE_OF_LOLTH){
		if(obj->ovar1) wt =  wt/4*obj->ovar1;
		else wt = wt/4;
	}

	if(obj->objsize != MZ_MEDIUM){
		int difsize = obj->objsize - MZ_MEDIUM;
		if(difsize > 0){
			difsize++;
			if(obj->oclass == ARMOR_CLASS || obj->oclass == WEAPON_CLASS) wt = wt*difsize;
			else wt = wt*difsize*difsize;
		} else {
			difsize = abs(difsize)+1;
			if(obj->oclass == ARMOR_CLASS || obj->oclass == WEAPON_CLASS) wt = wt/(difsize) + 1;
			else wt = wt/(difsize*difsize) + 1;
		}
	}
	
	if (obj->otyp == BOX && obj->spe){ /* Schroedinger's Cat */
		if(obj->spe == 1){
			wt += mons[PM_HOUSECAT].cwt;
		}else if(obj->spe == 4){
			wt += mons[PM_VAMPIRE].cwt;
		}
	}
	if ((Is_container(obj) && obj->otyp != MAGIC_CHEST) || obj->otyp == STATUE) {
		struct obj *contents;
		register int cwt = 0;

		if (obj->otyp == STATUE && obj->corpsenm >= LOW_PM)
		    wt = (int)obj->quan *
			 ((int)mons[obj->corpsenm].cwt * 3 / 2);

		if (obj->otyp == FOSSIL && obj->corpsenm >= LOW_PM)
		    wt = (int)obj->quan *
			 ((int)mons[obj->corpsenm].cwt * 1 / 2);

		for(contents=obj->cobj; contents; contents=contents->nobj)
			cwt += weight(contents);
		/*
		 *  The weight of bags of holding is calculated as the weight
		 *  of the bag plus the weight of the bag's contents modified
		 *  as follows:
		 *
		 *	Bag status	Weight of contents
		 *	----------	------------------
		 *	cursed			2x
		 *	blessed			x/4 + 1
		 *	otherwise		x/2 + 1
		 *
		 *  The macro DELTA_CWT in pickup.c also implements these
		 *  weight equations.
		 *
		 *  Note:  The above checks are performed in the given order.
		 *	   this means that if an object is both blessed and
		 *	   cursed (not supposed to happen), it will be treated
		 *	   as cursed.
		 */
		if (obj->otyp == BAG_OF_HOLDING)
		    cwt = obj->cursed ? (cwt * 2) :
					(1 + (cwt / (obj->blessed ? 4 : 2)));

		return wt + cwt;
	}
	if (obj->otyp == CORPSE && obj->corpsenm >= LOW_PM) {
		long long_wt = obj->quan * (long) mons[obj->corpsenm].cwt;

		wt = (long_wt > LARGEST_INT) ? LARGEST_INT : (int)long_wt;
		if (obj->oeaten) wt = eaten_stat(wt, obj);
		return wt;
	} else if (obj->oclass == FOOD_CLASS && obj->oeaten) {
		return eaten_stat((int)obj->quan * wt, obj);
	} else if (obj->oclass == COIN_CLASS)
		return (int)((obj->quan + 50L) / 100L);
	else if (obj->otyp == HEAVY_IRON_BALL && obj->owt != 0)
		return((int)(obj->owt));	/* kludge for "very" heavy iron ball */
	else if (obj->oartifact == ART_GREEN_DRAGON_CRESCENT_BLAD && obj->owt != 0)
		return((int)(obj->owt));	/* kludge for "very" heavy gdcb */
	return(wt ? wt*(int)obj->quan : ((int)obj->quan + 1)>>1);
}

static int treefruits[] = {APPLE,ORANGE,PEAR,BANANA,EUCALYPTUS_LEAF};

struct obj *
rnd_treefruit_at(x,y)
int x, y;
{
	struct obj *otmp;
	if(Is_zuggtmoy_level(&u.uz)){
		otmp = mksobj_at(SLIME_MOLD, x, y,TRUE,FALSE);
		otmp->spe = fruitadd("slime mold");
		return otmp;
	} else if(In_quest(&u.uz) && (Race_if(PM_DROW) || (Race_if(PM_DWARF) && Role_if(PM_NOBLEMAN)))){
		int chance = rn2(50);
		if(chance < 20){
			otmp = mksobj_at(EGG, x, y,TRUE,FALSE);
			otmp->corpsenm = PM_CAVE_SPIDER;
			return otmp;
		} else if(chance < 45){
			return mksobj_at(EUCALYPTUS_LEAF, x, y, TRUE, FALSE);
		} else {
			otmp = mksobj_at(SLIME_MOLD, x, y,TRUE,FALSE);
			otmp->spe = fruitadd("moldy remains");
			return otmp;
		}
	} else if((In_quest(&u.uz) && Role_if(PM_PIRATE)) || Is_paradise(&u.uz)){
		int chance = rn2(100);
		if(chance < 10){
			otmp = mksobj_at(SLIME_MOLD, x, y,TRUE,FALSE);
			otmp->spe = fruitadd("coconut");
			return otmp;
		} else if(chance < 25){
			return mksobj_at(BANANA, x, y, TRUE, FALSE);
		} else if(chance < 50){
			otmp = mksobj_at(SLIME_MOLD, x, y,TRUE,FALSE);
			otmp->spe = fruitadd("mango");
			return otmp;
		} else if(chance < 75){
			return mksobj_at(ORANGE, x, y, TRUE, FALSE);
		} else if(chance < 99){
			otmp = mksobj_at(EGG, x, y,TRUE,FALSE);
			otmp->corpsenm = PM_PARROT;
			return otmp;
		} else{
			otmp = mksobj_at(SLIME_MOLD, x, y,TRUE,FALSE);
			otmp->spe = fruitadd("tree squid");
			return otmp;
		}
	} else if(In_quest(&u.uz) && Role_if(PM_KNIGHT)){
		return mksobj_at(APPLE, x, y, TRUE, FALSE);
	}
	return mksobj_at(treefruits[rn2(SIZE(treefruits))], x, y, TRUE, FALSE);
}
#endif /* OVL0 */
#ifdef OVLB

struct obj *
mkgold(amount, x, y)
long amount;
int x, y;
{
    register struct obj *gold = g_at(x,y);

    if (amount <= 0L)
	amount = (long)(1 + rnd(level_difficulty()+2) * rnd(30));
    if (gold) {
	gold->quan += amount;
    } else {
	gold = mksobj_at(GOLD_PIECE, x, y, TRUE, FALSE);
	gold->quan = amount;
    }
    gold->owt = weight(gold);
    return (gold);
}

#endif /* OVLB */
#ifdef OVL1

/* return TRUE if the corpse has special timing */
#define special_corpse(num)  (((num) == PM_LIZARD)		\
				|| ((num) == PM_LICHEN)		\
				|| ((num) == PM_CROW_WINGED_HALF_DRAGON)		\
				|| ((num) == PM_BEHOLDER)		\
				|| (is_rider(&mons[num]))	\
				|| (mons[num].mlet == S_TROLL))

/*
 * OEXTRA note: Passing mtmp causes mtraits to be saved
 * even if ptr passed as well, but ptr is always used for
 * the corpse type (corpsenm). That allows the corpse type
 * to be different from the original monster,
 *	i.e.  vampire -> human corpse
 * yet still allow restoration of the original monster upon
 * resurrection.
 */
struct obj *
mkcorpstat(objtype, mtmp, ptr, x, y, init)
int objtype;	/* CORPSE or STATUE */
struct monst *mtmp;
struct permonst *ptr;
int x, y;
boolean init;
{
	register struct obj *otmp;

	if (objtype != CORPSE && objtype != STATUE)
	    impossible("making corpstat type %d", objtype);
	if (x == 0 && y == 0) {		/* special case - random placement */
		otmp = mksobj(objtype, init, FALSE);
		if (otmp) rloco(otmp);
	} else
		otmp = mksobj_at(objtype, x, y, init, FALSE);
	if (otmp) {
	    if (mtmp) {
		struct obj *otmp2;

		if (!ptr) ptr = mtmp->data;
		/* save_mtraits frees original data pointed to by otmp */
		otmp2 = save_mtraits(otmp, mtmp);
		if (otmp2) otmp = otmp2;
	    }
	    /* use the corpse or statue produced by mksobj() as-is
	       unless `ptr' is non-null */
	    if (ptr) {
			otmp->corpsenm = monsndx(ptr);
			otmp->owt = weight(otmp);
			// if (otmp->otyp == CORPSE &&
				// (special_corpse(old_corpsenm) ||
					// special_corpse(otmp->corpsenm))) {
			//Between molding and all the special effects, would be best to just reset timers for everything.
			if (otmp->otyp == CORPSE) {
				obj_stop_timers(otmp);
				start_corpse_timeout(otmp);
			}
	    }
	}
	return(otmp);
}

/*
 * Attach a monster id to an object, to provide
 * a lasting association between the two.
 */
struct obj *
obj_attach_mid(obj, mid)
struct obj *obj;
unsigned mid;
{
    struct obj *otmp;
    int lth, namelth;

    if (!mid || !obj) return (struct obj *)0;
    lth = sizeof(mid);
    namelth = obj->onamelth ? strlen(ONAME(obj)) + 1 : 0;
    if (namelth) 
	otmp = realloc_obj(obj, lth, (genericptr_t) &mid, namelth, ONAME(obj));
    else {
	otmp = obj;
	otmp->oxlth = sizeof(mid);
	(void) memcpy((genericptr_t)otmp->oextra, (genericptr_t)&mid,
								sizeof(mid));
    }
    if (otmp && otmp->oxlth) otmp->oattached = OATTACHED_M_ID;	/* mark it */
    return otmp;
}

static struct obj *
save_mtraits(obj, mtmp)
struct obj *obj;
struct monst *mtmp;
{
	struct obj *otmp;
	int lth, namelth;

	lth = sizeof(struct monst) + mtmp->mxlth + mtmp->mnamelth;
	namelth = obj->onamelth ? strlen(ONAME(obj)) + 1 : 0;
	otmp = realloc_obj(obj, lth, (genericptr_t) mtmp, namelth, ONAME(obj));
	if (otmp && otmp->oxlth) {
		struct monst *mtmp2 = (struct monst *)otmp->oextra;
		if (mtmp->data) mtmp2->mnum = monsndx(mtmp->data);
		/* invalidate pointers */
		/* m_id is needed to know if this is a revived quest leader */
		/* but m_id must be cleared when loading bones */
		mtmp2->nmon     = (struct monst *)0;
		mtmp2->data     = (struct permonst *)0;
		mtmp2->minvent  = (struct obj *)0;
		otmp->oattached = OATTACHED_MONST;	/* mark it */
	}
	return otmp;
}

/* returns a pointer to a new monst structure based on
 * the one contained within the obj.
 */
struct monst *
get_mtraits(obj, copyof)
struct obj *obj;
boolean copyof;
{
	struct monst *mtmp = (struct monst *)0;
	struct monst *mnew = (struct monst *)0;

	if (obj->oxlth && obj->oattached == OATTACHED_MONST)
		mtmp = (struct monst *)obj->oextra;
	if (mtmp) {
	    if (copyof) {
		int lth = mtmp->mxlth + mtmp->mnamelth;
		mnew = newmonst(lth);
		lth += sizeof(struct monst);
		(void) memcpy((genericptr_t)mnew,
				(genericptr_t)mtmp, lth);
	    } else {
	      /* Never insert this returned pointer into mon chains! */
	    	mnew = mtmp;
	    }
	}
	return mnew;
}

#endif /* OVL1 */
#ifdef OVLB

/* make an object named after someone listed in the scoreboard file */
struct obj *
mk_tt_object(objtype, x, y)
int objtype; /* CORPSE or STATUE */
register int x, y;
{
	register struct obj *otmp, *otmp2;
	boolean initialize_it;

	/* player statues never contain books */
	initialize_it = (objtype != STATUE);
	if ((otmp = mksobj_at(objtype, x, y, initialize_it, FALSE)) != 0) {
	    /* tt_oname will return null if the scoreboard is empty */
	    if ((otmp2 = tt_oname(otmp)) != 0) otmp = otmp2;
	}
	return(otmp);
}

/* make a new corpse or statue, uninitialized if a statue (i.e. no books) */
struct obj *
mk_named_object(objtype, ptr, x, y, nm)
int objtype;	/* CORPSE or STATUE */
struct permonst *ptr;
int x, y;
const char *nm;
{
	struct obj *otmp;

	otmp = mkcorpstat(objtype, (struct monst *)0, ptr,
				x, y, (boolean)(objtype != STATUE));
	if (nm)
		otmp = oname(otmp, nm);
	return(otmp);
}

boolean
is_flammable(otmp)
register struct obj *otmp;
{
	int otyp = otmp->otyp;
	int omat = otmp->obj_material;

	if (objects[otyp].oc_oprop == FIRE_RES || otyp == WAN_FIRE)
		return FALSE;

	return((boolean)((omat <= BONE && omat != LIQUID) || omat == PLASTIC));
}

boolean
is_rottable(otmp)
register struct obj *otmp;
{
	int otyp = otmp->otyp;

	return((boolean)(otmp->obj_material <= WOOD &&
			otmp->obj_material != LIQUID));
}

#endif /* OVLB */
#ifdef OVL1

/*
 * These routines maintain the single-linked lists headed in level.objects[][]
 * and threaded through the nexthere fields in the object-instance structure.
 */

/* put the object at the given location */
void
place_object(otmp, x, y)
register struct obj *otmp;
int x, y;
{
    register struct obj *otmp2 = level.objects[x][y];

    if (otmp->where != OBJ_FREE)
	panic("place_object: obj not free");

    obj_no_longer_held(otmp);
    if (is_boulder(otmp)) block_point(x,y);	/* vision */

    /* obj goes under boulders */
    if (otmp2 && is_boulder(otmp2)) {
	otmp->nexthere = otmp2->nexthere;
	otmp2->nexthere = otmp;
    } else {
	otmp->nexthere = otmp2;
	level.objects[x][y] = otmp;
    }

    /* set the new object's location */
    otmp->ox = x;
    otmp->oy = y;

    otmp->where = OBJ_FLOOR;

    /* add to floor chain */
    otmp->nobj = fobj;
    fobj = otmp;
    if (otmp->timed) obj_timer_checks(otmp, x, y, 0);
}

#define ON_ICE(a) ((a)->recharged)
#define ROT_ICE_ADJUSTMENT 2	/* rotting on ice takes 2 times as long */

/* If ice was affecting any objects correct that now
 * Also used for starting ice effects too. [zap.c]
 */
void
obj_ice_effects(x, y, do_buried)
int x, y;
boolean do_buried;
{
	struct obj *otmp;

	for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere) {
		if (otmp->timed) obj_timer_checks(otmp, x, y, 0);
	}
	if (do_buried) {
	    for (otmp = level.buriedobjlist; otmp; otmp = otmp->nobj) {
 		if (otmp->ox == x && otmp->oy == y) {
			if (otmp->timed) obj_timer_checks(otmp, x, y, 0);
		}
	    }
	}
}

/*
 * Returns an obj->age for a corpse object on ice, that would be the
 * actual obj->age if the corpse had just been lifted from the ice.
 * This is useful when just using obj->age in a check or calculation because
 * rot timers pertaining to the object don't have to be stopped and
 * restarted etc.
 */
long
peek_at_iced_corpse_age(otmp)
struct obj *otmp;
{
    long age, retval = otmp->age;
    
    if (otmp->otyp == CORPSE && ON_ICE(otmp)) {
	/* Adjust the age; must be same as obj_timer_checks() for off ice*/
	age = monstermoves - otmp->age;
	retval = otmp->age + (age / ROT_ICE_ADJUSTMENT);
#ifdef DEBUG_EFFECTS
	pline_The("%s age has ice modifications:otmp->age = %ld, returning %ld.",
		s_suffix(doname(otmp)),otmp->age, retval);
	pline("Effective age of corpse: %ld.",
		monstermoves - retval);
#endif
    }
    return retval;
}

STATIC_OVL void
obj_timer_checks(otmp, x, y, force)
struct obj *otmp;
xchar x, y;
int force;	/* 0 = no force so do checks, <0 = force off, >0 force on */
{
    long tleft = 0L;
    short action = ROT_CORPSE;
    boolean restart_timer = FALSE;
    boolean on_floor = (otmp->where == OBJ_FLOOR);
    boolean buried = (otmp->where == OBJ_BURIED);

    /* Check for corpses just placed on or in ice */
    if (otmp->otyp == CORPSE && (on_floor || buried) && is_ice(x,y)) {
		tleft = stop_timer(action, (genericptr_t)otmp);
		if (tleft == 0L) {
			action = REVIVE_MON;
			tleft = stop_timer(action, (genericptr_t)otmp);
		} 
		if (tleft != 0L) {
			long age;
			
			tleft = tleft - monstermoves;
			/* mark the corpse as being on ice */
			ON_ICE(otmp) = 1;
#ifdef DEBUG_EFFECTS
			pline("%s is now on ice at %d,%d.", The(xname(otmp)),x,y);
#endif
			/* Adjust the time remaining */
			tleft *= ROT_ICE_ADJUSTMENT;
			restart_timer = TRUE;
			/* Adjust the age; must be same as in obj_ice_age() */
			age = monstermoves - otmp->age;
			otmp->age = monstermoves - (age * ROT_ICE_ADJUSTMENT);
		}
    }
    /* Check for corpses coming off ice */
    else if ((force < 0) ||
	     (otmp->otyp == CORPSE && ON_ICE(otmp) &&
	     ((on_floor && !is_ice(x,y)) || !on_floor))) {
		tleft = stop_timer(action, (genericptr_t)otmp);
		if (tleft == 0L) {
			action = REVIVE_MON;
			tleft = stop_timer(action, (genericptr_t)otmp);
		}
		if (tleft != 0L) {
			long age;

			tleft = tleft - monstermoves;
			ON_ICE(otmp) = 0;
#ifdef DEBUG_EFFECTS
				pline("%s is no longer on ice at %d,%d.", The(xname(otmp)),x,y);
#endif
			/* Adjust the remaining time */
			tleft /= ROT_ICE_ADJUSTMENT;
			restart_timer = TRUE;
			/* Adjust the age */
			age = monstermoves - otmp->age;
			otmp->age = otmp->age + (age / ROT_ICE_ADJUSTMENT);
		}
    }
    /* now re-start the timer with the appropriate modifications */ 
    if (restart_timer)
	(void) start_timer(tleft, TIMER_OBJECT, action, (genericptr_t)otmp);
}

#undef ON_ICE
#undef ROT_ICE_ADJUSTMENT

void
remove_object(otmp)
register struct obj *otmp;
{
    xchar x = otmp->ox;
    xchar y = otmp->oy;

    if (otmp->where != OBJ_FLOOR)
	panic("remove_object: obj not on floor");
    if (is_boulder(otmp)) unblock_point(x,y); /* vision */
    extract_nexthere(otmp, &level.objects[x][y]);
    extract_nobj(otmp, &fobj);
    if (otmp->timed) obj_timer_checks(otmp,x,y,0);
}

/* throw away all of a monster's inventory */
void
discard_minvent(mtmp)
struct monst *mtmp;
{
    struct obj *otmp;

    while ((otmp = mtmp->minvent) != 0) {
	obj_extract_self(otmp);
	obfree(otmp, (struct obj *)0);	/* dealloc_obj() isn't sufficient */
    }
}

/*
 * Free obj from whatever list it is on in preperation of deleting it or
 * moving it elsewhere.  This will perform all high-level consequences
 * involved with removing the item.  E.g. if the object is in the hero's
 * inventory and confers heat resistance, the hero will lose it.
 *
 * Object positions:
 *	OBJ_FREE	not on any list
 *	OBJ_FLOOR	fobj, level.locations[][] chains (use remove_object)
 *	OBJ_CONTAINED	cobj chain of container object
 *	OBJ_INVENT	hero's invent chain (use freeinv)
 *	OBJ_MINVENT	monster's invent chain
 *	OBJ_MIGRATING	migrating chain
 *	OBJ_BURIED	level.buriedobjs chain
 *	OBJ_ONBILL	on billobjs chain
 *	OBJ_MAGIC_CHEST	magic chest chains
 */
void
obj_extract_self(obj)
    struct obj *obj;
{
    switch (obj->where) {
	case OBJ_FREE:
	    break;
	case OBJ_FLOOR:
	    remove_object(obj);
	    break;
	case OBJ_CONTAINED:
	    extract_nobj(obj, &obj->ocontainer->cobj);
	    container_weight(obj->ocontainer);
	    break;
	case OBJ_INVENT:
	    freeinv(obj);
	    break;
	case OBJ_MINVENT:
	    extract_nobj(obj, &obj->ocarry->minvent);
	    break;
	case OBJ_MIGRATING:
	    extract_nobj(obj, &migrating_objs);
	    break;
	case OBJ_BURIED:
	    extract_nobj(obj, &level.buriedobjlist);
	    break;
	case OBJ_ONBILL:
	    extract_nobj(obj, &billobjs);
	    break;
	case OBJ_MAGIC_CHEST:
	    extract_magic_chest_nobj(obj);
	    break;
	default:
	    panic("obj_extract_self");
	    break;
    }
}


/* Extract the given object from the chain, following nobj chain. */
void
extract_nobj(obj, head_ptr)
    struct obj *obj, **head_ptr;
{
    struct obj *curr, *prev;

    curr = *head_ptr;
    for (prev = (struct obj *) 0; curr; prev = curr, curr = curr->nobj) {
	if (curr == obj) {
	    if (prev)
		prev->nobj = curr->nobj;
	    else
		*head_ptr = curr->nobj;
	    break;
	}
    }
    if (!curr){
		panic("extract_nobj: object lost");
	}
    obj->where = OBJ_FREE;
}


/* Extract the given object from the magic_chest. */
void extract_magic_chest_nobj(struct obj *obj)
{
	int i;
    struct obj *curr, *prev, **head_ptr;

	for(i=0;i<10;i++){
		curr = magic_chest_objs[i];
		head_ptr = &(magic_chest_objs[i]);
		for (prev = NULL; curr; prev = curr, curr = curr->nobj) {
			if (curr == obj) {
				if (prev)
				prev->nobj = curr->nobj;
				else
				*head_ptr = curr->nobj;
				i=11;//break outer loop too
				break;
			}
		}
	}
    if (!curr) panic("extract_nobj: object lost");
    obj->where = OBJ_FREE;
}


/*
 * Extract the given object from the chain, following nexthere chain.
 *
 * This does not set obj->where, this function is expected to be called
 * in tandem with extract_nobj, which does set it.
 */
void
extract_nexthere(obj, head_ptr)
    struct obj *obj, **head_ptr;
{
    struct obj *curr, *prev;

    curr = *head_ptr;
    for (prev = (struct obj *) 0; curr; prev = curr, curr = curr->nexthere) {
	if (curr == obj) {
	    if (prev)
		prev->nexthere = curr->nexthere;
	    else
		*head_ptr = curr->nexthere;
	    break;
	}
    }
    if (!curr) panic("extract_nexthere: object lost");
}


/*
 * Add obj to mon's inventory.  If obj is able to merge with something already
 * in the inventory, then the passed obj is deleted and 1 is returned.
 * Otherwise 0 is returned.
 */
int
add_to_minv(mon, obj)
    struct monst *mon;
    struct obj *obj;
{
    struct obj *otmp;

    if (obj->where != OBJ_FREE)
	panic("add_to_minv: obj not free");

	if(mon->data == &mons[PM_MAID] && maid_clean(mon, obj) ) return 1; /*destroyed by maid*/

    /* merge if possible */
    for (otmp = mon->minvent; otmp; otmp = otmp->nobj)
	if (merged(&otmp, &obj))
	    return 1;	/* obj merged and then free'd */
    /* else insert; don't bother forcing it to end of chain */
    obj->where = OBJ_MINVENT;
    obj->ocarry = mon;
    obj->nobj = mon->minvent;
    mon->minvent = obj;
    return 0;	/* obj on mon's inventory chain */
}

/*
 * A maid is evaluating and repairing the object.
 * If she decides to throw it out, useup is called and 1 is returned.
 */
int
maid_clean(mon, obj)
    struct monst *mon;
    struct obj *obj;
{
	if(objects[obj->otyp].oc_unique || obj->oartifact == ART_PEN_OF_THE_VOID)
		return 0;
	if(obj->oeroded){
		if( d(1,20) < (int)is_rustprone(obj) ? (int)obj->oeroded : ((int)obj->oeroded) * 4){
			if(canseemon(mon)) pline("The maid breaks the %s trash down for parts.", is_rustprone(obj) ? "rusted-out" : "burned-out");
			obj->quan = 0;
			obfree(obj, (struct obj *)0);
			return 1;
		}
		obj->oeroded = 0;
		if(canseemon(mon)) pline("The maid %s", is_rustprone(obj) ? "scourers off the rust." : "patches the burned areas.");
	}
	if(obj->oeroded2){
		if( d(1,12) < (int)is_corrodeable(obj) ? (int)obj->oeroded2 : ((int)obj->oeroded2) * 4){
			if(canseemon(mon)) pline("The maid breaks the %s trash down for parts.", is_corrodeable(obj) ? "corroded" : "rotten");
			obj->quan = 0;
			obfree(obj, (struct obj *)0);
			return 1;
		}
		obj->oeroded2 = 0;
		if(canseemon(mon)) pline("The maid %s", is_corrodeable(obj) ? "scourers away the corrosion." : "excises and patches the rotted areas.");
	}
	if(obj->obroken){
		if(canseemon(mon)) pline("The maid mends the broken lock.");
		obj->obroken = 0;
	}
	if(obj->cursed){
		if( d(1,20) < 5){
			struct engr *oep = engr_at(mon->mx,mon->my);
			if(canseemon(mon)) pline("The maid banishes the unholy object to hell.");
			add_to_migration(obj);
			obj->ox = sanctum_level.dnum;
			obj->oy = sanctum_level.dlevel - 1; /* vs level, bottom of the accesible part of hell */
			obj->owornmask = (long)MIGR_RANDOM;
			if(!oep){
				make_engr_at(mon->mx, mon->my,
			     "U G I EI A", 0L, MARK);
				oep = engr_at(mon->mx,mon->my);
			}
			oep->ward_id = PENTAGRAM;
			oep->halu_ward = 0;
			oep->ward_type = MARK;
			oep->complete_wards = 1;
			return 1;
		}
		if(canseemon(mon)) pline("The maid sticks an ofuda to the offending object.");
		obj->cursed = 0;
	}
	if(obj->otyp == DWARVISH_HELM || obj->otyp == OIL_LAMP || obj->otyp == BRASS_LANTERN){
		if(obj->age < 750){
			obj->age += 750;
			if(canseemon(mon)) pline("The maid adds some oil.");
		}
	}
#ifdef TOURIST
	if(obj->otyp == EXPENSIVE_CAMERA){
		if (obj->spe <= 50){
			obj->spe = 50+rn1(16,10);		/* 10..25 */
			if(canseemon(mon)) pline("The maid replaces the film.");
		}
	}
#endif
	return 0;
}

/*
 * Place obj in a magic chest, make sure "obj" is free.
 * Returns (merged) object.
 * The input obj may be deleted in the process.
 * Based on the implementation of add_to_container.
 */
struct obj *add_to_magic_chest(struct obj *obj,int key)
{
    struct obj *otmp;

    if (obj->where != OBJ_FREE) {
	panic("add_to_magic_chest: obj not free (%d,%d,%d)",
	      obj->where, obj->otyp, obj->invlet);
    }

    /* merge if possible */
    for (otmp = magic_chest_objs[key]; otmp; otmp = otmp->nobj) if (merged(&otmp, &obj)) return otmp;

    obj->where = OBJ_MAGIC_CHEST;
    obj->nobj = magic_chest_objs[key];
    magic_chest_objs[key] = obj;
    return obj;
}

/*
 * Add obj to container, make sure obj is "free".  Returns (merged) obj.
 * The input obj may be deleted in the process.
 */
struct obj *
add_to_container(container, obj)
    struct obj *container, *obj;
{
    struct obj *otmp;

    if (obj->where != OBJ_FREE)
	panic("add_to_container: obj not free");
    if (container->where != OBJ_INVENT && container->where != OBJ_MINVENT)
	obj_no_longer_held(obj);

    /* merge if possible */
    for (otmp = container->cobj; otmp; otmp = otmp->nobj)
	if (merged(&otmp, &obj)) return (otmp);

    obj->where = OBJ_CONTAINED;
    obj->ocontainer = container;
    obj->nobj = container->cobj;
    container->cobj = obj;
    return (obj);
}

void
add_to_migration(obj)
    struct obj *obj;
{
    if (obj->where != OBJ_FREE)
	panic("add_to_migration: obj not free");

    obj->where = OBJ_MIGRATING;
    obj->nobj = migrating_objs;
    migrating_objs = obj;
}

void
add_to_buried(obj)
    struct obj *obj;
{
    if (obj->where != OBJ_FREE)
	panic("add_to_buried: obj not free");

    obj->where = OBJ_BURIED;
    obj->nobj = level.buriedobjlist;
    level.buriedobjlist = obj;
}

/* Recalculate the weight of this container and all of _its_ containers. */
STATIC_OVL void
container_weight(container)
    struct obj *container;
{
    container->owt = weight(container);
    if (container->where == OBJ_CONTAINED)
	container_weight(container->ocontainer);
/*
    else if (container->where == OBJ_INVENT)
	recalculate load delay here ???
*/
}

/*
 * Deallocate the object.  _All_ objects should be run through here for
 * them to be deallocated.
 */
void
dealloc_obj(obj)
    struct obj *obj;
{
    if (obj->where != OBJ_FREE)
	panic("dealloc_obj: obj not free");

    /* free up any timers attached to the object */
    if (obj->timed)
	obj_stop_timers(obj);

    /*
     * Free up any light sources attached to the object.
     *
     */
	del_light_source(LS_OBJECT, (genericptr_t) obj, TRUE);

    if (obj == thrownobj) thrownobj = (struct obj*)0;

    free((genericptr_t) obj);
}

#ifdef WIZARD
/* Check all object lists for consistency. */
void
obj_sanity_check()
{
    int x, y;
    struct obj *obj;
    struct monst *mon;
    const char *mesg;
    char obj_address[20], mon_address[20];  /* room for formatted pointers */

    mesg = "fobj sanity";
    for (obj = fobj; obj; obj = obj->nobj) {
	if (obj->where != OBJ_FLOOR) {
	    pline("%s obj %s %s@(%d,%d): %s\n", mesg,
		fmt_ptr((genericptr_t)obj, obj_address),
		where_name(obj->where),
		obj->ox, obj->oy, doname(obj));
	}
	check_contained(obj, mesg);
    }

    mesg = "location sanity";
    for (x = 0; x < COLNO; x++)
	for (y = 0; y < ROWNO; y++)
	    for (obj = level.objects[x][y]; obj; obj = obj->nexthere)
		if (obj->where != OBJ_FLOOR) {
		    pline("%s obj %s %s@(%d,%d): %s\n", mesg,
			fmt_ptr((genericptr_t)obj, obj_address),
			where_name(obj->where),
			obj->ox, obj->oy, doname(obj));
		}

    mesg = "invent sanity";
    for (obj = invent; obj; obj = obj->nobj) {
	if (obj->where != OBJ_INVENT) {
	    pline("%s obj %s %s: %s\n", mesg,
		fmt_ptr((genericptr_t)obj, obj_address),
		where_name(obj->where), doname(obj));
	}
	check_contained(obj, mesg);
    }

    mesg = "migrating sanity";
    for (obj = migrating_objs; obj; obj = obj->nobj) {
	if (obj->where != OBJ_MIGRATING) {
	    pline("%s obj %s %s: %s\n", mesg,
		fmt_ptr((genericptr_t)obj, obj_address),
		where_name(obj->where), doname(obj));
	}
	check_contained(obj, mesg);
    }

    mesg = "buried sanity";
    for (obj = level.buriedobjlist; obj; obj = obj->nobj) {
	if (obj->where != OBJ_BURIED) {
	    pline("%s obj %s %s: %s\n", mesg,
		fmt_ptr((genericptr_t)obj, obj_address),
		where_name(obj->where), doname(obj));
	}
	check_contained(obj, mesg);
    }

    mesg = "bill sanity";
    for (obj = billobjs; obj; obj = obj->nobj) {
	if (obj->where != OBJ_ONBILL) {
	    pline("%s obj %s %s: %s\n", mesg,
		fmt_ptr((genericptr_t)obj, obj_address),
		where_name(obj->where), doname(obj));
	}
	/* shouldn't be a full container on the bill */
	if (obj->cobj) {
	    pline("%s obj %s contains %s! %s\n", mesg,
		fmt_ptr((genericptr_t)obj, obj_address),
		something, doname(obj));
	}
    }

    mesg = "minvent sanity";
    for (mon = fmon; mon; mon = mon->nmon)
	for (obj = mon->minvent; obj; obj = obj->nobj) {
	    if (obj->where != OBJ_MINVENT) {
		pline("%s obj %s %s: %s\n", mesg,
			fmt_ptr((genericptr_t)obj, obj_address),
			where_name(obj->where), doname(obj));
	    }
	    if (obj->ocarry != mon) {
		pline("%s obj %s (%s) not held by mon %s (%s)\n", mesg,
			fmt_ptr((genericptr_t)obj, obj_address),
			doname(obj),
			fmt_ptr((genericptr_t)mon, mon_address),
			mon_nam(mon));
	    }
	    check_contained(obj, mesg);
	}
}

/* This must stay consistent with the defines in obj.h. */
static const char *obj_state_names[NOBJ_STATES] = {
	"free",		"floor",	"contained",	"invent",
	"minvent",	"migrating",	"buried",	"onbill"
};

STATIC_OVL const char *
where_name(where)
    int where;
{
    return (where<0 || where>=NOBJ_STATES) ? "unknown" : obj_state_names[where];
}

/* obj sanity check: check objs contained by container */
STATIC_OVL void
check_contained(container, mesg)
    struct obj *container;
    const char *mesg;
{
    struct obj *obj;
    char obj1_address[20], obj2_address[20];

    for (obj = container->cobj; obj; obj = obj->nobj) {
	if (obj->where != OBJ_CONTAINED)
	    pline("contained %s obj %s: %s\n", mesg,
		fmt_ptr((genericptr_t)obj, obj1_address),
		where_name(obj->where));
	else if (obj->ocontainer != container)
	    pline("%s obj %s not in container %s\n", mesg,
		fmt_ptr((genericptr_t)obj, obj1_address),
		fmt_ptr((genericptr_t)container, obj2_address));
    }
}
#endif /* WIZARD */

#endif /* OVL1 */

/*mkobj.c*/
