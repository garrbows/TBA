#include "Screen.h"	

#include "../game/Input.h"
#include "../common/Common.h"

//========================
//CONTENT DRAWING FUNCTIONS
//========================

void TextScreen::addContent(const std::string& str) {

	std::string s = str;

	if(this->content.size() == 0) {
		this->content.push_back("");
		if(s[0] == '\n') {
			s.erase(0,1);
		}
	}

	int indexOfLastSpace = -1;
	int tabStop = this->screenCharWidth/2;
	int cursorX;

	//Read last content line into a string and delete it from content vector
	std::string thisLine = this->content.at(this->content.size()-1);
	this->content.pop_back();

	//Iterate through added string
	for(int i=0;i<s.size();i++) {
		cursorX = thisLine.size()-1;
		if(thisLine.size() + 1 >= this->screenCharWidth) {
			//Push content back with current line and start new line
			if(s[i] == ' ') {
				//If last character in line is a space, start new line without the space
				this->content.push_back(thisLine);
				thisLine = "";
				indexOfLastSpace = -1;

			} else if(indexOfLastSpace < 0) {
				//If the line has no spaces, break word
				this->content.push_back(thisLine);
				thisLine = s[i];
				indexOfLastSpace = -1;
			} else {
				//If the last character is not a space and there are spaces in the line
				//Break line on the last space
				thisLine += s[i];
				this->content.push_back(thisLine.substr(0,indexOfLastSpace));
				thisLine = thisLine.substr(indexOfLastSpace+1,thisLine.size());
			}
		} else {
			if(s[i] == ' ') {
				//Record index of the last space on this line
				thisLine += ' ';
				indexOfLastSpace = thisLine.size()-1;
			} else if(s[i] == '') {
				//Non-breaking space
				//Add space but don't record as a breakpoint
				thisLine += ' ';
			} else if(s[i] == '\n') {
				//Create new line and drop newline character
				this->content.push_back(thisLine);
				thisLine = "";
				indexOfLastSpace = -1;
			}	else if(s[i] == '\t') {
					//Move "cursor" to next tabstop
					if(cursorX  <= 1) {
						thisLine += ' ';
					} else if(cursorX < tabStop) {
						while(cursorX != tabStop) {
							thisLine += ' ';
							cursorX = thisLine.size();
						}
					} else if(cursorX > tabStop) {
						this->content.push_back(thisLine);
						thisLine = " ";
						indexOfLastSpace = -1;
				}
			} else {
				//Character is a traditional character, add to line
				thisLine += s[i];
			}

		}
	}
	this->content.push_back(thisLine);
}

std::vector<std::string> TextScreen::prepareCommandForDisplay(const std::string& s) {
	
	//Convert string command line to string vector formatted for the display
	//Very similar to addContent except special characters (newline, tab)
	//Don't need to be accounted for

	std::vector<std::string> preparedLines = {""};
	int lineIndex = 0;
	std::string thisLine;

	for(int i=0;i<s.size();i++) {
	
		if(thisLine.size()+1 >= this->screenCharWidth) {

			preparedLines.at(lineIndex) = thisLine;
			preparedLines.push_back("");
			lineIndex++;
			thisLine = s[i];

		} else {
			thisLine += s[i];
		}
	}

	preparedLines.at(lineIndex) = thisLine;
	return preparedLines;

}

void TextScreen::addContent(char c) {

	std::string thisLine = this->content.at(this->content.size()-1);

	if(thisLine.size() + 1 >= this->screenCharWidth) {
		this->content.push_back("");
	}
	int lineIndex = this->content.size()-1;
	this->content.at(lineIndex) += c;

}

void TextScreen::setLastLine(const std::string& goodLine) {
}

void MapScreen::generateMapTiles() {
	int centerX = TBAGame->playerChar->x;
	int centerY = TBAGame->playerChar->y;

	this->map = TBAGame->gameWorld->getMapAt((this->w/this->charW)+2,centerX,centerY);
	this->lastMapX = centerX;
	this->lastMapY = centerY;
}

