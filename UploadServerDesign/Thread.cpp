#include "Thread.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "UploadServerThread.hpp"

void* startMethodInThread(void *arg)
{
	if (arg == NULL)
		return 0;
	UploadServerThread *thread = (UploadServerThread*)arg;
	thread->run();
	return NULL;
}
Thread::Thread(Thread *childThread) {
	this->state = malloc(sizeof(pthread_t));
	this->childThread = childThread;
}
void Thread::start() {
        pthread_t tid;
	pthread_create(&tid, NULL, startMethodInThread, (void *) this);
	memcpy(this->state, (const void *)&tid, sizeof(pthread_t));
}
Thread::~Thread() {
	free(this->state);
}