#ifndef SHARED_TYPES
#define SHARED_TYPES
#include <stdint.h>
#define ALGO_NUM (6)

typedef enum
{
    MD5_T    = 0,
    SHA1_T   = 1,
    SHA224_T = 2,
    SHA256_T = 3,
    SHA384_T = 4,
    SHA512_T = 5,
} algo_type_t;

typedef enum
{
    MD5_SZ    = 16,
    SHA_SZ    = 20,
    SHA224_SZ = 28,
    SHA256_SZ = 32,
    SHA384_SZ = 48,
    SHA512_SZ = 64,
} digest_len;

uint32_t digest_size_list[ALGO_NUM]={ (uint32_t)MD5_SZ, (uint32_t)SHA_SZ, (uint32_t)SHA224_SZ, (uint32_t)SHA256_SZ, (uint32_t)SHA384_SZ, (uint32_t)SHA512_SZ};

#endif /* SHARED_TYPES */
