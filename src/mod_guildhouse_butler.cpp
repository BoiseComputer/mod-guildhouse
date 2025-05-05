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
        GuildHouseSpawnerAI(Creature *creature) : ScriptedAI(creature)
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
        // Always ensure the flag is set
        creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        if (!player->GetGuild())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("You are not in a guild!");
            CloseGossipMenuFor(player);
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
        if (!guild)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Guild not found.");
            CloseGossipMenuFor(player);
            return false;
        }

        Guild::Member const *memberMe = guild->GetMember(player->GetGUID());
        if (!memberMe || !memberMe->IsRankNotLower(GuildHouseBuyRank))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("You are not authorized to make Guild House purchases.");
            CloseGossipMenuFor(player);
            return false;
        }

        // Clear and add gossip menu
        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Manage Essential NPCs", GOSSIP_SENDER_MAIN, 10020);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Spawn Class Trainers", GOSSIP_SENDER_MAIN, 2);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Spawn Profession Trainers", GOSSIP_SENDER_MAIN, 5);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Spawn Vendors", GOSSIP_SENDER_MAIN, 3);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Spawn City Portals", GOSSIP_SENDER_MAIN, 10005);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Spawn Objects", GOSSIP_SENDER_MAIN, 10006);
        AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Reset Guild House (Remove & Re-place All)", GOSSIP_SENDER_MAIN, 900000, "Are you sure you want to reset your Guild House? This will remove and re-place all purchased objects and NPCs.", 0, false);

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
                case 28047:
                    npcRole = "Stable Master";
                    break;
                case 8661:
                    npcRole = "Auctioneer Beardo";
                    break;
                case 6491:
                    npcRole = "Spirit Healer";
                    break;
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

                std::string trainerName;
                switch (entry)
                {
                case 26327:
                    trainerName = "Paladin Trainer";
                    break;
                case 26324:
                    trainerName = "Druid Trainer";
                    break;
                case 26325:
                    trainerName = "Hunter Trainer";
                    break;
                case 26326:
                    trainerName = "Mage Trainer";
                    break;
                case 26328:
                    trainerName = "Priest Trainer";
                    break;
                case 26329:
                    trainerName = "Rogue Trainer";
                    break;
                case 26330:
                    trainerName = "Shaman Trainer";
                    break;
                case 26331:
                    trainerName = "Warlock Trainer";
                    break;
                case 26332:
                    trainerName = "Warrior Trainer";
                    break;
                case 29195:
                    trainerName = "Death Knight Trainer";
                    break;
                default:
                    trainerName = "Class Trainer";
                    break;
                }

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

                std::string trainerName;

                switch (entry)
                {
                case 28703:
                    trainerName = "Alchemy Trainer";
                    break;
                case 28694:
                    trainerName = "Blacksmithing Trainer";
                    break;
                case 28693:
                    trainerName = "Enchanting Trainer";
                    break;
                case 29513:
                    trainerName = "Engineering Trainer";
                    break;
                case 28702:
                    trainerName = "Inscription Trainer";
                    break;
                case 28701:
                    trainerName = "Jewelcrafting Trainer";
                    break;
                case 28700:
                    trainerName = "Leatherworking Trainer";
                    break;
                case 28704:
                    trainerName = "Herbalism Trainer";
                    break;
                case 28697:
                    trainerName = "Mining Trainer";
                    break;
                case 28699:
                    trainerName = "Tailoring Trainer";
                    break;
                case 19185:
                    trainerName = "First Aid Trainer";
                    break;
                case 600001:
                    trainerName = "Cooking Trainer";
                    break;
                case 28742:
                    trainerName = "Fishing Trainer";
                    break;
                default:
                    trainerName = "Profession Trainer";
                    break;
                }

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
                // Check if already spawned in the guild house phase
                for (auto const &pair : player->GetMap()->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                    {
                        spawned = true;
                        break;
                    }
                }

                // Hide purchase option if already purchased (for dual spec trainer)
                if (entry == 500033 && spawned)
                {
                    AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Dual Spec Trainer (Already Purchased)", GOSSIP_SENDER_MAIN, 0, "", 0, true);
                    continue;
                }

                int costValue = GetGuildHouseEntryCost(entry);

                std::string vendorName;
                switch (entry)
                {
                case 28047:
                    vendorName = "Stable Master";
                    break;
                case 6491:
                    vendorName = "Spirit Healer";
                    break;
                case 28692:
                    vendorName = "General Goods Vendor";
                    break;
                case 28776:
                    vendorName = "Reagent Vendor";
                    break;
                case 4255:
                    vendorName = "Leatherworking Supplies";
                    break;
                case 29636:
                    vendorName = "Stable Supplies";
                    break;
                case 29493:
                    vendorName = "Specialty Ammunition Vendor";
                    break;
                case 2622:
                    vendorName = "Weapon Vendor";
                    break;
                case 32478:
                    vendorName = "Food & Drink Vendor";
                    break;
                case 500033:
                    vendorName = "Dual Spec Trainer";
                    break;
                case 500034:
                    vendorName = "Transmogrifier";
                    break;
                case 5000351:
                    vendorName = "Mount Vendor: Hoofed (Rams, Talbuks, Elekks, Kodos)";
                    break;
                case 5000352:
                    vendorName = "Mount Vendor: Steeds (Horses, Skeletal, Mammoths)";
                    break;
                case 5000353:
                    vendorName = "Mount Vendor: Wild (Wolves, Bears, Tigers, Sabers)";
                    break;
                case 5000354:
                    vendorName = "Mount Vendor: Raptors & Drakes";
                    break;
                case 5000355:
                    vendorName = "Mount Vendor: Exotics (Hawkstriders, Mechanostriders, etc)";
                    break;
                case 5000356:
                    vendorName = "Mount Vendor: Special (Qiraji, Carpets, Brooms, etc)";
                    break;
                case 500036:
                    vendorName = "Heirloom Vendor";
                    break;
                case 29533:
                    vendorName = "Battlemaster";
                    break;
                // Add any other vendors you use, with clear names
                default:
                {
                    std::ostringstream oss;
                    oss << "Unknown Vendor (Entry " << entry << ")";
                    vendorName = oss.str();
                    break;
                }
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
                // Skip starter portals
                if (entry == 500000 || entry == 500004)
                    continue;

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
                std::string objectName;
                switch (entry)
                {
                case 500000:
                    objectName = "Stormwind Portal";
                    break;
                case 500001:
                    objectName = "Exodar Portal";
                    break;
                case 500002:
                    objectName = "Darnassus Portal";
                    break;
                case 500003:
                    objectName = "Ironforge Portal";
                    break;
                case 500004:
                    objectName = "Orgrimmar Portal";
                    break;
                case 500005:
                    objectName = "Silvermoon Portal";
                    break;
                case 500006:
                    objectName = "Thunder Bluff Portal";
                    break;
                case 500007:
                    objectName = "Undercity Portal";
                    break;
                case 500008:
                    objectName = "Shattrath Portal";
                    break;
                case 500009:
                    objectName = "Dalaran Portal";
                    break;
                default:
                    objectName = "Unknown Portal";
                    break;
                }

                if (!spawned)
                {
                    notSpawned++;
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Spawn " + objectName, GOSSIP_SENDER_MAIN, entry, "Spawn a Portal?", costValue, false);
                }
                else
                {
                    int refundValue = costValue * GuildHouseRefundPercent / 100;
                    totalRefundAll += refundValue;
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Remove " + objectName + " Portal (Refund " + std::to_string(GuildHouseRefundPercent) + "%)", GOSSIP_SENDER_MAIN, 7000000 + entry, "Remove the Portal and get a refund?", refundValue, false);
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
                std::set<uint32> sellablePortals;
                for (auto entry : portalEntries)
                {
                    if (entry != 500000 && entry != 500004)
                        sellablePortals.insert(entry);
                }
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Sell All Portals", GOSSIP_SENDER_MAIN, 20005, "Remove all portals and get a refund?", totalRefundAll, false);
            }
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Back", GOSSIP_SENDER_MAIN, 9);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            break;
        }
        case 11005: // Buy All Portals
            return this->BuyAllEntries(player, creature, portalEntries, "gameobject", 10005);
        case 20005: // Sell All Portals
        {
            std::set<uint32> sellablePortals;
            for (auto entry : portalEntries)
            {
                if (entry != 500000 && entry != 500004)
                    sellablePortals.insert(entry);
            }
            return this->SellAllEntries(player, creature, sellablePortals, "gameobject", 10005);
        }

        // --- Objects Submenu ---
        case 10006:
        {
            ClearGossipMenuFor(player);
            uint32 guildPhase = this->GetGuildPhase(player);
            int notSpawned = 0, totalRefundAll = 0;

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

                int costValue = GetGuildHouseEntryCost(entry);
                std::string label;
                switch (entry)
                {
                case 184137:
                    label = "Mailbox";
                    break;
                case 1685:
                    label = "Guild Vault";
                    break;
                case 4087:
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
                default:
                    label = "Object";
                    break;
                }

                if (!spawned)
                {
                    notSpawned++;
                    AddGossipItemFor(player, GOSSIP_ICON_BATTLE, "Spawn " + label, GOSSIP_SENDER_MAIN, entry, "Spawn " + label + "?", costValue, false);
                }
                else
                {
                    int refundValue = costValue * GuildHouseRefundPercent / 100;
                    totalRefundAll += refundValue;
                    AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Remove " + label + " (Refund " + std::to_string(GuildHouseRefundPercent) + "%)", GOSSIP_SENDER_MAIN, 7000000 + entry, "Remove " + label + " and get a refund?", refundValue, false);
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
        {
            // Prevent buying/selling starter portals
            if ((portalEntries.count(action) && (action == 500000 || action == 500004)) ||
                (action >= 7000000 && portalEntries.count(action - 7000000) && ((action - 7000000) == 500000 || (action - 7000000) == 500004)))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("You cannot buy or remove the starter portals.");
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 10005);
                return true;
            }

            // Handle essential NPC spawn
            if (essentialNpcEntries.count(action))
            {
                int costValue = GetGuildHouseEntryCost(action);
                if (!player->HasEnoughMoney(costValue))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough gold.");
                    CloseGossipMenuFor(player);
                    return false;
                }
                player->ModifyMoney(-costValue);
                WorldDatabase.Execute("REPLACE INTO `guild_house_purchases` (`guild`, `entry`, `type`) VALUES ({}, {}, 'creature')", player->GetGuildId(), action);
                SpawnNPC(action, player, true);
                ChatHandler(player->GetSession()).PSendSysMessage("NPC spawned.");
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 10020);
                return true;
            }

            if (action >= 7000000 && essentialNpcEntries.count(action - 7000000))
            {
                uint32 entry = action - 7000000;
                Map *map = player->GetMap();
                uint32 guildPhase = this->GetGuildPhase(player);
                std::vector<Creature *> toDelete;
                for (auto const &pair : map->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                        toDelete.push_back(c);
                }
                for (auto c : toDelete)
                {
                    c->CombatStop();
                    c->RemoveAllAuras();
                    if (c->IsInWorld())
                        c->DespawnOrUnsummon();
                    c->DeleteFromDB();
                }
                WorldDatabase.Execute("DELETE FROM `guild_house_purchases` WHERE `guild`={} AND `entry`={} AND `type`='creature'", player->GetGuildId(), entry);
                int refundValue = GetGuildHouseEntryCost(entry) * GuildHouseRefundPercent / 100;
                player->ModifyMoney(refundValue);
                ChatHandler(player->GetSession()).PSendSysMessage("NPC removed and {} gold refunded.", refundValue / 10000);
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 10020);
                return true;
            }

            // Handle class trainers
            if (classTrainerEntries.count(action))
            {
                int costValue = GetGuildHouseEntryCost(action);
                if (!player->HasEnoughMoney(costValue))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough gold.");
                    CloseGossipMenuFor(player);
                    return false;
                }
                player->ModifyMoney(-costValue);
                WorldDatabase.Execute("REPLACE INTO `guild_house_purchases` (`guild`, `entry`, `type`) VALUES ({}, {}, 'creature')", player->GetGuildId(), action);
                SpawnNPC(action, player, true);
                ChatHandler(player->GetSession()).PSendSysMessage("Class trainer spawned.");
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 2);
                return true;
            }

            if (action >= 7000000 && classTrainerEntries.count(action - 7000000))
            {
                uint32 entry = action - 7000000;
                Map *map = player->GetMap();
                uint32 guildPhase = this->GetGuildPhase(player);
                std::vector<Creature *> toDelete;
                for (auto const &pair : map->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                        toDelete.push_back(c);
                }
                for (auto c : toDelete)
                {
                    c->CombatStop();
                    c->RemoveAllAuras();
                    if (c->IsInWorld())
                        c->DespawnOrUnsummon();
                    c->DeleteFromDB();
                }
                WorldDatabase.Execute("DELETE FROM `guild_house_purchases` WHERE `guild`={} AND `entry`={} AND `type`='creature'", player->GetGuildId(), entry);
                int refundValue = GetGuildHouseEntryCost(entry) * GuildHouseRefundPercent / 100;
                player->ModifyMoney(refundValue);
                ChatHandler(player->GetSession()).PSendSysMessage("Class trainer removed and {} gold refunded.", refundValue / 10000);
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 2);
                return true;
            }

            // Handle object spawn (entry in objectEntries)
            if (objectEntries.count(action))
            {
                int costValue = GetGuildHouseEntryCost(action);
                if (!player->HasEnoughMoney(costValue))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough gold.");
                    CloseGossipMenuFor(player);
                    return false;
                }
                player->ModifyMoney(-costValue);
                WorldDatabase.Execute("REPLACE INTO `guild_house_purchases` (`guild`, `entry`, `type`) VALUES ({}, {}, 'gameobject')", player->GetGuildId(), action);
                this->SpawnObject(action, player, creature, true);
                ChatHandler(player->GetSession()).PSendSysMessage("Object spawned.");
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 10006);
                return true;
            }

            // Handle object removal (7000000 + entry in objectEntries)
            if (action >= 7000000 && objectEntries.count(action - 7000000))
            {
                uint32 entry = action - 7000000;
                Map *map = player->GetMap();
                uint32 guildPhase = this->GetGuildPhase(player);
                std::vector<GameObject *> toDelete;
                for (auto const &pair : map->GetGameObjectBySpawnIdStore())
                {
                    GameObject *go = pair.second;
                    if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                        toDelete.push_back(go);
                }
                for (auto go : toDelete)
                {
                    if (go->IsInWorld())
                        go->RemoveFromWorld();
                    go->DeleteFromDB();
                    go->CleanupsBeforeDelete();
                }
                WorldDatabase.Execute("DELETE FROM `guild_house_purchases` WHERE `guild`={} AND `entry`={} AND `type`='gameobject'", player->GetGuildId(), entry);
                int refundValue = GetGuildHouseEntryCost(entry) * GuildHouseRefundPercent / 100;
                player->ModifyMoney(refundValue);
                ChatHandler(player->GetSession()).PSendSysMessage("Object removed and {} gold refunded.", refundValue / 10000);
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 10006);
                return true;
            }

            // Handle vendor spawn
            if (vendorEntries.count(action))
            {
                int costValue = GetGuildHouseEntryCost(action);
                if (!player->HasEnoughMoney(costValue))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough gold.");
                    CloseGossipMenuFor(player);
                    return false;
                }
                player->ModifyMoney(-costValue);
                WorldDatabase.Execute("REPLACE INTO `guild_house_purchases` (`guild`, `entry`, `type`) VALUES ({}, {}, 'creature')", player->GetGuildId(), action);
                SpawnNPC(action, player, true);
                ChatHandler(player->GetSession()).PSendSysMessage("Vendor spawned.");
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 3);
                return true;
            }
            // Handle vendor removal
            if (action >= 7000000 && vendorEntries.count(action - 7000000))
            {
                uint32 entry = action - 7000000;
                Map *map = player->GetMap();
                uint32 guildPhase = this->GetGuildPhase(player);
                std::vector<Creature *> toDelete;
                for (auto const &pair : map->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                        toDelete.push_back(c);
                }
                for (auto c : toDelete)
                {
                    c->CombatStop();
                    c->RemoveAllAuras();
                    if (c->IsInWorld())
                        c->DespawnOrUnsummon();
                    c->DeleteFromDB();
                }
                WorldDatabase.Execute("DELETE FROM `guild_house_purchases` WHERE `guild`={} AND `entry`={} AND `type`='creature'", player->GetGuildId(), entry);
                int refundValue = GetGuildHouseEntryCost(entry) * GuildHouseRefundPercent / 100;
                player->ModifyMoney(refundValue);
                ChatHandler(player->GetSession()).PSendSysMessage("Vendor removed and {} gold refunded.", refundValue / 10000);
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 3);
                return true;
            }
            // Handle portal spawn
            if (portalEntries.count(action))
            {
                int costValue = GetGuildHouseEntryCost(action);
                if (!player->HasEnoughMoney(costValue))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough gold.");
                    CloseGossipMenuFor(player);
                    return false;
                }
                player->ModifyMoney(-costValue);
                WorldDatabase.Execute("REPLACE INTO `guild_house_purchases` (`guild`, `entry`, `type`) VALUES ({}, {}, 'gameobject')", player->GetGuildId(), action);
                this->SpawnObject(action, player, creature, true);
                ChatHandler(player->GetSession()).PSendSysMessage("Portal spawned.");
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 10005);
                return true;
            }
            // Handle portal removal
            if (action >= 7000000 && portalEntries.count(action - 7000000))
            {
                uint32 entry = action - 7000000;
                Map *map = player->GetMap();
                uint32 guildPhase = this->GetGuildPhase(player);
                std::vector<GameObject *> toDelete;
                for (auto const &pair : map->GetGameObjectBySpawnIdStore())
                {
                    GameObject *go = pair.second;
                    if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                        toDelete.push_back(go);
                }
                for (auto go : toDelete)
                {
                    if (go->IsInWorld())
                        go->RemoveFromWorld();
                    go->DeleteFromDB();
                    go->CleanupsBeforeDelete();
                }
                WorldDatabase.Execute("DELETE FROM `guild_house_purchases` WHERE `guild`={} AND `entry`={} AND `type`='gameobject'", player->GetGuildId(), entry);
                int refundValue = GetGuildHouseEntryCost(entry) * GuildHouseRefundPercent / 100;
                player->ModifyMoney(refundValue);
                ChatHandler(player->GetSession()).PSendSysMessage("Portal removed and {} gold refunded.", refundValue / 10000);
                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, 10005);
                return true;
            }

            break;
        }
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
            LOG_ERROR("mod_guildhouse", "SpawnObject: failed to query spawn location for entry {}", entry);
            if (player && player->GetSession())
                ChatHandler(player->GetSession()).PSendSysMessage("No spawn location found for object entry {}.", entry);
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

        ChatHandler(player->GetSession()).PSendSysMessage("All missing items purchased and spawned for  gold.", totalCost / 10000);
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

        // Find which entries are actually spawned in the guild house phase
        for (auto entry : entries)
        {
            bool found = false;
            if (type == "creature")
            {
                for (auto const &pair : map->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                    {
                        found = true;
                        break;
                    }
                }
            }
            else if (type == "gameobject")
            {
                for (auto const &pair : map->GetGameObjectBySpawnIdStore())
                {
                    GameObject *go = pair.second;
                    if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                    {
                        found = true;
                        break;
                    }
                }
            }
            if (found)
            {
                toRemove.push_back(entry);
                totalRefund += GetGuildHouseEntryCost(entry) * GuildHouseRefundPercent / 100;
            }
        }

        if (toRemove.empty())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("No items found to remove.");
            ClearGossipMenuFor(player);
            OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, submenu);
            return false;
        }

        player->ModifyMoney(totalRefund);

        if (type == "creature")
        {
            for (auto entry : toRemove)
            {
                for (auto const &pair : map->GetCreatureBySpawnIdStore())
                {
                    Creature *c = pair.second;
                    if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                    {
                        c->CombatStop();
                        c->RemoveAllAuras();
                        if (c->IsInWorld())
                            c->DespawnOrUnsummon();
                        c->DeleteFromDB();
                        WorldDatabase.Execute("DELETE FROM `guild_house_purchases` WHERE `guild`={} AND `entry`={} AND `type`='creature'", player->GetGuildId(), entry);
                    }
                }
            }
        }
        else if (type == "gameobject")
        {
            for (auto entry : toRemove)
            {
                std::vector<GameObject *> toDelete;
                for (auto const &pair : map->GetGameObjectBySpawnIdStore())
                {
                    GameObject *go = pair.second;
                    if (go && go->GetEntry() == entry && go->GetPhaseMask() == guildPhase)
                    {
                        toDelete.push_back(go);
                    }
                }
                for (auto go : toDelete)
                {
                    if (go->IsInWorld())
                        go->RemoveFromWorld();
                    go->DeleteFromDB();
                    go->CleanupsBeforeDelete();
                    WorldDatabase.Execute("DELETE FROM `guild_house_purchases` WHERE `guild`={} AND `entry`={} AND `type`='gameobject'", player->GetGuildId(), entry);
                }
            }
        }

        ChatHandler(player->GetSession()).PSendSysMessage("All items removed and {} gold refunded.", totalRefund / 10000);
        ClearGossipMenuFor(player);
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
    }

    void OnStartup() override
    {
        LOG_INFO("modules", "Guild House Module is enabled. Guild houses are available on this server!");
    }
};

