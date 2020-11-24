#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

int token;            // current token
char *src, *old_src;  // pointer to source code string;
int poolsize;         // default size of text/data/stack
int line;             // line number
                                                // (2)
int *text,            // code                  
    *old_text,        // for dump text segment
    *stack;           // 局部变量，函数的参数值
char *data;           // 全局变量
                                                                // (2)
int *pc, *bp, *sp, ax, cycle; // virtual machine registers 本虚拟机只有一个寄存器
//instructions
enum { LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,
       OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,
       OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT };
       // 带有参数的指令在前，没有参数的指令在后
       // 这种顺序的唯一作用就是在打印调试信息时更加方便   "打印调试信息"???



void next() {
    token = *src++;
    return;
}

void expression(int level) {
    // do nothing
}

void program() {
    next();                  // get next token
    while (token > 0) {
        printf("token is: %c\n", token);
        next();
    }
}


int eval() { // do nothing yet
                                                                    // (2)
    int op, *tmp;
    while (1)
    {
        if (op == IMM) {ax = *pc++;}  // 把当前指令（不是指令的地址）搁置在ax里，然后准备运行下一条指令
        else if (op == LC) {ax = *(char *)ax;} // 根据ax里存的地址，找到那个字符存到ax里
        else if (op == LI) {ax = *(int *)ax;} // 根据地址，找到那个int存到ax里
        else if (op == SC) {*(char *)*sp = ax} // 一个挨一个地更新内存内容：把当前ax内的字符存入它本属于的内存地址，然后将下一格内存内容存入ax
                                                //stack里装着一个个内存地址。sp=stack顶端抽屉，*sp意为取抽屉里内容物（内存地址）。
        else if (op == SC) {*(char *)*sp = ax}
        else if (op == PUSH) {*--sp = ax;}          // push the value of ax onto the stack
        else if (op == JMP)  {pc = (int *)*pc;}     // JMP <addr> 是跳转指令。无条件地将当前的 PC 寄存器设置为指定的 <addr>。当前正在执行的指令(op)是“JMP”,pc指向下一句指令的地址
        else if (op == JZ)   {pc = ax ? pc + 1 : (int *)*pc;}         // 当前运行代码的地址是op；op+1==pc,*op==JMP,；即将要运行的那行代码的地址是pc+1。if ax==0， then pc:=即将要运行的那行代码的地址，else pc:=
    //  else if (op == JZ)   {pc = ax ? pc : (int *)*pc;}  如果你按原程序做，就会出现“跳一行”的现象
    //  else if (op == JZ)   {op = *pc; pc = ax ? pc + 1 : (int *)*pc;} 非得按作者说的做也可以，但必须赶在pc+1之前，记住pc当前值
    // 如果你发现按照作者说的做，运行居然没问题，那就说明：机器其实每次“跳转”(第1类跳转：条件JMP。第2类跳转：call)之后，都会偷偷地让op:=*(pc-1)
        else if (op == JNZ)  {pc = ax ? (int *)*pc : pc + 1;}        
    //  else if (op == JNZ)  {pc = ax ? (int *)*pc : pc;} 
    //  else if (op == JNZ)  {op = *pc; pc = ax ? (int *)*pc : pc + 1;} 
        
        else if (op == CALL) {*--sp = (int)(pc+1); pc = (int *)*pc;}           // call subroutine 当前运行：call ***，存下来的不是下一行，而是下下一行。待会儿跳回来的时候，机器偷偷地让op:=*(pc-1)
      //else if (op == RET)  {pc = (int *)*sp++;}                              // return from subroutine;这里我们把 RET 相关的内容注释了，是因为之后我们将用 LEV 指令来代替它

        else if (op == ENT)  {*--sp = (int)bp; bp = sp; sp = sp - *pc++;}      // make new stack frame  
// 作者直接pc++了，说明“ENT <size>”这句话处于子函数中，pc已经完成跳转

// 现在子函数要返回了，默认pc已完成跳转。下面这句话做的事情：回收子函数的frame
        else if (op == ADJ) {sp = sp + *pc++;} // 奇怪。。不用bp吗？？ 哦，下面用
        else if (op == LEV) {sp = bp; bp = (int *)*sp++; pc = (int *)*sp++;}
    
    ////////LEA




    
    }

    return 0;
}

int main(int argc, char **argv)
{
    int i, fd;

    argc--;
    argv++;

    poolsize = 256 * 1024; // arbitrary size
    line = 1;

    if ((fd = open(*argv, 0)) < 0) {
        printf("could not open(%s)\n", *argv);
        return -1;
    }

    if (!(src = old_src = malloc(poolsize))) {
        printf("could not malloc(%d) for source area\n", poolsize);
        return -1;
    }

    // read the source file
    if ((i = read(fd, src, poolsize-1)) <= 0) {
        printf("read() returned %d\n", i);
        return -1;
    }
    src[i] = 0; // add EOF character
    close(fd);

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

    memset(text, 0, poolsize);//initiate the memory
    memset(data, 0, poolsize);
    memset(stack, 0, poolsize);

    bp = sp = (int *)((int)stack + poolsize);//SP，指针寄存器，永远指向栈顶，“stack”是水位溢出警戒线
    ax = 0;////////////

    program();
    return eval();
}