#include "CommonHeader.h"

// TODO - any instance of eggs moving between sublevels seems to not work, and I don't know why
// As far as I can tell this may be related to which dragon models are loaded, but I'm not 100% sure
// Fix and re-implement them

vector<vector<EggData>> eggData = {
	/* 10 - Sunrise Spring Home   - Coltrane Isabelle Ami Bruce Liam */
	{
		{
			1,						//number;
			true,					//toUpdate; - for updating position and area ONLY, name and atlas entry should be updated as required
			0,						//levelNumber;
			0,						//area;
			{ 46372, 36246, 22383 },//position;
			0x61,					//yaw;
			"Absentfriend",			//name;
			"Superfly into the treetops."	//atlasName; Note that this is the first in the name list, I should probably order them in Atlas order
		},
		{
			0,
			false,
			0,
			0,
			{ 0, 0, 0 },
			0,
			"Stewart Copeland",
			"Chase the cheetah."
		},
		{
			2,
			true,
			0,
			0,
			{ 21250, 61850, 26500 },
			(char)0xD8,
			"LXShadow",
			"Source of the stream."
		},
		{
			3,
			true,
			0,
			0,
			{ 67176, 51987, 26749 },
			0x7D,
			"Gazco",
			"Admire the spire."
		},
		{
			4,
			false,
			0,
			0,
			{ 0, 0, 0 }, // 86624, 26825, 26638 for rock position
			0,
			"Dave Hancock",
			"Headbash the rock."
		}
	},
	/* 11 - Sunny Villa       - Sanders Lucy Emily Daisy Vanessa Miles */
	{
		{ 0, false, 1, 0, { 53487, 51808, 22998 }, 0,          "Roy",       "Rescue the mayor." }, // Move Mayor Leo onto the roof
		{ 4, true, 1, 0, { 22859, 35782, 14940 }, 0,           "Siegfried", "Take a bath." /*"Egg by the building."*/ }, // TEST THIS, the idea is that it should hopefully be collected in zombie only rather than when swimming or headbashing (currently headbashing works)
		{ 2, true, 1, 2, { 155500, 32052, 23255 }, 0x01,       "Neujahr",   "Beginner skater." },
		{ 3, true, 1, 0, { 49799, 76277, 22217 }, 0x3D,        "SSBMstuff", "Ancient out of bounds." }, // formerly a skateboarding area egg given by Hunter
		{ 1, true, 1, 0, { 64726, 74444, 21823 }, 0x00,        "ParsoFish", "Around the corner." /* "Hop to Rapunzel." */ },/* Previously { 69899, 69235, 23511 }, (char)0xBF */
		{ 5, true, 1, 2, { 196746, 37337, 24401 }, (char)0xA5, "Tony",      "Master skater." /* "Glide to the spring." */ } // formerly 155500, 32052, 23255 but not activated

	},
	/* 12 - Cloud Spires      - Henry LuLu Jake Bryan Stephanie Clare */
	{
		{ 0, false, 2, 0, { 36518, 57587, 29205 }, 0,          "Craig Stitt",     "Find the sky island." /* "Turn on the cloud generator." */ }, // Move Fluffy instead
		{ 4, true, 2, 0, { 47944, 65078, 15085 }, (char)0xE9,  "Vanessa Russell", "Moneybags has a clue." },
		{ 1, true, 2, 0, { 35289, 119759, 21399 }, (char)0xB8, "Lapogne",         "Far, far away." /* "Plant the sun seeds." */ },
		{ 2, false, 2, 2, { 0, 0, 0 }, 0,                      "DoctorAnsem",     "Bell tower spirits: reloaded." },
		{ 3, true, 2, 0, { 78898, 42803, 17805 }, (char)0x9C,  "ColdStone",       "Take the whirlwind." },
		{ 5, true, 2, 0, { 45799, 49078, 5589 }, (char)0xC7,   "Anne Chovy",      "Clear the city." } // Now linked to the Stephanie Rhynoc
	},
	/* 13 - Molten Crater     - Curlie Ryan Moira Kermitt Luna Rikki */
	{
		{ 0, false, 3, 0, { 59430, 74133, 21098 }, 0,          "Oliver Wade",    "Find the treetop tiki." /* "Get to the tiki lodge." */ }, // Move Rocky (229, 0) here
		{ 5, true, 3, 0, { 60166, 53851, 16125 }, (char)0xAC,  "RubbberRabbbit", "A halo of stars." /* "Egg by the lava river." */ },
		{ 1, true, 3, 2, { 24344, 64295, 14151 }, (char)0xA8,  "Moe",            "Destroy the armored chest." }, // Original position deprecated
		{ 4, true, 3, 0, { 26137, 44012, 14009 }, 0x09,        "Houdini",        "Turn the key and charge." /* "Sgt. Byrd Blows up a wall." */ },
		{ 2, false, 3, 2, { 0, 0, 0 }, 0,                      "Larry",          "Catch the thief." },
		{ 3, true, 3, 2, { 14093, 57038, 28196 }, (char)0xEA,  "Curly",          "Supercharge into the mountains." } // "Supercharge after the thief." - requires deactivating the second thief somehow and breaking that link, the state / moby data of the egg may need tweaking
	},
	/* 14 - Seashell Shore    - Dizzy Jason Mollie Jackie Duke Jared */
	{
		{ 0, false, 4, 0, { 48423, 56181, 18136 }, 0,          "Mr. Pop",       "Free the seals." /* "Free the seals." */ }, // Stanley - Unclear if this will work, we need him in the egg interaction state for this (try the octopus state too?) STATE STUFF TODO!!!
		{ 1, true, 4, 0, { 119500, 95545, 31091 }, (char)0x90, "Moxy",          "Stand on the seashell." /* "Under the docks." */ }, // "The Sandy Hills." 122557, 48149, 24348
		{ 4, true, 4, 2, { 82632, 48441, 19430 }, 0x27,        "Alex Schaefer", "The shovel sentinel." /* "Clear out the pipe." */ }, // Requires a state change too (0 -> 6) as this comes from area 3
		{ 3, false, 4, 2, { 0, 0, 0 }, 0,                      "ToastedKat",    "Defeat Bluto." },
		{ 2, true, 4, 0, { 73764, 40280, 14183 }, (char)0xBB,  "Ted Price",     "Sparx's magic trick." /* "Destroy the sand castle." */ },
		{ 5, true, 4, 1, { 145175, 63459, 13894 }, 0,          "Mark Cerny",    "Sheila finds an egg." /* "Hop to the secret cave." */ }
	},
	/* 15 - Mushroom Speedway - Sabina John Tater */
	{
		{ 0, false, 5, 0, { 0, 0, 0 }, 0,                 "ChrisLBC", "Time attack." },
		{ 1, true, 5, 0, { 106102, 117583, 27842 }, 0x25, "ThaRixer", "I'm stumped." /*"Race the butterflies."*/ },
		{ 2, true, 5, 0, { 173844, 112791, 28512 }, 0x2F, "Sonic",    "Green Hill Zone." /*"Hunter's dogfight."*/ }
	},
	/* 16 - Sheila's Alp      - Nan Jenny Ruby */
	{
		{ 0, false, 6, 0, { 0, 0, 0 }, 0,                     "Stephanie Duke",   "Help Bobby get home." },
		{ 1, true, 6, 0, { 70284, 83325, 21976 }, (char)0x80, "Roberto Rodriguez", "Find the secret path." },
		{ 2, true, 6, 0, { 60499, 50331, 38871 }, 0x52,       "Husbjo@urn",  "Climb the Alps." }
	},
	/* 17 - Buzz's Dungeon    - Grayson */
	{
		{ 0, true, 7, 0, { 65183, 34467, 15862 }, 0x1B, "Becci-chan", "Defeat Buzz." } /*TODO - worth testing whether this causes any problems*/
	},
	/* 18 - Crawdad Farm      - Nora */
	{
		{ 0, true, 8, 0, { 187425, 57765, 10225 }, 0x40, "Ryan Denniston", "Learn to Sparx." }
	},
	/* 20 - Midday Garden Home */
	{
		{ 0, true, 9, 0, { 27696, 66577, 11778 }, 0,          "Nerf",      "Secret triangle." /*"Underwater egg."*/ },
		{ 1, true, 9, 0, { 85289, 47238, 5161 }, (char)0x90,  "Zeuspot",   "Beneath frosty feet." /*"Secret ice cave."*/ },
		{ 2, true, 9, 0, { 58223, 15042, 13271 }, 0x60,       "Notvanni",  "Wallglide to the whirligig." },
		{ 3, false, 9, 0, { 0, 0, 0 }, 0,                     "Nuke",      "Fireball the flowerpots." },
		{ 4, true, 9, 0, { 46902, 54635, 15817 }, (char)0xC1, "Grantchil", "Scale the tower." /*"Climb to the ledge."*/ }
	},
	/* 21 - Icy Peak */
	{
		{ 0, false, 10, 0, { 60499, 39505, 16854 }, 0,         "Gavin Dodd",      "Rescue Doug." /*"Find Doug the polar bear."*/ },
		{ 4, true, 10, 0, { 79448, 60102, 25782 }, 0,          "Kiaa",            "Stand on the ceiling?" /*"On top of a ledge."*/ },
		{ 1, true, 10, 0, { 80202, 55112, 11269 }, 0x79,       "Newantox",        "Break the wall." /*"Protect Nancy the skater."*/},
		{ 2, true, 10, 0, { 100828, 40349, 17621 }, 0x2D,      "Dayoman",         "Just encased." /*"Speedy thieves I."*/ },
		{ 3, true, 10, 0, { 61833, 23351, 15174 }, (char)0xC2, "Rui2pop",         "Don't break the wall." /*"Speedy thieves II."*/ },
		{ 5, true, 10, 0, { 64445, 86195, 20517 }, (char)0x86, "Matthew Whiting", "Reach the icy peak." /*"Glide to the sky island."*/ }
	},
	/* 22 - Enchanted Towers */
	{
		{ 0, false, 11, 0, { 0, 0, 0 }, 0,                      "Burgerlands",       "Destroy the statue." },
		{ 5, true, 11, 0, { 41126, 73787, 24609 }, (char)0xC0,  "LuluPatate",        "Stand on the doorframe." /*"Glide to the small island."*/ },
		{ 3, true, 11, 0, { 124558, 69003, 27471 }, (char)0xB8, "Caroline Trujillo", "Glide to the enchanted tower." /*"Trick skater I."*/ },
		{ 4, true, 11, 2, { 61137, 49734, 16727 }, 0x21,        "WedSR",             "Play fetch." /*"Trick skater II."*/},
		{ 1, false, 11, 2, { 0, 0, 0 }, 0,                      "Fady180",           "Rescue the lost wolf." },
		{ 2, false, 11, 0, { 0, 0, 0 }, 0,                      "Tsmart",            "Collect the bones." }
	},
	/* 23 - Spooky Swamp */
	{
		{ 0, false, 12, 0, { 68919, 41619, 14778 }, 0,          "Thelonious",  "Find Shiny the firefly." },
		{ 1, true, 12, 0, { 50626, 20512, 7977 }, 0,            "Scooby",      "The spooky old tree." /*"Jump to the island."*/ },
		{ 3, true, 12, 0, { 37620, 88140, 7896 }, (char)0xE0,   "Copperfield", "Turn the key and charge." /*"Escort the twins I."*/ }, // formerly deprecated version of "The Tallest Mushroom."
		{ 4, true, 12, 1, { 135548, 47583, 1497 }, 0x3F,        "Moonscar",    "Discover the piranhas' treasure." /*"Escort the twins II."*/ },
		{ 5, false, 12, 1, { 0, 0, 0 }, 0,                      "Jeremy",      "Defeat sleepy head." },
		{ 2, true, 12, 2, { 116103, 95029, 10490 }, (char)0x9B, "Shaggy",      "Use the tallest mushroom." /*"Across the treetops."*/ }
	},
	/* 24 - Bamboo Terrace */
	{
		{ 0, false, 13, 0, { 118924, 38538, 25156 }, 0,          "Phorlo",         "Find the bridge extender." /*"Clear the pandas' path."*/ },
		{ 4, true, 13, 0, { 53146, 67936, 16600 }, (char)0xEA,   "Lukademus",      "Glide to the stone ledge." /*"Glide to the small island."*/ },
		{ 3, true, 13, 0, { 77655, 62486, 21734 }, 0,            "FranklyGD",      "Roof to roof." /*"Glide to the hidden cave."*/ },
		{ 2, true, 13, 0, { 100742, 28404, 23003 }, 0,           "Ryan Beveridge", "Atop the waterfall." /*"Smash to the mountain top."*/ },
		{ 1, true, 13, 1, { 112323, 106418, 19413 }, (char)0xC3, "Lewisb",         "Bamboatless." },
		{ 5, true, 13, 2, { 171623, 71791, 15919 }, 0x36,        "Brian Allgeier", "Glide from the mountain top." /*"Catch the thief."*/ } // Formerly Lei's House
	},
	/* 25 - Country Speedway */
	{
		{ 0, false, 14, 0, { 0, 0, 0 }, 0,                       "Jackie Evanochick", "Time attack." },
		{ 1, true, 14, 0, { 102824, 108037, 31509 }, (char)0x89, "Chad Dezern",       "Find Hunter's lost present." /*Race the pigs.*/},
		{ 2, true, 14, 0, { 73152, 139502, 23002 }, (char)0xD5,  "Laura",             "The giving tree." /*Hunter's rescue mission.*/}
	},
	/* 26 - Sgt. Byrd's Base */
	{
		{ 0, false, 15, 0, { 0, 0, 0 }, 0, "Fate Wolf",     "Clear the building." },
		{ 1, false, 15, 0, { 0, 0, 0 }, 0, "AdamtheDragon", "Track down Cpt. Byrd." },
		{ 2, false, 15, 0, { 0, 0, 0 }, 0, "JakeInTake",    "Hummingbird rescue." }
	},
	/* 27 - Spike's Arena */
	{
		{ 0, true, 16, 0, { 15040, 28231, 5079 }, (char)0xDB, "Nitrofski", "Defeat Spike." } /*TODO - worth testing whether this causes any problems*/
	},
	/* 28 - Spider Town */
	{
		{ 0, true, 17, 0, { 52300, 82507, 8151 }, (char)0xBF, "Shemp", "Grab the key." } // formerly Ripto
	},
	/* 30 - Evening Lake Home */
	{
		{ 1, true, 18, 0, { 28726, 48858, 35956 }, 0x0F,       "Wojowu", "Stand on the doorframe." /*"Glide to the tower."*/ },
		{ 4, true, 18, 0, { 22215, 72634, 11300 }, (char)0xF3, "YFourteen", "Trapped!" /*"I'm invincible!"*/ },
		{ 0, true, 18, 0, { 38232, 54297, 32308 }, (char)0xB0, "Yearofthe", "Under the bridge." /*"On the bridge."*/ },
		{ 2, false, 18, 0, { 0, 0, 0 }, 0,                     "Stooby", "Right where you left him." /*"Break the tower wall."*/ }, /*Reduced render distance to 0*/
		{ 3, true, 18, 0, { 62625, 57347, 34519 }, (char)0x86, "Toasty", "In hot water." /*"Belly of the whale."*/ } /*TODO - JT is not sure if this one will make the final cut*/
	},
	/* 31 - Frozen Altars */
	{
		{ 0, false, 19, 0, { 92279, 89058, 17933 }, 0,           "Quetzalcoatl",    "A cozy place." /*"Melt the snowmen."*/ }, /*EOL NPC, deprecated position*/
		{ 3, true, 19, 0, { 44512, 54180, 28311 }, 0,            "Ba'ah",           "Reach the roof." }, /*Actually "Frozen6" in the dev9 / dev10 sheets*/
		{ 5, true, 19, 1, { 146572, 69044, 12247 }, 0x5E,        "Xipe Totec",      "Speak to Bentley." /*"Across the rooftops."*/ }, // previously "That egg looks humongous!" area 0 { 76461, 64894, 28326 }, (char)0xBE "That egg looks humongous!"
		{ 4, true, 19, 0, { 72567, 78481, 36413 }, 0,            "Huitzilopochtli", "Stand on the Jungle Temple." /*"Glide from the temple roof."*/ },
		{ 1, true, 19, 0, { 73636, 105488, 20437 }, (char)0x80,  "Tezcatlipoca",    "Melt the snowman." /*"Box the yeti."*/ }, // formerly { 46322, 108118, 28119 }, (char)0xDE
		{ 2, true, 19, 0, { 113901, 111719, 29047 }, (char)0x98, "Coatlicue",       "The secret summit." /*"Box the yeti again!"*/ }
	},
	/* 32 - Lost Fleet */
	{
		{ 0, false, 20, 0, { 39267, 58396, 33574 }, 0,          "Novaspell",  "Find Crazy Ed's treasure." /*"Find Crazy Ed's treasure."*/ }, /*TODO - temporarily disabled - probably involves disabling Ed entirely and putting it into a visible state?*/
		{ 3, true, 20, 0, { 36811, 53261, 28330 }, 0x7B,        "Walkers",    "Blast the armored chest." /*"Swim through acid."*/ }, // Formerly { 36811, 53261, 28330 }, 0x77
		{ 1, true, 20, 0, { 92538, 64431, 44768 }, (char)0xC4,  "Ashriel",    "Bird's eye view." /*"Sink the subs I."*/ }, // name had hyphen, but this did not display in game
		{ 2, true, 20, 0, { 89257, 34869, 27701 }, 0x72,        "HoneySonya", "Wait for the whirlwind." /*"Sink the subs II."*/ },
		{ 4, true, 20, 0, { 38799, 57952, 33476 }, 0x1C,        "OddKara",    "Follow the vulture." /*"Skate race the rhynocs."*/ },
		{ 5, true, 20, 0, { 105848, 69670, 39121 }, (char)0xB9, "Spudlyman",  "Search from stem to stern." /*"Skate race Hunter."*/ } /* OLD - 41818, 112506, 18347 */ /*TODO - not sure how this'll work, due to the two stages of the challenge*/
	},
	/* 33 - Fireworks Factory */
	{
		{ 0, false, 21, 0, { 0, 0, 0 }, 0,                      "Hummeldon",    "Destwoy the wocket!" },
		{ 5, true, 21, 0, { 31339, 47462, 10202 }, 0,           "Jacques",      "Ninja in training." /*"Hidden in an alcove."*/ },
		{ 4, true, 21, 1, { 12156, 57432, 27582 }, 0x35,        "Tyzerra",      "In the dragons' den." /*"Bad dragon!"*/ }, /*TODO - remove Handel, also consider an easier location { 24338, 68508, 27254 }*/
		{ 3, true, 21, 0, { 65270, 55712, 5079 }, (char)0xBC,   "Klymenco",     "Secret in the tunnel." }, // previously area 2, { 78201, 48153, 14045 }, (char)0xFF
		{ 1, true, 21, 0, { 22813, 110244, 17621 }, (char)0xFF, "Michael John", "Near the impossible gems." /*"You're doomed!"*/ },
		{ 2, true, 21, 0, { 64667, 102247, 10710 }, 0x70,       "Dereklander",  "Leap of faith." /*"You're still doomed!"*/ }
	},
	/* 34 - Charmed Ridge */
	{
		{ 0, false, 22, 0, { 0, 0, 0 }, 0,                       "Composer",      "Rescue the Fairy Princess." },
		{ 2, true, 22, 0, { 127388, 103389, 15176 }, (char)0xA0, "Hypnoshark",    "1.0 spot." /*"Egg in the cave."*/ }, /*The platform only exists on version 1.0. I'm not sure if you can collect an egg during the goop death animation. This will have to be tested.*/
		{ 1, true, 22, 0, { 151149, 49365, 26413 }, (char)0xDD,  "Pepper",        "Find the limestone halo." /*"Glide to the tower."*/ },
		{ 3, true, 22, 2, { 78280, 40641, 25064 }, 0x7C,         "Alex Hastings", "Plant the seeds." },
		{ 4, true, 22, 0, { 71573, 76549, 29325 }, (char)0xCB,   "Shelley",       "Explore the mountain range." /*"Jack and the beanstalk I."*/ },
		{ 5, true, 22, 0, { 118607, 45670, 21928 }, 0x40,        "Lloyd Murphy",  "Become the Stairmaster." /*"Jack and the beanstalk II."*/ }
	},
	/* 35 - Honey Speedway */
	{
		{ 0, true, 23, 0, { 70658, 105345, 30203 }, 0x40,       "NeonGreen", "Climb the spiral tower." },
		{ 1, true, 23, 0, { 116301, 82675, 38293 }, (char)0x80, "Grapevine", "Atop the palace." }, /*Atlas name really doesn't have a full stop at the end lol*/
		{ 2, true, 23, 0, { 112856, 58174, 28882 }, (char)0x89, "Beuchiism", "Go out on a limb." } // formerly { 102636, 54889, 28563 }, 0x5F
	},
	/* 36 - Bentley's Outpost */
	{
		{ 1, true, 24, 0, { 61244, 72415, 13035 }, (char)0xD9,  "Disalyssa",  "Glide to the island." },
		{ 2, true, 24, 0, { 107746, 38732, 10847 }, (char)0xFF, "Spora",      "Behind the ice wall." },
		{ 0, true, 24, 0, { 14082, 54177, 13801 }, 0x00,        "John Unfil", "Help Bartholomew home." } // EOL egg
	},
	/* 37 - Scorch's Pit */
	{
		{ 0, true, 25, 0, { 31097, 3007, 22768 }, 0x78, "Zethical", "Defeat Scorch." }
	},
	/* 38 - Starfish Reef */
	{
		{ 0, false, 26, 0, { 0, 0, 0 }, 0, "Nameless", "Defeat the reef chief." }
	},
	/* 40 - Midnight Mountain Home */
	{
		{ 0, true, 27, 0, { 82917, 74254, 20000 }, 0x52,         "John Fiorito",   "Steal the snake's dinner." /*"Shhh, it's a secret."*/ },
		{ 3, true, 27, 0, { 37920, 41232, 20808 }, 0x46,         "XandoToaster",   "Monkey puzzle." /*"Glide to the island."*/ }, // { 35454, 38190, 20806 }, 0x1A // formerly { 17435, 74302, 15654 }, 0x37
		{ 4, true, 27, 0, { 37368, 36290, 9920 }, (char)0x91,    "Dactyly",        "Fall." /*"Stomp the floor."*/ },
		{ 2, true, 27, 0, { 76418, 123104, 30265 }, (char)0x9B,  "Touval",         "Skip into the Sorceress's lair." /*thief egg?*/ },
		{ 1, true, 27, 0, { 116552, 100412, 28227 }, (char)0xCB, "Puppetmaster",   "Spikes arena." /*"At the top of the waterfall."*/ },
		{ 5, true, 27, 0, { 69267, 12595, 23030 }, 0x3B,         "CrystalFissure", "Shhh, it's a secret." }
	},
	/* 41 - Crystal Islands */
	{
		{ 0, false, 28, 0, { 0, 0, 0 }, 0,                "Gnasty Gnorc", "Reach the crystal tower." },
		{ 2, true, 28, 0, { 58099, 89926, 17878 }, 0x5B,  "Angel",        "Turn the key and charge." /*"Whack a mole."*/ },
		{ 5, false, 28, 0, { 95747, 96798, 20040 }, 0x31, "Dan Johnson",  "Catch the flying thief." /*"Catch the flying thief."*/ },
		{ 3, true, 28, 0, { 95982, 97290, 19972 }, 0x35,  "Chuck Suong",  "Tucked away." /*"Fly to the hidden egg."*/ },
		{ 4, true, 28, 1, { 46908, 96127, 36929 }, 0,     "Bluntbows",    "The impossible egg." /*"Glide to the island."*/ },
		{ 1, true, 28, 1, { 84571, 53876, 24487 }, 0x03,  "Tom Kenny",    "Slide along the rail." /*"Ride the slide."*/ } // formerly { 85264, 55177, 24437 }, (char)0xCF
	},
	/* 42 - Desert Ruins */
	{
		{ 0, true, 29, 1, { 61301, 15095, 37851 }, 0x00,       "Whittaker", "Take the high road." /*"Raid the tomb."*/ }, // Formerly { 167601, 24285, 26646 }, 0x2C
		{ 1, true, 29, 0, { 108070, 45591, 28037 }, 0x75,      "Smith",     "Do a handstand?" /*"Shark shootin'."*/ },
		{ 2, true, 29, 2, { 66562, 65410, 37286 }, (char)0xC0, "Capaldi",   "Atop the Sunken Temple." /*"Krash Kangaroo I."*/ }, // Formerly { 66575, 63620, 37286 }, 0x3F // Formerly { 76803, 38354, 25145 }, 0; a key is now there
		{ 3, true, 29, 0, { 92216, 25365, 26583 }, 0x2E,       "Tennant",   "Scorch the scorps." /*"Krash Kangaroo II."*/ }, // Formerly { 33736, 52979, 20700 }, 0x49 // Formerly { 86592, 72293, 30915 }, (char)0xA0
		{ 4, true, 29, 0, { 46844, 79592, 22402 }, (char)0x80, "Eccleston", "It's alive!" /*"Sink or singe."*/ }, // Formerly { 59898, 71544, 24984 }, (char)0x13
		{ 5, true, 29, 0, { 73917, 59730, 24536 }, (char)0xED, "Blaine",    "Turn the key and charge." /*"Give me a hand"*/ } // Formerly { 45002, 93751, 22812 }, (char)0x9F // Formerly { 45219, 93433, 26148 }, (char)0xD0
	},
	/* 43 - Haunted Tomb */
	{
		{ 0, false, 30, 0, { 46832, 59490, 24023 }, 0,         "Jared Hardy",    "Let the dogs out." /*"Release the temple dweller."*/ },
		{ 1, true, 30, 1, { 145996, 50964, 22997 }, 0x3F,      "Drashed",        "Tank blast." /*"Snake slide."*/ },
		{ 2, true, 30, 0, { 41006, 47823, 17881 }, (char)0x91, "Brian Hastings", "Raid the tomb." }, // Not still tank blast
		{ 3, true, 30, 0, { 42965, 24584, 23510 }, 0x43,       "TOTOzigemm",     "Across the beams." }, // Not still tank blast
		{ 4, false, 30, 2, { 0, 0, 0 }, 0,                     "Hwd405",         "Clear the caves." },
		{ 5, true, 30, 0, { 42883, 81595, 20893 }, 0x10,       "Zachary",        "Secret in the doorframe." /*"Climb the wall."*/ }
	},
	/* 44 - Dino Mines */
	{
		{ 0, false, 31, 0, { 0, 0, 0 }, 0,                     "iBolt",          "Jail break!" },
		{ 1, true, 31, 3, { 76899, 85770, 44652 }, (char)0x82, "DanielDavidson", "Barrel blast off." /*"Shafted!" - name had hyphen, but this did not display in game*/ },
		{ 2, true, 31, 0, { 113700, 45528, 15091 }, 0x3F,      "James Justin",   "Beneath knocking footsteps." /*"Swim through the wall."*/ },
		{ 3, true, 31, 3, { 53932, 58376, 37132 }, (char)0xFC, "Altro",          "Shoot through the window." },
		{ 4, true, 31, 0, { 71818, 97637, 25557 }, 0,          "Crash41596",     "Fall down a chimney." /*"Leap of faith."*/ },
		{ 5, true, 31, 1, { 157371, 103538, 10710 }, 0,        "Shaw8b",         "The forgotten challenge." /*"Take it to the bank."*/ }
	},
	/* 45 - Harbor Speedway */
	{
		{ 0, true, 32, 0, { 99168, 163119, 11222 }, (char)0xDE, "Wafflewizard", "Time attack." },
		{ 1, true, 32, 0, { 152303, 97675, 16341 }, (char)0xAD, "John Lally",   "Find the stowaway." },
		{ 2, true, 32, 0, { 112014, 86392, 30615 }, (char)0xBD, "Bombercash",   "Superfly into the mountains." }
	},
	/* 46 - Agent 9's Lab */
	{
		{ 0, true, 33, 0, { 99829, 90565, 16342 }, (char)0xDF,  "Texensis",  "???" /*Blast and bomb the rhynocs.*/ }, // old - { 92034, 122713, 16341 }, 0x5B // old - { 107228, 82817, 16347 }, 0x64 // old - { 99196, 81944, 15884 }, 0xA4
		{ 1, false, 33, 0, { 0, 0, 0 }, 0,                      "Metalhead", "???" /*Snipe the boats.*/ },
		{ 2, true, 33, 0, { 98483, 133090, 17369 }, (char)0xE2, "Blowhard",  "???" /*This place has gone to the birds.*/ } // old - { 99829, 90565, 16346 }, (char)0xDF
	},
	/* 47 - Sorceress' Lair */
	{
		{ 0, false, 34, 0, { 0, 0, 0 }, 0, "Ted", "Defeat the Sorceress?" }
	},
	/* 48 - Bugbot Factory */
	{
		{ 0, false, 35, 0, { 0, 0, 0 }, 0, "Secret", "Defeat the mecha wiggler." } // name had hyphen (mecha-wiggler), but this did not display in game, so it was removed in case there were issues
	},
	/* 50 - Super Bonus Round */
	{
		{ 0, false, 36, 3, { 0, 0, 0 }, 0, "Alex and Brian", "The Sorceress's last stand." }
	}
};

Moby sampleEgg = {
	0x0000aef8,
	0,
	0,
	{ 0, 0, 0 },
	0,
	{ 0, 0, 0, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	0,
	132, // Class
	0,
	0x7F00,
	{ 0, 0, 0, 0 },
	0,
	{ 1, 0 },
	255,
	{ 0, 0, 73 },
	4,
	0, // State
	0,
	{ 16, 0 },
	32,
	0,
	16,
	0,
	255,
	0,
	1, // Level sector index, probably important
	7,
	0
};

vector<char> sampleEggTag = { 0, 0, 0, 0, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, 0, 0 };