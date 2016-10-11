/*	SCCS Id: @(#)were.c	3.4	2002/11/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef OVL0

void
were_change(mon)
register struct monst *mon;
{
	if( !is_were(mon->data) && 
		!(is_heladrin(mon->data) && mon->mhp < .5*mon->mhpmax && rn2(2)) && 
		!(is_eeladrin(mon->data) && mon->mhp > .5*mon->mhpmax) &&
		!(is_yochlol(mon->data) && mon->mhp < .5*mon->mhpmax)
	) return;

	if (is_human(mon->data)) {
	    if (!Protection_from_shape_changers &&
		!rn2(night() ? (flags.moonphase == FULL_MOON ?  3 : 30)
			     : (flags.moonphase == FULL_MOON ? 10 : 50))) {
		new_were(mon);		/* change into animal form */
		if (flags.soundok && !canseemon(mon)) {
		    const char *howler;

		    switch (monsndx(mon->data)) {
		    case PM_WEREWOLF:	howler = "wolf";    break;
		    case PM_WEREJACKAL: howler = "jackal";  break;
		    case PM_ANUBAN_JACKAL: howler = "jackal";  break;
		    default:		howler = (char *)0; break;
		    }
		    if (howler)
			You_hear("a %s howling at the moon.", howler);
		}
	    }
	} else if (!rn2(30) || (is_were(mon->data) && Protection_from_shape_changers) 
		|| is_heladrin(mon->data) 
		|| is_yochlol(mon->data) 
		|| (is_eeladrin(mon->data) && mon->mhp >= mon->mhpmax)
	) {
		if(mon->data == &mons[PM_BALL_OF_LIGHT]){
			mon->mflee = 0;
			mon->mfleetim = 0;
		}
	    new_were(mon);		/* change back into human form */
		if(is_yochlol(mon->data)) mon->movement += 12;
	}
}

#endif /* OVL0 */
#ifdef OVLB

STATIC_DCL int FDECL(counter_were,(int));

STATIC_OVL int
counter_were(pm)
int pm;
{
	switch(pm) {
	    case PM_WEREWOLF:	      return(PM_HUMAN_WEREWOLF);
	    case PM_HUMAN_WEREWOLF:   return(PM_WEREWOLF);
	    case PM_WEREJACKAL:	      return(PM_HUMAN_WEREJACKAL);
	    case PM_HUMAN_WEREJACKAL: return(PM_WEREJACKAL);
	    case PM_WERERAT:	      return(PM_HUMAN_WERERAT);
	    case PM_HUMAN_WERERAT:    return(PM_WERERAT);
		case PM_ANUBITE:		  return(PM_ANUBAN_JACKAL);
		case PM_ANUBAN_JACKAL:	  return(PM_ANUBITE);
		case PM_COURE:			  return(PM_MOTE_OF_LIGHT);
		case PM_MOTE_OF_LIGHT:	  return(PM_COURE);
		case PM_NOVIERE:		  return(PM_WATER_DOLPHIN);
		case PM_WATER_DOLPHIN:	  return(PM_NOVIERE);
		case PM_BRALANI:		  return(PM_SINGING_SAND);
		case PM_SINGING_SAND:	  return(PM_BRALANI);
		case PM_FIRRE:			  return(PM_DANCING_FLAME);
		case PM_DANCING_FLAME:	  return(PM_FIRRE);
		case PM_SHIERE:			  return(PM_BALL_OF_LIGHT);
		case PM_BALL_OF_LIGHT:	  return(PM_SHIERE);
		case PM_GHAELE:			  return(PM_LUMINOUS_CLOUD);
		case PM_LUMINOUS_CLOUD:	  return(PM_GHAELE);
		case PM_TULANI:			  return(PM_BALL_OF_RADIANCE);
		case PM_BALL_OF_RADIANCE: return(PM_TULANI);
		case PM_GWYNHARWYF:		  return(PM_FURIOUS_WHIRLWIND);
		case PM_FURIOUS_WHIRLWIND: return(PM_GWYNHARWYF);
		case PM_ASCODEL:		  return(PM_BLOODY_SUNSET);
		case PM_BLOODY_SUNSET:	  return(PM_ASCODEL);
		case PM_FAERINAAL:		  return(PM_BALL_OF_GOSSAMER_SUNLIGHT);
		case PM_BALL_OF_GOSSAMER_SUNLIGHT: return(PM_FAERINAAL);
		case PM_QUEEN_MAB:		  return(PM_COTERIE_OF_MOTES);
		case PM_COTERIE_OF_MOTES: return(PM_QUEEN_MAB);
		case PM_KETO:			  return(PM_ANCIENT_TEMPEST);
		case PM_ANCIENT_TEMPEST:  return(PM_KETO);
		case PM_TULANI_QUEEN:	  return(PM_ETERNAL_LIGHT);
		case PM_ETERNAL_LIGHT:	  return(PM_TULANI_QUEEN);
		case PM_ALRUNES:		  return(PM_HATEFUL_WHISPERS);
		case PM_HATEFUL_WHISPERS: return(PM_ALRUNES);
		
		case PM_YOCHLOL: 
			switch(rnd(3)){
			case 1: return(PM_UNEARTHLY_DROW);
			case 2: return(PM_STINKING_CLOUD);
			case 3: return(PM_DEMONIC_BLACK_WIDOW);
		}
		case PM_UNEARTHLY_DROW: 
			switch(rnd(3)){
			case 1: return(PM_YOCHLOL);
			case 2: return(PM_STINKING_CLOUD);
			case 3: return(PM_DEMONIC_BLACK_WIDOW);
		}
		case PM_STINKING_CLOUD: 
			switch(rnd(3)){
			case 1: return(PM_UNEARTHLY_DROW);
			case 2: return(PM_YOCHLOL);
			case 3: return(PM_DEMONIC_BLACK_WIDOW);
		}
		case PM_DEMONIC_BLACK_WIDOW: 
			switch(rnd(3)){
			case 1: return(PM_UNEARTHLY_DROW);
			case 2: return(PM_STINKING_CLOUD);
			case 3: return(PM_YOCHLOL);
		}
	    default:			      return(0);
	}
}

