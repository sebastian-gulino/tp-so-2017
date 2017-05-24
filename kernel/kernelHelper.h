#ifndef KERNELHELPER_H_
#define KERNELHELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/config.h>
#include <string.h>
#include <unistd.h>

typedef struct config_t {

	char * puertoProg;
	char * puertoCpu;
	char * ipMemoria;
	char * puertoMemoria;
	char * ipFS;
	char * puertoFS;
	char * quantum;
	char * quantumSleep;
	char * algoritmo;
	char * gradoMultiprog;
	char * semIDS;
	char * semINIT;
	char * sharedVars;
	char * stackSize;
	int puertoEscucha;

} t_configuracion;

t_configuracion configuracion;

t_configuracion cargarConfiguracion();

#endif /* KERNELHELPER_H_ */
