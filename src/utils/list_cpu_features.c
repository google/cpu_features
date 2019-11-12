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

// This program dumps current host data to the standard output.
// Output can be text or json if the `--json` flag is passed.

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu_features_macros.h"

#if defined(CPU_FEATURES_ARCH_X86)
#include "cpuinfo_x86.h"
#elif defined(CPU_FEATURES_ARCH_ARM)
#include "cpuinfo_arm.h"
#elif defined(CPU_FEATURES_ARCH_AARCH64)
#include "cpuinfo_aarch64.h"
#elif defined(CPU_FEATURES_ARCH_MIPS)
#include "cpuinfo_mips.h"
#elif defined(CPU_FEATURES_ARCH_PPC)
#include "cpuinfo_ppc.h"
#endif

// Design principles
// -----------------
// We build a tree structure containing all the data to be displayed.
// Then depending on the output type (text or json) we walk the tree and display
// the data accordingly.

// We use a bump allocator to allocate strings and nodes of the tree,
// Memory is not intented to be reclaimed.
typedef struct {
  char* ptr;
  size_t size;
} BumpAllocator;

// Allocate a buffer of size `size`.
static BumpAllocator BA_Create(size_t size) {
  char* const ptr = (char*)malloc(size);
  BumpAllocator BA;
  if (ptr) BA = (BumpAllocator){.ptr = ptr, .size = size};
  return BA;
}

// Update the available memory left in the BumpAllocator.
static void* BA_Bump(BumpAllocator* BA, size_t size) {
  assert(BA->size >= size);
  void* ptr = BA->ptr;
  BA->size -= size;
  BA->ptr += size;
  return ptr;
}

// The type of the nodes in the tree.
typedef enum {
  TNT_INVALID,
  TNT_INT,
  TNT_MAP,
  TNT_MAP_ENTRY,
  TNT_ARRAY,
  TNT_ARRAY_ELEMENT,
  TNT_STRING,
} TreeValueType;

// The node in the tree.
typedef struct TreeValue {
  TreeValueType type;
  unsigned integer;
  const char* string;
  struct TreeValue* value;
  struct TreeValue* next;
} TreeValue;

// Allocates a node inside a BumpAllocator.
static TreeValue* BA_TreeValue(BumpAllocator* BA, TreeValueType type) {
  TreeValue* TV = (TreeValue*)BA_Bump(BA, sizeof(TreeValue));
  assert(TV);
  TV->type = type;
  return TV;
}

// Allocates an integer node inside a BumpAllocator.
static TreeValue* CreateInt(BumpAllocator* BA, int value) {
  TreeValue* TV = BA_TreeValue(BA, TNT_INT);
  TV->integer = value;
  return TV;
}

// Allocates a string node inside a BumpAllocator.
// `value` must outlive the tree.
static TreeValue* CreateConstantString(BumpAllocator* BA, const char* value) {
  TreeValue* TV = BA_TreeValue(BA, TNT_STRING);
  TV->string = value;
  return TV;
}

// Allocates a map node inside a BumpAllocator.
static TreeValue* CreateMap(BumpAllocator* BA) {
  TreeValue* TV = BA_TreeValue(BA, TNT_MAP);
  TV->next = NULL;
  return TV;
}

// Allocates an array node inside a BumpAllocator.
static TreeValue* CreateArray(BumpAllocator* BA) {
  TreeValue* TV = BA_TreeValue(BA, TNT_ARRAY);
  TV->next = NULL;
  return TV;
}

// Allocates a formatted string inside a BumpAllocator.
static TreeValue* CreatePrintfString(BumpAllocator* BA, const char* format,
                                     ...) {
  va_list arglist;
  va_start(arglist, format);
  char* const ptr = BA->ptr;
  const int written = vsnprintf(ptr, BA->size, format, arglist);
  va_end(arglist);
  if (written < 0 || written >= BA->size) return NULL;
  return CreateConstantString(BA, (char*)BA_Bump(BA, written));
}

static TreeValue* CreateString(BumpAllocator* BA, const char* value) {
  return CreatePrintfString(BA, "%s", value);
}

// Allocates a map entry node inside a BumpAllocator.
static void AddMapEntry(BumpAllocator* BA, TreeValue* map, const char* key,
                        TreeValue* value) {
  assert(map && map->type == TNT_MAP);
  TreeValue* current = map;
  while (current->next) current = current->next;
  current->next = (TreeValue*)BA_Bump(BA, sizeof(TreeValue));
  current->next->type = TNT_MAP_ENTRY;
  current->next->string = key;
  current->next->value = value;
  current->next->next = NULL;
}

