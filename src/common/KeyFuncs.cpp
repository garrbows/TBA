#include "KeyFuncs.h"

#include "../common/Common.h"
#include "../game/Behavior.h"
#include "../game/Projectile.h"
#include "../game/FloatingText.h"
#include "../tools/Utility.h"
#include "../game/Input.h"
#include "../game/ResourceNode.h"
#include "../game/Squad.h"

#include "../game/AI.h"

#include <SDL2/SDL.h>
#include <tuple>

bool moving = false;
int activeScreen = 1;
int lastMove[2];
int lastHighlight[2];

void processKeystroke(int keycode) {
	if(moving) {
		switch(keycode) {
			case 114:
				resetScreen(activeScreen);
				moving = false;
				return;
			case 82: 
				resetAllScreens();
				moving = false;
				return;
		}
		
	}
	char c = keycode;
	TBAGame->gameWindow->textScreen->commandAppend(c);
}

//==========
//  COMMAND
//==========

void sendCommand() {
	TBAGame->gameWindow->textScreen->sendCommand();
}

void swapCommand(int i) {
	TBAGame->gameWindow->textScreen->swapCommand(i);
}

void delChar() {
	//Pop back removes last entry in data structure
	TBAGame->gameWindow->textScreen->deleteLastCharacter();
}

//==========
//	  MACROS
//==========

void autocomplete() {
	if(autocomplete(TBAGame->gameWindow->textScreen->command,TBAGame->commandStrings)) {
		if(contains(TBAGame->commandStrings,TBAGame->gameWindow->textScreen->command)) {
			TBAGame->gameWindow->textScreen->commandAppend(' ');
		}
	} else {
		//TBAGame->gameWindow->textScreen->setCommand(TBAGame->gameWindow->textScreen->command);
		TBAGame->gameWindow->textScreen->setCommandLine();
	}
}

void pause() {
	
	TBAGame->togglePause();

}

void clear() {
	TBAGame->gameWindow->textScreen->content.clear();
}

void clearCommand() {
	TBAGame->gameWindow->textScreen->setCommand("");
}

void paste() {
	std::string clipboard(SDL_GetClipboardText());
	// for(long i=0;i<92;i++) {
		// if(i != 27) TBAGame->gameWindow->textScreen->commandAppend((char)i);
	// }
	// debug((char)244);
	TBAGame->gameWindow->textScreen->commandAppend(clipboard);
}

//==========
//	  MOUSE
//==========

