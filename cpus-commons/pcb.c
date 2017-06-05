/*
 * pcb.c
 *
 *  Created on: 3/6/2017
 *      Author: utnso
 */
#include "pcb.h"

t_pcb crearPCB(char* programa, int PID, int pageSize) {

	t_metadata_program *metadata = metadata_desde_literal(programa);

	malloc(sizeof(t_pcb));

	indiceCodigo = list_create();
	indiceStack = list_create();

	pcb.cantidadPaginas = sizeof(programa) % pageSize;
	pcb.exitcode = 0;

	int programCounter = 0;

	while (programCounter!= metadata->instrucciones_size) {

		malloc(sizeof(limitesInstrucciones));
		limitesInstrucciones.inicioInstruccion = metadata->instrucciones_serializado[programCounter].start;
		limitesInstrucciones.longitudInstruccion = metadata->instrucciones_serializado[programCounter].offset;

		list_add(indiceCodigo, (void*) &limitesInstrucciones);

		programCounter++;
	}

	pcb.indiceCodigo = indiceCodigo;
	pcb.indiceEtiquetas = metadata->etiquetas;
	pcb.indiceStack = indiceStack;

	metadata_destruir(metadata);

	return pcb;
}

t_stack crearStack(unsigned char pos, t_list * argumentos, t_list * variables, unsigned char retPos, t_posicion_memoria retVar) {
	t_stack stack;
	malloc(sizeof(t_stack));
	stack.posicion = pos;
	stack.argumentos = argumentos;
	stack.variables = variables;
	stack.retPos = retPos;
	stack.posicionVariableRetorno = retVar;

	return stack;

}
