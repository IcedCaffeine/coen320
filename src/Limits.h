#ifndef LIMITS_H
#define LIMITS_H

// Radar
#define SIZE_SHM_PSR 4096
#define SIZE_SHM_SSR 4096
#define SIZE_SHM_AIRSPACE 8192
#define PRIMARY_SOURCE_RADAR_PERIOD 2000000
#define SECONDARY_SOURCE_RADAR_PERIOD 2000000

// Display Specifications
#define SCALER 3000
#define MARGIN 100000
#define DATA_DISPLAY_PERIOD 5000000 // 5sec period
#define SIZE_SHM_DISPLAY 8192

// Computer System specifications
#define CS_PERIOD 2000000
#define SIZE_SHM_PERIOD 64
#define NUM_PRINT 3

// Timer Specifications
#define ONE_THOUSAND 1000
#define ONE_MILLION 1000000


// Messages
#define MT_WAIT_DATA 2 // Message from client
#define MT_SEND_DATA 3 // Message from client

// Pulses
#define CODE_TIMER 1 // Pulse from timer

// Message reply definitions
#define MT_OK 0       // Message to client
#define MT_TIMEDOUT 1 // Message to client

// Airspace Specifications
#define SIZE_SHM_PLANES 128
#define OFFSET 1000000
#define MIN_X_AIRSPACE 0
#define MAX_X_AIRSPACE 100000
#define MIN_Y_AIRSPACE 0
#define MAX_Y_AIRSPACE 100000
#define MIN_Z_AIRSPACE 0
#define MAX_Z_AIRSPACE 25000
#define ALTITUDE 15000
#define AIRCRAFT_PERIOD 1000000

#endif /* LIMITS_H_ */
