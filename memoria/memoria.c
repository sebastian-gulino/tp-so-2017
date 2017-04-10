
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct config_t {

	char * puerto;
	char * marcos;
	char * marcoSize;
	char * entradasCache;
	char * cacheXProc;
	char * reemplazoCache;
	char * retardoMemoria;

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

}



int main(int arc, char * argv[]) {

	cargarConfiguracion();

	printf("El Puerto es %s\n",configuracion.puerto);
	printf("La cantidad de Marcos es %s\n",configuracion.marcos);
	printf("El tamaño de cada Marco es %s\n",configuracion.marcoSize);
	printf("Las entradas en Cache son %s\n",configuracion.entradasCache);
	printf("La cantidad maxima de de entradas de la cache asignables a cada programa es %s\n",configuracion.cacheXProc);
	printf("El reemplazo de cache es %s\n",configuracion.reemplazoCache);
	printf("El retardo de la Memoria es %s\n",configuracion.retardoMemoria);

	int se;

		se = crearCliente();//Se guarda el return de crearCliente() para saber si hubo algún error.

		if(se){
			printf("No se creo cliente");
			return EXIT_FAILURE;
		}

	return 0;

}
