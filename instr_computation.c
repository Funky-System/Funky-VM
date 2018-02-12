#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "instructions.h"
#include "vm.h"
#include "error_handling.h"

#define ARITH_OPERATOR(OP) { \
    USE_STACK(); \
    AJS_STACK(-1); \
     \
    if ((stack - 1)->type == VM_TYPE_UINT) { \
        if (stack->type == VM_TYPE_UINT) \
            *(stack - 1) = (vm_value_t) { .uint_value = (vm_type_t) (stack - 1)->uint_value OP stack->uint_value, .type = VM_TYPE_UINT }; \
        else if (stack->type == VM_TYPE_INT) \
            *(stack - 1) = (vm_value_t) { .int_value = (vm_type_signed_t) ((stack - 1)->uint_value OP stack->int_value), .type = VM_TYPE_INT }; \
        else if (stack->type == VM_TYPE_FLOAT) \
            *(stack - 1) = (vm_value_t) { .float_value = (vm_type_float_t) ((stack - 1)->uint_value OP stack->float_value), .type = VM_TYPE_FLOAT }; \
    } else if ((stack - 1)->type == VM_TYPE_INT) { \
        if (stack->type == VM_TYPE_UINT) \
            *(stack - 1) = (vm_value_t) { .int_value = (vm_type_signed_t) (stack - 1)->int_value OP stack->uint_value, .type = VM_TYPE_INT }; \
        else if (stack->type == VM_TYPE_INT) \
            *(stack - 1) = (vm_value_t) { .int_value = (vm_type_signed_t) ((stack - 1)->int_value OP stack->int_value), .type = VM_TYPE_INT }; \
        else if (stack->type == VM_TYPE_FLOAT) \
            *(stack - 1) = (vm_value_t) { .float_value = (vm_type_float_t) ((stack - 1)->int_value OP stack->float_value), .type = VM_TYPE_FLOAT }; \
    } else if ((stack - 1)->type == VM_TYPE_FLOAT) { \
        if (stack->type == VM_TYPE_UINT) \
            *(stack - 1) = (vm_value_t) { .float_value = (vm_type_float_t) (stack - 1)->float_value OP stack->uint_value, .type = VM_TYPE_FLOAT }; \
        else if (stack->type == VM_TYPE_INT) \
            *(stack - 1) = (vm_value_t) { .float_value = (vm_type_float_t) ((stack - 1)->float_value OP stack->int_value), .type = VM_TYPE_FLOAT }; \
        else if (stack->type == VM_TYPE_FLOAT) \
            *(stack - 1) = (vm_value_t) { .float_value = (vm_type_float_t) ((stack - 1)->float_value OP stack->float_value), .type = VM_TYPE_FLOAT }; \
    } else { \
        vm_error(state, "Operator %s has not been defined for stack type %d", #OP, (stack - 1)->type); \
        vm_exit(state, 1); \
    } \
}

/// Addition. Replaces 2 top stack values with the addition of those values.
INSTR(add) {
    USE_STACK();

    if ((stack - 1)->type == VM_TYPE_ARRAY) {
        // lhs is array, add to end of array
        if (stack->type != VM_TYPE_ARRAY) {
            instr_conv_arr(state);
        }
        instr_arr_concat(state);
        return;
    }

    if (stack->type == VM_TYPE_ARRAY) {
        // rhs is array, add to beginning of array
        if ((stack - 1)->type != VM_TYPE_ARRAY) {
            instr_conv_arr_rel(state, -1);
        }
        instr_arr_concat(state);
        return;
    }

    if ((stack - 1)->type == VM_TYPE_STRING) {
        // lhs is string, this is string concatenation
        if (stack->type != VM_TYPE_STRING) {
            instr_conv_str(state);
        }
        instr_strcat(state);
        return;
    }

    if (stack->type == VM_TYPE_STRING) {
        // rhs is string, this is string concatenation
        if ((stack - 1)->type != VM_TYPE_STRING) {
            instr_conv_str_rel(state, -1);
        }
        instr_strcat(state);
        return;
    }

    ARITH_OPERATOR(+);
}

