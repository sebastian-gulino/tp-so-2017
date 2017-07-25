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

t_stream * serialize(int tipoEstructura, void * estructuraOrigen){
	t_stream * paquete=NULL;

	switch (tipoEstructura){
			case D_STRUCT_NUMERO:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen, D_STRUCT_NUMERO);
				break;
			case D_STRUCT_CHAR:
				paquete = serializeStruct_char((t_struct_char *) estructuraOrigen, D_STRUCT_CHAR);
				break;
			case D_STRUCT_STRING:
				paquete = serializeStruct_string((t_struct_string *) estructuraOrigen, D_STRUCT_STRING);
				break;
			case D_STRUCT_MALC:
				paquete = serializeStruct_malc((t_struct_malloc *) estructuraOrigen);
				break;
			case D_STRUCT_PROG:
				paquete = serializeStruct_prog((t_struct_programa *) estructuraOrigen, D_STRUCT_PROG);
				break;
			case D_STRUCT_CODIGO:
				paquete = serializeStruct_prog((t_struct_programa *) estructuraOrigen, D_STRUCT_CODIGO);
				break;
			case D_STRUCT_IMPR:
				paquete = serializeStruct_string((t_struct_string *) estructuraOrigen, D_STRUCT_IMPR);
				break;
			case D_STRUCT_FIN_PROG:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen, D_STRUCT_FIN_PROG);
				break;
			case D_STRUCT_ABORTAR_EJECUCION:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen, D_STRUCT_ABORTAR_EJECUCION);
				break;
			case D_STRUCT_CONTINUAR_EJECUCION:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen, D_STRUCT_CONTINUAR_EJECUCION);
				break;
			case D_STRUCT_FIN_QUANTUM:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen, D_STRUCT_FIN_QUANTUM);
				break;
			case D_STRUCT_SOLICITAR_CODIGO:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen, D_STRUCT_SOLICITAR_CODIGO);
				break;
			case D_STRUCT_PCB:
				paquete = serializeStruct_pcb((t_struct_pcb *) estructuraOrigen, D_STRUCT_PCB);
				break;
			case D_STRUCT_PCB_FIN_ERROR:
				paquete = serializeStruct_pcb((t_struct_pcb *) estructuraOrigen, D_STRUCT_PCB_FIN_ERROR);
				break;
			case D_STRUCT_FIN_PCB:
				paquete = serializeStruct_pcb((t_struct_pcb *) estructuraOrigen, D_STRUCT_FIN_PCB);
				break;
			case D_STRUCT_ERROR_WAIT:
				paquete = serializeStruct_pcb((t_struct_pcb *) estructuraOrigen, D_STRUCT_ERROR_WAIT);
				break;
			case D_STRUCT_ERROR_SEM:
				paquete = serializeStruct_pcb((t_struct_pcb *) estructuraOrigen, D_STRUCT_ERROR_SEM);
				break;
			case D_STRUCT_PID:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen, D_STRUCT_PID);
				break;
			case D_STRUCT_LIBERAR_MEMORIA:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen, D_STRUCT_LIBERAR_MEMORIA);
				break;
			case D_STRUCT_LECT:
				paquete = serializeStruct_solLect((t_struct_sol_lectura *) estructuraOrigen, D_STRUCT_LECT);
				break;
			case D_STRUCT_LECT_VAR:
				paquete = serializeStruct_solLect((t_struct_sol_lectura *) estructuraOrigen, D_STRUCT_LECT_VAR);
				break;
			case D_STRUCT_ESCRIBIR_HEAP:
				paquete = serializeStruct_solLect((t_struct_sol_lectura *) estructuraOrigen, D_STRUCT_ESCRIBIR_HEAP);
				break;
			case D_STRUCT_LIBERAR_HEAP:
				paquete = serializeStruct_solLect((t_struct_sol_lectura *) estructuraOrigen, D_STRUCT_LIBERAR_HEAP);
				break;
			case D_STRUCT_ABORT:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen, D_STRUCT_ABORT);
				break;
			case D_STRUCT_SIGUSR1:
				paquete = serializeStruct_pcb((t_struct_pcb *) estructuraOrigen, D_STRUCT_SIGUSR1);
				break;
			case D_STRUCT_FIN_INSTRUCCION:
				paquete = serializeStruct_pcb((t_struct_pcb *) estructuraOrigen, D_STRUCT_FIN_INSTRUCCION);
				break;
			case D_STRUCT_SOL_ESCR:
				paquete = serializeStruct_solEscr((t_struct_sol_escritura *) estructuraOrigen, D_STRUCT_SOL_ESCR);
				break;
			case D_STRUCT_ESCRITURA_CODIGO:
				paquete = serializeStruct_solEscr((t_struct_sol_escritura *) estructuraOrigen, D_STRUCT_ESCRITURA_CODIGO);
				break;
			case D_STRUCT_COMPACTAR_HEAP:
				paquete = serializeStruct_solEscr((t_struct_sol_escritura *) estructuraOrigen, D_STRUCT_COMPACTAR_HEAP);
				break;
			case D_STRUCT_PCB_FIN_OK:
				paquete = serializeStruct_pcb((t_struct_pcb *) estructuraOrigen, D_STRUCT_PCB_FIN_OK);
				break;
			case D_STRUCT_WAIT:
				paquete = serializeStruct_string((t_struct_string *) estructuraOrigen, D_STRUCT_WAIT);
				break;
			case D_STRUCT_SIGNAL:
				paquete = serializeStruct_string((t_struct_string *) estructuraOrigen, D_STRUCT_SIGNAL);
				break;
			case D_STRUCT_OBTENER_COMPARTIDA:
				paquete = serializeStruct_string((t_struct_string *) estructuraOrigen, D_STRUCT_OBTENER_COMPARTIDA);
				break;
			case D_STRUCT_GRABAR_COMPARTIDA:
				paquete = serializeStruct_graComp((t_struct_var_compartida *) estructuraOrigen);
				break;
			case D_STRUCT_ARCHIVO_ESC:
				paquete = serializeStruct_archivo_esc((t_struct_archivo *) estructuraOrigen, D_STRUCT_ARCHIVO_ESC);
				break;
			case D_STRUCT_ARCHIVO_LEC:
				paquete = serializeStruct_archivo_esc((t_struct_archivo *) estructuraOrigen, D_STRUCT_ARCHIVO_LEC);
				break;
			case D_STRUCT_SOL_HEAP:
				paquete = serializeStruct_solHeap((t_struct_sol_heap *) estructuraOrigen);
				break;
			case D_STRUCT_RTA_HEAP:
				paquete = serializeStruct_numero((t_struct_numero *) estructuraOrigen, D_STRUCT_RTA_HEAP);
				break;
			case D_STRUCT_LIB_HEAP:
				paquete = serializeStruct_libHeap((t_struct_sol_heap *) estructuraOrigen, D_STRUCT_LIB_HEAP);
				break;
			case D_STRUCT_LIBERAR_PAGINA:
				paquete = serializeStruct_libHeap((t_struct_sol_heap *) estructuraOrigen, D_STRUCT_LIBERAR_PAGINA);
				break;
			case D_STRUCT_ARCHIVO_ABR:
				paquete = serializeStruct_archivo_esc((t_struct_archivo *) estructuraOrigen, D_STRUCT_ARCHIVO_ABR);
				break;
			case D_STRUCT_ARCHIVO_BOR:
				paquete = serializeStruct_archivo_esc((t_struct_archivo *) estructuraOrigen, D_STRUCT_ARCHIVO_BOR);
				break;
			case D_STRUCT_ARCHIVO_CER:
				paquete = serializeStruct_archivo_esc((t_struct_archivo *) estructuraOrigen, D_STRUCT_ARCHIVO_CER);
				break;
			case D_STRUCT_ARCHIVO_MOV:
				paquete = serializeStruct_archivo_esc((t_struct_archivo *) estructuraOrigen, D_STRUCT_ARCHIVO_MOV);
				break;
			case D_STRUCT_BORRAR:
				paquete = serializeStruct_borrar((t_struct_borrar *) estructuraOrigen);
				break;
			case D_STRUCT_ABRIR:
				paquete = serializeStruct_abrir((t_struct_abrir *) estructuraOrigen);
				break;
			case D_STRUCT_OBTENER:
				paquete = serializeStruct_obtener((t_struct_obtener *) estructuraOrigen);
				break;
			case D_STRUCT_GUARDAR:
				paquete = serializeStruct_guardar((t_struct_guardar *) estructuraOrigen);
				break;
			case D_STRUCT_METADATA_HEAP:
				paquete = serializeStruct_metadataHeap((t_struct_metadataHeap *) estructuraOrigen, D_STRUCT_METADATA_HEAP);
				break;

		}

	return paquete;
}

