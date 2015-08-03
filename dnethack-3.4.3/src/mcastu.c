/*	SCCS Id: @(#)mcastu.c	3.4	2003/01/08	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

extern const int monstr[];
extern void demonpet();

/* tactics() may call for a specific spell */
/* 0 = no spell */
       /* attack spells */
#define PSI_BOLT			   1
#define OPEN_WOUNDS			   PSI_BOLT+1
#define MAGIC_MISSILE          OPEN_WOUNDS+1 /* magic missile */
#define DRAIN_LIFE             MAGIC_MISSILE+1  /* drain life */
#define ARROW_RAIN             DRAIN_LIFE+1
#define CONE_OF_COLD           ARROW_RAIN+1  /* cone of cold */
#define LIGHTNING              CONE_OF_COLD+1
#define FIRE_PILLAR            LIGHTNING+1
#define GEYSER                 FIRE_PILLAR+1
#define ACID_RAIN              GEYSER+1
#define SUMMON_MONS            ACID_RAIN+1
#define SUMMON_DEVIL           SUMMON_MONS+1
#define DEATH_TOUCH			   SUMMON_DEVIL+1
       /* healing spells */
#define CURE_SELF              DEATH_TOUCH+1  /* healing */
#define MASS_CURE_CLOSE        CURE_SELF+1  /* heal allies */
#define MASS_CURE_FAR          MASS_CURE_CLOSE+1  /* heal allies */
#define RECOVER                MASS_CURE_FAR+1  /* remove afflictions */
       /* divination spells */
#define MAKE_VISIBLE           RECOVER+1
       /* (dis)enchantment spells */
#define HASTE_SELF             MAKE_VISIBLE+1 /* haste self */
#define STUN_YOU               HASTE_SELF+1
#define CONFUSE_YOU            STUN_YOU+1
#define PARALYZE               CONFUSE_YOU+1
#define BLIND_YOU              PARALYZE+1
#define SLEEP                  BLIND_YOU+1 /* sleep */
#define DRAIN_ENERGY           SLEEP+1
#define WEAKEN_STATS           DRAIN_ENERGY+1
#define WEAKEN_YOU			   WEAKEN_STATS+1
#define DESTRY_ARMR            WEAKEN_YOU+1
#define DESTRY_WEPN            DESTRY_ARMR+1
#define EVIL_EYE			   DESTRY_WEPN+1
       /* clerical spells */
#define CURSE_ITEMS            EVIL_EYE+1
#define INSECTS                CURSE_ITEMS+1
#define RAISE_DEAD             INSECTS+1
#define SUMMON_ANGEL           RAISE_DEAD+1
#define SUMMON_ALIEN           SUMMON_ANGEL+1
#define PLAGUE                 SUMMON_ALIEN+1
#define PUNISH                 PLAGUE+1
#define AGGRAVATION			   PUNISH+1
       /* escape spells */
#define DISAPPEAR              AGGRAVATION+1 /* invisibility */
       /* matter spells */
#define DARKNESS               DISAPPEAR+1
#define SUMMON_SPHERE          DARKNESS+1 /* flame sphere */
#define MAKE_WEB               SUMMON_SPHERE+1
#define DROP_BOULDER           MAKE_WEB+1
#define EARTHQUAKE             DROP_BOULDER+1
#define TURN_TO_STONE          EARTHQUAKE+1
       /* unique monster spells */
#define NIGHTMARE              TURN_TO_STONE+1
#define FILTH                  NIGHTMARE+1
#define CLONE_WIZ              FILTH+1
#define STRANGLE               CLONE_WIZ+1

extern void you_aggravate(struct monst *);

STATIC_DCL void FDECL(cursetxt,(struct monst *,BOOLEAN_P));
STATIC_DCL int FDECL(choose_magic_spell, (int,int,boolean));
STATIC_DCL int FDECL(choose_clerical_spell, (int,int,boolean));
STATIC_DCL void FDECL(cast_spell,(struct monst *, int,int));
STATIC_DCL boolean FDECL(is_undirected_spell,(int));
STATIC_DCL boolean FDECL(spell_would_be_useless,(struct monst *,int));
STATIC_DCL boolean FDECL(mspell_would_be_useless,(struct monst *,struct monst *,int));
STATIC_DCL boolean FDECL(uspell_would_be_useless,(struct monst *,int));
STATIC_DCL void FDECL(ucast_spell,(struct monst *,struct monst *,int,int));

#ifdef OVL0

extern const char * const flash_types[];	/* from zap.c */

/* feedback when frustrated monster couldn't cast a spell */
STATIC_OVL
void
cursetxt(mtmp, undirected)
struct monst *mtmp;
boolean undirected;
{
	if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
	    const char *point_msg;  /* spellcasting monsters are impolite */

	    if (undirected)
		point_msg = "all around, then curses";
	    else if ((Invis && !perceives(mtmp->data) &&
			(mtmp->mux != u.ux || mtmp->muy != u.uy)) ||
		    (youmonst.m_ap_type == M_AP_OBJECT &&
			youmonst.mappearance == STRANGE_OBJECT) ||
		    u.uundetected)
		point_msg = "and curses in your general direction";
	    else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
		point_msg = "and curses at your displaced image";
	    else
		point_msg = "at you, then curses";

	    pline("%s points %s.", Monnam(mtmp), point_msg);
	} else if ((!(moves % 4) || !rn2(4))) {
	    if (flags.soundok) Norep("You hear a mumbled curse.");
	}
}

#endif /* OVL0 */
#ifdef OVLB

/* default spell selection for mages */
STATIC_OVL int
choose_magic_spell(spellval,mid,hostile)
int spellval;
int mid;
boolean hostile;
{
	/* Alternative spell lists: since the alternative lists contain spells that aren't
		yet implemented for m vs m combat, non-hostile monsters always use the vanilla 
		list. Alternate list slection is based on the monster's ID number, which is
		annotated as staying constant.
	*/
	if(!hostile || mid % 10 < 5){
    switch (spellval % 24) {
    case 23:
    case 22:
    return TURN_TO_STONE;
    case 21:
    case 20:
	return DEATH_TOUCH;
    case 19:
    case 18:
	return CLONE_WIZ;
    case 17:
    case 16:
    case 15:
	return SUMMON_MONS;
    case 14:
    case 13:
	return AGGRAVATION;
    case 12:
    case 11:
    case 10:
	return CURSE_ITEMS;
    case 9:
    case 8:
	return DESTRY_ARMR;
    case 7:
    case 6:
	return WEAKEN_YOU;
    case 5:
    case 4:
	return DISAPPEAR;
    case 3:
	return STUN_YOU;
    case 2:
	return HASTE_SELF;
    case 1:
	return CURE_SELF;
    case 0:
    default:
	return PSI_BOLT;
    }
	}else if(mid % 10 < 7){
     switch (spellval % 24) {
   case 23:
    case 22:
    return DRAIN_LIFE;
    case 21:
    case 20:
	return DRAIN_ENERGY;
    case 19:
    case 18:
	return CLONE_WIZ;
    case 17:
    case 16:
    case 15:
	return WEAKEN_STATS;
    case 14:
    case 13:
	return CONFUSE_YOU;
    case 12:
    case 11:
    case 10:
	return DRAIN_LIFE;
    case 9:
    case 8:
	return DESTRY_WEPN;
    case 7:
    case 6:
	return WEAKEN_YOU;
    case 5:
    case 4:
	return DARKNESS;
    case 3:
	return STUN_YOU;
    case 2:
	return MAKE_WEB;
    case 1:
	return CURE_SELF;
    case 0:
    default:
	return PSI_BOLT;
	}
	}else if(mid % 10 < 9){
    switch (spellval % 24) {
    case 23:
    case 22:
    return CURE_SELF;
    case 21:
    case 20:
	return ARROW_RAIN;
    case 19:
    case 18:
	return CLONE_WIZ;
    case 17:
    case 16:
    case 15:
	return SUMMON_MONS;
    case 14:
    case 13:
	return SUMMON_SPHERE;
    case 12:
    case 11:
    case 10:
	return DROP_BOULDER;
    case 9:
    case 8:
	return DESTRY_ARMR;
    case 7:
    case 6:
	return EVIL_EYE;
    case 5:
    case 4:
	return MAKE_VISIBLE;
    case 3:
	return SUMMON_SPHERE;
    case 2:
	return HASTE_SELF;
    case 1:
	return CURE_SELF;
    case 0:
    default:
	return SUMMON_SPHERE;
	}
	}else{
    switch (spellval % 24) {
    case 23:
    case 22:
    case 21:
    case 20:
    case 19:
	return DRAIN_LIFE;
    case 18:
	return CLONE_WIZ;
    case 17:
    case 16:
    case 15:
	return SUMMON_MONS;
    case 14:
    case 13:
	return AGGRAVATION;
    case 12:
    case 11:
    case 10:
	return CURSE_ITEMS;
    case 9:
	return DESTRY_WEPN;
    case 8:
	return DESTRY_ARMR;
    case 7:
    case 6:
	return EVIL_EYE;
    case 5:
	return MAKE_VISIBLE;
    case 4:
	return DISAPPEAR;
    case 3:
	return STUN_YOU;
    case 2:
	return HASTE_SELF;
    case 1:
	return CONFUSE_YOU;
    case 0:
    default:
	return PSI_BOLT;
	}
	}
}

/* default spell selection for priests/monks */
STATIC_OVL int
choose_clerical_spell(spellnum,mid,hostile)
int spellnum;
int mid;
boolean hostile;
{
	/* Alternative spell lists: since the alternative lists contain spells that aren't
		yet implemented for m vs m combat, non-hostile monsters always use the vanilla 
		list. Alternate list slection is based on the monster's ID number, which is
		annotated as staying constant. Priests are divided up into constructive and
		destructive casters (constructives favor heal self, destructives favor 
		inflict wounds). Their spell list is divided into blocks. The order that
		they recieve spells from each block is radomized based on their monster
		ID.
	*/
	if(!hostile){
     switch (spellnum % 18) {
     case 17:
       return PUNISH;
     case 16:
       return SUMMON_ANGEL;
     case 14:
       return PLAGUE;
    case 13:
       return ACID_RAIN;
    case 12:
	return GEYSER;
    case 11:
	return FIRE_PILLAR;
    case 9:
	return LIGHTNING;
    case 8:
       return DRAIN_LIFE;
    case 7:
       return CURSE_ITEMS;
    case 6:
       return INSECTS;
    case 4:
	return BLIND_YOU;
    case 3:
	return PARALYZE;
    case 2:
	return CONFUSE_YOU;
    case 1:
	return OPEN_WOUNDS;
    case 0:
    default:/*5,10,15,18+*/
	return CURE_SELF;
    }
	}else{
		 spellnum = spellnum % 18;
		//case "17"
		if(spellnum == ((mid/100+3)%4)+14) return PUNISH;
		//case "16"
		//case "15"
		//Cure/Inflict
		if(spellnum == ((mid/100+2)%4)+14) return (mid % 2) ? SUMMON_ANGEL : SUMMON_DEVIL;
		//case "14"
		if(spellnum == ((mid/100+0)%4)+14) return PLAGUE;
		//case "13"
		if(spellnum == ((mid+4)%5)+9) return EARTHQUAKE;
		//case "12"
		if(spellnum == ((mid+3)%5)+9) return ( (mid/11) % 2) ? GEYSER : ACID_RAIN;
		//case "11"
		if(spellnum == ((mid+2)%5)+9) return FIRE_PILLAR;
		//case "10"
		//Cure/Inflict
		//case "9"
		if(spellnum == ((mid+0)%5)+9) return LIGHTNING;
		//case "8"
		if(spellnum == ((mid/10+3)%4)+5) return DRAIN_LIFE;
		//case "7"
		if(spellnum == ((mid/10+2)%4)+5) return ( (mid/3) % 2) ? CURSE_ITEMS : EVIL_EYE;
		//case "6"
		if(spellnum == ((mid/10+1)%4)+5) return INSECTS;
		//case "5"
		//Cure/Inflict
		//case "4"
		if(spellnum == ((mid+2)%3)+2) return BLIND_YOU;
		//case "3"
		if(spellnum == ((mid+1)%3)+2) return PARALYZE;
		//case "2"
		if(spellnum == ((mid+0)%3)+2) return CONFUSE_YOU;
		//case "1"
		if(spellnum == ((mid+1)%2)+0) return ( (mid+1) % 2) ? CURE_SELF : OPEN_WOUNDS;
		//case "0", "5", "10", "15", "18+"
		return (mid % 2) ? CURE_SELF : OPEN_WOUNDS;
	}
}

