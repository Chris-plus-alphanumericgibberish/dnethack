/*	SCCS Id: @(#)youprop.h	3.4	1999/07/02	*/
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef YOUPROP_H
#define YOUPROP_H

#include "prop.h"
#include "permonst.h"
#include "mondata.h"
#include "pm.h"


/* KMH, intrinsics patch.
 * Reorganized and rewritten for >32-bit properties.
 * HXxx refers to intrinsic bitfields while in human form.
 * EXxx refers to extrinsic bitfields from worn objects.
 * BXxx refers to the cause of the property being blocked.
 * Xxx refers to any source, including polymorph forms.
 */


#define maybe_polyd(if_so,if_not)	(Upolyd ? (if_so) : (if_not))


/*** Resistances to troubles ***/
#define Oona_resistance	((u.oonaenergy == AD_FIRE ? Fire_resistance : u.oonaenergy == AD_COLD ? Cold_resistance : u.oonaenergy == AD_ELEC ? Shock_resistance : FALSE))
/* With intrinsics and extrinsics */
#define HFire_resistance	u.uprops[FIRE_RES].intrinsic
#define EFire_resistance	u.uprops[FIRE_RES].extrinsic
#define Fire_resistance		(HFire_resistance || EFire_resistance || \
				 species_resists_fire(&youmonst) || (Race_if(PM_HALF_DRAGON) && flags.HDbreath == AD_FIRE) ||\
				 ward_at(u.ux,u.uy) == SIGIL_OF_CTHUGHA || u.sealsActive&SEAL_FAFNIR)
#define InvFire_resistance	(EFire_resistance || ward_at(u.ux,u.uy) == SIGIL_OF_CTHUGHA)

#define HCold_resistance	u.uprops[COLD_RES].intrinsic
#define ECold_resistance	u.uprops[COLD_RES].extrinsic
#define NCold_resistance		((Race_if(PM_HALF_DRAGON) && flags.HDbreath == AD_COLD) ||\
				 species_resists_cold(&youmonst) || \
				 ward_at(u.ux,u.uy) == BRAND_OF_ITHAQUA || u.sealsActive&SEAL_AMON)
#define Cold_resistance		(HCold_resistance || ECold_resistance || NCold_resistance)
#define InvCold_resistance	(ECold_resistance || ward_at(u.ux,u.uy) == BRAND_OF_ITHAQUA)

#define HSleep_resistance	u.uprops[SLEEP_RES].intrinsic
#define ESleep_resistance	u.uprops[SLEEP_RES].extrinsic
#define Sleep_resistance	(HSleep_resistance || ESleep_resistance || \
				 (Race_if(PM_HALF_DRAGON) && flags.HDbreath == AD_SLEE) || \
				 species_resists_sleep(&youmonst) || u.sealsActive&SEAL_CHUPOCLOPS)

#define HDisint_resistance	u.uprops[DISINT_RES].intrinsic
#define EDisint_resistance	u.uprops[DISINT_RES].extrinsic
#define Disint_resistance	(HDisint_resistance || EDisint_resistance || \
				 species_resists_disint(&youmonst) || u.sealsActive&SEAL_ORTHOS)

#define HShock_resistance	u.uprops[SHOCK_RES].intrinsic
#define EShock_resistance	u.uprops[SHOCK_RES].extrinsic
#define Shock_resistance	(HShock_resistance || EShock_resistance || \
				 species_resists_elec(&youmonst) || (Race_if(PM_HALF_DRAGON) && flags.HDbreath == AD_ELEC) ||\
				 ward_at(u.ux,u.uy) == TRACERY_OF_KARAKAL || u.sealsActive&SEAL_ASTAROTH)
#define InvShock_resistance	(EShock_resistance || ward_at(u.ux,u.uy) == TRACERY_OF_KARAKAL || (HShock_resistance&FROMRACE && Race_if(PM_ANDROID)))

