#include "Aircraft.h"

// constructor
Aircraft::Aircraft(int ID, int arrivalTime, int position[3], int speed[3]) {
	// initialize members
	this->arrivalTime = arrivalTime;
	this->ID = ID;

	// Positons
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
	shm_unlink(fileName.c_str());
	pthread_mutex_destroy(&mutex);
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
	((Aircraft *)context)->flyPlane();
	return 0;
}

// initialize thread and shm members
int Aircraft::initialize() {
	// set thread in detached state
	int rc = pthread_attr_init(&attr);
	if (rc) {
		printf("ERROR, RC from pthread_attr_init() is %d \n", rc);
	}

	rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (rc) {
		printf("ERROR; RC from pthread_attr_setdetachstate() is %d \n", rc);
	}

	// Setup File Name
	this->setFileName("plane_" + std::to_string(ID));

	// open shm object
	shm_fd = shm_open(this->getFileName().c_str(), O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1) {
		perror("in shm_open() plane");
		exit(1);
	}

	// set the size of shm
	ftruncate(shm_fd, SIZE_SHM_PLANES);

	// map shm
	ptr = mmap(0, SIZE_SHM_PLANES, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("map failed\n");
		return -1;
	}

	// update string of plane data
	this->toString();

	// initial write + space for comm system
	sprintf((char *)ptr, "%s0;", this->getPlaneMessage().c_str());

	return 0;
}

// update position every second from position and speed
void *Aircraft::flyPlane(void) {
	// create channel to link timer
	int chid = ChannelCreate(0);
	if (chid == -1) {
		std::cout << "couldn't create channel!\n";
	}

	// create timer and set offset and period
	Timer timer(chid);
	timer.setTimer(this->getArrivalTime() * 1000000, AIRCRAFT_PERIOD);

	// buffers for message from timer
	int receiveId;
	Message msg;

	bool start = true;
	while (true) {
		if (start) {
			start = false; // first cycle, wait for arrival time
		} else {
			if (receiveId == 0) {
				pthread_mutex_lock(&mutex);

				// check comm system for potential command
				this->answerComm();

				// update position based on speed
				this->updatePosition();

				// check for airspace limits, write to shm
				if (checkLimits() == 0) {
					std::cout << this->getFileName().c_str() << " terminated\n";
					time(&finishTime);
					double executionTime = difftime(finishTime, startTime);
					std::cout << this->getFileName().c_str() << " execution time: " << executionTime << " seconds\n";
					ChannelDestroy(chid);
					return 0;
				}

				pthread_mutex_unlock(&mutex);
			}
		}
		// wait until next timer pulse
		receiveId = MsgReceive(chid, &msg, sizeof(msg), NULL);
	}

	ChannelDestroy(chid);

	return 0;
}

// check comm system for potential commands
void Aircraft::answerComm() {
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
	for (; i < SIZE_SHM_PLANES; i++) {
		readCharacter = *((char *)ptr + i);
		if (readCharacter == ';') {
			break; // found end
		}
	}

	// check for command presence
	if (*((char *)ptr + i + 1) == ';' || *((char *)ptr + i + 1) == '0') {
		return;
	}

	// set index to next
	i++;
	int startIndex = i;
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
		char currentCharacter = buffer[j];

		switch (currentCharacter) {
		case ';':
			speed[currentParameter] = std::stoi(parseBuffer);
			break;
		case '/':
			speed[currentParameter] = std::stoi(parseBuffer);
			parseBuffer = "";
			continue;
		case ',':
			parseBuffer = "";
			continue;
		case 'x':
			currentParameter = 0;
			continue;
		case 'y':
			currentParameter = 1;
			continue;
		case 'z':
			currentParameter = 2;
			continue;
		default:
			parseBuffer += currentCharacter;
			continue;
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
	sprintf((char *)ptr + startIndex, "0;");
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
