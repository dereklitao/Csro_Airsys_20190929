#include "csro_devices.h"

void csro_device_init(void)
{
#ifdef AIRSYS
    csro_airsys_init();
#endif
}

void csro_device_on_connect(esp_mqtt_event_handle_t event)
{
#ifdef AIRSYS
    csro_airsys_on_connect(event);
#endif
}

void csro_device_on_message(esp_mqtt_event_handle_t event)
{
#ifdef AIRSYS
    csro_airsys_on_message(event);
#endif
}