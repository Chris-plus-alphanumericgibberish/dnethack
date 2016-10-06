/*	SCCS Id: @(#)mondata.h	3.4	2003/01/08	*/
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MONDATA_H
#define MONDATA_H

#define verysmall(ptr)		((ptr)->msize < MZ_SMALL)
#define bigmonst(ptr)		((ptr)->msize >= MZ_LARGE)

#define pm_resistance(ptr,typ)	(((ptr)->mresists & (typ)) != 0)

//#define resists_fire(mon)	(((mon)->mintrinsics & MR_FIRE) != 0 || (mon == u.usteed && u.sealsActive&SEAL_BERITH && Fire_resistance))
// #define resists_fire(mon)	(((mon)->mintrinsics & MR_FIRE) != 0)
// #define resists_cold(mon)	(((mon)->mintrinsics & MR_COLD) != 0)
// #define resists_sleep(mon)	(((mon)->mintrinsics & MR_SLEEP) != 0)
// #define resists_disint(mon)	(((mon)->mintrinsics & MR_DISINT) != 0)
// #define resists_elec(mon)	(((mon)->mintrinsics & MR_ELEC) != 0)
// #define resists_poison(mon)	(((mon)->mintrinsics & MR_POISON) != 0)
// #define resists_acid(mon)	(((mon)->mintrinsics & MR_ACID) != 0)
// #define resists_ston(mon)	(((mon)->mintrinsics & MR_STONE) != 0)
// #define resists_drain(mon)	(((mon)->mintrinsics & MR_DRAIN) != 0)
// #define resists_sickness(mon)	(((mon)->mintrinsics & MR_SICK) != 0)

#define	resist_attacks(ptr)	((ptr) == &mons[PM_HUNGRY_DEAD] || (ptr) == &mons[PM_STINKING_CLOUD] || (ptr) == &mons[PM_MORTAI])

#define resists_poly(ptr)	((ptr) == &mons[PM_OONA] || is_weeping(ptr) || is_yochlol(ptr))

#define is_blind(mon)		(!((mon)->mcansee) || (darksight((mon)->data) && (viz_array[(mon)->my][(mon)->mx]&TEMP_LIT1 || levl[(mon)->mx][(mon)->my].lit)))
#define is_deaf(mon)		(!((mon)->mcanhear))

#define is_molochan(ptr)	((ptr)->maligntyp == A_NONE)
#define is_voidalign(ptr)	((ptr)->maligntyp == A_VOID)
#define is_lawful(ptr)		((ptr)->maligntyp > A_NEUTRAL && !is_molochan(ptr) && !is_voidalign(ptr))
#define is_neutral(ptr)		((ptr)->maligntyp == A_NEUTRAL)
#define is_chaotic(ptr)		((ptr)->maligntyp < A_NEUTRAL && !is_molochan(ptr) && !is_voidalign(ptr))

#define is_lminion(mon)		(is_minion((mon)->data) && \
				 (mon)->data->maligntyp > A_NEUTRAL && \
				 ((mon)->data != &mons[PM_ANGEL] || \
				  EPRI(mon)->shralign > 0))

#define is_nminion(mon)		(is_minion((mon)->data) && \
				 (mon)->data->maligntyp == A_NEUTRAL && \
				 ((mon)->data != &mons[PM_ANGEL] || \
				  EPRI(mon)->shralign == 0))

#define is_cminion(mon)		(is_minion((mon)->data) && \
				 (mon)->data->maligntyp < A_NEUTRAL && \
				 ((mon)->data != &mons[PM_ANGEL] || \
				  EPRI(mon)->shralign < 0))

#define notonline(ptr)		(((ptr)->mflagsm & MM_NOTONL) != 0L)
#define fleetflee(ptr)		(((ptr)->mflagsm & MM_FLEETFLEE) != 0L)
#define bold(ptr)			(((ptr)->mflagst & MT_BOLD) != 0L)
#define is_flyer(ptr)		(((ptr)->mflagsm & MM_FLY) != 0L)
#define is_displacer(ptr)	(((ptr)->mflagsg & MG_DISPLACEMENT) != 0L)
#define is_floater(ptr)		(((ptr)->mflagsm & MM_FLOAT) != 0L)
#define is_clinger(ptr)		(((ptr)->mflagsm & MM_CLING) != 0L)
#define is_swimmer(ptr)		(((ptr)->mflagsm & MM_SWIM) != 0L)
#define is_suicidal(ptr)	(is_fern_spore(ptr) || ptr == &mons[PM_FREEZING_SPHERE] || ptr == &mons[PM_FLAMING_SPHERE] || ptr == &mons[PM_SHOCKING_SPHERE])
#define breathless(ptr)		(((ptr)->mflagsm & MM_BREATHLESS) != 0L)
#define amphibious(ptr)		(((ptr)->mflagsm & (MM_AMPHIBIOUS | MM_BREATHLESS)) != 0L)
#define passes_walls(ptr)	(((ptr)->mflagsm & MM_WALLWALK) != 0L)
#define amorphous(ptr)		(((ptr)->mflagsm & MM_AMORPHOUS) != 0L)
#define noncorporeal(ptr)	((ptr)->mlet == S_GHOST || (ptr)->mlet == S_SHADE)
#define tunnels(ptr)		(((ptr)->mflagsm & MM_TUNNEL) != 0L)
#define needspick(ptr)		(((ptr)->mflagsm & MM_NEEDPICK) != 0L)
#define hides_under(ptr)	(((ptr)->mflagst & MT_CONCEAL) != 0L)
#define is_hider(ptr)		(((ptr)->mflagst & MT_HIDE) != 0L)
/*#define haseyes(ptr)		(((ptr)->mflagsb & MB_NOEYES) == 0L) when did this get duplicated???*/
#define haseyes(ptr)		(((ptr)->mflagsb & MB_NOEYES) == 0L)
#define goodsmeller(ptr)	(((ptr)->mflagsv & MV_SCENT) == 0L)
#define is_tracker(ptr)		(((ptr)->mflagsg & MG_TRACKER) == 0L)
#define eyecount(ptr)		(!haseyes(ptr) ? 0 : \
				 ((ptr) == &mons[PM_CYCLOPS] || \
				  (ptr) == &mons[PM_MONOTON] || \
				  (ptr) == &mons[PM_FLOATING_EYE]) ? 1 : 2)
