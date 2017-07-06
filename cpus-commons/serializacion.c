#include "serializacion.h"

t_header crearHeader(uint8_t tipoEstructura, uint16_t lengthDatos){
	t_header header;
	header.tipoEstructura = tipoEstructura;
	header.length = lengthDatos;
	return header;
}

char * crearDataConHeader(uint8_t tipoEstructura, int length){
	char * data = malloc(length);

	uint16_t lengthDatos = length - sizeof(t_header);

	t_header header = crearHeader(tipoEstructura, lengthDatos); //creo el header

	int tamanoTotal = 0, tamanoDato = 0;

	memcpy(data, &header.tipoEstructura, tamanoDato = sizeof(uint8_t)); //copio el tipoEstructura del header a data
	tamanoTotal = tamanoDato;
	memcpy(data + tamanoTotal, &header.length, tamanoDato = sizeof(uint16_t)); //copio el length del header a data

	return data;
}

int getStartInstruccion(t_intructions instruccion){ return instruccion.start; }

int getOffsetInstruccion (t_intructions instruccion){ return instruccion.offset; }

t_intructions cargarIndiceCodigo(t_puntero_instruccion comienzo_instruccion, t_size longitud_instruccion){

	t_intructions instruccion = { .start = comienzo_instruccion, .offset = longitud_instruccion };

	return instruccion;
}

t_stream * serialize(int tipoEstructura, void * estructuraOrigen){
	t_stream * paquete=NULL;

	switch (tipoEstructura){
			case D_STRUCT_NUMERO:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen);
				break;
			case D_STRUCT_CHAR:
				paquete = serializeStruct_char((t_struct_char *) estructuraOrigen);
				break;
			case D_STRUCT_STRING:
				paquete = serializeStruct_string((t_struct_string *) estructuraOrigen);
				break;
			case D_STRUCT_MALC:
				paquete = serializeStruct_malc((t_struct_malloc *) estructuraOrigen);
				break;
			case D_STRUCT_PROG:
				paquete = serializeStruct_prog((t_struct_programa *) estructuraOrigen);
				break;
			case D_STRUCT_IMPR:
				paquete = serializeStruct_impr((t_struct_string *) estructuraOrigen);
				break;
			case D_STRUCT_FIN_PROG:
				paquete = serializeStruct_finProg((t_struct_numero *) estructuraOrigen);
				break;
			case D_STRUCT_PCB:
				paquete = serializeStruct_pcb((t_struct_pcb *) estructuraOrigen);
				break;
			case D_STRUCT_PID:
				paquete = serializeStruct_pid((t_struct_numero *) estructuraOrigen);
				break;
			case D_STRUCT_LECT:
				paquete = serializeStruct_lect((t_posicion_memoria *) estructuraOrigen);
				break;
			case D_STRUCT_LECT_VAR:
				paquete = serializeStruct_lectvar((t_posicion_memoria *) estructuraOrigen);
				break;
			case D_STRUCT_ABORT:
				paquete = serializeStruct_abort((t_struct_numero *) estructuraOrigen);
				break;
			case D_STRUCT_SIGUSR1:
				paquete = serializeStruct_sigusr1((t_struct_numero *) estructuraOrigen);
				break;
			case D_STRUCT_SOL_ESCR:
				paquete = serializeStruct_solEscr((t_struct_sol_escritura *) estructuraOrigen);
				break;
			case D_STRUCT_PCB_FINOK:
				paquete = serializeStruct_pcb_finOk((t_struct_pcb *) estructuraOrigen);
				break;
			case D_STRUCT_WAIT:
				paquete = serializeStruct_wait((t_struct_string *) estructuraOrigen);
				break;
			case D_STRUCT_OBTENER_COMPARTIDA:
				paquete = serializeStruct_obtComp((t_struct_string *) estructuraOrigen);
				break;
		}

	return paquete;
}

