#include "primitivas.h"

//FUNCIONES
t_puntero definirVariable(t_nombre_variable variable) {

	log_info(logger,"Se ingresa a la primitiva Definir Variable para la variable: %c",variable);

	// Verifico si se desborda la pila en memoria:
	if(pcbEjecutando->stackPointer + 4 > tamanio_pagina &&
			pcbEjecutando->paginaActualStack - pcbEjecutando->primerPaginaStack == tamanio_stack-1){

		log_info(logger, "Stack Overflow al definir variable '%c'.", variable);
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


		t_puntero punteroRetorno;

		if(!esArgumento(variable)){ // agrego nueva variable

			t_variable* new_var = malloc(sizeof(t_variable));

			if(pcbEjecutando->stackPointer + 4 > tamanio_pagina){
				pcbEjecutando->paginaActualStack++;
				new_var->posicionMemoria.offsetInstruccion=0;
				pcbEjecutando->stackPointer=4;
			}else{
				new_var->posicionMemoria.offsetInstruccion=pcbEjecutando->stackPointer;
				pcbEjecutando->stackPointer+=4;
			}

			new_var->identificador = variable;
			new_var->posicionMemoria.pagina = pcbEjecutando->paginaActualStack;
			new_var->posicionMemoria.longitudInstruccion = sizeof(int);

			list_add(regStack->vars, new_var);
			log_info(logger, "Se define variable en Dirección lógica: %i, %i, %i.",
					new_var->posicionMemoria.pagina, new_var->posicionMemoria.offsetInstruccion, sizeof(int));
			punteroRetorno = new_var->posicionMemoria.pagina * tamanio_pagina + new_var->posicionMemoria.offsetInstruccion;
		}
		else{ // agrego nuevo argumento
			t_posicion_memoria* new_arg = malloc(sizeof(variable));

			if(pcbEjecutando->stackPointer + 4 > tamanio_pagina){
				pcbEjecutando->paginaActualStack++;
				pcbEjecutando->stackPointer=4;
				new_arg->offsetInstruccion=0;
			}else{
				pcbEjecutando->stackPointer+=4;
				new_arg->offsetInstruccion=pcbEjecutando->stackPointer;
			}

			new_arg->pagina = pcbEjecutando->paginaActualStack;
			new_arg->longitudInstruccion = sizeof(int);

			list_add(regStack->args, new_arg);

			log_info(logger, "Se define argumento en Dirección lógica: %i, %i, %i.",new_arg->pagina, new_arg->offsetInstruccion, sizeof(int));
			punteroRetorno = new_arg->pagina * tamanio_pagina + new_arg->offsetInstruccion;
		}


		return punteroRetorno;
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

			int nroArg = idVariable - '0';
			t_posicion_memoria* argumento = list_get(regStack->args, nroArg);
			// Obtengo el argumento buscado:

			int arg_offset_absoluto = (argumento->pagina * tamanio_pagina) + argumento->offsetInstruccion;

			return arg_offset_absoluto;

		}
		log_error(logger, "No hay argumentos en el registro actual de stack.");
		return -1;
	}
}

