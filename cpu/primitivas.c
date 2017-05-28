#include "primitivas.h"

//FUNCIONES
t_puntero definirVariable(t_nombre_variable variable) {
	//TODO implementar
	return NULL;
}

t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	//TODO implementar
	return NULL;
}

t_valor_variable dereferenciar(t_puntero puntero) {
	//TODO implementar
	return NULL;
}

void asignar(t_puntero puntero, t_valor_variable variable) {
	//TODO implementaR
}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable) {
	//TODO implementar
	return NULL;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor) {
	//TODO implementar
	return NULL;
}

void irAlLabel(t_nombre_etiqueta etiqueta) {
	//TODO implementar
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta) {
	//TODO implementar
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar) {
	//TODO implementar
}

void finalizar(void)  {
	//TODO implementar
}

void retornar(t_valor_variable retorno) {
	//TODO implementar
}


//FUNCIONES KERNEL
void wait(t_nombre_semaforo identificador_semaforo) {
	//TODO implementar
}

void signal(t_nombre_semaforo identificador_semaforo) {
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

