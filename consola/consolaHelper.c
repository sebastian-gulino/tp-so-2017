#include "consolaHelper.h"


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
	log_info(logger,"PUERTO_KERNEL = %d \n",configuracion->puertoKernel);

	config_destroy(config);
}

int conectarAKernel (){

	//Genera el socket cliente y lo conecta al kernel
	int socketCliente = crearCliente(configuracion->ipKernel,configuracion->puertoKernel);

	//Se realiza el handshake con el kernel
	t_struct_numero* es_consola = malloc(sizeof(t_struct_numero));
	es_consola->numero = ES_CONSOLA;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_consola);
	free(es_consola);

	return socketCliente;

}

void commandHandler(){

	printf("Comandos disponibles:\n");
	printf("iniciar: Dado un path valido da inicio a un programa \n");
	printf("finalizar: Finaliza el programa especificado \n");
	printf("desconectar: Desconecta la consola\n");
	printf("limpiar: Limpia los mensajes en la consola\n");
	printf("\n");

	while(consolaConectada){

		puts("Ingrese algún comando no mayor a 50 caracteres");
		char * value = malloc(50);
		scanf("%s", value);

		switch(commandParser(value)){
			case 1:;
				puts("Ingrese el path del script ANSiSOP...");
				char * path = malloc(200);
				scanf("%s", path);

				t_proceso * proceso = malloc(sizeof(t_proceso));

				proceso->path = string_new();
				string_append(&(proceso->path), path);

				pthread_create(&proceso->hiloPrograma, NULL, iniciarPrograma, proceso);

				free(path);

				break;
			case 2:
				puts("Ingrese el Process ID que desea finalizar");
				int pidFinalizar;
				scanf("%i",&pidFinalizar);

				finalizarPrograma(pidFinalizar);

				break;
			case 3:
				pthread_mutex_lock(&mutex_log);
				log_info(logger,"Se envió la instrucción para desconectar la consola");
				pthread_mutex_unlock(&mutex_log);

				manejarDesconexion();

				consolaConectada=0;
				break;
			case 4:

				system("clear");
				pthread_mutex_lock(&mutex_log);
				log_info(logger,"Se envió la instrucción para limpiar los mensajes");
				pthread_mutex_unlock(&mutex_log);

				break;
			default:
				printf("Comando invalido...\n");
				break;
			}
		free(value);
	}
}

int commandParser(char* command){

	string_to_upper(command);

	if(strcmp(command, "INICIAR") == 0){
		return 1;
	} else if (strcmp(command, "FINALIZAR") == 0){
		return 2;
	} else if(strcmp(command, "DESCONECTAR") == 0){
		return 3;
	} else if (strcmp(command, "LIMPIAR") == 0){
		return 4;
	}else{
		return 6;
	}

}

void manejarSignal(int sign){
	switch(sign){
	case SIGUSR1:
		pthread_exit(0);
		break;
	case SIGINT:
		manejarDesconexion();
		break;
	}
}

