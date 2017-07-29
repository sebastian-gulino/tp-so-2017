#include "kernelHelper.h"

void cargarConfiguracion() {

	t_config * config;

	pathConfiguracion = "./config.txt";
	config = config_create(pathConfiguracion);

	if(config == NULL){

		pathConfiguracion = "../config.txt";
		config = config_create(pathConfiguracion);
	}

	configuracion = malloc(sizeof(t_configuracion));

	configuracion->puertoCpu = config_get_int_value(config, "PUERTO_CPU");
	log_info(logger,"PUERTO_CPU = %d",configuracion->puertoCpu);

	configuracion->ipMemoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
	log_info(logger,"IP_MEMORIA = %s",configuracion->ipMemoria);

	configuracion->puertoProg = config_get_int_value(config, "PUERTO_PROG");
	log_info(logger,"PUERTO_PROG = %d",configuracion->puertoProg);

	configuracion->puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");
	log_info(logger,"PUERTO_MEMORIA = %d",configuracion->puertoMemoria);

	configuracion->ipFS = strdup(config_get_string_value(config, "IP_FS"));
	log_info(logger,"IP_FILESYSTEM = %s",configuracion->ipFS);

	configuracion->puertoFS = config_get_int_value(config, "PUERTO_FS");
	log_info(logger,"PUERTO_FILESYSTEM = %d",configuracion->puertoFS);

	configuracion->quantum = config_get_int_value(config, "QUANTUM");
	log_info(logger,"QUANTUM = %d",configuracion->quantum);

	configuracion->quantumSleep = config_get_int_value(config, "QUANTUM_SLEEP");
	log_info(logger,"QUANTUM_SLEEP = %d",configuracion->quantumSleep);

	configuracion->algoritmo = strdup(config_get_string_value(config, "ALGORITMO"));
	log_info(logger,"ALGORITMO = %s",configuracion->algoritmo);

	configuracion->gradoMultiprog = config_get_int_value(config, "GRADO_MULTIPROG");
	log_info(logger,"GRADO_MULTIPROGRAMACION = %d",configuracion->gradoMultiprog);

	configuracion->semIDS = config_get_array_value(config, "SEM_IDS");

	configuracion->semINIT = config_get_array_value(config, "SEM_INIT");

	configuracion->sharedVars = config_get_array_value(config, "SHARED_VARS");

	configuracion->stackSize = config_get_int_value(config, "STACK_SIZE");
	log_info(logger,"STACK_SIZE = %d",configuracion->stackSize);

	configuracion->puertoEscucha = config_get_int_value(config,"PUERTO_ESCUCHA");
	log_info(logger,"PUERTO_ESCUCHA = %d \n",configuracion->puertoEscucha);

	cargarVariablesCompartidas();
	cargarSemaforos();

	config_destroy(config);

}

void cargarVariablesCompartidas(){


	listaVarCompartidas = list_create();

	int indice = 0;
	t_struct_var_compartida * varCompartida;

	while (configuracion->sharedVars[indice] != NULL){
		varCompartida = malloc(sizeof(t_struct_var_compartida));

		varCompartida->nombre = (t_nombre_variable*) configuracion->sharedVars[indice];
		varCompartida->valor=0;

		list_add(listaVarCompartidas, varCompartida);
		indice ++;
	}

	log_info(logger,"Se cargaron %d variables compartidas",indice);
}

void cargarSemaforos(){

	listaSemaforos = list_create();

	int indice = 0;
	t_struct_semaforo * semaforo;

	while (configuracion->semIDS[indice] != NULL){
		semaforo = malloc(sizeof(t_struct_semaforo)+1);

		char* nombreSemaforo = configuracion->semIDS[indice];
		string_append(&nombreSemaforo, "\0");

		semaforo->nombre = &(configuracion->semIDS[indice]);
		semaforo->valor=atoi(configuracion->semINIT[indice]);

		list_add(listaSemaforos,semaforo);
		indice++;
	}

	log_info(logger,"Se cargaron %d semaforos",indice);
}

void actualizarQuantumSleep() {

	t_config* config = config_create(pathConfiguracion);

	configuracion->quantumSleep = 0;
	configuracion->quantumSleep = config_get_int_value(config, "QUANTUM_SLEEP");

	log_info(logger,"El nuevo valor de quantum sleep es %d",configuracion->quantumSleep);

	config_destroy(config);

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

	listaInformacionProcesos = list_create();

	maximoPID = 1;
	kernelPlanificando = true;
	quantumSleepActualizado = false;

	// Listas planificacion
	cola_new = list_create();
	cola_exit = list_create();
	cola_ready = list_create();
	cola_block = list_create();
	cola_exec = list_create();

	sizeInotify =  (1024 * sizeof(struct inotify_event) + 40);
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

			t_cpu * nuevaCPU = malloc(sizeof(t_cpu));
			nuevaCPU->socket=socketCliente;
			nuevaCPU->PID=-1;
			nuevaCPU->quantum=0;

			list_add(listaCpuLibres, nuevaCPU);

			enviarConfiguracion(socketCliente,configuracion->stackSize);

			enviarConfiguracion(socketCliente,configuracion->quantum);

			log_info(logger,"El CPU %d se conectó.",socketCliente);

			if(list_size(cola_ready)>0){
				ejecutarPlanificacion(0);
			}

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

void manejarCpu(int socketCPU){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;


	if (socket_recibir(socketCPU,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"El Cpu %d cerró la conexión.",socketCPU);
		matarProcesoEnEjecucion(socketCPU, true);
		removerClientePorCierreDeConexion(socketCPU,&master_cpu);

	} else {

		switch(tipoEstructura){
		case D_STRUCT_SIGUSR1: ;

			log_info(logger,"La CPU %d recibio un signal SIGUSR1",socketCPU);

			t_struct_pcb * pcbSIGUSR = ((t_struct_pcb*) structRecibido);
			actualizarPCBExec(pcbSIGUSR);
			matarProcesoEnEjecucion(socketCPU, true);

			break;

		case D_STRUCT_PCB_FIN_ERROR: ;

			t_struct_pcb * pcbFinError = ((t_struct_pcb*) structRecibido);

			log_info(logger,"La CPU %d informa que el PID %d finaliza con error",socketCPU, pcbFinError->PID);

			actualizarPCBExec(pcbFinError);
			matarProcesoEnEjecucion(socketCPU, false);

			break;

		case D_STRUCT_PCB_FIN_OK: ;

			t_struct_pcb * pcbFinOk = ((t_struct_pcb*) structRecibido);

			log_info(logger,"La CPU %d informa que el PID %d finaliza OK",socketCPU, pcbFinOk->PID);

			finalizarProcesoOK(socketCPU, pcbFinOk);

			break;

		case D_STRUCT_OBTENER_COMPARTIDA: ;

			char * obtenerVarCompartida = ((t_struct_string*) structRecibido)->string ;

			log_info(logger,"La CPU %d solicita el valor de la variable compartida %s",socketCPU, obtenerVarCompartida);

			obtenerVariableCompartida(socketCPU, obtenerVarCompartida);

			break;

		case D_STRUCT_GRABAR_COMPARTIDA: ;

			t_struct_var_compartida * grabarVarCompartida = ((t_struct_var_compartida*) structRecibido);

			log_info(logger,"La CPU %d quiere grabar el valor %d en la variable compartida %s",socketCPU,
					grabarVarCompartida->valor,grabarVarCompartida->nombre);

			grabarVariableCompartida(socketCPU, grabarVarCompartida);

			break;

		case D_STRUCT_WAIT: ;

			// La cpu quiere realizar wait de un semaforo
			char * waitSemaforo = ((t_struct_string*) structRecibido)->string ;

			realizarWaitSemaforo(socketCPU,waitSemaforo);

			break;

		case D_STRUCT_SIGNAL: ;

			// La cpu quiere realizar signal de un semaforo
			char * signalSemaforo = ((t_struct_string*) structRecibido)->string ;
			realizarSignalSemaforo(socketCPU,signalSemaforo);

			break;

		case D_STRUCT_FIN_INSTRUCCION: ;

			ejecutarPlanificacion(socketCPU);

			break;

		case D_STRUCT_ARCHIVO_ABR: ;

			// La cpu quiere abrir un archivo
			t_struct_archivo * archivoAbrir = ((t_struct_archivo*) structRecibido);

			log_info(logger,"La CPU %d quiere abrir un archivo para el proceso %d",socketCPU,archivoAbrir->pid);

			abrirArchivo(socketCPU,archivoAbrir);

			break;

		case D_STRUCT_ARCHIVO_BOR: ;

			// La cpu quiere borrar un archivo
			t_struct_sol_lectura * archivoBorrar = ((t_struct_sol_lectura*) structRecibido);

			log_info(logger,"La CPU %d quiere borrar un archivo para el proceso %d",socketCPU,archivoBorrar->PID);

			borrarArchivo(socketCPU,archivoBorrar);

			break;

		case D_STRUCT_ARCHIVO_CER: ;

			// La cpu quiere cerrar un archivo
			t_struct_sol_lectura * archivoCerrar = ((t_struct_sol_lectura*) structRecibido);

			log_info(logger,"La CPU %d quiere cerrar un archivo para el proceso %d",socketCPU,archivoCerrar->PID);

			cerrarArchivo(socketCPU, archivoCerrar);

			break;

		case D_STRUCT_ARCHIVO_MOV: ;

			// La cpu quiere mover el cursor dentro de un archivo
			t_struct_sol_lectura * archivoMover = ((t_struct_sol_lectura*) structRecibido);

			log_info(logger,"La CPU %d quiere mover el cursor para un archivo para el proceso %d",socketCPU,archivoMover->PID);

			moverCursorArchivo(socketCPU, archivoMover);

			break;

		case D_STRUCT_ARCHIVO_ESC: ;

			// La cpu quiere escribir en un archivo
			t_struct_archivo * archivoEscribir = ((t_struct_archivo*) structRecibido);

			log_info(logger,"La CPU %d quiere escribir en un archivo para el proceso %d",socketCPU,archivoEscribir->pid);

			escribirArchivo(socketCPU, archivoEscribir);

			break;

		case D_STRUCT_ARCHIVO_LEC: ;

			// La cpu quiere escribir en un archivo
			t_struct_archivo * archivoLeer = ((t_struct_archivo*) structRecibido);

			log_info(logger,"La CPU %d quiere leer en un archivo para el proceso %d",socketCPU,archivoLeer->pid);

			leerArchivo(socketCPU, archivoLeer);

			break;

		case D_STRUCT_SOL_HEAP: ;

			t_struct_sol_heap * solicitudHeap = ((t_struct_sol_heap*) structRecibido);

			log_info(logger,"La CPU %d solicita alocar heap para el proceso %d",socketCPU,solicitudHeap->pid);

			reservarHeap(socketCPU, solicitudHeap);

			break;

		case D_STRUCT_LIB_HEAP: ;

			t_struct_sol_heap * heapLiberar = ((t_struct_sol_heap*) structRecibido);

			log_info(logger,"La CPU %d solicita liberar heap para el proceso %d",socketCPU,heapLiberar->pid);

			liberarHeap(socketCPU, heapLiberar);

			break;

		}

	}
};

void manejarConsola(int socketConsola){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(socketConsola,&tipoEstructura,&structRecibido) == -1) {

		log_info(logger,"La Consola %d cerró la conexión.",socketConsola);
		//TODO incluir para eliminar de la lista de consolas
		abortarPrograma(socketConsola,false);
		removerClientePorCierreDeConexion(socketConsola,&master_consola);

	} else {

		switch(tipoEstructura){
		case D_STRUCT_PROG: ;
			// La consola envia un programa para ejecutar
			int tamanio_programa = ((t_struct_programa*) structRecibido)->tamanio ;
			char * programa = malloc(tamanio_programa+1);
			memcpy(programa, ((t_struct_programa*) structRecibido)->buffer, tamanio_programa);

			inicializarProceso(socketConsola,programa,tamanio_programa);

			break;

		case D_STRUCT_FIN_PROG: ;
			// La consola envia finalizar un programa
			abortarPrograma(socketConsola,true);

			break;

		}

	}

};

