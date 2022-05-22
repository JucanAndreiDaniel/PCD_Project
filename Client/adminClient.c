#include <stdio.h>
#include <curl/curl.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <path> <algorithm/path>\n", argv[0]);
        return 1;
    }

    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();
    if (curl)
    {
        if (argv[1] == "algorithm")
        {
            curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8080/algorithm");
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "algorithm=6");
        }
        else if (argv[1] == "sensitiveFile")
        {
            curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8080/sensitiveFile");
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "sensitiveFile=6");
        }
        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));

        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    return 0;
}