#define HPoison_resistance	u.uprops[POISON_RES].intrinsic
#define EPoison_resistance	u.uprops[POISON_RES].extrinsic
#define Poison_resistance	(HPoison_resistance || EPoison_resistance || \
				 species_resists_poison(&youmonst) || (Race_if(PM_HALF_DRAGON) && flags.HDbreath == AD_DRST) ||\
				 (ward_at(u.ux,u.uy) == WINGS_OF_GARUDA && num_wards_at(u.ux, u.uy) > rn2(7))\
				 || u.sealsActive&SEAL_YMIR)

#define HAcid_resistance	u.uprops[ACID_RES].intrinsic
#define EAcid_resistance	u.uprops[ACID_RES].extrinsic
#define Acid_resistance		(HAcid_resistance || EAcid_resistance || (Race_if(PM_HALF_DRAGON) && flags.HDbreath == AD_ACID) ||\
							 species_resists_acid(&youmonst) || u.sealsActive&SEAL_ECHIDNA)
#define InvAcid_resistance	(EAcid_resistance)

#define HDrain_resistance	u.uprops[DRAIN_RES].intrinsic
#define EDrain_resistance	u.uprops[DRAIN_RES].extrinsic
#define Drain_resistance	(HDrain_resistance || EDrain_resistance || \
				 species_resists_drain(&youmonst) || is_undead(youracedata) || is_demon(youracedata) || is_were(youracedata) ||\
				 (ward_at(u.ux,u.uy) == CARTOUCHE_OF_THE_CAT_LORD && num_wards_at(u.ux, u.uy) >= 4 && \
					!( 	(mvitals[PM_KITTEN].mvflags & G_GENOD || mvitals[PM_KITTEN].died >= 120) && \
						(mvitals[PM_HOUSECAT].mvflags & G_GENOD || mvitals[PM_HOUSECAT].died >= 120) && \
						(mvitals[PM_LARGE_CAT].mvflags & G_GENOD || mvitals[PM_LARGE_CAT].died >= 120) \
					) \
				 ) || \
				 u.sealsActive&SEAL_TENEBROUS\
				)

#define HAntimagic		u.uprops[ANTIMAGIC].intrinsic
#define EAntimagic		u.uprops[ANTIMAGIC].extrinsic
#define Antimagic		(EAntimagic || HAntimagic ||\
						(u.sealsActive&SEAL_MOTHER) ||\
						(u.usteed && u.usteed->misc_worn_check & W_SADDLE \
						&& which_armor(u.usteed, W_SADDLE)->oartifact == ART_HELLRIDER_S_SADDLE) || \
						Nullmagic ||\
				 (Upolyd && resists_magm(&youmonst)))

#define HNullmagic		u.uprops[NULLMAGIC].intrinsic
#define ENullmagic		u.uprops[NULLMAGIC].extrinsic
#define Nullmagic		(ENullmagic || HNullmagic)

#define HStone_resistance	u.uprops[STONE_RES].intrinsic
#define EStone_resistance	u.uprops[STONE_RES].extrinsic
#define Stone_resistance	(HStone_resistance || EStone_resistance ||\
							u.sealsActive&SEAL_MARIONETTE || species_resists_ston(&youmonst))

#define HSick_resistance	u.uprops[SICK_RES].intrinsic
#define ESick_resistance	u.uprops[SICK_RES].extrinsic
#define Sick_resistance		(HSick_resistance || ESick_resistance || \
				 (ward_at(u.ux,u.uy) == CARTOUCHE_OF_THE_CAT_LORD && num_wards_at(u.ux, u.uy) == 7 && \
					!( 	(mvitals[PM_KITTEN].mvflags & G_GENOD || mvitals[PM_KITTEN].died >= 120) && \
						(mvitals[PM_HOUSECAT].mvflags & G_GENOD || mvitals[PM_HOUSECAT].died >= 120) && \
						(mvitals[PM_LARGE_CAT].mvflags & G_GENOD || mvitals[PM_LARGE_CAT].died >= 120) \
					) \
				 ) || u.sealsActive&SEAL_IRIS || species_resists_sickness(&youmonst))

