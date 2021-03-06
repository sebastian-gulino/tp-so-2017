#include "manejoMemoria.h"

#define CANTIDAD_ELEMENTOS_CACHE 15

void * memoriaPrincipal;

void cargarConfiguracion() {

	printf("INICIANDO MEMORIA\n");
	printf("\n");
	printf("\n");

	consolaConectada = 1;

	pthread_mutex_init(&mutex_log, NULL);
	pthread_mutex_init(&mutex_memoria, NULL);

	t_config * config;

	pathConfiguracion = "./config.txt";
	config = config_create(pathConfiguracion);

	if(config == NULL){

		pathConfiguracion = "../config.txt";
		config = config_create(pathConfiguracion);
	}

	retardoLecturaMemoria = 100;

	configuracion = malloc(sizeof(t_configuracion));

	configuracion->marcoSize = config_get_int_value(config, "MARCO_SIZE");
	configuracion->puerto = config_get_int_value(config, "PUERTO");
	configuracion->marcos = config_get_int_value(config, "MARCOS");
	configuracion->marcoSize = config_get_int_value(config, "MARCO_SIZE");
	configuracion->entradasCache = config_get_int_value(config, "ENTRADAS_CACHE");
	configuracion->cacheXProc = config_get_int_value(config, "CACHE_X_PROC");
	configuracion->reemplazoCache = config_get_int_value(config, "REEMPLAZO_CACHE");
	configuracion->retardoMemoria = config_get_int_value(config, "RETARDO_MEMORIA");
	configuracion->stackSize = config_get_int_value(config, "STACK_SIZE");

	pthread_mutex_lock(&mutex_log);

	log_info(logger,"El Puerto es %d\n",configuracion->puerto);
	log_info(logger,"La cantidad de Marcos es %d\n",configuracion->marcos);
	log_info(logger,"El tamaño de cada Marco es %d\n",configuracion->marcoSize);
	log_info(logger,"Las entradas en Cache son %d\n",configuracion->entradasCache);
	log_info(logger,"La cantidad maxima de de entradas de la cache asignables a cada programa es %d\n",configuracion->cacheXProc);
	log_info(logger,"El reemplazo de cache es %d\n",configuracion->reemplazoCache);
	log_info(logger,"El retardo de la Memoria es %d\n",configuracion->retardoMemoria);
	log_info(logger,"La cantidad de paginas asignada al Stack es %d\n",configuracion->stackSize);

	pthread_mutex_unlock(&mutex_log);

	config_destroy(config);
}

void crearMemoriaPrincipal() {
	memoriaPrincipal = malloc(configuracion->marcos*configuracion->marcoSize);
}

void inicializarListas(){
	listaKernel = list_create();
	listaCpus = list_create();
}

void crearThreadAtenderConexiones(){

	pthread_create(&threadAtenderConexiones, NULL, administrarConexiones, NULL);

}

void administrarConexiones(){

	//Creo el servidor de memoria que recibirá las nuevas conexiones
	int socketServidor = crearServidor(configuracion->puerto);

	pthread_t nueva_solicitud;

	tamanio_pagina = malloc(sizeof(t_struct_numero));
	tamanio_pagina->numero = configuracion->marcoSize;

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

								pthread_mutex_lock(&mutex_log);
								log_info(logger,"Se conecto el Kernel");
								pthread_mutex_unlock(&mutex_log);

								list_add(listaKernel, (void*) socketCliente);

								tamanio_pagina = malloc(sizeof(t_struct_numero));
								tamanio_pagina->numero = configuracion->marcoSize;
								socket_enviar(socketCliente, D_STRUCT_NUMERO, tamanio_pagina);
								free(tamanio_pagina);

								pthread_create(&nueva_solicitud, NULL, manejarKernel, socketCliente);

								break;
							case ES_CPU:

								pthread_mutex_lock(&mutex_log);
								log_info(logger,"Se conecto una CPU");
								pthread_mutex_unlock(&mutex_log);

								list_add(listaCpus, (void*) socketCliente);

								tamanio_pagina = malloc(sizeof(t_struct_numero));
								tamanio_pagina->numero = configuracion->marcoSize;
								socket_enviar(socketCliente, D_STRUCT_NUMERO, tamanio_pagina);
								free(tamanio_pagina);

								pthread_create(&nueva_solicitud, NULL, manejarCpu, socketCliente);

								break;

							default:

								pthread_mutex_lock(&mutex_log);
								log_error(logger,"No se pudo hacer el handshake");
								pthread_mutex_unlock(&mutex_log);
								//Ciero el FD del cliente que había aceptado
								close(socketCliente);
				}
			}

			free(structRecibido);
		}
	}
}

