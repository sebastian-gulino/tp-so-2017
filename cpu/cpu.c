#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <estructuras.h>

typedef struct config_t {

	char * ipKernel;
	int puertoKernel;

} t_configuracion;

t_configuracion configuracion;

t_log* logger;

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

t_log* crearLog(){

	char cad[80];
	char *directorioActual = getcwd(NULL, 0);

	strcat(cad,directorioActual);
	strcat(cad,"/cpu.log");

	logger = log_create(cad, "CPU", 0, LOG_LEVEL_TRACE);
	log_info(logger,"Comienza a ejecutar el proceso CPU");

	return logger;
}

int main(int arc, char * argv[]) {

	//Genera archivo log para poder escribir el trace de toda la ejecución
	t_log* logger = crearLog();

	//Levanta la configuración del proceso CPU
	cargarConfiguracion();

	//Genera el socket cliente y lo conecta al kernel
	int socketCliente = crearCliente(configuracion.ipKernel,configuracion.puertoKernel,logger);

	t_struct_numero* es_cpu = malloc(sizeof(t_struct_numero));
	es_cpu->numero = ES_CPU;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_cpu);
	free(es_cpu);

	return 0;

}
