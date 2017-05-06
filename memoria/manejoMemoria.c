/*
 * manejoMemoria.c
 *
 *  Created on: 5/5/2017
 *      Author: utnso
 */

#include <estructuras.h>
#include <commons/collections/list.h>

void* memoriaPrincipal;

typedef unsigned char frame[500];

void* crearMemoriaPrincipal(int frames, int frameSize) {
	void* memoriaPrincipal = malloc(frames*frameSize);
	return memoriaPrincipal;
}

void liberarMemoriaPrincipal(void* punteroMemoriaPrincipal){
	free(punteroMemoriaPrincipal);
}

void crearEstructurasAdministrativas(void* punteroMemoriaPrincipal,int frames){
	int i;
	memoriaPrincipal = punteroMemoriaPrincipal;
	t_filaTablaInvertida* fila = (t_filaTablaInvertida*)punteroMemoriaPrincipal;
	for(i = 0; i < frames; i++){
		fila[i].pid = 0;
		fila[i].pagina = i;
	}
}

void escribirEnMemoria(int pagina, char* texto){
	frame* marcosMemoria = &memoriaPrincipal;
	memcpy(marcosMemoria[pagina],texto,30);
}

void* leerPagina(int pagina){
	frame* marcosMemoria = &memoriaPrincipal;
	return marcosMemoria[pagina];
}

