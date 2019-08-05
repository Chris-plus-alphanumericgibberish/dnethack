/*	SCCS Id: @(#)apply.c	3.4	2003/11/18	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"

#ifdef OVLB

static const char tools[] = { COIN_CLASS, CHAIN_CLASS, TOOL_CLASS, WEAPON_CLASS, WAND_CLASS, 0 };
static const char tools_too[] = { COIN_CLASS, ALL_CLASSES, TOOL_CLASS, POTION_CLASS,
				  WEAPON_CLASS, WAND_CLASS, GEM_CLASS, CHAIN_CLASS, 0 };
static const char apply_armor[] = { ARMOR_CLASS, 0 };
static const char apply_corpse[] = { FOOD_CLASS, 0 };
static const char apply_all[] = { ALL_CLASSES, CHAIN_CLASS, 0 };

#ifdef TOURIST
STATIC_DCL int FDECL(use_camera, (struct obj *));
#endif
STATIC_DCL int FDECL(do_present_ring, (struct obj *));
STATIC_DCL int FDECL(use_towel, (struct obj *));
STATIC_DCL boolean FDECL(its_dead, (int,int,int *,struct obj*));
STATIC_DCL int FDECL(use_stethoscope, (struct obj *));
STATIC_DCL void FDECL(use_whistle, (struct obj *));
STATIC_DCL void FDECL(use_leash, (struct obj *));
STATIC_DCL int FDECL(use_mirror, (struct obj *));
STATIC_DCL void FDECL(use_candelabrum, (struct obj *));
STATIC_DCL void FDECL(use_candle, (struct obj **));
STATIC_DCL void FDECL(use_lamp, (struct obj *));
STATIC_DCL int FDECL(swap_aegis, (struct obj *));
STATIC_DCL int FDECL(use_rakuyo, (struct obj *));
STATIC_DCL int FDECL(use_force_blade, (struct obj *));
STATIC_DCL void FDECL(light_cocktail, (struct obj *));
STATIC_DCL void FDECL(light_torch, (struct obj *));
STATIC_DCL void FDECL(use_tinning_kit, (struct obj *));
STATIC_DCL void FDECL(use_figurine, (struct obj **));
STATIC_DCL void FDECL(use_grease, (struct obj *));
STATIC_DCL void FDECL(use_trap, (struct obj *));
STATIC_DCL void FDECL(use_stone, (struct obj *));
STATIC_DCL int FDECL(use_sensor, (struct obj *));
STATIC_DCL int NDECL(sensorMenu);
STATIC_DCL int FDECL(use_hypospray, (struct obj *));
STATIC_DCL int FDECL(use_droven_cloak, (struct obj **));
STATIC_DCL int FDECL(use_darkweavers_cloak, (struct obj *));
STATIC_PTR int NDECL(set_trap);		/* occupation callback */
STATIC_DCL int FDECL(use_pole, (struct obj *));
STATIC_DCL int FDECL(use_cream_pie, (struct obj *));
STATIC_DCL int FDECL(use_grapple, (struct obj *));
STATIC_DCL int FDECL(use_crook, (struct obj *));
STATIC_DCL int FDECL(do_break_wand, (struct obj *));
STATIC_DCL int FDECL(do_flip_coin, (struct obj *));
STATIC_DCL boolean FDECL(figurine_location_checks,
				(struct obj *, coord *, BOOLEAN_P));
STATIC_DCL boolean NDECL(uhave_graystone);
STATIC_DCL void FDECL(add_class, (char *, CHAR_P));
STATIC_DCL int FDECL(do_carve_obj, (struct obj *));
STATIC_PTR int FDECL(pick_rune, (BOOLEAN_P));
STATIC_DCL void FDECL(describe_rune, (int));
STATIC_PTR char NDECL(pick_carvee);


#ifdef	AMIGA
void FDECL( amii_speaker, ( struct obj *, char *, int ) );
#endif

static const char no_elbow_room[] = "don't have enough elbow-room to maneuver.";

#ifdef TOURIST
STATIC_OVL int
use_camera(obj)
	struct obj *obj;
{
	register struct monst *mtmp;

	if(Underwater) {
		pline("Using your camera underwater would void the warranty.");
		return 0;
	}
	if(!getdir((char *)0)) return 0;

	if (obj->spe <= 0) {
		pline("%s", nothing_happens);
		return (1);
	}
	consume_obj_charge(obj, TRUE);

	if (obj->cursed && !rn2(2)) {
		(void) zapyourself(obj, TRUE);
	} else if (u.uswallow) {
		You("take a picture of %s %s.", s_suffix(mon_nam(u.ustuck)),
		    mbodypart(u.ustuck, STOMACH));
	} else if (u.dz) {
		You("take a picture of the %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
	} else if (!u.dx && !u.dy) {
		(void) zapyourself(obj, TRUE);
	} else if ((mtmp = bhit(u.dx, u.dy, COLNO, FLASHED_LIGHT,
				(int FDECL((*),(MONST_P,OBJ_P)))0,
				(int FDECL((*),(OBJ_P,OBJ_P)))0,
				obj, NULL)) != 0) {
		obj->ox = u.ux,  obj->oy = u.uy;
		(void) flash_hits_mon(mtmp, obj);
	}
	return 1;
}
#endif

STATIC_OVL int
do_present_ring(obj)
	struct obj *obj;
{
	register struct monst *mtmp, *tm;

	if(!getdir((char *)0)) return 0;
	
	if(obj->oward == 0 && !(obj->ohaluengr)){
		exercise(A_WIS, FALSE);
		return 0;
	}
	
	if (u.uswallow) {
		You("display the ring's engraving to %s %s.", s_suffix(mon_nam(u.ustuck)),
		    mbodypart(u.ustuck, STOMACH));
		pline("Nothing happens.");
		exercise(A_WIS, FALSE);
		return 0;
	} else if (u.dz) {
		You("display the ring's engraving to the %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
		if(u.dz < 0){
			pline("Nothing happens.");
			exercise(A_WIS, FALSE);
			return 0;
		}
		if(is_lava(u.ux, u.uy) || flags.beginner || !(u.ualign.type == A_CHAOTIC || Hallucination) ){
			pline("Nothing happens.");
			if(u.ualign.type == A_LAWFUL) exercise(A_WIS, FALSE);
			return 0;
		}else{
			You_feel("as though the engraving on the ring could fall right off!");
			if(yn("Give it a push?") == 'n'){
				pline("Nothing happens.");
				return 0;
			}
			else{
				struct engr *engrHere = engr_at(u.ux,u.uy);
				if(u.ualign.type == A_LAWFUL) exercise(A_WIS, FALSE);
				if (IS_ALTAR(levl[u.ux][u.uy].typ)) {
					altar_wrath(u.ux, u.uy);
					return 0;
				}
				if(!engrHere){
					make_engr_at(u.ux, u.uy,	"", (moves - multi), DUST); /* absense of text =  dust */
					engrHere = engr_at(u.ux,u.uy); /*note: make_engr_at does not return the engraving it made, it returns void instead*/
				}
				if(obj->ohaluengr == engrHere->halu_ward && obj->oward == engrHere->ward_id){
					pline("the engraving tumbles off the ring to join it's fellows.");
					engrHere->complete_wards += engrHere->halu_ward ? 0 : get_num_wards_added(engrHere->ward_id,engrHere->complete_wards);
					obj->ohaluengr = FALSE;
					obj->oward = FALSE;
				}
				else{
					pline("the engraving tumbles off the ring%s.", engrHere->ward_id ? "and covers the existing drawing" : "");
					engrHere->ward_id = obj->oward;
					engrHere->halu_ward = obj->ohaluengr;
					engrHere->complete_wards = engrHere->halu_ward ? 1 : get_num_wards_added(engrHere->ward_id,0);
					engrHere->ward_type = obj->blessed ? BURN : obj->cursed ? DUST : ENGRAVE;
					if( !(obj->ohaluengr) && !(u.wardsknown & get_wardID(engrHere->ward_id)) ){
						You("have learned a new warding sign!");
						u.wardsknown |= get_wardID(engrHere->ward_id);
					}
					obj->ohaluengr = FALSE;
					obj->oward = FALSE;
				}
			}
		}
	} else if (!u.dx && !u.dy) {
		if(!(obj->ohaluengr)){
			pline("A %s is engraved on the ring.",wardDecode[obj->oward]);
			if( !(u.wardsknown & get_wardID(obj->oward)) ){
				You("have learned a new warding sign!");
				u.wardsknown |= get_wardID(obj->oward);
			}
		}
		else{
			pline("There is %s engraved on the ring.",fetchHaluWard((int)obj->oward));
		}
		return(1);
	} else if (isok(u.ux+u.dx, u.uy+u.dy) && (mtmp = m_at(u.ux+u.dx, u.uy+u.dy)) != 0) {
		You("display the ring's engraving to %s.", mon_nam(mtmp));
		if (obj->cursed && rn2(3)) {
			pline("But the ring's engraving is fogged over!");
			return 1;
		}
		if(!(obj->ohaluengr) || obj->oward == CERULEAN_SIGN){
			if(
				(obj->oward == HEPTAGRAM && scaryHept(1, mtmp)) ||
				(obj->oward == GORGONEION && scaryGorg(1, mtmp)) ||
				(obj->oward == CIRCLE_OF_ACHERON && scaryCircle(1, mtmp)) ||
				(obj->oward == PENTAGRAM && scaryPent(1, mtmp)) ||
				(obj->oward == HEXAGRAM && scaryHex(1, mtmp)) ||
				(obj->oward == HAMSA && scaryHam(1, mtmp)) ||
				( (obj->oward == ELDER_SIGN || obj->oward == CERULEAN_SIGN) && scarySign(1, mtmp)) ||
				(obj->oward == ELDER_ELEMENTAL_EYE && scaryEye(1, mtmp)) ||
				(obj->oward == SIGN_OF_THE_SCION_QUEEN && scaryQueen(1, mtmp)) ||
				(obj->oward == CARTOUCHE_OF_THE_CAT_LORD && scaryCat(1, mtmp)) ||
				(obj->oward == WINGS_OF_GARUDA && scaryWings(1, mtmp)) ||
/*				(obj->oward == SIGIL_OF_CTHUGHA && (1, mtmp)) ||
				(obj->oward == BRAND_OF_ITHAQUA && (1, mtmp)) ||
				(obj->oward == TRACERY_OF_KARAKAL && (1, mtmp)) || These wards curently don't have scaryX functions. */
				(obj->oward == YELLOW_SIGN && scaryYellow(1, mtmp)) ||
				(obj->oward == TOUSTEFNA && scaryTou(mtmp)) ||
				(obj->oward == DREPRUN && scaryDre(mtmp)) ||
/*				(obj->oward == OTTASTAFUR && (mtmp)) ||
				(obj->oward == KAUPALOKI && (mtmp)) || Unimplemented runes. */
				(obj->oward == VEIOISTAFUR && scaryVei(mtmp)) ||
				(obj->oward == THJOFASTAFUR && scaryThj(mtmp))
			){
				if (rn2(7))
					monflee(mtmp, rnd(10), TRUE, TRUE);
				else
					monflee(mtmp, rnd(100), TRUE, TRUE);
			}
		} else if(obj->ohaluengr && obj->oward >= FIRST_DROW_SYM && obj->oward <= LAST_DROW_SYM && 
			(is_elf(mtmp->data) || is_drow(mtmp->data) || mtmp->data == &mons[PM_EDDERKOP])
		){
			if(flags.stag && 
				(mtmp->mfaction == u.start_house || allied_faction(mtmp->mfaction,u.start_house)) && 
				obj->oward == EDDER_SYMBOL && 
				!(mtmp->female)
			){
				verbalize("The revolution has begun!");
				for(tm = fmon; tm; tm = tm->nmon){
					if((is_drow(tm->data) && (obj->oward == tm->mfaction || allied_faction(obj->oward, tm->mfaction))) || 
						((obj->oward == EDDER_SYMBOL || obj->oward == XAXOX) &&  tm->data == &mons[PM_EDDERKOP]) ||
						((tm->mfaction == u.start_house || allied_faction(tm->mfaction,u.start_house)) && 
							obj->oward == EDDER_SYMBOL && !(tm->female))
					){
						if(is_drow(tm->data)) tm->mfaction = EDDER_SYMBOL;
						tm->housealert = 1;
						tm->mpeaceful = 1;
					} else if(is_drow(tm->data) && !(obj->oward == tm->mfaction || allied_faction(obj->oward, tm->mfaction)) && mtmp->female){
						tm->housealert = 1;
						tm->mpeaceful = 0;
					}
				}
			} else if((obj->oward == mtmp->mfaction || allied_faction(obj->oward, mtmp->mfaction)) || 
				(obj->oward == EILISTRAEE_SYMBOL && is_elf(mtmp->data)) || 
				((obj->oward == EDDER_SYMBOL || obj->oward == XAXOX) &&  mtmp->data == &mons[PM_EDDERKOP])
			){
				if(mtmp->housealert && !(mtmp->mpeaceful)){
					verbalize("Die, spy!");
					for(tm = fmon; tm; tm = tm->nmon){
						if((is_drow(tm->data) && (obj->oward == tm->mfaction || allied_faction(obj->oward, tm->mfaction))) || 
							((obj->oward == EDDER_SYMBOL || obj->oward == XAXOX) &&  tm->data == &mons[PM_EDDERKOP]) ||
							(obj->oward == EILISTRAEE_SYMBOL && is_elf(tm->data))
						){
							tm->housealert = 1;
							tm->mpeaceful = 0;
						}
					}
				} else if(flags.female){
					if((obj->oward == XAXOX && uarm && uarm->oward && uarm->oward == obj->oward) ||
					   (obj->oward == EDDER_SYMBOL && uarm && uarm->oward && uarm->oward == obj->oward) ||
						!(uarm) || !(uarm->oward) || uarm->oward == obj->oward ||
						(
						 uarm->oward == LOLTH_SYMBOL && 
						 obj->oward != EILISTRAEE_SYMBOL &&   
						 obj->oward != XAXOX &&   
						 obj->oward != EDDER_SYMBOL
						)
					){
						verbalize("She's one of ours!");
						if(obj->oward != XAXOX && obj->oward != EDDER_SYMBOL) verbalize("Apologies, my lady!");
						for(tm = fmon; tm; tm = tm->nmon){
							if((is_drow(tm->data) && (obj->oward == tm->mfaction || allied_faction(obj->oward, tm->mfaction))) || 
								((obj->oward == EDDER_SYMBOL || obj->oward == XAXOX) &&  tm->data == &mons[PM_EDDERKOP]) ||
								(obj->oward == EILISTRAEE_SYMBOL && is_elf(tm->data))
							){
								tm->housealert = 1;
								tm->mpeaceful = 1;
							}
						}
					} else {
						verbalize("Die, spy!");
						for(tm = fmon; tm; tm = tm->nmon){
							if((is_drow(tm->data) && (obj->oward == tm->mfaction || allied_faction(obj->oward, tm->mfaction))) || 
								((obj->oward == EDDER_SYMBOL || obj->oward == XAXOX) &&  tm->data == &mons[PM_EDDERKOP]) ||
								(obj->oward == EILISTRAEE_SYMBOL && is_elf(tm->data))
							){
								tm->housealert = 1;
								tm->mpeaceful = 0;
							}
						}
					}
				} else {
					if(((obj->oward <= LAST_TOWER && obj->oward >= FIRST_TOWER) && 
						(!(uarm) || !(uarm->oward) || uarm->oward == obj->oward || allied_faction(uarm->oward,obj->oward))) ||
					   ((obj->oward == EDDER_SYMBOL || 
					     obj->oward == XAXOX || 
					     obj->oward == GHAUNADAUR_SYMBOL || 
					     obj->oward == LAST_BASTION_SYMBOL || 
						 obj->oward == EILISTRAEE_SYMBOL) && (!(uarm) || !(uarm->oward) || uarm->oward == obj->oward || allied_faction(uarm->oward,obj->oward))) ||
					   (uarm && uarm->oward && uarm->oward == obj->oward)
					){
						verbalize("He's one of ours!");
						verbalize("Move along, sir.");
						for(tm = fmon; tm; tm = tm->nmon){
							if((is_drow(tm->data) && (obj->oward == tm->mfaction || allied_faction(obj->oward, tm->mfaction))) || 
								((obj->oward == EDDER_SYMBOL || obj->oward == XAXOX) &&  tm->data == &mons[PM_EDDERKOP]) ||
								(obj->oward == EILISTRAEE_SYMBOL && is_elf(tm->data))
							){
								tm->mpeaceful = 1;
							}
						}
					} else {
						verbalize("Die, spy!");
						for(tm = fmon; tm; tm = tm->nmon){
							if((is_drow(tm->data) && (obj->oward == tm->mfaction || allied_faction(obj->oward, tm->mfaction))) || 
								((obj->oward == EDDER_SYMBOL || obj->oward == XAXOX) &&  tm->data == &mons[PM_EDDERKOP]) ||
								(obj->oward == EILISTRAEE_SYMBOL && is_elf(tm->data))
							){
								tm->housealert = 1;
								tm->mpeaceful = 0;
							}
						}
					}
				}
			} else {
				verbalize("Die!");
				mtmp->housealert = 1;
				for(tm = fmon; tm; tm = tm->nmon){
					if((is_drow(tm->data) && (mtmp->mfaction == tm->mfaction || allied_faction(mtmp->mfaction, tm->mfaction))) || 
						((mtmp->mfaction == EDDER_SYMBOL || mtmp->mfaction == XAXOX || tm->data == &mons[PM_EDDERKOP]) && 
							(tm->mfaction == EDDER_SYMBOL || tm->mfaction == XAXOX || tm->data == &mons[PM_EDDERKOP])) ||
						((mtmp->mfaction == EILISTRAEE_SYMBOL || is_elf(mtmp->data)) && 
							(tm->mfaction == EILISTRAEE_SYMBOL || is_elf(tm->data)))
					){
						tm->housealert = 1;
						tm->mpeaceful = 0;
					}
				}
			}
		}
	}
	return 1;
}

STATIC_OVL int
use_towel(obj)
	struct obj *obj;
{
	if(!freehand()) {
		You("have no free %s!", body_part(HAND));
		return 0;
	} else if (obj->owornmask) {
		You("cannot use it while you're wearing it!");
		return 0;
	} else if (obj->cursed) {
		long old;
		switch (rn2(3)) {
		case 2:
		    old = Glib;
		    Glib += rn1(10, 3);
		    Your("%s %s!", makeplural(body_part(HAND)),
			(old ? "are filthier than ever" : "get slimy"));
		    return 1;
		case 1:
		    if (!ublindf) {
			old = u.ucreamed;
			u.ucreamed += rn1(10, 3);
			pline("Yecch! Your %s %s gunk on it!", body_part(FACE),
			      (old ? "has more" : "now has"));
			make_blinded(Blinded + (long)u.ucreamed - old, TRUE);
		    } else {
			const char *what = (ublindf->otyp == LENSES) ?
					    "lenses" : (ublindf->otyp == MASK || ublindf->otyp == LIVING_MASK || ublindf->otyp == R_LYEHIAN_FACEPLATE) ? "mask" : "blindfold";
			if (ublindf->cursed) {
			    You("push your %s %s.", what,
				rn2(2) ? "cock-eyed" : "crooked");
			} else {
			    struct obj *saved_ublindf = ublindf;
			    You("push your %s off.", what);
			    Blindf_off(ublindf);
			    dropx(saved_ublindf);
			}
		    }
		    return 1;
		case 0:
		    break;
		}
	}

	if (Glib) {
		Glib = 0;
		You("wipe off your %s.", makeplural(body_part(HAND)));
		return 1;
	} else if(u.ucreamed) {
		Blinded -= u.ucreamed;
		u.ucreamed = 0;

		if (!Blinded) {
			pline("You've got the glop off.");
			Blinded = 1;
			make_blinded(0L,TRUE);
		} else {
			Your("%s feels clean now.", body_part(FACE));
		}
		return 1;
	}

	Your("%s and %s are already clean.",
		body_part(FACE), makeplural(body_part(HAND)));

	return 0;
}

/* maybe give a stethoscope message based on floor objects */
STATIC_OVL boolean
its_dead(rx, ry, resp, tobj)
int rx, ry, *resp;
struct obj* tobj;
{
	struct obj *otmp;
	struct trap *ttmp;

	if (!can_reach_floor()) return FALSE;

	/* additional stethoscope messages from jyoung@apanix.apana.org.au */
	if (Hallucination && sobj_at(CORPSE, rx, ry)) {
	    /* (a corpse doesn't retain the monster's sex,
	       so we're forced to use generic pronoun here) */
	    You_hear("a voice say, \"It's dead, Jim.\"");
	    *resp = 1;
	    return TRUE;
	} else if (Role_if(PM_HEALER) && ((otmp = sobj_at(CORPSE, rx, ry)) != 0 ||
				    (otmp = sobj_at(STATUE, rx, ry)) != 0 || (otmp = sobj_at(FOSSIL, rx, ry)) != 0)) {
	    /* possibly should check uppermost {corpse,statue} in the pile
	       if both types are present, but it's not worth the effort */
	    if (vobj_at(rx, ry)->otyp == STATUE) otmp = vobj_at(rx, ry);
	    if (otmp->otyp == CORPSE || otmp->otyp == FOSSIL) {
		You("determine that %s unfortunate being is %sdead.",
		    (rx == u.ux && ry == u.uy) ? "this" : "that", (otmp->otyp == FOSSIL) ? "very ":"");
	    } else {
		ttmp = t_at(rx, ry);
		pline("%s appears to be in %s health for a statue.",
		      The(mons[otmp->corpsenm].mname),
		      (ttmp && ttmp->ttyp == STATUE_TRAP) ?
			"extraordinary" : "excellent");
	    }
	    return TRUE;
	}

	/* listening to eggs is a little fishy, but so is stethoscopes detecting alignment
	 * The overcomplex wording is because all the monster-naming functions operate
	 * on actual instances of the monsters, and we're dealing with just an index
	 * so we can avoid things like "a owlbear", etc. */
	if ((otmp = sobj_at(EGG,rx,ry))) {
		if (Hallucination) {
			pline("You listen to the egg and guess... %s?",rndmonnam());
		} else {
			if (stale_egg(otmp) || otmp->corpsenm == NON_PM) {
				pline("The egg doesn't make much noise at all.");
			} else {
				pline("You listen to the egg and guess... %s?",mons[otmp->corpsenm].mname);
				learn_egg_type(otmp->corpsenm);
			}
		}
		return TRUE;
	}


	return FALSE;
}

static const char hollow_str[] = "a hollow sound.  This must be a secret %s!";

/* Strictly speaking it makes no sense for usage of a stethoscope to
   not take any time; however, unless it did, the stethoscope would be
   almost useless.  As a compromise, one use per turn is free, another
   uses up the turn; this makes curse status have a tangible effect. */
STATIC_OVL int
use_stethoscope(obj)
	register struct obj *obj;
{
	struct monst *mtmp;
	struct rm *lev;
	int rx, ry, res;
	boolean interference = (u.uswallow && is_whirly(u.ustuck->data) &&
				!rn2(Role_if(PM_HEALER) ? 10 : 3));

	if (nohands(youracedata)) {	/* should also check for no ears and/or deaf */
		You("have no hands!");	/* not `body_part(HAND)' */
		return 0;
	} else if (!freehand()) {
		You("have no free %s.", body_part(HAND));
		return 0;
	}
	if (!getdir((char *)0)) return 0;

	res = partial_action();

#ifdef STEED
	if (u.usteed && u.dz > 0) {
		if (interference) {
			pline("%s interferes.", Monnam(u.ustuck));
			mstatusline(u.ustuck);
		} else
			mstatusline(u.usteed);
		return res;
	} else
#endif
	if (u.uswallow && (u.dx || u.dy || u.dz)) {
		mstatusline(u.ustuck);
		return res;
	} else if (u.uswallow && interference) {
		pline("%s interferes.", Monnam(u.ustuck));
		mstatusline(u.ustuck);
		return res;
	} else if (u.dz) {
		if (Underwater)
		    You_hear("faint splashing.");
		else if (u.dz < 0 || !can_reach_floor())
		    You_cant("reach the %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
		else if (its_dead(u.ux, u.uy, &res, obj))
		    ;	/* message already given */
		else if (Is_stronghold(&u.uz))
		    You_hear("the crackling of hellfire.");
		else
		    pline_The("%s seems healthy enough.", surface(u.ux,u.uy));
		return res;
	} else if (obj->cursed && !rn2(2)) {
		You_hear("your %s %s.", body_part(HEART), body_part(BEAT));
		return res;
	}
	if (Stunned || (Confusion && !rn2(5))) confdir();
	if (!u.dx && !u.dy) {
		ustatusline();
		return res;
	}
	rx = u.ux + u.dx; ry = u.uy + u.dy;
	if (!isok(rx,ry)) {
		You_hear("a faint typing noise.");
		return 0;
	}
	if ((mtmp = m_at(rx,ry)) != 0) {
		mstatusline(mtmp);
		if (mtmp->mundetected) {
			mtmp->mundetected = 0;
			if (cansee(rx,ry)) newsym(mtmp->mx,mtmp->my);
		}
		if (!canspotmon(mtmp))
			map_invisible(rx,ry);
		return res;
	}
	if (glyph_is_invisible(levl[rx][ry].glyph)) {
		unmap_object(rx, ry);
		newsym(rx, ry);
		pline_The("invisible monster must have moved.");
	}
	lev = &levl[rx][ry];
	switch(lev->typ) {
	case SDOOR:
		You_hear(hollow_str, "door");
		cvt_sdoor_to_door(lev);		/* ->typ = DOOR */
		if (Blind) feel_location(rx,ry);
		else newsym(rx,ry);
		return res;
	case SCORR:
		You_hear(hollow_str, "passage");
		lev->typ = CORR;
		unblock_point(rx,ry);
		if (Blind) feel_location(rx,ry);
		else newsym(rx,ry);
		return res;
	}

	if (!its_dead(rx, ry, &res, obj))
	    You_hear("nothing special.");	/* not You_hear()  */
	return res;
}

static const char whistle_str[] = "produce a %s whistling sound.";

STATIC_OVL void
use_whistle(obj)
struct obj *obj;
{
	register struct monst *mtmp;

	You(whistle_str, obj->cursed ? "shrill" : "high");
	wake_nearby_noisy();
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (!DEADMONSTER(mtmp)) {
			if (mtmp->mtame && !mtmp->isminion)
				EDOG(mtmp)->whistletime = moves;
	    }
	}
}

void
use_magic_whistle(obj)
struct obj *obj;
{
	register struct monst *mtmp, *nextmon;

