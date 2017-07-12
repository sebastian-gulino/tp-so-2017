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

	config_destroy(config);

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

	listaInformacionProcesos = list_create();

	maximoPID = 1;
	kernelPlanificando = true;

	// Listas planificacion
	cola_new = list_create();
	cola_exit = list_create();
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

			t_cpu * nuevaCPU = malloc(sizeof(t_cpu));
			nuevaCPU->socket=socketCliente;
			nuevaCPU->PID=-1;
			nuevaCPU->quantum=0;

			list_add(listaCpuLibres, nuevaCPU);

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

void manejarCpu(int socketCPU){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;


	if (socket_recibir(socketCPU,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"El Cpu %d cerró la conexión.",socketCPU);
		removerClientePorCierreDeConexion(socketCPU,listaCpuLibres,&master_cpu);
		//TODO Incorporar la logica para liquidar procesos
	} else {

		switch(tipoEstructura){
		case D_STRUCT_OBTENER_COMPARTIDA: ;

			// La cpu quiere obtener el valor de una variable compartida
			char * obtenerVarCompartida = ((t_struct_string*) structRecibido)->string ;
			obtenerVariableCompartida(socketCPU, obtenerVarCompartida);

			break;

		case D_STRUCT_GRABAR_COMPARTIDA: ;

			// La cpu quiere asignar el valor de una variable compartida
			t_struct_var_compartida * grabarVarCompartida = ((t_struct_var_compartida*) structRecibido);
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

		case D_STRUCT_ARCHIVO_ABR: ;

			// La cpu quiere abrir un archivo
			t_struct_archivo * archivoAbrir = ((t_struct_archivo*) structRecibido);
			abrirArchivo(socketCPU,archivoAbrir);

			break;

		case D_STRUCT_ARCHIVO_BOR: ;

			// La cpu quiere borrar un archivo
			t_struct_archivo * archivoBorrar = ((t_struct_archivo*) structRecibido);
			borrarArchivo(socketCPU,archivoBorrar);

			break;

		case D_STRUCT_ARCHIVO_CER: ;

			// La cpu quiere cerrar un archivo
			t_struct_archivo * archivoCerrar = ((t_struct_archivo*) structRecibido);
			cerrarArchivo(socketCPU, archivoCerrar);

			break;

		case D_STRUCT_ARCHIVO_MOV: ;

			// La cpu quiere mover el cursor dentro de un archivo
			t_struct_archivo * archivoMover = ((t_struct_archivo*) structRecibido);
			moverCursorArchivo(socketCPU, archivoMover);

			break;

		case D_STRUCT_ARCHIVO_ESC: ;

			// La cpu quiere escribir en un archivo
			t_struct_archivo * archivoEscribir = ((t_struct_archivo*) structRecibido);
			escribirArchivo(socketCPU, archivoEscribir);

			break;

		}

	}
};

