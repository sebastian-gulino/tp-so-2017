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

	log_info(logger,"Socket cliente con kernel %d",socketCliente);

	//Se realiza el handshake con el kernel
	t_struct_numero* es_cpu = malloc(sizeof(t_struct_numero));
	es_cpu->numero = ES_CPU;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_cpu);
	free(es_cpu);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	// Se recibe el tamaño de stack
	int resultado = socket_recibir(socketCliente, &tipoEstructura, &structRecibido);

	if(resultado == -1){
		log_info(logger,"No se recibió el tamaño de stack");
	} else{
		tamanio_stack = ((t_struct_numero*) structRecibido)->numero;

		log_info(logger,"El tamaño de stack es %d",tamanio_stack);
	}

	return socketCliente;
}

int conectarAMemoria (){

	//Genera el socket cliente y lo conecta a la memoria
	int socketCliente = crearCliente(configuracion.ipMemoria,configuracion.puertoMemoria);

	//Se realiza el handshake con la memoria
	t_struct_numero* es_cpu = malloc(sizeof(t_struct_numero));
	es_cpu->numero = ES_CPU;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_cpu);
	free(es_cpu);


	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	// Se recibe el tamaño de pagina de la memoria
	int resultado = socket_recibir(socketCliente, &tipoEstructura, &structRecibido);

	if(resultado == -1){
		log_info(logger,"No se recibió el tamaño de pagina");
	} else{
		tamanio_pagina = ((t_struct_numero*) structRecibido)->numero;

		log_info(logger,"El tamaño de pagina es %d",tamanio_pagina);
	}

	return socketCliente;

}
