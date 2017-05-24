#include "kernelHelper.h"

t_configuracion cargarConfiguracion() {

	t_config * config;
	t_configuracion configuracion;

	config = config_create("./config.txt");

	if(config == NULL){

		config = config_create("../config.txt");

	}  //TODO: Encontrar una forma menos villera de hacer esto. Seba dixit.

	//Ir liberando datos de la configuracion a medida que sean necesarios

	//	configuracion.puertoCpu = strdup(config_get_string_value(config, "PUERTO_CPU"));
	//	configuracion.ipMemoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
	//	configuracion.puertoProg = strdup(config_get_string_value(config, "PUERTO_PROG"));
	//	configuracion.puertoMemoria = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
	//	configuracion.ipFS = strdup(config_get_string_value(config, "IP_FS"));
	//	configuracion.puertoFS = strdup(config_get_string_value(config, "PUERTO_FS"));
	//	configuracion.quantum = strdup(config_get_string_value(config, "QUANTUM"));
	//	configuracion.quantumSleep = strdup(config_get_string_value(config, "QUANTUM_SLEEP"));
	//	configuracion.algoritmo = strdup(config_get_string_value(config, "ALGORITMO"));
	//	configuracion.gradoMultiprog = strdup(config_get_string_value(config, "GRADO_MULTIPROG"));
	//	configuracion.semIDS = strdup(config_get_string_value(config, "SEM_IDS"));
	//	configuracion.semINIT = strdup(config_get_string_value(config, "SEM_INIT"));
	//	configuracion.sharedVars = strdup(config_get_string_value(config, "SHARED_VARS"));
	//	configuracion.stackSize = strdup(config_get_string_value(config, "STACK_SIZE"));

	configuracion.puertoEscucha = config_get_int_value(config,"PUERTO_ESCUCHA");

	//		printf("El puerto de la CPU es %s\n",configuracion.puertoCpu);
	//		printf("La IP de la Memoria es %s\n",configuracion.ipMemoria);
	//		printf("El puerto de la Memoria es %s\n",configuracion.puertoMemoria);
	//		printf("El puerto del Programa es %s\n",configuracion.puertoProg);
	//		printf("La IP del FS es %s\n",configuracion.ipFS);
	//		printf("El puero del FS es %s\n",configuracion.puertoFS);
	//		printf("El Quantum es %s\n",configuracion.quantum);
	//		printf("EL Quantum Sleep es %s\n",configuracion.quantumSleep);
	//		printf("El Algoritmo es %s\n",configuracion.algoritmo);
	//		printf("El grado multiprog es %s\n",configuracion.gradoMultiprog);
	//		printf("La Sem IDS es %s\n",configuracion.semIDS);
	//		printf("La Sem INIT es %s\n",configuracion.semINIT);
	//		printf("Las variables compartidas son %s\n",configuracion.sharedVars);
	//		printf("El tamaño del Stack es %s\n",configuracion.stackSize);

	return configuracion;
}
