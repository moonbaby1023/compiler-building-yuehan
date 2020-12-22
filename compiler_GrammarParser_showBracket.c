#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#define int long long

int tokenchar;            // current tokenchar
int token;
int calculatorOutput[128];
char isNum[128];
int nextEmpty = 0;

char *src, *old_src;  // pointer to source code string;
int line, col;             // count how many lines&columns have been parsed
int poolsize;         // default size of text/data/stack
                                                // (2)
int *text,            // code                  
    *old_text,        // for dump text segment
    *stack;           
char *data;           // ????
                                                                // (2)
int *pc, *bp, *sp, ax, cycle; // virtual machine registers 
//instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT }; //build-in functions
  

                                                                                //(3)
// tokens and classes (operators last and in precedence order)
enum {
  Num = 128, Fun, Sys, Glo, Loc, Id,
  Char, Else, Enum, If, Int, Return, Sizeof, While,
  Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak}; 
// Some charactors, such as "[" and "~", are not included because they themselves can be a token.
// the reason why we do not assign special tokens for "[" and "~" are:
// 1. the are single-character, not like "=="
// 2. they do nothing with precedence


// these three variables are used in next()
int token_val;  // used in next()-recognizing number
int *current_id,// used in next()-recognizing identifier. 
    *symbols;   // symble table.used in next()-recognizing identifier. 

// fields of identifier
enum {Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize};//attributes of the symbol table(except for the last one -- IdSize)
// each identifier takes up in total 9 cells of the table (Token ~ BValue)




