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

	//Limpio la consola del terminal antes de empezar
	system("clear");

	remove("./FILESYSTEM.log");
	logger = log_create("./FILESYSTEM.log", "FILESYSTEM", 0, LOG_LEVEL_TRACE);

	//Levanta la configuración del proceso filesystem
	cargarConfiguracion();

	setPuntoDeMontaje();

	setMetadata();

	crearBitmap();

	crearServidorMonocliente();


	pthread_join(threadAtenderKernel, NULL);

	return 0;

}

