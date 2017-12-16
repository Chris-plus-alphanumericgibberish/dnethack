/*	SCCS Id: @(#)read.c	3.4	2003/10/22	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"

/* KMH -- Copied from pray.c; this really belongs in a header file */
#define DEVOUT 14
#define STRIDENT 4

#define Your_Own_Role(mndx) \
	((mndx) == urole.malenum || \
	 (urole.femalenum != NON_PM && (mndx) == urole.femalenum))
#define Your_Own_Race(mndx) \
	((mndx) == urace.malenum || \
	 (urace.femalenum != NON_PM && (mndx) == urace.femalenum))

#ifdef OVLB

boolean	known;

static NEARDATA const char readable[] =
		   { ALL_CLASSES, SCROLL_CLASS, SPBOOK_CLASS, 0 };
static const char all_count[] = { ALLOW_COUNT, ALL_CLASSES, 0 };

static void FDECL(wand_explode, (struct obj *));
static void NDECL(do_class_genocide);
static void FDECL(stripspe,(struct obj *));
static void FDECL(p_glow1,(struct obj *));
static void FDECL(p_glow2,(struct obj *,const char *));
static void FDECL(randomize,(int *, int));
static void FDECL(forget_single_object, (int));
static void FDECL(maybe_tame, (struct monst *,struct obj *));

int
doread()
{
	register struct obj *scroll;
	register boolean confused;

	known = FALSE;
	if(check_capacity((char *)0)) return (0);
	scroll = getobj(readable, "read");
	if(!scroll) return(0);
	
	if((scroll->oartifact 
			&& !(scroll->oclass == SCROLL_CLASS)
			&& !(scroll->oclass == SPBOOK_CLASS)
			&& !arti_mandala(scroll)
		) || scroll->otyp==LIGHTSABER
	){
		if(scroll->oartifact == ART_ROD_OF_SEVEN_PARTS){
			if (Blind) {
				You_cant("see the writing!");
				exercise(A_WIS, FALSE); //The rod is critial of such logical blunders.
				return 0;
			}
			else{//Ruat Coelum, Fiat Justitia.  Ecce!  Lex Rex!
				pline(scroll->spe < 7 ? "The Rod is badly scarred, mute testimony to a tumultuous history." :
			"The surface of the Rod, once badly defaced, is now shiny and smooth, its mathematical perfection testament to the overriding power of Law.");
				switch(scroll->spe){
					case(-5):
					case(-4):
					case(-3):
						pline("The damage seems heaviest around the point, but the whole rod is really pretty messed up.");
					break;
					case(-2):
					case(-1):
					case(0):
						pline("It seems that at one point, an inscription spiralled around the Rod, but it can no longer be read.");
					break;
					case(1):
						pline("The damage is lighter further from the point, and looking close you can just make out the word \"Ruat\" near the pommel.");
						u.uconduct.literate++;
					break;
					case(2):
						pline("The damage is lighter further from the point.");
						pline("Looking close, you can just make out the phrase \"Ruat Coelum\" near the pommel.");
						pline("It seems to be part of a longer inscription, but the Rod has been thoroughly defaced.");
						u.uconduct.literate++;
					break;
					case(3):
						pline("The damage is lighter near the pommel, and an inscription spirals around the rod.");
						pline("You can make out the phrase \"Ruat Coelum, Fiat\", but the rest of the inscription is defaced.");
						u.uconduct.literate++;
					break;
					case(4):
						pline("The damage is lighter on the half nearest the pommel.");
						pline("You can make out the phrase \"Ruat Coelum, Fiat Justitia.\"");
						pline("There's more, but the rest of it is illegible.");
						u.uconduct.literate++;
					break;
					case(5):
						pline("The damage is heaviest near the tip.");
						pline("You read part of an inscription: \"Ruat Coelum, Fiat Justitia. Ecce!\"");
						pline("There's more, but the rest of it is illegible.");
						u.uconduct.literate++;
					break;
					case(6):
						pline("The damage is heaviest near the tip.");
						pline("You read most of the inscription: \"Ruat Coelum, Fiat Justitia. Ecce! Lex\"");
						pline("There's more, but the rest of it is scratched out.");
						u.uconduct.literate++;
					break;
					case(7):
						pline("An inscription spirals around the Rod, from pommel to tip:");
						pline("\"Ruat Coelum, Fiat Justitia. Ecce! Lex Rex!\"");
						u.uconduct.literate++;
					break;
				}
				if(u.RoSPflights > 0){
					static const char *numbers[]={
						"no", "a singe", "two","three","four","five","six","seven"
					};
					pline("Around the pommel, there is a crownlike decoration, with %s raised segment%s.",
						numbers[u.RoSPflights], u.RoSPflights>1 ? "s" : "");
				}
				return(1);
			}
		} else if(scroll->oartifact == ART_PEN_OF_THE_VOID){
			if (Blind) {
				You_cant("see the blade!");
				return 0;
			}
			else{
				int j;
				if(!(quest_status.killed_nemesis && Role_if(PM_EXILE))) pline("It seems this athame once had dual blades, but one blade has been snapped off at the hilt.");
				else pline("The dual-bladed athame vibrates faintly.");
				if(u.spiritTineA) for(j=0;j<32;j++){
					if((u.spiritTineA >> j) == 1){
						if(!quest_status.killed_nemesis) pline("The blade bears the seal of %s.",sealNames[j]);
						else pline("The first blade bears the seal of %s.",sealNames[j]);
						break;
					}
				}
				if(quest_status.killed_nemesis && u.spiritTineB) for(j=0;j<32;j++){
					if((u.spiritTineB >> j) == 1){
						pline("The second blade bears the seal of %s.",sealNames[j]);
						break;
					}
				}
				return(1);
			}
		} else if(scroll->oartifact == ART_EXCALIBUR){
			if (Blind) {
				You_cant("see the blade!");
				return 0;
			} else if(scroll == uwep) {
				pline("\"Cast me away\"");
			} else {
				pline("\"Take me up\"");
			}
			return(1);
		} else if(scroll->oartifact == ART_GLAMDRING){
			if (Blind) {
				You_cant("see the blade!");
				return 0;
			} else if(Race_if(PM_ELF)) {
				pline("\"Turgon, king of Gondolin, wields, has, and holds the sword Foe-hammer, Foe of Morgoth's realm, Hammer of the Orcs\".");
			} else {
				pline("\"Turgon aran Gondolin tortha gar a matha i vegil Glamdring gud daedheloth, dam an Glamhoth\".");
			}
			return(1);
		} else if(scroll->oartifact == ART_BOW_OF_SKADI){
			if (Blind) {
				You_cant("see the bow!");
				return 0;
			} else {
				int i;
				You("read the Galdr of Skadi!");
				for (i = 0; i < MAXSPELL; i++)  {
					if (spellid(i) == SPE_CONE_OF_COLD)  {
						if (spellknow(i) <= 1000) {
							Your("knowledge of Cone of Cold is keener.");
							spl_book[i].sp_know = 20000;
							exercise(A_WIS,TRUE);       /* extra study */
						} else { /* 1000 < spellknow(i) <= MAX_SPELL_STUDY */
							You("know Cone of Cold quite well already.");
						}
						break;
					} else if (spellid(i) == NO_SPELL)  {
						spl_book[i].sp_id = SPE_CONE_OF_COLD;
						spl_book[i].sp_lev = objects[SPE_CONE_OF_COLD].oc_level;
						spl_book[i].sp_know = 20000;
						You("learn to cast Cone of Cold!");
						break;
					}
				}
				if (i == MAXSPELL) impossible("Too many spells memorized!");
				return 1;
			}
		} else if(scroll->otyp == LIGHTSABER){
			if (Blind) {
				You_cant("see it!");
				return 0;
			} else {
				pline(lightsaber_hiltText(scroll),xname(scroll));
			}
			return(1);
		} else {
			pline(silly_thing_to, "read");
			return(0);
		}
    } else if(scroll->oartifact && scroll->oartifact == ART_ENCYCLOPEDIA_GALACTICA){
      const char *line;
      char buf[BUFSZ];

      line = getrumor(bcsign(scroll), buf, TRUE);
      if (!*line)
        line = "NetHack rumors file closed for renovation.";

      pline("%s:", Tobjnam(scroll, "display"));
      verbalize("%s", line);
      return 1;
    } else if(scroll->oartifact && scroll->oartifact == ART_LOG_OF_THE_CURATOR){
      int oindx = 1 + rn2(NUM_OBJECTS - 1);
      if(objects[oindx].oc_name_known){
        makeknown(oindx);
        You("study the pages of %s, you learn to recognize %s!", xname(scroll),
            obj_typename(oindx));
      } else {
        You("study the pages of %s, but you already can recognize that.", xname(scroll));
      }
      return 1;
	} else if(scroll->oclass == WEAPON_CLASS && (scroll)->obj_material == WOOD && scroll->otyp != MOON_AXE && scroll->ovar1 != 0){
		pline("A %s is carved into the wood.",wardDecode[decode_wardID(scroll->ovar1)]);
		if(! (u.wardsknown & scroll->ovar1) ){
			You("have learned a new warding stave!");
			u.wardsknown |= scroll->ovar1;
		}
		return(1);
	}
	else if(scroll->oclass == RING_CLASS && (isEngrRing((scroll)->otyp)||isSignetRing((scroll)->otyp)) && scroll->ovar1){
		if(!(scroll->ohaluengr)){
			pline("A %s is engraved on the ring.",wardDecode[scroll->ovar1]);
			if( !(u.wardsknown & get_wardID(scroll->ovar1)) ){
				You("have learned a new warding sign!");
				u.wardsknown |= get_wardID(scroll->ovar1);
			}
		}
		else{
			pline("There is %s engraved on the ring.",fetchHaluWard((int)scroll->ovar1));
		}
		return(1);
	}
	/* outrumor has its own blindness check */
	else if(scroll->otyp == FORTUNE_COOKIE) {
	    if(flags.verbose)
		You("break up the cookie and throw away the pieces.");
		if(scroll->ostolen && u.sealsActive&SEAL_ANDROMALIUS) unbind(SEAL_ANDROMALIUS, TRUE);
	    outrumor(bcsign(scroll), BY_COOKIE);
	    if (!Blind) u.uconduct.literate++;
	    useup(scroll);
	    return(1);
#ifdef TOURIST
	} else if(scroll->oclass == ARMOR_CLASS
		&& scroll->ohaluengr
		&& scroll->ovar1
		&& (scroll->otyp == DROVEN_PLATE_MAIL
			|| scroll->otyp == DROVEN_CHAIN_MAIL
			|| scroll->otyp == CONSORT_S_SUIT
		)
	){
		pline("There is %s engraved on the armor.",fetchHaluWard((int)scroll->ovar1));
		return(1);
	}else if (scroll->otyp == T_SHIRT) {
	    static const char *shirt_msgs[] = { /* Scott Bigham */
    "I explored the Dungeons of Doom and all I got was this lousy T-shirt!",
    "Is that Mjollnir in your pocket or are you just happy to see me?",
    "It's not the size of your sword, it's how #enhance'd you are with it.",
    "Madame Elvira's House O' Succubi Lifetime Customer",
    "Madame Elvira's House O' Succubi Employee of the Month",
    "Rogues Do It From Behind", /*the original D&D joke now fits*/
    "Ludios Vault Guards Do It In Small, Dark Rooms",
    "Yendor Military Soldiers Do It In Large Groups",
    "Arcadian Ants Do It In Swarms",
    "Autons Do It Under Close Supervision",
    "I survived Yendor Military Boot Camp",
    "Ludios Accounting School Intra-Mural Lacrosse Team",
    "Oracle(TM) Fountains 10th Annual Wet T-Shirt Contest",
    "Hey, black dragon!  Disintegrate THIS!",
    "I'm With Stupid -->",
    "Don't blame me, I voted for Izchak!",
    "Don't Panic",				/* HHGTTG */
    "Furinkan High School Athletic Dept.",	/* Ranma 1/2 */
    "Hel-LOOO, Nurse!",			/* Animaniacs */
    "Stand back, I'm going to try MAGIC!",
	"Magic. It works, witches!",
    "=^.^=",
    "100% goblin hair - do not wash",
    "Aberzombie and Fitch",
    "Aim >>> <<< here",
    "cK -- Cockatrice touches the Kop",
    "Croesus for President 2008 - Campaign Finance Reform Now (for the other guy)",
    "- - - - - - CUT HERE - - - - - -",
    "Don't ask me, I only adventure here",
    "Down With Pants!",
    "d, your dog or a killer?",
    "FREE PUG AND NEWT!",
    "Gehennoms Angels",
    "Glutton For Punishment",
    "Go Team Ant!",
    "Got Newt?",
    "Heading for Godhead",
    "Hello, my darlings!", /* Charlie Drake */
    "Hey! Nymphs! Steal This T-Shirt!",
    "I <3 Dungeon of Doom",
    "I <3 Maud",
    "I am a Valkyrie. If you see me running, try to keep up.",
    "I Am Not a Pack Rat - I Am a Collector",
    "I bounced off a rubber tree",
    "If you can read this, I can hit you with my polearm",
    "I'm Confused!",
    "I met Carl, the swordmaster of Jambalaya island and all I got was this lousy t-shirt",
    "I'm in ur base, killin ur doods",
    "I scored with the princess",
    "I Support Single Succubi",
    "I want to live forever or die in the attempt.",
    "I'll stick it in you!",
    "Kop Killaz",
    "Lichen Park",
    "LOST IN THOUGHT - please send search party",
    "Meat Is Mordor",
    "Minetown Better Business Bureau",
    "Minetown Watch",
    "Ms. Palm's House of Negotiable Affection -- A Very Reputable House Of Disrepute",
    "^^  My eyes are up there!  ^^",
    "Neferet/Pelias '08",
    "Next time you wave at me, use more than one finger, please.",
    "No Outfit Is Complete Without a Little Cat Fur",
    "Objects In This Shirt Are Closer Than They Appear",
    "Protection Racketeer",
    "P Happens",
    "Real men love Crom",
    "Rodney in '08. OR ELSE!",
    "Sokoban Gym -- Get Strong or Die Trying",
    "Somebody stole my Mojo!",
    "The Hellhound Gang",
    "The Werewolves",
    "They Might Be Storm Giants",
    "Up with miniskirts!",
    "Weapons don't kill people, I kill people",
    "Where's the beef?",
    "White Zombie",
    "Worship me",
    "You laugh because I'm different, I laugh because you're about to die",
    "You're killing me!",
    "You should hear what the voices in my head are saying about you.",
    "Anhur State University - Home of the Fighting Fire Ants!",
    "FREE HUGS",
    "Serial Ascender",
    "Plunder Island Brimstone Beach Club",
    "Real Men Are Valkyries",
    "Young Men's Cavedigging Association",
    "YMRCIGBSA - recovering mind, body, and spirit since 1844",
    "YHPA - Young Humanoid's Pagan Association",
    "Occupy Fort Ludios",
    "I couldn't afford this T-shirt so I stole it!",
    "End Mercantile Opacity Discrimination Now: Let Invisible Customers Shop!",
    "Elvira's House O'Succubi, granting the gift of immorality!",
    "Mind Flayers Suck",
    "I'm not wearing any pants",
    "Newt Fodder",
    "My Dog ate Og",
    "End Lich Prejudice Now!",
    "Down With The Living!",
    "Pudding Farmer",
    "Dudley",
    "I pray to Our Lady of Perpetual Mood Swings",
    "Soooo, When's the Wizard Getting Back to You About That Brain?",
    "Vegetarian",
    "I plan to go to Astral",
    "If They Don't Have Fruit Juice in Heaven, I Ain't Going",
    "Living Dead",
	
	/* #nethack suggestions */
    "Lich Park",
    "I'd rather be pudding farming",
	
    "Great Cthulhu for President -- why settle for the lesser evil?",
	    };
	    char buf[BUFSZ];
	    int erosion;
		if(arti_mandala(scroll)){
			if(uarmu && uarmu == scroll && uarm){
				if( uarm->obj_material == GLASS){
					You("look at your shirt through your glass armor.");
				}
				else{
					if (Blind) You_cant("see that.");
					else You_cant("see that through your %s.", aobjnam(uarm, (char *)0));
					return 0;
				}
			}
			if (Blind) You("feel as though you can see the image, despite your blindness.");
			You("feel a sense of peace come over you as you study the mandala.");
			use_unicorn_horn(scroll);
			return(1);
		}
		else{
			if (Blind) {
				You_cant("feel any Braille writing.");
				return 0;
			}
			if(uarmu && uarmu == scroll && uarm){
				if( uarm->obj_material == GLASS){
					You("look at your shirt through your glass armor.");
				}
				else{
					You_cant("read that through your %s.", aobjnam(uarm, (char *)0));
					return 0;
				}
			}
			u.uconduct.literate++;
			if(flags.verbose)
			pline("It reads:");
			Strcpy(buf, shirt_msgs[scroll->o_id % SIZE(shirt_msgs)]);
			erosion = greatest_erosion(scroll);
			if (erosion)
			wipeout_text(buf,
				(int)(strlen(buf) * erosion / (2*MAX_ERODE)),
				     scroll->o_id ^ (unsigned)u.ubirthday);
			pline("\"%s\"", buf);
			return 1;
		}
#endif	/* TOURIST */
	} else if (scroll->oclass != SCROLL_CLASS
		&& scroll->oclass != SPBOOK_CLASS) {
	    pline(silly_thing_to, "read");
	    return(0);
	} else if (Blind) {
	    const char *what = 0;
	    if (scroll->oclass == SPBOOK_CLASS)
		what = "mystic runes";
	    else if (!scroll->dknown)
		what = "formula on the scroll";
	    if (what) {
		pline("Being blind, you cannot read the %s.", what);
		return(0);
	    }
	}

	/* Actions required to win the game aren't counted towards conduct */
	if (scroll->otyp != SPE_BOOK_OF_THE_DEAD &&
#ifdef MAIL
	    scroll->otyp != SCR_MAIL &&
#endif
		scroll->otyp != SPE_BLANK_PAPER &&
		scroll->otyp != SCR_BLANK_PAPER){
			if(scroll->ostolen && u.sealsActive&SEAL_ANDROMALIUS) unbind(SEAL_ANDROMALIUS, TRUE);
			u.uconduct.literate++;
		}

	confused = (Confusion != 0);
#ifdef MAIL
	if (scroll->otyp == SCR_MAIL) confused = FALSE;
#endif
	if(scroll->oclass == SPBOOK_CLASS) {
	    return(study_book(scroll));
	}
	scroll->in_use = TRUE;	/* scroll, not spellbook, now being read */
	if(scroll->oartifact) {
		if(Blind) {
			pline("Being blind, you cannot see the %s.", the(xname(scroll)));
			return 0;
		}
		pline("You examine %s.", the(xname(scroll)));
	} else if(scroll->otyp != SCR_BLANK_PAPER) {
	  if(Blind)
	    pline("As you %s the formula on it, the scroll disappears.",
			is_silent(youracedata) ? "cogitate" : "pronounce");
	  else
	    pline("As you read the scroll, it disappears.");
	  if(confused) {
	    if (Hallucination)
		pline("Being so trippy, you screw up...");
	    else{
			pline("Being confused, you mis%s the magic words...",
				is_silent(youracedata) ? "understand" : "pronounce");
			if(u.sealsActive&SEAL_PAIMON) unbind(SEAL_PAIMON,TRUE);
		}
	  }
	}
	if(!seffects(scroll))  {
		if(!objects[scroll->otyp].oc_name_known) {
		    if(known) {
			makeknown(scroll->otyp);
			more_experienced(0,10);
		    } else if(!objects[scroll->otyp].oc_uname)
			docall(scroll);
		}
		if(scroll->otyp != SCR_BLANK_PAPER && !scroll->oartifact)
			useup(scroll);
		else scroll->in_use = FALSE;
	}
	return(1);
}

