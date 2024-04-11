#include "ComputerSystem.h"

// contructor
ComputerSystem::ComputerSystem(int planeCount) {
	this->planeCount = planeCount;
	this->currentPeriod = CS_PERIOD;
	this->initialize();
}

// destructor
ComputerSystem::~ComputerSystem() {
	shm_unlink("airspace");
	shm_unlink("display");
	for (std::string name : this->getCommunicationNames()) {
		shm_unlink(name.c_str());
	}
	pthread_mutex_destroy(&mutex);

	delete (timer);
}

// start computer system
void ComputerSystem::start() {
	time(&startTime);
	if (pthread_create(&computerSystemThread, &attr, &ComputerSystem::startThread, (void *)this) != EOK) {
		computerSystemThread = 0;
	}
}

// join computer system thread
int ComputerSystem::stop() {
	pthread_join(computerSystemThread, NULL);
	return 0;
}

std::vector<std::string> ComputerSystem::getCommunicationNames() const {
	return communicationNames;
}

void ComputerSystem::setCommunicationNames(std::vector<std::string> commNames) {
	this->communicationNames = commNames;
}

std::vector<void*> ComputerSystem::getCommunicationPtrs() const {
	return communicationPtr;
}

void ComputerSystem::setCommunicationPtrs(std::vector<void*> commPtrs) {
	this->communicationPtr = commPtrs;
}

int ComputerSystem::getCurrentPeriod() const {
	return currentPeriod;
}

void ComputerSystem::setCurrentPeriod(int currentPeriod) {
	this->currentPeriod = currentPeriod;
}

std::vector<plane*> ComputerSystem::getFlyingPlanesInfo() const {
	return flyingPlanesInfo;
}

void ComputerSystem::setFlyingPlanesInfo(std::vector<plane*> flyingPlanesInfo) {
	this->flyingPlanesInfo = flyingPlanesInfo;
}

int ComputerSystem::getPlaneCount() const {
	return planeCount;
}

void ComputerSystem::setPlaneCount(int planeCount) {
	this->planeCount = planeCount;
}

std::vector<trajectoryPrediction*> ComputerSystem::getTrajectoryPredictions() const {
	return trajectoryPredictions;
}

void ComputerSystem::setTrajectoryPredictions(std::vector<trajectoryPrediction*> trajectoryPredictions) {
	this->trajectoryPredictions = trajectoryPredictions;
}

// entry point for execution function
void *ComputerSystem::startThread(void *context) {
	((ComputerSystem *)context)->computePath();
	return NULL;
}

