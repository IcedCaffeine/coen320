#ifndef INPUT_FILE_READER_H
#define INPUT_FILE_READER_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <stdexcept>
#include "Aircraft.h"


class InputFileReader {

public:
    std::vector<Aircraft> aircraft_list;
    InputFileReader();
    void print_aircraft_data();
    void read_aircraft_data();
};


#endif /* INPUT_FILE_READER_H */
