
#include "consolaHelper.h"

int main(int arc, char * argv[]) {

	//Genera archivo log para poder escribir el trace de toda la ejecución
	logger = malloc(sizeof(t_log));
	crearLog("/CONSOLA");

	//Levanta la configuración del proceso consola
	configuracion = cargarConfiguracion();

	int socketConsola = conectarAKernel();

	return 0;

}
