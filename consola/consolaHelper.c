#include "consolaHelper.h"

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
	log_info(logger,"PUERTO_KERNEL = %d \n",configuracion.puertoKernel);

	return configuracion;
}

int conectarAKernel (){

	//Genera el socket cliente y lo conecta al kernel
	int socketCliente = crearCliente(configuracion.ipKernel,configuracion.puertoKernel);

	//Se realiza el handshake con el kernel
	t_struct_numero* es_consola = malloc(sizeof(t_struct_numero));
	es_consola->numero = ES_CONSOLA;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_consola);
	free(es_consola);

	return socketCliente;

}
