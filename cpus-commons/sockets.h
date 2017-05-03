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
#include "estructuras.h"
#include "serializacion.h"
#include <commons/log.h>

#ifndef SOCKETS_SOCKETS_H_
#define SOCKETS_SOCKETS_H_

int crearServidor(int puertoEscucha);

int crearCliente(char* ipServidor,int puertoServidor, t_log* logger);

int aceptarCliente(int socketEscucha);

fd_set combinar_master_fd(fd_set* master_cpu, fd_set* master_consola, int fdmax);

int socket_recibir(int socketEmisor, t_tipoEstructura * tipoEstructura, void** estructura);

int socket_enviar(int socketReceptor, t_tipoEstructura tipoEstructura, void* estructura);

#endif /* SOCKETS_SOCKETS_H_ */
