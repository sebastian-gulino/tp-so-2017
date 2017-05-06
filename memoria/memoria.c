
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <commons/collections/list.h>
#include "manejoMemoria.h"

t_configuracion configuracion;

//sock: Socket del cliente.
//cc: Return de la conexión
//ms: Return del envio de mensaje
int sock, cc, ms;
//server: Estructura de las direcciones del servidor a conectarse.
struct sockaddr_in server;
//mensaje: Mensaje a enviar
//respuesta: Respuesta del servidor
char mensaje[500] , respuesta[2000], unMensaje[500];

int enviarMensaje(int socketCliente){

	       while(1)
	       {


	    	 while(recv(socketCliente , respuesta , 2000 , 0)>0){
	    		 puts(respuesta);
	    	 }

	    	    if( send(socketCliente , mensaje , 300 , 0) > 5)
	    	       {
	    	        recv(socketCliente, respuesta, 2000, 0);
	    	           puts(respuesta);

	    	        } else {
	    	        	perror("No se pudo enviar el mensaje");
	    	        	return EXIT_FAILURE;
	    	        }


	       }

	    close(sock); //Cierra la conexión.
	    return 0;
}

void cargarConfiguracion(void) {
	t_config * config;

	config = config_create("./config.txt");

	configuracion.puerto = strdup(config_get_string_value(config, "PUERTO"));
	configuracion.marcos = strdup(config_get_string_value(config, "MARCOS"));
	configuracion.marcoSize = strdup(config_get_string_value(config, "MARCO_SIZE"));
	configuracion.entradasCache = strdup(config_get_string_value(config, "ENTRADAS_CACHE"));
	configuracion.cacheXProc = strdup(config_get_string_value(config, "CACHE_X_PROC"));
	configuracion.reemplazoCache = strdup(config_get_string_value(config, "REEMPLAZO_CACHE"));
	configuracion.retardoMemoria = strdup(config_get_string_value(config, "RETARDO_MEMORIA"));

	printf("El Puerto es %s\n",configuracion.puerto);
	printf("La cantidad de Marcos es %s\n",configuracion.marcos);
	printf("El tamaño de cada Marco es %s\n",configuracion.marcoSize);
	printf("Las entradas en Cache son %s\n",configuracion.entradasCache);
	printf("La cantidad maxima de de entradas de la cache asignables a cada programa es %s\n",configuracion.cacheXProc);
	printf("El reemplazo de cache es %s\n",configuracion.reemplazoCache);
	printf("El retardo de la Memoria es %s\n",configuracion.retardoMemoria);
}



int main(void) {

	setvbuf (stdout, NULL, _IONBF, 0);

//	char * prueba = "prueba";

//	char * mensaje = empaquetar('C', prueba);

//	printf("El Mensaje es %s\n",desempaquetar(mensaje));
//
//	printf("El Emisor es %s\n",procesoEmisor(mensaje));

//	cargarConfiguracion();
//
//
//	int socketCliente = crearCliente();
//
//	enviarMensaje(socketCliente);
//
	cargarConfiguracion();

	void* memoriaPrincipal = crearMemoriaPrincipal(atoi(configuracion.marcos),atoi(configuracion.marcoSize));

	crearEstructurasAdministrativas(memoriaPrincipal,atoi(configuracion.marcos));

	t_filaTablaInvertida* fila = (t_filaTablaInvertida*)memoriaPrincipal;

	escribirEnMemoria(6,"prueba de escritura en memoria");

	void* pagina = leerPagina(6);
	char* aver = (char*)pagina;
	printf("%s",aver);

	liberarMemoriaPrincipal(memoriaPrincipal);

	return 0;

}
