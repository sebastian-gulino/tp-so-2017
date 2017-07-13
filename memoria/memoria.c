#include "manejoMemoria.h"

int main(void) {

	setvbuf (stdout, NULL, _IONBF, 0);

	//Genera archivo log pars poder escribir el trace de toda la ejecución
	crearLog("/MEMORIA");

	//Levanta la configuración del proceso memoria
	cargarConfiguracion();

	//Crea la lista de clientes conectados para cpu y kernel
	inicializarListas();

	setvbuf (stdout, NULL, _IONBF, 0);

	crearMemoriaPrincipal();

	crearEstructurasAdministrativas();

	crearCache();

	reservarFramesProceso(111,2048,1);

	imprimirTablaPaginas();

	escribirEnMemoria(25,"esta es una prueba\n",18,100);

	void * pagina = leerPagina(2,111);

	printf("%s\n",pagina + 100);

	pagina = leerPagina(2,111);

	printf("%s\n",pagina + 100);


//	imprimirCache();

	vaciarCache();

//	imprimirTablaPaginas();

//	asignarPaginasProceso(666,1);

	liberarMemoriaPrincipal();

//	crearThreadAtenderConexiones();
//
//	pthread_join(threadAtenderConexiones, NULL);

	return 0;

}
