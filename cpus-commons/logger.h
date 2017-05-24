#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include <string.h>
#include <unistd.h>

t_log* logger;

void crearLog(char* tipo_proceso);

#endif /* LOGGER_H_ */
