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

typedef struct config_t {

	int puerto;
	char * marcos;
	int32_t marcoSize;
	char * entradasCache;
	char * cacheXProc;
	char * reemplazoCache;
	char * retardoMemoria;
	int32_t stackSize;

} t_configuracion;

t_list* listaCpus;
t_list* listaKernel;

pthread_t threadAtenderConexiones;

t_configuracion configuracion;

void* memoriaPrincipal;

typedef unsigned char frame[500];

typedef struct cache_t {
	int pid;
	int pagina;
	void* contenido;
} t_cache;

typedef struct resultado_busqueda_cache_t {
	int cantidad;
	int indices[15];
} t_resultado_busqueda_cache;

t_cache* cache;

t_struct_numero* tamanio_pagina;

t_filaTablaInvertida* tablaInvertida;

t_configuracion cargarConfiguracion();

void inicializarListas();

void actualizarCache(int pid,int pagina,void* punteroMarco);

void administrarConexiones();

void aplicarRetardo(int retardo);

int asignarPaginasProceso(int pid, int numeroFramesPedidos);

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



void escribirPagina(int pagina, void* bytes, int size, int offset);

void* leerPagina(int pagina, int pid);

void vaciarCache();



void imprimirTablaPaginas();

#endif /* MANEJOMEMORIA_H_ */
