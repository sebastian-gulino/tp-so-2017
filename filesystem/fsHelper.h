#ifndef FSHELPER_H_
#define FSHELPER_H_

#include <commons/config.h>
#include <logger.h>

typedef struct config_t {

	int puertoFS;
	char * puntoMontaje;
} t_configuracion;

t_configuracion configuracion;

t_configuracion cargarConfiguracion();

#endif /* FSHELPER_H_ */
