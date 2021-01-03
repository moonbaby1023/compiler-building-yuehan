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

//TO DO: 
//check all loop for runtime exceeding problem
// check if all {} and [] are implemented
// a- - - - -b negative sequence valid?
// Num recognizing: negative to be implemented?
// remember to initialize the char[]=""


//question:
//why must '\'' be an escape character??

char* srcFixed;
char *src;  // pointer to source code string;
char *stringRepo; // store the names of all identifiers and STRINGconstants.// it is likely that the source file might be too long for the src to load it in one time
char *stringRepoPtr;
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
char token_Op[3];
char* token_str_start;
int token_str_len;
int *current_id,// used in next()-recognizing identifier. 
    *symbols;   // used in next()-recognizing identifier.
short precTable[3][19]={1, 2, 3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 8, 8, 9, 9, 9, 10, 10, 
2, 3, 4, 5, 6, 6, 7, 7, 7, 7, 8, 8, 9, 9, 10, 10, 10, 20, 20, 
1, 2, 3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 8, 8, 9, 9, 9, 10, 10}; 

// ----------fields of the identifier library
enum {IdOrKey, Hash, Name, SizeOfId, Type, Class, Value, BType, BClass, BValue, IdSize};//attributes of the symbol table(except for the last one -- IdSize)
// each identifier takes up in total 9 cells of the table (Token ~ BValue)

// ----------types of tokens (operators last and in precedence order)
enum {
  Num = 128, CHAR, STRING, Ident,
  Int, Char, Struct, Void, 
  If, Else, While, For, Break, New, Delete, Return, Extern, 
  Lor, Lan, Eq, Ne, Le, Ge, Shl, Shr, Inc, Dec}; 
