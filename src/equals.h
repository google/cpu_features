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
