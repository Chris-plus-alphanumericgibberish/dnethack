/*	SCCS Id: @(#)quest.c	3.4	2000/05/05	*/
/*	Copyright 1991, M. Stephenson		  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"
#include "artilist.h"

/*  quest dungeon branch routines. */

#include "quest.h"
#include "qtext.h"

#define Not_firsttime	(on_level(&u.uz0, &u.uz))
#define Qstat(x)	(quest_status.x)

STATIC_DCL void NDECL(on_start);
STATIC_DCL void NDECL(on_locate);
STATIC_DCL void NDECL(on_goal);
STATIC_DCL boolean NDECL(not_capable);
STATIC_DCL int FDECL(is_pure, (BOOLEAN_P));
STATIC_DCL void FDECL(expulsion, (BOOLEAN_P));
STATIC_DCL void NDECL(chat_with_leader);
STATIC_DCL void NDECL(chat_with_nemesis);
STATIC_DCL void NDECL(chat_with_guardian);
STATIC_DCL void FDECL(prisoner_speaks, (struct monst *));


STATIC_OVL void
on_start()
{
  if(!Qstat(first_start)) {
    qt_pager(QT_FIRSTTIME + (flags.stag ? QT_TURNEDSTAG : 0));
    Qstat(first_start) = TRUE;
	livelog_write_string("has already received their quest via sticky-note");
  } else if((u.uz0.dnum != u.uz.dnum) || (u.uz0.dlevel < u.uz.dlevel)) {
    if(Qstat(not_ready) <= 2) qt_pager(QT_NEXTTIME + (flags.stag ? QT_TURNEDSTAG : 0));
    else	qt_pager(QT_OTHERTIME + (flags.stag ? QT_TURNEDSTAG : 0));
  }
}

STATIC_OVL void
on_locate()
{
  if(!Qstat(first_locate)) {
    qt_pager(QT_FIRSTLOCATE + (flags.stag ? QT_TURNEDSTAG : 0));
    Qstat(first_locate) = TRUE;
  } else if(u.uz0.dlevel < u.uz.dlevel && !Qstat(killed_nemesis))
	qt_pager(QT_NEXTLOCATE + (flags.stag ? QT_TURNEDSTAG : 0));
}

STATIC_OVL void
on_goal()
{
  if (Qstat(killed_nemesis)) {
    return;
  } else if (!Qstat(made_goal)) {
    qt_pager(QT_FIRSTGOAL + (flags.stag ? QT_TURNEDSTAG : 0));
    Qstat(made_goal) = 1;
  } else {
    qt_pager(QT_NEXTGOAL + (flags.stag ? QT_TURNEDSTAG : 0));
    if(Qstat(made_goal) < 7) Qstat(made_goal)++;
  }
}

void
onquest()
{
	if(u.uevent.qcompleted || Not_firsttime) return;
	if(!Is_special(&u.uz)) return;

	if(Is_qstart(&u.uz)) on_start();
	else if(Is_qlocate(&u.uz) && u.uz.dlevel > u.uz0.dlevel) on_locate();
	else if(Is_nemesis(&u.uz)) on_goal();
	return;
}

void
nemdead()
{
	if(!Qstat(killed_nemesis)) {
	    Qstat(killed_nemesis) = TRUE;
		if(Role_if(PM_EXILE)) u.uevent.qcompleted = TRUE;
	    qt_pager(QT_KILLEDNEM + (flags.stag ? QT_TURNEDSTAG : 0));
	}
}

void
artitouch()
{
	if(!Qstat(touched_artifact)) {
	    Qstat(touched_artifact) = TRUE;
	    qt_pager(QT_GOTIT + (flags.stag ? QT_TURNEDSTAG : 0));
	    exercise(A_WIS, TRUE);
	}
}

/* external hook for do.c (level change check) */
boolean
ok_to_quest()
{
	return((boolean)((Qstat(got_quest) || Qstat(got_thanks))));
}

