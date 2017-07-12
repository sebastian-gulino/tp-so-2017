
#include "consolaHelper.h"

int main(int arc, char * argv[]) {

	//Limpio la consola del terminal antes de empezar
	system("clear");

	//Genera archivo log para poder escribir el trace de toda la ejecución
	crearLog("/CONSOLA");

	//Levanta la configuración del proceso consola
	configuracion = cargarConfiguracion();

	inicializarEstructuras();

	// Crea el hilo que funcionará como interfaz de usuario
	pthread_create(&threadCommandHandler, NULL, (void*)&commandHandler,NULL);

	pthread_join(threadCommandHandler, NULL);

	signal(SIGINT, manejarSignal);

	return 0;

}
