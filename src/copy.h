#include <stddef.h>
#include <stdint.h>

static void copy(char *__restrict dst, const char *src, size_t count) {
  size_t offset = 0;

#define CHUNK_COPY(TYPE)                                     \
  while (count - offset >= sizeof(TYPE)) {                   \
    *(TYPE *)(dst + offset) = *(const TYPE *)(src + offset); \
    offset += sizeof(TYPE);                                  \
  }

  CHUNK_COPY(uint64_t)
  CHUNK_COPY(uint32_t)
  CHUNK_COPY(uint16_t)
  CHUNK_COPY(uint8_t)

#undef CHUNK_COPY
}