t_stream * serializeStruct_numero(t_struct_numero * estructuraOrigen, int headerOperacion){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + sizeof(int32_t);

	char * data = crearDataConHeader(headerOperacion, paquete->length); //creo el data

	memcpy(data + sizeof(t_header), estructuraOrigen, sizeof(t_struct_numero));		//copio a data el numero.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_char(t_struct_char * estructuraOrigen, int headerOperacion){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + sizeof(unsigned int);

	char * data = crearDataConHeader(headerOperacion, paquete->length); //creo el data

	memcpy(data + sizeof(t_header), &estructuraOrigen->letra, sizeof(char));		//copio a data el char.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_string(t_struct_string * estructuraOrigen, int headerOperacion){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + strlen(estructuraOrigen->string) + 1;

	char * data = crearDataConHeader(headerOperacion, paquete->length); //creo el data

	int tamanoTotal = sizeof(t_header);

	memcpy(data + tamanoTotal, estructuraOrigen->string, strlen(estructuraOrigen->string)+1);		//copio a data el string.

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_malc(t_struct_malloc * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + 2*sizeof(uint32_t);

	char * data = crearDataConHeader(D_STRUCT_MALC, paquete->length); //creo el data

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->PID, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->tamano_segmento, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;
}

t_stream* serializeStruct_prog(t_struct_programa * estructuraOrigen, int headerOperacion){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) + sizeof(uint32_t) +sizeof(uint32_t) + estructuraOrigen->tamanio;

	char* data = crearDataConHeader(headerOperacion, paquete->length);

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

int calcularTamanioIndiceStack(t_struct_pcb * pcb){

	int tamanio = 0;
	int indice;
	registroStack * registro = malloc(sizeof(registroStack));

	for(indice=0;indice<list_size(pcb->indiceStack);indice++){

		registro = list_get(pcb->indiceStack,indice);

		int tamanioFijo = (3 * sizeof(uint32_t)) + sizeof(t_posicion_memoria);

		int tamanioVariable = (registro->args->elements_count * sizeof(t_posicion_memoria)) +
				(registro->vars->elements_count * (sizeof(char) + sizeof(t_posicion_memoria)));

		int tamanioRegistro = tamanioFijo + tamanioVariable;

		tamanio += tamanioRegistro;

	}

	return tamanio;

}

t_stream * serializeStruct_pcb(t_struct_pcb * estructuraOrigen, int headerOperacion){

	t_stream* paquete = malloc(sizeof(t_stream));

	uint32_t tamanioStack = calcularTamanioIndiceStack(estructuraOrigen);
	uint32_t tamanioCodigo = sizeof(t_intructions) * (estructuraOrigen->cantidadInstrucciones);

	paquete->length = sizeof(t_header) + 15*sizeof(uint32_t)+ estructuraOrigen->tamanioIndiceEtiquetas
			+ tamanioCodigo	+ tamanioStack;

	char* data = crearDataConHeader(headerOperacion, paquete->length);

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

	memcpy(data + tamTot , &estructuraOrigen->estado, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->exitcode, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	memcpy(data + tamTot , estructuraOrigen->indiceEtiquetas, tamDato = estructuraOrigen->tamanioIndiceEtiquetas);
	tamTot += tamDato;

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

	memcpy(data + tamTot , &estructuraOrigen->quantum_sleep, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->retornoPCB, tamDato = sizeof(uint32_t));
	tamTot += tamDato;

	memcpy(data + tamTot , &estructuraOrigen->stackPointer, tamDato = sizeof(int));
	tamTot += tamDato;

	int contadorInstrucciones = 0;

	while (contadorInstrucciones < estructuraOrigen->cantidadInstrucciones){

		t_intructions * instruccion = (t_intructions*) list_get(estructuraOrigen->indiceCodigo, contadorInstrucciones);

		memcpy(data + tamTot, &instruccion->start, tamDato= sizeof(t_puntero_instruccion));
		tamTot += tamDato;

		memcpy(data + tamTot, &instruccion->offset, tamDato= sizeof(t_size));
		tamTot += tamDato;

		contadorInstrucciones++;
	}

	int contadorStack = 0;
	int contadorArgs;
	int contadorVars;

	while (contadorStack < estructuraOrigen->cantRegistrosStack) {

		registroStack * registro = list_get(estructuraOrigen->indiceStack, contadorStack);

		uint32_t cantArgs = list_size(registro->args);

		memcpy(data + tamTot, &cantArgs, tamDato = sizeof(uint32_t));
		tamTot += tamDato;

		for(contadorArgs=0; contadorArgs < cantArgs; contadorArgs++){
			t_posicion_memoria * argumento = list_get(registro->args, contadorArgs);

			memcpy(data + tamTot, &argumento->pagina, tamDato= sizeof(uint32_t));
			tamTot+=tamDato;

			memcpy(data + tamTot, &argumento->offsetInstruccion, tamDato= sizeof(uint32_t));
			tamTot+=tamDato;

			memcpy(data + tamTot, &argumento->longitudInstruccion, tamDato= sizeof(uint32_t));
			tamTot+=tamDato;

		}

		uint32_t cantVars = list_size(registro->vars);

		memcpy(data + tamTot, &cantVars, tamDato = sizeof(uint32_t));
		tamTot += tamDato;

		for(contadorVars=0; contadorVars < cantVars; contadorVars++){
			t_variable * variable = list_get(registro->vars, contadorVars);

			memcpy(data + tamTot, &variable->identificador, tamDato= sizeof(char));
			tamTot+=tamDato;

			memcpy(data + tamTot, &variable->posicionMemoria.pagina, tamDato= sizeof(uint32_t));
			tamTot+=tamDato;

			memcpy(data + tamTot, &variable->posicionMemoria.offsetInstruccion, tamDato= sizeof(uint32_t));
			tamTot+=tamDato;

			memcpy(data + tamTot, &variable->posicionMemoria.longitudInstruccion, tamDato= sizeof(uint32_t));
			tamTot+=tamDato;

		}

		memcpy(data + tamTot , &registro->retPos, tamDato = sizeof(int));
		tamTot += tamDato;

		memcpy(data + tamTot, &registro->retVar.pagina, tamDato= sizeof(uint32_t));
		tamTot+=tamDato;

		memcpy(data + tamTot, &registro->retVar.offsetInstruccion, tamDato= sizeof(uint32_t));
		tamTot+=tamDato;

		memcpy(data + tamTot, &registro->retVar.longitudInstruccion, tamDato= sizeof(uint32_t));
		tamTot+=tamDato;

		contadorStack++;

	}

	paquete->data = data;

	return paquete;
}

t_stream * serializeStruct_lect(t_posicion_memoria * estructuraOrigen, int headerOperacion){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) + sizeof(uint32_t) +sizeof(uint32_t);

	char* data = crearDataConHeader(headerOperacion, paquete->length);

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

t_stream * serializeStruct_solEscr(t_struct_sol_escritura * estructuraOrigen, int headerOperacion){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

	char* data = crearDataConHeader(headerOperacion, paquete->length);

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->pagina, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->offset, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->contenido, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->PID, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;

}

t_stream * serializeStruct_solLect(t_struct_sol_lectura * estructuraOrigen, int headerOperacion){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);

	char* data = crearDataConHeader(headerOperacion, paquete->length);

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->pagina, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->offset, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->contenido, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->PID, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;

}

t_stream * serializeStruct_metadataHeap(t_struct_metadataHeap * estructuraOrigen, int headerOperacion){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(bool) + sizeof(int32_t);

	char* data = crearDataConHeader(headerOperacion, paquete->length);

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->isFree, tamanoDato= sizeof(bool));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->size, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;

}