int ComputerSystem::initialize() {
	// Setup Threads
	int receive = pthread_attr_init(&attr);
	if (receive) {
		printf("ERROR, RC from pthread_attr_init() is %d \n", receive);
	}

	receive = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (receive) {
		printf("ERROR; RC from pthread_attr_setdetachstate() is %d \n", receive);
	}

	// shared memory members
	shm_airspace = shm_open("airspace", O_RDONLY, 0666);
	if (shm_airspace == -1) {
		perror("in Computer System shm_open() airspace");
		exit(1);
	}

	airspacePtr = mmap(0, SIZE_SHM_AIRSPACE, PROT_READ, MAP_SHARED, shm_airspace, 0);
	if (airspacePtr == MAP_FAILED) {
		perror("in Computer System map() airspace");
		exit(1);
	}

	shm_period = shm_open("period", O_RDWR, 0666);
	if (shm_period == -1) {
		perror("in shm_open() PSR: period");
		exit(1);
	}

	periodPtr = mmap(0, SIZE_SHM_PERIOD, PROT_READ | PROT_WRITE, MAP_SHARED,shm_period, 0);
	if (periodPtr == MAP_FAILED) {
		perror("in map() PSR: period");
		exit(1);
	}

	shm_display = shm_open("display", O_RDWR, 0666);
	if (shm_display == -1) {
		perror("in Computer System shm_open() display");
		exit(1);
	}


	displayPtr = mmap(0, SIZE_SHM_DISPLAY, PROT_READ | PROT_WRITE, MAP_SHARED,shm_display, 0);
	if (displayPtr == MAP_FAILED) {
		perror("in Computer System map() display");
		exit(1);
	}


	int shm_comm = shm_open("waiting_planes", O_RDONLY, 0666);
	if (shm_comm == -1) {
		perror("in Computer System shm_open()");
		exit(1);
	}

	void *communicationPtr = mmap(0, SIZE_SHM_PSR, PROT_READ, MAP_SHARED, shm_comm, 0);
	if (communicationPtr == MAP_FAILED) {
		perror("in Computer System map()");
		exit(1);
	}

	// generate pointers to plane shms
	std::string buffer = "";

	for (int i = 0; i < SIZE_SHM_PSR; i++) {
		char readCharacter = *((char *)communicationPtr + i);

		if(readCharacter == ';'){
			int shm_plane = shm_open(buffer.c_str(), O_RDWR, 0666);
			if (shm_plane == -1) {
				perror("in Computer System shm_open() plane");
				exit(1);
			}

			// Map Plane Current Memory
			void *ptr = mmap(0, SIZE_SHM_PLANES, PROT_READ | PROT_WRITE, MAP_SHARED,shm_plane, 0);
			if (ptr == MAP_FAILED) {
				perror("in Computer System map() plane");
				exit(1);
			}
			this->getCommunicationPtrs().push_back(ptr);
			this->getCommunicationNames().push_back(buffer);
			break;
		}
		else if(readCharacter == ','){
			int shm_plane = shm_open(buffer.c_str(), O_RDWR, 0666);
			if (shm_plane == -1) {
				perror("in Computer System shm_open() plane");
				exit(1);
			}

			// Map Plane Current Memory
			void *ptr = mmap(0, SIZE_SHM_PLANES, PROT_READ | PROT_WRITE, MAP_SHARED,shm_plane, 0);
			if (ptr == MAP_FAILED) {
				perror("in Computer System map() plane");
				exit(1);
			}
			this->getCommunicationPtrs().push_back(ptr);
			this->getCommunicationNames().push_back(buffer);

			// Update Buffer
			buffer = "";
			continue;
		}
		buffer += readCharacter;
	}
	return 0;
}

void *ComputerSystem::computePath() {
	// create channel to communicate with timer
	int channelId = ChannelCreate(0);
	if (channelId == -1) {
		std::cout << "couldn't create channel\n";
	}

	// Setup Timer & Message
	int receiveId;
	Message msg;
	Timer *newTimer = new Timer(channelId);
	newTimer->setTimer(CS_PERIOD, CS_PERIOD);
	this->timer = newTimer;


	// Logging commands
	std::ofstream outputStream("command");

	bool done = false;
	while (true) {
		if (receiveId == 0) {
			pthread_mutex_lock(&mutex);

			done = this->readAirspace(); // read airspace shm
			this->clearPredictions();// prune predictions
			this->findViolations(&outputStream); // compute airspace violations for all planes in the airspace
			this->writeAndDisplay(); // send airspace info to display / prune airspace info
			this->updatePeriod(channelId); // update the period based on the traffic

			pthread_mutex_unlock(&mutex);
		}

		// termination check
		if (this->getPlaneCount() <= 0 || done) {
			std::cout << "computer system done\n";
			time(&finishTime);
			std::cout << "computer system execution time: " << difftime(finishTime, startTime) << " s\n";
			sprintf((char *)displayPtr, "terminated");
			ChannelDestroy(channelId);
			return 0;
		}

		receiveId = MsgReceive(channelId, &msg, sizeof(msg), NULL);
	}

	outputStream.close();
	ChannelDestroy(channelId);
	return 0;
}

