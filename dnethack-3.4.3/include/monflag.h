/*	SCCS Id: @(#)monflag.h	3.4	1996/05/04	*/
/* Copyright (c) 1989 Mike Threepoint				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MONFLAG_H
#define MONFLAG_H

#define MS_SILENT	0	/* makes no sound */
#define MS_BARK		1	/* if full moon, may howl */
#define MS_MEW		2	/* mews or hisses */
#define MS_ROAR		3	/* roars */
#define MS_GROWL	4	/* growls */
#define MS_SQEEK	5	/* squeaks, as a rodent */
#define MS_SQAWK	6	/* squawks, as a bird */
#define MS_HISS		7	/* hisses */
#define MS_BUZZ		8	/* buzzes (killer bee) */
#define MS_GRUNT	9	/* grunts (or speaks own language) */
#define MS_NEIGH	10	/* neighs, as an equine */
#define MS_WAIL		11	/* wails, as a tortured soul */
#define MS_GURGLE	12	/* gurgles, as liquid or through saliva */
#define MS_BURBLE	13	/* burbles (jabberwock) */
#define MS_JUBJUB	14	/* jubjub bird's fear inducing call */
#define MS_ANIMAL	14	/* up to here are animal noises */ //note: checks are <=, so this should be == the last animal sound
#define MS_SHRIEK	16	/* wakes up others */
#define MS_BONES	17	/* rattles bones (skeleton) */
#define MS_LAUGH	18	/* grins, smiles, giggles, and laughs */
#define MS_MUMBLE	19	/* says something or other */
#define MS_IMITATE	20	/* imitates others (leocrotta) */
#define MS_ORC		MS_GRUNT	/* intelligent brutes */
#define MS_HUMANOID	21	/* generic traveling companion */
#define MS_ARREST	22	/* "Stop in the name of the law!" (was Keystones) */
#define MS_SOLDIER	23	/* army and watchmen expressions */
#define MS_GUARD	24	/* "Please drop that gold and follow me." */
#define MS_DJINNI	25	/* "Thank you for freeing me!" */
#define MS_NURSE	26	/* "Take off your shirt, please." */
#define MS_SEDUCE	27	/* "Hello, sailor." (Nymphs) */
#define MS_VAMPIRE	28	/* vampiric seduction, Vlad's exclamations */
#define MS_BRIBE	29	/* asks for money, or berates you */
#define MS_CUSS		30	/* berates (demons) or intimidates (Wiz) */
#define MS_RIDER	31	/* astral level special monsters */
#define MS_LEADER	32	/* your class leader */
#define MS_NEMESIS	33	/* your nemesis */
#define MS_GUARDIAN	34	/* your leader's guards */
#define MS_SELL		35	/* demand payment, complain about shoplifters */
#define MS_ORACLE	36	/* do a consultation */
#define MS_PRIEST	37	/* ask for contribution; do cleansing */
#define MS_SPELL	38	/* spellcaster not matching any of the above */
#define MS_WERE		39	/* lycanthrope in human form */
#define MS_BOAST	40	/* giants */
#define MS_DREAD	41	/* song of the dread seraphim */
#define MS_OONA		42	/* song of Oona */
#define MS_SONG		43	/* other singers */
#define MS_FLOWER	44	/* black flower's songs */


#define MR_FIRE		0x001	/* 1 resists fire */
#define MR_COLD		0x002	/* 2 resists cold */
#define MR_SLEEP	0x004	/* 4 resists sleep */
#define MR_DISINT	0x008	/* 8 resists disintegration */
#define MR_ELEC		0x010	/* 16 resists electricity */
#define MR_POISON	0x020	/* 32 resists poison */
#define MR_ACID		0x040	/* 64 resists acid */
#define MR_STONE	0x080	/* 128 resists petrification */
#define MR_DRAIN	0x100	/* 256 resists level drain */
#define MR_SICK		0x200	/* resists sickness */
#define MR_ALL		(MR_STONE|MR_ACID|MR_POISON|MR_ELEC|MR_DISINT|MR_SLEEP|MR_COLD|MR_FIRE|MR_DRAIN|MR_SICK)
/* other resistances: magic, sickness */
/* other conveyances: teleport, teleport control, telepathy */