static void
stripspe(obj)
register struct obj *obj;
{
	if (obj->blessed) pline1(nothing_happens);
	else {
		if (obj->spe > 0) {
		    obj->spe = 0;
		    if (obj->otyp == OIL_LAMP || obj->otyp == BRASS_LANTERN)
			obj->age = 0;
		    Your("%s %s briefly.",xname(obj), otense(obj, "vibrate"));
		} else pline1(nothing_happens);
	}
}

static void
p_glow1(otmp)
register struct obj	*otmp;
{
	Your("%s %s briefly.", xname(otmp),
	     otense(otmp, Blind ? "vibrate" : "glow"));
}

static void
p_glow2(otmp,color)
register struct obj	*otmp;
register const char *color;
{
	Your("%s %s%s%s for a moment.",
		xname(otmp),
		otense(otmp, Blind ? "vibrate" : "glow"),
		Blind ? "" : " ",
		Blind ? nul : hcolor(color));
}

/* Is the object chargeable?  For purposes of inventory display; it is */
/* possible to be able to charge things for which this returns FALSE. */
boolean
is_chargeable(obj)
struct obj *obj;
{
	if (obj->oclass == WAND_CLASS) return TRUE;
	/* known && !uname is possible after amnesia/mind flayer */
	if (obj->oclass == RING_CLASS)
	    return (boolean)(objects[obj->otyp].oc_charged &&
			(obj->known || objects[obj->otyp].oc_uname));
	if (is_lightsaber(obj) && obj->oartifact != ART_INFINITY_S_MIRRORED_ARC)
	    return TRUE;
//#ifdef FIREARMS
	if (is_blaster(obj) && (obj->recharged < 4 || (obj->otyp != HAND_BLASTER && obj->otyp != ARM_BLASTER)))
	    return TRUE;
	if ((obj->otyp == FORCE_PIKE || obj->otyp == VIBROBLADE || obj->otyp == SEISMIC_HAMMER))
	    return TRUE;
//#endif
	if (is_weptool(obj))	/* specific check before general tools */
	    return FALSE;
	if (obj->oclass == TOOL_CLASS)
	    return (boolean)(objects[obj->otyp].oc_charged);
	return FALSE; /* why are weapons/armor considered charged anyway? */
}

/*
 * recharge an object; curse_bless is -1 if the recharging implement
 * was cursed, +1 if blessed, 0 otherwise.
 */
