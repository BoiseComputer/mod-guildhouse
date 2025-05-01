-- !!! NOTE: set this before running the queries in order to avoid conflicts !!!
SET @MOUNT_VENDOR = 5000351;

-- Insert new creature templates for mount vendors (if not already present elsewhere)
REPLACE INTO `creature_template` (`entry`, `name`, `subname`, `minlevel`, `maxlevel`, `faction`, `npcflag`) VALUES
(@MOUNT_VENDOR + 0, 'Mount Vendor: Hoofed', 'Rams, Talbuks, Elekks, Kodos', 80, 80, 35, 128),
(@MOUNT_VENDOR + 1, 'Mount Vendor: Steeds', 'Horses, Skeletal, Mammoths', 80, 80, 35, 128),
(@MOUNT_VENDOR + 2, 'Mount Vendor: Wild', 'Wolves, Bears, Tigers, Sabers', 80, 80, 35, 128),
(@MOUNT_VENDOR + 3, 'Mount Vendor: Raptors & Drakes', 'Raptors, Netherwing, Proto, etc', 80, 80, 35, 128),
(@MOUNT_VENDOR + 4, 'Mount Vendor: Exotics', 'Hawkstriders, Mechanostriders, etc', 80, 80, 35, 128),
(@MOUNT_VENDOR + 5, 'Mount Vendor: Special', 'Special, Qiraji, Carpets, Brooms', 80, 80, 35, 128);

-- Each mount is only offered ONCE, in a single group/vendor.

-- Insert matching models for each mount vendor
REPLACE INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(@MOUNT_VENDOR + 0, 0, 26893, 1, 1, 0),
(@MOUNT_VENDOR + 1, 0, 26893, 1, 1, 0),
(@MOUNT_VENDOR + 2, 0, 26893, 1, 1, 0),
(@MOUNT_VENDOR + 3, 0, 26893, 1, 1, 0),
(@MOUNT_VENDOR + 4, 0, 26893, 1, 1, 0),
(@MOUNT_VENDOR + 5, 0, 26893, 1, 1, 0);