void iniciarPrograma(void* procesoCreado){

	int socketKernel = conectarAKernel();

	t_proceso * proceso = procesoCreado;

	// Abro el script a ejecutar en la ruta especificada
	FILE * archivo = fopen(proceso->path, "rb");

	// Calculo el tamaño del archivo
	fseek(archivo, 0L, SEEK_END);
	int tamanio_archivo = ftell(archivo);

	// Leo el contenido del archivo
	char * codigo = malloc(tamanio_archivo+1);
	fseek(archivo, SEEK_SET, 0);
	fread(codigo, 1, tamanio_archivo, archivo);

	codigo[tamanio_archivo]='\0';

	// Estructura que voy a utilizar para enviar el programa
	t_struct_programa* programa = malloc(sizeof(t_struct_programa));

	// Preparo la estructura programa que voy a enviar
	programa->tamanio = tamanio_archivo+1;
	programa->buffer = malloc(tamanio_archivo+1);
	programa->base = 1;
	programa->PID = 1 ;
	memcpy(programa->buffer,codigo,tamanio_archivo+1);

	proceso->tamanio=tamanio_archivo+1;

	int resultado = socket_enviar(socketKernel, D_STRUCT_PROG, programa);

	if(resultado == -1) {

		pthread_mutex_lock(&mutex_log);
		log_info(logger, "No se pudo enviar el programa al Kernel... Cierro el hilo");
		pthread_mutex_unlock(&mutex_log);

		close(socketKernel);
		pthread_cancel(pthread_self());

	}

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(socketKernel,&tipoEstructura,&structRecibido) == -1){

		pthread_mutex_lock(&mutex_log);
		log_info(logger, "No se pudo recibir el PID... Cierro el hilo");
		pthread_mutex_unlock(&mutex_log);

		close(socketKernel);
		pthread_cancel(pthread_self());

	} else if (((t_struct_numero *)structRecibido)->numero == -1) {

		pthread_mutex_lock(&mutex_log);
		log_info(logger, "El kernel no pudo crear el proceso... Cierro el hilo");
		pthread_mutex_unlock(&mutex_log);

		close(socketKernel);
		pthread_cancel(pthread_self());
	} else if (((t_struct_numero *)structRecibido)->numero == KERNEL_MULTIPROG){
		pthread_mutex_lock(&mutex_log);
		log_info(logger, "El proceso queda en la cola de new por el grado de multiprogramacion, espero la solicitud");
		pthread_mutex_unlock(&mutex_log);

		socket_recibir(socketKernel,&tipoEstructura,&structRecibido);

		if(tipoEstructura==D_STRUCT_SOLICITAR_CODIGO) socket_enviar(socketKernel, D_STRUCT_PROG, &programa);

		socket_recibir(socketKernel,&tipoEstructura,&structRecibido);
	}

	free(programa->buffer);
	free(programa);

	proceso->pid = ((t_struct_numero *)structRecibido)->numero;
	proceso->inicioEjec = time(&rawtime);
	proceso->cantImpresiones = 0;
	proceso->socketKernel = socketKernel;

	pthread_mutex_lock(&mutex_log);
	log_info(logger, "Inicia el proceso PID %d",proceso->pid);
	pthread_mutex_unlock(&mutex_log);

	free(structRecibido);

	list_add(listaProcesos, proceso);

	recibirMensajes(proceso);

}