#define sensitive_ears(ptr)		(((ptr)->mflagsv & MV_ECHOLOCATE) != 0L)
#define nohands(ptr)		(((ptr)->mflagsb & (MB_NOHANDS|MB_NOLIMBS)) != 0L)
#define nolimbs(ptr)		(((ptr)->mflagsb & MB_NOLIMBS) == MB_NOLIMBS)
#define notake(ptr)		(((ptr)->mflagst & MT_NOTAKE) != 0L)
#define has_head(ptr)		(((ptr)->mflagsb & MB_NOHEAD) == 0L)
#define has_horns(ptr)		(num_horns(ptr) > 0)
#define is_whirly(ptr)		((ptr)->mlet == S_VORTEX || \
				 (ptr) == &mons[PM_AIR_ELEMENTAL] ||\
				 (ptr) == &mons[PM_ILLURIEN_OF_THE_MYRIAD_GLIMPSES] ||\
				 (ptr) == &mons[PM_DREADBLOSSOM_SWARM])
#define flaming(ptr)		((ptr) == &mons[PM_FIRE_VORTEX] || \
				 (ptr) == &mons[PM_FLAMING_SPHERE] || \
				 (ptr) == &mons[PM_FIRE_ELEMENTAL] || \
				 (ptr) == &mons[PM_SALAMANDER])
#define is_stone(ptr)	((ptr) == &mons[PM_DUST_VORTEX] || \
				 (ptr) == &mons[PM_EARTH_ELEMENTAL] || \
				 (ptr) == &mons[PM_TERRACOTTA_SOLDIER] || \
				 (ptr) == &mons[PM_STONE_GOLEM] || \
				 (ptr) == &mons[PM_GARGOYLE] || \
				 (ptr) == &mons[PM_WINGED_GARGOYLE] || \
				 (ptr) == &mons[PM_XORN])
#define is_anhydrous(ptr)	(flaming(ptr)  || \
							 is_clockwork(ptr) || \
							 is_stone(ptr) || \
							 is_auton(ptr) || \
				 (ptr)->mlet == S_KETER || \
				 (ptr) == &mons[PM_AOA] || \
				 (ptr) == &mons[PM_AOA_DROPLET])
#define no_innards(ptr)	((ptr)->mlet == S_VORTEX || \
						 (ptr)->mlet == S_LIGHT || \
						 (ptr)->mlet == S_ELEMENTAL || \
						 ((ptr) == &mons[PM_SHAMBLING_HORROR] && u.shambin == 2) || \
						 ((ptr) == &mons[PM_STUMBLING_HORROR] && u.stumbin == 2) || \
						 ((ptr) == &mons[PM_WANDERING_HORROR] && u.wandein == 2) || \
						 ((ptr) == &mons[PM_HUNGRY_DEAD]) || \
						 ((ptr) == &mons[PM_SKELETON]) || \
						 ((ptr) == &mons[PM_SKELETAL_PIRATE]) || \
						 (ptr)->mlet == S_WRAITH || \
						 (ptr)->mlet == S_GHOST || \
						 (ptr)->mlet == S_SHADE || \
						 (ptr)->mlet == S_GOLEM \
						)
#define undiffed_innards(ptr)	((ptr)->mlet == S_BLOB || \
								 (ptr) == &mons[PM_FLOATING_EYE] || \
								 (ptr)->mlet == S_JELLY || \
								 (ptr)->mlet == S_TRAPPER || \
								 (ptr)->mlet == S_FUNGUS || \
								 (ptr)->mlet == S_PUDDING || \
								 ((ptr) == &mons[PM_SHAMBLING_HORROR] && u.shambin == 1) || \
								 ((ptr) == &mons[PM_STUMBLING_HORROR] && u.stumbin == 1) || \
								 ((ptr) == &mons[PM_WANDERING_HORROR] && u.wandein == 1) || \
								 (ptr)->mlet == S_PLANT \
								)
