#include "mb_config.h"

modbus_slave slave_hmi;
modbus_regs sys_regs;

static void slave_hmi_reply(void)
{
    slave_hmi.buff_len = slave_hmi.tx_len;
    slave_crc16(&slave_hmi, slave_hmi.tx_buf);
    slave_hmi.tx_buf[slave_hmi.tx_len++] = slave_hmi.crc_hi;
    slave_hmi.tx_buf[slave_hmi.tx_len++] = slave_hmi.crc_lo;
    uart_write_bytes(slave_hmi.uart_num, (const char *)slave_hmi.tx_buf, slave_hmi.tx_len);
}

static void slave_hmi_write_coils_callback(void)
{
    uart_write_bytes(UART_NUM_0, (const char *)"Write Coils\r\n", strlen("Write Coils\r\n"));
}
static void slave_hmi_write_holdings_callback(void)
{
    uart_write_bytes(UART_NUM_0, (const char *)"Write Holdings\r\n", strlen("Write Holdings\r\n"));
}

void csro_slave_hmi_wait_cmd(void *param)
{
    while (true)
    {
        if (xSemaphoreTake(slave_hmi.command_sem, portMAX_DELAY) == pdTRUE)
        {
            slave_handle_cmd(&slave_hmi);
            slave_hmi.rx_len = 0;
        }
    }
}

void csro_slave_hmi_init(uint8_t uart_number)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};
    uart_param_config(uart_number, &uart_config);
    uart_set_pin(uart_number, TXD2_PIN, RXD2_PIN, RTS2_PIN, UART_PIN_NO_CHANGE);
    uart_driver_install(uart_number, 2048, 2048, 0, NULL, 0);
    uart_set_mode(uart_number, UART_MODE_RS485_HALF_DUPLEX);

    slave_hmi.uart_num = uart_number;
    slave_hmi.slave_id = 1;
    slave_hmi.command_sem = xSemaphoreCreateBinary();
    slave_hmi.regs = &sys_regs;
    slave_hmi.slave_write_coils_callback = slave_hmi_write_coils_callback;
    slave_hmi.slave_write_holdings_callback = slave_hmi_write_holdings_callback;
    slave_hmi.slave_reply = slave_hmi_reply;
    xTaskCreate(csro_slave_hmi_wait_cmd, "csro_slave_hmi_wait_cmd", 2048, NULL, configMAX_PRIORITIES - 6, NULL);
}
