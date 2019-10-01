/*	SCCS Id: @(#)monmove.c	3.4	2002/04/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "mfndpos.h"
#include "artifact.h"
#include "epri.h"
#include "edog.h"

extern boolean notonhead;

#ifdef OVL0

STATIC_DCL int FDECL(disturb,(struct monst *));
STATIC_DCL void FDECL(distfleeck,(struct monst *,int *,int *,int *));
STATIC_DCL int FDECL(m_arrival, (struct monst *));
STATIC_DCL void FDECL(watch_on_duty,(struct monst *));

#endif /* OVL0 */
#ifdef OVLB

boolean /* TRUE : mtmp died */
mb_trapped(mtmp)
register struct monst *mtmp;
{
	if (flags.verbose) {
	    if (cansee(mtmp->mx, mtmp->my))
		pline("KABOOM!!  You see a door explode.");
	    else if (flags.soundok)
		You_hear("a distant explosion.");
	}
	wake_nearto_noisy(mtmp->mx, mtmp->my, 7*7);
	mtmp->mstun = 1;
	mtmp->mhp -= rnd(15);
	if(mtmp->mhp <= 0) {
		mondied(mtmp);
		if (mtmp->mhp > 0) /* lifesaved */
			return(FALSE);
		else
			return(TRUE);
	}
	return(FALSE);
}

#endif /* OVLB */
#ifdef OVL0

STATIC_OVL void
watch_on_duty(mtmp)
register struct monst *mtmp;
{
	int	x, y;

	if(mtmp->mpeaceful && in_town(u.ux+u.dx, u.uy+u.dy) &&
	   !is_blind(mtmp) && m_canseeu(mtmp) && !rn2(3)) {

//ifdef CONVICT
		if(Role_if(PM_CONVICT) && !Upolyd && !(ublindf && ublindf->otyp != LENSES)) {
            verbalize("%s yells: Hey!  You are the one from the wanted poster!",
             Amonnam(mtmp));
            (void) angry_guards(!(flags.soundok));
            stop_occupation();
            return;
        }
//endif /* CONVICT */
	    if(picking_lock(&x, &y) && IS_DOOR(levl[x][y].typ) &&
	       (levl[x][y].doormask & D_LOCKED)) {

		if(couldsee(mtmp->mx, mtmp->my)) {

		  pline("%s yells:", Amonnam(mtmp));
		  if(levl[x][y].looted & D_WARNED) {
			verbalize("Halt, thief!  You're under arrest!");
			(void) angry_guards(!(flags.soundok));
		  } else {
			verbalize("Hey, stop picking that lock!");
			levl[x][y].looted |=  D_WARNED;
		  }
		  stop_occupation();
		}
	    } else if (is_digging()) {
		/* chewing, wand/spell of digging are checked elsewhere */
		watch_dig(mtmp, digging.pos.x, digging.pos.y, FALSE);
	    }
	}
}

#endif /* OVL0 */
#ifdef OVL1

int
dochugw(mtmp)
	register struct monst *mtmp;
{
	register int x = mtmp->mx, y = mtmp->my;
	boolean already_saw_mon = !occupation ? 0 : canspotmon(mtmp);
	int rd = dochug(mtmp);
#if 0
	/* part of the original warning code which was replaced in 3.3.1 */
	int dd;

	if(Warning && !rd && !mtmp->mpeaceful &&
			(dd = distu(mtmp->mx,mtmp->my)) < distu(x,y) &&
			dd < 100 && !canseemon(mtmp)) {
	    /* Note: this assumes we only want to warn against the monster to
	     * which the weapon does extra damage, as there is no "monster
	     * which the weapon warns against" field.
	     */
	    if (spec_ability(uwep, SPFX_WARN) && spec_dbon(uwep, mtmp, 1))
		warnlevel = 100;
	    else if ((int) (mtmp->m_lev / 4) > warnlevel)
		warnlevel = (mtmp->m_lev / 4);
	}
#endif /* 0 */

	/* a similar check is in monster_nearby() in hack.c */
	/* check whether hero notices monster and stops current activity */
	if (occupation && !rd && !Confusion &&
	    (!mtmp->mpeaceful || Hallucination) &&
	    /* it's close enough to be a threat */
	    distu(mtmp->mx,mtmp->my) <= (BOLT_LIM+1)*(BOLT_LIM+1) &&
	    /* and either couldn't see it before, or it was too far away */
	    (!already_saw_mon || !couldsee(x,y) ||
		distu(x,y) > (BOLT_LIM+1)*(BOLT_LIM+1)) &&
	    /* can see it now, or sense it and would normally see it */
	    (canseemon(mtmp) ||
		(sensemon(mtmp) && couldsee(mtmp->mx,mtmp->my))) &&
	    mtmp->mcanmove && mtmp->mnotlaugh &&
	    !noattacks(mtmp->data) && !onscary(u.ux, u.uy, mtmp))
		stop_occupation();

	return(rd);
}

#endif /* OVL1 */
#ifdef OVL2

boolean
onscary(x, y, mtmp)
int x, y;
struct monst *mtmp;
{
	struct obj *alignedfearobj = aligned_sartprop3_at(SPFX3_FEAR, x, y);
	int wardAt = ward_at(x,y);
	struct monst *mat = m_at(x,y);
	
	if(no_upos(mtmp))
		return FALSE;
	
	return (boolean)(
				((
					sobj_at(SCR_SCARE_MONSTER, x, y)
				 || (alignedfearobj && !touch_artifact(alignedfearobj, mtmp, FALSE))
				 ) && scaryItem(mtmp)
				)
			 || (u.umonnum == PM_GHOUL && x == mtmp->mux && y == mtmp->muy && mtmp->data == &mons[PM_GUG])
			 || (mat && mat->data == &mons[PM_GHOUL] && mtmp->data == &mons[PM_GUG])
			 || (mat && mat->data == &mons[PM_MOVANIC_DEVA] && (is_animal(mtmp->data) || is_plant(mtmp->data)))
			 || (wardAt == HEPTAGRAM && scaryHept(num_wards_at(x,y), mtmp))
			 || (wardAt == GORGONEION && scaryGorg(num_wards_at(x,y), mtmp))
			 || (wardAt == CIRCLE_OF_ACHERON && scaryCircle(num_wards_at(x,y), mtmp))
			 || (wardAt == PENTAGRAM && scaryPent(num_wards_at(x,y), mtmp))
			 || (wardAt == HEXAGRAM && scaryHex(num_wards_at(x,y), mtmp))
			 || (wardAt == HAMSA && scaryHam(num_wards_at(x,y), mtmp))
			 || (wardAt == ELDER_SIGN && scarySign(num_wards_at(x,y), mtmp))
			 || (wardAt == ELDER_ELEMENTAL_EYE && scaryEye(num_wards_at(x,y), mtmp))
			 || (wardAt == SIGN_OF_THE_SCION_QUEEN && scaryQueen(num_wards_at(x,y), mtmp))
			 || (wardAt == CARTOUCHE_OF_THE_CAT_LORD && scaryCat(num_wards_at(x,y), mtmp))
			 || (wardAt == WINGS_OF_GARUDA && scaryWings(num_wards_at(x,y), mtmp))
			 || (wardAt == YELLOW_SIGN && scaryYellow(num_wards_at(x,y), mtmp))
			 || (scaryElb(mtmp) && sengr_at("Elbereth", x, y))
			 || (scaryLol(mtmp) && sengr_at("Lolth", x, y) && (mtmp->m_lev < u.ulevel || u.ualign.record-- > 0))
			 || (scaryTou(mtmp) && toustefna_at(x,y))
			 || (scaryDre(mtmp) && dreprun_at(x,y))
			 || (scaryVei(mtmp) && veioistafur_at(x,y))
			 || (scaryThj(mtmp) && thjofastafur_at(x,y))
			 || (is_vampire(mtmp->data) && IS_ALTAR(levl[x][y].typ))
			);
}

boolean
scaryWings(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->iswiz || 
			is_rider(mtmp->data)) return FALSE;
	return 	mtmp->data->mlet == S_COCKATRICE ||
			mtmp->data->mlet == S_RODENT ||
			mtmp->data->mlet == S_NAGA ||
			mtmp->data->mlet == S_SNAKE ||
			mtmp->data->mlet == S_LIZARD ||
			mtmp->data == &mons[PM_TOVE] ||
			mtmp->data == &mons[PM_KRAKEN];
}

boolean
scaryCat(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->iswiz || 
			is_rider(mtmp->data) ||
			( 	(mvitals[PM_KITTEN].mvflags & G_GENOD || mvitals[PM_KITTEN].died >= 120) && 
				(mvitals[PM_HOUSECAT].mvflags & G_GENOD || mvitals[PM_HOUSECAT].died >= 120) && 
				(mvitals[PM_LARGE_CAT].mvflags & G_GENOD || mvitals[PM_LARGE_CAT].died >= 120)
			)
		) return FALSE;
	else if(mtmp->data->mlet == S_FELINE
			){ /* && mvitals[PM_KITTEN].died == 0*/
				mtmp->mpeaceful = TRUE;
				mtmp->mhp = mtmp->mhpmax;
			}
	return 	is_bird(mtmp->data) ||
			is_bat(mtmp->data) ||
			mtmp->data->mlet == S_RODENT ||
			mtmp->data->mlet == S_SNAKE ||
			mtmp->data->mlet == S_SPIDER ||
			mtmp->data->mlet == S_EEL ||
			mtmp->data->mlet == S_LIZARD ||
			mtmp->data == &mons[PM_TOVE];
}

boolean
scaryTou(mtmp)
struct monst *mtmp;
{
	if(mtmp->isshk || mtmp->iswiz || 
			is_rider(mtmp->data)) return FALSE;
	return 	mtmp->data->mlet == S_DOG ||
			mtmp->data->mlet == S_FELINE;
}
boolean
scaryDre(mtmp)
struct monst *mtmp;
{
	if(mtmp->isshk || mtmp->iswiz || 
			is_rider(mtmp->data)) return FALSE;
	return 	is_bird(mtmp->data) ||
			is_bat(mtmp->data) ||
			mtmp->data->mlet == S_QUADRUPED ||
			mtmp->data->mlet == S_UNICORN;
}
boolean
scaryVei(mtmp)
struct monst *mtmp;
{
	if(mtmp->isshk || mtmp->iswiz || 
			is_rider(mtmp->data)) return FALSE;
	return 	mtmp->data->mlet == S_EEL;
}
boolean
scaryThj(mtmp)
struct monst *mtmp;
{
	if(mtmp->isshk || mtmp->iswiz || 
			is_rider(mtmp->data)) return FALSE;
	return 	mtmp->data->mlet == S_LEPRECHAUN ||
			mtmp->data->mlet == S_NYMPH ||
			is_thief(mtmp->data);
}

boolean
scaryQueen(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->iswiz || 
			is_rider(mtmp->data)) return FALSE;
	return 	mtmp->data->mlet == S_ANT ||
			mtmp->data->mlet == S_XAN ||
			mtmp->data->mlet == S_SPIDER ||
			mtmp->data->mlet == S_RUSTMONST;
}

boolean
scaryEye(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->iswiz || 
			is_rider(mtmp->data) ||
			(mtmp->data == &mons[PM_CHOKHMAH_SEPHIRAH]) ||
			(mtmp->data == &mons[PM_ELDER_PRIEST]) ||
			(mtmp->data == &mons[PM_GREAT_CTHULHU]) ||
			(mtmp->data == &mons[PM_CHAOS] && rn2(2)) ||
			(mtmp->data == &mons[PM_DEMOGORGON] && rn2(3)) ||
			(mtmp->data == &mons[PM_LAMASHTU] && rn2(3)) ||
			(mtmp->data == &mons[PM_ASMODEUS] && rn2(9))
		) return FALSE;
	return 	mtmp->data == &mons[PM_FREEZING_SPHERE] ||
			mtmp->data == &mons[PM_FLAMING_SPHERE] ||
			mtmp->data == &mons[PM_SHOCKING_SPHERE] ||
			mtmp->data->mlet == S_FUNGUS ||
			mtmp->data == &mons[PM_ZUGGTMOY] ||
			mtmp->data->mlet == S_VORTEX ||
			mtmp->data->mlet == S_ELEMENTAL ||
			mtmp->data->mlet == S_XORN ||
			(mtmp->data->mlet == S_LIGHT && complete >= 4) ||
			(mtmp->data->mlet == S_DRAGON && complete >= 4) ||
			(mtmp->data->mlet == S_NAGA && complete >= 4) ||
			(is_undead_mon(mtmp) && complete >= 4) ||
			(mtmp->data->mlet == S_TRAPPER && complete >= 4  && /*This one means "is a metroid", which are being counted as energions for this*/
				mtmp->data != &mons[PM_TRAPPER] &&
				mtmp->data != &mons[PM_LURKER_ABOVE]) ||
			(is_angel(mtmp->data) && complete == 7) ||
			(is_keter(mtmp->data) && complete == 7) ||
			(is_demon(mtmp->data) && complete == 7) ||
			(is_auton(mtmp->data) && complete == 7) ||
			(mtmp->data->mlet == S_IMP && complete == 7)
			;			
}

