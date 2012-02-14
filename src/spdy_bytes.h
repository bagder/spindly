#ifndef SPDY_BYTES_H_
#define SPDY_BYTES_H_

#define BE_LOAD_16(data) (data[1] | (data[0] << 8))
#define BE_LOAD_32(data) (data[3] | (data[2] << 8) |            \
                          (data[1] << 16) | (data[0] << 24))

#define BE_STORE_16(target, source)             \
  target[1] = source & 0xFF;                    \
  target[0] = (source >> 8) & 0xFF
#define BE_STORE_24(target, source)  \
  target[2] = source & 0xFF;         \
  target[1] = (source >> 8) & 0xFF;  \
  target[0] = (source >> 16) & 0xFF;
#define BE_STORE_32(target, source)  \
  target[3] = source & 0xFF;         \
  target[2] = (source >> 8) & 0xFF;  \
  target[1] = (source >> 16) & 0xFF; \
  target[0] = (source >> 24) & 0xFF

#endif

