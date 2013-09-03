/*	SCCS Id: @(#)sounds.c	3.4	2002/05/06	*/
/*	Copyright (c) 1989 Janet Walz, Mike Threepoint */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"
#ifdef USER_SOUNDS
# ifdef USER_SOUNDS_REGEX
#include <regex.h>
# endif
#endif

#ifdef OVLB

static int FDECL(domonnoise,(struct monst *));
int FDECL(dobinding,(int, int));
static int NDECL(dochat);

#endif /* OVLB */

#ifdef OVL0

static int FDECL(mon_in_room, (struct monst *,int));

//I am Buer, X,
static const char *buerTitles[] = {
	"grandmother huntress",
	"grandfather hunter",
	"the wandering sage",
	"the forsaken sage",
	"the banished sage",
	"interent teacher",
	"fallen of heaven",
	"risen of hell",
	"the first healer",
	"philosopher of nature",
	"of the philosophy of healing",
	"philosopher of healing",
	"philosopher of natural morality",
	"the lost",
	"the ignored",
	"paragon of morality"
};
static const char *buerSetOne[] = {
	"cursed by a witch",
	"doomed by the gods",
	"seduced by a demon and made",
	"taken by a fey spirit and left",
	"destined",
	"determined"
};
//to
static const char *buerSetTwo[] = {
	"wander for all time",
	"walk through the ages",
	"search the five corners of the world",
	"bear witness 'til all is redemed at last",
	"never be released by death",
	"search for redemption, but never to find it",
	"seek virtue forevermore"
};

/* this easily could be a macro, but it might overtax dumb compilers */
static int
mon_in_room(mon, rmtyp)
struct monst *mon;
int rmtyp;
{
    int rno = levl[mon->mx][mon->my].roomno;

    return rooms[rno - ROOMOFFSET].rtype == rmtyp;
}

void
dosounds()
{
    register struct mkroom *sroom;
    register int hallu, vx, vy;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
    int xx;
#endif
    struct monst *mtmp;

    if (!flags.soundok || u.uswallow || Underwater) return;

    hallu = Hallucination ? 1 : 0;

    if (level.flags.nfountains && !rn2(400)) {
	static const char * const fountain_msg[4] = {
		"bubbling water.",
		"water falling on coins.",
		"the splashing of a naiad.",
		"a soda fountain!",
	};
	You_hear(fountain_msg[rn2(3)+hallu]);
    }
#ifdef SINK
    if (level.flags.nsinks && !rn2(300)) {
	static const char * const sink_msg[3] = {
		"a slow drip.",
		"a gurgling noise.",
		"dishes being washed!",
	};
	You_hear(sink_msg[rn2(2)+hallu]);
    }
#endif
    if (level.flags.has_court && !rn2(200)) {
	static const char * const throne_msg[4] = {
		"the tones of courtly conversation.",
		"a sceptre pounded in judgment.",
		"Someone shouts \"Off with %s head!\"",
		"Queen Beruthiel's cats!",
	};
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if ((mtmp->msleeping ||
			is_lord(mtmp->data) || is_prince(mtmp->data)) &&
		!is_animal(mtmp->data) &&
		mon_in_room(mtmp, COURT)) {
		/* finding one is enough, at least for now */
		int which = rn2(3)+hallu;

		if (which != 2) You_hear(throne_msg[which]);
		else		pline(throne_msg[2], uhis());
		return;
	    }
	}
    }
    if (level.flags.has_swamp && !rn2(200)) {
	static const char * const swamp_msg[3] = {
		"hear mosquitoes!",
		"smell marsh gas!",	/* so it's a smell...*/
		"hear Donald Duck!",
	};
	You(swamp_msg[rn2(2)+hallu]);
	return;
    }
    if (level.flags.has_vault && !rn2(200)) {
	if (!(sroom = search_special(VAULT))) {
	    /* strange ... */
	    level.flags.has_vault = 0;
	    return;
	}
	if(gd_sound())
	    switch (rn2(2)+hallu) {
		case 1: {
		    boolean gold_in_vault = FALSE;

		    for (vx = sroom->lx;vx <= sroom->hx; vx++)
			for (vy = sroom->ly; vy <= sroom->hy; vy++)
			    if (g_at(vx, vy))
				gold_in_vault = TRUE;
#if defined(AMIGA) && defined(AZTEC_C_WORKAROUND)
		    /* Bug in aztec assembler here. Workaround below */
		    xx = ROOM_INDEX(sroom) + ROOMOFFSET;
		    xx = (xx != vault_occupied(u.urooms));
		    if(xx)
#else
		    if (vault_occupied(u.urooms) !=
			 (ROOM_INDEX(sroom) + ROOMOFFSET))
#endif /* AZTEC_C_WORKAROUND */
		    {
			if (gold_in_vault)
			    You_hear(!hallu ? "someone counting money." :
				"the quarterback calling the play.");
			else
			    You_hear("someone searching.");
			break;
		    }
		    /* fall into... (yes, even for hallucination) */
		}
		case 0:
		    You_hear("the footsteps of a guard on patrol.");
		    break;
		case 2:
		    You_hear("Ebenezer Scrooge!");
		    break;
	    }
	return;
    }
    if (level.flags.has_beehive && !rn2(200)) {
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if ((mtmp->data->mlet == S_ANT && is_flyer(mtmp->data)) &&
		mon_in_room(mtmp, BEEHIVE)) {
		switch (rn2(2)+hallu) {
		    case 0:
			You_hear("a low buzzing.");
			break;
		    case 1:
			You_hear("an angry drone.");
			break;
		    case 2:
			You_hear("bees in your %sbonnet!",
			    uarmh ? "" : "(nonexistent) ");
			break;
		}
		return;
	    }
	}
    }
    if (level.flags.has_morgue && !rn2(200)) {
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if (is_undead(mtmp->data) &&
		mon_in_room(mtmp, MORGUE)) {
		switch (rn2(2)+hallu) {
		    case 0:
			You("suddenly realize it is unnaturally quiet.");
			break;
		    case 1:
			pline_The("%s on the back of your %s stands up.",
				body_part(HAIR), body_part(NECK));
			break;
		    case 2:
			pline_The("%s on your %s seems to stand up.",
				body_part(HAIR), body_part(HEAD));
			break;
		}
		return;
	    }
	}
    }
    if (level.flags.has_barracks && !rn2(200)) {
	static const char * const barracks_msg[4] = {
		"blades being honed.",
		"loud snoring.",
		"dice being thrown.",
		"General MacArthur!",
	};
	int count = 0;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if (is_mercenary(mtmp->data) &&
#if 0		/* don't bother excluding these */
		!strstri(mtmp->data->mname, "watch") &&
		!strstri(mtmp->data->mname, "guard") &&
#endif
		mon_in_room(mtmp, BARRACKS) &&
		/* sleeping implies not-yet-disturbed (usually) */
		(mtmp->msleeping || ++count > 5)) {
		You_hear(barracks_msg[rn2(3)+hallu]);
		return;
	    }
	}
    }
    if (level.flags.has_zoo && !rn2(200)) {
	static const char * const zoo_msg[3] = {
		"a sound reminiscent of an elephant stepping on a peanut.",
		"a sound reminiscent of a seal barking.",
		"Doctor Doolittle!",
	};
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if ((mtmp->msleeping || is_animal(mtmp->data)) &&
		    mon_in_room(mtmp, ZOO)) {
		You_hear(zoo_msg[rn2(2)+hallu]);
		return;
	    }
	}
    }
    if (level.flags.has_shop && !rn2(200)) {
	if (!(sroom = search_special(ANY_SHOP))) {
	    /* strange... */
	    level.flags.has_shop = 0;
	    return;
	}
	if (tended_shop(sroom) &&
		!index(u.ushops, ROOM_INDEX(sroom) + ROOMOFFSET)) {
	    static const char * const shop_msg[3] = {
		    "someone cursing shoplifters.",
		    "the chime of a cash register.",
		    "Neiman and Marcus arguing!",
	    };
	    You_hear(shop_msg[rn2(2)+hallu]);
	}
	return;
    }
    if (Is_oracle_level(&u.uz) && !rn2(400)) {
	/* make sure the Oracle is still here */
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
	    if (!DEADMONSTER(mtmp) && mtmp->data == &mons[PM_ORACLE])
		break;
	/* and don't produce silly effects when she's clearly visible */
	if (mtmp && (hallu || !canseemon(mtmp))) {
	    static const char * const ora_msg[5] = {
		    "a strange wind.",		/* Jupiter at Dodona */
		    "convulsive ravings.",	/* Apollo at Delphi */
		    "snoring snakes.",		/* AEsculapius at Epidaurus */
		    "someone say \"No more woodchucks!\"",
		    "a loud ZOT!"		/* both rec.humor.oracle */
	    };
	    You_hear(ora_msg[rn2(3)+hallu*2]);
	}
	return;
    }
}

