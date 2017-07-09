#include "kernelHelper.h"

t_configuracion cargarConfiguracion() {

	t_config * config;
	t_configuracion configuracion;

	config = config_create("./config.txt");

	if(config == NULL){

		config = config_create("../config.txt");

	}  //TODO: Encontrar una forma menos villera de hacer esto. Seba dixit.

	configuracion.puertoCpu = config_get_int_value(config, "PUERTO_CPU");
	log_info(logger,"PUERTO_CPU = %d",configuracion.puertoCpu);

	configuracion.ipMemoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
	log_info(logger,"IP_MEMORIA = %s",configuracion.ipMemoria);

	configuracion.puertoProg = config_get_int_value(config, "PUERTO_PROG");
	log_info(logger,"PUERTO_PROG = %d",configuracion.puertoProg);

	configuracion.puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");
	log_info(logger,"PUERTO_MEMORIA = %d",configuracion.puertoMemoria);

	configuracion.ipFS = strdup(config_get_string_value(config, "IP_FS"));
	log_info(logger,"IP_FILESYSTEM = %s",configuracion.ipFS);

	configuracion.puertoFS = config_get_int_value(config, "PUERTO_FS");
	log_info(logger,"PUERTO_FILESYSTEM = %d",configuracion.puertoFS);

	configuracion.quantum = config_get_int_value(config, "QUANTUM");
	log_info(logger,"QUANTUM = %d",configuracion.quantum);

	configuracion.quantumSleep = strdup(config_get_string_value(config, "QUANTUM_SLEEP"));
	log_info(logger,"QUANTUM_SLEEP = %s",configuracion.quantumSleep);

	configuracion.algoritmo = strdup(config_get_string_value(config, "ALGORITMO"));
	log_info(logger,"ALGORITMO = %s",configuracion.algoritmo);

	configuracion.gradoMultiprog = config_get_int_value(config, "GRADO_MULTIPROG");
	log_info(logger,"GRADO_MULTIPROGRAMACION = %d",configuracion.gradoMultiprog);

	configuracion.semIDS = strdup(config_get_string_value(config, "SEM_IDS"));
	log_info(logger,"SEMAFOROS_IDS = %s",configuracion.semIDS);

	configuracion.semINIT = strdup(config_get_string_value(config, "SEM_INIT"));
	log_info(logger,"SEMAFOROS_INI = %s",configuracion.semINIT);

	configuracion.sharedVars = strdup(config_get_string_value(config, "SHARED_VARS"));
	log_info(logger,"SHARED_VARS = %s",configuracion.sharedVars);

	configuracion.stackSize = config_get_int_value(config, "STACK_SIZE");
	log_info(logger,"STACK_SIZE = %d",configuracion.stackSize);

	configuracion.puertoEscucha = config_get_int_value(config,"PUERTO_ESCUCHA");
	log_info(logger,"PUERTO_ESCUCHA = %d \n",configuracion.puertoEscucha);

	return configuracion;
}

void inicializarListas(){
	listaConsolas = list_create();
	listaCpuLibres = list_create();
	listaCpuOcupadas = list_create();
	listaProcesos = list_create();
	listaProcesosFinalizar = list_create();

	tablaHeap = list_create();
	tablaArchivosGlobal = list_create();
	tablaArchivosProceso = dictionary_create();


	maximoPID = 1;

	// Listas planificacion
	cola_new = queue_create();
	cola_exit = queue_create();
	cola_ready = list_create();
	cola_block = list_create();
	cola_exec = list_create();
}

void manejarNuevaConexion(int listener, int *fdmax){

	int socketCliente = aceptarCliente(listener);

	void * structRecibido;

	t_tipoEstructura tipoStruct;

	int resultado = socket_recibir(socketCliente, &tipoStruct, &structRecibido);
	if(resultado == -1 || tipoStruct != D_STRUCT_NUMERO){
		log_info(logger,"No se recibio correctamente a quien atendio en el kernel");

	}

	switch(((t_struct_numero *)structRecibido)->numero){
		case ES_CONSOLA:

			FD_SET(socketCliente, &master_consola);

			list_add(listaConsolas, (void*)socketCliente);

			log_info(logger,"La consola %d se conectó.", socketCliente);

			break;

		case ES_CPU:

			FD_SET(socketCliente, &master_cpu);

			list_add(listaCpuLibres, (void*)socketCliente);

			enviarConfiguracion(socketCliente,configuracion.stackSize);

			enviarConfiguracion(socketCliente,configuracion.quantum);

			log_info(logger,"El CPU %d se conectó.",socketCliente);

			break;

		default:
			log_info(logger,"No se acepta la conexion por ser de otro proceso");
			break;

	}

	if (socketCliente>*fdmax){
		*fdmax = socketCliente;
	}

	free(structRecibido);

}

