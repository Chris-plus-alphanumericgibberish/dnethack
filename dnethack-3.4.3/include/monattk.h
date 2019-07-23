/*	SCCS Id: @(#)monattk.h	3.4	2002/03/24	*/
/* NetHack may be freely redistributed.  See license for details. */
/* Copyright 1988, M. Stephenson */

#ifndef MONATTK_H
#define MONATTK_H

/*	Add new attack types below - ordering affects experience (exper.c).
 *	Attacks > AT_BUTT are worth extra experience.
 */
#define AT_ANY		(-1)	/* fake attack; dmgtype_fromattack wildcard */
#define AT_NONE		0	/* passive monster (ex. acid blob) */
#define AT_CLAW		1	/* claw (punch, hit, etc.) */
#define AT_BITE		2	/* bite */
#define AT_KICK		3	/* kick */
#define AT_BUTT		4	/* head butt (ex. a unicorn) */
#define AT_TUCH		5	/* touches */
#define AT_STNG		6	/* sting */
#define AT_HUGS		7	/* crushing bearhug */
#define AT_SPIT		10	/* spits substance - ranged */
#define AT_ENGL		11	/* engulf (swallow or by a cloud) */
#define AT_BREA		12	/* breath - ranged */
#define AT_EXPL		13	/* explodes - proximity */
#define AT_BOOM		14	/* explodes when killed */
#define AT_GAZE		15	/* gaze - ranged, active, like pyrolisk */
#define AT_TENT		16	/* tentacles */
#define AT_ARRW		17	/* fire silver arrows from internal reservour.  Other ammo created as needed. */
#define AT_WHIP		18	/* Whips you */
#define AT_LRCH		19	/* Reach attack */
#define AT_HODS		20  /* Hod Sephirah's mirror attack */
#define AT_LNCK		21  /* Bite attack with reach */
#define AT_MMGC		22	/* uses magic spell(s), but don't allow player spellcasting (Monster-only MaGiC) */
#define AT_ILUR		23	/* Two stage swallow attack, currently belongs to Illurien only */
#define AT_HITS		24	/* Automatic hit, no contact */
#define AT_WISP		25	/* Attack with wisps of mist, no contact */
#define AT_TNKR		26	/* Tinker attacks */
#define AT_SHDW		27	/* Phased non-contact attack */
#define AT_BEAM		28	/* non-contact ranged beam attack */
#define AT_DEVA		29	/* million-arm weapon attack */
#define AT_5SQR		30	/* five square reach touch */
#define AT_WDGZ		31	/* wide gaze - passive, like medusa */
#define AT_REND		32	/* hits if the previous two attacks hit, otherwise does nothing */

#define AT_WEAP		252	/* uses weapon */
#define AT_XWEP		253	/* uses offhanded weapon */
#define AT_MARI		254	/* uses (i-2)th unwielded weapon in inventory */
#define AT_MAGC		255	/* uses magic spell(s) */

/*	Add new damage types below.
 *
 *	Note that 1-10 correspond to the types of attack used in buzz().
 *	Please don't disturb the order unless you rewrite the buzz() code.
 */
#define AD_ANY		(-1)	/* fake damage; attacktype_fordmg wildcard */
#define AD_PHYS		0	/* ordinary physical */
#define AD_MAGM		1	/* magic missiles */
#define AD_FIRE		2	/* fire damage */
#define AD_COLD		3	/* frost damage */
#define AD_SLEE		4	/* sleep ray */
#define AD_DISN		5	/* disintegration (death ray) */
#define AD_ELEC		6	/* shock damage */
#define AD_DRST		7	/* drains str (poison) */
#define AD_ACID		8	/* acid damage */
#define AD_SPC1		9	/* for extension of buzz() */
#define AD_SPC2		10	/* for extension of buzz() */
#define AD_BLND		11	/* blinds (yellow light) */
#define AD_STUN		12	/* stuns */
#define AD_SLOW		13	/* slows */
#define AD_PLYS		14	/* paralyses */
#define AD_DRLI		15	/* drains life levels */
#define AD_DREN		16	/* drains magic energy */
#define AD_LEGS		17	/* damages legs (xan) */
#define AD_STON		18	/* petrifies (Medusa, cockatrice) */
#define AD_STCK		19	/* sticks to you (mimic) */
#define AD_SGLD		20	/* steals gold (leppie) */
#define AD_SITM		21	/* steals item (nymphs) */
#define AD_SEDU		22	/* seduces & steals multiple items */
#define AD_TLPT		23	/* teleports you (Quantum Mech.) */
#define AD_RUST		24	/* rusts armour (Rust Monster)*/
#define AD_CONF		25	/* confuses (Umber Hulk) */
#define AD_DGST		26	/* digests opponent (trapper, etc.) */
#define AD_HEAL		27	/* heals opponent's wounds (nurse) */
#define AD_WRAP		28	/* special "stick" for eels */
#define AD_WERE		29	/* confers lycanthropy */
#define AD_DRDX		30	/* drains dexterity (quasit) */
#define AD_DRCO		31	/* drains constitution */
#define AD_DRIN		32	/* drains intelligence (mind flayer) */
#define AD_DISE		33	/* confers diseases */
#define AD_DCAY		34	/* decays organics (brown Pudding) */
#define AD_SSEX		35	/* Succubus seduction (extended) */
				/* If no SEDUCE then same as AD_SEDU */
