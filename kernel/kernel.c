#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>

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
int socket_desc , client_sock , c , read_size;

//server: Direcciones del server (puerto, ip, etc).
//client: Direcciones del cliente.
struct sockaddr_in server , client;

//client_message[2000]: Buffer donde se almacena el mensaje recivido.
char client_message[2000];


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
	    server.sin_port = htons( 8300 ); //Especifica el puerto del servidor.

	    //Se liga (bind) el socket servidor con sus direcciones.
	    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        perror("Bind fallo. Error");
	        return 1;
	    }
	    puts("Bind realizado exitosamente.");

	    //Pone al servidor en modo listen (puede recibir llamados).
	    listen(socket_desc , 3);

	    puts("Servidor creado con exito.");
	    puts("Esperando por conexiones entrantes...");

	    //Se guarda el tamaño de la estructura "sockaddr_in" para ser utilizada en accept.
	    c = sizeof(struct sockaddr_in);

	    //Acepta la conexión de un cliente.
	    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
	    if (client_sock < 0)
	    {
	        perror("accept failed");
	        return 1;
	    }
	    puts("Connection accepted");

    return 0;
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