void
recharge(obj, curse_bless)
struct obj *obj;
int curse_bless;
{
	register int n;
	boolean is_cursed, is_blessed;

	is_cursed = curse_bless < 0;
	is_blessed = curse_bless > 0;

	if (obj->oclass == WAND_CLASS) {
	    /* undo any prior cancellation, even when is_cursed */
	    if (obj->spe == -1) obj->spe = 0;

	    /*
	     * Recharging might cause wands to explode.
	     *	v = number of previous recharges
	     *	      v = percentage chance to explode on this attempt
	     *		      v = cumulative odds for exploding
	     *	0 :   0       0
	     *	1 :   0.29    0.29
	     *	2 :   2.33    2.62
	     *	3 :   7.87   10.28
	     *	4 :  18.66   27.02
	     *	5 :  36.44   53.62
	     *	6 :  62.97   82.83
	     *	7 : 100     100
	     */
	    n = (int)obj->recharged;
	    if (!(obj->oartifact) && n > 0 && (obj->otyp == WAN_WISHING ||
		    (n * n * n > rn2(7*7*7)))) {	/* recharge_limit */
		wand_explode(obj);
		return;
	    }
	    /* didn't explode, so increment the recharge count */
	    if(n < 7) obj->recharged = (unsigned)(n + 1);

	    /* now handle the actual recharging */
	    if (is_cursed) {
		stripspe(obj);
	    } else {
		int lim = (obj->otyp == WAN_WISHING) ? 3 :
			(objects[obj->otyp].oc_dir != NODIR) ? 8 : 15;

		n = (lim == 3) ? 3 : rn1(5, lim + 1 - 5);
		if (!is_blessed) n = rnd(n);

		if (obj->spe < n) obj->spe = n;
		else obj->spe++;
		if (obj->otyp == WAN_WISHING && obj->spe > 3) {
		    wand_explode(obj);
		    return;
		}
		if (obj->spe >= lim) p_glow2(obj, NH_BLUE);
		else p_glow1(obj);
	    }

	} else if (obj->oclass == RING_CLASS &&
					objects[obj->otyp].oc_charged) {
	    /* charging does not affect ring's curse/bless status */
	    int s = is_blessed ? rnd(3) : is_cursed ? -rnd(2) : 1;
	    boolean is_on = (obj == uleft || obj == uright);

	    /* destruction depends on current state, not adjustment */
	    if (obj->spe > (6-rnl(7)) || obj->spe <= -5) {
			if(!obj->oartifact){
				Your("%s %s momentarily, then %s!",
					 xname(obj), otense(obj,"pulsate"), otense(obj,"explode"));
				if (is_on) Ring_gone(obj);
				s = rnd(3 * abs(obj->spe));	/* amount of damage */
				useup(obj);
				losehp(s, "exploding ring", KILLED_BY_AN);
			} else {
				long mask = is_on ? (obj == uleft ? LEFT_RING :
							 RIGHT_RING) : 0L;
				Your("%s %s momentarily!", xname(obj), otense(obj,"pulsate"));
				/* cause attributes and/or properties to be updated */
				if (is_on) Ring_off(obj);
				obj->spe = 0;	/* update the ring while it's off */
				if (is_on) setworn(obj, mask), Ring_on(obj);
				/* oartifact: if a touch-sensitive artifact ring is
				   ever created the above will need to be revised  */
			}
		} else {
			long mask = is_on ? (obj == uleft ? LEFT_RING :
						 RIGHT_RING) : 0L;
			Your("%s spins %sclockwise for a moment.",
				 xname(obj), s < 0 ? "counter" : "");
			/* cause attributes and/or properties to be updated */
			if (is_on) Ring_off(obj);
			obj->spe += s;	/* update the ring while it's off */
			if (is_on) setworn(obj, mask), Ring_on(obj);
			/* oartifact: if a touch-sensitive artifact ring is
			   ever created the above will need to be revised  */
	    }

	} else if (obj->oclass == TOOL_CLASS || is_blaster(obj)
		   || obj->otyp == DWARVISH_HELM || obj->otyp == VIBROBLADE 
		   || obj->otyp == FORCE_PIKE || obj->otyp == SEISMIC_HAMMER) {
	    int rechrg = (int)obj->recharged;

	    if (objects[obj->otyp].oc_charged) {
		/* tools don't have a limit, but the counter used does */
		if (rechrg < 7)	/* recharge_limit */
		    obj->recharged++;
	    }
	    switch(obj->otyp) {
	    case BELL_OF_OPENING:
		if (is_cursed) stripspe(obj);
		else if (is_blessed) obj->spe += rnd(3);
		else obj->spe += 1;
		if (obj->spe > 5) obj->spe = 5;
		break;
	    case MAGIC_MARKER:
	    case TINNING_KIT:
#ifdef TOURIST
	    case EXPENSIVE_CAMERA:
#endif
		if (is_cursed) stripspe(obj);
		else if (rechrg && obj->otyp == MAGIC_MARKER) {	/* previously recharged */
		    obj->recharged = 1;	/* override increment done above */
		    if (obj->spe < 3)
			Your("marker seems permanently dried out.");
		    else
			pline1(nothing_happens);
		} else if (is_blessed) {
		    n = rn1(16,15);		/* 15..30 */
		    if (obj->spe + n <= 50)
			obj->spe = 50;
		    else if (obj->spe + n <= 75)
			obj->spe = 75;
		    else {
			int chrg = (int)obj->spe;
			if ((chrg + n) > 127)
				obj->spe = 127;
			else
				obj->spe += n;
		    }
		    p_glow2(obj, NH_BLUE);
		} else {
		    n = rn1(11,10);		/* 10..20 */
		    if (obj->spe + n <= 50)
			obj->spe = 50;
		    else {
			int chrg = (int)obj->spe;
			if ((chrg + n) > 127)
				obj->spe = 127;
			else
				obj->spe += n;
		    }
		    p_glow2(obj, NH_WHITE);
		}
		break;
	    case DWARVISH_HELM:
	    case OIL_LAMP:
	    case BRASS_LANTERN:
		if (is_cursed) {
		    if (obj->otyp == DWARVISH_HELM) {
			/* Don't affect the +/- of the helm */
			obj->age = 0;
		    }
		    else 
				stripspe(obj);
		    if (obj->lamplit) {
			if (!Blind)
			    pline("%s out!", Tobjnam(obj, "go"));
			end_burn(obj, TRUE);
		    }
		} else if (is_blessed) {
		    if (obj->otyp != DWARVISH_HELM) {
				obj->spe = 1;
		    }
		    obj->age = 1500;
		    p_glow2(obj, NH_BLUE);
		} else {
		    if (obj->otyp != DWARVISH_HELM) {
				obj->spe = 1;
		    }
		}
//#ifdef FIREARMS
	    case HAND_BLASTER:
	    case ARM_BLASTER:
			if(obj->recharged > 4){
				obj->recharged = 4;
			} else {
				if(is_blessed) obj->ovar1 = 100L;
				else if(is_cursed) obj->ovar1 = 10L;
				else obj->ovar1 = 80L + rn2(20);
			}
		break;
	    case MASS_SHADOW_PISTOL:
			if(is_blessed) obj->ovar1 = 1000L;
			else if(is_cursed) obj->ovar1 = 100L;
			else obj->ovar1 = 800L + rn2(200);
		break;
	    case CUTTING_LASER:
	    case VIBROBLADE:
	    case FORCE_PIKE:
	    case SEISMIC_HAMMER:
			if(is_blessed) obj->ovar1 = 100L;
			else if(is_cursed) obj->ovar1 = 10L;
			else obj->ovar1 = 80L + rn2(20);
		break;
	    case RAYGUN:
			if(Role_if(PM_ANACHRONONAUT) || Role_if(PM_TOURIST)){
				if(is_blessed) obj->ovar1 = 160L;
				else if(is_cursed) obj->ovar1 = 10L;
				else obj->ovar1 = (8 + rn2(8))*10L;
			} else {
				//The Raygun's power cell is damaged:
				if(is_blessed) obj->ovar1 = 15L;
				else if(is_cursed) obj->ovar1 = 2L;
				else obj->ovar1 = 2+rnd(5)*2;
			}
		break;
//#endif
	    case LIGHTSABER:
	    case BEAMSWORD:
	    case DOUBLE_LIGHTSABER:
		if (is_cursed) {
		    if (obj->lamplit) {
			end_burn(obj, TRUE);
			obj->age = 0;
			if (!Blind)
			    pline("%s deactivates!", The(xname(obj)));
		    } else
			obj->age = 0;
		} else if (is_blessed) {
		    obj->age = 150000;
		    p_glow2(obj, NH_BLUE);
		} else {
		    obj->age += 75000;
		    if (obj->age > 150000) obj->age = 150000;
		    p_glow1(obj);
		}
		break;
	    case SENSOR_PACK:
			if(is_blessed) obj->spe = 25;
			else if(is_cursed) obj->spe = 0;
			else obj->spe = rn1(12,12);
		break;
	    case CRYSTAL_BALL:
		if (is_cursed) stripspe(obj);
		else if (is_blessed) {
		    obj->spe = 6;
		    p_glow2(obj, NH_BLUE);
		} else {
		    if (obj->spe < 5) {
			obj->spe++;
			p_glow1(obj);
		    } else pline1(nothing_happens);
		}
		break;
	    case HORN_OF_PLENTY:
	    case BAG_OF_TRICKS:
	    case CAN_OF_GREASE:
		if (is_cursed) stripspe(obj);
		else if (is_blessed) {
		    if (obj->spe <= 10)
			obj->spe += rn1(10, 6);
		    else obj->spe += rn1(5, 6);
		    if (obj->spe > 50) obj->spe = 50;
		    p_glow2(obj, NH_BLUE);
		} else {
		    obj->spe += rnd(5);
		    if (obj->spe > 50) obj->spe = 50;
		    p_glow1(obj);
		}
		break;
	    case MAGIC_FLUTE:
	    case MAGIC_HARP:
	    case FROST_HORN:
	    case FIRE_HORN:
	    case DRUM_OF_EARTHQUAKE:
		if (is_cursed) {
		    stripspe(obj);
		} else if (is_blessed) {
		    obj->spe += d(2,4);
		    if (obj->spe > 20) obj->spe = 20;
		    p_glow2(obj, NH_BLUE);
		} else {
		    obj->spe += rnd(4);
		    if (obj->spe > 20) obj->spe = 20;
		    p_glow1(obj);
		}
		break;
	    default:
		goto not_chargable;
		/*NOTREACHED*/
		break;
	    } /* switch */

	} else {
 not_chargable:
	    You("have a feeling of loss.");
	}
}


/* Forget known information about this object class. */
static void
forget_single_object(obj_id)
	int obj_id;
{
	objects[obj_id].oc_name_known = 0;
	objects[obj_id].oc_pre_discovered = 0;	/* a discovery when relearned */
	if (objects[obj_id].oc_uname) {
	    free((genericptr_t)objects[obj_id].oc_uname);
	    objects[obj_id].oc_uname = 0;
	}
	undiscover_object(obj_id);	/* after clearing oc_name_known */

	/* clear & free object names from matching inventory items too? */
}


#if 0	/* here if anyone wants it.... */
/* Forget everything known about a particular object class. */
static void
forget_objclass(oclass)
	int oclass;
{
	int i;

	for (i=bases[oclass];
		i < NUM_OBJECTS && objects[i].oc_class==oclass; i++)
	    forget_single_object(i);
}
#endif


/* randomize the given list of numbers  0 <= i < count */
static void
randomize(indices, count)
	int *indices;
	int count;
{
	int i, iswap, temp;

	for (i = count - 1; i > 0; i--) {
	    if ((iswap = rn2(i + 1)) == i) continue;
	    temp = indices[i];
	    indices[i] = indices[iswap];
	    indices[iswap] = temp;
	}
}


/* Forget % of known objects. */
void
forget_objects(percent)
	int percent;
{
	int i, count;
	int indices[NUM_OBJECTS];
	
	return; //Disable object forgetting
	
	if (percent == 0) return;
	if (percent <= 0 || percent > 100) {
	    impossible("forget_objects: bad percent %d", percent);
	    return;
	}

	for (count = 0, i = 1; i < NUM_OBJECTS; i++)
	    if (OBJ_DESCR(objects[i]) &&
		    (objects[i].oc_name_known || objects[i].oc_uname))
		indices[count++] = i;

	randomize(indices, count);
	
	/* forget first % of randomized indices */
	count = ((count * percent) + 50) / 100;
	for (i = 0; i < count; i++)
	    forget_single_object(indices[i]);
	
	if(rn2(100) < percent) u.uevent.uread_necronomicon = 0; /* potentially forget having read necronomicon */
}


/* Forget some or all of map (depends on parameters). */
void
forget_map(howmuch)
	int howmuch;
{
	register int zx, zy;

	if (In_sokoban(&u.uz))
	    return;

	known = TRUE;
	for(zx = 0; zx < COLNO; zx++) for(zy = 0; zy < ROWNO; zy++)
		if (howmuch >= 100 || rn2(100) < howmuch) {
			/* Zonk all memory of this location. */
			levl[zx][zy].seenv = 0;
			levl[zx][zy].waslit = 0;
			levl[zx][zy].glyph = cmap_to_glyph(S_stone);
			levl[zx][zy].styp = STONE;
		}
}

/* Forget all traps on the level. */
void
forget_traps()
{
	register struct trap *trap;

	/* forget all traps (except the one the hero is in :-) */
	for (trap = ftrap; trap; trap = trap->ntrap)
	    if ((trap->tx != u.ux || trap->ty != u.uy) && (trap->ttyp != HOLE))
		trap->tseen = 0;
}

/*
 * Forget given % of all levels that the hero has visited and not forgotten,
 * except this one.
 */
void
forget_levels(percent)
	int percent;
{
	int i, count;
	xchar  maxl, this_lev;
	int indices[MAXLINFO];

	if (percent == 0) return;

	if (percent <= 0 || percent > 100) {
	    impossible("forget_levels: bad percent %d", percent);
	    return;
	}

	this_lev = ledger_no(&u.uz);
	maxl = maxledgerno();

	/* count & save indices of non-forgotten visited levels */
	/* Sokoban levels are pre-mapped for the player, and should stay
	 * so, or they become nearly impossible to solve.  But try to
	 * shift the forgetting elsewhere by fiddling with percent
	 * instead of forgetting fewer levels.
	 */
	for (count = 0, i = 0; i <= maxl; i++)
	    if ((level_info[i].flags & VISITED) &&
			!(level_info[i].flags & FORGOTTEN) && i != this_lev) {
		if (ledger_to_dnum(i) == sokoban_dnum)
		    percent += 2;
		else
		    indices[count++] = i;
	    }
	
	if (percent > 100) percent = 100;

	randomize(indices, count);

	/* forget first % of randomized indices */
	count = ((count * percent) + 50) / 100;
	for (i = 0; i < count; i++) {
	    level_info[indices[i]].flags |= FORGOTTEN;
//		forget_mapseen(indices[i]);
	}
}

/*
 * Forget some things (e.g. after reading a scroll of amnesia).  When called,
 * the following are always forgotten:
 *
 *	- felt ball & chain
 *	- traps
 *	- part (6 out of 7) of the map
 *
 * Other things are subject to flags:
 *
 */
void
forget(howmuch)
int howmuch;
{
	u.wimage = 0; //clear wa image from your mind
	
	if(u.sealsActive&SEAL_HUGINN_MUNINN){
		unbind(SEAL_HUGINN_MUNINN,TRUE);
		return; //expel and end
	}
	if (Punished) u.bc_felt = 0;	/* forget felt ball&chain */

	forget_map(howmuch);
	forget_traps();

	/* 1 in 3 chance of forgetting some levels */
	if (howmuch && !rn2(3)) forget_levels(howmuch);

	/* 1 in 3 chance of forgeting some objects */
	if (howmuch && !rn2(3)) forget_objects(howmuch);

	if (howmuch) losespells(howmuch);
	/*
	 * Make sure that what was seen is restored correctly.  To do this,
	 * we need to go blind for an instant --- turn off the display,
	 * then restart it.  All this work is needed to correctly handle
	 * walls which are stone on one side and wall on the other.  Turning
	 * off the seen bits above will make the wall revert to stone,  but
	 * there are cases where we don't want this to happen.  The easiest
	 * thing to do is to run it through the vision system again, which
	 * is always correct.
	 */
	docrt();		/* this correctly will reset vision */
}

/* monster is hit by scroll of taming's effect */
static void
maybe_tame(mtmp, sobj)
struct monst *mtmp;
struct obj *sobj;
{
	if (sobj->cursed) {
	    setmangry(mtmp);
	} else {
	    if (mtmp->isshk)
			make_happy_shk(mtmp, FALSE);
	    else if (sobj->otyp == SPE_CHARM_MONSTER){
			int skill = spell_skilltype(sobj->otyp);
			int role_skill = P_SKILL(skill)-1; //P_basic would be 2
			if(Spellboost) role_skill++;
			if(role_skill < 1) role_skill = 1;
			
			for(role_skill; role_skill; role_skill--)
				if(!resist(mtmp, sobj->oclass, 0, NOTELL)){
					(void) tamedog(mtmp, sobj);
					return;
				}
	    } else if (!resist(mtmp, sobj->oclass, 0, NOTELL))
			(void) tamedog(mtmp, sobj);
	}
}

int
seffects(sobj)
struct obj	*sobj;
{
	int cval;
	boolean confused = (Confusion != 0);
	struct obj *otmp;
	boolean extra = FALSE;
	