	if(obj->cursed && !rn2(2)) {
		You("produce a high-pitched humming noise.");
		wake_nearby();
	} else {
		int pet_cnt = 0;
		You(whistle_str, Hallucination ? "normal" : "strange");
		for(mtmp = fmon; mtmp; mtmp = nextmon) {
		    nextmon = mtmp->nmon; /* trap might kill mon */
		    if (DEADMONSTER(mtmp)) continue;
		    if (mtmp->mtame) {
			if (mtmp->mtrapped) {
			    /* no longer in previous trap (affects mintrap) */
			    mtmp->mtrapped = 0;
			    fill_pit(mtmp->mx, mtmp->my);
			}
			mnexto(mtmp);
			if (canspotmon(mtmp)) ++pet_cnt;
			if (mintrap(mtmp) == 2) change_luck(-1);
		    }
		}
		if (pet_cnt > 0) makeknown(obj->otyp);
	}
}

boolean
um_dist(x,y,n)
register xchar x, y, n;
{
	return((boolean)(abs(u.ux - x) > n  || abs(u.uy - y) > n));
}

int
number_leashed()
{
	register int i = 0;
	register struct obj *obj;

	for(obj = invent; obj; obj = obj->nobj)
		if(obj->otyp == LEASH && obj->leashmon != 0) i++;
	return(i);
}

void
o_unleash(otmp)		/* otmp is about to be destroyed or stolen */
register struct obj *otmp;
{
	register struct monst *mtmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if(mtmp->m_id == (unsigned)otmp->leashmon)
			mtmp->mleashed = 0;
	otmp->leashmon = 0;
}

void
m_unleash(mtmp, feedback)	/* mtmp is about to die, or become untame */
register struct monst *mtmp;
boolean feedback;
{
	register struct obj *otmp;

	if (feedback) {
	    if (canseemon(mtmp))
		pline("%s pulls free of %s leash!", Monnam(mtmp), mhis(mtmp));
	    else
		Your("leash falls slack.");
	}
	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == LEASH &&
				otmp->leashmon == (int)mtmp->m_id)
			otmp->leashmon = 0;
	mtmp->mleashed = 0;
}

void
unleash_all()		/* player is about to die (for bones) */
{
	register struct obj *otmp;
	register struct monst *mtmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(otmp->otyp == LEASH) otmp->leashmon = 0;
	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		mtmp->mleashed = 0;
}

#define MAXLEASHED	2

/* ARGSUSED */
STATIC_OVL void
use_leash(obj)
struct obj *obj;
{
	coord cc;
	register struct monst *mtmp;
	int spotmon;

	if(!obj->leashmon && number_leashed() >= MAXLEASHED) {
		You("cannot leash any more pets.");
		return;
	}

	if(!get_adjacent_loc((char *)0, (char *)0, u.ux, u.uy, &cc)) return;

	if((cc.x == u.ux) && (cc.y == u.uy)) {
#ifdef STEED
		if (u.usteed && u.dz > 0) {
		    mtmp = u.usteed;
		    spotmon = 1;
		    goto got_target;
		}
#endif
		pline("Leash yourself?  Very funny...");
		return;
	}

	if(!(mtmp = m_at(cc.x, cc.y))) {
		There("is no creature there.");
		return;
	}

	spotmon = canspotmon(mtmp);
#ifdef STEED
 got_target:
#endif

	if(!mtmp->mtame) {
	    if(!spotmon)
		There("is no creature there.");
	    else
		pline("%s %s leashed!", Monnam(mtmp), (!obj->leashmon) ?
				"cannot be" : "is not");
	    return;
	}
	if(!obj->leashmon) {
		if(mtmp->mleashed) {
			pline("This %s is already leashed.",
			      spotmon ? l_monnam(mtmp) : "monster");
			return;
		}
		You("slip the leash around %s%s.",
		    spotmon ? "your " : "", l_monnam(mtmp));
		mtmp->mleashed = 1;
		obj->leashmon = (int)mtmp->m_id;
		mtmp->msleeping = 0;
		return;
	}
	if(obj->leashmon != (int)mtmp->m_id) {
		pline("This leash is not attached to that creature.");
		return;
	} else {
		if(obj->cursed) {
			pline_The("leash would not come off!");
			obj->bknown = TRUE;
			return;
		}
		mtmp->mleashed = 0;
		obj->leashmon = 0;
		You("remove the leash from %s%s.",
		    spotmon ? "your " : "", l_monnam(mtmp));
	}
	return;
}

struct obj *
get_mleash(mtmp)	/* assuming mtmp->mleashed has been checked */
register struct monst *mtmp;
{
	register struct obj *otmp;

	otmp = invent;
	while(otmp) {
		if(otmp->otyp == LEASH && otmp->leashmon == (int)mtmp->m_id)
			return(otmp);
		otmp = otmp->nobj;
	}
	return((struct obj *)0);
}

#endif /* OVLB */
#ifdef OVL1

boolean
next_to_u()
{
	register struct monst *mtmp;
	register struct obj *otmp;

	for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		if(mtmp->mleashed) {
			if (distu(mtmp->mx,mtmp->my) > 2) mnexto(mtmp);
			if (distu(mtmp->mx,mtmp->my) > 2) {
			    for(otmp = invent; otmp; otmp = otmp->nobj)
				if(otmp->otyp == LEASH &&
					otmp->leashmon == (int)mtmp->m_id) {
				    if(otmp->cursed) return(FALSE);
				    You_feel("%s leash go slack.",
					(number_leashed() > 1) ? "a" : "the");
				    mtmp->mleashed = 0;
				    otmp->leashmon = 0;
				}
			}
		}
	}
#ifdef STEED
	/* no pack mules for the Amulet */
	if (u.usteed && mon_has_amulet(u.usteed)) return FALSE;
#endif
	return(TRUE);
}

#endif /* OVL1 */
#ifdef OVL0

void
check_leash(x, y)
register xchar x, y;
{
	register struct obj *otmp;
	register struct monst *mtmp;

	for (otmp = invent; otmp; otmp = otmp->nobj) {
	    if (otmp->otyp != LEASH || otmp->leashmon == 0) continue;
	    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		if ((int)mtmp->m_id == otmp->leashmon) break; 
	    }
	    if (!mtmp) {
		impossible("leash in use isn't attached to anything?");
		otmp->leashmon = 0;
		continue;
	    }
	    if (dist2(u.ux,u.uy,mtmp->mx,mtmp->my) >
		    dist2(x,y,mtmp->mx,mtmp->my)) {
		if (!um_dist(mtmp->mx, mtmp->my, 3)) {
		    ;	/* still close enough */
		} else if (otmp->cursed && !breathless_mon(mtmp)) {
		    if (um_dist(mtmp->mx, mtmp->my, 5) ||
			    (mtmp->mhp -= rnd(2)) <= 0) {
			long save_pacifism = u.uconduct.killer;

			Your("leash chokes %s to death!", mon_nam(mtmp));
			/* hero might not have intended to kill pet, but
			   that's the result of his actions; gain experience,
			   lose pacifism, take alignment and luck hit, make
			   corpse less likely to remain tame after revival */
			xkilled(mtmp, 0);	/* no "you kill it" message */
			/* life-saving doesn't ordinarily reset this */
			if (mtmp->mhp > 0) u.uconduct.killer = save_pacifism;
		    } else {
			pline("%s chokes on the leash!", Monnam(mtmp));
			/* tameness eventually drops to 1 here (never 0) */
			if (mtmp->mtame && rn2(mtmp->mtame)) mtmp->mtame--;
			if(mtmp->data == &mons[PM_CROW] && u.sealsActive&SEAL_MALPHAS) unbind(SEAL_MALPHAS,TRUE);
		    }
		} else {
		    if (um_dist(mtmp->mx, mtmp->my, 5)) {
			pline("%s leash snaps loose!", s_suffix(Monnam(mtmp)));
			m_unleash(mtmp, FALSE);
		    } else {
			You("pull on the leash.");
			if (!is_silent_mon(mtmp))
			    switch (rn2(3)) {
			    case 0:  growl(mtmp);   break;
			    case 1:  yelp(mtmp);    break;
			    default: whimper(mtmp); break;
			    }
		    }
		}
	    }
	}
}

#endif /* OVL0 */
#ifdef OVLB

#define WEAK	3	/* from eat.c */

static const char look_str[] = "look %s.";

STATIC_OVL int
use_mirror(obj)
struct obj *obj;
{
	register struct monst *mtmp;
	register char mlet;
	boolean vis;

	if(!getdir((char *)0)) return 0;
	if(obj->cursed && !rn2(2)) {
		if (!Blind)
			pline_The("mirror fogs up and doesn't reflect!");
		return 1;
	}
	if(!u.dx && !u.dy && !u.dz) {
		if(obj->oartifact == ART_HAND_MIRROR_OF_CTHYLLA && obj->age < moves && !Blind){
			pline("An incomprehensible sight meets your eyes!");
			losehp(d(15,15), "looking into Cthylla's hand-mirror", KILLED_BY);
			obj->age = monstermoves + (long)(rnz(100)*(Role_if(PM_PRIEST) ? .8 : 1));
		} else if(!Blind && !Invisible) {
		    if (u.umonnum == PM_FLOATING_EYE && ward_at(u.ux, u.uy) != HAMSA) {
				if (!Free_action) {
					pline("%s", Hallucination ?
						  "Yow!  The mirror stares back!" :
						  "Yikes!  You've frozen yourself!");
					nomul(-rnd((MAXULEV+6) - u.ulevel), "frozen by your own reflection");
				} else You("stiffen momentarily under your gaze.");
		    } else if (youracedata->mlet == S_VAMPIRE)
				You("don't have a reflection.");
		    else if (u.umonnum == PM_UMBER_HULK && ward_at(u.ux, u.uy) != HAMSA) {
				pline("Huh?  That doesn't look like you!");
				make_confused(HConfusion + d(3,4),FALSE);
		    } else if (u.sealsActive&SEAL_IRIS){
				pline("What?  Who is that in the mirror!?");
				unbind(SEAL_IRIS,TRUE);
		    } else if (Hallucination)
				You(look_str, hcolor((char *)0));
		    else if (Sick)
				You(look_str, "peaked");
		    else if (u.uhs >= WEAK)
				You(look_str, "undernourished");
		    else You("look as %s as ever.",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "beautiful" : "handsome") :
				"ugly");
			signs_mirror();
		} else {
			You_cant("see your %s %s.",
				ACURR(A_CHA) > 14 ?
				(poly_gender()==1 ? "beautiful" : "handsome") :
				"ugly",
				body_part(FACE));
		}
		return 1;
	}
	if(u.uswallow) {
		if (!Blind) You("reflect %s %s.", s_suffix(mon_nam(u.ustuck)),
		    mbodypart(u.ustuck, STOMACH));
		return 1;
	}
	if(Underwater) {
		You(Hallucination ?
		    "give the fish a chance to fix their makeup." :
		    "reflect the murky water.");
		return 1;
	}
	if(u.dz) {
		if (!Blind)
		    You("reflect the %s.",
			(u.dz > 0) ? surface(u.ux,u.uy) : ceiling(u.ux,u.uy));
		return 1;
	}
	mtmp = bhit(u.dx, u.dy, COLNO, INVIS_BEAM,
		    (int FDECL((*),(MONST_P,OBJ_P)))0,
		    (int FDECL((*),(OBJ_P,OBJ_P)))0,
		    obj, NULL);
	if (!mtmp || !haseyes(mtmp->data))
		return 1;

	vis = canseemon(mtmp);
	mlet = mtmp->data->mlet;
	if (mtmp->msleeping) {
		if (vis)
		    pline ("%s is too tired to look at your mirror.",
			    Monnam(mtmp));
	} else if (is_blind(mtmp)) {
	    if (vis)
		pline("%s can't see anything right now.", Monnam(mtmp));
	/* some monsters do special things */
	} else if (mlet == S_VAMPIRE || mlet == S_GHOST || mlet == S_SHADE) {
	    if (vis)
		pline ("%s doesn't have a reflection.", Monnam(mtmp));
	} else if(obj->oartifact == ART_HAND_MIRROR_OF_CTHYLLA && obj->age < moves &&
				(!mtmp->minvis || mon_resistance(mtmp,SEE_INVIS))
	){
		obj->age = monstermoves + (long)(rnz(100)*(Role_if(PM_PRIEST) ? .8 : 1));
		if (vis)
		    pline("%s is blasted by its reflection.", Monnam(mtmp));
		monflee(mtmp, d(2,4), FALSE, FALSE);
		mtmp->mhp -= d(15,15);
		if (mtmp->mhp <= 0){
			mtmp->mhp = 0;
			xkilled(mtmp, 1);
		}
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_MEDUSA]  && 
					ward_at(mtmp->mx,mtmp->my) != HAMSA) {
		if (mon_reflects(mtmp, "The gaze is reflected away by %s %s!"))
			return 1;
		if(mtmp->mfaction != FRACTURED || !rn2(8)){
			if (vis)
				pline("%s is turned to stone!", Monnam(mtmp));
			stoned = TRUE;
			killed(mtmp);
		}
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_FLOATING_EYE] && 
					ward_at(mtmp->mx,mtmp->my) != HAMSA) {
		int tmp = d(min(MAX_BONUS_DICE, (int)mtmp->m_lev), (int)mtmp->data->mattk[0].damd);
		if (!rn2(4)) tmp = 120;
		if (vis)
			pline("%s is frozen by its reflection.", Monnam(mtmp));
		else You_hear("%s stop moving.",something);
		mtmp->mcanmove = 0;
		if ( (int) mtmp->mfrozen + tmp > 127)
			mtmp->mfrozen = 127;
		else mtmp->mfrozen += tmp;
	} else if(!mtmp->mcan && !mtmp->minvis &&
					mtmp->data == &mons[PM_UMBER_HULK] && 
					ward_at(mtmp->mx,mtmp->my) != HAMSA) {
		if (vis)
			pline ("%s confuses itself!", Monnam(mtmp));
		mtmp->mconf = 1;
	} else if(!mtmp->mcan && !mtmp->minvis && (mlet == S_NYMPH
				     || mtmp->data==&mons[PM_SUCCUBUS])) {
		if (vis) {
		    pline ("%s admires herself in your mirror.", Monnam(mtmp));
		    pline ("She takes it!");
		} else pline ("It steals your mirror!");
		setnotworn(obj); /* in case mirror was wielded */
		freeinv(obj);
		(void) mpickobj(mtmp,obj);
		if (!tele_restrict(mtmp)) (void) rloc(mtmp, FALSE);
	} else if(!mtmp->mcan && !mtmp->minvis && is_weeping(mtmp->data)) {
		if (vis)
			pline ("%s stares at its reflection with a stony expression.", Monnam(mtmp));
			mtmp->mcanmove = 0;
			mtmp->mfrozen = 1;
	} else if (!is_unicorn(mtmp->data) && is_animal(mtmp->data) &&
			(!mtmp->minvis || mon_resistance(mtmp,SEE_INVIS)) && rn2(5)) {
		if (vis)
		    pline("%s is frightened by its reflection.", Monnam(mtmp));
		monflee(mtmp, d(2,4), FALSE, FALSE);
	} else if (!Blind) {
		if (mtmp->minvis && !See_invisible(mtmp->mx, mtmp->my))
		    ;
		else if ((mtmp->minvis && !mon_resistance(mtmp,SEE_INVIS))
			 || !haseyes(mtmp->data))
		    pline("%s doesn't seem to notice its reflection.",
			Monnam(mtmp));
		else
		    pline("%s ignores %s reflection.",
			  Monnam(mtmp), mhis(mtmp));
	}
	return 1;
}

void
use_bell(optr, spiritseal)
struct obj **optr;
int spiritseal;
{
	register struct obj *obj = *optr;
	struct monst *mtmp;
	boolean wakem = FALSE, learno = FALSE,
		ordinary = (obj->otyp != BELL_OF_OPENING || (!obj->spe && !spiritseal)),
		invoking = (obj->otyp == BELL_OF_OPENING &&
			(spiritseal ?
			 ((invocation_pos(obj->ox, obj->oy) && !On_stairs(obj->ox, obj->oy)) || (invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy))) :
			 (invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy))
			));

	if(!spiritseal) You("ring %s.", the(xname(obj)));
	
	if(Role_if(PM_EXILE) && obj->otyp == BELL_OF_OPENING && !spiritseal){
		pline("It makes a rather sad clonk.");
		return;
	}
	
	if (Underwater || (u.uswallow && ordinary)) {
#ifdef	AMIGA
	    amii_speaker( obj, "AhDhGqEqDhEhAqDqFhGw", AMII_MUFFLED_VOLUME );
#endif
	    pline("But the sound is muffled.");

	} else if (invoking && ordinary) {
	    /* needs to be recharged... */
	    pline("But it makes no sound.");
	    learno = TRUE;	/* help player figure out why */

	} else if (ordinary) {
#ifdef	AMIGA
	    amii_speaker( obj, "ahdhgqeqdhehaqdqfhgw", AMII_MUFFLED_VOLUME );
#endif
	    if (obj->cursed && !rn2(4) &&
		    /* note: once any of them are gone, we stop all of them */
		    !(mvitals[PM_DRYAD].mvflags & G_GONE && !In_quest(&u.uz)) &&
		    !(mvitals[PM_NAIAD].mvflags & G_GONE && !In_quest(&u.uz)) &&
		    !(mvitals[PM_OREAD].mvflags & G_GONE && !In_quest(&u.uz)) &&
		    (mtmp = makemon(mkclass(S_NYMPH, Inhell ? G_HELL : G_NOHELL),
					u.ux, u.uy, NO_MINVENT)) != 0) {
		You("summon %s!", a_monnam(mtmp));
		if (!obj_resists(obj, 93, 100)) {
		    pline("%s shattered!", Tobjnam(obj, "have"));
		    useup(obj);
		    *optr = 0;
		} else switch (rn2(3)) {
			default:
				break;
			case 1:
				mon_adjust_speed(mtmp, 2, (struct obj *)0);
				break;
			case 2: /* no explanation; it just happens... */
				nomovemsg = "";
				nomul(-rnd(2), "ringing a bell");
				break;
		}
	    }
	    wakem = TRUE;

	} else {
	    /* charged Bell of Opening */
	    consume_obj_charge(obj, TRUE);
		
		if(uwep && uwep->oartifact == ART_SINGING_SWORD){
			uwep->ovar1 |= OHEARD_OPEN;
		}
		
	    if (u.uswallow) {
		if (!obj->cursed)
		    (void) openit();
		else
		    pline("%s", nothing_happens);

	    } else if (obj->cursed) {
		coord mm;

		mm.x = u.ux;
		mm.y = u.uy;
		pline("Graves open around you...");
		mkundead(&mm, FALSE, NO_MINVENT);
		wakem = TRUE;

	    } else  if (invoking) {
		pline("%s an unsettling shrill sound...",
		      Tobjnam(obj, "issue"));
#ifdef	AMIGA
		amii_speaker( obj, "aefeaefeaefeaefeaefe", AMII_LOUDER_VOLUME );
#endif
		if(spiritseal && !obj->cursed) u.rangBell = moves;
		obj->age = moves;
		learno = TRUE;
		wakem = TRUE;

	    } else if (obj->blessed) {
		int res = 0;

#ifdef	AMIGA
		amii_speaker( obj, "ahahahDhEhCw", AMII_SOFT_VOLUME );
#endif
		if (uchain) {
		    unpunish();
		    res = 1;
		}
		res += openit();
		switch (res) {
		  case 0:  pline("%s", nothing_happens); break;
		  case 1:  pline("%s opens...", Something);
			   learno = TRUE; break;
		  default: pline("Things open around you...");
			   learno = TRUE; break;
		}

	    } else {  /* uncursed */
#ifdef	AMIGA
		amii_speaker( obj, "AeFeaeFeAefegw", AMII_OKAY_VOLUME );
#endif
		if (findit() != 0) learno = TRUE;
		else pline("%s", nothing_happens);
	    }

	}	/* charged BofO */

	if (learno) {
	    makeknown(BELL_OF_OPENING);
	    obj->known = 1;
	}
	if (wakem) wake_nearby_noisy();
}

STATIC_OVL void
use_candelabrum(obj)
register struct obj *obj;
{
	const char *s = (obj->spe != 1) ? "candles" : "candle";

	if(Underwater) {
		You("cannot make fire under water.");
		return;
	}
	if(obj->lamplit) {
		You("snuff the %s.", s);
		end_burn(obj, TRUE);
		return;
	}
	if(obj->spe <= 0) {
		pline("This %s has no %s.", xname(obj), s);
		return;
	}
	if(u.uswallow || obj->cursed) {
		if (!Blind)
		    pline_The("%s %s for a moment, then %s.",
			      s, vtense(s, "flicker"), vtense(s, "die"));
		return;
	}
	if(obj->spe < 7) {
		There("%s only %d %s in %s.",
		      vtense(s, "are"), obj->spe, s, the(xname(obj)));
		if (!Blind)
		    pline("%s lit.  %s dimly.",
			  obj->spe == 1 ? "It is" : "They are",
			  Tobjnam(obj, "shine"));
	} else {
		pline("%s's %s burn%s", The(xname(obj)), s,
			(Blind ? "." : " brightly!"));
	}
	if (!invocation_pos(u.ux, u.uy)) {
		pline_The("%s %s being rapidly consumed!", s, vtense(s, "are"));
		obj->age /= 2;
	} else {
		if(obj->spe == 7) {
		    if (Blind)
		      pline("%s a strange warmth!", Tobjnam(obj, "radiate"));
		    else
		      pline("%s with a strange light!", Tobjnam(obj, "glow"));
		}
		obj->known = 1;
	}
	begin_burn(obj, FALSE);
}

STATIC_OVL void
use_candle(optr)
struct obj **optr;
{
	register struct obj *obj = *optr;
	register struct obj *otmp;
	const char *s = (obj->quan != 1) ? "candles" : "candle";
	char qbuf[QBUFSZ];

	if (u.uswallow){
		if(!is_whirly(u.ustuck->data)) {
			You(no_elbow_room);
			return;
		} else if(!obj->lamplit){
			You("can't get the %s to light.  It's quite hopeless under these conditions.", s);
			return;
		}
	}
	
	if(Underwater) {
		pline("Sorry, fire and water don't mix.");
		return;
	}

	otmp = carrying(CANDELABRUM_OF_INVOCATION);
	if(!otmp || obj->otyp==GNOMISH_POINTY_HAT || obj->otyp==CANDLE_OF_INVOCATION || otmp->spe == 7) {
		use_lamp(obj);
		return;
	}

	Sprintf(qbuf, "Attach %s", the(xname(obj)));
	Sprintf(eos(qbuf), " to %s?",
		safe_qbuf(qbuf, sizeof(" to ?"), the(xname(otmp)),
			the(simple_typename(otmp->otyp)), "it"));
	if(yn(qbuf) == 'n') {
		if (!obj->lamplit)
		    You("try to light %s...", the(xname(obj)));
		use_lamp(obj);
		return;
	} else {
		if ((long)otmp->spe + obj->quan > 7L)
		    obj = splitobj(obj, 7L - (long)otmp->spe);
		else *optr = 0;
		You("attach %ld%s %s to %s.",
		    obj->quan, !otmp->spe ? "" : " more",
		    s, the(xname(otmp)));
		if (!otmp->spe || otmp->age > obj->age)
		    otmp->age = obj->age;
		otmp->spe += (int)obj->quan;
		if (otmp->lamplit && !obj->lamplit)
		    pline_The("new %s magically %s!", s, vtense(s, "ignite"));
		else if (!otmp->lamplit && obj->lamplit)
		    pline("%s out.", (obj->quan > 1L) ? "They go" : "It goes");
		if (obj->unpaid)
		    verbalize("You %s %s, you bought %s!",
			      otmp->lamplit ? "burn" : "use",
			      (obj->quan > 1L) ? "them" : "it",
			      (obj->quan > 1L) ? "them" : "it");
		if (obj->quan < 7L && otmp->spe == 7)
		    pline("%s now has seven%s candles attached.",
			  The(xname(otmp)), otmp->lamplit ? " lit" : "");
		/* candelabrum's light range might increase */
		if (otmp->lamplit) obj_merge_light_sources(otmp, otmp);
		/* candles are no longer a separate light source */
		if (obj->lamplit) end_burn(obj, TRUE);
		/* candles are now gone */
		useupall(obj);
	}
}

