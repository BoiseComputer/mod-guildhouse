-- !!! NOTE: set these before running the queries in order to avoid conflicts !!!
SET @C_TEMPLATE = 500030;

-- Insert new creature templates
REPLACE INTO `creature_template` (`entry`, `difficulty_entry_1`, `difficulty_entry_2`, `difficulty_entry_3`, `KillCredit1`, `KillCredit2`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `speed_walk`, `speed_run`, `speed_swim`, `speed_flight`, `detection_range`, `scale`, `rank`, `dmgschool`, `DamageModifier`, `BaseAttackTime`, `RangeAttackTime`, `BaseVariance`, `RangeVariance`, `unit_class`, `unit_flags`, `unit_flags2`, `dynamicflags`, `family`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `HealthModifier`, `ManaModifier`, `ArmorModifier`, `ExperienceModifier`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `spell_school_immune_mask`, `flags_extra`, `ScriptName`, `VerifiedBuild`) VALUES
    (@C_TEMPLATE + 0, 0, 0, 0, 0, 0, 'Talamortis', 'Guild House Seller', '',   0, 35, 35, 0, 35, 0, 1, 1.14286, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 33536, 2048, 0, 0, 0, 0, 0, 0, 7, 4096, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 'GuildHouseSeller', 12340),
    (@C_TEMPLATE + 1, 0, 0, 0, 0, 0, 'Xrispins', 'Guild House Butler',   '',   0, 35, 35, 0, 35, 0, 1, 1.14286, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 33536, 2048, 0, 0, 0, 0, 0, 0, 7, 4096, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 'GuildHouseSpawner', 12340),
    (@C_TEMPLATE + 2, 0, 0, 0, 0, 0, 'Innkeeper Monica', 'Innkeeper', '', 0, 1,   2, 0, 35, 65536, 0.8, 0.28571, 1, 1, 20, 1, 0, 0, 4.6, 2000, 1900, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, '', 1, 1, 1, 1, 1, 1, 0, 70, 1, 0, 0, 2, 'npc_innkeeper', 12340),
    (@C_TEMPLATE + 3, 0, 0, 0, 0, 0, 'Dual Spec Trainer', 'Specializations', '', 0, 80, 80, 0, 35, 1, 1, 1.14286, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 7, 4096, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, '', 12340),
    (@C_TEMPLATE + 4, 0, 0, 0, 0, 0, 'Transmogrifier', 'Appearance', '', 0, 80, 80, 0, 35, 1, 1, 1.14286, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 7, 4096, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, '', 12340),
    (@C_TEMPLATE + 6, 0, 0, 0, 0, 0, 'Heirloom Vendor', 'Heirlooms', '', 0, 80, 80, 0, 35, 128, 1, 1.14286, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 7, 4096, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, '', 12340),
    (@C_TEMPLATE + 7, 0, 0, 0, 0, 0, 'Battlemaster', 'Battlegrounds', '', 0, 80, 80, 0, 35, 2, 1, 1.14286, 1, 1, 20, 1, 0, 0, 1, 2000, 2000, 1, 1, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 7, 4096, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, '', 12340);

-- Insert matching models for each template
REPLACE INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(@C_TEMPLATE + 0, 0, 25901, 1, 1, 0),   -- Talamortis (Human Male)
(@C_TEMPLATE + 1, 0, 25903, 1, 1, 0),   -- Xrispins (Human Female)
(@C_TEMPLATE + 2, 0, 18234, 1, 1, 0),   -- Innkeeper Monica (Draenei Female)
(@C_TEMPLATE + 3, 0, 26893, 1, 1, 0),   -- Dual Spec Trainer (Blood Elf Male)
(@C_TEMPLATE + 4, 0, 26894, 1, 1, 0),   -- Transmogrifier (Blood Elf Female)
(@C_TEMPLATE + 6, 0, 19724, 1, 1, 0),   -- Heirloom Vendor (Orc Female)
(@C_TEMPLATE + 7, 0, 21139, 1, 1, 0);   -- Battlemaster (Tauren Male)

SET @C_TEMPLATE = 5000351;

-- Add after your existing creature_template block
REPLACE INTO `creature_template` (`entry`, `name`, `subname`, `unit_class`, `minlevel`, `maxlevel`, `faction`, `npcflag`) VALUES
(@C_TEMPLATE + 0, 'Mount Vendor: Hoofed', 'Rams, Talbuks, Elekks, Kodos', 1, 80, 80, 35, 128),
(@C_TEMPLATE + 1, 'Mount Vendor: Steeds', 'Horses, Skeletal, Mammoths', 1, 80, 80, 35, 128),
(@C_TEMPLATE + 2, 'Mount Vendor: Wild', 'Wolves, Bears, Tigers, Sabers', 1, 80, 80, 35, 128),
(@C_TEMPLATE + 3, 'Mount Vendor: Raptors & Drakes', 'Raptors, Netherwing, Proto, etc', 1, 80, 80, 35, 128),
(@C_TEMPLATE + 4, 'Mount Vendor: Exotics', 'Hawkstriders, Mechanostriders, etc', 1, 80, 80, 35, 128),
(@C_TEMPLATE + 5, 'Mount Vendor: Special', 'Special, Qiraji, Carpets, Brooms', 1, 80, 80, 35, 128);

