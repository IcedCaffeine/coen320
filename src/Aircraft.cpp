#include "Aircraft.h"
#include <cmath> // For basic mathematical operations

// Constructor to initialize the Aircraft object with given parameters
Aircraft::Aircraft(){

}

Aircraft::Aircraft(int aircraftId, int posX, int posY, int posZ, int velX, int velY, int velZ, int arrivalTime): arrivalTime(arrivalTime){
	this->aircraftId = aircraftId;

	this->posX = posX;
	this->posY = posY;
	this->posZ = posZ;

	this->velX = velX;
	this->velY = velY;
	this->velZ = velZ;
}

// Destructor
Aircraft::~Aircraft() {
    // Any cleanup if necessary
}

int Aircraft::getAircraftId() const {
	return aircraftId;
}

void Aircraft::setAircraftId(int aircraftId) {
	this->aircraftId = aircraftId;
}

int Aircraft::getArrivalTime() const {
	return arrivalTime;
}

void Aircraft::setArrivalTime(int arrivalTime) {
	this->arrivalTime = arrivalTime;
}

int Aircraft::getPosX() const {
	return posX;
}

void Aircraft::setPosX(int posX) {
	this->posX = posX;
}

int Aircraft::getPosY() const {
	return posY;
}

void Aircraft::setPosY(int posY) {
	this->posY = posY;
}

int Aircraft::getPosZ() const {
	return posZ;
}

void Aircraft::setPosZ(int posZ) {
	this->posZ = posZ;
}

int Aircraft::getVelX() const {
	return velX;
}

void Aircraft::setVelX(int velX) {
	this->velX = velX;
}

int Aircraft::getVelY() const {
	return velY;
}

void Aircraft::setVelY(int velY) {
	this->velY = velY;
}

int Aircraft::getVelZ() const {
	return velZ;
}

void Aircraft::setVelZ(int velZ) {
	this->velZ = velZ;
}

void Aircraft::updatePosition() {
    // Implementation here
}

bool Aircraft::outsideAirSpace(){
	bool exceedX = this->getPosX() > MAX_AIRSPACE_X_REGION;
	bool exceedY = this->getPosY() > MAX_AIRSPACE_Y_REGION;
	bool exceedZ = this->getPosZ() > MAX_AIRSPACE_Z_REGION;

	return (exceedX || exceedY || exceedZ)? true : false;
}

std::vector<Aircraft> readAircraftFromFile(std::string fileName){
	std::vector<Aircraft> planes;
	std::ifstream inputFile(fileName);
	std::string line;


	while (std::getline(inputFile, line)) {
		std::stringstream ss(line);
		int id, arrivalTime;
		double xPos, yPos, zPos, xSpeed, ySpeed, zSpeed;
		char comma;
		if (ss >> arrivalTime >> comma >> id >> comma >> xPos >> comma >> yPos >> comma >> zPos >> comma >> xSpeed >> comma >> ySpeed >> comma >> zSpeed) {
			// Create Array for Positions and Velocity
			int xPosArr[] = { static_cast<int>(xPos) };
			int yPosArr[] = { static_cast<int>(yPos) };
			int zPosArr[] = { static_cast<int>(zPos) };
			int xVelArr[] = { static_cast<int>(xSpeed) };
			int yVelArr[] = { static_cast<int>(ySpeed) };
			int zVelArr[] = { static_cast<int>(zSpeed) };
			int *pos[3] = {xPosArr, yPosArr, zPosArr};

			// Store them in pos and vel
			int posX = *pos[0];
			int posY = *pos[1];
			int posZ = *pos[2];
			int *vel[3] = {xVelArr, yVelArr, zVelArr};
			int velX = *vel[0];
			int velY = *vel[1];
			int velZ = *vel[2];

			// Add to list
			planes.emplace_back(id, posX, posY, posZ, velX, velY, velZ, arrivalTime);
		}
	}
	return planes;
}

