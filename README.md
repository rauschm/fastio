# fastio

Erstaunlicherweise ist die Performance der C/C++ Standard-Ein- und Ausgabe-Funktionen `scanf`/`printf` und der Operatoren `<<`/`>>` nicht allzu hoch.
Ein simples Lesen und Schreiben einer CSV-Datei mit 20 Mio Zeilen, jede Zeile hat 4 Spalten mit zusammen ca. 41 Zeichen, dauert mit dem folgenden Programm ca. 67 Sekunden.

![](schnecke.png)

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
Diese Zahl alleine sagt noch nicht viel aus. Könnte doch der Computer insgesamt langsam sein. Allerdings wird von dem folgenden Programm, das die **fastio**-Funktionen verwendet, die gleiche Arbeit auf demselben Computer in ca. 3,6 Sekunden verrichtet, was einer Steigerung der Verarbeitungsgeschwindigkeit um **1760%** entspricht.
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
Mit dem folgenden C++-Programm, das ebenfalls wieder die gleiche Arbeit erledigt, dauert die Verarbeitung unerhörte 825 Sekunden.
```c++
/* slowcopypp.cpp */
#include <iostream>
#include <iomanip>

int main() {
  int                     i;
  unsigned int            x;
  unsigned long long int  xll;
  char                    s[100];
  char                    d;

  std::cout << std::uppercase << std::setfill('0');
  while (std::cin >> std::dec >> i   >> d
                  >> std::hex >> x   >> d 
                              >> xll >> d 
                              >> s) {
    std::cout << std::setw(1)  << std::dec << i   << ';'
              << std::setw(8)  << std::hex << x   << ';'
              << std::setw(16) <<             xll << ';'
              << std::setw(0)  <<             s   << '\n';
  }

  return 0;
}
```
Das folgende C++-Programm, das die **fastio**-Funktionen verwendet, braucht dagegen für die gleiche Arbeit nur 4,4 Sekunden.
```c++
/* fastcopypp.cpp */
#include "fastio.h"

int main() {
  int          i;
  fio::x       x;
  fio::llx     xll;
  const char*  s;

  fio_reader(in);
  fio_writer(ot);

  while (in.fio_read(i, x, xll, s)) {
    ot.fio_write(i, x, xll, s);
  }

  return 0;
}
```
Die Messungen erfolgten mit dem Microsoft (R) C/C++-Optimierungscompiler Version 19.28.29913 für x86.

Unter Linux mit dem GNU-C/C++-Compiler in der Version 9.3.0 wird lediglich eine Steigerung der Verarbeitungsgeschwindigkeit um **770%** (C) bzw **1530%** (C++) erreicht.

Das Projekt enthält alle Dateien, um den Test zu wiederholen. Ein `make perf` reicht aus. Zum Ausführen des Makefile wird allerdings GNU Make benötigt.
