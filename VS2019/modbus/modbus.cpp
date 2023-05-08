#include <iostream>
#include <co/all.h>
#include "modbus.h"
#include <mutex>
std::mutex mutex;
co::WaitGroup exitWait;

#define MOD_CANCLE 0
#define MOD_4 4
#define MOD_5 5

bool shijiashan_SB_zuo[20] = {0}; // 242
bool shijiashan_SB_you[20] = {0}; // 262

bool shijiashan_HY_zuo[20] = {0}; // 328
bool shijiashan_HY_you[20] = {0}; // 348

bool bogushan_SB_zuo[31] = {0}; // 482
bool bogushan_SB_you[31] = {0}; // 513

bool bogushan_HY_zuo[31] = {0}; // 610
bool bogushan_HY_you[31] = {0}; // 641

uint16 shijiashan_Ruo_zuo[20] = {0};   // 10063
uint16 shijiashan_Qiang_zuo[21] = {0}; // 10042

uint16 shijiashan_Ruo_you[20] = {0};   // 10022
uint16 shijiashan_Qiang_you[21] = {0}; // 10001

uint16 bogushan_Ruo_you[31] = {0};   // 10116
uint16 bogushan_Qiang_you[33] = {0}; // 10083

uint16 bogushan_Ruo_zuo[31] = {0};   // 10179
uint16 bogushan_Qiang_zuo[32] = {0}; // 10147

// #define TEST
#ifdef TEST
#define IP_ALARM "127.0.0.1"
#define PORT_ALARM 502
#else
#define IP_ALARM "33.66.240.133"
#define PORT_ALARM 502
#endif // TEST

void control(bool _isShijiashan, bool _iszuodong, uint16 _num);
void clean(bool _isShijiashan, bool _iszuodong);

void md_poll_shijiashan_zuo(void)
{
    co::sleep(1000);

    try
    {
        // auto g = co::scheduler();
        modbus mb = modbus(IP_ALARM, PORT_ALARM);
        mb.modbus_set_slave_id(1);
        std::lock_guard<std::mutex> grd(mutex);

        if (!mb.modbus_connect())
        {
            ELOG << "modbus 连接失败...... 一秒后重连...... ip: " << IP_ALARM;
        }
        else
        {
            if (mb.is_connected())
            {
                if (mb.modbus_read_coils(242, sizeof(shijiashan_SB_zuo), shijiashan_SB_zuo) == 0)
                {
                    for (int i = 0; i < sizeof(shijiashan_SB_zuo); i++)
                    {
                        // DLOG << "[modbus] id: " << i << "  value: " << hijiashan_SB_zuo[i] << "\n";
                        if (shijiashan_SB_zuo[i])
                        {

                            co::scheduler()->go([i]
                                                { control(true, true, i); });
                            mb.modbus_close();

                            co::scheduler()->go(md_poll_shijiashan_zuo);

                            return;
                        }
                    }
                }

                if (mb.modbus_read_coils(328, sizeof(shijiashan_HY_zuo), shijiashan_HY_zuo) == 0)
                {
                    for (int i = 0; i < sizeof(shijiashan_HY_zuo); i++)
                    {
                        // DLOG << "[modbus] id: " << i << "  value: " << read_holding_regs[i] << "\n";
                        if (shijiashan_HY_zuo[i])
                        {
                            co::scheduler()->go([i]
                                                { control(true, true, i); });
                            mb.modbus_close();
                            co::scheduler()->go(md_poll_shijiashan_zuo);

                            return;
                        }
                    }
                }
                co::scheduler()->go([]
                                    { clean(true, true); });
            }
            else
            {
                ELOG << "modbus 连接断开！ ip: " << IP_ALARM;
            }
        }
        mb.modbus_close();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cout << "Error!\n";
    }
    co::scheduler()->go(md_poll_shijiashan_zuo);
}

void md_poll_shijiashan_you(void)
{
    co::sleep(1000);

    try
    {
        // auto g = co::scheduler();
        modbus mb = modbus(IP_ALARM, PORT_ALARM);
        mb.modbus_set_slave_id(1);
        std::lock_guard<std::mutex> grd(mutex);
        if (!mb.modbus_connect())
        {
            ELOG << "modbus 连接失败...... 一秒后重连...... ip: " << IP_ALARM;
        }
        else
        {
            if (mb.is_connected())
            {
                if (mb.modbus_read_coils(262, sizeof(shijiashan_SB_you), shijiashan_SB_you) == 0)
                {
                    for (int i = 0; i < sizeof(shijiashan_SB_you); i++)
                    {
                        // DLOG << "[modbus] id: " << i << "  value: " << hijiashan_SB_you[i] << "\n";
                        if (shijiashan_SB_you[i])
                        {
                            co::scheduler()->go([i]
                                                { control(true, false, i); });
                            mb.modbus_close();
                            co::scheduler()->go(md_poll_shijiashan_you);

                            return;
                        }
                    }
                }

                if (mb.modbus_read_coils(348, sizeof(shijiashan_HY_you), shijiashan_HY_you) == 0)
                {
                    for (int i = 0; i < sizeof(shijiashan_HY_you); i++)
                    {
                        // DLOG << "[modbus] id: " << i << "  value: " << read_holding_regs[i] << "\n";
                        if (shijiashan_HY_you[i])
                        {
                            co::scheduler()->go([i]
                                                { control(true, false, i); });
                            mb.modbus_close();
                            co::scheduler()->go(md_poll_shijiashan_you);

                            return;
                        }
                    }
                }
                co::scheduler()->go([]
                                    { clean(true, false); });
            }
            else
            {
                ELOG << "modbus 连接断开！ ip: " << IP_ALARM;
            }
        }
        mb.modbus_close();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cout << "Error!\n";
    }
    co::scheduler()->go(md_poll_shijiashan_you);
}

