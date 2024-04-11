#include "AirTrafficControl.h"

AirTrafficControl::AirTrafficControl() {
  this->initialize();
  this->start();
}

AirTrafficControl::~AirTrafficControl() {
  // release all shared memory pointers
  shm_unlink("airspace");
  shm_unlink("period");
  shm_unlink("display");
  shm_unlink("waiting_planes");
  shm_unlink("flying_planes");

  for (Aircraft *aircraft : this->getPlanes()) {
    delete aircraft;
  }
  delete dataDisplay;
  delete computerSystem;
  delete primaryRadar;
  delete secondaryRadar;
}

ComputerSystem* AirTrafficControl::getComputerSystem() const {
	return computerSystem;
}

void AirTrafficControl::setComputerSystem(ComputerSystem *computerSystem) {
	this->computerSystem = computerSystem;
}

DataDisplay* AirTrafficControl::getDataDisplay() const {
	return dataDisplay;
}

void AirTrafficControl::setDataDisplay(DataDisplay *dataDisplay) {
	this->dataDisplay = dataDisplay;
}

std::vector<Aircraft*> AirTrafficControl::getPlanes() const {
	return planes;
}

void AirTrafficControl::setPlanes(std::vector<Aircraft*> planes) {
	this->planes = planes;
}

PrimaryRadar* AirTrafficControl::getPrimaryRadar() const {
	return primaryRadar;
}

void AirTrafficControl::setPrimaryRadar(PrimaryRadar *primaryRadar) {
	this->primaryRadar = primaryRadar;
}

SecondaryRadar* AirTrafficControl::getSecondaryRadar() const {
	return secondaryRadar;
}

void AirTrafficControl::setSecondaryRadar(SecondaryRadar *secondaryRadar) {
	this->secondaryRadar = secondaryRadar;
}


int AirTrafficControl::initialize() {
  // Read from Input File
	this->readInput();

  // Setup Waiting Planes
  shm_waitingPlanes = shm_open("waiting_planes", O_CREAT | O_RDWR, 0666);
  if (shm_waitingPlanes == -1) {
    perror("in shm_open() ATC: waiting planes");
    exit(1);
  }
  ftruncate(shm_waitingPlanes, SIZE_SHM_PSR);

  waitingPtr = mmap(0, SIZE_SHM_PSR, PROT_READ | PROT_WRITE, MAP_SHARED, shm_waitingPlanes, 0);
  if (waitingPtr == MAP_FAILED) {
    printf("map failed waiting planes\n");
    return -1;
  }

  int i = 0;
  for (Aircraft *plane : this->getPlanes()) {
    sprintf((char *)waitingPtr + i, "%s,", plane->getFileName().c_str());
    i += (strlen(plane->getFileName().c_str()) + 1);
  }
  sprintf((char *)waitingPtr + i - 1, ";"); // file termination character

  // Setup Flying Planes
  shm_flyingPlanes = shm_open("flying_planes", O_CREAT | O_RDWR, 0666);
  if (shm_flyingPlanes == -1) {
    perror("in shm_open() ATC: flying planes");
    exit(1);
  }
  ftruncate(shm_flyingPlanes, SIZE_SHM_SSR);

  flyingPlanesPtr = mmap(0, SIZE_SHM_SSR, PROT_READ | PROT_WRITE, MAP_SHARED,shm_flyingPlanes, 0);
  if (flyingPlanesPtr == MAP_FAILED) {
    printf("map failed flying planes\n");
    return -1;
  }
  sprintf((char *)flyingPlanesPtr, ";");
  
  // Setup Airspace
  shm_airspace = shm_open("airspace", O_CREAT | O_RDWR, 0666);
  if (shm_airspace == -1) {
    perror("in shm_open() ATC: airspace");
    exit(1);
  }
  ftruncate(shm_airspace, SIZE_SHM_AIRSPACE);

  airspacePtr = mmap(0, SIZE_SHM_AIRSPACE, PROT_READ | PROT_WRITE, MAP_SHARED,shm_airspace, 0);
  if (airspacePtr == MAP_FAILED) {
    printf("map failed airspace\n");
    return -1;
  }
  sprintf((char *)airspacePtr, ";");

  // Setup Period
  shm_period = shm_open("period", O_CREAT | O_RDWR, 0666);
  if (shm_period == -1) {
    perror("in shm_open() ATC: period");
    exit(1);
  }
  ftruncate(shm_period, SIZE_SHM_PERIOD);

  periodPtr = mmap(0, SIZE_SHM_PERIOD, PROT_READ | PROT_WRITE, MAP_SHARED,shm_period, 0);
  if (periodPtr == MAP_FAILED) {
    printf("map failed period\n");
    return -1;
  }
  int timePeriod = CS_PERIOD;
  std::string CSPeriod = std::to_string(timePeriod);
  sprintf((char *)periodPtr, CSPeriod.c_str());

  // Setup Display
  shm_display = shm_open("display", O_CREAT | O_RDWR, 0666);
  ftruncate(shm_display, SIZE_SHM_DISPLAY);

  void *displayPtr = mmap(0, SIZE_SHM_DISPLAY, PROT_READ | PROT_WRITE,MAP_SHARED, shm_display, 0);
  if (displayPtr == MAP_FAILED) {
    printf("Display ptr failed mapping\n");
    return -1;
  }
  sprintf((char *)displayPtr, ";");

  //Create Object thread
  this->setPrimaryRadar(new PrimaryRadar(this->getPlanes().size()));
  this->setSecondaryRadar(new SecondaryRadar(this->getPlanes().size()));
  this->setDataDisplay(new DataDisplay());
  this->setComputerSystem(new ComputerSystem(this->getPlanes().size()));

  return 0; // set to error code if any
}

int AirTrafficControl::start() {

	// Start Object threads
	this->getPrimaryRadar()->start();
	this->getSecondaryRadar()->start();
	this->getDataDisplay()->start();
	this->getComputerSystem()->start();

	for (Aircraft *plane : this->getPlanes()) {
		plane->start();
	}

	// End Object Threads
	for (Aircraft *plane : this->getPlanes()) {
		plane->stop();
	}

	this->getPrimaryRadar()->stop();
	this->getSecondaryRadar()->stop();
	this->getDataDisplay()->stop();
	this->getComputerSystem()->stop();

  return 0; // set to error code if any
}


int AirTrafficControl::readInput() {
  std::string fileName = "./inputFile.txt";
  std::ifstream inputFileStream;

  inputFileStream.open(fileName);
  if (!inputFileStream) {
    std::cout << "Can't find file inputFile.txt" << std::endl;
    return 1;
  }

  // Extract Data from File
  int id, arrivalTime;
  int arrivalX, arrivalY, arrivalZ;
  int arrivalSpeedX,arrivalSpeedY, arrivalSpeedZ;
  std::string separator = " ";

  while (inputFileStream >> id >> arrivalTime >> arrivalX >> arrivalY >> arrivalZ >> arrivalSpeedX >> arrivalSpeedY >> arrivalSpeedZ)
	{
		int position[3] = {
			arrivalX,
			arrivalY,
			arrivalZ
		};
		int velocity[3] = {
			arrivalSpeedX,
			arrivalSpeedY,
			arrivalSpeedZ
		};

		// Add new Aircraft to Plane vector
		Aircraft *plane = new Aircraft(id, arrivalTime, position, velocity);
		planes.push_back(plane);
	}

	return 0;
}

