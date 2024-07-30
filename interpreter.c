#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "linkedlist.h"
#include "talloc.h"
#include "interpreter.h"
#include "value.h"
#include "parser.h"


void evaluationError(int error) {
    printf("Evaluation error: \n");
    texit(1);
}

//basic helpers for bool values
Value *value_true() {
    // Function that returns a true boolean Value
    Value *valTrue = talloc(sizeof(Value));
    valTrue->type = BOOL_TYPE;
    valTrue->i = 1;
    return valTrue;
}

Value *value_false() {
    // Function that returns a false boolean Value
    Value *valFalse = talloc(sizeof(Value));
    valFalse->type = BOOL_TYPE;
    valFalse->i = 0;
    return valFalse;
}


//Search function for symbols
Value *symbol_search(Value *symbol, Frame *frame) {
    //Continuosly move up Frames until none are left
    Frame *curFrame = frame;
    while (frame != NULL) {
        Value *bindings = frame->bindings;
        while (bindings->type != NULL_TYPE) {
            //Value *firstSymbol_cons = car(bindings);
            Value *firstSymbol = car(car(bindings));
            assert(firstSymbol->type == SYMBOL_TYPE);
            
            // Checks if string member of binding matches that of input symbol
            if (strcmp(firstSymbol->s, symbol->s) == 0) {
                return car(cdr(car(bindings)));
            }
            // Otherwise continues search
            else {
                bindings = cdr(bindings);
            }
        }
        frame = frame->parent;
    }
    // If symbol not found, print evaluation error and exit
    evaluationError(1);
    return symbol;
}



//Evaluates If Statements
Value *eval_if(Value *args, Frame *frame) {
    //Input Checker:
    if (args->type != CONS_TYPE) 
        evaluationError(1);
    if (cdr(args)->type != CONS_TYPE) 
        evaluationError(1);
    if (cdr(cdr(args))->type != CONS_TYPE) 
        evaluationError(1);
    
    Value *bool_exp = eval(car(args), frame);

    if (bool_exp->type == BOOL_TYPE) {
        //True Result
        if (bool_exp->i == 1) 
            return eval(car(cdr(args)), frame);
        //False Result
        else 
            return eval(car(cdr(cdr(args))), frame);
    }
    else 
        evaluationError(0);
    return args;
}

Value *eval_each(Value *args, Frame *frame) {
    // Evaluates every argument, and returns the list
    Value *currentNode = args;
    Value *evaled_args = makeNull();
    
    while (currentNode->type != NULL_TYPE) {
        Value *arg = car(currentNode);
        Value *evaled_arg = eval(arg, frame);
        
        evaled_args = cons(evaled_arg, evaled_args);
        currentNode = cdr(currentNode);
    }
    // Reverse list after cons created the evaluated list in reverse
    evaled_args = reverse(evaled_args);
    
    return evaled_args;
}

// Evaluates arguments of a let statement and returns resulting Value
Value *eval_let(Value *args, Frame *frame) {
    // Create new frame and set input frame to be parent frame
    Frame *let_frame = talloc(sizeof(Frame));
    let_frame->parent = frame;
    // Create new linked list to store bindings created in let statement
    Value *let_bindings = makeNull();
    
    Value *currentNode = car(args);
    // Check valid input structure
    if (car(currentNode)->type != CONS_TYPE) 
        evaluationError(2);
    // Iterate through arguments...
    while (currentNode->type != NULL_TYPE) {
        // Sets up singular var and value pair
        Value *node_to_eval = car(cdr(car(currentNode)));
        Value *pointer = eval(node_to_eval, frame);
 
        Value *val = cons(pointer,makeNull());
        Value *symbol_val = car(car(currentNode));
        if (symbol_val->type == SYMBOL_TYPE) 
            val = cons(symbol_val, val);
        else 
            evaluationError(12);
        let_bindings = cons(val, let_bindings);
        
        currentNode = cdr(currentNode);
    }
    
    let_frame->bindings = let_bindings;
    
    Value *evaled = eval_each(cdr(args), let_frame);

    if (cdr(evaled)->type == NULL_TYPE) {
        return car(evaled);
    }
    else {
        return car(cdr(evaled));
    }

}


