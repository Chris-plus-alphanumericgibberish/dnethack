/*	SCCS Id: @(#)artifact.c 3.4	2003/08/11	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#ifdef OVLB
#include "artilist.h"
#else
STATIC_DCL struct artifact artilist[];
#endif
/*
 * Note:  both artilist[] and artiexist[] have a dummy element #0,
 *	  so loops over them should normally start at #1.  The primary
 *	  exception is the save & restore code, which doesn't care about
 *	  the contents, just the total size.
 */

extern boolean notonhead;	/* for long worms */

#define get_artifact(o) \
		(((o)&&(o)->oartifact) ? &artilist[(int) ((unsigned char) (o)->oartifact)] : 0)


//duplicates of other functions, created due to problems with the linker
STATIC_DCL void NDECL(cast_protection);
STATIC_DCL int NDECL(throweffect);
STATIC_DCL void FDECL(awaken_monsters,(int));
STATIC_DCL void FDECL(do_earthquake,(int));

int FDECL(donecromenu, (const char *,struct obj *));
int FDECL(dopetmenu, (const char *,struct obj *));
int FDECL(dolordsmenu, (const char *,struct obj *));

static NEARDATA schar delay;		/* moves left for this spell */
static NEARDATA struct obj *artiptr;/* last/current artifact being used */
static NEARDATA int necro_effect;	/* necro effect picked */
static NEARDATA int lostname;	/* spirit # picked */

static NEARDATA	int oozes[12] = {0, PM_ACID_BLOB, PM_QUIVERING_BLOB, 
					  PM_GELATINOUS_CUBE, PM_DARKNESS_GIVEN_HUNGER, PM_GRAY_OOZE, 
					  PM_BROWN_PUDDING, PM_BLACK_PUDDING, PM_GREEN_SLIME,
					  PM_AOA, PM_BROWN_MOLD, PM_RED_MOLD};

static NEARDATA	int devils[16] = {0, PM_IMP, PM_LEMURE, PM_IMP,
					  PM_LEMURE, PM_IMP, PM_LEMURE,
					  PM_LEGION_DEVIL_GRUNT, PM_LEGION_DEVIL_SOLDIER, PM_LEGION_DEVIL_SERGEANT, 
					  PM_HORNED_DEVIL, PM_BARBED_DEVIL, PM_BONE_DEVIL,
					  PM_ICE_DEVIL, PM_PIT_FIEND, PM_HORNED_DEVIL};

static NEARDATA	int demons[16] = {0, PM_QUASIT, PM_MANES, PM_QUASIT,
					  PM_MANES, PM_QUASIT, PM_MANES, 
					  PM_SUCCUBUS, PM_INCUBUS, PM_VROCK, 
					  PM_HEZROU, PM_NALFESHNEE, PM_MARILITH,
					  PM_BALROG, PM_VROCK, PM_HEZROU};


STATIC_PTR int NDECL(read_necro);
STATIC_PTR int NDECL(read_lost);

STATIC_DCL int FDECL(spec_applies, (const struct artifact *,struct monst *));
STATIC_DCL int FDECL(arti_invoke, (struct obj*));
STATIC_DCL boolean FDECL(Mb_hit, (struct monst *magr,struct monst *mdef,
				  struct obj *,int *,int,BOOLEAN_P,char *,char *));
STATIC_DCL boolean FDECL(voidPen_hit, (struct monst *magr,struct monst *mdef,
				  struct obj *,int *,int,BOOLEAN_P,char *));

/* The amount added to the victim's total hit points to insure that the
   victim will be killed even after damage bonus/penalty adjustments.
   Most such penalties are small, and 200 is plenty; the exception is
   half physical damage.  3.3.1 and previous versions tried to use a very
   large number to account for this case; now, we just compute the fatal
   damage by adding it to 2 times the total hit points instead of 1 time.
   Note: this will still break if they have more than about half the number
   of hit points that will fit in a 15 bit integer. */
#define FATAL_DAMAGE_MODIFIER 200

#ifndef OVLB
STATIC_DCL int spec_dbon_applies;
STATIC_DCL xchar artidisco[NROFARTIFACTS];
#else	/* OVLB */
/* coordinate effects from spec_dbon() with messages in artifact_hit() */
STATIC_OVL int spec_dbon_applies = 0;

/* flags including which artifacts have already been created */
static boolean artiexist[1+NROFARTIFACTS+1];
/* and a discovery list for them (no dummy first entry here) */
STATIC_OVL xchar artidisco[NROFARTIFACTS];

STATIC_DCL void NDECL(hack_artifacts);
STATIC_DCL boolean FDECL(attacks, (int,struct obj *));


const int FIRE = 1;
const int COLD = 2;
const int ELEC = 5;  //copied from explode.c
const int elements[4] = {0, 1, 2, 5};
const int explType[4] = {0, EXPL_FIERY, EXPL_FROSTY, EXPL_MAGICAL};

/* handle some special cases; must be called after u_init() 
	Uh, it isn't, it's called BEFORE u_init. See allmain */
STATIC_OVL void
hack_artifacts()
{
	struct artifact *art;
	int alignmnt = aligns[flags.initalign].value;

	/* Fix up the alignments of "gift" artifacts */
	for (art = artilist+1; art->otyp; art++)
	    if (art->role == Role_switch && art->alignment != A_NONE)
		art->alignment = alignmnt;

	/* Excalibur can be used by any lawful character, not just knights */
	if (!Role_if(PM_KNIGHT)){
 	    artilist[ART_EXCALIBUR].role = NON_PM;
	    artilist[ART_CLARENT].role = NON_PM;
	}
	artilist[ART_MANTLE_OF_HEAVEN].otyp = find_cope();
	artilist[ART_VESTMENT_OF_HELL].otyp = find_opera_cloak();
	/* Fix up the quest artifact */
	if(Role_if(PM_NOBLEMAN) && Race_if(PM_VAMPIRE)){
		urole.questarti = ART_VESTMENT_OF_HELL;
	}
	if (Role_if(PM_MONK)) {
	    artilist[ART_GRANDMASTER_S_ROBE].alignment = alignmnt;
	}
	if (urole.questarti) {
	    artilist[urole.questarti].alignment = alignmnt;
	    artilist[urole.questarti].role = Role_switch;
	}
	artilist[ART_PEN_OF_THE_VOID].alignment = A_VOID; //something changed this??? Change it back.
	return;
}

/* zero out the artifact existence list */
void
init_artifacts()
{
	(void) memset((genericptr_t) artiexist, 0, sizeof artiexist);
	(void) memset((genericptr_t) artidisco, 0, sizeof artidisco);
	hack_artifacts();
}

void
save_artifacts(fd)
int fd;
{
	bwrite(fd, (genericptr_t) artiexist, sizeof artiexist);
	bwrite(fd, (genericptr_t) artidisco, sizeof artidisco);
}

void
restore_artifacts(fd)
int fd;
{
	mread(fd, (genericptr_t) artiexist, sizeof artiexist);
	mread(fd, (genericptr_t) artidisco, sizeof artidisco);
	hack_artifacts();	/* redo non-saved special cases */
}

const char *
artiname(artinum)
int artinum;
{
	if (artinum <= 0 || artinum > NROFARTIFACTS) return("");
	return(artilist[artinum].name);
}

/*
   Make an artifact.  If a specific alignment is specified, then an object of
   the appropriate alignment is created from scratch, or 0 is returned if
   none is available.  (If at least one aligned artifact has already been
   given, then unaligned ones also become eligible for this.)
   If no alignment is given, then 'otmp' is converted
   into an artifact of matching type, or returned as-is if that's not possible.
   For the 2nd case, caller should use ``obj = mk_artifact(obj, A_NONE);''
   for the 1st, ``obj = mk_artifact((struct obj *)0, some_alignment);''.
 */
struct obj *
mk_artifact(otmp, alignment)
struct obj *otmp;	/* existing object; ignored if alignment specified */
aligntyp alignment;	/* target alignment, or A_NONE */
{
	const struct artifact *a;
	int n, m;
	boolean by_align = (alignment != A_NONE);
	short o_typ = (by_align || !otmp) ? 0 : otmp->otyp;
	boolean unique = !by_align && otmp && objects[o_typ].oc_unique;
	short eligible[NROFARTIFACTS];

	/* gather eligible artifacts */
	for (n = 0, a = artilist+1, m = 1; a->otyp; a++, m++)
	    if ((!by_align ? artitypematch(a, otmp) :
		    (a->alignment == alignment ||
			(a->alignment == A_NONE && u.ugifts > 0))) &&
		(!(a->spfx & SPFX_NOGEN) || unique || (m==ART_PEN_OF_THE_VOID && Role_if(PM_EXILE))) && !artiexist[m]) {
			if (by_align && a->race != NON_PM && race_hostile(&mons[a->race]))
				continue;	/* skip enemies' equipment */
			else if (by_align && Role_if(a->role))
				goto make_artif;	/* 'a' points to the desired one */
			else if(by_align && Role_if(PM_PIRATE)) continue; /* pirates are not gifted artifacts */
			else
				eligible[n++] = m;
	}

	if (n) {		/* found at least one candidate */
	    m = eligible[rn2(n)];	/* [0..n-1] */
	    a = &artilist[m];

	    /* make an appropriate object if necessary, then christen it */
make_artif: 
		if (by_align){
			otmp = mksobj((int)a->otyp, TRUE, FALSE);
		}
	    otmp = oname(otmp, a->name);
	    otmp->oartifact = m;
	    artiexist[m] = TRUE;
	} else {
	    /* nothing appropriate could be found; return the original object */
	    if (by_align) otmp = 0;	/* (there was no original object) */
	}
	return otmp;
}

/*
 * Returns the full name (with articles and correct capitalization) of an
 * artifact named "name" if one exists, or NULL, it not.
 * The given name must be rather close to the real name for it to match.
 * The object type of the artifact is returned in otyp if the return value
 * is non-NULL.
 */
const char*
artifact_name(name, otyp)
const char *name;
short *otyp;
{
    register const struct artifact *a;
    register const char *aname;

    if(!strncmpi(name, "the ", 4)) name += 4;

    for (a = artilist+1; a->otyp; a++) {
	aname = a->name;
	if(!strncmpi(aname, "the ", 4)) aname += 4;
	if(!strcmpi(name, aname)) {
	    *otyp = a->otyp;
	    return a->name;
	}
    }

    return (char *)0;
}

boolean
exist_artifact(otyp, name)
register int otyp;
register const char *name;
{
	register const struct artifact *a;
	register boolean *arex;

	if (otyp && *name)
	    for (a = artilist+1,arex = artiexist+1; a->otyp; a++,arex++)
		if ((int) a->otyp == otyp && !strcmp(a->name, name))
		    return *arex;
	return FALSE;
}

/* I beleive this is the declaration that the artifact now exists.... */
void
artifact_exists(otmp, name, mod)
register struct obj *otmp;
register const char *name;
register boolean mod;
{
	register const struct artifact *a;

	if (otmp && *name)
	    for (a = artilist+1; a->otyp; a++)
		if (a->otyp == otmp->otyp && !strcmp(a->name, name)) {
		    register int m = a - artilist;
		    otmp->oartifact = (char)(mod ? m : 0);
		    otmp->age = 0;
		    if(otmp->otyp == RIN_INCREASE_DAMAGE) otmp->spe = 0;
		    artiexist[m] = mod;
			if(otmp->oartifact == ART_DRAGON_PLATE){
				otmp->owt = (int)(otmp->owt * 1.5); //450
			}
			if(otmp->oartifact == ART_EARTH_CRYSTAL){
				otmp->owt = (int)(otmp->owt * 2); //300
			}
			if(otmp->oartifact == ART_ROD_OF_SEVEN_PARTS){
				u.RoSPkills = 7;//number of hits untill you gain a +
				u.RoSPflights = 0;//number of flights remaining
			}
			if(get_artifact(otmp)->inv_prop == SPEED_BANKAI){
				u.ZangetsuSafe = u.ulevel;//turns for which you can use Zangetsu safely
			}
			if(get_artifact(otmp)->inv_prop == ICE_SHIKAI){
				u.SnSd1 = 0;//turn on which you can reuse the first dance
				u.SnSd2 = 0;//turn on which you can reuse the second dance
				u.SnSd3 = 0;//turn on which you can reuse the third dance
				u.SnSd3duration = 0;//turn until which the weapon does full damage
			}
			if(get_artifact(otmp)->inv_prop == NECRONOMICON || get_artifact(otmp)->inv_prop == SPIRITNAMES){
				otmp->ovar1 = 0;//ovar1 will be used to track special powers, via flags
				otmp->spestudied = 0;//use to track number of powers discovered
			} if(get_artifact(otmp)->inv_prop == INFINITESPELLS){
				otmp->ovar1 = rn2(SPE_BLANK_PAPER - SPE_DIG) + SPE_DIG;
			}
			if( arti_light(otmp) ){
				if (otmp->where == OBJ_FREE){
					place_object(otmp, u.ux, u.uy);  /* make it viable light source */
					begin_burn(otmp, FALSE);
					obj_extract_self(otmp);	 /* now release it for caller's use */
				}
				else begin_burn(otmp, FALSE);
			}
		    break;
		}
	return;
}

int
nartifact_exist()
{
    int a = 0;
    int n = SIZE(artiexist);

    while(n > 1)
	if(artiexist[--n] && (CountsAgainstGifts(n) /*|| a < 1*/)) a++;

    return max(a-u.regifted, 0);
}
#endif /* OVLB */
#ifdef OVL0

boolean
spec_ability(otmp, abil)
struct obj *otmp;
unsigned long abil;
{
	const struct artifact *arti = get_artifact(otmp);

	return( 
		(boolean)(arti && (arti->spfx & abil))
		);
}

boolean
spec_ability2(otmp, abil)
struct obj *otmp;
unsigned long abil;
{
	const struct artifact *arti = get_artifact(otmp);

	return( 
		(boolean)(arti && (arti->spfx2 & abil))
		);
}

boolean
spec_ability3(otmp, abil)
struct obj *otmp;
unsigned long abil;
{
	const struct artifact *arti = get_artifact(otmp);

	return( 
		(boolean)(arti && (arti->cspfx3 & abil))
		);
}

boolean
spec_wability(otmp, abil)
struct obj *otmp;
unsigned long abil;
{
	const struct artifact *arti = get_artifact(otmp);

	return( 
		(boolean)(arti && (arti->wpfx & abil))
		);
}

/* used so that callers don't need to known about SPFX_ codes */
boolean
confers_luck(obj)
struct obj *obj;
{
    /* might as well check for this too */
    if (obj && obj->otyp == LUCKSTONE) return TRUE;

    return (obj && obj->oartifact && spec_ability(obj, SPFX_LUCK));
}

/* used so that callers don't need to known about SPFX_ codes */
boolean
arti_digs(obj)
struct obj *obj;
{
    /* might as well check for this too */
    if (obj && (obj->oclass == WEAPON_CLASS || obj->oclass == TOOL_CLASS) && (objects[obj->otyp].oc_skill == P_PICK_AXE)) return TRUE;

    return (obj && obj->oartifact && spec_ability2(obj, SPFX2_DIG));
}


/* used so that callers don't need to known about SPFX_ codes */
boolean
arti_poisoned(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && ((spec_ability2(obj, SPFX2_POISONED)) || (obj->oartifact == ART_PEN_OF_THE_VOID && obj->ovar1&SEAL_YMIR)));
}

/* used so that callers don't need to known about SPFX_ codes */
boolean
arti_silvered(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && (spec_ability2(obj, SPFX2_SILVERED) || 
									  (obj->oartifact == ART_PEN_OF_THE_VOID &&
									   obj->ovar1 & SEAL_EDEN)));
}

/* used so that callers don't need to known about SPFX_ codes */
boolean
arti_bright(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && spec_ability2(obj, SPFX2_BRIGHT));
}

boolean
arti_shattering(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && spec_ability2(obj, SPFX2_SHATTER));
}

boolean
arti_disarm(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && spec_ability2(obj, SPFX2_DISARM));
}

boolean
arti_steal(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && spec_ability2(obj, SPFX2_STEAL));
}

boolean
arti_tentRod(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && spec_ability2(obj, SPFX2_TENTROD));
}

boolean
arti_shining(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && spec_ability2(obj, SPFX2_SHINING));
}

boolean
arti_mandala(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && spec_ability3(obj, SPFX3_MANDALA));
}

boolean
arti_lighten(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && spec_wability(obj, WSFX_LIGHTEN));
}

boolean
arti_chawis(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && spec_wability(obj, WSFX_WCATRIB));
}

boolean
arti_plussev(obj)
struct obj *obj;
{
    return (obj && obj->oartifact && spec_wability(obj, WSFX_PLUSSEV));
}

/* used to check whether a monster is getting reflection from an artifact */
boolean
arti_reflects(obj)
struct obj *obj;
{
    const struct artifact *arti = get_artifact(obj);

    if (arti) {      
	/* while being worn */
	if ((obj->owornmask & ~W_ART) && (arti->spfx & SPFX_REFLECT))
	    return TRUE;
	/* just being carried */
	if (arti->cspfx & SPFX_REFLECT) return TRUE;
    }
    return FALSE;
}

#endif /* OVL0 */
#ifdef OVLB

boolean
restrict_name(otmp, name)  /* returns 1 if name is restricted for otmp->otyp */
register struct obj *otmp;
register const char *name;
{
	register const struct artifact *a;
	register const char *aname;

	if (!*name) return FALSE;
	if (!strncmpi(name, "the ", 4)) name += 4;

		/* Since almost every artifact is SPFX_RESTR, it doesn't cost
		   us much to do the string comparison before the spfx check.
		   Bug fix:  don't name multiple elven daggers "Sting".
		 */
	for (a = artilist+1; a->otyp; a++) {
	    /* if (a->otyp != otmp->otyp) continue; */ //don't consider type anymore -CM
	    aname = a->name;
	    if (!strncmpi(aname, "the ", 4)) aname += 4;
	    if (!strcmp(aname, name))
			return ((boolean)( !is_nameable_artifact(a) ||
				otmp->quan > 1L));
	}

	return FALSE;
}

STATIC_OVL boolean
attacks(adtyp, otmp)
register int adtyp;
register struct obj *otmp;
{
	register const struct artifact *weap;

	if ((weap = get_artifact(otmp)) != 0)
		return((boolean)(weap->attk.adtyp == adtyp));
	return FALSE;
}

boolean
defends(adtyp, otmp)
register int adtyp;
register struct obj *otmp;
{
	register const struct artifact *weap;

	if ((weap = get_artifact(otmp)) != 0)
		return((boolean)(weap->defn.adtyp == adtyp));
	return FALSE;
}

/* used for monsters */
boolean
protects(adtyp, otmp)
int adtyp;
struct obj *otmp;
{
	register const struct artifact *weap;

	if ((weap = get_artifact(otmp)) != 0)
		return (boolean)(weap->cary.adtyp == adtyp);
	return FALSE;
}

/*
 * a potential artifact has just been worn/wielded/picked-up or
 * unworn/unwielded/dropped.  Pickup/drop only set/reset the W_ART mask.
 */
void
set_artifact_intrinsic(otmp,on,wp_mask)
register struct obj *otmp;
boolean on;
long wp_mask;
{
	long *mask = 0;
	register const struct artifact *oart = get_artifact(otmp);
	uchar dtyp;
	long spfx, spfx2, spfx3;

	if (!oart) return;

	/* effects from the defn field */
	dtyp = (wp_mask != W_ART) ? oart->defn.adtyp : oart->cary.adtyp;
	
	if (dtyp == AD_FIRE)
	    mask = &EFire_resistance;
	else if (dtyp == AD_COLD)
	    mask = &ECold_resistance;
	else if (dtyp == AD_ELEC)
	    mask = &EShock_resistance;
	else if (dtyp == AD_ACID)
	    mask = &EAcid_resistance;
	else if (dtyp == AD_MAGM)
	    mask = &EAntimagic;
	else if (dtyp == AD_DISN)
	    mask = &EDisint_resistance;
	else if (dtyp == AD_DRST)
	    mask = &EPoison_resistance;

	if (mask && wp_mask == W_ART && !on) {
	    /* find out if some other artifact also confers this intrinsic */
	    /* if so, leave the mask alone */
	    register struct obj* obj;
	    for(obj = invent; obj; obj = obj->nobj)
		if(obj != otmp && obj->oartifact) {
		    register const struct artifact *art = get_artifact(obj);
		    if(art->cary.adtyp == dtyp) {
			mask = (long *) 0;
			break;
		    }
		}
	}
	if (mask) {
	    if (on) *mask |= wp_mask;
	    else *mask &= ~wp_mask;
	}

	/* intrinsics from the spfx field; there could be more than one */
	spfx = (wp_mask != W_ART) ? oart->spfx : oart->cspfx;
	spfx2 = (wp_mask != W_ART) ? oart->spfx2 : 0;
	spfx3 = (wp_mask != W_ART) ? 0 : oart->cspfx3;
	if(spfx && wp_mask == W_ART && !on) {
	    /* don't change any spfx also conferred by other artifacts */
	    register struct obj* obj;
	    for(obj = invent; obj; obj = obj->nobj)
		if(obj != otmp && obj->oartifact) {
		    register const struct artifact *art = get_artifact(obj);
		    spfx &= ~art->cspfx;
		}
	}

	if (spfx & SPFX_SEARCH) {
	    if(on) ESearching |= wp_mask;
	    else ESearching &= ~wp_mask;
	}
	if (spfx & SPFX_HALRES) {
	    /* make_hallucinated must (re)set the mask itself to get
	     * the display right */
	    /* restoring needed because this is the only artifact intrinsic
	     * that can print a message--need to guard against being printed
	     * when restoring a game
	     */
	    (void) make_hallucinated((long)!on, restoring ? FALSE : TRUE, wp_mask);
	}
	if (spfx & SPFX_ESP) {
	    if(on) ETelepat |= wp_mask;
	    else ETelepat &= ~wp_mask;
	    see_monsters();
	}
	if (spfx & SPFX_DISPL) { //L's Patch
	    if (on) EDisplaced |= wp_mask;
	    else EDisplaced &= ~wp_mask;
	}
	if (spfx & SPFX_REGEN) {
	    if (on) ERegeneration |= wp_mask;
	    else ERegeneration &= ~wp_mask;
	}
	if (spfx & SPFX_TCTRL) {
	    if (on) ETeleport_control |= wp_mask;
	    else ETeleport_control &= ~wp_mask;
		if(otmp->oartifact == ART_SILVER_KEY){
			if (on) EPolymorph_control |= wp_mask;
		    else EPolymorph_control &= ~wp_mask;
		}
	}
	if (spfx & SPFX_WARN) {
	    if (spec_m2(otmp)) {
	    	if (on) {
			EWarn_of_mon |= wp_mask;
			flags.warntype |= spec_m2(otmp);
	    	} else {
			EWarn_of_mon &= ~wp_mask;
	    		flags.warntype &= ~spec_m2(otmp);
		}
		see_monsters();
	    } else {
		if (on) EWarning |= wp_mask;
	    	else EWarning &= ~wp_mask;
	    }
	}
	if (spfx & SPFX_EREGEN) {
	    if (on) EEnergy_regeneration |= wp_mask;
	    else EEnergy_regeneration &= ~wp_mask;
	}
	if (spfx & SPFX_HSPDAM) {
	    if (on) EHalf_spell_damage |= wp_mask;
	    else EHalf_spell_damage &= ~wp_mask;
	}
	if (spfx & SPFX_HPHDAM) {
	    if (on) EHalf_physical_damage |= wp_mask;
	    else EHalf_physical_damage &= ~wp_mask;
	}
	if ((spfx & SPFX_XRAY) && (otmp->oartifact != ART_AXE_OF_THE_DWARVISH_LORDS || Race_if(PM_DWARF))) {
	    /* this assumes that no one else is using xray_range */
	    if (on) u.xray_range = 3;
	    else u.xray_range = -1;
	    vision_full_recalc = 1;
	}
	if ((spfx & SPFX_REFLECT) && (wp_mask & W_WEP)) {
	    if (on) EReflecting |= wp_mask;
	    else EReflecting &= ~wp_mask;
	}
	if (spfx & SPFX_CONFL) {
		if (on) EConflict |= wp_mask;
		else EConflict &= ~wp_mask;
	}
	if (spfx & SPFX_AGGRM) {
		if (on) EAggravate_monster |= wp_mask;
		else EAggravate_monster &= ~wp_mask;
	}

	if (spfx2 & SPFX2_STLTH) {
	    if (on) EStealth |= wp_mask;
	    else EStealth &= ~wp_mask;
	}
	if (spfx2 & SPFX2_SPELLUP) {
	    if (on) ESpellboost |= wp_mask;
	    else ESpellboost &= ~wp_mask;
	}

	if (spfx3 & SPFX3_PCTRL) {
		if (on) EPolymorph_control |= wp_mask;
		else EPolymorph_control &= ~wp_mask;
	}

	if(wp_mask == W_ART && !on && oart->inv_prop) {
	    /* might have to turn off invoked power too */
	    if (oart->inv_prop <= LAST_PROP &&
		(u.uprops[oart->inv_prop].extrinsic & W_ARTI))
		(void) arti_invoke(otmp);
	}
}