void SpawnNPC(uint32 entry, Player *player, bool force)
{
    if (!player)
        return;

    Map *map = player->GetMap();
    if (!map)
        return;

    uint32 guildPhase = GuildHousePhaseBase + player->GetGuildId();

    // Prevent duplicate spawns unless forced
    if (!force)
    {
        for (auto const &pair : map->GetCreatureBySpawnIdStore())
        {
            Creature *c = pair.second;
            if (c && c->GetEntry() == entry && c->GetPhaseMask() == guildPhase)
                return;
        }
    }

    // Get spawn location from DB
    QueryResult result = WorldDatabase.Query("SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={}", entry);
    if (!result)
        return;
    Field *fields = result->Fetch();
    float posX = fields[0].Get<float>();
    float posY = fields[1].Get<float>();
    float posZ = fields[2].Get<float>();
    float ori = fields[3].Get<float>();

    Creature *creature = new Creature();
    if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, guildPhase, entry, 0, posX, posY, posZ, ori))
    {
        delete creature;
        return;
    }
    creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), guildPhase);
    uint32 lowguid = creature->GetSpawnId();

    creature->CleanupsBeforeDelete();
    delete creature;
    creature = new Creature();
    if (!creature->LoadCreatureFromDB(lowguid, map))
    {
        delete creature;
        return;
    }
    sObjectMgr->AddCreatureToGrid(lowguid, sObjectMgr->GetCreatureData(lowguid));
};

extern void AddGuildHouseScripts();
extern void AddSC_npc_dual_spec_trainer();

void Addmod_guildhouseScripts()
{
    AddGuildHouseScripts();        // Adds the guild house scripts
    new GuildHouseSpawner();       // Loads the NPCs
    new GuildHouseButlerConf();    // Loads the configuration
    AddSC_npc_dual_spec_trainer(); // Adds the dual spec trainer
}
