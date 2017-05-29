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

typedef struct config_t {

	char * ipKernel;
	int puertoKernel;



} t_configuracion;

t_configuracion configuracion;

t_configuracion cargarConfiguracion();

pthread_t threadIniciar, threadCommandHandler;
int conectarAKernel ();
int commandHandler();
int commandParser();

#endif /* CONSOLAHELPER_H_ */
