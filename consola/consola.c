
#include "consolaHelper.h"

int main(int arc, char * argv[]) {

	//Limpio la consola del terminal antes de empezar
	system("clear");

	//Genera archivo log para poder escribir el trace de toda la ejecución
	//crearLog("/CONSOLA");
	remove("./CONSOLA.log");
	logger = log_create("./CONSOLA.log", "CONSOLA", 0, LOG_LEVEL_TRACE);

	//Levanta la configuración del proceso consola
	cargarConfiguracion();

	inicializarEstructuras();

	// Crea el hilo que funcionará como interfaz de usuario
	pthread_create(&threadCommandHandler, NULL, (void*)&commandHandler,NULL);

	pthread_join(threadCommandHandler, NULL);

	signal(SIGINT, manejarSignal);

	return 0;

}
