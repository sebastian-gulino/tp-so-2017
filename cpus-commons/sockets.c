/*
 * sockets.c
 *
 *  Created on: 14/4/2017
 *      Author: utnso
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "sockets.h"
#include <sys/socket.h>
#include <netdb.h>

int socketServer , client_sock , c , read_size, *new_sock, clientes[5], i;

//server: Direcciones del server (puerto, ip, etc).
//client: Direcciones del cliente.
struct sockaddr_in client;

//client_message[2000]: Buffer donde se almacena el mensaje recibido.
char client_message[2000];

char * empaquetar(char * inicioMensaje, char * mensaje)
{
	int size = strlen(mensaje);
	char * buffer = malloc(size + 1);
	memcpy(buffer, &inicioMensaje,1);
	strcat(buffer,mensaje);
	return buffer;
}

char * desempaquetar(char * mensajeEmpaquetado){
	int tamanio = strlen(mensajeEmpaquetado);
	char * subbuff = malloc(tamanio + 1);
	memcpy( subbuff, &mensajeEmpaquetado[1], tamanio );
	subbuff[tamanio-1] = '\0';
	return subbuff;
}

char * procesoEmisor(char * mensajeEmpaquetado){
	char * proceso;
	switch(mensajeEmpaquetado[0]){
		case 'C':
			proceso = "Consola";
			break;
		case 'M':
			proceso = "Memoria";
			break;
		case 'P':
			proceso = "CPU";
			break;
		case 'F':
			proceso = "FileSystem";
			break;
		default:
			break;
	}
	return proceso;
}

int crearServidor(void){

		struct sockaddr_in server;

	    //Se crea el socket
	    socketServer = socket(AF_INET , SOCK_STREAM , 0);

	    if (socketServer == -1)
	    {
	        puts("El socket no pudo ser creado.");
	        return EXIT_FAILURE;
	    }
	    puts("El socket fue creado exitosamente.");

	    //Se instancia la estructura "sockaddr_in" que contiene las direcciones del servidor.
	    server.sin_family = AF_INET; //Especifica familia de direcciones.
	    server.sin_addr.s_addr = INADDR_ANY; //Especifica que no se va a hacer bind a una IP especifica.
	    server.sin_port = htons( 8002); //Especifica el puerto del servidor.


	    printf("%s\n", inet_ntoa(server.sin_addr));

	    //Se liga (bind) el socket servidor con sus direcciones.
	    if( bind(socketServer,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        perror("Bind fallo. Error");
	        return 1;
	    }
	    puts("Bind realizado exitosamente.");

	    //Pone al servidor en modo listen (puede recibir llamados).
	    listen(socketServer , 3);

	    puts("Servidor creado con exito.");
	    puts("Esperando por conexiones entrantes...");

    return socketServer;
}


int crearCliente(void){

	int socketClient, cc, ms;
	//server: Estructura de las direcciones del servidor a conectarse.
	struct sockaddr_in server;
	 //Se crea el socket del cliente.
	  socketClient = socket(AF_INET , SOCK_STREAM , 0);
	    if (socketClient == -1)
	    {
	        puts("No se pudo crear el socket");
	    }
	    puts("Socket creado exitosamente");

	   //Se instancian las direcciones del servidor a conectarse.
	   server.sin_addr.s_addr = inet_addr("127.0.0.1"); //"127.0.0.1" es la ip de la maquina (localhost).
	   server.sin_family = AF_INET; //Familia de direcciones.
	   server.sin_port = htons( 8002 ); //"8300" es el puerto del servidor a conectarse (en este caso es el servidor KERNEL{kernel.c}).

	   puts("Cliente creado.");
	   puts("Intentando conexión...");

	  //Hace la conexión al servidor.
	   cc = connect(socketClient , (struct sockaddr *)&server , sizeof(server));

	    if(cc < 0){
	        perror("Conexión fallida. Error");
	        return 1;
	    }

	    puts("Conexión exitosa!\n");

	    return socketClient;
	}