#define HStrangled		u.uprops[STRANGLED].intrinsic
#define EStrangled		u.uprops[STRANGLED].extrinsic
#define Strangled		(HStrangled || EStrangled)

/* Intrinsics only */
#define Invulnerable		u.uprops[INVULNERABLE].intrinsic    /* [Tom] */

/* Extrinsics only */
//None

/*** Troubles ***/
/* Pseudo-property */
#define Punished		(uball)

/* Those implemented solely as timeouts (we use just intrinsic) */
#define HStun			u.uprops[STUNNED].intrinsic
#define Stunned			((HStun || u.umonnum == PM_STALKER || \
				 youmonst.data->mlet == S_BAT) && !(u.specialSealsActive&SEAL_NUMINA))
		/* Note: birds will also be stunned */

#define HConfusion		u.uprops[CONFUSION].intrinsic
#define Confusion		(HConfusion && !(u.specialSealsActive&SEAL_NUMINA))

#define LightBlind		(((darksight(youracedata) && !Is_waterlevel(&u.uz)) && !(u.sealsActive&SEAL_AMON) &&\
							((viz_array[u.uy][u.ux]&TEMP_LIT1 && !(viz_array[u.uy][u.ux]&TEMP_DRK1))\
							|| (levl[u.ux][u.uy].lit &&\
								!(viz_array[u.uy][u.ux]&TEMP_DRK1 && !(viz_array[u.uy][u.ux]&TEMP_LIT1)) &&\
								!(viz_array[u.uy][u.ux]&TEMP_DRK2)\
						&& !(ublindf && (ublindf->otyp == LENSES || ublindf->otyp == MASK)) && !Is_waterlevel(&u.uz)))))
#define Blinded			u.uprops[BLINDED].intrinsic
#define Blindfolded		(ublindf && ublindf->otyp != LENSES && ublindf->otyp != MASK && ublindf->otyp != ANDROID_VISOR)
		/* ...means blind because of a cover */
#define NoLightBlind	((Blinded || Blindfolded || !haseyes(youracedata)) && \
		 !(u.sealsActive&SEAL_DANTALION && !((uarm && uarm->obj_material != GLASS) || (uarmu && uarmu->obj_material != GLASS))) && \
		 !(ublindf && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) && !forcesight)
// #define Blind	((Blinded || Blindfolded || !haseyes(youracedata) || LightBlind) && \
		 // !(u.sealsActive&SEAL_DANTALION && !(uarm && uarm->otyp != CRYSTAL_PLATE_MAIL)) && \
		 // !(ublindf && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) && !forcesight)
#define Blind	((Blinded || Blindfolded || !haseyes(youracedata)) && \
		 !(u.sealsActive&SEAL_DANTALION && !(uarm && uarm->otyp != CRYSTAL_PLATE_MAIL)) && \
		 !(ublindf && ublindf->oartifact == ART_EYES_OF_THE_OVERWORLD) && !forcesight)
		/* ...the Eyes operate even when you really are blind
		    or don't have any eyes */

#define Sick			u.uprops[SICK].intrinsic
#define Stoned			u.uprops[STONED].intrinsic
#define Golded			u.uprops[GOLDED].intrinsic
#define Vomiting		u.uprops[VOMITING].intrinsic
#define Glib			u.uprops[GLIB].intrinsic
#define Slimed			u.uprops[SLIMED].intrinsic	/* [Tom] */

/* Hallucination is solely a timeout; its resistance is extrinsic */
#define HHallucination		u.uprops[HALLUC].intrinsic
#define EHalluc_resistance	u.uprops[HALLUC_RES].extrinsic
#define Halluc_resistance	(EHalluc_resistance || \
							 u.sealsActive&SEAL_HUGINN_MUNINN || \
				 (Upolyd && dmgtype(youmonst.data, AD_HALU)))
