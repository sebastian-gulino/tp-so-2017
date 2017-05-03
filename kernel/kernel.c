#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <commons/log.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sockets.h>
#include <estructuras.h>
#include <commons/collections/queue.h>

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
	int puertoEscucha;


} t_configuracion;

t_configuracion configuracion;

t_log* logger;

pthread_t threadAtenderConexiones;

//Declaro los conjuntos de descriptores que contendran a los clientes conectados
fd_set master_consola;
fd_set master_cpu;

//Utilizado para funcion inicializarListas
t_list* consolas;

t_log* crearLog(){

	char cad[80];
	char *directorioActual = getcwd(NULL, 0);

	strcat(cad,directorioActual);
	strcat(cad,"/kernel.log");

	logger = log_create(cad, "KERNEL", 0, LOG_LEVEL_TRACE);
	log_info(logger,"Comienza a ejecutar el proceso Kernel");

	return logger;
}

void cargarConfiguracion(void) {
	t_config * config;

	config = config_create("./config.txt");

	if(config == NULL){

		config = config_create("../config.txt");

	}  //TODO: Encontrar una forma menos villera de hacer esto. Seba dixit.

	//Ir liberando datos de la configuracion a medida que sean necesarios

	//	configuracion.puertoCpu = strdup(config_get_string_value(config, "PUERTO_CPU"));
	//	configuracion.ipMemoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
	//	configuracion.puertoProg = strdup(config_get_string_value(config, "PUERTO_PROG"));
	//	configuracion.puertoMemoria = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
	//	configuracion.ipFS = strdup(config_get_string_value(config, "IP_FS"));
	//	configuracion.puertoFS = strdup(config_get_string_value(config, "PUERTO_FS"));
	//	configuracion.quantum = strdup(config_get_string_value(config, "QUANTUM"));
	//	configuracion.quantumSleep = strdup(config_get_string_value(config, "QUANTUM_SLEEP"));
	//	configuracion.algoritmo = strdup(config_get_string_value(config, "ALGORITMO"));
	//	configuracion.gradoMultiprog = strdup(config_get_string_value(config, "GRADO_MULTIPROG"));
	//	configuracion.semIDS = strdup(config_get_string_value(config, "SEM_IDS"));
	//	configuracion.semINIT = strdup(config_get_string_value(config, "SEM_INIT"));
	//	configuracion.sharedVars = strdup(config_get_string_value(config, "SHARED_VARS"));
	//	configuracion.stackSize = strdup(config_get_string_value(config, "STACK_SIZE"));

	configuracion.puertoEscucha = config_get_int_value(config,"PUERTO_ESCUCHA");

	//		printf("El puerto de la CPU es %s\n",configuracion.puertoCpu);
	//		printf("La IP de la Memoria es %s\n",configuracion.ipMemoria);
	//		printf("El puerto de la Memoria es %s\n",configuracion.puertoMemoria);
	//		printf("El puerto del Programa es %s\n",configuracion.puertoProg);
	//		printf("La IP del FS es %s\n",configuracion.ipFS);
	//		printf("El puero del FS es %s\n",configuracion.puertoFS);
	//		printf("El Quantum es %s\n",configuracion.quantum);
	//		printf("EL Quantum Sleep es %s\n",configuracion.quantumSleep);
	//		printf("El Algoritmo es %s\n",configuracion.algoritmo);
	//		printf("El grado multiprog es %s\n",configuracion.gradoMultiprog);
	//		printf("La Sem IDS es %s\n",configuracion.semIDS);
	//		printf("La Sem INIT es %s\n",configuracion.semINIT);
	//		printf("Las variables compartidas son %s\n",configuracion.sharedVars);
	//		printf("El tamaño del Stack es %s\n",configuracion.stackSize);

}

void manejarCpu(int i){

};

void manejarConsola(int i){

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

			break;

		case ES_CPU:

			puts("SE CONECTO UNA CPU");
			//conexion_cpu(socketCliente);

			FD_SET(socketCliente, &master_cpu);

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
	consolas = list_create();

}


int main(int arc, char * argv[]) {

	//Genera archivo log para poder escribir el trace de toda la ejecución
	logger = crearLog();

	inicializarListas();

	cargarConfiguracion();

	crearThreadAtenderConexiones();

	pthread_join(threadAtenderConexiones, NULL);

	return 0;

}