// (3)  
void next() 
{    // when will it "return"? answer: when "while" ends. then, when will the "while" end? answer: (1)it recognizes sth (2)*src == 0, i.e. the end of the article
     // the business about "forced exit because of unknown charactor" is done by GrammarPaser   
	char *last_pos; //  src is the start pointer of the malloc(). i don't think it is good to let src change such as "src++;
    int hash;

    while (tokenchar = *src)  // while循环用来跳过：空白字符/不识别的字符
    {
        ++col;
		++src;
        // parse token here
        if (tokenchar == '\n')  //------------------------- line break
        {
            ++line;
            col = 0;
        }   
        else if (tokenchar == '#')
        {
            // skip macro, because we will not support it
            while (*src != 0 && *src != '\n') 
            {
                src++;
            }
        }    
        else if (  (tokenchar >='a'&& tokenchar<='z')  ||  (tokenchar >= 'A' && tokenchar <= 'Z')  ||  (tokenchar == '_')  )  
        { // --------------identifier
            last_pos = src - 1; // the address of the first char of the identifier
            hash = tokenchar;   // the current char

            while ((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_')) 
            {
                hash = hash * 147 + *src; // the so called "linear search". hash value of the current char
                src++;
            }

            // look for existing identifier, linear search 
            current_id = symbols;  // int* symbols is the address of the first char of the 
            while (current_id[0])// if current_id[Token] == true then the identifier might be an old one
            {
                // why not: if (current_id[Hash] == hash)?????????? in case that the identifier is so long that its hash becomes saturated?
                if ((current_id[Hash] == hash) && !memcmp((char *)current_id[Name], last_pos, src - last_pos))  
                // int memcmp (const void *s1, const void *s2, size_t n) is used to compare the top n characters of the memories pointed by s1 and s2
                {
                    //tokenchar = current_id[Token];
                    token = current_id[Token];
                    return;
                }
                current_id = current_id + IdSize;
            }

            // store new ID
            token = current_id[Token] = Id;
            //tokenchar = current_id[Token] = Id;             
            current_id[Hash] = hash;
            current_id[Name] = (int)last_pos;  // only store the first char of the identifier. ??????????need another box to store the length
                 
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
        else if (tokenchar == '/') 
        {
            if (*src == '/') // -------------------------------------- skip comments
            {           
                while (*src != 0 && *src != '\n') 
                {
                    ++src;
                }
            } 
            else // --------------------------------------------- divide operator
            {           
                //tokenchar = Div;
                token = Div;
                return;
            }
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
                //tokenchar = Shl; // << ??????????
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
        else if (tokenchar == '*') // ------------------------------------
        {
            //tokenchar = Mul;
            token = Mul;
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
		recordToken();
        match('(');        
		factorOut.value = expr();
        recordToken();
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
        addMem2record(nextEmpty, ')', 0);

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


int eval() 
{ // do nothing yet
                                                                    // (2)
    int op, *tmp;
    
    
    while (1)
    {
        op = *pc++;
		if (op == IMM) {ax = *pc++;}  // put *pc into ax
        else if (op == LC) {ax = *(char *)ax;} // put the char who lives in the address into ax
        else if (op == LI) {ax = *(int *)ax;} //put the int who lives in the address into ax
        else if (op == SC) {*(char *)*sp = ax;} // save the char value in the ax onto the stack                                        
        else if (op == SC) {*(char *)*sp = ax;}
        else if (op == PUSH) {*--sp = ax;}          // put the value of ax onto the stack
        else if (op == JMP)  {pc = (int *)*pc;}     // JMP <addr>. let pc jump to the address which is saved in <addr> (note: *pc == <addr>)
        else if (op == JZ)   {pc = ax ? pc + 1 : (int *)*pc;}   // ??????????op?op+1==pc,*op==JMP,???????????????pc+1?if ax==0? then pc:=??????????????else pc:=
    //  else if (op == JZ)   {pc = ax ? pc : (int *)*pc;}  ?????????????????????
    //  else if (op == JZ)   {op = *pc; pc = ax ? pc + 1 : (int *)*pc;} ?????????????????pc+1?????pc???
    // ????????????????????????????????????(?1??????JMP??2????call)?????????op:=*(pc-1)
        else if (op == JNZ)  {pc = ax ? (int *)*pc : pc + 1;}        
    //  else if (op == JNZ)  {pc = ax ? (int *)*pc : pc;} 
    //  else if (op == JNZ)  {op = *pc; pc = ax ? (int *)*pc : pc + 1;} 
        
        else if (op == CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;}           // call subroutine ?????call ***??????????????????????????????????op:=*(pc-1)
      //else if (op == RET)  {pc = (int *)*sp++;}                              // return from subroutine;????? RET ?????????????????? LEV ??????

        else if (op == ENT)  {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}      // make new stack frame  
// ????pc++?????ENT <size>???????????pc??????

// ????????????pc??????????????????????frame
        else if (op == ADJ) {sp = sp + *pc++;} // // ??????bp??? ?????
        else if (op == LEV) {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;}
    
        else if (op == LEA)  {ax = (int)(bp + *pc++);}     // load address for arguments.

        // 运算符指令
        else if (op == OR) {ax = *sp++ | ax; }  //但是当机器看到OR前面的数时，并不是知道要把它放入栈呀
        else if (op == XOR) {ax = *sp++ ^ ax;}
        else if (op == AND) {ax = *sp++ & ax;}  //按位与
        else if (op == EQ)  {ax = *sp++ == ax;}
        else if (op == NE)  {ax = *sp++ != ax;}
        else if (op == LT)  {ax = *sp++ < ax;}
        else if (op == LE)  {ax = *sp++ <= ax;}
        else if (op == GT)  {ax = *sp++ >  ax;}
        else if (op == GE)  {ax = *sp++ >= ax;}
        else if (op == SHL) {ax = *sp++ << ax;}
        else if (op == SHR) {ax = *sp++ >> ax;}
        else if (op == ADD) {ax = *sp++ + ax;}  // 这里的add只能操作ax寄存器
        else if (op == SUB) {ax = *sp++ - ax;}
        else if (op == MUL) {ax = *sp++ * ax;}
        else if (op == DIV) {ax = *sp++ / ax;}
        else if (op == MOD) {ax = *sp++ % ax;} //取余

        // 内置函数
        else if (op == EXIT) { printf("exit(%d)", *sp); return *sp;} // 汇编语言终止

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        else if (op == OPEN) { ax = open((char *)sp[1], sp[0]); } 
    // http://c.biancheng.net/cpp/html/238.html

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        else if (op == CLOS) { ax = close(*sp);}
    // http://c.biancheng.net/cpp/html/229.html
   
   
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
        else if (op == READ) { ax = read(sp[2], (char *)sp[1], *sp); }
        // http://c.biancheng.net/cpp/html/239.html
        
        
//build-in functions
        else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); } 
        else if (op == MALC) { ax = (int)malloc(*sp);} 
        else if (op == MSET) { ax = (int)memset((char *)sp[2], sp[1], *sp);}
        else if (op == MCMP) { ax = memcmp((char *)sp[2], (char *)sp[1], *sp);}

        else 
        {
            printf("unknown instruction:%d\n", op);
            return -1;
        }


    
    }

    return 0;
}
//                          (3)
// types of variable/function
enum {CHAR, INT, PTR};
int * idmain; // the 'main' function  -------(3)

//int main(int argc, char **argv)   //assembly file production platform
int main()
{
    int i, fd;



    // argc--;
    // argv++;

    poolsize = 256 * 1024; // arbitrary size, set the size of 
    line = 1;
    col = 0;

    // if ((fd = open(*argv, 0)) < 0) {
    //     printf("could not open(%s)\n", *argv);//////目前程序会输出这一行
    //     return -1;
    // }



    if (!(symbols = malloc(poolsize))) 
    {
        printf("could not malloc(%d) for symbol table\n", poolsize);
        return -1;
    }
      memset(symbols, 0, poolsize);


    // add keywords to symbol table------------------ (3)
    src = "char else enum if int return sizeof while "
        "open read close printf malloc memset memcmp exit void main";
    i = Char;
    while (i <= While) // we are now in the main-function, the symbol table is still empty.
    {
        next(); // src will point to a "white space" when next() returns
        // thanks to the "while loop", when *src== white space, next() will continue rather than return
        current_id[Token] = i++; // for common identifiers, current_id[Token] = Id, while for key words, current_id[Token] = KeyWord
    }

    // add library to symbol table ////////////////////// 
    i = OPEN;
    while (i <= EXIT) 
    {
        next(); 
        current_id[Class] = Sys;// ??????????
        current_id[Type] = INT;//  ??????????
        current_id[Value] = i++;// ??????????
    }

    next(); current_id[Token] = Char; // handle void type
    next(); idmain = current_id; 
    // when next() is running, current_id will traverse itself until point to a new black space.
    // the following space will be used for identifiers running in user's functions. // keep track of main 




	//src = "0x1Ab12 * (8+  2  )/012  -6";
    src = "(25*10)*3";
//    if (!(src = old_src = malloc(poolsize))) 
//	{
//        printf("could not malloc(%d) for source area\n", poolsize);
//        return -1;
//    }
    // memset(src, 0, poolsize);  //this sentence is added by myself


    // // read the source file
    // if ((i = read(fd, src, poolsize-1)) <= 0) {  // (全篇，读书人/内存地址，读入字数)
    //     printf("read() returned %d\n", i);
    //     return -1;
    // }
    // src[i] = 0; // add EOF character
    // close(fd);



    //                                                                 // (2) allocate memory for virtual machine

    // if (!(text = old_text = malloc(poolsize))) {
    //     printf("could not malloc(%d) for text area\n", poolsize);
    //     return -1;
    // }
    // if (!(data = malloc(poolsize))) {
    //     printf("could not malloc(%d) for data area\n", poolsize);
    //     return -1;
    // }
    // if (!(stack = malloc(poolsize))) {
    //     printf("could not malloc(%d) for stack area\n", poolsize);
    //     return -1;
    // }

    // memset(text, 0, poolsize);//initiate the memory =0
    // memset(data, 0, poolsize);
    // memset(stack, 0, poolsize);

    // bp = sp = (int *)((int)stack + poolsize);//SP
    // // int * stack points to the lowest address of its allocated space, e.g. stack==0x00010; stack++;stack==0x00014;
    // // while the first thing entering the stack should be put in the highest address.
    // ax = 0;




    // i = 0;
    // text[i++] = IMM;
    // text[i++] = 10;
    // text[i++] = PUSH;
    // text[i++] = IMM;
    // text[i++] = 20;
    // text[i++] = ADD;
    // text[i++] = PUSH;
    // text[i++] = EXIT;  // before exit, the programme will output the top content of the stack
    // //in this programme, the "EXIT" cannot clear the stack 
    // pc = text;
    // return eval();



 
    line = 1;
    col = 0;
    program();   
    return 0;
}
