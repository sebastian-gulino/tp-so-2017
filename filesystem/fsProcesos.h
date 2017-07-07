#ifndef FSPROCESOS_H_
#define FSPROCESOS_H_

#include "fsHelper.h"


void borrarArchivo(t_struct_borrar * archivo);
void validarArchivo(t_struct_abrir * archivo);
int crearArchivo(char * path);
void obtenerDatos(t_struct_obtener * archivo);
void guardarDatos(t_struct_guardar * archivo);

#endif /* FSPROCESOS_H_ */