-- Group 1: Rams, Talbuks, Elekks, Kodos (custom grouping)
REPLACE INTO `npc_vendor` (`entry`, `item`, `maxcount`, `incrtime`, `ExtendedCost`) VALUES
(5000351, 5864, 0, 0, 0), -- Gray Ram
(5000351, 5872, 0, 0, 0), -- Brown Ram
(5000351, 5873, 0, 0, 0), -- White Ram
(5000351, 13328, 0, 0, 0), -- Black Ram
(5000351, 13329, 0, 0, 0), -- Frost Ram
(5000351, 18244, 0, 0, 0), -- Black War Ram
(5000351, 18785, 0, 0, 0), -- Swift White Ram
(5000351, 18786, 0, 0, 0), -- Swift Brown Ram
(5000351, 18787, 0, 0, 0), -- Swift Gray Ram
(5000351, 29467, 0, 0, 0), -- Black War Ram
(5000351, 33976, 0, 0, 0), -- Brewfest Ram
(5000351, 33977, 0, 0, 0), -- Swift Brewfest Ram
(5000351, 45586, 0, 0, 0), -- Ironforge Ram
(5000351, 46748, 0, 0, 0), -- Swift Violet Ram
(5000351, 46762, 0, 0, 0), -- Swift Violet Ram
(5000351, 28915, 0, 0, 0), -- Reins of the Dark Riding Talbuk
(5000351, 29102, 0, 0, 0), -- Reins of the Cobalt War Talbuk
(5000351, 29103, 0, 0, 0), -- Reins of the White War Talbuk
(5000351, 29104, 0, 0, 0), -- Reins of the Silver War Talbuk
(5000351, 29105, 0, 0, 0), -- Reins of the Tan War Talbuk
(5000351, 29227, 0, 0, 0), -- Reins of the Cobalt War Talbuk
(5000351, 29228, 0, 0, 0), -- Reins of the Dark War Talbuk
(5000351, 29229, 0, 0, 0), -- Reins of the Silver War Talbuk
(5000351, 29230, 0, 0, 0), -- Reins of the Tan War Talbuk
(5000351, 29231, 0, 0, 0), -- Reins of the White War Talbuk
(5000351, 31829, 0, 0, 0), -- Reins of the Cobalt Riding Talbuk
(5000351, 31830, 0, 0, 0), -- Reins of the Cobalt Riding Talbuk
(5000351, 31831, 0, 0, 0), -- Reins of the Silver Riding Talbuk
(5000351, 31832, 0, 0, 0), -- Reins of the Silver Riding Talbuk
(5000351, 31833, 0, 0, 0), -- Reins of the Tan Riding Talbuk
(5000351, 31834, 0, 0, 0), -- Reins of the Tan Riding Talbuk
(5000351, 31835, 0, 0, 0), -- Reins of the White Riding Talbuk
(5000351, 31836, 0, 0, 0), -- Reins of the White Riding Talbuk
(5000351, 28481, 0, 0, 0), -- Brown Elekk
(5000351, 28482, 0, 0, 0), -- Great Elite Elekk
(5000351, 29743, 0, 0, 0), -- Purple Elekk
(5000351, 29744, 0, 0, 0), -- Gray Elekk
(5000351, 29745, 0, 0, 0), -- Great Blue Elekk
(5000351, 29746, 0, 0, 0), -- Great Green Elekk
(5000351, 29747, 0, 0, 0), -- Great Purple Elekk
(5000351, 35906, 0, 0, 0), -- Reins of the Black War Elekk
(5000351, 45590, 0, 0, 0), -- Exodar Elekk
(5000351, 46745, 0, 0, 0), -- Great Red Elekk
(5000351, 46756, 0, 0, 0), -- Great Red Elekk
(5000351, 15277, 0, 0, 0), -- Gray Kodo
(5000351, 15290, 0, 0, 0), -- Brown Kodo
(5000351, 15292, 0, 0, 0), -- Green Kodo
(5000351, 15293, 0, 0, 0), -- Teal Kodo
(5000351, 18247, 0, 0, 0), -- Black War Kodo
(5000351, 18793, 0, 0, 0), -- Great White Kodo
(5000351, 18794, 0, 0, 0), -- Great Brown Kodo
(5000351, 18795, 0, 0, 0), -- Great Gray Kodo
(5000351, 29466, 0, 0, 0), -- Black War Kodo
(5000351, 37827, 0, 0, 0), -- Brewfest Kodo
(5000351, 37828, 0, 0, 0), -- Great Brewfest Kodo
(5000351, 45592, 0, 0, 0), -- Thunder Bluff Kodo
(5000351, 46100, 0, 0, 0), -- White Kodo
(5000351, 46750, 0, 0, 0), -- Great Golden Kodo
(5000351, 46755, 0, 0, 0); -- Great Golden Kodo