char*tokenName[28] = {"Num","UINT", "CHAR", "STRING", "Ident",
  "TInt", "TChar", "Struct", "TVoid", 
  "If", "Else", "While", "For", "Break", "New", "Delete", "Return", "Extern", 
  "Lor", "Lan", "Eq", "Ne", "Le", "Ge", "Shl", "Shr", "Inc", "Dec"};

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
	
    while (tokenchar = *src)  // this while-loop is used to 1. skep whitespaces 2. report lexing error -- unknown character
    {        
		src++;
		col++;
        // parse token here
        if (tokenchar == '\n')  //------------------------- a new line will start
        {
            ++line;
            col = 0;
            token = '\n';
        }
        else if ( (tokenchar == 0x09) || (tokenchar == 0x20) || (tokenchar == 0xD) )  // ---------- whitespace
        {
            col++;
        }
        else if (tokenchar == '#')
        {
            // skip macro, because we will not support it
            while (*src != 0 && *src != '\n') 
            {
                col++;
				src++;
            }
        }    
        else if (tokenchar == '/') 
        {
            if (*src == '/') // --------------------------------------  single line comment
            {           
                while (*src != 0 && *src != '\n') 
                {
                    col++;
					src++;
                }
            } 
            else if (*src == '*')// --------------------------------------  multiline comment
            {
            	char endMultiComment = 0;
            	col++;
				src++;
				while ((*src != 0)   &&  (endMultiComment == 0)) 
                {
                    if (*src == '*')
                    {
                    	col++;
						src++;
                    	if (*src == '/')
                    	{
                    		endMultiComment = 1;
						}
					}
					col++;
					src++;
                }
			}
			else // --------------------------------------------- divide operator
            {           
                token = '/';
                token_Op[0] = '/'; token_Op[1] = 0;             
                return;
            }
        }
		else if (  (tokenchar >='a'&& tokenchar<='z')  ||  (tokenchar >= 'A' && tokenchar <= 'Z')  ||  (tokenchar == '_')  )  
        { // --------------identifiers or identifiers->keywords
            char *start_pos; 
            start_pos = src - 1; // the address of the first char of the identifier
           
            //hash = tokenchar;  

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) 
            {
                //hash = hash * 147 + *src; // each different identifier represents a different value, "hash" ; 
                // '0'=48  '9'=57 'A'=65 'Z'=90 '_'=95 'a'=97 'z'=122
                // No!!!! hash only support the identifier whose size is <=9. Otherwise, the hash value will saturate :-(                
                col++;
				src++;
            }
            int idLength = src - start_pos;
            // look for existing identifier, i.e. linear search 
            
            current_id = symbols;  // record the information of each identifier into the library:int * symbols
            while (current_id[IdOrKey])// if current_id[IdOrKey] == 0 then the identifier is a new comer
            {
				if (   !memcmp((char *)current_id[Name], start_pos, maxOne(current_id[SizeOfId], idLength))   )// if the identifier is old
                // int memcmp (const void *s1, const void *s2, size_t n) is used to compare the top n characters of the memories pointed by s1 and s2
                // maxOne() is used to solve this circumstance: the old existing Ident is "abcde", and the new one is "abc"
                {
                    token = current_id[IdOrKey];
                    token_val = (int)current_id;
                    token_str_start = (char*)(current_id[Name]);
                    token_str_len = current_id[SizeOfId];
                    return;
                }
                current_id = current_id + IdSize;
            }

            // the ID is a new ID, store it.
            int j = 0;
            char* start_posRepo = stringRepoPtr;
            for (j=0; j<idLength; j++)
            {
                *stringRepoPtr = start_pos[j];
                stringRepoPtr++;
            }
            token = current_id[IdOrKey] = Ident;
            token_val = (int)current_id;         
            current_id[Name] = (int)start_posRepo;  // stores the position of the first char of the identifier
            token_str_start = start_posRepo;
			current_id[SizeOfId] = token_str_len = idLength;// store how many character this identifier has
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
                    col++;
                }
            } 
            else 
            {
                if (*src == 'x' || *src == 'X')   //HEX
                { 
                    col++;
					src++;
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
                        col++;
						src++;
                    }
                    
                } 
                else    // OCT
                {
                    token_val = 0;
                    while (*src >= '0' && *src <= '7') 
                    {
                        token_val = token_val*8 + *src++ - '0';
                        col++;
                    }
                }
            }

            //tokenchar = Num;
            token = Num;// output of next()
            return;
        }
        else if (tokenchar == '\'') // ---------- character constant.  empty character, i.e.'', is not allowed
        {
            
			token = CHAR;
			if (*src == '\\')
			{
				token_Op[0] = '\\'; 
                col++;
				src++;
				if (*src == 'n')
					{token_val = 10; token_Op[1] = 'n';}
				else if (*src == 't')
					{token_val = 9; token_Op[1] = 't';}
				else if (*src == '\\')
					{token_val = 92; token_Op[1] = '\\';}
				else if (*src == '\'')
					{token_val = 39; token_Op[1] = '\'';}
				else if (*src == '\"')
					{token_val = 34; token_Op[1] = '\"';}
				else
				{
					printf("lexing error: illegal escape character at (line %d, col %d) but not", line, col);
                	exit(-1);
				}				
			}
			else
				{token_val = *src; token_Op[0] = token_val; token_Op[1] = 0;}
            	
				
	        src++; // jump to '    
            if (*src == '\'') // only the characters between 32~126 are allowed to appear in the ''
            {       
                col++;
				src++;// jump to the next to-be-dealt-with character
				return;
            }
            else
            {
                printf("lexing error at (line %d, col %d: \' problem or illegal char", line, col);
                exit(-1);
            }
            
            
        }
        else if (tokenchar == '\"')// ---------- string constant.  empty string, i.e."", is  allowed
        {
            token_str_start = stringRepoPtr;
            token_str_len = 0;

            char GoOn = 1;
			while (GoOn)
            {
                if (*src == '\\')
			    {
                    *stringRepoPtr = *src;
                    stringRepoPtr++;
                    token_str_len++;
                    col++;
					src++;
                    if (*src == 'n')
                        {;}//10
                    else if (*src == 't')
                        {;}//9
                    else if (*src == '\\')
                        {;}//92
                    else if (*src == '\'')
                        {;}//39
                    else if (*src == '\"')
                        {;}//34
                    else
                    {
                        printf("lexing error: illegal escape character at (line %d, col %d) but not", line, col);
                        exit(-1);
                    }                                      				
			    }
                else if (*src == '\"')
                {   GoOn = 0;}
				*stringRepoPtr = *src;
                stringRepoPtr++;
                token_str_len++;
                col++;
				src++;				                
            }
			           
            token = STRING;

            return;
        }
        else if (tokenchar == '+') 
        {
            // parse '+' and '++'// ------------------------------------
            if (*src == '+') 
            {
                col++;
				src++;
                token = Inc;
                 
            } 
            else 
            {
                token = '+';
                token_Op[0] = '+'; token_Op[1] = 0;
            }
            return;
        }
        else if (tokenchar == '-') 
        {
            // parse '-' and '--'// ------------------------------------
            if (*src == '-') 
            {
                col++;
				src++;
                token = Dec;
                 
            } 
            else 
            {
                token = '-';
                token_Op[0] = '-'; token_Op[1] = 0;
            }
            return;
        }
		else if (tokenchar == '*') 
        {
            token = '*';
            token_Op[0] = '*'; token_Op[1] = 0;
            return;
        }
		
        else if (tokenchar == '=') // --------------------------------------------- = + - * & && | || != > >= < <=
        {
            // parse '==' and '='-------------------------------------------
            if (*src == '=') 
            {
                src++;
                col++;
                token = Eq;
                token_Op[0] = '='; token_Op[1] = '='; token_Op[2] = 0; 
            } 
            else 
            {
                token = '=';
                token_Op[0] = '='; token_Op[1] = 0;
            }
            return;
        }
        
        else if (tokenchar == '!') 
        {
            // parse '!='// ------------------------------------
            if (*src == '=') 
            {
                src++;
                col++;
                token = Ne;
                token_Op[0] = '!'; token_Op[1] = '='; token_Op[2] = 0; 
            }
            else
            {
                token = '!';
                token_Op[0] = '!'; token_Op[1] = 0;
            }            
			return;
        }
        else if (tokenchar == '<') 
        {
            // parse '<=', '<<' or '<'// ------------------------------------
            if (*src == '=') 
            {
                src++;
                col++;
                token = Le;
                token_Op[0] = '<'; token_Op[1] = '='; token_Op[2] = 0; 
            } 
            else if (*src == '<')
            {
                src++;
                col++;
                token = Shl;
                token_Op[0] = '<'; token_Op[1] = '<'; token_Op[2] = 0; 
            } 
            else 
            {
                token = '<';
                token_Op[0] = '<'; token_Op[1] = 0;
            }
            return;
        }
        else if (tokenchar == '>') // ------------------------------------
        {
            // parse '>=', '>>' or '>'
            if (*src == '=') 
            {
                src++;
                col++;
                token = Ge;
                token_Op[0] = '>'; token_Op[1] = '='; token_Op[2] = 0; 
            } 
            else if (*src == '>') 
            {
                src++;
                col++;
                token = Shr;
                token_Op[0] = '>'; token_Op[1] = '>'; token_Op[2] = 0; 
            } 
            else 
            {
                token = '>';
                token_Op[0] = '>'; token_Op[1] = 0; 
            }
            return;
        }
        else if (tokenchar == '|') // ------------------------------------
        {
            // parse '||' or '|'
            if (*src == '|') 
            {
                src++;
                col++;
                token = Lor;
                token_Op[0] = '|'; token_Op[1] = '|'; token_Op[2] = 0; 
            } 
            else 
            {
                token = '|';
                token_Op[0] = '|'; token_Op[1] = 0; 
            }
            return;
        }
        else if (tokenchar == '&') // ------------------------------------
        {
            // parse '&&' and '&'
            if (*src == '&') 
            {
                src++;
                col++;
                token = Lan;
                token_Op[0] = '&'; token_Op[1] = '&'; token_Op[2] = 0; 
            } 
            else 
            {
                token = '&';
                token_Op[0] = '&'; token_Op[1] = 0;
            }
            return;
        }  
        else if (tokenchar == '~' || tokenchar == ';' || tokenchar == '{' || tokenchar == '}' || tokenchar == '[' || tokenchar == ']' || tokenchar == '(' || tokenchar == ')' || tokenchar == ']' || tokenchar == ',' || tokenchar == '.' || tokenchar == ':' || tokenchar == '?' || tokenchar == '%') 
        {
            // directly return the character as token;
            token = tokenchar;
            token_Op[0] = tokenchar; token_Op[1] = 0; 
            return;
        }
        else
        {
            printf("lexing error: unknown character in (line %d, col %d)", line, col);
            exit(-1);
        }
        

    }
    token = 0;// HERE
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
/*
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

    if (token == '*')
	{
		recordToken();
        match('*');               
		factorOut= factor();
        longValue = longValue * factorOut.value;
        // add ( at leftBracket
        addMem2record(leftBracket, '(', 0);
        // add ) at end
        addMem2record(nextEmpty, ')', 0);// first add ) then take the tail )

		return term_doMulDiv(longValue, leftBracket);
	}
	else if (token == '/')
	{
        recordToken();
		match('/'); 
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
    if (token == '+')
	{
		recordToken();
        match('+');
        termOut = term();        
		longValue = longValue + termOut.value;
        // add ( before leftBracket
        addMem2record(leftBracket, '(', 0);
        // add ) before nextEmpty
        addMem2record(nextEmpty, ')', 0);

		return expr_doAddSub(longValue, leftBracket);
	}
	else if (token == '-')
	{
        recordToken();
		match('-');
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
*/