/// Substraction. Replaces 2 top stack values with the subtraction of those values.
INSTR(sub) {
    ARITH_OPERATOR(-);
}

INSTR(mul) {
    ARITH_OPERATOR(*);
}
INSTR(div) {
    ARITH_OPERATOR(/);
}
INSTR(mod) {
    USE_STACK();
    AJS_STACK(-1);

    if ((stack - 1)->type == VM_TYPE_UINT) {
        if (stack->type == VM_TYPE_UINT)
            *(stack - 1) = (vm_value_t) { .uint_value = (vm_type_t) (stack - 1)->uint_value % stack->uint_value, .type = VM_TYPE_UINT };
        else if (stack->type == VM_TYPE_INT)
            *(stack - 1) = (vm_value_t) { .int_value = (vm_type_signed_t) ((stack - 1)->uint_value % stack->int_value), .type = VM_TYPE_INT };
        else if (stack->type == VM_TYPE_FLOAT)
            *(stack - 1) = (vm_value_t) { .float_value = (vm_type_float_t) fmodf((stack - 1)->uint_value, stack->float_value), .type = VM_TYPE_FLOAT };
    } else if ((stack - 1)->type == VM_TYPE_INT) {
        if (stack->type == VM_TYPE_UINT)
            *(stack - 1) = (vm_value_t) { .int_value = (vm_type_signed_t) (stack - 1)->int_value % stack->uint_value, .type = VM_TYPE_INT };
        else if (stack->type == VM_TYPE_INT)
            *(stack - 1) = (vm_value_t) { .int_value = (vm_type_signed_t) ((stack - 1)->int_value % stack->int_value), .type = VM_TYPE_INT };
        else if (stack->type == VM_TYPE_FLOAT)
            *(stack - 1) = (vm_value_t) { .float_value = (vm_type_float_t) fmodf((stack - 1)->int_value, stack->float_value), .type = VM_TYPE_FLOAT };
    } else if ((stack - 1)->type == VM_TYPE_FLOAT) {
        if (stack->type == VM_TYPE_UINT)
            *(stack - 1) = (vm_value_t) { .float_value = (vm_type_float_t) fmodf((stack - 1)->float_value, stack->uint_value), .type = VM_TYPE_FLOAT };
        else if (stack->type == VM_TYPE_INT)
            *(stack - 1) = (vm_value_t) { .float_value = (vm_type_float_t) fmodf((stack - 1)->float_value, stack->int_value), .type = VM_TYPE_FLOAT };
        else if (stack->type == VM_TYPE_FLOAT)
            *(stack - 1) = (vm_value_t) { .float_value = (vm_type_float_t) fmodf((stack - 1)->float_value, stack->float_value), .type = VM_TYPE_FLOAT };
    } else {
        vm_error(state, "Operator %% has not been defined for stack type %d", (stack - 1)->type);
        vm_exit(state, 1);
    }
}
INSTR(pow) {
    USE_STACK();
    AJS_STACK(-1);

    if ((stack - 1)->type == VM_TYPE_UINT) {
        if (stack->type == VM_TYPE_UINT)
            (stack - 1)->float_value = (vm_type_float_t) powf((stack - 1)->uint_value, stack->uint_value);
        else if (stack->type == VM_TYPE_INT)
            (stack - 1)->float_value = (vm_type_float_t) powf((stack - 1)->uint_value, stack->int_value);
        else if (stack->type == VM_TYPE_FLOAT)
            (stack - 1)->float_value = (vm_type_float_t) powf((stack - 1)->uint_value, stack->float_value);
    } else if ((stack - 1)->type == VM_TYPE_INT) {
        if (stack->type == VM_TYPE_UINT)
            (stack - 1)->float_value = (vm_type_float_t) powf((stack - 1)->int_value, stack->uint_value);
        else if (stack->type == VM_TYPE_INT)
            (stack - 1)->float_value = (vm_type_float_t) powf((stack - 1)->int_value, stack->int_value);
        else if (stack->type == VM_TYPE_FLOAT)
            (stack - 1)->float_value = (vm_type_float_t) powf((stack - 1)->int_value, stack->float_value);
    } else if ((stack - 1)->type == VM_TYPE_FLOAT) {
        if (stack->type == VM_TYPE_UINT)
            (stack - 1)->float_value = (vm_type_float_t) powf((stack - 1)->float_value, stack->uint_value);
        else if (stack->type == VM_TYPE_INT)
            (stack - 1)->float_value = (vm_type_float_t) powf((stack - 1)->float_value, stack->int_value);
        else if (stack->type == VM_TYPE_FLOAT)
            (stack - 1)->float_value = (vm_type_float_t) powf((stack - 1)->float_value, stack->float_value);
    } else {
        vm_error(state, "Operator pow has not been defined for stack type %d", (stack - 1)->type);
        vm_exit(state, 1);
    }
    (stack - 1)->type = VM_TYPE_FLOAT;
}
INSTR(neg) {
    USE_STACK();

    if (stack->type == VM_TYPE_UINT) {
        stack->int_value = (vm_type_signed_t) -stack->uint_value;
        stack->type = VM_TYPE_INT;
    } else if ((stack - 1)->type == VM_TYPE_INT) {
        stack->int_value = (vm_type_signed_t) -stack->int_value;
    } else if ((stack - 1)->type == VM_TYPE_FLOAT) {
        stack->float_value = (vm_type_float_t) -stack->float_value;
    } else {
        vm_error(state, "Unary operator - has not been defined for stack type %d", (stack - 1)->type);
        vm_exit(state, 1);
    }
}

