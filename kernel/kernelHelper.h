#ifndef KERNELHELPER_H_
#define KERNELHELPER_H_

#include <arpa/inet.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
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
#include <parser/metadata_program.h>
#include <parser/parser.h>

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
	int stackSize;
	int puertoEscucha;

} t_configuracion;

// Estructura que almacenará la configuracion del kernel
t_configuracion configuracion;

// Variable que almacenará el tamaño de cada pagina de memoria
int32_t tamanio_pagina;

// cantidad de PID del sistema
int cantidad_pid;

// FD del socket de comunicacion con memoria
int socketMemoria;

t_list *listaCpus;
t_list *listaConsolas;
t_list *listaProcesos;

//Declaro las listas para manejar la planificación
t_queue* cola_new;
t_queue* cola_exit;
t_list* cola_ready;
t_list* cola_block;
t_list* cola_exec;


pthread_t threadAtenderConexiones;

// Declaro los conjuntos de descriptores que contendran a los clientes conectados
fd_set master_consola;
fd_set master_cpu;

t_pcb pcb;
t_limites_instrucciones limitesInstrucciones;
t_list * indiceCodigo;
t_list * indiceStack;

void administrarConexiones();

void inicializarListas();

t_configuracion cargarConfiguracion();

void crearThreadAtenderConexiones();

int conectarAMemoria();

void manejarConsola(int i);

void manejarCpu(int i);

void manejarNuevaConexion(int listener, int *fdmax);

int obtener_pid();

void removerClientePorCierreDeConexion(int cliente, t_list* lista, fd_set *fdSet);

void enviarConfiguracion(int socketCliente, int valor)

t_pcb crearPCB(char* programa, int PID, int tamanioPrograma);

int solicitarSegmentoCodigo(int pid, int tam_programa);

int solicitarSegmentoStack(int pid);

#endif /* KERNELHELPER_H_ */
