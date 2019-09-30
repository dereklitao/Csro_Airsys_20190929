#include "mb_config.h"

modbus_master master_ap;
csro_ap_control ap_ctrl;

static bool master_ap_send_receive(uint16_t timeout)
{
    master_ap.rx_len = 0;
    master_ap.buff_len = master_ap.tx_len;
    master_crc16(&master_ap, master_ap.tx_buf);
    master_ap.tx_buf[master_ap.tx_len++] = master_ap.crc_hi;
    master_ap.tx_buf[master_ap.tx_len++] = master_ap.crc_lo;
    uart_write_bytes(master_ap.uart_num, (const char *)master_ap.tx_buf, master_ap.tx_len);
    if (xSemaphoreTake(master_ap.reply_sem, timeout / portTICK_RATE_MS) == pdTRUE)
    {
        master_ap.status = true;
    }
    else
    {
        master_ap.status = false;
    }
    return master_ap.status;
}

static void master_ap_read_write_task(void *param)
{
    uint8_t disc_result[35];
    int16_t input_result[25];
    int16_t value[5];
    while (true)
    {
        master_ap.read_addr = 0;
        master_ap.read_qty = 20;
        if (master_read_input_regs(&master_ap, input_result) == 1)
        {
            for (uint8_t i = 0; i < 20; i++)
            {
                sys_regs.inputs[I_USER_AC_STATUS_START + i] = input_result[i];
            }
        }
        vTaskDelay(100 / portTICK_RATE_MS);

        master_ap.write_addr = 0;
        master_ap.write_qty = 4;
        value[0] = ap_ctrl.ap_power;
        value[1] = ap_ctrl.ap_mode;
        value[2] = ap_ctrl.ap_temp;
        value[3] = ap_ctrl.ap_fan;
        master_write_multi_holding_regs(&master_ap, value);
        vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void csro_master_ap_init(uint8_t uart_number)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(uart_number, &uart_config);
    uart_set_pin(uart_number, TXD1_PIN, RXD1_PIN, RTS1_PIN, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_number, 2048, 2048, 0, NULL, 0);
    uart_set_mode(uart_number, UART_MODE_RS485_HALF_DUPLEX);

    master_ap.uart_num = uart_number;
    master_ap.slave_id = 1;
    master_ap.master_send_receive = master_ap_send_receive;
    master_ap.reply_sem = xSemaphoreCreateBinary();
    master_ap.uart_mutex = xSemaphoreCreateMutex();

    ap_ctrl.ap_power = 1;
    ap_ctrl.ap_mode = 2;
    ap_ctrl.ap_temp = 220;
    ap_ctrl.ap_fan = 2;

    xTaskCreate(master_ap_read_write_task, "master_ap_read_write_task", 2048, NULL, configMAX_PRIORITIES - 8, NULL);
}