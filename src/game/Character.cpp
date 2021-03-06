#include "Character.h"
#include "../common/Common.h"
#include "StatusEffect.h"
#include "../tools/Utility.h"
#include "GameObject.h"
#include "Inventory.h"
#include "Behavior.h"
#include "Equipment.h"
#include "ResourceNode.h"
#include "Statistics.h"
#include "StatusEffect.h"
#include "../common/Tile.h"	
#include "ItemManifest.h"
#include "Squad.h"

#include "../tools/Algorithm.h"

#include <cmath>
#include <tuple>
#include <random>

Character::Character(const std::string& _name, int capacity, std::tuple<float,float> loc): name(_name), GameObject(OBJ_CHARACTER) {

	float ox,oy;
	decompose(loc,ox,oy);

	this->x = ox;
	this->y = oy;

	this->setLocation(x,y);
	
	this->stats = new StatSet();
	this->init_stats();
	
	this->equipment = new Equipment();

	this->setStatus(STATUS_IDLE);
	this->target = nullptr;

	if(false) {
		this->maxHealth = 40;
		this->attackRate -= 20;
		TBAGame->playerChar = this;
		this->maxMoveSpeed = .5;
		this->displayID=3;
		this->traction = 2;
	} else {
		//Random movespeeds
		this->maxMoveSpeed = .3+(.1*((rand()%2)-1)); //10+-5
		this->displayID=6;
		this->traction = 1;
	}
	this->health = this->maxHealth;
	this->inventory = new Inventory(capacity);
	
	for(int i=0;i<10;i++) {
		// this->inventory->add(1+rand()%(itemManifest.size()-1));
	}

	this->evaluateEquipment();
	
	this->checkAI = TBA_Interval(this->AICheckInterval + (-100+(rand()%200)));
	
	this->addGoal(GOAL_IDLE);
	
	this->complete = true;
}

Character::Character(const std::string& _name, int capacity, std::tuple<float,float> loc,std::vector<int> inv): Character(_name,capacity,loc) {
	this->giveItems(inv);
	this->evaluateEquipment();
}