short operatorType(int operatorr)
{
    if ((operatorr=='!') || (operatorr=='~'))
    {   return 1;}//prefix
    else if ((operatorr=='+') || (operatorr=='-') || (operatorr=='*') || (operatorr=='/') || (operatorr=='%') || (operatorr=='<') || (operatorr=='>') || (operatorr==Le) || (operatorr==Ge) || (operatorr==Eq) || (operatorr==Ne) || (operatorr=='&') || (operatorr=='|') || (operatorr==Lan) || (operatorr==Lor) || (operatorr==Shl) || (operatorr==Shr))
    {   return 2;}//binary
    else
    {
        return 0;
    }
    
}



short indexx(int operatorr)
{    
    if (operatorr==Lor)
        return 0;
    else if (operatorr==Lan)
        return 1;
    else if (operatorr=='|')
        return 2;
    else if (operatorr=='&')
        return 3;
    else if (operatorr==Eq)
        return 4;
    else if (operatorr==Ne)
        return 5;
    else if (operatorr=='<')
        return 6;
    else if (operatorr=='>')
        return 7;
    else if (operatorr==Le)
        return 8;
    else if (operatorr==Ge)
        return 9;
    else if (operatorr==Shl)
        return 10;
    else if (operatorr==Shr)
        return 11;
    else if (operatorr=='+')
        return 12;
    else if (operatorr=='-')
        return 13;
    else if (operatorr=='*')
        return 14;
    else if (operatorr=='/')
        return 15;
    else if (operatorr=='%')
        return 16;
    else if (operatorr=='!')
        return 17;
    else if (operatorr=='~')
        return 18;
    else
    {
        return -2;
    }
}

