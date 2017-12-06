#ifdef _GAME
#include "g_local.h"
#endif

#ifdef _CGAME
#include "../cgame/cg_local.h"
#endif

#ifdef _GAME
//
// Global Stuff... This is for the server to keep track of unique item instances.
//
int					INVENTORY_ITEM_INSTANCES_COUNT = 0;	// TODO: This should be initialized to database's highest value at map load... Or even better, always store the max id to it's own DB field.
inventoryItem		*INVENTORY_ITEM_INSTANCES[8388608];
#endif

//
// A quality based price scale modifier... Used internally... Matches levels of itemQuality_t.
//
float qualityPriceModifier[5] = {
	1.0,
	2.0,
	4.0,
	8.0,
	16.0
};

//
// Construction/Destruction...
//
inventoryItem::inventoryItem()
{
#ifdef _GAME
	m_itemID = INVENTORY_ITEM_INSTANCES_COUNT;
#endif
	m_bgItemID = 0;
	m_quality = QUALITY_GREY;
	m_quantity = 0;
	m_playerID = -1;

	m_icon = NULL;

	m_transmitted = qfalse;

	m_destroyTime = -1;

	m_crystal = 0;
	m_basicStat1 = 0;
	m_basicStat2 = 0;
	m_basicStat3 = 0;
	m_basicStat1value = 0.0;
	m_basicStat2value = 0.0;
	m_basicStat3value = 0.0;
	m_modStat1 = 0;
	m_modStat2 = 0;
	m_modStat3 = 0;
	m_modStatValue1 = 0.0;
	m_modStatValue2 = 0.0;
	m_modStatValue3 = 0.0;

#ifdef _GAME
	INVENTORY_ITEM_INSTANCES[INVENTORY_ITEM_INSTANCES_COUNT] = this;
	INVENTORY_ITEM_INSTANCES_COUNT++;
#endif
}

inventoryItem::inventoryItem(int bgItemID, itemQuality_t quality, int amount = 1, int destroyTime = -1)
{
#ifdef _GAME
	m_itemID = INVENTORY_ITEM_INSTANCES_COUNT;
#endif

	m_bgItemID = bgItemID;
	m_quality = quality;
	m_quantity = amount;
	m_playerID = -1;

	m_icon = NULL;

#ifdef _CGAME
	m_icon = trap->R_RegisterShader(bg_itemlist[m_bgItemID].icon);
#endif

	m_transmitted = qfalse;

	m_crystal = 0;
	m_basicStat1 = 0;
	m_basicStat2 = 0;
	m_basicStat3 = 0;
	m_basicStat1value = 0.0;
	m_basicStat2value = 0.0;
	m_basicStat3value = 0.0;
	m_modStat1 = 0;
	m_modStat2 = 0;
	m_modStat3 = 0;
	m_modStatValue1 = 0.0;
	m_modStatValue2 = 0.0;
	m_modStatValue3 = 0.0;

	m_destroyTime = destroyTime;

#ifdef _GAME
	INVENTORY_ITEM_INSTANCES[INVENTORY_ITEM_INSTANCES_COUNT] = this;
	INVENTORY_ITEM_INSTANCES_COUNT++;
#endif
}

inventoryItem::~inventoryItem()
{

}

#ifdef _CGAME
inventoryItem *createInventoryItem(int bgItemID, itemQuality_t quality, int amount)
{
	inventoryItem *item = new inventoryItem(bgItemID, quality, amount);

	clientInfo_t *client = &cgs.clientinfo[cg.clientNum];
	client->inventory[client->inventoryCount] = item;

	return item;
}
#endif

#ifdef _GAME
inventoryItem *createInventoryItem(int bgItemID, itemQuality_t quality, int amount, int clientNum, int destroyTime)
{
	inventoryItem *item = new inventoryItem(bgItemID, quality, amount, destroyTime);

	if (clientNum >= 0)
	{
		gclient_t *client = g_entities[clientNum].client;
		client->inventory[client->inventoryCount] = item;
		item->setDestroyTime(-1); // Don't time out and destroy stuff in a player's inventory...
	}

	return item;
}
#endif

void destroyInventoryItem(inventoryItem *item)
{
	delete[] item;
}