boolean
scarySign(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->iswiz) return FALSE;
	return 	mtmp->data->mlet == S_BLOB ||
			mtmp->data->mlet == S_JELLY ||
			mtmp->data->mlet == S_MIMIC ||
			mtmp->data->mlet == S_PIERCER ||
			mtmp->data->mlet == S_WORM ||
			mtmp->data->mlet == S_LIGHT ||
			mtmp->data->mlet == S_PUDDING ||
			mtmp->data->mlet == S_UMBER ||
			mtmp->data->mlet == S_EEL ||
			mtmp->mfaction == PSEUDONATURAL ||
			mtmp->mfaction == TOMB_HERD ||
			mtmp->mfaction == YITH ||
			mtmp->mfaction == CRANIUM_RAT ||
			mtmp->mfaction == FRACTURED ||
			mtmp->data == &mons[PM_GUG] ||
			mtmp->data == &mons[PM_MIGO_WORKER] ||
			mtmp->data == &mons[PM_MIGO_SOLDIER] ||
			mtmp->data == &mons[PM_MIGO_PHILOSOPHER] ||
			mtmp->data == &mons[PM_MIGO_QUEEN] ||
			mtmp->data == &mons[PM_HOUND_OF_TINDALOS] ||
			mtmp->data == &mons[PM_TRAPPER] ||
			mtmp->data == &mons[PM_LURKER_ABOVE] ||
			mtmp->data == &mons[PM_NIGHTGAUNT] ||
			mtmp->data == &mons[PM_BYAKHEE] ||
			(mtmp->data == &mons[PM_HUNTING_HORROR] && complete == 6) ||
			mtmp->data == &mons[PM_MIND_FLAYER] ||
			mtmp->data == &mons[PM_PARASITIC_MIND_FLAYER] ||
			mtmp->data == &mons[PM_PARASITIZED_ANDROID] ||
			mtmp->data == &mons[PM_PARASITIZED_GYNOID] ||
			(mtmp->data == &mons[PM_PARASITIC_MASTER_MIND_FLAYER] && complete == 6) ||
			(mtmp->data == &mons[PM_PARASITIZED_EMBRACED_ALIDER] && complete == 6) ||
			(mtmp->data == &mons[PM_MASTER_MIND_FLAYER] && complete == 6) ||
			mtmp->data == &mons[PM_DEEP_ONE] ||
			mtmp->data == &mons[PM_DEEPER_ONE] ||
			(mtmp->data == &mons[PM_DEEPEST_ONE] && complete == 6) ||
			mtmp->data == &mons[PM_CHANGED] ||
			(mtmp->data == &mons[PM_WARRIOR_CHANGED] && complete == 6) ||
			mtmp->data == &mons[PM_EDDERKOP] ||
			mtmp->data == &mons[PM_NEVERWAS] ||
			mtmp->data == &mons[PM_INTONER] ||
			(mtmp->data == &mons[PM_BLACK_FLOWER] && complete == 6) ||
			mtmp->data == &mons[PM_DARK_YOUNG] ||
			mtmp->data == &mons[PM_WEEPING_ANGEL] ||
			mtmp->data == &mons[PM_ANCIENT_OF_ICE] ||
			(mtmp->data == &mons[PM_ANCIENT_OF_DEATH] && complete == 6) ||
			(mtmp->data == &mons[PM_JUIBLEX] && complete == 6) ||
			(mtmp->data == &mons[PM_MASKED_QUEEN] && complete == 6) ||
			(mtmp->data == &mons[PM_PALE_NIGHT] && complete == 6) ||
			(mtmp->data == &mons[PM_LEVIATHAN] && complete == 6) ||
			(mtmp->data == &mons[PM_BAALPHEGOR] && complete == 6) ||
			(mtmp->data == &mons[PM_VERIER] && complete == 6) ||
			(mtmp->data == &mons[PM_DAGON] && complete == 6) ||
			(mtmp->data == &mons[PM_DEMOGORGON] && complete == 6 && !rn2(3)) ||
			(mtmp->data == &mons[PM_GREAT_CTHULHU] && complete == 6) ||
			(mtmp->data == &mons[PM_LUGRIBOSSK] && complete == 6) ||
			(mtmp->data == &mons[PM_MAANZECORIAN] && complete == 6) ||
			(mtmp->data == &mons[PM_ELDER_PRIEST] && complete == 6) ||
			(mtmp->data == &mons[PM_PRIEST_OF_AN_UNKNOWN_GOD] && complete == 6);
}
boolean
scaryHam(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->iswiz || 
			is_rider(mtmp->data)) return FALSE;
	return 	is_auton(mtmp->data) ||
			mtmp->data == &mons[PM_FLOATING_EYE] || 
			mtmp->data == &mons[PM_BEHOLDER];

}
boolean
scaryHex(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->iswiz || mtmp->mpeaceful || 
			is_rider(mtmp->data) ||
			(mtmp->data == &mons[PM_CHOKHMAH_SEPHIRAH]) ||
			(mtmp->data == &mons[PM_ELDER_PRIEST]) ||
			(mtmp->data == &mons[PM_GREAT_CTHULHU]) ||
			(mtmp->data == &mons[PM_CHAOS] && rn2(2)) ||
			(mtmp->data == &mons[PM_DEMOGORGON] && rn2(3)) ||
			(mtmp->data == &mons[PM_LAMASHTU] && rn2(3)) ||
			(mtmp->data == &mons[PM_ASMODEUS] && complete <= d(1,8))
		) return FALSE;
	return 	(is_minion(mtmp->data) ||
			mtmp->data == &mons[PM_HELL_HOUND] || 
			mtmp->data == &mons[PM_HELL_HOUND_PUP] ||
			mtmp->data == &mons[PM_EYE_OF_DOOM] ||
			mtmp->data == &mons[PM_SON_OF_TYPHON] ||
			is_golem(mtmp->data) ||
			is_angel(mtmp->data) ||
			is_keter(mtmp->data) ||
			mtmp->data->mlet == S_QUANTMECH ||
			mtmp->data->mlet == S_IMP ||
			is_demon(mtmp->data));
}

boolean
scaryPent(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->iswiz || 
			is_rider(mtmp->data) ||
			(mtmp->data == &mons[PM_CHOKHMAH_SEPHIRAH]) ||
			(mtmp->data == &mons[PM_ELDER_PRIEST]) ||
			(mtmp->data == &mons[PM_GREAT_CTHULHU]) ||
			(mtmp->data == &mons[PM_CHAOS] && rn2(2)) ||
			(mtmp->data == &mons[PM_DEMOGORGON] && rn2(3)) ||
			(mtmp->data == &mons[PM_LAMASHTU] && rn2(3)) ||
			(mtmp->data == &mons[PM_ASMODEUS] && !rn2(9))
		) return FALSE;
	return 	(is_demon(mtmp->data) || 
			mtmp->data == &mons[PM_HELL_HOUND] ||
			mtmp->data == &mons[PM_HELL_HOUND_PUP] ||
			mtmp->data == &mons[PM_GARGOYLE] || 
			mtmp->data == &mons[PM_WINGED_GARGOYLE] ||
			mtmp->data == &mons[PM_DJINNI] ||
			mtmp->data == &mons[PM_SANDESTIN] ||
			mtmp->data == &mons[PM_SALAMANDER] ||
			mtmp->data->mlet == S_ELEMENTAL ||
			mtmp->data->mlet == S_IMP);
}

boolean
scaryCircle(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->iswiz || 
			is_lminion(mtmp) || mtmp->data == &mons[PM_ANGEL] ||
			mtmp->data == &mons[PM_MAANZECORIAN] ||
			is_rider(mtmp->data)) return FALSE;
	return mtmp->data == &mons[PM_CERBERUS] || is_undead_mon(mtmp);
}

boolean
scaryGorg(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->iswiz || 
			is_lminion(mtmp) || mtmp->data == &mons[PM_ANGEL] ||
			is_rider(mtmp->data)) return FALSE;
	else if(mtmp->data->mlet == S_SNAKE){
				mtmp->mpeaceful = TRUE;
			}
	return d(1,100) <= 33*complete && 
			(!is_golem(mtmp->data)) &&
			(mtmp->data != &mons[PM_CHOKHMAH_SEPHIRAH]) &&
			(mtmp->data != &mons[PM_ELDER_PRIEST]) &&
			(mtmp->data != &mons[PM_GREAT_CTHULHU]) &&
			(mtmp->data != &mons[PM_LUGRIBOSSK]) &&
			(mtmp->data != &mons[PM_MAANZECORIAN]) &&
			(mtmp->data != &mons[PM_CHAOS] || rn2(2)) &&
			(mtmp->data != &mons[PM_DEMOGORGON] || !rn2(3)) &&
			(mtmp->data != &mons[PM_LAMASHTU] || !rn2(3)) &&
			(mtmp->data != &mons[PM_ASMODEUS] || !rn2(9));
	
}

boolean
scaryHept(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->isgd || mtmp->iswiz || 
			mtmp->data->mlet == S_HUMAN || mtmp->mpeaceful ||
			is_lminion(mtmp) || mtmp->data == &mons[PM_ANGEL] ||
			is_rider(mtmp->data) ||
			(mtmp->data == &mons[PM_CHOKHMAH_SEPHIRAH]) ||
			(mtmp->data == &mons[PM_ELDER_PRIEST] && complete <= d(2,4)+2) ||
			(mtmp->data == &mons[PM_GREAT_CTHULHU] && complete <= d(2,4)+2) ||
			(mtmp->data == &mons[PM_CHAOS] && rn2(2)) ||
			(mtmp->data == &mons[PM_DEMOGORGON] && rn2(3)) ||
			(mtmp->data == &mons[PM_LAMASHTU] && rn2(3)) ||
			(mtmp->data == &mons[PM_ASMODEUS] && complete <= d(1,10))
		) return FALSE;
	return( !(is_human(mtmp->data) || is_elf(mtmp->data) || is_dwarf(mtmp->data) ||
		  is_gnome(mtmp->data) || is_orc(mtmp->data)) || 
		  is_undead_mon(mtmp) || is_were(mtmp->data));
	
}
boolean
scaryYellow(complete, mtmp)
int complete;
struct monst *mtmp;
{
	if(complete <= 0) return FALSE;
	else if(mtmp->isshk || mtmp->isgd || mtmp->iswiz || 
			is_lminion(mtmp) || mtmp->data == &mons[PM_ANGEL] ||
			is_rider(mtmp->data) ||
			(mtmp->data == &mons[PM_ELDER_PRIEST])
		) return FALSE;
	if((is_human(mtmp->data) || is_elf(mtmp->data) || is_dwarf(mtmp->data) ||
		  is_gnome(mtmp->data) || is_orc(mtmp->data)) && 
		  !is_undead_mon(mtmp) && 
		  !is_were(mtmp->data)){
			mtmp->mcrazed = 1;
			return !rn2(10);
	}
	return FALSE;
}
boolean
scaryItem(mtmp)
struct monst *mtmp;
{
	if (mtmp->isshk || mtmp->isgd || mtmp->iswiz || is_blind(mtmp) ||
	    mtmp->mpeaceful || mtmp->data->mlet == S_HUMAN || 
	    is_lminion(mtmp) || mtmp->data == &mons[PM_ANGEL] ||
	    is_rider(mtmp->data) || mtmp->data == &mons[PM_MINOTAUR])
		return(FALSE);
	return (boolean) (mtmp->data != &mons[PM_ELDER_PRIEST]) &&
					(mtmp->data != &mons[PM_GREAT_CTHULHU]) &&
					(mtmp->data != &mons[PM_CHOKHMAH_SEPHIRAH]) &&
					(mtmp->data != &mons[PM_CHAOS] || rn2(2)) &&
					(mtmp->data != &mons[PM_DEMOGORGON] || rn2(3)) &&
					(mtmp->data != &mons[PM_LAMASHTU] || rn2(3)) &&
					(mtmp->data != &mons[PM_ASMODEUS]);
}

boolean
scaryLol(mtmp)
struct monst *mtmp;
{
  if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)) return FALSE;
  if((Race_if(PM_DROW)) && !flags.stag){
	if (mtmp->isshk || mtmp->isgd || mtmp->iswiz || is_blind(mtmp) ||
	    mtmp->mpeaceful || mtmp->data->mlet == S_HUMAN || 
	    (is_rider(mtmp->data))
	)
		return(FALSE);
	return (boolean) (mtmp->data != &mons[PM_ELDER_PRIEST]) &&
					(mtmp->data != &mons[PM_GREAT_CTHULHU]) &&
					(mtmp->data != &mons[PM_CHOKHMAH_SEPHIRAH]) &&
					(mtmp->data != &mons[PM_CHAOS] || rn2(2)) &&
					(mtmp->data != &mons[PM_DEMOGORGON] || !rn2(3)) &&
					(mtmp->data != &mons[PM_LAMASHTU]) &&
					(mtmp->data != &mons[PM_ASMODEUS] || !rn2(9));
  } else return(FALSE);
}

boolean
scaryElb(mtmp)
struct monst *mtmp;
{
  if(Role_if(PM_ANACHRONONAUT) && In_quest(&u.uz)) return FALSE;
  if(Race_if(PM_ELF)){
	if (mtmp->isshk || mtmp->isgd || mtmp->iswiz || is_blind(mtmp) ||
	    mtmp->mpeaceful || mtmp->data->mlet == S_HUMAN || 
	    is_lminion(mtmp) || mtmp->data == &mons[PM_ANGEL] ||
	    (is_rider(mtmp->data) && !(mtmp->data == &mons[PM_NAZGUL])) || 
		mtmp->data == &mons[PM_MINOTAUR])
		return(FALSE);
	return (boolean) (mtmp->data != &mons[PM_ELDER_PRIEST]) &&
					(mtmp->data != &mons[PM_GREAT_CTHULHU]) &&
					(mtmp->data != &mons[PM_CHOKHMAH_SEPHIRAH]) &&
					(mtmp->data != &mons[PM_CHAOS] || rn2(2)) &&
					(mtmp->data != &mons[PM_DEMOGORGON] || !rn2(3)) &&
					(mtmp->data != &mons[PM_LAMASHTU]) &&
					(mtmp->data != &mons[PM_ASMODEUS] || !rn2(9));
  }
  else{
	if (mtmp->isshk || mtmp->isgd || mtmp->iswiz || is_blind(mtmp) ||
	    mtmp->mpeaceful || mtmp->data->mlet == S_HUMAN || 
	    is_lminion(mtmp) || mtmp->data == &mons[PM_ANGEL] ||
	    (is_rider(mtmp->data) && !(mtmp->data == &mons[PM_NAZGUL])) || 
		mtmp->data == &mons[PM_MINOTAUR])
		return(FALSE);
	return (boolean) mtmp->data->mlet == S_ORC || mtmp->data->mlet == S_OGRE 
				|| mtmp->data->mlet == S_TROLL || mtmp->data == &mons[PM_NAZGUL];
  }
	

}
#endif /* OVL2 */
#ifdef OVL0

