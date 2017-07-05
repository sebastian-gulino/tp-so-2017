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

		t_abrir abrirTest1;

		abrirTest1.modo_creacion = 1;
		abrirTest1.path = "/cacho/test/Test1.bin";

		validarArchivo(abrirTest1);

		t_abrir abrirTest2;

		abrirTest2.modo_creacion = 1;
		abrirTest2.path = "/cacho/test/Test2.bin";

		validarArchivo(abrirTest2);

		t_abrir abrirTest3;

		abrirTest3.modo_creacion = 0;
		abrirTest3.path = "/cacho/test/test3.bin";

		validarArchivo(abrirTest3);

		t_borrar borrarTest;
		borrarTest.path = "/cacho/test/Test2.bin";

		borrarArchivo(borrarTest.path);

		t_guardar test;

		test.buffer = "aaaaasssssdddddfffffggggghhhhhjjjjjkkkkklllll";
		test.modo_escritura = 1;
		test.offset = 10;
		test.path = "/cacho/test/Test1.bin";
		test.size= 45;

		guardarDatos(test);

		t_guardar test1;

		test1.buffer = "22222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222333333333333333";

		test1.modo_escritura = 1;
		test1.offset = 30;
		test1.path = "/cacho/test/Test1.bin";
		test1.size= 140;

		guardarDatos(test1);

		t_obtener test2;

		test2.modo_lectura = 1;
		test2.offset = 20;
		test2.path = "/cacho/test/Test1.bin";
		test2.size = 45;

		obtenerDatos(test2);


		crearServidorMonocliente();


		pthread_join(threadAtenderKernel, NULL);

		return 0;

}

