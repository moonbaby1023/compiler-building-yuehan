#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#define int long long
// some note:
// usually, arbitrary grammar can be converted to a backtrack-free Top-down parsing -- section 3.3.1
// catagories of Top-down parsers: 
//(1) top-down recursive-descent parser .    It is hand-coded
//(2) top-down table-driven LL(1) parser.    It is generated by computer

// backtrack-free grammar == predictive grammar
//TO DO: io-file

char *src;  // pointer to source code string;
int line, col;             // count how many lines&columns have been parsed
int poolsize;         // default size of text/data/stack

int tokenchar;            // current tokenchar
int token;
int calculatorOutput[128];
char isNum[128];
int nextEmpty = 0;

int * idmain; // the 'main' function  -------(3)

// these three variables are used in next()
int token_val;  // used in next()-recognizing number
int *current_id,// used in next()-recognizing identifier. 
    *symbols;   // symble table.used in next()-recognizing identifier. 

// ----------fields of the identifier library
enum {IdOrKey, Hash, Name, SizeOfId, Type, Class, Value, BType, BClass, BValue, IdSize};//attributes of the symbol table(except for the last one -- IdSize)
// each identifier takes up in total 9 cells of the table (Token ~ BValue)

// ----------types of tokens (operators last and in precedence order)
enum {
  Num = 128, Ident,
  Int, Char, Struct, Void, 
  If, Else, While, For, Break, New, Delete, Return, Extern, 
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak}; 
// Some charactors, such as "[" and "~", are not included because they themselves can be a token.
// the reason why we do not assign special tokens for "[" and "~" are:
// 1. the are single-character, not like "=="
// 2. they do nothing with precedence

int maxOne(int a, int b)
{
    if (a >= b)
        {return a;}
    else
    {
        {return b;}
    }
    
}
 
