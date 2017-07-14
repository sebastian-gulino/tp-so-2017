#include "manejoMemoria.h"

#define CANTIDAD_ELEMENTOS_CACHE 15

void * memoriaPrincipal;

void cargarConfiguracion() {

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

	log_info(logger,"El Puerto es %d\n",configuracion->puerto);
	log_info(logger,"La cantidad de Marcos es %d\n",configuracion->marcos);
	log_info(logger,"El tamaño de cada Marco es %d\n",configuracion->marcoSize);
	log_info(logger,"Las entradas en Cache son %d\n",configuracion->entradasCache);
	log_info(logger,"La cantidad maxima de de entradas de la cache asignables a cada programa es %d\n",configuracion->cacheXProc);
	log_info(logger,"El reemplazo de cache es %d\n",configuracion->reemplazoCache);
	log_info(logger,"El retardo de la Memoria es %d\n",configuracion->retardoMemoria);
	log_info(logger,"La cantidad de paginas asignada al Stack es %d\n",configuracion->stackSize);

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

								log_info(logger,"Se conecto el Kernel");

								list_add(listaKernel, (void*) socketCliente);

								tamanio_pagina = malloc(sizeof(t_struct_numero));
								tamanio_pagina->numero = configuracion->marcoSize;
								socket_enviar(socketCliente, D_STRUCT_NUMERO, tamanio_pagina);
								free(tamanio_pagina);

								pthread_create(&nueva_solicitud, NULL, manejarKernel, socketCliente);

								break;
							case ES_CPU:

								log_info(logger,"Se conecto una CPU");

								list_add(listaCpus, (void*) socketCliente);

								tamanio_pagina = malloc(sizeof(t_struct_numero));
								tamanio_pagina->numero = configuracion->marcoSize;
								socket_enviar(socketCliente, D_STRUCT_NUMERO, tamanio_pagina);
								free(tamanio_pagina);

								pthread_create(&nueva_solicitud, NULL, manejarCpu, socketCliente);

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

void manejarCpu(int socketCPU){

	t_tipoEstructura tipoEstructura;
	void * structRecibido;

	if (socket_recibir(socketCPU,&tipoEstructura,&structRecibido) == -1) {
		log_info(logger,"El Cpu %d cerró la conexión.",socketCPU);
		removerClientePorCierreDeConexion(socketCPU,listaCpus);
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
				direccionEscribir->offset, sizeof(int), direccionEscribir->contenido);

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

		}

	}
};

void manejarKernel(int socketKernel){

	while(1) {

		t_tipoEstructura tipoEstructura;
		void * structRecibido;

		if (socket_recibir(socketKernel,&tipoEstructura,&structRecibido) == -1) {

			log_info(logger,"El Kernel %d cerró la conexión.",socketKernel);
			removerClientePorCierreDeConexion(socketKernel,listaKernel);

		} else {

			switch(tipoEstructura){
			case D_STRUCT_MALC:

				log_info(logger,"Se solicita crear segmento para el PID %i",
						((t_struct_malloc* )structRecibido)->PID);

				int PID = ((t_struct_malloc* )structRecibido)->PID;
				int tamanioSegmento = ((t_struct_malloc* )structRecibido)->tamano_segmento;

				bool sePudoAsignar = reservarFramesProceso(PID,tamanioSegmento,0);

				//Le comunico al kernel si se pudo realizar operacion
				t_struct_numero* respuestaAsignacion = malloc(sizeof(t_struct_numero));
				respuestaAsignacion->numero = sePudoAsignar ? MEMORIA_OK : MEMORIA_ERROR;

				socket_enviar(socketKernel, D_STRUCT_NUMERO, respuestaAsignacion);

				if(!sePudoAsignar){
					log_error(logger,"No se pudo crear el segmento solicitado");
				} else {
					log_info(logger,"Se creo con exito el segmento solicitado");
				}

				free(respuestaAsignacion);

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
	log_info(logger, "Estructuras Administrativas creadas exitosamente.\n");
	log_info(logger, "Las estructuras administrativas ocupan %d marcos de memoria.\n", framesTablaInvertida);
}

int buscarPaginaMemoria(int pagina, int pid){
	int i = 0;
	int indice = -1;
	int cantidadFrames = configuracion->marcos;
	while(indice < 0 && i < cantidadFrames){
		if(tablaInvertida[i].pid == pid && tablaInvertida[i].pagina == pagina){
			indice = i;
		}
		i++;
	}
	return indice;
}

void escribirEnMemoria(int numeroFrame,void* contenido, int size, int offset){
	void* punteroFrame = memoriaPrincipal + numeroFrame * configuracion->marcoSize;
	memcpy(punteroFrame + offset,contenido,size);
	log_info(logger,"Se escribio la pagina %d", numeroFrame);
}

bool escribirPagina(int pagina, int pid, int offset, int tamanio, void * contenido){
	int numeroFrame = buscarPaginaMemoria(pagina,pid);
	bool resultado;
	if(numeroFrame > -1){
		escribirEnMemoria(numeroFrame,contenido,tamanio,offset);
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
	int limite = configuracion->marcos;
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
	int limite = configuracion->marcos;
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

bool reservarFramesProceso(int pid, int cantidadBytes, int bytesContiguos){ // 1 TRUE 0 FALSE
	int i = 0;
	int bytesPorFrame = configuracion->marcoSize;
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
			return true;
		} else {
			log_info(logger,"No se reservaron paginas para el proceso %d por que no se dispone de los %d frames libres contiguos necesarios.\n",pid,framesNecesarios);
			return false;
		}
	} else {
		int framesLibres = cantidadFramesLibres();
		if(framesNecesarios <= framesLibres){
			for(i = 0; i < framesNecesarios;i++){
				int numeroFrame = obtenerPrimerFrameLibre();
				registrarUsoDeFrame(pid,numeroFrame,i+1);
			}
			return true;
		} else {
			log_info(logger,"No se reservaron paginas para el proceso %d por que no se dispones de %d frames libres.\n",pid,framesNecesarios);
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


void * leerMemoria(int pagina, int pid){
	int numeroFrame = buscarPaginaMemoria(pagina,pid);
	aplicarRetardo();
	void * posicion = memoriaPrincipal + numeroFrame * configuracion->marcoSize;
	actualizarCache(pid,pagina,posicion);
	return posicion;
}


t_resultadoLectura leerPagina(int pagina, int pid, int offset, int tamanio){
	int indiceCache = buscarProcesoCache(pagina, pid);
	t_resultadoLectura resultado;
	void * lectura;
	resultado.resultado = false;
	resultado.contenido = malloc(tamanio);
	if(indiceCache > -1){
		printf("Se lee cache\n");
		lectura = cache[indiceCache].contenido;
		resultado.resultado = true;
		memcpy(resultado.contenido,lectura + offset,tamanio);
	} else {
		printf("Se lee memoria\n");
		lectura = leerMemoria(pagina,pid);
		resultado.resultado = true;
		memcpy(resultado.contenido,lectura + offset,tamanio);
	}
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
	int limite = configuracion->marcos;
	for(i = 0; i < limite; i++){
		if(tablaInvertida[i].pid == pid){
			tablaInvertida[i].pid = FRAME_LIBRE;
		}
	}
	borrarProcesoCache(pid);
	log_info(logger,"Se desasignaron todos los frames asignados al proceso %d",pid);
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