	if (objects[sobj->otyp].oc_magic)
		exercise(A_WIS, TRUE);		/* just for trying */
	switch(sobj->otyp) {
#ifdef MAIL
	case SCR_MAIL:
		known = TRUE;
		if (sobj->spe)
		    pline("This seems to be junk mail addressed to the finder of the Eye of Larn.");
		/* note to the puzzled: the game Larn actually sends you junk
		 * mail if you win!
		 */
		else readmail(sobj);
		break;
#endif
	case SCR_ENCHANT_ARMOR:
	    {
		register schar s;
		boolean special_armor;
		boolean same_color;

		otmp = some_armor(&youmonst);
		if(!otmp) {
			strange_feeling(sobj,
					!Blind ? "Your skin glows then fades." :
					"Your skin feels warm for a moment.");
			exercise(A_CON, !sobj->cursed);
			exercise(A_STR, !sobj->cursed);
			return(1);
		}
		if(confused) {
			otmp->oerodeproof = !(sobj->cursed);
			if (otmp->otyp >= GRAY_DRAGON_SCALES &&
				otmp->otyp <= YELLOW_DRAGON_SCALES) {
				/* dragon scales get turned into dragon scale shield */
				if(Blind) {
					Your("%s %s harder!", xname(otmp), otense(otmp, "feel"));
				} else {
					Your("%s merges and hardens!", xname(otmp));
				}
				setworn((struct obj *)0, W_ARMS);
				setworn((struct obj *)0, W_ARM);
				/* assumes same order */
				otmp->otyp = GRAY_DRAGON_SCALE_SHIELD +
					otmp->otyp - GRAY_DRAGON_SCALES;
				otmp->objsize = youracedata->msize;
				otmp->cursed = 0;
				if (sobj->blessed) {
					otmp->spe++;
					otmp->blessed = 1;
				}
				otmp->known = 1;
				fix_object(otmp);
				setworn(otmp, W_ARMS);
				break;
			} else {
				if(Blind) {
					otmp->rknown = FALSE;
					Your("%s %s warm for a moment.",
					xname(otmp), otense(otmp, "feel"));
				} else {
					otmp->rknown = TRUE;
					Your("%s %s covered by a %s %s %s!",
					xname(otmp), otense(otmp, "are"),
					sobj->cursed ? "mottled" : "shimmering",
					 hcolor(sobj->cursed ? NH_BLACK : NH_GOLDEN),
					sobj->cursed ? "glow" :
					  (is_shield(otmp) ? "layer" : "shield"));
				}
				if (otmp->oerodeproof &&
					(otmp->oeroded || otmp->oeroded2)) {
					otmp->oeroded = otmp->oeroded2 = 0;
					Your("%s %s as good as new!",
					 xname(otmp),
					 otense(otmp, Blind ? "feel" : "look"));
				}
			}
			break;
		}
		/* elven armor vibrates warningly when enchanted beyond a limit */
		special_armor = is_elven_armor(otmp) ||
			arti_plussev(otmp) || /* special artifact armors */
			(Role_if(PM_WIZARD) && otmp->otyp == CORNUTHAUM) ||
			(Role_if(PM_WIZARD) && otmp->otyp == ROBE && otmp->oartifact == ART_ROBE_OF_THE_ARCHMAGI) ||
			otmp->otyp == CRYSTAL_HELM ||
			otmp->otyp == CRYSTAL_PLATE_MAIL ||
			otmp->otyp == CRYSTAL_SHIELD ||
			otmp->otyp == CRYSTAL_GAUNTLETS ||
			otmp->otyp == CRYSTAL_BOOTS ||
			otmp->otyp == PLATE_MAIL || /*plate mails have finer manufacturing and can be more highly enchanted*/
			otmp->otyp == BRONZE_PLATE_MAIL ||
			otmp->otyp == CLOAK_OF_PROTECTION;//Cloaks of protection are specialized defensive items.
		if (sobj->cursed)
		    same_color =
			(otmp->otyp == BLACK_DRAGON_SCALE_MAIL ||
			 otmp->otyp == BLACK_DRAGON_SCALE_SHIELD ||
			 otmp->otyp == BLACK_DRAGON_SCALES);
		else
		    same_color =
			(otmp->otyp == SILVER_DRAGON_SCALE_MAIL ||
			 otmp->otyp == SILVER_DRAGON_SCALE_SHIELD ||
			 otmp->otyp == SILVER_DRAGON_SCALES ||
			 otmp->otyp == SHIELD_OF_REFLECTION);
		if (Blind) same_color = FALSE;

		/* KMH -- catch underflow */
		s = sobj->cursed ? -otmp->spe : otmp->spe;
		if (s > (special_armor ? 5 : 3) && rn2(s)) {
			if(otmp->oartifact){
				Your("%s violently %s%s%s for a while, then %s.",
					 xname(otmp),
					 otense(otmp, Blind ? "vibrate" : "glow"),
					 (!Blind && !same_color) ? " " : nul,
					 (Blind || same_color) ? nul :
					hcolor(sobj->cursed ? NH_BLACK : NH_SILVER),
					 otense(otmp, "fade"));
					otmp->spe = 0;
			} else {
				Your("%s violently %s%s%s for a while, then %s.",
					 xname(otmp),
					 otense(otmp, Blind ? "vibrate" : "glow"),
					 (!Blind && !same_color) ? " " : nul,
					 (Blind || same_color) ? nul :
					hcolor(sobj->cursed ? NH_BLACK : NH_SILVER),
					 otense(otmp, "evaporate"));
				if(is_cloak(otmp)) (void) Cloak_off();
				if(is_boots(otmp)) (void) Boots_off();
				if(is_helmet(otmp)) (void) Helmet_off();
				if(is_gloves(otmp)) (void) Gloves_off();
				if(is_shield(otmp)) (void) Shield_off();
				if(otmp == uarm) (void) Armor_gone();
				useup(otmp);
			}
			break;
		}
		s = sobj->cursed ? -1 :
		    otmp->spe >= 9 ? (rn2(otmp->spe) == 0) :
		    sobj->blessed ? rnd(3-otmp->spe/3) : 1;
		if (s >= 0 && otmp->otyp >= GRAY_DRAGON_SCALES &&
					otmp->otyp <= YELLOW_DRAGON_SCALES) {
			/* dragon scales get turned into dragon scale mail */
			Your("%s merges and hardens!", xname(otmp));
			setworn((struct obj *)0, W_ARM);
			/* assumes same order */
			otmp->otyp = GRAY_DRAGON_SCALE_MAIL +
						otmp->otyp - GRAY_DRAGON_SCALES;
			otmp->objsize = youracedata->msize;
			otmp->bodytypeflag = youracedata->mflagsb&MB_BODYTYPEMASK;
			otmp->cursed = 0;
			if (sobj->blessed) {
				otmp->spe++;
				otmp->blessed = 1;
			}
			otmp->known = 1;
			setworn(otmp, W_ARM);
			break;
		}
		Your("%s %s%s%s%s for a %s.",
			xname(otmp),
		        s == 0 ? "violently " : nul,
			otense(otmp, Blind ? "vibrate" : "glow"),
			(!Blind && !same_color) ? " " : nul,
			(Blind || same_color) ? nul : hcolor(sobj->cursed ? NH_BLACK : NH_SILVER),
			  (s*s>1) ? "while" : "moment");
		otmp->cursed = sobj->cursed;
		if (!otmp->blessed || sobj->cursed)
			otmp->blessed = sobj->blessed;
		if (s) {
			otmp->spe += s;
			adj_abon(otmp, s);
			known = otmp->known;
		}

		if ((otmp->spe > (special_armor ? 5 : 3)) &&
		    (special_armor || !rn2(7)))
			Your("%s suddenly %s %s.",
				xname(otmp), otense(otmp, "vibrate"),
				Blind ? "again" : "unexpectedly");
		break;
	    }
	case SCR_DESTROY_ARMOR:
	    {
		otmp = some_armor(&youmonst);
		if(confused) {
			if(!otmp) {
				char tempBuff[BUFSZ];
				Sprintf(tempBuff, "Your %s itch.", body_part(BONES));
				strange_feeling(sobj,tempBuff);
//				strange_feeling(sobj,"Your bones itch.");
				exercise(A_STR, FALSE);
				exercise(A_CON, FALSE);
				return(1);
			}
			otmp->oerodeproof = sobj->cursed;
			p_glow2(otmp, NH_PURPLE);
			break;
		}
		if(!sobj->cursed || !otmp || !otmp->cursed) {
		    if(!destroy_arm(otmp)) {
			strange_feeling(sobj,"Your skin itches.");
			exercise(A_STR, FALSE);
			exercise(A_CON, FALSE);
			return(1);
		    } else
			known = TRUE;
		} else {	/* armor and scroll both cursed */
		    Your("%s %s.", xname(otmp), otense(otmp, "vibrate"));
		    if (otmp->spe >= -6) otmp->spe--;
		    make_stunned(HStun + rn1(10, 10), TRUE);
		}
	    }
	    break;
	case SCR_CONFUSE_MONSTER:
	case SPE_CONFUSE_MONSTER:
		if(youracedata->mlet != S_HUMAN || sobj->cursed) {
			if(!HConfusion) You_feel("confused.");
			make_confused(HConfusion + rnd(100),FALSE);
		} else  if(confused) {
		    if(!sobj->blessed) {
			Your("%s begin to %s%s.",
			    makeplural(body_part(HAND)),
			    Blind ? "tingle" : "glow ",
			    Blind ? nul : hcolor(NH_PURPLE));
			make_confused(HConfusion + rnd(100),FALSE);
		    } else {
			pline("A %s%s surrounds your %s.",
			    Blind ? nul : hcolor(NH_RED),
			    Blind ? "faint buzz" : " glow",
			    body_part(HEAD));
			make_confused(0L,TRUE);
		    }
		} else {
		    if (!sobj->blessed) {
			Your("%s%s %s%s.",
			makeplural(body_part(HAND)),
			Blind ? "" : " begin to glow",
			Blind ? (const char *)"tingle" : hcolor(NH_RED),
			u.umconf ? " even more" : "");
			u.umconf++;
		    } else {
			if (Blind)
			    Your("%s tingle %s sharply.",
				makeplural(body_part(HAND)),
				u.umconf ? "even more" : "very");
			else
			    Your("%s glow a%s brilliant %s.",
				makeplural(body_part(HAND)),
				u.umconf ? "n even more" : "",
				hcolor(NH_RED));
			/* after a while, repeated uses become less effective */
			if (u.umconf >= 40)
			    u.umconf++;
			else
			    u.umconf += rn1(8, 2);
		    }
		}
		break;
	case SCR_SCARE_MONSTER:
	case SPE_CAUSE_FEAR:
	    {	register int ct = 0;
		register struct monst *mtmp;

		for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		    if (DEADMONSTER(mtmp)) continue;
		    if(cansee(mtmp->mx,mtmp->my)) {
			if(confused || sobj->cursed) {
			    mtmp->mflee = mtmp->mfrozen = mtmp->msleeping = 0;
			    mtmp->mcanmove = 1;
			} else
			    if (! resist(mtmp, sobj->oclass, 0, NOTELL))
				monflee(mtmp, 0, FALSE, FALSE);
			if(!mtmp->mtame) ct++;	/* pets don't laugh at you */
		    }
		}
		if(!ct)
		      You_hear("%s in the distance.",
			       (confused || sobj->cursed) ? "sad wailing" :
							"maniacal laughter");
		else if(sobj->otyp == SCR_SCARE_MONSTER)
			You_hear("%s close by.",
				  (confused || sobj->cursed) ? "sad wailing" :
						 "maniacal laughter");
		break;
	    }
	case SCR_BLANK_PAPER:
	    if (Blind)
		You("don't remember there being any magic words on this scroll.");
	    else
		pline("This scroll seems to be blank.");
	    known = TRUE;
	    break;
	case SCR_REMOVE_CURSE:
	case SPE_REMOVE_CURSE:
	    {	register struct obj *obj;
		if(confused)
		    if (Hallucination)
			You_feel("the power of the Force against you!");
		    else
			You_feel("like you need some help.");
		else
		    if (Hallucination)
			You_feel("in touch with the Universal Oneness.");
		    else
			You_feel("like someone is helping you.");

		if (sobj->cursed) {
		    pline_The("scroll disintegrates.");
		} else {
			if(!confused && u.sealsActive&SEAL_MARIONETTE){
				unbind(SEAL_MARIONETTE,TRUE);
			} 
			if(!confused) u.wimage = 0;
		    for (obj = invent; obj; obj = obj->nobj) {
			long wornmask;
#ifdef GOLDOBJ
			/* gold isn't subject to cursing and blessing */
			if (obj->oclass == COIN_CLASS) continue;
#endif
			wornmask = (obj->owornmask & ~(W_BALL|W_ART|W_ARTI));
			if (wornmask && !sobj->blessed) {
			    /* handle a couple of special cases; we don't
			       allow auxiliary weapon slots to be used to
			       artificially increase number of worn items */
			    if (obj == uswapwep) {
				if (!u.twoweap) wornmask = 0L;
			    } else if (obj == uquiver) {
				if (obj->oclass == WEAPON_CLASS) {
				    /* mergeable weapon test covers ammo,
				       missiles, spears, daggers & knives */
				    if (!objects[obj->otyp].oc_merge) 
					wornmask = 0L;
				} else if (obj->oclass == GEM_CLASS) {
				    /* possibly ought to check whether
				       alternate weapon is a sling... */
				    if (!uslinging()) wornmask = 0L;
				} else {
				    /* weptools don't merge and aren't
				       reasonable quivered weapons */
				    wornmask = 0L;
				}
			    }
			}
			if (sobj->blessed || wornmask ||
			     obj->otyp == LOADSTONE ||
			     (obj->otyp == LEASH && obj->leashmon)) {
			    if(confused) blessorcurse(obj, 2);
			    else uncurse(obj);
			}
		    }
		}
		if(Punished && !confused) unpunish();
		update_inventory();
		break;
	    }
	case SCR_CREATE_MONSTER:
	case SPE_CREATE_MONSTER:
	    if (create_critters(1 + ((confused || sobj->cursed) ? 12 : 0) +
				((sobj->blessed || rn2(73)) ? 0 : rnd(4)),
			confused ? &mons[PM_ACID_BLOB] : (struct permonst *)0))
		known = TRUE;
	    /* no need to flush monsters; we ask for identification only if the
	     * monsters are not visible
	     */
	    break;
	case SCR_ENCHANT_WEAPON:
		if(uwep && (uwep->oclass == WEAPON_CLASS || is_weptool(uwep))
			&& confused && uwep->oartifact != ART_ROD_OF_SEVEN_PARTS) {
		/* read text for the rod of seven parts may lead players to think they need to errode-proof it.
			Since this is a logical deduction, it is not penalized. CM */
		/* oclass check added 10/25/86 GAN */
			uwep->oerodeproof = !(sobj->cursed);
			if (Blind) {
			    uwep->rknown = FALSE;
			    Your("weapon feels warm for a moment.");
			} else {
			    uwep->rknown = TRUE;
			    Your("%s covered by a %s %s %s!",
				aobjnam(uwep, "are"),
				sobj->cursed ? "mottled" : "shimmering",
				hcolor(sobj->cursed ? NH_PURPLE : NH_GOLDEN),
				sobj->cursed ? "glow" : "shield");
			}
			if (uwep->oerodeproof && (uwep->oeroded || uwep->oeroded2)) {
			    uwep->oeroded = uwep->oeroded2 = 0;
			    Your("%s as good as new!",
				 aobjnam(uwep, Blind ? "feel" : "look"));
			}
		} else return !chwepon(sobj,
				       sobj->cursed ? -1 :
				       !uwep ? 1 :
				       uwep->spe >= 9 ? (rn2(uwep->spe) == 0) :
				       sobj->blessed ? rnd(3-uwep->spe/3) : 1);
		break;
	case SCR_TAMING:
	case SPE_CHARM_MONSTER:
		if (u.uswallow) {
		    maybe_tame(u.ustuck, sobj);
		} else {
		    struct monst *mtmp;
			if(confused){
				int i, j, bd = 5;
				for (i = -bd; i <= bd; i++) for(j = -bd; j <= bd; j++) {
					if (!isok(u.ux + i, u.uy + j)) continue;
					mtmp = m_at(u.ux + i, u.uy + j);
					if(mtmp){
						if(!mtmp->mtame){
							boolean waspeaceful = mtmp->mpeaceful;
							maybe_tame(mtmp, sobj);
							mtmp = m_at(u.ux + i, u.uy + j); /* Old monster was freed if it was tamed */
							if(mtmp && mtmp->mtame){
								EDOG(mtmp)->friend = TRUE;
								mtmp->mtame += ACURR(A_CHA)*10;
								EDOG(mtmp)->waspeaceful = waspeaceful;
								if(!waspeaceful) mtmp->mpeacetime += ACURR(A_CHA);
							}
						} else if(EDOG(mtmp)->friend){
							mtmp->mtame += ACURR(A_CHA)*10;
							if(mtmp->mpeacetime) mtmp->mpeacetime += ACURR(A_CHA);
						} else if(mtmp->mpeacetime) mtmp->mpeacetime += ACURR(A_CHA);
					}
				}
			} else {
				getdir((char *)0);
				while(!(u.dx || u.dy)){
					u.dx = rn2(3)-1;
					u.dy = rn2(3)-1;
				}
				if (!isok(u.ux + u.dx, u.uy + u.dy)) break;
				mtmp = m_at(u.ux + u.dx, u.uy + u.dy);
				if(mtmp){
					if(!mtmp->mtame){
						maybe_tame(mtmp, sobj);
					} else if(EDOG(mtmp)->friend){
						mtmp->mtame += ACURR(A_CHA)*10;
						if(mtmp->mpeacetime) mtmp->mpeacetime += ACURR(A_CHA);
					} else if(mtmp->mpeacetime) mtmp->mpeacetime += ACURR(A_CHA);
				}
			}
		}
		break;
	case SCR_GENOCIDE:
		You("have found a scroll of genocide!");
		known = TRUE;
		if (sobj->blessed) do_class_genocide();
		else do_genocide(!sobj->cursed | (2 * !!Confusion));
		break;
	case SCR_LIGHT:
		if(!Blind) known = TRUE;
		litroom(!confused && !sobj->cursed, sobj);
		if(!confused && !sobj->cursed && u.sealsActive&SEAL_TENEBROUS) unbind(SEAL_TENEBROUS,TRUE);
		break;
	case SCR_TELEPORTATION:
		if(confused || sobj->cursed) level_tele();
		else {
			if (sobj->blessed && !Teleport_control) {
				known = TRUE;
				if (yn("Do you wish to teleport?")=='n')
					break;
			}
			tele();
			if(Teleport_control || !couldsee(u.ux0, u.uy0) ||
			   (distu(u.ux0, u.uy0) >= 16))
				known = TRUE;
		}
		break;
	case SCR_GOLD_DETECTION:
		if (confused || sobj->cursed) return(trap_detect(sobj));
		else return(gold_detect(sobj));
	case SCR_FOOD_DETECTION:
	case SPE_DETECT_FOOD:
		if (food_detect(sobj))
			return(1);	/* nothing detected */
		break;
	case SPE_IDENTIFY:
		cval = rn2(5);
		goto id;
	case SCR_IDENTIFY:
		/* known = TRUE; */
		if(confused)
			You("identify this as an identify scroll.");
		else
			pline("This is an identify scroll.");
		if (sobj->blessed || (!sobj->cursed && !rn2(5))) {
			cval = rn2(5);
			/* Note: if rn2(5)==0, identify all items */
			if (cval == 1 && sobj->blessed && Luck > 0) ++cval;
		} else	cval = 1;
		if(!objects[sobj->otyp].oc_name_known) more_experienced(0,10);
		useup(sobj);
		makeknown(SCR_IDENTIFY);
	id:
		if(invent && !confused) {
		    identify_pack(cval);
		}
		return(1);
	case SCR_CHARGING:
		if (confused) {
		    You_feel("charged up!");
		    if (u.uen < u.uenmax)
				u.uen = min(u.uen+400, u.uenmax);
		    else
				u.uen = (u.uenmax += d(5,4));
		    flags.botl = 1;
		    break;
		}
		known = TRUE;
		pline("This is a charging scroll.");
		otmp = getobj(all_count, "charge");
		if (!otmp) break;
		recharge(otmp, sobj->cursed ? -1 : (sobj->blessed ? 1 : 0));
		break;
	case SCR_MAGIC_MAPPING:
		if (level.flags.nommap) {
		    Your("mind is filled with crazy lines!");
		    if (Hallucination)
			pline("Wow!  Modern art.");
		    else
			Your("%s spins in bewilderment.", body_part(HEAD));
		    make_confused(HConfusion + rnd(30), FALSE);
		    break;
		}
		if (sobj->blessed && !(sobj->oartifact)) {
		    register int x, y;

		    for (x = 1; x < COLNO; x++)
		    	for (y = 0; y < ROWNO; y++)
		    	    if (levl[x][y].typ == SDOOR)
		    	    	cvt_sdoor_to_door(&levl[x][y]);
		    /* do_mapping() already reveals secret passages */
		}
		if((sobj->blessed || sobj->oartifact) && Is_paradise(&u.uz)){
			struct obj* obj;
			for (obj = level.buriedobjlist; obj; obj = obj->nobj) {
				if(obj->otyp == CHEST || obj->otyp == MAGIC_LAMP){
					int x = obj->ox, y = obj->oy;
					extra = TRUE;
					make_engr_at(x,y,"X",moves,BURN);
					if(isok(x+1,y+1)) make_engr_at(x+1,y+1,"\\",moves,BURN);
					if(isok(x+1,y-1)) make_engr_at(x+1,y-1,"/",moves,BURN);
					if(isok(x-1,y+1)) make_engr_at(x-1,y+1,"/",moves,BURN);
					if(isok(x-1,y-1)) make_engr_at(x-1,y-1,"\\",moves,BURN);
				} else if(obj->otyp == FIGURINE){
					int x = obj->ox, y = obj->oy;
					make_engr_at(x,y,"x",moves,BURN);
				}
			}
		}
		known = TRUE;
	case SPE_MAGIC_MAPPING:
		if (level.flags.nommap) {
		    Your("%s spins as %s blocks the spell!", body_part(HEAD), something);
		    make_confused(HConfusion + rnd(30), FALSE);
		    break;
		}
		if(!(sobj->oartifact)){
		pline("A map coalesces in your mind!");
		if(extra) pline("X marks the spot!");
		cval = (sobj->cursed && !confused);
		if(cval) HConfusion = 1;	/* to screw up map */
		do_mapping();
		if(cval) {
		    HConfusion = 0;		/* restore */
		    pline("Unfortunately, you can't grasp the details.");
		}
		}
		else{
			if(sobj->age > monstermoves){
				pline("The map %s hard to see.", vtense((char *)0,"are"));
				nomul(-1*rnd(3), "studying a magic map");
				sobj->age += (long) d(3,10);
			} else sobj->age = monstermoves + (long) d(3,10);
			do_vicinity_map(u.ux,u.uy);
		}
		break;
	case SCR_AMNESIA:
		known = TRUE;
		forget(sobj->cursed ? 25 : sobj->blessed ? 0 : 10);
		if (Hallucination) /* Ommmmmm! */
			Your("mind releases itself from mundane concerns.");
		else if (!strncmpi(plname, "Maud", 4))
			pline("As your mind turns inward on itself, you forget everything else.");
		else if (rn2(2))
			pline("Who was that Maud person anyway?");
		else
			pline("Thinking of Maud you forget everything else.");
		exercise(A_WIS, FALSE);
		break;
	case SCR_FIRE:{
		/*
		 * Note: Modifications have been made as of 3.0 to allow for
		 * some damage under all potential cases.
		 */
		int damlevel = max(3, u.ulevel);
		cval = bcsign(sobj);
		if(Fire_resistance) cval *= -1;//if you resist fire, blessed does more damage than cursed
		if(!objects[sobj->otyp].oc_name_known) more_experienced(0,10);
		useup(sobj);
		makeknown(SCR_FIRE);
		if(confused) {
		    if(Fire_resistance) {
			shieldeff(u.ux, u.uy);
			if(!Blind)
			    pline("Oh, look, what a pretty fire in your %s.",
				makeplural(body_part(HAND)));
			else You_feel("a pleasant warmth in your %s.",
				makeplural(body_part(HAND)));
		    } else {
			pline_The("scroll catches fire and you burn your %s.",
				makeplural(body_part(HAND)));
			losehp(1, "scroll of fire", KILLED_BY_AN);
		    }
		    return(1);
		}
		if (Underwater)
			pline_The("water around you vaporizes violently!");
		else {
		    pline_The("scroll erupts in a tower of flame!");
		    burn_away_slime();
		}
		explode(u.ux, u.uy, 11, (2*(rn1(damlevel, damlevel) - (damlevel-1) * cval) + 1)/3,
							SCROLL_CLASS, EXPL_FIERY);
		return(1);
	}
	case SCR_EARTH:
	    /* TODO: handle steeds */
	    if (
#ifdef REINCARNATION
		!Is_rogue_level(&u.uz) && 
#endif
	    	 (!In_endgame(&u.uz) || Is_earthlevel(&u.uz))) {
	    	register int x, y;

	    	/* Identify the scroll */
	    	pline_The("%s rumbles %s you!", ceiling(u.ux,u.uy),
	    			sobj->blessed ? "around" : "above");
	    	known = 1;
	    	if (In_sokoban(&u.uz))
	    	    change_luck(-1);	/* Sokoban guilt */

	    	/* Loop through the surrounding squares */
	    	if (!sobj->cursed) for (x = u.ux-1; x <= u.ux+1; x++) {
	    	    for (y = u.uy-1; y <= u.uy+1; y++) {

	    	    	/* Is this a suitable spot? */
	    	    	if (isok(x, y) && !closed_door(x, y) &&
	    	    			!IS_ROCK(levl[x][y].typ) &&
	    	    			!IS_AIR(levl[x][y].typ) &&
					(x != u.ux || y != u.uy)) {
			    register struct obj *otmp2;
			    register struct monst *mtmp;

	    	    	    /* Make the object(s) */
	    	    	    otmp2 = mksobj(confused ? ROCK : BOULDER,
	    	    	    		FALSE, FALSE);
	    	    	    if (!otmp2) continue;  /* Shouldn't happen */
	    	    	    otmp2->quan = confused ? rn1(5,2) : 1;
	    	    	    otmp2->owt = weight(otmp2);

	    	    	    /* Find the monster here (won't be player) */
	    	    	    mtmp = m_at(x, y);
	    	    	    if (mtmp && !amorphous(mtmp->data) &&
	    	    	    		!passes_walls(mtmp->data) &&
	    	    	    		!noncorporeal(mtmp->data) &&
	    	    	    		!unsolid(mtmp->data)) {
				struct obj *helmet = which_armor(mtmp, W_ARMH);
				int mdmg;

				if (cansee(mtmp->mx, mtmp->my)) {
				    pline("%s is hit by %s!", Monnam(mtmp),
	    	    	    			doname(otmp2));
				    if (mtmp->minvis && !canspotmon(mtmp))
					map_invisible(mtmp->mx, mtmp->my);
				}
	    	    	    	mdmg = dmgval(otmp2, mtmp, 0) * otmp2->quan;
				if (helmet) {
				    if(is_hard(helmet)) {
					if (canspotmon(mtmp))
					    pline("Fortunately, %s is wearing a hard helmet.", mon_nam(mtmp));
					else if (flags.soundok)
					    You_hear("a clanging sound.");
					if (mdmg > 2) mdmg = 2;
				    } else {
					if (canspotmon(mtmp))
					    pline("%s's %s does not protect %s.",
						Monnam(mtmp), xname(helmet),
						mhim(mtmp));
				    }
				}
	    	    	    	mtmp->mhp -= mdmg;
	    	    	    	if (mtmp->mhp <= 0)
	    	    	    	    xkilled(mtmp, 1);
	    	    	    }
	    	    	    /* Drop the rock/boulder to the floor */
	    	    	    if (!flooreffects(otmp2, x, y, "fall")) {
	    	    	    	place_object(otmp2, x, y);
	    	    	    	stackobj(otmp2);
	    	    	    	newsym(x, y);  /* map the rock */
	    	    	    }
	    	    	}
		    }
		}
		/* Attack the player */
		if (!sobj->blessed) {
		    int dmg;
		    struct obj *otmp2;

		    /* Okay, _you_ write this without repeating the code */
		    otmp2 = mksobj(confused ? ROCK : BOULDER,
				FALSE, FALSE);
		    if (!otmp2) break;
		    otmp2->quan = confused ? rn1(5,2) : 1;
		    otmp2->owt = weight(otmp2);
		    if (!amorphous(youracedata) &&
				!Passes_walls &&
				!noncorporeal(youracedata) &&
				!unsolid(youracedata)) {
			You("are hit by %s!", doname(otmp2));
			dmg = dmgval(otmp2, &youmonst, 0) * otmp2->quan;
			if (uarmh && !sobj->cursed) {
			    if(is_hard(uarmh)) {
				pline("Fortunately, you are wearing a hard helmet.");
				if (dmg > 2) dmg = 2;
			    } else if (flags.verbose) {
				Your("%s does not protect you.",
						xname(uarmh));
			    }
			}
		    } else
			dmg = 0;
		    /* Must be before the losehp(), for bones files */
		    if (!flooreffects(otmp2, u.ux, u.uy, "fall")) {
			place_object(otmp2, u.ux, u.uy);
			stackobj(otmp2);
			newsym(u.ux, u.uy);
		    }
		    if (dmg) losehp(dmg, "scroll of earth", KILLED_BY_AN);
		}
	    }
	    break;
	case SCR_PUNISHMENT:
		known = TRUE;
		if(confused || sobj->blessed) {
			You_feel("guilty.");
			break;
		}
		punish(sobj);
		break;
	case SCR_WARD:{
		if(confused && !Hallucination){
		register struct monst *mtmp;
			//Scare nearby monsters.
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
				if (DEADMONSTER(mtmp)) continue;
				if(distu(mtmp->mx,mtmp->my) <= 2 ) {
					if(sobj->cursed) {
						mtmp->mflee = mtmp->mfrozen = mtmp->msleeping = 0;
						mtmp->mcanmove = 1;
					} else if (! resist(mtmp, sobj->oclass, 0, NOTELL)){
						if (rn2(7))
							monflee(mtmp, rnd(10), TRUE, TRUE);
						else
							monflee(mtmp, rnd(100), TRUE, TRUE);
					}
				}
			}
		}
		struct engr *engrHere = engr_at(u.ux,u.uy);
		if(is_lava(u.ux, u.uy)){
			pline("The lava shifts and flows beneath you.");
	break;
		}
		if (IS_ALTAR(levl[u.ux][u.uy].typ)) {
			pline("The ground around the altar shifts briefly."); 
			altar_wrath(u.ux, u.uy);
	break;
		}
		pline("The %s shifts beneath you,%sengraving a %s ward.", 
			surface(u.ux,u.uy),
			engrHere ? " wiping away the existing engraving and " : " ",
			wardDecode[sobj->ovar1]
		);
		known = TRUE;
		del_engr_ward_at(u.ux,u.uy);
		make_engr_at(u.ux, u.uy,	"", (moves - multi), DUST); /* absense of text =  dust */
			