//==========
//	MOVEMENT
//==========
// Simulate collision by stopping movement in the direction of the collision
void Character::resolveMove(float &newX, float &newY) {
	if(newX+this->width >= TBAGame->gameWorld->size) {
		newX = this->x;
	}
	if(newY+this->width >= TBAGame->gameWorld->size) {
		newY = this->y;
	}
	
	bool YP = TBAGame->gameWorld->getTileAt(this->x,newY+this->width)->isPassable();
	bool YN = TBAGame->gameWorld->getTileAt(this->x,newY-this->width)->isPassable();
	bool XP = TBAGame->gameWorld->getTileAt(newX+this->width,this->y)->isPassable();
	bool XN = TBAGame->gameWorld->getTileAt(newX-this->width,this->y)->isPassable();
	
	if(!YP) {
		if(newY > this->y) newY = this->y;
	}
	if(!YN) {
		if(newY < this->y) newY = this->y;
	}
	if(!XP) {
		if(newX > this->x) newX = this->x;
	}
	if(!XN) {
		if(newX < this->x) newX = this->x;
	}

}
// Set character location and occupy appropriate tile
void Character::setLocation(float newX,float newY) {
	Tile* newTile = TBAGame->gameWorld->getTileAt(newX,newY);
	if((char*)this->location != (char*)newTile) {
		if(this->location == nullptr) {
			this->location = newTile;
			this->location->occupyWith(this);
		} else {
			this->location->evict(this);
			this->location = newTile;
			this->location->occupyWith(this);
		}
	}
	this->x = newX;
	this->y = newY;
}
// (Update method) move character based on aimAngle 
void Character::move() {

	float newX = this->x+(std::cos(this->viewAng*CONV_DEGREES)*((float)this->velocity*(TBAGame->logicTicks - this->lastMove)*TBAGame->moveSpeedUnit));
	float newY = this->y+(std::sin(this->viewAng*CONV_DEGREES)*((float)this->velocity*(TBAGame->logicTicks - this->lastMove)*TBAGame->moveSpeedUnit));
	if(this->autoMove) {
		this->velocity += this->maxMoveSpeed;
	} else {
		if(this->move_forward) this->velocity += this->maxMoveSpeed;
		if(this->move_back) this->velocity -= this->maxMoveSpeed;
	}

	//If character is trying to move into a new space
	this->resolveMove(newX,newY);

	// this->x = newX;
	// this->y = newY;
	
	this->setLocation(newX,newY);
	
	//Velocity decay
	this->velocity *= .9;
	
	if(fabs(this->velocity) < 0.5f) {
		this->removeStatus(STATUS_TRAVEL);
	}

	this->lastMove = TBAGame->logicTicks;

}
bool Character::canReach(GameObject *o) {
	float ox,oy;
	decompose(o->getLocation(),ox,oy);
	if(this->hasPath()) {
		Tile* t1 = TBAGame->gameWorld->getTileAt(ox,oy);
		//If this character already has a path
		// that reaches the target object's location, or a tile adjacent to it
		if((char*)this->targetPath.at(this->targetPath.size()-1) == (char*)t1) {
			// debug("OPTIMIZED");
			return true;
		}
		if(this->targetPath.at(this->targetPath.size()-1)->adjacent(t1)) {
			// debug("OPTIMIZED");
			return true;
		}
	}
	return characterCanReach(this,ox,oy,true);
}
bool Character::canSee(GameObject *o) {
	return TBAGame->gameWorld->hasSimplePath(this,o);
}
//===================
//	  EQUIPMENT / STATS
//===================
//Determine if character has a primary weapon
bool Character::isUnarmed() {
	return (!this->equipment->hasEquipped(EQUIP_PRIMARY) and !this->equipment->hasEquipped(EQUIP_SECONDARY));
}
// Determine character attack damage based on equipment
int Character::getAttackDamage() {
	if(this->isUnarmed()) {
		return this->defaultAttackDamage;
	}
	return this->equipment->getSlot(EQUIP_PRIMARY)->getAttribute(ATTRIB_DAMAGE);
}
// Determine character attack range based on equipment and perhaps stats
float Character::getAttackRange() {
	if(this->isUnarmed()) {
		return this->defaultAttackRange;
	}
	return this->equipment->getSlot(EQUIP_PRIMARY)->getAttribute(ATTRIB_RANGE);
}
// Get string of inventory contents
std::string Character::getInvString() {
	return this->inventory->toString();
}
// Equip item if applicable
bool Character::equip(Item *item) {
	//Item is a weapon, placeholder for secondary equipping
	if(item->hasType(I_WEAPON)) {
		this->equipment->equip(item,EQUIP_PRIMARY);
		return true;
	}
	if(item->hasType(I_ARMOR)) {
		if(item->hasType(I_ARMOR_HEAD)) {
			this->equipment->equip(item,EQUIP_HEAD);
		} else if(item->hasType(I_ARMOR_BODY)) {
			this->equipment->equip(item,EQUIP_BODY);
		} else if(item->hasType(I_ARMOR_LEGS)) {
			this->equipment->equip(item,EQUIP_LEGS);
		} else if(item->hasType(I_ARMOR_FEET)) {
			this->equipment->equip(item,EQUIP_FEET);
		}
		return true;
	}
	return false;
}

//===================
//	  		INVENTORY
//===================

bool Character::giveItem(int id) {
	this->evaluateEquipment();
	return this->inventory->add(id);
}
bool Character::giveItem(Item* item) {
	this->evaluateEquipment();
	return this->inventory->add(item);
}
bool Character::giveItems(std::vector<Item*> items) {
	this->evaluateEquipment();
	return this->inventory->add(items);
}
bool Character::giveItems(std::vector<int> ids) {
	this->evaluateEquipment();
	return this->inventory->add(ids);
}


//===================
//	   STATUS EFFECTS
//===================

