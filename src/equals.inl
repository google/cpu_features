// Copyright 2021 Google LLC
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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static bool equals(const char *lhs, const char *rhs, size_t count) {
  size_t offset = 0;

#define CHUNK_EQUALS(TYPE)                  \
  while (count - offset >= sizeof(TYPE)) {  \
    TYPE l = *(const TYPE *)(lhs + offset); \
    TYPE r = *(const TYPE *)(rhs + offset); \
    if (l != r) return false;               \
    offset += sizeof(TYPE);                 \
  }

  CHUNK_EQUALS(uint64_t)
  CHUNK_EQUALS(uint32_t)
  CHUNK_EQUALS(uint16_t)
  CHUNK_EQUALS(uint8_t)
#undef CHUNK_EQUALS

  return true;
}
