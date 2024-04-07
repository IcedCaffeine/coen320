#include "Communication_System.h"

int main() {
	pthread_t CommunicationSystemThread = createCommunicationThread();
	pthread_join(CommunicationSystemThread, NULL);

	return 0;
}
