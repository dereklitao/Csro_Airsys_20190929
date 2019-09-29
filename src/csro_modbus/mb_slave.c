#include "mb_config.h"

static uint8_t slave_validate_cmd(modbus_slave *slave)
{
    if (slave->rx_len < 8)
    {
        return 0;
    }
    slave->buff_len = slave->rx_len - 2;
    slave_crc16(slave, slave->rx_buf);
    if ((slave->crc_hi != slave->rx_buf[slave->rx_len - 2]) || (slave->crc_lo != slave->rx_buf[slave->rx_len - 1]))
    {
        return 0;
    }
    if (slave->rx_buf[0] != slave->slave_id)
    {
        return 0;
    }
    return 1;
}

static void slave_read_discs(modbus_slave *slave)
{
    slave->tx_len = 0;
    slave->tx_buf[slave->tx_len++] = slave->slave_id;
    slave->read_addr = slave->rx_buf[2] * 256 + slave->rx_buf[3];
    slave->read_qty = slave->rx_buf[4] * 256 + slave->rx_buf[5];
    if (slave->read_addr + slave->read_qty > MODBUS_DISC_MAX)
    {
        slave->tx_buf[slave->tx_len++] = 0x80 + slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = 0x02;
    }
    else
    {
        slave->byte_count = (slave->read_qty % 8 == 0) ? (slave->read_qty / 8) : (slave->read_qty / 8 + 1);
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = slave->byte_count;
        for (slave->i = 0; slave->i < slave->byte_count * 8; slave->i++)
        {
            if (slave->i % 8 == 0)
            {
                slave->data = 0x00;
            }
            if (slave->i < slave->read_qty)
            {
                slave->data = (slave->regs->discs[slave->read_addr + slave->i] == 1) ? (slave->data >> 1 | 0x80) : (slave->data >> 1 & 0x7F);
            }
            else
            {
                slave->data = slave->data >> 1 & 0x7F;
            }
            if ((slave->i + 1) % 8 == 0)
            {
                slave->tx_buf[slave->tx_len++] = slave->data;
            }
        }
    }
}

static void slave_read_coils(modbus_slave *slave)
{
    slave->tx_len = 0;
    slave->tx_buf[slave->tx_len++] = slave->slave_id;
    slave->read_addr = slave->rx_buf[2] * 256 + slave->rx_buf[3];
    slave->read_qty = slave->rx_buf[4] * 256 + slave->rx_buf[5];
    if (slave->read_addr + slave->read_qty > MODBUS_COIL_MAX)
    {
        slave->tx_buf[slave->tx_len++] = 0x80 + slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = 0x02;
    }
    else
    {
        slave->byte_count = (slave->read_qty % 8 == 0) ? (slave->read_qty / 8) : (slave->read_qty / 8 + 1);
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = slave->byte_count;
        for (slave->i = 0; slave->i < slave->byte_count * 8; slave->i++)
        {
            if (slave->i % 8 == 0)
            {
                slave->data = 0x00;
            }
            if (slave->i < slave->read_qty)
            {
                slave->data = (slave->regs->coils[slave->read_addr + slave->i] == 1) ? ((slave->data >> 1) | 0x80) : ((slave->data >> 1) & 0x7F);
            }
            else
            {
                slave->data = (slave->data / 2) & 0x7F;
            }
            if ((slave->i + 1) % 8 == 0)
            {
                slave->tx_buf[slave->tx_len++] = slave->data;
            }
        }
    }
}

static void slave_read_input_regs(modbus_slave *slave)
{
    slave->tx_len = 0;
    slave->tx_buf[slave->tx_len++] = slave->slave_id;
    slave->read_addr = slave->rx_buf[2] * 256 + slave->rx_buf[3];
    slave->read_qty = slave->rx_buf[4] * 256 + slave->rx_buf[5];
    if (slave->read_addr + slave->read_qty > MODBUS_INPUT_MAX)
    {
        slave->tx_buf[slave->tx_len++] = 0x80 + slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = 0x02;
    }
    else
    {
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = slave->read_qty * 2;
        for (slave->i = 0; slave->i < slave->read_qty; slave->i++)
        {
            slave->tx_buf[slave->tx_len++] = slave->regs->inputs[slave->i + slave->read_addr] >> 8;
            slave->tx_buf[slave->tx_len++] = slave->regs->inputs[slave->i + slave->read_addr] & 0xFF;
        }
    }
}

