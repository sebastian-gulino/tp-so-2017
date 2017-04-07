#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>

typedef struct config_t {

	char * puertoCpu;

} t_configuracion;

t_configuracion configuracion;

void cargarConfiguracion(void) {
	t_config * config;

	config = config_create("/home/utnso/workspace/tp-2017-1c-Codeando-por-un-sueldo/src/kernel/config.txt");

	configuracion.puertoCpu = strdup(config_get_string_value(config, "PUERTO_CPU"));

}



int main(int arc, char * argv[]) {

	cargarConfiguracion();

	printf("El puerto de la CPU es %s",configuracion.puertoCpu);

	return 0;

}
