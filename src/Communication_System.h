#ifndef COMMUNICATION_SYSTEM_H
#define COMMUNICATION_SYSTEM_H

#include <pthread.h>
#include <iostream>
#include <string>
#include <sys/dispatch.h>

class CommunicationSystem {
public:
    CommunicationSystem();
    ~CommunicationSystem();

    void start();
    void sendMessage(const void *msg, int size);
    void disconnectFromChannel();

private:
    pthread_t communicationThread;
    name_attach_t *channel;
    static void* communicationMain(void*);
};

#endif // COMMUNICATION_SYSTEM_H