void click(SDL_MouseButtonEvent& event) {
	
	if(TBAGame->playerChar == nullptr) return;


	int centerX = 5+TBAGame->gameWindow->mapScreen->x+(TBAGame->gameWindow->mapScreen->w/2)-(TBAGame->gameWindow->mapScreen->charW*(TBAGame->playerChar->x));
	int centerY = 5+TBAGame->gameWindow->mapScreen->y+(TBAGame->gameWindow->mapScreen->h/2)-(TBAGame->gameWindow->mapScreen->charH*(TBAGame->playerChar->y));

	int tileX = std::round(((float)(event.x-centerX))/TBAGame->gameWindow->mapScreen->charW/TBAGame->gameWindow->mapScreen->zoom);
	int tileY = std::round(((float)(event.y-centerY))/TBAGame->gameWindow->mapScreen->charW/TBAGame->gameWindow->mapScreen->zoom);

	// std::cout << tileX << ", " << tileY << std::endl;

	Tile* thisTile = TBAGame->gameWorld->getTileAt(tileX,tileY);

	//Left click
	if(event.button == SDL_BUTTON_LEFT) {
		for(int i=TBAGame->gameWindow->screenVector.size()-1;i>=0;i--) {
			if(TBAGame->gameWindow->screenVector.at(i)->enclose(event.x,event.y)) {
				moving=true;
				activeScreen = TBAGame->gameWindow->screenVector.size()-1;
				TBAGame->gameWindow->bringToFront(TBAGame->gameWindow->screenVector.at(i));
				lastMove[0] = event.x;
				lastMove[1] = event.y;
				return;
			}
		}
		if(TBAGame->gameWindow->mapScreen->enclose(event.x,event.y)) {

			GameObject* testTarget = nullptr;
			
			//Temp options before removing invalid objects like projectiles
			std::vector<GameObject*> temp_options = thisTile->objects;
			for(int i=0;i<thisTile->occupiers.size();i++) {
				temp_options.push_back(thisTile->occupiers.at(i));
			}
			
			
			std::vector<GameObject*> options;
			for(int i=0;i<temp_options.size();i++) {
				if(temp_options.at(i)->type != OBJ_PROJECTILE) {
					options.push_back(temp_options.at(i));
				}
			}
			
			float tx,ty;
	
			if(options.size() > 0) {
				testTarget = options.at(0);
				if(TBAGame->hasDisplayTarget()) {
					decompose(TBAGame->displayTarget->getLocation(),tx,ty);
					if((char*)TBAGame->gameWorld->getTileAt(tx,ty) == (char*)thisTile) {
						testTarget = TBAGame->displayTarget;
					}
				}
				if((char*)TBAGame->displayTarget == (char*)testTarget) {
					int index = find(TBAGame->displayTarget,options);
					if(index == options.size()-1) {
						testTarget = options.at(0);
					} else {
						testTarget = options.at(index+1);
					}
		
					// testTarget = thisTile->getNextOccupant(static_cast<Character*>(testTarget)));
				}
				TBAGame->displayTarget = testTarget;
			}
		}
	//Presumably right click
	} else {
		// pthread_mutex_lock(&TBAGame->updateLock);
		if(thisTile->hasBlocks()) {
			thisTile->blocks.clear();
		} else {
			thisTile->addBlock(4);
		}
		// pthread_mutex_unlock(&TBAGame->updateLock);
	}

}

void release() {
	moving = false;
}

//==========
//	  SCREEN
//==========

void move(SDL_Event& event) {
	
	if(TBAGame->playerChar == nullptr) return;

	if(moving) {
		TBAGame->gameWindow->screenVector.at(activeScreen)->x += event.motion.x-lastMove[0];
		TBAGame->gameWindow->screenVector.at(activeScreen)->y += event.motion.y-lastMove[1];
	}

	lastMove[0] = event.motion.x;
	lastMove[1] = event.motion.y;

	int centerX = 5+TBAGame->gameWindow->mapScreen->x+(TBAGame->gameWindow->mapScreen->w/2)-(TBAGame->gameWindow->mapScreen->charW*(TBAGame->playerChar->x));
	int centerY = 5+TBAGame->gameWindow->mapScreen->y+(TBAGame->gameWindow->mapScreen->h/2)-(TBAGame->gameWindow->mapScreen->charH*(TBAGame->playerChar->y));

	int tileX = std::round(((float)(event.motion.x-centerX))/TBAGame->gameWindow->mapScreen->charW/TBAGame->gameWindow->mapScreen->zoom);
	int tileY = std::round(((float)(event.motion.y-centerY))/TBAGame->gameWindow->mapScreen->charW/TBAGame->gameWindow->mapScreen->zoom);

	TBAGame->gameWorld->getTileAt(lastHighlight[0],lastHighlight[1])->highlight = false;
	TBAGame->gameWorld->getTileAt(lastHighlight[0],lastHighlight[1])->needsUpdate = true;
	TBAGame->gameWorld->getTileAt(tileX,tileY)->highlight = true;

	lastHighlight[0] = tileX;
	lastHighlight[1] = tileY;
}

int getTopScreen(int x,int y) {
	int topScreenID = -1;
	for(int i=0;i<TBAGame->gameWindow->screenVector.size();i++) {
		if(TBAGame->gameWindow->screenVector.at(i)->enclose(x,y)) {
			topScreenID = i;
		}
	}
	return topScreenID;
}

void shiftContentWindow(int i) {
	
	//Get the topmost screen that the mouse is hovering over and scroll it
	int topScreenID = getTopScreen(lastMove[0],lastMove[1]);
	if(topScreenID < 0) return;
	TBAGame->gameWindow->screenVector.at(topScreenID)->shiftContentWindow(i);

}

