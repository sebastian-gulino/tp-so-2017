
#include "consolaHelper.h"

int main(int arc, char * argv[]) {

	//Limpio la consola del terminal antes de empezar
	system("clear");

	//Genera archivo log para poder escribir el trace de toda la ejecución
	logger = malloc(sizeof(t_log));
	crearLog("/CONSOLA");

	//Levanta la configuración del proceso consola
	configuracion = cargarConfiguracion();

	inicializarListas();

	// Crea el hilo que funcionará como interfaz de usuario
	pthread_create(&threadCommandHandler, NULL, commandHandler,NULL);

	pthread_join(threadCommandHandler, NULL);


	return 0;

}
