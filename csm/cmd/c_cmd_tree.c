#include "c_cmd_tree.h"

struct c_cmd_tree {
    uint8_t num_nodes;
    c_cmd_node * head;
};

c_cmd_tree * c_cmd_tree_create() {
    c_cmd_tree * inst = (c_cmd_tree *) malloc(sizeof(c_cmd_tree));

    if (inst == NULL) {
        perror("c_cmd_tree_create");
        return NULL;
    }

    inst->num_nodes = 0;
    inst->head = NULL;

    return inst;
}

void c_cmd_tree_destroy(c_cmd_tree * inst) {
    if (inst->num_nodes != 0) {
        c_cmd_node_destroy(inst->head);
    }

    free(inst);
}

c_cmd_node * c_cmd_tree_get_node(c_cmd_tree * inst, char name[RNO_G_CMD_NAME_LIM]) {
    c_cmd_node * cur_head = inst->head;
    int cmp;

    while (1) {
        if (cur_head == NULL) {
            return NULL;
        }

        cmp = strcmp(name, c_cmd_node_get_name(cur_head));

        if (cmp == 0) {
            return cur_head;
        }
        else if (cmp > 0) {
            cur_head = c_cmd_node_get_right(cur_head);
        }
        else {
            cur_head = c_cmd_node_get_left(cur_head);
        }
    }
}

int c_cmd_tree_add_node(c_cmd_tree * inst, c_cmd_node * node) {
    if (inst->num_nodes == 0) {
        inst->head = node;
        inst->num_nodes++;
    }
    else {
        c_cmd_node * cur_head = inst->head;
        int cmp;

        while (1) {
            cmp = strcmp(c_cmd_node_get_name(node), c_cmd_node_get_name(cur_head));

            if (cmp == 0) {
                return EXIT_FAILURE;
            }
            else if (cmp > 0) {
                if (c_cmd_node_get_right(cur_head) == NULL) {
                    c_cmd_node_set_right(cur_head, node);
                    break;
                }
                else {
                    cur_head = c_cmd_node_get_right(cur_head);
                }
            }
            else {
                if (c_cmd_node_get_left(cur_head) == NULL) {
                    c_cmd_node_set_left(cur_head, node);
                    break;
                }
                else {
                    cur_head = c_cmd_node_get_left(cur_head);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