STATIC_OVL boolean
not_capable()
{
	if(Role_if(PM_BARD)){
		struct monst *petm;
		int maxlev = 0;
		int cumlev = 0;
		for(petm = fmon; petm; petm = petm->nmon){
			if(petm->mtame){
				maxlev = max(maxlev, petm->m_lev);
				cumlev += petm->m_lev/2;
			}
		}
		cumlev += maxlev/2 + maxlev%2;
		cumlev += u.ulevel/2;
		
		if(cumlev >= MIN_QUEST_LEVEL) return FALSE;
	}
	
	if(Pantheon_if(PM_GNOME)) return((boolean)(u.ulevel < GNOMISH_MIN_QUEST_LEVEL));
	else if(Race_if(PM_HALF_DRAGON) && Role_if(PM_NOBLEMAN) && flags.initgend) return FALSE;
	else return((boolean)(u.ulevel < MIN_QUEST_LEVEL));
}

STATIC_OVL int
is_pure(talk)
boolean talk;
{
    int purity;
    aligntyp original_alignment = u.ualignbase[A_ORIGINAL];
	int racemod = Race_if(PM_VAMPIRE) ? 5 : 0;

#ifdef WIZARD
    if (wizard && talk) {
	if (u.ualign.type != original_alignment) {
	    You("are currently %s instead of %s.",
		align_str(u.ualign.type), align_str(original_alignment));
	} else if (u.ualignbase[A_CURRENT] != original_alignment) {
	    You("have converted.");
	} else if (u.ualign.record < (MIN_QUEST_ALIGN - racemod)) {
	    You("are currently %d and require %d.",
		u.ualign.record, MIN_QUEST_ALIGN-racemod);
	    if (yn_function("adjust?", (char *)0, 'y') == 'y')
		u.ualign.record = MIN_QUEST_ALIGN-racemod;
	}
    }
#endif
	if(Race_if(PM_ELF) && (Role_if(PM_RANGER) || Role_if(PM_WIZARD) || Role_if(PM_PRIEST) || Role_if(PM_NOBLEMAN))){
		purity = (u.ualign.record >= (MIN_QUEST_ALIGN-racemod))  ?  1 : 0;
	} else if(Role_if(PM_ANACHRONONAUT)){
		purity = (u.ualign.record >= (MIN_QUEST_ALIGN-racemod) &&
			  u.ualign.type == u.ualignbase[A_CURRENT] &&
			  u.ualignbase[A_CURRENT] != A_LAWFUL) ?  1 :
			 (u.ualignbase[A_CURRENT] == A_LAWFUL) ? -1 : 0;
	} else {
		purity = (u.ualign.record >= (MIN_QUEST_ALIGN-racemod) &&
			  u.ualign.type == original_alignment &&
			  u.ualignbase[A_CURRENT] == original_alignment) ?  1 :
			 (u.ualignbase[A_CURRENT] != original_alignment) ? -1 : 0;
	}
    return purity;
}

/*
 * Expell the player to the stairs on the parent of the quest dungeon.
 *
 * This assumes that the hero is currently _in_ the quest dungeon and that
 * there is a single branch to and from it.
 */
STATIC_OVL void
expulsion(seal)
boolean seal;
{
    branch *br;
    d_level *dest;
    struct trap *t;
    int portal_flag;
	
	if(Race_if(PM_GNOME) && Role_if(PM_RANGER) && !seal){ //If sealing dump out quest portal and seal it.
		dest = &minetown_level;
		portal_flag = 0;
	} else {
		br = dungeon_branch("The Quest");
		dest = (br->end1.dnum == u.uz.dnum) ? &br->end2 : &br->end1;
		portal_flag = u.uevent.qexpelled ? 0 :	/* returned via artifact? */
			  !seal ? 1 : -1;
	}
    schedule_goto(dest, FALSE, FALSE, portal_flag, (char *)0, (char *)0);
    if (seal) {	/* remove the portal to the quest - sealing it off */
		int reexpelled = u.uevent.qexpelled;
		u.uevent.qexpelled = 1;
		remdun_mapseen(quest_dnum);
		/* Delete the near portal now; the far (main dungeon side)
		   portal will be deleted as part of arrival on that level.
		   If monster movement is in progress, any who haven't moved
		   yet will now miss out on a chance to wander through it... */
		for (t = ftrap; t; t = t->ntrap)
			if (t->ttyp == MAGIC_PORTAL) break;
		if (t) deltrap(t);	/* (display might be briefly out of sync) */
		else if (!reexpelled) impossible("quest portal already gone?");
    }
}

