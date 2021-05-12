#include "fun.h"

void menu ()
{
    int option_user;
	//int option_topic;
	printf("Escolha uma das opções a baixo para ser seu Usuário no CHAT :\n");
	printf("Digite 1 para ser o user_01 :\n");
	printf("Digite 2 para ser o user_01 :\n");
	printf("Digite 3 para ser o user_01 :\n");
	printf("Digite 4 para ser o user_01 :\n");

    scanf("%d", &option_user);
	switch(option_user)
	{
		case 1:
			strcpy(CLIENTID, "U1_Control");
			printf("oii eu sou o 1 \n");
			break;
		case 2:
			strcpy(CLIENTID, "U2_Control");
			break;
		case 3:
			strcpy(CLIENTID, "U3_Control");
			break;
		case 4:
			strcpy(CLIENTID, "U4_Control");
			break;
	}

	strcpy(TOPIC, "U4_Control");

    printf("%s \n", CLIENTID);
}

//Recebe um 'packet' e encaminha ao roteador na tabela de roteamento que possua o menor custo.
void send_message(packet* pck, router* link)
{
	int ack_received = 0;
	
	//Caso seja mensagem do tipo dados, e o router seja inacesivel
	if(pck->type == TYPE_DATA){// && (
			//dv_table_.distance[LOCAL_ROUTER.id][link->id].allocated == 0 ||
			//dv_table_.distance[LOCAL_ROUTER.id][link->id].hop == INFINITE)) {
		pthread_mutex_lock(&printf_mutex);
		printf("\n Roteador inalcançável %d.\n", link->id);
		pthread_mutex_unlock(&printf_mutex);
		return;
	}
	 
	//Se a entrega não deu certo, assume-se que o enlace caiu.
	//Roteador precisa atualizar o vetor-distância.
	if (!ack_received) {
		pthread_mutex_lock(&printf_mutex);
		printf("-\nRoteador %d inalcançável após %d tentativas com estouro de timeout.\n", link->id, TRIES_UNTIL_DESCONNECT);
		printf("-\n");
		pthread_mutex_unlock(&printf_mutex);
	}
}

//Função a ser executada na thread que recebe as mensagens, identifica-as e toma uma decisão.
void* sub (void *data)
{
	int qt_routers;
	
	packet *pck;
	//router *link;

	while(1) {
		pck = (packet *)malloc(sizeof(packet));
		fflush(stdout);

		switch (pck->type) {
			case TYPE_DATA: // mensagem de dados, original, a ser imprimida ou repassada.
				
				if (pck->id_dst == LOCAL_ROUTER.id) {
					//Se este roteador for o destino, imprime a msg
					pthread_mutex_lock(&printf_mutex);
					printf("-\n\nMensagem de %d:                                 \n", pck->id_src);
					printf("%s\n\n-\n", pck->data);
					pthread_mutex_unlock(&printf_mutex);
				} else {
					//se já estourou a quantidade de saltos máxima, para de encaminhar.
					qt_routers = 0;
					if (pck->jump > qt_routers) break;
					(pck->jump)++;
					//Se o roteador não é o destino, encaminha ao próximo roteador.
					pthread_mutex_lock(&printf_mutex);
					printf("-\nRoteador %d encaminhando mensagem de %d com # sequência %d para o destino %d.\n-\n", LOCAL_ROUTER.id, pck->id_src, pck->seq_number, pck->id_dst);
					pthread_mutex_unlock(&printf_mutex);
					//Verifica qual é o vizinho que leva ao destino da mensagem
					
					//Manda para o vizinho
					//send_message(pck, link);
				}
		}
		
		free(pck);
	}

	pthread_exit(NULL);
}

//Construtor do tipo 'packet'.
packet create_packet(int id_src, int id_dst, int type, void* message)
{
	packet pck;
	pck.id_src = id_src;
	pck.id_dst = id_dst;
	pck.type = type;
	pck.seq_number = SEQ_NUMBER++;
	pck.jump = 0;
	strcpy(pck.data, message);
	return pck;
}

//Função a ser executada na thread que lê e envia as mensagens da entrada padrão.
void* pub (void *data)
{
	int id_dst;
	char message[MSGLEN];
	//router* next_link;
	
	// loop infinito, sempre espera por novos envios
	while(1) {
		scanf("%d", &id_dst);
		getchar();
		if (id_dst == LOCAL_ROUTER.id) {
			pthread_mutex_lock(&printf_mutex);
			printf("-\n ! O roteador %d é este !\n-\n", id_dst);
			pthread_mutex_unlock(&printf_mutex);
			continue;
		}
		
		pthread_mutex_lock(&printf_mutex);
		printf("Escreva a mensagem (limite 100 caracteres): ");
		pthread_mutex_unlock(&printf_mutex);
		fflush(stdout);
		fflush(stdin);
		fgets(message, MSGLEN, stdin);
		message[strlen(message)-1] = '\0';
		
		//cria o pacote, encontra o vizinho que leva ao destino e envia a mensagem
		//packet pck = create_packet(LOCAL_ROUTER.id, id_dst, TYPE_DATA, message);
		//next_link = &routers[vizinho_index];
		//send_message(&pck, next_link);
	}
	pthread_exit(NULL);
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

void onDisconnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Disconnect failed\n");
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

void onSend(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	//int rc;

	printf("Message with token value %d delivery confirmed\n", response->token);
	opts.onSuccess = onDisconnect;
	opts.onFailure = onDisconnectFailure;
	opts.context = client;
	/*if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start disconnect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}*/
}

void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Connect failed, rc %d\n", response ? response->code : 0);
	finished = 1;
}

void onConnect(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;

	printf("Successful connection\n");
	opts.onSuccess = onSend;
	opts.onFailure = onSendFailure;
	opts.context = client;
	pubmsg.payload = PAYLOAD;
	pubmsg.payloadlen = (int)strlen(PAYLOAD);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	if ((rc = MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start sendMessage, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
}

int messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* m)
{
	//not expecting any messages
	return 1;
}