#define is_silent(ptr)		((ptr)->msound == MS_SILENT)
#define unsolid(ptr)		(((ptr)->mflagsb & MB_UNSOLID) != 0L)
#define mindless(ptr)		(((ptr)->mflagst & MT_MINDLESS) != 0L || on_level(&valley_level, &u.uz))

#define slithy(ptr)			((ptr)->mflagsb & MB_SLITHY)
#define humanoid(ptr)		(((ptr)->mflagsb & MB_BODYTYPEMASK) == MB_HUMANOID)
#define animaloid(ptr)		(((ptr)->mflagsb & MB_BODYTYPEMASK) == MB_ANIMAL)
#define serpentine(ptr)		(((ptr)->mflagsb & MB_BODYTYPEMASK) == MB_SLITHY)
#define centauroid(ptr)		(((ptr)->mflagsb & MB_BODYTYPEMASK) == MB_HUMANOID|MB_ANIMAL)
#define snakemanoid(ptr)	(((ptr)->mflagsb & MB_BODYTYPEMASK) == MB_HUMANOID|MB_SLITHY)
#define leggedserpent(ptr)	(((ptr)->mflagsb & MB_BODYTYPEMASK) == MB_ANIMAL|MB_SLITHY)

#define is_animal(ptr)		(((ptr)->mflagst & MT_ANIMAL) != 0L)
#define is_plant(ptr)		(((ptr)->mflagsa & MA_PLANT) != 0L)
#define is_wooden(ptr)		((ptr) == &mons[PM_WOOD_GOLEM] || (ptr) == &mons[PM_LIVING_LECTERN] || is_plant(ptr))
#define thick_skinned(ptr)	(((ptr)->mflagsb & MB_THICK_HIDE) != 0L)
#define lays_eggs(ptr)		(((ptr)->mflagsb & MB_OVIPAROUS) != 0L)
#define regenerates(ptr)	(((ptr)->mflagsg & MG_REGEN) != 0L)
#define perceives(ptr)		(((ptr)->mflagsv & MV_SEE_INVIS) != 0L)
#define can_teleport(ptr)	(((ptr)->mflagsm & MM_TPORT) != 0L)
#define control_teleport(ptr)	(((ptr)->mflagsm & MM_TPORT_CNTRL) != 0L)
#define telepathic(ptr)		(((ptr)->mflagsv & MV_TELEPATHIC) != 0L)
#define is_armed(ptr)		attacktype(ptr, AT_WEAP)
#define crpsdanger(ptr)		(acidic(ptr) || poisonous(ptr) ||\
							 freezing(ptr) || burning(ptr))
#define acidic(ptr)			(((ptr)->mflagsb & MB_ACID) != 0L)
#define poisonous(ptr)		(((ptr)->mflagsb & MB_POIS) != 0L)
#define freezing(ptr)		(((ptr)->mflagsb & MB_CHILL) != 0L)
#define burning(ptr)		(((ptr)->mflagsb & MB_TOSTY) != 0L)
#define carnivorous(ptr)	(((ptr)->mflagst & MT_CARNIVORE) != 0L)
#define herbivorous(ptr)	(((ptr)->mflagst & MT_HERBIVORE) != 0L)
#define metallivorous(ptr)	(((ptr)->mflagst & MT_METALLIVORE) != 0L)
#define magivorous(ptr)		(((ptr)->mflagst & MT_MAGIVORE) != 0L)
#define polyok(ptr)			(((ptr)->mflagsg & MG_NOPOLY) == 0L)
#define is_Rebel(ptr)		(ptr == &mons[PM_REBEL_RINGLEADER] ||\
							 ptr == &mons[PM_ADVENTURING_WIZARD] ||\
							 ptr == &mons[PM_MILITANT_CLERIC] ||\
							 ptr == &mons[PM_HALF_ELF_RANGER])
#define is_undead(ptr)		(((ptr)->mflagsa & MA_UNDEAD) != 0L)
#define is_were(ptr)		(((ptr)->mflagsa & MA_WERE) != 0L)
#define is_eladrin(ptr)		(is_heladrin(ptr) || is_eeladrin(ptr))
#define is_heladrin(ptr)		(\
							 (ptr) == &mons[PM_COURE] || \
							 (ptr) == &mons[PM_NOVIERE] || \
							 (ptr) == &mons[PM_BRALANI] || \
							 (ptr) == &mons[PM_FIRRE] || \
							 (ptr) == &mons[PM_SHIERE] || \
							 (ptr) == &mons[PM_GHAELE] || \
							 (ptr) == &mons[PM_TULANI] || \
							 (ptr) == &mons[PM_GWYNHARWYF] ||\
							 (ptr) == &mons[PM_ASCODEL] ||\
							 (ptr) == &mons[PM_FAERINAAL] ||\
							 (ptr) == &mons[PM_QUEEN_MAB] ||\
							 (ptr) == &mons[PM_KETO] \
							)
