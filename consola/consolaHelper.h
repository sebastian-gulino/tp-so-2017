#ifndef CONSOLAHELPER_H_
#define CONSOLAHELPER_H_

#include <commons/config.h>
#include <logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <estructuras.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

typedef struct config_t {

	char * ipKernel;
	int puertoKernel;

} t_configuracion;

typedef struct t_program {
	int pid;
	int socketKernel;
	pthread_t hilo;
	time_t inicioEjec;
	time_t finEjec;
	int cantImpresiones;
} t_proceso;

time_t rawtime;
struct tm * timeinfo;

t_list* listaProcesos;

t_tipoEstructura tipoEstructura;

void * structRecibido;

pthread_mutex_t mutex_log;
pthread_mutex_t mutex_lista_procesos;

int cantidadThreads;
int consolaConectada;

pthread_t threadCommandHandler;

t_struct_numero confirmation_send;

t_configuracion configuracion;

t_configuracion cargarConfiguracion();

int conectarAKernel();

void commandHandler();

int commandParser();

void finalizarPrograma(int pid);

void inicializarEstructuras();

void iniciarPrograma(char* pathArchivo);

void manejarSignal(int sign);

void recibirMensajes(t_proceso* proceso);

void terminarProceso(t_proceso* proceso);

void manejarDesconexion();



#endif /* CONSOLAHELPER_H_ */
