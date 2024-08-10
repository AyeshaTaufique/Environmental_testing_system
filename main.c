#include <stdlib.h>
#include <stdio.h>
#include <curl/curl.h>
#include "cjson/cJSON.h"
#include <string.h>
#include "email_sender.h"
#include<time.h>
#include<unistd.h>

// Data structure to store daily averages
struct DailyAverages {
    double temperature;
    double humidity;
    double wind_kph;
    double wind_mph;
    double feels_like;
    int count;  // Number of data points for the day
};

// Function to initialize daily averages
void initDailyAverages(struct DailyAverages *daily) {
    daily->temperature = 0.0;
    daily->humidity = 0.0;
    daily->wind_kph = 0.0;
    daily->wind_mph = 0.0;
    daily->feels_like = 0.0;
    daily->count = 0;
}

// Function to update daily averages
void updateDailyAverages(struct DailyAverages *daily, double temp, double hum, double wind_kph, double wind_mph, double feels_like) {
    daily->temperature += temp;
    daily->humidity += hum;
    daily->wind_kph += wind_kph;
    daily->wind_mph += wind_mph;
    daily->feels_like += feels_like;
    daily->count++;
}

// Function to calculate and print daily averages
void printDailyAverages(const struct DailyAverages *daily, const char *date) {
    if (daily->count > 0) {
        printf("Date: %s\n", date);
        printf("Average Temperature: %.2fC\n", daily->temperature / daily->count);
        printf("Average Humidity: %.2f\n", daily->humidity / daily->count);
        printf("Average Wind_kph: %.2f\n", daily->wind_kph / daily->count);
        printf("Average Wind_mph: %.2f\n", daily->wind_mph / daily->count);
        printf("Average Feels Like: %.2fC\n", daily->feels_like / daily->count);
        printf("\n");
        FILE *finalize = fopen("Report_File.txt", "ab");
    	if (!finalize) {
        	fprintf(stderr, "Failed to open file for writing.\n");
        	return;
    }
    	fprintf(finalize,"Date: %s\n", date);
        fprintf(finalize,"Average Temperature: %.2fC\n", daily->temperature / daily->count);
        fprintf(finalize,"Average Humidity: %.2f\n", daily->humidity / daily->count);
        fprintf(finalize,"Average Wind_kph: %.2f\n", daily->wind_kph / daily->count);
        fprintf(finalize,"Average Wind_mph: %.2f\n", daily->wind_mph / daily->count);
        fprintf(finalize,"Average Feels Like: %.2fC\n", daily->feels_like / daily->count);
        fprintf(finalize,"\n");
         

}}
//   current report generation
int current_report(){
    const char *input_file = "api_response.txt";
    const char *output_file = "Report_File.txt";

    // Read the entire content of the input file
    FILE *file = fopen(input_file, "r");
    if (!file) {
        fprintf(stderr, "Error opening file %s\n", input_file);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *json_content = (char *)malloc(file_size + 1);
    fread(json_content, 1, file_size, file);
    fclose(file);

    json_content[file_size] = '\0';  // Null-terminate the string

    // Parse the JSON content
    cJSON *root = cJSON_Parse(json_content);
    free(json_content);

    if (!root) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        return 1;
    }

    // Get the current date
    time_t raw_time;
    struct tm *time_info;
    time(&raw_time);
    time_info = localtime(&raw_time);
    char current_date[11];
    strftime(current_date, sizeof(current_date), "%Y-%m-%d", time_info);

    // Filter data for the current date
    cJSON *forecast = cJSON_GetObjectItemCaseSensitive(root, "forecast");
    cJSON *forecastday = cJSON_GetObjectItemCaseSensitive(forecast, "forecastday");
    cJSON *filtered_data = cJSON_CreateArray();

    cJSON_ArrayForEach(forecast, forecastday) {
        cJSON *date_value = cJSON_GetObjectItemCaseSensitive(forecast, "date");

        if (date_value && cJSON_IsString(date_value) && strstr(date_value->valuestring, current_date) != NULL) {
            cJSON *hour = cJSON_GetObjectItemCaseSensitive(forecast, "hour");
            cJSON_ArrayForEach(hour, cJSON_GetObjectItemCaseSensitive(forecast, "hour")) {
                cJSON_AddItemToArray(filtered_data, cJSON_Duplicate(hour, 1));
            }
        }
    }

    // Calculate average temp_c, humidity, and feelslike_c
    double avg_temp_c = 0.0;
    double avg_humidity = 0.0;
    double avg_feelslike_c = 0.0;
    int num_entries = cJSON_GetArraySize(filtered_data);

    for (int i = 0; i < num_entries; ++i) {
        cJSON *entry = cJSON_GetArrayItem(filtered_data, i);
        cJSON *temp_c_value = cJSON_GetObjectItemCaseSensitive(entry, "temp_c");
        cJSON *humidity_value = cJSON_GetObjectItemCaseSensitive(entry, "humidity");
        cJSON *feelslike_c_value = cJSON_GetObjectItemCaseSensitive(entry, "feelslike_c");

        if (cJSON_IsNumber(temp_c_value) && cJSON_IsNumber(humidity_value) && cJSON_IsNumber(feelslike_c_value)) {
            avg_temp_c += temp_c_value->valuedouble;
            avg_humidity += humidity_value->valuedouble;
            avg_feelslike_c += feelslike_c_value->valuedouble;
        }
    }

    if (num_entries > 0) {
        avg_temp_c /= num_entries;
        avg_humidity /= num_entries;
        avg_feelslike_c /= num_entries;
    }

    // Save the average data into a new file
    file = fopen(output_file, "w");
    if (!file) {
        fprintf(stderr, "Error opening file %s for writing\n", output_file);
        cJSON_Delete(root);
        cJSON_Delete(filtered_data);
        return 1;
    }

    fprintf(file, "Average temp_c: %.2f\n", avg_temp_c);
    fprintf(file, "Average humidity: %.2f\n", avg_humidity);
    fprintf(file, "Average feelslike_c: %.2f\n", avg_feelslike_c);
    fprintf(file, "\nWeather Analysis Report\n");
    fprintf(file, "------------------------\n");

    fprintf(file, "Temperature: %.2f degrees Celsius\n", avg_temp_c);
    fprintf(file, "Humidity: %f%%\n", avg_humidity);
  
    fprintf(file, "Feels like: %.2f C\n", avg_feelslike_c);

    if (avg_temp_c> 25.0) {
        fprintf(file, "   - It feels quite warm with a high temperature.\n");
    } else if (avg_temp_c< 10.0) {
        fprintf(file, "   - It feels cold with a low temperature.\n");
    } else {
        fprintf(file, "   - The temperature is within a comfortable range.\n");
    }

    if (avg_humidity> 80) {
        fprintf(file, "   - It feels humid, consider staying hydrated.\n");
    } else if (avg_humidity < 30) {
        fprintf(file, "   - It feels dry, consider moisturizing.\n");
    } else {
        fprintf(file, "   - Humidity is within a comfortable range.\n");
    }

    

    if (avg_feelslike_c > 14.0 || avg_feelslike_c< 30.0) {
        fprintf(file, "   - feels good.\n");
    } else {
        fprintf(file, "   - its not like normal.\n");
    }

    fprintf(file, "Analysis Report Complete\n");
   
 fclose(file);

    printf("Average data for %s has been saved to %s.\n", current_date, output_file);

    // Clean up
    cJSON_Delete(root);
    cJSON_Delete(filtered_data);
    return 0;
  }

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

