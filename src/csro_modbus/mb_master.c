#include "mb_config.h"

static uint8_t master_validate_reply(modbus_master *master)
{
    if (master->rx_len < 5)
    {
        return 0;
    }
    master->buff_len = master->rx_len - 2;
    master_crc16(master, master->rx_buf);
    if ((master->crc_hi != master->rx_buf[master->rx_len - 2]) || (master->crc_lo != master->rx_buf[master->rx_len - 1]))
    {
        return 0;
    }
    if ((master->rx_buf[0] != master->slave_id) || (master->rx_buf[1] != master->func_code))
    {
        return 0;
    }
    return 1;
}

uint8_t master_read_discs(modbus_master *master, uint8_t *result)
{
    master->func_code = MODBUS_FC_READ_DISCRETE_INPUTS;
    master->tx_len = 0;
    master->tx_buf[master->tx_len++] = master->slave_id;
    master->tx_buf[master->tx_len++] = master->func_code;
    master->tx_buf[master->tx_len++] = master->read_addr >> 8;
    master->tx_buf[master->tx_len++] = master->read_addr & 0xFF;
    master->tx_buf[master->tx_len++] = master->read_qty >> 8;
    master->tx_buf[master->tx_len++] = master->read_qty & 0xFF;
    if ((master->master_send_receive(MODBUS_TIMEOUT) == 1) && (master_validate_reply(master) == 1))
    {
        if (master->rx_buf[2] != (master->read_qty % 8 == 0 ? master->read_qty / 8 : master->read_qty / 8 + 1))
        {
            return 0;
        }
        for (master->i = 0; master->i < master->read_qty; master->i++)
        {
            result[master->i] = 0x01 & ((master->rx_buf[3 + master->i / 8]) >> (master->i % 8));
        }
        return 1;
    }
    return 0;
}

uint8_t master_read_coils(modbus_master *master, uint8_t *result)
{
    master->func_code = MODBUS_FC_READ_COILS;
    master->tx_len = 0;
    master->tx_buf[master->tx_len++] = master->slave_id;
    master->tx_buf[master->tx_len++] = master->func_code;
    master->tx_buf[master->tx_len++] = master->read_addr >> 8;
    master->tx_buf[master->tx_len++] = master->read_addr & 0xFF;
    master->tx_buf[master->tx_len++] = master->read_qty >> 8;
    master->tx_buf[master->tx_len++] = master->read_qty & 0xFF;
    if ((master->master_send_receive(MODBUS_TIMEOUT) == 1) && (master_validate_reply(master) == 1))
    {
        if (master->rx_buf[2] != (master->read_qty % 8 == 0 ? master->read_qty / 8 : master->read_qty / 8 + 1))
        {
            return 0;
        }
        for (master->i = 0; master->i < master->read_qty; master->i++)
        {
            result[master->i] = 0x01 & ((master->rx_buf[3 + master->i / 8]) >> (master->i % 8));
        }
        return 1;
    }
    return 0;
}

uint8_t master_read_input_regs(modbus_master *master, int16_t *result)
{
    master->func_code = MODBUS_FC_READ_INPUT_REGISTERS;
    master->tx_len = 0;
    master->tx_buf[master->tx_len++] = master->slave_id;
    master->tx_buf[master->tx_len++] = master->func_code;
    master->tx_buf[master->tx_len++] = master->read_addr >> 8;
    master->tx_buf[master->tx_len++] = master->read_addr & 0xFF;
    master->tx_buf[master->tx_len++] = master->read_qty >> 8;
    master->tx_buf[master->tx_len++] = master->read_qty & 0xFF;
    if (master->master_send_receive(MODBUS_TIMEOUT) && master_validate_reply(master))
    {
        if (master->rx_buf[2] != master->read_qty * 2)
        {
            return 0;
        }
        for (master->i = 0; master->i < master->read_qty; master->i++)
        {
            result[master->i] = master->rx_buf[3 + master->i * 2] * 256 + master->rx_buf[4 + master->i * 2];
        }
        return 1;
    }
    return 0;
}

