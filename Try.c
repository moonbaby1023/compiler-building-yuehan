#include <stdio.h>
#include <stdlib.h>

int main()
{
//printf("ascii of 0 is %d\n", '0');
//printf("ascii of 9 is %d\n", '9');
//printf("ascii of A is %d\n", 'A');
//printf("ascii of Z is %d\n", 'Z');
//printf("ascii of a is %d\n", 'a');
//printf("ascii of z is %d\n", 'z');
//printf("ascii of _ is %d\n", '_');
long long a = 1;
int i = 0;
for (i = 0; i<63; i++)
{
	a = a*2;
}
printf("a = %lld\n", a);

}




