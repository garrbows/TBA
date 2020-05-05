#pragma once
#include "../tools/Utility.h"
#include "Inventory.h"
#include <string>
#include <utility>
#include <tuple>

class Character;
class Projectile;
class ResourceNode;
class Container;

enum objType {
	OBJ_GENERIC,
	OBJ_CHARACTER,
	OBJ_BLOCK,
	OBJ_PROJECTILE,
	OBJ_CONTAINER,
	OBJ_INTERACTIVE,
	UI_FLOATINGTEXT
};

class GameObject {

	public:

		int objectID;
		objType type = OBJ_GENERIC;
	
		Character* getAsCharacter();
		Projectile* getAsProjectile();
		ResourceNode* getAsResource();
		Container* getAsContainer();

		//Update function to be overridden for each object type
		virtual void update() {}
		virtual void cleanup(){}

		virtual std::string getName() {return "NAME_FUNC_UNDEFINED";}
		virtual std::string getFormattedName() {return "FORMATTED_NAME_FUNC_UNDEFINED";}
		virtual std::string getInfo() {return "INFO_FUNC_UNDEFINED";}
		virtual std::tuple<float,float> getLocation() {return {0,0};}
		virtual int getDisplayID() {return -1;}

		inline bool hasInventory() { return this->type == OBJ_CHARACTER or this->type == OBJ_CONTAINER or this->type == OBJ_INTERACTIVE;}
		Inventory* getInventory();
		std::string getInvString();

		GameObject(objType);

		~GameObject() {}

};