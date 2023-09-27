#include "c_cmd_node.h"

struct c_cmd_node {
    char name[RNO_G_CMD_NAME_LIM+1];
    uint8_t num_args;
    uint8_t arg_bounds[RNO_G_CMD_ARG_LIM][2];
    char comp[RNO_G_CMD_COMP_LIM];
    c_cmd_node * left;
    c_cmd_node * right;
};

c_cmd_node * c_cmd_node_create(char name[RNO_G_CMD_NAME_LIM+1], uint8_t num_args,
                               uint8_t arg_bounds[RNO_G_CMD_ARG_LIM][2], char comp[RNO_G_CMD_COMP_LIM]) {

    c_cmd_node * inst = (c_cmd_node *) malloc(sizeof(c_cmd_node));

    if (inst == NULL) {
        perror("c_cmd_node_create");
        return NULL;
    }

    strcpy(inst->name, name);
    inst->num_args = num_args;

    memset(inst->arg_bounds, 0, RNO_G_CMD_ARG_LIM * 2);

    for (int i = 0; i < num_args; i++) {
        inst->arg_bounds[i][0] = arg_bounds[i][0];
        inst->arg_bounds[i][1] = arg_bounds[i][1];
    }

    strcpy(inst->comp, comp);

    inst->left = NULL;
    inst->right = NULL;

    return inst;
}

void c_cmd_node_destroy(c_cmd_node * inst) {
    if (inst->left != NULL) {
        c_cmd_node_destroy(inst->left);
    }

    if (inst->right != NULL) {
        c_cmd_node_destroy(inst->right);
    }

    free(inst);
}

char * c_cmd_node_get_name(c_cmd_node * inst) {
    return inst->name;
}

uint8_t c_cmd_node_get_num_args(c_cmd_node * inst) {
    return inst->num_args;
}

uint8_t * c_cmd_node_get_arg_bound(c_cmd_node * inst, uint8_t index) {
    if (index > inst->num_args) {
        fprintf(stderr, "c_cmd_node_get_arg_bound: index out of bounds");
        return NULL;
    }

    return inst->arg_bounds[index];
}

char * c_cmd_node_get_comp(c_cmd_node * inst) {
    return inst->comp;
}

c_cmd_node * c_cmd_node_get_left (c_cmd_node * inst) {
    return inst->left;
}

void c_cmd_node_set_left(c_cmd_node * inst, c_cmd_node * left) {
    inst->left = left;
}

c_cmd_node * c_cmd_node_get_right (c_cmd_node * inst) {
    return inst->right;
}

void c_cmd_node_set_right(c_cmd_node * inst, c_cmd_node * right) {
    inst->right = right;
}