#endif /* OVL0 */
#ifdef OVLB

static const char * const h_sounds[] = {
    "beep", "boing", "sing", "belche", "creak", "cough", "rattle",
    "ululate", "pop", "jingle", "sniffle", "tinkle", "eep"
};

const char *
growl_sound(mtmp)
register struct monst *mtmp;
{
	const char *ret;

	switch (mtmp->data->msound) {
	case MS_MEW:
	case MS_HISS:
	    ret = "hiss";
	    break;
	case MS_BARK:
	case MS_GROWL:
	    ret = "growl";
	    break;
	case MS_ROAR:
	    ret = "roar";
	    break;
	case MS_BUZZ:
	    ret = "buzz";
	    break;
	case MS_SQEEK:
	    ret = "squeal";
	    break;
	case MS_SQAWK:
	    ret = "screech";
	    break;
	case MS_NEIGH:
	    ret = "neigh";
	    break;
	case MS_WAIL:
	    ret = "wail";
	    break;
	case MS_SILENT:
		ret = "commotion";
		break;
	default:
		ret = "scream";
	}
	return ret;
}

/* the sounds of a seriously abused pet, including player attacking it */
void
growl(mtmp)
register struct monst *mtmp;
{
    register const char *growl_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
	return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
	growl_verb = h_sounds[rn2(SIZE(h_sounds))];
    else
	growl_verb = growl_sound(mtmp);
    if (growl_verb) {
	pline("%s %s!", Monnam(mtmp), vtense((char *)0, growl_verb));
	if(flags.run) nomul(0);
	wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 18);
    }
}

/* the sounds of mistreated pets */
void
yelp(mtmp)
register struct monst *mtmp;
{
    register const char *yelp_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
	return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
	yelp_verb = h_sounds[rn2(SIZE(h_sounds))];
    else switch (mtmp->data->msound) {
	case MS_MEW:
	    yelp_verb = "yowl";
	    break;
	case MS_BARK:
	case MS_GROWL:
	    yelp_verb = "yelp";
	    break;
	case MS_ROAR:
	    yelp_verb = "snarl";
	    break;
	case MS_SQEEK:
	    yelp_verb = "squeal";
	    break;
	case MS_SQAWK:
	    yelp_verb = "screak";
	    break;
	case MS_WAIL:
	    yelp_verb = "wail";
	    break;
    }
    if (yelp_verb) {
	pline("%s %s!", Monnam(mtmp), vtense((char *)0, yelp_verb));
	if(flags.run) nomul(0);
	wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 12);
    }
}

/* the sounds of distressed pets */
void
whimper(mtmp)
register struct monst *mtmp;
{
    register const char *whimper_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->data->msound)
	return;

    /* presumably nearness and soundok checks have already been made */
    if (Hallucination)
	whimper_verb = h_sounds[rn2(SIZE(h_sounds))];
    else switch (mtmp->data->msound) {
	case MS_MEW:
	case MS_GROWL:
	    whimper_verb = "whimper";
	    break;
	case MS_BARK:
	    whimper_verb = "whine";
	    break;
	case MS_SQEEK:
	    whimper_verb = "squeal";
	    break;
    }
    if (whimper_verb) {
	pline("%s %s.", Monnam(mtmp), vtense((char *)0, whimper_verb));
	if(flags.run) nomul(0);
	wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 6);
    }
}

/* pet makes "I'm hungry" noises */
void
beg(mtmp)
register struct monst *mtmp;
{
    if (mtmp->msleeping || !mtmp->mcanmove ||
	    !(carnivorous(mtmp->data) || herbivorous(mtmp->data)))
	return;

    /* presumably nearness and soundok checks have already been made */
    if (!is_silent(mtmp->data) && mtmp->data->msound <= MS_ANIMAL)
	(void) domonnoise(mtmp);
    else if (mtmp->data->msound >= MS_HUMANOID) {
	if (!canspotmon(mtmp))
	    map_invisible(mtmp->mx, mtmp->my);
	verbalize("I'm hungry.");
    }
}

