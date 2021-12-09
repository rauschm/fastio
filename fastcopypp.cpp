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
