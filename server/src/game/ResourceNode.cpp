#include "ResourceNode.h"

std::string ResourceNode::serialize() {
	std::string node;
	//	00000000 00 00 000|c1|c2|c3|c4|...
	std::string type = std::to_string((int)this->type);
	std::string id = std::to_string(this->objectID);
	std::string display = std::to_string(this->getDisplayID());
	std::string name = this->getName();
	std::string x = std::to_string((int)(this->x*100));
	std::string y = std::to_string((int)(this->y*100));
	//ID
	pad(type,'0',PAD_SHORT);
	pad(display,'0',PAD_SHORT);
	pad(id,'0',PAD_INT);
	pad(name,' ',PAD_STR);
	pad(x,'0',PAD_FLOAT);
	pad(y,'0',PAD_FLOAT);
	
	node = type+display+id+name+x+y;
	
	return node;
}

std::string ResourceNode::getInfo() {
		return " Name:\t"+this->name + "\n" +
				"\tLocation:\t"+std::to_string((int)std::round(this->x)) + "," + std::to_string((int)std::round(this->y)) + "\n" +
				"\tResource:\t"+Item(this->primaryResource).getFormattedName()+"\n"+
				"\tProgress:\t"+std::to_string((int)(100*(this->progress/this->duration)))+"%\n"+
				"\tContents:"+this->inventory->contentString;
	}