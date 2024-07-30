#ifndef _VALUE
#define _VALUE

typedef enum {
    INT_TYPE, DOUBLE_TYPE, STR_TYPE, CONS_TYPE, NULL_TYPE, PTR_TYPE,
    OPEN_TYPE, CLOSE_TYPE, BOOL_TYPE, SYMBOL_TYPE,
    
    // Types below are only for bonus work
    OPENBRACKET_TYPE, CLOSEBRACKET_TYPE, DOT_TYPE, SINGLEQUOTE_TYPE,

    // Types below are new for define/lambda portion
    VOID_TYPE, CLOSURE_TYPE,

    // Type below is new for primitive portion
    PRIMITIVE_TYPE,

    // Type below is new for final portion
    UNSPECIFIED_TYPE
} valueType;

struct Value {
    valueType type;
    union {
        int i;
        double d;
        char *s;
        void *p;
        struct ConsCell {
            struct Value *car;
            struct Value *cdr;
        } c;
        // For purposes of this project a closure is just another type of value,
        // containing everything needed to execute a user-defined function: (1)
        // a list of formal parameter names; (2) a pointer to the function body;
        // (3) a pointer to the environment frame in which the function was
        // created.
        struct Closure {
            struct Value *paramNames;
            struct Value *functionCode;
            struct Frame *frame;
        } cl;
        
        // A primitive style function; just a pointer to it, with the right
        // signature (pf = primitive function)
        struct Value *(*pf)(struct Value *);
    };
};

typedef struct Value Value;


// A frame is a linked list of bindings, and a pointer to another frame.  A
// binding is a variable name (represented as a string), and a pointer to the
// Value it is bound to. I'm just going to put these in a flat list, with every
// other element being a symbol, and the following element being the value. I
// could do this via yet another data structure, but I think this will
// ultimately take less coding. It also will require less modification of
// existing code.

struct Frame {
    struct Value *bindings;
    struct Frame *parent;
};

typedef struct Frame Frame;




#endif
