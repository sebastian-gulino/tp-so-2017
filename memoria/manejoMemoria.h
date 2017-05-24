/*
 * manejoMemoria.h
 *
 *  Created on: 5/5/2017
 *      Author: utnso
 */

#include <commons/config.h>
#include <logger.h>

#ifndef MANEJOMEMORIA_H_
#define MANEJOMEMORIA_H_

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