static int
domonnoise(mtmp)
register struct monst *mtmp;
{
    register const char *pline_msg = 0,	/* Monnam(mtmp) will be prepended */
			*verbl_msg = 0;	/* verbalize() */
    struct permonst *ptr = mtmp->data;
    char verbuf[BUFSZ];

    /* presumably nearness and sleep checks have already been made */
    if (!flags.soundok) return(0);
    if (is_silent(ptr)) return(0);

    /* Make sure its your role's quest quardian; adjust if not */
    if (ptr->msound == MS_GUARDIAN && ptr != &mons[urole.guardnum]) {
    	int mndx = monsndx(ptr);
    	ptr = &mons[genus(mndx,1)];
    }

    /* be sure to do this before talking; the monster might teleport away, in
     * which case we want to check its pre-teleport position
     */
    if (!canspotmon(mtmp))
	map_invisible(mtmp->mx, mtmp->my);

    switch (ptr->msound) {
	case MS_ORACLE:
	    return doconsult(mtmp);
	case MS_PRIEST:
	    priest_talk(mtmp);
	    break;
	case MS_LEADER:
	case MS_NEMESIS:
	case MS_GUARDIAN:
	    quest_chat(mtmp);
	    break;
	case MS_SELL: /* pitch, pay, total */
	    shk_chat(mtmp);
	    break;
	case MS_VAMPIRE:
	    {
	    /* vampire messages are varied by tameness, peacefulness, and time of night */
		boolean isnight = night();
		boolean kindred =    (Upolyd && (u.umonnum == PM_VAMPIRE ||
				       u.umonnum == PM_VAMPIRE_LORD));
		boolean nightchild = (Upolyd && (u.umonnum == PM_WOLF ||
				       u.umonnum == PM_WINTER_WOLF ||
	    			       u.umonnum == PM_WINTER_WOLF_CUB));
		const char *racenoun = (flags.female && urace.individual.f) ?
					urace.individual.f : (urace.individual.m) ?
					urace.individual.m : urace.noun;

		if (mtmp->mtame) {
			if (kindred) {
				Sprintf(verbuf, "Good %s to you Master%s",
					isnight ? "evening" : "day",
					isnight ? "!" : ".  Why do we not rest?");
				verbl_msg = verbuf;
		    	} else {
		    	    Sprintf(verbuf,"%s%s",
				nightchild ? "Child of the night, " : "",
				midnight() ?
					"I can stand this craving no longer!" :
				isnight ?
					"I beg you, help me satisfy this growing craving!" :
					"I find myself growing a little weary.");
				verbl_msg = verbuf;
			}
		} else if (mtmp->mpeaceful) {
			if (kindred && isnight) {
				Sprintf(verbuf, "Good feeding %s!",
	    				flags.female ? "sister" : "brother");
				verbl_msg = verbuf;
 			} else if (nightchild && isnight) {
				Sprintf(verbuf,
				    "How nice to hear you, child of the night!");
				verbl_msg = verbuf;
	    		} else
		    		verbl_msg = "I only drink... potions.";
    	        } else {
			int vampindex;
	    		static const char * const vampmsg[] = {
			       /* These first two (0 and 1) are specially handled below */
	    			"I vant to suck your %s!",
	    			"I vill come after %s without regret!",
		    	       /* other famous vampire quotes can follow here if desired */
	    		};
			if (kindred)
			    verbl_msg = "This is my hunting ground that you dare to prowl!";
			else if (youmonst.data == &mons[PM_SILVER_DRAGON] ||
				 youmonst.data == &mons[PM_BABY_SILVER_DRAGON]) {
			    /* Silver dragons are silver in color, not made of silver */
			    Sprintf(verbuf, "%s! Your silver sheen does not frighten me!",
					youmonst.data == &mons[PM_SILVER_DRAGON] ?
					"Fool" : "Young Fool");
			    verbl_msg = verbuf; 
			} else {
			    vampindex = rn2(SIZE(vampmsg));
			    if (vampindex == 0) {
				Sprintf(verbuf, vampmsg[vampindex], body_part(BLOOD));
	    			verbl_msg = verbuf;
			    } else if (vampindex == 1) {
				Sprintf(verbuf, vampmsg[vampindex],
					Upolyd ? an(mons[u.umonnum].mname) : an(racenoun));
	    			verbl_msg = verbuf;
		    	    } else
			    	verbl_msg = vampmsg[vampindex];
			}
	        }
	    }
	    break;
	case MS_WERE:
	    if (flags.moonphase == FULL_MOON && (night() ^ !rn2(13))) {
		pline("%s throws back %s head and lets out a blood curdling %s!",
		      Monnam(mtmp), mhis(mtmp),
		      ptr == &mons[PM_HUMAN_WERERAT] ? "shriek" : "howl");
		wake_nearto(mtmp->mx, mtmp->my, 11*11);
	    } else
		pline_msg =
		     "whispers inaudibly.  All you can make out is \"moon\".";
	    break;
	case MS_BARK:
	    if (flags.moonphase == FULL_MOON && night()) {
		pline_msg = "howls.";
	    } else if (mtmp->mpeaceful) {
		if (mtmp->mtame &&
			(mtmp->mconf || mtmp->mflee || mtmp->mtrapped ||
			 moves > EDOG(mtmp)->hungrytime || mtmp->mtame < 5))
		    pline_msg = "whines.";
		else if (mtmp->mtame && EDOG(mtmp)->hungrytime > moves + 1000)
		    pline_msg = "yips.";
		else {
		    if (mtmp->data != &mons[PM_DINGO])	/* dingos do not actually bark */
			    pline_msg = "barks.";
		}
	    } else {
		pline_msg = "growls.";
	    }
	    break;
	case MS_MEW:
	    if (mtmp->mtame) {
		if (mtmp->mconf || mtmp->mflee || mtmp->mtrapped ||
			mtmp->mtame < 5)
		    pline_msg = "yowls.";
		else if (moves > EDOG(mtmp)->hungrytime)
		    pline_msg = "meows.";
		else if (EDOG(mtmp)->hungrytime > moves + 1000)
		    pline_msg = "purrs.";
		else
		    pline_msg = "mews.";
		break;
	    } /* else FALLTHRU */
	case MS_GROWL:
	    pline_msg = mtmp->mpeaceful ? "snarls." : "growls!";
	    break;
	case MS_ROAR:
	    pline_msg = mtmp->mpeaceful ? "snarls." : "roars!";
	    break;
	case MS_SQEEK:
	    pline_msg = "squeaks.";
	    break;
	case MS_SQAWK:
	    if (ptr == &mons[PM_RAVEN] && !mtmp->mpeaceful)
	    	verbl_msg = "Nevermore!";
	    else
	    	pline_msg = "squawks.";
	    break;
	case MS_HISS:
	    if (!mtmp->mpeaceful)
		pline_msg = "hisses!";
	    else return 0;	/* no sound */
	    break;
	case MS_BUZZ:
	    pline_msg = mtmp->mpeaceful ? "drones." : "buzzes angrily.";
	    break;
	case MS_GRUNT:
	    pline_msg = "grunts.";
	    break;
	case MS_NEIGH:
	    if (mtmp->mtame < 5)
		pline_msg = "neighs.";
	    else if (moves > EDOG(mtmp)->hungrytime)
		pline_msg = "whinnies.";
	    else
		pline_msg = "whickers.";
	    break;
	case MS_WAIL:
	    pline_msg = "wails mournfully.";
	    break;
	case MS_GURGLE:
	    pline_msg = "gurgles.";
	    break;
	case MS_BURBLE:
	    pline_msg = "burbles.";
	    break;
	case MS_SHRIEK:
	    pline_msg = "shrieks.";
	    aggravate();
	    break;
	case MS_IMITATE:
	    pline_msg = "imitates you.";
	    break;
	case MS_BONES:
	    pline("%s rattles noisily.", Monnam(mtmp));
	    You("freeze for a moment.");
	    nomul(-2);
	    break;
	case MS_LAUGH:
	    {
		static const char * const laugh_msg[4] = {
		    "giggles.", "chuckles.", "snickers.", "laughs.",
		};
		pline_msg = laugh_msg[rn2(4)];
	    }
	    break;
	case MS_MUMBLE:
	    pline_msg = "mumbles incomprehensibly.";
	    break;
	case MS_DJINNI:
	    if (mtmp->mtame) {
		verbl_msg = "Sorry, I'm all out of wishes.";
	    } else if (mtmp->mpeaceful) {
		if (ptr == &mons[PM_WATER_DEMON])
		    pline_msg = "gurgles.";
		else
		    verbl_msg = "I'm free!";
	    } else verbl_msg = "This will teach you not to disturb me!";
	    break;
	case MS_BOAST:	/* giants */
	    if (!mtmp->mpeaceful) {
		switch (rn2(4)) {
		case 0: pline("%s boasts about %s gem collection.",
			      Monnam(mtmp), mhis(mtmp));
			break;
		case 1: pline_msg = "complains about a diet of mutton.";
			break;
	       default: pline_msg = "shouts \"Fee Fie Foe Foo!\" and guffaws.";
			wake_nearto(mtmp->mx, mtmp->my, 7*7);
			break;
		}
		break;
	    }
	    /* else FALLTHRU */
	case MS_HUMANOID:
	    if (!mtmp->mpeaceful) {
		if (In_endgame(&u.uz) && is_mplayer(ptr)) {
		    mplayer_talk(mtmp);
		    break;
		} else return 0;	/* no sound */
	    }
	    /* Generic peaceful humanoid behaviour. */
	    if (mtmp->mflee)
		pline_msg = "wants nothing to do with you.";
	    else if (mtmp->mhp < mtmp->mhpmax/4)
		pline_msg = "moans.";
	    else if (mtmp->mconf || mtmp->mstun)
		verbl_msg = !rn2(3) ? "Huh?" : rn2(2) ? "What?" : "Eh?";
	    else if (!mtmp->mcansee)
		verbl_msg = "I can't see!";
	    else if (mtmp->mtrapped) {
		struct trap *t = t_at(mtmp->mx, mtmp->my);

		if (t) t->tseen = 1;
		verbl_msg = "I'm trapped!";
	    } else if (mtmp->mhp < mtmp->mhpmax/2)
		pline_msg = "asks for a potion of healing.";
	    else if (mtmp->mtame && !mtmp->isminion &&
						moves > EDOG(mtmp)->hungrytime)
		verbl_msg = "I'm hungry.";
	    /* Specific monsters' interests */
	    else if (is_elf(ptr))
		pline_msg = "curses orcs.";
	    else if (is_dwarf(ptr))
		pline_msg = "talks about mining.";
	    else if (likes_magic(ptr))
		pline_msg = "talks about spellcraft.";
	    else if (ptr->mlet == S_CENTAUR)
		pline_msg = "discusses hunting.";
	    else switch (monsndx(ptr)) {
		case PM_HOBBIT:
		    pline_msg = (mtmp->mhpmax - mtmp->mhp >= 10) ?
				"complains about unpleasant dungeon conditions."
				: "asks you about the One Ring.";
		    break;
		case PM_ARCHEOLOGIST:
    pline_msg = "describes a recent article in \"Spelunker Today\" magazine.";
		    break;
#ifdef TOURIST
		case PM_TOURIST:
		    verbl_msg = "Aloha.";
		    break;
#endif
		default:
		    pline_msg = "discusses dungeon exploration.";
		    break;
	    }
	    break;
	case MS_SEDUCE:
#ifdef SEDUCE
	    if (ptr->mlet != S_NYMPH &&
		could_seduce(mtmp, &youmonst, (struct attack *)0) == 1) {
			(void) doseduce(mtmp);
			break;
	    }
	    switch ((poly_gender() != (int) mtmp->female) ? rn2(3) : 0)
#else
	    switch ((poly_gender() == 0) ? rn2(3) : 0)
#endif
	    {
		case 2:
			verbl_msg = "Hello, sailor.";
			break;
		case 1:
			pline_msg = "comes on to you.";
			break;
		default:
			pline_msg = "cajoles you.";
	    }
	    break;
	case MS_ARREST:
	    if (mtmp->mpeaceful)
		verbalize("Just the facts, %s.",
		      flags.female ? "Ma'am" : "Sir");
	    else {
		static const char * const arrest_msg[3] = {
		    "Anything you say can be used against you.",
		    "You're under arrest!",
		    "Stop in the name of the Law!",
		};
		verbl_msg = arrest_msg[rn2(3)];
	    }
	    break;
	case MS_BRIBE:
#ifdef CONVICT        
        if (monsndx(ptr) == PM_PRISON_GUARD) {
            long gdemand = 500 * u.ulevel;
            long goffer = 0;

    	    if (!mtmp->mpeaceful && !mtmp->mtame) {
                pline("%s demands %ld %s to avoid re-arrest.",
                 Amonnam(mtmp), gdemand, currency(gdemand));
                if ((goffer = bribe(mtmp)) >= gdemand) {
                    verbl_msg = "Good.  Now beat it, scum!";
            	    mtmp->mpeaceful = 1;
            	    set_malign(mtmp);
                    break;
                } else {
                    pline("I said %ld!", gdemand);
                    mtmp->mspec_used = 1000;
                    break;
                }
            } else {
                verbl_msg = "Out of my way, scum!"; /* still a jerk */
            }
        } else
#endif /* CONVICT */
	    if (mtmp->mpeaceful && !mtmp->mtame) {
		(void) demon_talk(mtmp);
		break;
	    }
	    /* fall through */
	case MS_CUSS:
	    if (!mtmp->mpeaceful)
		cuss(mtmp);
	    break;
	case MS_SPELL:
	    /* deliberately vague, since it's not actually casting any spell */
	    pline_msg = "seems to mutter a cantrip.";
	    break;
	case MS_NURSE:
	    if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
		verbl_msg = "Put that weapon away before you hurt someone!";
	    else if (uarmc || uarm || uarmh || uarms || uarmg || uarmf)
		verbl_msg = Role_if(PM_HEALER) ?
			  "Doc, I can't help you unless you cooperate." :
			  "Please undress so I can examine you.";
#ifdef TOURIST
	    else if (uarmu)
		verbl_msg = "Take off your shirt, please.";
#endif
	    else verbl_msg = "Relax, this won't hurt a bit.";
	    break;
	case MS_GUARD:
#ifndef GOLDOBJ
	    if (u.ugold)
#else
	    if (money_cnt(invent))
#endif
		verbl_msg = "Please drop that gold and follow me.";
	    else
		verbl_msg = "Please follow me.";
	    break;
	case MS_SOLDIER:
	    {
		static const char * const soldier_foe_msg[3] = {
		    "Resistance is useless!",
		    "You're dog meat!",
		    "Surrender!",
		},		  * const soldier_pax_msg[3] = {
		    "What lousy pay we're getting here!",
		    "The food's not fit for Orcs!",
		    "My feet hurt, I've been on them all day!",
		};
		verbl_msg = mtmp->mpeaceful ? soldier_pax_msg[rn2(3)]
					    : soldier_foe_msg[rn2(3)];
	    }
	    break;
	case MS_RIDER:
	    if (ptr == &mons[PM_DEATH] && !rn2(10))
		pline_msg = "is busy reading a copy of Sandman #8.";
	    else verbl_msg = "Who do you think you are, War?";
	    break;
    }

    if (pline_msg) pline("%s %s", Monnam(mtmp), pline_msg);
    else if (verbl_msg) verbalize(verbl_msg);
    return(1);
}


