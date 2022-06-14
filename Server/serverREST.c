#include <arpa/inet.h>
#include <jansson.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <ulfius.h>
#include <unistd.h>

#include "include/http_compression_callback.h"
#include "include/checksum.h"
#include "include/u_example.h"

#define MAX_FILE_SIZE (1024 * 1024 * 1024)
#define TEST_ALGO (4)

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static char file_name[] = "log.txt";

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

    clock_t start = clock();

    algo_type_t option = (algo_type_t)(atoi(algorithm));

    char *validate = checksum_file(option, file_data, file_size);

    clock_t end = clock();

    float seconds = (float)(end - start) / CLOCKS_PER_SEC;

    pthread_mutex_lock(&mtx);

    writeLog(file_name, option, validate, file_size, seconds);

    pthread_mutex_unlock(&mtx);

    ulfius_set_string_body_response(response, 200, validate);

    if (validate != NULL)
    {
        free(validate);
        validate = NULL;
    }

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

    clock_t start = clock();

    algo_type_t option = (algo_type_t)(atoi(algorithm));

    char *validate = checksum_file(option, file_data, file_size);

    clock_t end = clock();

    float seconds = (float)(end - start) / CLOCKS_PER_SEC;

    pthread_mutex_lock(&mtx);

    writeLog(file_name, option, validate, file_size, seconds);

    pthread_mutex_unlock(&mtx);

    uint raspuns = strcmp(validate, checksum);

    if (validate != NULL)
    {
        free(validate);
        validate = NULL;
    }

    ulfius_set_string_body_response(response, 200,
                                    raspuns == 0 ? "OK" : "Not OK");

    return U_CALLBACK_CONTINUE;
}

void *rest_main(void *args)
{
    struct _u_instance instance;
    int port = *((int *)args);

    y_init_logs("REST SERVER", Y_LOG_MODE_CONSOLE, Y_LOG_LEVEL_DEBUG, NULL,
                "Starting CHECKSUM REST SERVER");

    if (ulfius_init_instance(&instance, port, NULL, NULL) != U_OK)
    {
        y_log_message(Y_LOG_LEVEL_ERROR, "Error ulfius_init_instance, abort");
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
