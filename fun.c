#include "fun.h"

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