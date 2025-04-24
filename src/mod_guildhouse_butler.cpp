#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Configuration/Config.h"
#include "Creature.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Define.h" // This header defines uint32 and other types
#include "GossipDef.h"
#include "DataMap.h"
#include "GameObject.h"
#include "Transport.h"
#include "CreatureAI.h"
#include "MapMgr.h"
#include "mod_guildhouse_lists.h"
#include <sstream> // Add this at the top if not present

extern int GuildHouseInnKeeper, GuildHouseBank, GuildHouseMailBox, GuildHouseAuctioneer, GuildHouseTrainer, GuildHouseVendor, GuildHouseObject, GuildHousePortal, GuildHouseSpirit, GuildHouseProf, GuildHouseBuyRank;
extern int GuildHouseRefundPercent;
extern uint32 GuildHouseMarkerEntry;
extern uint32 GuildHousePhaseBase;

inline uint32 GetGuildPhase(Player *player)
{
    extern uint32 GuildHousePhaseBase;
    return GuildHousePhaseBase + player->GetGuildId();
}

void ClearGuildHousePhase(uint32 guildId, uint32 guildPhase, Map *map);
void SpawnNPC(uint32 entry, Player *player, bool force = false);
int GetGuildHouseEntryCost(uint32 entry); // Declare the function from mod_guildhouse.cpp

bool BuyAllEntries(Player *player, Creature *creature, const std::set<uint32> &entries, const std::string &type, uint32 submenu);
bool SellAllEntries(Player *player, Creature *creature, const std::set<uint32> &entries, const std::string &type, uint32 submenu);

// Global configuration variables for guild house features
extern int cost, GuildHouseInnKeeper, GuildHouseBank, GuildHouseMailBox, GuildHouseAuctioneer, GuildHouseTrainer, GuildHouseVendor, GuildHouseObject, GuildHousePortal, GuildHouseSpirit, GuildHouseProf, GuildHouseBuyRank;
extern int GuildHouseRefundPercent;
extern uint32 GuildHouseMarkerEntry;
extern uint32 GuildHousePhaseBase;

// Stores a player's last position for teleportation
struct LastPosition : public DataMap::Base
{
    uint32 mapId = 0;
    float x = 0, y = 0, z = 0, ori = 0;
};

// Event: Despawns a marker creature after a delay
class DespawnMarkerEvent : public BasicEvent
{
    Creature *marker_;

public:
    DespawnMarkerEvent(Creature *marker) : marker_(marker) {}

    bool Execute(uint64 /*time*/, uint32 /*diff*/) override
    {
        if (marker_)
        {
            marker_->RemoveAllAuras();
            if (marker_->IsInWorld())
                marker_->DespawnOrUnsummon();
            marker_->DeleteFromDB();
            marker_ = nullptr;
        }
        // Remove this event after execution
        return false;
    }
};

class GuildHouseSpawner : public CreatureScript
{

public:
    GuildHouseSpawner() : CreatureScript("GuildHouseSpawner") {}

    struct GuildHouseSpawnerAI : public ScriptedAI
    {
        GuildHouseSpawnerAI(Creature *creature) : ScriptedAI(creature) {}

        void UpdateAI(uint32 /*diff*/) override
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
    };

    CreatureAI *GetAI(Creature *creature) const override
    {
        return new GuildHouseSpawnerAI(creature);
    }

    bool OnGossipHello(Player *player, Creature *creature) override
    {
        if (!player->GetGuild())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("You are not in a guild!");
            return false;
        }

