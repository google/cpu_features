#include <stdio.h>
#include "cpu-features.h"

int main() {
  printf("android_getCpuFamily()=%x\n", android_getCpuFamily());
  printf("android_getCpuFeatures()=%x\n", android_getCpuFeatures());
  printf("android_getCpuCount()=%x\n", android_getCpuCount());
#ifdef __arm__
  printf("android_getCpuIdArm()=%x\n", android_getCpuIdArm());
#endif  //__arm__
}
