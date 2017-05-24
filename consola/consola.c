#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <logger.h>
#include <estructuras.h>

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

	crearLog("/CONSOLA");

	//Levanta la configuración del proceso consola
	cargarConfiguracion();

	//Genera el socket cliente y lo conecta al kernel
	int socketCliente = crearCliente(configuracion.ipKernel,configuracion.puertoKernel,logger);

	//Se realiza el handshake con el kernel
	t_struct_numero* es_consola = malloc(sizeof(t_struct_numero));
	es_consola->numero = ES_CONSOLA;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_consola);
	free(es_consola);

	return 0;

}
