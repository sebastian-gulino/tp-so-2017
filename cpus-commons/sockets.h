/*
 * sockets.h
 *
 *  Created on: 14/4/2017
 *      Author: utnso
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef SOCKETS_SOCKETS_H_
#define SOCKETS_SOCKETS_H_


char * empaquetar(char * letraProceso, char * mensaje);

char * desempaquetar(char * mensajeEmpaquetado);

char * procesoEmisor(char * mensajeEmpaquetado);

int crearServidor(void);

#endif /* SOCKETS_SOCKETS_H_ */