void administrarConexiones (){

	int fdEntrada = 0;

	int inotifyDescriptor = inotify_init();
	if (inotifyDescriptor < 0) {
		perror("inotify_init");
	}

	watchInotify = inotify_add_watch(inotifyDescriptor,pathConfiguracion,IN_MODIFY);

	int socketServidor = crearServidor(configuracion->puertoEscucha);

	fd_set read_fds;
	fd_set inotify;
	fd_set STDIN;

	//Elimino el contenido de los conjuntos de descriptores
	FD_ZERO(&master_cpu);
	FD_ZERO(&master_consola);
	FD_ZERO(&inotify);
	FD_ZERO(&STDIN);
	FD_ZERO(&read_fds);

	FD_SET(inotifyDescriptor,&inotify);
	FD_SET(fdEntrada,&STDIN);

	// Declaro como descriptor de fichero mayor al socket que escucha
	int fdmax = socketServidor;
	int i;

	while(1){

		read_fds = combinar_master_fd(&master_cpu, &master_consola, fdmax); // se copia el master al temporal
		// Añado el descriptor del socket escucha al conjunto
		FD_SET(socketServidor, &read_fds);
		FD_SET(inotifyDescriptor,&read_fds);
		FD_SET(fdEntrada,&read_fds);

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

				if(FD_ISSET(i, &inotify)){
					modificacionArchConf(i);
				}

				if(FD_ISSET(i, &STDIN)){
					mensajeConsolaKernel(i);
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
	int socketCliente = crearCliente(configuracion->ipMemoria,configuracion->puertoMemoria);
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
	int socketCliente = crearCliente(configuracion->ipFS,configuracion->puertoFS);

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

void removerClientePorCierreDeConexion(int cliente, fd_set *fdSet) {

	//Elimino el cliente del fd_set
	FD_CLR(cliente,fdSet);

}

int obtener_pid(){

	int pid = maximoPID++;

	return pid;
}

void modificacionArchConf(int fdInotify) {
	char buffer[sizeInotify];
	int length_inotify = read(fdInotify, buffer, sizeInotify);
	int offset=0;

	if (length_inotify < 0) {
		log_error(logger,"Error al leer el archivo de Configuracion");
	}
	while (length_inotify > offset) {

		struct inotify_event *event = (struct inotify_event *) &buffer[offset];

		t_config * config = config_create(pathConfiguracion);

			if (event->mask & IN_MODIFY) {
				if (config && config_has_property(config, "QUANTUM_SLEEP")) {
					actualizarQuantumSleep();
				}
			}
			offset += sizeInotify + event->len;

		free(config);

	}
}

t_struct_pcb* crearPCB(int PID, t_struct_pcb* pcb){

	pcb->PID=PID;
	pcb->programCounter=0;
	pcb->stackPointer=0;
	pcb->estado=E_NEW;
	pcb->indiceStack = list_create();
	pcb->indiceCodigo = list_create();
	pcb->exitcode=99;
	pcb->cantRegistrosStack=0;
	pcb->retornoPCB=0;

	registroStack* registro_stack = malloc(sizeof(registroStack));
	registro_stack->args = list_create();
	registro_stack->vars = list_create();
	registro_stack->retPos = 0;
	registro_stack->retVar.offsetInstruccion = 0;
	registro_stack->retVar.pagina = 0;
	registro_stack->retVar.longitudInstruccion = 0;

	list_add(pcb->indiceStack,registro_stack);

	pcb->cantRegistrosStack++;

	crearArchivosPorProceso(pcb->PID);

	crearInformacionProcesoInicial(pcb->PID);

	log_info(logger, "Pudo crearse correctamente el PCB para el proceso PID: %d",PID);

	return pcb;

}

int solicitarSegmentoCodigoStack(int pid, int tamanio){

	// Pido a la memoria un segmento para el código
	t_struct_malloc* segmento = malloc(sizeof(t_struct_malloc));
	segmento->PID = pid;
	segmento->tamano_segmento = tamanio;

	// Envío la solicitud de memoria con el tamaño del programa que quiero ejecutar
	int resultado = socket_enviar(socketMemoria, D_STRUCT_MALC, segmento);

	if(resultado != 1){
		printf("No se pudo crear segmento de codigo\n");
		return -1;
	}
	free(segmento);

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
	seg_stack->tamano_segmento = tamanio_pagina * configuracion->stackSize;

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

void enviarCodigoMemoria(char * programa,int tamanioPrograma, t_struct_pcb * pcb){

	int cantidadPaginasCodigo = tamanioPrograma / tamanio_pagina;

	if(tamanioPrograma%tamanio_pagina>0) cantidadPaginasCodigo++;

	int indice;
	int cantCodigoEnviado=0;
	int cantCodigoPendiente=tamanioPrograma;
	int tamanioEnvio;

	for(indice=0; indice < cantidadPaginasCodigo; indice++ ){

		char * codigoPendiente = malloc(tamanioPrograma);
		codigoPendiente = string_new();
		char * codigoEnviar = malloc(tamanioPrograma);
		codigoEnviar = string_new();

		tamanioEnvio = cantCodigoPendiente>tamanio_pagina ? tamanio_pagina : cantCodigoPendiente;

		t_struct_sol_escritura * escrituraCodigo = malloc(sizeof(t_struct_sol_escritura));
		escrituraCodigo->PID=pcb->PID;
		escrituraCodigo->offset=0;
		escrituraCodigo->pagina=indice;
		escrituraCodigo->tamanio=tamanioEnvio;
		escrituraCodigo->contenido = malloc(tamanioEnvio);

		codigoPendiente = string_substring_from(programa,cantCodigoEnviado);
		codigoEnviar = string_substring_until(codigoPendiente,tamanioEnvio);

		memcpy(escrituraCodigo->contenido,codigoEnviar,tamanioEnvio);

		socket_enviar(socketMemoria,D_STRUCT_ESCRITURA_CODIGO,escrituraCodigo);

		free(codigoPendiente);
		free(codigoEnviar);

		cantCodigoEnviado += tamanioEnvio;
		cantCodigoPendiente -= tamanioEnvio;

	}

	log_info(logger,"Se envio el codigo del proceso %d a la memoria",pcb->PID);
}

registroStack* reg_stack_create(){
	registroStack* reg = malloc(sizeof(registroStack));
	reg->args = list_create();
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

	dictionary_put(tablaArchivosProceso,string_itoa(PID),archivosProceso);
}

void agregarColaListos(t_struct_pcb* pcb){

	if(kernelPlanificando){
		pcb->estado=E_READY;
		list_add(cola_ready, pcb);
		cantidadTotalPID++;
		log_info(logger, "Se agrega el PID: %d a la cola de listos, cantidad total de procesos en sistema %d",pcb->PID,cantidadTotalPID);
	} else {
		log_info(logger, "Se recibe un PCB para ingresar a ready pero no se esta planificando");
		return;
	}
}

int reservarPaginas(t_struct_pcb * pcb, char* programa, int tamanioPrograma){

	int cantidadPaginasCodigo = tamanioPrograma / tamanio_pagina;

	if(tamanioPrograma%tamanio_pagina>0) cantidadPaginasCodigo++;

	int tamanioReservar = (cantidadPaginasCodigo + configuracion->stackSize) * tamanio_pagina;

	if(solicitarSegmentoCodigoStack(pcb->PID, tamanioReservar)==MEMORIA_OK){

		pcb->paginaActualStack=cantidadPaginasCodigo;
		pcb->primerPaginaStack=pcb->paginaActualStack;
		pcb->stackPointer=0;
		pcb->paginasStack=configuracion->stackSize;
		pcb->paginasCodigo=cantidadPaginasCodigo;

		log_info(logger, "Se pudieron reservar paginas para codigo y stack del proceso PID: %d",pcb->PID);

		return 1;

	}

	log_info(logger,"No se pudieron reservar las paginas al proceso %d",pcb->PID);
	return -1;

}

void inicializarProceso(int socketConsola, char * programa, int tamanio_programa){

	log_info(logger, "Se recibio un proceso de la consola %d",socketConsola);
	t_struct_pcb * pcb = malloc(sizeof(t_struct_pcb));
	t_struct_numero* pid_struct = malloc(sizeof(t_struct_numero));

	if(cantidadTotalPID<configuracion->gradoMultiprog){

		pcb = crearPCB(obtener_pid(),pcb);

		agregarColaListos(pcb);

		// Reservo las paginas para codigo y stack
		if (reservarPaginas(pcb,programa,tamanio_programa) == -1) {

			// Si no pude asignarlas aviso al a consola del rechazo
			pid_struct->numero = -1;
			socket_enviar(socketConsola, D_STRUCT_NUMERO, pid_struct);
			free(pid_struct);

		} else {

			// Si pude reservarlas envio a la consola el process ID
			t_registroTablaProcesos * registroProceso = malloc(sizeof(t_registroTablaProcesos));
			registroProceso->PID = pcb->PID;
			registroProceso->socket = socketConsola;

			list_add(listaProcesos,registroProceso);

			pid_struct->numero = pcb->PID;
			socket_enviar(socketConsola, D_STRUCT_NUMERO, pid_struct);
			free(pid_struct);

			enviarCodigoMemoria(programa,tamanio_programa,pcb);

			t_metadata_program* datosPrograma = malloc(sizeof(t_metadata_program));
			datosPrograma = metadata_desde_literal(programa);

			pcb->quantum_sleep=configuracion->quantumSleep;
			pcb->programCounter=datosPrograma->instruccion_inicio;
			pcb->tamanioIndiceEtiquetas=datosPrograma->etiquetas_size;
			pcb->cantidadInstrucciones=datosPrograma->instrucciones_size;

			char * etiquetas = malloc(datosPrograma->etiquetas_size);

			memcpy(etiquetas,datosPrograma->etiquetas,datosPrograma->etiquetas_size);
			pcb->indiceEtiquetas=etiquetas;

			int i;
			for (i = 0;	i < datosPrograma->instrucciones_size;i++) {

				t_intructions instruccion =	datosPrograma->instrucciones_serializado[i];

				t_intructions* agregarInst = malloc(sizeof(t_intructions));

				agregarInst->offset = instruccion.offset;
				agregarInst->start = instruccion.start;

				list_add(pcb->indiceCodigo, agregarInst);
			}

			metadata_destruir(datosPrograma);
			log_info(logger, "Se cargo correctamente la metadata para el proceso PID: %d",pcb->PID);
		}

	} else {

		// El grado de multiprogramacion no me permite admitir el proceso aun..
		pcb = crearPCB(obtener_pid(), pcb);

		list_add(cola_new,pcb);

		t_registroTablaProcesos * registroProceso = malloc(sizeof(t_registroTablaProcesos));
		registroProceso->PID = pcb->PID;
		registroProceso->socket = socketConsola;

		list_add(listaProcesos,registroProceso);

		pid_struct->numero = KERNEL_MULTIPROG;
		socket_enviar(socketConsola, D_STRUCT_NUMERO, pid_struct);
		free(pid_struct);

	}

	if (list_size(cola_ready) > 0 && list_size(listaCpuLibres) > 0) {
		ejecutarPlanificacion(0);
	}

}

int obtenerPIDporConsola(int consolaBuscada){

	int indice;

	for (indice=0; indice < list_size(listaProcesos); indice++){
		t_registroTablaProcesos * registroProceso = list_get(listaProcesos,indice);

		if(registroProceso->socket == consolaBuscada){
			return registroProceso->PID;
		}
	}

	return -1;
}

t_registroTablaProcesos* obtenerConsolaPorPID(int PIDBuscado){

	int indice;

	for (indice=0; indice < list_size(listaProcesos); indice++){
		t_registroTablaProcesos * registroProceso = list_get(listaProcesos,indice);

		if(registroProceso->PID == PIDBuscado){
			return registroProceso;
		}
	}

	return NULL;
}

void eliminarRegistroProceso(int consolaBuscada){

	int indice;

	for (indice=0; indice < list_size(listaProcesos); indice++){
		t_registroTablaProcesos * registroProceso = list_get(listaProcesos,indice);

		if(registroProceso->socket == consolaBuscada){
			list_remove(listaProcesos,indice);
		}
	}
}

t_struct_pcb * buscarEnCola(t_list * cola,int PID){

	int indice;
	t_struct_pcb * pcbRecuperado;

	for(indice=0; indice<list_size(cola); indice++){
		pcbRecuperado = list_get(cola, indice);
		if (pcbRecuperado->PID == PID){
			return pcbRecuperado;
		}
	}

	return NULL;
}

t_struct_pcb * obtenerPCBActivo(int PID){

	t_struct_pcb * pcbRecuperado;

	// Busco en todas las colas activas de procesos
	pcbRecuperado = buscarEnCola(cola_new,PID);
	if(pcbRecuperado!=NULL) return pcbRecuperado;

	pcbRecuperado = buscarEnCola(cola_ready,PID);
	if(pcbRecuperado!=NULL) return pcbRecuperado;

	pcbRecuperado = buscarEnCola(cola_exec,PID);
	if(pcbRecuperado!=NULL) return pcbRecuperado;

	pcbRecuperado = buscarEnCola(cola_block,PID);
	if(pcbRecuperado!=NULL) return pcbRecuperado;

	return NULL;

}

void crearInformacionProcesoInicial(int PID){
	t_registroInformacionProceso * registro = malloc(sizeof(t_registroInformacionProceso));
	registro->pid = PID;
	registro->cantidad_liberar_heap=0;
	registro->cantidad_solicitar_heap=0;
	registro->rafagas=0;
	registro->semaforo_bloqueo = NULL;
	registro->syscall=0;
	registro->total_heap_liberado=0;
	registro->total_heap_solicitado=0;
	list_add(listaInformacionProcesos,registro);
}

t_registroInformacionProceso * recuperarInformacionProceso(int PID){
	int indice;

	for(indice=0; indice<list_size(listaInformacionProcesos); indice ++){
		t_registroInformacionProceso * registro = list_get(listaInformacionProcesos,indice);
		if(registro->pid == PID) return registro;
	}

	return NULL;
}

void liberarSemaforo(char* semaforo){

	int indice;

	t_struct_semaforo * semaforoRecuperado;

	for (indice=0; indice < list_size(listaSemaforos); indice++){
		semaforoRecuperado = list_get(listaSemaforos,indice);
		if(string_equals_ignore_case((char*)semaforoRecuperado->nombre,semaforo)){
			semaforoRecuperado->valor++;
			log_info(logger,"Se incrementa el semaforo %s",semaforoRecuperado->nombre);
		}
	}

}

void liberarSemaforoProceso(t_struct_pcb * pcb){

	t_registroInformacionProceso * registro = recuperarInformacionProceso(pcb->PID);

	if (registro->semaforo_bloqueo == NULL || string_is_empty(registro->semaforo_bloqueo)){
		return;
	}

	liberarSemaforo(registro->semaforo_bloqueo);

	free(registro->semaforo_bloqueo);
	registro->semaforo_bloqueo = string_new();

}

void liberarArchivosProceso(t_struct_pcb * pcb){

	t_list * archivosProceso = dictionary_get(tablaArchivosProceso,string_itoa(pcb->PID));

	int indice;

	// FD 0 / 1 / 2 reservados para el sistema
	for(indice=3; indice < list_size(archivosProceso); indice++){
		t_registroArchivosProc * registroTablaProceso = list_get(archivosProceso,indice);
		t_registroArchivosGlobal * registroTablaGlobal = list_get(tablaArchivosGlobal,registroTablaProceso->fd_TablaGlobal);

		if(registroTablaGlobal->cantidadAbierto==1){
			list_remove(tablaArchivosGlobal,registroTablaProceso->fd_TablaGlobal);
		} else {
			registroTablaGlobal->cantidadAbierto--;
		}
	}

}

void liberarMemoriaProceso(t_struct_pcb * pcb){

	t_struct_numero * PID = malloc(sizeof(t_struct_numero));

	PID->numero = pcb->PID;

	socket_enviar(socketMemoria,D_STRUCT_LIBERAR_MEMORIA, PID);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if(socket_recibir(socketMemoria,&tipoEstructura,&structRecibido) != -1){

		int resultado = ((t_struct_numero*) structRecibido)->numero;

		if (resultado==MEMORIA_OK){
			log_info(logger,"Se libero la memoria correspondiente al proceso %d",pcb->PID);
		} else {
			log_error(logger,"No se pudo liberar la memoria correspondiente al proceso %d",pcb->PID);
		}
	}
}

void removerDeCola(t_list * cola, t_list * nuevaCola, int nuevoEstado, int PID,
		bool liberarSemaforos, bool liberarArchivos, bool disminuirProcesos){

	if(!kernelPlanificando){
		log_info(logger,"Se solicito pasar el proceso %d al estado %d pero la planificacion esta detenida",PID,nuevoEstado);
		return;
	}

	int indice;
	t_struct_pcb * pcbRecuperado;

	for(indice=0; indice < list_size(cola); indice++){
		pcbRecuperado = list_get(cola,indice);
		if(pcbRecuperado->PID == PID){
			list_remove(cola,indice);

			if(disminuirProcesos) cantidadTotalPID--;
			if(liberarSemaforos) liberarSemaforoProceso(pcbRecuperado);
			if(liberarArchivos) liberarArchivosProceso(pcbRecuperado);

			list_add(nuevaCola,pcbRecuperado);
			pcbRecuperado->estado=nuevoEstado;
		}
	}

}

void pasarColaExit(t_struct_pcb * pcbFinalizar){
	if(!kernelPlanificando){
		log_info(logger,"El kernel no esta planificando, no se pasa el proceso %d a la cola de Exit",pcbFinalizar->PID);
		return;
	}

	if(pcbFinalizar->estado==E_NEW) removerDeCola(cola_new,cola_exit,E_EXIT,pcbFinalizar->PID,false,false,false);
	if(pcbFinalizar->estado==E_READY) removerDeCola(cola_ready,cola_exit,E_EXIT,pcbFinalizar->PID,true,true,true);
	if(pcbFinalizar->estado==E_EXEC) removerDeCola(cola_exec,cola_exit,E_EXIT,pcbFinalizar->PID,true,true,true);
	if(pcbFinalizar->estado==E_BLOCK) removerDeCola(cola_block,cola_exit,E_EXIT,pcbFinalizar->PID,true,true,true);
}

void informarLiberarHeap(t_struct_pcb * pcb){

	int indice;
	t_registroTablaHeap * registroHeapRecuperado;

	for(indice=0; indice < list_size(tablaHeap); indice++){

		registroHeapRecuperado = list_get(tablaHeap,indice);

		if(registroHeapRecuperado->PID==pcb->PID){
			log_info(logger,"El proceso %d no libero la pagina de heap %d, la libero..",pcb->PID,registroHeapRecuperado->numeroPagina);
			list_clean(registroHeapRecuperado->listaBloques);
			registroHeapRecuperado->PID=-1;
			registroHeapRecuperado->numeroPagina=-1;
			t_bloqueHeap * bloqueMetadata = malloc(sizeof(t_bloqueHeap));

			bloqueMetadata->isFree=true;
			bloqueMetadata->offset=5;
			bloqueMetadata->numeroBloque=-1;
			bloqueMetadata->size=tamanio_pagina-5;
			bloqueMetadata->fin=tamanio_pagina;

			list_add(registroHeapRecuperado->listaBloques,bloqueMetadata);
		}
	}

	log_info(logger,"El proceso %d libero todas las paginas de heap",pcb->PID);

}

void eliminarProcesoLista(t_registroTablaProcesos * proceso){
	int indice;

	for (indice=0; indice < list_size(listaProcesos); indice++){
		t_registroTablaProcesos * registroProceso = list_get(listaProcesos,indice);

		if(registroProceso->socket == proceso->socket){
			list_remove(listaProcesos,indice);
		}
	}
}

void informarFinalizacionConsola(t_struct_pcb * pcb){

	t_registroTablaProcesos * proceso = obtenerConsolaPorPID(pcb->PID);

	socket_enviar(proceso->socket,D_STRUCT_FIN_PCB,pcb);
	eliminarProcesoLista(proceso);
}

void abortarPrograma(int socketConsola, bool finalizarPrograma){

	int PID = obtenerPIDporConsola(socketConsola);

	if(PID ==-1) {
		log_error(logger,"La consola %d no esta ejecutando ningun proceso, puede cerrarse", socketConsola);
		return;
	}

	t_struct_pcb * pcbRecuperado = obtenerPCBActivo(PID);

	if(pcbRecuperado == NULL){
		log_error(logger,"El proceso PID %d ya finalizo, no debe abortarse", PID);
		return;
	}

	if(finalizarPrograma){
		pcbRecuperado->exitcode = EC_FINALIZADO_CONSOLA;
	} else {
		pcbRecuperado->exitcode = EC_DESCONEXION_CONSOLA;
	}

	if(pcbRecuperado->estado == E_EXEC){
		t_registroProcesoDestruir * pidFinalizar = malloc(sizeof(t_registroProcesoDestruir));
		pidFinalizar->pid=PID;
		list_add(listaProcesosFinalizar,pidFinalizar);
	} else {
		pasarColaExit(pcbRecuperado);
		liberarMemoriaProceso(pcbRecuperado);
		informarLiberarHeap(pcbRecuperado);
		if(finalizarPrograma) informarFinalizacionConsola(pcbRecuperado);
	}

}

void obtenerVariableCompartida(int socketCPU, char * varCompartida){

	t_struct_var_compartida * variableRecuperada = malloc(sizeof(t_struct_var_compartida));
	int indice;

	for(indice=0; indice < list_size(listaVarCompartidas); indice++){
		variableRecuperada = list_get(listaVarCompartidas,indice);

		if(string_equals_ignore_case(variableRecuperada->nombre,string_from_format("!%s",varCompartida))){

			t_struct_numero * valorCompartida = malloc(sizeof(t_struct_numero));
			valorCompartida->numero=variableRecuperada->valor;

			socket_enviar(socketCPU,D_STRUCT_NUMERO,valorCompartida);
			free(valorCompartida);

			break;
		}
	}
}

void grabarVariableCompartida(int socketCPU, t_struct_var_compartida * grabarVarCompartida){

	t_struct_var_compartida * variableRecuperada;
	int indice;

	for(indice=0; indice < list_size(listaVarCompartidas); indice++){
		variableRecuperada = list_get(listaVarCompartidas,indice);

		if(string_equals_ignore_case(variableRecuperada->nombre,string_from_format("!%s",grabarVarCompartida->nombre))){

			variableRecuperada->valor = grabarVarCompartida->valor;

			t_struct_numero * valorCompartida = malloc(sizeof(t_struct_numero));
			valorCompartida->numero=variableRecuperada->valor;

			socket_enviar(socketCPU,D_STRUCT_NUMERO,valorCompartida);
			free(valorCompartida);

			break;
		}
	}

}

bool verificarProcesoFinalizar(t_struct_pcb * pcb){

	int indice;

	for(indice=0; indice < list_size(listaProcesosFinalizar); indice++){
		t_registroProcesoDestruir * PIDRecuperado = list_get(listaProcesosFinalizar,indice);
		if(PIDRecuperado->pid == pcb->PID){
			list_remove(listaProcesosFinalizar,indice);
			return true;
		}
	}
	return false;
}

void actualizarPCBExec(t_struct_pcb * pcbBuscado){

	int indice;
	t_struct_pcb * pcbRecuperado;

	for(indice=0; indice < list_size(cola_exec); indice++){
		pcbRecuperado= list_get(cola_exec,indice);
		if(pcbRecuperado->PID==pcbBuscado->PID){
			list_remove(cola_exec,indice);
			list_add_in_index(cola_exec,indice,pcbBuscado);
		}
	}
}

void actualizarPCBBlock(t_struct_pcb * pcbBuscado){

	int indice;
	t_struct_pcb * pcbRecuperado;

	for(indice=0; indice < list_size(cola_block); indice++){
		pcbRecuperado= list_get(cola_block,indice);
		if(pcbRecuperado->PID==pcbBuscado->PID){
			list_remove(cola_block,indice);
			list_add_in_index(cola_block,indice,pcbBuscado);
		}
	}
}

t_cpu* buscarCPUenLista(t_list * lista, int socketCPU, bool quitarDeLista){

	int indice;
	t_cpu * cpuRecuperada;

	for(indice=0;indice<list_size(lista);indice++){
		cpuRecuperada = list_get(lista,indice);

		if(cpuRecuperada->socket==socketCPU){

			if(quitarDeLista) list_remove(lista,indice);

			return cpuRecuperada;
		}
	}

	return NULL;

}

t_cpu* obtenerCPUporSocket(int socketCPU, bool quitarDeLista){

	t_cpu* cpuEncontrada;

	cpuEncontrada = buscarCPUenLista(listaCpuLibres,socketCPU,quitarDeLista);
	if(cpuEncontrada!=NULL) return cpuEncontrada;

	cpuEncontrada = buscarCPUenLista(listaCpuOcupadas,socketCPU,quitarDeLista);
	if(cpuEncontrada!=NULL) return cpuEncontrada;

	return NULL;
}

void traerProcesoColaNew(){
	if(!kernelPlanificando){
		log_info(logger,"Se solicita traer proceso de la cola de new pero el kernel no esta planificando");
		return;
	}

	if(list_size(cola_new)==0) return;

	t_struct_pcb * pcbNew = list_remove(cola_new,0);

	log_info(logger, "Por disminuir la cantidad de procesos en sistema, traigo el PID: %d de la cola de NEW",pcbNew->PID);

	t_registroTablaProcesos * proceso = obtenerConsolaPorPID(pcbNew->PID);

	t_struct_numero * pidNew = malloc(sizeof(t_struct_numero));
	pidNew->numero=pcbNew->PID;

	socket_enviar(proceso->socket,D_STRUCT_SOLICITAR_CODIGO,pidNew);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	socket_recibir(proceso->socket,&tipoEstructura,&structRecibido);

	int tamanio_programa = ((t_struct_programa*) structRecibido)->tamanio ;
	char * programa = malloc(tamanio_programa+1);

	memcpy(programa, ((t_struct_programa*) structRecibido)->buffer, tamanio_programa);

	t_struct_numero* pid_struct = malloc(sizeof(t_struct_numero));

	if (reservarPaginas(pcbNew,programa,tamanio_programa) == -1) {

		// Si no pude asignarlas aviso al a consola del rechazo
		pid_struct->numero = -1;
		socket_enviar(proceso->socket, D_STRUCT_NUMERO, pid_struct);
		free(pid_struct);

	} else {

		pid_struct->numero = pcbNew->PID;
		socket_enviar(proceso->socket, D_STRUCT_NUMERO, pid_struct);
		free(pid_struct);

		enviarCodigoMemoria(programa,tamanio_programa,pcbNew);

		t_metadata_program* datosPrograma = malloc(sizeof(t_metadata_program));
		datosPrograma =	metadata_desde_literal(programa);

		pcbNew->quantum_sleep=configuracion->quantumSleep;
		pcbNew->programCounter=datosPrograma->instruccion_inicio;
		pcbNew->tamanioIndiceEtiquetas=datosPrograma->etiquetas_size;
		pcbNew->cantidadInstrucciones = datosPrograma->instrucciones_size;

		char * etiquetas = malloc(datosPrograma->etiquetas_size);

		memcpy(etiquetas,datosPrograma->etiquetas,datosPrograma->etiquetas_size);
		pcbNew->indiceEtiquetas=etiquetas;

		int i;
		for (i = 0;	i < datosPrograma->instrucciones_size;i++) {

			t_intructions instruccion =	datosPrograma->instrucciones_serializado[i];

			t_intructions* agregarInst = malloc(sizeof(t_intructions));

			agregarInst->offset = instruccion.offset;
			agregarInst->start = instruccion.start;

			list_add(pcbNew->indiceCodigo, agregarInst);
		}

		metadata_destruir(datosPrograma);
		log_info(logger, "Se cargo correctamente la metadata para el proceso PID: %d",pcbNew->PID);
	}

	agregarColaListos(pcbNew);
	free(programa);
	free(structRecibido);

	if(list_size(listaCpuLibres)>0){
		ejecutarPlanificacion(0);
	}

}

void desbloquearProcesoEnWait(t_struct_semaforo * semaforoRecuperado){

	int indice;
	t_struct_pcb * pcbRecuperado;

	for(indice=0;indice < list_size(cola_block);indice++){
		pcbRecuperado = list_get(cola_block,indice);
		t_registroInformacionProceso * registro = recuperarInformacionProceso(pcbRecuperado->PID);

		if(string_equals_ignore_case(registro->semaforo_bloqueo,*semaforoRecuperado->nombre)){
			log_info(logger,"Con el signal realizado sobre el semaforo %s se desbloquea el proceso %d",
					semaforoRecuperado->nombre,pcbRecuperado->PID);
			removerDeCola(cola_block,cola_ready,E_READY,pcbRecuperado->PID,false,false,false);
			free(registro->semaforo_bloqueo);registro->semaforo_bloqueo = string_new();

			break;
		}
	}

}

void realizarWaitSemaforo(int socketCPU,char * waitSemaforo){

	t_struct_semaforo * semaforoRecuperado;
	bool encontreSemaforo = false;
	t_struct_numero * respuesta = malloc(sizeof(t_struct_numero));
	int indice;

	for(indice=0; indice < list_size(listaSemaforos); indice++){
		semaforoRecuperado = list_get(listaSemaforos,indice);

		if(string_equals_ignore_case(*semaforoRecuperado->nombre,waitSemaforo)){

			log_info(logger,"CPU %d realiza wait sobre el semaforo %s",socketCPU,waitSemaforo);
			encontreSemaforo = true;
			semaforoRecuperado->valor--;

			// Si el semaforo queda menor a 0 el proceso queda bloqueado lo informo con un 1, caso contrario 0
			respuesta->numero = semaforoRecuperado->valor<0 ? 1 : 0;

			if(semaforoRecuperado->valor<0){

				socket_enviar(socketCPU,D_STRUCT_NUMERO,respuesta);
				free(respuesta);

				t_tipoEstructura tipoEstructura;
				void * structRecibido;

				socket_recibir(socketCPU,&tipoEstructura, &structRecibido);

				t_struct_pcb * pcbBloqueado = (t_struct_pcb*) structRecibido;

				if(verificarProcesoFinalizar(pcbBloqueado)){

					log_info(logger,"Corresponde finalizar el proceso %d",pcbBloqueado->PID);

					actualizarPCBExec(pcbBloqueado);
					pasarColaExit(pcbBloqueado);
					liberarMemoriaProceso(pcbBloqueado);
					informarFinalizacionConsola(pcbBloqueado);
					informarLiberarHeap(pcbBloqueado);
					traerProcesoColaNew();

					t_cpu* cpuProcesando = obtenerCPUporSocket(socketCPU, true);

					cpuProcesando->PID=-1;
					cpuProcesando->quantum=0;
					list_add(listaCpuLibres,cpuProcesando);
					semaforoRecuperado->valor++;

				}else{
					log_info(logger,"Corresponde pasar a block el proceso %d",pcbBloqueado->PID);

					actualizarPCBExec(pcbBloqueado);

					t_registroInformacionProceso * registro = recuperarInformacionProceso(pcbBloqueado->PID);
					free(registro->semaforo_bloqueo);registro->semaforo_bloqueo=string_new();

					string_append(&(registro->semaforo_bloqueo),waitSemaforo);

					removerDeCola(cola_exec,cola_block,E_BLOCK,pcbBloqueado->PID,false,false,false);

					t_cpu* cpuProcesando = obtenerCPUporSocket(socketCPU, true);
					cpuProcesando->PID=-1;
					cpuProcesando->quantum=0;

					list_add(listaCpuLibres,cpuProcesando);


				}

				if (list_size(cola_ready) > 0) {
					ejecutarPlanificacion(0);
				}

			} else {

				log_info(logger,"El semaforo %s no se bloquea, valor actual %d",waitSemaforo,semaforoRecuperado->valor);
				socket_enviar(socketCPU,D_STRUCT_NUMERO,respuesta);
				free(respuesta);
			}
		}
	}

	if(!encontreSemaforo){
		respuesta->numero=-1;
		log_info(logger,"No se pudo encontrar el semaforo buscado por el cpu %d",socketCPU);
		socket_enviar(socketCPU,D_STRUCT_NUMERO,respuesta);
		free(respuesta);
	}
}

void realizarSignalSemaforo(int socketCPU,char * signalSemaforo){
	t_struct_semaforo * semaforoRecuperado;
	bool encontreSemaforo = false;
	t_struct_numero * respuesta = malloc(sizeof(t_struct_numero));
	int indice;

	for(indice=0; indice < list_size(listaSemaforos); indice++){
		semaforoRecuperado = list_get(listaSemaforos,indice);

		if(string_equals_ignore_case(*semaforoRecuperado->nombre,signalSemaforo)){


			encontreSemaforo = true;
			semaforoRecuperado->valor++;
			respuesta->numero = KERNEL_OK;

			log_info(logger,"CPU %d realiza signal sobre el semaforo %s el nuevo valor es %d",
					socketCPU,signalSemaforo,semaforoRecuperado->valor);

			socket_enviar(socketCPU,D_STRUCT_NUMERO,respuesta);
			free(respuesta);

			desbloquearProcesoEnWait(semaforoRecuperado);
		}
	}

	if(!encontreSemaforo){
		respuesta->numero=KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,respuesta);
		free(respuesta);
	}

}

t_descriptor_archivo obtenerArchivoTablaGlobal(t_struct_archivo * archivo){

	int indice;
	for(indice=0; indice < list_size(tablaArchivosGlobal); indice++){
		t_registroArchivosGlobal * registro = list_get(tablaArchivosGlobal,indice);
		if(string_equals_ignore_case(registro->nombre,archivo->informacion)){
			return indice;
		}
	}
	return -1;
}

t_registroArchivosProc * obtenerRegistroTablaProceso(t_struct_archivo * archivo){

	t_list * archivosProceso = dictionary_get(tablaArchivosProceso,string_itoa(archivo->pid));

	if(archivosProceso==NULL || archivo->fileDescriptor<3){
		return NULL;
	} else {
		t_registroArchivosProc * registroArchivoProceso =list_get(archivosProceso,archivo->fileDescriptor);

		if(registroArchivoProceso==NULL){
			return NULL;
		} else {
			return registroArchivoProceso;
		}
	}
}

t_registroArchivosGlobal * obtenerRegistroTablaGlobal(t_registroArchivosProc * registroArchivoProceso){

	t_registroArchivosGlobal * registroGlobal =
						list_get(tablaArchivosGlobal,registroArchivoProceso->fd_TablaGlobal);

	return registroGlobal;

}

void abrirArchivo(int socketCPU,t_struct_archivo * archivo){

	t_struct_numero * resultadoAbrir = malloc(sizeof(t_struct_numero));

	t_registroInformacionProceso * registroInfo = recuperarInformacionProceso(archivo->pid);
	registroInfo->syscall++;

	t_list * archivosProceso = dictionary_get(tablaArchivosProceso,string_itoa(archivo->pid));

	t_registroArchivosProc * registroArchivoProceso = malloc(sizeof(t_registroArchivosProc));

	registroArchivoProceso->flags.creacion = archivo->flags.creacion;
	registroArchivoProceso->flags.escritura = archivo->flags.escritura;
	registroArchivoProceso->flags.lectura = archivo->flags.lectura;

	int existeEnTablaGlobal = obtenerArchivoTablaGlobal(archivo);

	socket_enviar(socketFS,D_STRUCT_ARCHIVO_ABR,archivo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	socket_recibir(socketFS,&tipoEstructura,&structRecibido);

	t_struct_numero * respuestaAbrir = malloc(sizeof(t_struct_numero));
	respuestaAbrir = ((t_struct_numero*) structRecibido);

	if(respuestaAbrir->numero==FS_ABRIR_CREAR_OK ||
			respuestaAbrir->numero==FS_ABRIR_NO_CREAR_OK){

		//Existia y lo abri ok o no existia pero lo pude crear

		if(existeEnTablaGlobal==-1){
			//Si no existe en la tabla global de archivos
			t_registroArchivosGlobal * registroGlobal = malloc(sizeof(t_registroArchivosGlobal));
			registroGlobal->cantidadAbierto = 1;
			registroGlobal->nombre = string_new();
			string_append(&(registroGlobal->nombre),archivo->informacion);
			int fd_TablaGlobal = list_add(tablaArchivosGlobal,registroGlobal);

			registroArchivoProceso->fd_TablaGlobal = fd_TablaGlobal;
			registroArchivoProceso->cursor=0;

			t_descriptor_archivo newFDProceso = list_size(archivosProceso);
			list_add(archivosProceso,registroArchivoProceso);

			resultadoAbrir->numero = newFDProceso;

			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoAbrir);
			log_info(logger, "El archivo solicitado por el proceso %d es fd %d", archivo->pid,newFDProceso);

			free(resultadoAbrir);
			free(respuestaAbrir);

			return;

		} else {
			//SI existe en la tabla global de archivos
			t_registroArchivosGlobal * registroGlobal =
					list_get(tablaArchivosGlobal,existeEnTablaGlobal);

			registroGlobal->cantidadAbierto++;

			registroArchivoProceso->cursor=0;

			t_descriptor_archivo newFDProceso = list_size(archivosProceso);
			list_add(archivosProceso,registroArchivoProceso);

			resultadoAbrir->numero = newFDProceso;

			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoAbrir);
			log_info(logger,"El archivo solicitado por el proceso %d es fd %d", archivo->pid,newFDProceso);

			free(resultadoAbrir);

			return;
		}

	} else {

		resultadoAbrir->numero = respuestaAbrir->numero;

		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoAbrir);
		log_info(logger,"El archivo solicitado por el proceso %d no se pudo abrir ni crear", archivo->pid);

		free(resultadoAbrir);
	}

}

void borrarArchivo(int socketCPU,t_struct_sol_lectura * archivoBorrar){

	t_struct_numero * resultadoBorrar = malloc(sizeof(t_struct_numero));

	t_struct_archivo * archivo = malloc(sizeof(t_struct_archivo));
	archivo->fileDescriptor=archivoBorrar->pagina;
	archivo->pid=archivoBorrar->PID;
	archivo->tamanio=archivoBorrar->offset;

	t_registroInformacionProceso * registroInfo = recuperarInformacionProceso(archivo->pid);
	registroInfo->syscall++;

	t_registroArchivosProc * registroArchivoProceso = obtenerRegistroTablaProceso(archivo);
	if(registroArchivoProceso==NULL){
		resultadoBorrar->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
		log_info(logger,"El proceso %d no tiene archivos abiertos", &(archivo->pid));
		free(archivo);
		free(resultadoBorrar);
		return;
	}

	t_registroArchivosGlobal * registroGlobal = obtenerRegistroTablaGlobal(registroArchivoProceso);
	if(registroGlobal==NULL){
		resultadoBorrar->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
		log_info(logger,"El proceso %d no tiene archivos abiertos", &(archivo->pid));
		free(archivo);
		free(resultadoBorrar);
		return;
	}

	if(registroGlobal->cantidadAbierto!=1){

		resultadoBorrar->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
		log_info(logger,"El archivo %d esta abierto por mas de un proceso", archivo->fileDescriptor);

		free(resultadoBorrar);

	} else {

		char * nombreLimpio = prepararInstruccion(registroGlobal->nombre);

		t_struct_string * pathBorrar = malloc(sizeof(t_struct_string));
		pathBorrar->string = nombreLimpio;

		//Puedo borrarlo, se lo pido al FS..
		socket_enviar(socketFS,D_STRUCT_ARCHIVO_BORRAR,archivo);

		socket_enviar(socketFS,D_STRUCT_STRING,pathBorrar);

		t_tipoEstructura tipoEstructura;
		void * structRecibido;

		socket_recibir(socketFS,&tipoEstructura,&structRecibido);

		t_struct_borrar * respuestaBorrar = ((t_struct_borrar*) structRecibido);

		if(respuestaBorrar->confirmacion==FS_BORRAR_OK){

			resultadoBorrar->numero = KERNEL_OK;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
			log_info(logger,"El archivo %d se pudo borrar", archivo->fileDescriptor);

			free(resultadoBorrar);
		} else {

			resultadoBorrar->numero = KERNEL_ERROR;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
			log_info(logger,"El archivo %d no se pudo borrar", archivo->fileDescriptor);

			free(resultadoBorrar);

		}
	}
	free(archivo);
}

void cerrarArchivo(int socketCPU,t_struct_sol_lectura * archivoCerrar){

	t_struct_numero * resultadoBorrar = malloc(sizeof(t_struct_numero));

	t_struct_archivo * archivo = malloc(sizeof(t_struct_archivo));
	archivo->fileDescriptor=archivoCerrar->pagina;
	archivo->pid=archivoCerrar->PID;
	archivo->tamanio=archivoCerrar->offset;

	t_registroInformacionProceso * registroInfo = recuperarInformacionProceso(archivo->pid);
	registroInfo->syscall++;

	t_list * archivosProceso = dictionary_get(tablaArchivosProceso,string_itoa(archivo->pid));

	t_registroArchivosProc * registroArchivoProceso = obtenerRegistroTablaProceso(archivo);

	if(registroArchivoProceso==NULL){
		resultadoBorrar->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
		log_info(logger,"El proceso %d no tiene archivos abiertos", &(archivo->pid));
		free(archivo);
		free(resultadoBorrar);
		return;
	}

	t_registroArchivosGlobal * registroGlobal = obtenerRegistroTablaGlobal(registroArchivoProceso);
	if(registroGlobal==NULL){
		resultadoBorrar->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
		log_info(logger,"El proceso %d no tiene archivos abiertos", &(archivo->pid));
		free(archivo);
		free(resultadoBorrar);
		return;
	}

	list_remove(archivosProceso,archivo->fileDescriptor);

	if(registroGlobal->cantidadAbierto==1){
		list_remove(tablaArchivosGlobal,registroArchivoProceso->fd_TablaGlobal);
	} else{
		registroGlobal->cantidadAbierto--;
	}

	resultadoBorrar->numero = KERNEL_OK;
	socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
	log_info(logger,"El proceso %d pudo cerrar el archivo solicitado", archivo->pid);
	free(archivo);
	free(resultadoBorrar);


}

void moverCursorArchivo(int socketCPU,t_struct_sol_lectura * archivoMover){

	t_struct_numero * resultadoMover = malloc(sizeof(t_struct_numero));

	t_struct_archivo * archivo = malloc(sizeof(t_struct_archivo));
	archivo->fileDescriptor=archivoMover->pagina;
	archivo->pid=archivoMover->PID;
	archivo->tamanio=archivoMover->offset;

	t_registroInformacionProceso * registroInfo = recuperarInformacionProceso(archivo->pid);
	registroInfo->syscall++;

	t_registroArchivosProc * registroArchivoProceso = obtenerRegistroTablaProceso(archivo);

	if(registroArchivoProceso==NULL){
		resultadoMover->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoMover);
		log_info(logger,"El proceso %d no tiene archivos abiertos", &(archivo->pid));
		free(archivo);
		free(resultadoMover);
		return;
	}

	registroArchivoProceso->cursor=archivo->tamanio;

	resultadoMover->numero = KERNEL_OK;
	socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoMover);
	log_info(logger,"El proceso %d movio el cursor dentro del archivo solicitado", archivo->pid);
	free(archivo);
	free(resultadoMover);

}

