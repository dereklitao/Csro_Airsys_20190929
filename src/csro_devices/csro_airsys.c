#include "csro_devices.h"
#include "csro_modbus/mb_config.h"

#ifdef AIR_SYS

static void uart_receive_one_byte(uart_port_t uart_num, uint8_t data)
{
    if (uart_num == master_ap.uart_num)
    {
        master_ap.rx_buf[(master_ap.rx_len++) % 1024] = data;
    }
    else if (uart_num == master_ac.uart_num)
    {
        master_ac.rx_buf[(master_ac.rx_len++) % 1024] = data;
    }
    else if (uart_num == slave_hmi.uart_num)
    {
        slave_hmi.rx_buf[(slave_hmi.rx_len++) % 1024] = data;
    }
}

static void uart_receive_complete(uart_port_t uart_num)
{
    static portBASE_TYPE HPTaskAwoken = 0;

    if (uart_num == master_ap.uart_num)
    {
        uart_flush(master_ap.uart_num);
        xSemaphoreGiveFromISR(master_ap.reply_sem, &HPTaskAwoken);
    }
    else if (uart_num == master_ac.uart_num)
    {
        uart_flush(master_ac.uart_num);
        xSemaphoreGiveFromISR(master_ac.reply_sem, &HPTaskAwoken);
    }
    else if (uart_num == slave_hmi.uart_num)
    {
        uart_flush(slave_hmi.uart_num);
        xSemaphoreGiveFromISR(slave_hmi.command_sem, &HPTaskAwoken);
    }
}

void csro_airsys_init(void)
{
    uart_handler.receive_one_byte = uart_receive_one_byte;
    uart_handler.receive_complete = uart_receive_complete;
    csro_master_ac_init(UART_NUM_0);
    csro_master_ap_init(UART_NUM_1);
    csro_slave_hmi_init(UART_NUM_2);
}

void csro_update_airsys_state(void)
{
    cJSON *state_json = cJSON_CreateObject();
    cJSON *airsys_json;

    cJSON_AddStringToObject(state_json, "time", sysinfo.time_str);
    cJSON_AddNumberToObject(state_json, "run", (int)(sysinfo.time_now - sysinfo.time_start));
    cJSON_AddItemToObject(state_json, "state", airsys_json = cJSON_CreateObject());
    cJSON_AddStringToObject(airsys_json, "mode", "heat");
    cJSON_AddNumberToObject(airsys_json, "temp", 19.5);
    cJSON_AddStringToObject(airsys_json, "fan", "low");
    char *out = cJSON_PrintUnformatted(state_json);
    strcpy(mqttinfo.content, out);
    free(out);
    cJSON_Delete(state_json);
    sprintf(mqttinfo.pub_topic, "csro/%s/%s/state", sysinfo.mac_str, sysinfo.dev_type);
    esp_mqtt_client_publish(mqttclient, mqttinfo.pub_topic, mqttinfo.content, 0, 0, 1);
}

void csro_airsys_on_connect(esp_mqtt_event_handle_t event)
{
    sprintf(mqttinfo.sub_topic, "csro/%s/%s/set/#", sysinfo.mac_str, sysinfo.dev_type);
    esp_mqtt_client_subscribe(event->client, mqttinfo.sub_topic, 1);

    char prefix[50], name[50];
    char *work_mode[4] = {"cool", "heat", "dry", "fan_only"};
    char *fan_mode[5] = {"auto", "low", "medhium", "high", "super"};
    sprintf(mqttinfo.pub_topic, "csro/climate/%s_%s/config", sysinfo.mac_str, sysinfo.dev_type);
    sprintf(prefix, "csro/%s/%s", sysinfo.mac_str, sysinfo.dev_type);
    sprintf(name, "%s_%s", sysinfo.dev_type, sysinfo.mac_str);
    cJSON *config_json = cJSON_CreateObject();
    cJSON *mode_json = cJSON_CreateStringArray(work_mode, 4);
    cJSON *fan_json = cJSON_CreateStringArray(fan_mode, 5);
    cJSON_AddStringToObject(config_json, "~", prefix);
    cJSON_AddStringToObject(config_json, "name", name);
    cJSON_AddStringToObject(config_json, "avty_t", "~/available");
    cJSON_AddStringToObject(config_json, "pl_avail", "online");
    cJSON_AddStringToObject(config_json, "pl_not_avail", "offline");
    cJSON_AddNumberToObject(config_json, "qos", 0);
    cJSON_AddItemToObject(config_json, "modes", mode_json);
    cJSON_AddItemToObject(config_json, "fan_modes", fan_json);

    cJSON_AddStringToObject(config_json, "mode_stat_t", "~/state");
    cJSON_AddStringToObject(config_json, "mode_stat_tpl", "{{value_json.state.mode}}");
    cJSON_AddStringToObject(config_json, "mode_cmd_t", "~/set/mode");
    cJSON_AddStringToObject(config_json, "temp_stat_t", "~/state");
    cJSON_AddStringToObject(config_json, "temp_stat_tpl", "{{value_json.state.temp}}");
    cJSON_AddStringToObject(config_json, "temp_cmd_t", "~/set/temp");
    cJSON_AddStringToObject(config_json, "fan_mode_stat_t", "~/state");
    cJSON_AddStringToObject(config_json, "fan_mode_stat_tpl", "{{value_json.state.fan}}");
    cJSON_AddStringToObject(config_json, "fan_mode_cmd_t", "~/set/fan");
    cJSON_AddNumberToObject(config_json, "min_temp", 10);
    cJSON_AddNumberToObject(config_json, "max_temp", 35);
    cJSON_AddNumberToObject(config_json, "temp_step", 0.5);

    char *out = cJSON_PrintUnformatted(config_json);
    strcpy(mqttinfo.content, out);
    free(out);
    cJSON_Delete(config_json);
    esp_mqtt_client_publish(event->client, mqttinfo.pub_topic, mqttinfo.content, 0, 1, 1);

    sprintf(mqttinfo.pub_topic, "csro/%s/%s/available", sysinfo.mac_str, sysinfo.dev_type);
    esp_mqtt_client_publish(event->client, mqttinfo.pub_topic, "online", 0, 1, 1);
    csro_update_airsys_state();
}
void csro_airsys_on_message(esp_mqtt_event_handle_t event)
{
}

#endif