void manejarCpu(int socketCPU){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;
	bool seguimo = true;
	while(seguimo){

	if (socket_recibir(socketCPU,&tipoEstructura,&structRecibido) == -1) {
		pthread_mutex_lock(&mutex_log);
		log_info(logger,"El Cpu %d cerró la conexión.",socketCPU);
		pthread_mutex_unlock(&mutex_log);
		removerClientePorCierreDeConexion(socketCPU,listaCpus);
		seguimo=false;
	} else {

		switch(tipoEstructura){
		case D_STRUCT_LECT_VAR: ;

			t_struct_sol_lectura * direccionLeer = ((t_struct_sol_lectura* )structRecibido);

			t_resultadoLectura resultadoLecturaVar = leerPagina( direccionLeer->pagina,
					direccionLeer->PID, direccionLeer->offset, direccionLeer->contenido);

			if(resultadoLecturaVar.resultado){

				t_struct_numero * rtaLectVar = malloc(sizeof(t_struct_numero));
				rtaLectVar->numero = MEMORIA_OK;
				socket_enviar(socketCPU,D_STRUCT_NUMERO,rtaLectVar);

				rtaLectVar->numero=*(int *)resultadoLecturaVar.contenido;
				socket_enviar(socketCPU,D_STRUCT_NUMERO,rtaLectVar);

			} else {

				t_struct_numero * rtaLectVar = malloc(sizeof(t_struct_numero));
				rtaLectVar->numero = MEMORIA_ERROR;
				socket_enviar(socketCPU,D_STRUCT_NUMERO,rtaLectVar);

			}

			break;

		case D_STRUCT_SOL_ESCR: ;

			t_struct_sol_escritura * direccionEscribir = ((t_struct_sol_escritura* )structRecibido);

			bool resultadoEscribir = escribirPagina(direccionEscribir->pagina, direccionEscribir->PID,
					direccionEscribir->offset, direccionEscribir->tamanio, direccionEscribir->contenido);

			if(resultadoEscribir){

				t_struct_numero * rtaEscribir = malloc(sizeof(t_struct_numero));
				rtaEscribir->numero = MEMORIA_OK;
				socket_enviar(socketCPU,D_STRUCT_NUMERO,rtaEscribir);

			} else {

				t_struct_numero * rtaEscribir = malloc(sizeof(t_struct_numero));
				rtaEscribir->numero = MEMORIA_ERROR;
				socket_enviar(socketCPU,D_STRUCT_NUMERO,rtaEscribir);

			}

			break;

		case D_STRUCT_LECT: ;

			t_struct_sol_lectura * direccionLeerInst = ((t_struct_sol_lectura* )structRecibido);

			t_resultadoLectura resultadoLecturaInst = leerPagina( direccionLeerInst->pagina,
					direccionLeerInst->PID, direccionLeerInst->offset, direccionLeerInst->contenido);

			if(resultadoLecturaInst.resultado){

				t_struct_numero * rtaLectInst = malloc(sizeof(t_struct_numero));
				rtaLectInst->numero = MEMORIA_OK;
				socket_enviar(socketCPU,D_STRUCT_NUMERO,rtaLectInst);

				t_struct_string * rtaInstruc = malloc(sizeof(t_struct_string));
				rtaInstruc->string=(char *)resultadoLecturaInst.contenido;
				socket_enviar(socketCPU,D_STRUCT_STRING,rtaInstruc);

			} else {

				t_struct_numero * rtaLectInst = malloc(sizeof(t_struct_numero));
				rtaLectInst->numero = MEMORIA_ERROR;
				socket_enviar(socketCPU,D_STRUCT_NUMERO,rtaLectInst);

			}

			break;

		}

	}
	}
};