static void slave_read_holding_regs(modbus_slave *slave)
{
    slave->tx_len = 0;
    slave->tx_buf[slave->tx_len++] = slave->slave_id;
    slave->read_addr = slave->rx_buf[2] * 256 + slave->rx_buf[3];
    slave->read_qty = slave->rx_buf[4] * 256 + slave->rx_buf[5];
    if (slave->read_addr + slave->read_qty > MODBUS_HOLDING_MAX)
    {
        slave->tx_buf[slave->tx_len++] = 0x80 + slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = 0x02;
    }
    else
    {
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = slave->read_qty * 2;
        for (slave->i = 0; slave->i < slave->read_qty; slave->i++)
        {
            slave->tx_buf[slave->tx_len++] = slave->regs->holdings[slave->i + slave->read_addr] >> 8;
            slave->tx_buf[slave->tx_len++] = slave->regs->holdings[slave->i + slave->read_addr] & 0xFF;
        }
    }
}

static void slave_write_single_coil(modbus_slave *slave)
{
    slave->tx_len = 0;
    slave->tx_buf[slave->tx_len++] = slave->slave_id;
    slave->write_addr = slave->rx_buf[2] * 256 + slave->rx_buf[3];
    if (slave->write_addr + 1 > MODBUS_COIL_MAX)
    {
        slave->tx_buf[slave->tx_len++] = 0x80 + slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = 0x02;
    }
    else
    {
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[2];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[3];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[4];
        slave->tx_buf[slave->tx_len++] = 0x00;
        slave->slave_write_coils_callback();
    }
}

static void slave_write_multi_coils(modbus_slave *slave)
{
    slave->tx_len = 0;
    slave->tx_buf[slave->tx_len++] = slave->slave_id;
    slave->write_addr = slave->rx_buf[2] * 256 + slave->rx_buf[3];
    slave->write_qty = slave->rx_buf[4] * 256 + slave->rx_buf[5];
    if (slave->write_addr + slave->write_qty > MODBUS_COIL_MAX)
    {
        slave->tx_buf[slave->tx_len++] = 0x80 + slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = 0x02;
    }
    else
    {
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[2];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[3];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[4];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[5];
        slave->slave_write_coils_callback();
    }
}

static void slave_write_single_holding_reg(modbus_slave *slave)
{
    slave->tx_len = 0;
    slave->tx_buf[slave->tx_len++] = slave->slave_id;
    slave->write_addr = slave->rx_buf[2] * 256 + slave->rx_buf[3];
    if (slave->write_addr + 1 > MODBUS_HOLDING_MAX)
    {
        slave->tx_buf[slave->tx_len++] = 0x80 + slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = 0x02;
    }
    else
    {
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[2];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[3];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[4];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[5];
        slave->slave_write_holdings_callback();
    }
}

static void slave_write_multi_holding_regs(modbus_slave *slave)
{
    slave->tx_len = 0;
    slave->tx_buf[slave->tx_len++] = slave->slave_id;
    slave->write_addr = slave->rx_buf[2] * 256 + slave->rx_buf[3];
    slave->write_qty = slave->rx_buf[4] * 256 + slave->rx_buf[5];
    if (slave->write_addr + slave->write_qty > MODBUS_HOLDING_MAX)
    {
        slave->tx_buf[slave->tx_len++] = 0x80 + slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = 0x02;
    }
    else
    {
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[1];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[2];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[3];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[4];
        slave->tx_buf[slave->tx_len++] = slave->rx_buf[5];
        slave->slave_write_holdings_callback();
    }
}

static void slave_unsupport_fucntion(modbus_slave *slave)
{
    slave->tx_len = 0;
    slave->tx_buf[slave->tx_len++] = slave->slave_id;
    slave->tx_buf[slave->tx_len++] = 0x80 + slave->rx_buf[1];
    slave->tx_buf[slave->tx_len++] = 0x01;
}

void slave_handle_cmd(modbus_slave *slave)
{
    if (slave_validate_cmd(slave) != 1)
    {
        return;
    }
    slave->func_code = slave->rx_buf[1];
    if (slave->func_code == MODBUS_FC_READ_DISCRETE_INPUTS)
    {
        slave_read_discs(slave);
    }
    else if (slave->func_code == MODBUS_FC_READ_COILS)
    {
        slave_read_coils(slave);
    }
    else if (slave->func_code == MODBUS_FC_READ_INPUT_REGISTERS)
    {
        slave_read_input_regs(slave);
    }
    else if (slave->func_code == MODBUS_FC_READ_HOLDING_REGISTERS)
    {
        slave_read_holding_regs(slave);
    }
    else if (slave->func_code == MODBUS_FC_WRITE_SINGLE_COIL)
    {
        slave_write_single_coil(slave);
    }
    else if (slave->func_code == MODBUS_FC_WRITE_MULTIPLE_COILS)
    {
        slave_write_multi_coils(slave);
    }
    else if (slave->func_code == MODBUS_FC_WRITE_SINGLE_REGISTER)
    {
        slave_write_single_holding_reg(slave);
    }
    else if (slave->func_code == MODBUS_FC_WRITE_MULTIPLE_REGISTERS)
    {
        slave_write_multi_holding_regs(slave);
    }
    else
    {
        slave_unsupport_fucntion(slave);
    }
    slave->slave_reply();
}
