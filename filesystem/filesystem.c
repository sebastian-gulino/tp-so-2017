#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <estructuras.h>
#include "fsHelper.h"
#include "fsProcesos.h"

int main(int arc, char * argv[]){

		//Genera archivo log para poder escribir el trace de toda la ejecución
		logger = malloc(sizeof(t_log));
		crearLog("/FILESYSTEM");

		//Levanta la configuración del proceso filesystem
		configuracion = cargarConfiguracion();
		cargarMetadata();
		crearBitmap();

		t_obtener test;

		test.modo_lectura = 1;
		test.offset=15;
		test.path="passwords/alumnos/Test1.bin";
		test.size=55;

		t_obtener test1;

			test1.modo_lectura = 0;
			test1.offset=15;
			test1.path="passwords/alumnos/Test1.bin";
			test1.size=55;

		if(obtenerDatos(test1)==0){
			puts("OK!");
		}

		if(obtenerDatos(test) == 1){
			puts("NICE!");
		}

		crearServidorMonocliente();



		pthread_join(threadAtenderKernel, NULL);

		return 0;

}