int
dotalk()
{
    int result;
    boolean save_soundok = flags.soundok;
    flags.soundok = 1;	/* always allow sounds while chatting */
    result = dochat();
    flags.soundok = save_soundok;
    return result;
}

static int
dochat()
{
    register struct monst *mtmp;
    register int tx,ty;
    struct obj *otmp;

    if (is_silent(youmonst.data)) {
	pline("As %s, you cannot speak.", an(youmonst.data->mname));
	return(0);
    }
    if (Strangled) {
	You_cant("speak.  You're choking!");
	return(0);
    }
    if (u.uswallow) {
	pline("They won't hear you out there.");
	return(0);
    }

    if (!Blind && (otmp = shop_object(u.ux, u.uy)) != (struct obj *)0) {
	/* standing on something in a shop and chatting causes the shopkeeper
	   to describe the price(s).  This can inhibit other chatting inside
	   a shop, but that shouldn't matter much.  shop_object() returns an
	   object iff inside a shop and the shopkeeper is present and willing
	   (not angry) and able (not asleep) to speak and the position contains
	   any objects other than just gold.
	*/
	price_quote(otmp);
	return(1);
    }

    if (!getdir("Talk to whom? (in what direction)")) {
	/* decided not to chat */
	return(0);
    }

#ifdef STEED
    if (u.usteed && u.dz > 0)
	return (domonnoise(u.usteed));
#endif
    if (u.dz) {
	pline("They won't hear you %s there.", u.dz < 0 ? "up" : "down");
	return(0);
    }

    if (u.dx == 0 && u.dy == 0) {
/*
 * Let's not include this.  It raises all sorts of questions: can you wear
 * 2 helmets, 2 amulets, 3 pairs of gloves or 6 rings as a marilith,
 * etc...  --KAA
	if (u.umonnum == PM_ETTIN) {
	    You("discover that your other head makes boring conversation.");
	    return(1);
	}
*/
	pline("Talking to yourself is a bad habit for a dungeoneer.");
	return(0);
    }

    tx = u.ux+u.dx; ty = u.uy+u.dy;
    mtmp = m_at(tx, ty);

    if (!mtmp || mtmp->mundetected ||
		mtmp->m_ap_type == M_AP_FURNITURE ||
		mtmp->m_ap_type == M_AP_OBJECT)
	return dobinding(tx,ty);

    if (Underwater) {
	Your("speech is unintelligible underwater.");
	return(0);
    }
    /* sleeping monsters won't talk, except priests (who wake up) */
    if ((!mtmp->mcanmove || mtmp->msleeping) && !mtmp->ispriest) {
		/* If it is unseen, the player can't tell the difference between
		   not noticing him and just not existing, so skip the message. */
		if (canspotmon(mtmp))
			pline("%s seems not to notice you.", Monnam(mtmp));
		return(0);
    }

    /* if this monster is waiting for something, prod it into action */
    mtmp->mstrategy &= ~STRAT_WAITMASK;

    if (mtmp->mtame && mtmp->meating) {
		if (!canspotmon(mtmp))
			map_invisible(mtmp->mx, mtmp->my);
		pline("%s is eating noisily.", Monnam(mtmp));
		return (0);
    }
	
	if(mtmp->data == &mons[PM_NIGHTGAUNT] && u.umonnum == PM_GHOUL){
		You("bark the secret passwords known to ghouls.");
		mtmp->mpeaceful = 1;
		mtmp = tamedog(mtmp, (struct obj *)0);
	}
#ifdef CONVICT
    if (Role_if(PM_CONVICT) && is_rat(mtmp->data) && !mtmp->mpeaceful &&
     !mtmp->mtame) {
        You("attempt to soothe the %s with chittering sounds.",
         l_monnam(mtmp));
        if (rnl(10) < 2) {
            (void) tamedog(mtmp, (struct obj *) 0);
        } else {
            if (rnl(10) > 8) {
                pline("%s unfortunately ignores your overtures.",
                 Monnam(mtmp));
                return 0;
            }
            mtmp->mpeaceful = 1;
            set_malign(mtmp);
        }
        return 0;
    }
#endif /* CONVICT */
    return domonnoise(mtmp);
}

