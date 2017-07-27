#include "fsHelper.h"

cargarConfiguracion() {

	t_config * config;

	pathConfiguracion = "./config.txt";
	config = config_create(pathConfiguracion);

	if(config == NULL){

		pathConfiguracion = "../config.txt";
		config = config_create(pathConfiguracion);
	}

	configuracion = malloc(sizeof(t_configuracion));

	configuracion->puertoFS = config_get_int_value(config, "PUERTO");
	log_info(logger,"PUERTO_FS = %d",configuracion->puertoFS);

	configuracion->puntoMontaje = strdup(config_get_string_value(config, "PUNTO_MONTAJE"));
	log_info(logger,"PUNTO_MONTAJE = %s",configuracion->puntoMontaje);

	config_destroy(config);

}

void setPuntoDeMontaje(){
	char pm_command[260];
	char mtdt_command[260];
	char arch_command[260];
	char bloque_command[260];

	sprintf(pm_command, "mkdir %s", configuracion->puntoMontaje);
	sprintf(mtdt_command, "mkdir %s/Metadata", configuracion->puntoMontaje);
	sprintf(arch_command, "mkdir %s/Archivos", configuracion->puntoMontaje);
	sprintf(bloque_command, "mkdir %s/Bloques", configuracion->puntoMontaje);

	system(pm_command);
	system(mtdt_command);
	system(arch_command);
	system(bloque_command);

	log_info(logger, "Las carpetas: Metadata, Archivos y Bloques creadas satisfactoriamente en el punto de montaje: %s", configuracion->puntoMontaje);
}

void setMetadata(){

	t_config * mtdt;
	char pathMetadata[260];
	char bloqueSize[300];
	int size = 20;
	char bloqueCant[300];
	int cant = 8;

		sprintf(pathMetadata, "%s/Metadata/Metadata.bin", configuracion->puntoMontaje );

		if (fopen(pathMetadata, "r") == NULL){

		fopen(pathMetadata, "w+");

		mtdt = config_create(pathMetadata);

		sprintf(bloqueSize, "%d", size);
		sprintf(bloqueCant, "%d", cant);
		config_set_value(mtdt, "TAMANIO_BLOQUES", bloqueSize);
		config_set_value(mtdt, "CANTIDAD_BLOQUES", bloqueCant);
		config_set_value(mtdt, "MAGIC_NUMBER", "SADICA");

		config_save(mtdt);
		log_info(logger, "El metadata se creo satisfactoriamente");

		metadata.bloque_cant = config_get_int_value(mtdt, "CANTIDAD_BLOQUES");
		metadata.bloque_size = config_get_int_value(mtdt, "TAMANIO_BLOQUES");

		log_info(logger, "El metadata se cargo satisfactoriamente");



	}
		mtdt = config_create(pathMetadata);
		metadata.bloque_cant = config_get_int_value(mtdt, "CANTIDAD_BLOQUES");
		metadata.bloque_size = config_get_int_value(mtdt, "TAMANIO_BLOQUES");

		config_destroy(mtdt);
}
void crearServidorMonocliente(){

	int socketServidor = crearServidor(configuracion->puertoFS);

	while(1){
			//Por defecto acepto el cliente que se está conectando
			socketCliente = aceptarCliente(socketServidor);

			void* structRecibido;

			t_tipoEstructura tipoStruct;

			if(socket_recibir(socketCliente, &tipoStruct, &structRecibido) == -1 || tipoStruct != D_STRUCT_NUMERO){
				log_info(logger,"No se recibio correctamente a quien atendio el Filesystem");

			} else if ((((t_struct_numero*) structRecibido)->numero) == ES_KERNEL){

					log_info(logger,"Se conecto el Kernel");

					pthread_create(&threadAtenderKernel, NULL, manejarKernel, socketCliente);

			} else {

				log_error(logger,"No se pudo hacer el handshake");

				//Ciero el FD del cliente que había aceptado
				close(socketCliente);
			}
	}

}