void addContent(const std::string& s) {
	TBAGame->gameWindow->textScreen->addContent(s);
}

void resetScreen(int screenPriority) {
	TBAGame->gameWindow->screenVector.at(screenPriority)->x = TBAGame->gameWindow->screenVector.at(screenPriority)->defaultX;
	TBAGame->gameWindow->screenVector.at(screenPriority)->y = TBAGame->gameWindow->screenVector.at(screenPriority)->defaultY;
}

void resetAllScreens() {
	int screenCount = TBAGame->gameWindow->screenVector.size();
	for(int i=0;i<screenCount;i++) {
		resetScreen(i);
	}
}

//====================
//		PLAYER STATUS_TRAVELMENT
//====================

void move(bool m_forward,bool m_back) {
	
	if(TBAGame->playerChar == nullptr) return;

	if(m_forward ^ m_back) {
		if(TBAGame->playerChar->isAlive()) {
			TBAGame->playerChar->addStatus(STATUS_TRAVEL);
			TBAGame->playerChar->autoMove = false;
		}
	}

	if(TBAGame->playerChar->isAlive()) {
		TBAGame->playerChar->move_forward = m_forward;
		TBAGame->playerChar->move_back = m_back;
	}
}

void turn(bool turn_left,bool turn_right) {

	if(TBAGame->playerChar == nullptr) return;

	if(turn_left) {
		TBAGame->playerChar->autoMove = false;
		TBAGame->playerChar->viewAng -= 2;
		if(TBAGame->playerChar->viewAng < 0) TBAGame->playerChar->viewAng = 360+TBAGame->playerChar->viewAng;
		TBAGame->playerChar->viewAng = (int)TBAGame->playerChar->viewAng%360;
		TBAGame->playerChar->targetAng = TBAGame->playerChar->viewAng;
	}
	if(turn_right) {
		TBAGame->playerChar->autoMove = false;
		TBAGame->playerChar->viewAng += 2;
		if(TBAGame->playerChar->viewAng < 0) TBAGame->playerChar->viewAng = 360+TBAGame->playerChar->viewAng;
		TBAGame->playerChar->viewAng = (int)TBAGame->playerChar->viewAng%360;
		TBAGame->playerChar->targetAng = TBAGame->playerChar->viewAng;
	}
}


//==========
//		MISC
//==========
int alpha = -1;