#define is_eeladrin(ptr)	(\
							 (ptr) == &mons[PM_MOTE_OF_LIGHT] || \
							 (ptr) == &mons[PM_WATER_DOLPHIN] || \
							 (ptr) == &mons[PM_SINGING_SAND] || \
							 (ptr) == &mons[PM_DANCING_FLAME] || \
							 (ptr) == &mons[PM_BALL_OF_LIGHT] || \
							 (ptr) == &mons[PM_LUMINOUS_CLOUD] || \
							 (ptr) == &mons[PM_BALL_OF_RADIANCE] || \
							 (ptr) == &mons[PM_FURIOUS_WHIRLWIND] ||\
							 (ptr) == &mons[PM_BLOODY_SUNSET] ||\
							 (ptr) == &mons[PM_BALL_OF_GOSSAMER_SUNLIGHT] ||\
							 (ptr) == &mons[PM_COTERIE_OF_MOTES] ||\
							 (ptr) == &mons[PM_ANCIENT_TEMPEST] \
							)
#define is_yochlol(ptr)		((ptr) == &mons[PM_YOCHLOL] ||\
							 (ptr) == &mons[PM_UNEARTHLY_DROW] ||\
							 (ptr) == &mons[PM_STINKING_CLOUD] ||\
							 (ptr) == &mons[PM_DEMONIC_BLACK_WIDOW])
#define is_vampire(ptr)		(((ptr)->mflagsa & MA_VAMPIRE) != 0L)
#define is_half_dragon(ptr)		attacktype_fordmg(ptr, AT_BREA, AD_HDRG)
#define is_elf(ptr)			(((ptr)->mflagsa & MA_ELF) != 0L && !is_drow(ptr))
#define is_drow(ptr)		(((ptr)->mflagsa & MA_DROW) != 0L)
#define is_dwarf(ptr)		(((ptr)->mflagsa & MA_DWARF) != 0L)
#define is_gnome(ptr)		(((ptr)->mflagsa & MA_GNOME) != 0L)
#define is_orc(ptr)		(((ptr)->mflagsa & MA_ORC) != 0L)
#define is_ogre(ptr)		((ptr)->mlet == S_OGRE)
#define is_troll(ptr)		((ptr)->mlet == S_TROLL)
#define is_kobold(ptr)		((ptr)->mlet == S_KOBOLD)
#define is_ettin(ptr)		((ptr) == &mons[PM_ETTIN])
#define is_human(ptr)		(((ptr)->mflagsa & MA_HUMAN) != 0L)
#define is_fungus(ptr)		((ptr)->mlet == S_FUNGUS)
#define is_migo(ptr)		((ptr) == &mons[PM_MIGO_WORKER] ||\
							 (ptr) == &mons[PM_MIGO_SOLDIER] ||\
							 (ptr) == &mons[PM_MIGO_PHILOSOPHER] ||\
							 (ptr) == &mons[PM_MIGO_QUEEN])
#define your_race(ptr)		(((ptr)->mflagsa & urace.selfmask) != 0L)
#define is_andromaliable(ptr)	(is_elf(ptr) || is_drow(ptr) || is_dwarf(ptr) || is_gnome(ptr) || is_orc(ptr) || is_human(ptr) || (ptr) == &mons[PM_HOBBIT] || \
								 (ptr) == &mons[PM_MONKEY] || (ptr) == &mons[PM_APE] || (ptr) == &mons[PM_YETI] || \
								 (ptr) == &mons[PM_CARNIVOROUS_APE] || (ptr) == &mons[PM_SASQUATCH]\
								)
#define is_bat(ptr)		((ptr) == &mons[PM_BAT] || \
				 (ptr) == &mons[PM_GIANT_BAT] || \
				 (ptr) == &mons[PM_VAMPIRE_BAT])
#define is_metroid(ptr) ((ptr)->mlet == S_TRAPPER && !((ptr) == &mons[PM_TRAPPER] || (ptr) == &mons[PM_LURKER_ABOVE]))
#define is_social_insect(ptr) ((ptr)->mlet == S_ANT && (ptr)->maligntyp > 0)
#define is_spider(ptr)	((ptr)->mlet == S_SPIDER && (\
				 (ptr) == &mons[PM_CAVE_SPIDER] ||\
				 (ptr) == &mons[PM_GIANT_SPIDER] ||\
				 (ptr) == &mons[PM_MIRKWOOD_SPIDER] ||\
				 (ptr) == &mons[PM_PHASE_SPIDER] ||\
				 (ptr) == &mons[PM_MIRKWOOD_ELDER] \
				 ))
#define is_rat(ptr)		((ptr) == &mons[PM_SEWER_RAT] || \
				 (ptr) == &mons[PM_GIANT_RAT] || \
				 (ptr) == &mons[PM_RABID_RAT] || \
				 (ptr) == &mons[PM_ENORMOUS_RAT] || \
				 (ptr) == &mons[PM_RODENT_OF_UNUSUAL_SIZE])
