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

	// Se recibe el tamaño de stack
	int resultado = socket_recibir(socketCliente, &tipoEstructura, &structRecibido);

	if(resultado == -1){
		log_info(logger,"No se recibió el tamaño de stack");
	} else{
		tamanio_stack = ((t_struct_numero*) structRecibido)->numero;

		log_info(logger,"El tamaño de stack es %d",tamanio_stack);
	}

	resultado = socket_recibir(socketCliente, &tipoEstructura, &structRecibido);

	if(resultado == -1){
		log_info(logger,"No se recibió el tamaño de stack");
	} else{
		quantum = ((t_struct_numero*) structRecibido)->numero;

		log_info(logger,"El quantum para la planificacion es %d",quantum);
	}

	free(structRecibido);

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

void recibirProcesoKernel(AnSISOP_funciones funcionesAnsisop,AnSISOP_kernel funciones_kernel){

	while(1) {

		void* structRecibido;
		t_tipoEstructura tipoEstructura;

		if (socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1) {

			salirErrorCpu();

		} else {

			if (tipoEstructura == D_STRUCT_PCB){

				pcbEjecutando = (t_struct_pcb*) structRecibido;

				// TODO manejar desde la memoria
				// Envio a memoria el process id del proceso que voy a ejecutar
				t_struct_numero* pid = malloc(sizeof(t_struct_numero));
				pid->numero = pcbEjecutando->PID;
				socket_enviar(socketMemoria, D_STRUCT_PID, pid);
				free(pid);

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

			prepararInstruccion(instruccion);

			if (pcbEjecutando->programCounter >= pcbEjecutando->cantidadInstrucciones-1
					&& string_starts_with(instruccion,"end")){

				log_info(logger,"El proceso %d finalizo exitosamente",pcbEjecutando->PID);

				socket_enviar(socketKernel, D_STRUCT_PCB_FIN_OK, pcbEjecutando);

				free(instruccion);
				instruccion=NULL;

				salirProceso(0);
				return;
			}

			analizadorLinea(instruccion,&funcionesAnsisop,&funciones_kernel);

			if (stackOverflow){

				log_error(logger, "Hubo stackoverflow, aborto el proceso");

				pcbEjecutando->retornoPCB=D_STRUCT_ERROR_STACK_OVERFLOW;

				free(instruccion);
				instruccion = NULL;

				salirProceso();
				return;
			}
			if(finPrograma){

				log_info(logger, "El proceso finalizo exitosamente");

				socket_enviar(socketKernel, D_STRUCT_PCB_FIN_OK, pcbEjecutando);

				free(instruccion);
				instruccion = NULL;
				salirProceso(0);
				return;
			}

			pcbEjecutando->programCounter++;

			if(pcbEjecutando->retornoPCB!=0 || signalFinalizarCPU){
				salirProceso();
			}

			free(instruccion);
			instruccion = NULL;

			socket_enviar(socketKernel,D_STRUCT_FIN_INSTRUCCION,pcbEjecutando);

			t_tipoEstructura tipoEstructura;
			void * structRecibido;

			socket_recibir(socketMemoria, &tipoEstructura, &structRecibido);

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
		int indice;
		if(tamanioStack > 0){
			for (indice=0; indice<tamanioStack; indice++){

				registroStack* registro = (registroStack*) list_remove(pcbEjecutando->indiceStack, indice);

				if(registro != NULL){
					liberarRegistroStack(registro);
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

	// Marco la cpu como disponible para un nuevo proceso
	cpuLibre=true;
	// Corto la condicion del while para no seguir leyendo instrucciones
	seguirEjecutando=false;

	if (pcbEjecutando->retornoPCB != 0){
		socket_enviar(socketKernel, D_STRUCT_PCB_FIN_ERROR, pcbEjecutando);
	} else if (signalFinalizarCPU){
		socket_enviar(socketMemoria, D_STRUCT_SIGUSR1, pcbEjecutando);
	}

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

void prepararInstruccion(char * instruccion){

	char * auxiliar = instruccion;
	int indice = 0;
	while (*instruccion != '\0') {
		if (!iscntrl(*instruccion)) {
			if (indice == 0 && isdigit((int )*instruccion)) {
				++instruccion;
			} else {
				*auxiliar++ = *instruccion++;
				indice++;
			}
		} else {
			++instruccion;
		}
	}
	*auxiliar = '\0';
}

void inicializarEstructuras(){

	cpuLibre = true;
	stackOverflow = false;
	signalFinalizarCPU = false;
	finPrograma = false;
	seguirEjecutando = true;


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

	t_intructions * indice = pcbEjecutando->indiceCodigo;
	indice = indice + pcbEjecutando->programCounter;

	t_intructions * instruccion = indice;

	int start = instruccion->start;
	int offset = instruccion->offset;

	// Armo la direccion lógica con la instruccion
	t_struct_sol_lectura* direccion = malloc(sizeof(t_posicion_memoria));
	direccion->pagina = start / tamanio_pagina;
	direccion->offset = start % tamanio_pagina;
	direccion->contenido = offset;
	direccion->PID = pcbEjecutando->PID;

	//TODO en memoria ante estos pedidos me va a tener que devolver un numero para indicar si es valido y luego la instruccion si corresponde
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

			char* instruccion = ((t_struct_string*) structRecibido)->string;
			return instruccion;
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