		engrHere = engr_at(u.ux,u.uy); /*note: make_engr_at does not return the engraving it made, it returns void instead*/
		if(Hallucination){
			//Scribe hallucinatory ward
			engrHere->halu_ward = TRUE;
			engrHere->ward_type = ENGRAVE;
			engrHere->ward_id = randHaluWard();
	break;
		}
		else{
			engrHere->ward_id = sobj->ovar1;
			if(sobj->cursed){
				if(is_pool(u.ux, u.uy, TRUE)){
					pline("The lines of blood quickly disperse into the water.");
	break;
				}
				engrHere->ward_type = ENGR_BLOOD;
			} else engrHere->ward_type = ENGRAVE;
			engrHere->complete_wards = sobj->blessed ? 
										wardMax[sobj->ovar1] : 
										get_num_wards_added(engrHere->ward_id,0);
			if( !(u.wardsknown & get_wardID(sobj->ovar1)) ){
				You("have learned a new warding sign!");
				u.wardsknown |= get_wardID(sobj->ovar1);
			}
		}
	break;}
	case SCR_WARDING:{
		//Confused: Scribe random ward that you DON'T know.
		//Hallucination: A ripple of %color energy.... Changes all wards to hallucinatory random wards.
		if(sobj->cursed){
			//Cursed: Destroy all wards on the level
			pline("A ripple of black fire spreads out over the %s.", surface(u.ux,u.uy));
			blank_all_wards();
	break;
		}
		else if(Hallucination){
			//Convert all wards to random wards
			pline("A ripple of %s fire spreads out over the %s.", hcolor(0), surface(u.ux,u.uy));
			randomize_all_wards();
	break;
		}
		struct engr *engrHere = engr_at(u.ux,u.uy);
		int wardNum;
		if(!confused) wardNum = pick_ward(FALSE);
		else wardNum = random_unknown_ward();
//		pline("%d",wardNum);
		known = TRUE;
		if(!wardNum) /*Player cancelled prompt*/
	break;
		if(is_lava(u.ux, u.uy)){
			pline("Brilliant fire dances over the lava for a moment.");
	break;
		}
		if (IS_ALTAR(levl[u.ux][u.uy].typ)) {
			pline("Brilliant fire dances over the altar briefly."); 
			altar_wrath(u.ux, u.uy);
	break;
		}
		pline("Brilliant fire plays over the %s, burning a %s ward into it.", 
			surface(u.ux,u.uy),
			wardNum != -1 ? wardDecode[wardNum] : "Cerulean Sign"
		);
		known = TRUE;
		if(!engrHere){
			make_engr_at(u.ux, u.uy,	"", (moves - multi), DUST); /* absense of text =  dust */
			engrHere = engr_at(u.ux,u.uy); /*note: make_engr_at does not return the engraving it made, it returns void instead*/
		}
		
		if(wardNum == -1){ /*Confused reading of scroll with no unknown wards remaining. Engrave Cerulean Sign, which can't be learned*/
			engrHere->halu_ward = TRUE;
			engrHere->ward_id = CERULEAN_SIGN;
			engrHere->ward_type = BURN;
			engrHere->complete_wards = 1;
		}
		else if(engrHere->ward_id != wardNum 
				|| engrHere->ward_type != BURN){
			engrHere->ward_id = wardNum;
			engrHere->ward_type = BURN;
			engrHere->complete_wards = sobj->blessed ? 
										wardMax[wardNum] : 
										get_num_wards_added(engrHere->ward_id,0);
		}
		else{
			engrHere->complete_wards = sobj->blessed ? 
										wardMax[wardNum] : 
										engrHere->complete_wards + get_num_wards_added(engrHere->ward_id, engrHere->complete_wards);
		}
	break;}
	case SCR_STINKING_CLOUD: {
	        coord cc;

		You("have found a scroll of stinking cloud!");
		known = TRUE;
		pline("Where do you want to center the cloud?");
		cc.x = u.ux;
		cc.y = u.uy;
		if (getpos(&cc, TRUE, "the desired position") < 0) {
		    pline1(Never_mind);
		    return 0;
		}
		if (!cansee(cc.x, cc.y) || distu(cc.x, cc.y) >= 32) {
		    You("smell rotten eggs.");
		    return 0;
		}
		(void) create_gas_cloud(cc.x, cc.y, 3+bcsign(sobj),
						8+4*bcsign(sobj));
		break;
	}
	case SCR_ANTIMAGIC:{
		if(confused && sobj->cursed){
			//Confused
			pline("Shimmering sparks shoot into your body!");
			if(u.uen + 400 > u.uenmax){
				u.uenmax += 4;
				u.uen = u.uenmax;
			} else u.uen += 400;
	break;
		}
		if(sobj->cursed){
			//Cursed: Attacks YOUR magic
			pline("Shimmering sparks shoot from your body!");
			u.uen = max(0,u.uen-400);
	break;
		}
		if(confused){
			//Confused: Remove magic shield
			if(HNullmagic && ( HNullmagic & ~TIMEOUT) == 0L)
				pline("The shimmering film around you pops!");
			else pline("Nothing happens.");
			HNullmagic &= ~TIMEOUT;
	break;
		}
		if(!Nullmagic) pline("A shimmering film surrounds you!");
		else pline("The shimmering film grows brighter!");
		if( (HNullmagic & TIMEOUT) + 400L < TIMEOUT) {
			long timer = (HNullmagic & TIMEOUT) + 400L;
			HNullmagic &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HNullmagic |= timer; //set new timer
		}
		else{
			HNullmagic |= TIMEOUT; //set timer to max value
		}
	}break;
	case SCR_RESISTANCE:{
		if(confused){
			int damlevel = max(3, u.ulevel), sx = u.ux, sy = u.uy;
			cval = bcsign(sobj);
			sx = u.ux+rnd(3)-2; 
			sy = u.uy+rnd(3)-2;
			if (!isok(sx,sy) ||
				IS_STWALL(levl[sx][sy].typ) || u.uswallow) {
				/* Spell is reflected back to center */
				sx = u.ux; 
				sy = u.uy;
			}
			explode(sx, sy, 11, (2*(rn1(damlevel, damlevel) - (damlevel-1) * cval) + 1)/3,
							SCROLL_CLASS, EXPL_FIERY);
			sx = u.ux+rnd(3)-2; 
			sy = u.uy+rnd(3)-2;
			if (!isok(sx,sy) ||
				IS_STWALL(levl[sx][sy].typ) || u.uswallow) {
				/* Spell is reflected back to center */
				sx = u.ux; 
				sy = u.uy;
			}
			explode(sx, sy, 12, (2*(rn1(damlevel, damlevel) - (damlevel-1) * cval) + 1)/3,
							SCROLL_CLASS, EXPL_FROSTY);
			sx = u.ux+rnd(3)-2; 
			sy = u.uy+rnd(3)-2;
			if (!isok(sx,sy) ||
				IS_STWALL(levl[sx][sy].typ) || u.uswallow) {
				/* Spell is reflected back to center */
				sx = u.ux; 
				sy = u.uy;
			}
			explode(sx, sy, 15, (2*(rn1(damlevel, damlevel) - (damlevel-1) * cval) + 1)/3,
							SCROLL_CLASS, EXPL_MAGICAL);
			sx = u.ux+rnd(3)-2; 
			sy = u.uy+rnd(3)-2;
			if (!isok(sx,sy) ||
				IS_STWALL(levl[sx][sy].typ) || u.uswallow) {
				/* Spell is reflected back to center */
				sx = u.ux; 
				sy = u.uy;
			}
			explode(sx, sy, 17, (2*(rn1(damlevel, damlevel) - (damlevel-1) * cval) + 1)/3,
							SCROLL_CLASS, EXPL_NOXIOUS);
	break;
		}
		long rturns = sobj->blessed ? 5000L : sobj->cursed ? 5L : 250L;
		if( !(HFire_resistance) ) {
			You(Hallucination ? "be chillin'." :
			    "feel a momentary chill.");
		}
		if( (HFire_resistance & TIMEOUT) + rturns < TIMEOUT) {
			long timer = (HFire_resistance & TIMEOUT) + rturns;
			HFire_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HFire_resistance |= timer; //set new timer
		}
		else{
			HFire_resistance |= TIMEOUT; //set timer to max value
		}
		
		if( !(HSleep_resistance) ) {
			You_feel("wide awake.");
		}
		if( (HSleep_resistance & TIMEOUT) + rturns < TIMEOUT) {
			long timer = (HSleep_resistance & TIMEOUT) + rturns;
			HSleep_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HSleep_resistance |= timer; //set new timer
		}
		else{
			HSleep_resistance |= TIMEOUT; //set timer to max value
		}
		
		if( !(HCold_resistance) ) {
			You_feel("full of hot air.");
		}
		if( (HCold_resistance & TIMEOUT) + rturns < TIMEOUT) {
			long timer = (HCold_resistance & TIMEOUT) + rturns;
			HCold_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HCold_resistance |= timer; //set new timer
		}
		else{
			HCold_resistance |= TIMEOUT; //set timer to max value
		}
		
		if( !(HShock_resistance) ) {
			if (Hallucination)
				rn2(2) ? You_feel("grounded in reality.") : Your("health currently feels amplified!");
			else
				You_feel("well grounded.");
		}
		if( (HShock_resistance & TIMEOUT) + rturns < TIMEOUT) {
			long timer = (HShock_resistance & TIMEOUT) + rturns;
			HShock_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HShock_resistance |= timer; //set new timer
		}
		else{
			HShock_resistance |= TIMEOUT; //set timer to max value
		}
		
		if( !(HAcid_resistance) ) {
			if (Hallucination)
				rn2(2) ? You_feel("like you've really gotten back to basics!") : You_feel("insoluble.");
			else
				Your("skin feels leathery.");
		}
		if( (HAcid_resistance & TIMEOUT) + rturns < TIMEOUT) {
			// long timer = max((HAcid_resistance & TIMEOUT), (long)(nutval*multiplier));
			long timer = (HAcid_resistance & TIMEOUT) + rturns;
			HAcid_resistance &= ~TIMEOUT; //wipe old timer, leaving higher bits in place
			HAcid_resistance |= timer; //set new timer
		}
		else{
			HAcid_resistance |= TIMEOUT; //set timer to max value
		}
	}break;
	case SCR_CONSECRATION:
	if(In_endgame(&u.uz)){
		if(Is_astralevel(&u.uz)) pline("This place is already pretty consecrated.");
		else pline("It would seem base matter alone cannot be consecrated.");
	break;
	} else if(Is_sanctum(&u.uz)){
		pline("This place is much too unholy for the scroll to work.");
	break;
	} else {
		aligntyp whichgod;
		if(sobj->cursed || In_hell(&u.uz)){
			whichgod = A_NONE;
		} else if(confused){
			whichgod = (aligntyp)(rn2(3) - 1);
		} else whichgod = u.ualign.type;
		
		if(levl[u.ux][u.uy].typ == CORR || levl[u.ux][u.uy].typ == ROOM){
			levl[u.ux][u.uy].typ = ALTAR;
			levl[u.ux][u.uy].altarmask = Align2amask( whichgod );
			pline("%s altar appears in front of you!", An(align_str(whichgod)));
			newsym(u.ux, u.uy);
		} else angrygods(whichgod);
	}break;
	case SCR_GOLD_SCROLL_OF_LAW: {
		register struct monst *mtmp;
		aligntyp mal, ual = u.ualign.type;
	    int i, j, bd = confused ? 7 : 1;
		You("read from the golden scroll.");
		if(u.ualign.type == A_LAWFUL && u.ualign.record > 20){
		 pline("It is a log-book from the Eternal Library.");
		 pline("It's meaning is clear in your mind, and the pronunciation obvious.");
		 known = TRUE; //id the scroll
		 more_experienced(777,0);//the knowledge from the scroll transfers to you.
		 if (Upolyd) {// the lawful energies rebuild your body
			u.mh += u.ualign.record;
			if(u.mhmax < u.mh)
				u.mh = u.mhmax;
		 }
		 else{
			u.uhp += u.ualign.record;
			if(u.uhp > u.uhpmax)
				u.uhp = u.uhpmax;
		 }
		 if (u.uswallow) {
		    if(u.ustuck->data->maligntyp > A_NEUTRAL) maybe_tame(u.ustuck, sobj);
			else if(u.ustuck->data->maligntyp == A_NEUTRAL) monflee(u.ustuck, 7, FALSE, FALSE);
			else monflee(u.ustuck, 77, FALSE, FALSE);
	break;
		 }//else:
	     for (i = -bd; i <= bd; i++) for(j = -bd; j <= bd; j++) {
			if (!isok(u.ux + i, u.uy + j)) continue;
			if ((mtmp = m_at(u.ux + i, u.uy + j)) == 0) continue;
			if (DEADMONSTER(mtmp)) continue;
			if(confused || sobj->cursed) {
			    mtmp->mflee = mtmp->mfrozen = mtmp->msleeping = 0;
			    mtmp->mcanmove = 1;
			} else{
				mal = mtmp->data->maligntyp;
				if(mal < A_NEUTRAL && !resist(mtmp, sobj->oclass, 0, NOTELL)){
					monflee(mtmp, 77, FALSE, FALSE);
				}
				else if(mal == A_NEUTRAL && !resist(mtmp, sobj->oclass, 0, NOTELL)){
					monflee(mtmp, 7, FALSE, FALSE);
				}
				else if(mal > A_NEUTRAL){//maybe_tame includes a resistence check.
				    maybe_tame(mtmp, sobj);
				}
			}
	     }
		 You_feel("guilty for destroying such a valuable text.");
		 u.ualign.record /= 2;
		 u.ualign.sins += 4;
		}
		else if(u.ualign.type == A_NEUTRAL || (u.ualign.type == A_LAWFUL && u.ualign.record > 0)){
		 pline("It's meaning is unclear, and you think you didn't pronounce it quite right.");
		 if (u.uswallow) {
			if(u.ustuck->data->maligntyp < A_NEUTRAL) monflee(u.ustuck, 7, FALSE, FALSE);
	break;
		 }//else:
	     for (i = -bd; i <= bd; i++) for(j = -bd; j <= bd; j++) {
			if (!isok(u.ux + i, u.uy + j)) continue;
			if ((mtmp = m_at(u.ux + i, u.uy + j)) == 0) continue;
			if (DEADMONSTER(mtmp)) continue;
			if(confused || sobj->cursed) {
			    mtmp->mflee = mtmp->mfrozen = mtmp->msleeping = 0;
			    mtmp->mcanmove = 1;
			} else{
				mal = mtmp->data->maligntyp;
				if(mal < A_NEUTRAL && !resist(mtmp, sobj->oclass, 0, NOTELL)){
					monflee(mtmp, 7, FALSE, FALSE);
				}
			}
	     }
		 if(u.ualign.type == A_NEUTRAL) {
			You_feel("less neutral.");
			u.ualign.record -= 8;
			u.ualign.sins += 8;
		 }
		 else{
			You_feel("guilty for destroying such a valuable text.");
			u.ualign.record -= 20;
			u.ualign.sins += 10;
		 }
		}
		else{//chaotic or poorly aligned lawful
		 You("find it quite confusing.");
		 incr_itimeout(&HConfusion, max(u.ualign.record, -1*u.ualign.record));
		 if(u.ualign.type == A_LAWFUL){
			You_feel("unworthy of such a lawful text.");
			u.ualign.record -= 8;
			u.ualign.sins += 8;
		 }
		}
	}
	break;
	default:
		impossible("What weird effect is this? (%u)", sobj->otyp);
	}
	return(0);
}

