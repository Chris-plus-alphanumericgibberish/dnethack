/*	SCCS Id: @(#)spell.c	3.4	2003/01/17	*/
/*	Copyright (c) M. Stephenson 1988			  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static NEARDATA schar delay;		/* moves left for this spell */
static NEARDATA struct obj *book;	/* last/current book being xscribed */
static NEARDATA int RoSbook;		/* Read spell or Study Wards?" */

#define KEEN 20000
#define READ_SPELL 1
#define STUDY_WARD 2
#define MAINTAINED_SPELL_PW_MULTIPLIER 3
#define MAINTAINED_SPELL_HUNGER_MULTIPLIER 1
#define incrnknow(spell)        spl_book[spell].sp_know = KEEN
#define ndecrnknow(spell, knw)        spl_book[spell].sp_know = max(0, spl_book[spell].sp_know - (KEEN*knw)/100)

#define spellev(spell)		spl_book[spell].sp_lev
#define spellname(spell)	OBJ_NAME(objects[spellid(spell)])
#define spellet(spell)	\
	((char)((spell < 26) ? ('a' + spell) : ('A' + spell - 26)))

STATIC_PTR int NDECL(purifying_blast);
STATIC_PTR int NDECL(stargate);
STATIC_PTR struct permonst * NDECL(choose_crystal_summon);
STATIC_DCL int FDECL(spell_let_to_idx, (CHAR_P));
STATIC_DCL boolean FDECL(check_spirit_let, (char));
STATIC_DCL boolean FDECL(cursed_book, (struct obj *bp));
STATIC_DCL boolean FDECL(confused_book, (struct obj *));
STATIC_DCL void FDECL(deadbook, (struct obj *));
STATIC_PTR int NDECL(learn);
STATIC_DCL boolean FDECL(getspell, (int *, int));
STATIC_DCL boolean FDECL(getspirit, (int *));
STATIC_DCL boolean FDECL(spiritLets, (char *, int));
STATIC_DCL int FDECL(dospiritmenu, (const char *, int *, int));
STATIC_DCL boolean FDECL(dospellmenu, (int,int *));
STATIC_DCL void FDECL(describe_spell, (int));
STATIC_DCL int FDECL(percent_success, (int));
STATIC_DCL int NDECL(throwspell);
STATIC_DCL int NDECL(throwgaze);
STATIC_DCL void NDECL(cast_protection);
STATIC_DCL boolean FDECL(sightwedge, (int,int, int,int, int,int));
STATIC_DCL void FDECL(spell_backfire, (int));
STATIC_DCL int FDECL(spellhunger, (int));
STATIC_DCL int FDECL(isqrt, (int));
STATIC_DCL boolean FDECL(run_maintained_spell, (int));
STATIC_DCL boolean FDECL(can_maintain_spell, (int));
STATIC_DCL void NDECL(update_alternate_spells);

long FDECL(doreadstudy, (const char *));

//definition of an extern in you.h
char *wardDecode[26] = {
	"digit",
	"heptagram",
	"Gorgoneion",
	"circle of Acheron",
	"pentagram",
	"hexagram",
	"hamsa mark",
	"elder sign",
	"elder elemental eye",
	"sign of the Scion Queen Mother",
	"cartouche of the Cat Lord",
	"sketch of the wings of Garuda",
	"sigil of Cthugha",
	"brand of Ithaqua",
	"tracery of Karakal",
	"yellow sign",
	"Hypergeometric transit equation",
	"Hypergeometric stabilization equation",
	"toustefna stave",
	"dreprun stave",
	"ottastafur stave",
	"kaupaloki stave",
	"veioistafur stave",
	"thjofastafur stave",
};
/* The roles[] table lists the role-specific values for tuning
 * percent_success().
 *
 * Reasoning:
 *   spelbase, spelheal:
 *	Arc are aware of magic through historical research
 *	Bar abhor magic (Conan finds it "interferes with his animal instincts")
 *	Brd know a lot about general lore, including a bit of magic
 *	Cav are ignorant to magic
 *	Hea are very aware of healing magic through medical research
 *	Kni are moderately aware of healing from Paladin training
 *	Mon use magic to attack and defend in lieu of weapons and armor
 *	Pri are very aware of healing magic through theological research
 *	Ran avoid magic, preferring to fight unseen and unheard
 *	Rog are moderately aware of magic through trickery
 *	Sam have limited magical awareness, prefering meditation to conjuring
 *	Tou are aware of magic from all the great films they have seen
 *	Val have limited magical awareness, prefering fighting
 *	Wiz are trained mages
 *
 *	The arms penalty is lessened for trained fighters Bar, Kni, Ran,
 *	Sam, Val -
 *	the penalty is its metal interference, not encumbrance.
 *	The `spelspec' is a single spell which is fundamentally easier
 *	 for that role to cast.
 *
 *  spelspec, spelsbon:
 *	Arc map masters (SPE_MAGIC_MAPPING)
 *	Bar fugue/berserker (SPE_HASTE_SELF)
 *  Brd to put to sleep (SPE_SLEEP)
 *	Cav born to dig (SPE_DIG)
 *	Hea to heal (SPE_CURE_SICKNESS)
 *	Kni to turn back evil (SPE_TURN_UNDEAD)
 *	Mon to preserve their abilities (SPE_RESTORE_ABILITY)
 *	Pri to bless (SPE_REMOVE_CURSE)
 *	Ran to hide (SPE_INVISIBILITY)
 *	Rog to find loot (SPE_DETECT_TREASURE)
 *	Sam to be At One (SPE_CLAIRVOYANCE)
 *	Tou to smile (SPE_CHARM_MONSTER)
 *	Val control the cold (SPE_CONE_OF_COLD)
 *	Wiz all really, but SPE_MAGIC_MISSILE is their party trick
 *
 *	See percent_success() below for more comments.
 *
 *  uarmbon, uarmsbon, uarmhbon, uarmgbon, uarmfbon:
 *	Fighters find body armour & shield a little less limiting.
 *	Headgear, Gauntlets and Footwear are not role-specific (but
 *	still have an effect, except helm of brilliance, which is designed
 *	to permit magic-use).
 */

#define uarmhbon 4 /* Metal helmets interfere with the mind */
#define uarmgbon 6 /* Casting channels through the hands */
#define uarmfbon 2 /* All metal interferes to some degree */

/* since the spellbook itself doesn't blow up, don't say just "explodes" */
static const char explodes[] = "radiates explosive energy";
static const char where_to_cast[] = "Where do you want to cast the spell?";
static const char where_to_gaze[] = "Where do you want to look?";

/* convert a letter into a number in the range 0..51, or -1 if not a letter */
STATIC_OVL int
spell_let_to_idx(ilet)
char ilet;
{
    int indx;

    indx = ilet - 'a';
    if (indx >= 0 && indx < 26) return indx;
    indx = ilet - 'A';
    if (indx >= 0 && indx < 26) return indx + 26;
    return -1;
}

/* TRUE: book should be destroyed by caller */
STATIC_OVL boolean
cursed_book(bp)
	struct obj *bp;
{
	int lev = objects[bp->otyp].oc_level;
	
	if(RoSbook == STUDY_WARD) lev -= 1; //The wardings paritally protect you from the magic contained within the book.
	
	switch(rn2(lev)) {
	case 0:
		You_feel("a wrenching sensation.");
		tele();		/* teleport him */
		break;
	case 1:
		You_feel("threatened.");
		aggravate();
		break;
	case 2:
		make_blinded(Blinded + rn1(100,250),TRUE);
		break;
	case 3:
		take_gold();
		break;
	case 4:
		pline("These runes were just too much to comprehend.");
		make_confused(HConfusion + rn1(7,16),FALSE);
		break;
	case 5:
		pline_The("book was coated with contact poison!");
		if (uarmg) {
		    if (uarmg->oerodeproof || !is_corrodeable(uarmg)) {
			Your("gloves seem unaffected.");
		    } else if (uarmg->oeroded2 < MAX_ERODE) {
			if (uarmg->greased) {
			    grease_protect(uarmg, "gloves", &youmonst);
			} else {
			    Your("gloves corrode%s!",
				 uarmg->oeroded2+1 == MAX_ERODE ?
				 " completely" : uarmg->oeroded2 ?
				 " further" : "");
			    uarmg->oeroded2++;
			}
		    } else
			Your("gloves %s completely corroded.",
			     Blind ? "feel" : "look");
		    break;
		}
		/* temp disable in_use; death should not destroy the book */
		bp->in_use = FALSE;
		losestr(Poison_resistance ? rn1(2,1) : rn1(4,3));
		losehp(rnd(Poison_resistance ? 6 : 10),
		       "contact-poisoned spellbook", KILLED_BY_AN);
		bp->in_use = TRUE;
		break;
	case 6:
		if(Antimagic) {
		    shieldeff(u.ux, u.uy);
		    pline_The("book %s, but you are unharmed!", explodes);
		} else {
		    pline("As you read the book, it %s in your %s!",
			  explodes, body_part(FACE));
		    losehp(2*rnd(10)+5, "exploding rune", KILLED_BY_AN);
		}
		return TRUE;
	default:
		rndcurse();
		break;
	}
	return FALSE;
}

/* study while confused: returns TRUE if the book is destroyed */
STATIC_OVL boolean
confused_book(spellbook)
struct obj *spellbook;
{
	boolean gone = FALSE;

	if (!rn2(3) && spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
	    spellbook->in_use = TRUE;	/* in case called from learn */
	    pline(
	"Being confused you have difficulties in controlling your actions.");
	    display_nhwindow(WIN_MESSAGE, FALSE);
	    You("accidentally tear the spellbook to pieces.");
	    if (!objects[spellbook->otyp].oc_name_known &&
		!objects[spellbook->otyp].oc_uname)
		docall(spellbook);
	    useup(spellbook);
	    gone = TRUE;
	} else {
	    You("find yourself reading the %s line over and over again.",
		spellbook == book ? "next" : "first");
	}
	return gone;
}

/* special effects for The Book of the Dead */
STATIC_OVL void
deadbook(book2)
struct obj *book2;
{
    struct monst *mtmp, *mtmp2;
    coord mm;

    You("turn the pages of the Book of the Dead...");
    makeknown(SPE_BOOK_OF_THE_DEAD);
    /* KMH -- Need ->known to avoid "_a_ Book of the Dead" */
    book2->known = 1;
    if(invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
	register struct obj *otmp;
	register boolean arti1_primed = FALSE, arti2_primed = FALSE,
			 arti_cursed = FALSE;

	if(book2->cursed) {
	    pline_The("runes appear scrambled.  You can't read them!");
	    return;
	}

	if(!(u.uhave.bell || (uwep && uwep->oartifact == ART_SILVER_KEY) || (u.voidChime && (u.sealsActive&SEAL_OTIAX)) || (moves - u.rangBell < 5)) || !u.uhave.menorah) {
	    pline("A chill runs down your %s.", body_part(SPINE));
	    if(!(u.uhave.bell || (uwep && uwep->oartifact == ART_SILVER_KEY) || (u.voidChime && (u.sealsActive&SEAL_OTIAX)) || (moves - u.rangBell < 5))) 
				You_hear("a faint chime...");
	    if(!u.uhave.menorah) pline("Vlad's doppelganger is amused.");
	    return;
	}

	for(otmp = invent; otmp; otmp = otmp->nobj) {
	    if(otmp->otyp == CANDELABRUM_OF_INVOCATION &&
	       otmp->spe == 7 && otmp->lamplit) {
		if(!otmp->cursed) arti1_primed = TRUE;
		else arti_cursed = TRUE;
	    }
	    if(otmp->otyp == BELL_OF_OPENING &&
	       (moves - otmp->age) < 5L
		) { /* you rang it recently */
			if(!otmp->cursed) arti2_primed = TRUE;
			else arti_cursed = TRUE;
	    }
	}
	if(u.voidChime && u.sealsActive&SEAL_OTIAX){
		arti2_primed = TRUE;
	}
	if(moves - u.rangBell < 5L){
		arti2_primed = TRUE;
	}
	if(!arti2_primed && !arti_cursed && uwep && uwep->oartifact == ART_SILVER_KEY){
		pline("As you read from the book, you unconsciously move the silver key through a complex unlocking gesture.");
		if(!uwep->cursed) arti2_primed = TRUE;
		else arti_cursed = TRUE;
	}

	if(arti_cursed) {
	    pline_The("invocation fails!");
	    pline("At least one of your artifacts is cursed...");
	} else if(arti1_primed && arti2_primed) {
	    unsigned soon = (unsigned) d(2,6);	/* time til next intervene() */

	    /* successful invocation */
	    mkinvokearea();
	    u.uevent.invoked = 1;
	    /* in case you haven't killed the Wizard yet, behave as if
	       you just did */
	    u.uevent.udemigod = 1;	/* wizdead() */
	    if (!u.udg_cnt || u.udg_cnt > soon) u.udg_cnt = soon;
	} else {	/* at least one artifact not prepared properly */
	    You("have a feeling that %s is amiss...", something);
	    goto raise_dead;
	}
	return;
    }

    /* when not an invocation situation */
    if (book2->cursed) {
raise_dead:

	You("raised the dead!");
	/* first maybe place a dangerous adversary */
	if (!rn2(3) && ((mtmp = makemon(&mons[PM_MASTER_LICH],
					u.ux, u.uy, NO_MINVENT)) != 0 ||
			(mtmp = makemon(&mons[PM_NALFESHNEE],
					u.ux, u.uy, NO_MINVENT)) != 0)) {
	    mtmp->mpeaceful = 0;
	    set_malign(mtmp);
	}
	/* next handle the affect on things you're carrying */
	(void) unturn_dead(&youmonst);
	/* last place some monsters around you */
	mm.x = u.ux;
	mm.y = u.uy;
	mkundead(&mm, TRUE, NO_MINVENT);
    } else if(book2->blessed) {
	for(mtmp = fmon; mtmp; mtmp = mtmp2) {
	    mtmp2 = mtmp->nmon;		/* tamedog() changes chain */
	    if (DEADMONSTER(mtmp)) continue;

	    if (is_undead_mon(mtmp) && cansee(mtmp->mx, mtmp->my)) {
		mtmp->mpeaceful = TRUE;
		if(sgn(mtmp->data->maligntyp) == sgn(u.ualign.type)
		   && distu(mtmp->mx, mtmp->my) < 4)
		    if (mtmp->mtame) {
			if (mtmp->mtame < 20)
			    mtmp->mtame++;
		    } else
			(void) tamedog(mtmp, (struct obj *)0);
		else monflee(mtmp, 0, FALSE, TRUE);
	    }
	}
    } else {
	switch(rn2(3)) {
	case 0:
	    Your("ancestors are annoyed with you!");
	    break;
	case 1:
	    pline_The("headstones in the cemetery begin to move!");
	    break;
	default:
	    pline("Oh my!  Your name appears in the book!");
	}
    }
    return;
}

STATIC_PTR int
learn()
{
	int i;
	short booktype;
	char splname[BUFSZ];
	boolean costly = TRUE;

	/* JDS: lenses give 50% faster reading; 33% smaller read time */
	if (delay && ublindf && ublindf->otyp == LENSES && rn2(2)) delay++;
	if (Confusion) {		/* became confused while learning */
	    (void) confused_book(book);
	    book = 0;			/* no longer studying */
	    nomul(delay, "reading a book");		/* remaining delay is uninterrupted */
	    delay = 0;
	    return(0);
	}
	if (delay) {	/* not if (delay++), so at end delay == 0 */
	    delay++;
	    return(1); /* still busy */
	}
	exercise(A_WIS, TRUE);		/* you're studying. */
	booktype = book->otyp;
	if(booktype == SPE_BOOK_OF_THE_DEAD) {
	    deadbook(book);
	    return(0);
	}
	if(booktype == SPE_SECRETS){
		if(book->oartifact) doparticularinvoke(book); //this is a redundant check
		else{ 
			if(!(book->ovar1) || 
				book->ovar1 > 10 ||
				book->ovar1 < 1) book->ovar1 = d(1,10);
			switch(book->ovar1){
				case 1:
					pline("...these are some surprisingly petty secrets.");
				break;
				case 2:
					pline("...apparently the language is ALSO a secret.");
				break;
				case 3:
					pline("It's blank. You guess the author KEPT his secrets.");
				    book->otyp = booktype = SPE_BLANK_PAPER;
					book->ovar1 = 0;
				break;
				case 4:
					pline("...these metallurgical techniques are 200 years out of date.");
				break;
				case 5:{
					struct obj *otmp;
					otmp = mksobj(rnd(4) ? ELVEN_DAGGER : DAGGER, TRUE, FALSE);
					otmp->spe = d(1,4)+1;
					otmp->quan += d(2,3);
					if(otmp->otyp == DAGGER) otmp->obj_material = SILVER;
					pline("...it's been hollowed out.  There is a set of throwing daggers inside.");
					useup(book);
					otmp = hold_another_object(otmp, "The %s out.",
				       aobjnam(otmp, "fall"), (const char *)0);
				}
				break;
				case 6:{
					struct obj *otmp;
					otmp = mkobj(WAND_CLASS, FALSE);
					pline("...it's been hollowed out.  There is a wand inside.");
					useup(book);
					otmp = hold_another_object(otmp, "The %s out.",
				       aobjnam(otmp, "fall"), (const char *)0);
				}
				break;
				case 7:
					pline("...these are some surprisingly mundane secrets.");
				break;
				case 8:
					pline("...it's about YOU.");
				break;
				case 9:
					pline("It details the true location of the fabled Dungeons of Doom.");
				break;
				case 10:{
					struct obj *otmp;
					otmp = mkobj(SPBOOK_CLASS, FALSE);
					pline("It's a false cover, a different book is inside.");
					useup(book);
					otmp = hold_another_object(otmp, "The %s out.",
				       aobjnam(otmp, "fall"), (const char *)0);
				}
				break;
			}
		}
		book = 0;
	    return(0);
	}
	if(RoSbook == STUDY_WARD){
	 if((book->oward)){
		pline("The spellbook is warded with a %s.", wardDecode[decode_wardID(book->oward)]);
		if( !(u.wardsknown & book->oward) ){
			u.wardsknown |= book->oward;
		}
		else{
			You("are already familiar with this ward.");
		}
	 } else{
		pline("The spellbook is warded with a thaumaturgical ward, good for spellbooks but not much else.");
	 }
	 return(0);
	}
	Sprintf(splname, objects[booktype].oc_name_known ?
			"\"%s\"" : "the \"%s\" spell",
		OBJ_NAME(objects[booktype]));
	for (i = 0; i < MAXSPELL; i++)  {
		if (spellid(i) == booktype)  {
			if (book->spestudied > MAX_SPELL_STUDY) {
			    pline("This spellbook is too faint to be read any more.");
			    book->otyp = booktype = SPE_BLANK_PAPER;
			} else if (spellknow(i) <= 1000) {
			    Your("knowledge of %s is keener.", splname);
			    incrnknow(i);
			    book->spestudied++;
			    exercise(A_WIS,TRUE);       /* extra study */
			} else { /* 1000 < spellknow(i) <= MAX_SPELL_STUDY */
			    You("know %s quite well already.", splname);
			    costly = FALSE;
			}
			/* make book become known even when spell is already
			   known, in case amnesia made you forget the book */
			makeknown((int)booktype);
			break;
		} else if (spellid(i) == NO_SPELL)  {
			spl_book[i].sp_id = booktype;
			spl_book[i].sp_lev = objects[booktype].oc_level;
			incrnknow(i);
			book->spestudied++;
			You(i > 0 ? "add %s to your repertoire." : "learn %s.",
			    splname);
			makeknown((int)booktype);
			break;
		}
	}

	if ((booktype = further_study(book->otyp)))
	{
		You("are able to apply the writings of the book to learn another spell.");
		Sprintf(splname, objects[booktype].oc_name_known ?
			"\"%s\"" : "the \"%s\" spell",
			OBJ_NAME(objects[booktype]));
		for (i = 0; i < MAXSPELL; i++)  {
			if (spellid(i) == booktype)  {
				if (spellknow(i) <= 1000) {
					Your("knowledge of %s is keener.", splname);
					incrnknow(i);
					exercise(A_WIS, TRUE);       /* extra study */
				}
				else { /* 1000 < spellknow(i) <= MAX_SPELL_STUDY */
					You("know %s quite well already.", splname);
				}
				break;
			}
			else if (spellid(i) == NO_SPELL)  {
				spl_book[i].sp_id = booktype;
				spl_book[i].sp_lev = objects[booktype].oc_level;
				incrnknow(i);
				You("add %s to your repertoire.", splname);
				break;
			}
		}
	}

	if (i == MAXSPELL) impossible("Too many spells memorized!");

	if (book->cursed) {	/* maybe a demon cursed it */
	    if (cursed_book(book)) {
		useup(book);
		book = 0;
		return 0;
	    }
	}
	if (costly) check_unpaid(book);
	book = 0;
	return(0);
}

int
further_study(booktype) /* if the player is skilled enough in the book's spell school, they can learn another spell from the orginial tome */
int booktype;
{
	int skill = P_SKILL(spell_skilltype(booktype));
	int related = 0;
	int skillmin = 0;

#define set_related(spell) if(!related) related = spell
	switch (booktype)
	{
	case SPE_FINGER_OF_DEATH:	set_related(SPE_DRAIN_LIFE);
		skillmin = P_BASIC;
		break;
	case SPE_FIREBALL:			set_related(SPE_FIRE_STORM);
	case SPE_CONE_OF_COLD:		set_related(SPE_BLIZZARD);
	case SPE_FIRE_STORM:		set_related(SPE_FIREBALL);
	case SPE_BLIZZARD:		set_related(SPE_CONE_OF_COLD);
	case SPE_LIGHTNING_STORM:	set_related(SPE_LIGHTNING_BOLT);
	case SPE_EXTRA_HEALING:		set_related(SPE_HEALING);
	case SPE_CHARM_MONSTER:		set_related(SPE_PACIFY_MONSTER);
		skillmin = P_SKILLED;
		break;
	case SPE_LIGHTNING_BOLT:	set_related(SPE_LIGHTNING_STORM);
	case SPE_HEALING:			set_related(SPE_EXTRA_HEALING);
	case SPE_DRAIN_LIFE:		set_related(SPE_FINGER_OF_DEATH);
	case SPE_CREATE_MONSTER:	set_related(SPE_CREATE_FAMILIAR);
	case SPE_CREATE_FAMILIAR:	set_related(SPE_CREATE_MONSTER);
	case SPE_PACIFY_MONSTER:	set_related(SPE_CHARM_MONSTER);
		skillmin = P_EXPERT;
		break;
	}
	return (related && skill >= skillmin) ? related : 0;
#undef set_related
}

int
study_book(spellbook)
struct obj *spellbook;
{
	register int	 booktype = spellbook->otyp;
	register boolean confused = (Confusion != 0);
	char splname[BUFSZ];
	boolean too_hard = FALSE;

	if(spellbook->oartifact){ //this is the primary artifact-book check.
		if(spellbook->oartifact != ART_BOOK_OF_INFINITE_SPELLS){
			doparticularinvoke(spellbook);//there is a redundant check in the spell learning code
			return 1;					//which should never be reached, and only catches books of secrets anyway.
		} else {
			int i;
			boolean read_book = FALSE;
			Sprintf(splname, objects[spellbook->ovar1].oc_name_known ?
					"\"%s\"" : "the \"%s\" spell",
				OBJ_NAME(objects[spellbook->ovar1]));
			for (i = 0; i < MAXSPELL; i++)  {
				if (spellid(i) == spellbook->ovar1)  {
					pline("The endless pages of the book cover the material of a spellbook of %s in exhaustive detail.",OBJ_NAME(objects[spellbook->ovar1]));
					if (spellknow(i) <= 1000) {
						Your("knowledge of %s is keener.", splname);
						incrnknow(i);
						read_book = TRUE;
						exercise(A_WIS,TRUE);       /* extra study */
					} else { /* 1000 < spellknow(i) <= MAX_SPELL_STUDY */
						You("know %s quite well already.", splname);
					}
					break;
				} else if (spellid(i) == NO_SPELL)  {
					spl_book[i].sp_id = spellbook->ovar1;
					spl_book[i].sp_lev = objects[spellbook->ovar1].oc_level;
					incrnknow(i);
					read_book = TRUE;
					pline("The endless pages of the book cover the material of a spellbook of %s in exhaustive detail.",OBJ_NAME(objects[spellbook->ovar1]));
					pline("Using the instructions on the pages, you easily learn to cast the spell!");
					break;
				}
			}
			int booktype;
			if ((booktype = further_study(spellbook->ovar1))){
				You("understand the material thoroughly, and can see a way to cast another spell.");
				Sprintf(splname, objects[booktype].oc_name_known ?
					"\"%s\"" : "the \"%s\" spell",
					OBJ_NAME(objects[booktype]));
				for (i = 0; i < MAXSPELL; i++)  {
					if (spellid(i) == booktype)  {
						if (spellknow(i) <= 1000) {
							Your("knowledge of %s is keener.", splname);
							incrnknow(i);
							exercise(A_WIS, TRUE);       /* extra study */
							read_book = TRUE;
						}
						else { /* 1000 < spellknow(i) <= MAX_SPELL_STUDY */
							You("know %s quite well already.", splname);
						}
						break;
					}
					else if (spellid(i) == NO_SPELL)  {
						spl_book[i].sp_id = booktype;
						spl_book[i].sp_lev = objects[booktype].oc_level;
						incrnknow(i);
						You("add %s to your repertoire.", splname);
						read_book = TRUE;
						break;
					}
				}
			}
			if (read_book && !rn2(20)){
				spellbook->ovar1 = rn2(SPE_BLANK_PAPER - SPE_DIG) + SPE_DIG;
				pline("The endless pages of the book turn themselves. They settle on a section describing %s.", OBJ_NAME(objects[spellbook->ovar1]));
			}
			if (i == MAXSPELL) impossible("Too many spells memorized!");
			return 1;
		}
	}

