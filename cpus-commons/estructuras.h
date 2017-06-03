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

	typedef uint8_t t_tipoEstructura;

	typedef struct IndiceCodigo {
		t_list * limitesInstrucciones;
	} t_indice_codigo;

	typedef struct LimitesInstrucciones {
		unsigned char inicioInstruccion;
		unsigned char longitudInstruccion;
	} t_limites_instrucciones;

	typedef struct PosicionMemoria {
		unsigned char pagina;
		unsigned char offsetInstruccion;
		unsigned char longitudInstruccion;
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

	typedef struct PCB {
		unsigned char PID;
		unsigned char PC;
		unsigned char cantidadPaginas;
		t_list * indiceCodigo;
		char * indiceEtiquetas;
		t_list * indiceStack;
		int exitcode;
	} t_pcb;

	typedef struct Stream {
		int length;
		char* data;
	} t_stream;

	typedef struct Cabecera {
			uint32_t id;		/* ID de operacion */
			uint32_t longitud; /* Longitud del mensaje, en bytes */
		} cabecera_t;

	enum{
		//Generales
		D_STRUCT_NUMERO=1,
		D_STRUCT_CHAR=2,
		D_STRUCT_STRING=3,

		//Comunicacion Kernel-CPU
		ES_CPU=4,

		//Comunicacion Kernel-Consola
		ES_CONSOLA=119,

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

	//MEMORIA

	typedef struct FilaTablaInvertida {
		int frame;
		int pid;
		int pagina;
	} t_filaTablaInvertida;



#endif /* ESTRUCTURAS_H_ */
