#include "Game.h"
#include "KeyFuncs.h"
#include "Keys.h"

#include <map>

//ESC 27
//BACKSPACE 08
//SPACE 32
//ENTER 13
//TAB 9

std::map<int,int> shiftMap = {
	{47,63}, //Forward slash
	{48,41}, //0-9
	{49,33},
	{50,64},
	{51,35},
	{52,36},
	{53,37},
	{54,94},
	{55,38},
	{56,42},
	{57,40}, 
	{92,124}, //Backslash
	{59,58},  //Semicolon
	{46,62},  //Period
	{44,60},  //Comma
	{39,34},  //Apostrophe
	{61,43},  //Equals
	{45,95},  //Minus
	{96,126}, //Backtick -> tilde
	{91,123}, //Left bracket
	{93,125}, //Right  bracket
};

void Game::input() {
		
	// bool shift = false;

	SDL_Event event;

	int keycode;
	
	SDL_PumpEvents();

	while(SDL_PollEvent(&event)) {
		
		keycode = event.key.keysym.sym; 

		switch(event.type) {
			case SDL_QUIT:
				this->gameRunning = false;
				break;
			//=========
			// KEYDOWN
			//=========
			case SDL_KEYDOWN:
				switch(keycode) {
					case 27:
						this->gameRunning = false;
						break;
					case 8:
						delChar();
						break;
					case 9:
						autocomplete();
						break;
					case SDLK_KP_ENTER:
					case 13:
						sendCommand();
						break;
					// case SDLK_LSHIFT:
						// shift=true;
						// break;
					case 96:
						//backtick not supported
						break;
					case 32:
						if(SDL_GetModState() & (KMOD_LCTRL | KMOD_CAPS)) {
							pause();
						}	else {
							processKeystroke(32);
						}
						break;
					case 108:
						if(SDL_GetModState() & (KMOD_LCTRL)) {
							clear();
						} else {
							if(SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) {
								processKeystroke(76);
							} else {
								processKeystroke(108);
							}
						}
						break;
					case 117:
						if(SDL_GetModState() & (KMOD_LCTRL)) {
							clearCommand();
							
						} else {
							if(SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) {
								processKeystroke(85);
							} else {
								processKeystroke(117);
							}
						}
						break;
					case 118:
						if(SDL_GetModState() & (KMOD_LCTRL)) {
							paste();
						} else {
							if(SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) {
								processKeystroke(86);
							} else {
								processKeystroke(118);
							}
						}
						break;
					case SDLK_KP_PLUS: //DEBUG KEY
						debugKey();
						break;
					case SDLK_LEFT:
					case SDLK_RIGHT:
						if(SDL_GetModState() & (KMOD_LCTRL | KMOD_CAPS)) {
							switch(keycode) {
								case SDLK_LEFT:
									this->turn_left = true;
									break;
								case SDLK_RIGHT:
									this->turn_right = true;
									break;
							}
						}
						break;
					case SDLK_UP:
					case SDLK_DOWN:
						if(SDL_GetModState() & (KMOD_LCTRL | KMOD_CAPS)) {
							switch(keycode) {
								case SDLK_UP:
									this->m_forward = true;
									break;
								case SDLK_DOWN:
									this->m_back = true;
									break;
							}
						} else {
							switch(keycode) {
								case SDLK_UP:
									swapCommand(-1);
									break;
								case SDLK_DOWN:
									swapCommand(1);
									break;
							}
						}
						break;
					default:
						if(SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT)) {
							if( keycode >= 97 and keycode <= 122) {
								keycode = keycode - 32;
							} else if(shiftMap.find(keycode) != shiftMap.end()) {
								keycode = shiftMap[keycode];
							}
						}
						if( (keycode >= 0) && (keycode < 256)) {
							processKeystroke(keycode);
						}
						break;
				}
				break;
			//=======
			//  KEYUP
			//=======
			case SDL_KEYUP:
				switch(keycode) {
					// case SDLK_LSHIFT:
						// shift=false;
						// SDL_FlushEvent(SDL_KEYUP);
						// break;
					case SDLK_UP:
					case SDLK_DOWN:
						switch(keycode) {
							case SDLK_UP:
								this->m_forward = false;
								break;
							case SDLK_DOWN:
								this->m_back = false;
								break;
							}
						break;
					case SDLK_LEFT:
					case SDLK_RIGHT:
						switch(keycode) {
							case SDLK_LEFT:
								this->turn_left = false;
								break;
							case SDLK_RIGHT:
								this->turn_right = false;
								break;
							}
						break;
				}
				break;
				case SDL_MOUSEBUTTONDOWN:
					switch(event.button.button) {
						case SDL_BUTTON_LEFT:
						case SDL_BUTTON_RIGHT:
							click(event.button);
							break;
					}
					break;
			//=======
			//  MOUSE
			//=======
				case SDL_MOUSEWHEEL:
					shiftContentWindow(event.wheel.y);
					break;
				case SDL_MOUSEBUTTONUP:
					switch(event.button.button) {
						case SDL_BUTTON_LEFT:
							release();
							break;
					}
					break;
				case SDL_MOUSEMOTION:
					move(event);
					break;
			}
		}
		move(this->m_forward,this->m_back);
		turn(this->turn_left,this->turn_right);
	}