Value *eval_letStar(Value *args, Frame *frame) {
    Frame *cur_frame = frame;
    Value *currentNode = car(args);
    if (car(currentNode)->type != CONS_TYPE) {
        evaluationError(1);
    }
    //Create each binding
    while (currentNode->type != NULL_TYPE) {
        Value *let_binding = makeNull();
        Value *node_to_eval = car(cdr(car(currentNode)));
        Value *val = cons(eval(node_to_eval, cur_frame), makeNull());
        //check for symbol
        if (car(car(currentNode))->type == SYMBOL_TYPE) {
            //assign value
            val = cons(car(car(currentNode)), val);
        }
        else {
            evaluationError(1);
        }
        let_binding = cons(val, let_binding);
        
        Frame *let_frame = talloc(sizeof(Frame));
        let_frame->parent = cur_frame;
        let_frame->bindings = let_binding;
        cur_frame = let_frame;
        currentNode = cdr(currentNode);
    }
    
    //Evaluate body in the current frame
    return eval(car(cdr(args)), cur_frame);
}

Value *eval_letRec(Value *args, Frame *frame) {
    //Need a new frame and bindings
    Frame *let_frame = talloc(sizeof(Frame));
    let_frame->parent = frame;
    Value *let_bindings = makeNull();
    
    Value *currentNode = car(args);
    if (car(currentNode)->type != CONS_TYPE) {
        evaluationError(2);
    }
    while (currentNode->type != NULL_TYPE) {
        // Sets up singular var and value pair
        Value *node_to_eval = car(cdr(car(currentNode)));
 
        Value *val = cons(eval(node_to_eval, let_frame),makeNull());
        Value *symbol_val = car(car(currentNode));
        if (symbol_val->type == SYMBOL_TYPE) {
            val = cons(symbol_val, val);
        }
        else {
            evaluationError(12);
        }
        let_bindings = cons(val, let_bindings);
        currentNode = cdr(currentNode);
    }
    
    let_frame->bindings = let_bindings;
    return eval(car(cdr(args)), let_frame);
}



//part 7
//Evaluates define
Value *eval_define(Value *args, Frame *frame) {
    Value *var = car(args);
    //input structure checker:
    if (car(cdr(args))== NULL) {
        evaluationError(1);
    }
    if (cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(6);
    }
    
    Value *eval_expr = eval(car(cdr(args)), frame);
    Value *new_bindings = makeNull();
    new_bindings = cons(eval_expr, new_bindings);
    new_bindings = cons(var, new_bindings);
    
    frame->bindings = cons(new_bindings, frame->bindings);
    
    // Returns void Value for interpreter to ignore
    Value *void_val = talloc(sizeof(Value));
    void_val->type = VOID_TYPE;
    
    return void_val;
}

//Part 7
//Evaluates Lambdas
Value *eval_lambda(Value *args, Frame *frame) {
    if (args->type != CONS_TYPE) {
        evaluationError(7);
    }
  
    struct Closure cl;
    cl.paramNames = car(args);
    cl.functionCode = car(cdr(args));
    cl.frame = frame;
    
    Value* closure = talloc(sizeof(Value));
    closure->type = CLOSURE_TYPE;
    closure->cl = cl;
    
    return closure;
}


Value *apply(Value *function, Value *args) {
    // Applies given function to multiple arguments

    assert(function->type == CLOSURE_TYPE || function->type == PRIMITIVE_TYPE);
    
    if (function->type == PRIMITIVE_TYPE) {
        return function->pf(args);
    }
    struct Closure closure = function->cl;
    
    Frame *frame = talloc(sizeof(Frame));
    frame->parent = closure.frame;
    
    Value *new_bindings = makeNull();
    Value *currentNode = args;
    Value *params = closure.paramNames;
    Value *cur_param = params;
    
    while (currentNode->type != NULL_TYPE) {
        if (cur_param->type == NULL_TYPE) {
            evaluationError(8);
        }
        
        Value *list = makeNull();
        list = cons(car(currentNode), list);
        list = cons(car(cur_param), list);
        
        new_bindings = cons(list, new_bindings);
        
        currentNode = cdr(currentNode);
        cur_param = cdr(cur_param);
    }
    if (cur_param->type != NULL_TYPE) {
        evaluationError(9);
    }
    
    frame->bindings = new_bindings;
    Value *body = closure.functionCode;
    
    return eval(body, frame);
}