        QueryResult result = CharacterDatabase.Query("SELECT 1 FROM `guild_house` WHERE `guild`={}", player->GetGuildId());
        if (!result)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Your guild does not own a Guild House yet.");
            CloseGossipMenuFor(player);
            return false;
        }

        Guild *guild = sGuildMgr->GetGuildById(player->GetGuildId());
        Guild::Member const *memberMe = guild->GetMember(player->GetGUID());
        if (!memberMe->IsRankNotLower(GuildHouseBuyRank))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("You are not authorized to make Guild House purchases.");
            return false;
        }

        ClearGossipMenuFor(player);

        // Essential NPCs submenu
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Manage Essential NPCs", GOSSIP_SENDER_MAIN, 10020);

        // Vendors, Trainers, Portals, Objects
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Spawn Class Trainers", GOSSIP_SENDER_MAIN, 2);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Spawn Profession Trainers", GOSSIP_SENDER_MAIN, 5);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Spawn Vendors", GOSSIP_SENDER_MAIN, 3);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Spawn City Portals", GOSSIP_SENDER_MAIN, 10005);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Spawn Objects", GOSSIP_SENDER_MAIN, 10006);

        // Reset
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Reset Guild House (Remove & Re-place All)", GOSSIP_SENDER_MAIN, 900000, "Are you sure you want to reset your Guild House? This will remove and re-place all purchased objects and NPCs.", 0, false);

        // return option if we have a stored position
        auto last = player->CustomData.Get<LastPosition>("lastPos");
        if (last && last->mapId)
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Return to Previous Location", GOSSIP_SENDER_MAIN, 10030);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 /*sender*/, uint32 action) override
    {
        if (!player || !player->GetSession())
            return false;

        switch (action)
        {
        case 9: // Go Back!
            OnGossipHello(player, creature);
            break;

        // --- Essential NPCs Submenu ---
        case 10020:
        {
            ClearGossipMenuFor(player);
            uint32 guildPhase = this->GetGuildPhase(player);
            int notSpawned = 0, totalRefundAll = 0;

            for (auto entry : essentialNpcEntries)
            {
                if (entry == 6491 || entry == 500031) // Skip Spirit Healer and Butler
                    continue;
                bool spawned = false;
                for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                    {
                        spawned = true;
                        break;
                    }
                }
                int costValue = GetGuildHouseEntryCost(entry);

                std::string npcRole;
                switch (entry)
                {
                case 6930:
                    npcRole = "Innkeeper";
                    break;
                case 28690:
                    npcRole = "Stable Master";
                    break;
                case 6491:
                    npcRole = "Spirit Healer";
                    break;
                case 24545:
                    npcRole = "Guild Banker";
                    break;
                case 9856:
                    npcRole = "Auctioneer";
                    break;
                case 8719:
                    npcRole = "Auctioneer";
                    break;
                case 9858:
                    npcRole = "Auctioneer";
                    break;
                case 500031:
                    npcRole = "Butler";
                    break;
                case 19052:
                    npcRole = "Mailbox";
                    break;
                case 19871:
                    npcRole = "Guild Vault";
                    break;
                case 28776:
                    npcRole = "Reagent Vendor";
                    break;
                case 28692:
                    npcRole = "General Goods Vendor";
                    break;
                case 2622:
                    npcRole = "Weapon Vendor";
                    break;
                case 29493:
                    npcRole = "Food & Drink Vendor";
                    break;
                case 4255:
                    npcRole = "Leatherworking Supplies";
                    break;
                case 29636:
                    npcRole = "Stable Supplies";
                    break;
                case 32478:
                    npcRole = "Enchanting Supplies";
                    break;
                case 500033:
                    npcRole = "Dual Spec Trainer";
                    break;
                case 500034:
                    npcRole = "Transmog Vendor";
                    break;
                case 500035:
                    npcRole = "Mount Vendor";
                    break;
                case 500036:
                    npcRole = "Heirloom Vendor";
                    break;
                case 500037:
                    npcRole = "Battlemaster";
                    break;
                case 2879:
                    npcRole = "Banker";
                    break;
                case 24547:
                    npcRole = "Tabard Vendor";
                    break;
                case 2878:
                    npcRole = "Guild Tabard Designer";
                    break;
                case 500038:
                    npcRole = "Barber";
                    break;
                case 500039:
                    npcRole = "Void Storage";
                    break;
                // Add more as needed
                default:
                    npcRole = "Vendor";
                    break;
                }

                if (!spawned)
                {
                    notSpawned++;
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Spawn " + npcRole, GOSSIP_SENDER_MAIN, entry, "Spawn " + npcRole + "?", costValue, false);
                }
                else
                {
                    int refundValue = costValue * GuildHouseRefundPercent / 100;
                    totalRefundAll += refundValue;
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Remove " + npcRole + " (Refund " + std::to_string(GuildHouseRefundPercent) + "%)", GOSSIP_SENDER_MAIN, 7000000 + entry, "Remove " + npcRole + " and get a refund?", refundValue, false);
                }
            }
            if (notSpawned > 0)
            {
                int buyAllCost = 0;
                for (auto entry : essentialNpcEntries)
                {
                    if (entry == 6491 || entry == 500031)
                        continue;
                    bool spawned = false;
                    for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                    {
                        Creature *c = pair.second;
                        if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                        {
                            spawned = true;
                            break;
                        }
                    }
                    if (!spawned)
                        buyAllCost += GetGuildHouseEntryCost(entry);
                }
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Buy All Essential NPCs", GOSSIP_SENDER_MAIN, 10021, "Buy and spawn all missing essential NPCs?", buyAllCost, false);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Sell All Essential NPCs", GOSSIP_SENDER_MAIN, 20021, "Remove all essential NPCs and get a refund?", totalRefundAll, false);
            }
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", GOSSIP_SENDER_MAIN, 9);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            break;
        }
        case 10021: // Buy All Essential NPCs
            return this->BuyAllEntries(player, creature, essentialNpcEntries, "creature", 10020);
        case 20021: // Sell All Essential NPCs
            return this->SellAllEntries(player, creature, essentialNpcEntries, "creature", 10020);

        // --- Class Trainers Submenu ---
        case 2:
        {
            ClearGossipMenuFor(player);
            uint32 guildPhase = this->GetGuildPhase(player);
            int notSpawned = 0, totalRefundAll = 0;

            for (auto entry : classTrainerEntries)
            {
                bool spawned = false;
                for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                    {
                        spawned = true;
                        break;
                    }
                }
                int costValue = GetGuildHouseEntryCost(entry);

                std::string trainerName = "Class Trainer";

                if (!spawned)
                {
                    notSpawned++;
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Spawn " + trainerName, GOSSIP_SENDER_MAIN, entry, "Spawn " + trainerName + "?", costValue, false);
                }
                else
                {
                    int refundValue = costValue * GuildHouseRefundPercent / 100;
                    totalRefundAll += refundValue;
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Remove " + trainerName + " (Refund " + std::to_string(GuildHouseRefundPercent) + "%)", GOSSIP_SENDER_MAIN, 7000000 + entry, "Remove the " + trainerName + " and get a refund?", refundValue, false);
                }
            }
            if (notSpawned > 0)
            {
                int buyAllCost = 0;
                for (auto entry : classTrainerEntries)
                {
                    bool spawned = false;
                    for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                    {
                        Creature *c = pair.second;
                        if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                        {
                            spawned = true;
                            break;
                        }
                    }
                    if (!spawned)
                        buyAllCost += GetGuildHouseEntryCost(entry);
                }
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Buy All Class Trainers", GOSSIP_SENDER_MAIN, 10012, "Buy and spawn all missing class trainers?", buyAllCost, false);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Sell All Class Trainers", GOSSIP_SENDER_MAIN, 20012, "Remove all class trainers and get a refund?", totalRefundAll, false);
            }
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", GOSSIP_SENDER_MAIN, 9);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            break;
        }
        case 10012: // Buy All Class Trainers
            return this->BuyAllEntries(player, creature, classTrainerEntries, "creature", 2);
        case 20012: // Sell All Class Trainers
            return this->SellAllEntries(player, creature, classTrainerEntries, "creature", 2);

        // --- Profession Trainers Submenu ---
        case 5:
        {
            ClearGossipMenuFor(player);
            uint32 guildPhase = this->GetGuildPhase(player);
            int notSpawned = 0, totalRefundAll = 0;

            for (auto entry : professionTrainerEntries)
            {
                bool spawned = false;
                for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                    {
                        spawned = true;
                        break;
                    }
                }
                int costValue = GetGuildHouseEntryCost(entry);

                std::string trainerName = "Profession Trainer";

                if (!spawned)
                {
                    notSpawned++;
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Spawn " + trainerName, GOSSIP_SENDER_MAIN, entry, "Spawn " + trainerName + "?", costValue, false);
                }
                else
                {
                    int refundValue = costValue * GuildHouseRefundPercent / 100;
                    totalRefundAll += refundValue;
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Remove " + trainerName + " (Refund " + std::to_string(GuildHouseRefundPercent) + "%)", GOSSIP_SENDER_MAIN, 7000000 + entry, "Remove the " + trainerName + " and get a refund?", refundValue, false);
                }
            }
            if (notSpawned > 0)
            {
                int buyAllCost = 0;
                for (auto entry : professionTrainerEntries)
                {
                    bool spawned = false;
                    for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                    {
                        Creature *c = pair.second;
                        if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                        {
                            spawned = true;
                            break;
                        }
                    }
                    if (!spawned)
                        buyAllCost += GetGuildHouseEntryCost(entry);
                }
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Buy All Prof Trainers", GOSSIP_SENDER_MAIN, 10013, "Buy and spawn all missing prof trainers?", buyAllCost, false);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Sell All Prof Trainers", GOSSIP_SENDER_MAIN, 20013, "Remove all prof trainers and get a refund?", totalRefundAll, false);
            }
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", GOSSIP_SENDER_MAIN, 9);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            break;
        }
        case 10013: // Buy All Prof Trainers
            return this->BuyAllEntries(player, creature, professionTrainerEntries, "creature", 5);
        case 20013: // Sell All Prof Trainers
            return this->SellAllEntries(player, creature, professionTrainerEntries, "creature", 5);

        // --- Vendors Submenu ---
        case 3:
        {
            ClearGossipMenuFor(player);
            uint32 guildPhase = this->GetGuildPhase(player);
            int notSpawned = 0, totalRefundAll = 0;

            for (auto entry : vendorEntries)
            {
                bool spawned = false;
                for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                    {
                        spawned = true;
                        break;
                    }
                }
                int costValue = GetGuildHouseEntryCost(entry);

                std::string vendorName;
                switch (entry)
                {
                case 28776:
                    vendorName = "Reagent Vendor";
                    break;
                case 28692:
                    vendorName = "General Goods Vendor";
                    break;
                case 2622:
                    vendorName = "Weapon Vendor";
                    break;
                case 29493:
                    vendorName = "Food & Drink Vendor";
                    break;
                case 4255:
                    vendorName = "Leatherworking Supplies";
                    break;
                case 29636:
                    vendorName = "Stable Supplies";
                    break;
                case 32478:
                    vendorName = "Enchanting Supplies";
                    break;
                case 500033:
                    vendorName = "Dual Spec Trainer";
                    break;
                case 500034:
                    vendorName = "Transmog Vendor";
                    break;
                case 500035:
                    vendorName = "Mount Vendor";
                    break;
                case 500036:
                    vendorName = "Heirloom Vendor";
                    break;
                case 500037:
                    vendorName = "Battlemaster";
                    break;
                case 2879:
                    vendorName = "Banker";
                    break;
                case 24547:
                    vendorName = "Tabard Vendor";
                    break;
                case 2878:
                    vendorName = "Guild Tabard Designer";
                    break;
                case 500038:
                    vendorName = "Barber";
                    break;
                case 500039:
                    vendorName = "Void Storage";
                    break;
                // Add more as needed
                default:
                    vendorName = "Vendor";
                    break;
                }

                if (!spawned)
                {
                    notSpawned++;
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Spawn " + vendorName, GOSSIP_SENDER_MAIN, entry, "Spawn " + vendorName + "?", costValue, false);
                }
                else
                {
                    int refundValue = costValue * GuildHouseRefundPercent / 100;
                    totalRefundAll += refundValue;
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Remove " + vendorName + " (Refund " + std::to_string(GuildHouseRefundPercent) + "%)", GOSSIP_SENDER_MAIN, 7000000 + entry, "Remove " + vendorName + " and get a refund?", refundValue, false);
                }
            }
            if (notSpawned > 0)
            {
                int buyAllCost = 0;
                for (auto entry : vendorEntries)
                {
                    bool spawned = false;
                    for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                    {
                        Creature *c = pair.second;
                        if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                        {
                            spawned = true;
                            break;
                        }
                    }
                    if (!spawned)
                        buyAllCost += GetGuildHouseEntryCost(entry);
                }
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Buy All Vendors", GOSSIP_SENDER_MAIN, 10003, "Buy and spawn all missing vendors?", buyAllCost, false);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Sell All Vendors", GOSSIP_SENDER_MAIN, 20003, "Remove all vendors and get a refund?", totalRefundAll, false);
            }
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", GOSSIP_SENDER_MAIN, 9);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            break;
        }
        case 10003: // Buy All Vendors
            return this->BuyAllEntries(player, creature, vendorEntries, "creature", 3);
        case 20003: // Sell All Vendors
            return this->SellAllEntries(player, creature, vendorEntries, "creature", 3);

        // --- Portals Submenu ---
        case 10005:
        {
            ClearGossipMenuFor(player);
            uint32 guildPhase = this->GetGuildPhase(player);
            int notSpawned = 0, totalRefundAll = 0;

            for (auto entry : portalEntries)
            {
                bool spawned = false;
                for (auto const &pair : player->GetMap()->GetGameObjectBySpawnIdStore())
                {
                    GameObject *go = pair.second;
                    if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                    {
                        spawned = true;
                        break;
                    }
                }
                int costValue = GetGuildHouseEntryCost(entry);
                std::string objectName = "Object";
                if (auto tmpl = sObjectMgr->GetGameObjectTemplate(entry))
                    objectName = tmpl->name;

                if (!spawned)
                {
                    notSpawned++;
                    std::string portalLabel;
                    switch (entry)
                    {
                    case 500000:
                        portalLabel = "Stormwind Portal";
                        break;
                    case 500001:
                        portalLabel = "Darnassus Portal";
                        break;
                    case 500002:
                        portalLabel = "Exodar Portal";
                        break;
                    case 500003:
                        portalLabel = "Ironforge Portal";
                        break;
                    case 500004:
                        portalLabel = "Orgrimmar Portal";
                        break;
                    case 500005:
                        portalLabel = "Silvermoon Portal";
                        break;
                    case 500006:
                        portalLabel = "Thunder Bluff Portal";
                        break;
                    case 500007:
                        portalLabel = "Undercity Portal";
                        break;
                    case 500008:
                        portalLabel = "Shattrath Portal";
                        break;
                    case 500009:
                        portalLabel = "Dalaran Portal";
                        break;
                    default:
                        portalLabel = "Portal";
                        break;
                    }
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Spawn " + portalLabel, GOSSIP_SENDER_MAIN, entry, "Spawn a Portal?", costValue, false);
                }
                else
                {
                    int refundValue = costValue * GuildHouseRefundPercent / 100;
                    totalRefundAll += refundValue;
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Remove Portal (Refund " + std::to_string(GuildHouseRefundPercent) + "%)", GOSSIP_SENDER_MAIN, 7000000 + entry, "Remove the Portal and get a refund?", refundValue, false);
                }
            }
            if (notSpawned > 0)
            {
                int buyAllCost = 0;
                for (auto entry : portalEntries)
                {
                    bool spawned = false;
                    for (auto const &pair : player->GetMap()->GetGameObjectBySpawnIdStore())
                    {
                        GameObject *go = pair.second;
                        if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                        {
                            spawned = true;
                            break;
                        }
                    }
                    if (!spawned)
                        buyAllCost += GetGuildHouseEntryCost(entry);
                }
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Buy All Portals", GOSSIP_SENDER_MAIN, 11005, "Buy and spawn all missing portals?", buyAllCost, false);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Sell All Portals", GOSSIP_SENDER_MAIN, 20005, "Remove all portals and get a refund?", totalRefundAll, false);
            }
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", GOSSIP_SENDER_MAIN, 9);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            break;
        }
        case 11005: // Buy All Portals
            return this->BuyAllEntries(player, creature, portalEntries, "gameobject", 10005);

        // --- Objects Submenu ---
        case 10006:
        {
            ClearGossipMenuFor(player);
            uint32 guildPhase = this->GetGuildPhase(player);
            int notSpawned = 0, totalRefundAll = 0;

            for (auto entry : objectEntries)
            {
                std::string label;
                switch (entry)
                {
                case 184137:
                    label = "Mailbox";
                    break;
                case 19871:
                    label = "Guild Vault";
                    break;
                case 19052:
                    label = "Guild Bank";
                    break;
                case 500040:
                    label = "Training Dummy";
                    break;
                case 500041:
                    label = "Forge";
                    break;
                case 500042:
                    label = "Anvil";
                    break;
                case 500043:
                    label = "Alchemy Lab";
                    break;
                case 500044:
                    label = "Cooking Fire";
                    break;
                // Add more cases for your custom object entries as needed
                default:
                    label = "Object";
                    break;
                }

                bool spawned = false;
                for (auto const &pair : player->GetMap()->GetGameObjectBySpawnIdStore())
                {
                    GameObject *go = pair.second;
                    if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                    {
                        spawned = true;
                        break;
                    }
                }
                int costValue = GetGuildHouseEntryCost(entry);

                if (!spawned)
                {
                    notSpawned++;
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Spawn " + label, GOSSIP_SENDER_MAIN, entry, "Spawn a " + label + "?", costValue, false);
                }
                else
                {
                    int refundValue = costValue * GuildHouseRefundPercent / 100;
                    totalRefundAll += refundValue;
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Remove " + label + " (Refund " + std::to_string(GuildHouseRefundPercent) + "%)", GOSSIP_SENDER_MAIN, 7000000 + entry, "Remove the " + label + " and get a refund?", refundValue, false);
                }
            }
            if (notSpawned > 0)
            {
                int buyAllCost = 0;
                for (auto entry : objectEntries)
                {
                    bool spawned = false;
                    for (auto const &pair : player->GetMap()->GetGameObjectBySpawnIdStore())
                    {
                        GameObject *go = pair.second;
                        if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                        {
                            spawned = true;
                            break;
                        }
                    }
                    if (!spawned)
                        buyAllCost += GetGuildHouseEntryCost(entry);
                }
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Buy All Objects", GOSSIP_SENDER_MAIN, 10015, "Buy and spawn all missing objects?", buyAllCost, false);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Sell All Objects", GOSSIP_SENDER_MAIN, 20015, "Remove all objects and get a refund?", totalRefundAll, false);
            }
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", GOSSIP_SENDER_MAIN, 9);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            break;
        }
        case 10015: // Buy All Objects
            return this->BuyAllEntries(player, creature, objectEntries, "gameobject", 10006);
        case 20015: // Sell All Objects
            return this->SellAllEntries(player, creature, objectEntries, "gameobject", 10006);

        // --- Return to Previous Location ---
        case 10030:
        {
            auto last = player->CustomData.Get<LastPosition>("lastPos");
            if (last && last->mapId)
            {
                player->TeleportTo(last->mapId, last->x, last->y, last->z, last->ori);
                player->CustomData.Erase("lastPos");
            }
            CloseGossipMenuFor(player);
            break;
        }

        // --- Default: Handle single removals and spawns ---
        default:
            // ...your existing default logic for removals and spawns...
            break;
        }

        return true;
    }

    uint32 GetGuildPhase(Player *player)
    {
        return GuildHousePhaseBase + player->GetGuildId();
    };

    void SpawnObject(uint32 entry, Player *player, Creature *creature, bool force = false)
    {
        Map *map = player ? player->GetMap() : nullptr;
        if (!map)
        {
            if (player && player->GetSession())
                ChatHandler(player->GetSession()).PSendSysMessage("Error: Invalid player or map context.");
            return;
        }

        uint32 guildPhase = this->GetGuildPhase(player);

        if (!force)
        {
            for (auto const &pair : map->GetGameObjectBySpawnIdStore())
            {
                GameObject *go = pair.second;
                if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                {
                    if (player && player->GetSession())
                        ChatHandler(player->GetSession()).PSendSysMessage("This object already exists!");
                    return;
                }
            }
        }

        float posX, posY, posZ, ori;
        QueryResult result = WorldDatabase.Query("SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={}", entry);
        if (!result)
        {
            LOG_ERROR("mod_guildhouse", "SpawnObject: failed to query spawn location for entry %u", entry);
            if (player && player->GetSession())
                ChatHandler(player->GetSession()).PSendSysMessage("No spawn location found for object entry %u.", entry);
            return;
        }
        Field *fields = result->Fetch();
        posX = fields[0].Get<float>();
        posY = fields[1].Get<float>();
        posZ = fields[2].Get<float>();
        ori = fields[3].Get<float>();

        GameObject *newObject = new GameObject();
        uint32 lowguid = map->GenerateLowGuid<HighGuid::GameObject>();

        if (!newObject->Create(lowguid, entry, map, guildPhase, posX, posY, posZ, ori, G3D::Quat(), 0, GO_STATE_READY))
        {
            delete newObject;
            if (player && player->GetSession())
                ChatHandler(player->GetSession()).PSendSysMessage("Failed to create object.");
            return;
        }

        newObject->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), guildPhase);
        uint32 db_guid = newObject->GetSpawnId(); // Use GetSpawnId()
        delete newObject;

        newObject = new GameObject();
        if (!newObject->LoadGameObjectFromDB(db_guid, map, true))
        {
            delete newObject;
            if (player && player->GetSession())
                ChatHandler(player->GetSession()).PSendSysMessage("Failed to load object from DB.");
            return;
        }

        return;
    }

    // Helper for Buy All
    bool BuyAllEntries(Player *player, Creature *creature, const std::set<uint32> &entries, const std::string &type, uint32 submenu)
    {
        uint32 guildPhase = GuildHousePhaseBase + player->GetGuildId();
        int totalCost = 0;
        std::vector<uint32> toSpawn;

        for (auto entry : entries)
        {
            bool spawned = false;
            if (type == "creature")
            {
                for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                    {
                        spawned = true;
                        break;
                    }
                }
            }
            else if (type == "gameobject")
            {
                for (auto const &pair : player->GetMap()->GetGameObjectBySpawnIdStore())
                {
                    GameObject *go = pair.second;
                    if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                    {
                        spawned = true;
                        break;
                    }
                }
            }
            if (!spawned)
            {
                toSpawn.push_back(entry);
                totalCost += GetGuildHouseEntryCost(entry);
            }
        }

        if (totalCost == 0)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("All items are already spawned.");
            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, submenu);
            return false;
        }

        if (!player->HasEnoughMoney(totalCost))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Your guild does not have enough gold in the bank.");
            CloseGossipMenuFor(player);
            return false;
        }

        player->ModifyMoney(-totalCost);

        for (uint32 entry : toSpawn)
        {
            WorldDatabase.Execute("REPLACE INTO `guild_house_purchases` (`guild`, `entry`, `type`) VALUES ({}, {}, '{}')", player->GetGuildId(), entry, type);
            if (type == "creature")
                SpawnNPC(entry, player, true);
            else
                this->SpawnObject(entry, player, creature, true);
        }

        ChatHandler(player->GetSession()).PSendSysMessage("All missing items purchased and spawned for %u gold.", totalCost / 10000);
        this->OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, submenu);
        return true;
    }

    // Helper function
    template <typename Container>
    std::string JoinContainer(const Container &container, const std::string &delimiter)
    {
        std::ostringstream oss;
        auto it = container.begin();
        if (it != container.end())
        {
            oss << *it;
            ++it;
        }
        for (; it != container.end(); ++it)
            oss << delimiter << *it;
        return oss.str();
    }

    // Helper for Sell All
    bool SellAllEntries(Player *player, Creature *creature, const std::set<uint32> &entries, const std::string &type, uint32 submenu)
    {
        Map *map = player ? player->GetMap() : nullptr;
        if (!map)
            return false;

        uint32 guildPhase = GuildHousePhaseBase + player->GetGuildId();
        int totalRefund = 0;
        std::vector<uint32> toRemove;

        if (type == "creature")
        {
            for (auto entry : entries)
            {
                for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                    {
                        toRemove.push_back(entry);
                        totalRefund += GetGuildHouseEntryCost(entry) * GuildHouseRefundPercent / 100;
                    }
                }
            }
        }
        else if (type == "gameobject")
        {
            for (auto entry : entries)
            {
                for (auto const &pair : player->GetMap()->GetGameObjectBySpawnIdStore())
                {
                    GameObject *go = pair.second;
                    if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                    {
                        toRemove.push_back(entry);
                        totalRefund += GetGuildHouseEntryCost(entry) * GuildHouseRefundPercent / 100;
                    }
                }
            }
        }

        if (toRemove.empty())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("No items found to remove.");
            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, submenu);
            return false;
        }

        player->ModifyMoney(totalRefund);

        if (type == "creature")
        {
            QueryResult CreatureResult = WorldDatabase.Query("SELECT lowguid FROM creature_spawn WHERE entry IN ({})", JoinContainer(entries, ","));
            if (CreatureResult)
            {
                do
                {
                    Field *fields = CreatureResult->Fetch();
                    uint32 lowguid = fields[0].Get<int32>();
                    if (CreatureData const *cr_data = sObjectMgr->GetCreatureData(lowguid))
                    {
                        Creature *creature = map->GetCreature(ObjectGuid::Create<HighGuid::Unit>(cr_data->id1, lowguid));
                        if (creature)
                        {
                            creature->CombatStop();
                            creature->RemoveAllAuras();
                            if (creature->IsInWorld())
                                creature->DespawnOrUnsummon();
                            creature->DeleteFromDB();
                            // Do not call delete creature;
                        }
                    }
                } while (CreatureResult->NextRow());
            }
        }
        else if (type == "gameobject")
        {
            for (uint32 entry : toRemove)
            {
                for (auto const &pair : player->GetMap()->GetGameObjectBySpawnIdStore())
                {
                    GameObject *go = pair.second;
                    if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                    {
                        if (go->IsInWorld())
                            go->RemoveFromWorld();
                        go->DeleteFromDB();
                        // Do not call delete go;
                        WorldDatabase.Execute("DELETE FROM `guild_house_purchases` WHERE `guild`={} AND `entry`={} AND `type`='gameobject'", player->GetGuildId(), entry);
                    }
                }
            }
        }

        ChatHandler(player->GetSession()).PSendSysMessage("All items removed and %u gold refunded.", totalRefund / 10000);
        OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, submenu);
        return true;
    }
};