short prec(int operatorr)
{
    short tableCol = indexx(operatorr);
    if (tableCol == -2)
    {
    	return -2;
	}
	else
	{
		return precTable[0][tableCol];	
	}
	
}

short tightPrec(int operatorr)
{
    return precTable[1][indexx(operatorr)];
}

short loosePrec(int operatorr)
{
    return precTable[2][indexx(operatorr)];
}



void ty(char* toTellSuperior) // has error report; does next() 
{
    char T[64]="";
   
    if (token == Void)
    {   strcat(T, "TVoid");}
    else if (token == Int)
    {   strcat(T, "TInt");}
    else if (token == Char)
    {   strcat(T, "TChar");}
    else if (token == Ident)
    {   
        strcat(T, "TIdent(\"");

        char* k = token_str_start;
        int i = 0;
        char tmp[50]="";
        for (i=0; i<token_str_len; i++)
        {
            tmp[i] = *k;
            k++;
        }
        strcat(T, tmp);

        strcat(T, "\")");
    }
    else
    {
        printf("parser error");
        exit(-1);
    }
            
    next();
    if (token == '*') 
    {
        strcat(toTellSuperior, "TPoint(");
        strcat(toTellSuperior, T);
        strcat(toTellSuperior, ")");
        next();
    }
    else
    {
        strcat(toTellSuperior, T);
    }
    
    
}




