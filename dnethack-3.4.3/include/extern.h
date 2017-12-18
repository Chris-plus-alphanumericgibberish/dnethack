/*	SCCS Id: @(#)extern.h	3.4	2003/03/10	*/
/* Copyright (c) Steve Creps, 1988.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef EXTERN_H
#define EXTERN_H

#define E extern

/* ### alloc.c ### */

#if 0
E long *FDECL(alloc, (unsigned int));
#endif
E char *FDECL(fmt_ptr, (const genericptr,char *));

/* This next pre-processor directive covers almost the entire file,
 * interrupted only occasionally to pick up specific functions as needed. */
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)

/* ### allmain.c ### */

E void NDECL(moveloop);
E void NDECL(stop_occupation);
E void NDECL(display_gamewindows);
E void NDECL(newgame);
E void FDECL(welcome, (BOOLEAN_P));
#if defined(RECORD_REALTIME) || defined(REALTIME_ON_BOTL)
E time_t NDECL(get_realtime);
#endif

/* ### apply.c ### */

E int NDECL(doapply);
E int NDECL(dorub);
E int NDECL(dojump);
E int FDECL(jump, (int));
E int NDECL(number_leashed);
E void FDECL(o_unleash, (struct obj *));
E void FDECL(m_unleash, (struct monst *,BOOLEAN_P));
E void NDECL(unleash_all);
E boolean NDECL(next_to_u);
E struct obj *FDECL(get_mleash, (struct monst *));
E void FDECL(check_leash, (XCHAR_P,XCHAR_P));
E boolean FDECL(um_dist, (XCHAR_P,XCHAR_P,XCHAR_P));
E boolean FDECL(snuff_candle, (struct obj *));
E boolean FDECL(snuff_lit, (struct obj *));
E boolean FDECL(catch_lit, (struct obj *));
E void FDECL(use_unicorn_horn, (struct obj *));
E void FDECL(use_bell, (struct obj **, boolean));
E boolean FDECL(tinnable, (struct obj *));
E void NDECL(reset_trapset);
E void FDECL(fig_transform, (genericptr_t, long));
E int FDECL(unfixable_trouble_count,(BOOLEAN_P));

/* ### artifact.c ### */

E boolean FDECL(CountsAgainstGifts, (int));
E void NDECL(init_artifacts);
E void FDECL(save_artifacts, (int));
E void FDECL(restore_artifacts, (int));
E const char *FDECL(artiname, (int));
E struct obj *FDECL(mk_artifact, (struct obj *,ALIGNTYP_P));
E const char *FDECL(artifact_name, (const char *,short *));
E boolean FDECL(exist_artifact, (int,const char *));
E void FDECL(artifact_exists, (struct obj *,const char *,BOOLEAN_P));
E int NDECL(nartifact_exist);
E boolean FDECL(spec_ability, (struct obj *,unsigned long));
E boolean FDECL(spec_ability2, (struct obj *,unsigned long));
E boolean FDECL(spec_ability3, (struct obj *,unsigned long));
E boolean FDECL(spec_wability, (struct obj *,unsigned long));
E boolean FDECL(confers_luck, (struct obj *));
E boolean FDECL(arti_digs, (struct obj *));
E boolean FDECL(arti_poisoned, (struct obj *));
E boolean FDECL(arti_bright, (struct obj *));
E boolean FDECL(arti_shattering, (struct obj *));
E boolean FDECL(arti_disarm, (struct obj *));
E boolean FDECL(arti_steal, (struct obj *));
E boolean FDECL(arti_tentRod, (struct obj *));
E boolean FDECL(arti_shining, (struct obj *));
E boolean FDECL(arti_mandala, (struct obj *));
E boolean FDECL(arti_lighten, (struct obj *));
E boolean FDECL(arti_chawis, (struct obj *));
E boolean FDECL(arti_plussev, (struct obj *));
E boolean FDECL(arti_silvered, (struct obj *));
E boolean FDECL(arti_reflects, (struct obj *));
E boolean FDECL(arti_light, (struct obj *));
E boolean FDECL(restrict_name, (struct obj *,const char *));
E boolean FDECL(defends, (int,struct obj *));
E boolean FDECL(protects, (int,struct obj *));
E void FDECL(set_artifact_intrinsic, (struct obj *,BOOLEAN_P,long));
E int FDECL(touch_artifact, (struct obj *,struct monst *, boolean));
E int FDECL(spec_abon, (struct obj *,struct monst *));
E int FDECL(spec_dbon, (struct obj *,struct monst *,int));
E int FDECL(narrow_spec_applies, (struct obj *,struct monst *));
E void FDECL(discover_artifact, (int));
E boolean FDECL(undiscovered_artifact, (int));
E int FDECL(disp_artifact_discoveries, (winid));
E boolean FDECL(artifact_hit, (struct monst *,struct monst *,
				struct obj *,int *,int));
E int NDECL(doinvoke);
E int FDECL(doparticularinvoke,(struct obj *));
E void FDECL(arti_speak, (struct obj *));
E boolean FDECL(artifact_light, (struct obj *));
E int FDECL(artifact_wet, (struct obj *, BOOLEAN_P));
E long FDECL(spec_mm, (struct obj *));
E long FDECL(spec_mt, (struct obj *));
E long FDECL(spec_mb, (struct obj *));
E long FDECL(spec_mg, (struct obj *));
E long FDECL(spec_ma, (struct obj *));
E long FDECL(spec_mv, (struct obj *));
E long FDECL(spec_s, (struct obj *));
E boolean FDECL(artifact_has_invprop, (struct obj *,UCHAR_P));
E long FDECL(arti_cost, (struct obj *));
E void FDECL(arti_poly_contents, (struct obj *));

/* ### attrib.c ### */

E boolean FDECL(adjattrib, (int,int,int));
E void FDECL(change_luck, (SCHAR_P));
E int FDECL(stone_luck, (BOOLEAN_P));
E boolean NDECL(has_luckitem);
E void NDECL(set_moreluck);
E void FDECL(gainstr, (struct obj *,int));
E void FDECL(losestr, (int));
E void NDECL(restore_attrib);
E void FDECL(exercise, (int,BOOLEAN_P));
E void NDECL(exerchk);
E void NDECL(reset_attribute_clock);
E void FDECL(init_attr, (int));
E void NDECL(redist_attr);
E void FDECL(adjabil, (int,int));
E int NDECL(newhp);
E int FDECL(conplus, (int));
E schar FDECL(acurr, (int));
E schar FDECL(acurrstr, (int));
E void FDECL(adjalign, (int));
E int NDECL(uhp);
E int NDECL(uhpmax);

/* ### ball.c ### */

E void NDECL(ballfall);
E void NDECL(placebc);
E void NDECL(unplacebc);
E void FDECL(set_bc, (int));
E void FDECL(move_bc, (int,int,XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
E boolean FDECL(drag_ball, (XCHAR_P,XCHAR_P,
		int *,xchar *,xchar *,xchar *,xchar *, boolean *,BOOLEAN_P));
E void FDECL(drop_ball, (XCHAR_P,XCHAR_P));
E void NDECL(drag_down);

/* ### bones.c ### */

E boolean NDECL(can_make_bones);
E void FDECL(savebones, (struct obj *));
E int NDECL(getbones);

/* ### botl.c ### */

E int FDECL(xlev_to_rank, (int));
E int FDECL(title_to_mon, (const char *,int *,int *));
E void NDECL(max_rank_sz);
#ifdef SCORE_ON_BOTL
E long NDECL(botl_score);
#endif
E int FDECL(describe_level, (char *));
E const char *FDECL(code_of, (SHORT_P));
E const char *FDECL(rank_of, (int,SHORT_P,BOOLEAN_P));
E void NDECL(bot);

/* ### cmd.c ### */

#ifdef USE_TRAMPOLI
E int NDECL(doextcmd);
E int NDECL(domonability);
E int NDECL(dofightingform);
E int NDECL(doprev_message);
E int NDECL(timed_occupation);
E int NDECL(wiz_attributes);
E int NDECL(enter_explore_mode);
# ifdef WIZARD
E int NDECL(wiz_detect);
E int NDECL(wiz_genesis);
E int NDECL(wiz_identify);
E int NDECL(wiz_level_tele);
E int NDECL(wiz_map);
E int NDECL(wiz_where);
E int NDECL(wiz_wish);
# endif /* WIZARD */
#endif /* USE_TRAMPOLI */
E void NDECL(reset_occupations);
E void FDECL(set_occupation, (int (*)(void),const char *,int));
#ifdef REDO
E char NDECL(pgetchar);
E void FDECL(pushch, (CHAR_P));
E void FDECL(savech, (CHAR_P));
#endif
#ifdef WIZARD
E void NDECL(add_debug_extended_commands);
#endif /* WIZARD */
E void FDECL(rhack, (char *));
E int NDECL(doextlist);
E int NDECL(extcmd_via_menu);
E void FDECL(enlightenment, (int));
E void NDECL(signs_mirror);
E void FDECL(show_conduct, (int));
E int FDECL(xytod, (SCHAR_P,SCHAR_P));
E void FDECL(dtoxy, (coord *,int));
E int FDECL(movecmd, (CHAR_P));
E int FDECL(getdir, (const char *));
E void NDECL(confdir);
E int FDECL(isok, (int,int));
E int FDECL(get_adjacent_loc, (const char *, const char *, XCHAR_P, XCHAR_P, coord *));
E const char *FDECL(click_to_cmd, (int,int,int));
E char NDECL(readchar);
#ifdef WIZARD
E void NDECL(sanity_check);
#endif
E char FDECL(yn_function, (const char *, const char *, CHAR_P));
E int NDECL(dotravel);

/* ### dbridge.c ### */

E boolean FDECL(is_pool, (int,int, BOOLEAN_P));
E boolean FDECL(is_3dwater, (int,int));
E boolean FDECL(is_lava, (int,int));
E boolean FDECL(is_ice, (int,int));
E int FDECL(is_drawbridge_wall, (int,int));
E boolean FDECL(is_db_wall, (int,int));
E boolean FDECL(find_drawbridge, (int *,int*));
E boolean FDECL(create_drawbridge, (int,int,int,BOOLEAN_P));
E void FDECL(open_drawbridge, (int,int));
E void FDECL(close_drawbridge, (int,int));
E void FDECL(destroy_drawbridge, (int,int));

/* ### decl.c ### */

E void NDECL(decl_init);

/* ### detect.c ### */

E struct obj *FDECL(o_in, (struct obj*,CHAR_P));
E struct obj *FDECL(o_material, (struct obj*,unsigned));
E struct obj *FDECL(o_artifact, (struct obj*));
E int FDECL(gold_detect, (struct obj *));
E int FDECL(food_detect, (struct obj *));
E int FDECL(object_detect, (struct obj *,int));
E int FDECL(artifact_detect, (struct obj *));
E int FDECL(book_detect, (boolean));
E int FDECL(monster_detect, (struct obj *,int));
E int FDECL(pet_detect_and_tame, (struct obj *));
E int FDECL(trap_detect, (struct obj *));
E const char *FDECL(level_distance, (d_level *));
E int FDECL(use_crystal_ball, (struct obj *));
E void NDECL(do_mapping);
E void FDECL(do_vicinity_map, (int,int));
E void FDECL(cvt_sdoor_to_door, (struct rm *));
#ifdef USE_TRAMPOLI
E void FDECL(findone, (int,int,genericptr_t));
E void FDECL(openone, (int,int,genericptr_t));
#endif
E int NDECL(findit);
E int NDECL(openit);
E void FDECL(find_trap, (struct trap *));
E int FDECL(dosearch0, (int));
E int NDECL(dosearch);
E void NDECL(sokoban_detect);

/* ### dig.c ### */

E schar FDECL(fillholetyp, (int, int));
E boolean NDECL(is_digging);
#ifdef USE_TRAMPOLI
E int NDECL(dig);
#endif
E int NDECL(holetime);
E boolean FDECL(dig_check, (struct monst *, BOOLEAN_P, int, int));
E void FDECL(openactualrocktrap, (int,int,struct monst *));
E void FDECL(openactualdoor, (int,int,struct monst *,int));
E void FDECL(digactualhole, (int,int,struct monst *,int,boolean,boolean));
E boolean NDECL(openrocktrap);
E boolean FDECL(opennewdoor, (int,int));
E boolean FDECL(opentrapdoor, (BOOLEAN_P));
E boolean FDECL(dighole, (BOOLEAN_P));
E boolean FDECL(digfarhole, (BOOLEAN_P,int,int));
E int FDECL(use_pick_axe, (struct obj *));
E int FDECL(use_pick_axe2, (struct obj *));
E boolean FDECL(mdig_tunnel, (struct monst *));
E void FDECL(watch_dig, (struct monst *,XCHAR_P,XCHAR_P,BOOLEAN_P));
E void FDECL(zap_dig, (int,int,int));
E struct obj *FDECL(bury_an_obj, (struct obj *));
E void FDECL(bury_objs, (int,int));
E void FDECL(unearth_objs, (int,int));
E void FDECL(rot_organic, (genericptr_t, long));
E void FDECL(rot_corpse, (genericptr_t, long));
E void FDECL(dig_up_grave, (int, int));
#if 0
E void FDECL(bury_monst, (struct monst *));
E void NDECL(bury_you);
E void NDECL(unearth_you);
E void NDECL(escape_tomb);
E void FDECL(bury_obj, (struct obj *));
#endif

/* ### display.c ### */

#ifdef INVISIBLE_OBJECTS
E struct obj * FDECL(vobj_at, (XCHAR_P,XCHAR_P));
#endif /* INVISIBLE_OBJECTS */
E void FDECL(magic_map_background, (XCHAR_P,XCHAR_P,int));
E void FDECL(map_background, (XCHAR_P,XCHAR_P,int));
E void FDECL(map_trap, (struct trap *,int));
E void FDECL(map_object, (struct obj *,int));
E void FDECL(map_invisible, (XCHAR_P,XCHAR_P));
E void FDECL(unmap_object, (int,int));
E void FDECL(map_location, (int,int,int));
E void FDECL(feel_location, (XCHAR_P,XCHAR_P));
E void FDECL(echo_location, (XCHAR_P,XCHAR_P));
E void FDECL(newsym, (int,int));
E boolean FDECL(sense_by_scent, (struct monst *));
E void FDECL(shieldeff, (XCHAR_P,XCHAR_P));
E void FDECL(tmp_at, (int,int));
E void FDECL(swallowed, (int));
E void FDECL(under_ground, (int));
E void FDECL(under_water, (int));
E void NDECL(see_monsters);
E void NDECL(set_mimic_blocking);
E void NDECL(see_objects);
E void NDECL(see_traps);
E void NDECL(curs_on_u);
E int NDECL(doredraw);
E void NDECL(docrt);
E void FDECL(show_glyph, (int,int,int));
E void NDECL(clear_glyph_buffer);
E void FDECL(row_refresh, (int,int,int));
E void NDECL(cls);
E void FDECL(flush_screen, (int));
E int FDECL(back_to_glyph, (XCHAR_P,XCHAR_P));
E int FDECL(zapdir_to_glyph, (int,int,int));
E int FDECL(glyph_at, (XCHAR_P,XCHAR_P));
E void NDECL(set_wall_state);

/* ### do.c ### */

#ifdef USE_TRAMPOLI
E int FDECL(drop, (struct obj *));
E int NDECL(wipeoff);
#endif
E int NDECL(dodrop);
E boolean FDECL(boulder_hits_pool, (struct obj *,int,int,BOOLEAN_P));
E boolean FDECL(flooreffects, (struct obj *,int,int,const char *));
E void FDECL(doaltarobj, (struct obj *));
E boolean FDECL(canletgo, (struct obj *,const char *));
E void FDECL(dropx, (struct obj *));
E void FDECL(dropy, (struct obj *));
E void FDECL(obj_no_longer_held, (struct obj *));
E int NDECL(doddrop);
E int NDECL(dodown);
E int NDECL(doup);
#ifdef INSURANCE
E void NDECL(save_currentstate);
#endif
E void FDECL(goto_level, (d_level *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P));
E void FDECL(schedule_goto, (d_level *,BOOLEAN_P,BOOLEAN_P,int,
			     const char *,const char *));
E void NDECL(deferred_goto);
E boolean FDECL(revive_corpse, (struct obj *, int));
E void FDECL(revive_mon, (genericptr_t, long));
E void FDECL(moldy_corpse, (genericptr_t, long));
E void FDECL(slimy_corpse, (genericptr_t, long));
E void FDECL(shady_corpse, (genericptr_t, long));
E void FDECL(zombie_corpse, (genericptr_t, long));
E int NDECL(donull);
E int NDECL(dowipe);
E void FDECL(set_wounded_legs, (long,int));
E void NDECL(heal_legs);
E int NDECL(dowait);
E int NDECL(docome);

/* ### do_name.c ### */

E int FDECL(getpos, (coord *,BOOLEAN_P,const char *));
E struct monst *FDECL(christen_monst, (struct monst *,const char *));
E int NDECL(do_mname);
E struct obj *FDECL(oname, (struct obj *,const char *));
E int NDECL(ddocall);
E void FDECL(docall, (struct obj *));
E const char *NDECL(rndghostname);
E char *FDECL(x_monnam, (struct monst *,int,const char *,int,BOOLEAN_P));
E char *FDECL(x_ptrnam, (struct permonst *,int,const char *, BOOLEAN_P));
E char *FDECL(l_monnam, (struct monst *));
E char *FDECL(mon_nam, (struct monst *));
E const char *FDECL(sheheit, (struct monst *));
E const char *FDECL(SheHeIt, (struct monst *));
E char *FDECL(noit_mon_nam, (struct monst *));
E char *FDECL(noit_nohalu_mon_nam, (struct monst *));
E char *FDECL(Monnam, (struct monst *));
E char *FDECL(noit_Monnam, (struct monst *));
E char *FDECL(m_monnam, (struct monst *));
E char *FDECL(y_monnam, (struct monst *));
E char *FDECL(Adjmonnam, (struct monst *,const char *));
E char *FDECL(Amonnam, (struct monst *));
E char *FDECL(Aptrnam, (struct permonst *));
E char *FDECL(a_monnam, (struct monst *));
E char *FDECL(a_ptrnam, (struct permonst *));
E char *FDECL(getDrowHouse, (long));
E char *FDECL(distant_monnam, (struct monst *,int,char *));
E const char *NDECL(rndmonnam);
E const char *FDECL(hcolor, (const char *));
E const char *NDECL(rndcolor);
#ifdef REINCARNATION
E const char *NDECL(roguename);
#endif
E struct obj *FDECL(realloc_obj,
		(struct obj *, int, genericptr_t, int, const char *));
E char *FDECL(coyotename, (struct monst *,char *));

/* ### do_wear.c ### */

#ifdef USE_TRAMPOLI
E int NDECL(Boots_on);
E int NDECL(Gloves_on);
E int NDECL(Helmet_on);
E int FDECL(select_off, (struct obj *));
E int NDECL(take_off);
#endif
E void FDECL(off_msg, (struct obj *));
E void NDECL(set_wear);
E boolean FDECL(donning, (struct obj *));
E void NDECL(cancel_don);
E int NDECL(Armor_on);
E int NDECL(Armor_off);
E int NDECL(Armor_gone);
E int NDECL(Helmet_on);
E int NDECL(Helmet_off);
E int NDECL(Gloves_off);
E int NDECL(Boots_on);
E int NDECL(Boots_off);
E int NDECL(Cloak_off);
E int NDECL(Shield_off);
E int NDECL(Shirt_on);
E int NDECL(Shirt_off);
E void NDECL(Amulet_off);
E void NDECL(Amulet_on);
E void FDECL(Ring_on, (struct obj *));
E void FDECL(Ring_off, (struct obj *));
E void FDECL(Ring_gone, (struct obj *));
E void FDECL(Blindf_on, (struct obj *));
E void FDECL(Blindf_off, (struct obj *));
E int NDECL(dotakeoff);
E int NDECL(doremring);
E int FDECL(cursed, (struct obj *));
E int FDECL(armoroff, (struct obj *));
E int FDECL(canwearobj, (struct obj *, long *, BOOLEAN_P));
E int NDECL(dowear);
E int NDECL(doputon);
E int NDECL(base_uac);
E void NDECL(find_ac);
E void NDECL(glibr);
E struct obj *FDECL(some_armor,(struct monst *));
E void FDECL(erode_armor, (struct monst *,BOOLEAN_P));
E struct obj *FDECL(stuck_ring, (struct obj *,int));
E struct obj *NDECL(unchanger);
E void NDECL(reset_remarm);
E int NDECL(doddoremarm);
E int FDECL(destroy_arm, (struct obj *));
E int FDECL(claws_destroy_arm, (struct obj *));
E int FDECL(teleport_arm, (struct obj *));
E int FDECL(tent_destroy_arm, (struct obj *));
E void FDECL(adj_abon, (struct obj *,SCHAR_P));

/* ### dog.c ### */

E void FDECL(initedog, (struct monst *));
E struct monst *FDECL(make_familiar, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P));
E struct monst *NDECL(makedog);
E void NDECL(update_mlstmv);
E void NDECL(losedogs);
E void FDECL(mon_arrive, (struct monst *,BOOLEAN_P));
E void FDECL(mon_catchup_elapsed_time, (struct monst *,long));
E void FDECL(keepdogs, (BOOLEAN_P));
E void FDECL(migrate_to_level, (struct monst *,XCHAR_P,XCHAR_P,coord *));
E int FDECL(dogfood, (struct monst *,struct obj *));
E struct monst *FDECL(tamedog, (struct monst *,struct obj *));
E struct monst *FDECL(make_pet_minion, (int,aligntyp));
E void FDECL(abuse_dog, (struct monst *));
E void FDECL(wary_dog, (struct monst *, BOOLEAN_P));

/* ### dogmove.c ### */

E boolean FDECL(could_use_item, (struct monst *, struct obj *, boolean));
E boolean FDECL(acceptable_pet_target, (struct monst *,struct monst *,BOOLEAN_P));
E boolean FDECL(betrayed, (struct monst *));
E int FDECL(dog_nutrition, (struct monst *,struct obj *));
E int FDECL(dog_eat, (struct monst *,struct obj *,int,int,BOOLEAN_P));
E int FDECL(dog_move, (struct monst *,int));
#ifdef USE_TRAMPOLI
E void FDECL(wantdoor, (int,int,genericptr_t));
#endif

/* ### dokick.c ### */

E boolean FDECL(ghitm, (struct monst *,struct obj *));
E void FDECL(container_impact_dmg, (struct obj *));
E int NDECL(dokick);
E boolean FDECL(ship_object, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P));
E void NDECL(obj_delivery);
E schar FDECL(down_gate, (XCHAR_P,XCHAR_P));
E void FDECL(impact_drop, (struct obj *,XCHAR_P,XCHAR_P,XCHAR_P));

