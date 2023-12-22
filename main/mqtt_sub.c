/*	MQTT (over TCP) Example

	This example code is in the Public Domain (or CC0 licensed, at your option.)

	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "esp_log.h"
#include "esp_event.h"
#include "lwip/dns.h"
#include "esp_mac.h"
#include "mqtt_client.h"
#include "cJSON.h"

#include "mqtt.h"
#include "json.h"

static const char *TAG = "SUB";

extern MessageBufferHandle_t xMessageBufferToClient;


#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
#else
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
#endif
{
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
	esp_mqtt_event_handle_t event = event_data;
	MQTT_t *mqttBuf = handler_args;
#else
	MQTT_t *mqttBuf = event->user_context; 
#endif
	ESP_LOGI(TAG, "taskHandle=0x%x", (unsigned int)mqttBuf->taskHandle);
	mqttBuf->event_id = event->event_id;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xTaskNotifyGive( mqttBuf->taskHandle );
			break;
		case MQTT_EVENT_DISCONNECTED:
			ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			xTaskNotifyGive( mqttBuf->taskHandle );
			break;
		case MQTT_EVENT_SUBSCRIBED:
			ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_UNSUBSCRIBED:
			ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_PUBLISHED:
			ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
			break;
		case MQTT_EVENT_DATA:
			ESP_LOGI(TAG, "MQTT_EVENT_DATA");
			ESP_LOGI(TAG, "TOPIC=[%.*s] DATA=[%.*s]\r", event->topic_len, event->topic, event->data_len, event->data);

			mqttBuf->topic_len = event->topic_len;
			for(int i=0;i<event->topic_len;i++) {
				mqttBuf->topic[i] = event->topic[i];
				mqttBuf->topic[i+1] = 0;
			}
			mqttBuf->data_len = event->data_len;
			for(int i=0;i<event->data_len;i++) {
				mqttBuf->data[i] = event->data[i];
				mqttBuf->data[i+1] = 0;
			}
			xTaskNotifyGive( mqttBuf->taskHandle );
			break;
		case MQTT_EVENT_ERROR:
			ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
			xTaskNotifyGive( mqttBuf->taskHandle );
			break;
		default:
			ESP_LOGI(TAG, "Other event id:%d", event->event_id);
			break;
	}
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
	return ESP_OK;
#endif
}

bool isFloat(char * data, float *value) {
	int index = 0;
	if (data[index] == '+') {
		index++;
	}
	if ( data[index] == '-') {
		index++;
	}
	for (; index<strlen(data); index++) {
		//printf("index=%d\n",index);
		if (data[index] >= 48 && data[index] <= 57) continue;
		if (data[index] == 46) continue;
		return false;
	}
	*value = strtof(data, NULL);
	return true;
}

esp_err_t query_mdns_host(const char * host_name, char *ip);
void convert_mdns_host(char * from, char * to);
esp_err_t JSON_Analyze(const cJSON * const root, JSON_t **elements, size_t *elementsLength);

void mqtt_sub(void *pvParameters)
{
	ESP_LOGI(TAG, "Start");
	ESP_LOGI(TAG, "CONFIG_MQTT_BROKER=[%s]", CONFIG_MQTT_BROKER);

	// Set client id from mac
	uint8_t mac[8];
	ESP_ERROR_CHECK(esp_base_mac_addr_get(mac));
	for(int i=0;i<8;i++) {
		ESP_LOGD(TAG, "mac[%d]=%x", i, mac[i]);
	}
	char client_id[64];
	sprintf(client_id, "esp32-%02x%02x%02x%02x%02x%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	ESP_LOGI(TAG, "client_id=[%s]", client_id);

	// Resolve mDNS host name
	char ip[128];
	ESP_LOGI(TAG, "CONFIG_MQTT_BROKER=[%s]", CONFIG_MQTT_BROKER);
	convert_mdns_host(CONFIG_MQTT_BROKER, ip);
	ESP_LOGI(TAG, "ip=[%s]", ip);
	char uri[138];
	sprintf(uri, "mqtt://%s", ip);
	ESP_LOGI(TAG, "uri=[%s]", uri);

	// Initialize user context
	MQTT_t mqttBuf;
	mqttBuf.taskHandle = xTaskGetCurrentTaskHandle();
	ESP_LOGI(TAG, "taskHandle=0x%x", (unsigned int)mqttBuf.taskHandle);
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
	esp_mqtt_client_config_t mqtt_cfg = {
		.broker.address.uri = uri,
		.broker.address.port = 1883,
#if CONFIG_BROKER_AUTHENTICATION
		.credentials.username = CONFIG_AUTHENTICATION_USERNAME,
		.credentials.authentication.password = CONFIG_AUTHENTICATION_PASSWORD,
#endif
		.credentials.client_id = client_id
	};
#else
	esp_mqtt_client_config_t mqtt_cfg = {
		.user_context = &mqttBuf,
		.uri = uri,
		.port = 1883,
		.event_handle = mqtt_event_handler,
#if CONFIG_BROKER_AUTHENTICATION
		.username = CONFIG_AUTHENTICATION_USERNAME,
		.password = CONFIG_AUTHENTICATION_PASSWORD,
#endif
		.client_id = client_id
	};
#endif

	esp_mqtt_client_handle_t mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
	esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, &mqttBuf);
#endif

	esp_mqtt_client_start(mqtt_client);

	while (1) {
		ulTaskNotifyTake( pdTRUE, portMAX_DELAY );
		ESP_LOGI(TAG, "ulTaskNotifyTake");
		ESP_LOGI(TAG, "event_id=%"PRIi32, mqttBuf.event_id);

		if (mqttBuf.event_id == MQTT_EVENT_CONNECTED) {
			esp_mqtt_client_subscribe(mqtt_client, CONFIG_MQTT_SUB_TOPIC, 0);
			ESP_LOGI(TAG, "Subscribe to MQTT Server");
		} else if (mqttBuf.event_id == MQTT_EVENT_DISCONNECTED) {
			break;
		} else if (mqttBuf.event_id == MQTT_EVENT_DATA) {
			ESP_LOGI(TAG, "TOPIC=[%.*s]\r", mqttBuf.topic_len, mqttBuf.topic);
			ESP_LOGI(TAG, "DATA=[%.*s]\r", mqttBuf.data_len, mqttBuf.data);
			mqttBuf.topic[mqttBuf.topic_len] = 0;
			mqttBuf.data[mqttBuf.data_len] = 0;

			// /topic/test --> test
			char bottom_topic[64];
			strcpy(bottom_topic, basename(mqttBuf.topic));
			ESP_LOGI(TAG, "bottom_topic=[%s]", bottom_topic);

			// is mqtt payload numeric
			float floatValue;
			bool ret = isFloat(mqttBuf.data, &floatValue);
			ESP_LOGI(TAG, "isFloat=%d floatValue=%f", ret, floatValue);
			// mqtt payload is numeric
			if (ret) {
				cJSON *root;
				root = cJSON_CreateObject();
				cJSON_AddStringToObject(root, "id", "mqtt");
				cJSON_AddNumberToObject(root, bottom_topic, floatValue);
/*
I (37814) SUB: my_json_string
{
        "id":   "mqtt",
        "test": 4
}
*/
				//char *my_json_string = cJSON_Print(root);
				char *my_json_string = cJSON_PrintUnformatted(root);
				ESP_LOGD(TAG, "my_json_string\n%s",my_json_string);
				xMessageBufferSend(xMessageBufferToClient, my_json_string, strlen(my_json_string), portMAX_DELAY);
				cJSON_Delete(root);
				cJSON_free(my_json_string);

			// mqtt payload is json
			} else {
				// Parse the payload
				cJSON *root = cJSON_Parse(mqttBuf.data);
				if (root == NULL) {
					ESP_LOGW(TAG, "Illegal json format");
				} else {
					// Check validity of payload
					JSON_t *elements = NULL;
					size_t elementsLength = 0;
					JSON_Analyze(root, &elements, &elementsLength);
					int objects = 0;
					bool payloadCheck = true;
					ESP_LOGI(TAG, "elementsLength=%d", elementsLength);
					for (int i=0;i<elementsLength;i++) {
						if ((elements+i)->type == cJSON_Invalid) {
							ESP_LOGE(TAG, "element[%d] cJSON_Invalid", i);
							payloadCheck = false;
						} else if ((elements+i)->type == cJSON_False) {
							ESP_LOGE(TAG, "element[%d] cJSON_False", i);
							payloadCheck = false;
						} else if ((elements+i)->type == cJSON_True) {
							ESP_LOGE(TAG, "element[%d] cJSON_True", i);
							payloadCheck = false;
						} else if ((elements+i)->type == cJSON_NULL) {
							ESP_LOGE(TAG, "element[%d] cJSON_NULL", i);
							payloadCheck = false;
						} else if ((elements+i)->type == cJSON_Number) {
							ESP_LOGI(TAG, "element[%d] cJSON_Number [%s]=%lf", i, (elements+i)->name, (elements+i)->valuedouble);
							objects++;
							if (objects > 3) payloadCheck = false;
						} else if ((elements+i)->type == cJSON_String) {
							ESP_LOGE(TAG, "element[%d] cJSON_String", i);
							ESP_LOGE(TAG, " [%s]", (elements+i)->valuestring);
							payloadCheck = false;
						} else if ((elements+i)->type == cJSON_Array) {
							ESP_LOGE(TAG, "element[%d] cJSON_Array", i);
							payloadCheck = false;
						} else if ((elements+i)->type == cJSON_Object) {
							ESP_LOGE(TAG, "element[%d] cJSON_Object", i);
							payloadCheck = false;
						}
					}
					free(elements);

					ESP_LOGI(TAG, "payloadCheck=%d", payloadCheck);
					if (payloadCheck == true) {
						cJSON_AddStringToObject(root, "id", "mqtt");
/*
{
        "sin":  0.34202,
        "cos":  0.93969,
        "tan":  0.36397,
        "id":   "mqtt"
}
*/
						//char *my_json_string = cJSON_Print(root);
						char *my_json_string = cJSON_PrintUnformatted(root);
						ESP_LOGD(TAG, "my_json_string\n%s",my_json_string);
						xMessageBufferSend(xMessageBufferToClient, my_json_string, strlen(my_json_string), portMAX_DELAY);
						cJSON_free(my_json_string);
					}
					cJSON_Delete(root);
				}
			}

		} else if (mqttBuf.event_id == MQTT_EVENT_ERROR) {
			break;
		}
	} // end while

	ESP_LOGI(TAG, "Task Delete");
	esp_mqtt_client_stop(mqtt_client);
	vTaskDelete(NULL);

}