Value *eval_set(Value *args, Frame *frame) {
    Value *symbol = car(args);
    Value *new_val = eval(car(cdr(args)), frame);
    
    while (frame != NULL) {
        Value *bindings = frame->bindings;
        while (bindings == NULL) {
            if (frame->parent == NULL) {
                evaluationError(3);
            }
            frame = frame->parent;
            bindings = frame->bindings;
        }
        while (bindings->type != NULL_TYPE) {
            Value *firstSymbol_cons = car(bindings);
            Value *firstSymbol = car(firstSymbol_cons);
            
            assert(firstSymbol->type == SYMBOL_TYPE);
            if (!strcmp(firstSymbol->s, symbol->s)) {
                struct ConsCell cons_cell = firstSymbol_cons->c;
                cons_cell.cdr = cons(new_val, makeNull());
                firstSymbol_cons->c = cons_cell;
                
                Value *void_val = talloc(sizeof(Value));
                void_val->type = VOID_TYPE;
                return void_val;
            }
            else {
                bindings = cdr(bindings);
            }
        }
        frame = frame->parent;
    }
    evaluationError(3);
    return symbol;
}

Value *eval_begin(Value *args, Frame *frame) {
    Value *evaledArgs = eval_each(args, frame);
    
    Value *currentVal = evaledArgs;
    while (currentVal->type != NULL_TYPE) {
        if (cdr(currentVal)->type == NULL_TYPE) {
            return car(currentVal);
        }
        else {
            currentVal = cdr(currentVal);
        }
    }
    Value *void_val = talloc(sizeof(Value));
    void_val->type = VOID_TYPE;
    return void_val;
}

Value *eval_and(Value *args, Frame *frame) {
    while (args->type != NULL_TYPE) {
        Value *bool_val = eval(car(args), frame);
        if (bool_val->type != BOOL_TYPE) {
            evaluationError(1); 
        }
        else if (bool_val->i == 0) {
            return value_false();
        }
        else {
            args = cdr(args);
        }
    }
    return value_true();
}

Value *eval_or(Value *args, Frame *frame) {
    while (args->type != NULL_TYPE) {
        Value *bool_val = eval(car(args), frame);
        if (bool_val->type != BOOL_TYPE) {
            evaluationError(1); 
        }
        else if (bool_val->i == 1) {
            return value_true();
        }
        else {
            args = cdr(args);
        }
    }
    return value_false();
}

//Part 8 Helpers
Value *prim_add(Value *args) {
    double result = 0;
    while (args->type != NULL_TYPE) {
        Value *currentNode = car(args);
        if (currentNode->type != INT_TYPE) {
            if (currentNode->type != DOUBLE_TYPE) {
                evaluationError(1);
            }
            result += currentNode->d;
            args = cdr(args);
        }
        else {
            result += currentNode->i;
            args = cdr(args);
        }
    }
    Value *resultVal = talloc(sizeof(Value));
    if ((int)result == result){
        resultVal->type = INT_TYPE;
        resultVal->i = result;
    }
    else {
        resultVal->type = DOUBLE_TYPE;
        resultVal->d = result;
    }
    return resultVal;
}

Value *prim_car(Value *args) {
    if (args->type == NULL_TYPE){
        evaluationError(10);
    }
    assert(args->type == CONS_TYPE);
    if (car(args)->type != CONS_TYPE)
        evaluationError(10);
    
    if (cdr(args)->type != NULL_TYPE) {
        evaluationError(10);
    }
    Value *argument = car(args);
    return car(argument);
}

Value *prim_cdr(Value *args) {
    if (args->type == NULL_TYPE){
        evaluationError(1);
    }
    assert(args->type == CONS_TYPE);
    if (cdr(args)->type != NULL_TYPE) {
        evaluationError(1);
    }
    Value *lst = car(args);
    if (lst->type != CONS_TYPE) {
        evaluationError(1);
    }
    return cdr(lst);
}

Value *prim_cons(Value *args) {
    //Input handling:
    if (args->type == NULL_TYPE) {
        evaluationError(1);
    }
    if (car(args)->type == NULL_TYPE) {
        evaluationError(1);
    }
    if (cdr(args)->type == NULL_TYPE) {
        evaluationError(1);
    }
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(1);
    }
    Value *arg1 = car(args);
    Value *arg2;
    
    //Assign arg2
    if (cdr(args)->type == CONS_TYPE) {
        arg2 = car(cdr(args));
    }
    else {
        arg2 = cdr(args);
    }

    //add together
    Value *resultVal = cons(arg1, arg2);
    return resultVal;
}