#define Hallucination		(HHallucination && !Halluc_resistance)

/* Timeout, plus a worn mask */
#define HFumbling		u.uprops[FUMBLING].intrinsic
#define EFumbling		u.uprops[FUMBLING].extrinsic
#define Fumbling		(HFumbling || EFumbling)

#define HWounded_legs		u.uprops[WOUNDED_LEGS].intrinsic
#define EWounded_legs		u.uprops[WOUNDED_LEGS].extrinsic
#define Wounded_legs		((HWounded_legs || EWounded_legs))

#define HSleeping		u.uprops[SLEEPING].intrinsic
#define ESleeping		u.uprops[SLEEPING].extrinsic
#define Sleeping		(HSleeping || ESleeping)

#define HHunger			u.uprops[HUNGER].intrinsic
#define EHunger			u.uprops[HUNGER].extrinsic
#define Hunger			(HHunger || EHunger)


/*** Vision and senses ***/
#define HNormalvision		u.uprops[NORMALVISION].intrinsic
#define ENormalvision		u.uprops[NORMALVISION].extrinsic
#define Normalvision		(HNormalvision || ENormalvision || \
				 normalvision(youracedata))

#define HLowlightsight		u.uprops[LOWLIGHTSIGHT].intrinsic
#define ELowlightsight		u.uprops[LOWLIGHTSIGHT].extrinsic
#define Lowlightsight		(HLowlightsight || ELowlightsight || \
				 lowlightsight2(youracedata))

#define HElfsight		u.uprops[ELFSIGHT].intrinsic
#define EElfsight		u.uprops[ELFSIGHT].extrinsic
#define Elfsight		(HElfsight || EElfsight || \
				 lowlightsight3(youracedata))

#define HDarksight		u.uprops[DARKSIGHT].intrinsic
#define EDarksight		u.uprops[DARKSIGHT].extrinsic
#define Darksight		(HDarksight || EDarksight || \
				 darksight(youracedata))

#define HCatsight		u.uprops[CATSIGHT].intrinsic
#define ECatsight		u.uprops[CATSIGHT].extrinsic
#define Catsight		(HCatsight || ECatsight || \
				 catsight(youracedata))

#define HExtramission		u.uprops[EXTRAMISSION].intrinsic
#define EExtramission		u.uprops[EXTRAMISSION].extrinsic
#define Extramission		(HExtramission || EExtramission || \
				 extramission(youracedata) || u.sealsActive&SEAL_AMON)

#define HSee_invisible		u.uprops[SEE_INVIS].intrinsic
#define ESee_invisible		u.uprops[SEE_INVIS].extrinsic
#define See_invisible_old	(HSee_invisible || ESee_invisible || \
				 species_perceives(youracedata) || u.sealsActive&SEAL_NABERIUS)
#define See_invisible(X,Y)	(ESee_invisible || (See_invisible_old && dist2(u.ux, u.uy, X, Y)<13))

#define HTelepat		u.uprops[TELEPAT].intrinsic
#define ETelepat		u.uprops[TELEPAT].extrinsic
#define Blind_telepat		(HTelepat || ETelepat || \
				 species_is_telepathic(youracedata) || u.sealsActive&SEAL_DANTALION)
#define Unblind_telepat		(ETelepat || u.sealsActive&SEAL_DANTALION)

#define HWarning		u.uprops[WARNING].intrinsic
#define EWarning		u.uprops[WARNING].extrinsic
#define Warning			(HWarning || EWarning || u.sealsActive&SEAL_HUGINN_MUNINN)

/* Warning for a specific type of monster */
#define HWarn_of_mon		u.uprops[WARN_OF_MON].intrinsic
#define EWarn_of_mon		u.uprops[WARN_OF_MON].extrinsic
#define Warn_of_mon		(HWarn_of_mon || EWarn_of_mon || (uwep && uwep->oclass == WEAPON_CLASS && (uwep)->obj_material == WOOD && (uwep)->otyp != MOON_AXE && \
					(uwep->oward & WARD_THJOFASTAFUR)) || u.sealsActive&SEAL_PAIMON || u.sealsActive&SEAL_ANDREALPHUS)

