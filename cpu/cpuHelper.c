#include "cpuHelper.h"

t_configuracion cargarConfiguracion() {

	t_config * config;
	t_configuracion configuracion;

	config = config_create("./config.txt");

	if(config == NULL){

		config = config_create("../config.txt");

	}

	configuracion.ipKernel = strdup(config_get_string_value(config, "IP_KERNEL"));
	log_info(logger,"IP_KERNEL = %s",configuracion.ipKernel);

	configuracion.puertoKernel = config_get_int_value(config, "PUERTO_KERNEL");
	log_info(logger,"PUERTO_KERNEL = %d",configuracion.puertoKernel);

	configuracion.ipMemoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
	log_info(logger,"IP_MEMORIA = %s",configuracion.ipMemoria);

	configuracion.puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");
	log_info(logger,"PUERTO_MEMORIA = %d",configuracion.puertoMemoria);

	return configuracion;
}

int conectarAKernel (){

	//Genera el socket cliente y lo conecta al kernel
	int socketCliente = crearCliente(configuracion.ipKernel,configuracion.puertoKernel);

	//Se realiza el handshake con el kernel
	t_struct_numero* es_cpu = malloc(sizeof(t_struct_numero));
	es_cpu->numero = ES_CPU;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_cpu);
	free(es_cpu);

	return socketCliente;


}
