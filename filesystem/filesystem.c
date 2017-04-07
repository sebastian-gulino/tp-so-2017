#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>

typedef struct config_t {

	char * puerto;
	char * puertoMontaje;
} t_configuracion;

t_configuracion configuracion;

void cargarConfiguracion(void){

	t_config * config;

	config = config_create("/home/utnso/Escritorio/UTNSOTP/tp-2017-1c-Codeando-por-un-sueldo/filesystem/config.txt");

	configuracion.puerto = strdup(config_get_string_value(config, "PUERTO"));
	configuracion.puertoMontaje = strdup(config_get_string_value(config, "PUERTO_MONTAJE"));

}

int main(int arc, char * argv[]){

	cargarConfiguracion();

	printf("El Puerto es %s\n", configuracion.puerto);
	printf("El Puerto de Montaje es %s\n", configuracion.puertoMontaje);

	return 0;

}

