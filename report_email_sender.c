#include "email_sender.h"
#include <stdio.h>
#include <curl/curl.h>
#include<stdlib.h>

size_t read_callback(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t retcode = fread(ptr, size, nmemb, stream);
  
    return retcode;
}


char *readFromFile(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file %s for reading\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *content = malloc(file_size + 1);
    if (content == NULL) {
        fprintf(stderr, "Error: Unable to allocate memory for reading file %s\n", filename);
        fclose(file);
        return NULL;
    }

    if (fread(content, 1, file_size, file) != file_size) {
        fprintf(stderr, "Error: Unable to read content from file %s\n", filename);
        free(content);
        fclose(file);
        return NULL;
    }

    content[file_size] = '\0';

    fclose(file);
    return content;
}


void send_email() {
   CURL *emailCurl = curl_easy_init();

    if (emailCurl) {
        const char *from = "aminashahzadkhan@gmail.com";
        const char *to = "aminashahzadkhan@gmail.com";
        const char *subject = "Weather Report";
        const char *body  = readFromFile("Report_File");
       
       
        curl_easy_setopt(emailCurl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
        curl_easy_setopt(emailCurl, CURLOPT_USERNAME, "aminashahzadkhan@gmail.com");
        curl_easy_setopt(emailCurl, CURLOPT_PASSWORD, "mmtz fjqc owtp unfk");
        curl_easy_setopt(emailCurl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(emailCurl, CURLOPT_MAIL_FROM, from);

        struct curl_slist *recipients = NULL;
        recipients = curl_slist_append(recipients, to);
        curl_easy_setopt(emailCurl, CURLOPT_MAIL_RCPT, recipients);

        curl_easy_setopt(emailCurl, CURLOPT_USERNAME, from);
        curl_easy_setopt(emailCurl, CURLOPT_PASSWORD, "mmtz fjqc owtp unfk");

        curl_easy_setopt(emailCurl, CURLOPT_READFUNCTION, read_callback);
        FILE *file = fopen("Report_File", "r");
        curl_easy_setopt(emailCurl, CURLOPT_READDATA, file);

        curl_easy_setopt(emailCurl, CURLOPT_UPLOAD, 1L);

        CURLcode emailRes = curl_easy_perform(emailCurl);

        if (emailRes != CURLE_OK) {
            fprintf(stderr, "Email sending failed: %s\n", curl_easy_strerror(emailRes));
        }

        curl_slist_free_all(recipients);
        curl_easy_cleanup(emailCurl);
        
    }

}
