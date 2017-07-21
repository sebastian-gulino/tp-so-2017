#include "primitivas.h"

//FUNCIONES
t_puntero definirVariable(t_nombre_variable variable) {

	log_info(logger,"Se ingresa a la primitiva Definir Variable para la variable: %c",variable);
	// Defino una nueva posición en el stack para la variable:
	int var_pagina = pcbEjecutando->primerPaginaStack;
	int var_offset = pcbEjecutando->stackPointer;

	while(var_offset > tamanio_pagina){
		(var_pagina)++;
		var_offset -= tamanio_pagina;
	}
	// Verifico si se desborda la pila en memoria:
	if(pcbEjecutando->stackPointer + 4 > (tamanio_pagina*tamanio_stack)){
			log_trace(logger, "Stack Overflow al definir variable '%c'.", variable);
			stackOverflow = true;

		return -1;
	}else{
		// Selecciono registro actual del índice de stack:
		registroStack* regStack = list_get(pcbEjecutando->indiceStack, pcbEjecutando->cantRegistrosStack -1);

		// Si no hay registros, creo uno nuevo
		if(regStack == NULL){
			regStack = reg_stack_create();
			// Guardo el nuevo registro en el índice:
			list_add(pcbEjecutando->indiceStack, regStack);
		}

		if(!esArgumento(variable)){ // agrego nueva variable
			t_variable* new_var = malloc(sizeof(t_variable));
			new_var->identificador = variable;
			new_var->posicionMemoria.pagina = var_pagina;
			new_var->posicionMemoria.offsetInstruccion = var_offset;
			new_var->posicionMemoria.longitudInstruccion = sizeof(int);

			list_add(regStack->vars, new_var);
		}
		else{ // agrego nuevo argumento
			t_variable* new_arg = malloc(sizeof(variable));
			new_arg->identificador = variable;
			new_arg->posicionMemoria.pagina = var_pagina;
			new_arg->posicionMemoria.offsetInstruccion = var_offset;
			new_arg->posicionMemoria.longitudInstruccion = sizeof(int);

			list_add(regStack->args, new_arg);
		}

		log_trace(logger, "'%c' -> Dirección lógica definida: %i, %i, %i.", variable, var_pagina, var_offset, sizeof(int));

		// Actualizo parámetros del PCB:
		int total_heap_offset = (pcbEjecutando->paginasCodigo * tamanio_pagina) + pcbEjecutando->stackPointer;
		pcbEjecutando->stackPointer += sizeof(int);
		pcbEjecutando->paginaActualStack = (total_heap_offset + sizeof(int)) / tamanio_pagina;

		return total_heap_offset;
	}
}

t_puntero obtenerPosicionVariable(t_nombre_variable idVariable) {

	log_info(logger,"Se ingresa a la primitiva Obtener Posicion Variable con la variable: %c", idVariable);

	/// Obtengo el registro del stack correspondiente al contexto de ejecución actual:
	registroStack* regStack = list_get(pcbEjecutando->indiceStack, pcbEjecutando->indiceStack->elements_count -1);

	// Busco en este registro la variable que coincida con el nombre solicitado:
	if(!esArgumento(idVariable)){
		if(regStack->vars->elements_count > 0){ // si hay variables en la lista:

			// Obtengo la variable buscada:
			int i;
			for(i = 0; i<regStack->vars->elements_count; i++){

				t_variable* variable = list_get(regStack->vars, i);
				if(variable->identificador == idVariable){

					int var_offset_absoluto = (variable->posicionMemoria.pagina * tamanio_pagina) + variable->posicionMemoria.offsetInstruccion;

					return var_offset_absoluto;
				}
			}
		}
		log_error(logger, "No hay variables en el registro actual de stack.");
		return -1;
	}
	else{
		if(regStack->args->elements_count > 0){

			// Obtengo el argumento buscado:
			int j;
			for(j = 0; j<regStack->args->elements_count; j++){

				t_variable* argumento = list_get(regStack->args, j);
				if(argumento->identificador == idVariable){

					int arg_offset_absoluto = (argumento->posicionMemoria.pagina * tamanio_pagina) + argumento->posicionMemoria.offsetInstruccion;

					return arg_offset_absoluto;
				}
			}
		}
		log_error(logger, "No hay argumentos en el registro actual de stack.");
		return -1;
	}
}