bool ComputerSystem::readAirspace() {
	// Initialize Variables
	int id, arrTime, pos[3], vel[3];
	std::string readBuffer = "";
	int j = 0;

	// Get Plane Locations
	for (int i = 0; i < SIZE_SHM_AIRSPACE; i++) {
		char readCharacter = *((char *)airspacePtr + i);
		if (readCharacter == 't') {
			return true;
		}
		// end of airspace shm, line termination found
		if (readCharacter == ';') {
			// i=0 no planes found
			if (i == 0) {
				break;
			}

			// load last field in buffer
			vel[2] = atoi(readBuffer.c_str());

			// check if already in airspace, if yes update with current data
			bool inList = false;
			for (plane *plane : this->getFlyingPlanesInfo()) {
				if (plane->id == id) {
					plane->posX = pos[0];plane->posY = pos[1];plane->posZ = pos[2];
					plane->velX = vel[0];plane->velY = vel[1];plane->velZ= vel[2];
					plane->keep = true;
					inList = true;

					for (trajectoryPrediction *prediction : this->getTrajectoryPredictions()) {
						if (prediction->id == plane->id) {

							// Check Time
							bool predictionX = prediction->time >= (int)prediction->posX.size();
							bool predictionY = prediction->time >= (int)prediction->posX.size();
							bool predictionZ = prediction->time >= (int)prediction->posX.size();
							if (predictionX || predictionY || predictionZ) {
								break;
							}

							// Check Position
							predictionX = prediction->posX.at(prediction->time) == plane->posX;
							predictionY = prediction->posY.at(prediction->time) == plane->posY;
							predictionZ = prediction->posZ.at(prediction->time) == plane->posZ;


							if (predictionX && predictionY && predictionZ) {
								// Get Last X Position
								auto it = prediction->posX.end();
								it -= 2;
								int lastX = *it + (this->getCurrentPeriod() / 1000000) * plane->velX;

								// Get Last Y Position
								it = prediction->posY.end();
								it -= 2;
								int lastY = *it + (this->getCurrentPeriod() / 1000000) * plane->velY;

								// Get Last Z Position
								it = prediction->posZ.end();
								it -= 2;
								int lastZ = *it + (this->getCurrentPeriod() / 1000000) * plane->velZ;

								// check if still within limits
								bool outOfBoundsX = lastX > MAX_X_AIRSPACE || lastX < MIN_X_AIRSPACE;
								bool outOfBoundsY = lastY > MAX_Y_AIRSPACE || lastY < MIN_Y_AIRSPACE;
								bool outOfBoundsZ = lastZ > MAX_Z_AIRSPACE || lastZ < MIN_Z_AIRSPACE;

								if(outOfBoundsX || outOfBoundsY || outOfBoundsZ){
									prediction->keep = true;
									prediction->time++;
									break;
								}


								// Add New Predictions
								prediction->posX.pop_back();
								prediction->posX.push_back(lastX);
								prediction->posX.push_back(-1);

								prediction->posY.pop_back();
								prediction->posY.push_back(lastY);
								prediction->posY.push_back(-1);

								prediction->posZ.pop_back();
								prediction->posZ.push_back(lastZ);
								prediction->posZ.push_back(-1);

								prediction->keep = true;
								prediction->time++;
							}
							else {
								prediction->posX.clear();
								prediction->posY.clear();
								prediction->posZ.clear();

								for (int i = 0; i < (180 / (this->getCurrentPeriod() / 1000000)); i++) {
									int currX = plane->posX + i * (this->getCurrentPeriod() / 1000000) * plane->velX;
									int currY = plane->posY + i * (this->getCurrentPeriod() / 1000000) * plane->velY;
									int currZ = plane->posZ + i * (this->getCurrentPeriod() / 1000000) * plane->velZ;

									prediction->posX.push_back(currX);
									prediction->posY.push_back(currY);
									prediction->posZ.push_back(currZ);

									bool outOfBoundsX = currX > MAX_X_AIRSPACE || currX < MIN_X_AIRSPACE;
									bool outOfBoundsY = currY > MAX_Y_AIRSPACE || currY < MIN_Y_AIRSPACE;
									bool outOfBoundsZ = currZ > MAX_Z_AIRSPACE || currZ < MIN_Z_AIRSPACE;

									if(outOfBoundsX || outOfBoundsY || outOfBoundsZ){
										break;
									}

								}
								// set termination character
								prediction->posX.push_back(-1);
								prediction->posY.push_back(-1);
								prediction->posZ.push_back(-1);
								// set prediction index to next, keep for computation
								prediction->time = 1;
								prediction->keep = true;
							}
						}
					}
					break;
				}
			}

			// if plane was not already in the list, add it
			if (!inList) {
				// Add Flying Plane
				plane *currentAircraft = new plane();
				currentAircraft->id = id;
				currentAircraft->arrivalTime = arrTime;
				currentAircraft->posX = pos[0];currentAircraft->posY = pos[1];currentAircraft->posZ = pos[2];
				currentAircraft->velX = vel[0];currentAircraft->velY = vel[1];currentAircraft->velZ = vel[2];
				currentAircraft->keep = true;
				this->getFlyingPlanesInfo().push_back(currentAircraft);

				// Setup trajectory Prediction
				trajectoryPrediction *currentPrediction = new trajectoryPrediction();
				currentPrediction->id = currentAircraft->id;

				for (int i = 0; i < 180 / (this->getCurrentPeriod() / 1000000); i++) {
					int currX = currentAircraft->posX + i * (this->getCurrentPeriod() / 1000000) * currentAircraft->velX;
					int currY = currentAircraft->posY + i * (this->getCurrentPeriod() / 1000000) * currentAircraft->velY;
					int currZ = currentAircraft->posZ + i * (this->getCurrentPeriod() / 1000000) * currentAircraft->velZ;

					currentPrediction->posX.push_back(currX);
					currentPrediction->posY.push_back(currY);
					currentPrediction->posZ.push_back(currZ);

					bool outOfBoundsX = currX > MAX_X_AIRSPACE || currX < MIN_X_AIRSPACE;
					bool outOfBoundsY = currY > MAX_Y_AIRSPACE || currY < MIN_Y_AIRSPACE;
					bool outOfBoundsZ = currZ > MAX_Z_AIRSPACE || currZ < MIN_Z_AIRSPACE;

					if(outOfBoundsX || outOfBoundsY || outOfBoundsZ){
						break;
					}

				}
				// set termination character
				currentPrediction->posX.push_back(-1);
				currentPrediction->posY.push_back(-1);
				currentPrediction->posZ.push_back(-1);
				currentPrediction->time = 1;
				currentPrediction->keep = true;
				this->getTrajectoryPredictions().push_back(currentPrediction);
				break;
			}
			break;
		}
		// found plane in airspace (plane termination character)
		else if (readCharacter == '/') {
			// load last value in buffer
			vel[2] = atoi(readBuffer.c_str());

			// check if already in airspace, if yes update with current data
			bool inList = false;
			for (plane *plane : this->getFlyingPlanesInfo()) {

				if (plane->id == id) {

					// if found, update with current info
					plane->posX = pos[0];plane->posY = pos[1];plane->posZ = pos[2];
					plane->velX = vel[0];plane->velY = vel[1];plane->velZ = vel[2];
					plane->keep = true;
					inList = true;

					for (trajectoryPrediction *prediction : this->getTrajectoryPredictions()) {
						if (prediction->id == plane->id) {

							// if end of prediction reached, break
							bool predictionX = prediction->time >= (int)prediction->posX.size();
							bool predictionY = prediction->time >= (int)prediction->posY.size();
							bool predictionZ = prediction->time >= (int)prediction->posZ.size();

							if (predictionX || predictionY || predictionZ) {
								break;
							}

							// check positions are the same
							predictionX = prediction->posX.at(prediction->time) == plane->posX;
							predictionY = prediction->posY.at(prediction->time) == plane->posY;
							predictionZ = prediction->posZ.at(prediction->time) == plane->posZ;

							if (predictionX && predictionY && predictionZ) {
								// Setup Last X values
								auto it = prediction->posX.end();
								it -= 2;
								int lastX = *it + (this->getCurrentPeriod() / 1000000) * plane->velX;

								it = prediction->posY.end();
								it -= 2;
								int lastY = *it + (this->getCurrentPeriod() / 1000000) * plane->velY;

								it = prediction->posZ.end();
								it -= 2;
								int lastZ = *it + (this->getCurrentPeriod() / 1000000) * plane->velZ;

								// check if still within limits
								bool outOfXBound = lastX > MAX_X_AIRSPACE || lastX < MIN_X_AIRSPACE;
								bool outOfYBound = lastY > MAX_Y_AIRSPACE || lastY < MIN_Y_AIRSPACE;
								bool outOfZBound = lastZ > MAX_Z_AIRSPACE || lastZ < MIN_Z_AIRSPACE;

								if (outOfXBound || outOfYBound || outOfZBound) {
									prediction->keep = true;
									prediction->time++;
									break;
								}

								// add new last prediction
								prediction->posX.pop_back();
								prediction->posX.push_back(lastX);
								prediction->posX.push_back(-1);

								prediction->posY.pop_back();
								prediction->posY.push_back(lastY);
								prediction->posY.push_back(-1);

								prediction->posZ.pop_back();
								prediction->posZ.push_back(lastZ);
								prediction->posZ.push_back(-1);

								// keep for computation and increment index
								prediction->keep = true;
								prediction->time++;
							}
							// update the prediction
							else {
								prediction->posX.clear();
								prediction->posY.clear();
								prediction->posZ.clear();

								for (int i = 0; i < 180 / (currentPeriod / 1000000); i++) {
									int currX = plane->posX + i * (currentPeriod / 1000000) * plane->velX;
									int currY = plane->posY + i * (currentPeriod / 1000000) * plane->velY;
									int currZ = plane->posZ + i * (currentPeriod / 1000000) * plane->velZ;

									prediction->posX.push_back(currX);
									prediction->posY.push_back(currY);
									prediction->posZ.push_back(currZ);

									bool outOfXBound = currX > MAX_X_AIRSPACE || currX < MIN_X_AIRSPACE;
									bool outOfYBound = currY > MAX_Y_AIRSPACE || currY < MIN_Y_AIRSPACE;
									bool outOfZBound = currZ > MAX_Z_AIRSPACE || currZ < MIN_Z_AIRSPACE;

									if (outOfXBound || outOfYBound || outOfZBound) {
										break;
									}
								}
								// set termination character
								prediction->posX.push_back(-1);
								prediction->posY.push_back(-1);
								prediction->posZ.push_back(-1);
								prediction->time = 1;
								prediction->keep = true;
							}
						}
					}

					break;
				}
			}

			// Adds new List if it's not already in
			if (!inList) {
				plane *currentAircraft = new plane();
				currentAircraft->id = id;
				currentAircraft->arrivalTime = arrTime;
				currentAircraft->posX = pos[0];currentAircraft->posY = pos[1];currentAircraft->posZ = pos[2];
				currentAircraft->velX = vel[0];currentAircraft->velY = vel[1];currentAircraft->velZ = vel[2];
				currentAircraft->keep = true;
				this->getFlyingPlanesInfo().push_back(currentAircraft);

				trajectoryPrediction *currentPrediction = new trajectoryPrediction();
				currentPrediction->id = currentAircraft->id;

				for (int i = 0; i < 180 / (currentPeriod / 1000000); i++) {
					int currX = currentAircraft->posX + i * (currentPeriod / 1000000) * currentAircraft->velX;
					int currY = currentAircraft->posY + i * (currentPeriod / 1000000) * currentAircraft->velY;
					int currZ = currentAircraft->posZ + i * (currentPeriod / 1000000) * currentAircraft->velZ;

					currentPrediction->posX.push_back(currX);
					currentPrediction->posY.push_back(currY);
					currentPrediction->posZ.push_back(currZ);

					bool outOfXBound = currX > MAX_X_AIRSPACE || currX < MIN_X_AIRSPACE;
					bool outOfYBound = currY > MAX_Y_AIRSPACE || currY < MIN_Y_AIRSPACE;
					bool outOfZBound = currZ > MAX_Z_AIRSPACE || currZ < MIN_Z_AIRSPACE;
					if (outOfXBound || outOfYBound || outOfZBound) {
						break;
					}
				}

				// Set terminal Character
				currentPrediction->posX.push_back(-1);
				currentPrediction->posY.push_back(-1);
				currentPrediction->posZ.push_back(-1);
				currentPrediction->time = 1;
				currentPrediction->keep = true;
				this->getTrajectoryPredictions().push_back(currentPrediction);
			}

			readBuffer = "";
			j = 0;
			continue;
		}
		// found next data field in current plane
		else if (readCharacter == ',') {

			switch (j) {
			// add whichever character the index j has arrived to
			case 0:
				id = atoi(readBuffer.c_str());
				readBuffer = "";
				j++;
				continue;
			case 1:
				arrTime = atoi(readBuffer.c_str());
				readBuffer = "";
				j++;
				continue;
			case 2:
				pos[0] = atoi(readBuffer.c_str());
				readBuffer = "";
				j++;
				continue;
			case 3:
				pos[1] = atoi(readBuffer.c_str());
				readBuffer = "";
				j++;
				continue;
			case 4:
				pos[2] = atoi(readBuffer.c_str());
				readBuffer = "";
				j++;
				continue;
			case 5:
				vel[0] = atoi(readBuffer.c_str());
				readBuffer = "";
				j++;
				continue;
			case 6:
				vel[1] = atoi(readBuffer.c_str());
				readBuffer = "";
				j++;
				continue;
			default:
				// just reset buffer
				readBuffer = "";
				break;
			}
		}
		readBuffer += readCharacter;
	}
	return false;
}

