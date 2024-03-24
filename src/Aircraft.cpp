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
Aircraft::Aircraft(int id, int posX, int posY, int posZ, int velX, int velY, int velZ, int time)
: aircraftID(id), arrivalTime(time), outsideAirspaceStatus(true) {
    positionTrue[0] = posX;
    positionTrue[1] = posY;
    positionTrue[2] = posZ;
    velocity[0] = velX;
    velocity[1] = velY;
    velocity[2] = velZ;
}

// Destructor
Aircraft::~Aircraft() {
    // Any cleanup if necessary
}

// Updates the position of the aircraft based on its velocity and the elapsed time
void Aircraft::updatePosition() {
    // Implementation here
}

// Checks if the aircraft is outside the designated airspace
bool Aircraft::outsideAirspace() const {
    // Implementation here
}

// Retrieves the aircraft's ID
int Aircraft::getAircraftID() const {
    // Implementation here
}

// Retrieves the aircraft's arrival time
int Aircraft::getArrivalTime() const {
    // Implementation here
}

// Retrieves the aircraft's position on the requested axis (0 for X, 1 for Y, 2 for Z)
int Aircraft::getPosition(int axis) const {
    // Implementation here
}

// Retrieves the aircraft's velocity on the requested axis (0 for X, 1 for Y, 2 for Z)
int Aircraft::getVelocity(int axis) const {
    // Implementation here
}

// Additional functionalities as required by the project


