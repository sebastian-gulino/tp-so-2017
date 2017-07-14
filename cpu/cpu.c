#include "primitivas.h"

AnSISOP_funciones funcionesAnsisop = {
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
		.AnSISOP_wait					= s_wait,
		.AnSISOP_signal					= s_signal,
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

	// Genera archivo log para poder escribir el trace de toda la ejecución
	crearLog("/CPU");

	// Levanta la configuración del proceso CPU
	cargarConfiguracion();

	inicializarEstructuras();

	// Conecta la cpu incorporada al Kernel
	socketKernel = conectarAKernel();

	// Conecta la cpu incorporada a la Memoria
	socketMemoria = conectarAMemoria();

	// Defino el handler que manejará la señal SIGUSR1 para desconectar la CPU
	signal(SIGUSR1, manejarSignal);

	// Comienza a iterar para escuchar los pedidos de ejecución del kernel
	recibirProcesoKernel(funcionesAnsisop,funciones_kernel);

	return 0;

}


