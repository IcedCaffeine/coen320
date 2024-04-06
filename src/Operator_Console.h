#ifndef OPERATOR_CONSOLE_H
#define OPERATOR_CONSOLE_H

#include <string>
//#include "Structure.h"
#include "Aircraft.h"
#include "Communication_System.h"

class Operator_Console {
	private:
		char message;
		Computer_System* compSystem;

	public:
		// Constructor
		Operator_Console();

		// Thread
		void* operatorMain(void* arg);

		// Store the commands in a vector
		void storeCommands();

		// Get the information of the plane that is requesting for commands
		void fetchPlaneData(Aircraft* plane);

		// Function to send messages to the Computer System
		void sendMessage(Computer_System* compSystem);

		// Function to display the information of the specific aircraft
		void printDetails(int id);
};

#endif
