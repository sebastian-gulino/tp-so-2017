#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <estructuras.h>
#include "fsHelper.h"

//sock: Socket del cliente.
//cc: Return de la conexión
//ms: Return del envio de mensaje
int sock, cc, ms;
//server: Estructura de las direcciones del servidor a conectarse.
struct sockaddr_in server;
//mensaje: Mensaje a enviar
//respuesta: Respuesta del servidor
char mensaje[500] , respuesta[2000], unMensaje[500];

int main(int arc, char * argv[]){

		//Genera archivo log para poder escribir el trace de toda la ejecución
		logger = malloc(sizeof(t_log));
		crearLog("/FILESYSTEM");

		//Levanta la configuración del proceso filesystem
		configuracion = cargarConfiguracion();

		//int socketCliente = crearCliente();

		//enviarMensaje(socketCliente);
		return 0;

}

