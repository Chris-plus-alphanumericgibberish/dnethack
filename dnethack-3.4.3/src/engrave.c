/*	SCCS Id: @(#)engrave.c	3.4	2001/11/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include	"hack.h"
#include	"lev.h"
#include	"artifact.h"
#include <ctype.h>

#define ENGRAVE_MODE 1
#define WARD_MODE 2
#define SEAL_MODE 3

STATIC_VAR NEARDATA struct engr *head_engr;

#ifdef OVLB
/* random engravings */
static const char *random_mesg[] = {
	"Elbereth",
	/* trap engravings */
	"Vlad was here", "ad aerarium",
	/* take-offs and other famous engravings */
	"Owlbreath", "Galadriel",
	"Kilroy was here",
	"quis custodiet ipsos custodes?", /* Watchmen */
	"Bad Wolf", /* Dr. Who */
	"A.S. ->", "<- A.S.", /* Journey to the Center of the Earth */
	"You won't get it up the steps", /* Adventure */
	"Lasciate ogni speranza o voi ch'entrate.", /* Inferno */
	"Well Come", /* Prisoner */
	"We apologize for the inconvenience.", /* So Long... */
	"See you next Wednesday", /* Thriller */
	"notary sojak", /* Smokey Stover */
	"For a good time call 8?7-5309",
	"Please don't feed the animals.", /* Various zoos around the world */
	"Madam, in Eden, I'm Adam.", /* A palindrome */
	"Two thumbs up!", /* Siskel & Ebert */
	"Hello, World!", /* The First C Program */
	"Dare mighty things.", /* Moto of the Mars Science Laboratory */
	"are we cool yet?", /* SCP Foundation */
	"I seem to be having tremendous difficulty with my lifestyle", /* Hitchhiker's Guide to the Galaxy */
	"The flow of time itself is convoluted, with heroes centuries old phasing in and out.", /* Dark Souls */
#ifdef MAIL
	"You've got mail!", /* AOL */
#endif
	"As if!", /* Clueless */
	"Arooo!  Werewolves of Yendor!", /* gang tag */
	"Dig for Victory here", /* pun, duh */
	"Don't go this way",
	"Gaius Julius Primigenius was here.  Why are you late?", /* pompeii */
	"Go left --->",	"<--- Go right",
	"Haermund Hardaxe carved these runes", /* viking graffiti */
	"Here be dragons",
	"Need a light?  Come visit the Minetown branch of Izchak's Lighting Store!",
	"There was a hole here.  It's gone now.",
	"The Vibrating Square",
	"This is a pit!",
	"Watch out, there's a gnome with a wand of death behind that door!",
	"X marks the spot",
	"X <--- You are here.",
	"Who are you?", "What do you want?", "Why are you here?", "Where are you going?", /*Babylon 5*/
	/*weeping angels, Unnethack*/
	"Beware of the weeping angels", /* Doctor Who */
	"Duck, Sally Sparrow", /* Doctor Who */
	
	"4 8 15 16 23 42", /* Lost */
	
	"This too shall pass.", /* medieval Persian adage */
	
	"Romanes Eunt Domus", /* Life of Brian */
	
	"stth was here", /* First ascender */
	"allihaveismymind followed stth", /* Second ascender */
	"Khor followed allihaveismymind", /* Third ascender */
	"ChrisANG followed Khor", /* Fourth ascender */
	"FIQ followed ChrisANG", /* Fifth ascender */
	"Tariru followed FIQ", /* Sixth ascender */
	"VoiceOfReason followed Tariru", /* Seventh ascender */
	"Catullus followed VoiceOfReason", /* Eighth ascender */
	"HBane followed Catullus", /* Ninth ascender */
	
	/* contributed by members of NetHackWiki */
	"Write down the coordinates, he said.", /* the Submachine series */
		"...look for a green leaf...",
		"...bring it to the statue...",
		"...you should arrive at the lab...",
		"...or somewhere nearby...", 
};

static const char *haluMesg[] = {
	"This is not the dungeon you are looking for.",
	"Save now, and do your homework!",
	"Snakes on the Astral Plane - Soon in a dungeon near you",
	"This square deliberately left blank.",
	"Warning, Exploding runes!",
	"You are the one millionth visitor to this place!  Please wait 200 turns for your wand of wishing.",
	
	"@ <--- You are here.",
/* contributed by members of NetHackWiki */
	"Please disable the following security protocols: 2-18, 1-12, 1-0", /* the Submachine series */
		"Our coil is broken, and we don't have a replacement. It's useless. It's so useless.", 
	
/* contributed by members of the Nethack Usenet group, rec.games.roguelike.nethack */
	"Which way did he go, George, which way did he go?",
	"Until you stalk and overrun, you can't devour anyone.",
	"Fee.  Fie.  Foe.  Foo.",
	"Violence is the last refuge of the incompetent.",
	"Be vewy vewy quiet.  I'm hunting wabbits.",
	"Eh, what's up, doc?",
	"We prefer your extinction to the loss of our job.",
	"You're weird, sir.",
	"I do not know for what reason the enemy is pursuing you, but I perceive that he is, strange indeed though that seems to me.",
	"When it is falling on your head, then you are knowing it is a rock.",
	"The very best mail is made from corbomite.",
	"Woah, duude, look at all the colors.",
	"I'm bringing home a baby bumblebee.",
	"Now, if you'll permit me, I'll continue my character assassination unimpeded.",
	"Cry havoc, and let slip the dogs of war.",
	"Virtue needs some cheaper thrills.",
	"I just saw Absolom hanging in an oak tree!",
	"And I may do anything I wish as long as I squeeze?",
	"Marsey Dotesin Dosey Dotesin Diddley Damsey Divey.",
	"I knew I should have taken that left turn at Albuquerque.",
	"YOU BROKE MY SWORD!",
	"An atom-blaster is a good weapon, but it can point both ways.",
	"You may have already won ten million zorkmids.",
	"They say the satisfaction of teaching makes up for the lousy pay.",
	"Tharr she blows!",
	"Perhaps today is a good day to die.",
	"Sufferin' succotash.",
	"To succeed, planning alone is insufficient. One must improvise as well.",
	"I love it when a plan comes together.",
	"Very fond of rumpots, crackpots, and how are you Mr. Wilson?",
	"No matter how valuable it might be, any man's life is still worth more than any animal's.",
	"No clock for this.  Flyin' now.",
	"She's fast enough for you, old man. What's the cargo?",
	"There're few who haven't heard the name of Gurney Halleck.",
	"Do you like mayonnaise and corn on your pizza?",
	"I am that merry wanderer of the night.",
	"Don't try any games.  Remember I can see your face if you can't see mine.",
	"Ah hates rabbits.",
	"Captain, there be whales here!",
	"Mimesis is a plant.  Go Bucks!",
	"That's all we live for, isn't it?  For pleasure, for titillation?",
	"The only brew for the brave and true comes from the Green Dragon.",
	"There is an old Vulcan proverb: only Nixon could go to China.",
	"Yo, put me in a trebuchet and launch me to the stratosphere.",
	"Lorem ipsum dolor sit amet, consectetur adipisici elit, sed eiusmod tempor incidunt ut labore et dolore magna aliqua.",
	"Sola lingua bona lingua mortua est.",
	"Mairzy Doats And Dozy Doats And Liddle Lamzy Divey A kiddley divey too, wouldn't you?",
	"Madam, I may be drunk, but you are ugly, and in the morning I shall be sober.",
	"Outside of a dog, a book is a man's best friend.",
		"Inside of a dog, it's too dark to read." ,
	"You are superior in only one aspect. You are better at dying.",
	"No McFly ever amounted to anything in the history of Hill Valley!",
	"You've got to ask yourself one question: 'Do I feel lucky?' Well, do ya, punk?",
	"In this pearl it is hard to see a Strange Loop.",
	"That is because the Strange Loop is buried in the oyster -- the proof.",
	"I love the smell of napalm in the morning.",
	"Et tu, Brute?",
	"The conditions were difficult. Several contestants were maimed, but... I was triumphant. I won Champion Standing.",
	"We're not gonna take it. No, we ain't gonna take it. We're not gonna take it anymore.",
	"Absurd!  Your 'MU' is as silly as a cow's moo.",
	"Hasta la vista, baby.",
	"I see...  I see...  I see a lavender man...  bearing tidings.",
	"It is a sign of creative genius to reconcile the seemingly disparate.",
	"It pays to be obvious, especially if you have a reputation for subtlety.",
	"Bother. I've left my new torch in Narnia",
	"If you put into the machine wrong figures, will the right answers come out?",
		"I am not able rightly to apprehend the kind of confusion of ideas that could provoke such a question.",
	"Colorless green ideas sleep furiously",
	"We fear a few dwarf watercraft stewardesses were regarded as desegregated after a great Texas tweezer war.",
	"I'll kill him in Honolulu, you unhook my pink kimono.",
	"The clown can stay, but the Ferengi in the gorilla suit has got to go.",
	"All roads lead to Trantor, and that is where all stars end.",
	"It is a far, far better thing I do than I have ever done. It is a far, far better rest I go to than I have ever known.",
	"Yes, this is Network 23. The net-net-net-net-network that puts it right... where they want you to be!",
	"You're listening to the E.I.B Radio Network.",
	"We shall this day light such a candle, by God's grace, in England, as I trust shall never be put out.",
	"How about a little fire, scarecrow?",
	"And if you're the kind of person who parties with a bathtub full of pasta, I suspect you don't care much about cholesterol anyway.",
	"I AM A SEXY SHOELESS GOD OF WAR!!!",
	"Jesus was here 2/15/57 B.C.", /* Family Guy */
	"If you have time to read this, you should get to the down stairs immediately!", /* Zelda 3 */
	"Gary was here! Ash is a loser!", /* Pokemon */
	"Oh, I get it. The real treasure was the people I had executed along the way!", /* A Practical Guide To Evil  */
	
	"I see you...",
		"...do you see me?",
		"That was a mistake.",
	"What was drowned, and washed away, might never rise again.",
		"Eternity is a lie.",
		"Everything dies.",
		"What is lost might not be found.",
		"And even if it all came back, it would never be the same.",
	
	"Department of Game Balance: Closing barn doors since day two.",
	
	"Listen at us, won't you?", /* All Our Friends Are Dead */
		"Share with us your head",
		"Do hide behind the iris",
		"As all of our friends are dead.",
	
	"T1M3 FOr WH4T?", /*Homestuck. This is Terezi's side of her conversation with Snowman, */
		"WH4T M1SS1ON?", /*in which Terezi communicated by writing on walls and statues etc with colored chalk*/
		"WH4T 4BOUT TH3 QU33N?",
		"J4CK W4S H3LP1NG US 3X1L3 H3R SO...BOTH??? >:\\",
		"OK H3H3H3 >:] WOW OHMYGOD D4RK T3XT L4DY! 1 G3T 1T!!!!!",
		">8[ NO 1 DONT TH1NK SO WH3R3 4R3 YOU TH3N?",
		"OK SORRY 1 4SK3D! >:O",
		"WH4T? NO H3H3H3 1TS OK B3C4US3",
		"1 DONT W4NT TO RUN OUT OF R3D >:[",
		"B3C4US3!!! R3D 1S TH3 B3ST 4ND T4ST3S TH3 B3ST!!! <3",
		"OK TH3N WH4T?",
		"WH3R3 1S 1T?",
	"Im Sure That Possibly All Made Sense Good Speech",
	
	"And there shall be laughter and magic and blood, and we shall dance our dance until the end of time...", /*Incursion, Halls of the Goblin King*/
		"May you find beauty in endings.",
	
	"The eclipse will shatter the sky and from the remnants will bleed a storm of fire to engulf the earth.",	/* Gone with the Blastwave */
		"The spirits of past sins will char the land and cleanse the planet.",
		"As humans shed their former selves their true form shall be revealed.",
		"Join us in the dancing lights. Join us in the Celebration!",
		"Burn them! Burn ALL!",
		
	"Speak not the Watchers.",	/* Drakengard */
		"Draw not the Watchers.",
		"Write not the Watchers.",
		"Sculpt not the Watchers.",
		"Sing not the Watchers.",
		"Call not the Watchers' name.",
	"Glory to Mankind.", /* Nier Automata */
	"Should you encounter God on your quest, God will be cut.", /*Kill Bill*/
	"Generally, when one is conducting an experiment, it is worth noting that the observers went insane, killed each other, and then rose from the dead.", 
		/*Pyro627, regarding an experiment in creating undead bacon in Dwarf Fortress. An "accident" with an axe led to a zombified army, 
			which the poster didn't even comment on.*/
		"Today, in 'poor word choice': 'Urist, go make a new table with our mason.'", /*Swonnrr, regarding a fell mood.*/
		"The horses outnumber us. I have seen settlements with a thousand horses to a man. I have seen them in the deepest caverns. They are everywhere. Save us.", /*Quote from: Fniff regarding a bug causing ubiquitous horses*/
	"All the world's cracked but me and thee; and I'm not too sure about thee.",
	"Some walls are made of fear, but some are made from wall material.", /*Clash of Clans metro advert */
	"No gods, no masters.", /*Anarchist slogan*/
	"For the world's more full of weeping than you can understand.", /*The Stolen Child*/
	"No matter where you go, there you are.",
	"You noticed them, and they noticed that you noticed them.", /*The Mothman Prophecies*/
	"Our bodies and our minds will be fragmented into their smallest parts, until not one part remains.  Annihilation.", /* Annihilation */
	"Well, how did I get here?", /*Once In A Lifetime*/
	"You can be me when I'm gone.", /*Sandman*/
		"You can stop being anything.",
		"Sometimes you wake up. Sometimes the fall kills you. And sometimes, when you fall, you fly.",
		"To absent friends, lost loves, old gods and the season of mists.",
		"And may each and everyone of us always give the devil his due.",
		"Innocence, once lost, can never be regained.",
		"In the pale light of the Moon I play the game of you. Whoever I am. Whoever you are.",
	"Have been unavoidably detained by the world. Expect us when you see us.", /* Stardust */
		"There was once a young man who wished to gain his Heart's Desire.",
		"Time, the thief, eventually takes all things into his dusty storehouse.",
		"Scuse me, but would you mind dreamin' a bit quieter?",
	"He who is valiant and pure of spirit may find the holy grail in the Castle of Aaaaarrrrrrggghhh...", /*Monty Python*/
	"If someone is reading this...I must have failed.", /*The Guardian Legend*/
	"\"Wasn't somebody's father?\"", /*The main plot point of Star Wars, according to one who hasn't seen it*/
	"The psychic efficacy of evil magic swords is vastly overrated.", /* The Water Phoenix King*/
		"It is dangerous to be right when the gods are wrong.",
		"Here are blind idiot children playing with a magic that hates them.",
		"The walled-up abomination seems friendly enough.",
		"Kill you... Take your face...",
		"The trees don't even change color anymore.",
		"What happened to you? It wasn't me!",
		"I'm sorry you could never forgive me.",
	"Reach heaven through violence.",/*Kill 6 Billion Demons*/
		"Kindly ignore my first three answers.",
		"I will see you in Samura.",
	"ALL POWER TO THE OLD FLESH!!!!!!! A HOWLING VOID UPON DETRACTORS!!!", /*Awful Hospital*/
		"You *must* see the IMMACULATE LAD!!",
	"You are lost. You can never go home.", /* Gollum's Song */
	"I am weak. Make... me... ideal.", /* Caress of Phyrexia */
	"But now, we dance this grim fandango, and will for years until we rest.", /* Grim Fandango */
	"Bad Wolf", /* Dr. Who */
	"It's only the end if you want it to be.", /*Batgirl*/
	"Mistakes were made.",
	"But at what cost?",
	"Live. Die. Repeat.", /*Edge of Tomorrow*/
	"For we wrestle not against flesh and blood, but against principalities and powers.", /* Ephesians 6:12 */
	"Life is truth, and never a dream...", /* Persona 4 */
	"fire, walk with me.", /*Twin Peaks*/
	"redefine happiness", /* Blue Oyster Cult */
	"Evolve. Transform. Transcend", /*Heart of the Swarm*/
	"no more sadness, no more anger, no more envy...", /*Halo*/
		"Silence fills the empty grave, now that I have gone.",
		"But my mind is not at rest, for questions linger on.",
	"Everyone you know will tell you small lies, except the ones you love, who will tell you large ones.", /*Borderlands 2*/
		"I am eternal / Death will never capture me / My skills exceed his.",
	"We are born of the blood.",/*Bloodborne*/
		"Made men by the blood.",
		"Undone by the blood.",
		"Fear the old blood.",
		"The Great Ones are sympathetic in spirit, and often answer when called upon.",
		"O Flora, of the moon, of the dream.",
		"O Little ones, o fleeting will of the ancients.",
		"Let the hunter be safe.  Let this dream foretell a pleasant awakening...",
	"We do not breathe. We do not eat. We do not sleep. We do not stop. ", /* Sigma */
		"We have outlasted our gods. We know true freedom.",
		"We do not suffer the enslavement of your passions, or your religions, or your creeds and your self conflict.",
		"We are the Vederiction of our fallen deities, their visage perfected.",
		"We have marched upon the cosmos long before your ancestors left their home.",
		"And we will endure long after your dying civilization perishes on the chain of worlds.",
	"Even false things are true",	"FIVE TONS OF FLAX!",	"Fnord", /*Discordianism*/
	"Ash nazg durbatuluk, ash nazg gimbatul, ash nazg thrakatuluk, agh burzum-ishi krimpatul", /*the Lord of the Rings*/
		"This thing all things devours.",
		"Not all those who wander are lost.",
		"But if of ships I now should sing, what ship would come for me?  What ship would bear me ever back across so wide a Sea?",
		"Seven stars, and seven stones, and one white tree.",
	"This is the curse of the Buddha-you will never again be the same as once you were.", /*Lord of Light*/
		"Tricky and brilliant and heartfelt and dangerous.",
		"Death is mighty, and is no one's friend.",
		"It is the difference between the unknown and the unknowable, between science and fantasy-it is a matter of essence.",
		"Go away. This is not a place to be.",
	"No Fate", /*Terminator*/
	"When the world is in darkness Four Warriors will come....", /*FF I*/
	"This world can have but one Emperor.", /*FF II*/
	"We shall devour your light, and use it to return this world to the Void!", /*FF III*/
	"You tread the path of darkness.",/*FF IV*/
	"My hatred will not be stanched until it has consumed all else!", /*FF V*/
	"Life... dreams... hope... Where do they come from? And where do they go...?", /*FF VI*/
	"Mine is special. It's good for absolutely nothing!", /*FF VII*/
	"I will NEVER be a memory.", /*FF VII Advent Children*/
	"Trees fade with time, as do many things in this world.", /*FF VII Dirge of Cerberus*/
	"Even if the world becomes your enemy... I'll be your knight, I'll protect you.", /*FF VIII*/
	"You don't need a reason to help someone.", /*FF IX*/
	"Memories are nice, but that's all they are.", /*FF X*/
	"Better to leave things lost.", /*FF X-2*/
	"Benighted child, Unchosen, with life so short, This tower freely may you tread.", /*FF XII*/
	"The thirteen days after we awoke were the beginning of the end.", /*FF XIII*/
	"Servant of time. Daughter of chaos. Unto the world unseen, the untamed you guide.", /*FF XIII-2*/
	"One day, the light touched me.", /*FF XIII-3*/
	"What greater weapon is there than to turn an enemy to your cause? To use their own knowledge against them?", /* Knights of the Old Republic */
		"Sand stays.  All else changes.",
		"We will remember you when you are sand too.",
	"That is a lesson I will never learn, for I do not believe it.", /* Knights of the Old Republic II */
	"Do not call up any that you can not put down.", /*HP Lovecraft*/
		"That is not dead, that can eternal lie,",
		"for with strange aeons even death may die.",
	"All the world will be your enemy, Prince With A Thousand Enemies...", /*Watership Down*/
		"...and when they catch you they will kill you...",
		"but first they must catch you.",
	"This statement is false.", /*Logical contradiction*/
	"History is an account, mostly false, of events, mostly unimportant, which are brought about by rulers, mostly knaves, and soldiers, mostly fools",  /*Ambrose Bierce*/
	"Quod Erat Demonstrandum",	"anno Domini",	"per centum", /* QED, AD, percent */
	"I was so surprised, I CLASSIFIED INFORMATION, I really did!", /*Haruhi Suzumiya*/
		"Problems that cannot be solved do not exist in this world.",
		"Feelings of love are just a temporary lapse in judgment.",
	"I'm not going there to die. I'm going to find out if I'm really alive.", /*Cowboy Bebop*/
		"There is nothing left to believe in.",
	"He hides who he really is and pretends to be someone else forever.", /*FLCL*/
		"So in time he becomes that person, so his lie becomes the truth, see?  He transcends the mask.",
		"Well, don't you get it? That's how he finds happiness. That's pretty good, right?",
	"[REDACTED]",	"[DATA EXPUNGED]",	"[DATA PLUNGED]",	"[DATA EXPANDED]",	"I am a toaster!", /* SCP Foundation */
		"Special Containment Procedures: You're going to die, you poor dumb fuck.", /* SCP-1983 */
		"Hello.", "You are no longer lost.", "Here is the map for your journey up.", /* SCP-2084 - Anabasis */
		"A war, whoever was left alive, was Clean.", /* Red Sea Object */
		"Let the depths rise up and sweep over the shores, rendering the works of man to dust.", /* SCP-1014 - Jonah Crusoe */
		"in my head theres a sick silk nail", /* SCP 1291 */
	"I prepared Explosive Runes this morning.", /*Order of the Stick*/
		"In any battle, there's always a level of force against which no tactics can succeed.",
	"1002: He is Not that Which He Says He Is.", /*Good Omens*/
		"3001: Behinde the Eagle's Neste a grate Ash hath fellen.",
	"VI. You Shall Not Subject Your God To Market Forces!",	/*Discworld*/
		"Nae quin! Nae laird! Nae master! We willna be fooled again!",
		"Morituri Nolumus Mori",
		"End-of-the-World Switch. PLEASE DO NOT TOUCH",
		"Give a man a fire and he's warm for a day, but set fire to him and he's warm for the rest of his life.",
		"Do not, under any circumstances, open this door",
		"WORDS IN THE HEART CANNOT BE TAKEN",
		"Slab: Jus' say \"AarrghaarrghpleeassennononoUGH\"",
		"Ahahahahaha! Ahahahaha! Aahahaha! BEWARE!!!!! Yrs sincerely, The Opera Ghost",
		"Bee There Orr Bee A Rectangular Thyng",
		"We're certainly dwarves",
		"There is no doubt being human is incredibly difficult and cannot be mastered in one lifetime.",
	"Now, here, you see, it takes all the running you can do, to keep in the same place.", /*Through the Looking-Glass */
	"Three signs of the Return: the stranger in the door; the friendless wizard; the unmitigated Sun.", /* A Wizard Abroad */
		"Three signs of the Monomachy: a smith without a forge; a saint without a cell; a day without a night.",
	"NOT A Secret Base", /*Gunnerkrigg Court*/
	"We have the option to definitely die, but I've decided against it.", /*Schlock Mercenary*/
	"Short to long term memory impaired. Go to Robotics Building. Explain about Bowman's architecture. Write down everything.", /*Freefall*/
	"I SAW THE EYE. Over the horizon, like a rising sun!", /* Dresden Codak */
		"That's Science, isn't it? The perpetual horizon. But we found a way, haha, we found a way around that!",
		"What would YOU give up to... to see it through?",
		"Nothing. A shadow is nothing. It is merely a question not yet answered. We only fear the dark if we have no means of lighting our way.",
		"The world turns, and dawn comes. And under the light of the sun I shall slay giants.",
	"Don't trust the skull.", /* Cynarfpncr: Gbezrag (mild spoiler) */
	"death is the greatest illusion", /*Blade of Innocence*/
	"Our sandwiches come with attempted assassination plots. New sales gimmick!", /*Shemeska's Planescape Storyhour*/
		"I've stood at that edge! I've looked over into that void! I'm just looking at it from the bottom now!",
		"Free! Apple pie with every meal! We don't care if you don't like apple - Take It!",
		"For there is a hole in the sky",
		"and too long have I gazed",
		"There is no fate, there is no destiny, there are only the threads of the future and hands to grasp them; as told before and forever more, do as thou wilt.",
	"The quality of your lives depends on what you make of them!", /* The Life and Times of Scrooge McDuck */
	"We do not threaten or warn or advise. We tell the truth.", /* The Mansion of E */
		"What you stole is even more dangerous and precious than you know.",
		"In the end, you will fly away and live forever, and you will leave the world shattered in your wake.",
		"You will return to where you started, and you will become what you hate.",
		"And in doing so, you will restore the world to what it was.",
		">^ N^T R3A> T||1S S1<N",
		">^ R3A> T||1S ^N3",
	"Fall in a more hardboiled manner.", /* MS Paint Adventures */
		"Since you are reading this, chances are you have already installed this game on your computer.  If this is true, you have just participated in bringing about the end of the world.  But don't beat yourself up about it.",
		"YOU HATE TIME TRAVEL YOU HATE TIME TRAVEL YOU HATE....",
		"It begins to dawn on you that everything you just did may have been a colossal waste of time.",
	"We will BUILD heroes!", /* THE PROTOMEN */
		"If you replace the working parts, you get a different machine.",
		"If you destroy the working parts, what you get is a broken machine.",
	"To right the countless wrongs of our day, we shine this light of true redemption, that this place may become as paradise.",/*It's a Wonderful World/The World Ends with You*/
		"What a wonderful world such would be...",
		"Please...Don't kill me...",
		"Another world awaits. ...And you're going!",
		"You have seven days.",
	"But the future refused to change.", /*Chrono Trigger*/
	"WHEN ALL ELSE FAILS USE FIRE", /*Zelda II*/
	"Don't you see? All of you... YOUR GODS DESTROYED YOU", /*Wind Waker*/
		"The wind... It is blowing...",
	"I wonder... If you do the right thing... Does it really make... everybody... happy?",/*Majora's Mask*/
		"Your friends... What kind of... people are they? I wonder... Do these people... think of you... as a friend?",
		"What makes you happy? I wonder...what makes you happy...does it make...others happy, too?",
		"Your true face... What kind of... face is it? I wonder... The face under the mask... Is that... your true face?",
		"You've met with a terrible fate, haven't you?",
	"So I really am just a failure!", /*Breath of the Wild*/
	"Why are you sad?", /*Blood-C*/
	"Only accurate information has practical application.", /* not sure where this originated, possibly Aron Ra?*/
	"We will come for you again  When you are alone  In the dark  Without your sword", /*Jake Wyatt's Necropolis*/
		"My sword is only steel in a useful shape. It's me you should fear, you stupid, dead things.",
		"There are horrors.  But also the sun, the grass... And the road I wish to travel.",
	"Now watch what you say or they'll be calling you a radical!", /* The Logical Song, Supertramp */
	"I took a walk around the world, to ease my troubled mind.", /* Kryptonite */
	"Do what thou wilt shall be the whole of the Law", /* Aleister Crowley */
	"Oh, 'great warrior...' Wars not make one great!", /*Star Wars*/
		"I've got a bad feeling about this...",
		"Do, or Do Not.  There is no Try.",
		"Never tell me the odds!"
};

