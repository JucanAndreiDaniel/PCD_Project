#include "checksum.h"
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GENERIC_CONTEXT_SIZE \
    (2 * 1024) // 2048 bytes (2KB) should be enought for any context

static uint32_t digest_size_list[ALGO_NUM] = {(uint32_t)MD5_SZ, (uint32_t)SHA_SZ, (uint32_t)SHA224_SZ, (uint32_t)SHA256_SZ, (uint32_t)SHA384_SZ, (uint32_t)SHA512_SZ};

void call_sum_init(algo_type_t algo, void *CTX)
{
    switch (algo)
    {
    case SHA256_T:
        (void)SHA256_Init((SHA256_CTX *)CTX); // need to check error
        break;
    case SHA512_T:
        (void)SHA512_Init((SHA512_CTX *)CTX); // need to check error
        break;
    case MD5_T:
        (void)MD5_Init((MD5_CTX *)CTX); // need to check error
        break;
    case SHA1_T:
        (void)SHA1_Init((SHA_CTX *)CTX); // need to check error
        break;
    case SHA224_T:
        (void)SHA224_Init((SHA256_CTX *)CTX); // need to check error
        break;
    case SHA384_T:
        (void)SHA384_Init((SHA512_CTX *)CTX); // need to check error
        break;
    default:
        break;
    }
}

void call_sum_update(algo_type_t algo, void *CTX, uint8_t *data,
                     long data_len)
{
    switch (algo)
    {
    case SHA256_T:
        (void)SHA256_Update((SHA256_CTX *)CTX, data, data_len);
        break;
    case SHA512_T:
        (void)SHA512_Update((SHA512_CTX *)CTX, data, data_len);
        break;
    case MD5_T:
        (void)MD5_Update((MD5_CTX *)CTX, data, data_len);
        break;
    case SHA1_T:
        (void)SHA1_Update((SHA_CTX *)CTX, data, data_len);
        break;
    case SHA224_T:
        (void)SHA224_Update((SHA256_CTX *)CTX, data, data_len);
        break;
    case SHA384_T:
        (void)SHA384_Update((SHA512_CTX *)CTX, data, data_len);
        break;
    default:
        DBG_PRINT("default for now\n");
        break;
    }
}

void call_sum_finale(algo_type_t algo, void *CTX, uint8_t *digest)
{
    switch (algo)
    {
    case SHA256_T:
        (void)SHA256_Final(digest, (SHA256_CTX *)CTX);
        break;
    case SHA512_T:
        (void)SHA512_Final(digest, (SHA512_CTX *)CTX);
        break;
    case MD5_T:
        (void)MD5_Final(digest, (MD5_CTX *)CTX);
        break;
    case SHA1_T:
        (void)SHA1_Final(digest, (SHA_CTX *)CTX);
        break;
    case SHA224_T:
        (void)SHA224_Final(digest, (SHA256_CTX *)CTX);
        break;
    case SHA384_T:
        (void)SHA384_Final(digest, (SHA512_CTX *)CTX);
        break;
    default:
        DBG_PRINT("default for now\n");
        break;
    }
}

uint8_t *checksum(algo_type_t option, uint8_t *file_data, long file_size)
{
    void *ctx = malloc(GENERIC_CONTEXT_SIZE);

    if (NULL == ctx)
    {
        return;
    }
    call_sum_init(option, ctx);
    call_sum_update(option, ctx, file_data, file_size);
    uint8_t *digest = malloc(digest_size_list[(uint8_t)option]);

    call_sum_finale(option, ctx, digest);

    if (NULL != ctx)
    {
        free(ctx);
        ctx = NULL;
    }
    return digest;
}

char *checksum_file(algo_type_t option, uint8_t *file_data, long file_size)
{

    uint8_t *digest = checksum(option, file_data, file_size);

    char *checksum_str = malloc(sizeof(char) * digest_size_list[(uint8_t)option] * 2 + 1);
    for (int i = 0, j = 0; i < digest_size_list[(uint8_t)option]; i++, j += 2)
        sprintf(checksum_str + j, "%02x", digest[i]);
    checksum_str[sizeof(char) * digest_size_list[(uint8_t)option] * 2] = 0;

    if (digest != NULL)
    {
        free(digest);
        digest = NULL;
    }

    return checksum_str;
}

void writeLog(char *file_name, algo_type_t option, char *checksum, long file_size, float seconds)
{
    FILE *fp;
    fp = fopen(file_name, "a");

    if (NULL != fp)
    {
        fprintf(fp, "%d;%s,%zu;%f\n", (int)option, checksum, file_size, seconds);
        fflush(fp);
        fclose(fp);
    }
}