// Add status effects determined by action to character
void Character::triggerItemEffects(Item* item,Action action) {
	std::vector<StatusEffect*> effectsOnAction = item->getEffectsOnAction(action);
	for(int i=0;i<effectsOnAction.size();i++) {
		this->effects.push_back(effectsOnAction.at(i));
		// debug("Added effect to "+this->getName()+" with magnitude "+std::to_string(effectsOnAction.at(i)->magnitude));
	}
}
// Process all status effects currently applied to character
void Character::processEffects() {
	for(int i=0;i<this->effects.size();i++) {
		//Apply effects or remove if inactive
		if(!this->effects.at(i)->applyEffect(this)) {
			this->effects.erase(this->effects.begin()+i);
		}
	}
	this->checkBody();
}

//===========
//	  ACTIONS
//===========
//Plant seed
bool Character::plant(Item *item) {
	//Item is a weapon, placeholder for secondary equipping
	if(item->hasType(I_PLANTABLE)) {
		this->location->planted = true;
		this->inventory->remove(item);
		return true;
	}
	return false;
}
// Consume item
bool Character::consume(Item *item) {
	//Trigger various effects from item on this character
	if(item->hasType(I_CONSUMABLE)) {
		this->triggerItemEffects(item,ACTION_CONSUME);
		this->inventory->remove(item);
		return true;
	}
	return false;
}
// Try to work resource or object
bool Character::work(GameObject* node) {
	if(node->type == OBJ_INTERACTIVE) {
		// this->addStatus(STATUS_WTORK);
		this->workTarget = node;
		this->addGoal(GOAL_WORK);
		return true;
	} else {
		return false;
	}
}
// Carry out work if character is working
void Character::processWork() {
	//If work target is out of range, go to it
	if(!this->workTargetInRange()) {
		if(!this->goTo(this->getWorkTarget(),true)) {
			debug("Error: "+this->getEntityName()+" cannot reach work target");
		}
	} else {
		// debug(this->getEntityName()+" is in working range");
		//Otherwise start working
		this->targetPath.clear();
		this->removeStatus(STATUS_TRAVEL);
		this->autoMove = false;
		this->move_forward = false;
		if(!this->hasStatus(STATUS_COMBAT)) {
			this->workTarget->getAsResource()->work();
		}
	}
}

//===========
//	  	SQUAD
//===========
//Try to add character to this character's squad
bool Character::addToSquad(Character *c,bool createNew) {
	if(this->hasSquad()) {
		this->squad->add(c);
		return true;
	} else if(createNew) {
		this->squad = TBAGame->createSquad();
		this->squad->add(this);
		this->squad->add(c);
		return true;
	}
	return false;
}

