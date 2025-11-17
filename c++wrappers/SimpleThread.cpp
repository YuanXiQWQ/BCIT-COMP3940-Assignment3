#include "SimpleThread.h"
#include <unistd.h>
#include <stdio.h>
SimpleThread::SimpleThread(char* name):Thread(this) {
   this->_name = name;
}
void SimpleThread::run() {
  for (int i=0; i < 10; i++) {
    printf("%s\n", this->_name);
    sleep(1);
  } 
        
}
SimpleThread::~SimpleThread() {

}