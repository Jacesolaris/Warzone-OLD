#pragma once

#ifndef _INVENTORY_
#define _INVENTORY_

#include "../game/bg_public.h"
#include "../qcommon/q_shared.h"

#include <string>

//
// Quality levels of items...
//
typedef enum {
	QUALITY_GREY,		// on weapons/sabers/items - 0 base stats and 0 mod slots.
	QUALITY_GREEN,		// on weapons/sabers/items - 1 base stats and 1 mod slots.
	QUALITY_BLUE,		// on weapons/sabers/items - 2 base stats and 2 mod slots.
	QUALITY_PURPLE,		// on weapons/sabers/items - 3 base stats and 3 mod slots. 1.25x stat bonus.
	QUALITY_GOLD		// on weapons/sabers/items - 3 base stats and 3 mod slots. 2x stat bonus.
} itemQuality_t;

//
// Stats available to items...
//

// For weapons/sabers/mods/items crystal (power) slot only. Damage types for weapons, and damage resistance types for items.
typedef enum {
	ITEM_CRYSTAL_DEFAULT,			// GREY shots/blade? No special damage/resistance type...
	ITEM_CRYSTAL_RED,				// Bonus Heat Damage/Resistance
	ITEM_CRYSTAL_GREEN,				// Bonus Kinetic (force) Damage/Resistance
	ITEM_CRYSTAL_BLUE,				// Bonus Electric Damage/Resistance
	ITEM_CRYSTAL_WHITE,				// Bonus Cold Damage/Resistance
	// Maybe extras below should be combos of the above? Stois? Thoughts?
	ITEM_CRYSTAL_YELLOW,			// Bonus 1/2 Heat + 1/2 Kinetic??? Damage/Resistance
	ITEM_CRYSTAL_PURPLE,			// Bonus 1/2 Electric + 1/2 Heat??? Damage/Resistance
	ITEM_CRYSTAL_ORANGE,			// Bonus 2/3 Heat + 1/3 Kinetic??? Damage/Resistance
	ITEM_CRYSTAL_PINK,				// Bonus 1/4 Heat + 1/3 Electric + 1/3 Cold Damage/Resistance
} baseItemPowerCrystal_t;

// For weapons/weapon-mods slot 1 only.
typedef enum {
	WEAPON_STAT1_DEFAULT,
	WEAPON_STAT1_FIRE_ACCURACY_MODIFIER,
	WEAPON_STAT1_FIRE_RATE_MODIFIER,
	WEAPON_STAT1_FIRE_DAMAGE_MODIFIER,
	WEAPON_STAT1_FIRE_HEAT_MODIFIER,
	WEAPON_STAT1_MAX
} weaponStat1_t;

// For weapons/weapon-mods slot 2 only.
typedef enum {
	WEAPON_STAT2_DEFAULT,
	WEAPON_STAT2_CRITICAL_CHANCE_MODIFIER,
	WEAPON_STAT2_CRITICAL_POWER_MODIFIER,
	WEAPON_STAT2_HEAT_ACCUMULATION_MODIFIER,
	WEAPON_STAT2_MAX
} weaponStat2_t;

// For weapons/mods/items slot 3 only.
typedef enum {
	WEAPON_STAT3_SHOT_DEFAULT,
	WEAPON_STAT3_SHOT_BOUNCE,
	WEAPON_STAT3_SHOT_EXPLODE,
	WEAPON_STAT3_SHOT_EXPLOSIVE,
	WEAPON_STAT3_SHOT_VELOCITY,
	WEAPON_STAT3_SHOT_BEAM,
	WEAPON_STAT3_SHOT_WIDE,
	WEAPON_STAT3_MAX
} weaponStat3_t;

// For sabers/saber-mods slot 1 only.
typedef enum {
	SABER_STAT1_DEFAULT,
	SABER_STAT1_DAMAGE_MODIFIER,
	SABER_STAT1_SHIELD_PENETRATION_MODIFIER,
	SABER_STAT1_MAX
} saberStat1_t;

// For sabers/saber-mods slot 2 only.
typedef enum {
	SABER_STAT2_DEFAULT,
	SABER_STAT2_CRITICAL_CHANCE_MODIFIER,
	SABER_STAT2_CRITICAL_POWER_MODIFIER,
	SABER_STAT2_MAX
} saberStat2_t;

// For sabers/saber-mods slot 3 only.
typedef enum {
	SABER_STAT3_DEFAULT,
	SABER_STAT3_LENGTH_MODIFIER,
	SABER_STAT3_SPEED_MODIFIER,
	SABER_STAT3_MAX
} saberStat3_t;

// For any items/item-mods slots only.
typedef enum {
	ITEM_STAT1_DEFAULT,
	ITEM_STAT1_HEALTH_MAX_MODIFIER,
	ITEM_STAT1_HEALTH_REGEN_MODIFIER,
	ITEM_STAT1_SHIELD_MAX_MODIFIER,
	ITEM_STAT1_SHIELD_REGEN_MODIFIER,
	ITEM_STAT1_FORCEPOWER_MAX_MODIFIER,
	ITEM_STAT1_FORCEPOWER_REGEN_MODIFIER,
	ITEM_STAT1_STRENGTH_MODIFIER,
	ITEM_STAT1_EVASION_MODIFIER,
	ITEM_STAT1_SPEED_MODIFIER,
	ITEM_STAT1_AGILITY_MODIFIER, // 1/2 speed + 1/2 evasion
	ITEM_STAT1_BLOCKING_MODIFIER,
	ITEM_STAT1_PHYSICAL_DAMAGE_REDUCTION_MODIFIER,
	ITEM_STAT1_PHYSICAL_DAMAGE_PENETRATION_REDUCTION_MODIFIER,
	ITEM_STAT1_RANGED_DAMAGE_REDUCTION_MODIFIER,
	ITEM_STAT1_RANGED_DAMAGE_PENETRATION_REDUCTION_MODIFIER,
	ITEM_STAT1_MAX
} itemStat_t;

