
#include "consolaHelper.h"

int main(int arc, char * argv[]) {

	//Limpio la consola del terminal antes de empezar
	system("clear");

	remove("./CONSOLA.log");
	logger = log_create("./CONSOLA.log", "CONSOLA", 0, LOG_LEVEL_TRACE);

	//Levanta la configuración del proceso consola
	cargarConfiguracion();

	inicializarEstructuras();

	signal(SIGINT, manejarSignal);

	commandHandler();

	return 0;

}