void expr(char* toTellSuperior);
void EE(short p, char* toTellSuperior);

void P(char* toTellSuperior) // return string
{       
    if (token == '-')
    {
        strcat(toTellSuperior, "EUnOp(-, ");

        next();
        P(toTellSuperior);
        // short pp = tightPrec('-');
		// EE(pp, toTellSuperior);

        strcat(toTellSuperior, ")");
    }
    else if (token == '(')
    {
        next();
        EE(0, toTellSuperior);
        match(')');
    }
    else if (token == Num || token == CHAR || token == STRING)
    {
        if (token == Num)
        {
            
			strcat(toTellSuperior, "EInt(");
            
            char int2char[20]="";
            char tmp[20];
            int n = 0, j = 0;
            int eint = token_val;
	            tmp[n] = eint % 10 + '0';// in case of EInt(0)
	            n++;
	            eint = eint/10;
            while (eint > 0)
            {
                tmp[n] = eint % 10 + '0';
                n++;
                eint = eint/10;
            }
            for (j=0; j<n; j++)
            {
                int2char[j] = tmp[n-j-1];
            }            
            strcat(toTellSuperior, int2char);

            strcat(toTellSuperior, ")");
		            
        }
        else if (token == CHAR)
        {
            
			strcat(toTellSuperior, "EChar(\'");
            
            strcat(toTellSuperior, token_Op);
            
            strcat(toTellSuperior, "\')");
           
        }
        else 
        {//token == STRING
            
            strcat(toTellSuperior, "EString(\"");

            char* k = token_str_start;
            char tmp[128] = "";// set to 0
            int i = 0;
            for(i=0; i<token_str_len; i++)
            {
                tmp[0] = *k;
                strcat(toTellSuperior, tmp);
                k++;
            }

            strcat(toTellSuperior, "\")");
           
        }
        
        
        next();        
    }
    else if (token == Ident)
    {
        char tmp[128]="";//important set to 0!!!!
        char* k = token_str_start;
        int i = 0;
        for (i=0; i<token_str_len; i++)
        {
            tmp[i] = *k;
            k++;
        }

        next();
        if (token == '(')
        {
            strcat(toTellSuperior, "ECall(\"");
            strcat(toTellSuperior, tmp);
            strcat(toTellSuperior, "\",{");

            next();
            if (token !=')')
            {
                expr(toTellSuperior);
                while (token == ',')
                {
                    match(',');
                    strcat(toTellSuperior, " ");
                    expr(toTellSuperior);
                }
            }            
            match(')');

            strcat(toTellSuperior, "})");
        }
        else if (token == '[')
        {
            strcat(toTellSuperior, "EArrayAccess(\"");
            strcat(toTellSuperior, tmp);
            strcat(toTellSuperior, "\",");

            next();
            expr(toTellSuperior);
            strcat(toTellSuperior, ",");

            match(']');
            if (token == '.')
            {    
                next();
                match(Ident);
                
                char tmp2[64]="";
                k = token_str_start;
                for (i=0; i<token_str_len; i++)
                {
                    tmp2[i] = *k;
                    k++;
                }
                strcat(toTellSuperior, "\"");
                strcat(toTellSuperior, tmp2);
                strcat(toTellSuperior, "\"");         
            }

            strcat(toTellSuperior, ")");
        }
        else
        {
            strcat(toTellSuperior, "EVar(\"");           
            strcat(toTellSuperior, tmp);
            strcat(toTellSuperior, "\")");
        }                       
    }
    else if (token == New)
    {
        strcat(toTellSuperior, "ENew(");

        next();
        ty(toTellSuperior);// HERE
        strcat(toTellSuperior, ", ");

        match('[');
        char result_expr[512]="";
        expr(result_expr);
        strcat(toTellSuperior, result_expr);
        match(']');
        strcat(toTellSuperior, ")");


    }       
    else
    {
        printf("parsing error at (line %d, col %d)\n", line, col);
        exit(-1);
    }
    
}


