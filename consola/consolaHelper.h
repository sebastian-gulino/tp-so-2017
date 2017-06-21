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
#include <time.h>

typedef struct config_t {

	char * ipKernel;
	int puertoKernel;

} t_configuracion;

time_t rawtime;
struct tm * timeinfo;

t_list* listaProcesos;

typedef struct t_program {
	int pid;
	int socketKernel;
	pthread_t hilo;
	time_t inicioEjec;
	time_t finEjec;
	int cantImpresiones;
} t_proceso;

t_tipoEstructura tipoEstructura;

void * structRecibido;

t_struct_numero confirmation_send;

t_configuracion configuracion;

t_configuracion cargarConfiguracion();

pthread_t threadCommandHandler;

int cantidadThreads;

int conectarAKernel();

int commandHandler();

int commandParser();

void iniciarPrograma(char* pathArchivo);

void recibirMensajesPrograma(int pid);

void inicializarListas();

#endif /* CONSOLAHELPER_H_ */