#define HUndead_warning		u.uprops[WARN_UNDEAD].intrinsic
#define Undead_warning		(HUndead_warning || u.specialSealsActive&SEAL_ACERERAK)

#define HSearching		u.uprops[SEARCHING].intrinsic
#define ESearching		u.uprops[SEARCHING].extrinsic
#define Searching		(HSearching || ESearching || u.sealsActive&SEAL_OTIAX)

#define HClairvoyant		u.uprops[CLAIRVOYANT].intrinsic
#define EClairvoyant		u.uprops[CLAIRVOYANT].extrinsic
#define BClairvoyant		u.uprops[CLAIRVOYANT].blocked
#define Clairvoyant		((HClairvoyant || EClairvoyant || u.sealsActive&SEAL_MOTHER || \
							(uwep && uwep->oartifact == ART_HOLY_MOONLIGHT_SWORD && uwep->lamplit)\
						) && !BClairvoyant)

#define HInfravision		u.uprops[INFRAVISION].intrinsic
#define EInfravision		u.uprops[INFRAVISION].extrinsic
#define Infravision		(HInfravision || EInfravision || \
				  infravision(youracedata) || u.sealsActive&SEAL_FAFNIR)

#define HBloodsense		u.uprops[BLOODSENSE].intrinsic
#define EBloodsense		u.uprops[BLOODSENSE].extrinsic
#define Bloodsense		(HBloodsense || EBloodsense || \
				  bloodsense(youracedata))

#define HLifesense		u.uprops[LIFESENSE].intrinsic
#define ELifesense		u.uprops[LIFESENSE].extrinsic
#define Lifesense		(HLifesense || ELifesense || \
				  lifesense(youracedata))

#define HSenseall		u.uprops[SENSEALL].intrinsic
#define ESenseall		u.uprops[SENSEALL].extrinsic
#define Senseall		(HSenseall || ESenseall || \
				  senseall(youracedata))

#define HEarthsense		u.uprops[EARTHSENSE].intrinsic
#define EEarthsense		u.uprops[EARTHSENSE].extrinsic
#define Earthsense		(HEarthsense || EEarthsense || \
				  earthsense(youracedata))

#define HEcholocation		u.uprops[ECHOLOCATION].intrinsic
#define EEcholocation		u.uprops[ECHOLOCATION].extrinsic
#define Echolocation		(HEcholocation || EEcholocation || \
				  echolocation(youracedata))

#define HDetect_monsters	u.uprops[DETECT_MONSTERS].intrinsic
#define EDetect_monsters	u.uprops[DETECT_MONSTERS].extrinsic
#define Detect_monsters		(HDetect_monsters || EDetect_monsters || u.specialSealsActive&SEAL_NUMINA)

#define HWeldproof	u.uprops[WELDPROOF].intrinsic
#define EWeldproof	u.uprops[WELDPROOF].extrinsic
#define Weldproof	(HWeldproof || EWeldproof || \
					 is_demon(youracedata) || is_undead(youracedata) || (u.ulycn >= LOW_PM) || (Race_if(PM_ANDROID)) ||\
					 (u.specialSealsActive&SEAL_ACERERAK)\
					)

/*** Appearance and behavior ***/
#define Adornment		u.uprops[ADORNED].extrinsic

#define HInvis			u.uprops[INVIS].intrinsic
#define EInvis			u.uprops[INVIS].extrinsic
#define BInvis			u.uprops[INVIS].blocked
#define NoBInvis		(HInvis || EInvis || Underwater || \
						 pm_invisible(youracedata) || \
						 (ward_at(u.ux,u.uy) == HAMSA \
							&& num_wards_at(u.ux, u.uy) == 6 ) || \
						  u.sealsActive&SEAL_SHIRO || \
						  (flags.run != 0 && uwep && uwep->oartifact == ART_TOBIUME))
