#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sockets.h>
#include <estructuras.h>
#include <logger.h>
#include <commons/collections/queue.h>
#include "kernelHelper.h"

t_list *listaCpus;
t_list *listaConsolas;

pthread_t threadAtenderConexiones;

//Declaro los conjuntos de descriptores que contendran a los clientes conectados
fd_set master_consola;
fd_set master_cpu;


void manejarCpu(int i){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(i,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"El Cpu %d cerró la conexión.",i);
		removerClientePorCierreDeConexion(i,listaCpus,&master_cpu);
	} else {
	}
};

void manejarConsola(int i){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(i,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"La Consola %d cerró la conexión.",i);
		removerClientePorCierreDeConexion(i,listaConsolas,&master_consola);
	} else {
	}

};

void administrarConexiones (){

	int socketServidor = crearServidor(configuracion.puertoEscucha);

	fd_set read_fds;

	//Elimino el contenido de los conjuntos de descriptores
	FD_ZERO(&master_cpu);
	FD_ZERO(&master_consola);
	FD_ZERO(&read_fds);

	// Declaro como descriptor de fichero mayor al socket que escucha
	int fdmax = socketServidor;
	int i;

	while(1){

		read_fds = combinar_master_fd(&master_cpu, &master_consola, fdmax); // se copia el master al temporal
		// Añado el descriptor del socket escucha al conjunto
		FD_SET(socketServidor, &read_fds);

		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		for(i = 0; i <= fdmax; i++) {

			//testea si un file descriptor es parte del set
			if (FD_ISSET(i, &read_fds)) {

				if(FD_ISSET(i, &master_cpu)){
					manejarCpu(i);
				}

				if(FD_ISSET(i, &master_consola)){
					manejarConsola(i);
				}

				if(i==socketServidor){
					manejarNuevaConexion(i, &fdmax);
				}

			}

		} //end_for

	}//end_while

}

void manejarNuevaConexion(int listener, int *fdmax){

	int socketCliente = aceptarCliente(listener);

	void * structRecibido;

	t_tipoEstructura tipoStruct;

	int resultado = socket_recibir(socketCliente, &tipoStruct, &structRecibido);
	if(resultado == -1 || tipoStruct != D_STRUCT_NUMERO){
		printf("No se recibio correctamente a quien atiendo en el kernel\n");
	}

	switch(((t_struct_numero *)structRecibido)->numero){
		case ES_CONSOLA:

			puts("SE CONECTO UNA CONSOLA");
			//conexion_consola(socketCliente);

			FD_SET(socketCliente, &master_consola);

			list_add(listaConsolas, (void*)socketCliente);

			log_info(logger,"La consola %d se conectó.", socketCliente);

			break;

		case ES_CPU:

			puts("SE CONECTO UNA CPU");
			//conexion_cpu(socketCliente);

			FD_SET(socketCliente, &master_cpu);

			list_add(listaCpus, (void*)socketCliente);

			log_info(logger,"El Cpu %d se conectó.",socketCliente);

			break;
	}

	if (socketCliente>*fdmax){
		*fdmax = socketCliente;
	}

	free(structRecibido);

}

void crearThreadAtenderConexiones(){

	pthread_create(&threadAtenderConexiones, NULL, administrarConexiones, NULL);

}

void inicializarListas(){
	listaConsolas = list_create();
	listaCpus = list_create();
}


int main(int arc, char * argv[]) {

	//Genera archivo log para poder escribir el trace de toda la ejecución
	logger = malloc(sizeof(t_log));

	crearLog("/KERNEL");

	//Levanta la configuración del proceso filesystem
	configuracion = cargarConfiguracion();

	inicializarListas();

	crearThreadAtenderConexiones();

	pthread_join(threadAtenderConexiones, NULL);

	return 0;

}

void removerClientePorCierreDeConexion(int cliente, t_list* lista, fd_set *fdSet) {

	//Elimino el cliente de la lista
	bool _es_cliente_numero(int elemento) {
		return (elemento == cliente);
	}
	list_remove_by_condition(lista, (void*) _es_cliente_numero);

	//Elimino el cliente del fd_set
	FD_CLR(cliente,fdSet);

}
