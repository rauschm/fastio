#include <iostream>
#include <iomanip>

int main() {
  int                     i;
  unsigned int            x;
  unsigned long long int  xll;
  char                    s[100];
  char                    d;

  /*std::ios::sync_with_stdio(false);*/
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