#define AD_HALU		36	/* causes hallucination */
#define AD_DETH		37	/* for Death only */
#define AD_PEST		38	/* for Pestilence only */
#define AD_FAMN		39	/* for Famine only */
#define AD_SLIM		40	/* turns you into green slime */
#define AD_ENCH		41	/* remove enchantment (disenchanter) */
#define AD_CORR		42	/* corrode armor (black pudding) */
#define AD_POSN		43	/* poison damage */
#define AD_WISD		44	/* wisdom damage */
#define AD_VORP		45	/* vorpal strike */
#define AD_SHRD		46	/* shreads low enchantment armor, disenchants high ench armor */
#define AD_SLVR		47	/* arrows should be silver */
#define AD_BALL		48	/* arrows should be iron balls */
#define AD_BLDR		49	/* arrows should be boulders */
#define AD_VBLD		50	/* arrows should be boulders and fired in a random spread */
#define AD_TCKL		51	/* Tickle (Nightgaunts) */
#define AD_WET		52	/* Splash with water */
#define AD_LETHE	53	/* Splash with lethe water */
#define AD_BIST		54	/* yora hawk bisecting beak attack (not implemented) */
#define AD_CNCL		60	/* same effect as a wand of cancelation */
#define AD_DEAD		61	/* deadly gaze (gaze ONLY, please!) */
#define AD_SUCK		62	/* tries to suck you appart */
#define AD_MALK		63	/* tries immobalizes you and destroys wands */
#define AD_UVUU		64	/* Uvuudaum head spike attack */
#define AD_ABDC		65	/* Abduction attack, teleports you regardless of monster cancellation */
#define AD_KAOS		66  /* Spawn Chaos */
#define AD_LSEX		67	/* DEPRICATED? Lilith's seduction (extended) */
#define AD_HLBD		68  /* Asmodeus's blood */
#define AD_SPNL		69	/* Spawns Leviathan when used with AT_BOOM*/
#define AD_MIST		70 /* Mi-go mist projector.  Works with AT_GAZE */
#define AD_TELE		71 /* Monster teleports away.  Use for hit and run attacks */
#define AD_POLY		72	/* Monster alters your DNA (was for the now-defunct genetic enginier Q) */
#define AD_PSON		73 /* DEFERED psionic attacks.  uses some from cleric and wiz */
#define AD_GROW		74 /* Monster causes a child to grow up upon death.  'ton special */
#define AD_SOUL		75 /* Monster causes like monsters to power up upon death.  Deep One special */
#define AD_TENT		76	/* extended tentacle invasion (elder priest) */
#define AD_JAILER	77	/* Sets Lucifer to apear and drops third key of law when killed */
#define AD_AXUS		78 /* Multi-element counterattack, angers 'tons */
#define AD_UNKNWN	79	/* Priest of an unknown God */
#define AD_SOLR		80	/* Light Archon's silver arrow attack */
#define AD_CHKH		81 /* Chokhmah Sephirah's escalating damage attack */
#define AD_HODS		82 /* Hod Sephirah's mirror attack */
#define AD_CHRN		83 /* Nightmare's horn attack */
#define AD_LOAD		84 /* throws loadstones */
#define AD_GARO		85 /* blows up after dispensing rumor */
#define AD_GARO_MASTER	86 /* blows up after dispensing oracle */
#define AD_LVLT		87	/* level teleport (was weeping angel) */
#define AD_BLNK		88	/* mental invasion (weeping angel) */
#define AD_WEEP		89	/* Level teleport and drain (weeping angel) */
#define AD_SPOR		90	/* generate spore */
#define AD_FNEX		91	/* FerN spore EXplosion */
#define AD_SSUN		92	/* Slaver Sunflower gaze */
#define AD_MAND		93	/* Mandrake's dying shriek (kills all on level, use w/ AT_BOOM) */
#define AD_BARB		94	/* Physical damage retalitory attack */
#define AD_LUCK		95	/* Luck-draining gaze (UnNetHack) */
#define AD_VAMP		96	/* Vampire's blood drain attack */
#define AD_WEBS		97	/* Spreads webbing on a hit */
#define AD_ILUR		98 /* memory draining engulf attack belonging to Illurien */
#define AD_TNKR		99 /* Tinker attacks */
#define AD_FRWK		100 /* "Firework" explosions */
#define AD_STDY		101 /* Monster studies you, other monsters do more damage. */
#define AD_OONA		102 /* Oona's variable energy type and v and e spawning */
#define AD_NTZC		103 /* netzach sephiroth's anti-equipment attack */
#define AD_WTCH		104 /* The Watcher in the water's tentacle-spawning gaze */
#define AD_SHDW		105 /* Black Web shadow weapons */
#define AD_STTP		106 /* Steal by Teleportation: Teleports your gear away */
#define AD_HDRG		107 /* Half-dragon breath weapon */
#define AD_STAR		108 /* Tulani silver starlight rapier */
#define AD_EELC		109	/* Elemental electric */
#define AD_EFIR		110	/* Elemental fire */
#define AD_EDRC		111	/* Elemental poison (con) */
#define AD_ECLD		112	/* Elemental cold */
#define AD_EACD		113	/* Elemental acid */
#define AD_CNFT		114	/* War's conflict-inducing touch */
#define AD_BLUD		115	/* Sword of Blood inflicts heavy damage on creatures with blood */
#define AD_SURY		116	/* Surya Deva's arrow of slaying */
#define AD_NPDC		117	/* drains constitution (not poison) */
#define AD_GLSS		118	/* silver mirror shards */
#define AD_MERC		119	/* mercury blade */
#define AD_GOLD     120 /* goldify (only implemented for mammon's breath attack!) */
#define AD_ACFR     121 /* Archon fire (1x Phys, +1x Fire, +1x Holy) */
#define AD_DESC     122 /* Suck water out of the target */

