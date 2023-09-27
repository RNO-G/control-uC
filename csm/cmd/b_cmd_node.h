#ifndef RNO_G_B_CMD_NODE_H
#define RNO_G_B_CMD_NODE_H

#define RNO_G_CMD_NAME_LIM 32
#define RNO_G_CMD_ARG_LIM 8

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct b_cmd_node b_cmd_node;

b_cmd_node * b_cmd_node_create(char name[RNO_G_CMD_NAME_LIM+1], uint8_t num_args,
                               uint8_t arg_bounds[RNO_G_CMD_ARG_LIM][2]);

void b_cmd_node_destroy(b_cmd_node * inst);

char * b_cmd_node_get_name(b_cmd_node * inst);

uint8_t b_cmd_node_get_num_args(b_cmd_node * inst);

uint8_t * b_cmd_node_get_arg_bound(b_cmd_node * inst, uint8_t index);

b_cmd_node * b_cmd_node_get_left(b_cmd_node * left);

void b_cmd_node_set_left(b_cmd_node * inst, b_cmd_node * left);

b_cmd_node * b_cmd_node_get_right(b_cmd_node * right);

void b_cmd_node_set_right(b_cmd_node * inst, b_cmd_node * right);

#endif
