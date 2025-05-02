CREATE TABLE IF NOT EXISTS `guild_house_spawns` (
  `id` int NOT NULL AUTO_INCREMENT,
  `entry` int NOT NULL DEFAULT '0',
  `posX` float NOT NULL DEFAULT '0',
  `posY` float NOT NULL DEFAULT '0',
  `posZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `comment` varchar(500) DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `entry` (`entry`)
) ENGINE=InnoDB AUTO_INCREMENT=51 DEFAULT CHARSET=UTF8MB4;

REPLACE INTO `guild_house_spawns` (`id`, `entry`, `posX`, `posY`, `posZ`, `orientation`, `comment`) VALUES
-- Class Trainers
(1, 26327, 16216.5, 16279.4, 20.9306, 0.552869, 'Paladin Trainer'),
(2, 26324, 16221.3, 16275.7, 20.9285, 1.37363, 'Druid Trainer'),
(3, 26325, 16218.6, 16277, 20.9872, 0.967188, 'Hunter Trainer'),
(4, 26326, 16224.9, 16274.9, 20.9319, 1.58765, 'Mage Trainer'),
(5, 26328, 16227.9, 16275.9, 20.9254, 1.9941, 'Priest Trainer'),
(6, 26329, 16231.4, 16278.1, 20.9222, 2.20026, 'Rogue Trainer'),
(7, 26330, 16235.5, 16280.8, 20.9257, 2.18652, 'Shaman Trainer'),
(8, 26331, 16240.8, 16283.3, 20.9299, 1.86843, 'Warlock Trainer'),
(9, 26332, 16246.6, 16284.5, 20.9301, 1.68975, 'Warrior Trainer'),
(10, 29195, 16252.3, 16284.9, 20.9324, 1.79537, 'Death Knight Trainer'),

-- Essential NPCs
(11, 6930, 16220.1, 16287.7, 13.1753, 6.18533, 'Innkeeper'),
(12, 30605, 16228, 16280.5, 13.1761, 2.98877, 'Banker'),
(13, 8661, 16240, 16291.9, 22.9311, 1.5, 'Auctioneer Beardo'),
(14, 6491, 16319.9, 16242.4, 24.4747, 2.20683, 'Spirit Healer'),
(15, 28047, 16227, 16267.9, 13.15, 4.6533, 'Stable Master (Mei Francis)'),

-- Dalaran Master Profession Trainers
(16, 28703, 16233.293, 16325.65, 20.922863, 4.65, 'Alchemy Trainer (Linzy Blackbolt)'),
(17, 28694, 16230.171, 16326.046, 20.92381, 4.65, 'Blacksmithing Trainer (Alard Schmied)'),
(18, 28693, 16226.933, 16326.357, 20.924126, 4.65, 'Enchanting Trainer (Enchanter Nalthanis)'),
(19, 29513, 16223.973, 16326.623, 20.922243, 4.65, 'Engineering Trainer (Didi the Wrench)'),
(20, 28702, 16238.571, 16325.184, 20.92316, 4.65, 'Inscription Trainer (Professor Pallin)'),
(21, 28701, 16236.166, 16325.681, 20.928278, 4.65, 'Jewelcrafting Trainer (Timothy Jones)'),
(22, 28700, 16249.182, 16324.335, 20.925695, 4.65, 'Leatherworking Trainer (Awan Iceborn)'),
(23, 28697, 16234.0, 16300.0, 13.2, 4.65, 'Mining Trainer (Jedidiah Handers)'),
(24, 28704, 16243.894, 16324.694, 20.924065, 4.65, 'Herbalism Trainer (Dorothy Egan)'),
(25, 28699, 16246.662, 16324.426, 20.923574, 4.65, 'Tailoring Trainer (Charles Worth)'),
(26, 19185, 16240.0, 16300.0, 13.2, 4.65, 'First Aid Trainer (Brynna Wilson)'),
(27, 600001, 16254.671, 16323.381, 20.919258, 4.65, 'Cooking Trainer (Custom)'),
(28, 28742, 16207.1, 16213.2, 1.02763, 1.4572, 'Fishing Trainer (Marcia Chase)'),
(29, 29513, 16251.971, 16323.958, 20.92428, 4.65, 'Engineering Trainer (Timofey Oshenko)'),

-- Vendors
(30, 28692, 16236.2, 16315.7, 20.8454, 4.64365, 'Trade Supplies'),
(31, 28776, 16224.3, 16294.6, 20.8541, 6.26512, 'Reagent Vendor'),
(32, 4255, 16230.2, 16316.1, 20.8455, 4.64365, 'Leatherworking Supplies'),
(33, 29636, 16233.2, 16315.9, 20.8454, 4.64365, 'Stable Supplies'),
(34, 29493, 16229.1, 16286.4, 13.176, 3.03831, 'Specialty Ammunition Vendor'),
(35, 2622, 16242.8, 16302.1, 13.176, 4.5557, 'Weapon Vendor'),
(36, 32478, 16246.0, 16302.1, 13.176, 4.5557, 'Food & Drink Vendor'),
(37, 500033, 16225, 16307.0, 29.263, 6.22119, 'Dual Spec Trainer'),
(38, 500034, 16225, 16309.0, 29.263, 6.22119, 'Transmogrifier'),
(39, 500036, 16225, 16313.0, 29.263, 6.22119, 'Heirloom Vendor'),
(40, 29533, 16225, 16315.0, 29.263, 6.22119, 'Battlemaster'),
(41, 5000351, 16233.446, 16320.974, 20.85582, 1.5, 'Mount Vendor: Hoofed'),
(42, 5000352, 16238.438, 16320.926, 20.860443, 1.5, 'Mount Vendor: Steeds'),
(43, 5000353, 16241.24, 16320.743, 20.858412, 1.5, 'Mount Vendor: Wild'),
(44, 5000354, 16236.041, 16320.894, 20.857548, 1.5, 'Mount Vendor: Raptors & Drakes'),
(45, 5000355, 16246.173, 16320.229, 20.86169, 1.5, 'Mount Vendor: Exotics'),
(46, 5000356, 16243.798, 16320.507, 20.861155, 1.5, 'Mount Vendor: Special'),

-- Portals
(47, 500000, 16257.589, 16252.013, 22.178692, 0.0, 'Stormwind Portal'),
(48, 500001, 16267.484, 16244.679, 25.803255, 0.0, 'Exodar Portal'),
(49, 500002, 16262.1045, 16247.697, 24.280844, 0.0, 'Darnassus Portal'),
(50, 500003, 16253.489, 16258.224, 19.10805, 0.0, 'Ironforge Portal'),
(51, 500004, 16279.409, 16264.733, 20.226196, 0.0, 'Orgrimmar Portal'),
(52, 500005, 16273.9, 16269.5, 18.32, 0.0, 'Silvermoon Portal'),
(53, 500006, 16283.128, 16259.68, 22.388813, 0.0, 'Thunder Bluff Portal'),
(54, 500007, 16283.408, 16254.021, 24.274466, 0.0, 'Undercity Portal'),
(55, 500008, 16279.706, 16248.536, 25.480366, 0.0, 'Shattrath Portal'),
(56, 500009, 16273.428, 16245.385, 26.067795, 0.0, 'Dalaran Portal'),

-- Objects
(57, 500040, 16218.0, 16312.0, 21.0, 0.0, 'Training Dummy'),
(58, 500041, 16220.0, 16312.0, 21.0, 0.0, 'Forge'),
(59, 500042, 16222.0, 16312.0, 21.0, 0.0, 'Anvil'),
(60, 500043, 16224.0, 16312.0, 21.0, 0.0, 'Alchemy Lab'),
(61, 500044, 16226.0, 16312.0, 21.0, 0.0, 'Cooking Fire'),
(62, 1685, 16228.0, 16314.0, 21.0, 0.0, 'Guild Vault'),
(63, 4087, 16230.0, 16314.0, 21.0, 0.0, 'Guild Bank'),
(64, 184137, 16220.2, 16272.8, 12.9625, 2.12365, 'Mailbox'),


-- Kendor Kabonka (Alliance)
(65, 340, 16260.0, 16310.0, 21.0, 0.0, 'Alliance Cooking Recipe Vendor'),
-- Keena (Horde)
(66, 35556, 16262.0, 16310.0, 21.0, 0.0, 'Horde Cooking Recipe Vendor');