/* ...but first, check for monster-specific spells */
STATIC_OVL int
choose_magic_special(mtmp, type)
struct monst *mtmp;
unsigned int type;
{
    if (rn2(2)) {
       switch(monsndx(mtmp->data)) {
       case PM_WIZARD_OF_YENDOR:
           return (rn2(4) ? rnd(STRANGLE) :
                 (!rn2(3) ? STRANGLE : !rn2(2) ? CLONE_WIZ : HASTE_SELF));

       case PM_ORCUS:
       case PM_NALZOK:
           if (rn2(2)) return RAISE_DEAD;
		break;
       case PM_DISPATER:
           if (rn2(2)) return (rn2(2) ? TURN_TO_STONE : CURSE_ITEMS);
		break;
       case PM_DEMOGORGON:
           return (!rn2(3) ? HASTE_SELF : rn2(2) ? FILTH : WEAKEN_YOU);
		   
       case PM_LAMASHTU:
			// pline("favored");
			switch(rnd(8)){
				case 1:
					return SUMMON_ANGEL;
				break;
				case 2:
					return SUMMON_DEVIL;
				break;
				case 3:
					return SUMMON_ALIEN;
				break;
				case 4:
					return NIGHTMARE;
				break;
				case 5:
					return FILTH;
				break;
				case 6:
					return CURSE_ITEMS;
				break;
				case 7:
					return DEATH_TOUCH;
				break;
				case 8:
					return EVIL_EYE;
				break;
			}
		break;

       case PM_APPRENTICE:
          if (rn2(3)) return SUMMON_SPHERE;
       /* fallthrough */
       case PM_NEFERET_THE_GREEN:
           return ARROW_RAIN;

       case PM_DARK_ONE:
           return (!rn2(4) ? TURN_TO_STONE : !rn2(3) ? RAISE_DEAD :
                    rn2(2) ? DARKNESS : MAKE_WEB);

       case PM_THOTH_AMON:
           if (!rn2(3)) return NIGHTMARE;
       /* fallthrough */
       case PM_CHROMATIC_DRAGON:
           return (rn2(2) ? DESTRY_WEPN : EARTHQUAKE);

       case PM_IXOTH:
			return FIRE_PILLAR;
       case PM_NIMUNE:
           if(!rn2(3)) return NIGHTMARE;
		   else if(rn2(2)) return MASS_CURE_CLOSE;
		   else return SLEEP;

	case PM_CHAOS:
		switch(rn2(10)){
			case 0:
				return CURE_SELF;
			break;
			case 1:
			case 2:
			case 3:
				return LIGHTNING;
			break;
			case 4:
			case 5:
			case 6:
				return FIRE_PILLAR;
			break;
			case 7:
			case 8:
			case 9:
				return GEYSER;
			break;
		}

       case PM_GRAND_MASTER:
       case PM_MASTER_KAEN:
          return (rn2(2) ? WEAKEN_YOU : EARTHQUAKE);

       case PM_MINION_OF_HUHETOTL:
           return (rn2(2) ? CURSE_ITEMS : (rn2(2) ? DESTRY_WEPN : DROP_BOULDER));

       case PM_TITAN:
           return (rn2(2) ? DROP_BOULDER : LIGHTNING);
       case PM_ARCHON:
           return (rn2(2) ? SUMMON_ANGEL : LIGHTNING);
       case PM_KI_RIN:
           return FIRE_PILLAR;

       case PM_ARCH_LICH:
           if (!rn2(6)) return TURN_TO_STONE;
       /* fallthrough */
#if 0
       case PM_VAMPIRE_MAGE:
#endif
       case PM_MASTER_LICH:
           if (!rn2(5)) return RAISE_DEAD;
       /* fallthrough */
       case PM_DEMILICH:
           if (!rn2(4)) return DRAIN_LIFE;
       /* fallthrough */
       case PM_LICH:
           if (!rn2(3)) return CURSE_ITEMS;
       /* fallthrough */
       case PM_NALFESHNEE:
           if (rn2(2)) return (rn2(2) ? DESTRY_ARMR : DESTRY_WEPN);
       /* fallthrough */
       case PM_BARROW_WIGHT:
           return (!rn2(3) ? DARKNESS : (rn2(2) ? MAKE_WEB : SLEEP));

       case PM_GNOMISH_WIZARD:
           if (rn2(2)) return SUMMON_SPHERE;
       }
    }//50% favored spells
	
	//100% favored spells
	switch(monsndx(mtmp->data)) {
	case PM_GREAT_HIGH_SHAMAN_OF_KURTULMAK:
		return SUMMON_DEVIL; 
	case PM_SHOGGOTH:
		if(!rn2(20)) return SUMMON_MONS; 
		else return 0;
	case PM_VERIER: 
		if(!rn2(3)) return WEAKEN_YOU;
		else return DESTRY_ARMR;
	case PM_CRONE_LILITH:
		switch(rn2(6)){
			case 0:
			case 1:
			case 2:
				return CURSE_ITEMS;
			break;
			case 3:
				return WEAKEN_STATS;
			break;
			case 4:
				return CURE_SELF;
			break;
			case 5:
				return DEATH_TOUCH;
			break;
		}
	case PM_MILITANT_CLERIC:
		switch(rn2(6)){
			case 0:
			case 1:
				return CURE_SELF;
			break;
			case 2:
				return MASS_CURE_FAR;
			break;
			case 3:
				return MASS_CURE_CLOSE;
			break;
			case 4:
				return RECOVER;
			break;
			case 5:
				return FIRE_PILLAR;
			break;
		}
	case PM_ADVENTURING_WIZARD:
		switch(rn2(4)){
			case 0:
			case 1:
				return SLEEP;
			break;
			case 2:
				return MAGIC_MISSILE;
			break;
			case 3:
				return SUMMON_SPHERE;
			break;
		}
	case PM_PALE_NIGHT:
		switch(rn2(5)){
			case 0:
				return OPEN_WOUNDS;
			break;
			case 1:
				return WEAKEN_YOU;
			break;
			case 2:
				return PSI_BOLT;
			break;
			case 3:
				return CURSE_ITEMS;
			break;
			case 4:
				return DEATH_TOUCH;
			break;
		}
	case PM_ASMODEUS:
		switch(rn2(9)){
			case 0:
				return CURE_SELF;
			break;
			case 1:
				return OPEN_WOUNDS;
			break;
			case 2:
				return ACID_RAIN;
			break;
			case 3:
				return LIGHTNING;
			break;
			case 4:
				return FIRE_PILLAR;
			break;
			case 5:
				return GEYSER;
			break;
			case 6:
				return SUMMON_MONS;
			break;
			case 7:
				return PARALYZE;
			break;
			case 8:
				return DEATH_TOUCH;
			break;
		}
	case PM_MINOTAUR_PRIESTESS:
		switch (d(1,5)+8) {
			case 13:
			return GEYSER;
			break;
			case 12:
			return FIRE_PILLAR;
			break;
			case 11:
			return LIGHTNING;
			break;
			case 10:
			case 9:
			return CURSE_ITEMS;
			break;
			default:
			return OPEN_WOUNDS;
			break;
		}
	case PM_GNOLL_MATRIARCH:
		switch (d(1,10)-4) {
			case 6:
				return BLIND_YOU;
			break;
			case 5:
			case 4:
				return PARALYZE;
			break;
			case 3:
			case 2:
				return CONFUSE_YOU;
			break;
			case 1:
				return MASS_CURE_FAR;
			break;
			case 0:
			default:
				return OPEN_WOUNDS;
			break;
		}
	}
    if (type == AD_CLRC)
        return choose_clerical_spell(rn2(mtmp->m_lev),mtmp->mnum,!(mtmp->mpeaceful));
    return choose_magic_spell(rn2(mtmp->m_lev),mtmp->mnum,!(mtmp->mpeaceful));
}

/* return values:
 * 1: successful spell
 * 0: unsuccessful spell
 */
int
castmu(mtmp, mattk, thinks_it_foundyou, foundyou)
	register struct monst *mtmp;
	register struct attack *mattk;
	boolean thinks_it_foundyou;
	boolean foundyou;
{
	int	dmg, ml = mtmp->m_lev;
	int ret;
	int spellnum = 0;

	if(mtmp->data->maligntyp < 0 && u.uz.dnum == law_dnum && on_level(&illregrd_level,&u.uz)) return 0;
	/* Three cases:
	 * -- monster is attacking you.  Search for a useful spell.
	 * -- monster thinks it's attacking you.  Search for a useful spell,
	 *    without checking for undirected.  If the spell found is directed,
	 *    it fails with cursetxt() and loss of mspec_used.
	 * -- monster isn't trying to attack.  Select a spell once.  Don't keep
	 *    searching; if that spell is not useful (or if it's directed),
	 *    return and do something else. 
	 * Since most spells are directed, this means that a monster that isn't
	 * attacking casts spells only a small portion of the time that an
	 * attacking monster does.
	 */
	if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) && ml) {
	    int cnt = 40;
		
		if(Race_if(PM_DROW) && mtmp->data == &mons[PM_AVATAR_OF_LOLTH] && !Role_if(PM_EXILE) && !mtmp->mpeaceful){
			u.ugangr[Align2gangr(A_CHAOTIC)]++;
			angrygods(A_CHAOTIC);
		}
           do {
               spellnum = choose_magic_special(mtmp, mattk->adtyp);
				if(!spellnum) return 0; //The monster's spellcasting code aborted the cast.
		/* not trying to attack?  don't allow directed spells */
		if (!thinks_it_foundyou) {
				if (!is_undirected_spell(spellnum) ||
                       spell_would_be_useless(mtmp, spellnum)
			) {
//				if (foundyou)
//					impossible("spellcasting monster found you and doesn't know it?");
				return 0;
		    }
		    break;
		}
	    } while(--cnt > 0 &&
                   spell_would_be_useless(mtmp, spellnum));
	    if (cnt == 0) return 0;
	}

	/* monster unable to cast spells? */
	if(mtmp->mcan || mtmp->mspec_used || !ml || u.uinvulnerable || u.spiritPColdowns[PWR_PHASE_STEP] >= moves+20) {
	    cursetxt(mtmp, is_undirected_spell(spellnum));
	    return(0);
	}

	if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
	    if(mtmp->data == &mons[PM_HEDROW_WARRIOR]) mtmp->mspec_used = d(4,4);
		else mtmp->mspec_used = 10 - mtmp->m_lev;
	    if (mtmp->mspec_used < 2) mtmp->mspec_used = 2;
	}

	/* monster can cast spells, but is casting a directed spell at the
	   wrong place?  If so, give a message, and return.  Do this *after*
	   penalizing mspec_used. */
	if (!foundyou && thinks_it_foundyou &&
		!is_undirected_spell(spellnum)) {
	    pline("%s casts a spell at %s!",
		canseemon(mtmp) ? Monnam(mtmp) : "Something",
		levl[mtmp->mux][mtmp->muy].typ == WATER
		    ? "empty water" : "thin air");
	    return(0);
	}

	nomul(0, NULL);
       /* increased to rn2(ml*20) to make kobold/orc shamans less helpless */
       if(rn2(ml*20) < (mtmp->mconf ? 100 : 20)) {     /* fumbled attack */
	    if (canseemon(mtmp) && flags.soundok)
		pline_The("air crackles around %s.", mon_nam(mtmp));
	    return(0);
	}
	if (canspotmon(mtmp) || !is_undirected_spell(spellnum)) {
	    pline("%s casts a spell%s!",
		  canspotmon(mtmp) ? Monnam(mtmp) : "Something",
		  is_undirected_spell(spellnum) ? "" :
		  (Invisible && !perceives(mtmp->data) && 
		   (mtmp->mux != u.ux || mtmp->muy != u.uy)) ?
		  " at a spot near you" :
		  (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy)) ?
		  " at your displaced image" :
		  " at you");
	}

/*
 *	As these are spells, the damage is related to the level
 *	of the monster casting the spell.
 */
	if (!foundyou) {
	    dmg = 0;
	    if (mattk->adtyp != AD_SPEL && mattk->adtyp != AD_CLRC) {
		impossible(
	      "%s casting non-hand-to-hand version of hand-to-hand spell %d?",
			   Monnam(mtmp), mattk->adtyp);
		return(0);
	    }
	} else {
		int dmd = 6, dmn = ml/2;
		if (mattk->damd) dmd = (int)(mattk->damd);
		
		if (mattk->damn) dmn+= (int)(mattk->damn);
		else dmn += 1;
		
	    dmg = d(dmn, dmd);
	}
	if (Half_spell_damage) dmg = (dmg+1) / 2;

	ret = 1;

	switch (mattk->adtyp) {
		case AD_RBRE:
			switch(rnd(3)){
				case 1: goto elec_spell;
				case 2: goto fire_spell;
				case 3: goto cold_spell;
			}
		break;
	    case AD_ELEC:
elec_spell:
		pline("Lightning crackles around you.");
		if(Shock_resistance) {
			shieldeff(u.ux, u.uy);
			pline("But you resist the effects.");
			dmg = 0;
			if(!rn2(10)) destroy_item(WAND_CLASS, AD_ELEC);
		}
		else{
			destroy_item(WAND_CLASS, AD_ELEC);
			if(!rn2(10)) destroy_item(RING_CLASS, AD_ELEC);
		}
		stop_occupation();
		break;
	    case AD_FIRE:
fire_spell:
		pline("You're enveloped in flames.");
		if(Fire_resistance) {
			shieldeff(u.ux, u.uy);
			pline("But you resist the effects.");
			dmg = 0;
			if(!rn2(6)) destroy_item(POTION_CLASS, AD_FIRE);
			if(!rn2(10)) destroy_item(SCROLL_CLASS, AD_FIRE);
		}
		else{
			destroy_item(POTION_CLASS, AD_FIRE);
			if(!rn2(6)) destroy_item(SCROLL_CLASS, AD_FIRE);
			if(!rn2(10)) destroy_item(SPBOOK_CLASS, AD_FIRE);
		}
		burn_away_slime();
		stop_occupation();
		break;
	    case AD_COLD:
cold_spell:
		pline("You're covered in frost.");
		if(Cold_resistance) {
			shieldeff(u.ux, u.uy);
			pline("But you resist the effects.");
			dmg = 0;
			if(!rn2(4)) destroy_item(POTION_CLASS, AD_COLD);
		}
		else destroy_item(POTION_CLASS, AD_COLD);
		stop_occupation();
		break;
	    case AD_MAGM:
		You("are hit by a shower of missiles!");
		if(Antimagic) {
			shieldeff(u.ux, u.uy);
			pline_The("missiles bounce off!");
			dmg = 0;
		} //else dmg = d((int)mtmp->m_lev/2 + 1,6);
		stop_occupation();
		break;
        default:
	    {
        cast_spell(mtmp, dmg, spellnum);
		dmg = 0; /* done by the spell casting functions */
		break;
	    }
	}
	if(dmg) mdamageu(mtmp, dmg);
	return(ret);
}

/* monster wizard and cleric spellcasting functions */
/*
   If dmg is zero, then the monster is not casting at you.
   If the monster is intentionally not casting at you, we have previously
   called spell_would_be_useless() and spellnum should always be a valid
   undirected spell.
   If you modify either of these, be sure to change is_undirected_spell()
   and spell_would_be_useless().
 */
