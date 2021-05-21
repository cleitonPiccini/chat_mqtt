#include "fun.h"

//Falha na comunicação.
void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

//Mensagem entregue ao broker.
void delivered(void *context, MQTTClient_deliveryToken dt)
{
    //printf("\n");
    deliveredtoken = dt;
}

//Função de CallBack que recebe as publicações nos tópicos.
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{

	if (flag_pub_global == 0)
	{
		pthread_mutex_lock(&new_message_mutex);
		strcpy(send_message_global, (char*)message->payload);
		strcpy(send_message_topicname_global, (char*)topicName);
		new_message_global	= 1;
	
	}
	pthread_mutex_lock(&flag_pub_mutex);
	flag_pub_global = 0;
	pthread_mutex_unlock(&flag_pub_mutex);

	MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

//Modela o payload 
void payload_message( char buffer_[MAX_NAME_USER + MAX_MESSAGE], int type_msg)
{   
	char message[MAX_NAME_USER + MAX_MESSAGE] = "";
	
	pthread_mutex_lock(&payload_mutex);
	
	switch(type_msg){
		//Mensagem de convite
		case 1:
			strcpy(message, "--Solicitacao " );
			strcat(message, CLIENTID );
			strcat(message, "\n" );
			strcpy(PAYLOAD, message);
			break;
		//Mensagem avisando que está saindo do CHAT.
		case 2:
			strcpy(message, CLIENTID );
			strcat(message, " Saiu do CHAT\n" );
			strcpy(PAYLOAD, message);
			break;
		//Mensagem de conversação do CHAT.
		case 3:
			strcpy(message, CLIENTID );
			strcat(message, " => ");
			strcat(message, buffer_);
			strcpy(PAYLOAD, message);
			break;
		//Mensagem de sim para solicitação no chat.
		case 4:
			strcpy(message, CLIENTID);
			strcat(message, "-s\n" );
			strcpy(PAYLOAD, message);
			break;
		//Mensagem de não para solicitação no chat.
		case 5:
			strcpy(message, CLIENTID);
			strcat(message, "-n\n" );
			strcpy(PAYLOAD, message);
			break;

		case 6: 
			strcpy(message, CLIENTID);
			strcat(message, " Está ocupado\n");
			strcpy(PAYLOAD, message);
			break;

		//Valor do tipo de mensagem errado.
		default:
			printf("Erro, tipo de mensagem invalido\n");
			strcpy(PAYLOAD, "***Erro mensagem***\n");
	}
    pthread_mutex_unlock(&payload_mutex);
}

//Controle do Client MQTT
void * mqtt_control ()
{

	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    	MQTTClient_message pubmsg = MQTTClient_message_initializer;
    	MQTTClient_deliveryToken token;
	int rc;

	conn_opts.keepAliveInterval = 20;
   	conn_opts.cleansession = 1;

	while (!finished)
	{
		//Cria um novo cliente.
		while (create_client_global == 0 && !finished);
	
		if (create_client_global == 1 && !finished)
		{
			//Se conecta ao broker.
			if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    		{
        		printf("Failed to create client, return code %d\n", rc);
        		rc = EXIT_FAILURE;
        		finished = 1;
    		}
			if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    		{
        		printf("Failed to set callbacks, return code %d\n", rc);
        		rc = EXIT_FAILURE;
        		finished = 1;
    		}
			//Se conecta ou Broker.
	    	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    		{
    	    	printf("Failed to connect, return code %d\n", rc);
    	    	rc = EXIT_FAILURE;
    	    	finished = 1;
				//goto destroy_exit;
    		}
			
			//pthread_mutex_lock(&create_client_mutex);
			create_client_global = 0;
			pthread_mutex_unlock(&create_client_mutex);
		}
	

		while (!finished)
		{
			//Efetua assinatura em tópicos.
			if (subscribe_topic_global == 1)
			{
				//Assina tópico.
				if ((rc = MQTTClient_subscribe(client, TOPIC, QOS)) != MQTTCLIENT_SUCCESS)
    			{
    				printf("Failed to subscribe, return code %d\n", rc);
    				rc = EXIT_FAILURE;
					finished = 1;
					continue;
    			}
				//pthread_mutex_lock(&subscribe_topic_mutex);
				subscribe_topic_global = 0;
				pthread_mutex_unlock(&subscribe_topic_mutex);
			}
			//Efetua a publicação nos tópicos (Envia mensagens).
			if ( control_message_global == 1 )
			{
				pubmsg.payload = PAYLOAD;
    			pubmsg.payloadlen = (int)strlen(PAYLOAD);
    			pubmsg.qos = QOS;
    			pubmsg.retained = 0;

				//Evita o "eco" nas mensagens enviadas.
				pthread_mutex_lock(&flag_pub_mutex);
				flag_pub_global = 1;

    			if ((rc = MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    			{
        			printf("Failed to publish message, return code %d\n", rc);
					rc = EXIT_FAILURE;
					finished = 1;
					continue;
    			}

				pthread_mutex_unlock(&flag_pub_mutex);
			    
				rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);

				control_message_global = 0;
				pthread_mutex_unlock(&control_message_mutex);

    		}

			if (unsubiscribe_topic_global == 1)
			{
				if ((rc = MQTTClient_unsubscribe(client, TOPIC) != MQTTCLIENT_SUCCESS))
    			{
    				printf("Failed to unsubscribe, return code %d\n", rc);
        			rc = EXIT_FAILURE;
					finished = 1;
					continue;
    			}
				//pthread_mutex_lock(&unsubiscribe_topic_mutex);
				unsubiscribe_topic_global = 0;
				pthread_mutex_unlock(&unsubiscribe_topic_mutex);
			}

		}
	}
    MQTTClient_destroy(&client);
	sleep(1);
	printf("Controle do Client MQTT foi fechado\n");
	pthread_exit(NULL);
}

//Função a ser executada na thread que recebe as mensagens, identifica-as e toma uma decisão.
void* subscribe_thread ()
{
	char message [MAX_MESSAGE + MAX_NAME_USER] = "";
	char message_s [MAX_MESSAGE + MAX_NAME_USER] = "";
	char message_n [MAX_MESSAGE + MAX_NAME_USER] = "";
	char TOPIC_AUX [MAX_NAME_USER] = "";
	char *token = NULL;
	
	while (!finished)
	{
		if (new_message_global == 1)
		{	
			strcpy(message, send_message_global);
			

			//Aplicação esperando a resposta do convite feito a outro usuário.
			if (resposta_convite_global == 1)
			{
				strcpy(message_s, TOPIC);
				strcat(message_s, "-s\n");
				strcpy(message_n, TOPIC);
				strcat(message_n, "-n\n");

				//Respondeu que sim.
				if ( strcmp(message, message_s) == 0 )
				{
					//printf("respondeu sim\n");//usando para debug.
					strcpy(TOPIC_CHAT, CLIENTID);
					strcat(TOPIC_CHAT, "-");
					strcat(TOPIC_CHAT, TOPIC);	
					resposta_convite_global = 2;
					pthread_mutex_unlock(&resposta_convite_mutex);
					
				//Respondeu que não.
				} else if ( strcmp(message, message_n) == 0) {
					resposta_convite_global = 3;
					pthread_mutex_unlock(&resposta_convite_mutex);
				} else {
					printf("Erro na resposta: ");
				}
				strcpy(message_s, "");
				strcpy(message_n, "");
			}

			//Aplicação esperando ser chada.
			if (espera_convite_global == 1)
			{
				token = strtok(message, "\n");
				token = strtok(token, " ");

				if (strcmp(token, "--Solicitacao") == 0) {
					
					token = strtok(NULL, " ");
					strcpy (TOPIC_AUX, token);
					printf("Vc recebeu uma solicitação do Usuário %s para o CHAT\n", TOPIC_AUX);
					strcpy(TOPIC_PUB, TOPIC_AUX);
					strcat (TOPIC_AUX, "-");
					strcat (TOPIC_AUX, CLIENTID);
					strcpy (TOPIC_CHAT, TOPIC_AUX);
					printf("Para aceitar => Digite -s\n");
					printf("Para recusar => Digite -n\n");

				} else {
					printf("Mensagem fora de padão\n");
				}
			}
			
			if (libera_print_message_global == 1){
				pthread_mutex_lock(&printf_mutex);
    			printf("%s", message);
    			pthread_mutex_unlock(&printf_mutex);
			}

			strcpy(message, " ");
			new_message_global	= 0;
			pthread_mutex_unlock(&new_message_mutex);
		}
	}
//flag_local_pub = 1;
sleep(2);
printf("Controle de assinaturas dos Topicos foi fechado\n");
pthread_exit(NULL);
}

//Função a ser executada na thread que lê e envia as mensagens da entrada padrão.
void* menu_thread (void *data)
{
	int i;
	int option_user, option_chat, option_topic;
	int exit_ = 1;
	char buffer_ [MAX_NAME_USER + MAX_MESSAGE];
	
	printf("\e[H\e[2J");
	while (exit_ == 1 && finished == 0)
	{
		printf("\n\n");
		printf("Escolha uma das opções a baixo para ser seu Usuário no CHAT :\n\n");
		printf("Para ser o user_01 => Digite 1.\n");
		printf("Para ser o user_02 => Digite 2.\n");
		printf("Para ser o user_03 => Digite 3.\n");
		printf("Para ser o user_04 => Digite 4.\n");
		printf("Para sair          => Digite 5.\n");

    	scanf("%d", &option_user);
		printf("\n");

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
			case 5:
				goto destroy_exit;
			default:
				printf("Valor invalido tente novamente.\n");
		}	
	}
	//Cria client
	//Ativa as funções de CallBack.
	//Se conecta ao broker.
	pthread_mutex_lock(&create_client_mutex);
	create_client_global = 1;
	while (create_client_global == 1);
	
	pthread_mutex_lock(&subscribe_topic_mutex);
	strcpy(TOPIC, CLIENTID);//Tópico do usuário.
	subscribe_topic_global = 1;
	while (subscribe_topic_global == 1);

	exit_ = 1;
	while (exit_ == 1 && finished == 0)
	{
		libera_print_message_global = 0;
		printf("\n");
		printf("Escolha uma das opções de conversa :\n\n");
		printf("Conversa particular entre você e outro usuário => Digite 1.\n");
		printf("Conversa em grupo com todos os usuários online => Digite 2.\n");
		printf("Aguardar convite de outro usuário              => Digite 3.\n");
		printf("Para sair                                      => Digite 5.\n");

		scanf("%d", &option_chat);
		printf("\n");
		
		//Opção 1 conversa particular.
		if (option_chat == 1)
		{
			printf("\n");
			printf("Escolha o outro usuário para a conversa:\n\n");
			if (strcmp(CLIENTID, "U1_Control")) printf("Para conversar com o user 1 => Digite 1.\n");
			if (strcmp(CLIENTID, "U2_Control")) printf("Para conversar com o user 2 => Digite 2.\n");
			if (strcmp(CLIENTID, "U3_Control")) printf("Para conversar com o user 3 => Digite 3\n");
			if (strcmp(CLIENTID, "U4_Control")) printf("Para conversar com o user 4 => Digite 4\n");
			printf("Para sair                   => Digite 5.\n");

			scanf("%d", &option_topic);
			printf("\n");

			switch(option_topic)
			{	
				case 1:
					strcpy(TOPIC, "U1_Control");
					break;
				case 2:
					strcpy(TOPIC, "U2_Control");
					break;
				case 3:
					strcpy(TOPIC, "U3_Control");
					break;
				case 4:
					strcpy(TOPIC, "U4_Control");
					break;
				case 5:
					finished = 1;
					goto destroy_exit;
				default:
					printf("Valor invalido tente novamente.\n");
					sleep(1);
					continue;
			}
			
			sleep(1);

			//Envio de mensagem para solicitar a conversa.
			pthread_mutex_lock(&resposta_convite_mutex);
			resposta_convite_global = 1;
			//libera_print_message_global = 1;
			
			for (i = 0; i < TRIES_UNTIL_DESCONNECT && resposta_convite_global == 1; i++)
			{
				printf("Aguardando resposta do %s\n", TOPIC);
				pthread_mutex_lock(&control_message_mutex);
				payload_message(" ",1);// 1 = Mensagem de convite.
				control_message_global = 1;
				pthread_mutex_unlock(&control_message_mutex);
				while (control_message_global == 1);
				sleep(10);
			}
			
			if ( resposta_convite_global == 1)
			{
				//flag_pub_global = 0;
				//pthread_mutex_lock(&resposta_convite_mutex);
				resposta_convite_global = 0;
				pthread_mutex_unlock(&resposta_convite_mutex);
				printf("\nSem resposta do %s \n", TOPIC);
			}

			//Respondeu sim.
			if (resposta_convite_global == 2)
			{
				printf("%s aceitou a conversa\n", TOPIC);
				printf("Para sair digite --Sair\n\n");
				
				strcpy(TOPIC, TOPIC_CHAT);
				pthread_mutex_lock(&subscribe_topic_mutex);
				subscribe_topic_global = 1;
				while (subscribe_topic_global == 1);
				libera_print_message_global = 1;
				do
				{
					setbuf(stdin, 0);
					fgets(buffer_, sizeof(buffer_), stdin);
					
					pthread_mutex_lock(&control_message_mutex);
	
					//Tratamento de mensagem para sair do chat.
					if (strcmp(buffer_, "--Sair\n") == 0)
					{
						//Manda a mensagem dizendo q saiu do Chat.
						payload_message(" ",2);
						control_message_global = 1;
						while (control_message_global == 1);
						
						//Sai do topico da conversa.
						pthread_mutex_lock(&unsubiscribe_topic_mutex);
						unsubiscribe_topic_global = 1;
						while (unsubiscribe_topic_global == 1);
						
						resposta_convite_global = 0;
						pthread_mutex_unlock(&resposta_convite_mutex);
						break;
					}

					payload_message(buffer_, 3);// 3 = Mensagem do CHAT.
					control_message_global = 1;
					while (control_message_global == 1);

				} while (!finished);
				
				continue; //Tem que sair desse while e ir para o chat.
				//Assina o novo topico = junção dos dois usuários.
			}
			//Respondeu Não
			if (resposta_convite_global == 3)
			{
				printf("\n%s não aceitou a conversa\n", TOPIC);
				printf("\nEscolha outra opção\n");
				
				//Sai do topico da conversa.
				pthread_mutex_lock(&unsubiscribe_topic_mutex);
				unsubiscribe_topic_global = 1;
				while (unsubiscribe_topic_global == 1);
				resposta_convite_global = 0;
				pthread_mutex_unlock(&resposta_convite_mutex);
				sleep(1);		
				continue;
			}
		} 
		
		//Opção 2 conversa em grupo.
		if (option_chat == 2)
		{
			printf("\n");
			printf("O CHAT com todos os usuários online foi iniciado:\n");
			printf("Para sair digite --Sair\n\n");
			
			strcpy(TOPIC, "G_Control");
			
			pthread_mutex_lock(&subscribe_topic_mutex);
			subscribe_topic_global = 1;
			while (subscribe_topic_global == 1);
			libera_print_message_global = 1;
			sleep(1);
			
			do
			{
				setbuf(stdin, 0);
				fgets(buffer_, sizeof(buffer_), stdin);
					
				pthread_mutex_lock(&control_message_mutex);
	
				//Tratamento de mensagem para sair do chat.
				if (strcmp(buffer_, "--Sair\n") == 0)
				{
					//Manda a mensagem dizendo q saiu do Chat.
					payload_message(" ",2);
					control_message_global = 1;
					//pthread_mutex_unlock(&control_message_mutex);
					while (control_message_global == 1);
					
					//Sai do topico da conversa.
					pthread_mutex_lock(&unsubiscribe_topic_mutex);
					unsubiscribe_topic_global = 1;
					//pthread_mutex_unlock(&unsubiscribe_topic_mutex);
					while (unsubiscribe_topic_global == 1);
					break;
				}

				payload_message(buffer_, 3);// 3 = Mensagem do CHAT.
				control_message_global = 1;
				//pthread_mutex_unlock(&control_message_mutex);
				while (control_message_global == 1);

			} while (!finished);
				
			continue; //Tem que sair desse while e ir para o chat.
		} 
		
		//Opção 3 esperar ser chamado o chat.
		if (option_chat == 3)
		{
			printf("Aguardando convite para o CHAT:\n");
			
			pthread_mutex_lock(&espera_convite_mutex);
			espera_convite_global = 1;
			
			printf("Para sair digite --Sair\n\n");
			
			do
			{
				setbuf(stdin, 0);
				fgets(buffer_, sizeof(buffer_), stdin);
				
				//Tratamento de mensagem para sair do chat.
				if (strcmp(buffer_, "--Sair\n") == 0)
				{
					espera_convite_global = 0;
					pthread_mutex_unlock(&espera_convite_mutex);
					break;
				}
				//Convite aceito.
				if (strcmp(buffer_, "-s\n") == 0 && espera_convite_global == 1)
				{
					//Manda a mensagem dizendo q entrou no Chat.
					pthread_mutex_lock(&control_message_mutex);
					payload_message(" ",4);
					strcpy(TOPIC, TOPIC_PUB);
					control_message_global = 1;
					while (control_message_global == 1);

					pthread_mutex_lock(&control_message_mutex);
					payload_message(" ",4);
					strcpy(TOPIC, TOPIC_PUB);
					control_message_global = 1;
					while (control_message_global == 1);
					
					strcpy(TOPIC, TOPIC_CHAT);
					sleep(1);
					//Sai do topico da conversa.
					pthread_mutex_lock(&subscribe_topic_mutex);
					subscribe_topic_global = 1;
					while (subscribe_topic_global == 1);
					espera_convite_global = 0;
					pthread_mutex_unlock(&espera_convite_mutex);
					sleep(4);
					printf("Conversa iniciada :\n");
					do
					{	
						libera_print_message_global = 1;
						setbuf(stdin, 0);
						fgets(buffer_, sizeof(buffer_), stdin);
						pthread_mutex_lock(&control_message_mutex);
						
						//Tratamento de mensagem para sair do chat.
						if (strcmp(buffer_, "--Sair\n") == 0)
						{
							//Manda a mensagem dizendo q saiu do Chat.
							payload_message(" ",2);
							control_message_global = 1;
							while (control_message_global == 1);
					
							//Sai do topico da conversa.
							pthread_mutex_lock(&unsubiscribe_topic_mutex);
							unsubiscribe_topic_global = 1;
							//pthread_mutex_unlock(&unsubiscribe_topic_mutex);
							while (unsubiscribe_topic_global == 1);
							
							espera_convite_global = 0;
							pthread_mutex_unlock(&espera_convite_mutex);
							break;
						}
					
						payload_message(buffer_, 3);
						control_message_global = 1;
						//pthread_mutex_unlock(&control_message_mutex);
						while (control_message_global == 1);
					
					} while (!finished);
					
					break;
				}

				//Resposta não
				if (strcmp(buffer_, "-n\n") == 0 && espera_convite_global == 1)
				{
					//Manda a mensagem dizendo q saiu do Chat.
					pthread_mutex_lock(&control_message_mutex);
					payload_message(" ",5);
					strcpy(TOPIC, TOPIC_PUB);
					control_message_global = 1;
					//pthread_mutex_unlock(&control_message_mutex);
					while (control_message_global == 1);
					
					//Manda a mensagem dizendo q saiu do Chat.
					pthread_mutex_lock(&control_message_mutex);
					payload_message(" ",5);
					strcpy(TOPIC, TOPIC_PUB);
					control_message_global = 1;
					while (control_message_global == 1);
					
					espera_convite_global = 0;
					pthread_mutex_unlock(&espera_convite_mutex);
					break;
				}

			} while (!finished);
			espera_convite_global = 0;
			pthread_mutex_unlock(&espera_convite_mutex);
			
		}

		//Opção 5 esperar ser chamado o chat.
		if (option_chat == 5)
		{
			exit_ = 0;
		}
		
	}

destroy_exit:
	finished = 1;
	printf("Menu foi fechado\n");
	pthread_exit(NULL);
}

//Inicia as Threads da aplicação.
void start_chat() 
{
	pthread_t threads[3];
	
	pthread_create(&(threads[0]), NULL, mqtt_control, NULL);
	pthread_create(&(threads[1]), NULL, subscribe_thread, NULL);
	pthread_create(&(threads[2]), NULL, menu_thread, NULL);
	pthread_join(threads[2], NULL);
	pthread_join(threads[1], NULL);
	pthread_join(threads[0], NULL);
}
