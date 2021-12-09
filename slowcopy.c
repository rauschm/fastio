#include <stdio.h>

void main(void) {
  int                     i;
  unsigned int            x;
  unsigned long long int  xll;
  char                    s[100];

  while (scanf("%d;%x;%llx;%s", &i, &x, &xll, s) != EOF) {
    printf("%d;%08X;%016llX;%s\n", i, x, xll, s);
  }
}
