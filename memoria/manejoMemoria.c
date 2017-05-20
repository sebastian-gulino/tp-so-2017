/*
 * manejoMemoria.c
 *
 *  Created on: 5/5/2017
 *      Author: utnso
 */

#include <estructuras.h>
#include <commons/collections/list.h>
#include <unistd.h>
#include <commons/log.h>

#define CANTIDAD_ELEMENTOS_CACHE 15

void* memoriaPrincipal;

typedef unsigned char frame[500];

typedef struct config_t {

	char * puerto;
	char * marcos;
	char * marcoSize;
	char * entradasCache;
	char * cacheXProc;
	char * reemplazoCache;
	char * retardoMemoria;
	char * stackSize;

} t_configuracion;

typedef struct cache_t {
	int pid;
	int pagina;
	void* contenido;
} t_cache;

typedef struct resultado_busqueda_cache_t {
	int cantidad;
	int indices[15];
} t_resultado_busqueda_cache;

t_cache* cache;

t_configuracion configuracion;

t_log* logger;

char buffLog[80];

t_filaTablaInvertida* tablaInvertida;

void* memoriaPrincipal;

void crearLog(){

	char cad[80];
	char *directorioActual = getcwd(NULL, 0);

	strcat(cad,directorioActual);
	strcat(cad,"/memoria.log");

	logger = log_create(cad, "MEMORIA", 0, LOG_LEVEL_TRACE);
	log_info(logger,"Comienza a ejecutar el proceso MEMORIA");
}


void cargarConfiguracion(void) {
	t_config * config;

	config = config_create("./config.txt");

	crearLog();

	configuracion.puerto = strdup(config_get_string_value(config, "PUERTO"));
	configuracion.marcos = strdup(config_get_string_value(config, "MARCOS"));
	configuracion.marcoSize = strdup(config_get_string_value(config, "MARCO_SIZE"));
	configuracion.entradasCache = strdup(config_get_string_value(config, "ENTRADAS_CACHE"));
	configuracion.cacheXProc = strdup(config_get_string_value(config, "CACHE_X_PROC"));
	configuracion.reemplazoCache = strdup(config_get_string_value(config, "REEMPLAZO_CACHE"));
	configuracion.retardoMemoria = strdup(config_get_string_value(config, "RETARDO_MEMORIA"));
	configuracion.stackSize = strdup(config_get_string_value(config, "STACK_SIZE"));

	log_info(logger,"El Puerto es %s\n",configuracion.puerto);
	log_info(logger,"La cantidad de Marcos es %s\n",configuracion.marcos);
	log_info(logger,"El tamaño de cada Marco es %s\n",configuracion.marcoSize);
	log_info(logger,"Las entradas en Cache son %s\n",configuracion.entradasCache);
	log_info(logger,"La cantidad maxima de de entradas de la cache asignables a cada programa es %s\n",configuracion.cacheXProc);
	log_info(logger,"El reemplazo de cache es %s\n",configuracion.reemplazoCache);
	log_info(logger,"El retardo de la Memoria es %s\n",configuracion.retardoMemoria);
	log_info(logger,"La cantidad de paginas asignada al Stack es %s\n",configuracion.stackSize);
}

void crearMemoriaPrincipal() {
	memoriaPrincipal = malloc(atoi(configuracion.marcos)*atoi(configuracion.marcoSize));
}

void liberarMemoriaPrincipal(){
	free(memoriaPrincipal);
}

void crearEstructurasAdministrativas(){
	int i;
	memoriaPrincipal = &memoriaPrincipal;
	tablaInvertida = (t_filaTablaInvertida*)memoriaPrincipal;
	int limit = atoi(configuracion.marcos);
	for(i = 0; i < limit; i++){
		tablaInvertida[i].pid = 0;
		tablaInvertida[i].frame = i;
	}
	int tamanioFrame = atoi(configuracion.marcoSize);
	int bytesTablaInvertida = sizeof(t_filaTablaInvertida)*500;
	int framesTablaInvertida = bytesTablaInvertida/tamanioFrame;
	if(bytesTablaInvertida%tamanioFrame != 0){
		framesTablaInvertida++;
	}
	for(i = 0; i < framesTablaInvertida; i++){
		tablaInvertida[i].pid = -1;
	}
	log_info(logger, "Estructuras Administrativas creadas exitosamente.");
	log_info(logger, "Las estructuras administrativas ocupan %d marcos de memoria", framesTablaInvertida);
}

void escribirPagina(int pagina, void* bytes, int size, int offset){
	frame* marcosMemoria = &memoriaPrincipal;
	unsigned char* punteroOffset = marcosMemoria[pagina];
	memcpy(punteroOffset[offset],bytes,size);
	log_debug(logger,"Se escribio la pagina %d", pagina);
}

