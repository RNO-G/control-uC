#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"

/*
 * function : errno_check
 * ----------------------
 * check if an errno-modifying function has encountered an error
 *
 * func_return : the return value of the function
 *
 * func_name : the name of the function
 *
 * returns : nothing
 */
void errno_check(int func_return, char * func_name);

#endif
