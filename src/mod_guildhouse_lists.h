#pragma once
#include <set>
#include <vector>
#include <cstdint>

static const std::set<uint32_t> essentialNpcEntries = {
    6930,  // Innkeeper
    28047, // Stable Master (Mei Francis)
    8661,  // Auctioneer Beardo (Neutral)
    6491   // Spirit Healer
};

static const std::set<uint32_t> classTrainerEntries = {
    26327, // Death Knight
    26324, // Druid
    26325, // Hunter
    26326, // Mage
    26328, // Paladin
    26329, // Priest
    26330, // Rogue
    26331, // Shaman
    26332, // Warlock
    29195  // Warrior
};

static const std::set<uint32_t> professionTrainerEntries = {
    28703, // Alchemy (Linzy Blackbolt)
    28694, // Blacksmithing (Alard Schmied)
    28693, // Enchanting (Enchanter Nalthanis)
    29513, // Engineering (Didi the Wrench)
    28702, // Inscription (Professor Pallin)
    28701, // Jewelcrafting (Timothy Jones)
    28700, // Leatherworking (Awan Iceborn)
    28697, // Mining (Jedidiah Handers)
    28704, // Herbalism (Dorothy Egan)
    28699, // Tailoring (Charles Worth)
    19185, // First Aid (Brynna Wilson, not in Dalaran but still Master)
    28742, // Fishing (Marcia Chase)
    600001 // Neutral Grand Master Cooking Trainer
};

static const std::set<uint32_t> vendorEntries = {
    500033,  // Dual Spec Trainer
    500034,  // Transmogrifier
    5000351, // Mount Vendor: Hoofed
    5000352, // Mount Vendor: Steeds
    5000353, // Mount Vendor: Wild
    5000354, // Mount Vendor: Raptors & Drakes
    5000355, // Mount Vendor: Exotics
    5000356, // Mount Vendor: Special
    500036,  // Heirloom Vendor
    29533,   // Battlemaster (Alliance/Horde)
    32478,   // Food & Drink Vendor (Slosh)
    29493,   // Specialty Ammunition Vendor (Alchemist Finklestein)
    // ...other vendors...
};

static const std::set<uint32_t> objectEntries = {
    184137, // Mailbox
    1685,   // Guild Vault
    4087,   // Guild Bank
    500040, // Training Dummy
    500041, // Forge
    500042, // Anvil
    500043, // Alchemy Lab
    500044  // Cooking Fire
    // Add more as needed
};

static const std::set<uint32_t> portalEntries = {
    500000, // Stormwind
    500001, // Exodar
    500002, // Darnassus
    500003, // Ironforge
    500004, // Orgrimmar
    500005, // Silvermoon
    500006, // Thunder Bluff
    500007, // Undercity
    500008, // Shattrath
    500009  // Dalaran
};