-- Group 2: Horses, Skeletal Horses, Steeds, Mammoths, and related
REPLACE INTO `npc_vendor` (`entry`, `item`, `maxcount`, `incrtime`, `ExtendedCost`) VALUES
(5000352, 2411, 0, 0, 0), -- Black Stallion Bridle
(5000352, 2414, 0, 0, 0), -- Pinto Bridle
(5000352, 5655, 0, 0, 0), -- Chestnut Mare Bridle
(5000352, 5656, 0, 0, 0), -- Brown Horse Bridle
(5000352, 12353, 0, 0, 0), -- White Stallion Bridle
(5000352, 12354, 0, 0, 0), -- Palomino Bridle
(5000352, 13331, 0, 0, 0), -- Red Skeletal Horse
(5000352, 13332, 0, 0, 0), -- Blue Skeletal Horse
(5000352, 13333, 0, 0, 0), -- Brown Skeletal Horse
(5000352, 13334, 0, 0, 0), -- Green Skeletal Warhorse
(5000352, 18241, 0, 0, 0), -- Black War Steed Bridle
(5000352, 18248, 0, 0, 0), -- Red Skeletal Warhorse
(5000352, 18791, 0, 0, 0), -- Purple Skeletal Warhorse
(5000352, 29468, 0, 0, 0), -- Black War Steed Bridle
(5000352, 29470, 0, 0, 0), -- Red Skeletal Warhorse
(5000352, 30480, 0, 0, 0), -- Fiery Warhorse's Reins
(5000352, 37012, 0, 0, 0), -- The Horseman's Reins
(5000352, 45597, 0, 0, 0), -- Forsaken Warhorse
(5000352, 46101, 0, 0, 0), -- Blue Skeletal Warhorse
(5000352, 46308, 0, 0, 0), -- Black Skeletal Horse
(5000352, 46746, 0, 0, 0), -- White Skeletal Warhorse
(5000352, 46764, 0, 0, 0), -- White Skeletal Warhorse
(5000352, 47101, 0, 0, 0), -- Ochre Skeletal Warhorse
(5000352, 47180, 0, 0, 0), -- Argent Warhorse
(5000352, 49096, 0, 0, 0), -- Crusader's White Warhorse
(5000352, 49098, 0, 0, 0), -- Crusader's Black Warhorse
(5000352, 18777, 0, 0, 0), -- Swift Brown Steed
(5000352, 18778, 0, 0, 0), -- Swift White Steed
(5000352, 40775, 0, 0, 0), -- Winged Steed of the Ebon Blade
(5000352, 45125, 0, 0, 0), -- Stormwind Steed
(5000352, 46752, 0, 0, 0), -- Swift Gray Steed
(5000352, 46758, 0, 0, 0), -- Swift Gray Steed
(5000352, 46815, 0, 0, 0), -- Quel'dorei Steed
(5000352, 49044, 0, 0, 0), -- Swift Alliance Steed
(5000352, 54811, 0, 0, 0), -- Celestial Steed
(5000352, 13335, 0, 0, 0), -- Deathcharger's Reins
(5000352, 18776, 0, 0, 0), -- Swift Palomino
(5000352, 19030, 0, 0, 0), -- Stormpike Battle Charger
(5000352, 47179, 0, 0, 0), -- Argent Charger
(5000352, 43956, 0, 0, 0), -- Reins of the Black War Mammoth
(5000352, 43958, 0, 0, 0), -- Reins of the Ice Mammoth
(5000352, 43959, 0, 0, 0), -- Reins of the Grand Black War Mammoth
(5000352, 43961, 0, 0, 0), -- Reins of the Grand Ice Mammoth
(5000352, 44077, 0, 0, 0), -- Reins of the Black War Mammoth
(5000352, 44080, 0, 0, 0), -- Reins of the Ice Mammoth
(5000352, 44083, 0, 0, 0), -- Reins of the Grand Black War Mammoth
(5000352, 44086, 0, 0, 0), -- Reins of the Grand Ice Mammoth
(5000352, 44230, 0, 0, 0), -- Reins of the Wooly Mammoth
(5000352, 44231, 0, 0, 0), -- Reins of the Wooly Mammoth
(5000352, 44234, 0, 0, 0), -- Reins of the Traveler's Tundra Mammoth
(5000352, 44235, 0, 0, 0), -- Reins of the Traveler's Tundra Mammoth
(5000352, 54068, 0, 0, 0); -- Wooly White Rhino