t_stream * serializeStruct_graComp(t_struct_var_compartida * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

	paquete->length = sizeof(t_header) + strlen(estructuraOrigen->nombre) + 1 + sizeof(uint32_t);

	char * data = crearDataConHeader(D_STRUCT_GRABAR_COMPARTIDA, paquete->length); //creo el data

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, estructuraOrigen->nombre, tamanoDato = strlen(estructuraOrigen->nombre)+1);		//copio a data el string.

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->valor, tamanoDato = sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;
}

t_stream* serializeStruct_archivo_esc(t_struct_archivo * estructuraOrigen, int headerOperacion){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) + estructuraOrigen->tamanio + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(t_flags) ;

	char* data = crearDataConHeader(headerOperacion, paquete->length);

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->fileDescriptor, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->tamanio, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, estructuraOrigen->informacion, tamanoDato = (estructuraOrigen->tamanio));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->pid, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->flags.creacion, tamanoDato= sizeof(bool));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->flags.escritura, tamanoDato= sizeof(bool));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->flags.lectura, tamanoDato= sizeof(bool));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;

}

t_stream * serializeStruct_solHeap(t_struct_sol_heap * estructuraOrigen){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) +sizeof(uint32_t);

	char* data = crearDataConHeader(D_STRUCT_SOL_HEAP, paquete->length);

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->pointer, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->pid, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;

}

