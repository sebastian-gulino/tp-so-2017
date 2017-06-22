#ifndef FSHELPER_H_
#define FSHELPER_H_

#include <arpa/inet.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <estructuras.h>
#include <logger.h>
#include <netdb.h>
#include <pthread.h>
#include <sockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <commons/bitarray.h>
#include <sys/mman.h>

typedef struct config_t {

	int puertoFS;
	char * puntoMontaje;
} t_configuracion;

t_configuracion configuracion;

pthread_t threadAtenderKernel;

t_configuracion cargarConfiguracion();

void crearServidorMonocliente();

void manejarKernel(int i);

void crearBitmap();

int validarArchivo();

#endif /* FSHELPER_H_ */