#ifdef _GAME
void destroyOldLootItems(void)
{// Call this periodically to remove items players never looted after they expire... (only needed if we ever decide to apply inventory item info and mods pre-looting).
	for (int i = 0; i < INVENTORY_ITEM_INSTANCES_COUNT; i++)
	{
		inventoryItem *item = INVENTORY_ITEM_INSTANCES[i];

		if (item)
		{
			int destroyTime = item->getDestroyTime();

			if (destroyTime >= 0 && destroyTime >= level.time)
			{
				destroyInventoryItem(item);
			}
		}
	}
}
#endif

//
// Item Setup Functions...
//
void inventoryItem::setItemID(int itemID)
{
	m_itemID = itemID;
}

void inventoryItem::setBaseItem(int bgItemID)
{
	m_bgItemID = bgItemID;
}

void inventoryItem::setQuality(itemQuality_t quality)
{
	m_quality = quality;
}

void inventoryItem::setQuantity(int amount)
{
	m_quantity = amount;
}

void inventoryItem::setPlayerID(int playerID)
{
	m_playerID = playerID;
}

void inventoryItem::setDestroyTime(int destroyTime)
{
	m_destroyTime = destroyTime;
}

void inventoryItem::setCrystal(int crystalType)
{
	m_crystal = crystalType;
}

void inventoryItem::setStat1(int statType, float statValue)
{
	if (m_quality <= QUALITY_GREY) return; // Not available...
	if (isCrystal()) return; // crystals can not have stats.
	if (isModification() && (m_basicStat2 || m_basicStat3)) return; // mods can only have 1 stat type.
	
	m_basicStat1 = statType;
	m_basicStat1value = statValue;
}

void inventoryItem::setStat2(int statType, float statValue)
{
	if (m_quality <= QUALITY_GREEN) return; // Not available...
	if (isCrystal()) return; // crystals can not have stats.
	if (isModification() && (m_basicStat1 || m_basicStat3)) return; // mods can only have 1 stat type.

	m_basicStat2 = statType;
	m_basicStat2value = statValue;
}

void inventoryItem::setStat3(int statType, float statValue)
{
	if (m_quality <= QUALITY_BLUE) return; // Not available...
	if (isCrystal()) return; // crystals can not have stats.
	if (isModification() && (m_basicStat1 || m_basicStat2)) return; // mods can only have 1 stat type.

	m_basicStat3 = statType;
	m_basicStat3value = statValue;
}

void inventoryItem::setMod1(int statType, float statValue)
{
	if (m_quality <= QUALITY_GREY) return; // Not available...
	if (isCrystal()) return; // crystals can not have stats.
	if (isModification()) return; // mods can't have mods :)

	m_modStat1 = statType;
	m_modStatValue1 = statValue;
}

void inventoryItem::setMod2(int statType, float statValue)
{
	if (m_quality <= QUALITY_GREEN) return; // Not available...
	if (isCrystal()) return; // crystals can not have stats.
	if (isModification()) return; // mods can't have mods :)

	m_modStat2 = statType;
	m_modStatValue2 = statValue;
}

void inventoryItem::setMod3(int statType, float statValue)
{
	if (m_quality <= QUALITY_BLUE) return; // Not available...
	if (isCrystal()) return; // crystals can not have stats.
	if (isModification()) return; // mods can't have mods :)

	m_modStat3 = statType;
	m_modStatValue3 = statValue;
}

//
// Item Accessing Functions...
//
gitem_t *inventoryItem::getBaseItem()
{
	return &bg_itemlist[m_bgItemID];
}

int inventoryItem::getBaseItemID()
{
	return m_bgItemID;
}

qhandle_t inventoryItem::getIcon()
{
	return m_icon;
}

itemQuality_t inventoryItem::getQuality()
{
	return m_quality;
}

int inventoryItem::getQuantity()
{
	return m_quantity;
}

int inventoryItem::getBasicStat1()
{
	return m_basicStat1;
}

int inventoryItem::getBasicStat2()
{
	return m_basicStat2;
}

int inventoryItem::getBasicStat3()
{
	return m_basicStat3;
}

float inventoryItem::getBasicStat1Value()
{
	return m_basicStat1value;
}

float inventoryItem::getBasicStat2Value()
{
	return m_basicStat2value;
}

float inventoryItem::getBasicStat3Value()
{
	return m_basicStat3value;
}

int inventoryItem::getCrystal()
{
	return m_crystal;
}

int inventoryItem::getMod1Stat()
{
	return m_modStat1;
}

int inventoryItem::getMod2Stat()
{
	return m_modStat2;
}

int inventoryItem::getMod3Stat()
{
	return m_modStat3;
}

