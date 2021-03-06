//Biblioteca

#define TRIES_UNTIL_DESCONNECT 3
#define ADDRESS     "tcp://localhost:1883"
#define QOS         1
#define TIMEOUT     10000L
#define MAX_NAME_USER 100
#define MAX_MESSAGE 400

void connlost(void *context, char *cause);
void delivered(void *context, MQTTClient_deliveryToken dt);
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void payload_message( char buffer_[MAX_NAME_USER + MAX_MESSAGE], int type_msg);
void * mqtt_control ();
void* subscribe_thread ();
void* menu_thread (void *data);
void start_chat();
//
int finished = 0;
int libera_print_message_global = 0;
//
char TOPIC [MAX_NAME_USER];
char TOPIC_CHAT [MAX_NAME_USER];
char TOPIC_PUB [MAX_NAME_USER];
char CLIENTID [MAX_NAME_USER];
char PAYLOAD [MAX_MESSAGE + MAX_NAME_USER];
//
pthread_mutex_t payload_mutex = PTHREAD_MUTEX_INITIALIZER;
//
int espera_convite_global = 0; 
int resposta_convite_global = 0; //0 = não convidou, 1 = convidou, 2 = sim, 3 = não.
int new_message_global = 0;
//
pthread_mutex_t espera_convite_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resposta_convite_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t new_message_mutex = PTHREAD_MUTEX_INITIALIZER;
//
char send_message_global [MAX_MESSAGE + MAX_NAME_USER];
char send_message_topicname_global [MAX_NAME_USER];
//
int create_client_global = 0;//Controla a criação do client
int subscribe_topic_global = 0;//Controla a assinatura nos tópicos
int unsubiscribe_topic_global = 0;//Controla o fim da assinatura nos tópicos.
int control_message_global = 0;//Controla o envio de mensagens (publicações).
int flag_pub_global = 0;//Flag para evitar o "eco" de mensagens quando a aplicação publica no tópico.
//
pthread_mutex_t create_client_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t subscribe_topic_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t control_message_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t unsubiscribe_topic_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t flag_pub_mutex = PTHREAD_MUTEX_INITIALIZER;

volatile MQTTClient_deliveryToken deliveredtoken;