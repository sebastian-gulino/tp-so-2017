#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sockets.h>

typedef struct config_t {

	char * puertoProg;
	char * puertoCpu;
	char * ipMemoria;
	char * puertoMemoria;
	char * ipFS;
	char * puertoFS;
	char * quantum;
	char * quantumSleep;
	char * algoritmo;
	char * gradoMultiprog;
	char * semIDS;
	char * semINIT;
	char * sharedVars;
	char * stackSize;


} t_configuracion;

t_configuracion configuracion;


//socket_cliente: Socker del cliente a aceptar.
//fdmax: número máximo de descriptores de fichero.
int socket_cliente, fdmax, i, j, addrlen, recvBytes;

char * buffer_mens_cliente[500];
//server: Direcciones del server (puerto, ip, etc).
//client: Direcciones del cliente.
struct sockaddr_in server, client;

//master: conjunto maestro de descriptores de fichero
//read_fds: conjunto temporal de descriptores de fichero para select()
fd_set master, read_fds;




int recibirConexiones_select (int socketServidor){

	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);

	// añadir servidor al conjunto maestro
	FD_SET(socketServidor, &master);
	// seguir la pista del descriptor de fichero mayor
	fdmax = socketServidor;

	while(1){

		read_fds = master; // se copia el master al temporal

		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
		 perror("select");
		   exit(1);
		 }

		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { //testea si un file descriptor es parte del set
				 if (i == socketServidor) {

					 addrlen = sizeof(client);

					 socket_cliente = accept(socketServidor, (struct sockaddr *)&client, &addrlen); //Acepta una conexión
					  //TODO: Hacer el handshake
					 if (socket_cliente == -1) {
					  perror("accept");
					 }	else {
						 FD_SET(socket_cliente, &master); // añadir al conjunto maestro

						 if (socket_cliente > fdmax) {    // actualizar el máximo
						 fdmax = socket_cliente;
					 }
						 printf("Kernel: Nueva conexión de la IP %s en el " "socket %d\n", inet_ntoa(client.sin_addr), socket_cliente);
					 }
			} else {
				 // Recibe de un cliente
				  if ((recvBytes = recv(i, buffer_mens_cliente, sizeof(buffer_mens_cliente), 0)) <= 0) {
					  if (recvBytes == 0) {
					  // conexión cerrada
					  printf("Kernel: socket %d ha cerrado la comunicación\n", i);
					  } else {
					       perror("recv");
					   	  }
					  close(i); // Se cierra la comunicación con el socket
					  FD_CLR(i, &master); // elimina el socket cliente del conjunto maestro


				  } else { // Se recibieron datos

					  for(j = 0; j <= fdmax; j++) { //Replicación del mensaje
						  // TODO: Hacer la replicación con listas para poder manejar a quien le mando los mensajes.
						  if (FD_ISSET(j, &master)) {

							 if (j != socketServidor && j != i) { //No se lo envío ni al servidor ni al cliente que mando al mensaje

								if (send(j, buffer_mens_cliente, recvBytes, 0) == -1) {
								   perror("send");
							  }
							}
						 }
					  }
				   }

			 }
		}

    } //end_for


  }//end_while
	return 0;
}//end recibirConexiones_select


void cargarConfiguracion(void) {
	t_config * config;

	config = config_create("./config.txt");

	if(config == NULL){

		config = config_create("../config.txt");

	}  //TODO: Encontrar una forma menos villera de hacer esto. Seba dixit.

	configuracion.puertoCpu = strdup(config_get_string_value(config, "PUERTO_CPU"));
	configuracion.ipMemoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
	configuracion.puertoProg = strdup(config_get_string_value(config, "PUERTO_PROG"));
	configuracion.puertoMemoria = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
	configuracion.ipFS = strdup(config_get_string_value(config, "IP_FS"));
	configuracion.puertoFS = strdup(config_get_string_value(config, "PUERTO_FS"));
	configuracion.quantum = strdup(config_get_string_value(config, "QUANTUM"));
	configuracion.quantumSleep = strdup(config_get_string_value(config, "QUANTUM_SLEEP"));
	configuracion.algoritmo = strdup(config_get_string_value(config, "ALGORITMO"));
	configuracion.gradoMultiprog = strdup(config_get_string_value(config, "GRADO_MULTIPROG"));
	configuracion.semIDS = strdup(config_get_string_value(config, "SEM_IDS"));
	configuracion.semINIT = strdup(config_get_string_value(config, "SEM_INIT"));
	configuracion.sharedVars = strdup(config_get_string_value(config, "SHARED_VARS"));
	configuracion.stackSize = strdup(config_get_string_value(config, "STACK_SIZE"));

	printf("El puerto de la CPU es %s\n",configuracion.puertoCpu);
		printf("La IP de la Memoria es %s\n",configuracion.ipMemoria);
		printf("El puerto de la Memoria es %s\n",configuracion.puertoMemoria);
		printf("El puerto del Programa es %s\n",configuracion.puertoProg);
		printf("La IP del FS es %s\n",configuracion.ipFS);
		printf("El puero del FS es %s\n",configuracion.puertoFS);
		printf("El Quantum es %s\n",configuracion.quantum);
		printf("EL Quantum Sleep es %s\n",configuracion.quantumSleep);
		printf("El Algoritmo es %s\n",configuracion.algoritmo);
		printf("El grado multiprog es %s\n",configuracion.gradoMultiprog);
		printf("La Sem IDS es %s\n",configuracion.semIDS);
		printf("La Sem INIT es %s\n",configuracion.semINIT);
		printf("Las variables compartidas son %s\n",configuracion.sharedVars);
		printf("El tamaño del Stack es %s\n",configuracion.stackSize);


}



int main(int arc, char * argv[]) {

	cargarConfiguracion();

	int socketServer = crearServidor();

	recibirConexiones_select(socketServer);

	return 0;

}
