/*
 * sockets.c
 *
 *  Created on: 14/4/2017
 *      Author: utnso
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char * empaquetar(char * inicioMensaje, char * mensaje){
	int size = strlen(mensaje);
	char * buffer = malloc(size + 1);
	memcpy(buffer, &inicioMensaje,1);
	strcat(buffer,mensaje);
	return buffer;
}

char * desempaquetar(char * mensajeEmpaquetado){
	int tamanio = strlen(mensajeEmpaquetado);
	char * subbuff = malloc(tamanio + 1);
	memcpy( subbuff, &mensajeEmpaquetado[1], tamanio );
	subbuff[tamanio-1] = '\0';
	return subbuff;
}

char * procesoEmisor(char * mensajeEmpaquetado){
	char * proceso;
	switch(mensajeEmpaquetado[0]){
		case 'C':
			proceso = "Consola";
			break;
		case 'M':
			proceso = "Memoria";
			break;
		case 'P':
			proceso = "CPU";
			break;
		case 'F':
			proceso = "FileSystem";
			break;
		default:
			break;
	}
	return proceso;
}