/* ### dothrow.c ### */

E int FDECL(throw_obj, (struct obj *,int, int));
E int FDECL(zap_raygun, (struct obj *,int, int));
E int NDECL(dothrow);
E int NDECL(dofire);
E void FDECL(hitfloor, (struct obj *));
E void FDECL(hurtle, (int,int,int,BOOLEAN_P));
E void FDECL(mhurtle, (struct monst *,int,int,int));
E void FDECL(throwit, (struct obj *,long,BOOLEAN_P,int));
E int FDECL(omon_adj, (struct monst *,struct obj *,BOOLEAN_P));
E int FDECL(thitmonst, (struct monst *,struct obj *,int));
E int FDECL(hero_breaks, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P));
E int FDECL(breaks, (struct obj *,XCHAR_P,XCHAR_P));
E boolean FDECL(breaktest, (struct obj *));
E boolean FDECL(walk_path, (coord *, coord *, boolean (*)(genericptr_t,int,int), genericptr_t));
E boolean FDECL(hurtle_step, (genericptr_t, int, int));

/* ### drawing.c ### */
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
E int FDECL(def_char_to_objclass, (CHAR_P));
E int FDECL(def_char_to_monclass, (CHAR_P));
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
E void FDECL(assign_graphics, (uchar *,int,int,int));
E void FDECL(switch_graphics, (int));
#ifdef REINCARNATION
E void FDECL(assign_rogue_graphics, (BOOLEAN_P));
#endif

/* ### dungeon.c ### */

E void FDECL(save_dungeon, (int,BOOLEAN_P,BOOLEAN_P));
E void FDECL(restore_dungeon, (int));
E void FDECL(insert_branch, (branch *,BOOLEAN_P));
E void NDECL(init_dungeons);
E s_level *FDECL(find_level, (const char *));
E s_level *FDECL(Is_special, (d_level *));
E branch *FDECL(Is_branchlev, (d_level *));
E xchar FDECL(ledger_no, (d_level *));
E xchar NDECL(maxledgerno);
E schar FDECL(depth, (d_level *));
E xchar FDECL(dunlev, (d_level *));
E xchar FDECL(dunlevs_in_dungeon, (d_level *));
E xchar FDECL(ledger_to_dnum, (XCHAR_P));
E xchar FDECL(ledger_to_dlev, (XCHAR_P));
E xchar FDECL(deepest_lev_reached, (BOOLEAN_P));
E boolean FDECL(on_level, (d_level *,d_level *));
E void FDECL(next_level, (BOOLEAN_P));
E void FDECL(prev_level, (BOOLEAN_P));
E void FDECL(u_on_newpos, (int,int));
E void NDECL(u_on_sstairs);
E void NDECL(u_on_upstairs);
E void NDECL(u_on_dnstairs);
E boolean FDECL(On_stairs, (XCHAR_P,XCHAR_P));
E void FDECL(get_level, (d_level *,int));
E boolean FDECL(Is_botlevel, (d_level *));
E boolean FDECL(Can_fall_thru, (d_level *));
E boolean FDECL(Can_dig_down, (d_level *));
E boolean FDECL(Can_rise_up, (int,int,d_level *));
E boolean FDECL(In_quest, (d_level *));
E boolean FDECL(In_outdoors, (d_level *));
E boolean FDECL(In_cave, (d_level *));
E boolean FDECL(In_mines, (d_level *));
E boolean FDECL(In_mines_quest, (d_level *));
E boolean FDECL(In_neu, (d_level *));
E boolean FDECL(In_outlands, (d_level *));
E boolean FDECL(In_cha, (d_level *));
E boolean FDECL(In_law, (d_level *));
E branch *FDECL(dungeon_branch, (const char *));
E boolean FDECL(at_dgn_entrance, (const char *));
E boolean FDECL(In_hell, (d_level *));
E boolean FDECL(In_V_tower, (d_level *));
E boolean FDECL(On_W_tower_level, (d_level *));
E boolean FDECL(In_W_tower, (int,int,d_level *));
E void FDECL(find_hell, (d_level *));
E void FDECL(goto_hell, (BOOLEAN_P,BOOLEAN_P));
E void FDECL(assign_level, (d_level *,d_level *));
E void FDECL(assign_rnd_level, (d_level *,d_level *,int));
E int FDECL(induced_align, (int));
E boolean FDECL(Invocation_lev, (d_level *));
E xchar NDECL(level_difficulty);
E schar FDECL(lev_by_name, (const char *));
#ifdef WIZARD
E schar FDECL(print_dungeon, (BOOLEAN_P,schar *,xchar *));
#endif
E int NDECL(donamelevel);
E int NDECL(dooverview);
E void FDECL(forget_mapseen, (int));
E void FDECL(init_mapseen, (d_level *));
E void NDECL(recalc_mapseen);
E void FDECL(recbranch_mapseen, (d_level *, d_level *));
E void FDECL(remdun_mapseen, (int));

/* ### eat.c ### */

#ifdef USE_TRAMPOLI
E int NDECL(eatmdone);
E int NDECL(eatfood);
E int NDECL(opentin);
E int NDECL(unfaint);
#endif
E void FDECL(cprefx, (int, BOOLEAN_P, BOOLEAN_P));
E void FDECL(cpostfx, (int, BOOLEAN_P, BOOLEAN_P, BOOLEAN_P));
E boolean FDECL(is_edible, (struct obj *));
E void NDECL(init_uhunger);
E void NDECL(reset_uhunger);
E int NDECL(Hear_again);
E void NDECL(reset_eat);
E int NDECL(doeat);
E void NDECL(gethungry);
E void FDECL(morehungry, (int));
E void FDECL(lesshungry, (int));
E boolean NDECL(is_fainted);
E void NDECL(reset_faint);
E void NDECL(violated_vegetarian);
#if 0
E void NDECL(sync_hunger);
#endif
E void FDECL(newuhs, (BOOLEAN_P));
E struct obj *FDECL(floorfood, (const char *,int));
E void NDECL(vomit);
E int FDECL(eaten_stat, (int,struct obj *));
E void FDECL(food_disappears, (struct obj *));
E void FDECL(food_substitution, (struct obj *,struct obj *));
E boolean FDECL(bite_monster, (struct monst *));
E void NDECL(fix_petrification);
E void FDECL(consume_oeaten, (struct obj *,int));
E boolean FDECL(maybe_finished_meal, (BOOLEAN_P));
E int FDECL(ask_turns, (struct monst *, int, int));
E int FDECL(ask_cp, (struct monst *, int));
E int FDECL(start_clockwinding, (struct obj *,struct monst *, int));

/* ### end.c ### */

E void FDECL(done1, (int));
E int NDECL(done2);
#ifdef USE_TRAMPOLI
E void FDECL(done_intr, (int));
#endif
E void FDECL(done_in_by, (struct monst *));
#endif /* !MAKEDEFS_C && !LEV_LEX_C */
E void VDECL(panic, (const char *,...)) PRINTF_F(1,2);
#if !defined(MAKEDEFS_C) && !defined(LEV_LEX_C)
E void FDECL(done, (int));
E void FDECL(container_contents, (struct obj *,BOOLEAN_P,BOOLEAN_P));
E void FDECL(terminate, (int));
E int NDECL(num_genocides);

/* ### engrave.c ### */

E void FDECL(bindspirit, (int));
E void NDECL(blank_all_wards);
E void NDECL(randomize_all_wards);
E int NDECL(pick_seal);
E int FDECL(pick_ward, (boolean));
E void FDECL(describe_ward, (int));
E int NDECL(randHaluWard);
E int NDECL(random_unknown_ward);
E int FDECL(get_num_wards_added, (int, int));
E char *FDECL(random_engraving, (char *));
E int NDECL(random_haluIndex);
E const char *FDECL(fetchHaluWard, (int));
E void FDECL(wipeout_text, (char *,int,unsigned));
E boolean NDECL(can_reach_floor);
E struct engr *NDECL(get_head_engr);
E const char *FDECL(surface, (int,int));
E const char *FDECL(ceiling, (int,int));
E struct engr *FDECL(engr_at, (XCHAR_P,XCHAR_P));
E void NDECL(move_gliders);
E int FDECL(sengr_at, (const char *,XCHAR_P,XCHAR_P));
E int FDECL(ward_at, (XCHAR_P,XCHAR_P));
E int FDECL(num_wards_at, (XCHAR_P,XCHAR_P));
E void FDECL(u_wipe_engr, (int));
E int FDECL(decode_wardID, (int));
E int FDECL(get_wardID, (int));
E void FDECL(wipe_engr_at, (XCHAR_P,XCHAR_P,XCHAR_P));
E void FDECL(read_engr_at, (int,int));
E void FDECL(make_engr_at, (int,int,const char *,long,XCHAR_P));
E void FDECL(del_engr_at, (int,int));
E void FDECL(del_ward_at, (int,int));
E void FDECL(del_engr_ward_at, (int,int));
E int NDECL(freehand);
E int NDECL(doengrave);
E int NDECL(doward);
E int NDECL(doseal);
E int NDECL(doengward);
E void FDECL(save_engravings, (int,int));
E void FDECL(rest_engravings, (int));
E void FDECL(del_engr, (struct engr *));
E void FDECL(del_ward, (struct engr *));
E void FDECL(del_engr_ward, (struct engr *));
E void FDECL(rloc_engr, (struct engr *));
E void FDECL(make_grave, (int,int,const char *));
E boolean FDECL(allied_faction, (int,int));

