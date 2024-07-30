
#include <stdio.h>
#include <assert.h>
#include "linkedlist.h"
#include "value.h"
#include "talloc.h"

// Adds a token to a parse tree
Value *addToParseTree(Value *tree, int *depth, Value *token) {
    // If token is an open paren, increments depth
    if (token->type == OPEN_TYPE) {
        tree = cons(token, tree);
        (*depth)++;
        return tree;
    }
    else {
        tree = cons(token, tree);
        return tree;
    }
}

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens) {
    //STEP 1: Initialize an empty stack
    Value *tree = makeNull();
    int depth = 0;

    Value *current = tokens;

    //STEP 2: While there are more tokens
    while (current->type != NULL_TYPE) {
        //STEP 4: If the token is anything other than a close paren, push it onto the stack.
        if (car(current)->type != CLOSE_TYPE) {
            Value *token = car(current);
            tree = addToParseTree(tree, &depth, token);
            current = cdr(current);
        }
        // STEP 5: If the token is a close paren 
        // Push that list back on the stack.
        else {
            Value *cur_node = tree;
            Value *list_to_add = makeNull();
            // start popping items from your stack until you pop off an open paren (and form a list of them as you go).
            while (cur_node->type != NULL_TYPE && car(cur_node)->type != OPEN_TYPE) {
                Value *token = car(cur_node);
                list_to_add = cons(token, list_to_add);
                cur_node = cdr(cur_node);
            }
            // If it reaches bottom of tree stack without hitting an open paren
            if (cur_node->type == NULL_TYPE) {
                printf("Syntax error: too many close parentheses.\n");
                texit(1);
            }
            tree = cdr(cur_node);
            tree = cons(list_to_add, tree);
            depth--;
            current = cdr(current);
        }
    }
    
    //Error Case: Depth should always be at 0 when returning
    if (depth != 0) {
        printf("Syntax error: not enough close parentheses.\n");
        texit(1);
    }
    return reverse(tree);
}

void printTree(Value *tree) {
    Value *currentNote = tree;
    while (currentNote->type != NULL_TYPE) {
        if (currentNote->type != CONS_TYPE) {
            switch (currentNote->type) {
                case BOOL_TYPE:
                    if (currentNote->i == 0) {
                        printf(". #f");
                    }
                    else {
                        printf(". #t");
                    }
                    break;
                case INT_TYPE:
                    printf(". %i", currentNote->i);
                    break;
                case DOUBLE_TYPE:
                    printf(". %f", currentNote->d);
                    break;
                case STR_TYPE:
                    printf(". \"%s\"", currentNote->s);
                    break;
                case SYMBOL_TYPE:
                    printf(". %s", currentNote->s);
                    break;
                default:
                    break;
            }
            break;
        }
       
        if (car(currentNote)->type == CONS_TYPE) {
            printf("(");
            printTree(car(currentNote));
            printf(") ");
        }
        else if (car(currentNote)->type == NULL_TYPE) {
            printf("()");
        }
        else if (cdr(currentNote)->type == NULL_TYPE) {
            Value *carVal = car(currentNote);
            int car_type = carVal->type;
           
            switch (car_type) {
                case BOOL_TYPE:
                    if (carVal->i == 0) {
                        printf("#f");
                    }
                    else {
                        printf("#t");
                    }
                    break;
                case INT_TYPE:
                    printf("%i", carVal->i);
                    break;
                case DOUBLE_TYPE:
                    printf("%f", carVal->d);
                    break;
                case STR_TYPE:
                    printf("\"%s\"", carVal->s);
                    break;
                case SYMBOL_TYPE:
                    printf("%s", carVal->s);
                    break;
            }
        }
        else {
            Value *carVal = car(currentNote);
            int car_type = carVal->type;
           
            switch (car_type) {
                case BOOL_TYPE:
                    if (carVal->i == 0) {
                        printf("#f ");
                    }
                    else {
                        printf("#t ");
                    }
                    break;
                case INT_TYPE:
                    printf("%i ", carVal->i);
                    break;
                case DOUBLE_TYPE:
                    printf("%f ", carVal->d);
                    break;
                case STR_TYPE:
                    printf("\"%s\" ", carVal->s);
                    break;
                case SYMBOL_TYPE:
                    printf("%s ", carVal->s);
                    break;
            }
        }
        currentNote = cdr(currentNote);
    }
}