void next() 
{    // when will it "return"? answer: when "while" ends. then, when will the "while" end? answer: (1)it recognizes sth (2)*src == 0, i.e. the end of the article
     // the business about "forced exit because of unknown charactor" is done by GrammarPaser   
	char *last_pos; //  src is the start pointer of the malloc(). i don't think it is good to let src change such as "src++;
    int hash;

    while (tokenchar = *src)  // this while-loop is used to 1. skep whitespaces 2. report lexing error -- unknown character
    {
        ++col;
		++src;
        // parse token here
        if (tokenchar == '\n')  //------------------------- a new line will start
        {
            ++line;
            col = 0;
        }
        else if ( (tokenchar == 0x09) || (tokenchar == 0x20) || (tokenchar == 0xD) )  // ---------- whitespace
        {
            ;
        }
        else if (tokenchar == '#')
        {
            // skip macro, because we will not support it
            while (*src != 0 && *src != '\n') 
            {
                src++;
            }
        }    
        else if (tokenchar == '/') 
        {
            if (*src == '/') // --------------------------------------  single line comment
            {           
                while (*src != 0 && *src != '\n') 
                {
                    ++src;
                }
            } 
            else if (*src == '*')// --------------------------------------  multiline comment
            {
            	char endMultiComment = 0;
            	src++;
				while ((*src != 0)   &&  (endMultiComment == 0)) 
                {
                    if (*src == '*')
                    {
                    	src++;
                    	if (*src == '/')
                    	{
                    		endMultiComment = 1;
						}
					}
					src++;
                }
			}
			else // --------------------------------------------- divide operator
            {           
                //tokenchar = Div;
                token = Div;
                return;
            }
        }
		else if (  (tokenchar >='a'&& tokenchar<='z')  ||  (tokenchar >= 'A' && tokenchar <= 'Z')  ||  (tokenchar == '_')  )  
        { // --------------identifiers or identifiers->keywords
            last_pos = src - 1; // the address of the first char of the identifier
            hash = tokenchar;  

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) 
            {
                //hash = hash * 147 + *src; // each different identifier represents a different value, "hash" ; 
                // '0'=48  '9'=57 'A'=65 'Z'=90 '_'=95 'a'=97 'z'=122
                // No!!!! hash only support the identifier whose size is <=9. Otherwise, the hash value will saturate :-(
                src++;
            }

            // look for existing identifier, i.e. linear search 
            current_id = symbols;  // record the information of each identifier into the library:int * symbols
            while (current_id[IdOrKey])// if current_id[IdOrKey] == 0 then the identifier is a new comer
            {
                if (   !memcmp((char *)current_id[Name], last_pos, maxOne(current_id[SizeOfId], src - last_pos))   )
                // int memcmp (const void *s1, const void *s2, size_t n) is used to compare the top n characters of the memories pointed by s1 and s2
                // maxOne() is used to solve this circumstance: the old existing Ident is "abcde", and the new one is "abc"
                {
                    token = current_id[IdOrKey];
                    return;
                }
                current_id = current_id + IdSize;
            }

            // store new ID
            token = current_id[IdOrKey] = Ident;          
            current_id[Name] = (int)last_pos;  // stores the position of the first char of the identifier
            current_id[SizeOfId] = src - last_pos;// store how many character this identifier has
            return;

        }   
        else if (tokenchar >= '0' && tokenchar <= '9')   // ------------------------------number dec(123) hex(0x123) oct(017) // WHAT IF "1234ZABC": it is the same as "1234 ZABC"
        {

            if (tokenchar > '0')   // DEC, starts with [1-9]
            {
                token_val = tokenchar - '0';
                while (*src >= '0' && *src <= '9') 
                {
                    token_val = token_val*10 + *src++ - '0';
                }
            } 
            else 
            {
                if (*src == 'x' || *src == 'X')   //HEX
                { 
                    ++src;
                    int current_SlgDigit= 0;
                    token_val = 0;
                    while ((*src >= '0' && *src <= '9') || (*src >= 'A' && *src <= 'F') || (*src >= 'a' && *src <= 'f')) 
                    {
                        if (*src >= 'A' && *src <= 'F') 
                        {
                            current_SlgDigit = *src - 'A' + 10;
                        }
                        else if (*src >= 'a' && *src <= 'f')
                        {
                            current_SlgDigit = *src - 'a' + 10;
                        }
                        else
                        {
                            current_SlgDigit = *src - '0';
                        }

                        token_val = token_val*16 + current_SlgDigit;
                        ++src;
                    }
                    
                } 
                else    // OCT
                {
                    token_val = 0;
                    while (*src >= '0' && *src <= '7') 
                    {
                        token_val = token_val*8 + *src++ - '0';
                    }
                }
            }

            //tokenchar = Num;
            token = Num;// output of next()
            return;
        }
        else if (tokenchar == '\'') // ---------- single char
        {
            token = Char;
            token_val = *src++;
            if (*src == '\'')
            {       
                src++;
            }
            else
            {
                printf("lexing error: \' should appear at (line %d, col %d) but not", line, col);
                exit(-1);
            }
            
        }
        else if (tokenchar == '+') 
        {
            // parse '+' and '++'// ------------------------------------
            if (*src == '+') 
            {
                src ++;
                //tokenchar = Inc;
                token = Inc;
            } 
            else 
            {
                //tokenchar = Add;
                token = Add;
            }
            return;
        }
        else if (tokenchar == '-') 
        {
            // parse '-' and '--'// ------------------------------------
            if (*src == '-') 
            {
                src ++;
                //tokenchar = Dec;
                token = Dec;
            } 
            else 
            {
                //tokenchar = Sub;
                token = Sub;
            }
            return;
        }
		 else if (tokenchar == '*') 
        {
            //tokenchar = Mul;
            token = Mul;
            return;
        }
		
        else if (tokenchar == '=') // --------------------------------------------- = + - * & && | || != > >= < <=
        {
            // parse '==' and '='-------------------------------------------
            if (*src == '=') 
            {
                src ++;
                //tokenchar = Eq;
                token = Eq;
            } 
            else 
            {
                //tokenchar = Assign;
                token = Assign;
            }
            return;
        }
        
        else if (tokenchar == '!') 
        {
            // parse '!='// ------------------------------------
            if (*src == '=') 
            {
                src++;
                //tokenchar = Ne;
                token = Ne;
            }
            return;
        }
        else if (tokenchar == '<') 
        {
            // parse '<=', '<<' or '<'// ------------------------------------
            if (*src == '=') 
            {
                src ++;
                //tokenchar = Le;
                token = Le;
            } 
            else if (*src == '<')
            {
                src ++;
                token = Shl;
                //tokenchar = Shl; // 
            } 
            else 
            {
                //tokenchar = Lt;
                token = Lt;
            }
            return;
        }
        else if (tokenchar == '>') // ------------------------------------
        {
            // parse '>=', '>>' or '>'
            if (*src == '=') 
            {
                src ++;
                //tokenchar = Ge;
                token = Ge;
            } 
            else if (*src == '>') 
            {
                src ++;
                //tokenchar = Shr;
                token = Shr;
            } 
            else 
            {
                //tokenchar = Gt;
                token = Gt;
            }
            return;
        }
        else if (tokenchar == '|') // ------------------------------------
        {
            // parse '|' or '||'
            if (*src == '|') 
            {
                src ++;
                //tokenchar = Lor;
                token = Lor;
            } 
            else 
            {
                //tokenchar = Or;
                token = Or;
            }
            return;
        }
        else if (tokenchar == '&') // ------------------------------------
        {
            // parse '&' and '&&'
            if (*src == '&') 
            {
                src ++;
                //tokenchar = Lan;
                token = Lan;
            } 
            else 
            {
                //tokenchar = And;
                token = And;
            }
            return;
        }
        else if (tokenchar == '^') // ------------------------------------
        {
            //tokenchar = Xor;
            token = Xor;
            return;
        }
        else if (tokenchar == '%') // ------------------------------------
        {
            //tokenchar = Mod;
            token = Mod;
            return;
        }
       
        else if (tokenchar == '[')   // for array?// ------------------------------------
        {
            //tokenchar = Brak;
            token = Brak;
            return;
        }
        else if (tokenchar == '?') // ??????????// ------------------------------------
        {
            //tokenchar = Cond;
            token = Cond;
            return;
        }
        else if (tokenchar == '~' || tokenchar == ';' || tokenchar == '{' || tokenchar == '}' || tokenchar == '(' || tokenchar == ')' || tokenchar == ']' || tokenchar == ',' || tokenchar == ':') 
        {
            // directly return the character as token;
            token = tokenchar;// this is newly added in compiler_GrammarParser.c
            return;
        }
        else
        {
            printf("lexing error: unknown character in (line %d, col %d)", line, col);
            exit(-1);
        }
        

    }
    
    return;
}

