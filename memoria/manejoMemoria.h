/*
 * manejoMemoria.h
 *
 *  Created on: 5/5/2017
 *      Author: utnso
 */

#include <commons/config.h>

#ifndef MANEJOMEMORIA_H_
#define MANEJOMEMORIA_H_

typedef struct config_t {

	char * puerto;
	char * marcos;
	char * marcoSize;
	char * entradasCache;
	char * cacheXProc;
	char * reemplazoCache;
	char * retardoMemoria;

} t_configuracion;

void* crearMemoriaPrincipal(int frames, int frameSize);

void liberarMemoriaPrincipal(void* punteroMemoriaPrincipal);

void crearEstructurasAdministrativas(void* punteroMemoriaPrincipal,int frames);

void escribirEnMemoria(int pagina, char* texto);

void* leerPagina(int pagina);

#endif /* MANEJOMEMORIA_H_ */
