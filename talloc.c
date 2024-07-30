#include <stdlib.h>
#include "value.h"
#include <assert.h>

Value *head = NULL;

// Create a new NULL_TYPE value node.
Value *makeNull2() {
    Value* val = malloc(sizeof(Value));
    val->type = NULL_TYPE;
    return val;
}

// Create a new CONS_TYPE value node.
Value *cons2(Value *newCar, Value *newCdr) {
    Value* newCons = malloc(sizeof(Value));
    newCons->type = CONS_TYPE;
    newCons->c.car = newCar;
    newCons->c.cdr = newCdr;
    return newCons;
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car2(Value *list) {
    assert(list != NULL);
    return list->c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr2(Value *list) {
    assert(list != NULL);
    return list->c.cdr;
}


// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size) {
    if (head == NULL) {
        head = malloc(sizeof(Value));
        head->type = NULL_TYPE;
    }

    Value *temp = malloc(sizeof(Value));
    temp->type = PTR_TYPE;
    temp->p = malloc(size);
    head = cons2(temp, head);
    return head->c.car->p;    
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree() {
    Value *temp;
    if (head != NULL) {
        while (head->type != NULL_TYPE) {
            free(head->c.car->p);
            free(head->c.car);
            temp = head->c.cdr;
            free(head);
            head = temp;
        }
    free(head);
    head = NULL;
    }
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status) {
    tfree();
    exit(0);
}