/* Either you've returned to quest leader while carrying the quest
   artifact or you've just thrown it to/at him or her.  If quest
   completion text hasn't been given yet, give it now.  Otherwise
   give another message about the character keeping the artifact
   and using the magic portal to return to the dungeon. */
void
finish_quest(obj)
struct obj *obj;	/* quest artifact; possibly null if carrying Amulet */
{
	struct obj *otmp;

	if (u.uhave.amulet) {	/* unlikely but not impossible */
	    qt_pager(QT_HASAMULET + (flags.stag ? QT_TURNEDSTAG : 0));
	    /* leader IDs the real amulet but ignores any fakes */
	    if ((otmp = carrying(AMULET_OF_YENDOR)) != 0)
			fully_identify_obj(otmp);
		if(Race_if(PM_DROW) && (Role_if(PM_PRIEST) || Role_if(PM_ROGUE) || Role_if(PM_RANGER) || Role_if(PM_WIZARD)) && !flags.stag){
			struct monst *tm;
			flags.leader_backstab = TRUE;
			mons[urole.ldrnum].msound = MS_CUSS;
			mons[urole.ldrnum].mflagst &= ~(MT_PEACEFUL);
			mons[urole.ldrnum].mflagst |= (MT_WANTSARTI|MT_WAITFORU|MT_STALK|MT_HOSTILE);
			mons[urole.ldrnum].mflagsg |= MG_NASTY;
			
			mons[urole.guardnum].msound = MS_CUSS;
			mons[urole.guardnum].mflagst &= ~(MT_PEACEFUL);
			mons[urole.guardnum].mflagst |= (MT_WAITFORU|MT_STALK|MT_HOSTILE);
			mons[urole.guardnum].mflagsg |= MG_NASTY;
			for(tm = fmon; tm; tm = tm->nmon){
				if(tm->mfaction != EILISTRAEE_SYMBOL && 
					tm->mfaction != XAXOX && tm->mfaction != EDDER_SYMBOL && 
					is_drow(tm->data) && !tm->mtame 
				){
					tm->housealert = 1;
					tm->mpeaceful = 0;
					tm->mtraitor = 1;
					tm->mstrategy &= ~STRAT_WAITMASK;
					set_malign(tm);
				}
			}
		}
	} else {
	    qt_pager((!Qstat(got_thanks) ? QT_OFFEREDIT : is_primary_quest_artifact(obj) ? QT_OFFEREDIT2 : QT_OFFERART2) + (flags.stag ? QT_TURNEDSTAG : 0));
	    /* should have obtained bell during quest;
	       if not, suggest returning for it now */
	    if ((otmp = carrying(BELL_OF_OPENING)) == 0 && !Role_if(PM_ANACHRONONAUT))
			com_pager(5);
	}
	Qstat(got_thanks) = TRUE;

	if (obj) {
	    if(!Role_if(PM_EXILE)) u.uevent.qcompleted = 1;	/* you did it! */
	    /* behave as if leader imparts sufficient info about the
	       quest artifact */
	    fully_identify_obj(obj);
	    update_inventory();
	}
}

