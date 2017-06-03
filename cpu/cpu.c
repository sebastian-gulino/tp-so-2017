#include <parser/metadata_program.h>
#include <parser/parser.h>
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

static const char* PROGRAMA =
"begin\n"
"variables a, b\n"
"a = 3\n"
"b = 5\n"
"a = b + 12\n"
"end\n"
"\n";

char *const conseguirDatosDeLaMemoria(char *start, t_puntero_instruccion offset, t_size i);

int main(int arc, char * argv[]) {

	//Genera archivo log para poder escribir el trace de toda la ejecución
	logger = malloc(sizeof(t_log));
	crearLog("/CPU");

	//Levanta la configuración del proceso CPU
	configuracion = cargarConfiguracion();

	//Conecta la cpu incorporada al Kernel
	int socketKernel = conectarAKernel();

	//Conecta la cpu incorporada a la Memoria
	int socketMemoria = conectarAMemoria();

	return 0;

}

int pruebaDeEjecucion() {

	char *programa = strdup(PROGRAMA);

	t_metadata_program *metadata = metadata_desde_literal(programa);

	int programCounter = 0;

	while (!terminoElPrograma()) {

		char* const linea = conseguirDatosDeLaMemoria(programa,
								metadata->instrucciones_serializado[programCounter].start,
								metadata->instrucciones_serializado[programCounter].offset);

		printf("\t Evaluando -> %s", linea);

		analizadorLinea(linea, &funciones, &funciones_kernel);
		free(linea);
		programCounter++;
	}

	metadata_destruir(metadata);
	printf("================\n");
	return EXIT_SUCCESS;
}

char *const conseguirDatosDeLaMemoria(char *programa, t_puntero_instruccion inicioDeLaInstruccion, t_size tamanio) {
	char *aRetornar = calloc(1, 100);
	memcpy(aRetornar, programa + inicioDeLaInstruccion, tamanio);
	return aRetornar;
}