	if (delay && !confused && spellbook == book &&
		    /* handle the sequence: start reading, get interrupted,
		       have book become erased somehow, resume reading it */
		    booktype != SPE_BLANK_PAPER) {
		You("continue your efforts to memorize the spell.");
	} else {
		/* KMH -- Simplified this code */
		if (booktype == SPE_BLANK_PAPER) {
			pline("This spellbook is all blank.");
			makeknown(booktype);
			return(1);
		}
		switch (objects[booktype].oc_level) {
		 case 1:
		 case 2:
			delay = -objects[booktype].oc_delay;
			break;
		 case 3:
		 case 4:
			delay = -(objects[booktype].oc_level - 1) *
				objects[booktype].oc_delay;
			break;
		 case 5:
		 case 6:
			delay = -objects[booktype].oc_level *
				objects[booktype].oc_delay;
			break;
		 case 7:
			delay = -8 * objects[booktype].oc_delay;
			break;
		 default:
			impossible("Unknown spellbook level %d, book %d;",
				objects[booktype].oc_level, booktype);
			return 0;
		}
		RoSbook = doreadstudy("You open the spellbook.");
		if(!RoSbook){
			delay = 0;
			return 0;
		}
		if((spellbook->oward) && RoSbook == STUDY_WARD){
			if( (u.wardsknown & spellbook->oward) ){
				pline("The spellbook is warded with a %s.", wardDecode[decode_wardID(spellbook->oward)]);
				You("are already familiar with this ward.");
				delay = 0;
				return 0;
			}
		}		/* Books are often wiser than their readers (Rus.) */
		spellbook->in_use = TRUE;
		if (!spellbook->blessed &&
			!spellbook->oartifact && 
		    spellbook->otyp != SPE_BOOK_OF_THE_DEAD) {
		    if (spellbook->cursed) {
				too_hard = TRUE;
		    } else {
			/* uncursed - chance to fail */
			int read_ability = ACURR(A_INT) + 4 + u.ulevel/2
			    + ((RoSbook == STUDY_WARD) ? 10 : 0)
			    - 2*objects[booktype].oc_level
			    + ((ublindf && ublindf->otyp == LENSES) ? 2 : 0);
			/* only wizards know if a spell is too difficult */
			if ((Role_if(PM_WIZARD) || u.sealsActive&SEAL_PAIMON) && read_ability < 20 &&
			    !confused) {
			    char qbuf[QBUFSZ];
			    Sprintf(qbuf,
		      "This spellbook is %sdifficult to comprehend. Continue?",
				    (read_ability < 12 ? "very " : ""));
			    if (yn(qbuf) != 'y') {
				spellbook->in_use = FALSE;
				return(1);
			    }
			}
			/* its up to random luck now */
			if (rnd(20) > read_ability) {
			    too_hard = TRUE;
			}
		    }
		}

		if (too_hard) {
		    boolean gone = cursed_book(spellbook);

		    nomul(delay, "reading a book");			/* study time */
		    delay = 0;
		    if(gone || !rn2(3)) {
			if (!gone) pline_The("spellbook crumbles to dust!");
			if (!objects[spellbook->otyp].oc_name_known &&
				!objects[spellbook->otyp].oc_uname)
			    docall(spellbook);
			useup(spellbook);
		    } else
			spellbook->in_use = FALSE;
		    return(1);
		} else if (confused) {
		    if (!confused_book(spellbook)) {
			spellbook->in_use = FALSE;
		    }
		    nomul(delay, "reading a book");
		    delay = 0;
		    return(1);
		}
		spellbook->in_use = FALSE;

		RoSbook == READ_SPELL ? 
			You("begin to %s the runes.",
				spellbook->otyp == SPE_BOOK_OF_THE_DEAD ? "recite" :
				"memorize") 
			: You("begin to study the ward.");
	}

	book = spellbook;
	set_occupation(learn, "studying", 0);
	return(1);
}

boolean
spell_maintained(spell)
int spell;
{
    spell = (spell - SPE_DIG);
    return !!(u.spells_maintained & ((unsigned long long int)1 << spell));
}

void
spell_maintain(spell)
int spell;
{
    spell = (spell - SPE_DIG);
    u.spells_maintained |= ((unsigned long long int)1 << spell);
}

void
spell_unmaintain(spell)
int spell;
{
    spell = (spell - SPE_DIG);
    u.spells_maintained &= ~((unsigned long long int)1 << spell);
}

void
run_maintained_spells()
{
    int spell;
	int spell_index;
	/* Check forgotten spells */
	for (spell = SPE_DIG; spell != SPE_BLANK_PAPER; spell++) {
		if (!spell_maintained(spell))
			continue;
	
		for (spell_index = 0; spell_index < MAXSPELL; spell_index++)
			if (spellid(spell_index) == spell)
				break;

		boolean knows_spell = FALSE; /* might not be true if amnesia removed it */
		if (spell_index < MAXSPELL)
			knows_spell = TRUE;

		/* We can't use spellname() but need to use OBJ_NAME directly, because
		   amnesia can delete any trace of spell index... */
		if (!knows_spell || spellknow(spell_index) <= 0 || Confusion) {
			pline("You can no longer maintain %s.",
				  OBJ_NAME(objects[spell]));
			spell_unmaintain(spell);
			continue;
		}

		/* Decrease power depending on spell level and proficiency.
		   If an attempted cast fails 5 times in a row, unmaintain the spell. */
		int chance = percent_success(spell_index);
		int tries = 5;
		while (tries) {
			if (rnd(100) > chance) {
				tries--;
				continue;
			}
			break;
		}
		if (!tries) {
			pline("You lose concentration and fail to maintain %s!",
				  spellname(spell_index));
			spell_unmaintain(spell);
			continue;
		}
		/* catch reasons we can't cast the spell (pw, food) */
		int spell_level = objects[spell].oc_level;
		if (u.uhave.amulet)
			spell_level *= 2;
		int hungr = spellhunger(spell_level * 5) * MAINTAINED_SPELL_HUNGER_MULTIPLIER;
		if (u.uen < spell_level){
			You("lack the energy to maintain %s.",
				spellname(spell_index));
			spell_unmaintain(spell);
			continue;
		}
		if (hungr > YouHunger - 3){
			You("are too hungry to maintain %s.",
				spellname(spell_index));
			spell_unmaintain(spell);
			continue;
		}

		/* cast the spell if it needs re-casting */
		if (run_maintained_spell(spell)){			// reminder: this line re-casts the spell
			/* first, power cost*/
			spell_level += (spell_level*(5 - tries)) / 2;	//costs more if it has a higher failure rate

			u.maintained_en_debt += spell_level * 5 * MAINTAINED_SPELL_PW_MULTIPLIER;

			/* next, hunger cost */
			if (spell != SPE_DETECT_FOOD && !(Race_if(PM_INCANTIFIER)))	// detect_food exception is moot, but consistent
				morehungry(hungr);
		}
	}
}

STATIC_DCL boolean
can_maintain_spell(spell)
int spell;
{
	switch (spell)
	{
	case SPE_DETECT_MONSTERS:
		if (P_SKILL(spell_skilltype(SPE_DETECT_MONSTERS)) + (Spellboost ? 1 : 0) < P_SKILLED)
			break;
	case SPE_HASTE_SELF:
	case SPE_LEVITATION:
	case SPE_INVISIBILITY:
	case SPE_DETECT_UNSEEN:
	case SPE_PROTECTION:
	case SPE_ANTIMAGIC_SHIELD:
		return TRUE;
	}
	return FALSE;
}

STATIC_OVL boolean
run_maintained_spell(spell)
int spell;
{
    // /* Find the skill for the given spell type category */
    // int skill = P_SKILL(spell_skilltype(spell));
	boolean cast = FALSE;
    int leviprop = LEVITATION;
    // if (skill == P_EXPERT)
        // leviprop = FLYING;

	switch (spell) {
	case SPE_HASTE_SELF:
		if ((HFast&TIMEOUT) < 10)
		{
			incr_itimeout(&HFast, rn1(10, 100));
			cast = TRUE;
		}
		break;
	case SPE_DETECT_MONSTERS:
		if ((HDetect_monsters&TIMEOUT) < 10)
		{
			incr_itimeout(&HDetect_monsters, rn1(40, 21));
			cast = TRUE;
		}
		break;
	case SPE_LEVITATION:
		if ((HLevitation&TIMEOUT) < 10)
		{
			incr_itimeout(&HLevitation, rn1(140, 10));
			cast = TRUE;
		}
		break;
	case SPE_INVISIBILITY:
		if ((HInvis&TIMEOUT) < 10)
		{
			incr_itimeout(&HInvis, rn1(15, 31));
			cast = TRUE;
		}
		break;
	case SPE_DETECT_UNSEEN:
		if ((HSee_invisible&TIMEOUT) < 10)
		{
			incr_itimeout(&HSee_invisible, rn1(10, 75));
			cast = TRUE;
		}
        break;
    case SPE_PROTECTION:
		u.usptime++;
		if (!(moves % 15))
		{
			cast = TRUE;
		}
        break;
	case SPE_ANTIMAGIC_SHIELD:
		if ((HNullmagic&TIMEOUT) < 10) {
			incr_itimeout(&HNullmagic, 100);
			cast = TRUE;
		}
		break;
    default:
        impossible("player maintaining an unmaintainable spell? (%d)", spell);
        spell_unmaintain(spell);
        break;
    }
	return cast;
}

/* a spellbook has been destroyed or the character has changed levels;
   the stored address for the current book is no longer valid */
void
book_disappears(obj)
struct obj *obj;
{
	if (obj == book) book = (struct obj *)0;
}

/* renaming an object usually results in it having a different address;
   so the sequence start reading, get interrupted, name the book, resume
   reading would read the "new" book from scratch */
void
book_substitution(old_obj, new_obj)
struct obj *old_obj, *new_obj;
{
	if (old_obj == book) book = new_obj;
}

/* called from moveloop() */
void
age_spells()
{
	int i;
	/*
	 * The time relative to the hero (a pass through move
	 * loop) causes all spell knowledge to be decremented.
	 * The hero's speed, rest status, conscious status etc.
	 * does not alter the loss of memory.
	 */
	for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++)
	    if (spellknow(i))
		decrnknow(i);
	return;
}

/*
 * Return TRUE if a spell was picked, with the spell index in the return
 * parameter.  Otherwise return FALSE.
 */
STATIC_OVL boolean
getspell(spell_no, menutype)
int *spell_no;
int menutype;
{
	int nspells, idx;
	char ilet, lets[BUFSZ], qbuf[QBUFSZ];

	if (spellid(0) == NO_SPELL && !((uarmh && uarmh->oartifact == ART_STORMHELM)  
		|| (uwep && uwep->oartifact == ART_DEATH_SPEAR_OF_VHAERUN) 
		|| (uwep && uwep->oartifact == ART_ANNULUS && uwep->otyp == CHAKRAM)
	)){
	    You("don't know any spells right now.");
	    return FALSE;
	}
	if (flags.menu_style == MENU_TRADITIONAL) {
	    /* we know there is at least 1 known spell */
		for (nspells = 1; nspells < MAXSPELL
				&& spellid(nspells) != NO_SPELL; nspells++)
		continue;

	    if (nspells == 1)  Strcpy(lets, "a");
	    else if (nspells < 27)  Sprintf(lets, "a-%c", 'a' + nspells - 1);
	    else if (nspells == 27)  Sprintf(lets, "a-zA");
	    else Sprintf(lets, "a-zA-%c", 'A' + nspells - 27);
		
		for(;;)  {
		Sprintf(qbuf, "Cast which spell? [%s ?]", lets);
		if ((ilet = yn_function(qbuf, (char *)0, '\0')) == '?')
			break;
		
		if (index(quitchars, ilet))
			return FALSE;
		
		idx = spell_let_to_idx(ilet);
		if (idx >= 0 && idx < nspells) {
			*spell_no = idx;
			return TRUE;
		} else
			You("don't know that spell.");
		}
	}
	return dospellmenu(menutype, spell_no);
}
/*
 * Return TRUE if a spell was picked, with the spell index in the return
 * parameter.  Otherwise return FALSE.
 */
 
STATIC_OVL boolean
getspirit(power_no)
	int *power_no;
{
	int nspells, idx;
	char ilet, lets[BUFSZ], qbuf[QBUFSZ];

	if (flags.menu_style == MENU_TRADITIONAL) {
	    /* we know there is at least 1 known spell */
		
		//put characters into lets here
		spiritLets(lets, TRUE);
		
	    for(;;)  {
		Sprintf(qbuf, "Use which power? [%s ?]", lets);
		if ((ilet = yn_function(qbuf, (char *)0, '\0')) == '?')
		    break;

		if (index(quitchars, ilet))
		    return FALSE;
		
		if(check_spirit_let(ilet)){
			*power_no = u.spiritPOrder[(int) ilet];
			return TRUE;
		} else
		    You("don't know that power.");
		}
	}
	return dospiritmenu("Choose which power to use", power_no, TRUE);
}

static const long spiritPOwner[NUMBER_POWERS] = {
	SEAL_AHAZU,
	SEAL_AMON,
	SEAL_ANDREALPHUS, SEAL_ANDREALPHUS,
	SEAL_ANDROMALIUS, SEAL_ANDROMALIUS,
	SEAL_ASTAROTH, SEAL_ASTAROTH,
	SEAL_BALAM, SEAL_BALAM,
	SEAL_BERITH, SEAL_BERITH,
	SEAL_BUER, SEAL_BUER,
	SEAL_CHUPOCLOPS,
	SEAL_DANTALION, SEAL_DANTALION,
	SEAL_SHIRO,
	SEAL_ECHIDNA, SEAL_ECHIDNA,
	SEAL_EDEN, SEAL_EDEN, SEAL_EDEN,
	SEAL_ENKI, SEAL_ENKI,
	SEAL_EURYNOME, SEAL_EURYNOME,
	SEAL_EVE, SEAL_EVE,
	SEAL_FAFNIR, SEAL_FAFNIR,
	SEAL_HUGINN_MUNINN,
	SEAL_IRIS, SEAL_IRIS,
	SEAL_JACK, SEAL_JACK,
	SEAL_MALPHAS,
	SEAL_MARIONETTE, SEAL_MARIONETTE,
	SEAL_MOTHER,
	SEAL_NABERIUS, SEAL_NABERIUS,
	SEAL_ORTHOS,
	SEAL_OSE, SEAL_OSE,
	SEAL_OTIAX,
	SEAL_PAIMON, SEAL_PAIMON,
	SEAL_SIMURGH, SEAL_SIMURGH, SEAL_SIMURGH,
	SEAL_TENEBROUS, SEAL_TENEBROUS, SEAL_TENEBROUS,
	SEAL_YMIR, SEAL_YMIR,
	SEAL_SPECIAL|SEAL_DAHLVER_NAR,
	SEAL_SPECIAL|SEAL_ACERERAK,
	SEAL_SPECIAL|SEAL_COUNCIL,
	SEAL_SPECIAL|SEAL_COSMOS,
	SEAL_SPECIAL|SEAL_LIVING_CRYSTAL,
	SEAL_SPECIAL|SEAL_TWO_TREES,SEAL_SPECIAL|SEAL_TWO_TREES,
	SEAL_SPECIAL|SEAL_NUDZIRATH, SEAL_SPECIAL|SEAL_NUDZIRATH,
	SEAL_SPECIAL|SEAL_ALIGNMENT_THING,
	SEAL_SPECIAL|SEAL_UNKNOWN_GOD,
	SEAL_SPECIAL|SEAL_BLACK_WEB, SEAL_SPECIAL|SEAL_BLACK_WEB,
	SEAL_SPECIAL|SEAL_NUMINA, SEAL_SPECIAL|SEAL_NUMINA, SEAL_SPECIAL|SEAL_NUMINA, SEAL_SPECIAL|SEAL_NUMINA
};

static const char *spiritPName[NUMBER_POWERS] = {
	"Abduction",
	"Fire Breath",
	"Transdimensional Ray", "Teleport",
	"Jester's Mirth", "Thief's Instincts",
	"Astaroth's Assembly", "Astaroth's Shards",
	"Icy Glare", "Balam's Anointing",
	"Blood Mercenary", "Sow Discord",
	"Gift of Healing", "Gift of Health",
	"Throw webbing",
	"Thought Travel", "Dread of Dantalion",
	"Earth Swallow",
	"Echidna's Venom", "Sing Lullaby",
	"Purifying Blast", "Recall to Eden", "Stargate",
	"Walk among Thresholds", "Geyser",
	"Vengeance", "Shape the Wind",
	"Thorns and Stones", "Barrage",
	"Breathe Poison", "Ruinous Strike",
	"Raven's Talons",
	"Horrid Wilting", "Horrid Rainbow",
	"Refill Lantern", "Hellfire", 
	"Call Murder",
	"Root Shout", "Yank Wires",
	"Disgusted Gaze",
	"Bloody Tongue", "Silver Tongue",
	"Exhalation of the Rift", 
	"Querient Thoughts", "Great Leap",
	"Open",
	"Read Spell", "Book Telepathy",
	"Unite the Earth and Sky", "Hook in the Sky", "Enlightenment",
	"Damning Darkness", "Touch of the Void", "Echoes of the Last Word",
	"Poison Gaze", "Gap Step",
	"Moan",
	"Swallow Soul",
	"Embassy of Elements",
	"Crystal Memories",
	"Pseudonatural Surge",
	"Silver dew of Telperion","Golden dew of Laurelin",
	"Mirror Shatter", "Mirror Walk",
	"Flowing Forms",
	"Phase step",
	"Black Bolt", "Weave a Black Web",
	"Identify", "Clairvoyance", "Find Path", "Gnosis Premonition"
};