void manejarKernel(int socketKernel){

	bool seguimo = true;

	while(seguimo) {

		t_tipoEstructura tipoEstructura;
		void * structRecibido;

		if (socket_recibir(socketKernel,&tipoEstructura,&structRecibido) == -1) {

			log_info(logger,"El Kernel %d cerró la conexión.",socketKernel);
			removerClientePorCierreDeConexion(socketKernel,listaKernel);
			seguimo = false;
		} else {

			switch(tipoEstructura){
			case D_STRUCT_MALC:

				pthread_mutex_lock(&mutex_log);
				log_info(logger,"Se solicita crear segmento para el PID %i",
						((t_struct_malloc* )structRecibido)->PID);

				pthread_mutex_unlock(&mutex_log);
				int PID = ((t_struct_malloc* )structRecibido)->PID;
				int tamanioSegmento = ((t_struct_malloc* )structRecibido)->tamano_segmento;

				bool sePudoAsignar = reservarFramesProceso(PID,tamanioSegmento,1);

				//Le comunico al kernel si se pudo realizar operacion
				t_struct_numero* respuestaAsignacion = malloc(sizeof(t_struct_numero));
				respuestaAsignacion->numero = sePudoAsignar ? MEMORIA_OK : MEMORIA_ERROR;

				socket_enviar(socketKernel, D_STRUCT_NUMERO, respuestaAsignacion);

				pthread_mutex_lock(&mutex_log);
				if(!sePudoAsignar){
					log_error(logger,"No se pudo crear el segmento solicitado");
				} else {
					log_info(logger,"Se creo con exito el segmento solicitado");
				}
				pthread_mutex_unlock(&mutex_log);

				free(respuestaAsignacion);

				break;

			case D_STRUCT_ESCRITURA_CODIGO: ;

				t_struct_sol_escritura * solicitudEscritura = malloc(sizeof(t_struct_sol_escritura));

				solicitudEscritura = (t_struct_sol_escritura* )structRecibido;

				bool sePudoEscribir = escribirPagina(solicitudEscritura->pagina,solicitudEscritura->PID,
						solicitudEscritura->offset,solicitudEscritura->tamanio, solicitudEscritura->contenido);

				if(!sePudoEscribir){
					log_error(logger,"No se pudo escribir el codigo en memoria del proceso PID", solicitudEscritura->PID);
				} else {
					log_info(logger, "Se escribio correctamente el codigo del proceso PID %d en memoria", solicitudEscritura->PID);
				}

				free(solicitudEscritura);

				break;

			case D_STRUCT_LIBERAR_MEMORIA: ;

				t_struct_numero * pidFinalizar = malloc(sizeof(t_struct_numero));

				pidFinalizar = (t_struct_numero* )structRecibido;

				finalizarPrograma(pidFinalizar->numero);

				//Le comunico al kernel si se pudo realizar operacion
				t_struct_numero* respuestaFinalizar = malloc(sizeof(t_struct_numero));
				respuestaFinalizar->numero = MEMORIA_OK;

				socket_enviar(socketKernel, D_STRUCT_NUMERO, respuestaFinalizar);

				log_info(logger, "Se finalizo el proceso PID %d en memoria", pidFinalizar->numero);

				free(pidFinalizar);

				break;

			case D_STRUCT_COMPACTAR_HEAP:
			case D_STRUCT_LIBERAR_HEAP:
			case D_STRUCT_ESCRIBIR_HEAP: ;

				t_struct_sol_escritura * solicitudEscrituraHeap = malloc(sizeof(t_struct_sol_escritura));

				solicitudEscrituraHeap = (t_struct_sol_escritura* )structRecibido;

				bool sePudoEscribirHeap = escribirPagina(solicitudEscrituraHeap->pagina,solicitudEscrituraHeap->PID,
						solicitudEscrituraHeap->offset,solicitudEscrituraHeap->tamanio, solicitudEscrituraHeap->contenido);

				//Le comunico al kernel si se pudo realizar operacion
				t_struct_numero* respuestaHeap = malloc(sizeof(t_struct_numero));
				respuestaHeap->numero = sePudoEscribirHeap ? MEMORIA_OK : MEMORIA_ERROR;

				socket_enviar(socketKernel, D_STRUCT_NUMERO, respuestaHeap);

				if(!sePudoEscribirHeap){
					log_error(logger,"No se pudo escribir el codigo en memoria del proceso PID", solicitudEscrituraHeap->PID);
				} else {
					log_info(logger, "Se escribio correctamente el codigo del proceso PID %d en memoria", solicitudEscrituraHeap->PID);
				}

//				free(solicitudEscrituraHeap);
				free(respuestaHeap);
//				free(metadata);

				break;

			case D_STRUCT_LIBERAR_PAGINA: ;

				t_struct_sol_heap * liberarPaginaHeap = malloc(sizeof(t_struct_sol_heap));

				liberarPaginaHeap = (t_struct_sol_heap* )structRecibido;

				bool sePudoLiberar = liberarPagina(liberarPaginaHeap->pointer,liberarPaginaHeap->pid);

				//Le comunico al kernel si se pudo realizar operacion
				t_struct_numero* respuestaLiberarPagina = malloc(sizeof(t_struct_numero));
				respuestaLiberarPagina->numero = sePudoLiberar ? MEMORIA_OK : MEMORIA_ERROR;

				socket_enviar(socketKernel, D_STRUCT_NUMERO, respuestaLiberarPagina);

				if(!sePudoLiberar){
					log_error(logger,"No se pudo liberar la pagina de heap %d del PID %d", liberarPaginaHeap->pointer,liberarPaginaHeap->pid);
				} else {
					log_info(logger, "Se pudo liberar la pagina de heap %d del PID %d", liberarPaginaHeap->pointer,liberarPaginaHeap->pid);
				}

				free(liberarPaginaHeap);
				free(respuestaLiberarPagina);

				break;
			}
		}
	}
}

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
	int limit = configuracion->marcos;
	int tamanioFrame = configuracion->marcoSize;
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
	pthread_mutex_lock(&mutex_log);
	log_info(logger, "Estructuras Administrativas creadas exitosamente.\n");
	log_info(logger, "Las estructuras administrativas ocupan %d marcos de memoria.\n", framesTablaInvertida);
	pthread_mutex_unlock(&mutex_log);
}

