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
	m_baseItem = &bg_itemlist[0];
	m_quality = QUALITY_WHITE;
	m_modification1 = NULL;
	m_modification2 = NULL;
	m_modification3 = NULL;
	m_modification4 = NULL;
	m_description = "";
	m_quantity = 0;
	m_cost = 0;
	m_totalCost = 0;
	m_playerID = -1;

#ifdef _CGAME
	m_customIcon = NULL;
#endif

	m_destroyTime = -1;

#ifdef _GAME
	INVENTORY_ITEM_INSTANCES[INVENTORY_ITEM_INSTANCES_COUNT] = this;
	INVENTORY_ITEM_INSTANCES_COUNT++;
#endif
}

inventoryItem::inventoryItem(gitem_t *bgItem, itemQuality_t quality, std::string name, std::string description, double price, int amount = 1, std::string customIcon = "", int destroyTime = -1)
{
#ifdef _GAME
	m_itemID = INVENTORY_ITEM_INSTANCES_COUNT;
#endif
	m_baseItem = bgItem;
	m_quality = quality;
	m_modification1 = NULL;
	m_modification2 = NULL;
	m_modification3 = NULL;
	m_modification4 = NULL;
	m_name = name;
	m_description = description;
	m_quantity = amount;
	m_cost = price;
	m_playerID = -1;

#ifdef _CGAME
	m_customIcon = trap->R_RegisterShader(customIcon.length() > 0 ? customIcon.c_str() : bgItem->icon);
#endif

	setTotalCost();

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
inventoryItem *createInventoryItem(gitem_t *bgItem, itemQuality_t quality, std::string name, std::string description, double price, int amount, std::string customIcon)
{
	inventoryItem *item = new inventoryItem(bgItem, quality, name, description, price, amount, customIcon);

	clientInfo_t *client = &cgs.clientinfo[cg.clientNum];
	client->inventory[client->inventoryCount] = item;

	return item;
}
#endif

#ifdef _GAME
inventoryItem *createInventoryItem(gitem_t *bgItem, itemQuality_t quality, std::string name, std::string description, double price, int amount, std::string customIcon, int clientNum, int destroyTime)
{
	inventoryItem *item = new inventoryItem(bgItem, quality, name, description, price, amount, customIcon, destroyTime);

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
	inventoryItem *mod1 = item->getModification1();
	inventoryItem *mod2 = item->getModification2();
	inventoryItem *mod3 = item->getModification3();
	inventoryItem *mod4 = item->getModification4();

	// Also trash any mods on it...
	if (mod1) delete[] mod1;
	if (mod2) delete[] mod2;
	if (mod3) delete[] mod3;
	if (mod4) delete[] mod4;

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

void inventoryItem::setBaseItem(gitem_t *item)
{
	m_baseItem = item;
}

void inventoryItem::setQuality(itemQuality_t quality)
{
	m_quality = quality;
	setTotalCost();
}

void inventoryItem::setModification1(inventoryItem *modification)
{
	m_modification1 = modification;
	setTotalCost();
}

void inventoryItem::setModification2(inventoryItem *modification)
{
	m_modification2 = modification;
	setTotalCost();
}

void inventoryItem::setModification3(inventoryItem *modification)
{
	m_modification3 = modification;
	setTotalCost();
}

void inventoryItem::setModification4(inventoryItem *modification)
{
	m_modification4 = modification;
	setTotalCost();
}

void inventoryItem::setName(std::string name)
{
	m_name = name;
}

void inventoryItem::setDescription(std::string description)
{
	m_description = description;
}

void inventoryItem::setQuantity(int amount)
{
	m_quantity = amount;
	setTotalCost();
}

void inventoryItem::setCost(double price)
{
	m_cost = price;
	setTotalCost();
}

void inventoryItem::setTotalCost()
{
	m_totalCost = m_quantity * m_cost;
}

void inventoryItem::setPlayerID(int playerID)
{
	m_playerID = playerID;
}

void inventoryItem::setCustomIcon(qhandle_t icon)
{
	m_customIcon = icon;
}

void inventoryItem::setDestroyTime(int destroyTime)
{
	m_destroyTime = destroyTime;
}

//
// Item Accessing Functions...
//
gitem_t *inventoryItem::getBaseItem()
{
	return m_baseItem;
}

itemQuality_t inventoryItem::getQuality()
{
	return m_quality;
}

inventoryItem *inventoryItem::getModification1()
{
	return m_modification1;
}

inventoryItem *inventoryItem::getModification2()
{
	return m_modification2;
}

inventoryItem *inventoryItem::getModification3()
{
	return m_modification3;
}

inventoryItem *inventoryItem::getModification4()
{
	return m_modification4;
}

std::string inventoryItem::getName()
{
	return m_name;
}

std::string inventoryItem::getDescription()
{
	return m_description;
}

int inventoryItem::getQuantity()
{
	return m_quantity;
}

double inventoryItem::getCost()
{
	double modCost1 = m_modification1 ? m_modification1->getTotalCost() : 0.0;
	double modCost2 = m_modification2 ? m_modification2->getTotalCost() : 0.0;
	double modCost3 = m_modification3 ? m_modification3->getTotalCost() : 0.0;
	double modCost4 = m_modification4 ? m_modification4->getTotalCost() : 0.0;
	return (m_cost + modCost1 + modCost2 + modCost3 + modCost4) * qualityPriceModifier[m_quality];
}

double inventoryItem::getTotalCost()
{
	double modCost1 = m_modification1 ? m_modification1->getTotalCost() : 0.0;
	double modCost2 = m_modification2 ? m_modification2->getTotalCost() : 0.0;
	double modCost3 = m_modification3 ? m_modification3->getTotalCost() : 0.0;
	double modCost4 = m_modification4 ? m_modification4->getTotalCost() : 0.0;
	return (m_totalCost + modCost1 + modCost2 + modCost3 + modCost4) * qualityPriceModifier[m_quality];
}

int inventoryItem::getPlayerID()
{
	return m_playerID;
}

qhandle_t inventoryItem::getIcon()
{
	return m_customIcon;
}

int inventoryItem::getDestroyTime()
{
	return m_destroyTime;
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
#if 0
	gclient_t *client = g_entities[clientNum].client;
	// TODO: Construct a compressed char string for this item and send.
	uncompressedItemBuffer_t *itemBuffer = addItemFieldsToBufferHere...
	char *compressedItemBuffer = zlibCompress(itemBuffer);
	trap->SendServerCommand(clientNum, compressedItemBuffer);
#endif
}

void inventoryItem::uploadFullInventoryToClient(int clientNum)
{// To be called at login, or on map change...
	gclient_t *client = g_entities[clientNum].client;

	for (int i = 0; i < client->inventoryCount; i++)
	{
		uploadToClient(clientNum, client->inventory[i]);
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
#if 0
	uncompressedItemBuffer_t *buffer = zlibDecompress((compressedItemBuffer_t *)packet);
	clientInfo_t *client = &cgs.clientinfo[cg.clientNum];

	if (buffer->isNewItem)
	{// Brand spanking new item...
		inventoryItem *item = createInventoryItem(&bg_itemlist[buffer->baseItemID], buffer->quality, buffer->name, buffer->description, buffer->cost, buffer->quantity, buffer->cutomIcon);
		
		item->setItemID(buffer->itemID);
		item.setModification1(buffer->modification1);
		item.setModification2(buffer->modification2);
		item.setModification3(buffer->modification3);
		item.setModification4(buffer->modification4);

		client->inventory[client->inventoryCount] = &item;
		client->inventoryCount++;
	}
	else
	{// Update to old item...
		inventoryItem item = NULL;
		
		for (int i = 0; i < client->inventoryCount; i++)
		{
			if (client->inventory[i].m_itemID == buffer->itemID)
			{
				item = client->inventory[i];
				break;
			}
		}

		item->setItemID(buffer->itemID);
		item->setBaseItem(&bg_itemlist[buffer->baseItemID]);
		item->setQuality(buffer->quality);
		item->setModification1(buffer->modification1);
		item->setModification2(buffer->modification2);
		item->setModification3(buffer->modification3);
		item->setModification4(buffer->modification4);
		item->setName(buffer->name);
		item->setDescription(buffer->description);
		item->setQuantity(buffer->quantity);
		item->setCost(buffer->cost);
		item->setCustomIcon(buffer->cutomIcon);
		//... etc
	}
#endif
}
#endif