void EE(short p, char* toTellSuperior) 
{
    
    P(toTellSuperior);   

    short r = 10;
    short precedenceRT = prec(token);
    if (precedenceRT == -2)
    {
		return;
	}
    short typeTokenRT = operatorType(token);
        
	if (!(  (typeTokenRT==2 || typeTokenRT==3)  &&  (p<=precedenceRT && precedenceRT<=r)   ))
	{
		return;
	}	
	
	
    while ((typeTokenRT==2 || typeTokenRT==3)  &&  (p<=precedenceRT && precedenceRT<=r))
    {
        short tightprecB = tightPrec(token);
        short looseprecB = loosePrec(token);
        short typeTokenB = operatorType(token);
        char operatorB[3];
        operatorB[0] = token_Op[0];  
        operatorB[1] = token_Op[1];  
        operatorB[2] = token_Op[2];  
        next();
        
        if (typeTokenB == 2) 
        {    
            char tmp[512] = "";
			strcat(tmp, "EBinOp(");
            strcat(tmp, operatorB);
            strcat(tmp, ", ");

            strcat(tmp, toTellSuperior);
            strcat(tmp, ", ");

            char result_EE[512]="";// it is soooooo important to initialize an array to ZERO!
			EE(tightprecB, result_EE);
			strcat(tmp, result_EE);
            strcat(tmp, ")");
            
            toTellSuperior[0] = 0;
        	strcat(toTellSuperior, tmp);
        }
        else if (typeTokenB == 3)
        {
            ;// there is no postfix operator in Cigrid grammar
        }

        r = looseprecB;
        precedenceRT = prec(token);       
    }

}


void expr(char* toTellSuperior)
{
    EE(0, toTellSuperior);
}

/*
void params() 
{
    if (  (token == Void) || (token == Int) || (token == Char) || (token == Ident)  ) 
    {
        ty();
        match(Ident);
        while (token == ',') 
        {
            match(',');
            ty();
            match(Ident);
        }
    }// else do nothing
}


void assignSuffix()
{
    if (token == '=')
    {
        next();
        expr();
    }
    else if (token == Inc)
    {
        next();
    }
    else
    {
        match(Dec);
    }
    
}

void assignSuffix2()
{
    if (token == '(') // assign sth by running a function
    {
        next();
        if (token != ')')
        {
            expr();
            while (token == ',')
            {
                next();
                expr();
            }
        }
        match(')');
    }
    else if (token == '[') // assign array
    {
        next();
        expr();
        match(']');
        if (token == '.')
        {
            next();
            match(Ident);
        }
        assignSuffix();
    }
    else // assign identifier
    {
        assignSuffix();
    }
}

void assign()
{
	match(Ident);
	assignSuffix2();
}

void varassign()
{
    if (token != Ident)
    {
        ty();// local variable initialization
        match(Ident);
        match('=');
        expr();          
    }
    else
    {
        match(Ident);
        if (token == Ident)
        {
            next();
            match('=');
            expr();
        }
        else if (token == '*')
        {
            next();
            match(Ident);
            match('=');
            expr();
        }
        else 
        {   
            assignSuffix2();
        }      
    }
}


void stmtElseSuffix();
void stmt() 
{
    if (token == '{')
    {
        next();
        while (token != '}') 
        {
            stmt();
        }
        match('}');    
    }
    else if (token == If)
    {
        next();
        match('(');
        expr();
        match(')');
        stmt();
        stmtElseSuffix();
    }
    else if (token == While)
    {
        next();
        match('(');
        expr();
        match(')');
        stmt();
    }
    else if (token == Break)
    {
        next();
        match(';');
    }
    else if (token == Return)
    {
        next();
        if (token != ';')
            {expr();}
        match(';');
    }
    else if (token == Delete)
    {
        next();
        match('[');
        match(']');
        match(Ident);
        match(';');
    }
    else if (token == For)
    {
        next();
        match('(');
        varassign();
        match(';');
        expr();
        match(';');
        assign();
        match(')');
        stmt();
    }
    else
    {
        varassign();
        match(';');
    }
    

}
void stmtElseSuffix()
{
    if (token == Else)
    {
        next();
        stmt();
    }// else do nothing
}


void globalSuffix2() 
{
    if (token == '(') 
    {
        next();
        params();
        match(')');
        match(';');
    }
    else
    {
        match(';');
    }
}

void globalSuffix1() 
{
    if (token == '(') 
    {
        next();
        params();
        match(')');
        match('{');
        while (token != '}') 
        {
            stmt();//if stmt() eats an unknown token, it will report error
        }
        match('}');

    }
    else if (token == '=') 
    {
        expr();
        match(';');
    }

}


void global() 
{
    if (token == Extern) 
    {
        match(Extern);
        ty();
        match(Ident);
        globalSuffix2();
    }//HERE HERE why does the original program use 2 char enum?
    else if (  (token == Void) || (token == Int) || (token == Char) || (token == Ident)  ) 
    {
        ty() ;
        match(Ident);
        globalSuffix1();
    }
    else
    {
        match(Struct);
        match(Ident);
        match('{') ;
        while (  (token == Void) || (token == Int) || (token == Char) || (token == Ident)  )  // 首选开头匹配，如果是nonterminal 才选结尾不匹配
        {
            ty() ;
            match(Ident) ;
            match(';') ;
        }
        match('}') ;
        match(';') ;
    }

}

void program() 
{
   next() ;//initialize tokenchar
   while (token > 0) //to avoid conflict against ascii, enum should start from 128
   {
       global () ;
   }
}

*/

