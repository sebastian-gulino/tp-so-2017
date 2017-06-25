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

void recibirProcesoKernel(){

	//TODO ver de pasar a una variable para el while
	while(1) {

		void* structRecibido;
		t_tipoEstructura tipoEstructura;

		if (socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1) {

			//TODO manejar desconexion del kernel

		} else {

			if (tipoEstructura == D_STRUCT_PCB){

				pcbEjecutando = (t_struct_pcb*) structRecibido;

				// Envio a memoria el process id del proceso que voy a ejecutar
				t_struct_numero* pid = malloc(sizeof(t_struct_numero));
				pid->numero = pcbEjecutando->PID;
				socket_enviar(socketMemoria, D_STRUCT_PID, pid);
				free(pid);

				ejecutarProceso();

			}
			//TODO ver si hay que recibir algun otro tipo de mensaje del kernel

		}
	}
}

void ejecutarProceso(){
	log_info(logger,"Comienza a ejecutar el proceso %d", pcbEjecutando->PID);

	// TODO el kernel debe enviarme el quantum cuando conecto
	int quantumDisponible = configuracion.puertoKernel;

	while(quantumDisponible > 0){

		char * instruccion = pedirSiguienteInstruccion();

		if (instruccion != NULL){

			//TODO limpiar instruccion

			if (pcbEjecutando->programCounter >= pcbEjecutando->cantidadInstrucciones-1
					&& string_starts_with(instruccion,"end")){

				log_info(logger,"El proceso %d finalizo exitosamente",pcbEjecutando->PID);

				// Envio nuevamente el pcb al kernel
				socket_enviar(socketKernel,D_STRUCT_PCB,pcbEjecutando);

				free(instruccion);


			}

		}

	}

}

void liberarPCB(){

	//Hago los free de memoria correspondientes para liberar lo contenido en el pcb
	if(pcbEjecutando->indiceCodigo != NULL){
		free(pcbEjecutando->indiceCodigo);
		pcbEjecutando->indiceCodigo = NULL;
	}

	if(pcbEjecutando->indiceEtiquetas != NULL){
		free(pcbEjecutando->indiceEtiquetas);
		pcbEjecutando->indiceEtiquetas = NULL;
	}

	if(pcbEjecutando->indiceStack != NULL){
		int tamanioStack = list_size(pcbEjecutando->indiceStack);
		if(tamanioStack > 0){
			for (int indice=0; indice<tamanioStack; indice++){
				registroStack* registroStack = registroStack* list_remove(pcbEjecutando->indiceStack, indice);
				if(registroStack != NULL){
					liberarRegistroStack(registroStack);
				}
			}
		}
		free(pcbEjecutando->indiceStack);
		pcbEjecutando->indiceStack=NULL;

	}

	if(pcbEjecutando!=NULL){
		free(pcbEjecutando);
		pcbEjecutando=NULL;
	}

}


void liberarRegistroStack(registroStack* registroStack){

	if(registroStack != NULL){

		int indice;

		if(registroStack->args != NULL){

			for(indice = 0; indice < list_size(registroStack->args); indice++){
				t_variable* arg = list_remove(registroStack->args, indice);
				if(arg != NULL){
					free(arg); arg = NULL;
				}
			}
			free(registroStack->args); registroStack->args = NULL;
		}

		if(registroStack->vars != NULL){

			for(indice = 0; indice < list_size(registroStack->vars); indice++){
				t_variable* var = list_remove(registroStack->vars, indice);
				if(var != NULL){
					free(var); var = NULL;
				}
			}
			free(registroStack->vars); registroStack->vars = NULL;
		}

		free(registroStack);
		registroStack = NULL;
	}
}


void salirProceso(){
	//Marco la cpu como disponible para un nuevo proceso
	cpuLibre=true;

	//Libero los recursos del pcb
	liberarPCB();

	if (signalFinalizarCPU){

		close(socketKernel);
		close(socketMemoria);
		log_info(logger,"Se solicito finalizar el proceso CPU por medio de una señal");
		liberarRecursosCPU();

		exit(EXIT_SUCCESS);
	}
}

void liberarRecursosCPU(){
	log_destroy(logger);
	if(pcbEjecutando!=NULL){
		liberarPCB();
	}
}
