#ifndef FSPROCESOS_H_
#define FSPROCESOS_H_

#include "fsHelper.h"


int borrarArchivo(char * path);
int validarArchivo(t_abrir archivo);
int crearArchivo(char * path);
int obtenerDatos(t_obtener archivo);


#endif /* FSPROCESOS_H_ */
