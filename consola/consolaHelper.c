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

int commandHandler(int socket){



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
				data_to_send data;
				data.socket = socket;
				data.path  = path;

				pthread_create(&threadProgramHandler, NULL, programHandler, &data);
				pthread_join(threadProgramHandler, NULL);


				break;

			case 2:
				printf("Finaliza programa\n");

				break;

			case 3:

				confirmation_send.numero = 243;

				socket_enviar(socket, D_STRUCT_NUMERO, &confirmation_send);

				socket_recibir(socket,&tipoEstructura,&structRecibido);

				if(((t_struct_numero *)structRecibido)->numero == 1){

					printf("Consola desconectada...\n");
					pthread_join(socket, NULL);
					pthread_exit(&socket);
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

void programHandler(void* arg){

	data_to_send * data = arg;
	t_struct_string path;
	path.string=data->path;


	socket_enviar(data->socket, D_STRUCT_STRING, &path);
	socket_recibir(data->socket,&tipoEstructura,&structRecibido);
	printf("El PID del programa es : %d\n", ((t_struct_numero *)structRecibido)->numero);




}


