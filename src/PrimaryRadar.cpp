#include "PrimaryRadar.h"

PrimaryRadar::PrimaryRadar(int numberOfPlanes) {
	this->currPeriod = PRIMARY_SOURCE_RADAR_PERIOD;
	this->numWaitingPlanes = numberOfPlanes;
	this->initialize(numberOfPlanes);
}

PrimaryRadar::~PrimaryRadar() {
	shm_unlink("waiting_planes");
	shm_unlink("flying_planes");
	shm_unlink("period");
	for(std::string name : flyingFileNames){
		shm_unlink(name.c_str());
	}
	pthread_mutex_destroy(&mutex);
	delete this->getTimer();
}

void PrimaryRadar::start() {
	time(&at);
	time(&startTime);
	if (pthread_create(&primaryRadarThread, &attr, &PrimaryRadar::startPrimaryRadar, (void *)this) != EOK) {
		primaryRadarThread = 0;
	}
}

int PrimaryRadar::stop() {
	pthread_join(primaryRadarThread, NULL);

	return 0;
}

int PrimaryRadar::getCurrPeriod() const {
	return currPeriod;
}

void PrimaryRadar::setCurrPeriod(int currPeriod) {
	this->currPeriod = currPeriod;
}

std::vector<std::string> PrimaryRadar::getFlyingFileNames() const {
	return flyingFileNames;
}

void PrimaryRadar::setFlyingFileNames(std::vector<std::string> flyingFileNames) {
	this->flyingFileNames = flyingFileNames;
}

int PrimaryRadar::getNumWaitingPlanes() const {
	return numWaitingPlanes;
}

void PrimaryRadar::setNumWaitingPlanes(int numWaitingPlanes) {
	this->numWaitingPlanes = numWaitingPlanes;
}

std::vector<void*> PrimaryRadar::getPlanePtrs() const {
	return planePtrs;
}

void PrimaryRadar::setPlanePtrs(std::vector<void*> planePtrs) {
	this->planePtrs = planePtrs;
}

Timer* PrimaryRadar::getTimer() const {
	return timer;
}

void PrimaryRadar::setTimer(Timer *timer) {
	this->timer = timer;
}

std::vector<std::string> PrimaryRadar::getWaitingFileNames() const {
	return waitingFileNames;
}

void PrimaryRadar::setWaitingFileNames(std::vector<std::string> waitingFileNames) {
	this->waitingFileNames = waitingFileNames;
}

void *PrimaryRadar::startPrimaryRadar(void *context) {
	((PrimaryRadar *)context)->OperatePrimaryRadar();
	return NULL;
}

int PrimaryRadar::initialize(int numberOfPlanes) {
	int rc = pthread_attr_init(&attr);
	if (rc) {
		printf("ERROR, RC from pthread_attr_init() is %d \n", rc);
	}

	rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (rc) {
		printf("ERROR; RC from pthread_attr_setdetachstate() is %d \n", rc);
	}

	// open list of waiting planes shm
	shm_waitingPlanes = shm_open("waiting_planes", O_RDWR, 0666);
	if (shm_waitingPlanes == -1) {
		perror("in shm_open() PSR");
		exit(1);
	}

	// map waiting planes shm
	waitingPlanesPtr = mmap(0, SIZE_SHM_PSR, PROT_READ | PROT_WRITE, MAP_SHARED, shm_waitingPlanes, 0);
	if (waitingPlanesPtr == MAP_FAILED) {
		perror("in map() PSR");
		exit(1);
	}

	std::string fileDataBuffer = "";
	std::vector<std::string> waitFile = this->getWaitingFileNames();

	for (int i = 0; i < SIZE_SHM_PSR; i++) {
		char readChar = *((char *)waitingPlanesPtr + i);
		if (readChar == ',') {
			waitFile.push_back(fileDataBuffer);
			int shm_plane = shm_open(fileDataBuffer.c_str(), O_RDONLY, 0666);
			if (shm_plane == -1) {
				perror("in shm_open() Primary Radar plane");
				exit(1);
			}

			// map memory for current plane
			void *ptr = mmap(0, SIZE_SHM_PLANES, PROT_READ, MAP_SHARED, shm_plane, 0);
			if (ptr == MAP_FAILED) {
				perror("in map() Primary Radar");
				exit(1);
			}
			planePtrs.push_back(ptr);
			fileDataBuffer = "";
			continue;
		} else if (readChar == ';') {
			waitFile.push_back(fileDataBuffer);

			// open shm for current plane
			int shm_plane = shm_open(fileDataBuffer.c_str(), O_RDONLY, 0666);
			if (shm_plane == -1) {
				perror("in shm_open() PSR plane");

				exit(1);
			}

			// map memory for current plane
			void *ptr = mmap(0, SIZE_SHM_PLANES, PROT_READ, MAP_SHARED, shm_plane, 0);
			if (ptr == MAP_FAILED) {
				perror("in map() PSR");
				exit(1);
			}

			planePtrs.push_back(ptr);
			break;
		}

		fileDataBuffer += readChar;
	}
	this->setWaitingFileNames(waitFile);

	std::cout << "Waiting File Name Count" << this->getWaitingFileNames().size() << "\n";

	shm_flyingPlanes = shm_open("flying_planes", O_RDWR, 0666);
	if (shm_flyingPlanes == -1) {
		perror("in shm_open() PSR: airspace");
		exit(1);
	}

	flyingPlanesPtr = mmap(0, SIZE_SHM_SSR, PROT_READ | PROT_WRITE, MAP_SHARED, shm_flyingPlanes, 0);
	if (flyingPlanesPtr == MAP_FAILED) {
		printf("map failed airspace\n");
		return -1;
	}

	shm_period = shm_open("period", O_RDONLY, 0666);
	if (shm_period == -1) {
		perror("in shm_open() PSR: period");
		exit(1);
	}

	periodPtr = mmap(0, SIZE_SHM_PERIOD, PROT_READ, MAP_SHARED, shm_period, 0);
	if (periodPtr == MAP_FAILED) {
		perror("in map() PSR: period");
		exit(1);
	}

	return 0;
}