/* ### exper.c ### */

E int FDECL(experience, (struct monst *,int));
E int FDECL(ptrexperience, (struct permonst *));
E void FDECL(more_experienced, (int,int));
E void FDECL(losexp, (const char *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P));
E void NDECL(newexplevel);
E void NDECL(binderup);
E void FDECL(pluslvl, (BOOLEAN_P));
E long FDECL(rndexp, (BOOLEAN_P));

/* ### explode.c ### */

E void FDECL(explode, (int,int,int,int,CHAR_P,int));
E void FDECL(explode2, (int,int,int,int,CHAR_P,int));
E void FDECL(big_explode, (int,int,int,int,CHAR_P,int,int));
E long FDECL(scatter, (int, int, int, unsigned int, struct obj *));
E void FDECL(splatter_burning_oil, (int, int));
//#ifdef FIREARMS
E void FDECL(grenade_explode, (struct obj *, int, int, BOOLEAN_P, int));
E void FDECL(arm_bomb, (struct obj *, BOOLEAN_P));
//#endif

/* ### extralev.c ### */

#ifdef REINCARNATION
E void NDECL(makeroguerooms);
E void FDECL(corr, (int,int));
E void NDECL(makerogueghost);
#endif

/* ### files.c ### */

E char *FDECL(fname_encode, (const char *, CHAR_P, char *, char *, int));
E char *FDECL(fname_decode, (CHAR_P, char *, char *, int));
E const char *FDECL(fqname, (const char *, int, int));
E FILE *FDECL(fopen_datafile, (const char *,const char *,int));
E boolean FDECL(uptodate, (int,const char *));
E void FDECL(store_version, (int));
#ifdef MFLOPPY
E void NDECL(set_lock_and_bones);
#endif
E void FDECL(set_levelfile_name, (char *,int));
E int FDECL(create_levelfile, (int,char *));
E int FDECL(open_levelfile, (int,char *));
E void FDECL(delete_levelfile, (int));
E void NDECL(clearlocks);
E int FDECL(create_bonesfile, (d_level*,char **, char *));
#ifdef MFLOPPY
E void NDECL(cancel_bonesfile);
#endif
E void FDECL(commit_bonesfile, (d_level *));
E int FDECL(open_bonesfile, (d_level*,char **));
E int FDECL(delete_bonesfile, (d_level*));
E void NDECL(compress_bonesfile);
E void NDECL(set_savefile_name);
#ifdef INSURANCE
E void FDECL(save_savefile_name, (int));
#endif
#if defined(WIZARD) && !defined(MICRO)
E void NDECL(set_error_savefile);
#endif
E int NDECL(create_savefile);
E int NDECL(open_savefile);
E int NDECL(delete_savefile);
E int NDECL(restore_saved_game);
E void FDECL(compress, (const char *));
E void FDECL(uncompress, (const char *));
E boolean FDECL(lock_file, (const char *,int,int));
E void FDECL(unlock_file, (const char *));
#ifdef USER_SOUNDS
E boolean FDECL(can_read_file, (const char *));
#endif
E void FDECL(read_config_file, (const char *));
E void FDECL(check_recordfile, (const char *));
#if defined(WIZARD)
E void NDECL(read_wizkit);
#endif
E void FDECL(paniclog, (const char *, const char *));
E int FDECL(validate_prefix_locations, (char *));
E char** NDECL(get_saved_games);
E void FDECL(free_saved_games, (char**));
#ifdef SELF_RECOVER
E boolean NDECL(recover_savefile);
#endif
#ifdef HOLD_LOCKFILE_OPEN
E void NDECL(really_close);
#endif

/* ### fountain.c ### */

E void FDECL(floating_above, (const char *));
E void FDECL(dogushforth, (int));
# ifdef USE_TRAMPOLI
E void FDECL(gush, (int,int,genericptr_t));
# endif
E void FDECL(dryup, (XCHAR_P,XCHAR_P, BOOLEAN_P));
E void NDECL(drinkfountain);
E void FDECL(dipfountain, (struct obj *));
#ifdef SINKS
E void FDECL(breaksink, (int,int));
E void NDECL(drinksink);
#endif

/* ### hack.c ### */

E int FDECL(max_ints, (int,int));
E int FDECL(min_ints, (int,int));
E boolean FDECL(revive_nasty, (int,int,const char*));
E void FDECL(movobj, (struct obj *,XCHAR_P,XCHAR_P));
E boolean FDECL(may_dig, (XCHAR_P,XCHAR_P));
E boolean FDECL(may_passwall, (XCHAR_P,XCHAR_P));
E boolean FDECL(bad_rock, (struct permonst *,XCHAR_P,XCHAR_P));
E boolean FDECL(invocation_pos, (XCHAR_P,XCHAR_P));
E boolean FDECL(test_move, (int, int, int, int, int));
E void NDECL(domove);
E void NDECL(invocation_message);
E void FDECL(spoteffects, (BOOLEAN_P));
E char *FDECL(in_rooms, (XCHAR_P,XCHAR_P,int));
E boolean FDECL(in_town, (int,int));
E void FDECL(check_special_room, (BOOLEAN_P));
E int NDECL(dopickup);
E void NDECL(lookaround);
E int NDECL(monster_nearby);
E void FDECL(nomul, (int, const char *));
E void FDECL(unmul, (const char *));
E void FDECL(showdmg, (int, BOOLEAN_P));
E void FDECL(losehp, (int,const char *,BOOLEAN_P));
E void FDECL(set_uhpmax, (int,BOOLEAN_P));
E void NDECL(check_uhpmax);
E int NDECL(weight_cap);
E int NDECL(inv_weight);
E int NDECL(near_capacity);
E int FDECL(calc_capacity, (int));
E int NDECL(max_capacity);
E boolean FDECL(check_capacity, (const char *));
E int NDECL(inv_cnt);
#ifdef GOLDOBJ
E long FDECL(money_cnt, (struct obj *));
#endif

/* ### hacklib.c ### */

E boolean FDECL(digit, (CHAR_P));
E boolean FDECL(letter, (CHAR_P));
E char FDECL(highc, (CHAR_P));
E char FDECL(lowc, (CHAR_P));
E char *FDECL(lcase, (char *));
E char *FDECL(upstart, (char *));
E char *FDECL(mungspaces, (char *));
E char *FDECL(eos, (char *));
E char *FDECL(strkitten, (char *,CHAR_P));
E char *FDECL(s_suffix, (const char *));
E char *FDECL(xcrypt, (const char *,char *));
E boolean FDECL(onlyspace, (const char *));
E char *FDECL(tabexpand, (char *));
E char *FDECL(visctrl, (CHAR_P));
E const char *FDECL(ordin, (int));
E char *FDECL(sitoa, (int));
E int FDECL(sgn, (int));
E int FDECL(rounddiv, (long,int));
E int FDECL(dist2, (int,int,int,int));
E int FDECL(distmin, (int,int,int,int));
E boolean FDECL(online2, (int,int,int,int));
E boolean FDECL(pmatch, (const char *,const char *));
#ifndef STRNCMPI
E int FDECL(strncmpi, (const char *,const char *,int));
#endif
#ifndef STRSTRI
E char *FDECL(strstri, (const char *,const char *));
#endif
E boolean FDECL(fuzzymatch, (const char *,const char *,const char *,BOOLEAN_P));
E void NDECL(setrandom);
E int NDECL(getyear);
#if 0
E char *FDECL(yymmdd, (time_t));
#endif
E long FDECL(yyyymmdd, (time_t));
E int NDECL(phase_of_the_moon);
E boolean NDECL(friday_13th);
E int NDECL(night);
E int NDECL(midnight);

/* ### invent.c ### */

E void FDECL(assigninvlet, (struct obj *));
E struct obj *FDECL(merge_choice, (struct obj *,struct obj *));
E int FDECL(merged, (struct obj **,struct obj **));
#ifdef USE_TRAMPOLI
E int FDECL(ckunpaid, (struct obj *));
#endif
E void FDECL(addinv_core1, (struct obj *));
E void FDECL(addinv_core2, (struct obj *));
E struct obj *FDECL(addinv, (struct obj *));
E struct obj *FDECL(hold_another_object,
			(struct obj *,const char *,const char *,const char *));
E void FDECL(useupall, (struct obj *));
E void FDECL(useup, (struct obj *));
E void FDECL(consume_obj_charge, (struct obj *,BOOLEAN_P));
E void FDECL(freeinv_core, (struct obj *));
E void FDECL(freeinv, (struct obj *));
E void FDECL(delallobj, (int,int));
E void FDECL(delobj, (struct obj *));
E struct obj *FDECL(sobj_at, (int,int,int));
E struct obj *FDECL(boulder_at, (int,int));
E struct obj *FDECL(toustefna_at, (int,int));
E struct obj *FDECL(dreprun_at, (int,int));
E struct obj *FDECL(veioistafur_at, (int,int));
E struct obj *FDECL(thjofastafur_at, (int,int));
E struct obj *FDECL(aligned_sartprop3_at, (int,int,int));
E struct obj *FDECL(carrying, (int));
E boolean NDECL(have_lizard);
E struct obj *FDECL(o_on, (unsigned int,struct obj *));
E boolean FDECL(obj_here, (struct obj *,int,int));
E boolean NDECL(wearing_armor);
E boolean FDECL(is_worn, (struct obj *));
E struct obj *FDECL(g_at, (int,int));
E struct obj *FDECL(mkgoldobj, (long));
E struct obj *FDECL(getobj, (const char *,const char *));
E int FDECL(ggetobj, (const char *,int (*)(OBJ_P),int,BOOLEAN_P,unsigned *));
E void FDECL(fully_identify_obj, (struct obj *));
E int FDECL(identify, (struct obj *));
E void FDECL(identify_pack, (int));
E int FDECL(askchain, (struct obj **,const char *,int,int (*)(OBJ_P),
			int (*)(OBJ_P),int,const char *));
E void FDECL(prinv, (const char *,struct obj *,long));
E char *FDECL(xprname, (struct obj *,const char *,CHAR_P,BOOLEAN_P,long,long));
E int NDECL(ddoinv);
E char FDECL(display_inventory, (const char *,BOOLEAN_P));
E int FDECL(display_binventory, (int,int,BOOLEAN_P));
E struct obj *FDECL(display_cinventory,(struct obj *));
E struct obj *FDECL(display_minventory,(struct monst *,int,char *));
E int NDECL(dotypeinv);
E const char *FDECL(dfeature_at, (int,int,char *));
E int FDECL(look_here, (int,BOOLEAN_P));
E int NDECL(dolook);
E boolean FDECL(will_feel_cockatrice, (struct obj *,BOOLEAN_P));
E void FDECL(feel_cockatrice, (struct obj *,BOOLEAN_P));
E void FDECL(stackobj, (struct obj *));
E int NDECL(doprgold);
E int NDECL(doprwep);
E int NDECL(doprarm);
E int NDECL(doprring);
E int NDECL(dopramulet);
E int NDECL(doprtool);
E int NDECL(doprinuse);
E void FDECL(useupf, (struct obj *,long));
E char *FDECL(let_to_name, (CHAR_P,BOOLEAN_P));
E char *NDECL(rand_class_name);
E void NDECL(free_invbuf);
E void NDECL(reassign);
E int NDECL(doorganize);
E int FDECL(count_unpaid, (struct obj *));
E int FDECL(count_buc, (struct obj *,int));
E void FDECL(carry_obj_effects, (struct obj *));
E const char *FDECL(currency, (long));
E void FDECL(silly_thing, (const char *,struct obj *));
E int NDECL(u_healing_penalty);

/* ### ioctl.c ### */

#if defined(UNIX) || defined(__BEOS__)
E void NDECL(getwindowsz);
E void NDECL(getioctls);
E void NDECL(setioctls);
# ifdef SUSPEND
E int NDECL(dosuspend);
# endif /* SUSPEND */
#endif /* UNIX || __BEOS__ */

/* ### light.c ### */

E void FDECL(new_light_source, (XCHAR_P, XCHAR_P, int, int, genericptr_t));
E void FDECL(del_light_source, (int, genericptr_t, boolean));
E void FDECL(do_light_sources, (char **));
E struct monst *FDECL(find_mid, (unsigned, unsigned));
E void FDECL(save_light_sources, (int, int, int));
E void FDECL(restore_light_sources, (int));
E void FDECL(relink_light_sources, (BOOLEAN_P));
E void FDECL(obj_move_light_source, (struct obj *, struct obj *));
E boolean NDECL(any_light_source);
E void FDECL(snuff_light_source, (int, int));
E boolean FDECL(obj_sheds_light, (struct obj *));
E boolean FDECL(obj_is_burning, (struct obj *));
E boolean FDECL(litsaber, (struct obj *));
E void FDECL(obj_split_light_source, (struct obj *, struct obj *));
E void FDECL(obj_merge_light_sources, (struct obj *,struct obj *));
E int FDECL(candle_light_range, (struct obj *));
#ifdef WIZARD
E int NDECL(wiz_light_sources);
#endif

/* ### lock.c ### */

#ifdef USE_TRAMPOLI
E int NDECL(forcelock);
E int NDECL(picklock);
#endif
E boolean FDECL(picking_lock, (int *,int *));
E boolean FDECL(picking_at, (int,int));
E void NDECL(reset_pick);
E int FDECL(pick_lock, (struct obj *));
E int NDECL(doforce);
E boolean FDECL(boxlock, (struct obj *,struct obj *));
E boolean FDECL(doorlock, (struct obj *,int,int));
E int NDECL(doopen);
E int FDECL(doopen_indir, (int,int));
E int NDECL(doclose);
E int FDECL(artifact_door, (int,int));

#ifdef MAC
/* These declarations are here because the main code calls them. */

/* ### macfile.c ### */

E int FDECL(maccreat, (const char *,long));
E int FDECL(macopen, (const char *,int,long));
E int FDECL(macclose, (int));
E int FDECL(macread, (int,void *,unsigned));
E int FDECL(macwrite, (int,void *,unsigned));
E long FDECL(macseek, (int,long,short));
E int FDECL(macunlink, (const char *));

/* ### macsnd.c ### */

E void FDECL(mac_speaker, (struct obj *,char *));

/* ### macunix.c ### */

E void FDECL(regularize, (char *));
E void NDECL(getlock);

/* ### macwin.c ### */

E void FDECL(lock_mouse_cursor, (Boolean));
E int NDECL(SanePositions);

/* ### mttymain.c ### */

