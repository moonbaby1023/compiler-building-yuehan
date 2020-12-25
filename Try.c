#include <stdio.h>
#include <stdlib.h>

int main()
{


FILE *fp = NULL; 
int i;
char *filepath = "./files/test.txt";
char * src;
int poolsize = 1024;

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
 {  // (全篇，读书人/内存地址，读入字数)
     printf("read() returned %d\n", i);
     return -1;
 }
 src[i] = 0; // add EOF character
 close(fp);

printf("%s", src);
}




