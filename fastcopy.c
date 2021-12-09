#include "fastio.h"

void main(void) {
  int                     i;
  unsigned int            x;
  unsigned long long int  xll;
  const char*             s;

  fio_reader(in);
  fio_writer(ot);

  fio_init_r(in);
  fio_init_w(ot);

  while (fio_read(in, di(i), xu(x), xU(xll), sc(s))) {
    fio_write(ot, id(i), ux(x), Ux(xll), cs(s));
  }

  fio_exit_r(in);
  fio_exit_w(ot);
}