t_valor_variable dereferenciar(t_puntero total_heap_offset) {

	log_info(logger,"Se ingresa a la primitiva Dereferenciar con el puntero: %d", total_heap_offset);

	t_struct_sol_lectura * var_direccion = malloc(sizeof(t_posicion_memoria));

	var_direccion->pagina = total_heap_offset / tamanio_pagina;
	var_direccion->offset = total_heap_offset % tamanio_pagina;
	var_direccion->contenido = sizeof(int);
	var_direccion->PID = pcbEjecutando->PID;

	log_trace(logger, "Solicitud Lectura -> Página: %i, Offset: %i, Size: %i.",
			var_direccion->pagina, var_direccion->offset, sizeof(int));

	socket_enviar(socketMemoria,D_STRUCT_LECT_VAR,var_direccion);

	free(var_direccion);
	var_direccion = NULL;


	if(!validarPedidoMemoria()){
		log_error(logger, "La variable no pudo dereferenciarse.");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_MEMORIA;
		return -1;
	}
	else{ // no hubo error de lectura
		t_tipoEstructura tipoEstructura;
		void * structRecibido;

		if ( socket_recibir(socketMemoria, &tipoEstructura, &structRecibido) == -1){

			log_error(logger, "La memoria se desconecto del sistema");
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_MEMORIA;
			return NULL;

		} else {

			t_valor_variable valor = ((t_struct_numero*) structRecibido)->numero;

			log_trace(logger, "Variable dereferenciada -> Valor: %d.", valor);
			free(structRecibido);
			structRecibido = NULL;

			return valor;
		}

	}
}

void asignar(t_puntero total_heap_offset, t_valor_variable valor) {

	log_info(logger,"Se ingresa a la primitiva Asignar para el puntero %d con el valor %d", total_heap_offset,valor);

	t_struct_sol_escritura* var_escritura = malloc(sizeof(t_struct_sol_escritura));

	var_escritura->pagina = total_heap_offset / tamanio_pagina;
	var_escritura->offset = total_heap_offset % tamanio_pagina;
	var_escritura->contenido = valor;
	var_escritura->PID = pcbEjecutando->PID;

	log_trace(logger, "Solicitud Escritura -> Página: %i, Offset: %i, Contenido: %d.",
			var_escritura->pagina, var_escritura->offset, valor);

	socket_enviar(socketMemoria, D_STRUCT_SOL_ESCR, var_escritura);

	free(var_escritura);
	var_escritura = NULL;

	if(!validarPedidoMemoria()){
		log_error(logger, "La variable no pudo asignarse en memoria.");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_MEMORIA;
	}
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variableCompartida) {

	log_info(logger, "Se ingresa a la primitiva Obtener valor variable compartida para la variable: '%s'.", variableCompartida);

	t_struct_string* obtenerVarCompartida = malloc(sizeof(t_struct_string));

	strcpy(obtenerVarCompartida->string,"!\0");
	strcat(obtenerVarCompartida->string,variableCompartida);

	socket_enviar(socketKernel, D_STRUCT_OBTENER_COMPARTIDA, obtenerVarCompartida);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;
		return NULL;

	} else {

		// El kernel ante una solicitud de variable compartida debe retornarme el valor entero correspondiente.
		t_valor_variable valor = ((t_struct_numero*) structRecibido)->numero;

		log_trace(logger, "Variable Compartida: '%s' -> Valor: '%d'.", variableCompartida, valor);

		free(structRecibido);
		structRecibido = NULL;

		return valor;
	}

}

