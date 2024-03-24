/*
 * Timer.cpp
 *
 *  Created on: Mar. 19, 2024
 *      Author: Srira
 */

#include "Timer.h"

Timer::Timer(uint32_t timeInSec) {
	Timer(floor(timeInSec), (timeInSec-floor(timeInSec))*1000);
}

Timer::Timer(uint32_t sec, uint32_t msec) {
	this->channelId = ChannelCreate(0);
	this->connectionId = ConnectAttach(0,0,this->channelId,0,0);
	if(this->connectionId == -1){
		std::cerr << "Timer, Connect Attach error : " << errno << "\n";
	}

	SIGEV_PULSE_INIT(&(this->event), this->connectionId, SIGEV_PULSE_PRIO_INHERIT, 1, 0);

	// Setup Timer
	if(timer_create(CLOCK_REALTIME, &this->event, &this->timerId) == -1){
		std::cerr << "Timer, Init error : " << errno << "\n";
	}
	setTimerSpec(sec,1000000* msec);
	this->cyclePerSecond = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
}

Timer::~Timer() {
	// TODO Auto-generated destructor stub
}

int Timer::getChannelId() const {
	return channelId;
}

void Timer::setChannelId(int channelId) {
	this->channelId = channelId;
}

int Timer::getConnectionId() const {
	return connectionId;
}

void Timer::setConnectionId(int connectionId) {
	this->connectionId = connectionId;
}

uint64_t Timer::getCyclePerSecond() const {
	return cyclePerSecond;
}

void Timer::setCyclePerSecond(uint64_t cyclePerSecond) {
	this->cyclePerSecond = cyclePerSecond;
}

const struct sigevent& Timer::getEvent() const {
	return event;
}

void Timer::setEvent(const struct sigevent &event) {
	this->event = event;
}

const char* Timer::getMsgBuffer() const {
	return msgBuffer;
}

uint64_t Timer::getTickCycles() const {
	return tickCycles;
}

void Timer::setTickCycles(uint64_t tickCycles) {
	this->tickCycles = tickCycles;
}

timer_t Timer::getTimerId() const {
	return timerId;
}

void Timer::setTimerId(timer_t timerId) {
	this->timerId = timerId;
}

const struct itimerspec& Timer::getTimerSpec() const {
	return timerSpec;
}

void Timer::setTimerSpec(uint32_t sec, uint32_t msec){
	this->timerSpec.it_value.tv_sec = sec;
	this->timerSpec.it_value.tv_nsec = msec*1000;
	this->timerSpec.it_interval.tv_sec = sec;
	this->timerSpec.it_interval.tv_nsec = msec*1000;
	timer_settime(this->timerId, 0, &(this->timerSpec), NULL);
}

uint64_t Timer::getTockCycles() const {
	return tockCycles;
}

void Timer::setTockCycles(uint64_t tockCycles) {
	this->tockCycles = tockCycles;
}

void Timer::waitTimer(){
	int rcvid;
	void * message = (void*)(this->getMsgBuffer());
	rcvid = MsgReceive(this->getChannelId(), &(message), sizeof(message), NULL);
}

void Timer::startTimer(){
	timer_settime(this->getTimerId(), 0, &(this->getTimerSpec()), NULL);
}

void Timer::tick(){
	this->setTickCycles(ClockCycles());
}

double Timer::tock(){
	this->setTockCycles(ClockCycles());
	return (double)((int)(((double)(this->getTockCycles()-this->getTickCycles())/this->getCyclePerSecond())*100000))/10;
}


