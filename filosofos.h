#define TRUE 1
#define FALSE 0

#define NB_THREADS 4

#define DURMIENDO 0
#define PENSANDO 1
#define COMIENDO 2
#define HAMBREANDO 3

#define LIBRE 0
#define OCUPADO 1


typedef struct 
{
	int id;
	pthread_mutex_t *mtx;
	int estado;
	int pensando;
	int comiendo;
	int hambreando;
} filo_t;

typedef struct 
{
	filo_t * filosofos;
	int * tenedores;
} moni_t;

typedef struct
{
	filo_t * filosofo;
	int * tenedorD;
	int * tenedorI;
} instan_t;

void * monitor_hnd (void *);
void * filosofos_hnd (void *);

void init(filo_t[], pthread_mutex_t *, int[], moni_t *);
void mostrar_filo(filo_t []);
void mostrar_tenedor(int[]);
void initCurse();


