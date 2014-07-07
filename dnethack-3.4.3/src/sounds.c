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

static const char *FDECL(DantalionRace,(int));
static int FDECL(domonnoise,(struct monst *));
int FDECL(dobinding,(int, int));
static int NDECL(dochat);

static const char tools[] = { TOOL_CLASS, 0 };

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

static const char *echidnaTitles[] = {
	"the She Viper",
	"mate of Typhon",
	"daughter of Tartarus and Gaia",
	"daughter of hell and earth",
	"enemy of the Gods",
	"grim and ageless",
	"guardian of Arima",
	"Mother of Monsters"
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
    "ululate", "pop", "jingle", "sniffle", "tinkle", "eep",
    "clatter", "hum", "sizzle", "twitter", "wheeze", "rustle",
    "honk", "lisp", "yodel", "coo", "burp", "moo", "boom",
    "murmur", "oink", "quack", "rumble", "twang", "bellow",
    "toot", "gargle", "hoot", "warble"
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
	if(flags.run) nomul(0, NULL);
	wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 18);
    }
}

/* the sounds of mistreated pets */
void
yelp(mtmp)
register struct monst *mtmp;
{
    register const char *yelp_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->mnotlaugh || !mtmp->data->msound)
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
	if(flags.run) nomul(0, NULL);
	wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 12);
    }
}

/* the sounds of distressed pets */
void
whimper(mtmp)
register struct monst *mtmp;
{
    register const char *whimper_verb = 0;

    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->mnotlaugh || !mtmp->data->msound)
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
	if(flags.run) nomul(0, NULL);
	wake_nearto(mtmp->mx, mtmp->my, mtmp->data->mlevel * 6);
    }
}

