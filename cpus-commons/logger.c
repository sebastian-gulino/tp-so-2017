#include "logger.h"

void crearLog(char* tipo_proceso){

	char* log = getcwd(NULL, 0);


	string_append(&log,tipo_proceso);
	string_append(&log,".log");

	remove(log);

	logger = log_create(log, tipo_proceso, 0, LOG_LEVEL_TRACE);

	log_info(logger,"Comienza a ejecutar el proceso %s ",tipo_proceso);

	free(log);

}
