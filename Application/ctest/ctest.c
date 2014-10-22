/***************************************************************************
*
*  ctest
*
* a test c program
 ***************************************************************************/
#include <stdio.h>

int main(void)
{
  int a, b;
  a = 0;
  b = 0;
  printf("hello! C program in UEFI!\n");
  printf("Test plus math, please input two number:");
  scanf("%d %d", &a, &b);
  printf("%d + %d = %d\n", a, b, a+b);
  return 0;
}
