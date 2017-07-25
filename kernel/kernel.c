#include "kernelHelper.h"

int main(int arc, char * argv[]) {

	//Limpio la consola del terminal antes de empezar
	system("clear");

	//Genera archivo log para poder escribir el trace de toda la ejecución
	crearLog("/KERNEL");

	//Levanta la configuración del proceso kernel
	cargarConfiguracion();

	//Crea la lista de clientes conectados para cpu y consola
	inicializarListas();

	//Conecta el Kernel a la memoria del sistema
	socketMemoria = conectarAMemoria();

	//Conecta el Kernel al filesystem
	socketFS = conectarAFS();

	iniciarConsolaKernel();

	crearThreadAtenderConexiones();

	pthread_join(threadAtenderConexiones, NULL);

	return 0;

}