int
pick_council_seal()
{
	winid tmpwin;
	int i, n, how;
	char buf[BUFSZ];
	char incntlet = 'a';
	long seal_flag = 0x1L;
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */
	
	Sprintf(buf,	"Pick spirit to re-contact:");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	
	for(i = 0; i < (QUEST_SPIRITS-FIRST_SEAL); i++){
		seal_flag = 0x1L << i;
		if(u.sealsKnown&seal_flag && !(u.sealsActive&seal_flag) && (u.sealsUsed&seal_flag) && u.sealTimeout[i] < moves ){
			Sprintf(buf,	"%s%s", sealNames[i], sealTitles[i]);
			any.a_int = i+FIRST_SEAL;	/* must be non-zero */
			add_menu(tmpwin, NO_GLYPH, &any,
				incntlet, 0, ATR_NONE, buf,
				MENU_UNSELECTED);
			incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
		}
	}
	end_menu(tmpwin,	"Choose seal:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return ( n > 0 ) ? selected[0].item.a_int : 0;
}

int
pick_gnosis_seal()
{
	winid tmpwin;
	int i, n, how;
	char buf[BUFSZ];
	char incntlet = 'a';
	long seal_flag = 0x1L;
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */
	
	Sprintf(buf,	"Pick spirit:");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	
	for(i = 0; i < (QUEST_SPIRITS-FIRST_SEAL); i++){
		seal_flag = 0x1L << i;
		if(u.sealsKnown&seal_flag && !(u.sealsActive&seal_flag) && u.sealTimeout[i] < moves){
			Sprintf(buf,	"%s%s", sealNames[i], sealTitles[i]);
			any.a_int = i+FIRST_SEAL;	/* must be non-zero */
			add_menu(tmpwin, NO_GLYPH, &any,
				incntlet, 0, ATR_NONE, buf,
				MENU_UNSELECTED);
			incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
		}
	}
	end_menu(tmpwin,	"Choose seal:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return ( n > 0 ) ? selected[0].item.a_int : 0;
}

STATIC_OVL boolean
spiritLets(lets, respect_timeout)
	char *lets;
	int respect_timeout;
{
	int i,s;
	if(flags.timeoutOrder){
		for(s=0; s<NUM_BIND_SPRITS; s++){
			if(u.spirit[s]) for(i = 0; i<52; i++){
				if(spiritPOwner[u.spiritPOrder[i]] == u.spirit[s] && (u.spiritPColdowns[u.spiritPOrder[i]] < monstermoves || !respect_timeout)){
					Sprintf(lets, "%c", i<26 ? 'a'+(char)i : 'A'+(char)(i-26));
				}
			}
		}
	} else {
		for(i = 0; i<52; i++){
			if(((spiritPOwner[u.spiritPOrder[i]] & u.sealsActive &&
				!(spiritPOwner[u.spiritPOrder[i]] & SEAL_SPECIAL)) || 
				spiritPOwner[u.spiritPOrder[i]] & u.specialSealsActive & ~SEAL_SPECIAL) &&
				(u.spiritPColdowns[u.spiritPOrder[i]] < monstermoves || !respect_timeout)
			){
				Sprintf(lets, "%c", i<26 ? 'a'+(char)i : 'A'+(char)(i-26));
			}
		}
	}
	return TRUE;
}

/* the 'Z' command -- cast a spell */
int
docast()
{
	int spell_no;
	if (getspell(&spell_no, SPELLMENU_CAST))
					return spelleffects(spell_no, FALSE, 0);
	return 0;
}

/* allow the player to conditionally cast spells via equipped artifacts */
void
update_alternate_spells()
{
	int i;

	// for artifacts
	if (uarmh && uarmh->oartifact == ART_STORMHELM){
		for (i = 0; i < MAXSPELL; i++) {
			if (spellid(i) == SPE_LIGHTNING_STORM) {
				if (spl_book[i].sp_know < 1) spl_book[i].sp_know = 1;
				break;
			}
			if (spellid(i) == NO_SPELL)  {
				spl_book[i].sp_id = SPE_LIGHTNING_STORM;
				spl_book[i].sp_lev = objects[SPE_LIGHTNING_STORM].oc_level;
				spl_book[i].sp_know = 1;
				break;
			}
		}
	}
	if (uwep && uwep->oartifact == ART_DEATH_SPEAR_OF_VHAERUN){
		for (i = 0; i < MAXSPELL; i++) {
			if (spellid(i) == SPE_DRAIN_LIFE) {
				if (spl_book[i].sp_know < 1) spl_book[i].sp_know = 1;
				break;
			}
			if (spellid(i) == NO_SPELL)  {
				spl_book[i].sp_id = SPE_DRAIN_LIFE;
				spl_book[i].sp_lev = objects[SPE_DRAIN_LIFE].oc_level;
				spl_book[i].sp_know = 1;
				break;
			}
		}
	}
	if (uwep && uwep->oartifact == ART_ANNULUS && uwep->otyp == CHAKRAM){
		for (i = 0; i < MAXSPELL; i++) {
			if (spellid(i) == SPE_MAGIC_MISSILE) {
				if (spl_book[i].sp_know < 1) spl_book[i].sp_know = 1;
				break;
			}
			if (spellid(i) == NO_SPELL) {
				spl_book[i].sp_id = SPE_MAGIC_MISSILE;
				spl_book[i].sp_lev = objects[SPE_MAGIC_MISSILE].oc_level;
				spl_book[i].sp_know = 1;
				break;
			}
		}
		for (i = 0; i < MAXSPELL; i++) {
			if (spellid(i) == SPE_FORCE_BOLT) {
				if (spl_book[i].sp_know < 1) spl_book[i].sp_know = 1;
				break;
			}
			if (spellid(i) == NO_SPELL) {
				spl_book[i].sp_id = SPE_FORCE_BOLT;
				spl_book[i].sp_lev = objects[SPE_FORCE_BOLT].oc_level;
				spl_book[i].sp_know = 1;
				break;
			}
		}
	}
}

/* the '^f' command -- fire a spirit power */
int
dospirit()
{
	int power_no;
	
	if(!u.sealsActive && !u.specialSealsActive){
		if(Role_if(PM_EXILE) && u.ulevel > 1 && u.spiritPColdowns[PWR_GNOSIS_PREMONITION] < monstermoves){
			if(yn("Use Gnosis Premonition?") == 'y'){
				if(spiriteffects(PWR_GNOSIS_PREMONITION, FALSE))
					u.spiritPColdowns[PWR_GNOSIS_PREMONITION] = moves + 125;
			}
			return 1;
		} else {
			You("don't have any spirits bound.");
			return 0;
		}
	}
	
	if (getspirit(&power_no))
					return spiriteffects(power_no, FALSE);
	return 0;
}

STATIC_OVL boolean
check_spirit_let(let)
	char let;
{
	int i;
	if(let >= 'a' && let <= 'z') i = (int)(let - 'a');
	else if(let >= 'A' && let <= 'Z') i = (int)(let - 'A' + 26);
	else return FALSE; //not a valid letter
	
	return u.spiritPColdowns[u.spiritPOrder[i]] < monstermoves && spiritPOwner[u.spiritPOrder[i]] &&
			((spiritPOwner[u.spiritPOrder[i]] & u.sealsActive &&
			!(spiritPOwner[u.spiritPOrder[i]] & SEAL_SPECIAL)) || 
			spiritPOwner[u.spiritPOrder[i]] & u.specialSealsActive & ~SEAL_SPECIAL);
}
/*
 * returns the skill associated with a spell damage type
 * assumes all spells of a damage type are in the same school
 */
int
spell_skill_from_adtype(atype)
int atype;
{
	int spell = 0;
	switch (atype)
	{
	case AD_MAGM:
	case AD_FIRE:
	case AD_COLD:
	case AD_ELEC:
	case AD_DEAD:
	case AD_DRLI:
		return P_ATTACK_SPELL;
	case AD_DRST:
	case AD_ACID:
		return P_MATTER_SPELL;
	case AD_SLEE:
		return P_ENCHANTMENT_SPELL;
	default:
		impossible("unaccounted for atype in spell_skill_from_adtype: %d", atype);
		return P_ATTACK_SPELL;
	}
}

int
spell_adtype(spell)
int spell;
{
	switch (spell)
	{
	case SPE_MAGIC_MISSILE:
		return AD_MAGM;
	case SPE_FIREBALL:
	case SPE_FIRE_STORM:
		return AD_FIRE;
	case SPE_CONE_OF_COLD:
	case SPE_BLIZZARD:
		return AD_COLD;
	case SPE_LIGHTNING_BOLT:
	case SPE_LIGHTNING_STORM:
		return AD_ELEC;
	case SPE_ACID_SPLASH:
		return AD_ACID;
	case SPE_POISON_SPRAY:
		return AD_DRST;
	case SPE_FINGER_OF_DEATH:
		return AD_DEAD;
	case SPE_SLEEP:
		return AD_SLEE;
	default:
		impossible("unaccounted-for spell passed to spell_adtype: %d", spell);
		return -1;
	}
}

const char *
spelltypemnemonic(skill)
int skill;
{
	switch (skill) {
	    case P_ATTACK_SPELL:
		return "attack";
	    case P_HEALING_SPELL:
		return "healing";
	    case P_DIVINATION_SPELL:
		return "divination";
	    case P_ENCHANTMENT_SPELL:
		return "enchantment";
	    case P_CLERIC_SPELL:
		return "clerical";
	    case P_ESCAPE_SPELL:
		return "escape";
	    case P_MATTER_SPELL:
		return "matter";
	    default:
		impossible("Unknown spell skill, %d;", skill);
		return "";
	}
}

STATIC_OVL int
spellhunger(energy)
int energy;
{
	int hungr = energy * 2;
	int intell;

	/* If hero is a wizard, their current intelligence
	 * (bonuses + temporary + current)
	 * affects hunger reduction in casting a spell.
	 * 1. int = 17-18 no reduction
	 * 2. int = 16    1/4 hungr
	 * 3. int = 15    1/2 hungr
	 * 4. int = 1-14  normal reduction
	 * The reason for this is:
	 * a) Intelligence affects the amount of exertion
	 * in thinking.
	 * b) Wizards have spent their life at magic and
	 * understand quite well how to cast spells.
	 */
	intell = acurr(A_INT);
	if (!Role_if(PM_WIZARD)){
		if(u.sealsActive&SEAL_PAIMON) intell -= 6;
		else intell -= 10;
	}
	if(intell < 15);
	else if(intell < 16) hungr /= 2;
	else if(intell < 20) hungr /= 4;
	else hungr = 0;
	// switch (intell) {
		// case 25: case 24: case 23: case 22:
		// case 21: case 20: case 19: case 18:
		// case 17: hungr = 0; break;
		// case 16: hungr /= 4; break;
		// case 15: hungr /= 2; break;
	// }
	return hungr;
}
int
spell_skilltype(booktype)
int booktype;
{
	return (objects[booktype].oc_skill);
}

STATIC_OVL void
cast_protection()
{
	int loglev = 0;
	int l = u.ulevel;
	int natac = u.uac + u.uspellprot;
	int gain;

	/* loglev=log2(u.ulevel)+1 (1..5) */
	while (l) {
	    loglev++;
	    l /= 2;
	}

	/* The more u.uspellprot you already have, the less you get,
	 * and the better your natural ac, the less you get.
	 *
	 *	LEVEL AC    SPELLPROT from sucessive SPE_PROTECTION casts
	 *      1     10    0,  1,  2,  3,  4
	 *      1      0    0,  1,  2,  3
	 *      1    -10    0,  1,  2
	 *      2-3   10    0,  2,  4,  5,  6,  7,  8
	 *      2-3    0    0,  2,  4,  5,  6
	 *      2-3  -10    0,  2,  3,  4
	 *      4-7   10    0,  3,  6,  8,  9, 10, 11, 12
	 *      4-7    0    0,  3,  5,  7,  8,  9
	 *      4-7  -10    0,  3,  5,  6
	 *      7-15 -10    0,  3,  5,  6
	 *      8-15  10    0,  4,  7, 10, 12, 13, 14, 15, 16
	 *      8-15   0    0,  4,  7,  9, 10, 11, 12
	 *      8-15 -10    0,  4,  6,  7,  8
	 *     16-30  10    0,  5,  9, 12, 14, 16, 17, 18, 19, 20
	 *     16-30   0    0,  5,  9, 11, 13, 14, 15
	 *     16-30 -10    0,  5,  8,  9, 10
	 */
	// gain = loglev - (int)u.uspellprot / (4 - min(3,(10 - natac)/10));
	
	gain = 2*loglev - u.uspellprot; //refill spellprot, don't stack it.
	
	if (gain > 0) {
	    if (!Blind) {
		const char *hgolden = hcolor(NH_GOLDEN);

		if (u.uspellprot)
		    pline_The("%s haze around you becomes more dense.",
			      hgolden);
		else
		    pline_The("%s around you begins to shimmer with %s haze.",
			      (Underwater || Is_waterlevel(&u.uz)) ? "water" :
			       u.uswallow ? mbodypart(u.ustuck, STOMACH) :
			      IS_STWALL(levl[u.ux][u.uy].typ) ? "stone" : "air",
			      an(hgolden));
	    }
	    u.uspellprot += gain;
	    u.uspmtime =
			P_SKILL(spell_skilltype(SPE_PROTECTION)) == P_EXPERT ? 30:
			P_SKILL(spell_skilltype(SPE_PROTECTION)) == P_SKILLED ? 20: 
			P_SKILL(spell_skilltype(SPE_PROTECTION)) == P_BASIC ? 15: 10;
		u.usptime = u.uspmtime;
	    find_ac();
	} else {
	    Your("skin feels warm for a moment.");
	}
}

/* attempting to cast a forgotten spell will cause disorientation */
STATIC_OVL void
spell_backfire(spell)
int spell;
{
    long duration = (long)((spellev(spell) + 1) * 3);	 /* 6..24 */

    /* prior to 3.4.1, the only effect was confusion; it still predominates */
    switch (rn2(10)) {
    case 0:
    case 1:
    case 2:
    case 3: make_confused(duration, FALSE);			/* 40% */
	    break;
    case 4:
    case 5:
    case 6: make_confused(2L * duration / 3L, FALSE);		/* 30% */
	    make_stunned(duration / 3L, FALSE);
	    break;
    case 7:
    case 8: make_stunned(2L * duration / 3L, FALSE);		/* 20% */
	    make_confused(duration / 3L, FALSE);
	    break;
    case 9: make_stunned(duration, FALSE);			/* 10% */
	    break;
    }
    return;
}

STATIC_OVL boolean
sightwedge
(dx,dy,x1,y1,x2,y2)
int dx,dy,x1,y1,x2,y2;
{
	boolean gx=FALSE, gy=FALSE;
	int deltax = x2-x1, deltay = y2-y1;
	if(dy == 0){
		if(dx > 0) return deltay<deltax && -deltay<deltax;
		if(dx < 0) return deltay>deltax && -deltay>deltax;
		if(dx == 0)return FALSE;
	}
	if(dx == 0){
		if(dy > 0) return deltay>deltax && -deltay<deltax;
		if(dy < 0) return deltay<deltax && -deltay>deltax;
		if(dy == 0)return FALSE;
	}
	if(dy > 0 && dx > 0) return deltax>0 && deltay>0;
	if(dy > 0 && dx < 0) return deltax<0 && deltay>0;
	if(dy < 0 && dx < 0) return deltax<0 && deltay<0;
	if(dy < 0 && dx > 0) return deltax>0 && deltay<0;
	return FALSE;
	// // if(dy){
		// // if(dx < 0){
			// // if(x1 < x2) gx = TRUE;
		// // } else if(dx > 0){
			// // if(x1 > x2) gx = TRUE;
		// // } else {
			// // if(dy > 0 && y2 > y1){
				// // int deltax = x2 - x1, deltay = y2 - y1;
				// // if(deltax < deltay && deltax > -1*deltay){
					// // gx = TRUE;
					// // gy = TRUE;
				// // }
			// // }
			// // else if(dy < 0 && y2 < y1){
				// // int deltax = x2 - x1, deltay = y2 - y1;
				// // if(deltax > deltay && deltax < -1*deltay){
					// // gx = TRUE;
					// // gy = TRUE;
				// // }
			// // }
		// // }
	// // }
	// // if(dx){
		// // if(dy < 0){
			// // if(y1 < y2) gy = TRUE;
		// // } else if(dy > 0){
			// // if(y1 > y2) gy = TRUE;
		// // } else {
			// // if(dx > 0 && x2 > x1){
				// // int deltax = x2 - x1, deltay = y2 - y1;
				// // if(deltay < deltax && deltay < -1*deltax){
					// // gx = TRUE;
					// // gy = TRUE;
				// // }
			// // }
			// // else if(dx < 0 && x2 < x1){
				// // int deltax = x2 - x1, deltay = y2 - y1;
				// // if(deltay > deltax && deltay > -1*deltax){
					// // gx = TRUE;
					// // gy = TRUE;
				// // }
			// // }
		// // }
	// // }
	// return gx && gy;
}

void
damningdark(x,y,val)
int x, y;
genericptr_t val;
{
	int nd = *((int *)val);
	struct monst *mon=m_at(x,y);
	if (levl[x][y].lit){
	    levl[x][y].lit = 0;
	    snuff_light_source(x, y);
		newsym(x,y);
		if(mon){
			if(is_undead_mon(mon) || resists_drain(mon) || is_demon(mon->data)){
				shieldeff(mon->mx, mon->my);
			} else {
				setmangry(mon);
				mon->mhp -= d(5, nd);
				pline("%s weakens.", Monnam(mon));
			}
		}
	} else {
		if(mon){
			if(is_undead_mon(mon) || resists_drain(mon) || is_demon(mon->data)){
				shieldeff(mon->mx, mon->my);
			} else {
				setmangry(mon);
				mon->mhp -= d(rnd(5), nd);
				pline("%s weakens.", Monnam(mon));
			}
		}
	}
	if (mon && mon->mhp <= 0){
		mon->mhp = 0;
		xkilled(mon, 1);
	}
}

STATIC_OVL boolean
masterDoorBox(x,y)
	int x, y;
{
	struct obj *otmp;
	char qbuf[QBUFSZ];
	char c;
	for(otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
	if (Is_box(otmp)) {
		if (!can_reach_floor()) {
			You_cant("reach %s from up here.", the(xname(otmp)));
			return FALSE;
		}
		
		if (otmp->obroken || !otmp->olocked) continue;
		
		Sprintf(qbuf, "There is %s here, unlock it?",
				safe_qbuf("", sizeof("There is  here, unlock its lock?"),
				doname(otmp), an(simple_typename(otmp->otyp)), "a box"));

		c = ynq(qbuf);
		if(c == 'q') return(FALSE);
		if(c == 'n') continue;
		
		otmp->olocked = 0;
		return TRUE;
	}
	return FALSE;
}

boolean
tt_findadjacent(cc, mon)
coord *cc;
struct monst *mon;
{
	int x, y, spot=0;
	for(x = mon->mx-1; x <= mon->mx+1; x++){
		for(y = mon->my-1; y <= mon->my+1; y++){
			if(teleok(x,y,FALSE) && !m_at(x,y)){
				spot++;
			}
		}
	}
	if(spot>1) spot = rnd(spot);
	for(x = mon->mx-1; x <= mon->mx+1; x++){
		for(y = mon->my-1; y <= mon->my+1; y++){
			if(teleok(x,y,FALSE) && !m_at(x,y) && !(--spot)){
				cc->x = x;
				cc->y = y;
				return TRUE;
			}
		}
	}
	return FALSE;
}

int
spiritDsize()
{
    int bonus = 1;
    if(ublindf && ublindf->oartifact == ART_SOUL_LENS) bonus = 2;
	if(u.ulevel <= 2) return 1 * bonus;
	else if(u.ulevel <= 5) return 2 * bonus;
	else if(u.ulevel <= 9) return 3 * bonus;
	else if(u.ulevel <= 13) return 4 * bonus;
	else if(u.ulevel <= 17) return 5 * bonus;
	else if(u.ulevel <= 21) return 6 * bonus;
	else if(u.ulevel <= 25) return 7 * bonus;
	else if(u.ulevel <= 29) return 8 * bonus;
	else return 9 * bonus;
}
STATIC_PTR int
purifying_blast()
{
	struct monst *mon;
	int dmg;
	int dsize = spiritDsize();
	
	mon = m_at(u.ux+u.dx, u.uy+u.dy);
	if(!mon){
		buzz(AD_FIRE, SPBOOK_CLASS, TRUE, 0,
			u.ux, u.uy, u.dx, u.dy,25,d(10,dsize));
	} else if(resists_elec(mon) || resists_disint(mon)){
		shieldeff(mon->mx, mon->my);
		buzz(AD_FIRE, SPBOOK_CLASS, TRUE, 0,
			u.ux+u.dx, u.uy+u.dy, u.dx, u.dy,25,d(10,dsize));
	} else {
		mhurtle(mon, u.dx, u.dy, 25);
		dmg = d(10,dsize);
		mon->mhp -= dmg;
		setmangry(mon);
		if(hates_silver(mon->data)) mon->mhp -= d(5,dsize);
		if (mon->mhp <= 0){
			xkilled(mon, 1);
		}
		buzz(AD_FIRE, SPBOOK_CLASS, TRUE, 0,
			u.ux, u.uy, u.dx, u.dy,25,d(10,dsize));
	}
	// u.uacinc-=7;  //Note: was added when purifying blast began to charge.
	return 0;
}

STATIC_PTR int
stargate()
{
	int i, num_ok_dungeons, last_ok_dungeon = 0;
	d_level newlev;
	extern int n_dgns; /* from dungeon.c */
	winid tmpwin = create_nhwindow(NHW_MENU);
	anything any;

	any.a_void = 0;	/* set all bits to zero */
	start_menu(tmpwin);
	/* use index+1 (cant use 0) as identifier */
	for (i = num_ok_dungeons = 0; i < n_dgns; i++) {
	if (!dungeons[i].dunlev_ureached) continue;
	any.a_int = i+1;
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
		 dungeons[i].dname, MENU_UNSELECTED);
	num_ok_dungeons++;
	last_ok_dungeon = i;
	}
	end_menu(tmpwin, "Open a portal to which dungeon?");
	if (num_ok_dungeons > 1) {
	/* more than one entry; display menu for choices */
	menu_item *selected;
	int n;

	n = select_menu(tmpwin, PICK_ONE, &selected);
	if (n <= 0) {
		destroy_nhwindow(tmpwin);
		return 0;
	}
	i = selected[0].item.a_int - 1;
	free((genericptr_t)selected);
	} else
	i = last_ok_dungeon;	/* also first & only OK dungeon */
	destroy_nhwindow(tmpwin);

	/*
	 * i is now index into dungeon structure for the new dungeon.
	 * Find the closest level in the given dungeon, open
	 * a use-once portal to that dungeon and go there.
	 * The closest level is either the entry or dunlev_ureached.
	 */
	newlev.dnum = i;
	if(dungeons[i].depth_start >= depth(&u.uz))
	newlev.dlevel = dungeons[i].entry_lev;
	else
	newlev.dlevel = dungeons[i].dunlev_ureached;
	if(u.uhave.amulet || In_endgame(&u.uz) || In_endgame(&newlev) ||
	   newlev.dnum == u.uz.dnum) {
	You_feel("very disoriented for a moment.");
	} else {
	if(u.usteed && mon_has_amulet(u.usteed)){
		dismount_steed(DISMOUNT_VANISHED);
	}
	if(!Blind) You("are surrounded by a shimmering sphere!");
	else You_feel("weightless for a moment.");
	u.uen -= 125;
	if(u.uen < 0 && !Race_if(PM_INCANTIFIER)) u.uen = 0;
	goto_level(&newlev, FALSE, FALSE, FALSE);
	}
	return 0;
}

int
spiriteffects(power, atme)
	int power;
	boolean atme;
{
	int dsize = spiritDsize();
	int tmp, weptmp, tchtmp;
	
	boolean reveal_invis = FALSE;
	switch(power){
		case PWR_ABDUCTION:{
			struct monst *mon;
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon){
					pline("There is no target there.");
					break;
				}
				if(mon->uhurtm && mon->data->geno & G_GENO){
#define MAXVALUE 24
					extern const int monstr[];
					int value = min(monstr[monsndx(mon->data)] + 1,MAXVALUE);
					Your("shadow flows under %s, swallowing %s up!",mon_nam(mon),mhim(mon));
					cprefx(monsndx(mon->data), TRUE, TRUE);
					cpostfx(monsndx(mon->data), FALSE, TRUE, FALSE);
					if(u.ugangr[Align2gangr(u.ualign.type)]) {
						u.ugangr[Align2gangr(u.ualign.type)] -= ((value * (u.ualign.type == A_CHAOTIC ? 2 : 3)) / MAXVALUE);
						if(u.ugangr[Align2gangr(u.ualign.type)] < 0) u.ugangr[Align2gangr(u.ualign.type)] = 0;
					} else if(u.ualign.record < 0) {
						if(value > MAXVALUE) value = MAXVALUE;
						if(value > -u.ualign.record) value = -u.ualign.record;
						adjalign(value);
					} else if (u.ublesscnt > 0) {
						u.ublesscnt -=
						((value * (u.ualign.type == A_CHAOTIC ? 500 : 300)) / MAXVALUE);
						if(u.ublesscnt < 0) u.ublesscnt = 0;
					}
					mongone(mon);
				} else {
					Your("shadow flows under %s, but nothing happens.",mon_nam(mon));
				}
			} else {
				pline("There is no target there.");
				break;
			}
		}break;
		case PWR_FIRE_BREATH:
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			buzz(AD_FIRE, FOOD_CLASS, TRUE, 0,
				u.ux, u.uy, u.dx, u.dy,0,d(5,dsize));
		break;
		case PWR_TRANSDIMENSIONAL_RAY:{
			int dmg;
			int range = rn1(7,7);
			xchar lsx, lsy, sx, sy;
			struct monst *mon;
			sx = u.ux;
			sy = u.uy;
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			if(u.uswallow){
				mon = u.ustuck;
				if (!resists_magm(mon)) {
					dmg = d(rnd(5),dsize);
					mon->mhp -= dmg;
					if (mon->mhp <= 0){
						xkilled(mon, 1);
					} else {
						You("hit %s.",mon_nam(mon));
						setmangry(mon);
						u_teleport_mon(mon, TRUE);
					}
				}
			}
			while(range-- > 0){
				sx += u.dx;
				sy += u.dy;
				if(isok(sx,sy)) {
					mon = m_at(sx, sy);
					/* reveal/unreveal invisible monsters before tmp_at() */
					if (mon && !canspotmon(mon) && cansee(sx,sy))
						map_invisible(sx, sy);
					else if (!mon && glyph_is_invisible(levl[sx][sy].glyph)) {
						unmap_object(sx, sy);
						newsym(sx, sy);
					}
					if (mon) {
						reveal_invis = TRUE;
						if (resists_magm(mon)) {	/* match effect on player */
							shieldeff(mon->mx, mon->my);
						} else {
							dmg = d(d(1,5),dsize);
							mon->mhp -= dmg;
							if (mon->mhp <= 0){
								xkilled(mon, 1);
								continue;
							} else{
								You("hit %s.",mon_nam(mon));
								setmangry(mon);
							}
							u_teleport_mon(mon, TRUE);
						}
					}
				} else break;
			}
		}break;
		case PWR_TELEPORT:
			tele();//hungerless teleport
		break;
		case PWR_JESTER_S_MIRTH:{
			struct monst *mon;
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon){
					pline("There is no target there.");
					break;
				}
				mon->mnotlaugh = 0;
				mon->mlaughing = d(1,5)+u.ulevel/10;
				pline("%s collapses in a fit of laughter.", Monnam(mon));
			} else{
				pline("There is no target there.");
				break;
			}
		}break;
		case PWR_THIEF_S_INSTINCTS:
			You("exercise your instincts.");
			findit();
		break;
		case PWR_ASTAROTH_S_ASSEMBLY:{
			int dmg;
			int range = 5;
			xchar lsx, lsy, sx, sy;
			struct monst *mon;
			sx = u.ux;
			sy = u.uy;
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			if(u.uswallow){
				explode(u.ux, u.uy, AD_ELEC, WAND_CLASS, d(range, dsize) * (Double_spell_size ? 3 : 2) / 2, EXPL_MAGICAL, 1 + !!Double_spell_size);
			} else {
				while(--range >= 0){
					lsx = sx; sx += u.dx;
					lsy = sy; sy += u.dy;
					if(isok(sx,sy) && isok(lsx,lsy) && !IS_STWALL(levl[sx][sy].typ)) {
						mon = m_at(sx, sy);
						if(mon){
							dmg = d(range+1,dsize); //Damage decreases with range
							explode(sx, sy, AD_ELEC, WAND_CLASS, dmg * (Double_spell_size ? 3 : 2) / 2, EXPL_MAGICAL, 1 + !!Double_spell_size);
							break;//break loop
						}
					} else {
						if(range < 4) range++;
						dmg = d(range+1,dsize); //Damage decreases with range
						explode(lsx, lsy, AD_ELEC, WAND_CLASS, dmg * (Double_spell_size ? 3 : 2) / 2, EXPL_MAGICAL, 1 + !!Double_spell_size);
						break;//break loop
					}
				}
				pline("Leftover electrical potential produces a field around you.");
				u.uspellprot = max(5 - (range+1),u.uspellprot);
				u.usptime = 5;
				u.uspmtime = 5;
			}
		}break;
		case PWR_ASTAROTH_S_SHARDS:
		if (getdir((char *)0) || !(u.dx || u.dy)){
		    struct obj *otmp;
			int i, x, y;
			x =  u.ux;
			y = u.uy;
			for(i = dsize+u.ulevel/10+1; i > 0; i--){
				int xadj=0;
				int yadj=0;
				if(u.dy == 0) yadj = d(1,3)-2 + d(1,3)-2;
				else if(u.dx == 0) xadj = d(1,3)-2 + d(1,3)-2;
				else if(u.dx == u.dy){
					int dd = d(1,3)-2 + d(1,3)-2;
					xadj = dd;
					yadj = -1 * dd;
				}
				else{
					int dd = d(1,3)-2 + d(1,3)-2;
					xadj = yadj = dd;
				}
				if(!isok(u.ux + xadj + u.dx,u.uy + yadj + u.dy) ||
					!isok(u.ux + xadj,u.uy + yadj) ||
					!ACCESSIBLE(levl[u.ux + xadj + u.dx][u.uy + yadj + u.dy].typ)
				){
					if(rn2(5)) i++;
					continue;
				}
				otmp = mksobj(SHURIKEN, TRUE, FALSE);
			    otmp->blessed = 0;
			    otmp->cursed = 0;
				set_destroy_thrown(1); //state variable referenced in drop_throw
					m_throw(&youmonst, u.ux + xadj, u.uy + yadj, u.dx, u.dy,
						rn1(5,5), otmp,TRUE);
					nomul(0, NULL);
				set_destroy_thrown(0);  //state variable referenced in drop_throw
			}
			losehp(dsize, "little shards of metal ripping out of your body", KILLED_BY);
		}
		break;
		case PWR_ICY_GLARE:{
			int range = (u.ulevel/2+1),dmg;
			struct monst *mprime, *mon, *nxtmon;
			xchar sx, sy;
			boolean gx=FALSE, gy=FALSE;
			sx = u.ux;
			sy = u.uy;
			if(Blind){
				You("need to be able to see in order to glare!");
				return 0;
			}
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			if(u.uswallow){
				You("can't see well enough in here!");
				break;
			}
			while(--range >= 0){
				sx += u.dx;
				sy += u.dy;
				if(isok(sx,sy)) {
				  if(cansee(sx,sy)){
					mon = m_at(sx, sy);
					if(mon && canseemon(mon)) {
						mprime = mon;
						if (resists_cold(mon)) {	/* match effect on player */
							shieldeff(mon->mx, mon->my);
						} else {
							dmg = d(5,dsize);
							mon->mhp -= dmg;
							if(resists_fire(mon)) mon->mhp -= dmg;
							if (mon->mhp <= 0){
								xkilled(mon, 1);
								break;
							}
							setmangry(mon);
						}
						break;
					}
				  }
				} else break;
			}
			range = (u.ulevel/2+1)^2;
			for(mon = fmon; mon; mon = nxtmon){
				nxtmon = mon->nmon;
				if (DEADMONSTER(mon)) continue;
				if(mprime != mon && canseemon(mon) && dist2(u.ux,u.uy,mon->mx,mon->my)<=range){
					if(sightwedge(u.dx,u.dy,u.ux,u.uy,mon->mx, mon->my)){
						if (resists_cold(mon)) {	/* match effect on player */
							shieldeff(mon->mx, mon->my);
						} else {
							dmg = 0.5*d(5,dsize);
							mon->mhp -= dmg;
							if(resists_fire(mon)) mon->mhp -= dmg;
							if (mon->mhp <= 0){
								mon->mhp = 0;
								xkilled(mon, 1);
								break;
							} else You("glare at %s.", mon_nam(mon));
							setmangry(mon);
						}
					}
				}
			}
			for(sx = 0; sx < COLNO; sx++){
				for(sy = 0; sy < ROWNO; sy++){
					if(levl[sx][sy].typ == POOL || levl[sx][sy].typ == MOAT){
						if(dist2(u.ux,u.uy,sx,sy)<=range && cansee(sx,sy)){
							if(sightwedge(u.dx,u.dy,u.ux,u.uy,sx,sy)){
								if(levl[sx][sy].typ == POOL){
									levl[sx][sy].icedpool = ICED_POOL;
								} else {
									levl[sx][sy].icedpool = ICED_MOAT;
								}
								levl[sx][sy].typ = ICE;
								newsym(sx,sy);
							}
						}
					}
				}
			}
			You("feel your eyes freeze and pop!");
			make_blinded(Blinded+5L,FALSE);
		}break;
		case PWR_BALAM_S_ANOINTING:{
			//thick-skinned creatures loose their eyes
			struct monst *mon;
			int dmg;
			if (!getdir((char *)0)  || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon){
					You("reach out to touch empty space!");
					break;
				} if(!freehand()){
					You("need a free hand to make a touch attack!");
					return 0;
				}
				find_to_hit_rolls(mon, &tmp, &weptmp, &tchtmp);
				if(tchtmp <= rnd(20)){
					You("miss.");
					break;
				}
				if (resists_cold(mon) || is_anhydrous(mon->data)) {	/* match effect on player */
					shieldeff(mon->mx, mon->my);
				} else {
					if(rn2(10) || !has_head(mon->data)){
						dmg = d(5,dsize);
					} else {
						pline("An icicle grows on %s head.", s_suffix(mon_nam(mon)));
						if(thick_skinned(mon->data)){
							dmg = d(10,dsize);
							if(haseyes(mon->data)){
								pline("It is blinded!");
								mon->mcansee = 0;
								mon->mblinded = 0;
							}
						} else {
							pline("It is pierced through!");
							dmg = mon->mhp;
						}
					}
					mon->mhp -= dmg;
					if(resists_fire(mon)) mon->mhp -= dmg;
					if (mon->mhp <= 0){
						mon->mhp = 0;
						xkilled(mon, 1);
						break;
					} else You("hit %s.", mon_nam(mon));
					setmangry(mon);
				}
			} else{
				You("reach out to touch empty space!");
				break;
			}
		}break;
		case PWR_BLOOD_MERCENARY:{
			int dmg;
			int range = rn1(7,7);
			boolean enoughGold;
			xchar sx, sy;
			struct monst *mon;
			sx = u.ux;
			sy = u.uy;
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			if(u.uswallow){
				enoughGold = FALSE;
				reveal_invis = TRUE;
				dmg = d(5,dsize);
#ifndef GOLDOBJ
				if (u.ugold >= dmg) enoughGold = TRUE;
#else
				if (money_cnt(invent) >= dmg) enoughGold = TRUE;
#endif
				if(enoughGold){
#ifndef GOLDOBJ
					u.ugold -= dmg;
#else
					money2none(dmg);
#endif
					mon->mhp -= dmg;
					if (mon->mhp <= 0){
						xkilled(mon, 1);
						break;
					} else You("hit %s.", mon_nam(mon));
					setmangry(mon);
				} else You("don't have enough gold on hand.");
			}
			You("fire a blast of gold and blood.");
			while(range-- > 0){
				sx += u.dx;
				sy += u.dy;
				if(isok(sx,sy) && !IS_ROCK(levl[sx][sy].typ)) {
					mon = m_at(sx, sy);
					/* reveal/unreveal invisible monsters before tmp_at() */
					if (mon && !canspotmon(mon) && cansee(sx,sy))
						map_invisible(sx, sy);
					else if (!mon && glyph_is_invisible(levl[sx][sy].glyph)) {
						unmap_object(sx, sy);
						newsym(sx, sy);
					}
					if (mon) {
						enoughGold = FALSE;
						reveal_invis = TRUE;
						dmg = d(5,dsize);
#ifndef GOLDOBJ
						if (u.ugold >= dmg) enoughGold = TRUE;
#else
						if (money_cnt(invent) >= dmg) enoughGold = TRUE;
#endif
						if(enoughGold){
#ifndef GOLDOBJ
							u.ugold -= dmg;
#else
							money2none(dmg);
#endif
							mon->mhp -= dmg;
							if (mon->mhp <= 0){
								xkilled(mon, 1);
								continue;
							} else You("hit %s.", mon_nam(mon));
							setmangry(mon);
						} You("don't have enough gold on hand.");
					}
				} else break;
			}
		}break;
		case PWR_SOW_DISCORD:
			You("sow discord amongst your enemies.");
			u.sowdisc = 1+rnd(dsize);
		break;
		case PWR_GIFT_OF_HEALING:{
			struct monst *mon;
			int dmg;
			if (!getdir((char *)0) || (u.dz)) return(0);
			if(!(u.dx || u.dy)){
				You("heal yourself.");
				healup(d(5,dsize), 0, FALSE, FALSE);
			} else if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon) break;
				You("heal %s.", mon_nam(mon));
				if (nonliving_mon(mon)){	/* match effect on player */
					shieldeff(mon->mx, mon->my);
				} else {
					mon->mhp += d(5,dsize);
					if(mon->mhp > mon->mhpmax) mon->mhp = mon->mhpmax;
				}
			} else break;
		}break;
		case PWR_GIFT_OF_HEALTH:{
			struct monst *mon;
			struct obj *pseudo;
			int dmg;
			if (!getdir((char *)0) || (u.dz)) return(0);
			if(!(u.dx || u.dy)){
				int idx, recover, val_limit, aprobs = 0, fixpoint, curpoint;
				
				You("recover.");
				if (Sick) make_sick(0L, (char *) 0, TRUE, SICK_ALL);
				if (Blinded > (long)u.ucreamed) make_blinded((long)u.ucreamed, TRUE);
				if (HHallucination) (void) make_hallucinated(0L, TRUE, 0L);
				if (Vomiting) make_vomiting(0L, TRUE);
				if (HConfusion) make_confused(0L, TRUE);
				if (HStun) make_stunned(0L, TRUE);
				if (Wounded_legs) HWounded_legs = 1L;


				/* collect attribute troubles */
				for (idx = 0; idx < A_MAX; idx++) {
					val_limit = AMAX(idx);
					/* don't recover strength lost from hunger */
					if (idx == A_STR && u.uhs >= WEAK) val_limit--;
					
					if (val_limit > ABASE(idx)) aprobs += val_limit - ABASE(idx);
				}
				if(aprobs) for(recover = 0; recover < 5; recover++){
					if(!aprobs) break;
					fixpoint = rnd(aprobs);
					curpoint = 0;
					for (idx = 0; idx < A_MAX; idx++) {
						val_limit = AMAX(idx);
						/* don't recover strength lost from hunger */
						if (idx == A_STR && u.uhs >= WEAK) val_limit--;
						
						if (val_limit > ABASE(idx)) curpoint += val_limit - ABASE(idx);
						if (curpoint >= fixpoint){
							ABASE(idx)++;
							aprobs--;
					break;
						}
					}
				}
			} else if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon) break;
				if (nonliving_mon(mon)){	/* match effect on player */
					shieldeff(mon->mx, mon->my);
				} else {
					pline("%s recovers.", Monnam(mon));
					if(mon->permspeed == MSLOW){
						mon->permspeed = 0;
						if(mon->mspeed == MSLOW) mon->mspeed = 0;
					}
					mon->mcan = 0;
					mon->mcrazed = 0; 
					mon->mcansee = 1;
					mon->mblinded = 0;
					mon->mcanmove = 1;
					mon->mfrozen = 0;
					mon->msleeping = 0;
					mon->mstun = 0;
					mon->mconf = 0;
					if (canseemon(mon))
						pline("%s looks recovered.", Monnam(mon));
				}
			} else break;
		}break;
		case PWR_THROW_WEBBING:
			if (getdir((char *)0) || !(u.dx || u.dy)){
				struct obj *otmp;
				You("throw a ball of webbing.");
				otmp = mksobj(BALL_OF_WEBBING, TRUE, FALSE);
				otmp->blessed = 0;
				otmp->cursed = 0;
				otmp->spe = 1; /* to indicate it's yours */
				otmp->ovar1 = 1 + u.ulevel/10;
				throwit(otmp, 0L, FALSE, 0);
				// m_throw(&youmonst, u.ux, u.uy, u.dx, u.dy,
					// rn1(5,5), otmp,TRUE);
				nomul(0, NULL);
			} else return 0;
		break;
		case PWR_THOUGHT_TRAVEL:{
			if(Is_astralevel(&u.uz)){
				dotravel();
			} else {
				coord cc;
				int cancelled;
				struct monst *mon;
				cc.x = u.ux;
				cc.y = u.uy;
				pline("To what creature do you wish to travel?");
				cancelled = getpos(&cc, TRUE, "the desired creature");
				while( !((mon = m_at(cc.x,cc.y)) && tp_sensemon(mon) && tt_findadjacent(&cc, mon)) && cancelled >= 0){
					if(!mon || !(sensemon(mon) || (mon_visible(mon) && cansee(mon->mx,mon->my)) )) You("don't sense anything there.");
					else if (!tp_sensemon(mon)) You("can't sense that monster's mind!");
					else if (!tt_findadjacent(&cc, mon)) pline("Something blocks your way!");
					cancelled = getpos(&cc, TRUE, "the desired creature");
				}
				if(cancelled < 0) return 0; /*abort*/
//			    if (u.usteed){
//				}
				teleds(cc.x, cc.y, FALSE);
			}
		}break;
		case PWR_DREAD_OF_DANTALION:{
			register struct monst *mtmp;
			You("evoke dread!");
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
				if (DEADMONSTER(mtmp)) continue;
				if(cansee(mtmp->mx,mtmp->my) && !(mtmp->data->geno & G_UNIQ)) {
					monflee(mtmp, d(5,dsize), FALSE, TRUE);
				}
			}
		}break;
		case PWR_EARTH_SWALLOW:{
			if (!getdir((char *)0)  || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				struct obj *otmp;
				You("ask the earth to open.");
				digfarhole(TRUE, u.ux+u.dx, u.uy+u.dy);
				otmp = mksobj(BOULDER, TRUE, FALSE);
				m_throw(&youmonst, u.ux, u.uy, u.dx, u.dy, 1, otmp,TRUE);
				nomul(0, NULL);
			} else break;
		}break;
		case PWR_ECHIDNA_S_VENOM:{
			struct obj *otmp;
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			otmp = mksobj(ACID_VENOM, TRUE, FALSE);
			otmp->spe = 1; /* to indicate it's yours */
			otmp->ovar1 = d(5,dsize); /* save the damge this should do */
			You("spit venom.");
			m_throw(&youmonst, u.ux, u.uy, u.dx, u.dy, 10, otmp,TRUE);
			// throwit(otmp, 0L, FALSE, 0);
		}break;
		case PWR_SUCKLE_MONSTER:{
			struct monst *mon;
			int dmg;
			if (!getdir((char *)0)  || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon) break;
				You("croon the sibilant lullaby of Echidna.");
				if(rn2(u.ulevel) > mon->m_lev && !mindless_mon(mon) && (is_animal(mon->data) || slithy(mon->data) || nohands(mon->data))){
					if (mon->isshk) make_happy_shk(mon, FALSE);
					else (void) tamedog(mon, (struct obj *) 0);
				} else {
					if(resists_sleep(mon) || resist(mon, '\0', 0, TELL)) shieldeff(mon->mx, mon->my);
					else {
						mon->msleeping = 1;
						slept_monst(mon);
					}
				}
			} else break;
		}break;
		case PWR_PURIFYING_BLAST:{
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				// u.uacinc+=7; //Note: purifying_blast subtracts 7 from uacinc to compensate.
				u.edenshield = moves+5;
				nomul(-5, "charging up a purifying blast");
				nomovemsg = "You finish charging.";
				afternmv = purifying_blast;
			}
		}break;
		case PWR_RECALL_TO_EDEN:{
			struct monst *mon;
			int perc;
			if (!getdir((char *)0)  || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon || mon->data->geno & G_UNIQ) break;
				You("attempt to recall %s to Eden.", mon_nam(mon));
				if(Upolyd) perc = (u.mh - mon->mhp)*100/u.mh;
				else perc = (u.uhp - mon->mhp)*100/u.uhp;
				
				if(perc>0 && rnd(100) < perc){
					pline("%s vanishes.", Monnam(mon));
					healup(d(rnd(5),dsize), 0, FALSE, FALSE);
					mongone(mon);
					break;
				} else You("fail.");
				setmangry(mon);
			} else break;
		}break;
		case PWR_STARGATE:
			u.edenshield = moves+5;
			nomul(-5, "charging up a stargate");
			nomovemsg = "You finish charging.";
			afternmv = stargate;
		break;
		case PWR_WALKER_OF_THRESHOLDS:{
			coord cc;
			int cancelled;
		    cc.x = u.ux;
		    cc.y = u.uy;
		    pline("To which doorway do you wish to travel?");
			do cancelled = getpos(&cc, TRUE, "the desired doorway");
			while( !(IS_DOOR(levl[cc.x][cc.y].typ) && teleok(cc.x, cc.y, FALSE)) && cancelled >= 0);
			if(cancelled < 0) return 0; /*abort*/
//		    if (u.usteed){
//			}
			teleds(cc.x, cc.y, FALSE);
		}break;
		case PWR_GEYSER:{
			int dmg = 0;
			struct monst *mon;
			struct trap *t;
			if (!getdir((char *)0)  || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				t = t_at(u.ux+u.dx, u.uy+u.dy);
				if(mon){
					struct obj* boots;
					boots = which_armor(mon, W_ARMF);
					find_to_hit_rolls(mon, &tmp, &weptmp, &tchtmp);
					if(tmp <= rnd(20)){
						if(boots && boots->otyp == WATER_WALKING_BOOTS){
							pline("A sudden geyser from the abzu washes under %s's feet!", mon_nam(mon));
							if(canseemon(mon)) makeknown(boots->otyp);
						} else {
							pline("A sudden geyser from the abzu washes over %s!", mon_nam(mon));
							dmg = d(rnd(5), dsize);
							water_damage(mon->minvent, FALSE, FALSE, FALSE, mon);
						}
					} else {
						if(boots && boots->otyp == WATER_WALKING_BOOTS){
							pline("A sudden geyser from the abzu erupts under %s's feet!", mon_nam(mon));
							if(mon->data->mmove >= 14){
								pline("%s puts the added momentum to good use!", Monnam(mon));
								if(mon->data->mmove >= 25) mon->movement += 12;
								else if(mon->data->mmove >= 18) mon->movement += 8;
								else mon->movement += 6;
							} else if(mon->data->mmove <= 10){
								pline("%s is knocked around by the geyser's force!", Monnam(mon));
								if(mon->data->mmove <= 3) dmg = d(8, dsize);
								else if(mon->data->mmove <= 6) dmg = d(4, dsize);
								else dmg = rnd(dsize);
							}
							if(canseemon(mon)) makeknown(boots->otyp);
						} else {
							pline("A sudden geyser from the abzu slams into %s!", mon_nam(mon));
							dmg = d(8, dsize);
							water_damage(mon->minvent, FALSE, FALSE, FALSE, mon);
						}
					}
					if(flaming(mon->data) || mon->data == &mons[PM_EARTH_ELEMENTAL] || mon->data == &mons[PM_IRON_GOLEM] || mon->data == &mons[PM_CHAIN_GOLEM]) dmg *= 2;
					if(mon->data == &mons[PM_GREMLIN] && rn2(3)){
						(void)split_mon(mon, (struct monst *)0);
					}
					mon->mhp -= dmg;
					if (mon->mhp <= 0){
						mon->mhp = 0;
						xkilled(mon, 1);
						break;
					}
					setmangry(mon);
				}
			} else break;
		}break;
		case PWR_VENGANCE:{
			struct monst *mon;
			int i,j;
			You("lash out in vengeance!");
			for(i=-1;i<=1;i++){
			  for(j=-1;j<=1;j++){
				if((i!=0 || j!=0) && isok(u.ux+i, u.uy+j) && (mon = m_at(u.ux+i, u.uy+j)) && mon->mhurtu){
					mon->mhp -= d(5,dsize);
					if (mon->mhp <= 0){
						mon->mhp = 0;
						xkilled(mon, 1);
						break;
					} else You("hit %s.",mon_nam(mon));
					setmangry(mon);
				}
			  }
			}
		}break;
		case PWR_SHAPE_THE_WIND:{
			struct monst *mon;
			struct permonst *pm;
			int i;
			You("dance and shape the wind.");
			for(i=dsize; i > 0; i--){
				do pm = &mons[rn2(PM_LONG_WORM_TAIL)];
				while( (pm->geno & (G_UNIQ|G_NOGEN)) || pm->mlevel >= u.ulevel+5);
				mon = makemon(pm, u.ux, u.uy, MM_EDOG|MM_ADJACENTOK|MM_NOCOUNTBIRTH|NO_MINVENT);
				if(mon){
					initedog(mon);
					mon->mvanishes = 5;
				}
			}
		}break;
		case PWR_THORNS_AND_STONES:{
			struct obj *otmp;
			You("pull out some of the thorns and stones that torment Eve.");
			if(uwep){
				switch(uwep->otyp){
					case BOW:
						otmp = mksobj(ARROW, TRUE, FALSE);
						otmp->objsize = uwep->objsize;
					break;
					case ELVEN_BOW:
						otmp = mksobj(ELVEN_ARROW, TRUE, FALSE);
						otmp->objsize = uwep->objsize;
					break;
					case ORCISH_BOW:
						otmp = mksobj(ORCISH_ARROW, TRUE, FALSE);
						otmp->objsize = uwep->objsize;
					break;
					case YUMI:
						otmp = mksobj(YA, TRUE, FALSE);
						otmp->objsize = uwep->objsize;
					break;
					case SLING:
						otmp = mksobj(FLINT, TRUE, FALSE);
					break;
					case CROSSBOW:
						otmp = mksobj(CROSSBOW_BOLT, TRUE, FALSE);
						otmp->objsize = uwep->objsize;
					break;
					case DROVEN_CROSSBOW:
						otmp = mksobj(DROVEN_BOLT, TRUE, FALSE);
						otmp->objsize = uwep->objsize;
					break;
					default:
						if(!rn2(3)){
							otmp = mksobj(FLINT, TRUE, FALSE);
						} else if(!rn2(2)){
							otmp = mksobj(ARROW, TRUE, FALSE);
							otmp->objsize = uwep->objsize;
						} else {
							otmp = mksobj(CROSSBOW_BOLT, TRUE, FALSE);
							otmp->objsize = uwep->objsize;
						}
					break;
				}
			} else{
				if(!rn2(3)){
					otmp = mksobj(FLINT, TRUE, FALSE);
				} else if(!rn2(2)){
					otmp = mksobj(ARROW, TRUE, FALSE);
					otmp->objsize = youracedata->msize;
				} else {
					otmp = mksobj(CROSSBOW_BOLT, TRUE, FALSE);
					otmp->objsize = youracedata->msize;
				}
			}
			fix_object(otmp);
			otmp->blessed = 0;
			otmp->cursed = 0;
			otmp->bknown = 1;
			otmp->spe = 0;
			otmp->quan = rnd(5) + rnd(dsize);
			otmp->owt = weight(otmp);
			losehp(dsize, "thorns and stones piercing your body", KILLED_BY);
			otmp = hold_another_object(otmp, "They %s out.",
						   aobjnam(otmp, "fall"), (const char *)0);
		}break;
		case PWR_BARAGE:
			You("get ready to fire a barrage.");
			if(uquiver){
				barage = TRUE; //state variable
				throw_obj(uquiver, 0, 1);
				barage = FALSE;
			} else {
				You("have nothing quivered.");
				return 0;
			}
		break;
		case PWR_BREATH_POISON:{
	        coord cc;
			pline("Breathe where?");
			cc.x = u.ux;
			cc.y = u.uy;
			if (getpos(&cc, TRUE, "the desired position") < 0) {
				pline1(Never_mind);
				return 0;
			}
			if (!cansee(cc.x, cc.y) || distu(cc.x, cc.y) >= 32) {
				You("smell rotten eggs.");
				break;
			}
			(void) create_gas_cloud(cc.x, cc.y, u.ulevel/10+1, d(rnd(5),dsize));
		}break;
		case PWR_RUINOUS_STRIKE:{
			int dmg;
			struct monst *mon;
			struct trap *ttmp;
			if (u.utrap){
				You("can't use a ruinous strike while stuck in a trap!");
				return 0;
			}
			if (!getdir((char *)0)  || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				You("deliver a ruinous strike.");
				zap_dig(-1,-1,1);
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if ((ttmp = t_at(u.ux+u.dx, u.uy+u.dy)) && ttmp->ttyp != PIT && ttmp->ttyp != HOLE && ttmp->ttyp != MAGIC_PORTAL) {
					if(ttmp->ttyp == SPIKED_PIT){
						ttmp->ttyp = PIT;
						ttmp->tseen = 1;
						newsym(u.ux+u.dx, u.uy+u.dy);
					} else if(ttmp->ttyp == TRAPDOOR){
						ttmp->ttyp = HOLE;
						ttmp->tseen = 1;
						newsym(u.ux+u.dx, u.uy+u.dy);
					} else {
						deltrap(ttmp);
						newsym(u.ux+u.dx, u.uy+u.dy);
					}
					if(mon && ttmp->ttyp != PIT) mon->mtrapped = 0;
				}
				if(mon && is_golem(mon->data)){
					mon->mhp = 0;
					xkilled(mon, 1);
				} else if(mon && nonliving_mon(mon)){
					mon->mhp -= d(rnd(5),dsize);
					if(mon->mhp <= 0){
						mon->mhp = 0;
						xkilled(mon,1);
						break;
					} else You("hit %s.", mon_nam(mon));
					setmangry(mon);
				}
			} else break;
		}break;
		case PWR_RAVEN_S_TALONS:{
			int dmg;
			struct monst *mon;
			if (!getdir((char *)0)  || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon) break;
				dmg = d(rnd(5),dsize);
				if(haseyes(mon->data) && mon->mcansee && can_blnd(&youmonst, mon, AT_CLAW, (struct obj*)0)){
					if(mon->data == &mons[PM_FLOATING_EYE]){
						You("claw at %s.", mon_nam(mon));
						dmg += d(5,dsize); //Bonus damage, since this renders wards inefective and is single target.
						dmg *= 2;
					} else {
						You("claw at %s's eyes.", mon_nam(mon));
						dmg += d(5,dsize); //Bonus damage, since this renders wards inefective and is single target.
					}
					mon->mcansee = 0;
					mon->mblinded = 0;
				}
				mon->mhp -= dmg;
				if (mon->mhp <= 0){
					mon->mhp = 0;
					xkilled(mon, 1);
					break;
				} else You("hit %s.", mon_nam(mon));
				setmangry(mon);
			} else break;
		}break;
		case PWR_HORRID_WILTING:{
			int dmg;
			struct monst *mon;
			if (!getdir((char *)0)  || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon){
					pline("\"There's no one there, buddy!\"");
					return 0;
				} if(nonliving_mon(mon) || is_anhydrous(mon->data)){
					shieldeff(mon->mx, mon->my);
					break;
				}
				dmg = d(5,dsize);
				u.irisAttack = moves;
				pline("Arcs of iridescent droplets tear free from %s body!", s_suffix(mon_nam(mon)));
				pline("They merge with and are absorbed by your tentacles!");
				if(mon->data==&mons[PM_WATER_ELEMENTAL] || mon->data==&mons[PM_FOG_CLOUD] || mon->data==&mons[PM_ILLURIEN_OF_THE_MYRIAD_GLIMPSES]) dmg *= 2;
				if (mon->mhp <= dmg){
					dmg = mon->mhp;
					mon->mhp = 0;
					xkilled(mon, 1);
					break;
				} else{
					mon->mhp -= dmg;
					setmangry(mon);
				}
				healup(dmg, 0, FALSE, FALSE);
			} else {
				pline("\"There's no one there, buddy!\"");
				return 0;
			}
		}break;
		case PWR_TURN_ANIMALS_AND_HUMANOIDS:{
			register struct monst *mtmp;
			int range = BOLT_LIM + dsize;	/* 5 to 14 */
			range *= range;
			You("try to turn away or slay animals and humanoids.");
			pline("A rainbow of unearthly colors dances before your eyes!");
			for(mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
				if (DEADMONSTER(mtmp)) continue;
				if(cansee(mtmp->mx,mtmp->my) && distu(mtmp->mx,mtmp->my) <= range &&
					(is_animal(mtmp->data) || 
					 mtmp->data->mlet == S_HUMAN  || 
					 mtmp->data->mlet == S_HUMANOID ) 
					 && !mindless_mon(mtmp)
					 && !resist(mtmp, '\0', 0, TELL)
				) {
					if(u.ulevel >= mtmp->m_lev+6 && !resist(mtmp, '\0', 0, NOTELL)){
						killed(mtmp);
					} else monflee(mtmp, d(5,dsize), FALSE, TRUE);
				}
			}
			nomul(5,"recovering from the Horrid Rainbow");
		}break;
		case PWR_REFILL_LANTERN:
			if(uwep && (uwep->otyp == OIL_LAMP || (is_lightsaber(uwep) && uwep->oartifact != ART_INFINITY_S_MIRRORED_ARC && uwep->otyp != KAMEREL_VAJRA)) && !uwep->oartifact){
				int multiplier = is_lightsaber(uwep) ? 100 : 1;
				uwep->age += d(5,dsize) * 10 * multiplier;
				if(uwep->age > 1500*multiplier) uwep->age = 1500*multiplier;
				You("refill %s.",the(xname(uwep)));
				if(uwep->lamplit){
					end_burn(uwep, TRUE);
					begin_burn(uwep, FALSE);
				}
			} else return 0;
		break;
		case PWR_HELLFIRE:
			if(uwep && (uwep->otyp == OIL_LAMP || uwep->otyp == POT_OIL || (is_lightsaber(uwep) && uwep->oartifact != ART_INFINITY_S_MIRRORED_ARC && uwep->otyp != KAMEREL_VAJRA)) && !uwep->oartifact && uwep->lamplit){
				if (throwspell()) {
					if(uwep->age < 500) uwep->age = 0;
					else uwep->age -= 500;
					explode(u.dx, u.dy, AD_FIRE, WAND_CLASS, d(rnd(5), dsize)* (Double_spell_size ? 3 : 2) / 2, EXPL_FIERY, 1 + !!Double_spell_size);
					end_burn(uwep, TRUE);
					begin_burn(uwep, FALSE);
				} else return 0;
			} else{
				if(uwep && uwep->otyp == LANTERN) pline("You need an oil lamp. These modern lamps just aren't the same!");
				else You("must wield a burning lamp!");
				return 0;
			}
		break;
		case PWR_CALL_MURDER:{
			struct monst *mon;
			You("call down backup!");
			mon = makemon(&mons[PM_CROW], u.ux, u.uy, MM_EDOG|MM_ADJACENTOK);
			if(mon){
				struct monst *curmon, *weakdog = (struct monst *)0;
				int numdogs = 0;
				initedog(mon);
				mon->m_lev += (u.ulevel - mon->m_lev)/3;
				mon->mhpmax = (mon->m_lev * 8) - 4;
				mon->mhp =  mon->mhpmax;
				for(curmon = fmon; curmon; curmon = curmon->nmon){
					if(curmon->mspiritual && curmon->mvanishes < 0){
						numdogs++;
						if(!weakdog) weakdog = curmon;
						if(weakdog->m_lev > curmon->m_lev) weakdog = curmon;
						else if(weakdog->mtame > curmon->mtame) weakdog = curmon;
						else if(weakdog->mtame > curmon->mtame) weakdog = curmon;
						else if(weakdog->mtame > curmon->mtame) weakdog = curmon;
					}
				}
				if(weakdog && numdogs > (ACURR(A_CHA)/3) ) mon->mvanishes = 5;
				mon->mspiritual = TRUE;
			}
		}break;
		case PWR_ROOT_SHOUT:{
			int dmg;
			int range = rn1(7,7);
			xchar sx, sy;
			struct monst *mon;
			sx = u.ux;
			sy = u.uy;
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			You("scream!");
			if(u.uswallow){
				zap_dig(-1,-1,1); /*try to blast free of engulfing monster*/
				if(u.uswallow) break;
			}
			zap_dig(-1,-1,range);
			while(range-- > 0){
				sx += u.dx;
				sy += u.dy;
				if(isok(sx,sy) && !IS_ROCK(levl[sx][sy].typ)) {
					mon = m_at(sx, sy);
					/* reveal/unreveal invisible monsters before tmp_at() */
					if (mon && !canspotmon(mon) && cansee(sx,sy))
						map_invisible(sx, sy);
					else if (!mon && glyph_is_invisible(levl[sx][sy].glyph)) {
						unmap_object(sx, sy);
						newsym(sx, sy);
					}
					if (mon) {
						mon->mhp -= d(rnd(5), dsize);/*lower damage*/
						if (mon->mhp <= 0){
							xkilled(mon, 1);
							continue;
						} else You("hit %s.", mon_nam(mon));
						setmangry(mon);
					}
				} else break;
			}
		}break;
		case PWR_PULL_WIRES:{
			int sx, sy, range,i;
			struct trap *t;
			const char *pullmsg = "The wires yank you out of the %s!";
			boolean once = FALSE;
			for(i=0;i<dsize;i++){
			if (!getdir((char *)0)  || !(u.dx || u.dy)) break;
				once = TRUE;
				sx = u.ux;
				sy = u.uy;
				for(range = (int)((ACURRSTR)/2 - 2); range > 0; range--){
					sx += u.dx;
					sy += u.dy;
					if(!(ACCESSIBLE(levl[sx][sy].typ) || 
						 levl[sx][sy].typ == POOL || 
						 levl[sx][sy].typ == MOAT || 
						 levl[sx][sy].typ == LAVAPOOL
						) || m_at(sx,sy) || !isok(sx,sy) ||
							(IS_DOOR(levl[sx][sy].typ) && (levl[sx][sy].doormask&(D_LOCKED|D_CLOSED|D_TRAPPED)))
					) {
						sx -= u.dx;
						sy -= u.dy;
				break;
					} else if(In_sokoban(&u.uz) ? (t = t_at(sx, sy)) && t->ttyp >= HOLE : (t = t_at(sx, sy)) && t->ttyp > TRAPDOOR){
				break; //trap indexed above trapdoors have triggers that extend above the ground. Sokoban winds prevent passage over holes and trapdoors
					}
				}
				if (sx != u.ux || sy != u.uy) {
					if (u.utrap && u.utraptype != TT_INFLOOR) {
						switch(u.utraptype) {
						case TT_PIT:
						pline(pullmsg, "pit");
						break;
						case TT_WEB:
						pline(pullmsg, "web");
						if(!Is_lolth_level(&u.uz) && !(u.specialSealsActive&SEAL_BLACK_WEB)){
							pline_The("web is destroyed!");
							deltrap(t_at(u.ux,u.uy));
						}
						break;
						case TT_LAVA:
						pline(pullmsg, "lava");
						break;
						case TT_BEARTRAP: {
						register long side = rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
						static int jboots6 = 0;
						if (!jboots6) jboots6 = find_jboots();
						pline(pullmsg, "bear trap");
	#ifdef STEED
						if(u.usteed) set_wounded_legs(side, rn1(1000, 500)); /*clunky as hell, but gets the job done! */
						if (!u.usteed)
	#endif
						{
							if (uarmf && uarmf->otyp == jboots6){
								int bootdamage = d(1,10);
								losehp(2, "leg damage from being pulled out of a bear trap",
										KILLED_BY);
								set_wounded_legs(side, rn1(100,50));
								if(bootdamage > uarmf->spe){
									claws_destroy_arm(uarmf);
								}else{
									for(; bootdamage >= 0; bootdamage--) drain_item(uarmf);
									Your("boots are damaged!");
								}
							}
							else{
								losehp(rn1(10,20), "leg damage from being pulled out of a bear trap", KILLED_BY);
								set_wounded_legs(side, rn1(1000,500));
								Your("%s %s is severely damaged.",
										(side == LEFT_SIDE) ? "left" : "right",
										body_part(LEG));
							}
						}
						}break;
						}
						u.utrap = 0;
						fill_pit(u.ux, u.uy);
					}
					u.ux0 = u.ux;
					u.uy0 = u.uy;
					u.ux = sx;
					u.uy = sy;
					vision_full_recalc = 1;	/* hero has moved, recalculate vision later */
					doredraw();
				}
				spoteffects(FALSE);
			}
			if(!once) return 0; //Canceled first prompt
		}break;
		case PWR_DISGUSTED_GAZE:{
			struct monst *mon;
			struct obj *obj;
			if(!uarmg && !(uarmc && uarmc->otyp == MUMMY_WRAPPING)){
				if(throwgaze()){
					if((mon = m_at(u.dx,u.dy)) && canseemon(mon)){
						Your("arms swing up and your hands jerk open in a single, spasmodic motion.");
						if(uwep){
							obj = uwep;
							uwepgone();
							obj_extract_self(obj);
							You("drop %s.",the(xname(obj))); 
							dropy(obj);
						}
						if(uswapwep && u.twoweap){
							obj = uswapwep;
							untwoweapon();
							uswapwepgone();
							obj_extract_self(obj);
							You("drop %s.",the(xname(obj))); 
							dropy(obj);
						}
						youmonst.movement += NORMAL_SPEED;
						pline("The wild, staring eyes that cover your hands focus on %s.", mon_nam(mon));
						if(distmin(u.ux, u.uy, mon->mx, mon->my) <= u.ulevel/10+1 && !resist(mon, '\0', 0, NOTELL)){
							mon->mcanmove = 0;
							mon->mfrozen = max(mon->mfrozen, 5);
						}
						pline("%s is struck by a bolt of lightning.", Monnam(mon));
						if (resists_elec(mon)) {
							shieldeff(mon->mx, mon->my);
						} else {
							mon->mhp -= d(5,dsize);
							if (mon->mhp <= 0){
								mon->mhp = 0;
								xkilled(mon, 1);
								break;
							}
							setmangry(mon);
						}
					} else {
						You("don't see a monster there.");
						return 0;
					}
				} else return 0;
			} else{
				pline("The eyes on your hands are covered!");
				return 0;
			}
		}break;
		case PWR_BLOODY_TOUNGE:{
			struct monst *mon;
			if(!getdir((char *)0)  || !(u.dx || u.dy)) return 0;
			Your("forked red tongue speaks of its own accord.");
			mon = m_at(u.ux+u.dx, u.uy+u.dy);
			if(mon){
				mon->mflee = 1;//does not make monster hostile
				pline("%s turns to flee.", Monnam(mon));
			} else return 0;
		}break;
		case PWR_SILVER_TOUNGE:{
			struct monst *mon;
			if(!getdir((char *)0)  || !(u.dx || u.dy)) return 0;
			mon = m_at(u.ux+u.dx, u.uy+u.dy);
			if(mon){
				Your("forked tongue speaks with silvery grace.");
				if((!always_hostile_mon(mon) &&
				!(mon->data->geno & G_UNIQ) &&
				!mon->mtraitor) || !resist(mon, '\0', 0, NOTELL)
				){
					if (mon->isshk){
						make_happy_shk(mon, FALSE);
						mon->mpeaceful = 1;
					} else {
						(void) tamedog(mon, (struct obj *)0);
					}
				} else {
					shieldeff(mon->mx, mon->my);
					pline("%s doesn't waver, though.", Monnam(mon));
				}
			} else{
				pline("There's nothing there!");
				return 0;
			}
		}break;
		case PWR_EXHALATION_OF_THE_RIFT:{
			int dmg, i;
			int range = rn1(5,5);
			xchar lsx, lsy, sx, sy;
			struct monst *mon;
			sx = u.ux;
			sy = u.uy;
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			if(u.uswallow){
				mon = u.ustuck;
				if(is_whirly(mon->data)){
					pline("%s blows apart in the wind.",Monnam(mon));
					expels(mon, u.ustuck->data, TRUE);
					mon->mhp = 0;
					xkilled(mon, 1);
					break;
				} else expels(mon, mon->data, TRUE);
			} else {
				pline("a whistling wind springs up.");
				for(i=range; i > 0; i--){
					sx += u.dx;
					sy += u.dy;
					if(!isok(sx,sy) || !ZAP_POS(levl[sx][sy].typ)) {
						sx -= u.dx;
						sy -= u.dy;
				break;
					}
				}
				while(sx != u.ux || sy != u.uy){
					i++;
					mon = m_at(sx, sy);
					if(mon){
						mhurtle(mon, u.dx, u.dy, range+i);
						mon->mhp -= d(5,dsize);
						setmangry(mon);
						if (mon->mhp <= 0){
							mon->mhp = 0;
							xkilled(mon, 1);
						} else {
							pline("%s is flung back by the wind.",Monnam(mon));
							setmangry(mon);
						}
					}
					if(!u.dx){
						if(isok(sx+1,sy)){
							mon = m_at(sx+1, sy);
							if(mon){
								mhurtle(mon, u.dx, u.dy, range+i);
								mon->mhp -= d(rnd(5),dsize);
								if (mon->mhp <= 0){
									mon->mhp = 0;
									xkilled(mon, 1);
								} else {
									pline("%s is flung back by the wind.",Monnam(mon));
									setmangry(mon);
								}
							}
						}
						if(isok(sx-1,sy)){
							mon = m_at(sx-1, sy);
							if(mon){
								mhurtle(mon, u.dx, u.dy, range+i);
								mon->mhp -= d(rnd(5),dsize);
								if (mon->mhp <= 0){
									mon->mhp = 0;
									xkilled(mon, 1);
								} else {
									pline("%s is flung back by the wind.",Monnam(mon));
									setmangry(mon);
								}
							}
						}
					} else if(!u.dy){
						if(isok(sx,sy+1)){
							mon = m_at(sx, sy+1);
							if(mon){
								mhurtle(mon, u.dx, u.dy, range+i);
								mon->mhp -= d(rnd(5),dsize);
								if (mon->mhp <= 0){
									mon->mhp = 0;
									xkilled(mon, 1);
								} else {
									pline("%s is flung back by the wind.",Monnam(mon));
									setmangry(mon);
								}
							}
						}
						if(isok(sx,sy-1)){
							mon = m_at(sx, sy-1);
							if(mon){
								mhurtle(mon, u.dx, u.dy, range+i);
								mon->mhp -= d(rnd(5),dsize);
								if (mon->mhp <= 0){
									mon->mhp = 0;
									xkilled(mon, 1);
								} else {
									pline("%s is flung back by the wind.",Monnam(mon));
									setmangry(mon);
								}
							}
						}
					} else {
						if(isok(sx,sy-u.dy)){
							mon = m_at(sx, sy-u.dy);
							if(mon){
								mhurtle(mon, u.dx, u.dy, range+i);
								mon->mhp -= d(rnd(5),dsize);
								if (mon->mhp <= 0){
									mon->mhp = 0;
									xkilled(mon, 1);
								} else {
									pline("%s is flung back by the wind.",Monnam(mon));
									setmangry(mon);
								}
							}
						}
						if(isok(sx-u.dx,sy)){
							mon = m_at(sx-u.dx, sy);
							if(mon){
								mhurtle(mon, u.dx, u.dy, range+i);
								mon->mhp -= d(rnd(5),dsize);
								if (mon->mhp <= 0){
									mon->mhp = 0;
									xkilled(mon, 1);
								} else {
									pline("%s is flung back by the wind.",Monnam(mon));
									setmangry(mon);
								}
							}
						}
					}
					sx -= u.dx;
					sy -= u.dy;
				}
			}
		}break;
		case PWR_QUERIENT_THOUGHTS:{
			struct monst *mon, *nmon;
			You("release a burst of mental static.");
			for(mon=fmon; mon; mon = nmon) {
				nmon = mon->nmon;
				if (DEADMONSTER(mon)) continue;
				if (mon->mpeaceful) continue;
				if (mindless_mon(mon)) continue;
				if (mon_resistance(mon,TELEPAT) || !rn2(5)){
					mon->mhp -= d(5,15);
					if (mon->mhp <= 0) xkilled(mon, 1);
					else {
						setmangry(mon);
						mon->msleeping = 1;
						slept_monst(mon);
					}
				}
				else mon->msleeping = 0;
			}
		}break;
		case PWR_GREAT_LEAP:
			You("plunge through the ceiling!");
			morehungry(rnd(625));
			level_tele();
		break;
		case PWR_MASTER_OF_DOORWAYS:{
			//with apologies to Neil Gaiman
			struct monst *mon;
			if (!getdir((char *)0)) return(0);
			if(!(u.dx || u.dy || u.dz)){
				if(masterDoorBox(u.ux,u.uy)) break;
				//else
				pline("Maybe your innards should stay inside your body?");
				return 0;
			} else if(u.dz > 0) {
				if(!opentrapdoor(!Can_dig_down(&u.uz))) return 0;
				else break;
			} else if(u.dz < 0) {
				if(!openrocktrap()) return 0;
				else break;
			} else if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon){
					if(!opennewdoor(u.ux+u.dx, u.uy+u.dy) && !masterDoorBox(u.ux+u.dx,u.uy+u.dy)) return 0;
					else break;
				} 
				find_to_hit_rolls(mon, &tmp, &weptmp, &tchtmp);
				if((((uarmg && arti_shining(uarmg)) || u.sealsActive&SEAL_CHUPOCLOPS) ? tchtmp : tmp) <= rnd(20)){
					You("miss.");
					break;
				} else if(unsolid(mon->data)){
					pline("%s is not solid enough to open a door in.",Monnam(mon));
					shieldeff(mon->mx, mon->my);
					return FALSE;
				} else if (skeleton_innards(mon->data)) {
					pline("Skeletons don't really do \"inside\" vs \"outside\".");
					shieldeff(mon->mx, mon->my);
					break;
				} else if (removed_innards(mon->data)) {
					pline("%s organs have already been removed!", HisHerIts(mon));
					shieldeff(mon->mx, mon->my);
					break;
				} else if (no_innards(mon->data)) {
					pline("The inside of %s is much like the outside.",mon_nam(mon));
					shieldeff(mon->mx, mon->my);
					break;
				} else {
					if(!resist(mon, '\0', 0, TELL)){
						if(undiffed_innards(mon->data)){
							You("open a door into %s. Some of %s insides leak out.", mon_nam(mon), hisherits(mon));
							mon->mhp -= d(rnd(5),dsize);
						} else {
							if(rn2(10)){
								You("open a door into %s, exposing %s internal organs.", mon_nam(mon), hisherits(mon));
								mon->mhp -= d(rnd(5) * 5,dsize);
							} else {
								You("open a door into %s, releasing %s vital organs!", mon_nam(mon), hisherits(mon));
								mon->mhp = 0;
							}
						}
						if (mon->mhp <= 0){
							mon->mhp = 0;
							xkilled(mon, 1);
							break;
						}
						setmangry(mon);
					}
				}
			} else return 0;
		}break;
		case PWR_READ_SPELL:{
			if(uwep && uwep->oclass == SPBOOK_CLASS && !uwep->oartifact && 
				uwep->otyp != SPE_BLANK_PAPER && 
				uwep->otyp != SPE_SECRETS && 
				uwep->otyp != SPE_BOOK_OF_THE_DEAD
			){
				You("read from the spellbook in your hands.");
				// uwep->spestudied++;
				// costly_cancel(uwep);
				spelleffects(0,FALSE,uwep->otyp);
	    	    // if(uwep->spestudied > MAX_SPELL_STUDY){
					// pline("The magical energy within %s is exhausted.",the(xname(uwep)));
					// uwep->otyp = SPE_BLANK_PAPER;
				// }
			} else{
				You("need to be holding a spellbook.");
				return 0;
			}
		}break;
		case PWR_BOOK_TELEPATHY:
			book_detect(u.ulevel>13);
		break;
		case PWR_UNITE_THE_EARTH_AND_SKY:
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)){
				struct trap *t = t_at(u.ux+u.dx, u.uy+u.dy);
				struct monst *mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(t && (t->ttyp == PIT || t->ttyp == SPIKED_PIT)){
					pline("Water rains down from above and fills the pit.");
					mksobj_at(KELP_FROND,u.ux,u.uy,FALSE,FALSE);
					deltrap(t);
					levl[u.ux+u.dx][u.uy+u.dy].typ = POOL;
					newsym(u.ux+u.dx, u.uy+u.dy);
					if(mon) mon->mtrapped = 0;
				} else if(levl[u.ux+u.dx][u.uy+u.dy].typ == ROOM || levl[u.ux+u.dx][u.uy+u.dy].typ == CORR){
					pline("Water rains down from above, and a tree grows up from the ground.");
					levl[u.ux+u.dx][u.uy+u.dy].typ = TREE;
					newsym(u.ux+u.dx, u.uy+u.dy);
				} else if(levl[u.ux+u.dx][u.uy+u.dy].typ == TREE && 
							levl[u.ux+u.dx][u.uy+u.dy].looted & TREE_LOOTED
				){
					pline("Water rains down from above, and fruit grows on the tree before you.");
					levl[u.ux+u.dx][u.uy+u.dy].looted &= ~TREE_LOOTED;
					newsym(u.ux+u.dx, u.uy+u.dy);
				} else if(levl[u.ux+u.dx][u.uy+u.dy].typ == DEADTREE){
					pline("Water rains down from above, bringing new life to the dead tree.");
					levl[u.ux+u.dx][u.uy+u.dy].typ = TREE;
					newsym(u.ux+u.dx, u.uy+u.dy);
				}
			}
		break;
		case PWR_HOOK_IN_THE_SKY:
			if(!u.uevent.invoked){
				if((ledger_no(&u.uz) == 1 && u.uhave.amulet) ||
					Can_rise_up(u.ux, u.uy, &u.uz)) {
					const char *riseup ="are yanked up, through the %s!";
					if(ledger_no(&u.uz) == 1) {
						You(riseup, ceiling(u.ux,u.uy));
					goto_level(&earth_level, FALSE, FALSE, FALSE);
					} else {
						register int newlev = depth(&u.uz)-1;
						d_level newlevel;

						get_level(&newlevel, newlev);
						if(on_level(&newlevel, &u.uz)) {
							You("are yanked up, and hit your %s on the %s.",
								body_part(HEAD),
								ceiling(u.ux,u.uy));
							losehp(uarmh ? 1 : rnd(10),
								"colliding with the ceiling",
								KILLED_BY);
					break;
						} else You(riseup, ceiling(u.ux,u.uy));
						goto_level(&newlevel, FALSE, FALSE, FALSE);
					}
				} else You("have an uneasy feeling.");
			} else {
				You("are yanked up, and hit your %s on the %s.",
					body_part(HEAD),
					ceiling(u.ux,u.uy));
				losehp(uarmh ? 1 : rnd(10),
					"colliding with the ceiling",
					KILLED_BY);
				if(!Levitation) {
					HLevitation = 500;
					if(Levitation) float_up();
					HLevitation |= I_SPECIAL;
				} else You("have an uneasy feeling.");
			}
		break;
		case PWR_ENLIGHTENMENT:
			You_feel("self-knowledgeable...");
			display_nhwindow(WIN_MESSAGE, FALSE);
			enlightenment(0);
			pline_The("feeling subsides.");
		break;
		case PWR_DAMNING_DARKNESS:
			You("call on the wrath of shadow.");
			do_clear_area(u.ux, u.uy, dsize, damningdark, (genericptr_t) &dsize);
			vision_full_recalc = 1;	/* lighting changed */
			doredraw();
		break;
		case PWR_TOUCH_OF_THE_VOID:{
			struct monst *mon;
			if(!getdir((char *)0) || (!u.dx && !u.dy)) return 0;
			mon = m_at(u.ux+u.dx,u.uy+u.dy);
			if(!mon) return 0;
			if(!freehand()){
				You("need a free hand to make a touch attack!");
				return 0;
			}
			find_to_hit_rolls(mon, &tmp, &weptmp, &tchtmp);
			if(tchtmp <= rnd(20)){
				You("miss.");
				break;
			}
			if (resists_drli(mon) || resist(mon, '\0', 0, NOTELL)){
				pline("%s is unaffected by the touch of the void.", Monnam(mon));
				shieldeff(mon->mx, mon->my);
				break;
			} else {
				int dmg,lvls,tmp;
				lvls = rnd(5);
				tmp = rnd(dsize);
				lvls = min(lvls,tmp);
				dmg = d(lvls,8);
				mon->mhp -= dmg;
				mon->mhpmax -= dmg;
				if(lvls >= mon->m_lev) mon->mhp = 0; /* >_< Thanks bcode */
				else mon->m_lev -= lvls;
				
				if (mon->mhp <= 0 || mon->mhpmax <= 0) {
					xkilled(mon, 1);
				} else {
					if (canseemon(mon))
						pline("%s suddenly seems weaker!", Monnam(mon));
					setmangry(mon);
				}
			}
		}break;
		case PWR_ECHOS_OF_THE_LAST_WORD:{
			struct monst *mon;
			if(!getdir((char *)0) || (!u.dx && !u.dy)) return 0;
			mon = m_at(u.ux+u.dx,u.uy+u.dy);
			if(!mon) return 0;
			You("speak an echo of the Last Word of creation.");
			if(mon->data == &mons[PM_DREAD_SERAPH] || mon->data == &mons[PM_BLACK_FLOWER]){
				pline("Its voice harmonizes with your own!");
				unbind(SEAL_TENEBROUS, TRUE);
			} else if(mon->data == &mons[PM_INTONER]){
				pline("%s screams!", SheHeIt(mon));
				newcham(mon, &mons[PM_BLACK_FLOWER], FALSE, FALSE);
			} else if(resists_drli(mon) || !(mon->data->geno & G_GENO) || resists_death(mon)){
				int nlev;
				d_level tolevel;
				int migrate_typ = MIGR_RANDOM;
				if(mon->data->geno & G_UNIQ || mon_has_amulet(mon)){
					if (mon_has_amulet(mon) || In_endgame(&u.uz)) {
						if (canspotmon(mon))
						pline("%s flickers for a moment.",
							Monnam(mon));
						return 0;
					}
					nlev = random_teleport_level();
					if (nlev == depth(&u.uz)) {
						if (canspotmon(mon))
							pline("%s flickers for a moment.", Monnam(mon));
						return 0;
					}
					get_level(&tolevel, nlev);
					if (canspotmon(mon)) {
						pline("Suddenly, %s disappears out of sight.", mon_nam(mon));
					}
					migrate_to_level(mon, ledger_no(&tolevel),
							 migrate_typ, (coord *)0);
				} else {
					pline("%s collapses in on %sself.", Monnam(mon), himherit(mon));
					mongone(mon);
				}
			} else pline("%s looks briefly unsettled.", Monnam(mon));
		}break;
		case PWR_POISON_GAZE:{
			coord cc;
			int cancelled;
			struct monst *mon;
		    cc.x = u.ux;
		    cc.y = u.uy;
		    pline("At what monster do you wish to gaze?");
			do cancelled = getpos(&cc, TRUE, "the monster to gaze at");
			while( !((mon=m_at(cc.x,cc.y))  && canspotmon(mon)) && cancelled >= 0);
			if(cancelled < 0) return 0; /*abort*/
			if(!mon || !canseemon(mon)){
				You("don't see a monster there.");
				break;
			}
			if(resists_poison(mon)){
				pline("%s is immune to the poison.", Monnam(mon));
				shieldeff(mon->mx, mon->my);
			} else if(rn2(10)){
				mon->mhp = 0;
				xkilled(mon, 1);
				break;
			} else {
				mon->mhp -= d(5,dsize);
				if(mon->mhp <= 0){
					mon->mhp = 0;
					xkilled(mon, 1);
					break;
				} else You("shoot a venomous glare at %s.", mon_nam(mon));
				setmangry(mon);
			}
		}break;
		case PWR_GAP_STEP:
			if(!Levitation) {
				int i;
				pline("\"There was, in times of old,");
				pline("when Ymir lived,");
				pline("but a yawning gap, and grass nowhere.\"");
				for(i=0; i < GATE_SPIRITS; i++) if(u.spirit[i] == SEAL_YMIR) break;
				if(i<GATE_SPIRITS) HLevitation = u.spiritT[i] - moves + 5; //Remaining time until binding expires, plus safety margin
				else HLevitation = u.voidChime + 5; //Remaining time until chime expires (apparently, it was in the Pen), plus safety margin
				if(Levitation) float_up();
				HLevitation |= I_SPECIAL;
			} else {
				You("are already levitating.");
				return 0;
			}
		break;
		case PWR_MOAN:{
			struct monst *mon;
			You("let out a frightful moan.");
			for(mon = fmon; mon; mon = mon->nmon){
				if(!DEADMONSTER(mon) && dist2(u.ux,u.uy,mon->mx,mon->my)<=u.ulevel && 
					!mindless_mon(mon) && !resist(mon, '\0', 0, TELL)
				) {
					if(u.ulevel >= mon->m_lev-5){
						mon->mconf = 1;
						if(u.ulevel >= mon->m_lev+5){
							if(canseemon(mon)) pline("%s goes insane.", Monnam(mon));
							mon->mcrazed = 1;
						} else if(canseemon(mon)) pline("%s looks dizzy.", Monnam(mon));
					}
					monflee(mon, d(5,dsize), FALSE, TRUE);
				}
			}
		}break;
		case PWR_SWALLOW_SOUL:{
			struct monst *mon;
			if(!getdir((char *)0) || (!u.dx && !u.dy)) return 0;
			mon = m_at(u.ux+u.dx,u.uy+u.dy);
			if(!mon) return 0;
			if(resists_drli(mon) || nonliving_mon(mon) || mon->m_lev > u.ulevel){
				pline("You can't swallow the soul of %s.", mon_nam(mon));
				shieldeff(mon->mx, mon->my);
			} else {
				You("suck out %s's soul.", mon_nam(mon));
				mon->mhp = 0;
				xkilled(mon,1);
				if(u.ulevelmax > u.ulevel) pluslvl(FALSE);
				else healup(d(mon->m_lev,4), 0, FALSE, FALSE);
			}
		}break;
		case PWR_EMBASSY_OF_ELEMENTS:{
		    int spirit_id = pick_council_seal();
			if(spirit_id) councilspirit(spirit_id);
			else return 0;
			You("re-contact %s.", sealNames[spirit_id-FIRST_SEAL]);
		}break;
		case PWR_SUMMON_MONSTER:{
			struct monst *mon;
			struct permonst *pm;
			pm = choose_crystal_summon();
			if(pm && (mon = makemon(pm, u.ux, u.uy, MM_EDOG|MM_ADJACENTOK|MM_NOCOUNTBIRTH|NO_MINVENT))){
				initedog(mon);
				mon->mvanishes = 10+u.ulevel/2;
			} else return 0;
		}break;
		case PWR_PSEUDONATURAL_SURGE:
			pline("The dustlight seethes around you as tentacles erupt from your body!");
			nomul(-5, "channeling a pseudonatural.");
			dopseudonatural();
		break;
		case PWR_SILVER_DEW:{
			int dmg;
			struct monst *mon;
			if (!getdir((char *)0)  || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon) break;
				dmg = d(5,dsize);
				if(mon->mpeaceful){
					if(canseemon(mon))
						pline("%s looks rested and healthy!", Monnam(mon));
					mon->msleeping = 0;
					mon->mhp += dmg;
					mon->mhp = min(mon->mhp, mon->mhpmax);
				} else {
					if(hates_silver(mon->data)){
						pline("The silver dew sears %s!", mon_nam(mon));
						dmg += d(5,20); //Ouch (Melee-ranged attack that takes your whole turn)
					}
					else {
						pline("%s is laden with dew!", Monnam(mon));
						if (amphibious_mon(mon) &&
							!flaming(mon->data)
						){
							dmg = 0;
							pline("%s seems unharmed.", Monnam(mon));
						}
					}
					mon->mhp -= dmg;
					if (mon->mhp <= 0){
						mon->mhp = 0;
						xkilled(mon, 1);
						break;
					}
				}
			} else break;
		}break;
		case PWR_GOLDEN_DEW:{
			int dmg;
			struct monst *mon;
			if (!getdir((char *)0)  || !(u.dx || u.dy)) return(0);
			if(isok(u.ux+u.dx, u.uy+u.dy)) {
				mon = m_at(u.ux+u.dx, u.uy+u.dy);
				if(!mon) break;
				dmg = d(5,dsize);
				if(mon->mpeaceful){
					if(canseemon(mon))
						pline("%s looks full of righteous wrath!", Monnam(mon));
					if(mon->encouraged < BASE_DOG_ENCOURAGED_MAX || mon->encouraged < dmg){
						mon->encouraged += dmg;
						mon->encouraged = min(mon->encouraged, max(dmg, BASE_DOG_ENCOURAGED_MAX));
					}
				} else {
					if(hates_holy_mon(mon)){
						pline("The golden dew sears %s!", mon_nam(mon));
						dmg += d(5,20); //Ouch (Melee-ranged attack that takes your whole turn)
					}
					else {
						pline("%s is laden with dew!", Monnam(mon));
						if (amphibious_mon(mon) &&
							!flaming(mon->data)
						){
							dmg = 0;
							pline("%s seems unharmed.", Monnam(mon));
						}
					}
					mon->mhp -= dmg;
					if (mon->mhp <= 0){
						mon->mhp = 0;
						xkilled(mon, 1);
						break;
					}
				}
			} else break;
		}break;
		case PWR_MIRROR_SHATTER:{
			int dmg;
			boolean done = FALSE;
			struct obj *obj;
			struct obj *umirror;
			xchar lsx, lsy, sx, sy;
			struct monst *mon;
			sx = u.ux;
			sy = u.uy;
			if(u.uswallow){
				You("can't do that in here!");
				return 0;
			}
			for(umirror = invent; umirror; umirror = umirror->nobj)
				if(umirror->otyp == MIRROR && !(umirror->oartifact)) break;
			if(!(umirror)){
				You("must have a breakable mirror in inventory to use this power!");
				return 0;
			}
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			while(!done){
				sx += u.dx;
				sy += u.dy;
				if(!isok(sx,sy)) return 0;
				else {
					for (obj = level.objects[sx][sy]; obj; obj = obj->nexthere) {
						if(obj->otyp == MIRROR){
							useupf(obj, 1);
							done = TRUE;
							break;
						}
					}
					mon = m_at(sx, sy);
					if(!done && mon){
						for (obj = mon->minvent; obj; obj = obj->nobj) {
							if(obj->otyp == MIRROR && !(obj->oartifact)){
								m_useup(mon, obj);
								done = TRUE;
								break;
							}
						}
					}
				}
			}
			if(!done) return 0;
			//else
			useup(umirror);
			if(u.sealsActive&SEAL_ASTAROTH) unbind(SEAL_ASTAROTH,TRUE);
			explode(u.ux,u.uy, AD_PHYS, TOOL_CLASS, d(5,dsize), HI_SILVER, 1);
			explode(sx,sy, AD_PHYS, TOOL_CLASS, d(5,dsize), HI_SILVER, 1);
			
			while(sx != u.ux && sy != u.uy){
				sx -= u.dx;
				sy -= u.dy;
				if(!isok(sx,sy)) break; //shouldn't need this, but....
				else {
					mon = m_at(sx, sy);
					/* reveal/unreveal invisible monsters before tmp_at() */
					if (mon && !canspotmon(mon) && cansee(sx,sy))
						map_invisible(sx, sy);
					else if (!mon && glyph_is_invisible(levl[sx][sy].glyph)) {
						unmap_object(sx, sy);
						newsym(sx, sy);
					}
					if (mon) {
						reveal_invis = TRUE;
						if (resists_magm(mon)) {	/* match effect on player */
							shieldeff(mon->mx, mon->my);
						} else {
							dmg = d(5,dsize);
							if(hates_silver(mon->data)){
								dmg += rnd(20);
								pline("The flying shards of mirror sear %s!", mon_nam(mon));
							} else {
								pline("The flying shards of mirror hit %s.", mon_nam(mon));
								u_teleport_mon(mon, TRUE);
							}
							mon->mhp -= dmg;
							if (mon->mhp <= 0){
								xkilled(mon, 1);
							}
						}
					}
				}
			}
		}break;
		case PWR_FLOWING_FORMS:{
			struct monst *mon;
			if(!getdir((char *)0) || u.dz)
				return 0;
			else if((!u.dx && !u.dy)) {
				You_feel("a little %s.", Hallucination ? "normal" : "strange");
				if (!Unchanging) polyself(FALSE);
				break;
			}
			mon = m_at(u.ux+u.dx,u.uy+u.dy);
			if(!mon) return 0;
			if (resist(mon, '\0', 0, NOTELL) || resists_poly(mon->data)){
				shieldeff(mon->mx, mon->my);
				break;
			} else {
				struct permonst *mdat = 0;
				int mndx, tryct=0;
				while (++tryct <= 100) {
				mndx = rn2(PM_LONG_WORM_TAIL); /* Old polymorph method */
				mdat = &mons[mndx];
				if ((mvitals[mndx].mvflags & G_GENOD && !In_quest(&u.uz)) != 0 ||
					is_placeholder(mdat)) continue;
				/* polyok rules out all MG_PNAME and MA_WERE's;
				   select_newcham_form might deliberately pick a player
				   character type, so we can't arbitrarily rule out all
				   human forms any more */
				if (is_mplayer(mdat) || (!is_human(mdat) && polyok(mdat)))
					break;
				}
				if (tryct > 100) return 0;	/* Should never happen */
				newcham(mon, mdat, FALSE, FALSE);
			}
		}break;
		case PWR_PHASE_STEP:{
			if(!Passes_walls){
				if (!Hallucination) {    
					Your("body begins to feel less solid.");
				} else {
					You_feel("one with the spirit world.");
				}
			}
			incr_itimeout(&Phasing, 5);
		}break;
		case PWR_BLACK_BOLT:{
			struct obj *qvr;
			xchar lsx, lsy, sx, sy;
			struct monst *mon = (struct monst *)0;
			struct trap *ttmp2;
			sx = u.ux;
			sy = u.uy;
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			// if(u.uswallow); //Phase through the surounding monster.
			while(TRUE){ //Exits via break, phase through walls.
				sx += u.dx;
				sy += u.dy;
				if(isok(sx,sy) && may_passwall(sx,sy)) {
					mon = m_at(sx, sy);
					/* reveal/unreveal invisible monsters before tmp_at() */
					if (mon && !canspotmon(mon) && cansee(sx,sy))
						map_invisible(sx, sy);
					else if (!mon && glyph_is_invisible(levl[sx][sy].glyph)) {
						unmap_object(sx, sy);
						newsym(sx, sy);
					}
					
					if(mon) break;
				} else break;
			}
			if(!mon) return 0;
			qvr = mksobj(SPIKE, TRUE, FALSE);
			qvr->blessed = 0;
			qvr->cursed = 0;
			qvr->quan = 1;
			qvr->spe = d(5,dsize) + 8;
			qvr->obj_material = SHADOWSTEEL;
			qvr->opoisoned = (OPOISON_BASIC|OPOISON_BLIND);
			set_bypassDR(1);  //state variable referenced in drop_throw
			set_destroy_thrown(1); //state variable referenced in drop_throw
				m_throw(&youmonst, mon->mx + (-u.dx), mon->my + (-u.dy), u.dx, u.dy,
					1, qvr,TRUE);
				if(!DEADMONSTER(mon) && mon_can_see_you(mon)) // and mon_can_see_you(mon)?
					setmangry(mon);
				ttmp2 = maketrap(mon->mx, mon->my, WEB);
				if (ttmp2 && !DEADMONSTER(mon)) mintrap(mon);
			set_destroy_thrown(0);  //state variable referenced in drop_throw
			set_bypassDR(0);  //state variable referenced in drop_throw
		}break;
		case PWR_WEAVE_BLACK_WEB:{
		    pline("The poison shadow of the Black Web flows in your wake.");
			static struct attack webattack[] = 
			{
				{AT_SHDW,AD_SHDW,4,8},
				{0,0,0,0}
			};
			struct monst *mon;
			int i, tmp, weptmp, tchtmp;
			for(i=0; i<8;i++){
				if(isok(u.ux+xdir[i],u.uy+ydir[i])){
					mon = m_at(u.ux+xdir[i],u.uy+ydir[i]);
					if(mon && !mon->mpeaceful){
						find_to_hit_rolls(mon,&tmp,&weptmp,&tchtmp);
						hmonwith(mon, tmp, weptmp, tchtmp, webattack, 1);
					}
				}
			}
		} break;
		case PWR_IDENTIFY_INVENTORY:
		    identify_pack(0);
		break;
		case PWR_CLAIRVOYANCE:
			do_vicinity_map(u.ux,u.uy);
		break;
		case PWR_FIND_PATH:{
			struct trap *ttmp;
			int du = 255; /*Arbitrary value larger than 144*/
			You("try to find nearby paths.");
			for(ttmp = ftrap; ttmp; ttmp = ttmp->ntrap) {
				if(ttmp->ttyp == MAGIC_PORTAL) {/*may be more than one portal on some levels*/
					du = min(du, distu(ttmp->tx, ttmp->ty));
				}
				if (du <= 9)
				You_feel("%s path under your feet!", In_endgame(&u.uz) ? "the":"a");
				else if (du <= 64)
				You_feel("%s path nearby.", In_endgame(&u.uz) ? "the":"a");
				else if (du <= 144)
				You_feel("%s path in the distance.", In_endgame(&u.uz) ? "the":"a");
			}
		}break;
		case PWR_GNOSIS_PREMONITION:{
		    int spirit_id = pick_gnosis_seal();
			if(spirit_id) gnosisspirit(spirit_id);
			else return 0;
			You("dream of %s...", sealNames[spirit_id-FIRST_SEAL]);
		}break;
		default:
			pline("BANG! That's not going to kill TOO many people, is it...? (Unknown power %d.)", power);
			//Note: Eddie Izzard joke
		break;
	}
	u.spiritPColdowns[power] = monstermoves + 25;
	return 1;
}

