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

//socket_desc: Socket del servidor.
//client_sock: Socker del cliente a aceptar.
//c: Tamaño de la estructura del socket cliente.
//read_size: Tamaño del mensaje leido.
//*new_sock: Socket del cliente, parametro para la creación del thread.
int socket_desc , client_sock , c , read_size, *new_sock, clientes[5], i;

//server: Direcciones del server (puerto, ip, etc).
//client: Direcciones del cliente.
struct sockaddr_in server , client;

//client_message[2000]: Buffer donde se almacena el mensaje recibido.
char client_message[2000];

void *atender_cliente(void *);

int recibirConexiones (int socketServidor){
	while (1) {

			struct sockaddr_storage addr;
			socklen_t addrlen = sizeof (addr);

			int clienteConectado = accept(socketServidor, (struct sockaddr *) &addr, &addrlen);

			if (clienteConectado<0){
				perror("accept");
				return EXIT_FAILURE;
			}

			pthread_t hiloCliente;

			pthread_create(&hiloCliente, NULL, atender_cliente, (void *) clienteConectado);
		}

}


void *atender_cliente(void *arg)
{
	int cl = (long)arg; //Socket cliente aceptado
	int tam_mens; //Tamaño del return de recv() (tamaño del mensaje recibido)
	char mens_cliente[500]; //Buffer donde se almacena el mensaje del cliente



	while(1){
		int j;

		tam_mens = recv(cl, mens_cliente, 500, 0); //Recibe mensaje del cliente (cl)
		puts(mens_cliente);

		for(j = 0; j<5; j++){
			write(clientes[j], mens_cliente, sizeof(mens_cliente));
		}

		if (tam_mens == -1){ //Reconoce error al recibir
			perror("No se pudo recibir mensaje\n");
			return EXIT_FAILURE;
		}

			}

	return NULL;

	}



void cargarConfiguracion(void) {
	t_config * config;

	config = config_create("./config.txt");

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

	recibirConexiones(socketServer);

	return 0;

}