t_stream * serializeStruct_libHeap(t_struct_sol_heap * estructuraOrigen, int headerOperacion){

	t_stream* paquete = malloc(sizeof(t_stream));

	paquete->length = sizeof(t_header) + sizeof(uint32_t) +sizeof(uint32_t);

	char* data = crearDataConHeader(headerOperacion, paquete->length);

	int tamanoTotal = sizeof(t_header), tamanoDato = 0;

	memcpy(data + tamanoTotal, &estructuraOrigen->pointer, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	memcpy(data + tamanoTotal, &estructuraOrigen->pid, tamanoDato= sizeof(uint32_t));

	tamanoTotal+=tamanoDato;

	paquete->data = data;

	return paquete;

}

t_stream * serializeStruct_borrar(t_struct_borrar * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

		paquete->length = sizeof(t_header) + strlen(estructuraOrigen->path) + 1 + sizeof(uint32_t);

		char * data = crearDataConHeader(D_STRUCT_BORRAR, paquete->length); //creo el data

		int tamanoTotal = sizeof(t_header), tamanoDato = 0;

		memcpy(data + tamanoTotal, estructuraOrigen->path, tamanoDato = strlen(estructuraOrigen->path)+1);		//copio a data el string.

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->confirmacion, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		paquete->data = data;

		return paquete;
}

t_stream * serializeStruct_abrir(t_struct_abrir * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

		paquete->length = sizeof(t_header) + strlen(estructuraOrigen->path) + 1 + sizeof(uint32_t) + sizeof(uint32_t);

		char * data = crearDataConHeader(D_STRUCT_ABRIR, paquete->length); //creo el data

		int tamanoTotal = sizeof(t_header), tamanoDato = 0;

		memcpy(data + tamanoTotal, estructuraOrigen->path, tamanoDato = strlen(estructuraOrigen->path)+1);		//copio a data el string.

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->confirmacion, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->modo_creacion, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		paquete->data = data;

		return paquete;
}

t_stream * serializeStruct_obtener(t_struct_obtener * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

		paquete->length = sizeof(t_header) + strlen(estructuraOrigen->path) + 1 + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + estructuraOrigen->size;

		char * data = crearDataConHeader(D_STRUCT_OBTENER, paquete->length); //creo el data

		int tamanoTotal = sizeof(t_header), tamanoDato = 0;

		memcpy(data + tamanoTotal, estructuraOrigen->path, tamanoDato = strlen(estructuraOrigen->path)+1);		//copio a data el string.

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->confirmacion, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->modo_lectura, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->offset, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->size, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->obtenido, tamanoDato = estructuraOrigen->size);

		tamanoTotal+=tamanoDato;

		paquete->data = data;

		return paquete;
}

