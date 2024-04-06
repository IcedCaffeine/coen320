#include "Computer_System.h"

#include "DataDisplay.h"
#include "Aircraft.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include "Structure.h"
#include <pthread.h>

#include "Communication_System.h"

using namespace std;

pthread_t createComputerSysThread(void* collisionCallback) {

    int receivedComm;
	pthread_t thread;
	pthread_attr_t attr;

	receivedComm = pthread_attr_init(&attr);

	receivedComm = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	receivedComm = pthread_create(&thread, &attr, Computer_System_Main, NULL, void* collisionCallback);

	return thread;
}


void * Computer_System_Main(void *arg) {

    cout << "the thread for the computer system is started..." << endl;

	string nameChannel= "ComputerChannel";

    name_attach_t *pathChannel;
	MsgToComputerSys message;
	int receivedID;

	pathChannel = name_attach(NULL, nameChannel.c_str(), 0);

	if(pathChannel == NULL) {
		cout << "Error in the channel name" << endl;
		return NULL;
	}

    time_t timerClock;
    time(&timerClock);

	while (true) {
		receivedID = MsgReceive(pathChannel->chid, &message, sizeof(message), NULL);

        cout << "Message received!" << endl;

		bool messageFormat = properMsgFormat(receivedID, message);

        if (messageFormat != true){
            continue;
        }

        switch(message.type){
            case AirplaneRadarUpdate:
                cout << "message from airplane received" << endl;
                radarUpdate();
                break;

            case ClockTimerUpdate:
                collisionCheck();
                break;
            case addAirplane:
                addingAirplane();
                break;
            case removeAirplane:
                removingAirplane();
                break;

            default:
                cout << "An unknown message has been received..." << endl;
                break;
        }


	name_detach(pathChannel, 0);

	return NULL;
}

void addingAirplane(){

}

void removingAirplane(){

}

void radarUpdate(){

}

vector airplanes_checking_collision;

void collisionCheck(){

    cout << "There is a request to update the clock timer.." << endl;

	double collisionTime;

    for (int i = 0; i < airplanes_checking_collision.size(); ++i) {

        for (int j = i + 1; j < airplanes_checking_collision.size(); ++j) {

            if (airplanes_checking_collision[i].IsCollidingWith(airplanes_checking_collision[j])) {

                cout << "there is a collision..." << endl;
                cout << "We are going to be displaying th plane ID1 and ID2, time of collision, position in the 3d matrix, and collision time" << endl;

                string collisionWarning = "Airplane with ID " << airplanes_checking_collision[i].id << " colliding with airplane with ID " << airplanes_checking_collision[j].id << endl;
                collisionCallback(collisionWarning);
            }
        }
    }
}

void Computer_System::changeSpeed(int id, double s) {
	Aircraft* plane = nullptr;

	for(int i = 0; i < plane.size(); i++) {
		if(plane[i].getAircraftId() == id) {
			plane = &plane[i];
		}
	}

	if(plane = nullptr) {
		cout << "This aircraft is not found" << endl;
	}

	plane -> setVelX(s);
}

void Computer_System::changeAltitude(int id, int a) {
	Aircraft* plane = nullptr;
	for(int i = 0; i < plane.size(); i++) {
		if(plane[i].getAircraftId() == id) {
			plane = &plane[i];
		}
	}

	if(plane == nullptr) {
		cout << "This aircraft is not found" << endl;
	}

	plane -> setPosZ(a);
}

void Computer_System::changePosition(int id, float x) {
	Aircraft* plane = nullptr;
	for(int i = 0; i < plane.size(); i++) {
		if(plane[i].getAircraftId() == id) {
			plane = &plane[i];
		}
	}

	if(plane == nullptr) {
		cout << "This aircraft is not found" << endl;
	}

	plane -> setPosX(x);
}