void md_poll_bogushan_zuo(void)
{
    co::sleep(1000);

    try
    {
        // auto g = co::scheduler();
        modbus mb = modbus(IP_ALARM, PORT_ALARM);
        mb.modbus_set_slave_id(1);
        std::lock_guard<std::mutex> grd(mutex);
        if (!mb.modbus_connect())
        {
            ELOG << "modbus 连接失败...... 一秒后重连...... ip: " << IP_ALARM;
        }
        else
        {
            if (mb.is_connected())
            {
                if (mb.modbus_read_coils(482, sizeof(bogushan_SB_zuo), bogushan_SB_zuo) == 0)
                {
                    for (int i = 0; i < sizeof(bogushan_SB_zuo); i++)
                    {
                        // DLOG << "[modbus] id: " << i << "  value: " << hijiashan_SB_zuo[i] << "\n";
                        if (bogushan_SB_zuo[i])
                        {
                            co::scheduler()->go([i]
                                                { control(false, true, i); });
                            mb.modbus_close();
                            co::scheduler()->go(md_poll_bogushan_zuo);

                            return;
                        }
                    }
                }

                if (mb.modbus_read_coils(610, sizeof(bogushan_HY_zuo), bogushan_HY_zuo) == 0)
                {
                    for (int i = 0; i < sizeof(bogushan_HY_zuo); i++)
                    {
                        // DLOG << "[modbus] id: " << i << "  value: " << read_holding_regs[i] << "\n";
                        if (bogushan_HY_zuo[i])
                        {
                            co::scheduler()->go([i]
                                                { control(false, true, i); });
                            mb.modbus_close();
                            co::scheduler()->go(md_poll_bogushan_zuo);

                            return;
                        }
                    }
                }
                co::scheduler()->go([]
                                    { clean(false, true); });
            }
            else
            {
                ELOG << "modbus 连接断开！ ip: " << IP_ALARM;
            }
        }
        mb.modbus_close();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cout << "Error!\n";
    }

    co::scheduler()->go(md_poll_bogushan_zuo);
}

void md_poll_bogushan_you(void)
{
    co::sleep(1000);

    try
    {
        // auto g = co::scheduler();

        modbus mb = modbus(IP_ALARM, PORT_ALARM);
        mb.modbus_set_slave_id(1);
        std::lock_guard<std::mutex> grd(mutex);

        if (!mb.modbus_connect())
        {
            ELOG << "modbus 连接失败...... 一秒后重连...... ip: " << IP_ALARM;
        }
        else
        {
            if (mb.is_connected())
            {
                if (mb.modbus_read_coils(513, sizeof(bogushan_SB_you), bogushan_SB_you) == 0)
                {
                    for (int i = 0; i < sizeof(bogushan_SB_you); i++)
                    {
                        // DLOG << "[modbus] id: " << i << "  value: " << hijiashan_SB_you[i] << "\n";
                        if (bogushan_SB_you[i])
                        {
                            co::scheduler()->go([i]
                                                { control(false, false, i); });
                            mb.modbus_close();
                            co::scheduler()->go(md_poll_bogushan_you);
                            return;
                        }
                    }
                }

                if (mb.modbus_read_coils(641, sizeof(bogushan_HY_you), bogushan_HY_you) == 0)
                {
                    for (int i = 0; i < sizeof(bogushan_HY_you); i++)
                    {
                        // DLOG << "[modbus] id: " << i << "  value: " << read_holding_regs[i] << "\n";
                        if (bogushan_HY_you[i])
                        {
                            co::scheduler()->go([i]
                                                { control(false, false, i); });
                            mb.modbus_close();
                            co::scheduler()->go(md_poll_bogushan_you);

                            return;
                        }
                    }
                }
                co::scheduler()->go([]
                                    { clean(false, false); });
            }
            else
            {
                ELOG << "modbus 连接断开！ ip: " << IP_ALARM;
            }
        }
        mb.modbus_close();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cout << "Error!\n";
    }
    co::scheduler()->go(md_poll_bogushan_you);
}

void pollFunction()
{
    try
    {
        md_poll_bogushan_you();
        md_poll_bogushan_zuo();
        md_poll_shijiashan_you();
        md_poll_shijiashan_zuo();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cout << "Error!\n";
    }
}

