#include "fsProcesos.h"

//Validación de archivos con pedido de apertura.

void  validarArchivo(t_struct_abrir * archivo){

	char pathFile[260];
	t_struct_abrir * toSend = malloc(sizeof(t_struct_abrir));
	toSend = archivo;
	sprintf(pathFile, "%s/Archivos%s", configuracion.puntoMontaje, archivo->path); //Se usa sprintf para obtener el path
																					//completo en el punto de montaje especificado

	FILE * file = fopen(pathFile, "r"); //Se abre el archivo, de esta manera verificamos que exista.

	if (file == NULL){ //Si no existe:

		if (archivo->modo_creacion==1){ //Verificación del modo creación

			if(crearArchivo(archivo->path) == 1){ //Se crea el archivo en el path especificado

				//fclose(file);
				toSend->confirmacion = FS_ABRIR_CREAR_OK;
				log_info(logger, "Se creo el archivo en: %s", pathFile);
				socket_enviar(socketCliente, D_STRUCT_ABRIR, toSend);
				return;

			}

			toSend->confirmacion = FS_ABRIR_CREAR_ERROR;
			log_error(logger, "No se pudo crear el archivo en: %s", pathFile);
			socket_enviar(socketCliente, D_STRUCT_ABRIR, toSend);
			return;

		}

		fclose(file);
		log_error(logger, "El archivo en el path: %s no existe", pathFile);
		toSend->confirmacion = FS_ABRIR_NO_CREAR_ERROR;
		socket_enviar(socketCliente, D_STRUCT_ABRIR, toSend);
		return;

	} else{

		fclose(file);
		log_error(logger, "Se verifico el archivo del path: %s", pathFile);
		toSend->confirmacion = FS_ABRIR_NO_CREAR_OK;
		socket_enviar(socketCliente, D_STRUCT_ABRIR, toSend);
		return;
	}
}

//Creación de un archivo, en caso que al abrir uno este no exista y se haya abierto en modo creación

int crearArchivo(char * path){

	char pathFile[260];

	sprintf(pathFile, "%s/Archivos/%s", configuracion.puntoMontaje, path);//Se usa sprintf para obtener el path
																			//completo en el punto de montaje especificado

	char inexPaths[20][260];

	int i = 0, j;

	char * create_path = strdup(pathFile); //Se obtiene la primer carpeta donde se encuentra el archivo

	char *buffer = dirname(create_path); //Se verifica que exista

	while(opendir(buffer)==NULL){ //Si no existe, se van guardando todas las carpetas necesarias crear en el array inexPaths

		strcpy(inexPaths[i], buffer);

		i++;

		buffer = dirname(buffer);
	}

	for (j = i; j > -1; --j) {

		mkdir(inexPaths[j], S_IRWXU | S_IRWXG | S_IRWXO); //Se crean las carpetas con permisos totales para todos los usuarios

	}

	FILE * file = fopen(pathFile, "w+"); //Se crea el archivo

	t_config * data = config_create(pathFile); //Se crea un config en el archivo para manejar sus datos

	config_set_value(data, "TAMANIO", "0"); //Se agrega el tamaño, al ser un archivo nuevo, su tamaño es 0

	config_save(data); //Se guarda el config

	fclose(file);

	if (asignarBloque(data)==1){ //Por default se le asigna un bloque

		return 1;
	}
	else {
		log_info(logger, "El archivo no pudo ser creado en el path: %s", pathFile);
		remove(pathFile); //Si no se le puede asignar un bloque al no haber ninguno disponible, se elimina el archivo.
		return 0;
	}


}

//Borrado de archivos y liberación de sus bloques en bitmap. Se eliminan también los .bin de los bloques para evitar posibles datos basura

