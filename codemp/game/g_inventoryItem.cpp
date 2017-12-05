#include "g_local.h"
#include "g_inventoryItem.h"

//
// Global Stuff...
//
int					INVENTORY_ITEM_INSTANCES_COUNT = 0;	// TODO: This should be initialized to database's highest value at map load... Or even better, always store the max id to it's own DB field.
inventoryItem		*INVENTORY_ITEM_INSTANCES[8388608];

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
	m_baseItem = bg_itemlist[0];
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

	INVENTORY_ITEM_INSTANCES[INVENTORY_ITEM_INSTANCES_COUNT] = this;
	INVENTORY_ITEM_INSTANCES_COUNT++;
}

inventoryItem::inventoryItem(gitem_t bgItem, itemQuality_t quality, std::string name, std::string description, double price, int amount)
{
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

	setTotalCost();

	INVENTORY_ITEM_INSTANCES[INVENTORY_ITEM_INSTANCES_COUNT] = this;
	INVENTORY_ITEM_INSTANCES_COUNT++;
}

inventoryItem::~inventoryItem()
{

}

//
// Database functions... TODO...
//
void inventoryItem::storeToDatabase(inventoryItem *item)
{

}

inventoryItem *inventoryItem::loadFromDatabase(int playerID)
{
	return NULL;
}

//
// Item Setup Functions...
//
void inventoryItem::setBaseItem(gitem_t item)
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

//
// Item Accessing Functions...
//
gitem_t inventoryItem::getBaseItem()
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