int buscarEnTabla(int pagina, int pid){
	int i = aplicarFuncionHash(pid, pagina);
	int frameInicialBusqueda = i;
	int indice = -1;
	int cantidadFrames = configuracion->marcos;
	while(indice < 0){
		if(tablaInvertida[i].pid == pid && tablaInvertida[i].pagina == pagina){
			indice = i;
		} else {
			i = aplicarManejoColisiones(i);
			if(i==frameInicialBusqueda) break;
		}
	}
	return indice;
}

void escribirEnMemoria(int numeroFrame,void* contenido, int size, int offset){
	pthread_mutex_lock(&mutex_memoria);
	void * punteroFrame =	memoriaPrincipal + numeroFrame * configuracion->marcoSize;
	memcpy(punteroFrame + offset,contenido,size);
	pthread_mutex_lock(&mutex_log);
	log_info(logger,"Se escribio la pagina %d", numeroFrame);
	pthread_mutex_unlock(&mutex_log);
	pthread_mutex_unlock(&mutex_memoria);
}

void * leerMemoria(int pagina, int pid){
	int numeroFrame = buscarEnTabla(pagina,pid);
	aplicarRetardo();
	void * posicion = memoriaPrincipal + numeroFrame * configuracion->marcoSize;
	actualizarCache(pid,pagina,posicion);
	return posicion;
}

