#include "kernelHelper.h"

int pidk = 0;

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

	configuracion.stackSize = strdup(config_get_string_value(config, "STACK_SIZE"));
	log_info(logger,"STACK_SIZE = %s",configuracion.stackSize);

	configuracion.puertoEscucha = config_get_int_value(config,"PUERTO_ESCUCHA");
	log_info(logger,"PUERTO_ESCUCHA = %d \n",configuracion.puertoEscucha);

	return configuracion;
}

void inicializarListas(){
	listaConsolas = list_create();
	listaCpus = list_create();
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

void manejarConsola(int i){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;


	if (socket_recibir(i,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"La Consola %d cerró la conexión.",i);
		removerClientePorCierreDeConexion(i,listaConsolas,&master_consola);
	} else {

		if(((t_struct_numero *)structRecibido)->numero == 243){
			t_struct_numero confirmation_send;

			if (programKiller(i) == 0){

				confirmation_send.numero = 0;

				socket_enviar(i, D_STRUCT_NUMERO, &confirmation_send);

			} else {

				confirmation_send.numero = 1;
				socket_enviar(i, D_STRUCT_NUMERO, &confirmation_send);
			}

		}

		t_struct_numero pid_send;
		pid_send.numero = pidk;
		char * programa = malloc(sizeof(((t_struct_string *)structRecibido)->string));
		programa = ((t_struct_string *)structRecibido)->string;

		log_info(logger,"La Consola %d envió el path: %s", i, programa);

		t_metadata_program *program_data = metadata_desde_literal(programa);

		printf("Cant funciones %d\n", program_data->cantidad_de_funciones);
		printf("Cant eti %d\n", program_data->cantidad_de_etiquetas);


		socket_enviar(i, D_STRUCT_NUMERO, &pid_send);

		pidk++;

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

	//Se realiza el handshake con la memoria
	t_struct_numero* es_kernel = malloc(sizeof(t_struct_numero));
	es_kernel->numero = ES_CONSOLA;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_kernel);
	free(es_kernel);

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

