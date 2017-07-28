#include "cpuHelper.h"

cargarConfiguracion(){

	t_config * config;

	pathConfiguracion = "./config.txt";
	config = config_create(pathConfiguracion);

	if(config == NULL){

		pathConfiguracion = "../config.txt";
		config = config_create(pathConfiguracion);
	}

	configuracion = malloc(sizeof(t_configuracion));

	configuracion->ipKernel = strdup(config_get_string_value(config, "IP_KERNEL"));
	log_info(logger,"IP_KERNEL = %s",configuracion->ipKernel);

	configuracion->puertoKernel = config_get_int_value(config, "PUERTO_KERNEL");
	log_info(logger,"PUERTO_KERNEL = %d",configuracion->puertoKernel);

	configuracion->ipMemoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
	log_info(logger,"IP_MEMORIA = %s",configuracion->ipMemoria);

	configuracion->puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");
	log_info(logger,"PUERTO_MEMORIA = %d",configuracion->puertoMemoria);

	config_destroy(config);

}

int conectarAKernel (){

	//Genera el socket cliente y lo conecta al kernel
	int socketCliente = crearCliente(configuracion->ipKernel,configuracion->puertoKernel);

	log_info(logger,"Socket cliente con kernel %d",socketCliente);

	//Se realiza el handshake con el kernel
	t_struct_numero* es_cpu = malloc(sizeof(t_struct_numero));
	es_cpu->numero = ES_CPU;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_cpu);
	free(es_cpu);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if(socket_recibir(socketCliente, &tipoEstructura, &structRecibido) == -1){
		log_info(logger,"No se recibió el tamaño de stack");
	} else{
		tamanio_stack = ((t_struct_numero*) structRecibido)->numero;

		log_info(logger,"El tamaño de stack es %d",tamanio_stack);
	}

	if(socket_recibir(socketCliente, &tipoEstructura, &structRecibido) == -1){
		log_info(logger,"No se recibió el quantum");
	} else{
		quantum = ((t_struct_numero*) structRecibido)->numero;

		log_info(logger,"El quantum para la planificacion es %d",quantum);
	}

	return socketCliente;
}

int conectarAMemoria (){

	//Genera el socket cliente y lo conecta a la memoria
	int socketCliente = crearCliente(configuracion->ipMemoria,configuracion->puertoMemoria);
	log_info(logger,"Socket cliente con la memoria %d",socketCliente);

	//Se realiza el handshake con la memoria
	t_struct_numero* es_cpu = malloc(sizeof(t_struct_numero));
	es_cpu->numero = ES_CPU;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_cpu);
	free(es_cpu);


	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if(socket_recibir(socketCliente, &tipoEstructura, &structRecibido) == -1){
		log_info(logger,"No se recibió el tamaño de pagina");
	} else{

		tamanio_pagina = malloc(sizeof(int32_t));

		tamanio_pagina = ((t_struct_numero*) structRecibido)->numero;

		log_info(logger,"El tamaño de pagina es %d",tamanio_pagina);
	}

	return socketCliente;

}

void recibirProcesoKernel(AnSISOP_funciones funcionesAnsisop,AnSISOP_kernel funciones_kernel){

	while(1) {

		void* structRecibido;
		t_tipoEstructura tipoEstructura;

		if (socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1) {

			salirErrorCpu();

		} else {

			if (tipoEstructura == D_STRUCT_PCB){

				pcbEjecutando = malloc(sizeof(t_struct_pcb));

				pcbEjecutando = (t_struct_pcb*) structRecibido;

				seguirEjecutando=true;
				cpuLibre=false;

				ejecutarProceso(funcionesAnsisop,funciones_kernel);

			}

		}
	}
}

void ejecutarProceso(AnSISOP_funciones funcionesAnsisop,AnSISOP_kernel funciones_kernel){

	log_info(logger,"Comienza a ejecutar el proceso %d", pcbEjecutando->PID);

	while(seguirEjecutando){

		char * instruccion = pedirSiguienteInstruccion();

		if (instruccion != NULL){

			char * instruccionLimpia = prepararInstruccion(instruccion);

			if (pcbEjecutando->programCounter >= pcbEjecutando->cantidadInstrucciones-1
					&& string_starts_with(instruccionLimpia,"end")){

				log_info(logger,"El proceso %d finalizo exitosamente",pcbEjecutando->PID);

				socket_enviar(socketKernel, D_STRUCT_PCB_FIN_OK, pcbEjecutando);

				free(instruccionLimpia);
				instruccionLimpia=NULL;

				salirProceso(0);
				return;
			}

			analizadorLinea(instruccionLimpia,&funcionesAnsisop,&funciones_kernel);

			if (stackOverflow){

				log_error(logger, "Hubo stackoverflow, aborto el proceso");

				pcbEjecutando->retornoPCB=D_STRUCT_ERROR_STACK_OVERFLOW;

				free(instruccionLimpia);
				instruccionLimpia = NULL;

				salirProceso();
				return;
			}
			if(finPrograma){

				log_info(logger, "El proceso finalizo exitosamente");

				socket_enviar(socketKernel, D_STRUCT_PCB_FIN_OK, pcbEjecutando);

				free(instruccionLimpia);
				instruccionLimpia = NULL;
				salirProceso(0);

				finPrograma = false;

				return;
			}

			pcbEjecutando->programCounter++;

			if(pcbEjecutando->retornoPCB!=0 || signalFinalizarCPU){
				salirProceso();
				return;
			}

			free(instruccionLimpia);
			instruccionLimpia = NULL;

			socket_enviar(socketKernel,D_STRUCT_FIN_INSTRUCCION,pcbEjecutando);

			t_tipoEstructura tipoEstructura;
			void * structRecibido;

			socket_recibir(socketKernel, &tipoEstructura, &structRecibido);

			if(tipoEstructura == D_STRUCT_ABORTAR_EJECUCION){

				socket_enviar(socketKernel,D_STRUCT_PCB,pcbEjecutando);
				salirProceso();
				return;

			} else if (tipoEstructura == D_STRUCT_FIN_QUANTUM){
				socket_enviar(socketKernel,D_STRUCT_PCB,pcbEjecutando);
				salirProceso();
				return;

			} else if (tipoEstructura == D_STRUCT_CONTINUAR_EJECUCION){

				pcbEjecutando->quantum_sleep = ((t_struct_numero*) structRecibido)->numero;
				usleep(pcbEjecutando->quantum_sleep * 1000);

			}

		} else {
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_INSTRUCCION;
			return;
		}

	}

}