void ComputerSystem::clearPredictions() {
	int j = 0;
	auto predictionPtr = this->getTrajectoryPredictions().begin();
	while (predictionPtr != this->getTrajectoryPredictions().end()) {
		bool temp = (*predictionPtr)->keep;

		// check if plane was terminated
		if (!temp) {
			delete this->getTrajectoryPredictions().at(j);
			predictionPtr = this->getTrajectoryPredictions().erase(predictionPtr);
		}
		else {
			for (int i = (*predictionPtr)->time - 1; i < (*predictionPtr)->time + (180 / (this->getCurrentPeriod() / 1000000)); i++) {
				if ((*predictionPtr)->posX.at(i) == -1 || (*predictionPtr)->posY.at(i) == -1 || (*predictionPtr)->posZ.at(i) == -1) {
					break;
				}
			}

			(*predictionPtr)->keep = false; // if found next time, this will become true again
			j++;
			++predictionPtr;
		}
	}
}

void ComputerSystem::findViolations(std::ofstream *out) {
	auto itIndex = this->getTrajectoryPredictions().begin();
	while (itIndex != this->getTrajectoryPredictions().end()) {
		auto itNext = itIndex;
		++itNext;
		while (itNext != this->getTrajectoryPredictions().end()) {
			int j = (*itNext)->time - 1;
			for (int i = (*itIndex)->time - 1;i < (*itIndex)->time - 1 + (180 / (this->getCurrentPeriod() / 1000000)); i++) {
				int currX = (*itIndex)->posX.at(i);
				int currY = (*itIndex)->posY.at(i);
				int currZ = (*itIndex)->posZ.at(i);
				int compX = (*itNext)->posX.at(j);
				int compY = (*itNext)->posY.at(j);
				int compZ = (*itNext)->posZ.at(j);

				if (currX == -1 || currY == -1 || currZ == -1 || compX == -1 || compY == -1 || compZ == -1) {
					break;
				}

				// Positions
				bool currentX = abs(currX - compX) <= 3000;
				bool currentY = abs(currY - compY) <= 3000;
				bool currentZ = abs(currZ - compZ) <= 1000;

				if (currentX && currentY && currentZ) {
					std::cout << "airspace violation detected between planes "
							<< (*itIndex)->id << " and " << (*itNext)->id
							<< " at time current + " << i * (currentPeriod) / 1000000
							<< "\n";

					// Setup Communication
					bool currCommunications = false;
					bool compCommunications = false;
					for (void *comm : this->getCommunicationPtrs()) {
						int communicationId = atoi((char *)comm);
						if (communicationId == (*itIndex)->id) {
							int k = 0;
							char readChar;
							for (; k < SIZE_SHM_PLANES; k++) {
								readChar = *((char *)comm + k);
								if (readChar == ';') {
									break;
								}
							}
							// set pointer after plane termination
							k++;
							std::string command = "z,200;";
							sprintf((char *)comm + k, "%s", command.c_str());
							std::streambuf *coutbuf = std::cout.rdbuf(); // save old buf
							std::cout.rdbuf(out->rdbuf()); // redirect std::cout to command
							std::cout << "Command: Plane " << (*itIndex)->id << " increases altitude by 200 feet" << std::endl;
							std::cout.rdbuf(coutbuf); // reset to standard output again
							currCommunications = true;
						}
						if (communicationId == (*itNext)->id) {
							// find command index in plane shm
							int k = 0;
							char readChar;
							for (; k < SIZE_SHM_PLANES; k++) {
								readChar = *((char *)comm + k);
								if (readChar == ';') {
									break;
								}
							}
							// set pointer after plane termination
							k++;
							std::string command = "z,-200;";
							sprintf((char *)comm + k, "%s", command.c_str());
							std::streambuf *coutbuf = std::cout.rdbuf(); // save old buf
							std::cout.rdbuf(out->rdbuf()); // redirect std::cout to command
							std::cout << "Command: Plane " << (*itNext)->id << " decreases altitude by 200 feet" << std::endl;
							std::cout.rdbuf(coutbuf); // reset to standard output again
							compCommunications = true;
						}
						if (currCommunications && compCommunications) {
							break;
						}
					}
					// set associate craft info request to true, display height
					currCommunications = false;
					compCommunications = false;
					for (plane *craft : this->getFlyingPlanesInfo()) {
						if (craft->id == (*itIndex)->id) {
							craft->moreInfo = true;
							craft->commandCounter = NUM_PRINT;
							currCommunications = true;
						}
						if (craft->id == (*itNext)->id) {
							craft->moreInfo = true;
							craft->commandCounter = NUM_PRINT;
							compCommunications = true;
						}
						if (currCommunications && compCommunications) {
							break;
						}
					}
					break;
				}
				j++;
			}
			++itNext;
		}
		++itIndex;
	}
}