//definition of an extern in you.h
//A bag, a silver key, a gold ring, (a pair of dice), a (copper) coin, a dagger, an apple, a scroll, (a comb), a whistle, a mirror, an egg, a potion, a dead spider, (an oak leaf), a dead human (skull and arm bone), (a lock), (a closed black book) a spellbook, a bell, (a (live?) dove), a set of lockpicks, or a live? sewer rat (mouse). The items are consumed.
char *andromaliusItems[18] = {
/*00*/	"a sack",
/*01*/	"a silver key",
/*02*/	"a gold ring",
/*03*/	"a coin",
/*04*/	"a dagger",
/*05*/	"an apple",
/*06*/	"a scroll",
/*07*/	"a whistle",
/*08*/	"a mirror",
/*09*/	"an egg",
/*00*/	"a potion",
/*11*/	"a dead spider",
/*12*/	"a human skull",
/*13*/	"an arm bone",
/*14*/	"a spellbook",
/*15*/	"a bell",
/*16*/	"a set of lockpicks",
/*17*/	"a live sewer rat"
};

int
dobinding(tx,ty)
int tx,ty;
{
	struct engr *ep = get_head_engr();
	int numSlots;
	int bindingPeriod = 5000;
	for(ep;ep;ep=ep->nxt_engr) 
		if(ep->engr_x==tx && ep->engr_y==ty)
			break;//else continue
	if(!(ep)) return 0; //no engraving found
	if(ep->halu_ward || ep->ward_id < FIRST_SEAL || 
		ep->complete_wards < 1 || ep->engr_time+5 < moves) return 0; //engraving does not contain a valid seal, or is too old.
	
	if(u.ulevel <= 2) numSlots=1;
	else if(u.ulevel <= 9) numSlots=2;
	else if(u.ulevel <= 18) numSlots=3;
	else if(u.ulevel <= 25) numSlots=4;
	else numSlots=5;
	
	switch(ep->ward_id){
	case AHAZU:{
		if(u.ahazu < moves){
			struct trap *t=t_at(tx,ty);
			if(t->ttyp == PIT){ //Ahazu requires that his seal be drawn in a pit.
				pline("The walls of the pit are lifted swiftly away, revealing a vast starry expanse beneath the world.");
				if(u.sealCounts < numSlots){
					pline("A voice whispers from bellow:");
					pline("\"All shall feed the shattered night.\"");
					u.sealsActive |= SEAL_AHAZU;
					u.spirit[numSlots] = SEAL_AHAZU;
					u.spiritT[numSlots] = moves + bindingPeriod;
					u.sealCounts++;
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("A voice whispers from bellow:");
					pline("\"All shall feed the shattered night.\"");
					uwep->ovar1 |= SEAL_AHAZU;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_AHAZU;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_AHAZU;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("A voice whispers from bellow, but you don't catch what it says.");
				}
				u.ahazu = moves + bindingPeriod;
			}
		}
	}break;
	case AMON:{
		if(u.amon < moves){
			int curx, cury;
			char altarfound=0;
			//Amon can't be invoked on levels with altars, and in fact doing so causes imediate level loss, as for a broken taboo.
			for(curx=1;curx < COLNO;curx++)
				for(curx=1;cury < ROWNO;cury++)
					if(IS_ALTAR(levl[curx][cury].typ)){ altarfound=1; cury=ROWNO; curx=COLNO;}//end search
			
			if(!altarfound){
				pline("A golden flame roars suddenly to life within the seal, throwning the world into a stark relief of hard-edged shadows and brilliant light.");
				if(u.sealCounts < numSlots){
					pline("No sooner are the shadows born than they rise up against their creator, smothering the flame under a tide of darkness.");
					pline("Even as it dies, a voice speaks from the blood-red flame:");
					pline("\"Cursed are you who calls me forth. I damn you to bear my sign and my flames, alone in this world of darkness!\"");
					u.sealsActive |= SEAL_AMON;
					u.spirit[numSlots] = SEAL_AMON;
					u.spiritT[numSlots] = moves + bindingPeriod;
					u.sealCounts++;
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("No sooner are the shadows born than they rise up against their creator, smothering the flame under a tide of darkness.");
					pline("Even as it dies, a voice speaks from the blood-red flame:");
					pline("\"Cursed are you who calls me forth. I damn you to bear my flames, alone in this world of darkness!\"");
					uwep->ovar1 |= SEAL_AMON;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_AMON;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_AMON;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("No sooner are the shadows born than they rise up against their creator, smothering the flame under a tide of darkness.");
				}
			}
			else{
				losexp("shreading of the soul",TRUE,TRUE,TRUE);
				if(in_rooms(tx, ty, TEMPLE)){
//					struct monst *priest = findpriest(roomno);
					//invoking Amon inside a temple angers the resident deity
					altar_wrath(tx, ty);
					angrygods(a_align(tx,ty));
				}
			}
			u.amon = moves + bindingPeriod; // invoking amon on a level with an altar still triggers the binding period.
		}
	}break;
	case ANDREALPHUS:{
		if(u.andrealphus < moves){
			//Andrealphus requires that his seal be drawn in a corner.
			if(isok(tx+(tx-u.ux), ty+(ty-u.uy)) && IS_CORNER(levl[tx+(tx-u.ux)][ty+(ty-u.uy)].typ) && 
				IS_WALL(levl[tx+(tx-u.ux)][ty].typ) && IS_WALL(levl[tx][ty+(ty-u.uy)].typ)
			){
				Your("perspective shifts, and the walls before you take on new depth.");
				pline("The dim dungeon light refracts oddly, casting the alien figure before you in rainbow hues.");
				if(u.sealCounts < numSlots){
					pline("\"I am Andrealphus, born of angles. In this soft world of curves, I alone am straight and true.\"");
					pline("\"Though born of curves, by my square you shall rectify the world.\"");
					u.sealsActive |= SEAL_ANDREALPHUS;
					u.spirit[numSlots] = SEAL_ANDREALPHUS;
					u.spiritT[numSlots] = moves + bindingPeriod;
					u.sealCounts++;
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("\"I am Andrealphus, born of angles. In this soft world of curves, I alone am straight and true.\"");
					pline("\"Though your instrument is born of the point, by my square it shall rectify the world.\"");
					uwep->ovar1 |= SEAL_ANDREALPHUS;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_ANDREALPHUS;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_ANDREALPHUS;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("\"I am Andrealphus, born of angles. In this soft world of curves, I alone am straight and true.\"");
					pline("\"You, born of dishonest curves, are unworthy of my measure.\"");
				}
				u.andrealphus = moves + bindingPeriod;
			}
		}
	}break;
	case ANDROMALIUS:{ /*UNFINISHED*/
		if(u.andromalius < moves){
			//Seal must be drawn around any two of a bag, a silver key, a gold ring, (a pair of dice), a (copper) coin, a dagger, an apple, a scroll, (a comb), a whistle, a mirror, an egg, a potion, a dead spider, (an oak leaf), a dead human (skull and arm bone), (a lock), (a closed black book) a spellbook, a bell, (a (live?) dove), a set of lockpicks, or a live? sewer rat (mouse). The items are consumed.
			struct obj *o1 = 0, *o2 = 0, *otmp;
			int count = 0;
			int t1, t2;
			static int gldring = 0;
			if (!gldring) gldring = find_gold_ring();
			for(otmp = level.objects[tx][ty]; otmp; otmp = otmp->nexthere)
				if(!otmp->oartifact){
					if(!o1){
						if(otmp->otyp == SACK){ o1 = otmp; t1 = 0;}
						else if(otmp->otyp == UNIVERSAL_KEY){ o1 = otmp; t1 = 1;}
						else if(otmp->oclass == RING_CLASS && otmp->otyp == gldring){ o1 = otmp; t1 = 2;}
						else if(otmp->oclass == COIN_CLASS){ o1 = otmp; t1 = 3;}
						else if(otmp->otyp == DAGGER){ o1 = otmp; t1 = 4;}
						else if(otmp->otyp == APPLE){ o1 = otmp; t1 = 5;}
						else if(otmp->oclass == SCROLL_CLASS){ o1 = otmp; t1 = 6;}
						else if(otmp->otyp == TIN_WHISTLE){ o1 = otmp; t1 = 7;}
						else if(otmp->otyp == MIRROR){ o1 = otmp; t1 = 8;}
						else if(otmp->otyp == EGG){ o1 = otmp; t1 = 9;}
						else if(otmp->oclass == POTION_CLASS){ o1 = otmp; t1 = 10;}
						else if(otmp->oclass == CORPSE && otmp->corpsenm==PM_CAVE_SPIDER){ o1 = otmp; t1 = 11;}
						else if(otmp->oclass == CORPSE && your_race(&mons[otmp->corpsenm])){ o1 = otmp; t1 = 12;}
						else if(otmp->oclass == CORPSE && is_andromaliable(&mons[otmp->corpsenm]) ){ o1 = otmp; t1 = 13;}
						else if(otmp->oclass == SPBOOK_CLASS){ o1 = otmp; t1 = 14;}
						else if(otmp->otyp == BELL){ o1 = otmp; t1 = 15;}
						else if(otmp->otyp == LOCK_PICK){ o1 = otmp; t1 = 16;}
						//live sewer rat
					}
					else if(!o2){
						if(otmp->otyp == SACK && otmp->otyp != o1->otyp){ o2 = otmp; t2 = 0;}
						else if(otmp->otyp == UNIVERSAL_KEY && otmp->otyp != o1->otyp){ o2 = otmp; t2 = 1;}
						else if(otmp->oclass == RING_CLASS && otmp->otyp == gldring && otmp->oclass != o1->oclass){ o2 = otmp; t2 = 2;}
						else if(otmp->oclass == COIN_CLASS && otmp->oclass != o1->oclass){ o2 = otmp; t2 = 3;}
						else if(otmp->otyp == DAGGER && otmp->otyp != o1->otyp){ o2 = otmp; t2 = 4;}
						else if(otmp->otyp == APPLE && otmp->otyp != o1->otyp){ o2 = otmp; t2 = 5;}
						else if(otmp->oclass == SCROLL_CLASS && otmp->oclass != o1->oclass){ o2 = otmp; t2 = 6;}
						else if(otmp->otyp == TIN_WHISTLE && otmp->otyp != o1->otyp){ o2 = otmp; t2 = 7;}
						else if(otmp->otyp == MIRROR && otmp->otyp != o1->otyp){ o2 = otmp; t2 = 8;}
						else if(otmp->otyp == EGG && otmp->otyp != o1->otyp){ o2 = otmp; t2 = 9;}
						else if(otmp->oclass == POTION_CLASS && otmp->oclass != o1->oclass){ o2 = otmp; t2 = 10;}
						else if(otmp->oclass == CORPSE && otmp->corpsenm==PM_CAVE_SPIDER && t1 != 11){ o2 = otmp; t2 = 11;}
						else if(otmp->oclass == CORPSE && your_race(&mons[otmp->corpsenm]) && t1 != 12 && otmp != o1){ o2 = otmp; t2 = 12;}
						else if(otmp->oclass == CORPSE && is_andromaliable(&mons[otmp->corpsenm]) && t1 != 13 && otmp != o1){ o2 = otmp; t2 = 13;}
						else if(otmp->oclass == SPBOOK_CLASS && otmp->oclass != o1->oclass){ o2 = otmp; t2 = 14;}
						else if(otmp->otyp == BELL && otmp->otyp != o1->otyp){ o2 = otmp; t2 = 15;}
						else if(otmp->otyp == LOCK_PICK && otmp->otyp != o1->otyp){ o2 = otmp; t2 = 16;}
						//live sewer rat
					}
					else break;
				}
			if(o1 && o2){
				int i1 = rn2(18), i2 = rn2(18), i3 = rn2(18);
				
				while(i1 == t1 || i1 == t2) i1 = rn2(18);
				while(i2 == t1 || i2 == t2 || i2 == i1) i2 = rn2(18);
				while(i3 == t1 || i3 == t2 || i3 == i1 || i3 == i2) i3 = rn2(18);
				
				pline("Gloved hands reach down and pick up %s and %s from the confines of the seal.", andromaliusItems[t1], andromaliusItems[t2]);
				pline("The hands begin to juggle. They move faster and faster, adding new objects as they go.");
				pline("You spot %s and %s before loosing track of the individual objects.",andromaliusItems[i1],andromaliusItems[i2]);
				if(u.sealCounts < numSlots){
					pline("Suddenly, the hands toss one of the whrilling objects to you.");
					/*make object here*/
					pline("When your attention returns to the seal, the hands have gone.");
					u.sealsActive |= SEAL_ANDROMALIUS;
					u.spirit[numSlots] = SEAL_ANDROMALIUS;
					u.spiritT[numSlots] = moves + bindingPeriod;
					u.sealCounts++;
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("Suddenly, the hands toss one of the whrilling objects to you.");
					pline("It's the Pen of the Void. You didn't notice it had been taken!");
					uwep->ovar1 |= SEAL_ANDROMALIUS;
					if(!u.spiritTineA){
						u.spiritTineA = SEAL_ANDROMALIUS;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_ANDROMALIUS;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("Suddenly, the hands toss one of the whrilling objects at you.");
					pline("The %s passes far over your head, out of reach.", andromaliusItems[i3]);
					pline("When your attention returns to the seal, the hands have gone.");
				}
				u.andrealphus = moves + bindingPeriod;
				//coins: use up just one.
				useup(o1);
				useup(o2);
			}
		}
	}break;
	case ASTAROTH:{
		if(u.astaroth < moves){
			struct obj *o = 0, *otmp;
			for(otmp = level.objects[tx][ty]; otmp; otmp = otmp->nexthere){
				if(otmp->spe < 0 || otmp->oeroded || otmp->oeroded2){
					o = otmp;
					otmp = 0; //breaks out of loop.
				}
			}
			if(o && u.sealCounts < numSlots){ //Astaroth requires that his seal be drawn on a square with a damaged item.
				pline("A hand of worn and broken clockwork on a rusted metal arm reaches into the seal.");
				pline("The hand gently touches the %s %s, then rests on the seal's surface as its unseen owner shifts his weight onto that arm.");
				pline("There is the sound of shrieking metal, and a cracked porcelain face swings into view on a metalic armature.");
				pline("A voice speaks to you, as the immobile white face weeps tears of black oil onto the %s.");
				pline("*I am Astaroth, the Clockmaker. You shall be my instrument, to repair this broken world.*");
					u.sealsActive |= SEAL_ASTAROTH;
					u.spirit[numSlots] = SEAL_ASTAROTH;
					u.spiritT[numSlots] = moves + bindingPeriod;
					u.sealCounts++;
				if(o->spe<0) o->spe=0;
				if(o->oeroded) o->oeroded=0;
				if(o->oeroded2) o->oeroded2=0;
				}
			else if(uwep && (uwep->spe<0 || uwep->oeroded || uwep->oeroded2) && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
				pline("A hand of worn and broken clockwork on a rusted metal arm reaches into the seal.");
				pline("The hand slowly reaches out towards you, then rests on the seal's surface as its unseen owner shifts his weight onto that arm.");
				pline("There is the sound of shrieking metal, and a cracked porcelain face swings into view on a metalic armature.");
				pline("A voice speaks to you, as the immobile white face studies you and weeps tears of black oil.");
				pline("*I am Astaroth, the Clockmaker. You shall hold my instrument, to repair this broken world.*");
					uwep->ovar1 |= SEAL_ASTAROTH;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_ASTAROTH;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_ASTAROTH;
						u.spiritTineTB= moves + bindingPeriod;
					}
				pline("The hand catches a teardrop and anoints the Pen of the Void with the glistening oil.");
				if(uwep->spe<0) uwep->spe=0;
				if(uwep->oeroded) uwep->oeroded=0;
				if(uwep->oeroded2) uwep->oeroded2=0;
				}
			else if(o || (uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (uwep->spe<0 || uwep->oeroded || uwep->oeroded2))){
				pline("Black oil falls like teardrops into the seal.");
				if(o){
					if(o->spe<0) o->spe++;
					if(o->oeroded) o->oeroded--;
					if(o->oeroded2) o->oeroded2--;
					pline("But nothing else occurs.");
				}
				else{
					if(uwep->spe<0) uwep->spe++;
					if(uwep->oeroded) uwep->oeroded--;
					if(uwep->oeroded2) uwep->oeroded2--;
					pline("The Pen of the Void drips black oil, as if in sympathy.");
				}
				}
				u.astaroth = moves + bindingPeriod;
			}
	}break;
	case BALAM:{
		if(u.balam < moves){
			if(levl[tx][ty].typ == ICE){ //Balam requires that her seal be drawn on an icy square.
				You("stab your weapon down into the ice, cracking it.");
				if(u.sealCounts < numSlots){
					pline("A woman's scream echos through your mind as the cracks form a vaguely humanoid outline on the ice.");
					pline("A voice sobs in your ear:");
					pline("\"I am Balam, offered up as the last sacrifice; condemned to bleed until the end of all suffering.\"");
					pline("\"In your name was this done, therefore you shall bear my stigmata and share my suffering.\"");
					u.sealsActive |= SEAL_BALAM;
					u.spirit[numSlots] = SEAL_BALAM;
					u.spiritT[numSlots] = moves + bindingPeriod;
					u.sealCounts++;
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("A woman's scream echos through your mind as the cracks form a vaguely humanoid outline on the ice.");
					pline("A voice sobs from under the ice:");
					pline("\"I am Balam, offered up as the last sacrifice; condemned to bleed until the end of all suffering.\"");
					pline("\"By your hand was this done, therefore you shall be stained by my blood.\"");
					uwep->ovar1 |= SEAL_BALAM;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_BALAM;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_BALAM;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("A woman's scream drifts through your mind, and the cracks describe a vaguely humanoid outline on the ice.");
					pline("But nothing else occurs....");
				}
				u.balam = moves + bindingPeriod;
			}
		}
	}break;
	case BERITH:{
		static int slvring = 0;
		if (!slvring) slvring = find_silver_ring();
		if(u.berith < moves){
			struct obj *o = 0, *otmp;
	//Berith requires that his seal be drawn around a set of riding gloves, riding boots, a saddle, a saber, a longsword, a bow, or a lance.
			for(otmp = level.objects[tx][ty]; otmp; otmp = otmp->nexthere){
				if(is_berithable(otmp)){
					o = otmp;
					otmp = 0; //breaks out of loop.
				}
			}
	//Berith further requires that the summoner wear a blessed silver ring on his or her left hand.
			if(o && uleft && uleft->otyp == slvring && uleft->blessed){
				pline("Gold rains down within the circumference of the seal, melting slowly to blood where it lands.");
				pline("A figure takes form within the showering gold, staring down at you from a crimson horse.");
				pline("His crown is gold, and his clothes are red like blood.");
				pline("\"I am Berith, %s.",rn2(2) ? "war-leader of the forgotten" : "god of the covenant of blood");
				if(u.sealCounts < numSlots){
					pline("I anoint you in Blood and Gold, that bloodshed and riches shall follow in your wake.");
					pline("That is my covenant, my blessing, and my curse.\"");
					u.sealsActive |= SEAL_BERITH;
					u.spirit[numSlots] = SEAL_BERITH;
					u.spiritT[numSlots] = moves + bindingPeriod;
					u.sealCounts++;
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("I anoint your blade with Blood, for blood calls to blood.");
					pline("That is the covenant and curse of Berith.\"");
					uwep->ovar1 |= SEAL_BERITH;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_BERITH;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_BERITH;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					You("think you can hear faint hoofbeats from within the seal.");
					pline("But they fade away before you can be sure.");
				}
				u.berith = moves + bindingPeriod;
			}
		}
	}break;
	case BUER:{
		if(u.buer < moves){
		//Buer's seal may be drawn anywhere.
			pline("You hear hooved footfalls approaching quickly, though you can’t make out from what direction.");
			pline("They set an odd tempo; very regular and faster by far than any animal of four legs could comfortably keep.");
			pline("The footfalls reach a crescendo, and an odd creature rolls into the seal in front of you.");
			pline("The creature’s five legs are arranged in a star pattern, and to move it rolls from foot to foot.");
			pline("At the center of the wheel is a lion’s head, complete with a glorious mane.");
			pline("The creature speaks to you; and it’s voice, though deep, is clearly that of a woman.");
			pline("\"I am Buer, %s, %s to %s.", buerTitles[rn2(SIZE(buerTitles))], buerSetOne[rn2(SIZE(buerSetOne))], buerSetOne[rn2(SIZE(buerSetTwo))]);
			if(u.sealCounts < numSlots){
				pline("Will you walk with me?\"");
				u.sealsActive |= SEAL_BUER;
				u.spirit[numSlots] = SEAL_BUER;
				u.spiritT[numSlots] = moves + bindingPeriod;
				u.sealCounts++;
			}
			else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
				pline("I will walk beside you.\"");
				uwep->ovar1 |= SEAL_BUER;
				if(!u.spiritTineA){ 
					u.spiritTineA = SEAL_BUER;
					u.spiritTineTA= moves + bindingPeriod;
				}
				else{
					u.spiritTineB = SEAL_BUER;
					u.spiritTineTB= moves + bindingPeriod;
				}
			}
			else{
				pline("I wish you well as you walk your path.\"");
			}
			u.buer = moves + bindingPeriod;
		}
	}break;
	case CHUPOCLOPS:{
		// if(u.vestige < moves){
			// if(){ //Spirit requires that his seal be drawn .
				// Your(".");
				// pline(".");
				// if(u.sealCounts < numSlots){
					// pline("");
					// pline("");
					// u.sealsActive |= SEAL_;
					// u.spirit[numSlots] = SEAL_;
					// u.spiritT[numSlots] = moves + bindingPeriod;
					// u.sealCounts++;
				// }
				// else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					// pline("");
					// pline("");
					// uwep->ovar1 |= SEAL_;
					// if(!u.spiritTineA){ 
						// u.spiritTineA = SEAL_;
						// u.spiritTineTA= moves + bindingPeriod;
					// }
					// else{
						// u.spiritTineB = SEAL_;
						// u.spiritTineTB= moves + bindingPeriod;
					// }
				// }
				// else{
					// pline("");
					// pline(".");
				// }
				// u.vestige = moves + bindingPeriod;
			// }
		// }
	}break;
	case DANTALION:{
	}break;
	case DUNSTAN:{
	}break;
	case ECHIDNA:{
	}break;
	case EDEN:{
	}break;
	case ERIDU:{
	}break;
	case EURYNOME:{
	}break;
	case EVE:{
	}break;
	case FAFNIR:{
	}break;
	case HUGINN_MUNINN:{
	}break;
	case IRIS:{
	}break;
	case JACK:{
	}break;
	case MALPHAS:{
	}break;
	case MARIONETTE:{
	}break;
	case MOTHER:{
	}break;
	case NABERIUS:{
	}break;
	case ORTHOS:{
	}break;
	case OSE:{
	}break;
	case OTIAX:{
	}break;
	case PAIMON:{
	}break;
	case SIMURGH:{
	}break;
	case TENEBROUS:{
	}break;
	case YMIR:{
	}break;
	case DAHLVER_NAR:{
	}break;
	case ACERERAK:{
	}break;
	case NUMINA:{
	}break;
	}
	
}
#ifdef USER_SOUNDS