-- Group 3: Wolves, Bears, Tigers, Sabers, Spectral, and related
REPLACE INTO `npc_vendor` (`entry`, `item`, `maxcount`, `incrtime`, `ExtendedCost`) VALUES
(5000353, 1132, 0, 0, 0), -- Horn of the Timber Wolf
(5000353, 5665, 0, 0, 0), -- Horn of the Dire Wolf
(5000353, 5668, 0, 0, 0), -- Horn of the Brown Wolf
(5000353, 12330, 0, 0, 0), -- Horn of the Red Wolf
(5000353, 12351, 0, 0, 0), -- Horn of the Arctic Wolf
(5000353, 18245, 0, 0, 0), -- Horn of the Black War Wolf
(5000353, 18796, 0, 0, 0), -- Horn of the Swift Brown Wolf
(5000353, 18797, 0, 0, 0), -- Horn of the Swift Timber Wolf
(5000353, 18798, 0, 0, 0), -- Horn of the Swift Gray Wolf
(5000353, 19029, 0, 0, 0), -- Horn of the Frostwolf Howler
(5000353, 29469, 0, 0, 0), -- Horn of the Black War Wolf
(5000353, 45595, 0, 0, 0), -- Orgrimmar Wolf
(5000353, 46099, 0, 0, 0), -- Horn of the Black Wolf
(5000353, 46749, 0, 0, 0), -- Swift Burgundy Wolf
(5000353, 46757, 0, 0, 0), -- Swift Burgundy Wolf
(5000353, 49046, 0, 0, 0), -- Swift Horde Wolf
(5000353, 33809, 0, 0, 0), -- Amani War Bear
(5000353, 38576, 0, 0, 0), -- Big Battle Bear
(5000353, 40777, 0, 0, 0), -- Polar Bear Harness
(5000353, 43599, 0, 0, 0), -- Big Blizzard Bear
(5000353, 43962, 0, 0, 0), -- Reins of the White Polar Bear
(5000353, 43963, 0, 0, 0), -- Reins of the Brown Polar Bear
(5000353, 43964, 0, 0, 0), -- Reins of the Black Polar Bear
(5000353, 44223, 0, 0, 0), -- Reins of the Black War Bear
(5000353, 44224, 0, 0, 0), -- Reins of the Black War Bear
(5000353, 44225, 0, 0, 0), -- Reins of the Armored Brown Bear
(5000353, 44226, 0, 0, 0), -- Reins of the Armored Brown Bear
(5000353, 49282, 0, 0, 0), -- Big Battle Bear
(5000353, 18242, 0, 0, 0), -- Reins of the Black War Tiger
(5000353, 19902, 0, 0, 0), -- Swift Zulian Tiger
(5000353, 29471, 0, 0, 0), -- Reins of the Black War Tiger
(5000353, 33224, 0, 0, 0), -- Reins of the Spectral Tiger
(5000353, 33225, 0, 0, 0), -- Reins of the Swift Spectral Tiger
(5000353, 49283, 0, 0, 0), -- Reins of the Spectral Tiger
(5000353, 49284, 0, 0, 0), -- Reins of the Swift Spectral Tiger
(5000353, 12325, 0, 0, 0), -- Reins of the Primal Leopard
(5000353, 8629, 0, 0, 0), -- Reins of the Striped Nightsaber
(5000353, 8631, 0, 0, 0), -- Reins of the Striped Frostsaber
(5000353, 8632, 0, 0, 0), -- Reins of the Spotted Frostsaber
(5000353, 12302, 0, 0, 0), -- Reins of the Ancient Frostsaber
(5000353, 12303, 0, 0, 0), -- Reins of the Nightsaber
(5000353, 12326, 0, 0, 0), -- Reins of the Tawny Sabercat
(5000353, 12327, 0, 0, 0), -- Reins of the Golden Sabercat
(5000353, 13086, 0, 0, 0), -- Reins of the Winterspring Frostsaber
(5000353, 18766, 0, 0, 0), -- Reins of the Swift Frostsaber
(5000353, 18767, 0, 0, 0), -- Reins of the Swift Mistsaber
(5000353, 18768, 0, 0, 0), -- Reins of the Swift Dawnsaber
(5000353, 18902, 0, 0, 0), -- Reins of the Swift Stormsaber
(5000353, 45591, 0, 0, 0), -- Darnassian Nightsaber
(5000353, 46744, 0, 0, 0), -- Swift Moonsaber
(5000353, 46759, 0, 0, 0), -- Swift Moonsaber
(5000353, 47100, 0, 0, 0); -- Reins of the Striped Dawnsaber

