#include "manejoMemoria.h"

#define CANTIDAD_ELEMENTOS_CACHE 15

t_config * config;

t_configuracion configuracion;

void * memoriaPrincipal;

void cargarConfiguracion(void) {

	config = config_create("./config.txt");

	if(config == NULL){

		config = config_create("../config.txt");

	}

	retardoLecturaMemoria = 100;

	configuracion.marcoSize = config_get_int_value(config, "MARCO_SIZE");
	configuracion.puerto = config_get_int_value(config, "PUERTO");
	configuracion.marcos = config_get_int_value(config, "MARCOS");
	configuracion.marcoSize = config_get_int_value(config, "MARCO_SIZE");
	configuracion.entradasCache = config_get_int_value(config, "ENTRADAS_CACHE");
	configuracion.cacheXProc = config_get_int_value(config, "CACHE_X_PROC");
	configuracion.reemplazoCache = config_get_int_value(config, "REEMPLAZO_CACHE");
	configuracion.retardoMemoria = config_get_int_value(config, "RETARDO_MEMORIA");
	configuracion.stackSize = config_get_int_value(config, "STACK_SIZE");

	log_info(logger,"El Puerto es %d\n",configuracion.puerto);
	log_info(logger,"La cantidad de Marcos es %d\n",configuracion.marcos);
	log_info(logger,"El tamaño de cada Marco es %d\n",configuracion.marcoSize);
	log_info(logger,"Las entradas en Cache son %d\n",configuracion.entradasCache);
	log_info(logger,"La cantidad maxima de de entradas de la cache asignables a cada programa es %d\n",configuracion.cacheXProc);
	log_info(logger,"El reemplazo de cache es %d\n",configuracion.reemplazoCache);
	log_info(logger,"El retardo de la Memoria es %d\n",configuracion.retardoMemoria);
	log_info(logger,"La cantidad de paginas asignada al Stack es %d\n",configuracion.stackSize);
}

void crearMemoriaPrincipal() {
	memoriaPrincipal = malloc(configuracion.marcos*configuracion.marcoSize);
}

void inicializarListas(){
	listaKernel = list_create();
	listaCpus = list_create();
}

void crearThreadAtenderConexiones(){

//	pthread_create(&threadAtenderConexiones, NULL, administrarConexiones, NULL);

}

void administrarConexiones(){

	//Creo el servidor de memoria que recibirá las nuevas conexiones
	int socketServidor = crearServidor(configuracion.puerto);

	pthread_t nueva_solicitud;

	tamanio_pagina = malloc(sizeof(t_struct_numero));
	tamanio_pagina->numero = configuracion.marcoSize;

	while(1){
		//Por defecto acepto el cliente que se está conectando
		int socketCliente = aceptarCliente(socketServidor);

		if(socketCliente!=-1){

			void* structRecibido;
			t_tipoEstructura tipoStruct;

			//Recibo el mensaje para identificar quien es y hacer el handshake
			int resultado = socket_recibir(socketCliente, &tipoStruct, &structRecibido);

			if(resultado == -1 || tipoStruct != D_STRUCT_NUMERO){
				continue;

			} else {

				switch(((t_struct_numero*) structRecibido)->numero){
							case ES_KERNEL:

								log_info(logger,"Se conecto el Kernel");

								list_add(listaKernel, (void*) socketCliente);

								tamanio_pagina = malloc(sizeof(t_struct_numero));
								tamanio_pagina->numero = configuracion.marcoSize;
								socket_enviar(socketCliente, D_STRUCT_NUMERO, tamanio_pagina);
								free(tamanio_pagina);

//								pthread_create(&nueva_solicitud, NULL, manejarKernel, socketCliente);

								break;
							case ES_CPU:

								log_info(logger,"Se conecto una CPU");

								list_add(listaCpus, (void*) socketCliente);

								tamanio_pagina = malloc(sizeof(t_struct_numero));
								tamanio_pagina->numero = configuracion.marcoSize;
								socket_enviar(socketCliente, D_STRUCT_NUMERO, tamanio_pagina);
								free(tamanio_pagina);

//								pthread_create(&nueva_solicitud, NULL, manejarCpu, socketCliente);

								break;

							default:

								log_error(logger,"No se pudo hacer el handshake");

								//Ciero el FD del cliente que había aceptado
								close(socketCliente);
				}
			}

			free(structRecibido);
		}
	}
}

void manejarCpu(int i){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(i,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"El Cpu %d cerró la conexión.",i);
		removerClientePorCierreDeConexion(i,listaCpus);
	} else {
	}
};

