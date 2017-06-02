#include "manejoMemoria.h"

int main(void) {

	//Genera archivo log para poder escribir el trace de toda la ejecución
	logger = malloc(sizeof(t_log));
	crearLog("/MEMORIA");

	configuracion = cargarConfiguracion();

	crearThreadAtenderConexiones();

//	setvbuf (stdout, NULL, _IONBF, 0);
//
//	crearMemoriaPrincipal();
//
//	crearEstructurasAdministrativas();
//
////	t_filaTablaInvertida* fila = (t_filaTablaInvertida*)memoriaPrincipal;
////
////	escribirEnMemoria(6,"prueba de escritura en memoria");
////
////	void* pagina = leerPagina(6);
////	char* aver = (char*)pagina;
////	printf("%s",aver); //Leo lo que escribi en la página 6
//
//	imprimirTablaPaginas();
//	liberarMemoriaPrincipal();
//
//	//Crea la lista de clientes conectados para cpu y kernel
//	inicializarListas();
//
//	//Crea el thread encargado de recibir las nuevas conexiones de CPU y Kernel
//
//
//	pthread_join(threadAtenderConexiones, NULL);

	return 0;

}
