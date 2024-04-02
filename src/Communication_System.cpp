#include "Communication_System.h"
#include "Aircraft.h"

CommunicationSystem::CommunicationSystem() : channel(nullptr) {}

CommunicationSystem::~CommunicationSystem() {
    if (channel != nullptr) {
        name_detach(channel, 0);
    }
}

void CommunicationSystem::start() {
    std::cout << "The communication has begun..." << std::endl;

    std::string channelName = "CommunicationChannel";
    std::cout << "Creating messaging system with " << channelName << std::endl;

    channel = name_attach(NULL, channelName.c_str(), 0);

    if (channel == nullptr) {
        std::cerr << "Error in creating the channel" << std::endl;
        return;
    }

    if (pthread_create(&communicationThread, NULL, &CommunicationSystem::communicationMain, this) != 0) {
        std::cerr << "Error creating communication thread" << std::endl;
        return;
    }
}

void CommunicationSystem::sendMessage(const void *msg, int size) {
    if (channel == nullptr) {
        std::cerr << "Channel is not initialized" << std::endl;
        return;
    }

    int status = MsgSend(channel->chid, msg, size, NULL, 0);
    if (status == -1) {
        std::cerr << "Error sending message" << std::endl;
    }
}

void CommunicationSystem::disconnectFromChannel() {
    if (channel != nullptr) {
        int status = name_detach(channel, 0);
        if (status == -1) {
            std::cerr << "Error detaching connection" << std::endl;
        }
        channel = nullptr;
    }
}

void* CommunicationSystem::communicationMain(void* arg) {
    CommunicationSystem* commSys = static_cast<CommunicationSystem*>(arg);
    if (commSys == nullptr) {
        std::cerr << "Invalid argument for communication thread" << std::endl;
        return nullptr;
    }

    int receivedID;
    while (true) {
        // Assuming PlaneClass is defined somewhere else
        Aircraft message;

        receivedID = MsgReceive(commSys->channel->chid, &message, sizeof(message), NULL);
        if (receivedID == -1) {
            std::cerr << "Error receiving message" << std::endl;
            continue;
        }

        std::cout << "Message received!" << std::endl;
        // Include forwarding message by operator to airplane
    }

    return nullptr;
}
