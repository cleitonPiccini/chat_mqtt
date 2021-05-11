#define MAX_ROUTERS 100 //número maximo de roteadores
#define MAX_LINE 300
#define INFINITE 20 

#define MSGLEN 100 //tamanho maximo da mensagem
#define BUFLEN sizeof(packet)

#define TYPE_DATA 1 //identificador de mensagem contendos dados
#define TYPE_DV 2 //identificador de mensagem contendo o vetor distancia

#define TRIES_UNTIL_DESCONNECT 3
#define TIMEOUT 2

#define DV_RESEND_INTERVAL 10 //tempo de espera para mandar o vetor distancia.

#define PRINT_TABLE 1
#define PRINT_AND_SEND_TABLE 2 

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
	distance_vector dv[MAX_ROUTERS]; //seu vetor distância
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

packet create_packet(int id_src, int id_dst, int type, void* message);
void* sub (void *data);
void send_message(packet* pck, router* link);
void* pub (void *data);
void start_chat();

//necessário mutex para proteger escrita concorrente na dv_table_
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t printf_mutex = PTHREAD_MUTEX_INITIALIZER;
router LOCAL_ROUTER;
unsigned int SEQ_NUMBER;
router * routers; 	//enlaces deste roteador
int qt_links;	 			//routers.length()
//dv_table dv_table_;			//tabela vetor-distância deste roteador