STATIC_OVL void
chat_with_leader()
{
/*	Rule 0:	Cheater checks.		No -C_ANG			*/
	// if(u.uhave.questart && !Qstat(met_nemesis))
	    // Qstat(cheater) = TRUE;

/*	It is possible for you to get the amulet without completing
 *	the quest.  If so, try to induce the player to quest.
 */
	if(Qstat(got_thanks)) {
/*	Rule 1:	You've gone back with/without the amulet.	*/
	    if(u.uhave.amulet)	finish_quest((struct obj *)0);

/*	Rule 2:	You've gone back before going for the amulet.	*/
	    else		qt_pager(QT_POSTHANKS + (flags.stag ? QT_TURNEDSTAG : 0));
	}

/*	Rule 3: You've got the artifact and are back to return it. */
	  else if(u.uhave.questart && (!Role_if(PM_EXILE) || quest_status.killed_nemesis)) {
	    struct obj *otmp;

	    for (otmp = invent; otmp; otmp = otmp->nobj)
		if (is_quest_artifact(otmp)) break;

	    finish_quest(otmp);

/*	Rule 4: You haven't got the artifact yet.	*/
	} else if(Qstat(got_quest)) {
	    qt_pager(rn1(10, QT_ENCOURAGE + (flags.stag ? QT_TURNEDSTAG : 0)));

/*	Rule 5: You aren't yet acceptable - or are you? */
	} else {
	  if(!Qstat(met_leader)) {
	    qt_pager(QT_FIRSTLEADER + (flags.stag ? QT_TURNEDSTAG : 0));
	    Qstat(met_leader) = TRUE;
	    Qstat(not_ready) = 0;
	  } else if(Qstat(not_ready) < 3) qt_pager(QT_NEXTLEADER + (flags.stag ? QT_TURNEDSTAG : 0));
	   else qt_pager(QT_OTHERLEADER + (flags.stag ? QT_TURNEDSTAG : 0));
	  /* the quest leader might have passed through the portal into
	     the regular dungeon; none of the remaining make sense there */
	  if (!on_level(&u.uz, &qstart_level) && !(Race_if(PM_DROW) && (Role_if(PM_PRIEST) || Role_if(PM_ROGUE) || Role_if(PM_RANGER) || Role_if(PM_WIZARD)))) return;

	  if(not_capable() && !flags.stag) {
	    qt_pager(QT_BADLEVEL + (flags.stag ? QT_TURNEDSTAG : 0));
	    exercise(A_WIS, TRUE);
	    expulsion(FALSE);
	  } else if(is_pure(TRUE) < 0 && !flags.stag) {
	    com_pager(QT_BANISHED + (flags.stag ? QT_TURNEDSTAG : 0));
	    expulsion(TRUE);
	  } else if(is_pure(TRUE) == 0 && !flags.stag) {
	    qt_pager(QT_BADALIGN + (flags.stag ? QT_TURNEDSTAG : 0));
	    if(Qstat(not_ready) == MAX_QUEST_TRIES) {
	      qt_pager(QT_LASTLEADER + (flags.stag ? QT_TURNEDSTAG : 0));
	      expulsion(TRUE);
	    } else {
	      if(!Role_if(PM_EXILE)) Qstat(not_ready)++;
	      exercise(A_WIS, TRUE);
	      expulsion(FALSE);
	    }
	  } else {	/* You are worthy! */
	    qt_pager(QT_ASSIGNQUEST + (flags.stag ? QT_TURNEDSTAG : 0));
	    exercise(A_WIS, TRUE);
	    Qstat(got_quest) = TRUE;
		livelog_write_string("was given their Quest");
		if(Role_if(PM_EXILE)){
			bindspirit(DAHLVER_NAR);
			u.sealTimeout[DAHLVER_NAR-FIRST_SEAL] = moves + 5000;
		} else if(Role_if(PM_ANACHRONONAUT)){
			flags.questprogress = 1;
			urole.lgod = getAnachrononautLgod();
		} else if(Role_if(PM_CONVICT)){
			struct obj *obj;
			obj = mksobj(HEAVY_IRON_BALL, TRUE, FALSE);
			obj = oname(obj, artiname(ART_IRON_BALL_OF_LEVITATION));
			obj->oerodeproof = TRUE;
			obj->blessed = TRUE;
			obj->cursed = FALSE;
			pline("\"I had enchanted my iron ball for a later part of the escape, take it with you on your quest!\"");
			pline("He hands %s to you.", the(xname(obj)));
			obj = addinv(obj);	/* into your inventory */
			(void) encumber_msg();
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
	  }
	}
}

void
leader_speaks(mtmp)
	register struct monst *mtmp;
{
	/* maybe you attacked leader? */
	if(!mtmp->mpeaceful) {
		if(!Role_if(PM_EXILE)){
			Qstat(pissed_off) = TRUE;
			mtmp->mstrategy &= ~STRAT_WAITMASK;	/* end the inaction */
		} else {
			mtmp->mpeaceful = 1;
			Qstat(pissed_off) = FALSE;
		}
	}
	/* the quest leader might have passed through the portal into the
	   regular dungeon; if so, mustn't perform "backwards expulsion" */
	/* Some leaders (Eclavdra, currently) can attack the player under certain conditions */
	if (!on_level(&u.uz, &qstart_level) || flags.leader_backstab) return;

	if(Qstat(pissed_off)) {
	  qt_pager(QT_LASTLEADER + (flags.stag ? QT_TURNEDSTAG : 0));
	  expulsion(TRUE);
	} else chat_with_leader();
}

STATIC_OVL void
chat_with_nemesis()
{
/*	The nemesis will do most of the talking, but... */
	qt_pager(rn1(10, QT_DISCOURAGE + (flags.stag ? QT_TURNEDSTAG : 0)));
	// if(!Qstat(met_nemesis)) Qstat(met_nemesis++);
}

void
nemesis_speaks()
{
	if(!Qstat(in_battle)) {
	  if(u.uhave.questart && 
		(Qstat(met_nemesis) || 
		!((Race_if(PM_DROW) && Role_if(PM_NOBLEMAN && flags.initgend)) || (Race_if(PM_DWARF) && Role_if(PM_NOBLEMAN)) || (Role_if(PM_EXILE)))
		)
	  ) qt_pager(QT_NEMWANTSIT + (flags.stag ? QT_TURNEDSTAG : 0));
	  else if(!Qstat(met_nemesis))
	      qt_pager(QT_FIRSTNEMESIS + (flags.stag ? QT_TURNEDSTAG : 0));
	  else if(Qstat(made_goal) < 4) qt_pager(QT_NEXTNEMESIS + (flags.stag ? QT_TURNEDSTAG : 0));
	  else if(Qstat(made_goal) < 7) qt_pager(QT_OTHERNEMESIS + (flags.stag ? QT_TURNEDSTAG : 0));
	  else if(!rn2(5))	qt_pager(rn1(10, QT_DISCOURAGE + (flags.stag ? QT_TURNEDSTAG : 0)));
	  if(Qstat(made_goal) < 7) Qstat(made_goal)++;
	  if(!Qstat(met_nemesis) && Race_if(PM_DROW) && 
		(Role_if(PM_PRIEST) || Role_if(PM_ROGUE) || Role_if(PM_RANGER) || Role_if(PM_WIZARD)) && 
		!flags.stag && (yn("Betray your current quest leader and join the uprising?") == 'y')){
		turn_stag();
	  }
	  Qstat(met_nemesis) = TRUE;
	} else {
		if(!Qstat(met_nemesis)){
			qt_pager(QT_FIRSTNEMESIS + (flags.stag ? QT_TURNEDSTAG : 0));
			  Qstat(met_nemesis) = TRUE;
			  if(Race_if(PM_DROW) && 
				(Role_if(PM_PRIEST) || Role_if(PM_ROGUE) || Role_if(PM_RANGER) || Role_if(PM_WIZARD)) && 
				!flags.stag && (yn("Betray your current quest leader and join the uprising?") == 'y')){
				turn_stag();
			  }
		/* he will spit out random maledictions */
		} else if(!rn2(5))	qt_pager(rn1(10, QT_DISCOURAGE + (flags.stag ? QT_TURNEDSTAG : 0)));
	}
	if(Race_if(PM_DROW) && (Role_if(PM_PRIEST) || Role_if(PM_ROGUE) || Role_if(PM_RANGER) || Role_if(PM_WIZARD)) && flags.stag){
	/*Correct peacefulness settings*/
		struct monst *tm;
		for(tm = fmon; tm; tm = tm->nmon){
			if(tm->mfaction != EILISTRAEE_SYMBOL && 
				tm->mfaction != XAXOX && tm->mfaction != EDDER_SYMBOL && 
				is_drow(tm->data) && !tm->mtame 
			){
				tm->housealert = 1;
				tm->mpeaceful = 0;
				tm->mstrategy &= ~STRAT_WAITMASK;
				set_malign(tm);
			}
		}
	}
}

STATIC_OVL void
chat_with_guardian()
{
/*	These guys/gals really don't have much to say... */
	if (u.uhave.questart && Qstat(killed_nemesis))
	    qt_pager(rn1(5, QT_GUARDTALK2 + (flags.stag ? QT_TURNEDSTAG : 0)));
	else
	    qt_pager(rn1(5, QT_GUARDTALK + (flags.stag ? QT_TURNEDSTAG : 0)));
}

STATIC_OVL void
prisoner_speaks(mtmp)
	register struct monst *mtmp;
{
	if (mtmp->data == &mons[PM_PRISONER] &&
			(mtmp->mstrategy & STRAT_WAITMASK)) {
	    /* Awaken the prisoner */
	    if (canseemon(mtmp))
	    	pline("%s speaks:", Monnam(mtmp));
	    verbalize("I'm finally free!");
	    mtmp->mstrategy &= ~STRAT_WAITMASK;
	    mtmp->mpeaceful = 1;

	    /* Your god is happy... */
	    adjalign(3);

		/* ...But the guards are not */
	    (void) angry_guards(FALSE);
	} else if ((mtmp->data == &mons[PM_MINDLESS_THRALL] || mtmp->data == &mons[PM_A_GONE]) &&
			(mtmp->mstrategy & STRAT_WAITMASK)) {
	    /* Awaken the prisoner */
	    if (canseemon(mtmp))
	    	pline("%s speaks:", Monnam(mtmp));
	    domonnoise(mtmp, TRUE);
	    mtmp->mstrategy &= ~STRAT_WAITMASK;
	}
	return;
}

static const char *elderBrain[] = {
	"YOU ARE ALL LOST.",
	"ALL TIME IS ONE IN US.",
	"SUBMIT.",
	"ABANDON YOUR SELF.",
	"GIVE IN TO US.",
	"REST.",
	"SLEEP FOREVER.",
	"GIVE UP.",
	"WHY DO YOU STRUGGLE?",
	"DO NOT BE AFRAID.",
	"WE RULE ALL.",
	"YOU PROLONG YOUR SUFFERING.",
	"TO WHAT END DO YOU STRIVE?",
	"PENUMBRA HANGS OVER ALL.",
	"PENUMBRA'S ARC IS CLOSED."
};

void
quest_chat(mtmp)
	register struct monst *mtmp;
{
    if (mtmp->m_id == Qstat(leader_m_id)) {
		chat_with_leader();
		return;
    }
	
	if(Role_if(PM_ANACHRONONAUT) && mtmp->data == &mons[PM_ELDER_BRAIN]){
		pline("%s",elderBrain[rn2(SIZE(elderBrain))]);
		return;
	}
	
	if(
		(Role_if(PM_NOBLEMAN) && 
			(mtmp->data == &mons[PM_KNIGHT] 
				|| mtmp->data == &mons[PM_MAID]) && mtmp->mpeaceful)
	  || (Role_if(PM_ANACHRONONAUT) && 
			(mtmp->data == &mons[PM_MYRKALFAR_WARRIOR] || mtmp->data == &mons[PM_ALIDER] 
				|| mtmp->data == &mons[PM_MYRKALFAR_MATRON]) && mtmp->mpeaceful)
	  || (Race_if(PM_DROW) && is_drow(mtmp->data))
	  || (Role_if(PM_EXILE) && 
			mtmp->data == &mons[PM_PEASANT] && mtmp->mpeaceful)
	  || (Race_if(PM_GNOME) && Role_if(PM_RANGER) && (mtmp->data == &mons[PM_GNOME] || mtmp->data == &mons[PM_GNOME_LORD] || mtmp->data == &mons[PM_GNOME_KING]
			|| mtmp->data == &mons[PM_TINKER_GNOME] || mtmp->data == &mons[PM_GNOMISH_WIZARD]) && mtmp->mpeaceful)
	){
		chat_with_guardian();
	} else {
		switch(mtmp->data->msound) {
			case MS_NEMESIS:	chat_with_nemesis(); break;
			case MS_GUARDIAN:	chat_with_guardian(); break;
			default:	impossible("quest_chat: Unknown quest character %s.",
					   mon_nam(mtmp));
		}
	}
}

void
quest_talk(mtmp)
	register struct monst *mtmp;
{
    if (mtmp->m_id == Qstat(leader_m_id)) {
	leader_speaks(mtmp);
	return;
    }
    switch(mtmp->data->msound) {
	    case MS_NEMESIS:	nemesis_speaks(); break;
	    case MS_DJINNI:	prisoner_speaks(mtmp); break;
	    default:		break;
	}
}

void
quest_stat_check(mtmp)
	struct monst *mtmp;
{
    if(mtmp->data->msound == MS_NEMESIS)
	Qstat(in_battle) = (mtmp->mcanmove && mtmp->mnotlaugh && !mtmp->msleeping &&
			    monnear(mtmp, u.ux, u.uy));
}

void
turn_stag()
{
	flags.stag = TRUE;
	/*Convert to new alignment (Even if already did once before) */
	if(u.ualignbase[A_CURRENT] != A_LAWFUL){
		You("have a strong feeling that Lolth is angry...");
		u.ugangr[Align2gangr(A_CHAOTIC)]+=20;
		u.ugangr[Align2gangr(A_NEUTRAL)]+=20;
		if(flags.initgend){
			pline("Eilistraee accepts your allegiance.");
			u.uhouse = EILISTRAEE_SYMBOL;
		}
		else {
			pline("The black web enfolds you.");
			u.uhouse = EDDER_SYMBOL;
		}
		if (uarmh && uarmh->otyp == HELM_OF_OPPOSITE_ALIGNMENT)
		u.ualignbase[A_CURRENT] = A_LAWFUL;
		else
		u.ualign.type = u.ualignbase[A_CURRENT] = A_LAWFUL;
		u.ublessed = 0;
		flags.botl = 1;

		You("have a sudden sense of a new direction.");
		/* Beware, Conversion is costly */
		change_luck(-3);
		u.ublesscnt += 300;
		u.lastprayed = moves;
		u.reconciled = REC_NONE;
		u.lastprayresult = PRAY_CONV;
		adjalign((int)(u.ualignbase[A_ORIGINAL] * (ALIGNLIM / 2)));
	}
	/*Eliminate old monster tags*/
	    mons[urole.ldrnum].msound = MS_CUSS;
	    mons[urole.ldrnum].mflagst &= ~(MT_PEACEFUL);
	    mons[urole.ldrnum].mflagst |= (MT_STALK|MT_HOSTILE|MT_WANTSARTI|MT_WAITFORU);
	    mons[urole.ldrnum].mflagsg |= MG_NASTY;
		
	    mons[urole.guardnum].msound = MS_CUSS;
	    mons[urole.guardnum].mflagst &= ~(MT_PEACEFUL);
	    mons[urole.guardnum].mflagst |= MT_WAITFORU|MT_STALK|MT_HOSTILE;
	    mons[urole.guardnum].mflagsg |= (MG_NASTY);
		
	    mons[urole.neminum].msound = MS_DJINNI;
	    mons[urole.neminum].mflagst |= MT_PEACEFUL;
	    mons[urole.neminum].mflagst &= ~(MT_STALK|MT_HOSTILE|MT_WAITFORU|MT_COVETOUS);
	    mons[urole.neminum].mflagsg &= ~(MG_NASTY);
	/*Fix up role*/
		if(flags.initgend){
			/*true = female*/
			urole.homebase = "the Grove of Eilistraee";
			urole.intermed = "Erelhei-Cinlu";
			urole.questarti = ART_TENTACLE_ROD;
			
			urole.ldrnum = PM_SEYLL_AUZKOVYN;
			urole.guardnum = PM_STJARNA_ALFR;
			urole.neminum = PM_ECLAVDRA;
			
			urole.enemy1num = PM_HEDROW_WARRIOR;
			urole.enemy2num = PM_DROW_MATRON;
			urole.enemy1sym = S_IMP;
			urole.enemy2sym = S_DEMON;
		} else{
			urole.lgod = getDrowMaleLgodKnown();
			
			urole.homebase = "Tower Xaxox";
			urole.intermed = "Menzoberranzan";
			urole.questarti = ART_TENTACLE_ROD;
			
			urole.ldrnum = PM_DARUTH_XAXOX;
			urole.guardnum = PM_DROW_ALIENIST;
			urole.neminum = PM_ECLAVDRA;
			
			urole.enemy1num = PM_DROW_MATRON;
			urole.enemy2num = PM_HEDROW_ZOMBIE;
			urole.enemy1sym = S_DEMON;
			urole.enemy2sym = S_ZOMBIE;
		}
	/*Fix up artifacts*/
	{
		struct artifact *art;
		int alignmnt = flags.stag ? u.ualign.type : aligns[flags.initalign].value;
		/* Fix up the alignments of "gift" artifacts */
		for (art = artilist+1; art->otyp; art++)
			if (art->role == Role_switch && art->alignment != A_NONE)
				art->alignment = alignmnt;
		if (urole.questarti) {
			artilist[urole.questarti].alignment = alignmnt;
			artilist[urole.questarti].role = Role_switch;
		}
	}
	/*Fix monster flags*/
	{
	    mons[urole.ldrnum].msound = MS_LEADER;
	    mons[urole.ldrnum].mflagst |= (MT_PEACEFUL|MT_CLOSE);
	    mons[urole.ldrnum].mflagst &= ~(MT_WAITFORU|MT_COVETOUS);
		
	    mons[urole.guardnum].msound = MS_GUARDIAN;
	    mons[urole.guardnum].mflagst |= (MT_PEACEFUL);
	    mons[urole.guardnum].mflagst &= ~(MT_WAITFORU|MT_COVETOUS);
		
	    mons[urole.neminum].msound = MS_NEMESIS;
	    mons[urole.neminum].mflagst &= ~(MT_PEACEFUL);
	    mons[urole.neminum].mflagst |= (MT_WANTSARTI|MT_WAITFORU|MT_STALK|MT_HOSTILE);
	    mons[urole.neminum].mflagsg |= MG_NASTY;
	}
	/*Correct Qstat settings*/
	{
		struct monst *tm;
		for(tm = fmon; tm; tm = tm->nmon){
			if(tm->data == &mons[urole.ldrnum]){
				tm->housealert = 1;
				tm->mpeaceful = 1;
				tm->mstrategy = STRAT_CLOSE;
				Qstat(leader_m_id) = tm->m_id;
				set_malign(tm);
				break;
			}
		}
		Qstat(met_leader) = 0;
		Qstat(pissed_off) = 0;
		Qstat(got_quest) = 0;
		Qstat(got_final) = 0;
		Qstat(first_start) = 0;
		Qstat(made_goal) = 0;
		Qstat(met_nemesis) = 0;
		Qstat(killed_nemesis) = 0;
		Qstat(in_battle) = 0;
		Qstat(touched_artifact) = 0;
		Qstat(offered_artifact) = 0;
		Qstat(got_thanks) = 0;
		u.uhave.questart = 0; /*Maybe will flip if you got Tent rod first somehow*/
		u.uevent.qcompleted = 0;	/* you didn't do it! */
	}
	/*Correct peacefulness settings*/
	{
		struct monst *tm;
		if(flags.initgend){
			for(tm = fmon; tm; tm = tm->nmon){
				if(tm->mfaction == EILISTRAEE_SYMBOL || is_elf(tm->data)){
					tm->housealert = 1;
					tm->mpeaceful = 1;
					set_malign(tm);
				}
			}
		} else {
			for(tm = fmon; tm; tm = tm->nmon){
				if(tm->mfaction == EDDER_SYMBOL || 
					tm->mfaction == XAXOX || 
					tm->data == &mons[PM_EDDERKOP]
				){
					tm->housealert = 1;
					tm->mpeaceful = 1;
					set_malign(tm);
				}
			}
		}
	}
	/*Give you an appropriate signet ring*/
	{
		struct obj *otmp;
		otmp = mksobj(find_signet_ring(), TRUE, FALSE);
		otmp->ohaluengr = TRUE;
		otmp->oward = flags.initgend ? EILISTRAEE_SYMBOL : EDDER_SYMBOL;
		pline("\"Take this, to identify you as one of ours!  You should take off any armor with another faction's crest, though.\"");
		hold_another_object(otmp, "You drop %s!",
							  doname(otmp), (const char *)0);	}
}

/*quest.c*/
