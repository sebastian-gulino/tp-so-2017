#include "consolaHelper.h"


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
	log_info(logger,"PUERTO_KERNEL = %d \n",configuracion.puertoKernel);

	return configuracion;
}

int conectarAKernel (){

	//Genera el socket cliente y lo conecta al kernel
	int socketCliente = crearCliente(configuracion.ipKernel,configuracion.puertoKernel);

	//Se realiza el handshake con el kernel
	t_struct_numero* es_consola = malloc(sizeof(t_struct_numero));
	es_consola->numero = ES_CONSOLA;
	socket_enviar(socketCliente, D_STRUCT_NUMERO, es_consola);
	free(es_consola);

	return socketCliente;

}

int commandHandler(){

	printf("Comandos disponibles:\n");
	printf("iniciar: Dado un path valido da inicio a un programa \n");
	printf("finalizar: Finaliza el programa especificado \n");
	printf("desconectar: Desconecta la consola\n");
	printf("limpiar: Limpia los mensajes en la consola\n");
	printf("\n");


	while(1){

		char * value = malloc(50);

		printf("Ingrese algún comando...\n");

		scanf("%s", value);

		switch(commandParser(value)){

			case 1:;

				puts("Ingrese el path del script ANSiSOP...");

				char * path = malloc(200);
				scanf("%s", path);

				pthread_t hiloPrograma;

				pthread_create(&hiloPrograma, NULL, iniciarPrograma, path);

				pthread_join(hiloPrograma, NULL);

				free(path);

				break;

			case 2:

				puts("Ingrese el Process ID que desea finalizar");

				int pidFinalizar;

				scanf("%i",&pidFinalizar);

				finalizarPrograma(pidFinalizar);

				break;

			case 3:

				//TODO implementar desconectar consola

				break;

			case 4:

				system("clear");
				log_info(logger,"Se envió la instrucción para limpiar los mensajes");

				break;

			default:
				printf("Comando invalido...\n");
				break;
			}
		free(value);
	}
	return 0;
}

int commandParser(char* command){


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

void iniciarPrograma(char* pathArchivo){

	printf("hilo programa id: %d \n",pthread_self());

	int socketKernel = conectarAKernel();

	// Estructura que voy a utilizar para enviar el programa
	t_struct_programa* programa = malloc(sizeof(t_struct_programa));

	// Abro el script a ejecutar en la ruta especificada
	FILE * archivo = fopen(pathArchivo, "r");

	// Calculo el tamaño del archivo
	fseek(archivo, 0L, SEEK_END);
	int tamanio_archivo = ftell(archivo);

	// Leo el contenido del archivo
	void * codigo = malloc(tamanio_archivo);
	fseek(archivo, SEEK_SET, 0);
	fread(codigo, 1, tamanio_archivo, archivo);

	// Preparo la estructura programa que voy a enviar
	programa->tamanio = tamanio_archivo;
	programa->buffer = malloc(tamanio_archivo);
	programa->base = 1;
	programa->PID = 1 ;
	memcpy(programa->buffer,codigo,tamanio_archivo);

	int resultado = socket_enviar(socketKernel, D_STRUCT_PROG, &programa);

	if(resultado == -1) {

		log_info(logger, "No se pudo enviar el programa al Kernel... Cierro el hilo");
		close(socketKernel);
		pthread_cancel(pthread_self());

	}

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(socketKernel,&tipoEstructura,&structRecibido) == -1){

		log_info(logger, "No se pudo recibir el PID... Cierro el hilo");
		close(socketKernel);
		pthread_cancel(pthread_self());

	} else if (((t_struct_numero *)structRecibido)->numero == -1) {
		log_info(logger, "El kernel no pudo crear el proceso... Cierro el hilo");
		close(socketKernel);
		pthread_cancel(pthread_self());
	};

	t_proceso* proceso;

	proceso->pid = ((t_struct_numero *)structRecibido)->numero;
	proceso->inicioEjec = time(&rawtime);
	proceso->cantImpresiones = 0;
	proceso->socketKernel = socketKernel;
	proceso->hilo = pthread_self();

	free(structRecibido);

	// Cierro el archivo utilizado
	fclose(archivo);

	// Libero la memoria solicitada con malloc
	free(programa->buffer);
	free(codigo);
	free(programa);

	list_add(listaProcesos, proceso);

	recibirMensajes(proceso);

}

void recibirMensajes(t_proceso* proceso){

	int programaEjecutando = 1;

	while(programaEjecutando){

		t_tipoEstructura tipoEstructura;
		void * structRecibido;

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
				log_info(logger,"El proceso con PID: %d ejecutando en el hilo: %d informa %s\n",
						proceso->pid, proceso->hilo, texto);

				free(structRecibido2);
				free(structRecibido);

				break;
			case D_STRUCT_FIN_PROG:
				//El programa finalizo correctamente
				printf("El proceso con PID:%d finalizó correctamente \n",proceso->pid);
				free(structRecibido);

				proceso->finEjec =time(&rawtime);
				double tiempoEjecucion = difftime(proceso->finEjec, proceso->inicioEjec);

				terminarProceso(proceso);

				programaEjecutando = 0;
			}
		}
	}
}

void inicializarListas(){
	listaProcesos = list_create();
	cantidadThreads = 0;
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

			log_info(logger, "No se pudo finalizar el programa");

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

	printf("Inicio de ejecución: %s", asctime(localtime(&proceso->inicioEjec)));
	printf("Fin de ejecución: %s", asctime(localtime(&proceso->finEjec)));
	printf("Cantidad de impresiones: %d",proceso->cantImpresiones);
	printf("Tiempo total de ejecución: %d segundos",tiempoEjecucion);

	// Cierro el socket correspondiente al hilo programa y mato el thread
	close(proceso->socketKernel);
	pthread_cancel(proceso->hilo);
}