void enviarConfiguracion(int socketCliente, int valor){

	t_struct_numero* parametro;

	parametro = malloc(sizeof(t_struct_numero));
	parametro->numero = valor;

	socket_enviar(socketCliente, D_STRUCT_NUMERO, parametro);
	free(parametro);

}

void manejarCpu(int i){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(i,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"El Cpu %d cerró la conexión.",i);
		removerClientePorCierreDeConexion(i,listaCpuLibres,&master_cpu);
	} else {
	}
};

void manejarConsola(int socketConsola){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(socketConsola,&tipoEstructura,&structRecibido) == -1) {

		log_info(logger,"La Consola %d cerró la conexión.",socketConsola);
		removerClientePorCierreDeConexion(socketConsola,listaConsolas,&master_consola);
		//TODO esta consola podría tener algun proceso en ejecucion hay que cancelarlo de la cpu

	} else {

		switch(tipoEstructura){
		case D_STRUCT_PROG: ;
			// La consola envia un programa para ejecutar

			int tamanio_programa = ((t_struct_programa*) structRecibido)->tamanio ;
			char * programa = malloc(tamanio_programa);
			memcpy(programa, ((t_struct_programa*) structRecibido)->buffer, tamanio_programa);

			inicializarProceso(socketConsola,programa,tamanio_programa);

			break;

		}

//		TODO para desconectar la consola hay que verificar si el programa esta en CPU,
//		cancelarlo eliminarlo de las colas, etc.
//		if(((t_struct_numero *)structRecibido)->numero == 243){
//			t_struct_numero confirmation_send;
//
//			if (programKiller(socketConsola) == 0){
//
//				confirmation_send.numero = 0;
//
//				socket_enviar(socketConsola, D_STRUCT_NUMERO, &confirmation_send);
//
//			} else {
//
//				confirmation_send.numero = 1;
//				socket_enviar(socketConsola, D_STRUCT_NUMERO, &confirmation_send);
//			}
//
//		}

		}

};

void administrarConexiones (){

	int socketServidor = crearServidor(configuracion.puertoEscucha);

	fd_set read_fds;

	//Elimino el contenido de los conjuntos de descriptores
	FD_ZERO(&master_cpu);
	FD_ZERO(&master_consola);
	FD_ZERO(&read_fds);

	// Declaro como descriptor de fichero mayor al socket que escucha
	int fdmax = socketServidor;
	int i;

	while(1){

		read_fds = combinar_master_fd(&master_cpu, &master_consola, fdmax); // se copia el master al temporal
		// Añado el descriptor del socket escucha al conjunto
		FD_SET(socketServidor, &read_fds);

		if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		}

		for(i = 0; i <= fdmax; i++) {

			//testea si un file descriptor es parte del set
			if (FD_ISSET(i, &read_fds)) {

				if(FD_ISSET(i, &master_cpu)){
					manejarCpu(i);
				}

				if(FD_ISSET(i, &master_consola)){
					manejarConsola(i);
				}

				if(i==socketServidor){
					manejarNuevaConexion(i, &fdmax);
				}

			}

		} //end_for

	}//end_while

}

int conectarAMemoria (){

	//Genera el socket cliente y lo conecta a la memoria
	int socketCliente = crearCliente(configuracion.ipMemoria,configuracion.puertoMemoria);
	log_info(logger,"Kernel conectado con la memoria");

	//Se realiza el handshake con la memoria
	t_struct_numero* es_kernel = malloc(sizeof(t_struct_numero));
	es_kernel->numero = ES_KERNEL;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_kernel);
	free(es_kernel);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	// Se recibe el tamaño de pagina de la memoria
	int resultado = socket_recibir(socketCliente, &tipoEstructura, &structRecibido);

	if(resultado == -1){
		log_info(logger,"No se recibió el tamaño de pagina");
	} else{
		tamanio_pagina = ((t_struct_numero*) structRecibido)->numero;
		free(structRecibido);
		log_info(logger,"El tamaño de pagina es %d",tamanio_pagina);
	}

	return socketCliente;

}

int conectarAFS(){

	//Genera el socket cliente y lo conecta a la memoria
	int socketCliente = crearCliente(configuracion.ipFS,configuracion.puertoFS);

	//Se realiza el handshake con la memoria
	t_struct_numero* es_kernel = malloc(sizeof(t_struct_numero));
	es_kernel->numero = ES_KERNEL;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_kernel);
	free(es_kernel);

	return socketCliente;

}

void crearThreadAtenderConexiones(){

	pthread_create(&threadAtenderConexiones, NULL, administrarConexiones, NULL);

}

