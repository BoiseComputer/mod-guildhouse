#pragma once
#include <set>
#include <vector>
#include <cstdint>

static const std::set<uint32_t> essentialNpcEntries = {
    6930,  // Innkeeper
    28690, // Stable Master
    9858,  // Auctioneer Kresky
    8719,  // Auctioneer Fitch
    9856,  // Auctioneer Grimful
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
    2836,  // Alchemy
    8128,  // Blacksmithing
    8736,  // Enchanting
    19187, // Engineering
    19180, // Herbalism
    19052, // Inscription
    908,   // Leatherworking
    2627,  // Mining
    19184, // Skinning
    2834,  // Tailoring
    19185  // First Aid
};

static const std::set<uint32_t> vendorEntries = {
    28692,  // General Goods Vendor
    28776,  // Reagent Vendor
    4255,   // Leatherworking Supplies
    29636,  // Stable Supplies
    29493,  // Food & Drink Vendor
    2622,   // Weapon Vendor
    32478,  // Enchanting Supplies
    500033, // Dual Spec Trainer
    500034, // Transmog Vendor
    500035, // Mount Vendor
    500036, // Fishing Vendor
    500037  // Battlemaster
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