void escribirArchivo(int socketCPU,t_struct_archivo * archivo){

	t_struct_numero * resultadoEscribir = malloc(sizeof(t_struct_numero));

	t_registroInformacionProceso * registroInfo = recuperarInformacionProceso(archivo->pid);
	registroInfo->syscall++;

	if(archivo->fileDescriptor==1){

		t_registroTablaProcesos* registroProceso = obtenerConsolaPorPID(archivo->pid);

		if(registroProceso!=NULL){

			t_struct_string * textoImprimir = malloc(sizeof(t_struct_string));
			textoImprimir->string = archivo->informacion;

			t_struct_numero * tamanioImprimir = malloc(sizeof(t_struct_numero));
			tamanioImprimir->numero = archivo->tamanio;

			socket_enviar(registroProceso->socket,D_STRUCT_IMPR,textoImprimir);

			socket_enviar(registroProceso->socket,D_STRUCT_NUMERO,tamanioImprimir);

			resultadoEscribir->numero = KERNEL_OK;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoEscribir);
			log_info(logger,"El proceso %d solicita escribir por consola", &(archivo->pid));
			free(resultadoEscribir);
			return;

		} else {

			resultadoEscribir->numero = KERNEL_ERROR;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoEscribir);
			log_info(logger,"El proceso %d solicita escribir por consola pero no puedo determinar a cual corresponde", &(archivo->pid));
			free(resultadoEscribir);
			return;

		}

	} else {

		t_registroArchivosProc * registroArchivoProceso = obtenerRegistroTablaProceso(archivo);

		if(registroArchivoProceso==NULL){
			resultadoEscribir->numero = KERNEL_ERROR;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoEscribir);
			log_info(logger,"El proceso %d no el archivo abierto", &(archivo->pid));

			free(resultadoEscribir);
			return;
		}

		t_registroArchivosGlobal * registroGlobal = obtenerRegistroTablaGlobal(registroArchivoProceso);

		t_struct_string * path = malloc(sizeof(t_struct_string));

		char * nombreLimpio = prepararInstruccion(registroGlobal->nombre);

		path->string = nombreLimpio;

		t_struct_numero * offset = malloc(sizeof(t_struct_numero));

		offset->numero=registroArchivoProceso->cursor;

		archivo->flags.creacion = registroArchivoProceso->flags.creacion;
		archivo->flags.escritura = registroArchivoProceso->flags.escritura;
		archivo->flags.lectura = registroArchivoProceso->flags.lectura;

		//Envio el archivo que tengo que escribir al filesystem
		socket_enviar(socketFS,D_STRUCT_ARCHIVO_ESC,archivo);

		//Envio el desplazamiento para la escritura dentro del archivo.
		socket_enviar(socketFS,D_STRUCT_NUMERO,offset);

		//Envio el path del archivo.
		socket_enviar(socketFS,D_STRUCT_STRING,path);


		t_tipoEstructura tipoEstructura;
		void * structRecibido;

		socket_recibir(socketFS,&tipoEstructura,&structRecibido);

		t_struct_numero * respuestaGuardar = malloc(sizeof(t_struct_numero));
		respuestaGuardar = ((t_struct_numero*) structRecibido);

		if(respuestaGuardar->numero==FS_ESCRIBIR_OK){

			registroArchivoProceso->cursor+=archivo->tamanio;
			resultadoEscribir->numero = KERNEL_OK;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoEscribir);
			log_info(logger,"El archivo %d se pudo escribir", archivo->fileDescriptor);

			free(resultadoEscribir);
		} else {

			resultadoEscribir->numero = KERNEL_ERROR;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoEscribir);
			log_info(logger,"El archivo %d no se pudo borrar", archivo->fileDescriptor);

			free(resultadoEscribir);

		}

	}

}

