#include "Operator_Console.h"

using namespace std;

Operator_Console::Operator_Console() {
	// initialize to not receiving anything from the Communication system
	// receivedComm(false);
	// initialize a pointer to the Computer System to send it data
	this->compSystem = nullptr;
}

void Operator_Console::storeCommands() {
	string filename = "log.txt";
	string command;
	std::vector<std::string> commandList;
	std::ofstream outputFile(filename);

	if(outputFile.is_open()){
		for(unsigned int i=0; i<commandList.size(); i++){
			outputFile << commandList[i];
		}
		outputFile.close();
	}

}

void Operator_Console::fetchPlaneData(Aircraft* plane) {
	Aircraft data;
}

void Operator_Console::sendMessage(Computer_System* compSystem) {
	this->compSystem = compSystem;
}

void Operator_Console::printDetails(int id) {
	this->compSystem;
}

void* Operator_Console::operatorMain(void* arg) {
	Aircraft planeID;
	Computer_System command;

	while(planeID.outsideAirSpace()) {

		if("changeSpeed") {
			command.changeSpeed(planeID, 0.00);
		}
		else if("changeAltitude") {
			command.changeAltitude(planeID, 15);
		}
		else if("changePosition") {
			command.changePosition(planeID, 5);
		}
		else {
			cout << "This is an invalid command" << endl;
		}
		this->sendMessage(this->compSystem);
	}
}
