/*
 * Aircraft.cpp
 *
 *  Created on: Mar 23, 2024
 *      Author: Fahad Abdul Rahman
 *      #40157997
 */


#include "Aircraft.h"
#include <cmath> // For basic mathematical operations

// Constructor to initialize the Aircraft object with given parameters
Aircraft::Aircraft(){

}

Aircraft::Aircraft(int aircraftId, int posX, int posY, int posZ, int velX, int velY, int velZ, int arrivalTime): arrivalTime(arrivalTime){
	this->aircraftId = aircraftId;

	this->posX = posX;
	this->posY = posY;
	this->posZ = posZ;

	this->velX = velX;
	this->velY = velY;
	this->velZ = velZ;
}

// Destructor
Aircraft::~Aircraft() {
    // Any cleanup if necessary
}

int Aircraft::getAircraftId() const {
	return aircraftId;
}

void Aircraft::setAircraftId(int aircraftId) {
	this->aircraftId = aircraftId;
}

int Aircraft::getArrivalTime() const {
	return arrivalTime;
}

void Aircraft::setArrivalTime(int arrivalTime) {
	this->arrivalTime = arrivalTime;
}

int Aircraft::getPosX() const {
	return posX;
}

void Aircraft::setPosX(int posX) {
	this->posX = posX;
}

int Aircraft::getPosY() const {
	return posY;
}

void Aircraft::setPosY(int posY) {
	this->posY = posY;
}

int Aircraft::getPosZ() const {
	return posZ;
}

void Aircraft::setPosZ(int posZ) {
	this->posZ = posZ;
}

int Aircraft::getVelX() const {
	return velX;
}

void Aircraft::setVelX(int velX) {
	this->velX = velX;
}

int Aircraft::getVelY() const {
	return velY;
}

void Aircraft::setVelY(int velY) {
	this->velY = velY;
}

int Aircraft::getVelZ() const {
	return velZ;
}

void Aircraft::setVelZ(int velZ) {
	this->velZ = velZ;
}


// Updates the position of the aircraft based on its velocity and the elapsed time
void Aircraft::updatePosition() {
    // Implementation here
}