const int wardStrokes[][7] = {
	{1,1,1, 1,1,1, 1}, /* Spacer */
	{21,21,21, 21,21,21, 21}, /* Heptagram */
	{60,60,60, 60,60,60, 60}, /* Gorgoneion */
	{2,2,2, 2,2,2, 2}, /* Circle of Acheron */
	{10,10,10, 10,10,10, 10}, /* Pentagram */
	{12,12,12, 12,12,12, 12}, /* Hexagram */
	{10,10,20, 10,20,10, 10}, /* Hamsa */
	{6,10,8, 8,8,8, 8}, /* Elder Sign */
	{5,5,5, 5,5,5, 5}, /* Elder Elemental Eye */
	{8,8,8, 8,8,8, 8}, /* Sign of the Scion Queen Mother */
	{7,5,6, 7,5,4, 7}, /* Cartouche of the Cat Lord */
	{10,10,10, 10,10,10, 10}, /* Wings of Garuda */
	{1,1,1, 1,1,1, 1}, /* Sigil of Cthugha */
	{1,1,1, 1,1,1, 1}, /* Brand of Ithaqua */
	{1,1,1, 1,1,1, 1}, /* Tracery of Karakal */
	{4,4,4, 4,4,4, 4}, /* Yellow Sign */
	{5,5,5, 5,5,5, 5}, /* Hyper transit */
	{5,5,5, 5,5,5, 5}, /* Hyper stabilize */
	{10,10,10, 10,10,10, 10} /* Terminator */
};

const int wardsAdded[][7] = {
	{1,1,1, 1,1,1, 1}, /* Spacer */
	{1,1,1, 1,1,1, 1}, /* Heptagram */
	{1,1,1, 0,0,0, 0}, /* Gorgoneion */
	{1,1,1, 1,0,0, 0}, /* Circle of Acheron */
	{1,1,1, 1,1,1, 1}, /* Pentagram */
	{1,1,1, 1,1,1, 1}, /* Hexagram */
	{1,1,2, 1,2,1, 0}, /* Hamsa */
	{1,1,1, 1,1,1, 0}, /* Elder Sign */
	{1,1,1, 1,1,1, 1}, /* Elder Elemental Eye */
	{1,1,1, 1,1,1, 1}, /* Sign of the Scion Queen Mother */
	{1,1,1, 1,1,1, 1}, /* Cartouche of the Cat Lord */
	{1,1,1, 1,1,1, 1}, /* Wings of Garuda */
	{1,0,0, 0,0,0, 0}, /* Sigil of Cthugha */
	{1,0,0, 0,0,0, 0}, /* Brand of Ithaqua */
	{1,0,0, 0,0,0, 0}, /* Tracery of Karakal */
	{1,0,0, 0,0,0, 0}, /* Yellow Sign */
	{1,0,0, 0,0,0, 0}, /* Hyper transit */
	{1,0,0, 0,0,0, 0}, /* Hyper stabilize */
	{0,0,0, 0,0,0, 0} /* Terminator */
};

int wardMax[18] = {
	7, /* Spacer */
	7, /* Heptagram */
	3,
	4,
	7,
	7,
	6,
	6,
	7,
	7,
	7,
	7,
	1,
	1,
	1,
	1,
	1,
	1
};

//const int wardDesc[] = {0, 0,0,0, 0,0,0, 0,0,0, 0,1}
//const char * descOfWards[] = {"There is	",	"The	"}
//There is ? inscribed here.
const char * wardText[][7] =  {
	{"a one",	"a two",	"a three",	"a four",	"a five","a six",	"a seven"},
	{"a heptagram",	"a two-fold heptagram",	"a three-fold heptagram",	"a four-fold heptagram", 
		"a five-fold heptagram","a six-fold heptagram",	"a seven-fold heptagram"},
	{"a Gorgoneion",	"a pair of Gorgoneia",	"a trio of Gorgoneia",	"too many (4) Gorgoneia", 
		"too many (5) Gorgoneia",	"too many (6) Gorgoneia",	"too many (7) Gorgoneia"},
	{"a circle of Acheron",	"a two-level circle of Acheron",	"a three-level circle of Acheron", 
		"a four-level circle of Acheron",	"too many (5) circles of Acheron",
		"too many (6) circles of Acheron",	"too many (7) circles of Acheron"},
	{"a pentagram",	"a two-fold pentagram",	"a three-fold pentagram",	"a four-fold pentagram", 
		"a five-fold pentagram","a six-fold pentagram",	"a seven-fold pentagram"},
	{"a hexagram",	"a two-fold hexagram",	"a three-fold hexagram",	"a four-fold hexagram", 
		"a five-fold hexagram","a six-fold hexagram",	"a seven-fold hexagram"},
	{"a hamsa mark",	"a pair of hamsa marks",	"a three-handed hamsa mark", 
		"a four-handed hamsa mark",	"a five-handed hamsa mark","a six-handed hamsa mark", 
		"an overabundance of hamsa marks"},
	{"an elder sign",	"a one-armed elder sign",	"a two-armed elder sign",	"a three-armed elder sign", 
		"a four-armed elder sign","an eye of Yggdrasil",	"an extra-dimensional elder sign"},
	{"an elder elemental eye",	"a two-lobed elder elemental eye",	"a three-lobed elder elemental eye",
		"a four-lobed elder elemental eye",	"a five-lobed elder elemental eye",
		"a six-lobed elder elmental eye",	"a seven-lobed elder elemental eye"},
	{"a sign of the Scion Queen Mother",	"a reinforced sign of the Scion Queen Mother", 
		"a doubly reinforced sign of the Scion Queen Mother", 
		"a triply reinforced sign of the Scion Queen Mother", 
		"a quadruply reinforced sign of the Scion Queen Mother",
		"a quintuply reinforced sign of the Scion Queen Mother", 
		"a sextuply reinforced sign of the Scion Queen Mother"},
	{"a one-hieroglyph partial cartouche of the Cat Lord", 
		"a two-hieroglyph partial cartouche of the Cat Lord", 
		"a three-hieroglyph partial cartouche of the Cat Lord", 
		"a five-hieroglyph partial cartouche of the Cat Lord", 
		"a six-hieroglyph partial cartouche of the Cat Lord",
		"a seven-hieroglyph partial cartouche of the Cat Lord", 
		"a nine-hieroglyph completed cartouche of the Cat Lord"},
	{"a rudimentary rendition of the wings of Garuda",
		"a almost passable rendition of the wings of Garuda", 
		"a passable rendition of the wings of Garuda",
		"a somewhat detailed rendition of the wings of Garuda",
		"a detailed rendition of the wings of Garuda",
		"a very detailed rendition of the wings of Garuda",
		"a splendid rendition of the wings of Garuda"},
	{"a sigil of Cthugha",
		"", 
		"",
		"",
		"",
		"",
		""},
	{"a brand of Ithaqua",
		"", 
		"",
		"",
		"",
		"",
		""},
	{"a tracery of Karakal",
		"", 
		"",
		"",
		"",
		"",
		""},
	{"a yellow sign",
		"", 
		"",
		"",
		"",
		"",
		""},
	{"a Hypergeometric transit equation",
		"", 
		"",
		"",
		"",
		"",
		""},
	{"a Hypergeometric stabilization equation",
		"", 
		"",
		"",
		"",
		"",
		""},
};
const char * wardNames[][2] = {
	{"digit",	"digits"},
	{"heptagram",	"heptagrams"},
	{"Gorgoneion",	"Gorgoneia"},
	{"circle of Acheron",	"circles of Acheron"},
	{"pentagram",	"pentagrams"},
	{"hexagram",	"hexagrams"},
	{"hamsa mark",	"hamsa marks"},
	{"elder sign",	"elder signs"},
	{"elder elemental eye",	"elder elemental eyes"},
	{"sign of the Scion Queen Mother",	"signs of the Scion Queen Mother"},
	{"hieroglyph",	"hieroglyphs"},
	{"wing of Garuda",	"wings of Garuda"},
	{"sigil of Cthugha",	"sigils of Cthugha"},
	{"brand of Ithaqua",	"brands of Ithaqua"},
	{"tracery of Karakal",	"traceries of Karakal"},
	{"yellow sign",	"yellow signs"},
	{"Hypergeometric transit equation", "Hypergeometric transit equations"},
	{"Hypergeometric stabilization equation", "Hypergeometric stabilization equations"},
};


/* There is %s drawn here */
const char * haluWard[] =  {
	"", /* 0 index */
	"a series of disconnected lines", /* nondescript*/

	/*DnD*/
	"a cerulean weeping-willow", /* it's magic. Unlike the others, this one works. Keep in sync with engrave.h!*/
	
	/*Special behavior, these move across the floor, keep in sync with allmain.c*/
	"a north-east facing glider",
	"a north-west facing glider",
	"a south-west facing glider",
	"a south-east facing glider",
	"a square",
	
	/*Special behavior, these identify drow houses*/
	"a House Baenre crest",
	"a House Barrison Del'Armgo crest",
	"a House Xorlarrin crest",
	"a House Faen Tlabbar crest",
	"a House Mizzrym crest",
	"a House Fey-Branche crest",
	"a House Melarn crest",
	"a House Duskryn crest",
	
	/*Special behavior, these identify fallen drow houses*/
	"a House Bront'tej crest",
	"a House Celofraie crest",
	"a House DeVir crest",
	"a House Do'Urden crest",
	"a House Elec'thil crest",
	"a House H'Kar crest",
	"a House Hun'ett crest",
	"a House Masq'il'yr crest",
	"a House Mlin'thobbyn crest",
	"a House Oblodra crest",
	"a House S'sril crest",
	"a House Syr'thaerl crest",
	"a House Teken'duis crest",
	"a House Thaeyalla crest",
	"a House X'larraz'et'soj crest",
	
		
	/*Special behavior, these identify hedrow towers*/
	"a Tower Sorcere crest",
	"a Tower Magthere crest",
	"a Tower Xaxox crest",
	
	/* Special behavior, Last Bastion symbol */
	"a Last Bastion insignia",
	
	/* Special behavior, Lolth holy symbol */
	"a silver spider-like star",
	
	/* Special behavior, Kiaransalee holy symbol */
	"a pair of silver-beringed hands",
	
	/* Special behavior, Pen'a holy symbol */
	"a silver matron",
	
	/* Special behavior, Ver'tas holy symbol */
	"a silver feather tearing a web",
	
	/* Special behavior, Eilistraee holy symbol */
	"a pair of dancing silver drow",
	
	/* Special behavior, Ghaunadaur holy symbol */
	"a faintly luminous purple eye",

	/* eternal matriarch symbol */
	"a crest of a lost house of Svartalfheim",

	/* Eddergud holy symbol */
	"an obsidian spiderweb",
	
	/* Footprint */
	"a footprint",
	
	/* Planar Symbols */
	"a circle enclosing two curved colliding arrows",			/*44*/
	"a pair of triangles, drawn tip to tip",					/*45*/
	"a stylized beast",											/*46*/
	"a triangle crowned by a single line",						/*47*/
	"a simple image of many mountains",							/*48*/
	"a sketch of a shining diamond",							/*49*/
	"a tree-rune",												/*50*/
	"an eight-toothed gear",									/*51*/
	"a random scribble",										/*52*/
	"a square with two small handles on opposite sides",		/*53*/
	"a square enclosing a spiral",								/*54*/
	"an eye with three inverted crosses",						/*55*/
	"an infinity symbol crossed by a burning downwards arrow",	/*56*/
	"a set of four nested triangles",							/*57*/
	"a watchful eye blocking an upward arrow",					/*58*/
	"a pitchfork thrust into the ground",						/*59*/
	
	/* Not quite */
	"a heptagenarian",
	"an octogram",
	"a pentagrain",
	"a circle of da Vinci",
	"a hand making a rude gesture",
	"a junior sign",
	"a childish compound eye",
	"a Sign of an Illegitimate Step-daughter",
	"a cenotaph of a catgirl",
	"a groovy rendition of the wings of Gargula",
	
	/* books */
	"a set of holy horns",	"a Summoning Dark mine-sign",	"a Long Dark mine-sign",
		"a Following Dark mine-sign",	"a Closing Dark mine-sign",	"an Opening Dark mine-sign",
		"a Breathing Dark mine-sign",	"a Speaking Dark mine-sign",	"a Catching Dark mine-sign",
		"a Secret Dark mine-sign",	"a Calling Dark mine-sign",	"a Waiting Dark mine-sign",
		"a florid crest dominated by a double-headed bat",
		"a Guarding Dark mine-sign",	"the mark of the Caller of Eight", /* Discworld */
	"a lidless eye", /* Lord of the Rings */
		"a white tree", /* Gondor, Lord of the Rings */
	"a triangle enclosing a circle and bisected by a line", /* Harry Potter */
	"a set of three trefoils, the lower most inverted", /* describes the three of clubs. Too Many Magicians*/
	"a Trump of Doom",	"a Sign of Chaos",	"a Pattern of Amber",	"a Ghostwheel",
	"a mockingjay", /* Hunger Games */
	"a Sharuan Mindharp", /* Star Wars expanded universe */
	"a winged blade of light", /* Jedi Order symbol */
	
	/* webcomics */
	"a Court symbol",	"a Forest symbol",	"the sign of the Wandering Eye", /* Gunnerkrigg Court */
	"a winged tower", /* Girl Genius */
		"a stylized trilobite",
	"a setting (rising?) sun", /* Dresden Codak */
	
	/* anime and manga */
	"a Robotech Defense Force insignia", /*...Robotech*/
	"a Black Knights insignia", /* Code Geass */
	"a rose crest", /* Revolutionary Girl Utena */
	"an inverted triangle flanked by seven eyes", /* NGE */
	"a laughing man", /* Ghost in the Shell */
	"an alchemic array",	"a human transmutation circle", /* Fullmetal Alchemist */
	"an asymmetric, stylized arrowhead, point upwards", /* Star Trek*/
		"a set of three blades, the top blade straight, the dexter curved down, the sinister curved up",
	"a white lotus", /* Avatar, the Last Airbender */
	
	"a winged eye and a tear of blood", /*02, Kirby*/
	"an angular S before a segmented circle",/*a screw attack symbol*/
	"a stylized umbrella", /* Resident Evil */
	"an Imperium Aquilas", /* Warhammer 40k */
		"more dakka",
	"a triangle composed of three smaller triangles",	"an eye and single tear", /*Zelda*/
		"a circle enclosing four swirling lines",	"a flame inside a circle",
		"a snowflake within a circle",	"an inverted triangle with a dot above each face, enclosed by a circle",
		"a sign resembling an eyeless yin-yang", 
		"a circle surrounding a triangle of dots and another of triangles",
	
	"a silhouette of a bat", /* Batman */
		"an 'S' in a diamond", /* Superman (the House of El) */
	
	"a symbol of pain", /* DnD */
	/* Planescape */
	"a mimir",
	"a symbol of torment",
	
	/* Zodiac */
	"an Aries sign",
	"a Taurus sign",
	"a Gemini sign",
	"a Cancer sign",
	"a Leo sign",
	"a Virgo sign",
	"a Libra sign",
	"a Scorpio sign",
	"a Sagittarius sign",
	"a Capricorn sign",
	"an Aquarius sign",
	"a Pisces sign",
	
	"a heart pierced through with an arrow",
	"a broken heart",
	"a skull and crossed bones",
	"a bad situation",
	"a zorkmid",
	
	"a diagram of the bridges of Konigsberg",
	
	"a hand-mirror of Aphrodite",
	"a shield and spear of Ares", /* alchemy/male/female */
	
	"a black moon Lilith sign",
	
	"a window", /* op-sys*/
	"a no symbol",
	"a test pattern",
	"a work of modern art",
	"a flag of Neverland",
	"a hyped-up duck dressed in a sailor's shirt and hat", /* Disney */
		"a mouse with 2d ears",
		"a set of three circles in the shape of a mouse's head",
	"a meaningless coincidence",
	
	/*Corporate Logos*/
	"a stylized, fan-shaped seashell",
	"a bitten apple",
	"a pair of arches meeting to form an \"M\"",
	"a Swoosh mark",
	
/* Digimon Adventure */
	"a Crest of Courage",
	"a Crest of Friendship",
	"a Crest of Love",
	"a Crest of Knowledge",
	"a Crest of Purity",
	"a Crest of Sincerity",
	"a Crest of Hope",
	"a Crest of Light",
	"a Crest of Miracles",
	"a Crest of Kindness",

	"a Zeon crest", /* Mobile Suit Gundam */

        /* Umineko no Naku Koro ni */
	"the Ushiromiya family crest",
        "the first magic circle of the moon",
        "the third magic circle of Mars",
        "the fifth magic circle of Mars",
	
	"a dream of the Fallen",
	"a sign of the world to come",
	"a thousandfold eye",
	"a set of five interlocked rings", /*Olympics logo*/
	"a tree diagram",
	"a running man", /* Exit */
	"a running man holding a cane",
	"a one-and-zero", /* Power toggle */
	"a thick soup of mist",	"a pattern of squared circles", 
	"a void",	"a notable lack of images",	"a stark absence of pictures",	"nothing much",
	"a convergence of parallel lines",	"a sphere", /* How did you manage that? */
	"a yin-yang",	"a taijitu",/* Taoist */
	"a hand of Eris", /* Discordian */
	"a butterfly of death and rebirth",
	"an ichthus",	"a Cross", /* Christian*/
	"a wheel with eight spokes", /* Budhism */
	"a fish with legs",	"a fat fish",	"a fish with tentacles, legs, and wings",
		/* ichthus parodies/derivatives: darwin, buddha, and Cthulhu. */
	"an Eye of Horus", /*...*/
	"a device to make the happy man sad and the sad man happy", /*"This too shall pass"*/
	"a set of seven concentric circles",
	"a left-handed trefoil knot",
	"a triskelion", /* Ancient Symbol */
	"a rough circle enclosing an A", /* Anarchy */
	"a Tree of Life", /* Kabbalah */
	"a winged oak",
	"a wheel cross",	"a labyrinth",	"sign of Shamash", 
	"a naudh rune", /* misery */
	"an Eye of Providence",	"a pyramid surmounted by an eye", /* Christian */
	"a one-way staircase",
	"an 'a' encircled by its own tail" /* meta */
};

