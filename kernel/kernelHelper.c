#include "kernelHelper.h"

t_configuracion cargarConfiguracion() {

	t_config * config;
	t_configuracion configuracion;

	config = config_create("./config.txt");

	if(config == NULL){

		config = config_create("../config.txt");

	}  //TODO: Encontrar una forma menos villera de hacer esto. Seba dixit.

	configuracion.puertoCpu = config_get_int_value(config, "PUERTO_CPU");
	log_info(logger,"PUERTO_CPU = %d",configuracion.puertoCpu);

	configuracion.ipMemoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
	log_info(logger,"IP_MEMORIA = %s",configuracion.ipMemoria);

	configuracion.puertoProg = config_get_int_value(config, "PUERTO_PROG");
	log_info(logger,"PUERTO_PROG = %d",configuracion.puertoProg);

	configuracion.puertoMemoria = config_get_int_value(config, "PUERTO_MEMORIA");
	log_info(logger,"PUERTO_MEMORIA = %d",configuracion.puertoMemoria);

	configuracion.ipFS = strdup(config_get_string_value(config, "IP_FS"));
	log_info(logger,"IP_FILESYSTEM = %s",configuracion.ipFS);

	configuracion.puertoFS = config_get_int_value(config, "PUERTO_FS");
	log_info(logger,"PUERTO_FILESYSTEM = %d",configuracion.puertoFS);

	configuracion.quantum = config_get_int_value(config, "QUANTUM");
	log_info(logger,"QUANTUM = %d",configuracion.quantum);

	configuracion.quantumSleep = strdup(config_get_string_value(config, "QUANTUM_SLEEP"));
	log_info(logger,"QUANTUM_SLEEP = %s",configuracion.quantumSleep);

	configuracion.algoritmo = strdup(config_get_string_value(config, "ALGORITMO"));
	log_info(logger,"ALGORITMO = %s",configuracion.algoritmo);

	configuracion.gradoMultiprog = config_get_int_value(config, "GRADO_MULTIPROG");
	log_info(logger,"GRADO_MULTIPROGRAMACION = %d",configuracion.gradoMultiprog);

	configuracion.semIDS = strdup(config_get_string_value(config, "SEM_IDS"));
	log_info(logger,"SEMAFOROS_IDS = %s",configuracion.semIDS);

	configuracion.semINIT = strdup(config_get_string_value(config, "SEM_INIT"));
	log_info(logger,"SEMAFOROS_INI = %s",configuracion.semINIT);

	configuracion.sharedVars = strdup(config_get_string_value(config, "SHARED_VARS"));
	log_info(logger,"SHARED_VARS = %s",configuracion.sharedVars);

	configuracion.stackSize = strdup(config_get_string_value(config, "STACK_SIZE"));
	log_info(logger,"STACK_SIZE = %s",configuracion.stackSize);

	configuracion.puertoEscucha = config_get_int_value(config,"PUERTO_ESCUCHA");
	log_info(logger,"PUERTO_ESCUCHA = %d",configuracion.puertoEscucha);

	return configuracion;
}