std::vector<uint32> alliancePortals = {
    500000, // Stormwind
    500001, // Darnassus
    500002, // Exodar
    500003  // Ironforge
};
std::vector<uint32> hordePortals = {
    500004, // Orgrimmar
    500005, // Silvermoon
    500006, // Thunder Bluff
    500007  // Undercity
};
std::vector<uint32> neutralPortals = {
    500008, // Shattrath
    500009  // Dalaran
};

// Add this function if not present, or update your existing starter spawn logic:
void SpawnStarterNPCs(Player *player)
{
    uint32 guildPhase = GuildHousePhaseBase + player->GetGuildId();
    Map *map = player->GetMap();

    // Spawn Spirit Healer at its default location if not already present
    bool spiritHealerSpawned = false;
    for (auto const &pair : map->GetCreatureBySpawnIdStore())
    {
        Creature *c = pair.second;
        if (c && c->GetEntry() == 6491 && c->GetPhaseMask() == guildPhase)
        {
            spiritHealerSpawned = true;
            break;
        }
    }
    if (!spiritHealerSpawned)
    {
        WorldDatabase.Execute("REPLACE INTO `guild_house_purchases` (`guild`, `entry`, `type`) VALUES ({}, {}, 'creature')", player->GetGuildId(), 6491);
        SpawnNPC(6491, player, true);
    }
}