STATIC_OVL
void
cast_spell(mtmp, dmg, spellnum)
struct monst *mtmp;
int dmg;
int spellnum;
{
    boolean malediction = (mtmp && (mtmp->iswiz || (mtmp->data->msound == MS_NEMESIS && rn2(2))));
    int zap; /* used for ray spells */
    
    if (dmg == 0 && !is_undirected_spell(spellnum)) {
	impossible("cast directed wizard spell (%d) with dmg=0?", spellnum);
	return;
    }

    switch (spellnum) {
    case DEATH_TOUCH:
	pline("Oh no, %s's using the touch of death!", mtmp ? mhe(mtmp) : "something");
	if (nonliving(youmonst.data) || is_demon(youmonst.data)) {
	    You("seem no deader than before.");
		dmg = 0; //you don't take damage
	} else if (!Antimagic && (!mtmp || rn2(mtmp->m_lev) > 12) && !(u.sealsActive&SEAL_OSE)) {
	    if (Hallucination) {
		You("have an out of body experience.");
	    } else {
		killer_format = KILLED_BY_AN;
		killer = "touch of death";
		dmg = 0; //no additional damage
		done(DIED);
	    }
	} else if(!(u.sealsActive&SEAL_OSE)){
	    if (Antimagic) shieldeff(u.ux, u.uy);
//	    pline("Lucky for you, it didn't work!");
		Your("%s flutters!", body_part(HEART));
		dmg = mtmp ? rnd(mtmp->m_lev) : 10; //you still take damage
	} else{
		dmg = 0;
		shieldeff(u.ux, u.uy);
	}
	stop_occupation();
	break;
    case CLONE_WIZ:
	if (mtmp && mtmp->iswiz && flags.no_of_wizards == 1) {
	    pline("Double Trouble...");
	    clonewiz();
	    dmg = 0;
	} else {
	    if(mtmp) impossible("bad wizard cloning?");
		dmg = 0;
	}
		//else end with no message.
	break;
    case FILTH:
    {
       struct monst *mtmp2;
       long old;
       pline("A cascade of filth pours onto you!");
       if (freehand() && rn2(3)) {
           old = Glib;
           Glib += rn1(20, 9);
           Your("%s %s!", makeplural(body_part(HAND)),
               (old ? "are filthier than ever" : "get slimy"));
       }
       if(haseyes(youmonst.data) && !Blindfolded && mtmp && monsndx(mtmp->data) != PM_DEMOGORGON && rn2(3)) {
           old = u.ucreamed;
           u.ucreamed += rn1(20, 9);
           Your("%s is coated in %sgunk!", body_part(FACE),
                   (old ? "even more " : ""));
           make_blinded(Blinded + (long)u.ucreamed - old, FALSE);
       }
       You("smell putrid! You gag and vomit.");
       /* same effect as "This water gives you bad breath!" */
       for(mtmp2 = fmon; mtmp2; mtmp2 = mtmp2->nmon) {
           if(!DEADMONSTER(mtmp2) && (mtmp2 != mtmp))
           monflee(mtmp2, 0, FALSE, FALSE);
	}
    vomit();
    dmg = rnd(Half_physical_damage ? 5 : 10);
	stop_occupation();
	break;
	}
    case STRANGLE:
    {
       struct obj *otmp;
       if (uamul && (Antimagic || uamul->oartifact || uamul->otyp == AMULET_OF_YENDOR)) {
	    shieldeff(u.ux, u.uy);
            if (!Blind) Your("%s looks vaguely %s for a moment.", xname(uamul),
                               OBJ_DESCR(objects[AMULET_OF_STRANGULATION]));
           else You_feel("a momentary pressure around your %s.",body_part(NECK));
	} else {
           if (uamul) {
               Your("%s warps strangely, then turns %s.", xname(uamul),
                               OBJ_DESCR(objects[AMULET_OF_STRANGULATION]));
               poly_obj(uamul, AMULET_OF_STRANGULATION);
               curse(uamul);
               Amulet_on();
           }
           else {
               if (malediction)
                       verbalize(rn2(2) ? "Thou desirest the amulet? I'll give thee the amulet!" :
                                          "Here is the only amulet you'll need!");
               otmp = mksobj(AMULET_OF_STRANGULATION, FALSE, FALSE);
               curse(otmp);
               (void) addinv(otmp);
               pline("%s appears around your %s!",An(xname(otmp)),body_part(NECK));
               setworn(otmp,W_AMUL);
               Amulet_on();
           }
	}
	dmg = 0;
	stop_occupation();
	break;
    }
     case TURN_TO_STONE:
		if (malediction) /* give a warning to the player */
		   verbalize(rn2(2) ? "I shall make a statue of thee!" :
							  "I condemn thee to eternity unmoving!");
        if (!(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
           if(!Stone_resistance && (!rn2(10) || !(have_lizard() || Free_action)) ){
			You_feel("less limber.");
			Stoned = 5;
		   }else{
			You_feel("a momentary stiffness.");
		   }
        } 
		dmg = 0;
	 stop_occupation();
     break;
     case MAKE_VISIBLE:
       HInvis &= ~INTRINSIC;
       You_feel("paranoid.");
       dmg = 0;
       break;
	 stop_occupation();
     case PLAGUE:
       if (!Sick_resistance) {
          You("are afflicted with disease!");
           make_sick(Sick ? Sick/3L + 1L : (long)rn1(ACURR(A_CON), 20),
                        (char *)0, TRUE, SICK_NONVOMITABLE);
       } else You_feel("slightly infectious.");
       dmg = 0;
       break;
	 stop_occupation();
    case PUNISH:
		if(u.ualign.record <= 1 || !rn2(min(u.ualign.record,20))){
			if (!Punished) {
					punish((struct obj *)0);
				   if (mtmp && is_prince(mtmp->data)) uball->owt += 160;
			} else {
						Your("iron ball gets heavier!");
						if (mtmp && is_prince(mtmp->data)) uball->owt += 240;
						else uball->owt += 160;
			}
		} else Your("sins do not demand punishment.");
		dmg = 0;
		stop_occupation();
	break;
    case EARTHQUAKE:
		pline_The("entire %s is shaking around you!",
               In_endgame(&u.uz) ? "plane" : "dungeon");
        /* Quest nemesis maledictions */
		if (malediction && (!In_endgame(&u.uz) || Is_earthlevel(&u.uz))) {
			if (rn2(2)) verbalize("The earth trembles before my %s!",
                                    rn2(2) ? "power" : "might");
            else verbalize("Open thy maw, mighty earth!");
		}
		mtmp ? 
			do_earthquake(min(((int)mtmp->m_lev - 1) / 6 + 1,12), TRUE, mtmp)
		:	do_earthquake(rnd(5), TRUE, 1); //Fixme: true "not my fault" flag needed.
		aggravate(); /* wake up without scaring */
		dmg = 0;
		stop_occupation();
		doredraw();
	break;
    case ACID_RAIN: /* as seen in the Lethe patch */
		pline("A torrent of burning acid rains down on you!");
		if(uarmh && uarmh->otyp == SEDGE_HAT){
			pline("It runs off the brim of your wide straw hat.");
			dmg = 0;
		} else {
			dmg = d(8, 6);
			if (Acid_resistance) {
				shieldeff(u.ux, u.uy);
				pline("It feels mildly uncomfortable.");
				dmg = 0;
			} else {
				destroy_item(POTION_CLASS, AD_FIRE);
			}
			erode_obj(uwep, TRUE, FALSE);
			erode_obj(uswapwep, TRUE, FALSE);
			erode_armor(&youmonst, TRUE);
			water_damage(invent, FALSE, FALSE, FALSE);
			if (!resists_blnd(&youmonst) && rn2(2)) {
				pline_The("acid gets into your %s!", eyecount(youmonst.data) == 1 ?
						body_part(EYE) : makeplural(body_part(EYE)));
				make_blinded((long)rnd(Acid_resistance ? 10 : 50),FALSE);
				if (!Blind) Your(vision_clears);
			}
		}
		/* TODO: corrode floor objects */
	stop_occupation();
    break;
    case AGGRAVATION:
	You_feel("that monsters are aware of your presence.");
	aggravate();
	dmg = 0;
	stop_occupation();
	break;
    case GEYSER:
	dmg = 0;
	/* this is physical damage, not magical damage */
	if(Wwalking){
		pline("A sudden geyser erupts under your feet!");
		if(ACURR(A_DEX) >= 14){
			You("put the added momentum to good use.");
			if(ACURR(A_DEX) == 25) youmonst.movement += 12;
			else if(ACURR(A_DEX) >= 18) youmonst.movement += 8;
			else youmonst.movement += 6;
		} else if(ACURR(A_DEX) <= 10){
			You("are knocked around by the geyser's force!");
			if(ACURR(A_DEX) <= 3) dmg = d(8, 6);
			else if(ACURR(A_DEX) <= 6) dmg = d(4, 6);
			else if(ACURR(A_DEX) <= 10) dmg = rnd(6);
		}
		if(uarmf && uarmf->otyp == WATER_WALKING_BOOTS) makeknown(uarmf->otyp);
	} else {
		pline("A sudden geyser slams into you from nowhere!");
		dmg = d(8, 6);
		water_damage(invent, FALSE, FALSE, FALSE);
		if (Half_physical_damage) dmg = (dmg + 1) / 2;
	}
	stop_occupation();
	break;
    case FIRE_PILLAR:
	pline("A pillar of fire strikes all around you!");
	if (Fire_resistance) {
	    shieldeff(u.ux, u.uy);
	    dmg = 0;
	} else{
	    dmg = d(8, 6);
		destroy_item(SCROLL_CLASS, AD_FIRE);
		destroy_item(POTION_CLASS, AD_FIRE);
		destroy_item(SPBOOK_CLASS, AD_FIRE);
	}
	if (Half_spell_damage) dmg = (dmg + 1) / 2;
	burn_away_slime();
	(void) burnarmor(&youmonst);
	(void) burn_floor_paper(u.ux, u.uy, TRUE, FALSE);
	stop_occupation();
	break;
    case LIGHTNING:
    if (mtmp && !dmgtype(mtmp->data, AD_CLRC)) {
       zap = AD_ELEC;
       goto ray;
    } else {
	boolean reflects;

	pline("A bolt of lightning strikes down at you from above!");
	reflects = ureflects("It bounces off your %s%s.", "");
	if (reflects || Shock_resistance) {
	    shieldeff(u.ux, u.uy);
	    dmg = 0;
	    if (reflects) break;
	} else {
	    dmg = d(8, 6);
		destroy_item(WAND_CLASS, AD_ELEC);
		destroy_item(RING_CLASS, AD_ELEC);
	}
	if (Half_spell_damage) dmg = (dmg + 1) / 2;
       if (!resists_blnd(&youmonst)) {
           You("are blinded by the flash!");
           make_blinded((long)rnd(100),FALSE);
           if (!Blind) Your(vision_clears);
        }
	   stop_occupation();
       break;
    }
    case SUMMON_ANGEL: /* cleric only */
    {
       struct monst *mtmp2;
	   if(!mtmp) goto psibolt;
	   if(is_alienist(mtmp->data)) goto summon_alien;
	   mtmp2 = mk_roamer(&mons[PM_ANGEL],
               sgn(mtmp->data->maligntyp), mtmp->mux, mtmp->muy, FALSE);
       if (mtmp2) {
           if (canspotmon(mtmp2))
               pline("%s %s!",
                       An(Hallucination ? rndmonnam() : "angel"),
                       Is_astralevel(&u.uz) ? "appears near you" :
                                              "descends from above");
           else
               You("sense the arrival of %s.",
                       an(Hallucination ? rndmonnam() : "hostile angel"));
       }
       dmg = 0;
       break;
    }
    case SUMMON_ALIEN: /* special only */
summon_alien:
    {
       struct monst *mtmp2;
	   int tries = 0;
	   static struct permonst *aliens[] = {&mons[PM_HOOLOOVOO],
									&mons[PM_SHAMBLING_HORROR],
									&mons[PM_STUMBLING_HORROR],
									&mons[PM_WANDERING_HORROR],
									&mons[PM_MASTER_MIND_FLAYER],
									&mons[PM_EDDERKOP],
									&mons[PM_AOA],
									&mons[PM_HUNTING_HORROR],
									&mons[PM_BYAKHEE],
									&mons[PM_UVUUDAUM]};
	   if(!mtmp) goto psibolt;
	   while (!(mtmp2 = makemon(aliens[rn2(SIZE(aliens))], mtmp->mux, mtmp->muy, MM_ADJACENTOK|MM_NOCOUNTBIRTH)) && tries++ < 10);
       if (mtmp2) {
           if (canspotmon(mtmp2))
               pline("The world tears open, and %s steps through!",
                       an(Hallucination ? rndmonnam() : "alien"));
           else
               You("sense the arrival of %s.",
                       an(Hallucination ? rndmonnam() : "alien"));
       }
       dmg = 0;
       break;
    }
    case SUMMON_DEVIL: /* cleric only */
    {
       struct monst *mtmp2;
	   if(!mtmp) goto psibolt;
	   if(is_alienist(mtmp->data)) goto summon_alien;
	   if(!(mtmp->data->maligntyp)) mtmp2 = summon_minion(A_NEUTRAL, FALSE, TRUE);
	   else if((mtmp->data->maligntyp) > 0) mtmp2 = summon_minion(A_LAWFUL, FALSE, TRUE);
	   else mtmp2 = summon_minion(A_CHAOTIC, FALSE, TRUE);
       if (mtmp2) {
           if (canspotmon(mtmp2))
               pline("%s ascends from below!",
                       An(Hallucination ? rndmonnam() : "fiend"));
           else
               You("sense the arrival of %s.",
                       an(Hallucination ? rndmonnam() : "hostile fiend"));
       }
       dmg = 0;
       break;
    }
    case SUMMON_MONS:
    {
	int count;
	if(!mtmp || u.summonMonster) goto psibolt;
	u.summonMonster = TRUE;
	count = nasty(mtmp);	/* summon something nasty */
	if (mtmp->iswiz)
	    verbalize("Destroy the thief, my pet%s!", plur(count));
	else {
	    const char *mappear =
		(count == 1) ? "A monster appears" : "Monsters appear";

	    /* messages not quite right if plural monsters created but
	       only a single monster is seen */
	    if (Invisible && !perceives(mtmp->data) &&
				    (mtmp->mux != u.ux || mtmp->muy != u.uy))
		pline("%s around a spot near you!", mappear);
	    else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
		pline("%s around your displaced image!", mappear);
	    else
		pline("%s from nowhere!", mappear);
	}
	dmg = 0;
	break;
    }
    case INSECTS:
	if(!mtmp) goto psibolt;
      {
	/* Try for insects, and if there are none
	   left, go for (sticks to) snakes.  -3. */
	struct permonst *mdat = mtmp->data;
	struct permonst *pm;
	struct monst *mtmp2 = (struct monst *)0;
	char let;
	boolean success;
	int i,j;
	coord bypos;
	int quan;
	
	if(is_drow(mdat)){
		j = 0;
		do pm = mkclass(S_SPIDER, G_NOHELL|G_HELL);
		while(!is_spider(pm) && j++ < 30);
		let = (pm ? S_SPIDER : S_SNAKE);
	} else{
		pm = mkclass(S_ANT,G_NOHELL|G_HELL);
		let = (pm ? S_ANT : S_SNAKE);
	}
	quan = (mtmp->m_lev < 2) ? 1 : rnd((int)mtmp->m_lev / 2);
	if (quan < 3) quan = 3;
	success = pm ? TRUE : FALSE;
	for (i = 0; i <= quan; i++) {
	    if (!enexto(&bypos, mtmp->mux, mtmp->muy, mtmp->data)) break;
		if(is_drow(mdat)){
			j = 0;
			do pm = mkclass(S_SPIDER, G_NOHELL|G_HELL);
			while(!is_spider(pm) && j++ < 30);
			let = (pm ? S_SPIDER : S_SNAKE);
		} else{
			pm = mkclass(S_ANT,G_NOHELL|G_HELL);
			let = (pm ? S_ANT : S_SNAKE);
		}
	    if (pm != 0 && (mtmp2 = makemon(pm, bypos.x, bypos.y, NO_MM_FLAGS)) != 0) {
			success = TRUE;
			mtmp2->msleeping = mtmp2->mpeaceful = mtmp2->mtame = 0;
				   /* arbitrarily strengthen enemies in astral and sanctum */
				   if (Is_astralevel(&u.uz) || Is_sanctum(&u.uz)) {
					   mtmp2->m_lev += rn1(3,3);
					   mtmp2->mhp = (mtmp2->mhpmax += rn1((int)mtmp->m_lev,20));
				   }
			set_malign(mtmp2);
	    }
	}
	/* Not quite right:
         * -- message doesn't always make sense for unseen caster (particularly
	 *    the first message)
         * -- message assumes plural monsters summoned (non-plural should be
         *    very rare, unlike in nasty())
         * -- message assumes plural monsters seen
         */
	if (!success)
	    pline("%s casts at a clump of sticks, but nothing happens.",
		Monnam(mtmp));
	else if (let == S_SNAKE)
	    pline("%s transforms a clump of sticks into snakes!",
		Monnam(mtmp));
	else if (Invisible && !perceives(mtmp->data) &&
				(mtmp->mux != u.ux || mtmp->muy != u.uy))
	    pline("%s summons %s around a spot near you!",
		Monnam(mtmp), let == S_SPIDER ? "arachnids" : "insects");
	else if (Displaced && (mtmp->mux != u.ux || mtmp->muy != u.uy))
	    pline("%s summons %s around your displaced image!",
		Monnam(mtmp), let == S_SPIDER ? "arachnids" : "insects");
	else
	    pline("%s summons %s!", Monnam(mtmp), let == S_SPIDER ? "arachnids" : "insects");
	dmg = 0;
	break;
      }
     case RAISE_DEAD:
     {
       coord mm;
       register int x, y;
       pline("%s raised the dead!", mtmp ? Monnam(mtmp) : "Something");
       mm.x = mtmp ? mtmp->mx : u.ux;
       mm.y = mtmp ? mtmp->my : u.uy;
       mkundead(&mm, TRUE, NO_MINVENT);
       dmg = 0;
       break;
     }
    case CONE_OF_COLD: /* simulates basic cone of cold */
       zap = AD_COLD;
       goto ray;
    case CURSE_ITEMS:
       You_feel("as if you need some help.");
       rndcurse();
       dmg = 0;
	   stop_occupation();
       break;
     case ARROW_RAIN: /* actually other things as well */
     {
       struct obj *otmp;
       int weap, i, tdmg;
       if (!rn2(3)) weap = ARROW;
       else if (!rn2(3)) weap = DAGGER;
       else if (!rn2(3)) weap = SPEAR;
       else if (!rn2(3)) weap = KNIFE;
       else if (!rn2(3)) weap = JAVELIN;
       else if (!rn2(3)) weap = AXE;
       else {
          weap = rnd_class(ARROW,WORM_TOOTH-1);
           if (weap == TRIDENT) weap = JAVELIN;
       }
       otmp = mksobj(weap, TRUE, FALSE);
       otmp->quan = (long) rn1(7,mtmp ? mtmp->m_lev/2 : rnd(15) );
       otmp->owt = weight(otmp);
       You("are hit from all directions by a %s of %s!",
               rn2(2) ? "shower" : "hail", xname(otmp));
       dmg = 0;
       for (i = 0; i < otmp->quan; i++) {
			tdmg = dmgval(otmp, &youmonst, 0);
			if (tdmg && u.uac < 0) {
				if(u.sealsActive&SEAL_BALAM) tdmg -= min_ints(rnd(-u.uac),rnd(-u.uac));
				else tdmg -= rnd(-u.uac);
				
				if (tdmg < 1) tdmg = 1;
				
			}
			if (Half_physical_damage) tdmg = (tdmg + 1) / 2;
			dmg += tdmg;
       }
        if (!flooreffects(otmp, u.ux, u.uy, "fall")) {
            place_object(otmp, u.ux, u.uy);
            stackobj(otmp);
            newsym(u.ux, u.uy);
        }
	   stop_occupation();
     } break;
     case DROP_BOULDER:
     {
        struct obj *otmp;
        boolean iron = (!rn2(4) ||
#ifdef REINCARNATION
            Is_rogue_level(&u.uz) || 
#endif
            (In_endgame(&u.uz) && !Is_earthlevel(&u.uz)));
        otmp = mksobj(iron ? HEAVY_IRON_BALL : BOULDER,
                        FALSE, FALSE);
        otmp->quan = 1;
        otmp->owt = weight(otmp);
       if (iron) otmp->owt += 160 * rn2(2);
        pline("%s drops out of %s and hits you!", An(xname(otmp)),
                iron ? "nowhere" : the(ceiling(u.ux,u.uy)));
        dmg = dmgval(otmp, &youmonst, 0);
        if (uarmh) {
            if (is_hard(uarmh)) {
                pline("Fortunately, you are wearing a hard helmet.");
                if (dmg > 2) dmg = 2;
            } else if (flags.verbose) {
                Your("%s does not protect you.",
                        xname(uarmh));
            }
        }
        if (!flooreffects(otmp, u.ux, u.uy, "fall")) {
            place_object(otmp, u.ux, u.uy);
            stackobj(otmp);
            newsym(u.ux, u.uy);
        }
       if (Half_physical_damage) dmg = (dmg + 1) / 2;
	   stop_occupation();
        break;
    }
     case DESTRY_WEPN:
     {
        struct obj *otmp = uwep;
        const char *hands;
       hands = bimanual(otmp) ? makeplural(body_part(HAND)) : body_part(HAND);
        if (otmp->oclass == WEAPON_CLASS && !Antimagic && !otmp->oartifact && rn2(4)) {
               /* Quest nemesis maledictions */
			   if(otmp->spe > -7){
					otmp->spe -= 1;
					if(otmp->spe < -7) otmp->spe = -7;
					pline("Your %s has been damaged!", xname(otmp));
			   }
               else if ((rn2(3) && mtmp->data->maligntyp < 0) && !Hallucination) {
                   if (malediction)
                       verbalize("%s, your %s broken!", plname, aobjnam(otmp,"are"));
                   Your("%s to pieces in your %s!", aobjnam(otmp, "shatter"), hands);
                   setuwep((struct obj *)0);
                   useup(otmp);
               } else {
                  Your("%s shape in your %s.", aobjnam(otmp, "change"), hands);
                  poly_obj(otmp, BANANA);
               }
        } else if (otmp && !welded(otmp) && otmp->otyp != LOADSTONE && (!Antimagic || !rn2(4))){
			if(mtmp){
				if(rn2(((int)mtmp->m_lev)) > (acurrstr())) {
					Your("%s knocked out of your %s!",
						aobjnam(otmp,"are"), hands);
					setuwep((struct obj *)0);
					dropx(otmp);
				}
				else Your("%s for a moment.", aobjnam(otmp, "shudder"));
			} else {
				if(acurrstr() < rnd(25) ){
					Your("%s knocked out of your %s!",
						aobjnam(otmp,"are"), hands);
					setuwep((struct obj *)0);
					dropx(otmp);
				}
				else Your("%s for a moment.", aobjnam(otmp, "shudder"));
			}
        } else{
            Your("%s for a moment.", aobjnam(otmp, "shudder"));
        } dmg = 0;
	   stop_occupation();
    } break;
    case DESTRY_ARMR:{
		struct obj *smarm;
		if (Antimagic) {
		   shieldeff(u.ux, u.uy);
		   pline("A field of force surrounds you!");
		} else if ((smarm = some_armor(&youmonst)) == (struct obj *)0) {
		   Your("skin itches.");
		/* Quest nemesis maledictions */
		} else {
			if(smarm->spe <= -1*objects[smarm->otyp].a_ac) destroy_arm(smarm);
			else{
				smarm->spe -= 1;
				if(smarm->spe < -1*objects[smarm->otyp].a_ac) smarm->spe = -1*objects[smarm->otyp].a_ac;
				pline("A field of force surrounds your %s!", xname(smarm));
			}
			if (malediction) {
				if (rn2(2)) verbalize("Thy defenses are useless!");
				else verbalize("Thou might as well be naked!");
			}
		}
		dmg = 0;
		stop_occupation();
	} break;
    case EVIL_EYE:
		if(mtmp){
			struct attack evilEye = {AT_GAZE, AD_LUCK, 1, 1};
			gazemu(mtmp, &evilEye);
		}
		else{
			You_feel("your luck running out.");
			change_luck(-1);
		}
    stop_occupation();
	dmg = 0;
    break;
    case DRAIN_LIFE:  /* simulates player spell "drain life" */
		if(!mtmp || distmin(mtmp->mx,mtmp->my,u.ux,u.uy) < 2){
			if (Drain_resistance) {
				/* note: magic resistance doesn't protect
			 * against "drain life" spell
			 */
				shieldeff(u.ux, u.uy);
				You_feel("momentarily frail.");
			} else {
				Your("body deteriorates!");
				exercise(A_CON, FALSE);
				losexp("life drainage",TRUE,FALSE,FALSE);
			}
			dmg = 0;
			stop_occupation();
		}
        break;
    case WEAKEN_YOU:		/* drain strength */
	if (Fixed_abil) {
		You_feel("momentarily weakened.");
	} else if (Antimagic) {
	    shieldeff(u.ux, u.uy);
		if(rn2(2)){
			You_feel("a bit weaker.");
		    losestr(1);
		    if (u.uhp < 1 && mtmp)
				done_in_by(mtmp) ;
	    }else You_feel("momentarily weakened.");
	} else {
	    You("suddenly feel weaker!");
	    dmg = rnd(2);
	    if (Half_spell_damage) dmg = (dmg + 1) / 2;
			losestr(dmg);
	    if (u.uhp < 1 && mtmp)
			done_in_by(mtmp);
	}
	dmg = 0;
	stop_occupation();
	break;
    case WEAKEN_STATS:           /* drain any stat */
       if (Fixed_abil) {
           You_feel("momentarily weakened.");
       } else if (mtmp ? is_prince(mtmp->data) : rn2(3)) {
          int typ = 0;
          boolean change = FALSE;
          do {
              if (adjattrib(typ, -rnd(2), -1)) change = TRUE;
          } while (++typ < A_MAX);
          if (!change) goto drainhp;
       } else {
          int typ = rn2(A_MAX);
           dmg = rnd(4);
           if (Half_spell_damage) dmg = (dmg + 1) / 2;
           if (dmg < 1) dmg = 1;
          /* try for a random stat */
           if (adjattrib(typ, -dmg, -1)) {
               /* Quest nemesis maledictions */
              if (malediction) verbalize("Thy powers are waning, %s!", plname);
           } else { /* if that fails, drain max HP a bit */
drainhp:
              You_feel("your life force draining away...");
			  dmg*=5;
              if (dmg > 20) dmg = 20;
              if (Upolyd) {
                  u.mh -= dmg;
                  u.mhmax -= dmg;
              } else {
                  u.uhp -= dmg;
                  u.uhpmax -= dmg;
              }
              if (u.uhp < 1 && mtmp) done_in_by(mtmp);
               /* Quest nemesis maledictions */
              if (malediction)
                 verbalize("Verily, thou art no mightier than the merest newt.");
          }
       }
       dmg = 0;
	   stop_occupation();
       break;
    case NIGHTMARE:
		dmg = mtmp ? rnd((int)mtmp->m_lev) : rnd(10);
		You_hear("%s laugh menacingly as the world blurs around you...", mtmp ? mon_nam(mtmp) : "Someone");
		if(Antimagic||Half_spell_damage) dmg = (dmg + 1) / ((Antimagic + Half_spell_damage) * 2);
		make_confused(HConfusion + dmg*10, FALSE);
		make_stunned(HStun + dmg*5, FALSE);
		make_hallucinated(HHallucination + dmg*15, FALSE, 0L);
		dmg = 0;
		stop_occupation();
	break;
    case MAKE_WEB:
       You("become entangled in hundreds of %s!",
               Hallucination ? "two-minute noodles" : "thick cobwebs");
       /* We've already made sure this is safe */
       dotrap(maketrap(u.ux,u.uy,WEB), NOWEBMSG);
       newsym(u.ux,u.uy);
        /* Quest nemesis maledictions */
       if (malediction) {
           if (rn2(ACURR(A_STR)) > 15) verbalize("Thou art dressed like a meal for %s!",
                                        rn2(2) ? "Ungoliant" : "Arachne");
            else verbalize("Struggle all you might, but it will get thee nowhere.");
       }
        dmg = 0;
	   stop_occupation();
       break;
    case DISAPPEAR:            /* makes self invisible */
		if(!mtmp) goto openwounds;
       if (!mtmp->minvis && !mtmp->invis_blkd) {
           if (canseemon(mtmp))
               pline("%s suddenly %s!", Monnam(mtmp),
                     !See_invisible ? "disappears" : "becomes transparent");
           mon_set_minvis(mtmp);
          if (malediction && !canspotmon(mtmp))
               You("hear %s fiendish laughter all around you.", s_suffix(mon_nam(mtmp)));
           dmg = 0;
       } else
           impossible("no reason for monster to cast disappear spell?");
       break;
     case DRAIN_ENERGY: /* stronger than antimagic field */
        if(Antimagic && !Race_if(PM_INCANTIFIER)) {
            shieldeff(u.ux, u.uy);
           You_feel("momentarily lethargic.");
        } else drain_en(rn1(u.ulevel,dmg));
        dmg = 0;
	    stop_occupation();
        break;
    case SLEEP:
       zap = AD_SLEE;
       goto ray;
    case MAGIC_MISSILE:
       zap = AD_MAGM;
ray:
	   if(!mtmp) goto psibolt;
       nomul(0, NULL);
       if(canspotmon(mtmp))
           pline("%s zaps you with a %s!", Monnam(mtmp),
                     flash_types[10+zap-1]);
       buzz(-(10+zap-1),(mtmp->m_lev/2)+1, mtmp->mx, mtmp->my, sgn(tbx), sgn(tby),0,0);
       dmg = 0;
	   stop_occupation();
       break;
    case BLIND_YOU:
	/* note: resists_blnd() doesn't apply here */
	if (!Blinded) {
	    int num_eyes = eyecount(youmonst.data);
	    if (mtmp && dmgtype(mtmp->data, AD_CLRC))
	    pline("Scales cover your %s!",
		  (num_eyes == 1) ?
		  body_part(EYE) : makeplural(body_part(EYE)));
	    else if (Hallucination)
		pline("Oh, bummer!  Everything is dark!  Help!");
	    else pline("A cloud of darkness falls upon you.");
	    make_blinded(Half_spell_damage ? 100L : 200L, FALSE);
	    if (!Blind) Your(vision_clears);
	    dmg = 0;
	} else
	    impossible("no reason for monster to cast blindness spell?");
	stop_occupation();
	break;
    case PARALYZE:
	if (Antimagic || Free_action) {
	    shieldeff(u.ux, u.uy);
	    if (multi >= 0)
		You("stiffen briefly.");
	    nomul(-1, "paralyzed by a monster");
	} else {
	    if (multi >= 0)
		You("are frozen in place!");
	    dmg = rnd(4) + mtmp ? rnd((int)mtmp->m_lev) : rnd(30);
	    if (Half_spell_damage) dmg = (dmg + 1) / 2;
	    nomul(-dmg, "paralyzed by a monster");
	}
	dmg = 0;
	stop_occupation();
	break;
    case CONFUSE_YOU:
	if (Antimagic) {
	    shieldeff(u.ux, u.uy);
	    You_feel("momentarily dizzy.");
	} else {
	    boolean oldprop = !!Confusion;

	    dmg = rnd(10) + mtmp ? rnd((int)mtmp->m_lev) : rnd(30);
	    if (Half_spell_damage) dmg = (dmg + 1) / 2;
	    make_confused(HConfusion + dmg, TRUE);
	    if (Hallucination)
		You_feel("%s!", oldprop ? "trippier" : "trippy");
	    else
		You_feel("%sconfused!", oldprop ? "more " : "");
	}
	dmg = 0;
	stop_occupation();
	break;
    case STUN_YOU:
       if (Antimagic || Free_action) {
           shieldeff(u.ux, u.uy);
           if (!Stunned)
               You_feel("momentarily disoriented.");
           make_stunned(1L, FALSE);
       } else {
           You(Stunned ? "struggle to keep your balance." : "reel...");
           dmg = d(ACURR(A_DEX) < 12 ? 6 : 4, 4);
           if (Half_spell_damage) dmg = (dmg + 1) / 2;
           make_stunned(HStun + dmg, FALSE);
       }
       dmg = 0;
	   stop_occupation();
       break;
    case SUMMON_SPHERE:
    {
       /* For a change, let's not assume the spheres are together. : ) */
       int sphere = (!rn2(3) ? PM_FLAMING_SPHERE : (!rn2(2) ?
                             PM_FREEZING_SPHERE : PM_SHOCKING_SPHERE));
       boolean created = FALSE;
       struct monst *mon;
       if (!(mvitals[sphere].mvflags & G_GONE) &&
		(mon = makemon(&mons[sphere],
			u.ux, u.uy, NO_MINVENT)) != 0)
				if (canspotmon(mon)) created++;
       if (created)
           pline("%s is created!",
                      Hallucination ? rndmonnam() : Amonnam(mon));
       dmg = 0;
	   stop_occupation();
       break;
    }
    case DARKNESS:
       litroom(FALSE, (struct obj *)0);
       dmg = 0;
	   stop_occupation();
       break;
    case HASTE_SELF:
	   if(!mtmp) goto psibolt;
       mon_adjust_speed(mtmp, 1, (struct obj *)0);
       dmg = 0;
       break;
    case CURE_SELF:
	if(!mtmp) goto openwounds;
	if (mtmp->mhp < mtmp->mhpmax) {
	    if (canseemon(mtmp))
		pline("%s looks better.", Monnam(mtmp));
	    /* note: player healing does 6d4; this used to do 1d8; then it did 3d6 */
	    if ((mtmp->mhp += min(d(mtmp->m_lev/3+1,8), 10)) > mtmp->mhpmax)
			mtmp->mhp = mtmp->mhpmax;
	    dmg = 0;
	}
	break;
	case MASS_CURE_CLOSE:{
		int x, y, n;
		struct monst *cmon;
		if(!mtmp){
			x = u.ux;
			y = u.uy;
			n = 8;
		} else {
			x = mtmp->mx;
			x = mtmp->my;
			n = mtmp->m_lev/3+1;
		}
		for(cmon = fmon; cmon; cmon = cmon->nmon){
			if( cmon != mtmp &&
				cmon->mhp<cmon->mhpmax && 
				((mtmp && cmon->mpeaceful == mtmp->mpeaceful) ||
				 (!mtmp && !cmon->mpeaceful)) &&
				dist2(x,y,cmon->mx,cmon->my) <= 4
			){
				if((cmon->mhp += d(n,8)) > cmon->mhpmax)
					cmon->mhp = cmon->mhpmax;
				if (canseemon(cmon))
					pline("%s looks better.", Monnam(mtmp));
			}
		}

	}break;
	case MASS_CURE_FAR:{
		int x, y, n;
		struct monst *cmon;
		if(!mtmp){
			x = u.ux;
			y = u.uy;
			n = 8;
		} else {
			x = mtmp->mux;
			x = mtmp->muy;
			n = mtmp->m_lev/3+1;
		}
		for(cmon = fmon; cmon; cmon = cmon->nmon){
			if( cmon != mtmp &&
				cmon->mhp<cmon->mhpmax && 
				((mtmp && cmon->mpeaceful == mtmp->mpeaceful) ||
				 (!mtmp && !cmon->mpeaceful)) &&
				dist2(x,y,cmon->mx,cmon->my) <= 4
			){
				if((cmon->mhp += d(n,8)) > cmon->mhpmax)
					cmon->mhp = cmon->mhpmax;
				if (canseemon(cmon))
					pline("%s looks better.", Monnam(mtmp));
			}
		}

	}break;
	case RECOVER:
		if(!mtmp) goto openwounds;
		if(!mtmp->perminvis) mtmp->minvis = 0;
		if(mtmp->permspeed == MSLOW) mtmp->permspeed = 0;
		mtmp->mcan = 0;
		mtmp->mcrazed = 0; 
		mtmp->mcansee = 1;
		mtmp->mblinded = 0;
		mtmp->mcanmove = 1;
		mtmp->mfrozen = 0;
		mtmp->msleeping = 0;
		mtmp->mstun = 0;
		mtmp->mconf = 0;
	    if (canseemon(mtmp))
			pline("%s looks recovered.", Monnam(mtmp));
	break;
    case OPEN_WOUNDS:
	openwounds:
	if (Antimagic) {
	    shieldeff(u.ux, u.uy);
	    dmg = (dmg + 1) / 2;
		if( dmg > 30) dmg = 30;
	} else if( dmg > 60) dmg = 60;
	if(dmg < (Upolyd ? u.mh : u.uhp)){
		if (dmg <= 5)
		    Your("skin itches badly for a moment.");
		else if (dmg <= 15)
		    pline("Wounds appear on your body!");
		else if (dmg <= 30)
		    pline("Severe wounds appear on your body!");
		else
		    Your("body is covered with deep, painful wounds!");
	}
	else{
		Your("body is covered with deadly wounds!");
		dmg = max(Upolyd ? u.mh - 5 : u.uhp -5, 0);
	}
	stop_occupation();
	break;
    case PSI_BOLT:
	psibolt:
	/* prior to 3.4.0 Antimagic was setting the damage to 1--this
	   made the spell virtually harmless to players with magic res. */
	if (Antimagic) {
	    shieldeff(u.ux, u.uy);
	    dmg = (dmg + 1) / 2;
		if(dmg > 25) dmg = 25;
	}
	if (dmg <= 5)
	    You("get a slight %sache.", body_part(HEAD));
	else if (dmg <= 10)
	    Your("brain is on fire!");
	else if (dmg <= 20)
	    Your("%s suddenly aches painfully!", body_part(HEAD));
	else{
	    Your("%s suddenly aches very painfully!", body_part(HEAD));
		if(dmg > 50) dmg = 50;
	}
	stop_occupation();
	break;
    default:
       impossible("mcastu: invalid magic spell (%d)", spellnum);
	dmg = 0;
	break;
    }

    if (dmg) mtmp ? mdamageu(mtmp, dmg) : losehp(dmg, "malevolent spell", KILLED_BY_AN);;
}

STATIC_DCL
boolean
is_undirected_spell(spellnum)
int spellnum;
{
	switch (spellnum) {
	case CLONE_WIZ:
	case RAISE_DEAD:
	case SUMMON_ANGEL:
	case SUMMON_ALIEN:
	case SUMMON_DEVIL:
	case SUMMON_MONS:
	case DISAPPEAR:
	case HASTE_SELF:
	case CURE_SELF:
	case INSECTS:
	    return TRUE;
	default:
	    break;
	}
    return FALSE;
}

/* Some spells are useless under some circumstances. */
STATIC_DCL
boolean
spell_would_be_useless(mtmp, spellnum)
struct monst *mtmp;
int spellnum;
{
	int wardAt = ward_at(mtmp->mux,mtmp->muy);
    /* Some spells don't require the player to really be there and can be cast
     * by the monster when you're invisible, yet still shouldn't be cast when
     * the monster doesn't even think you're there.
     * This check isn't quite right because it always uses your real position.
     * We really want something like "if the monster could see mux, muy".
     */
    boolean mcouldseeu = couldsee(mtmp->mx, mtmp->my);

	/*Don't cast at warded spaces*/
	if(onscary(mtmp->mux, mtmp->muy, mtmp) && !is_undirected_spell(spellnum))
		return TRUE;
	
	if(spellnum == DEATH_TOUCH && (wardAt == CIRCLE_OF_ACHERON || wardAt == HEPTAGRAM || wardAt == HEXAGRAM))
		return TRUE;
	
	if(is_drow(mtmp->data)){
		if(!Race_if(PM_DROW)){
			if(sengr_at("Elbereth", mtmp->mux, mtmp->muy)) return TRUE;
		} else {
			if(sengr_at("Lolth", mtmp->mux, mtmp->muy) && (mtmp->m_lev < u.ulevel || u.ualign.record-- > 0)) return TRUE;
		}
	}
       /* only the wiz makes a clone */
	if ((!mtmp->iswiz || flags.no_of_wizards > 1) && spellnum == CLONE_WIZ) return TRUE;
	/* aggravate monsters won't be cast by peaceful monsters */
	if (mtmp->mpeaceful && (spellnum == AGGRAVATION))
	    return TRUE;
       /* don't make "ruler" monsters cast ineffective spells */
	if (is_prince(mtmp->data) && 
		(((spellnum == MAGIC_MISSILE || spellnum == DESTRY_ARMR ||
		spellnum == PARALYZE || spellnum == CONFUSE_YOU ||
		spellnum == STUN_YOU || spellnum == DRAIN_ENERGY) && Antimagic) ||
		((spellnum == MAGIC_MISSILE || spellnum == SLEEP ||
		spellnum == CONE_OF_COLD || spellnum == LIGHTNING) && Reflecting) ||
		((spellnum == STUN_YOU || spellnum == PARALYZE) && Free_action) ||
		(spellnum == FIRE_PILLAR && Fire_resistance) ||
		(spellnum == CONE_OF_COLD && Cold_resistance) ||
		(spellnum == SLEEP && Sleep_resistance) ||
		(spellnum == LIGHTNING && Shock_resistance) ||
		(spellnum == ACID_RAIN && Acid_resistance) ||
		(spellnum == DRAIN_LIFE && Drain_resistance) ||
		(spellnum == PLAGUE && Sick_resistance) ||
		(spellnum == WEAKEN_YOU && Fixed_abil)))
	  return TRUE;
       /* the wiz won't use the following cleric-specific or otherwise weak spells */
       if (mtmp->iswiz && (spellnum == SUMMON_SPHERE || spellnum == DARKNESS ||
		spellnum == PUNISH || spellnum == INSECTS ||
		spellnum == SUMMON_ANGEL || spellnum == DROP_BOULDER))
	  return TRUE;
       /* ray attack when monster isn't lined up */
       if(!lined_up(mtmp) && (spellnum == MAGIC_MISSILE ||
              spellnum == SLEEP || spellnum == CONE_OF_COLD || spellnum == LIGHTNING))
          return TRUE;
       /* drain energy when you have less than 5 */
       if(spellnum == DRAIN_ENERGY && u.uen < 5) return TRUE;
       /* don't cast acid rain if the player is petrifying */
       if (spellnum == ACID_RAIN && Stoned) return TRUE;
       /* don't cast drain life if not in range */
       if (spellnum == DRAIN_LIFE &&  distmin(mtmp->mx,mtmp->my,u.ux,u.uy) < 2) return TRUE;
       /* don't destroy weapon if not wielding anything */
       if (spellnum == DESTRY_WEPN && !uwep) return TRUE;
       /* don't do strangulation if already strangled or there's no room in inventory */
       if (((inv_cnt() == 52 && (!uamul || uamul->oartifact ||
       uamul->otyp == AMULET_OF_YENDOR)) || Strangled) && spellnum == STRANGLE)
           return TRUE;
       /* sleep ray when already asleep */
       if (Sleeping && spellnum == SLEEP)
          return TRUE;
       /* hallucination when already hallucinating */
       if ((u.umonnum == PM_BLACK_LIGHT || u.umonnum == PM_VIOLET_FUNGUS ||
               Hallucination) && spellnum == NIGHTMARE)
           return TRUE;
       /* turn to stone when already/can't be stoned */
       if ((Stoned || Stone_resistance) && spellnum == TURN_TO_STONE)
           return TRUE;
       /* earthquake where the pits won't be effective */
       if ((Underwater || Levitation || Flying || In_endgame(&u.uz) || 
               (u.utrap && u.utraptype == TT_PIT)) && spellnum == EARTHQUAKE)
           return TRUE;
       /* various conditions where webs won't be effective */
       if ((levl[u.ux][u.uy].typ <= IRONBARS || levl[u.ux][u.uy].typ > ICE ||
               t_at(u.ux,u.uy) || amorphous(youmonst.data) ||
               is_whirly(youmonst.data) || flaming(youmonst.data) ||
               unsolid(youmonst.data) || uwep && uwep->oartifact == ART_STING ||
               ACURR(A_STR) >= 18) && spellnum == MAKE_WEB)
           return TRUE;
       /* don't summon spheres when one type is gone */
       if (spellnum == SUMMON_SPHERE &&
               (mvitals[PM_FLAMING_SPHERE].mvflags & G_GONE) ||
               (mvitals[PM_FREEZING_SPHERE].mvflags & G_GONE) ||
               (mvitals[PM_SHOCKING_SPHERE].mvflags & G_GONE))
	    return TRUE;
	/* haste self when already fast */
	if (mtmp->permspeed == MFAST && spellnum == HASTE_SELF)
	    return TRUE;
	/* invisibility when already invisible */
	if ((mtmp->minvis || mtmp->invis_blkd) && spellnum == DISAPPEAR)
	    return TRUE;
	/* peaceful monster won't cast invisibility if you can't see invisible,
	   same as when monsters drink potions of invisibility.  This doesn't
	   really make a lot of sense, but lets the player avoid hitting
	   peaceful monsters by mistake */
	if (mtmp->mpeaceful && !See_invisible && spellnum == DISAPPEAR)
	    return TRUE;
	/* healing when already healed */
	if (mtmp->mhp == mtmp->mhpmax && spellnum == CURE_SELF)
	    return TRUE;
	/* healing when stats are ok */
	if (spellnum == RECOVER && !(mtmp->mcan || mtmp->mcrazed || 
								!mtmp->mcansee || !mtmp->mcanmove || 
								!mtmp->msleeping || mtmp->mstun || 
								mtmp->mconf || mtmp->permspeed == MSLOW))
	    return TRUE;
       /* don't summon anything if it doesn't think you're around
          or the caster is peaceful */
       if ((!mcouldseeu || mtmp->mpeaceful) &&
             (spellnum == SUMMON_MONS || spellnum == INSECTS ||
               spellnum == SUMMON_SPHERE || spellnum == RAISE_DEAD ||
               spellnum == CLONE_WIZ || spellnum == SUMMON_ANGEL || 
			   spellnum == SUMMON_ALIEN || spellnum == SUMMON_DEVIL))
	    return TRUE;
       /* angels can't be summoned in Gehennom */
       if (spellnum == SUMMON_ANGEL && (In_hell(&u.uz)))
	    return TRUE;
       /* make visible spell by spellcaster with see invisible. */
       /* also won't cast it if your invisibility isn't intrinsic. */
	if ((!(HInvis & INTRINSIC) || perceives(mtmp->data))
		   && spellnum == MAKE_VISIBLE)
		return TRUE;
	/* blindness spell on blinded player */
	if (Blinded && spellnum == BLIND_YOU)
	    return TRUE;
    return FALSE;
}
/* return values:
 * 2: target died
 * 1: successful spell
 * 0: unsuccessful spell
 */
int
castmm(mtmp, mdef, mattk)
	register struct monst *mtmp;
	register struct monst *mdef;
	register struct attack *mattk;
{
	int	dmg, ml = mtmp->m_lev;
	int ret;
	int spellnum = 0;

	if(mtmp->data->maligntyp < 0 && u.uz.dnum == law_dnum && on_level(&illregrd_level,&u.uz)) return 0;
	
	if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) && ml) {
	    int cnt = 40;

	    do {
               spellnum = choose_magic_special(mtmp, mattk->adtyp);
				if(!spellnum) return 0; //The monster's spellcasting code aborted the cast.
		/* not trying to attack?  don't allow directed spells */
	    } while(--cnt > 0 &&
		    mspell_would_be_useless(mtmp, mdef, spellnum));
	    if (cnt == 0) return 0;
	}

	/* monster unable to cast spells? */
	if(mtmp->mcan || mtmp->mspec_used || !ml) {
	    if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my))
	    {
                char buf[BUFSZ];
		Sprintf(buf, Monnam(mtmp));

		if (is_undirected_spell(spellnum))
	            pline("%s points all around, then curses.", buf);
		else
	            pline("%s points at %s, then curses.",
		          buf, mon_nam(mdef));

	    } else if ((!(moves % 4) || !rn2(4))) {
	        if (flags.soundok) Norep("You hear a mumbled curse.");
	    }
	    return(0);
	}

	if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
	    mtmp->mspec_used = 10 - mtmp->m_lev;
	    if (mtmp->mspec_used < 2) mtmp->mspec_used = 2;
	}

	if(rn2(ml*10) < (mtmp->mconf ? 100 : 20)) {	/* fumbled attack */
	    if (canseemon(mtmp) && flags.soundok)
		pline_The("air crackles around %s.", mon_nam(mtmp));
	    return(0);
	}
	if (cansee(mtmp->mx, mtmp->my) ||
	    canseemon(mtmp) ||
	    (!is_undirected_spell(spellnum) &&
	     (cansee(mdef->mx, mdef->my) || canseemon(mdef)))) {
            char buf[BUFSZ];
	    Sprintf(buf, " at ");
	    Strcat(buf, mon_nam(mdef));
	    pline("%s casts a spell%s!",
		  canspotmon(mtmp) ? Monnam(mtmp) : "Something",
		  is_undirected_spell(spellnum) ? "" : buf);
	}

	{
		int dmd = 6, dmn = ml/2;
		if (mattk->damd) dmd = (int)(mattk->damd);
		
		if (mattk->damn) dmn+= (int)(mattk->damn);
		else dmn += 1;
		
	    dmg = d(dmn, dmd);
	}
	
	ret = 1;

	switch (mattk->adtyp) {
		case AD_RBRE:
			switch(rnd(3)){
				case 1: goto elec_mm;
				case 2: goto fire_mm;
				case 3: goto cold_mm;
			}
		break;
		case AD_ELEC:
elec_mm:
	        if (canspotmon(mdef))
		    pline("Lightning crackles around %s.", Monnam(mdef));
		if(resists_elec(mdef)) {
			shieldeff(mdef->mx, mdef->my);
	        if (canspotmon(mdef))
			   pline("But %s resists the effects.",
			        mhe(mdef));
			dmg = 0;
		}
		break;
	    case AD_FIRE:
fire_mm:
	        if (canspotmon(mdef))
		    pline("%s is enveloped in flames.", Monnam(mdef));
		if(resists_fire(mdef)) {
			shieldeff(mdef->mx, mdef->my);
	                if (canspotmon(mdef))
			    pline("But %s resists the effects.",
			        mhe(mdef));
			dmg = 0;
		}
		break;
	    case AD_COLD:
cold_mm:
	        if (canspotmon(mdef))
		    pline("%s is covered in frost.", Monnam(mdef));
		if(resists_fire(mdef)) {
			shieldeff(mdef->mx, mdef->my);
	                if (canspotmon(mdef))
			    pline("But %s resists the effects.",
			        mhe(mdef));
			dmg = 0;
		}
		break;
	    case AD_MAGM:
	        if (canspotmon(mdef))
		    pline("%s is hit by a shower of missiles!", Monnam(mdef));
		if(resists_magm(mdef)) {
			shieldeff(mdef->mx, mdef->my);
	                if (canspotmon(mdef))
			    pline_The("missiles bounce off!");
			dmg = 0;
		} //else dmg = d((int)mtmp->m_lev/2 + 1,6);
		break;
	    default:
		{
	    /*aggravation is a special case;*/
		/*it's undirected but should still target the*/
		/*victim so as to aggravate you*/
	        if (is_undirected_spell(spellnum)
		&& (spellnum != AGGRAVATION &&
		      spellnum != SUMMON_MONS))
		    cast_spell(mtmp, dmg, spellnum);
		else
		    ucast_spell(mtmp, mdef, dmg, spellnum);
		dmg = 0; /* done by the spell casting functions */
		break;
	    }
	}
	if (dmg > 0 && mdef->mhp > 0)
	{
	    mdef->mhp -= dmg;
	    if (mdef->mhp < 1) monkilled(mdef, "", mattk->adtyp);
	}
	if (mdef && mdef->mhp < 1) return 2;
	return(ret);
}