t_stream * serializeStruct_numero(t_struct_numero * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + sizeof(int32_t);

	char * data = crearDataConHeader(D_STRUCT_NUMERO, paquete->length); //creo el data

	memcpy(data + sizeof(t_header), estructuraOrigen, sizeof(t_struct_numero));		//copio a data el numero.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_char(t_struct_char * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + sizeof(unsigned int);

	char * data = crearDataConHeader(D_STRUCT_CHAR, paquete->length); //creo el data

	memcpy(data + sizeof(t_header), &estructuraOrigen->letra, sizeof(char));		//copio a data el char.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_string(t_struct_string * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + strlen(estructuraOrigen->string) + 1;

	char * data = crearDataConHeader(D_STRUCT_STRING, paquete->length); //creo el data

	int tamanoTotal = sizeof(t_header);

	memcpy(data + tamanoTotal, estructuraOrigen->string, strlen(estructuraOrigen->string)+1);		//copio a data el string.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_malc(t_struct_malloc * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + 2*sizeof(uint32_t);

	char * data = crearDataConHeader(D_STRUCT_MALC, paquete->length); //creo el data

	memcpy(data + sizeof(t_header), estructuraOrigen, sizeof(t_struct_malloc));		//copio a data el numero.

	paquete->data = data;

	return paquete;
}

t_stream* serializeStruct_prog(t_struct_programa * estructuraOrigen){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) + sizeof(uint32_t) +sizeof(uint32_t) + estructuraOrigen->tamanio;

	char* data = crearDataConHeader(D_STRUCT_PROG, paquete->length);

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->PID, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->base, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->tamanio, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, estructuraOrigen->buffer, tamanoDato = (estructuraOrigen->tamanio));

	paquete->data = data;

	return paquete;

}