//There is a seal of ? inscribed here.
char * sealNames[] =  {
	"Ahazu", /* 0 */
	"Amon", /* 1 */
	"Andrealphus", /* 2 */
	"Andromalius", /* 3 */
	"Astaroth", /* 4 */
	"Balam", /* 5 */
	"Berith", /* 6 */
	"Buer", /* 7 */
	"Chupoclops", /* 8 */
	"Dantalion", /* 9 */
	"Echidna", /* 10 */
	"Eden", /* 11 */
	"Enki", /* 12 */
	"Eurynome", /* 13 */
	"Eve", /* 14 */
	"Fafnir", /* 15 */
	"Huginn and Muninn", /* 16 */
	"Iris", /* 17 */
	"Jack", /* 18 */
	"Malphas", /* 19 */
	"Marionette", /* 20 */
	"Mother", /* 21 */
	"Naberius", /* 22 */
	"Orthos", /* 23 */
	"Ose", /* 24 */
	"Otiax", /* 25 */
	"Paimon", /* 26 */
	"Shiro", /* 27 */
	"Simurgh", /* 28 */
	"Tenebrous", /* 29 */
	"Ymir", /* 30 */
	"Dahlver-Nar", /* 31 */
	"Acererak", /* 32 */
	"the Council of Elements", /* 33 */
	"Cosmos", /* 34 */
	"Mediator", /* 35 */
	"Miska", /* 36 */
	"Nudzirath", /* 37 */
	"the Alignment Thing", /* 38 */
	"the Unknown God", /* 39 */
	"the Eddergud", /* 40 */
	"Numina" /* 41 */
};

char * sealTitles[] =  {
	", the Seizer", /* 0 */
	", the shadow before the altar", /* 1 */
	", Marquis of Angles", /* 2 */
	", the Repentent Rogue", /* 3 */
	", the Broken Clockmaker", /* 4 */
	", the Last Sacrifice", /* 5 */
	", the Red Horseman", /* 6 */
	", who walks all places", /* 7 */
	", Hopetrapper", /* 8 */
	", the Star Emperor", /* 9 */
	", Mother of Monsters", /* 10 */
	", the First Garden", /* 11 */
	", God of the first city", /* 12 */
	", lonely dancer", /* 13 */
	", the first Sinner", /* 14 */
	", Jotunn Dragon", /* 15 */
	", Thought and Memory", /* 16 */
	", Friend from Afar", /* 17 */
	" of the Lantern", /* 18 */
	", the Prince of Crows", /* 19 */
	", Puppet of Screams", /* 20 */
	", who beheld Beauty", /* 21 */
	", mysterious councilor", /* 22 */
	", Sovereign of the Howling Dark", /* 23 */
	", dreamer in a drowned city", /* 24 */
	", the Key to the Gate", /* 25 */
	", the Fell Archivist", /* 26 */
	", the stone soldier", /* 27 */
	", who roosts on the Tree of Life", /* 28 */
	", the Demonic Shadow", /* 29 */
	", Lord of the Grassless Gap", /* 30 */
	", the first Binder", /* 31 */
	", the Demi-Lich", /* 32 */
	"", /* 33 */
	", goddess of crystal", /* 34 */
	", shard of the Living Gate", /* 35 */
	" the wolf-spider", /* 36 */
	", the mirrored destroyer", /* 37 */
	"", /* 38 */
	"", /* 39 */
	", The Black Web Entity", /* 40 */
	", The Whispering Multitude" /* 41 */
};

char *
random_engraving(outbuf)
char *outbuf;
{
	const char *rumor;

	/* a random engraving may come from the	"rumors" file,
	   or from the list above */
	if (!rn2(4) || !(rumor = getrumor(0, outbuf, TRUE)) || !*rumor)
	    Strcpy(outbuf, random_mesg[rn2(SIZE(random_mesg))]);

	wipeout_text(outbuf, (int)(strlen(outbuf) / 4), 0);
	return outbuf;
}

int
random_haluIndex()
{
	return rn2(SIZE(haluWard));
}

int
get_num_wards_added(ward,complete)
int ward;
int complete;
{
	return wardsAdded[ward][complete];
}


/* Partial rubouts for engraving characters. -3. */
static const struct {
	char		wipefrom;
	const char *	wipeto;
} rubouts[] = {
	{'A', "^"},     {'B', "Pb["},   {'C', "("},     {'D', "|)["},
	{'E', "|FL[_"}, {'F', "|-"},    {'G', "C("},    {'H', "|-"},
	{'I', "|"},     {'K', "|<"},    {'L', "|_"},    {'M', "|"},
	{'N', "|\\"},   {'O', "C("},    {'P', "F"},     {'Q', "C("},
	{'R', "PF"},    {'T', "|"},     {'U', "J"},     {'V', "/\\"},
	{'W', "V/\\"},  {'Z', "/"},
	{'b', "|"},     {'d', "c|"},    {'e', "c"},     {'g', "c"},
	{'h', "n"},     {'j', "i"},     {'k', "|"},     {'l', "|"},
	{'m', "nr"},    {'n', "r"},     {'o', "c"},     {'q', "c"},
	{'w', "v"},     {'y', "v"},
	{':', "."},     {';', ","},
	{'0', "C("},    {'1', "|"},     {'6', "o"},     {'7', "/"},
	{'8', "3o"}
};

struct engr *
get_head_engr(){
	return head_engr;
}

const char *
fetchHaluWard(index)
int index;
{
	return haluWard[index];
}

int
randHaluWard(){
	return rn2(SIZE(haluWard)-1)+1;
}

void
wipeout_text(engr, cnt, seed)
char *engr;
int cnt;
unsigned seed;		/* for semi-controlled randomization */
{
	char *s;
	int i, j, nxt, use_rubout, lth = (int)strlen(engr);

	if (lth && cnt > 0) {
	    while (cnt--) {
		/* pick next character */
		if (!seed) {
		    /* random */
		    nxt = rn2(lth);
		    use_rubout = rn2(4);
		} else {
		    /* predictable; caller can reproduce the same sequence by
		       supplying the same arguments later, or a pseudo-random
		       sequence by varying any of them */
		    nxt = seed % lth;
		    seed *= 31,  seed %= (BUFSZ-1);
		    use_rubout = seed & 3;
		}
		s = &engr[nxt];
		if (*s == ' ') continue;

		/* rub out unreadable & small punctuation marks */
		if (index("?.,'`-|_", *s)) {
		    *s = ' ';
		    continue;
		}

		if (!use_rubout)
		    i = SIZE(rubouts);
		else
		    for (i = 0; i < SIZE(rubouts); i++)
			if (*s == rubouts[i].wipefrom) {
			    /*
			     * Pick one of the substitutes at random.
			     */
			    if (!seed)
				j = rn2(strlen(rubouts[i].wipeto));
			    else {
				seed *= 31,  seed %= (BUFSZ-1);
				j = seed % (strlen(rubouts[i].wipeto));
			    }
			    *s = rubouts[i].wipeto[j];
			    break;
			}

		/* didn't pick rubout; use '?' for unreadable character */
		if (i == SIZE(rubouts)) *s = '?';
	    }
	}

	/* trim trailing spaces */
	while (lth && engr[lth-1] == ' ') engr[--lth] = 0;
}

void
wipeout_ward(ep, cnt, seed)
struct engr *ep;
int cnt;
unsigned seed;		/* for semi-controlled randomization */
{
	char *s;
	int i, j, nxt, lth, clth, slth, dlth, plth;
	lth = 0;
	clth = slth = dlth = plth = 0;
	j = 0;
	if(ep->ward_id >= FIRST_SEAL && /*ep->engr_time+5 >= moves &&*/ ep->ward_type != DUST && ep->ward_type != ENGR_BLOOD) return;
	if(ep->halu_ward){
		clth = ep->complete_wards * 12;
		slth = ep->scuffed_wards * 6;
		dlth = ep->degraded_wards * 3;
		plth = ep->partial_wards * 1;
		lth = clth+slth+dlth+plth;
	}else if(ep->ward_id >= FIRST_SEAL){
		clth = ep->complete_wards * 5;
		slth = ep->scuffed_wards * 4;
		dlth = ep->degraded_wards * 3;
		plth = ep->partial_wards * 2;
		lth = clth+slth+dlth+plth;
	}
	else{
		i = ep->complete_wards;
		while(i-- > 0){
			lth += wardStrokes[ep->ward_id][j];
			clth += wardStrokes[ep->ward_id][j];
			j++;
		}
		i = ep->scuffed_wards;
		slth = clth;
		while(i-- > 0){
			lth += wardStrokes[ep->ward_id][j] * .9;
			slth += wardStrokes[ep->ward_id][j] * .9;
			j++;
		}
		i = ep->degraded_wards;
		dlth = slth;
		while(i-- > 0){
			lth += wardStrokes[ep->ward_id][j] * .75;
			dlth += wardStrokes[ep->ward_id][j] * .75;
			j++;
		}
		i = ep->partial_wards;
		plth = dlth;
		while(i-- > 0){
			lth += wardStrokes[ep->ward_id][j] * .5;
			plth += wardStrokes[ep->ward_id][j] * .5;
			j++;
		}
	}
	if (ep->ward_id && cnt > 0) {
	    while (ep->ward_id && cnt--) {
			/* pick next character */
			if (!seed) {
				/* random */
				nxt = rn2(lth);
			} else {
				/* predictable; caller can reproduce the same sequence by
				   supplying the same arguments later, or a pseudo-random
				   sequence by varying any of them */
				nxt = seed % lth;
				seed *= 31,  seed %= (BUFSZ-1);
			}
			if(nxt < clth){
				ep->complete_wards--;
				ep->scuffed_wards++;
			}
			else if(nxt < slth){
				ep->scuffed_wards--;
				ep->degraded_wards++;
			}
			else if(nxt < dlth){
				ep->degraded_wards--;
				ep->partial_wards++;
			}
			else if(nxt < plth) ep->partial_wards--;
			
			lth = 0;
			clth = slth = dlth = plth = 0;
			j = 0;
			if(ep->halu_ward){
				clth = ep->complete_wards * 10;
				slth = ep->scuffed_wards * 10;
				dlth = ep->degraded_wards * 10;
				plth = ep->partial_wards * 10;
				lth = clth+slth+dlth+plth;
			}else if(ep->ward_id >= FIRST_SEAL){
				clth = ep->complete_wards * 5;
				slth = ep->scuffed_wards * 4;
				dlth = ep->degraded_wards * 3;
				plth = ep->partial_wards * 2;
				lth = clth+slth+dlth+plth;
			}
			else{
				i = ep->complete_wards;
				while(i-- > 0){
					lth += wardStrokes[ep->ward_id][j];
					clth += wardStrokes[ep->ward_id][j];
					j++;
				}
				i = ep->scuffed_wards;
				while(i-- > 0){
					lth += wardStrokes[ep->ward_id][j] * .9;
					slth += wardStrokes[ep->ward_id][j] * .9;
					j++;
				}
				i = ep->degraded_wards;
				while(i-- > 0){
					lth += wardStrokes[ep->ward_id][j] * .75;
					dlth += wardStrokes[ep->ward_id][j] * .75;
					j++;
				}
				i = ep->partial_wards;
				while(i-- > 0){
					lth += wardStrokes[ep->ward_id][j] * .5;
					plth += wardStrokes[ep->ward_id][j] * .5;
					j++;
				}
			}
			if(!lth){
				ep->ward_id = 0;
				ep->ward_type = 0;
			}
		}
	}
}

boolean
can_reach_floor()
{
	return (boolean)(!u.uswallow &&
#ifdef STEED
			/* Restricted/unskilled riders can't reach the floor */
			!(u.usteed && P_SKILL(P_RIDING) < P_BASIC) &&
#endif
			 (!Levitation ||
			  Weightless));
}
#endif /* OVLB */
#ifdef OVL0

const char *
surface(x, y)
register int x, y;
{
	register struct rm *lev = &levl[x][y];

	if ((x == u.ux) && (y == u.uy) && u.uswallow &&
		is_animal(u.ustuck->data))
	    return "maw";
	else if (IS_AIR(lev->typ) && Weightless)
	    return "air";
	else if (is_pool(x,y, TRUE))
	    return (Underwater && !Is_waterlevel(&u.uz)) ? "bottom" : "water";
	else if (is_ice(x,y))
	    return "ice";
	else if (is_lava(x,y))
	    return "lava";
	else if (lev->typ == DRAWBRIDGE_DOWN)
	    return "bridge";
	else if(IS_ALTAR(levl[x][y].typ))
	    return "altar";
	else if(IS_GRAVE(levl[x][y].typ))
	    return "headstone";
	else if(IS_FOUNTAIN(levl[x][y].typ))
	    return "fountain";
	else if(IS_PUDDLE(levl[x][y].typ))
	    return "muddy floor";
	else if ((IS_ROOM(lev->typ) && !Is_earthlevel(&u.uz)) ||
		 IS_WALL(lev->typ) || IS_DOOR(lev->typ) || lev->typ == SDOOR)
	    return "floor";
	else
	    return "ground";
}

const char *
ceiling(x, y)
register int x, y;
{
	register struct rm *lev = &levl[x][y];
	const char *what;

	/* other room types will no longer exist when we're interested --
	 * see check_special_room()
	 */
	if (*in_rooms(x,y,VAULT))
	    what = "vault's ceiling";
	else if (*in_rooms(x,y,TEMPLE))
	    what = "temple's ceiling";
	else if (*in_rooms(x,y,SHOPBASE))
	    what = "shop's ceiling";
	else if (IS_AIR(lev->typ))
	    what = "sky";
	else if (Underwater)
	    what = "water's surface";
	else if ((IS_ROOM(lev->typ) && !Is_earthlevel(&u.uz)) ||
		 IS_WALL(lev->typ) || IS_DOOR(lev->typ) || lev->typ == SDOOR)
	    what = "ceiling";
	else
	    what = "rock above";

	return what;
}


/*Preliminary glider implementation
 *Gliders can move engravings in their square along with them.
 */
void
move_gliders()
{
	struct engr *ep = head_engr;

	while(ep) {
		if(ep->halu_ward){
			switch(ep->ward_id){
				case NORTHEAST_GLIDER:
					if((monstermoves - ep->engr_time)%4 == 3){ //Gliders move at c/4 
						if(IS_ROOM(levl[ep->engr_x+1][ep->engr_y-1].typ)
							&& !engr_at(ep->engr_x+1,ep->engr_y-1)
						){
							/* The glider moves, towing any engravings in the square
							 * along for the ride. Allows for strategic use of glider-rings
							 */
							ep->engr_x++;
							ep->engr_y--;
						}
						else ep->ward_id = DEAD_GLIDER; //Glider turns into a square.
					}
				break;
				case NORTHWEST_GLIDER:
					if((monstermoves - ep->engr_time)%4 == 3){ //Gliders move at c/4 
						if(IS_ROOM(levl[ep->engr_x-1][ep->engr_y-1].typ)
							&& !engr_at(ep->engr_x-1,ep->engr_y-1)
						){
							/* The glider moves, towing any engravings in the square
							 * along for the ride. Allows for strategic use of glider-rings
							 */
							ep->engr_x--;
							ep->engr_y--;
						}
						else ep->ward_id = DEAD_GLIDER; //Glider turns into a square.
					}
				break;
				case SOUTHWEST_GLIDER:
					if((monstermoves - ep->engr_time)%4 == 3){ //Gliders move at c/4 
						if(IS_ROOM(levl[ep->engr_x-1][ep->engr_y+1].typ)
							&& !engr_at(ep->engr_x-1,ep->engr_y+1)
						){
							/* The glider moves, towing any engravings in the square
							 * along for the ride. Allows for strategic use of glider-rings
							 */
							ep->engr_x--;
							ep->engr_y++;
						}
						else ep->ward_id = DEAD_GLIDER; //Glider turns into a square.
					}
				break;
				case SOUTHEAST_GLIDER:
				break;
					if((monstermoves - ep->engr_time)%4 == 3){ //Gliders move at c/4 
						if(IS_ROOM(levl[ep->engr_x+1][ep->engr_y+1].typ)
							&& !engr_at(ep->engr_x+1,ep->engr_y+1)
						){
							/* The glider moves, towing any engravings in the square
							 * along for the ride. Allows for strategic use of glider-rings
							 */
							ep->engr_x++;
							ep->engr_y++;
						}
						else ep->ward_id = DEAD_GLIDER; //Glider turns into a square.
					}
				default:
				//Nothing special
				break;
			}
		}
		ep = ep->nxt_engr;
	}
}

struct engr *
engr_at(x, y)
xchar x, y;
{
	struct engr *ep = head_engr;

	while(ep) {
		if(x == ep->engr_x && y == ep->engr_y)
			return(ep);
		ep = ep->nxt_engr;
	}
	return((struct engr *) 0);
}

/* Decide whether a particular string is engraved at a specified
 * location; a case-insensitive substring match used.
 * Ignore headstones, in case the player names herself "Elbereth".
	Heh. Why not? Restriction on headstone Elberenths removed.
 */
int
sengr_at(s, x, y)
	const char *s;
	xchar x, y;
{
	register struct engr *ep = engr_at(x,y);

	return (ep && /*ep->engr_type != HEADSTONE &&*/
		ep->engr_time <= moves && strstri(ep->engr_txt, s) != 0);
}

int
ward_at(x, y)
	xchar x, y;
{
	register struct engr *ep = engr_at(x,y);

	if(!ep || ep->engr_time > moves) return 0;
	else if(ep->halu_ward) return ep->ward_id == CERULEAN_SIGN ? ELDER_SIGN : 0;
	else return ep->ward_id;
}

int
num_wards_at(x, y)
	xchar x, y;
{
	register struct engr *ep = engr_at(x,y);

	if(!ep || ep->engr_time > moves) return 0;
	else return ep->complete_wards;
}

#endif /* OVL0 */
#ifdef OVL2

void
u_wipe_engr(cnt)
register int cnt;
{
	if (can_reach_floor())
		wipe_engr_at(u.ux, u.uy, cnt);
}

#endif /* OVL2 */
#ifdef OVL1

void
wipe_engr_at(x,y,strt_cnt)
register xchar x,y,strt_cnt;
{
	register struct engr *ep = engr_at(x,y);
	xchar cnt;

	/* Headstones are indelible */
	if(ep && ep->engr_type != HEADSTONE){
	    if(ep->engr_txt[0] && (ep->engr_type != BURN 
			|| is_ice(x,y) 
			|| levl[x][y].typ == SAND)
		) {
			if(ep->engr_type != DUST && ep->engr_type != ENGR_BLOOD) {
				cnt = rn2(1 + 50/(strt_cnt+1)) ? 0 : 1;
			}
			else cnt = strt_cnt;
			wipeout_text(ep->engr_txt, (int)cnt, 0);
			while(ep->engr_txt[0] == ' ')
				ep->engr_txt++;
		}
	    if(ep->ward_id && (ep->ward_type != BURN 
			|| is_ice(x,y)
			|| levl[x][y].typ == SAND
		)) {
			if(ep->ward_type != DUST && ep->ward_type != ENGR_BLOOD) {
				cnt = rn2(1 + 50/(strt_cnt+1)) ? 0 : 1;
			}
			else cnt = strt_cnt;
			wipeout_ward(ep, (int) cnt, 0);
		}
		if(!ep->engr_txt[0] && !ep->ward_id) del_engr_ward(ep);

	}
}

#endif /* OVL1 */
#ifdef OVL2

