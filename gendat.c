/*------------------------------------------------------------------------------
  G E N D A T . C

  Das Programm erzeugt eine vorgebbare Anzahl von mehr oder weniger zufälligen
  CSV-Datensätzen zum Testen von Dateiverarbeitungen.

  Aufruf: gendat [Anzahl] [> Datei]
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>

int my_random(void);

void main(int argc, char*argv[])
{
  int n_max = (argc == 2) ? atoi(argv[1]) : 10;

  for (int n = 0; n < n_max; n++)
    printf("%d;%08X;%016llX;ABCDE\n",
           my_random() % 1000000000,
           my_random() * 2,
           (my_random() * 100000ULL + my_random()) * 100000ULL + my_random());
}


int my_random(void)
{
  static int r = 1161100658;
  return r = ((long long) r * 48271) % 2147483647;
}