#define is_dragon(ptr)		(((ptr)->mflagsa & MA_DRAGON) != 0L)
#define is_true_dragon(ptr)		(monsndx(ptr) >= PM_BABY_GRAY_DRAGON && monsndx(ptr) <= PM_YELLOW_DRAGON)
#define is_pseudodragon(ptr)	(monsndx(ptr) >= PM_TINY_PSEUDODRAGON && monsndx(ptr) <= PM_GIGANTIC_PSEUDODRAGON)
#define is_bird(ptr)		(((ptr)->mflagsa & MA_AVIAN) != 0L)
#define is_giant(ptr)		(((ptr)->mflagsa & MA_GIANT) != 0L)
#define is_gnoll(ptr)		((ptr) == &mons[PM_GNOLL] || \
				 (ptr) == &mons[PM_GNOLL_GHOUL] || \
				 (ptr) == &mons[PM_ANUBITE] || \
				 (ptr) == &mons[PM_GNOLL_MATRIARCH] || \
				 (ptr) == &mons[PM_YEENOGHU])
#define is_minotaur(ptr)		((ptr) == &mons[PM_MINOTAUR] || \
				 (ptr) == &mons[PM_MINOTAUR_PRIESTESS] || \
				 (ptr) == &mons[PM_BAPHOMET])
#define is_pirate(ptr)	((ptr) == &mons[PM_PIRATE] || \
				 (ptr) == &mons[PM_PIRATE_BROTHER] || \
				 (ptr) == &mons[PM_SKELETAL_PIRATE] || \
				 (ptr) == &mons[PM_DAMNED_PIRATE] || \
				 (ptr) == &mons[PM_GITHYANKI_PIRATE] || \
				 (ptr) == &mons[PM_MAYOR_CUMMERBUND])
#define is_golem(ptr)		((ptr)->mlet == S_GOLEM)
#define is_clockwork(ptr)	(((ptr)->mflagsa & MA_CLOCK) != 0L)
#define is_domestic(ptr)	(((ptr)->mflagst & MT_DOMESTIC) != 0L)
#define is_demon(ptr)		(((ptr)->mflagsa & MA_DEMON) != 0L)
#define is_keter(ptr)		((ptr)->mlet == S_KETER)
#define is_angel(ptr)		((((ptr)->mflagsa & MA_MINION) != 0L) && ((ptr)->mlet == S_LAW_ANGEL || (ptr)->mlet == S_NEU_ANGEL || (ptr)->mlet == S_CHA_ANGEL))
#define is_auton(ptr)		(	(ptr) == &mons[PM_MONOTON] ||\
								(ptr) == &mons[PM_DUTON] ||\
								(ptr) == &mons[PM_TRITON] ||\
								(ptr) == &mons[PM_QUATON] ||\
								(ptr) == &mons[PM_QUINON] ||\
								(ptr) == &mons[PM_AXUS]\
							)
#define is_divider(ptr)		( (ptr) == &mons[PM_BLACK_PUDDING]\
							  || (ptr) == &mons[PM_BROWN_PUDDING]\
							  || (ptr) == &mons[PM_DARKNESS_GIVEN_HUNGER]\
							  || (ptr) == &mons[PM_GREMLIN]\
							  || (ptr) == &mons[PM_DUNGEON_FERN_SPORE]\
							  || (ptr) == &mons[PM_DUNGEON_FERN_SPROUT]\
							  || (ptr) == &mons[PM_BURNING_FERN_SPORE]\
							  || (ptr) == &mons[PM_BURNING_FERN_SPROUT]\
							  || (ptr) == &mons[PM_SWAMP_FERN_SPORE]\
							  || (ptr) == &mons[PM_SWAMP_FERN_SPROUT]\
							  || (ptr) == &mons[PM_RAZORVINE]\
							)
#define is_mercenary(ptr)	(((ptr)->mflagsg & MG_MERC) != 0L)
#define is_male(ptr)		(((ptr)->mflagsb & MB_MALE) != 0L)
#define is_female(ptr)		(((ptr)->mflagsb & MB_FEMALE) != 0L)
#define is_neuter(ptr)		(((ptr)->mflagsb & MB_NEUTER) != 0L)
#define is_wanderer(ptr)	(((ptr)->mflagst & MT_WANDER) != 0L)
#define always_hostile(ptr)	(((ptr)->mflagst & MT_HOSTILE) != 0L)
#define always_peaceful(ptr)	(((ptr)->mflagst & MT_PEACEFUL) != 0L)
#define race_hostile(ptr)	(((ptr)->mflagsa & urace.hatemask) != 0L)
#define race_peaceful(ptr)	(((ptr)->mflagsa & urace.lovemask) != 0L)
#define extra_nasty(ptr)	(((ptr)->mflagsg & MG_NASTY) != 0L)
#define strongmonst(ptr)	(((ptr)->mflagsb & MB_STRONG) != 0L)
#define can_breathe(ptr)	attacktype(ptr, AT_BREA)

#define cantwield(ptr)		(nohands(ptr))
#define could_twoweap(ptr)	((ptr)->mattk[1].aatyp == AT_WEAP)
// define cantweararm(ptr)	(breakarm(ptr) || sliparm(ptr))
#define arm_match(ptr,obj)	(Is_dragon_scales(obj) || (((ptr->mflagsb&MB_BODYTYPEMASK) != 0) && \
		((ptr->mflagsb&MB_BODYTYPEMASK) == (obj->bodytypeflag&MB_BODYTYPEMASK))))
