#ifndef TIMER_H
#define TIMER_H

// C++ Header
#include <sys/neutrino.h>
#include <sys/siginfo.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// Object Header
#include "Limits.h"


// Structures
typedef struct {
  int messageType; // contains both message to and from client
  int messageData; // optional data, depending upon message
} ClientMessage;

typedef union {
  ClientMessage clientMessage;   // a message can be either from a client
  struct _pulse pulse; // a pulse
} Message;

class Timer {
private:
	// Data
	int connectionId;
	timer_t timerId;         // timer ID
	struct sigevent event;   // event to deliver
	struct itimerspec timer; // timer data structure
	
public:
	// Constructor
	Timer(int channelId);
	int setTimer(int offset, int period);
	
	// Set & Get
	int getConnectionId() const;
	void setConnectionId(int connectionId);
};

#endif /* TIMER_H */
