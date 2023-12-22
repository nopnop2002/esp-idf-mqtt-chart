/*
	Example using WEB Socket.
	This example code is in the Public Domain (or CC0 licensed, at your option.)
	Unless required by applicable law or agreed to in writing, this
	software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/message_buffer.h"
#include "esp_log.h"
#include "cJSON.h"

static const char *TAG = "web_client";

#include "websocket_server.h"
#include "json.h"

extern MessageBufferHandle_t xMessageBufferToClient;

esp_err_t JSON_Analyze(const cJSON * const root, JSON_t **elements, size_t *elementsLength);

void client_task(void* pvParameters) {
	ESP_LOGI(TAG, "Start");

	char cRxBuffer[512];
	char DEL = 0x04;
	char outBuffer[128];

	while (1) {
		size_t readBytes = xMessageBufferReceive(xMessageBufferToClient, cRxBuffer, sizeof(cRxBuffer), portMAX_DELAY );
		ESP_LOGD(TAG, "readBytes=%d", readBytes);
		ESP_LOGI(TAG, "cRxBuffer=[%.*s]", readBytes, cRxBuffer);
		cJSON *root = cJSON_Parse(cRxBuffer);
		if (cJSON_GetObjectItem(root, "id")) {
			char *id = cJSON_GetObjectItem(root,"id")->valuestring;
			ESP_LOGI(TAG, "id=[%s]",id);

			if ( strcmp (id, "init") == 0) {
				sprintf(outBuffer,"HEAD%cMonitoring MQTT topics in %s", DEL, CONFIG_MQTT_SUB_TOPIC);
				ESP_LOGD(TAG, "outBuffer=[%s]", outBuffer);
				ws_server_send_text_all(outBuffer,strlen(outBuffer));

				sprintf(outBuffer,"BROKER%cConnecting to %s", DEL, CONFIG_MQTT_BROKER);
				ESP_LOGD(TAG, "outBuffer=[%s]", outBuffer);
				ws_server_send_text_all(outBuffer,strlen(outBuffer));
			} // end if

			if ( strcmp (id, "mqtt") == 0) {
				JSON_t *elements = NULL;
				size_t elementsLength = 0;
				JSON_Analyze(root, &elements, &elementsLength);
				ESP_LOGI(TAG, "elementsLength=%d", elementsLength);
				char name[3][32];
				double valuedouble[3];
				memset(name, 0, sizeof(name));
				int index = 0;
				for (int i=0;i<elementsLength;i++) {
					if ((elements+i)->type == cJSON_Number) {
						ESP_LOGI(TAG, "element[%d] cJSON_Number [%s]=%lf", i, (elements+i)->name, (elements+i)->valuedouble);
						strcpy(name[index], (elements+i)->name);
						valuedouble[index] = (elements+i)->valuedouble;
						index++;
					}
				}
				free(elements);
				for (int i=0;i<3;i++) {
					ESP_LOGI(TAG, "name[%d] [%s]=%f", i, name[i], valuedouble[i]);
				}
				sprintf(outBuffer,"NAME%c%s%c%s%c%s", DEL,name[0],DEL,name[1],DEL,name[2]);
				ESP_LOGD(TAG, "outBuffer=[%s]", outBuffer);
				ws_server_send_text_all(outBuffer,strlen(outBuffer));

				sprintf(outBuffer,"DATA%c%f%c%f%c%f", DEL,valuedouble[0],DEL,valuedouble[1],DEL,valuedouble[2]);
				ESP_LOGD(TAG, "outBuffer=[%s]", outBuffer);
				ws_server_send_text_all(outBuffer,strlen(outBuffer));
			} // end if

		} // end if

		// Delete a cJSON structure
		cJSON_Delete(root);

	} // end while

	// Never reach here
	vTaskDelete(NULL);
}
