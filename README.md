dNetHack
========
A variant of nethack 3.4.4

In addition to my original stuff, there is a whole mess of code that was appropriated from other nethack mods; Slash�em, L, Malcolm Ryan, James, Patric Muller, Pasi Kallinen, Nephi, Bulwersator, Karl Garrison, probably others (wasn�t always good about documenting who�s code I was stealing...). New tiles for windowed mode are based mostly on existing nethack tiles and tiles from SLASH'EM. The tile indicating giant turtles was adapted from Legend of Zelda, a Link to the Past. The Chokhmah Sephirah is based on the Recoudut, an epic inevitable posted to some message board by someone, years ago, somewhere out there on the internet. That original source has long since vanished. The other Keter Sephiroth are based on the DnD inevitables.

Implemented YANI's from rec.games.roguelike.nethack and the nethack wiki, including several of Jonadab the Unsightly One's suggestions.

dNethack does three main things: it greatly increases the number of artifacts, implements new monsters, and re-organizes and expands the dungeon. A few changes have also been made to existing classes and items. Most new artifacts have custom effects and/or invocations. Most new monsters have code support beyond simply an entry in the monster list. Dungeon changes include: multi-level alignment �quest� branches, additional side branches off the main dungeon, shortened gehennom, demon lairs randomly drawn from a small pool, fewer straight-up maze levels. Characters of any alignment may enter any alignment �quest.� To complete the game, you must explore one quest thoroughly or multiple quests partially. Quests may be entered at any time, and start easy, although the difficulty quickly increases beyond what a beginning character can handle.

Overall, dnethack is more difficult than vanilla nethack, as there are more ways to get into trouble as a late-game character, and several ways to go from ascension-ready character to tombstone in just a few rounds. The design goal is for all such threats to be counterable, so that the late game is more difficult and dangerous but not randomly lethal.


(...Two major sources of inspiration: Dungeons and Dragons (DnD) and Cthulhu Mythos) 
____________________________
__Player Character Changes__

-Elbereth removed

--New system using warding signs implemented instead.

-----Wards are learned from spellbooks, engraved magic rings, prayer, and sometimes the ground.

-----Wards are engraved using the regular engrave command (E), or Ctrl^q.

-----Squares may contain one type of ward AND one text engraving (as well as any number of staves, see below).

-----Wards and engravings may be of different types (ie, you can have a burned ward and a dust-engraved message in the same square)

-----Altering the ward or the engraving obscures the ward, making you vulnerable until you finish the alteration.

--A suplementary system of carved wards has also been implemented.

-----Islandic magical staves are warding signs that can be carved into wooden weapons.

-----Carve staves by applying (a) a knife.

-----Use staves by dropping them at your feet. Squares may contain any number of stave-carved weapons as well as a text engraving and a warding sign. 

-----Some staves grant other abilities when a weapon carved with one is wielded.

-Barbarians can reach expert in Broadsword and Trident

-Merged in the Convict patch.

--Convicts will find they are unable to raise their alignment record as fast as other roles can.

--Convicts will find a slightly altered first gift

--Convicts will find they have trouble keeping their luck up in the later stages of the game.

-Knights can reach expert in polearms but only basic in two-weapon fighting.

-Monks recive extra AC if they aren't wearing body armor. Dependent on dexterity, wisdom, and level.

-Priests get special treatment w/ regards to artifacts

-----Can attack more effectively with artifacts

-----The probability of receiving an artifact after sacrificing is based only on prior gifts given, not on found artifacts.

-Priests and Knights can expend 30 energy to turn undead in 1 turn instead of 5 turns.

-Samurai can reach skilled in polearms, and gain a +1 skill-level bonus when using naginatas (glaives)

-Samurai, Archeologist, and Knight special alignment penalties made harsher.

-Tourist quest monsters revised to better reflect Ank-Morpork

-Valks can reach expert in spear but only skilled in longsword

-Valks start with a +1 spear instead of a +1 longsword (as in Slash'em).

-Valks start with leather armor and +1 small shields

-Gnomes begin with Gnomish hats and a small pile of tallow candles

-Ability Score drop via abuse can no longer be cured with unicorn horns.

-Changed how gaining intrinsic resistances from food works

--You have a shot at all resistances a corpse can give, not just a randomly chosen one.

----Some corpses grant temporary intrinsic displacement as well.

--Fire, Sleep, Cold, Acid, and Electricity resistances gained from corpses time out.

--Resistances received from leveling up and from crowning do not time out.

-"Fixed" a few dietary violations of common sense, so watch what you eat!

-Tweaked skills slightly

--Expert gives +5 to-hit and +3 damage (-2 to hit and +2 damage for twoweapon fighting)

--Expert riding gives +2 to-hit and +5 damage


(...)
_____________________________
__Changes to Existing Items__

Tweaked armor

-Crystal plate mail is very heavy but allows extremely low AC.  It is MC2, can be enchanted to +14, and enchants at twice the normal rate.

-Plate mails of other varieties give MC3 and low AC. All plate mails can be enchanted to +7.

-Robes are MC2.

-Mithril is very light.

-Dragon scale mail:

---All dragon scale is heavier

---All dragon scale mail other than silver and grey enhances your attacks as well as your defenses

-Dexterity contributes to AC.  Dex bonuses are not cumulative with body armor base AC, but penalties are.

---special exception: dex bonus is cumulative with leather jackets and mithril mail.

Tweaked tools

-Blessed crystal balls are easier to use

Some items get special bonuses based on appearance

-Engagement ring blocks seduction attacks while worn.

-Iron Boots reduce being pushed around by currents underwater. They're heavy.

-Combat boots grant extra AC and attack

-Mud boots make you more resistant to wrap attacks

-Hiking boots grant a small carrying cap bonus

-Buckled boots resist being sucked off by... well, monsters which can suck off boots.

-Jungle boots prevent many types of leg injury

-old gloves can't be eroded

-padded gloves +1 ac

-fencing gloves +2 attack with one handed weapon and no shield



(...Dungeons and Extra-Terrestrial hack)
________________________________
__Changes to Existing Monsters__

-Trappers changed color, to brown t

----there is a new monster identified by the dark green t

-Vampire bats are now gray. Ravens are still black.

-Master liches are now orange, and arch-liches are bright magenta.

----There is no plain magenta Lich.

-Removed Keystone Kops

-Nurse behavior altered. Hostile nurses will heal enemies. Tame nurses will heal you.


(...DnEThack. dnethack.)
___________________________
__Changes to Existing Artifacts__

BANNED wishing for quest artifacts

Only your quest Nemeses will deliberately steal your quest artifact, other monsters may steal the amulet and invocation items. The quest artifact may of course be randomly stolen by nymphs and the like.

	-Makes QA a dependable source of extrinsics, deepening differences between classes.
	
-Foo-Bane artifacts are better (don�t need to be enchanted to be effective), and can be wielded in the off-hand behind a second artifact.


(ha. ha.)
______________
__Known Bugs__

Improved AI patch seems to sometimes cause monsters to switch back and forth between ranged and melee weapons if you are 1 square away.

SPFX_RAM: if the thrown target goes over a land mine and is killed, it gives an error message?

Messed up some compile-time options. The game will not compile without TOURIST set, probably others.

Memmory draining attacks will attempt to remove the memmory of objects or levels that the character doesn't know. This seems to be harmless. As such I have commented out the error message.

Not bugs so much as incomplete features:

-Most new monster attacks are coded solely for monster vs. player, not monster vs. monster or player vs. monster.

-Many things that should have database entries don't.

I hang around on rec.games.roguelikes.nethack