void MapScreen::generateMapTexture() {

	int centerX = TBAGame->playerChar->x;
	int centerY = TBAGame->playerChar->y;

	if(centerX != this->lastMapX or centerY != this->lastMapY) {
		this->generateMapTiles();
	}

	SDL_SetRenderTarget(TBAGame->gameWindow->renderer,this->mapTexture);

	//Source rectangle taken from screenFont->fontTexture
	SDL_Rect sRect;
	fChar charInfo;

	//Destination rectangle on screen
	SDL_Rect dRect;

	//Format raw content string for display
	//std::vector<std::vector<Tile*>> visibleContent = subVec(this->map,0,this->map.size());

	//visibleContent.push_back("->");
	Tile* thisTile;

	//Cursor
	int cursor[2] = {0,0};

	int tileID;
	int occupierTileID;
	int index = -1;

	//For each line of visible Content
	for(int i=0;i<map.size();i++) {
		for(int j=0;j<map.size();j++) {
			thisTile = this->map.at(i).at(j);
	
			//Get tile ID from tile and get tile texture from tileset

			tileID = thisTile->id;

			if(this->screenFont->fontMap.find(tileID) == this->screenFont->fontMap.end()) {
				debug("ERROR: Missing charMap entry for tile "+tileID);
				exit(0);
			}

			charInfo = this->screenFont->fontMap.at(tileID);

			//Read values from charInfo into SDL_Rect
			sRect = {charInfo.x,charInfo.y,charInfo.w,charInfo.h};

			//Modify display location rectangle based on cursor and font display values << FIX
			SDL_Rect dRect = {(1*cursor[0]),(this->charH*cursor[1])+charInfo.yo,this->charW,this->charH};

			//SDL_RenderCopy(TBAGame->gameWindow->renderer,this->screenFont->fontTexture,&sRect,&dRect);
			SDL_RenderCopyEx(TBAGame->gameWindow->renderer,this->screenFont->fontTexture,&sRect,&dRect,thisTile->rotation,NULL,thisTile->flip);

			if(thisTile->hasBlocks()) {
				
				
				if(this->screenFont->fontMap.find(tileID) == this->screenFont->fontMap.end()) {
					debug("ERROR: Missing charMap entry for tile "+tileID);
					exit(0);
				}

				tileID = thisTile->getDisplayID();

				charInfo = this->screenFont->fontMap.at(tileID);
				sRect = {charInfo.x,charInfo.y,charInfo.w,charInfo.h};

				// Modify alpha to give impression of lit interiors
				// Uint8 alpha;
				// SDL_GetTextureAlphaMod(this->screenFont->fontTexture,&alpha);
				// SDL_SetTextureAlphaMod(this->screenFont->fontTexture,255);
				SDL_RenderCopyEx(TBAGame->gameWindow->renderer,this->screenFont->fontTexture,&sRect,&dRect,thisTile->getRotation(),NULL,thisTile->getFlip());
				//SDL_SetTextureAlphaMod(this->screenFont->fontTexture,alpha);
			}

			if(thisTile->occupied) {
				//occupierTileID = thisTile->occupier->displayID;

				//charInfo = this->screenFont->fontMap.at(occupierTileID);
				charInfo = TBAGame->gameWindow->textScreen->screenFont->fontMap.at((int)thisTile->occupier->getName()[0]);

				//Read values from charInfo into SDL_Rect
				sRect = {charInfo.x,charInfo.y,charInfo.w,charInfo.h};

				//Modify display location rectangle based on cursor and font display values << FIX
				SDL_Rect dRect = {(1*cursor[0])+(charInfo.w/2)+2,(this->charH*cursor[1])+(charInfo.h/2),charInfo.w,charInfo.h};

				//SDL_RenderCopy(TBAGame->gameWindow->renderer,this->screenFont->fontTexture,&sRect,&dRect);
				SDL_RenderCopyEx(TBAGame->gameWindow->renderer,TBAGame->gameWindow->textScreen->screenFont->fontTexture,&sRect,&dRect,0,NULL,SDL_FLIP_NONE);
			}

			//Advance cursor for next character
			cursor[0] += this->charW;
		}
		//Advance cursor to next line
		cursor[1]++;
		cursor[0] = 0;

	}
	SDL_SetRenderTarget(TBAGame->gameWindow->renderer,NULL);

}

