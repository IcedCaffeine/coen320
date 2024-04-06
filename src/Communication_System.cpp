#include "Communication_System.h"

#include "Aircraft.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <pthread.h>
#include <vector>
#include "Computer_System.h"


using namespace std;

typedef struct _pulse msg_header_t;

void * communicationMain() {
	cout << "The communication has begun..." << endl;

	string nameChannel = "CommunicationChannel";

	cout << "Creating messaging system with" << nameChannel << endl;

	name_attach_t *pathChannel;

	string message;

	int receivedID;

	pathChannel = name_attach(NULL, nameChannel.c_str(), 0);

	if(pathChannel == NULL) {
		cout << "Error in the channel name" << endl;
		return NULL;
	}

	while(true) {
		receivedID = MsgReceive(pathChannel->chid, &message, sizeof(message), NULL);
		cout << "Message received!" << endl;
	}
	return NULL;
}

void disconnectFromChannel(int coid) {
    int status = ConnectDetach(coid);
    if (status == -1) {
        cout<< "Error detaching connection" << endl;
    }
}


pthread_t createCommunication() {
	int receivedComm;
	pthread_t thread;
	pthread_attr_t attribute;

	receivedComm = pthread_attr_init(&attribute);

	receivedComm = pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_JOINABLE);

	return thread;

}

void sendMessage(int chid, const void *msg, int size) {
    int receivedID;
    struct _msg_info info;

    int status = MsgSend(chid, msg, size, NULL, 0);

    if (status == -1) {
        std::cout << "Error sending message" << endl;
        return;
    }

    receivedID = MsgReceive(chid, NULL, 0, &info);

    if (receivedID == -1) {
        std::cout << "Error receiving reply" << endl;
        return;
    }
}

void Communication_System::sendMessage(){
}