t_stream * serializeStruct_guardar(t_struct_guardar * estructuraOrigen){

	t_stream * paquete = malloc(sizeof(t_stream));		//creo el paquete

		paquete->length = sizeof(t_header) + strlen(estructuraOrigen->path) + 1 + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + estructuraOrigen->size;

		char * data = crearDataConHeader(D_STRUCT_GUARDAR, paquete->length); //creo el data

		int tamanoTotal = sizeof(t_header), tamanoDato = 0;

		memcpy(data + tamanoTotal, estructuraOrigen->path, tamanoDato = strlen(estructuraOrigen->path)+1);		//copio a data el string.

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->confirmacion, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->modo_escritura, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->offset, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->size, tamanoDato = sizeof(uint32_t));

		tamanoTotal+=tamanoDato;

		memcpy(data + tamanoTotal, &estructuraOrigen->buffer, tamanoDato = estructuraOrigen->size);

		tamanoTotal+=tamanoDato;

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
			case D_STRUCT_CODIGO:
				estructuraDestino = deserializeStruct_prog(dataPaquete, length);
				break;
			case D_STRUCT_IMPR:
				estructuraDestino = deserializeStruct_string(dataPaquete, length);
				break;
			case D_STRUCT_FIN_PROG:
				estructuraDestino = deserializeStruct_numero(dataPaquete, length);
				break;
			case D_STRUCT_ABORTAR_EJECUCION:
				estructuraDestino = deserializeStruct_numero(dataPaquete, length);
				break;
			case D_STRUCT_CONTINUAR_EJECUCION:
				estructuraDestino = deserializeStruct_numero(dataPaquete, length);
				break;
			case D_STRUCT_FIN_QUANTUM:
				estructuraDestino = deserializeStruct_numero(dataPaquete, length);
				break;
			case D_STRUCT_SOLICITAR_CODIGO:
				estructuraDestino = deserializeStruct_numero(dataPaquete, length);
				break;
			case D_STRUCT_PCB:
				estructuraDestino = deserializeStruct_pcb(dataPaquete, length);
				break;
			case D_STRUCT_PCB_FIN_ERROR:
				estructuraDestino = deserializeStruct_pcb(dataPaquete, length);
				break;
			case D_STRUCT_FIN_PCB:
				estructuraDestino = deserializeStruct_pcb(dataPaquete, length);
				break;
			case D_STRUCT_ERROR_WAIT:
				estructuraDestino = deserializeStruct_pcb(dataPaquete, length);
				break;
			case D_STRUCT_ERROR_SEM:
				estructuraDestino = deserializeStruct_pcb(dataPaquete, length);
				break;
			case D_STRUCT_PID:
				estructuraDestino = deserializeStruct_numero(dataPaquete, length);
				break;
			case D_STRUCT_LIBERAR_MEMORIA:
				estructuraDestino = deserializeStruct_numero(dataPaquete, length);
				break;
			case D_STRUCT_LECT:
				estructuraDestino = deserializeStruct_solLect(dataPaquete, length);
				break;
			case D_STRUCT_LECT_VAR:
				estructuraDestino = deserializeStruct_solLect(dataPaquete, length);
				break;
			case D_STRUCT_ESCRIBIR_HEAP:
				estructuraDestino = deserializeStruct_solLect(dataPaquete, length);
				break;
			case D_STRUCT_LIBERAR_HEAP:
				estructuraDestino = deserializeStruct_solLect(dataPaquete, length);
				break;
			case D_STRUCT_COMPACTAR_HEAP:
				estructuraDestino = deserializeStruct_solLect(dataPaquete, length);
				break;
			case D_STRUCT_ABORT:
				estructuraDestino = deserializeStruct_numero(dataPaquete, length);
				break;
			case D_STRUCT_SIGUSR1:
				estructuraDestino = deserializeStruct_pcb(dataPaquete, length);
				break;
			case D_STRUCT_FIN_INSTRUCCION:
				estructuraDestino = deserializeStruct_pcb(dataPaquete, length);
				break;
			case D_STRUCT_SOL_ESCR:
				estructuraDestino = deserializeStruct_solEscr(dataPaquete, length);
				break;
			case D_STRUCT_ESCRITURA_CODIGO:
				estructuraDestino = deserializeStruct_solEscr(dataPaquete, length);
				break;
			case D_STRUCT_PCB_FIN_OK:
				estructuraDestino = deserializeStruct_pcb(dataPaquete, length);
				break;
			case D_STRUCT_WAIT:
				estructuraDestino = deserializeStruct_string(dataPaquete, length);
				break;
			case D_STRUCT_SIGNAL:
				estructuraDestino = deserializeStruct_string(dataPaquete, length);
				break;
			case D_STRUCT_OBTENER_COMPARTIDA:
				estructuraDestino = deserializeStruct_string(dataPaquete, length);
				break;
			case D_STRUCT_GRABAR_COMPARTIDA:
				estructuraDestino = deserializeStruct_graComp(dataPaquete, length);
				break;
			case D_STRUCT_ARCHIVO_ESC:
				estructuraDestino = deserializeStruct_archivo_esc(dataPaquete, length);
				break;
			case D_STRUCT_ARCHIVO_LEC:
				estructuraDestino = deserializeStruct_archivo_esc(dataPaquete, length);
				break;
			case D_STRUCT_SOL_HEAP:
				estructuraDestino = deserializeStruct_solHeap(dataPaquete, length);
				break;
			case D_STRUCT_RTA_HEAP:
				estructuraDestino = deserializeStruct_numero(dataPaquete, length);
				break;
			case D_STRUCT_LIB_HEAP:
				estructuraDestino = deserializeStruct_libHeap(dataPaquete, length);
				break;
			case D_STRUCT_LIBERAR_PAGINA:
				estructuraDestino = deserializeStruct_libHeap(dataPaquete, length);
				break;
			case D_STRUCT_ARCHIVO_ABR:
				estructuraDestino = deserializeStruct_archivo_esc(dataPaquete, length);
				break;
			case D_STRUCT_ARCHIVO_CER:
				estructuraDestino = deserializeStruct_archivo_esc(dataPaquete, length);
				break;
			case D_STRUCT_ARCHIVO_BOR:
				estructuraDestino = deserializeStruct_archivo_esc(dataPaquete, length);
				break;
			case D_STRUCT_ARCHIVO_MOV:
				estructuraDestino = deserializeStruct_archivo_esc(dataPaquete, length);
				break;
			case D_STRUCT_BORRAR:
				estructuraDestino = deserializeStruct_borrar(dataPaquete, length);
				break;
			case D_STRUCT_ABRIR:
				estructuraDestino = deserializeStruct_abrir(dataPaquete, length);
				break;
			case D_STRUCT_OBTENER:
				estructuraDestino = deserializeStruct_obtener(dataPaquete, length);
				break;
			case D_STRUCT_GUARDAR:
				estructuraDestino = deserializeStruct_guardar(dataPaquete, length);
				break;
			case D_STRUCT_METADATA_HEAP:
				estructuraDestino = deserializeStruct_metadataHeap(dataPaquete, length);
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

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->PID,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->tamano_segmento,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

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