-- Mount Vendor variants
REPLACE INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(@C_TEMPLATE + 0, 0, 657, 1, 1, 0),             -- Mount Vendor: Hoofed (Crazy Leonetti model)
(@C_TEMPLATE + 1, 0, 25901, 1, 1, 0),           -- Mount Vendor: Steeds (Human Male)
(@C_TEMPLATE + 2, 0, 19748, 1, 1, 0),           -- Mount Vendor: Wild (Orc Male)
(@C_TEMPLATE + 3, 0, 21175, 1, 1, 0),           -- Mount Vendor: Raptors & Drakes (Troll Male)
(@C_TEMPLATE + 4, 0, 27987, 1, 1, 0),           -- Mount Vendor: Exotics (Creature)
(@C_TEMPLATE + 5, 0, 25903, 1, 1, 0);           -- Mount Vendor: Special (Human Female)

-- !!! NOTE: set these before running the queries in order to avoid conflicts !!!
SET @GO_TEMPLATE = 500000;

REPLACE INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(@GO_TEMPLATE + 0, 22, 4396, 'Portal to Stormwind', '', '', '', 1, 17334, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 1, 22, 4393, 'Portal to Darnassus', '', '', '', 1, 17608, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 2, 22, 6955, 'Portal to Exodar', '', '', '', 1, 32268, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 3, 22, 4394, 'Portal to Ironforge', '', '', '', 1, 17607, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 4, 22, 4395, 'Portal to Orgrimmar', '', '', '', 1, 17609, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 5, 22, 6956, 'Portal to Silvermoon', '', '', '', 1, 32270, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 6, 22, 4397, 'Portal to Thunder Bluff', '', '', '', 1, 17610, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 7, 22, 4398, 'Portal to Undercity', '', '', '', 1, 17611, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 8, 22, 7146, 'Portal to Shattrath', '', '', '', 1, 35718, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 9, 22, 8111, 'Portal to Dalaran', '', '', '', 1, 53141, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 10, 22, 7967, 'Portal to Isle of Quel''Danas', '', '', '', 1, 49361, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 11, 22, 4399, 'Portal to Caverns of Time', '', '', '', 1, 32271, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0);

-- Add these after your existing gameobject_template entries

SET @GO_TEMPLATE = 500040;

REPLACE INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `Data0`, `Data1`, `Data2`, `Data3`, `Data4`, `Data5`, `Data6`, `Data7`, `Data8`, `Data9`, `Data10`, `Data11`, `Data12`, `Data13`, `Data14`, `Data15`, `Data16`, `Data17`, `Data18`, `Data19`, `Data20`, `Data21`, `Data22`, `Data23`, `AIName`, `ScriptName`, `VerifiedBuild`) VALUES
(@GO_TEMPLATE + 0, 33, 4467, 'Training Dummy', '', '', '', 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 1, 7, 192, 'Forge', '', '', '', 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 2, 7, 4085, 'Anvil', '', '', '', 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 3, 7, 500043, 'Alchemy Lab', '', '', '', 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0),
(@GO_TEMPLATE + 4, 7, 416, 'Cooking Fire', '', '', '', 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', '', 0);


SET @GO_TEMPLATE = 600000;

REPLACE INTO `creature_template` (`entry`, `name`, `subname`, `minlevel`, `maxlevel`, `faction`, `npcflag`, `trainer_type`, `trainer_class`, `trainer_race`) VALUES 
(@GO_TEMPLATE + 1, 'Grand Master Cooking Trainer', 'Cooking Trainer', 80, 80, 35, 16, 2, 0, 0);

REPLACE INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(@GO_TEMPLATE + 1, 0, 25903, 1, 1, 0); -- Human Female (or choose any displayId you prefer)

REPLACE INTO `npc_trainer` (`ID`, `SpellID`, `MoneyCost`, `ReqSkillLine`, `ReqSkillRank`, `ReqLevel`) VALUES
(@GO_TEMPLATE + 1, 2550, 1000, 185, 0, 1),      -- Cooking (Apprentice)
(@GO_TEMPLATE + 1, 3102, 5000, 185, 50, 10),    -- Journeyman Cooking
(@GO_TEMPLATE + 1, 3413, 10000, 185, 125, 20),  -- Expert Cooking
(@GO_TEMPLATE + 1, 18260, 25000, 185, 200, 35), -- Artisan Cooking
(@GO_TEMPLATE + 1, 33359, 50000, 185, 275, 50), -- Master Cooking
(@GO_TEMPLATE + 1, 51296, 100000, 185, 350, 65);-- Grand Master Cooking