boolean
snuff_candle(otmp)  /* call in drop, throw, and put in box, etc. */
register struct obj *otmp;
{
	register boolean candle = Is_candle(otmp);

	if ((candle || otmp->otyp == CANDELABRUM_OF_INVOCATION) &&
		otmp->lamplit) {
	    char buf[BUFSZ];
	    xchar x, y;
	    register boolean many = candle ? otmp->quan > 1L : otmp->spe > 1;

	    (void) get_obj_location(otmp, &x, &y, 0);
	    if (otmp->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
		pline("%s %scandle%s flame%s extinguished.",
		      Shk_Your(buf, otmp),
		      (candle ? "" : "candelabrum's "),
		      (many ? "s'" : "'s"), (many ? "s are" : " is"));
	   end_burn(otmp, TRUE);
	   return(TRUE);
	}
	return(FALSE);
}

/* called when lit lamp is hit by water or put into a container or
   you've been swallowed by a monster; obj might be in transit while
   being thrown or dropped so don't assume that its location is valid */
boolean
snuff_lit(obj)
struct obj *obj;
{
	xchar x, y;

	if (obj->lamplit) {
	    if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		    obj->otyp == LANTERN || obj->otyp == POT_OIL ||
			obj->otyp == DWARVISH_HELM || obj->otyp == GNOMISH_POINTY_HAT) {
		(void) get_obj_location(obj, &x, &y, 0);
		if (obj->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
		    pline("%s %s out!", Yname2(obj), otense(obj, "go"));
		end_burn(obj, TRUE);
		return TRUE;
	    }
	    if (snuff_candle(obj)) return TRUE;
	}
	return FALSE;
}

/* Called when potentially lightable object is affected by fire_damage().
	Return TRUE if object was lit and FALSE otherwise --ALI */
boolean
catch_lit(obj)
struct obj *obj;
{
	xchar x, y;

	if (!obj->lamplit && (obj->otyp == MAGIC_LAMP || ignitable(obj))) {
	    if ((obj->otyp == MAGIC_LAMP ||
		 obj->otyp == CANDELABRUM_OF_INVOCATION) &&
		obj->spe == 0)
		return FALSE;
	    else if (obj->otyp != MAGIC_LAMP && obj->age == 0)
		return FALSE;
	    if (!get_obj_location(obj, &x, &y, 0))
		return FALSE;
	    if (obj->otyp == CANDELABRUM_OF_INVOCATION && obj->cursed)
		return FALSE;
	    if ((obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		 obj->otyp == LANTERN || obj->otyp == DWARVISH_HELM ||
		 obj->otyp == GNOMISH_POINTY_HAT) && 
			obj->cursed && !rn2(2))
		return FALSE;
	    if (obj->where == OBJ_MINVENT ? cansee(x,y) : !Blind)
		pline("%s %s light!", Yname2(obj), otense(obj, "catch"));
	    if (obj->otyp == POT_OIL) makeknown(obj->otyp);
	    if (obj->unpaid && costly_spot(u.ux, u.uy) && (obj->where == OBJ_INVENT)) {
	        /* if it catches while you have it, then it's your tough luck */
		check_unpaid(obj);
	        verbalize("That's in addition to the cost of %s %s, of course.",
				Yname2(obj), obj->quan == 1 ? "itself" : "themselves");
		bill_dummy_object(obj);
	    }
	    begin_burn(obj, FALSE);
	    return TRUE;
	}
	return FALSE;
}

STATIC_OVL int
swap_aegis(obj)
struct obj *obj;
{
	if(obj->owornmask){
		You("must take %s off to modify it.", the(xname(obj)));
		return 0;
	} else if(obj->otyp == CLOAK){
		You("wrap %s up, making a serviceable shield.", the(xname(obj)));
		obj->otyp = ROUNDSHIELD;
		return 1;
	} else if(obj->otyp == ROUNDSHIELD){
		You("unwrap %s, making a cloak.", the(xname(obj)));
		obj->otyp = CLOAK;
		return 1;
	} else {
		pline("Aegis in unexpected state?");
		return 0;
	}
}

STATIC_OVL int
use_rakuyo(obj)
struct obj *obj;
{
	struct obj *dagger;
	if(obj != uwep){
		if(obj->otyp == RAKUYO) You("must wield %s to unlatch it.", the(xname(obj)));
		else You("must wield %s to latch it.", the(xname(obj)));
		return 0;
	}
	
	if(obj->unpaid 
	|| (obj->otyp == RAKUYO_SABER && uswapwep && uswapwep->otyp == RAKUYO_DAGGER && uswapwep->unpaid)
	){
		You("need to buy it.");
		return 0;
	}
	
	if(obj->otyp == RAKUYO){
		You("unlatch %s.",the(xname(obj)));
		obj->otyp = RAKUYO_SABER;
		fix_object(obj);
		obj->quan += 1;
	    dagger = splitobj(obj, 1L);
		obj_extract_self(dagger);
		dagger->otyp = RAKUYO_DAGGER;
		fix_object(obj);
		dagger = hold_another_object(dagger, "You drop %s!",
				      doname(obj), (const char *)0); /*shouldn't merge, but may drop*/
		if(dagger && !uswapwep && carried(dagger)){
			setuswapwep(dagger);
			dotwoweapon();
		}
	} else {
		if(!uswapwep || uswapwep->otyp != RAKUYO_DAGGER){
			You("need the matching dagger.");
			return 0;
		}
		if(!mergable_traits(obj, uswapwep)){
			pline("They don't fit together!");
			return 0;
		}
		if (u.twoweap) {
			u.twoweap = 0;
			update_inventory();
		}
		useupall(uswapwep);
		obj->otyp = RAKUYO;
		fix_object(obj);
		You("latch %s.",the(xname(obj)));
	}
	return 0;
}

STATIC_OVL int
use_force_blade(obj)
struct obj *obj;
{
	struct obj *dagger;
	if(obj != uwep){
		if(obj->otyp == DOUBLE_FORCE_BLADE) You("must wield %s to unlatch it.", the(xname(obj)));
		else You("must wield %s to latch it.", the(xname(obj)));
		return 0;
	}
	
	if(obj->unpaid 
	|| (obj->otyp == FORCE_BLADE && uswapwep && uswapwep->otyp == FORCE_BLADE && uswapwep->unpaid)
	){
		You("need to buy it.");
		return 0;
	}
	
	if(obj->otyp == DOUBLE_FORCE_BLADE){
		You("unlatch %s.",the(xname(obj)));
		obj->otyp = FORCE_BLADE;
		fix_object(obj);
		obj->quan += 1;
	    dagger = splitobj(obj, 1L);
		obj_extract_self(dagger);
		fix_object(obj);
		dagger = hold_another_object(dagger, "You drop %s!",
				      doname(obj), (const char *)0); /*shouldn't merge, but may drop*/
		if(dagger && !uswapwep && carried(dagger)){
			setuswapwep(dagger);
			dotwoweapon();
		}
	} else {
		if(!uswapwep || uswapwep->otyp != FORCE_BLADE){
			You("need the matching blade.");
			return 0;
		}
		if(!mergable_traits(obj, uswapwep)){
			pline("They don't fit together!");
			return 0;
		}
		if (u.twoweap) {
			u.twoweap = 0;
			update_inventory();
		}
		obj->ovar1 = (obj->ovar1 + uswapwep->ovar1)/2;
		useupall(uswapwep);
		obj->otyp = DOUBLE_FORCE_BLADE;
		fix_object(obj);
		You("latch %s.",the(xname(obj)));
		update_inventory();
	}
	return 0;
}

int
use_force_sword(obj)
struct obj *obj;
{
	if(obj->unpaid){
		You("need to buy it.");
		return 0;
	}
	
	if(obj->otyp == FORCE_SWORD){
		You("unlock %s.",the(xname(obj)));
		obj->otyp = FORCE_WHIP;
	} else {
		You("lock %s.",the(xname(obj)));
		obj->otyp = FORCE_SWORD;
	}
	fix_object(obj);
	update_inventory();
	return 0;
}

STATIC_OVL void
use_lamp(obj)
struct obj *obj;
{
	char buf[BUFSZ];

	if(obj->oartifact == ART_INFINITY_S_MIRRORED_ARC){
		You("can't find an %s switch", litsaber(obj) ? "off" : "on");
		return;
	}
	if(Underwater && obj->oartifact != ART_HOLY_MOONLIGHT_SWORD) {
		pline("This is not a diving lamp.");
		return;
	}
	if(obj->lamplit) {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		   obj->otyp == LANTERN ||
		   obj->otyp == DWARVISH_HELM)
		    pline("%s lamp is now off.", Shk_Your(buf, obj));
		else if(is_lightsaber(obj)) {
		    if (obj->otyp == DOUBLE_LIGHTSABER || obj->oartifact == ART_ANNULUS) {
			/* Do we want to activate dual bladed mode? */
			if (!obj->altmode && (!obj->cursed || rn2(4))) {
			    You("ignite the second blade of %s.", yname(obj));
			    obj->altmode = TRUE;
			    return;
			} else obj->altmode = FALSE;
		    }
		    lightsaber_deactivate(obj, TRUE);
		    return;
		} else
		    You("snuff out %s.", yname(obj));
		end_burn(obj, TRUE);
		return;
	}
	/* magic lamps with an spe == 0 (wished for) cannot be lit */
	if ((!Is_candle(obj) && obj->age == 0 && obj->oartifact != ART_HOLY_MOONLIGHT_SWORD &&
			!(is_lightsaber(obj) && obj->oartifact == ART_ATMA_WEAPON && !Drain_resistance))
			|| (obj->otyp == MAGIC_LAMP && obj->spe == 0)
		) {
		if (obj->otyp == LANTERN || 
			obj->otyp == DWARVISH_HELM || 
			is_lightsaber(obj)
		)
			Your("%s has run out of power.", xname(obj));
		else pline("This %s has no %s.", xname(obj), obj->otyp != GNOMISH_POINTY_HAT ? "oil" : "wax");
		return;
	}
	if(is_lightsaber(obj) && 
		obj->cursed && 
		obj->oartifact == ART_ATMA_WEAPON
	){
		if (!Drain_resistance) {
			pline("%s for a moment, then %s brightly.",
		      Tobjnam(obj, "flicker"), otense(obj, "burn"));
			losexp("life force drain",TRUE,TRUE,TRUE);
			obj->cursed = FALSE;
		}
	}
	if (obj->cursed && (!rn2(2) || obj->otyp == CANDLE_OF_INVOCATION) && obj->oartifact != ART_HOLY_MOONLIGHT_SWORD) {
		pline("%s for a moment, then %s.",
		      Tobjnam(obj, "flicker"), otense(obj, "die"));
	} else {
		if(obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
				obj->otyp == LANTERN || obj->otyp == DWARVISH_HELM) {
		    check_unpaid(obj);
		    pline("%s lamp is now on.", Shk_Your(buf, obj));
		} else if (is_lightsaber(obj)) {
			obj->lamplit = 1; //Make yname print out the color of the lightsaber
		    You("ignite %s.", yname(obj));
			obj->lamplit = 0;
		    unweapon = FALSE;
		} else if (obj->oartifact == ART_HOLY_MOONLIGHT_SWORD) {
			int biman;
			obj->lamplit = 1; //Check if the HMS will be two handed
			biman = bimanual(obj,youracedata);
			obj->lamplit = 0;
			if(biman && uarms){
				You_cant("invoke %s while wearing a shield!", yname(obj));
				return;
			}
		    You("invoke %s.", yname(obj));
			if(biman && u.twoweap){
				You("must now hold %s with both hands!", yname(obj)); 
				untwoweapon();
			}
		    unweapon = FALSE;
		} else {	/* candle(s) */
		    pline("%s flame%s %s%s",
			s_suffix(Yname2(obj)),
			plur(obj->quan), otense(obj, "burn"),
			Blind ? "." : " brightly!");
		    if (obj->unpaid && costly_spot(u.ux, u.uy) &&
			  obj->age == 20L * (long)objects[obj->otyp].oc_cost) {
			const char *ithem = obj->quan > 1L ? "them" : "it";
			verbalize("You burn %s, you bought %s!", ithem, ithem);
			bill_dummy_object(obj);
		    }
		}
		begin_burn(obj, FALSE);
	}
}

STATIC_OVL void
light_cocktail(obj)
	struct obj *obj;	/* obj is a potion of oil */
{
	char buf[BUFSZ];
	const char *objnam =
//#ifdef FIREARMS
	    obj->otyp == POT_OIL ? "potion" : "stick";
//#else
//	    "potion";
//#endif

	if (u.uswallow){
		if(!is_whirly(u.ustuck->data)) {
			You(no_elbow_room);
			return;
		} else if(!obj->lamplit){
			You("can't get the %s to light.  It's quite hopeless under these conditions.", objnam);
			return;
		}
	}
	
	if(Underwater) {
		You("can't light this underwater!");
		return;
	}

	if (obj->lamplit) {
	    You("snuff the lit %s.", objnam);
	    end_burn(obj, TRUE);
	    /*
	     * Free & add to re-merge potion.  This will average the
	     * age of the potions.  Not exactly the best solution,
	     * but its easy.
	     */
	    freeinv(obj);
	    (void) addinv(obj);
	    return;
	} else if (Underwater) {
	    There("is not enough oxygen to sustain a fire.");
	    return;
	}

	You("light %s %s.%s", shk_your(buf, obj), objnam,
	    Blind ? "" : "  It gives off a dim light.");
	if (obj->unpaid && costly_spot(u.ux, u.uy)) {
	    /* Normally, we shouldn't both partially and fully charge
	     * for an item, but (Yendorian Fuel) Taxes are inevitable...
	     */
//#ifdef FIREARMS
	    if (obj->otyp != STICK_OF_DYNAMITE) {
//#endif
	    check_unpaid(obj);
	    verbalize("That's in addition to the cost of the potion, of course.");
//#ifdef FIREARMS
	    } else {
		const char *ithem = obj->quan > 1L ? "them" : "it";
		verbalize("You burn %s, you bought %s!", ithem, ithem);
	    }
//#endif
	    bill_dummy_object(obj);
	}
	makeknown(obj->otyp);
//#ifdef FIREARMS
	if (obj->otyp == STICK_OF_DYNAMITE) obj->yours=TRUE;
//#endif

	if (obj->quan > 1L) {
	    obj = splitobj(obj, 1L);
	    begin_burn(obj, FALSE);	/* burn before free to get position */
	    obj_extract_self(obj);	/* free from inv */

	    /* shouldn't merge */
	    obj = hold_another_object(obj, "You drop %s!",
				      doname(obj), (const char *)0);
	} else
	    begin_burn(obj, FALSE);
}

STATIC_OVL void
light_torch(obj)
	struct obj *obj;	/* obj is a shadowlander's torch */
{
	char buf[BUFSZ];
	const char *objnam = "torch";

	if (u.uswallow){
		if(!is_whirly(u.ustuck->data)) {
			You(no_elbow_room);
			return;
		} else if(!obj->lamplit && obj->otyp != SUNROD && rn2(4)){
			You("can't quite get the %s to light!", objnam);
			return;
		}
	}
	
	if(obj->lamplit && obj->otyp == SUNROD){
		You("can't snuff the lit %s!", objnam);
		return;
	}
	
	if(Underwater) {
		You("can't light this underwater!");
		return;
	}

	if (obj->lamplit) {
	    You("snuff the lit %s.", objnam);
	    end_burn(obj, TRUE);
	    /*
	     * Free & add to re-merge potion.  This will average the
	     * age of the potions.  Not exactly the best solution,
	     * but its easy.
	     */
	    freeinv(obj);
	    (void) addinv(obj);
	    return;
	} else if (Underwater) {
	    There("is not enough oxygen to sustain a fire.");
	    return;
	}
	if(obj->otyp == SHADOWLANDER_S_TORCH){
		You("light %s %s.%s", shk_your(buf, obj), objnam,
			Blind ? "" : "  It gives off dark shadows.");
	} else if(obj->otyp == SUNROD){
		You("light %s %s.%s", shk_your(buf, obj), objnam,
			Blind ? "" : "  It gives off brilliant light.");
	} else {
		You("light %s %s.%s", shk_your(buf, obj), objnam,
			Blind ? "" : "  It gives off bright flickering light.");
	}
	if (obj->unpaid && costly_spot(u.ux, u.uy)) {
		verbalize("You burn it, you bought it!");
	    bill_dummy_object(obj);
	}

	if (obj->quan > 1L) {
	    obj = splitobj(obj, 1L);
	    begin_burn(obj, FALSE);	/* burn before free to get position */
	    obj_extract_self(obj);	/* free from inv */

	    /* shouldn't merge */
	    obj = hold_another_object(obj, "You drop %s!",
				      doname(obj), (const char *)0);
	} else
	    begin_burn(obj, FALSE);
}

static NEARDATA const char cuddly[] = { TOOL_CLASS, GEM_CLASS, 0 };

int
dorub()
{
	struct obj *obj = getobj(cuddly, "rub");

	if (obj && obj->oclass == GEM_CLASS) {
	    if (is_graystone(obj)) {
		use_stone(obj);
		return 1;
	    } else {
		pline("Sorry, I don't know how to use that.");
		return 0;
	    }
	}

	if (!obj || !wield_tool(obj, "rub")) return 0;

	/* now uwep is obj */
	if (uwep->otyp == MAGIC_LAMP) {
	    if (uwep->spe > 0 && !rn2(3)) {
		check_unpaid_usage(uwep, TRUE);		/* unusual item use */
		djinni_from_bottle(uwep);
		makeknown(MAGIC_LAMP);
		uwep->otyp = OIL_LAMP;
		uwep->spe = 0; /* for safety */
		uwep->age = rn1(500,1000);
		if (uwep->lamplit) begin_burn(uwep, TRUE);
		update_inventory();
	    } else if (rn2(2) && !Blind)
		You("see a puff of smoke.");
	    else pline1(nothing_happens);
	} else if (obj->otyp == LANTERN || obj->otyp == DWARVISH_HELM) {
	    /* message from Adventure */
	    pline("Rubbing the electric lamp is not particularly rewarding.");
	    pline("Anyway, nothing exciting happens.");
	} else pline1(nothing_happens);
	return 1;
}

int
dojump()
{
	/* Physical jump */
	return jump(0);
}

int
jump(magic)
int magic; /* 0=Physical, otherwise skill level */
{
	coord cc;

	if (!magic && (nolimbs(youracedata) || slithy(youracedata))) {
		/* normally (nolimbs || slithy) implies !Jumping,
		   but that isn't necessarily the case for knights */
		You_cant("jump; you have no legs!");
		return 0;
	} else if (!magic && !Jumping) {
		You_cant("jump very far.");
		return 0;
	} else if (u.uswallow) {
		if (magic) {
			You("bounce around a little.");
			return 1;
		} else {
			pline("You've got to be kidding!");
		return 0;
		}
		return 0;
	} else if (u.uinwater) {
		if (magic) {
			You("swish around a little.");
			return 1;
		} else {
			pline("This calls for swimming, not jumping!");
		return 0;
		}
		return 0;
	} else if (u.ustuck) {
		if (u.ustuck->mtame && !Conflict && !u.ustuck->mconf) {
		    You("pull free from %s.", mon_nam(u.ustuck));
		    u.ustuck = 0;
		    return 1;
		}
		if (magic) {
			You("writhe a little in the grasp of %s!", mon_nam(u.ustuck));
			return 1;
		} else {
			You("cannot escape from %s!", mon_nam(u.ustuck));
			return 0;
		}

		return 0;
	} else if (Levitation || Weightless || Is_waterlevel(&u.uz)) {
		if (magic) {
			You("flail around a little.");
			return 1;
		} else {
			You("don't have enough traction to jump.");
			return 0;
		}
	} else if (!magic && near_capacity() > UNENCUMBERED) {
		You("are carrying too much to jump!");
		return 0;
	} else if (!magic && (YouHunger <= 100 || ACURR(A_STR) < 6)) {
		You("lack the strength to jump!");
		return 0;
	} else if (Wounded_legs) {
		long wl = (Wounded_legs & BOTH_SIDES);
		const char *bp = body_part(LEG);

		if (wl == BOTH_SIDES) bp = makeplural(bp);
#ifdef STEED
		if (u.usteed)
		    pline("%s is in no shape for jumping.", Monnam(u.usteed));
		else
#endif
		Your("%s%s %s in no shape for jumping.",
		     (wl == LEFT_SIDE) ? "left " :
			(wl == RIGHT_SIDE) ? "right " : "",
		     bp, (wl == BOTH_SIDES) ? "are" : "is");
		return 0;
	}
#ifdef STEED
	else if (u.usteed && u.utrap) {
		pline("%s is stuck in a trap.", Monnam(u.usteed));
		return (0);
	}
#endif

	pline("Where do you want to jump?");
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "the desired position") < 0)
		return 0;	/* user pressed ESC */
	if (!magic && !(HJumping & ~INTRINSIC) && !EJumping && !(u.sealsActive&SEAL_OSE) &&
			distu(cc.x, cc.y) != 5) {
		/* The Knight jumping restriction still applies when riding a
		 * horse.  After all, what shape is the knight piece in chess?
		 */
		pline("Illegal move!");
		return 0;
	} else if (distu(cc.x, cc.y) > (magic ? 6+magic*3 : 9)) {
		pline("Too far!");
		return 0;
	} else if (!cansee(cc.x, cc.y)) {
		You("cannot see where to land!");
		return 0;
	} else if (!isok(cc.x, cc.y)) {
		You("cannot jump there!");
		return 0;
	} else {
	    coord uc;
	    int range, temp;

	    if(u.utrap)
		switch(u.utraptype) {
		case TT_BEARTRAP: {
		    long side = rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
			static int jboots4 = 0;
			if (!jboots4) jboots4 = find_jboots();
		    You("rip yourself free of the bear trap!  Ouch!");
			if (uarmf && uarmf->otyp == jboots4){
				int bootdamage = d(1,10);
				losehp(rnd(10), "jumping out of a bear trap", KILLED_BY);
				set_wounded_legs(side, rn1(100,50));
				if(bootdamage > uarmf->spe){
					claws_destroy_arm(uarmf);
				}else{
					for(; bootdamage >= 0; bootdamage--) drain_item(uarmf);
					Your("boots are damaged!");
				}
			}
		    else{
				losehp(d(5,6), "jumping out of a bear trap", KILLED_BY);
				set_wounded_legs(side, rn1(1000,500));
			}
		    break;
		  }
		case TT_PIT:
		    You("leap from the pit!");
		    break;
		case TT_WEB:
			if(Is_lolth_level(&u.uz)){
				You("cannot free yourself from the web!");
				return 0;
			} else {
				You("tear the web apart as you pull yourself free!");
				deltrap(t_at(u.ux,u.uy));
			}
		    break;
		case TT_LAVA:
		    You("pull yourself above the lava!");
		    u.utrap = 0;
		    return 1;
		case TT_INFLOOR:
		    You("strain your %s, but you're still stuck in the floor.",
			makeplural(body_part(LEG)));
		    set_wounded_legs(LEFT_SIDE, rn1(10, 11));
		    set_wounded_legs(RIGHT_SIDE, rn1(10, 11));
		    return 1;
		}

	    /*
	     * Check the path from uc to cc, calling hurtle_step at each
	     * location.  The final position actually reached will be
	     * in cc.
	     */
	    uc.x = u.ux;
	    uc.y = u.uy;
	    /* calculate max(abs(dx), abs(dy)) as the range */
	    range = cc.x - uc.x;
	    if (range < 0) range = -range;
	    temp = cc.y - uc.y;
	    if (temp < 0) temp = -temp;
	    if (range < temp)
		range = temp;
	    (void) walk_path(&uc, &cc, hurtle_step, (genericptr_t)&range);

	    /* A little Sokoban guilt... */
	    if (In_sokoban(&u.uz))
		change_luck(-1);

	    teleds(cc.x, cc.y, TRUE);
	    nomul(-1, "jumping around");
	    nomovemsg = "";
	    morehungry(rnd(25));
	    return 1;
	}
}

boolean
tinnable(corpse)
struct obj *corpse;
{
	if (corpse->otyp != CORPSE) return 0;
	if (corpse->oeaten) return 0;
	if (corpse->odrained) return 0;
	if (!mons[corpse->corpsenm].cnutrit) return 0;
	return 1;
}

STATIC_OVL void
use_tinning_kit(obj)
register struct obj *obj;
{
	register struct obj *corpse, *can=0, *bld=0;

	/* This takes only 1 move.  If this is to be changed to take many
	 * moves, we've got to deal with decaying corpses...
	 */
	if (obj->spe <= 0) {
		You("seem to be out of tins.");
		return;
	}
	if (!(corpse = floorfood("tin", 2))) return;
	if (corpse->otyp == CORPSE && (corpse->oeaten || corpse->odrained)) {
		You("cannot tin %s which is partly eaten.",something);
		return;
	}
	if (!tinnable(corpse)) {
		You_cant("tin that!");
		return;
	}
	if (touch_petrifies(&mons[corpse->corpsenm])
		&& !Stone_resistance && !uarmg) {
	    char kbuf[BUFSZ];

	    if (poly_when_stoned(youracedata))
		You("tin %s without wearing gloves.",
			an(mons[corpse->corpsenm].mname));
	    else {
		pline("Tinning %s without wearing gloves is a fatal mistake...",
			an(mons[corpse->corpsenm].mname));
		Sprintf(kbuf, "trying to tin %s without gloves",
			an(mons[corpse->corpsenm].mname));
	    }
	    instapetrify(kbuf);
	}
	if (is_rider(&mons[corpse->corpsenm])) {
		if(Is_astralevel(&u.uz)) verbalize("Yes...  But War does not preserve its enemies...");
		(void) revive_corpse(corpse, REVIVE_MONSTER);
		return;
	}
	if (mons[corpse->corpsenm].cnutrit == 0) {
		pline("That's too insubstantial to tin.");
		return;
	}
	consume_obj_charge(obj, TRUE);
	if(has_blood(&mons[corpse->corpsenm])
		|| !(Race_if(PM_VAMPIRE) || Race_if(PM_INCANTIFIER) || 
			umechanoid)
		|| yn("This corpse does not have blood. Tin it?") == 'y'
	){
		if ((can = mksobj(TIN, FALSE, FALSE)) != 0) {
			static const char you_buy_it[] = "You tin it, you bought it!";

			can->corpsenm = corpse->corpsenm;
			can->cursed = obj->cursed;
			can->blessed = obj->blessed;
			can->owt = weight(can);
			can->known = 1;
			can->spe = -1;  /* Mark tinned tins. No spinach allowed... */
			if(has_blood(&mons[corpse->corpsenm])){
				if ((bld = mksobj(POT_BLOOD, FALSE, FALSE)) != 0) {
					bld->corpsenm = corpse->corpsenm;
					bld->cursed = obj->cursed;
					bld->blessed = obj->blessed;
					bld->known = 1;
				}
			}
			if (carried(corpse)) {
			if (corpse->unpaid)
				verbalize(you_buy_it);
			useup(corpse);
			} else {
			if (costly_spot(corpse->ox, corpse->oy) && !corpse->no_charge)
				verbalize(you_buy_it);
			useupf(corpse, 1L);
			}
			can = hold_another_object(can, "You make, but cannot pick up, %s.",
						  doname(can), (const char *)0);
			if(bld) bld = hold_another_object(bld, "You make, but cannot pick up, %s.",
						  doname(bld), (const char *)0);
		} else impossible("Tinning failed.");
	}
}

void
use_unicorn_horn(obj)
struct obj *obj;
{
#define PROP_COUNT 6		/* number of properties we're dealing with */
#define ATTR_COUNT (A_MAX*3)	/* number of attribute points we might fix */
	int idx, val, val_limit,
	    trouble_count, unfixable_trbl, did_prop, did_attr;
	int trouble_list[PROP_COUNT + ATTR_COUNT];

	if (obj && obj->cursed) {
	    long lcount = (long) rnd(100);

	    switch (rn2(5)) {
	    case 0: make_sick(Sick ? Sick/3L + 1L : (long)rn1(ACURR(A_CON),20),
			xname(obj), TRUE, SICK_NONVOMITABLE);
		    break;
	    case 1: make_blinded(Blinded + lcount, TRUE);
		    break;
	    case 2: if (!Confusion)
			You("suddenly feel %s.",
			    Hallucination ? "trippy" : "confused");
		    make_confused(HConfusion + lcount, TRUE);
		    break;
	    case 3: make_stunned(HStun + lcount, TRUE);
		    break;
	    // case 4: (void) adjattrib(rn2(A_MAX), -1, FALSE);
		    // break;
	    case 4: (void) make_hallucinated(HHallucination + lcount, TRUE, 0L);
		    break;
	    }
	    return;
	}

/*
 * Entries in the trouble list use a very simple encoding scheme.
 */
#define prop2trbl(X)	((X) + A_MAX)
#define attr2trbl(Y)	(Y)
#define prop_trouble(X) trouble_list[trouble_count++] = prop2trbl(X)
#define attr_trouble(Y) trouble_list[trouble_count++] = attr2trbl(Y)