t_valor_variable asignarValorCompartida(t_nombre_compartida variableCompartida, t_valor_variable valor) {

	log_info(logger, "Se ingresa a la primitiva Asignar valor compartida para  '%d' a Variable Compartida '%s'.", valor, variableCompartida);
	t_struct_var_compartida * varCompartida = malloc(strlen(variableCompartida)+ 5);

	varCompartida->valor = valor;

	strcpy(varCompartida->nombre,"!\0");
	strcat(varCompartida->nombre,variableCompartida);

	socket_enviar(socketKernel,D_STRUCT_GRABAR_COMPARTIDA,varCompartida);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;
		return -1;

	} else {

		// El kernel ante una solicitud de grabar variable compartida debe retornarme el valor asignado.
		t_valor_variable valor = ((t_struct_numero*) structRecibido)->numero;

		log_trace(logger, "Variable Compartida: '%s' -> Valor: '%d'.", variableCompartida, valor);

		free(structRecibido);
		structRecibido = NULL;

		return valor;
	}

}

void irAlLabel(t_nombre_etiqueta etiqueta) {

	log_info(logger, "Se ingresa a la primitiva Ir a Label con la etiqueta: '%s'.", etiqueta);

	t_puntero_instruccion posicion_etiqueta = metadata_buscar_etiqueta(etiqueta, pcbEjecutando->indiceEtiquetas, pcbEjecutando->tamanioIndiceEtiquetas);

	if(posicion_etiqueta == -1) log_error(logger, "La etiqueta '%s' no se encuentra en el índice.", etiqueta);

	pcbEjecutando->programCounter = posicion_etiqueta - 1;
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta) {

	log_info(logger, "Se ingresa a la primitiva Llamar sin Retorno con la etiqueta: %s ",etiqueta);
    registroStack* nuevoRegistro = reg_stack_create();
    // Guardo el valor actual del program counter
    nuevoRegistro->retPos = pcbEjecutando->programCounter;
    list_add(pcbEjecutando->indiceStack, nuevoRegistro);

    irAlLabel(etiqueta);
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar) {
	log_info(logger, "Se ingresa a la primitiva Llamar con Retorno con la etiqueta: %s ",etiqueta);
	log_info(logger, "Preservando contexto de ejecución actual.");

	// Calculo la dirección de retorno y la guardo:
	registroStack * nuevoRegistro = reg_stack_create();
	nuevoRegistro->retVar.pagina = donde_retornar / tamanio_pagina;
	nuevoRegistro->retVar.offsetInstruccion = donde_retornar % tamanio_pagina;
	nuevoRegistro->retVar.longitudInstruccion = sizeof(int);

	// Guardo el valor actual del program counter
	nuevoRegistro->retPos = pcbEjecutando->programCounter;
	list_add(pcbEjecutando->indiceStack, nuevoRegistro);

	irAlLabel(etiqueta);
}

void finalizar(void)  {
	log_info(logger, "Se ingresa a la primitiva Finalizar");
	restaurarContextoDeEjecucion();

	if(list_is_empty(pcbEjecutando->indiceStack)){
		log_trace(logger, "Finalizar contexto principal.");
		finPrograma = true;
	}
	else{
		log_trace(logger, "Finalizar contexto actual.");
	}
}

void retornar(t_valor_variable retorno) {

	log_info(logger, "Se ingresa a la primitiva Retornar con el valor de variable %d ",retorno);
	// Tomo contexto actual:
	registroStack* registroActual = list_get(pcbEjecutando->indiceStack, pcbEjecutando->indiceStack->elements_count -1);

	// Calculo la dirección de retorno a partir de retVar:
	t_puntero offset_absoluto = (registroActual->retVar.pagina * tamanio_pagina) + registroActual->retVar.offsetInstruccion;
	asignar(offset_absoluto, retorno);

	finalizar();
}

