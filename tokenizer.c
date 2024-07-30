
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "talloc.h"
#include "linkedlist.h"
#include "value.h"

//Possible Tokens: boolean, integer, double, string, symbol, open, close

//BASIC HELPERS
int is_digit(char input){
     if (input == '0' || input == '1' || input == '2' || input == '3' || input == '4' || input == '5' || input == '6' || input == '7' || input == '8' || input == '9') 
        return 1;
    else 
        return 0;
}

int is_letter(char input) {
    if ((input >= 'a' && input <= 'z') || (input >= 'A' && input <= 'Z')) 
        return 1;
    else 
        return 0;
}

int is_bool(char *input) {
    int size = 0;
    if (input[0] == '#')
        return 1;
    else 
        return 0;
}

int is_uinteger(char *input) {
    int x = 0;
    //make sure each spot is a valid digit
    for (int x = 0; input[x] != '\0'; x++) {
        if (!is_digit(input[x])) 
            return 0;
    }
    return 1;
}

int is_udecimal(char* input) {
    int decs = 0;
     for (int i = 0; input[i] != '\0'; i++) {
        if (!is_digit(input[i])) {
            //count decimals
            if (input[i] == '.') {
                decs++;
                if (decs > 1) 
                    return 0;
                else 
                    return 1;
            }
            else 
                return 0;
        }
    }  
    return 0;
}

int is_ureal(char *input) {
    if (is_uinteger(input) || is_udecimal(input)) 
        return 1;  
    else 
        return 0;
}

int is_bracket(char input) {
    if (input == '(' || input == ')') 
        return 1;
    return 0;
}

int is_break(char input) {
    if (input == ' ' || input == '\n') 
        return 1;
    return 0;
}

int sign(char input) {
    if(input == '+' || input == '-') 
        return 1;
    else 
        return 0;
}

int is_number(char* input) {
    int len = 0;
    //counts len of num
    for(int i = 0; input[i] != '\0'; i++) {
        len++;
    }
    //basic check for real numbers
    if (is_ureal(input)) 
        return 1;
    //check for valid nums + signs
    else if (sign(input[0]) && len > 1 && is_ureal(&input[1])) 
        return 1;
    else 
        return 0;
}

//HELPER FUNCTIONS FOR SYMBOLS
//Possible Symbols: <=identifier>, initial, subsequent, letter, digit

int is_initial(char input) {
    //Split up for readability
    if (input == '$' || input == '/' || input == ':' || input == '<' || input == '_' || input == '>' || input == '?' || input == '~') 
        return 1;
    else if (input == '!' || input == '*' || input == '%' || input == '&' || input == '=' || input == '^') 
        return 1;
    
    else if (is_letter(input)) 
        return 1;
    else 
        return 0;
}

int is_subsequent(char input) {
    if (is_initial(input) || is_digit(input)) 
        return 1;
    else if (input == '.' || input == '+' || input == '-') 
        return 1;
    else 
        return 0;
}

int is_identifier(char* input) {
    if (is_initial(input[0])) {
        for (int i = 0; input[i] != '\0'; i++) {
            if (!is_subsequent(input[i])) 
                return 0;
        }
        return 1;
    }

    else if (input[0] == '+' && input[1] == '\0') 
        return 1;
    else if (input[0] == '-' && input[1] == '\0') 
        return 1;
    else 
        return 0;
}


