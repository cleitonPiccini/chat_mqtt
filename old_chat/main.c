#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <stdlib.h> 
#include <sys/time.h>
//#include "MQTTAsync.h"
#include "MQTTClient.h"
#include <unistd.h>
#include "fun.c"

int main (){

    /*MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;
	int option_user, option_chat, option_topic;
	int exit_ = 1;
	char buffer_ [MAX_NAME_USER + MAX_MESSAGE];*/
	
	pthread_t threads[2];
	//pthread_create(&(threads[0]), NULL, pub, NULL);
	//pthread_create(&(threads[1]), NULL, sub, NULL);

	pthread_create(&(threads[0]), NULL, sub, NULL);
	pthread_create(&(threads[1]), NULL, pub, NULL);
	pthread_join(threads[1], NULL);
	pthread_join(threads[0], NULL);
	
	//pthread_join(threads[0], NULL);
	//char message[MAX_NAME_USER+MAX_MESSAGE];
	//Definição de usuario para conectar ao broker.
	/*while (exit_ == 1)
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
		
		//Opção 1 conversa particular.
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
			pthread_mutex_lock(&resposta_convite_mutex);
			resposta_convite = 1;
			pthread_mutex_unlock(&resposta_convite_mutex);
			
			printf("Aguardado resposta do %s", TOPIC);
			
			while (resposta_convite == 1)
			{
				send_message(client, opts, pubmsg, 1);
				sleep(10);
			}
			disconnect(client);
			sleep(2);
			if (resposta_convite == 2)
			{
				convite_chat(client, conn_opts);
				sleep(2);
				while (!finished) 
				{
					send_message(client, opts, pubmsg, 3);
				}
				continue; //Tem que sair desse while e ir para o chat.
				//Assina o novo topico = junção dos dois usuários.
			}
			if (resposta_convite == 3)
			{
				//Fecha a assinatura ea publicação no tópico.
				continue;
			}
			//exit_ = 0;
			
		} 
		
		//Opção 2 conversa em grupo. (falta completar está parte)
		if (option_chat == 2){
			printf("O CHAT com todos os usuários online foi iniciado:\n");
			exit_ = 0;
			sleep(8);
		} 
		
		//Opção 3 esperar ser chamado o chat.
		if (option_chat == 3){
			printf("Aguardando convite para o CHAT:\n");
			pthread_mutex_lock(&espera_convite_mutex);
			espera_convite = 1;
			pthread_mutex_unlock(&espera_convite_mutex);
			strcpy(TOPIC, CLIENTID);
			convite_chat(client, conn_opts);
			//Aguarda ser chamado por alguém.
			while (espera_convite == 1)
			{
				
			}
			
			setbuf(stdin, 0);
			fgets(buffer_, sizeof(buffer_), stdin);
    		//strcat(message, CLIENTID );
			//strcat(message, " => ");
			//strcat(message, buffer_);
			if (strcmp(buffer_, "--s"))
			{
				send_message(client, opts, pubmsg, 4);
				sleep(2);
				disconnect(client);
				sleep(2);
				//strcat(TOPIC, TOPIC_AUX);
				strcat(TOPIC_AUX, TOPIC);
				convite_chat(client, conn_opts);
				sleep(2);
				while (!finished) 
				{
					send_message(client, opts, pubmsg, 3);
				}
				continue;
			}
			if (strcmp(buffer_, "--n"))
			{
				send_message(client, opts, pubmsg, 5);
				sleep(2);
				disconnect(client);
				sleep(2);
				continue;
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


	MQTTAsync_destroy(&client);
 	return rc;
*/
	return 0;
}