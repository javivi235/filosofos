#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ncurses.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <pthread.h>
#include <time.h>
#include "filosofos.h"

int
main()
{
	pthread_t monitor_th;
	pthread_t filosofos_th[NB_THREADS];
	
	pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

	filo_t filosofos[NB_THREADS];
	int tenedores[NB_THREADS];
	moni_t monitor;

	init(filosofos, &mtx, tenedores, &monitor);

	instan_t instancias[NB_THREADS];
	for (int i = 0; i < NB_THREADS; i++)
	{
		instancias[i].filosofo = &(monitor.filosofos[i]);		
		instancias[i].tenedorI = &monitor.tenedores[i];
		instancias[i].tenedorD = &monitor.tenedores[(i+1)%4];
	}

	pthread_create (&monitor_th, NULL, monitor_hnd, (void *)(&monitor));
	
	for (int i = 0; i < NB_THREADS; i++) 
	{
	pthread_create (&filosofos_th[i], NULL, filosofos_hnd, (void  *)(&instancias[i]));
	}

	pthread_join (monitor_th, NULL);

	for (int i = 0; i < NB_THREADS; i++) 
	{
	pthread_join (filosofos_th[i], NULL);
	}
	
	exit(0);
}

void esperar(int tiempoTotal, int *contador)
{
	for (int i = 0; i < tiempoTotal; i++)
	{
		usleep(1000000);
		*contador = *contador + 1;
	}
}

void
decodificadorEstadoF (int estado, char *id)
{

	char *newID = id; 
	switch (estado)
	{
		
		case 0:
		*newID = 'D';		
		break;	
		case 1:
		*newID = 'P';		
		break;	
		case 2:
		*newID = 'C';		
		break;	
		case 3:
		*newID = 'H';		
		break;
		default:
		*newID = 'E';
		break;
		
	}
}

void * filosofos_hnd (void *arg)
{
	instan_t * instancia = (instan_t *) arg;
	int tiempoEnEstado = 0;
	int tiempoTranscurrido;
	while (TRUE)
	{	
		tiempoTranscurrido = 0;
		tiempoEnEstado = rand() %11;
		switch (instancia -> filosofo -> estado) {
		case DURMIENDO:
			esperar(tiempoEnEstado, &tiempoTranscurrido);
			(instancia -> filosofo -> estado) = PENSANDO;
		break;
		case PENSANDO:		
			esperar(tiempoEnEstado, &tiempoTranscurrido);
			(instancia -> filosofo -> pensando) = (instancia -> filosofo -> pensando) + tiempoTranscurrido;
			(instancia -> filosofo -> estado) = COMIENDO;		
		break;
		case COMIENDO:
		pthread_mutex_lock((instancia->filosofo->mtx));
		if (*(instancia->tenedorI) == LIBRE && *(instancia -> tenedorD) == LIBRE)
		{
			*(instancia -> tenedorI) = OCUPADO;			
			*(instancia -> tenedorD) = OCUPADO;
		
		} else {
			(instancia -> filosofo -> estado) = HAMBREANDO;	
		}
		pthread_mutex_unlock((instancia->filosofo->mtx));
		if ((instancia -> filosofo -> estado) == COMIENDO)
		{
			esperar(tiempoEnEstado, &tiempoTranscurrido);
			(instancia -> filosofo -> comiendo) = (instancia -> filosofo -> comiendo) + tiempoTranscurrido;
			*(instancia -> tenedorI) = LIBRE;			
			*(instancia -> tenedorD) = LIBRE;
			(instancia -> filosofo -> estado) = PENSANDO;	
		}
		break;
		case HAMBREANDO:
		esperar(tiempoEnEstado, &tiempoTranscurrido);
		(instancia -> filosofo -> hambreando) =(instancia -> filosofo -> hambreando) + tiempoTranscurrido;
		(instancia -> filosofo -> estado) = COMIENDO;	
		break;
		}
		
	}
}

void 
init(filo_t filosofos[], pthread_mutex_t *mtx, int tenedores[] , moni_t *monitor)
{
	for (int i=0; i<NB_THREADS; i++)
	{
		filosofos[i].id = i;
		filosofos[i].mtx = mtx;
		filosofos[i].estado = DURMIENDO;
		filosofos[i].pensando = 0;
		filosofos[i].comiendo = 0;
		filosofos[i].hambreando = 0;

		tenedores[i] = LIBRE;
	}

	monitor->filosofos = filosofos;
	monitor->tenedores = tenedores;
}

void
mostrar_filo(filo_t filosofos[])
{
	char estado;
	for (int i=0; i<NB_THREADS; i++)
	{
		decodificadorEstadoF(filosofos[i].estado, &estado);
		mvprintw(0, i*20, "Filosofo    %d:\n", i);
		mvprintw(2, i*20, "id:         %d\n", filosofos[i].id);
		mvprintw(4, i*20, "estado:     %c\n",estado);
		mvprintw(6, i*20, "pensando:   %d\n",filosofos[i].pensando);
		mvprintw(8, i*20, "comiendo:   %d\n",filosofos[i].comiendo);
		mvprintw(10, i*20,"hambreando: %d\n",filosofos[i].hambreando);
	}
}

void
mostrar_tenedor(int tenedores[])
{
	for (int i=0; i<NB_THREADS; i++)
	{
		mvprintw(18, i*20,"Tenedor %d:\n", i);
		mvprintw(20, i*20,"id:     %d\n",i);
		mvprintw(22, i*20,"estado: %d\n",tenedores[i]);
	}
}

void *
monitor_hnd (void *arg)
{
	initCurse();
	moni_t * monitor=(moni_t*)arg;
	while (TRUE)
	{
		
		clear();
		usleep(100);
		mostrar_filo(monitor->filosofos);
		mostrar_tenedor(monitor->tenedores);
		refresh();	
	}

	return (NULL);	
}
void
initCurse()
{
	initscr();
	start_color();

	init_pair(1, COLOR_YELLOW, COLOR_BLUE);
	init_pair(2, COLOR_WHITE, COLOR_BLUE);
	init_pair(3, COLOR_GREEN, COLOR_BLUE);

	bkgd(COLOR_PAIR(1) | A_BOLD);
}