t_valor_variable dereferenciar(t_puntero total_heap_offset) {

	log_info(logger,"Se ingresa a la primitiva Dereferenciar con el puntero: %d", total_heap_offset);

	t_struct_sol_lectura * var_direccion = malloc(sizeof(t_struct_sol_lectura));

	var_direccion->pagina = total_heap_offset / tamanio_pagina;
	var_direccion->offset = total_heap_offset % tamanio_pagina;
	var_direccion->contenido = sizeof(int);
	var_direccion->PID = pcbEjecutando->PID;

	log_info(logger, "Solicitud Lectura -> Página: %i, Offset: %i, Size: %i.",
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

void asignar(t_puntero direccion, t_valor_variable valor) {

	log_info(logger,"Se ingresa a la primitiva Asignar para el puntero %d con el valor %d", direccion,valor);

	t_struct_sol_escritura* var_escritura = malloc(sizeof(t_struct_sol_escritura));

	var_escritura->pagina = direccion / tamanio_pagina;
	var_escritura->offset = direccion % tamanio_pagina;
	var_escritura->tamanio = sizeof(t_valor_variable);
	var_escritura->PID = pcbEjecutando->PID;
	var_escritura->contenido = malloc(sizeof(t_valor_variable));

	memcpy(var_escritura->contenido, &valor, sizeof(t_valor_variable));

	log_trace(logger, "Solicitud Escritura -> Página: %i, Offset: %i, Size: %i, Contenido: %s.",
			var_escritura->pagina, var_escritura->offset, var_escritura->tamanio, var_escritura->contenido);

	socket_enviar(socketMemoria, D_STRUCT_SOL_ESCR, var_escritura);

	free(var_escritura);
	var_escritura = NULL;

	if(!validarPedidoMemoria()){
		log_error(logger, "La variable no pudo asignarse en memoria.");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_MEMORIA;
	}
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variableCompartida) {

	char * variableLimpia = prepararInstruccion(variableCompartida);

	log_info(logger, "Se ingresa a la primitiva Obtener valor variable compartida para la variable: '%s'.", variableLimpia);

	t_struct_string* obtenerVarCompartida = malloc(sizeof(t_struct_string));

	obtenerVarCompartida->string = variableLimpia;

	socket_enviar(socketKernel, D_STRUCT_OBTENER_COMPARTIDA, obtenerVarCompartida);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		free(variableLimpia);
		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;
		return NULL;

	} else {

		// El kernel ante una solicitud de variable compartida debe retornarme el valor entero correspondiente.
		t_valor_variable valor = ((t_struct_numero*) structRecibido)->numero;

		log_trace(logger, "Variable Compartida: '%s' -> Valor: '%d'.", variableLimpia, valor);

		free(structRecibido);
		structRecibido = NULL;
		free(variableLimpia);
		return valor;
	}

}

t_valor_variable asignarValorCompartida(t_nombre_compartida variableCompartida, t_valor_variable valor) {

	char * variableLimpia = prepararInstruccion(variableCompartida);

	log_info(logger, "Se ingresa a la primitiva Asignar valor compartida para  '%d' a Variable Compartida '%s'.", valor, variableLimpia);
	t_struct_var_compartida * varCompartida = malloc(strlen(variableCompartida)+ 5);

	varCompartida->valor = valor;

//	strcpy(varCompartida->nombre,"!\0");
	varCompartida->nombre = variableLimpia;

	socket_enviar(socketKernel,D_STRUCT_GRABAR_COMPARTIDA,varCompartida);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		free(variableLimpia);
		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;
		return -1;

	} else {

		// El kernel ante una solicitud de grabar variable compartida debe retornarme el valor asignado.
		t_valor_variable valor = ((t_struct_numero*) structRecibido)->numero;

		log_trace(logger, "Variable Compartida: '%s' -> Valor: '%d'.", variableCompartida, valor);

		free(structRecibido);
		structRecibido = NULL;
		free(variableLimpia);
		return valor;
	}

}

void irAlLabel(t_nombre_etiqueta etiqueta) {

	char * etiquetaLimpia = prepararInstruccion(etiqueta);

	log_info(logger, "Se ingresa a la primitiva Ir a Label con la etiqueta: '%s'.", etiquetaLimpia);

	t_puntero_instruccion posicion_etiqueta = metadata_buscar_etiqueta(etiquetaLimpia,
			pcbEjecutando->indiceEtiquetas, pcbEjecutando->tamanioIndiceEtiquetas);

	if(posicion_etiqueta == -1) log_error(logger, "La etiqueta '%s' no se encuentra en el índice.", etiquetaLimpia);

	free(etiquetaLimpia);

	pcbEjecutando->programCounter = posicion_etiqueta - 1;

}

void llamarSinRetorno(t_nombre_etiqueta etiqueta) {

	char * etiquetaLimpia = prepararInstruccion(etiqueta);

	log_info(logger, "Se ingresa a la primitiva Llamar sin Retorno con la etiqueta: %s ",etiquetaLimpia);

    registroStack* nuevoRegistro = reg_stack_create();

    // Guardo el valor actual del program counter
    nuevoRegistro->retPos = pcbEjecutando->programCounter;
    list_add(pcbEjecutando->indiceStack, nuevoRegistro);

    irAlLabel(etiqueta);
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar) {

	char * etiquetaLimpia = prepararInstruccion(etiqueta);

	log_info(logger, "Se ingresa a la primitiva Llamar con Retorno con la etiqueta: %s ",etiquetaLimpia);
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

	char * semaforoLimpio = prepararInstruccion(semaforo);

	log_info(logger,"Se ingreso a la funcion kernel Wait para el semaforo %s",semaforoLimpio);

	t_struct_string * waitSemaforo = malloc(sizeof(t_struct_string));

	waitSemaforo->string=semaforoLimpio;

	socket_enviar(socketKernel,D_STRUCT_WAIT,waitSemaforo);
	free(waitSemaforo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;
		free(semaforoLimpio);

	} else {

		// Debe retornar un 1 si se bloqueo el semaforo o 0 en caso contrario
		int bloqueado = ((t_struct_numero*) structRecibido)->numero;

		if(bloqueado==1){
			pcbEjecutando->retornoPCB = D_STRUCT_ERROR_WAIT;
			log_trace(logger, "Proceso #%d bloqueado al hacer WAIT del semáforo: '%s'.", pcbEjecutando->PID, semaforoLimpio);
		} else if (bloqueado==0) {
			log_trace(logger, "WAIT del semáforo: '%s'. No hubo bloqueo.", semaforoLimpio);
		} else  {
			pcbEjecutando->retornoPCB = D_STRUCT_ERROR_SEM;
		}

		free(structRecibido);
		structRecibido = NULL;
		free(semaforoLimpio);
	}
}