/* regenerate lost hit points */
void
mon_regen(mon, digest_meal)
struct monst *mon;
boolean digest_meal;
{
	if(mon->mtrapped && t_at(mon->mx, mon->my) && t_at(mon->mx, mon->my)->ttyp == VIVI_TRAP)
		return;
	
	if(mon->mtame && u.ufirst_life)
		mon->mhp++;
		
	if(is_alabaster_mummy(mon->data) 
		&& mon->mvar1 == SYLLABLE_OF_LIFE__HOON
	){
		mon->mhp += 10;
	}
	
	if(!DEADMONSTER(mon) &&  mon->entangled == RAZOR_WIRE){
		int beat;
		mon->mhp -= rnd(6);
		if(hates_silver(mon->data) && entangle_material(mon, SILVER))
			mon->mhp -= rnd(20);
		if(hates_iron(mon->data) && entangle_material(mon, IRON))
			mon->mhp -= rnd(mon->m_lev);
		beat = entangle_beatitude(mon, -1);
		if(hates_unholy_mon(mon) && beat)
			mon->mhp -= beat == 2 ? d(2,9) : rnd(9);
		beat = entangle_beatitude(mon, 1);
		if(hates_holy_mon(mon) && beat)
			mon->mhp -= beat == 2 ? rnd(20) : rnd(4);
		if(mon->mhp <= 0){
			mon->mhp = 0;
			if(canspotmon(mon))
				pline("%s is sliced to ribbons in %s struggles!", Monnam(mon), hisherits(mon));
			mondied(mon);
			return;
		}
	}
	
	if(!DEADMONSTER(mon) && mon->mhp < mon->mhpmax/2 && (mon->data == &mons[PM_CHANGED] || mon->data == &mons[PM_WARRIOR_CHANGED])){
		mon->mhp -= 1;
		flags.cth_attk=TRUE;//state machine stuff.
		create_gas_cloud(mon->mx+rn2(3)-1, mon->my+rn2(3)-1, rnd(3), rnd(3)+1);
		flags.cth_attk=FALSE;
		if(mon->mhp == 0){
			mondied(mon);
			return;
		}
		if (mon->mspec_used) mon->mspec_used--;
		if (digest_meal) {
			if (mon->meating) mon->meating--;
		}
		return;
	}
	if(!DEADMONSTER(mon) && mon->data == &mons[PM_INVIDIAK] && !isdark(mon->mx, mon->my)){
		mon->mhp -= 1;
		if(mon->mhp == 0){
			mondied(mon);
			return;
		}
		if (mon->mspec_used) mon->mspec_used--;
		if (digest_meal) {
			if (mon->meating) mon->meating--;
		}
		return;
	}
	/* Clouds on Lolth's level deal damage */
	if(Is_lolth_level(&u.uz) && levl[mon->mx][mon->my].typ == CLOUD){
		if (!(nonliving_mon(mon) || breathless_mon(mon))){
			if (haseyes(mon->data) && mon->mcansee){
				mon->mblinded = 1;
				mon->mcansee = 0;
			}
			if (!resists_poison(mon)) {
				pline("%s coughs!", Monnam(mon));
				mon->mhp -= (d(3,8) + ((Amphibious && !flaming(youracedata)) ? 0 : rnd(6)));
			} else if (!(amphibious_mon(mon) && !flaming(youracedata))){
				/* NB: Amphibious includes Breathless */
				if (!(amphibious_mon(mon) && !flaming(youracedata))) mon->mhp -= rnd(6);
			}
			if(mon->mhp <= 0){
				monkilled(mon, "gas cloud", AD_DRST);
				if(mon->mhp <= 0) return;	/* not lifesaved */
			}
		} else {
			/* NB: Amphibious includes Breathless */
			mon->mhp -= rnd(6);
			if(mon->mhp <= 0){
				monkilled(mon, "suffocating cloud", AD_DRST);
				if(mon->mhp <= 0) return;	/* not lifesaved */
			}
		}
	}
	if(is_uvuudaum(mon->data)){
		mon->mhp += 25; //Fast healing
	} else {
		if(mon->mhp < mon->mhpmax && mon_resistance(mon,REGENERATION)) mon->mhp++;
		if(!nonliving_mon(mon)){
			if (mon->mhp < mon->mhpmax){
				//recover 1/30th hp per turn:
				mon->mhp += (mon->m_lev)/30;
				//Now deal with any remainder
				if(((moves)*((mon->m_lev)%30))/30 > ((moves-1)*((mon->m_lev)%30))/30) mon->mhp += 1;
			}
		}
	}
	if(uwep && uwep->oartifact == ART_SINGING_SWORD && uwep->osinging == OSING_HEALING && !mindless_mon(mon) && !is_deaf(mon) && mon->mtame) mon->mhp += 1;
	if (mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
	if (mon->mspec_used) mon->mspec_used--;
	if (digest_meal) {
	    if (mon->meating) mon->meating--;
	}
}

/*
 * Possibly awaken the given monster.  Return a 1 if the monster has been
 * jolted awake.
 */
STATIC_OVL int
disturb(mtmp)
	register struct monst *mtmp;
{
	/*
	 * + Ettins are hard to surprise.
	 * + Nymphs, jabberwocks, and leprechauns do not easily wake up.
	 *
	 * Wake up if:
	 *	in direct LOS						AND
	 *	within 10 squares					AND
	 *	not stealthy or (mon is an ettin and 9/10)		AND
	 *	(mon is not a nymph, jabberwock, or leprechaun) or 1/50	AND
	 *	Aggravate or mon is (dog or human) or
	 *	    (1/7 and mon is not mimicing furniture or object)
	 */
	if(couldsee(mtmp->mx,mtmp->my) &&
		distu(mtmp->mx,mtmp->my) <= 100 &&
		(!Stealth || (mtmp->data == &mons[PM_ETTIN] && rn2(10))) &&
		(!(mtmp->data->mlet == S_NYMPH
			|| mtmp->data == &mons[PM_JABBERWOCK]
			|| mtmp->data->mlet == S_LEPRECHAUN) || !rn2(50)) &&
		(Aggravate_monster
			|| (mtmp->data->mlet == S_DOG ||
				mtmp->data->mlet == S_HUMAN)
			|| (sensitive_ears(mtmp->data) && !is_deaf(mtmp))
			|| (!rn2(7) && mtmp->m_ap_type != M_AP_FURNITURE &&
				mtmp->m_ap_type != M_AP_OBJECT) )) {
		mtmp->msleeping = 0;
		return(1);
	}
	return(0);
}

/* monster begins fleeing for the specified time, 0 means untimed flee
 * if first, only adds fleetime if monster isn't already fleeing
 * if fleemsg, prints a message about new flight, otherwise, caller should */
void
monflee(mtmp, fleetime, first, fleemsg)
struct monst *mtmp;
int fleetime;
boolean first;
boolean fleemsg;
{
	int j;
	if (u.ustuck == mtmp) {
	    if (u.uswallow)
		expels(mtmp, mtmp->data, TRUE);
	    else if (!sticks(youracedata)) {
		unstuck(mtmp);	/* monster lets go when fleeing */
		You("get released!");
	    }
	}
	/*Clear track so they can actually move away in narrow spaces*/
	for (j = 0; j < MTSZ; j++){
		mtmp->mtrack[j].x = 0;
		mtmp->mtrack[j].y = 0;
	}

	if(mtmp->data == &mons[PM_VROCK]){
		struct monst *tmpm;
		if(!(mtmp->mspec_used || mtmp->mcan)){
			pline("%s screeches.", Monnam(mtmp));
			mtmp->mspec_used = 10;
			for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
				if(tmpm != mtmp && !DEADMONSTER(tmpm)){
					if(tmpm->mpeaceful != mtmp->mpeaceful && !resist(tmpm, 0, 0, FALSE)){
						tmpm->mconf = 1;
					}
				}
			}
			if(!mtmp->mpeaceful){
				make_stunned(HStun + 10, TRUE);
			}
		}
	}
	
	if (!first || !mtmp->mflee) {
	    /* don't lose untimed scare */
	    if (!fleetime)
		mtmp->mfleetim = 0;
	    else if (!mtmp->mflee || mtmp->mfleetim) {
			fleetime += mtmp->mfleetim;
			/* ensure monster flees long enough to visibly stop fighting */
			if (fleetime == 1) fleetime++;
				mtmp->mfleetim = min(fleetime, 127);
	    }
		if( !mtmp->mflee && mtmp->data == &mons[PM_GIANT_TURTLE]){
		 mtmp->mcanmove=0;
		 // mtmp->mfrozen = mtmp->mfleetim;
		 if(canseemon(mtmp)) 
		   pline("%s hides in %s shell!",Monnam(mtmp),mhis(mtmp));
		} else if (!mtmp->mflee && fleemsg && canseemon(mtmp) && !mtmp->mfrozen) 
			mtmp->data == &mons[PM_BANDERSNATCH] ? pline("%s becomes frumious!", (Monnam(mtmp)))
												 : pline("%s turns to flee!", (Monnam(mtmp)));
	    mtmp->mflee = 1;
	}
}

STATIC_OVL void
distfleeck(mtmp,inrange,nearby,scared)
register struct monst *mtmp;
int *inrange, *nearby, *scared;
{
	int seescaryx, seescaryy;

	*inrange = !no_upos(mtmp) && (dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <=
							(BOLT_LIM * BOLT_LIM));
	*nearby = *inrange && monnear(mtmp, mtmp->mux, mtmp->muy);

	/* Note: if your image is displaced, the monster sees the Elbereth
	 * at your displaced position, thus never attacking your displaced
	 * position, but possibly attacking you by accident.  If you are
	 * invisible, it sees the Elbereth at your real position, thus never
	 * running into you by accident but possibly attacking the spot
	 * where it guesses you are.
	 */
	if (is_blind(mtmp) || (Invis && !mon_resistance(mtmp,SEE_INVIS))) {
		seescaryx = mtmp->mux;
		seescaryy = mtmp->muy;
	} else {
		seescaryx = u.ux;
		seescaryy = u.uy;
	}
	*scared = (*nearby && (onscary(seescaryx, seescaryy, mtmp) ||
			       (!mtmp->mpeaceful &&
				    in_your_sanctuary(mtmp, 0, 0))));
	
	if(*scared && mtmp->mvanishes > 0) mtmp->mvanishes = mtmp->mvanishes/2 + 1;
	
	if(mtmp->data == &mons[PM_DAUGHTER_OF_BEDLAM] && !rn2(20)) *scared = TRUE;
	else if(*nearby && !mtmp->mflee && fleetflee(mtmp->data) && (mtmp->data->mmove > youracedata->mmove || noattacks(mtmp->data))) *scared = TRUE;
	
	if(*scared) {
		if (rn2(7))
		    monflee(mtmp, rnd(10), TRUE, TRUE);
		else
		    monflee(mtmp, rnd(100), TRUE, TRUE);
	}

}

/* perform a special one-time action for a monster; returns -1 if nothing
   special happened, 0 if monster uses up its turn, 1 if monster is killed */
STATIC_OVL int
m_arrival(mon)
struct monst *mon;
{
	mon->mstrategy &= ~STRAT_ARRIVE;	/* always reset */

	return -1;
}

/* returns 1 if monster died moving, 0 otherwise */
/* The whole dochugw/m_move/distfleeck/mfndpos section is serious spaghetti
 * code. --KAA
 */
