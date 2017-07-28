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
#include <sys/inotify.h>

typedef struct config_t {

	int puertoProg;
	int puertoCpu;
	char * ipMemoria;
	int puertoMemoria;
	char * ipFS;
	int puertoFS;
	int quantum;
	int quantumSleep;
	char * algoritmo;
	int gradoMultiprog;
	char** semIDS;
	char** semINIT;
	char** sharedVars;
	int stackSize;
	int puertoEscucha;

} t_configuracion;

// Estructura que almacenará la configuracion del kernel
t_configuracion * configuracion;

// Variable que almacenará el tamaño de cada pagina de memoria
int32_t tamanio_pagina;

int maximoPID, cantidadTotalPID;

// FD del socket de comunicacion con memoria y filesystem
int socketMemoria, socketFS;

bool kernelPlanificando, quantumSleepActualizado;

int socketInotify;
int watchInotify;
int sizeInotify;

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
t_list* listaVarCompartidas;

int bloqueEspecial;
char * pathConfiguracion;

pthread_t threadAtenderConexiones;

// Declaro los conjuntos de descriptores que contendran a los clientes conectados
fd_set master_consola;
fd_set master_cpu;

t_struct_pcb pcb;
t_limites_instrucciones limitesInstrucciones;

t_list * indiceStack;

void administrarConexiones();

void inicializarListas();

void cargarConfiguracion();
void cargarSemaforos();
void cargarVariablesCompartidas();

void crearThreadAtenderConexiones();

int conectarAMemoria();

void finalizarProcesoOK(int socketCPU, t_struct_pcb * pcbFinalizado);

void manejarConsola(int i);

void manejarCpu(int i);

void manejarNuevaConexion(int listener, int *fdmax);

int obtener_pid();

void removerClientePorCierreDeConexion(int cliente, fd_set *fdSet);

void enviarConfiguracion(int socketCliente, int valor);

t_struct_pcb* crearPCB(int PID, t_struct_pcb * pcb);

char * prepararInstruccion(char * instruccion);

int solicitarSegmentoCodigo(int pid, int tam_programa);

int solicitarSegmentoStack(int pid);

registroStack* reg_stack_create();

void agregarColaListos(t_struct_pcb* pcb);

void crearArchivosPorProceso(int PID);

void abrirArchivo(int socketCPU,t_struct_archivo * archivo);
void leerArchivo(int socketCPU,t_struct_archivo * archivo);
void escribirArchivo(int socketCPU,t_struct_archivo * archivo);
void moverCursorArchivo(int socketCPU,t_struct_sol_lectura * archivoMover);
void borrarArchivo(int socketCPU,t_struct_sol_lectura * archivoBorrar);
void cerrarArchivo(int socketCPU,t_struct_sol_lectura * archivoCerrar);

void grabarVariableCompartida(int socketCPU, t_struct_var_compartida * grabarVarCompartida);
void obtenerVariableCompartida(int socketCPU, char * varCompartida);

void realizarWaitSemaforo(int socketCPU,char * waitSemaforo);
void realizarSignalSemaforo(int socketCPU,char * signalSemaforo);

void inicializarProceso(int socketConsola, char * programa, int tamanio_programa);
void matarProcesoEnEjecucion(int socketCPU, bool desconectarCPU);
void abortarPrograma(int socketConsola, bool finalizarPrograma);
void finalizarProcesoConsola(int pid);
void actualizarPCBExec(t_struct_pcb * pcbBuscado);

void crearInformacionProcesoInicial(int PID);
void imprimirTablaGlobalArchivos();
void imprimirArchivosProceso(int pid);
void imprimirSyscallProceso(int pid);
void imprimirRafagasProceso(int pid);

void recuperarInformacion(int pid, char * info);

void listarProcesosEnCola(t_list * cola, char * estado);

void reservarHeap(int socketCPU, t_struct_sol_heap * solicitudHeap);

void imprimirHeapProceso(int pid);

void revisarPaginaslibres(t_registroTablaHeap * paginaRevisar);


void modificacionArchConf(int fdInotify);

int commandParser(char* operacion);

void mensajeConsolaKernel();

void liberarHeap(int socketCPU, t_struct_sol_heap * solicitudHeap);

void imprimirProcesos(char* estado);

void ejecutarPlanificacion(int socketCPU);

#endif /* KERNELHELPER_H_ */
