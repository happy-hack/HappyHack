#include "skinchanger.hpp"
#include "../../Menu/Menu.h"
void skins::apply() {

	knife_changer();
}

void skins::knife_changer() {
	auto pWeapon = g::local->GetActiveWeapon();

	if (pWeapon) {
		if (pWeapon == nullptr)
			return;

		int model_butterfly = g_pModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");

		if (pWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_KNIFE) {
			pWeapon->SetModelIndex(model_butterfly);
			pWeapon->ViewModelIndex(model_butterfly);
			pWeapon->ModelIndex(model_butterfly);
			pWeapon->SetItemDefinitionIndex(ItemDefinitionIndex::WEAPON_KNIFE_BUTTERFLY);
		}
		//*pWeapon->OwnerXuidHigh() = 0;
		//pWeapon->FallbackWear(0.1f);
		//pWeapon->ItemIDHigh(1);
	}
}