int
dochug(mtmp)
register struct monst *mtmp;
{
	register struct permonst *mdat;
	struct monst *gazemon, *nxtmon;
	char buf[BUFSZ];
	register int tmp=0;
	int inrange, nearby, scared;
#ifdef GOLDOBJ
        struct obj *ygold = 0, *lepgold = 0;
#endif

	
/*	Pre-movement adjustments	*/

	mdat = mtmp->data;
	
	mtmp->mattackedu = 0; /*Clear out attacked bit*/
	
	if(mdat == &mons[PM_GNOLL_MATRIARCH]){
		if(!rn2(20)){
			makemon(&mons[PM_GNOLL], mtmp->mx, mtmp->my, NO_MINVENT|MM_ADJACENTOK|MM_ADJACENTSTRICT);
		}
	}
	if(mdat == &mons[PM_FORD_GUARDIAN]){
		if(!rn2(20) && mtmp->mux == u.ux && mtmp->muy == u.uy && !(mtmp->mstrategy&STRAT_WAITFORU)){
			int i = rnd(4);
			pline("The waters of the ford rise to the aid of the guardian!");
			for(; i > 0; i--)
				makemon(&mons[PM_FORD_ELEMENTAL], mtmp->mx, mtmp->my, NO_MINVENT|MM_ADJACENTOK|MM_ADJACENTSTRICT);
		}
	}
	if(mdat == &mons[PM_LEGION]){
		rn2(7) ? makemon(mkclass(S_ZOMBIE, G_NOHELL|G_HELL), mtmp->mx, mtmp->my, NO_MINVENT|MM_ADJACENTOK|MM_ADJACENTSTRICT): 
				 makemon(&mons[PM_LEGIONNAIRE], mtmp->mx, mtmp->my, NO_MINVENT|MM_ADJACENTOK|MM_ADJACENTSTRICT);
	}
	if (mtmp->mstrategy & STRAT_ARRIVE) {
	    int res = m_arrival(mtmp);
	    if (res >= 0) return res;
	}

	/* check for waitmask status change */
	if ((mtmp->mstrategy & STRAT_WAITFORU) &&
		(m_canseeu(mtmp) || mtmp->mhp < mtmp->mhpmax))
	    mtmp->mstrategy &= ~STRAT_WAITFORU;

	/* update quest status flags */
	quest_stat_check(mtmp);
	
	if(mdat == &mons[PM_CENTER_OF_ALL] 
		&& !mtmp->mtame 
		&& !Is_astralevel(&u.uz)
		&& (near_capacity()>SLT_ENCUMBER || u.ulevel < 14 || mtmp->mpeaceful) 
		&& (near_capacity()>MOD_ENCUMBER || !rn2(4))
	){
		int nlev;
		d_level flev;

		if (!(mon_has_amulet(mtmp) || In_endgame(&u.uz))) {
			nlev = random_teleport_level();
			// pline("going to %d",nlev);
			if (nlev != depth(&u.uz)) {
				struct engr *oep = engr_at(mtmp->mx,mtmp->my);
				if(!oep){
					make_engr_at(mtmp->mx, mtmp->my,
					 "", 0L, DUST);
					oep = engr_at(mtmp->mx,mtmp->my);
				}
				if(oep){
					oep->ward_id = FOOTPRINT;
					oep->halu_ward = 1;
					oep->ward_type = BURN;
					oep->complete_wards = 1;
				}
				get_level(&flev, nlev);
				migrate_to_level(mtmp, ledger_no(&flev), MIGR_RANDOM,
					(coord *)0);
				return 0;
			}
		}
	}

   if (mdat != &mons[PM_GIANT_TURTLE] || !mtmp->mflee)
	if (!(mtmp->mcanmove && mtmp->mnotlaugh) || (mtmp->mstrategy & STRAT_WAITMASK)) {
	    if (Hallucination) newsym(mtmp->mx,mtmp->my);
	    if (mtmp->mcanmove && mtmp->mnotlaugh && (mtmp->mstrategy & STRAT_CLOSE) &&
	       !mtmp->msleeping && monnear(mtmp, u.ux, u.uy))
		quest_talk(mtmp);	/* give the leaders a chance to speak */
	    return(0);	/* other frozen monsters can't do anything */
	}

	/* there is a chance we will wake it */
	if (mtmp->msleeping && !disturb(mtmp)) {
		if (Hallucination) newsym(mtmp->mx,mtmp->my);
		return(0);
	}

	/* not frozen or sleeping: wipe out texts written in the dust */
	wipe_engr_at(mtmp->mx, mtmp->my, 1);

	/* confused monsters get unconfused with small probability */
	if (mtmp->mconf && !rn2(50)) mtmp->mconf = 0;

	/* berserk monsters calm down with small probability */
	if (mtmp->mberserk && !rn2(50)) mtmp->mberserk = 0;

	if (mtmp->mcrazed){
		if(!rn2(4))mtmp->mconf = 1;
		(void) set_apparxy(mtmp);
		if(!rn2(4))mtmp->mberserk = 1;
		(void) set_apparxy(mtmp);
		if(!rn2(10)){
			mtmp->mnotlaugh=0;
			mtmp->mlaughing=rnd(5);
		}
	}
	
	if(mtmp->data == &mons[PM_MAD_SEER]){
		if(!rn2(80)){
			mtmp->mnotlaugh=0;
			mtmp->mlaughing=rnd(5);
		}
	}
	
	if(mtmp->data == &mons[PM_QUIVERING_BLOB] && !mtmp->mflee){
		if(!rn2(20)){
			monflee(mtmp, 0, TRUE, TRUE);
		}
	}
	if(mtmp->data == &mons[PM_GRUE]){
		if(!mtmp->mflee && !isdark(mtmp->mx, mtmp->my)){
			monflee(mtmp, 0, TRUE, TRUE);
		} else if(mtmp->mflee && isdark(mtmp->mx, mtmp->my)){
			mtmp->mflee = 0;
			mtmp->mfleetim = 0;
		}
	}
	
	/* stunned monsters get un-stunned with larger probability */
	if (mtmp->mstun && !rn2(10)) mtmp->mstun = 0;

	/* some monsters teleport */
	if (mon_resistance(mtmp,TELEPORT) && (mtmp->mflee || !rn2(5)) && !rn2(40) && !mtmp->iswiz &&
	    !(noactions(mtmp)) &&
	    !level.flags.noteleport) {
		(void) rloc(mtmp, FALSE);
		return(0);
	}
	
	if(mtmp->data == &mons[PM_DRACAE_ELADRIN]){
		if(!mtmp->mvar1){
			struct permonst *ptr = mkclass(S_CHA_ANGEL, G_PLANES);
			if(ptr)
				mtmp->mvar1 = monsndx(ptr);
		}
		else if(mtmp->mvar2 < 6){
			mtmp->mvar2 += rnd(3);
		} else {
			int ox = mtmp->mx, oy = mtmp->my;
			rloc(mtmp, FALSE);
			if(mtmp->mx != ox || mtmp->my != oy){
				int type = mtmp->mvar1;
				mtmp->mvar1 = 0;
				mtmp->mvar2 = 0;
				mtmp = makemon(&mons[type], ox, oy, NO_MINVENT);
				if(mtmp){
					struct obj *otmp;
					otmp = mksobj(LONG_SWORD, FALSE, FALSE);
					otmp->obj_material = DRAGON_HIDE;
					otmp->objsize = mtmp->data->msize;
					otmp->oproperties = OPROP_ACIDW;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(PLATE_MAIL, TRUE, FALSE);
					otmp->obj_material = SHELL;
					otmp->objsize = mtmp->data->msize;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(GAUNTLETS, TRUE, FALSE);
					otmp->obj_material = SHELL;
					otmp->objsize = mtmp->data->msize;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(ARMORED_BOOTS, TRUE, FALSE);
					otmp->obj_material = SHELL;
					otmp->objsize = mtmp->data->msize;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					m_dowear(mtmp, TRUE);
				}
			}
			return 0;
		}
	}
	if(mtmp->data == &mons[PM_MOTHERING_MASS]){
		if(!mtmp->mvar1){
			struct permonst *ptr = mkclass(S_CHA_ANGEL, G_PLANES);
			if(ptr)
				mtmp->mvar1 = monsndx(ptr);
		}
		else if(mtmp->mvar2 < 6){
			mtmp->mvar2 += rnd(3);
		} else {
			int i;
			int ox = mtmp->mx, oy = mtmp->my;
			int type = mtmp->mvar1;
			int etyp = counter_were(type);
			mtmp->mvar1 = 0;
			mtmp->mvar2 = 0;
			for(i = rnd(4); i; i--){
				if(etyp)
					mtmp = makemon(&mons[etyp], ox, oy, NO_MINVENT|MM_ADJACENTOK|MM_ADJACENTSTRICT);
				else return 0;
				if(mtmp){
					struct obj *otmp;
					otmp = mksobj(LONG_SWORD, FALSE, FALSE);
					otmp->obj_material = DRAGON_HIDE;
					otmp->objsize = mons[type].msize;
					otmp->oproperties = OPROP_ACIDW;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(PLATE_MAIL, TRUE, FALSE);
					otmp->obj_material = SHELL;
					otmp->objsize = mons[type].msize;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(GAUNTLETS, TRUE, FALSE);
					otmp->obj_material = SHELL;
					otmp->objsize = mons[type].msize;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
					otmp = mksobj(ARMORED_BOOTS, TRUE, FALSE);
					otmp->obj_material = SHELL;
					otmp->objsize = mons[type].msize;
					fix_object(otmp);
					(void) mpickobj(mtmp, otmp);
				}
			}
			return 0;
		}
	}
	
	if(mdat == &mons[PM_OONA] && !mtmp->mspec_used){
		nearby = FALSE;
		struct monst *tmpm;
		for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
			if(tmpm != mtmp && !DEADMONSTER(tmpm)){
				if ( mtmp->mpeaceful != tmpm->mpeaceful && distmin(mtmp->mx,mtmp->my,tmpm->mx,tmpm->my) < 4 && resists_oona(tmpm)) {
					nearby=TRUE;
				}
			}
		}
		if(!mtmp->mpeaceful && distmin(mtmp->mx,mtmp->my,u.ux,u.uy) < 4 && Oona_resistance){
			nearby=TRUE;
		}
	}

	if (is_commander(mdat) && mfind_target(mtmp, FALSE))
		m_command(mtmp);
	
	if ((mdat->msound == MS_SHRIEK && !um_dist(mtmp->mx, mtmp->my, 1)) || 
		(mdat->msound == MS_SHOG && !rn2(8)) ||
		(mdat->msound == MS_JUBJUB && !rn2(10) && (!um_dist(mtmp->mx, mtmp->my, 3) || !rn2(10))) ||
		(mdat->msound == MS_TRUMPET && !rn2(10) && !um_dist(mtmp->mx, mtmp->my, 3)) ||
		(mdat->msound == MS_DREAD && !rn2(4)) ||
		(mdat->msound == MS_OONA && (nearby || !rn2(6))) ||
		(mdat->msound == MS_SONG && !rn2(6)) ||
		(mdat == &mons[PM_RHYMER] && !mtmp->mspec_used && !rn2(6)) ||
		(mdat->msound == MS_INTONE && !rn2(6)) ||
		(mdat->msound == MS_FLOWER && !rn2(6)) ||
		(mdat == &mons[PM_LAMASHTU] && !rn2(7))
	) m_respond(mtmp);
	
	if(!mtmp->mblinded) for (gazemon = fmon; gazemon; gazemon = nxtmon){
		nxtmon = gazemon->nmon;
		if(DEADMONSTER(gazemon))
			continue;
		if (gazemon != mtmp
			&& mon_can_see_mon(mtmp, gazemon)
			&& clear_path(mtmp->mx, mtmp->my, gazemon->mx, gazemon->my)
		){
			int i;
			if(gazemon->data == &mons[PM_MEDUSA] && resists_ston(mtmp)
			) continue;
			
			if (hideablewidegaze(gazemon->data) &&
				(rn2(3) < magic_negation(gazemon))
			) continue;
			
			if (controlledwidegaze(gazemon->data)
				&& !mm_aggression(gazemon, mtmp)
			) continue;
			
			for(i = 0; i < NATTK; i++)
				 if(gazemon->data->mattk[i].aatyp == AT_WDGZ) {
					if((gazemon->data->mattk[i].adtyp ==  AD_CONF 
						|| gazemon->data->mattk[i].adtyp ==  AD_WISD 
					) && (dmgtype_fromattack(mtmp->data, AD_CONF, AT_WDGZ)
						|| dmgtype_fromattack(mtmp->data, AD_WISD, AT_WDGZ)
					)) continue;
					/*
					if(canseemon(mtmp) && canseemon(gazemon)){
						Sprintf(buf,"%s can see", Monnam(mtmp));
						pline("%s %s...", buf, mon_nam(gazemon));
					}*/
					(void) gazemm(gazemon, mtmp, &gazemon->data->mattk[i]);
					break;
				 }
		}
	}
	
	if (mtmp->mhp <= 0) return(1); /* m_respond gaze can kill medusa */

	/* fleeing monsters might regain courage */
	if (mtmp->mflee && !mtmp->mfleetim
	   && mtmp->mhp == mtmp->mhpmax && !rn2(25)){
		mtmp->mflee = 0;
        if(canseemon(mtmp) && (mdat == &mons[PM_GIANT_TURTLE]) && !(mtmp->mfrozen)){
         pline("%s comes out of %s shell.", Monnam(mtmp), mhis(mtmp));
         mtmp->mcanmove=1;
		}
    }
   else if (mdat == &mons[PM_GIANT_TURTLE] && !mtmp->mcanmove)
     return (0);

	set_apparxy(mtmp);
	/* Must be done after you move and before the monster does.  The
	 * set_apparxy() call in m_move() doesn't suffice since the variables
	 * inrange, etc. all depend on stuff set by set_apparxy().
	 */

	/* Monsters that want to acquire things */
	/* may teleport, so do it before inrange is set */
	if(is_covetous(mdat) && (mdat!=&mons[PM_DEMOGORGON] || !rn2(3)) 
		&& mdat!=&mons[PM_ELDER_PRIEST] /*&& mdat!=&mons[PM_SHAMI_AMOURAE]*/
		&& mdat!=&mons[PM_LEGION] /*&& mdat!=&mons[PM_SHAMI_AMOURAE]*/
		&& !(noactions(mtmp))
		&& !(mtmp->mpeaceful && !mtmp->mtame) /*Don't telespam the player if peaceful*/
	) (void) tactics(mtmp);
	
	// if(mdat == &mons[PM_GREAT_CTHULHU] && !rn2(20)){
		// (void) tactics(mtmp);
		// return 0;
	// }
	
	/* check distance and scariness of attacks */
	distfleeck(mtmp,&inrange,&nearby,&scared);

	if(find_defensive(mtmp)) {
		if (use_defensive(mtmp) != 0)
			return 1;
	} else if(find_misc(mtmp)) {
		if (use_misc(mtmp) != 0)
			return 1;
	}
	
	if((is_drow(mtmp->data) || mtmp->data == &mons[PM_LUGRIBOSSK] || mtmp->data == &mons[PM_MAANZECORIAN])
		&& (!mtmp->mpeaceful || Darksight)
		&& (levl[mtmp->mx][mtmp->my].lit == 1 || viz_array[mtmp->my][mtmp->mx]&TEMP_LIT1)
		&& !mtmp->mcan && mtmp->mspec_used < 4
		&& !(noactions(mtmp))
		&& !(mindless_mon(mtmp))
	){
		if(cansee(mtmp->mx,mtmp->my)) pline("%s invokes the darkness.",Monnam(mtmp));
	    do_clear_area(mtmp->mx,mtmp->my, 5, set_lit, (genericptr_t)0);
		doredraw();
	    if(mtmp->data == &mons[PM_HEDROW_WARRIOR]) mtmp->mspec_used += d(4,4);
		else mtmp->mspec_used += max(10 - mtmp->m_lev,2);
	}

	if((mtmp->data == &mons[PM_ANDROID] || mtmp->data == &mons[PM_GYNOID])
		&& MON_WEP(mtmp)
		&& (is_vibroweapon(MON_WEP(mtmp)) || is_blaster(MON_WEP(mtmp)))
		&& MON_WEP(mtmp)->ovar1 <= 0
		&& (!(MON_WEP(mtmp)->otyp == HAND_BLASTER || MON_WEP(mtmp)->otyp == ARM_BLASTER) || MON_WEP(mtmp)->recharged < 4)
		&& !mtmp->mcan && !mtmp->mspec_used
		&& !(noactions(mtmp))
		&& !(mindless_mon(mtmp))
		&& !rn2(20)
	){
		if(canspotmon(mtmp)) pline("%s uses %s on-board recharger.",Monnam(mtmp), hisherits(mtmp));
		if(MON_WEP(mtmp)->otyp == MASS_SHADOW_PISTOL){
			MON_WEP(mtmp)->ovar1 = 800L + rn2(200);
		} else if(MON_WEP(mtmp)->otyp == RAYGUN){
			if(Role_if(PM_ANACHRONONAUT) || Role_if(PM_TOURIST))
				MON_WEP(mtmp)->ovar1 = (8 + rn2(8))*10L;
			else MON_WEP(mtmp)->ovar1 = 2+rnd(5)*2;
		} else {
			MON_WEP(mtmp)->ovar1 =80L + rn2(20);
		}
		if(MON_WEP(mtmp)->recharged < 7) MON_WEP(mtmp)->recharged++;
		mtmp->mspec_used = 10;
		return 0;
	}

	if((mtmp->data == &mons[PM_ANDROID] || mtmp->data == &mons[PM_GYNOID])
		&& MON_WEP(mtmp)
		&& (is_lightsaber(MON_WEP(mtmp)) && MON_WEP(mtmp)->oartifact != ART_INFINITY_S_MIRRORED_ARC && MON_WEP(mtmp)->otyp != KAMEREL_VAJRA)
		&& MON_WEP(mtmp)->age <= 0
		&& (!(MON_WEP(mtmp)->otyp == HAND_BLASTER || MON_WEP(mtmp)->otyp == ARM_BLASTER) || MON_WEP(mtmp)->recharged < 4)
		&& !mtmp->mcan && !mtmp->mspec_used
		&& !(noactions(mtmp))
		&& !(mindless_mon(mtmp))
		&& !rn2(20)
	){
		if(canspotmon(mtmp)) pline("%s uses %s on-board recharger.",Monnam(mtmp), hisherits(mtmp));
		MON_WEP(mtmp)->age = 75000;
		if(MON_WEP(mtmp)->recharged < 7) MON_WEP(mtmp)->recharged++;
		mtmp->mspec_used = 10;
		return 0;
	}

	if(mtmp->data == &mons[PM_PHALANX]
		&& !mtmp->mcan
		&& !(noactions(mtmp))
	){
		struct obj *otmp;
		int sprcnt = 0;
		for(otmp = mtmp->minvent; otmp; otmp = otmp->nobj)
			if(otmp->otyp == SPEAR)
				sprcnt++;
		if(sprcnt < 3){
			if(canspotmon(mtmp)) pline("%s pulls a spear out of its rotting mass.",Monnam(mtmp));
			(void) mongets(mtmp, SPEAR);
			init_mon_wield_item(mtmp);
			return 0;
		}
	}

	/* Demonic Blackmail! */
	if(nearby && mdat->msound == MS_BRIBE &&
#ifdef CONVICT
       (monsndx(mdat) != PM_PRISON_GUARD) &&
#endif /* CONVICT */
       !no_upos(mtmp) &&
	   mtmp->mpeaceful && !mtmp->mtame && !u.uswallow) {
		if (mtmp->mux != u.ux || mtmp->muy != u.uy) {
			pline("%s whispers at thin air.",
			    cansee(mtmp->mux, mtmp->muy) ? Monnam(mtmp) : "It");

			if (is_demon(youracedata)) {
			  /* "Good hunting, brother" */
			    if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
			} else {
			    mtmp->minvis = mtmp->perminvis = 0;
			    /* Why?  For the same reason in real demon talk */
			    pline("%s gets angry!", Amonnam(mtmp));
			    mtmp->mpeaceful = 0;
			    /* since no way is an image going to pay it off */
			}
		} else if(demon_talk(mtmp)) return(1);	/* you paid it off */
	}

#ifdef CONVICT
	/* Prison guard extortion */
    if(nearby && (monsndx(mdat) == PM_PRISON_GUARD) && !mtmp->mpeaceful
	 && !mtmp->mtame && !u.uswallow && (!mtmp->mspec_used)) {
        long gdemand = 500 * u.ulevel;
        long goffer = 0;

        pline("%s demands %ld %s to avoid re-arrest.", Amonnam(mtmp),
         gdemand, currency(gdemand));
        if ((goffer = bribe(mtmp)) >= gdemand) {
            verbalize("Good.  Now beat it, scum!");
            mtmp->mpeaceful = 1;
            set_malign(mtmp);
        } else {
            pline("I said %ld!", gdemand);
            mtmp->mspec_used = 1000;
        }
    }
