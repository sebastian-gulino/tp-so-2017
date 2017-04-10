#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct config_t {

	char * puerto;
	char * puertoMontaje;
} t_configuracion;

t_configuracion configuracion;

//sock: Socket del cliente.
int sock;
//server: Estructura de las direcciones del servidor a conectarse.
struct sockaddr_in server;

int crearCliente(void){

	 //Se crea el socket del cliente.
	  sock = socket(AF_INET , SOCK_STREAM , 0);
	    if (sock == -1)
	    {
	        printf("No se pudo crear el socket");
	    }
	    puts("Socket creado exitosamente");

	   //Se instancian las direcciones del servidor a conectarse.
	   server.sin_addr.s_addr = inet_addr("127.0.0.1");//"127.0.0.1" es la ip de la maquina (localhost).
	   server.sin_family = AF_INET;//Familia de direcciones.
	   server.sin_port = htons( 8300 );//"8300" es el puerto del servidor a conectarse (en este caso es el servidor KERNEL{kernel.c}).

	   puts("Cliente creado.");
	   puts("Intentando conexión...");

	  //Hace la conexión al servidor.
	   if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	    {
	        perror("Conexión fallida. Error");
	        return 1;
	    }

	    puts("Conexión exitosa!\n");

	    close(sock); //Cierra la conexión.
	    return 0;
}

void cargarConfiguracion(void){

	t_config * config;

	config = config_create("./config.txt");

	configuracion.puerto = strdup(config_get_string_value(config, "PUERTO"));
	configuracion.puertoMontaje = strdup(config_get_string_value(config, "PUERTO_MONTAJE"));

}

int main(int arc, char * argv[]){

	cargarConfiguracion();

	printf("El Puerto es %s\n", configuracion.puerto);
	printf("El Puerto de Montaje es %s\n", configuracion.puertoMontaje);

	return 0;

	int se;

		se = crearCliente();//Se guarda el return de crearCliente() para saber si hubo algún error.

		if(se){
			printf("No se creo cliente");
			return EXIT_FAILURE;
		}

}