void leerArchivo(int socketCPU,t_struct_archivo * archivo){

	t_struct_numero * resultadoLeer = malloc(sizeof(t_struct_numero));

	t_registroInformacionProceso * registroInfo = recuperarInformacionProceso(archivo->pid);
	registroInfo->syscall++;

	t_registroArchivosProc * registroArchivoProceso = obtenerRegistroTablaProceso(archivo);

	if(registroArchivoProceso==NULL){
		resultadoLeer->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoLeer);
		log_info(logger,"El proceso %d no el archivo abierto", &(archivo->pid));

		free(resultadoLeer);
		return;
	}

	t_registroArchivosGlobal * registroGlobal = obtenerRegistroTablaGlobal(registroArchivoProceso);

	t_struct_string * path = malloc(sizeof(t_struct_string));
	path->string = registroGlobal->nombre;

	t_struct_numero * offset = malloc(sizeof(t_struct_numero));

	offset->numero=registroArchivoProceso->cursor;

	archivo->flags.creacion = registroArchivoProceso->flags.creacion;
	archivo->flags.escritura = registroArchivoProceso->flags.escritura;
	archivo->flags.lectura = registroArchivoProceso->flags.lectura;

	//Envio el archivo que tengo que escribir al filesystem
	socket_enviar(socketFS,D_STRUCT_ARCHIVO_LEC,archivo);

	//Envio el desplazamiento para la escritura dentro del archivo.
	socket_enviar(socketFS,D_STRUCT_NUMERO,offset);

	//Envio el path del archivo.
	socket_enviar(socketFS,D_STRUCT_STRING,path);


	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	socket_recibir(socketFS,&tipoEstructura,&structRecibido);

	t_struct_obtener * respuestaLeer = ((t_struct_obtener*) structRecibido);

	if(respuestaLeer->confirmacion==FS_LEER_OK){


		socket_recibir(socketFS,&tipoEstructura,&structRecibido);

		t_struct_string * lecturaArchivo = malloc(sizeof(t_struct_string));
		lecturaArchivo = ((t_struct_string *) structRecibido);

		registroArchivoProceso->cursor+=archivo->tamanio;

		resultadoLeer->numero = KERNEL_OK;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoLeer);
		log_info(logger,"El archivo %d se pudo leer", archivo->fileDescriptor);

		free(resultadoLeer);

		socket_enviar(socketCPU, D_STRUCT_STRING, lecturaArchivo);

		free(lecturaArchivo);

	} else {

		resultadoLeer->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoLeer);
		log_info(logger,"El archivo %d no se pudo borrar", archivo->fileDescriptor);

		free(resultadoLeer);

	}

}

