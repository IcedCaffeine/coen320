/*
 * Aircraft.h
 *
 *  Created on: Mar 23, 2024
 *      Author: Fahad Abdul Rahman
 *      #40157997
 */

#ifndef AIRCRAFT_H
#define AIRCRAFT_H

class Aircraft{
	private:
		int posX,posY,posZ;
		int velX,velY,velZ;
		int aircraftId;
		int arrivalTime;

	public:
		Aircraft();
		Aircraft(int aircraftId, int posX, int posY, int posZ, int velX, int velY, int velZ, int arrivalTime);
		~Aircraft();

		int getAircraftId() const;
		void setAircraftId(int aircraftId);
		int getArrivalTime() const;
		void setArrivalTime(int arrivalTime);
		int getPosX() const;
		void setPosX(int posX);
		int getPosY() const;
		void setPosY(int posY);
		int getPosZ() const;
		void setPosZ(int posZ);
		int getVelX() const;
		void setVelX(int velX);
		int getVelY() const;
		void setVelY(int velY);
		int getVelZ() const;
		void setVelZ(int velZ);

		void updatePosition();
		bool outsideAirSpace();
};



#endif /* SRC_AIRCRAFT_H_ */