struct permonst *
choose_crystal_summon()
{
	winid tmpwin;
	int i, n, how;
	char buf[BUFSZ];
	char incntlet = 'a';
	long seal_flag = 0x1L;
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */
	
	Sprintf(buf,	"Pick creature to summon:");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	
	for(i = 0; i < NUMMONS; i++){
		if( (mons[i].geno&G_UNIQ && mvitals[i].died >= 1 && i != PM_CREATURE_IN_THE_ICE)){
			Sprintf(buf, "%s", mons[i].mname);
			any.a_int = i;	/* must be non-zero */
			add_menu(tmpwin, NO_GLYPH, &any,
				incntlet, 0, ATR_NONE, buf,
				MENU_UNSELECTED);
			incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
		}
	}
	end_menu(tmpwin,	"Choose summon:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return ( n > 0 ) ? &mons[selected[0].item.a_int] : (struct permonst *) 0;
}


void
blessedlight(x,y)
int x, y;
{
	int nd;
	int dmod = 1;
	nd = max(u.ulevel/2, 1);
	struct monst *mon=m_at(x,y);
	if (!levl[x][y].lit){
	    levl[x][y].lit = 1;
		newsym(x,y);
	}
	if(mon && !mon->mpeaceful){
		if(is_undead_mon(mon))
			dmod++;
		if(is_demon(mon->data))
			dmod++;
		if(is_primordial(mon->data))
			dmod++;
		if(dmgtype(mon->data, AD_DRLI) || dmgtype(mon->data, AD_VAMP))
			dmod++;
		if(dmgtype(mon->data, AD_DISN) && !dmgtype(mon->data, AD_ACFR))
			dmod++;
		if(mon->data->mlet == S_SHADE 
		|| mon->data->mlet == S_WRAITH
		|| mon->data->mlet == S_UMBER
		)
			dmod++;
		mon->mhp -= d(nd, 3*dmod);
		pline("%s is seared by the Light.", Monnam(mon));
	}
	if (mon && mon->mhp <= 0){
		mon->mhp = 0;
		xkilled(mon, 1);
	}
}

int
wordeffects(spell)
int spell;
{
	int sx = u.ux, sy = u.uy, nd = max(u.ulevel/2, 1);
	struct monst *mon;
	switch(spell){
		case FIRST_LIGHT:
			for(sy = 0; sy < ROWNO; sy++){
				for(sx = 0; sx < COLNO; sx++){
					if(isok(sx,sy) && couldsee(sx,sy)) blessedlight(sx, sy);
				}
			}
			vision_full_recalc = 1;	/* lighting changed */
			if(u.sealsActive&SEAL_TENEBROUS) unbind(SEAL_TENEBROUS,TRUE); /* First Word vs Last Word */
			doredraw();
			u.ufirst_light_timeout = moves + (long)(rnz(100)*(Role_if(PM_PRIEST) ? .8 : 1));
		break;
		case PART_WATER:{
			boolean parted = FALSE;
			struct trap *ttmp;
			if (!getdir((char *)0) || !(u.dx || u.dy)) return(0);
			if(u.uswallow){
				mon = u.ustuck;
				pline("%s splits in half!",Monnam(mon));
				expels(mon, u.ustuck->data, TRUE);
				mon->mhp = 0;
				xkilled(mon, 1);
			}
			sx += u.dx, sy += u.dy;
			while(isok(sx,sy) && ZAP_POS(levl[sx][sy].typ)){
				if(levl[sx][sy].typ == POOL 
				|| levl[sx][sy].typ == MOAT 
				|| levl[sx][sy].typ == PUDDLE
				){
					if((levl[sx][sy].typ == POOL
						|| levl[sx][sy].typ == MOAT)
					&& !t_at(sx, sy)
					){
						levl[sx][sy].typ = ROOM;
						ttmp = maketrap(sx, sy, PIT);
						ttmp->tseen = 1;
					} else levl[sx][sy].typ = ROOM;
					if(!parted)
						pline("The waters part.");
					parted = TRUE;
					newsym(sx,sy);
				}
				mon = m_at(sx, sy);
				if(mon && !mon->mpeaceful){
					if(!rn2(20)){
						pline("%s is bisected!", Monnam(mon));
						mon->mhp = 0;
						xkilled(mon, 1);
					} else if(rn2(2)){
						pline("%s is thrown to the side.", Monnam(mon));
						mon->mhp -= d(nd,3);
						if (mon->mhp <= 0){
							mon->mhp = 0;
							xkilled(mon, 1);
						} else mhurtle(mon, -1*u.dy, u.dx, 33);
					} else {
						pline("%s is thrown to the side.", Monnam(mon));
						mon->mhp -= d(nd,3);
						if (mon->mhp <= 0){
							mon->mhp = 0;
							xkilled(mon, 1);
						} else mhurtle(mon, u.dy, -1*u.dx, 33);
					}
				}
				//Update
				sx += u.dx, sy += u.dy;
			}
			u.ufirst_sky_timeout = moves + (long)(rnz(100)*(Role_if(PM_PRIEST) ? .8 : 1));
		}break;
		case OVERGROW:
			for(sy = 0; sy < ROWNO; sy++){
				for(sx = 0; sx < COLNO; sx++){
					if(isok(sx,sy) && couldsee(sx,sy)){
						mon = m_at(sx, sy);
						if(levl[sx][sy].typ == ROOM || levl[sx][sy].typ == SOIL){
							levl[sx][sy].typ = GRASS;
							newsym(sx,sy);
						}
						if(mon && !mon->mpeaceful){
							if(is_elemental(mon->data) 
								|| is_undead_mon(mon) 
								|| mon->data == &mons[PM_ASPECT_OF_THE_SILENCE] 
								|| mon->data == &mons[PM_SENTINEL_OF_MITHARDIR] 
								|| mon->data == &mons[PM_STONE_GOLEM] 
								|| mon->data == &mons[PM_CLAY_GOLEM] 
								|| mon->data == &mons[PM_FLESH_GOLEM]
							) {
								mon->mhp -= d(nd,12);
								if (mon->mhp <= 0){
									mon->mhp = 0;
									xkilled(mon, 1);
									if(levl[sx][sy].typ == ROOM 
									|| levl[sx][sy].typ == SOIL
									|| levl[sx][sy].typ == GRASS
									|| levl[sx][sy].typ == PUDDLE
									|| levl[sx][sy].typ == CORR
									){
										levl[sx][sy].typ = TREE;
										bury_objs(sx, sy);
										newsym(sx,sy);
									}
								} else {
									mon->movement -= 12;//Vines etc.
								}
							}
						}
						if(levl[sx][sy].typ == TREE)
							levl[sx][sy].looted &= ~TREE_LOOTED;
					}
				}
			}
			for(sy = 0; sy < ROWNO; sy++){
				for(sx = 0; sx < COLNO; sx++){
						if(levl[sx][sy].typ == TREE)
							block_point(sx,sy);
				}
			}
			vision_full_recalc = 1;	/* trees may have sprouted */
			doredraw();
			u.ufirst_life_timeout = moves + (long)(rnz(100)*(Role_if(PM_PRIEST) ? .8 : 1));
		break;
		default:
			pline("Unknown word of power!");
			return 0;
		break;
	}
	{
	int res;
	//Speak one word of power per move free.
	return partial_action();
	}
}
int
spelleffects(spell, atme, spelltyp)
int spell, spelltyp;
boolean atme;
{
	int energy, damage, chance, n, intell;
	int skill, role_skill;
	boolean confused = (Confusion != 0);
	struct obj *pseudo;
	coord cc;
	int color = 0;
	int inacc = 0;
	boolean miss = FALSE;
	int dam = 0;
	int rad = 0;
	
	if(!spelltyp){
		/*
		 * Spell casting no longer affects knowledge of the spell. A
		 * decrement of spell knowledge is done every turn.
		 */
		if (spellknow(spell) <= 0) {
			Your("knowledge of this spell is twisted.");
			pline("It invokes nightmarish images in your mind...");
			spell_backfire(spell);
			return(0);
		} else if (
			!(spellid(spell) == SPE_LIGHTNING_BOLT && uarmh && uarmh->oartifact == ART_STORMHELM) &&
			!((spellid(spell) == SPE_FORCE_BOLT || spellid(spell) == SPE_MAGIC_MISSILE) && 
				uwep && uwep->oartifact == ART_ANNULUS && uwep->otyp == CHAKRAM)
		) {
			if(spellknow(spell) <= 200) { /* 1% */
				You("strain to recall the spell.");
			} else if (spellknow(spell) <= 1000 && 
				!(spellid(spell) == SPE_LIGHTNING_BOLT || !uarmh || uarmh->oartifact != ART_STORMHELM)
			) { /* 5% */
				Your("knowledge of this spell is growing faint.");
			}
		}
		energy = (spellev(spell) * 5);    /* 5 <= energy <= 35 */

		if (!Race_if(PM_INCANTIFIER) && u.uhunger <= 10 && spellid(spell) != SPE_DETECT_FOOD) {
			You("are too hungry to cast that spell.");
			return(0);
		} else if (ACURR(A_STR) < 4)  {
			You("lack the strength to cast spells.");
			return(0);
		} else if(check_capacity(
			"Your concentration falters while carrying so much stuff.")) {
			return (1);
		} else if (!freehand()) {
			Your("arms are not free to cast!");
			return (0);
		}

		if (u.uhave.amulet) {
			You_feel("the amulet draining your energy away.");
			energy += rnd(2*energy);
		}
		if(energy > u.uen)  {
			You("don't have enough energy to cast that spell (need %d).", energy);
			return(0);
		} else {
			if (spellid(spell) != SPE_DETECT_FOOD && !(Race_if(PM_INCANTIFIER)) ) {
				int hungr = spellhunger(energy);
				/* don't put player (quite) into fainting from
				 * casting a spell, particularly since they might
				 * not even be hungry at the beginning; however,
				 * this is low enough that they must eat before
				 * casting anything else except detect food
				 */
				if (hungr > YouHunger-3)
					hungr = YouHunger-3;
				morehungry(hungr);
			}
		}

		chance = percent_success(spell);
		if (confused || (rnd(100) > chance)) {
			You("fail to cast the spell correctly.");
			u.uen -= energy / 2;
			flags.botl = 1;
			return(1);
		}

		u.uen -= energy;
		flags.botl = 1;
		exercise(A_WIS, TRUE);
		/* pseudo is a temporary "false" object containing the spell stats */
		pseudo = mksobj(spellid(spell), FALSE, FALSE);
		pseudo->blessed = pseudo->cursed = 0;
		pseudo->quan = 20L;			/* do not let useup get it */
		
		if(uwep && uwep->oartifact == ART_STAFF_OF_TWELVE_MIRRORS){
			wake_nearto_noisy(u.ux, u.uy, energy*energy*2);
		}
		
	} else {
		pseudo = mksobj(spelltyp, FALSE, FALSE);
		pseudo->blessed = pseudo->cursed = 0;
		pseudo->quan = 20L;			/* do not let useup get it */
		
		//book-casting doesn't use the staff of twelve mirrors
	}
	/*
	 * Find the skill the hero has in a spell type category.
	 * See spell_skilltype for categories.
	 */
	skill = spell_skilltype(pseudo->otyp);
	role_skill = P_SKILL(skill);
	if(Spellboost) role_skill++;
	
	n = 0;
	switch(pseudo->otyp)  {
	/*
	 * At first spells act as expected.  As the hero increases in skill
	 * with the appropriate spell type, some spells increase in their
	 * effects, e.g. more damage, further distance, and so on, without
	 * additional cost to the spellcaster.
	 */
	case SPE_LIGHTNING_STORM:	color = EXPL_MAGICAL;
								n = rnd(6) + 6;
								dam = u.ulevel + spell_damage_bonus();
								rad = 0;
								inacc = 3;
								goto dothrowspell;
	case SPE_BLIZZARD:			color = EXPL_FROSTY;
								n = rnd(3) + 1;
								dam = u.ulevel + spell_damage_bonus();
								rad = 1;
								inacc = 1;
								goto dothrowspell;
	case SPE_FIRE_STORM:		color = EXPL_FIERY;
								n = 1;
								dam = rnd(u.ulevel) + rnd(u.ulevel) + u.ulevel + spell_damage_bonus();
								rad = 2;
								inacc = 1;
								goto dothrowspell;
dothrowspell:
		if (Double_spell_size){
			n = n * 3 / 2;
			if (rad)
				rad += 1;
			else
				n += 2;
		}

		if (throwspell()) {
			cc.x = u.dx; cc.y = u.dy;
			while (n--) {
				// aim
				miss = FALSE;
				if (u.uswallow){
					u.dx = cc.x;
					u.dy = cc.y;
				}
				else
				{
					boolean once = TRUE;
					while (once || !isok(u.dx, u.dy) || !cansee(u.dx, u.dy) || IS_STWALL(levl[u.dx][u.dy].typ)) {
						if (pseudo->otyp == SPE_LIGHTNING_STORM && !rn2(10))
							miss = TRUE;	//lightning storm is more accurate out in the open
						u.dx = cc.x + rnd(1 + inacc * 2) - inacc - 1; u.dy = cc.y + rnd(1 + inacc * 2) - inacc - 1;
						once = FALSE;
					}
				}
				if (pseudo->otyp == SPE_LIGHTNING_STORM && !miss && !(m_at(u.dx, u.dy) || (u.dx == u.ux && u.dy == u.uy && !(uarmh && uarmh->oartifact == ART_STORMHELM))) && rn2(250))
				{ //lightning storm prefers to hit creatures (including you)
					n++;
				}
				else
				{
					// fire
					if (!u.dx && !u.dy && !u.dz) {
						if ((damage = zapyourself(pseudo, TRUE)) != 0) {
							char buf[BUFSZ];
							Sprintf(buf, "zapped %sself with a spell", uhim());
							losehp(damage, buf, NO_KILLER_PREFIX);
						}
					}
					else {
						explode(u.dx, u.dy, spell_adtype(pseudo->otyp), 0, dam, color, rad);
					}
				}
			}
		}
		break;
	/* these spells are handled in zap.c */
	case SPE_LIGHTNING_BOLT:
	case SPE_CONE_OF_COLD:
	case SPE_FIREBALL:
	case SPE_ACID_SPLASH:
	case SPE_HASTE_SELF:
	case SPE_FORCE_BOLT:
	case SPE_SLEEP:
	case SPE_MAGIC_MISSILE:
	case SPE_KNOCK:
	case SPE_SLOW_MONSTER:
	case SPE_WIZARD_LOCK:
	case SPE_DIG:
	case SPE_TURN_UNDEAD:
	case SPE_POLYMORPH:
	case SPE_TELEPORT_AWAY:
	case SPE_CANCELLATION:
	case SPE_FINGER_OF_DEATH:
	case SPE_POISON_SPRAY:
	case SPE_LIGHT:
	case SPE_DETECT_UNSEEN:
	case SPE_HEALING:
	case SPE_EXTRA_HEALING:
	case SPE_DRAIN_LIFE:
	case SPE_STONE_TO_FLESH:
		if (!(objects[pseudo->otyp].oc_dir == NODIR)) {
			if (atme) u.dx = u.dy = u.dz = 0;
			else if (!getdir((char *)0)) { //Oh, getdir must set the .d_ variables below.
			    /* getdir cancelled, re-use previous direction */
			    pline_The("magical energy is released!");
			}
			if(!u.dx && !u.dy && !u.dz) {
			    if ((damage = zapyourself(pseudo, TRUE)) != 0) {
				char buf[BUFSZ];
				Sprintf(buf, "zapped %sself with a spell", uhim());
				losehp(damage, buf, NO_KILLER_PREFIX);
			    }
			} else weffects(pseudo);
		} else{
			weffects(pseudo);
		}
		update_inventory();	/* spell may modify inventory */
		break;

	/* these are all duplicates of scroll effects */
	case SPE_REMOVE_CURSE:
	case SPE_CONFUSE_MONSTER:
	case SPE_DETECT_FOOD:
	case SPE_CAUSE_FEAR:
		/* high skill yields effect equivalent to blessed scroll */
		if (role_skill >= P_SKILLED) pseudo->blessed = 1;
		/* fall through */
	case SPE_PACIFY_MONSTER:
	case SPE_CHARM_MONSTER:
	case SPE_MAGIC_MAPPING:
	case SPE_CREATE_MONSTER:
	case SPE_IDENTIFY:
	case SPE_ANTIMAGIC_SHIELD:
		(void) seffects(pseudo);
		break;

	/* these are all duplicates of potion effects */
	case SPE_DETECT_TREASURE:
	case SPE_DETECT_MONSTERS:
	case SPE_LEVITATION:
	case SPE_RESTORE_ABILITY:
		/* high skill yields effect equivalent to blessed potion */
		if (role_skill >= P_SKILLED) pseudo->blessed = 1;
		/* fall through */
	case SPE_INVISIBILITY:
		(void) peffects(pseudo);
		break;

	case SPE_CURE_BLINDNESS:
		healup(0, 0, FALSE, TRUE);
		break;
	case SPE_CURE_SICKNESS:
		if (Sick) You("are no longer ill.");
		if (Slimed) {
		    pline_The("slime disappears!");
		    Slimed = 0;
		 /* flags.botl = 1; -- healup() handles this */
		}
		healup(0, 0, TRUE, FALSE);
		break;
	case SPE_CREATE_FAMILIAR:
		(void) make_familiar((struct obj *)0, u.ux, u.uy, FALSE);
		break;
	case SPE_CLAIRVOYANCE:
		if (!BClairvoyant) {
		    if (role_skill >= P_SKILLED) {
			coord cc;
			pline(where_to_cast);
			cc.x = u.ux;
			cc.y = u.uy;
			if (getpos(&cc, TRUE, "the desired position") >= 0)
		    	    do_vicinity_map(cc.x,cc.y);
		    } else do_vicinity_map(u.ux,u.uy);
		/* at present, only one thing blocks clairvoyance */
		} else if (uarmh && uarmh->otyp == CORNUTHAUM)
		    You("sense a pointy hat on top of your %s.",
			body_part(HEAD));
		break;
	case SPE_PROTECTION:
		cast_protection();
		break;
	case SPE_JUMPING:
		if (!jump(max(role_skill,1)))
			pline1(nothing_happens);
		break;
	default:
		impossible("Unknown spell %d attempted.", spell);
		obfree(pseudo, (struct obj *)0);
		return(0);
	}
	
	/* gain skill for successful cast */
	use_skill(skill, spellev(spell));
	u.lastcast = monstermoves + spellev(spell);
	if(uwep && uwep->oartifact == ART_INFINITY_S_MIRRORED_ARC && uwep->spe > 0)
		u.lastcast += uwep->spe;

	obfree(pseudo, (struct obj *)0);	/* now, get rid of it */
	return(1);
}

/* Choose location where spell takes effect. */
STATIC_OVL int
throwspell()
{
	coord cc;

	if (u.uinwater) {
	    pline("You're joking! In this weather?"); return 0;
	} else if (Is_waterlevel(&u.uz)) {
	    You("had better wait for the sun to come out."); return 0;
	}

	pline(where_to_cast);
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "the desired position") < 0)
	    return 0;	/* user pressed ESC */
	/* The number of moves from hero to where the spell drops.*/
	if (distmin(u.ux, u.uy, cc.x, cc.y) > 10) {
	    pline_The("spell dissipates over the distance!");
	    return 0;
	} else if (u.uswallow) {
	    pline_The("spell is cut short!");
	    exercise(A_WIS, FALSE); /* What were you THINKING! */
	    u.dx = 0;
	    u.dy = 0;
	    return 1;
	} else if (!cansee(cc.x, cc.y) || IS_STWALL(levl[cc.x][cc.y].typ)) {
	    Your("mind fails to lock onto that location!");
	    return 0;
	} else {
	    u.dx=cc.x;
	    u.dy=cc.y;
	    return 1;
	}
}