bool correspondeAbortarProcesoDeCPU(int socketCPU){

	t_cpu* cpu = obtenerCPUporSocket(socketCPU,false);

	t_struct_pcb * pcb = obtenerPCBActivo(cpu->PID);

	if(verificarProcesoFinalizar(pcb)) return true;

	return false;

}

bool procesoEnWait(int socketCPU){

	t_cpu* cpu = obtenerCPUporSocket(socketCPU,false);

	t_struct_pcb * pcb = buscarEnCola(cola_block,cpu->PID);

	if(pcb!=NULL) return true;

	return false;

}

int determinarExitCode(t_struct_pcb * pcb){

	int motivoRetorno = pcb->retornoPCB;

	if(motivoRetorno==D_STRUCT_ERROR_MEMORIA) return EC_EXCEP_MEMORIA;
	if(motivoRetorno==D_STRUCT_ERROR_KERNEL) return EC_DESCONEXION_KERNEL;
	if(motivoRetorno==D_STRUCT_ERROR_ESCRITURA) return EC_ARCHIVO_ES_PERMISOS;
	if(motivoRetorno==D_STRUCT_ERROR_LECTURA) return EC_ARCHIVO_LE_PERMISOS;
	if(motivoRetorno==D_STRUCT_ERROR_HEAP_MAX) return EC_RESERVA_MAYOR_PAGINA;
	if(motivoRetorno==D_STRUCT_ERROR_STACK_OVERFLOW) return EC_STACK_OVERFLOW;
	if(motivoRetorno==D_STRUCT_ERROR_HEAP) return EC_EXCEP_MEMORIA;
	if(motivoRetorno==D_STRUCT_ERROR_HEAP_LIB) return EC_EXCEP_MEMORIA;
	if(motivoRetorno==D_STRUCT_ERROR_ABRIR) return EC_ARCHIVO_INEX;
	if(motivoRetorno==D_STRUCT_ERROR_ESPACIO_ARCHIVO) return EC_ESPACIO_ARCHIVO;

	return EC_SIN_DEFINICION;

}

void matarProcesoEnEjecucion(int socketCPU, bool desconectarCPU){

	t_cpu * cpu = obtenerCPUporSocket(socketCPU,true);

	if(cpu->PID!=-1){
		t_struct_pcb * pcb = obtenerPCBActivo(cpu->PID);

		if(!desconectarCPU) {
			cpu->PID = -1;
			cpu->quantum = 0;
			list_add(listaCpuLibres,cpu);

			ejecutarPlanificacion(0);
		}

		if(pcb->estado==E_EXEC){
			pcb->exitcode= desconectarCPU ? EC_DESCONEXION_CPU : determinarExitCode(pcb);
			liberarMemoriaProceso(pcb);
			informarLiberarHeap(pcb);
			informarFinalizacionConsola(pcb);
			removerDeCola(cola_exec,cola_exit,E_EXIT,pcb->PID,true,true,true);
			traerProcesoColaNew();
		}
	}
}

void ejecutarProximoProceso(t_cpu * cpuEjecutar){

	t_struct_pcb * pcbEjecutar = malloc(sizeof(t_struct_pcb));

	pcbEjecutar = list_get(cola_ready,0);

	if(pcbEjecutar==NULL){
		log_info(logger,"No hay procesos en la cola de listos para ejecutar");
		return;
	}

	removerDeCola(cola_ready,cola_exec,E_EXEC,pcbEjecutar->PID,false,false,false);

	t_registroInformacionProceso * registro = recuperarInformacionProceso(pcbEjecutar->PID);
	registro->rafagas++;
	cpuEjecutar->PID=pcbEjecutar->PID;
	cpuEjecutar->quantum=1;
	list_add(listaCpuOcupadas,cpuEjecutar);
	pcbEjecutar->cpuID=cpuEjecutar->socket;

	socket_enviar(cpuEjecutar->socket,D_STRUCT_PCB,pcbEjecutar);

}

