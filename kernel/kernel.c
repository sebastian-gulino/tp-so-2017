#include "kernelHelper.h"

int main(int arc, char * argv[]) {

	//Genera archivo log para poder escribir el trace de toda la ejecución
	logger = malloc(sizeof(t_log));
	crearLog("/KERNEL");

	//Levanta la configuración del proceso kernel
	configuracion = cargarConfiguracion();

	//Crea la lista de clientes conectados para cpu y consola
	inicializarListas();

	//Conecta el Kernel a la memoria del sistema
	socketMemoria = conectarAMemoria();

	//Conecta el Kernel al filesystem
	int socketFS = conectarAFS();

	crearThreadAtenderConexiones();

	t_struct_abrir * test = malloc(sizeof(t_struct_abrir));
	void * retest;
	t_tipoEstructura tipoStruct;

	test->confirmacion = 0;
	test->modo_creacion = 1;
	test->path = "/cacho/Test.bin";
	int asd;
	socket_enviar(socketFS, D_STRUCT_ABRIR, test);
	puts("aaa");
	socket_recibir(socketFS, &tipoStruct, &retest);

	if(tipoStruct == D_STRUCT_ABRIR){
	asd = ((t_struct_abrir *) retest)->confirmacion;
	puts("asd");
	printf("%d", asd);
	}

	if(asd == 2){
		puts("omg i'm great");
	}

	puts("fack");


	pthread_join(threadAtenderConexiones, NULL);

	return 0;

}