// Allocates aan array element node inside a BumpAllocator.
static void AddArrayElement(BumpAllocator* BA, TreeValue* array,
                            TreeValue* value) {
  assert(array && array->type == TNT_ARRAY);
  TreeValue* current = array;
  while (current->next) current = current->next;
  current->next = (TreeValue*)BA_Bump(BA, sizeof(TreeValue));
  current->next->type = TNT_ARRAY_ELEMENT;
  current->next->value = value;
  current->next->next = NULL;
}

static int cmp(const void* p1, const void* p2) {
  return strcmp(*(const char* const*)p1, *(const char* const*)p2);
}

#define DEFINE_ADD_FLAGS(HasFeature, FeatureName, FeatureType, LastEnum) \
  static void AddFlags(BumpAllocator* BA, TreeValue* map,                \
                       const FeatureType* features) {                    \
    size_t i;                                                            \
    const char* ptrs[LastEnum] = {0};                                    \
    size_t count = 0;                                                    \
    for (i = 0; i < LastEnum; ++i) {                                     \
      if (HasFeature(features, i)) {                                     \
        ptrs[count] = FeatureName(i);                                    \
        ++count;                                                         \
      }                                                                  \
    }                                                                    \
    qsort((void*)ptrs, count, sizeof(char*), cmp);                       \
    TreeValue* const array = CreateArray(BA);                            \
    for (i = 0; i < count; ++i)                                          \
      AddArrayElement(BA, array, CreateConstantString(BA, ptrs[i]));     \
    AddMapEntry(BA, map, "flags", array);                                \
  }

#if defined(CPU_FEATURES_ARCH_X86)
DEFINE_ADD_FLAGS(GetX86FeaturesEnumValue, GetX86FeaturesEnumName, X86Features,
                 X86_LAST_)
#elif defined(CPU_FEATURES_ARCH_ARM)
DEFINE_ADD_FLAGS(GetArmFeaturesEnumValue, GetArmFeaturesEnumName, ArmFeatures,
                 ARM_LAST_)
#elif defined(CPU_FEATURES_ARCH_AARCH64)
DEFINE_ADD_FLAGS(GetAarch64FeaturesEnumValue, GetAarch64FeaturesEnumName,
                 Aarch64Features, AARCH64_LAST_)
#elif defined(CPU_FEATURES_ARCH_MIPS)
DEFINE_ADD_FLAGS(GetMipsFeaturesEnumValue, GetMipsFeaturesEnumName,
                 MipsFeatures, MIPS_LAST_)
#elif defined(CPU_FEATURES_ARCH_PPC)
DEFINE_ADD_FLAGS(GetPPCFeaturesEnumValue, GetPPCFeaturesEnumName, PPCFeatures,
                 PPC_LAST_)
#endif

// Prints a json string with characters escaping.
static void printJsonString(const char* str) {
  putchar('"');
  for (; str && *str; ++str) {
    switch (*str) {
      case '\"':
      case '\\':
      case '/':
      case '\b':
      case '\f':
      case '\n':
      case '\r':
      case '\t':
        putchar('\\');
    }
    putchar(*str);
  }
  putchar('"');
}

// Walks a TreeValue and print it as json.
static void printJson(const TreeValue* current) {
  assert(current);
  switch (current->type) {
    case TNT_INT:
      printf("%d", current->integer);
      break;
    case TNT_STRING:
      printJsonString(current->string);
      break;
    case TNT_ARRAY:
      putchar('[');
      if (current->next) printJson(current->next);
      putchar(']');
      break;
    case TNT_MAP:
      putchar('{');
      if (current->next) printJson(current->next);
      putchar('}');
      break;
    case TNT_MAP_ENTRY:
      printf("\"%s\":", current->string);
      printJson(current->value);
      if (current->next) {
        putchar(',');
        printJson(current->next);
      }
      break;
    case TNT_ARRAY_ELEMENT:
      printJson(current->value);
      if (current->next) {
        putchar(',');
        printJson(current->next);
      }
      break;
  }
}

// Walks a TreeValue and print it as text.
static void printTextField(const TreeValue* current) {
  switch (current->type) {
    case TNT_INT:
      printf("%3d (0x%02X)", current->integer, current->integer);
      break;
    case TNT_STRING:
      fputs(current->string, stdout);
      break;
    case TNT_ARRAY:
      if (current->next) printTextField(current->next);
      break;
    case TNT_MAP:
      if (current->next) printJson(current->next);
      break;
    case TNT_MAP_ENTRY:
      printf("%-15s : ", current->string);
      printTextField(current->value);
      if (current->next) {
        putchar('\n');
        printTextField(current->next);
      }
      break;
    case TNT_ARRAY_ELEMENT:
      printTextField(current->value);
      if (current->next) {
        putchar(',');
        printTextField(current->next);
      }
      break;
  }
}

