
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>
#include <commons/collections/list.h>
#include "manejoMemoria.h"



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

	crearMemoriaPrincipal();

	crearEstructurasAdministrativas();

//	t_filaTablaInvertida* fila = (t_filaTablaInvertida*)memoriaPrincipal;
//
//	escribirEnMemoria(6,"prueba de escritura en memoria");
//
//	void* pagina = leerPagina(6);
//	char* aver = (char*)pagina;
//	printf("%s",aver); //Leo lo que escribi en la página 6

	imprimirTablaPaginas();
	liberarMemoriaPrincipal();

	return 0;

}