-- Group 4: Raptors, Netherwing Drakes, Proto-Drakes, and related
REPLACE INTO `npc_vendor` (`entry`, `item`, `maxcount`, `incrtime`, `ExtendedCost`) VALUES
(5000354, 8586, 0, 0, 0), -- Whistle of the Mottled Red Raptor
(5000354, 8588, 0, 0, 0), -- Whistle of the Emerald Raptor
(5000354, 8591, 0, 0, 0), -- Whistle of the Turquoise Raptor
(5000354, 8592, 0, 0, 0), -- Whistle of the Violet Raptor
(5000354, 13317, 0, 0, 0), -- Whistle of the Ivory Raptor
(5000354, 18246, 0, 0, 0), -- Whistle of the Black War Raptor
(5000354, 18788, 0, 0, 0), -- Swift Blue Raptor
(5000354, 18789, 0, 0, 0), -- Swift Olive Raptor
(5000354, 18790, 0, 0, 0), -- Swift Orange Raptor
(5000354, 19872, 0, 0, 0), -- Swift Razzashi Raptor
(5000354, 29472, 0, 0, 0), -- Whistle of the Black War Raptor
(5000354, 45593, 0, 0, 0), -- Darkspear Raptor
(5000354, 46743, 0, 0, 0), -- Swift Purple Raptor
(5000354, 46760, 0, 0, 0), -- Swift Purple Raptor
(5000354, 30609, 0, 0, 0), -- Swift Nether Drake
(5000354, 32857, 0, 0, 0), -- Reins of the Onyx Netherwing Drake
(5000354, 32858, 0, 0, 0), -- Reins of the Azure Netherwing Drake
(5000354, 32859, 0, 0, 0), -- Reins of the Cobalt Netherwing Drake
(5000354, 32860, 0, 0, 0), -- Reins of the Purple Netherwing Drake
(5000354, 32861, 0, 0, 0), -- Reins of the Veridian Netherwing Drake
(5000354, 32862, 0, 0, 0), -- Reins of the Violet Netherwing Drake
(5000354, 34092, 0, 0, 0), -- Merciless Nether Drake
(5000354, 37676, 0, 0, 0), -- Vengeful Nether Drake
(5000354, 43516, 0, 0, 0), -- Brutal Nether Drake
(5000354, 43951, 0, 0, 0), -- Reins of the Bronze Drake
(5000354, 43952, 0, 0, 0), -- Reins of the Azure Drake
(5000354, 43953, 0, 0, 0), -- Reins of the Blue Drake
(5000354, 43954, 0, 0, 0), -- Reins of the Twilight Drake
(5000354, 43955, 0, 0, 0), -- Reins of the Red Drake
(5000354, 43986, 0, 0, 0), -- Reins of the Black Drake
(5000354, 44151, 0, 0, 0), -- Reins of the Blue Proto-Drake
(5000354, 44160, 0, 0, 0), -- Reins of the Red Proto-Drake
(5000354, 44164, 0, 0, 0), -- Reins of the Black Proto-Drake
(5000354, 44168, 0, 0, 0), -- Reins of the Time-Lost Proto-Drake
(5000354, 44175, 0, 0, 0), -- Reins of the Plagued Proto-Drake
(5000354, 44177, 0, 0, 0), -- Reins of the Violet Proto-Drake
(5000354, 44178, 0, 0, 0), -- Reins of the Albino Drake
(5000354, 44707, 0, 0, 0), -- Reins of the Green Proto-Drake
(5000354, 45801, 0, 0, 0), -- Reins of the Ironbound Proto-Drake
(5000354, 45802, 0, 0, 0), -- Reins of the Rusted Proto-Drake
(5000354, 49636, 0, 0, 0); -- Reins of the Onyxian Drake

