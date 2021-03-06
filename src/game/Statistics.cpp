#include "Statistics.h"
#include  "../common/Common.h"
	
void Character::init_stats() {

	//Rates in format ticks per action
	
	//Rate (ticks) at which attacks can be initiated
	this->defaultAttackRate = 5;

	//Unarmed damage
	this->defaultAttackDamage = 30;

	//Unarmed striking speed
	// # of ticks it takes for an attack to connect after being initiated
	// this->defaultAttackSpeed = 20;
	this->defaultAttackSpeed = 20;

	this->maxHealth = 10;
	this->health = this->maxHealth;

	//In format
	this->defaultAttackRange = .5;
	this->defaultMoveSpeed = 2;
	
	this->defaultTurnSpeed = 4;
	this->turnSpeed = this->defaultTurnSpeed;
	this->defaultFOV = 90;

	this->attackRate = this->defaultAttackRate - 2*((-500+rand()%1000)/50);

}