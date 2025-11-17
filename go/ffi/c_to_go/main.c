#include <stdio.h>
#include <unistd.h>

#include "libgo.h"

int main(int argc, char* argv[]) {
  printf("inside main ...\n");

  // Call the Go function AddInt
  GoUint64 result = AddInt(42, 58);
  printf("Result from Go AddInt(42, 58): %llu\n", (unsigned long long)result);

  printf("Sleeping for 100 seconds...\n");
  sleep(100);
  printf("Woke up after sleep!\n");

  return 0;
}
