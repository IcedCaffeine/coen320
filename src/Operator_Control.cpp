#include "Operator_Console.h"
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;

// Constructor
Operator_Control::Operator_Control() : compSystem(nullptr) {}

// Thread function for operator console
void* Operator_Control::operatorMain(void* arg) {
    while (true) {
        // Check for new commands every iteration
        usleep(500000); // Wait for 0.5 seconds (adjust as needed)

        // Process commands if there are any
        if (!commands.empty()) {
            sendCommands(); // Send stored commands to the Computer System
            commands.clear(); // Clear the commands after sending
        }
    }
    return nullptr;
}

// Store a command in the log
void Operator_Control::storeCommand(const string& command) {
    commands.push_back(command); // Add command to the list
    // Optionally, log the command to a file for record-keeping
    ofstream logfile("commands.log", ios::app);
    if (logfile.is_open()) {
        logfile << command << endl;
        logfile.close();
    } else {
        cerr << "Error: Unable to open log file for writing." << endl;
    }
}

// Fetch information of a specific plane
Aircraft Operator_Control::fetchPlaneData(int planeID) {
    // Access plane information from the appropriate data source
    // For simplicity, let's assume it's retrieved from a database or stored locally
	Aircraft Aircraft; // Assuming PlaneData is a struct or class containing aircraft details
    // Populate planeData with relevant information
    return Aircraft;
}

// Send commands to the Computer System
void Operator_Control::sendCommands() {
    if (compSystem) {
        // Send commands to the Computer System for execution
        for (const string& command : commands) {
            compSystem->executeCommand(command);
        }
    } else {
        cerr << "Error: Computer System is not available." << endl;
    }
}

// Display details of a specific aircraft
void Operator_Control::displayAircraftDetails(int aircraftID) {
    // Call appropriate method in CompSystem to display aircraft details
    if (compSystem) {
        compSystem->displayAircraftDetails(aircraftID);
    } else {
        cerr << "Error: Computer System is not available." << endl;
    }
}
