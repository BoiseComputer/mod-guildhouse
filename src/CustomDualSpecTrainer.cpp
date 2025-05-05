#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "GossipDef.h"
#include "ScriptedGossip.h"
#include "Configuration/Config.h"
#include "Log.h"

bool HasDualSpec(Player *player)
{
    bool hasAlliance = player->HasSpell(63680);
    bool hasHorde = player->HasSpell(63624);
    LOG_INFO("module.guildhouse", "Player [{}] HasSpell(63680): {}, HasSpell(63624): {}", player->GetName(), hasAlliance, hasHorde);
    return hasAlliance || hasHorde;
}

class npc_dual_spec_trainer : public CreatureScript
{
public:
    npc_dual_spec_trainer() : CreatureScript("npc_dual_spec_trainer") {}

    bool OnGossipHello(Player *player, Creature *creature) override
    {
        LOG_INFO("module.guildhouse", "OnGossipHello: Player [{}] level [{}]", player->GetName(), player->GetLevel());
        if (HasDualSpec(player))
        {
            LOG_INFO("module.guildhouse", "Player [{}] already has dual spec.", player->GetName());
            server shutdown 1
                // Show a message in the gossip window instead of just a system message
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "You already have dual specialization.", GOSSIP_SENDER_MAIN, 0);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            return true;
        }

        if (player->GetLevel() >= 40)
        {
            int32 dualSpecCost = sConfigMgr->GetOption<int32>("GuildHouseDualSpecTrainer", 1000000);
            LOG_INFO("module.guildhouse", "Player [{}] can buy dual spec for {} copper.", player->GetName(), dualSpecCost);
            std::ostringstream oss;
            oss << "Learn Dual Specialization (" << (dualSpecCost / 10000) << "g)";
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, oss.str(), GOSSIP_SENDER_MAIN, 1);
        }

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == 1)
        {
            LOG_INFO("module.guildhouse", "OnGossipSelect: Player [{}] selected dual spec option.", player->GetName());
            if (HasDualSpec(player))
            {
                LOG_INFO("module.guildhouse", "Player [{}] already has dual spec (select).", player->GetName());
                ChatHandler(player->GetSession()).PSendSysMessage("You already have dual specialization.");
            }
            else
            {
                int32 dualSpecCost = sConfigMgr->GetOption<int32>("GuildHouseDualSpecTrainer", 1000000);
                LOG_INFO("module.guildhouse", "Player [{}] money: {}, cost: {}", player->GetName(), player->GetMoney(), dualSpecCost);
                if (player->GetMoney() < dualSpecCost)
                {
                    LOG_INFO("module.guildhouse", "Player [{}] does not have enough gold.", player->GetName());
                    ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough gold.");
                }
                else
                {
                    player->ModifyMoney(-dualSpecCost);
                    player->learnSpell(63680, false);
                    player->learnSpell(63624, false);
                    LOG_INFO("module.guildhouse", "Player [{}] granted dual spec.", player->GetName());
                    ChatHandler(player->GetSession()).PSendSysMessage("You have learned dual specialization!");
                }
            }
            player->PlayerTalkClass->SendCloseGossip();
        }
        return true;
    }
};

void AddSC_npc_dual_spec_trainer()
{
    new npc_dual_spec_trainer();
}