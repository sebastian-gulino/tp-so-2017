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

int conectarAKernel ();

#endif /* CPUHELPER_H_ */
