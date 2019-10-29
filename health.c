/*
 * health.c
 *
 * Created: 24/05/2019 12:28:55 AM
 *  Author: Minh Anh Bui
 */ 

#include "health.h"

uint8_t health;

void set_health(){
	health = 4;
}

void lose_life() {
	if (health>0)
	{
		health=health-1;
	}
}

uint8_t get_health(){
	return health;
}