#define can_wear_gloves(ptr)	(!nohands(ptr))
#define can_wear_amulet(ptr)	(has_head(ptr))
#define can_wear_boots(ptr)	(humanoid(ptr))
#define shirt_match(ptr,obj)	(((ptr->mflagsb&MB_HUMANOID) && (obj->bodytypeflag&MB_HUMANOID)) || \
		(((ptr->mflagsb&MB_BODYTYPEMASK) != 0) && ((ptr->mflagsb&MB_BODYTYPEMASK) == (obj->bodytypeflag&MB_BODYTYPEMASK))))
#define helm_match(ptr,obj)	(((ptr->mflagsb&MB_HEADMODIMASK) == (obj->bodytypeflag&MB_HEADMODIMASK)))
/*Note: No-modifier helms are "normal"*/

#define hates_unholy(ptr)	((ptr->mflagsg&MG_HATESUNHOLY) != 0)
#define hates_silver(ptr)	((ptr->mflagsg&MG_HATESSILVER) != 0)
#define hates_iron(ptr)		((ptr->mflagsg&MG_HATESIRON) != 0)

#define throws_rocks(ptr)	(((ptr)->mflagst & MT_ROCKTHROW) != 0L)
#define type_is_pname(ptr)	(((ptr)->mflagsg & MG_PNAME) != 0L)
#define is_thief(ptr)		( dmgtype(ptr, AD_SGLD)  || dmgtype(ptr, AD_SITM) || dmgtype(ptr, AD_SEDU) )
#define is_magical(ptr)		( attacktype(ptr, AT_MMGC) || attacktype(ptr, AT_MAGC) )
#define is_lord(ptr)		(((ptr)->mflagsg & MG_LORD) != 0L)
#define is_prince(ptr)		(((ptr)->mflagsg & MG_PRINCE) != 0L)
#define is_ndemon(ptr)		(is_demon(ptr) && \
				 (((ptr)->mflagsg & (MG_LORD|MG_PRINCE)) == 0L))
#define is_dlord(ptr)		(is_demon(ptr) && is_lord(ptr))
#define is_dprince(ptr)		(is_demon(ptr) && is_prince(ptr))
#define is_minion(ptr)		((ptr)->mflagsa & MA_MINION)
#define likes_gold(ptr)		(((ptr)->mflagst & MT_GREEDY) != 0L)
#define likes_gems(ptr)		(((ptr)->mflagst & MT_JEWELS) != 0L)
#define likes_objs(ptr)		(((ptr)->mflagst & MT_COLLECT) != 0L || \
				 is_armed(ptr))
#define likes_magic(ptr)	(((ptr)->mflagst & MT_MAGIC) != 0L)
#define webmaker(ptr)		((ptr) == &mons[PM_CAVE_SPIDER] || \
				 (ptr) == &mons[PM_GIANT_SPIDER] || (ptr) == &mons[PM_PHASE_SPIDER] || \
				 (ptr) == &mons[PM_MIRKWOOD_SPIDER] || (ptr) == &mons[PM_MIRKWOOD_ELDER] || \
				 (ptr) == &mons[PM_SPROW] || (ptr) == &mons[PM_DRIDER] || \
				 (ptr) == &mons[PM_AVATAR_OF_LOLTH] || (ptr) == &mons[PM_DROW_MUMMY])
#define is_unicorn(ptr)		((ptr)->mlet == S_UNICORN && likes_gems(ptr))
#define is_longworm(ptr)	(((ptr) == &mons[PM_BABY_LONG_WORM]) || \
				 ((ptr) == &mons[PM_LONG_WORM]) || \
				 ((ptr) == &mons[PM_LONG_WORM_TAIL]))
#define wants_bell(ptr)	((ptr->mflagst & MT_WANTSBELL))
#define wants_book(ptr)	((ptr->mflagst & MT_WANTSBOOK))
#define wants_cand(ptr)	((ptr->mflagst & MT_WANTSCAND))
#define wants_qart(ptr)	((ptr->mflagst & MT_WANTSARTI))
#define wants_amul(ptr)	((ptr->mflagst & MT_COVETOUS))
#define is_covetous(ptr)	((ptr->mflagst & MT_COVETOUS))

#define normalvision(ptr)	((ptr->mflagsv & MV_NORMAL))
#define darksight(ptr)		((ptr->mflagsv & MV_DARKSIGHT))
#define catsight(ptr)		((ptr->mflagsv & MV_CATSIGHT))
#define lowlightsight2(ptr)	((ptr->mflagsv & MV_LOWLIGHT2))
#define lowlightsight3(ptr)	((ptr->mflagsv & MV_LOWLIGHT3))
#define echolocation(ptr)	((ptr->mflagsv & MV_ECHOLOCATE))
#define extramission(ptr)	((ptr->mflagsv & MV_EXTRAMISSION))
#define rlyehiansight(ptr)	((ptr->mflagsv & MV_RLYEHIAN))

