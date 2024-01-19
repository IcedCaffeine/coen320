#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

struct Aircraft {
    std::string id;
    double latitude;
    double longitude;
    double altitude;
    double speed;
    double heading;
};

class InputHandler {
public:
    std::vector<Aircraft> readAircraftData(const std::string& fileName) {
        std::vector<Aircraft> aircrafts;
        std::ifstream file(fileName);
        std::string line;
        
        while (getline(file, line)) {
            std::stringstream ss(line);
            Aircraft aircraft;
            ss >> aircraft.id >> aircraft.latitude >> aircraft.longitude >> aircraft.altitude >> aircraft.speed >> aircraft.heading;
            aircrafts.push_back(aircraft);
        }

        return aircrafts;
    }
};

int main() {
    InputHandler inputHandler;
    auto aircrafts = inputHandler.readAircraftData("aircraft_data.txt");

    // For demonstration: print the read data
    for (const auto& aircraft : aircrafts) {
        std::cout << "Aircraft ID: " << aircraft.id << ", Position: (" << aircraft.latitude << ", " << aircraft.longitude << "), Altitude: " << aircraft.altitude << ", Speed: " << aircraft.speed << ", Heading: " << aircraft.heading << std::endl;
    }

    return 0;
}
