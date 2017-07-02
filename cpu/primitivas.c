#include "primitivas.h"

bool termino = false;

//TODO borrar. constantes de prueba
static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;

//FUNCIONES
t_puntero definirVariable(t_nombre_variable variable) {

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
		registroStack* regStack = list_get(pcbEjecutando->indiceStack, pcbEjecutando->indiceStack->elements_count -1);

		// Si no hay registros, creo uno nuevo
		if(regStack == NULL){
			regStack = reg_stack_create();
			// Guardo el nuevo registro en el índice:
			list_add(pcbEjecutando->indiceStack, regStack);
		}

		if(!esArgumento(variable)){ // agrego nueva variable
			t_variable* new_var = malloc(sizeof(variable));
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
	} // fin else ERROR
}

t_puntero obtenerPosicionVariable(t_nombre_variable variable) {

	/// Obtengo el registro del stack correspondiente al contexto de ejecución actual:
	registroStack* regStack = list_get(pcbEjecutando->indiceStack, pcbEjecutando->indiceStack->elements_count -1);

	// Busco en este registro la variable que coincida con el nombre solicitado:
	if(!esArgumento(variable)){
		if(regStack->vars->elements_count > 0){ // si hay variables en la lista:

			// Obtengo la variable buscada:
			int i;
			for(i = 0; i<regStack->vars->elements_count; i++){

				t_variable* variable = list_get(regStack->vars, i);
				if(variable->identificador == variable){

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
				if(argumento->identificador == variable){

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

	t_posicion_memoria * var_direccion = malloc(sizeof(t_posicion_memoria));

	var_direccion->pagina = total_heap_offset / tamanio_pagina;
	var_direccion->offsetInstruccion = total_heap_offset % tamanio_pagina;
	var_direccion->longitudInstruccion = sizeof(int);

	log_trace(logger, "Solicitud Lectura -> Página: %i, Offset: %i, Size: %i.",
			var_direccion->pagina, var_direccion->offsetInstruccion, sizeof(int));

	// TODO handler en memoria y tipo de operacion para lectura variable PEDIDO_LECTURA_VARIABLE
	socket_enviar(socketKernel,D_STRUCT_LECT,var_direccion);

	free(var_direccion);
	var_direccion = NULL;

	// Valido el pedido de lectura a UMC:
	if(!validarPedidoMemoria()){ // hubo error de lectura
		log_error(logger, "La variable no pudo dereferenciarse.");
		//TODO implementar
		exitPorErrorUMC();

		return -1;
	}
	else{ // no hubo error de lectura
		t_tipoEstructura tipoEstructura;
		void * structRecibido;

		if ( socket_recibir(socketMemoria, &tipoEstructura, &structRecibido) == -1){

			log_error(logger, "La memoria se desconecto del sistema");
			exitFailureCPU();
			return NULL;

		} else {

			// TODO cambiar por estructura partipara devolver variable
			int valor = ((t_struct_numero*) structRecibido)->numero;

			log_trace(logger, "Variable dereferenciada -> Valor: %d.", valor);
			free(structRecibido);
			structRecibido = NULL;

			return valor;
		}

	}
}

void asignar(t_puntero total_heap_offset, t_valor_variable valor) {

	t_struct_sol_escritura* var_escritura = malloc(sizeof(t_struct_sol_escritura));

	var_escritura->pagina = total_heap_offset / tamanio_pagina;
	var_escritura->offset = total_heap_offset % tamanio_pagina;
	var_escritura->contenido = (char*) &valor;

	log_trace(logger, "Solicitud Escritura -> Página: %i, Offset: %i, Contenido: %d.",
			var_escritura->pagina, var_escritura->offset, valor);

	//TODO armar un nuevo tipo de operacion para enviar t_struct_sol_escritura PEDIDO_ESCRITURA
	socket_enviar(socketMemoria, D_STRUCT_LECT, var_escritura);

	free(var_escritura);
	var_escritura = NULL;

	if(!validarPedidoMemoria()){
		log_error(logger, "La variable no pudo asignarse en memoria.");
		//TODO implementar
		exitPorErrorUMC();
	}
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variableCompartida) {

	log_trace(logger, "Obteniendo valor de Variable Compartida: '%s'.", variableCompartida);

	//TODO implementar un nuevo tipo de operacion para OBTENER_VAR_COMPARTIDA
	socket_enviar(socketKernel, D_STRUCT_LECT, variableCompartida);

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if ( socket_recibir(socketKernel, &tipoEstructura, &structRecibido) == -1){

		log_error(logger, "El kernel se desconecto del sistema");
		exitFailureCPU();
		return NULL;

	} else {

		// TODO cambiar por estructura para devolver variable compartida
		t_valor_variable valor = ((t_struct_numero*) structRecibido)->numero;

		log_trace(logger, "Variable Compartida: '%s' -> Valor: '%d'.", variableCompartida, valor);

		free(structRecibido);
		structRecibido = NULL;

		return valor;
	}

}

t_valor_variable asignarValorCompartida(t_nombre_compartida variableCompartida, t_valor_variable valor) {

	log_trace(logger, "Asignando el valor '%d' a Variable Compartida '%s'.", valor, variableCompartida);
	t_struct_var_compartida * varCompartida = malloc(strlen(variableCompartida)+ 5);

	varCompartida->valor = valor;
	varCompartida->nombre = variableCompartida;

	//TODO crear un tipo de operacion para manejar grabar las variables compartidas GRABAR_VAR_COMPARTIDA
	socket_enviar(socketKernel,D_STRUCT_NUMERO,varCompartida);

	return valor;
}

void irAlLabel(t_nombre_etiqueta etiqueta) {

	log_trace(logger, "Llendo a la etiqueta: '%s'.", etiqueta);
	t_puntero_instruccion posicion_etiqueta = metadata_buscar_etiqueta(etiqueta, pcbEjecutando->indiceEtiquetas, pcbEjecutando->tamanioIndiceEtiquetas);

	if(posicion_etiqueta == -1) log_error(logger, "La etiqueta '%s' no se encuentra en el índice.", etiqueta);

	pcbEjecutando->programCounter = posicion_etiqueta - 1;
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta) {

	log_trace(logger, "Llamada sin retorno.");
    registroStack* nuevoRegistro = reg_stack_create();
    // Guardo el valor actual del program counter
    nuevoRegistro->retPos = pcbEjecutando->programCounter;
    list_add(pcbEjecutando->indiceStack, nuevoRegistro);

    irAlLabel(etiqueta);
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar) {
	log_trace(logger, "Llamada con retorno. Preservando contexto de ejecución actual.");

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

	log_trace(logger, "Llamada a función 'retornar'.");
	// Tomo contexto actual:
	registroStack* registroActual = list_get(pcbEjecutando->indiceStack, pcbEjecutando->indiceStack->elements_count -1);

	// Calculo la dirección de retorno a partir de retVar:
	t_puntero offset_absoluto = (registroActual->retVar.pagina * tamanio_pagina) + registroActual->retVar.offsetInstruccion;
	asignar(offset_absoluto, retorno);

	finalizar();
}

//FUNCIONES KERNEL
void s_wait(t_nombre_semaforo identificador_semaforo) {
	//TODO implementar
}

void s_signal(t_nombre_semaforo identificador_semaforo) {
	//TODO implementar
}

t_puntero reservar(t_valor_variable espacio) {
	//TODO implementar
	return NULL;
}

void liberar(t_puntero puntero) {
	//TODO implementar
}

t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas flags) {
	//TODO implementar
	return NULL;
}

void borrar(t_descriptor_archivo descriptor_archivo) {
	//TODO implementar
}

void cerrar(t_descriptor_archivo descriptor_archivo) {
	//TODO implementar
}

void moverCursor(t_descriptor_archivo descriptor_archivo, t_valor_variable posicion) {
	//TODO implementar
}
void escribir(t_descriptor_archivo descriptor_archivo, void* informacion, t_valor_variable tamanio) {
	//TODO implementar
}

void leer(t_descriptor_archivo descriptor_archivo, t_puntero informacion, t_valor_variable tamanio) {
	//TODO implementar
}

bool terminoElPrograma() {
	return termino;
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
	reg->cantidad_args = 0;
	reg->args = list_create();
	reg->cantidad_vars = 0;
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

