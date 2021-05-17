#include "fun.h"

//Recebe um 'packet' e encaminha ao roteador na tabela de roteamento que possua o menor custo.
void send_message(MQTTAsync client, MQTTAsync_responseOptions opts, MQTTAsync_message pubmsg, int type_msg)
{   
	char buffer_[MAX_MESSAGE];
	char message[MAX_NAME_USER + MAX_MESSAGE];
	int n, rc;
	
	switch(type_msg){
		//Mensagem de convite
		case 1:
			strcat(message, "--Solicitação de CHAT do " );
			strcat(message, CLIENTID );
			strcat(message, " --s = sim / --n = não.\n" );
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
		//Valor do tipo de mensagem errado.
		default:
			printf("Erro, tipo de mensagem invalido\n");
			return;
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

void convite_chat(MQTTAsync client, MQTTAsync_connectOptions conn_opts)
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
void* sub (void *data)
{
	/*int qt_routers;
	
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
	*/
	pthread_exit(NULL);
}

//Função a ser executada na thread que lê e envia as mensagens da entrada padrão.
void* pub (void *data)
{
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
int messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* m)
{
	char message [MAX_MESSAGE + MAX_NAME_USER];
	char message_s [MAX_MESSAGE + MAX_NAME_USER];
	char message_n [MAX_MESSAGE + MAX_NAME_USER];
	char message_convite [MAX_MESSAGE + MAX_NAME_USER];

	strcpy(message, (char*)m->payload);
	strcat(message_s, TOPIC);
	strcat(message_s, " => --s");
	strcat(message_n, TOPIC);
	strcat(message_n, " => --n");

	message_convite[0] = message[0];
	message_convite[1] = message[1];
	message_convite[2] = message[2];
	message_convite[3] = message[3];

	//printf("recebi uma mensagem\n");
	
	if ( !strcmp(message, message_s) && espera_convite == 1)
	{
		printf("respondeu sim\n");
		pthread_mutex_lock(&espera_convite_mutex);
		espera_convite	= 2;
		pthread_mutex_unlock(&espera_convite_mutex);
	} else if ( !strcmp(message, message_n) && espera_convite == 1) {
		printf("respondeu não\n");
		pthread_mutex_lock(&espera_convite_mutex);
		espera_convite	= 3;
		pthread_mutex_unlock(&espera_convite_mutex);
	} else if (!strcmp(message_convite, "--So") && resposta_convite == 1 )
	{
		//strcpy(message_convite, " ");
		//message_convite[0] = message[27];
		//message_convite[1] = message[28];
		//strcat(TOPIC,message_convite);
		//strcat(TOPIC,"_Control");
		printf("recebi o convite\n");
		pthread_mutex_lock(&resposta_convite_mutex);
		resposta_convite = 0;
		pthread_mutex_unlock(&resposta_convite_mutex);
	}

	pthread_mutex_lock(&printf_mutex);	
	

	if (flag_local_pub == 0)
	{
		//printf("%s, topc name = %s",(char*)m->payload, (char*) topicName);
		printf("recebido = %s",message);
	}
	flag_local_pub = 0;
	MQTTAsync_freeMessage(&m);
    MQTTAsync_free(topicName);
	pthread_mutex_unlock(&printf_mutex);
	return 1;
}