	trouble_count = unfixable_trbl = did_prop = did_attr = 0;

	/* collect property troubles */
	if (Sick) prop_trouble(SICK);
	if (Blinded > (long)u.ucreamed) prop_trouble(BLINDED);
	if (HHallucination) prop_trouble(HALLUC);
	if (Vomiting) prop_trouble(VOMITING);
	if (HConfusion) prop_trouble(CONFUSION);
	if (HStun) prop_trouble(STUNNED);

	unfixable_trbl = unfixable_trouble_count(TRUE);

	// /* collect attribute troubles */
	// for (idx = 0; idx < A_MAX; idx++) {
	    // val_limit = AMAX(idx);
	    // /* don't recover strength lost from hunger */
	    // if (idx == A_STR && u.uhs >= WEAK) val_limit--;
	    // /* don't recover more than 3 points worth of any attribute */
	    // if (val_limit > ABASE(idx) + 3) val_limit = ABASE(idx) + 3;

	    // for (val = ABASE(idx); val < val_limit; val++)
		// attr_trouble(idx);
	    // /* keep track of unfixed trouble, for message adjustment below */
	    // unfixable_trbl += (AMAX(idx) - val_limit);
	// }

	if (trouble_count == 0) {
	    pline1(nothing_happens);
	    return;
	} else if (trouble_count > 1) {		/* shuffle */
	    int i, j, k;

	    for (i = trouble_count - 1; i > 0; i--)
		if ((j = rn2(i + 1)) != i) {
		    k = trouble_list[j];
		    trouble_list[j] = trouble_list[i];
		    trouble_list[i] = k;
		}
	}

	/*
	 *		Chances for number of troubles to be fixed
	 *		 0	1      2      3      4	    5	   6	  7
	 *   blessed:  22.7%  22.7%  19.5%  15.4%  10.7%   5.7%   2.6%	 0.8%
	 *  uncursed:  35.4%  35.4%  22.9%   6.3%    0	    0	   0	  0
	 */
	val_limit = rn2( d(2, (obj && obj->blessed) ? 4 : 2) );
	if (val_limit > trouble_count) val_limit = trouble_count;

	/* fix [some of] the troubles */
	for (val = 0; val < val_limit; val++) {
	    idx = trouble_list[val];

	    switch (idx) {
	    case prop2trbl(SICK):
		make_sick(0L, (char *) 0, TRUE, SICK_ALL);
		did_prop++;
		break;
	    case prop2trbl(BLINDED):
		make_blinded((long)u.ucreamed, TRUE);
		did_prop++;
		break;
	    case prop2trbl(HALLUC):
		(void) make_hallucinated(0L, TRUE, 0L);
		did_prop++;
		break;
	    case prop2trbl(VOMITING):
		make_vomiting(0L, TRUE);
		did_prop++;
		break;
	    case prop2trbl(CONFUSION):
		make_confused(0L, TRUE);
		did_prop++;
		break;
	    case prop2trbl(STUNNED):
		make_stunned(0L, TRUE);
		did_prop++;
		break;
	    default:
		if (idx >= 0 && idx < A_MAX) {
		    ABASE(idx) += 1;
		    did_attr++;
		} else
		    panic("use_unicorn_horn: bad trouble? (%d)", idx);
		break;
	    }
	}

	if (did_attr)
	    pline("This makes you feel %s!",
		  (did_prop + did_attr) == (trouble_count + unfixable_trbl) ?
		  "great" : "better");
	else if (!did_prop)
	    pline("Nothing seems to happen.");

	flags.botl = (did_attr || did_prop);
#undef PROP_COUNT
#undef ATTR_COUNT
#undef prop2trbl
#undef attr2trbl
#undef prop_trouble
#undef attr_trouble
}

/*
 * Timer callback routine: turn figurine into monster
 */
void
fig_transform(arg, timeout)
genericptr_t arg;
long timeout;
{
	struct obj *figurine = (struct obj *)arg;
	struct monst *mtmp;
	coord cc;
	boolean cansee_spot, silent, okay_spot;
	boolean redraw = FALSE;
	char monnambuf[BUFSZ], carriedby[BUFSZ];

	if (!figurine) {
#ifdef DEBUG
	    pline("null figurine in fig_transform()");
#endif
	    return;
	}
	silent = (timeout != monstermoves); /* happened while away */
	okay_spot = get_obj_location(figurine, &cc.x, &cc.y, 0);
	if (figurine->where == OBJ_INVENT ||
	    figurine->where == OBJ_MINVENT)
		okay_spot = enexto(&cc, cc.x, cc.y,
				   &mons[figurine->corpsenm]);
	if (!okay_spot ||
	    !figurine_location_checks(figurine,&cc, TRUE)) {
		/* reset the timer to try again later */
		(void) start_timer((long)rnd(5000), TIMER_OBJECT,
				FIG_TRANSFORM, (genericptr_t)figurine);
		return;
	}

	cansee_spot = cansee(cc.x, cc.y);
	mtmp = make_familiar(figurine, cc.x, cc.y, TRUE);
	if (mtmp) {
	    Sprintf(monnambuf, "%s",an(m_monnam(mtmp)));
	    switch (figurine->where) {
		case OBJ_INVENT:
		    if (Blind)
			You_feel("%s %s from your pack!", something,
			    locomotion(mtmp,"drop"));
		    else
			You("see %s %s out of your pack!",
			    monnambuf,
			    locomotion(mtmp,"drop"));
		    break;

		case OBJ_FLOOR:
		    if (cansee_spot && !silent) {
			You("suddenly see a figurine transform into %s!",
				monnambuf);
			redraw = TRUE;	/* update figurine's map location */
		    }
		    break;

		case OBJ_MINVENT:
		    if (cansee_spot && !silent) {
			struct monst *mon;
			mon = figurine->ocarry;
			/* figurine carring monster might be invisible */
			if (canseemon(figurine->ocarry)) {
			    Sprintf(carriedby, "%s pack",
				     s_suffix(a_monnam(mon)));
			}
			else if (is_pool(mon->mx, mon->my, FALSE))
			    Strcpy(carriedby, "empty water");
			else
			    Strcpy(carriedby, "thin air");
			You("see %s %s out of %s!", monnambuf,
			    locomotion(mtmp, "drop"), carriedby);
		    }
		    break;
#if 0
		case OBJ_MIGRATING:
		    break;
#endif

		default:
		    impossible("figurine came to life where? (%d)",
				(int)figurine->where);
		break;
	    }
	}
	/* free figurine now */
	obj_extract_self(figurine);
	obfree(figurine, (struct obj *)0);
	if (redraw) newsym(cc.x, cc.y);
}

STATIC_OVL boolean
figurine_location_checks(obj, cc, quietly)
struct obj *obj;
coord *cc;
boolean quietly;
{
	xchar x,y;

	if (carried(obj) && u.uswallow) {
		if (!quietly)
			You("don't have enough room in here.");
		return FALSE;
	}
	x = cc->x; y = cc->y;
	if (!isok(x,y)) {
		if (!quietly)
			You("cannot put the figurine there.");
		return FALSE;
	}
	if (IS_ROCK(levl[x][y].typ) &&
	    !(species_passes_walls(&mons[obj->corpsenm]) && may_passwall(x,y))) {
		if (!quietly)
		    You("cannot place a figurine in %s!",
			IS_TREES(levl[x][y].typ) ? "a tree" : "solid rock");
		return FALSE;
	}
	if (boulder_at(x,y) && !species_passes_walls(&mons[obj->corpsenm])
			&& !throws_rocks(&mons[obj->corpsenm])) {
		if (!quietly)
			You("cannot fit the figurine on the %s.",xname(boulder_at(x,y)));
		return FALSE;
	}
	return TRUE;
}

STATIC_OVL void
use_figurine(optr)
struct obj **optr;
{
	register struct obj *obj = *optr;
	xchar x, y;
	coord cc;

	if (u.uswallow) {
		/* can't activate a figurine while swallowed */
		if (!figurine_location_checks(obj, (coord *)0, FALSE))
			return;
	}
	if(!getdir((char *)0)) {
		flags.move = multi = 0;
		return;
	}
	x = u.ux + u.dx; y = u.uy + u.dy;
	cc.x = x; cc.y = y;
	/* Passing FALSE arg here will result in messages displayed */
	if (!figurine_location_checks(obj, &cc, FALSE)) return;
	You("%s and it transforms.",
	    (u.dx||u.dy) ? "set the figurine beside you" :
	    (Weightless || Is_waterlevel(&u.uz) ||
	     is_pool(cc.x, cc.y, TRUE)) ?
		"release the figurine" :
	    (u.dz < 0 ?
		"toss the figurine into the air" :
		"set the figurine on the ground"));
	(void) make_familiar(obj, cc.x, cc.y, FALSE);
	(void) stop_timer(FIG_TRANSFORM, (genericptr_t)obj);
	useup(obj);
	*optr = 0;
}

static NEARDATA const char lubricables[] = { ALL_CLASSES, ALLOW_NONE, 0 };
static NEARDATA const char need_to_remove_outer_armor[] =
			"need to remove your %s to grease your %s.";

STATIC_OVL void
use_grease(obj)
struct obj *obj;
{
	struct obj *otmp;
	char buf[BUFSZ];

	if (Glib) {
	    pline("%s from your %s.", Tobjnam(obj, "slip"),
		  makeplural(body_part(FINGER)));
	    dropx(obj);
	    return;
	}

	if (obj->spe > 0) {
		if ((obj->cursed || Fumbling) && !rn2(2)) {
			consume_obj_charge(obj, TRUE);

			pline("%s from your %s.", Tobjnam(obj, "slip"),
			      makeplural(body_part(FINGER)));
			dropx(obj);
			return;
		}
		otmp = getobj(lubricables, "grease");
		if (!otmp) return;
		if ((otmp->owornmask & WORN_ARMOR) && uarmc) {
			Strcpy(buf, xname(uarmc));
			You(need_to_remove_outer_armor, buf, xname(otmp));
			return;
		}
#ifdef TOURIST
		if ((otmp->owornmask & WORN_SHIRT) && (uarmc || uarm)) {
			Strcpy(buf, uarmc ? xname(uarmc) : "");
			if (uarmc && uarm) Strcat(buf, " and ");
			Strcat(buf, uarm ? xname(uarm) : "");
			You(need_to_remove_outer_armor, buf, xname(otmp));
			return;
		}
#endif
		consume_obj_charge(obj, TRUE);

		if (otmp != &zeroobj) {
			You("cover %s with a thick layer of grease.",
			    yname(otmp));
			otmp->greased = 1;
			if (obj->cursed && !nohands(youracedata)) {
			    incr_itimeout(&Glib, rnd(15));
			    pline("Some of the grease gets all over your %s.",
				makeplural(body_part(HAND)));
			}
		} else {
			Glib += rnd(15);
			You("coat your %s with grease.",
			    makeplural(body_part(FINGER)));
		}
	} else {
	    if (obj->known)
		pline("%s empty.", Tobjnam(obj, "are"));
	    else
		pline("%s to be empty.", Tobjnam(obj, "seem"));
	}
	update_inventory();
}

static struct trapinfo {
	struct obj *tobj;
	xchar tx, ty;
	int time_needed;
	boolean force_bungle;
} trapinfo;

void
reset_trapset()
{
	trapinfo.tobj = 0;
	trapinfo.force_bungle = 0;
}

/* touchstones - by Ken Arnold */
STATIC_OVL void
use_stone(tstone)
struct obj *tstone;
{
    struct obj *obj;
    boolean do_scratch;
    const char *streak_color, *choices;
    char stonebuf[QBUFSZ];
    static const char scritch[] = "\"scritch, scritch\"";
    static const char allowall[3] = { COIN_CLASS, ALL_CLASSES, 0 };
    static const char justgems[3] = { ALLOW_NONE, GEM_CLASS, 0 };
#ifndef GOLDOBJ
    struct obj goldobj;
#endif

    /* in case it was acquired while blinded */
    if (!Blind) tstone->dknown = 1;
    /* when the touchstone is fully known, don't bother listing extra
       junk as likely candidates for rubbing */
    choices = (tstone->otyp == TOUCHSTONE && tstone->dknown &&
		objects[TOUCHSTONE].oc_name_known) ? justgems : allowall;
    Sprintf(stonebuf, "rub on the stone%s", plur(tstone->quan));
    if ((obj = getobj(choices, stonebuf)) == 0)
	return;
#ifndef GOLDOBJ
    if (obj->oclass == COIN_CLASS) {
	u.ugold += obj->quan;	/* keep botl up to date */
	goldobj = *obj;
	dealloc_obj(obj);
	obj = &goldobj;
    }
#endif

    if (obj == tstone && obj->quan == 1) {
	You_cant("rub %s on itself.", the(xname(obj)));
	return;
    }

    if (tstone->otyp == TOUCHSTONE && tstone->cursed &&
	    obj->oclass == GEM_CLASS && !is_graystone(obj) &&
	    !obj_resists(obj, 80, 100)) {
	if (Blind)
	    pline("You feel something shatter.");
	else if (Hallucination)
	    pline("Oh, wow, look at the pretty shards.");
	else
	    pline("A sharp crack shatters %s%s.",
		  (obj->quan > 1) ? "one of " : "", the(xname(obj)));
#ifndef GOLDOBJ
     /* assert(obj != &goldobj); */
#endif
	useup(obj);
	return;
    }

    if (Blind) {
	pline("%s", scritch);
	return;
    } else if (Hallucination) {
	pline("Oh wow, man: Fractals!");
	return;
    }

    do_scratch = FALSE;
    streak_color = 0;

    switch (obj->oclass) {
    case GEM_CLASS:	/* these have class-specific handling below */
    case RING_CLASS:
	if (tstone->otyp != TOUCHSTONE) {
	    do_scratch = TRUE;
	} else if (obj->oclass == GEM_CLASS && (tstone->blessed ||
		(!tstone->cursed &&
		    (Role_if(PM_ARCHEOLOGIST) || Race_if(PM_GNOME))))) {
	    makeknown(TOUCHSTONE);
	    makeknown(obj->otyp);
	    prinv((char *)0, obj, 0L);
	    return;
	} else {
	    /* either a ring or the touchstone was not effective */
	    if (obj->obj_material == GLASS) {
		do_scratch = TRUE;
		break;
	    }
	}
	streak_color = c_obj_colors[objects[obj->otyp].oc_color];
	break;		/* gem or ring */

    default:
	switch (obj->obj_material) {
	case CLOTH:
	    pline("%s a little more polished now.", Tobjnam(tstone, "look"));
	    return;
	case LIQUID:
	    if (!obj->known)		/* note: not "whetstone" */
		You("must think this is a wetstone, do you?");
	    else
		pline("%s a little wetter now.", Tobjnam(tstone, "are"));
	    return;
	case WAX:
	    streak_color = "waxy";
	    break;		/* okay even if not touchstone */
	case WOOD:
	    streak_color = "wooden";
	    break;		/* okay even if not touchstone */
	case GOLD:
	    do_scratch = TRUE;	/* scratching and streaks */
	    streak_color = "golden";
	    break;
	case SILVER:
	    do_scratch = TRUE;	/* scratching and streaks */
	    streak_color = "silvery";
	    break;
	case GEMSTONE:
		if (obj->ovar1 && !obj_type_uses_ovar1(obj) && !obj_art_uses_ovar1(obj)) {
			/* similare check as above */
			if (tstone->otyp != TOUCHSTONE) {
				do_scratch = TRUE;
			}
			else if (tstone->blessed || (!tstone->cursed &&
				(Role_if(PM_ARCHEOLOGIST) || Race_if(PM_GNOME)))) {
				makeknown(TOUCHSTONE);
				makeknown(obj->ovar1);
				prinv((char *)0, obj, 0L);
				return;
			}
			/* the touchstone was not effective */
			streak_color = c_obj_colors[objects[obj->ovar1].oc_color];
		}
		else {
			do_scratch = (tstone->otyp != TOUCHSTONE);
		}
		break;
	default:
	    /* Objects passing the is_flimsy() test will not
	       scratch a stone.  They will leave streaks on
	       non-touchstones and touchstones alike. */
	    if (is_flimsy(obj))
		streak_color = c_obj_colors[objects[obj->otyp].oc_color];
	    else
		do_scratch = (tstone->otyp != TOUCHSTONE);
	    break;
	}
	break;		/* default oclass */
    }

    Sprintf(stonebuf, "stone%s", plur(tstone->quan));
    if (do_scratch)
	pline("You make %s%sscratch marks on the %s.",
	      streak_color ? streak_color : (const char *)"",
	      streak_color ? " " : "", stonebuf);
    else if (streak_color)
	pline("You see %s streaks on the %s.", streak_color, stonebuf);
    else
	pline("%s", scritch);
    return;
}

STATIC_OVL int
use_sensor(sensor)
struct obj *sensor;
{
	int scantype = 0;
	if(sensor->spe <= 0){
		pline("It seems inert.");
		return 0;
	} else {
		scantype = sensorMenu();
		if(!scantype) return 0;
		
		switch(scantype){
			case POT_MONSTER_DETECTION:
				monster_detect(sensor, 0);
			break;
			case POT_OBJECT_DETECTION:
				object_detect(sensor, 0);
			break;
			case SPE_CLAIRVOYANCE:
				do_vicinity_map(u.ux,u.uy);
			break;
			case WAN_PROBING:{
				struct obj *pobj;
				if (!getdir((char *)0)) return 0;
				if (u.dz < 0) {
					You("scan the %s thoroughly.  It seems it is %s.", ceiling(u.ux,u.uy), an(ceiling(u.ux,u.uy)));
				} else if(u.dz > 0) {
					pobj = level.objects[u.ux][u.uy];
					for(; pobj; pobj = pobj->nexthere){
						/* target object has now been "seen (up close)" */
						pobj->dknown = 1;
						if (Is_container(pobj) || pobj->otyp == STATUE) {
							if (!pobj->cobj)
							pline("%s empty.", Tobjnam(pobj, "are"));
							else {
							struct obj *o;
							/* view contents (not recursively) */
							for (o = pobj->cobj; o; o = o->nobj)
								o->dknown = 1;	/* "seen", even if blind */
							(void) display_cinventory(pobj);
							}
						}
					}
					You("probe beneath the %s.", surface(u.ux,u.uy));
					display_binventory(u.ux, u.uy, TRUE);
				} else {
					struct monst *mat = m_at(u.ux+u.dx,u.uy+u.dy);
					if(mat) probe_monster(mat);
					pobj = level.objects[u.ux+u.dx][u.uy+u.dy];
					for(; pobj; pobj = pobj->nexthere){
						/* target object has now been "seen (up close)" */
						pobj->dknown = 1;
						if (Is_container(pobj) || pobj->otyp == STATUE) {
							if (!pobj->cobj)
							pline("%s empty.", Tobjnam(pobj, "are"));
							else {
							struct obj *o;
							/* view contents (not recursively) */
							for (o = pobj->cobj; o; o = o->nobj)
								o->dknown = 1;	/* "seen", even if blind */
							(void) display_cinventory(pobj);
							}
						}
					}
				}
			} break;
			case WAN_SECRET_DOOR_DETECTION:
				findit();
			break;
		}
	}
	sensor->spe--;
	return 1;
}

STATIC_OVL int
sensorMenu()
{
	winid tmpwin;
	int n, how;
	char buf[BUFSZ];
	char incntlet;
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */
	
	Sprintf(buf, "Functions");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	
	incntlet = 'c';
	Sprintf(buf, "Scan for creatures");
	any.a_int = POT_MONSTER_DETECTION;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);

	incntlet = 'd';
	Sprintf(buf, "Scan for secret doors");
	any.a_int = WAN_SECRET_DOOR_DETECTION;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);

	incntlet = 'o';
	Sprintf(buf, "Scan for objects");
	any.a_int = POT_OBJECT_DETECTION;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);

	incntlet = 'p';
	Sprintf(buf, "Focused probe");
	any.a_int = WAN_PROBING;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);

	incntlet = 't';
	Sprintf(buf, "Survey terrain");
	any.a_int = SPE_CLAIRVOYANCE;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);
	
	end_menu(tmpwin, "Choose function:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return ( n > 0 ) ? selected[0].item.a_int : 0;
}

