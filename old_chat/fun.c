#include "fun.h"

//Recebe um 'packet' e encaminha ao roteador na tabela de roteamento que possua o menor custo.
void send_message(MQTTAsync client, MQTTAsync_responseOptions opts, MQTTAsync_message pubmsg, int type_msg)
{   
	char buffer_[MAX_MESSAGE];
	char message[MAX_NAME_USER + MAX_MESSAGE] ;//= "oi user control do caralho";
	int n, rc;
	
	switch(type_msg){
		//Mensagem de convite
		case 1:
			
			strcat(message, "--Solicitacao " );
			strcat(message, CLIENTID );
			strcat(message, " deseja trocar mensagens ? --s = sim / --n = não.\n" );
			//printf("valor mensagem = %s\n", message);
			
			//strcat(message, "é um convite" );
			break;
		//Mensagem avisando que está saindo do CHAT.
		case 2:
			strcat(message, CLIENTID );
			strcat(message, " Saiu do CHAT" );
			break;
		//Mensagem de conversação do CHAT.
		case 3:
			setbuf(stdin, 0);
			fgets(buffer_, sizeof(buffer_), stdin);
    		strcat(message, CLIENTID );
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
	opts.onSuccess = onSend;
	opts.onFailure = onSendFailure;
	opts.context = client;
	pubmsg.payload = message;
	pubmsg.payloadlen = n;
	pubmsg.qos = QOS;
	pubmsg.retained = 0;

	if ((rc = MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start sendMessage, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	flag_local_pub = 1;
    
}


void conect_b(MQTTAsync client, MQTTAsync_connectOptions conn_opts)
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
}


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

//Função a ser executada na thread que recebe as mensagens, identifica-as e toma uma decisão.
void* sub ()
{
	char message [MAX_MESSAGE + MAX_NAME_USER];
	char message_s [MAX_MESSAGE + MAX_NAME_USER];
	char message_n [MAX_MESSAGE + MAX_NAME_USER];
	char message_convite [MAX_MESSAGE + MAX_NAME_USER] = "--Solicitacao U1_Control deseja trocar mensagens ? --s = sim / --n = não.";
	char *token;

	while (1)
	{
		if (new_message == 1)
		{	
			pthread_mutex_lock(&new_message_mutex);
			strcpy(message, message_global);
			new_message	= 0;
			pthread_mutex_unlock(&new_message_mutex);

			//Aplicação esperando a resposta do convite feito a outro usuário.
			if (resposta_convite == 1)
			{
				strcat(message_s, TOPIC);
				strcat(message_s, " => --s");
				strcat(message_n, TOPIC);
				strcat(message_n, " => --n");
				//Respondeu que sim.
				if ( !strcmp(message, message_s))
				{
					printf("respondeu sim\n");//usando para debug.
					strcat(TOPIC, "-");
					strcat(TOPIC, CLIENTID);
					pthread_mutex_lock(&resposta_convite_mutex);
					resposta_convite = 2;
					pthread_mutex_unlock(&resposta_convite_mutex);
				//Respondeu que não.
				} else if ( !strcmp(message, message_n)) {
					printf("respondeu não\n");//usando para debug.
					pthread_mutex_lock(&resposta_convite_mutex);
					resposta_convite = 3;
					// cancela a assinatura do tópico.
					// para de publicar no tópico.
					// volta para o inicio do menu.
					pthread_mutex_unlock(&resposta_convite_mutex);
				}
			}

			if (espera_convite == 1)
			{
				token = strtok(message, "\n");
				//strcpy(message_convite, token);
			//	printf("oi mundo mult thread\n");
			//	strcat(message, "--Solicitacao " );
			//strcat(message, CLIENTID );
			//strcat(message, "\n" );
				
				printf("eu sou o message =%s", token);//usando para debug.
				//message_convite = "--Solicitacao U1_Control deseja trocar mensagens ? --s = sim / --n = não.";
				//--Solicitacao U1_Control deseja trocar mensagens ? --s = sim / --n = não.
				if (strcmp(token, "--Solicitacao U1_Control deseja trocar mensagens ? --s = sim / --n = não.") == 0) {
					printf("sou uma solicitacao\n");//usando para debug.
					//strcpy(message_convite, " ");
					//send_message()
					//strcat(TOPIC,message_convite);
					//strcat(TOPIC,"_Control");
					//token = strtok(NULL, message);
					//strcpy (TOPIC_AUX, token);
					/*while( token != NULL ) {
     					printf( " %s\n", token);
      					token = strtok(NULL, message);
					}*/
					pthread_mutex_lock(&resposta_convite_mutex);
					espera_convite = 0;
					pthread_mutex_unlock(&resposta_convite_mutex);
				}
			}
		}
	}
	
	/*if (flag_local_pub == 0)
	{

		else {
			pthread_mutex_lock(&printf_mutex);	
			//printf("%s, topc name = %s",(char*)m->payload, (char*) topicName);
			printf("recebido = %s",message);
			pthread_mutex_unlock(&printf_mutex);
		}
	}*/

	flag_local_pub = 0;
	pthread_exit(NULL);
}

//Função a ser executada na thread que lê e envia as mensagens da entrada padrão.
void* pub (void *data)
{

	MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc, i;
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
	if ((rc = MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to create client object, return code %d\n", rc);
		exit(EXIT_FAILURE);
		MQTTAsync_destroy(&client);
		pthread_exit(NULL);
 		//return;
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
			
			i = 0;
			while ( i < TRIES_UNTIL_DESCONNECT && resposta_convite == 1)
			{
				send_message(client, opts, pubmsg, 1);	
				i++;
				sleep(20);
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
				/* code */
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

	pthread_exit(NULL);
}

void disconnect (void * context)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	//printf("Message with token value %d delivery confirmed\n", response->token);
	opts.onSuccess = onDisconnect;
	opts.onFailure = onDisconnectFailure;
	opts.context = client;
	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start disconnect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
}

//Após as configurações e carregamentos iniciais, inicia as funcionalidades do roteador.
//Cria 3 threads, uma para receber, outra para enviar e outra para atualizar os vetores-distância.
void start_chat() 
{
	pthread_t threads[2];
	
	printf("\n##############\n");
	printf("DIGITE O ID DO DISPOSITIVO AO QUAL DESEJA INICIAR A CONVERSA\n");
	printf("##############\n\n");
	
	pthread_create(&(threads[0]), NULL, sub, NULL);
	pthread_create(&(threads[1]), NULL, pub, NULL);
	pthread_join(threads[1], NULL);
	pthread_join(threads[0], NULL);
}



void connlost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);

	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
 		finished = 1;
	}
}
//Incrementada com código do sub
void onDisconnectFailure(void* context, MQTTAsync_failureData* response)
{
	//printf("Disconnect failed\n");
	printf("Disconnect failed, rc %d\n", response->code);
	finished = 1;
}

void onDisconnect(void* context, MQTTAsync_successData* response)
{
	printf("Successful disconnection\n");
	finished = 1;
}

void onSendFailure(void* context, MQTTAsync_failureData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	printf("Message send failed token %d error code %d\n", response->token, response->code);
	opts.onSuccess = onDisconnect;
	opts.onFailure = onDisconnectFailure;
	opts.context = client;
	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start disconnect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
}

//Mensagem enviada.
void onSend(void* context, MQTTAsync_successData* response)
{
	printf("-\n");
	/*MQTTAsync client = (MQTTAsync)context;
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
	}*/
}

//Assinatura do topico funcionou.
void onSubscribe(void* context, MQTTAsync_successData* response)
{
	//printf("Subscribe succeeded\n");
	subscribed = 1;
}
//Assinatura do topico falhou.
void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Subscribe failed, rc %d\n", response->code);
	finished = 1;
}
//Conexão falhou.
void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Connect failed, rc %d\n", response ? response->code : 0);
	printf("Connect failed, _teste rc %d\n", response->code);
	finished = 1;
}
//Se conecta ao topico.
//Efetua a assinatura do topico.
void onConnect(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	//MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	//printf("Successful connection\n");

	//Assinatura do Proprio Tópico.
	opts.onSuccess = onSubscribe;
	opts.onFailure = onSubscribeFailure;
	opts.context = client;
	//pubmsg.qos = QOS;
	//pubmsg.retained = 0;

	if ((rc = MQTTAsync_subscribe(client, TOPIC, QOS, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start subscribe, return code %d\n", rc);
		finished = 1;
	}
}

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
	
	//return 1;*/

	printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);
	printf("   message 2: %s\n", message_global);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}
