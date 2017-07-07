#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

	#include "estructuras.h"

	t_header crearHeader(uint8_t tipoEstructura, uint16_t lengthDatos);
	char * crearDataConHeader(uint8_t tipoEstructura, int length);

	int getStartInstruccion(t_intructions instruccion);
	int getOffsetInstruccion (t_intructions instruccion);
	t_intructions cargarIndiceCodigo(t_puntero_instruccion primera_instruccion, t_size offset_instruccion);

	t_stream * serialize(int tipoEstructura, void * estructuraOrigen);

	t_stream * serializeStruct_numero(t_struct_numero * estructuraOrigen);
	t_stream * serializeStruct_char(t_struct_char * estructuraOrigen);
	t_stream * serializeStruct_string(t_struct_string * estructuraOrigen);
	t_stream * serializeStruct_malc(t_struct_malloc * estructuraOrigen);
	t_stream * serializeStruct_prog(t_struct_programa * estructuraOrigen);
	t_stream * serializeStruct_impr(t_struct_string * estructuraOrigen);
	t_stream * serializeStruct_finProg(t_struct_numero * estructuraOrigen);
	t_stream * serializeStruct_pcb(t_struct_pcb * estructuraOrigen);
	t_stream * serializeStruct_pid(t_struct_numero * estructuraOrigen);
	t_stream * serializeStruct_lect(t_posicion_memoria * estructuraOrigen);
	t_stream * serializeStruct_lectvar(t_posicion_memoria * estructuraOrigen);
	t_stream * serializeStruct_abort(t_struct_numero * estructuraOrigen);
	t_stream * serializeStruct_sigusr1(t_struct_numero * estructuraOrigen);
	t_stream * serializeStruct_solEscr(t_struct_sol_escritura * estructuraOrigen);
	t_stream * serializeStruct_pcb_finOk(t_struct_pcb * estructuraOrigen);
	t_stream * serializeStruct_wait(t_struct_string * estructuraOrigen);
	t_stream * serializeStruct_obtComp(t_struct_string * estructuraOrigen);
	t_stream * serializeStruct_graComp(t_struct_var_compartida * estructuraOrigen);
	t_stream* serializeStruct_archivo_esc(t_struct_archivo * estructuraOrigen);

	t_header desempaquetarHeader(char * header);
	void * deserialize(uint8_t tipoEstructura, char * dataPaquete, uint16_t length);

	t_struct_numero * deserializeStruct_numero(char * dataPaquete, uint16_t length);
	t_struct_char * deserializeStruct_char(char * dataPaquete, uint16_t length);
	t_struct_string * deserializeStruct_string(char * dataPaquete, uint16_t length);
	t_struct_malloc * deserializeStruct_malc(char * dataPaquete, uint16_t length);
	t_struct_programa * deserializeStruct_prog(char* dataPaquete, uint16_t length);
	t_struct_string * deserializeStruct_impr(char * dataPaquete, uint16_t length);
	t_struct_numero * deserializeStruct_finProg(char * dataPaquete, uint16_t length);
	t_struct_pcb * deserializeStruct_pcb(char* dataPaquete, uint16_t length);
	t_struct_numero * deserializeStruct_pid(char * dataPaquete, uint16_t length);
	t_posicion_memoria * deserializeStruct_lect(char* dataPaquete, uint16_t length);
	t_posicion_memoria * deserializeStruct_lectvar(char* dataPaquete, uint16_t length);
	t_struct_numero * deserializeStruct_abort(char * dataPaquete, uint16_t length);
	t_struct_numero * deserializeStruct_sigusr1(char * dataPaquete, uint16_t length);
	t_struct_sol_escritura * deserializeStruct_solEscr(char* dataPaquete, uint16_t length);
	t_struct_pcb * deserializeStruct_pcb_finOk(char* dataPaquete, uint16_t length);
	t_struct_string * deserializeStruct_wait(char * dataPaquete, uint16_t length);
	t_struct_string * deserializeStruct_obtComp(char * dataPaquete, uint16_t length);
	t_struct_var_compartida * deserializeStruct_graComp(char * dataPaquete, uint16_t length);
	t_struct_archivo * deserializeStruct_archivo_esc(char* dataPaquete, uint16_t length);

#endif /* SERIALIZACION_H_ */
