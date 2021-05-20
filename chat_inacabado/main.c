#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <stdlib.h> 
#include <sys/time.h>
#include "MQTTClient.h"
#include <unistd.h>
#include "fun.c"

int main (){

	start_chat();

	printf("Fechando a aplicação de CHAT \n");

	return 0;
}