//#define AD_AHAZU	116 /*  */
//Amon is a headbutt (AT_BUTT/AD_PHYS)
//Chupoclops is a poisonous bite (AT_BITE/AD_DRST)
#define AD_DUNSTAN	123
#define AD_IRIS		AD_DUNSTAN+1
#define AD_NABERIUS	AD_DUNSTAN+2
#define AD_OTIAX	AD_DUNSTAN+3
#define AD_SIMURGH	AD_DUNSTAN+4


//#define AD_VMSL		124 //Vorlon missile: elect, disintegration, blast.  Triple damage.
#define AD_CMSL		AD_SIMURGH+1 //Cold missile
#define AD_FMSL		AD_SIMURGH+2 //Fire missile
#define AD_EMSL		AD_SIMURGH+3 //Electric missile
#define AD_SMSL		AD_SIMURGH+4 //Shrapnel missile: physical

//#define AD_VTGT		129 //Vorlon targeting GAZE
#define AD_WMTG		AD_SIMURGH+5 //War machine targeting GAZE

#define AD_CLRC		AD_WMTG+1	/* random clerical spell */
#define AD_SPEL		AD_CLRC+1	/* random magic spell */
#define AD_RBRE		AD_CLRC+2	/* random breath weapon */
#define AD_RGAZ		AD_CLRC+3	/* random gaze attack */
#define AD_RETR		AD_CLRC+4 /* elemental gaze attack */

#define AD_SAMU		AD_CLRC+5	/* hits, may steal Amulet (Wizard) */
#define AD_CURS		AD_CLRC+6	/* random curse (ex. gremlin) */
#define AD_SQUE		AD_CLRC+7	/* hits, may steal Quest Art or Amulet (Nemeses) */


/*
 *  Monster to monster attacks.  When a monster attacks another (mattackm),
 *  any or all of the following can be returned.  See mattackm() for more
 *  details.
 */
#define MM_MISS		0x0	/* aggressor missed */
#define MM_HIT		0x1	/* aggressor hit defender */
#define MM_DEF_DIED	0x2	/* defender died */
#define MM_AGR_DIED	0x4	/* aggressor died */

#endif /* MONATTK_H */
