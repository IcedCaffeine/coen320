#include "Communication_System.h"

int main() {
	Communication_System communicationSystem;
	pthread_t CommSystemThread = communicationSystem.createCommunication();
	pthread_join(CommSystemThread, NULL);

	return 0;
}
