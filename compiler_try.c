#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#define int long long

int tokenchar;            // current token
char *src, *old_src;  // pointer to source code string;    用int比较好吧？万一是汉字呢?
int poolsize;         // default size of text/data/stack
int line;             // line number
                                                // (2)
int *text,            // code                  
    *old_text,        // for dump text segment
    *stack;           // ???????????
char *data;           // ????
                                                                // (2)
int *pc, *bp, *sp, ax, cycle; // virtual machine registers ???????????
//instructions
enum active { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };
      // ???????????????????
       // ???????????????????????   "??????"???


void next() {
    tokenchar = *src++;
    return;
}

void expression(int level) {
    // do nothing
}

void program() {
    next();                  // get next token
    while (tokenchar > 0) {
        printf("tokenchar is: %c\n", tokenchar);
        next();
    }
}


int eval() { // do nothing yet
                                                                    // (2)
    int op, *tmp;
    
    
    while (1)
    {
        op = *pc++;
		if (op == IMM) {ax = *pc++;}  // ?????????????????ax??????pc++??IMM??????
        else if (op == LC) {ax = *(char *)ax;} // ??ax??????????????ax?
        else if (op == LI) {ax = *(int *)ax;} //?????????int??ax?
        else if (op == SC) {*(char *)*sp = ax;} // ????????????????ax????????????????????????????ax
                                                //stack???????????sp=stack?????*sp????????????????
        else if (op == SC) {*(char *)*sp = ax;}
        else if (op == PUSH) {*--sp = ax;}          // push the value of ax onto the stack
        else if (op == JMP)  {pc = (int *)*pc;}     // JMP <addr> ?????????????? PC ????????? <addr>??????????(op)??JMP?,pc??????????
        else if (op == JZ)   {pc = ax ? pc + 1 : (int *)*pc;}         // ??????????op?op+1==pc,*op==JMP,???????????????pc+1?if ax==0? then pc:=??????????????else pc:=
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
        
        
///////////////////////////////////////////////////////////////////???
        else if (op == PRTF) { tmp = sp + pc[1]; ax = printf((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]); } // ?????????????????????
        else if (op == MALC) { ax = (int)malloc(*sp);} // ??????????????????????????????????????????
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

//int main(int argc, char **argv)   //assembly file production platform
int main()
{
    int i, fd;




    // argc--;
    // argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;

    // if ((fd = open(*argv, 0)) < 0) {
    //     printf("could not open(%s)\n", *argv);//////目前程序会输出这一行
    //     return -1;
    // }


    // if (!(src = old_src = malloc(poolsize))) {
    //     printf("could not malloc(%d) for source area\n", poolsize);
    //     return -1;
    // }

    // // read the source file
    // if ((i = read(fd, src, poolsize-1)) <= 0) {  // (全篇，读书人/内存地址，读入字数)
    //     printf("read() returned %d\n", i);
    //     return -1;
    // }
    // src[i] = 0; // add EOF character
    // close(fd);





                                                                    // (2) allocate memory for virtual machine
    if (!(text = old_text = malloc(poolsize))) {
        printf("could not malloc(%d) for text area\n", poolsize);
        return -1;
    }
    if (!(data = malloc(poolsize))) {
        printf("could not malloc(%d) for data area\n", poolsize);
        return -1;
    }
    if (!(stack = malloc(poolsize))) {
        printf("could not malloc(%d) for stack area\n", poolsize);
        return -1;
    }

    memset(text, 0, poolsize);//initiate the memory =0
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    bp = sp = (int *)((int)stack + poolsize);//SP
    // int * stack points to the lowest address of its allocated space, e.g. stack==0x00010; stack++;stack==0x00014;
    // while the first thing entering the stack should be put in the highest address.
    ax = 0;




    i = 0;
    text[i++] = IMM;
    text[i++] = 10;
    text[i++] = PUSH;
    text[i++] = IMM;
    text[i++] = 20;
    text[i++] = ADD;
    text[i++] = PUSH;
    text[i++] = EXIT;  // before exit, the programme will output the top content of the stack
    //in this programme, the "EXIT" cannot clear the stack 
    pc = text;






    //program();   
    return eval();
}
