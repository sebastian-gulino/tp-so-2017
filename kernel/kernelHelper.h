#ifndef KERNELHELPER_H_
#define KERNELHELPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <commons/config.h>
#include <logger.h>
#include <string.h>
#include <unistd.h>
#include <commons/collections/queue.h>
#include <sys/socket.h>
#include <sockets.h>
#include <estructuras.h>
#include <netdb.h>
#include <arpa/inet.h>

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

t_list *listaCpus;
t_list *listaConsolas;

pthread_t threadAtenderConexiones;

//Declaro los conjuntos de descriptores que contendran a los clientes conectados
fd_set master_consola;
fd_set master_cpu;


t_configuracion cargarConfiguracion();

void inicializarListas();

void manejarNuevaConexion(int listener, int *fdmax);

void crearThreadAtenderConexiones();

void administrarConexiones();

void manejarConsola(int i);

void manejarCpu(int i);

void removerClientePorCierreDeConexion(int cliente, t_list* lista, fd_set *fdSet);

#endif /* KERNELHELPER_H_ */