/* individual resistances */
//#define MR2_SEE_INVIS	0x0100	/* slot taken by MR_DRAIN see invisible */
//#define MR2_LEVITATE	0x0200	/* slot taken by MR_SICK levitation */
#define MR2_WATERWALK	0x0400	/* water walking */
#define MR2_MAGBREATH	0x0800	/* magical breathing */
#define MR2_DISPLACED	0x1000	/* displaced */
#define MR2_STRENGTH	0x2000	/* gauntlets of power */
#define MR2_FUMBLING	0x4000	/* clumsy */

//MM_ Monster Motility
#define MM_FLY			0x00000001L	/* can fly or float */
#define MM_SWIM			0x00000002L	/* can traverse water */
#define MM_AMORPHOUS	0x00000004L	/* can flow under doors */
#define MM_WALLWALK		0x00000008L	/* can phase thru rock */
#define MM_CLING		0x00000010L	/* can cling to ceiling */
#define MM_TUNNEL		0x00000020L	/* can tunnel thru rock */
#define MM_NEEDPICK		0x00000040L	/* needs pick to tunnel */
#define MM_AMPHIBIOUS	0x00000080L	/* can survive underwater */
#define MM_BREATHLESS	0x00000100L	/* doesn't need to breathe */
#define MM_TPORT		0x00000200L	/* can teleport */
#define MM_TPORT_CNTRL	0x00000400L	/* controls where it teleports to */
#define MM_TENGTPORT	0x00000800L	/* Monster teleports as Tengu */
#define MM_STATIONARY	0x00001000L	/* does not move. */
#define MM_FLOAT		0x00002000L	/* Monster floats instead of truely flying (modifier) */
#define MM_NOTONL		0x00004000L	/* Tries to avoid being on same line as player */
#define MM_FLEETFLEE	0x00008000L	/* Flees if adjacent if it thinks it can outrun you */

//Monster Thoughts and behavior
#define MT_CONCEAL		0x00000001L	/* hides under objects */
#define MT_HIDE			0x00000002L	/* mimics, blends in with ceiling */
#define MT_MINDLESS		0x00000004L	/* has no mind--golem, zombie, mold */
#define MT_ANIMAL		0x00000008L	/* has animal mind */
#define MT_CARNIVORE	0x00000010L	/* eats corpses */
#define MT_HERBIVORE	0x00000020L	/* eats fruits */
#define MT_HOSTILE		0x00000040L	/* always starts hostile */
#define MT_PEACEFUL		0x00000080L	/* always starts peaceful */
#define MT_DOMESTIC		0x00000100L	/* can be tamed by feeding */
#define MT_WANDER		0x00000200L	/* wanders randomly */
#define MT_STALK		0x00000400L	/* follows you to other levels */
#define MT_ROCKTHROW	0x00000800L	/* throws boulders */
#define MT_GREEDY		0x00001000L	/* likes gold */
#define MT_JEWELS		0x00002000L	/* likes gems */
#define MT_COLLECT		0x00004000L	/* picks up weapons and food */
#define MT_MAGIC		0x00008000L	/* picks up magic items */
#define MT_WANTSAMUL	0x00010000L	/* would like to steal the amulet */
#define MT_WANTSBELL	0x00020000L	/* wants the bell */
#define MT_WANTSBOOK	0x00040000L	/* wants the book */
#define MT_WANTSCAND	0x00080000L	/* wants the candelabrum */
#define MT_WANTSARTI	0x00100000L	/* wants the quest artifact */
#define MT_WAITFORU		0x00200000L	/* waits to see you or get attacked */
#define MT_CLOSE		0x00400000L	/* lets you close unless attacked */
#define MT_TRAITOR		0x00800000L	/* slash'em tag. */
#define MT_NOTAKE		0x01000000L	/* doesn't pick up items. */
#define MT_METALLIVORE	0x02000000L	/* eats metal. */

#define MT_MAGIVORE		0x04000000L	/* eats magic */

#define MT_OMNIVORE		(MT_CARNIVORE|MT_HERBIVORE)	/* eats both */
#define MT_MAID		(MT_MAGIC|MT_COLLECT|MT_JEWELS|MT_GREEDY)	/* tiddies up the dungeon */
#define MT_WANTSALL		(MT_WANTSAMUL|MT_WANTSBELL|MT_WANTSBOOK|MT_WANTSCAND|MT_WANTSARTI)	/* wants any major artifact */
#define MT_COVETOUS		MT_WANTSALL		/* wants something */
#define MT_WAITMASK		(MT_WAITFORU|MT_CLOSE)	/* waiting... */


