#define MSGLEN 100 //tamanho maximo da mensagem
#define BUFLEN sizeof(packet)

#define TRIES_UNTIL_DESCONNECT 3

#define DV_RESEND_INTERVAL 10 //tempo de espera para mandar o vetor distancia.

#define ADDRESS     "tcp://localhost:1883"
//#define CLIENTID    "ExampleClientPub"
//#define TOPIC       "MQTT Examples"
//#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
#define MAX_NAME_USER 100
#define MAX_MESSAGE 400

/*
typedef struct
{
	char topico [MAX_MESSAGE];

};
*/

void menu();
void* sub ();
void send_message(MQTTAsync client, MQTTAsync_responseOptions opts, MQTTAsync_message pubmsg, int type_msg);
void convite_chat(MQTTAsync client, MQTTAsync_connectOptions conn_opts);
void* pub (void *data);
void start_chat();

void connlost(void *context, char *cause);
void onDisconnectFailure(void* context, MQTTAsync_failureData* response);
void onDisconnect(void* context, MQTTAsync_successData* response);
void onSendFailure(void* context, MQTTAsync_failureData* response);
void onSend(void* context, MQTTAsync_successData* response);
void onConnectFailure(void* context, MQTTAsync_failureData* response);
void onConnect(void* context, MQTTAsync_successData* response);
int messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* m);
void disconnect (void * context);

//necessário mutex para proteger escrita concorrente na dv_table_
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t espera_convite_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resposta_convite_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t new_message_mutex = PTHREAD_MUTEX_INITIALIZER;

int disc_finished = 0;
int subscribed = 0;
int finished;
int flag_local_pub = 0;

char TOPIC [MAX_NAME_USER];
char TOPIC_AUX [MAX_NAME_USER];
char CLIENTID [MAX_NAME_USER];

int ocupado = 0;
int online = 0;
int espera_convite = 0; 
int resposta_convite = 0; //0 = não convidou, 1 = convidou, 2 = sim, 3 = não.
int exit_total = 1;

char message_global [MAX_MESSAGE + MAX_NAME_USER];
int new_message = 0;

int send_message_global = 0;