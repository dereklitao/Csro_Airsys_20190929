#ifndef CSRO_DEVICES_H_
#define CSRO_DEVICES_H_

#include "./csro_common/csro_common.h"

//csro_devices.c
void csro_device_init(void);
void csro_device_on_connect(esp_mqtt_event_handle_t event);
void csro_device_on_message(esp_mqtt_event_handle_t event);

//csro_airsys.c
void csro_airsys_init(void);
void csro_airsys_on_connect(esp_mqtt_event_handle_t event);
void csro_airsys_on_message(esp_mqtt_event_handle_t event);

#endif