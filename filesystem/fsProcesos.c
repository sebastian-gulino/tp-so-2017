#include "fsProcesos.h"


int validarArchivo(t_abrir archivo){

	char pathFile[260];
	int fd = 3;

	sprintf(pathFile, "%s/Archivos/%s", configuracion.puntoMontaje, archivo.path);

	FILE * file = fopen(pathFile, "r");

	if (file == NULL){

		if (archivo.modo_creacion==1){

			if(crearArchivo(archivo.path) == 1){

				fd++;

				return fd;

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

		archivo.offset=archivo.offset - metadata.bloque_size;

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

