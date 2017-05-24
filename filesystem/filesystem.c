#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <logger.h>
#include <estructuras.h>

typedef struct config_t {

	char * puertoFS;
	char * puntoMontaje;
} t_configuracion;

t_configuracion configuracion;

char buffLog[80];

//sock: Socket del cliente.
//cc: Return de la conexión
//ms: Return del envio de mensaje
int sock, cc, ms;
//server: Estructura de las direcciones del servidor a conectarse.
struct sockaddr_in server;
//mensaje: Mensaje a enviar
//respuesta: Respuesta del servidor
char mensaje[500] , respuesta[2000], unMensaje[500];


void cargarConfiguracion(void) {

	t_config * config;

	config = config_create("./config.txt");

	configuracion.puertoFS = config_get_int_value(config, "PUERTO");
	sprintf(buffLog,"PUERTO_FS = [%d]",configuracion.puertoFS);
	log_debug(logger,buffLog);

	configuracion.puntoMontaje = strdup(config_get_string_value(config, "PUNTO_MONTAJE"));
	sprintf(buffLog,"PUNTO_MONTAJE = [%s]",configuracion.puntoMontaje);
	log_debug(logger,buffLog);

}


int main(int arc, char * argv[]){

		//Genera archivo log para poder escribir el trace de toda la ejecución
		logger = malloc(sizeof(t_log));

		crearLog("/FILESYSTEM");

		//Levanta la configuración del proceso filesystem
		cargarConfiguracion();

		//int socketCliente = crearCliente();

		//enviarMensaje(socketCliente);
		return 0;

}

