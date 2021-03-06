#pragma once

#include <SDL2/SDL.h>
#include <string>

//General processing
void processKeystroke(int);
void commandAppend(char);


//Content functions
void addContent(const std::string&);
void delChar();
void sendCommand();
void swapCommand(int);
void shiftContentWindow(int);

//Macro
void autocomplete();
void clear();
void paste();
void clearCommand();

//Mouse functions
void click(SDL_MouseButtonEvent&);
void move(SDL_Event&);
void release();

//Screen functions
void resetScreen(int);
void resetAllScreens();

//Movement
void move(bool,bool);
void turn(bool,bool);

//Misc
void pause();
void debugKey();