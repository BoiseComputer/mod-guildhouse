#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "GossipDef.h"
#include "ScriptedGossip.h"

class npc_dual_spec_trainer : public CreatureScript
{
public:
    npc_dual_spec_trainer() : CreatureScript("npc_dual_spec_trainer") {}

    bool OnGossipHello(Player *player, Creature *creature) override
    {
        if (player->GetLevel() >= 40 && !player->HasSpell(63680) && !player->HasSpell(63624))
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Learn Dual Specialization (1000g)", GOSSIP_SENDER_MAIN, 1);

        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == 1)
        {
            if (player->GetMoney() < 100000000)
            {
                ChatHandler(player->GetSession()).PSendSysMessage("You do not have enough gold.");
            }
            else if (player->HasSpell(63680) || player->HasSpell(63624))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("You already have dual specialization.");
            }
            else
            {
                player->ModifyMoney(-100000000);
                player->CastSpell(player, 63680, true); // Learn dual spec (Alliance)
                player->CastSpell(player, 63624, true); // Learn dual spec (Horde)
                ChatHandler(player->GetSession()).PSendSysMessage("You have learned dual specialization!");
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