//Monster Body plan
#define MB_NOEYES		0x00000001L	/* no eyes to gaze into or blind */
#define MB_NOHANDS		0x00000002L	/* no hands to handle things */
#define MB_NOLIMBS		0x00000004L	/* no arms/legs to kick/wear on */
#define MB_NOHEAD		0x00000008L	/* no head to behead */
#define MB_HUMANOID		0x00000010L	/* has humanoid head/arms/torso */
#define MB_ANIMAL		0x00000020L	/* has animal body */
#define MB_SLITHY		0x00000040L	/* has serpent body */
#define MB_UNSOLID		0x00000080L	/* has no solid or liquid body */
#define MB_THICK_HIDE	0x00000100L	/* has thick hide or scales */
#define MB_OVIPAROUS	0x00000200L	/* can lay eggs */
#define MB_ACID			0x00000400L	/* acidic to eat */
#define MB_POIS			0x00000800L	/* poisonous to eat */
#define MB_CHILL		0x00001000L		/* cold to eat */
#define MB_TOSTY		0x00002000L		/* hot to eat */
#define MB_MALE			0x00004000L	/* always male */
#define MB_FEMALE		0x00008000L	/* always female */
#define MB_NEUTER		0x00010000L	/* neither male nor female */
#define MB_STRONG		0x00020000L	/* strong (or big) monster */
#define MB_WINGS		0x00040000L	/* has wings */
#define MB_LONGHEAD		0x00080000L	/* has 'long' (animal) head */
#define MB_LONGNECK		0x00100000L	/* has 'long' (snakelike) head and neck */

#define MB_SNAKELEG	(MB_HUMANOID|MB_SLITHY)
#define MB_CENTAUR	(MB_HUMANOID|MB_ANIMAL)

#define MB_BODYTYPEMASK	(MB_HUMANOID|MB_ANIMAL|MB_SLITHY)
#define MB_HEADMODIMASK	(MB_LONGHEAD|MB_LONGNECK)



//Monster Vision types and other sensorium details
#define MV_NORMAL		0x00000001L		/* can't see more than 1 square in the dark */
#define MV_INFRAVISION	0x00000002L		/* has infravision (does not show squares) */
#define MV_DARKSIGHT	0x00000004L		/* sees in the dark, blinded by light */
#define MV_LOWLIGHT		0x00000008L		/* sees farther in the dark, defaults to 2 with the possibility of special casing */
#define MV_CATSIGHT		0x00000010L		/* sees in the dark, sees in the light, not both at once */
#define MV_ECHOLOCATE	0x00800020L		/* sees via sound, in both light and dark, blinded and stunned by loud noises */
#define MV_BLOODSENSE	0x00000040L		/* detect monsters by seeing their blood (does not show squares) */
#define MV_LIFESENSE	0x00000080L		/* detect monsters by sensing their life-force (does not show squares) */
#define MV_EXTRAMISSION	0x00000100L		/* see perfectly in light and dark */
//define MV_BLINDSIGHT	0x00000000L		/* see without eyes OBSOLETE*/
#define MV_TELEPATHIC	0x00000200L		/* Monster is telepathic, detects any non-mindless creatures on the level (does not show squares) */
#define MV_RLYEHIAN		0x00000400L		/* Cannot see into or through water squares. Blind in water */
#define MV_SEE_INVIS	0x00000800L		/* can see invisible creatures (modifier) */
#define MV_DETECTION	0x00001000L		/* detect monsters in line-of-sight (does not show squares) */
#define MV_OMNI			0x00002000L		/* non-square-sight does not require line-of-sight */

