/*
 * manejoMemoria.h
 *
 *  Created on: 5/5/2017
 *      Author: utnso
 */
#ifndef MANEJOMEMORIA_H_
#define MANEJOMEMORIA_H_

#include <commons/config.h>
#include <logger.h>

typedef struct config_t {

	int puerto;
	char * marcos;
	char * marcoSize;
	char * entradasCache;
	char * cacheXProc;
	char * reemplazoCache;
	char * retardoMemoria;
	char * stackSize;

} t_configuracion;

t_configuracion configuracion;

t_configuracion cargarConfiguracion();

void crearMemoriaPrincipal();

void liberarMemoriaPrincipal();

void crearEstructurasAdministrativas();

void escribirPagina(int pagina, void* bytes, int size, int offset);

void* leerPagina(int pagina, int pid);

void vaciarCache();

void crearCache();

void crearCache();

void imprimirTablaPaginas();

#endif /* MANEJOMEMORIA_H_ */