/*
 * creature (usually player) tries to touch (pick up or wield) an artifact obj.
 * Returns 0 if the object refuses to be touched.
 * This routine does not change any object chains.
 * Ignores such things as gauntlets, assuming the artifact is not
 * fooled by such trappings.
 */
int
touch_artifact(obj,mon)
    struct obj *obj;
    struct monst *mon;
{
    register const struct artifact *oart = get_artifact(obj);
    boolean badclass, badalign, self_willed, yours;

    if(!oart) return 1;

    yours = (mon == &youmonst);
    /* all quest artifacts are self-willed; it this ever changes, `badclass'
       will have to be extended to explicitly include quest artifacts */
    self_willed = ((oart->spfx & SPFX_INTEL) != 0);
    if (yours) {
		if(oart->otyp != UNICORN_HORN){
			badclass = self_willed &&
			   ((oart->role != NON_PM && !Role_if(oart->role)) ||
			    (oart->race != NON_PM && !Race_if(oart->race)));
			badalign = (oart->spfx & SPFX_RESTR) && oart->alignment != A_NONE &&
			   (oart->alignment != u.ualign.type || u.ualign.record < 0);
			if(badalign && !badclass && self_willed && oart->role == NON_PM 
				&& oart->race == NON_PM) badclass = TRUE;
			if(oart == &artilist[ART_KUSANAGI_NO_TSURUGI] && !(u.ulevel >= 30 || u.uhave.amulet)){
				badclass = TRUE;
				badalign = TRUE;
			}
		}
		else {
			badclass = TRUE; //always get blasted by unicorn horns.  
							//They've been removed from the unicorn, after all -D_E
			badalign = (oart->spfx & SPFX_RESTR) && oart->alignment != A_NONE &&
			   (oart->alignment == -1*u.ualign.type); //Unicorn horns blast OPOSITE alignment alignment -D_E
														//Neutral horns blast NEUTRAL.
		}
    } else if (!is_covetous(mon->data) && !is_mplayer(mon->data)) {
		if(oart->otyp != UNICORN_HORN){
			badclass = self_willed &&
				   oart->role != NON_PM && !(oart == &artilist[ART_EXCALIBUR]
				   || oart == &artilist[ART_CLARENT]);
			badalign = (oart->spfx & SPFX_RESTR) && oart->alignment != A_NONE &&
			   (oart->alignment != sgn(mon->data->maligntyp));
		}
		else{/* Unicorn horns */
			badclass = TRUE;
			badalign = oart->alignment != A_NONE && (oart->alignment != sgn(mon->data->maligntyp));
		}
    } else {    /* an M3_WANTSxxx monster or a fake player */
		/* special monsters trying to take the Amulet, invocation tools or
		   quest item can touch anything except for `spec_applies' artifacts */
		badclass = badalign = FALSE;
    }
    /* weapons which attack specific categories of monsters are
       bad for them even if their alignments happen to match */
    if (!badalign && (oart->spfx & SPFX_DBONUS) != 0) {
		struct artifact tmp;
	
		tmp = *oart;
		tmp.spfx &= SPFX_DBONUS;
		badalign = !!spec_applies(&tmp, mon);
    }

	if(mon->data == &mons[PM_DAEMON]) badclass = badalign = FALSE;

	if (((badclass || badalign) && self_willed) ||
       (badalign && (!yours || !rn2(4)))
	   )  {
		int dmg;
		char buf[BUFSZ];
	
		if (!yours) return 0;
		You("are blasted by %s power!", s_suffix(the(xname(obj))));
		dmg = d((Antimagic ? 2 : 4), (self_willed ? 10 : 4));
		Sprintf(buf, "touching %s", oart->name);
		losehp(dmg, buf, KILLED_BY);
		exercise(A_WIS, FALSE);
    }

    /* can pick it up unless you're totally non-synch'd with the artifact */
    if (badclass && badalign && self_willed) {
		if (yours) pline("%s your grasp!", Tobjnam(obj, "evade"));
		return 0;
    } //Clarent sticks itself into it's surroundings if dropped
//	if (oart == &artilist[ART_CLARENT]  && (!yours || ) )
#ifdef CONVICT
    /* This is a kludge, but I'm not sure where else to put it */
    if (oart == &artilist[ART_IRON_BALL_OF_LIBERATION]) {
	if (Role_if(PM_CONVICT) && (!obj->oerodeproof)) {
	    obj->oerodeproof = TRUE;
	    obj->owt = 300; /* Magically lightened, but still heavy */
	}

	if (Punished && (obj != uball)) {
	    unpunish(); /* Remove a mundane heavy iron ball */
	}
    }
#endif /* CONVICT */

    return 1;
}

#endif /* OVLB */
#ifdef OVL1

/* decide whether an artifact's special attacks apply against mtmp */
STATIC_OVL int
spec_applies(weap, mtmp)
register const struct artifact *weap;
struct monst *mtmp;
{
	struct permonst *ptr;
	boolean yours;

	if(!mtmp) return FALSE; //Invoked with a null monster while calculating hypothetical data (I think)

	if(weap == &artilist[ART_PEN_OF_THE_VOID]) return quest_status.killed_nemesis;
	
	if(!(weap->spfx & (SPFX_DBONUS | SPFX_ATTK)))
	    return(weap->attk.adtyp == AD_PHYS);

	yours = (mtmp == &youmonst);
	ptr = mtmp->data;

	if (weap->spfx & SPFX_DMONS) {
	    return (ptr == &mons[(int)weap->mtype]);
	} else if (weap->spfx & SPFX_DCLAS) {
	    return (weap->mtype == (unsigned long)ptr->mlet);
	} else if (weap->spfx & SPFX_DFLAG1) {
	    return ((ptr->mflags1 & weap->mtype) != 0L);
	} else if (weap->spfx & SPFX_DFLAG2) {
	    return (
			(ptr->mflags2 & weap->mtype) || 
			(yours && 
				((!Upolyd && (urace.selfmask & weap->mtype)) ||
				((weap->mtype & M2_WERE) && u.ulycn >= LOW_PM))
			) ||
			(weap == &artilist[ART_STING] && ptr->mlet == S_SPIDER) 
		);
	} else if (weap->spfx & SPFX_DALIGN) {
	    return yours ? (u.ualign.type != weap->alignment) :
			   (ptr->maligntyp == A_NONE ||
				sgn(ptr->maligntyp) != weap->alignment);
	} else if (weap->spfx & SPFX_ATTK) {
	    struct obj *defending_weapon = (yours ? uwep : MON_WEP(mtmp));

	    if (defending_weapon && defending_weapon->oartifact &&
		    defends((int)weap->attk.adtyp, defending_weapon))
		return FALSE;
	    switch(weap->attk.adtyp) {
		case AD_FIRE:
			return !(yours ? Fire_resistance : resists_fire(mtmp));
		case AD_COLD:
			return !(yours ? Cold_resistance : resists_cold(mtmp));
		case AD_ELEC:
			return !(yours ? Shock_resistance : resists_elec(mtmp));
		case AD_ACID:
			return !(yours ? Acid_resistance : resists_acid(mtmp));
		case AD_MAGM:
		case AD_STUN:
			return !(yours ? Antimagic : (rn2(100) < ptr->mr));
		case AD_DRST:
			return !(yours ? Poison_resistance : resists_poison(mtmp));
		case AD_DRLI:
			return !(yours ? Drain_resistance : resists_drli(mtmp));
		case AD_STON:
			return !(yours ? Stone_resistance : resists_ston(mtmp));
		default:	impossible("Weird weapon special attack.");
	    }
	}
	return(0);
}

/* return the M2 flags of monster that an artifact's special attacks apply against */
long
spec_m2(otmp)
struct obj *otmp;
{
	register const struct artifact *artifact = get_artifact(otmp);
	if (artifact)
		return artifact->mtype;
	return 0L;
}

/* special attack bonus */
int
spec_abon(otmp, mon)
struct obj *otmp;
struct monst *mon;
{
	register const struct artifact *weap = get_artifact(otmp);
	/* no need for an extra check for `NO_ATTK' because this will
	   always return 0 for any artifact which has that attribute */
	if(Role_if(PM_PRIEST)) return weap->attk.damn; //priests always get the maximum to-hit bonus.

	if (weap && weap->attk.damn && spec_applies(weap, mon))
	    return rnd((int)weap->attk.damn);
	return 0;
}

/* special damage bonus */
int
spec_dbon(otmp, mon, tmp)
struct obj *otmp;
struct monst *mon;
int tmp;
{
	register const struct artifact *weap = get_artifact(otmp);

	if (!weap || (weap->attk.adtyp == AD_PHYS && /* check for `NO_ATTK' */
			weap->attk.damn == 0 && weap->attk.damd == 0) 
			|| (weap->inv_prop == ICE_SHIKAI && u.SnSd3duration < monstermoves))
	    spec_dbon_applies = FALSE;
	else{
	    spec_dbon_applies = spec_applies(weap, mon);
//		pline("dbon applies");
	}
	if (spec_dbon_applies)
	    return weap->attk.damd ? rnd((int)weap->attk.damd) : max(tmp,1);
	return 0;
}

/* add identified artifact to discoveries list */
void
discover_artifact(m)
xchar m;
{
    int i;

    /* look for this artifact in the discoveries list;
       if we hit an empty slot then it's not present, so add it */
    for (i = 0; i < NROFARTIFACTS; i++)
	if (artidisco[i] == 0 || artidisco[i] == m) {
	    artidisco[i] = m;
	    return;
	}
    /* there is one slot per artifact, so we should never reach the
       end without either finding the artifact or an empty slot... */
    impossible("couldn't discover artifact (%d)", (int)m);
}

/* used to decide whether an artifact has been fully identified */
boolean
undiscovered_artifact(m)
xchar m;
{
    int i;

    /* look for this artifact in the discoveries list;
       if we hit an empty slot then it's undiscovered */
    for (i = 0; i < NROFARTIFACTS; i++)
	if (artidisco[i] == m)
	    return FALSE;
	else if (artidisco[i] == 0)
	    break;
    return TRUE;
}

/* display a list of discovered artifacts; return their count */
int
disp_artifact_discoveries(tmpwin)
winid tmpwin;		/* supplied by dodiscover() */
{
    int i, m, otyp;
    char buf[BUFSZ];

    for (i = 0; i < NROFARTIFACTS; i++) {
	if (artidisco[i] == 0) break;	/* empty slot implies end of list */
	if (i == 0) putstr(tmpwin, iflags.menu_headings, "Artifacts");
	m = artidisco[i];
	otyp = artilist[m].otyp;
	Sprintf(buf, "  %s [%s %s]", artiname(m),
		align_str(artilist[m].alignment), simple_typename(otyp));
	putstr(tmpwin, 0, buf);
    }
    return i;
}

#endif /* OVL1 */

#ifdef OVLB


	/*
	 * Magicbane's intrinsic magic is incompatible with normal
	 * enchantment magic.  Thus, its effects have a negative
	 * dependence on spe.  Against low mr victims, it typically
	 * does "double athame" damage, 2d4.  Occasionally, it will
	 * cast unbalancing magic which effectively averages out to
	 * 4d4 damage (3d4 against high mr victims), for spe = 0.
	 *
	 * Prior to 3.4.1, the cancel (aka purge) effect always
	 * included the scare effect too; now it's one or the other.
	 * Likewise, the stun effect won't be combined with either
	 * of those two; it will be chosen separately or possibly
	 * used as a fallback when scare or cancel fails.
	 *
	 * [Historical note: a change to artifact_hit() for 3.4.0
	 * unintentionally made all of Magicbane's special effects
	 * be blocked if the defender successfully saved against a
	 * stun attack.  As of 3.4.1, those effects can occur but
	 * will be slightly less likely than they were in 3.3.x.]
	 */
#define MB_MAX_DIEROLL		8	/* rolls above this aren't magical */
static const char * const mb_verb[2][4] = {
	{ "probe", "stun", "scare", "cancel" },
	{ "prod", "amaze", "tickle", "purge" },
};
#define MB_INDEX_PROBE		0
#define MB_INDEX_STUN		1
#define MB_INDEX_SCARE		2
#define MB_INDEX_CANCEL		3

/* called when someone is being hit by Magicbane */
STATIC_OVL boolean
voidPen_hit(magr, mdef, pen, dmgptr, dieroll, vis, hittee)
struct monst *magr, *mdef;	/* attacker and defender */
struct obj *pen;			/* Pen of the Void */
int *dmgptr;			/* extra damage target will suffer */
int dieroll;			/* d20 that has already scored a hit */
boolean vis;			/* whether the action can be seen */
char *hittee;			/* target's name: "you" or mon_nam(mdef) */
{
    char buf[BUFSZ];
	boolean youdefend = mdef == &youmonst;
	boolean youattack = magr == &youmonst;
	boolean and = FALSE;
	int dnum = (Role_if(PM_EXILE) && quest_status.killed_nemesis) ? 2 : 1;
	
	buf[0] = '\0';
	if(u.voidChime){
		pline("The ringing blade hits %s.", hittee);
	}
	if (pen->ovar1&SEAL_AMON) {
	    if (vis){ 
			Sprintf(buf, "fiery");
			and = TRUE;
		}
	    if (!rn2(4)) (void) destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
	    if (!rn2(4)) (void) destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
	    if (!rn2(7)) (void) destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
	    if (youdefend && Slimed) burn_away_slime();
	}
	if (pen->ovar1&SEAL_BALAM) {
	    if (vis){ 
			and ? Sprintf(buf, "yet freezing") : Sprintf(buf, "freezing");
			and = TRUE;
		}
	    if (!rn2(4)) (void) destroy_mitem(mdef, POTION_CLASS, AD_COLD);
	}
	if (pen->ovar1&SEAL_ASTAROTH) {
	    if (vis){ 
			and ? Sprintf(buf, "and crackling") : Sprintf(buf, "crackling");
			and = TRUE;
		}
	    if (!rn2(5)) (void) destroy_mitem(mdef, RING_CLASS, AD_ELEC);
	    if (!rn2(5)) (void) destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
	}
	if (pen->ovar1&SEAL_ENKI) {
	    if (vis){ 
			and ? Sprintf(buf, "and dripping") : Sprintf(buf, "dripping");
			and = TRUE;
		}
		/*water damage?*/
	}
	if (pen->ovar1&SEAL_ECHIDNA) {
	    if (vis){ 
			and ? Sprintf(buf, "and sizzling") : Sprintf(buf, "sizzling");
			and = TRUE;
		}
	    if (!rn2(2)) (void) destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
	}
	
	if(vis && (and || (pen->ovar1&SEAL_FAFNIR))) pline("The %s%s blade hits %s.", !(pen->ovar1&SEAL_FAFNIR) ? "" : and ? "ruinous " : "ruinous", buf, hittee);
	
	if(pen->ovar1&SEAL_AHAZU && dieroll < 5){
		if(vis) pline("The blade's shadow catches on %s.", hittee);
		mdef->movement -= 3;
		and = TRUE;
	}
	if(pen->ovar1&SEAL_BUER){
		if(youattack) healup(d(dnum,4), 0, FALSE, FALSE);
		else{
			magr->mhp = min(magr->mhp + d(dnum,4),magr->mhpmax);
		}
	}
	if(pen->ovar1&SEAL_CHUPOCLOPS && dieroll < 3){
		struct trap *ttmp2 = maketrap(mdef->mx, mdef->my, WEB);
		if (ttmp2){
			if(youdefend){
				pline_The("webbing sticks to you. You're caught!");
				dotrap(ttmp2, NOWEBMSG);
#ifdef STEED
				if (u.usteed && u.utrap) {
				/* you, not steed, are trapped */
				dismount_steed(DISMOUNT_FELL);
				}
#endif
			}
			else mintrap(mdef);
		}
	}
	if(pen->ovar1&SEAL_DANTALION && dieroll < 3){
		if(youdefend) aggravate();
	    else probe_monster(mdef);
	}
	if(pen->ovar1&SEAL_SHIRO && dieroll < ACURR(A_CHA)){
		struct obj *otmp;
		otmp = mksobj(ROCK, TRUE, FALSE);
		otmp->blessed = 0;
		otmp->cursed = 0;
		set_destroy_thrown(1); //state variable referenced in drop_throw
			m_throw(magr, magr->mx, magr->my, mdef->mx-magr->mx, mdef->my-magr->my, 1, otmp,TRUE);
		set_destroy_thrown(0);  //state variable referenced in drop_throw
		if(mdef->mhp <= 0) return vis;//Monster was killed by throw and we should stop.
	}
	if(pen->ovar1&SEAL_MOTHER && dieroll < 2){
		if(youdefend) nomul(5,"held by the pen of the void");
	    else if(mdef->mcanmove || mdef->mfrozen){
			mdef->mcanmove = 0;
			mdef->mfrozen = max(mdef->mfrozen, 5);
		}
	}
	if(pen->ovar1&SEAL_ORTHOS && dieroll < 3){
		if(youdefend){
			You("are addled by the gusting winds!");
			make_stunned((HStun + 3), FALSE);
		} else {
			if(vis) pline("%s is thrown backwards by the gusting winds!",Monnam(mdef));
			if(bigmonst(mdef->data)) mhurtle(mdef, u.dx, u.dy, 1);
			else mhurtle(mdef, u.dx, u.dy, 10);
			if(mdef->mhp <= 0) return vis;//Monster was killed as part of movement and we should stop.
		}
		and = TRUE;
	}
	if(pen->ovar1&SEAL_OTIAX && dieroll < 2){
		if(youattack){
			struct obj *otmp2, **minvent_ptr;
			long unwornmask;

			/* Don't steal worn items, and downweight wielded items */
			if((otmp2 = mdef->minvent) != 0) {
				while(otmp2 && 
					  otmp2->owornmask&W_ARMOR && 
					  !( (otmp2->owornmask&W_WEP) && !rn2(10))
				) otmp2 = otmp2->nobj;
			}
			/* Still has handling for worn items, incase that changes */
			if(otmp2 != 0){
				int dx,dy;
				/* take the object away from the monster */
				if(otmp2->quan > 1L){
					otmp2 = splitobj(otmp2, 1L);
					obj_extract_self(otmp2); //wornmask is cleared by splitobj
				} else{
					obj_extract_self(otmp2);
					if ((unwornmask = otmp2->owornmask) != 0L) {
						mdef->misc_worn_check &= ~unwornmask;
						if (otmp2->owornmask & W_WEP) {
							setmnotwielded(mdef,otmp2);
							MON_NOWEP(mdef);
						}
						otmp2->owornmask = 0L;
						update_mon_intrinsics(mdef, otmp2, FALSE, FALSE);
					}
				}
				dx = rn2(3) - 1;
				dy = rn2(3) - 1;
				if(dx || dy){
					Your("winds toss %s away.",doname(otmp2));
					m_throw(&youmonst, mdef->mx, mdef->my, dx, dy, 1, otmp2,TRUE);
				}
				else{
					Your("winds drop %s at your feet.",doname(otmp2));
					(void) dropy(otmp2);
				}
				/* more take-away handling, after theft message */
				if (unwornmask & W_WEP) {		/* stole wielded weapon */
					possibly_unwield(mdef, FALSE);
				} else if (unwornmask & W_ARMG) {	/* stole worn gloves */
					mselftouch(mdef, (const char *)0, TRUE);
					if (mdef->mhp <= 0)	/* it's now a statue */
						return 1; /* monster is dead */
				}
			}
		} else if(youdefend){
			if(!(u.sealsActive&SEAL_ANDROMALIUS)){
				buf[0] = '\0';
				switch(steal(magr, buf,FALSE)){
				  case -1:
					return vis;
				  case 0:
				  break;
				  default:
					pline("%s steals %s.", Monnam(magr), buf);
				  break;
				}
			}
		} else {
			struct obj *otmp;
			for (otmp = mdef->minvent; otmp; otmp = otmp->nobj) if (!magr->mtame || !otmp->cursed) break;

			if (otmp) {
				char onambuf[BUFSZ], mdefnambuf[BUFSZ];

				/* make a special x_monnam() call that never omits
				   the saddle, and save it for later messages */
				Strcpy(mdefnambuf, x_monnam(mdef, ARTICLE_THE, (char *)0, 0, FALSE));

	#ifdef STEED
				if (u.usteed == mdef &&
						otmp == which_armor(mdef, W_SADDLE))
					/* "You can no longer ride <steed>." */
					dismount_steed(DISMOUNT_POLY);
	#endif
				obj_extract_self(otmp);
				if (otmp->owornmask) {
					mdef->misc_worn_check &= ~otmp->owornmask;
					if (otmp->owornmask & W_WEP)
						setmnotwielded(mdef,otmp);
					otmp->owornmask = 0L;
					update_mon_intrinsics(mdef, otmp, FALSE, FALSE);
				}
				/* add_to_minv() might free otmp [if it merges] */
				if (vis)
					Strcpy(onambuf, doname(otmp));
				(void) add_to_minv(magr, otmp);
				if (vis) {
					Strcpy(buf, Monnam(magr));
					pline("%s steals %s from %s!", buf,
						onambuf, mdefnambuf);
				}
				possibly_unwield(mdef, FALSE);
				mdef->mstrategy &= ~STRAT_WAITFORU;
				mselftouch(mdef, (const char *)0, FALSE);
				
				if (mdef->mhp <= 0) return vis;
			}
		}
	}
	if(pen->ovar1&SEAL_PAIMON){
		if(youattack) u.uen = min(u.uen+dnum,u.uenmax);
		else magr->mspec_used = max(magr->mspec_used - dnum,0);
	}
	if(pen->ovar1&SEAL_SIMURGH){
		if(youdefend && !Blind){
			You("are dazzled by prismatic feathers!");
			make_stunned((HStun + 5), FALSE);
		}
		else if(mdef->mcansee && haseyes(mdef->data)){
			pline("%s is dazzled by prismatic feathers!", Monnam(mdef));
			mdef->mstun = 1;
			mdef->mconf = 1;
		}
		and = TRUE;
	}
	if(pen->ovar1&SEAL_TENEBROUS && dieroll <= dnum){
		if(youdefend && !Blind){
			if (Blind)
				You_feel("an unholy blade drain your life!");
			else pline_The("unholy blade drains your life!");
			losexp("life drainage",TRUE,FALSE,FALSE);
		}
		else if(mdef->mcansee && haseyes(mdef->data)){
			if (vis) {
				pline_The("unholy blade draws the life from %s!",
				      mon_nam(mdef));
			}
			if (mdef->m_lev == 0) {
			    *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
			} else {
			    int drain = rnd(8);
			    *dmgptr += drain;
			    mdef->mhpmax -= drain;
			    mdef->m_lev--;
			    drain /= 2;
			}
		}
		and = TRUE;
	}
	return vis&&and;
}

