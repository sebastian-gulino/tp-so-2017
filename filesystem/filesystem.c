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

		setPuntoDeMontaje();

		setMetadata();

		crearBitmap();
//
		t_abrir test1;
//
		test1.modo_creacion = 1;
		test1.path = "/cacho/test/Test5.bin";
//
		if(validarArchivo(test1)==0){
		puts("good");
		}

//		t_borrar test2;
//		test2.path = "/cacho/test/Test5.bin";
//
//		borrarArchivo(test2.path);
//
//		t_guardar test;
//
//				test.buffer = "aaaaasssssdddddfffffggggghhhhhjjjjjkkkkklllll";
//
//				test.modo_escritura = 1;
//				test.offset = 10;
//				test.path = "/cacho/test/Test5.bin";
//				test.size= 45;
//
//				guardarDatos(test);

	t_guardar test;

		test.buffer = "22222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222333333333333333";

		test.modo_escritura = 1;
		test.offset = 30;
		test.path = "/cacho/test/Test5.bin";
		test.size= 140;

		if(guardarDatos(test)==-1){
			puts("i'm genius");
		}



//		t_obtener test2;
//
//		test2.modo_lectura = 1;
//		test2.offset = 20;
//		test2.path = "/cacho/test/Test1.bin";
//		test2.size = 45;
//
//		obtenerDatos(test2);


		crearServidorMonocliente();


		pthread_join(threadAtenderKernel, NULL);

		return 0;

}

