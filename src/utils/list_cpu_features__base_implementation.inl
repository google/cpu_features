// Copyright 2017 Google LLC
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

// Design principles
// -----------------
// We build a tree structure containing all the data to be displayed.
// Then depending on the output type (text or json) we walk the tree and display
// the data accordingly.

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bump_allocator.h"
#include "node.h"

static void internal_error() {
  fputs("internal error\n", stderr);
  exit(EXIT_FAILURE);
}

char gGlobalBuffer[64 * 1024];
BumpAllocator gBumpAllocator = {.ptr = gGlobalBuffer,
                                .size = sizeof(gGlobalBuffer)};

void assertAligned() {
  if ((uintptr_t)(gBumpAllocator.ptr) % ALIGN) internal_error();
}

void BA_Align() {
  while (gBumpAllocator.size && (uintptr_t)(gBumpAllocator.ptr) % ALIGN) {
    --gBumpAllocator.size;
    ++gBumpAllocator.ptr;
  }
  assertAligned();
}

// Update the available memory left in the BumpAllocator.
void* BA_Bump(size_t size) {
  assertAligned();
  // Align size to next 8B boundary.
  size = (size + ALIGN - 1) / ALIGN * ALIGN;
  if (gBumpAllocator.size < size) internal_error();
  void* ptr = gBumpAllocator.ptr;
  gBumpAllocator.size -= size;
  gBumpAllocator.ptr += size;
  return ptr;
}

// Creates an initialized Node.
Node* BA_CreateNode(NodeType type) {
  Node* tv = (Node*)BA_Bump(sizeof(Node));
  assert(tv);
  *tv = (Node){.type = type};
  return tv;
}

// Adds an integer node.
Node* CreateInt(int value) {
  Node* tv = BA_CreateNode(NT_INT);
  tv->integer = value;
  return tv;
}

// Adds a string node.
// `value` must outlive the tree.
Node* CreateConstantString(const char* value) {
  Node* tv = BA_CreateNode(NT_STRING);
  tv->string = value;
  return tv;
}

// Adds a map node.
Node* CreateMap() { return BA_CreateNode(NT_MAP); }

// Adds an array node.
Node* CreateArray() { return BA_CreateNode(NT_ARRAY); }

// Adds a formatted string node.
Node* CreatePrintfString(const char* format, ...) {
  va_list arglist;
  va_start(arglist, format);
  char* const ptr = gBumpAllocator.ptr;
  const int written = vsnprintf(ptr, gBumpAllocator.size, format, arglist);
  va_end(arglist);
  if (written < 0 || written >= (int)gBumpAllocator.size) internal_error();
  return CreateConstantString((char*)BA_Bump(written));
}

// Adds a string node.
Node* CreateString(const char* value) {
  return CreatePrintfString("%s", value);
}

// Adds a map entry node.
void AddMapEntry(Node* map, const char* key, Node* value) {
  assert(map && map->type == NT_MAP);
  Node* current = map;
  while (current->next) current = current->next;
  current->next = (Node*)BA_Bump(sizeof(Node));
  *current->next = (Node){.type = NT_MAP_ENTRY, .string = key, .value = value};
}

// Adds an array element node.
void AddArrayElement(Node* array, Node* value) {
  assert(array && array->type == NT_ARRAY);
  Node* current = array;
  while (current->next) current = current->next;
  current->next = (Node*)BA_Bump(sizeof(Node));
  *current->next = (Node){.type = NT_ARRAY_ELEMENT, .value = value};
}

static int cmp(const void* p1, const void* p2) {
  return strcmp(*(const char* const*)p1, *(const char* const*)p2);
}

#define DEFINE_ADD_FLAGS(HasFeature, FeatureName, FeatureType, LastEnum) \
  static void AddFlags(Node* map, const FeatureType* features) {         \
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
    Node* const array = CreateArray();                                   \
    for (i = 0; i < count; ++i)                                          \
      AddArrayElement(array, CreateConstantString(ptrs[i]));             \
    AddMapEntry(map, "flags", array);                                    \
  }

Node* GetCacheTypeString(CacheType cache_type) {
  switch (cache_type) {
    case CPU_FEATURE_CACHE_NULL:
      return CreateConstantString("null");
    case CPU_FEATURE_CACHE_DATA:
      return CreateConstantString("data");
    case CPU_FEATURE_CACHE_INSTRUCTION:
      return CreateConstantString("instruction");
    case CPU_FEATURE_CACHE_UNIFIED:
      return CreateConstantString("unified");
    case CPU_FEATURE_CACHE_TLB:
      return CreateConstantString("tlb");
    case CPU_FEATURE_CACHE_DTLB:
      return CreateConstantString("dtlb");
    case CPU_FEATURE_CACHE_STLB:
      return CreateConstantString("stlb");
    case CPU_FEATURE_CACHE_PREFETCH:
      return CreateConstantString("prefetch");
  }
  UNREACHABLE();
}

void AddCacheInfo(Node* root, const CacheInfo* cache_info) {
  Node* array = CreateArray();
  for (int i = 0; i < cache_info->size; ++i) {
    CacheLevelInfo info = cache_info->levels[i];
    Node* map = CreateMap();
    AddMapEntry(map, "level", CreateInt(info.level));
    AddMapEntry(map, "cache_type", GetCacheTypeString(info.cache_type));
    AddMapEntry(map, "cache_size", CreateInt(info.cache_size));
    AddMapEntry(map, "ways", CreateInt(info.ways));
    AddMapEntry(map, "line_size", CreateInt(info.line_size));
    AddMapEntry(map, "tlb_entries", CreateInt(info.tlb_entries));
    AddMapEntry(map, "partitioning", CreateInt(info.partitioning));
    AddArrayElement(array, map);
  }
  AddMapEntry(root, "cache_info", array);
}

void printJsonString(const char* str) {
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

void printJson(const Node* current) {
  assert(current);
  switch (current->type) {
    case NT_INVALID:
      break;
    case NT_INT:
      printf("%d", current->integer);
      break;
    case NT_STRING:
      printJsonString(current->string);
      break;
    case NT_ARRAY:
      putchar('[');
      if (current->next) printJson(current->next);
      putchar(']');
      break;
    case NT_MAP:
      putchar('{');
      if (current->next) printJson(current->next);
      putchar('}');
      break;
    case NT_MAP_ENTRY:
      printf("\"%s\":", current->string);
      printJson(current->value);
      if (current->next) {
        putchar(',');
        printJson(current->next);
      }
      break;
    case NT_ARRAY_ELEMENT:
      printJson(current->value);
      if (current->next) {
        putchar(',');
        printJson(current->next);
      }
      break;
  }
}

void printTextField(const Node* current) {
  switch (current->type) {
    case NT_INVALID:
      break;
    case NT_INT:
      printf("%3d (0x%02X)", current->integer, current->integer);
      break;
    case NT_STRING:
      fputs(current->string, stdout);
      break;
    case NT_ARRAY:
      if (current->next) printTextField(current->next);
      break;
    case NT_MAP:
      if (current->next) {
        printf("{");
        printJson(current->next);
        printf("}");
      }
      break;
    case NT_MAP_ENTRY:
      printf("%-15s : ", current->string);
      printTextField(current->value);
      if (current->next) {
        putchar('\n');
        printTextField(current->next);
      }
      break;
    case NT_ARRAY_ELEMENT:
      printTextField(current->value);
      if (current->next) {
        putchar(',');
        printTextField(current->next);
      }
      break;
  }
}

void printTextRoot(const Node* current) {
  if (current->type == NT_MAP && current->next) printTextField(current->next);
}