#define Invis			(((HInvis || EInvis || \
						 pm_invisible(youracedata) || \
						 (ward_at(u.ux,u.uy) == HAMSA \
							&& num_wards_at(u.ux, u.uy) == 6 ) || \
						  u.sealsActive&SEAL_SHIRO || \
						  (flags.run != 0 && uwep && uwep->oartifact == ART_TOBIUME)\
						  ) && !BInvis) || Underwater)
#define Invisible		(Invis && !See_invisible(u.ux,u.uy) && !Underwater)
		/* Note: invisibility also hides inventory and steed */

#define HDisplaced		u.uprops[DISPLACED].intrinsic
#define EDisplaced		u.uprops[DISPLACED].extrinsic
#define Displaced		(HDisplaced || EDisplaced || \
						 species_displaces(youracedata) || u.sealsActive&SEAL_ORTHOS || u.uvaul_duration)

#define HStealth		u.uprops[STEALTH].intrinsic
#define EStealth		u.uprops[STEALTH].extrinsic
#define BStealth		(u.uprops[STEALTH].blocked || (uwep && uwep->otyp == KHAKKHARA))
#define Stealth			((HStealth || EStealth || Underwater || u.sealsActive&SEAL_SHIRO || \
						 (flags.run != 0 && uwep && uwep->oartifact == ART_TOBIUME)) && !BStealth)

#define HAggravate_monster	u.uprops[AGGRAVATE_MONSTER].intrinsic
#define EAggravate_monster	u.uprops[AGGRAVATE_MONSTER].extrinsic
#define Aggravate_monster	(HAggravate_monster || EAggravate_monster || u.specialSealsActive&SEAL_ALIGNMENT_THING)

#define HConflict		u.uprops[CONFLICT].intrinsic
#define EConflict		u.uprops[CONFLICT].extrinsic
#define Conflict		(HConflict || EConflict)


/*** Transportation ***/
#define HJumping		u.uprops[JUMPING].intrinsic
#define EJumping		u.uprops[JUMPING].extrinsic
#define Jumping			(HJumping || EJumping || u.sealsActive&SEAL_OSE)

#define HTeleportation		u.uprops[TELEPORT].intrinsic
#define ETeleportation		u.uprops[TELEPORT].extrinsic
#define Teleportation		(HTeleportation || ETeleportation || \
							 ward_at(u.ux,u.uy) == ANDREALPHUS_TRANSIT || \
				 species_teleports(youracedata))

#define HTeleport_control	u.uprops[TELEPORT_CONTROL].intrinsic
#define ETeleport_control	u.uprops[TELEPORT_CONTROL].extrinsic
#define Teleport_control	(HTeleport_control || ETeleport_control || \
				 u.sealsActive&SEAL_ANDREALPHUS || \
				 ward_at(u.ux,u.uy) == ANDREALPHUS_STABILIZE || \
				 species_controls_teleports(youracedata))

#define HLevitation		u.uprops[LEVITATION].intrinsic
#define ELevitation		u.uprops[LEVITATION].extrinsic
#define Levitation		(HLevitation || ELevitation || \
				 species_floats(youracedata))
	/* Can't touch surface, can't go under water; overrides all others */
#define Lev_at_will		(((HLevitation & I_SPECIAL) != 0L || \
				 (ELevitation & W_ARTI) != 0L) && \
				 (HLevitation & ~(I_SPECIAL|TIMEOUT)) == 0L && \
				 (ELevitation & ~W_ARTI) == 0L && \
				 !species_floats(youracedata))

#define HFlying			u.uprops[FLYING].intrinsic
#define EFlying			u.uprops[FLYING].extrinsic
#ifdef STEED
# define Flying			(EFlying || HFlying || species_flies(youracedata) || \
				 u.ufirst_light || (u.usteed && mon_resistance(u.usteed,FLYING)))