/* Choose location where spell takes effect. */
STATIC_OVL int
throwgaze()
{
	coord cc;

	pline(where_to_gaze);
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, TRUE, "the desired position") < 0)
	    return 0;	/* user pressed ESC */
	/* The number of moves from hero to where the spell drops.*/
	if (distmin(u.ux, u.uy, cc.x, cc.y) > 10) {
	    You("can't see something that far away clearly enough!");
	    return 0;
	} else if (u.uswallow) {
	    You("can't see anything in here!");
	    exercise(A_WIS, FALSE); /* What were you THINKING! */
	    u.dx = 0;
	    u.dy = 0;
	    return 1;
	} else if (!cansee(cc.x, cc.y) || IS_STWALL(levl[cc.x][cc.y].typ)) {
	    You("can't see that location!");
	    return 0;
	} else {
	    u.dx=cc.x;
	    u.dy=cc.y;
	    return 1;
	}
}

void
losespells(howmuch)
	int howmuch;
{
	int  n;

	for (n = 0; n < MAXSPELL && spellid(n) != NO_SPELL; n++){
		ndecrnknow(n, howmuch);
	}
}

/* the '+' command -- view known spells */
int
dovspell()
{
	int spell_no;
	if (getspell(&spell_no, SPELLMENU_VIEW))
		return spelleffects(spell_no, FALSE, 0);
	return 0;
}