//FUNCIONES KERNEL
void s_wait(t_nombre_semaforo semaforo) {

	t_struct_string * waitSemaforo = malloc(sizeof(t_struct_string));
	waitSemaforo->string=semaforo;

	socket_enviar(socketKernel,D_STRUCT_WAIT,semaforo);
	free(waitSemaforo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;

	} else {

		// Debe retornar un 1 si se bloqueo el semaforo o 0 en caso contrario
		int bloqueado = ((t_struct_numero*) structRecibido)->numero;

		if(bloqueado==1){
			pcbEjecutando->retornoPCB = D_STRUCT_ERROR_WAIT;
			log_trace(logger, "Proceso #%d bloqueado al hacer WAIT del semáforo: '%s'.", pcbEjecutando->PID, semaforo);
		} else if (bloqueado==0) {
			log_trace(logger, "WAIT del semáforo: '%s'. No hubo bloqueo.", pcbEjecutando->PID, semaforo);
		} else  {
			pcbEjecutando->retornoPCB = D_STRUCT_ERROR_SEM;
		}

		free(structRecibido);
		structRecibido = NULL;
	}
}

void s_signal(t_nombre_semaforo semaforo) {

	t_struct_string * signalSemaforo = malloc(sizeof(t_struct_string));
	signalSemaforo->string=semaforo;

	socket_enviar(socketKernel,D_STRUCT_SIGNAL,semaforo);
	free(signalSemaforo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;

	} else {

		int resultado = ((t_struct_numero*) structRecibido)->numero;

		if(resultado==KERNEL_ERROR){
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_SEM;
			log_trace(logger, "No se pudo realizar SIGNAL del semáforo: '%s'", semaforo);
		} else {
			log_trace(logger, "SIGNAL del semáforo: '%s' realizado exitosamente", semaforo);
		}

		free(structRecibido);
		structRecibido = NULL;
	}

}

t_puntero reservar(t_valor_variable espacio) {

	t_struct_sol_heap * heap = malloc(sizeof(t_struct_sol_heap));

	heap->pointer=espacio;
	heap->pid=pcbEjecutando->PID;

	socket_enviar(socketKernel,D_STRUCT_SOL_HEAP,heap);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;
		return -1;

	} else {

		switch(tipoEstructura){
		case D_STRUCT_RTA_HEAP: ;

			int pointer = ((t_struct_numero*) structRecibido)->numero ;
			log_trace(logger, "Memoria dinamica reservada exitosamente, puntero %d", pointer);
			free(structRecibido);
			structRecibido = NULL;

			return pointer;

			break;

		case D_STRUCT_ERROR_HEAP_MAX: ;

			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_HEAP_MAX;

			log_error(logger,"Tamaño de pagina solicitada supera el maximo");
			free(structRecibido);
			structRecibido = NULL;

			return NULL;

			break;

		case D_STRUCT_ERROR_HEAP: ;

			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_HEAP;

			log_error(logger,"No se pudo asignar paginas al proceso");
			free(structRecibido);
			structRecibido = NULL;

			return NULL;

			break;

		default:

			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;

			log_error(logger,"No se pudo reservar la memoria dinamica");
			free(structRecibido);
			structRecibido = NULL;

			return NULL;

			break;
		}
	}
}

void liberar(t_puntero puntero) {

	t_struct_sol_heap * heap = malloc(sizeof(t_struct_sol_heap));

	heap->pointer=puntero;
	heap->pid=pcbEjecutando->PID;

	socket_enviar(socketKernel,D_STRUCT_LIB_HEAP,heap);

	free(heap);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;

	} else {

		int resultado = ((t_struct_numero*) structRecibido)->numero;

		if(resultado==KERNEL_ERROR){
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_HEAP_LIB;
			log_trace(logger, "No se pudo liberar el puntero #%d en la memoria dinamica",puntero);
		} else {
			log_trace(logger, "Se libero correctamente el puntero #%d en la memoria dinamica",puntero);
		}

	}
}