/* pet makes "I'm hungry" noises */
void
beg(mtmp)
register struct monst *mtmp;
{
    if (mtmp->msleeping || !mtmp->mcanmove || !mtmp->mnotlaugh ||
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
	char class_list[MAXOCLASSES+2];

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
asGuardian:
	    if (mtmp->mpeaceful && uclockwork && yn("(Ask for help winding your clockwork?)") == 'y'){
			struct obj *key;
			int turns = 0;
			
			Strcpy(class_list, tools);
			key = getobj(class_list, "wind with");
			if (!key){
				pline(Never_mind);
				break;
			}
			turns = ask_turns(mtmp, 0, 0);
			if(!turns){
				pline(Never_mind);
				break;
			}
			start_clockwinding(key, mtmp, turns);
			break;
		}
	    quest_chat(mtmp);
	    break;
	case MS_SELL: /* pitch, pay, total */
	    shk_chat(mtmp);
	    break;
	case MS_VAMPIRE:
	    {
	    /* vampire messages are varied by tameness, peacefulness, and time of night */
		boolean isnight = night();
		boolean kindred = maybe_polyd(u.umonnum == PM_VAMPIRE ||
				    u.umonnum == PM_VAMPIRE_LORD,
				    /* DEFERRED u.umonnum == PM_VAMPIRE_MAGE, */
				    Race_if(PM_VAMPIRE));
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
	case MS_JUBJUB:{
		struct monst *tmpm;
		if(!(mtmp->mspec_used)){
		    pline_msg = "screams high and shrill.";
				mtmp->mspec_used = 10;
			for(tmpm = fmon; tmpm; tmpm = tmpm->nmon){
				if(tmpm != mtmp){
					if(tmpm->mtame && tmpm->mtame<20) tmpm->mtame++;
					if(d(1,tmpm->mhp) < mtmp->mhpmax){
						tmpm->mflee = 1;
					}
				}
			}
			make_stunned(HStun + mtmp->mhp/10, TRUE);
		}
	}break;
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
	    nomul(-2, "scared by rattling bones");
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
		if(Role_if(PM_NOBLEMAN) && 
			(mtmp->data == &mons[PM_KNIGHT] 
				|| mtmp->data == &mons[PM_MAID]) && 
			mtmp->mpeaceful
		) goto asGuardian; /* Jump up to a different case in this switch statment */
		else if(Role_if(PM_KNIGHT) && 
			mtmp->data == &mons[PM_KNIGHT] && 
			mtmp->mpeaceful
		) goto asGuardian; /* Jump up to a different case in this switch statment */
		else if(Role_if(PM_EXILE) && 
			mtmp->data == &mons[PM_PEASANT] && 
			mtmp->mpeaceful
		) goto asGuardian; /* Jump up to a different case in this switch statment */

	    if (!mtmp->mpeaceful) {
		if (In_endgame(&u.uz) && is_mplayer(ptr)) {
		    mplayer_talk(mtmp);
		    break;
		} else return 0;	/* no sound */
	    }
		
	    /* Generic peaceful humanoid behaviour. */
	    if (mtmp->mpeaceful && uclockwork && yn("(Ask for help winding your clockwork?)") == 'y'){
			struct obj *key;
			int turns = 0;
			
			Strcpy(class_list, tools);
			key = getobj(class_list, "wind with");
			if (!key){
				pline(Never_mind);
				break;
			}
			if(!mtmp->mtame) turns = ask_turns(mtmp, u.ulevel*10 + 100, u.ulevel);
			else turns = ask_turns(mtmp, 0, 0);
			if(!turns){
				pline(Never_mind);
				break;
			}
			start_clockwinding(key, mtmp, turns);
			break;
		}
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
	    if (mtmp->mpeaceful && uclockwork && yn("(Ask for help winding your clockwork?)") == 'y'){
			struct obj *key;
			int turns = 0;
			
			Strcpy(class_list, tools);
			key = getobj(class_list, "wind with");
			if (!key){
				pline(Never_mind);
				break;
			}
			if(!mtmp->mtame) turns = ask_turns(mtmp, u.ulevel*11 +111, u.ulevel*1.1);
			else turns = ask_turns(mtmp, 0, 0);
			if(!turns){
				pline(Never_mind);
				break;
			}
			start_clockwinding(key, mtmp, turns);
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
	    if (mtmp->mpeaceful) {
			if(uclockwork && yn("(Ask for help winding your clockwork?)") == 'y'){
				struct obj *key;
				int turns = 0;
				
				Strcpy(class_list, tools);
				key = getobj(class_list, "wind with");
				if (!key){
					pline(Never_mind);
					break;
				}
				if(!mtmp->mtame) turns = ask_turns(mtmp, u.ulevel*20, u.ulevel*5);
				else turns = ask_turns(mtmp, 0, 0);
				if(!turns){
					pline(Never_mind);
					break;
				}
				start_clockwinding(key, mtmp, turns);
				break;
			} else if(!mtmp->mtame) (void) demon_talk(mtmp);
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
	    if (mtmp->mpeaceful && uclockwork && yn("(Ask for help winding your clockwork?)") == 'y'){
			struct obj *key;
			int turns = 0;
			
			Strcpy(class_list, tools);
			key = getobj(class_list, "wind with");
			if (!key){
				pline(Never_mind);
				break;
			}
			if(!mtmp->mtame) turns = ask_turns(mtmp, 0, u.ulevel*.5);
			else turns = ask_turns(mtmp, 0, 0);
			if(!turns){
				pline(Never_mind);
				break;
			}
			start_clockwinding(key, mtmp, turns);
			break;
		}
	    if (uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep)))
		verbl_msg = "Put that weapon away before you hurt someone!";
	    else verbl_msg = "Relax, this won't hurt a bit.";
	    // else if (uarmc || uarm || uarmh || uarms || uarmg || uarmf)
		// verbl_msg = Role_if(PM_HEALER) ?
			  // "Doc, I can't help you unless you cooperate." :
			  // "Please undress so I can examine you.";
// #ifdef TOURIST
	    // else if (uarmu)
		// verbl_msg = "Take off your shirt, please.";
// #endif
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
	    if (mtmp->mpeaceful && uclockwork && yn("(Ask for help winding your clockwork?)") == 'y'){
			struct obj *key;
			int turns = 0;
			
			Strcpy(class_list, tools);
			key = getobj(class_list, "wind with");
			if (!key){
				pline(Never_mind);
				break;
			}
			if(!mtmp->mtame) turns = ask_turns(mtmp, u.ulevel*20+200, 0);
			else turns = ask_turns(mtmp, 0, 0);
			if(!turns){
				pline(Never_mind);
				break;
			}
			start_clockwinding(key, mtmp, turns);
			break;
		}
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
    register int tx,ty,bindresult;
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
		struct engr *ep = get_head_engr();
		for(ep;ep;ep=ep->nxt_engr) 
			if(ep->engr_x==u.ux && ep->engr_y==u.uy)
				break;//else continue
		if(!ep || ep->halu_ward || ep->ward_id < FIRST_SEAL) pline("They won't hear you %s there.", u.dz < 0 ? "up" : "down");
		else pline("The gate won't open with you standing on the seal!");
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

	if(bindresult = dobinding(tx,ty)) return bindresult;

	if((otmp = level.objects[tx][ty]) && otmp->otyp == CORPSE && !mindless(&mons[otmp->corpsenm]) && !nohands(&mons[otmp->corpsenm])){
		You("speak to the shadow that dwells within this corpse.");
		if(otmp->ovar1 < moves){
			outrumor(rn2(2), BY_OTHER);
			otmp->ovar1 = moves + rnz(100);
		}
		else pline("....");
	}
	
    if (!mtmp || mtmp->mundetected ||
		mtmp->m_ap_type == M_AP_FURNITURE ||
		mtmp->m_ap_type == M_AP_OBJECT) return 0;

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

    /* laughing monsters can't talk */
    if (!mtmp->mnotlaugh) {
		if (!is_silent(mtmp->data)) pline("%s laughs hysterically", Monnam(mtmp));
		return(0);
    }
	
    if (mtmp->mtame && mtmp->mnotlaugh && mtmp->meating) {
		if (!canspotmon(mtmp))
			map_invisible(mtmp->mx, mtmp->my);
		pline("%s is eating noisily.", Monnam(mtmp));
		return (0);
    }
	
	if(mtmp->data == &mons[PM_NIGHTGAUNT] && u.umonnum == PM_GHOUL){
		You("bark the secret passwords known to ghouls.");
		mtmp->mpeaceful = 1;
		mtmp = tamedog(mtmp, (struct obj *)0);
		return 1;
	}
	if(is_undead(mtmp->data) && u.specialSealsActive&SEAL_ACERERAK && u.ulevel > mtmp->m_lev){
		You("order the lesser dead to stand at ease.");
		mtmp->mpeaceful = 1;
		mtmp->mhp = mtmp->mhpmax;
		return 1;
	}
    /* That is IT. EVERYBODY OUT. You are DEAD SERIOUS. */
    if (mtmp->data == &mons[PM_URANIUM_IMP]) {
		monflee(mtmp, rn1(20,10), TRUE, FALSE);
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

//definition of externs in you.h
long wis_spirits = SEAL_AMON|SEAL_BUER|SEAL_MOTHER|SEAL_SIMURGH;
long int_spirits = SEAL_ANDREALPHUS|SEAL_NABERIUS|SEAL_OSE|SEAL_PAIMON;

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
static const int androCorpses[] = {
	PM_ELF,
	PM_DWARF,
	PM_GNOME,
	PM_ORC,
	PM_HUMAN,
	PM_HOBBIT,
	PM_DEEP_ONE,
	PM_MONKEY,
	PM_APE,
	PM_YETI,
	PM_CARNIVOROUS_APE,
	PM_SASQUATCH
};

int
dobinding(tx,ty)
int tx,ty;
{
	struct engr *ep = get_head_engr();
	int numSlots, i;
	int bindingPeriod = 5000;
	for(ep;ep;ep=ep->nxt_engr) 
		if(ep->engr_x==tx && ep->engr_y==ty)
			break;//else continue
	if(!(ep)) return 0; //no engraving found
	if(ep->halu_ward || ep->ward_id < FIRST_SEAL) return 0;
	else if(ep->complete_wards < 1){
		pline("The seal has been damaged.");
		return 0;
	} else if(ep->engr_time+5 < moves){
		pline("The seal is too old.");
		return 0;
	}
	
	if(Role_if(PM_EXILE)){
	if(u.ulevel <= 2) numSlots=1;
	else if(u.ulevel <= 9) numSlots=2;
	else if(u.ulevel <= 18) numSlots=3;
	else if(u.ulevel <= 25) numSlots=4;
	else numSlots=5;
	} else {
		numSlots=1;
	}
	
	if(m_at(tx,ty) && (ep->ward_id != ANDROMALIUS || m_at(tx,ty)->data != &mons[PM_SEWER_RAT])) return 0;
	
	switch(ep->ward_id){
	case AHAZU:{
		if(u.ahazu < moves){
			struct trap *t=t_at(tx,ty);
			//Ahazu requires that his seal be drawn in a pit.
			if(t && t->ttyp == PIT){
				pline("The walls of the pit are lifted swiftly away, revealing a vast starry expanse beneath the world.");
				if(u.sealCounts < numSlots){
					pline("A voice whispers from bellow:");
					pline("\"All shall feed the shattered night.\"");
					bindspirit(ep->ward_id);
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
			} else{
				pline("Thoughts of falling and of narrow skys come unbidden into your mind.");
				u.ahazu = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case AMON:{
		if(u.amon < moves){
			int curx, cury;
			char altarfound=0;
			//Amon can't be invoked on levels with altars, and in fact doing so causes imediate level loss, as for a broken taboo.
			for(curx=1;curx < COLNO;curx++)
				for(cury=1;cury < ROWNO;cury++)
					if(IS_ALTAR(levl[curx][cury].typ)){ altarfound=1; cury=ROWNO; curx=COLNO;}//end search
			
			if(!altarfound){
				pline("A golden flame roars suddenly to life within the seal, throwning the world into a stark relief of hard-edged shadows and brilliant light.");
				if(u.sealCounts < numSlots){
					pline("No sooner are the shadows born than they rise up against their creator, smothering the flame under a tide of darkness.");
					pline("Even as it dies, a voice speaks from the blood-red flame:");
					pline("\"Cursed are you who calls me forth. I damn you to bear my sign and my flames, alone in this world of darkness!\"");
					bindspirit(ep->ward_id);
					vision_full_recalc = 1; //can now see perfectly in the dark
					doredraw();
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
				Your("mind's eye is blinded by a flame blasting through an altar.");
				losexp("shredding of the soul",TRUE,TRUE,TRUE);
				if(in_rooms(tx, ty, TEMPLE)){
//					struct monst *priest = findpriest(roomno);
					//invoking Amon inside a temple angers the resident deity
					altar_wrath(tx, ty);
					angrygods(a_align(tx,ty));
				}
			}
			u.amon = moves + bindingPeriod; // invoking amon on a level with an altar still triggers the binding period.
		} else pline("You can't feel the spirit.");
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
					bindspirit(ep->ward_id);
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
			} else{
				pline("Thoughts intersecting lines rise to the forefront of your mind.");
				u.andrealphus = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case ANDROMALIUS:{ /*UNFINISHED*/
		//Seal must be drawn around any two of a bag, a silver key, a gold ring, (a pair of dice), a (copper) coin, a dagger, an apple, a scroll, (a comb), a whistle, a mirror, an egg, a potion, a dead spider, (an oak leaf), a dead human (skull and arm bone), (a lock), (a closed black book) a spellbook, a bell, (a (live?) dove), a set of lockpicks, or a live? sewer rat (mouse). The items are consumed.
		if(u.andromalius < moves){
			struct obj *o1 = 0, *o2 = 0, *otmp;
			struct monst *rat = 0;
			int count = 0;
			int t1, t2;
			static int gldring = 0;
			if (!gldring) gldring = find_gold_ring();
			
			rat = m_at(tx,ty);
			if(rat && rat->data == &mons[PM_SEWER_RAT]) t2=17;
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
					}
					else if(!o2 && !rat){
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
					}
					else break;
				}
			if((o2 || rat) && o1){
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
					switch(i3){
						case 0:
							otmp = mksobj(SACK, TRUE, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 1:
							otmp = mksobj(UNIVERSAL_KEY, TRUE, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 2:
							otmp = mksobj(gldring, TRUE, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = TRUE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 3://Coin
							otmp = mkobj(COIN_CLASS, FALSE);
							otmp->quan = 1;
							otmp->owt = weight(otmp);
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 4:
							otmp = mksobj(DAGGER, TRUE, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 5:
							otmp = mksobj(APPLE, TRUE, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 6:
							otmp = mkobj(SCROLL_CLASS, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 7:
							otmp = mksobj(TIN_WHISTLE, TRUE, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 8:
							otmp = mksobj(MIRROR, TRUE, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 9:
							otmp = mksobj(EGG, TRUE, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 10:
							otmp = mkobj(POTION_CLASS, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 11:
							otmp = mksobj(CORPSE, TRUE, FALSE);
							otmp->corpsenm = PM_CAVE_SPIDER;
							otmp->owt = weight(otmp);
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 12:
							otmp = mksobj(CORPSE, TRUE, FALSE);
							otmp->corpsenm = urace.malenum;
							otmp->oeaten = mons[otmp->corpsenm].cnutrit;
							consume_oeaten(otmp, 1);
							otmp->owt = weight(otmp);
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 13:
							otmp = mksobj(CORPSE, TRUE, FALSE);
							otmp->corpsenm = androCorpses[SIZE(androCorpses)];
							otmp->oeaten = mons[otmp->corpsenm].cnutrit;
							consume_oeaten(otmp, 1);
							otmp->owt = weight(otmp);
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 14:
							otmp = mkobj(SPBOOK_CLASS, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = TRUE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 15:
							otmp = mksobj(BELL, TRUE, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 16:
							otmp = mksobj(LOCK_PICK, TRUE, FALSE);
							otmp->blessed = FALSE;
							otmp->cursed = FALSE;
							hold_another_object(otmp, "You drop %s!",
								doname(otmp), (const char *)0);
						break;
						case 17:
							rat = makemon(&mons[PM_SEWER_RAT], u.ux, u.uy, MM_EDOG|MM_ADJACENTOK|NO_MINVENT|MM_NOCOUNTBIRTH);
							initedog(rat);
							rat->mtame = 10;
							rat->mpeaceful = 1;
							pline("A startled-looking rat lands in your %s, then leaps to the floor.",makeplural(body_part(HAND)));
						break;
					}
					pline("When your attention returns to the seal, the hands have gone.");
					bindspirit(ep->ward_id);
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
					You("see %s pass far over your %s, out of reach.", andromaliusItems[i3], body_part(HEAD));
					pline("When your attention returns to the seal, the hands have gone.");
				}
				u.andromalius = moves + bindingPeriod;
				if(o1){
					if(o1->quan > 1) o1->quan--; 
					else{
						obj_extract_self(o1);
						obfree(o1, (struct obj *)0);
						o1 = (struct obj *) 0;
						// useup(o1);
					}
				}
				if(rat) mongone(rat);
				if(o2){
					if(o2->quan > 1) o2->quan--; 
					else{
						obj_extract_self(o2);
						obfree(o2, (struct obj *)0);
						o2 = (struct obj *) 0;
						// useup(o2);
					}
				}
			} else{
				int i1 = rn2(18), i2 = rn2(18);				
				while(i1 == i2) i2 = rn2(18);
				pline("The image of a pair of gloved hands comes unbidden to the forefront of your mind.");
				pline("With your mind's eye, you watch as they make a show of prestidigitation,");
				pline("palming and unpalming %s. Suddenly, they throw %s at your face!", andromaliusItems[i1], andromaliusItems[i2]);
				You("come out of your revere with a start.");
				u.andromalius = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case ASTAROTH:{
		if(u.astaroth < moves){
			struct obj *o = 0, *otmp;
			char prefix[32]; //thoroughly 11, corroded 9
			boolean iscrys;
			prefix[0] = '\0';
			for(otmp = level.objects[tx][ty]; otmp; otmp = otmp->nexthere){
				if(otmp->spe < 0 || otmp->oeroded || otmp->oeroded2){
					o = otmp;
			break;
				}
			}
			//Astaroth requires that his seal be drawn on a square with a damaged item.
			if(o && u.sealCounts < numSlots){
				iscrys = (o->otyp == CRYSKNIFE);
				if (o->oeroded && !iscrys) {
					switch (o->oeroded) {
						case 2:	Strcat(prefix, "very "); break;
						case 3:	Strcat(prefix, "thoroughly "); break;
					}			
					Strcat(prefix, is_rustprone(o) ? "rusty " : "burnt ");
				}
				if (o->oeroded2 && !iscrys) {
					switch (o->oeroded2) {
						case 2:	Strcat(prefix, "very "); break;
						case 3:	Strcat(prefix, "thoroughly "); break;
					}			
					Strcat(prefix, is_corrodeable(o) ? "corroded " :
						"rotted ");
				}
				pline("A hand of worn and broken clockwork on a rusted metal arm reaches into the seal.");
				pline("The hand gently touches the %s%s, then rests on the seal's surface as its unseen owner shifts his weight onto that arm.", prefix, xname(o));
				pline("There is the sound of shrieking metal, and a cracked porcelain face swings into view on a metalic armature.");
				pline("A voice speaks to you, as the immobile white face weeps tears of black oil onto the %s.", surface(tx,ty));
				pline("*I am Astaroth, the Clockmaker. You shall be my instrument, to repair this broken world.*");
				bindspirit(ep->ward_id);
				if(o->spe<0) o->spe=0;
				if(o->oeroded) o->oeroded=0;
				if(o->oeroded2) o->oeroded2=0;
				}
			else if(uwep && (uwep->spe<0 || uwep->oeroded || uwep->oeroded2) && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
				pline("A hand of worn and broken clockwork on a rusted metal arm reaches into the seal.");
				pline("The hand slowly stretches out towards you, then rests on the seal's surface as its unseen owner shifts his weight onto that arm.");
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
				u.astaroth = moves + bindingPeriod;
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
				u.astaroth = moves + bindingPeriod;
			} else{
				pline("You think of all the loyal items used up and thrown away each day, and shed a tear.");
				u.astaroth = moves + bindingPeriod/10;
				}
		} else pline("You can't feel the spirit.");
	}break;
	case BALAM:{
		if(u.balam < moves){
			//Balam requires that her seal be drawn on an icy square.
			if(levl[tx][ty].typ == ICE){
				You("stab your weapon down into the ice, cracking it.");
				if(u.sealCounts < numSlots){
					if(!Blind){
					pline("A woman's scream echos through your mind as the cracks form a vaguely humanoid outline on the ice.");
					pline("A voice sobs in your ear:");
					} else {
						pline("A woman's scream echos through your mind, then a voice sobs in your ear:");
					}
					pline("\"I am Balam, offered up as the last sacrifice; condemned to bleed until the end of all suffering.\"");
					pline("\"In your name was this done, therefore you shall bear my stigmata and share my suffering.\"");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(!Blind){
					pline("A woman's scream echos through your mind as the cracks form a vaguely humanoid outline on the ice.");
					pline("A voice sobs from under the ice:");
					} else {
						pline("A woman's scream echos through your mind, then a voice sobs from under the ice:");
					}
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
					if(!Blind){
					pline("A woman's scream drifts through your mind, and the cracks describe a vaguely humanoid outline on the ice.");
					pline("But nothing else occurs....");
					} else {
						pline("A woman's scream drifts through your mind, but nothing else occurs....");
					}
				}
				u.balam = moves + bindingPeriod;
			} else{
				You("shiver violently.");
				u.balam = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
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
			break;
				}
			}
	//Berith further requires that the summoner wear a blessed silver ring on his or her left hand.
			if(o && uleft && uleft->otyp == slvring && uleft->blessed){
				if(!Blind){
				pline("Gold rains down within the circumference of the seal, melting slowly to blood where it lands.");
				pline("A figure takes form within the showering gold, staring down at you from a crimson horse.");
				pline("His crown is gold, and his clothes are red like blood.");
				}
				pline("\"I am Berith, %s.",rn2(2) ? "war-leader of the forgotten" : "god of the covenant of blood");
				if(u.sealCounts < numSlots){
					pline("I anoint you in Blood and Gold, that bloodshed and riches shall follow in your wake.");
					pline("That is my covenant, my blessing, and my curse.\"");
					bindspirit(ep->ward_id);
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
			} else{
				You("think of cavalry and silver rings.");
				u.berith = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case BUER:{
		//Buer's seal may be drawn anywhere.
		if(u.buer < moves){
			pline("You hear hooved footfalls approaching quickly, though you can't make out from what direction.");
			pline("They set an odd tempo; very regular and faster by far than any animal of four legs could comfortably keep.");
			if(!Blind){
			pline("The footfalls reach a crescendo, and an odd creature rolls into the seal in front of you.");
			pline("The creature's five legs are arranged in a star pattern, and to move it rolls from foot to foot.");
			pline("At the center of the wheel is a lion's head, complete with a glorious mane.");
			}
			pline("The creature speaks to you; and it's voice, though deep, is clearly that of a woman.");
			pline("\"I am Buer, %s, %s to %s.", buerTitles[rn2(SIZE(buerTitles))], buerSetOne[rn2(SIZE(buerSetOne))], buerSetTwo[rn2(SIZE(buerSetTwo))]);
			if(u.sealCounts < numSlots){
				pline("Will you walk with me?\"");
				bindspirit(ep->ward_id);
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
		} else pline("You can't feel the spirit.");
	}break;
	case CHUPOCLOPS:{
		if(u.chupoclops < moves){
			struct obj *o = 0, *otmp;
			for(otmp = level.objects[tx][ty]; otmp; otmp = otmp->nexthere){
				if(is_chupodible(otmp)){
					o = otmp;
			break;
				}
			}
			//Chupoclops requires that her seal be drawn around a humanoid corpse or grave.
			if(o || IS_GRAVE(levl[tx][ty].typ)){
				// pline("The %s within the seal begins to twitch and shake.");
				// Your("consciousness expands, and you sense great currents of despair and mortality that wrap the world like silken threads.");
				// pline("The %s falls still, and you know you're in the presence of the Spider.");
				pline("The great sweep of lives and civilizations seems represented in microcosm by the scene within the seal’s boundaries.");
				You("sense the twin threads of mortality and despair,");
				pline("that wrap this world and its inhabitants like silken bindings, and, suddenly,");
				pline("you know you are in the presence of the Spider.");
				if(u.sealCounts < numSlots){
					pline("She wraps you tight in her bitter cords and sends you forth, bait within her web.");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("She wraps your blade tight in her bitter cords, making it an anchor for her web.");
					uwep->ovar1 |= SEAL_CHUPOCLOPS;
					if(!u.spiritTineA){
						u.spiritTineA = SEAL_CHUPOCLOPS;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_CHUPOCLOPS;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("The Spider passes you over, and collects her prize.");
					
					if(o) useupf(o, 1L);
					else{
						levl[tx][ty].typ = ROOM;
						digfarhole(TRUE,tx,ty);
					}
				}
				u.chupoclops = moves + bindingPeriod;
			} else{
				pline("Thoughts of death and despair almost overcome you.");
				u.chupoclops = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case DANTALION:{
		if(u.dantalion < moves){
			//Spirit requires that his seal be drawn around a throne.
			if(IS_THRONE(levl[tx][ty].typ)){
				if(!Blind) {
					You("see a man with many countenances step out from behind the throne.");
					pline("Below his crown are many faces of %s,", DantalionRace(u.umonster)); /*  */
					pline("and as he nods and cranes his head, new faces are constantly revealed.");
				}
				pline("\"Tremble, for I am Dantalion, king over all the kings of %s\"",urace.coll);
				if(u.sealCounts < numSlots){
					if(!Blind) {
						pline("The staring faces seem vaguely familiar...");
						pline("With a start, you realize they remind you of yourself, and your family!");
					}
					pline("\"You, who bear my bloodline,\"");
					pline("\"go forth %swith my blessing.\"", flags.female ? "":"and rule ");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(flags.initgend==1){ /*(female)*/
					}
					pline("\"You, who bear my sceptre,\"");
					pline("\"go forth, in my name!\"");
					uwep->ovar1 |= SEAL_DANTALION;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_DANTALION;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_DANTALION;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					if(!Blind) {
						pline("His myriad faces study you with disapproval, and he departs as suddenly as he arived.");
					}
				}
				u.dantalion = moves + bindingPeriod;
			} else {
				You_hear("royal trumpets.");
				u.dantalion = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case SHIRO:{
		if(u.shiro < moves){
			int ttx, tty;
			boolean validLocation = TRUE;
			struct obj *otmp;
			for(ttx=tx-1; ttx<=tx+1; ttx++){
				for(tty=ty-1; tty<=ty+1; tty++){
					if(!isok(ttx,tty)) validLocation = FALSE;
					else if( !((otmp = level.objects[ttx][tty]) && otmp->otyp == ROCK) && !(ttx==tx && tty==ty)){
						validLocation = FALSE;
						pline("what the hell");
					}
				}
			}
			//Spirit requires that his seal be drawn in a ring of rocks.
			if(validLocation){
				pline("\"I'm shocked. So few ever speak to me, everyone ignores me and passes me by.\"");
				pline("\"It's 'cause I'm about as impressive as a stone, right?...I'm used to it, though.\"");
				if(u.sealCounts < numSlots){
					pline("\"You look like a pretty distinctive person.\"");
					pline("\"Let me follow you and practice standing out.\"");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("\"That looks like a pretty distinctive weapon.\"");
					pline("\"Let me follow you and see how you use it.\"");
					uwep->ovar1 |= SEAL_SHIRO;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_SHIRO;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_SHIRO;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("\"Well, I'm certain a person as distinctive as you has better things to do than talk to me.\"");
					pline("\"I hope you visit again some time.\"");
				}
				u.shiro = moves + bindingPeriod;
			} else{
				pline("For some reason you want to arrange rocks in a circle.");
				u.shiro = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case ECHIDNA:{
		if(u.echidna < moves){
			//NOT YET IMPLEMENTED: Spirit requires that her seal be drawn in a cave.
			if(In_mines(&u.uz)){
				if(!Blind){
					You("suddenly notice a monstrous nymph reclining in the center of the seal.");
					pline("She is half a fair woman, with glancing eyes and fair cheeks,");
					pline("and half again a terible dragon, with great scaly wings and serpent's tails where legs should be.");
				}
				if(u.sealCounts < numSlots){
					pline("\"I am Echidna, %s.\"",echidnaTitles[rn2(SIZE(echidnaTitles))]);
					pline("\"Free me from this place, and I and my brood shall fight for your cause.\"");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("\"I am Echidna, %s.\"",echidnaTitles[rn2(SIZE(echidnaTitles))]);
					pline("\"Cut my bonds, and I shall lend my wrath to your cause.\"");
					uwep->ovar1 |= SEAL_ECHIDNA;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_ECHIDNA;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_ECHIDNA;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					if(!Blind){
						pline("She hisses at you, then slithers away.");
					} else {
						pline("Something hisses at you, then slithers away.");
					}
				}
				u.echidna = moves + bindingPeriod;
			} else{
				You("hear scales scraping against stone echo through a cave.");
				u.echidna = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case EDEN:{
		if(u.eden < moves){
			//Spirit requires that its seal be drawn by a fountain.
			if(IS_FOUNTAIN(levl[tx][ty].typ)){
				if(!Blind){
					pline("The water in the fountain begins to bubble.");
					pline("A dome of cerulean crystal emerges from the center of the fountain,");
					pline("the apex of a circular silver cathedral.");
					pline("As the water falls away, you see that the cathedral perches atop a stylized silver dragon.");
					pline("The needle-like tail of the dragon clears the fountain,");
					pline("and the gate in the dragon's jaws begins to open.");
				} else {
					You_hear("bubbling.");
				}
				if(u.sealCounts < numSlots){
					pline("Radiant light falls upon you,");
					pline("blinding you to what lies beyond.");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("Radiant light falls on your weapon.");
					pline("The gates are angled such that you can't see past.");
					uwep->ovar1 |= SEAL_EDEN;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_EDEN;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_EDEN;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("But before they do, the whole construct jerks suddenly upwards,");
					pline("out of sight.");
				}
				u.eden = moves + bindingPeriod;
			} else{
				You_hear("water splashing.");
				u.eden = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case ENKI:{
		if(u.enki < moves){
			boolean largeRoom = TRUE;
			int i, j;
			for(i = -2; i <= 2; i++){
				for(j=-2;j<= 2; j++){
					if(!isok(tx+i,ty+j) || 
						!IS_ROOM(levl[tx+i][ty+j].typ)
					) largeRoom = FALSE;
				}
			}
			//Spirit requires that his seal be drawn in a large open space.
			if(largeRoom){
				pline("Water bubbles up inside the seal,");
				pline("and a figure rises within it.");
				if(u.sealCounts < numSlots){
					pline("I am Enki, god of the first city.");
					pline("Bow to me, and I shall teach the arts of civilzation.");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("I am Enki, god of Eridu.");
					pline("Bow to me, and I shall lend aid from within the Abzu.");
					uwep->ovar1 |= SEAL_ENKI;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_ENKI;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_ENKI;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("But the figure is asleep,");
					pline("and sinks again without saying a word.");
				}
				u.enki = moves + bindingPeriod;
			} else{
				You("dream of wide open spaces.");
				u.enki = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case EURYNOME:{
		if(u.eurynome < moves){
			//Spirit requires that her seal be drawn before some water.
			if(isok(tx+(tx-u.ux), ty+(ty-u.uy)) && 
				IS_POOL(levl[tx+(tx-u.ux)][ty+(ty-u.uy)].typ)
			){
				if(!Blind)
					You("see a figure dancing, far out upon the waters.");
				if(u.sealCounts < numSlots){
					if(!Blind){
						pline("She dances up to you and sweeps you up into her dance.");
						pline("She is beautiful, like nothing you have ever seen before.");
						pline("And yet, she reminds you of EVERYTHING you've ever seen before.");
					} else {
						pline("Something sweeps you into a dance.");
						pline("It feels odd to your touch, like touching everything in the world,");
						pline("and beyond it, all at once.");
					}
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(!Blind){
						pline("She dances up to you and performs a sword dance with her claws.");
					}
					else You_hear("splashing.");
					uwep->ovar1 |= SEAL_EURYNOME;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_EURYNOME;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_EURYNOME;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					if(!Blind) pline("but it pays you no heed.");
				}
				u.eurynome = moves + bindingPeriod;
			} else{
				You("daydream of dancing across waves.");
				u.eurynome = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case EVE:{
		if(u.eve < moves){
			//Spirit requires that her seal be drawn beside a tree.
			if(isok(tx+(tx-u.ux), ty+(ty-u.uy)) && 
				IS_TREE(levl[tx+(tx-u.ux)][ty+(ty-u.uy)].typ) 
			){
				You("%s a woman within the seal.", Blind ? "sense" : "see");
				pline("She is both young and old at once.");
				pline("She looks hurt.");
				if(u.sealCounts < numSlots){
					You("help her to her feet.");
					pline("\"Shall we hunt together?\"");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					You("try to help her, but she is unable to stand.");
					pline("She blesses your blade as thanks.");
					uwep->ovar1 |= SEAL_EVE;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_EVE;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_EVE;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("In fact, you don't think she can move.");
				}
				u.eve = moves + bindingPeriod;
			} else{
				You_hear("wind in the trees.");
				u.eve = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case FAFNIR:{
		if(u.fafnir < moves){
			boolean coins = FALSE;
			struct obj *otmp;
			for(otmp = level.objects[tx][ty]; otmp; otmp = otmp->nexthere){
				if(otmp->oclass == COIN_CLASS && otmp->quan >= 1000*u.ulevel){
					coins = TRUE;
			break;
				}
			}
			//Spirit requires that his seal be drawn in a vault, or on a pile of 1000xyour level coins.
			if(coins || (*in_rooms(tx,ty,VAULT) && u.uinvault)){
				if(!Blind) You("suddenly notice a dragon %s", coins ? "buired in the coins" : "in the room.");
				if(u.sealCounts < numSlots){
					if(!Blind) pline("The dragon lunges forwards to bite you.");
					else pline("something bites you!");
					Your("left finger stings!");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(!Blind) pline("The dragon tries to steal your weapon!");
					else pline("Something tries to steal your weapon!");
					You("fight it off.");
					uwep->ovar1 |= SEAL_FAFNIR;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_FAFNIR;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_FAFNIR;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("It roars at you to leave it alone.");
				}
				u.fafnir = moves + bindingPeriod;
			} else{
				You_hear("the clink of coins.");
				u.fafnir = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case HUGINN_MUNINN:{
		if(u.huginn_muninn < moves){
			//Spirit places no restrictions on where their seal is drawn.
			You_hear("flapping wings.");
			if(!Blind) pline("A pair of ravens land in the seal.");
			if(u.sealCounts < numSlots){
				if(!Blind) pline("They hop up to your shoulders and begin to croak raucously in your ears.");
				else pline("A pair of large birds land on you and begin to croak raucously in your ears.");
				You("try to shoo them away, only to find that they have vanished.");
				bindspirit(ep->ward_id);
				if(ACURR(A_WIS)>ATTRMIN(A_WIS)){
					adjattrib(A_WIS, -1, FALSE);
					AMAX(A_WIS) -= 1;
				}
				if(ACURR(A_INT)>ATTRMIN(A_INT)){
					adjattrib(A_INT, -1, FALSE);
					AMAX(A_INT) -= 1;
				}
			}
			else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
				if(!Blind) pline("They croak raucously at your weapon.");
				else You_hear("Raucous croaking.");
				uwep->ovar1 |= SEAL_HUGINN_MUNINN;
				if(!u.spiritTineA){ 
					u.spiritTineA = SEAL_HUGINN_MUNINN;
					u.spiritTineTA= moves + bindingPeriod;
				}
				else{
					u.spiritTineB = SEAL_HUGINN_MUNINN;
					u.spiritTineTB= moves + bindingPeriod;
				}
				if(ACURR(A_WIS)>ATTRMIN(A_WIS)){
					adjattrib(A_WIS, -1, FALSE);
				}
				if(ACURR(A_INT)>ATTRMIN(A_INT)){
					adjattrib(A_INT, -1, FALSE);
				}
			}
			else{
				if(!Blind) pline("They stare at you for a moment, and then leave just as suddenly as they came.");
			}
			u.huginn_muninn = moves + bindingPeriod;
		} else pline("You can't feel the spirit.");
	}break;
	case IRIS:{
		if(u.iris < moves){
			//Spirit requires that her seal be drawn inside a stinking cloud.
			if(check_stinking_cloud_region((xchar)tx,(xchar)ty)){ 
				You("catch a glimpse of somthing moving in the stinking cloud....");
				pline("But you can't see what it was.");
				if(u.sealCounts < numSlots){
					pline("Something jumps on you from behind!");
					pline("\"YAY! Lets play together!!\"");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("Something grabs your weapon!");
					pline("\"Let me play with that!\"");
					uwep->ovar1 |= SEAL_IRIS;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_IRIS;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_IRIS;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("Apparently it was nothing.");
				}
				u.iris = moves + bindingPeriod;
			} else{
				You("smell sulfur.");
				u.iris = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case JACK:{
		if(u.jack < moves){
			//Spirit requires that his seal be drawn outside of hell and the endgame.
			if(!In_hell(&u.uz) && !In_endgame(&u.uz)){
				if(u.sealCounts < numSlots){
					You("feel something climb onto your back!");
					pline("\"Will you let me stay with you?\"");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("A will-o-wisp drifts out of the seal to hover near your weapon.");
					pline("\"Please let me stay with you!\"");
					uwep->ovar1 |= SEAL_JACK;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_JACK;
						u.spiritTineTA= moves + bindingPeriod;
						if(!uwep->lamplit) begin_burn(uwep, FALSE);
					}
					else{
						u.spiritTineB = SEAL_JACK;
						u.spiritTineTB= moves + bindingPeriod;
						if(!uwep->lamplit) begin_burn(uwep, FALSE);
					}
				}
				else{
					You_hear("aimless footsteps and creaking joints.");
				}
				u.jack = moves + bindingPeriod;
			} else{
				You("think of the wide earth.");
				u.jack = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case MALPHAS:{
		if(u.malphas < moves){
			struct obj *otmp, *o=NULL;
			struct monst *mtmp;
			//Spirit requires that his seal be drawn in a square with a fresh corpse.
			for(otmp = level.objects[tx][ty]; otmp; otmp = otmp->nexthere){
				if(otmp->otyp == CORPSE && 
				(otmp->corpsenm == PM_ACID_BLOB
				|| (monstermoves <= peek_at_iced_corpse_age(otmp) + 50)
				)
			){ 
					o = otmp;
			break;
				}
			}
			if(o){
				  /*////////////////////////////////*/
				 /* Do a light sacrificing routine */
				/*////////////////////////////////*/
				if (your_race(&mons[otmp->corpsenm])) {
					/* Human sacrifice on a chaotic or unaligned altar */
					/* is equivalent to demon summoning */
					pline_The("%s blood covers the %s!", urace.adj, surface(tx,ty));
					change_luck(-2);
					
					if (is_demon(youmonst.data)) {
						You("find the idea of sealing a pact with the blood of your own race to be pleasing.");
						exercise(A_WIS, TRUE);
						adjalign(5);
					} else if (u.ualign.type != A_CHAOTIC) {
						You_feel("you'll regret this infamous offense!");
						(void) adjattrib(A_WIS, -1, TRUE);
						exercise(A_WIS, FALSE);
						adjalign(-5);
						u.ugangr += 3;
						if (!Inhell) {
							angrygods(u.ualign.type);
							change_luck(-5);
						}
					}
				} /* your race */
				else if (otmp->oxlth && otmp->oattached == OATTACHED_MONST
						&& ((mtmp = get_mtraits(otmp, FALSE)) != (struct monst *)0)
						&& mtmp->mtame) {
					/* mtmp is a temporary pointer to a tame monster's attributes,
					 * not a real monster */
					pline("So this is how you repay loyalty?");
					adjalign(-3);
					HAggravate_monster |= FROMOUTSIDE;
				} else if (is_unicorn(&mons[otmp->corpsenm])) {
					int unicalign = sgn((&mons[otmp->corpsenm])->maligntyp);

					if (unicalign == u.ualign.type) {
						pline("This is a grave insult to %s!",
							  (unicalign == A_CHAOTIC)
							  ? "chaos" : unicalign ? "law" : "balance");
						u.ualign.record = -1;
						u.ualign.sins += 10;
					} else{
						if (u.ualign.record < ALIGNLIM)
							You_feel("appropriately %s.", align_str(u.ualign.type));
						else You_feel("you are thoroughly on the right path.");
						adjalign(5);
					}
				}
				Your("sacrifice is accepted.");
				if(!Blind){
					pline("A murder of crows decends on the seal.");
					pline("There is something else in the flock...");
				} else You_hear("many wings.");
				if(u.sealCounts < numSlots){
					if(!Blind) pline("A black-feathered humanoid steps forth.");
					pline("\"I am Malphas. You feed my flock. One way or the other.\"");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(!Blind) pline("A large black feather setles within the seal.");
					pline("\"I am Malphas. With that instrument, you feed my flock.\"");
					uwep->ovar1 |= SEAL_MALPHAS;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_MALPHAS;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_MALPHAS;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					if(!Blind) pline("But it does not make itself known to you.");
				}
				u.malphas = moves + bindingPeriod;
				useupf(otmp, 1L);
			} else{
				You("smell fresh blood.");
				u.malphas = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case MARIONETTE:{
		if(u.marionette < moves){
			//Spirit requires that her seal be drawn in the Valley of the Dead or in a graveyard.
			boolean in_a_graveyard = rooms[levl[tx][ty].roomno - ROOMOFFSET].rtype == MORGUE;
			if(in_a_graveyard || on_level(&valley_level, &u.uz)){
				if(!Blind) You("notice metal wires sticking out of the ground within the seal.");
				if(u.sealCounts < numSlots){
					if(!Blind) pline("In fact, there are wires sticking up all around you.");
					if(!Blind) pline("Shreaks and screems echo down from whence the wires come.");
					else You("hear screaming!");
					pline("You feel sharp pains in your elbowes and knees!");
					if(!Blind) pline("It seems that you, are but a puppet.");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(!Blind) pline("The wires wrap around your weapon!");
					else pline("Something tangles around your weapon!");
					uwep->ovar1 |= SEAL_MARIONETTE;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_MARIONETTE;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_MARIONETTE;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("But they don't do anything interesting.");
				}
				u.marionette = moves + bindingPeriod;
			} else{
				You_hear("the sounds of digging, and of bones rattling together.");
				u.marionette = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case MOTHER:{
		if(u.mother < moves){
			//Spirit requires that her seal addressed while blind.
			if(Blind){
				Your("Hands itch painfully.");
				if(u.sealCounts < numSlots){
					You("feel eyes open in your hands!");
					pline("But you still can't see...");
					pline("...the eyeballs don't belong to you!");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("You're pretty sure somthing is staring at your weapon....");
					uwep->ovar1 |= SEAL_MOTHER;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_MOTHER;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_MOTHER;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("The itching subsides.");
				}
				u.mother = moves + bindingPeriod;
			} else{
				You("blink.");
				u.mother = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case NABERIUS:{
		if(u.naberius < moves){
			//Spirit requires that his seal be drawn by an intelligent and wise person.
			if(ACURR(A_INT) >= 14 &&
				ACURR(A_WIS) >= 14
			){ 
				You_hear("a snuffing noise.");
				if(u.sealCounts < numSlots){
					if(!Blind){
						pline("A dog wanders in to the seal, nose to the ground.");
						pline("It wanders back and forth, then looks up at you.");
						pline("It looks up at you with all three heads.");
					}
					pline("\"Hello, I am Naberius, the councilor.\"");
					pline("\"I can smell the weaknesses others try to hide.\"");
					pline("\"I can make men trust or make men flee.\"");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(!Blind){
						pline("A dog wanders in to the seal, nose to the ground.");
						pline("With its second head, it licks your hand.");
						pline("But its third head steals your weapon!");
					} else pline("Something steals your weapon!");
					pline("You begin to chase the animal,");
					pline("and it abandons your blade in the center of the seal.");
					uwep->ovar1 |= SEAL_NABERIUS;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_NABERIUS;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_NABERIUS;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("The sound fades away.");
				}
				u.naberius = moves + bindingPeriod;
			} else{
				You_hear("retoric and sage advice.");
				u.naberius = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case ORTHOS:{
		if(u.orthos < moves){
			struct trap *t=t_at(tx,ty);
			//Spirit requires that his seal be drawn in a square with a hole.
			if(t && t->ttyp == HOLE){
				if(!Blind) pline("The hole grows darker, and a whistling occurs at the edge of hearing.");
				else pline("A whistling occurs at the edge of hearing.");
				pline("The mournful whistle grows louder, as the air around you flows into the pit.");
				if(!Blind) pline("But that is all that occurs. Darkness. Wind. And a lonely whistle.");
				if(u.sealCounts < numSlots){
					pline("You feel that it will stay with you for a while.");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					Your("blade vibrates for a moment.");
					uwep->ovar1 |= SEAL_ORTHOS;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_ORTHOS;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_ORTHOS;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				u.orthos = moves + bindingPeriod;
			} else{
				pline("For an instant you are falling.");
				u.orthos = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case OSE:{
		if(u.ose < moves){
			//Spirit requires that its seal be drawn underwater.
			if(IS_POOL(levl[tx][ty].typ) && IS_POOL(levl[u.ux][u.uy].typ) && u.uinwater && Underwater){ 
				if(u.sealCounts < numSlots){
					pline("The sea-bottom within the seal fades, as if it were silt settling out of muddy water.");
					pline("A sleeping %s floats gently up out of the dark seas below the seal.",u.osegen);
					pline("You supose %s could be called comely,",u.osepro);
					pline("though to be honest %s is about average among %s you have known.",u.osepro,makeplural(u.osegen));
					if(!rn2(20)) pline("The figure's eyes open, and you have a long negotiation before achieving a good pact.");
					else pline("You know that this is Ose, despite never having met.");
					pline("The seabed rises.");
					bindspirit(ep->ward_id);
					levl[tx][ty].typ = ROOM;
					newsym(tx,ty);
					levl[u.ux][u.uy].typ = ROOM;
					newsym(u.ux,u.uy);
					vision_recalc(2);	/* unsee old position */
					vision_full_recalc = 1;
					spoteffects(FALSE);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(!Blind) pline("The sea bottom swirls below your weapon.");
					uwep->ovar1 |= SEAL_OSE;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_OSE;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_OSE;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					if(!Blind) pline("The silt in the seal swirls a bit. Otherwise, nothing happens.");
				}
				u.ose = moves + bindingPeriod;
			} else{
				You("feel wet....");
				u.ose = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case OTIAX:{
		if(u.otiax < moves){
			//Spirit requires that its seal be drawn on a closed door.
			if(IS_DOOR(levl[tx][ty].typ) && closed_door(tx,ty)){ 
				if(!Blind) pline("Thick fingers of mist reach under the door.");
				if(u.sealCounts < numSlots){
					pline("BANG!! Something hits the door from the other side!");
					if(!Blind) pline("The door opens, a bank of fog pours out to curl around you.");
					levl[tx][ty].doormask &= ~(D_CLOSED|D_LOCKED);
					levl[tx][ty].doormask |= D_ISOPEN;
					newsym(tx,ty);
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(!Blind) pline("The mist fingers curl around your blade.");
					uwep->ovar1 |= SEAL_OTIAX;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_OTIAX;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_OTIAX;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					if(!Blind) pline("They fade away without incident.");
				}
				u.otiax = moves + bindingPeriod;
			} else{
				You("instinctively look around for a door to open.");
				u.otiax = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case PAIMON:{
		if(u.paimon < moves){
			struct obj *otmp, *o=NULL;
			struct monst *mtmp;
			for(otmp = level.objects[tx][ty]; otmp; otmp = otmp->nexthere){
				if(otmp->oclass == SPBOOK_CLASS && 
					!(
					  otmp->corpsenm == SPE_BLANK_PAPER ||
					  otmp->corpsenm == SPE_BOOK_OF_THE_DEAD ||
					  otmp->corpsenm == SPE_SECRETS
					) && !(otmp->oartifact)
				){
					o = otmp;
			break;
				}
			}
			//Spirit requires that her seal be drawn around a spellbook. The summoner must face toward the northwest during the ritual.
			if( o && tx - u.ux < 0 && ty - u.uy < 0){
				pline("The pages of %s begin to turn.", xname(o));
				if(u.sealCounts < numSlots){
					if(!Blind){
						pline("A beautiful woman rides into the seal on a camel.");
						pline("She scoops up the book and begins pouring through it.");
						pline("As she reads, she absentmindedly hands you her crown.");
					}
					pline("\"Your contribution is appreciated. Now don't bother me.\"");
					o->otyp = SPE_BLANK_PAPER;
					newsym(tx,ty);
					bindspirit(ep->ward_id);
					u.paimon = moves + bindingPeriod;
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(!Blind) pline("A beautiful woman walks into the seal.");
					pline("\"Somtimes, a subtle aproach is better.\"");
					if(!Blind) pline("She dips her fingers into the ink of %s and writes on your weapon.");
					o->spestudied++;
					uwep->ovar1 |= SEAL_PAIMON;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_PAIMON;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_PAIMON;
						u.spiritTineTB= moves + bindingPeriod;
					}
					u.paimon = moves + bindingPeriod;
				}
				else{
					if(!Blind) pline("A beautiful woman rides into the seal on a camel.");
					if(!Blind) pline("Her face darkens.");
					if(!Blind) pline("Woman and camel both vanish, replaced by a demon of black smoke.");
					pline("A very masculine voice booms out:");
					pline("\"How dare you waste the time of the fell archivist!\".");
					losexp("shredding of the soul",TRUE,TRUE,TRUE);
				}
			} else{
				You("dream briefly of a library in the northwest kingdom.");
				u.paimon = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case SIMURGH:{
		if(u.simurgh < moves){
			//Spirit requires that her seal be drawn outside.
			if(In_outdoors(&u.uz)){
				pline("A brilliantly colored bird with iron claws flies high overhead.");
				if(u.sealCounts < numSlots){
					pline("It swoops down and lands on your shoulder.");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("Loose feathers rain down around your blade.");
					uwep->ovar1 |= SEAL_SIMURGH;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_SIMURGH;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_SIMURGH;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					pline("It leaves");
				}
				u.simurgh = moves + bindingPeriod;
			} else{
				You("yearn for open skies.");
				u.simurgh = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case TENEBROUS:{
		if(u.tenebrous < moves){
			//Spirit requires that his seal be drawn in darkness.
			if( !(levl[tx][ty].lit) && !(levl[u.ux][u.uy].lit) ){
				if(!Blind) pline("Within the seal, darkness takes on its own meaning,");
				if(!Blind) pline("beyond mere absense of light.");
				if(u.sealCounts < numSlots){
					if(!Blind) pline("The darkness inside the seal flows out to pool around you.");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					if(!Blind) pline("The darkness inside the seal flows out to stain your weapon.");
					uwep->ovar1 |= SEAL_TENEBROUS;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_TENEBROUS;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_TENEBROUS;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				else{
					if(!Blind)  pline("Gradually, the lighting returns to normal.");
				}
				u.tenebrous = moves + bindingPeriod;
			} else{
				You("feel that Tenebrous exists only in darkness.");
				u.tenebrous = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case YMIR:{
		if(u.ymir < moves){
			struct obj *otmp, *o=NULL;
			struct monst *mtmp;
			for(otmp = level.objects[tx][ty]; otmp; otmp = otmp->nexthere){
				if(	otmp->otyp == CORPSE && 
					otmp->corpsenm != PM_LICHEN && 
					otmp->corpsenm != PM_LIZARD && 
					otmp->corpsenm != PM_BEHOLDER && 
					(monstermoves - peek_at_iced_corpse_age(otmp))/(10L) > 5L &&
					poisonous(&mons[otmp->corpsenm])
				){
					o = otmp;
			break;
				}
			}
			//Spirit requires that his seal be drawn around a rotting corpse of a poisonous creature.
			if(	o ){
				pline("An eye opens on the ground within the seal,");
				pline("and a voice speaks to you out of the Earth:");
				pline("\"There was, in times of old, when Ymir lived,\"");
				pline("neither sea nor sand nor waves,");
				pline("no earth, nor heaven above,");
				pline("but a yawning gap, and grass nowhere.");
				pline("From Ymir's flesh the earth was formed,");
				pline("and from his bones the hills.");
				pline("From Ymir's skull, the ice-cold sky,");
				pline("and from his blood the sea.\"");
				if(u.sealCounts < numSlots){
					pline("\"I was Ymir, god of poison,");
					pline("and you the maggots in my corpse.");
					pline("But I will make a pact with you,");
					pline("to throw down the false gods,");
					pline("that ordered my demise.\"");
					bindspirit(ep->ward_id);
				}
				else if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && (!u.spiritTineA || (!u.spiritTineB && quest_status.killed_nemesis))){
					pline("\"I was Ymir, god of poison,");
					pline("this steel is the steel of my teeth,");
					pline("and the gods shall feel their bite.\"");
					uwep->ovar1 |= SEAL_YMIR;
					if(!u.spiritTineA){ 
						u.spiritTineA = SEAL_YMIR;
						u.spiritTineTA= moves + bindingPeriod;
					}
					else{
						u.spiritTineB = SEAL_YMIR;
						u.spiritTineTB= moves + bindingPeriod;
					}
				}
				u.ymir = moves + bindingPeriod;
			}  else{
				pline("Rot calls rot, and poison calls poison.");
				u.ymir = moves + bindingPeriod/10;
			}
		} else pline("You can't feel the spirit.");
	}break;
	case DAHLVER_NAR:{
		if(u.dahlver_nar < moves){
			//Spirit requires that his seal be drawn by a level 14+ Binder.
			if(quest_status.got_quest && Role_if(PM_EXILE)){
				pline("The bloody, tooth-torn corpse of Dahlver-Nar hanges over the seal.");
				pline("He moans and reaches out to you.");
				bindspirit(ep->ward_id);
			}
		} else pline("You hear distant moaning.");
	}break;
	case ACERERAK:{
		if(u.acererak < moves){
			//Spirit requires that his seal be drawn by a Binder who has killed him.
			if(Role_if(PM_EXILE) && quest_status.killed_nemesis){
				pline("A golden skull hanges over the seal.");
				pline("\"I am Acererak. Long ago, I dared the Gates of Teeth.\"");
				pline("\"Now I am trapped outside of time,");
				pline("beyond life, motion, and thought.\"");
				bindspirit(ep->ward_id);
			}
		} else pline("You can't feel the spirit.");
	}break;
	case NUMINA:{
		//Spirit requires that its seal be drawn by a level 30 Binder.
		//There is no binding period.
		if(u.ulevel == 30 && Role_if(PM_EXILE)){
			int skill;
			You("hear a tumultuous babble of voices.");
			pline("So insistent are they that even the un initiated can hear,");
			pline("albeit only in the form of whispers.");
			bindspirit(ep->ward_id);
		} else pline("You hear whispering all around you.");
	}break;
	}
	return 1;
}

void
bindspirit(seal_id)
	int seal_id;
{
	int bindingPeriod = 5000;
	switch(seal_id){
		case AHAZU:{
			if(u.ahazu < moves){
				unrestrict_weapon_skill(P_FLAIL);
				u.sealsActive |= SEAL_AHAZU;
				u.spirit[u.sealCounts] = SEAL_AHAZU;
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				set_spirit_powers(SEAL_AHAZU);
				u.sealCounts++;
				u.ahazu = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case AMON:{
			if(u.amon < moves){
				unrestrict_weapon_skill(P_CLERIC_SPELL);
				u.sealsActive |= SEAL_AMON;
				u.spirit[u.sealCounts] = SEAL_AMON;
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				set_spirit_powers(SEAL_AMON);
				u.wisSpirits++;
				u.sealCounts++;
				u.amon = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case ANDREALPHUS:{
			if(u.andrealphus < moves){
				unrestrict_weapon_skill(P_ESCAPE_SPELL);
				u.sealsActive |= SEAL_ANDREALPHUS;
				u.spirit[u.sealCounts] = SEAL_ANDREALPHUS;
				set_spirit_powers(SEAL_ANDREALPHUS);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.intSpirits++;
				u.sealCounts++;
				u.andrealphus = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case ANDROMALIUS:{
			if(u.andromalius < moves){
				unrestrict_weapon_skill(P_DAGGER);
				u.sealsActive |= SEAL_ANDROMALIUS;
				u.spirit[u.sealCounts] = SEAL_ANDROMALIUS;
				set_spirit_powers(SEAL_ANDROMALIUS);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.andromalius = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case ASTAROTH:{
			if(u.astaroth < moves){
				unrestrict_weapon_skill(P_CROSSBOW);
				unrestrict_weapon_skill(P_SHURIKEN);
				u.sealsActive |= SEAL_ASTAROTH;
				u.spirit[u.sealCounts] = SEAL_ASTAROTH;
				set_spirit_powers(SEAL_ASTAROTH);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.astaroth = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case BALAM:{
			if(u.balam < moves){
				unrestrict_weapon_skill(P_WHIP);
				u.sealsActive |= SEAL_BALAM;
				u.spirit[u.sealCounts] = SEAL_BALAM;
				set_spirit_powers(SEAL_BALAM);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.balam = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case BERITH:{
			if(u.berith < moves){
				unrestrict_weapon_skill(P_SABER);
				unrestrict_weapon_skill(P_LANCE);
				unrestrict_weapon_skill(P_RIDING);
				u.sealsActive |= SEAL_BERITH;
				u.spirit[u.sealCounts] = SEAL_BERITH;
				set_spirit_powers(SEAL_BERITH);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.berith = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case BUER:{
			if(u.buer < moves){
				unrestrict_weapon_skill(P_HEALING_SPELL);
				u.sealsActive |= SEAL_BUER;
				u.spirit[u.sealCounts] = SEAL_BUER;
				set_spirit_powers(SEAL_BUER);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.wisSpirits++;
				u.sealCounts++;
				u.buer = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case CHUPOCLOPS:{
			if(u.chupoclops < moves){
				unrestrict_weapon_skill(P_KNIFE);
				u.sealsActive |= SEAL_CHUPOCLOPS;
				u.spirit[u.sealCounts] = SEAL_CHUPOCLOPS;
				set_spirit_powers(SEAL_CHUPOCLOPS);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.chupoclops = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case DANTALION:{
			if(u.dantalion < moves){
				unrestrict_weapon_skill(P_BROAD_SWORD);
				unrestrict_weapon_skill(P_TWO_HANDED_SWORD);
				unrestrict_weapon_skill(P_SCIMITAR);
				u.sealsActive |= SEAL_DANTALION;
				u.spirit[u.sealCounts] = SEAL_DANTALION;
				set_spirit_powers(SEAL_DANTALION);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.dantalion = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case SHIRO:{
			if(u.shiro < moves){
				unrestrict_weapon_skill(P_POLEARMS);
				u.sealsActive |= SEAL_SHIRO;
				u.spirit[u.sealCounts] = SEAL_SHIRO;
				set_spirit_powers(SEAL_SHIRO);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.shiro = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case ECHIDNA:{
			if(u.echidna < moves){
				unrestrict_weapon_skill(P_UNICORN_HORN);
				u.sealsActive |= SEAL_ECHIDNA;
				u.spirit[u.sealCounts] = SEAL_ECHIDNA;
				set_spirit_powers(SEAL_ECHIDNA);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.echidna = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case EDEN:{
			if(u.eden < moves){
				unrestrict_weapon_skill(P_LONG_SWORD);
				u.sealsActive |= SEAL_EDEN;
				u.spirit[u.sealCounts] = SEAL_EDEN;
				set_spirit_powers(SEAL_EDEN);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.eden = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case ENKI:{
			if(u.enki < moves){
				unrestrict_weapon_skill(P_SHORT_SWORD);
				unrestrict_weapon_skill(P_HAMMER);
				unrestrict_weapon_skill(P_JAVELIN);
				unrestrict_weapon_skill(P_SLING);
				unrestrict_weapon_skill(P_DART);
				unrestrict_weapon_skill(P_BOOMERANG);
				u.sealsActive |= SEAL_ENKI;
				u.spirit[u.sealCounts] = SEAL_ENKI;
				set_spirit_powers(SEAL_ENKI);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.enki = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case EURYNOME:{
			if(u.eurynome < moves){
				unrestrict_weapon_skill(P_BARE_HANDED_COMBAT);
				u.sealsActive |= SEAL_EURYNOME;
				u.spirit[u.sealCounts] = SEAL_EURYNOME;
				set_spirit_powers(SEAL_EURYNOME);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.eurynome = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case EVE:{
			if(u.eve < moves){
				unrestrict_weapon_skill(P_BOW);
				unrestrict_weapon_skill(P_HARVEST);
				u.sealsActive |= SEAL_EVE;
				u.spirit[u.sealCounts] = SEAL_EVE;
				set_spirit_powers(SEAL_EVE);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.eve = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case FAFNIR:{
			if(u.fafnir < moves){
				unrestrict_weapon_skill(P_PICK_AXE);
				u.sealsActive |= SEAL_FAFNIR;
				u.spirit[u.sealCounts] = SEAL_FAFNIR;
				set_spirit_powers(SEAL_FAFNIR);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.fafnir = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case HUGINN_MUNINN:{
			if(u.huginn_muninn < moves){
				unrestrict_weapon_skill(P_SPEAR);
				u.sealsActive |= SEAL_HUGINN_MUNINN;
				u.spirit[u.sealCounts] = SEAL_HUGINN_MUNINN;
				set_spirit_powers(SEAL_HUGINN_MUNINN);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.huginn_muninn = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case IRIS:{
			if(u.iris < moves){
				unrestrict_weapon_skill(P_MORNING_STAR);
				u.sealsActive |= SEAL_IRIS;
				u.spirit[u.sealCounts] = SEAL_IRIS;
				set_spirit_powers(SEAL_IRIS);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.iris = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case JACK:{
			if(u.jack < moves){
				u.sealsActive |= SEAL_JACK;
				u.spirit[u.sealCounts] = SEAL_JACK;
				set_spirit_powers(SEAL_JACK);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.jack = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case MALPHAS:{
			if(u.malphas < moves){
				u.sealsActive |= SEAL_MALPHAS;
				u.spirit[u.sealCounts] = SEAL_MALPHAS;
				set_spirit_powers(SEAL_MALPHAS);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.malphas = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case MARIONETTE:{
			if(u.marionette < moves){
				u.sealsActive |= SEAL_MARIONETTE;
				u.spirit[u.sealCounts] = SEAL_MARIONETTE;
				set_spirit_powers(SEAL_MARIONETTE);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.marionette = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case MOTHER:{
			if(u.mother < moves){
				unrestrict_weapon_skill(P_DIVINATION_SPELL);
				u.sealsActive |= SEAL_MOTHER;
				u.spirit[u.sealCounts] = SEAL_MOTHER;
				set_spirit_powers(SEAL_MOTHER);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.wisSpirits++;
				u.sealCounts++;
				u.mother = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case NABERIUS:{
			if(u.naberius < moves){
				unrestrict_weapon_skill(P_QUARTERSTAFF);
				unrestrict_weapon_skill(P_ATTACK_SPELL);
				u.sealsActive |= SEAL_NABERIUS;
				u.spirit[u.sealCounts] = SEAL_NABERIUS;
				set_spirit_powers(SEAL_NABERIUS);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.intSpirits++;
				u.sealCounts++;
				u.naberius = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case ORTHOS:{
			if(u.orthos < moves){
				u.sealsActive |= SEAL_ORTHOS;
				u.spirit[u.sealCounts] = SEAL_ORTHOS;
				set_spirit_powers(SEAL_ORTHOS);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.orthos = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case OSE:{
			if(u.ose < moves){
				unrestrict_weapon_skill(P_TRIDENT);
				u.sealsActive |= SEAL_OSE;
				u.spirit[u.sealCounts] = SEAL_OSE;
				set_spirit_powers(SEAL_OSE);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.intSpirits++;
				u.sealCounts++;
				u.ose = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case OTIAX:{
			if(u.otiax < moves){
				u.sealsActive |= SEAL_OTIAX;
				u.spirit[u.sealCounts] = SEAL_OTIAX;
				set_spirit_powers(SEAL_OTIAX);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.otiax = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case PAIMON:{
			if(u.paimon < moves){
				u.sealsActive |= SEAL_PAIMON;
				u.spirit[u.sealCounts] = SEAL_PAIMON;
				set_spirit_powers(SEAL_PAIMON);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.intSpirits++;
				u.sealCounts++;
				u.paimon = moves + bindingPeriod;
				u.paimon = moves + bindingPeriod/10;
			} else You("can't feel the spirit.");
		}break;
		case SIMURGH:{
			if(u.simurgh < moves){
				unrestrict_weapon_skill(P_ENCHANTMENT_SPELL);
				u.sealsActive |= SEAL_SIMURGH;
				u.spirit[u.sealCounts] = SEAL_SIMURGH;
				set_spirit_powers(SEAL_SIMURGH);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.simurgh = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case TENEBROUS:{
			if(u.tenebrous < moves){
				unrestrict_weapon_skill(P_MACE);
				u.sealsActive |= SEAL_TENEBROUS;
				u.spirit[u.sealCounts] = SEAL_TENEBROUS;
				set_spirit_powers(SEAL_TENEBROUS);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.tenebrous = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case YMIR:{
			if(u.ymir < moves){
				unrestrict_weapon_skill(P_CLUB);
				u.sealsActive |= SEAL_YMIR;
				u.spirit[u.sealCounts] = SEAL_YMIR;
				set_spirit_powers(SEAL_YMIR);
				u.spiritT[u.sealCounts] = moves + bindingPeriod;
				u.sealCounts++;
				u.ymir = moves + bindingPeriod;
			} else You("can't feel the spirit.");
		}break;
		case DAHLVER_NAR:{
			if(u.dahlver_nar < moves){
					if(u.spirit[QUEST_SPIRIT]){
						//Eject current quest spirit
					if(u.specialSealsActive&SEAL_ACERERAK) unbind(SEAL_SPECIAL|SEAL_ACERERAK,TRUE);
					}
					u.specialSealsActive |= SEAL_SPECIAL|SEAL_DAHLVER_NAR;
					u.spirit[QUEST_SPIRIT] = SEAL_SPECIAL|SEAL_DAHLVER_NAR;
					set_spirit_powers(SEAL_SPECIAL|SEAL_DAHLVER_NAR);
					u.spiritT[QUEST_SPIRIT] = moves + bindingPeriod;
				u.wisSpirits++;
				u.dahlver_nar = moves + bindingPeriod;
			} else You("can't feel the spirit.");
	}break;
	case ACERERAK:{
		if(u.acererak < moves){
					if(u.spirit[QUEST_SPIRIT]){
						//Eject current quest spirit
					if(u.specialSealsActive&SEAL_DAHLVER_NAR) unbind(SEAL_SPECIAL|SEAL_DAHLVER_NAR,TRUE);
					}
					u.specialSealsActive |= SEAL_SPECIAL|SEAL_ACERERAK;
					u.spirit[QUEST_SPIRIT] = SEAL_SPECIAL|SEAL_ACERERAK;
					set_spirit_powers(SEAL_SPECIAL|SEAL_ACERERAK);
					u.spiritT[QUEST_SPIRIT] = moves + bindingPeriod;
				u.acererak = moves + bindingPeriod;
				u.intSpirits++;
			} else You("can't feel the spirit.");
	}break;
	case NUMINA:{
		if(u.ulevel == 30 && Role_if(PM_EXILE)){
			int skill;
			for (skill = 0; skill < P_NUM_SKILLS; skill++) {
				OLD_P_SKILL(skill) = P_UNSKILLED;
			}
			u.spirit[OUTER_SPIRIT] = SEAL_SPECIAL|SEAL_NUMINA;
			u.specialSealsActive |= SEAL_SPECIAL|SEAL_NUMINA;
			set_spirit_powers(SEAL_SPECIAL|SEAL_NUMINA);
			} else You("can't hear the numina.");
	}break;
	}
	return;
}

static
const char *
DantalionRace(pmon)
int pmon;
{
	switch(pmon){
		case PM_HUMAN:
		case PM_INCANTIFIER:
		case PM_VAMPIRE:
		case PM_CLOCKWORK_AUTOMATON:
		default:
			return "men";
		break;
		case PM_ELF:
			return "elf-lords";
		break;
		case PM_DROW:
			return "hedrow";
		break;
		case PM_DWARF:
			return "dwarves";
		break;
		case PM_GNOME:
			return "gnomes";
		break;
		case PM_ORC:
			return "orcs";
		break;
	}
}

P_MAX_SKILL(p_skill)
int p_skill;
{
	int maxskill = OLD_P_MAX_SKILL(p_skill);
	if(u.specialSealsActive&SEAL_NUMINA || spiritSkill(p_skill)) maxskill = max(P_EXPERT,maxskill);
	return maxskill;
}

int
P_SKILL(p_skill)
int p_skill;
{
	int curskill = OLD_P_SKILL(p_skill),
		maxskill = P_MAX_SKILL(p_skill);
	if(spiritSkill(p_skill) || roleSkill(p_skill)){
		curskill += 1;
		maxskill += 1;
	}
	return min(curskill, maxskill);
}

int
P_RESTRICTED(p_skill)
int p_skill;
{
	return (u.weapon_skills[p_skill].skill==P_ISRESTRICTED 
		&& !(spiritSkill(p_skill) || u.specialSealsActive&SEAL_NUMINA) );
}

boolean
roleSkill(p_skill)
int p_skill;
{
	if(p_skill == P_POLEARMS) return (Role_if(PM_SAMURAI) && uwep && uwep->otyp == GLAIVE);
	else return FALSE;
}

boolean
spiritSkill(p_skill)
int p_skill;
{
	// if(u.specialSealsActive & SEAL_NUMINA) return TRUE;
	if(p_skill == P_DAGGER) return u.sealsActive & SEAL_ANDROMALIUS? TRUE : FALSE;
	if(p_skill == P_KNIFE) return u.sealsActive & SEAL_CHUPOCLOPS? TRUE : FALSE;
	if(p_skill == P_AXE) return u.sealsActive & SEAL_MARIONETTE? TRUE : FALSE;
	if(p_skill == P_PICK_AXE) return u.sealsActive & SEAL_FAFNIR? TRUE : FALSE;
	if(p_skill == P_SHORT_SWORD) return u.sealsActive & SEAL_ENKI? TRUE : FALSE;
	if(p_skill == P_BROAD_SWORD) return u.sealsActive & SEAL_DANTALION? TRUE : FALSE;
	if(p_skill == P_LONG_SWORD) return u.sealsActive & SEAL_EDEN? TRUE : FALSE;
	if(p_skill == P_TWO_HANDED_SWORD) return u.sealsActive & SEAL_DANTALION? TRUE : FALSE;
	if(p_skill == P_SCIMITAR) return u.sealsActive & SEAL_DANTALION? TRUE : FALSE;
	if(p_skill == P_SABER) return u.sealsActive & SEAL_BERITH? TRUE : FALSE;
	if(p_skill == P_CLUB) return u.sealsActive & SEAL_YMIR? TRUE : FALSE;
	if(p_skill == P_MACE) return u.sealsActive & SEAL_TENEBROUS? TRUE : FALSE;
	if(p_skill == P_MORNING_STAR) return u.sealsActive & SEAL_IRIS? TRUE : FALSE;
	if(p_skill == P_FLAIL) return u.sealsActive & SEAL_AHAZU? TRUE : FALSE;
	if(p_skill == P_HAMMER) return u.sealsActive & SEAL_ENKI ? TRUE : FALSE;
	if(p_skill == P_QUARTERSTAFF) return u.sealsActive & SEAL_NABERIUS? TRUE : FALSE;
	if(p_skill == P_POLEARMS) return u.sealsActive & SEAL_SHIRO? TRUE : FALSE;
	if(p_skill == P_SPEAR) return u.sealsActive & SEAL_HUGINN_MUNINN? TRUE : FALSE;
	if(p_skill == P_JAVELIN) return u.sealsActive & SEAL_ENKI? TRUE : FALSE;
	if(p_skill == P_TRIDENT) return u.sealsActive & SEAL_OSE? TRUE : FALSE;
	if(p_skill == P_LANCE) return u.sealsActive & SEAL_BERITH? TRUE : FALSE;
	if(p_skill == P_BOW) return u.sealsActive & SEAL_EVE? TRUE : (u.sealsActive & SEAL_BERITH && u.usteed)? TRUE : FALSE;
	if(p_skill == P_SLING) return u.sealsActive & SEAL_ENKI? TRUE : FALSE;
	if(p_skill == P_CROSSBOW) return u.sealsActive & SEAL_ASTAROTH? TRUE : FALSE;
	if(p_skill == P_DART) return u.sealsActive & SEAL_ENKI? TRUE : FALSE;
	if(p_skill == P_SHURIKEN) return u.sealsActive & SEAL_ASTAROTH? TRUE : FALSE;
	if(p_skill == P_BOOMERANG) return u.sealsActive & SEAL_ENKI? TRUE : FALSE;
	if(p_skill == P_WHIP) return u.sealsActive & SEAL_BALAM? TRUE : FALSE;
	if(p_skill == P_HARVEST) return u.sealsActive & SEAL_EVE? TRUE : FALSE;
	if(p_skill == P_UNICORN_HORN) return u.sealsActive & SEAL_ECHIDNA? TRUE : FALSE;
	if(p_skill == P_ATTACK_SPELL) return u.sealsActive & SEAL_NABERIUS? TRUE : FALSE;
	if(p_skill == P_HEALING_SPELL) return u.sealsActive & SEAL_BUER? TRUE : FALSE;
	if(p_skill == P_DIVINATION_SPELL) return u.sealsActive & SEAL_MOTHER? TRUE : FALSE;
	if(p_skill == P_ENCHANTMENT_SPELL) return u.sealsActive & SEAL_SIMURGH? TRUE : FALSE;
	if(p_skill == P_CLERIC_SPELL) return u.sealsActive & SEAL_AMON? TRUE : FALSE;
	if(p_skill == P_ESCAPE_SPELL) return u.sealsActive & SEAL_ANDREALPHUS? TRUE : FALSE;
	if(p_skill == P_MATTER_SPELL) return u.sealsActive & SEAL_MARIONETTE? TRUE : FALSE;
	if(p_skill == P_RIDING) return u.sealsActive & SEAL_BERITH? TRUE : FALSE;
	if(p_skill == P_BARE_HANDED_COMBAT) return u.sealsActive & SEAL_EURYNOME? TRUE : FALSE;
	return FALSE;
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
