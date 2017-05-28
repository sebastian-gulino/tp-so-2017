#include "cpuHelper.h"

int main(int arc, char * argv[]) {

	//Genera archivo log para poder escribir el trace de toda la ejecución
	logger = malloc(sizeof(t_log));
	crearLog("/CPU");

	//Levanta la configuración del proceso CPU
	configuracion = cargarConfiguracion();

	int socketCPU = conectarAKernel();

	return 0;

}
