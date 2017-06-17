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

t_tipoEstructura tipoEstructura;

void * structRecibido;

int socketKernel;

t_struct_numero confirmation_send;

t_configuracion configuracion;

t_configuracion cargarConfiguracion();

pthread_t threadProgramHandler, threadCommandHandler;

int conectarAKernel();

int commandHandler();

int commandParser();

void iniciarPrograma(char* pathArchivo);

void recibirMensajesPrograma(int pid);

#endif /* CONSOLAHELPER_H_ */
