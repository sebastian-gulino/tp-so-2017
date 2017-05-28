#ifndef KERNELHELPER_H_
#define KERNELHELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <logger.h>
#include <string.h>
#include <unistd.h>

typedef struct config_t {

	int puertoProg;
	int puertoCpu;
	char * ipMemoria;
	int puertoMemoria;
	char * ipFS;
	int puertoFS;
	int quantum;
	char * quantumSleep;
	char * algoritmo;
	int gradoMultiprog;
	char * semIDS;
	char * semINIT;
	char * sharedVars;
	char * stackSize;
	int puertoEscucha;

} t_configuracion;

t_configuracion configuracion;

t_configuracion cargarConfiguracion();


char buffLog[100];

#endif /* KERNELHELPER_H_ */