float inventoryItem::getMod1Value()
{
	return m_modStatValue1;
}

float inventoryItem::getMod2Value()
{
	return m_modStatValue2;
}

float inventoryItem::getMod3Value()
{
	return m_modStatValue3;
}

double inventoryItem::getCost()
{// Apply multipliers based on how many extra stats this item has...
	double crystalCostMultiplier = getCrystal() ? 1.5 : 0.0;
	double statCostMultiplier1 = getBasicStat1() ? 1.25 : 0.0;
	double statCostMultiplier2 = getBasicStat1() ? 1.25 : 0.0;
	double statCostMultiplier3 = getBasicStat1() ? 1.25 : 0.0;
	double modCostMultiplier1 = getMod1Stat() ? 1.25 : 0.0;
	double modCostMultiplier2 = getMod2Stat() ? 1.25 : 0.0;
	double modCostMultiplier3 = getMod3Stat() ? 1.25 : 0.0;
	return getBaseItem()->price * statCostMultiplier1 * statCostMultiplier2 * statCostMultiplier3 * modCostMultiplier1 * modCostMultiplier2 * modCostMultiplier3 * crystalCostMultiplier * qualityPriceModifier[m_quality];
}

double inventoryItem::getTotalCost()
{// Apply multipliers based on how many extra stats this item has...
	double crystalCostMultiplier = getCrystal() ? 1.5 : 0.0;
	double statCostMultiplier1 = getBasicStat1() ? 1.25 : 0.0;
	double statCostMultiplier2 = getBasicStat1() ? 1.25 : 0.0;
	double statCostMultiplier3 = getBasicStat1() ? 1.25 : 0.0;
	double modCostMultiplier1 = getMod1Stat() ? 1.25 : 0.0;
	double modCostMultiplier2 = getMod2Stat() ? 1.25 : 0.0;
	double modCostMultiplier3 = getMod3Stat() ? 1.25 : 0.0;
	return getBaseItem()->price * m_quantity * statCostMultiplier1 * statCostMultiplier2 * statCostMultiplier3 * modCostMultiplier1 * modCostMultiplier2 * modCostMultiplier3 * crystalCostMultiplier * qualityPriceModifier[m_quality];
}

int inventoryItem::getPlayerID()
{
	return m_playerID;
}

int inventoryItem::getDestroyTime()
{
	return m_destroyTime;
}


qboolean inventoryItem::isModification()
{
	int giType = getBaseItem()->giType;

	if (giType == IT_ITEM_MODIFICATION || giType == IT_WEAPON_MODIFICATION || giType == IT_SABER_MODIFICATION)
		return qtrue;

	return qfalse;
}

qboolean inventoryItem::isCrystal()
{
	int giType = getBaseItem()->giType;

	if (giType == IT_SABER_CRYSTAL || giType == IT_WEAPON_CRYSTAL || giType == IT_ITEM_CRYSTAL)
		return qtrue;

	return qfalse;
}

//
// Database functions... TODO...
//
#ifdef _GAME
void inventoryItem::storeToDatabase(inventoryItem *item)
{// Will be called when an item has been modified or looted...
 // TODO: Write to DB here...
}

void inventoryItem::storeToDatabase(int clientNum, int playerID)
{// Will be called on server, before game shutdown/rest or when player logs out...
	gclient_t *client = g_entities[clientNum].client;

	for (int i = 0; i < client->inventoryCount; i++)
	{
		storeToDatabase(client->inventory[i]);
	}
}

void inventoryItem::loadFromDatabase(int clientNum, int playerID)
{// Will be called on server when client logs in...
	gclient_t *client = g_entities[clientNum].client;
	client->inventoryCount = 0;

	for (int i = 0; i < INVENTORY_ITEM_INSTANCES_COUNT; i++)
	{
		inventoryItem *item = NULL; // TODO: lookup DB instead of &bg_itemlist[0] - eg: get all item SQL entries for playerID.
		INVENTORY_ITEM_INSTANCES[m_itemID] = item;
		client->inventory[client->inventoryCount] = item;
		client->inventoryCount = 0;
	}
}

void inventoryItem::removeFromDatabase(inventoryItem *item)
{// Will be called when an item has been modified or looted...
 // TODO: Write to DB here...
}
#endif

