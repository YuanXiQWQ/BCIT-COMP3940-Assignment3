#pragma once
#include "Thread.h"
class SimpleThread : public Thread {
public:
	SimpleThread(char* name);
	void run();
	~SimpleThread();
private:
   char* _name;
};