static void
wand_explode(obj)
register struct obj *obj;
{
    obj->in_use = TRUE;	/* in case losehp() is fatal */
    Your("%s vibrates violently, and explodes!",xname(obj));
    nhbell();
    losehp(rnd(2*(u.uhpmax+1)/3), "exploding wand", KILLED_BY_AN);
    useup(obj);
    exercise(A_STR, FALSE);
}

/*
 * Low-level lit-field update routine.
 */
void
set_lit(x,y,val)
int x, y;
genericptr_t val;
{
	if (val)
	    levl[x][y].lit = 1;
	else {
	    levl[x][y].lit = 0;
	    snuff_light_source(x, y);
	}
}

// lights/snuffs simple lightsources lying at the location, lighting assumes this is triggered by the player for shk purposes
// this could also be added to set_lit, I suppose
void
ranged_set_lightsources(x,y,val)
int x, y;
genericptr_t val;
{
	if (val) {
		struct obj *ispe = mksobj(SPE_LIGHT, TRUE, FALSE);
		bhitpile(ispe, bhito, x, y);
	}
	else {
		snuff_light_source(x, y);
	}
}

void
litroom(on,obj)
register boolean on;
struct obj *obj;
{
	char is_lit;	/* value is irrelevant; we use its address
			   as a `not null' flag for set_lit() */
	boolean permanent_darkness = Is_grue_level(&u.uz);
	/* first produce the text (provided you're not blind) */
	if(!on) {
		register struct obj *otmp;

		if (!Blind) {
		    if(u.uswallow) {
			pline("It seems even darker in here than before.");
			return;
		    }
		    if (uwep && artifact_light(uwep) && uwep->lamplit)
			pline("Suddenly, the only light left comes from %s!",
				the(xname(uwep)));
		    else
			You("are surrounded by darkness!");
		}

		/* the magic douses lamps, et al, too */
		for(otmp = invent; otmp; otmp = otmp->nobj)
		    if (otmp->lamplit)
			(void) snuff_lit(otmp);
		if (Blind) goto do_it;
	} else {
		if (Blind) goto do_it;
		if(u.uswallow){
			if (is_animal(u.ustuck->data))
				pline("%s %s is lit.",
				        s_suffix(Monnam(u.ustuck)),
					mbodypart(u.ustuck, STOMACH));
			else
				if (is_whirly(u.ustuck->data))
					pline("%s shines briefly.",
					      Monnam(u.ustuck));
				else
					pline("%s glistens.", Monnam(u.ustuck));
			return;
		}
		if (!permanent_darkness)
			pline("A lit field surrounds you!");
		else
			pline("A ripple of energy travels through the darkness.");
	}

do_it:
	/* No-op in water - can only see the adjacent squares and that's it! */
	if (Underwater || Is_waterlevel(&u.uz)) return;
	/*
	 *  If we are darkening the room and the hero is punished but not
	 *  blind, then we have to pick up and replace the ball and chain so
	 *  that we don't remember them if they are out of sight.
	 */
	if (Punished && (darksight(youracedata) ? on : !on) && !Blind)
	    move_bc(1, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);

#ifdef REINCARNATION
	if (Is_rogue_level(&u.uz)) {
	    /* Can't use do_clear_area because MAX_RADIUS is too small */
	    /* rogue lighting must light the entire room */
	    int rnum = levl[u.ux][u.uy].roomno - ROOMOFFSET;
	    int rx, ry;
	    if(rnum >= 0) {
		for(rx = rooms[rnum].lx-1; rx <= rooms[rnum].hx+1; rx++)
		    for(ry = rooms[rnum].ly-1; ry <= rooms[rnum].hy+1; ry++)
			set_lit(rx, ry,
				(genericptr_t)(on ? &is_lit : (char *)0));
		rooms[rnum].rlit = on;
	    }
	    /* hallways remain dark on the rogue level */
	} else
#endif
	    do_clear_area(u.ux,u.uy,
		((obj && obj->oclass==SCROLL_CLASS && obj->blessed) ? 9 : 5) * (permanent_darkness ? 3 : 2) / 2, permanent_darkness ? ranged_set_lightsources :
		set_lit, (genericptr_t)(on ? &is_lit : (char *)0));