int main()
{
    flag::set_value("cout", "true");

    go(md_poll_bogushan_you);
    go(md_poll_bogushan_zuo);
    go(md_poll_shijiashan_you);
    go(md_poll_shijiashan_zuo);

    // co::scheduler()->go(pollFunction);

    exitWait.add();
    exitWait.wait();
}

void control(bool _isShijiashan, bool _iszuodong, uint16 _num)
{
    try
    {
        WLOG << "接收到消防报警指令"
             << " _isShijiashan: " << _isShijiashan << " |_iszuodong: " << _iszuodong << " _num: " << _num;
        uint16 bool_buf[50] = {0};
        uint16 bool_buf_right[50] = {0};

        uint16 _amout_q = 0;
        uint16 _amout_r = 0;
        uint16 _add_q = 0;
        uint16 _add_r = 0;

        std::string _ip;
        int _slaveId = 0;
        if (_isShijiashan)
        {
            _ip = "33.66.240.138";
            _slaveId = 1;
            if (_iszuodong)
            {
                _amout_q = 21;
                _amout_r = 20;
                _add_q = 10042;
                _add_r = 10063;
            }
            else
            {
                _amout_q = 21;
                _amout_r = 20;
                _add_q = 10001;
                _add_r = 10022;
            }
        }
        else
        {
            _ip = "33.66.240.139";
            _slaveId = 2;
            if (_iszuodong)
            {
                _amout_q = 32;
                _amout_r = 31;
                _add_q = 10147;
                _add_r = 10179;
            }
            else
            {
                _amout_q = 33;
                _amout_r = 31;
                _add_q = 10083;
                _add_r = 10116;
            }
        }

        for (size_t i = 0; i < _num; i++)
        {
            if (_iszuodong)
                bool_buf[i] = MOD_5;
            else
                bool_buf[i] = MOD_4;
        }

        for (size_t i = _num; i < _amout_q; i++)
        {
            if (_iszuodong)
                bool_buf[i] = MOD_4;
            else
                bool_buf[i] = MOD_5;
        }

        modbus mb = modbus(_ip, 502);
        mb.modbus_set_slave_id(_slaveId);
        std::lock_guard<std::mutex> grd(mutex);
        if (!mb.modbus_connect())
        {
            ELOG << "modbus 连接失败...... 下发控制失败...... ip: " << _ip;
        }
        else
        {
            if (mb.is_connected())
            {

                mb.modbus_write_registers(_add_q, _amout_q, bool_buf);
                // ELOG << "modbus 寄存器写入失败...... 控制下发失败...... ip: " << _ip;

                for (size_t i = _num; i < _amout_r; i++)
                {
                    if (bool_buf[i] == MOD_5)
                        bool_buf[i] = MOD_4;
                    else
                        bool_buf[i] = MOD_5;
                }
                mb.modbus_write_registers(_add_r, _amout_r, bool_buf_right);
                // ELOG << "modbus 寄存器写入失败...... 控制下发失败...... ip: " << _ip;
            }
            else
            {
                ELOG << "modbus 连接失败...... 下发控制失败...... ip: " << _ip;
            }
        }
        mb.modbus_close();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cout << "Error!\n";
    }
}

void clean(bool _isShijiashan, bool _iszuodong)
{
    try
    {
        uint16 bool_buf[50] = {0};
        uint16 _amout_q = 0;
        uint16 _amout_r = 0;
        uint16 _add_q = 0;
        uint16 _add_r = 0;

        std::string _ip;
        int _slaveId = 0;
        if (_isShijiashan)
        {
            _ip = "33.66.240.138";
            _slaveId = 1;
            if (_iszuodong)
            {
                _amout_q = 21;
                _amout_r = 20;
                _add_q = 10042;
                _add_r = 10063;
            }
            else
            {
                _amout_q = 21;
                _amout_r = 20;
                _add_q = 10001;
                _add_r = 10022;
            }
        }
        else
        {
            _ip = "33.66.240.139";
            _slaveId = 2;
            if (_iszuodong)
            {
                _amout_q = 32;
                _amout_r = 31;
                _add_q = 10147;
                _add_r = 10179;
            }
            else
            {
                _amout_q = 33;
                _amout_r = 31;
                _add_q = 10083;
                _add_r = 10116;
            }
        }

        modbus mb = modbus(_ip, 502);
        mb.modbus_set_slave_id(_slaveId);
        std::lock_guard<std::mutex> grd(mutex);
        if (!mb.modbus_connect())
        {
            ELOG << "modbus 连接失败...... 取消应急失败...... ip: " << _ip;
        }
        else
        {
            if (mb.is_connected())
            {
                mb.modbus_write_registers(_add_q, _amout_q, bool_buf);
                // ELOG << "modbus 寄存器写入失败...... 取消应急失败...... ip: " << _ip;
                mb.modbus_write_registers(_add_r, _amout_r, bool_buf);
                // ELOG << "modbus 寄存器写入失败...... 取消应急失败...... ip: " << _ip;
            }
            else
            {
                ELOG << "modbus 连接失败...... 取消应急失败...... ip: " << _ip;
            }
        }
        mb.modbus_close();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    catch (...)
    {
        std::cout << "Error!\n";
    }
}