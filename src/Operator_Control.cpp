#include "Operator_Console.h"
#include <iostream>
#include <fstream>
using namespace std;

OperatorConsole::OperatorConsole() {}

void OperatorConsole::storeCommands(const vector<string>& newCommands) {
    // Assuming commands are received externally and added to the existing list
    commands.insert(commands.end(), newCommands.begin(), newCommands.end());

    // Store commands in a log file
    ofstream logFile("log.txt");
    if (logFile.is_open()) {
        for (const auto& command : commands) {
            logFile << command << endl;
        }
        logFile.close();
    } else {
        cerr << "Unable to open log file." << endl;
    }
}

void OperatorConsole::fetchPlaneData(Aircraft& plane) {
    // Assuming plane data is fetched from an external source
    // and stored in the provided Plane object
}

void OperatorConsole::sendMessage(CompSystem& compSystem) {
    // Assuming the operator console sends commands to the computer system
    compSystem.receiveCommands(commands);
}

void OperatorConsole::printDetails(int id) {
    // Assuming the operator console displays details of a specific aircraft
    cout << "Details of aircraft with ID " << id << ":" << std::endl;
    // Print details here
}

void* OperatorConsole::operatorMain(void* arg) {
    // Main function of the Operator Console thread
    OperatorConsole* opConsole = static_cast<OperatorConsole*>(arg);

    // Assuming some kind of loop or event handling mechanism
    // to process commands and interact with the computer system
    for (const auto& command : opConsole->commands) {
        // Process each command
        // For example:
        if (command == "changeSpeed") {
            // Handle speed change command
        } else if (command == "changeAltitude") {
            // Handle altitude change command
        } else if (command == "changePosition") {
            // Handle position change command
        } else {
            std::cerr << "Invalid command: " << command << std::endl;
        }
    }

    // Assuming some cleanup or termination process
    return nullptr;
}