void parse_hourly_data(cJSON *hourArray, const char *city_name) {
    
    cJSON *hour = NULL;
    struct DailyAverages daily;
    char current_date[11] = "";  // Format: YYYY-MM-DD

    initDailyAverages(&daily);

    cJSON_ArrayForEach(hour, hourArray) {
        const char *time = cJSON_GetObjectItem(hour, "time")->valuestring;
        const char *date = time ? time : "N/A";
        if (strncmp(date, current_date, 10) != 0) {
            // Print previous day's averages
            printDailyAverages(&daily, current_date);
            
            // Start calculating averages for the new day
            initDailyAverages(&daily);
            strncpy(current_date, date, 10);
        }

        double temp_c = cJSON_GetObjectItem(hour, "temp_c")->valuedouble;
        double humidity = cJSON_GetObjectItem(hour, "humidity")->valuedouble;
        double wind_mph = cJSON_GetObjectItem(hour, "wind_mph")->valuedouble;
        double wind_kph = cJSON_GetObjectItem(hour, "wind_kph")->valuedouble;
        double feelslike_c = cJSON_GetObjectItem(hour, "feelslike_c")->valuedouble;
           
           
           
         // comparing and saving in report file
        

        // Update daily averages
        updateDailyAverages(&daily, temp_c, humidity, wind_kph, wind_mph, feelslike_c);

        // Save formatted data into a file to my_data2.txt
        char file_name[256];
    	snprintf(file_name, sizeof(file_name), "my_data2.txt");
    	FILE *forg = fopen(file_name, "ab");
    	if (!forg) {
        	fprintf(stderr, "Failed to open file for writing.\n");
        	return;
    }

        fprintf(forg, "Time: %s\n Temperature (C): %.2fC\n Humidity: %.2f\n Wind_mph: %.3f\n Wind_kph: %.3f\n Feels Like: %.2fC\n\n", time, temp_c, humidity, wind_mph, wind_kph, feelslike_c);
        fclose(forg);
    }

    // Print averages for the last day
     FILE *finalize = fopen("Report_File.txt", "wb");
    	if (!finalize) {
        	fprintf(stderr, "Failed to open file for writing.\n");
        	return;
    }
    printDailyAverages(&daily, current_date);
fclose(finalize);
   
}

