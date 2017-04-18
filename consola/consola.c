#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sockets.h>

typedef struct config_t {

	char * ipKernel;
	char * puertoKernel;

} t_configuracion;

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

	    	 write(socketCliente, "Soy consola", 15);
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

	configuracion.ipKernel = strdup(config_get_string_value(config, "IP_KERNEL"));
	configuracion.puertoKernel = strdup(config_get_string_value(config, "PUERTO_KERNEL"));


}



int main(int arc, char * argv[]) {

	cargarConfiguracion();

	printf("El puerto del Kernel %s\n",configuracion.puertoKernel);
	printf("La IP del Kernel %s\n",configuracion.ipKernel);

	int socketCliente = crearCliente();

	enviarMensaje(socketCliente);

	return 0;

}









