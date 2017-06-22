#include "fsHelper.h"

t_configuracion cargarConfiguracion() {

	t_config * config;
	t_configuracion configuracion;

	config = config_create("./config.txt");

	if(config == NULL){

		config = config_create("../config.txt");

	}

	configuracion.puertoFS = config_get_int_value(config, "PUERTO");
	log_info(logger,"PUERTO_FS = %d",configuracion.puertoFS);

	configuracion.puntoMontaje = strdup(config_get_string_value(config, "PUNTO_MONTAJE"));
	log_info(logger,"PUNTO_MONTAJE = %s",configuracion.puntoMontaje);

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



	FILE * bitmap = fopen("/home/utnso/Escritorio/mount/Metadata/Bitmap.bin", "rb");

	void * bmap = mmap(NULL, sizeof(bitmap), MAP_SHARED, MAP_FIXED, bitmap, sysconf(_SC_PAGE_SIZE));

	t_bitarray * bitarray = bitarray_create_with_mode(bmap, 5200/8, MSB_FIRST);



}

int validarArchivo(char * path){

	FILE * file = fopen(path, "r");

	if (file == NULL){

		return 0; //El archivo no existe
	}

	return 1; //El archivo existe

}

void crearArchivo(char * path){


}

void asignarBloque(){


}
void manejarKernel(int i){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(i,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"El Kernel %d cerró la conexión.",i);
	}

};