STATIC_OVL int
use_hypospray(hypo)
struct obj *hypo;
{
	struct obj *amp = getobj(tools, "inject");
	int i, ii, nothing=0;
	if(!amp) return 0;
	if(amp->otyp != HYPOSPRAY_AMPULE){
		You("can't inject that!");
		return 0;
	}
    if (!getdir((char *)0)) return 0;
	if(u.dz < 0){
		You("don't see a patient up there.");
		return 0;
	} else if(u.dz > 0){
		You("doubt the floor will respond to drugs.");
		return 0;
	} else if(u.dx || u.dy){
		struct monst *mtarg = m_at(u.ux+u.dx,u.uy+u.dy);
		if(!mtarg){
			You("don't find a patient there.");
			return 1;
		}
		if(amp->spe <= 0){
			pline("The ampule is empty!");
			return 1;
		}
		if(!has_blood_mon(mtarg)){
			pline("It would seem that the patient has no circulatory system....");
		} else switch(amp->ovar1){
			case POT_HEALING:
				if (mtarg->data == &mons[PM_PESTILENCE]){
					mtarg->mhp -= d(6 + 2 * bcsign(amp), 4);
					if(mtarg->mhp <= 0) xkilled(mtarg,1);
					break;
				}
				if(mtarg->mhp < mtarg->mhpmax) {
					mtarg->mhp = min_ints(mtarg->mhpmax,mtarg->mhp+d(6 + 2 * bcsign(amp), 4));
					if (canseemon(mtarg))
					pline("%s looks better.", Monnam(mtarg));
				}
			break;
			case POT_EXTRA_HEALING:
				if (mtarg->data == &mons[PM_PESTILENCE]){
					mtarg->mhp -= d(6 + 2 * bcsign(amp), 8);
					if(mtarg->mhp <= 0) xkilled(mtarg,1);
					break;
				}
				if(mtarg->mhp < mtarg->mhpmax) {
					mtarg->mhp = min_ints(mtarg->mhpmax,mtarg->mhp+d(6 + 2 * bcsign(amp), 8));
					if (canseemon(mtarg))
					pline("%s looks much better.", Monnam(mtarg));
				}
			break;
			case POT_FULL_HEALING:
				if (mtarg->data == &mons[PM_PESTILENCE]){
					if((mtarg->mhpmax > 3) && !resist(mtarg, POTION_CLASS, 0, NOTELL))
						mtarg->mhpmax /= 2;
					if((mtarg->mhp > 2) && !resist(mtarg, POTION_CLASS, 0, NOTELL))
						mtarg->mhp /= 2;
					if (mtarg->mhp > mtarg->mhpmax) mtarg->mhp = mtarg->mhpmax;
					if(mtarg->mhp <= 0) xkilled(mtarg,1);
					if (canseemon(mtarg))
						pline("%s looks rather ill.", Monnam(mtarg));
						break;
				}
			case POT_GAIN_ABILITY:
			case POT_RESTORE_ABILITY:
				if(mtarg->mhp < mtarg->mhpmax) {
					mtarg->mhp = min(mtarg->mhpmax,mtarg->mhp+400);
					if (canseemon(mtarg))
					pline("%s looks sound and hale again.", Monnam(mtarg));
				}
			break;
			case POT_BLINDNESS:
				if(haseyes(mtarg->data)) {
					register int btmp = rn1(200, 250 - 125 * bcsign(amp));
					btmp += mtarg->mblinded;
					mtarg->mblinded = min(btmp,127);
					mtarg->mcansee = 0;
				}
			break;
			case POT_HALLUCINATION:
				if(!resist(mtarg, POTION_CLASS, 0, NOTELL)) 
					mtarg->mstun = TRUE;
			case POT_CONFUSION:
				if(!resist(mtarg, POTION_CLASS, 0, NOTELL)) 
					mtarg->mconf = TRUE;
			break;
			case POT_PARALYSIS:
				if (mtarg->mcanmove) {
					mtarg->mcanmove = 0;
					mtarg->mfrozen = rn1(10, 25 - 12*bcsign(amp));
				}
			break;
			case POT_SPEED:
				mon_adjust_speed(mtarg, 1, amp);
			break;
			case POT_GAIN_ENERGY:
				if(!amp->cursed){
					if (canseemon(mtarg))
						pline("%s looks lackluster.", Monnam(mtarg));
					mtarg->mcan = 1;
				} else {
					if (canseemon(mtarg))
						pline("%s looks full of energy.", Monnam(mtarg));
					mtarg->mspec_used = 0;
					mtarg->mcan = 0;
				}
			break;
			case POT_SLEEPING:
				if (sleep_monst(mtarg, rn1(10, 25 - 12*bcsign(amp)), POTION_CLASS)) {
					pline("%s falls asleep.", Monnam(mtarg));
					slept_monst(mtarg);
				}
			break;
			case POT_POLYMORPH:
				if (canseemon(mtarg)) pline("%s suddenly mutates!", Monnam(mtarg));
				if(!resists_poly(mtarg->data))
					newcham(mtarg, (struct permonst *) 0, FALSE, FALSE);
			break;
			case POT_AMNESIA:
				if(!amp->cursed){
					if (canseemon(mtarg))
						pline("%s looks more tranquil.", Monnam(mtarg));
					if(!amp->blessed){
						mtarg->mtame = 0;
						mtarg->mferal = 0;
						mtarg->mpeaceful = 1;
					}
					mtarg->mcrazed = 0;
					mtarg->mberserk = 0;
				} else {
					if (canseemon(mtarg))
						pline("%s looks angry and confused!", Monnam(mtarg));
					mtarg->mcrazed = 1;
					mtarg->mberserk = 1;
					mtarg->mconf = 1;
					mtarg->mtame = 0;
					mtarg->mferal = 0;
					mtarg->mpeaceful = 0;
				}
			break;
		}
	} else {
		if(amp->spe <= 0){
			pline("The ampule is empty!");
			return 1;
		}
		switch(amp->ovar1){
			case POT_GAIN_ABILITY:
				if(amp->cursed) {
					//poison
				} else if (Fixed_abil) {
					nothing++;
				} else {      /* If blessed, increase all; if not, try up to */
					int itmp; /* 6 times to find one which can be increased. */
					i = -1;		/* increment to 0 */
					for (ii = A_MAX; ii > 0; ii--) {
					i = (amp->blessed ? i + 1 : rn2(A_MAX));
					/* only give "your X is already as high as it can get"
					   message on last attempt (except blessed potions) */
					itmp = (amp->blessed || ii == 1) ? 0 : -1;
					if (adjattrib(i, 1, itmp) && !amp->blessed)
						break;
					}
				}
			break;
			case POT_RESTORE_ABILITY:
				if (amp->blessed && u.ulevel < u.ulevelmax) {
					pluslvl(FALSE);
				}
				if(amp->cursed) {
					pline("Ulch!  This makes you feel mediocre!");
					break;
				} else {
					int lim;
					pline("Wow!  This makes you feel %s!",
					  (amp->blessed) ?
						(unfixable_trouble_count(FALSE) ? "better" : "great")
					  : "good");
					i = rn2(A_MAX);		/* start at a random point */
					for (ii = 0; ii < A_MAX; ii++) {
					lim = AMAX(i);
					if (i == A_STR && u.uhs >= 3) --lim;	/* WEAK */
					if (ABASE(i) < lim) {
						ABASE(i) = lim;
						flags.botl = 1;
						/* only first found if not blessed */
						if (!amp->blessed) break;
					}
					if(++i >= A_MAX) i = 0;
					}
				}
			break;
			case POT_BLINDNESS:
				if(Blind) nothing++;
				make_blinded(itimeout_incr(Blinded,
							   rn1(200, 250 - 125 * bcsign(amp))),
						 (boolean)!Blind);
			break;
			case POT_CONFUSION:
				if(!Confusion)
					if (Hallucination) {
						pline("What a trippy feeling!");
					} else pline("Huh, What?  Where am I?");
				else nothing++;
				make_confused(itimeout_incr(HConfusion,
								rn1(7, 16 - 8 * bcsign(amp))),
						  FALSE);
			break;
			case POT_PARALYSIS:
				if (Free_action)
					You("stiffen momentarily.");
				else {
					if (Levitation || Weightless || Is_waterlevel(&u.uz))
					You("are motionlessly suspended.");
#ifdef STEED
					else if (u.usteed)
					You("are frozen in place!");
#endif
					else
					Your("%s are frozen to the %s!",
						 makeplural(body_part(FOOT)), surface(u.ux, u.uy));
					nomul(-(rn1(10, 25 - 12*bcsign(amp))), "frozen by a potion");
					nomovemsg = You_can_move_again;
					exercise(A_DEX, FALSE);
				}
			break;
			case POT_SPEED:
				if(Wounded_legs && !amp->cursed
#ifdef STEED
				&& !u.usteed	/* heal_legs() would heal steeds legs */
#endif
								) {
					heal_legs();
					break;
				}
				if (!(HFast & INTRINSIC)) {
					if (!Fast) You("speed up.");
					else Your("quickness feels more natural.");
					HFast |= FROMOUTSIDE;
				} else nothing++;
				exercise(A_DEX, TRUE);
			break;
			case POT_HALLUCINATION:
				if (Hallucination || Halluc_resistance) nothing++;
				(void) make_hallucinated(itimeout_incr(HHallucination,
							   rn1(200, 600 - 300 * bcsign(amp))),
						  TRUE, 0L);
			break;
			case POT_HEALING:
				You_feel("better.");
				healup(d(6 + 2 * bcsign(amp), 4),
					   (!amp->cursed ? 1 : 0), !!amp->blessed, !amp->cursed);
				exercise(A_CON, TRUE);
			break;
			case POT_EXTRA_HEALING:
				You_feel("much better.");
				healup(d(6 + 2 * bcsign(amp), 8),
					   (amp->blessed ? 5 : !amp->cursed ? 2 : 0), !amp->cursed, TRUE);
				(void) make_hallucinated(0L,TRUE,0L);
				exercise(A_CON, TRUE);
				exercise(A_STR, TRUE);
			break;
			case POT_GAIN_ENERGY:
			{	register int num;
				if(amp->cursed)
					You_feel("lackluster.");
				else
					pline("Magical energies course through your body.");
				num = rnd(5) + 5 * amp->blessed + 1;
				u.uen += (amp->cursed) ? -100 : (amp->blessed) ? 200 : 100;
				if(u.uenmax <= 0) u.uenmax = 0;
				if(u.uen > u.uenmax) u.uen = u.uenmax;
				if(u.uen <= 0 && !Race_if(PM_INCANTIFIER)) u.uen = 0;
				flags.botl = 1;
				if(!amp->cursed) exercise(A_WIS, TRUE);
			}
			break;
			case POT_SLEEPING:
				if(Sleep_resistance || Free_action)
					You("yawn.");
				else {
					You("suddenly fall asleep!");
					fall_asleep(-rn1(10, 25 - 12*bcsign(amp)), TRUE);
				}
			break;
			case POT_FULL_HEALING:
				You_feel("completely healed.");
				healup(400, (4+4*bcsign(amp)), !amp->cursed, TRUE);
				/* Restore one lost level if blessed */
				if (amp->blessed && u.ulevel < u.ulevelmax) {
					///* when multiple levels have been lost, drinking
					//   multiple potions will only get half of them back */
					// u.ulevelmax -= 1;
					pluslvl(FALSE);
				}
				(void) make_hallucinated(0L,TRUE,0L);
				exercise(A_STR, TRUE);
				exercise(A_CON, TRUE);
			break;
			case POT_POLYMORPH:
				You_feel("a little %s.", Hallucination ? "normal" : "strange");
				if (!Unchanging) polyself(FALSE);
			break;
			case POT_AMNESIA:
				forget(amp->cursed ? 25 : amp->blessed ? 0 : 10);
				if (Hallucination)
					pline("Hakuna matata!");
				else
					You_feel("your memories dissolve.");

				/* Blessed amnesia makes you forget lycanthropy, sickness */
				if (amp->blessed) {
					if (u.ulycn >= LOW_PM && !Race_if(PM_HUMAN_WEREWOLF)) {
					You("forget your affinity to %s!",
							makeplural(mons[u.ulycn].mname));
					if (youmonst.data == &mons[u.ulycn])
						you_unwere(FALSE);
					u.ulycn = NON_PM;	/* cure lycanthropy */
					}
					make_sick(0L, (char *) 0, TRUE, SICK_ALL);

					/* You feel refreshed */
					if(Race_if(PM_INCANTIFIER)) u.uen += 50 + rnd(50);
					else u.uhunger += 50 + rnd(50);
					
					newuhs(FALSE);
				} else
					exercise(A_WIS, FALSE);
			break;
		}
		if(nothing) {
			You("have a %s feeling for a moment, then it passes.",
			  Hallucination ? "normal" : "peculiar");
		}
	}
	amp->spe--;
	return 1;
}

/* Place a landmine/bear trap.  Helge Hafting */
STATIC_OVL void
use_trap(otmp)
struct obj *otmp;
{
	int ttyp, tmp;
	const char *what = (char *)0;
	char buf[BUFSZ];
	const char *occutext = "setting the trap";

	if (nohands(youracedata))
	    what = "without hands";
	else if(!freehand())
	    what = "without free hands";
	else if (Stunned)
	    what = "while stunned";
	else if (u.uswallow)
	    what = is_animal(u.ustuck->data) ? "while swallowed" :
			"while engulfed";
	else if (Underwater)
	    what = "underwater";
	else if (Levitation)
	    what = "while levitating";
	else if (is_pool(u.ux, u.uy, TRUE))
	    what = "in water";
	else if (is_lava(u.ux, u.uy))
	    what = "in lava";
	else if (On_stairs(u.ux, u.uy))
	    what = (u.ux == xdnladder || u.ux == xupladder) ?
			"on the ladder" : "on the stairs";
	else if (IS_FURNITURE(levl[u.ux][u.uy].typ) ||
		IS_ROCK(levl[u.ux][u.uy].typ) ||
		closed_door(u.ux, u.uy) || t_at(u.ux, u.uy))
	    what = "here";
	if (what) {
	    You_cant("set a trap %s!",what);
	    reset_trapset();
	    return;
	}
	ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
	if (otmp == trapinfo.tobj &&
		u.ux == trapinfo.tx && u.uy == trapinfo.ty) {
	    You("resume setting %s %s.",
		shk_your(buf, otmp),
		defsyms[trap_to_defsym(what_trap(ttyp))].explanation);
	    set_occupation(set_trap, occutext, 0);
	    return;
	}
	trapinfo.tobj = otmp;
	trapinfo.tx = u.ux,  trapinfo.ty = u.uy;
	tmp = ACURR(A_DEX);
	trapinfo.time_needed = (tmp > 17) ? 2 : (tmp > 12) ? 3 :
				(tmp > 7) ? 4 : 5;
	if (Blind) trapinfo.time_needed *= 2;
	tmp = ACURR(A_STR);
	if (ttyp == BEAR_TRAP && tmp < 18)
	    trapinfo.time_needed += (tmp > 12) ? 1 : (tmp > 7) ? 2 : 4;
	/*[fumbling and/or confusion and/or cursed object check(s)
	   should be incorporated here instead of in set_trap]*/
#ifdef STEED
	if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
	    boolean chance;

	    if (Fumbling || otmp->cursed) chance = (rnl(100) > 30);
	    else  chance = (rnl(100) > 50);
	    You("aren't very skilled at reaching from %s.",
		mon_nam(u.usteed));
	    Sprintf(buf, "Continue your attempt to set %s?",
		the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
	    if(yn(buf) == 'y') {
		if (chance) {
			switch(ttyp) {
			    case LANDMINE:	/* set it off */
			    	trapinfo.time_needed = 0;
			    	trapinfo.force_bungle = TRUE;
				break;
			    case BEAR_TRAP:	/* drop it without arming it */
				reset_trapset();
				You("drop %s!",
			  the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
				dropx(otmp);
				return;
			}
		}
	    } else {
	    	reset_trapset();
		return;
	    }
	}
#endif
	You("begin setting %s %s.",
	    shk_your(buf, otmp),
	    defsyms[trap_to_defsym(what_trap(ttyp))].explanation);
	set_occupation(set_trap, occutext, 0);
	return;
}

STATIC_PTR
int
set_trap()
{
	struct obj *otmp = trapinfo.tobj;
	struct trap *ttmp;
	int ttyp;

	if (!otmp || !carried(otmp) ||
		u.ux != trapinfo.tx || u.uy != trapinfo.ty) {
	    /* ?? */
	    reset_trapset();
	    return 0;
	}

	if (--trapinfo.time_needed > 0) return 1;	/* still busy */

	ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
	ttmp = maketrap(u.ux, u.uy, ttyp);
	if (ttmp) {
	    ttmp->tseen = 1;
	    ttmp->madeby_u = 1;
	    newsym(u.ux, u.uy); /* if our hero happens to be invisible */
	    if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
		add_damage(u.ux, u.uy, 0L);		/* schedule removal */
	    }
	    if (!trapinfo.force_bungle)
		You("finish arming %s.",
			the(defsyms[trap_to_defsym(what_trap(ttyp))].explanation));
	    if (((otmp->cursed || Fumbling) && (rnl(100) > 50)) || trapinfo.force_bungle)
		dotrap(ttmp,
			(unsigned)(trapinfo.force_bungle ? FORCEBUNGLE : 0));
	} else {
	    /* this shouldn't happen */
	    Your("trap setting attempt fails.");
	}
	useup(otmp);
	reset_trapset();
	return 0;
}

STATIC_OVL int
use_droven_cloak(optr)
struct obj **optr;
{
	struct obj *otmp = *optr;
	int rx, ry;
	const char *what = (char *)0;
	struct trap *ttmp;
	struct monst *mtmp;

    if (!getdir((char *)0) || u.dz < 0) return 0;

    if (Stunned || (Confusion && !rn2(5))) confdir();
    rx = u.ux + u.dx;
    ry = u.uy + u.dy;
    mtmp = m_at(rx, ry);
	ttmp = t_at(rx, ry);

	if (Stunned)
	    what = "while stunned";
	else if (u.uswallow)
	    what = is_animal(u.ustuck->data) ? "while swallowed" :
			"while engulfed";
	else if (Underwater)
	    what = "underwater";
	else if (Levitation)
	    what = "while levitating";
	else if (is_pool(rx, ry, TRUE))
	    what = "in water";
	else if (is_lava(rx, ry))
	    what = "in lava";
	else if (On_stairs(rx, ry) && !(ttmp && ttmp->ttyp == WEB))
	    what = (rx == xdnladder || rx == xupladder) ?
			"on the ladder" : "on the stairs";
	else if (IS_FURNITURE(levl[rx][ry].typ) ||
		IS_ROCK(levl[rx][ry].typ) ||
		closed_door(rx, ry) || (ttmp && ttmp->ttyp != WEB))
	    what = "here";
	if (what && !(ttmp && ttmp->ttyp == WEB)) {
	    You_cant("set a trap %s!",what);
	    reset_trapset();
	    return 0;
	}
	
	if(ttmp) {
		if(otmp->oeroded3) otmp->oeroded3--;
		pline("The cloak sweeps up a web!");
		if(!Is_lolth_level(&u.uz)){ //results in unlimited recharging in lolths domain, no big deal
			deltrap(ttmp);
			newsym(rx, ry);
		}
		if(rx==u.ux && ry==u.uy) u.utrap = 0;
		else if(mtmp) mtmp->mtrapped = 0;
	}
	else if(!(otmp->oartifact) || otmp->oeroded3 < 3){
		ttmp = maketrap(rx, ry, WEB);
		if(ttmp){
			pline("A web spins out from the cloak!");
			ttmp->madeby_u = 1;
			ttmp->tseen = 1;
			newsym(rx, ry);
			if (*in_rooms(rx,ry,SHOPBASE)) {
				add_damage(rx, ry, 0L);		/* schedule removal */
			}
			if(rx==u.ux && ry==u.uy) dotrap(ttmp, NOWEBMSG);
			else if(mtmp) mintrap(mtmp);
		} else pline("The cloak cannot spin a web there!");
		if(otmp->oeroded3 == 3){
			useup(otmp);
			*optr = 0;
			pline("The thoroughly tattered cloak falls to pieces");
		} else otmp->oeroded3++;
	} else {
		pline("The cloak cannot spin any more webs.");
		return 0;
	}
	reset_trapset();
	return 1;
}

STATIC_OVL int
use_darkweavers_cloak(otmp)
struct obj *otmp;
{
	const char *what = (char *)0;

	if (Stunned)
	    what = "while stunned";
	else if (u.uswallow)
	    what = is_animal(u.ustuck->data) ? "while swallowed" :
			"while engulfed";
	else if (Underwater)
	    what = "underwater";
	if (what) {
	    You_cant("release darkness %s!",what);
	    return 0;
	}
	
	if(!levl[u.ux][u.uy].lit) {
		if(otmp->spe < 7) otmp->spe++;
		pline("The cloak sweeps up the dark!");
		litroom(TRUE, otmp);	/* only needs to be done once */
	}
	else if(otmp->spe > -5){
		otmp->spe--;
		pline("The cloak releases a cloud of darkness!");
		litroom(FALSE, otmp);	/* only needs to be done once */
	}
	return 1;
}

int
use_whip(obj)
struct obj *obj;
{
    char buf[BUFSZ];
    struct monst *mtmp;
    struct obj *otmp;
    int rx, ry, proficient, res = 0;
    const char *msg_slipsfree = "The whip slips free.";
    const char *msg_snap = "Snap!";

    if (obj != uwep) {
	if (!wield_tool(obj, "lash")) return 0;
	else res = 1;
    }
    if (!getdir((char *)0)) return res;

	if(obj->otyp == FORCE_WHIP && !u.dx && !u.dy && !u.dz){
		return use_force_sword(obj);
	}

    if (Stunned || (Confusion && !rn2(5))) confdir();
    rx = u.ux + u.dx;
    ry = u.uy + u.dy;
    mtmp = m_at(rx, ry);

    /* fake some proficiency checks */
    proficient = 0;
    if (Role_if(PM_ARCHEOLOGIST)) ++proficient;
    if (ACURR(A_DEX) < 6) proficient--;
    else if (ACURR(A_DEX) >= 14) proficient += (ACURR(A_DEX) - 14);
    if (Fumbling) --proficient;
    if (proficient > 3) proficient = 3;
    if (proficient < 0) proficient = 0;

    if (u.uswallow && attack(u.ustuck)) {
	There("is not enough room to flick your whip.");

    } else if (Underwater) {
	There("is too much resistance to flick your whip.");

    } else if (u.dz < 0) {
	You("flick a bug off of the %s.",ceiling(u.ux,u.uy));

    } else if ((!u.dx && !u.dy) || (u.dz > 0)) {
	int dam;

#ifdef STEED
		/* Sometimes you hit your steed by mistake */
		if (u.usteed && !rn2(proficient + 2)) {
			You("whip %s!", mon_nam(u.usteed));
			kick_steed();
			return 1;
		}
#endif
		if (Levitation
#ifdef STEED
			|| u.usteed
#endif
			) {
			/* Have a shot at snaring something on the floor */
			otmp = level.objects[u.ux][u.uy];
			if (otmp && otmp->otyp == CORPSE && otmp->corpsenm == PM_HORSE) {
			pline("Why beat a dead horse?");
			return 1;
			}
			if (otmp && proficient) {
			You("wrap your whip around %s on the %s.",
				an(singular(otmp, xname)), surface(u.ux, u.uy));
			if (rnl(100) >= 16 || pickup_object(otmp, 1L, TRUE) < 1)
				pline("%s", msg_slipsfree);
			return 1;
			}
		}
		dam = rnd(2) + dbon(obj) + obj->spe;
		if (dam <= 0) dam = 1;
		You("hit your %s with your whip.", body_part(FOOT));
		Sprintf(buf, "killed %sself with %s whip", uhim(), uhis());
		losehp(dam, buf, NO_KILLER_PREFIX);
		flags.botl = 1;
		return 1;

    } else if ((Fumbling || Glib) && !rn2(5)) {
		pline_The("whip slips out of your %s.", body_part(HAND));
		dropx(obj);

    } else if (u.utrap && u.utraptype == TT_PIT) {
		/*
		 *     Assumptions:
		 *
		 *	if you're in a pit
		 *		- you are attempting to get out of the pit
		 *		- or, if you are applying it towards a small
		 *		  monster then it is assumed that you are
		 *		  trying to hit it.
		 *	else if the monster is wielding a weapon
		 *		- you are attempting to disarm a monster
		 *	else
		 *		- you are attempting to hit the monster
		 *
		 *	if you're confused (and thus off the mark)
		 *		- you only end up hitting.
		 *
		 */
		const char *wrapped_what = (char *)0;

		if (mtmp) {
			if (bigmonst(mtmp->data)) {
			wrapped_what = strcpy(buf, mon_nam(mtmp));
			} else if (proficient) {
			if (attack(mtmp)) return 1;
			else pline("%s", msg_snap);
			}
		}
		if (!wrapped_what) {
			if (IS_FURNITURE(levl[rx][ry].typ))
			wrapped_what = something;
			else if (boulder_at(rx, ry))
			wrapped_what = xname(boulder_at(rx,ry));
		}
		if (wrapped_what) {
			coord cc;

			cc.x = rx; cc.y = ry;
			You("wrap your whip around %s.", wrapped_what);
			if (proficient && rn2(proficient + 2)) {
			if (!mtmp || enexto(&cc, rx, ry, youracedata)) {
				You("yank yourself out of the pit!");
				teleds(cc.x, cc.y, TRUE);
				u.utrap = 0;
				vision_full_recalc = 1;
			}
			} else {
			pline("%s", msg_slipsfree);
			}
			if (mtmp) wakeup(mtmp, TRUE);
		} else pline("%s", msg_snap);

    } else if (mtmp) {
		if (!canspotmon(mtmp) &&
			!glyph_is_invisible(levl[rx][ry].glyph)) {
		   pline("A monster is there that you couldn't see.");
		   map_invisible(rx, ry);
		}
		otmp = rn2(3) ? MON_WEP(mtmp) : MON_SWEP(mtmp);	/* can be null */
		if (otmp) {
			char onambuf[BUFSZ];
			const char *mon_hand;
			boolean gotit = proficient && (!Fumbling || !rn2(10));

			Strcpy(onambuf, cxname(otmp));
			if (gotit) {
			mon_hand = mbodypart(mtmp, HAND);
			if (bimanual(otmp,mtmp->data)) mon_hand = makeplural(mon_hand);
			} else
			mon_hand = 0;	/* lint suppression */

			You("wrap your whip around %s %s.",
			s_suffix(mon_nam(mtmp)), onambuf);
			if (gotit && otmp->cursed && !is_weldproof_mon(mtmp)) {
			pline("%s welded to %s %s%c",
				  (otmp->quan == 1L) ? "It is" : "They are",
				  mhis(mtmp), mon_hand,
				  !otmp->bknown ? '!' : '.');
			otmp->bknown = 1;
			gotit = FALSE;	/* can't pull it free */
			}
			if (gotit) {
			obj_extract_self(otmp);
			possibly_unwield(mtmp, FALSE);
			setmnotwielded(mtmp,otmp);

			switch (rn2(proficient + 1)) {
			case 2:
				/* to floor near you */
				You("yank %s %s to the %s!", s_suffix(mon_nam(mtmp)),
				onambuf, surface(u.ux, u.uy));
				place_object(otmp, u.ux, u.uy);
				stackobj(otmp);
				break;
			case 3:
				/* right to you */
#if 0
				if (!rn2(25)) {
				/* proficient with whip, but maybe not
				   so proficient at catching weapons */
				int hitu, hitvalu;

				hitvalu = 8 + otmp->spe;
				hitu = thitu(hitvalu,
						 dmgval(otmp, &youmonst, 0),
						 otmp, (char *)0, FALSE);
				if (hitu) {
					pline_The("%s hits you as you try to snatch it!",
					the(onambuf));
				}
				place_object(otmp, u.ux, u.uy);
				stackobj(otmp);
				break;
				}
#endif /* 0 */
				/* right into your inventory */
				You("snatch %s %s!", s_suffix(mon_nam(mtmp)), onambuf);
				if (otmp->otyp == CORPSE &&
					touch_petrifies(&mons[otmp->corpsenm]) &&
					!uarmg && !Stone_resistance &&
					!(poly_when_stoned(youracedata) &&
					polymon(PM_STONE_GOLEM))) {
				char kbuf[BUFSZ];

				Sprintf(kbuf, "%s corpse",
					an(mons[otmp->corpsenm].mname));
				pline("Snatching %s is a fatal mistake.", kbuf);
				instapetrify(kbuf);
				}
				otmp = hold_another_object(otmp, "You drop %s!",
							   doname(otmp), (const char *)0);
				break;
			default:
				/* to floor beneath mon */
				You("yank %s from %s %s!", the(onambuf),
				s_suffix(mon_nam(mtmp)), mon_hand);
				obj_no_longer_held(otmp);
				place_object(otmp, mtmp->mx, mtmp->my);
				stackobj(otmp);
				break;
			}
			} else {
			pline("%s", msg_slipsfree);
			}
			wakeup(mtmp, TRUE);
		} else {
			if (mtmp->m_ap_type &&
			!Protection_from_shape_changers && !sensemon(mtmp))
			stumble_onto_mimic(mtmp);
			else You("flick your whip towards %s.", mon_nam(mtmp));
			if (proficient) {
			if (attack(mtmp)) return 1;
			else pline("%s", msg_snap);
			}
		}

    } else if (Weightless || Is_waterlevel(&u.uz)) {
	    /* it must be air -- water checked above */
	    You("snap your whip through thin air.");

    } else {
	pline("%s", msg_snap);

    }
    return 1;
}


static const char
	not_enough_room[] = "There's not enough room here to use that.",
	where_to_hit[] = "Where do you want to hit?",
	cant_see_spot[] = "won't hit anything if you can't see that spot.",
	cant_reach[] = "can't reach that spot from here.";

/* Distance attacks by pole-weapons */
STATIC_OVL int
use_pole (obj)
	struct obj *obj;
{
	int res = 0, typ, max_range = 4, min_range = 4, skillBonus = 0;
	coord cc;
	struct monst *mtmp;


	/* Are you allowed to use the pole? */
	if (u.uswallow) {
	    pline("%s", not_enough_room);
	    return (0);
	}
	if (obj != uwep && obj != uarmg) {
	    if (!wield_tool(obj, "swing")) return 0;
	    else res = 1;
	}
     /* assert(obj == uwep); */

	/* Prompt for a location */
	pline("%s", where_to_hit);
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "the spot to hit") < 0)
	    return 0;	/* user pressed ESC */

	/* Calculate range */
	typ = uwep_skill_type();
	if (typ == P_NONE || (P_SKILL(typ)+skillBonus) <= P_BASIC) max_range = 4;
	else if ( (P_SKILL(typ)+skillBonus) == P_SKILLED) max_range = 5;
	else max_range = 8;
	if (distu(cc.x, cc.y) > max_range) {
	    pline("Too far!");
	    return (res);
	} else if (distu(cc.x, cc.y) < min_range) {
	    pline("Too close!");
	    return (res);
	} else if (!cansee(cc.x, cc.y) &&
		   ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0 ||
		    !canseemon(mtmp))) {
	    You(cant_see_spot);
	    return (res);
	} else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
	    You(cant_reach);
	    return res;
	}

	/* Attack the monster there */
	if ((mtmp = m_at(cc.x, cc.y)) != (struct monst *)0) {
	    int oldhp = mtmp->mhp;

	    bhitpos = cc;
	    check_caitiff(mtmp);
	    (void) thitmonst(mtmp, uwep, 0);
	    /* check the monster's HP because thitmonst() doesn't return
	     * an indication of whether it hit.  Not perfect (what if it's a
	     * non-silver weapon on a shade?)
	     */
	    if (mtmp->mhp < oldhp)
		u.uconduct.weaphit++;
	} else if(levl[cc.x][cc.y].typ == GRASS){
		   levl[cc.x][cc.y].typ = SOIL;
		   if(!rn2(3)) mksobj_at(SHEAF_OF_HAY,cc.x,cc.y,TRUE,FALSE);
		   You("cut away the grass!");
		   newsym(cc.x,cc.y);
	} else {
	    /* Now you know that nothing is there... */
	    pline("%s", nothing_happens);
	}
	return (1);
}

STATIC_OVL int
use_cream_pie(obj)
struct obj *obj;
{
	boolean wasblind = Blind;
	boolean wascreamed = u.ucreamed;
	boolean several = FALSE;

