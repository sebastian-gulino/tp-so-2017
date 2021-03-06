#ifndef MANEJOMEMORIA_H_
#define MANEJOMEMORIA_H_

#include <arpa/inet.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <estructuras.h>
#include <logger.h>
#include <netdb.h>
#include <pthread.h>
#include <sockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define FRAME_LIBRE 0
#define FRAME_ESTRUCTURA_ADMINISTRATIVA -1

t_list* listaCpus;
t_list* listaKernel;

pthread_t threadAtenderConexiones;
pthread_t threadCommandHandler;
int consolaConectada;
pthread_mutex_t mutex_log;
pthread_mutex_t mutex_memoria;

typedef struct config_t {

	int32_t puerto;
	int32_t marcos;
	int32_t marcoSize;
	int32_t entradasCache;
	int32_t cacheXProc;
	int32_t reemplazoCache;
	int32_t retardoMemoria;
	int32_t stackSize;

} t_configuracion;

typedef struct cache_t {
	int pid;
	int pagina;
	void* contenido;
	int contadorDeUso;
} t_cache;

typedef struct resultadoLectura {
	bool resultado;
	void * contenido;
} t_resultadoLectura;

t_cache* cache;

t_struct_numero* tamanio_pagina;

t_filaTablaInvertida* tablaInvertida;

int retardoLecturaMemoria;

t_configuracion * configuracion;
char * pathConfiguracion;
void cargarConfiguracion();


void inicializarListas();

void actualizarCache(int pid,int pagina,void* punteroMarco);

void administrarConexiones();

void aplicarRetardo();

bool reservarFramesProceso(int pid, int cantidadBytes, int bytesContiguos);

void atenderPedidoEscritura(char * solicitante, int pid, int cantidadFrames);

void crearCache();

void crearEstructurasAdministrativas();

void crearMemoriaPrincipal();

void crearThreadConexiones();

void finalizarPrograma(int pid);

void imprimirTablaPaginas();

void manejarCpu(int i);

void manejarKernel(int i);

void removerClientePorCierreDeConexion(int cliente, t_list* lista);

void liberarMemoriaPrincipal();

void vaciarCache();

void imprimirTablaPaginas();

void imprimirCache();

int aplicarFuncionHash(int pid, int pagina);
int aplicarManejoColisiones(int frame);

int obtenerPrimerosNFramesLibres(int pid, int cantidad);

bool liberarPagina(int pagina, int pid);

t_resultadoLectura leerPagina(int pagina, int pid, int offset, int tamanio);

bool escribirPagina(int pagina, int pid, int offset, int tamanio, void * contenido);

void manejoConsola();

#endif /* MANEJOMEMORIA_H_ */
