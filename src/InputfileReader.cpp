/*
 * InputfileReader.cpp
 *
 *  Created on: Mar 23, 2024
 *      Author: Fahad Abdul Rahman
 *      #40157997
 */

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdexcept>
#include "Aircraft.h"

class InputFileReader {
public:
    std::vector<Aircraft> aircraft_list;

    InputFileReader() {
        read_aircraft_data();
    }

    void print_aircraft_data() {
        for (const auto& aircraft : aircraft_list) {
            aircraft.PrintAircraft();
        }
    }

    void read_aircraft_data() {
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
                Aircraft aircraft(ID, arrivalTime, {x, y, z}, {speed_x, speed_y, speed_z});
                aircraft_list.push_back(aircraft);
            }
        }
    }
};