void recordToken()
{
    if (token == Num)
    {
        calculatorOutput[nextEmpty] = token_val;
        isNum[nextEmpty] = 1;
    }
    else
    {
        calculatorOutput[nextEmpty] = token;
        isNum[nextEmpty] = 0;  
    }
    nextEmpty++;
}


void addMem2record(int insert, int tokenInsert, char isNumInsert)
{
    int i=0;
	for (i=nextEmpty; i>insert; i--)
    {
        calculatorOutput[i] = calculatorOutput[i-1];
        isNum[i] = isNum[i-1];
    }
    calculatorOutput[insert] = tokenInsert;
    isNum[insert] = isNumInsert;
    nextEmpty++;
}

void match(int expectedChar)
{
	if (token != expectedChar)
	{
		printf("------error in (line %d, col %d): --------\n", line, col);
		printf("expected tokenchar: %c(%d), but got: %c(%d)\n", expectedChar, expectedChar, tokenchar,tokenchar);
		exit(-1); // main()is forced to stop
	}
	
	next();

}

struct multiOutput
{
    int value;
    int location;
};


int expr();

struct multiOutput factor()
{
    struct multiOutput factorOut;
    factorOut.location = nextEmpty;
  
	if (token == '(')
	{
		//recordToken();
        match('(');        
		factorOut.value = expr();
        
        //recordToken();
		match(')');        
	}
	else
	{
		factorOut.value = token_val;      
		recordToken();
        match(Num); // see if the tokenchar is a number, if not, the main() will be forced to exit        
	}
	return factorOut;
}


