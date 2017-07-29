#ifndef SOCKETS_SOCKETS_H_
#define SOCKETS_SOCKETS_H_

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "estructuras.h"
#include "serializacion.h"
#include "logger.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

int crearServidor(int puertoEscucha);

int crearCliente(char* ipServidor,int puertoServidor);

int aceptarCliente(int socketEscucha);

fd_set combinar_master_fd(fd_set* master_cpu, fd_set* master_consola, int fdmax);

int socket_recibir(int socketEmisor, t_tipoEstructura * tipoEstructura, void** estructura);

int socket_enviar(int socketReceptor, t_tipoEstructura tipoEstructura, void* estructura);

#endif /* SOCKETS_SOCKETS_H_ */