STATIC_OVL boolean
Mb_hit(magr, mdef, mb, dmgptr, dieroll, vis, hittee, type)
struct monst *magr, *mdef;	/* attacker and defender */
struct obj *mb;			/* Magicbane */
int *dmgptr;			/* extra damage target will suffer */
int dieroll;			/* d20 that has already scored a hit */
boolean vis;			/* whether the action can be seen */
char *hittee;			/* target's name: "you" or mon_nam(mdef) */
char *type;			/* blade, staff, etc */
{
    struct permonst *old_uasmon;
    const char *verb;
    boolean youattack = (magr == &youmonst),
	    youdefend = (mdef == &youmonst),
	    resisted = FALSE, do_stun, do_confuse, result;
    int attack_indx, scare_dieroll = MB_MAX_DIEROLL / 2;

    result = FALSE;		/* no message given yet */
    /* the most severe effects are less likely at higher enchantment */
    if (mb->spe >= 3)
	scare_dieroll /= (1 << (mb->spe / 3));
    /* if target successfully resisted the artifact damage bonus,
       reduce overall likelihood of the assorted special effects */
    if (!spec_dbon_applies) dieroll += 1;

    /* might stun even when attempting a more severe effect, but
       in that case it will only happen if the other effect fails;
       extra damage will apply regardless; 3.4.1: sometimes might
       just probe even when it hasn't been enchanted */
    do_stun = (max(mb->spe,0) < rn2(spec_dbon_applies ? 11 : 7));

    /* the special effects also boost physical damage; increments are
       generally cumulative, but since the stun effect is based on a
       different criterium its damage might not be included; the base
       damage is either 1d4 (athame) or 2d4 (athame+spec_dbon) depending
       on target's resistance check against AD_STUN (handled by caller)
       [note that a successful save against AD_STUN doesn't actually
       prevent the target from ending up stunned] */
    attack_indx = MB_INDEX_PROBE;
    *dmgptr += rnd(4);			/* (2..3)d4 */
    if (do_stun) {
	attack_indx = MB_INDEX_STUN;
	*dmgptr += rnd(4);		/* (3..4)d4 */
    }
    if (dieroll <= scare_dieroll) {
	attack_indx = MB_INDEX_SCARE;
	*dmgptr += rnd(4);		/* (3..5)d4 */
    }
    if (dieroll <= (scare_dieroll / 2)) {
	attack_indx = MB_INDEX_CANCEL;
	*dmgptr += rnd(4);		/* (4..6)d4 */
    }

    /* give the hit message prior to inflicting the effects */
    verb = mb_verb[!!Hallucination][attack_indx];
    if (youattack || youdefend || vis) {
	result = TRUE;
	pline_The("magic-absorbing %s %s %s!",
		  type, vtense((const char *)0, verb), hittee);
	/* assume probing has some sort of noticeable feedback
	   even if it is being done by one monster to another */
	if (attack_indx == MB_INDEX_PROBE && !canspotmon(mdef))
	    map_invisible(mdef->mx, mdef->my);
    }

    /* now perform special effects */
    switch (attack_indx) {
    case MB_INDEX_CANCEL:
	old_uasmon = youmonst.data;
	/* No mdef->mcan check: even a cancelled monster can be polymorphed
	 * into a golem, and the "cancel" effect acts as if some magical
	 * energy remains in spellcasting defenders to be absorbed later.
	 */
	if (!cancel_monst(mdef, mb, youattack, FALSE, FALSE,0)) {
	    resisted = TRUE;
	} else {
	    do_stun = FALSE;
	    if (youdefend) {
		if (youmonst.data != old_uasmon)
		    *dmgptr = 0;    /* rehumanized, so no more damage */
		if (u.uenmax > 0) {
		    You("lose magical energy!");
		    u.uenmax--;
		    if (u.uen > 0) u.uen--;
		    flags.botl = 1;
		}
	    } else {
		if (mdef->data == &mons[PM_CLAY_GOLEM] || mdef->data == &mons[PM_SPELL_GOLEM])
		    mdef->mhp = 1;	/* cancelled clay golems will die */
		if (youattack && (attacktype(mdef->data, AT_MAGC) || attacktype(mdef->data, AT_MMGC))) {
		    You("absorb magical energy!");
		    u.uenmax++;
		    u.uen++;
		    flags.botl = 1;
		}
	    }
	}
	break;

    case MB_INDEX_SCARE:
	if (youdefend) {
	    if (Antimagic) {
		resisted = TRUE;
	    } else {
		nomul(-3, "being scared stiff");
		nomovemsg = "";
		if (magr && magr == u.ustuck && sticks(youmonst.data)) {
		    u.ustuck = (struct monst *)0;
		    You("release %s!", mon_nam(magr));
		}
	    }
	} else {
	    if (rn2(2) && resist(mdef, WEAPON_CLASS, 0, NOTELL))
		resisted = TRUE;
	    else
		monflee(mdef, 3, FALSE, (mdef->mhp > *dmgptr));
	}
	if (!resisted) do_stun = FALSE;
	break;

    case MB_INDEX_STUN:
	do_stun = TRUE;		/* (this is redundant...) */
	break;

    case MB_INDEX_PROBE:
	if (youattack && (mb->spe == 0 || !rn2(3 * abs(mb->spe)))) {
	    pline_The("%s is insightful.", verb);
	    /* pre-damage status */
	    probe_monster(mdef);
	}
	break;
    }
    /* stun if that was selected and a worse effect didn't occur */
    if (do_stun) {
	if (youdefend)
	    make_stunned((HStun + 3), FALSE);
	else
	    mdef->mstun = 1;
	/* avoid extra stun message below if we used mb_verb["stun"] above */
	if (attack_indx == MB_INDEX_STUN) do_stun = FALSE;
    }
    /* lastly, all this magic can be confusing... */
    do_confuse = !rn2(12);
    if (do_confuse) {
	if (youdefend)
	    make_confused(HConfusion + 4, FALSE);
	else
	    mdef->mconf = 1;
    }

    if (youattack || youdefend || vis) {
	(void) upstart(hittee);	/* capitalize */
	if (resisted) {
	    pline("%s %s!", hittee, vtense(hittee, "resist"));
	    shieldeff(youdefend ? u.ux : mdef->mx,
		      youdefend ? u.uy : mdef->my);
	}
	if ((do_stun || do_confuse) && flags.verbose) {
	    char buf[BUFSZ];

	    buf[0] = '\0';
	    if (do_stun) Strcat(buf, "stunned");
	    if (do_stun && do_confuse) Strcat(buf, " and ");
	    if (do_confuse) Strcat(buf, "confused");
	    pline("%s %s %s%c", hittee, vtense(hittee, "are"),
		  buf, (do_stun && do_confuse) ? '!' : '.');
	}
    }

    return result;
}
  
/* Function used when someone attacks someone else with an artifact
 * weapon.  Only adds the special (artifact) damage, and returns a 1 if it
 * did something special (in which case the caller won't print the normal
 * hit message).  This should be called once upon every artifact attack;
 * dmgval() no longer takes artifact bonuses into account.  Possible
 * extension: change the killer so that when an orc kills you with
 * Stormbringer it's "killed by Stormbringer" instead of "killed by an orc".
 */
boolean
artifact_hit(magr, mdef, otmp, dmgptr, dieroll)
struct monst *magr, *mdef;
struct obj *otmp;
int *dmgptr;
int dieroll; /* needed for Magicbane and vorpal blades */
{
	boolean youattack = (magr == &youmonst);
	boolean youdefend = (mdef == &youmonst);
	boolean vis = (!youattack && magr && cansee(magr->mx, magr->my))
	    || (!youdefend && cansee(mdef->mx, mdef->my))
	    || (youattack && u.uswallow && mdef == u.ustuck && !Blind);
	boolean realizes_damage;
	const char *wepdesc;
	static const char you[] = "you";
	char hittee[BUFSZ];

	Strcpy(hittee, youdefend ? you : mon_nam(mdef));

	/* The following takes care of most of the damage, but not all--
	 * the exception being for level draining, which is specially
	 * handled.  Messages are done in this function, however.
	 */
	*dmgptr += spec_dbon(otmp, mdef, *dmgptr);
	if (otmp->oartifact == ART_ICONOCLAST && mdef->data->mlet == S_ANGEL ) *dmgptr += 9;

	if(Role_if(PM_PRIEST)) *dmgptr += d(1,u.ulevel); //priests do extra damage when attacking with artifacts

	if (youattack && youdefend) {
	    impossible("attacking yourself with weapon?");
	    return FALSE;
	}

	realizes_damage = (youdefend || vis || 
			   /* feel the effect even if not seen */
			   (youattack && mdef == u.ustuck));
	if( (spec_ability2(otmp, SPFX2_RAM) && !rn2(4)) || 
		(spec_ability2(otmp, SPFX2_RAM2) && (otmp->oartifact != ART_TOBIUME || *dmgptr+6 >= mdef->mhp))
		){
		if(youattack){
			pline("%s is thrown backwards by the force of your blow!",Monnam(mdef));
			if(bigmonst(mdef->data)) mhurtle(mdef, u.dx, u.dy, 10);
			else mhurtle(mdef, u.dx, u.dy, 10);
			if(mdef->mhp <= 0) return TRUE;//Monster was killed as part of movement and we should stop.
		}
		else if(youdefend){
			You("are addled by the force of the blow!");
			make_stunned((HStun + 3), FALSE);
		}
	}
	if( (spec_ability2(otmp, SPFX2_FIRE) && !rn2(4)) || 
		(spec_ability2(otmp, SPFX2_FIRE2) && (otmp->oartifact != ART_TOBIUME || *dmgptr+6 >= mdef->mhp))
		){
		if(youattack){
			explode(mdef->mx, mdef->my,
				1, //1 = AD_FIRE, explode uses nonstandard damage type flags...
				d(6,6), 0,
				EXPL_FIERY);
		}
		else if(youdefend){
			explode(u.ux, u.uy,
				31, //1 = AD_FIRE, monster's attack explode uses nonstandard damage type flags...
				d(6,6), 0,
				EXPL_FIERY);
		}
		else{
			explode(mdef->mx, mdef->my,
				31, //1 = AD_FIRE, explode uses nonstandard damage type flags...
				d(6,6), 0,
				EXPL_FIERY);
		}
	}
	if( (spec_ability2(otmp, SPFX2_COLD) && !rn2(4)) || spec_ability2(otmp, SPFX2_COLD2)){
		if(youattack){
			explode(mdef->mx, mdef->my,
				2, //1 = AD_COLD, explode uses nonstandard damage type flags...
				d(6,6), 0,
				EXPL_FROSTY);
		}
		else if(youdefend){
			explode(u.ux, u.uy,
				32, //1 = AD_COLD, monster's attack explode uses nonstandard damage type flags...
				d(6,6), 0,
				EXPL_FROSTY);
		}
		else{
			explode(mdef->mx, mdef->my,
				32, //1 = AD_COLD, explode uses nonstandard damage type flags...
				d(6,6), 0,
				EXPL_FROSTY);
		}
	}
	if( (spec_ability2(otmp, SPFX2_ELEC) && !rn2(4)) || spec_ability2(otmp, SPFX2_ELEC2)){
		if(youattack){
			int deltax = mdef->mx-u.ux;
			int deltay = mdef->my-u.uy;
			if( (abs(deltax) > 1 || abs(deltay) > 1) /*&& otmp->oartifact == ART_RAMIEL)
				|| otmp->oartifact == ART_COLD_SOUL */
				){
				explode(mdef->mx, mdef->my,
					WAN_LIGHTNING - WAN_MAGIC_MISSILE,
					d(6,6), 0,
					EXPL_MAGICAL);
			}
			else{
				deltax = deltax > 1 ? 1 : deltax < -1 ? -1 : deltax;
				deltay = deltay > 1 ? 1 : deltay < -1 ? -1 : deltay;
				buzz(WAN_LIGHTNING - WAN_MAGIC_MISSILE,
				 6, u.ux, u.uy, deltax, deltay,0,0);
			}
		}
		else if(youdefend){
			int deltax = u.ux-magr->mx;
			int deltay = u.uy-magr->my;
			if((abs(deltax) > 1 || abs(deltay) > 1) && otmp->oartifact == ART_RAMIEL){
				explode(u.ux, u.uy,
					-30 - (WAN_LIGHTNING - WAN_MAGIC_MISSILE),
					d(6,6), 0,
					EXPL_MAGICAL);
			}
			else{
				deltax = deltax > 1 ? 1 : deltax < -1 ? -1 : deltax;
				deltay = deltay > 1 ? 1 : deltay < -1 ? -1 : deltay;
				buzz( -30 - (WAN_LIGHTNING - WAN_MAGIC_MISSILE),
			     6, u.ux, u.uy, deltax, deltay,0,0);
			}
		}
		else{
			int deltax = mdef->mx-magr->mx;
			int deltay = mdef->my-magr->my;
			if((abs(deltax) > 1 || abs(deltay) > 1) && otmp->oartifact == ART_RAMIEL){
				explode(mdef->mx, mdef->my,
					-30 - WAN_LIGHTNING - WAN_MAGIC_MISSILE,
					d(6,6), 0,
					EXPL_MAGICAL);
			}
			else{
				deltax = deltax > 1 ? 1 : deltax < -1 ? -1 : deltax;
				deltay = deltay > 1 ? 1 : deltay < -1 ? -1 : deltay;
				buzz( -30 - (WAN_LIGHTNING - WAN_MAGIC_MISSILE),
			     6, magr->mx, magr->mx, deltax, deltay,0,0);
			}
		}
	}
	if(mdef->mhp <= 0) return TRUE; //killed it.
	/* the four basic attacks: fire, cold, shock and missiles */
	if (attacks(AD_FIRE, otmp)) {
	    if (realizes_damage)
			pline_The("fiery %s %s %s%c", otmp->oartifact == ART_LIMB_OF_THE_BLACK_TREE ? "tree-branch" : "blade",
				!spec_dbon_applies ? "hits" :
				(mdef->data == &mons[PM_WATER_ELEMENTAL] || mdef->data == &mons[PM_LETHE_ELEMENTAL]) ?
				"vaporizes part of" : "burns",
				hittee, !spec_dbon_applies ? '.' : '!');
	    if (!rn2(4)) (void) destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
	    if (!rn2(4)) (void) destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
	    if (!rn2(7)) (void) destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
	    if (youdefend && Slimed) burn_away_slime();
	    return realizes_damage;
	}
	if (attacks(AD_COLD, otmp) && (get_artifact(otmp)->inv_prop != ICE_SHIKAI || u.SnSd3duration > monstermoves) ) {
	    if (realizes_damage)
			pline_The("ice-cold %s %s %s%c", otmp->oartifact == ART_LASH_OF_THE_COLD_WASTE ? "whip" : 
											 ((unsigned char) otmp->oartifact) == ART_SCEPTRE_OF_THE_FROZEN_FLOO ? "staff" : "blade",
				!spec_dbon_applies ? "hits" : "freezes",
				hittee, !spec_dbon_applies ? '.' : '!');
	    if (!rn2(4)) (void) destroy_mitem(mdef, POTION_CLASS, AD_COLD);
	    return realizes_damage;
	}
	if (attacks(AD_ELEC, otmp)) {
	    if (realizes_damage)
			pline_The("%s hits%s %s%c", otmp->oartifact == ART_CARESS ? "lashing whip" : 
										otmp->oartifact == ART_RAMIEL ? "thundering polearm" : 
										"massive hammer",
			  !spec_dbon_applies ? "" : "!  Lightning strikes",
			  hittee, !spec_dbon_applies ? '.' : '!');
	    if (!rn2(5)) (void) destroy_mitem(mdef, RING_CLASS, AD_ELEC);
	    if (!rn2(5)) (void) destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
	    return realizes_damage;
	}
	if (attacks(AD_MAGM, otmp)) {
	    if (realizes_damage)
		pline_The("imaginary widget hits%s %s%c",
			  !spec_dbon_applies ? "" :
				"!  A hail of magic missiles strikes",
			  hittee, !spec_dbon_applies ? '.' : '!');
	    return realizes_damage;
	}

	if (attacks(AD_ACID, otmp)) {
	    if (realizes_damage)
		pline_The("foul blade %s %s%c",
			!spec_dbon_applies ? "hits" :
			"burns", hittee, !spec_dbon_applies ? '.' : '!');
	    if (!rn2(2)) (void) destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
//	    if (!rn2(4)) (void) destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
//	    if (!rn2(7)) (void) destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
	    return realizes_damage;
	}
	if (attacks(AD_STUN, otmp) && dieroll <= MB_MAX_DIEROLL) {
	    /* Magicbane's special attacks (possibly modifies hittee[]) */
		return Mb_hit(magr, mdef, otmp, dmgptr, dieroll, vis, hittee, 
			otmp->oartifact == ART_STAFF_OF_THE_ARCHMAGI ? "staff" : "blade");
	}
	if(otmp->oartifact == ART_PEN_OF_THE_VOID){
		return voidPen_hit(magr, mdef, otmp, dmgptr, dieroll, vis, hittee);
	}
//	pline("D20 role was %d", dieroll);
//	pline("%d", otmp->oeaten);
	if (otmp->oartifact == ART_ROD_OF_SEVEN_PARTS ) {
		if(--u.RoSPkills < 1){ 
			if(otmp->spe < 7 && u.ulevel/3 > otmp->spe){
				otmp->spe++;
				pline("Your weapon has become more perfect!");
				u.RoSPkills = 7;
			}
			else u.RoSPkills=1;
		}
		if(sgn(mdef->data->maligntyp) < 0){
			*dmgptr += 7 + otmp->spe;
		}
	}
		//sunlight code adapted from Sporkhack
	if ((mdef->data == &mons[PM_GREMLIN]) && arti_bright(otmp)) { 
          wepdesc = artilist[otmp->oartifact].name;
			   /* Sunlight kills gremlins */
		  if (realizes_damage)
            pline("%s sunlight sears %s!", s_suffix(wepdesc), hittee);
  		otmp->dknown = TRUE;
        *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
        return TRUE;
	}
	if ( (mdef->data->mlet == S_TROLL) && 
		  arti_bright(otmp)) { 
          wepdesc = artilist[otmp->oartifact].name;
        /* Sunlight turns trolls to stone (Middle-earth) */
          if (mdef->data->mlet == S_TROLL &&
                   !munstone(mdef, TRUE)) {
              if (realizes_damage)
                 pline("%s sunlight strikes %s!", s_suffix(wepdesc), hittee);
              if (youdefend) instapetrify(wepdesc);
              else minstapetrify(mdef,youattack);
  				otmp->dknown = TRUE;
			   return TRUE;
		  }
	}
	if( (Hallucination && youattack) &&
			(is_covetous(mdef->data)) && 
			otmp->oartifact == ART_TROLLSBANE){
				You("flame the nasty troll!");  //trollsbane hits monsters that pop in to ruin your day.
				*dmgptr += d(2,20) + 2*otmp->spe; //boosts power better than demonbane hitting silver hating.
	}
    if ( spec_ability2(otmp, SPFX2_BLIND) && !resists_blnd(mdef) && !rn2(3)) {
          long rnd_tmp;
          wepdesc = "brilliant light";
           if (realizes_damage && mdef->mcansee)
                   pline_The("%s blinds %s!", wepdesc, hittee);
           rnd_tmp = rn2(5)+otmp->spe;
           if(rnd_tmp<2) rnd_tmp = 2;
           if(youdefend) {
                   make_blinded(Blinded+rnd_tmp, FALSE);
                   if (!Blind) Your(vision_clears);
           } else {
               if ((rnd_tmp += mdef->mblinded) > 127) rnd_tmp = 127;
               mdef->mblinded = rnd_tmp;
               mdef->mcansee = 0;
               mdef->mstrategy &= ~STRAT_WAITFORU;
           }
	}	
	if( spec_ability2(otmp, SPFX2_DEMIBANE)
			&& is_demihuman(mdef->data)  ){
			wepdesc = artilist[otmp->oartifact].name;
			if (realizes_damage)
				pline("%s sears %s!", wepdesc, hittee);
			*dmgptr += d(1,20);
	} 
	if(otmp->oartifact == ART_KUSANAGI_NO_TSURUGI){
		if (is_whirly(mdef->data)){
			wepdesc = "The winds";
			if (youattack && u.uswallow && mdef == u.ustuck) {
				pline("%s blow %s open!  It dissipates in the breeze.", wepdesc, mon_nam(mdef));
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				return TRUE;
			}
			else if (!youdefend) {
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				pline("%s blow %s apart!", wepdesc, mon_nam(mdef));
				otmp->dknown = TRUE;
				return TRUE;
			}
		}else if(flaming(mdef->data) && youattack){
				struct monst *mtmp = 0;
				pline("The winds fan the flames into a roaring inferno!");
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				otmp->dknown = TRUE;
				mtmp = makemon(&mons[PM_FIRE_VORTEX], u.ux, u.uy, MM_EDOG|MM_ADJACENTOK|NO_MINVENT|MM_NOCOUNTBIRTH);
				initedog(mtmp);
				mtmp->m_lev = u.ulevel / 2 + 1;
				mtmp->mhpmax = (mtmp->m_lev * 8) - 4;
				mtmp->mhp =  mtmp->mhpmax;
				mtmp->mtame = 10;
				mtmp->mpeaceful = 1;
				return TRUE;
		} else if(mdef->data == &mons[PM_EARTH_ELEMENTAL] && youattack){
				struct monst *mtmp = 0;
				pline("The winds blast the stone and sweep the fragments into a whirling dust storm!");
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				otmp->dknown = TRUE;
				mtmp = makemon(&mons[PM_DUST_VORTEX], u.ux, u.uy, MM_EDOG|MM_ADJACENTOK|NO_MINVENT|MM_NOCOUNTBIRTH);
				initedog(mtmp);
				mtmp->m_lev = u.ulevel / 2 + 1;
				mtmp->mhpmax = (mtmp->m_lev * 8) - 4;
				mtmp->mhp =  mtmp->mhpmax;
				mtmp->mtame = 10;
				mtmp->mpeaceful = 1;
				return TRUE;
		} else if(mdef->data == &mons[PM_WATER_ELEMENTAL] && youattack){
				struct monst *mtmp = 0;
				pline("The winds whip the waters into a rolling fog!");
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				otmp->dknown = TRUE;
				mtmp = makemon(&mons[PM_FOG_CLOUD], u.ux, u.uy, MM_EDOG|MM_ADJACENTOK|NO_MINVENT|MM_NOCOUNTBIRTH);
				initedog(mtmp);
				mtmp->m_lev = u.ulevel / 2 + 1;
				mtmp->mhpmax = (mtmp->m_lev * 8) - 4;
				mtmp->mhp =  mtmp->mhpmax;
				mtmp->mtame = 10;
				mtmp->mpeaceful = 1;
				return TRUE;
		}
	}
	if(arti_steal(otmp)){
	 if(youattack){
	  if(mdef->minvent && (Role_if(PM_PIRATE) || !rn2(10) ) ){
		struct obj *otmp2, **minvent_ptr;
		long unwornmask;

		/* Don't steal worn items, and downweight wielded items */
		if((otmp2 = mdef->minvent) != 0) {
			while(otmp2 && 
				  otmp2->owornmask&W_ARMOR && 
				  !( (otmp2->owornmask&W_WEP) && !rn2(10))
			) otmp2 = otmp2->nobj;
		}
		
		/* Still has handling for worn items, incase that changes */
		if(otmp2 != 0){
			/* take the object away from the monster */
			if(otmp2->quan > 1L){
				otmp2 = splitobj(otmp2, 1L);
				obj_extract_self(otmp2); //wornmask is cleared by splitobj
			} else{
			obj_extract_self(otmp2);
			if ((unwornmask = otmp2->owornmask) != 0L) {
				mdef->misc_worn_check &= ~unwornmask;
				if (otmp2->owornmask & W_WEP) {
					setmnotwielded(mdef,otmp2);
					MON_NOWEP(mdef);
				}
				otmp2->owornmask = 0L;
				update_mon_intrinsics(mdef, otmp2, FALSE, FALSE);
			}
			}
			/* Ask the player if they want to keep the object */
			pline("Your blade sweaps %s away from %s", doname(otmp2), mon_nam(mdef));
			if(yn("Do you try to grab it for yourself?") == 'y'){
			/* give the object to the character */
			otmp2 = Role_if(PM_PIRATE) ? 
				hold_another_object(otmp2, "Ye snatched but dropped %s.",
						   doname(otmp2), "Ye steal: ") :
				hold_another_object(otmp2, "You snatched but dropped %s.",
						   doname(otmp2), "You steal: ");
			if (otmp2->otyp == CORPSE &&
				touch_petrifies(&mons[otmp2->corpsenm]) && !uarmg) {
				char kbuf[BUFSZ];

				Sprintf(kbuf, "stolen %s corpse", mons[otmp2->corpsenm].mname);
				instapetrify(kbuf);
				}
			} else{
				getdir((char *)0);
				if(u.dx || u.dy){
					You("toss it away.");
					m_throw(&youmonst, u.ux, u.uy, u.dx, u.dy,
							(int)((ACURRSTR)/2 - otmp2->owt/40), otmp2,TRUE);
				}
				else{
					You("drop it at your feet.");
					(void) dropy(otmp2);
				}
				if(mdef->mhp <= 0) /* flung weapon killed monster */
				    return TRUE;
			}
			/* more take-away handling, after theft message */
			if (unwornmask & W_WEP) {		/* stole wielded weapon */
				possibly_unwield(mdef, FALSE);
			} else if (unwornmask & W_ARMG) {	/* stole worn gloves */
				mselftouch(mdef, (const char *)0, TRUE);
				if (mdef->mhp <= 0)	/* it's now a statue */
					return 1; /* monster is dead */
			}
		}
	  }
	 }
	 else if(youdefend){
		char buf[BUFSZ];
		buf[0] = '\0';
		steal(magr, buf, TRUE);
	 }
	 else{
		struct obj *obj;
		/* find an object to steal, non-cursed if magr is tame */
		for (obj = mdef->minvent; obj; obj = obj->nobj)
		    if (!magr->mtame || !obj->cursed)
				break;

		if (obj) {
			char buf[BUFSZ], onambuf[BUFSZ], mdefnambuf[BUFSZ];

			/* make a special x_monnam() call that never omits
			   the saddle, and save it for later messages */
			Strcpy(mdefnambuf, x_monnam(mdef, ARTICLE_THE, (char *)0, 0, FALSE));
#ifdef STEED
			if (u.usteed == mdef &&
					obj == which_armor(mdef, W_SADDLE))
				/* "You can no longer ride <steed>." */
				dismount_steed(DISMOUNT_POLY);
#endif
			obj_extract_self(obj);
			if (obj->owornmask) {
				mdef->misc_worn_check &= ~obj->owornmask;
				if (obj->owornmask & W_WEP)
				    setmnotwielded(mdef,obj);
				obj->owornmask = 0L;
				update_mon_intrinsics(mdef, obj, FALSE, FALSE);
			}
			/* add_to_minv() might free obj [if it merges] */
			if (vis)
				Strcpy(onambuf, doname(obj));
			(void) add_to_minv(magr, obj);
			if (vis) {
				Strcpy(buf, Monnam(magr));
				pline("%s steals %s from %s!", buf,
				    onambuf, mdefnambuf);
			}
			possibly_unwield(mdef, FALSE);
			mdef->mstrategy &= ~STRAT_WAITFORU;
			mselftouch(mdef, (const char *)0, FALSE);
			if (mdef->mhp <= 0)
				return 1;
		}
	 }
	}
	if(otmp->oartifact == ART_GIANTSLAYER){//If we're here, the target has the apropriate flag.
		mdef->movement -= NORMAL_SPEED / 2;
		if (!youdefend) pline("The great axe hamstrings %s!",mon_nam(mdef));
		else pline("The hateful axe hamstrings you!");
	}
	if(spec_ability2(otmp, SPFX2_DANCER)){
		if(!youdefend && uwep == otmp) magr->movement += NORMAL_SPEED / 3;
		else if(!youdefend && uswapwep == otmp) magr->movement += NORMAL_SPEED / 6;
		else if(youdefend) magr->movement += NORMAL_SPEED / 2;
	}
	if (spec_ability(otmp, SPFX_BEHEAD)) {
		if (otmp->oartifact == ART_OGRESMASHER && mdef->data->mlet == S_OGRE){
			wepdesc = "The vengeful hammer";
			if (youattack && u.uswallow && mdef == u.ustuck) {
				exercise(A_STR, TRUE);
				exercise(A_STR, TRUE);
				exercise(A_WIS, TRUE);
				You("smash %s wide open!", mon_nam(mdef));
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				return TRUE;
			}
			else if (!youdefend) {
				if(youattack){
					exercise(A_STR, TRUE);
					exercise(A_STR, TRUE);
					exercise(A_WIS, TRUE);
				}
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				pline("%s smashes %s flat!", wepdesc, mon_nam(mdef));
				otmp->dknown = TRUE;
				return TRUE;
			}
			else{
				*dmgptr = 2 * (Upolyd ? u.mh : u.uhp) + FATAL_DAMAGE_MODIFIER;
				pline("%s smashes you flat!", wepdesc);
				otmp->dknown = TRUE;
				return TRUE;
			}
		}
	/* We really want "on a natural 20" but Nethack does it in */
	/* reverse from AD&D. */
	    else if (otmp->oartifact == ART_TSURUGI_OF_MURAMASA && dieroll == 1) {
			wepdesc = "The razor-sharp blade";
			/* not really beheading, but so close, why add another SPFX */
			if (youattack && u.uswallow && mdef == u.ustuck) {
				You("slice %s wide open!", mon_nam(mdef));
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				return TRUE;
			}
			if (!youdefend) {
				/* allow normal cutworm() call to add extra damage */
				if(notonhead)
					return FALSE;

				if (bigmonst(mdef->data)) {
					if (youattack)
						You("slice deeply into %s!",
							mon_nam(mdef));
					else if (vis)
						pline("%s cuts deeply into %s!",
							  Monnam(magr), hittee);
					*dmgptr *= 2;
					return TRUE;
				}
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				pline("%s cuts %s in half!", wepdesc, mon_nam(mdef));
				otmp->dknown = TRUE;
				return TRUE;
			} else {
				if (bigmonst(youmonst.data)) {
					pline("%s cuts deeply into you!",
						  magr ? Monnam(magr) : wepdesc);
					*dmgptr *= 2;
					return TRUE;
				}

				/* Players with negative AC's take less damage instead
				 * of just not getting hit.  We must add a large enough
				 * value to the damage so that this reduction in
				 * damage does not prevent death.
				 */
				*dmgptr = 2 * (Upolyd ? u.mh : u.uhp) + FATAL_DAMAGE_MODIFIER;
				pline("%s cuts you in half!", wepdesc);
				otmp->dknown = TRUE;
				return TRUE;
			}
		} else if (otmp->oartifact == ART_KUSANAGI_NO_TSURUGI && dieroll <= 2) {
			static const char * const behead_msg[2] = {
				 "%s beheads %s!",
				 "%s decapitates %s!"
			};
			wepdesc = "The razor-sharp blade";
			/* not really beheading, but so close, why add another SPFX */
			if (youattack && u.uswallow && mdef == u.ustuck) {
				You("slice %s wide open!", mon_nam(mdef));
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				return TRUE;
			}
			if (!youdefend) {
				/* allow normal cutworm() call to add extra damage */
				if(notonhead)
					return FALSE;

				if (bigmonst(mdef->data)) {
					if (!has_head(mdef->data) || notonhead || u.uswallow) {
						if (youattack)
							You("slice deeply into %s!",
								mon_nam(mdef));
						else if (vis)
							pline("%s cuts deeply into %s!",
								  Monnam(magr), hittee);
						*dmgptr *= 2;
						return TRUE;
					}
					if (noncorporeal(mdef->data) || amorphous(mdef->data)) {
						pline("%s slices through %s %s.", wepdesc,
							  s_suffix(mon_nam(mdef)),
							  mbodypart(mdef,NECK));
						pline("It blows apart in the wind.");
						*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
						otmp->dknown = TRUE;
						return TRUE;
					}
					*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
					pline(behead_msg[rn2(SIZE(behead_msg))],
						  wepdesc, mon_nam(mdef));
					otmp->dknown = TRUE;
					return TRUE;
				}
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				pline("%s cuts %s in half!", wepdesc, mon_nam(mdef));
				otmp->dknown = TRUE;
				return TRUE;
			} else {
				if (bigmonst(youmonst.data)) {
					if (!has_head(youmonst.data)) {
						pline("%s cuts deeply into you!",
							  magr ? Monnam(magr) : wepdesc);
						*dmgptr *= 2;
						return TRUE;
					}
					if (noncorporeal(youmonst.data) || amorphous(youmonst.data)) {
						pline("%s slices through your %s.",
							  wepdesc, body_part(NECK));
						pline("It blows apart in the wind.");
						*dmgptr = 2 * (Upolyd ? u.mh : u.uhp)
							  + FATAL_DAMAGE_MODIFIER;
						pline(behead_msg[rn2(SIZE(behead_msg))],
							  wepdesc, "you");
						otmp->dknown = TRUE;
						/* Should amulets fall off? */
						return TRUE;
					}
					*dmgptr = 2 * (Upolyd ? u.mh : u.uhp)
						  + FATAL_DAMAGE_MODIFIER;
					pline(behead_msg[rn2(SIZE(behead_msg))],
						  wepdesc, "you");
					otmp->dknown = TRUE;
					/* Should amulets fall off? */
					return TRUE;
				}

				/* Players with negative AC's take less damage instead
				 * of just not getting hit.  We must add a large enough
				 * value to the damage so that this reduction in
				 * damage does not prevent death.
				 */
				*dmgptr = 2 * (Upolyd ? u.mh : u.uhp) + FATAL_DAMAGE_MODIFIER;
				pline("%s cuts you in half!", wepdesc);
				otmp->dknown = TRUE;
				return TRUE;
			}
		} else if ( (otmp->oartifact == ART_VORPAL_BLADE ||
						otmp->oartifact == ART_SNICKERSNEE
					)&&
			(dieroll == 1 || mdef->data == &mons[PM_JABBERWOCK]) ) { 
			// || mdef->data == &mons[PM_VORPAL_JABBERWOCK]) ) {
		static const char * const behead_msg[2] = {
		     "%s beheads %s!",
		     "%s decapitates %s!"
		};

		if (youattack && u.uswallow && mdef == u.ustuck)
			return FALSE;
		wepdesc = artilist[ART_VORPAL_BLADE].name;
		if (!youdefend) {
			if (!has_head(mdef->data) || notonhead || u.uswallow) {
				if (youattack)
					pline("Somehow, you miss %s wildly.",
						mon_nam(mdef));
				else if (vis)
					pline("Somehow, %s misses wildly.",
						mon_nam(magr));
				*dmgptr = 0;
				return ((boolean)(youattack || vis));
			}
			if (noncorporeal(mdef->data) || amorphous(mdef->data)) {
				pline("%s slices through %s %s.", wepdesc,
				      s_suffix(mon_nam(mdef)),
				      mbodypart(mdef,NECK));
				return TRUE;
			}
			*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
			pline(behead_msg[rn2(SIZE(behead_msg))],
			      wepdesc, mon_nam(mdef));
			otmp->dknown = TRUE;
			return TRUE;
		} else {
			if (!has_head(youmonst.data)) {
				pline("Somehow, %s misses you wildly.",
				      magr ? mon_nam(magr) : wepdesc);
				*dmgptr = 0;
				return TRUE;
			}
			if (noncorporeal(youmonst.data) || amorphous(youmonst.data)) {
				pline("%s slices through your %s.",
				      wepdesc, body_part(NECK));
				return TRUE;
			}
			*dmgptr = 2 * (Upolyd ? u.mh : u.uhp)
				  + FATAL_DAMAGE_MODIFIER;
			pline(behead_msg[rn2(SIZE(behead_msg))],
			      wepdesc, "you");
			otmp->dknown = TRUE;
			/* Should amulets fall off? */
			return TRUE;
		}
	    }
	}
	if(arti_tentRod(otmp) && !youdefend){
		int extrahits = rn2(7);
		int monAC, extrahit=0;
		while(extrahits--){
			monAC = find_roll_to_hit(mdef)-2*extrahits-2;
			if(u.uswallow || monAC > rnd(20)){
				*dmgptr += dmgval(otmp, mdef, 0);
				extrahit++;
			}
		}
		if(extrahit >= 2){
			switch(rn2(3)){
			case 0:
				mdef->mcansee = 0;
				mdef->mblinded = d(3,3);
			break;
			case 1:
				mdef->mstun = 1;
			break;
			case 2:
				mdef->mconf = 1;
			break;
			}
		}
		if(extrahit >= 6){
			switch(rn2(3)){
			case 0:
				mdef->mspeed = MSLOW;
				mdef->permspeed = MSLOW;
			break;
			case 1:
				mdef->mcanmove = 0;
				mdef->mfrozen = d(1,6);
			break;
			case 2:
				mdef->mcrazed = 1;
			break;
			}
		}
		if(extrahit == 7){
			if(!resists_fire(mdef)) *dmgptr += d(1, 7);
			if(!resists_cold(mdef)) *dmgptr += d(1, 7);
			if(!resists_elec(mdef)) *dmgptr += d(1, 7);
			if(!resists_acid(mdef)) *dmgptr += d(1, 7);
			if(!resists_magm(mdef)) *dmgptr += d(1, 7);
			if(!resists_poison(mdef)) *dmgptr += d(1, 7);
			if(!resists_drli(mdef)) *dmgptr += d(1, 7);
		}
	} else if(arti_tentRod(otmp) && youdefend){
		int extrahits = rn2(7);
		int extrahit = 0;
		while(extrahits--){
			if(u.uswallow || u.uac > rnd(20)){
				*dmgptr += dmgval(otmp, mdef, 0);
				extrahit++;
			}
		}
		if(extrahit >= 2){
			switch(rn2(3)){
			case 0:
				make_blinded(Blinded+d(3,3), FALSE);
			break;
			case 1:
				make_stunned((HStun)+d(3,3),FALSE);
			break;
			case 2:
				make_confused(HConfusion+d(3,3),FALSE);
			break;
			}
		}
		if(extrahit >= 6){
			switch(rn2(3)){
			case 0:
				u_slow_down();
			break;
			case 1:
				nomul(-1*d(3,3), "paralyzed by the Tentacle Rod.");
			break;
			case 2:
				make_confused(10000,FALSE); //very large value representing insanity
			break;
			}
		}
		if(extrahit == 7){
			if(!Fire_resistance) *dmgptr += d(1, 7);
			if(!Cold_resistance) *dmgptr += d(1, 7);
			if(!Shock_resistance) *dmgptr += d(1, 7);
			if(!Acid_resistance) *dmgptr += d(1, 7);
			if(!Antimagic) *dmgptr += d(1, 7);
			if(!Poison_resistance) *dmgptr += d(1, 7);
			if(!Drain_resistance) *dmgptr += d(1, 7);
		}
	}
	if (!spec_dbon_applies) {
	    /* since damage bonus didn't apply, nothing more to do;  
	       no further attacks have side-effects on inventory */
	    return FALSE;
	}
	if (spec_ability(otmp, SPFX_DRLI)) {
		if (!youdefend) {
			if (vis) {
			    if(otmp->oartifact == ART_STORMBRINGER)
				pline_The("%s blade draws the life from %s!",
				      hcolor(NH_BLACK),
				      mon_nam(mdef));
			    else
				pline("%s draws the life from %s!",
				      The(distant_name(otmp, xname)),
				      mon_nam(mdef));
			}
			if (mdef->m_lev == 0) {
			    *dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
			} else {
			    int drain = rnd(8);
			    *dmgptr += drain;
			    mdef->mhpmax -= drain;
			    mdef->m_lev--;
			    drain /= 2;
			    if (drain) healup(drain, 0, FALSE, FALSE);
			}
			return vis;
		} else { /* youdefend */
			int oldhpmax = u.uhpmax;

			if (Blind)
				You_feel("an %s drain your life!",
				    otmp->oartifact == ART_STORMBRINGER ?
				    "unholy blade" : "object");
			else if (otmp->oartifact == ART_STORMBRINGER)
				pline_The("%s blade drains your life!",
				      hcolor(NH_BLACK));
			else
				pline("%s drains your life!",
				      The(distant_name(otmp, xname)));
			losexp("life drainage",TRUE,FALSE,FALSE);
			if (magr && magr->mhp < magr->mhpmax) {
			    magr->mhp += (oldhpmax - u.uhpmax)/2;
			    if (magr->mhp > magr->mhpmax) magr->mhp = magr->mhpmax;
			}
			return TRUE;
		}
	}
	return FALSE;
}