#endif /* CONVICT */

	/* the watch will look around and see if you are up to no good :-) */
	if (mdat == &mons[PM_WATCHMAN] || mdat == &mons[PM_WATCH_CAPTAIN])
		watch_on_duty(mtmp);
	if(mdat == &mons[PM_NURSE]){
		int i, j;
		struct trap *ttmp;
		struct monst *tmon;
		for(i=-1; i<2; i++)
			for(j=-1; j<2; j++)
				if(isok(mtmp->mx+i,mtmp->my+j)){
					ttmp = t_at(mtmp->mx+i,mtmp->my+j);
					tmon = m_at(mtmp->mx+i,mtmp->my+j);
					if(ttmp && ttmp->ttyp == VIVI_TRAP && tmon && tmon->mtrapped){
						if(canspotmon(mtmp))
							pline("%s frees a vivisected prisoner from an essence trap!", Monnam(mtmp));
						tmon->mpeaceful = mtmp->mpeaceful;
						tmon->mtrapped = 0;
						if(mtmp->mtame){
							reward_untrap(ttmp, tmon);
							u.uevent.uaxus_foe = 1;
							pline("An alarm sounds!");
							aggravate();
						}
						deltrap(ttmp);
						newsym(mtmp->mx+i,mtmp->my+j);
						return 1;
					}
				}
	}
	if(mdat == &mons[PM_TOVE] && !rn2(20)){
		struct trap *ttmp = t_at(mtmp->mx, mtmp->my);
		struct rm *lev = &levl[mtmp->mx][mtmp->my];
		schar typ;
		boolean nohole = !Can_dig_down(&u.uz);
		if (!(
			!isok(mtmp->mx,mtmp->my) || 
			(ttmp && ttmp->ttyp == MAGIC_PORTAL) ||
		   /* ALI - artifact doors from slash'em */
			(IS_DOOR(levl[mtmp->mx][mtmp->my].typ) && artifact_door(mtmp->mx, mtmp->my)) ||
			(IS_ROCK(lev->typ) && lev->typ != SDOOR &&
			(lev->wall_info & W_NONDIGGABLE) != 0) ||
			(is_pool(mtmp->mx, mtmp->my, TRUE) || is_lava(mtmp->mx, mtmp->my)) ||
			(lev->typ == DRAWBRIDGE_DOWN ||
			   (is_drawbridge_wall(mtmp->mx, mtmp->my) >= 0)) ||
			(boulder_at(mtmp->mx, mtmp->my)) ||
			(IS_GRAVE(lev->typ)) ||
			(lev->typ == DRAWBRIDGE_UP) ||
			(IS_THRONE(lev->typ)) ||
			(IS_ALTAR(lev->typ)) ||
			(Is_airlevel(&u.uz))
		)){
			typ = fillholetyp(mtmp->mx,mtmp->my);
			if (canseemon(mtmp))
				pline("%s gyres and gimbles into the %s.", Monnam(mtmp),surface(mtmp->mx,mtmp->my));
			if (typ != ROOM) {
				lev->typ = typ;
				if (ttmp) (void) delfloortrap(ttmp);
				/* if any objects were frozen here, they're released now */
				unearth_objs(mtmp->mx, mtmp->my);

					if(cansee(mtmp->mx, mtmp->my))
						pline_The("hole fills with %s!",
						  typ == LAVAPOOL ? "lava" : "water");
				if (!Levitation && !Flying && mtmp->mx==u.ux && mtmp->my==u.uy) {
					if (typ == LAVAPOOL)
					(void) lava_effects();
					else if (!Wwalking)
					(void) drown();
				}
			}
			if (nohole || !ttmp || (ttmp->ttyp != PIT && ttmp->ttyp != SPIKED_PIT && ttmp->ttyp != TRAPDOOR))
				digactualhole(mtmp->mx, mtmp->my, mtmp, PIT, FALSE, TRUE);
			else
				digactualhole(mtmp->mx, mtmp->my, mtmp, HOLE, FALSE, TRUE);
		}
	}
	if (has_mind_blast(mdat) && !u.uinvulnerable && !rn2(mdat == &mons[PM_ELDER_BRAIN] ? 10 : 20)) {
		boolean reducedFlayerMessages = (((Role_if(PM_NOBLEMAN) && Race_if(PM_DROW) && flags.initgend) || Role_if(PM_ANACHRONONAUT)) && In_quest(&u.uz));
		struct monst *m2, *nmon = (struct monst *)0;
		
		if (canseemon(mtmp))
			pline("%s concentrates.", Monnam(mtmp));
		// if (distu(mtmp->mx, mtmp->my) > BOLT_LIM * BOLT_LIM) {
			// You("sense a faint wave of psychic energy.");
			// goto toofar;
		// }
		if(!reducedFlayerMessages) pline("A wave of psychic energy pours over you!");
		if (mtmp->mpeaceful &&
		    (!Conflict || resist(mtmp, RING_CLASS, 0, 0))){
			if(!reducedFlayerMessages) pline("It feels quite soothing.");
		} else {
			register boolean m_sen = tp_sensemon(mtmp);
			
			if(mdat == &mons[PM_ELDER_BRAIN]) quest_chat(mtmp);
			
			if (m_sen || (Blind_telepat && rn2(2)) || !rn2(10)) {
				int dmg;
				pline("It locks on to your %s!",
					m_sen ? "telepathy" :
					Blind_telepat ? "latent telepathy" : "mind");
				dmg = (mdat == &mons[PM_GREAT_CTHULHU] || mdat == &mons[PM_LUGRIBOSSK] || mdat == &mons[PM_MAANZECORIAN]) ? d(5,15) : (mdat == &mons[PM_ELDER_BRAIN]) ? d(3,15) : rnd(15);
				if (Half_spell_damage) dmg = (dmg+1) / 2;
				if(u.uvaul_duration) dmg = (dmg + 1) / 2;
				losehp(dmg, "psychic blast", KILLED_BY_AN);
				if(mdat == &mons[PM_SEMBLANCE]) make_hallucinated(HHallucination + dmg, FALSE, 0L);
				if(mdat == &mons[PM_GREAT_CTHULHU]) make_stunned(HStun + dmg*10, TRUE);
				if (mdat == &mons[PM_ELDER_BRAIN]) {
					for (m2 = fmon; m2; m2 = nmon) {
						nmon = m2->nmon;
						if (!DEADMONSTER(m2) && (m2->mpeaceful == mtmp->mpeaceful) && mon_resistance(m2,TELEPAT) && (m2!=mtmp))
						{
							m2->msleeping = 0;
							if (!m2->mcanmove && !rn2(5)) {
								m2->mfrozen = 0;
								if (m2->data != &mons[PM_GIANT_TURTLE] || !(m2->mflee))
									m2->mcanmove = 1;
							}
							m2->mux = u.ux;
							m2->muy = u.uy;
							m2->encouraged = max(m2->encouraged, dmg / 3);
						}
					}
				}
			}
		}
		for(m2=fmon; m2; m2 = nmon) {
			nmon = m2->nmon;
			if (DEADMONSTER(m2)) continue;
			if (m2->mpeaceful == mtmp->mpeaceful) continue;
			if (mindless_mon(m2)) continue;
			if (m2 == mtmp) continue;
			if ((mon_resistance(m2,TELEPAT) &&
			    (rn2(2) || m2->mblinded)) || !rn2(10)) {
				if (cansee(m2->mx, m2->my))
				    pline("It locks on to %s.", mon_nam(m2));
				if(mdat == &mons[PM_GREAT_CTHULHU]) m2->mconf=TRUE;
				if(mdat == &mons[PM_GREAT_CTHULHU] || mdat == &mons[PM_LUGRIBOSSK] || mdat == &mons[PM_MAANZECORIAN]) m2->mhp -= d(5,15);
				else if(mdat == &mons[PM_ELDER_BRAIN]){
					int mfdmg = d(3,15);
					m2->mhp -= mfdmg;
					m2->mstdy = max(mfdmg, m2->mstdy);
				} else {
					m2->mhp -= rnd(15);
					if(mdat == &mons[PM_SEMBLANCE]) m2->mconf=TRUE;
				}
				if (m2->mhp <= 0)
				    monkilled(m2, "", AD_DRIN);
				else
				    m2->msleeping = 0;
			}
		}
	}
toofar:

	/* If monster is nearby you, and has to wield a weapon, do so.   This
	 * costs the monster a move, of course.
	 */
	if((!mtmp->mpeaceful || mdat == &mons[PM_NURSE] || Conflict) && inrange &&
	   dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 8
	   && !no_upos(mtmp)
	   && attacktype(mdat, AT_WEAP)) {
	    struct obj *mw_tmp;

	    /* The scared check is necessary.  Otherwise a monster that is
	     * one square near the player but fleeing into a wall would keep	
	     * switching between pick-axe and weapon.  If monster is stuck
	     * in a trap, prefer ranged weapon (wielding is done in thrwmu).
	     * This may cost the monster an attack, but keeps the monster
	     * from switching back and forth if carrying both.
	     */
	    mw_tmp = MON_WEP(mtmp);
	    if (!(scared && mw_tmp && is_pick(mw_tmp)) &&
		mtmp->weapon_check == NEED_WEAPON &&
		!(mtmp->mtrapped && !nearby && select_rwep(mtmp))) {
			mtmp->combat_mode = HNDHND_MODE;
			mtmp->weapon_check = NEED_HTH_WEAPON;
			if (mon_wield_item(mtmp) != 0) return(0);
	    }
	}