INSTR(not) {
    USE_STACK();

    if (stack->type == VM_TYPE_UINT) {
        stack->int_value = (vm_type_t) ~stack->uint_value;
    } else if ((stack - 1)->type == VM_TYPE_INT) {
        stack->int_value = (vm_type_signed_t) ~stack->int_value;
    } else {
        vm_error(state, "Unary operator - has not been defined for stack type %d", (stack - 1)->type);
        vm_exit(state, 1);
    }
}

#define BITWISE_OPERATOR(OP) { \
    USE_STACK(); \
    AJS_STACK(-1); \
     \
    if ((stack - 1)->type == VM_TYPE_UINT) { \
        if (stack->type == VM_TYPE_UINT) \
            *(stack - 1) = (vm_value_t) { .uint_value = (vm_type_t) (stack - 1)->uint_value OP stack->uint_value, .type = VM_TYPE_UINT }; \
        else if (stack->type == VM_TYPE_INT) \
            *(stack - 1) = (vm_value_t) { .int_value = (vm_type_signed_t) ((stack - 1)->uint_value OP stack->int_value), .type = VM_TYPE_INT }; \
    } else if ((stack - 1)->type == VM_TYPE_INT) { \
        if (stack->type == VM_TYPE_UINT) \
            *(stack - 1) = (vm_value_t) { .int_value = (vm_type_signed_t) (stack - 1)->int_value OP stack->uint_value, .type = VM_TYPE_INT }; \
        else if (stack->type == VM_TYPE_INT) \
            *(stack - 1) = (vm_value_t) { .int_value = (vm_type_signed_t) ((stack - 1)->int_value OP stack->int_value), .type = VM_TYPE_INT }; \
    } else { \
        vm_error(state, "Operator %s has not been defined for stack type %d", #OP, (stack - 1)->type); \
        vm_exit(state, 1); \
    } \
}

INSTR(and) {
    BITWISE_OPERATOR(&);
}
INSTR(or) {
    BITWISE_OPERATOR(|);
}
INSTR(xor) {
    BITWISE_OPERATOR(^);
}
INSTR(lsh) {
    BITWISE_OPERATOR(<<);
}
INSTR(rsh) {
    BITWISE_OPERATOR(>>);
}

