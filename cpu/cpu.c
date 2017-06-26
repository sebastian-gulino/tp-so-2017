
#include "cpuHelper.h"

int main(int arc, char * argv[]) {

	// Genera archivo log para poder escribir el trace de toda la ejecución
	logger = malloc(sizeof(t_log));
	crearLog("/CPU");

	// Levanta la configuración del proceso CPU
	configuracion = cargarConfiguracion();

	inicializarEstructuras();

	// Conecta la cpu incorporada al Kernel
	socketKernel = conectarAKernel();

	// Conecta la cpu incorporada a la Memoria
	socketMemoria = conectarAMemoria();

	// Defino el handler que manejará la señal SIGUSR1 para desconectar la CPU
	signal(SIGUSR1, manejarSignal);

	// Comienza a iterar para escuchar los pedidos de ejecución del kernel
	recibirProcesoKernel();

	return 0;

}