	if (obj->quan > 1L) {
		several = TRUE;
		obj = splitobj(obj, 1L);
	}
	if (Hallucination)
		You("give yourself a facial.");
	else
		pline("You immerse your %s in %s%s.", body_part(FACE),
			several ? "one of " : "",
			several ? makeplural(the(xname(obj))) : the(xname(obj)));
	if(can_blnd((struct monst*)0, &youmonst, AT_WEAP, obj)) {
		int blindinc = rnd(25);
		u.ucreamed += blindinc;
		make_blinded(Blinded + (long)blindinc, FALSE);
		if (!Blind || (Blind && wasblind))
			pline("There's %ssticky goop all over your %s.",
				wascreamed ? "more " : "",
				body_part(FACE));
		else /* Blind  && !wasblind */
			You_cant("see through all the sticky goop on your %s.",
				body_part(FACE));
	}
	if (obj->unpaid) {
		verbalize("You used it, you bought it!");
		bill_dummy_object(obj);
	}
	obj_extract_self(obj);
	delobj(obj);
	return 0;
}

STATIC_OVL int
use_grapple (obj)
	struct obj *obj;
{
	int res = 0, typ, max_range = 4, tohit;
	coord cc;
	struct monst *mtmp;
	struct obj *otmp;

	/* Are you allowed to use the hook? */
	if (u.uswallow) {
	    pline("%s", not_enough_room);
	    return (0);
	}
	if (obj != uwep) {
	    if (!wield_tool(obj, "cast")) return 0;
	    else res = 1;
	}
     /* assert(obj == uwep); */

	/* Prompt for a location */
	pline("%s", where_to_hit);
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "the spot to hit") < 0)
	    return 0;	/* user pressed ESC */

	/* Calculate range */
	typ = uwep_skill_type();
	if (typ == P_NONE || P_SKILL(typ) <= P_BASIC) max_range = 4;
	else if (P_SKILL(typ) == P_SKILLED) max_range = 5;
	else max_range = 8;
	if (distu(cc.x, cc.y) > max_range) {
	    pline("Too far!");
	    return (res);
	} else if (!cansee(cc.x, cc.y)) {
	    You(cant_see_spot);
	    return (res);
	} else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
	    You(cant_reach);
	    return res;
	}

	/* What do you want to hit? */
	tohit = rn2(5);
	if (typ != P_NONE && P_SKILL(typ) >= P_SKILLED) {
	    winid tmpwin = create_nhwindow(NHW_MENU);
	    anything any;
	    char buf[BUFSZ];
	    menu_item *selected;

	    any.a_void = 0;	/* set all bits to zero */
	    any.a_int = 1;	/* use index+1 (cant use 0) as identifier */
	    start_menu(tmpwin);
	    any.a_int++;
	    Sprintf(buf, "an object on the %s", surface(cc.x, cc.y));
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
	    any.a_int++;
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			"a monster", MENU_UNSELECTED);
	    any.a_int++;
	    Sprintf(buf, "the %s", surface(cc.x, cc.y));
	    add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
	    end_menu(tmpwin, "Aim for what?");
	    tohit = rn2(4);
	    if (select_menu(tmpwin, PICK_ONE, &selected) > 0 &&
			rn2(P_SKILL(typ) > P_SKILLED ? 20 : 2))
		tohit = selected[0].item.a_int - 1;
	    free((genericptr_t)selected);
	    destroy_nhwindow(tmpwin);
	}

	/* What did you hit? */
	switch (tohit) {
	case 0:	/* Trap */
	    /* FIXME -- untrap needs to deal with non-adjacent traps */
	    break;
	case 1:	/* Object */
	    if ((otmp = level.objects[cc.x][cc.y]) != 0) {
		You("snag an object from the %s!", surface(cc.x, cc.y));
		(void) pickup_object(otmp, 1L, FALSE);
		/* If pickup fails, leave it alone */
		newsym(cc.x, cc.y);
		return (1);
	    }
	break;
	case 2:	/* Monster */
	    if ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0) break;
	    if (verysmall(mtmp->data) && !rn2(4) &&
			enexto(&cc, u.ux, u.uy, (struct permonst *)0)) {
		You("pull in %s!", mon_nam(mtmp));
		mtmp->mundetected = 0;
		rloc_to(mtmp, cc.x, cc.y);
		return (1);
	    } else if ((!bigmonst(mtmp->data) && !strongmonst(mtmp->data)) ||
		       rn2(4)) {
		(void) thitmonst(mtmp, uwep, 0);
		return (1);
	    }
	    /* FALL THROUGH */
	case 3:	/* Surface */
	    if (IS_AIR(levl[cc.x][cc.y].typ) || is_pool(cc.x, cc.y, TRUE))
		pline_The("hook slices through the %s.", surface(cc.x, cc.y));
	    else {
		You("are yanked toward the %s!", surface(cc.x, cc.y));
		hurtle(sgn(cc.x-u.ux), sgn(cc.y-u.uy), 1, FALSE);
		spoteffects(TRUE);
	    }
	    return (1);
	default:	/* Yourself (oops!) */
	    if (P_SKILL(typ) <= P_BASIC) {
		You("hook yourself!");
		losehp(rn1(10,10), "a grappling hook", KILLED_BY);
		return (1);
	    }
	    break;
	}
	pline("%s", nothing_happens);
	return (1);
}

STATIC_OVL int
use_crook (obj)
	struct obj *obj;
{
	int res = 0, typ, max_range = 4, tohit;
	coord cc;
	struct monst *mtmp;
	struct obj *otmp;

	/* Are you allowed to use the crook? */
	if (u.uswallow) {
	    pline("%s", not_enough_room);
	    return (0);
	}
	if (obj != uwep) {
	    if (!wield_tool(obj, "hook")) return 0;
	    else res = 1;
	}
     /* assert(obj == uwep); */

	/* Prompt for a location */
	pline("%s", where_to_hit);
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "the spot to hook") < 0)
	    return 0;	/* user pressed ESC */

	/* Calculate range */
	typ = uwep_skill_type();
	max_range = 8;
	if (distu(cc.x, cc.y) > max_range) {
	    pline("Too far!");
	    return (res);
	} else if (!cansee(cc.x, cc.y)) {
	    You(cant_see_spot);
	    return (res);
	} else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
	    You(cant_reach);
	    return res;
	}

	/* What do you want to hit? */
	{
	    winid tmpwin = create_nhwindow(NHW_MENU);
	    anything any;
	    char buf[BUFSZ];
	    menu_item *selected;

	    any.a_void = 0;	/* set all bits to zero */
	    any.a_int = 1;	/* use index+1 (cant use 0) as identifier */
	    start_menu(tmpwin);
			any.a_int++;
			add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
				"hit a monster", MENU_UNSELECTED);
			
			any.a_int++;
			add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
				"pull a monster", MENU_UNSELECTED);
			
			any.a_int++;
			Sprintf(buf, "pull an object on the %s", surface(cc.x, cc.y));
			add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
				 buf, MENU_UNSELECTED);
	    end_menu(tmpwin, "Aim for what?");
	    if (select_menu(tmpwin, PICK_ONE, &selected) > 0)
			tohit = selected[0].item.a_int - 1;
		else return 0;
	    free((genericptr_t)selected);
	    destroy_nhwindow(tmpwin);
	}

	/* What did you hit? */
	switch (tohit) {
	case 0:	/* Trap */
	    /* FIXME -- untrap needs to deal with non-adjacent traps */
	    break;
	case 1:	/*Hit Monster */
	    if ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0) break;
		(void) thitmonst(mtmp, uwep, 0);
		return (1);
	break;
	case 2:	/*Hook Monster */
	    if ((mtmp = m_at(cc.x, cc.y)) == (struct monst *)0) break;
		if(mtmp->mpeaceful){
			if (!bigmonst(mtmp->data) &&
				enexto(&cc, u.ux, u.uy, (struct permonst *)0)
			) {
				You("pull in %s!", mon_nam(mtmp));
				mtmp->mundetected = 0;
				mtmp->movement = max(0, mtmp->movement - 12);
				rloc_to(mtmp, cc.x, cc.y);
				return (1);
			} else {
				You("pull yourself toward %s!", mon_nam(mtmp));
				hurtle(sgn(mtmp->mx-u.ux), sgn(mtmp->my-u.uy), 1, FALSE);
				spoteffects(TRUE);
				return (1);
			}
		} else {
			if (!bigmonst(mtmp->data) && !strongmonst(mtmp->data) && rn2(P_SKILL(typ)) &&
				enexto(&cc, u.ux, u.uy, (struct permonst *)0)
			) {
				You("pull in %s!", mon_nam(mtmp));
				mtmp->mundetected = 0;
				mtmp->movement = max(0, mtmp->movement - 12);
				rloc_to(mtmp, cc.x, cc.y);
				return (1);
			} else if ((!bigmonst(mtmp->data) && !strongmonst(mtmp->data)) ||
				   rn2(P_SKILL(typ))
			) {
				(void) thitmonst(mtmp, uwep, 0);
				return (1);
			} else {
				You("are yanked toward %s!", mon_nam(mtmp));
				hurtle(sgn(mtmp->mx-u.ux), sgn(mtmp->my-u.uy), 1, FALSE);
				spoteffects(TRUE);
				return (1);
			}
		}
	break;
	case 3:	/* Object */
	    if ((otmp = level.objects[cc.x][cc.y]) != 0) {
			You("snag an object from the %s!", surface(cc.x, cc.y));
			(void) pickup_object(otmp, 1L, FALSE);
			/* If pickup fails, leave it alone */
			newsym(cc.x, cc.y);
			return (1);
	    }
	    break;
	// case 3:	/* Surface */
	    // if (IS_AIR(levl[cc.x][cc.y].typ) || is_pool(cc.x, cc.y, TRUE))
			// pline_The("hook slices through the %s.", surface(cc.x, cc.y));
	    // else {
			// You("are yanked toward the %s!", surface(cc.x, cc.y));
			// hurtle(sgn(cc.x-u.ux), sgn(cc.y-u.uy), 1, FALSE);
			// spoteffects(TRUE);
	    // }
	    // return (1);
	// break;
	default:
		pline("Invalid target for crook-hook");
	break;
	}
	pline("%s", nothing_happens);
	return (1);
}

boolean
use_ring_of_wishes(obj)
struct obj *obj;
{
	struct monst * mtmp;
	struct monst * mtmp2 = (struct monst*)0;
	struct monst * mtmp3 = (struct monst*)0;
	boolean madewish = FALSE;

	if (obj->otyp != RIN_WISHES)
	{
		impossible("object other than ring of wishes passed to use_ring_of_wishes");
		return FALSE;
	}
	if (obj->cursed || (Luck + rn2(5) < 0)){	// to be less cruel, it doesn't use up a charge
		pline1(nothing_happens);
		return FALSE;
	}
	if (obj->spe <= 0){
		pline1(nothing_happens);
		pline("The ring crumbles to dust!");
		useupall(obj);
		return FALSE;
	}
	if (!(obj->owornmask & W_RING)) {
		if (objects[RIN_WISHES].oc_name_known)
			You_feel("that you should be wearing %s.", the(xname(obj)));
		else
			pline1(nothing_happens);
		return FALSE;
	}

	if (obj->spe > 0)
	{
		if (!(mtmp = makemon(&mons[PM_DJINNI], u.ux, u.uy, NO_MM_FLAGS))){
			pline1(nothing_happens);
		}
		else
		{
			if ((u.uevent.utook_castle & ARTWISH_EARNED) && !(u.uevent.utook_castle & ARTWISH_SPENT))
				mtmp2 = makemon(&mons[PM_PSYCHOPOMP], u.ux, u.uy, NO_MM_FLAGS);
			if ((u.uevent.uunknowngod & ARTWISH_EARNED) && !(u.uevent.uunknowngod & ARTWISH_SPENT))
				mtmp3 = makemon(&mons[PM_PRIEST_OF_AN_UNKNOWN_GOD], u.ux, u.uy, NO_MM_FLAGS);

			if (!Blind) {
				pline("%s appears in a cloud of smoke!", Amonnam(mtmp));
				if (mtmp2 || mtmp3)
					pline("It is accompanied by %s%s%s.",
					mtmp2 ? a_monnam(mtmp2) : "",
					(mtmp2 && mtmp3) ? " and " : "",
					mtmp3 ? a_monnam(mtmp3) : "");
				pline("%s speaks.", Monnam(mtmp));
			}
			else {
				You("smell acrid fumes.");
				pline("%s speaks.", Something);
			}
			verbalize("I am the djinni of the ring.  I will grant one wish!");
			int artwishes = u.uconduct.wisharti;
			if (makewish(allow_artwish() | WISH_VERBOSE)) {
				obj->spe--;
				madewish = TRUE;
			}
			if (u.uconduct.wisharti > artwishes) {
				/* made artifact wish */
				if (mtmp2) {
					pline("You feel %s presence fade.", s_suffix(mon_nam(mtmp2)));
					u.uevent.utook_castle |= ARTWISH_SPENT;
				}
				else if (mtmp3) {
					pline("You feel %s presence fade.", s_suffix(mon_nam(mtmp3)));
					u.uevent.uunknowngod |= ARTWISH_SPENT;
				}
			}
			mongone(mtmp);
			if (mtmp2)	mongone(mtmp2);
			if (mtmp3)	mongone(mtmp3);

			if (!objects[RIN_WISHES].oc_name_known) {
				makeknown(RIN_WISHES);
				more_experienced(0, 10);
			}
		}
	}
	else
	{
		pline1(nothing_happens);
	}
	if (obj->spe <= 0)
	{
		pline("The ring crumbles to dust!");
		useupall(obj);
	}
	return madewish;
}

#define SUMMON_DJINNI		1
#define SUMMON_SERVANT		2
#define SUMMON_DEMON_LORD	3

int
do_candle_menu()
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

	Sprintf(buf, "Summon Djinni");
	any.a_int = SUMMON_DJINNI;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);
	incntlet = (incntlet != 'z') ? (incntlet + 1) : 'A';

	Sprintf(buf, "Summon Servant");
	any.a_int = SUMMON_SERVANT;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);
	incntlet = (incntlet != 'z') ? (incntlet + 1) : 'A';

	Sprintf(buf, "Summon Demon Lord");
	any.a_int = SUMMON_DEMON_LORD;	/* must be non-zero */
	add_menu(tmpwin, NO_GLYPH, &any,
		incntlet, 0, ATR_NONE, buf,
		MENU_UNSELECTED);
	incntlet = (incntlet != 'z') ? (incntlet + 1) : 'A';

	end_menu(tmpwin, "What creature do you wish to summon?");

	how = PICK_ONE;
	do{
		n = select_menu(tmpwin, how, &selected);
	} while (n <= 0);
	destroy_nhwindow(tmpwin);
	return selected[0].item.a_int;
}

int
do_demon_lord_summon_menu()
{
	winid tmpwin;
	int n, how, i;
	char buf[BUFSZ];
	char incntlet = 'a';
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */

	for (i = 0; i >= LOW_PM && i<SPECIAL_PM; i++)
	{
		if (is_dlord(&mons[i]) && type_is_pname(&mons[i]) && !(mvitals[i].mvflags & G_EXTINCT))
		{
			Sprintf(buf, "%s", mons[i].mname);
			any.a_int = i;	/* must be non-zero */
			add_menu(tmpwin, NO_GLYPH, &any,
				incntlet, 0, ATR_NONE, buf,
				MENU_UNSELECTED);
			incntlet = (incntlet != 'z') ? (incntlet + 1) : 'A';
		}
	}

	end_menu(tmpwin, "Which demon do you wish to summon?");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return (n > 0) ? selected[0].item.a_int : 0;
}

boolean
use_candle_of_invocation(obj)	// incomplete
struct obj *obj;
{
	int choice = 0;
	boolean consumed = FALSE;
	struct monst * mtmp = (struct monst *)0;

	if (obj->otyp != CANDLE_OF_INVOCATION)
	{
		impossible("object other than candle of invocation passed to use_candle_of_invocation");
		return FALSE;
	}
	if (!obj->lamplit) {
		pline1(nothing_happens);
		return FALSE;
	}

	pline("The %s flares, and a planar gate opens!", xname(obj));
	do{
		choice = do_candle_menu();

		switch (choice)
		{
		case SUMMON_DJINNI:
			if (!(mtmp = makemon(&mons[PM_DJINNI], u.ux, u.uy, NO_MM_FLAGS))){
				pline("Nothing responds to the call.");
				consumed = FALSE;
				break;
			}
			else {
				struct monst *mtmp2 = (struct monst*)0;
				struct monst *mtmp3 = (struct monst*)0;
				if ((u.uevent.utook_castle & ARTWISH_EARNED) && !(u.uevent.utook_castle & ARTWISH_SPENT))
					mtmp2 = makemon(&mons[PM_PSYCHOPOMP], u.ux, u.uy, NO_MM_FLAGS);
				if ((u.uevent.uunknowngod & ARTWISH_EARNED) && !(u.uevent.uunknowngod & ARTWISH_SPENT))
					mtmp3 = makemon(&mons[PM_PRIEST_OF_AN_UNKNOWN_GOD], u.ux, u.uy, NO_MM_FLAGS);

				if (!Blind) {
					pline("%s passes through the gate in a cloud of smoke!", Amonnam(mtmp));
					if (mtmp2 || mtmp3)
						pline("It is accompanied by %s%s%s.",
							mtmp2 ? a_monnam(mtmp2) : "",
							(mtmp2 && mtmp3) ? " and " : "",
							mtmp3 ? a_monnam(mtmp3) : "");
					pline("%s speaks.", Monnam(mtmp));
				}
				else {
					You("smell acrid fumes.");
					pline("%s speaks.", Something);
				}
				verbalize("You have summoned me.  I will grant one wish!");
				int artwishes = u.uconduct.wisharti;
				makewish(allow_artwish() | WISH_VERBOSE);
				if (u.uconduct.wisharti > artwishes) {
					/* made artifact wish */
					if (mtmp2) {
						pline("You feel %s presence fade.", s_suffix(mon_nam(mtmp2)));
						u.uevent.utook_castle |= ARTWISH_SPENT;
					}
					else if (mtmp3) {
						pline("You feel %s presence fade.", s_suffix(mon_nam(mtmp3)));
						u.uevent.uunknowngod |= ARTWISH_SPENT;
					}
				}
				mongone(mtmp);
				if (mtmp2)	mongone(mtmp2);
				if (mtmp3)	mongone(mtmp3);
				consumed = TRUE;
			}
			break;
		case SUMMON_SERVANT:
			mtmp = create_particular(MT_DOMESTIC, 0, FALSE, MA_MINION | MA_DEMON | MA_FEY | MA_PRIMORDIAL, MG_NOWISH | MG_NOTAME, G_UNIQ);
			if (!mtmp) {
				pline("Perhaps try summoning something else?");
				consumed = FALSE;
			}
			else {
				pline("The gate closes as %s passes through.", a_monnam(mtmp));
				consumed = TRUE;
			}
			break;
		case SUMMON_DEMON_LORD:
			choice = do_demon_lord_summon_menu();
			if (!choice){
				// the player didn't choose an option
				consumed = FALSE;
				break;
			}
			if (!(mtmp = makemon(&mons[choice], u.ux, u.uy, NO_MM_FLAGS))){
				// the demon was already generated
				pline("Nothing responds to the call.");
				consumed = FALSE;
				break;
			}
			else {
				if (!Blind) {
					pline("%s passes through the gate.", Monnam(mtmp));
				}
				else {
					You_feel("a hostile presence.");
				}
				mtmp->mpeaceful = 0;
				consumed = TRUE;
			}
			break;
		default:
			consumed = FALSE;
			break;
		}
	} while (!consumed);
	pline("The %s is consumed.", xname(obj));
	useupall(obj);
	if (!objects[CANDLE_OF_INVOCATION].oc_name_known) {
		makeknown(CANDLE_OF_INVOCATION);
		more_experienced(0, 10);
	}
	return TRUE;
}

#undef SUMMON_DJINNI
#undef SUMMON_SERVANT
#undef SUMMON_DEMON_LORD

#define BY_OBJECT	((struct monst *)0)

/* return 1 if the wand is broken, hence some time elapsed */
STATIC_OVL int
do_break_wand(obj)
    struct obj *obj;
{
    static const char nothing_else_happens[] = "But nothing else happens...";
    register int i, x, y;
    register struct monst *mon;
    int dmg, damage;
    boolean affects_objects, is_fragile;
    boolean shop_damage = FALSE;
    int expltype = EXPL_MAGICAL;
    char confirm[QBUFSZ], the_wand[BUFSZ], buf[BUFSZ];

    Strcpy(the_wand, yname(obj));
    Sprintf(confirm, "Are you really sure you want to break %s?",
	safe_qbuf("", sizeof("Are you really sure you want to break ?"),
				the_wand, ysimple_name(obj), "the wand"));
    if (yn(confirm) == 'n' ) return 0;

    is_fragile = (!strcmp(OBJ_DESCR(objects[obj->otyp]), "balsa"));

    if (nolimbs(youracedata)) {
	You_cant("break %s without limbs!", the_wand);
	return 0;
    } else if (obj->oartifact || ACURR(A_STR) < (is_fragile ? 5 : 10)) {
	You("don't have the strength to break %s!", the_wand);
	return 0;
    }
    pline("Raising %s high above your %s, you %s it in two!",
	  the_wand, body_part(HEAD), is_fragile ? "snap" : "break");

    /* [ALI] Do this first so that wand is removed from bill. Otherwise,
     * the freeinv() below also hides it from setpaid() which causes problems.
     */
    if (obj->unpaid) {
	check_unpaid(obj);		/* Extra charge for use */
	bill_dummy_object(obj);
    }
	
	if(u.sealsActive&SEAL_ASTAROTH) unbind(SEAL_ASTAROTH, TRUE);

    current_wand = obj;		/* destroy_item might reset this */
    freeinv(obj);		/* hide it from destroy_item instead... */
    setnotworn(obj);		/* so we need to do this ourselves */

    if (obj->spe <= 0) {
	pline("%s", nothing_else_happens);
	goto discard_broken_wand;
    }
    obj->ox = u.ux;
    obj->oy = u.uy;
    dmg = obj->spe * 4;
    affects_objects = FALSE;