//=============
// MISC / CLEANUP
//==============
GameObject* Character::selectClosestObject(const std::vector<GameObject*> &v) {
	float cdist = 0xFFFF;
	GameObject *closest = nullptr;
	for(int i=0;i<v.size();i++) {
		if(dist(this->getLocation(),v.at(i)->getLocation()) < cdist) {
			cdist = dist(this->getLocation(),v.at(i)->getLocation());
			closest = v.at(i);
		}
	}
	if(closest == nullptr) debug("ERROR: (Character::selectClosestObject()): Returning nullptr");
	return closest;
}
//Get absolute, on-screen location of character tile
std::tuple<float,float> Character::getApproximateLocation() {

	// float x = TBAGame->gameWindow->mapScreen->x+(TBAGame->gameWindow->mapScreen->w/2)-(TBAGame->gameWindow->mapScreen->charW*(TBAGame->playerChar->location->x - this->location->x));
	// float y = TBAGame->gameWindow->mapScreen->y+(TBAGame->gameWindow->mapScreen->h/2)-(TBAGame->gameWindow->mapScreen->charH*(TBAGame->playerChar->location->y + this->location->y));

	return {0,0};

}
// Get absolute, on-screen location of character within tile
std::tuple<float,float> Character::getAbsoluteLocation() {

	// float x = TBAGame->gameWindow->mapScreen->x+(TBAGame->gameWindow->mapScreen->w/2)-(TBAGame->gameWindow->mapScreen->charW*(TBAGame->playerChar->x - this->x));
	// float y = TBAGame->gameWindow->mapScreen->y+(TBAGame->gameWindow->mapScreen->h/2)-(TBAGame->gameWindow->mapScreen->charH*(TBAGame->playerChar->y - this->y));

	return {0,0};

}
// Retrieve list of statuses currently active on character
std::string Character::getStatusString() {
	//std::string statusString;
	std::vector<std::string> statuses;
	//return statusString;
	flag statFlag;
	for(int i=0;i<log2((flag)STATUS_END);i++) {
		statFlag = 1 << i;
		if(this->hasStatus((StatusIndicator)statFlag)) {
			statuses.push_back(statusMap.at((StatusIndicator)statFlag));
		}
	}
	if(statuses.size() == 0) {
		return "";
	}
	return "\t"+join("\n\t...\t",statuses);
	//return statusString;
}
//Determine if character is player
bool Character::isPlayer() {
	return (char*)TBAGame->playerChar == (char*)this;
}
// Get detailed info about character
std::string Character::getInfo() {
	
	bool debug = true;
	
	std::string info = " Name:\t"+this->getEntityName() + "\n" +
				"\tStatus:"+this->getStatusString() + "\n" ;
				// "\tAim Angle:"+std::to_string(this->viewAng) + "\n" +
				// "\tTarget Angle:"+std::to_string(this->targetAng) + "\n" ;
				// "\tLocation:\t"+std::to_string((int)std::round(this->x)) + "," + std::to_string((int)std::round(this->y)) + "\n";
				if(this->hasTarget()) {
					info += "\tTarget:\t"+this->getTarget()->getEntityName() + "\n";
					// info += "\tDistance to target:\t"+std::to_string(dist(this->getLocation(),this->getTarget()->getLocation())) + "\n";
					
				}
				//"\tVelcocity:\t"+std::to_string(this->velocityX) + "," + std::to_string(this->velocityY) + "\n" +
				// info += "\tHealth:\t"+std::to_string(this->health) + "/" + std::to_string(this->maxHealth) + "\n\n" + 
				// info += "\n\n"+ //"\tAttack rate: "+std::to_string(this->attackRate) + "\n\n" +
				
				if(debug) {
					// Pathing info
					std::string pathing = (this->simple)? "simple" : "complex";
					info += "Using "+pathing+ " pathing\n";
					info += "Current path:";
					for(int i=0;i<this->targetPath.size();i++) {
						info += "\t"+std::to_string(this->targetPath.at(i)->x)+", "+std::to_string(this->targetPath.at(i)->y);
					}
					info += "\n";
					
					info += std::to_string(this->viewAng)+"\n";
					if(this->hasGoals()) {
						std::vector<GoalType> t = this->goals.get_elements();
						for(int i=0;i<t.size();i++) {
							info += std::to_string(t.at(i))+"\n";
						}
					}
				}
				
				//Limb info
				info += this->body->getInfo();
					
				if(this->hasEffects()) {
					info += "\n\tStatus Effects:";
					for(int i=0;i<this->effects.size();i++) {
						info += "\n "+this->effects.at(i)->getTypeString()+" ("+this->effects.at(i)->getDurationString()+")";
					}
					info += "\n";
				}
				if(this->hasSquad()) {
					info += "\n\n\tSquad:\t";
					info += join("\t   \t",this->squad->getMemberNames());
				}
	return info;
}
// Kill this character
void Character::kill() {
	if(this->health > 0) { this->health = 0;}
	this->move_forward = false;
	this->move_back = false;
	this->targetPath.clear();
	//a warrior's death
	//this->target = nullptr;
	// this->removeStatus((StatusIndicator)0xFFFFFF);
	// debug("Killing "+this->getName());
	this->setStatus(STATUS_DEAD);
}
// Delete this character, remove from game
void Character::cleanup() {
	this->location->evict(this);
	this->target = nullptr;
	this->location = nullptr;
	delete this->inventory;
	if(this->isPlayer()) {
		new Character("Generic",160,{0,0});
	}
	TBAGame->removeObject(this);
	delete this;
}