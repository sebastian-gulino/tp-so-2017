#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>

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

typedef struct config_clientID{

	int fd;
	char * cosa;
} s_clientID;

t_configuracion configuracion;


//socket_desc: Socket del servidor.
//client_sock: Socker del cliente a aceptar.
//c: Tamaño de la estructura del socket cliente.
//read_size: Tamaño del mensaje leido.
//*new_sock: Socket del cliente, parametro para la creación del thread.
int socket_desc , client_sock , c , read_size, *new_sock, i;

s_clientID clientes[5];

//server: Direcciones del server (puerto, ip, etc).
//client: Direcciones del cliente.
struct sockaddr_in server , client;

////Buffer donde se almacena el mensaje del cliente
char mens_cliente[500];;

void *atender_cliente(void *);

//Funcion para crear el servidor.
int crearServidor(void){

	    //Se crea el socket
	    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	    if (socket_desc == -1)
	    {
	        printf("El socket no pudo ser creado.");
	        return EXIT_FAILURE;
	    }
	    puts("El socket fue creado exitosamente.");

	    //Se instancia la estructura "sockaddr_in" que contiene las direcciones del servidor.
	    server.sin_family = AF_INET; //Especifica familia de direcciones.
	    server.sin_addr.s_addr = INADDR_ANY; //Especifica que no se va a hacer bind a una IP especifica.
	    server.sin_port = htons( 8002); //Especifica el puerto del servidor.


	    printf("%s\n", inet_ntoa(server.sin_addr));
	    //Se liga (bind) el socket servidor con sus direcciones.
	    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        perror("Bind fallo. Error");
	        return 1;
	    }
	    puts("Bind realizado exitosamente.");

	    int yes = 1;
	    	if (setsockopt(socket_desc,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
	    	    perror("setsockopt");
	    	    exit(1);
	    	}

	    //Pone al servidor en modo listen (puede recibir llamados).
	    listen(socket_desc , 3);

	    puts("Servidor creado con exito.");
	    puts("Esperando por conexiones entrantes...");

	    i = 0;

	    while(1)
	    	{
	    		int cl; //Socket cliente ACEPTADO
	    		struct sockaddr addr; //Direcciones del cliente
	    		socklen_t addrlen = sizeof(addr); //Tamaño de las direcciones del cliente
	    		pthread_t threadID; //ID del thread creado

	    		cl = accept(socket_desc, &addr, &addrlen); //Se acepta el socket

	    		if(cl < 0)
	    			  {
	    			    perror("accept");
	    			    return EXIT_FAILURE;
	    			  }

	    		recv(cl, mens_cliente, 20, 0);


	    		if(verificarCliente(mens_cliente)==0){

	    			s_clientID clientID;


	    			clientID.fd = cl;
	    			clientID.cosa = mens_cliente;

	    			clientes[i]= clientID;

	    			i++;

	    			pthread_create(&threadID, NULL, atender_cliente, (void*)(long)cl); //Se crea el thread con el socket aceptado (cl) y la funcion atender_cliente que lo maneje
	    		}
	    	}


    return 0;
}

void *atender_cliente(void *arg)
{
	int cl = (long)arg; //Socket cliente aceptado
	int tam_mens; //Tamaño del return de recv() (tamaño del mensaje recibido)



	while(1){
		int j;

		tam_mens = recv(cl, mens_cliente, 20, 0); //Recibe mensaje del cliente (cl)
		puts(mens_cliente);

		for(j = 0; j<5; j++){
			write(clientes[j].fd, mens_cliente, sizeof(mens_cliente));
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
}

int verificarCliente(char*mensaje){

		puts("Credenciales verificadas...");
		return 0;

}

int main(int arc, char * argv[]) {

	cargarConfiguracion();

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

	int se;//Toma el return de crearServidor() para saber si hubo error en la creación del servidor.

	se = crearServidor();

		if(se){
			printf("El servidor no se ha creado");
			return EXIT_FAILURE;
		}

	return 0;

}