//void manejarKernel(int socketKernel){
//
//	while(1) {
//
//		t_tipoEstructura tipoEstructura;
//		void * structRecibido;
//
//		if (socket_recibir(socketKernel,&tipoEstructura,&structRecibido) == -1) {
//
//			log_info(logger,"El Kernel %d cerró la conexión.",socketKernel);
//			removerClientePorCierreDeConexion(socketKernel,listaKernel);
//
//		} else {
//
//			switch(tipoEstructura){
//			case D_STRUCT_MALC:
//
//				log_info(logger,"Se solicita crear segmento para el PID %i",
//						((t_struct_malloc* )structRecibido)->PID);
//
//				// TODO ver con edu que pasa con los valores no enteros de paginas
//				int paginasNecesarias =
//						((t_struct_malloc* )structRecibido)->tamano_segmento / configuracion.marcoSize;
//
////				int resultado = asignarPaginasProceso(((t_struct_malloc* )structRecibido)->PID, paginasNecesarias);
//
//				//Le comunico al kernel si se pudo realizar operacion
//				t_struct_numero* respuestaAsignacion = malloc(sizeof(t_struct_numero));
//				respuestaAsignacion->numero = resultado;
//					socket_enviar(socketKernel, D_STRUCT_NUMERO, respuestaAsignacion);
//
//					if(resultado == -1){
//					log_error(logger,"No se pudo crear el segmento solicitado");
//					} else {
//					log_info(logger,"Se creo con exito el segmento solicitado");
//					}
//
//					free(respuestaAsignacion);
//
//					break;
//
//				}
//			}
//	}
//}

void removerClientePorCierreDeConexion(int cliente, t_list* lista) {

	//Elimino el cliente de la lista
	bool _es_cliente_numero(int elemento) {
		return (elemento == cliente);
	}
	list_remove_by_condition(lista, (void*) _es_cliente_numero);

	//Cierro el FD correspondiente al cliente
	close(cliente);

}

void liberarMemoriaPrincipal(){
	free(memoriaPrincipal);
}

void crearEstructurasAdministrativas(){
	int i;

	tablaInvertida = (t_filaTablaInvertida*)memoriaPrincipal;
	int limit = configuracion.marcos;
	int tamanioFrame = configuracion.marcoSize;
	for(i = 0; i < limit; i++){
		tablaInvertida[i].pid = FRAME_LIBRE;
		tablaInvertida[i].frame = i;
		tablaInvertida[i].pagina = 0;
	}
	int bytesTablaInvertida = sizeof(t_filaTablaInvertida)*limit;
	int framesTablaInvertida = bytesTablaInvertida/tamanioFrame;
	if(bytesTablaInvertida%tamanioFrame != 0){
		framesTablaInvertida++;
	}
	for(i = 0; i < framesTablaInvertida; i++){
		tablaInvertida[i].pid = FRAME_ESTRUCTURA_ADMINISTRATIVA;
	}
	log_info(logger, "Estructuras Administrativas creadas exitosamente.\n");
	log_info(logger, "Las estructuras administrativas ocupan %d marcos de memoria.\n", framesTablaInvertida);
}

void escribirEnMemoria(int numeroFrame,void* contenido, int size, int offset){
	void* punteroFrame = memoriaPrincipal + numeroFrame * configuracion.marcoSize;
	memcpy(punteroFrame + offset,contenido,size);
	log_info(logger,"Se escribio la pagina %d", numeroFrame);
}

int cantidadFramesLibres(){
	int limite = configuracion.marcos;
	int i = 0;
	int framesLibres = 0;
	for(i = 0; i < limite; i++){
		if(tablaInvertida[i].pid == FRAME_LIBRE){
			framesLibres++;
		}
	}
	log_info(logger,"Quedan %d frames libres.", framesLibres);
	return framesLibres;
}

void registrarUsoDeFrame(int pid,int numeroFrame, int paginaProceso){
	tablaInvertida[numeroFrame].pid = pid;
	tablaInvertida[numeroFrame].pagina = paginaProceso;
}

int obtenerPrimerFrameLibre(){
	int i = 0;
	int primeroLibre = -1;
	int limite = configuracion.marcos;
	while(primeroLibre < 0 && i < limite){
		if(tablaInvertida[i].pid == FRAME_LIBRE){
			primeroLibre = i;
		}
		i++;
	}
	return primeroLibre;
}

int obtenerPrimerosNFramesLibre(int cantidadDeFrames){
	int i = 0;
	int contador = 0;
	int limite = configuracion.marcos;
	while(contador < cantidadDeFrames && i < limite){
		if(tablaInvertida[i].pid == FRAME_LIBRE){
			contador++;
		} else {
			contador = 0;
		}
		i++;
	}
	int indice = -1;
	if(contador == cantidadDeFrames){
		indice = i - contador;
	}
	return indice;
}


