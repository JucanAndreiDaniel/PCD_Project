/**
 *
 * Ulfius Framework sheep_counter program
 *
 * This program implements a small set of webservices and a tiny static files
 * server
 *
 * As a result, the index.html page will increment a sheep counter and
 * shows a new sheep on the screen every time an increment is done (every 5
 * seconds)
 *
 * Copyright 2015-2022 Nicolas Mora <mail@babelouest.org>
 *
 * License MIT
 *
 */

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
#include <ulfius.h>
#include <unistd.h>

#include "include/http_compression_callback.h"
#include "include/static_compressed_inmemory_website_callback.h"
#include "include/u_example.h"

#define PORT 8080

#define SIZE (1024) // this should be changed
#define GENERIC_CONTEXT_SIZE                                                   \
  (2 * 1024) // 2048 bytes (2KB) should be enought for any context
#define TEST_ALGO (4)

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static char file_name[] = "log.txt";

void call_sum_init(algo_type_t algo, void *CTX) {
  switch (algo) {
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
                     long data_len) {
  switch (algo) {
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

void call_sum_finale(algo_type_t algo, void *CTX, uint8_t *digest) {
  switch (algo) {
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
char *print_map(const struct _u_map *map) {
  char *line, *to_return = NULL;
  const char **keys, *value;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i = 0; keys[i] != NULL; i++) {
      value = u_map_get(map, keys[i]);
      len = snprintf(NULL, 0, "key is %s, length is %zu, value is %.*s",
                     keys[i], u_map_get_length(map, keys[i]),
                     (int)u_map_get_length(map, keys[i]), value);
      line = o_malloc((len + 1) * sizeof(char));
      snprintf(line, (len + 1), "key is %s, length is %zu, value is %.*s",
               keys[i], u_map_get_length(map, keys[i]),
               (int)u_map_get_length(map, keys[i]), value);
      if (to_return != NULL) {
        len = o_strlen(to_return) + o_strlen(line) + 1;
        to_return = o_realloc(to_return, (len + 1) * sizeof(char));
        if (o_strlen(to_return) > 0) {
          strcat(to_return, "\n");
        }
      } else {
        to_return = o_malloc((o_strlen(line) + 1) * sizeof(char));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      o_free(line);
    }
    return to_return;
  } else {
    return NULL;
  }
}

int callback_custom_upload_function(const struct _u_request *request,
                                    struct _u_response *response,
                                    void *user_data) {
  const char *file_data = u_map_get(request->map_post_body, "file_parameter");
  const char *algorithm = u_map_get(request->map_post_body, "algorithm");
  size_t file_size = u_map_get_length(request->map_post_body, "file_parameter");

  void *ctx = malloc(GENERIC_CONTEXT_SIZE);

  if (NULL == ctx) {
    return U_OK;
  }

  algo_type_t option = (algo_type_t)(atoi(algorithm));
  call_sum_init(option, ctx);
  call_sum_update(option, (void *)ctx, file_data, file_size);
  uint8_t digest[digest_size_list[(uint8_t)option]];
  call_sum_finale(option, (void *)ctx, digest);
  pthread_mutex_lock(&mtx);
  FILE *fp;
  fp = fopen(file_name, "a"); // open file to append it

  if (NULL != fp) {
    fprintf(fp, "%d;", (int)option);

    fflush(fp);

    for (int i = 0; i < digest_size_list[(uint8_t)option]; i++) {
      fprintf(fp, "%02x", digest[i]);

      fflush(fp);
    }

    fprintf(fp, ";%zu\n", file_size);

    fflush(fp);

    fclose(fp);
  }

  pthread_mutex_unlock(&mtx);
  if (NULL != ctx) {
    free(ctx);
  }
  char finalisimo[sizeof(digest) * 2 + 1];
  for (int i = 0, j = 0; i < digest_size_list[(uint8_t)option]; i++, j += 2)
    sprintf(finalisimo + j, "%02x", digest[i]);
  finalisimo[sizeof(digest) * 2] = 0;

  ulfius_set_string_body_response(response, 200, finalisimo);

  return U_CALLBACK_CONTINUE;
}

/**
 * File upload callback function
 */
// static int file_upload_callback (const struct _u_request * request,
//                           const char * key,
//                           const char * filename,
//                           const char * content_type,
//                           const char * transfer_encoding,
//                           const char * data,
//                           uint64_t off,
//                           size_t size,
//                           void * cls) {
//   y_log_message(Y_LOG_LEVEL_DEBUG, "Got from file '%s' of the key '%s',
//   offset %llu, size %zu, cls is '%s'", filename, key, off, size, cls);

//   return U_OK;
// }

/**
 * Main function
 */
int main(int argc, char **argv) {

  struct _u_compressed_inmemory_website_config file_config;
  json_int_t nb_sheep = 0;

  // Initialize the instance
  struct _u_instance instance;

  y_init_logs("sheep_counter", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL,
              "Starting sheep_counter");

  if (u_init_compressed_inmemory_website_config(&file_config) == U_OK) {
    u_map_put(&file_config.mime_types, ".html", "text/html");
    u_map_put(&file_config.mime_types, ".css", "text/css");
    u_map_put(&file_config.mime_types, ".js", "application/javascript");
    u_map_put(&file_config.mime_types, ".png", "image/png");
    u_map_put(&file_config.mime_types, ".jpg", "image/jpeg");
    u_map_put(&file_config.mime_types, ".jpeg", "image/jpeg");
    u_map_put(&file_config.mime_types, ".ttf", "font/ttf");
    u_map_put(&file_config.mime_types, ".woff", "font/woff");
    u_map_put(&file_config.mime_types, ".woff2", "font/woff2");
    u_map_put(&file_config.mime_types, ".map", "application/octet-stream");
    u_map_put(&file_config.mime_types, ".json", "application/json");
    u_map_put(&file_config.mime_types, "*", "application/octet-stream");
    file_config.files_path = "static";
    // file_config.url_prefix = FILE_PREFIX;

    if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
      y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
      return (1);
    }

    // Max post param size is 16 Kb, which means an uploaded file is no more
    // than 16 Kb
    instance.max_post_param_size = 16 * 1024;
    instance.check_utf8 = 0;

    // if (ulfius_set_upload_file_callback_function(&instance,
    // &file_upload_callback, "my cls") != U_OK) {
    //   y_log_message(Y_LOG_LEVEL_ERROR, "Error
    //   ulfius_set_upload_file_callback_function");
    // }

    ulfius_add_endpoint_by_val(&instance, "*", "/upload", NULL, 0,
                               &callback_custom_upload_function, NULL);

    // Start the framework
    if (ulfius_start_framework(&instance) == U_OK) {
      printf("Start sheep counter on port %u\n", instance.port);

      // Wait for the user to press <enter> on the console to quit the
      // application
      getchar();
    } else {
      printf("Error starting framework\n");
    }

    printf("End framework\n");
    ulfius_stop_framework(&instance);
    ulfius_clean_instance(&instance);
    u_clean_compressed_inmemory_website_config(&file_config);
  }

  y_close_logs();

  return 0;
}