int term_doMulDiv(int longValue, int leftBracket)
{
	struct multiOutput factorOut;

    if (token == Mul)
	{
		recordToken();
        match(Mul);               
		factorOut= factor();
        longValue = longValue * factorOut.value;
        // add ( at leftBracket
        addMem2record(leftBracket, '(', 0);
        // add ) at end
        addMem2record(nextEmpty, ')', 0);// first add ) then take the tail )

		return term_doMulDiv(longValue, leftBracket);
	}
	else if (token == Div)
	{
        recordToken();
		match(Div); 
        factorOut= factor();
        longValue = longValue / factorOut.value;      

        // add ( at leftBracket
        addMem2record(leftBracket, '(', 0);
        // add ) at end
        addMem2record(nextEmpty, ')', 0);

		return term_doMulDiv(longValue, leftBracket);
	}
	else
	{
		return longValue;	
	}
}


struct multiOutput term()
{
	struct multiOutput factorOut = factor();
    struct multiOutput termOut;
    termOut.value = term_doMulDiv(factorOut.value, factorOut.location);
	termOut.location = factorOut.location;
    return termOut;
}


int expr_doAddSub(int longValue, int leftBracket)
{
	struct multiOutput termOut;
    if (token == Add)
	{
		recordToken();
        match(Add);
        termOut = term();        
		longValue = longValue + termOut.value;
        // add ( before leftBracket
        addMem2record(leftBracket, '(', 0);
        // add ) before nextEmpty
        addMem2record(nextEmpty, ')', 0);

		return expr_doAddSub(longValue, leftBracket);
	}
	else if (token == Sub)
	{
        recordToken();
		match(Sub);
        termOut = term();         
		longValue = longValue - termOut.value;
        // add ( before leftBracket
        addMem2record(leftBracket, '(', 0);
        // add ) before nextEmpty
        addMem2record(nextEmpty, ')', 0);

		return expr_doAddSub(longValue, leftBracket);
	}
	else
	{
		return longValue;
	}
}

int expr()
{
    struct multiOutput termOut = term();
	return expr_doAddSub(termOut.value, termOut.location);
}

void program() 
{
    next();                  // get next tokenchar
    while (tokenchar > 0)
    {    
        int caculatorR = expr();
        
		int i=0;
        for (i=0; i<nextEmpty; i++)
        {
            if (isNum[i] == 1)
            {
                printf("%d", calculatorOutput[i]);
            }
            else
            {
                if (calculatorOutput[i] == Add)
                {printf(" + ");}
                else if (calculatorOutput[i] == Sub)
                {printf(" - ");}
                else if (calculatorOutput[i] == Mul)
                {printf(" * ");}
                else if (calculatorOutput[i] == Div)
                {printf(" / ");}
                else 
                {printf("%c", calculatorOutput[i]);} // '(',  ')'
            }          
        }
        printf("\n= %d", caculatorR);
        // the calculator do not have to consider EOF, the "while-loop" will 
    }

}



int main()
{
    int i;

    poolsize = 256 * 1024; // arbitrary size, set the size of 
    line = 1;
    col = 0;



// ---------add keywords to symbol table
    if (!(symbols = malloc(poolsize))) // save identifiers and keywords. the pointer, "current_id", slides itself to represent a certain identifier/keyword
    {
        printf("could not malloc(%d) for symbol table\n", poolsize);
        return -1;
    }
      memset(symbols, 0, poolsize);
    //current_id = symbols;no need to initialize the current_id here. next() will do this job

    src = " Int Char Struct Void If Else While For Break New Delete Return Extern main";
    i = Int;
    while (i <= Extern) // we are now in the main-function, the symbol table is still empty.
    {
        next(); // src will point to a "white space" when next() returns
        // thanks to the "while loop", when *src== white space, next() will continue rather than return
        current_id[IdOrKey] = i++; // for common identifiers, current_id[Token] = Id, while for key words, current_id[Token] = KeyWord
    }
    next(); idmain = current_id; // ??????????
    // when next() is running, current_id will traverse itself until point to a new black space.
    // the following space will be used for identifiers running in user's functions. // keep track of main 



// ----------start to read source code
    FILE * fp = NULL;
    char * filepath = "./files/test.txt";

    if (  (fp = fopen(filepath, "r")  ) == NULL) 
    {
        printf("could not open(%s)\n", filepath);
        return -1;
    }

    
    if (!(src = malloc(poolsize))) 
    {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }
    
    if (  (i = fread(src, sizeof(char), poolsize-1, fp)) <= 0) 
    { 
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // add EOF character
    fclose(fp);

    line = 1;
    col = 0;
    program();   
    return 0;
}