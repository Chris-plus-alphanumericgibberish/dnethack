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
#define AT_GAZE		15	/* gaze - ranged */
#define AT_TENT		16	/* tentacles */
#define AT_ARRW		17	/* fire silver arrows from internal reservour.  Other ammo created as needed. */
#define AT_WHIP		18	/* Whips you */
#define AT_LRCH		19	/* Reach attack */
#define AT_HODS		20  /* Hod Sephirah's mirror attack */
#define AT_LNCK		21  /* Bite attack with reach */
#define AT_MMGC		22	/* uses magic spell(s), but don't allow player spellcasting (Monster-only MaGiC) */
#define AT_ILUR		23	/* Two stage swallow attack, currently belongs to Illurien only */

#define AT_WEAP		254	/* uses weapon */
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
#define AD_MALK		63	/* tries immobalizes you and destroys wands and rings */
#define AD_UVUU		64	/* Uvuudaum head spike attack */
#define AD_ABDC		65	/* Abduction attack, teleports you regardless of monster cancellation */
#define AD_KAOS		96  /* Spawn Chaos */
#define AD_LSEX		97	/* DEPRICATED? Lilith's seduction (extended) */
#define AD_HLBD		99  /* Asmodeus's blood */
#define AD_SPNL		100	/* Spawns Leviathan when used with AT_BOOM*/
#define AD_MIST		110 /* Mi-go mist projector.  Works with AT_GAZE */
#define AD_TELE		111 /* Monster teleports away.  Use for hit and run attacks */
#define AD_POLY		112	/* Monster alters your DNA (was for the now-defunct genetic enginier Q) */
#define AD_PSON		113 /* DEFERED psionic attacks.  uses some from cleric and wiz */
#define AD_GROW		114 /* Monster causes a child to grow up upon death.  'ton special */
#define AD_SOUL		115 /* Monster causes like monsters to power up upon death.  Deep One special */
#define AD_TENT		116	/* extended tentacle invasion (elder priest) */
#define AD_JAILER	117	/* Sets Lucifer to apear and drops third key of law when killed */
#define AD_AXUS		118 /* Multi-element counterattack, angers 'tons */
#define AD_UNKNWN	119	/* Priest of an unknown God */
#define AD_SOLR		120	/* Solar's silver arrow attack */
#define AD_CHKH		121 /* Chokhmah Sephirah's escalating damage attack */
#define AD_HODS		122 /* Hod Sephirah's mirror attack */
#define AD_CHRN		123 /* Nightmare's horn attack */
#define AD_LOAD		124 /* throws loadstones */
#define AD_GARO		125 /* blows up after dispensing rumor */
#define AD_GARO_MASTER	126 /* blows up after dispensing oracle */
#define AD_LVLT		127	/* level teleport (was weeping angel) */
#define AD_BLNK		128	/* mental invasion (weeping angel) */
#define AD_WEEP		129	/* Level teleport and drain (weeping angel) */
#define AD_SPOR		130	/* generate spore */
#define AD_FNEX		131	/* FerN spore EXplosion */
#define AD_SSUN		132	/* Slaver Sunflower gaze */
#define AD_MAND		133	/* Mandrake's dying shriek (kills all on level, use w/ AT_BOOM) */
#define AD_BARB		134	/* Physical damage retalitory attack */
#define AD_LUCK		135	/* Luck-draining gaze (UnNetHack) */
#define AD_VAMP		136	/* Vampire's blood drain attack */
#define AD_WEBS		137	/* Vampire's blood drain attack */
#define AD_ILUR		138 /* memory draining engulf attack belonging to Illurien */


#define AD_AHAZU	150 /*  */
#define AD_AMON		151 /*  */


//#define AD_VMSL		180 //Vorlon missile: elect, disintegration, blast.  Triple damage.
#define AD_CMSL		181 //Cold missile
#define AD_FMSL		182 //Fire missile
#define AD_EMSL		183 //Electric missile
#define AD_SMSL		184 //Shrapnel missile: physical

//#define AD_VTGT		200 //Vorlon targeting GAZE
#define AD_WMTG		201 //War machine targeting GAZE

#define AD_CLRC		240	/* random clerical spell */
#define AD_SPEL		241	/* random magic spell */
#define AD_RBRE		242	/* random breath weapon */
#define AD_RGAZ		242	/* random gaze attack */
#define AD_RETR		243 /* elemental gaze attack */

#define AD_SAMU		252	/* hits, may steal Amulet (Wizard) */
#define AD_CURS		253	/* random curse (ex. gremlin) */
#define AD_SQUE		254	/* hits, may steal Quest Art or Amulet (Nemeses) */


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
