#include "cpuHelper.h"
#include "primitivas.h"

AnSISOP_funciones funciones = {
	.AnSISOP_definirVariable 			= definirVariable,
	.AnSISOP_obtenerPosicionVariable 	= obtenerPosicionVariable,
	.AnSISOP_dereferenciar				= dereferenciar,
	.AnSISOP_asignar					= asignar,
	.AnSISOP_obtenerValorCompartida		= obtenerValorCompartida,
	.AnSISOP_asignarValorCompartida		= asignarValorCompartida,
	.AnSISOP_irAlLabel					= irAlLabel,
	.AnSISOP_llamarSinRetorno			= llamarSinRetorno,
	.AnSISOP_llamarConRetorno			= llamarConRetorno,
	.AnSISOP_finalizar					= finalizar,
	.AnSISOP_retornar					= retornar,
};

AnSISOP_kernel funciones_kernel = {
		.AnSISOP_wait					= wait,
		.AnSISOP_signal					= signal,
		.AnSISOP_reservar				= reservar,
		.AnSISOP_liberar				= liberar,
		.AnSISOP_abrir					= abrir,
		.AnSISOP_cerrar					= cerrar,
		.AnSISOP_borrar					= borrar,
		.AnSISOP_leer					= leer,
		.AnSISOP_escribir				= escribir,
		.AnSISOP_moverCursor			= moverCursor,
};

int main(int arc, char * argv[]) {

	//Genera archivo log para poder escribir el trace de toda la ejecución
	logger = malloc(sizeof(t_log));
	crearLog("/CPU");

	//Levanta la configuración del proceso CPU
	configuracion = cargarConfiguracion();

	int socketCPU = conectarAKernel();

	return 0;

}