void borrarArchivo(t_struct_borrar * archivo){

	char pathFile[260];
	char blockPath[260];
	t_config * fileData;
	t_struct_borrar * toSend = malloc(sizeof(t_struct_borrar));
	toSend = archivo;

		sprintf(pathFile, "%s/Archivos/%s", configuracion.puntoMontaje, archivo->path);

		if(fopen(pathFile, "r")==NULL){ //Se verifica que el archivo a borrarse exista.

			log_info(logger, "El archivo del path: %s no existe", pathFile);

			toSend->confirmacion = FS_BORRAR_ERROR;

			socket_enviar(socketCliente, D_STRUCT_BORRAR, toSend);

			return;

		}

		fileData = config_create(pathFile); //Se carga el archivo como config para manejar sus datos

		char** bloques= config_get_array_value(fileData, "BLOQUES"); //Se cargan los bloques que contienen los datos del archivo.

		int var = 0;

		while(bloques[var] != NULL) { //Se itera por todos los bloques del archivo

			char * charBloque = bloques[var];

			int bloque = atoi(charBloque); //Ya que el config me da un char* necesito pasarlo a int para proximos manejos del bloque

			sprintf(blockPath, "%s/Bloques/%zu.bin", configuracion.puntoMontaje, bloque); //Se obtiene el path absoluto del bloque

			remove(blockPath); //Se elimina el bloque

			bitarray_clean_bit(bitarray, bloque); //Se setea a 0 el bloque

			if(msync(bmap, mystat.st_size, MS_SYNC) < 0){ //Se actualiza el archivo bitmap.bin mapeado
					printf("Error es: %s\n", strerror(errno));
				}

			var++;
		}
			if (remove(pathFile)==0){; //Se elimina el archivo


			log_info(logger, "El archivo del path: %s fue eliminado satisfactoriamente", pathFile);

			toSend->confirmacion = FS_BORRAR_OK;

			socket_enviar(socketCliente, D_STRUCT_BORRAR, toSend);
			return;

			}

			log_info(logger, "El archivo del path: %s NO fue eliminado satisfactoriamente", pathFile);

			toSend->confirmacion = FS_BORRAR_ERROR;

			socket_enviar(socketCliente, D_STRUCT_BORRAR, toSend);
			return;

}

//Obtiene los datos de los bloques de un archivo según un offset y un tamaño de lectura.

void obtenerDatos(t_struct_obtener * archivo){

	char pathFile[260];
	t_config * fileData;
	int var = 0, bloq = 0, cantBloques = 0;
	int end_flag = 0;
	char *dataObtenida = string_new();
	void * bufferToSend = malloc(archivo->size);
	char blockPath[260];
	int readSize;
	FILE * file;
	int tamanioTotal = archivo->size;

	t_struct_obtener * toSend = malloc(sizeof(t_struct_obtener));

	toSend = archivo;

	sprintf(pathFile, "%s/Archivos/%s", configuracion.puntoMontaje, archivo->path);

	if(archivo->modo_lectura==0){ //Verifico que el archivo este en modo lectura

		log_error(logger, "El archivo del path: %s no tiene permisos de lectura", pathFile);

		toSend->confirmacion = FS_LEER_ERROR;
		socket_enviar(socketCliente, D_STRUCT_OBTENER, toSend);
		return;
	}

		fileData = config_create(pathFile);

		char** bloques= config_get_array_value(fileData, "BLOQUES");

		while(bloques[bloq]!=NULL){
			cantBloques++;
			bloq++;
		}

		float ratio = archivo->offset / metadata.bloque_size; //Con este calculo verifico cuantos bloques tengo que "saltear" antes de empezar a leer

		if(ratio>0){

		int offblocks = floor(ratio);

		if(cantBloques<offblocks){

			log_error(logger, "Se esta intentando leer fuera de los datos del archivo del path: %s ", pathFile);

			toSend->confirmacion = FS_LEER_ERROR;
			socket_enviar(socketCliente, D_STRUCT_OBTENER, toSend);
			return;
		}

		var = var+offblocks;

		archivo->offset=archivo->offset - metadata.bloque_size*offblocks;
		//Según la cantidad de bloques salteados, me moví una cantidad de posiciones del offset dado por lo que lo quito.

		}



		while(bloques[var] != NULL && end_flag == 0){ //Se itera por todos los bloques o hasta que se haya terminado la lectura

			void * buffer = malloc(metadata.bloque_size); //Se aloca memoria del tamaño del bloque del FS ya que es lo máximo que va a leer por bloque

			char * charBloque = bloques[var];

			int bloque = atoi(charBloque);

			sprintf(blockPath, "%s/Bloques/%d.bin", configuracion.puntoMontaje, bloque); //Path absoluto del bloque

			file = fopen(blockPath, "r"); //Abro el bloque solo para lectura

			//Calculo la cantidad de datos a leer en el bloque
			if(archivo->size <= ( metadata.bloque_size - archivo->offset)){
				readSize = archivo->size;
			} else if(archivo->size > metadata.bloque_size){
				readSize = metadata.bloque_size - archivo->offset;
			}

			int fd = fileno(file);

			//Leo del bloque y lo guardo en buffer
			if(pread(fd, buffer, readSize, archivo->offset)< 0){
				log_error(logger, "Error de lectura (%s) del archivo en el path: %s ", strerror(errno), pathFile);

				toSend->confirmacion = FS_LEER_ERROR;
				socket_enviar(socketCliente, D_STRUCT_OBTENER, toSend);
							return;

			}

			//Guardo lo leido en un buffer mas grande.
			string_append(&dataObtenida, buffer);

			//Seteo el offset en 0 ya que me movi anteriormente, por lo que el proximo bloque debo leerlo de la posicion 0
			archivo->offset = 0;

			//Reduzco el tamaño de lectura por lo leido
			archivo->size = archivo->size - readSize;

			if(archivo->size == 0){
				end_flag = 1;
			}

			fclose(file);

			var++;

		}


		memcpy(bufferToSend, dataObtenida, tamanioTotal);

		toSend->obtenido = bufferToSend;

		toSend->confirmacion = FS_LEER_OK;
		socket_enviar(socketCliente, D_STRUCT_OBTENER, toSend);
		log_info(logger, "Se leyeron %d bytes de datos del archivo en el path: %s", tamanioTotal, pathFile);
		return;

}

