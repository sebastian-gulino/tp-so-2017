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

//	t_filaTablaInvertida* fila = (t_filaTablaInvertida*)memoriaPrincipal;
//
//	escribirEnMemoria(6,"prueba de escritura en memoria");
//
//	void* pagina = leerPagina(6);
//	char* aver = (char*)pagina;
//	printf("%s",aver); //Leo lo que escribi en la página 6

//	reservarFramesProceso(111,4096,1);
//
//	reservarFramesProceso(666,1024,0);
//
//	reservarFramesProceso(555,512,0);
//
//	reservarFramesProceso(444,512,0);
//
//	finalizarPrograma(555);
//
//	reservarFramesProceso(333,1024,1);
//
//	imprimirTablaPaginas();
//
////	asignarPaginasProceso(666,1);
//
//	liberarMemoriaPrincipal();

	crearThreadAtenderConexiones();

	pthread_join(threadAtenderConexiones, NULL);

	return 0;

}