void s_signal(t_nombre_semaforo semaforo) {

	char * semaforoLimpio = prepararInstruccion(semaforo);

	log_info(logger,"Se ingreso a la funcion kernel Signal para el semaforo %s",semaforoLimpio);

	t_struct_string * signalSemaforo = malloc(sizeof(t_struct_string));
	signalSemaforo->string=semaforoLimpio;

	socket_enviar(socketKernel,D_STRUCT_SIGNAL,signalSemaforo);
	free(signalSemaforo);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		pcbEjecutando->retornoPCB=D_STRUCT_ERROR_KERNEL;
		free(semaforoLimpio);

	} else {

		int resultado = ((t_struct_numero*) structRecibido)->numero;

		if(resultado==KERNEL_ERROR){
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_SEM;
			log_trace(logger, "No se pudo realizar SIGNAL del semáforo: '%s'", semaforoLimpio);
		} else {
			log_trace(logger, "SIGNAL del semáforo: '%s' realizado exitosamente", semaforoLimpio);
		}

		free(structRecibido);
		structRecibido = NULL;
		free(semaforoLimpio);
	}

}

t_puntero reservar(t_valor_variable espacio) {

	log_info(logger,"Se ingreso a la funcion kernel Reservar con el valor %d",espacio);

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

	log_info(logger,"Se ingreso a la funcion kernel Liberar con el valor %d",puntero);

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

	log_info(logger,"Se ingreso a la funcion kernel Abrir archivo para el path %s",direccion);

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

		if(archivo==FS_ABRIR_CREAR_ERROR){

			log_error(logger, "El archivo para el PID %d no se pudo crear por falta de espacio",pcbEjecutando->PID);
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_ESPACIO_ARCHIVO;
			free(structRecibido);
			structRecibido = NULL;
			return -1;

		} else if (archivo==FS_ABRIR_NO_CREAR_ERROR){
			log_error(logger, "El archivo para el PID %d no se pudo crear por falta de permisos",pcbEjecutando->PID);
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_ABRIR;
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

	log_info(logger,"Se ingreso a la funcion kernel Borrar archivo para el FD %d",fdArchivo);

	t_struct_sol_lectura * archivo = malloc(sizeof(t_struct_sol_lectura));

	archivo->pagina=fdArchivo;
	archivo->PID=pcbEjecutando->PID;
	archivo->contenido=0;
	archivo->offset=0;

	socket_enviar(socketKernel,D_STRUCT_ARCHIVO_BOR,archivo);

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
	free(archivo);
}

void cerrar(t_descriptor_archivo fdArchivo) {

	log_info(logger,"Se ingreso a la funcion kernel Cerrar archivo para el FD %d",fdArchivo);

	t_struct_sol_lectura * archivo = malloc(sizeof(t_struct_sol_lectura));

	archivo->pagina=fdArchivo;
	archivo->PID=pcbEjecutando->PID;
	archivo->offset=0;
	archivo->contenido=0;

	socket_enviar(socketKernel,D_STRUCT_ARCHIVO_CER,archivo);

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
	free(archivo);
}

void moverCursor(t_descriptor_archivo fdArchivo, t_valor_variable posicion) {

	log_info(logger,"Se ingreso a la funcion kernel Mover Cursor Archivo para el FD %d",fdArchivo);

	t_struct_sol_lectura * moverCursor = malloc(sizeof(t_struct_sol_lectura));

	moverCursor->pagina=fdArchivo;
	moverCursor->PID=pcbEjecutando->PID;
	moverCursor->offset=posicion;
	moverCursor->contenido=0;

	socket_enviar(socketKernel,D_STRUCT_ARCHIVO_MOV,moverCursor);

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
	free(moverCursor);
}

void escribir(t_descriptor_archivo fdArchivo, void* informacion, t_valor_variable tamanio) {

	log_info(logger,"Se ingreso a la funcion kernel Escribir archivo para el FD %d",fdArchivo);

	t_struct_archivo * archivo = malloc(sizeof(t_struct_archivo));

	archivo->fileDescriptor=fdArchivo;
	archivo->informacion=malloc(tamanio);
	memcpy(archivo->informacion,informacion,tamanio);
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
			log_info(logger, "Error de escritura de archivo para el PID %d",pcbEjecutando->PID);
			pcbEjecutando->retornoPCB=D_STRUCT_ERROR_ESCRITURA;
		} else {
			log_info(logger, "Escritura de archivo exitosa por el PID %d",pcbEjecutando->PID);
		}

		free(structRecibido);
		structRecibido = NULL;

	}
}

void leer(t_descriptor_archivo fdArchivo, t_puntero informacion, t_valor_variable tamanio) {

	log_info(logger,"Se ingreso a la funcion kernel Leer archivo para el FD %d",fdArchivo);

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

		asignar(informacion,&lectura->string);

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

