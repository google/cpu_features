// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu_features_macros.h"
#include "cpuinfo_aarch64.h"
#include "cpuinfo_arm.h"
#include "cpuinfo_mips.h"
#include "cpuinfo_x86.h"

// Json file handle
FILE *jsonFile = (void *)0;

// Helper function to print usage
static void showUsage(int argc, char **argv) {
  fprintf(stdout,
          "\n"
          "Usage: %s [options]\n"
          "      Options:\n"
          "      -h | --help     Show help message\n"
          "      -l | --list     List cpu features (default)\n"
          "      -j | --json     Redirect info to json file [filename]\n"
          "\n",
          argv[0]
  );
}

// Prints a named numeric value in both decimal and hexadecimal.
void PrintN(const char* field, int value) {
  fprintf(stdout, "%-15s : %3d (0x%02X)\n", field, value, value);
  if (jsonFile)
    fprintf(jsonFile, "    \"%s\": %d,\n", field, value);
}

// Prints a named string.
void PrintS(const char* field, const char* value) {
  fprintf(stdout, "%-15s : %s\n", field, value);
  if (jsonFile)
    fprintf(jsonFile, "    \"%s\": \"%s\",\n", field, value);
}

static int cmp(const void* p1, const void* p2) {
  return strcmp(*(const char* const*)p1, *(const char* const*)p2);
}

#define DEFINE_PRINT_FLAGS(HasFeature, FeatureName, FeatureType, LastEnum) \
  void PrintFlags(const FeatureType* features) {                           \
    size_t i;                                                              \
    const char* ptrs[LastEnum] = {0};                                      \
    size_t count = 0;                                                      \
    for (i = 0; i < LastEnum; ++i) {                                       \
      if (HasFeature(features, i)) {                                       \
        ptrs[count] = FeatureName(i);                                      \
        ++count;                                                           \
      }                                                                    \
    }                                                                      \
    qsort(ptrs, count, sizeof(char*), cmp);                                \
    fprintf(stdout, "%-15s : ", "flags");                                  \
    if (jsonFile) fprintf(jsonFile, "    \"%s\" : [\n", "flags");          \
    for (i = 0; i < count; ++i) {                                          \
      if (i > 0) {                                                         \
        fprintf(stdout, ", ");                                             \
        if (jsonFile) fprintf(jsonFile, ",\n");                            \
      }                                                                    \
      fprintf(stdout, "%s", ptrs[i]);                                      \
      if (jsonFile) fprintf(jsonFile, "        \"%s\"", ptrs[i]);          \
    }                                                                      \
    fprintf(stdout, "\n");                                                 \
    if (jsonFile) fprintf(jsonFile, "\n    ]\n");                          \
  }

#if defined(CPU_FEATURES_ARCH_X86)
DEFINE_PRINT_FLAGS(GetX86FeaturesEnumValue, GetX86FeaturesEnumName, X86Features,
                   X86_LAST_)
#elif defined(CPU_FEATURES_ARCH_ARM)
DEFINE_PRINT_FLAGS(GetArmFeaturesEnumValue, GetArmFeaturesEnumName, ArmFeatures,
                   ARM_LAST_)
#elif defined(CPU_FEATURES_ARCH_AARCH64)
DEFINE_PRINT_FLAGS(GetAarch64FeaturesEnumValue, GetAarch64FeaturesEnumName,
                   Aarch64Features, AARCH64_LAST_)
#elif defined(CPU_FEATURES_ARCH_MIPS)
DEFINE_PRINT_FLAGS(GetMipsFeaturesEnumValue, GetMipsFeaturesEnumName,
                   MipsFeatures, MIPS_LAST_)
#endif

//
// Program entry
//
int main(int argc, char** argv) {

  char *optionJsonFile = NULL;
  int idx = 0;

  for (idx = 1; idx < argc; ++idx) {

    if ((strcmp(argv[idx], "-h") == 0) || (strcmp(argv[idx], "--h") == 0)) {
        showUsage(argc, argv);
        exit(EXIT_FAILURE);
    }
    else if ((strcmp(argv[idx], "-l") == 0) || (strcmp(argv[idx], "--list") == 0)) {
      // Nothing to do
    }
    else if ((strcmp(argv[idx], "-j") == 0) || (strcmp(argv[idx], "--json") == 0)) {
      if (argv[idx + 1])
        optionJsonFile = argv[++idx];
    }
  }

  if (optionJsonFile != 0) {
    jsonFile = fopen(optionJsonFile, "w+");

    if (jsonFile) {
      fprintf(jsonFile, "# cpu_features\n");
      fprintf(jsonFile, "{\n");
    }
  }

#if defined(CPU_FEATURES_ARCH_X86)
  char brand_string[49];
  const X86Info info = GetX86Info();
  FillX86BrandString(brand_string);

  PrintS("arch", "x86");
  PrintS("brand", brand_string);
  PrintN("family", info.family);
  PrintN("model", info.model);
  PrintN("stepping", info.stepping);
  PrintS("uarch", GetX86MicroarchitectureName(GetX86Microarchitecture(&info)));
  PrintFlags(&info.features);

#elif defined(CPU_FEATURES_ARCH_ARM)
  const ArmInfo info = GetArmInfo();

  PrintS("arch", "ARM");
  PrintN("implementer", info.implementer);
  PrintN("architecture", info.architecture);
  PrintN("variant", info.variant);
  PrintN("part", info.part);
  PrintN("revision", info.revision);
  PrintFlags(&info.features);

#elif defined(CPU_FEATURES_ARCH_AARCH64)
  const Aarch64Info info = GetAarch64Info();

  PrintS("arch", "aarch64");
  PrintN("implementer", info.implementer);
  PrintN("variant", info.variant);
  PrintN("part", info.part);
  PrintN("revision", info.revision);
  PrintFlags(&info.features);

#elif defined(CPU_FEATURES_ARCH_MIPS)
  const MipsInfo info = GetMipsInfo();

  PrintS("arch", "mips");
  PrintFlags(&info.features);

#endif

  if (jsonFile) {
    fprintf(jsonFile, "}\n");
    fclose(jsonFile);
  }

  return 0;
}