#define infravision(ptr)	((ptr->mflagsv & MV_INFRAVISION))
#define infravisible(ptr)	((ptr->mflagsg & MG_INFRAVISIBLE))
#define bloodsense(ptr)		((ptr->mflagsv & MV_BLOODSENSE))
#define lifesense(ptr)		((ptr->mflagsv & MV_LIFESENSE))
#define earthsense(ptr)		((ptr->mflagsv & MV_EARTHSENSE))
#define senseall(ptr)		((ptr->mflagsv & MV_DETECTION))

#define ominsense(ptr)		((ptr->mflagsv & MV_OMNI))

#define can_betray(ptr)		((ptr->mflagst & MT_TRAITOR))
#define opaque(ptr)	(((ptr)->mflagsg & MG_OPAQUE))
#define mteleport(ptr)	(((ptr)->mflagsm & MM_TENGTPORT))
#define is_mplayer(ptr)		(((ptr) >= &mons[PM_ARCHEOLOGIST]) && \
				 ((ptr) <= &mons[PM_WIZARD]))
#define is_deadly(ptr)		((ptr)->mflagsg & MG_DEADLY)
#define is_rider(ptr)		((ptr)->mflagsg & MG_RIDER)
#define is_placeholder(ptr)	((ptr) == &mons[PM_ORC] || \
				 (ptr) == &mons[PM_GIANT] || \
				 (ptr) == &mons[PM_ELF] || \
				 (ptr) == &mons[PM_HUMAN])

/* return TRUE if the monster tends to revive */
#define is_reviver(ptr)		(is_rider(ptr) || (ptr)->mlet == S_TROLL || (ptr)->mlet == S_FUNGUS)

#define is_demihuman(ptr)	((ptr->mflagsa & MA_DEMIHUMAN))

/* this returns the light's range, or 0 if none; if we add more light emitting
   monsters, we'll likely have to add a new light range field to mons[] */
#define emits_light(ptr)	(((ptr)->mlet == S_LIGHT || \
				  (ptr) == &mons[PM_FLAMING_SPHERE] || \
				  (ptr) == &mons[PM_SHOCKING_SPHERE] || \
				  (ptr) == &mons[PM_MOTE_OF_LIGHT] || \
				  (ptr) == &mons[PM_BALL_OF_LIGHT] || \
				  (ptr) == &mons[PM_BLOODY_SUNSET] || \
				  (ptr) == &mons[PM_BALL_OF_GOSSAMER_SUNLIGHT] || \
				  (ptr) == &mons[PM_LUMINOUS_CLOUD] || \
				  (ptr) == &mons[PM_HOOLOOVOO] || \
				  (ptr) == &mons[PM_LIGHTNING_PARAELEMENTAL] || \
				  (ptr) == &mons[PM_FALLEN_ANGEL] || \
				  (ptr) == &mons[PM_FIRE_VORTEX]) ? 1 : \
				 ((ptr) == &mons[PM_FIRE_ELEMENTAL] ||\
				  (ptr) == &mons[PM_DANCING_FLAME] ||\
				  (ptr) == &mons[PM_COTERIE_OF_MOTES] ||\
				  (ptr) == &mons[PM_BALL_OF_RADIANCE]) ? 2 : \
				 ((ptr) == &mons[PM_LIGHT_ARCHON]|| \
				  (ptr) == &mons[PM_LUCIFER]) ? 7 : \
				 ((ptr) == &mons[PM_EDDERKOP]) ? 8 : \
				 0)
#define Is_darklight_monster(ptr)	((ptr) == &mons[PM_EDDERKOP])
/*	[note: the light ranges above were reduced to 1 for performance...] */
#define likes_lava(ptr)		(ptr == &mons[PM_FIRE_ELEMENTAL] || \
				 ptr == &mons[PM_SALAMANDER])
#define pm_invisible(ptr) ((ptr) == &mons[PM_STALKER] || \
			   (ptr) == &mons[PM_BLACK_LIGHT] ||\
			   (ptr) == &mons[PM_PHANTOM_FUNGUS] ||\
			   (ptr) == &mons[PM_CENTER_OF_ALL] ||\
			   (ptr) == &mons[PM_DARKNESS_GIVEN_HUNGER] ||\
			   (ptr) == &mons[PM_ANCIENT_OF_DEATH]\
			   )

/* could probably add more */
#define likes_fire(ptr)		((ptr) == &mons[PM_FIRE_VORTEX] || \
				  (ptr) == &mons[PM_FLAMING_SPHERE] || \
				 likes_lava(ptr))

#define touch_petrifies(ptr)	((ptr) == &mons[PM_COCKATRICE] || \
				 (ptr) == &mons[PM_CHICKATRICE])

#define is_weeping(ptr)		((ptr) == &mons[PM_WEEPING_ANGEL])

#define is_alienist(ptr)		(is_mind_flayer(ptr) || \
								 (ptr)->mlet == S_UMBER ||\
								 (ptr) == &mons[PM_DROW_ALIENIST] ||\
								 (ptr) == &mons[PM_DARUTH_XAXOX] ||\
								 (ptr) == &mons[PM_EMBRACED_DROWESS]\
								)
