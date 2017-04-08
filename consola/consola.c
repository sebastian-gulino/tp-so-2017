#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>

typedef struct config_t {

	char * ipKernel;
	char * puertoKernel;

} t_configuracion;

t_configuracion configuracion;

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

	return 0;

}