//Monster Game mechanics and bookkeeping
#define MG_REGEN		0x00000001L	/* regenerates hit points */
#define MG_NOPOLY		0x00000002L	/* players mayn't poly into one */
#define MG_MERC			0x00000004L	/* is a guard or soldier */
#define MG_PNAME		0x00000008L	/* monster name is a proper name */
#define MG_LORD			0x00000010L	/* is a lord to its kind */
#define MG_PRINCE		0x00000020L	/* is an overlord to its kind */
#define MG_NASTY		0x00000040L	/* extra-nasty monster (more xp) */
#define MG_INFRAVISIBLE	0x00000080L	/* visible by infravision */
#define MG_OPAQUE		0x00000100L	/* Monster blocks line of sight */
#define MG_DISPLACEMENT	0x00000200L	/* Monster has displacement */
#define MG_HATESSILVER	0x00000400L	/* Monster hates silver */
#define MG_HATESIRON	0x00000800L	/* Monster hates iron/steel */
#define MG_HATESUNHOLY	0x00001000L	/* Monster hates cursed objects */
#define MG_RIDER		0x00002000L	/* Monster has perfect resurection */
#define MG_DEADLY		0x00004000L	/* Monster's corpse is deadly to taste */

//Monster rAce
#define MA_UNDEAD		0x00000001L	/* is walking dead */
#define MA_WERE			0x00000002L	/* is a lycanthrope */
#define MA_HUMAN		0x00000004L	/* is a human */
#define MA_ELF			0x00000008L	/* is an elf */
#define MA_DROW			0x00000010L	/* is a drow */
#define MA_DWARF		0x00000020L	/* is a dwarf */
#define MA_GNOME		0x00000040L	/* is a gnome */
#define MA_ORC			0x00000080L	/* is an orc */
#define MA_VAMPIRE		0x00000100L	/* is a vampire */
#define MA_CLOCK		0x00000200L	/* is a clockwork automaton */
#define MA_UNLIVING		0x00000400L	/* isn't alive */
#define MA_PLANT		0x00000800L	/* is a plant (or a fungus.  Which is SORTA a plant) */
#define MA_GIANT		0x00001000L	/* is a giant */
#define MA_INSECTOID	0x00002000L	/* is an insectoid creature */
#define MA_ARACHNID		0x00004000L	/* is an insectoid creature */
#define MA_AVIAN		0x00008000L	/* is a bird-like-creature */
#define MA_REPTILIAN	0x00010000L	/* is a reptilian creature */
#define MA_ANIMAL		0x00020000L	/* is a more-or-less mundane animal (or a hybrid of an animal and something else) */
#define MA_AQUATIC		0x00040000L	/* is a water-dwelling creature */
#define MA_DEMIHUMAN	0x00080000L	/* is a humanoid with animal traits */
#define MA_FEY			0x00100000L	/* is a member of the fair folk */
#define MA_ELEMENTAL	0x00200000L	/* is an elemental being */
#define MA_DRAGON		0x00400000L	/* is a dragon */
#define MA_DEMON		0x00800000L	/* is a demon */
#define MA_MINION		0x01000000L	/* is a minion of a deity */
#define MA_PRIMORDIAL	0x02000000L	/* is an ancient race */
#define MA_ET			0x04000000L	/* is an alien race */

#define MZ_TINY		0		/* < 2' */
#define MZ_SMALL	1		/* 2-4' */
#define MZ_MEDIUM	2		/* 4-7' */
#define MZ_HUMAN	MZ_MEDIUM	/* human-sized */
#define MZ_LARGE	3		/* 7-12' */
#define MZ_HUGE		4		/* 12-25' */
#define MZ_GIGANTIC	7		/* off the scale */

/* for mons[].geno (constant during game) */
#define G_UNIQ		0x2000		/* generated only once */
#define G_PLANES	0x1000		/* generated only in planes */
#define G_NOHELL	0x0800		/* not generated in "hell" */
#define G_HELL		0x0400		/* generated only in "hell" */
#define G_NOGEN		0x0200		/* generated only specially */
#define G_SGROUP	0x0080		/* appear in small groups normally */
#define G_LGROUP	0x0040		/* appear in large groups normally */
#define G_GENO		0x0020		/* can be genocided */
#define G_NOCORPSE	0x0010		/* no corpse left ever */
#define G_FREQ		0x0007		/* creation frequency mask */

/* for mvitals[].mvflags (variant during game), along with G_NOCORPSE */
#define G_KNOWN		0x0004		/* have been encountered */
#define G_GONE		(G_GENOD|G_EXTINCT)
#define G_GENOD		0x0002		/* have been genocided */
#define G_EXTINCT	0x0001		/* have been extinguished as
					   population control */
#define MV_KNOWS_EGG	0x0008		/* player recognizes egg of this
					   monster type */

#endif /* MONFLAG_H */