void debugKey() { //kp_plus

	//SDL_Delay(100);

	//Delete oldest object
	/*TBAGame->gameObjects.at(0)->cleanup();
	TBAGame->gameObjects.erase(TBAGame->gameObjects.begin());
	TBAGame->gameObjects.shrink_to_fit();*/

	//TBAGame->popupText(1,"Building is too close to existing settlement");


	//TBAGame->playerChar->setStatus(STATUS_ATTACK);

	//Check zoom
	//TBAGame->gameWindow->mapScreen->charW-=2;
	//TBAGame->gameWindow->mapScreen->charH-=2;

	//Increase movespeed
	//TBAGame->playerChar->maxMoveSpeed++;
	//debug(TBAGame->playerChar->maxMoveSpeed);

	//Simulate physics push
	//TBAGame->playerChar->move({200,150});

	//TBAGame->playerChar->isPlayer = false;
	//TBAGame->playerChar = static_cast<Character*>(TBAGame->gameObjects.at(1));
	//TBAGame->playerChar->isPlayer = true;

	// for(int i=0;i<10;i++) {
	// }

	//debug(TBAGame->gameObjects.size());
	//FloatingText* text = new FloatingText(1,{"TES"},{200,200});

	//TBAGame->playerChar->target->receiveAttack(0,TBAGame->playerChar);

	/*
	float x,y;
	decompose(TBAGame->playerChar->getLocation(),x,y);

	if(!TBAGame->playerChar->hasTarget()) {
		new Projectile(TBAGame->playerChar,{x+.2,y+.2},TBAGame->playerChar->viewAng*CONV_DEGREES,.5);
	} else {
		float tx,ty;
		decompose(TBAGame->playerChar->target->getLocation(),tx,ty);
		new Projectile(TBAGame->playerChar,{x,y},(-15+rand()%31)-atan2(ty-y,tx-x)*CONV_DEGREES,.5);
	}

	// static_cast<Character*>(TBAGame->displayTarget)->setTarget(TBAGame->playerChar);
	
	// TBAGame->setPlayer(static_cast<Character*>(TBAGame->displayTarget));
	*/

	// TBAGame->gameWindow->createPopup("Cannot place building here",TBAGame->convert(5000),false);
	
	// TBAGame->playerChar->moveTo(1,1);
	
	// static_cast<Character*>(TBAGame->displayTarget)->generatePathTo(TBAGame->playerChar->location->x,-TBAGame->playerChar->location->y);
	
	// TBAGame->logicTickRate /= 2;
	
	// GameObject *node = new ResourceNode("Rich Stone",{2.0f,2.0f},7,10,22);
	GameObject *node = TBAGame->gameWorld->getTileAt(1,1)->objects.at(0);
	Character *amelia;
	for(int i=0;i<1;i++) {
		amelia = new Character("Amelia",8,{-rand()%10,-rand()%20});
		// amelia = new Character("Amelia",8,{-1,-1});
		// amelia->maxMoveSpeed = 4;
		// amelia->turnSpeed = 4;
		amelia->work(node);
	}
	// TBAGame->playerChar->addGoal(GOAL_HEAL_SELF);

	// amelia->goals.push(new Goal(GOAL_USE_RANGED));
	// debug(sizeof(Item);
	// Tile* x = TBAGame->gameWorld->getNearestValidAdjacentTile(amelia,TBAGame->gameWorld->getTileAt(1,1));
	// debug(x->x);
	// debug(-x->y);
	// debug("Added amelia to  "+std::to_string(amelia->x)+", "+std::to_string(amelia->y));
	// debug(TBAGame->gameWorld->isCorner(TBAGame->gameWorld->getTileAt(-4,-4),TBAGame->gameWorld->getTileAt(-3,-3)));
	// TBAGame->playerChar->addToSquad(amelia,true);
	
	// debug(TBAGame->playerChar->limbs.at(0)->health);

	// TBAGame->gameWindow->popupBox->setToggledContent("Paused");
	// TBAGame->gameWindow->popupBox->toggled = true;

	/*SDL_Texture* blockTexture = TBAGame->gameWorld->getTileAt(-2,-2)->blocks.at(TBAGame->gameWorld->getTileAt(-2,-2)->blocks.size()-1)->blockTexture;
	
	SDL_SetRenderTarget(TBAGame->gameWindow->renderer,blockTexture);

	fChar charInfo = TBAGame->gameWindow->mapScreen->screenFont->fontMap.at(TBAGame->gameWorld->getTileAt(-2,-2)->id);
	SDL_Rect sRect = {charInfo.x,charInfo.y,charInfo.w,charInfo.h};
	SDL_Rect dRect = {-3,-3,6,16};

	SDL_RenderCopyEx(TBAGame->gameWindow->renderer,TBAGame->gameWindow->mapScreen->screenFont->fontTexture,&sRect,&dRect,45,NULL,SDL_FLIP_NONE);

	SDL_SetRenderTarget(TBAGame->gameWindow->renderer,NULL);

	//TBAGame->graphicsTickRate = (++alpha%2 == 0)? 60 : 144;

	/*(std::vector<Tile*> tiles = TBAGame->gameWorld->getTilesInRadius(std::round(TBAGame->playerChar->x),std::round(TBAGame->playerChar->y),5);
	for(int i=0;i<tiles.size();i++) {
		tiles.at(i)->id = -1;
	}*/

	//SDL_SetTextureAlphaMod(TBAGame->gameWindow->mapScreen->screenFont->fontTexture,--alpha);
	//SDL_SetTextureColorMod(TBAGame->gameWindow->mapScreen->screenFont->fontTexture,255,alpha/2,alpha);
		

}