STATIC_DCL
boolean
mspell_would_be_useless(mtmp, mdef, spellnum)
struct monst *mtmp;
struct monst *mdef;
int spellnum;
{
	int wardAt = ward_at(mdef->mx, mdef->my);
	
	/*Don't cast at warded spaces*/
	if(onscary(mdef->mx, mdef->my, mtmp) && !is_undirected_spell(spellnum))
		return TRUE;
	
	if(spellnum == DEATH_TOUCH && (wardAt == CIRCLE_OF_ACHERON || wardAt == HEPTAGRAM || wardAt == HEXAGRAM))
		return TRUE;
	
	if(is_drow(mdef->data)){
		if(!is_drow(mtmp->data)){
			if(sengr_at("Elbereth", mdef->mux, mdef->muy)) return TRUE;
		} else {
			if(sengr_at("Lolth", mdef->mux, mdef->muy) && (mdef->m_lev < mtmp->m_lev)) return TRUE;
		}
	}
	
	if ((!mtmp->iswiz || flags.no_of_wizards > 1) && spellnum == CLONE_WIZ) 
		return TRUE;
 	/* haste self when already fast */
	if (mtmp->permspeed == MFAST && spellnum == HASTE_SELF)
	    return TRUE;
	/* invisibility when already invisible */
	if ((mtmp->minvis || mtmp->invis_blkd) && spellnum == DISAPPEAR)
	    return TRUE;
	/* healing when already healed */
	if (mtmp->mhp == mtmp->mhpmax && spellnum == CURE_SELF)
	    return TRUE;
	/* healing when stats are ok */
	if (spellnum == RECOVER && !(mtmp->mcan || mtmp->mcrazed || 
								!mtmp->mcansee || !mtmp->mcanmove || 
								!mtmp->msleeping || mtmp->mstun || 
								mtmp->mconf || mtmp->permspeed == MSLOW))
	/* don't summon monsters if it doesn't think you're around */
#ifndef TAME_SUMMONING
        if (spellnum == SUMMON_MONS)
	    return TRUE;
        if (spellnum == SUMMON_ANGEL)
	    return TRUE;
        if (spellnum == SUMMON_ALIEN)
	    return TRUE;
        if (spellnum == SUMMON_DEVIL)
	    return TRUE;
        if (spellnum == INSECTS)
	    return TRUE;
#endif
	/* blindness spell on blinded player */
	if ((!haseyes(mdef->data) || mdef->mblinded) && spellnum == BLIND_YOU)
	    return TRUE;
    return FALSE;
}


