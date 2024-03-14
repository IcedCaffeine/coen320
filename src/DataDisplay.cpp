#include "DataDisplay.h"

DataDisplay::DataDisplay() {
	this->fileId = -1;
	this->channelId = -1;

}

DataDisplay::~DataDisplay() {
	// TODO Auto-generated destructor stub
}

int DataDisplay::getChannelId() const {
	return channelId;
}

int DataDisplay::getFileId() const {
	return fileId;
}

void DataDisplay::setFileId(int fileId) {
	this->fileId = fileId;
}

void DataDisplay::setChannelId(int channelId) {
	this->channelId = channelId;
}

void DataDisplay::run(){
	// Channel For Aircraft
	this->setChannelId(ChannelCreate(0));
	if(this->getChannelId() == -1){
		printf("Failed to create Channel. Exit Thread! \n");
	}

	this->setFileId(creat("/data/home/qnxuser/file.dat", S_IRUSR | S_IWUSR | S_IXUSR));
	if (this->getFileId() == -1)
	{
		std::cout << "Log file could not be created" << std::endl;
	}

	// Get Message
	this->getMessage();

}

void DataDisplay::getMessage(){
	// Get Aircrafts
	planeMessage msg;
	int receiveId;

	while(true){
		receiveId = MsgReceive(this->getChannelId(), &msg, sizeof(msg), NULL);

		// Write Based on Message
		switch(msg.command){
			case WARNING:{
				// Acknowledge message
				MsgReply(receiveId, EOK, NULL, 0);

				// Get Plane Position and Velocity
				printf("WARNING\n");
				printf("Aircraft ID : %d\n", (int) msg.onePlane.getAircraftId());
				printf("Position (X,Y,Z) = %d,%d,%d\n",
					(int)msg.onePlane.getPosX(),
					(int)msg.onePlane.getPosY(),
					(int)msg.onePlane.getPosZ()
				);
				printf("Velocity (X,Y,Z) = %d,%d,%d\n",
					(int)msg.onePlane.getVelX(),
					(int)msg.onePlane.getVelY(),
					(int)msg.onePlane.getVelZ()
				);
				break;
			}
			case PLANE:{
				// Acknowledge message
				MsgReply(receiveId, EOK, NULL, 0);

				// Get Plane Position and Velocity
				printf("Aircraft ID : %d\n", (int) msg.onePlane.getAircraftId());
				printf("Position (X,Y,Z) = %d,%d,%d\n",
						(int)msg.onePlane.getPosX(),
						(int)msg.onePlane.getPosY(),
						(int)msg.onePlane.getPosZ()
				);
				printf("Velocity (X,Y,Z) = %d,%d,%d\n",
						(int)msg.onePlane.getPosX(),
						(int)msg.onePlane.getPosY(),
						(int)msg.onePlane.getPosZ()
				);
				break;
			}
			case GRID:{
				// Acknowledge message
				MsgReply(receiveId, EOK, NULL, 0);
				std::cout << this->makeGrid(msg.planes); // Draw Grid
				break;
			}
			case LOG:{
				// Get String
				const char* buffer = this->makeGrid(msg.planes).c_str();
				MsgReply(receiveId, EOK, NULL, 0);

				// Get Size of Grid
				int writtenSize = write(this->getFileId(), buffer, sizeof(buffer));
				if(writtenSize != sizeof(buffer)){
					perror("Error writing to Log File");
				}
				break;
			}

		}
	}

}

std::string DataDisplay::makeGrid(std::vector <Aircraft> aircrafts){
	std::string grid[ROW][COLUMN];

	// Generate 3D Grid
	for(unsigned int i=0; i<aircrafts.size(); i++){
		// Rows
		for(int x=0;x<ROW;x++){
			bool validPosition =
					aircrafts[i].getPosX() >= (CELLSIZE *x) &&
					aircrafts[i].getPosY() >= (CELLSIZE *(x+1));

			if(validPosition){
				// Columns
				for(int y=0;y<COLUMN;y++){
					validPosition =
							aircrafts[i].getPosX() >= (CELLSIZE *y) &&
							aircrafts[i].getPosY() >= (CELLSIZE *(y+1));
					if(validPosition){
						if(grid[x][y] != ""){
							grid[x][y] += " & ";
						}
						grid[x][y] += std::to_string(aircrafts[i].getAircraftId());
					}
				}
			}
		}
	}

	// Print Grid
	std::stringstream out;
	for(int i=0;i<ROW;i++){
		for(int j=0;j<COLUMN;j++){
			if(grid[i][j] == ""){
				out << "|        ";
			}
			else{
				out << "|";
				out << grid[i][j] << "    ";
			}
		}

		for(int i=0;i<10*ROW;i++){
			out << "-";
		}
		out << "\n";
	}
	out << "\n";

	return out.str();
}