#else
# define Flying			(EFlying || HFlying || species_flies(youracedata))
#endif
	/* May touch surface; does not override any others */

#define Wwalking		((HWwalking || EWwalking || u.sealsActive&SEAL_EURYNOME) && \
				 !is_3dwater(u.ux, u.uy))

#define HWwalking	u.uprops[WWALKING].intrinsic
#define EWwalking	u.uprops[WWALKING].extrinsic

	/* Don't get wet, can't go under water; overrides others except levitation */
	/* Wwalking is meaningless on water level */

#define HSwimming		u.uprops[SWIMMING].intrinsic
#define ESwimming		u.uprops[SWIMMING].extrinsic	/* [Tom] */
#ifdef STEED
# define Swimming	(((HSwimming || ESwimming || \
				 species_swims(youracedata) || \
				 Is_waterlevel(&u.uz) || \
				 u.sealsActive&SEAL_ENKI) && !Punished && inv_weight() < 0) || \
				 (u.usteed && mon_resistance(u.usteed,SWIMMING)))
# define NoburdSwimming	(((HSwimming || ESwimming || \
				 species_swims(youracedata) || \
				 Is_waterlevel(&u.uz) || \
				 u.sealsActive&SEAL_ENKI) && !Punished) || \
				 (u.usteed && mon_resistance(u.usteed,SWIMMING)))
#else
# define Swimming	(((HSwimming || ESwimming || \
				 species_swims(youracedata) || \
				 u.sealsActive&SEAL_ENKI) \
				 && !Punished && inv_weight() < 0) || Is_waterlevel(&u.uz))
# define NoburdSwimming	(((HSwimming || ESwimming || \
				 species_swims(youracedata) || \
				 u.sealsActive&SEAL_ENKI) \
				 && !Punished) || Is_waterlevel(&u.uz))
#endif
	/* Get wet, don't go under water unless if amphibious */

#define HMagical_breathing	u.uprops[MAGICAL_BREATHING].intrinsic
#define EMagical_breathing	u.uprops[MAGICAL_BREATHING].extrinsic
#define Amphibious		(HMagical_breathing || EMagical_breathing || \
				 u.sealsActive&SEAL_ASTAROTH || \
				 (u.divetimer > 0 && u.usubwater) || \
				 amphibious(youracedata))
	/* Get wet, may go under surface */

#define Breathless		(HMagical_breathing || EMagical_breathing || \
				 u.sealsActive&SEAL_ASTAROTH || \
				 breathless(youracedata))

//define Underwater		(u.uinwater)
#define Underwater		(u.usubwater || is_3dwater(u.ux,u.uy))
/* Note that Underwater and u.uinwater are both used in code.
   The latter form is for later implementation of other in-water
   states, like swimming, wading, etc. */

#define HPasses_walls		u.uprops[PASSES_WALLS].intrinsic
#define EPasses_walls		u.uprops[PASSES_WALLS].extrinsic
#define Passes_walls		(HPasses_walls || EPasses_walls || \
				 (uclockwork && u.phasengn)|| species_passes_walls(youracedata))
#ifdef CONVICT
# define Phasing            u.uprops[PASSES_WALLS].intrinsic
#endif /* CONVICT */

/*** Physical attributes ***/
#define HSlow_digestion		u.uprops[SLOW_DIGESTION].intrinsic
#define ESlow_digestion		u.uprops[SLOW_DIGESTION].extrinsic
#define Slow_digestion		(HSlow_digestion || ESlow_digestion)  /* KMH */

#define HHalf_spell_damage	u.uprops[HALF_SPDAM].intrinsic
#define EHalf_spell_damage	u.uprops[HALF_SPDAM].extrinsic
#define Half_spell_damage	(HHalf_spell_damage || EHalf_spell_damage || u.sealsActive&SEAL_BALAM)