	/*
	 *  If we are not blind, then force a redraw on all positions in sight
	 *  by temporarily blinding the hero.  The vision recalculation will
	 *  correctly update all previously seen positions *and* correctly
	 *  set the waslit bit [could be messed up from above].
	 */
	if (!Blind) {
	    vision_recalc(2);

	    /* replace ball&chain */
	    if (Punished && (darksight(youracedata) ? on : !on))
			move_bc(0, 0, uball->ox, uball->oy, uchain->ox, uchain->oy);
	}

	vision_full_recalc = 1;	/* delayed vision recalculation */
}

static void
do_class_genocide()
{
	int i, j, immunecnt, gonecnt, goodcnt, class, feel_dead = 0;
	char buf[BUFSZ];
	boolean gameover = FALSE;	/* true iff killed self */

	for(j=0; ; j++) {
		if (j >= 5) {
			pline1(thats_enough_tries);
			return;
		}
		do {
		    getlin("What class of monsters do you wish to genocide?",
			buf);
		    (void)mungspaces(buf);
		} while (buf[0]=='\033' || !buf[0]);
		/* choosing "none" preserves genocideless conduct */
		if (!strcmpi(buf, "none") ||
		    !strcmpi(buf, "nothing")) return;

		if (strlen(buf) == 1) {
		    if (buf[0] == ILLOBJ_SYM)
			buf[0] = def_monsyms[S_MIMIC];
		    class = def_char_to_monclass(buf[0]);
		} else {
		    char buf2[BUFSZ];

		    class = 0;
		    Strcpy(buf2, makesingular(buf));
		    Strcpy(buf, buf2);
		}
		immunecnt = gonecnt = goodcnt = 0;
		for (i = LOW_PM; i < NUMMONS; i++) {
		    if (class == 0 &&
			    strstri(monexplain[(int)mons[i].mlet], buf) != 0)
			class = mons[i].mlet;
		    if (mons[i].mlet == class) {
			if (!(mons[i].geno & G_GENO)) immunecnt++;
			else if(mvitals[i].mvflags & G_GENOD) gonecnt++;
			else goodcnt++;
		    }
		}
		/*
		 * TODO[?]: If user's input doesn't match any class
		 *	    description, check individual species names.
		 */
		if (!goodcnt && class != mons[urole.malenum].mlet &&
				class != mons[urace.malenum].mlet) {
			if (gonecnt)
	pline("All such monsters are already nonexistent.");
			else if (immunecnt ||
				(buf[0] == DEF_INVISIBLE && buf[1] == '\0'))
	You("aren't permitted to genocide such monsters.");
			else
#ifdef WIZARD	/* to aid in topology testing; remove pesky monsters */
			  if (wizard && buf[0] == '*') {
			    register struct monst *mtmp, *mtmp2;

			    gonecnt = 0;
			    for (mtmp = fmon; mtmp; mtmp = mtmp2) {
					mtmp2 = mtmp->nmon;
				    	if (DEADMONSTER(mtmp)) continue;
					mongone(mtmp);
					gonecnt++;
			    }
			pline("Eliminated %d monster%s.", gonecnt, plur(gonecnt));
			    return;
			} else
#endif
	pline("That symbol does not represent any monster.");
			continue;
		}

		for (i = LOW_PM; i < NUMMONS; i++) {
		    if(mons[i].mlet == class) {
			char nam[BUFSZ];

			Strcpy(nam, makeplural(mons[i].mname));
			/* Although "genus" is Latin for race, the hero benefits
			 * from both race and role; thus genocide affects either.
			 */
			if (Your_Own_Role(i) || Your_Own_Race(i) ||
				((mons[i].geno & G_GENO)
				&& !(mvitals[i].mvflags & G_GENOD))) {
			/* This check must be first since player monsters might
			 * have G_GENOD or !G_GENO.
			 */
			    mvitals[i].mvflags |= (G_GENOD|G_NOCORPSE);
			    reset_rndmonst(i);
			    kill_genocided_monsters();
			    update_inventory();		/* eggs & tins */
			    pline("Wiped out all %s.", nam);
				if(u.sealsActive&SEAL_MOTHER) unbind(SEAL_MOTHER,TRUE);
			    if (Upolyd && i == u.umonnum) {
				u.mh = -1;
				if (Unchanging) {
				    if (!feel_dead++) You("die.");
				    /* finish genociding this class of
				       monsters before ultimately dying */
				    gameover = TRUE;
				} else
				    rehumanize();
			    }
			    /* Self-genocide if it matches either your race
			       or role.  Assumption:  male and female forms
			       share same monster class. */
			    if (i == urole.malenum || i == urace.malenum) {
				u.uhp = -1;
				if (Upolyd) {
				    if (!feel_dead++) You_feel("dead inside.");
				} else {
				    if (!feel_dead++) You("die.");
				    gameover = TRUE;
				}
			    }
			} else if (mvitals[i].mvflags & G_GENOD) {
			    if (!gameover)
				pline("All %s are already nonexistent.", nam);
			} else if (!gameover) {
			  /* suppress feedback about quest beings except
			     for those applicable to our own role */
			  if ((mons[i].msound != MS_LEADER ||
			       quest_info(MS_LEADER) == i)
			   && (mons[i].msound != MS_NEMESIS ||
			       quest_info(MS_NEMESIS) == i)
			   && (mons[i].msound != MS_GUARDIAN ||
			       quest_info(MS_GUARDIAN) == i)
			/* non-leader/nemesis/guardian role-specific monster */
			   && (i != PM_NINJA ||		/* nuisance */
			       Role_if(PM_SAMURAI))) {
				boolean named, uniq;

				named = type_is_pname(&mons[i]) ? TRUE : FALSE;
				uniq = (mons[i].geno & G_UNIQ) ? TRUE : FALSE;
				/* one special case */
				if (i == PM_HIGH_PRIEST) uniq = FALSE;

				You("aren't permitted to genocide %s%s.",
				    (uniq && !named) ? "the " : "",
				    (uniq || named) ? mons[i].mname : nam);
			    }
			}
		    }
		}
		if (gameover || u.uhp == -1) {
		    killer_format = KILLED_BY_AN;
		    killer = "scroll of genocide";
		    if (gameover) done(GENOCIDED);
		}
		return;
	}
}

