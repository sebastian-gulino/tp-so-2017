#include "kernelHelper.h"

int main(int arc, char * argv[]) {

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

	crearThreadAtenderConexiones();

	pthread_join(threadAtenderConexiones, NULL);

	return 0;

}