/*      Look for other monsters to fight (at a distance) */
	if ((
	      (attacktype(mtmp->data, AT_GAZE) && !mtmp->mcan) ||
	      attacktype(mtmp->data, AT_ARRW) ||
	      attacktype(mtmp->data, AT_LNCK) ||
	      attacktype(mtmp->data, AT_LRCH) ||
	      attacktype(mtmp->data, AT_5SQR) ||
	      attacktype(mtmp->data, AT_5SBT) ||
	      (!mtmp->mspec_used && 
			(attacktype(mtmp->data, AT_SPIT) ||
			 attacktype(mtmp->data, AT_TNKR) ||
			 attacktype(mtmp->data, AT_BEAM) ||
			 (attacktype(mtmp->data, AT_BREA) && !mtmp->mcan)
			)
		  )||
	     (attacktype(mtmp->data, AT_MMGC) && !mtmp->mcan &&
			(((attacktype_fordmg(mtmp->data, AT_MMGC, AD_ANY))->adtyp
	         <= AD_SPC2))
	      ) ||
	     (attacktype(mtmp->data, AT_MAGC) && !mtmp->mcan &&
	      (((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp
	         <= AD_SPC2))
	      ) ||
	     (attacktype(mtmp->data, AT_MAGC) && !mtmp->mcan &&
	      (((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp
	         == AD_RBRE))
	      ) ||
	     (attacktype(mtmp->data, AT_MAGC) && !mtmp->mcan &&
	      (((attacktype_fordmg(mtmp->data, AT_MAGC, AD_ANY))->adtyp
	         == AD_OONA))
	      ) ||
	     (attacktype(mtmp->data, AT_WEAP) &&
	      (select_rwep(mtmp) != 0)) ||
	      find_offensive(mtmp)) && 
	    mtmp->mlstmv != monstermoves)
	{
	    register struct monst *mtmp2 = mfind_target(mtmp, FALSE);
	    if (mtmp2 && 
	        (mtmp2 != &youmonst || 
				dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) > 2) &&
			(mtmp2 != mtmp)
		){
	        int res;
			mon_ranged_gazeonly = 1;//State variable
			res = (mtmp2 == &youmonst) ? mattacku(mtmp)
		                           : mattackm(mtmp, mtmp2);
			/* note: mattacku and mattackm have different returns */
			if ((mtmp2 == &youmonst) ? (res == 1) : (res & MM_AGR_DIED)) return 1; /* Oops. */

			if(!(mon_ranged_gazeonly))
				return 0; /* that was our move for the round */
	    }
	}

/*	Now the actual movement phase	*/

#ifndef GOLDOBJ
	if(!nearby || (mtmp->mflee && mtmp->data != &mons[PM_BANDERSNATCH]) || scared ||
	   mtmp->mconf || mtmp->mstun || (mtmp->minvis && !rn2(3)) ||
	   (mdat->mlet == S_LEPRECHAUN && !u.ugold && (mtmp->mgold || rn2(2))) ||
#else
    if (mdat->mlet == S_LEPRECHAUN) {
	    ygold = findgold(invent);
	    lepgold = findgold(mtmp->minvent);
	}
	if(!nearby || (mtmp->mflee && mtmp->data != &mons[PM_BANDERSNATCH]) || scared ||
	   mtmp->mconf || mtmp->mstun || (mtmp->minvis && !rn2(3)) ||
	   (mdat->mlet == S_LEPRECHAUN && !ygold && (lepgold || rn2(2))) ||
#endif
	   (is_wanderer(mdat) && !rn2(4)) || (Conflict && !mtmp->iswiz) ||
	   (is_blind(mtmp) && !rn2(4)) || (mtmp->mpeaceful && mdat != &mons[PM_NURSE])
	) {
		/* Possibly cast an undirected spell if not attacking you */
		/* note that most of the time castmu() will pick a directed
		   spell and do nothing, so the monster moves normally */
		/* arbitrary distance restriction to keep monster far away
		   from you from having cast dozens of sticks-to-snakes
		   or similar spells by the time you reach it */
		if (dist2(mtmp->mx, mtmp->my, u.ux, u.uy) <= 49 && !mtmp->mspec_used) {
		    struct attack *a;

		    for (a = &mdat->mattk[0]; a < &mdat->mattk[NATTK]; a++) {
			if ((a->aatyp == AT_MAGC || a->aatyp == AT_MMGC) && (a->adtyp == AD_SPEL || a->adtyp == AD_CLRC)) {
			    if (mtmp->mux==u.ux && mtmp->muy==u.uy && couldsee(mtmp->mx, mtmp->my) && !mtmp->mpeaceful && 
					dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= BOLT_LIM*BOLT_LIM
				){
					if(castmu(mtmp, a, TRUE, TRUE)){
						tmp = 3;
						// if(mdat != &mons[PM_DEMOGORGON]) break;
					}
			    } else {
					if(castmu(mtmp, a, FALSE, FALSE)){
						tmp = 3;
						// if(mdat != &mons[PM_DEMOGORGON]) break;
					}
				}
			}
		    }
		}

		tmp = m_move(mtmp, 0);
		distfleeck(mtmp,&inrange,&nearby,&scared);	/* recalc */

		switch (tmp) {
		    case 0:	/* no movement, but it can still attack you */
		    case 3:	/* absolutely no movement */
				/* for pets, case 0 and 3 are equivalent */
			/* vault guard might have vanished */
			if (mtmp->isgd && (mtmp->mhp < 1 ||
					    (mtmp->mx == 0 && mtmp->my == 0)))
			    return 1;	/* behave as if it died */
			/* During hallucination, monster appearance should
			 * still change - even if it doesn't move.
			 */
			if(Hallucination) newsym(mtmp->mx,mtmp->my);
			break;
		    case 1:	/* monster moved */
			/* Maybe it stepped on a trap and fell asleep... */
			if (mtmp->msleeping || !(mtmp->mcanmove && mtmp->mnotlaugh)) return(0);
			/* Long worms thrash around */
			if(mtmp->wormno && (!mtmp->mpeaceful || Conflict)) wormhitu(mtmp);
			if(!nearby &&
			  (ranged_attk(mdat) || find_offensive(mtmp))){
				if(mdat == &mons[PM_GREAT_CTHULHU] || mdat == &mons[PM_WATCHER_IN_THE_WATER] || mdat == &mons[PM_KETO] || mdat == &mons[PM_ARCADIAN_AVENGER]){
					break;
			    } else return(0);
			    // return(0);
			}
 			else if(u.uswallow && mtmp == u.ustuck) {
			    /* a monster that's digesting you can move at the
			     * same time -dlc
			     */
			    return(mattacku(mtmp));
			} else
				return(0);
			/*NOTREACHED*/
			break;
		    case 2:	/* monster died */
			return(1);
		}
	}
/*	Now, attack the player if possible - one attack set per monst	*/

	if (!mtmp->mpeaceful || mdat == &mons[PM_NURSE] ||
	    (Conflict && !resist(mtmp, RING_CLASS, 0, 0))) {
	    if(inrange && !noattacks(mdat) && u.uhp > 0 && !scared && tmp != 3)
			if(mattacku(mtmp)) return(1); /* monster died (e.g. exploded) */
	}
	/* special speeches for quest monsters */
	if (!mtmp->msleeping && mtmp->mcanmove && mtmp->mnotlaugh && nearby)
	    quest_talk(mtmp);
	/* extra emotional attack for vile monsters */
	if (inrange && mtmp->data->msound == MS_CUSS && !mtmp->mpeaceful &&
		couldsee(mtmp->mx, mtmp->my) && ((!mtmp->minvis && !rn2(5)) || 
										mtmp->data == &mons[PM_SIR_GARLAND] || mtmp->data == &mons[PM_GARLAND] ||
										(mtmp->data == &mons[PM_CHAOS] && (mtmp->mvar2 < 5 || !rn2(5)) )|| 
										mtmp->data == &mons[PM_APOLLYON]
	) )
	    cuss(mtmp);

	return(tmp == 2);
}

static NEARDATA const char practical[] = { WEAPON_CLASS, ARMOR_CLASS, GEM_CLASS, FOOD_CLASS, 0 };
static NEARDATA const char magical[] = {
	AMULET_CLASS, POTION_CLASS, SCROLL_CLASS, WAND_CLASS, RING_CLASS,
	SPBOOK_CLASS, 0 };
static NEARDATA const char indigestion[] = { BALL_CLASS, ROCK_CLASS, 0 };
static NEARDATA const char boulder_class[] = { ROCK_CLASS, 0 };
static NEARDATA const char gem_class[] = { GEM_CLASS, 0 };

boolean
itsstuck(mtmp)
register struct monst *mtmp;
{
	if (sticks(youracedata) && mtmp==u.ustuck && !u.uswallow) {
		pline("%s cannot escape from you!", Monnam(mtmp));
		return(TRUE);
	}
	return(FALSE);
}

/* Return values:
 * 0: did not move, but can still attack and do other stuff.
 * 1: moved, possibly can attack.
 * 2: monster died.
 * 3: did not move, and can't do anything else either.
 */
int
m_move(mtmp, after)
register struct monst *mtmp;
register int after;
{
	int appr;
	xchar gx,gy,nix,niy,chcnt;
	int chi;	/* could be schar except for stupid Sun-2 compiler */
	boolean likegold=0, likegems=0, likeobjs=0, likemagic=0, conceals=0;
	boolean likerock=0, can_tunnel=0;
	boolean can_open=0, can_unlock=0, doorbuster=0;
	boolean uses_items=0, setlikes=0;
	boolean avoid=FALSE;
	struct permonst *ptr;
	struct monst *mtoo;
	schar mmoved = 0;	/* not strictly nec.: chi >= 0 will do */
	long info[9];
	long flag;
	int  omx = mtmp->mx, omy = mtmp->my;
	struct obj *mw_tmp;

	if (stationary(mtmp->data) || sessile(mtmp->data)) return(0);
	if(mtmp->mtrapped) {
	    int i = mintrap(mtmp);
	    if(i >= 2) { newsym(mtmp->mx,mtmp->my); return(2); }/* it died */
	    if(i == 1) return(0);	/* still in trap, so didn't move */
	}
	ptr = mtmp->data; /* mintrap() can change mtmp->data -dlc */

	if (mtmp->meating) {
		if(mtmp->meating < 0){
			pline("ERROR RECOVERY: Negative meating timer (%d) set to 0", mtmp->meating);
			mtmp->meating = 0;
		}
		if(mtmp->meating > 1000){
			pline("ERROR RECOVERY: Overlong meating timer (%d) truncated to 100 turns", mtmp->meating);
			mtmp->meating = 0;
		}
	    else mtmp->meating--;
	    return 3;			/* still eating */
	}
	if (hides_under(ptr) && OBJ_AT(mtmp->mx, mtmp->my) && !mtmp->mtame && rn2(10))
	    return 0;		/* do not leave hiding place */

	set_apparxy(mtmp);
	/* where does mtmp think you are? */
	/* Not necessary if m_move called from this file, but necessary in
	 * other calls of m_move (ex. leprechauns dodging)
	 */
#ifdef REINCARNATION
	if (!Is_rogue_level(&u.uz))
#endif
	    can_tunnel = tunnels(ptr);
	can_open = !(nohands(ptr) || verysmall(ptr));
	can_unlock = ((can_open && (m_carrying(mtmp, SKELETON_KEY)||m_carrying(mtmp, UNIVERSAL_KEY))) ||
		      mtmp->iswiz || is_rider(ptr) || ptr==&mons[PM_DREAD_SERAPH]);
	doorbuster = is_giant(ptr);
	if(mtmp->wormno) goto not_special;
	/* my dog gets special treatment */
	if(mtmp->mtame) {
	    mmoved = dog_move(mtmp, after);
	    goto postmov;
	}

	/* likewise for shopkeeper */
	if(mtmp->isshk) {
	    mmoved = shk_move(mtmp);
	    if(mmoved == -2) return(2);
	    if(mmoved >= 0) goto postmov;
	    mmoved = 0;		/* follow player outside shop */
	}

	/* and for the guard */
	if(mtmp->isgd) {
	    mmoved = gd_move(mtmp);
	    if(mmoved == -2) return(2);
	    if(mmoved >= 0) goto postmov;
	    mmoved = 0;
	}

	/* and the acquisitive monsters get special treatment */
	if(is_covetous(ptr)) {
	    xchar tx = STRAT_GOALX(mtmp->mstrategy),
		  ty = STRAT_GOALY(mtmp->mstrategy);
	    struct monst *intruder = m_at(tx, ty);
	    /*
	     * if there's a monster on the object or in possesion of it,
	     * attack it.
	     */
	    if((dist2(mtmp->mx, mtmp->my, tx, ty) < 2) &&
	       intruder && (intruder != mtmp)) {
			notonhead = (intruder->mx != tx || intruder->my != ty);
			if(mattackm(mtmp, intruder) == 2) return(2);
			mmoved = 1;
			goto postmov;
		} else if(mtmp->data != &mons[PM_DEMOGORGON] 
			   && mtmp->data!=&mons[PM_ELDER_PRIEST]
			   /*&& mtmp->data!=&mons[PM_SHAMI_AMOURAE]*/
			   ){
			mmoved = 0;
		    goto postmov;
		}
//	    goto postmov;
	}

	/* and for the priest */
	if(mtmp->ispriest) {
	    mmoved = pri_move(mtmp);
	    if(mmoved == -2) return(2);
	    if(mmoved >= 0) goto postmov;
	    mmoved = 0;
	}

#ifdef MAIL
	if(ptr == &mons[PM_MAIL_DAEMON]) {
	    if(flags.soundok && canseemon(mtmp))
		verbalize("I'm late!");
	    mongone(mtmp);
	    return(2);
	}
#endif

	/* teleport if that lies in our nature */
	if((mteleport(ptr) || mtmp->mfaction == TOMB_HERD) && !rn2(5) && !mtmp->mcan &&
	   !tele_restrict(mtmp) && !(noactions(mtmp))
	) {
	    if(mtmp->mhp < 7 || mtmp->mpeaceful || rn2(2))
		(void) rloc(mtmp, FALSE);
	    else
		mnexto(mtmp);
	    mmoved = 1;
	    goto postmov;
	}
not_special:
	if(u.uswallow && !mtmp->mflee && u.ustuck != mtmp) return(1);
	omx = mtmp->mx;
	omy = mtmp->my;
	gx = mtmp->mux;
	gy = mtmp->muy;
	appr = (mtmp->mflee && mtmp->data != &mons[PM_BANDERSNATCH]) ? -1 : 1;
	if (mtmp->mconf || (u.uswallow && mtmp == u.ustuck) || (gx == 0 && gy == 0))
		appr = 0;
	else {
#ifdef GOLDOBJ
		struct obj *lepgold, *ygold;
#endif
		boolean should_see = mon_can_see_you(mtmp);
		
		if ((youmonst.m_ap_type == M_AP_OBJECT && youmonst.mappearance == STRANGE_OBJECT) || u.uundetected ||
		    (youmonst.m_ap_type == M_AP_OBJECT && youmonst.mappearance == GOLD_PIECE && !likes_gold(ptr)) ||
		    (mtmp->mpeaceful && !mtmp->isshk) ||  /* allow shks to follow */
		    ((monsndx(ptr) == PM_STALKER || is_bat(ptr) || monsndx(ptr) == PM_HUNTING_HORROR ||
		      ptr->mlet == S_LIGHT) && !rn2(3)))
			appr = 0;

		if(monsndx(ptr) == PM_LEPRECHAUN && (appr == 1) &&
#ifndef GOLDOBJ
		   (mtmp->mgold > u.ugold))
#else
		   ( (lepgold = findgold(mtmp->minvent)) && 
                   (lepgold->quan > ((ygold = findgold(invent)) ? ygold->quan : 0L)) ))
#endif
			appr = -1;

		if (!should_see && can_track(ptr) && (goodsmeller(ptr) || (mtmp)->mcansee)) {
			register coord *cp;

			cp = gettrack(omx,omy);
			if (cp) {
				gx = cp->x;
				gy = cp->y;
			}
		}
	}

	if (( !mtmp->mpeaceful || mtmp->data == &mons[PM_MAID] || !rn2(10) )
#ifdef REINCARNATION
				    && (!Is_rogue_level(&u.uz))
#endif
	) {
	    boolean in_line = lined_up(mtmp) &&
		!no_upos(mtmp) &&
		(distmin(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <=
		    (throws_rocks(youracedata) ? 20 : ACURRSTR/2+1)
		);

	    if ((appr == 0 || !in_line) && !is_derived_undead_mon(mtmp)) {
			/* Monsters in combat or fleeing won't pick stuff up, avoiding the
			 * situation where you toss arrows at it and it has nothing
			 * better to do than pick the arrows up.
			 */
			register int pctload = (curr_mon_load(mtmp) * 100) /
				max_mon_load(mtmp);
			
			/* look for gold or jewels nearby */
			likegold = (likes_gold(ptr) && pctload < 95);
			likegems = (likes_gems(ptr) && pctload < 85);
			uses_items = (!mindless_mon(mtmp) && !is_animal(ptr)
				&& pctload < 75);
			likeobjs = (likes_objs(ptr) && pctload < 75);
			likemagic = (likes_magic(ptr) && pctload < 85);
			likerock = (throws_rocks(ptr) && pctload < 50 && !In_sokoban(&u.uz));
			conceals = hides_under(ptr);
			setlikes = TRUE;
	    }
	}

#define SQSRCHRADIUS	5

      { register int minr = SQSRCHRADIUS;	/* not too far away */
	register struct obj *otmp;
	register int xx, yy;
	int oomx, oomy, lmx, lmy;
	
	/* cut down the search radius if it thinks character is closer. */
	if(distmin(mtmp->mux, mtmp->muy, omx, omy) < SQSRCHRADIUS &&
		!no_upos(mtmp) &&
	    (!mtmp->mpeaceful || mtmp->data == &mons[PM_NURSE])) minr--;
	/* guards shouldn't get too distracted */
	if(!mtmp->mpeaceful && is_mercenary(ptr)) minr = 1;

	if((likegold || likegems || likeobjs || likemagic || likerock || conceals)
	      && (!*in_rooms(omx, omy, SHOPBASE) || (!rn2(25) && !mtmp->isshk))) {
	look_for_obj:
	    oomx = min(COLNO-1, omx+minr);
	    oomy = min(ROWNO-1, omy+minr);
	    lmx = max(1, omx-minr);
	    lmy = max(0, omy-minr);
	    for(otmp = fobj; otmp; otmp = otmp->nobj) {
		/* monsters may pick rocks up, but won't go out of their way
		   to grab them; this might hamper sling wielders, but it cuts
		   down on move overhead by filtering out most common item */
		if (otmp->otyp == ROCK) continue;
		/* skip objects not in line-of-sight */
		if (!((mtmp->mx == otmp->ox && mtmp->my == otmp->oy)
			|| clear_path(mtmp->mx, mtmp->my, otmp->ox, otmp->oy))) continue;
		xx = otmp->ox;
		yy = otmp->oy;
		/* Nymphs take everything.  Most other creatures should not
		 * pick up corpses except as a special case like in
		 * searches_for_item().  We need to do this check in
		 * mpickstuff() as well.
		 */
		if(xx >= lmx && xx <= oomx && yy >= lmy && yy <= oomy) {
		    /* don't get stuck circling around an object that's underneath
		       an immobile or hidden monster; paralysis victims excluded */
		    if ((mtoo = m_at(xx,yy)) != 0 &&
			(mtoo->msleeping || mtoo->mundetected ||
			 (mtoo->mappearance && !mtoo->iswiz) ||
			 !mtoo->data->mmove)) continue;
			//Don't approach artifacts you can't touch
			if(otmp->oartifact && !touch_artifact(otmp, mtmp, TRUE))
				continue;
			
		    if(((likegold && otmp->oclass == COIN_CLASS) ||
		       (likeobjs && index(practical, otmp->oclass) &&
			(otmp->otyp != CORPSE || (ptr->mlet == S_NYMPH
			   && !is_rider(&mons[otmp->corpsenm])))) ||
		       (likemagic && index(magical, otmp->oclass)) ||
		       (uses_items && searches_for_item(mtmp, otmp)) ||
		       (likerock && is_boulder(otmp)) ||
		       (likegems && otmp->oclass == GEM_CLASS &&
			otmp->obj_material != MINERAL) ||
		       (conceals && !cansee(otmp->ox,otmp->oy)) ||
		       (ptr == &mons[PM_GELATINOUS_CUBE] &&
			!index(indigestion, otmp->oclass) &&
			!(otmp->otyp == CORPSE &&
			  touch_petrifies(&mons[otmp->corpsenm])))
		      ) && touch_artifact(otmp, mtmp, FALSE)) {
				if(can_carry(mtmp,otmp) &&
				 (throws_rocks(ptr) ||
				  !boulder_at(xx,yy)) &&
				 (!is_unicorn(ptr) ||
				  otmp->obj_material == GEMSTONE) &&
				   /* Don't get stuck circling an Elbereth */
				  !(onscary(xx, yy, mtmp))) {
					minr = distmin(omx,omy,xx,yy);
					oomx = min(COLNO-1, omx+minr);
					oomy = min(ROWNO-1, omy+minr);
					lmx = max(1, omx-minr);
					lmy = max(0, omy-minr);
					gx = otmp->ox;
					gy = otmp->oy;
					if(mtmp->data == &mons[PM_MAID] && appr == 0) appr = 1;
					if (gx == omx && gy == omy) {
						mmoved = 3; /* actually unnecessary */
						goto postmov;
					}
				}
		    }
		}
	    }
	} else if(likegold) {
	    /* don't try to pick up anything else, but use the same loop */
	    uses_items = 0;
	    likegems = likeobjs = likemagic = likerock = conceals = 0;
	    goto look_for_obj;
	}

	if(minr < SQSRCHRADIUS && appr == -1) {
	    if(!no_upos(mtmp) && distmin(omx,omy,mtmp->mux,mtmp->muy) <= 3) {
		gx = mtmp->mux;
		gy = mtmp->muy;
	    } else
		appr = 1;
	}
      }

	/* don't tunnel if hostile and close enough to prefer a weapon */
	if (can_tunnel && needspick(ptr) &&
	    ((!mtmp->mpeaceful || Conflict) && !no_upos(mtmp) && 
	     dist2(mtmp->mx, mtmp->my, mtmp->mux, mtmp->muy) <= 8))
	    can_tunnel = FALSE;

	nix = omx;
	niy = omy;
	flag = 0L;
	if (mtmp->mpeaceful && (!Conflict || resist(mtmp, RING_CLASS, 0, 0)))
	    flag |= (ALLOW_SANCT | ALLOW_SSM);
	else flag |= ALLOW_U;
	if (is_minion(ptr) || is_rider(ptr)) flag |= ALLOW_SANCT;
	/* unicorn may not be able to avoid hero on a noteleport level */
	if (notonline(ptr) && ((mon_resistance(mtmp,TELEPORT) || is_unicorn(ptr)) && !level.flags.noteleport)) flag |= NOTONL;
	if (mon_resistance(mtmp,PASSES_WALLS)) flag |= (ALLOW_WALL | ALLOW_ROCK);
	if (passes_bars(mtmp) && !Is_illregrd(&u.uz) ) flag |= ALLOW_BARS;
	if (can_tunnel) flag |= ALLOW_DIG;
	if (is_human(ptr) || ptr == &mons[PM_MINOTAUR]) flag |= ALLOW_SSM;
	if (is_undead_mon(mtmp) && ptr->mlet != S_GHOST && ptr->mlet != S_SHADE) flag |= NOGARLIC;
	if (throws_rocks(ptr)) flag |= ALLOW_ROCK;
	if (can_open) flag |= OPENDOOR;
	if (can_unlock) flag |= UNLOCKDOOR;
	if (doorbuster) flag |= BUSTDOOR;
	{
	    register int i, j, nx, ny, nearer;
		int leader_target = FALSE;
	    int jcnt, cnt;
	    int ndist, nidist;
	    register coord *mtrk;
	    coord poss[9];

	    cnt = mfndpos(mtmp, poss, info, flag);
	    chcnt = 0;
	    jcnt = min(MTSZ, cnt-1);
	    chi = -1;
	    nidist = dist2(nix,niy,gx,gy);
	    /* allow monsters be shortsighted on some levels for balance */
	    if((!mtmp->mpeaceful || mtmp->data == &mons[PM_NURSE]) && level.flags.shortsighted &&
	       nidist > (couldsee(nix,niy) ? 144 : 36) && appr == 1) appr = 0;
	    if (notonline(ptr)){
			int curappr = appr;
			if(appr == 1){
				appr = -1; //If approaching, default to running away instead
			}
			/* on noteleport levels, perhaps we cannot avoid hero */
			for(i = 0; i < cnt; i++){
				if(!(info[i] & NOTONL)) avoid=TRUE;
				else appr = curappr; //If was approaching and swithced to running away, reinstates approaching
			}
	    }
		
		if(appr == 0){
			struct monst *m2 = (struct monst *)0;
			int distminbest = SQSRCHRADIUS;
			for(m2=fmon; m2; m2 = m2->nmon){
				if(distmin(mtmp->mx,mtmp->my,m2->mx,m2->my) < distminbest
						&& mm_aggression(mtmp,m2)
						&& (clear_path(mtmp->mx, mtmp->my, m2->mx, m2->my) || !rn2(10))
						&& mon_can_see_mon(mtmp, m2)
				){
					distminbest = distmin(mtmp->mx,mtmp->my,m2->mx,m2->my);
					leader_target = FALSE;
					gx = m2->mx;
					gy = m2->my;
					mtmp->mux = m2->mx;
					mtmp->muy = m2->my;
					if(MON_WEP(mtmp) && 
						(is_launcher(MON_WEP(mtmp)) || is_firearm(MON_WEP(mtmp)) )
					){
						if(distmin(mtmp->mx,mtmp->my,m2->mx,m2->my) >= BOLT_LIM) appr = 1;
						else if(distmin(mtmp->mx,mtmp->my,m2->mx,m2->my) < 4) appr = -1;
						else appr = 0;
					} else {
						appr = 1;
					}
				}
			}//End target closest hostile
			
			if(Role_if(PM_ANACHRONONAUT) && !mtmp->mpeaceful && In_quest(&u.uz)){
				for(m2=fmon; m2; m2 = m2->nmon){
					if(m2->m_id == quest_status.leader_m_id){
						if(distminbest >= SQSRCHRADIUS){
							/*make a beeline for the leader*/
							leader_target = TRUE;
							gx = m2->mx;
							gy = m2->my;
							mtmp->mux = m2->mx;
							mtmp->muy = m2->my;
							appr = 1;
						}
						break;
					}
			}
			}//End target ana leader
		}
		if(Role_if(PM_ANACHRONONAUT) && !mtmp->mpeaceful && In_quest(&u.uz) && Is_qstart(&u.uz)){
			if(mtmp->mhp == mtmp->mhpmax && (
				(mtmp->data == &mons[PM_DEEP_ONE] && !rn2(1000))
				|| (mtmp->data == &mons[PM_DEEPER_ONE] && !rn2(500))
				|| (mtmp->data == &mons[PM_DEEPEST_ONE] && !rn2(100))
			)){
				int i, j, count=0;
				for(i = -1; i < 2; i++) for(j = -1; j < 2; j++){
					if(isok(mtmp->mx+i, mtmp->my+j) && IS_WALL(levl[mtmp->mx+i][mtmp->my+j].typ)){
						count++;
					}
				}
				if(count){
					count = rn2(count);
					struct obj *breacher;
					for(i = -1; i < 2; i++) for(j = -1; j < 2; j++){
						if(isok(mtmp->mx+i, mtmp->my+j) && IS_WALL(levl[mtmp->mx+i][mtmp->my+j].typ)){
							if(count-- == 0){
								breacher = mksobj(STICK_OF_DYNAMITE, FALSE, FALSE);
								breacher->quan = 1L;
								breacher->cursed = 0;
								breacher->blessed = 0;
								breacher->age = rn1(10,10);
								fix_object(breacher);
								place_object(breacher, mtmp->mx+i, mtmp->my+j);
								begin_burn(breacher, FALSE);
								if(canseemon(mtmp))
									pline("%s plants a breaching charge!", Monnam(mtmp));
							}
						}
					}
				}
			} else if((mtmp->data == &mons[PM_DEEP_ONE] && !rn2(20))
				|| (mtmp->data == &mons[PM_DEEPER_ONE] && !rn2(5))
				|| (mtmp->data == &mons[PM_DEEPEST_ONE])
			){
				int i, j, count=0;
				for(i = -1; i < 2; i++) for(j = -1; j < 2; j++){
					if(isok(mtmp->mx+i, mtmp->my+j) && t_at(mtmp->mx+i, mtmp->my+j) && t_at(mtmp->mx+i, mtmp->my+j)->ttyp == PIT){
						count++;
					}
				}
				if(count){
					count = rn2(count);
					struct obj *breacher;
					for(i = -1; i < 2; i++) for(j = -1; j < 2; j++){
						if(isok(mtmp->mx+i, mtmp->my+j) && t_at(mtmp->mx+i, mtmp->my+j) && t_at(mtmp->mx+i, mtmp->my+j)->ttyp == PIT){
							if(count-- == 0){
								delfloortrap(t_at(mtmp->mx+i, mtmp->my+j));
								if(canseemon(mtmp))
									pline("%s fills in a trench!", Monnam(mtmp));
							}
						}
					}
				}
			}
		}
		
	    for(i=0; i < cnt; i++) {
			if (avoid && (info[i] & NOTONL)) continue;
			nx = poss[i].x;
			ny = poss[i].y;

			if (appr != 0) {
				mtrk = &mtmp->mtrack[0];
				for(j=0; j < jcnt; mtrk++, j++)
				if(nx == mtrk->x && ny == mtrk->y)
					if(rn2(4*(cnt-j)))
					goto nxti;
			}

			nearer = ((ndist = dist2(nx,ny,gx,gy)) < nidist);

			if((appr == 1 && nearer) || (appr == -1 && !nearer) ||
			   (!appr && !rn2(++chcnt)) || !mmoved) {
				nix = nx;
				niy = ny;
				nidist = ndist;
				chi = i;
				mmoved = 1;
			}
			nxti:	;
	    }
	}

	if(mmoved) {
	    register int j;

	    if (mmoved==1 && (u.ux != nix || u.uy != niy) && itsstuck(mtmp))
		return(3);

	    if (((IS_ROCK(levl[nix][niy].typ) && may_dig(nix,niy)) ||
		 closed_door(nix, niy)) &&
		mmoved==1 && can_tunnel && needspick(ptr)) {
		if (closed_door(nix, niy)) {
		    if (!(mw_tmp = MON_WEP(mtmp)) ||
			!is_pick(mw_tmp) || !is_axe(mw_tmp))
			mtmp->weapon_check = NEED_PICK_OR_AXE;
		} else if (IS_TREE(levl[nix][niy].typ)) {
		    if (!(mw_tmp = MON_WEP(mtmp)) || !is_axe(mw_tmp))
			mtmp->weapon_check = NEED_AXE;
		} else if (!(mw_tmp = MON_WEP(mtmp)) || !is_pick(mw_tmp)) {
		    mtmp->weapon_check = NEED_PICK_AXE;
		}
		if (mtmp->weapon_check >= NEED_PICK_AXE) {
			if (mon_wield_item(mtmp))
				return(3);	/* did not move, spent turn wielding item */
			else
				return(0);	/* can't move into that position, but didn't take time wielding item */
		}
	    }
	    /* If ALLOW_U is set, either it's trying to attack you, or it
	     * thinks it is.  In either case, attack this spot in preference to
	     * all others.
	     */
	/* Actually, this whole section of code doesn't work as you'd expect.
	 * Most attacks are handled in dochug().  It calls distfleeck(), which
	 * among other things sets nearby if the monster is near you--and if
	 * nearby is set, we never call m_move unless it is a special case
	 * (confused, stun, etc.)  The effect is that this ALLOW_U (and
	 * mfndpos) has no effect for normal attacks, though it lets a confused
	 * monster attack you by accident.
	 */
	    if(info[chi] & ALLOW_U) {
		nix = mtmp->mux;
		niy = mtmp->muy;
	    }
	    if (nix == u.ux && niy == u.uy) {
		mtmp->mux = u.ux;
		mtmp->muy = u.uy;
		return(0);
	    }
	    /* The monster may attack another based on 1 of 2 conditions:
	     * 1 - It may be confused.
	     * 2 - It may mistake the monster for your (displaced) image.
	     * Pets get taken care of above and shouldn't reach this code.
	     * Conflict gets handled even farther away (movemon()).
	     */
		/* Actually, it seems to be THREE conditions:
		 * 3 - It may hate the other monster, as per mm_aggression in mon.c.
		 * Pet-to-pet combat seems to be handled elsewhere, though
		 */
	    if((info[chi] & ALLOW_M) ||
		   (nix == mtmp->mux && niy == mtmp->muy)) {
		struct monst *mtmp2;
		int mstatus;
		mtmp2 = m_at(nix,niy);

		notonhead = mtmp2 && (nix != mtmp2->mx || niy != mtmp2->my);
		/* note: mstatus returns 0 if mtmp2 is nonexistent */
		mstatus = mattackm(mtmp, mtmp2);
		
		if (mstatus & MM_AGR_DIED)		/* aggressor died */
		    return 2;

		if ((mstatus & MM_HIT) && !(mstatus & MM_DEF_DIED)  &&
		    rn2(4) && mtmp2->movement >= NORMAL_SPEED /*don't counter allied nurses*/
			&& (mtmp->data != &mons[PM_NURSE] || mtmp->mpeaceful != mtmp2->mpeaceful)) {
			mtmp2->movement -= NORMAL_SPEED;
		    notonhead = 0;
		    mstatus = mattackm(mtmp2, mtmp);	/* return attack */
		    if (mstatus & MM_DEF_DIED)
			return 2;
		}
		return 3;
	    }

	    if (!m_in_out_region(mtmp,nix,niy))
	        return 3;
	    remove_monster(omx, omy);
	    place_monster(mtmp, nix, niy);
	    for(j = MTSZ-1; j > 0; j--)
		mtmp->mtrack[j] = mtmp->mtrack[j-1];
	    mtmp->mtrack[0].x = omx;
	    mtmp->mtrack[0].y = omy;
		if(mtmp->data == &mons[PM_HEMORRHAGIC_THING]){
			//The thing leaves bloody footprints, that appear *after* the monster has passed by
			if(isok(mtmp->mtrack[1].x, mtmp->mtrack[1].y) && mtmp->mtrack[1].x != 0){
				struct engr *oep = engr_at(mtmp->mtrack[1].x, mtmp->mtrack[1].y);
				if(!oep){
					make_engr_at(mtmp->mtrack[1].x, mtmp->mtrack[1].y,
					 "", 0L, DUST);
					oep = engr_at(mtmp->mtrack[1].x, mtmp->mtrack[1].y);
				}
				if(oep && (oep->ward_type == DUST || oep->ward_type == ENGR_BLOOD)){
					oep->ward_id = FOOTPRINT;
					oep->halu_ward = 1;
					oep->ward_type = ENGR_BLOOD;
					oep->complete_wards = 1;
				}
			}
		}
	    /* Place a segment at the old position. */
	    if (mtmp->wormno) worm_move(mtmp);
		
		if(mtmp->data == &mons[PM_SURYA_DEVA]){
			struct monst *blade;
			for(blade = fmon; blade; blade = blade->nmon) if(blade->data == &mons[PM_DANCING_BLADE] && mtmp->m_id == blade->mvar1) break;
			if(blade){
				int bx = blade->mx, by = blade->my;
				remove_monster(bx, by);
				place_monster(blade, omx, omy);
				newsym(omx,omy);
				newsym(bx,by);
			}
		}
	} else {
	    if(is_unicorn(ptr) && rn2(2) && !tele_restrict(mtmp) && 
			!(mtmp->data->maligntyp < 0 && !(noactions(mtmp)))
		) {
			(void) rloc(mtmp, FALSE);
			return(1);
	    }
	    if(mtmp->wormno) worm_nomove(mtmp);
	}
postmov:
	if(mmoved == 1 || mmoved == 3) {
	    boolean canseeit = cansee(mtmp->mx, mtmp->my);

	    if(mmoved == 1) {
		newsym(omx,omy);		/* update the old position */
		if (mintrap(mtmp) >= 2) {
		    if(mtmp->mx) newsym(mtmp->mx,mtmp->my);
		    return(2);	/* it died */
		}
		ptr = mtmp->data;

		/* open a door, or crash through it, if you can */
		if(IS_DOOR(levl[mtmp->mx][mtmp->my].typ)
			&& !mon_resistance(mtmp,PASSES_WALLS) /* doesn't need to open doors */
			&& !can_tunnel /* taken care of below */
		      ) {
		    struct rm *here = &levl[mtmp->mx][mtmp->my];
		    boolean btrapped = (here->doormask & D_TRAPPED);

		    if(here->doormask & (D_LOCKED|D_CLOSED) && amorphous(ptr)) {
			if (flags.verbose && canseemon(mtmp))
			    pline("%s %s under the door.", Monnam(mtmp),
				  (ptr == &mons[PM_FOG_CLOUD] ||
				   ptr == &mons[PM_YELLOW_LIGHT])
				  ? "flows" : "oozes");
		    } else if(here->doormask & D_LOCKED && can_unlock) {
			if(btrapped) {
			    here->doormask = D_NODOOR;
			    newsym(mtmp->mx, mtmp->my);
			    unblock_point(mtmp->mx,mtmp->my); /* vision */
			    if(mb_trapped(mtmp)) return(2);
			} else {
			    if (flags.verbose) {
				if (canseeit)
				   You("see a door unlock and open.");
				else if (flags.soundok)
				   You_hear("a door unlock and open.");
			    }
			    here->doormask = D_ISOPEN;
			    /* newsym(mtmp->mx, mtmp->my); */
			    unblock_point(mtmp->mx,mtmp->my); /* vision */
			}
		    } else if (here->doormask == D_CLOSED && can_open) {
			if(btrapped) {
			    here->doormask = D_NODOOR;
			    newsym(mtmp->mx, mtmp->my);
			    unblock_point(mtmp->mx,mtmp->my); /* vision */
			    if(mb_trapped(mtmp)) return(2);
			} else {
			    if (flags.verbose) {
				if (canseeit)
				     You("see a door open.");
				else if (flags.soundok)
				     You_hear("a door open.");
			    }
			    here->doormask = D_ISOPEN;
			    /* newsym(mtmp->mx, mtmp->my); */  /* done below */
			    unblock_point(mtmp->mx,mtmp->my); /* vision */
			}
		    } else if (here->doormask & (D_LOCKED|D_CLOSED)) {
			/* mfndpos guarantees this must be a doorbuster */
			if(btrapped) {
			    here->doormask = D_NODOOR;
			    newsym(mtmp->mx, mtmp->my);
			    unblock_point(mtmp->mx,mtmp->my); /* vision */
			    if(mb_trapped(mtmp)) return(2);
			} else {
			    if (flags.verbose) {
				if (canseeit)
				    You("see a door crash open.");
				else if (flags.soundok)
				    You_hear("a door crash open.");
			    }
			    if (here->doormask & D_LOCKED && !rn2(2))
				    here->doormask = D_NODOOR;
			    else here->doormask = D_BROKEN;
			    /* newsym(mtmp->mx, mtmp->my); */ /* done below */
			    unblock_point(mtmp->mx,mtmp->my); /* vision */
			}
			/* if it's a shop door, schedule repair */
			if (*in_rooms(mtmp->mx, mtmp->my, SHOPBASE))
			    add_damage(mtmp->mx, mtmp->my, 0L);
		    }
		} else if (levl[mtmp->mx][mtmp->my].typ == IRONBARS && !Is_illregrd(&u.uz)) {
		    if ( (dmgtype(ptr,AD_RUST) && ptr != &mons[PM_NAIAD]) || dmgtype(ptr,AD_CORR)) {
				if (canseemon(mtmp)) {
					pline("%s eats through the iron bars.", 
					Monnam(mtmp)); 
				}
				dissolve_bars(mtmp->mx, mtmp->my);
				if(mtmp->data == &mons[PM_RUST_MONSTER] && mtmp->mtame && mtmp->isminion){
					EDOG(mtmp)->hungrytime += 5*objects[BAR].oc_nutrition;
				}
			}
		    else if (flags.verbose && canseemon(mtmp))
				Norep("%s %s %s the iron bars.", Monnam(mtmp),
				  /* pluralization fakes verb conjugation */
				  makeplural(locomotion(mtmp, "pass")),
				  mon_resistance(mtmp,PASSES_WALLS) ? "through" : "between");
		}

		/* possibly dig */
		if (can_tunnel && mdig_tunnel(mtmp))
			return(2);  /* mon died (position already updated) */

		/* set also in domove(), hack.c */
		if (u.uswallow && mtmp == u.ustuck &&
					(mtmp->mx != omx || mtmp->my != omy)) {
		    /* If the monster moved, then update */
		    u.ux0 = u.ux;
		    u.uy0 = u.uy;
		    u.ux = mtmp->mx;
		    u.uy = mtmp->my;
		    swallowed(0);
		} else
		newsym(mtmp->mx,mtmp->my);
	    }
	    if(OBJ_AT(mtmp->mx, mtmp->my) && mtmp->mcanmove && mtmp->mnotlaugh) {
		/* recompute the likes tests, in case we polymorphed
		 * or if the "likegold" case got taken above */
		if (setlikes) {
		    register int pctload = (curr_mon_load(mtmp) * 100) /
			max_mon_load(mtmp);

		    /* look for gold or jewels nearby */
		    likegold = (likes_gold(ptr) && pctload < 95);
		    likegems = (likes_gems(ptr) && pctload < 85);
		    uses_items = (!mindless_mon(mtmp) && !is_animal(ptr)
				  && pctload < 75);
		    likeobjs = (likes_objs(ptr) && pctload < 75);
		    likemagic = (likes_magic(ptr) && pctload < 85);
		    likerock = (throws_rocks(ptr) && pctload < 50 &&
				!In_sokoban(&u.uz));
		    conceals = hides_under(ptr);
		}

		/* Maybe a rock mole just ate some metal object */
		if (metallivorous(ptr)) {
		    if (meatmetal(mtmp) == 2) return 2;	/* it died */
		}

		if(g_at(mtmp->mx,mtmp->my) && likegold) mpickgold(mtmp);

		/* Maybe a cube ate just about anything */
		if (ptr == &mons[PM_GELATINOUS_CUBE]) {
		    if (meatobj(mtmp) == 2) return 2;	/* it died */
		}

		if(!*in_rooms(mtmp->mx, mtmp->my, SHOPBASE) || !rn2(25)) {
		    boolean picked = FALSE;

		    if(likeobjs) picked |= mpickstuff(mtmp, practical);
		    if(likemagic) picked |= mpickstuff(mtmp, magical);
		    if(likerock) picked |= mpickstuff(mtmp, boulder_class);
		    if(likegems) picked |= mpickstuff(mtmp, gem_class);
		    if(uses_items) picked |= mpickstuff(mtmp, (char *)0);
		    if(picked) mmoved = 3;
		}

		if(mtmp->minvis) {
		    newsym(mtmp->mx, mtmp->my);
		    if (mtmp->wormno) see_wsegs(mtmp);
		}
	    }

	    if(hides_under(ptr) || ptr->mlet == S_EEL) {
		/* Always set--or reset--mundetected if it's already hidden
		   (just in case the object it was hiding under went away);
		   usually set mundetected unless monster can't move.  */
		if (mtmp->mundetected ||
			(mtmp->mcanmove && mtmp->mnotlaugh && !mtmp->msleeping && rn2(5)))
		    mtmp->mundetected = (ptr->mlet != S_EEL) ?
			OBJ_AT(mtmp->mx, mtmp->my) :
			(is_pool(mtmp->mx, mtmp->my, FALSE) && !Is_waterlevel(&u.uz));
		newsym(mtmp->mx, mtmp->my);
	    }
	    if (mtmp->isshk) {
		after_shk_move(mtmp);
	    }
	}
	return(mmoved);
}

#endif /* OVL0 */
#ifdef OVL2

/* break iron bars at the given location */
void
break_iron_bars(x, y, heard)
int x, y;			/* coordinates of iron bars */
boolean heard;		/* print You_hear() message? */
{
	int numbars;
	struct obj *obj;

	if (levl[x][y].typ != IRONBARS) {
		impossible("Breaking non-existant iron bars @ (%d,%d)", x, y);
		return;
	}

	if (heard)
		You_hear("a sharp crack!");
	levl[x][y].typ = (Is_special(&u.uz) || *in_rooms(x, y, 0)) ? ROOM : CORR;

	for (numbars = d(2, 4) - 1; numbars > 0; numbars--){
		obj = mksobj_at(BAR, x, y, FALSE, FALSE);
		set_material(obj, Is_illregrd(&u.uz) ? METAL : IRON);
		obj->spe = 0;
		obj->cursed = obj->blessed = FALSE;
	}
	newsym(x, y);
	return;
}

//Malcolm Ryan's bar eating patch
void
dissolve_bars(x, y)
register int x, y;
{
    levl[x][y].typ = (Is_special(&u.uz) || *in_rooms(x,y,0)) ? ROOM : CORR; 
    newsym(x, y);    
}


boolean
closed_door(x, y)
register int x, y;
{
	return((boolean)(IS_DOOR(levl[x][y].typ) &&
			(levl[x][y].doormask & (D_LOCKED | D_CLOSED))));
}

boolean
accessible(x, y)
register int x, y;
{
	return((boolean)(ACCESSIBLE(levl[x][y].typ) && !closed_door(x, y)));
}

#endif /* OVL2 */
#ifdef OVL0

/* decide where the monster thinks you are standing */
void
set_apparxy(mtmp)
register struct monst *mtmp;
{
	boolean notseen, gotu = FALSE;
	register int disp, mx = mtmp->mux, my = mtmp->muy;
#ifdef GOLDOBJ
	long umoney = money_cnt(invent);
#endif

	/*
	 * do cheapest and/or most likely tests first
	 */

	/* pet knows your smell; grabber still has hold of you */
	if (mtmp->mtame || mtmp == u.ustuck) goto found_you;

	/* monsters which know where you are don't suddenly forget,
	   if you haven't moved away.  Assuming they aren't crazy. */
	if (mx == u.ux && my == u.uy && (!mtmp->mcrazed || rn2(4))) goto found_you;

	notseen = !mon_can_see_you(mtmp);
	/* Can also learn your position via hearing you */
	if(couldsee(mtmp->mx,mtmp->my) &&
		distu(mtmp->mx,mtmp->my) <= 100 &&
		(!Stealth || (mtmp->data == &mons[PM_ETTIN] && rn2(10))) &&
		(Aggravate_monster || ((sensitive_ears(mtmp->data) || !rn2(7)) && !is_deaf(mtmp)))
	) {
		notseen = FALSE;
	}
	
	/* add cases as required.  eg. Displacement ... */
	if(notseen){
		if((distmin(mtmp->mx,mtmp->my,mx,my) <= 1 && distmin(mtmp->mx,mtmp->my,u.ux,u.uy) > 1) || !rn2(100)){
			/*well, now it has NO clue where you are...*/
			mtmp->mux = 0;
			mtmp->muy = 0;
		}
		if(is_wanderer(mtmp->data) && !rn2(20)){
			/*x goes from 1 to colno-1, y goes from 0 to rowno-1*/
			mtmp->mux = rnd(COLNO-1);
			mtmp->muy = rn2(ROWNO);
		}
		/* Otherwise, don't change the current */
		return;
	} else if ( Underwater) {
		disp = 1;
	} else if (Displaced || mtmp->mcrazed) {
	    disp = couldsee(mx, my) ? 2 : 1;
	} else disp = 0;
	if (!disp) goto found_you;

	/* without something like the following, invis. and displ.
	   are too powerful */
	gotu = Displaced ? !rn2(4) : FALSE;

#if 0		/* this never worked as intended & isn't needed anyway */
	/* If invis but not displaced, staying around gets you 'discovered' */
	gotu |= (!Displaced && u.dx == 0 && u.dy == 0);
#endif

	if (!gotu) {
	    register int try_cnt = 0;
	    do {
		if (++try_cnt > 200) goto found_you;		/* punt */
		mx = u.ux - disp + rn2(2*disp+1);
		my = u.uy - disp + rn2(2*disp+1);
	    } while (!isok(mx,my)
		  || (disp != 2 && mx == mtmp->mx && my == mtmp->my)
		  || ((mx != u.ux || my != u.uy) &&
		      !mon_resistance(mtmp,PASSES_WALLS) &&
		      (!ACCESSIBLE(levl[mx][my].typ) ||
		       (closed_door(mx, my) && !can_ooze(mtmp))))
		  || !couldsee(mx, my));
	} else {
found_you:
	    mx = u.ux;
	    my = u.uy;
	}

	mtmp->mux = mx;
	mtmp->muy = my;
}

boolean
can_ooze(mtmp)
struct monst *mtmp;
{
	struct obj *chain, *obj;

	if (!amorphous(mtmp->data)) return FALSE;
	if (mtmp == &youmonst) {
#ifndef GOLDOBJ
		if (u.ugold > 100L) return FALSE;
#endif
		chain = invent;
	} else {
#ifndef GOLDOBJ
		if (mtmp->mgold > 100L) return FALSE;
#endif
		chain = mtmp->minvent;
	}
	for (obj = chain; obj; obj = obj->nobj) {
		int typ = obj->otyp;

#ifdef GOLDOBJ
                if (typ == COIN_CLASS && obj->quan > 100L) return FALSE;
#endif
		if (obj->oclass != GEM_CLASS &&
		    !(typ >= ARROW && typ <= BOOMERANG) &&
		    !(typ >= DAGGER && typ <= CRYSKNIFE) &&
		    typ != SLING &&
		    !is_cloak(obj) && typ != FEDORA &&
		    !is_gloves(obj) && typ != JACKET &&
#ifdef TOURIST
		    typ != CREDIT_CARD && !is_shirt(obj) &&
#endif
		    !(typ == CORPSE && verysmall(&mons[obj->corpsenm])) &&
		    typ != FORTUNE_COOKIE && typ != CANDY_BAR &&
		    typ != PANCAKE && typ != LEMBAS_WAFER &&
		    !(typ >= LUMP_OF_ROYAL_JELLY && typ <= HONEYCOMB) &&
		    obj->oclass != AMULET_CLASS &&
		    obj->oclass != RING_CLASS &&
#ifdef WIZARD
		    obj->oclass != VENOM_CLASS &&
#endif
		    typ != SACK && typ != BAG_OF_HOLDING &&
		    typ != BAG_OF_TRICKS && !Is_candle(obj) &&
		    typ != OILSKIN_SACK && typ != LEASH &&
		    typ != STETHOSCOPE && typ != BLINDFOLD && 
		    typ != ANDROID_VISOR && 
			typ != TOWEL && typ != R_LYEHIAN_FACEPLATE &&
		    typ != WHISTLE && typ != MAGIC_WHISTLE &&
		    typ != MAGIC_MARKER && typ != TIN_OPENER &&
		    typ != SKELETON_KEY && typ != UNIVERSAL_KEY &&
			typ != LOCK_PICK
		) return FALSE;
		if (Is_container(obj) && obj->cobj) return FALSE;
		    
	}
	return TRUE;
}

void
remove_monster(x, y)
int x;
int y;
{
	struct monst * mtmp = level.monsters[x][y];
	
	if(mtmp){
		if (opaque(mtmp->data))
			unblock_point(mtmp->mx, mtmp->my);
		mtmp = (struct monst *)0;
		level.monsters[x][y] = (struct monst *)0;
	}
	return;
}

#endif /* OVL0 */

/*monmove.c*/
