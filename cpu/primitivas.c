#include "primitivas.h"

bool termino = false;

//TODO borrar. constantes de prueba
static const int CONTENIDO_VARIABLE = 20;
static const int POSICION_MEMORIA = 0x10;

//FUNCIONES
t_puntero definirVariable(t_nombre_variable variable) {
	//TODO borrar. implementación de prueba
	printf("definir la variable %c\n", variable);
	return POSICION_MEMORIA;
}

t_puntero obtenerPosicionVariable(t_nombre_variable variable) {
	//TODO borrar. implementación de prueba
	printf("Obtener posicion de %c\n", variable);
	return POSICION_MEMORIA;
}

t_valor_variable dereferenciar(t_puntero puntero) {
	//TODO borrar. implementación de prueba
	printf("Dereferenciar %d y su valor es: %d\n", puntero, CONTENIDO_VARIABLE);
	return CONTENIDO_VARIABLE;
}

void asignar(t_puntero puntero, t_valor_variable variable) {
	//TODO borrar. implementación de prueba
	printf("Asignando en %d el valor %d\n", puntero, variable);
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
	//TODO borrar. implementación de prueba
	termino = true;
	printf("Finalizar\n");
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

bool terminoElPrograma() {
	return termino;
}