Value *prim_null(Value *args) {
    if (args->type == NULL_TYPE) {
        evaluationError(10);
    }
    assert(args->type == CONS_TYPE);
    if (cdr(args)->type != NULL_TYPE) {
        evaluationError(10);
    }
    if (car(args)->type == NULL_TYPE) {
        return value_true();
    }
    else {
        return value_false();
    }
}



void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    // Add primitive functions to top-level bindings list
    Value *funcVal = talloc(sizeof(Value));
    funcVal->type = PRIMITIVE_TYPE;
    funcVal->pf = function;
    // Add binding of name to value
    Value *symbol = talloc(sizeof(Value));
    symbol->type = SYMBOL_TYPE;
    symbol->s = name;
    
    Value *binding = makeNull();
    binding = cons(funcVal, binding);
    binding = cons(symbol, binding);
    
    frame->bindings = cons(binding, frame->bindings);
}



Value *prim_equal(Value *args) {
    // Must have two arguments
    if (args->type == NULL_TYPE) {
        evaluationError(1);
    }
    if (car(args)->type == NULL_TYPE) {
        evaluationError(1);
    }
    if (cdr(args)->type == NULL_TYPE) {
        evaluationError(1);
    }
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(1);
    }
    
    double arg1;
    double arg2;
    
    if (car(args)->type == INT_TYPE) {
        Value *argument = car(args);
        arg1 = (double) argument->i;
    }
    else if (car(args)->type == DOUBLE_TYPE) {
        Value *argument = car(args);
        arg1 = argument->d;
    }
    else {
        evaluationError(10);
    }
    
    if (car(cdr(args))->type == INT_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = (double) argument1->i;
    }
    else if (car(cdr(args))->type == DOUBLE_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = argument1->d;
    }
    else {
        evaluationError(10);
    }
    
    if (arg1 == arg2) {
        return value_true();
    }
    else {
        return value_false();
    }
}

