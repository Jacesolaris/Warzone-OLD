#pragma once

#ifndef _INVENTORY_
#define _INVENTORY_

#include <string>

//
// Quality levels of items...
//
typedef enum {
	QUALITY_WHITE,
	QUALITY_GREEN,
	QUALITY_BLUE,
	QUALITY_PURPLE,
	QUALITY_GOLD
} itemQuality_t;

//
// A quality based price scale modifier... Used internally... Matches levels of itemQuality_t.
//
extern float qualityPriceModifier[5];

class inventoryItem
{
private:
	// Values...
	gitem_t				m_baseItem;
	itemQuality_t		m_quality;
	inventoryItem		*m_modification1;
	inventoryItem		*m_modification2;
	inventoryItem		*m_modification3;
	inventoryItem		*m_modification4;
	std::string			m_name;
	std::string			m_description;
	int					m_quantity;
	double				m_cost;
	double				m_totalCost;
	int					m_playerID;

	//
	// Private Internal Functions...
	//
	void setTotalCost();

public:
	//
	// Construction/Destruction...
	//
	inventoryItem(); // default constructor member variables are 0 or NULL.
	inventoryItem(gitem_t, itemQuality_t, std::string, std::string, double, int); // paramterized constructor
	~inventoryItem(); // destructor

	//
	// Database functions...
	//
	void storeToDatabase(inventoryItem*);
	inventoryItem *loadFromDatabase(int);

	//
	// Item Setup Functions...
	//
	void setBaseItem(gitem_t);
	void setQuality(itemQuality_t);
	void setModification1(inventoryItem*);
	void setModification2(inventoryItem*);
	void setModification3(inventoryItem*);
	void setModification4(inventoryItem*);
	void setName(std::string);
	void setDescription(std::string);
	void setQuantity(int);
	void setCost(double);
	void setPlayerID(int);
	
	//
	// Item Accessing Functions...
	//
	gitem_t	getBaseItem();
	itemQuality_t getQuality();
	inventoryItem *getModification1();
	inventoryItem *getModification2();
	inventoryItem *getModification3();
	inventoryItem *getModification4();
	std::string getName();
	std::string getDescription();
	int getQuantity();
	double getCost();
	double getTotalCost();
	int getPlayerID();
};

//
// Global Stuff...
//
extern int				INVENTORY_ITEM_INSTANCES_COUNT;
extern inventoryItem	*INVENTORY_ITEM_INSTANCES[8388608];

#endif
