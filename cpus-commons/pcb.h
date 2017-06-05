#ifndef PCB_H_
#define PCB_H_
#include "estructuras.h"
#include <parser/metadata_program.h>
#include <parser/parser.h>
#include <commons/collections/list.h>

t_pcb pcb;
t_limites_instrucciones limitesInstrucciones;
t_list * indiceCodigo;
t_list * indiceStack;

t_pcb crearPCB(char* programa, int PID, int pageSize);

#endif /* PCB_H_ */