#define has_mind_blast(ptr)	(is_mind_flayer(ptr) || \
				 (ptr) == &mons[PM_BRAIN_GOLEM] || \
				 (ptr) == &mons[PM_SEMBLANCE] \
				)

#define is_mind_flayer(ptr)	((ptr) == &mons[PM_MIND_FLAYER] || \
				 (ptr) == &mons[PM_MASTER_MIND_FLAYER] || \
				 (ptr) == &mons[PM_ALHOON] || \
				 (ptr) == &mons[PM_ELDER_BRAIN] || \
				 (ptr) == &mons[PM_LUGRIBOSSK] || \
				 (ptr) == &mons[PM_MAANZECORIAN] || \
				 (ptr) == &mons[PM_GREAT_CTHULHU] \
				)

#define nonliving(ptr)		(is_unalive(ptr) || is_undead(ptr) || \
				 (ptr)->mlet == S_VORTEX || \
				 (ptr) == &mons[PM_MANES] \
				)

#define is_unalive(ptr)		(on_level(&valley_level, &u.uz) || ((ptr->mflagsa & MA_UNLIVING)) )

#define likes_swamp(ptr)	((ptr)->mlet == S_PUDDING || \
				 (ptr)->mlet == S_FUNGUS || \
				 (ptr) == &mons[PM_OCHRE_JELLY])
#define stationary(ptr)		((ptr)->mflagsm & MM_STATIONARY)

/* Used for conduct with corpses, tins, and digestion attacks */
/* G_NOCORPSE monsters might still be swallowed as a purple worm */
/* Maybe someday this could be in mflags... */
#define vegan(ptr)		(((ptr)->mlet == S_BLOB && \
							(ptr) != &mons[PM_BLOB_OF_PRESERVED_ORGANS]) || \
				 (ptr)->mlet == S_JELLY ||            \
				((ptr)->mlet == S_FUNGUS && 		  \
					!is_migo(ptr)) ||				  \
				 (ptr)->mlet == S_VORTEX ||           \
				 (ptr)->mlet == S_LIGHT ||            \
				 (ptr)->mlet == S_PLANT ||            \
				((ptr)->mlet == S_ELEMENTAL &&        \
				 (ptr) != &mons[PM_STALKER]) ||       \
				((ptr)->mlet == S_GOLEM &&            \
				 (ptr) != &mons[PM_FLESH_GOLEM] &&    \
				 (ptr) != &mons[PM_LEATHER_GOLEM]) || \
				 (ptr) == &mons[PM_WOOD_TROLL] ||     \
				 noncorporeal(ptr))
#define is_burnable(ptr)	((ptr)->mlet == S_PLANT || \
							((ptr)->mlet == S_FUNGUS && !is_migo(ptr)) || \
							(ptr) == &mons[PM_WOOD_TROLL])
#define vegetarian(ptr)		(vegan(ptr) || \
				((ptr)->mlet == S_PUDDING &&         \
				 (ptr) != &mons[PM_BLACK_PUDDING] && \
				 (ptr) != &mons[PM_DARKNESS_GIVEN_HUNGER]))

/* For vampires */
#define has_blood(ptr)		(!vegetarian(ptr) && \
				   (ptr)->mlet != S_GOLEM && \
				   (ptr)->mlet != S_KETER && \
				   (ptr)->mlet != S_MIMIC && \
				   !is_clockwork(ptr) && \
				   (!is_undead(ptr) || is_vampire(ptr)))

/* Keep track of ferns, fern sprouts, fern spores, and other plants */

#define is_fern_sprout(ptr)	((ptr) == &mons[PM_DUNGEON_FERN_SPROUT] || \
				 (ptr) == &mons[PM_SWAMP_FERN_SPROUT] || \
				 (ptr) == &mons[PM_BURNING_FERN_SPROUT])

#define is_fern_spore(ptr)	((ptr) == &mons[PM_DUNGEON_FERN_SPORE] || \
				 (ptr) == &mons[PM_SWAMP_FERN_SPORE] || \
				 (ptr) == &mons[PM_BURNING_FERN_SPORE])

#define is_fern(ptr)		(is_fern_sprout(ptr) || \
				 (ptr) == &mons[PM_DUNGEON_FERN] || \
				 (ptr) == &mons[PM_SWAMP_FERN] || \
				 (ptr) == &mons[PM_BURNING_FERN])

#define is_vegetation(ptr)	((ptr)->mlet == S_PLANT || is_fern(ptr))

#ifdef CONVICT
#define befriend_with_obj(ptr, obj) (((obj)->oclass == FOOD_CLASS ||\
				      (obj)->otyp == SHEAF_OF_HAY) && \
				      (is_domestic(ptr) || (is_rat(ptr) && Role_if(PM_CONVICT))))
#else
#define befriend_with_obj(ptr, obj) (((obj)->oclass == FOOD_CLASS || \
				      (obj)->otyp == SHEAF_OF_HAY) && \
				      is_domestic(ptr))
#endif

#endif /* MONDATA_H */