uint8_t master_read_holding_regs(modbus_master *master, int16_t *result)
{
    master->func_code = MODBUS_FC_READ_HOLDING_REGISTERS;
    master->tx_len = 0;
    master->tx_buf[master->tx_len++] = master->slave_id;
    master->tx_buf[master->tx_len++] = master->func_code;
    master->tx_buf[master->tx_len++] = master->read_addr >> 8;
    master->tx_buf[master->tx_len++] = master->read_addr & 0xFF;
    master->tx_buf[master->tx_len++] = master->read_qty >> 8;
    master->tx_buf[master->tx_len++] = master->read_qty & 0xFF;
    if (master->master_send_receive(MODBUS_TIMEOUT) && master_validate_reply(master))
    {
        if (master->rx_buf[2] != master->read_qty * 2)
        {
            return 0;
        }
        for (master->i = 0; master->i < master->read_qty; master->i++)
        {
            result[master->i] = master->rx_buf[3 + master->i * 2] * 256 + master->rx_buf[4 + master->i * 2];
        }
        return 1;
    }
    return 0;
}

uint8_t master_write_single_coil(modbus_master *master, uint8_t *value)
{
    master->func_code = MODBUS_FC_WRITE_SINGLE_COIL;
    master->tx_len = 0;
    master->tx_buf[master->tx_len++] = master->slave_id;
    master->tx_buf[master->tx_len++] = master->func_code;
    master->tx_buf[master->tx_len++] = master->write_addr >> 8;
    master->tx_buf[master->tx_len++] = master->write_addr & 0xFF;
    master->tx_buf[master->tx_len++] = *value ? 0xFF : 0x00;
    master->tx_buf[master->tx_len++] = 0x00;
    if (master->master_send_receive(MODBUS_TIMEOUT) && master_validate_reply(master))
    {
        if (master->rx_buf[2] != (master->write_addr >> 8) || master->rx_buf[3] != (master->write_addr & 0xFF))
        {
            return 0;
        }
        if (master->rx_buf[4] != (value ? 0xFF : 0x00) || master->rx_buf[5] != 0x00)
        {
            return 0;
        }
        return 1;
    }
    return 0;
}

uint8_t master_write_single_holding_reg(modbus_master *master, int16_t *value)
{
    master->func_code = MODBUS_FC_WRITE_SINGLE_REGISTER;
    master->tx_len = 0;
    master->tx_buf[master->tx_len++] = master->slave_id;
    master->tx_buf[master->tx_len++] = master->func_code;
    master->tx_buf[master->tx_len++] = master->write_addr >> 8;
    master->tx_buf[master->tx_len++] = master->write_addr & 0xFF;
    master->tx_buf[master->tx_len++] = *value >> 8;
    master->tx_buf[master->tx_len++] = *value & 0xFF;
    if (master->master_send_receive(MODBUS_TIMEOUT) && master_validate_reply(master))
    {
        if (master->rx_buf[2] != (master->write_addr >> 8) || master->rx_buf[3] != (master->write_addr & 0xFF))
        {
            return 0;
        }
        if (master->rx_buf[4] != (*value >> 8) || master->rx_buf[5] != (*value & 0xFF))
        {
            return 0;
        }
        return 1;
    }
    return 0;
}

uint8_t master_write_multi_holding_regs(modbus_master *master, int16_t *value)
{
    master->func_code = MODBUS_FC_WRITE_MULTIPLE_REGISTERS;
    master->tx_len = 0;
    master->tx_buf[master->tx_len++] = master->slave_id;
    master->tx_buf[master->tx_len++] = master->func_code;
    master->tx_buf[master->tx_len++] = master->write_addr >> 8;
    master->tx_buf[master->tx_len++] = master->write_addr & 0xFF;
    master->tx_buf[master->tx_len++] = master->write_qty >> 8;
    master->tx_buf[master->tx_len++] = master->write_qty & 0xFF;
    master->tx_buf[master->tx_len++] = master->write_qty * 2;

    for (size_t i = 0; i < master->write_qty; i++)
    {
        master->tx_buf[master->tx_len++] = value[i] >> 8;
        master->tx_buf[master->tx_len++] = value[i] & 0xFF;
    }
    if (master->master_send_receive(MODBUS_TIMEOUT) && master_validate_reply(master))
    {
        if (master->rx_buf[2] != (master->write_addr >> 8) || master->rx_buf[3] != (master->write_addr & 0xFF))
        {
            return 0;
        }
        if (master->rx_buf[4] != (master->write_qty >> 8) || master->rx_buf[5] != (master->write_qty & 0xFF))
        {
            return 0;
        }
        return 1;
    }
    return 0;
}