/*void program() 
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
                printf(" %c ", calculatorOutput[i]);                              
            }          
        }
        printf("\n= %d", caculatorR);
        // the calculator do not have to consider EOF, the "while-loop" will 
    }

}*/

void scannerCheck()
{
    next();
    while (tokenchar > 0)
    {
        if (token >= 128)
            {printf("%s\n", tokenName[token-128]);}
        else if (token > 0)
            {printf("%c\n", token);}
        next();
    }

}

int main()
{
    int i;

    poolsize = 256 * 1024; // arbitrary size, set the size of 
    line = 1;
    col = 0;


// ----------- allocate space for stringRepo
if (!(stringRepo = malloc(poolsize))) 
    {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }
    memset(stringRepo, 0, poolsize);
stringRepoPtr = stringRepo; 

// ---------add keywords to symbol table
    if (!(symbols = malloc(poolsize))) // save identifiers and keywords. the pointer, "current_id", slides itself to represent a certain identifier/keyword
    {
        printf("could not malloc(%d) for symbol table\n", poolsize);
        return -1;
    } 
    memset(symbols, 0, poolsize);
	//current_id = symbols;//no need to initialize the current_id here. next() will do this job


   src = "int char struct void if else while for break new delete return extern main";
   current_id = symbols;
   i = Int;
   while (i <= Extern) // we are now in the main-function, the symbol table is still empty.
   {
       next();      
       current_id[IdOrKey] = i++; 
   }
   next(); idmain = current_id; // ?????????? 
   // the following space will be used for identifiers running in user's functions. // keep track of main 


// ----------start to read source code
    FILE * fp = NULL;
    char * filepath = "./files/test.txt";

    if (  (fp = fopen(filepath, "r")  ) == NULL) 
    {
        printf("could not open(%s)\n", filepath);
        return -1;
    }

    
    if (!(srcFixed = malloc(poolsize))) 
    {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }
    
    if (  (i = fread(srcFixed, sizeof(char), poolsize-1, fp)) <= 0) 
    { 
        printf("read() returned %d\n", i);
        return -1;
    }
    srcFixed[i] = 0; // add EOF character
    close(fp);
    src = srcFixed;
    


// ----------------- scanner and parser Go!
    line = 1;
    col = 1;
	
	next();
    char outExpr[512]=""; 
    expr(outExpr);
    printf("here: \n%s\n", outExpr);
    //program();  
    
    free(stringRepo);
    free(symbols);
    free(srcFixed);
 
    return 0;
}
