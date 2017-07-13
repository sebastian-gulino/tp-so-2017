#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <parser/metadata_program.h>

typedef uint8_t t_tipoEstructura;

typedef struct IndiceCodigo {
	t_list * limitesInstrucciones;
} t_indice_codigo;

typedef struct LimitesInstrucciones {
	unsigned char inicioInstruccion;
	unsigned char longitudInstruccion;
} t_limites_instrucciones;

typedef struct PosicionMemoria {
	int pagina;
	int offsetInstruccion;
	int longitudInstruccion;
} t_posicion_memoria;

typedef struct Stack {
	unsigned char posicion;
	t_list *argumentos;
	t_list *variables;
	unsigned char retPos;
	t_posicion_memoria posicionVariableRetorno;
} t_stack;

typedef struct Argumento {
	char identificador[2]; // digito del 0 al 9 anteponiendo el signo $
	t_posicion_memoria posicionMemoria;
} t_argumento;

typedef struct Variable {
	char identificador; // nombre de la variable
	t_posicion_memoria posicionMemoria;
} t_variable;

typedef struct {
	int cantidad_args;
	t_list* args;
	int cantidad_vars;
	t_list* vars;
	int retPos;
	t_posicion_memoria retVar;
}  __attribute__((packed)) registroStack;

typedef struct registroTablaProcesos {
	uint32_t socket;
	uint32_t PID;
} t_registroTablaProcesos;

typedef struct Stream {
	int length;
	char* data;
} t_stream;

typedef struct Cabecera {
	uint32_t id;		/* ID de operacion */
	uint32_t longitud; /* Longitud del mensaje, en bytes */
} cabecera_t;

typedef struct{
	bool lectura;
	bool escritura;
	bool creacion;
} t_flags;

typedef struct{
	uint32_t socket;
	uint32_t quantum;
	uint32_t PID;
} t_cpu;

typedef struct {
	uint32_t pid;
	uint32_t rafagas;
	uint32_t syscall;
	uint32_t cantidad_solicitar_heap;
	uint32_t total_heap_solicitado;
	uint32_t cantidad_liberar_heap;
	uint32_t total_heap_liberado;
	char * semaforo_bloqueo;
} __attribute__ ((__packed__)) t_registroInformacionProceso;

typedef struct RegistroTablaHeap {
	uint32_t PID;
	uint32_t numeroPagina;
	t_list * listaBloques;
	uint32_t espacioDisponible;
	uint32_t espacioMaximoBloque;
} t_registroTablaHeap;

typedef struct BloqueHeap {
	bool isFree;
	uint32_t size;
	uint32_t numeroBloque;
	uint32_t offset;
	uint32_t fin;
} t_bloqueHeap;

enum{
	//Generales
	D_STRUCT_NUMERO=1,
	D_STRUCT_CHAR=2,
	D_STRUCT_STRING=3,

	//Comunicacion Kernel - Memoria
	D_STRUCT_MALC=4,
	D_STRUCT_LIBERAR_MEMORIA=49,
	D_STRUCT_ESCRITURA_CODIGO=50,
	D_STRUCT_CODIGO=51,

	//Comunicacion Consola - Kernel
	D_STRUCT_PROG=5,
	D_STRUCT_IMPR=6,
	D_STRUCT_FIN_PROG=7,
	D_STRUCT_SOLICITAR_CODIGO=52,
	D_STRUCT_FIN_PCB=57,