void *PrimaryRadar::OperatePrimaryRadar(void) {
	time(&at);
	// create channel to communicate with timer
	int chid = ChannelCreate(0);
	if (chid == -1) {
		std::cout << "couldn't create channel!\n";
	}

	Timer *newTimer = new Timer(chid);
	this->setTimer(newTimer) ;
	this->getTimer()->setTimer(PRIMARY_SOURCE_RADAR_PERIOD, PRIMARY_SOURCE_RADAR_PERIOD);

	int receiveId;
	Message message;

	while (true) {
		if (receiveId == 0) {
			// lock mutex
			pthread_mutex_lock(&mutex);

			this->updatePeriod();
			bool move = this->readWaitingPlanes();
			if (move) {
				this->writeFlyingPlanes();
			}

			this->getFlyingFileNames().clear();

			// unlock mutex
			pthread_mutex_unlock(&mutex);

			// check for PSR termination
			if (this->getNumWaitingPlanes() <= 0) {
				std::cout << "Primary radar is done\n";
				time(&finishTime);
				double execTime = difftime(finishTime, startTime);
				std::cout << "PSR execution time: " << execTime << " seconds\n";
				ChannelDestroy(chid);
				return 0;
			}
		}
		receiveId = MsgReceive(chid, &message, sizeof(message), NULL);
	}

	ChannelDestroy(chid);

	return 0;
}


void PrimaryRadar::updatePeriod() {
	int newPeriod = atoi((char *)periodPtr);
	if (newPeriod != this->getCurrPeriod()) {
		this->setCurrPeriod(newPeriod);
		this->getTimer()->setTimer(currPeriod, currPeriod);
	}
}


bool PrimaryRadar::readWaitingPlanes() {
	// Initialize Values
	std::vector<std::string> waitPlanes = this->getWaitingFileNames();
	std::vector<std::string> flyingPlanes = this->getFlyingFileNames();
	bool move = false;
	int i = 0;
	auto it = planePtrs.begin();
	while (it != planePtrs.end()) {
		int j = 0;
		for (; j < 4; j++) {
			if (*((char *)*it + j) != ',') {
				break;
			}
		}

		// extract arrival time
		int curr_arrival_time = atoi((char *)(*it) + j + 1);
		time(&et);
		double t_current = difftime(et, at);
		if (curr_arrival_time <= t_current) {
			if(!this->getWaitingFileNames().empty()){
				move = true;
				flyingPlanes.push_back(waitPlanes.at(i));
				waitPlanes.erase(waitPlanes.begin() + i);
				it = planePtrs.erase(it);
				this->setNumWaitingPlanes(waitPlanes.size());
			}

		}
		else {
			i++; // only increment if no plane to transfer
			++it;
		}
	}
	this->setWaitingFileNames(waitPlanes);
	this->setFlyingFileNames(flyingPlanes);
	return move;
}

void PrimaryRadar::writeFlyingPlanes() {
	std::string currentAirspace = "";
	std::string currentPlane = "";

	// read current to flying planes shm
	int i = 0;
	while (i < SIZE_SHM_SSR) {
		char readChar = *((char *)flyingPlanesPtr + i);

		if (readChar == ';') { // Terminate
			if (i == 0) {
				break;
			}

			// check if plane already in list
			bool inList = true;
			for (std::string name : this->getFlyingFileNames()) {
				if (currentPlane == name) {
					inList = false;
					break;
				}
			}

			if (inList) {
				currentAirspace += currentPlane;
				currentAirspace += ',';
			}

			break;
		}
		else if (readChar == ',') {
			// check if plane already in list
			bool inList = true;
			for (std::string name : this->getFlyingFileNames()) {
				if (currentPlane == name) {
					inList = false;
					break;
				}
			}

			if (inList) {
				currentAirspace += currentPlane;
				currentAirspace += ',';
			}

			currentPlane = "";
			i++;
			continue;
		}

		currentPlane += readChar;
		i++;
	}

	// add planes to transfer buffer
	i = 0;
	for (std::string filename : this->getFlyingFileNames()) {
		if (i == 0) {
			currentAirspace += filename;
			i++;
		} else {
			currentAirspace += ",";
			currentAirspace += filename;
		}
	}
	currentAirspace += ";";
	// write new flying planes list to shm
	sprintf((char *)flyingPlanesPtr, "%s", currentAirspace.c_str());
}
