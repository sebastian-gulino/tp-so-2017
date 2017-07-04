#include "fsProcesos.h"


int validarArchivo(t_abrir archivo){

	char pathFile[260];
	int fd = 3;

	sprintf(pathFile, "%s/Archivos/%s", configuracion.puntoMontaje, archivo.path);

	FILE * file = fopen(pathFile, "r");

	if (file == NULL){

		if (archivo.modo_creacion==1){

			if(crearArchivo(archivo.path) == 1){

				return 1;

			} else return 0;

		}
		return 0; //El archivo no existe
	}


	fd++;

	return fd; //El archivo existe

}

int crearArchivo(char * path){

	char pathFile[260];
		sprintf(pathFile, "%s/Archivos/%s", configuracion.puntoMontaje, path);

	char inexPaths[20][260];
	int i = 0, j;

	char * create_path = strdup(pathFile);

	char *buffer = dirname(create_path);

	while(opendir(buffer)==NULL){

		strcpy(inexPaths[i], buffer);

		i++;

		buffer = dirname(buffer);
	}

	for (j = i; j > -1; --j) {

		mkdir(inexPaths[j], S_IRWXU | S_IRWXG | S_IRWXO);

	}

	fopen(pathFile, "ab+");


	t_config * data = config_create(pathFile);

	config_set_value(data, "SIZE", "0");
	config_set_value(data, "MAGIC_NUMBER", "SADICA");

	config_save(data);

	if (asignarBloque(data)==1){
		return 1;
	}
	else {
		remove(pathFile);
		return 0;
	}


}

int borrarArchivo(char * path){

	char pathFile[260];
	char blockPath[260];
	t_config * fileData;

		sprintf(pathFile, "%s/Archivos/%s", configuracion.puntoMontaje, path);
		if(fopen(pathFile, "r")==NULL){
			return -1;
		}
		fileData = config_create(pathFile);

		char** bloques= config_get_array_value(fileData, "BLOCKS");

		int var = 0;

		while(bloques[var] != NULL) {

			char * charBloque = bloques[var];

			int bloque = atoi(charBloque);

			sprintf(blockPath, "%s/Bloques/%zu.bin", configuracion.puntoMontaje, bloque);

			remove(blockPath);

			bitarray_clean_bit(bitarray, bloque);

			if(msync(bmap, mystat.st_size, MS_SYNC) < 0){
					printf("Error es: %s\n", strerror(errno));
				}

			var++;
		}

			remove(pathFile);

			return 1;

}

int obtenerDatos(t_obtener archivo){

	char pathFile[260];
	t_config * fileData;
	int var = 0;
	int end_flag = 0;
	char *dataObtenida = string_new();

	char blockPath[260];
	int readSize;
	FILE * file;

	if(archivo.modo_lectura==0){
		return 0;
	}

		sprintf(pathFile, "%s/Archivos/%s", configuracion.puntoMontaje, archivo.path);

		fileData = config_create(pathFile);

		char** bloques= config_get_array_value(fileData, "BLOCKS");

		float ratio = archivo.offset / metadata.bloque_size;

		if(ratio>0){

		int offblocks = floor(ratio);

		var = var+offblocks;

		archivo.offset=archivo.offset - metadata.bloque_size*offblocks;

		}

		while(bloques[var] != NULL && end_flag == 0){


			void * buffer = malloc(metadata.bloque_size);

			char * charBloque = bloques[var];

			int bloque = atoi(charBloque);

			sprintf(blockPath, "%s/Bloques/%d.bin", configuracion.puntoMontaje, bloque);

			 file = fopen(blockPath, "r");

			if(archivo.size <= ( metadata.bloque_size - archivo.offset)){
				readSize = archivo.size;
			} else if(archivo.size > metadata.bloque_size){
				readSize = metadata.bloque_size - archivo.offset;
			}


			int fd = fileno(file);

			pread(fd, buffer, readSize, archivo.offset);

			string_append(&dataObtenida, buffer);

			archivo.offset = 0;



			archivo.size = archivo.size - readSize;

			if(archivo.size == 0){
				end_flag = 1;
			}

			var++;

		}

		printf("%s", dataObtenida);


	return 1;
}

int guardarDatos(t_guardar archivo){


	t_config * fileData;

	off_t posicion = 0;

	FILE * file;

	void * block_buffer;

	int var = 0;
	int writeSize = 0;
	char pathFile[260];
	char *bloques_array = string_new();
	char pathBloque[260];
	char * block;
	char * char_size;
	int a = 0;

	sprintf(pathFile, "%s/Archivos/%s", configuracion.puntoMontaje, archivo.path);



	if(fopen(pathFile, "r")==NULL){
		return -1;
	}	if(archivo.modo_escritura == 0){
		return -2;
	}

	fileData = config_create(pathFile);

	char** bloques= config_get_array_value(fileData, "BLOCKS");
	int size = config_get_int_value(fileData, "SIZE");

	size+=archivo.size;

	char_size = string_itoa(size);

	config_set_value(fileData, "SIZE", char_size);
	config_save(fileData);

	string_append(&bloques_array,"[");

	while(bloques[var]!= NULL){

		block = bloques[var];

		string_append_with_format(&bloques_array, "%s,", block);

		var++;
	}

	float ratio = archivo.offset / metadata.bloque_size;

	var = 0;
	int new_blocks;

	if(ratio>0){
				int counter;
				int offblocks = floor(ratio);
				new_blocks = offblocks;
				for (counter = 0; counter < offblocks; ++counter) {

					if(bloques[var]!=NULL){
						posicion = atoi(bloques[var]);
					}
					var++;
					new_blocks--;

				}

					if(bloques[var]==NULL){

						if(bloquesLibres()==0){
							return -3;
						}

						posicion = 0;

					for (counter = 0; counter < new_blocks; ++counter) {

						while(bitarray_test_bit(bitarray, posicion)){

									posicion++;
							}
						}
				}


				archivo.offset=archivo.offset - metadata.bloque_size*offblocks;

			}

	posicion = atoi(bloques[var]);

		while(archivo.size>0){

			block_buffer = malloc(metadata.bloque_size);

			int bloque = posicion;

			sprintf(pathBloque, "%s/Bloques/%d.bin", configuracion.puntoMontaje, bloque);

			file = fopen(pathBloque, "r+");

			if(file == NULL){
				file = fopen(pathBloque, "w+");
			}

			if(archivo.size <= ( metadata.bloque_size - archivo.offset)){
					writeSize = archivo.size;


				} else if(archivo.size > metadata.bloque_size){
					writeSize = metadata.bloque_size - archivo.offset;

				}

			memcpy(block_buffer, archivo.buffer, writeSize);

			archivo.buffer += writeSize;

			int fd = fileno(file);

			pwrite(fd, block_buffer, writeSize, archivo.offset);

			bitarray_set_bit(bitarray, posicion);

			archivo.offset = 0;

			archivo.size = archivo.size - writeSize;

			block = string_itoa(bloque);

			if(!(string_contains(bloques_array, block))){

			string_append_with_format(&bloques_array, "%s,", block);

			}

			var++;

			if(bloques[var]==NULL){

				var--;
				if(bloquesLibres()==0){
					break;
				}

			while(bitarray_test_bit(bitarray, a)){

								a++;
							}
			posicion = a;

			} else
				posicion = atoi(bloques[var]);
		}


			string_append(&bloques_array, "]");

			config_set_value(fileData, "BLOCKS", bloques_array);

			config_save(fileData);

			if(archivo.size>0){
				return -1;
			}


	return 1;

}
