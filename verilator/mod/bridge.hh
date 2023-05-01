#ifndef SVDPI_BRIDGE
#define SVDPI_BRIDGE

// System-Verilog Direct Programming Interface
#include <svdpi.h>

// Python Development Header
#include <Python.h>
#include <abstract.h>
#include <boolobject.h>
#include <import.h>
#include <listobject.h>
#include <longobject.h>
#include <object.h>
#include <tupleobject.h>
#include <unicodeobject.h>

// Standard Library Header
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <map>
#include <utility>


#ifdef __cplusplus
extern "C" {
#endif

extern void init_python_env();
extern void deinit_python_env();

extern void array_handle(char*, char*, svOpenArrayHandle, svOpenArrayHandle);

#ifdef __cplusplus
}
#endif

#endif /* SVDPI_BRIDGE */