void liberarPCB(){

	void liberarMemoria(void* liberar){
		free(liberar);
	}

	list_destroy_and_destroy_elements(pcbEjecutando->indiceCodigo,liberarMemoria);

	list_destroy_and_destroy_elements(pcbEjecutando->indiceStack,liberarMemoria);

	if(pcbEjecutando->tamanioIndiceEtiquetas > 0){
		free(pcbEjecutando->indiceEtiquetas);
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

	// Marco la cpu como disponible para un nuevo proceso
	cpuLibre=true;
	// Corto la condicion del while para no seguir leyendo instrucciones
	seguirEjecutando=false;

	if (pcbEjecutando->retornoPCB != 0){
		log_info(logger,"El proceso finalizara por el motivo de retorno pcb %d",pcbEjecutando->retornoPCB);
		socket_enviar(socketKernel, D_STRUCT_PCB_FIN_ERROR, pcbEjecutando);
	} else if (signalFinalizarCPU){
		log_info(logger,"El proceso finalizara por haberse realizado el signal SIGUSR1 en cpu");
		socket_enviar(socketMemoria, D_STRUCT_SIGUSR1, pcbEjecutando);
	}

	//Libero los recursos del pcb
	//liberarPCB();

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

char * prepararInstruccion(char * instruccion){

	char* instruccionLimpia = string_new();
	int size  = 0;

	while (instruccion[size] != '\0' && instruccion[size] != '\n') {

		char* caracter = malloc(sizeof(char) * 2);
		caracter[0] = instruccion[size];
		caracter[1] = '\0';

		string_append(&instruccionLimpia, caracter);

		free(caracter);
		size++;

	}

	instruccionLimpia[size] = '\0';

	return instruccionLimpia;
}

void inicializarEstructuras(){

	cpuLibre = true;
	stackOverflow = false;
	signalFinalizarCPU = false;
	finPrograma = false;
	seguirEjecutando = true;

	pcbEjecutando = malloc(sizeof(t_struct_pcb));

}

void manejarSignal(){
	log_info(logger, "Se recibió señal SIGUSR1 para desconectar la CPU");

	if (cpuLibre){
		desconectarCPU();
		exit(EXIT_FAILURE);
	}
	else{
		signalFinalizarCPU = true;
		log_info(logger, "El CPU se cerrará cuando finalice la rafaga en curso");
	}
}

void desconectarCPU(){
	log_info(logger, "Se desconecta la CPU");
	liberarRecursosCPU(); // Libero memoria utilizada
	close(socketMemoria);
	close(socketKernel);
}

char * pedirSiguienteInstruccion(){

	t_intructions * instruccion = (t_intructions*) list_get(pcbEjecutando->indiceCodigo,pcbEjecutando->programCounter);

	int inicio = instruccion->start;
	int offset = instruccion->offset;

	// Armo la direccion lógica con la instruccion
	t_struct_sol_lectura* direccion = malloc(sizeof(t_struct_sol_lectura));
	direccion->pagina = inicio / tamanio_pagina;
	direccion->offset = inicio % tamanio_pagina;
	direccion->contenido = offset;
	direccion->PID = pcbEjecutando->PID;

	log_info(logger,"Se solicita una instruccion PID %d Pagina %d Offset %d Contenido %d a memoria",
			direccion->PID, direccion->pagina, direccion->offset, direccion->contenido);

	socket_enviar(socketMemoria, D_STRUCT_LECT, direccion);
	//free(direccion);

	if(validarPedidoMemoria()){

		t_tipoEstructura tipoEstructura;
		void * structRecibido;

		if ( socket_recibir(socketMemoria, &tipoEstructura, &structRecibido) == -1){

			log_error(logger, "La memoria se desconecto del sistema");
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_MEMORIA;
			return NULL;

		} else {

			char* instruccion2 = ((t_struct_string*) structRecibido)->string;
			return instruccion2;
		}

	}

	return NULL;

}

bool validarPedidoMemoria(){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketMemoria, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "La memoria se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_MEMORIA;
		return false;

	} else {

		int estadoPedido = ((t_struct_numero*) structRecibido)->numero;

		//free(structRecibido);

		//Pedido rechazado por la memoria
		if(estadoPedido == MEMORIA_ERROR){
			return false;
		}
		return true;

	}
}

void salirErrorCpu(){
	close(socketKernel);
	close(socketMemoria);

	log_info(logger,"Se desconecta la CPU por un error");

	liberarRecursosCPU();
}