//
// TODO: Upload items to clients...
//
#ifdef _GAME
void inventoryItem::uploadToClient(int clientNum, inventoryItem *item)
{// To be called when a new item is looted, or an item is modified.
	gclient_t *client = g_entities[clientNum].client;
	std::string command = "inv ";
	command += va("\"%i\"", item->m_transmitted ? 0 : 1);
	command += va("\"%i\"", item->getBaseItem());
	command += va("\"%i\"", item->getQuality());
	command += va("\"%i\"", item->getQuantity());
	command += va("\"%i\"", item->getCrystal());
	command += va("\"%i\"", item->getBasicStat1());
	command += va("\"%i\"", item->getBasicStat2());
	command += va("\"%i\"", item->getBasicStat3());
	command += va("\"%f\"", item->getBasicStat1Value());
	command += va("\"%f\"", item->getBasicStat2Value());
	command += va("\"%f\"", item->getBasicStat3Value());
	command += va("\"%i\"", item->getMod1Stat());
	command += va("\"%i\"", item->getMod2Stat());
	command += va("\"%i\"", item->getMod3Stat());
	command += va("\"%f\"", item->getMod1Value());
	command += va("\"%f\"", item->getMod2Value());
	command += va("\"%f\"", item->getMod3Value());
	trap->SendServerCommand(clientNum, command.c_str());
	item->m_transmitted = qtrue;
}

void inventoryItem::uploadFullInventoryToClient(int clientNum)
{// To be called at login, or on map change...
	gclient_t *client = g_entities[clientNum].client;

	for (int i = 0; i < client->inventoryCount; i++)
	{
		if (client->inventory[i])
		{
			uploadToClient(clientNum, client->inventory[i]);
		}
	}
}
#endif

//
// TODO: Recieve new items from server...
//
#ifdef _CGAME
void inventoryItem::downloadFromServer(char *packet)
{
	// TODO: Deconstruct compressed char string the server sent, and add the item to inventory...
	clientInfo_t *client = &cgs.clientinfo[cg.clientNum];

	qboolean isNewitem = (qboolean)atoi(CG_Argv(2));
	int itemID = atoi(CG_Argv(3));
	int baseItemID = atoi(CG_Argv(4));
	int quality = atoi(CG_Argv(5));
	int quantity = atoi(CG_Argv(6));
	int crystal = atoi(CG_Argv(7));
	int basicStat1 = atoi(CG_Argv(8));
	int basicStat2 = atoi(CG_Argv(9));
	int basicStat3 = atoi(CG_Argv(10));
	int basicStat1Value = atof(CG_Argv(11));
	int basicStat2Value = atof(CG_Argv(12));
	int basicStat3Value = atof(CG_Argv(13));
	int mod1Type = atoi(CG_Argv(14));
	int mod2Type = atoi(CG_Argv(15));
	int mod3Type = atoi(CG_Argv(16));
	int mod1Value = atof(CG_Argv(17));
	int mod2Value = atof(CG_Argv(18));
	int mod3Value = atof(CG_Argv(19));

	if (isNewitem)
	{// Brand spanking new item...
		inventoryItem *item = createInventoryItem(baseItemID, (itemQuality_t)quality, quantity);
		
		item->setItemID(itemID);

		item->setCrystal(crystal);

		item->setStat1(basicStat1, basicStat1Value);
		item->setStat2(basicStat2, basicStat2Value);
		item->setStat3(basicStat3, basicStat3Value);
		
		item->setMod1(mod1Type, mod1Value);
		item->setMod2(mod3Type, mod2Value);
		item->setMod3(mod2Type, mod3Value);

		client->inventory[client->inventoryCount] = item;
		client->inventoryCount++;
	}
	else
	{// Update to old item...
		inventoryItem *item = NULL;
		
		for (int i = 0; i < client->inventoryCount; i++)
		{
			if (client->inventory[i] && client->inventory[i]->m_itemID == itemID)
			{
				item = client->inventory[i];
				break;
			}
		}

		item->setItemID(itemID);
		item->setBaseItem(baseItemID);
		item->setQuality((itemQuality_t)quality);
		item->setCrystal(crystal);
		
		item->setStat1(basicStat1, basicStat1Value);
		item->setStat2(basicStat2, basicStat2Value);
		item->setStat3(basicStat3, basicStat3Value);

		item->setMod1(mod1Type, mod1Value);
		item->setMod2(mod3Type, mod2Value);
		item->setMod3(mod2Type, mod3Value);

		item->setQuantity(quantity);
		//... etc
	}
}
#endif
