#include "Radar.h"

Radar::Radar() {
	// TODO Auto-generated constructor stub

}

Radar::~Radar() {
	// TODO Auto-generated destructor stub
}


void* radarThreadServer(void * args){
	// Declare Variables
	name_attach_t *nameAttach = name_attach(NULL, CHANNEL, 0); // Name Attach Value
	RadarData radarData; // Plane Data
	int receiveId; // Receive Data

	// Check for Errors
	if(nameAttach == NULL){
		perror("Error while creating channel");
	}

	// Initialize Grid
	for(int i=0; i<ROW; i++){
		for(int j=0; j<COLUMN; j++){
			for(int k=0; k<DEPTH; k++){
				grid[i][j][k] = 0;
			}
		}
	}

	// Get Message from Client
	while(true){
		receiveId = MsgReceive(nameAttach->chid, &radarData, sizeof(radarData), NULL);
		if(receiveId == -1){
			perror("Error receiving message");
			break;
		}
		else if(receiveId == 0){
			// Pulse Received
			switch(radarData.header.code){
				case _PULSE_CODE_UNBLOCK:
					ConnectDetach(radarData.header.scoid);
					break;

				case _PULSE_CODE_DISCONNECT:
					break;

				default:
					break;
			}
		}

		// Send a EOK Message
		if(radarData.header.type == _IO_CONNECT){
			MsgReply(receiveId, EOK, NULL, 0);
			continue;
		}

		// Reject other IO Message
		if(radarData.header.type == _IO_CONNECT){
			MsgError(receiveId, ENOSYS);
			continue;
		}

		// Reply Client Error
		if(radarData.header.type == 0x00){
			if(radarData.header.type == 0x01){
				// Velocity Message Received
				if(radarData.arrivedyet){
					printf(" Plane %d outside of airspace! \n", (int)radarData.id);
				}
				else{
					// Setup Point in Grid
					int gridX = (radarData.positionX-MIN_X_REGION)/((MAX_X_REGION-MIN_X_REGION)/ROW);
					int gridY = (radarData.positionY-MIN_Y_REGION)/((MAX_Y_REGION-MIN_Y_REGION)/COLUMN);
					int gridZ = (radarData.positionZ-MIN_Z_REGION)/((MAX_Z_REGION-MIN_Z_REGION)/DEPTH);
					grid[gridX][gridY][gridZ] = 1;

					// Display Data
					printf("Plane %d ", (int)radarData.id);
					printf("Velocity = (%d.%d,%d)\n", (int)radarData.velocityX, radarData.velocityY, radarData.velocityZ);
					printf("Position = (%d.%d,%d)\n", (int)radarData.positionX, radarData.positionY, radarData.positionZ);

					// Setup Rest of the Grid
					for(int i=0;i<ROW;i++){
						for(int j=0;j<COLUMN;j++){
							int gridValue = 0;
							for(int k=0;k<DEPTH;k++){
								gridValue = grid[i][j][k];
							}
							(gridValue) ? printf(" %d ", (int)radarData.id) : printf(" 0 ");
						}
						printf("\n\n");
					}
					grid[gridX][gridY][gridZ] = 0;
				}
			}
			else{
				printf("Plane %d has not arrived!\n", (int)radarData.id);
			}
		}
		else if(radarData.header.type == 0x02){
			printf(" Position from Plane %d = (%d,%d,%d)",
					(int)radarData.id, (int)radarData.positionX, radarData.positionY, radarData.positionZ);
		}
		else{
			MsgError(receiveId, ENOSYS);
			continue;
		}
		MsgReply(receiveId, EOK, 0, 0);
	}

	// Destroy Channel
	name_detach(nameAttach, 0);
	return NULL;

}

// Main Function in Radar Function
int main(int argc, char*argv[]){
		//Declare thread
		pthread_t serverThread;
		int err_no;
		err_no = pthread_create(&serverThread, NULL, &radarThreadServer, NULL);
		if (err_no != 0){
			perror("Error creating server thread! \n");
		}

		//Join thread to start it
		err_no = pthread_join(serverThread, NULL);
		if (err_no != 0){
			perror("Error joining server thread!");
		}

		pthread_exit(EXIT_SUCCESS);
		return 0;
}