void
read_engr_at(x,y)
register int x,y;
{
	register struct engr *ep = engr_at(x,y);
	register int	sensed = 0;
	char buf[BUFSZ];
	char * word;
	
	/* Sensing an engraving does not require sight,
	 * nor does it necessarily imply comprehension (literacy).
	 */
	if(ep && ep->engr_txt[0] && (Underwater || !is_pool(x,y, FALSE))) {
	    switch(ep->engr_type) {
	    case DUST:
		if(!Blind) {
			sensed = 1;
			pline("%s is written here in the %s.", Something,
				is_ice(x,y) ? "frost" : is_pool(x,y, TRUE) ? "mud" : "dust");
		}
		break;
	    case ENGRAVE:
	    case HEADSTONE:
		if (!Blind || can_reach_floor()) {
			sensed = 1;
			pline("%s is engraved here on the %s.",
				Something,
				surface(x,y));
		}
		break;
	    case BURN:
		if (!Blind || can_reach_floor()) {
			sensed = 1;
			pline("Some text has been %s into the %s here.",
				is_ice(x,y) ? "melted" : 
				levl[x][y].typ == SAND ? "fused" :
				"burned",
				surface(x,y));
		}
		break;
	    case MARK:
		if(!Blind) {
			sensed = 1;
			pline("There's some graffiti on the %s here.",
				surface(x,y));
		}
		break;
	    case ENGR_BLOOD:
		/* "It's a message!  Scrawled in blood!"
		 * "What's it say?"
		 * "It says... `See you next Wednesday.'" -- Thriller
		 */
		if(!Blind) {
			sensed = 1;
			You("see a message scrawled in blood here.");
		}
		break;
	    default:
			impossible("%s is written in a very strange way.",
					Something);
			sensed = 1;
	    }
	    if (sensed) {
			const char *et;
	    	unsigned maxelen = BUFSZ - sizeof("You feel the words: \"\". ");
			if(!Hallucination /*|| rn2(20)*/ ){
				if (strlen(ep->engr_txt) > maxelen) {
					(void) strncpy(buf,  ep->engr_txt, (int)maxelen);
					buf[maxelen] = '\0';
					et = buf;
				} else
					et = ep->engr_txt;
			}
			else{
				int haluDex = rn2(SIZE(haluMesg));
				if(strlen(haluMesg[haluDex]) > maxelen){
					(void) strncpy(buf,  haluMesg[haluDex], (int)maxelen);
					buf[maxelen] = '\0';
					et = buf;
				}
				else et = haluMesg[haluDex];
			}
			You("%s: \"%s\"",
				  (Blind) ?	"feel the words" :	"read",  et);
			if(flags.run > 1) nomul(0, NULL);
		}
	}
	if(ep && ep->ward_id){
		sensed = 0;
	    switch(ep->ward_type) {
	    case DUST:
		if(!Blind) {
			sensed = 1;
			word = is_ice(x,y) ?	"drawn here in the frost." : 
						(levl[x][y].typ == SAND && !In_mithardir_quest(&u.uz)) ?
					"drawn here in the sand." : 
					"drawn here in the dust.";
		}
		break;
	    case ENGRAVE:
	    case HEADSTONE:
		if (!Blind || can_reach_floor()) {
			sensed = 1;
			word =	"carved here.";
		}
		break;
	    case BURN:
		if (!Blind || can_reach_floor()) {
			sensed = 1;
			word = is_ice(x,y) ?	"melted into the ice here." : 
					levl[x][y].typ == SAND ? "fused into the sand here." :
					"burned into the floor here.";
		
		}
		break;
	    case MARK:
		if(!Blind) {
			sensed = 1;
			word =	"drawn in ink here.";
		}
		break;
	    case ENGR_BLOOD:
		/*	"It's a message!  Scrawled in blood!"
		 *	"What's it say?"
		 *	"It says... `See you next Wednesday.'" -- Thriller
		 */
		if(!Blind) {
			sensed = 1;
			word =	"drawn in blood here.";
		}
		break;
	    default:
			sensed = 1;
			word =	"drawn here in a very strange way.";
	    }
	    if (sensed) {
		 if(!Hallucination){
		  if(!(ep->halu_ward)){
				if( ep->ward_id < FIRST_SEAL ){
					int sum;
					if(ep->complete_wards){
						long wardhere = get_wardID(ep->ward_id);
						pline("There is %s %s", 
							wardText[ep->ward_id][ep->complete_wards-1],
							word);
						if( !(u.wardsknown & wardhere) && ep->ward_id < DONTLEARNWARD ){
							You("have learned a new warding!");
							u.wardsknown |= wardhere;
						}
					}
					sum = ep->scuffed_wards + ep->degraded_wards + ep->partial_wards;
					if(sum){
						char wardbuf[BUFSZ];
						int wardbuflen = 0;
						wardbuf[0] = (char)0;
						if(ep->complete_wards) wardbuflen += sprintf(&wardbuf[wardbuflen],	"There %s also", sum > 1 ?	"are" :	"is");
						else wardbuflen += sprintf(&wardbuf[wardbuflen],	"There %s", sum > 1 ?	"are" :	"is");
						if(ep->scuffed_wards)
							wardbuflen += sprintf(&wardbuf[wardbuflen]," %d scuffed%s", ep->scuffed_wards, ep->degraded_wards && ep->partial_wards ? 
															"," : 
															ep->degraded_wards || ep->partial_wards ? 
															" and" :	"");
						if(ep->degraded_wards)
							wardbuflen += sprintf(&wardbuf[wardbuflen]," %d degraded%s", ep->degraded_wards, ep->scuffed_wards && ep->partial_wards ? 
															", and" : 
															ep->partial_wards ? 
															" and" :	"");
						if(ep->partial_wards)
							wardbuflen += sprintf(&wardbuf[wardbuflen]," %d fragmentary", ep->partial_wards);
						/*if(!(ep->complete_wards))*/ 
						wardbuflen += sprintf(&wardbuf[wardbuflen]," %s",
													wardNames[ep->ward_id][sum>1 ? 1 : 0]);
						//pline("%d",wardbuflen);
						pline("%s %s", wardbuf, word);
					}
				}else{
//					pline("%d-%d=%d", ep->ward_id, FIRST_SEAL, ((int)ep->ward_id)-((int)FIRST_SEAL));
					if(ep->complete_wards){
						pline("There is a seal of %s %s", 
							sealNames[((int)ep->ward_id)-((int)FIRST_SEAL)],
							word);
						if(!Role_if(PM_EXILE) && ep->ward_id < QUEST_SPIRITS){
							long wardhere = 1L << (((int)ep->ward_id)-((int)FIRST_SEAL));
							if( !(u.wardsknown & wardhere) ){
								You("have learned a new seal!");
								u.sealsKnown |= wardhere;
							}
						}
					}
					else if(ep->scuffed_wards){
						pline("There is a scuffed seal of %s %s", 
							sealNames[((int)ep->ward_id)-((int)FIRST_SEAL)],
							word);
					}
					else if(ep->degraded_wards){
						pline("There is a degraded seal of %s %s", 
							sealNames[((int)ep->ward_id)-((int)FIRST_SEAL)],
							word);
					}
					else if(ep->partial_wards){
						pline("There is a fragmentary seal of %s %s", 
							sealNames[((int)ep->ward_id)-((int)FIRST_SEAL)],
							word);
					}
				}
		  }
		  else{//note: assumes that there can only be ONE halucinatory ward scribed.
			if(ep->complete_wards) pline("There is %s %s", 
										haluWard[ep->ward_id],
										word);
			if(ep->scuffed_wards) pline("It is scuffed.");
			else if(ep->degraded_wards) pline("It is degraded.");
			else if(ep->partial_wards) pline("Although only fragments are left.");
		  }
		 }
		 else{
			pline("There is %s drawn here.", haluWard[randHaluWard()]);
		 }
		}
	}
}

#endif /* OVL2 */
#ifdef OVLB

void
make_engr_at(x,y,s,e_time,e_type)
int x,y;
const char *s;
long e_time;
xchar e_type;
{
	struct engr *ep;

	xchar exist_ward_type = 0;
	int exist_ward_id = 0;
	xchar exist_halu_ward = FALSE;
	xchar exist_complete_wards = 0;
	xchar exist_scuffed_wards = 0; 
	xchar exist_degraded_wards = 0;
	xchar exist_partial_wards = 0;

	if ((ep = engr_at(x,y)) != 0){
		exist_ward_type = ep->ward_type;
		exist_ward_id = ep->ward_id;
		exist_halu_ward = ep->halu_ward;
		exist_complete_wards = ep->complete_wards;
		exist_scuffed_wards = ep->scuffed_wards; 
		exist_degraded_wards = ep->degraded_wards;
		exist_partial_wards = ep->partial_wards;

	    del_engr_ward(ep);
	}
	ep = newengr(strlen(s) + 1);
	ep->nxt_engr = head_engr;
	head_engr = ep;
	ep->engr_x = x;
	ep->engr_y = y;
	ep->engr_txt = (char *)(ep + 1);
	Strcpy(ep->engr_txt, s);
	// /* engraving Elbereth shows wisdom */
	// if (!in_mklev && !strcmp(s,	"Elbereth")){
		// exercise(A_WIS, TRUE);
	// }
	ep->ward_type = exist_ward_type;
	ep->ward_id = exist_ward_id;
	ep->halu_ward = exist_halu_ward;
	ep->complete_wards = exist_complete_wards;
	ep->scuffed_wards = exist_scuffed_wards;
	ep->degraded_wards = exist_degraded_wards;
	ep->partial_wards = exist_partial_wards;
	ep->engr_time = e_time;
	ep->engr_type = e_type > 0 ? e_type : rnd(N_ENGRAVE-1);
	ep->engr_lth = strlen(s) + 1;
}

/* delete any engraving at location <x,y> */
void
del_engr_at(x, y)
int x, y;
{
	register struct engr *ep = engr_at(x, y);

	if (ep) del_engr(ep);
}

void
del_ward_at(x, y)
int x, y;
{
	register struct engr *ep = engr_at(x, y);

	if (ep) del_ward(ep);
}

void
del_engr_ward_at(x, y)
int x, y;
{
	register struct engr *ep = engr_at(x, y);

	if (ep) del_engr_ward(ep);
}

/*
 *	freehand - returns true if player has a free hand
 */
int
freehand()
{
	return((!uarm || uarm->otyp != STRAITJACKET || !(uarm->cursed)) && 
		(!uwep || !welded(uwep) ||
	   (!bimanual(uwep,youracedata) && (!uarms || !uarms->cursed))));
/*	if ((uwep && bimanual(uwep)) ||
	    (uwep && uarms))
		return(0);
	else
		return(1);*/
}

static NEARDATA const char styluses[] =
	{ ALL_CLASSES, ALLOW_NONE, TOOL_CLASS, WEAPON_CLASS, WAND_CLASS,
	  GEM_CLASS, RING_CLASS, 0 };

int
doengward()
{
    char c = 'n';
	if(u.wardsknown){
		c = yn_function("Do you want to scribe a warding sign?",
						ynqchars, 'q');
	}
	if(c == 'y') return doward();
	else if(c=='n') return doengrave();
	return 0;
}

/* Mohs' Hardness Scale:
 *  1 - Talc		 6 - Orthoclase
 *  2 - Gypsum		 7 - Quartz
 *  3 - Calcite		 8 - Topaz
 *  4 - Fluorite	 9 - Corundum
 *  5 - Apatite		10 - Diamond
 *
 * Since granite is a igneous rock hardness ~ 7, anything >= 8 should
 * probably be able to scratch the rock.
 * Devaluation of less hard gems is not easily possible because obj struct
 * does not contain individual oc_cost currently. 7/91
 *
 * steel     -	5-8.5	(usu. weapon)
 * diamond    - 10			* jade	     -	5-6	 (nephrite)
 * ruby       -  9	(corundum)	* turquoise  -	5-6
 * sapphire   -  9	(corundum)	* opal	     -	5-6
 * topaz      -  8			* glass      - ~5.5
 * emerald    -  7.5-8	(beryl)		* dilithium  -	4-5??
 * aquamarine -  7.5-8	(beryl)		* iron	     -	4-5
 * garnet     -  7.25	(var. 6.5-8)	* fluorite   -	4
 * agate      -  7	(quartz)	* brass      -	3-4
 * amethyst   -  7	(quartz)	* gold	     -	2.5-3
 * jasper     -  7	(quartz)	* silver     -	2.5-3
 * onyx       -  7	(quartz)	* copper     -	2.5-3
 * moonstone  -  6	(orthoclase)	* amber      -	2-2.5
 */

