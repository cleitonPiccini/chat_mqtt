#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h> 
#include <sys/time.h>
#include "fun.c"
#include "MQTTAsync.h"

#if !defined(_WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif

#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif

int main (){

    start_chat();
    return 0;

}