    switch (obj->otyp) {
    case WAN_NOTHING:
	pline("Suddenly, and without warning, nothing happens.");
	goto discard_broken_wand;
    case WAN_WISHING:
    case WAN_LOCKING:
    case WAN_PROBING:
    case WAN_ENLIGHTENMENT:
    case WAN_SECRET_DOOR_DETECTION:
	pline("%s", nothing_else_happens);
	goto discard_broken_wand;
    case WAN_OPENING:
	/* make trap door if you broke a wand of opening */
	if ((obj->spe > 2) && rn2(obj->spe - 2) && !u.uswallow &&
	    !On_stairs(u.ux, u.uy) && (!IS_FURNITURE(levl[u.ux][u.uy].typ) &&
	    !IS_ROCK(levl[u.ux][u.uy].typ) && !closed_door(u.ux, u.uy) &&
	    !t_at(u.ux, u.uy) && Can_dig_down(&u.uz))) {
		struct trap *ttmp;
		ttmp = maketrap(u.ux, u.uy, TRAPDOOR);
		    if (ttmp) {
			ttmp->madeby_u = 1;
			ttmp->tseen = 1;
			newsym(u.ux, u.uy);
			    if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
				add_damage(u.ux, u.uy, 0L);             
			    }
		    }
	}
	goto discard_broken_wand;
    case WAN_DEATH:
    case WAN_LIGHTNING:
	dmg *= 4;
	goto wanexpl;
    case WAN_FIRE:
	expltype = EXPL_FIERY;
    case WAN_COLD:
	if (expltype == EXPL_MAGICAL) expltype = EXPL_FROSTY;
	dmg *= 2;
    case WAN_MAGIC_MISSILE:
    wanexpl:
	explode(u.ux, u.uy,
		wand_adtype(obj->otyp), WAND_CLASS, dmg, expltype, 1);
	makeknown(obj->otyp);	/* explode described the effect */
	/* make magic trap if you broke a wand of magic missile */
	if ((obj->spe > 2) && rn2(obj->spe - 2) && !u.uswallow &&
	    !On_stairs(u.ux, u.uy) && (!IS_FURNITURE(levl[u.ux][u.uy].typ) &&
	    !IS_ROCK(levl[u.ux][u.uy].typ) &&
	    !closed_door(u.ux, u.uy) && !t_at(u.ux, u.uy))) {
		struct trap *ttmp;
		ttmp = maketrap(u.ux, u.uy, MAGIC_TRAP);
		    if (ttmp) {
			ttmp->madeby_u = 1;
			ttmp->tseen = 1;
			newsym(u.ux, u.uy);
			    if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
				add_damage(u.ux, u.uy, 0L);             
			    }
		    }
	}
	goto discard_broken_wand;
    case WAN_STRIKING:
	/* we want this before the explosion instead of at the very end */
	pline("A wall of force smashes down around you!");
	dmg = d(1 + obj->spe,6);	/* normally 2d12 */
	break;
    case WAN_TELEPORTATION:
		/* WAC make tele trap if you broke a wand of teleport */
		/* But make sure the spot is valid! */
	    if ((obj->spe > 2) && rn2(obj->spe - 2) && !level.flags.noteleport &&
		    !u.uswallow && !On_stairs(u.ux, u.uy) && (!IS_FURNITURE(levl[u.ux][u.uy].typ) &&
		    !IS_ROCK(levl[u.ux][u.uy].typ) &&
		    !closed_door(u.ux, u.uy) && !t_at(u.ux, u.uy))) {
			struct trap *ttmp;
			ttmp = maketrap(u.ux, u.uy, TELEP_TRAP);
			if (ttmp) {
				ttmp->madeby_u = 1;
				ttmp->tseen = 1;
				newsym(u.ux, u.uy); /* if our hero happens to be invisible */
				if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
					/* shopkeeper will remove it */
					add_damage(u.ux, u.uy, 0L);             
				}
			}
		}
	affects_objects = TRUE;
	break;
	case WAN_POLYMORPH:
	/* make poly trap if you broke a wand of polymorph */
	if ((obj->spe > 2) && rn2(obj->spe - 2) && !u.uswallow &&
	    !On_stairs(u.ux, u.uy) && (!IS_FURNITURE(levl[u.ux][u.uy].typ) &&
	    !IS_ROCK(levl[u.ux][u.uy].typ) &&
	    !closed_door(u.ux, u.uy) && !t_at(u.ux, u.uy))) {
		struct trap *ttmp;
		ttmp = maketrap(u.ux, u.uy, POLY_TRAP);
		    if (ttmp) {
			ttmp->madeby_u = 1;
			ttmp->tseen = 1;
			newsym(u.ux, u.uy);
			    if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
				add_damage(u.ux, u.uy, 0L);             
			    }
		    }
	}
	affects_objects = TRUE;
	break;
	case WAN_SLEEP:
	/* make sleeping gas trap if you broke a wand of sleep */
	if ((obj->spe > 2) && rn2(obj->spe - 2) && !u.uswallow &&
	    !On_stairs(u.ux, u.uy) && (!IS_FURNITURE(levl[u.ux][u.uy].typ) &&
	    !IS_ROCK(levl[u.ux][u.uy].typ) &&
	    !closed_door(u.ux, u.uy) && !t_at(u.ux, u.uy))) {
		struct trap *ttmp;
		ttmp = maketrap(u.ux, u.uy, SLP_GAS_TRAP);
		    if (ttmp) {
			ttmp->madeby_u = 1;
			ttmp->tseen = 1;
			newsym(u.ux, u.uy);
			    if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
				add_damage(u.ux, u.uy, 0L);             
			    }
		    }
	}
	break;
    case WAN_CANCELLATION:
	/* make anti-magic trap if you broke a wand of cancellation */
	if ((obj->spe > 2) && rn2(obj->spe - 2) && !u.uswallow &&
	    !On_stairs(u.ux, u.uy) && (!IS_FURNITURE(levl[u.ux][u.uy].typ) &&
	    !IS_ROCK(levl[u.ux][u.uy].typ) &&
	    !closed_door(u.ux, u.uy) && !t_at(u.ux, u.uy))) {
		struct trap *ttmp;
		ttmp = maketrap(u.ux, u.uy, ANTI_MAGIC);
		    if (ttmp) {
			ttmp->madeby_u = 1;
			ttmp->tseen = 1;
			newsym(u.ux, u.uy);
			    if (*in_rooms(u.ux,u.uy,SHOPBASE)) {
				add_damage(u.ux, u.uy, 0L);             
			    }
		    }
	}
	affects_objects = TRUE;
	break;
    case WAN_UNDEAD_TURNING:
	affects_objects = TRUE;
	break;
    default:
	break;
    }

    /* magical explosion and its visual effect occur before specific effects */
    explode(obj->ox, obj->oy, AD_MAGM, WAND_CLASS, rnd(dmg), EXPL_MAGICAL, 1);

    /* this makes it hit us last, so that we can see the action first */
    for (i = 0; i <= 8; i++) {
	bhitpos.x = x = obj->ox + xdir[i];
	bhitpos.y = y = obj->oy + ydir[i];
	if (!isok(x,y)) continue;

	if (obj->otyp == WAN_DIGGING) {
	    if(dig_check(BY_OBJECT, FALSE, x, y)) {
		if (IS_WALL(levl[x][y].typ) || IS_DOOR(levl[x][y].typ)) {
		    /* normally, pits and holes don't anger guards, but they
		     * do if it's a wall or door that's being dug */
		    watch_dig((struct monst *)0, x, y, TRUE);
		    if (*in_rooms(x,y,SHOPBASE)) shop_damage = TRUE;
		}
		if(IS_GRAVE(levl[x][y].typ)){
			digactualhole(x, y, BY_OBJECT, PIT, FALSE, TRUE);
			dig_up_grave(x,y);
		} else{
			digactualhole(x, y, BY_OBJECT,
					  (rn2(obj->spe) < 3 || !Can_dig_down(&u.uz)) ?
					   PIT : HOLE, FALSE, TRUE);
			}
		}
	    continue;
	} else if(obj->otyp == WAN_CREATE_MONSTER) {
	    /* u.ux,u.uy creates it near you--x,y might create it in rock */
	    (void) makemon((struct permonst *)0, u.ux, u.uy, NO_MM_FLAGS);
	    continue;
	} else {
	    if (x == u.ux && y == u.uy) {
		/* teleport objects first to avoid race with tele control and
		   autopickup.  Other wand/object effects handled after
		   possible wand damage is assessed */
		if (obj->otyp == WAN_TELEPORTATION &&
		    affects_objects && level.objects[x][y]) {
		    (void) bhitpile(obj, bhito, x, y);
		    if (flags.botl) bot();		/* potion effects */
		}
		damage = zapyourself(obj, FALSE);
		if (damage) {
		    Sprintf(buf, "killed %sself by breaking a wand", uhim());
		    losehp(damage, buf, NO_KILLER_PREFIX);
		}
		if (flags.botl) bot();		/* blindness */
	    } else if ((mon = m_at(x, y)) != 0) {
		(void) bhitm(mon, obj);
	     /* if (flags.botl) bot(); */
	    }
	    if (affects_objects && level.objects[x][y]) {
		(void) bhitpile(obj, bhito, x, y);
		if (flags.botl) bot();		/* potion effects */
	    }
	}
    }

    /* Note: if player fell thru, this call is a no-op.
       Damage is handled in digactualhole in that case */
    if (shop_damage) pay_for_damage("dig into", FALSE);

    if (obj->otyp == WAN_LIGHT){
		litroom(TRUE, obj);	/* only needs to be done once */
		if(u.sealsActive&SEAL_TENEBROUS) unbind(SEAL_TENEBROUS,TRUE);
	}
    else if (obj->otyp == WAN_DARKNESS){
		litroom(FALSE, obj);	/* only needs to be done once */
	}

 discard_broken_wand:
    obj = current_wand;		/* [see dozap() and destroy_item()] */
    current_wand = 0;
    if (obj)
	delobj(obj);
    nomul(0, NULL);
    return 1;
}

STATIC_OVL boolean
uhave_graystone()
{
	register struct obj *otmp;

	for(otmp = invent; otmp; otmp = otmp->nobj)
		if(is_graystone(otmp))
			return TRUE;
	return FALSE;
}

STATIC_OVL int
do_flip_coin(obj)
struct obj *obj;
{
#ifndef GOLDOBJ
    u.ugold += obj->quan;
    dealloc_obj(obj);
#endif

    if (nohands(youracedata)) {
	pline("And how would you flip the coin without hands?");
	return 0;
    } else if (!freehand()) {
	You("need at least one free %s.", body_part(HAND));
	return 0;
    } else if (Underwater) {
	pline("This coin wasn't designed to be flipped underwater.");
	return 0;
    }

    You("flip %s coin.",
#ifndef GOLDOBJ
	(u.ugold > 1)
#else
	(obj->quan > 1)
#endif
	? "a" : "the");

    if (!Fumbling && !Glib && !Blind &&
	((ACURR(A_DEX) + Luck) > 0) && rn2((ACURR(A_DEX) + Luck))) {
	xchar ht = rn2(2);
	pline("%s.", ht ? "Heads" : "Tails");
	if (Hallucination && ht && !rn2(8))
	    pline("Oh my, it %s at you!", rn2(2) ? "grins" : "winks");
    } else {
	struct obj *gold;
	You("try to catch the coin but it slips from your %s.",
	    makeplural(body_part(HAND)));
#ifndef GOLDOBJ
	gold = mkgoldobj(1);
#else
	if (obj->quan > 1) gold = splitobj(obj, 1L);
	else gold = obj;
#endif
	dropx(gold);
    }
    return 1;
}

STATIC_OVL void
add_class(cl, class)
char *cl;
char class;
{
	char tmp[2];
	tmp[0] = class;
	tmp[1] = '\0';
	Strcat(cl, tmp);
}

const int carveTurns[6] = {3,4,2,1,5,2};

int
do_carve_obj(obj)
struct obj *obj;
{
	int rune;
	char carveelet;
	struct obj *carvee;
	struct obj *otmp;
	multi = 0;		/* moves consumed */
	nomovemsg = (char *)0;	/* occupation end message */

	rune = pick_rune(FALSE);
	if(!rune) return 0;
	carveelet = pick_carvee();
	
	for (otmp = invent; otmp; otmp = otmp->nobj) {
		if(otmp->invlet == carveelet) break;
	}
	if(otmp) carvee = otmp;
	else return 0;
	
	if(carvee == obj){
//		pline("Your grasp of physics is appalling.");
		pline("Is this a zen thing?");
		return 0;
	}
	
	if(carvee->spe > obj->spe){
		pline("The %s is too dull to cut into the %s.", xname(obj), xname(carvee));
		return 0;
	}
	if(carvee->oward != 0 ){
		You("chip off the existing rune.");
		multi-=1;
		if(carvee->oartifact) pline("The wood heals like the rune was never there.");
		else carvee->spe -= 1;
		if(carvee->spe < -1*rn2(8)){
			You("destroyed the %s in the process.", xname(carvee));
			useup(carvee);
			return 0;
		}
	}
	multi -= carveTurns[rune-FIRST_RUNE];
	nomovemsg = "You finish carving.";;
	carvee->oward = get_wardID(rune);
	You("carve a %s into the %s.",wardDecode[decode_wardID(carvee->oward)],xname(carvee));
    u.uconduct.wardless++;
	see_monsters(); //Some magic staves grant detection, so recheck that now.
	return 1;
}