int cantidadFramesLibres(){
	int limite = atoi(configuracion.marcos);
	int i, framesLibres;
	for(i = 0; i < limite; i++){
		if(tablaInvertida[i].pid == 0){
			framesLibres++;
		}
	}
	log_debug("Quedan %d frames libres.", framesLibres);
	return framesLibres;
}

void registrarUsoDeFrame(int pid,int numeroFrame, int paginaProceso){
	tablaInvertida[numeroFrame].pid = pid;
	tablaInvertida[numeroFrame].pagina = paginaProceso;
}

int obtenerPrimerFrameLibre(){
	int i;
	int primeroLibre = -1;
	int limite = atoi(configuracion.marcos);
	while(primeroLibre < 0 && i < limite){
		if(tablaInvertida[i].pid == 0){
			primeroLibre = i;
		}
		i++;
	}
	return primeroLibre;
}


void escribirEnMemoria(int pid, void* contenido, int cantidadBytes){
	int bytesPorFrame = atoi(configuracion.marcoSize);
	int framesNecesarios = cantidadBytes/bytesPorFrame;
	if(cantidadBytes%bytesPorFrame != 0){
		framesNecesarios++;
	}
	int framesLibres = cantidadFramesLibres();
	if(framesNecesarios <= framesLibres){
		int cantidadBytesLibres = bytesPorFrame * framesLibres;
		if(cantidadBytes <= cantidadBytesLibres){
			int i;
			for(i = 0; i < framesLibres;i++){
				int numeroFrame = obtenerPrimerFrameLibre();
				registrarUsoDeFrame(pid,numeroFrame,i+1);
				if(cantidadBytes <= bytesPorFrame){
					//ESCRIBIR
				} else {
					cantidadBytes = cantidadBytes - bytesPorFrame;
				}
			}
		} else {
			log_debug(logger,"No se escribio en memoria por que lo que se queria escribir era muy grande.");
		}
	} else {
		log_debug(logger,"No se escribio en memoria por falta de paginas libres.");
	}
}


t_resultado_busqueda_cache buscarPIDCache(int pid){
	int i;
	t_resultado_busqueda_cache resultado;
	resultado.cantidad = 0;
	for(i = 0;i < CANTIDAD_ELEMENTOS_CACHE; i++){
		if(cache[i].pid == pid){
			resultado.indices[resultado.cantidad] = i;
			resultado.cantidad++;
		}
	}
	return resultado;
}

void* leerPagina(int pagina, int pid){
	frame* marcosMemoria = &memoriaPrincipal;
	actualizarCache(pid,pagina,marcosMemoria[pagina]);
	aplicarRetardo(1);
	return marcosMemoria[pagina];
}

void crearCache(){
	cache = malloc(sizeof(t_cache) * CANTIDAD_ELEMENTOS_CACHE);
	int i;
	for(i = 0; i < CANTIDAD_ELEMENTOS_CACHE; i++){
		cache[i].pid = 0;
	}
}

void vaciarCache(){
	free(cache);
}

void aplicarRetardo(int retardo){
	sleep(retardo);
	log_info(logger,"Durmiendo %d segundos...zzzzzzz",retardo);
}

void actualizarCache(int pid,int pagina,void* punteroMarco){

}

void asignarPaginasProceso(int pid, int numeroFramesPedidos){
	int framesLibres = cantidadFramesLibres();
	if(numeroFramesPedidos <= framesLibres){
		int i;
		for(i = 0;i < numeroFramesPedidos; i++){
			int indiceFrameLibre = obtenerPrimerFrameLibre();
			tablaInvertida[indiceFrameLibre].pid = pid;
		}
	}
	log_debug(logger,"Se reservaron %d frames para el proceso %d",numeroFramesPedidos,pid);
}

void finalizarPrograma(int pid){
	int i;
	int limite = atoi(configuracion.marcos);
	for(i = 0; i < limite; i++){
		if(tablaInvertida[i].pid == pid){
			tablaInvertida[i].pid = 0;
		}
	}
	t_resultado_busqueda_cache resultado = buscarPIDCache(pid);
	for(i = 0; i < resultado.cantidad; i++){
		cache[resultado.indices[i]].pid = 0;
	}
	log_debug(logger,"Se desasignaron todos los frames asignados al proceso %d",pid);
}

void imprimirTablaPaginas(){
	int i;
	int limite = atoi(configuracion.marcos);
	for(i = 0; i < limite; i++){
		printf("Frame: %d  PID: %d  #Pagina: %d",tablaInvertida[i].pagina,tablaInvertida[i].pid,tablaInvertida[i].pagina);
	}
}

void atenderPedidoEscritura(char * solicitante, int pid, int cantidadFrames){
	switch(solicitante){
		case "kernel":
			//reservar memoria para codigo
			asignarPaginasProceso(pid,cantidadFrames);
			break;
		case "cpu":
			//reservar memoria para stack o heap
			asignarPaginasProceso(pid,cantidadFrames);
			//escribir en stack o heap
			break;
		default:
			break;
	}
}
