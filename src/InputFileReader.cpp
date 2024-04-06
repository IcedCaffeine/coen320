#include "InputFileReader.h"

InputFileReader::InputFileReader() {
	this->read_aircraft_data();
}

void InputFileReader::print_aircraft_data() {
	for(unsigned int i=0; i<this->aircraft_list.size(); i++){
		Aircraft aircraft = aircraft_list[i];
		// aircraft.PrintAircraft();
	}
}

void InputFileReader::read_aircraft_data() {
	std::string file_path = "./Aircraftinfo.txt";
	std::ifstream input_file(file_path);
	if (!input_file) {
		throw std::runtime_error("Failed to open input file: " + file_path);
	}

	// Skip the first line (header)
	std::string line;
	std::getline(input_file, line);

	while (std::getline(input_file, line)) {
		std::istringstream iss(line);
		int ID, arrivalTime, x, y, z, speed_x, speed_y, speed_z;
		if (iss >> ID >> arrivalTime >> x >> y >> z >> speed_x >> speed_y >> speed_z) {
			Aircraft aircraft(ID,x,y,z,speed_x, speed_y, speed_z, arrivalTime);
			this->aircraft_list.push_back(aircraft);
		}
	}
}




