#ifndef RNO_G_B_CMD_TREE_H
#define RNO_G_B_CMD_TREE_H

#include "b_cmd_node.h"

typedef struct b_cmd_tree b_cmd_tree;

b_cmd_tree * b_cmd_tree_create();

void b_cmd_tree_destroy(b_cmd_tree * inst);

b_cmd_node * b_cmd_tree_get_node(b_cmd_tree * inst, char name[RNO_G_CMD_NAME_LIM]);

int b_cmd_tree_add_node(b_cmd_tree * inst, b_cmd_node * node);

#endif