static NEARDATA const char recharge_type[] = { ALLOW_COUNT, ALL_CLASSES, 0 };
static NEARDATA const char invoke_types[] = { ALL_CLASSES, 0 };
		/* #invoke: an "ugly check" filters out most objects */

int
doinvoke()
{
    register struct obj *obj;

    obj = getobj(invoke_types, "invoke");
    if (!obj) return 0;
    if (obj->oartifact && !touch_artifact(obj, &youmonst)) return 1;
    return arti_invoke(obj);
}

int
doparticularinvoke(obj)
    register struct obj *obj;
{
    if (!obj) return 0;
    if (obj->oartifact && !touch_artifact(obj, &youmonst)) return 1;
    return arti_invoke(obj);
}

STATIC_OVL int
arti_invoke(obj)
    register struct obj *obj;
{
    register const struct artifact *oart = get_artifact(obj);
    char buf[BUFSZ];
 	struct obj *pseudo, *otmp, *pseudo2, *pseudo3;
	int summons[9] = {0, PM_FOG_CLOUD, PM_DUST_VORTEX, 
					  PM_ICE_VORTEX, PM_ENERGY_VORTEX, PM_STEAM_VORTEX, 
					  PM_FIRE_VORTEX, PM_STALKER, PM_AIR_ELEMENTAL};
	coord cc;
	int n, damage;
	struct permonst *pm;
	struct monst *mtmp = 0;
   if(!oart || !oart->inv_prop) {
	if(obj->otyp == CRYSTAL_BALL)
	    use_crystal_ball(obj);
	else
	    pline(nothing_happens);
	return 1;
    }

    if(oart->inv_prop > LAST_PROP) {
	/* It's a special power, not "just" a property */
	if(obj->age > monstermoves && 
		oart->inv_prop != FIRE_SHIKAI && 
		oart->inv_prop != SEVENFOLD && 
		oart->inv_prop != LORDLY
	) {
	    /* the artifact is tired :-) */
		if(obj->oartifact == ART_FIELD_MARSHAL_S_BATON){
			You_hear("the sounds of hurried preparation.");
			return 1;
		}
	    You_feel("that %s %s ignoring you.",
		     the(xname(obj)), otense(obj, "are"));
	    /* and just got more so; patience is essential... */
		obj->age += Role_if(PM_PRIEST) ? (long) d(1,20) : (long) d(3,10);
	    return 1;
	}
	if( /* some properties can be used as often as desired, or track cooldowns in a different way */
		oart->inv_prop != FIRE_SHIKAI &&
		oart->inv_prop != ICE_SHIKAI &&
		oart->inv_prop != BLESS &&
		oart->inv_prop != NECRONOMICON &&
		oart->inv_prop != SPIRITNAMES &&
		oart->inv_prop != LORDLY &&
		oart->inv_prop != SEVENFOLD
	)obj->age = monstermoves + (long)(rnz(100)*(Role_if(PM_PRIEST) ? .8 : 1));

	switch(oart->inv_prop) {
	case TAMING: {
	    struct obj pseudo;

	    pseudo = zeroobj;	/* neither cursed nor blessed */
	    pseudo.otyp = SCR_TAMING;
	    (void) seffects(&pseudo);
	    break;
	  }
	case LEADERSHIP: {
	    (void) pet_detect_and_tame(obj);
	    break;
	  }
	case HEALING: {
	    int healamt = (u.uhpmax + 1 - u.uhp) / 2;
	    long creamed = (long)u.ucreamed;

	    if (Upolyd) healamt = (u.mhmax + 1 - u.mh) / 2;
	    if (healamt || Sick || Slimed || Blinded > creamed)
		You_feel("better.");
	    else
		goto nothing_special;
	    if (healamt > 0) {
		if (Upolyd) u.mh += healamt;
		else u.uhp += healamt;
	    }
	    if(Sick) make_sick(0L,(char *)0,FALSE,SICK_ALL);
	    if(Slimed) Slimed = 0L;
	    if (Blinded > creamed) make_blinded(creamed, FALSE);
	    flags.botl = 1;
	    break;
	  }
	case ENERGY_BOOST: {
	    int epboost = (u.uenmax + 1 - u.uen) / 2;
	    if (epboost > 120) epboost = 120;		/* arbitrary */
	    else if (epboost < 12) epboost = u.uenmax - u.uen;
	    if(epboost) {
		You_feel("re-energized.");
		u.uen += epboost;
		flags.botl = 1;
	    } else
		goto nothing_special;
	    break;
	  }
	case UNTRAP: {
	    if(!untrap(TRUE)) {
		obj->age = 0; /* don't charge for changing their mind */
		return 0;
	    }
	    break;
	  }
	case CHARGE_OBJ: {
	    struct obj *otmp = getobj(recharge_type, "charge");
	    boolean b_effect;

	    if (!otmp) {
		obj->age = 0;
		return 0;
	    }
	    b_effect = obj->blessed &&
		(Role_switch == oart->role || !oart->role);
	    recharge(otmp, b_effect ? 1 : obj->cursed ? -1 : 0);
	    update_inventory();
	    break;
	  }
	case LEV_TELE:
	    level_tele();
	    break;
	case CREATE_PORTAL: {
	    int i, num_ok_dungeons, last_ok_dungeon = 0;
	    d_level newlev;
	    extern int n_dgns; /* from dungeon.c */
	    winid tmpwin = create_nhwindow(NHW_MENU);
	    anything any;

	    any.a_void = 0;	/* set all bits to zero */
	    start_menu(tmpwin);
	    /* use index+1 (cant use 0) as identifier */
	    for (i = num_ok_dungeons = 0; i < n_dgns; i++) {
		if (!dungeons[i].dunlev_ureached) continue;
		any.a_int = i+1;
		add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 dungeons[i].dname, MENU_UNSELECTED);
		num_ok_dungeons++;
		last_ok_dungeon = i;
	    }
	    end_menu(tmpwin, "Open a portal to which dungeon?");
	    if (num_ok_dungeons > 1) {
		/* more than one entry; display menu for choices */
		menu_item *selected;
		int n;

		n = select_menu(tmpwin, PICK_ONE, &selected);
		if (n <= 0) {
		    destroy_nhwindow(tmpwin);
		    goto nothing_special;
		}
		i = selected[0].item.a_int - 1;
		free((genericptr_t)selected);
	    } else
		i = last_ok_dungeon;	/* also first & only OK dungeon */
	    destroy_nhwindow(tmpwin);

	    /*
	     * i is now index into dungeon structure for the new dungeon.
	     * Find the closest level in the given dungeon, open
	     * a use-once portal to that dungeon and go there.
	     * The closest level is either the entry or dunlev_ureached.
	     */
	    newlev.dnum = i;
	    if(dungeons[i].depth_start >= depth(&u.uz))
		newlev.dlevel = dungeons[i].entry_lev;
	    else
		newlev.dlevel = dungeons[i].dunlev_ureached;
	    if(u.uhave.amulet || In_endgame(&u.uz) || In_endgame(&newlev) ||
	       newlev.dnum == u.uz.dnum) {
		You_feel("very disoriented for a moment.");
	    } else {
		if(!Blind) You("are surrounded by a shimmering sphere!");
		else You_feel("weightless for a moment.");
		goto_level(&newlev, FALSE, FALSE, FALSE);
	    }
	    break;
	  }
	case ENLIGHTENING:
	    enlightenment(0);
	break;
	case CREATE_AMMO: {
	    struct obj *otmp;
		if(obj->oartifact == ART_LONGBOW_OF_DIANA) otmp = mksobj(ARROW, TRUE, FALSE);
		else if(obj->oartifact == ART_FUMA_ITTO_NO_KEN) otmp = mksobj(SHURIKEN, TRUE, FALSE);
		else if(obj->oartifact == ART_SILVER_STARLIGHT) otmp = mksobj(SHURIKEN, TRUE, FALSE);
		else if(obj->oartifact == ART_YOICHI_NO_YUMI) otmp = mksobj(YA, TRUE, FALSE);
		else if(obj->oartifact == ART_WRATHFUL_SPIDER) otmp = mksobj(DROVEN_BOLT, TRUE, FALSE);

	    if (!otmp) goto nothing_special;
	    otmp->blessed = obj->blessed;
	    otmp->cursed = obj->cursed;
	    otmp->bknown = obj->bknown;
	    if (obj->blessed) {
		if (otmp->spe < 0) otmp->spe = 0;
		otmp->quan += rnd(10);
	    } else if (obj->cursed) {
		if (otmp->spe > 0) otmp->spe = 0;
	    } else
		otmp->quan += rnd(5);
	    otmp->owt = weight(otmp);
	    otmp = hold_another_object(otmp, "Suddenly %s out.",
				       aobjnam(otmp, "fall"), (const char *)0);
	}
	case OBJECT_DET:
		object_detect(obj, 0);
		artifact_detect(obj);
	break;
	case TELEPORT_SHOES:
		if(obj == uarmf){
			tele();//hungerless teleport
		}else if(!obj->owornmask){
			obj_extract_self(obj);
			dropy(obj);
			rloco(obj);
			pline("%s teleports without you.",xname(obj));
		}else{
			pline("%s shakes for an instant.");
		}
	break;
#ifdef CONVICT
	case PHASING:   /* Walk through walls and stone like a xorn */
        if (Passes_walls) goto nothing_special;
	    if (oart == &artilist[ART_IRON_BALL_OF_LIBERATION]) {
		if (Punished && (obj != uball)) {
		    unpunish(); /* Remove a mundane heavy iron ball */
		}
		
		if (!Punished) {
		    setworn(mkobj(CHAIN_CLASS, TRUE), W_CHAIN);
		    setworn(obj, W_BALL);
		    uball->spe = 1;
		    if (!u.uswallow) {
			placebc();
			if (Blind) set_bc(1);	/* set up ball and chain variables */
			newsym(u.ux,u.uy);		/* see ball&chain if can't see self */
		    }
		    Your("%s chains itself to you!", xname(obj));
		}
	    }
        if (!Hallucination) {    
            Your("body begins to feel less solid.");
        } else {
            You_feel("one with the spirit world.");
        }
        incr_itimeout(&Phasing, (50 + rnd(100)));
        obj->age += Phasing; /* Time begins after phasing ends */
    break;
#endif /* CONVICT */

	case SATURN:
		{
			if ( !(uwep && uwep == obj) ) {
				You_feel("that you should be wielding %s", the(xname(obj)));
				obj->age = 0;
				break;
			}
			if (!getdir((char *)0)) { //Oh, getdir must set the .d_ variables below.
			    /* getdir cancelled, just do the nondirectional scroll */
				obj->age = 0;
				break;
			}
			else if(u.dx || u.dy) {
				pline("Death Reborn Revolution!");
				pseudo = mksobj(SPE_MAGIC_MISSILE, FALSE, FALSE);
				pseudo->blessed = pseudo->cursed = 0;
				pseudo->quan = 20L;			/* do not let useup get it */
				weffects(pseudo);
				obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
			}
			else if(u.dz < 0){
				pline("Silence Wall!");
				cast_protection();
			}
			else if(u.dz > 0 && (yn("Are you sure?") == 'y') ){
				register struct monst *mtmp, *mtmp2;
				int gonecnt = 0;
				You("touch the tip of the Silence Glaive to the ground.");
				pline("The walls of the dungeon quake!");
				do_earthquake(100);
				do_earthquake(100);
				do_earthquake(100);
				You("call out to the souls and spirits inhabiting this land.");
				for (mtmp = fmon; mtmp; mtmp = mtmp2) {
					mtmp2 = mtmp->nmon;
					if(mtmp->data->geno & G_GENO){
						if (DEADMONSTER(mtmp)) continue;
						mongone(mtmp);
						gonecnt++;
					}
				}
				pline("Having gathered %d followers, you set out for a distant place, your charges in tow.",
					gonecnt);
				killer_format = KILLED_BY;
				killer = "leading departed souls to another world";
				done(DIED);
				You("return, having delivered your followers to their final destination.");
			}
			else{
				obj->age = 0;
			}
		}
	break;
 	case PLUTO:
		{
			if ( !(uwep && uwep == obj) ) {
				You_feel("that you should be wielding %s", the(xname(obj)));
				obj->age = 0;
				break;
			}
			if (!getdir((char *)0)) { //Oh, getdir must set the .d_ variables below.
			    /* getdir cancelled, just do the nondirectional scroll */
				obj->age = 0;
				break;
			}
			else if(u.dx || u.dy) {
				pline("Dead Scream.");
				pseudo = mksobj(SPE_MAGIC_MISSILE, FALSE, FALSE);
				pseudo->blessed = pseudo->cursed = 0;
				pseudo->quan = 20L;			/* do not let useup get it */
				weffects(pseudo);
				obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
			}
			else if(u.dz < 0 && (yn("This is a forbidden technique.  Do you wish to use it anyway?") == 'y')){
				pline("Time Stop!");
				youmonst.movement += NORMAL_SPEED*10;
				Stoned = 5;
				delayed_killer = "termination of personal timeline.";
				killer_format = KILLED_BY;
			}
			else{
				obj->age = 0;
			}
		}
	break;
 	case SPEED_BANKAI:
		{
			if ( !(uwep && uwep == obj) ) {
				You_feel("that you should be wielding %s", the(xname(obj)));
				obj->age = 0;
				break;
			}
			if (!getdir((char *)0)) { //Oh, getdir must set the .d_ variables below.
				obj->age = 0;
				break;
			}
			else if(u.dx || u.dy) {
				int range = u.ulevel+obj->spe;
			    int x,y;
				uchar typ;
				boolean shopdoor = FALSE;
				bhitpos.x = u.ux;
				bhitpos.y = u.uy;
				pline("Getsuga Tensho!");
				pseudo = mksobj(SPE_FORCE_BOLT, FALSE, FALSE);
				pseudo->blessed = pseudo->cursed = 0;
				pseudo->quan = 20L;
				while(range-- > 0) {
					
				    bhitpos.x += u.dx;
				    bhitpos.y += u.dy;
				    x = bhitpos.x; y = bhitpos.y;
					
				    if(!isok(x, y)) {
						bhitpos.x -= u.dx;
						bhitpos.y -= u.dy;
						break;
				    }
					zap_dig(x,y,1);
				    typ = levl[bhitpos.x][bhitpos.y].typ;
					
				    if (typ == IRONBARS){
						char numbars;
						struct obj *obj;
						You_hear("a sharp crack!");
					    levl[bhitpos.x][bhitpos.y].typ = CORR;
						for(numbars = d(2,4)-1; numbars > 0; numbars--){
							obj = mksobj_at(IRON_BAR, bhitpos.x, bhitpos.y, FALSE, FALSE);
						    obj->spe = 0;
						    obj->cursed = obj->blessed = FALSE;
						}
					    newsym(bhitpos.x, bhitpos.y);
				    }

				    if (find_drawbridge(&x,&y))
						    destroy_drawbridge(x,y);

				    if ((mtmp = m_at(bhitpos.x, bhitpos.y)) != 0) {
//						pline("mon found at %d, %d", bhitpos.x, bhitpos.y);
					    if (cansee(bhitpos.x,bhitpos.y) && !canspotmon(mtmp)) {
						    map_invisible(bhitpos.x, bhitpos.y);
						}
					    resist(mtmp, WEAPON_CLASS, d(u.ulevel+obj->spe,12) + spell_damage_bonus()*3, FALSE);
					}
					bhitpile(pseudo,bhito,bhitpos.x,bhitpos.y);
				    if(IS_DOOR(typ) || typ == SDOOR) {
					    if (doorlock(pseudo, bhitpos.x, bhitpos.y)) {
							if (levl[bhitpos.x][bhitpos.y].doormask == D_BROKEN
							    && *in_rooms(bhitpos.x, bhitpos.y, SHOPBASE)) {
								    shopdoor = TRUE;
								    add_damage(bhitpos.x, bhitpos.y, 400L);
							}
					    }
				    }
				    typ = levl[bhitpos.x][bhitpos.y].typ;//must check type again, incase the preceding code changed it.
				    if(!ZAP_POS(typ) || closed_door(bhitpos.x, bhitpos.y)) {
//						pline("breaking, due to %d!",typ);
						bhitpos.x -= u.dx;
						bhitpos.y -= u.dy;
						break;
					}
				}
				if(shopdoor) pay_for_damage("destroy", FALSE);
				obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
			}
			else{
				obj->age = 0;
			}
		}
	break;
 	case ICE_SHIKAI:
		{
			char dancenumber = 0;
			if ( !(uwep && uwep == obj) ) {
				You_feel("that you should be wielding %s", the(xname(obj)));
				break;
			}
			pline("What dance will you use (1/2/3)?");
			dancenumber =  readchar();
			if (dancenumber == '1') {
				if(u.SnSd1 > monstermoves){
				    You_feel("that %s %s ignoring you.",
					     the(xname(obj)), otense(obj, "are"));
					/* and just got more so; patience is essential... */
					u.SnSd1 += Role_if(PM_PRIEST)||Role_if(PM_SAMURAI) ? (long) d(1,20) : (long) d(3,10);
				    return 1;
				}
				else if(throweffect()){
					pline("Some no mai, Tsukishiro!");
					u.SnSd1 = monstermoves + (long)(rnz(100)*(Role_if(PM_PRIEST)||Role_if(PM_SAMURAI) ? .9 : 1));
						explode(u.dx, u.dy,
							2, //2 = AD_COLD, explode uses nonstandard damage type flags...
							(u.ulevel + obj->spe + spell_damage_bonus())*3, 0,
							EXPL_FROSTY);
					nomul(-1, "performing a sword dance");//both the first and the second dance leave you momentarily exposed.
				}
			}
			else if (dancenumber == '2') {
				if(u.SnSd2 > monstermoves){
				    You_feel("that %s %s ignoring you.",
					     the(xname(obj)), otense(obj, "are"));
					/* and just got more so; patience is essential... */
					u.SnSd2 += Role_if(PM_PRIEST)||Role_if(PM_SAMURAI) ? (long) d(1,20) : (long) d(3,10);
				    return 1;
				}
				else if(getdir((char *)0) && (u.dx || u.dy)) {
					pline("Tsugi no mai, Hakuren!");
					u.SnSd2 = monstermoves + (long)(rnz(100)*(Role_if(PM_PRIEST)||Role_if(PM_SAMURAI) ? .9 : 1));
					buzz(WAN_COLD - WAN_MAGIC_MISSILE,
						 (u.ulevel + obj->spe + spell_damage_bonus()), u.ux, u.uy, u.dx, u.dy,7+obj->spe,0);
				}
					nomul(-1, "performing a sword dance");//both the first and the second dance leave you momentarily exposed.
			}
			else if (dancenumber == '3') {
				if(u.SnSd3 > monstermoves){
				    You_feel("that %s %s ignoring you.",
					     the(xname(obj)), otense(obj, "are"));
					/* and just got more so; patience is essential... */
					u.SnSd3 += Role_if(PM_PRIEST)||Role_if(PM_SAMURAI) ? (long) d(1,20) : (long) d(3,10);
				    return 1;
				}
				else{
					pline("San no mai, Shirafune!");
					pline("Ice crackles around your weapon!");
					u.SnSd3 = monstermoves + (long)(rnz(100)*(Role_if(PM_PRIEST)||Role_if(PM_SAMURAI) ? .9 : 1));
					obj->cursed = 0;
					obj->blessed = 1;
					obj->oeroded = 0;
					obj->oeroded2= 0;
					if(obj->spe < 3) obj->spe = 3;
					u.SnSd3duration = monstermoves + (long) u.ulevel + obj->spe;
				}
			}
			else{
				You("don't know that dance....");
			}
		}
	break;
 	case FIRE_SHIKAI:
		{
			boolean toosoon = monstermoves < obj->age;
			if ( !(uwep && uwep == obj) ) {
				You_feel("that you should be wielding %s", the(xname(obj)));
	break;
			}
			else {
				int energy, damage, n;
				int role_skill;
				boolean confused = (Confusion != 0);
				struct obj *pseudo;
				coord cc;

				energy = toosoon ? 25 : 15;
				pseudo = mksobj(SPE_FIREBALL, FALSE, FALSE);
				pseudo->blessed = pseudo->cursed = 0;
				pseudo->quan = 20L;			/* do not let useup get it */
				role_skill = max(P_SKILL(uwep_skill_type()), P_SKILL(spell_skilltype(pseudo->otyp)) );
				if (role_skill >= P_SKILLED && yn("Use advanced technique?") == 'y'){
					if(energy > u.uen) {
						You("don't have enough energy to use this technique");
						if(!toosoon) obj->age = 0;
	break;
					}
			        if (throweffect()) {
						pline("Snap, %s!", ONAME(obj));
					    cc.x=u.dx;cc.y=u.dy;
					    n=rnd(role_skill*2)+role_skill*2;
					    while(n--) {
							if(!u.dx && !u.dy && !u.dz) {
							    if ((damage = zapyourself(pseudo, TRUE)) != 0) {
									char buf[BUFSZ];
									Sprintf(buf, "blasted %sself with a fireball", uhim());
									losehp(damage + obj->spe, buf, NO_KILLER_PREFIX);
							    }
							} else {
							    explode(u.dx, u.dy,
								    pseudo->otyp - SPE_MAGIC_MISSILE + 10,
								    u.ulevel/2 + 10 + obj->spe, 0,
									EXPL_FIERY);
							}
							u.dx = cc.x+rnd(5)-3; u.dy = cc.y+rnd(5)-3;
							if (!isok(u.dx,u.dy) || !cansee(u.dx,u.dy) ||
							    IS_STWALL(levl[u.dx][u.dy].typ) || u.uswallow) {
							    /* Spell is reflected back to center */
								    u.dx = cc.x;
								    u.dy = cc.y;
					        }
					    }
					}
				}
				else{
					if (role_skill >= P_SKILLED) energy = toosoon ? 15 : 5;
					if(energy > u.uen) {
						You("don't have enough energy to use this technique");
						if(!toosoon) obj->age = 0;
	break;
					}
					if (!getdir((char *)0)) { //Oh, getdir must set the .d_ variables below.
					    /* getdir cancelled, re-use previous direction */
//					    pline_The("spiritual energy is released!");
	break;
					}
					pline("Snap, %s!", ONAME(obj));
					if(!u.dx && !u.dy && !u.dz) {
					    if ((damage = zapyourself(pseudo, TRUE)) != 0) {
							char buf[BUFSZ];
							Sprintf(buf, "zapped %sself with a spiritual technique", uhim());
							losehp(damage, buf, NO_KILLER_PREFIX);
					    }
					} 
					else weffects(pseudo);
				}
				update_inventory();	/* spell may modify inventory */
				u.uen -= energy;
			}
		}
		obj->age = monstermoves + d(1,100);
	break;
	case SMOKE_CLOUD: {
           coord cc;
           cc.x = u.ux;
           cc.y = u.uy;
           /* Cause trouble if cursed or player is wrong role */
           if (obj->cursed || (Role_switch == oart->role || !oart->role)) {
              You("may summon a stinking cloud.");
               pline("Where do you want to center the cloud?");
               if (getpos(&cc, TRUE, "the desired position") < 0) {
                   pline(Never_mind);
                   obj->age = 0;
                   return 0;
               }
               if (!cansee(cc.x, cc.y) || distu(cc.x, cc.y) >= 32) {
                   You("smell rotten eggs.");
                   return 0;
               }
           }
           pline("A cloud of toxic smoke pours out!");
           (void) create_gas_cloud(cc.x, cc.y, 3+bcsign(obj),
                                           8+4*bcsign(obj));
		   }
    break;
	case BLESS:
		You("bless your artifact.");
		if(cansee(u.ux, u.uy)) pline("Holy light shines upon it!");
		obj->cursed = 0;
		obj->blessed = 1;
		obj->oeroded = 0;
		obj->oeroded2= 0;
		obj->oerodeproof = 1;
		if(obj->spe < 3) obj->spe = 3;
    break;
	case CANNONADE:
		You_hear("a voice shouting\"By your order, Sah!\"");
		if (getdir((char *)0) && (u.dx || u.dy)){
		    struct obj *otmp;
			int i, x, y;
			x =  u.ux;
			y = u.uy;
			for(i = 12; i > 0; i--){
				int xadj=0;
				int yadj=0;
				otmp = mksobj(HEAVY_IRON_BALL, TRUE, FALSE);
			    otmp->blessed = 0;
			    otmp->cursed = 0;
				if(u.dy == 0) yadj = d(1,3)-2;
				else if(u.dx == 0) xadj = d(1,3)-2;
				else if(u.dx == u.dy){
					int dd = d(1,3)-2;
					xadj = dd;
					yadj = -1 * dd;
				}
				else{
					int dd = d(1,3)-2;
					xadj = yadj = dd;
				}
				set_destroy_thrown(1); //state variable referenced in drop_throw
					m_throw(&youmonst, u.ux + xadj, u.uy + yadj, u.dx, u.dy,
						2*BOLT_LIM, otmp,TRUE);
					nomul(0, NULL);
				set_destroy_thrown(0);  //state variable referenced in drop_throw
			}
		}
    break;
	case FIRAGA:
				if( (obj->spe > 0) && throweffect()){
					exercise(A_WIS, TRUE);
					cc.x=u.dx;cc.y=u.dy;
					n=3;
					while(n--) {
						explode(u.dx, u.dy,
							1, //1 = AD_FIRE, explode uses nonstandard damage type flags...
							u.ulevel + 10 + spell_damage_bonus(), 0,
							EXPL_FIERY);
						u.dx = cc.x+rnd(3)-2; u.dy = cc.y+rnd(3)-2;
						if (!isok(u.dx,u.dy) || !cansee(u.dx,u.dy) ||
							IS_STWALL(levl[u.dx][u.dy].typ) || u.uswallow) {
							/* Spell is reflected back to center */
								u.dx = cc.x;
								u.dy = cc.y;
						}
					}
				}
	break;
	case BLIZAGA:
				if( (obj->spe > 0) && throweffect()){
					exercise(A_WIS, TRUE);
					cc.x=u.dx;cc.y=u.dy;
					n=3;
					while(n--) {
						explode(u.dx, u.dy,
							2, //2 = AD_COLD, explode uses nonstandard damage type flags...
							u.ulevel + 10 + spell_damage_bonus(), 0,
							EXPL_FROSTY);
						u.dx = cc.x+rnd(3)-2; u.dy = cc.y+rnd(3)-2;
						if (!isok(u.dx,u.dy) || !cansee(u.dx,u.dy) ||
							IS_STWALL(levl[u.dx][u.dy].typ) || u.uswallow) {
							/* Spell is reflected back to center */
								u.dx = cc.x;
								u.dy = cc.y;
						}
					}
				}
	break;
	case THUNDAGA:
				if( (obj->spe > 0) && throweffect()){
					exercise(A_WIS, TRUE);
					cc.x=u.dx;cc.y=u.dy;
					n=3;
					while(n--) {
						explode(u.dx, u.dy,
							5, //2 = AD_ELEC, explode uses nonstandard damage type flags...
							u.ulevel + 10 + spell_damage_bonus(), 0,
							EXPL_MAGICAL);
						u.dx = cc.x+rnd(3)-2; u.dy = cc.y+rnd(3)-2;
						if (!isok(u.dx,u.dy) || !cansee(u.dx,u.dy) ||
							IS_STWALL(levl[u.dx][u.dy].typ) || u.uswallow) {
							/* Spell is reflected back to center */
								u.dx = cc.x;
								u.dy = cc.y;
						}
					}
				}
	break;
	case QUAKE:{
				register struct monst *mtmp, *mtmp2;
				int gonecnt = 0;
				pline("For a moment, you feel a crushing weight settle over you.");
				for (mtmp = fmon; mtmp; mtmp = mtmp2) {
					mtmp2 = mtmp->nmon;
					if(mtmp->data->geno & G_GENO){
						if (DEADMONSTER(mtmp)) continue;
						mtmp->mhp = mtmp->mhp/4 + 1;
					}
				}
				pline_The("entire dungeon is quaking around you!");
				do_earthquake(u.ulevel / 4 + 1);
				do_earthquake(u.ulevel / 2 + 1);
				awaken_monsters(ROWNO * COLNO);
			   }
	break;
	case SHADOW_FLARE:
				if( (obj->spe > 0) && throweffect()){
					exercise(A_WIS, TRUE);
					cc.x=u.dx;cc.y=u.dy;
					n=3;
					while(n--) {
						int type = d(1,3);
						explode(u.dx, u.dy,
							4, //4 = AD_DISN, explode uses nonstandard damage type flags...
							u.ulevel + 10 + spell_damage_bonus(), 0,
							EXPL_DARK);
						u.dx = cc.x+rnd(3)-2; u.dy = cc.y+rnd(3)-2;
						if (!isok(u.dx,u.dy) || !cansee(u.dx,u.dy) ||
							IS_STWALL(levl[u.dx][u.dy].typ) || u.uswallow) {
							/* Spell is reflected back to center */
								u.dx = cc.x;
								u.dy = cc.y;
						}
					}
				}
	break;
	case SING_SPEAR:
		if(!exist_artifact(SILVER_SPEAR, "Blade Singer's Spear") ){
			otmp = mksobj(SILVER_SPEAR, TRUE, FALSE);
			otmp = oname(otmp, artiname(ART_BLADE_SINGER_S_SPEAR));		
			if (otmp->spe < 0) otmp->spe = 0;
			if (otmp->cursed) uncurse(otmp);
			otmp->oerodeproof = TRUE;
			dropy(otmp);
		    pline("An object apears at your feet");
		}
	break;
	case DANCE_DAGGER:
		if(!exist_artifact(SILVER_DAGGER, "Blade Dancer's Dagger") ){
			otmp = mksobj(SILVER_DAGGER, TRUE, FALSE);
			otmp = oname(otmp, artiname(ART_BLADE_DANCER_S_DAGGER));		
			if (otmp->spe < 0) otmp->spe = 0;
			if (otmp->cursed) uncurse(otmp);
			otmp->oerodeproof = TRUE;
			dropy(otmp);
		    pline("An object apears at your feet");
		}	
	break;	
	case SEVENFOLD:
//Ruat:  Fall.  Makes a pit.  One charge.
//Coelum: Heaven.  Casts cure.  One charge.
//Fiat:  Let it be.  Creates food.  One charge.
//Justitia:  Justice.  Creates light.  One charge.
//Ecce:  See.  Casts detect monsters or detect unseen.  Two charges.
//Lex:  Law.  Grants you enlightenment.  Two charges.
//Rex:  King.  Grants Levitation.  Three charges.
//Ruat Coelum:  Heaven Falls.  Attack spell.  One charge.
//Fiat Justitia:  Let justice be done.  Slows, fears and damages target.  Two charges.
//Lex Rex:  Law is King.  Heals aflictions, removes curses.  Three charges.
//Ecce!  Lex Rex:  See!  Law is King.  Charms monsters.  Five charges.
//Ruat Coelum, Fiat Justitia:  Though heaven falls, let justice be done.  Calls pets.  Seven charges.
//Ruat Coelum, Fiat Justitia.  Ecce!  Lex Rex!:  Though heaven falls, let justice be done.  See!  Law is King!  Heals you fully, calls pets, casts charm monster.  Fourteen charges.
		getlin("Speak your incantation:", buf);
			if(!*buf || *buf == '\033'){
				if( (d(1,20)-10) > 0 ){
					exercise(A_WIS, FALSE);
					obj->spe--;//lose charge for false invoke
					pline("Your weapon has become more flawed.");
				}
				else pline("Your weapon rattles warningly.");
				break;
			}
			if ( !(uwep && uwep == obj) ) {
				You_feel("that you should be wielding %s", the(xname(obj)));
				if( (d(1,20)-10) > 0 ){
					obj->spe--;//lose charge for false invoke
					exercise(A_WIS, FALSE);
					pline("Your weapon has become more flawed.");
				}
				else pline("Your weapon rattles warningly.");
		break;
			}
			if(strcmp(buf, "Ruat") == 0 ||
						strcmp(buf, "Ruat.") == 0 ||
						strcmp(buf, "Ruat!") == 0 ){ //Ruat:  Fall.  Makes a pit.  One charge.
				if( (obj->spe > -7) ){
					exercise(A_WIS, TRUE);
					if (!getdir((char *)0)) {
					 if( (d(1,20)-10) > 0 ){
						obj->spe--;//lose charge for false invoke
						exercise(A_WIS, FALSE);
						pline("Your weapon has become more flawed.");
					 }
					}
					else{
						int x, y;
						x = u.ux + u.dx;
						y = u.uy + u.dy;
						if(isok(x, y) && digfarhole(TRUE, x, y)){
							obj->spe--; // lose charge
							pline("Your weapon has become more flawed.");
						} else if( (d(1,20)-10) > 0 ){
							obj->spe--;//lose charge for false invoke
							exercise(A_WIS, FALSE);
							pline("Your weapon has become more flawed.");
						}
					}
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Coelum") == 0 ||
						strcmp(buf, "Coelum.") == 0 ||
						strcmp(buf, "Coelum!") == 0 ){//Coelum: Heaven.  Casts cure.  One charge.
				if( (obj->spe > -7) ){
					exercise(A_WIS, TRUE);
					You_feel("better.");
					healup(d(1 + u.ulevel/3, 4)+ u.ulevel, 0, FALSE, FALSE);
					exercise(A_CON, TRUE);
					obj->spe--; // lose charge
					pline("Your weapon has become more flawed.");
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Fiat") == 0 ||
						strcmp(buf, "Fiat.") == 0 ||
						strcmp(buf, "Fiat!") == 0 ){//Fiat:  Let it be.  Creates food.  One charge.
				if( (obj->spe > -7)){
					exercise(A_WIS, TRUE);
					otmp = mksobj(FOOD_RATION, TRUE, FALSE);
					hold_another_object(otmp, "Suddenly %s out.",
				       aobjnam(otmp, "fall"), (const char *)0);
					obj->spe--; // lose charge
					pline("Your weapon has become more flawed.");
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Justitia") == 0 ||
						strcmp(buf, "Justitia.") == 0 ||
						strcmp(buf, "Justitia!") == 0 ){//Justitia:  Justice.  Creates light.  One charge.
				if( (obj->spe > -7) ){
					exercise(A_WIS, TRUE);
					exercise(A_WIS, TRUE);
					pseudo = mksobj(SPE_LIGHT, FALSE, FALSE);
					pseudo->blessed = pseudo->cursed = 0;
					pseudo->quan = 20L;			/* do not let useup get it */
					litroom(TRUE,pseudo);
					obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
					obj->spe--; // lose charge
					pline("Your weapon has become more flawed.");
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Ecce") == 0 ||
						strcmp(buf, "Ecce.") == 0 ||
						strcmp(buf, "Ecce!") == 0 ){//Ecce:  See.  Casts detect monsters or detect unseen.  Two charges.
				if( (obj->spe > -6) ){
					exercise(A_WIS, TRUE);
//					switch(d(1,2)){
//					case 1:
						pseudo = mksobj(SPE_DETECT_MONSTERS, FALSE, FALSE);
						pseudo->blessed = pseudo->cursed = 0;
						pseudo->blessed = TRUE;
						pseudo->quan = 20L;			/* do not let useup get it */
						(void) peffects(pseudo);
						obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
//					break;
//					case 2:
						pseudo = mksobj(SPE_DETECT_UNSEEN, FALSE, FALSE);
						pseudo->blessed = pseudo->cursed = 0;
						pseudo->blessed = TRUE;
						pseudo->quan = 20L;			/* do not let useup get it */
						weffects(pseudo);
						obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
//					break;
//					}

					obj->spe--; obj->spe--; // lose two charge
					pline("Your weapon has become more flawed!");
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Lex") == 0 ||
						strcmp(buf, "Lex.") == 0 ||
						strcmp(buf, "Lex!") == 0 ){//Lex:  Law.  Grants you enlightenment.  Two charges.
				if((obj->spe > -6)){
					exercise(A_WIS, TRUE);
					exercise(A_WIS, TRUE);
					enlightenment(FALSE); //not dead yet!
					unrestrict_weapon_skill(P_SPEAR);
				    discover_artifact(ART_ROD_OF_SEVEN_PARTS);
					identify(obj);
					update_inventory();
					obj->spe--; obj->spe--; // lose two charge
					pline("Your weapon has become more flawed!");
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Rex") == 0){//Rex:  King.  Grants Levitation.  Three charges.
				if( (u.RoSPflights > 0) ){
					pseudo = mksobj(SPE_LEVITATION, FALSE, FALSE);
					pseudo->blessed = pseudo->cursed = 0;
					pseudo->blessed = TRUE;
					pseudo->quan = 23L;			/* do not let useup get it */
					(void) peffects(pseudo);
					(void) peffects(pseudo);
					(void) peffects(pseudo);
					obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
					u.RoSPflights--;
				}
				else if( (obj->spe > -5) ){
					exercise(A_WIS, TRUE);
					exercise(A_DEX, TRUE);
					pseudo = mksobj(SPE_LEVITATION, FALSE, FALSE);
					pseudo->blessed = pseudo->cursed = 0;
					pseudo->blessed = TRUE;
					pseudo->quan = 23L;			/* do not let useup get it */
					(void) peffects(pseudo);
					(void) peffects(pseudo);
					(void) peffects(pseudo);
					obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
					obj->spe--; obj->spe--; obj->spe--; // lose three charge
					pline("Your weapon has become much more flawed!");
					u.RoSPflights = 7;
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Ruat Coelum") == 0 ||
						strcmp(buf, "Ruat Coelum.") == 0 ||
						strcmp(buf, "Ruat Coelum!") == 0 ){//Ruat Coelum:  Heaven Falls.  Attack spell.  One charge.
				if( (obj->spe > 0) && throweffect()){
					exercise(A_WIS, TRUE);
					cc.x=u.dx;cc.y=u.dy;
					n=u.ulevel/5 + 1;
					while(n--) {
						int type = d(1,3);
						explode(u.dx, u.dy,
							elements[type],
							u.ulevel/2 + 1 + spell_damage_bonus(), 0,
							explType[type]);
						u.dx = cc.x+rnd(3)-2; u.dy = cc.y+rnd(3)-2;
						if (!isok(u.dx,u.dy) || !cansee(u.dx,u.dy) ||
							IS_STWALL(levl[u.dx][u.dy].typ) || u.uswallow) {
							/* Spell is reflected back to center */
								u.dx = cc.x;
								u.dy = cc.y;
						}
					}
					obj->spe--; // lose one charge
					pline("Your weapon has become more flawed.");
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Fiat Justitia") == 0 ||
						strcmp(buf, "Fiat Justitia.") == 0 ||
						strcmp(buf, "Fiat Justitia!") == 0 ){//Fiat Justitia:  Let justice be done.  Slows, fears and damages target.  Two charges.
				if( (obj->spe > 1) ){
					exercise(A_WIS, TRUE);
					pseudo = mksobj(SPE_SLOW_MONSTER, FALSE, FALSE);
					pseudo->blessed = pseudo->cursed = 0;
					pseudo->quan = 20L;			/* do not let useup get it */
					pseudo2 = mksobj(SPE_FORCE_BOLT, FALSE, FALSE);
					pseudo2->blessed = pseudo2->cursed = 0;
					pseudo2->quan = 20L;			/* do not let useup get it */
					pseudo3 = mksobj(SPE_CAUSE_FEAR, FALSE, FALSE);
					pseudo3->blessed = pseudo3->cursed = 0;
					pseudo3->quan = 20L;			/* do not let useup get it */
					if(u.ulevel > 13) pseudo3->blessed = TRUE;
					if (!getdir((char *)0)) { //Oh, getdir must set the .d_ variables below.
					    /* getdir cancelled, just do the nondirectional scroll */
						(void) seffects(pseudo3);
					}
					else if(!u.dx && !u.dy && !u.dz) {
					    if ((damage = zapyourself(pseudo2, TRUE)) != 0) {
							char buf[BUFSZ];
							Sprintf(buf, "zapped %sself with a spell", uhim());
							losehp(damage, buf, NO_KILLER_PREFIX);
							weffects(pseudo);
							(void) seffects(pseudo3);
						}
					}
					else {
						weffects(pseudo);
						weffects(pseudo2);
						(void) seffects(pseudo3);
					}
					obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
					obfree(pseudo2, (struct obj *)0);	/* now, get rid of it */
					obfree(pseudo3, (struct obj *)0);	/* now, get rid of it */
					obj->spe--; obj->spe--; // lose two charge
					pline("Your weapon has become more flawed!");
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Lex Rex") == 0 ||
						strcmp(buf, "Lex Rex.") == 0 ||
						strcmp(buf, "Lex Rex!") == 0 ){//Lex Rex:  Law is King.  Heals aflictions, removes curses.  Three charges.
				if( (obj->spe > 2) ){
					exercise(A_WIS, TRUE);
					pseudo = mksobj(SPE_REMOVE_CURSE, FALSE, FALSE);
					pseudo->blessed = pseudo->cursed = 0;
					pseudo->quan = 20L;			/* do not let useup get it */
					if(u.ulevel > 13) pseudo->blessed = TRUE;
					(void) seffects(pseudo);
					if (Sick) You("are no longer ill.");
					if (Slimed) {
						pline_The("slime disappears!");
						Slimed = 0;
					 /* flags.botl = 1; -- healup() handles this */
					}
					healup(u.ulevel, 0, TRUE, TRUE);
					obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
					obj->spe--; obj->spe--; obj->spe--; // lose three charge
					pline("Your weapon has become much more flawed!");
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Ecce! Lex Rex") == 0 ||
						strcmp(buf, "Ecce! Lex Rex.") == 0 ||
						strcmp(buf, "Ecce! Lex Rex!") == 0 ||
						strcmp(buf, "Ecce!  Lex Rex") == 0 ||
						strcmp(buf, "Ecce!  Lex Rex.") == 0 ||
						strcmp(buf, "Ecce!  Lex Rex!") == 0){//Ecce!  Lex Rex:  See!  Law is King.  Charms monsters.  Five charges.
				if( (obj->spe > 4) ){
					pseudo = mksobj(SPE_CHARM_MONSTER, FALSE, FALSE);
					pseudo->blessed = pseudo->cursed = 0;
					pseudo->quan = 20L;			/* do not let useup get it */
					if(u.ulevel > 13) pseudo->blessed = TRUE;
					(void) seffects(pseudo);
					obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
					obj->spe = obj->spe - 5;// lose five charge
					pline("Your weapon rattles alarmingly.  It has become much more flawed!");
					wake_nearto(u.ux, u.uy, 7);
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Ruat Coelum, Fiat Justitia") == 0 ||
						strcmp(buf, "Ruat Coelum, Fiat Justitia.") == 0 ||
						strcmp(buf, "Ruat Coelum, Fiat Justitia!") == 0){//Ruat Coelum, Fiat Justitia:  Though heaven falls, let justice be done.  Calls pets.  Seven charges.
				if( (obj->spe > 6) ){
					exercise(A_WIS, TRUE);
					n=u.ulevel/5 + 1;
					cast_protection();
					while(n--) {
						pm = &mons[summons[d(1,8)]];
						mtmp = makemon(pm, u.ux, u.uy, MM_EDOG|MM_ADJACENTOK|NO_MINVENT|MM_NOCOUNTBIRTH);
						initedog(mtmp);
						if(u.ulevel > 12) mtmp->m_lev += u.ulevel / 3;
						mtmp->mhpmax = (mtmp->m_lev * 8) - 4;
						mtmp->mhp =  mtmp->mhpmax;
						mtmp->mtame = 10;
						mtmp->mpeaceful = 1;
					}
					obj->spe = obj->spe - 7;// lose seven charge
					pline("Your weapon rattles alarmingly!  It has become exceedingly flawed!");
					wake_nearto(u.ux, u.uy, 21);
				} else pline("Your weapon rattles faintly.");
			}
			else if(strcmp(buf, "Ruat Coelum, Fiat Justitia.  Ecce!  Lex Rex!") == 0 ||
						strcmp(buf, "Ruat Coelum, Fiat Justitia. Ecce! Lex Rex!") == 0){//Ruat Coelum, Fiat Justitia.  Ecce!  Lex Rex!:  Though heaven falls, let justice be done.  See!  Law is King!  Heals you fully, calls pets, casts charm monster.
				if( (obj->spe > 6) ){
					exercise(A_WIS, TRUE);
					n=u.ulevel/5 + 1;
					if (Sick) You("are no longer ill.");
					if (Slimed) {
						pline_The("slime disappears!");
						Slimed = 0;
					 /* flags.botl = 1; -- healup() handles this */
					}
					healup(u.uhpmax - u.uhp, 0, TRUE, TRUE); //heal spell
					while(n--) {
						pm = &mons[summons[d(1,8)]];
						mtmp = makemon(pm, u.ux, u.uy, MM_EDOG|MM_ADJACENTOK);
						initedog(mtmp);
						mtmp->m_lev += 7;
						if(u.ulevel > 12) mtmp->m_lev += u.ulevel / 3;
						mtmp->mhpmax = (mtmp->m_lev * 8) - 4;
						mtmp->mhp =  mtmp->mhpmax;
					}
					pseudo = mksobj(SPE_CHARM_MONSTER, FALSE, FALSE);
					pseudo->blessed = pseudo->cursed = 0;
					pseudo->quan = 20L;			/* do not let useup get it */
					if(u.ulevel > 13) pseudo->blessed = TRUE;
					(void) seffects(pseudo);
					obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
					u.RoSPflights = 7; /* Max out your flight gauge */
					obj->spe = obj->spe - 14;// lose fourteen charge
					pline("Your weapon rattles alarmingly!!  It has become much more flawed!");
					wake_nearto(u.ux, u.uy, 77);
				} else pline("Your weapon rattles faintly.");
			}
			else {
				You_feel("foolish.", the(xname(obj)));
				exercise(A_WIS, FALSE);
				if( (d(1,20)-10) > 0 ){
					obj->spe--;//lose charge for false invoke
					exercise(A_WIS, FALSE);
					pline("Your weapon has become more flawed.");
				}
				else pline("Your weapon rattles warningly.");
			}
		break;
		case PETMASTER:{
			int pet_effect = 0;
			if(uarm && uarm == obj && yn("Take something out of your pockets?") == 'y'){
				pet_effect = dopetmenu("Take what out of your pockets?", obj);
				switch(pet_effect){
					case 0:
					break;
					case SELECT_WHISTLE:
					otmp = mksobj(MAGIC_WHISTLE, TRUE, FALSE);
					otmp->blessed = obj->blessed;
					otmp->cursed = obj->cursed;
					otmp->bknown = obj->bknown;
					hold_another_object(otmp, "You fumble and %s.",
				       aobjnam(otmp, "fall"), (const char *)0);
					break;
					case SELECT_LEASH:
					otmp = mksobj(LEASH, TRUE, FALSE);
					otmp->blessed = obj->blessed;
					otmp->cursed = obj->cursed;
					otmp->bknown = obj->bknown;
					hold_another_object(otmp, "You fumble and %s.",
				       aobjnam(otmp, "fall"), (const char *)0);
					break;
					case SELECT_SADDLE:
					otmp = mksobj(SADDLE, TRUE, FALSE);
					otmp->blessed = obj->blessed;
					otmp->cursed = obj->cursed;
					otmp->bknown = obj->bknown;
					hold_another_object(otmp, "You fumble and %s.",
				       aobjnam(otmp, "fall"), (const char *)0);
					break;
					case SELECT_TRIPE:
					otmp = mksobj(TRIPE_RATION, TRUE, FALSE);
					otmp->blessed = obj->blessed;
					otmp->cursed = obj->cursed;
					otmp->bknown = obj->bknown;
					hold_another_object(otmp, "You fumble and %s.",
				       aobjnam(otmp, "fall"), (const char *)0);
					break;
					case SELECT_APPLE:
					otmp = mksobj(APPLE, TRUE, FALSE);
					otmp->blessed = obj->blessed;
					otmp->cursed = obj->cursed;
					otmp->bknown = obj->bknown;
					hold_another_object(otmp, "You fumble and %s.",
				       aobjnam(otmp, "fall"), (const char *)0);
					break;
					case SELECT_BANANA:
					otmp = mksobj(BANANA, TRUE, FALSE);
					otmp->blessed = obj->blessed;
					otmp->cursed = obj->cursed;
					otmp->bknown = obj->bknown;
					hold_another_object(otmp, "You fumble and %s.",
				       aobjnam(otmp, "fall"), (const char *)0);
					break;
				}
			}else if(uarm && uarm == obj && yn("Restore discipline?") == 'y'){
				(void) pet_detect_and_tame(obj);
			}else{
				if(!(uarm && uarm == obj)) You_feel("that you should be wearing the %s", xname(obj));
				obj->age = 0; //didn't invoke
			}
		} break;
		case NECRONOMICON:
		   if(yn("Open the Necronomicon?")=='y'){
			if(Blind){
				You_cant("feel any Braille writing.");
		break;
			}
			if(!u.uevent.uread_necronomicon){
				if(obj->ovar1) You("find notes scribbled the margins!  These will come in handy!");
				u.uevent.uread_necronomicon = 1;
			}
			discover_artifact(ART_NECRONOMICON);
			identify(obj);
			update_inventory();
			if(obj->ovar1 && yn("Read a known incantation?") == 'y'){
				int booktype = 0;
				u.uconduct.literate++;
				necro_effect = donecromenu("Choose which incantation to read", obj);
				//first switch sets the delay if a new effect, or finds the spellbook if a spell effect
				switch(necro_effect){
					case 0:
						delay = 0;
					break;
					case SELECT_BYAKHEE:
						delay = -2;
					break;
					case SELECT_NIGHTGAUNT:
						delay = -3;
					break;
					case SELECT_SHOGGOTH:
						delay = -2;
					break;
					case SELECT_OOZE:
						delay = -1;
					break;
					case SELECT_DEMON:
						delay = -6;
					break;
					case SELECT_DEVIL:
						delay = -9;
					break;
					case SELECT_PROTECTION:
						booktype = SPE_PROTECTION;
					break;
					case SELECT_TURN_UNDEAD:
						booktype = SPE_TURN_UNDEAD;
					break;
					case SELECT_FORCE_BOLT:
						booktype = SPE_FORCE_BOLT;
					break;
					case SELECT_DRAIN_LIFE:
						booktype = SPE_DRAIN_LIFE;
					break;
					case SELECT_DEATH:
						booktype = SPE_FINGER_OF_DEATH;
					break;
					case SELECT_DETECT_MNSTR:
						booktype = SPE_DETECT_MONSTERS;
					break;
					case SELECT_CLAIRVOYANCE:
						booktype = SPE_CLAIRVOYANCE;
					break;
					case SELECT_DETECT_UNSN:
						booktype = SPE_DETECT_UNSEEN;
					break;
					case SELECT_IDENTIFY:
						booktype = SPE_IDENTIFY;
					break;
					case SELECT_CONFUSE:
						booktype = SPE_CONFUSE_MONSTER;
					break;
					case SELECT_CAUSE_FEAR:
						booktype = SPE_CAUSE_FEAR;
					break;
					case SELECT_LEVITATION:
						booktype = SPE_LEVITATION;
					break;
					case SELECT_STONE_FLESH:
						booktype = SPE_STONE_TO_FLESH;
					break;
					case SELECT_CANCELLATION:
						booktype = SPE_CANCELLATION;
					break;
					case SELECT_COMBAT:
						delay = -100;
					break;
					case SELECT_HEALTH:
						delay = 0;
					break;
					case SELECT_SIGN:
						delay = -100;
					break;
					case SELECT_WARDS:
						delay = -60;
					break;
					case SELECT_ELEMENTS:
						delay = -100;
					break;
					case SELECT_SPIRITS1:
						delay = -125;
					break;
					case SELECT_SPIRITS2:
						delay = -125;
					break;
				}
				//if it WAS a spellbook effect, set the delay here
				if(booktype) switch (objects[booktype].oc_level) {
				 case 1:
				 case 2:
					delay = -objects[booktype].oc_delay;
					break;
				 case 3:
				 case 4:
					delay = -(objects[booktype].oc_level - 1) *
						objects[booktype].oc_delay;
					break;
				 case 5:
				 case 6:
					delay = -objects[booktype].oc_level *
						objects[booktype].oc_delay;
					break;
				 case 7:
					delay = -8 * objects[booktype].oc_delay;
					break;
				 default:
					impossible("Unknown spellbook level %d, book %d;",
						objects[booktype].oc_level, booktype);
					return 0;
				}
				artiptr = obj;
				set_occupation(read_necro, "studying", 0);
			}
			else if(yn(obj->ovar1 ? "Study the parts you don't yet understand?" : "Study the text?") == 'y'){
				int chance = 0;
				u.uconduct.literate++;
				You("struggle to understand the mad scrawlings of Abdul Alhazred and the horrid, rushed annotations of those who came after him.");
				necro_effect = SELECT_STUDY;
				delay = -100;
				artiptr = obj;
				set_occupation(read_necro, "studying", 0);
				exercise(A_WIS, FALSE);
			}
			else{
				if(Hallucination) pline("The strange symbols stare at you reproachfully.");
				You("close the Necronomicon.");
			}
		   }
		   else{
				Hallucination ? 
					pline("Cool it, Link.  It's just a book.") : 
					You("hold the Necronomicon awkwardly, then put it away.");
		   }
		break;
		case SPIRITNAMES:{
		   if(yn("Open the Book of Lost Names?")=='y'){
			if(Blind){
				You_cant("feel any Braille writing.");
		break;
			}
			discover_artifact(ART_BOOK_OF_LOST_NAMES);
			identify(obj);
			update_inventory();
			if(obj->ovar1 && yn("Contact a known spirit?") == 'y'){
				long yourseals = u.sealsKnown;
				u.sealsKnown = obj->ovar1;
				u.uconduct.literate++;
				lostname = pick_seal();
				if(!lostname) break;
				delay = -25;
				artiptr = obj;
				u.sealsKnown = yourseals;
				set_occupation(read_lost, "studying", 0);
			}
			else if(yn("Risk your name amongst the Lost?") == 'y'){
				int chance = 0;
				u.uconduct.literate++;
				You("open your mind to the cold winds of the Void.");
				lostname = QUEST_SPIRITS;
				delay = -125;
				artiptr = obj;
				set_occupation(read_lost, "studying", 0);
				exercise(A_WIS, FALSE);
			}
			else{
				if(Hallucination) pline("The whisperers berate you ceaselessly.");
				You("close the Book of Lost Names.");
			}
		   }
		   else{
				Hallucination ? 
					pline("Cool it, Link.  It's just a book.") : 
					You("hold the Book of Lost Names awkwardly, then put it away.");
		   }
		}break;
		case INFINITESPELLS:{
			pline("The endless pages of the book cover the material of a spellbook of %s in exhaustive detail.",OBJ_NAME(objects[obj->ovar1]));
			spelleffects(0,FALSE,obj->ovar1);
			if(!rn2(20)){
				obj->ovar1 = rn2(SPE_BLANK_PAPER - SPE_DIG) + SPE_DIG;
			}
		}break;
		case LORDLY:
			if(uwep && uwep == obj){
				//struct obj *otmp;
				int lordlydictum = dolordsmenu("What is your command, my Lord?", obj);
				switch(lordlydictum){
					case 0:
					break;
					/*These effects can be used at any time*/
					case COMMAND_RAPIER:
						uwep->otyp = RAPIER;
					break;
					case COMMAND_AXE:
						uwep->otyp = AXE;
					break;
					case COMMAND_MACE:
						uwep->otyp = MACE;
					break;
					case COMMAND_SPEAR:
						uwep->otyp = SPEAR;
					break;
					case COMMAND_LANCE:
						uwep->otyp = LANCE;
					break;
					/*These effects are limited by timeout*/
					case COMMAND_LADDER:
						if(u.uswallow){
							mtmp = u.ustuck;
							if (!is_whirly(mtmp->data)) {
								if (is_animal(mtmp->data))
									pline("The Rod quickly lengthens and pierces %s %s wall!",
									s_suffix(mon_nam(mtmp)), mbodypart(mtmp, STOMACH));
								if(mtmp->data == &mons[PM_JUIBLEX] 
									|| mtmp->data == &mons[PM_LEVIATHAN]
									|| mtmp->data == &mons[PM_METROID_QUEEN]
								) mtmp->mhp = (int)(.75*mtmp->mhp + 1);
								else mtmp->mhp = 1;		/* almost dead */
								expels(mtmp, mtmp->data, !is_animal(mtmp->data));
							} else{
								pline("The Rod quickly lengthens and pierces %s %s",
									s_suffix(mon_nam(mtmp)), mbodypart(mtmp, STOMACH));
								pline("However, %s is unfazed", mon_nam(mtmp));
							}
					break;
						} else if(!u.uhave.amulet){
							if(Can_rise_up(u.ux, u.uy, &u.uz)) {
								int newlev = depth(&u.uz)-1;
								d_level newlevel;
								
								pline("The Rod extends quickly, reaching for the %s",ceiling(u.ux,u.uy));
								
								get_level(&newlevel, newlev);
								if(on_level(&newlevel, &u.uz)) {
									pline("However, it is unable to pierce the %s.",ceiling(u.ux,u.uy));
					break;
								} else pline("The %s obediently yields before the Rod, and you climb to the level above.",ceiling(u.ux,u.uy));
								goto_level(&newlevel, FALSE, FALSE, FALSE);
							}
						} else{
							if (!Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz) && !Underwater) {
								pline("The Rod begins to extend quickly upwards.");
								pline("However, a mysterious force slams it back into the %s below.", surface(u.ux, u.uy));
								watch_dig((struct monst *)0, u.ux, u.uy, TRUE);
								(void) dighole(FALSE);
							}
						}
					break;
					case COMMAND_CLAIRVOYANCE:
						do_vicinity_map(u.ux,u.uy); /*Note that this is not blocked by pointy hats*/
					break;
					case COMMAND_FEAR:
						You("thrust the Rod into the air, that all may know of your Might.");
						for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
							if (DEADMONSTER(mtmp)) continue;
							if(mtmp->mcansee && couldsee(mtmp->mx,mtmp->my)) {
//								if (! resist(mtmp, sobj->oclass, 0, NOTELL))
								monflee(mtmp, 0, FALSE, FALSE);
							}
						}
					break;
					case COMMAND_LIFE:
						if(!getdir((char *)0) ||
							(!u.dx && !u.dy) ||
							((mtmp = m_at(u.ux+u.dx,u.uy+u.dy)) == 0)
						){
							pline("The Rod glows and then fades.");
						} else {
							int dmg = d(2,8);
							int rcvr;
							if (resists_drli(mtmp)){
								shieldeff(mtmp->mx, mtmp->my);
					break;
							} else {
								mtmp->mhp -= 2*dmg;
								mtmp->mhpmax -= dmg;
								mtmp->m_lev -= 2;
								if (mtmp->mhp <= 0 || mtmp->mhpmax <= 0 || mtmp->m_lev < 1)
									xkilled(mtmp, 1);
								else {
								if (canseemon(mtmp))
									pline("%s suddenly seems weaker!", Monnam(mtmp));
								}
								healup(2*dmg, 0, FALSE, TRUE);
								You_feel("better.");
							}
						}
					break;
					case COMMAND_KNEEL:
						if(!getdir((char *)0) ||
							(!u.dx && !u.dy) ||
							((mtmp = m_at(u.ux+u.dx,u.uy+u.dy)) == 0)
						){
							pline("The Rod glows and then fades.");
						} else{
							mtmp->mcanmove = 0;
							mtmp->mfrozen = max(1, u.ulevel - ((int)(mtmp->m_lev)));
							pline("%s kneels before you.",Monnam(mtmp));
						}
					break;
					default:
						pline("What is this strange command!?");
					break;
				}
				if(lordlydictum >= COMMAND_LADDER) obj->age = monstermoves + (long)(rnz(100)*(Role_if(PM_PRIEST) ? .8 : 1)); 
			} else You_feel("that you should be wielding %s", the(xname(obj)));;
		break;
		case VOID_CHIME:
			if(quest_status.killed_nemesis){
				int i;
				u.voidChime = 5;
				pline("You strike the twin-bladed athame like a tuning fork. The beautiful chime is like nothing you have ever heard.");
				obj->ovar1 |= u.sealsActive;
				u.sealsActive |= obj->ovar1;
				set_spirit_powers(u.spiritTineA);
					if(u.spiritTineA&wis_spirits) u.wisSpirits++;
					if(u.spiritTineA&int_spirits) u.intSpirits++;
				set_spirit_powers(u.spiritTineB);
					if(u.spiritTineB&wis_spirits) u.wisSpirits++;
					if(u.spiritTineB&int_spirits) u.intSpirits++;
				for(i=0;i < NUMBER_POWERS;i++){
					u.spiritPColdowns[i] = 0;
				}
				if(obj == uwep && uwep->lamplit && artifact_light(obj)) begin_burn(uwep, FALSE);
			}
			else pline("You strike the single-bladed athame, but nothing happens.");
		break;
		default: pline("Program in dissorder.  Artifact invoke property not recognized");
		break;
	} //end of first case:  Artifact Specials!!!!

    } else {
	long eprop = (u.uprops[oart->inv_prop].extrinsic ^= W_ARTI),
	     iprop = u.uprops[oart->inv_prop].intrinsic;
	boolean on = (eprop & W_ARTI) != 0; /* true if invoked prop just set */

	if(on && obj->age > monstermoves) {
	    /* the artifact is tired :-) */
	    u.uprops[oart->inv_prop].extrinsic ^= W_ARTI;
	    You_feel("that %s %s ignoring you.",
		     the(xname(obj)), otense(obj, "are"));
	    /* can't just keep repeatedly trying */
	    obj->age += (long) d(3,10);
	    return 1;
	} else if(!on) {
	    /* when turning off property, determine downtime */
	    /* arbitrary for now until we can tune this -dlc */
//	    obj->age = monstermoves + rnz(100);
		obj->age = monstermoves + (long)(rnz(100)*(Role_if(PM_PRIEST) ? .75 : 1));
	}

	if ((eprop & ~W_ARTI) || iprop) {
nothing_special:
	    /* you had the property from some other source too */
	    if (carried(obj))
		You_feel("a surge of power, but nothing seems to happen.");
	    return 1;
	}
	switch(oart->inv_prop) {
	case CONFLICT:
	    if(on) You_feel("like a rabble-rouser.");
	    else You_feel("the tension decrease around you.");
	    break;
	case LEVITATION:
	    if(on) {
		float_up();
		spoteffects(FALSE);
	    } else (void) float_down(I_SPECIAL|TIMEOUT, W_ARTI);
	    break;
	case INVIS:
	    if (BInvis || Blind) goto nothing_special;
	    newsym(u.ux, u.uy);
	    if (on)
		Your("body takes on a %s transparency...",
		     Hallucination ? "normal" : "strange");
	    else
		Your("body seems to unfade...");
	    break;
		
	}
    }

    return 1;
}

int
donecromenu(prompt, obj)
const char *prompt;
struct obj *obj;
{
	winid tmpwin;
	int n, how;
	char buf[BUFSZ];
	char incntlet = 'a';
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */

	Sprintf(buf, "Known Passages");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	if(obj->ovar1 & S_BYAKHEE){
		Sprintf(buf, "Summon byakhee");
		any.a_int = SELECT_BYAKHEE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & S_NIGHTGAUNT){
		Sprintf(buf, "Summon Night-gaunt");
		any.a_int = SELECT_NIGHTGAUNT;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & S_SHOGGOTH){
		Sprintf(buf, "Summon Shoggoth");
		any.a_int = SELECT_SHOGGOTH;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & S_OOZE){
		Sprintf(buf, "Summon Ooze");
		any.a_int = SELECT_OOZE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SUM_DEMON){
		Sprintf(buf, "Summon Demon");
		any.a_int = SELECT_DEMON;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & S_DEVIL){
		Sprintf(buf, "Summon Devil");
		any.a_int = SELECT_DEVIL;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_PROTECTION){
		Sprintf(buf, "Study secrets of defense");
		any.a_int = SELECT_PROTECTION;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_TURN_UNDEAD){
		Sprintf(buf, "Study secrets of life and death");
		any.a_int = SELECT_TURN_UNDEAD;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_FORCE_BOLT){
		Sprintf(buf, "Study secrets of the intangible forces");
		any.a_int = SELECT_FORCE_BOLT;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_DRAIN_LIFE){
		Sprintf(buf, "Study secrets of time and decay");
		any.a_int = SELECT_DRAIN_LIFE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_DEATH){
		Sprintf(buf, "Study secrets of the Reaper");
		any.a_int = SELECT_DEATH;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_DETECT_MNSTR){
		Sprintf(buf, "Study secrets of monster detection");
		any.a_int = SELECT_DETECT_MNSTR;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_CLAIRVOYANCE){
		Sprintf(buf, "Study secrets of clairvoyance");
		any.a_int = SELECT_CLAIRVOYANCE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_DETECT_UNSN){
		Sprintf(buf, "Study secrets of unseen things");
		any.a_int = SELECT_DETECT_UNSN;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_IDENTIFY){
		Sprintf(buf, "Study secrets of knowledge without learning");
		any.a_int = SELECT_IDENTIFY;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_CONFUSE){
		Sprintf(buf, "Study secrets of confusion");
		any.a_int = SELECT_CONFUSE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_CAUSE_FEAR){
		Sprintf(buf, "Study secrets of fear");
		any.a_int = SELECT_CAUSE_FEAR;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_LEVITATION){
		Sprintf(buf, "Study secrets of gravity");
		any.a_int = SELECT_LEVITATION;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_STONE_FLESH){
		Sprintf(buf, "Study secrets of the earth mother");
		any.a_int = SELECT_STONE_FLESH;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_CANCELLATION){
		Sprintf(buf, "Study secrets of fraying the weave");
		any.a_int = SELECT_CANCELLATION;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_COMBAT){
		Sprintf(buf, "Study secrets of war and killing");
		any.a_int = SELECT_COMBAT;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & SP_HEALTH){
		Sprintf(buf, "Study secrets of health and recovery");
		any.a_int = SELECT_HEALTH;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & R_Y_SIGN){
		Sprintf(buf, "Study the legends of lost Carcosa");
		any.a_int = SELECT_SIGN;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & R_WARDS){
		Sprintf(buf, "Study up on ancient warding signs");
		any.a_int = SELECT_WARDS;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & R_ELEMENTS){
		Sprintf(buf, "Study the Lords of the Elements");
		any.a_int = SELECT_ELEMENTS;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & R_NAMES_1){
		Sprintf(buf, "Study the first half of the testament of whispers");
		any.a_int = SELECT_SPIRITS1;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(obj->ovar1 & R_NAMES_2){
		Sprintf(buf, "Study the second half of the testament of whispers");
		any.a_int = SELECT_SPIRITS2;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	end_menu(tmpwin, prompt);

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return (n < SELECT_STUDY && n > 0) ? selected[0].item.a_int : 0;
}

int
dopetmenu(prompt, obj)
const char *prompt;
struct obj *obj;
{
	winid tmpwin;
	int n, how;
	char buf[BUFSZ];
	char incntlet = 'a';
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */

	Sprintf(buf, "What will you take out?");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	Sprintf(buf, "Magic whistle");
	any.a_int = SELECT_WHISTLE;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);
	incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	
	Sprintf(buf, "Leash");
	any.a_int = SELECT_LEASH;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);
	incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	
	
	Sprintf(buf, "Saddle");
	any.a_int = SELECT_SADDLE;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);
	incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	
	
	Sprintf(buf, "Tripe");
	any.a_int = SELECT_TRIPE;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);
	incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	
	
	Sprintf(buf, "Apple");
	any.a_int = SELECT_APPLE;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);
	incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	
	
	Sprintf(buf, "Banana");
	any.a_int = SELECT_BANANA;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);
	incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	
	end_menu(tmpwin, prompt);

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return (n > 0) ? selected[0].item.a_int : 0;
}

int
dolordsmenu(prompt, obj)
const char *prompt;
struct obj *obj;
{
	winid tmpwin;
	int n, how;
	char buf[BUFSZ];
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */

	Sprintf(buf, "What do you command?");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	
	if(obj->otyp != RAPIER){
		Sprintf(buf, "Become a rapier");
		any.a_int = COMMAND_RAPIER;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'r', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	
	if(obj->otyp != AXE){
		Sprintf(buf, "Become an axe");
		any.a_int = COMMAND_AXE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'a', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	
	if(obj->otyp != MACE){
		Sprintf(buf, "Become a mace");
		any.a_int = COMMAND_MACE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'm', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	
	if(obj->otyp != SPEAR){
		Sprintf(buf, "Become a spear");
		any.a_int = COMMAND_SPEAR;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			's', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	
	if(obj->otyp != LANCE){
		Sprintf(buf, "Become a lance");
		any.a_int = COMMAND_LANCE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'l', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	
	if(obj->age < monstermoves){
		if(obj->otyp == MACE && (
		   u.uswallow || 
		   (!u.uhave.amulet && Can_rise_up(u.ux, u.uy, &u.uz)) || 
		   (u.uhave.amulet && !Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz) && !Underwater) 
		  )){
			Sprintf(buf, "Become a ladder");
			any.a_int = COMMAND_LADDER;	/* must be non-zero */
			add_menu(tmpwin, NO_GLYPH, &any,
				'L', 0, ATR_NONE, buf,
				MENU_UNSELECTED);
		}
		
		Sprintf(buf, "Show me my surroundings");
		any.a_int = COMMAND_CLAIRVOYANCE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'S', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		
		Sprintf(buf, "Inspire fear");
		any.a_int = COMMAND_FEAR;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'F', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		
		if(obj->otyp == SPEAR){
			Sprintf(buf, "Give me your life");
			any.a_int = COMMAND_LIFE;	/* must be non-zero */
			add_menu(tmpwin, NO_GLYPH, &any,
				'G', 0, ATR_NONE, buf,
				MENU_UNSELECTED);
		}
		
		Sprintf(buf, "Kneel");
		any.a_int = COMMAND_KNEEL;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'K', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	end_menu(tmpwin, prompt);

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return (n > 0) ? selected[0].item.a_int : 0;
}

STATIC_PTR int
read_necro(VOID_ARGS)
{
	struct permonst *pm;
	struct monst *mtmp = 0;
	int i;
	short booktype;
	char splname[BUFSZ];

	if (Confusion) {		/* became confused while learning */
//	    (void) confused_book(book);
	    artiptr = 0;		/* no longer studying */
	    nomul(delay, "struggling with the Necronomicon");		/* remaining delay is uninterrupted */
	    delay = 0;
	    return(0);
	}
	if (delay) {	/* not if (delay++), so at end delay == 0 */
	/* lenses give 50% faster reading */
//	    nomul( (ublindf && ublindf->otyp == LENSES) ? 
//			(-26+ACURR(A_INT))/2 :
//			(-26+ACURR(A_INT))); /* Necronomicon is difficult to put down */
//	    delay -= -26+ACURR(A_INT); /* - a negative number is + */
		delay++;
		if(ublindf && ublindf->otyp == LENSES) delay++;
		if(delay < 0){
			return(1); /* still busy */
		}
		delay = 0;
	}
	if(necro_effect < SELECTED_SPELL){/* summoning spell*/
		switch(necro_effect){
			case 0:
				Hallucination ? 
					pline("The strange symbols stare at you reproachfully.") :
					You("close the Necronomicon.");
			break;
			case SELECT_BYAKHEE:
				pm = &mons[PM_BYAKHEE];
				if(u.uen >= 20){
					u.uen -= 20;
					for(i=max(1, d(1,20) - 16); i > 0; i--){
						mtmp = makemon(pm, u.ux, u.uy, MM_EDOG|MM_ADJACENTOK);
						initedog(mtmp);
						mtmp->m_lev += d(1,15) - 5;
						if(u.ulevel < mtmp->m_lev){
							mtmp->mtame = 0;
							mtmp->mpeaceful = 0;
							mtmp->mtraitor = 1;
						}
						mtmp->mhpmax = (mtmp->m_lev * 8) - 4;
						mtmp->mhp =  mtmp->mhpmax;
					}
				}
			break;
			case SELECT_NIGHTGAUNT:
				pm = &mons[PM_NIGHTGAUNT];
				for(i=d(1,4); i > 0; i--){
					if(u.uen >= 10){
						u.uen -= 10;
						mtmp = makemon(pm, u.ux, u.uy, MM_EDOG|MM_ADJACENTOK);
						initedog(mtmp);
					}
				}
			break;
			case SELECT_SHOGGOTH:
				if(u.uen > 30){
					pm = &mons[PM_SHOGGOTH];
					mtmp = makemon(pm, u.ux+d(1,5)-3, u.uy+d(1,5)-3, MM_ADJACENTOK);
					mtmp->mcrazed = 1;
					mtmp->msleeping = 1;
				}
			break;
			case SELECT_OOZE:
				if(u.uen >= 20){
					u.uen -= 20;
					for(i=max(1, d(1,10) - 2); i > 0; i--){
						mtmp = makemon(&mons[oozes[d(1,11)]], u.ux+d(1,5)-3, u.uy+d(1,5)-3, MM_EDOG|MM_ADJACENTOK);
						initedog(mtmp);
						mtmp->m_lev += d(1,(3 * mtmp->m_lev)/2);
						if(u.ulevel < mtmp->m_lev){
							mtmp->mtame = 0;
							mtmp->mpeaceful = 0;
							mtmp->mtraitor = 1;
						}
						mtmp->mhpmax = (mtmp->m_lev * 8) - 4;
						mtmp->mhp =  mtmp->mhpmax;
						mtmp->mcrazed = 1;
					}
				}
			break;
			case SELECT_DEVIL:
				if(u.uen >= 60){
					u.uen -= 60;
					mtmp = makemon(&mons[devils[d(1,15)]], u.ux, u.uy, MM_EDOG|MM_ADJACENTOK);
					initedog(mtmp);
					mtmp->m_lev += d(1,(3 * mtmp->m_lev)/2);
					if(!rn2(9)) mtmp->m_lev += d(1,(3 * mtmp->m_lev)/2);
					if(u.ulevel < mtmp->m_lev && rn2(2)){
						mtmp->mtame = 0;
						mtmp->mpeaceful = 0;
						mtmp->mtraitor = 1;
					}
				}
			break;
			case SELECT_DEMON:
				if(u.uen >= 45){
					u.uen -= 45;
					mtmp = makemon(&mons[demons[d(1,15)]], u.ux, u.uy, MM_EDOG|MM_ADJACENTOK);
					initedog(mtmp);
					if(!rn2(6)) mtmp->m_lev += d(1,(3 * mtmp->m_lev)/2);
					if(u.ulevel < mtmp->m_lev || rn2(2)){
						mtmp->mtame = 0;
						mtmp->mpeaceful = 0;
						mtmp->mtraitor = 1;
					}
					mtmp->mcrazed = 1;
				}
			break;
		}
	}
	else if(necro_effect < SELECTED_SPECIAL){ /* spellbook-like */
		switch(necro_effect){
			case SELECT_PROTECTION:
				booktype = SPE_PROTECTION;
			break;
			case SELECT_TURN_UNDEAD:
				booktype = SPE_TURN_UNDEAD;
			break;
			case SELECT_FORCE_BOLT:
				booktype = SPE_FORCE_BOLT;
			break;
			case SELECT_DRAIN_LIFE:
				booktype = SPE_DRAIN_LIFE;
			break;
			case SELECT_DEATH:
				booktype = SPE_FINGER_OF_DEATH;
			break;
			case SELECT_DETECT_MNSTR:
				booktype = SPE_DETECT_MONSTERS;
			break;
			case SELECT_CLAIRVOYANCE:
				booktype = SPE_CLAIRVOYANCE;
			break;
			case SELECT_DETECT_UNSN:
				booktype = SPE_DETECT_UNSEEN;
			break;
			case SELECT_IDENTIFY:
				booktype = SPE_IDENTIFY;
			break;
			case SELECT_CONFUSE:
				booktype = SPE_CONFUSE_MONSTER;
			break;
			case SELECT_CAUSE_FEAR:
				booktype = SPE_CAUSE_FEAR;
			break;
			case SELECT_LEVITATION:
				booktype = SPE_LEVITATION;
			break;
			case SELECT_STONE_FLESH:
				booktype = SPE_STONE_TO_FLESH;
			break;
			case SELECT_CANCELLATION:
				booktype = SPE_CANCELLATION;
			break;
		}
		Sprintf(splname, objects[booktype].oc_name_known ?
				"\"%s\"" : "the \"%s\" spell",
			OBJ_NAME(objects[booktype]));
		for (i = 0; i < MAXSPELL; i++)  {
			if (spellid(i) == booktype)  {
				if (spellknow(i) <= 1000) {
					Your("knowledge of %s is keener.", splname);
					spl_book[i].sp_know += 5000;
					exercise(A_WIS,TRUE);
				} else {
					You("know %s quite well already.", splname);
				}
				/* make book become known even when spell is already
				   known, in case amnesia made you forget the book */
				makeknown((int)booktype);
				break;
			} else if (spellid(i) == NO_SPELL)  {
				spl_book[i].sp_id = booktype;
				spl_book[i].sp_lev = objects[booktype].oc_level;
				spl_book[i].sp_know = 5000;
				You(i > 0 ? "add %s to your repertoire." : "learn %s.",
					splname);
				makeknown((int)booktype);
				break;
			}
		}
		if (i == MAXSPELL) impossible("Too many spells memorized!");
	}
	else if(necro_effect < SELECT_STUDY){/* special effect */
		switch(necro_effect){
			case SELECT_COMBAT:
				You("grasp the basics of your weapon's use.");
				if( uwep && P_MAX_SKILL(objects[uwep->otyp].oc_skill) < P_EXPERT && u.uen >= 100){
					u.uen -= 100;
					u.uenmax -= 20;
					unrestrict_weapon_skill(objects[uwep->otyp].oc_skill);
					u.weapon_skills[objects[uwep->otyp].oc_skill].max_skill = P_EXPERT;
				}
			break;
			case SELECT_HEALTH:
				use_unicorn_horn(artiptr);
			break;
			case SELECT_SIGN:
				if(u.wardsknown & WARD_YELLOW) You("skim the legends of Lost Carcosa");
				else You("read the legends of Lost Carcosa.");
				u.wardsknown |= WARD_YELLOW;
			break;
			case SELECT_WARDS:
				You("review the section on ancient warding signs.");
				u.wardsknown |= WARD_ACHERON|WARD_PENTAGRAM|WARD_HEXAGRAM|WARD_ELDER_SIGN|WARD_HAMSA|WARD_EYE|WARD_QUEEN|WARD_CAT_LORD|WARD_GARUDA;
			break;
			case SELECT_ELEMENTS:
				if( (u.wardsknown & WARD_CTHUGHA) && (u.wardsknown & WARD_ITHAQUA) && (u.wardsknown & WARD_KARAKAL)){
					You("page through the section on the Lords of the Elements");
				}else if((u.wardsknown & WARD_CTHUGHA) || (u.wardsknown & WARD_ITHAQUA) || (u.wardsknown & WARD_KARAKAL)){
					You("refresh your memory about the Lords of the Elements");
				}else{
					You("read about the Lords of the Elements");
				}
				u.wardsknown |= WARD_CTHUGHA|WARD_ITHAQUA|WARD_KARAKAL;
			break;
			case SELECT_SPIRITS1:{
				int i;
				You("read the first half of the testament of whispers.");
				for(i=0; i<16; i++) u.sealsKnown &= sealKey[u.sealorder[i]];
			}break;
			case SELECT_SPIRITS2:{
				int i;
				You("read the second half of the testament of whispers.");
				for(i=15; i<32; i++) u.sealsKnown &= sealKey[u.sealorder[i]];
			}break;
		}
	}
	else if(necro_effect == SELECT_STUDY){
		int chance = 0;
//		pline("learned %d abilities",artiptr->spestudied);
		if(!(artiptr->ovar1)) artiptr->spestudied = 0; /* Sanity enforcement. Something wierd is going on with the artifact creation code.*/
//		pline("learned %d abilities",artiptr->spestudied);
		if(d(1,10) - artiptr->spestudied - 15 + ACURR(A_INT) > 0){
			if(!(artiptr->ovar1 & LAST_PAGE)){
				chance = d(1,27);
			}
			else if(!(artiptr->ovar1 & (S_OOZE|S_SHOGGOTH|S_NIGHTGAUNT|S_BYAKHEE|SUM_DEMON|S_DEVIL) )){
				chance = d(1,6);
			}
		}
		switch(chance){
			case 0:
				You("fail.");
				exercise(A_WIS, FALSE);
				exercise(A_WIS, FALSE);
				exercise(A_INT, FALSE);
			break;
			case 1:
				if(!(artiptr->ovar1 & S_OOZE)){
					You("learn how to bend the myriad amoeboid monsters of earth to your will.");
					artiptr->ovar1 |= S_OOZE;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another incantation for controling amoeboid monsters.");
				}
			break;
			case 2:
				if(!(artiptr->ovar1 & S_SHOGGOTH)){
					You("find a spell for summoning shoggoths.");
					artiptr->ovar1 |= S_SHOGGOTH;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another way to summon shoggoths.");
				}
			break;
			case 3:
				if(!(artiptr->ovar1 & S_NIGHTGAUNT)){
					You("learn to invoke the servetors of Nodens.");
					artiptr->ovar1 |= S_NIGHTGAUNT;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another invocation to Nodens.");
				}
			break;
			case 4:
				if(!(artiptr->ovar1 & S_BYAKHEE)){
					You("learn to call down byakhees.");
					artiptr->ovar1 |= S_BYAKHEE;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another way to call down byakhees.");
				}
			break;
			case 5:
				if(!(artiptr->ovar1 & SUM_DEMON)){
					You("learn to call the children of the abyss.");
					artiptr->ovar1 |= SUM_DEMON;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another way to summon demons.");
				}
			break;
			case 6:
				if(!(artiptr->ovar1 & S_DEVIL)){
					You("learn to free the prisoners of hell.");
					artiptr->ovar1 |= S_DEVIL;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another way to bind devils.");
				}
			break;
			case 7:
				if(!(artiptr->ovar1 & SP_PROTECTION)){
					You("find a spell of protection.");
					artiptr->ovar1 |= SP_PROTECTION;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another spell of protection.");
				}
			break;
			case 8:
				if(!(artiptr->ovar1 & SP_TURN_UNDEAD)){
					You("find a passage concerning the secrets of life and death.");
					artiptr->ovar1 |= SP_TURN_UNDEAD;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another passage concerning life and death.");
				}
			break;
			case 9:
				if(!(artiptr->ovar1 & SP_FORCE_BOLT)){
					You("find a passage detailing the invisible forces.");
					artiptr->ovar1 |= SP_FORCE_BOLT;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another passage describing the invisible forces.");
				}
			break;
			case 10:
				if(!(artiptr->ovar1 & SP_DRAIN_LIFE)){
					You("find a passage concerning the secrets of time and decay.");
					artiptr->ovar1 |= SP_DRAIN_LIFE;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another passage concerning time and decay.");
				}
			break;
			case 11:
				if(!(artiptr->ovar1 & SP_DEATH)){
					You("find a passage detailing the arts of the Reaper.");
					artiptr->ovar1 |= SP_DEATH;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another passage concerning the Reaper's arts.");
				}
			break;
			case 12:
				if(!(artiptr->ovar1 & SP_DETECT_MNSTR)){
					You("find a passage teaching monster detection.");
					artiptr->ovar1 |= SP_DETECT_MNSTR;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find an alternative school of monster detection.");
				}
			break;
			case 13:
				if(!(artiptr->ovar1 & SP_CLAIRVOYANCE)){
					You("learn the secret of clairvoyance.");
					artiptr->ovar1 |= SP_CLAIRVOYANCE;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another reference to the secret of clairvoyance.");
				}
			break;
			case 14:
				if(!(artiptr->ovar1 & SP_DETECT_UNSN)){
					You("learn the ways of unseen things.");
					artiptr->ovar1 |= SP_DETECT_UNSN;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another reference to the ways of unseen things.");
				}
			break;
			case 15:
				if(!(artiptr->ovar1 & SP_IDENTIFY)){
					You("discover the secret of knowledge without learning.");
					artiptr->ovar1 |= SP_IDENTIFY;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("already know the secret of knowledge without learning.");
				}
			break;
			case 16:
				if(!(artiptr->ovar1 & SP_CONFUSE)){
					You("learn the ways of confusion.");
					artiptr->ovar1 |= SP_CONFUSE;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("learn more about the ways of confusion.");
				}
			break;
			case 17:
				if(!(artiptr->ovar1 & SP_CAUSE_FEAR)){
					You("learn the ways of fear.");
					artiptr->ovar1 |= SP_CAUSE_FEAR;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("learn more about the ways of fear.");
				}
			break;
			case 18:
				if(!(artiptr->ovar1 & SP_LEVITATION)){
					You("learn the true nature of gravity.");
					artiptr->ovar1 |= SP_LEVITATION;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("already know the true nature of gravity.");
				}
			break;
			case 19:
				if(!(artiptr->ovar1 & SP_STONE_FLESH)){
					You("learn the true name of the earth mother.");
					artiptr->ovar1 |= SP_STONE_FLESH;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("already know the true name of the earth mother.");
				}
			break;
			case 20:
				if(!(artiptr->ovar1 & SP_CANCELLATION)){
					You("learn the weaknesses of the cosmic weave.");
					artiptr->ovar1 |= SP_CANCELLATION;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("learn a few more weaknesses of the cosmic weave.");
				}
			break;
			case 21:
				if(!(artiptr->ovar1 & SP_COMBAT)){
					You("learn the secrets of war and killing.");
					artiptr->ovar1 |= SP_COMBAT;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("already know the secrets of war and killing.");
				}
			break;
			case 22:
				if(!(artiptr->ovar1 & SP_HEALTH)){
					You("learn the secret to health and recovery.");
					artiptr->ovar1 |= SP_HEALTH;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another secret to health and recovery.");
				}
			break;
			case 23:
				if(!(artiptr->ovar1 & R_Y_SIGN)){
					You("discover the legends of Lost Carcosa.");
					artiptr->ovar1 |= R_Y_SIGN;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("find another section on Lost Carcosa.");
				}
//				u.wardsknown |= WARD_YELLOW;
			break;
			case 24:
				if(!(artiptr->ovar1 & R_WARDS)){
					You("discover a treatise on ancient warding signs.");
					artiptr->ovar1 |= R_WARDS;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("discover another treatise on ancient warding signs.");
				}
//				u.wardsknown |= WARD_ACHERON|WARD_PENTAGRAM|WARD_HEXAGRAM|WARD_ELDER_SIGN|WARD_HAMSA|WARD_EYE|WARD_QUEEN|WARD_CAT_LORD|WARD_GARUDA;
			break;
			case 25:
				if(!(artiptr->ovar1 & R_ELEMENTS)){
					You("discover a section detailing the Lords of the Elements.");
					artiptr->ovar1 |= R_ELEMENTS;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("discover another section on the Lords of the Elements.");
				}
//				u.wardsknown |= WARD_CTHUGHA|WARD_ITHAQUA|WARD_KARAKAL;
			case 26:
				if(!(artiptr->ovar1 & R_NAMES_1)){
					if(artiptr->ovar1 & R_NAMES_2) You("find the first half of the testament of whispers.");
					else You("come across the first half of something called 'the testament of whispers.'");
					artiptr->ovar1 |= R_NAMES_1;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("discover another transcription of the first half of the testament of whispers.");
				}
//				u.wardsknown |= WARD_CTHUGHA|WARD_ITHAQUA|WARD_KARAKAL;
			case 27:
				if(!(artiptr->ovar1 & R_NAMES_2)){
					if(artiptr->ovar1 & R_NAMES_1) You("find the second half of the testament of whispers.");
					else You("come across the latter half of something called 'the testament of whispers.'");
					artiptr->ovar1 |= R_NAMES_2;
					artiptr->spestudied++;
					if(artiptr->spestudied > rn1(4,6)){
						artiptr->ovar1|= LAST_PAGE;
					}
				}
				else{
					You("discover another transcription of the latter half of the testament of whispers.");
				}
//				u.wardsknown |= WARD_CTHUGHA|WARD_ITHAQUA|WARD_KARAKAL;
			break;
		}
//		artiptr->ovar1 = ~0; //debug
	}
	else{
		pline("Unrecognized Necronomicon effect.");
	}
	artiptr = 0;
	return(0);
}

STATIC_PTR int
read_lost(VOID_ARGS)
{
	struct permonst *pm;
	struct monst *mtmp = 0;
	int i, numSlots;
	short booktype;
	char splname[BUFSZ];

	if (Confusion) {		/* became confused while learning */
//	    (void) confused_book(book);
	    artiptr = 0;		/* no longer studying */
	    nomul(delay, "struggling with the Book of Lost Names");		/* remaining delay is uninterrupted */
		losexp("getting lost in a book",TRUE,TRUE,TRUE);
	    delay = 0;
	    return(0);
	}
	if (delay) {	/* not if (delay++), so at end delay == 0 */
	/* lenses give 50% faster reading */
//	    nomul( (ublindf && ublindf->otyp == LENSES) ? 
//			(-26+ACURR(A_INT))/2 :
//			(-26+ACURR(A_INT))); /* Necronomicon is difficult to put down */
//	    delay -= -26+ACURR(A_INT); /* - a negative number is + */
		delay++;
		if(ublindf && ublindf->otyp == LENSES) delay++;
		if(delay < 0){
			return(1); /* still busy */
		}
		delay = 0;
	}
	if(lostname < QUEST_SPIRITS){/* Bind spirit */
		if(Role_if(PM_EXILE)){
			if(u.ulevel <= 2) numSlots=1;
			else if(u.ulevel <= 9) numSlots=2;
			else if(u.ulevel <= 18) numSlots=3;
			else if(u.ulevel <= 25) numSlots=4;
			else numSlots=5;
		} else {
			numSlots=1;
		}
		// for(i=0;i<QUEST_SPIRITS;i++){
			// pline("#%d:%s",i,sealNames[i]);
		// }
		pline("Using the rituals in the book, you attempt to form a bond with %s",sealNames[lostname-FIRST_SEAL-2]); /*Why doesn't math work?*/
		if(u.sealCounts < numSlots){
			bindspirit(lostname);
		} else You("can't feel the spirit.");
	}
	else if(lostname == QUEST_SPIRITS){
		int chance = 0;
		if(!(artiptr->ovar1)) artiptr->spestudied = 0; /* Sanity enforcement. Something wierd is going on with the artifact creation code.*/

		if(!(artiptr->ovar1 & SEAL_SPECIAL)){
			long putativeSeal;
			i = rn2(31);
			putativeSeal = 1L << i;
			if(artiptr->ovar1 & putativeSeal) losexp("getting lost in a book",TRUE,TRUE,TRUE);
			else{
				artiptr->ovar1 |= putativeSeal;
				You("learn the name \"%s\" while studying the book.",sealNames[i]);
				artiptr->spestudied++;
			}
		} else losexp("getting lost in a book",TRUE,TRUE,TRUE);
		
		if(artiptr->spestudied > 5 && !rn2(500)) artiptr->ovar1 |= SEAL_SPECIAL;
		
	}
	else{
		pline("Unrecognized Lost Names effect.");
	}
	artiptr = 0;
	return(0);
}

/*
 * Artifact is dipped into water
 * -1 not handled here (not used up here)
 *  0 no effect but used up
 *  else return
 *  AD_FIRE, etc.
 *  Note caller should handle what happens to the medium in these cases.
 *      This only prints messages about the actual artifact.
 */

int
artifact_wet(obj, silent)
struct obj *obj;
boolean silent;
{
	 if (!obj->oartifact) return (-1);
	 switch (artilist[(int) (obj)->oartifact].attk.adtyp) {
		 case AD_FIRE:
			 if (!silent) {
				pline("A cloud of steam rises.");
				pline("%s is untouched.", The(xname(obj)));
			 }
			 return (AD_FIRE);
		 case AD_COLD:
			 if (!silent) {
				pline("Icicles form and fall from the freezing %s.",
			             the(xname(obj)));
			 }
			 return (AD_COLD);
		 case AD_ELEC:
			 if (!silent) {
				pline_The("humid air crackles with electricity from %s.",
						the(xname(obj)));
			 }
			 return (AD_ELEC);
		 case AD_DRLI:
			 if (!silent) {
				pline("%s absorbs the water!", The(xname(obj)));
			 }
			 return (AD_DRLI);
		 default:
			 break;
	}
	return (-1);
}

/* WAC return TRUE if artifact is always lit */
boolean
artifact_light(obj)
    struct obj *obj;
{
    return	(get_artifact(obj) && 
				(obj->oartifact == ART_SUNSWORD ||
				 (obj->oartifact == ART_PEN_OF_THE_VOID && obj->ovar1&SEAL_JACK) ||
				 (obj->oartifact >= ART_SWORD_OF_ERATHAOL &&
				  obj->oartifact <= ART_HAMMER_OF_BARQUIEL)
				)
			);
}

/* return TRUE if artifact is permanently lit */
boolean
arti_light(obj)
    struct obj *obj;
{
	const struct artifact *arti = get_artifact(obj);
    return	(arti && 
				(arti->cspfx3 & SPFX3_LIGHT)
			);
}

/* KMH -- Talking artifacts are finally implemented */
void
arti_speak(obj)
    struct obj *obj;
{
	register const struct artifact *oart = get_artifact(obj);
	const char *line;
	char buf[BUFSZ];


	/* Is this a speaking artifact? */
	if (!oart || !(oart->spfx & SPFX_SPEAK))
		return;

	line = getrumor(bcsign(obj), buf, TRUE);
	if (!*line)
		line = "NetHack rumors file closed for renovation.";
	pline("%s:", Tobjnam(obj, "whisper"));
	verbalize("%s", line);
	return;
}

boolean
artifact_has_invprop(otmp, inv_prop)
struct obj *otmp;
uchar inv_prop;
{
	const struct artifact *arti = get_artifact(otmp);

	return((boolean)(arti && (arti->inv_prop == inv_prop)));
}

/* Return the price sold to the hero of a given artifact or unique item */
long
arti_cost(otmp)
struct obj *otmp;
{
	if (!otmp->oartifact)
	    return ((long)objects[otmp->otyp].oc_cost);
	else if (artilist[(int) otmp->oartifact].cost)
	    return (artilist[(int) otmp->oartifact].cost);
	else
	    return (100L * (long)objects[otmp->otyp].oc_cost);
}

static const char *random_seasound[] = {
	"distant waves",
	"distant surf",
	"the distant sea",
	"the call of the ocean",
	"waves against the shore",
	"flowing water",
	"the sighing of waves",
	"quarrelling gulls",
	"the song of the deep",
	"rumbling in the deeps",
	"the singing of Eidothea",
	"the laughter of the protean nymphs",
	"rushing tides",
	"the elusive sea change",
	"the silence of the sea",
	"the passage of the albatross",
	"dancing raindrops",
	"coins rolling on the seabed",
	"treasure galleons crumbling in the depths",
	"waves lapping against a hull"
};

/* Polymorph obj contents */
void
arti_poly_contents(obj)
    struct obj *obj;
{
    struct obj *dobj = 0;  /*object to be deleted*/
    struct obj *otmp;
	You_hear("%s.",random_seasound[rn2(SIZE(random_seasound))]);
	for (otmp = obj->cobj; otmp; otmp = otmp->nobj){
		if (dobj) {
			delobj(dobj);
			dobj = 0;
		}
		if(!obj_resists(otmp, 5, 95)){
			/* KMH, conduct */
			u.uconduct.polypiles++;
			/* any saved lock context will be dangerously obsolete */
			if (Is_box(otmp)) (void) boxlock(otmp, obj);

			if (obj_shudders(otmp)) {
				dobj = otmp;
			}
			else otmp = poly_obj(otmp, STRANGE_OBJECT);
		}
	}
	if (dobj) {
		delobj(dobj);
		dobj = 0;
	}
}
#endif /* OVLB */

STATIC_OVL int
throweffect()
{
	coord cc;

	if (u.uinwater) {
	    pline("You're joking! In this weather?"); return 0;
	} else if (Is_waterlevel(&u.uz)) {
	    You("had better wait for the sun to come out."); return 0;
	}

	pline("Where do you want to cast the spell?");
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "the desired position") < 0)
	    return 0;	/* user pressed ESC */
	/* The number of moves from hero to where the spell drops.*/
	if (distmin(u.ux, u.uy, cc.x, cc.y) > 10) {
	    pline_The("spell dissipates over the distance!");
	    return 0;
	} else if (u.uswallow) {
	    pline_The("spell is cut short!");
	    exercise(A_WIS, FALSE); /* What were you THINKING! */
	    u.dx = 0;
	    u.dy = 0;
	    return 1;
	} else if (!cansee(cc.x, cc.y) || IS_STWALL(levl[cc.x][cc.y].typ)) {
	    Your("mind fails to lock onto that location!");
	    return 0;
	} else {
	    u.dx=cc.x;
	    u.dy=cc.y;
	    return 1;
	}
}

STATIC_OVL void
cast_protection()
{
	int loglev = 0;
	int l = u.ulevel;
	int natac = u.uac - u.uspellprot;
	int gain;

	/* loglev=log2(u.ulevel)+1 (1..5) */
	while (l) {
	    loglev++;
	    l /= 2;
	}

	/* The more u.uspellprot you already have, the less you get,
	 * and the better your natural ac, the less you get.
	 */
	gain = loglev - (int)u.uspellprot / (4 - min(3,(10 - natac)/10));

	if (gain > 0) {
	    if (!Blind) {
		const char *hgolden = hcolor(NH_GOLDEN);

		if (u.uspellprot)
		    pline_The("%s haze around you becomes more dense.",
			      hgolden);
		else
		    pline_The("%s around you begins to shimmer with %s haze.",
			/*[ what about being inside solid rock while polyd? ]*/
			(Underwater || Is_waterlevel(&u.uz)) ? "water" : "air",
			      an(hgolden));
	    }
	    u.uspellprot += gain;
	    u.uspmtime =
			P_SKILL(spell_skilltype(SPE_PROTECTION)) == P_EXPERT ? 50 : 20;
	    if (!u.usptime)
			u.usptime = 100;
	    find_ac();
	} else {
	    Your("skin feels warm for a moment.");
	}
}
STATIC_OVL void
awaken_monsters(distance)
int distance;
{
	register struct monst *mtmp = fmon;
	register int distm;

	while(mtmp) {
	    if (!DEADMONSTER(mtmp)) {
		distm = distu(mtmp->mx, mtmp->my);
		if (distm < distance) {
		    mtmp->msleeping = 0;
		    mtmp->mcanmove = 1;
		    mtmp->mfrozen = 0;
		}
	    }
	    mtmp = mtmp->nmon;
	}
}
/* Generate earthquake :-) of desired force.
 * That is:  create random chasms (pits).
 */

STATIC_OVL void
do_earthquake(force)
int force;
{
	register int x,y;
	struct monst *mtmp;
	struct obj *otmp;
	struct trap *chasm;
	int start_x, start_y, end_x, end_y;

	start_x = u.ux - (force * 2);
	start_y = u.uy - (force * 2);
	end_x = u.ux + (force * 2);
	end_y = u.uy + (force * 2);
	if (start_x < 1) start_x = 1;
	if (start_y < 1) start_y = 1;
	if (end_x >= COLNO) end_x = COLNO - 1;
	if (end_y >= ROWNO) end_y = ROWNO - 1;
	for (x=start_x; x<=end_x; x++) for (y=start_y; y<=end_y; y++) {
	    if ((mtmp = m_at(x,y)) != 0) {
		wakeup(mtmp);	/* peaceful monster will become hostile */
		if (mtmp->mundetected && is_hider(mtmp->data)) {
		    mtmp->mundetected = 0;
		    if (cansee(x,y))
			pline("%s is shaken loose from the ceiling!",
							    Amonnam(mtmp));
		    else
			You_hear("a thumping sound.");
		    if (x==u.ux && y==u.uy)
			You("easily dodge the falling %s.",
							    mon_nam(mtmp));
		    newsym(x,y);
		}
	    }
	    if (!rn2(14 - force)) switch (levl[x][y].typ) {
		  case FOUNTAIN : /* Make the fountain disappear */
			if (cansee(x,y))
				pline_The("fountain falls into a chasm.");
			goto do_pit;
#ifdef SINKS
		  case SINK :
			if (cansee(x,y))
				pline_The("kitchen sink falls into a chasm.");
			goto do_pit;
#endif
		  case ALTAR :
			if (Is_astralevel(&u.uz) || Is_sanctum(&u.uz)) break;

			if (cansee(x,y))
				pline_The("altar falls into a chasm.");
			goto do_pit;
		  case GRAVE :
			if (cansee(x,y))
				pline_The("headstone topples into a chasm.");
			goto do_pit;
		  case TREE:
			if (cansee(x,y))
				pline_The("tree topples into a chasm.");
			if(u.sealsActive&SEAL_EDEN) unbind(SEAL_EDEN,TRUE);
			goto do_pit;
		  case DEADTREE:
			if (cansee(x,y))
				pline_The("dead tree topples into a chasm.");
			goto do_pit;
		  case THRONE :
			if (cansee(x,y))
				pline_The("throne falls into a chasm.");
			if(u.sealsActive&SEAL_DANTALION) unbind(SEAL_DANTALION,TRUE);
			/* Falls into next case */
		  case ROOM :
		  case CORR : /* Try to make a pit */
do_pit:		    chasm = maketrap(x,y,PIT);
		    if (!chasm) break;	/* no pit if portal at that location */
		    chasm->tseen = 1;

		    levl[x][y].doormask = 0;

		    mtmp = m_at(x,y);

		    if ((otmp = boulder_at(x, y)) != 0) {
			if (cansee(x, y))
			   pline("KADOOM! The %s falls into a chasm%s!", xname(boulder_at(x,y)),
			      ((x == u.ux) && (y == u.uy)) ? " below you" : "");
			if (mtmp)
				mtmp->mtrapped = 0;
			obj_extract_self(otmp);
			(void) flooreffects(otmp, x, y, "");
			break;
		    }

		    /* We have to check whether monsters or player
		       falls in a chasm... */

		    if (mtmp) {
			if(!is_flyer(mtmp->data) && !is_clinger(mtmp->data)) {
			    mtmp->mtrapped = 1;
			    if(cansee(x,y))
				pline("%s falls into a chasm!", Monnam(mtmp));
			    else if (flags.soundok && humanoid(mtmp->data))
				You_hear("a scream!");
			    mselftouch(mtmp, "Falling, ", TRUE);
			    if (mtmp->mhp > 0)
				if ((mtmp->mhp -= rnd(6)) <= 0) {
				    if(!cansee(x,y))
					pline("It is destroyed!");
				    else {
					You("destroy %s!", mtmp->mtame ?
					    x_monnam(mtmp, ARTICLE_THE, "poor",
				mtmp->mnamelth ? SUPPRESS_SADDLE : 0, FALSE):
					    mon_nam(mtmp));
				    }
				    xkilled(mtmp,0);
				}
			}
		    } else if (x == u.ux && y == u.uy) {
			    if (Levitation || Flying ||
						is_clinger(youmonst.data)) {
				    pline("A chasm opens up under you!");
				    You("don't fall in!");
			    } else {
				    You("fall into a chasm!");
				    u.utrap = rn1(6,2);
				    u.utraptype = TT_PIT;
				    losehp(rnd(6),"fell into a chasm",
					NO_KILLER_PREFIX);
				    selftouch("Falling, you");
			    }
		    } else newsym(x,y);
		    break;
		  case DOOR : /* Make the door collapse */
		    /* ALI - artifact doors from Slash'em*/
		    if (artifact_door(x, y))  break;
		    if (levl[x][y].doormask == D_NODOOR) goto do_pit;
		    if (cansee(x,y))
			pline_The("door collapses.");
		    if (*in_rooms(x, y, SHOPBASE))
			add_damage(x, y, 0L);
		    levl[x][y].doormask = D_NODOOR;
		    unblock_point(x,y);
		    newsym(x,y);
		    break;
	    }
	}
}

/*artifact.c*/
