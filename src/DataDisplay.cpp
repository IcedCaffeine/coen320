#include "DataDisplay.h"

#include <iostream>

DataDisplay::DataDisplay() {
	initialize(); // Construction Initialization Call
}

DataDisplay::~DataDisplay() {
	shm_unlink("display"); // Unlink shared memory on termination
}

int DataDisplay::initialize() {
	/*Make threads in detached state*/
	int receive = pthread_attr_init(&attr);
	if (receive) {
		printf("ERROR, Receive from pthread_attr_init() is %d \n", receive);
	}

	receive = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (receive) {
		printf("ERROR; Receive from pthread_attr_setdetachstate() is %d \n", receive);
	}

	// Setup Data Display
	shm_display = shm_open("display", O_RDWR, 0666);
	if (shm_display == -1) {
		perror("in shm_open() Display");
		exit(1);
	}

	ptr_display = mmap(0, SIZE_SHM_DISPLAY, PROT_READ | PROT_WRITE, MAP_SHARED, shm_display, 0);
	if (ptr_display == MAP_FAILED) {
		perror("in map() Display");
		exit(1);
	}

	return 0;
}

void DataDisplay::start() {
	time(&startTime);
	if (pthread_create(&dataDisplayThread, &attr, &DataDisplay::startThread, (void *)this) != EOK) {
		dataDisplayThread = 0;
	}
}

int DataDisplay::stop() {
	pthread_join(dataDisplayThread, NULL); // Join thread on stop
	return 0;
}

void *DataDisplay::startThread(void *context) {
	((DataDisplay *)context)->updateMap();
	return NULL;
}

void *DataDisplay::updateMap(void) {


	// Link to channel
	int channelId = ChannelCreate(0);
	if (channelId == -1) {
		std::cout << "couldn't create display channel!\n";
	}

	// Setup period of 5 seconds
	Timer timer(channelId);
	timer.setTimer(DATA_DISPLAY_PERIOD, DATA_DISPLAY_PERIOD);

	// Receive data
	int receiveId;
	Message message;
	int logCounter = 1;
	std::ofstream out("log");

	while (true) {
		if (receiveId == 0) {
			pthread_mutex_lock(&mutex);

			// Axis and Altitude
			int axis = 0;
			int z = 0; // 0=ID, 1=X, 2=Y, 3=Z, 4= Height;
			std::string x = "";
			std::string y = "";

			// Parsing buffers
			std::string buffer = "";
			std::string displayBit = "";
			std::string id = "";

			// Read from shared memory pointer
			for (int i = 0; i < SIZE_SHM_DISPLAY; i++) {
				// Read Character
				char readCharacter = *((char *)ptr_display + i);

				if (readCharacter == 't') {
					std::cout << "Data Display done\n";
					time(&finishTime);
					std::cout << "Data Display execution time: " << difftime(finishTime, startTime) << " seconds\n";
					ChannelDestroy(channelId);
					out.close();
					return 0;
				}

				// Check Ending Character
				if (readCharacter == ';') {
					break;
				}
				else if(readCharacter == ','){
					// Update Based on Buffer
					if(buffer.length() > 0 && axis == 0){
						id = buffer;
					}
					else if(buffer.length() > 0 && axis == 1){
						x = buffer;
					}
					else if(buffer.length() > 0 && axis == 2){
						y = buffer;
					}
					else if(buffer.length() > 0 && axis == 3){
						z = stoi(buffer) + ALTITUDE;
					}
					else if(buffer.length() > 0 && axis == 4){
						displayBit = buffer;
					}
					axis++;
					buffer = "";
				}
				else if(readCharacter == '/'){
					// Update Based on Buffer
					if(buffer.length() > 0 && axis == 0){
						id = buffer;
					}
					else if(buffer.length() > 0 && axis == 1){
						x = buffer;
					}
					else if(buffer.length() > 0 && axis == 2){
						y = buffer;
					}
					else if(buffer.length() > 0 && axis == 3){
						z = stoi(buffer) + ALTITUDE;
					}
					else if(buffer.length() > 0 && axis == 4){
						displayBit = buffer;
					}

					// Add id
					int row = (100000 - stoi(y)) / SCALER;
					int column = stoi(x) / SCALER;
					if(this->getGrid(row, column) == ""){
						this->setGrid(row, column, id);
					}
					else{
						this->setGrid(row, column, "\\" + id);
					}

					// Update based on Display COntrol
					if(displayBit == "1"){
						this->getDisplayedHeight() += "Plane " + id;
						this->getDisplayedHeight() += " has height of " + std::to_string(z);
						this->getDisplayedHeight()	+= " ft\n";
					}
					x=""; y=""; z=0;
					displayBit = ""; axis = 0; buffer = "";
				}
				else{
					buffer += readCharacter; // Load Buffer with Character
				}
			}
			pthread_mutex_unlock(&mutex);

			// Logging the airspace into a log file
			if (logCounter == 6) {
				std::cout << "Logging current airspace..." << std::endl;
				std::streambuf *coutbuf = std::cout.rdbuf();
				std::cout.rdbuf(out.rdbuf()); // redirect std::cout to out.txt!
				this->displayMap(); // Display map and height command
				std::cout << std::endl;
				std::cout.rdbuf(coutbuf);
				logCounter = 1;
			}

			else {
				this->displayMap(); // Display map and height command
			}
			logCounter++;

			this->setDisplayedHeight(""); // Reset buffer

			// Reset map array
			this->resetMap();
		}
		receiveId = MsgReceive(channelId, &message, sizeof(message), NULL);
	}
	ChannelDestroy(channelId);
	return 0;
}


// Printing grid
void DataDisplay::displayMap() {
	for (int i = 0; i < blockCount; i++) {
		for (int j = 0; j < blockCount; j++) {
			if (this->getGrid(i,j) == "") {
				std::cout << "_|";
			}
			else {
				std::cout << this->getGrid(i,j) << "|";
			}
		}
		std::cout << "\n";
	}
	std::cout << "\n";
	printf("%s\n\n", this->getDisplayedHeight().c_str());
}

void DataDisplay::resetMap(){
	for (int i = 0; i < blockCount; i++) {
		for (int j = 0; j < blockCount; j++) {
			this->setGrid(i, j, "");
		}
	}
}

std::string DataDisplay::getDisplayedHeight() const {
	return displayedHeight;
}

void DataDisplay::setDisplayedHeight(std::string displayedHeight) {
	this->displayedHeight = displayedHeight;
}

void DataDisplay::setGrid(int row, int column, std::string value){
	this->grid[row][column] = value;
}

std::string DataDisplay::getGrid(int row, int column){
	return this->grid[row][column];
}
