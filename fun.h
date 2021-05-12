#define MAX_NAME_USER 100

#define MSGLEN 100 //tamanho maximo da mensagem
#define BUFLEN sizeof(packet)

#define TYPE_DATA 1 //identificador de mensagem contendos dados
#define TYPE_DV 2 //identificador de mensagem contendo o vetor distancia

#define TRIES_UNTIL_DESCONNECT 3

#define DV_RESEND_INTERVAL 10 //tempo de espera para mandar o vetor distancia.

#define PRINT_TABLE 1
#define PRINT_AND_SEND_TABLE 2 

#define ADDRESS     "tcp://localhost:1883"
//#define CLIENTID    "ExampleClientPub"
//#define TOPIC       "MQTT Examples"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

//estrutura com os dados de um enlace vizinho
typedef struct {
	int id;
	char ip[16];
	int process;
	int hop;
} router;

//uma posição da matriz/tabela de roteamento
typedef struct {
	//id_dst é obtido através do index 'j' da matriz dv_table.distance[i][j]
	int id; 	//qual enlace de saída para chegar ao id_dst
	int hop;			//custo do menor caminho até id_dst
	int allocated;		//posição do vetor distância está sendo usada
} distance_vector;

//estrutura da mensagem com o vetor distância
typedef struct {
	int id;			//roteador origem
	distance_vector dv[100]; //seu vetor distância
} dv_payload;

//estrutura da matriz/tabela de roteamento
/*typedef struct {
	distance_vector distance[MAX_ROUTERS][MAX_ROUTERS];
} dv_table;
*/
//estrutura do pacote transmitido
typedef struct {
	short type; 
	int id_src; 
	int id_dst; 
	unsigned int seq_number;
	char data[100];
	dv_payload dv;
	short jump;
} packet;

void menu();
packet create_packet(int id_src, int id_dst, int type, void* message);
void* sub (void *data);
void send_message(packet* pck, router* link);
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

//necessário mutex para proteger escrita concorrente na dv_table_
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
router LOCAL_ROUTER;
unsigned int SEQ_NUMBER;
router * routers; 	//enlaces deste roteador
int qt_links;	 			//routers.length()
//dv_table dv_table_;			//tabela vetor-distância deste roteador
int finished;

char TOPIC [MAX_NAME_USER];
char CLIENTID [MAX_NAME_USER];
