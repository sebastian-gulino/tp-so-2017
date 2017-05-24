#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <logger.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <estructuras.h>
#include <commons/collections/list.h>

typedef struct config_t {

	char * ipKernel;
	int puertoKernel;

} t_configuracion;

t_configuracion configuracion;

char buffLog[80];

void cargarConfiguracion(void) {

	t_config * config;

	config = config_create("./config.txt");

	configuracion.ipKernel = strdup(config_get_string_value(config, "IP_KERNEL"));
	sprintf(buffLog,"IP_KERNEL = [%s]",configuracion.ipKernel);
	log_debug(logger,buffLog);

	configuracion.puertoKernel = config_get_int_value(config, "PUERTO_KERNEL");
	sprintf(buffLog,"PUERTO_KERNEL = [%d]",configuracion.puertoKernel);
	log_debug(logger,buffLog);
}

int main(int arc, char * argv[]) {

	//Genera archivo log para poder escribir el trace de toda la ejecución
	logger = malloc(sizeof(t_log));

	crearLog("/CPU");

	//Levanta la configuración del proceso CPaU
	cargarConfiguracion();

	//Genera el socket cliente y lo conecta al kernel
	int socketCliente = crearCliente(configuracion.ipKernel,configuracion.puertoKernel,logger);

	t_struct_numero* es_cpu = malloc(sizeof(t_struct_numero));
	es_cpu->numero = ES_CPU;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_cpu);
	free(es_cpu);

//	t_struct_string* unString = malloc(sizeof(t_struct_string));
//	unString->string = "Hola soy tu cpu 1";
//	socket_enviar(socketCliente, D_STRUCT_STRING, unString);

	return 0;

}