-- Group 5: Hawkstriders, Wind Riders, Dragonhawks, Nether Rays, Gryphons, Hippogryphs, Mechanostriders, and related
REPLACE INTO `npc_vendor` (`entry`, `item`, `maxcount`, `incrtime`, `ExtendedCost`) VALUES
(5000355, 28927, 0, 0, 0), -- Red Hawkstrider
(5000355, 28936, 0, 0, 0), -- Swift Pink Hawkstrider
(5000355, 29220, 0, 0, 0), -- Blue Hawkstrider
(5000355, 29221, 0, 0, 0), -- Black Hawkstrider
(5000355, 29222, 0, 0, 0), -- Purple Hawkstrider
(5000355, 29223, 0, 0, 0), -- Swift Green Hawkstrider
(5000355, 29224, 0, 0, 0), -- Swift Purple Hawkstrider
(5000355, 35513, 0, 0, 0), -- Swift White Hawkstrider
(5000355, 45596, 0, 0, 0), -- Silvermoon Hawkstrider
(5000355, 46751, 0, 0, 0), -- Swift Red Hawkstrider
(5000355, 46761, 0, 0, 0), -- Swift Red Hawkstrider
(5000355, 46816, 0, 0, 0), -- Sunreaver Hawkstrider
(5000355, 25474, 0, 0, 0), -- Tawny Wind Rider
(5000355, 25475, 0, 0, 0), -- Blue Wind Rider
(5000355, 25476, 0, 0, 0), -- Green Wind Rider
(5000355, 25477, 0, 0, 0), -- Swift Red Wind Rider
(5000355, 25531, 0, 0, 0), -- Swift Green Wind Rider
(5000355, 25532, 0, 0, 0), -- Swift Yellow Wind Rider
(5000355, 25533, 0, 0, 0), -- Swift Purple Wind Rider
(5000355, 44690, 0, 0, 0), -- Armored Blue Wind Rider
(5000355, 44842, 0, 0, 0), -- Red Dragonhawk Mount
(5000355, 44843, 0, 0, 0), -- Blue Dragonhawk Mount
(5000355, 46814, 0, 0, 0), -- Sunreaver Dragonhawk
(5000355, 32314, 0, 0, 0), -- Green Riding Nether Ray
(5000355, 32316, 0, 0, 0), -- Purple Riding Nether Ray
(5000355, 32317, 0, 0, 0), -- Red Riding Nether Ray
(5000355, 32318, 0, 0, 0), -- Silver Riding Nether Ray
(5000355, 32319, 0, 0, 0), -- Blue Riding Nether Ray
(5000355, 25470, 0, 0, 0), -- Golden Gryphon
(5000355, 25471, 0, 0, 0), -- Ebon Gryphon
(5000355, 25472, 0, 0, 0), -- Snowy Gryphon
(5000355, 25473, 0, 0, 0), -- Swift Blue Gryphon
(5000355, 25527, 0, 0, 0), -- Swift Red Gryphon
(5000355, 25528, 0, 0, 0), -- Swift Green Gryphon
(5000355, 25529, 0, 0, 0), -- Swift Purple Gryphon
(5000355, 44689, 0, 0, 0), -- Armored Snowy Gryphon
(5000355, 33999, 0, 0, 0), -- Cenarion War Hippogryph
(5000355, 45725, 0, 0, 0), -- Argent Hippogryph
(5000355, 46813, 0, 0, 0), -- Silver Covenant Hippogryph
(5000355, 54069, 0, 0, 0), -- Blazing Hippogryph
(5000355, 8563, 0, 0, 0), -- Red Mechanostrider
(5000355, 8595, 0, 0, 0), -- Blue Mechanostrider
(5000355, 13321, 0, 0, 0), -- Green Mechanostrider
(5000355, 13322, 0, 0, 0), -- Unpainted Mechanostrider
(5000355, 13323, 0, 0, 0), -- Purple Mechanostrider
(5000355, 13324, 0, 0, 0), -- Red and Blue Mechanostrider
(5000355, 13325, 0, 0, 0), -- Fluorescent Green Mechanostrider
(5000355, 13326, 0, 0, 0), -- White Mechanostrider Mod B
(5000355, 13327, 0, 0, 0), -- Icy Blue Mechanostrider Mod A
(5000355, 18243, 0, 0, 0), -- Black Battlestrider
(5000355, 18772, 0, 0, 0), -- Swift Green Mechanostrider
(5000355, 18773, 0, 0, 0), -- Swift White Mechanostrider
(5000355, 18774, 0, 0, 0), -- Swift Yellow Mechanostrider
(5000355, 29465, 0, 0, 0), -- Black Battlestrider
(5000355, 34129, 0, 0, 0), -- Swift Warstrider
(5000355, 45589, 0, 0, 0), -- Gnomeregan Mechanostrider
(5000355, 46747, 0, 0, 0), -- Turbostrider
(5000355, 46763, 0, 0, 0); -- Turbostrider

