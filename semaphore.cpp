/*
 * semaphore.cpp
 *
 *  Created on: 26 нояб. 2016 г.
 *      Author: Dexiz
 */
#include"semaphore.h"

Semaphore::Semaphore()
: count(0)
{

}

void Semaphore::signalWorkStarted()
{
	//mut.lock();
	std::unique_lock<std::mutex> lk(mut);
	++count;
}

void Semaphore::signalWorkFinished()
{
	//mut.lock();
	std::unique_lock<std::mutex> lk(mut);
	--count;

	if (count == 0)
	{
		conVar.notify_one();
	};
}

void Semaphore::wait()
{
	std::unique_lock<std::mutex> lk(mut);

	while (count != 0) // защита от ложных пробуждений
		conVar.wait(lk); // освобождает мьютекс
}
