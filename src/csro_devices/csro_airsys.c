#include "csro_devices.h"
#include "csro_modbus/mb_config.h"

#ifdef AIRSYS

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
void csro_airsys_on_connect(esp_mqtt_event_handle_t event)
{
}
void csro_airsys_on_message(esp_mqtt_event_handle_t event)
{
}

#endif