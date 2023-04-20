#ifndef KERNEL_HILOS_H
#define KERNEL_HILOS_H

#include "kernel.h"


void *atender_cliente();
void *atender_consola();
t_list* separarSegmentoDeInstrucciones();

#endif