-- Group 6: Special, Qiraji, Carpets, Brooms, Rockets, Frost Wyrms, and other unique mounts
REPLACE INTO `npc_vendor` (`entry`, `item`, `maxcount`, `incrtime`, `ExtendedCost`) VALUES
(5000356, 32458, 0, 0, 0), -- Ashes of Al'ar
(5000356, 32768, 0, 0, 0), -- Reins of the Raven Lord
(5000356, 34060, 0, 0, 0), -- Flying Machine Control
(5000356, 34061, 0, 0, 0), -- Turbo-Charged Flying Machine Control
(5000356, 35225, 0, 0, 0), -- X-51 Nether-Rocket
(5000356, 35226, 0, 0, 0), -- X-51 Nether-Rocket X-TREME
(5000356, 37719, 0, 0, 0), -- Swift Zhevra
(5000356, 41508, 0, 0, 0), -- Mechano-hog
(5000356, 45693, 0, 0, 0), -- Mimiron's Head
(5000356, 46102, 0, 0, 0), -- Whistle of the Venomhide Ravasaur
(5000356, 46109, 0, 0, 0), -- Sea Turtle
(5000356, 46171, 0, 0, 0), -- Furious Gladiator's Frost Wyrm
(5000356, 46708, 0, 0, 0), -- Deadly Gladiator's Frost Wyrm
(5000356, 46778, 0, 0, 0), -- Magic Rooster Egg
(5000356, 47840, 0, 0, 0), -- Relentless Gladiator's Frost Wyrm
(5000356, 49285, 0, 0, 0), -- X-51 Nether-Rocket
(5000356, 49286, 0, 0, 0), -- X-51 Nether-Rocket X-TREME
(5000356, 49290, 0, 0, 0), -- Magic Rooster Egg
(5000356, 50250, 0, 0, 0), -- Big Love Rocket
(5000356, 50435, 0, 0, 0), -- Wrathful Gladiator's Frost Wyrm
(5000356, 50818, 0, 0, 0), -- Invincible's Reins
(5000356, 51954, 0, 0, 0), -- Reins of the Bloodbathed Frostbrood Vanquisher
(5000356, 51955, 0, 0, 0), -- Reins of the Icebound Frostbrood Vanquisher
(5000356, 52200, 0, 0, 0), -- Reins of the Crimson Deathcharger
(5000356, 54860, 0, 0, 0), -- X-53 Touring Rocket
(5000356, 21176, 0, 0, 0), -- Black Qiraji Resonating Crystal
(5000356, 21218, 0, 0, 0), -- Blue Qiraji Resonating Crystal
(5000356, 21321, 0, 0, 0), -- Red Qiraji Resonating Crystal
(5000356, 21323, 0, 0, 0), -- Green Qiraji Resonating Crystal
(5000356, 21324, 0, 0, 0), -- Yellow Qiraji Resonating Crystal
(5000356, 44413, 0, 0, 0), -- Mekgineer's Chopper
(5000356, 44554, 0, 0, 0), -- Flying Carpet
(5000356, 44555, 0, 0, 0), -- Swift Mooncloth Carpet
(5000356, 44556, 0, 0, 0), -- Swift Spellfire Carpet
(5000356, 44557, 0, 0, 0), -- Swift Ebonweave Carpet
(5000356, 39303, 0, 0, 0), -- Swift Flying Carpet
(5000356, 44558, 0, 0, 0), -- Magnificent Flying Carpet
(5000356, 54797, 0, 0, 0), -- Frosty Flying Carpet
(5000356, 33176, 0, 0, 0), -- Flying Broom
(5000356, 33182, 0, 0, 0), -- Swift Flying Broom
(5000356, 33183, 0, 0, 0), -- Old Magic Broom
(5000356, 33184, 0, 0, 0), -- Swift Magic Broom
(5000356, 37011, 0, 0, 0); -- Magic Broom