void
new_were(mon)
struct monst *mon;
{
	int pm;
	struct permonst *olddata = mon->data;
	struct obj *otmp;

	pm = counter_were(monsndx(mon->data));
	if(!pm) {
	    impossible("unknown lycanthrope %s.", mon->data->mname);
	    return;
	}

	if(canseemon(mon) && !Hallucination) {
		if(mon->data != &mons[PM_ANUBITE] && mon->data != &mons[PM_ANUBAN_JACKAL] &&
		  !is_eladrin(mon->data) && !is_yochlol(mon->data)
		) pline("%s changes into a %s.", Monnam(mon),
			is_human(&mons[pm]) ? "human" :
			mons[pm].mname+4);
		else pline("%s changes into a %s.", Monnam(mon),
			mons[pm].mname);
	}

	set_mon_data(mon, &mons[pm], 0);
	if (mon->msleeping || !mon->mcanmove) {
	    /* transformation wakens and/or revitalizes */
	    mon->msleeping = 0;
	    mon->mfrozen = 0;	/* not asleep or paralyzed */
	    mon->mcanmove = 1;
	}
	/* regenerate by 1/4 of the lost hit points */
	mon->mhp += (mon->mhpmax - mon->mhp) / 4;
	if (emits_light(olddata) != emits_light(mon->data)) {
	    /* used to give light, now doesn't, or vice versa,
	       or light's range has changed */
	    if (emits_light(olddata))
		del_light_source(LS_MONSTER, (genericptr_t)mon);
	    if (emits_light(mon->data))
		new_light_source(mon->mx, mon->my, emits_light(mon->data),
				 LS_MONSTER, (genericptr_t)mon);
	}
	newsym(mon->mx,mon->my);
	if(is_eeladrin(mon->data)){
		for(otmp = mon->minvent; otmp; otmp = otmp->nobj){
			mon->misc_worn_check &= ~otmp->owornmask;
			if (otmp->owornmask)
				update_mon_intrinsics(mon, otmp, FALSE, FALSE);
			otmp->owornmask = 0L;
		}
		if(mon->data == &mons[PM_ANCIENT_TEMPEST]){
			struct monst *ltnt;
			for(ltnt = fmon; ltnt; ltnt = ltnt->nmon) if(ltnt->data == &mons[PM_WIDE_CLUBBED_TENTACLE]){
				switch(rnd(4)){
					case 1:
						if (emits_light(ltnt->data)) del_light_source(LS_MONSTER, (genericptr_t)ltnt);
						set_mon_data(ltnt, &mons[PM_WATER_ELEMENTAL], 0);
						newsym(ltnt->mx,ltnt->my);
						if (emits_light(ltnt->data)) new_light_source(ltnt->mx, ltnt->my, emits_light(ltnt->data),
								 LS_MONSTER, (genericptr_t)ltnt);
					break;
					case 2:
						if (emits_light(ltnt->data)) del_light_source(LS_MONSTER, (genericptr_t)ltnt);
						set_mon_data(ltnt, &mons[PM_AIR_ELEMENTAL], 0);
						newsym(ltnt->mx,ltnt->my);
						if (emits_light(ltnt->data)) new_light_source(ltnt->mx, ltnt->my, emits_light(ltnt->data),
								 LS_MONSTER, (genericptr_t)ltnt);
					break;
					case 3:
						if (emits_light(ltnt->data)) del_light_source(LS_MONSTER, (genericptr_t)ltnt);
						set_mon_data(ltnt, &mons[PM_LIGHTNING_PARAELEMENTAL], 0);
						newsym(ltnt->mx,ltnt->my);
						if (emits_light(ltnt->data)) new_light_source(ltnt->mx, ltnt->my, emits_light(ltnt->data),
								 LS_MONSTER, (genericptr_t)ltnt);
					break;
					case 4:
						if (emits_light(ltnt->data)) del_light_source(LS_MONSTER, (genericptr_t)ltnt);
						set_mon_data(ltnt, &mons[PM_ICE_PARAELEMENTAL], 0);
						newsym(ltnt->mx,ltnt->my);
						if (emits_light(ltnt->data)) new_light_source(ltnt->mx, ltnt->my, emits_light(ltnt->data),
								 LS_MONSTER, (genericptr_t)ltnt);
					break;
				}
			}
		}
	} else if(is_heladrin(mon->data)){
		m_dowear(mon, TRUE);
		init_mon_wield_item(mon);
	} else mon_break_armor(mon, FALSE);
	possibly_unwield(mon, FALSE);
}