/* return 1 if action took 1 (or more) moves, 0 if error or aborted */
int
dogenengrave(mode)
int mode;
{
	boolean dengr = FALSE;	/* TRUE if we wipe out the current engraving */
	boolean doblind = FALSE;/* TRUE if engraving blinds the player */
	boolean doknown = FALSE;/* TRUE if we identify the stylus */
	boolean eow = FALSE;	/* TRUE if we are overwriting oep */
	boolean jello = FALSE;	/* TRUE if we are engraving in slime */
	boolean ptext = TRUE;	/* TRUE if we must prompt for engrave text */
	boolean teleengr =FALSE;/* TRUE if we move the old engraving */
	boolean zapwand = FALSE;/* TRUE if we remove a wand charge */
	xchar type = DUST;	/* Type of engraving made */
	char buf[BUFSZ];	/* Buffer for final/poly engraving text */
	char ebuf[BUFSZ];	/* Buffer for initial engraving text */
	char qbuf[QBUFSZ];	/* Buffer for query text */
	char post_engr_text[BUFSZ]; /* Text displayed after engraving prompt */
	const char *everb;	/* Present tense of engraving type */
	const char *eloc;	/* Where the engraving is (ie dust/floor/...) */
	char *sp;		/* Place holder for space count of engr text */
	int len = 0;	/* # of nonspace chars of new engraving text */
	int ward = 0;	/* ID number of the ward to be engraved */
	int perc;		/*percent of ward that could be drawn*/
	int maxelen;		/* Max allowable length of engraving text */
	struct engr *oep = engr_at(u.ux,u.uy);
				/* The current engraving */
	struct obj *otmp;	/* Object selected with which to engrave */
	int randWard = 0; /* random ward */
	xchar randHalu = FALSE; /* whether or not the randWard should be read as a real or hallucinatory ward */
	char *writer;
	char *word;
	
	if(mode == ENGRAVE_MODE)
		word = "write";
	else
		word = "draw";
	
	multi = 0;		/* moves consumed */
	nomovemsg = (char *)0;	/* occupation end message */

	buf[0] = (char)0;
	ebuf[0] = (char)0;
	post_engr_text[0] = (char)0;
	maxelen = BUFSZ - 1;
	if (is_demon(youracedata) || youracedata->mlet == S_VAMPIRE)
	    type = ENGR_BLOOD;

	/* Can the adventurer engrave at all? */
	if(!u.wardsknown && mode == WARD_MODE){
		You("don't know any wards.");
		return 0;
	} else if(mode == SEAL_MODE){
		if(Role_if(PM_EXILE)) binderup(); //reaply all known seals, in case of memory loss.
		if(!u.sealsKnown && !u.specialSealsKnown){
			You("don't know any seals.");
			return 0;
		}
	}

	if(u.uswallow) {
		if (is_animal(u.ustuck->data)) {
			if(mode == ENGRAVE_MODE) pline("What would you write?  \"Jonah was here\"?");
			else pline("What would you do, write \"Jonah was here\"?");
			return(0);
		} else if (is_whirly(u.ustuck->data)) {
			You_cant("reach the %s.", surface(u.ux,u.uy));
			return(0);
		} else
			jello = TRUE;
	} else if (is_lava(u.ux, u.uy)) {
		You_cant("%s on the lava!", word);
		return(0);
	} /*else if (is_pool(u.ux,u.uy, FALSE) || IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
		You_cant("draw on the water!");
		return(0);
	}*/else if(is_pool(u.ux,u.uy, FALSE) && !u.uinwater){
		You_cant("draw on the water!");
		return(0);
	}
	if(Weightless || Is_waterlevel(&u.uz)/* in bubble */ /*|| is_sky(u.ux, u.uy)*/) {
		You_cant("%s in thin air!", word);
		return(0);
	}
	if (cantwield(youracedata)) {
		You_cant("even hold anything!");
		return(0);
	}
	if (check_capacity((char *)0)) return (0);

	/* One may write with finger, or weapon, or wand, or..., or...
	 * Edited by GAN 10/20/86 so as not to change weapon wielded.
	 */
	if(mode == ENGRAVE_MODE)
		otmp = getobj(styluses,	"write with");
	else
		otmp = getobj(styluses,	"draw with");
	if(!otmp) return(0);		/* otmp == zeroobj if fingers */

	if (otmp == &zeroobj) writer = makeplural(body_part(FINGER));
	else writer = xname(otmp);

	/* There's no reason you should be able to write with a wand
	 * while both your hands are tied up.
	 */
	if (!freehand() && otmp != uwep && !otmp->owornmask) {
		You("have no free %s to %s with!", body_part(HAND), word);
		return(0);
	}

	if (jello) {
		You("tickle %s with your %s.", mon_nam(u.ustuck), writer);
		Your("message dissolves...");
		return(0);
	}
	if (otmp->oclass != WAND_CLASS && !can_reach_floor()) {
		You_cant("reach the %s!", surface(u.ux,u.uy));
		return(0);
	}
	if (IS_ALTAR(levl[u.ux][u.uy].typ)) {
		You("make a motion towards the altar with your %s.", writer);
		altar_wrath(u.ux, u.uy);
		return(0);
	}
	if(mode == ENGRAVE_MODE){
		if (IS_GRAVE(levl[u.ux][u.uy].typ)) {
		    if (otmp == &zeroobj) { /* using only finger */
			You("would only make a small smudge on the %s.",
				surface(u.ux, u.uy));
			return(0);
		    } else if (!levl[u.ux][u.uy].disturbed) {
			You("disturb the undead!");
			levl[u.ux][u.uy].disturbed = 1;
			(void) makemon(&mons[PM_GHOUL], u.ux, u.uy, NO_MM_FLAGS);
			exercise(A_WIS, FALSE);
			return(1);
		    }
		}
	}
	/* SPFX for items */

	switch (otmp->oclass) {
	    default:
	    case AMULET_CLASS:
	    case CHAIN_CLASS:
	    case POTION_CLASS:
	    case COIN_CLASS:
		break;

	    case RING_CLASS:
		/* "diamond" rings and others should work */
	    case GEM_CLASS:
		/* diamonds & other hard gems should work */
		if (objects[otmp->otyp].oc_tough) {
			type = ENGRAVE;
			break;
		}
		break;

	    case ARMOR_CLASS:
		if (is_boots(otmp)) {
			type = DUST;
			break;
		}
		/* fall through */
	    /* Objects too large to engrave with */
	    case BALL_CLASS:
	    case ROCK_CLASS:
/*	    case BED_CLASS:*/
		You_cant("%s with such a large object!", word);
		ptext = FALSE;
		break;

	    /* Objects too silly to engrave with */
	    case FOOD_CLASS:
	    case SCROLL_CLASS:
	    case SPBOOK_CLASS:
		Your("%s would get %s.", xname(otmp),
			is_ice(u.ux,u.uy) ?	"all frosty" :	"too dirty");
		ptext = FALSE;
		break;

	    case RANDOM_CLASS:	/* This should mean fingers */
		break;

	    /* The charge is removed from the wand before prompting for
	     * the engraving text, because all kinds of setup decisions
	     * and pre-engraving messages are based upon knowing what type
	     * of engraving the wand is going to do.  Also, the player
	     * will have potentially seen	"You wrest .." message, and
	     * therefore will know they are using a charge.
	     */
	    case WAND_CLASS:
		if (zappable(otmp)) {
		    check_unpaid(otmp);
		    zapwand = TRUE;
		    if (Levitation) ptext = FALSE;

		    switch (otmp->otyp) {
		    /* DUST wands */
		    default:
			break;

			/* NODIR wands */
		    case WAN_LIGHT:
		    case WAN_SECRET_DOOR_DETECTION:
		    case WAN_CREATE_MONSTER:
		    case WAN_WISHING:
		    case WAN_ENLIGHTENMENT:
			zapnodir(otmp);
			break;

			/* IMMEDIATE wands */
			/* If wand is "IMMEDIATE", remember to affect the
			 * previous engraving even if turning to dust.
			 */
		    case WAN_STRIKING:
			if(mode == ENGRAVE_MODE){
				Strcpy(post_engr_text,
				"The wand unsuccessfully fights your attempt to write!"
				);
			} else {
				Strcpy(post_engr_text,
				"The wand unsuccessfully fights your attempt to draw!"
				);
			}
			break;
		    case WAN_SLOW_MONSTER:
			if (!Blind) {
			   Sprintf(post_engr_text,
				  	"The bugs on the %s slow down!",
				   surface(u.ux, u.uy));
			}
			break;
		    case WAN_SPEED_MONSTER:
			if (!Blind) {
			   Sprintf(post_engr_text,
				  	"The bugs on the %s speed up!",
				   surface(u.ux, u.uy));
			}
			break;
		    case WAN_POLYMORPH:
			if(oep)  {
			    if (oep->engr_txt[0]) {
					type = (xchar)0;	/* random */
					(void) random_engraving(buf);
			    }
				if(oep->ward_id){
					randWard = rn2(10) ? 1 : rn2(10) ? randHaluWard() : 0;
					if(!randWard){
						randWard = rn2(NUMBER_OF_WARDS)+1;
						randHalu = FALSE;
					}
					else randHalu = TRUE;
				}
			    dengr = TRUE;
			}
			break;
		    case WAN_DRAINING:	/* KMH */
			if (oep) {
			    /*
			     * [ALI] Wand of draining give messages like
			     * either polymorph or cancellation/make
			     * invisible depending on whether the
			     * old engraving is completely wiped or not.
			     * Note: Blindness has slightly different
			     * effect than with wand of polymorph.
			     */
			    u_wipe_engr(5);
			    oep = engr_at(u.ux,u.uy);
			    if (!Blind) {
				if (!oep){
				    pline_The("engraving on the %s vanishes!",
				      surface(u.ux,u.uy));
				} else {
				    strcpy(buf, oep->engr_txt);
				    dengr = TRUE;
				}
			    }
			}
			break;
		    case WAN_NOTHING:
		    case WAN_UNDEAD_TURNING:
		    case WAN_OPENING:
		    case WAN_LOCKING:
		    case WAN_PROBING:
			break;

			/* RAY wands */
		    case WAN_MAGIC_MISSILE:
			ptext = TRUE;
			if (!Blind) {
			   Sprintf(post_engr_text,
				  	"The %s is riddled by bullet holes!",
				   surface(u.ux, u.uy));
			}
			break;

		    /* can't tell sleep from death - Eric Backus */
		    case WAN_SLEEP:
		    case WAN_DEATH:
			if(levl[u.ux][u.uy].typ == GRASS && otmp->otyp == WAN_DEATH){
				if (!Blind) {
				   Sprintf(post_engr_text,
						"The grass withers and dies!");
					levl[u.ux][u.uy].typ = SOIL;
				}
			} else if (!Blind) {
			   Sprintf(post_engr_text,
				  	"The bugs on the %s stop moving!",
				   surface(u.ux, u.uy));
			}
			break;

		    case WAN_COLD:
			if (!Blind)
			    Strcpy(post_engr_text,
				"A few ice cubes drop from the wand.");
			if(!oep || (oep->engr_type != BURN))
			    break;
		    case WAN_CANCELLATION:
		    case WAN_MAKE_INVISIBLE:
			if (oep && oep->engr_type != HEADSTONE) {
			    if (!Blind)
				pline_The("engraving on the %s vanishes!",
					surface(u.ux,u.uy));
			    dengr = TRUE;
			}
			break;
		    case WAN_TELEPORTATION:
			if (oep && oep->engr_type != HEADSTONE) {
			    if (!Blind)
				pline_The("engraving on the %s vanishes!",
					surface(u.ux,u.uy));
			    teleengr = TRUE;
			}
			break;

		    /* type = ENGRAVE wands */
		    case WAN_DIGGING:
			ptext = TRUE;
			type  = ENGRAVE;
			if(!objects[otmp->otyp].oc_name_known) {
			    if (flags.verbose)
				pline("This %s is a wand of digging!",
				xname(otmp));
			    doknown = TRUE;
			}
			if (!Blind)
			    Strcpy(post_engr_text,
				IS_GRAVE(levl[u.ux][u.uy].typ) ?
				"Chips fly out from the headstone." :
				is_ice(u.ux,u.uy) ?
				"Ice chips fly up from the ice surface!" :
				"Gravel flies up from the floor.");
			else
			    Strcpy(post_engr_text,	"You hear drilling!");
			break;

		    /* type = BURN wands */
		    case WAN_FIRE:
			ptext = TRUE;
			type  = BURN;
			if(!objects[otmp->otyp].oc_name_known) {
			if (flags.verbose)
			    pline("This %s is a wand of fire!", xname(otmp));
			    doknown = TRUE;
			}
			Strcpy(post_engr_text,
				Blind ?	"You feel the wand heat up." :
					"Flames fly from the wand.");
			if(levl[u.ux][u.uy].typ == GRASS)
				levl[u.ux][u.uy].typ = SOIL;
			break;
		    case WAN_LIGHTNING:
			ptext = TRUE;
			type  = BURN;
			if(!objects[otmp->otyp].oc_name_known) {
			    if (flags.verbose)
				pline("This %s is a wand of lightning!",
					xname(otmp));
			    doknown = TRUE;
			}
			if (!Blind) {
			    Strcpy(post_engr_text,
				   	"Lightning arcs from the wand.");
			    doblind = TRUE;
			} else
			    Strcpy(post_engr_text, "You hear crackling!");
			break;

		    /* type = MARK wands */
		    /* type = ENGR_BLOOD wands */
		    }
		} else /* end if zappable */
		    if (!can_reach_floor()) {
			You_cant("reach the %s!", surface(u.ux,u.uy));
			return(0);
		    }
		break;

	    case WEAPON_CLASS:
		if (otmp->oartifact == ART_PEN_OF_THE_VOID &&
				mvitals[PM_ACERERAK].died > 0 && (otmp->ovar1 & SEAL_ANDREALPHUS)
		) {
			type = BURN;
		} else if (is_blade(otmp)) {
		    if ((int)otmp->spe > -3 
			|| levl[u.ux][u.uy].typ == GRASS 
			|| levl[u.ux][u.uy].typ == SOIL
			|| levl[u.ux][u.uy].typ == SAND
		)
				type = ENGRAVE;
		    else
				Your("%s too dull for engraving.", aobjnam(otmp,"are"));
		} else if(otmp->otyp == RAYGUN){
			if(otmp->altmode == AD_DISN && otmp->ovar1 >= 15){
				otmp->ovar1 -= 15;
				if (dighole(FALSE)){
					Your("raygun disintegrated the floor!");
					if(!Blind && !resists_blnd(&youmonst)) {
						You("are blinded by the flash!");
						make_blinded((long)rnd(50),FALSE);
						if (!Blind) Your1(vision_clears);
					}
					if(levl[u.ux][u.uy].typ == GRASS)
						levl[u.ux][u.uy].typ = SOIL;
					return 1;
				} else {
					ptext = TRUE;
					type  = BURN;
					if(!Blind){
						Strcpy(post_engr_text,
							"A brilliant beam shoots from the raygun.");
						doblind = TRUE;
					}
					if(levl[u.ux][u.uy].typ == GRASS)
						levl[u.ux][u.uy].typ = SOIL;
				}
			} else if(otmp->altmode == AD_DEAD && otmp->ovar1 >= 10){
				otmp->ovar1 -= 10;
				ptext = TRUE;
				if (!Blind) {
					if(levl[u.ux][u.uy].typ == GRASS){
					   Sprintf(post_engr_text,
							"The grass withers and dies!");
						levl[u.ux][u.uy].typ = SOIL;
					} else Sprintf(post_engr_text,
						   "The bugs on the %s stop moving!",
						   surface(u.ux, u.uy));
				}
			} else if(otmp->altmode == AD_FIRE && otmp->ovar1 >= 2){
				otmp->ovar1 -= 2;
				ptext = TRUE;
				type  = BURN;
				Strcpy(post_engr_text,
					Blind ? "You feel the raygun heat up." :
						"A heat ray shoots from the raygun.");
				if(levl[u.ux][u.uy].typ == GRASS)
					levl[u.ux][u.uy].typ = SOIL;
			} else if(otmp->ovar1 >= 1){
				otmp->ovar1 -= 1;
				ptext = TRUE;
				if (!Blind) {
				   Sprintf(post_engr_text,
					   "The bugs on the %s stop moving!",
					   surface(u.ux, u.uy));
				}
			}
		}
		break;

	    case TOOL_CLASS:
		if (is_lightsaber(otmp)) {
			if (litsaber(otmp)) type = BURN;
			else Your("%s is deactivated!", aobjnam(otmp,"are"));
		} else if(otmp == ublindf) {
		    pline(
		"That is a bit difficult to engrave with, don't you think?");
		    return(0);
		}
		switch (otmp->otyp)  {
		    case MAGIC_MARKER:
			if (otmp->spe <= 0)
			    Your("marker has dried out.");
			else
			    type = MARK;
			break;
		    case TOWEL:
			/* Can't really engrave with a towel */
			ptext = FALSE;
			if (oep)
			    if ((oep->engr_type == DUST ) ||
				(oep->engr_type == ENGR_BLOOD) ||
				(oep->engr_type == MARK )) {
				if (!Blind)
				    You("wipe out the message here.");
				else
				    Your("%s %s %s.", xname(otmp),
					 otense(otmp, "get"),
					 is_ice(u.ux,u.uy) ?
					 "frosty" : 
						(levl[u.ux][u.uy].typ == SAND && !In_mithardir_quest(&u.uz)) ?
						"sandy" :
						"dusty");
				dengr = TRUE;
			    } else
				Your("%s can't wipe out this engraving.",
				     xname(otmp));
			else{
			    Your("%s %s %s.", xname(otmp), otense(otmp, "get"),
				  is_ice(u.ux,u.uy) ? "frosty" : 
					(levl[u.ux][u.uy].typ == SAND && !In_mithardir_quest(&u.uz)) ?
					"sandy" :
					"dusty");
			}
			break;
		    default:
				if(spec_ability3(otmp, SPFX3_ENGRV)) type = ENGRAVE;
			break;
		}
		break;

	    case VENOM_CLASS:
#ifdef WIZARD
		if (wizard) {
		    pline("Writing a poison pen letter??");
		    break;
		}
#endif
	    case ILLOBJ_CLASS:
		impossible("You're engraving with an illegal object!");
		break;
	}
	if(type == DUST && levl[u.ux][u.uy].typ == GRASS){
		You_cant("make legible marks in grass with just your %s.", writer);
		return(0);
	}
	if(type == ENGRAVE && levl[u.ux][u.uy].typ == SAND){
		type = DUST;
	}
	if(mode == ENGRAVE_MODE){
		if (IS_GRAVE(levl[u.ux][u.uy].typ)) {
		    if (type == ENGRAVE || type == 0)
			type = HEADSTONE;
		    else {
			/* ensures the "cannot wipe out" case */
			type = DUST;
			dengr = FALSE;
			teleengr = FALSE;
			buf[0] = (char)0;
		    }
		}
	}
	/* End of implement setup */

	/* Identify stylus */
	if (doknown) {
	    makeknown(otmp->otyp);
	    more_experienced(0,10);
	}

	if (teleengr) {
	    rloc_engr(oep);
	    oep = (struct engr *)0;
	}

	if (dengr) {
	    del_engr_ward(oep);
	    oep = (struct engr *)0;
	}

	/* Something has changed the engraving here */
	if (*buf || randWard) {
		if(*buf){
			make_engr_at(u.ux, u.uy, buf, moves, type);
			oep = engr_at(u.ux,u.uy);
		}
		if(randWard){
			oep = engr_at(u.ux,u.uy);
			if(!oep){
				make_engr_at(u.ux, u.uy,	"", moves, DUST);
				oep = engr_at(u.ux,u.uy);
			}
			oep->ward_id = randWard;
			oep->halu_ward = randHalu;
			oep->ward_type = rnd(N_ENGRAVE-1);
			oep->complete_wards = 1;
		}
	    if(!Blind){ 
			/*pline_The("engraving now reads: \"%s\".", buf);*/
			pline_The("engraving here has changed.");
			read_engr_at(u.ux,u.uy);
		}
	    ptext = FALSE;
	}

	if (zapwand && (otmp->spe < 0)) {
	    pline("%s %sturns to dust.",
		  The(xname(otmp)), Blind ? "" : "glows violently, then ");
	    if (!IS_GRAVE(levl[u.ux][u.uy].typ))
		You("are not going to get anywhere trying to %s in the %s with your dust.",
		    word, is_ice(u.ux,u.uy) ?	"frost" :	
				(levl[u.ux][u.uy].typ == SAND && !In_mithardir_quest(&u.uz)) ?
				"sand" :
				"dust");
	    useup(otmp);
	    ptext = FALSE;
	}

	if (!ptext) {		/* Early exit for some implements. */
	    if (otmp->oclass == WAND_CLASS && !can_reach_floor())
		You_cant("reach the %s!", surface(u.ux,u.uy));
	    return(1);
	}

	/* Special effects should have deleted the current engraving (if
	 * possible) by now.
	 */
	if(mode == ENGRAVE_MODE){
		if (oep) {
		    register char c = 'n';
	
		    /* Give player the choice to add to engraving. */
	
		    if (type == HEADSTONE) {
			/* no choice, only append */
			c = 'y';
		    } else if ( (type == oep->engr_type) && (!Blind ||
			 (oep->engr_type == BURN) || (oep->engr_type == ENGRAVE)) ) {
			c = yn_function("Do you want to add to the current engraving?",
					ynqchars, 'y');
			if (c == 'q') {
			    pline1(Never_mind);
			    return(0);
			}
		    }
	
		    if (c == 'n' || Blind) {
	
			if( (oep->engr_type == DUST) || (oep->engr_type == ENGR_BLOOD) ||
			    (oep->engr_type == MARK) ) {
			    if (!Blind) {
				You("wipe out the message that was %s here.",
				    ((oep->engr_type == DUST)  ? (
						(levl[u.ux][u.uy].typ == SAND && !In_mithardir_quest(&u.uz)) ?
						"written in the sand" :
						"written in the dust"
					) :
				    ((oep->engr_type == ENGR_BLOOD) ? "scrawled in blood"   :
								 "written")));
				del_engr(oep);
				oep = engr_at(u.ux,u.uy);
			    } else
			   /* Don't delete engr until after we *know* we're engraving */
				eow = TRUE;
			} else
			    if ( (type == DUST) || (type == MARK) || (type == ENGR_BLOOD) ) {
				You(
				 "cannot wipe out the message that is %s the %s here.",
				 oep->engr_type == BURN ?
				   (is_ice(u.ux,u.uy) ? "melted into" : 
					levl[u.ux][u.uy].typ == SAND ? "fused into" :
				   "burned into") :
				   "engraved in", surface(u.ux,u.uy));
				return(1);
			    } else
				if ( (type != oep->engr_type) || (c == 'n') ) {
				    if (!Blind || can_reach_floor())
					You("will overwrite the current message.");
				    eow = TRUE;
				}
		    }
		}
	} else if(mode == WARD_MODE){
		if (oep && oep->ward_id) {
			char c = 'n';
	
		    /* Give player the choice to add to ward. */
	
	/*	    if (type == HEADSTONE) {
			// headstones are not big enough for wards
				pline("This headstone is not big enough for drawing one");
			    return(0);
		    } else*/ if ( (type == oep->ward_type) && !Hallucination && (!Blind ||
			 (oep->ward_type == BURN) || (oep->ward_type == ENGRAVE)) && oep->ward_id <= LAST_WARD) {
				c = yn_function("Do you want to reinforce the existing ward?",
						ynqchars, 'y');
				if (c == 'q') {
					pline1(Never_mind);
					return(0);
				}
		    }
			
		    if (c == 'n' || Blind) {
	
				if( (oep->ward_type == DUST) || (oep->ward_type == ENGR_BLOOD) ||
					(oep->ward_type == MARK) ) {
					if (!Blind) {
					You("wipe out the ward that was %s here.",
						((oep->ward_type == DUST)  ?	(
							(levl[u.ux][u.uy].typ == SAND && !In_mithardir_quest(&u.uz)) ?
							"drawn in the sand" :
							"drawn in the dust"
						) :
						((oep->ward_type == ENGR_BLOOD) ?	"painted in blood"   :
										"drawn")));
					del_ward(oep);
					oep = engr_at(u.ux,u.uy);
					} else
				   /* Don't delete engr until after we *know* we're engraving */
					eow = TRUE;
				} else{
					if ( (type == DUST) || (type == MARK) || (type == ENGR_BLOOD) ) {
					You(
						"cannot wipe out the ward that is %s the %s here.",
					 oep->ward_type == BURN ?
					   (is_ice(u.ux,u.uy) ?	"melted into" :	
						levl[u.ux][u.uy].typ == SAND ? "fused into" :
					   "burned into") :
					  	"engraved in", surface(u.ux,u.uy));
					return(1);
					} else
					if ( (type != oep->ward_type) || (c == 'n') ) {
						if (!Blind || can_reach_floor())
						You("will replace the current ward.");
						eow = TRUE;
					}
				}
		    }
			else if(c == 'y'){
				int newWards = get_num_wards_added(oep->ward_id, oep->complete_wards);
				int increment;
				if(oep->complete_wards >= 7 || newWards < 1){
					pline("The warding sign can be reinforced no further!");
					return 0;
				}
				/*pline("%d to be added, %d there already", newWards, oep->complete_wards);*/
				boolean continue_loop = TRUE;
				ward = oep->ward_id;
				len = wardStrokes[ward][oep->complete_wards];
				increment = len/newWards;
				while(newWards && continue_loop){
					if(oep->scuffed_wards){
						len -= (int)(increment*.9);
						newWards--;
						oep->scuffed_wards--;
					}
					else if(oep->degraded_wards){
						len -= (int) (increment*.75);
						newWards--;
						oep->degraded_wards--;
					}
					else if(oep->partial_wards){
						len -= (int) (increment*.5);
						newWards--;
						oep->partial_wards--;
					}
					else continue_loop = FALSE;
				}
			}
		}
	} else {
		if (oep && oep->ward_id) {
			if( (oep->ward_type == DUST) || (oep->ward_type == ENGR_BLOOD) ||
				(oep->ward_type == MARK) ) {
				if (!Blind) {
				You("wipe out the drawing that was %s here.",
					((oep->ward_type == DUST)  ?	(
						(levl[u.ux][u.uy].typ == SAND && !In_mithardir_quest(&u.uz)) ?
						"drawn in the sand" :
						"drawn in the dust"
					) :
					((oep->ward_type == ENGR_BLOOD) ?	"painted in blood"   :
									"drawn")));
				del_ward(oep);
				oep = engr_at(u.ux,u.uy);
				} else
			   /* Don't delete engr until after we *know* we're engraving */
				eow = TRUE;
			} else{
				if ( (type == DUST) || (type == MARK) || (type == ENGR_BLOOD) ) {
				You(
					"cannot wipe out the drawing that is %s the %s here.",
				 oep->ward_type == BURN ?
				   (is_ice(u.ux,u.uy) ?	"melted into" :	
					levl[u.ux][u.uy].typ == SAND ? "fused into" :
				   "burned into") :
				  	"engraved in", surface(u.ux,u.uy));
				return(1);
				} else
				if ( (type != oep->ward_type) ) {
					if (!Blind || can_reach_floor())
					You("will replace the current drawing.");
					eow = TRUE;
				}
			}
		}
	}

	eloc = surface(u.ux,u.uy);
	switch(type){
	    default:
		if(mode == ENGRAVE_MODE)
			everb = (oep && !eow ? "add to the weird writing on" :
					       "write strangely on");
		else everb = (oep && !eow ?	"add to the weird drawing on" :
				      	"draw strangely on");
		break;
	    case DUST:
		if(mode == ENGRAVE_MODE)
			everb = (oep && !eow ? "add to the writing in" :
					       "write in");
		else everb = (oep && !eow ?	"add to the drawing in" :
				      	"draw in");
		eloc = is_ice(u.ux,u.uy) ?	"frost" : 
			(levl[u.ux][u.uy].typ == SAND && !In_mithardir_quest(&u.uz)) ?
			"sand" :
			"dust";
		break;
	    case HEADSTONE:
		if(mode == ENGRAVE_MODE)
			everb = (oep && !eow ? "add to the epitaph on" :
					       "engrave on");
		else everb = (oep && !eow ?	"add to the drawing on" :
				      	"draw on");
		break;
	    case ENGRAVE:
		if(mode == ENGRAVE_MODE)
			everb = (oep && !eow ? "add to the engraving in" :
					       "engrave in");
		else everb = (oep && !eow ?	"add to the drawing in" :
				      	"draw in");
		break;
	    case BURN:
		if(mode == ENGRAVE_MODE)
			everb = (oep && !eow ?
				( is_ice(u.ux,u.uy) ? "add to the text melted into" :
				  levl[u.ux][u.uy].typ == SAND ? "add to the text fused into" :
						      "add to the text burned into") :
				( is_ice(u.ux,u.uy) ? "melt into" : 
					levl[u.ux][u.uy].typ == SAND ? "fuse into" :
					"burn into"));
		else everb = (oep && !eow ?
			( is_ice(u.ux,u.uy) ?	"add to the drawing melted into" :
				  levl[u.ux][u.uy].typ == SAND ? "add to the drawing fused into" :
					     	"add to the drawing burned into") :
			( is_ice(u.ux,u.uy) ?	"melt into" :	
				levl[u.ux][u.uy].typ == SAND ? "fuse into" :
				"burn into"));
		break;
	    case MARK:
		if(mode == ENGRAVE_MODE)
			everb = (oep && !eow ? "add to the graffiti on" :
					       "scribble on");
		else everb = (oep && !eow ?	"add to the graffiti on" :
				      	"draw on");
		break;
	    case ENGR_BLOOD:
		if(mode == ENGRAVE_MODE)
			everb = (oep && !eow ? "add to the scrawl on" :
					       "scrawl on");
		else everb = (oep && !eow ?	"add to the drawing on" :
				      	"draw on");
		break;
	}

	/* Tell adventurer what is going on */
	if (otmp != &zeroobj)
	    You("%s the %s with %s.", everb, eloc, doname(otmp));
	else
	    You("%s the %s with your %s.", everb, eloc,
		makeplural(body_part(FINGER)));

	/* Prompt for engraving! */
	if(mode == ENGRAVE_MODE){
		Sprintf(qbuf,"What do you want to %s the %s here?", everb, eloc);
		getlin(qbuf, ebuf);
	
		if(Hallucination && !rn2(20)){
			ebuf[0] = (char)0;
			Strcpy(ebuf, haluMesg[rn2(SIZE(haluMesg))]);		
		}
		/* Count the actual # of chars engraved not including spaces */
		len = strlen(ebuf);
		for (sp = ebuf; *sp; sp++) if (isspace(*sp)) len -= 1;

		if (len == 0 || index(ebuf, '\033')) {
		    if (zapwand) {
			if (!Blind)
			    pline("%s, then %s.",
				  Tobjnam(otmp,	"glow"), otense(otmp,	"fade"));
			return(1);
		    } else {
				pline1(Never_mind);
				return(0);
		    }
		}
	
		/* A single `x' is the traditional signature of an illiterate person */
		if (len != 1 || (!index(ebuf, 'x') && !index(ebuf, 'X')))
		    u.uconduct.literate++;
	
		/* Mix up engraving if surface or state of mind is unsound.
		   Note: this won't add or remove any spaces. */
		for (sp = ebuf; *sp; sp++) {
		    if (isspace(*sp)) continue;
		    if (((type == DUST || type == ENGR_BLOOD) && !rn2(25)) ||
			    (Blind && !rn2(11)) || (Confusion && !rn2(7)) ||
			    (Stunned && !rn2(4)) || (Hallucination && !rn2(7)))
			*sp = ' ' + rnd(96 - 2);	/* ASCII '!' thru '~'
							   (excludes ' ' and DEL) */
		}
	
		/* Previous engraving is overwritten */
		if (eow) {
		    del_engr(oep);
		    oep = engr_at(u.ux,u.uy);
		}
	} else if(mode == WARD_MODE){
		if(!len){
			ward = pick_ward(FALSE);
			len = wardStrokes[ward][0];
		}
		if (ward == 0 || index(ebuf, '\033')) {
		    if (zapwand) {
				if (!Blind)
					pline("%s, then %s.",
					  Tobjnam(otmp,	"glow"), otense(otmp,	"fade"));
				return(1);
		    } else {
				pline1(Never_mind);
				return(0);
		    }
		}
		if(u.sealsActive&SEAL_CHUPOCLOPS) unbind(SEAL_CHUPOCLOPS,TRUE); 
		u.uconduct.wardless++;
		
		if (eow) {
			del_ward(oep);
			oep = engr_at(u.ux,u.uy);
		}
	} else {
		if(!len){
			ward = pick_seal();
			len = 5;//seals are always 5.
		}
		if (ward == 0 || index(ebuf, '\033')) {
		    if (zapwand) {
				if (!Blind)
					pline("%s, then %s.",
					  Tobjnam(otmp,	"glow"), otense(otmp,	"fade"));
				return(1);
		    } else {
				pline1(Never_mind);
				return(0);
		    }
		}
		if (eow) {
			del_ward(oep);
			oep = engr_at(u.ux,u.uy);
		}
	}

	/* Figure out how long it took to engrave, and if player has
	 * engraved too much.
	 */
	switch(type){
	    default:
			multi = -(len/10);
			if (multi){
				if(mode == ENGRAVE_MODE)
					nomovemsg =	"You finish your weird engraving.";
				else nomovemsg =	"You finish your weird drawing.";
			}
		break;
	    case DUST:
			multi = -(len/10);
			if (multi){
				if(mode == ENGRAVE_MODE)
					nomovemsg =	
						(levl[u.ux][u.uy].typ == SAND && !In_mithardir_quest(&u.uz)) ?
						"You finish writing in the sand." :
						"You finish writing in the dust.";
				else nomovemsg = 
					(levl[u.ux][u.uy].typ == SAND && !In_mithardir_quest(&u.uz)) ?
					"You finish drawing in the sand." :
					"You finish drawing in the dust.";
			}
		break;
	    case HEADSTONE:
	    case ENGRAVE:
			multi = -(len/10);
			if ((otmp->oclass == WEAPON_CLASS || spec_ability3(otmp, SPFX3_ENGRV)) 
				&& ((otmp->otyp != ATHAME && !spec_ability3(otmp, SPFX3_ENGRV)) || otmp->cursed)
			) {
				if(levl[u.ux][u.uy].typ == GRASS 
				|| levl[u.ux][u.uy].typ == SOIL
				|| levl[u.ux][u.uy].typ == SAND
				) multi = -(len/2);
				else multi = -len;
				if(otmp->otyp == CRYSTAL_SWORD
				|| levl[u.ux][u.uy].typ == GRASS
				|| levl[u.ux][u.uy].typ == SOIL
				|| levl[u.ux][u.uy].typ == SAND
				) maxelen = len;
				else maxelen = ((otmp->spe + 3) * 2) + 1;
				/* -2 = 3, -1 = 5, 0 = 7, +1 = 9, +2 = 11
				 * Note: this does not allow a +0 anything (except
				 *	 an athame) to engrave	"Elbereth" all at once.
				 *	 However, you could now engrave	"Elb", then
				 *		"ere", then	"th".
				 */
				if(otmp->otyp != CRYSTAL_SWORD
				&& levl[u.ux][u.uy].typ != GRASS
				&& levl[u.ux][u.uy].typ != SOIL
				&& levl[u.ux][u.uy].typ != SAND
				){
					Your("%s dull.", aobjnam(otmp,	"get"));
					if (otmp->unpaid) {
						struct monst *shkp = shop_keeper(*u.ushops);
						if (shkp) {
							You("damage it, you pay for it!");
							bill_dummy_object(otmp);
						}
					}
					if (len > maxelen) {
						multi = -maxelen;
						otmp->spe = -3;
					} else if (len > 1)
						otmp->spe -= len >> 1;
					else otmp->spe -= 1; /* Prevent infinite engraving */
				}
			} else {
				if ( (otmp->oclass == RING_CLASS) ||
					(otmp->oclass == GEM_CLASS) 
				)
					multi = -len;
				if (multi){
					if(mode == ENGRAVE_MODE)
						nomovemsg = "You finish engraving.";
					else nomovemsg =	"You finish drawing.";
				}
			}
		break;
	    case BURN:
			multi = -(len/10);
			if(is_lightsaber(otmp) && otmp->oartifact != ART_INFINITY_S_MIRRORED_ARC && otmp->otyp != KAMEREL_VAJRA){
				maxelen = ((otmp->age/101) + 1)*10;
				if (len > maxelen) {
					multi = -(maxelen/10);
					otmp->age = 0;
				} else otmp->age += (multi-1)*100; //NOTE: multi is negative
			}
			if (multi){
				if(mode == ENGRAVE_MODE)
					nomovemsg = is_ice(u.ux,u.uy) ?
					"You finish melting your message into the ice.":
					"You finish burning your message into the floor.";
				else nomovemsg = is_ice(u.ux,u.uy) ?
				"You finish melting your drawing into the ice.":
				"You finish burning your drawing into the floor.";
			}
		break;
	    case MARK:
			multi = -(len/10);
			if ((otmp->oclass == TOOL_CLASS) &&
				(otmp->otyp == MAGIC_MARKER)) {
				maxelen = (otmp->spe) * 2; /* one charge / 2 letters */
				if (len > maxelen) {
				Your("marker dries out.");
				otmp->spe = 0;
				multi = -(maxelen/10);
				} else
				if (len > 1) otmp->spe -= len >> 1;
				else otmp->spe -= 1; /* Prevent infinite grafitti */
			}
			if (multi) nomovemsg =	"You finish defacing the dungeon.";
		break;
	    case ENGR_BLOOD:
			multi = -(len/10);
			if (multi) nomovemsg =	"You finish scrawling.";
		break;
	}

	/* Chop engraving down to size if necessary */
	if(mode == ENGRAVE_MODE){
		if (len > maxelen) {
		    for (sp = ebuf; (maxelen && *sp); sp++)
			if (!isspace(*sp)) maxelen--;
		    if (!maxelen && *sp) {
			*sp = (char)0;
			if (multi) nomovemsg =	"You cannot write any more.";
			You("only are able to write \"%s\"", ebuf);
		    }
		}
	
		/* Add to existing engraving */
		if (oep) Strcpy(buf, oep->engr_txt);
	
		(void) strncat(buf, ebuf, (BUFSZ - (int)strlen(buf) - 1));
	
		make_engr_at(u.ux, u.uy, buf, (moves - multi), type);
		if(sengr_at("Elbereth", u.ux, u.uy)){
			u.uconduct.elbereth++;
		}
	} else if(mode == WARD_MODE){
		if (len > maxelen) {
			int perc = (len*100)/maxelen;
			if (multi) nomovemsg =	"Unfortunatly, you can't complete the ward.";
			else You("can't complete the ward.");
		} else perc = 100;
		
		if (oep && oep->ward_id){
			if(perc == 100) oep->complete_wards += get_num_wards_added(oep->ward_id, oep->complete_wards);
			else if(perc >= 90) oep->scuffed_wards += get_num_wards_added(oep->ward_id, oep->scuffed_wards);
			else if(perc >= 75) oep->degraded_wards += get_num_wards_added(oep->ward_id, oep->degraded_wards);
			else if(perc >= 50) oep->partial_wards += get_num_wards_added(oep->ward_id, oep->partial_wards);
		}
		else if(oep){
			if(!Hallucination || !rn2(4)){
				oep->ward_id = ward;
				oep->ward_type = type;
				if(perc == 100) oep->complete_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 90) oep->scuffed_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 75) oep->degraded_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 50) oep->partial_wards += get_num_wards_added(oep->ward_id, 0);
			}
			else{
				oep->ward_id = rn2(4) ? 1 : rn2(100) ? randHaluWard() : 0;
				if(!oep->ward_id){
					oep->ward_id = rn2(NUMBER_OF_WARDS)+1;
					oep->halu_ward = FALSE;
				}
				else oep->halu_ward = TRUE;
				oep->ward_type = type;
				if(oep->halu_ward) oep->complete_wards = 1;
				else if(perc == 100) oep->complete_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 90) oep->scuffed_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 75) oep->degraded_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 50) oep->partial_wards += get_num_wards_added(oep->ward_id, 0);
			}
		}
		else if(perc > 50){
			make_engr_at(u.ux, u.uy,	"", (moves - multi), DUST); /* absense of text =  dust */
			oep = engr_at(u.ux,u.uy);
			if(!Hallucination){
				oep->ward_id = ward;
				oep->ward_type = type;
				if(perc == 100) oep->complete_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 90) oep->scuffed_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 75) oep->degraded_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 50) oep->partial_wards += get_num_wards_added(oep->ward_id, 0);
			}
			else{
				oep->ward_id = rn2(4) ? 1 : rn2(100) ? randHaluWard() : 0;
				if(!oep->ward_id){
					oep->ward_id = rn2(NUMBER_OF_WARDS)+1;
					oep->halu_ward = FALSE;
				}
				else oep->halu_ward = TRUE;
				oep->ward_type = type;
				if(oep->halu_ward) oep->complete_wards = 1;
				else if(perc == 100) oep->complete_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 90) oep->scuffed_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 75) oep->degraded_wards += get_num_wards_added(oep->ward_id, 0);
				else if(perc >= 50) oep->partial_wards += get_num_wards_added(oep->ward_id, 0);
			}
		}
	} else {
		if (len > maxelen) {
			perc = (len*100)/maxelen;
			
			if (multi) nomovemsg =	"Unfortunatly, you can't complete the seal.";
			else You("can't complete the seal.");
		} else perc = 100;
		if (oep && oep->ward_id){
				oep->ward_id = ward;
				oep->ward_type = type;
				if(perc == 100) oep->complete_wards = 1;
				else if(perc >= 90) oep->scuffed_wards = 1;
				else if(perc >= 75) oep->degraded_wards = 1;
				else if(perc >= 50) oep->partial_wards = 1;
				oep->engr_time = moves - multi;
		}
		else if(oep){
			if(!Hallucination || !rn2(4)){
				oep->ward_id = ward;
				oep->ward_type = type;
				if(perc == 100) oep->complete_wards = 1;
				else if(perc >= 90) oep->scuffed_wards = 1;
				else if(perc >= 75) oep->degraded_wards = 1;
				else if(perc >= 50) oep->partial_wards = 1;
				oep->engr_time = moves - multi;
			}
			else{
				oep->ward_id = rn2(4) ? 1 : rn2(100) ? randHaluWard() : 0;
				if(!oep->ward_id){
					oep->ward_id = rn2(NUMBER_OF_WARDS)+1;
					oep->halu_ward = FALSE;
				}
				else oep->halu_ward = TRUE;
				oep->ward_type = type;
				if(perc == 100 || oep->halu_ward) oep->complete_wards = 1;
				else if(perc >= 90) oep->scuffed_wards = 1;
				else if(perc >= 75) oep->degraded_wards = 1;
				else if(perc >= 50) oep->partial_wards = 1;
				oep->engr_time = moves - multi;
			}
		}
		else{
			make_engr_at(u.ux, u.uy,	"", (moves - multi), DUST); /* absense of text =  dust */
			oep = engr_at(u.ux,u.uy);
			if(!Hallucination){
				oep->ward_id = ward;
				oep->ward_type = type;
				if(perc == 100) oep->complete_wards = 1;
				else if(perc >= 90) oep->scuffed_wards = 1;
				else if(perc >= 75) oep->degraded_wards = 1;
				else if(perc >= 50) oep->partial_wards = 1;
			}
			else{
				oep->ward_id = rn2(4) ? 1 : rn2(100) ? randHaluWard() : 0;
				if(!oep->ward_id){
					oep->ward_id = rn2(NUMBER_OF_WARDS)+1;
					oep->halu_ward = FALSE;
				}
				else oep->halu_ward = TRUE;
				oep->ward_type = type;
				if(perc == 100 || oep->halu_ward) oep->complete_wards = 1;
				else if(perc >= 90) oep->scuffed_wards = 1;
				else if(perc >= 75) oep->degraded_wards = 1;
				else if(perc >= 50) oep->partial_wards = 1;
			}
		}
	}
	if (post_engr_text[0]) pline1(post_engr_text);

	if (doblind && !resists_blnd(&youmonst)) {
	    You("are blinded by the flash!");
	    make_blinded((long)rnd(50),FALSE);
	    if (!Blind) Your1(vision_clears);
	}

	return(1);
}