void SpawnStarterPortal(Player *player)
{
    if (!player)
        return;
    std::vector<uint32> portalEntries = {500000, 500004}; // Stormwind, Orgrimmar
    Map *map = sMapMgr->FindMap(1, 0);
    if (!map)
        return;

    for (uint32 entry : portalEntries)
    {
        QueryResult result = WorldDatabase.Query("SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={}", entry);
        if (!result)
        {
            LOG_INFO("modules", "GUILDHOUSE: Unable to find data on portal for entry: {}", entry);
            continue;
        }
        Field *fields = result->Fetch();
        float posX = fields[0].Get<float>();
        float posY = fields[1].Get<float>();
        float posZ = fields[2].Get<float>();
        float ori = fields[3].Get<float>();

        const GameObjectTemplate *objectInfo = sObjectMgr->GetGameObjectTemplate(entry);
        if (!objectInfo)
        {
            LOG_INFO("modules", "GUILDHOUSE: objectInfo is NULL!");
            continue;
        }
        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
        {
            LOG_INFO("modules", "GUILDHOUSE: Unable to find displayId??");
            continue;
        }

        ObjectGuid::LowType guidLow = map->GenerateLowGuid<HighGuid::GameObject>();
        GameObject *object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();

        if (!object->Create(guidLow, objectInfo->entry, map, GetGuildPhase(player), posX, posY, posZ, ori, G3D::Quat(), 0, GO_STATE_READY))
        {
            delete object;
            LOG_INFO("modules", "GUILDHOUSE: Unable to create object!!");
            continue;
        }

        object->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), GetGuildPhase(player));
        guidLow = object->GetSpawnId();
        delete object;

        object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
        if (!object->LoadGameObjectFromDB(guidLow, map, true))
        {
            delete object;
            continue;
        }
        sObjectMgr->AddGameobjectToGrid(guidLow, sObjectMgr->GetGameObjectData(guidLow));
    }
    CloseGossipMenuFor(player);
}