#define COMPARE_OPERATOR(OP) { \
    USE_STACK(); \
    AJS_STACK(-1); \
     \
    if ((stack - 1)->type == VM_TYPE_UINT) { \
        if (stack->type == VM_TYPE_UINT) \
            (stack - 1)->uint_value = (vm_type_t) ((stack - 1)->uint_value OP stack->uint_value); \
        else if (stack->type == VM_TYPE_INT) \
            (stack - 1)->uint_value = (vm_type_t) ((stack - 1)->uint_value OP stack->int_value); \
        else if (stack->type == VM_TYPE_FLOAT) \
            (stack - 1)->uint_value = (vm_type_t) ((stack - 1)->uint_value OP stack->float_value); \
    } else if ((stack - 1)->type == VM_TYPE_INT) { \
        if (stack->type == VM_TYPE_UINT) \
            (stack - 1)->uint_value = (vm_type_t) ((stack - 1)->int_value OP stack->uint_value); \
        else if (stack->type == VM_TYPE_INT) \
            (stack - 1)->uint_value = (vm_type_t) ((stack - 1)->int_value OP stack->int_value); \
        else if (stack->type == VM_TYPE_FLOAT) \
            (stack - 1)->uint_value = (vm_type_t) ((stack - 1)->int_value OP stack->float_value); \
    } else if ((stack - 1)->type == VM_TYPE_FLOAT) { \
        if (stack->type == VM_TYPE_UINT) \
            (stack - 1)->uint_value = (vm_type_t) ((stack - 1)->float_value OP stack->uint_value); \
        else if (stack->type == VM_TYPE_INT) \
            (stack - 1)->uint_value = (vm_type_t) ((stack - 1)->float_value OP stack->int_value); \
        else if (stack->type == VM_TYPE_FLOAT) \
            (stack - 1)->uint_value = (vm_type_t) ((stack - 1)->float_value OP stack->float_value); \
    } else { \
        vm_error(state, "Operator %s has not been defined for stack type %d", #OP, (stack - 1)->type); \
        vm_exit(state, 1); \
    } \
    (stack - 1)->type = VM_TYPE_UINT; \
}

/// put an int value on the stack which is interpreted as a status register value containing condition code to be used by a branch instruction.
INSTR(cmp) {
    instr_sub(state);
}
INSTR(eq) {
    {
        USE_STACK();
        if ((stack - 1)->type == VM_TYPE_STRING && stack->type == VM_TYPE_STRING) {
            str_eq(state);
            return;
        }
    }

    COMPARE_OPERATOR(==);
}
INSTR(ne) {
    COMPARE_OPERATOR(!=);
}
INSTR(lt) {
    COMPARE_OPERATOR(<);
}
INSTR(gt) {
    COMPARE_OPERATOR(>);
}
INSTR(le) {
    COMPARE_OPERATOR(<=);
}
INSTR(ge) {
    COMPARE_OPERATOR(>=);
}

#define USE_TYPECHECK() \
	USE_STACK(); \
	if (stack->type != (stack - 1)->type) { \
        AJS_STACK(-1);\
        *(stack - 1) = (vm_value_t) { .uint_value = 0, .type = VM_TYPE_UINT }; \
        return; \
    }

INSTR(cmp_id) {
	USE_TYPECHECK();
	instr_cmp(state);
}

INSTR(eq_id) {
	USE_TYPECHECK();
	instr_eq(state);
}

INSTR(ne_id) {
	USE_TYPECHECK();
	instr_ne(state);
}

INSTR(lt_id) {
	USE_TYPECHECK();
	instr_lt(state);
}

INSTR(gt_id) {
	USE_TYPECHECK();
	instr_gt(state);
}

INSTR(le_id) {
	USE_TYPECHECK();
	instr_le(state);
}

INSTR(ge_id) {
	USE_TYPECHECK();
	instr_ge(state);
}