void ejecutarPlanificacion(int socketCPU){

	if(!kernelPlanificando){
		log_info(logger,"Se solicita planificar pero el kernel no esta planificando");
		return;
	}

	if(socketCPU != 0){

		log_info(logger,"La CPU %d finalizo una instruccion reejecuto la logica de planificacion para la misma",socketCPU);

		if(correspondeAbortarProcesoDeCPU(socketCPU)){

			t_struct_numero * abortar = malloc(sizeof(t_struct_numero));
			abortar->numero=1;

			socket_enviar(socketCPU,D_STRUCT_ABORTAR_EJECUCION,abortar);

			t_tipoEstructura tipoEstructura;
			void * structRecibido;

			socket_recibir(socketCPU,&tipoEstructura,&structRecibido);

			t_struct_pcb * pcbEjecutando = ((t_struct_pcb*) structRecibido);

			log_info(logger,"Corresponde abortar el PID %d que se encontraba corriendo en la cpu %d",pcbEjecutando->PID,socketCPU);

			actualizarPCBExec(pcbEjecutando);

			removerDeCola(cola_exec,cola_exit,E_EXIT,pcbEjecutando->PID,true,false,true);
			liberarMemoriaProceso(pcbEjecutando);
			informarFinalizacionConsola(pcbEjecutando);
			informarLiberarHeap(pcbEjecutando);
			traerProcesoColaNew();

			t_cpu* cpu = obtenerCPUporSocket(socketCPU,true);
			cpu->PID = -1;
			cpu->quantum=0;
			list_add(listaCpuLibres,cpu);

			ejecutarProximoProceso(cpu);
			return;
		}

		if(string_equals_ignore_case(configuracion->algoritmo,"FIFO")){


			t_struct_numero * quantumSleep = malloc(sizeof(t_struct_numero));
			quantumSleep->numero=configuracion->quantumSleep;

			socket_enviar(socketCPU,D_STRUCT_CONTINUAR_EJECUCION,quantumSleep);

			t_cpu* cpu = obtenerCPUporSocket(socketCPU,false);

			t_registroInformacionProceso * registro = recuperarInformacionProceso(cpu->PID);
			registro->rafagas++;
			log_info(logger,"CPU %d puede continuar la ejecucion del proceso %d - Alg: FIFO",socketCPU,cpu->PID);

		} else {

			t_cpu* cpu = obtenerCPUporSocket(socketCPU,false);

			if(cpu->quantum==configuracion->quantum){

				t_struct_numero * finQuantum = malloc(sizeof(t_struct_numero));
				finQuantum->numero=1;

				log_info(logger,"CPU %d se quedo sin quantum para el proceso %d - Alg: RR",socketCPU,cpu->PID);

				socket_enviar(socketCPU,D_STRUCT_FIN_QUANTUM,finQuantum);

				t_tipoEstructura tipoEstructura;
				void * structRecibido;

				socket_recibir(socketCPU,&tipoEstructura,&structRecibido);

				t_struct_pcb * pcbEjecutando = ((t_struct_pcb*) structRecibido);

				actualizarPCBExec(pcbEjecutando);

				removerDeCola(cola_exec,cola_ready,E_READY,pcbEjecutando->PID,false,false,false);

				obtenerCPUporSocket(socketCPU,true);

				cpu->PID = -1;
				cpu->quantum=0;
				list_add(listaCpuLibres,cpu);

				ejecutarProximoProceso(cpu);

			} else {

				t_struct_numero * quantumSleep = malloc(sizeof(t_struct_numero));
				quantumSleep->numero=configuracion->quantumSleep;

				t_cpu* cpu = obtenerCPUporSocket(socketCPU,false);

				t_registroInformacionProceso * registro = recuperarInformacionProceso(cpu->PID);
				registro->rafagas++;
				cpu->quantum++;

				log_info(logger,"CPU %d tiene quantum disponible continuo ejecutando el proceso %d - Alg: RR",socketCPU,cpu->PID);

				socket_enviar(socketCPU,D_STRUCT_CONTINUAR_EJECUCION,quantumSleep);

			}
		}

	} else {

		if(list_size(listaCpuLibres)==0){
			log_info(logger,"No hay CPU disponible para la planificacion");
			return;
		}

		t_cpu* cpu = list_get(listaCpuLibres,0);
		obtenerCPUporSocket(cpu->socket,true);

		log_info(logger, "La cpu disponible para ejecutar la proxima rafaga es %d",cpu->socket);

		if(!string_equals_ignore_case(configuracion->algoritmo,"FIFO")) cpu->quantum++;

		ejecutarProximoProceso(cpu);

	}

}

void finalizarProcesoOK(int socketCPU, t_struct_pcb * pcbFinalizado){

	t_cpu * cpu = obtenerCPUporSocket(socketCPU,true);
	cpu->PID = -1;
	cpu->quantum = 0;
	list_add(listaCpuLibres,cpu);

	pcbFinalizado->exitcode=EC_FINALIZO_OK;
	actualizarPCBExec(pcbFinalizado);
	liberarMemoriaProceso(pcbFinalizado);
	informarLiberarHeap(pcbFinalizado);
	informarFinalizacionConsola(pcbFinalizado);
	removerDeCola(cola_exec,cola_exit,E_EXIT,pcbFinalizado->PID,true,true,true);
	traerProcesoColaNew();
	if(list_size(cola_ready)>0) ejecutarPlanificacion(0);

}

int verificarPaginaHeapDisponible(t_struct_sol_heap * solHeap){

	int indice, indice2;

	for(indice=0; indice < list_size(tablaHeap); indice++){

		t_registroTablaHeap * registroHeap = list_get(tablaHeap,indice);

		if(registroHeap->PID==solHeap->pid){

			for(indice2=0; indice2 < list_size(registroHeap->listaBloques); indice2++){

				t_bloqueHeap * bloqueHeap = list_get(registroHeap->listaBloques, indice2);

				if(bloqueHeap->numeroBloque>=0 && bloqueHeap->isFree && bloqueHeap->size >= solHeap->pointer){

					log_info(logger,"Se asigno %d bytes de la pagina %d al proceso %d",solHeap->pointer,registroHeap->numeroPagina,solHeap->pid);
					return registroHeap->numeroPagina;
				}

				if(bloqueHeap->numeroBloque<0 && bloqueHeap->isFree && bloqueHeap->size >= (solHeap->pointer + 5)){

					log_info(logger,"Se asigno %d bytes de un bloque nuevo en la pagina %d al proceso %d",solHeap->pointer,registroHeap->numeroPagina,solHeap->pid);
					return registroHeap->numeroPagina;

				}
			}
		}
	}

	return -1;

}

t_bloqueHeap * buscarUltimoBloque(t_list* listaBloques) {

	int maxBloque = 0;
	t_bloqueHeap * bloqueRetorno = malloc(sizeof(t_bloqueHeap));
	bloqueRetorno->numeroBloque = 0;

	int indice ;
	for (indice=0; indice < list_size(listaBloques); indice++) {

		t_bloqueHeap * bloque = list_get(listaBloques,indice);

		if (bloque->numeroBloque > maxBloque) {
			maxBloque = bloque->numeroBloque;
			bloqueRetorno = bloque;
		}
	}

	return bloqueRetorno;
}

t_bloqueHeap * crearBloqueHeap(uint32_t pointer, t_bloqueHeap * bloque, t_list* listaBloques){

	t_bloqueHeap * bloqueNuevo = malloc(sizeof(t_bloqueHeap));

	bloqueNuevo->isFree=false;
	bloqueNuevo->size=pointer;
	bloqueNuevo->numeroBloque=buscarUltimoBloque(listaBloques)->numeroBloque;
	bloqueNuevo->offset=bloque->offset;
	bloqueNuevo->fin=bloqueNuevo->offset+pointer;

	return bloqueNuevo;
}

void recalcularOffset(t_bloqueHeap* bloqueEspecial, t_bloqueHeap* bloqueNuevo) {

	bloqueEspecial->offset = bloqueEspecial->offset + bloqueNuevo->size + 5;
	bloqueEspecial->size = bloqueEspecial->size - bloqueNuevo->size - 5;

}

t_bloqueHeap * buscarPrimerBloqueHeap(t_struct_sol_heap * solicitudHeap, int pagina){

	int indice, indice2;
	t_bloqueHeap * bloqueHeap;

	for(indice=0; indice < list_size(tablaHeap); indice++){

		t_registroTablaHeap * registroHeap = list_get(tablaHeap,indice);
		if(registroHeap->numeroPagina==pagina){

			for(indice2=0; indice2 < list_size(registroHeap->listaBloques); indice2++){

				bloqueHeap = list_get(registroHeap->listaBloques,indice2);

				if(bloqueHeap->isFree && bloqueHeap->size >= solicitudHeap->pointer && bloqueHeap->numeroBloque >= 0){

					bloqueEspecial=0;
					bloqueHeap->isFree=false;
					bloqueHeap->fin=solicitudHeap->pointer;

					indice = list_size(tablaHeap);
					indice2 = list_size(registroHeap->listaBloques);

				}

				if(bloqueHeap->isFree && bloqueHeap->size >= (solicitudHeap->pointer+5) && bloqueHeap->numeroBloque < 0){

					bloqueEspecial=-1;
					t_bloqueHeap * bloqueNuevo = crearBloqueHeap(solicitudHeap->pointer, bloqueHeap, registroHeap->listaBloques);
					recalcularOffset(bloqueHeap,bloqueNuevo);

					t_bloqueHeap * bloqueMetadata = list_remove(registroHeap->listaBloques,indice2);

					list_add(registroHeap->listaBloques,bloqueNuevo);
					list_add(registroHeap->listaBloques,bloqueMetadata);

					bloqueHeap=bloqueNuevo;

					indice = list_size(tablaHeap);
					indice2 = list_size(registroHeap->listaBloques);
				}
			}
		}
	}
	return bloqueHeap;
}

t_registroTablaHeap * buscarPagina(int pagina, int PID){

	int indice;
	t_registroTablaHeap * registro;

	for(indice=0;indice< list_size(tablaHeap);indice++){

		registro = list_get(tablaHeap,indice);
		if(registro->PID==PID && registro->numeroPagina==pagina) return registro;

	}
	return registro;
}

int buscarPosicionBloque(int pagina, int offset, int PID){

	t_registroTablaHeap * registro=buscarPagina(pagina, PID);
	int indice;

	for(indice=0; indice<list_size(registro->listaBloques); indice++){

		t_bloqueHeap * bloque=list_get(registro->listaBloques, indice);
		if(bloque->offset==offset) return indice;

	}
	return -1;
}

int posicionBloqueMetadata(t_list * listaBloques){

	int indice;
	t_bloqueHeap * bloqueActual;

	for(indice=0; indice < list_size(listaBloques); indice++){
		bloqueActual=list_get(listaBloques,indice);
		if(bloqueActual->numeroBloque<0) return indice;
	}
	return -1;
}

t_struct_metadataHeap * generarHeapMetadata(int bytes, bool free){

	t_struct_metadataHeap * metadata = malloc(sizeof(t_struct_metadataHeap));
	metadata->isFree = free;
	metadata->size = bytes;

	return metadata;
}

int calcularNuevoNumeroPagina(int pid){
	t_struct_pcb* pcb =obtenerPCBActivo(pid);
	int max = pcb->paginasCodigo + pcb->paginasStack;
	int indice;

	for(indice=0; indice<list_size(tablaHeap);indice++){
		t_registroTablaHeap * registro = list_get(tablaHeap,indice);

		if(registro->numeroPagina>=max && registro->PID==pid){
			max = registro->numeroPagina;
			max++;
		}
	}
	return max;
}

t_registroTablaHeap * crearNuevaPagina(int bytes, int PID, int numeroPagina){

	t_registroTablaHeap* nuevaPagina = malloc(sizeof(t_registroTablaHeap));

	nuevaPagina->PID=PID;
	nuevaPagina->espacioMaximoBloque = tamanio_pagina - 5;
	nuevaPagina->numeroPagina=numeroPagina;
	nuevaPagina->listaBloques = list_create();

	t_bloqueHeap* bloqueMetadata = malloc(sizeof(t_bloqueHeap));
	bloqueMetadata->isFree = true;
	bloqueMetadata->numeroBloque=-1;
	bloqueMetadata->offset=5;
	bloqueMetadata->size=tamanio_pagina - 5;
	bloqueMetadata->fin=tamanio_pagina;

	list_add(nuevaPagina->listaBloques,bloqueMetadata);
	list_add(tablaHeap,nuevaPagina);

	return nuevaPagina;

}

