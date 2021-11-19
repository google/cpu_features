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

#ifndef CPU_FEATURES_BUMP_ALLOCATOR_H_
#define CPU_FEATURES_BUMP_ALLOCATOR_H_

#include <stddef.h>

#define ALIGN 8

// We use a bump allocator to allocate strings and nodes of the tree,
// Memory is not intended to be reclaimed.
typedef struct {
  char* ptr;
  size_t size;
} BumpAllocator;

void assertAligned();
void BA_Align();
void* BA_Bump(size_t size);

#endif  // CPU_FEATURES_BUMP_ALLOCATOR_H_