t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas flags) {

	t_struct_archivo * archivo = malloc(sizeof(t_struct_archivo));

	archivo->fileDescriptor=0;
	archivo->tamanio=strlen(direccion)+1;
	archivo->informacion=malloc(strlen(direccion)+1);
	archivo->informacion=direccion;
	archivo->pid=pcbEjecutando->PID;
	archivo->flags.creacion=flags.creacion;
	archivo->flags.escritura=flags.escritura;
	archivo->flags.lectura=flags.lectura;

	socket_enviar(socketKernel,D_STRUCT_ARCHIVO_ABR,archivo);

	free(archivo->informacion);
	free(archivo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;
		return -1;

	} else {

		int archivo = ((t_struct_numero*) structRecibido)->numero;

		if(archivo==KERNEL_ERROR){

			log_trace(logger, "Error de apertura de archivo para el PID %d",pcbEjecutando->PID);
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_APERTURA;

			free(structRecibido);
			structRecibido = NULL;

			return -1;
		}

		log_trace(logger, "Apertura de archivo exitosa por el PID %d",pcbEjecutando->PID);

		free(structRecibido);
		structRecibido = NULL;

		return archivo;

	}
}

void borrar(t_descriptor_archivo fdArchivo) {

	t_struct_archivo * archivo = malloc(sizeof(t_struct_archivo));

	archivo->fileDescriptor=fdArchivo;
	archivo->pid=pcbEjecutando->PID;
	archivo->tamanio=0;

	socket_enviar(socketKernel,D_STRUCT_ARCHIVO_BOR,archivo);

	free(archivo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;

	} else {

		int respuesta = ((t_struct_numero*) structRecibido)->numero;

		if(respuesta==KERNEL_ERROR){

			log_trace(logger, "Error al borrar archivo para el PID %d",pcbEjecutando->PID);
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_BORRAR;

			free(structRecibido);
			structRecibido = NULL;
		} else {

			log_trace(logger, "Proceso #%d pudo borrar el archivo %d",pcbEjecutando->PID,fdArchivo);

			free(structRecibido);
			structRecibido = NULL;

		}

	}
}

void cerrar(t_descriptor_archivo fdArchivo) {

	t_struct_archivo * archivo = malloc(sizeof(t_struct_archivo));

	archivo->fileDescriptor=fdArchivo;
	archivo->pid=pcbEjecutando->PID;
	archivo->tamanio=0;

	socket_enviar(socketKernel,D_STRUCT_ARCHIVO_CER,archivo);

	free(archivo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;

	} else {

		int respuesta = ((t_struct_numero*) structRecibido)->numero;

		if(respuesta==KERNEL_ERROR){

			log_trace(logger, "Error al cerrar archivo para el PID %d",pcbEjecutando->PID);
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_CERRAR;

			free(structRecibido);
			structRecibido = NULL;
		} else {

			log_trace(logger, "Proceso #%d pudo cerrar el archivo %d",pcbEjecutando->PID,fdArchivo);

			free(structRecibido);
			structRecibido = NULL;

		}
	}
}

void moverCursor(t_descriptor_archivo fdArchivo, t_valor_variable posicion) {

	t_struct_archivo * archivo = malloc(sizeof(t_struct_archivo));

	archivo->fileDescriptor=fdArchivo;
	archivo->pid=pcbEjecutando->PID;
	archivo->tamanio=posicion;

	socket_enviar(socketKernel,D_STRUCT_ARCHIVO_MOV,archivo);

	free(archivo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;

	} else {

		int respuesta = ((t_struct_numero*) structRecibido)->numero;

		if(respuesta==KERNEL_ERROR){

			log_trace(logger, "Error al mover el cursor dentro del archivo para el PID %d",pcbEjecutando->PID);
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_CURSOR;

			free(structRecibido);
			structRecibido = NULL;
		} else {

			log_trace(logger, "Proceso #%d pudo mover el cursor dentro del archivo %d",pcbEjecutando->PID,fdArchivo);

			free(structRecibido);
			structRecibido = NULL;

		}

	}
}

