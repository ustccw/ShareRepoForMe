/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "lightbulb.h"
static const char *TAG = "lightbulb_main";

static void lightbulb_example_task(void *pvParameter){
    printf("Start Lightbulb task!\n");
    lightbulb_init();

    bulb_state_t* esp_bulb_current_state = NULL;
    while(1){
    	esp_bulb_current_state = get_current_bulb_state();
    	ESP_LOGI(TAG,"Get state!!!set on state:%d flash interval:%d H:%f S:%f B:%d",\
    			esp_bulb_current_state->set_on,esp_bulb_current_state->flash_interval,\
				esp_bulb_current_state->hue_value,esp_bulb_current_state->saturation_value,esp_bulb_current_state->brightness_value);
		lightbulb_set_hue(&(esp_bulb_current_state->hue_value));
		lightbulb_set_saturation(&(esp_bulb_current_state->saturation_value));
		lightbulb_set_brightness(&(esp_bulb_current_state->brightness_value));
		lightbulb_set_on(&(esp_bulb_current_state->set_on));
		vTaskDelay(10);
		ESP_LOGI(TAG,"Delay 1000 ticks!");

		if(esp_bulb_current_state->flash_interval != 0){
			lightbulb_set_off();
			vTaskDelay(esp_bulb_current_state->flash_interval);
		}
		ESP_LOGI(TAG,"Delay interval*1000 ticks!");
    }
    (void)vTaskDelete(NULL);
}

void app_main()
{
    printf("Hello Lightbulb!\n");
    xTaskCreate(&lightbulb_example_task, "lightbulb_example_task", 8192, NULL, 5, NULL);
//    notify_lightbulb_state(BULB_STATE_RED,50);
//    //ESP_LOGI(TAG,"red flash 1s!");
//    vTaskDelay(1000);
//    notify_lightbulb_state(BULB_STATE_GREEN,30);
//    vTaskDelay(1000);
//    notify_lightbulb_state(BULB_STATE_BLUE,10);
//    vTaskDelay(1000);
//    notify_lightbulb_state(BULB_STATE_RED,0);
//    vTaskDelay(1000);
    while(1){
    	notify_lightbulb_state(BULB_STATE_GREEN,0);
	vTaskDelay(50);
		notify_lightbulb_state(BULB_STATE_BLUE,0);
	vTaskDelay(50);
		notify_lightbulb_state(BULB_STATE_RED,0);
	vTaskDelay(50);
		notify_lightbulb_state(BULB_STATE_OTHERS,0);
	vTaskDelay(50);
    	//vTaskDelay(1000);
    }


    ESP_LOGI(TAG,"task over!");

}
