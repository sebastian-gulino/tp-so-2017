#ifndef CPUHELPER_H_
#define CPUHELPER_H_

#include <commons/config.h>
#include <logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <estructuras.h>
#include <commons/collections/list.h>

typedef struct config_t {

	char * ipKernel;
	int puertoKernel;
	char * ipMemoria;
	int puertoMemoria;

} t_configuracion;

t_configuracion configuracion;

t_configuracion cargarConfiguracion();

// FD para mantener las conexiones con Kernel y Memorias
int socketKernel, socketMemoria;

// Variable que almacenará el tamaño de cada pagina de memoria
int32_t tamanio_pagina;

// Variable que almacenará el tamaño de stack
int32_t tamanio_stack;

int conectarAKernel ();

int conectarAMemoria ();

#endif /* CPUHELPER_H_ */