void ClearGuildHousePhase(uint32 guildId, uint32 guildPhase, Map *map)
{
    // Remove all creatures in the guild's phase except Spirit Healer
    for (auto const &pair : map->GetCreatureBySpawnIdStore())
    {
        Creature *c = pair.second;
        if (c && c->GetPhaseMask() == guildPhase)
        {
            // If you want to skip Spirit Healer, keep this check:
            if (c->GetEntry() == 6491)
                continue;

            c->RemoveAllAuras();
            if (c->IsInWorld())
                c->DespawnOrUnsummon();
            c->DeleteFromDB();
        }
    }

    // Remove all gameobjects in the guild's phase
    for (auto const &pair : map->GetGameObjectBySpawnIdStore())
    {
        GameObject *go = pair.second;
        if (go && go->GetPhaseMask() == guildPhase)
        {
            if (go->IsInWorld())
                go->RemoveFromWorld();
            if (sObjectMgr->GetGameObjectData(go->GetSpawnId()))
                go->DeleteFromDB();
        }
    }

    // Do NOT remove purchase records here; Reset will respawn from purchase history
    // Do NOT remove the guild house record here
}

void SpawnNPC(uint32 entry, Player *player, bool force)
{
    Map *map = player ? player->GetMap() : nullptr;
    if (!map)
    {
        if (player && player->GetSession())
            ChatHandler(player->GetSession()).PSendSysMessage("Error: Invalid player or map context.");
        return;
    }

    uint32 guildPhase = GuildHousePhaseBase + player->GetGuildId();

    // If not force, skip if already exists
    if (!force)
    {
        for (auto const &pair : map->GetCreatureBySpawnIdStore())
        {
            Creature *c = pair.second;
            if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
            {
                if (player && player->GetSession())
                    ChatHandler(player->GetSession()).PSendSysMessage("This NPC already exists!");
                CloseGossipMenuFor(player);
                return;
            }
        }
    }
    else
    {
        // If force, remove any existing NPC of this entry in this phase
        std::vector<Creature *> toRemove;
        for (auto const &pair : map->GetCreatureBySpawnIdStore())
        {
            Creature *c = pair.second;
            if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                toRemove.push_back(c);
        }
        for (Creature *c : toRemove)
        {
            c->RemoveAllAuras();
            if (c->IsInWorld())
                c->DespawnOrUnsummon();
            c->DeleteFromDB();
        }
    }

    float posX, posY, posZ, ori;
    QueryResult result = WorldDatabase.Query("SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={}", entry);
    if (!result)
    {
        LOG_ERROR("mod_guildhouse", "SpawnNPC: failed to query spawn location for entry %u", entry);
        if (player && player->GetSession())
            ChatHandler(player->GetSession()).PSendSysMessage("No spawn location found for NPC entry %u.", entry);
        CloseGossipMenuFor(player);
        return;
    }
    Field *fields = result->Fetch();
    posX = fields[0].Get<float>();
    posY = fields[1].Get<float>();
    posZ = fields[2].Get<float>();
    ori = fields[3].Get<float>();

    // Before teleporting the player to the guild house:
    auto last = new LastPosition();
    last->mapId = player->GetMapId();
    last->x = player->GetPositionX();
    last->y = player->GetPositionY();
    last->z = player->GetPositionZ();
    last->ori = player->GetOrientation();
    player->CustomData.Set("lastPos", last);

    Creature *newCreature = new Creature();
    uint32 lowguid = map->GenerateLowGuid<HighGuid::Unit>();

    if (!newCreature->Create(lowguid, map, guildPhase, entry, 0, posX, posY, posZ, ori))
    {
        delete newCreature;
        if (player && player->GetSession())
            ChatHandler(player->GetSession()).PSendSysMessage("Failed to create creature object.");
        return;
    }

    newCreature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), guildPhase);
    uint32 db_guid = newCreature->GetSpawnId();
    delete newCreature;

    newCreature = new Creature();
    if (!newCreature->LoadFromDB(db_guid, map, true))
    {
        delete newCreature;
        if (player && player->GetSession())
            ChatHandler(player->GetSession()).PSendSysMessage("Failed to load creature from DB.");
        return;
    }

    if (!map->AddToMap(newCreature))
    {
        delete newCreature;
        if (player && player->GetSession())
            ChatHandler(player->GetSession()).PSendSysMessage("Failed to add creature to map.");
        return;
    }
}

