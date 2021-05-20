//Biblioteca

#define TRIES_UNTIL_DESCONNECT 3
#define ADDRESS     "tcp://localhost:1883"
#define QOS         1
#define TIMEOUT     10000L
#define MAX_NAME_USER 100
#define MAX_MESSAGE 400


void* pub (void *data);
void start_chat();

void connlost(void *context, char *cause);
//void onDisconnectFailure(void* context, MQTTAsync_failureData* response);
//void onDisconnect(void* context, MQTTAsync_successData* response);
//void onSendFailure(void* context, MQTTAsync_failureData* response);
//void onSend(void* context, MQTTAsync_successData* response);
void delivered(void *context, MQTTClient_deliveryToken dt);
//void onConnectFailure(void* context, MQTTAsync_failureData* response);
//void onConnect(void* context, MQTTAsync_successData* response);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void disconnect (void * context);

//necessário mutex para proteger escrita concorrente na dv_table_
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t espera_convite_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resposta_convite_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t new_message_mutex = PTHREAD_MUTEX_INITIALIZER;

int disc_finished = 0;
int subscribed = 0;
int finished=0;
int libera_print_message_global = 0;

char TOPIC [MAX_NAME_USER];
char TOPIC_CHAT [MAX_NAME_USER];
char TOPIC_PUB [MAX_NAME_USER];

char CLIENTID [MAX_NAME_USER];
char PAYLOAD [MAX_MESSAGE + MAX_NAME_USER];

pthread_mutex_t payload_mutex = PTHREAD_MUTEX_INITIALIZER;

int ocupado_global = 0;

int espera_convite_global = 0; 
int resposta_convite_global = 0; //0 = não convidou, 1 = convidou, 2 = sim, 3 = não.
int exit_total = 1;

char send_message_global [MAX_MESSAGE + MAX_NAME_USER];
int new_message_global = 0;

//int send_message_global = 0;

volatile MQTTClient_deliveryToken deliveredtoken;

int create_client_global = 0;//Controla a criação do client
//int conect_broker_global = 0;//Controla a conexão ao broker
int subscribe_topic_global = 0;//Controla a assinatura nos tópicos
int unsubiscribe_topic_global = 0;//Controla o fim da assinatura nos tópicos.
int control_message_global = 0;//Controla o envio de mensagens (publicações).
int flag_pub_global = 0;//Flag para evitar o "eco" de mensagens quando a aplicação publica no tópico.

pthread_mutex_t create_client_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t conect_broker_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t subscribe_topic_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t control_message_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t unsubiscribe_topic_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flag_pub_mutex = PTHREAD_MUTEX_INITIALIZER;