t_struct_pcb * deserializeStruct_pcb(char* dataPaquete, uint16_t length){

	t_struct_pcb* estructuraDestino = malloc(length);

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

	memcpy(&estructuraDestino->estado,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->exitcode,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	if(estructuraDestino->tamanioIndiceEtiquetas>0){

	char * etiquetas = malloc(estructuraDestino->tamanioIndiceEtiquetas);

	memcpy(etiquetas,dataPaquete+tamanoTotal,tamanoDato= estructuraDestino->tamanioIndiceEtiquetas);

	estructuraDestino->indiceEtiquetas = etiquetas;

	tamanoTotal+= tamanoDato;

	}

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

	memcpy(&estructuraDestino->quantum_sleep,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->retornoPCB,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));
	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->stackPointer,dataPaquete+tamanoTotal,tamanoDato=sizeof(int));
	tamanoTotal+= tamanoDato;

	int contadorInstrucciones = 0;
	estructuraDestino->indiceCodigo = list_create();

	while(contadorInstrucciones < estructuraDestino->cantidadInstrucciones){

		t_intructions* instruccion = malloc(sizeof(t_intructions));

		memcpy(&instruccion->start, dataPaquete + tamanoTotal, tamanoDato = sizeof(t_puntero_instruccion));
		tamanoTotal += tamanoDato;

		memcpy(&instruccion->offset, dataPaquete + tamanoTotal, tamanoDato = sizeof(t_size));
		tamanoTotal += tamanoDato;

		list_add(estructuraDestino->indiceCodigo,instruccion);

		contadorInstrucciones++;
	}

	int contadorStack = 0;
	int contadorArgs;
	int contadorVars;

	estructuraDestino->indiceStack = list_create();

	while(contadorStack < estructuraDestino->cantRegistrosStack){

		registroStack * registro = malloc(sizeof(registroStack));

		registro->args = list_create();

		int cantArgs = 0;

		memcpy(&cantArgs, dataPaquete + tamanoTotal, tamanoDato = sizeof(int));
		tamanoTotal += tamanoDato;

		for (contadorArgs=0; contadorArgs < cantArgs; contadorArgs++){

			t_posicion_memoria * argumento = malloc(sizeof(t_posicion_memoria));

			memcpy(&argumento->pagina, dataPaquete + tamanoTotal, tamanoDato = sizeof(uint32_t));
			tamanoTotal += tamanoDato;

			memcpy(&argumento->offsetInstruccion, dataPaquete + tamanoTotal, tamanoDato = sizeof(uint32_t));
			tamanoTotal += tamanoDato;

			memcpy(&argumento->longitudInstruccion, dataPaquete + tamanoTotal, tamanoDato = sizeof(uint32_t));
			tamanoTotal += tamanoDato;

			list_add(registro->args,argumento);

		}

		registro->vars = list_create();

		int cantVars = 0;

		memcpy(&cantVars, dataPaquete + tamanoTotal, tamanoDato = sizeof(int));
		tamanoTotal += tamanoDato;

		for (contadorVars=0; contadorVars < cantVars; contadorVars++){

			t_variable * variable = malloc(sizeof(t_variable));

			memcpy(&variable->identificador, dataPaquete + tamanoTotal, tamanoDato = sizeof(char));
			tamanoTotal += tamanoDato;

			memcpy(&variable->posicionMemoria.pagina, dataPaquete + tamanoTotal, tamanoDato = sizeof(uint32_t));
			tamanoTotal += tamanoDato;

			memcpy(&variable->posicionMemoria.offsetInstruccion, dataPaquete + tamanoTotal, tamanoDato = sizeof(uint32_t));
			tamanoTotal += tamanoDato;

			memcpy(&variable->posicionMemoria.longitudInstruccion, dataPaquete + tamanoTotal, tamanoDato = sizeof(uint32_t));
			tamanoTotal += tamanoDato;

			list_add(registro->vars, variable);

		}

		memcpy(&registro->retPos, dataPaquete + tamanoTotal, tamanoDato = sizeof(int));
		tamanoTotal += tamanoDato;

		memcpy(&registro->retVar.pagina, dataPaquete + tamanoTotal, tamanoDato = sizeof(uint32_t));
		tamanoTotal += tamanoDato;

		memcpy(&registro->retVar.offsetInstruccion, dataPaquete + tamanoTotal, tamanoDato = sizeof(uint32_t));
		tamanoTotal += tamanoDato;

		memcpy(&registro->retVar.longitudInstruccion, dataPaquete + tamanoTotal, tamanoDato = sizeof(uint32_t));
		tamanoTotal += tamanoDato;

		list_add(estructuraDestino->indiceStack,registro);
		contadorStack++;
	}

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