#define REALLY 1
#define PLAYER 2
#define ONTHRONE 4
void
do_genocide(how)
int how;
/* 0 = no genocide; create monsters (cursed scroll) */
/* 1 = normal genocide */
/* 3 = forced genocide of player */
/* 5 (4 | 1) = normal genocide from throne */
{
	char buf[BUFSZ];
	register int	i, killplayer = 0;
	register int mndx;
	register struct permonst *ptr;
	const char *which;

	if (how & PLAYER) {
		mndx = u.umonster;	/* non-polymorphed mon num */
		ptr = &mons[mndx];
		Strcpy(buf, ptr->mname);
		killplayer++;
	} else {
	    for(i = 0; ; i++) {
		if(i >= 5) {
		    pline1(thats_enough_tries);
		    return;
		}
		getlin("What monster do you want to genocide? [type the name]",
			buf);
		(void)mungspaces(buf);
		/* choosing "none" preserves genocideless conduct */
		if (!strcmpi(buf, "none") || !strcmpi(buf, "nothing")) {
		    /* ... but no free pass if cursed */
		    if (!(how & REALLY)) {
			ptr = rndmonst();
			if (!ptr) return; /* no message, like normal case */
			mndx = monsndx(ptr);
			break;		/* remaining checks don't apply */
		    } else return;
		}

		mndx = name_to_mon(buf);
		if (mndx == NON_PM || (mvitals[mndx].mvflags & G_GENOD)) {
			pline("Such creatures %s exist in this world.",
			      (mndx == NON_PM) ? "do not" : "no longer");
			continue;
		}
		ptr = &mons[mndx];
		/* Although "genus" is Latin for race, the hero benefits
		 * from both race and role; thus genocide affects either.
		 */
		if (Your_Own_Role(mndx) || Your_Own_Race(mndx)) {
			killplayer++;
			break;
		}
		if (is_human(ptr)) adjalign(-sgn(u.ualign.type));
		if (is_demon(ptr)) adjalign(sgn(u.ualign.type));

		if(!(ptr->geno & G_GENO)) {
			if(flags.soundok) {
	/* fixme: unconditional "caverns" will be silly in some circumstances */
			    if(flags.verbose)
			pline("A thunderous voice booms through the caverns:");
			    verbalize("No, mortal!  That will not be done.");
			}
			continue;
		}
		/* KMH -- Unchanging prevents rehumanization */
		if (Unchanging && ptr == youmonst.data)
		    killplayer++;
		break;
	    }
	}

	which = "all ";
	if (Hallucination) {
	    if (Upolyd)
		Strcpy(buf,youmonst.data->mname);
	    else {
		Strcpy(buf, (flags.female && urole.name.f) ?
				urole.name.f : urole.name.m);
		buf[0] = lowc(buf[0]);
	    }
	} else {
	    Strcpy(buf, ptr->mname); /* make sure we have standard singular */
	    if ((ptr->geno & G_UNIQ) && ptr != &mons[PM_HIGH_PRIEST])
		which = !type_is_pname(ptr) ? "the " : "";
	}
	if (how & REALLY) {
	    /* setting no-corpse affects wishing and random tin generation */
	    mvitals[mndx].mvflags |= (G_GENOD | G_NOCORPSE);
	    pline("Wiped out %s%s.", which,
		  (*which != 'a') ? buf : makeplural(buf));
		if(u.sealsActive&SEAL_MOTHER) unbind(SEAL_MOTHER,TRUE);

	    if (killplayer) {
		/* might need to wipe out dual role */
		if (urole.femalenum != NON_PM && mndx == urole.malenum)
		    mvitals[urole.femalenum].mvflags |= (G_GENOD | G_NOCORPSE);
		if (urole.femalenum != NON_PM && mndx == urole.femalenum)
		    mvitals[urole.malenum].mvflags |= (G_GENOD | G_NOCORPSE);
		if (urace.femalenum != NON_PM && mndx == urace.malenum)
		    mvitals[urace.femalenum].mvflags |= (G_GENOD | G_NOCORPSE);
		if (urace.femalenum != NON_PM && mndx == urace.femalenum)
		    mvitals[urace.malenum].mvflags |= (G_GENOD | G_NOCORPSE);

		u.uhp = -1;
		if (how & PLAYER) {
		    killer_format = KILLED_BY;
		    killer = "genocidal confusion";
		} else if (how & ONTHRONE) {
		    /* player selected while on a throne */
		    killer_format = KILLED_BY_AN;
		    killer = "imperious order";
		} else { /* selected player deliberately, not confused */
		    killer_format = KILLED_BY_AN;
		    killer = "scroll of genocide";
		}

	/* Polymorphed characters will die as soon as they're rehumanized. */
	/* KMH -- Unchanging prevents rehumanization */
		if (Upolyd && ptr != youmonst.data) {
			delayed_killer = killer;
			killer = 0;
			You_feel("dead inside.");
		} else
			done(GENOCIDED);
	    } else if (ptr == youmonst.data) {
		rehumanize();
	    }
	    reset_rndmonst(mndx);
	    kill_genocided_monsters();
	    update_inventory();	/* in case identified eggs were affected */
	} else {
	    int cnt = 0;

	    if (!(mons[mndx].geno & G_UNIQ) &&
		    !(mvitals[mndx].mvflags & G_GONE && !In_quest(&u.uz)))
		for (i = rn1(3, 4); i > 0; i--) {
		    if (!makemon(ptr, u.ux, u.uy, NO_MINVENT))
			break;	/* couldn't make one */
		    ++cnt;
		    if (mvitals[mndx].mvflags & G_EXTINCT && !In_quest(&u.uz))
			break;	/* just made last one */
		}
	    if (cnt)
		pline("Sent in some %s.", makeplural(buf));
	    else
		pline1(nothing_happens);
	}
}

void
punish(sobj)
register struct obj	*sobj;
{
#ifdef CONVICT
    struct obj *otmp;
#endif /* CONVICT */
	/* KMH -- Punishment is still okay when you are riding */
	if(u.sealsActive&SEAL_MALPHAS){
		You("avoid punishment for your misbehavior!");
		return;
	}
	You("are being punished for your misbehavior!");
	if(Punished && uball->owt < 1600){
		Your("iron ball gets heavier.");
		uball->owt += 160 * (1 + sobj->cursed);
		return;
	}
	if (amorphous(youracedata) || is_whirly(youracedata) || unsolid(youracedata)) {
		pline("A ball and chain appears, then falls away.");
		dropy(mkobj(BALL_CLASS, TRUE));
		return;
	}
	setworn(mkobj(CHAIN_CLASS, TRUE), W_CHAIN);
#ifdef CONVICT
    if (((otmp = carrying(HEAVY_IRON_BALL)) != 0) &&(otmp->oartifact ==
     ART_IRON_BALL_OF_LEVITATION)) {
        setworn(otmp, W_BALL);
        Your("%s chains itself to you!", xname(otmp));
    } else {
		setworn(mkobj(BALL_CLASS, TRUE), W_BALL);
    }
#else
	setworn(mkobj(BALL_CLASS, TRUE), W_BALL);
#endif /* CONVICT */
	uball->spe = 1;		/* special ball (see save) */

	/*
	 *  Place ball & chain if not swallowed.  If swallowed, the ball &
	 *  chain variables will be set at the next call to placebc().
	 */
	if (!u.uswallow) {
	    placebc();
	    if (Blind) set_bc(1);	/* set up ball and chain variables */
	    newsym(u.ux,u.uy);		/* see ball&chain if can't see self */
	}
}

void
unpunish()
{	    /* remove the ball and chain */
	struct obj *savechain = uchain;

	obj_extract_self(uchain);
	newsym(uchain->ox,uchain->oy);
	setworn((struct obj *)0, W_CHAIN);
	dealloc_obj(savechain);
	uball->spe = 0;
	setworn((struct obj *)0, W_BALL);
}

/* some creatures have special data structures that only make sense in their
 * normal locations -- if the player tries to create one elsewhere, or to revive
 * one, the disoriented creature becomes a zombie
 */
boolean
cant_create(mtype, revival)
int *mtype;
boolean revival;
{

	/* SHOPKEEPERS can be revived now */
	if (*mtype==PM_GUARD || (*mtype==PM_SHOPKEEPER && !revival)
	     || *mtype==PM_ALIGNED_PRIEST || *mtype==PM_ANGEL) {
		*mtype = PM_ZOMBIE;
		return TRUE;
	} else if (*mtype==PM_LONG_WORM_TAIL) {	/* for create_particular() */
		*mtype = PM_LONG_WORM;
		return TRUE;
	} else if (*mtype==PM_HUNTING_HORROR_TAIL) {	/* for create_particular() */
		*mtype = PM_HUNTING_HORROR;
		return TRUE;
	}
	return FALSE;
}

#ifdef WIZARD
/*
 * Make a new monster with the type controlled by the user.
 *
 * Note:  when creating a monster by class letter, specifying the
 * "strange object" (']') symbol produces a random monster rather
 * than a mimic; this behavior quirk is useful so don't "fix" it...
 */
boolean
create_particular()
{
	char buf[BUFSZ], *bufp, monclass = MAXMCLASSES;
	int which, tries, i;
	struct permonst *whichpm;
	struct monst *mtmp;
	boolean madeany = FALSE;
	boolean maketame, makepeaceful, makehostile;

	tries = 0;
	do {
	    which = urole.malenum;	/* an arbitrary index into mons[] */
	    maketame = makepeaceful = makehostile = FALSE;
	    getlin("Create what kind of monster? [type the name or symbol]",
		   buf);
	    bufp = mungspaces(buf);
	    if (*bufp == '\033') return FALSE;
	    /* allow the initial disposition to be specified */
	    if (!strncmpi(bufp, "tame ", 5)) {
		bufp += 5;
		maketame = TRUE;
	    } else if (!strncmpi(bufp, "peaceful ", 9)) {
		bufp += 9;
		makepeaceful = TRUE;
	    } else if (!strncmpi(bufp, "hostile ", 8)) {
		bufp += 8;
		makehostile = TRUE;
	    }
	    /* decide whether a valid monster was chosen */
	    if (strlen(bufp) == 1) {
		monclass = def_char_to_monclass(*bufp);
		if (monclass != MAXMCLASSES) break;	/* got one */
	    } else {
		which = name_to_mon(bufp);
		if (which >= LOW_PM) break;		/* got one */
	    }
	    /* no good; try again... */
	    pline("I've never heard of such monsters.");
	} while (++tries < 5);

	if (tries == 5) {
	    pline1(thats_enough_tries);
	} else {
	    (void) cant_create(&which, FALSE);
	    whichpm = &mons[which];
	    for (i = 0; i <= multi; i++) {
		if (monclass != MAXMCLASSES)
		    whichpm = mkclass(monclass, Inhell ? G_HELL : G_NOHELL);
		if (maketame) {
		    mtmp = makemon(whichpm, u.ux, u.uy, MM_EDOG);
		    if (mtmp) {
			initedog(mtmp);
			set_malign(mtmp);
		    }
		} else {
		    mtmp = makemon(whichpm, u.ux, u.uy, NO_MM_FLAGS);
		    if ((makepeaceful || makehostile) && mtmp) {
			mtmp->mtame = 0;	/* sanity precaution */
			mtmp->mpeaceful = makepeaceful ? 1 : 0;
			set_malign(mtmp);
		    }
		}
		if (mtmp) madeany = TRUE;
	    }
	}
	return madeany;
}
#endif /* WIZARD */

#endif /* OVLB */

/*read.c*/
