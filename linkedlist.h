#include <stdbool.h>
#include "value.h"

#ifndef _LINKEDLIST
#define _LINKEDLIST

// Create a new NULL_TYPE value node.
Value *makeNull();

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr);

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list);

// Return a new list that is the reverse of the one that is passed in. No stored
// data within the linked list should be duplicated; rather, a new linked list
// of CONS_TYPE nodes should be created, that point to items in the original
// list.
Value *reverse(Value *list);

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list);

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list);

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value);

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value);


#endif
