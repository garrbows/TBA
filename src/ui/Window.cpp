#include "Window.h"
#include "../common/Common.h"
#include "../tools/Utility.h"

#include <iostream>
#include <SDL2/SDL.h>
#include <vector>

#include <thread>
#include <chrono>

Window::Window(int w,int h) {
	windowLog->writeln("New window initialized");

	this->window = SDL_CreateWindow("TBA",
													SDL_WINDOWPOS_UNDEFINED,
													SDL_WINDOWPOS_UNDEFINED,
													w,h,
													SDL_WINDOW_OPENGL
													);

	this->renderer = SDL_CreateRenderer(this->window,-1,SDL_RENDERER_ACCELERATED);

	this->width = w;
	this->height = h;

	//SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_NONE);

	//std::string path = "assets/icon64.png";

	SDL_Surface *loadedSurface;
	SDL_RWops *rwop;
	rwop = SDL_RWFromFile("assets/icon/icon64.png","rb");
	loadedSurface = IMG_LoadPNG_RW(rwop);

	SDL_SetWindowIcon(window,loadedSurface);
	SDL_FreeSurface(loadedSurface);

}

void Window::generateScreenFontTextures() {
	
	
	this->mapScreen->screenFont->generateFontTexture(this->window,this->renderer);

	// this->popupBox->screenFont->generateFontTexture(this->window,this->renderer);
	
	//=================================
	//!!!!!SET ACTIVE TILE COLOR/ALPHA HERE !!!!!
	//=================================
	SDL_SetTextureColorMod(this->mapScreen->screenFont->fontTexture,200,200,200);

	for(int i=0;i<this->screenVector.size();i++) {
		screenVector.at(i)->screenFont->generateFontTexture(this->window,this->renderer);
		// SDL_SetTextureAlphaMod(screenVector.at(i)->screenFont->fontTexture,200);
		SDL_SetTextureColorMod(this->screenVector.at(i)->screenFont->fontTexture,180,180,180);
	}	
}

void Window::drawScreens() {

	for(int i=0;i<this->screenVector.size();i++) {
		screenVector.at(i)->update();
	}	

}

void Window::bringToFront(Screen* screen) {
	//Advance screen through update priorities until it is at the front
	while(screen->updatePriority != this->screenVector.size()-1) {//2) {
		swap(this->screenVector,screen->updatePriority,screen->updatePriority+1);
		int oldPriority = screen->updatePriority;
		screen->updatePriority++;
		this->screenVector.at(oldPriority)->updatePriority = oldPriority;
	}
}

void Window::deleteFirstToggledPopup() {
	for(int i=0;i<this->popupVector.size();i++) {
		if(popupVector.at(i)->toggled) {
			this->popupVector.erase(this->popupVector.begin()+i);
		}
	}
}

void Window::createPopup(const std::string &message,int duration,bool toggled) {
	int borderSize = 25;
	DynamicTextBox *newPopup = new DynamicTextBox(message,duration,this->mapScreen->x+(this->mapScreen->w/2),borderSize+(borderSize/2)+mapScreen->h);
	// newPopup->screenFont->generateFontTexture(this->window,this->renderer);
	// newPopup->generateTexture(newPopup->content);
	newPopup->toggled = toggled;
	this->popupVector.push_back(newPopup);
}

void Window::update(bool gdebug) {
	
	// debug("Updating screens");

	SDL_SetRenderDrawColor(this->renderer,bgColor.r,bgColor.g,bgColor.b,bgColor.a);
	SDL_RenderClear(this->renderer);
	if(gdebug == false) this->mapScreen->update();
	TBAGame->updateGameUIObjects();
	//this->mapPanel->update();
	 if(this->popupVector.size() > 0) {
		this->popupVector.at(0)->update();
		// debug("Updated popup");
		if(!this->popupVector.at(0)->active) {
			// debug("Erasing invalid popup");
			this->popupVector.erase(popupVector.begin());
			this->popupVector.shrink_to_fit();
		}
	 }

	// dumpVec(this->popupVector);
	
	//Main drawing functions go here
	//-------------------
	this->drawScreens();
	//-------------------

	//TBAGame->updateGameUIObjects();

	//DEBUG MAP CENTER
	//SDL_SetRenderDrawColor(this->renderer,255,255,255,255);
	//SDL_RenderDrawPoint(this->renderer,this->mapScreen->x+this->mapScreen->w/2,this->mapScreen->y+this->mapScreen->h/2);

	SDL_RenderPresent(this->renderer);

}