void Screen::shiftContentWindow(int i) {

	this->contentWindowOffset -= 2*i;
}

void TextScreen::resetContentWindowOffset() {
	this->contentWindowOffset = 0;
}

void TextBox::setContent(const std::string& content) {
	this->content.clear();
	this->addContent(content);
}

void TextBox::prepareContent() {

	std::string newContent = 
		"Inventory:"+TBAGame->playerChar->inventory->contentString +
		"\n\nPlayer info:\n\t"+TBAGame->playerChar->getInfo();

		if(TBAGame->playerChar->target != nullptr) {
			newContent += "\n\nTarget info:\n\t"+TBAGame->playerChar->getTargetInfo();
		}

	this->setContent(newContent);	

}

void DynamicTextBox::setContent(const std::string& content) {
	this->content.clear();

	//Reset box dimensions based on new content
	this->w = std::min(this->maxWidth*charW,(this->charW*(int)content.size()) + (this->borderSize*2));
	this->screenCharWidth = this->w/this->charW;

	this->h = (this->charH*((int)content.size()/this->screenCharWidth+1)) + 6;
	this->screenCharHeight = this->h/this->charH;

	this->x = this->defaultX - this->w/2;
	//this->y = this->defaultY + this->h/2;

	this->addContent(content);
}

//=================
//COMMAND FUNCTIONS
//=================

void TextScreen::setCommand(const std::string& s) {

	//Set the potentially incomplete command at the end of command history
	this->command = s;
	this->commandHistory.at(this->commandHistory.size()-1) = s;
	this->setCommandLine();
	this->resetContentWindowOffset();

}

void TextScreen::setCommandLine() {
	this->commandLines = this->prepareCommandForDisplay(tabChar+command+"_");
}

void TextScreen::commandAppend(char c) {
	this->setCommand(this->command+c);
}

void TextScreen::deleteLastCharacter() {

	if(this->command.size() > 0) {
		this->setCommand(this->command.substr(0,this->command.size()-1));
	}

}

void TextScreen::sendCommand() {


	std::string commandReturn;

	//COMMAND HISTORY HANDLING
	//If command history has more than 1 entry
	//If this command is the same as the last one, don't add it to command history
	if(this->commandHistory.size() > 1) {
		if(this->command != this->commandHistory.at(this->commandHistory.size()-2)) {
			this->commandHistory.push_back("");
			this->commandIndex++;
		}
	} else {
		this->commandHistory.push_back("");
		this->commandIndex++;
	}

	//Remove cursor character (_) when displaying previous commands
	this->commandLines.at(this->commandLines.size()-1).pop_back();

	//Add command line to history
	this->content = extend(this->content,this->commandLines);

	//Newlines are added automatically before and after command line
	//Retrieve command return from input function
	commandReturn = processCommand(this->command);

	//Add command to content
	//Command line is reset and added automatically whenever the command is updated
	if(commandReturn.size() > 0) {
		this->addContent(commandReturn);
	}
	this->setCommand("");
	this->commandIndex = this->commandHistory.size()-1;

}

void TextScreen::swapCommand(int i) {
	//Swaps commands from commandHistory
	//For use with command history functionality

	this->commandIndex += i;

	if(this->commandHistory.size() == 0) {return;}

	if(this->commandIndex < 0) {
		this->commandIndex = 0;
	} else if(this->commandIndex > this->commandHistory.size()-1) {
		this->commandIndex = this->commandHistory.size()-1;
	}

	this->setCommand(this->commandHistory.at(commandIndex));
	//this->command = this->commandHistory.at(commandIndex);

}

void TextScreen::addCommand() {

	//Add command to command history unless duplicate of previous command
	if(this->command != this->commandHistory.at(this->commandHistory.size()-1)) {
		this->commandHistory.push_back(this->command);
		commandIndex++;
	}

}

//=================
//    MISC FUNCTIONS
//=================

bool Screen::enclose(int x,int y) {
	//Determine if point x,y is within this screen

	if(x >= this->x and x < this->x+this->w) {
		if(y >= this->y and y < this->y+this->h) {
			return true;
		}
	}
	return false;
}