void escribir(t_descriptor_archivo fdArchivo, void* informacion, t_valor_variable tamanio) {

	t_struct_archivo * archivo = malloc(sizeof(t_struct_archivo));

	archivo->fileDescriptor=fdArchivo;
	archivo->informacion=malloc(tamanio);
	archivo->informacion=informacion;
	archivo->tamanio=tamanio;
	archivo->pid=pcbEjecutando->PID;

	socket_enviar(socketKernel,D_STRUCT_ARCHIVO_ESC,archivo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;

	} else {

		int resultado = ((t_struct_numero*) structRecibido)->numero;

		if(resultado==KERNEL_ERROR){
			log_trace(logger, "Error de escritura de archivo para el PID %d",pcbEjecutando->PID);
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_ESCRITURA;
		}

		log_trace(logger, "Escritura de archivo exitosa por el PID %d",pcbEjecutando->PID);

		free(structRecibido);
		structRecibido = NULL;

	}
}

void leer(t_descriptor_archivo fdArchivo, t_puntero informacion, t_valor_variable tamanio) {

	t_struct_archivo * archivo = malloc(sizeof(t_struct_archivo));

	archivo->fileDescriptor=fdArchivo;
	archivo->tamanio=tamanio;
	archivo->informacion=malloc(sizeof(t_puntero));
	sprintf(archivo->informacion,"%d",informacion);
	archivo->pid=pcbEjecutando->PID;

	socket_enviar(socketKernel,D_STRUCT_ARCHIVO_LEC,archivo);

	free(archivo->informacion);
	free(archivo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB = D_STRUCT_ERROR_KERNEL;

	} else {

		int resultado = ((t_struct_numero*) structRecibido)->numero;

		if(resultado==KERNEL_ERROR){
			log_trace(logger, "Error de Lectura de archivo para el PID %d",pcbEjecutando->PID);
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_LECTURA;
			return;
		}

		socket_recibir(socketKernel, &tipoEstructura, &structRecibido);

		t_struct_string * lectura = ((t_struct_string*) structRecibido);

		asignar(informacion,atoi(lectura->string));

		log_trace(logger, "Lectura de archivo exitosa por el PID %d",pcbEjecutando->PID);

		free(structRecibido);
		structRecibido = NULL;
	}
}

bool esArgumento(t_nombre_variable identificador_variable){
	if(isdigit(identificador_variable)){
		return true;
	}else{
		return false;
	}
}

registroStack* reg_stack_create(){
	registroStack* reg = malloc(sizeof(registroStack));
	reg->args = list_create();
	reg->vars = list_create();
	reg->retPos = 0;
	reg->retVar.offsetInstruccion = 0;
	reg->retVar.pagina = 0;
	reg->retVar.longitudInstruccion = 0;

	return reg;
}

void restaurarContextoDeEjecucion(){

	log_trace(logger, "Restaurando contexto de ejecución anterior.");
	registroStack* registroActual = list_remove(pcbEjecutando->indiceStack, pcbEjecutando->indiceStack->elements_count -1);

	// Limpio los argumentos del registro y descuento el espacio que ocupan en el stack en memoria:
	int i;
	for(i = 0; i < registroActual->args->elements_count; i++){
		t_variable* arg = list_remove(registroActual->args, i);
		pcbEjecutando->stackPointer -= sizeof(int);
		free(arg); arg = NULL;
	}
	// Limpio las variables del registro y descuento el espacio que ocupan en el stack en memoria:
	for(i = 0; i < registroActual->vars->elements_count; i++){
		t_variable* var = list_remove(registroActual->vars, i);
		pcbEjecutando->stackPointer -= sizeof(int);
		free(var); var = NULL;
	}
	// Elimino el contexto actual del índice de stack, y seteo el nuevo contexto de ejecución en el index anterior:
	pcbEjecutando->programCounter = registroActual->retPos;

	free(registroActual);
	registroActual = NULL;
}

