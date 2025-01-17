#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>

#include "scheduler.hpp"
#include "game/game.hpp"

namespace loot
{
	namespace
	{
		game::dvar_t* dvar_cg_unlockall_loot;
		game::dvar_t* dvar_cg_unlockall_purchases;
		game::dvar_t* dvar_cg_unlockall_attachments;
		game::dvar_t* dvar_cg_cg_unlockall_camos;

		utils::hook::detour loot_getitemquantity_hook;
		utils::hook::detour liveinventory_getitemquantity_hook;
		utils::hook::detour liveinventory_areextraslotspurchased_hook;
		utils::hook::detour bg_unlockablesisitempurchased_hook;
		utils::hook::detour bg_unlockablesisitemattachmentlocked_hook;
		utils::hook::detour bg_unlockablesisattachmentslotlocked_hook;

		int loot_getitemquantity_stub(const game::ControllerIndex_t controller_index, const game::eModes mode,
		                              const int item_id)
		{
			if (!dvar_cg_unlockall_loot->current.value.enabled)
			{
				return loot_getitemquantity_hook.invoke<int>(controller_index, mode, item_id);
			}

			if (mode == game::eModes::MODE_ZOMBIES)
			{
				return 999;
			}

			return 1;
		}

		int liveinventory_getitemquantity_stub(const game::ControllerIndex_t controller_index, const int item_id)
		{
			// Item id's for extra CaC slots, CWL camo's and paid specialist outfits
			if (dvar_cg_unlockall_loot->current.value.enabled && (item_id == 99003 || item_id >= 99018 && item_id <= 99021 || item_id == 99025||
				item_id >= 90047 && item_id <= 90064))
			{
				return 1;
			}

			return liveinventory_getitemquantity_hook.invoke<int>(controller_index, item_id);
		}

		bool liveinventory_areextraslotspurchased_stub(const game::ControllerIndex_t controller_index)
		{
			if (dvar_cg_unlockall_loot->current.value.enabled)
			{
				return true;
			}

			return liveinventory_areextraslotspurchased_hook.invoke<bool>(controller_index);
		}

		bool bg_unlockablesisitempurchased_stub(game::eModes mode, const game::ControllerIndex_t controller_index, int item_index)
		{
			if (dvar_cg_unlockall_purchases->current.value.enabled)
			{
				return true;
			}

			return bg_unlockablesisitempurchased_hook.invoke<bool>(mode, controller_index, item_index);
		}

		bool bg_unlockablesisitemattachmentlocked_stub(game::eModes mode, const game::ControllerIndex_t controller_index, int item_index, int attachment_num)
		{
			if (dvar_cg_unlockall_attachments->current.value.enabled)
			{
				return false;
			}

			return bg_unlockablesisitemattachmentlocked_hook.invoke<bool>(mode, controller_index, item_index, attachment_num);
		}

		bool bg_unlockablesisattachmentslotlocked_stub(game::eModes mode, const game::ControllerIndex_t controller_index, int item_index, int attachment_slot_index)
		{
			if (dvar_cg_unlockall_attachments->current.value.enabled)
			{
				return false;
			}

			return bg_unlockablesisattachmentslotlocked_hook.invoke<bool>(mode, controller_index, item_index, attachment_slot_index);
		}
	};

	struct component final : client_component
	{
		void post_unpack() override
		{
			dvar_cg_unlockall_loot = game::Dvar_RegisterBool(game::Dvar_GenerateHash("cg_unlockall_loot"), "cg_unlockall_loot", false, (game::dvarFlags_e)0x0, "Unlocks blackmarket loot");
			dvar_cg_unlockall_loot->debugName = "cg_unlockall_loot";
			dvar_cg_unlockall_purchases = game::Dvar_RegisterBool(game::Dvar_GenerateHash("cg_unlockall_purchases"), "cg_unlockall_purchases", false, (game::dvarFlags_e)0x0, "Unlock all purchases with tokens");
			dvar_cg_unlockall_purchases->debugName = "cg_unlockall_purchases";
			dvar_cg_unlockall_attachments = game::Dvar_RegisterBool(game::Dvar_GenerateHash("cg_unlockall_attachments"), "cg_unlockall_attachments", false, (game::dvarFlags_e)0x0, "Unlocks all attachments");
			dvar_cg_unlockall_attachments->debugName = "cg_unlockall_attachments";
			dvar_cg_cg_unlockall_camos = game::Dvar_RegisterBool(game::Dvar_GenerateHash("cg_unlockall_camos"), "cg_unlockall_camos", false, (game::dvarFlags_e)0x0, "Unlocks all camos");
			dvar_cg_cg_unlockall_camos->debugName = "cg_unlockall_camos";

			loot_getitemquantity_hook.create(0x141E82C00_g, loot_getitemquantity_stub);
			liveinventory_getitemquantity_hook.create(0x141E09030_g, liveinventory_getitemquantity_stub);
			liveinventory_areextraslotspurchased_hook.create(0x141E08950_g, liveinventory_areextraslotspurchased_stub);
			bg_unlockablesisitempurchased_hook.create(0x1426A9620_g, bg_unlockablesisitempurchased_stub);
			bg_unlockablesisitemattachmentlocked_hook.create(0x1426A88D0_g, bg_unlockablesisitemattachmentlocked_stub);
			bg_unlockablesisattachmentslotlocked_hook.create(0x1426A86D0_g, bg_unlockablesisattachmentslotlocked_stub);

			scheduler::once([]() {
				if (dvar_cg_unlockall_loot->current.value.enabled)
				{
					game::Dvar_SetFromStringByName("ui_enableAllHeroes", "1", true);
				}
			}, scheduler::pipeline::dvars_loaded);
		}
	};
};

REGISTER_COMPONENT(loot::component)