static void printTextRoot(const TreeValue* current) {
  if (current->type == TNT_MAP && current->next) printTextField(current->next);
}
static void showUsage(const char* name) {
  printf(
      "\n"
      "Usage: %s [options]\n"
      "      Options:\n"
      "      -h | --help     Show help message.\n"
      "      -j | --json     Format output as json instead of plain text.\n"
      "\n",
      name);
}

static TreeValue* CreateTree(BumpAllocator* BA) {
  TreeValue* root = CreateMap(BA);
#if defined(CPU_FEATURES_ARCH_X86)
  char brand_string[49];
  const X86Info info = GetX86Info();
  FillX86BrandString(brand_string);
  AddMapEntry(BA, root, "arch", CreateString(BA, "x86"));
  AddMapEntry(BA, root, "brand", CreateString(BA, brand_string));
  AddMapEntry(BA, root, "family", CreateInt(BA, info.family));
  AddMapEntry(BA, root, "model", CreateInt(BA, info.model));
  AddMapEntry(BA, root, "stepping", CreateInt(BA, info.stepping));
  AddMapEntry(BA, root, "uarch",
              CreateString(BA, GetX86MicroarchitectureName(
                                   GetX86Microarchitecture(&info))));
  AddFlags(BA, root, &info.features);
#elif defined(CPU_FEATURES_ARCH_ARM)
  const ArmInfo info = GetArmInfo();
  AddMapEntry(BA, root, "arch", CreateString(BA, "ARM"));
  AddMapEntry(BA, root, "implementer", CreateInt(BA, info.implementer));
  AddMapEntry(BA, root, "architecture", CreateInt(BA, info.architecture));
  AddMapEntry(BA, root, "variant", CreateInt(BA, info.variant));
  AddMapEntry(BA, root, "part", CreateInt(BA, info.part));
  AddMapEntry(BA, root, "revision", CreateInt(BA, info.revision));
  AddFlags(BA, root, &info.features);
#elif defined(CPU_FEATURES_ARCH_AARCH64)
  const Aarch64Info info = GetAarch64Info();
  AddMapEntry(BA, root, "arch", CreateString(BA, "aarch64"));
  AddMapEntry(BA, root, "implementer", CreateInt(BA, info.implementer));
  AddMapEntry(BA, root, "variant", CreateInt(BA, info.variant));
  AddMapEntry(BA, root, "part", CreateInt(BA, info.part));
  AddMapEntry(BA, root, "revision", CreateInt(BA, info.revision));
  AddFlags(BA, root, &info.features);
#elif defined(CPU_FEATURES_ARCH_MIPS)
  const MipsInfo info = GetMipsInfo();
  AddMapEntry(BA, root, "arch", CreateString(BA, "mips"));
  AddFlags(BA, root, &info.features);
#elif defined(CPU_FEATURES_ARCH_PPC)
  const PPCInfo info = GetPPCInfo();
  const PPCPlatformStrings strings = GetPPCPlatformStrings();
  AddMapEntry(BA, root, "arch", CreateString(BA, "ppc"));
  AddMapEntry(BA, root, "platform", CreateString(BA, strings.platform));
  AddMapEntry(BA, root, "model", CreateString(BA, strings.model));
  AddMapEntry(BA, root, "machine", CreateString(BA, strings.machine));
  AddMapEntry(BA, root, "cpu", CreateString(BA, strings.cpu));
  AddMapEntry(BA, root, "instruction", CreateString(BA, strings.type.platform));
  AddMapEntry(BA, root, "microarchitecture",
              CreateString(BA, strings.type.base_platform));
  AddFlags(BA, root, &info.features);
#endif
  return root;
}

int main(int argc, char** argv) {
  BumpAllocator BA = BA_Create(64 * 1024);
  const TreeValue* const root = CreateTree(&BA);
  bool outputJson = false;
  int i = 1;
  for (; i < argc; ++i) {
    const char* arg = argv[i];
    if (strcmp(arg, "-j") == 0 || strcmp(arg, "--json") == 0) {
      outputJson = true;
    } else {
      showUsage(argv[0]);
      if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        return EXIT_SUCCESS;
      return EXIT_FAILURE;
    }
  }
  if (outputJson)
    printJson(root);
  else
    printTextRoot(root);
  putchar('\n');
  return EXIT_SUCCESS;
}
