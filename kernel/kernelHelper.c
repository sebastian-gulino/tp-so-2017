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
	listaCpus = list_create();
	listaProcesos = list_create();
	cantidad_pid = 1;

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

			//conexion_consola(socketCliente);

			FD_SET(socketCliente, &master_consola);

			list_add(listaConsolas, (void*)socketCliente);

			log_info(logger,"La consola %d se conectó.", socketCliente);

			break;

		case ES_CPU:

			//conexion_cpu(socketCliente);

			FD_SET(socketCliente, &master_cpu);

			list_add(listaCpus, (void*)socketCliente);

			// TODO pasar a un metodo
			t_struct_numero* tamanio_stack;
			tamanio_stack = malloc(sizeof(t_struct_numero));
			tamanio_stack->numero = configuracion.stackSize;
			socket_enviar(socketCliente, D_STRUCT_NUMERO, tamanio_stack);
			free(tamanio_stack);

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

void manejarCpu(int i){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(i,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"El Cpu %d cerró la conexión.",i);
		removerClientePorCierreDeConexion(i,listaCpus,&master_cpu);
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


			t_pcb pcb = crearPCB(programa, obtener_pid(), tamanio_programa);

			//Envio el process id
			t_struct_numero* pid_struct = malloc(sizeof(t_struct_numero));
			pid_struct->numero = pcb.PID;
			socket_enviar(socketConsola, D_STRUCT_NUMERO, pid_struct);
			free(pid_struct);

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
	es_kernel->numero = ES_CONSOLA;
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

	int pid = cantidad_pid++;

	return pid;
}

t_pcb crearPCB(char* programa, int PID, int tamanioPrograma) {

	t_metadata_program *metadata = metadata_desde_literal(programa);

	malloc(sizeof(t_pcb));

	indiceCodigo = list_create();
	indiceStack = list_create();

	pcb.cantidadPaginas = sizeof(programa) % tamanio_pagina;
	pcb.exitcode = 0;
	pcb.PID=PID;

	int programCounter = 0;

	while (programCounter!= metadata->instrucciones_size) {

		malloc(sizeof(limitesInstrucciones));
		limitesInstrucciones.inicioInstruccion = metadata->instrucciones_serializado[programCounter].start;
		limitesInstrucciones.longitudInstruccion = metadata->instrucciones_serializado[programCounter].offset;

		list_add(indiceCodigo, (void*) &limitesInstrucciones);

		programCounter++;
	}

	pcb.indiceCodigo = indiceCodigo;
	pcb.indiceEtiquetas = metadata->etiquetas;
	pcb.indiceStack = indiceStack;

	metadata_destruir(metadata);

	int segmentoCodigo = solicitarSegmentoCodigo(pcb.PID, tamanioPrograma);

	int segmentoStack = solicitarSegmentoStack(pcb.PID);

	if(segmentoStack == -1 || segmentoCodigo == -1){
		//Si no pude asignarle memoria dejo el PID en -1 para informar a la consola
		pcb.PID = -1;
	}

	return pcb;
}

t_stack crearStack(unsigned char pos, t_list * argumentos, t_list * variables, unsigned char retPos, t_posicion_memoria retVar) {
	t_stack stack;
	malloc(sizeof(t_stack));
	stack.posicion = pos;
	stack.argumentos = argumentos;
	stack.variables = variables;
	stack.retPos = retPos;
	stack.posicionVariableRetorno = retVar;

	return stack;

}

int solicitarSegmentoCodigo(int pid, int tam_programa){

	// Pido a la memoria un segmento para el código
	t_struct_malloc* seg_codigo = malloc(sizeof(t_struct_malloc));
	seg_codigo->PID = pid;
	seg_codigo->tamano_segmento = tam_programa;

	// Envío la solicitud de memoria con el tamaño del programa que quiero ejecutar
	int resultado = socket_enviar(socketMemoria, D_STRUCT_MALC, seg_codigo);

	if(resultado != 1){
		printf("No se pudo crear segmento de codigo\n");
		return -1;
	}
	free(seg_codigo);

	void * structRecibido;
	t_tipoEstructura tipoStruct;

	uint32_t dir_codigo;
	int respuesta;

	// Recibo la direccion del nuevo segmento de código
	socket_recibir(socketMemoria, &tipoStruct, &structRecibido);

	if(tipoStruct == D_STRUCT_NUMERO ){

		respuesta = ((t_struct_numero *) structRecibido)->numero;
		if (respuesta != -1){
			dir_codigo = ((t_struct_numero *) structRecibido)->numero;
			log_info(logger,"La direccion del segmento de codigo es %d",dir_codigo);
		}else{
			// TODO Imprimir en la consola que no hay espacio para el codigo
			free(structRecibido);
			return -1;
		}
	} else {
		printf("No se recibio la direccion del segmento de codigo del proceso\n");
//		free(structRecibido);
		return -1;
	}
	free(structRecibido);
	return dir_codigo;
}

int solicitarSegmentoStack(int pid){
	// Pido a la memoria un segmento para el stack
	t_struct_malloc* seg_stack = malloc(sizeof(t_struct_malloc));
	seg_stack->PID = pid;
	seg_stack->tamano_segmento = configuracion.stackSize;

	// Envío la solicitud de memoria con el tamaño de stack definido por conf
	int resultado = socket_enviar(socketMemoria, D_STRUCT_MALC, seg_stack);

	if(resultado != 1){
		log_info(logger,"No se pudo crear el segmento de stack");
		//TODO eliminar el segmento de codigo solicitado
		return -1;
	}
	free(seg_stack);

	void * structRecibido;
	t_tipoEstructura tipoStruct;

	uint32_t dir_stack;
	int respuesta;

	// Recibo la direccion del nuevo segmento de código
	socket_recibir(socketMemoria, &tipoStruct, &structRecibido);

	if(tipoStruct == D_STRUCT_NUMERO ){
		respuesta = ((t_struct_numero *) structRecibido)->numero;
		if (respuesta != -1){
			dir_stack = ((t_struct_numero *) structRecibido)->numero;
			log_info(logger,"La direccion del segmento de stack es %d",dir_stack);
		}else{
			// TODO Imprimir en la consola que no hay espacio para el stack
			// TODO Eliminar el segmento de codigo solicitado
			free(structRecibido);
			return -1;
		}
	} else {
		printf("No se recibio la direccion del segmento de codigo del proceso\n");
		free(structRecibido);
		return -1;
	}
	free(structRecibido);
	return dir_stack;
}