Value *prim_greater(Value *args) {
    if (args->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (car(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(10);
    }
    
    double arg1;
    double arg2;
    
    //Setting up first val
    if (car(args)->type == INT_TYPE) {
        Value *argument = car(args);
        arg1 = (double) argument->i;
    }
    else if (car(args)->type == DOUBLE_TYPE) {
        Value *argument = car(args);
        arg1 = argument->d;
    }
    else {
        evaluationError(10);
    }
    
    //second Val
    if (car(cdr(args))->type == INT_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = (double) argument1->i;
    }
    else if (car(cdr(args))->type == DOUBLE_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = argument1->d;
    }
    else {
        evaluationError(10);
    }
    
    //Actual Comparison
    if (arg1 > arg2) {
        return value_true();
    }
    else {
        return value_false();
    }
}

Value *prim_less(Value *args) {
    if (args->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (car(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == NULL_TYPE) {
        evaluationError(10);
    }
    if (cdr(args)->type == CONS_TYPE && cdr(cdr(args))->type != NULL_TYPE) {
        evaluationError(10);
    }
    
    double arg1;
    double arg2;
    
    //First val
    if (car(args)->type == INT_TYPE) {
        Value *argument = car(args);
        arg1 = (double) argument->i;
    }
    else if (car(args)->type == DOUBLE_TYPE) {
        Value *argument = car(args);
        arg1 = argument->d;
    }
    else {
        evaluationError(10);
    }
    
    //second val
    if (car(cdr(args))->type == INT_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = (double) argument1->i;
    }
    else if (car(cdr(args))->type == DOUBLE_TYPE) {
        Value *argument1 = car(cdr(args));
        arg2 = argument1->d;
    }
    else {
        evaluationError(10);
    }
    //comparison
    if (arg1 < arg2) {
        return value_true();
    }
    else {
        return value_false();
    }
}


Value *prim_subtract(Value *args) {
    double result = 0;
    
    Value *currentNode = car(args);
    if (currentNode->type != INT_TYPE) {
        if (currentNode->type != DOUBLE_TYPE) {
            evaluationError(10);
        }
        result = result + currentNode->d;
        args = cdr(args);
    }
    else {
        result = result + currentNode->i;
        args = cdr(args);
    }
    
    while (args->type != NULL_TYPE) {
        Value *currentNode = car(args);
        if (currentNode->type != INT_TYPE) {
            if (currentNode->type != DOUBLE_TYPE) {
                evaluationError(10);
            }
            result = result - currentNode->d;
            args = cdr(args);
        }
        else {
            result = result - currentNode->i;
            args = cdr(args);
        }
    }
    // Return value of result
    Value *resultVal = talloc(sizeof(Value));
    if ((int)result == result){
        resultVal->type = INT_TYPE;
        resultVal->i = result;
    }
    else {
        resultVal->type = DOUBLE_TYPE;
        resultVal->d = result;
    }
    return resultVal;
}

void interpret(Value *tree) {
    Frame *global = talloc(sizeof(Frame));
    global->bindings = makeNull();
    global->parent = NULL;
  

    bind("car", prim_car, global);
    bind("cdr", prim_cdr, global);
    bind("cons", prim_cons, global);
    bind("+", prim_add, global);
    bind("null?", prim_null, global);
    bind("=", prim_equal, global);
    bind(">", prim_greater, global);
    bind("<", prim_less, global);
    bind("-", prim_subtract, global);

    while (tree->type != NULL_TYPE) {
        assert(tree->type == CONS_TYPE);
        Value *result = eval(car(tree), global);
        switch (result->type) {
            case BOOL_TYPE:
                if (result->i == 0) {
                    printf("#f\n");
                }
                else {
                    printf("#t\n");
                }
                break;
            case INT_TYPE:
                printf("%i\n", result->i);
                break;
            case DOUBLE_TYPE:
                printf("%f\n", result->d);
                break;
            case STR_TYPE:
                printf("\"%s\"\n", result->s);
                break;
            case SYMBOL_TYPE:
                printf("%s\n", result->s);
                break;
            case CONS_TYPE:
                printf("(");
                printTree(result);
                printf(")\n");
                break;
            case CLOSURE_TYPE:
                printf("#<procedure>\n");
                break;
            case PRIMITIVE_TYPE:
                printf("#<procedure>\n");
                break;
            case NULL_TYPE:
                printf("()\n");
                break;
            default:
                break;    
        }
        
        tree = cdr(tree);
    
    }
}

Value *eval(Value *tree, Frame *frame) {
    Value *result;
    switch (tree->type) {
        case INT_TYPE: {
            result = tree;
            break;
        }
        case DOUBLE_TYPE: {
            result = tree;
            break;
        }
        case STR_TYPE: {
            result = tree;
            break;
        }
        case BOOL_TYPE: {
            result = tree;
            break;
        }
        // Looks for symbol in frames
        case SYMBOL_TYPE: {
            result = symbol_search(tree, frame);
            break;
        }
        case CONS_TYPE: {
            Value *first = car(tree);
            Value *args = cdr(tree);
            //// Error checking on first omitted...
            // Checking first argument...
            // If the first argument is a string...
            if (first->type == SYMBOL_TYPE) {
                if (strcmp(first->s, "if") == 0) 
                    result = eval_if(args, frame);

                else if (strcmp(first->s, "let") == 0) 
                    result = eval_let(args, frame);

                else if (strcmp(first->s, "quote") == 0) {
                    result = car(args);
                }
                else if (strcmp(first->s, "define") == 0) {
                    result = eval_define(args, frame);
                }

                else if (strcmp(first->s, "lambda") == 0) {
                    result = eval_lambda(args, frame);
                }
                 else if (strcmp(first->s, "set!") == 0) {
                    result = eval_set(args, frame);
                }
                
                else if (strcmp(first->s, "begin") == 0) {
                    result = eval_begin(args, frame);
                }
                
                else if (strcmp(first->s, "and") == 0) {
                    result = eval_and(args, frame);
                }
                
                else if (strcmp(first->s, "or") == 0) {
                    result = eval_or(args, frame);
                }
                else if (strcmp(first->s, "let*") == 0) {
                    result = eval_letStar(args, frame);
                }
                
                else if (strcmp(first->s, "letrec") == 0) {
                    result = eval_letRec(args, frame);
                }
                else  {
                //Eval the operator and apply to each arg
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = eval_each(args, frame);
                return apply(evaledOperator, evaledArgs);
                }
            }
            else {
                Value *evaledOperator = eval(first, frame);
                Value *evaledArgs = eval_each(args, frame);
                return apply(evaledOperator, evaledArgs);
            }
            break;
        }
        default:
        break;
    }
    return result;
}