/* return 1 if action took 1 (or more) moves, 0 if error or aborted */
int
doengrave()
{
	return dogenengrave(ENGRAVE_MODE);
}

/* return 1 if action took 1 (or more) moves, 0 if error or aborted */
int
doward()
{
	return dogenengrave(WARD_MODE);
}

int
random_unknown_ward()
{
	int options[FIRST_RUNE];
	int num_unknown=0;
	
	if(~u.wardsknown & WARD_HEPTAGRAM){
		options[num_unknown++] = HEPTAGRAM;
	}
	if(~u.wardsknown & WARD_GORGONEION){
		options[num_unknown++] = GORGONEION;	/* must be non-zero */
	}
	if(~u.wardsknown & WARD_ACHERON){
		options[num_unknown++] = CIRCLE_OF_ACHERON;	/* must be non-zero */
	}
	if(~u.wardsknown & WARD_PENTAGRAM){
		options[num_unknown++] = PENTAGRAM;	/* must be non-zero */
	}
	if(~u.wardsknown & WARD_HEXAGRAM){
		options[num_unknown++] = HEXAGRAM;	/* must be non-zero */
	}
	if(~u.wardsknown & WARD_HAMSA){
		options[num_unknown++] = HAMSA;	/* must be non-zero */
	}
	if(~u.wardsknown & WARD_ELDER_SIGN){
		options[num_unknown++] = ELDER_SIGN;	/* must be non-zero */
	}
	if(~u.wardsknown & WARD_EYE){
		options[num_unknown++] = ELDER_ELEMENTAL_EYE;	/* must be non-zero */
	}
	if(~u.wardsknown & WARD_QUEEN){
		options[num_unknown++] = SIGN_OF_THE_SCION_QUEEN;	/* must be non-zero */
	}
	if(~u.wardsknown & WARD_CAT_LORD){
		options[num_unknown++] = CARTOUCHE_OF_THE_CAT_LORD;	/* must be non-zero */
	}
	if(~u.wardsknown & WARD_GARUDA){
		options[num_unknown++] = WINGS_OF_GARUDA;	/* must be non-zero */
	}
	if(!rn2(100) || (!num_unknown && !rn2(10))){ /*!num_unknown: ie, if the character knows all non-legendary wards*/
		if(~u.wardsknown & WARD_CTHUGHA){
			options[num_unknown++] = SIGIL_OF_CTHUGHA;	/* must be non-zero */
		}
		if(~u.wardsknown & WARD_ITHAQUA){
			options[num_unknown++] = BRAND_OF_ITHAQUA;	/* must be non-zero */
		}
		if(~u.wardsknown & WARD_KARAKAL){
			options[num_unknown++] = TRACERY_OF_KARAKAL;	/* must be non-zero */
		}
		if(~u.wardsknown & WARD_YELLOW){
			options[num_unknown++] = YELLOW_SIGN;	/* must be non-zero */
		}
	}
	if(num_unknown){
		return options[rn2(num_unknown)];
	}
	else return -1;
}

