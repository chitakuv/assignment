#include <pthread.h>   
#include <stddef.h>    
#include "callback.h"

void *func(void *arg){
    while(1){
        pthread_testcancel();
    }
    return NULL;
}