	//Comunicacion Kernel - CPU
	D_STRUCT_PCB=8,
	D_STRUCT_ABORT=12,
	D_STRUCT_SIGUSR1=13,
	D_STRUCT_PCB_FIN_OK=15,
	D_STRUCT_WAIT=16,
	D_STRUCT_SIGNAL=17,
	D_STRUCT_OBTENER_COMPARTIDA=18,
	D_STRUCT_GRABAR_COMPARTIDA=19,
	D_STRUCT_ARCHIVO_ESC=20,
	D_STRUCT_ARCHIVO_LEC=21,
	D_STRUCT_SOL_HEAP=22,
	D_STRUCT_RTA_HEAP=23,
	D_STRUCT_LIB_HEAP=24,
	D_STRUCT_ARCHIVO_ABR=25,
	D_STRUCT_ARCHIVO_CER=26,
	D_STRUCT_ARCHIVO_BOR=27,
	D_STRUCT_ARCHIVO_MOV=28,
	D_STRUCT_PCB_FIN_ERROR=48,
	D_STRUCT_ABORTAR_EJECUCION=53,
	D_STRUCT_CONTINUAR_EJECUCION=54,
	D_STRUCT_FIN_QUANTUM=55,
	D_STRUCT_FIN_INSTRUCCION=56,


	//Motivos de Retorno PCB
	D_STRUCT_ERROR_MEMORIA=33,
	D_STRUCT_ERROR_KERNEL=34,
	D_STRUCT_ERROR_ESCRITURA=35,
	D_STRUCT_ERROR_LECTURA=36,
	D_STRUCT_ERROR_WAIT=37,
	D_STRUCT_ERROR_SEM=38,
	D_STRUCT_ERROR_HEAP_MAX=39,
	D_STRUCT_ERROR_HEAP=40,
	D_STRUCT_ERROR_HEAP_LIB=41,
	D_STRUCT_ERROR_APERTURA=42,
	D_STRUCT_ERROR_BORRAR=43,
	D_STRUCT_ERROR_CERRAR=44,
	D_STRUCT_ERROR_CURSOR=45,
	D_STRUCT_ERROR_INSTRUCCION=46,
	D_STRUCT_ERROR_STACK_OVERFLOW=47,

	//Comunicacion CPU - Memoria
	D_STRUCT_PID=9,
	D_STRUCT_LECT=10,
	D_STRUCT_LECT_VAR=11,
	D_STRUCT_SOL_ESCR=14,


	//Comunicacion Kernel - Filesystem
	D_STRUCT_BORRAR = 29,
	D_STRUCT_ABRIR = 30,
	D_STRUCT_OBTENER = 31,
	D_STRUCT_GUARDAR = 32,

	//Handshake
	ES_KERNEL=100,
	ES_CPU=101,
	ES_MEMORIA=102,
	ES_FILESYSTEM=103,
	ES_CONSOLA=104,

	//Confirmaciones memoria
	MEMORIA_OK=105,
	MEMORIA_ERROR=106,

	//Confirmaciones kernel
	KERNEL_OK=107,
	KERNEL_ERROR=108,
	KERNEL_MULTIPROG=999,

	//Confirmaciones Filesystem
	FS_ABRIR_CREAR_OK=109,
	FS_ABRIR_CREAR_ERROR=110,
	FS_ABRIR_NO_CREAR_OK=111,
	FS_ABRIR_NO_CREAR_ERROR=112,
	FS_BORRAR_OK=113,
	FS_BORRAR_ERROR=114,
	FS_ESCRIBIR_OK=115,
	FS_ESCRIBIR_ERROR=116,
	FS_LEER_ERROR=117,
	FS_LEER_OK=118



} t_operaciones;

// Header de stream
typedef struct {
	uint8_t tipoEstructura;
	uint16_t length;
} __attribute__ ((__packed__)) t_header;


// Estructuras segun tipo de datos a enviar por sockets

typedef struct struct_numero {
	int32_t numero;
} __attribute__ ((__packed__)) t_struct_numero;

typedef struct struct_char {
	char letra;
} __attribute__ ((__packed__)) t_struct_char;

typedef struct struct_string {
	char * string;
} __attribute__ ((__packed__)) t_struct_string;

typedef struct struct_malloc{
	uint32_t PID;
	uint32_t tamano_segmento;
}__attribute__((__packed__)) t_struct_malloc;

