#ifndef RNO_G_C_CMD_TREE_H
#define RNO_G_C_CMD_TREE_H

#include "c_cmd_node.h"

typedef struct c_cmd_tree c_cmd_tree;

c_cmd_tree * c_cmd_tree_create();

void c_cmd_tree_destroy(c_cmd_tree * inst);

c_cmd_node * c_cmd_tree_get_node(c_cmd_tree * inst, char name[RNO_G_CMD_NAME_LIM]);

int c_cmd_tree_add_node(c_cmd_tree * inst, c_cmd_node * node);

#endif
