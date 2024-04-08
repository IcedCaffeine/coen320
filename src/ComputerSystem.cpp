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
	for (std::string name : this->getCommNames()) {
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

std::vector<std::string> ComputerSystem::getCommNames() const {
	return commNames;
}

void ComputerSystem::setCommNames(std::vector<std::string> commNames) {
	this->commNames = commNames;
}

std::vector<void*> ComputerSystem::getCommPtrs() const {
	return commPtrs;
}

void ComputerSystem::setCommPtrs(std::vector<void*> commPtrs) {
	this->commPtrs = commPtrs;
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
	((ComputerSystem *)context)->calculateTrajectories();
	return NULL;
}

int ComputerSystem::initialize() {
	// initialize thread members

	// set threads in detached state
	int rc = pthread_attr_init(&attr);
	if (rc) {
		printf("ERROR, RC from pthread_attr_init() is %d \n", rc);
	}

	rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	if (rc) {
		printf("ERROR; RC from pthread_attr_setdetachstate() is %d \n", rc);
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


	periodPtr = mmap(0, SIZE_SHM_PERIOD, PROT_READ | PROT_WRITE, MAP_SHARED,
			shm_period, 0);
	if (periodPtr == MAP_FAILED) {
		perror("in map() PSR: period");
		exit(1);
	}


	shm_display = shm_open("display", O_RDWR, 0666);
	if (shm_display == -1) {
		perror("in compsys shm_open() display");
		exit(1);
	}


	displayPtr = mmap(0, SIZE_SHM_DISPLAY, PROT_READ | PROT_WRITE, MAP_SHARED,
			shm_display, 0);
	if (displayPtr == MAP_FAILED) {
		perror("in copmsys map() display");
		exit(1);
	}


	int shm_comm = shm_open("waiting_planes", O_RDONLY, 0666);
	if (shm_comm == -1) {
		perror("in compsys shm_open() comm");
		exit(1);
	}

	void *commPtr = mmap(0, SIZE_SHM_PSR, PROT_READ, MAP_SHARED, shm_comm, 0);
	if (commPtr == MAP_FAILED) {
		perror("in compsys map() comm");
		exit(1);
	}

	// generate pointers to plane shms
	std::string buffer = "";

	for (int i = 0; i < SIZE_SHM_PSR; i++) {
		char readChar = *((char *)commPtr + i);

		if (readChar == ',') {

			// open shm for current plane
			int shm_plane = shm_open(buffer.c_str(), O_RDWR, 0666);
			if (shm_plane == -1) {
				perror("in compsys shm_open() plane");

				exit(1);
			}

			// map memory for current plane
			void *ptr = mmap(0, SIZE_SHM_PLANES, PROT_READ | PROT_WRITE, MAP_SHARED,
					shm_plane, 0);
			if (ptr == MAP_FAILED) {
				perror("in compsys map() plane");
				exit(1);
			}
			this->getCommPtrs().push_back(ptr);
			this->getCommNames().push_back(buffer);

			buffer = "";
			continue;
		} else if (readChar == ';') {
			// open shm for current plane
			int shm_plane = shm_open(buffer.c_str(), O_RDWR, 0666);
			if (shm_plane == -1) {
				perror("in compsys shm_open() plane");

				exit(1);
			}

			// map memory for current plane
			void *ptr = mmap(0, SIZE_SHM_PLANES, PROT_READ | PROT_WRITE, MAP_SHARED,
					shm_plane, 0);
			if (ptr == MAP_FAILED) {
				perror("in compsys map() plane");
				exit(1);
			}
			this->getCommPtrs().push_back(ptr);
			this->getCommNames().push_back(buffer);

			break;
		}

		buffer += readChar;
	}

	return 0;
}

void *ComputerSystem::calculateTrajectories() {
	// create channel to communicate with timer
	int chid = ChannelCreate(0);
	if (chid == -1) {
		std::cout << "couldn't create channel\n";
	}

	Timer *newTimer = new Timer(chid);
	timer = newTimer;
	timer->setTimer(CS_PERIOD, CS_PERIOD);

	int receiveId;
	Message msg;
	// Logging commands
	std::ofstream outputStream("command");

	bool done = false;
	while (1) {
		if (receiveId == 0) {
			pthread_mutex_lock(&mutex);

			done = this->readAirspace(); // read airspace shm
			this->cleanPredictions();// prune predictions
			this->computeViolations(&outputStream); // compute airspace violations for all planes in the airspace
			this->writeAndDisplay(); // send airspace info to display / prune airspace info
			this->updatePeriod(chid); // update the period based on the traffic

			pthread_mutex_unlock(&mutex);
		}

		// termination check
		if (this->getPlaneCount() <= 0 || done) {
			std::cout << "computer system done\n";
			time(&finishTime);
			double execTime = difftime(finishTime, startTime);
			std::cout << "computer system execution time: " << execTime << " seconds\n";
			sprintf((char *)displayPtr, "terminated");
			ChannelDestroy(chid);
			return 0;
		}

		receiveId = MsgReceive(chid, &msg, sizeof(msg), NULL);
	}

	outputStream.close();

	ChannelDestroy(chid);

	return 0;
}

bool ComputerSystem::readAirspace() {
	std::string readBuffer = "";
	int j = 0;

	// variable buffer, these get overwritten as needed
	int id, arrTime, pos[3], vel[3];

	// find planes in airspace
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
			for (plane *craft : flyingPlanesInfo) {
				if (craft->id == id) {
					// if found, update with current info
					craft->posX = pos[0];
					craft->posX = pos[1];
					craft->posX = pos[2];
					craft->velX = vel[0];
					craft->velY = vel[1];
					craft->velZ= vel[2];
					// if found, keep, if not will be removed
					craft->keep = true;
					// it is already in the list, do not add new
					inList = true;

					for (trajectoryPrediction *prediction : trajectoryPredictions) {
						if (prediction->id == craft->id) {

							// if end of prediction reached, break
							if (prediction->time >= (int)prediction->posX.size() ||
									prediction->time >= (int)prediction->posY.size() ||
									prediction->time >= (int)prediction->posZ.size()) {
								break;
							}

							// check posx, posy and poz if same
							if (prediction->posX.at(prediction->time) == craft->posX &&
									prediction->posY.at(prediction->time) == craft->posY &&
									prediction->posZ.at(prediction->time) == craft->posZ) {
								// set prediction index to next
								// update last entry in predictions
								auto it = prediction->posX.end();
								it -= 2;
								int lastX = *it + (this->getCurrentPeriod() / 1000000) * craft->velX;
								it = prediction->posY.end();
								it -= 2;
								int lastY = *it + (this->getCurrentPeriod() / 1000000) * craft->velY;
								it = prediction->posZ.end();
								it -= 2;
								int lastZ = *it + (this->getCurrentPeriod() / 1000000) * craft->velZ;

								// check if still within limits
								bool outOfBounds = false;
								if (lastX > MAX_X_AIRSPACE || lastX < MIN_X_AIRSPACE) {
									outOfBounds = true;
								}
								if (lastY > MAX_Y_AIRSPACE || lastY < MIN_Y_AIRSPACE) {
									outOfBounds = true;
								}
								if (lastZ > MAX_Z_AIRSPACE || lastZ < MIN_Z_AIRSPACE) {
									outOfBounds = true;
								}
								// if not, just increment
								if (outOfBounds) {
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

								for (int i = 0; i < (180 / (this->getCurrentPeriod() / 1000000)); i++) {
									int currX = craft->posX + i * (this->getCurrentPeriod() / 1000000) * craft->velX;
									int currY = craft->posY + i * (this->getCurrentPeriod() / 1000000) * craft->velY;
									int currZ = craft->posZ + i * (this->getCurrentPeriod() / 1000000) * craft->velZ;

									prediction->posX.push_back(currX);
									prediction->posY.push_back(currY);
									prediction->posZ.push_back(currZ);

									bool outOfBounds = false;
									if (currX > MAX_X_AIRSPACE || currX < MIN_X_AIRSPACE) {
										outOfBounds = true;
									}
									if (currY > MAX_Y_AIRSPACE || currY < MIN_Y_AIRSPACE) {
										outOfBounds = true;
									}
									if (currZ > MAX_Z_AIRSPACE || currZ < MIN_Z_AIRSPACE) {
										outOfBounds = true;
									}
									if (outOfBounds) {
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
				// new pointer to struct, set members from read
				plane *currentAircraft = new plane();
				currentAircraft->id = id;
				currentAircraft->arrivalTime = arrTime;
				currentAircraft->posX = pos[0];
				currentAircraft->posY = pos[1];
				currentAircraft->posZ = pos[2];
				currentAircraft->velX = vel[0];
				currentAircraft->velY = vel[1];
				currentAircraft->velZ = vel[2];
				currentAircraft->keep = true; // keep for first computation
				this->getFlyingPlanesInfo().push_back(currentAircraft);

				trajectoryPrediction *currentPrediction = new trajectoryPrediction();

				currentPrediction->id = currentAircraft->id;

				for (int i = 0; i < 180 / (this->getCurrentPeriod() / 1000000); i++) {
					int currX = currentAircraft->posX + i * (this->getCurrentPeriod() / 1000000) * currentAircraft->velX;
					int currY = currentAircraft->posY + i * (this->getCurrentPeriod() / 1000000) * currentAircraft->velY;
					int currZ = currentAircraft->posZ + i * (this->getCurrentPeriod() / 1000000) * currentAircraft->velZ;

					currentPrediction->posX.push_back(currX);
					currentPrediction->posY.push_back(currY);
					currentPrediction->posZ.push_back(currZ);

					bool outOfBounds = false;
					if (currX > MAX_X_AIRSPACE || currX < MIN_X_AIRSPACE) {
						outOfBounds = true;
					}
					if (currY > MAX_Y_AIRSPACE || currY < MIN_Y_AIRSPACE) {
						outOfBounds = true;
					}
					if (currZ > MAX_Z_AIRSPACE || currZ < MIN_Z_AIRSPACE) {
						outOfBounds = true;
					}
					if (outOfBounds) {
						break;
					}
				}
				// set termination character
				currentPrediction->posX.push_back(-1);
				currentPrediction->posY.push_back(-1);
				currentPrediction->posZ.push_back(-1);
				// set prediction index to next, keep for first computation
				currentPrediction->time = 1;
				currentPrediction->keep = true;
				// add to list
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
			for (plane *craft : this->getFlyingPlanesInfo()) {

				if (craft->id == id) {

					// if found, update with current info
					craft->posX = pos[0];
					craft->posY = pos[1];
					craft->posZ = pos[2];
					craft->velX = vel[0];
					craft->velY = vel[1];
					craft->velZ = vel[2];
					// if found, keep, if not will be removed
					craft->keep = true;
					// if already in list, do not add new
					inList = true;

					for (trajectoryPrediction *prediction : this->getTrajectoryPredictions()) {
						if (prediction->id == craft->id) {

							// if end of prediction reached, break
							bool predictionX = prediction->time >= (int)prediction->posX.size();
							bool predictionY = prediction->time >= (int)prediction->posY.size();
							bool predictionZ = prediction->time >= (int)prediction->posZ.size();

							if (predictionX || predictionY || predictionZ) {
								break;
							}

							// check posx, posy and poz if same
							if (prediction->posX.at(prediction->time) == craft->posX && prediction->posY.at(prediction->time) == craft->posY && prediction->posZ.at(prediction->time) == craft->posZ) {
								auto it = prediction->posX.end();
								it -= 2;
								int lastX = *it + (this->getCurrentPeriod() / 1000000) * craft->velX;
								it = prediction->posY.end();
								it -= 2;
								int lastY = *it + (this->getCurrentPeriod() / 1000000) * craft->velY;
								it = prediction->posZ.end();
								it -= 2;
								int lastZ = *it + (this->getCurrentPeriod() / 1000000) * craft->velZ;

								// check if still within limits
								bool outOfBounds = false;
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
									int currX = craft->posX + i * (currentPeriod / 1000000) * craft->velX;
									int currY = craft->posY + i * (currentPeriod / 1000000) * craft->velY;
									int currZ = craft->posZ + i * (currentPeriod / 1000000) * craft->velZ;

									prediction->posX.push_back(currX);
									prediction->posY.push_back(currY);
									prediction->posZ.push_back(currZ);

									bool outOfBounds = false;
									bool outOfXBound = currX > MAX_X_AIRSPACE || currX < MIN_X_AIRSPACE;
									bool outOfYBound = currY > MAX_Y_AIRSPACE || currY < MIN_Y_AIRSPACE;
									bool outOfZBound = currZ > MAX_Z_AIRSPACE || currZ < MIN_Z_AIRSPACE;

									if (outOfXBound || outOfYBound || outOfZBound) {
										outOfBounds = true;
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
				plane *currentAircraft = new plane();
				currentAircraft->id = id;
				currentAircraft->arrivalTime = arrTime;
				currentAircraft->posX = pos[0];
				currentAircraft->posY = pos[1];
				currentAircraft->posZ = pos[2];
				currentAircraft->velX = vel[0];
				currentAircraft->velY = vel[1];
				currentAircraft->velZ = vel[2];
				currentAircraft->keep = true; // keep for first computation
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

					bool outOfBounds = false;
					bool outOfXBound = currX > MAX_X_AIRSPACE || currX < MIN_X_AIRSPACE;
					bool outOfYBound = currY > MAX_Y_AIRSPACE || currY < MIN_Y_AIRSPACE;
					bool outOfZBound = currZ > MAX_Z_AIRSPACE || currZ < MIN_Z_AIRSPACE;

					if (outOfXBound || outOfYBound || outOfZBound) {
						outOfBounds = true;
						break;
					}
				}
				// set termination character
				currentPrediction->posX.push_back(-1);
				currentPrediction->posY.push_back(-1);
				currentPrediction->posZ.push_back(-1);
				// set prediction index to next, keep for first computation
				currentPrediction->time = 1;
				currentPrediction->keep = true;
				// add to list
				this->getTrajectoryPredictions().push_back(currentPrediction);
			}

			// reset buffer and index for next plane
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

void ComputerSystem::cleanPredictions() {
	int j = 0;
	auto itpred = this->getTrajectoryPredictions().begin();
	while (itpred != this->getTrajectoryPredictions().end()) {
		bool temp = (*itpred)->keep;

		// check if plane was terminated
		if (!temp) {
			delete this->getTrajectoryPredictions().at(j);
			itpred = this->getTrajectoryPredictions().erase(itpred);
		}
		else {
			for (int i = (*itpred)->time - 1; i < (*itpred)->time + (180 / (this->getCurrentPeriod() / 1000000)); i++) {
				int currX = (*itpred)->posX.at(i);
				int currY = (*itpred)->posY.at(i);
				int currZ = (*itpred)->posZ.at(i);

				if (currX == -1 || currY == -1 || currZ == -1) {
					break;
				}
			}

			(*itpred)->keep = false; // if found next time, this will become true again
			j++;
			++itpred;
		}
	}
}

void ComputerSystem::computeViolations(std::ofstream *out) {
	auto itIndex = this->getTrajectoryPredictions().begin();
	while (itIndex != this->getTrajectoryPredictions().end()) {
		auto itNext = itIndex;
		++itNext;
		while (itNext != this->getTrajectoryPredictions().end()) {
			// compare predictions, starting at current
			int j = (*itNext)->time - 1;
			for (int i = (*itIndex)->time - 1;
					i < (*itIndex)->time - 1 + (180 / (this->getCurrentPeriod() / 1000000)); i++) {
				int currX = (*itIndex)->posX.at(i);
				int currY = (*itIndex)->posY.at(i);
				int currZ = (*itIndex)->posZ.at(i);
				int compX = (*itNext)->posX.at(j);
				int compY = (*itNext)->posY.at(j);
				int compZ = (*itNext)->posZ.at(j);

				if (currX == -1 || currY == -1 || currZ == -1) {
					break;
				}
				if (compX == -1 || compY == -1 || compZ == -1) {
					break;
				}

				if ((abs(currX - compX) <= 3000 && abs(currY - compY) <= 3000) &&
						abs(currZ - compZ) <= 1000) {
					std::cout << "airspace violation detected between planes "
							<< (*itIndex)->id << " and " << (*itNext)->id
							<< " at time current + " << i * (currentPeriod) / 1000000
							<< "\n";
					bool currComm = false;
					bool compComm = false;
					// find comm
					for (void *comm : this->getCommPtrs()) {
						// comm shm id
						int commId = atoi((char *)comm);

						if (commId == (*itIndex)->id) {
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
							std::string command = "z,200;";
							sprintf((char *)comm + k, "%s", command.c_str());
							std::streambuf *coutbuf = std::cout.rdbuf(); // save old buf
							std::cout.rdbuf(out->rdbuf()); // redirect std::cout to command
							std::cout << "Command: Plane " << (*itIndex)->id << " increases altitude by 200 feet" << std::endl;
							std::cout.rdbuf(coutbuf); // reset to standard output again
							currComm = true;
						}
						if (commId == (*itNext)->id) {
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
							compComm = true;
						}
						if (currComm && compComm) {
							break;
						}
					}
					// set associate craft info request to true, display height
					currComm = false;
					compComm = false;
					for (plane *craft : this->getFlyingPlanesInfo()) {
						if (craft->id == (*itIndex)->id) {
							craft->moreInfo = true;
							craft->commandCounter = NUM_PRINT;
							currComm = true;
						}
						if (craft->id == (*itNext)->id) {
							craft->moreInfo = true;
							craft->commandCounter = NUM_PRINT;
							compComm = true;
						}
						if (currComm && compComm) {
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

	std::string displayBuffer = "";
	std::string currentPlaneBuffer = "";
	// print what was found, remove what is no longer in the airspace
	int i = 0;
	auto it = this->getFlyingPlanesInfo().begin();
	while (it != this->getFlyingPlanesInfo().end()) {
		bool temp = (*it)->keep; // check if plane was terminated

		if (!temp) {
			delete this->getFlyingPlanesInfo().at(i);
			it = this->getFlyingPlanesInfo().erase(it);
			this->setPlaneCount(this->getPlaneCount()-1);
		} else {
			// add plane to buffer for display

			// id,posx,posy,posz,info
			// ex: 1,15000,20000,5000,0
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
	if (this->getTrajectoryPredictions().size() <= 5) {
		// set period low
		newPeriod = 5000000;
	}
	if (this->getTrajectoryPredictions().size() > 5 && this->getTrajectoryPredictions().size() <= 20) {
		// set period medium
		newPeriod = 3000000;
	}
	if (this->getTrajectoryPredictions().size() > 20 && this->getTrajectoryPredictions().size() <= 50) {
		// set period high
		newPeriod = 2000000;
	}
	if (this->getTrajectoryPredictions().size() > 50 && this->getTrajectoryPredictions().size() <= 200) {
		// set period overdrive
		newPeriod = 1000000;
	}



	if (this->getCurrentPeriod() != newPeriod) {
		this->setCurrentPeriod(newPeriod);
		std::string CSPeriod = std::to_string(this->getCurrentPeriod());
		sprintf((char *)periodPtr, CSPeriod.c_str());
		timer->setTimer(this->getCurrentPeriod(), this->getCurrentPeriod());
	}
}
