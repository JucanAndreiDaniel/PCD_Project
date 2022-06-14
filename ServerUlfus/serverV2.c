#include "../shared/Utils.h"
#include "../shared/types.h"
#include <arpa/inet.h>
#include <jansson.h>
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <ulfius.h>
#include <unistd.h>

#include "include/http_compression_callback.h"
#include "include/u_example.h"

#define PORT 8080

#define SIZE (1024) // this should be changed
#define GENERIC_CONTEXT_SIZE \
  (2 * 1024) // 2048 bytes (2KB) should be enought for any context
#define MAX_FILE_SIZE (1024 * 1024 * 1024)
#define TEST_ALGO (4)

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static char file_name[] = "log.txt";

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

/**
 * decode a u_map into a string
 */
char *print_map(const struct _u_map *map)
{
  char *line, *to_return = NULL;
  const char **keys, *value;
  int len, i;
  if (map != NULL)
  {
    keys = u_map_enum_keys(map);
    for (i = 0; keys[i] != NULL; i++)
    {
      value = u_map_get(map, keys[i]);
      len = snprintf(NULL, 0, "key is %s, length is %zu, value is %.*s",
                     keys[i], u_map_get_length(map, keys[i]),
                     (int)u_map_get_length(map, keys[i]), value);
      line = o_malloc((len + 1) * sizeof(char));
      snprintf(line, (len + 1), "key is %s, length is %zu, value is %.*s",
               keys[i], u_map_get_length(map, keys[i]),
               (int)u_map_get_length(map, keys[i]), value);
      if (to_return != NULL)
      {
        len = o_strlen(to_return) + o_strlen(line) + 1;
        to_return = o_realloc(to_return, (len + 1) * sizeof(char));
        if (o_strlen(to_return) > 0)
        {
          strcat(to_return, "\n");
        }
      }
      else
      {
        to_return = o_malloc((o_strlen(line) + 1) * sizeof(char));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      o_free(line);
    }
    return to_return;
  }
  else
  {
    return NULL;
  }
}

int callback_custom_upload_function(const struct _u_request *request,
                                    struct _u_response *response,
                                    void *user_data)
{
  const char *file_data = u_map_get(request->map_post_body, "file_parameter");
  const char *algorithm = u_map_get(request->map_post_body, "algorithm");
  size_t file_size = u_map_get_length(request->map_post_body, "file_parameter");

  void *ctx = malloc(GENERIC_CONTEXT_SIZE);

  if (NULL == ctx)
  {
    return U_OK;
  }

  clock_t start = clock();

  algo_type_t option = (algo_type_t)(atoi(algorithm));

  uint8_t digest[digest_size_list[(uint8_t)option]];

  call_sum_init(option, ctx);
  call_sum_update(option, (void *)ctx, file_data, file_size);
  call_sum_finale(option, (void *)ctx, digest);

  clock_t end = clock();

  char finalisimo[sizeof(digest) * 2 + 1];
  for (int i = 0, j = 0; i < digest_size_list[(uint8_t)option]; i++, j += 2)
    sprintf(finalisimo + j, "%02x", digest[i]);
  finalisimo[sizeof(digest) * 2] = 0;

  pthread_mutex_lock(&mtx);

  float seconds = (float)(end - start) / CLOCKS_PER_SEC;

  FILE *fp;
  fp = fopen(file_name, "a"); // open file to append it

  if (NULL != fp)
  {
    fprintf(fp, "%d;%s,%zu;%f\n", (int)option, finalisimo, file_size, seconds);
    fflush(fp);
    fclose(fp);
  }

  pthread_mutex_unlock(&mtx);

  if (NULL != ctx)
  {
    free(ctx);
  }

  ulfius_set_string_body_response(response, 200, finalisimo);

  return U_CALLBACK_CONTINUE;
}

int callback_checksum_response_function(const struct _u_request *request,
                                        struct _u_response *response,
                                        void *user_data)
{
  const char *file_data = u_map_get(request->map_post_body, "file_parameter");
  const char *algorithm = u_map_get(request->map_post_body, "algorithm");
  const char *checksum = u_map_get(request->map_post_body, "checksum");
  size_t file_size = u_map_get_length(request->map_post_body, "file_parameter");

  void *ctx = malloc(GENERIC_CONTEXT_SIZE);

  if (NULL == ctx)
  {
    return U_OK;
  }

  clock_t start = clock();

  algo_type_t option = (algo_type_t)(atoi(algorithm));

  uint8_t digest[digest_size_list[(uint8_t)option]];

  call_sum_init(option, ctx);
  call_sum_update(option, (void *)ctx, file_data, file_size);
  call_sum_finale(option, (void *)ctx, digest);

  clock_t end = clock();

  char finalisimo[sizeof(digest) * 2 + 1];
  for (int i = 0, j = 0; i < digest_size_list[(uint8_t)option]; i++, j += 2)
    sprintf(finalisimo + j, "%02x", digest[i]);
  finalisimo[sizeof(digest) * 2] = 0;

  pthread_mutex_lock(&mtx);

  float seconds = (float)(end - start) / CLOCKS_PER_SEC;

  FILE *fp;
  fp = fopen(file_name, "a"); // open file to append it

  if (NULL != fp)
  {
    fprintf(fp, "%d;%s,%zu;%f\n", (int)option, finalisimo, file_size, seconds);
    fflush(fp);
    fclose(fp);
  }

  pthread_mutex_unlock(&mtx);

  if (NULL != ctx)
  {
    free(ctx);
  }

  uint raspuns = strcmp(finalisimo, checksum);

  ulfius_set_string_body_response(response, 200, raspuns == 0 ? "OK" : "Not OK");

  return U_CALLBACK_CONTINUE;
}

int main(int argc, char **argv)
{
  struct _u_instance instance;

  y_init_logs("REST SERVER", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL,
              "Starting CHECKSUM REST SERVER");

  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK)
  {
    y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
    return (1);
  }

  instance.max_post_param_size = MAX_FILE_SIZE;
  instance.check_utf8 = 0;

  ulfius_add_endpoint_by_val(&instance, "*", "/checksum", NULL, 0,
                             &callback_custom_upload_function, NULL);

  ulfius_add_endpoint_by_val(&instance, "*", "/verify_checksum", NULL, 0,
                             &callback_checksum_response_function, NULL);

  if (ulfius_start_framework(&instance) == U_OK)
  {
    printf("Start REST SERVER on port %u\n", instance.port);

    getchar();
  }
  else
  {
    printf("Error starting framework\n");
  }

  printf("End framework\n");
  ulfius_stop_framework(&instance);
  ulfius_clean_instance(&instance);

  y_close_logs();

  return 0;
}