int compactar(t_registroTablaHeap * paginaCompactar){

	int cantidadBloques = list_size(paginaCompactar->listaBloques);

	if (cantidadBloques > 1) {

		int i = 0;
		int j = 1;

		t_bloqueHeap* bloque1 = list_get(paginaCompactar->listaBloques, i);
		t_bloqueHeap* bloque2 = list_get(paginaCompactar->listaBloques, j);

		while (j < cantidadBloques) {

			if (bloque1->isFree && bloque2->isFree) {

				t_struct_sol_escritura * solCompactar = malloc(sizeof(t_struct_sol_escritura));
				solCompactar->PID = paginaCompactar->PID;
				solCompactar->pagina = paginaCompactar->numeroPagina;
				solCompactar->tamanio = 5;
				solCompactar->offset = bloque1->offset- 5;

				t_struct_metadataHeap* metadata = generarHeapMetadata(bloque1->size+bloque2->size+5, true);

				solCompactar->contenido = metadata;

				socket_enviar(socketMemoria,D_STRUCT_COMPACTAR_HEAP,solCompactar);

				t_tipoEstructura tipoEstructura;
				void * structRecibido;

				socket_recibir(socketMemoria,&tipoEstructura,&structRecibido);
				t_struct_numero * respuestaMemoria = ((t_struct_numero*) structRecibido);

				if(respuestaMemoria->numero==MEMORIA_OK){

					bloque1->size += bloque2->size + 5;

					log_info(logger,"Se compactaron los bloques %d y %d, de la pagina %d",i, j, paginaCompactar->numeroPagina);
					log_info(logger, "Quedaron compactados %d bytes",bloque1->size);

					list_remove(paginaCompactar->listaBloques, j);
					j = cantidadBloques;
					return 1 + compactar(paginaCompactar);

				}else{
					log_info(logger, "Falló la compactación de la pagina %d del proceso %d",paginaCompactar->numeroPagina, paginaCompactar->PID);
					j = cantidadBloques;
					return 0;
				}

			} else {

				i++;
				j++;
				bloque1 = list_get(paginaCompactar->listaBloques, i);
				bloque2 = list_get(paginaCompactar->listaBloques, j);

			}
		}

		return 0;

	}

	int cantidadFinal = list_size(paginaCompactar->listaBloques);

	t_bloqueHeap * bloque_especial = list_get(paginaCompactar->listaBloques,cantidadFinal - 1);

	bloque_especial->numeroBloque = -1;

	if (cantidadFinal == cantidadBloques) {
		log_info(logger, "No se compactó ningun bloque de la pagina %d",paginaCompactar->numeroPagina);
	}

	revisarPaginaslibres(paginaCompactar);

	return 0;

}

void borrarPaginaKernel(t_registroTablaHeap * pagina){
	int indice;
	int cantidadRegistrosHeap = list_size(tablaHeap);
	for(indice=0;indice<cantidadRegistrosHeap;indice++){
		t_registroTablaHeap* registro = list_get(tablaHeap,indice);
		if(registro->PID==pagina->PID && registro->numeroPagina==pagina->numeroPagina){
			list_remove(tablaHeap,indice);
			indice = cantidadRegistrosHeap;
		}
	}
}

void revisarPaginaslibres(t_registroTablaHeap * paginaRevisar){

	if (list_size(paginaRevisar->listaBloques) == 1) {

		t_struct_sol_heap * paginaLiberar = malloc(sizeof(t_struct_sol_heap));
		paginaLiberar->pid=paginaRevisar->PID;
		paginaLiberar->pointer=paginaRevisar->numeroPagina;

		socket_enviar(socketMemoria,D_STRUCT_LIBERAR_PAGINA,paginaLiberar);

		t_tipoEstructura tipoEstructura;
		void * structRecibido;

		socket_recibir(socketMemoria,&tipoEstructura,&structRecibido);
		t_struct_numero * respuestaMemoria = ((t_struct_numero*) structRecibido);


		if (respuestaMemoria->numero == MEMORIA_OK) {

			log_info(logger,"Se liberó la pagina %d utilizada por el proceso %d \n ",paginaRevisar->numeroPagina, paginaRevisar->PID);
			borrarPaginaKernel(paginaRevisar);

		} else {
			log_info(logger,"No se puso borrar la pagina %d del proceso %d\n ",	paginaRevisar->numeroPagina, paginaRevisar->PID);
		}
	}
}

void reservarHeap(int socketCPU, t_struct_sol_heap * solicitudHeap){

	t_struct_numero * respuesta = malloc(sizeof(t_struct_numero));

	if((solicitudHeap->pointer + 10) > tamanio_pagina){

		respuesta->numero=KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_ERROR_HEAP_MAX,respuesta);
		log_info(logger,"La solicitud de heap realizada por el PID %d supera el maximo permitido",solicitudHeap->pid);

	}

	t_registroInformacionProceso * registro = recuperarInformacionProceso(solicitudHeap->pid);
	registro->cantidad_solicitar_heap++;
	registro->total_heap_solicitado += solicitudHeap->pointer;

	t_bloqueHeap * bloqueAsignado;
	t_struct_sol_escritura * punteroStruct = malloc(sizeof(t_struct_sol_escritura));
	t_struct_sol_escritura * punteroStructEspecial = malloc(sizeof(t_struct_sol_escritura));

	int paginaHeap = verificarPaginaHeapDisponible(solicitudHeap);

	if(paginaHeap>=0){

		bloqueAsignado = buscarPrimerBloqueHeap(solicitudHeap,paginaHeap);

		if(bloqueAsignado->size > solicitudHeap->pointer + 5){

			int indice=buscarPosicionBloque(paginaHeap,bloqueAsignado->offset,solicitudHeap->pid);

			if(indice  >= 0){

			t_bloqueHeap * bloqueParticionado= malloc(sizeof(t_bloqueHeap));
			bloqueParticionado->size= bloqueAsignado->size-solicitudHeap->pointer-5;
			bloqueParticionado->isFree=true;
			bloqueParticionado->numeroBloque=0;
			bloqueParticionado->offset=bloqueAsignado->offset+solicitudHeap->pointer+5;
			bloqueParticionado->fin=bloqueParticionado->offset+bloqueParticionado->size;

			bloqueAsignado->size=solicitudHeap->pointer;
			bloqueAsignado->fin=bloqueAsignado->offset+solicitudHeap->pointer;

			t_registroTablaHeap * estructuraPagina = buscarPagina(paginaHeap, solicitudHeap->pid);
			list_add_in_index(estructuraPagina->listaBloques, indice+1, bloqueParticionado);

			t_struct_sol_escritura * punteroStructParticionado = malloc(sizeof(t_struct_sol_escritura));
			punteroStructParticionado->PID = solicitudHeap->pid;
			punteroStructParticionado->pagina = paginaHeap;
			punteroStructParticionado->tamanio = 5;
			punteroStructParticionado->offset = bloqueParticionado->offset- 5;

			t_struct_metadataHeap* heapMetadataParticionada = generarHeapMetadata(bloqueParticionado->size, true);

			punteroStructParticionado->contenido = heapMetadataParticionada;

			socket_enviar(socketMemoria,D_STRUCT_ESCRIBIR_HEAP,punteroStructParticionado);

			t_tipoEstructura tipoEstructura;
			void * structRecibido;

			socket_recibir(socketMemoria,&tipoEstructura,&structRecibido);

			}else {
				log_info(logger, "No se encontró el indice del bloque para agregar el bloque particionado");
			}
		}

		punteroStruct->PID = solicitudHeap->pid;
		punteroStruct->pagina = paginaHeap;
		punteroStruct->tamanio = solicitudHeap->pointer;
		punteroStruct->offset = bloqueAsignado->offset;

		t_puntero puntero = punteroStruct->pagina*tamanio_pagina + punteroStruct->offset;

		punteroStruct->offset = bloqueAsignado->offset - 5;
		punteroStruct->tamanio = 5;

		t_struct_metadataHeap * heapMetadataActual = generarHeapMetadata(solicitudHeap->pointer, true);

		punteroStruct->contenido = heapMetadataActual;

		t_registroTablaHeap * paginaStruct = buscarPagina(paginaHeap, solicitudHeap->pid);

		int posicionBE = list_size(paginaStruct->listaBloques);

		t_bloqueHeap * bloque_especial = list_get(paginaStruct->listaBloques,posicionBE - 1);

		punteroStructEspecial->PID = solicitudHeap->pid;
		punteroStructEspecial->pagina = paginaHeap;
		punteroStructEspecial->tamanio = 5;
		punteroStructEspecial->offset = bloque_especial->offset- 5;

		t_struct_metadataHeap* heapMetadataEspecial = generarHeapMetadata(bloque_especial->size, false);

		punteroStructEspecial->contenido = heapMetadataEspecial;

		socket_enviar(socketMemoria,D_STRUCT_ESCRIBIR_HEAP,punteroStruct);

		t_tipoEstructura tipoEstructura;
		void * structRecibido;

		bool rtaMemoria = false;
		socket_recibir(socketMemoria,&tipoEstructura,&structRecibido);
		t_struct_numero * respuestaMemoria = ((t_struct_numero*) structRecibido);

		if(respuestaMemoria->numero==MEMORIA_OK){
			rtaMemoria=true;
		}

		socket_enviar(socketMemoria,D_STRUCT_ESCRIBIR_HEAP,punteroStructEspecial);

		t_tipoEstructura tipoEstructura2;
		void * structRecibido2;

		bool rtaMemoriaEspecial = false;
		socket_recibir(socketMemoria,&tipoEstructura2,&structRecibido2);
		t_struct_numero * respuestaMemoriaEspecial = ((t_struct_numero*) structRecibido2);

		if(respuestaMemoriaEspecial->numero==MEMORIA_OK){
			rtaMemoriaEspecial=true;
		}

		if(rtaMemoria && rtaMemoriaEspecial){
			t_struct_numero * punteroEnviar = malloc(sizeof(t_struct_numero));
			punteroEnviar->numero=puntero;
			socket_enviar(socketCPU,D_STRUCT_RTA_HEAP,punteroEnviar);
		} else {
			t_struct_numero * punteroError = malloc(sizeof(t_struct_numero));
			punteroError->numero=KERNEL_ERROR;
			socket_enviar(socketCPU,D_STRUCT_ERROR_HEAP,punteroError);
		}

	} else {

		int respuesta = solicitarSegmentoCodigoStack(solicitudHeap->pid,tamanio_pagina);

		if(respuesta==MEMORIA_OK){

			paginaHeap = calcularNuevoNumeroPagina(solicitudHeap->pid);

			log_info(logger,"Se creo una nueva pagina de heap %d para el proceso %d",paginaHeap,solicitudHeap->pid);

			t_registroTablaHeap* nuevaPagina = crearNuevaPagina(solicitudHeap->pointer,solicitudHeap->pid,paginaHeap);

			int posBloqueMetadata = posicionBloqueMetadata(nuevaPagina->listaBloques);
			t_bloqueHeap* bloque_especial = list_get(nuevaPagina->listaBloques,posBloqueMetadata);

			bloqueAsignado = crearBloqueHeap(solicitudHeap->pointer, bloque_especial,nuevaPagina->listaBloques);
			recalcularOffset(bloque_especial, bloqueAsignado);

			list_remove(nuevaPagina->listaBloques, posBloqueMetadata);
			list_add(nuevaPagina->listaBloques, bloqueAsignado);
			list_add(nuevaPagina->listaBloques, bloque_especial);

			punteroStruct->PID = solicitudHeap->pid;
			punteroStruct->pagina = paginaHeap;
			punteroStruct->tamanio = solicitudHeap->pointer;
			punteroStruct->offset = bloqueAsignado->offset;

			t_puntero puntero = punteroStruct->pagina*tamanio_pagina + punteroStruct->offset;

			punteroStruct->offset = bloqueAsignado->offset - 5;
			punteroStruct->tamanio = 5;

			t_struct_metadataHeap * heapMetadataActual = generarHeapMetadata(solicitudHeap->pointer, false);

			punteroStruct->contenido = heapMetadataActual;

			punteroStructEspecial->PID = solicitudHeap->pid;
			punteroStructEspecial->pagina = paginaHeap;
			punteroStructEspecial->tamanio = 5;
			punteroStructEspecial->offset = bloque_especial->offset- 5;

			t_struct_metadataHeap* heapMetadataEspecial = generarHeapMetadata(bloque_especial->size, true);

			punteroStructEspecial->contenido = heapMetadataEspecial;

			socket_enviar(socketMemoria,D_STRUCT_ESCRIBIR_HEAP,punteroStruct);

			t_tipoEstructura tipoEstructura;
			void * structRecibido;

			bool rtaMemoria = false;
			socket_recibir(socketMemoria,&tipoEstructura,&structRecibido);
			t_struct_numero * respuestaMemoria = ((t_struct_numero*) structRecibido);

			if(respuestaMemoria->numero==MEMORIA_OK){
				rtaMemoria=true;
			}

			socket_enviar(socketMemoria,D_STRUCT_ESCRIBIR_HEAP,punteroStructEspecial);

			t_tipoEstructura tipoEstructura2;
			void * structRecibido2;

			bool rtaMemoriaEspecial = false;
			socket_recibir(socketMemoria,&tipoEstructura2,&structRecibido2);
			t_struct_numero * respuestaMemoriaEspecial = ((t_struct_numero*) structRecibido2);

			if(respuestaMemoriaEspecial->numero==MEMORIA_OK){
				rtaMemoriaEspecial=true;
			}

			if(rtaMemoria && rtaMemoriaEspecial){
				t_struct_numero * punteroEnviar = malloc(sizeof(t_struct_numero));
				punteroEnviar->numero=puntero;
				socket_enviar(socketCPU,D_STRUCT_RTA_HEAP,punteroEnviar);
			} else {
				t_struct_numero * punteroError = malloc(sizeof(t_struct_numero));
				punteroError->numero=KERNEL_ERROR;
				socket_enviar(socketCPU,D_STRUCT_ERROR_HEAP,punteroError);
			}


		} else {
			t_struct_numero * punteroError = malloc(sizeof(t_struct_numero));
			punteroError->numero=KERNEL_ERROR;
			socket_enviar(socketCPU,D_STRUCT_ERROR_HEAP,punteroError);
			log_info(logger,"No se pudieron asignar paginas de heap al proceso %d",solicitudHeap->pid);
		}
	}
}