void removerClientePorCierreDeConexion(int cliente, t_list* lista, fd_set *fdSet) {

	//Elimino el cliente de la lista
	bool _es_cliente_numero(int elemento) {
		return (elemento == cliente);
	}
	list_remove_by_condition(lista, (void*) _es_cliente_numero);

	//Elimino el cliente del fd_set
	FD_CLR(cliente,fdSet);

}

int programKiller(int i){

	log_info(logger,"La Consola %d se ha desconectado", i);
	return 1;
}


int obtener_pid(){

	int pid = maximoPID++;

	return pid;
}

t_struct_pcb* crearPCB(int PID){

	t_struct_pcb * pcb = malloc(sizeof(t_struct_pcb));

	pcb->PID=PID;
	pcb->programCounter=0;
	pcb->stackPointer=0;
	pcb->estado=E_NEW;
	pcb->indiceStack = list_create();
	pcb->indiceCodigo = list_create();
	//TODO REVISAR
	pcb->exitcode=0;
	pcb->cantRegistrosStack=0;

	registroStack * registro_stack = reg_stack_create();
	list_add(pcb->indiceStack,registro_stack);

	pcb->cantRegistrosStack++;

	crearArchivosPorProceso(pcb->PID);

	//TODO agrego una entrada en la tabla de info por proceso
//	entrada_info_proceso* entrada = malloc(sizeof(entrada_info_proceso));
//	entrada->pid = pcb->PID;
//	entrada->rafagas_ejecutadas = 0;
//	entrada->syscall_ejecutadas = 0;
//	entrada->sem_bloqueado_por = string_new();
//	entrada->cantidadAllocs =0;
//	entrada->cantidadLiberaciones = 0;
//	entrada->totalAllocado = 0;
//	entrada->totalLiberado = 0;
//	list_add(listaInfoProcesos, entrada);
//
//	pcb->banderas.cambio_proceso = false;
//	pcb->banderas.desconexion = false;
//	pcb->banderas.ejecutando = false;
//	pcb->banderas.llamada_a_funcion = false;
//	pcb->banderas.llamada_sin_retorno = false;
//	pcb->banderas.se_llamo_a_wait = false;
//	pcb->banderas.terminacion_anormal = false;
//	pcb->banderas.termino_programa = false;

	return pcb;

}


int solicitarSegmentoCodigo(int pid, int tamanioPrograma){

	// Pido a la memoria un segmento para el código
	t_struct_malloc* seg_codigo = malloc(sizeof(t_struct_malloc));
	seg_codigo->PID = pid;
	seg_codigo->tamano_segmento = tamanioPrograma;

	// Envío la solicitud de memoria con el tamaño del programa que quiero ejecutar
	int resultado = socket_enviar(socketMemoria, D_STRUCT_MALC, seg_codigo);

	if(resultado != 1){
		printf("No se pudo crear segmento de codigo\n");
		return -1;
	}
	free(seg_codigo);

	void * structRecibido;
	t_tipoEstructura tipoStruct;

	// Recibo la direccion del nuevo segmento de código
	if (socket_recibir(socketMemoria, &tipoStruct, &structRecibido) == -1){
		printf("No se pudo crear segmento de codigo\n");
		return -1;
	}

	int respuesta = ((t_struct_numero *) structRecibido)->numero;
	if (respuesta == MEMORIA_OK){
		free(structRecibido);
		return respuesta;
	}else{
		free(structRecibido);
		return -1;
	}
}

int solicitarSegmentoStack(int pid){
	// Pido a la memoria un segmento para el stack
	t_struct_malloc* seg_stack = malloc(sizeof(t_struct_malloc));
	seg_stack->PID = pid;
	seg_stack->tamano_segmento = tamanio_pagina * configuracion.stackSize;

	// Envío la solicitud de memoria con el tamaño de stack definido por conf
	int resultado = socket_enviar(socketMemoria, D_STRUCT_MALC, seg_stack);

	if(resultado != 1){
		log_info(logger,"No se pudo crear el segmento de stack");
		return -1;
	}
	free(seg_stack);

	void * structRecibido;
	t_tipoEstructura tipoStruct;

	// Recibo la direccion del nuevo segmento de stack
	if (socket_recibir(socketMemoria, &tipoStruct, &structRecibido) == -1){
		printf("No se pudo crear segmento de codigo\n");
		return -1;
	}

	int respuesta = ((t_struct_numero *) structRecibido)->numero;
	if (respuesta == MEMORIA_OK){
		free(structRecibido);
		return respuesta;
	}else{
		free(structRecibido);
		return -1;
	}
}

registroStack* reg_stack_create(){
	registroStack* reg = malloc(sizeof(registroStack));
	reg->cantidad_args = 0;
	reg->args = list_create();
	reg->cantidad_vars = 0;
	reg->vars = list_create();
	reg->retPos = 0;
	reg->retVar.offsetInstruccion = 0;
	reg->retVar.pagina = 0;
	reg->retVar.longitudInstruccion = 0;

	return reg;
}

