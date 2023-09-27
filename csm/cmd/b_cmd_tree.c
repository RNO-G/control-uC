#include "b_cmd_tree.h"

struct b_cmd_tree {
    uint8_t num_nodes;
    b_cmd_node * head;
};

b_cmd_tree * b_cmd_tree_create() {
    b_cmd_tree * inst = (b_cmd_tree *) malloc(sizeof(b_cmd_tree));

    if (inst == NULL) {
        perror("b_cmd_tree_create");
        return NULL;
    }

    inst->num_nodes = 0;
    inst->head = NULL;

    return inst;
}

void b_cmd_tree_destroy(b_cmd_tree * inst) {
    if (inst->num_nodes != 0) {
        b_cmd_node_destroy(inst->head);
    }

    free(inst);
}

b_cmd_node * b_cmd_tree_get_node(b_cmd_tree * inst, char name[RNO_G_CMD_NAME_LIM]) {
    b_cmd_node * cur_head = inst->head;
    int cmp;

    while (1) {
        if (cur_head == NULL) {
            return NULL;
        }

        cmp = strcmp(name, b_cmd_node_get_name(cur_head));

        if (cmp == 0) {
            return cur_head;
        }
        else if (cmp > 0) {
            cur_head = b_cmd_node_get_right(cur_head);
        }
        else {
            cur_head = b_cmd_node_get_left(cur_head);
        }
    }
}

int b_cmd_tree_add_node(b_cmd_tree * inst, b_cmd_node * node) {
    if (inst->num_nodes == 0) {
        inst->head = node;
        inst->num_nodes++;
    }
    else {
        b_cmd_node * cur_head = inst->head;
        int cmp;

        while (1) {
            cmp = strcmp(b_cmd_node_get_name(node), b_cmd_node_get_name(cur_head));

            if (cmp == 0) {
                return EXIT_FAILURE;
            }
            else if (cmp > 0) {
                if (b_cmd_node_get_right(cur_head) == NULL) {
                    b_cmd_node_set_right(cur_head, node);
                    break;
                }
                else {
                    cur_head = b_cmd_node_get_right(cur_head);
                }
            }
            else {
                if (b_cmd_node_get_left(cur_head) == NULL) {
                    b_cmd_node_set_left(cur_head, node);
                    break;
                }
                else {
                    cur_head = b_cmd_node_get_left(cur_head);
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
