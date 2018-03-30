//
// Created by c6s on 18-3-31.
//
#include <CRC32.h>
#include <assert.h>
using namespace std;
int main() {
  assert (CRC32::getCRC32("1234567890") == 0x261DAEE5);
}