bool escribirPagina(int pagina, int pid, int offset, int tamanio, void * contenido){
	int numeroFrame = buscarEnTabla(pagina,pid);
	bool resultado;
	if(numeroFrame > -1){
		escribirEnMemoria(numeroFrame,contenido,tamanio,offset);
		resultado = true;
	} else {
		resultado = false;
	}
	return resultado;
}

bool liberarPagina(int pagina, int pid){
	//TODO REVISAR
	int numeroFrame = buscarEnTabla(pagina,pid);
	bool resultado;
	if(numeroFrame > -1){
		tablaInvertida[numeroFrame].pid=FRAME_LIBRE;
		tablaInvertida[numeroFrame].pagina=0;
		resultado = true;
	} else {
		resultado = false;
	}
	return resultado;
}

int cantidadFramesLibres(){
	int limite = configuracion->marcos;
	int i = 0;
	int framesLibres = 0;
	for(i = 0; i < limite; i++){
		if(tablaInvertida[i].pid == FRAME_LIBRE){
			framesLibres++;
		}
	}
	pthread_mutex_lock(&mutex_log);
	log_info(logger,"Quedan %d frames libres.", framesLibres);
	pthread_mutex_unlock(&mutex_log);
	return framesLibres;
}

void registrarUsoDeFrame(int pid,int numeroFrame, int paginaProceso){
	tablaInvertida[numeroFrame].pid = pid;
	tablaInvertida[numeroFrame].pagina = paginaProceso;
}

int obtenerPrimerFrameLibre(int pid, int pagina){
	int i;
	int primeroLibre = -1;
	int limite = configuracion->marcos;
	i = aplicarFuncionHash(pid, pagina);
	int frameInicialBusqueda = i;
	while(primeroLibre < 0){
		if(tablaInvertida[i].pid == FRAME_LIBRE){
			primeroLibre = i;
		} else {
			i = aplicarManejoColisiones(i);
			if(i==frameInicialBusqueda) break;
		}
	}
	return primeroLibre;
}

int obtenerPrimerosNFramesLibres(int pid, int cantidadDeFrames){
	int i;
	int contador = 0;
	i = aplicarFuncionHash(pid, 0);
	int frameInicialBusqueda = i;
	while(contador < cantidadDeFrames){
		if(tablaInvertida[i].pid == FRAME_LIBRE){
			contador++;
			i++;
		} else {
			i = aplicarManejoColisiones(i);
			contador = 0;
			if(i==frameInicialBusqueda) break;

		}
	}
	int indice = -1;
	if(contador == cantidadDeFrames){
		indice = i - contador;
	}
	return indice;
}

int buscarMaxPaginaProceso (int pid){

	int cant = 0;
	int indice;
	int cantidadFrames = configuracion->marcos;

	for (indice=0; indice < configuracion->marcos-1 ; indice ++){
		if(tablaInvertida[indice].pid == pid) cant++;
	}

	return cant;

}

