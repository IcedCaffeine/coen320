#pragma once

#include <string>
#include <vector>
#include "Aircraft.h"
//#include "CompSystem.h"

using namespace std;

class Operator_Control {
private:
    // Store commands sent to the operator console
    vector<string> commands;

    // Pointer to the Computer System
    ATC* compSystem;

public:
    // Constructor
    Operator_Control();

    // Thread function for operator console
    void* operatorMain(void* arg);

    // Store a command in the log
    void storeCommand(const string& command);

    // Fetch information of a specific plane
    Aircraft fetchPlaneData(int planeID);

    // Send commands to the Computer System
    void sendCommands();

    // Display details of a specific aircraft
    void displayAircraftDetails(int aircraftID);
};
