/*
 * semaphore.h
 *
 *  Created on: 26 нояб 2016 г.
 *      Author: Dexiz
 */

#ifndef CUSTOMSEMAPHORE_H_
#define CUSTOMSEMAPHORE_H_

#include <condition_variable>
#include <mutex>

class Semaphore
{
	std::condition_variable conVar;
	std::mutex mut;
	int count;


public:
	Semaphore();

	void signalWorkStarted();
	void signalWorkFinished();
	void wait();
};


#endif /* CUSTOMSEMAPHORE_H_ */
