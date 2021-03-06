#ifndef FSHELPER_H_
#define FSHELPER_H_

#include <arpa/inet.h>
#include <commons/collections/queue.h>
#include <commons/config.h>
#include <estructuras.h>
#include <logger.h>
#include <netdb.h>
#include <pthread.h>
#include <sockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <commons/bitarray.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <dirent.h>
#include <math.h>
#include <estructuras.h>


enum {
    SADICA = 1024
};
typedef struct config_t {

  int puertoFS;
  char * puntoMontaje;
} t_configuracion;

typedef struct mtdt_t{

  int bloque_size;
  int bloque_cant;

} t_metadata;

t_bitarray * bitarray;
void * bmap;
int socketCliente;

t_metadata metadata;

pthread_t threadAtenderKernel;

struct stat mystat;
int block_counter;

t_configuracion * configuracion;
char * pathConfiguracion;
void cargarConfiguracion();

void crearServidorMonocliente();

void manejarKernel(int i);

void crearBitmap();

int asignarBloque(t_config * data);

int bloquesLibres();

void setMetadata();

void setPuntoDeMontaje();

void obtenerDatos(t_struct_obtener * archivo);

void guardarDatos(t_struct_guardar * archivo);

void borrarArchivo(t_struct_borrar * archivo);

void validarArchivo(t_struct_abrir * archivo);

#endif /* FSHELPER_H_ */
