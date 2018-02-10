#ifndef _MOUSE_H
#define _MOUSE_H

#include "lib.h"
#include "i8259.h"
#include "terminal.h"


#define MOUSE_INT_NUM 12


/* mouse initializer */
void mouse_init();

void mouse_handler();

#endif