//Big Tokenize Function
Value *tokenize() {
    char charRead;
    Value *list = makeNull();
    charRead = (char)fgetc(stdin);

    //While loop seaching through the full file
    while (charRead != EOF) {
        //First, we check open and closed parens
        if (charRead == '(') {
            char *str_val = "(";
            Value *open = talloc(sizeof(Value));
            open->type = OPEN_TYPE;
            open->s = str_val;
            list = cons(open, list);

        } 
        else if (charRead == ')') {
            char *str_val = ")";
            Value *close = talloc(sizeof(Value));
            close->type = CLOSE_TYPE;
            close->s = str_val;
            list = cons(close, list);
        }

        //Tokenizing Strings:
        else if (charRead == '"') {
            int len = 0;
            char *first_string = talloc(sizeof(char) * 300);
            first_string[0] = '\0';
            charRead = fgetc(stdin);

            while(charRead != '"') {
                //Check if file doesn't end mid string
                if (charRead == EOF) {
                    printf("String missing quote");
                    texit(1);
                }
                //Add to string and shift
                first_string[len] = charRead;
                len++;
                first_string[len] = '\0';
                charRead = fgetc(stdin);
            }
            //Add final String to list
            Value *final_string = talloc(sizeof(Value));
            final_string->type = STR_TYPE;
            final_string->s = first_string;
            list = cons(final_string, list);
        }
        //Ignore various spaces
        else if (charRead == ' ' || charRead == '\n' || charRead == '\t') {
            ;
        }
        //Semicolons - scan to the end of the line or file (whichever is first)
        else if (charRead == ';') {
            while (charRead != '\n' && charRead != EOF) 
                charRead = fgetc(stdin);
        }
        //Next, we tackle symbols and numbers
        else {
            //Create String to store token of max size
            char *token = talloc(sizeof(char) * 300);
            token[0] = '\0';
            int len2 = 0;
            int memSize2 = 299;

            //While no interuptions - we get the token
            while ((charRead != '(' && charRead != ')') && (charRead !=  ' ' && charRead != '\n')) {
                //Ensure file doesn't end in the middle of a Token
                if (charRead == EOF) {
                    printf("Syntax Error: Hanging Token\n");
                    texit(1);
                }
                //Ignore new lines
                if (charRead == '\n' || charRead == '\t') 
                    ;
                //Adding to the Token
                else {
                    token[len2] = charRead;
                    len2++;
                    token[len2] = '\0';
                }
                charRead = fgetc(stdin);
            }
            //get rid of bracket or space from above
            ungetc(charRead, stdin);

            //Now that we have the token, we can proceed with catagorizing it

            //Number case          
            if (is_number(token)) {
                if (sign(token[0])) {
                    //sets new token as the size of the Token above
                    char *utoken = talloc(sizeof(char) * strlen(token) - 1);
                    //adds everything in token to the new token, except the first
                    for (int i = 1; token[i] != '\0'; i++) {
                        utoken[i-1] = token[i];
                    }
                    int index = strlen(token) - 2;
                    utoken[index] = '\0';
                    //Tokenize Integers
                    if (is_uinteger(utoken)) {
                        Value *new_val = talloc(sizeof(Value));
                        new_val->type = INT_TYPE;
                        new_val->i = strtol(token, NULL, 0);
                        list = cons(new_val, list);
                    }
                    //Tokenize Doubles
                    else if (is_udecimal(utoken)) {
                        Value *new_val = talloc(sizeof(Value));
                        new_val->type = DOUBLE_TYPE;
                        new_val->d = strtod(token, NULL);
                        list = cons(new_val, list);
                    }
                    // If the number isn't an Integer or Double
                    else {
                        printf("Syntax Error: '%s' Cannot be tokenized - Not a valid number \n", token);
                    }
                }
                
                //Number that doesn't start with a sign
                else  {
                    if (is_udecimal(token)) {
                        Value *new_val = talloc(sizeof(Value));
                        new_val->type = DOUBLE_TYPE;
                        new_val->d = strtod(token, NULL);
                        list = cons(new_val, list);
                    }
                    else if (is_uinteger(token)) {
                        Value *new_val = talloc(sizeof(Value));
                        new_val->type = INT_TYPE;
                        new_val->i = strtol(token, NULL, 0);
                        list = cons(new_val, list);
                    }
                    else {
                        printf("Syntax Error: '%s' Cannot be tokenized \n", token);
                    }
                }
            }

            //Tokenizing Symbols
            else if (is_identifier(token)) {
                Value *sym = talloc(sizeof(Value));
                sym->type = SYMBOL_TYPE;
                sym->s = token;
                list = cons(sym, list);
            }
            //Tokenizing Booleans
            else if (is_bool(token)) {
                if (token[1] == 'f') {
                    Value *new_val = talloc(sizeof(Value));
                    new_val->type = BOOL_TYPE;
                    new_val->i = 0;
                    list = cons(new_val, list);
                }
                else if (token[1] == 't') {
                    Value *new_val= talloc(sizeof(Value));
                    new_val->type = BOOL_TYPE;
                    new_val->i = 1;
                    list = cons(new_val, list);
                }
                else {
                    printf("Syntax error (readBoolean: boolean was not #t or #f\n");
                    texit(1);
                }
            }
            //If not any of the above: 
            else {
                printf("Syntax Error: '%s' Cannot be tokenized \n", token);
                texit(1);
            }
        }
        //Get next char
        charRead = (char)fgetc(stdin);
    }
    //Reverse the list for correct order
    Value *revList = reverse(list);
    return revList;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list) {  
    Value *cur = list;
    while (cur->type != NULL_TYPE) {
        struct ConsCell cur_cons = cur->c;
        Value *car_val = cur_cons.car;
        Value *cdr_val = cur_cons.cdr;
        int car_type = car_val->type;
        if (car_type == INT_TYPE) 
                printf("%i:integer\n", cur->c.car->i);
        if (car_type == DOUBLE_TYPE)
                printf("%f:double\n", cur->c.car->d);
        if (car_type == STR_TYPE)
                printf("\"%s\":string\n", cur->c.car->s);
        if (car_type == SYMBOL_TYPE)
                printf("%s:symbol\n", cur->c.car->s);
        if (car_type == OPEN_TYPE)
                printf("%s:open\n", cur->c.car->s);
        if (car_type == CLOSE_TYPE)
                printf("%s:close\n", cur->c.car->s);
        if (car_type == BOOL_TYPE)  {
            if (cur->c.car->i == 0) 
                printf("#f:boolean\n");
            else 
                printf("#t:boolean\n");   
        }
        cur = cdr_val;
    }
}