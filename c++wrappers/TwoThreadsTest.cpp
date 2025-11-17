#include "SimpleThread.h"
#include <unistd.h>
#include <stdio.h>

int main(){

    SimpleThread *thread1 = new SimpleThread("Jamaica");
    thread1->start();
    SimpleThread *thread2 = new SimpleThread("Fiji");
    thread2->start();
    getchar();

    return 0;
}