void liberarHeap(int socketCPU, t_struct_sol_heap * solicitudHeap){

	t_struct_sol_escritura * solLiberar = malloc(sizeof(t_struct_sol_escritura));
	t_struct_numero * rtaLiberar = malloc(sizeof(t_struct_numero));
	t_registroInformacionProceso * registro;
	bool pudeLiberar = false;

	solLiberar->PID=solicitudHeap->pid;
	solLiberar->offset=solicitudHeap->pointer % tamanio_pagina;
	solLiberar->tamanio=4;
	solLiberar->pagina = solicitudHeap->pointer / tamanio_pagina;

	int numeroPagina, nroBloque;
	int indice, indice2;
	int size;

	int tamanioTabla = list_size(tablaHeap);

	for(indice=0; indice < tamanioTabla; indice++){

		t_registroTablaHeap * pagina = list_get(tablaHeap,indice);
		if(pagina->PID==solicitudHeap->pid && pagina->numeroPagina == solLiberar->pagina){

			int cantidadBloques = list_size(pagina->listaBloques);

			for(indice2=0; indice2 < cantidadBloques; indice2++){
				t_bloqueHeap* bloqueBorrar = list_get(pagina->listaBloques,indice2);

				registro = recuperarInformacionProceso(solicitudHeap->pid);
				registro->cantidad_liberar_heap++;
				registro->total_heap_liberado+=bloqueBorrar->size;

				if(bloqueBorrar->offset == solLiberar->offset){
					pudeLiberar=true;
					bloqueBorrar->isFree=true;
					size = bloqueBorrar->size;
					numeroPagina = indice;
					nroBloque = indice2;
					indice = tamanioTabla;
					indice2 = cantidadBloques;
				}
			}
		}
	}

	if(!pudeLiberar){

		rtaLiberar->numero=KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,rtaLiberar);
		return;
	}

	solLiberar->tamanio = 5;
	solLiberar->offset= solLiberar->offset-5;

	t_struct_metadataHeap* heap = generarHeapMetadata(size,true);

	solLiberar->contenido = heap;

	socket_enviar(socketMemoria,D_STRUCT_LIBERAR_HEAP,solLiberar);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	socket_recibir(socketMemoria,&tipoEstructura,&structRecibido);
	t_struct_numero * respuestaMemoria = ((t_struct_numero*) structRecibido);

	registro->syscall++;

	if(respuestaMemoria->numero==MEMORIA_OK){
		rtaLiberar->numero=KERNEL_OK;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,rtaLiberar);

		t_registroTablaHeap * paginaBorrar = list_get(tablaHeap,numeroPagina);
		t_bloqueHeap * bloqueBorrar = list_get(paginaBorrar->listaBloques,nroBloque);
		bloqueBorrar->isFree=true;

		log_info(logger,"El proceso con PID %d pudo liberar un bloque de heap",solicitudHeap->pid);

		compactar(paginaBorrar);

		return;

	} else {
		rtaLiberar->numero=KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,rtaLiberar);
		return;
	}

}

void iniciarConsolaKernel(){

	printf("Esta es la consola del Kernel por favor, ingrese la operación que desea realizar: \n");
	printf("PROCESOS - Lista los procesos de acuerdo al estado indicado en el siguiente paso \n");
	printf("INFOPROCESO - Permite obtener información de un proceso en particular \n");
	printf("ARCHIVOS - Permite visualizar el contenido actual de la tabla global de archivos \n");
	printf("GRADO - Modificar el grado de multiprogramacion del sistema \n");
	printf("FINALIZAR - Permite finalizar un proceso en particular \n");
	printf("PLANIFIACION - Permite detener o reanudar la planificacion del kernel \n");
	printf("\n");

}

void mensajeConsolaKernel(){
	char * operacion = malloc(50);
	scanf("%s", operacion);
	
	switch(commandParser(operacion)){
			case 1:;
				puts("Ingrese uno de los siguientes estados que desee visualizar \n");
				puts("NEW - READY - EXEC - BLOCK - EXIT - TODOS \n");

				char * estado = malloc(20);
				scanf("%s", estado);

				string_to_upper(estado);

				imprimirProcesos(estado);

				free(estado);

				break;
			case 2:
				puts("Ingrese el Process ID del que desea obtener informacion");
				int pid;
				scanf("%i",&pid);

				puts("Ingrese una de las siguientes opciones sobre la cual quiera obtener informacion \n");
				puts("SYSCALL - RAFAGAS - ARCHIVOS - HEAP \n");

				char * info = malloc(20);
				scanf("%s", info);

				recuperarInformacion(pid,info);

				break;
			case 3:
				imprimirTablaGlobalArchivos();

				break;
			case 4:
				puts("Ingrese el nuevo valor del grado de multiprogramación");
				int grado;
				scanf("%i",&grado);

				finalizarProcesoConsola(pid);

				break;
			case 5:

				puts("Ingrese el Process ID que desea finalizar");
				int pidFinalizar;
				scanf("%i",&pidFinalizar);

				finalizarProcesoConsola(pidFinalizar);

				break;
			case 6: ;
				kernelPlanificando = !kernelPlanificando;

				if(kernelPlanificando){
					printf("Se restauro la planificacion del kernel");
				} else {
					printf("Se detuvo la planificacion del kernel");
				}

				break;
			default:
				printf("Comando invalido...\n");
				break;
			}
		free(operacion);
}

int commandParser(char* operacion){

	string_to_upper(operacion);

	if(strcmp(operacion, "PROCESOS") == 0){
		return 1;
	} else if (strcmp(operacion, "INFOPROCESO") == 0){
		return 2;
	} else if(strcmp(operacion, "ARCHIVOS") == 0){
		return 3;
	} else if (strcmp(operacion, "GRADO") == 0){
		return 4;
	} else if (strcmp(operacion, "FINALIZAR") == 0){
		return 5;
	} else if (strcmp(operacion, "PLANIFICACION") == 0){
		return 6;
	}else{
		return 7;
	}

}

void imprimirProcesos(char* estado){
	if(strcmp(estado, "NEW") == 0){
		listarProcesosEnCola(cola_new, estado);
	} else if (strcmp(estado, "READY") == 0){
		listarProcesosEnCola(cola_ready, estado);
	} else if(strcmp(estado, "EXEC") == 0){
		listarProcesosEnCola(cola_exec, estado);
	} else if (strcmp(estado, "BLOCK") == 0){
		listarProcesosEnCola(cola_block, estado);
	} else if (strcmp(estado, "EXIT") == 0){
		listarProcesosEnCola(cola_exit, estado);
	} else if (strcmp(estado, "TODOS") == 0){
		listarProcesosEnCola(cola_new, "NEW");
		listarProcesosEnCola(cola_ready, "READY");
		listarProcesosEnCola(cola_exec, "EXEC");
		listarProcesosEnCola(cola_block, "BLOCK");
		listarProcesosEnCola(cola_exit, "EXIT");
	} else {
		printf("La cola ingresada es incorrecta, por favor vuelva ingresar PROCESOS para solicitar nuevamente \n");
	}
}

void listarProcesosEnCola(t_list * cola, char * estado){
	
	if(list_size(cola)==0){
		printf("La cola de %s solicitada no tiene procesos \n", estado);
	}

	int indice;

	for(indice=0; indice<list_size(cola); indice++){
		t_struct_pcb * pcbRecuperado = list_get(cola, indice);
		printf("Proceso PID: %d se encuentra en Estado: %s \n", pcbRecuperado->PID, estado);
		fflush(stdout);		
	}

}

void recuperarInformacion(int pid, char * info){
	if(strcmp(info, "SYSCALL") == 0){
		imprimirSyscallProceso(pid);
	} else if (strcmp(info, "RAFAGAS") == 0){
		imprimirRafagasProceso(pid);
	} else if(strcmp(info, "ARCHIVOS") == 0){
		imprimirArchivosProceso(pid);
	} else if (strcmp(info, "HEAP") == 0){
		imprimirHeapProceso(pid);
	} else {
		printf("La opcion ingresada es incorrecta ingrese INFOPROCESO para solicitar nuevamente \n");
	}
}

void imprimirSyscallProceso(int pid){
	int indice;

	for (indice=0; indice < list_size(listaInformacionProcesos); indice++) {
		t_registroInformacionProceso * registro = list_get(listaInformacionProcesos, indice);
		if (registro->pid == pid) {
			printf("El proceso ejecuto %d syscalls. \n", registro->syscall);
			return;
		}
	}
	printf("El pid ingresado es inexistente \n");
}

void imprimirRafagasProceso(int pid){
	int indice;

	for (indice=0; indice < list_size(listaInformacionProcesos); indice++) {
		t_registroInformacionProceso * registro = list_get(listaInformacionProcesos, indice);
		if (registro->pid == pid) {
			printf("El proceso ejecuto %d rafagas. \n", registro->rafagas);
			return;
		}
	}
	printf("El pid ingresado es inexistente \n");
}

void imprimirArchivosProceso(int pid){
	
	t_list* archivosProceso = dictionary_get(tablaArchivosProceso, string_itoa(pid));

	if(list_size(archivosProceso)==0){
		printf("El pid ingresado no existe");
		return;
	}

	int indice;
	bool tieneArchivos = false;

	for (indice=3; indice < list_size(archivosProceso); indice++) {
		t_registroArchivosProc * registro = list_get(archivosProceso, indice);
		printf("-------Archivo del proceso FD: %d------- \n",indice);
		printf("FD de la tabla global: %d, Cursor posicionado en: %d \n",registro->fd_TablaGlobal, registro->cursor);
		if(registro->flags.lectura) printf("Abierto en modo lectura \n");
		if(registro->flags.escritura) printf("Abierto en modo escritura \n");
		if(registro->flags.creacion) printf("Abierto en modo creacion \n");

		tieneArchivos=true;
	}
	if(!tieneArchivos){
		printf("El pid ingresado no tiene archivos abiertos \n");
	}
}

void imprimirHeapProceso(int pid){
	int indice;
	int paginasHeap = 0;
	for (indice=0; indice < list_size(tablaHeap); indice++){
		t_registroTablaHeap * registro = list_get(tablaHeap,indice);
		if (registro->PID == pid) {
			paginasHeap++;
		}
	}

	printf("El proceso tiene asignadas %d paginas de heap", paginasHeap);

	for (indice=0; indice < list_size(listaInformacionProcesos); indice++) {
		t_registroInformacionProceso * registro = list_get(listaInformacionProcesos, indice);
		if (registro->pid == pid) {
			printf("El proceso solicito %d operaciones Alloc por un total de %d bytes \n",
			 registro->cantidad_solicitar_heap, registro->total_heap_solicitado);
			printf("El proceso solicito %d operaciones Liberar por un total de %d bytes \n",
			 registro->cantidad_liberar_heap, registro->total_heap_liberado);
			return;
		}
	}
}

void imprimirTablaGlobalArchivos(){
	int indice;
	t_registroArchivosGlobal * registro;

	for(indice=0; indice < list_size(tablaArchivosGlobal); indice++){
		registro = list_get(tablaArchivosGlobal,indice);
		printf("Archivo con path: %s abierto %d veces \n",registro->nombre, registro->cantidadAbierto);
	}

	if(registro==NULL) printf("No hay archivos abiertos en el sistema");

}

void finalizarProcesoConsola(int pid){

	t_struct_pcb * pcbRecuperado = obtenerPCBActivo(pid);

	if(pcbRecuperado==NULL){
		printf("El proceso no existe");
		return;
	}

	pcbRecuperado->exitcode = EC_FINALIZADO_CONSOLA_KERNEL;

	if(pcbRecuperado->estado==E_EXEC){
		printf("El proceso esta ejecutando, se marca para finalizar luego de la rafaga actual");
		t_registroProcesoDestruir * pidFinalizar = malloc(sizeof(t_registroProcesoDestruir));
		pidFinalizar->pid=pcbRecuperado->PID;
		list_add(listaProcesosFinalizar,pidFinalizar);
	} else {
		printf("El proceso no esta ejecutando, se finaliza");
		pasarColaExit(pcbRecuperado);
		liberarMemoriaProceso(pcbRecuperado);
		informarLiberarHeap(pcbRecuperado);
		informarFinalizacionConsola(pcbRecuperado);
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
