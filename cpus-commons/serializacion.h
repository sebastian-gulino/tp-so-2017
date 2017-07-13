#ifndef SERIALIZACION_H_
#define SERIALIZACION_H_

	#include "estructuras.h"

	t_header crearHeader(uint8_t tipoEstructura, uint16_t lengthDatos);
	char * crearDataConHeader(uint8_t tipoEstructura, int length);

	int getStartInstruccion(t_intructions instruccion);
	int getOffsetInstruccion (t_intructions instruccion);
	t_intructions cargarIndiceCodigo(t_puntero_instruccion primera_instruccion, t_size offset_instruccion);

	t_stream * serialize(int tipoEstructura, void * estructuraOrigen);

	t_stream * serializeStruct_numero(t_struct_numero * estructuraOrigen, int headerOperacion);
	t_stream * serializeStruct_char(t_struct_char * estructuraOrigen, int headerOperacion);
	t_stream * serializeStruct_string(t_struct_string * estructuraOrigen, int headerOperacion);
	t_stream * serializeStruct_malc(t_struct_malloc * estructuraOrigen);
	t_stream* serializeStruct_prog(t_struct_programa * estructuraOrigen, int headerOperacion);
	t_stream * serializeStruct_pcb(t_struct_pcb * estructuraOrigen, int headerOperacion);
	t_stream * serializeStruct_lect(t_posicion_memoria * estructuraOrigen, int headerOperacion);
	t_stream * serializeStruct_solLect(t_struct_sol_lectura * estructuraOrigen, int headerOperacion);
	t_stream * serializeStruct_solEscr(t_struct_sol_escritura * estructuraOrigen, int headerOperacion);
	t_stream * serializeStruct_graComp(t_struct_var_compartida * estructuraOrigen);
	t_stream* serializeStruct_archivo_esc(t_struct_archivo * estructuraOrigen, int headerOperacion);
	t_stream * serializeStruct_solHeap(t_struct_sol_heap * estructuraOrigen);
	t_stream * serializeStruct_libHeap(t_struct_sol_heap * estructuraOrigen);
	t_stream * serializeStruct_borrar(t_struct_borrar * estructuraOrigen);
	t_stream * serializeStruct_abrir(t_struct_abrir * estructuraOrigen);
	t_stream * serializeStruct_obtener(t_struct_obtener * estructuraOrigen);
	t_stream * serializeStruct_guardar(t_struct_guardar * estructuraOrigen);

	t_header desempaquetarHeader(char * header);
	void * deserialize(uint8_t tipoEstructura, char * dataPaquete, uint16_t length);

	t_struct_numero * deserializeStruct_numero(char * dataPaquete, uint16_t length);
	t_struct_char * deserializeStruct_char(char * dataPaquete, uint16_t length);
	t_struct_string * deserializeStruct_string(char * dataPaquete, uint16_t length);
	t_struct_malloc * deserializeStruct_malc(char * dataPaquete, uint16_t length);
	t_struct_programa * deserializeStruct_prog(char* dataPaquete, uint16_t length);
	t_struct_pcb * deserializeStruct_pcb(char* dataPaquete, uint16_t length);
	t_posicion_memoria * deserializeStruct_lect(char* dataPaquete, uint16_t length);
	t_struct_sol_escritura * deserializeStruct_solEscr(char* dataPaquete, uint16_t length);
	t_struct_sol_lectura * deserializeStruct_solLect(char* dataPaquete, uint16_t length);
	t_struct_var_compartida * deserializeStruct_graComp(char * dataPaquete, uint16_t length);
	t_struct_archivo * deserializeStruct_archivo_esc(char* dataPaquete, uint16_t length);
	t_struct_sol_heap * deserializeStruct_solHeap(char* dataPaquete, uint16_t length);
	t_struct_sol_heap * deserializeStruct_libHeap(char* dataPaquete, uint16_t length);
	t_struct_borrar * deserializeStruct_borrar(char * dataPaquete, uint16_t length);
	t_struct_abrir * deserializeStruct_abrir(char * dataPaquete, uint16_t length);
	t_struct_obtener * deserializeStruct_obtener(char * dataPaquete, uint16_t length);
	t_struct_guardar * deserializeStruct_guardar(char * dataPaquete, uint16_t length);

#endif /* SERIALIZACION_H_ */
