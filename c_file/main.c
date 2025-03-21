#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "callback.h"

int main()
{
    pthread_t tid;
    if(pthread_create(&tid, NULL,func,NULL) != 0){
        perror("fail to create pthread");
        exit(1);
    }

    printf("I am main thread, will kill %lu after 3s\n", tid);
    pthread_cancel(tid);
    
    int ret = pthread_join(tid, NULL);
    if(ret > 0){
        printf("ret = %d, msg = %s\n", ret, strerror(ret));
    }

    return 0;
}

