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

//	reservarFramesProceso(111,2048,1);
//
//	imprimirTablaPaginas();
//
//	bool escritura = escribirPagina(2,111,100,20,"esta es una prueba\n");
//
//	printf("%d\n",escritura);
//
//	t_resultadoLectura resultado =  leerPagina(2, 111, 100, 20);
//
//	printf("%s\n",resultado.contenido);
//
//	printf("%d\n",resultado.resultado);


//	imprimirCache();

//

//	imprimirTablaPaginas();

//	asignarPaginasProceso(666,1);

	pthread_create(&threadCommandHandler, NULL, (void*)&manejoConsola,NULL);

	crearThreadAtenderConexiones();

	pthread_join(threadAtenderConexiones, NULL);

	liberarMemoriaPrincipal();
	vaciarCache();

	return 0;

}
