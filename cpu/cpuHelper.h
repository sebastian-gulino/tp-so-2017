#ifndef CPUHELPER_H_
#define CPUHELPER_H_

#include <commons/config.h>
#include <commons/collections/list.h>
#include <parser/metadata_program.h>
#include <parser/parser.h>
#include <logger.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <estructuras.h>
#include <signal.h>
#include "primitivas.h"

typedef struct config_t {

	char * ipKernel;
	int puertoKernel;
	char * ipMemoria;
	int puertoMemoria;

} t_configuracion;

AnSISOP_funciones funcionesAnsisop = {
	.AnSISOP_definirVariable 			= definirVariable,
	.AnSISOP_obtenerPosicionVariable 	= obtenerPosicionVariable,
	.AnSISOP_dereferenciar				= dereferenciar,
	.AnSISOP_asignar					= asignar,
	.AnSISOP_obtenerValorCompartida		= obtenerValorCompartida,
	.AnSISOP_asignarValorCompartida		= asignarValorCompartida,
	.AnSISOP_irAlLabel					= irAlLabel,
	.AnSISOP_llamarSinRetorno			= llamarSinRetorno,
	.AnSISOP_llamarConRetorno			= llamarConRetorno,
	.AnSISOP_finalizar					= finalizar,
	.AnSISOP_retornar					= retornar,
};

AnSISOP_kernel funciones_kernel = {
		.AnSISOP_wait					= s_wait,
		.AnSISOP_signal					= s_signal,
		.AnSISOP_reservar				= reservar,
		.AnSISOP_liberar				= liberar,
		.AnSISOP_abrir					= abrir,
		.AnSISOP_cerrar					= cerrar,
		.AnSISOP_borrar					= borrar,
		.AnSISOP_leer					= leer,
		.AnSISOP_escribir				= escribir,
		.AnSISOP_moverCursor			= moverCursor,
};

t_configuracion configuracion;

t_configuracion cargarConfiguracion();


bool cpuLibre, signalFinalizarCPU, stackOverflow, finPrograma;

int devolvioPcb;

#define WAIT 1
#define IO 2

// Estructura que almacenara el pcb del proceso que la CPU esta ejecutando
t_struct_pcb* pcbEjecutando;
// FD para mantener las conexiones con Kernel y Memorias
int socketKernel, socketMemoria;
// Variable que almacenará el tamaño de cada pagina de memoria
int32_t tamanio_pagina;
// Variable que almacenará el tamaño de stack
int32_t tamanio_stack;
// Variable que resguardara el quantum para planificacion
int32_t quantum;
// Variable que resguardara el retardo de quantum entre cada instruccion
int32_t quantumSleep;

int conectarAKernel ();

int conectarAMemoria ();

void manejarSignal ();

void desconectarCPU ();

void inicializarEstructuras ();

void prepararInstruccion(char * instruccion);

void liberarRecursosCPU();

void salirProceso();

void liberarRegistroStack(registroStack* registroStack);

void liberarPCB();

void ejecutarProceso();

void recibirProcesoKernel();

char * pedirSiguienteInstruccion()

#endif /* CPUHELPER_H_ */