int
were_summon(ptr,yours,visible,genbuf)	/* were-creature (even you) summons a horde */
register struct permonst *ptr;
register boolean yours;
int *visible;			/* number of visible helpers created */
char *genbuf;
{
	register int i, typ, pm = monsndx(ptr);
	register struct monst *mtmp;
	int total = 0;

	*visible = 0;
	if(Protection_from_shape_changers && !yours)
		return 0;
	for(i = rnd(5); i > 0; i--) {
	   switch(pm) {

		case PM_WERERAT:
		case PM_HUMAN_WERERAT:
			typ = rn2(3) ? PM_SEWER_RAT : rn2(3) ? PM_GIANT_RAT : PM_RABID_RAT ;
			if (genbuf) Strcpy(genbuf, "rat");
			break;
		case PM_WEREJACKAL:
		case PM_HUMAN_WEREJACKAL:
			typ = PM_JACKAL;
			if (genbuf) Strcpy(genbuf, "jackal");
			break;
		case PM_WEREWOLF:
		case PM_HUMAN_WEREWOLF:
			typ = rn2(5) ? PM_WOLF : PM_WINTER_WOLF ;
			if (genbuf) Strcpy(genbuf, "wolf");
			break;
		case PM_ANUBITE:
		case PM_ANUBAN_JACKAL:
			typ = PM_WEREJACKAL;
			if (genbuf) Strcpy(genbuf, "werejackal");
		default:
			continue;
	    }
	    mtmp = makemon(&mons[typ], u.ux, u.uy, NO_MM_FLAGS);
	    if (mtmp) {
		total++;
		if (canseemon(mtmp)) *visible += 1;
	    }
	    if (yours && mtmp)
		(void) tamedog(mtmp, (struct obj *) 0);
	}
	return total;
}

void
you_were()
{
	char qbuf[QBUFSZ];

	if (Unchanging || (u.umonnum == u.ulycn)) return;
	if (Polymorph_control) {
	    /* `+4' => skip "were" prefix to get name of beast */
	    Sprintf(qbuf, "Do you want to change into %s? ",
		    an(mons[u.ulycn].mname+4));
	    if(yn(qbuf) == 'n') return;
	}
	(void) polymon(u.ulycn);
}

void
you_unwere(purify)
boolean purify;
{
	if (purify) {
	    You_feel("purified.");
	    u.ulycn = NON_PM;	/* cure lycanthropy */
	}
	if (!Unchanging && is_were(youmonst.data) &&
		(!Polymorph_control || yn("Remain in beast form?") == 'n'))
	    rehumanize();
}

#endif /* OVLB */

/*were.c*/