t_stream * serializeStruct_impr(t_struct_string * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + strlen(estructuraOrigen->string) + 1;

	char * data = crearDataConHeader(D_STRUCT_IMPR, paquete->length); //creo el data

	int tamanoTotal = sizeof(t_header);

	memcpy(data + tamanoTotal, estructuraOrigen->string, strlen(estructuraOrigen->string)+1);		//copio a data el string.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_finProg(t_struct_numero * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + sizeof(int32_t);

	char * data = crearDataConHeader(D_STRUCT_FIN_PROG, paquete->length); //creo el data

	memcpy(data + sizeof(t_header), estructuraOrigen, sizeof(t_struct_numero));		//copio a data el numero.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_pcb(t_struct_pcb * estructuraOrigen){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(t_struct_pcb);

	char* data = crearDataConHeader(D_STRUCT_PCB, paquete->length);

	int tamDato = 0;
	int tamTot = sizeof(t_header);

	memcpy(data + tamTot , &estructuraOrigen->PID, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	estructuraOrigen->cantRegistrosStack = estructuraOrigen->indiceStack->elements_count;
	memcpy(data + tamTot , &estructuraOrigen->cantRegistrosStack, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->cantidadInstrucciones, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->tamanioIndiceEtiquetas, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->cpuID, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->exitcode, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	int contadorInstrucciones = 0;

	while (contadorInstrucciones < estructuraOrigen->cantidadInstrucciones){
		t_puntero_instruccion startInstruccion = getStartInstruccion((estructuraOrigen->indiceCodigo)[contadorInstrucciones]);
		memcpy(data + tamTot , &startInstruccion, tamDato = sizeof(t_puntero_instruccion));
		tamTot += tamDato;

		t_size offsetInstruccion = getOffsetInstruccion((estructuraOrigen->indiceCodigo)[contadorInstrucciones]);
		memcpy(data + tamTot , &offsetInstruccion, tamDato = sizeof(t_size));
		tamTot += tamDato;

		contadorInstrucciones++;
	}

	memcpy(data + tamTot , estructuraOrigen->indiceEtiquetas, tamDato = estructuraOrigen->tamanioIndiceEtiquetas);
	tamTot += tamDato;

	void serializeRegistroStack(registroStack * registro){

		void serializeVarsList(t_variable* variable){
			memcpy(data + tamTot , &variable->identificador, tamDato = sizeof(char));
			tamTot += tamDato;

			memcpy(data + tamTot , &variable->posicionMemoria, tamDato = sizeof(t_posicion_memoria));
			tamTot += tamDato;
		}

		registro->cantidad_args = registro->args->elements_count;

		memcpy(data + tamTot , &registro->cantidad_args, tamDato = sizeof(int));
		tamTot += tamDato;

		list_iterate(registro->args, (void*) serializeVarsList);

		memcpy(data + tamTot , &registro->args->elements_count, tamDato = sizeof(int));
		tamTot += tamDato;

		registro->cantidad_vars = registro->vars->elements_count;

		memcpy(data + tamTot , &registro->cantidad_vars, tamDato = sizeof(int));
		tamTot += tamDato;

		list_iterate(registro->vars, (void*) serializeVarsList);

		memcpy(data + tamTot , &registro->vars->elements_count, tamDato = sizeof(int));
		tamTot += tamDato;

		memcpy(data + tamTot , &registro->retPos, tamDato = sizeof(int));
		tamTot += tamDato;

		memcpy(data + tamTot , &registro->retVar, tamDato = sizeof(t_posicion_memoria));
		tamTot += tamDato;

	}

	list_iterate(estructuraOrigen->indiceStack, (void*) serializeRegistroStack);

	memcpy(data + tamTot , &estructuraOrigen->paginaActualStack, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->paginasCodigo, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->paginasStack, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->primerPaginaStack, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->programCounter, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->stackPointer, tamDato = sizeof(int));
	tamTot += tamDato;

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_pid(t_struct_numero * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + sizeof(int32_t);

	char * data = crearDataConHeader(D_STRUCT_PID, paquete->length); //creo el data

	memcpy(data + sizeof(t_header), estructuraOrigen, sizeof(t_struct_numero));		//copio a data el numero.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_lect(t_posicion_memoria * estructuraOrigen){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) + sizeof(uint32_t) +sizeof(uint32_t);

	char* data = crearDataConHeader(D_STRUCT_LECT, paquete->length);

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->pagina, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->offsetInstruccion, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->longitudInstruccion, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;

}

t_stream * serializeStruct_lectvar(t_posicion_memoria * estructuraOrigen){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) + sizeof(uint32_t) +sizeof(uint32_t);

	char* data = crearDataConHeader(D_STRUCT_LECT_VAR, paquete->length);

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->pagina, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->offsetInstruccion, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->longitudInstruccion, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;

}

t_stream * serializeStruct_abort(t_struct_numero * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + sizeof(int32_t);

	char * data = crearDataConHeader(D_STRUCT_ABORT, paquete->length); //creo el data

	memcpy(data + sizeof(t_header), estructuraOrigen, sizeof(t_struct_numero));		//copio a data el numero.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_sigusr1(t_struct_numero * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + sizeof(int32_t);

	char * data = crearDataConHeader(D_STRUCT_SIGUSR1, paquete->length); //creo el data

	memcpy(data + sizeof(t_header), estructuraOrigen, sizeof(t_struct_numero));		//copio a data el numero.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_solEscr(t_struct_sol_escritura * estructuraOrigen){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) + sizeof(uint32_t) +sizeof(uint32_t);

	char* data = crearDataConHeader(D_STRUCT_SOL_ESCR, paquete->length);

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->pagina, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->offset, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->contenido, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;

}

t_stream * serializeStruct_pcb_finOk(t_struct_pcb * estructuraOrigen){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(t_struct_pcb);

	char* data = crearDataConHeader(D_STRUCT_PCB_FINOK, paquete->length);

	int tamDato = 0;
	int tamTot = sizeof(t_header);

	memcpy(data + tamTot , &estructuraOrigen->PID, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	estructuraOrigen->cantRegistrosStack = estructuraOrigen->indiceStack->elements_count;
	memcpy(data + tamTot , &estructuraOrigen->cantRegistrosStack, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->cantidadInstrucciones, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->tamanioIndiceEtiquetas, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->cpuID, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->exitcode, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	int contadorInstrucciones = 0;

	while (contadorInstrucciones < estructuraOrigen->cantidadInstrucciones){
		t_puntero_instruccion startInstruccion = getStartInstruccion((estructuraOrigen->indiceCodigo)[contadorInstrucciones]);
		memcpy(data + tamTot , &startInstruccion, tamDato = sizeof(t_puntero_instruccion));
		tamTot += tamDato;

		t_size offsetInstruccion = getOffsetInstruccion((estructuraOrigen->indiceCodigo)[contadorInstrucciones]);
		memcpy(data + tamTot , &offsetInstruccion, tamDato = sizeof(t_size));
		tamTot += tamDato;

		contadorInstrucciones++;
	}

	memcpy(data + tamTot , estructuraOrigen->indiceEtiquetas, tamDato = estructuraOrigen->tamanioIndiceEtiquetas);
	tamTot += tamDato;

	void serializeRegistroStack(registroStack * registro){

		void serializeVarsList(t_variable* variable){
			memcpy(data + tamTot , &variable->identificador, tamDato = sizeof(char));
			tamTot += tamDato;

			memcpy(data + tamTot , &variable->posicionMemoria, tamDato = sizeof(t_posicion_memoria));
			tamTot += tamDato;
		}

		registro->cantidad_args = registro->args->elements_count;

		memcpy(data + tamTot , &registro->cantidad_args, tamDato = sizeof(int));
		tamTot += tamDato;

		list_iterate(registro->args, (void*) serializeVarsList);

		memcpy(data + tamTot , &registro->args->elements_count, tamDato = sizeof(int));
		tamTot += tamDato;

		registro->cantidad_vars = registro->vars->elements_count;

		memcpy(data + tamTot , &registro->cantidad_vars, tamDato = sizeof(int));
		tamTot += tamDato;

		list_iterate(registro->vars, (void*) serializeVarsList);

		memcpy(data + tamTot , &registro->vars->elements_count, tamDato = sizeof(int));
		tamTot += tamDato;

		memcpy(data + tamTot , &registro->retPos, tamDato = sizeof(int));
		tamTot += tamDato;

		memcpy(data + tamTot , &registro->retVar, tamDato = sizeof(t_posicion_memoria));
		tamTot += tamDato;

	}

	list_iterate(estructuraOrigen->indiceStack, (void*) serializeRegistroStack);

	memcpy(data + tamTot , &estructuraOrigen->paginaActualStack, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->paginasCodigo, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->paginasStack, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->primerPaginaStack, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->programCounter, tamDato = sizeof(int));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->stackPointer, tamDato = sizeof(int));
	tamTot += tamDato;

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_wait(t_struct_string * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + strlen(estructuraOrigen->string) + 1;

	char * data = crearDataConHeader(D_STRUCT_WAIT, paquete->length); //creo el data

	int tamanoTotal = sizeof(t_header);

	memcpy(data + tamanoTotal, estructuraOrigen->string, strlen(estructuraOrigen->string)+1);		//copio a data el string.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_obtComp(t_struct_string * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + strlen(estructuraOrigen->string) + 1;

	char * data = crearDataConHeader(D_STRUCT_OBTENER_COMPARTIDA, paquete->length); //creo el data

	int tamanoTotal = sizeof(t_header);

	memcpy(data + tamanoTotal, estructuraOrigen->string, strlen(estructuraOrigen->string)+1);		//copio a data el string.

	paquete->data = data;

	return paquete;
}

t_header desempaquetarHeader(char * header){
	t_header estructuraHeader;

	int tamanoTotal = 0, tamanoDato = 0;
	memcpy(&estructuraHeader.tipoEstructura, header + tamanoTotal, tamanoDato = sizeof(uint8_t));
	tamanoTotal = tamanoDato;
	memcpy(&estructuraHeader.length, header + tamanoTotal, tamanoDato = sizeof(uint16_t));

	return estructuraHeader;
}

void * deserialize(uint8_t tipoEstructura, char * dataPaquete, uint16_t length){

	void * estructuraDestino;

	switch (tipoEstructura){
			case D_STRUCT_NUMERO:
				estructuraDestino = deserializeStruct_numero(dataPaquete, length);
				break;
			case D_STRUCT_CHAR:
				estructuraDestino = deserializeStruct_char(dataPaquete, length);
				break;
			case D_STRUCT_STRING:
				estructuraDestino = deserializeStruct_string(dataPaquete, length);
				break;
			case D_STRUCT_MALC:
				estructuraDestino = deserializeStruct_malc(dataPaquete, length);
				break;
			case D_STRUCT_PROG:
				estructuraDestino = deserializeStruct_prog(dataPaquete, length);
				break;
			case D_STRUCT_IMPR:
				estructuraDestino = deserializeStruct_impr(dataPaquete, length);
				break;
			case D_STRUCT_FIN_PROG:
				estructuraDestino = deserializeStruct_finProg(dataPaquete, length);
				break;
			case D_STRUCT_PCB:
				estructuraDestino = deserializeStruct_pcb(dataPaquete, length);
				break;
			case D_STRUCT_PID:
				estructuraDestino = deserializeStruct_pid(dataPaquete, length);
				break;
			case D_STRUCT_LECT:
				estructuraDestino = deserializeStruct_lect(dataPaquete, length);
				break;
			case D_STRUCT_LECT_VAR:
				estructuraDestino = deserializeStruct_lectvar(dataPaquete, length);
				break;
			case D_STRUCT_ABORT:
				estructuraDestino = deserializeStruct_abort(dataPaquete, length);
				break;
			case D_STRUCT_SIGUSR1:
				estructuraDestino = deserializeStruct_abort(dataPaquete, length);
				break;
			case D_STRUCT_SOL_ESCR:
				estructuraDestino = deserializeStruct_solEscr(dataPaquete, length);
				break;
			case D_STRUCT_PCB_FINOK:
				estructuraDestino = deserializeStruct_pcb_finOk(dataPaquete, length);
				break;
			case D_STRUCT_WAIT:
				estructuraDestino = deserializeStruct_wait(dataPaquete, length);
				break;
			case D_STRUCT_OBTENER_COMPARTIDA:
				estructuraDestino = deserializeStruct_obtComp(dataPaquete, length);
				break;
	}

	return estructuraDestino;
}

t_struct_numero * deserializeStruct_numero(char * dataPaquete, uint16_t length){
	t_struct_numero * estructuraDestino = malloc(sizeof(t_struct_numero));

	memcpy(estructuraDestino, dataPaquete, sizeof(int32_t)); //copio el data del paquete a la estructura.

	return estructuraDestino;
}

t_struct_char * deserializeStruct_char(char * dataPaquete, uint16_t length){
	t_struct_char * estructuraDestino = malloc(sizeof(t_struct_char));

	memcpy(&estructuraDestino->letra, dataPaquete, sizeof(char)); //copio la letra a la estructura

	return estructuraDestino;
}

t_struct_string * deserializeStruct_string(char * dataPaquete, uint16_t length){
	t_struct_string * estructuraDestino = malloc(sizeof(t_struct_string));

	int tamanoTotal = 0, tamanoDato = 0;

	tamanoTotal = tamanoDato;

	for(tamanoDato = 1; (dataPaquete + tamanoTotal)[tamanoDato -1] != '\0';tamanoDato++); 	//incremento tamanoDato, hasta el tamaño del nombre.

	estructuraDestino->string = malloc(tamanoDato);
	memcpy(estructuraDestino->string, dataPaquete + tamanoTotal, tamanoDato); //copio el string a la estructura

	return estructuraDestino;
}

t_struct_malloc * deserializeStruct_malc(char * dataPaquete, uint16_t length){
	t_struct_malloc * estructuraDestino = malloc(sizeof(t_struct_malloc));

	memcpy(estructuraDestino, dataPaquete, 2*sizeof(uint32_t)); //copio el data del paquete a la estructura.

	return estructuraDestino;
}

t_struct_programa * deserializeStruct_prog(char* dataPaquete, uint16_t length){
	t_struct_programa* estructuraDestino = malloc(sizeof(t_struct_programa));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->PID,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->base,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->tamanio,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	tamanoDato=estructuraDestino->tamanio;

	estructuraDestino->buffer= malloc(estructuraDestino->tamanio);
	memcpy(estructuraDestino->buffer, dataPaquete + tamanoTotal, tamanoDato);

	return estructuraDestino;
}

t_struct_string * deserializeStruct_impr(char * dataPaquete, uint16_t length){
	t_struct_string * estructuraDestino = malloc(sizeof(t_struct_string));

	int tamanoTotal = 0, tamanoDato = 0;

	tamanoTotal = tamanoDato;

	for(tamanoDato = 1; (dataPaquete + tamanoTotal)[tamanoDato -1] != '\0';tamanoDato++); 	//incremento tamanoDato, hasta el tamaño del nombre.

	estructuraDestino->string = malloc(tamanoDato);
	memcpy(estructuraDestino->string, dataPaquete + tamanoTotal, tamanoDato); //copio el string a la estructura

	return estructuraDestino;
}

t_struct_numero * deserializeStruct_finProg(char * dataPaquete, uint16_t length){
	t_struct_numero * estructuraDestino = malloc(sizeof(t_struct_numero));

	memcpy(estructuraDestino, dataPaquete, sizeof(int32_t)); //copio el data del paquete a la estructura.

	return estructuraDestino;
}

t_struct_pcb * deserializeStruct_pcb(char* dataPaquete, uint16_t length){

	t_struct_pcb* estructuraDestino = malloc(sizeof(t_struct_pcb));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->PID,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->cantRegistrosStack,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->cantidadInstrucciones,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->tamanioIndiceEtiquetas,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->cpuID,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->exitcode,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	estructuraDestino->indiceCodigo = malloc(sizeof(t_intructions)*(estructuraDestino->cantidadInstrucciones));

	int contadorInstrucciones = 0;

	while(contadorInstrucciones < estructuraDestino->cantidadInstrucciones){

		t_puntero_instruccion startInstruccion = 0;
		t_size offsetInstruccion = 0;

		memcpy(&startInstruccion,dataPaquete+tamanoTotal,tamanoDato=sizeof(t_puntero_instruccion));
		tamanoTotal+= tamanoDato;

		memcpy(&offsetInstruccion,dataPaquete+tamanoTotal,tamanoDato=sizeof(t_size));
		tamanoTotal+= tamanoDato;


		(estructuraDestino->indiceCodigo)[contadorInstrucciones] = cargarIndiceCodigo(startInstruccion, offsetInstruccion);

		contadorInstrucciones++;
	}

	estructuraDestino->indiceEtiquetas = malloc(estructuraDestino->tamanioIndiceEtiquetas);

	memcpy(estructuraDestino->indiceEtiquetas,dataPaquete+tamanoTotal,tamanoDato= estructuraDestino->tamanioIndiceEtiquetas);

	tamanoTotal+= tamanoDato;

	estructuraDestino->indiceStack = list_create();

	int contadorRegistrosStack = 0;

	while(contadorRegistrosStack < estructuraDestino->cantRegistrosStack){

		registroStack * registro = malloc(sizeof(registroStack));

		memcpy(&registro->cantidad_args,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
		tamanoTotal+= tamanoDato;

		registro->args = list_create();

		int contadorArgumentos = 0;

		while(contadorArgumentos < registro->cantidad_args){

			t_variable * arg = malloc(sizeof(t_variable));

			memcpy(&arg->identificador,dataPaquete+tamanoTotal,tamanoDato=sizeof(char));
			tamanoTotal+= tamanoDato;

			memcpy(&arg->posicionMemoria,dataPaquete+tamanoTotal,tamanoDato=sizeof(t_posicion_memoria));
			tamanoTotal+= tamanoDato;

			list_add(registro->args,arg);

			contadorArgumentos++;

		}

		memcpy(&registro->args->elements_count,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
		tamanoTotal+= tamanoDato;

		memcpy(&registro->cantidad_vars,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
		tamanoTotal+= tamanoDato;

		registro->vars = list_create();

		int contadorVariables = 0;

		while(contadorVariables < registro->cantidad_vars){

			t_variable * var = malloc(sizeof(t_variable));

			memcpy(&var->identificador,dataPaquete+tamanoTotal,tamanoDato=sizeof(char));
			tamanoTotal+= tamanoDato;

			memcpy(&var->posicionMemoria,dataPaquete+tamanoTotal,tamanoDato=sizeof(t_posicion_memoria));
			tamanoTotal+= tamanoDato;

			list_add(registro->vars, var);

			contadorVariables++;

		}

		memcpy(&registro->vars->elements_count,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
		tamanoTotal+= tamanoDato;

		memcpy(&registro->retPos,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
		tamanoTotal+= tamanoDato;

		memcpy(&registro->retVar,dataPaquete+tamanoTotal,tamanoDato=sizeof(t_posicion_memoria));
		tamanoTotal+= tamanoDato;

		list_add(estructuraDestino->indiceStack,registro);

		contadorRegistrosStack++;

	}

	memcpy(&estructuraDestino->indiceStack->elements_count,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->paginaActualStack,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->paginasCodigo,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->paginasStack,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->primerPaginaStack,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->programCounter,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->stackPointer,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_numero * deserializeStruct_pid(char * dataPaquete, uint16_t length){
	t_struct_numero * estructuraDestino = malloc(sizeof(t_struct_numero));

	memcpy(estructuraDestino, dataPaquete, sizeof(int32_t)); //copio el data del paquete a la estructura.

	return estructuraDestino;
}

t_posicion_memoria * deserializeStruct_lect(char* dataPaquete, uint16_t length){
	t_posicion_memoria* estructuraDestino = malloc(sizeof(t_posicion_memoria));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->pagina,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->offsetInstruccion,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->longitudInstruccion,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_posicion_memoria * deserializeStruct_lectvar(char* dataPaquete, uint16_t length){
	t_posicion_memoria* estructuraDestino = malloc(sizeof(t_posicion_memoria));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->pagina,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->offsetInstruccion,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->longitudInstruccion,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_numero * deserializeStruct_abort(char * dataPaquete, uint16_t length){
	t_struct_numero * estructuraDestino = malloc(sizeof(t_struct_numero));

	memcpy(estructuraDestino, dataPaquete, sizeof(int32_t)); //copio el data del paquete a la estructura.

	return estructuraDestino;
}

t_struct_numero * deserializeStruct_sigusr1(char * dataPaquete, uint16_t length){
	t_struct_numero * estructuraDestino = malloc(sizeof(t_struct_numero));

	memcpy(estructuraDestino, dataPaquete, sizeof(int32_t)); //copio el data del paquete a la estructura.

	return estructuraDestino;
}

t_struct_sol_escritura * deserializeStruct_solEscr(char* dataPaquete, uint16_t length){
	t_struct_sol_escritura* estructuraDestino = malloc(sizeof(t_struct_sol_escritura));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->pagina,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->offset,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->contenido,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_pcb * deserializeStruct_pcb_finOk(char* dataPaquete, uint16_t length){

	t_struct_pcb* estructuraDestino = malloc(sizeof(t_struct_pcb));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->PID,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->cantRegistrosStack,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->cantidadInstrucciones,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->tamanioIndiceEtiquetas,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->cpuID,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->exitcode,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	estructuraDestino->indiceCodigo = malloc(sizeof(t_intructions)*(estructuraDestino->cantidadInstrucciones));

	int contadorInstrucciones = 0;

	while(contadorInstrucciones < estructuraDestino->cantidadInstrucciones){

		t_puntero_instruccion startInstruccion = 0;
		t_size offsetInstruccion = 0;

		memcpy(&startInstruccion,dataPaquete+tamanoTotal,tamanoDato=sizeof(t_puntero_instruccion));
		tamanoTotal+= tamanoDato;

		memcpy(&offsetInstruccion,dataPaquete+tamanoTotal,tamanoDato=sizeof(t_size));
		tamanoTotal+= tamanoDato;


		(estructuraDestino->indiceCodigo)[contadorInstrucciones] = cargarIndiceCodigo(startInstruccion, offsetInstruccion);

		contadorInstrucciones++;
	}

	estructuraDestino->indiceEtiquetas = malloc(estructuraDestino->tamanioIndiceEtiquetas);

	memcpy(estructuraDestino->indiceEtiquetas,dataPaquete+tamanoTotal,tamanoDato= estructuraDestino->tamanioIndiceEtiquetas);

	tamanoTotal+= tamanoDato;

	estructuraDestino->indiceStack = list_create();

	int contadorRegistrosStack = 0;

	while(contadorRegistrosStack < estructuraDestino->cantRegistrosStack){

		registroStack * registro = malloc(sizeof(registroStack));

		memcpy(&registro->cantidad_args,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
		tamanoTotal+= tamanoDato;

		registro->args = list_create();

		int contadorArgumentos = 0;

		while(contadorArgumentos < registro->cantidad_args){

			t_variable * arg = malloc(sizeof(t_variable));

			memcpy(&arg->identificador,dataPaquete+tamanoTotal,tamanoDato=sizeof(char));
			tamanoTotal+= tamanoDato;

			memcpy(&arg->posicionMemoria,dataPaquete+tamanoTotal,tamanoDato=sizeof(t_posicion_memoria));
			tamanoTotal+= tamanoDato;

			list_add(registro->args,arg);

			contadorArgumentos++;

		}

		memcpy(&registro->args->elements_count,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
		tamanoTotal+= tamanoDato;

		memcpy(&registro->cantidad_vars,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
		tamanoTotal+= tamanoDato;

		registro->vars = list_create();

		int contadorVariables = 0;

		while(contadorVariables < registro->cantidad_vars){

			t_variable * var = malloc(sizeof(t_variable));

			memcpy(&var->identificador,dataPaquete+tamanoTotal,tamanoDato=sizeof(char));
			tamanoTotal+= tamanoDato;

			memcpy(&var->posicionMemoria,dataPaquete+tamanoTotal,tamanoDato=sizeof(t_posicion_memoria));
			tamanoTotal+= tamanoDato;

			list_add(registro->vars, var);

			contadorVariables++;

		}

		memcpy(&registro->vars->elements_count,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
		tamanoTotal+= tamanoDato;

		memcpy(&registro->retPos,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
		tamanoTotal+= tamanoDato;

		memcpy(&registro->retVar,dataPaquete+tamanoTotal,tamanoDato=sizeof(t_posicion_memoria));
		tamanoTotal+= tamanoDato;

		list_add(estructuraDestino->indiceStack,registro);

		contadorRegistrosStack++;

	}

	memcpy(&estructuraDestino->indiceStack->elements_count,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->paginaActualStack,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->paginasCodigo,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->paginasStack,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->primerPaginaStack,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->programCounter,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->stackPointer,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_string * deserializeStruct_wait(char * dataPaquete, uint16_t length){
	t_struct_string * estructuraDestino = malloc(sizeof(t_struct_string));

	int tamanoTotal = 0, tamanoDato = 0;

	tamanoTotal = tamanoDato;

	for(tamanoDato = 1; (dataPaquete + tamanoTotal)[tamanoDato -1] != '\0';tamanoDato++); 	//incremento tamanoDato, hasta el tamaño del nombre.

	estructuraDestino->string = malloc(tamanoDato);
	memcpy(estructuraDestino->string, dataPaquete + tamanoTotal, tamanoDato); //copio el string a la estructura

	return estructuraDestino;
}

t_struct_string * deserializeStruct_obtComp(char * dataPaquete, uint16_t length){
	t_struct_string * estructuraDestino = malloc(sizeof(t_struct_string));

	int tamanoTotal = 0, tamanoDato = 0;

	tamanoTotal = tamanoDato;

	for(tamanoDato = 1; (dataPaquete + tamanoTotal)[tamanoDato -1] != '\0';tamanoDato++); 	//incremento tamanoDato, hasta el tamaño del nombre.

	estructuraDestino->string = malloc(tamanoDato);
	memcpy(estructuraDestino->string, dataPaquete + tamanoTotal, tamanoDato); //copio el string a la estructura

	return estructuraDestino;
}
