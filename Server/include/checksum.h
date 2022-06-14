#ifndef CHECKSUM
#define CHECKSUM

#include "../../shared/Utils.h"
#include "../../shared/types.h"

void call_sum_init(algo_type_t algo, void *CTX);

void call_sum_update(algo_type_t algo, void *CTX, uint8_t *data,
                     long data_len);

void call_sum_finale(algo_type_t algo, void *CTX, uint8_t *digest);

uint8_t *checksum(algo_type_t option, uint8_t *file_data, long file_size);

char *checksum_file(algo_type_t option, uint8_t *file_data, long file_size);

void writeLog(char *file_name, algo_type_t option, char *checksum, long file_size, float seconds);

char *verify_checksum(algo_type_t option, char *file_data, long file_size,
                      char *checksum_file);

#endif // CHECKSUM