void recibirMensajes(t_proceso* proceso){

	bool programaEjecutando = true;

	while(programaEjecutando){

		t_tipoEstructura tipoEstructura;
		void * structRecibido;

		signal(SIGUSR1, manejarSignal);

		if (socket_recibir(proceso->socketKernel, &tipoEstructura, &structRecibido) != -1) {
			switch(tipoEstructura){
			case D_STRUCT_IMPR:
				;
				//Recibi un mensaje que debo imprimir por pantalla
				t_tipoEstructura tipoEstructura2;
				void * structRecibido2;

				socket_recibir(proceso->socketKernel, &tipoEstructura2, &structRecibido2);

				int length = ((t_struct_numero*) structRecibido2)->numero;
				char* texto = ((t_struct_string*)structRecibido)->string;

				texto[length] = '\0';

				//Aumento el contador de impresiones
				proceso->cantImpresiones++;

				printf("El proceso con PID:%d informa %s\n",proceso->pid, texto);

				pthread_mutex_lock(&mutex_log);
				log_info(logger,"El proceso con PID: %d informa %s\n",
						proceso->pid, texto);
				pthread_mutex_unlock(&mutex_log);

				free(structRecibido2);
				free(structRecibido);

				break;
			case D_STRUCT_FIN_PCB: ;

				t_struct_pcb * pcbFinOk = ((t_struct_pcb*) structRecibido);

				if(pcbFinOk->exitcode==EC_FINALIZO_OK) printf("El proceso con PID:%d finalizó OK \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_ARCHIVO_ES_PERMISOS) printf("El proceso con PID:%d finalizó por intentar escribir archivo sin permisos \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_ARCHIVO_INEX) printf("El proceso con PID:%d finalizó por intentar acceder a un archivo inexistente \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_ARCHIVO_LE_PERMISOS) printf("El proceso con PID:%d finalizó por intentar leer archivo sin permisos \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_DESCONEXION_CONSOLA) printf("El proceso con PID:%d finalizó por desconectarse la consola \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_DESCONEXION_CPU) printf("El proceso con PID:%d finalizó por desconectarse la CPU \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_DESCONEXION_KERNEL) printf("El proceso con PID:%d finalizó por desconectarse el kernel \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_EXCEP_MEMORIA) printf("El proceso con PID:%d finalizó por una excepcion de memoria \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_FINALIZADO_CONSOLA) printf("El proceso con PID:%d fue finalizado por consola \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_MAXIMO_PAGINAS) printf("El proceso con PID:%d finalizó por solicitar mas paginas del valor maximo \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_RESERVA_MAYOR_PAGINA) printf("El proceso con PID:%d finalizó por intentar reservar heap mayor al tamano de pagina \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_STACK_OVERFLOW) printf("El proceso con PID:%d finalizó por STACK OVERFLOW \n",proceso->pid);
				if(pcbFinOk->exitcode==EC_SIN_DEFINICION) printf("El proceso con PID:%d finalizó por un error sin definicion \n",proceso->pid);

				free(structRecibido);

				terminarProceso(proceso);

				programaEjecutando = false;
			}
		}
	}
}

void inicializarEstructuras(){
	listaProcesos = list_create();
	cantidadThreads = 0;
	consolaConectada = 1;
	pthread_mutex_init(&mutex_log, NULL);
}

void finalizarPrograma(int pid){

	bool es_el_pid_consola(t_proceso* proceso){
		return (proceso->pid == pid);
	}

	t_proceso* procesoFinalizar = list_find(listaProcesos, (void *)es_el_pid_consola);

	//Envio el PID del programa que deseo eliminar
	t_struct_numero* pidFinalizar = malloc(sizeof(t_struct_numero));
	pidFinalizar->numero = procesoFinalizar->pid;
	socket_enviar(procesoFinalizar->socketKernel, D_STRUCT_FIN_PROG, pidFinalizar);
	free(pidFinalizar);

	int programaFinalizo = 0;

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	while (programaFinalizo==0){

		if (socket_recibir(procesoFinalizar->socketKernel,&tipoEstructura,&structRecibido) == -1){

			pthread_mutex_lock(&mutex_log);
			log_info(logger, "No se pudo finalizar el programa");
			pthread_mutex_unlock(&mutex_log);

		} else {
			programaFinalizo = 1;
			terminarProceso(procesoFinalizar);
		}

	}

}

void terminarProceso(t_proceso* proceso){

	bool es_el_pid_consola(t_proceso* proc){
		return (proc->pid == proceso->pid);
	}
	// Elimino el proceso de la lista de procesos en ejecución
	list_remove_by_condition(listaProcesos,(void*)es_el_pid_consola);

	proceso->finEjec =time(&rawtime);

	double tiempoEjecucion = difftime(proceso->finEjec, proceso->inicioEjec);
	printf("=========FIN DE EJECUCION DE PROCESO=========");
	printf("Inicio de ejecución: %s", asctime(localtime(&proceso->inicioEjec)));
	printf("Fin de ejecución: %s", asctime(localtime(&proceso->finEjec)));
	printf("Cantidad de impresiones: %d \n",proceso->cantImpresiones);
	printf("Tiempo total de ejecución: %f segundos \n",tiempoEjecucion);
	printf("=========Datos de ejecucion=========");

	// Cierro el socket correspondiente al hilo programa y mato el thread
	close(proceso->socketKernel);
	pthread_cancel(proceso->hiloPrograma);
}

void manejarDesconexion(){
	log_destroy(logger);

	list_iterate(listaProcesos, (void*) terminarProceso);

	list_destroy(listaProcesos);

	exit(EXIT_SUCCESS);
}
