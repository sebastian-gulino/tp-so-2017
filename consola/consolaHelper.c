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

	char value[20];

	printf("Comandos disponibles:\n");
	printf("iniciar: Dado un path valido da inicio a un programa \n");
	printf("finalizar: Finaliza el programa especificado \n");
	printf("desconectar: Desconecta la consola\n");
	printf("limpiar: Limpia los mensajes en la consola\n");
	printf("\n");


	while(1){

		printf("Ingrese algún comando...\n");

		scanf("%s", &value);

		switch(commandParser(value)){

			case 1:

				printf("Ingrese la ubicación del programa\n");
				scanf("%s", &path);
				pthread_create(&threadProgramHandler, NULL, programHandler(path, socket), NULL);
				pthread_join(&threadProgramHandler, NULL);

				break;

			case 2:

				printf("Finaliza programa\n");

				break;

			case 3:

				printf("Desconecta consola\n");

				break;

			case 4:
				printf("Limpia consola\n");

				break;

			default:
				printf("Comando invalido...\n");

				break;

			}


	}

	return 0;
}

int commandParser(char* command){

	if(strcmp(command, "iniciar") == 0){
		return 1;
	} else if (strcmp(command, "finalizar") == 0){
		return 2;
	} else if(strcmp(command, "desconectar") == 0){
		return 3;
	} else if (strcmp(command, "limpiar") == 0){
		return 4;
	}else{
		return 6;
	}

}

int programHandler(char * path, int socketCliente){

	t_struct_string* aPath;
	aPath->string = path;
	t_tipoEstructura tipoEstructura;
		void * structRecibido;

	socket_enviar(socketCliente, D_STRUCT_STRING, aPath);

}



//	char buf[1024];
//	char test[20];
//	FILE *file;
//	size_t nread;
//
//	while(1){
//
//		scanf("%s", &test);
//
//		file = fopen(path, "r");
//		if (file) {
//		    while ((nread = fread(buf, 1, sizeof buf, file)) > 0)
//		        fwrite(buf, 1, nread, stdout);
//		    if (ferror(file)) {
//		        printf("ERROR\n");
//		        return 0;
//		    }
//		    fclose(file);
//		}
//
//	}