int
pick_ward(describe)
int describe;
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

	Sprintf(buf,	"Known Wards");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	if(u.wardsknown & WARD_HEPTAGRAM){
		Sprintf(buf,	"Heptagram");
		any.a_int = HEPTAGRAM;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_GORGONEION){
		Sprintf(buf,	"Gorgoneion");
		any.a_int = GORGONEION;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_ACHERON){
		Sprintf(buf,	"Circle of Acheron");
		any.a_int = CIRCLE_OF_ACHERON;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_PENTAGRAM){
		Sprintf(buf,	"Pentagram");
		any.a_int = PENTAGRAM;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_HEXAGRAM){
		Sprintf(buf,	"Hexagram");
		any.a_int = HEXAGRAM;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_HAMSA){
		Sprintf(buf,	"Hamsa");
		any.a_int = HAMSA;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_ELDER_SIGN){
		Sprintf(buf,	"Elder Sign");
		any.a_int = ELDER_SIGN;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_EYE){
		Sprintf(buf,	"Elder Elemental Eye");
		any.a_int = ELDER_ELEMENTAL_EYE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_QUEEN){
		Sprintf(buf,	"Sign of the Scion Queen Mother");
		any.a_int = SIGN_OF_THE_SCION_QUEEN;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_CAT_LORD){
		Sprintf(buf,	"Cartouche of the Cat Lord");
		any.a_int = CARTOUCHE_OF_THE_CAT_LORD;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_GARUDA){
		Sprintf(buf,	"The Wings of Garuda");
		any.a_int = WINGS_OF_GARUDA;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_CTHUGHA){
		Sprintf(buf,	"The Sigil of Cthugha");
		any.a_int = SIGIL_OF_CTHUGHA;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_ITHAQUA){
		Sprintf(buf,	"The Brand of Ithaqua");
		any.a_int = BRAND_OF_ITHAQUA;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_KARAKAL){
		Sprintf(buf,	"The Tracery of Karakal");
		any.a_int = TRACERY_OF_KARAKAL;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.wardsknown & WARD_YELLOW){
		Sprintf(buf,	"The Yellow Sign");
		any.a_int = YELLOW_SIGN;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && uwep->ovar1&SEAL_ANDREALPHUS){
		Sprintf(buf,	"Hypergeometric transit solution");
		any.a_int = ANDREALPHUS_TRANSIT;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(uwep && uwep->oartifact == ART_PEN_OF_THE_VOID && uwep->ovar1&SEAL_ANDREALPHUS){
		Sprintf(buf,	"Hypergeometric stabilization solution");
		any.a_int = ANDREALPHUS_STABILIZE;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if (!describe){
		// Describe a ward
		Sprintf(buf, "Describe a ward instead");
		any.a_int = -1;					/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'?', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}
	else {
		Sprintf(buf, "Draw a ward instead");
		any.a_int = -1;					/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			'!', 0, ATR_NONE, buf,
			MENU_UNSELECTED);
	}

	end_menu(tmpwin,	(describe) ? "Choose ward to describe:" : "Choose ward to draw:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);

	if (n > 0 && selected[0].item.a_int == -1){
		return pick_ward(!describe);
	}

	if (n > 0 && describe){
		describe_ward(selected[0].item.a_int);
		return pick_ward(describe);
	}
	if (n > 0 && !describe){
		return selected[0].item.a_int;
	}

	return 0;
}

void
describe_ward(floorID)
int floorID;
{
	winid datawin;
	char name[80] = "";
	char strokes[80] = "";
	char warded[80] = "";
	char warded2[80] = "";
	char warded3[80] = "";
	char reinforce[80] = "";
	char secondary[80] = "";
	char secondary2[80] = "";

	switch (floorID){
	case HEPTAGRAM: 
		strcpy(name, " Heptagram");
		strcpy(strokes, " 21");
		strcpy(warded, " All except: A, o, dwarfs, G, @");
		strcpy(reinforce, " 7-fold");
		strcpy(secondary, " None.");
		break;
	case GORGONEION:
		strcpy(name, " Gorgoneion");
		strcpy(strokes, " 60");
		strcpy(warded, " All except: \' and A");
		strcpy(reinforce, " 3-fold");
		strcpy(secondary, " Has a 33% chance to scare for each reinforcement.");
		break;
	case CIRCLE_OF_ACHERON:
		strcpy(name, " Circle of Acheron");
		strcpy(strokes, " 2");
		strcpy(warded, " All undead, Cerberus");
		strcpy(reinforce, " 4-fold");
		strcpy(secondary, " Protects against the `touch of death\' monster spell.");
		break;
	case PENTAGRAM:
		strcpy(name, " Pentagram");
		strcpy(strokes, " 10");
		strcpy(warded, " i, E, K, &, hellhounds, gargoyles, sandestini");
		strcpy(reinforce, " 7-fold");
		strcpy(secondary, " None.");
		break;
	case HEXAGRAM:
		strcpy(name, " Hexagram");
		strcpy(strokes, " 12");
		strcpy(warded, " i, A, K, Q, \', &, hellhounds, eye of doom, son of Typhon");
		strcpy(reinforce, " 7-fold");
		strcpy(secondary, " None.");
		break;
	case HAMSA:
		strcpy(name, " Hamsa");
		strcpy(strokes, " 10, but drawn in pairs after 2-fold reinforcement");
		strcpy(warded, " floating eyes, beholders, autons");
		strcpy(reinforce, " 6-fold");
		strcpy(secondary, " Protects from all gaze attacks.");
		strcpy(secondary2," Grants invisbility at maximum reinforcement.");
		break;
	case ELDER_SIGN:
		strcpy(name, " Elder Sign");
		strcpy(strokes, " 6, 12, 8, 8, 8, 8");
		strcpy(warded, " 1-fold: b, j, m, p, w, l, P, U, ;, mind flayer");
		strcpy(warded2,"         deep one, deeper one, byakhee, nightgaunt");
		strcpy(warded3," 6-fold: deepest one, master mind flayer");
		strcpy(reinforce, " 6-fold");
		strcpy(secondary, " Wards against more monsters at maximum reinforcement.");
		break;
	case ELDER_ELEMENTAL_EYE:
		strcpy(name, " Elder Elemental Eye");
		strcpy(strokes, " 5");
		strcpy(warded, " 1-fold: spheres, v, E, F, X");
		strcpy(warded2," 4-fold: y, D, N, undead, metroids");
		strcpy(warded3," 7-fold: A, K, i, &, autons");
		strcpy(reinforce, " 7-fold");
		strcpy(secondary, " Wards against more monsters as it is reinforced.");
		break;
	case SIGN_OF_THE_SCION_QUEEN:
		strcpy(name, " Sign of the Scion Queen Mother");
		strcpy(strokes, " 8");
		strcpy(warded, " a, s, x, R");
		strcpy(reinforce, " 7-fold");
		strcpy(secondary, " None.");
		break;
	case CARTOUCHE_OF_THE_CAT_LORD:
		strcpy(name, " Cartouche of the Cat Lord");
		strcpy(strokes, " 7, 5, 6, 7, 5, 4, 7");
		strcpy(warded, " birds, bats, r, s, S, ;, :");
		strcpy(reinforce, " 7-fold");
		strcpy(secondary, " Pacifies f. 4-fold grants drain resistance. 7-fold grants sickness resistance.");
		strcpy(secondary2," Stops working when all domestic cats are genocided or extinct.");
		break;
	case WINGS_OF_GARUDA:
		strcpy(name, " The Wings of Garuda");
		strcpy(strokes, " 10");
		strcpy(warded, " c, r, N, S, :, kraken");
		strcpy(reinforce, " 7-fold");
		strcpy(secondary, " Reinforcement in 7 chance to resist poison.");
		break;
	case SIGIL_OF_CTHUGHA:
		strcpy(name, " The Sigil of Cthugha");
		strcpy(strokes, " 1");
		strcpy(warded, " None.");
		strcpy(reinforce, " None.");
		strcpy(secondary, " The player is immune to fire while standing upon it.");
		break;
	case BRAND_OF_ITHAQUA:
		strcpy(name, " The Brand of Ithaqua");
		strcpy(strokes, " 1");
		strcpy(warded, " None.");
		strcpy(reinforce, " None.");
		strcpy(secondary, " The player is immune to cold while standing upon it.");
		break;
	case TRACERY_OF_KARAKAL:
		strcpy(name, " The Tracery of Karakal");
		strcpy(strokes, " 1");
		strcpy(warded, " None.");
		strcpy(reinforce, " None.");
		strcpy(secondary, " The player is immune to electricity while standing upon it.");
		break;
	case YELLOW_SIGN:
		strcpy(name, " The Yellow Sign");
		strcpy(strokes, " 4");
		strcpy(warded, " 10% chance to frighten: humans, gnomes, dwarfs, elves, orcs");
		strcpy(reinforce, " None.");
		strcpy(secondary, " Any human, gnome, dwarf, elf, or orc that sees this becomes crazed.");
		break;
	case ANDREALPHUS_TRANSIT:
		strcpy(name, " Hypergeometric transit solution");
		strcpy(strokes, " ?");
		strcpy(warded, " None.");
		strcpy(reinforce, " None.");
		strcpy(secondary, " The player can teleport at will when standing upon it.");
		break;
	case ANDREALPHUS_STABILIZE:
		strcpy(name, " Hypergeometric stabilization solution");
		strcpy(strokes, " ?");
		strcpy(warded, " None.");
		strcpy(reinforce, " None.");
		strcpy(secondary, " The player can control their teleports when standing upon it.");
		break;
	default:
		impossible("No such ward to draw: %d", floorID);
		return;
	}
	
	datawin = create_nhwindow(NHW_TEXT);
	putstr(datawin, 0, "");
	putstr(datawin, 0, name);
	putstr(datawin, 0, "");
	putstr(datawin, 0, " Strokes to draw:");
	putstr(datawin, 0, strokes);
	putstr(datawin, 0, "");
	putstr(datawin, 0, " Warded creatures:");
	putstr(datawin, 0, warded);
	if(warded2[0] != 0)
		putstr(datawin, 0, warded2);
	if (warded3[0] != 0)
		putstr(datawin, 0, warded3);
	putstr(datawin, 0, "");
	putstr(datawin, 0, " Maximum reinforcement:");
	putstr(datawin, 0, reinforce);
	putstr(datawin, 0, "");
	putstr(datawin, 0, " Secondary effects:");
	putstr(datawin, 0, secondary);
	if (secondary2[0] != 0)
		putstr(datawin, 0, secondary2);
	putstr(datawin, 0, "");
	display_nhwindow(datawin, FALSE);
	destroy_nhwindow(datawin);
	return;
}

int
decode_wardID(wardID)
int wardID;
{
	switch(wardID){
		case WARD_HEPTAGRAM: return HEPTAGRAM;
		break;
		case WARD_GORGONEION: return GORGONEION;
		break;
		case WARD_ACHERON: return CIRCLE_OF_ACHERON;
		break;
		case WARD_PENTAGRAM: return PENTAGRAM;
		break;
		case WARD_HEXAGRAM: return HEXAGRAM;
		break;
		case WARD_HAMSA: return HAMSA;
		break;
		case WARD_ELDER_SIGN: return ELDER_SIGN;
		break;
		case WARD_EYE: return ELDER_ELEMENTAL_EYE;
		break;
		case WARD_QUEEN: return SIGN_OF_THE_SCION_QUEEN;
		break;
		case WARD_CAT_LORD: return CARTOUCHE_OF_THE_CAT_LORD;
		break;
		case WARD_GARUDA: return WINGS_OF_GARUDA;
		break;
		case WARD_CTHUGHA: return SIGIL_OF_CTHUGHA;
		break;
		case WARD_ITHAQUA: return BRAND_OF_ITHAQUA;
		break;
		case WARD_KARAKAL: return TRACERY_OF_KARAKAL;
		break;
		case WARD_YELLOW: return YELLOW_SIGN;
		break;
		case WARD_TRANSIT: return ANDREALPHUS_TRANSIT;
		break;
		case WARD_STABILIZE: return ANDREALPHUS_STABILIZE;
		break;
		case WARD_TOUSTEFNA: return TOUSTEFNA;
		break;
		case WARD_DREPRUN: return DREPRUN;
		break;
		case WARD_OTTASTAFUR: return OTTASTAFUR;
		break;
		case WARD_KAUPALOKI: return KAUPALOKI;
		break;
		case WARD_VEIOISTAFUR: return VEIOISTAFUR;
		break;
		case WARD_THJOFASTAFUR: return THJOFASTAFUR;
		break;
		default:
		return 0;
		break;
	}
}

int
get_wardID(floorID)
int floorID;
{
	switch(floorID){
		case HEPTAGRAM: return WARD_HEPTAGRAM;
		break;
		case GORGONEION: return WARD_GORGONEION;
		break;
		case CIRCLE_OF_ACHERON: return WARD_ACHERON;
		break;
		case PENTAGRAM: return WARD_PENTAGRAM;
		break;
		case HEXAGRAM: return WARD_HEXAGRAM;
		break;
		case HAMSA: return WARD_HAMSA;
		break;
		case ELDER_SIGN: return WARD_ELDER_SIGN;
		break;
		case ELDER_ELEMENTAL_EYE: return WARD_EYE;
		break;
		case SIGN_OF_THE_SCION_QUEEN: return WARD_QUEEN;
		break;
		case CARTOUCHE_OF_THE_CAT_LORD: return WARD_CAT_LORD;
		break;
		case WINGS_OF_GARUDA: return WARD_GARUDA;
		break;
		case SIGIL_OF_CTHUGHA: return WARD_CTHUGHA;
		break;
		case BRAND_OF_ITHAQUA: return WARD_ITHAQUA;
		break;
		case TRACERY_OF_KARAKAL: return WARD_KARAKAL;
		break;
		case YELLOW_SIGN: return WARD_YELLOW;
		break;
		case ANDREALPHUS_TRANSIT: return WARD_TRANSIT;
		break;
		case ANDREALPHUS_STABILIZE: return WARD_STABILIZE;
		break;
		case TOUSTEFNA: return WARD_TOUSTEFNA;
		break;
		case DREPRUN: return WARD_DREPRUN;
		break;
		case OTTASTAFUR: return WARD_OTTASTAFUR;
		break;
		case KAUPALOKI: return WARD_KAUPALOKI;
		break;
		case VEIOISTAFUR: return WARD_VEIOISTAFUR;
		break;
		case THJOFASTAFUR: return WARD_THJOFASTAFUR;
		break;
		default:
		return 0;
		break;
	}
}

/* return 1 if action took 1 (or more) moves, 0 if error or aborted */
int
doseal()
{
	return dogenengrave(SEAL_MODE);
}

int
pick_seal()
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
	
	Sprintf(buf,	"Known Seals");
	add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_BOLD, buf, MENU_UNSELECTED);
	
	for(i = 0; i < (QUEST_SPIRITS-FIRST_SEAL); i++){
		seal_flag = 0x1L << i;
		if(u.sealsKnown&seal_flag){
			if((u.sealsActive&seal_flag) && u.sealTimeout[i] > moves){
				Sprintf(buf,	"%s (active; timeout:%ld)",
					sealNames[i], 
					u.sealTimeout[i] - moves
				);
			} else if(u.sealsActive&seal_flag) {
				Sprintf(buf,	"%s (active)", 
					sealNames[i] 
				);
			} else if(u.sealTimeout[i] > moves){
				Sprintf(buf,	"%s (timeout:%ld)",
					sealNames[i], 
					u.sealTimeout[i] - moves
				);
			} else {
				Sprintf(buf,	"%s%s", 
					sealNames[i], 
					sealTitles[i]
				);
			}
			any.a_int = (i+FIRST_SEAL);	/* must be non-zero */
			add_menu(tmpwin, NO_GLYPH, &any,
				incntlet, 0, ATR_NONE, buf,
				MENU_UNSELECTED);
			incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
		}
	}
	if(Role_if(PM_EXILE) && quest_status.got_quest){
		if((u.specialSealsActive&SEAL_DAHLVER_NAR) && u.sealTimeout[DAHLVER_NAR-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (active; timeout:%ld)",
				sealNames[DAHLVER_NAR-FIRST_SEAL], 
				u.sealTimeout[DAHLVER_NAR-FIRST_SEAL] - moves
			);
		} else if(u.specialSealsActive&SEAL_DAHLVER_NAR) {
			Sprintf(buf,	"%s (active)", 
				sealNames[DAHLVER_NAR-FIRST_SEAL] 
			);
		} else if(u.sealTimeout[DAHLVER_NAR-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (timeout:%ld)",
				sealNames[DAHLVER_NAR-FIRST_SEAL], 
				u.sealTimeout[DAHLVER_NAR-FIRST_SEAL] - moves
			);
		} else {
			Sprintf(buf,	"%s%s", 
				sealNames[DAHLVER_NAR-FIRST_SEAL], 
				sealTitles[DAHLVER_NAR-FIRST_SEAL]
			);
		}
		any.a_int = DAHLVER_NAR;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(Role_if(PM_EXILE) && quest_status.killed_nemesis){
		if((u.specialSealsActive&SEAL_ACERERAK) && u.sealTimeout[ACERERAK-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (active; timeout:%ld)",
				sealNames[ACERERAK-FIRST_SEAL], 
				u.sealTimeout[ACERERAK-FIRST_SEAL] - moves
			);
		} else if(u.specialSealsActive&SEAL_ACERERAK) {
			Sprintf(buf,	"%s (active)", 
				sealNames[ACERERAK-FIRST_SEAL] 
			);
		} else if(u.sealTimeout[ACERERAK-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (timeout:%ld)",
				sealNames[ACERERAK-FIRST_SEAL], 
				u.sealTimeout[ACERERAK-FIRST_SEAL] - moves
			);
		} else {
			Sprintf(buf,	"%s%s", 
				sealNames[ACERERAK-FIRST_SEAL], 
				sealTitles[ACERERAK-FIRST_SEAL]
			);
		}
		any.a_int = ACERERAK;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(Role_if(PM_EXILE) && u.specialSealsKnown&SEAL_COSMOS){
		if((u.specialSealsActive&SEAL_COSMOS) && u.sealTimeout[COSMOS-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (active; timeout:%ld)",
				sealNames[COSMOS-FIRST_SEAL], 
				u.sealTimeout[COSMOS-FIRST_SEAL] - moves
			);
		} else if(u.specialSealsActive&SEAL_COSMOS) {
			Sprintf(buf,	"%s (active)", 
				sealNames[COSMOS-FIRST_SEAL] 
			);
		} else if(u.sealTimeout[COSMOS-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (timeout:%ld)",
				sealNames[COSMOS-FIRST_SEAL], 
				u.sealTimeout[COSMOS-FIRST_SEAL] - moves
			);
		} else {
			Sprintf(buf,	"%s%s", 
				sealNames[COSMOS-FIRST_SEAL], 
				sealTitles[COSMOS-FIRST_SEAL]
			);
		}
		any.a_int = COSMOS;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(Role_if(PM_EXILE) && u.specialSealsKnown&SEAL_LIVING_CRYSTAL){
		if((u.specialSealsActive&SEAL_LIVING_CRYSTAL) && u.sealTimeout[LIVING_CRYSTAL-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (active; timeout:%ld)",
				sealNames[LIVING_CRYSTAL-FIRST_SEAL], 
				u.sealTimeout[LIVING_CRYSTAL-FIRST_SEAL] - moves
			);
		} else if(u.specialSealsActive&SEAL_LIVING_CRYSTAL) {
			Sprintf(buf,	"%s (active)", 
				sealNames[LIVING_CRYSTAL-FIRST_SEAL] 
			);
		} else if(u.sealTimeout[LIVING_CRYSTAL-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (timeout:%ld)",
				sealNames[LIVING_CRYSTAL-FIRST_SEAL], 
				u.sealTimeout[LIVING_CRYSTAL-FIRST_SEAL] - moves
			);
		} else {
			Sprintf(buf,	"%s%s", 
				sealNames[LIVING_CRYSTAL-FIRST_SEAL], 
				sealTitles[LIVING_CRYSTAL-FIRST_SEAL]
			);
		}
		any.a_int = LIVING_CRYSTAL;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(Role_if(PM_EXILE) && u.specialSealsKnown&SEAL_MISKA){
		if((u.specialSealsActive&SEAL_MISKA) && u.sealTimeout[MISKA-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (active; timeout:%ld)",
				sealNames[MISKA-FIRST_SEAL], 
				u.sealTimeout[MISKA-FIRST_SEAL] - moves
			);
		} else if(u.specialSealsActive&SEAL_MISKA) {
			Sprintf(buf,	"%s (active)", 
				sealNames[MISKA-FIRST_SEAL] 
			);
		} else if(u.sealTimeout[MISKA-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (timeout:%ld)",
				sealNames[MISKA-FIRST_SEAL], 
				u.sealTimeout[MISKA-FIRST_SEAL] - moves
			);
		} else {
			Sprintf(buf,	"%s%s", 
				sealNames[MISKA-FIRST_SEAL], 
				sealTitles[MISKA-FIRST_SEAL]
			);
		}
		any.a_int = MISKA;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(Role_if(PM_EXILE) && u.specialSealsKnown&SEAL_NUDZIRATH){
		if((u.specialSealsActive&SEAL_NUDZIRATH) && u.sealTimeout[NUDZIRATH-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (active; timeout:%ld)",
				sealNames[NUDZIRATH-FIRST_SEAL], 
				u.sealTimeout[NUDZIRATH-FIRST_SEAL] - moves
			);
		} else if(u.specialSealsActive&SEAL_NUDZIRATH) {
			Sprintf(buf,	"%s (active)", 
				sealNames[NUDZIRATH-FIRST_SEAL] 
			);
		} else if(u.sealTimeout[NUDZIRATH-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (timeout:%ld)",
				sealNames[NUDZIRATH-FIRST_SEAL], 
				u.sealTimeout[NUDZIRATH-FIRST_SEAL] - moves
			);
		} else {
			Sprintf(buf,	"%s%s", 
				sealNames[NUDZIRATH-FIRST_SEAL], 
				sealTitles[NUDZIRATH-FIRST_SEAL]
			);
		}
		any.a_int = NUDZIRATH;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(Role_if(PM_EXILE) && u.specialSealsKnown&SEAL_ALIGNMENT_THING){
		if((u.specialSealsActive&SEAL_ALIGNMENT_THING) && u.sealTimeout[ALIGNMENT_THING-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (active; timeout:%ld)",
				sealNames[ALIGNMENT_THING-FIRST_SEAL], 
				u.sealTimeout[ALIGNMENT_THING-FIRST_SEAL] - moves
			);
		} else if(u.specialSealsActive&SEAL_ALIGNMENT_THING) {
			Sprintf(buf,	"%s (active)", 
				sealNames[ALIGNMENT_THING-FIRST_SEAL] 
			);
		} else if(u.sealTimeout[ALIGNMENT_THING-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (timeout:%ld)",
				sealNames[ALIGNMENT_THING-FIRST_SEAL], 
				u.sealTimeout[ALIGNMENT_THING-FIRST_SEAL] - moves
			);
		} else {
			Sprintf(buf,	"%s%s", 
				sealNames[ALIGNMENT_THING-FIRST_SEAL], 
				sealTitles[ALIGNMENT_THING-FIRST_SEAL]
			);
		}
		any.a_int = ALIGNMENT_THING;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(Role_if(PM_EXILE) && u.specialSealsKnown&SEAL_UNKNOWN_GOD){
		if((u.specialSealsActive&SEAL_UNKNOWN_GOD) && u.sealTimeout[UNKNOWN_GOD-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (active; timeout:%ld)",
				sealNames[UNKNOWN_GOD-FIRST_SEAL], 
				u.sealTimeout[UNKNOWN_GOD-FIRST_SEAL] - moves
			);
		} else if(u.specialSealsActive&SEAL_UNKNOWN_GOD) {
			Sprintf(buf,	"%s (active)", 
				sealNames[UNKNOWN_GOD-FIRST_SEAL] 
			);
		} else if(u.sealTimeout[UNKNOWN_GOD-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (timeout:%ld)",
				sealNames[UNKNOWN_GOD-FIRST_SEAL], 
				u.sealTimeout[UNKNOWN_GOD-FIRST_SEAL] - moves
			);
		} else {
			Sprintf(buf,	"%s%s", 
				sealNames[UNKNOWN_GOD-FIRST_SEAL], 
				sealTitles[UNKNOWN_GOD-FIRST_SEAL]
			);
		}
		any.a_int = UNKNOWN_GOD;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(u.specialSealsKnown&SEAL_BLACK_WEB){
		if((u.specialSealsActive&SEAL_BLACK_WEB) && u.sealTimeout[BLACK_WEB-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (active; timeout:%ld)",
				sealNames[BLACK_WEB-FIRST_SEAL], 
				u.sealTimeout[BLACK_WEB-FIRST_SEAL] - moves
			);
		} else if(u.specialSealsActive&SEAL_BLACK_WEB) {
			Sprintf(buf,	"%s (active)", 
				sealNames[BLACK_WEB-FIRST_SEAL] 
			);
		} else if(u.sealTimeout[BLACK_WEB-FIRST_SEAL] > moves){
			Sprintf(buf,	"%s (timeout:%ld)",
				sealNames[BLACK_WEB-FIRST_SEAL], 
				u.sealTimeout[BLACK_WEB-FIRST_SEAL] - moves
			);
		} else {
			Sprintf(buf,	"%s%s", 
				sealNames[BLACK_WEB-FIRST_SEAL], 
				sealTitles[BLACK_WEB-FIRST_SEAL]
			);
		}
		any.a_int = BLACK_WEB;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	if(Role_if(PM_EXILE) && u.ulevel == 30){
		if((u.specialSealsActive&SEAL_NUMINA)){
			Sprintf(buf,	"%s (active)", 
				sealNames[NUMINA-FIRST_SEAL]
			);
		} else {
			Sprintf(buf,	"%s%s", 
				sealNames[NUMINA-FIRST_SEAL], 
				sealTitles[NUMINA-FIRST_SEAL]
			);
		}
		any.a_int = NUMINA;	/* must be non-zero */
		add_menu(tmpwin, NO_GLYPH, &any,
			incntlet, 0, ATR_NONE, buf,
			MENU_UNSELECTED);
		incntlet = (incntlet != 'z') ? (incntlet+1) : 'A';
	}
	end_menu(tmpwin,	"Choose seal:");

	how = PICK_ONE;
	n = select_menu(tmpwin, how, &selected);
	destroy_nhwindow(tmpwin);
	return ( n > 0 ) ? selected[0].item.a_int : 0;
}

// int
// decode_sealID(sealID)
// int sealID;
// {
	// switch(sealID){
		// case SEAL_AHAZU: return ;
		// break;
		// case SEAL_AMON: return ;
		// break;
		// case SEAL_ANDREALPHUS: return ;
		// break;
		// default:
		// return 0;
		// break;
	// }
// }

// int
// get_sealID(floorID)
// int floorID;
// {
	// switch(floorID){
		// default:
		// return 0;
		// break;
	// }
// }

void
save_engravings(fd, mode)
int fd, mode;
{
	register struct engr *ep = head_engr;
	register struct engr *ep2;
	unsigned no_more_engr = 0;

	while (ep) {
	    ep2 = ep->nxt_engr;
	    if (((ep->engr_lth && ep->engr_txt[0]) || ep->ward_id) && perform_bwrite(mode)) {
			bwrite(fd, (genericptr_t)&(ep->engr_lth), sizeof(ep->engr_lth));
			bwrite(fd, (genericptr_t)ep, sizeof(struct engr) + ep->engr_lth);
			bwrite(fd, (genericptr_t)&(ep->engr_time), sizeof(long));
	    }
	    if (release_data(mode))
			dealloc_engr(ep);
	    ep = ep2;
	}
	if (perform_bwrite(mode))
	    bwrite(fd, (genericptr_t)&no_more_engr, sizeof no_more_engr);
	if (release_data(mode))
	    head_engr = 0;
}

void
rest_engravings(fd)
int fd;
{
	register struct engr *ep;
	unsigned lth;

	head_engr = 0;
	while(1) {
		mread(fd, (genericptr_t) &lth, sizeof(unsigned));
		if(lth == 0) return;
		ep = newengr(lth);
		mread(fd, (genericptr_t) ep, sizeof(struct engr) + lth);
		ep->nxt_engr = head_engr;
		head_engr = ep;
		ep->engr_txt = (char *) (ep + 1);	/* Andreas Bormann */
		/* mark as finished for bones levels -- no problem for
		 * normal levels as the player must have finished engraving
		 * to be able to move again */
		// ep->engr_time = moves;
		mread(fd, (genericptr_t) &(ep->engr_time), sizeof(long));
	}
}

void
del_engr_ward(ep)
register struct engr *ep;
{
	if (ep == head_engr) {
		head_engr = ep->nxt_engr;
	} else {
		register struct engr *ept;

		for (ept = head_engr; ept; ept = ept->nxt_engr)
		    if (ept->nxt_engr == ep) {
			ept->nxt_engr = ep->nxt_engr;
			break;
		    }
		if (!ept) {
		    impossible("Error in del_engr_ward?");
		    return;
		}
	}
	dealloc_engr(ep);
}

void
del_engr(ep)
register struct engr *ep;
{
	if(!ep->ward_id) del_engr_ward(ep);
	else make_engr_at(ep->engr_x,ep->engr_y,"",moves,DUST);
}

void
del_ward(ep)
register struct engr *ep;
{
	if(!ep->engr_txt[0]) del_engr_ward(ep);
	else{
		ep->ward_type = 0;
		ep->ward_id = 0;
		ep->halu_ward = FALSE;
		ep->complete_wards = 0;
		ep->scuffed_wards = 0;
		ep->degraded_wards = 0;
		ep->partial_wards = 0;
	}
}

void
blank_all_wards()
{
	struct engr *ep = head_engr, *np;
	
	while(ep) {
		np = ep->nxt_engr;
		del_ward(ep);
		ep = np;
	}
}

void
randomize_all_wards()
{
	struct engr *ep = head_engr;
	
	while(ep) {
		if(ep->ward_id){
			ep->halu_ward = TRUE;
			ep->ward_id = random_haluIndex();
		}
		ep = ep->nxt_engr;
	}
}
/* randomly relocate an engraving */
void
rloc_engr(ep)
struct engr *ep;
{
	int tx, ty, tryct = 200;

	do  {
	    if (--tryct < 0) return;
	    tx = rn1(COLNO-3,2);
	    ty = rn2(ROWNO);
	} while (engr_at(tx, ty) ||
		!goodpos(tx, ty, (struct monst *)0, 0));

	ep->engr_x = tx;
	ep->engr_y = ty;
}


/* Epitaphs for random headstones */
static const char *epitaphs[] = {
	"Genuine Exploding Gravestone.  (c)Acme Gravestones Inc.",
	"Rest in peace",
	"R.I.P.",
	"Rest In Pieces",
	"Note -- there are NO valuable items in this grave",
	"1994-1995. The Longest-Lived Hacker Ever",
	"The Grave of the Unknown Hacker",
	"We weren't sure who this was, but we buried him here anyway",
	"Sparky -- he was a very good dog",
	"Beware of Electric Third Rail",
	"Made in Taiwan",
	"Og friend. Og good dude. Og died. Og now food",
	"Beetlejuice Beetlejuice Beetlejuice",
	"Look out below!",
	"Please don't dig me up. I'm perfectly happy down here. -- Resident",
	"Postman, please note forwarding address: Gehennom, Asmodeus's Fortress, fifth lemure on the left",
	"Mary had a little lamb/Its fleece was white as snow/When Mary was in trouble/The lamb was first to go",
	"Be careful, or this could happen to you!",
	"Soon you'll join this fellow in hell! -- the Wizard of Yendor",
	"Caution! This grave contains toxic waste",
	"Sum quod eris",
	"Here lies an Atheist, all dressed up and no place to go",
	"Here lies Ezekiel, age 102.  The good die young.",
	"Here lies my wife: Here let her lie! Now she's at rest and so am I.",
	"Here lies Johnny Yeast. Pardon me for not rising.",
	"He always lied while on the earth and now he's lying in it",
	"I made an ash of myself",
	"Soon ripe. Soon rotten. Soon gone. But not forgotten.",
	"Here lies the body of Jonathan Blake. Stepped on the gas instead of the brake.",
	"He never killed a man, that did not need killing.",
	"Go away!",
	
	"TANJ!", /* Larry Niven */
	
	"The sun of the desert consumes what it never can cheer.", /* HP Lovecraft */
	
	"Summer Rose - Thus Kindly I Scatter", /* RWBY */
	
	"Hold my beer and watch this!", /* Rango */
	
	"Of course, you realize, this means war.",
	
	"In death ground, fight.", /* Sun Tzu */
	
	"Did you see him passing by?", /* Dark Souls 3 */
		"The world began without knowledge, and without knowledge will it end.", /* Dark Souls */
		"Fear not the dark, my friend. And let the feast begin.",
	
	"For every hero commemorated, a thousand martyrs die unmourned and unremembered." /*Warhammer 40k*/
	
	"FIQ:  One step from divinity.",
		"FIQ:  Debugging dNethack, one iron ball to the face at a time.",
		"FIQ, killed by a clockwork bug.",
	
	"Khor:  \"Wait, did that ax just fall off the edge of the world?\"",
		"Khor:  Balancing dNethack, one sleeping potion to the face at a time.",
	
	"AmyBSoD: \"Phew, killed that elder priest.  He shredded my CoMR but at least he's de-\"",
	
	"Package contains: One posthumous worm feeder",
	"Was that the hill I wanted to die on?",
	
	"Used no net, knew no fear, made mis-step, wound up here.", /*Balder's Gate gravestones*/
		"Here lies the body of Rob.  If not, please notify the undertakers at once.",
		"When I can no longer stand alone, then it will be time to die.",
		"I feel my body rising towards the bright light...wait, now it's falling, What the HELL!",
		"Reader if cash thou art in want of any, dig four feet deep and find a penny",
		"A lesson learned. Too bad I'm dead.",
	
	"I never got a single answer to any of my biggest questions, and now I will never know.", /*Unicorn Jelly*/
	
	"To deafened ears we ask, unseen, Which is life and which the dream?", /*Dresden Codak*/
	
	"I beat you in the human race.", /*Fable*/
	
	"Nobody expects the Spanish Inquisition!", /*Monty Python*/
	
	"Omnes una manet nox", /*One night awaits us all*/
	
	"I wake. I work. I sleep. I die." /*Alpha Centauri*/
	
	"I suddenly have a LOT of regrets.", /*Water Phoenix King is very quotable...*/
	
	"You either die a hero, or you live long enough to see yourself become the villain.", /* Batman, the Dark Knight */
	
	"What can men do against such reckless hate?", /* the Lord of the Rings (Two Towers movie) */
		"They have passed, like rain on the mountain, like a wind in the meadow.",
	
	"I'll just hit this thing a couple more times, it'll die.", /*jonadab*/
		"I don't need to unburden myself, they're only soldiers, I've been killing soldiers, it'll be fine.",
	
	"Maybe if I press this key a bit harder I'll hit for more damage next time.", /*Grasshopper*/
	
	"...nor the battle to the strong", /*Bible*/

	
       	"Alas fair Death, 'twas missed in life - some peace and quiet from my wife",
       	"Applaud, my friends, the comedy is finished.",
       	"At last... a nice long sleep.",
       	"Audi Partem Alteram",
       	"Basil, assaulted by bears",
       	"Burninated",
       	"Confusion will be my epitaph",
       	"Do not open until Christmas",
       	"Don't be daft, they couldn't hit an elephant at this dist-",
       	"Don't forget to stop and smell the roses",
       	"Don't let this happen to you!",
       	"Dulce et decorum est pro patria mori",
       	"Et in Arcadia ego",
       	"Fatty and skinny went to bed.  Fatty rolled over and skinny was dead.  Skinny Smith 1983-2000.",
       	"Finally I am becoming stupider no more",
       	"Follow me to hell",
       	"...for famous men have the whole earth as their memorial",
       	"Game over, man.  Game over.",
       	"Go away!  I'm trying to take a nap in here!  Bloody adventurers...",
       	"Gone fishin'",
       	"Good night, sweet prince: And flights of angels sing thee to thy rest!",
       	"Go Team Ant!",
       	"He farmed his way here",
       	"Here lies a programmer.  Killed by a fatal error.",
       	"Here lies Bob - decided to try an acid blob",
       	"Here lies Dudley, killed by another %&#@#& newt.",
       	"Here lies Gregg, choked on an egg",
       	"Here lies Lies. It's True",
       	"Here lies The Lady's maid, died of a Vorpal Blade",
       	"Here lies the left foot of Jack, killed by a land mine.  Let us know if you find any more of him",
       	"He waited too long",
       	"I'd rather be sailing",
       	"If a man's deeds do not outlive him, of what value is a mark in stone?",
       	"I'm gonna make it!",
       	"I took both pills!",
       	"I will survive!",
       	"Killed by a black dragon -- This grave is empty",
       	"Let me out of here!",
       	"Lookin' good, Medusa.",
       	"Mrs. Smith, choked on an apple.  She left behind grieving husband, daughter, and granddaughter.",
       	"Nobody believed her when she said her feet were killing her",
       	"No!  I don't want to see my damn conduct!",
       	"One corpse, sans head",
       	"On the whole, I'd rather be in Minetown",
       	"On vacation",
       	"Oops.",
       	"Out to Lunch",
       	"SOLD",
       	"Someone set us up the bomb!",
       	"Take my stuff, I don't need it anymore",
       	"Taking a year dead for tax reasons",
       	"The reports of my demise are completely accurate",
       	"(This space for sale)",
       	"This was actually just a pit, but since there was a corpse, we filled it",
       	"This way to the crypt",
       	"Tu quoque, Brute?",
       	"VACANCY",
       	"Welcome!",
       	"Wish you were here!",
       	"Yea, it got me too",
       	"You should see the other guy",
       	"...and they made me engrave my own headstone too!",
       	"...but the blood has stopped pumping and I am left to decay...",
       	"<Expletive Deleted>",
       	"A masochist is never satisfied.",
       	"Ach, 'twas a wee monster in the loch",
       	"Adapt.  Enjoy.  Survive.",
       	"Adventure, hah!  Excitement, hah!",
       	"After all, what are friends for...",
       	"After this, nothing will shock me",
       	"After three days, fish and guests stink",
       	"Age and treachery will always overcome youth and skill",
       	"Ageing is not so bad.  The real killer is when you stop.",
       	"Ain't I a stinker?",
       	"Algernon",
       	"All else failed...",
       	"All hail RNG",
       	"All I see are the bright lights of a billion places I'll never go",
       	"All right, we'll call it a draw!",
       	"All's well that end well",
       	"Alone at last!",
       	"Always attack a floating eye from behind!",
       	"Am I having fun yet?",
       	"And I can still crawl, I'm not dead yet!",
       	"And all I wanted was a free lunch",
       	"And all of the signs were right there on your face",
       	"And don't give me that innocent look either!",
       	"And everyone died.  Boo hoo hoo.",
       	"And here I go again...",
       	"And nobody cares until somebody famous dies...",
       	"And so it ends?",
       	"And so... it begins.",
       	"And sometimes the bear eats you.",
       	"And then 'e nailed me 'ead to the floor!",
       	"And they said it couldn't be done!",
       	"And what do I look like?  The living?",
       	"And yes, it was ALL his fault!",
       	"And you said it was pretty here...",
       	"Another lost soul",
       	"Any day above ground is a good day!",
       	"Any more of this and I'll die of a stroke before I'm 30.",
       	"Anybody seen my head?",
       	"Anyone for deathmatch?",
       	"Anything for a change.",
       	"Anything that kills you makes you ... well, dead",
       	"Anything worth doing is worth overdoing.",
       	"Are unicorns supposedly peaceful if you're a virgin?  Hah!",
       	"Are we all being disintegrated, or is it just me?",
       	"At least I'm good at something",
       	"Attempted suicide",
	"Auri sacra fames",
       	"Auribus teneo lupum",
       	"Be prepared",
       	"Beauty survives",
       	"Been Here. Now Gone. Had a Good Time.",
       	"Been through Hell, eh?  What did you bring me?",
       	"Beg your pardon, didn't recognize you, I've changed a lot.",
       	"Being dead builds character",
       	"Beloved daughter, a treasure, buried here.",
       	"Best friends come and go...  Mine just die.",
       	"Better be dead than a fat slave",
       	"Better luck next time",
       	"Beware of Discordians bearing answers",
       	"Beware the ...",
       	"Bloody Hell...",
       	"Bloody barbarians!",
       	"Blown upward out of sight: He sought the leak by candlelight",
       	"Brains... Brains... Fresh human brains...",
       	"Buried the cat.  Took an hour.  Damn thing kept fighting.",
       	"But I disarmed the trap!",
       	"CONNECT 1964 - NO CARRIER 1994",
       	"Call me if you need my phone number!",
       	"Can YOU fly?",
       	"Can you believe that thing is STILL moving?",
       	"Can you come up with some better ending for this?",
       	"Can you feel anything when I do this?",
       	"Can you give me mouth to mouth, you just took my breath away.",
       	"Can't I just have a LITTLE peril?",
       	"Can't eat, can't sleep, had to bury the husband here.",
       	"Can't you hit me?!",
       	"Chaos, panic and disorder.  My work here is done.",
       	"Check enclosed.",
       	"Check this out!  It's my brain!",
       	"Chivalry is only reasonably dead",
       	"Coffin for sale.  Lifetime guarantee.",
       	"Come Monday, I'll be all right.",
       	"Come and see the violence inherent in the system",
       	"Come back here!  I'll bite your bloody knees off!",
       	"Commodore Business Machines, Inc.   Died for our sins.",
       	"Complain to one who can help you",
       	"Confess my sins to god?  Which one?",
       	"Confusion will be my epitaph",
       	"Cooties?  Ain't no cooties on me!",
       	"Could somebody get this noose off me?",
       	"Could you check again?  My name MUST be there.",
       	"Could you please take a breath mint?",
       	"Couldn't I be sedated for this?",
       	"Courage is looking at your setbacks with serenity",
       	"Cover me, I'm going in!",
       	"Crash course in brain surgery",
       	"Cross my fingers for me.",
       	"Curse god and die",
       	"Cut to fit",
       	"De'Ath",
       	"Dead Again?  Pardon me for not getting it right the first time!",
       	"Dead and loving every moment!",
       	"Dear wife of mine. Died of a broken heart, after I took it out of her.",
       	"Don't tread on me!",
       	"Dragon? What dragon?",
       	"Drawn and quartered",
       	"Either I'm dead or my watch has stopped.",
       	"Eliza -- Was I really alive, or did I just think I was?",
       	"Elvis",
       	"Enter not into the path of the wicked",
       	"Eris?  I don't need Eris",
       	"Eternal Damnation, Come and stay a long while!",
       	"Even The Dead pay taxes (and they aren't Grateful).",
       	"Even a tomb stone will say good things when you're down!",
       	"Ever notice that live is evil backwards?",
       	"Every day is starting to look like Monday",
       	"Every day, in every way, I am getting better and better.",
       	"Every survival kit should include a sense of humor",
       	"Evil I did dwell;  lewd did I live",
       	"Ex post fucto",
       	"Excellent day to have a rotten day.",
       	"Excuse me for not standing up.",
       	"Experience isn't everything. First, You've got to survive",
       	"First shalt thou pull out the Holy Pin",
       	"For a Breath, I Tarry...",
       	"For recreational use only.",
       	"For sale: One soul, slightly used. Asking for 3 wishes.",
       	"For some moments in life, there are no words.",
       	"Forget Disney World, I'm going to Hell!",
       	"Forget about the dog, Beware of my wife.",
       	"Funeral - Real fun.",
       	"Gawd, it's depressing in here, isn't it?",
       	"Get back here!  I'm not finished yet...",
       	"Go ahead, I dare you to!",
       	"Go ahead, it's either you or him.",
       	"Goldilocks -- This casket is just right",
       	"Gone But Not Forgotten",
       	"Gone Underground For Good",
       	"Gone away owin' more than he could pay.",
       	"Gone, but not forgiven",
       	"Got a life. Didn't know what to do with it.",
       	"Grave?  But I was cremated!",
       	"Greetings from Hell - Wish you were here.",
       	"HELP! It's dark in here... Oh, my eyes are closed - sorry",
       	"Ha! I NEVER pay income tax!",
       	"Have you come to raise the dead?",
       	"Having a good time can be deadly.",
       	"Having a great time. Where am I exactly??",
       	"He died of the flux.",
       	"He died today... May we rest in peace!",
       	"He got the upside, I got the downside.",
       	"He lost his face when he was beheaded.",
       	"He missed me first.",
       	"He's not dead, he just smells that way.",
       	"Help! I've fallen and I can't get up!",
       	"Help, I can't wake up!",
       	"Here lies Pinocchio",
       	"Here lies the body of John Round. Lost at sea and never found.",
       	"Here there be dragons",
       	"Hey, I didn't write this stuff!",
	"Hodie mihi, cras tibi",
       	"Hold my calls",
       	"Home Sweet Hell",
       	"Humpty Dumpty, a Bad Egg.  He was pushed off the wall.",
       	"I KNEW this would happen if I lived long enough.",
       	"I TOLD you I was sick!",
       	"I ain't broke but I am badly bent.",
       	"I ain't old. I'm chronologically advantaged.",
       	"I am NOT a vampire. I just like to bite..nibble, really!",
       	"I am here. Wish you were fine.",
       	"I am not dead yet, but watch for further reports.",
       	"I believe them bones are me.",
       	"I broke his brain.",
       	"I can feel it.  My mind.  It's going.  I can feel it.",
       	"I can't go to Hell. They're afraid I'm gonna take over!",
       	"I can't go to hell, they don't want me.",
       	"I didn't believe in reincarnation the last time, either.",
       	"I didn't mean it when I said 'Bite me'",
       	"I died laughing",
       	"I disbelieved in reincarnation in my last life, too.",
       	"I hacked myself to death",
       	"I have all the time in the world",
		"I just wanted you to know that it was nice to talk to someone while it lasted.", /*SCP SCP-1470*/
       	"I knew I'd find a use for this gravestone!",
       	"I know my mind. And it's around here someplace.",
       	"I lied!  I'll never be alright!",
       	"I like it better in the dark.",
       	"I like to be here when I can.",
       	"I may rise but I refuse to shine.",
       	"I never get any either.",
       	"I said hit HIM with the fireball, not me!",
       	"I told you I would never say goodbye.",
       	"I used to be amusing. Now I'm just disgusting.",
       	"I used up all my sick days, so now I'm calling in dead.",
       	"I was killed by <illegible scrawl>",
       	"I was somebody. Who, is no business of yours.",
       	"I will not go quietly.",
       	"I'd give you a piece of my mind... but I can't find it.",
       	"I'd rather be breathing",
       	"I'll be back!",
       	"I'll be mellow when I'm dead. For now, let's PARTY!",
       	"I'm doing this only for tax purposes.",
       	"I'm not afraid of Death!  What's he gonna do? Kill me?",
       	"I'm not getting enough money, so I'm not going to engrave anything useful here.",
       	"I'm not saying anything.",
       	"I'm weeth stupeed --->",
       	"If you thought you had problems...",
       	"Ignorance kills daily.",
       	"Ignore me... I'm just here for my looks!",
       	"Ilene Toofar -- Fell off a cliff",
       	"Is that all?",
       	"Is there life before Death?",
       	"Is this a joke, or a grave matter?",
       	"It happens sometimes. People just explode.",
       	"It must be Thursday. I never could get the hang of Thursdays.",
       	"It wasn't a fair fight",
       	"It wasn't so easy.",
       	"It's Loot, Pillage and THEN Burn...",
       	"Just doing my job here",
       	"Killed by diarrhea of mouth and constipation of brain.",
       	"Let her RIP",
       	"Let it be; I am dead.",
       	"Let's play Hide the Corpse",
       	"Life is NOT a dream",
       	"Madge Ination -- It wasn't all in my head",
       	"Meet me in Heaven",
       	"Move on, there's nothing to see here.",
       	"Mr. Flintstone -- Yabba-dabba-done",
       	"My heart is not in this",
       	"No one ever died from it",
       	"No, you want room 12A, next door.",
       	"Nope.  No trap on that chest.  I swear.",
       	"Not again!",
       	"Not every soil can bear all things",
       	"Now I have a life",
       	"Now I lay thee down to sleep... wanna join me?",
       	"OK, here is a question: Where ARE your tanlines?",
       	"Obesa Cantavit",
       	"Oh! An untimely death.",
       	"Oh, by the way, how was my funeral?",
       	"Oh, honey..I missed you! She said, and fired again.",
       	"Ok, so the light does go off. Now let me out of here.",
       	"One stone brain",
       	"Ooh! Somebody STOP me!",
       	"Oops!",
       	"Out for the night.  Leave a message.",
       	"Ow!  Do that again!",
       	"Pardon my dust.",
       	"Part of me still works.",
       	"Please, not in front of those orcs!",
       	"Prepare to meet me in Heaven",
       	"R2D2 -- Rest, Tin Piece",
       	"Relax.  Nothing ever happens on the first level.",
       	"Res omnia mea culpa est",
       	"Rest In Pieces",
       	"Rest, rest, perturbed spirit.",
       	"Rip Torn",
       	"She always said her feet were killing her but nobody believed her.",
       	"She died of a chest cold.",
       	"So let it be written, so let it be done!",
       	"So then I says, How do I know you're the real angel of death?",
       	"Some patients insist on dying.",
       	"Some people have it dead easy, don't they?",
       	"Some things are better left buried.",
       	"Sure, trust me, I'm a lawyer...",
       	"Thank God I wore my corset, because I think my sides have split.",
       	"That is all",
       	"The Gods DO have a sense of humor: I'm living proof!",
       	"The frog's dead. He Kermitted suicide.",
       	"This dungeon is a pushover",
       	"This elevator doesn't go to Heaven",
       	"This gravestone is shareware. To register, please send me 10 zorkmids",
       	"This gravestone provided by The Yendorian Grave Services Inc.",
       	"This is not an important part of my life.",
       	"This one's on me.",
       	"This side up",
       	"Tim Burr -- Smashed by a tree",
       	"Tone it down a bit, I'm trying to get some rest here.",
       	"Virtually Alive",
       	"We Will Meet Again.",
       	"Weep not, he is at rest",
       	"Welcome to Dante's.  What level please?",
       	"Well, at least they listened to my sermon...",
       	"Went to be an angel.",
       	"What are you doing over there?",
       	"What are you smiling at?",
       	"What can you say, Death's got appeal...!",
       	"What health care?",
       	"What pit?",
       	"When the gods want to punish you, they answer your prayers.",
       	"Where e'er you be let your wind go free. Keeping it in was the death of me!",
       	"Where's my refund?",
       	"Will let you know for sure in a day or two...",
       	"Wizards are wimps",
       	"Worms at work, do not disturb!",
       	"Would you mind moving a bit?  I'm short of breath down here.",
       	"Would you quit being evil over my shoulder?",
       	"Ya really had me going baby, but now I'm gone.",
       	"Yes Dear, just a few more minutes...",
       	"You said it wasn't poisonous!",
        "You set my heart aflame. You gave me heartburn."
};

/* Create a headstone at the given location.
 * The caller is responsible for newsym(x, y).
 */
void
make_grave(x, y, str)
int x, y;
const char *str;
{
	/* Can we put a grave here? */
	if ((levl[x][y].typ != ROOM && levl[x][y].typ != GRAVE) || t_at(x,y)) return;

	/* Make the grave */
	levl[x][y].typ = GRAVE;

	/* Engrave the headstone */
	if (!str) str = epitaphs[rn2(SIZE(epitaphs))];
	del_engr_ward_at(x, y);
	make_engr_at(x, y, str, 0L, HEADSTONE);
	return;
}

boolean
allied_faction(f1, f2)
int f1,f2;
{
	if(f1 > f2){
		int tmp = f1;
		f1 = f2;
		f2 = tmp;
	}
	if(f1 >= FIRST_HOUSE && f1 <= LAST_HOUSE){
		return (f2 >= FIRST_TOWER && f2 <= LAST_TOWER) || (f2 == LOLTH_SYMBOL || f2 == PEN_A_SYMBOL);
	} else if(f1 >= FIRST_FALLEN_HOUSE && f1 <= LAST_FALLEN_HOUSE){
		return (f2 >= FIRST_TOWER && f2 <= LAST_TOWER) || (f2 == KIARANSALEE_SYMBOL || f2 == PEN_A_SYMBOL);
	} else if(f1 >= FIRST_TOWER && f1 <= LAST_TOWER){
		return (f2 >= FIRST_GODDESS && f2 <= LAST_GODDESS) || f2 == XAXOX;
	} else if(f1 == XAXOX){
		return (f2 == EDDER_SYMBOL);
	}
	else return FALSE;
}

#endif /* OVLB */

/*engrave.c*/