//Guardado de datos en los bloques de un archivo dado un buffer(los datos), offset y tamaño de escritura
void guardarDatos(t_struct_guardar * archivo){

	t_config * fileData;
	off_t posicion = 0;
	FILE * file;
	void * block_buffer;
	int var = 0, writeSize = 0, tamanioTotal = archivo->size, bloquesAsig = 0;
	char pathFile[260], pathBloque[260];
	char * bloques_array = string_new();
	char * block;
	char *char_size;
	t_struct_guardar * toSend = malloc(sizeof(t_struct_guardar));
	toSend = archivo;

	sprintf(pathFile, "%s/Archivos/%s", configuracion.puntoMontaje, archivo->path);

	if(fopen(pathFile, "r")==NULL){ //Verifico que el archivo exista
		log_info(logger, "El archivo del path: %s no existe", pathFile);
		toSend->confirmacion = FS_ESCRIBIR_ERROR;
		socket_enviar(socketCliente, D_STRUCT_GUARDAR, toSend);
		return;
	}	if(archivo->modo_escritura == 0){ //Verifico que el modo escritura este activo
		log_info(logger, "El archivo del path: %s no tiene permisos de escritura", pathFile);
		toSend->confirmacion = FS_ESCRIBIR_ERROR;
		socket_enviar(socketCliente, D_STRUCT_GUARDAR, toSend);
		return;
	}

	fileData = config_create(pathFile); //Cargo el config para manejar datos

	char** bloques= config_get_array_value(fileData, "BLOQUES"); //Obtengo los bloques que tiene el archivo
	int size = config_get_int_value(fileData, "TAMANIO"); //Obtengo el tamaño del archivo

	if(archivo->size > size){ //Si el tamaño de escritura es mayor al tamaño actual del archivo, es el tamaño final.
		size = archivo->size;
	}

	char_size = string_itoa(size);

	config_set_value(fileData, "TAMANIO", char_size); //Seteo el size actualizado del archivo
	config_save(fileData);

	//Empiezo a crear el array de bloques que van a ser asignados (comienzo con los que ya estan asignados, para luego agregar si es necesario)
	string_append(&bloques_array,"[");

	while(bloques[var]!= NULL){

		block = bloques[var];

		string_append_with_format(&bloques_array, "%s,", block);

		var++;

		bloquesAsig++;
	}

	//Calculo cuantos bloques enteros debo moverme por el offset
	float ratio = archivo->offset / metadata.bloque_size;

	var = 0;
	int new_blocks;
	posicion = atoi(bloques[var]);
		if(ratio>0){
			int counter;
			int offblocks = floor(ratio);
			new_blocks = offblocks;

			if(bloquesAsig>offblocks){
				bloquesAsig = offblocks;
			}

				for (counter = 0; counter < bloquesAsig; ++counter) {

					if(bloques[var]!=NULL){
						posicion = atoi(bloques[var]);
					}
					var++;
					new_blocks--;

				}
					if(bloques[var]==NULL){

						if(bloquesLibres()==0){

							log_error(logger, "No se pueden guardar los datos en el archivo: %s, por falta de bloques", pathFile);
							toSend->confirmacion = FS_ESCRIBIR_ERROR;
							socket_enviar(socketCliente, D_STRUCT_GUARDAR, toSend);
							return;

						}


					for (counter = 0; counter < new_blocks; ++counter) {

						posicion++;

						while(bitarray_test_bit(bitarray, posicion)){

									posicion++;
							}

						}

					posicion++;

				}
				posicion++;
				archivo->offset=archivo->offset - metadata.bloque_size*offblocks;
			}

//Escribo lo pedido en cada bloque
		while(archivo->size>0){

			block_buffer = malloc(metadata.bloque_size); //Maximo que va a ser escrito.

			int bloque = posicion;

			sprintf(pathBloque, "%s/Bloques/%d.bin", configuracion.puntoMontaje, bloque); //Path absoluto del bloque

			file = fopen(pathBloque, "r+"); //Abro el bloque en lectura y escritura

			if(file == NULL){
				file = fopen(pathBloque, "w+"); //Si el bloque fue asignado nuevo, lo creo
			}

			//Calculo el tamaño a escribir
			if(archivo->size <= ( metadata.bloque_size - archivo->offset)){
					writeSize = archivo->size;


				} else if(archivo->size > metadata.bloque_size - archivo->offset){
					writeSize = metadata.bloque_size - archivo->offset;

			}

			//Copio en el buffer del tamaño maximo del bloque lo contenido en el buffer de datos a escribir con el tamaño calculado
			memcpy(block_buffer, archivo->buffer, writeSize);

			//Me muevo en el buffer
			archivo->buffer += writeSize;

			int fd = fileno(file);

			//Escribo en el bloque
			if(pwrite(fd, block_buffer, writeSize, archivo->offset)<0){
				log_error(logger, "Error de escritura (%s) del archivo en el path: %s ", strerror(errno), pathFile);

						toSend->confirmacion = FS_ESCRIBIR_ERROR;
						socket_enviar(socketCliente, D_STRUCT_OBTENER, toSend);
					break;
			}

			//Seteo el bloque en el bitarray
			bitarray_set_bit(bitarray, posicion);

			if(msync(bmap, mystat.st_size, MS_SYNC) < 0){ //Se actualiza el archivo bitmap.bin mapeado
								printf("Error es: %s\n", strerror(errno));
							}

			archivo->offset = 0;

			archivo->size = archivo->size - writeSize;

			block = string_itoa(bloque);

			//Agrego el bloque si este fue asignado nuevo
			if(!(string_contains(bloques_array, block))){

			string_append_with_format(&bloques_array, "%s,", block);

			}
		if(bloques[var]!=NULL){
			var++;
		}

			fclose(file);

			//Verifico si tengo que asignar un bloque nuevo para seguir escribiendo o puedo seguir escribiendo los bloques ya asignados
			if(bloques[var]==NULL){

				if(bloquesLibres()==0){

					break;
				}

			while(bitarray_test_bit(bitarray, posicion)){

								posicion++;
							}
			} else
				posicion = atoi(bloques[var]);
		}

		//Cierro el array, lo seteo y guardo.

			bloques_array[strlen(bloques_array)-1] = 0;

			string_append(&bloques_array, "]");

			config_set_value(fileData, "BLOQUES", bloques_array);

			config_save(fileData);

			log_info(logger, "Se guardaron %d bytes en el archivo del path: %s. Tamaño final del archivo: %d", tamanioTotal-archivo->size, pathFile, (size-archivo->size));

		//Si lo pedido no se termino de escribir por falta de bloques.
			if(archivo->size>0){

				log_error(logger, "No se guardaron %d bytes en el archivo del path: %s.", archivo->size, pathFile);
				toSend->confirmacion = FS_ESCRIBIR_ERROR;
				socket_enviar(socketCliente, D_STRUCT_GUARDAR, toSend);
				return;
			}

			toSend->confirmacion = FS_ESCRIBIR_OK;
			socket_enviar(socketCliente, D_STRUCT_GUARDAR, toSend);

	return;

}