//
// A quality based price scale modifier... Used internally... Matches levels of itemQuality_t.
//
extern float qualityPriceModifier[5];

class inventoryItem
{
private:
	// Values...
	int					m_itemID;
	int					m_bgItemID;
	qhandle_t			m_icon;
	itemQuality_t		m_quality;
	int					m_quantity;
	int					m_playerID;

	int					m_destroyTime;

	// Base item modifier stats... The item itself has 1 crystal (color modifier), and 3 stat modifiers.
	int					m_crystal;				// itemPowerCrystal_t - Damage types for weapons, and damage resistance types for items. This crystal is replaceable.
	int					m_basicStat1;			// weapons: weaponStat1_t. sabers: saberStat1_t. items: itemStat_t
	int					m_basicStat2;			// weapons: weaponStat2_t. sabers: saberStat2_t. items: itemStat_t
	int					m_basicStat3;			// weapons: weaponStat3_t. sabers: saberStat3_t. items: itemStat_t
	float				m_basicStat1value;		// m_basicStat1 strength multiplier
	float				m_basicStat2value;		// m_basicStat2 strength multiplier
	float				m_basicStat3value;		// m_basicStat3 strength multiplier

	// Installed module modifier stats... The item can support up to 3 extra modules.
	int					m_modStat1;				// weapons: weaponStat1_t. sabers: saberStat1_t. items: itemStat_t
	int					m_modStat2;				// weapons: weaponStat2_t. sabers: saberStat2_t. items: itemStat_t
	int					m_modStat3;				// weapons: weaponStat3_t. sabers: saberStat3_t. items: itemStat_t
	float				m_modStatValue1;		// m_modStat1 strength multiplier
	float				m_modStatValue2;		// m_modStat2 strength multiplier
	float				m_modStatValue3;		// m_modStat3 strength multiplier

	//
	// Private Internal Functions...
	//
	void setItemID(int itemID);
	void setTotalCost();

public:
	//
	// Construction/Destruction...
	//
	inventoryItem(); // default constructor member variables are 0 or NULL.
	inventoryItem(int, itemQuality_t, int, int); // paramterized constructor
	~inventoryItem(); // destructor

	qboolean			m_transmitted;

	//
	// Item Setup Functions...
	//
	void setBaseItem(int);
	void setQuality(itemQuality_t);
	void setQuantity(int);
	void setPlayerID(int);
	void setDestroyTime(int);

	// Base item modifier stats...
	void setStat1(int, float);
	void setStat2(int, float);
	void setStat3(int, float);

	// Installed module modifier stats...
	void setCrystal(int);
	void setMod1(int, float);
	void setMod2(int, float);
	void setMod3(int, float);
	
	//
	// Item Accessing Functions...
	//
	gitem_t	*getBaseItem();
	int getBaseItemID();
	itemQuality_t getQuality();
	int getQuantity();
	double getCost();
	double getTotalCost();
	int getPlayerID();
	qhandle_t getIcon();
	int getDestroyTime();

	qboolean isModification();
	qboolean isCrystal();

	// Base item modifier stats...
	int getBasicStat1();
	int getBasicStat2();
	int getBasicStat3();
	float getBasicStat1Value();
	float getBasicStat2Value();
	float getBasicStat3Value();

	// Installed module modifier stats...
	int getCrystal();
	int getMod1Stat();
	int getMod2Stat();
	int getMod3Stat();
	float getMod1Value();
	float getMod2Value();
	float getMod3Value();

	//
	// Database functions...
	//
#ifdef _GAME
	void storeToDatabase(inventoryItem*);
	void storeToDatabase(int, int);
	void loadFromDatabase(int, int);
	void removeFromDatabase(inventoryItem*);
#endif

	//
	// TODO: Upload items to clients...
	//
#ifdef _GAME
	void uploadToClient(int, inventoryItem*);
	void uploadFullInventoryToClient(int);
#endif

	//
	// TODO: Recieve new items from server...
	//
#ifdef _CGAME
	void downloadFromServer(char *packet);
#endif
};

//
// Global Stuff...
//
#ifdef _GAME
extern int				INVENTORY_ITEM_INSTANCES_COUNT;
extern inventoryItem	*INVENTORY_ITEM_INSTANCES[8388608];
#endif

#ifdef _CGAME
inventoryItem *createInventoryItem(int bgItemID, itemQuality_t quality, int amount = 1);
#endif

#ifdef _GAME
inventoryItem *createInventoryItem(int bgItemID, itemQuality_t quality, int amount = 1, int clientNum = -1, int destroyTime = -1);
void destroyOldLootItems(void);
#endif

void destroyInventoryItem(inventoryItem *item);

#endif