E void FDECL(getreturn, (char *));
E void VDECL(msmsg, (const char *,...));
E void NDECL(gettty);
E void NDECL(setftty);
E void FDECL(settty, (const char *));
E int NDECL(tgetch);
E void FDECL(cmov, (int x, int y));
E void FDECL(nocmov, (int x, int y));

#endif /* MAC */

/* ### mail.c ### */

#ifdef MAIL
# ifdef UNIX
E void NDECL(getmailstatus);
# endif
E void NDECL(ckmailstatus);
E void FDECL(readmail, (struct obj *));
#endif /* MAIL */

/* ### makemon.c ### */

E void FDECL(m_initgrp,(struct monst *,int,int,int));
E boolean FDECL(is_home_elemental, (struct permonst *));
E struct monst *FDECL(clone_mon, (struct monst *,XCHAR_P,XCHAR_P));
E struct monst *FDECL(makemon, (struct permonst *,int,int,int));
E struct monst *FDECL(makeundead, (struct permonst *,int,int,int,int));
E boolean FDECL(create_critters, (int,struct permonst *));
E struct permonst *NDECL(rndmonst);
E int NDECL(rndshape);
E void FDECL(reset_rndmonst, (int));
E struct permonst *FDECL(mkclass, (CHAR_P,int));
E struct permonst *NDECL(mkzombie);
E int FDECL(adj_lev, (struct permonst *));
E struct permonst *FDECL(grow_up, (struct monst *,struct monst *));
E int FDECL(mongets, (struct monst *,int));
E int FDECL(golemhp, (int));
E boolean FDECL(peace_minded, (struct permonst *));
E void FDECL(set_malign, (struct monst *));
E void FDECL(set_mimic_sym, (struct monst *));
E void FDECL(set_curhouse, (int));
E int FDECL(mbirth_limit, (int));
E void FDECL(mimic_hit_msg, (struct monst *, SHORT_P));
#ifdef GOLDOBJ
E void FDECL(mkmonmoney, (struct monst *, long));
#endif
E void FDECL(bagotricks, (struct obj *));
E boolean FDECL(propagate, (int, BOOLEAN_P,BOOLEAN_P));
E boolean FDECL(mon_can_see_you, (struct monst *));
E boolean FDECL(mon_can_see_mon, (struct monst *, struct monst *));

/* ### mapglyph.c ### */

E void FDECL(mapglyph, (int, int *, int *, unsigned *, int, int));

/* ### mcastu.c ### */

E int FDECL(castmu, (struct monst *,struct attack *,BOOLEAN_P,BOOLEAN_P));
E int FDECL(buzzmu, (struct monst *,struct attack *, int));
E int FDECL(buzzmm, (struct monst *,struct monst *,struct attack *, int));
E int FDECL(castmm, (struct monst *,struct monst *,struct attack *));
E int FDECL(castum, (struct monst *,struct attack *));
E void FDECL(cast_spell, (struct monst *, int,int));

/* ### mhitm.c ### */

E int FDECL(fightm, (struct monst *));
E int FDECL(mattackm, (struct monst *,struct monst *));
E int FDECL(noattacks, (struct permonst *));
E int FDECL(sleep_monst, (struct monst *,int,int));
E void FDECL(slept_monst, (struct monst *));
E long FDECL(attk_protection, (int));
E int FDECL(thrwmm, (struct monst *, struct monst *));
E int FDECL(gazemm, (struct monst *,struct monst *,struct attack *));

/* ### mhitu.c ### */

E boolean FDECL(diseasemu, (struct permonst *));
E const char *FDECL(mpoisons_subj, (struct monst *,struct attack *));
E void NDECL(u_slow_down);
E struct monst *NDECL(cloneu);
E void FDECL(expels, (struct monst *,struct permonst *,BOOLEAN_P));
E struct attack *FDECL(getmattk, (struct permonst *,int,int *,struct attack *));
E int FDECL(mattacku, (struct monst *));
E int FDECL(magic_negation, (struct monst *));
E int FDECL(gazemu, (struct monst *,struct attack *));
E void FDECL(mdamageu, (struct monst *,int));
E int FDECL(could_seduce, (struct monst *,struct monst *,struct attack *));
#ifdef SEDUCE
E int FDECL(doseduce, (struct monst *));
E int FDECL(dolilithseduce, (struct monst *));
E int FDECL(domlcseduce, (struct monst *));
E int FDECL(dosflseduce, (struct monst *));
E int FDECL(dopaleseduce, (struct monst *));
E int FDECL(dotent, (struct monst *, int));
#endif

/* ### minion.c ### */

E void FDECL(msummon, (struct monst *));
E struct monst * FDECL(summon_god_minion, (const char *,ALIGNTYP_P,BOOLEAN_P));
E struct monst * FDECL(summon_minion, (ALIGNTYP_P,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P));
E int FDECL(demon_talk, (struct monst *));
E long FDECL(bribe, (struct monst *));
E int FDECL(dprince, (struct permonst *, ALIGNTYP_P));
E int FDECL(dlord, (struct permonst *, ALIGNTYP_P));
E int NDECL(llord);
E int NDECL(nlord);
E int NDECL(clord);
E int FDECL(ndemon, (ALIGNTYP_P));
E int NDECL(lminion);
E int NDECL(nminion);
E int NDECL(cminion);

/* ### mklev.c ### */

#ifdef USE_TRAMPOLI
E int FDECL(do_comp, (genericptr_t,genericptr_t));
#endif
E void NDECL(sort_rooms);
E void FDECL(add_room, (int,int,int,int,BOOLEAN_P,SCHAR_P,BOOLEAN_P));
E void FDECL(add_subroom, (struct mkroom *,int,int,int,int,
			   BOOLEAN_P,SCHAR_P,BOOLEAN_P));
E void NDECL(makecorridors);
E int FDECL(add_door, (int,int,struct mkroom *));
E void NDECL(mkpoolroom);
E void NDECL(mklev);
#ifdef SPECIALIZATION
E void FDECL(topologize, (struct mkroom *,BOOLEAN_P));
#else
E void FDECL(topologize, (struct mkroom *));
#endif
E void FDECL(place_branch, (branch *,XCHAR_P,XCHAR_P));
E boolean FDECL(occupied, (XCHAR_P,XCHAR_P));
E int FDECL(okdoor, (XCHAR_P,XCHAR_P));
E void FDECL(dodoor, (int,int,struct mkroom *));
E void FDECL(mktrap, (int,int,struct mkroom *,coord*));
E void FDECL(mkstairs, (XCHAR_P,XCHAR_P,CHAR_P,struct mkroom *));
E void NDECL(mkinvokearea);

/* ### mkmap.c ### */

void FDECL(flood_fill_rm, (int,int,int,BOOLEAN_P,BOOLEAN_P));
void FDECL(remove_rooms, (int,int,int,int));

/* ### mkmaze.c ### */

