#include "fsHelper.h"

t_configuracion cargarConfiguracion() {

	t_config * config;
	t_configuracion configuracion;
	t_config * mtdt;
	t_metadata metadataFS;

	config = config_create("./config.txt");

	if(config == NULL){

		config = config_create("../config.txt");

	}

	configuracion.puertoFS = config_get_int_value(config, "PUERTO");
	log_info(logger,"PUERTO_FS = %d",configuracion.puertoFS);

	configuracion.puntoMontaje = strdup(config_get_string_value(config, "PUNTO_MONTAJE"));
	log_info(logger,"PUNTO_MONTAJE = %s",configuracion.puntoMontaje);

	char pathMetadata[260];
	sprintf(pathMetadata, "%s/Metadata/Metadata.bin", configuracion.puntoMontaje );

	mtdt = config_create(pathMetadata);

	metadataFS.bloque_cant = config_get_int_value(mtdt, "TAMANIO_BLOQUES");
	metadataFS.bloque_size = config_get_int_value(mtdt, "CANTIDAD_BLOQUES");
	return configuracion;

}

void crearServidorMonocliente(){

	int socketServidor = crearServidor(configuracion.puertoFS);

	while(1){
			//Por defecto acepto el cliente que se está conectando
			int socketCliente = aceptarCliente(socketServidor);

			void* structRecibido;

		 	t_tipoEstructura tipoStruct;

			//Recibo el mensaje para identificar quien es y hacer el handshake
			int resultado = socket_recibir(socketCliente, &tipoStruct, &structRecibido);

			if(resultado == -1 || tipoStruct != D_STRUCT_NUMERO){
				log_info(logger,"No se recibio correctamente a quien atendio el Filesystem");

			} else if ((((t_struct_numero*) structRecibido)->numero) == ES_CONSOLA){

					log_info(logger,"Se conecto el Kernel");

					pthread_create(&threadAtenderKernel, NULL, manejarKernel, socketCliente);

			} else {

				log_error(logger,"No se pudo hacer el handshake");

				//Ciero el FD del cliente que había aceptado
				close(socketCliente);
			}

			free(structRecibido);
	}

}

void crearBitmap(){

		char pathBitmap[260];
		sprintf(pathBitmap, "%s/Metadata/Bitmap.bin", configuracion.puntoMontaje );

	int bitmap = open(pathBitmap, O_RDWR);


	if (fstat(bitmap, &mystat) < 0) {
	    printf("Error al establecer fstat\n");
	    close(bitmap);
	}

	bmap = mmap(NULL, mystat.st_size, PROT_WRITE | PROT_READ, MAP_SHARED, bitmap, 0);

	if (bmap == MAP_FAILED) {
			printf("Error al mapear a memoria: %s\n", strerror(errno));

	}

	bitarray = bitarray_create_with_mode(bmap, metadata.bloque_cant/8, MSB_FIRST);

}

int validarArchivo(char * path){

	FILE * file = fopen(path, "r");

	if (file == NULL){

		return 0; //El archivo no existe
	}

	return 1; //El archivo existe

}

void crearArchivo(){

	char * testPath;
	char * inexPaths[20];
	int i = 0, j;

	testPath = "/home/utnso/Escritorio/mount/Archivos/passwords/alumnos/Alumnos.bin";

	char * path = strdup(testPath);

	char *buffer = dirname(path);

	while(opendir(buffer)==NULL){

		strcpy(inexPaths[i], buffer);

		i++;

		buffer = dirname(buffer);
	}

	for (j = i; j > -1; --j) {

		mkdir(inexPaths[j], S_IRWXU | S_IRWXG | S_IRWXO);

	}

	fopen(testPath, "ab+");

	asignarBloque();

}

void asignarBloque(){

	off_t test = 1;

	bitarray_set_bit(bitarray,test);

	if(bitarray_test_bit(bitarray, test)){
		puts("ok");
	}

	if(msync(bmap, mystat.st_size, MS_SYNC) < 0){
		printf("Error es: %s\n", strerror(errno));
	}

}

void manejarKernel(int i){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(i,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"El Kernel %d cerró la conexión.",i);
	}

};
