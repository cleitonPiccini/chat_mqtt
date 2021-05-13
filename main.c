#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <stdlib.h> 
#include <sys/time.h>
#include "MQTTAsync.h"
#include <unistd.h>
#include "fun.c"

/*#if defined(_WRS_KERNEL)
#include <OsWrapper.h>
#endif
*/
int main (){

    MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    /*char user[MAX_NAME_USER];
    char id_user[4] = "ID_";
    
    printf("Informe seu nome de Usuário para o CHAT :\n");
    scanf("%s", user);
    strcat(id_user, user);
    strcpy(TOPIC, id_user);
    //TOPIC = strncpy(id_user);
    //id_user += "oi";
    //TOPIC = id_user;
    printf("%s", id_user);
    */
    menu();
    
	if ((rc = MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to create client object, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	if ((rc = MQTTAsync_setCallbacks(client, NULL, connlost, messageArrived, NULL)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to set callback, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	printf("Waiting for publication of %s\n"
         "on topic %s for client with ClientID: %s\n",
         PAYLOAD, TOPIC, CLIENTID);
	/*while (!finished)
    //while (1)
		#if defined(_WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif
*/
    while (!finished) {
		//int64_t t = getTime();

		char buf[256];
        //buf = malloc (256 * sizeof (char));
		int n; //= snprintf(buf, sizeof(buf), "%lld", (long long) t);
        //pthread_mutex_lock(&printf_mutex);
		printf("tamanho da string %d\n",n);
        scanf("%s",buf);
        //buf = getchar();
        n = (int) strlen(buf);
        printf("tamanho da string %d\n",n);

		opts.onSuccess = onSend;
		opts.onFailure = onSendFailure;
		opts.context = client;

		pubmsg.payload = buf;
		pubmsg.payloadlen = n;
		pubmsg.qos = QOS;
		pubmsg.retained = 0;

		if ((rc = MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
		{
			printf("Failed to start sendMessage, return code %d\n", rc);
			exit(EXIT_FAILURE);
		}
        //pthread_mutex_unlock(&printf_mutex);
		#if defined(_WIN32)
			Sleep(SAMPLE_PERIOD);
		#else
			usleep(1000);
		#endif
	}

	MQTTAsync_destroy(&client);
 	return rc;

}