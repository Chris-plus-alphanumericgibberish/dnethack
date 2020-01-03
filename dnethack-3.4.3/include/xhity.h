#ifndef XHITY_H
#define XHITY_H

/* macros to unify player and monster */
#define x(mon)				((mon)==&youmonst ? u.ux : (mon)->mx)
#define y(mon)				((mon)==&youmonst ? u.uy : (mon)->my)
#define trapped(mon)		((mon)==&youmonst ? u.utrap : (mon)->mtrapped)
#define cantmove(mon)		((mon)==&youmonst ? (multi<0 || u.usleep) : helpless((mon)))
#define mlev(mon)			((mon)==&youmonst ? (Upolyd ? mons[u.umonnum].mlevel : u.ulevel) : (mon)->m_lev)
#define hp(mon)				((mon)==&youmonst ? (Upolyd ? &(u.mh) : &(u.uhp)) : &((mon)->mhp))
#define hpmax(mon)			((mon)==&youmonst ? (Upolyd ? &(u.mhmax) : &(u.uhpmax)) : &((mon)->mhpmax))
#define Fire_res(mon)		((mon)==&youmonst ? Fire_resistance : resists_fire((mon)))
#define InvFire_res(mon)	(((mon)==&youmonst ? EFire_resistance : resists_fire((mon))) || ward_at(x((mon)),y((mon))) == SIGIL_OF_CTHUGHA)
#define Cold_res(mon)		((mon)==&youmonst ? Cold_resistance : resists_cold((mon)))
#define InvCold_res(mon)	(((mon)==&youmonst ? ECold_resistance : resists_cold((mon))) || ward_at(x((mon)),y((mon))) == BRAND_OF_ITHAQUA)
#define Shock_res(mon)		((mon)==&youmonst ? Shock_resistance : resists_elec((mon)))
#define InvShock_res(mon)	(((mon)==&youmonst ? EShock_resistance : resists_elec((mon))) || ward_at(x((mon)),y((mon))) == TRACERY_OF_KARAKAL)
#define Acid_res(mon)		((mon)==&youmonst ? Acid_resistance : resists_acid((mon)))
#define InvAcid_res(mon)	((mon)==&youmonst ? EAcid_resistance : resists_acid((mon)))
#define Sleep_res(mon)		((mon)==&youmonst ? Sleep_resistance : resists_sleep((mon)))
#define Disint_res(mon)		((mon)==&youmonst ? Disint_resistance : resists_disint((mon)))
#define Poison_res(mon)		((mon)==&youmonst ? Poison_resistance : resists_poison((mon)))
#define Drain_res(mon)		((mon)==&youmonst ? Drain_resistance : resists_drli((mon)))
#define Sick_res(mon)		((mon)==&youmonst ? Sick_resistance : resists_sickness((mon)))
#define Stone_res(mon)		((mon)==&youmonst ? Stone_resistance : resists_ston((mon)))
#define Magic_res(mon)		((mon)==&youmonst ? Antimagic : resists_magm((mon)))
#define Half_phys(mon)		((mon)==&youmonst ? Half_physical_damage : mon_resistance((mon), HALF_PHDAM))
#define Change_res(mon)		((mon)==&youmonst ? Unchanging : mon_resistance((mon), UNCHANGING))
#define is_null_attk(attk)	((attk) && ((attk)->aatyp == 0 && (attk)->adtyp == 0 && (attk)->damn == 0 && (attk)->damd == 0))
#define creature_at(x,y)	(isok(x,y) ? MON_AT(x, y) ? level.monsters[x][y] : (x==u.ux && y==u.uy) ? &youmonst : (struct monst *)0 : (struct monst *)0)

#define FATAL_DAMAGE_MODIFIER 9001

#define VIS_MAGR	0x01	/* aggressor is clearly visible */
#define VIS_MDEF	0x02	/* defender is clearly visible */
#define VIS_NONE	0x04	/* you are aware of at least one of magr and mdef */

#define ATTACKCHECK_NONE		0x00	/* do not attack */
#define ATTACKCHECK_ATTACK		0x01	/* attack normally */
#define ATTACKCHECK_BLDTHRST	0x02	/* attack against the player's will */

/* TODO: put these in their specified header files */
/* additional TODO: add these to monsters in monst.c */
#define MB_IRON				0x01000000L
#define MB_SILVER			0x02000000L
/* mondata.h */
#define is_iron_mon(mon)	(((mon)->data->mflagsb & MB_IRON) != 0L)
#define is_silver_mon(mon)	(((mon)->data->mflagsb & MB_SILVER) != 0L || ((mon)==&youmonst && u.sealsActive&SEAL_EDEN))
#define is_holy_mon(mon)	(is_uvuudaum((mon)->data) || ((mon)->mfaction == ILLUMINATED))
#define is_unholy_mon(mon)	((mon)->data == &mons[PM_UVUUDAUM])
/* obj. h*/
#define multistriking(otmp)	(!(otmp) ? 0 : \
	(otmp)->otyp == SET_OF_CROW_TALONS ? 2 : \
	(otmp)->otyp == VIPERWHIP ? ((otmp)->ovar1 - 1) : \
	arti_threeHead((otmp)) ? 2 : \
	arti_tentRod((otmp)) ? 6 : \
	0)
#define martial_aid(otmp)	(is_lightsaber((otmp)) && !litsaber((otmp)) && (otmp)->otyp != KAMEREL_VAJRA)
#define valid_weapon(otmp)		((otmp)->oclass == WEAPON_CLASS || \
	is_weptool((otmp)) || \
	(otmp)->otyp == HEAVY_IRON_BALL || \
	(otmp)->otyp == CHAIN || \
	(otmp)->oclass == GEM_CLASS)
#define throwing_weapon(otmp)	(is_missile(otmp) || is_spear(otmp) || \
								(is_blade(otmp) && !is_sword(otmp) && \
								(objects[otmp->otyp].oc_dir & PIERCE)) || \
								otmp->otyp == WAR_HAMMER || otmp->otyp == AKLYS)
#endif