int
dospiritmenu(prompt, power_no, respect_timeout)
const char *prompt;
int *power_no;
int respect_timeout;
{
	winid tmpwin;
	int n, how;
	char buf[BUFSZ];
	menu_item *selected;
	anything any, anyvoid;
	int i,s,j;
	long place;
	
	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */
	anyvoid.a_void = 0;		/* zero out all bits */
	
	Sprintf(buf, "Select spirit power:");
	add_menu(tmpwin, NO_GLYPH, &anyvoid, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	
	if(flags.timeoutOrder){
		for(s=0; s<NUM_BIND_SPRITS; s++){
			if(u.spirit[s]){
				j=0;
				place = 1;
				while(!(spiritPOwner[u.spiritPOrder[j]] & place)){
					j++;
					place = place << 1;
				}
				add_menu(tmpwin, NO_GLYPH, &anyvoid, 0, 0, ATR_BOLD, sealNames[j], MENU_UNSELECTED);
				for(i = 0; i<52; i++){
					if(spiritPOwner[u.spiritPOrder[i]] == u.spirit[s]){
						if(u.spiritPColdowns[u.spiritPOrder[i]] < monstermoves || !respect_timeout){
							Sprintf1(buf, spiritPName[u.spiritPOrder[i]]);
							any.a_int = u.spiritPOrder[i]+1;	/* must be non-zero */
							add_menu(tmpwin, NO_GLYPH, &any,
								i<26 ? 'a'+(char)i : 'A'+(char)(i-26), 
								0, ATR_NONE, buf, MENU_UNSELECTED);
						} else {
							Sprintf(buf, " %2ld %s", u.spiritPColdowns[u.spiritPOrder[i]] - monstermoves + 1, spiritPName[u.spiritPOrder[i]]);
							add_menu(tmpwin, NO_GLYPH, &anyvoid, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
						}
					}
				}
			}
		}
	} else {
		for(i = 0; i<52; i++){
			if(((u.spiritPOrder[i] != -1 &&
				spiritPOwner[u.spiritPOrder[i]] & u.sealsActive &&
				!(spiritPOwner[u.spiritPOrder[i]] & SEAL_SPECIAL)) || 
				(spiritPOwner[u.spiritPOrder[i]] & SEAL_SPECIAL && 
				spiritPOwner[u.spiritPOrder[i]] & u.specialSealsActive & ~SEAL_SPECIAL))
			){
				if(u.spiritPColdowns[u.spiritPOrder[i]] < monstermoves || !respect_timeout){
					Sprintf1(buf, spiritPName[u.spiritPOrder[i]]);
					any.a_int = u.spiritPOrder[i]+1;	/* must be non-zero */
					add_menu(tmpwin, NO_GLYPH, &any,
						i<26 ? 'a'+(char)i : 'A'+(char)(i-26), 
						0, ATR_NONE, buf, MENU_UNSELECTED);
				} else {
					Sprintf(buf, " %2ld %s", u.spiritPColdowns[u.spiritPOrder[i]] - monstermoves + 1, spiritPName[u.spiritPOrder[i]]);
					add_menu(tmpwin, NO_GLYPH, &anyvoid, 0, 0, ATR_NONE, buf, MENU_UNSELECTED);
				}
			}
		}
	}
	end_menu(tmpwin, prompt);

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	if(n > 0) *power_no = selected[0].item.a_int - 1;
	destroy_nhwindow(tmpwin);
	return (n > 0) ? TRUE : FALSE;
}

STATIC_OVL boolean
dospellmenu(splaction, spell_no)
int splaction;	/* SPELLMENU_CAST, SPELLMENU_VIEW, SPELLMENU_DESCRIBE, SPELLMENU_MAINTAIN, or spl_book[] index */
int *spell_no;
{
	winid tmpwin;
	int i, n, how;
	int maintainable = 0;
	char buf[BUFSZ];
	char buf2[BUFSZ];
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */

	update_alternate_spells();	// make sure all spells are listed
	
	/*
	 * The correct spacing of the columns depends on the
	 * following that (1) the font is monospaced and (2)
	 * that selection letters are pre-pended to the given
	 * string and are of the form "a - ".
	 *
	 * To do it right would require that we implement columns
	 * in the window-ports (say via a tab character).
	 */
	any.a_void = 0;		/* zero out all bits */
	//Standard Spells
	if (!iflags.menu_tab_sep)
		Sprintf(buf, "%-20s     Level  %-12s Fail   Memory", "    Name", "Category");
	else
		Sprintf(buf, "Name\tLevel\tCategory\tFail\tMemory");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {

		if (can_maintain_spell(spellid(i))){
			maintainable += 1;
		}
		else {
			if (splaction == SPELLMENU_MAINTAIN)
				continue;
		}
		Sprintf(buf2, "%s%s", spellname(i), spell_maintained(spellid(i)) ? " [M]" : "");
		Sprintf(buf, iflags.menu_tab_sep ?
			"%s\t%-d%s\t%s\t%-d%%\t%-d%%\t" : "%-20s  %2d%s   %-12s %3d%%     %3d%%",
			buf2, spellev(i),
			spellknow(i) ? " " : "*",
			spelltypemnemonic(spell_skilltype(spellid(i))),
			100 - percent_success(i),
			(spellknow(i) * 100 + (KEEN - 1)) / KEEN
		);

		any.a_int = i+1;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			 spellet(i), 0, ATR_NONE, buf,
			 (i == splaction) ? MENU_SELECTED : MENU_UNSELECTED);
	}
	//Other menu options
	if (splaction != SPELLMENU_CAST && splaction < 0) {
		Sprintf(buf, "Cast a spell instead");
		any.a_int = SPELLMENU_CAST;
		add_menu(tmpwin, NO_GLYPH, &any,
			'!', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	if (splaction != SPELLMENU_MAINTAIN && splaction < 0 && maintainable){
		// Maintain a spell
		Sprintf(buf, "Maintain a spell instead");
		any.a_int = SPELLMENU_MAINTAIN;
		add_menu(tmpwin, NO_GLYPH, &any,
			'#', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	if (splaction != SPELLMENU_DESCRIBE && splaction < 0){
		// Describe a spell
		Sprintf(buf, "Describe a spell instead");
		any.a_int = SPELLMENU_DESCRIBE;
		add_menu(tmpwin, NO_GLYPH, &any,
			'?', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	if (splaction != SPELLMENU_VIEW && splaction < 0 && spellid(1) != NO_SPELL){
		// Describe a spell
		Sprintf(buf, "Rearrange spells instead");
		any.a_int = SPELLMENU_VIEW;
		add_menu(tmpwin, NO_GLYPH, &any,
			'+', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	switch (splaction)
	{
	case SPELLMENU_VIEW:
		Sprintf(buf, "Choose which spell to reorder");
		break;
	case SPELLMENU_CAST:
		Sprintf(buf, "Choose which spell to cast");
		break;
	case SPELLMENU_MAINTAIN:
		Sprintf(buf, "Choose which spell to (un)maintain");
		break;
	case SPELLMENU_DESCRIBE:
		Sprintf(buf, "Choose which spell to describe");
		break;
	default:
		Sprintf(buf, "Reordering spells; swap '%c' with", spellet(splaction));
		break;
	}
	end_menu(tmpwin, buf);

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);

	if (n > 0){
		int s_no = selected[0].item.a_int - 1;

		if (selected[0].item.a_int < 0){
			return dospellmenu(selected[0].item.a_int, spell_no);
		}
		else if (!(splaction == SPELLMENU_VIEW && spellid(1) == NO_SPELL)) {
			/* we aren't attempting to rearrange spells with only 1 spell known */
			switch (splaction)
			{
			case SPELLMENU_VIEW:
				*spell_no = s_no;
				return dospellmenu(s_no, spell_no);

			case SPELLMENU_CAST:
				*spell_no = s_no;
				return TRUE;

			case SPELLMENU_MAINTAIN:
				if (!spell_maintained(spellid(s_no)))
				{
					spell_maintain(spellid(s_no));
					You("begin maintaining %s.", spellname(s_no));
				}
				else
				{
					spell_unmaintain(spellid(s_no));
					You("stop maintaining %s.", spellname(s_no));
				}
				return FALSE;

			case SPELLMENU_DESCRIBE:
				describe_spell(s_no);
				return dospellmenu(splaction, spell_no);

			default:
			{
				struct spell spl_tmp;
				spl_tmp = spl_book[*spell_no];
				spl_book[*spell_no] = spl_book[s_no];
				spl_book[s_no] = spl_tmp;
				return dospellmenu(SPELLMENU_VIEW, spell_no);
			}
			} // switch(splaction)
		} // doing something allowable
	} // menu item was selected
	return FALSE;
}


STATIC_OVL void
describe_spell(spellID)
int spellID;
{
	struct obj *pseudo;

	winid datawin;
	char name[20];
	char stats[30];
	char fail[20];
	char known[20];
	
	char desc1[80] = " ";
	char desc2[80] = " ";
	char desc3[80] = " ";
	char desc4[80] = " ";

		pseudo = mksobj(spellid(spellID), FALSE, FALSE);
		pseudo->blessed = pseudo->cursed = 0;
		sprintf(name,  " %s", spellname(spellID));
		name[1] = name[1] - 32;
		sprintf(stats, " Level %d %s spell", spellev(spellID), spelltypemnemonic(spell_skilltype(spellid(spellID))));
		sprintf(fail,  " Fail chance: %d%%", (100 - percent_success(spellID)));
		sprintf(known, " Memory:      %d%%", (spellknow(spellID) * 100 + (KEEN - 1)) / KEEN);

	switch (pseudo->otyp){
		case SPE_LIGHTNING_BOLT:
			strcat(desc1, "Creates a directed bolt of lightning that can bounce off walls.");
			strcat(desc2, "The flash is blindingly bright, and the shock can damage wands.");
			strcat(desc3, "Deals no damage to shock-resistant creatures.");
			strcat(desc4, "");
			break;
		case SPE_CONE_OF_COLD:
			strcat(desc1, "Creates a directed cone of cold that can bounce off walls.");
			strcat(desc2, "The chill can freeze potions, shattering them.");
			strcat(desc3, "Deals no damage to cold-resistant creatures.");
			strcat(desc4, "");
			break;
		case SPE_FIREBALL:
			strcat(desc1, "Launches a directed fireball that explodes on hitting something.");
			strcat(desc2, "The fire can boil potions and burn other flammable items.");
			strcat(desc3, "Deals no damage to fire-resistant creatures.");
			strcat(desc4, "");
			break;
		case SPE_ACID_SPLASH:
			strcat(desc1, "Splashes acid in an area next to you.");
			strcat(desc2, "The acid can boil potions and wet other items.");
			strcat(desc3, "Deals no damage to acid-resistant creatures.");
			strcat(desc4, "");
			break;
		case SPE_LIGHTNING_STORM:
			strcat(desc1, "Causes many lightning strikes in a large area around a target.");
			strcat(desc2, "The electric shock can damage wands.");
			strcat(desc3, "Confined spaces are detrimental to the spell\'s accuracy.");
			strcat(desc4, "Deals no damage to shock-resistant creatures.");
			break;
		case SPE_BLIZZARD:
			strcat(desc1, "Creates a series of cold explosions centered around a target.");
			strcat(desc2, "The chill can freeze potions, shattering them.");
			strcat(desc3, "Deals no damage to cold-resistant creatures.");
			strcat(desc4, "");
			break;
		case SPE_FIRE_STORM:
			strcat(desc1, "Creates a large firey explosion nearby a target.");
			strcat(desc2, "The fire can boil potions and burn other flammable items.");
			strcat(desc3, "Worn armor is also damaged.");
			strcat(desc4, "Deals no damage to fire-resistant creatures.");
			break;
		case SPE_HASTE_SELF:
			strcat(desc1, "You temporarily move very quickly.");
			strcat(desc2, "Casting while already very fast increase the duration of your haste.");
			strcat(desc3, "Can be maintained.");
			strcat(desc4, "");
			break;
		case SPE_FORCE_BOLT:
			strcat(desc1, "Creates a directed ray of physical force.");
			strcat(desc2, "It can shatter boulders, doors, drawbridges, iron bars, and more.");
			strcat(desc3, "Deals no damage to magic-resistant creatures.");
			strcat(desc4, "");
			break;
		case SPE_SLEEP:
			strcat(desc1, "Creates a directed beam of magic that bounces off walls.");
			strcat(desc2, "Creatures hit by the beam fall asleep.");
			strcat(desc3, "Has no effect on sleep-resistant creatures.");
			strcat(desc4, "");
			break;
		case SPE_MAGIC_MISSILE:
			strcat(desc1, "Creates a directed beam of magic missiles that bounces off walls.");
			strcat(desc2, "Deals no damage to magic-resistant creatures.");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_KNOCK:
			strcat(desc1, "Creates a directed ray of unlocking magic.");
			strcat(desc2, "Containers and doors hit by the ray unlock.");
			strcat(desc3, "Secret doors are uncovered and drawbridges are lowered.");
			strcat(desc4, "Iron balls can be unchained from you by casting downwards.");
			break;
		case SPE_SLOW_MONSTER:
			strcat(desc1, "Creates a directed ray of slowing magic.");
			strcat(desc2, "Creatures affected by it move more slowly.");
			strcat(desc3, "The strength of the spell is dependent on your skill.");
			strcat(desc4, "Creatures can roll to resist the effect.");
			break;
		case SPE_WIZARD_LOCK:
			strcat(desc1, "Creates a directed ray of locking magic.");
			strcat(desc2, "Containers and doors hit by the ray lock.");
			strcat(desc3, "Doors appear in doorways and drawbridges are raised.");
			strcat(desc4, "");
			break;
		case SPE_DIG:
			strcat(desc1, "Creates a directed beam of excavating magic.");
			strcat(desc2, "Solid rock, walls, and trees are removed.");
			strcat(desc3, "However, in some places, the terrain refuses to be altered.");
			strcat(desc4, "");
			break;
		case SPE_TURN_UNDEAD:
			strcat(desc1, "Creates a directed ray of necromatic magic.");
			strcat(desc2, "Corpses hit by the ray revive into living creatures.");
			strcat(desc3, "Pets revived this way will not always be revived tame.");
			strcat(desc4, "");
			break;
		case SPE_POLYMORPH:
			strcat(desc1, "Creates a directed ray of polymorphing magic.");
			strcat(desc2, "Creatures affected by the ray polymorph into different creatures.");
			strcat(desc3, "Items hit by the ray polymorph into similar items, or evaporate.");
			strcat(desc4, "Creatures can roll to resist the effect. Magic resistance provides immunity.");
			break;
		case SPE_TELEPORT_AWAY:
			strcat(desc1, "Creates a directed ray of teleportation magic.");
			strcat(desc2, "Creatures and items hit by the ray are teleported away.");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_CANCELLATION:
			strcat(desc1, "Creates a directed ray of negating magic.");
			strcat(desc2, "Creatures affected by the ray are cancelled, losing most special abilites.");
			strcat(desc3, "Items hit by the ray are disenchanted, made uncursed, and made mundane.");
			strcat(desc4, "Creatures can roll to resist the effect.");
			break;
		case SPE_FINGER_OF_DEATH:
			strcat(desc1, "Creates a directed beam of death magic that bounces off walls.");
			strcat(desc2, "Creatures hit by the beam are killed outright.");
			strcat(desc3, "Has no effect on undead, demons, or magic-resistant creatures.");
			strcat(desc4, "");
			break;
		case SPE_POISON_SPRAY:
			strcat(desc1, "Creates a directed spray of posion.");
			strcat(desc2, "Creatures hit are killed instantly.");
			strcat(desc3, "Has no effect on poison-resistant creatures.");
			strcat(desc4, "");
			break;
		case SPE_LIGHT:
			strcat(desc1, "Creates a lit field around you.");
			strcat(desc2, "");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_DETECT_UNSEEN:
			strcat(desc1, "Detects unseen things in an area around you.");
			strcat(desc2, "The location of monsters are shown.");
			strcat(desc3, "Traps and secret doors become visible.");
			strcat(desc4, "Can be maintained to grant see invisible.");
			break;
		case SPE_HEALING:
			strcat(desc1, "Creates a directed ray of healing magic.");
			strcat(desc2, "Creatures hit by the ray are healed for a small amount.");
			strcat(desc3, "Blinded monsters hit by the ray can see again.");
			strcat(desc4, "");
			break;
		case SPE_EXTRA_HEALING:
			strcat(desc1, "Creates a directed ray of healing magic.");
			strcat(desc2, "Creatures hit by the ray are healed for a moderate amount.");
			strcat(desc3, "Blinded creatures hit by the ray can see again.");
			strcat(desc4, "");
			break;
		case SPE_DRAIN_LIFE:
			strcat(desc1, "Creates a directed ray of draining magic.");
			strcat(desc2, "Creatures affected by the ray lose a level and max health.");
			strcat(desc3, "Items affected by the ray lose one point of enchantment.");
			strcat(desc4, "Has no effect on undead, demons, or other drain-resistant creatures.");
			break;
		case SPE_STONE_TO_FLESH:
			strcat(desc1, "Creates a directed ray of magic that turns stone into meat.");
			strcat(desc2, "Boulders, rocks, and stone items turn into meaty equivalents.");
			strcat(desc3, "Stone golems turn into flesh golems.");
			strcat(desc4, "Cast at oneself, it counteracts the effects of delayed petrification.");
			break;
		case SPE_REMOVE_CURSE:
			strcat(desc1, "Cleanses curses from items in your inventory.");
			strcat(desc2, "At Unskilled or Basic, it only affects loadstones and wielded or worn items.");
			strcat(desc3, "At Skilled or better, it affects your entire inventory.");
			strcat(desc4, "");
			break;
		case SPE_CONFUSE_MONSTER:
			strcat(desc1, "Makes your next melee attack confuse the monster you hit.");
			strcat(desc2, "Casting multiple times increases the number of remaining hits.");
			strcat(desc3, "At Skilled, one cast grants several confusing hits.");
			strcat(desc4, "Creatures can roll to resist the effect.");
			break;
		case SPE_DETECT_FOOD:
			strcat(desc1, "Detects all food items on the level.");
			strcat(desc2, "Has no nutrition cost to cast.");
			strcat(desc3, "At Skilled or better, you will be warned of the first harmful food you eat.");
			strcat(desc4, "");
			break;
		case SPE_CAUSE_FEAR:
			strcat(desc1, "Causes all enemies in line-of-sight to flee.");
			strcat(desc2, "Creatures can roll to resist the effect.");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_PACIFY_MONSTER:
			strcat(desc1, "Attempts to pacify an adjacent creature.");
			strcat(desc2, "Creatures can roll to resist the effect.");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_CHARM_MONSTER:
			strcat(desc1, "Attempts to charm an adjacent creature.");
			strcat(desc2, "Untamable creatures become peaceful.");
			strcat(desc3, "Creatures can roll to resist the effect.");
			strcat(desc4, "");
			break;
		case SPE_MAGIC_MAPPING:
			strcat(desc1, "Maps the current level.");
			strcat(desc2, "However, some levels are unmappable.");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_CREATE_MONSTER:
			strcat(desc1, "Creates a monster next to you.");
			strcat(desc2, "It creates monsters with normal generation.");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_IDENTIFY:
			strcat(desc1, "Identifes some (to all) of your inventory.");
			strcat(desc2, "");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_DETECT_TREASURE:
			strcat(desc1, "Detects all items on the level.");
			strcat(desc2, "At Skilled or better, the detected objects are marked on your map.");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_DETECT_MONSTERS:
			strcat(desc1, "Detects all monsters on the level.");
			strcat(desc2, "At Skilled or better, you continue detecting monsters for a time.");
			strcat(desc3, "Can be maintained at Skilled or better.");
			strcat(desc4, "");
			break;
		case SPE_LEVITATION:
			strcat(desc1, "You start levitating for a time.");
			strcat(desc2, "At Skilled or better, you can descend at will.");
			strcat(desc3, "Can be maintained.");
			strcat(desc4, "");
			break;
		case SPE_RESTORE_ABILITY:
			strcat(desc1, "Restores your drained attribute scores.");
			strcat(desc2, "Attributes lost from abuse cannot be restored.");
			strcat(desc3, "At Skilled or better, it restores all lost attributes at once.");
			strcat(desc4, "");
			break;
		case SPE_INVISIBILITY:
			strcat(desc1, "You turn invisible for a time.");
			strcat(desc2, "Can be maintained.");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_CURE_BLINDNESS:
			strcat(desc1, "Cures blindness.");
			strcat(desc2, "");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_CURE_SICKNESS:
			strcat(desc1, "Cures food poisioning, illness, and sliming.");
			strcat(desc2, "");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_CREATE_FAMILIAR:
			strcat(desc1, "Creates a tame creature.");
			strcat(desc2, "1/3 of the time, the summoned creature is a tame domestic pet.");
			strcat(desc3, "2/3 of the time, the summoned creature is a tame random monster.");
			strcat(desc4, "");
			break;
		case SPE_CLAIRVOYANCE:
			strcat(desc1, "You map out an area of the dungeon around you.");
			strcat(desc2, "At Skilled or better, you can target anywhere on the level.");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		case SPE_ANTIMAGIC_SHIELD:
			strcat(desc1, "Temporarily protects you from magic.");
			strcat(desc2, "While active, you cannot cast any spell but this.");
			strcat(desc3, "Recasting increases the duration of the effect.");
			strcat(desc4, "Can be maintained.");
		case SPE_PROTECTION:
			strcat(desc1, "Temporarily improves your AC. AC from this spell is better than normal.");
			strcat(desc2, "While active, it reduces your magic power recovery.");
			strcat(desc3, "The strength and duration of the effect is improved with casting skill.");
			strcat(desc4, "Can be maintained.");
			break;
		case SPE_JUMPING:
			strcat(desc1, "You make a magically-boosted jump.");
			strcat(desc2, "The range that you can jump is improved with casting skill.");
			strcat(desc3, "");
			strcat(desc4, "");
			break;
		default:
			impossible("Spell %d?", pseudo->otyp);
	}
	datawin = create_nhwindow(NHW_TEXT);
	if(spellID < MAXSPELL){
		putstr(datawin, 0, "");
		putstr(datawin, 0, name);
		putstr(datawin, 0, "");
		putstr(datawin, 0, stats);
		putstr(datawin, 0, "");
		putstr(datawin, 0, fail);
		putstr(datawin, 0, known);
		putstr(datawin, 0, "");
	}
	if (desc1[3] != 0) putstr(datawin, 0, desc1);
	if (desc2[3] != 0) putstr(datawin, 0, desc2);
	if (desc3[3] != 0) putstr(datawin, 0, desc3);
	if (desc4[3] != 0) putstr(datawin, 0, desc4);
	display_nhwindow(datawin, FALSE);
	destroy_nhwindow(datawin);
	return;
}


#ifdef DUMP_LOG
void 
dump_spells()
{
	int i;
	char buf[BUFSZ];

	if (spellid(0) == NO_SPELL) {
	    dump("", "You didn't know any spells.");
	    dump("", "");
	    return;
	}
	dump("", "Spells known in the end");

	Sprintf(buf, "%-20s   Level    %-12s Fail", "    Name", "Category");
	dump("  ",buf);
	for (i = 0; i < MAXSPELL && spellid(i) != NO_SPELL; i++) {
		Sprintf(buf, "%c - %-20s  %2d%s   %-12s %3d%%",
			spellet(i), spellname(i), spellev(i),
			spellknow(i) ? " " : "*",
			spelltypemnemonic(spell_skilltype(spellid(i))),
			100 - percent_success(i));
		dump("  ", buf);
	}
	dump("","");

} /* dump_spells */
#endif

/* Integer square root function without using floating point. */
STATIC_OVL int
isqrt(val)
int val;
{
    int rt = 0;
    int odd = 1;
    while(val >= odd) {
	val = val-odd;
	odd = odd+2;
	rt = rt + 1;
    }
    return rt;
}

STATIC_OVL int
percent_success(spell)
int spell;
{
	/* Intrinsic and learned ability are combined to calculate
	 * the probability of player's success at cast a given spell.
	 */
	int chance, splcaster, special, statused;
	int difficulty;
	int skill;
	
	if(Nullmagic && spellid(spell)!=SPE_ANTIMAGIC_SHIELD) return 0;
	
	if(
		((spellid(spell) == SPE_FORCE_BOLT || spellid(spell) == SPE_MAGIC_MISSILE) && 
			uwep && uwep->oartifact == ART_ANNULUS && uwep->otyp == CHAKRAM)
	) return 100;
	
	/* Calculate intrinsic ability (splcaster) */

	splcaster = urole.spelbase;
	special = urole.spelheal;
	
	if(u.specialSealsActive&SEAL_NUMINA){
		if(abs(u.wisSpirits - u.intSpirits) <= 1) statused = max(ACURR(A_WIS), ACURR(A_INT));
	} else if(u.wisSpirits > u.intSpirits){
		statused = ACURR(A_WIS);
	} else if(u.wisSpirits < u.intSpirits){
		statused = ACURR(A_INT);
	} else if(u.wisSpirits || u.intSpirits){
		statused = max(ACURR(A_WIS), ACURR(A_INT));
	} else {
		if(Race_if(PM_INCANTIFIER)) statused = ACURR(A_INT);
		else if(Role_if(PM_EXILE)) statused = min(ACURR(A_WIS), ACURR(A_INT));
		else if(Role_if(PM_BARD) && spell_skilltype(spellid(spell)) == P_ENCHANTMENT_SPELL)
			statused = ACURR(A_CHA);
		else statused = ACURR(urole.spelstat);
	}
	
	if(uwep){
		int cast_bon;
		// powerful channeling artifacts
		if(uwep->oartifact == ART_TENTACLE_ROD
			|| uwep->oartifact == ART_ARYFAERN_KERYM
			|| uwep->oartifact == ART_INFINITY_S_MIRRORED_ARC
			|| uwep->oartifact == ART_PROFANED_GREATSCYTHE
			|| uwep->oartifact == ART_GARNET_ROD
		) splcaster -= urole.spelarmr;

		if (uwep->otyp == ART_DEATH_SPEAR_OF_VHAERUN
			&& spell_skilltype(spellid(spell)) == P_ATTACK_SPELL
			&& Race_if(PM_DROW)
			&& !flags.initgend
		) {	// Bonus to attack spells, including granted drain life
			splcaster -= urole.spelarmr;
		}

		if (uwep->otyp == KHAKKHARA) {	// a priestly channeling tool
			cast_bon = 0;
			if(spell_skilltype(spellid(spell)) == P_CLERIC_SPELL
			 || spell_skilltype(spellid(spell)) == P_HEALING_SPELL
			 || Role_if(PM_PRIEST)
			 || Role_if(PM_MONK)
			) cast_bon += 2;
			if (uwep->oartifact)
				cast_bon *= 2;
			splcaster -= urole.spelarmr * cast_bon / 3;
		}

		if (uwep->otyp == BELL || uwep->otyp == BELL_OF_OPENING) {	// a priestly channeling tool
			cast_bon = 0;
			if(spell_skilltype(spellid(spell)) == P_CLERIC_SPELL
			 || spell_skilltype(spellid(spell)) == P_HEALING_SPELL
			 || Role_if(PM_PRIEST)
			) cast_bon += 2;
			if (uwep->oartifact || objects[uwep->otyp].oc_unique)
				cast_bon *= 2;
			splcaster -= urole.spelarmr * cast_bon / 3;
		}

		if (uwep->otyp == SCALPEL) {	// a tool of healing
			cast_bon = 0;
			if(spell_skilltype(spellid(spell)) == P_HEALING_SPELL)
			cast_bon += 2;
			if (uwep->oartifact)
				cast_bon *= 2;
			splcaster -= urole.spelarmr * cast_bon / 3;
		}

		if (uwep->otyp == CRYSTAL_BALL) {	// a diviner's tool
			cast_bon = 0;
			if(spell_skilltype(spellid(spell)) == P_DIVINATION_SPELL)
			cast_bon += 2;
			if (uwep->oartifact)
				cast_bon *= 2;
			splcaster -= urole.spelarmr * cast_bon / 3;
		}

		if (uwep->otyp == QUARTERSTAFF) {	// a sorcerous channeling tool
			cast_bon = 0;
			if (spell_skilltype(spellid(spell)) == P_ATTACK_SPELL)
				cast_bon += 2;
			if (uwep->oartifact)
				cast_bon *= 2;
			splcaster -= urole.spelarmr * cast_bon / 3;
		}

		if (uwep->otyp == SHEPHERD_S_CROOK) {	// a tool for leading and manipulating things
			cast_bon = 0;
			if (spell_skilltype(spellid(spell)) == P_ENCHANTMENT_SPELL)
				cast_bon += 2;
			if (uwep->oartifact)
				cast_bon *= 2;
			splcaster -= urole.spelarmr * cast_bon / 3;
		}

		if (uwep->otyp == ATHAME) {	// a lesser sorecerous channeling tool
			cast_bon = 0;
			if (spell_skilltype(spellid(spell)) == P_ATTACK_SPELL || spell_skilltype(spellid(spell)) == P_ENCHANTMENT_SPELL)
				cast_bon += 1;
			if (uwep->oartifact && !(uwep->oartifact == ART_PEN_OF_THE_VOID && !(mvitals[PM_ACERERAK].died > 0)))
				cast_bon *= 2;
			splcaster -= cast_bon;
		}
		
		if(Role_if(PM_WIZARD) && uwep->oclass == WAND_CLASS) {	// a tool of spellweaving
			cast_bon = 1;
			if (uwep->oartifact)
				cast_bon *= 2;
			splcaster -= urole.spelarmr * cast_bon / 3;
		}
	}

	if (uarm){
		if (is_metallic(uarm) || uarm->oartifact == ART_DRAGON_PLATE)
			splcaster += urole.spelarmr;

		if (uarm->otyp == DROVEN_CHAIN_MAIL)
			splcaster -= urole.spelarmr;
	}
	
	if (uarmc){
		if (uarmc->otyp == WHITE_FACELESS_ROBE)
			splcaster -= 1;
		if (uarmc->otyp == BLACK_FACELESS_ROBE)
			splcaster -= 2;
		if (uarmc->otyp == SMOKY_VIOLET_FACELESS_ROBE)
			splcaster -= 4;
		if (uarmc->otyp == ROBE)
			splcaster -= (urole.spelarmr
			* ((uarmc->oartifact) ? 2 : 1)
			/ ((uarm && (is_metallic(uarm) || uarm->oartifact == ART_DRAGON_PLATE)) ? 2 : 1));
	}

	if (uarmh) {
		if (is_metallic(uarmh) && uarmh->otyp != HELM_OF_BRILLIANCE)
			splcaster += uarmhbon;

		if (uarmh->oartifact == ART_STORMHELM && spellid(spell) == SPE_LIGHTNING_STORM)
			splcaster -= urole.spelarmr;
	}

	if (uarmg) {
		if (is_metallic(uarmg))
			splcaster += uarmgbon;
	}

	if (uarmf) {
		if (is_metallic(uarmf))
			splcaster += uarmfbon;
	}

	if (uarms) {
		splcaster += urole.spelshld;
	}

	if(u.sealsActive&SEAL_PAIMON) splcaster -= urole.spelarmr;
	
	if(Race_if(PM_INCANTIFIER))
		splcaster += max(-3*urole.spelarmr,urole.spelsbon);

	if(spellid(spell) == urole.spelspec)
		splcaster += urole.spelsbon;

	/* `healing spell' bonus */
	if(spellid(spell) == SPE_HEALING ||
	    spellid(spell) == SPE_EXTRA_HEALING ||
	    spellid(spell) == SPE_CURE_BLINDNESS ||
	    spellid(spell) == SPE_CURE_SICKNESS ||
	    spellid(spell) == SPE_RESTORE_ABILITY ||
	    spellid(spell) == SPE_REMOVE_CURSE
	) (uarm && uarm->otyp == HEALER_UNIFORM) ? 
		(splcaster += 2*special) :
		(splcaster += special);

	if(splcaster > 20) splcaster = 20;

	/* Calculate learned ability */

	/* Players basic likelihood of being able to cast any spell
	 * is based of their `magic' statistic. (Int or Wis)
	 */
	chance = 11 * statused / 2;

	/*
	 * High level spells are harder.  Easier for higher level casters.
	 * The difficulty is based on the hero's level and their skill level
	 * in that spell type.
	 */
	skill = P_SKILL(spell_skilltype(spellid(spell)));
	skill = max(skill,P_UNSKILLED) - 1;	/* unskilled => 0 */
	difficulty= (spellev(spell)-1) * 4 - ((skill * 6) + (u.ulevel/3) + 1);

	if(difficulty > 0) {
		/* Player is too low level or unskilled. */
		chance -= isqrt(900 * difficulty + 2000);
	} else {
		/* Player is above level.  Learning continues, but the
		 * law of diminishing returns sets in quickly for
		 * low-level spells.  That is, a player quickly gains
		 * no advantage for raising level.
		 */
		int learning = 15 * -difficulty / spellev(spell);
		chance += learning > 20 ? 20 : learning;
	}

	/* Clamp the chance: >18 stat and advanced learning only help
	 * to a limit, while chances below "hopeless" only raise the
	 * specter of overflowing 16-bit ints (and permit wearing a
	 * shield to raise the chances :-).
	 */
	if (chance < 0) chance = 0;
	if (chance > 120) chance = 120;

	/* Wearing anything but a light shield makes it very awkward
	 * to cast a spell.  The penalty is not quite so bad for the
	 * player's role-specific spell.
	 */
	if (uarms && (is_metallic(uarms) || weight(uarms) > (int) objects[BUCKLER].oc_weight)) {
		if (spellid(spell) == urole.spelspec) {
			chance /= 2;
		} else {
			chance /= 4;
		}
	}

	/* Finally, chance (based on player intell/wisdom and level) is
	 * combined with ability (based on player intrinsics and
	 * encumbrances).  No matter how intelligent/wise and advanced
	 * a player is, intrinsics and encumbrance can prevent casting;
	 * and no matter how able, learning is always required.
	 */
	chance = chance * (20-splcaster) / 15 - splcaster;
	
	if(flags.silence_level){
		struct monst *cmon;
		int dist = 0;
		for(cmon = fmon; cmon; cmon = cmon->nmon){
			if(cmon->data == &mons[PM_ASPECT_OF_THE_SILENCE]){
				dist = (int) (3*sqrt(dist2(u.ux,u.uy,cmon->mx,cmon->my)));
				break;
			}
		}
		chance = min(100, chance);
		if(dist < 100) chance -= 100-dist;
	}
	
	//While the Naen syllable is active, the only spell failure chance comes from the Spire 
	if(u.unaen_duration)
		chance = 100;
	
	if(u.uz.dnum == neutral_dnum && u.uz.dlevel <= sum_of_all_level.dlevel){
		if(u.uz.dlevel == spire_level.dlevel) chance = 0;
		else if(u.uz.dlevel == sum_of_all_level.dlevel) chance += 100 - 10*spellev(spell);
		else if(u.uz.dlevel == spire_level.dlevel-1) chance -= 50*spellev(spell);
		else if(u.uz.dlevel == spire_level.dlevel-2) chance -= 40*spellev(spell);
		else if(u.uz.dlevel == spire_level.dlevel-3) chance -= 30*spellev(spell);
		else if(u.uz.dlevel == spire_level.dlevel-4) chance -= 20*spellev(spell);
		else if(u.uz.dlevel == spire_level.dlevel-5) chance -= 10*spellev(spell);
	}
	
	/* Clamp to percentile */
	if (chance > 100) chance = 100;
	if (chance < 0) chance = 0;

	return chance;
}


/* Learn a spell during creation of the initial inventory */
void
initialspell(obj)
struct obj *obj;
{
	int i;

	for (i = 0; i < MAXSPELL; i++) {
	    if (spellid(i) == obj->otyp) {
	         pline("Error: Spell %s already known.",
	         		OBJ_NAME(objects[obj->otyp]));
	         return;
	    }
	    if (spellid(i) == NO_SPELL)  {
	        spl_book[i].sp_id = obj->otyp;
	        spl_book[i].sp_lev = objects[obj->otyp].oc_level;
	        incrnknow(i);
	        return;
	    }
	}
	impossible("Too many spells memorized!");
	return;
}

/* Learn a ward during creation of the initial inventory */
void
initialward(obj)
struct obj *obj;
{
	// WARD_ACHERON			0x0000008L
	// WARD_QUEEN			0x0000200L
	// WARD_GARUDA			0x0000800L
	
	// WARD_ELDER_SIGN		0x0000080L
	// WARD_EYE				0x0000100L
	// WARD_CAT_LORD		0x0000400L
	if(obj->oward && !(u.wardsknown & obj->oward)){
		u.wardsknown |= obj->oward;
		return;
	}
	
	switch(rn2(16)){
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			if(!(u.wardsknown & WARD_ACHERON)){
				obj->oward = WARD_ACHERON;
				u.wardsknown |= obj->oward;
		break;
			}
		case 6:
		case 7:
		case 8:
			if(!(u.wardsknown & WARD_QUEEN)){
				obj->oward = WARD_QUEEN;
				u.wardsknown |= obj->oward;
		break;
			}
		case 9:
		case 10:
		case 11:
			if(!(u.wardsknown & WARD_GARUDA)){
				obj->oward = WARD_GARUDA;
				u.wardsknown |= obj->oward;
		break;
			}
		case 12:
		case 13:
			if(!(u.wardsknown & WARD_EYE)){
				obj->oward = WARD_EYE;
				u.wardsknown |= obj->oward;
		break;
			}
		case 14:
			if(!(u.wardsknown & WARD_CAT_LORD)){
				obj->oward = WARD_CAT_LORD;
				u.wardsknown |= obj->oward;
		break;
			}
		case 15:
			if(!(u.wardsknown & WARD_ELDER_SIGN)){
				obj->oward = WARD_ELDER_SIGN;
				u.wardsknown |= obj->oward;
			}
		break; /*Fall through to here*/
	}
	return;
}

long
doreadstudy(prompt)
const char *prompt;
{
	winid tmpwin;
	int n, how;
	char buf[BUFSZ];
	char incntlet = 'a';
	menu_item *selected;
	anything any;

	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_void = 0;		/* zero out all bits */

	Sprintf(buf, "Read the spellbook or Study its wardings?");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	
		Sprintf(buf, "Read spell");
		any.a_int = READ_SPELL;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'r', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		Sprintf(buf, "Study warding");
		any.a_int = STUDY_WARD;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			's', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	
	end_menu(tmpwin, prompt);

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return (n > 0) ? selected[0].item.a_int : 0;
}


void
set_spirit_powers(spirits_seal)
	long spirits_seal;
{
	int i,j;
	if(spirits_seal==0) return;
	for(i=0;i<NUMBER_POWERS;i++){
		if(spiritPOwner[i]==spirits_seal){
			u.spiritPColdowns[i] = 0;
			for(j=0;j<52;j++){
				if(u.spiritPOrder[j] == i) break;
			}
			if(j==52) for(j=0;j<52;j++){
				if(u.spiritPOrder[j] == -1 ||
					!((spiritPOwner[u.spiritPOrder[j]] & u.sealsActive &&
					!(spiritPOwner[u.spiritPOrder[j]] & SEAL_SPECIAL)) || 
					spiritPOwner[u.spiritPOrder[j]] & u.specialSealsActive & ~SEAL_SPECIAL)
				){
					u.spiritPOrder[j] = i;
					break;
				}
			}
			if(j==52) impossible("Could not find a free letter for power %d",i);
		}
	}
}

int
reorder_spirit_powers()
{
	int power_indx = -1;
	char swaplet;
	if(!u.sealsActive && !u.specialSealsActive){
		You("don't have any spirits bound.");
		return 0;
	}
	if (flags.menu_style == MENU_TRADITIONAL) {
		char ilet, lets[BUFSZ], qbuf[QBUFSZ];
	    /* we know there is at least 1 known spell */
		
		//put characters into lets here
		spiritLets(lets, FALSE);
		
		Sprintf(qbuf, "Use which power? [%s ?]", lets);
		ilet = yn_function(qbuf, (char *)0, '\0');

		if (index(quitchars, ilet))
		    return 0;
		
		if(check_spirit_let(ilet)){
			if(ilet >= 'a' && ilet <= 'z'){
				power_indx = (int)ilet-'a';
			} else if(swaplet >= 'A' && swaplet <= 'Z'){
				power_indx = (int)ilet-'A'+26;
			}
		} else {
			You("don't know that power.");
			return 0;
		}
	} else {
		int power_no;
		if(dospiritmenu("Choose which power to reorder", &power_no, TRUE))
			for(power_indx = 0; power_indx < 52; power_indx++){
				if(power_no == u.spiritPOrder[power_indx])
					break;
			}
	}
	if(power_indx == -1) return 0;
	pline("Move power to what letter? (a-z, A-Z)");
	swaplet = readchar();
	if(swaplet >= 'a' && swaplet <= 'z'){
		int power = u.spiritPOrder[power_indx];
		u.spiritPOrder[power_indx] = u.spiritPOrder[(int)swaplet-'a'];
		u.spiritPOrder[(int)swaplet-'a'] = power;
		pline("Power reordered.");
		return 0;
	} else if(swaplet >= 'A' && swaplet <= 'Z'){
		int power = u.spiritPOrder[power_indx];
		u.spiritPOrder[power_indx] = u.spiritPOrder[swaplet-'A'+26];
		u.spiritPOrder[swaplet-'A'+26] = power;
		pline("Power reordered.");
		return 0;
	} else {
		pline("Invalid letter.");
		return 0;
	}
}

void
dopseudonatural()
{
	struct monst *mon, *nmon;
	int tmp, weptmp, tchtmp;
	struct attack symbiote[] = 
	{
		{AT_TENT,AD_PHYS,5,spiritDsize()},
		{0,0,0,0}
	};
	for(mon = fmon;mon;mon = nmon){
		nmon = mon->nmon;
		if(!mon || mon->mtame || !rn2(5))
			continue;
		if(distmin(u.ux, u.uy, mon->mx, mon->my) > 2 || !couldsee(mon->mx, mon->my))
			continue;
		if((!Stone_resistance && (touch_petrifies(mon->data)
		 || mon->data == &mons[PM_MEDUSA]))
		 || mon->data == &mons[PM_PALE_NIGHT]
		) continue;
		
		if(mon){
			find_to_hit_rolls(mon,&tmp,&weptmp,&tchtmp);
			hmonwith(mon, tmp, weptmp, tchtmp, symbiote, 1);
		}
	}
}

/*spell.c*/