void crearBitmap(){

			char pathBitmap[260];
		char bitmap_command[260];
		sprintf(pathBitmap, "%s/Metadata/Bitmap.bin", configuracion->puntoMontaje );



	if (fopen(pathBitmap, "r") == NULL){

		sprintf(bitmap_command, "dd bs=%d seek=1 of=%s count=0", metadata.bloque_cant/8, pathBitmap);
		system(bitmap_command);
	}

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



int asignarBloque(t_config * data){


	if(bloquesLibres()>0){

	off_t posicion = 0;

	char pathBloque[260];

	while(bitarray_test_bit(bitarray, posicion)){

		posicion++;

	}

	int offset = posicion;
	bitarray_set_bit(bitarray,posicion);

	sprintf(pathBloque, "%s/Bloques/%d.bin", configuracion->puntoMontaje, offset);

	FILE * file = fopen(pathBloque, "w+");

	if(msync(bmap, mystat.st_size, MS_SYNC) < 0){
		printf("Error es: %s\n", strerror(errno));
	}
	char bloque[100];

	sprintf(bloque, "[%d]", offset);

	config_set_value(data, "BLOQUES", bloque);
	config_save(data);

	fclose(file);

	return 1;

}
	else return 0;

}

int bloquesLibres(){

int contador=0, var, posicion=0;
size_t max = bitarray_get_max_bit(bitarray);

	for (var = 0; var < max; ++var) {

		if(bitarray_test_bit(bitarray, posicion)){
			contador++;
		}
		posicion++;

	}

	if(contador==max){
		return 0;
	}
	return max-contador;
}




void manejarKernel(int i){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;


	while(socket_recibir(i,&tipoEstructura,&structRecibido) > 0){

			switch(tipoEstructura){

			case D_STRUCT_ARCHIVO_ABR: ;

				t_struct_archivo * archivoAb = ((t_struct_archivo*) structRecibido);

				t_struct_abrir * archivoAbrir = malloc(sizeof(t_struct_abrir));
				archivoAbrir->path=archivoAb->informacion;
				archivoAbrir->modo_creacion = archivoAb->flags.creacion ? 1 : 0;

				validarArchivo(archivoAbrir);

			break;

			case D_STRUCT_ARCHIVO_BOR: ;

				socket_recibir(socketCliente,&tipoEstructura,&structRecibido);
				t_struct_string * pathBorrar = ((t_struct_string*) structRecibido);

				t_struct_borrar * archivoBorrar = malloc(sizeof(t_struct_borrar));
				archivoBorrar->path=pathBorrar->string;

				borrarArchivo(archivoBorrar);

			break;

			case D_STRUCT_ARCHIVO_ESC: ;

				t_struct_archivo * archivoEsc = ((t_struct_archivo*) structRecibido);

				socket_recibir(socketCliente,&tipoEstructura,&structRecibido);
				t_struct_numero * desplazamiento = ((t_struct_numero*) structRecibido);

				socket_recibir(socketCliente,&tipoEstructura,&structRecibido);
				t_struct_string * path = ((t_struct_string*) structRecibido);

				t_struct_guardar * archivoGuardar = malloc(sizeof(t_struct_guardar));
				archivoGuardar->path = path->string;
				archivoGuardar->modo_escritura=archivoEsc->flags.escritura ? 1 : 0;
				archivoGuardar->offset=desplazamiento->numero;
				archivoGuardar->size = archivoEsc->tamanio;
				archivoGuardar->buffer = archivoEsc->informacion;

				guardarDatos(archivoGuardar);

			break;

			case D_STRUCT_ARCHIVO_LEC: ;

				t_struct_archivo * archivoLec = ((t_struct_archivo*) structRecibido);

				socket_recibir(socketCliente,&tipoEstructura,&structRecibido);
				t_struct_numero * offset = ((t_struct_numero*) structRecibido);

				socket_recibir(socketCliente,&tipoEstructura,&structRecibido);
				t_struct_string * pathArch = ((t_struct_string*) structRecibido);

				t_struct_obtener * archivoLeer = malloc(sizeof(t_struct_obtener));

				archivoLeer->path=pathArch->string;
				archivoLeer->modo_lectura=archivoLec->flags.lectura ? 1 : 0;
				archivoLeer->size=archivoLec->tamanio;
				archivoLeer->offset=offset->numero;

				obtenerDatos(archivoLeer);

			break;
			}


	}


	if (socket_recibir(i,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"El Kernel %d cerró la conexión.",i);
	}
}
