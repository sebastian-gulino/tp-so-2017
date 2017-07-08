#include "manejoMemoria.h"

int main(void) {

	setvbuf (stdout, NULL, _IONBF, 0);

	//Genera archivo log pars poder escribir el trace de toda la ejecución

	logger = malloc(sizeof(t_log));

	crearLog("/MEMORIA");

	//Levanta la configuración del proceso memoria
	cargarConfiguracion();

	//Crea la lista de clientes conectados para cpu y kernel
	inicializarListas();

	setvbuf (stdout, NULL, _IONBF, 0);

	crearMemoriaPrincipal();

	crearEstructurasAdministrativas();

//	t_filaTablaInvertida* fila = (t_filaTablaInvertida*)memoriaPrincipal;
//
//	escribirEnMemoria(6,"prueba de escritura en memoria");
//
//	void* pagina = leerPagina(6);
//	char* aver = (char*)pagina;
//	printf("%s",aver); //Leo lo que escribi en la página 6

	imprimirTablaPaginas();

	liberarMemoriaPrincipal();

	crearThreadAtenderConexiones();

//	pthread_join(threadAtenderConexiones, NULL);


	return 0;

}