STATIC_DCL
boolean
uspell_would_be_useless(mdef, spellnum)
struct monst *mdef;
int spellnum;
{
	int wardAt = ward_at(mdef->mx, mdef->my);
	
	/*Don't cast at warded spaces*/
	if(onscary(mdef->mx, mdef->my, &youmonst) && !is_undirected_spell(spellnum))
		return TRUE;
	
	if(spellnum == DEATH_TOUCH && (wardAt == CIRCLE_OF_ACHERON || wardAt == HEPTAGRAM || wardAt == HEXAGRAM))
		return TRUE;
	
	/* PC drow can't be warded off this way */
	
	/* aggravate monsters, etc. won't be cast by peaceful monsters */
	if (spellnum == CLONE_WIZ)
	    return TRUE;
	/* haste self when already fast */
	if (Fast && spellnum == HASTE_SELF)
	    return TRUE;
	/* invisibility when already invisible */
	if ((HInvis & INTRINSIC) && spellnum == DISAPPEAR)
	    return TRUE;
	/* healing when already healed */
	if (u.mh == u.mhmax && spellnum == CURE_SELF)
	    return TRUE;
#ifndef TAME_SUMMONING
        if (spellnum == SUMMON_MONS)
	    return TRUE;
#endif
    return FALSE;
}
#endif /* OVLB */
#ifdef OVL0

