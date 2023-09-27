#ifndef RNO_G_C_CMD_NODE_H
#define RNO_G_C_CMD_NODE_H

#define RNO_G_CMD_NAME_LIM 32
#define RNO_G_CMD_ARG_LIM 8
#define RNO_G_CMD_COMP_LIM 64

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct c_cmd_node c_cmd_node;

c_cmd_node * c_cmd_node_create(char name[RNO_G_CMD_NAME_LIM+1], uint8_t num_args,
                               uint8_t arg_bounds[RNO_G_CMD_ARG_LIM][2], char comp[RNO_G_CMD_COMP_LIM]);

void c_cmd_node_destroy(c_cmd_node * inst);

char * c_cmd_node_get_name(c_cmd_node * inst);

uint8_t c_cmd_node_get_num_args(c_cmd_node * inst);

uint8_t * c_cmd_node_get_arg_bound(c_cmd_node * inst, uint8_t index);

char * c_cmd_node_get_comp(c_cmd_node * inst);

c_cmd_node * c_cmd_node_get_left(c_cmd_node * left);

void c_cmd_node_set_left(c_cmd_node * inst, c_cmd_node * left);

c_cmd_node * c_cmd_node_get_right(c_cmd_node * right);

void c_cmd_node_set_right(c_cmd_node * inst, c_cmd_node * right);

#endif