class GuildHouseButlerConf : public WorldScript
{
public:
    GuildHouseButlerConf() : WorldScript("GuildHouseButlerConf") {}

    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        GuildHouseInnKeeper = sConfigMgr->GetOption<int32>("GuildHouseInnKeeper", 1000000);
        GuildHouseBank = sConfigMgr->GetOption<int32>("GuildHouseBank", 1000000);
        GuildHouseMailBox = sConfigMgr->GetOption<int32>("GuildHouseMailbox", 500000);
        GuildHouseAuctioneer = sConfigMgr->GetOption<int32>("GuildHouseAuctioneer", 500000);
        GuildHouseTrainer = sConfigMgr->GetOption<int32>("GuildHouseTrainerCost", 1000000);
        GuildHouseVendor = sConfigMgr->GetOption<int32>("GuildHouseVendor", 500000);
        GuildHouseObject = sConfigMgr->GetOption<int32>("GuildHouseObject", 500000);
        GuildHousePortal = sConfigMgr->GetOption<int32>("GuildHousePortal", 500000);
        GuildHouseProf = sConfigMgr->GetOption<int32>("GuildHouseProf", 500000);
        GuildHouseSpirit = sConfigMgr->GetOption<int32>("GuildHouseSpirit", 100000);
        GuildHouseBuyRank = sConfigMgr->GetOption<int32>("GuildHouseBuyRank", 4);
        GuildHouseRefundPercent = sConfigMgr->GetOption<int32>("GuildHouseRefundPercent", 50);

        GuildHouseMarkerEntry = sConfigMgr->GetOption<uint32>("GuildHouseMarkerEntry", 999999);
        GuildHousePhaseBase = sConfigMgr->GetOption<uint32>("GuildHousePhaseBase", 10000);

        WorldDatabase.Execute("UPDATE guild_house_spawns SET posX=16236.0, posY=16264.1 WHERE entry=500004;");
    }

    void OnStartup() override
    {
        LOG_INFO("modules", "Guild House Module is enabled. Guild houses are available on this server!");
    }
};

void AddGuildHouseButlerScripts()
{
    new GuildHouseSpawner();
    new GuildHouseButlerConf();
}