E void FDECL(wallification, (int,int,int,int));
E void FDECL(walkfrom, (int,int));
E void FDECL(makemaz, (const char *));
E void FDECL(mazexy, (coord *));
E void NDECL(bound_digging);
E void FDECL(mkportal, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
E boolean FDECL(bad_location, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
E void FDECL(place_lregion, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,
			     XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,
			     XCHAR_P,d_level *));
E void NDECL(movebubbles);
E void NDECL(water_friction);
E void FDECL(save_waterlevel, (int,int));
E void FDECL(restore_waterlevel, (int));
E const char *FDECL(waterbody_name, (XCHAR_P,XCHAR_P));

/* ### mkobj.c ### */

E struct obj *FDECL(mkobj_at, (CHAR_P,int,int,BOOLEAN_P));
E struct obj *FDECL(mksobj_at, (int,int,int,BOOLEAN_P,BOOLEAN_P));
E struct obj *FDECL(mkobj, (CHAR_P,BOOLEAN_P));
E int NDECL(rndmonnum);
E struct obj *FDECL(splitobj, (struct obj *,long));
E void FDECL(replace_object, (struct obj *,struct obj *));
E void FDECL(bill_dummy_object, (struct obj *));
E struct obj *FDECL(mksobj, (int,BOOLEAN_P,BOOLEAN_P));
E int FDECL(bcsign, (struct obj *));
E void FDECL(set_material, (struct obj *, int));
E int FDECL(weight, (struct obj *));
E struct obj *FDECL(mkgold, (long,int,int));
E struct obj *FDECL(mkcorpstat,
		(int,struct monst *,struct permonst *,int,int,BOOLEAN_P));
E struct obj *FDECL(obj_attach_mid, (struct obj *, unsigned));
E struct monst *FDECL(get_mtraits, (struct obj *, BOOLEAN_P));
E struct obj *FDECL(mk_tt_object, (int,int,int));
E struct obj *FDECL(mk_named_object,
			(int,struct permonst *,int,int,const char *));
E struct obj *FDECL(rnd_treefruit_at, (int, int));
E void FDECL(start_corpse_timeout, (struct obj *));
E void FDECL(bless, (struct obj *));
E void FDECL(unbless, (struct obj *));
E void FDECL(curse, (struct obj *));
E void FDECL(uncurse, (struct obj *));
E void FDECL(blessorcurse, (struct obj *,int));
E boolean FDECL(is_flammable, (struct obj *));
E boolean FDECL(is_rottable, (struct obj *));
E void FDECL(place_object, (struct obj *,int,int));
E void FDECL(remove_object, (struct obj *));
E void FDECL(discard_minvent, (struct monst *));
E void FDECL(obj_extract_self, (struct obj *));
E void FDECL(extract_magic_chest_nobj, (struct obj *));
E void FDECL(extract_nobj, (struct obj *, struct obj **));
E void FDECL(extract_nexthere, (struct obj *, struct obj **));
E int FDECL(add_to_minv, (struct monst *, struct obj *));
E struct obj *FDECL(add_to_magic_chest, (struct obj *, int));
E struct obj *FDECL(add_to_container, (struct obj *, struct obj *));
E void FDECL(add_to_migration, (struct obj *));
E void FDECL(add_to_buried, (struct obj *));
E void FDECL(dealloc_obj, (struct obj *));
E void FDECL(obj_ice_effects, (int, int, BOOLEAN_P));
E long FDECL(peek_at_iced_corpse_age, (struct obj *));
E void FDECL(doMaskStats, (struct obj *));
#ifdef WIZARD
E void NDECL(obj_sanity_check);
#endif

/* ### mkroom.c ### */

E void NDECL(mksepulcher);
E void NDECL(mkmivault);
E void FDECL(mkmivaultitem,(struct obj *));
E void NDECL(place_lolth_vaults);
E void NDECL(place_neutral_features);
E struct mkroom * FDECL(pick_room,(BOOLEAN_P));
E void FDECL(mkroom, (int));
E void FDECL(fill_zoo, (struct mkroom *));
E boolean FDECL(nexttodoor, (int,int));
E boolean FDECL(has_dnstairs, (struct mkroom *));
E boolean FDECL(has_upstairs, (struct mkroom *));
E int FDECL(somex, (struct mkroom *));
E int FDECL(somey, (struct mkroom *));
E boolean FDECL(inside_room, (struct mkroom *,XCHAR_P,XCHAR_P));
E boolean FDECL(somexy, (struct mkroom *,coord *));
E void FDECL(mkundead, (coord *,BOOLEAN_P,int));
E struct permonst *FDECL(courtmon,(int));
E struct permonst *NDECL(mivaultmon);
E void FDECL(save_rooms, (int));
E void FDECL(rest_rooms, (int));
E struct mkroom *FDECL(search_special, (SCHAR_P));

/* ### mon.c ### */

E void FDECL(remove_monster, (int, int));
E int FDECL(undead_to_corpse, (int));
E int FDECL(genus, (int,int));
E int FDECL(pm_to_cham, (int));
E int FDECL(minliquid, (struct monst *));
E int NDECL(movemon);
E int FDECL(meatmetal, (struct monst *));
E int FDECL(meatobj, (struct monst *));
E void FDECL(mpickgold, (struct monst *));
E boolean FDECL(mpickstuff, (struct monst *,const char *));
E int FDECL(curr_mon_load, (struct monst *));
E int FDECL(max_mon_load, (struct monst *));
E boolean FDECL(can_carry, (struct monst *,struct obj *));
E int FDECL(mfndpos, (struct monst *,coord *,long *,long));
E long FDECL(mm_aggression, (struct monst *,struct monst *));
E boolean FDECL(monnear, (struct monst *,int,int));
E void NDECL(dmonsfree);
E int FDECL(mcalcmove, (struct monst*));
E void NDECL(mcalcdistress);
E void FDECL(replmon, (struct monst *,struct monst *));
E void FDECL(relmon, (struct monst *));
E struct obj *FDECL(mlifesaver, (struct monst *));
E boolean FDECL(corpse_chance,(struct monst *,struct monst *,BOOLEAN_P));
E void FDECL(mondead, (struct monst *));
E void FDECL(spore_dies, (struct monst *));
E void FDECL(mondied, (struct monst *));
E void FDECL(monvanished, (struct monst *));
E void FDECL(mongone, (struct monst *));
E void FDECL(monstone, (struct monst *));
E void FDECL(mongolded, (struct monst *));
E void FDECL(monglassed, (struct monst *));
E void FDECL(monkilled, (struct monst *,const char *,int));
E void FDECL(unstuck, (struct monst *));
E void FDECL(killed, (struct monst *));
E void FDECL(xkilled, (struct monst *,int));
E void FDECL(mon_to_stone, (struct monst*));
E void FDECL(mnexto, (struct monst *));
E void FDECL(monline, (struct monst *));
E void FDECL(mofflin, (struct monst *));
E boolean FDECL(mnearto, (struct monst *,XCHAR_P,XCHAR_P,BOOLEAN_P));
E void FDECL(poisontell, (int));
E void FDECL(poisoned, (const char *,int,const char *,int,int));
E void FDECL(m_respond, (struct monst *));
E void FDECL(setmangry, (struct monst *));
E void FDECL(wakeup, (struct monst *, boolean));
E void NDECL(wake_nearby);
E void NDECL(wake_nearby_noisy);
E void FDECL(wake_nearto, (int,int,int));
E void FDECL(wake_nearto_noisy, (int,int,int));
E void FDECL(seemimic, (struct monst *));
E void NDECL(rescham);
E void NDECL(restartcham);
E void FDECL(restore_cham, (struct monst *));
E void FDECL(mon_animal_list, (BOOLEAN_P));
E int FDECL(newcham, (struct monst *,struct permonst *,BOOLEAN_P,BOOLEAN_P));
E int FDECL(can_be_hatched, (int));
E int FDECL(egg_type_from_parent, (int,BOOLEAN_P));
E boolean FDECL(dead_species, (int,BOOLEAN_P));
E void NDECL(kill_genocided_monsters);
E void FDECL(golemeffects, (struct monst *,int,int));
E boolean FDECL(angry_guards, (BOOLEAN_P));
E void NDECL(pacify_guards);

/* ### mondata.c ### */

E void FDECL(set_mon_data, (struct monst *,struct permonst *,int));
E struct attack *FDECL(attacktype_fordmg, (struct permonst *,int,int));
E boolean FDECL(attacktype, (struct permonst *,int));
E int FDECL(attackindex, (struct permonst *,int,int));
E boolean FDECL(poly_when_stoned, (struct permonst *));
E boolean FDECL(resists_drli, (struct monst *));
E boolean FDECL(resists_oona, (struct monst *));
E boolean FDECL(resists_fire, (struct monst *));
E boolean FDECL(resists_cold, (struct monst *));
E boolean FDECL(resists_sleep, (struct monst *));
E boolean FDECL(resists_disint, (struct monst *));
E boolean FDECL(resists_elec, (struct monst *));
E boolean FDECL(resists_poison, (struct monst *));
E boolean FDECL(resists_acid, (struct monst *));
E boolean FDECL(resists_ston, (struct monst *));
E boolean FDECL(resists_drain, (struct monst *));
E boolean FDECL(resists_sickness, (struct monst *));
E boolean FDECL(resists_magm, (struct monst *));
E boolean FDECL(resists_death, (struct monst *));
E boolean FDECL(resists_blnd, (struct monst *));
E boolean FDECL(can_blnd, (struct monst *,struct monst *,UCHAR_P,struct obj *));
E boolean FDECL(ranged_attk, (struct permonst *));
E boolean FDECL(passes_bars, (struct permonst *));
E boolean FDECL(can_track, (struct permonst *));
E boolean FDECL(sticks, (struct permonst *));
E int FDECL(num_horns, (struct permonst *));
/* E boolean FDECL(canseemon, (struct monst *)); */
E struct attack *FDECL(dmgtype_fromattack, (struct permonst *,int,int));
E boolean FDECL(dmgtype, (struct permonst *,int));
E int FDECL(max_passive_dmg, (struct monst *,struct monst *));
E int FDECL(monsndx, (struct permonst *));
E int FDECL(name_to_mon, (const char *));
E int FDECL(gender, (struct monst *));
E int FDECL(pronoun_gender, (struct monst *));
E boolean FDECL(levl_follower, (struct monst *));
E int FDECL(little_to_big, (int,boolean));
E int FDECL(big_to_little, (int));
E const char *FDECL(locomotion, (const struct permonst *,const char *));
E const char *FDECL(stagger, (const struct permonst *,const char *));
E const char *FDECL(on_fire, (struct permonst *,struct attack *));
E const struct permonst *FDECL(raceptr, (struct monst *));

/* ### monmove.c ### */

E boolean FDECL(scaryElb, (struct monst *));
E boolean FDECL(scaryLol, (struct monst *));
E boolean FDECL(scaryItem, (struct monst *));
E boolean FDECL(scaryYellow, (int, struct monst *));
E boolean FDECL(scaryHept, (int, struct monst *));
E boolean FDECL(scaryGorg, (int, struct monst *));
E boolean FDECL(scaryCircle, (int, struct monst *));
E boolean FDECL(scaryPent, (int, struct monst *));
E boolean FDECL(scaryHex, (int, struct monst *));
E boolean FDECL(scaryHam, (int, struct monst *));
E boolean FDECL(scarySign, (int, struct monst *));
E boolean FDECL(scaryEye, (int, struct monst *));
E boolean FDECL(scaryQueen, (int, struct monst *));
E boolean FDECL(scaryCat, (int, struct monst *));
E boolean FDECL(scaryWings, (int, struct monst *));
E boolean FDECL(scaryTou, (struct monst *));
E boolean FDECL(scaryDre, (struct monst *));
E boolean FDECL(scaryVei, (struct monst *));
E boolean FDECL(scaryThj, (struct monst *));
E boolean FDECL(itsstuck, (struct monst *));
E boolean FDECL(mb_trapped, (struct monst *));
E void FDECL(mon_regen, (struct monst *,BOOLEAN_P));
E int FDECL(dochugw, (struct monst *));
E boolean FDECL(onscary, (int,int,struct monst *));
E void FDECL(monflee, (struct monst *, int, BOOLEAN_P, BOOLEAN_P));
E int FDECL(dochug, (struct monst *));
E int FDECL(m_move, (struct monst *,int));
E void FDECL(dissolve_bars, (int,int));
E boolean FDECL(closed_door, (int,int));
E boolean FDECL(accessible, (int,int));
E void FDECL(set_apparxy, (struct monst *));
E boolean FDECL(can_ooze, (struct monst *));

/* ### monst.c ### */

E void NDECL(monst_init);

/* ### monstr.c ### */

E void NDECL(monstr_init);

/* ### mplayer.c ### */

E struct monst *FDECL(mk_mplayer, (struct permonst *,XCHAR_P,
				   XCHAR_P,BOOLEAN_P));
E void FDECL(create_mplayers, (int,BOOLEAN_P));
E void FDECL(mplayer_talk, (struct monst *));

#if defined(MICRO) || defined(WIN32)

/* ### msdos.c,os2.c,tos.c,winnt.c ### */

#  ifndef WIN32
E int NDECL(tgetch);
#  endif
#  ifndef TOS
E char NDECL(switchar);
#  endif
# ifndef __GO32__
E long FDECL(freediskspace, (char *));
#  ifdef MSDOS
E int FDECL(findfirst_file, (char *));
E int NDECL(findnext_file);
E long FDECL(filesize_nh, (char *));
#  else
E int FDECL(findfirst, (char *));
E int NDECL(findnext);
E long FDECL(filesize, (char *));
#  endif /* MSDOS */
E char *NDECL(foundfile_buffer);
# endif /* __GO32__ */
E void FDECL(chdrive, (char *));
# ifndef TOS
E void NDECL(disable_ctrlP);
E void NDECL(enable_ctrlP);
# endif
# if defined(MICRO) && !defined(WINNT)
E void NDECL(get_scr_size);
#  ifndef TOS
E void FDECL(gotoxy, (int,int));
#  endif
# endif
# ifdef TOS
E int FDECL(_copyfile, (char *,char *));
E int NDECL(kbhit);
E void NDECL(set_colors);
E void NDECL(restore_colors);
#  ifdef SUSPEND
E int NDECL(dosuspend);
#  endif
# endif /* TOS */
# ifdef WIN32
E char *FDECL(get_username, (int *));
E void FDECL(nt_regularize, (char *));
E int NDECL((*nt_kbhit));
E void FDECL(Delay, (int));
# endif /* WIN32 */
#endif /* MICRO || WIN32 */

/* ### mthrowu.c ### */

E int FDECL(set_bypassDR, (int));
E int FDECL(set_destroy_thrown, (int));
E int FDECL(thitu, (int,int,struct obj *,const char *, boolean));
E int FDECL(ohitmon, (struct monst *,struct monst *,struct obj *,int,BOOLEAN_P));
E void FDECL(thrwmu, (struct monst *));
E int FDECL(spitmu, (struct monst *,struct attack *));
E int FDECL(firemu, (struct monst *,struct attack *));
E int FDECL(firemm, (struct monst *,struct monst *,struct attack *));
/*E int FDECL(firemm, (struct monst *,struct attack *));*/
E int FDECL(spitmm, (struct monst *,struct monst *,struct attack *));
E int FDECL(breamu, (struct monst *,struct attack *));
E int FDECL(breamm, (struct monst *,struct monst *,struct attack *));
E struct monst *FDECL(mfind_target, (struct monst *));
E boolean FDECL(linedup, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
E boolean FDECL(lined_up, (struct monst *));
E boolean FDECL(mlined_up, (struct monst *,struct monst *,BOOLEAN_P));
E struct obj *FDECL(m_carrying, (struct monst *,int));
E struct obj *FDECL(m_carrying_charged, (struct monst *,int));
E void FDECL(m_useup, (struct monst *,struct obj *));
E void FDECL(m_throw, (struct monst *,int,int,int,int,int,struct obj *,BOOLEAN_P));
E boolean FDECL(hits_bars, (struct obj **,int,int,int,int));

/* ### muse.c ### */

E boolean FDECL(find_defensive, (struct monst *));
E int FDECL(use_defensive, (struct monst *));
E int FDECL(rnd_defensive_item, (struct monst *));
E boolean FDECL(find_offensive, (struct monst *));
E struct permonst * FDECL(find_mask, (struct monst *));
#ifdef USE_TRAMPOLI
E int FDECL(mbhitm, (struct monst *,struct obj *));
#endif
E int FDECL(use_offensive, (struct monst *));
E int FDECL(rnd_offensive_item, (struct monst *));
E boolean FDECL(is_attack_wand, (int));
E int FDECL(rnd_attack_wand, (struct monst *));
E int FDECL(rnd_attack_potion, (struct monst *));
E int FDECL(rnd_utility_wand, (struct monst *));
E int FDECL(rnd_utility_potion, (struct monst *));
E boolean FDECL(find_misc, (struct monst *));
E int FDECL(use_misc, (struct monst *));
E int FDECL(rnd_misc_item, (struct monst *));
E boolean FDECL(searches_for_item, (struct monst *,struct obj *));
E boolean FDECL(mon_reflects, (struct monst *,const char *));
E boolean FDECL(ureflects, (const char *,const char *));
E boolean FDECL(munstone, (struct monst *,BOOLEAN_P));

/* ### music.c ### */

E void NDECL(awaken_soldiers);
E void FDECL(do_earthquake, (int, int, int, int,BOOLEAN_P,struct monst *));
E int FDECL(do_play_instrument, (struct obj *));
#ifdef BARD
E int FDECL(pet_can_sing, (struct monst *,BOOLEAN_P));
E int FDECL(mon_can_counter_sing, (struct monst *,BOOLEAN_P));
#endif

/* ### nhlan.c ### */
#ifdef LAN_FEATURES
E void NDECL(init_lan_features);
E char *NDECL(lan_username);
# ifdef LAN_MAIL
E boolean NDECL(lan_mail_check);
E void FDECL(lan_mail_read, (struct obj *));
E void NDECL(lan_mail_init);
E void NDECL(lan_mail_finish);
E void NDECL(lan_mail_terminate);
# endif
#endif

/* ### nttty.c ### */

#ifdef WIN32CON
E void NDECL(get_scr_size);
E int NDECL(nttty_kbhit);
E void NDECL(nttty_open);
E void NDECL(nttty_rubout);
E int NDECL(tgetch);
E int FDECL(ntposkey,(int *, int *, int *));
E void FDECL(set_output_mode, (int));
E void NDECL(synch_cursor);
#endif

/* ### o_init.c ### */

E void NDECL(init_objects);
E int NDECL(find_sawant);
E int NDECL(find_gcirclet);
E int NDECL(find_sring);
E int NDECL(find_vhelm);
E int NDECL(find_skates);
E int NDECL(find_cboots);
E int NDECL(find_mboots);
E int NDECL(find_hboots);
E int NDECL(find_bboots);
E int NDECL(find_jboots);
E int NDECL(find_cope);
E int NDECL(find_opera_cloak);
E int NDECL(find_signet_ring);
E int NDECL(find_engagement_ring);
E int NDECL(find_gold_ring);
E int NDECL(find_silver_ring);
E int NDECL(find_opal_ring);
E int NDECL(find_clay_ring);
E int NDECL(find_coral_ring);
E int NDECL(find_onyx_ring);
E int NDECL(find_moonstone_ring);
E int NDECL(find_jade_ring);
E int NDECL(find_agate_ring);
E int NDECL(find_topaz_ring);
E int NDECL(find_sapphire_ring);
E int NDECL(find_ruby_ring);
E int NDECL(find_pearl_ring);
E int NDECL(find_ivory_ring);
E int NDECL(find_emerald_ring);
E int NDECL(find_droven_ring);
E boolean FDECL(isEngrRing, (int));
E boolean FDECL(isSignetRing, (int));
E int NDECL(find_golden_potion);
E int NDECL(find_cloth_book);
E int NDECL(find_leather_book);
E int NDECL(find_bronze_book);
E int NDECL(find_silver_book);
E int NDECL(find_gold_book);
E int NDECL(find_hexagonal_wand);
E int NDECL(find_short_wand);
E int NDECL(find_runed_wand);
E int NDECL(find_long_wand);
E int NDECL(find_curved_wand);
E int NDECL(find_forked_wand);
E int NDECL(find_spiked_wand);
E int NDECL(find_jeweled_wand);
E int FDECL(matWand, (int, int));
E int NDECL(find_ogloves);
E int NDECL(find_tgloves);
E int NDECL(find_pgloves);
E int NDECL(find_fgloves);
E void NDECL(oinit);
E void FDECL(savenames, (int,int));
E void FDECL(restnames, (int));
E void FDECL(discover_object, (int,BOOLEAN_P,BOOLEAN_P));
E void FDECL(undiscover_object, (int));
E int NDECL(dodiscovered);
E void FDECL(fix_object, (struct obj *));

/* ### objects.c ### */

E void NDECL(objects_init);

/* ### objnam.c ### */

E const char *FDECL(lightsaber_colorText, (struct obj *));
E char *FDECL(lightsaber_hiltText, (struct obj *));
E int NDECL(random_saber_hilt);
E char *FDECL(obj_typename, (int));
E char *FDECL(simple_typename, (int));
E boolean FDECL(obj_is_pname, (struct obj *));
E char *FDECL(distant_name, (struct obj *,char *(*)(OBJ_P)));
E char *FDECL(fruitname, (BOOLEAN_P));
E char *FDECL(xname, (struct obj *));
E char *FDECL(mshot_xname, (struct obj *));
E boolean FDECL(the_unique_obj, (struct obj *obj));
E char *FDECL(doname, (struct obj *));
E boolean FDECL(not_fully_identified, (struct obj *));
E char *FDECL(corpse_xname, (struct obj *,BOOLEAN_P));
E char *FDECL(cxname, (struct obj *));
E char *FDECL(killer_xname, (struct obj *));
E const char *FDECL(singular, (struct obj *,char *(*)(OBJ_P)));
E char *FDECL(an, (const char *));
E char *FDECL(An, (const char *));
E char *FDECL(The, (const char *));
E char *FDECL(the, (const char *));
E char *FDECL(aobjnam, (struct obj *,const char *));
E char *FDECL(Tobjnam, (struct obj *,const char *));
E char *FDECL(otense, (struct obj *,const char *));
E char *FDECL(vtense, (const char *,const char *));
E char *FDECL(Doname2, (struct obj *));
E char *FDECL(yname, (struct obj *));
E char *FDECL(Yname2, (struct obj *));
E char *FDECL(ysimple_name, (struct obj *));
E char *FDECL(Ysimple_name2, (struct obj *));
E char *FDECL(makeplural, (const char *));
E char *FDECL(makesingular, (const char *));
E struct obj *FDECL(readobjnam, (char *,struct obj *,BOOLEAN_P));
E int FDECL(rnd_class, (int,int));
E const char *FDECL(cloak_simple_name, (struct obj *));
E const char *FDECL(mimic_obj_name, (struct monst *));

/* ### options.c ### */

E boolean FDECL(match_optname, (const char *,const char *,int,BOOLEAN_P));
E void NDECL(initoptions);
E void FDECL(parseoptions, (char *,BOOLEAN_P,BOOLEAN_P));
E int NDECL(doset);
E int NDECL(dotogglepickup);
E void NDECL(option_help);
E void FDECL(next_opt, (winid,const char *));
E int FDECL(fruitadd, (char *));
E int FDECL(choose_classes_menu, (const char *,int,BOOLEAN_P,char *,char *));
E void FDECL(add_menu_cmd_alias, (CHAR_P, CHAR_P));
E char FDECL(map_menu_cmd, (CHAR_P));
E void FDECL(assign_warnings, (uchar *));
E char *FDECL(nh_getenv, (const char *));
E void FDECL(set_duplicate_opt_detection, (int));
E void FDECL(set_wc_option_mod_status, (unsigned long, int));
E void FDECL(set_wc2_option_mod_status, (unsigned long, int));
E void FDECL(set_option_mod_status, (const char *,int));
#ifdef AUTOPICKUP_EXCEPTIONS
E int FDECL(add_autopickup_exception, (const char *));
E void NDECL(free_autopickup_exceptions);
#endif /* AUTOPICKUP_EXCEPTIONS */

/* ### pager.c ### */
E int NDECL(dowhatis);
E int NDECL(doquickwhatis);
E int NDECL(doidtrap);
E int NDECL(dowhatdoes);
E const char *NDECL(rndobjnam);
E char *FDECL(dowhatdoes_core,(CHAR_P, char *));
E int NDECL(dohelp);
E int NDECL(dohistory);

/* ### pcmain.c ### */

#if defined(MICRO) || defined(WIN32)
# ifdef CHDIR
E void FDECL(chdirx, (char *,BOOLEAN_P));
# endif /* CHDIR */
#endif /* MICRO || WIN32 */

/* ### pcsys.c ### */

#if defined(MICRO) || defined(WIN32)
E void NDECL(flushout);
E int NDECL(dosh);
# ifdef MFLOPPY
E void FDECL(eraseall, (const char *,const char *));
E void FDECL(copybones, (int));
E void NDECL(playwoRAMdisk);
E int FDECL(saveDiskPrompt, (int));
E void NDECL(gameDiskPrompt);
# endif
E void FDECL(append_slash, (char *));
E void FDECL(getreturn, (const char *));
# ifndef AMIGA
E void VDECL(msmsg, (const char *,...));
# endif
E FILE *FDECL(fopenp, (const char *,const char *));
#endif /* MICRO || WIN32 */

/* ### pctty.c ### */

#if defined(MICRO) || defined(WIN32)
E void NDECL(gettty);
E void FDECL(settty, (const char *));
E void NDECL(setftty);
E void VDECL(error, (const char *,...));
#if defined(TIMED_DELAY) && defined(_MSC_VER)
E void FDECL(msleep, (unsigned));
#endif
#endif /* MICRO || WIN32 */

/* ### pcunix.c ### */

#if defined(MICRO)
E void FDECL(regularize, (char *));
#endif /* MICRO */
#if defined(PC_LOCKING)
E void NDECL(getlock);
#endif

/* ### pickup.c ### */

E void FDECL(observe_quantum_cat, (struct obj *, boolean));
E void FDECL(open_coffin, (struct obj *, boolean));
#ifdef GOLDOBJ
E int FDECL(collect_obj_classes,
	(char *,struct obj *,BOOLEAN_P,boolean FDECL((*),(OBJ_P)), int *));
#else
E int FDECL(collect_obj_classes,
	(char *,struct obj *,BOOLEAN_P,BOOLEAN_P,boolean FDECL((*),(OBJ_P)), int *));
#endif
E void FDECL(add_valid_menu_class, (int));
E boolean FDECL(allow_all, (struct obj *));
E boolean FDECL(allow_category, (struct obj *));
E boolean FDECL(is_worn_by_type, (struct obj *));
#ifdef USE_TRAMPOLI
E int FDECL(ck_bag, (struct obj *));
E int FDECL(in_container, (struct obj *));
E int FDECL(out_container, (struct obj *));
#endif
E int FDECL(pickup, (int));
E int FDECL(pickup_object, (struct obj *, long, BOOLEAN_P));
E int FDECL(query_category, (const char *, struct obj *, int,
				menu_item **, int));
E int FDECL(query_objlist, (const char *, struct obj *, int,
				menu_item **, int, boolean (*)(OBJ_P)));
E struct obj *FDECL(pick_obj, (struct obj *));
E int NDECL(encumber_msg);
E int NDECL(doloot);
E int NDECL(dopetequip);
E int FDECL(use_container, (struct obj *,int));
E int FDECL(use_massblaster, (struct obj *));
E int FDECL(loot_mon, (struct monst *,int *,boolean *));
E const char *FDECL(safe_qbuf, (const char *,unsigned,
				const char *,const char *,const char *));
E boolean FDECL(is_autopickup_exception, (struct obj *, BOOLEAN_P));

/* ### pline.c ### */

E void VDECL(pline, (const char *,...)) PRINTF_F(1,2);
E void VDECL(Norep, (const char *,...)) PRINTF_F(1,2);
E void NDECL(free_youbuf);
E void VDECL(You, (const char *,...)) PRINTF_F(1,2);
E void VDECL(Your, (const char *,...)) PRINTF_F(1,2);
E void VDECL(You_feel, (const char *,...)) PRINTF_F(1,2);
E void VDECL(You_cant, (const char *,...)) PRINTF_F(1,2);
E void VDECL(You_hear, (const char *,...)) PRINTF_F(1,2);
E void VDECL(pline_The, (const char *,...)) PRINTF_F(1,2);
E void VDECL(There, (const char *,...)) PRINTF_F(1,2);
E void VDECL(verbalize, (const char *,...)) PRINTF_F(1,2);
E void VDECL(raw_printf, (const char *,...)) PRINTF_F(1,2);
E void VDECL(impossible, (const char *,...)) PRINTF_F(1,2);
E const char *FDECL(align_str, (ALIGNTYP_P));
E void FDECL(mstatusline, (struct monst *));
E void NDECL(ustatusline);
E void NDECL(self_invis_message);
E const char *FDECL(piratesay, (const char *));

/* ### polyself.c ### */

E void NDECL(init_uasmon);
E void NDECL(set_uasmon);
E void NDECL(change_sex);
E void FDECL(polyself, (BOOLEAN_P));
E int FDECL(polymon, (int));
E void NDECL(rehumanize);
E int FDECL(dobreathe, (struct permonst *));
E int NDECL(doelementalbreath);
E int NDECL(dospit);
E int NDECL(doremove);
E int NDECL(dospinweb);
E int NDECL(dosummon);
E int NDECL(dogaze);
E int NDECL(dohide);
E int NDECL(domindblast);
E int NDECL(dodarken);
E int NDECL(doclockspeed);
E void FDECL(skinback, (BOOLEAN_P));
E const char *FDECL(mbodypart, (struct monst *,int));
E const char *FDECL(body_part, (int));
E int NDECL(poly_gender);
E void FDECL(ugolemeffects, (int,int));

/* ### potion.c ### */

E void FDECL(set_itimeout, (long *,long));
E void FDECL(incr_itimeout, (long *,int));
E long FDECL(itimeout_incr, (long,int));
E void FDECL(make_confused, (long,BOOLEAN_P));
E void FDECL(make_stunned, (long,BOOLEAN_P));
E void FDECL(make_blinded, (long,BOOLEAN_P));
E void FDECL(make_sick, (long, const char *, BOOLEAN_P,int));
E void FDECL(make_vomiting, (long,BOOLEAN_P));
E boolean FDECL(make_hallucinated, (long,BOOLEAN_P,long));
E int NDECL(dodrink);
E int FDECL(dopotion, (struct obj *));
E int FDECL(peffects, (struct obj *));
E void FDECL(healup, (int,int,BOOLEAN_P,BOOLEAN_P));
E void FDECL(strange_feeling, (struct obj *,const char *));
E void FDECL(potionhit, (struct monst *,struct obj *,BOOLEAN_P));
E void FDECL(potionbreathe, (struct obj *));
E boolean FDECL(get_wet, (struct obj *, BOOLEAN_P));
E int NDECL(dodip);
E void FDECL(djinni_from_bottle, (struct obj *));
E struct monst *FDECL(split_mon, (struct monst *,struct monst *));
E const char *NDECL(bottlename);

/* ### pray.c ### */

E const char * NDECL(getDrowMaleLgodKnown);
E const char * NDECL(getAnachrononautLgod);
E const char * NDECL(getAnachrononautLgodEnd);
#ifdef USE_TRAMPOLI
E int NDECL(prayer_done);
#endif
E void FDECL(gods_angry,(ALIGNTYP_P));
E void FDECL(gods_upset,(ALIGNTYP_P));
E void FDECL(angrygods,(ALIGNTYP_P));
E int NDECL(dosacrifice);
E boolean FDECL(can_pray, (BOOLEAN_P));
E int NDECL(dopray);
E const char *NDECL(u_gname);
E int NDECL(doturn);
E const char *NDECL(a_gname);
E const char *FDECL(a_gname_at, (XCHAR_P x,XCHAR_P y));
E const char *FDECL(align_gname, (ALIGNTYP_P));
E const char *FDECL(align_gname_full, (ALIGNTYP_P));
E const char *FDECL(halu_gname, (ALIGNTYP_P));
E const char *FDECL(align_gtitle, (ALIGNTYP_P));
E void FDECL(altar_wrath, (int,int));
E int FDECL(candle_on_altar, (struct obj *));

/* ### priest.c ### */

E coord * FDECL(shrine_pos, (int));
E aligntyp FDECL(temple_alignment, (int));
E int FDECL(move_special, (struct monst *,BOOLEAN_P,SCHAR_P,BOOLEAN_P,BOOLEAN_P,
			   XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
E char FDECL(temple_occupied, (char *));
E int FDECL(pri_move, (struct monst *));
E void FDECL(priestini, (d_level *,struct mkroom *,int,int,BOOLEAN_P));
E char *FDECL(priestname, (struct monst *,char *));
E boolean FDECL(p_coaligned, (struct monst *));
E struct monst *FDECL(findpriest, (CHAR_P));
E void FDECL(intemple, (int));
E void FDECL(priest_talk, (struct monst *));
E struct monst *FDECL(mk_roamer, (struct permonst *,ALIGNTYP_P,
				  XCHAR_P,XCHAR_P,BOOLEAN_P));
E void FDECL(reset_hostility, (struct monst *));
E boolean FDECL(in_your_sanctuary, (struct monst *,XCHAR_P,XCHAR_P));
E void FDECL(ghod_hitsu, (struct monst *));
E void NDECL(angry_priest);
E void NDECL(clearpriests);
E void FDECL(restpriest, (struct monst *,BOOLEAN_P));

/* ### quest.c ### */

E void NDECL(onquest);
E void NDECL(nemdead);
E void NDECL(artitouch);
E boolean NDECL(ok_to_quest);
E void FDECL(leader_speaks, (struct monst *));
E void NDECL(nemesis_speaks);
E void FDECL(quest_chat, (struct monst *));
E void FDECL(quest_talk, (struct monst *));
E void FDECL(quest_stat_check, (struct monst *));
E void FDECL(finish_quest, (struct obj *));
E void NDECL(turn_stag);

/* ### questpgr.c ### */

E void NDECL(load_qtlist);
E void NDECL(unload_qtlist);
E short FDECL(quest_info, (int));
E const char *NDECL(ldrname);
E boolean FDECL(is_quest_artifact, (struct obj*));
E boolean FDECL(is_primary_quest_artifact, (struct obj*));
E void FDECL(com_pager, (int));
E void FDECL(qt_pager, (int));
E struct permonst *NDECL(qt_montype);
E struct permonst *NDECL(chaos_montype);
E struct permonst *NDECL(neutral_montype);
E struct permonst *NDECL(law_montype);

/* ### random.c ### */

#if defined(RANDOM) && !defined(__GO32__) /* djgpp has its own random */
E void FDECL(srandom, (unsigned));
E char *FDECL(initstate, (unsigned,char *,int));
E char *FDECL(setstate, (char *));
E long NDECL(random);
#endif /* RANDOM */

/* ### read.c ### */

E int NDECL(doread);
E boolean FDECL(is_chargeable, (struct obj *));
E void FDECL(recharge, (struct obj *,int));
E void FDECL(forget, (int));
E void FDECL(forget_objects, (int));
E void FDECL(forget_levels, (int));
E void NDECL(forget_traps);
E void FDECL(forget_map, (int));
E int FDECL(seffects, (struct obj *));
E void FDECL(litroom, (BOOLEAN_P,struct obj *));
E void FDECL(set_lit, (int,int,genericptr_t));
E void FDECL(do_genocide, (int));
E void FDECL(punish, (struct obj *));
E void NDECL(unpunish);
E boolean FDECL(cant_create, (int *, BOOLEAN_P));
#ifdef WIZARD
E boolean NDECL(create_particular);
#endif

/* ### rect.c ### */

E void NDECL(init_rect);
E NhRect *FDECL(get_rect, (NhRect *));
E NhRect *NDECL(rnd_rect);
E void FDECL(remove_rect, (NhRect *));
E void FDECL(add_rect, (NhRect *));
E void FDECL(split_rects, (NhRect *,NhRect *));

/* ## region.c ### */
E void NDECL(clear_regions);
E void NDECL(run_regions);
E boolean FDECL(In_fog_cloud, (struct monst *));
E boolean FDECL(check_solid_fog_region, (XCHAR_P,XCHAR_P));
E boolean FDECL(check_stinking_cloud_region, (XCHAR_P,XCHAR_P));
E boolean FDECL(in_out_region, (XCHAR_P,XCHAR_P));
E boolean FDECL(m_in_out_region, (struct monst *,XCHAR_P,XCHAR_P));
E void NDECL(update_player_regions);
E void FDECL(update_monster_region, (struct monst *));
E NhRegion *FDECL(visible_region_at, (XCHAR_P,XCHAR_P));
E void FDECL(show_region, (NhRegion*, XCHAR_P, XCHAR_P));
E void FDECL(save_regions, (int,int));
E void FDECL(rest_regions, (int,BOOLEAN_P));
E NhRegion* FDECL(create_gas_cloud, (XCHAR_P, XCHAR_P, int, int));

/* ### restore.c ### */

E void FDECL(inven_inuse, (BOOLEAN_P));
E int FDECL(dorecover, (int));
E void FDECL(trickery, (char *));
E void FDECL(getlev, (int,int,XCHAR_P,BOOLEAN_P));
E void NDECL(minit);
E boolean FDECL(lookup_id_mapping, (unsigned, unsigned *));
#ifdef ZEROCOMP
E int FDECL(mread, (int,genericptr_t,unsigned int));
#else
E void FDECL(mread, (int,genericptr_t,unsigned int));
#endif

/* ### rip.c ### */

E void FDECL(genl_outrip, (winid,int));

/* ### rnd.c ### */

E int FDECL(rn2, (int));
E int FDECL(rnl, (int));
E int FDECL(rnd, (int));
E int FDECL(d, (int,int));
E int FDECL(exploding_d, (int,int,int));
E int FDECL(lucky_exploding_d, (int,int,int));
E int FDECL(unlucky_exploding_d, (int,int,int));
E int FDECL(rne, (int));
E int FDECL(rnz, (int));

/* ### role.c ### */

E boolean FDECL(validrole, (int));
E boolean FDECL(validrace, (int, int));
E boolean FDECL(validgend, (int, int, int));
E boolean FDECL(validalign, (int, int, int));
E int FDECL(randrole, (int));
E int FDECL(randrace, (int));
E int FDECL(randgend, (int, int));
E int FDECL(randalign, (int, int));
E const int * FDECL(god_minions, (const char *));
E int FDECL(god_faction, (const char *));
E struct monst * FDECL(god_priest, (const char *, int, int, boolean));
E int FDECL(str2role, (char *));
E const struct Role *FDECL(pm2role, (int));
E int FDECL(str2race, (char *));
E int FDECL(str2gend, (char *));
E int FDECL(str2align, (char *));
E boolean FDECL(ok_role, (int, int, int, int));
E int FDECL(pick_role, (int, int, int, int));
E boolean FDECL(ok_race, (int, int, int, int));
E int FDECL(pick_race, (int, int, int, int));
E boolean FDECL(ok_gend, (int, int, int, int));
E int FDECL(pick_gend, (int, int, int, int));
E boolean FDECL(ok_align, (int, int, int, int));
E int FDECL(pick_align, (int, int, int, int));
E void NDECL(role_init);
E void NDECL(rigid_role_checks);
E void NDECL(plnamesuffix);
E const char *FDECL(Hello, (struct monst *));
E const char *NDECL(Goodbye);
E char *FDECL(build_plselection_prompt, (char *, int, int, int, int, int));
E char *FDECL(root_plselection_prompt, (char *, int, int, int, int, int));
E const char *NDECL(getDnDElfLgod);
E const char *NDECL(getDnDElfNgod);
E const char *NDECL(getDnDElfCgod);
E const char *NDECL(getDnDHumLgod);
E const char *NDECL(getDnDHumNgod);
E const char *NDECL(getDnDHumCgod);

/* ### rumors.c ### */

E char *FDECL(getrumor, (int,char *, BOOLEAN_P));
E void FDECL(outrumor, (int,int));
E void NDECL(outgmaster);
E void FDECL(outoracle, (BOOLEAN_P, BOOLEAN_P));
E void FDECL(save_oracles, (int,int));
E void FDECL(restore_oracles, (int));
E int FDECL(doconsult, (struct monst *));

/* ### save.c ### */

E int NDECL(dosave);
#if defined(UNIX) || defined(VMS) || defined(__EMX__) || defined(WIN32)
E void FDECL(hangup, (int));
#endif
E int NDECL(dosave0);
#ifdef INSURANCE
E void NDECL(savestateinlock);
#endif
#ifdef MFLOPPY
E boolean FDECL(savelev, (int,XCHAR_P,int));
E boolean FDECL(swapin_file, (int));
E void NDECL(co_false);
#else
E void FDECL(savelev, (int,XCHAR_P,int));
#endif
E void FDECL(bufon, (int));
E void FDECL(bufoff, (int));
E void FDECL(bflush, (int));
E void FDECL(bwrite, (int,genericptr_t,unsigned int));
E void FDECL(bclose, (int));
E void FDECL(savefruitchn, (int,int));
E void NDECL(free_dungeons);
E void NDECL(freedynamicdata);
E boolean NDECL(dahlverNarVis);

/* ### shk.c ### */

#ifdef GOLDOBJ
E long FDECL(money2mon, (struct monst *, long));
E long FDECL(money2none, (long));
E void FDECL(money2u, (struct monst *, long));
#endif
E char *FDECL(shkname, (struct monst *));
E void FDECL(shkgone, (struct monst *));
E void FDECL(set_residency, (struct monst *,BOOLEAN_P));
E void FDECL(replshk, (struct monst *,struct monst *));
E void FDECL(restshk, (struct monst *,BOOLEAN_P));
E char FDECL(inside_shop, (XCHAR_P,XCHAR_P));
E void FDECL(u_left_shop, (char *,BOOLEAN_P));
E void FDECL(remote_burglary, (XCHAR_P,XCHAR_P));
E void FDECL(u_entered_shop, (char *));
E boolean FDECL(same_price, (struct obj *,struct obj *));
E void NDECL(shopper_financial_report);
E int FDECL(inhishop, (struct monst *));
E struct monst *FDECL(shop_keeper, (CHAR_P));
E boolean FDECL(tended_shop, (struct mkroom *));
E void FDECL(delete_contents, (struct obj *));
E void FDECL(obfree, (struct obj *,struct obj *));
E void FDECL(home_shk, (struct monst *,BOOLEAN_P));
E void FDECL(make_happy_shk, (struct monst *,BOOLEAN_P));
E void FDECL(hot_pursuit, (struct monst *));
E void FDECL(make_angry_shk, (struct monst *,XCHAR_P,XCHAR_P));
E int NDECL(dopay);
E boolean FDECL(paybill, (int));
E void NDECL(finish_paybill);
E struct obj *FDECL(find_oid, (unsigned));
E long FDECL(contained_cost, (struct obj *,struct monst *,long,BOOLEAN_P, BOOLEAN_P));
E long FDECL(contained_gold, (struct obj *));
E void FDECL(picked_container, (struct obj *));
E long FDECL(unpaid_cost, (struct obj *));
E void FDECL(addtobill, (struct obj *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P));
E void FDECL(splitbill, (struct obj *,struct obj *));
E void FDECL(subfrombill, (struct obj *,struct monst *));
E long FDECL(stolen_value, (struct obj *,XCHAR_P,XCHAR_P,BOOLEAN_P,BOOLEAN_P));
E void FDECL(sellobj_state, (int));
E void FDECL(sellobj, (struct obj *,XCHAR_P,XCHAR_P));
E int FDECL(doinvbill, (int));
E struct monst *FDECL(shkcatch, (struct obj *,XCHAR_P,XCHAR_P));
E void FDECL(add_damage, (XCHAR_P,XCHAR_P,long));
E int FDECL(repair_damage, (struct monst *,struct damage *,BOOLEAN_P));
E int FDECL(shk_move, (struct monst *));
E void FDECL(after_shk_move, (struct monst *));
E boolean FDECL(is_fshk, (struct monst *));
E void FDECL(shopdig, (int));
E void FDECL(pay_for_damage, (const char *,BOOLEAN_P));
E boolean FDECL(costly_spot, (XCHAR_P,XCHAR_P));
E struct obj *FDECL(shop_object, (XCHAR_P,XCHAR_P));
E void FDECL(price_quote, (struct obj *));
E void FDECL(shk_chat, (struct monst *));
E void FDECL(check_unpaid_usage, (struct obj *,BOOLEAN_P));
E void FDECL(check_unpaid, (struct obj *));
E void FDECL(costly_gold, (XCHAR_P,XCHAR_P,long));
E boolean FDECL(block_door, (XCHAR_P,XCHAR_P));
E boolean FDECL(block_entry, (XCHAR_P,XCHAR_P));
E char *FDECL(shk_your, (char *,struct obj *));
E char *FDECL(Shk_Your, (char *,struct obj *));
E void FDECL(makeketer, (coord *));
E int FDECL(countFarSigns, (struct monst *));
E int FDECL(countCloseSigns, (struct monst *));

/* ### shknam.c ### */

E void FDECL(stock_room, (int,struct mkroom *));
E boolean FDECL(saleable, (struct monst *,struct obj *));
E int FDECL(get_shop_item, (int));

/* ### sit.c ### */

E void NDECL(take_gold);
E int NDECL(dosit);
E void NDECL(rndcurse);
E void FDECL(mrndcurse, (struct monst *));
E void NDECL(attrcurse);

/* ### sounds.c ### */

E void NDECL(dosounds);
E const char *FDECL(growl_sound, (struct monst *));
E void FDECL(growl, (struct monst *));
E void FDECL(yelp, (struct monst *));
E void FDECL(whimper, (struct monst *));
E void FDECL(beg, (struct monst *));
E int NDECL(dotalk);
E void FDECL(councilspirit, (int));
E void FDECL(gnosisspirit, (int));
E int FDECL(P_MAX_SKILL, (int));
E int FDECL(P_SKILL, (int));
E int FDECL(P_RESTRICTED, (int));
E boolean FDECL(spiritSkill, (int));
E boolean FDECL(roleSkill, (int));
E int FDECL(domonnoise,(struct monst *));
#ifdef USER_SOUNDS
E int FDECL(add_sound_mapping, (const char *));
E void FDECL(play_sound_for_message, (const char *));
#endif

/* ### sys/msdos/sound.c ### */

#ifdef MSDOS
E int FDECL(assign_soundcard, (char *));
#endif

/* ### sp_lev.c ### */

E boolean FDECL(check_room, (xchar *,xchar *,xchar *,xchar *,BOOLEAN_P));
E boolean FDECL(create_room, (XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P,
			      XCHAR_P,XCHAR_P,XCHAR_P,XCHAR_P));
E void FDECL(create_secret_door, (struct mkroom *,XCHAR_P));
E boolean FDECL(dig_corridor, (coord *,coord *,BOOLEAN_P,SCHAR_P,SCHAR_P));
E void FDECL(fill_room, (struct mkroom *,BOOLEAN_P));
E boolean FDECL(load_special, (const char *));

/* ### spell.c ### */

#ifdef USE_TRAMPOLI
E int NDECL(learn);
#endif
E int FDECL(study_book, (struct obj *));
E boolean FDECL(spell_maintained, (int));
E void FDECL(spell_maintain, (int));
E void FDECL(spell_unmaintain, (int));
E void NDECL(run_maintained_spells);
E void FDECL(book_disappears, (struct obj *));
E void FDECL(book_substitution, (struct obj *,struct obj *));
E void NDECL(age_spells);
E int NDECL(docast);
E boolean FDECL(tt_findadjacent, (coord *, struct monst *));
E int NDECL(spiritDsize);
E int NDECL(dospirit);
E int FDECL(spell_skilltype, (int));
E int FDECL(spiriteffects, (int,BOOLEAN_P));
E int FDECL(spelleffects, (int,BOOLEAN_P,int));
E void FDECL(losespells, (int));
E int NDECL(dovspell);
E void FDECL(initialspell, (struct obj *));
E void FDECL(initialward, (struct obj *));
E void FDECL(set_spirit_powers, (long));

/* ### steal.c ### */

#ifdef USE_TRAMPOLI
E int NDECL(stealarm);
#endif
#ifdef GOLDOBJ
E long FDECL(somegold, (long));
#else
E long NDECL(somegold);
#endif
E void FDECL(stealgold, (struct monst *));
E void FDECL(remove_worn_item, (struct obj *,BOOLEAN_P));
E int FDECL(steal, (struct monst *, char *,boolean,boolean));
E int FDECL(mpickobj, (struct monst *,struct obj *));
E void FDECL(stealamulet, (struct monst *));
E void FDECL(stealquestart, (struct monst *));
E void FDECL(mdrop_special_objs, (struct monst *));
E void FDECL(relobj, (struct monst *,int,BOOLEAN_P));
E void FDECL(mdrop_obj, (struct monst *,struct obj *,BOOLEAN_P));
#ifdef GOLDOBJ
E struct obj *FDECL(findgold, (struct obj *));
#endif

/* ### steed.c ### */

#ifdef STEED
E void NDECL(rider_cant_reach);
E boolean FDECL(can_saddle, (struct monst *));
E int FDECL(use_saddle, (struct obj *));
E boolean FDECL(can_ride, (struct monst *));
E int NDECL(doride);
E boolean FDECL(mount_steed, (struct monst *, BOOLEAN_P));
E void NDECL(exercise_steed);
E void NDECL(kick_steed);
E void FDECL(dismount_steed, (int));
E void FDECL(place_monster, (struct monst *,int,int));
#endif

/* ### teleport.c ### */

E boolean FDECL(teleok, (int,int,BOOLEAN_P));
E boolean FDECL(goodpos, (int,int,struct monst *,unsigned));
E boolean FDECL(eonline, (coord *,XCHAR_P,XCHAR_P,struct permonst *));
E boolean FDECL(eofflin, (coord *,XCHAR_P,XCHAR_P,struct permonst *));
E boolean FDECL(enexto, (coord *,XCHAR_P,XCHAR_P,struct permonst *));
E boolean FDECL(enexto_core, (coord *,XCHAR_P,XCHAR_P,struct permonst *,unsigned));
E void FDECL(xpathto, (int,XCHAR_P,XCHAR_P,int (*)(genericptr_t,int,int),void *));
E void FDECL(teleds, (int,int,BOOLEAN_P));
E boolean FDECL(safe_teleds, (BOOLEAN_P));
E boolean FDECL(teleport_pet, (struct monst *,BOOLEAN_P));
E void NDECL(tele);
E int NDECL(dotele);
E void NDECL(level_tele);
E void FDECL(domagicportal, (struct trap *));
E void FDECL(tele_trap, (struct trap *));
E void FDECL(level_tele_trap, (struct trap *));
E void FDECL(rloc_to, (struct monst *,int,int));
E boolean FDECL(rloc, (struct monst *, BOOLEAN_P));
E boolean FDECL(tele_restrict, (struct monst *));
E void FDECL(mtele_trap, (struct monst *, struct trap *,int));
E int FDECL(mlevel_tele_trap, (struct monst *, struct trap *,BOOLEAN_P,int));
E void FDECL(rloco, (struct obj *));
E int NDECL(random_teleport_level);
E boolean FDECL(u_teleport_mon, (struct monst *,BOOLEAN_P));

/* ### tile.c ### */
#ifdef USE_TILES
E void FDECL(substitute_tiles, (d_level *));
#endif

/* ### timeout.c ### */

E void NDECL(burn_away_slime);
E void FDECL(unbind, (long, boolean));
E void NDECL(nh_timeout);
E void FDECL(fall_asleep, (int, BOOLEAN_P));
E void FDECL(attach_bomb_blow_timeout, (struct obj *,int,BOOLEAN_P));
E void FDECL(attach_egg_hatch_timeout, (struct obj *));
E void FDECL(attach_fig_transform_timeout, (struct obj *));
E void FDECL(kill_egg, (struct obj *));
E void FDECL(hatch_egg, (genericptr_t, long));
E void FDECL(learn_egg_type, (int));
E void FDECL(burn_object, (genericptr_t, long));
E void FDECL(begin_burn, (struct obj *, BOOLEAN_P));
E void FDECL(end_burn, (struct obj *, BOOLEAN_P));
E void FDECL(lightsaber_deactivate, (struct obj *, BOOLEAN_P));
E void NDECL(do_storms);
E boolean FDECL(start_timer, (long, SHORT_P, SHORT_P, genericptr_t));
// E long FDECL(report_timer, (struct level *, short, const void *));
E long FDECL(stop_timer, (SHORT_P, genericptr_t));
E void NDECL(run_timers);
E void FDECL(obj_move_timers, (struct obj *, struct obj *));
E void FDECL(obj_split_timers, (struct obj *, struct obj *));
E void FDECL(obj_stop_timers, (struct obj *));
E boolean FDECL(obj_is_local, (struct obj *));
E void FDECL(save_timers, (int,int,int));
E void FDECL(restore_timers, (int,int,BOOLEAN_P,long));
E void FDECL(relink_timers, (BOOLEAN_P));
#ifdef WIZARD
E int NDECL(wiz_timeout_queue);
E void NDECL(timer_sanity_check);
#endif

/* ### topten.c ### */

E void FDECL(topten, (int));
E void FDECL(prscore, (int,char **));
E struct obj *FDECL(tt_oname, (struct obj *));

/* ### track.c ### */

E void NDECL(initrack);
E void NDECL(settrack);
E coord *FDECL(gettrack, (int,int));

/* ### trap.c ### */

E boolean FDECL(burnarmor,(struct monst *));
E boolean FDECL(rust_dmg, (struct obj *,const char *,int,BOOLEAN_P,struct monst *));
E void FDECL(grease_protect, (struct obj *,const char *,struct monst *));
E struct trap *FDECL(maketrap, (int,int,int));
E void FDECL(fall_through, (BOOLEAN_P));
E struct monst *FDECL(animate_statue, (struct obj *,XCHAR_P,XCHAR_P,int,int *));
E struct monst *FDECL(activate_statue_trap,
			(struct trap *,XCHAR_P,XCHAR_P,BOOLEAN_P));
E void FDECL(dotrap, (struct trap *, unsigned));
E void FDECL(seetrap, (struct trap *));
E int FDECL(mintrap, (struct monst *));
E void FDECL(instapetrify, (const char *));
E void FDECL(minstapetrify, (struct monst *,BOOLEAN_P));
E void FDECL(selftouch, (const char *));
E void FDECL(mselftouch, (struct monst *,const char *,BOOLEAN_P));
E void NDECL(float_up);
E void FDECL(fill_pit, (int,int));
E int FDECL(float_down, (long, long));
E int FDECL(fire_damage, (struct obj *,BOOLEAN_P,BOOLEAN_P,XCHAR_P,XCHAR_P));
E boolean FDECL(water_damage, (struct obj *,BOOLEAN_P,BOOLEAN_P,BOOLEAN_P, struct monst *));
E boolean NDECL(drown);
E int NDECL(dodeepswim);
E void FDECL(drain_en, (int));
E int NDECL(dountrap);
E int FDECL(untrap, (BOOLEAN_P));
E boolean FDECL(chest_trap, (struct obj *,int,BOOLEAN_P));
E void FDECL(deltrap, (struct trap *));
E boolean FDECL(delfloortrap, (struct trap *));
E struct trap *FDECL(t_at, (int,int));
E void FDECL(b_trapped, (const char *,int));
E boolean NDECL(unconscious);
E boolean NDECL(lava_effects);
E void FDECL(blow_up_landmine, (struct trap *));
E int FDECL(launch_obj,(SHORT_P,int,int,int,int,int));
E void FDECL(dowebgush, (int,int,int));
E void FDECL(webgush, (int,int,genericptr_t));

/* ### u_init.c ### */

E void NDECL(u_init);

/* ### uhitm.c ### */

E void FDECL(hurtmarmor,(struct monst *,int));
E boolean FDECL(attack_checks, (struct monst *,struct obj *));
E void FDECL(check_caitiff, (struct monst *));
E void FDECL(find_to_hit_rolls, (struct monst *,int *,int *,int *));
E int FDECL(find_roll_to_hit, (struct monst *, boolean));
E boolean FDECL(attack, (struct monst *));
E boolean FDECL(hmon, (struct monst *,struct obj *,int));
E int FDECL(damageum, (struct monst *,struct attack *));
E void FDECL(missum, (struct monst *,struct attack *));
E int FDECL(passive, (struct monst *,BOOLEAN_P,int,UCHAR_P,UCHAR_P));
E void FDECL(passive_obj, (struct monst *,struct obj *,struct attack *));
E void FDECL(stumble_onto_mimic, (struct monst *));
E int FDECL(flash_hits_mon, (struct monst *,struct obj *));
E boolean FDECL(hmonas, (struct monst *,struct permonst *,int,int,int));
E boolean FDECL(hmonwith, (struct monst *, int, int, int, struct attack *, int));

/* ### unixmain.c ### */

#ifdef UNIX
# ifdef PORT_HELP
E void NDECL(port_help);
# endif
#endif /* UNIX */


/* ### unixtty.c ### */

#if defined(UNIX) || defined(__BEOS__)
E void NDECL(gettty);
E void FDECL(settty, (const char *));
E void NDECL(setftty);
E void NDECL(intron);
E void NDECL(introff);
E void VDECL(error, (const char *,...)) PRINTF_F(1,2);
#endif /* UNIX || __BEOS__ */

/* ### unixunix.c ### */

#ifdef UNIX
E void NDECL(getlock);
E void FDECL(regularize, (char *));
# if defined(TIMED_DELAY) && !defined(msleep) && defined(SYSV)
E void FDECL(msleep, (unsigned));
# endif
# ifdef SHELL
E int NDECL(dosh);
# endif /* SHELL */
# if defined(SHELL) || defined(DEF_PAGER) || defined(DEF_MAILREADER)
E int FDECL(child, (int));
# endif
#endif /* UNIX */

/* ### unixres.c ### */

#ifdef UNIX
# ifdef GNOME_GRAPHICS 
E int FDECL(hide_privileges, (BOOLEAN_P));
# endif
#endif /* UNIX */

/* ### vault.c ### */

E boolean FDECL(grddead, (struct monst *));
E char FDECL(vault_occupied, (char *));
E void NDECL(invault);
E int FDECL(gd_move, (struct monst *));
E void NDECL(paygd);
E long NDECL(hidden_gold);
E boolean NDECL(gd_sound);

/* ### version.c ### */

E char *FDECL(version_string, (char *));
E char *FDECL(getversionstring, (char *));
E int NDECL(doversion);
E int NDECL(doextversion);
#ifdef MICRO
E boolean FDECL(comp_times, (long));
#endif
E boolean FDECL(check_version, (struct version_info *,
				const char *,BOOLEAN_P));
E unsigned long FDECL(get_feature_notice_ver, (char *));
E unsigned long NDECL(get_current_feature_ver);
#ifdef RUNTIME_PORT_ID
E void FDECL(append_port_id, (char *));
#endif

/* ### video.c ### */

#ifdef MSDOS
E int FDECL(assign_video, (char *));
# ifdef NO_TERMS
E void NDECL(gr_init);
E void NDECL(gr_finish);
# endif
E void FDECL(tileview,(BOOLEAN_P));
#endif
#ifdef VIDEOSHADES
E int FDECL(assign_videoshades, (char *));
E int FDECL(assign_videocolors, (char *));
#endif

/* ### vis_tab.c ### */

#ifdef VISION_TABLES
E void NDECL(vis_tab_init);
#endif

/* ### vision.c ### */

E void NDECL(vision_init);
E int FDECL(does_block, (int,int,struct rm*));
E void NDECL(vision_reset);
E void FDECL(vision_recalc, (int));
E void FDECL(block_point, (int,int));
E void FDECL(unblock_point, (int,int));
E boolean FDECL(clear_path, (int,int,int,int));
E void FDECL(do_clear_area, (int,int,int,
			     void (*)(int,int,genericptr_t),genericptr_t));

#ifdef VMS

/* ### vmsfiles.c ### */

E int FDECL(vms_link, (const char *,const char *));
E int FDECL(vms_unlink, (const char *));
E int FDECL(vms_creat, (const char *,unsigned int));
E int FDECL(vms_open, (const char *,int,unsigned int));
E boolean FDECL(same_dir, (const char *,const char *));
E int FDECL(c__translate, (int));
E char *FDECL(vms_basename, (const char *));

/* ### vmsmail.c ### */

E unsigned long NDECL(init_broadcast_trapping);
E unsigned long NDECL(enable_broadcast_trapping);
E unsigned long NDECL(disable_broadcast_trapping);
# if 0
E struct mail_info *NDECL(parse_next_broadcast);
# endif /*0*/

/* ### vmsmain.c ### */

E int FDECL(main, (int, char **));
# ifdef CHDIR
E void FDECL(chdirx, (const char *,BOOLEAN_P));
# endif /* CHDIR */

/* ### vmsmisc.c ### */

E void NDECL(vms_abort);
E void FDECL(vms_exit, (int));

/* ### vmstty.c ### */

E int NDECL(vms_getchar);
E void NDECL(gettty);
E void FDECL(settty, (const char *));
E void FDECL(shuttty, (const char *));
E void NDECL(setftty);
E void NDECL(intron);
E void NDECL(introff);
E void VDECL(error, (const char *,...)) PRINTF_F(1,2);
#ifdef TIMED_DELAY
E void FDECL(msleep, (unsigned));
#endif

/* ### vmsunix.c ### */

E void NDECL(getlock);
E void FDECL(regularize, (char *));
E int NDECL(vms_getuid);
E boolean FDECL(file_is_stmlf, (int));
E int FDECL(vms_define, (const char *,const char *,int));
E int FDECL(vms_putenv, (const char *));
E char *NDECL(verify_termcap);
# if defined(CHDIR) || defined(SHELL) || defined(SECURE)
E void NDECL(privoff);
E void NDECL(privon);
# endif
# ifdef SHELL
E int NDECL(dosh);
# endif
# if defined(SHELL) || defined(MAIL)
E int FDECL(vms_doshell, (const char *,BOOLEAN_P));
# endif
# ifdef SUSPEND
E int NDECL(dosuspend);
# endif

#endif /* VMS */

/* ### weapon.c ### */

E int FDECL(hitval, (struct obj *,struct monst *));
E int FDECL(dmgval, (struct obj *,struct monst *, int));
E struct obj *FDECL(select_rwep, (struct monst *));
E struct obj *FDECL(select_hwep, (struct monst *));
E void FDECL(possibly_unwield, (struct monst *,BOOLEAN_P));
E int FDECL(mon_wield_item, (struct monst *));
E void FDECL(init_mon_wield_item, (struct monst *));
E int NDECL(abon);
E int FDECL(dbon, (struct obj *));
E int FDECL(dtypbon, (int));
E int NDECL(enhance_weapon_skill);
E void FDECL(expert_weapon_skill, (int));
E void FDECL(gm_weapon_skill, (int));
E void FDECL(unrestrict_weapon_skill, (int));
E void FDECL(use_skill, (int,int));
E void FDECL(add_weapon_skill, (int));
E void FDECL(lose_weapon_skill, (int));
E int FDECL(weapon_type, (struct obj *));
E int NDECL(uwep_skill_type);
E int FDECL(weapon_hit_bonus, (struct obj *));
E int FDECL(weapon_dam_bonus, (struct obj *));
E void FDECL(skill_init, (const struct def_skill *));
E void FDECL(skill_add, (const struct def_skill *));

/* ### were.c ### */

E void FDECL(were_change, (struct monst *));
E void FDECL(new_were, (struct monst *));
E int FDECL(were_summon, (struct permonst *,BOOLEAN_P,int *,char *));
E void NDECL(you_were);
E void FDECL(you_unwere, (BOOLEAN_P));

/* ### wield.c ### */

E void FDECL(setuwep, (struct obj *));
E void FDECL(setuqwep, (struct obj *));
E void FDECL(setuswapwep, (struct obj *));
E int NDECL(dowield);
E int NDECL(doswapweapon);
E int NDECL(dowieldquiver);
E boolean FDECL(wield_tool, (struct obj *,const char *));
E int NDECL(can_twoweapon);
E void NDECL(drop_uswapwep);
E int NDECL(dotwoweapon);
E void NDECL(uwepgone);
E void NDECL(uswapwepgone);
E void NDECL(uqwepgone);
E void NDECL(untwoweapon);
E void FDECL(erode_obj, (struct obj *,BOOLEAN_P,BOOLEAN_P));
E int FDECL(chwepon, (struct obj *,int));
E int FDECL(welded, (struct obj *));
E void FDECL(weldmsg, (struct obj *));
E void FDECL(setmnotwielded, (struct monst *,struct obj *));

/* ### windows.c ### */

E void FDECL(choose_windows, (const char *));
E char FDECL(genl_message_menu, (CHAR_P,int,const char *));
E void FDECL(genl_preference_update, (const char *));

/* ### wizard.c ### */

E void NDECL(amulet);
E int FDECL(mon_has_amulet, (struct monst *));
E int FDECL(mon_has_special, (struct monst *));
E int FDECL(tactics, (struct monst *));
E void NDECL(aggravate);
E void NDECL(clonewiz);
E int NDECL(pick_nasty);
E int FDECL(nasty, (struct monst*));
E void NDECL(resurrect);
E void NDECL(illur_resurrect);
E void NDECL(intervene);
E void NDECL(illur_intervene);
E void NDECL(wizdead);
E void FDECL(cuss, (struct monst *));

/* ### worm.c ### */

E int NDECL(get_wormno);
E void FDECL(initworm, (struct monst *,int));
E void FDECL(worm_move, (struct monst *));
E void FDECL(worm_nomove, (struct monst *));
E void FDECL(wormgone, (struct monst *));
E void FDECL(wormhitu, (struct monst *));
E void FDECL(cutworm, (struct monst *,XCHAR_P,XCHAR_P,struct obj *));
E void FDECL(see_wsegs, (struct monst *));
E void FDECL(detect_wsegs, (struct monst *,BOOLEAN_P));
E void FDECL(save_worm, (int,int));
E void FDECL(rest_worm, (int));
E void FDECL(place_wsegs, (struct monst *));
E void FDECL(remove_worm, (struct monst *));
E void FDECL(place_worm_tail_randomly, (struct monst *,XCHAR_P,XCHAR_P));
E int FDECL(count_wsegs, (struct monst *));
E boolean FDECL(worm_known, (struct monst *));

/* ### worn.c ### */

E void FDECL(setworn, (struct obj *,long));
E void FDECL(setnotworn, (struct obj *));
E void FDECL(mon_set_minvis, (struct monst *));
E void FDECL(mon_adjust_speed, (struct monst *,int,struct obj *));
E void FDECL(update_mon_intrinsics,
		(struct monst *,struct obj *,BOOLEAN_P,BOOLEAN_P));
E int FDECL(base_mac, (struct monst *));
E int FDECL(find_mac, (struct monst *));
E int FDECL(full_mac, (struct monst *));
E int FDECL(full_marmorac, (struct monst *));
E void FDECL(m_dowear, (struct monst *,BOOLEAN_P));
E struct obj *FDECL(which_armor, (struct monst *,long));
E void FDECL(mon_break_armor, (struct monst *,BOOLEAN_P));
E int FDECL(extra_pref, (struct monst *, struct obj *));
E void FDECL(bypass_obj, (struct obj *));
E void NDECL(clear_bypasses);
E void FDECL(light_damage, (genericptr_t, long));

/* ### write.c ### */

E int FDECL(dowrite, (struct obj *));

/* ### zap.c ### */

E int FDECL(bhitm, (struct monst *,struct obj *));
E void FDECL(probe_monster, (struct monst *));
E boolean FDECL(get_obj_location, (struct obj *,xchar *,xchar *,int));
E boolean FDECL(get_mon_location, (struct monst *,xchar *,xchar *,int));
E struct monst *FDECL(get_container_location, (struct obj *obj, int *, int *));
E struct monst *FDECL(montraits, (struct obj *,coord *));
E struct monst *FDECL(revive, (struct obj *));
E int FDECL(unturn_dead, (struct monst *));
E void FDECL(cancel_item, (struct obj *));
E boolean FDECL(drain_item, (struct obj *));
E boolean FDECL(damage_item, (struct obj *));
E struct obj *FDECL(poly_obj, (struct obj *, int));
E boolean FDECL(obj_resists, (struct obj *,int,int));
E boolean FDECL(obj_shudders, (struct obj *));
E void FDECL(do_osshock, (struct obj *));
E int FDECL(bhito, (struct obj *,struct obj *));
E int FDECL(bhitpile, (struct obj *,int (*)(OBJ_P,OBJ_P),int,int));
E int FDECL(zappable, (struct obj *));
E void FDECL(zapnodir, (struct obj *));
E int NDECL(dozap);
E int FDECL(zapyourself, (struct obj *,BOOLEAN_P));
E boolean FDECL(cancel_monst, (struct monst *,struct obj *,
			       BOOLEAN_P,BOOLEAN_P,BOOLEAN_P,int));
E void FDECL(weffects, (struct obj *));
E int NDECL(spell_damage_bonus);
E const char *FDECL(exclam, (int force));
E void FDECL(hit, (const char *,struct monst *,const char *));
E void FDECL(miss, (const char *,struct monst *));
E struct monst *FDECL(bhit, (int,int,int,int,int (*)(MONST_P,OBJ_P),
			     int (*)(OBJ_P,OBJ_P),struct obj *, boolean *));
E struct monst *FDECL(boomhit, (struct obj *,int,int));
E int FDECL(burn_floor_paper, (int,int,BOOLEAN_P,BOOLEAN_P));
E void FDECL(buzz, (int,int,XCHAR_P,XCHAR_P,int,int,int,int));
E void FDECL(melt_ice, (XCHAR_P,XCHAR_P));
E int FDECL(zap_over_floor, (XCHAR_P,XCHAR_P,int,boolean *));
E void FDECL(fracture_rock, (struct obj *));
E boolean FDECL(break_statue, (struct obj *));
E boolean FDECL(break_crate, (struct obj *));
E void FDECL(destroy_item, (int,int));
E int FDECL(destroy_mitem, (struct monst *,int,int));
E int FDECL(resist, (struct monst *,CHAR_P,int,int));
E void NDECL(makewish);
E void FDECL(costly_cancel, (struct obj *));
E void FDECL(revive_egg, (struct obj *));

#endif /* !MAKEDEFS_C && !LEV_LEX_C */

#undef E

#endif /* EXTERN_H */