void* AircraftClientThread(void *){
	Plane_Data_t airspaceMsg;

	//Server connection ID
	int radarServerId;
	if ((radarServerId = name_open(ATTACH_POINT, 0)) == -1) {
			perror("Error occurred while attaching the channel");
	}

	// Setup Parameters
	std::vector<Aircraft> planes = readAircraftFromFile("./inputFile.txt");
	uint32_t period_sec=1;
	double period=period_sec;
	Timer timer(period_sec,period_sec);
	int count=0;
	double t = 0;
	airspaceMsg.hdr.type=0x00;
	airspaceMsg.hdr.subtype=0x01;

	while(t<30.0){
		t=count*period;
		for (unsigned int i=0; i<planes.size(); i++) {
			Aircraft plane = planes[i];
			if (plane.getArrivalTime() > t){
				// Get Data
				airspaceMsg.id = plane.getAircraftId();
				airspaceMsg.velocity[0] = plane.getVelX(); airspaceMsg.position[0] = plane.getPosX();
				airspaceMsg.velocity[1] = plane.getVelY(); airspaceMsg.position[1] = plane.getPosY();
				airspaceMsg.velocity[2] = plane.getVelZ(); airspaceMsg.position[2] = plane.getPosZ();

				if(plane.outsideAirSpace()){
					airspaceMsg.outsideAirspace = true;
				}
				else {
					airspaceMsg.outsideAirspace = false;
				}
				airspaceMsg.ArrivedYet = false;
				if (MsgSend(radarServerId, &airspaceMsg, sizeof(airspaceMsg), NULL,0) == -1){
					perror("Error sending speed message");
					break;
				}

			}
			else{
				// Get Data
				airspaceMsg.id = plane.getAircraftId();
				airspaceMsg.velocity[0] = plane.getVelX(); airspaceMsg.position[0] = plane.getPosX();
				airspaceMsg.velocity[1] = plane.getVelY(); airspaceMsg.position[1] = plane.getPosY();
				airspaceMsg.velocity[2] = plane.getVelZ(); airspaceMsg.position[2] = plane.getPosZ();

				if(plane.outsideAirSpace()){
					airspaceMsg.outsideAirspace = true;
				}
				else {
					airspaceMsg.outsideAirspace = false;
				}
				if (MsgSend(radarServerId, &airspaceMsg, sizeof(airspaceMsg), NULL,0) == -1){
					perror("Error sending speed message");
					break;
				}
			}
		}
		std::cout << "Time is " << t << "\n";
		timer.waitTimer();
		count++;

	}

	//Finally, close connection with server when done
	//int name_close(int coid);
	//coid is the connection ID returned by name_open() function
	name_close(radarServerId);
	return EXIT_SUCCESS;
}

void* TimerReceiverThread(void *){
	name_attach_t *name_attach_t;

	//Instantiate message based on structure
	Time_Data_t msg;

	//Create receive ID
	int receiveId;

	//Path is the name of the channel
	if ((name_attach_t = name_attach(NULL, ATTACH_POINT, 0)) == NULL){
			perror("Error occurred during creation of channel");
	}

	while(true){
		receiveId = MsgReceive(name_attach_t->chid, &msg, sizeof(msg), NULL);
		if(receiveId == -1) {
			perror("Error receiving message");
			break;
		}
		else if(receiveId == 0){
			if(msg.hdr.code == _PULSE_CODE_DISCONNECT){
				ConnectDetach(msg.hdr.scoid);
			}
			continue;
		}

		//name_open() sends a connect message, must EOK this */
		if (msg.hdr.type == _IO_CONNECT ) {
			MsgReply( receiveId, EOK, NULL, 0 );
			continue;
		}

		/* Some other QNX IO message was received; reject it */
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX ) {
			MsgError( receiveId, ENOSYS );
			continue;
		}
		if (msg.hdr.type == 0x00){
			//Motion message received
			//0x01 is subtype for velocity
			if (msg.hdr.subtype == 0x01){
				std::cout << "Time is : " << msg.time << "\n";
			}
			else{
				MsgError(receiveId, ENOSYS);
				continue;
			}
		}

		MsgReply(receiveId, EOK, 0, 0);
	}

	name_detach(name_attach_t, 0);
	return NULL;

}

bool Aircraft::IsCollidingWith(Aircraft& otherAircraft){
	bool xCollide = this->getPosX() == otherAircraft.getPosX();
	bool yCollide = this->getPosY() == otherAircraft.getPosY();
	bool zCollide = this->getPosZ() == otherAircraft.getPosZ();
	return (xCollide && yCollide && zCollide)? true : false;
}
