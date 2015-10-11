#include <iostream>
#include <cstdlib>
#include <cstdio>

using namespace std;

void printer_func(int z);

void printer_func(int z) {
  cout<<endl<<"Hello World"<<endl;
}

int main() {
  printer_func(0);
  return 0;
}