typedef struct struct_env_bytes{
		uint32_t base;
		uint32_t PID;
		uint32_t tamanio;
		void* buffer;
}__attribute__ ((__packed__)) t_struct_programa;

typedef struct semaforo{
	t_nombre_semaforo * nombre;
	uint32_t valor;
}__attribute__ ((__packed__)) t_struct_semaforo;

enum {
	E_NEW=1,
	E_READY=2,
	E_BLOCK=3,
	E_EXEC=4,
	E_EXIT=5
} estadosPrograma;

enum {
	EC_FINALIZO_OK=0,
	EC_NO_RECURSOS=-1,
	EC_ARCHIVO_INEX=-2,
	EC_ARCHIVO_LE_PERMISOS=-3,
	EC_ARCHIVO_ES_PERMISOS=-4,
	EC_EXCEP_MEMORIA=-5,
	EC_DESCONEXION_CONSOLA=-6,
	EC_FINALIZADO_CONSOLA=-7,
	EC_RESERVA_MAYOR_PAGINA=-8,
	EC_MAXIMO_PAGINAS=9,
	EC_DESCONEXION_CPU=10,
	EC_DESCONEXION_KERNEL=11,
	EC_STACK_OVERFLOW=12,
	EC_SIN_DEFINICION=-20
} exitCodeValidos;

typedef struct struct_pcb {
		int PID;
		int programCounter;
		int paginasCodigo;
		int cpuID;
		t_list * indiceCodigo;
		int paginasStack;
		int cantRegistrosStack;
		int stackPointer;
		int paginaActualStack;
		int primerPaginaStack;
		t_list * indiceStack;
		int tamanioIndiceEtiquetas;
		char * indiceEtiquetas;
		int exitcode;
		uint32_t cantidadInstrucciones;
		uint32_t quantum_sleep;
		uint32_t retornoPCB;
		uint32_t estado;

} __attribute__ ((__packed__)) t_struct_pcb;

typedef struct {
	int pagina, offset, contenido, PID;
} __attribute__((packed)) t_struct_sol_escritura;

typedef struct {
	int pagina, offset, contenido, PID;
} __attribute__((packed)) t_struct_sol_lectura;

typedef struct {
	t_nombre_variable * nombre;
	t_valor_variable valor;
} __attribute__((packed)) t_struct_var_compartida;

typedef struct {
	uint32_t fileDescriptor;
	char* informacion;
	uint32_t tamanio;
	uint32_t pid;
	t_flags flags;
}__attribute__((__packed__)) t_struct_archivo;

typedef struct {
	uint32_t pointer;
	uint32_t pid;
}__attribute__((__packed__)) t_struct_sol_heap;

//MEMORIA
typedef struct FilaTablaInvertida {
	int frame;
	int pid;
	int pagina;
} t_filaTablaInvertida;

typedef struct struct_borrar{
	char * path;
	int confirmacion;
}__attribute__((__packed__)) t_struct_borrar;

typedef struct struct_abrir{
	int modo_creacion;
	char * path;
	int confirmacion;
}__attribute__((__packed__)) t_struct_abrir;

typedef struct struct_obtener{
	int modo_lectura;
	char * path;
	int offset;
	int size;
	int confirmacion;
	void * obtenido;
}__attribute__((__packed__)) t_struct_obtener;

typedef struct struct_guardar{
	int modo_escritura;
	char * path;
	int offset;
	int size;
	void * buffer;
	int confirmacion;
}__attribute__((__packed__)) t_struct_guardar;

typedef struct {
	uint32_t PID;
	t_list* tabla;
} t_tabla_archivos_proc;

typedef struct registroArchivosProceso{
	uint32_t cursor;
	t_descriptor_archivo fd_TablaGlobal;
	t_flags flags;
} t_registroArchivosProc;

typedef struct registroArchivosGlobal{
	char* nombre;
	uint32_t cantidadAbierto;
} t_registroArchivosGlobal;


#endif /* ESTRUCTURAS_H_ */
