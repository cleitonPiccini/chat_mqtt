#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <stdlib.h> 
#include <sys/time.h>
#include "MQTTAsync.h"
#include <unistd.h>
#include "fun.c"

int main (){

    MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;
	int option_user, option_chat, option_topic;
	int exit_ = 1;
	//char message[MAX_NAME_USER+MAX_MESSAGE];
	//Definição de usuario para conectar ao broker.
	while (exit_ == 1)
	{
		printf("\e[H\e[2J");
		printf("Escolha uma das opções a baixo para ser seu Usuário no CHAT :\n");
		printf("Digite 1 para ser o user_01.\n");
		printf("Digite 2 para ser o user_02.\n");
		printf("Digite 3 para ser o user_03.\n");
		printf("Digite 4 para ser o user_04.\n");
		printf("Digite 5 sair.\n");

    	scanf("%d", &option_user);
		switch(option_user)
		{
			case 1:
				strcpy(CLIENTID, "U1_Control");
				exit_ = 0;
				break;
			case 2:
				strcpy(CLIENTID, "U2_Control");
				exit_ = 0;
				break;
			case 3:
				strcpy(CLIENTID, "U3_Control");
				exit_ = 0;
				break;
			case 4:
				strcpy(CLIENTID, "U4_Control");
				exit_ = 0;
				break;
			default:
				printf("Valor invalido tente novamente.");
		}	
	}
	//Se conecta ao broker.
	if ((rc = MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to create client object, return code %d\n", rc);
		exit(EXIT_FAILURE);
		MQTTAsync_destroy(&client);
 		return rc;
	}
	
	exit_ = 1;
	while (exit_ == 1)
	{
		printf("\e[H\e[2J");
		printf("Escolha uma das duas opções de conversa :\n");
		printf("Conversa particular entre você e outro usuário => Digite 1\n");
		printf("Conversa em grupo com todos os usuários online => Digite 2\n");
		printf("Aguardar convite de outro usuário => Digite 3\n");
		printf("Sair do CHAT => Digite 4\n");

		scanf("%d", &option_chat);
		printf("\e[H\e[2J");
		
		if (option_chat == 1)
		{
			printf("Escolha o outro usuário para a conversa:\n");
			if (strcmp(CLIENTID, "U1_Control")) printf("Digite 1 para conversar com o user 1 :\n");
			if (strcmp(CLIENTID, "U2_Control")) printf("Digite 2 para conversar com o user 2 :\n");
			if (strcmp(CLIENTID, "U3_Control")) printf("Digite 3 para conversar com o user 3 :\n");
			if (strcmp(CLIENTID, "U4_Control")) printf("Digite 4 para conversar com o user 4 :\n");

			scanf("%d", &option_topic);

			switch(option_topic)
			{	
				case 1:
					strcpy(TOPIC, "U1_Control");
					exit_ = 0;
					break;
				case 2:
					strcpy(TOPIC, "U2_Control");
					exit_ = 0;
					break;
				case 3:
					strcpy(TOPIC, "U3_Control");
					exit_ = 0;
					break;
				case 4:
					strcpy(TOPIC, "U4_Control");
					exit_ = 0;
					break;
				default:
					printf("Valor invalido tente novamente.\n");
					sleep(2);
					continue;
			}
			convite_chat(client, conn_opts);
			sleep(1);
			pthread_mutex_lock(&espera_convite_mutex);
			espera_convite = 1;
			pthread_mutex_unlock(&espera_convite_mutex);
			printf("Aguardado resposta do %s", TOPIC);
			while (espera_convite == 1)
			{
				send_message(client, opts, pubmsg, 1);
				sleep(8);
			}
			
			
			/*if ((rc = MQTTAsync_setCallbacks(client, NULL, connlost, messageArrived, NULL)) != MQTTASYNC_SUCCESS)
			{
				printf("Failed to set callback, return code %d\n", rc);
				exit(EXIT_FAILURE);
			}*/
			/*conn_opts.keepAliveInterval = 20;
			conn_opts.cleansession = 1;
			conn_opts.onSuccess = onConnect;
			conn_opts.onFailure = onConnectFailure;
			conn_opts.context = client;
			if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
			{
				printf("Failed to start connect, return code %d\n", rc);
				exit(EXIT_FAILURE);
			}
			*/
			//while ( !resposta_convite )
			//{
				/* code */
			//}
			

		} else if (option_chat == 2){
			printf("O CHAT com todos os usuários online foi iniciado:\n");
			exit_ = 0;
			sleep(8);
		} else if (option_chat == 3){
			printf("Aguardando convite para o CHAT:\n");
			pthread_mutex_lock(&resposta_convite_mutex);
			resposta_convite = 1;
			pthread_mutex_unlock(&resposta_convite_mutex);
			strcpy(TOPIC, CLIENTID);
			convite_chat(client, conn_opts);
			while (resposta_convite == 1)
			{
				/* code */
			}
		} else if (option_chat == 4){
			exit_total = 0;
		} else{
			printf("Valor invalido tente novamente.");
			continue;
		}
	}
	printf("\e[H\e[2J");
	printf("USUÁRIO = %s\n", CLIENTID);

	
	
	//---------------------------------------------------
	/*if ((rc = MQTTAsync_setCallbacks(client, NULL, connlost, messageArrived, NULL)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to set callback, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}*/

	
	/*
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
*/
    while (!finished) {

		send_message(client, opts, pubmsg, 3);
	}

	MQTTAsync_destroy(&client);
 	return rc;

}