void ComputerSystem::writeAndDisplay() {

	std::string displayBuffer = "", currentPlaneBuffer = "";


	int i = 0;
	auto it = this->getFlyingPlanesInfo().begin();
	while (it != this->getFlyingPlanesInfo().end()) {
		bool temp = (*it)->keep; // check if plane was terminated

		if (!temp) {
			delete this->getFlyingPlanesInfo().at(i);
			it = this->getFlyingPlanesInfo().erase(it);
			this->setPlaneCount(this->getPlaneCount()-1);
		} else {

			displayBuffer = displayBuffer +
					std::to_string((*it)->id) + "," +
					std::to_string((*it)->posX) + "," +
					std::to_string((*it)->posY) + "," +
					std::to_string((*it)->posZ) + ",";

			// check if more info requested
			if ((*it)->moreInfo) {
				(*it)->commandCounter--;
				if ((*it)->commandCounter <= 0) {
					(*it)->moreInfo = false;
				}
				displayBuffer += "1/";
			} else {
				displayBuffer += "0/";
			}

			(*it)->keep = false; // if found next time, this will become true again

			// only increment if no plane to remove
			i++;
			++it;
		}
	}
	// termination character
	displayBuffer = displayBuffer + ";";

	sprintf((char *)displayPtr, "%s", displayBuffer.c_str());
}

void ComputerSystem::updatePeriod(int chid) {
	int newPeriod = 0;
	int size = this->getTrajectoryPredictions().size();

	// Setup Trajectory Size
	if (size > 50 && size <= 200) {
		// set period overdrive
		newPeriod = 1000000;
	}
	else if (size > 20 && size <= 50) {
		// set period high
		newPeriod = 2000000;
	}
	else if (size > 5 && size <= 20) {
		// set period medium
		newPeriod = 3000000;
	}
	else if (size <= 5) {
		// set period low
		newPeriod = 5000000;
	}

	// Update period
	if (this->getCurrentPeriod() != newPeriod) {
		this->setCurrentPeriod(newPeriod);
		std::string CSPeriod = std::to_string(this->getCurrentPeriod());
		sprintf((char *)periodPtr, CSPeriod.c_str());
		timer->setTimer(this->getCurrentPeriod(), this->getCurrentPeriod());
	}
}