void reservarFramesProceso(int pid, int cantidadBytes, int bytesContiguos){ // 1 TRUE 0 FALSE
	int i = 0;
	int bytesPorFrame = configuracion.marcoSize;
	int framesNecesarios = cantidadBytes/bytesPorFrame;
	if(cantidadBytes%bytesPorFrame != 0){
		framesNecesarios++;
	}
	if(bytesContiguos > 0){
		int primerFrameLibre = obtenerPrimerosNFramesLibre(framesNecesarios);
		if(primerFrameLibre > 0){ //Tiene N frames libres contiguos
			for(i = 0; i < framesNecesarios;i++){
				registrarUsoDeFrame(pid,primerFrameLibre+i,i+1);
			}
		} else {
			log_info(logger,"No se reservaron paginas para el proceso %d por que no se dispone de los %d frames libres contiguos necesarios.\n",pid,framesNecesarios);
		}
	} else {
		int framesLibres = cantidadFramesLibres();
		if(framesNecesarios <= framesLibres){
			for(i = 0; i < framesNecesarios;i++){
				int numeroFrame = obtenerPrimerFrameLibre();
				registrarUsoDeFrame(pid,numeroFrame,i+1);
			}
		} else {
			log_info(logger,"No se reservaron paginas para el proceso %d por que no se dispones de %d frames libres.\n",pid,framesNecesarios);
		}
	}
}

int buscarProcesoCache(int pagina, int pid){
	int i = 0;
	int indice = -1;
	for(i = 0; i < CANTIDAD_ELEMENTOS_CACHE; i++){
		if(cache[i].pid == pid && cache[i].pagina == pagina){
			indice = i;
		}
	}
	return indice;
}


void* leerPagina(int pagina, int pid){
	int indiceCache = buscarProcesoCache(pagina, pid);
	if(indiceCache > -1){
		return cache[indiceCache].contenido;
	} else {
		return leerMemoria(pagina,pid);
	}
}

int buscarPaginaMemoria(int pagina, int pid){
	int i = 0;
	int indice = -1;
	int cantidadFrames = configuracion.marcos;
	while(indice < 0 && i < cantidadFrames){
		if(tablaInvertida[i].pid == pid && tablaInvertida[i].pagina == pagina){
			indice = i;
		}
		i++;
	}
	return indice;
}

void* leerMemoria(int pagina, int pid){
	int numeroFrame = buscarPaginaMemoria(pagina,pid);
	void * posicion = memoriaPrincipal + numeroFrame*configuracion.marcoSize;
	actualizarCache(pid,pagina,posicion);
//	aplicarRetardo();
	return posicion;
}

void crearCache(){
	cache = malloc(sizeof(t_cache) * CANTIDAD_ELEMENTOS_CACHE);
	int i;
	for(i = 0; i < CANTIDAD_ELEMENTOS_CACHE; i++){
		cache[i].pid = FRAME_LIBRE;
		cache[i].pagina = 0;
		cache[i].contadorDeUso = 0;
	}
}

void vaciarCache(){
	free(cache);
}

void aplicarRetardo(){
	usleep(retardoLecturaMemoria);
	log_info(logger,"Durmiendo %d milisegundos...zzzzzzz",retardoLecturaMemoria);
}

void establecerRetardoMemoria(int cantidad){
	retardoLecturaMemoria = cantidad;
}

int obtenerLRUElementoCache(){
	int i = 0;
	int menor = -1;
	for(i = 0; i < CANTIDAD_ELEMENTOS_CACHE; i++){
		if(cache[i].contadorDeUso > menor){
			menor = i;
		}
	}
	return menor;
}

void actualizarCache(int pid,int pagina,void* punteroMarco){
	int indiceCache = obtenerLRUElementoCache();
	cache[indiceCache].contadorDeUso = 1;
	cache[indiceCache].contenido = punteroMarco;
	cache[indiceCache].pagina = pagina;
	cache[indiceCache].pid = pid;
}

void borrarProcesoCache(int pid){
	int i = 0;
	for(i = 0; i < CANTIDAD_ELEMENTOS_CACHE; i++){
		if(cache[i].pid == pid){
			cache[i].contadorDeUso = 0;
			cache[i].pagina = 0;
			cache[i].pid = FRAME_LIBRE;
		}
	}
}

void finalizarPrograma(int pid){
	int i;
	int limite = configuracion.marcos;
	for(i = 0; i < limite; i++){
		if(tablaInvertida[i].pid == pid){
			tablaInvertida[i].pid = FRAME_LIBRE;
		}
	}
	borrarProcesoCache(pid);
	log_info(logger,"Se desasignaron todos los frames asignados al proceso %d",pid);
}

void* leerOffsetPagina(void* marco,int offset,int longitud){
	char * pagina = (char *) marco;
	char * contenido = malloc(longitud);
	memcpy(contenido,pagina[offset],longitud);
	return contenido;
}

void imprimirTablaPaginas(){
	int i;
	int limite = configuracion.marcos;
	for(i = 0; i < limite; i++){
		printf("Frame: %d  PID: %d  #Pagina: %d\n",tablaInvertida[i].frame,tablaInvertida[i].pid,tablaInvertida[i].pagina);
	}
}


