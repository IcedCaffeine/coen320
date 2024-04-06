#include "Computer_System.h"

pthread_t createComputerSysThread(void* collisionCallback) {

    int receivedComm;
	pthread_t thread;
	pthread_attr_t attr;

	receivedComm = pthread_attr_init(&attr);

	receivedComm = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	receivedComm = pthread_create(&thread, &attr, Computer_System_Main, collisionCallback);

	return thread;
}


void * Computer_System_Main(void *arg) {

    std::cout << "the thread for the computer system is started..." << std::endl;

    std::string nameChannel= "ComputerChannel";

    name_attach_t *pathChannel;
	MsgToComputerSys message;
	int receivedID;

	pathChannel = name_attach(NULL, nameChannel.c_str(), 0);

	if(pathChannel == NULL) {
		std::cout << "Error in the channel name" << std::endl;
		return NULL;
	}

    time_t timerClock;
    time(&timerClock);

	while (true) {
		receivedID = MsgReceive(pathChannel->chid, &message, sizeof(message), NULL);
		std::cout << "Message received!" << std::endl;

		/*
		bool messageFormat = properMsgFormat(receivedID, message);

        if (messageFormat != true){
            continue;
        }

		*/
        switch(message.type){
            case AirplaneRadarUpdate:
            	std::cout << "message from airplane received" << std::endl;
            	radarUpdate();
                break;

            case ClockTimerUpdate:
                collisionCheck();
                break;
            case AddAirplane:
            	addingAirplane();
                break;
            case RemoveAirplane:
            	removingAirplane();
                break;

            default:
            	std::cout << "An unknown message has been received..." << std::endl;
                break;
        }
	}

	name_detach(pathChannel, 0);
	return NULL;
}

void Computer_System::changeSpeed(Aircraft aircraft, double s) {
	aircraft.setVelX(static_cast<int>(s));
	aircraft.setVelY(static_cast<int>(s));
	aircraft.setVelZ(static_cast<int>(s));
}

//definition of function to change flight level of aircraft
void Computer_System::changeAltitude(Aircraft aircraft, int a) {
	aircraft.setPosZ(a);
}

//definition of function to change position of aircraft
void Computer_System::changePosition(Aircraft aircraft, float orient) {
	aircraft.setPosX(static_cast<int>(orient));
	aircraft.setPosY(static_cast<int>(orient));
	aircraft.setPosZ(static_cast<int>(orient));
}


void addingAirplane(){

}

void removingAirplane(){

}

void radarUpdate(){

}

void collisionCallback(std::string message){
	std::cout << message << std::endl;
}


typedef std::vector<Aircraft> checkAirPlaneCollision;

void collisionCheck(){

	// Notify About Request
    std::cout << "There is a request to update the clock timer.." << std::endl;

    // Initialize
	checkAirPlaneCollision collidingAirplane;

    for(unsigned int i=0;i<collidingAirplane.size();i++){
    	for(unsigned int j=i+1;i<collidingAirplane.size();i++){
    		if(collidingAirplane[i].IsCollidingWith(collidingAirplane[j])){
    			std::cout << "there is a collision..." << std::endl;
    			std::cout << "We are going to be displaying th plane ID1 and ID2, time of collision, position in the 3d matrix, and collision time" << std::endl;

				//callback that will send to the Data Display an alert that with the aircraft information that needs to be displayed in case of a collision
    			std::string collisionWarning = "Airplane with ID ";
    			collisionWarning +=  std::to_string(collidingAirplane[i].getAircraftId());
    			collisionWarning += " colliding with airplane with ID ";
    			collisionWarning += std::to_string(collidingAirplane[j].getAircraftId());
    			collisionWarning += "\n";

    			collisionCallback(collisionWarning);
    		}
		}
    }
}
