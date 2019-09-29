#include "mb_config.h"

#define TXD0_PIN (GPIO_NUM_15)
#define RXD0_PIN (GPIO_NUM_2)
#define RTS0_PIN (GPIO_NUM_0)
#define TXD1_PIN (GPIO_NUM_4)
#define RXD1_PIN (GPIO_NUM_17)
#define RTS1_PIN (GPIO_NUM_16)
#define TXD2_PIN (GPIO_NUM_5)
#define RXD2_PIN (GPIO_NUM_19)
#define RTS2_PIN (GPIO_NUM_18)

modbus_master master_ac;

static bool master_ac_send_receive(uint16_t timeout)
{
    master_ac.rx_len = 0;
    uart_write_bytes(master_ac.uart_num, (const char *)master_ac.tx_buf, master_ac.tx_len);
    if (xSemaphoreTake(master_ac.reply_sem, timeout / portTICK_RATE_MS) == pdTRUE)
    {
        master_ac.status = true;
    }
    else
    {
        master_ac.status = false;
    }
    return master_ac.status;
}

static void master_ac_mdobus_task(void *param)
{
    while (true)
    {
        uart_write_bytes(UART_NUM_2, "444\r\n", strlen("444\r\n"));
        if (xSemaphoreTake(master_ac.reply_sem, portMAX_DELAY) == pdTRUE)
        {
            uart_write_bytes(master_ac.uart_num, "received complete\r\n", strlen("received complete\r\n"));
            master_ac.rx_len = 0;
        }
        uart_write_bytes(UART_NUM_2, "555\r\n", strlen("555\r\n"));
    }
    vTaskDelete(NULL);
}

void csro_master_ac_init(uint8_t number)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(number, &uart_config);
    uart_set_pin(number, TXD2_PIN, RXD2_PIN, RTS2_PIN, UART_PIN_NO_CHANGE);
    uart_driver_install(number, 2048, 2048, 0, NULL, 0);
    uart_set_mode(number, UART_MODE_RS485_HALF_DUPLEX);

    uart_write_bytes(number, "UART2 START\r\n", strlen("UART2 START\r\n"));

    master_ac.uart_num = number;
    uart_write_bytes(number, "111\r\n", strlen("111\r\n"));
    master_ac.slave_id = 1;
    // master_ac.master_send_receive = master_ac_send_receive;
    master_ac.reply_sem = xSemaphoreCreateBinary();
    master_ac.uart_mutex = xSemaphoreCreateMutex();
    uart_write_bytes(number, "222\r\n", strlen("222\r\n"));
    xTaskCreate(master_ac_mdobus_task, "master_ac_mdobus_task", 4096, NULL, configMAX_PRIORITIES - 7, NULL);
    uart_write_bytes(number, "333\r\n", strlen("333\r\n"));
}
