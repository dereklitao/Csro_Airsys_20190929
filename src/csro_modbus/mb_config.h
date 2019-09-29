#ifndef __MB_CONFIG_H
#define __MB_CONFIG_H

#include "csro_common/csro_common.h"
#include "mb_address.h"

//function code
#define MODBUS_FC_READ_COILS 0x01
#define MODBUS_FC_READ_DISCRETE_INPUTS 0x02
#define MODBUS_FC_READ_HOLDING_REGISTERS 0x03
#define MODBUS_FC_READ_INPUT_REGISTERS 0x04
#define MODBUS_FC_WRITE_SINGLE_COIL 0x05
#define MODBUS_FC_WRITE_SINGLE_REGISTER 0x06
#define MODBUS_FC_READ_EXCEPTION_STATUS 0x07
#define MODBUS_FC_WRITE_MULTIPLE_COILS 0x0F
#define MODBUS_FC_WRITE_MULTIPLE_REGISTERS 0x10
#define MODBUS_FC_REPORT_SLAVE_ID 0x11
#define MODBUS_FC_MASK_WRITE_REGISTER 0x16
#define MODBUS_FC_WRITE_AND_READ_REGISTERS 0x17

//buffer and timeout
#define MODBUS_BUFFER_LENGTH 1024
#define MODBUS_TIMEOUT 800

//register length
#define MODBUS_DISC_MAX 256
#define MODBUS_COIL_MAX 256
#define MODBUS_INPUT_MAX 256
#define MODBUS_HOLDING_MAX 256

typedef struct
{
    uint8_t discs[MODBUS_DISC_MAX];
    uint8_t coils[MODBUS_COIL_MAX];
    int16_t inputs[MODBUS_INPUT_MAX];
    int16_t holdings[MODBUS_HOLDING_MAX];
} modbus_regs;

typedef struct
{
    uint8_t uart_num;
    uint8_t slave_id;
    uint8_t func_code;

    uint8_t crc_hi;
    uint8_t crc_lo;
    uint16_t buff_len;
    uint16_t index;

    uint16_t read_addr;
    uint16_t read_qty;
    uint16_t write_addr;
    uint16_t write_qty;
    uint16_t i;

    uint8_t rx_buf[MODBUS_BUFFER_LENGTH];
    uint16_t rx_len;
    uint8_t tx_buf[MODBUS_BUFFER_LENGTH];
    uint16_t tx_len;
    uint8_t status;

    bool (*master_send_receive)(uint16_t timeout);
    SemaphoreHandle_t reply_sem;
    SemaphoreHandle_t uart_mutex;
} modbus_master;

typedef struct
{
    uint8_t uart_num;
    uint8_t slave_id;
    uint8_t func_code;

    uint8_t crc_hi;
    uint8_t crc_lo;
    uint16_t buff_len;
    uint16_t index;

    uint16_t read_addr;
    uint16_t read_qty;
    uint16_t write_addr;
    uint16_t write_qty;

    uint16_t byte_count;
    uint8_t data;
    uint16_t i;

    uint8_t rx_buf[MODBUS_BUFFER_LENGTH];
    uint16_t rx_len;
    uint8_t tx_buf[MODBUS_BUFFER_LENGTH];
    uint16_t tx_len;

    modbus_regs *regs;
    void (*slave_reply)(void);
    void (*slave_write_coils_callback)(void);
    void (*slave_write_holdings_callback)(void);
    SemaphoreHandle_t command_sem;
} modbus_slave;

extern modbus_master master_ac;
extern modbus_master master_ap;
extern modbus_slave slave_hmi;
extern modbus_regs sys_regs;

//mb_crc.c
uint16_t master_crc16(modbus_master *master, uint8_t *buffer);
uint16_t slave_crc16(modbus_slave *slave, uint8_t *buffer);

//mb_master.c
uint8_t master_read_discs(modbus_master *master, uint8_t *result);
uint8_t master_read_coils(modbus_master *master, uint8_t *result);
uint8_t master_read_input_regs(modbus_master *master, int16_t *result);
uint8_t master_read_holding_regs(modbus_master *master, int16_t *result);
uint8_t master_write_single_coil(modbus_master *master, uint8_t *value);
uint8_t master_write_single_holding_reg(modbus_master *master, int16_t *value);
uint8_t master_write_multi_holding_regs(modbus_master *master, int16_t *value);

//mb_slave.c
void slave_handle_cmd(modbus_slave *slave);

//mb_master_ac.c
void csro_master_ac_init(uint8_t uart_num);
void csro_master_ac_task(void);

//mb_master_ap.c
void csro_master_ap_init(uint8_t uart_num);
void csro_master_ap_task(void);

//mb_slave_hmi.c
void csro_slave_hmi_init(uint8_t uart_num);
void csro_slave_hmi_wait_cmd(void);

#endif