t_struct_sol_escritura * deserializeStruct_solEscr(char* dataPaquete, uint16_t length){
	t_struct_sol_escritura* estructuraDestino = malloc(sizeof(t_struct_sol_escritura));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->pagina,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->offset,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->contenido,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->PID,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_sol_lectura * deserializeStruct_solLect(char* dataPaquete, uint16_t length){
	t_struct_sol_lectura* estructuraDestino = malloc(sizeof(t_struct_sol_lectura));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->pagina,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->offset,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->contenido,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->PID,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_metadataHeap * deserializeStruct_metadataHeap(char* dataPaquete, uint16_t length){
	t_struct_metadataHeap* estructuraDestino = malloc(sizeof(t_struct_metadataHeap));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->isFree,dataPaquete+tamanoTotal,tamanoDato=sizeof(bool));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->size,dataPaquete+tamanoTotal,tamanoDato=sizeof(int32_t));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_var_compartida * deserializeStruct_graComp(char * dataPaquete, uint16_t length){
	t_struct_var_compartida * estructuraDestino = malloc(sizeof(t_struct_var_compartida));

	int tamanoTotal = 0, tamanoDato = 0;

	tamanoTotal = tamanoDato;

	for(tamanoDato = 1; (dataPaquete + tamanoTotal)[tamanoDato -1] != '\0';tamanoDato++); 	//incremento tamanoDato, hasta el tamaño del nombre.

	estructuraDestino->nombre = malloc(tamanoDato);
	memcpy(estructuraDestino->nombre, dataPaquete + tamanoTotal, tamanoDato); //copio el string a la estructura

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->valor,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_archivo * deserializeStruct_archivo_esc(char* dataPaquete, uint16_t length){
	t_struct_archivo* estructuraDestino = malloc(sizeof(t_struct_archivo));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->fileDescriptor,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->tamanio,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	estructuraDestino->informacion= malloc(estructuraDestino->tamanio);
	memcpy(estructuraDestino->informacion, dataPaquete + tamanoTotal, tamanoDato = estructuraDestino->tamanio);

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->pid,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->flags.creacion,dataPaquete+tamanoTotal,tamanoDato=sizeof(bool));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->flags.escritura,dataPaquete+tamanoTotal,tamanoDato=sizeof(bool));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->flags.lectura,dataPaquete+tamanoTotal,tamanoDato=sizeof(bool));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_sol_heap * deserializeStruct_solHeap(char* dataPaquete, uint16_t length){
	t_struct_sol_heap* estructuraDestino = malloc(sizeof(t_struct_sol_heap));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->pointer,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->pid,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_sol_heap * deserializeStruct_libHeap(char* dataPaquete, uint16_t length){
	t_struct_sol_heap* estructuraDestino = malloc(sizeof(t_struct_sol_heap));

	int tamanoDato = 0, tamanoTotal = 0;

	memcpy(&estructuraDestino->pointer,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->pid,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}


t_struct_borrar * deserializeStruct_borrar(char * dataPaquete, uint16_t length){

	t_struct_borrar * estructuraDestino = malloc(sizeof(t_struct_borrar));

	int tamanoTotal = 0, tamanoDato = 0;

	tamanoTotal = tamanoDato;

	for(tamanoDato = 1; (dataPaquete + tamanoTotal)[tamanoDato -1] != '\0';tamanoDato++); 	//incremento tamanoDato, hasta el tamaño del nombre.

	estructuraDestino->path = malloc(tamanoDato);
	memcpy(estructuraDestino->path, dataPaquete + tamanoTotal, tamanoDato); //copio el string a la estructura

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->confirmacion,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_abrir * deserializeStruct_abrir(char * dataPaquete, uint16_t length){

	t_struct_abrir * estructuraDestino = malloc(sizeof(t_struct_abrir));

	int tamanoTotal = 0, tamanoDato = 0;

	tamanoTotal = tamanoDato;

	for(tamanoDato = 1; (dataPaquete + tamanoTotal)[tamanoDato -1] != '\0';tamanoDato++); 	//incremento tamanoDato, hasta el tamaño del nombre.

	estructuraDestino->path = malloc(tamanoDato);
	memcpy(estructuraDestino->path, dataPaquete + tamanoTotal, tamanoDato); //copio el string a la estructura

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->confirmacion,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->modo_creacion,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

		tamanoTotal+= tamanoDato;

	return estructuraDestino;
}

t_struct_obtener * deserializeStruct_obtener(char * dataPaquete, uint16_t length){

	t_struct_obtener * estructuraDestino = malloc(sizeof(t_struct_obtener));

	int tamanoTotal = 0, tamanoDato = 0;

	tamanoTotal = tamanoDato;

	for(tamanoDato = 1; (dataPaquete + tamanoTotal)[tamanoDato -1] != '\0';tamanoDato++); 	//incremento tamanoDato, hasta el tamaño del nombre.

	estructuraDestino->path = malloc(tamanoDato);
	memcpy(estructuraDestino->path, dataPaquete + tamanoTotal, tamanoDato); //copio el string a la estructura

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->confirmacion,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->modo_lectura,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->offset,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->size,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	tamanoDato=estructuraDestino->size;

	estructuraDestino->obtenido= malloc(estructuraDestino->size);

	memcpy(estructuraDestino->obtenido, dataPaquete + tamanoTotal, tamanoDato);

	return estructuraDestino;
}

t_struct_guardar * deserializeStruct_guardar(char * dataPaquete, uint16_t length){

	t_struct_guardar * estructuraDestino = malloc(sizeof(t_struct_obtener));

	int tamanoTotal = 0, tamanoDato = 0;

	tamanoTotal = tamanoDato;

	for(tamanoDato = 1; (dataPaquete + tamanoTotal)[tamanoDato -1] != '\0';tamanoDato++); 	//incremento tamanoDato, hasta el tamaño del nombre.

	estructuraDestino->path = malloc(tamanoDato);
	memcpy(estructuraDestino->path, dataPaquete + tamanoTotal, tamanoDato); //copio el string a la estructura

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->confirmacion,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->modo_escritura,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->offset,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	memcpy(&estructuraDestino->size,dataPaquete+tamanoTotal,tamanoDato=sizeof(uint32_t));

	tamanoTotal+= tamanoDato;

	tamanoDato=estructuraDestino->size;

	estructuraDestino->buffer= malloc(estructuraDestino->size);

	memcpy(estructuraDestino->buffer, dataPaquete + tamanoTotal, tamanoDato);

	return estructuraDestino;
}
