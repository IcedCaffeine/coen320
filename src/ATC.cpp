#include "ATC.h"

ATC::ATC() {
  initialize();
  start();
}

ATC::~ATC() {
  // release all shared memory pointers
  shm_unlink("airspace");
  shm_unlink("waiting_planes");
  shm_unlink("flying_planes");
  shm_unlink("period");
  shm_unlink("display");

  for (Aircraft *plane : this->getPlanes()) {
    delete plane;
  }
  delete primaryRadar;
  delete secondaryRadar;
  delete dataDisplay;
  delete computerSystem;
}

ComputerSystem* ATC::getComputerSystem() const {
	return computerSystem;
}

void ATC::setComputerSystem(ComputerSystem *computerSystem) {
	this->computerSystem = computerSystem;
}

DataDisplay* ATC::getDataDisplay() const {
	return dataDisplay;
}

void ATC::setDataDisplay(DataDisplay *dataDisplay) {
	this->dataDisplay = dataDisplay;
}

std::vector<Aircraft*> ATC::getPlanes() const {
	return planes;
}

void ATC::setPlanes(std::vector<Aircraft*> planes) {
	this->planes = planes;
}

PrimaryRadar* ATC::getPrimaryRadar() const {
	return primaryRadar;
}

void ATC::setPrimaryRadar(PrimaryRadar *primaryRadar) {
	this->primaryRadar = primaryRadar;
}

SecondaryRadar* ATC::getSecondaryRadar() const {
	return secondaryRadar;
}

void ATC::setSecondaryRadar(SecondaryRadar *secondaryRadar) {
	this->secondaryRadar = secondaryRadar;
}

// initialize thread and shm members
int ATC::initialize() {
  // read input from file
  readInput();

  // initialize shm for waiting planes (contains all planes)
  shm_waitingPlanes = shm_open("waiting_planes", O_CREAT | O_RDWR, 0666);
  if (shm_waitingPlanes == -1) {
    perror("in shm_open() ATC: waiting planes");
    exit(1);
  }

  // set shm size
  ftruncate(shm_waitingPlanes, SIZE_SHM_PSR);

  // map shm
  waitingPtr = mmap(0, SIZE_SHM_PSR, PROT_READ | PROT_WRITE, MAP_SHARED, shm_waitingPlanes, 0);
  if (waitingPtr == MAP_FAILED) {
    printf("map failed waiting planes\n");
    return -1;
  }

  // save file descriptors to shm
  int i = 0;
  for (Aircraft *plane : this->getPlanes()) {
    sprintf((char *)waitingPtr + i, "%s,", plane->getFileName().c_str());
    i += (strlen(plane->getFileName().c_str()) + 1);
  }
  sprintf((char *)waitingPtr + i - 1, ";"); // file termination character

  //  initialize shm for flying planes (contains no planes)
  shm_flyingPlanes = shm_open("flying_planes", O_CREAT | O_RDWR, 0666);
  if (shm_flyingPlanes == -1) {
    perror("in shm_open() ATC: flying planes");
    exit(1);
  }

  // set shm size
  ftruncate(shm_flyingPlanes, SIZE_SHM_SSR);

  // map shm
  flyingPlanesPtr = mmap(0, SIZE_SHM_SSR, PROT_READ | PROT_WRITE, MAP_SHARED,shm_flyingPlanes, 0);
  if (flyingPlanesPtr == MAP_FAILED) {
    printf("map failed flying planes\n");
    return -1;
  }
  sprintf((char *)flyingPlanesPtr, ";");

  shm_airspace = shm_open("airspace", O_CREAT | O_RDWR, 0666);
  if (shm_airspace == -1) {
    perror("in shm_open() ATC: airspace");
    exit(1);
  }

  // set shm size
  ftruncate(shm_airspace, SIZE_SHM_AIRSPACE);

  // map shm
  airspacePtr = mmap(0, SIZE_SHM_AIRSPACE, PROT_READ | PROT_WRITE, MAP_SHARED,
                     shm_airspace, 0);
  if (airspacePtr == MAP_FAILED) {
    printf("map failed airspace\n");
    return -1;
  }
  sprintf((char *)airspacePtr, ";");

  // initialize shm for period update
  shm_period = shm_open("period", O_CREAT | O_RDWR, 0666);
  if (shm_period == -1) {
    perror("in shm_open() ATC: period");
    exit(1);
  }

  // set shm size
  ftruncate(shm_period, SIZE_SHM_PERIOD);

  // map shm
  periodPtr = mmap(0, SIZE_SHM_PERIOD, PROT_READ | PROT_WRITE, MAP_SHARED,
                   shm_period, 0);
  if (periodPtr == MAP_FAILED) {
    printf("map failed period\n");
    return -1;
  }
  int per = CS_PERIOD;
  std::string CSPeriod = std::to_string(per);
  sprintf((char *)periodPtr, CSPeriod.c_str());

  // initialize shm for display
  shm_display = shm_open("display", O_CREAT | O_RDWR, 0666);

  // set shm size
  ftruncate(shm_display, SIZE_SHM_DISPLAY);

  // map the memory

  void *displayPtr = mmap(0, SIZE_SHM_DISPLAY, PROT_READ | PROT_WRITE,
                          MAP_SHARED, shm_display, 0);
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

int ATC::start() {

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

// read input file and
int ATC::readInput() {
  std::string fileName = "./input.txt";
  std::ifstream inputFileStream;

  inputFileStream.open(fileName);

  if (!inputFileStream) {
    std::cout << "Can't find file input.txt" << std::endl;
    return 1;
  }


  int ID, arrivalTime, arrivalX, arrivalY, arrivalZ, arrivalSpeedX,arrivalSpeedY, arrivalSpeedZ;
  std::string separator = " ";
  while (inputFileStream >> ID >> arrivalTime >> arrivalX >> arrivalY >> arrivalZ >> arrivalSpeedX >> arrivalSpeedY >> arrivalSpeedZ)
	{
		int position[3] = {arrivalX, arrivalY, arrivalZ};
		int velocity[3] = {arrivalSpeedX, arrivalSpeedY, arrivalSpeedZ};

		// create plane objects and add pointer to each plane to a vector
		Aircraft *plane = new Aircraft(ID, arrivalTime, position, velocity);
		planes.push_back(plane);
	}

	return 0;
}

