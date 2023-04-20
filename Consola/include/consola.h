#ifndef CONSOLA_H
#define CONSOLA_H
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include "shared_utils.h"
#include "tests.h"
#endif

int inicializar_cliente();
char* mostrarEnPantalla();
char* recibirNotificacion(int);
void responderPorTeclado(int);