#pragma once
#include <vector>
#include <map>
#include <utility>

#include "GameObject.h"
#include "Command.h"
#include "../tools/Utility.h"

std::map<std::string,std::pair<std::vector<std::string>,std::vector<std::string>>> helpMap = {
	//command,{ {usage1,usage2},{returns}}
	{"help",{{"help","help [command]"},{"Displays command list","Displays command's usage and function"}}},
	{"clear",{{"clear"},{"Clears screen"}}},
	{"inventory",{{"inventory"},{"Displays player inventory"}}},
	{"search",{{"search"},{"Displays inventory of selected container"}}},
	{"examine",{{"examine","examine [item]","examine [partial item]"},{"Displays information about surroundings","Displays item info"}}},
	{"equip",{{"equip [item]","equip [partial item]"},{"Equips item"}}},
	{"take",{{"take [item name]","take [partial name]"},{"Takes item from selected container"}}},
	{"put",{{"put [item name]","put [partial name]"},{"Puts item in selected container"}}},
	{"move",{{"move [direction]"},{"Moves player in direction"}}},
	{"pause",{{"pause"},{"Pauses game"}}},
	{"stop",{{"stop"},{"Halts player movement"}}},
	{"unpause",{{"unpause"},{"Unpauses game"}}},
	{"attack",{{"attack"},{"Engages player in combat with current target"}}},
	{"zoom",{{"zoom","zoom [in]","zoom [out]"},{"Changes or resets screen zoom level"}}},
	{"say",{{"say [text]"},{"Creates player speech"}}},
	{"exit",{{"exit"},{"Exits game"}}},
	{"select",{{"select [object]","select [character]"},{"Selects entity"}}},
	{"target",{{"target [character]"},{"Targets character"}}},
};

int moveItems(int itemCount,Item *goodItem,Inventory *source, Inventory *destination) {
	int taken = 0;
	int index = source->find(goodItem->name);

	for(taken;taken<itemCount;taken++) {
		goodItem = source->remove(index);
		destination->add(goodItem);
		index = source->find(goodItem->name);
		if(index == -1) {
			taken++;
			break;
		}
	}
	return taken;
}

int findItemSource(std::string itemName,Inventory* &inv,std::string &sourceName) {

	Container *testContainer;

	int index = -1;

	//Search containers in area
	std::vector<GameObject*> containers = TBAGame->playerChar->getObjectsInRadius(OBJ_CONTAINER);

	for(int i=0;i<containers.size();i++) {
		testContainer = static_cast<Container*>(containers.at(i));
		index = testContainer->inventory->find(itemName);
		if(index >= 0) {
			inv = testContainer->inventory;
			sourceName = testContainer->getName();
		} else {
			inv = nullptr;
		}
	}
	return index;
}

std::string parseInteraction(Command *cmd, std::vector<std::string> args) {

	//Get command basename
	std::string command = cmd->aliases.at(0);

	//Set default itemcount
	int itemCount = 1;
	bool takeAll = false;
	std::string itemName;

	//Remove empty strings from args
	args = strip(args);

	//determine itemcount
	for(int i=0;i<args.size();i++) {
		if(isdigit(args.at(i))) {
			itemCount = std::stoi(strip(args.at(i)));
			args.erase(args.begin() + i);
			break;
		} else if(args.at(i) == "*") {
			takeAll = true;
			args.erase(args.begin() + i);
			break;
		}
	}

	if(args.size() == 0) return "\nNo item specified";

	//Join everything between [command,itemcount] into item name 
	itemName = join(' ',args);

	Inventory *source;
	Inventory *destination;

	//std::string sourceName;
	std::string targetName;
	std::string tense,preposition;

	if(command == "take") {

		tense = "Took";
		preposition = "from";

		targetName = TBAGame->displayTarget->getName();

		//Source and destination containers
		// source is errorchecked and is a valid container

		source = TBAGame->displayTarget->getInventory();
		destination = TBAGame->playerChar->inventory;

	} else if(command == "put") {

		//sourceName = "inventory";
		targetName = TBAGame->displayTarget->getName();

		//Source and destination containers
		// source is errorchecked and is a valid container
		source = TBAGame->playerChar->inventory;
		destination = TBAGame->displayTarget->getInventory();

		if(TBAGame->displayTarget->type == OBJ_CHARACTER) {
			tense = "Gave";
			preposition = "to";
		} else {
			tense = "Put";
			preposition = "in";
		}
	}

	//Check if item appears in selected container	
	int index = source->find(itemName);

	if(index >= 0) {
		//itemName was found in a container
		Item *goodItem;
		goodItem = source->getItem(index);
		if(takeAll) itemCount = source->itemCount(goodItem->id);
		int taken = moveItems(itemCount,goodItem,source,destination);
		if(taken > 1) return "\n"+tense+" "+goodItem->name + " ("+std::to_string(taken)+")"+" "+preposition+" "+targetName;
		return "\n"+tense+" "+goodItem->name+" "+preposition+" "+targetName;
	} else if(index == -2) {
		//itemName was found in a container but with multiple matches
		return "";
	} else {
		//itemName was not found in any containers
		return "\nItem not found in "+targetName;
	}
}