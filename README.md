# fastio

Erstaunlicherweise ist die Performance der C/C++ Standard-Ein- und Ausgabe-Funktionen `scanf`/`printf` und der Operatoren `<<`/`>>` nicht allzu hoch.
Ein simples Lesen und Schreiben einer CSV-Datei mit 20 Mio Zeilen, jede Zeile hat 4 Spalten mit zusammen ca. 41 Zeichen, dauert mit dem folgenden Programm ca. 65 Sekunden.
```c
/* slowcopy.c */
#include "stdio.h"
void main(void) {
  int                    i;
  unsigned int           x;
  unsigned long long int xll;
  char                   s[100];

  while (scanf("%d;%x;%llx;%s", &i, &x, &xll, s) != EOF) {
    printf("%d;%08X;%016llX;%s\n", i, x, xll, s);
  }
}
```
Diese Zahl alleine sagt noch nicht viel aus. Könnte doch der Computer insgesamt langsam sein. Allerdings wird von dem folgenden Programm, das die **fastio**-Funktionen verwendet, die gleiche Arbeit auf demselben Computer in ca. 4,3 Sekunden verrichtet, was einer Steigerung der Verarbeitungsgeschwindigkeit um **1400%** entspricht.
```c
/* fastcopy.c */
#include "fastio.h"
void main(void) {
  int                    i;
  unsigned int           x;
  unsigned long long int xll;
  const char*            s;

  fio_reader(in);
  fio_writer(ot);

  fio_init_r(in);
  fio_init_w(ot);

  while (fio_read(in, di(i), xu(x), xU(xll), sc(s))) {
    fio_write(ot, id(i), ux(x), Ux(xll), cs(s));
  }

  fio_exit_r(in);
  fio_exit_w(ot);

  return 0;
}
```
