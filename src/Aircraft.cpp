#include "Aircraft.h"

// constructor
Aircraft::Aircraft(int ID, int arrivalTime, int position[3], int speed[3]) {
	// initialize members
	this->arrivalTime = arrivalTime;
	this->ID = ID;

	// Positions
	this->x = position[0];
	this->y = position[1];
	this->z = position[2];

	// Speed
	this->speedX = speed[0];
	this->speedY = speed[1];
	this->speedZ = speed[2];

	this->commandCounter = 0;
	this->commandInProgress = false;

	// initialize thread members
	this->initialize();
}

Aircraft::~Aircraft() {
	shm_unlink(this->getFileName().c_str());
	shm_unlink(this->getFileName().c_str());
	pthread_mutex_destroy(&mutex);
}


// initialize thread and shm members
int Aircraft::initialize() {
	// set thread in detached state
	int received = pthread_attr_init(&attr);
	if (received) {
		printf("Error : pthread_attr_init() RC = %d \n", received);
	}

	received = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (received) {
		printf("Error : pthread_attr_setdetachstate() RC = %d \n", received);
	}

	// Setup File Name
	this->setFileName(std::to_string(ID));

	// open shm object
	shm_fd = shm_open(this->getFileName().c_str(), O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {
		perror("in shm_open() fileData");
		exit(1);
	}
	ftruncate(shm_fd, SIZE_SHM_PLANES);

	ptr = mmap(0, SIZE_SHM_PLANES, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("map failed\n");
		return -1;
	}

	// update string of plane data
	this->toString();
	sprintf((char *)ptr, "%s0;", this->getPlaneMessage().c_str());
	return 0;
}


void *Aircraft::navigatePlane(void) {
	// Initialize Message Buffers
	int receiveId;
	Message msg;

	// Link Timer to new Channel
	int chid = ChannelCreate(0);
	if (chid == -1) {
		std::cout << "couldn't create channel!\n";
	}
	Timer timer(chid);
	timer.setTimer(this->getArrivalTime() * 1000000, AIRCRAFT_PERIOD);

	bool begin = true;
	while (true) {
		if (begin) {
			begin = false; // first cycle, wait for arrival time
		}
		else {
			if (receiveId == 0) {
				pthread_mutex_lock(&mutex);

				// Check Comm and Update
				this->answerCommunications();
				this->updatePosition();

				// Check Airspace limits & share to memory
				int limit = checkLimits();
				if (limit == 0) {
					std::cout << this->getFileName().c_str() << " terminated\n";
					time(&finishTime);
					std::cout << this->getFileName().c_str() << " execution time: " << difftime(finishTime, startTime) << "s\n";
					ChannelDestroy(chid);
					return 0;
				}

				pthread_mutex_unlock(&mutex);
			}
		}
		receiveId = MsgReceive(chid, &msg, sizeof(msg), NULL);
	}

	ChannelDestroy(chid);
	return 0;
}

// check comm system for potential commands
void Aircraft::answerCommunications() {
	// check if executing command
	if (this->isCommandInProgress()) {
		// decrement counter
		this->setCommandCounter(this->getCommandCounter() - 1);
		if (this->getCommandCounter() <= 0) {
			this->setCommandInProgress(false);
		}
		return;
	}
	this->setSpeedZ(0);


	// find end of plane info
	int i = 0;
	char readCharacter;
	while(i < SIZE_SHM_PLANES){
		readCharacter = *((char *)ptr + i);
		if (readCharacter == ';') {
			break; // found end
		}
		i++;
	}

	// check for Command
	if (*((char *)ptr + i + 1) == '0' || *((char *)ptr + i + 1) == ';') {
		return;
	}

	// set index to next
	i++;
	int index = i;
	readCharacter = *((char *)ptr + i);
	std::string buffer = "";
	while (readCharacter != ';') {
		buffer += readCharacter;
		readCharacter = *((char *)ptr + ++i);
	}
	buffer += ';';

	// parse command
	std::string parseBuffer = "";
	int currentParameter;
	int speed[3];
	for (int j = 0; j <= (int)buffer.size(); j++) {
		if(buffer[j] == 'x'){
			currentParameter = 0;
		}
		else if(buffer[j] == 'y'){
			currentParameter = 1;
		}
		else if(buffer[j] == 'z'){
			currentParameter = 2;
		}
		else if(buffer[j] == '/'){
			speed[currentParameter] = std::stoi(parseBuffer);
			parseBuffer = "";
		}
		else if(buffer[j] == ';'){
			speed[currentParameter] = std::stoi(parseBuffer);
		}
		else if(buffer[j] == ','){
			parseBuffer = "";
		}
		else{
			parseBuffer += buffer[j];
		}
	}
	this->setSpeedX(speed[0]);
	this->setSpeedY(speed[1]);
	this->setSpeedZ(speed[2]);

	for (int k = 0; k < 500; k += abs(this->getSpeedZ())) {
		this->setCommandCounter(this->getCommandCounter() + 1);
	}
	this->setCommandInProgress(true);

	// remove command
	sprintf((char *)ptr + index, "0;");
}

// update position based on speed
void Aircraft::updatePosition() {
	this->setX(this->getX() + this->getSpeedX());
	this->setY(this->getY() + this->getSpeedY());
	this->setZ(this->getZ() + this->getSpeedZ());
	this->toString();
}

// stringify plane data members
void Aircraft::toString() {
	std::string planeMessage = std::to_string(this->getId()) + ",";
	planeMessage += std::to_string(this->getArrivalTime()) + ",";
	planeMessage += std::to_string(this->getX()) + ",";
	planeMessage += std::to_string(this->getY()) + ",";
	planeMessage += std::to_string(this->getX()) + ",";
	planeMessage += std::to_string(this->getSpeedX()) + ",";
	planeMessage += std::to_string(this->getSpeedY()) + ",";
	planeMessage += std::to_string(this->getSpeedZ()) + ";";
	this->setPlaneMessage(planeMessage);

}

// check airspace limits for operation termination
int Aircraft::checkLimits() {
	bool exceedLimitX = this->getX() < MIN_X_AIRSPACE || this->getX() > MAX_X_AIRSPACE;
	bool exceedLimitY = this->getY() < MIN_Y_AIRSPACE || this->getY() > MAX_Y_AIRSPACE;
	bool exceedLimitZ = this->getZ() < MIN_Z_AIRSPACE || this->getZ() > MAX_Z_AIRSPACE;

	if(exceedLimitX || exceedLimitY || exceedLimitZ){
		this->setPlaneMessage("terminated");
		sprintf((char *)ptr, "%s0;", this->getPlaneMessage().c_str());
		return 0;
	}

	// write plane to shared memory
	sprintf((char *)ptr, "%s0;", planeMessage.c_str());
	return 1;
}

void Aircraft::print() {
	std::cout << planeMessage << "\n";
}

int Aircraft::getArrivalTime() const {
	return arrivalTime;
}

void Aircraft::setArrivalTime(int arrivalTime) {
	this->arrivalTime = arrivalTime;
}

int Aircraft::getCommandCounter() const {
	return commandCounter;
}

void Aircraft::setCommandCounter(int commandCounter) {
	this->commandCounter = commandCounter;
}

bool Aircraft::isCommandInProgress() const {
	return commandInProgress;
}

void Aircraft::setCommandInProgress(bool commandInProgress) {
	this->commandInProgress = commandInProgress;
}

std::string Aircraft::getFileName() const {
	return fileName;
}

void Aircraft::setFileName(std::string fileName) {
	this->fileName = fileName;
}

int Aircraft::getId() const {
	return ID;
}

void Aircraft::setId(int id) {
	ID = id;
}

std::string Aircraft::getPlaneMessage() const {
	return planeMessage;
}

void Aircraft::setPlaneMessage(std::string planeMessage) {
	this->planeMessage = planeMessage;
}

int Aircraft::getSpeedX() const {
	return speedX;
}

void Aircraft::setSpeedX(int speedX) {
	this->speedX = speedX;
}

int Aircraft::getSpeedY() const {
	return speedY;
}

void Aircraft::setSpeedY(int speedY) {
	this->speedY = speedY;
}

int Aircraft::getSpeedZ() const {
	return speedZ;
}

void Aircraft::setSpeedZ(int speedZ) {
	this->speedZ = speedZ;
}

int Aircraft::getX() const {
	return x;
}

void Aircraft::setX(int x) {
	this->x = x;
}

int Aircraft::getY() const {
	return y;
}

void Aircraft::setY(int y) {
	this->y = y;
}

int Aircraft::getZ() const {
	return z;
}

void Aircraft::setZ(int z) {
	this->z = z;
}

int Aircraft::start() {
	time(&startTime);
	if (pthread_create(&thread, &attr, &Aircraft::startThread, (void *)this) !=EOK) {
		thread = 0;
	}

	return 0;
}

bool Aircraft::stop() {
	pthread_join(thread, NULL);
	return 0;
}

void *Aircraft::startThread(void *context) {
	// set priority
	((Aircraft *)context)->navigatePlane();
	return 0;
}