void manejarConsola(int socketConsola){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(socketConsola,&tipoEstructura,&structRecibido) == -1) {

		log_info(logger,"La Consola %d cerró la conexión.",socketConsola);
		abortarPrograma(socketConsola,false);
		removerClientePorCierreDeConexion(socketConsola,listaConsolas,&master_consola);

	} else {

		switch(tipoEstructura){
		case D_STRUCT_PROG: ;
			// La consola envia un programa para ejecutar

			int tamanio_programa = ((t_struct_programa*) structRecibido)->tamanio ;
			char * programa = malloc(tamanio_programa);
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
	pcb->exitcode=99;
	pcb->cantRegistrosStack=0;

	registroStack * registro_stack = reg_stack_create();
	list_add(pcb->indiceStack,registro_stack);

	pcb->cantRegistrosStack++;

	crearArchivosPorProceso(pcb->PID);

	crearInformacionProcesoInicial(pcb->PID);

// TODO VER si hace falta
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

void enviarCodigoMemoria(char * programa,int tamanioPrograma, t_struct_pcb * pcb){

	int cantidadPaginasCodigo = tamanioPrograma / tamanio_pagina;

	if(tamanioPrograma%tamanio_pagina>0) cantidadPaginasCodigo++;

	int indice;
	int cantCodigoEnviado=0;
	int cantCodigoPendiente=tamanioPrograma;
	int tamanioEnvio;

	for(indice=0; indice < cantidadPaginasCodigo; indice++ ){

		char * codigoPendiente=string_new();
		char * codigoEnviar=string_new();

		t_struct_sol_escritura * escrituraCodigo = malloc(sizeof(t_struct_sol_escritura));
		escrituraCodigo->PID=pcb->PID;
		escrituraCodigo->offset=0;
		escrituraCodigo->pagina=indice;
		escrituraCodigo->contenido=tamanio_pagina;

		//TODO handlear estas solicitudes desde la memoria
		socket_enviar(socketMemoria,D_STRUCT_ESCRITURA_CODIGO,escrituraCodigo);

		tamanioEnvio = cantCodigoPendiente>tamanio_pagina ? tamanio_pagina : cantCodigoPendiente;

		codigoPendiente = string_substring_from(programa,cantCodigoEnviado);
		codigoEnviar = string_substring_until(codigoPendiente,tamanioEnvio);

		t_struct_programa* programa = malloc(sizeof(t_struct_programa));
		programa->tamanio = tamanioEnvio;
		programa->buffer = malloc(tamanioEnvio);
		programa->base = 1;
		programa->PID = 1 ;
		memcpy(programa->buffer,codigoEnviar,tamanioEnvio);

		socket_enviar(socketMemoria,D_STRUCT_CODIGO,programa);

		free(programa->buffer);
		free(programa);
		free(codigoPendiente);
		free(codigoEnviar);

		cantCodigoEnviado += tamanioEnvio;
		cantCodigoPendiente -= tamanioEnvio;

	}

	log_info(logger,"Se envio el codigo del proceso %d a la memoria",pcb->PID);
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

	dictionary_put(tablaArchivosProceso,string_itoa(PID),archivosProceso);
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

		return 1;

	}

	log_info(logger,"No se pudieron reservar las paginas al proceso %d",pcb->PID);
	return -1;

}

void inicializarProceso(int socketConsola, char * programa, int tamanio_programa){

	t_struct_pcb * pcb = NULL;
	t_struct_numero* pid_struct = malloc(sizeof(t_struct_numero));

	if(cantidadTotalPID<configuracion.gradoMultiprog){

		pcb = crearPCB(obtener_pid());

		agregarColaListos(pcb);

		// Reservo las paginas para codigo y stack
		if (reservarPaginas(pcb,programa,tamanio_programa) == -1) {

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

			enviarCodigoMemoria(programa,tamanio_programa,pcb);

			t_metadata_program* datosPrograma = metadata_desde_literal(programa);

			pcb->quantum=configuracion.quantum;
			pcb->quantum_sleep=configuracion.quantumSleep;
			pcb->programCounter=datosPrograma->instruccion_inicio;
			pcb->rafagas=0;
			pcb->tamanioIndiceEtiquetas=datosPrograma->etiquetas_size;
			pcb->cantidadInstrucciones=datosPrograma->instrucciones_size;

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
		socket_enviar(socketConsola, D_STRUCT_NUMERO, pid_struct);
		free(pid_struct);

	}

	if (list_size(cola_ready) > 0 && list_size(listaCpuLibres) > 0) {
		//TODO implementar.
		//planificar(NULL);
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

void liberarSemaforo(t_nombre_semaforo* semaforo){

	int indice;

	t_struct_semaforo * semaforoRecuperado;

	for (indice=0; indice < list_size(listaSemaforos); indice++){
		semaforoRecuperado = list_get(listaSemaforos,indice);
		if(string_equals_ignore_case(semaforoRecuperado->nombre,&semaforo)){
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

	//TODO manejar en memoria para liberar
	socket_enviar(socketMemoria,D_STRUCT_LIBERAR_MEMORIA, PID);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if(socket_recibir(socketMemoria,&tipoEstructura,&structRecibido) == -1){

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

	t_struct_numero * PID = malloc(sizeof(t_struct_numero));
	PID->numero = pcb->PID;

	socket_enviar(proceso->socket,D_STRUCT_FIN_PROG,PID);
	free(PID);
	eliminarProcesoLista(proceso);
}

void abortarPrograma(int socketConsola, bool finalizarPrograma){

	int PID = obtenerPIDporConsola(socketConsola);

	if(PID ==-1) {
		log_error(logger,"La consola %d no esta ejecutando ningun proceso, puede cerrarse", socketConsola);
		return;
	}

	eliminarRegistroProceso(socketConsola);

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
		int * pidFinalizar = malloc(sizeof(t_registroTablaProcesos));
		pidFinalizar=PID;
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

		if(string_equals_ignore_case(variableRecuperada->nombre,varCompartida)){

			t_struct_numero * valorCompartida = malloc(sizeof(t_struct_numero));
			valorCompartida->numero=variableRecuperada->valor;

			socket_enviar(socketCPU,D_STRUCT_NUMERO,valorCompartida);
			free(valorCompartida);

			break;
		}
	}

	free(variableRecuperada);

}

void grabarVariableCompartida(int socketCPU, t_struct_var_compartida * grabarVarCompartida){

	t_struct_var_compartida * variableRecuperada;
	int indice;

	for(indice=0; indice < list_size(listaVarCompartidas); indice++){
		variableRecuperada = list_get(listaVarCompartidas,indice);

		if(string_equals_ignore_case(variableRecuperada->nombre,grabarVarCompartida->nombre)){

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
		int * PIDRecuperado = list_get(listaProcesosFinalizar,indice);
		if(&PIDRecuperado == pcb->PID){
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

	t_registroTablaProcesos * proceso = obtenerConsolaPorPID(pcbNew->PID);

	t_struct_numero * pidNew = malloc(sizeof(t_struct_numero));
	pidNew->numero=pcbNew->PID;

	// TODO HANDLEAR EN LA CONSOLA
	socket_enviar(proceso->socket,D_STRUCT_SOLICITAR_CODIGO,pidNew);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	socket_recibir(proceso->socket,&tipoEstructura,&structRecibido);

	int tamanio_programa = ((t_struct_programa*) structRecibido)->tamanio ;
	char * programa = malloc(tamanio_programa);

	memcpy(programa, ((t_struct_programa*) structRecibido)->buffer, tamanio_programa);

	t_struct_numero* pid_struct = malloc(sizeof(t_struct_numero));

	if (reservarPaginas(programa,pcbNew,tamanio_programa) == -1) {

		// Si no pude asignarlas aviso al a consola del rechazo
		pid_struct->numero = -1;
		socket_enviar(proceso->socket, D_STRUCT_NUMERO, pid_struct);
		free(pid_struct);

	} else {

		pid_struct->numero = pcbNew->PID;
		socket_enviar(proceso->socket, D_STRUCT_NUMERO, pid_struct);
		free(pid_struct);

		enviarCodigoMemoria(programa,tamanio_programa,pcbNew);

		t_metadata_program* datosPrograma = metadata_desde_literal(programa);

		pcbNew->quantum=configuracion.quantum;
		pcbNew->quantum_sleep=configuracion.quantumSleep;
		pcbNew->programCounter=datosPrograma->instruccion_inicio;
		pcbNew->rafagas=0;
		pcbNew->tamanioIndiceEtiquetas=datosPrograma->etiquetas_size;

		memcpy(pcbNew->indiceEtiquetas,datosPrograma->etiquetas,datosPrograma->etiquetas_size);

		int i;
		for (i = 0;	i < datosPrograma->instrucciones_size;i++) {

			t_intructions instruccion =	datosPrograma->instrucciones_serializado[i];

			t_intructions* agregarInst = malloc(sizeof(t_intructions));

			agregarInst->offset = instruccion.offset;
			agregarInst->start = instruccion.start;

			list_add(pcbNew->indiceCodigo, agregarInst);
		}

		metadata_destruir(datosPrograma);

	}

	agregarColaListos(pcbNew);
	free(programa);
	free(structRecibido);

	if(list_size(listaCpuLibres)>0){
		//TODO Implementar
		//planificar(NULL);
	}

}

void desbloquearProcesoEnWait(t_struct_semaforo * semaforoRecuperado){

	int indice;
	t_struct_pcb * pcbRecuperado;

	for(indice=0;indice < list_size(cola_block);indice++){
		pcbRecuperado = list_get(cola_block,indice);
		t_registroInformacionProceso * registro = recuperarInformacionProceso(pcbRecuperado->PID);

		if(string_equals_ignore_case(registro->semaforo_bloqueo,semaforoRecuperado->nombre)){
			removerDeCola(cola_block,cola_ready,E_READY,pcbRecuperado->PID,false,false,false);
			free(registro->semaforo_bloqueo);registro->semaforo_bloqueo = string_new();

			if(list_size(listaCpuLibres)>0){
				// TODO Implementar
				// planificar(NULL);
			}
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

		if(string_equals_ignore_case(semaforoRecuperado->nombre,waitSemaforo)){

			log_info(logger,"CPU %d realiza wait sobre el semaforo %s",socketCPU,waitSemaforo);
			encontreSemaforo = true;
			semaforoRecuperado->valor--;

			// Si el semaforo queda menor a 0 el proceso queda bloqueado lo informo con un 1, caso contrario 0
			respuesta->numero = semaforoRecuperado->valor<0 ? 1 : 0;

			socket_enviar(socketCPU,D_STRUCT_NUMERO,respuesta);
			free(respuesta);

			if(semaforoRecuperado->valor<0){

				t_tipoEstructura tipoEstructura;
				void * structRecibido;

				log_info(logger,"El semaforo se bloquea, verifico si corresponde matar el proceso o pasarlo a block");

				t_struct_pcb * pcbBloqueado = ((t_struct_pcb*) structRecibido);
				//TODO actualizar exit code con mensaje por semaforo bloqueado

				if(verificarProcesoFinalizar(pcbBloqueado)){

					log_info(logger,"Corresponde finalizar el proceso %d",pcbBloqueado->PID);

					actualizarPCBExec(pcbBloqueado);
					pasarColaExit(pcbBloqueado);
					liberarMemoriaProceso(pcbBloqueado);
					informarFinalizacionConsola(pcbBloqueado);
					informarLiberarHeap(pcbBloqueado);
					traerProcesoColaNew();

					t_cpu* cpuProcesando = obtenerCPUporSocket(socketCPU, true);
					list_add(listaCpuLibres,cpuProcesando);

					semaforoRecuperado->valor++;
				}else{
					log_info(logger,"Corresponde pasar a block el proceso %d",pcbBloqueado->PID);

					actualizarPCBExec(pcbBloqueado);

					t_registroInformacionProceso * registro = recuperarInformacionProceso(pcbBloqueado->PID);
					free(registro->semaforo_bloqueo);registro->semaforo_bloqueo=string_new();

					string_append(&(registro->semaforo_bloqueo),waitSemaforo);

					t_cpu* cpuProcesando = obtenerCPUporSocket(socketCPU, true);
					list_add(listaCpuLibres,cpuProcesando);

					removerDeCola(cola_exec,cola_block,E_BLOCK,pcbBloqueado->PID,false,false,false);
				}

				if (list_size(cola_ready) > 0) {
					planificar(NULL);
				}
			}
		}
	}

	if(!encontreSemaforo){
		respuesta->numero=-1;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,respuesta);
		free(respuesta);
	}

}

void realizarSignalSemaforo(socketCPU,signalSemaforo){
	t_struct_semaforo * semaforoRecuperado;
	bool encontreSemaforo = false;
	t_struct_numero * respuesta = malloc(sizeof(t_struct_numero));
	int indice;

	for(indice=0; indice < list_size(listaSemaforos); indice++){
		semaforoRecuperado = list_get(listaSemaforos,indice);

		if(string_equals_ignore_case(semaforoRecuperado->nombre,signalSemaforo)){

			log_info(logger,"CPU %d realiza signal sobre el semaforo %s",socketCPU,signalSemaforo);
			encontreSemaforo = true;
			semaforoRecuperado->valor++;
			respuesta->numero = KERNEL_OK;

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

	t_list * tablaArchivosProceso = dictionary_get(tablaArchivosProceso,string_itoa(archivo->pid));

	if(tablaArchivosProceso==NULL || archivo->fileDescriptor<3){
		return NULL;
	} else {
		t_registroArchivosProc * registroArchivoProceso =list_get(tablaArchivosProceso,archivo->fileDescriptor);

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

	t_list * tablaArchivosProceso = dictionary_get(tablaArchivosProceso,string_itoa(archivo->pid));

	t_registroArchivosProc * registroArchivoProceso = malloc(sizeof(t_registroArchivosProc));

	registroArchivoProceso->flags=archivo->flags;
	registroArchivoProceso->fd_TablaGlobal = obtenerArchivoTablaGlobal(archivo);

	socket_enviar(socketFS,D_STRUCT_ARCHIVO_ABR,archivo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	socket_recibir(socketFS,&tipoEstructura,&structRecibido);

	t_struct_abrir * respuestaAbrir = ((t_struct_abrir*) structRecibido);

	if(respuestaAbrir->confirmacion==FS_ABRIR_CREAR_OK ||
			respuestaAbrir->confirmacion==FS_ABRIR_NO_CREAR_OK){
		//Existia y lo abri ok o no existia pero lo pude crear
		if(registroArchivoProceso->fd_TablaGlobal==-1){
			//Si no existe en la tabla global de archivos
			t_registroArchivosGlobal * registroGlobal = malloc(sizeof(t_registroArchivosGlobal));
			registroGlobal->cantidadAbierto = 1;
			registroGlobal->nombre = string_new();
			string_append(registroGlobal->nombre,archivo->informacion);
			int fd_TablaGlobal = list_add(tablaArchivosGlobal,registroGlobal);

			registroArchivoProceso->fd_TablaGlobal = fd_TablaGlobal;
			registroArchivoProceso->cursor=0;

			t_descriptor_archivo newFDProceso = list_size(tablaArchivosProceso);
			list_add(tablaArchivosProceso,registroArchivoProceso);

			resultadoAbrir->numero = newFDProceso;

			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoAbrir);
			log_info(logger, "El archivo solicitado por el proceso %d es fd %d", archivo->pid,newFDProceso);

			free(resultadoAbrir);

			return;

		} else {
			//SI existe en la tabla global de archivos
			t_registroArchivosGlobal * registroGlobal =
					list_get(tablaArchivosGlobal,registroArchivoProceso->fd_TablaGlobal);

			registroGlobal->cantidadAbierto++;

			registroArchivoProceso->cursor=0;

			t_descriptor_archivo newFDProceso = list_size(tablaArchivosProceso);
			list_add(tablaArchivosProceso,registroArchivoProceso);

			resultadoAbrir->numero = newFDProceso;

			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoAbrir);
			log_info(logger,"El archivo solicitado por el proceso %d es fd %d", archivo->pid,newFDProceso);

			free(resultadoAbrir);

			return;
		}

	} else {

		resultadoAbrir->numero = KERNEL_ERROR;

		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoAbrir);
		log_info(logger,"El archivo solicitado por el proceso %d no se pudo abrir ni crear", archivo->pid);

		free(resultadoAbrir);
	}

}

void borrarArchivo(int socketCPU,t_struct_archivo * archivo){

	t_struct_numero * resultadoBorrar = malloc(sizeof(t_struct_numero));

	t_registroInformacionProceso * registroInfo = recuperarInformacionProceso(archivo->pid);
	registroInfo->syscall++;

	t_registroArchivosProc * registroArchivoProceso = obtenerRegistroTablaProceso(archivo);
	if(registroArchivoProceso==NULL){
		resultadoBorrar->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
		log_info(logger,"El proceso %d no tiene archivos abiertos", &(archivo->pid));

		free(resultadoBorrar);
		return;
	}

	t_registroArchivosGlobal * registroGlobal = obtenerRegistroTablaGlobal(registroArchivoProceso);
	if(registroGlobal==NULL){
		resultadoBorrar->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
		log_info(logger,"El proceso %d no tiene archivos abiertos", &(archivo->pid));

		free(resultadoBorrar);
		return;
	}

	if(registroGlobal->cantidadAbierto!=1){

		resultadoBorrar->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
		log_info(logger,"El archivo %d esta abierto por mas de un proceso", archivo->fileDescriptor);

		free(resultadoBorrar);

	} else {
		//Puedo borrarlo, se lo pido al FS..
		socket_enviar(socketFS,D_STRUCT_ARCHIVO_BOR,archivo);

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
}

void cerrarArchivo(int socketCPU,t_struct_archivo * archivo){

	t_struct_numero * resultadoBorrar = malloc(sizeof(t_struct_numero));

	t_registroInformacionProceso * registroInfo = recuperarInformacionProceso(archivo->pid);
	registroInfo->syscall++;

	t_list * tablaArchivosProceso = dictionary_get(tablaArchivosProceso,string_itoa(archivo->pid));

	if(tablaArchivosProceso==NULL || archivo->fileDescriptor<3){

		resultadoBorrar->numero = KERNEL_ERROR;
		socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
		log_info(logger,"El proceso %d no tiene archivos abiertos", &(archivo->pid));

		free(resultadoBorrar);

	} else {

		t_registroArchivosProc * registroArchivoProceso =
				list_get(tablaArchivosProceso,archivo->fileDescriptor);

		if(registroArchivoProceso==NULL){

			resultadoBorrar->numero = KERNEL_ERROR;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
			log_info(logger,"El proceso %d no tiene abierto el archivo solicitado", archivo->pid);

			free(resultadoBorrar);

		} else {

			list_remove(tablaArchivosProceso,archivo->fileDescriptor);

			t_registroArchivosGlobal * registroGlobal =
					list_get(tablaArchivosGlobal,registroArchivoProceso->fd_TablaGlobal);

			if(registroGlobal->cantidadAbierto==1){
				list_remove(tablaArchivosGlobal,registroArchivoProceso->fd_TablaGlobal);
			} else{
				registroGlobal->cantidadAbierto--;
			}

			resultadoBorrar->numero = KERNEL_OK;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoBorrar);
			log_info(logger,"El proceso %d pudo cerrar el archivo solicitado", archivo->pid);

			free(resultadoBorrar);
		}
	}
}

void moverCursorArchivo(int socketCPU,t_struct_archivo * archivo){

	t_struct_numero * resultadoMover = malloc(sizeof(t_struct_numero));

	t_registroInformacionProceso * registroInfo = recuperarInformacionProceso(archivo->pid);
	registroInfo->syscall++;

	t_list * tablaArchivosProceso = dictionary_get(tablaArchivosProceso,string_itoa(archivo->pid));

	if(tablaArchivosProceso==NULL || archivo->fileDescriptor<3){

			resultadoMover->numero = KERNEL_ERROR;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoMover);
			log_info(logger,"El proceso %d no tiene archivos abiertos", &(archivo->pid));

			free(resultadoMover);

	} else {

		t_registroArchivosProc * registroArchivoProceso =
				list_get(tablaArchivosProceso,archivo->fileDescriptor);

		if(registroArchivoProceso==NULL){

			resultadoMover->numero = KERNEL_ERROR;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoMover);
			log_info(logger,"El proceso %d no tiene abierto el archivo solicitado", archivo->pid);

			free(resultadoMover);

		} else {

			registroArchivoProceso->cursor=archivo->tamanio;

			resultadoMover->numero = KERNEL_OK;
			socket_enviar(socketCPU,D_STRUCT_NUMERO,resultadoMover);
			log_info(logger,"El proceso %d movio el cursor dentro del archivo solicitado", archivo->pid);

			free(resultadoMover);
		}
	}
}

void escribirArchivo(int socketCPU,t_struct_archivo * archivo){

}