#define HHalf_physical_damage	u.uprops[HALF_PHDAM].intrinsic
#define EHalf_physical_damage	u.uprops[HALF_PHDAM].extrinsic
#define Half_physical_damage	(HHalf_physical_damage || EHalf_physical_damage || u.sealsActive&SEAL_EVE)

#define HRegeneration		u.uprops[REGENERATION].intrinsic
#define ERegeneration		u.uprops[REGENERATION].extrinsic
#define Regeneration		(HRegeneration || ERegeneration || \
				 species_regenerates(youracedata))

#define HEnergy_regeneration	u.uprops[ENERGY_REGENERATION].intrinsic
#define EEnergy_regeneration	u.uprops[ENERGY_REGENERATION].extrinsic
#define Energy_regeneration	(HEnergy_regeneration || EEnergy_regeneration)

#define HProtection		u.uprops[PROTECTION].intrinsic
#define EProtection		u.uprops[PROTECTION].extrinsic
#define Protection		(HProtection || EProtection)

#define HProtection_from_shape_changers \
				u.uprops[PROT_FROM_SHAPE_CHANGERS].intrinsic
#define EProtection_from_shape_changers \
				u.uprops[PROT_FROM_SHAPE_CHANGERS].extrinsic
#define Protection_from_shape_changers \
				(HProtection_from_shape_changers || \
				 EProtection_from_shape_changers)

#define HPolymorph		u.uprops[POLYMORPH].intrinsic
#define EPolymorph		u.uprops[POLYMORPH].extrinsic
#define Polymorph		(HPolymorph || EPolymorph)

#define HPolymorph_control	u.uprops[POLYMORPH_CONTROL].intrinsic
#define EPolymorph_control	u.uprops[POLYMORPH_CONTROL].extrinsic
#define Polymorph_control	(HPolymorph_control || EPolymorph_control)

#define HUnchanging		u.uprops[UNCHANGING].intrinsic
#define EUnchanging		u.uprops[UNCHANGING].extrinsic
#define Unchanging		(HUnchanging || EUnchanging)	/* KMH */

#define HSpellboost		u.uprops[SPELLBOOST].intrinsic
#define ESpellboost		u.uprops[SPELLBOOST].extrinsic
#define Spellboost		(HSpellboost || ESpellboost || u.specialSealsActive&SEAL_NUDZIRATH)

#define HFast			u.uprops[FAST].intrinsic
#define EFast			u.uprops[FAST].extrinsic
#define Fast			(HFast || EFast)
#define Very_fast		((HFast & ~INTRINSIC) || EFast)

#define EReflecting		u.uprops[REFLECTING].extrinsic
#define Reflecting		(EReflecting || \
						 (u.sealsActive&SEAL_EDEN) || \
						 (uwep && is_lightsaber(uwep) && uwep->lamplit && ((u.fightingForm == FFORM_SORESU && (!uarm || is_light_armor(uarm) || is_medium_armor(uarm))) || (u.fightingForm == FFORM_SHIEN && (!uarm || is_light_armor(uarm))))) || \
						 (u.usteed && u.usteed->misc_worn_check & W_SADDLE \
						 && which_armor(u.usteed, W_SADDLE)->oartifact == ART_HELLRIDER_S_SADDLE) || \
						species_reflects(&youmonst))

#define EFree_action		u.uprops[FREE_ACTION].extrinsic

#define Free_action		(EFree_action || u.sealsActive&SEAL_EURYNOME) /* [Tom] */

#define Fixed_abil		(u.uprops[FIXED_ABIL].extrinsic || u.specialSealsActive&SEAL_DAHLVER_NAR)	/* KMH */

#define Lifesaved		(u.uprops[LIFESAVED].extrinsic || u.sealsActive&SEAL_JACK)

#define Necrospellboost	(u.uprops[NECROSPELLS].extrinsic)

#define Double_spell_size	(u.sealsActive&SEAL_NABERIUS)

#endif /* YOUPROP_H */