int
pick_rune(describe)
boolean describe;
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
	
	if(!(u.wardsknown & (WARD_TOUSTEFNA|WARD_DREPRUN|WARD_OTTASTAFUR|WARD_KAUPALOKI|WARD_VEIOISTAFUR|WARD_THJOFASTAFUR) )){
		You("can't think of anything to carve.");
		return 0;
	}
	Sprintf(buf, "Known Magical Staves");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	if(u.wardsknown & WARD_TOUSTEFNA){
		Sprintf(buf, "Toustefna stave");
		any.a_int = TOUSTEFNA;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_DREPRUN){
		Sprintf(buf, "Dreprun stave");
		any.a_int = DREPRUN;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_VEIOISTAFUR){
		Sprintf(buf, "Veioistafur stave");
		any.a_int = VEIOISTAFUR;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_THJOFASTAFUR){
		Sprintf(buf, "Thjofastafur stave");
		any.a_int = THJOFASTAFUR;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}

	if (!describe){
		// Describe a glyph
		Sprintf(buf, "Describe a glyph instead");
		any.a_int = -1;					/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'?', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	else {
		Sprintf(buf, "Carve a glyph instead");
		any.a_int = -1;					/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'!', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}

	end_menu(tmpwin, (describe) ? "Choose stave to describe:" : "Choose stave to carve:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);

	if (n > 0 && selected[0].item.a_int == -1){
		return pick_rune(!describe);
	}

	if (n > 0 && describe){
		describe_rune(selected[0].item.a_int);
		return pick_rune(describe);
	}
	if (n > 0 && !describe){
		return selected[0].item.a_int;
	}

	return 0;
}


void
describe_rune(floorID)
int floorID;
{
	winid datawin;
	char name[80];
	char turns[80];
	char warded[80];
	char reinforce[80];
	char secondary[80];

	switch (floorID){
	case TOUSTEFNA:
		strcpy(name, " Toustefna stave");
		strcpy(turns, " 3 turns");
		strcpy(warded, " d, f");
		strcpy(secondary, " None.");
		break;
	case DREPRUN:
		strcpy(name, " Dreprun stave");
		strcpy(turns, " 4 turns");
		strcpy(warded, " q, u, bats, birds");
		strcpy(secondary, " None.");
		break;
	case VEIOISTAFUR:
		strcpy(name, " Veioistafur stave");
		strcpy(turns, " 5 turns");
		strcpy(warded, " ;");
		strcpy(secondary, " Bonus d20 damage vs ; when carved onto wielded weapon.");
		break;
	case THJOFASTAFUR:
		strcpy(name, " Thjofastafur stave");
		strcpy(turns, " 2 turns");
		strcpy(warded, " n, l");
		strcpy(secondary, " Grants detection of nymphs and leprechauns while wielded.");
		break;
	default:
		impossible("No such stave to carve: %d", floorID);
		return;
	}

	datawin = create_nhwindow(NHW_TEXT);
	putstr(datawin, 0, "");
	putstr(datawin, 0, name);
	putstr(datawin, 0, "");
	putstr(datawin, 0, " Turns to carve:");
	putstr(datawin, 0, turns);
	putstr(datawin, 0, "");
	putstr(datawin, 0, " Warded creatures:");
	putstr(datawin, 0, warded);
	putstr(datawin, 0, "");
	putstr(datawin, 0, " Secondary effects:");
	putstr(datawin, 0, secondary);
	putstr(datawin, 0, "");
	display_nhwindow(datawin, FALSE);
	destroy_nhwindow(datawin);
	return;
}

char
pick_carvee()
{
	winid tmpwin;
	int n=0, how,count=0;
	char buf[BUFSZ];
	struct obj *otmp;
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */
	
	Sprintf(buf, "Carvable items");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	for(otmp = invent; otmp; otmp = otmp->nobj){
		if(otmp->oclass == WEAPON_CLASS && otmp->obj_material == WOOD && otmp->otyp != MOON_AXE && otmp->oartifact != ART_BOW_OF_SKADI){
			Sprintf1(buf, doname(otmp));
			any.a_char = otmp->invlet;	/* must be non-zero */
			add_menu(tmpwin, NO_GLYPH, &any,
				otmp->invlet, 0, ATR_NONE, buf,
				MENU_UNSELECTED);
			count++;
		}
	}
	end_menu(tmpwin, "Choose ward:");

	how = PICK_ONE;
	if(count) n = select_menu(tmpwin, how, &selected);
	else You("don't have any carvable items.");
	destroy_nhwindow(tmpwin);
	return ( n > 0 ) ? selected[0].item.a_char : 0;
}

STATIC_OVL boolean
clockwork_location_checks(obj, cc, quietly)
struct obj *obj;
coord *cc;
boolean quietly;
{
	xchar x,y;

	if (carried(obj) && u.uswallow) {
		if (!quietly)
			You("don't have enough room in here to build a clockwork.");
		return FALSE;
	}
	x = cc->x; y = cc->y;
	if (!isok(x,y)) {
		if (!quietly)
			You("cannot build a clockwork there.");
		return FALSE;
	}
	if (IS_ROCK(levl[x][y].typ) &&
	    !(species_passes_walls(&mons[obj->corpsenm]) && may_passwall(x,y))) {
		if (!quietly)
		    You("cannot build a clockwork in %s!",
			IS_TREES(levl[x][y].typ) ? "a tree" : "solid rock");
		return FALSE;
	}
	if (boulder_at(x,y) && !species_passes_walls(&mons[obj->corpsenm])
			&& !throws_rocks(&mons[obj->corpsenm])) {
		if (!quietly)
			You("cannot fit a clockwork under the %s.",xname(boulder_at(x,y)));
		return FALSE;
	}
	if (m_at(x,y)) {
		if (!quietly)
			You("cannot fit a clockwork there!");
		return FALSE;
	}
	return TRUE;
}

STATIC_OVL struct permonst *
clockworkMenu(obj)
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
	
	Sprintf(buf, "Clockwork types");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	if(obj->otyp == CLOCKWORK_COMPONENT){
		Sprintf(buf, "clockwork soldier");
		any.a_int = PM_CLOCKWORK_SOLDIER;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
		
		Sprintf(buf, "clockwork dwarf");
		any.a_int = PM_CLOCKWORK_DWARF;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
		
		Sprintf(buf, "faberge sphere");
		any.a_int = PM_FABERGE_SPHERE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	} else if(obj->otyp == SUBETHAIC_COMPONENT){
		Sprintf(buf, "golden heart");
		any.a_int = PM_GOLDEN_HEART;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	} else if(obj->otyp == HELLFIRE_COMPONENT){
		Sprintf(buf, "hellfire orb");
		any.a_int = PM_HELLFIRE_ORB;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	end_menu(tmpwin, "Choose type:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return ( n > 0 ) ? &mons[selected[0].item.a_int] : (struct permonst *) 0;
}

STATIC_OVL int
doUseComponents(optr)
struct obj **optr;
{
	struct obj *obj = *optr;
	struct monst *mm;
	struct permonst *pmm;
	xchar x, y;
	coord cc;

	if(!getdir((char *)0)) {
		flags.move = multi = 0;
		return 0;
	}
	if(u.dx || u.dy || u.dz){
		if (u.uswallow) {
			/* can't activate a figurine while swallowed */
			if (!clockwork_location_checks(obj, (coord *)0, FALSE))
				return 0;
		}
		x = u.ux + u.dx; y = u.uy + u.dy;
		if((mm = m_at(x,y))){
			if(is_clockwork(mm->data)){
				if(obj->otyp == CLOCKWORK_COMPONENT){
					if(	mm->data != &mons[PM_GOLDEN_HEART] && 
						mm->data != &mons[PM_ID_JUGGERNAUT] && 
						mm->data != &mons[PM_HELLFIRE_ORB] && 
						mm->data != &mons[PM_HELLFIRE_COLOSSUS]
					){
						if(mm->mhp < mm->mhpmax){
							if(yn("Repair it?") == 'y'){
								mm->mhp += mm->m_lev;
								if(mm->mhp > mm->mhpmax) mm->mhp = mm->mhpmax;
								useup(obj);
								*optr = 0;
							}
						} else pline("It doesn't need repairs.");
					}
				} else if(obj->otyp == HELLFIRE_COMPONENT && (
					mm->data == &mons[PM_HELLFIRE_ORB] || 
					mm->data == &mons[PM_HELLFIRE_COLOSSUS] || 
					mm->data == &mons[PM_SCRAP_TITAN]
				)){
					if(mm->mhp < mm->mhpmax){
						if(yn("Repair it?") == 'y'){
							mm->mhp += mm->m_lev;
							if(mm->mhp > mm->mhpmax) mm->mhp = mm->mhpmax;
							useup(obj);
							*optr = 0;
						}
					} else pline("It doesn't need repairs.");
				} else if(obj->otyp == SUBETHAIC_COMPONENT && (
					mm->data == &mons[PM_GOLDEN_HEART] || 
					mm->data == &mons[PM_ID_JUGGERNAUT] || 
					mm->data == &mons[PM_SCRAP_TITAN]
				)){
					if(mm->mhp < mm->mhpmax){
						if(yn("Repair it?") == 'y'){
							mm->mhp += mm->m_lev;
							if(mm->mhp > mm->mhpmax) mm->mhp = mm->mhpmax;
							useup(obj);
							*optr = 0;
						}
					} else pline("It doesn't need repairs.");
				} else pline("This device can't take this part.");
			} else pline("It isn't made of clockwork.");
		}
		else {
			pline("You don't see anything there.");
		}
		cc.x = x; cc.y = y;
		/* Passing FALSE arg here will result in messages displayed */
		
		if(obj->quan < 10 && obj->otyp != SCRAP){
			You("don't have enough components to build a clockwork servant");
			return 0;
		}
		if(obj->otyp == SCRAP) return 0;
		pmm = clockworkMenu(obj);
		if(!pmm) return 0;
		obj->quan -= 9;
		if (!clockwork_location_checks(obj, &cc, FALSE)) return 0;
		You("build a clockwork and %s.",
			(u.dx||u.dy) ? "set it beside you" :
			(Weightless || Is_waterlevel(&u.uz) ||
			 is_pool(cc.x, cc.y, TRUE)) ?
			"release it" :
			(u.dz < 0 ?
			"toss it into the air" :
			"set it on the ground"));
		
		mm = makemon(pmm, u.ux+u.dx, u.uy+u.dy, MM_EDOG|MM_ADJACENTOK|NO_MINVENT|MM_NOCOUNTBIRTH);
		if(mm){
			initedog(mm);
			mm->m_lev = u.ulevel / 2 + 1;
			mm->mhpmax = (mm->m_lev * 8) - 4;
			mm->mhp =  mm->mhpmax;
			mm->mtame = 10;
			mm->mpeaceful = 1;
			if((u.dx || u.dy) && (mm->data == &mons[PM_CLOCKWORK_SOLDIER] || mm->data == &mons[PM_CLOCKWORK_DWARF] || 
				mm->data == &mons[PM_FABERGE_SPHERE] || mm->data == &mons[PM_FIREWORK_CART] || 
				mm->data == &mons[PM_JUGGERNAUT] || mm->data == &mons[PM_ID_JUGGERNAUT])
			){
				mm->mvar1 = -1;
				while(xdir[(int)(++mm->mvar1)] != u.dx || ydir[(int)mm->mvar1] != u.dy);
			}
		}
		useup(obj);
		*optr = 0;
	} else {
		if(umechanoid){
			if(Upolyd && u.mh < u.mhmax) u.mh = min(u.mhmax,u.mh+mons[u.umonnum].mlevel);
			else if(!Upolyd && u.uhp < u.uhpmax) u.uhp = min(u.uhpmax,u.uhp + u.ulevel);
			else {
				You("don't need repairs.");
				return 0;
			}
			useup(obj);
			*optr = 0;
		} else {
			You("aren't made of clockwork!");
			return 0;
		}
	}
	return 1;
}

STATIC_OVL long
upgradeMenu()
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
	
	Sprintf(buf, "Upgrade types");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	if(!(u.clockworkUpgrades&OIL_STOVE)){
		Sprintf(buf, "oil stove");
		any.a_int = 1;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(!(u.clockworkUpgrades&WOOD_STOVE) && u.clockworkUpgrades&OIL_STOVE){
		Sprintf(buf, "wood stove");
		any.a_int = 2;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(!(u.clockworkUpgrades&FAST_SWITCH)){
		Sprintf(buf, "fast speed switch");
		any.a_int = 3;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(!(u.clockworkUpgrades&EFFICIENT_SWITCH)){
		Sprintf(buf, "efficient speed switch");
		any.a_int = 4;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(!(u.clockworkUpgrades&ARMOR_PLATING)){
		Sprintf(buf, "armor plating");
		any.a_int = 5;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(!(u.clockworkUpgrades&PHASE_ENGINE) && !flags.beginner){
		Sprintf(buf, "phase engine");
		any.a_int = 6;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(!(u.clockworkUpgrades&MAGIC_FURNACE) && u.clockworkUpgrades&OIL_STOVE && !flags.beginner){
		Sprintf(buf, "magic furnace");
		any.a_int = 7;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(!(u.clockworkUpgrades&HELLFIRE_FURNACE) && u.clockworkUpgrades&OIL_STOVE && !flags.beginner){
		Sprintf(buf, "hellfire furnace");
		any.a_int = 8;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(!(u.clockworkUpgrades&SCRAP_MAW && !flags.beginner)){
		Sprintf(buf, "scrap maw");
		any.a_int = 9;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(!(u.clockworkUpgrades&HIGH_TENSION)){
		Sprintf(buf, "high-tension spring");
		any.a_int = 10;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	end_menu(tmpwin, "Choose type:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return ( n > 0 ) ? 0x1L<<(selected[0].item.a_int - 1) :  0;
}

STATIC_OVL int
resizeArmor()
{
	struct obj *otmp;
	struct permonst *ptr;
	struct monst *mtmp;
	int rx, ry;
	
    if (!getdir("Resize armor to fit what creature? (in what direction)")) {
		/* decided not to */
		return 0;
	}

#ifdef STEED
    if (u.usteed && u.dz > 0) ptr = u.usteed->data;
	else 
#endif
	if(u.dz){
		pline("No creature there.");
		return 0;
	} else if (u.dx == 0 && u.dy == 0) {
		ptr = youracedata;
    } else {
		rx = u.ux+u.dx; ry = u.uy+u.dy;
		mtmp = m_at(rx, ry);
		if(!mtmp){
			pline("No creature there.");
			return 0;
		}
		ptr = mtmp->data;
	}
	
	
	// attempt to find a piece of armor to resize
	NEARDATA const char clothes[] = { ARMOR_CLASS, 0 };
	otmp = getobj(clothes, "resize");
	if (!otmp) return(0);

	// check that the armor is not currently being worn
	if (otmp->owornmask){
		You("are wearing that!");
		return(0);
	}
	// check that the armor is not dragon scales (which cannot be resized)
	if (Is_dragon_scales(otmp)){
		pline("Dragon scales cannot be resized.");
		return(0);
	}

	// change size
	otmp->objsize = ptr->msize;
	// change shape
	if (is_shirt(otmp) || otmp->otyp == ELVEN_TOGA) otmp->bodytypeflag = (ptr->mflagsb&MB_HUMANOID) ? MB_HUMANOID : (ptr->mflagsb&MB_BODYTYPEMASK);
	else if (is_suit(otmp)) otmp->bodytypeflag = (ptr->mflagsb&MB_BODYTYPEMASK);
	else if (is_helmet(otmp)) otmp->bodytypeflag = (ptr->mflagsb&MB_HEADMODIMASK);
	
	fix_object(otmp);
	
	You("resize the armor to fit.");
	pline("The kit is used up.");
	return(1);
}

STATIC_OVL int
doUseUpgradeKit(optr)
struct obj **optr;
{
	struct obj *obj = *optr;
	struct obj *comp;
	if(uclockwork)
		if (yn("Make an upgrade to yourself?") == 'y'){
			long upgrade = upgradeMenu();
			switch(upgrade){
				case OIL_STOVE:
					You("use the components in the upgrade kit to install an oil stove.");
					u.clockworkUpgrades |= upgrade;
					useup(obj);
					*optr = 0;
					return 1;
				break;
				case WOOD_STOVE:
					comp = getobj(tools, "upgrade your stove with");
					if(!comp || comp->otyp != TINNING_KIT){
						pline("Never mind.");
						return 0;
					}
					You("use the components in the upgrade kit and the tinning kit to install a wood-burning stove.");
					u.clockworkUpgrades |= upgrade;
					useup(comp);
					useup(obj);
					*optr = 0;
					return 1;
				break;
				case FAST_SWITCH:
					You("use the components in the upgrade kit to install a fast switch on your clock.");
					u.clockworkUpgrades |= upgrade;
					useup(obj);
					*optr = 0;
					return 1;
				break;
				case EFFICIENT_SWITCH:
					comp = getobj(tools, "upgrade your switch with");
					if(!comp || comp->otyp != CROSSBOW){
						pline("Never mind.");
						return 0;
					}
					You("use the components in the upgrade kit and the crossbow to upgrade the switch on your clock.");
					u.clockworkUpgrades |= upgrade;
					useup(comp);
					useup(obj);
					*optr = 0;
					return 1;
				break;
				case ARMOR_PLATING:
					comp = getobj(apply_armor, "upgrade your armor with");
					if(!comp ||
						!((comp->otyp == ARCHAIC_PLATE_MAIL || comp->otyp == PLATE_MAIL) &&
						(comp->obj_material == COPPER))){
						pline("Never mind.");
						return 0;
					}
					You("use the components in the upgrade kit to reinforce your armor with bronze plates.");
					u.clockworkUpgrades |= upgrade;
					useup(comp);
					useup(obj);
					*optr = 0;
					return 1;
				break;
				case PHASE_ENGINE:
					comp = getobj(apply_all, "build a phase engine with");
					if(!comp || comp->otyp != SUBETHAIC_COMPONENT){
						pline("Never mind.");
						return 0;
					}
					You("combine the components in the upgrade kit with the subethaic component and build a phase engine.");
					u.clockworkUpgrades |= upgrade;
					useup(comp);
					useup(obj);
					*optr = 0;
					return 1;
				break;
				case MAGIC_FURNACE:
					comp = getobj(apply_corpse, "build a magic furnace with");
					if(!comp || comp->otyp != CORPSE || comp->corpsenm != PM_DISENCHANTER){
						pline("Never mind.");
						return 0;
					}
					You("combine the components in the upgrade kit with the disenchanter corpse and build a magic furnace.");
					u.clockworkUpgrades |= upgrade;
					useup(comp);
					useup(obj);
					*optr = 0;
					return 1;
				break;
				case HELLFIRE_FURNACE:
					comp = getobj(apply_all, "build a hellfire furnace with");
					if(!comp || comp->otyp != HELLFIRE_COMPONENT){
						pline("Never mind.");
						return 0;
					}
					You("combine the components in the upgrade kit with the hellfire component and build a hellfire furnace.");
					u.clockworkUpgrades |= upgrade;
					useup(comp);
					useup(obj);
					*optr = 0;
					return 1;
				break;
				case SCRAP_MAW:
					comp = getobj(tools, "build a scrap maw with");
					if(!comp || comp->otyp != SCRAP){
						pline("Never mind.");
						return 0;
					}
					You("combine the components in the upgrade kit with the scrap and build a scrap maw.");
					u.clockworkUpgrades |= upgrade;
					useup(comp);
					useup(obj);
					*optr = 0;
					return 1;
				break;
				case HIGH_TENSION:
					// Maybe one day a spring pistol or something
					// comp = getobj(tools, "build a scrap maw with");
					// if(!comp || comp->otyp != SCRAP){
						// pline("Never mind.");
						// return 0;
					// }
					You("use the components in the upgrade kit to increase the maximum tension in your mainspring.");
					u.uhungermax += DEFAULT_HMAX;
					if(u.uhungermax >= DEFAULT_HMAX*10) u.clockworkUpgrades |= upgrade;
					// useup(comp);
					useup(obj);
					*optr = 0;
					return 1;
				break;
			}
	}
	if (yn("Resize a piece of armor?") == 'y'){
		if (resizeArmor()){
			useup(obj);
			*optr = 0;
			return 1;
		}
		else
			return(0);
	}
	return 0;
}

int
doapply()
{
	struct obj *obj;
	register int res = 1;
	int wasmergable = FALSE;
	char class_list[MAXOCLASSES+2];

	if(check_capacity((char *)0)) return (0);

	if (carrying(POT_OIL) || uhave_graystone())
		Strcpy(class_list, tools_too);
	else
		Strcpy(class_list, tools);
	if (carrying(CREAM_PIE) || carrying(EUCALYPTUS_LEAF))
		add_class(class_list, FOOD_CLASS);
	if (carrying(DWARVISH_HELM) || carrying(GNOMISH_POINTY_HAT) 
		|| carrying(DROVEN_CLOAK) || carrying_art(ART_AEGIS))
		add_class(class_list, ARMOR_CLASS);


	obj = getobj(class_list, "use or apply");
	if(!obj) return 0;

	wasmergable = objects[obj->otyp].oc_merge; //Some functions leave a stale pointer here if they merge the item
	
	if (obj->oartifact && !touch_artifact(obj, &youmonst, FALSE))
	    return 1;	/* evading your grasp costs a turn; just be
			   grateful that you don't drop it as well */

	if(obj->ostolen && u.sealsActive&SEAL_ANDROMALIUS) unbind(SEAL_ANDROMALIUS, TRUE);

	if (obj->oclass == WAND_CLASS)
	    return do_break_wand(obj);
	else if (obj->oclass == COIN_CLASS)
	    return do_flip_coin(obj);
	else if (obj->oclass == RING_CLASS)
	    return do_present_ring(obj);
	else if(is_knife(obj) && !(obj->oartifact==ART_PEN_OF_THE_VOID && obj->ovar1&SEAL_MARIONETTE)) 
		return do_carve_obj(obj);
	
	if(obj->oartifact == ART_SILVER_STARLIGHT) res = do_play_instrument(obj);
	else if(obj->oartifact == ART_HOLY_MOONLIGHT_SWORD) use_lamp(obj);
	else if(obj->oartifact == ART_AEGIS) res = swap_aegis(obj);
	else if(obj->otyp == RAKUYO || obj->otyp == RAKUYO_SABER){
		return use_rakuyo(obj);
	} else if(obj->otyp == DOUBLE_FORCE_BLADE || obj->otyp == FORCE_BLADE){
		return use_force_blade(obj);
	} else switch(obj->otyp){
	case BLINDFOLD:
	case ANDROID_VISOR:
	case LENSES:
	case LIVING_MASK:
	case R_LYEHIAN_FACEPLATE:
	case MASK:
		if (obj == ublindf) {
		    if (!cursed(obj)) Blindf_off(obj);
		} else if (!ublindf)
		    Blindf_on(obj);
		else You("are already %s.",
			ublindf->otyp == TOWEL ?     "covered by a towel" :
			(ublindf->otyp == MASK || ublindf->otyp == LIVING_MASK || ublindf->otyp == R_LYEHIAN_FACEPLATE ) ? "wearing a mask" :
			(ublindf->otyp == BLINDFOLD || ublindf->otyp == ANDROID_VISOR) ? "wearing a blindfold" :
						     "wearing lenses");
		break;
	case CREAM_PIE:
		res = use_cream_pie(obj);
		break;
	case FORCE_SWORD:
		res = use_force_sword(obj);
		break;
	case FORCE_WHIP:
	case VIPERWHIP:
	case BULLWHIP:
		res = use_whip(obj);
		break;
	case GRAPPLING_HOOK:
		res = use_grapple(obj);
		break;
	case SHEPHERD_S_CROOK:
		res = use_crook(obj);
		break;
	case BOX:
	case CHEST:
	case ICE_BOX:
	case SACK:
	case BAG_OF_HOLDING:
	case OILSKIN_SACK:
		res = use_container(obj, 1);
		goto xit2; /* obj may have been destroyed */
		break;
	case BAG_OF_TRICKS:
		bagotricks(obj);
		break;
	case CAN_OF_GREASE:
		use_grease(obj);
		break;
	case LOCK_PICK:
#ifdef TOURIST
	case CREDIT_CARD:
#endif
	case SKELETON_KEY:
		res = pick_lock(obj);
		break;
	case UNIVERSAL_KEY:
		res =  pick_lock(obj);
		break;
	case PICK_AXE:
	case DWARVISH_MATTOCK:
	case SEISMIC_HAMMER:
		res = use_pick_axe(obj);
		break;
	case TINNING_KIT:
		use_tinning_kit(obj);
		break;
	case LEASH:
		use_leash(obj);
		break;
#ifdef STEED
	case SADDLE:
		res = use_saddle(obj);
		break;
#endif
	case MAGIC_WHISTLE:
		use_magic_whistle(obj);
		break;
	case WHISTLE:
		use_whistle(obj);
		break;
	case EUCALYPTUS_LEAF:
		/* MRKR: Every Australian knows that a gum leaf makes an */
		/*	 excellent whistle, especially if your pet is a  */
		/*	 tame kangaroo named Skippy.			 */
		if (obj->blessed) {
		    use_magic_whistle(obj);
		    /* sometimes the blessing will be worn off */
		    if (!rn2(49)) {
			if(obj->quan > 1) obj = splitobj(obj, 1L);

			if (!Blind) {
			    char buf[BUFSZ];

			    pline("%s %s %s.", Shk_Your(buf, obj),
				  aobjnam(obj, "glow"), hcolor("brown"));
			    obj->bknown = 1;
			}
			unbless(obj);
			obj_extract_self(obj);	/* free from inv */
			/* shouldn't merge */
			obj = hold_another_object(obj, "You drop %s!",
						  doname(obj), (const char *)0);
		    }
		} else {
		    use_whistle(obj);
		}
		break;
	case STETHOSCOPE:
		res = use_stethoscope(obj);
		break;
	case MIRROR:
		res = use_mirror(obj);
		break;
	case SPOON:
		if(Role_if(PM_CONVICT)) pline("The guards used to hand these out with our food rations.  No one was ever able to figure out why.");
		else pline("You have never in your life seen such an odd item.  You have no idea how to use it.");
	break;
	case BELL:
	case BELL_OF_OPENING:
		use_bell(&obj, FALSE);
		break;
	case CANDELABRUM_OF_INVOCATION:
		use_candelabrum(obj);
		break;
	case WAX_CANDLE:
	case TALLOW_CANDLE:
	case CANDLE_OF_INVOCATION:
	case GNOMISH_POINTY_HAT:
		use_candle(&obj);
	break;
	case BULLET_FABBER:
	if(!(Role_if(PM_ANACHRONONAUT) || Role_if(PM_TOURIST))) pline("It seems inert.");
	else {
		static const char all_count[] = { ALLOW_COUNT, WEAPON_CLASS, GEM_CLASS, 0 };
		struct obj *otmp = getobj(all_count, "feed to the fabber");
		if (!otmp) break;
		switch(otmp->otyp){
			case ROCK:
				obj_extract_self(otmp);
				otmp = poly_obj(otmp,BULLET);
				otmp = hold_another_object(otmp, u.uswallow ?
						   "Oops!  %s out of your reach!" :
						(Weightless ||
						 Is_waterlevel(&u.uz) ||
						 levl[u.ux][u.uy].typ < IRONBARS ||
						 levl[u.ux][u.uy].typ >= ICE) ?
							   "Oops!  %s away from you!" :
							   "Oops!  %s to the floor!",
							   The(aobjnam(otmp, "slip")),
							   (const char *)0);
			break;
			case SILVER_SLINGSTONE:
				obj_extract_self(otmp);
				otmp = poly_obj(otmp,SILVER_BULLET);
				otmp->quan *= 2;
				otmp->owt = weight(otmp);
				otmp = hold_another_object(otmp, u.uswallow ?
						   "Oops!  %s out of your reach!" :
						(Weightless ||
						 Is_waterlevel(&u.uz) ||
						 levl[u.ux][u.uy].typ < IRONBARS ||
						 levl[u.ux][u.uy].typ >= ICE) ?
							   "Oops!  %s away from you!" :
							   "Oops!  %s to the floor!",
							   The(aobjnam(otmp, "slip")),
							   (const char *)0);
			break;
			case FLINT:
				obj_extract_self(otmp);
				otmp = poly_obj(otmp,SHOTGUN_SHELL);
				otmp = hold_another_object(otmp, u.uswallow ?
						   "Oops!  %s out of your reach!" :
						(Weightless ||
						 Is_waterlevel(&u.uz) ||
						 levl[u.ux][u.uy].typ < IRONBARS ||
						 levl[u.ux][u.uy].typ >= ICE) ?
							   "Oops!  %s away from you!" :
							   "Oops!  %s to the floor!",
							   The(aobjnam(otmp, "slip")),
							   (const char *)0);
			break;
			case LOADSTONE:
				obj_extract_self(otmp);
				otmp = poly_obj(otmp,ROCKET);
				otmp = hold_another_object(otmp, u.uswallow ?
						   "Oops!  %s out of your reach!" :
						(Weightless ||
						 Is_waterlevel(&u.uz) ||
						 levl[u.ux][u.uy].typ < IRONBARS ||
						 levl[u.ux][u.uy].typ >= ICE) ?
							   "Oops!  %s away from you!" :
							   "Oops!  %s to the floor!",
							   The(aobjnam(otmp, "slip")),
							   (const char *)0);
			break;
			case BULLET:
				obj_extract_self(otmp);
				otmp = poly_obj(otmp,SHOTGUN_SHELL);
				otmp = hold_another_object(otmp, u.uswallow ?
						   "Oops!  %s out of your reach!" :
						(Weightless ||
						 Is_waterlevel(&u.uz) ||
						 levl[u.ux][u.uy].typ < IRONBARS ||
						 levl[u.ux][u.uy].typ >= ICE) ?
							   "Oops!  %s away from you!" :
							   "Oops!  %s to the floor!",
							   The(aobjnam(otmp, "slip")),
							   (const char *)0);
			break;
			case SHOTGUN_SHELL:
				obj_extract_self(otmp);
				otmp = poly_obj(otmp,BULLET);
				otmp = hold_another_object(otmp, u.uswallow ?
						   "Oops!  %s out of your reach!" :
						(Weightless ||
						 Is_waterlevel(&u.uz) ||
						 levl[u.ux][u.uy].typ < IRONBARS ||
						 levl[u.ux][u.uy].typ >= ICE) ?
							   "Oops!  %s away from you!" :
							   "Oops!  %s to the floor!",
							   The(aobjnam(otmp, "slip")),
							   (const char *)0);
			break;
			case SILVER_BULLET:{
				struct obj *rocket;
				rocket = mksobj(ROCKET, FALSE, FALSE);
				rocket->blessed = otmp->blessed;
				rocket->cursed = otmp->cursed;
				rocket->quan = (otmp->quan)/10;
				rocket->spe = otmp->spe;
				rocket->dknown = TRUE;
				rocket->known = otmp->known;
				rocket->bknown = otmp->bknown;
				rocket->rknown = otmp->rknown;
				rocket->sknown = otmp->sknown;
				if((otmp->quan = (otmp->quan)%10) == 0) useup(otmp);
				else otmp->owt = weight(otmp);
				rocket->owt = weight(rocket);
				rocket = hold_another_object(rocket, u.uswallow ?
						   "Oops!  %s out of your reach!" :
						(Weightless ||
						 Is_waterlevel(&u.uz) ||
						 levl[u.ux][u.uy].typ < IRONBARS ||
						 levl[u.ux][u.uy].typ >= ICE) ?
							   "Oops!  %s away from you!" :
							   "Oops!  %s to the floor!",
							   The(aobjnam(rocket, "slip")),
							   (const char *)0);
			}break;
			case ROCKET:{
				struct obj *bullets;
				bullets = mksobj(SILVER_BULLET, FALSE, FALSE);
				bullets->blessed = otmp->blessed;
				bullets->cursed = otmp->cursed;
				bullets->quan = (otmp->quan)*10;
				bullets->spe = otmp->spe;
				bullets->dknown = TRUE;
				bullets->known = otmp->known;
				bullets->bknown = otmp->bknown;
				bullets->rknown = otmp->rknown;
				bullets->sknown = otmp->sknown;
				useupall(otmp);
				bullets->owt = weight(bullets);
				bullets = hold_another_object(bullets, u.uswallow ?
						   "Oops!  %s out of your reach!" :
						(Weightless ||
						 Is_waterlevel(&u.uz) ||
						 levl[u.ux][u.uy].typ < IRONBARS ||
						 levl[u.ux][u.uy].typ >= ICE) ?
							   "Oops!  %s away from you!" :
							   "Oops!  %s to the floor!",
							   The(aobjnam(bullets, "slip")),
							   (const char *)0);
			}break;
		}
	}
	break;
	case POWER_PACK:{
		static const char all_count[] = { ALLOW_COUNT, ALL_CLASSES, 0 };
		struct obj *otmp = getobj(all_count, "charge");
		if (!otmp) break;
		if(otmp == obj){
			if(obj->quan > 1) pline("That seems rather pointless.");
			else pline("That seems rather difficult.");
		}
		You("press %s up against %s.", the(singular(obj,xname)), the(xname(otmp)));
		recharge(otmp, obj->cursed ? -1 : (obj->blessed ? 1 : 0));
		pline("%s is used up!",The(singular(obj,xname)));
		if(obj->quan>1)
			useup(obj);
		else{
			useup(obj);
			obj = 0;
		}
	}break;
	case LIGHTSABER:
  	case BEAMSWORD:
	case DOUBLE_LIGHTSABER:
		if (uwep != obj && !(u.twoweap && uswapwep == obj) && !wield_tool(obj, (const char *)0)) break;
		/* Fall through - activate via use_lamp */
		    
/* MRKR: dwarvish helms are mining helmets. 
 * They can be used as brass lanterns. 	
 * From an idea posted to RGRN by "Dr Darth"
 * Code by Malcom Ryan
 */
	case DWARVISH_HELM: 
	case OIL_LAMP:
	case MAGIC_LAMP:
	case LANTERN:
		use_lamp(obj);
	break;
	case POT_OIL:
		light_cocktail(obj);
		obj = 0; //May have been dealocated, just get rid of it
	break;
	case SHADOWLANDER_S_TORCH:
		light_torch(obj);
	break;
	case SUNROD:
		light_torch(obj);
	break;
	case TORCH:
		light_torch(obj);
	break;
#ifdef TOURIST
	case EXPENSIVE_CAMERA:
		res = use_camera(obj);
	break;
#endif
	case TOWEL:
		res = use_towel(obj);
	break;
	case CRYSTAL_BALL:
		res = use_crystal_ball(obj);
	break;
	case MAGIC_MARKER:
		res = dowrite(obj);
	break;
	case TIN_OPENER:
		if(!carrying(TIN)) {
			You("have no tin to open.");
			goto xit;
		}
		You("cannot open a tin without eating or discarding its contents.");
		if(flags.verbose)
			pline("In order to eat, use the 'e' command.");
		if(obj != uwep)
    pline("Opening the tin will be much easier if you wield the tin opener.");
		goto xit;

	case FIGURINE:
		use_figurine(&obj);
	break;
	case EFFIGY:{
	    struct obj *curo;
		if (Hallucination) You_feel("the tall leather doll take up your burdens!");
		else You_feel("like someone is helping you.");

		if(u.sealsActive&SEAL_MARIONETTE){
			unbind(SEAL_MARIONETTE,TRUE);
		}
		
		for (curo = invent; curo; curo = curo->nobj) {
#ifdef GOLDOBJ
			/* gold isn't subject to cursing and blessing */
			if (curo->oclass == COIN_CLASS) continue;
#endif
			if (curo->cursed) uncurse(curo);
		}
		if(Punished) unpunish();
		
		if(u.wimage >= 10){
			u.wimage = 0;
			makemon(&mons[PM_WEEPING_ANGEL], u.ux, u.uy, MM_ADJACENTOK|NO_MINVENT|MM_NOCOUNTBIRTH);
			if(Blind) pline("The effigy grows and turns to stone!");
			else pline("The effigy becomes a weeping angel!");
		} else if(u.umorgul){
			int i = rnd(u.umorgul);
			struct obj *frags;
			u.umorgul -= i;
			frags = mksobj(SHURIKEN, FALSE, FALSE);
			if(frags){
				frags->quan = i;
				frags->oproperties = OPROP_MORGW|OPROP_LESSW;
				frags->obj_material = METAL;
				curse(frags);
				fix_object(frags);
				pline("The effigy is pierced by %s!", 
					i==1 ? "a blade" : "blades");
				frags = hold_another_object(frags, "You drop %s!",
							  doname(frags), (const char *)0); /*shouldn't merge, but may drop*/
			}
			if(Blind) pline("The effigy bursts into flames!");
			else pline("The effigy burns with sickly flames!");
		} else {
			if(Blind) pline("The effigy bursts into flames!");
			else pline("The effigy burns with sickly flames!");
		}
		u.wimage = 0; //Sub-critical images are removed anyway.
		if(obj->quan>1)
			useup(obj);
		else{
			useup(obj);
			obj = 0;
		}
		update_inventory();
	} break;
	case UNICORN_HORN:
		use_unicorn_horn(obj);
	break;
	case FLUTE:
	case MAGIC_FLUTE:
	case TOOLED_HORN:
	case FROST_HORN:
	case FIRE_HORN:
	case HARP:
	case MAGIC_HARP:
	case BUGLE:
	case DRUM:
	case DRUM_OF_EARTHQUAKE:
		res = do_play_instrument(obj);
	break;
	case HORN_OF_PLENTY:	/* not a musical instrument */
		if (obj->spe > 0) {
		    struct obj *otmp;
		    const char *what;

		    consume_obj_charge(obj, TRUE);
		    if (!rn2(13)) {
			otmp = mkobj(POTION_CLASS, FALSE);
			if (objects[otmp->otyp].oc_magic) do {
			    otmp->otyp = rnd_class(POT_BOOZE, POT_WATER);
			} while (otmp->otyp == POT_SICKNESS);
			what = "A potion";
		    } else {
			otmp = mkobj(FOOD_CLASS, FALSE);
			if (otmp->otyp == FOOD_RATION && !rn2(7))
			    otmp->otyp = LUMP_OF_ROYAL_JELLY;
			what = "Some food";
		    }
		    pline("%s spills out.", what);
		    otmp->blessed = obj->blessed;
		    otmp->cursed = obj->cursed;
		    otmp->owt = weight(otmp);
		    otmp = hold_another_object(otmp, u.uswallow ?
				       "Oops!  %s out of your reach!" :
					(Weightless ||
					 Is_waterlevel(&u.uz) ||
					 levl[u.ux][u.uy].typ < IRONBARS ||
					 levl[u.ux][u.uy].typ >= ICE) ?
					       "Oops!  %s away from you!" :
					       "Oops!  %s to the floor!",
					       The(aobjnam(otmp, "slip")),
					       (const char *)0);
		    makeknown(HORN_OF_PLENTY);
		} else
		    pline("%s", nothing_happens);
	break;
	case LAND_MINE:
	case BEARTRAP:
		use_trap(obj);
	break;
	case DROVEN_CLOAK:
		if(obj->oartifact == ART_DARKWEAVER_S_CLOAK) res = use_darkweavers_cloak(obj);
		else res = use_droven_cloak(&obj);
	break;
	case FLINT:
	case LUCKSTONE:
	case LOADSTONE:
	case TOUCHSTONE:
		use_stone(obj);
	break;
//ifdef FIREARMS
	case SENSOR_PACK:
		res = use_sensor(obj);
	break;
	case HYPOSPRAY:
		res = use_hypospray(obj);
	break;
	case RAYGUN:
		if(obj->altmode == AD_FIRE){
			obj->altmode = AD_DEAD;
			You("set %s to kill.", yname(obj));
		} else if(obj->altmode == AD_DEAD){
			obj->altmode = AD_DISN;
			You("set %s to disintegrate.", yname(obj));
		} else if(obj->altmode == AD_DISN){
			obj->altmode = AD_SLEE;
			You("set %s to stun.", yname(obj));
		} else {
			obj->altmode = AD_FIRE;
			You("set %s to heat.", yname(obj));
		}
	break;
	case MASS_SHADOW_PISTOL:
		res = use_massblaster(obj);
	break;
	case ARM_BLASTER:
	case ASSAULT_RIFLE:
		/* Switch between WP_MODE_SINGLE, WP_MODE_BURST and WP_MODE_AUTO */

		if (obj->altmode == WP_MODE_AUTO) {
			obj->altmode = WP_MODE_BURST;
		} else if (obj->altmode == WP_MODE_BURST) {
			obj->altmode = WP_MODE_SINGLE;
		} else {
			obj->altmode = WP_MODE_AUTO;
		}
		
		You("switch %s to %s mode.", yname(obj), 
			((obj->altmode == WP_MODE_SINGLE) ? "semi-automatic" : 
			 ((obj->altmode == WP_MODE_BURST) ? "burst" :
			  "full automatic")));
		break;	
	case BFG:
		if (obj->altmode == WP_MODE_AUTO) obj-> altmode = WP_MODE_BURST;
		else obj->altmode = WP_MODE_AUTO;
		You("switch %s to %s mode.", yname(obj), 
			(obj->altmode ? "burst" : "full automatic"));
		break;
	case AUTO_SHOTGUN:
	case SUBMACHINE_GUN:
		if (obj->altmode == WP_MODE_AUTO) obj-> altmode = WP_MODE_SINGLE;
		else obj->altmode = WP_MODE_AUTO;
		You("switch %s to %s mode.", yname(obj), 
			(obj->altmode ? "semi-automatic" : "full automatic"));
		break;
	case FRAG_GRENADE:
	case GAS_GRENADE:
		if (!obj->oarmed) {
			You("arm %s.", yname(obj));
			arm_bomb(obj, TRUE);
		} else pline("It's already armed!");
		break;
	case STICK_OF_DYNAMITE:
		light_cocktail(obj);
		break;
//endif
	case CLOCKWORK_COMPONENT:
	case SUBETHAIC_COMPONENT:
	case HELLFIRE_COMPONENT:
	case SCRAP:
		res = doUseComponents(&obj);
	break;
	case UPGRADE_KIT:
		res = doUseUpgradeKit(&obj);
	break;
	default:
		/* Pole-weapons can strike at a distance */
		if (is_pole(obj)) {
			res = use_pole(obj);
			break;
		} else if (is_pick(obj) || is_axe(obj)) {
			res = use_pick_axe(obj);
			break;
		}
		pline("Sorry, I don't know how to use that.");
	xit:
		nomul(0, NULL);
		return 0;
	}
	if (res && !wasmergable && obj && obj->oartifact) arti_speak(obj);
xit2:
	nomul(0, NULL);
	return res;
}

/* Keep track of unfixable troubles for purposes of messages saying you feel
 * great.
 */
int
unfixable_trouble_count(is_horn)
	boolean is_horn;
{
	int unfixable_trbl = 0;

	if (Stoned) unfixable_trbl++;
	if (Golded) unfixable_trbl++;
	if (Strangled) unfixable_trbl++;
	if (Wounded_legs
#ifdef STEED
		    && !u.usteed
#endif
				) unfixable_trbl++;
	if (Slimed) unfixable_trbl++;
	/* lycanthropy is not desirable, but it doesn't actually make you feel
	   bad */

	/* we'll assume that intrinsic stunning from being a bat/stalker
	   doesn't make you feel bad */
	if (!is_horn) {
	    if (Confusion) unfixable_trbl++;
	    if (Sick) unfixable_trbl++;
	    if (HHallucination) unfixable_trbl++;
	    if (Vomiting) unfixable_trbl++;
	    if (HStun) unfixable_trbl++;
	}
	return unfixable_trbl;
}

//Returns 0 if this is the first time its called this round, 1 otherwise.
int
partial_action()
{
	int res = (moves == u.last_used_move) &&
	      (youmonst.movement == u.last_used_movement);
	u.last_used_move = moves;
	u.last_used_movement = youmonst.movement;
	return res;
}

#endif /* OVLB */

/*apply.c*/