/* convert 1..10 to 0..9; add 10 for second group (spell casting) */
#define ad_to_typ(k) (10 + (int)k - 1)

int
buzzmu(mtmp, mattk, ml)		/* monster uses spell (ranged) */
	register struct monst *mtmp;
	register struct attack  *mattk;
	int ml;
{
	int dmn = (int)(ml/2);
	int type = mattk->adtyp;
	
	if(mattk->damn) dmn += mattk->damn;
	else dmn += 1;
	
	if(type == AD_RBRE){
		switch(rnd(3)){
			case 1:
				type = AD_ELEC;
			break;
			case 2:
				type = AD_COLD;
			break;
			case 3:
				type = AD_FIRE;
			break;
		}
	}

	
	/* don't print constant stream of curse messages for 'normal'
	   spellcasting monsters at range */
	if (type > AD_SPC2)
	    return(0);
	
	if (mtmp->mcan) {
	    cursetxt(mtmp, FALSE);
	    return(0);
	}
	if(lined_up(mtmp) && rn2(3)) {
	    nomul(0, NULL);
	    if(type && (type < 11)) { /* no cf unsigned >0 */
		if(canseemon(mtmp))
		    pline("%s zaps you with a %s!", Monnam(mtmp),
			  flash_types[ad_to_typ(type)]);
		buzz(-ad_to_typ(type), dmn,
		     mtmp->mx, mtmp->my, sgn(tbx), sgn(tby),0,0);
	    } else impossible("Monster spell %d cast", type-1);
	}
	return(1);
}

