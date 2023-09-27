#include "b_cmd_node.h"

struct b_cmd_node {
    char name[RNO_G_CMD_NAME_LIM+1];
    uint8_t num_args;
    uint8_t arg_bounds[RNO_G_CMD_ARG_LIM][2];
    b_cmd_node * left;
    b_cmd_node * right;
};

b_cmd_node * b_cmd_node_create(char name[RNO_G_CMD_NAME_LIM+1], uint8_t num_args,
                               uint8_t arg_bounds[RNO_G_CMD_ARG_LIM][2]) {

    b_cmd_node * inst = (b_cmd_node *) malloc(sizeof(b_cmd_node));

    if (inst == NULL) {
        perror("b_cmd_node_create");
        return NULL;
    }

    strcpy(inst->name, name);
    inst->num_args = num_args;

    memset(inst->arg_bounds, 0, RNO_G_CMD_ARG_LIM * 2);

    for (int i = 0; i < num_args; i++) {
        inst->arg_bounds[i][0] = arg_bounds[i][0];
        inst->arg_bounds[i][1] = arg_bounds[i][1];
    }

    inst->left = NULL;
    inst->right = NULL;

    return inst;
}

void b_cmd_node_destroy(b_cmd_node * inst) {
    if (inst->left != NULL) {
        b_cmd_node_destroy(inst->left);
    }

    if (inst->right != NULL) {
        b_cmd_node_destroy(inst->right);
    }

    free(inst);
}

char * b_cmd_node_get_name(b_cmd_node * inst) {
    return inst->name;
}

uint8_t b_cmd_node_get_num_args(b_cmd_node * inst) {
    return inst->num_args;
}

uint8_t * b_cmd_node_get_arg_bound(b_cmd_node * inst, uint8_t index) {
    if (index > inst->num_args) {
        fprintf(stderr, "b_cmd_node_get_arg_bound: index out of bounds");
        return NULL;
    }

    return inst->arg_bounds[index];
}

b_cmd_node * b_cmd_node_get_left (b_cmd_node * inst) {
    return inst->left;
}

void b_cmd_node_set_left(b_cmd_node * inst, b_cmd_node * left) {
    inst->left = left;
}

b_cmd_node * b_cmd_node_get_right (b_cmd_node * inst) {
    return inst->right;
}

void b_cmd_node_set_right(b_cmd_node * inst, b_cmd_node * right) {
    inst->right = right;
}

