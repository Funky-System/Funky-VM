#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "instructions.h"
#include "../../../include/funkyvm/funkyvm.h"
#include "../error_handling.h"

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

/**!
 * instruction: add
 * category: Arithmetics
 * opcode: "0x30"
 * description: Adds two values.
 * extra_info: Combines two values using the 'add' operator; <i>first</i> <code>+</code> <i>second</i>.
 *             <p>If <i>first</i> is an array, then this operation adds <i>second</i> to the end of that array.</p>
 *             <p>If <i>second</i> is an array, then this operation adds <i>first</i> to the beginning of that array.</p>
 *             <p>If both <i>first</i> and <i>second</i> are an array, then this operation equals <code>arr.concat</code>.</p>
 *             <p>If <i>first</i> or <i>second</i> is a string, then both <i>first</i> and <i>second</i> are converted to string (if they didn't already were) and concatenated with <code>str.concat</code>.</p>
 * stack_pre:
 *   - type: any
 *     description: Second value
 *   - type: any
 *     description: First value
 * stack_post:
 *   - type: undefined
 *     description: Result is of the same type of the most specific type of <i>first</i> or <i>second</i>
 */
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
            if (conv_str_rel(state, 0)) return;
        }
        instr_strcat(state);
        return;
    }

    if (stack->type == VM_TYPE_STRING) {
        // rhs is string, this is string concatenation
        if ((stack - 1)->type != VM_TYPE_STRING) {
            if (conv_str_rel(state, -1)) return;
        }
        instr_strcat(state);
        return;
    }

    ARITH_OPERATOR(+);
}

/**!
 * instruction: sub
 * category: Arithmetics
 * opcode: "0x31"
 * description: Subtracts two values.
 * extra_info: Combines two values using the 'subtract' operator; <i>first</i> <code>-</code> <i>second</i>.
 * stack_pre:
 *   - type: any
 *     description: Second value
 *   - type: any
 *     description: First value
 * stack_post:
 *   - type: undefined
 *     description: Result is of the same type of the most specific type of <i>first</i> or <i>second</i>
 */
INSTR(sub) {
    ARITH_OPERATOR(-);
}

/**!
 * instruction: mul
 * category: Arithmetics
 * opcode: "0x32"
 * description: Multiplies two values.
 * extra_info: Combines two values using the 'multiply' operator; <i>first</i> <code>*</code> <i>second</i>.
 * stack_pre:
 *   - type: any
 *     description: Second value
 *   - type: any
 *     description: First value
 * stack_post:
 *   - type: undefined
 *     description: Result is of the same type of the most specific type of <i>first</i> or <i>second</i>
 */
INSTR(mul) {
    ARITH_OPERATOR(*);
}

/**!
 * instruction: div
 * category: Arithmetics
 * opcode: "0x33"
 * description: Divides two values.
 * extra_info: Combines two values using the 'division' operator; <i>first</i> <code>/</code> <i>second</i>.
 * stack_pre:
 *   - type: any
 *     description: Second value
 *   - type: any
 *     description: First value
 * stack_post:
 *   - type: undefined
 *     description: Result is of the same type of the most specific type of <i>first</i> or <i>second</i>
 */
INSTR(div) {
    ARITH_OPERATOR(/);
}

/**!
 * instruction: mod
 * category: Arithmetics
 * opcode: "0x34"
 * description: Modulus operator.
 * extra_info: Combines two values using the 'modulus' operator; <i>first</i> <code>%</code> <i>second</i>.
 * stack_pre:
 *   - type: any
 *     description: Second value
 *   - type: any
 *     description: First value
 * stack_post:
 *   - type: undefined
 *     description: Result is of the same type of the most specific type of <i>first</i> or <i>second</i>
 */
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

/**!
 * instruction: pow
 * category: Arithmetics
 * opcode: "0x41"
 * description: Exponentiation operator.
 * extra_info: Computes the value of base raised to the power exponent; <i>base</i> <code>**</code> <i>exponent</i>.
 * stack_pre:
 *   - type: any
 *     description: Exponent value
 *   - type: any
 *     description: Base value
 * stack_post:
 *   - type: undefined
 *     description: Result is of the same type of the most specific type of <i>first</i> or <i>second</i>
 */
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
    } else if (stack->type == VM_TYPE_INT) {
        stack->int_value = (vm_type_signed_t) -stack->int_value;
    } else if (stack->type == VM_TYPE_FLOAT) {
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
    } else if (stack->type == VM_TYPE_INT) {
        stack->int_value = (vm_type_signed_t) ~stack->uint_value;
    } else {
        vm_error(state, "Unary operator ~ has not been defined for stack type %d", (stack - 1)->type);
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
        /*(stack - 1)->uint_value = (vm_type_t) (1 OP 0);*/ \
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
        if ((stack - 1)->type == VM_TYPE_STRING || stack->type == VM_TYPE_STRING) {
            str_eq(state);
            return;
        }
        if ((stack - 1)->type == VM_TYPE_ARRAY && stack->type == VM_TYPE_ARRAY) {
            arr_eq(state);
            return;
        }
        if ((stack - 1)->type == VM_TYPE_EMPTY || stack->type == VM_TYPE_EMPTY) {
            if ((stack - 1)->type == VM_TYPE_EMPTY && stack->type == VM_TYPE_EMPTY) {
                (stack - 1)->uint_value = 1;
            } else {
                (stack - 1)->uint_value = 0;
            }
            (stack - 1)->type = VM_TYPE_UINT;
            AJS_STACK(-1);
            return;
        }
    }

    COMPARE_OPERATOR(==);
}
INSTR(ne) {
    {
        USE_STACK();
        if ((stack - 1)->type == VM_TYPE_STRING || stack->type == VM_TYPE_STRING) {
            str_ne(state);
            return;
        }
        if ((stack - 1)->type == VM_TYPE_ARRAY && stack->type == VM_TYPE_ARRAY) {
            arr_ne(state);
            return;
        }
        if ((stack - 1)->type == VM_TYPE_EMPTY || stack->type == VM_TYPE_EMPTY) {
            if ((stack - 1)->type == VM_TYPE_EMPTY && stack->type == VM_TYPE_EMPTY) {
                (stack - 1)->uint_value = 0;
            } else {
                (stack - 1)->uint_value = 1;
            }
            (stack - 1)->type = VM_TYPE_UINT;
            AJS_STACK(-1);
            return;
        }
    }

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
