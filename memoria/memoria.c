
#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>

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

void cargarConfiguracion(void) {
	t_config * config;

	config = config_create("/home/utnso/Escritorio/UTNSOTP/tp-2017-1c-Codeando-por-un-sueldo/memoria/config.txt");

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


	return 0;

}
