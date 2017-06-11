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

		char * path = malloc(200);

		switch(commandParser(value)){

			case 1:

				puts("Ingrese el path del script ANSiSOP...");
				scanf("%s", path);

				pthread_create(&threadProgramHandler, NULL, iniciarPrograma, path);

				pthread_join(threadProgramHandler, NULL);

				break;

			case 2:
				printf("Finaliza programa\n");

				break;

			case 3:

				confirmation_send.numero = 243;

				socket_enviar(socketKernel, D_STRUCT_NUMERO, &confirmation_send);

				socket_recibir(socketKernel,&tipoEstructura,&structRecibido);

				if(((t_struct_numero *)structRecibido)->numero == 1){

					printf("Consola desconectada...\n");
					pthread_join(socketKernel, NULL);
					pthread_exit(&socketKernel);
				}

				break;

			case 4:
				printf("Limpia consola\n");

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

		log_info(logger, "No se pudo enviar el programa al Kernel");
		//TODO matar el hilo

	}

	t_tipoEstructura tipoEstructura;
	void * structRecibido;
	int pid;

	if (socket_recibir(socketKernel,&tipoEstructura,&structRecibido) != -1){

		pid = ((t_struct_numero *)structRecibido)->numero;
		log_info(logger,"El PID asignado es %d",pid);
	};

	free(structRecibido);

	// Cierro el archivo utilizado
	fclose(archivo);

	// Libero la memoria solicitada con malloc
	free(programa->buffer);
	free(codigo);
	free(programa);

	recibirMensajesPrograma(pid);



}

void recibirMensajesPrograma(int pid){

	while(1){
			//Estandarizar el envío de mensajes de impresión
			socket_recibir(socketKernel, &tipoEstructura, &structRecibido);

			printf("Programa %d: %s", pid, ((t_struct_string *)structRecibido)->string);

		}

}