int
buzzmm(magr, mdef, mattk, ml)		/* monster uses spell (ranged) */
	struct monst *magr;
	struct monst *mdef;
	struct attack  *mattk;
	int ml;
{
	int dmn = (int)(ml/2);
	int type = mattk->adtyp;
    char buf[BUFSZ];
	
	if(mattk->damn) dmn += mattk->damn;
	else dmn += 1;
	
	if(type == AD_RBRE){
		switch(rnd(3)){
			case 1:
				type = AD_ELEC;
			break;
			case 2:
				type = AD_COLD;
			break;
			case 3:
				type = AD_FIRE;
			break;
		}
	}

	
	/* don't print constant stream of curse messages for 'normal'
	   spellcasting monsters at range */
	if (type > AD_SPC2)
	    return(0);
	
	if (magr->mcan) {
	    cursetxt(magr, FALSE);
	    return(0);
	}
	if(mlined_up(magr, mdef, TRUE) && rn2(3)) {
	    // nomul(0, NULL);
	    if(type && (type < 11)) { /* no cf unsigned >0 */
		Sprintf(buf,"%s zaps ",Monnam(magr));
		Sprintf(buf,"%s%s with a ",buf,mon_nam(mdef));
		Sprintf(buf,"%s%s!",buf,flash_types[ad_to_typ(type)]);
		if(canseemon(magr)) pline(buf);
		buzz(-ad_to_typ(type), dmn,
		     magr->mx, magr->my, sgn(mdef->mx - magr->mx), sgn(mdef->my - magr->my),0,0);
	    } else impossible("Monster spell %d cast", type-1);
	}
	return(1);
}

/* return values:
 * 2: target died
 * 1: successful spell
 * 0: unsuccessful spell
 */
