/// @file defines.h
/// @brief Contiene la definizioni di variabili
///         e funzioni specifiche del progetto.

#pragma once

#ifndef _CONFIG_HH
#define _CONFIG_HH

#include "config.h"

//utils
#include <stdlib.h>  /* atoi, itoa */
#include <stdio.h>
#include <string.h>
#include <time.h>

//pid_t, tok_t
#include <sys/types.h>

//System call
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h> 
#include <sys/msg.h> 
#include <sys/ipc.h>
#include <sys/sem.h>


//Utility
#define FALSE (1==0)
#define TRUE  (1==1)


//Data types


#endif