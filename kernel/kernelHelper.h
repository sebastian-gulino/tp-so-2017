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

int maximoPID, cantidadTotalPID;

// FD del socket de comunicacion con memoria y filesystem
int socketMemoria, socketFS;

bool kernelPlanificando;

t_list *listaConsolas;
t_list *listaCpuLibres;
t_list *listaCpuOcupadas;
t_list *listaProcesos;
t_list *listaProcesosFinalizar;
t_list *tablaHeap;

// Listas para la gestion de archivos
t_list *tablaArchivosGlobal;
t_dictionary *tablaArchivosProceso;

// Lista en la que iremos resguardando la informacion para la consola del kernel
t_list *listaInformacionProcesos;

//Declaro las listas para manejar la planificación
t_list* cola_new;
t_list* cola_exit;
t_list* cola_ready;
t_list* cola_block;
t_list* cola_exec;

t_list* listaSemaforos;

pthread_t threadAtenderConexiones;

// Declaro los conjuntos de descriptores que contendran a los clientes conectados
fd_set master_consola;
fd_set master_cpu;

t_struct_pcb pcb;
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

void enviarConfiguracion(int socketCliente, int valor);

t_struct_pcb* crearPCB(int PID);

int solicitarSegmentoCodigo(int pid, int tam_programa);

int solicitarSegmentoStack(int pid);

registroStack* reg_stack_create();

void agregarColaListos(t_struct_pcb* pcb);

void crearArchivosPorProceso(int PID);

#endif /* KERNELHELPER_H_ */