int
castum(mtmp, mattk)
        register struct monst *mtmp; 
	register struct attack *mattk;
{
	int dmg, ml = mons[u.umonnum].mlevel;
	int ret;
	int spellnum = 0;
	boolean directed = FALSE;

	/* Three cases:
	 * -- monster is attacking you.  Search for a useful spell.
	 * -- monster thinks it's attacking you.  Search for a useful spell,
	 *    without checking for undirected.  If the spell found is directed,
	 *    it fails with cursetxt() and loss of mspec_used.
	 * -- monster isn't trying to attack.  Select a spell once.  Don't keep
	 *    searching; if that spell is not useful (or if it's directed),
	 *    return and do something else. 
	 * Since most spells are directed, this means that a monster that isn't
	 * attacking casts spells only a small portion of the time that an
	 * attacking monster does.
	 */
	if ((mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) && ml) {
	    int cnt = 40;
		
	    if (!spellnum) do {
	        spellnum = choose_magic_special(&youmonst, mattk->adtyp);
             if(!spellnum) return 0; //The monster's spellcasting code aborted the cast.
//		/* not trying to attack?  don't allow directed spells */
//		if (!mtmp || mtmp->mhp < 1) {
//		    if (is_undirected_spell(spellnum) && 
//			!uspell_would_be_useless(mtmp, spellnum)) {
//		        break;
//		    }
//		}
	    } while(--cnt > 0 &&
	            ((!mtmp && !is_undirected_spell(spellnum))
		    || uspell_would_be_useless(mtmp, spellnum)));
	    if (cnt == 0) {
	        You("have no spells to cast right now!");
		return 0;
	    }
	}

	if (spellnum == AGGRAVATION && !mtmp)
	{
	    /* choose a random monster on the level */
	    int j = 0, k = 0;
	    for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	        if (!mtmp->mtame && !mtmp->mpeaceful) j++;
	    if (j > 0)
	    {
	        k = rn2(j); 
	        for(mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	            if (!mtmp->mtame && !mtmp->mpeaceful)
		        if (--k < 0) break;
	    }
	}

	directed = mtmp && !is_undirected_spell(spellnum);

	/* unable to cast spells? */
	if(u.uen < ml) {
	    if (directed)
	        You("point at %s, then curse.", mon_nam(mtmp));
	    else
	        You("point all around, then curse.");
	    return(0);
	}

	if (mattk->adtyp == AD_SPEL || mattk->adtyp == AD_CLRC) {
	    u.uen -= ml;
	}

	if(rn2(ml*10) < (Confusion ? 100 : 20)) {	/* fumbled attack */
	    pline_The("air crackles around you.");
	    return(0);
	}

        You("cast a spell%s%s!",
	      directed ? " at " : "",
	      directed ? mon_nam(mtmp) : "");

/*
 *	As these are spells, the damage is related to the level
 *	of the monster casting the spell.
 */
	{
		int dmd = 6, dmn = ml/2;
		if (mattk->damd) dmd = (int)(mattk->damd);
		
		if (mattk->damn) dmn+= (int)(mattk->damn);
		else dmn += 1;
		
	    dmg = d(dmn, dmd);
	}

	ret = 1;

	switch (mattk->adtyp) {
		case AD_RBRE:
			switch(rnd(3)){
				case 1: goto elec_um;
				case 2: goto fire_um;
				case 3: goto cold_um;
			}
		break;
		case AD_ELEC:
elec_um:
	        if (canspotmon(mtmp))
		    pline("Lightning crackles around %s.", Monnam(mtmp));
		if(resists_elec(mtmp)) {
			shieldeff(mtmp->mx, mtmp->my);
	        if (canspotmon(mtmp))
			   pline("But %s resists the effects.",
			        mhe(mtmp));
			dmg = 0;
		}
	    case AD_FIRE:
fire_um:
		pline("%s is enveloped in flames.", Monnam(mtmp));
		if(resists_fire(mtmp)) {
			shieldeff(mtmp->mx, mtmp->my);
			pline("But %s resists the effects.",
			    mhe(mtmp));
			dmg = 0;
		}
		break;
	    case AD_COLD:
cold_um:
		pline("%s is covered in frost.", Monnam(mtmp));
		if(resists_fire(mtmp)) {
			shieldeff(mtmp->mx, mtmp->my);
			pline("But %s resists the effects.",
			    mhe(mtmp));
			dmg = 0;
		}
		break;
	    case AD_MAGM:
		pline("%s is hit by a shower of missiles!", Monnam(mtmp));
		if(resists_magm(mtmp)) {
			shieldeff(mtmp->mx, mtmp->my);
			pline_The("missiles bounce off!");
			dmg = 0;
		}// else dmg = d((int)ml/2 + 1,6);
		break;
	    default:
	    {
		ucast_spell(&youmonst, mtmp, dmg, spellnum);
		dmg = 0; /* done by the spell casting functions */
		break;
	    }
	}

	if (mtmp && dmg > 0 && mtmp->mhp > 0)
	{
	    mtmp->mhp -= dmg;
	    if (mtmp->mhp < 1) killed(mtmp);
	}
	if (mtmp && mtmp->mhp < 1) return 2;

	return(ret);
}

extern NEARDATA const int nasties[];

/* monster wizard and cleric spellcasting functions */
/*
   If dmg is zero, then the monster is not casting at you.
   If the monster is intentionally not casting at you, we have previously
   called spell_would_be_useless() and spellnum should always be a valid
   undirected spell.
   If you modify either of these, be sure to change is_undirected_spell()
   and spell_would_be_useless().
 */
STATIC_OVL
void
ucast_spell(mattk, mtmp, dmg, spellnum)
struct monst *mattk;
struct monst *mtmp;
int dmg;
int spellnum;
{
    boolean resisted = FALSE;
    boolean yours = (mattk == &youmonst);

    if (dmg == 0 && !is_undirected_spell(spellnum)) {
	impossible("cast directed wizard spell (%d) with dmg=0?", spellnum);
	return;
    }

    if (mtmp && mtmp->mhp < 1)
    {
        impossible("monster already dead?");
	return;
    }
	if(spellnum == RAISE_DEAD) spellnum = CURE_SELF;
	else if(spellnum == SUMMON_ANGEL) spellnum = CURE_SELF;
	else if(spellnum == SUMMON_ALIEN) spellnum = CURE_SELF;
	else if(spellnum == SUMMON_DEVIL) spellnum = CURE_SELF;
    switch (spellnum) {
    case DEATH_TOUCH:
	if (!mtmp || mtmp->mhp < 1) {
	    impossible("touch of death with no mtmp");
	    return;
	}
	if (yours){
		if(u.sealsActive&SEAL_BUER) goto uspsibolt; //not your fault.
	    pline("You're using the touch of death!");
	} else if (canseemon(mattk))
	{
	    char buf[BUFSZ];
	    Sprintf(buf, "%s%s", mtmp->mtame ? "Oh no, " : "",
	                         mhe(mattk));
	    if (!mtmp->mtame)
	        *buf = highc(*buf);

	    pline("%s's using the touch of death!", buf);
	}

	if (nonliving(mtmp->data) || is_demon(mtmp->data)) {
	    if (yours || canseemon(mtmp))
	        pline("%s seems no deader than before.", Monnam(mtmp));
	} else if (!(resisted = (resists_magm(mtmp) || resist(mtmp, 0, 0, FALSE))) ||
	           rn2(mons[u.umonnum].mlevel) > 12) {
            mtmp->mhp = -1;
	    if (yours) killed(mtmp);
	    else monkilled(mtmp, "", AD_SPEL);
	    return;
	} else {
	    if (resisted) shieldeff(mtmp->mx, mtmp->my);
	    if (yours || canseemon(mtmp))
	    {
	        if (mtmp->mtame)
		    pline("Lucky for %s, it didn't work!", mon_nam(mtmp));
		else
	            pline("That didn't work...");
            }
	}
	dmg = 0;
	break;
    case SUMMON_SPHERE:
    {
       /* For a change, let's not assume the spheres are together. : ) */
       int sphere = (!rn2(3) ? PM_FLAMING_SPHERE : (!rn2(2) ?
                             PM_FREEZING_SPHERE : PM_SHOCKING_SPHERE));
       boolean created = FALSE;
       struct monst *mpet;
	   if(!yours && !(mattk && mtmp && mattk->mtame != mtmp->mtame)) goto uspsibolt;
	   if(yours || mattk->mtame){
		   if (!(mvitals[sphere].mvflags & G_GONE) &&
			(mpet = makemon(&mons[sphere],
				u.ux, u.uy, MM_EDOG|NO_MINVENT)) != 0){
					if (canspotmon(mpet)) created++;
					initedog(mpet);
			}
		} else {
		   if (!(mvitals[sphere].mvflags & G_GONE) &&
			(mpet = makemon(&mons[sphere],
				u.ux, u.uy, NO_MINVENT)) != 0){
					if (canspotmon(mpet)) created++;
			}
		}
		if (created)
           pline("%s is created!",
                      Hallucination ? rndmonnam() : Amonnam(mpet));
       dmg = 0;
       break;
    }
    case SUMMON_MONS:
    {
	int count = 0;
        register struct monst *mpet;

        if (!rn2(10) && Inhell) {
			if (yours) demonpet();
			else msummon(mattk);
		} else {
			register int i, j;
				int makeindex, tmp = (u.ulevel > 3) ? u.ulevel / 3 : 1;
			coord bypos;

			if (mtmp)
				bypos.x = mtmp->mx, bypos.y = mtmp->my;
			else if (yours)
				bypos.x = u.ux, bypos.y = u.uy;
				else
				bypos.x = mattk->mx, bypos.y = mattk->my;

			for (i = rnd(tmp); i > 0; --i)
				for(j=0; j<20; j++) {

	            do {
	                makeindex = pick_nasty();
	            } while (attacktype(&mons[makeindex], AT_MAGC) &&
	                     monstr[makeindex] >= monstr[u.umonnum]);
                    if (yours &&
		        !enexto(&bypos, u.ux, u.uy, &mons[makeindex]))
		        continue;
                    if (!yours &&
		        !enexto(&bypos, mattk->mx, mattk->my, &mons[makeindex]))
		        continue;
		    if ((mpet = makemon(&mons[makeindex], 
                          bypos.x, bypos.y, 
			  (yours || mattk->mtame) ? MM_EDOG :
											NO_MM_FLAGS)) != 0
				) {
                        mpet->msleeping = 0;
					if (yours || mattk->mtame) initedog(mpet);
					else if (mattk->mpeaceful) mpet->mpeaceful = 1;
			else mpet->mpeaceful = mpet->mtame = 0;
					mpet->mvanishes = yours ? (u.ulevel+rnd(u.ulevel)) : mtmp ? (mtmp->m_lev+rnd(mtmp->m_lev)) : d(2,20);
                        set_malign(mpet);
                    } else /* GENOD? */
                        mpet = makemon((struct permonst *)0,
                                            bypos.x, bypos.y, NO_MM_FLAGS);
                    if(mpet && (u.ualign.type == 0 ||
		        mpet->data->maligntyp == 0 || 
						sgn(mpet->data->maligntyp) == sgn(u.ualign.type)) 
					) {
                        count++;
                        break;
                    }
                }

	    const char *mappear =
		    (count == 1) ? "A monster appears" : "Monsters appear";

	    if (yours || canseemon(mtmp))
	        pline("%s from nowhere!", mappear);
	}

	dmg = 0;
	break;
    }
    case AGGRAVATION:
	if (!mtmp || mtmp->mhp < 1) {
	    You_feel("lonely.");
	    return;
	}
	you_aggravate(mtmp);
	dmg = 0;
	break;
    case CURSE_ITEMS:
	if (!mtmp || mtmp->mhp < 1) {
	    impossible("curse spell with no mtmp");
	    return;
	}
	if (yours || canseemon(mtmp))
	    You_feel("as though %s needs some help.", mon_nam(mtmp));
	mrndcurse(mtmp);
	dmg = 0;
	break;
    case DESTRY_ARMR:
	if (!mtmp || mtmp->mhp < 1) {
	    impossible("destroy spell with no mtmp");
	    return;
	}
	if (resists_magm(mtmp) || resist(mtmp, 0, 0, FALSE)) { 
	    shieldeff(mtmp->mx, mtmp->my);
	    if (yours || canseemon(mtmp))
	        pline("A field of force surrounds %s!",
	               mon_nam(mtmp));
	} else {
            register struct obj *otmp = some_armor(mtmp), *otmp2;

#define oresist_disintegration(obj) \
		(objects[obj->otyp].oc_oprop == DISINT_RES || \
		 obj_resists(obj, 0, 90) || is_quest_artifact(obj))

	    if (otmp &&
	        !oresist_disintegration(otmp))
	    {
	       if (yours || canseemon(mtmp)) pline("%s %s %s!",
		      s_suffix(Monnam(mtmp)),
		      xname(otmp),
		      is_cloak(otmp)  ? "crumbles and turns to dust" :
		      is_shirt(otmp)  ? "crumbles into tiny threads" :
		      is_helmet(otmp) ? "turns to dust and is blown away" :
		      is_gloves(otmp) ? "vanish" :
		      is_boots(otmp)  ? "disintegrate" :
		      is_shield(otmp) ? "crumbles away" :
		                        "turns to dust"
		      );
		obj_extract_self(otmp);
		obfree(otmp, (struct obj *)0);
 	    }
	    else if (yours || canseemon(mtmp))
	        pline("%s looks itchy.", Monnam(mtmp)); 
	}
	dmg = 0;
	break;
    case WEAKEN_YOU:		/* drain strength */
	if (!mtmp || mtmp->mhp < 1) {
	    impossible("weaken spell with no mtmp");
	    return;
	}
	if (resists_magm(mtmp) || resist(mtmp, 0, 0, FALSE)) {
	    shieldeff(mtmp->mx, mtmp->my);
	    if (yours || canseemon(mtmp)) pline("%s looks momentarily weakened.", Monnam(mtmp));
	} else {
		dmg = rnd(4)*5;
	    if (mtmp->mhp < 1)
	    {
	        impossible("trying to drain monster that's already dead");
		return;
	    }
	    if (yours || canseemon(mtmp))
	        pline("%s suddenly seems weaker!", Monnam(mtmp));
            /* monsters don't have strength, so drain max hp instead */
	    mtmp->mhpmax -= dmg;
	    if ((mtmp->mhp -= dmg) <= 0) {
	        if (yours) killed(mtmp);
		else monkilled(mtmp, "", AD_SPEL);
            }
	}
	dmg = 0;
	break;
    case DISAPPEAR:		/* makes self invisible */
        if (!yours) {
	    impossible("ucast disappear but not yours?");
	    return;
	}
	if (!(HInvis & INTRINSIC)) {
	    HInvis |= FROMOUTSIDE;
	    if (!Blind && !BInvis) self_invis_message();
	    dmg = 0;
	} else
	    impossible("no reason for player to cast disappear spell?");
	break;
    case STUN_YOU:
	if (!mtmp || mtmp->mhp < 1) {
	    impossible("stun spell with no mtmp");
	    return;
	}
	if (resists_magm(mtmp) || resist(mtmp, 0, 0, FALSE)) { 
	    shieldeff(mtmp->mx, mtmp->my);
	    if (yours || canseemon(mtmp))
	        pline("%s seems momentarily disoriented.", Monnam(mtmp));
	} else {
	    
	    if (yours || canseemon(mtmp)) {
	        if (mtmp->mstun)
	            pline("%s struggles to keep %s balance.",
	 	          Monnam(mtmp), mhis(mtmp));
                else
	            pline("%s reels...", Monnam(mtmp));
	    }
	    mtmp->mstun = 1;
	}
	dmg = 0;
	break;
    case HASTE_SELF:
    if (!yours) {
	    impossible("ucast haste but not yours?");
	    return;
	}
        if (!(HFast & INTRINSIC))
	    You("are suddenly moving faster.");
	HFast |= INTRINSIC;
	dmg = 0;
	break;
    case CURE_SELF:
        if (!yours) impossible("ucast healing but not yours?");
		else if (u.mh < u.mhmax) {
			You("feel better.");
			if ((u.mh += d(3,6)) > u.mhmax)
			u.mh = u.mhmax;
			flags.botl = 1;
		}
		dmg = 0;
	break;
    case PSI_BOLT:
	default:
uspsibolt:
	if (!mtmp || mtmp->mhp < 1) {
	    impossible("psibolt spell with no mtmp");
	    return;
	}
	if (resists_magm(mtmp) || resist(mtmp, 0, 0, FALSE)) { 
	    shieldeff(mtmp->mx, mtmp->my);
	    dmg = (dmg + 1) / 2;
	}
	if (canseemon(mtmp))
	    pline("%s winces%s", Monnam(mtmp), (dmg <= 5) ? "." : "!");
	break;
    case GEYSER:{
		struct obj* boots;
		if (!mtmp || mtmp->mhp < 1) {
			impossible("geyser spell with no mtmp");
			return;
		}
		dmg = 0;
		boots = which_armor(mtmp, W_ARMF);
		if(boots && boots->otyp == WATER_WALKING_BOOTS){
			if (yours || canseemon(mtmp)){
				pline("A sudden geyser erupts under %s's feet!", mon_nam(mtmp));
				if(mtmp->data->mmove >= 14) pline("%s puts the added monmentum to good use!", Monnam(mtmp));
				else if(mtmp->data->mmove <=10) pline("%s is knocked around by the geyser's force!", Monnam(mtmp));
				if(canseemon(mtmp)) makeknown(boots->otyp);
			}
			if(mtmp->data->mmove >= 25) mtmp->movement += 12;
			else if(mtmp->data->mmove >= 18) mtmp->movement += 8;
			else if(mtmp->data->mmove >= 14) mtmp->movement += 6;
			else if(mtmp->data->mmove <= 3) dmg = d(8, 6);
			else if(mtmp->data->mmove <= 6) dmg = d(4, 6);
			else if(mtmp->data->mmove <=10) dmg = rnd(6);
		} else {
			if (yours || canseemon(mtmp))
				pline("A sudden geyser slams into %s from nowhere!", mon_nam(mtmp));
			/* this is physical damage, not magical damage */
			dmg = d(8, 6);
			water_damage(mtmp->minvent, FALSE, FALSE, FALSE);
		}
	}break;
    case FIRE_PILLAR:
	if (!mtmp || mtmp->mhp < 1) {
	    impossible("firepillar spell with no mtmp");
	    return;
	}
	if (yours || canseemon(mtmp))
	    pline("A pillar of fire strikes all around %s!", mon_nam(mtmp));
	if (resists_fire(mtmp)) {
	    shieldeff(mtmp->mx, mtmp->my);
	    dmg = 0;
	} else
	    dmg = d(8, 6);
	(void) burnarmor(mtmp);
	destroy_mitem(mtmp, SCROLL_CLASS, AD_FIRE);
	destroy_mitem(mtmp, POTION_CLASS, AD_FIRE);
	destroy_mitem(mtmp, SPBOOK_CLASS, AD_FIRE);
	(void) burn_floor_paper(mtmp->mx, mtmp->my, TRUE, FALSE);
	break;
    case LIGHTNING:
    {
	boolean reflects;
	if (!mtmp || mtmp->mhp < 1) {
	    impossible("lightning spell with no mtmp");
	    return;
	}

	if (yours || canseemon(mtmp))
	    pline("A bolt of lightning strikes down at %s from above!",
	          mon_nam(mtmp));
	reflects = mon_reflects(mtmp, "It bounces off %s %s.");
	if (reflects || resists_elec(mtmp)) {
	    shieldeff(u.ux, u.uy);
	    dmg = 0;
	    if (reflects)
		break;
	} else
	    dmg = d(8, 6);
	destroy_mitem(mtmp, WAND_CLASS, AD_ELEC);
	destroy_mitem(mtmp, RING_CLASS, AD_ELEC);
	break;
    }
    case INSECTS:
	if (!mtmp || mtmp->mhp < 1) {
	    impossible("insect spell with no target");
	    return;
	}
      {
	/* Try for insects, and if there are none
	   left, go for (sticks to) snakes.  -3. */
	struct permonst *pm;
	struct monst *mtmp2 = (struct monst *)0;
	char let;
	boolean success;
	int i,j;
	coord bypos;
	int quan;
    
	if((yours && Race_if(PM_DROW)) || 
		(!yours && is_drow((mattk->data)) )
	){ /*summoning is determined by your actual race*/\
		j = 0;
		do pm = mkclass(S_SPIDER, G_NOHELL|G_HELL);
		while(!is_spider(pm) && j++ < 30);
		let = (pm ? S_SPIDER : S_SNAKE);
	} else {
		pm = mkclass(S_ANT,G_NOHELL|G_HELL);
		let = (pm ? S_ANT : S_SNAKE);
	}
	
	if(yours) quan = (mons[u.umonnum].mlevel < 2) ? 1 : 
	       rnd(mons[u.umonnum].mlevel / 2);
	else quan = (mattk->m_lev < 2) ? 1 : rnd((int)mattk->m_lev / 2);
	if (quan < 3) quan = 3;
	success = pm ? TRUE : FALSE;
	for (i = 0; i <= quan; i++) {
	    if (!enexto(&bypos, mtmp->mx, mtmp->my, mtmp->data)) break;
		if((yours && Race_if(PM_DROW)) || 
			(!yours && is_drow((mattk->data)) )
		){ /*summoning is determined by your actual race*/\
			j = 0;
			do pm = mkclass(S_SPIDER, G_NOHELL|G_HELL);
			while(!is_spider(pm) && j++ < 30);
			let = (pm ? S_SPIDER : S_SNAKE);
		} else {
			pm = mkclass(S_ANT,G_NOHELL|G_HELL);
			let = (pm ? S_ANT : S_SNAKE);
		}
	    if (pm && (mtmp2 = makemon(pm, bypos.x, bypos.y, NO_MM_FLAGS)) != 0) {
		success = TRUE;
		mtmp2->msleeping = 0;
		if (yours || mattk->mtame)
		    (void) tamedog(mtmp2, (struct obj *)0);
		else if (mattk->mpeaceful)
		    mattk->mpeaceful = 1;
		else mattk->mpeaceful = 0;

		set_malign(mtmp2);
	    }
	}

        if (yours)
	{
	    if (!success)
	        You("cast at a clump of sticks, but nothing happens.");
	    else if (let == S_SNAKE)
	        You("transforms a clump of sticks into snakes!");
	    else
	        You("summon %s!",let == S_SPIDER ? "arachnids" : "insects");
        } else if (canseemon(mtmp)) {
	    if (!success)
	        pline("%s casts at a clump of sticks, but nothing happens.",
		      Monnam(mattk));
	    else if (let == S_SNAKE)
	        pline("%s transforms a clump of sticks into snakes!",
		      Monnam(mattk));
	    else
	        pline("%s summons %s!", Monnam(mattk),let == S_SPIDER ? "arachnids" : "insects");
	}
	dmg = 0;
	break;
      }
    case BLIND_YOU:
        if (!mtmp || mtmp->mhp < 1) {
	    impossible("blindness spell with no mtmp");
	    return;
	}
	/* note: resists_blnd() doesn't apply here */
	if (!mtmp->mblinded &&
	    haseyes(mtmp->data)) {
	    if (!resists_blnd(mtmp)) {
	        int num_eyes = eyecount(mtmp->data);
	        pline("Scales cover %s %s!",
	          s_suffix(mon_nam(mtmp)),
		  (num_eyes == 1) ? "eye" : "eyes");

		  mtmp->mblinded = 127;
	    }
	    dmg = 0;

	} else
	    impossible("no reason for monster to cast blindness spell?");
	break;
    case PARALYZE:
        if (!mtmp || mtmp->mhp < 1) {
	    impossible("paralysis spell with no mtmp");
	    return;
	}
	if (resists_magm(mtmp) || resist(mtmp, 0, 0, FALSE)) { 
	    shieldeff(mtmp->mx, mtmp->my);
	    if (yours || canseemon(mtmp))
	        pline("%s stiffens briefly.", Monnam(mtmp));
	} else {
	    if (yours || canseemon(mtmp))
	        pline("%s is frozen in place!", Monnam(mtmp));
	    dmg = 4 + mons[u.umonnum].mlevel;
	    mtmp->mcanmove = 0;
	    mtmp->mfrozen = dmg;
	}
	dmg = 0;
	break;
    case CONFUSE_YOU:
        if (!mtmp || mtmp->mhp < 1) {
	    impossible("confusion spell with no mtmp");
	    return;
	}
	if (resists_magm(mtmp) || resist(mtmp, 0, 0, FALSE)) { 
	    shieldeff(mtmp->mx, mtmp->my);
	    if (yours || canseemon(mtmp))
	        pline("%s seems momentarily dizzy.", Monnam(mtmp));
	} else {
	    if (yours || canseemon(mtmp))
	        pline("%s seems %sconfused!", Monnam(mtmp),
	              mtmp->mconf ? "more " : "");
	    mtmp->mconf = 1;
	}
	dmg = 0;
	break;
    case OPEN_WOUNDS:
        if (!mtmp || mtmp->mhp < 1) {
	    impossible("wound spell with no mtmp");
	    return;
	}
	if (resists_magm(mtmp) || resist(mtmp, 0, 0, FALSE)) { 
	    shieldeff(mtmp->mx, mtmp->my);
	    dmg = (dmg + 1) / 2;
	}
	/* not canseemon; if you can't see it you don't know it was wounded */
	if (yours)
	{
	    if (dmg <= 5)
	        pline("%s looks itchy!", Monnam(mtmp));
	    else if (dmg <= 10)
	        pline("Wounds appear on %s!", mon_nam(mtmp));
	    else if (dmg <= 20)
	        pline("Severe wounds appear on %s!", mon_nam(mtmp));
	    else
	        pline("%s is covered in wounds!", Monnam(mtmp));
	}
	break;
    }

    if (dmg > 0 && mtmp->mhp > 0)
    {
        mtmp->mhp -= dmg;
        if (mtmp->mhp < 1) {
	    if (yours) killed(mtmp);
	    else monkilled(mtmp, "", AD_CLRC);
	}
    }
}

#endif /* OVL0 */

/*mcastu.c*/