extern void FDECL(play_usersound, (const char*, int));

typedef struct audio_mapping_rec {
#ifdef USER_SOUNDS_REGEX
	struct re_pattern_buffer regex;
#else
	char *pattern;
#endif
	char *filename;
	int volume;
	struct audio_mapping_rec *next;
} audio_mapping;

static audio_mapping *soundmap = 0;

char* sounddir = ".";

/* adds a sound file mapping, returns 0 on failure, 1 on success */
int
add_sound_mapping(mapping)
const char *mapping;
{
	char text[256];
	char filename[256];
	char filespec[256];
	int volume;

	if (sscanf(mapping, "MESG \"%255[^\"]\"%*[\t ]\"%255[^\"]\" %d",
		   text, filename, &volume) == 3) {
	    const char *err;
	    audio_mapping *new_map;

	    if (strlen(sounddir) + strlen(filename) > 254) {
		raw_print("sound file name too long");
		return 0;
	    }
	    Sprintf(filespec, "%s/%s", sounddir, filename);

	    if (can_read_file(filespec)) {
		new_map = (audio_mapping *)alloc(sizeof(audio_mapping));
#ifdef USER_SOUNDS_REGEX
		new_map->regex.translate = 0;
		new_map->regex.fastmap = 0;
		new_map->regex.buffer = 0;
		new_map->regex.allocated = 0;
		new_map->regex.regs_allocated = REGS_FIXED;
#else
		new_map->pattern = (char *)alloc(strlen(text) + 1);
		Strcpy(new_map->pattern, text);
#endif
		new_map->filename = strdup(filespec);
		new_map->volume = volume;
		new_map->next = soundmap;

#ifdef USER_SOUNDS_REGEX
		err = re_compile_pattern(text, strlen(text), &new_map->regex);
#else
		err = 0;
#endif
		if (err) {
		    raw_print(err);
		    free(new_map->filename);
		    free(new_map);
		    return 0;
		} else {
		    soundmap = new_map;
		}
	    } else {
		Sprintf(text, "cannot read %.243s", filespec);
		raw_print(text);
		return 0;
	    }
	} else {
	    raw_print("syntax error in SOUND");
	    return 0;
	}

	return 1;
}

void
play_sound_for_message(msg)
const char* msg;
{
	audio_mapping* cursor = soundmap;

	while (cursor) {
#ifdef USER_SOUNDS_REGEX
	    if (re_search(&cursor->regex, msg, strlen(msg), 0, 9999, 0) >= 0) {
#else
	    if (pmatch(cursor->pattern, msg)) {
#endif
		play_usersound(cursor->filename, cursor->volume);
	    }
	    cursor = cursor->next;
	}
}

#endif /* USER_SOUNDS */

#endif /* OVLB */

/*sounds.c*/