bool reservarFramesProceso(int pid, int cantidadBytes, int bytesContiguos){ // 1 TRUE 0 FALSE
	int i = 0;
	int bytesPorFrame = configuracion->marcoSize;
	int framesNecesarios = cantidadBytes/bytesPorFrame;
	if(cantidadBytes%bytesPorFrame != 0){
		framesNecesarios++;
	}
	int numeroPaginaAsignar = buscarMaxPaginaProceso(pid);
	if(bytesContiguos > 0){
		int primerFrameLibre = obtenerPrimerosNFramesLibres(pid,framesNecesarios);
		if(primerFrameLibre > 0){ //Tiene N frames libres contiguos
			for(i = 0; i < framesNecesarios;i++){
				registrarUsoDeFrame(pid,primerFrameLibre+i,numeroPaginaAsignar);
				numeroPaginaAsignar++;
			}
			return true;
		} else {
			pthread_mutex_lock(&mutex_log);
			log_info(logger,"No se reservaron paginas para el proceso %d por que no se dispone de los %d frames libres contiguos necesarios.\n",pid,framesNecesarios);
			pthread_mutex_unlock(&mutex_log);
			return false;
		}
	} else {
		int framesLibres = cantidadFramesLibres();
		if(framesNecesarios <= framesLibres){
			for(i = 0; i < framesNecesarios;i++){
				int numeroFrame = obtenerPrimerFrameLibre(pid,i);
				registrarUsoDeFrame(pid,numeroFrame,numeroPaginaAsignar);
				numeroPaginaAsignar++;
			}
			return true;
		} else {
			pthread_mutex_lock(&mutex_log);
			log_info(logger,"No se reservaron paginas para el proceso %d por que no se dispones de %d frames libres.\n",pid,framesNecesarios);
			pthread_mutex_unlock(&mutex_log);
			return false;
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


t_resultadoLectura leerPagina(int pagina, int pid, int offset, int tamanio){
	pthread_mutex_lock(&mutex_memoria);
	int indiceCache = buscarProcesoCache(pagina, pid);
	t_resultadoLectura resultado;
	void * lectura;
	resultado.resultado = false;
	resultado.contenido = malloc(tamanio);
	if(indiceCache > -1){
		pthread_mutex_lock(&mutex_log);
		log_info(logger,"Se leyo la pagina %d del proceso %d, de la Cache.",pagina,pid);
		pthread_mutex_unlock(&mutex_log);
		lectura = cache[indiceCache].contenido;
		resultado.resultado = true;
		memcpy(resultado.contenido,lectura + offset,tamanio);
	} else {
		pthread_mutex_lock(&mutex_log);
		log_info(logger,"Se leyo la pagina %d del proceso %d, de la Memoria.",pagina,pid);
		pthread_mutex_unlock(&mutex_log);
		lectura = leerMemoria(pagina,pid);
		resultado.resultado = true;
		memcpy(resultado.contenido,lectura + offset,tamanio);
	}
	pthread_mutex_unlock(&mutex_memoria);
	return resultado;
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
	pthread_mutex_lock(&mutex_log);
	log_info(logger,"Aplicando retardo de %d milisegundos, debido a que el frame no estaba en cache.",configuracion->retardoMemoria);
	pthread_mutex_unlock(&mutex_log);
}

void establecerRetardoMemoria(int cantidad){
	configuracion->retardoMemoria = cantidad;
	pthread_mutex_lock(&mutex_log);
	log_info(logger,"Se cambio el retardo en milisegundos a d%.", configuracion->retardoMemoria);
	pthread_mutex_unlock(&mutex_log);
}

int obtenerLRUElementoCache(){
	int i = 0;
	int posicion = -1;

	for(i = 0; i < CANTIDAD_ELEMENTOS_CACHE; i++){
		if(cache[i].contadorDeUso == 0){
			posicion = i;
			break;
		}
	}

	if(posicion==-1){
		i = 0;
		int minimo = cache[i].contadorDeUso;
		for(i = 0; i < CANTIDAD_ELEMENTOS_CACHE; i++){
			if(cache[i].contadorDeUso < minimo){
				minimo = cache[i].contadorDeUso;
				posicion = i;
			}
		}
	}

	if(cache[posicion].contadorDeUso > 0){
		pthread_mutex_lock(&mutex_log);
		log_info(logger,"Se reemplaza al proceso con PID %d, del frame %d de la Cache por LRU.", cache[posicion].pid, posicion);
		pthread_mutex_unlock(&mutex_log);
	}
	return posicion;
}

int calcularAparicionesCache(int pid){
	int i = 0;
	int cantidad = 0;
	for(i = 0; i < CANTIDAD_ELEMENTOS_CACHE; i++){
		if(cache[i].pid == pid){
			cantidad++;
		}
	}
	return cantidad;
}

void actualizarCache(int pid,int pagina,void* punteroMarco){
	int apariciones = calcularAparicionesCache(pid);
	if(apariciones < configuracion->cacheXProc){
		int indiceCache = obtenerLRUElementoCache();
		cache[indiceCache].contadorDeUso = apariciones + 1;
		cache[indiceCache].contenido = punteroMarco;
		cache[indiceCache].pagina = pagina;
		cache[indiceCache].pid = pid;
		pthread_mutex_lock(&mutex_log);
		log_info(logger,"Se escribio en el frame %d de la Cache, el PID %d, con la pagina %d.", indiceCache, pid, pagina);
		pthread_mutex_unlock(&mutex_log);
	} else {
		pthread_mutex_lock(&mutex_log);
		log_info(logger,"El proceso con PID %d no se guardo en la cache por alcanzar el maximo de Procesos x Cache.", pid);
		pthread_mutex_unlock(&mutex_log);
	}
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
	int limite = configuracion->marcos;
	for(i = 0; i < limite; i++){
		if(tablaInvertida[i].pid == pid){
			tablaInvertida[i].pid = FRAME_LIBRE;
		}
	}
	borrarProcesoCache(pid);
	pthread_mutex_lock(&mutex_log);
	log_info(logger,"Se desasignaron todos los frames asignados al proceso %d",pid);
	pthread_mutex_unlock(&mutex_log);
}

void imprimirTablaPaginas(){
	int i;
	int limite = configuracion->marcos;
	for(i = 0; i < limite; i++){
		printf("Frame: %d  PID: %d  #Pagina: %d\n",tablaInvertida[i].frame,tablaInvertida[i].pid,tablaInvertida[i].pagina);
	}
}

void imprimirCache(){
	int i;
	int limite = CANTIDAD_ELEMENTOS_CACHE;
	for(i = 0; i < limite; i++){
		printf("PID: %d  #Pagina: %d \n",cache[i].pid,cache[i].pagina);
	}
}

int calcularFramesOcupados(int pid){
	int i;
	int limite = configuracion->marcos;
	int contador = 0;
	for(i = 0; i < limite; i++){
		if(tablaInvertida[i].pid == pid){
			contador++;
		}
	}
	return contador;
}

t_list * obtenerProcesosActivos(){
	int i;
	int limite = configuracion->marcos;
	t_list * procesosActivos = list_create();
	for(i = 0; i < limite; i++){

		bool contiene_pid(int nuevoPid){
			return (nuevoPid == tablaInvertida[i].pid);
		}
		if(tablaInvertida[i].pid!=-1 && tablaInvertida[i].pid!=0 && !list_find(procesosActivos,(void*)contiene_pid)){
			list_add(procesosActivos,tablaInvertida[i].pid);
		}
	}
	return procesosActivos;
}

void dumpMemoria(){
	FILE *f = fopen("dump.txt","w");

	fprintf(f,"Cache\n");
	fprintf(f,"\n");
	int i;
	int limite = CANTIDAD_ELEMENTOS_CACHE;
	for(i = 0; i < limite; i++){
		fprintf(f,"PID: %d  #Pagina: %d \n",cache[i].pid,cache[i].pagina);
	}

	fprintf(f,"\n");
	fprintf(f,"Tabla de Páginas\n");
	fprintf(f,"\n");
	limite = configuracion->marcos;
	for(i = 0; i < limite; i++){
		fprintf(f,"Frame: %d  PID: %d  #Pagina: %d\n",tablaInvertida[i].frame,tablaInvertida[i].pid,tablaInvertida[i].pagina);
	}

	fprintf(f,"\n");
	fprintf(f,"Procesos Activos\n");
	fprintf(f,"\n");
	t_list * procesosActivos = obtenerProcesosActivos();
	for(i = 0; i < list_size(procesosActivos); i++){
		fprintf(f,"PID: %d\n",list_get(procesosActivos,i));
	}

	fprintf(f,"(Bonus Track) Retardo de Memoria: %d milisegundos.\n", configuracion->retardoMemoria);

	fclose(f);

}

void manejoConsola(){

	printf("Comandos disponibles:\n");
	printf("retardo: Cambia el retardo para leer de memoria.\n");
	printf("dump: Generar reporte con detalles de cache, estructuras y contenido.\n");
	printf("size:Memory o PID con el process id imprime datos relacionados al tamaño.\n");
	printf("flush: Limpiar la cache.\n");
	printf("exit: Cerrar la consola.\n");
	printf("\n");

	while(consolaConectada){

		puts("Ingrese algún comando no mayor a 50 caracteres\n");
		char * value = malloc(50);
		scanf("%s", value);

		switch(commandParser(value)){
			case 1:;
				puts("Ingrese el nuevo retardo en milisegundos...\n");
				char * path = malloc(200);
				scanf("%s", path);

				pthread_t hiloPrograma;

				establecerRetardoMemoria(atoi(path));

				free(path);

				break;
			case 2:
				pthread_mutex_lock(&mutex_log);
				log_info(logger,"Se hizo dump de toda la memoria.");
				pthread_mutex_unlock(&mutex_log);

				printf("\n");
				printf("Cache:\n");
				printf("\n");
				imprimirCache();

				printf("\n");
				printf("Tabla de Páginas:\n");
				printf("\n");
				imprimirTablaPaginas();

				printf("\n");
				printf("Procesos Activos:\n");
				printf("\n");
				t_list *  procesosActivos = obtenerProcesosActivos();
				int i = 0;
				for(i = 0; i < list_size(procesosActivos); i++){
					printf("PID: %d\n",list_get(procesosActivos,i));
				}

				printf("(Bonus Track) Retardo de Memoria: %d milisegundos.\n", configuracion->retardoMemoria);

				dumpMemoria();

				break;
			case 3:

				puts("Ingrese opción MEMORY o PID...\n");
				char * otro = malloc(200);
				scanf("%s", otro);

				switch(commandParser(otro)){
					case 6:
						pthread_mutex_lock(&mutex_log);
						log_info(logger,"Se consulto la cantidad de frames de memoria.");
						pthread_mutex_unlock(&mutex_log);

						//consultar Memoria
						printf("Cantidad de Frames: %d.\n", configuracion->marcos);
						printf("Cantidad de Frames Ocupados: %d.\n", configuracion->marcos - cantidadFramesLibres());
						printf("Cantidad de Frames Libres: %d.\n", cantidadFramesLibres());

						break;
					case 7:
						puts("Ingrese PID\n");
						char * pid = malloc(200);
						scanf("%s", pid);

						pthread_mutex_lock(&mutex_log);
						log_info(logger,"Se contulto el espacio ocupado por le proceso con PID: %s.",pid);
						pthread_mutex_unlock(&mutex_log);

						//consultar PID
						int total = calcularFramesOcupados(atoi(pid));
						printf("Cantidad de Frames Ocupados por el Proceso: %d.\n", total);
						printf("Cantidad de Bytes Ocupados por el Proceso: %d.\n", total * configuracion->marcoSize);

						free(pid);
						break;
					default:
						printf("Comando invalido...\n");
						break;
				}

				free(otro);

				break;
			case 4:

				printf("Se vacia la Cache...\n");

				pthread_mutex_lock(&mutex_log);

				log_info(logger,"Se vacia la cache.");

				pthread_mutex_unlock(&mutex_log);

				vaciarCache();
				crearCache();

				break;

			case 5:

				pthread_mutex_lock(&mutex_log);

				log_info(logger,"Se envió la instrucción para desconectar la consola.");

				pthread_mutex_unlock(&mutex_log);

				consolaConectada=0;
				break;
			default:
				printf("Comando invalido...\n");
				break;
			}
		free(value);
	}
}

int commandParser(char* command){

	string_to_upper(command);

	if(strcmp(command, "RETARDO") == 0){
		return 1;
	} else if (strcmp(command, "DUMP") == 0){
		return 2;
	} else if(strcmp(command, "SIZE") == 0){
		return 3;
	} else if (strcmp(command, "FLUSH") == 0){
		return 4;
	} else if (strcmp(command, "EXIT") == 0){
		return 5;
	} else if (strcmp(command, "MEMORY") == 0){
		return 6;
	} else if (strcmp(command, "PID") == 0){
		return 7;
	} else {
		return 8;
	}

}

int aplicarFuncionHash(int pid, int pagina){
	return (pagina + pid) % configuracion->marcos;
}

int aplicarManejoColisiones(int frame){
	if(frame < (configuracion->marcos - 1)){
		return frame + 1;
	} else {
		return 0;
	}
}