void parse_forecast_data(cJSON *forecast, const char *city_name) {
    cJSON *forecastday = cJSON_GetObjectItem(forecast, "forecastday");
    cJSON *day = NULL;
    cJSON_ArrayForEach(day, forecastday) {
        cJSON *hourArray = cJSON_GetObjectItem(day, "hour");
        if (hourArray) {
            // Parse and print hourly data for each day
            parse_hourly_data(hourArray, city_name);
        }
    }
}




int main() {
    // Ask the user to input the city name
     // Adjust the size based on your needs
    printf("Preparing  weather data for KARACHI: \n");
     char city_name[10]="karachi";
     sleep(3);

    // Ask the user to input the number of days
    int number_of_days;
    printf("Two days average data: \n");
    number_of_days=2;
    sleep(3);

    // Construct the API URL
    char url[256];  // Adjust the size based on your needs
    snprintf(url, sizeof(url), "https://api.weatherapi.com/v1/forecast.json?key=e6ff049c207841f883e205341232412&q=%s&days=%d&aqi=no&alerts=no", city_name, number_of_days);

    CURL *hnd = curl_easy_init();
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(hnd, CURLOPT_URL, url);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "X-RapidAPI-Key: e6ff049c207841f883e205341232412");
    headers = curl_slist_append(headers, "X-RapidAPI-Host: api.weatherapi.com");

    FILE *fp = fopen("api_response.txt", "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open file for writing.\n");
        return 1;
    }
    curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, fp);

    CURLcode ret = curl_easy_perform(hnd);

    // Clean up resources
    curl_slist_free_all(headers);
    curl_easy_cleanup(hnd);
    fclose(fp);

    if (ret != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
        return 1;
    }
    // To reset the file
    char file_name[256];
    snprintf(file_name, sizeof(file_name), "my_data2.txt");
    FILE *forg = fopen(file_name, "wb");
    if (!forg) {
        fprintf(stderr, "Failed to open file for writing.\n");
        return 1;
    }

    // Parse JSON response
    fp = fopen("api_response.txt", "rb");
    if (!fp) {
        fprintf(stderr, "Failed to open file for reading.\n");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *json_data = (char *)malloc(file_size + 1);
    fread(json_data, 1, file_size, fp);
    json_data[file_size] = '\0';

    cJSON *root = cJSON_Parse(json_data);
    if (!root) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }

        // Clean up cJSON and close the file
        cJSON_Delete(root);
        fclose(fp);
        free(json_data);

        return 1;
    }

    cJSON *forecast = cJSON_GetObjectItem(root, "forecast");
    if (!forecast) {
        fprintf(stderr, "Failed to get 'forecast' object.\n");

        // Clean up cJSON and close the file
        cJSON_Delete(root);
        fclose(fp);
        free(json_data);

        return 1;
    }

    // Parse and print the complete forecast data
    parse_forecast_data(forecast, city_name);

    // Clean up cJSON and close the file
    cJSON_Delete(root);
    fclose(fp);
    free(json_data);
   current_report();
   send_email();

    return 0;
}