void crearArchivosPorProceso(int PID){

	t_list * archivosProceso = list_create();
	t_registroArchivosProc archivo;

	list_add(archivosProceso, &archivo);
	list_add(archivosProceso, &archivo);
	list_add(archivosProceso, &archivo);

	dictionary_put(tablaArchivosProceso,PID,archivosProceso);
}

void agregarColaListos(t_struct_pcb* pcb){

	if(kernelPlanificando){
		pcb->estado=E_READY;
		list_add(cola_ready, pcb);
		cantidadTotalPID++;
	} else {
		log_info(logger, "Se recibe un PCB para ingresar a ready pero no se esta planificando");
		return;
	}
}

int reservarPaginas(t_struct_pcb * pcb, char* programa, int tamanioPrograma){

	if(solicitarSegmentoCodigo(pcb->PID,tamanioPrograma)==MEMORIA_OK
			&& solicitarSegmentoStack(pcb->PID)==MEMORIA_OK){

		int cantidadPaginasCodigo = tamanioPrograma / tamanio_pagina;

		if(tamanioPrograma%tamanio_pagina>0) cantidadPaginasCodigo++;

		pcb->paginaActualStack=cantidadPaginasCodigo;
		pcb->primerPaginaStack=pcb->paginaActualStack;
		pcb->stackPointer=cantidadPaginasCodigo;
		pcb->paginasStack=configuracion.stackSize;
		pcb->paginasCodigo=cantidadPaginasCodigo;

	}

	log_info(logger,"No se pudieron reservar las paginas al proceso %d",pcb->PID);
	return -1;

}

inicializarProceso(int socketConsola, char * programa, int tamanio_programa){

	t_struct_pcb * pcb = NULL;
	t_struct_numero* pid_struct = malloc(sizeof(t_struct_numero));

	if(cantidadTotalPID<configuracion.gradoMultiprog){

		pcb = crearPCB(obtener_pid());

		agregarColaListos(pcb);

		// Reservo las paginas para codigo y stack
		if (reservarPaginas(programa,pcb,tamanio_programa) == -1) {

			// Si no pude asignarlas aviso al a consola del rechazo
			pid_struct->numero = -1;
			socket_enviar(socketConsola, D_STRUCT_NUMERO, pid_struct);
			free(pid_struct);

		} else {

			// Si pude rechazarlas envio a la consola el process ID
			t_registroTablaProcesos * registroProceso = malloc(sizeof(t_registroTablaProcesos));
			registroProceso->PID = pcb->PID;
			registroProceso->socket = socketConsola;

			list_add(listaProcesos,registroProceso);

			pid_struct->numero = pcb->PID;
			socket_enviar(socketConsola, D_STRUCT_NUMERO, pid_struct);
			free(pid_struct);

			t_metadata_program* datosPrograma = metadata_desde_literal(programa);

			pcb->quantum=configuracion.quantum;
			pcb->quantum_sleep=configuracion.quantumSleep;
			pcb->programCounter=datosPrograma->instruccion_inicio;
			pcb->rafagas=0;
			pcb->tamanioIndiceEtiquetas=datosPrograma->etiquetas_size;

			memcpy(pcb->indiceEtiquetas,datosPrograma->etiquetas,datosPrograma->etiquetas_size);

			int i;
			for (i = 0;	i < datosPrograma->instrucciones_size;i++) {

				t_intructions instruccion =	datosPrograma->instrucciones_serializado[i];

				t_intructions* agregarInst = malloc(sizeof(t_intructions));

				agregarInst->offset = instruccion.offset;
				agregarInst->start = instruccion.start;

				list_add(pcb->indiceCodigo, agregarInst);
			}

			metadata_destruir(datosPrograma);

		}

	} else {

		// El grado de multiprogramacion no me permite admitir el proceso aun..
		pcb = crearPCB(obtener_pid());

		list_add(cola_new,pcb);

		t_registroTablaProcesos * registroProceso = malloc(sizeof(t_registroTablaProcesos));
		registroProceso->PID = pcb->PID;
		registroProceso->socket = socketConsola;

		list_add(listaProcesos,registroProceso);

		pid_struct->numero = pcb->PID;
		//TODO agregar nueva operacion para informar que el proceso esta en espera y manejar en consola
		socket_enviar(socketConsola, D_STRUCT_NUMERO, pid_struct);
		free(pid_struct);

	}

	if (list_size(cola_ready) > 0 && list_size(listaCpuLibres) > 0) {
		//TODO implementar.
		planificar(NULL);
	}

}
