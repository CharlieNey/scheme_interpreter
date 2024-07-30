#include <stdbool.h>
#include "value.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "linkedlist.h"
#include "talloc.h"



// Create a new NULL_TYPE value node.
Value *makeNull() {
    Value* val = talloc(sizeof(Value));
    val->type = NULL_TYPE;
    return val;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr) {
    Value* newCons = talloc(sizeof(Value));
    newCons->type = CONS_TYPE;
    newCons->c.car = newCar;
    newCons->c.cdr = newCdr;
    return newCons;
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list) {
    while (list->type != NULL_TYPE)
    {
        printf("%d\n", list->c.car->i);
        list = list->c.cdr;
    }
    
}


// Return a new list that is the reverse of the one that is passed in. All
// content within the list should be duplicated; there should be no shared
// memory whatsoever between the original list and the new one.
//
// FAQ: What if there are nested lists inside that list?
// ANS: There won't be for this assignment. There will be later, but that will
// be after we've got an easier way of managing memory.
Value *reverse(Value *list) {
    Value *revList = makeNull();

    while(list->type != NULL_TYPE) {
        revList = cons(list->c.car, revList);
        list = list->c.cdr;
    }
    return revList;
}


// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list) {
    assert(list != NULL);
    //assert(list->c.cdr != NULL);
    return list->c.car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list) {
    assert(list != NULL);
    assert(list->c.cdr != NULL);
    return list->c.cdr;
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value) {
    assert(value != NULL);

    if (value->type == NULL_TYPE) {
        return true;
    }
    else {
        return false;
    }
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value) {
    assert(value != NULL);
    int len = 0;

    while (value->type != NULL_TYPE) {
        len++;
        value = cdr(value);
    }
    return len;
}