#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <estructuras.h>
#include "fsHelper.h"

int main(int arc, char * argv[]){

		//Genera archivo log para poder escribir el trace de toda la ejecución
		logger = malloc(sizeof(t_log));
		crearLog("/FILESYSTEM");

		//Levanta la configuración del proceso filesystem
		configuracion = cargarConfiguracion();
		crearBitmap();

		crearServidorMonocliente();


		pthread_join(threadAtenderKernel, NULL);

		return 0;

}

