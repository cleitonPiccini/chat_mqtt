#include "fun.h"


//Recebe um 'packet' e encaminha ao roteador na tabela de roteamento que possua o menor custo.
void send_message(MQTTClient client, MQTTClient_deliveryToken token, char topico[MAX_NAME_USER], int type_msg)
{   
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
    
	
	char buffer_[MAX_MESSAGE];
	char message[MAX_NAME_USER + MAX_MESSAGE] ;//= "oi user control do caralho";
	int n, rc;
	
	switch(type_msg){
		//Mensagem de convite
		case 1:
			
			strcat(message, "--Solicitacao " );
			strcat(message, TOPIC );
			strcat(message, " deseja trocar mensagens ? --s = sim / --n = não.\n" );
			//printf("valor mensagem = %s\n", message);
			
			//strcat(message, "é um convite" );
			break;
		//Mensagem avisando que está saindo do CHAT.
		case 2:
			strcat(message, TOPIC );
			strcat(message, " Saiu do CHAT" );
			break;
		//Mensagem de conversação do CHAT.
		case 3:
			setbuf(stdin, 0);
			fgets(buffer_, sizeof(buffer_), stdin);
    		strcat(message, TOPIC );
			strcat(message, " => ");
			strcat(message, buffer_);
			break;
		//Mensagem de sim para solicitação no chat.
		case 4:
			strcpy(message, "--s" );
			break;
		//Mensagem de não para solicitação no chat.
		case 5:
			strcpy(message, "--n" );
			break;
		//Valor do tipo de mensagem errado.
		default:
			printf("Erro, tipo de mensagem invalido\n");
			//return;
	}

	//printf("oi mensagem = %s\n", message);
	n = (int) strlen(message);
	pubmsg.payload = message;
    pubmsg.payloadlen = n;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    
    if ((rc = MQTTClient_publishMessage(client, topico, &pubmsg, &token)) != MQTTCLIENT_SUCCESS)
    {
         printf("Failed to publish message, return code %d\n", rc);
         exit(EXIT_FAILURE);
    }

	flag_local_pub = 1;
    
}


void client_conect(MQTTClient client, MQTTClient_connectOptions conn_opts)
{
	int rc;

	conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        rc = EXIT_FAILURE;
        finished = 1;
		//goto destroy_exit;
    }
	sleep(2);
}

