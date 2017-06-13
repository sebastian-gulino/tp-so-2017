/*
 * sockets.c
 *
 *  Created on: 14/4/2017
 *      Author: utnso
 */

#include "sockets.h"

int crearServidor(int puertoEscucha){

		struct sockaddr_in server;

	    //Se crea el socket
	    int socketServer = socket(AF_INET , SOCK_STREAM , 0);

	    if (socketServer == -1)
	    {
	        log_error(logger,"El socket no pudo ser creado");

	        return EXIT_FAILURE;
	    }

	    //Se instancia la estructura "sockaddr_in" que contiene las direcciones del servidor.
	    server.sin_family = AF_INET; //Especifica familia de direcciones.
	    server.sin_addr.s_addr = INADDR_ANY; //Especifica que no se va a hacer bind a una IP especifica.
	    server.sin_port = htons(puertoEscucha); //Especifica el puerto del servidor.

	    //Se liga (bind) el socket servidor con sus direcciones.
	    if( bind(socketServer,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	    	log_error(logger,"Bind fallo. Error");
	        return 1;
	    }

	    //Pone al servidor en modo listen (puede recibir llamados).
	    listen(socketServer , 5);

	    log_info(logger,"Servidor escuchando en el puerto %d", puertoEscucha);
	    log_info(logger,"Esperando por conexiones entrantes...");

    return socketServer;
}

int crearCliente(char* ipServidor,int puertoServidor){

	int socketClient, cc;
	//server: Estructura de las direcciones del servidor a conectarse.
	struct sockaddr_in server;
	 //Se crea el socket del cliente.
	  socketClient = socket(AF_INET , SOCK_STREAM , 0);
	    if (socketClient == -1)
	    {
	    	log_error(logger,"Error al crear socket cliente");
	    }

	   //Se instancian las direcciones del servidor a conectarse.
	   server.sin_addr.s_addr = inet_addr(ipServidor); //"127.0.0.1" es la ip de la maquina (localhost).
	   server.sin_family = AF_INET; //Familia de direcciones.
	   server.sin_port = htons(puertoServidor); //"8300" es el puerto del servidor a conectarse (en este caso es el servidor KERNEL{kernel.c}).

	  //Hace la conexión al servidor.
	   cc = connect(socketClient , (struct sockaddr *)&server , sizeof(server));

	    if(cc < 0){
	    	log_error(logger,"Error al conectar con servidor");
	        return 1;
	    }

	    log_info(logger,"Cliente conectado correctamente al servidor IP:%s Puerto: %d", ipServidor, puertoServidor);

	    return socketClient;
	}

int aceptarCliente(int socketEscucha){
	int socketNuevaConexion;
	unsigned int size_sockAddrIn;
	struct sockaddr_in suSocket;
	size_sockAddrIn = sizeof(struct sockaddr_in);
	socketNuevaConexion = accept(socketEscucha, (struct sockaddr *)&suSocket, &size_sockAddrIn);
	if(socketNuevaConexion < 0) {
//		log_error(logger,"Error al aceptar conexion entrante");
		return -1;
	}
	return socketNuevaConexion;
}

//Combina los conjuntos de descriptores de consolas y cpus
fd_set combinar_master_fd(fd_set* master1, fd_set* master2, int maxfd){
	fd_set combinado;
	FD_ZERO(&combinado);
	int i;

	for(i=0; i <= maxfd; i++){
		if(FD_ISSET(i, master1) || FD_ISSET(i, master2)){
			FD_SET(i, &combinado);
		}
	}

	return combinado;
}

int socket_enviar(int socketReceptor, t_tipoEstructura tipoEstructura, void* estructura){
	int cantBytesEnviados;

	t_stream * paquete = serialize(tipoEstructura, estructura);

	cantBytesEnviados = send(socketReceptor, paquete->data, paquete->length, 0);
	free(paquete->data);
	free(paquete);
	if( cantBytesEnviados == -1){
		log_error(logger,"Server no encontrado\n");
		return 0;
	}
	else {
		return 1;
	}
}

int socket_recibir(int socketEmisor, t_tipoEstructura * tipoEstructura, void** estructura){
	int cantBytesRecibidos;
	t_header header;
	char* buffer;
	char* bufferHeader;

	bufferHeader = malloc(sizeof(t_header));

	cantBytesRecibidos = recv(socketEmisor, bufferHeader, sizeof(t_header), MSG_WAITALL);	//ReciBo por partes, primero el header.
	if(cantBytesRecibidos == -1){
		free(bufferHeader);
		//TODO ver como manejar esto al seguir buscando novedades llena el log
		//log_error(logger,"Error al recibir datos\n");
		return 0;
	}

	//Si se recibe 0, es porque se desconectó el socketEmisor
	if(cantBytesRecibidos == 0){
		free(bufferHeader);
		return -1;
	}

	header = desempaquetarHeader(bufferHeader);

	free(bufferHeader);

	if (tipoEstructura != NULL) {
		*tipoEstructura = header.tipoEstructura;
	}

	if(header.length == 0){	//Si recibo mensaje con length 0 retorno 1 y *estructura NULL.
		if (estructura != NULL) {
			*estructura = NULL;
		}
		return 1;
	}

	buffer = malloc(header.length);
	cantBytesRecibidos = recv(socketEmisor, buffer, header.length, MSG_WAITALL);	//Recibo el resto del mensaje con el tamaño justo de buffer.
	if(cantBytesRecibidos == -1){
		free(buffer);
		//TODO ver como manejar esto al seguir buscando novedades llena el log
		//log_error(logger,"Error al recibir datos");
		return 0;
	}

	//Si se recibe 0, es porque se desconectó el socketEmisor
	if(cantBytesRecibidos == 0){
		free(buffer);
		return -1;
	}

	if(estructura != NULL) {
		*estructura = deserialize(header.tipoEstructura, buffer, header.length);
	}

	free(buffer);

	return 1;
}