//Inicia a assinatura de um tópico.
void subscribe_topic (MQTTClient client, char topico[MAX_NAME_USER])
{
	int rc;
	// Efetua a assinatura do tópico.
	if ((rc = MQTTClient_subscribe(client, topico, QOS)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to subscribe, return code %d\n", rc);
    	rc = EXIT_FAILURE;
    }
}

//Cancela a assinatura de um tópico.
void unsubscribe_topic (MQTTClient client, char topico[MAX_NAME_USER])
{
	int rc;

	if ((rc = MQTTClient_unsubscribe(client, topico)) != MQTTCLIENT_SUCCESS)
    {
    	printf("Failed to unsubscribe, return code %d\n", rc);
        rc = EXIT_FAILURE;
    }
}

/*
void sub_topic(MQTTAsync client, MQTTAsync_connectOptions conn_opts)
{
	int rc;

	if ((rc = MQTTAsync_setCallbacks(client, NULL, connlost, messageArrived, NULL)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to set callback, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	sleep(2);
}
*/

/*
void pub_topic(MQTTAsync client, MQTTAsync_connectOptions conn_opts)
{
	int rc;

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
	sleep(2);
	if ((rc = MQTTAsync_setCallbacks(client, NULL, connlost, messageArrived, NULL)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to set callback, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
}
*/


//Função a ser executada na thread que recebe as mensagens, identifica-as e toma uma decisão.
void* subscribe_thread ()
{
	char message [MAX_MESSAGE + MAX_NAME_USER];
	char message_s [MAX_MESSAGE + MAX_NAME_USER];
	char message_n [MAX_MESSAGE + MAX_NAME_USER];
	//char message_convite [MAX_MESSAGE + MAX_NAME_USER] = "--Solicitacao U1_Control deseja trocar mensagens ? --s = sim / --n = não.";
	char *token;
	

	
	while (1)
	{
		if (new_message_global == 1)
		{	
			pthread_mutex_lock(&new_message_mutex);
			strcpy(message, send_message_global);
			new_message_global	= 0;
			pthread_mutex_unlock(&new_message_mutex);

			//Aplicação esperando a resposta do convite feito a outro usuário.
			if (resposta_convite_global == 1)
			{
				strcat(message_s, TOPIC_CHAT);
				strcat(message_s, " => --s");
				strcat(message_n, TOPIC_CHAT);
				strcat(message_n, " => --n");
				//Respondeu que sim.
				if ( strcmp(message, message_s) == 0 )
				{
					printf("respondeu sim\n");//usando para debug.
					pthread_mutex_lock(&resposta_convite_mutex);
					strcat(TOPIC_CHAT, "-");
					strcat(TOPIC_CHAT, TOPIC);
					resposta_convite_global = 2;
					pthread_mutex_unlock(&resposta_convite_mutex);
				//Respondeu que não.
				} else if ( strcmp(message, message_n) == 0) {
					printf("respondeu não\n");//usando para debug.
					pthread_mutex_lock(&resposta_convite_mutex);
					resposta_convite_global = 3;
					pthread_mutex_unlock(&resposta_convite_mutex);
				}
			}

			//Aplicação esperando ser chada.
			if (espera_convite_global == 1)
			{
				token = strtok(message, "\n");
				token = strtok(token, " ");
				//strcpy(message_convite, token);
			//	printf("oi mundo mult thread\n");
			//	strcat(message, "--Solicitacao " ); 
			//strcat(message, CLIENTID );
			//strcat(message, "\n" );
				
				printf("eu sou o message =%s", token);//usando para debug.
				//message_convite = "--Solicitacao U1_Control deseja trocar mensagens ? --s = sim / --n = não.";
				//--Solicitacao U1_Control deseja trocar mensagens ? --s = sim / --n = não.
				if (strcmp(token, "--Solicitacao") == 0) {
					printf("sou uma solicitacao\n");//usando para debug.
					//strcpy(message_convite, " ");
					//send_message()
					//strcat(TOPIC,message_convite);
					//strcat(TOPIC,"_Control");
					pthread_mutex_lock(&espera_convite_mutex);
					token = strtok(NULL, " ");
					strcpy (TOPIC_CHAT, token);
					espera_convite_global = 0;
					pthread_mutex_unlock(&espera_convite_mutex);
				}
			}
		}
	}
	


flag_local_pub = 1;
pthread_exit(NULL);
}



void delivered(void *context, MQTTClient_deliveryToken dt)
{
    //printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	if (flag_local_pub == 0)
	{
		pthread_mutex_lock(&new_message_mutex);
		strcpy(send_message_global, message->payload);
		new_message_global	= 1;
		pthread_mutex_unlock(&new_message_mutex);
    
		pthread_mutex_lock(&printf_mutex);
    	//printf("     topic: %s\n", topicName);
    	printf("message: %.*s\n", message->payloadlen, (char*)message->payload);
    	pthread_mutex_unlock(&printf_mutex);
	}
	flag_local_pub = 0;
	MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}



//Função a ser executada na thread que lê e envia as mensagens da entrada padrão.
void* menu_thread (void *data)
{

	MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    //MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    
	int i;
	int rc;
	int option_user, option_chat, option_topic;
	int exit_ = 1;
	char buffer_ [MAX_NAME_USER + MAX_MESSAGE];
	
	
	//pthread_join(threads[0], NULL);
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
	if ((rc = MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to create client, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto exit;
    }
	if ((rc = MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to set callbacks, return code %d\n", rc);
        rc = EXIT_FAILURE;
        goto destroy_exit;
    }
	strcpy(TOPIC, CLIENTID);
	client_conect(client, conn_opts);
	subscribe_topic(client, TOPIC);

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
					strcpy(TOPIC_CHAT, "U1_Control");
					exit_ = 0;
					break;
				case 2:
					strcpy(TOPIC_CHAT, "U2_Control");
					exit_ = 0;
					break;
				case 3:
					strcpy(TOPIC_CHAT, "U3_Control");
					exit_ = 0;
					break;
				case 4:
					strcpy(TOPIC_CHAT, "U4_Control");
					exit_ = 0;
					break;
				default:
					printf("Valor invalido tente novamente.\n");
					sleep(2);
					continue;
			}
			
			sleep(1);
			pthread_mutex_lock(&resposta_convite_mutex);
			resposta_convite_global = 1;
			subscribe_topic(client, TOPIC_CHAT);
			pthread_mutex_unlock(&resposta_convite_mutex);
			
			printf("Aguardado resposta do %s", TOPIC_CHAT);
			
			i = 0;
			for (i = 0; i < TRIES_UNTIL_DESCONNECT && resposta_convite_global == 1; i++)
			{
				send_message(client, token, TOPIC_CHAT, 1);
				sleep(10);
			}
			unsubscribe_topic(client, TOPIC_CHAT);
			sleep(1);
			//Respondeu sim.
			if (resposta_convite_global == 2)
			{
				subscribe_topic(client, TOPIC_CHAT);
				sleep(2);
				while (!finished) 
				{
					send_message(client, token, TOPIC_CHAT, 3);
				}
				continue; //Tem que sair desse while e ir para o chat.
				//Assina o novo topico = junção dos dois usuários.
			}
			if (resposta_convite_global == 3)
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
			sleep(1);
			strcpy(TOPIC_CHAT, "G_Control");
			subscribe_topic(client, TOPIC_CHAT);
			while (!finished) 
			{
				send_message(client, token, TOPIC_CHAT, 3);
			}
			continue;
		} 
		
		//Opção 3 esperar ser chamado o chat.
		if (option_chat == 3){
			printf("Aguardando convite para o CHAT:\n");
			pthread_mutex_lock(&espera_convite_mutex);
			espera_convite_global = 1;
			pthread_mutex_unlock(&espera_convite_mutex);
			
			//Aguarda ser chamado por alguém.
			
			while (espera_convite_global == 1)
			{
				
			}
			
			/*int ch;
    		do
    		{
        		ch = getchar();
    		} while (ch!='Q' && ch != 'q' && espera_convite_global == 1);
			*/
			setbuf(stdin, 0);
			fgets(buffer_, sizeof(buffer_), stdin);
    		//strcat(message, CLIENTID );
			//strcat(message, " => ");
			//strcat(message, buffer_);
			if (strcmp(buffer_, "--s"))
			{
				send_message(client, token, TOPIC, 4);
				sleep(2);
				//strcat(TOPIC, TOPIC_AUX);
				//strcat(TOPIC_AUX, TOPIC);
				//convite_chat(client, conn_opts);
				subscribe_topic(client, TOPIC_CHAT);
				sleep(1);
				while (!finished) 
				{
					send_message(client, token, TOPIC_CHAT, 3);
				}
				continue;
			}
			if (strcmp(buffer_, "--n"))
			{
				send_message(client, token, TOPIC, 5);
				sleep(2);
				//disconnect(client);
				
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

	/*rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    printf("Message with delivery token %d delivered\n", token);

    if ((rc = MQTTClient_disconnect(client, 10000)) != MQTTCLIENT_SUCCESS)
    	printf("Failed to disconnect, return code %d\n", rc);
	*/
destroy_exit:
    MQTTClient_destroy(&client);
exit:
	pthread_exit(NULL);
}


//Após as configurações e carregamentos iniciais, inicia as funcionalidades do roteador.
//Cria 3 threads, uma para receber, outra para enviar e outra para atualizar os vetores-distância.
void start_chat(MQTTClient client) 
{
	pthread_t threads[3];
	
	printf("\n##############\n");
	printf("DIGITE O ID DO DISPOSITIVO AO QUAL DESEJA INICIAR A CONVERSA\n");
	printf("##############\n\n");
	
	pthread_create(&(threads[0]), NULL, subscribe_thread, &client);
	pthread_create(&(threads[1]), NULL, subscribe_thread, &client);
	pthread_join(threads[1], NULL);
	pthread_join(threads[0], NULL);
}

/*
//Mensagem enviada.
void onSend(void* context, MQTTAsync_successData* response)
{
	printf("-\n");
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	printf("Message with token value %d delivery confirmed\n", response->token);
	opts.onSuccess = onDisconnect;
	opts.onFailure = onDisconnectFailure;
	opts.context = client;
	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start disconnect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
}
*/

/*
//Função callback - Recebe mensagem.
int messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
	pthread_mutex_lock(&new_message_mutex);
	//strcpy(message, message_global);
	strcpy(message_global, message->payload);
	new_message	= 1;
	pthread_mutex_unlock(&new_message_mutex);
	//new_message = 1;
	
	//flag_local_pub = 0;
	//printf("%s", message_global);
	//MQTTAsync_freeMessage(&m);
    //MQTTAsync_free(topicName);
	
	//return 1;

	printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);
	printf("   message 2: %s\n", message_global);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}*/
