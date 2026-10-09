#include "../include/radio_altitude.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include "radio_altitude.h"

SensorDataReceiver::SensorDataReceiver(const std::string &port, uint32_t baud_rate)
:port_(port), baud_rate_(baud_rate), capture_thread_flag(true)
{
    //memset(r_buffer, 0, sizeof(r_buffer));  // 确保缓冲区清空

    // try
    // {
    //     ser.setPort(port);
    //     ser.setBaudrate(baud_rate);
    //     serial::Timeout to = serial::Timeout::simpleTimeout(1000);
    //     ser.setTimeout(to);
    //     ser.open();
    // }
    // catch (serial::IOException &e)
    // {
    //     std::cerr << "Unable to open serial port" << std::endl;
    //     throw std::runtime_error("Serial port open failed");
    // }

    // if (ser.isOpen())
    // {
    //     std::cout << "Succeed to open port" << std::endl;
    // }
}

SensorDataReceiver::~SensorDataReceiver()
{

    closeSerial();
}

void SensorDataReceiver::record_radio_altitude()
{
    Radio_result temp_result;
    temp_result.altitude = -1;
    // 读取并解码数据
    float decodedHeight = readAndDecodeData();
    temp_result.gps_time = gps_time_str;;
    // 如果解码后的数据有效，写入文件
    if (decodedHeight != 0.0f)
    {
        temp_result.altitude = decodedHeight;
    }
    else
    {
        std::cerr << "No valid data to write to file" << std::endl;
    }
    result = temp_result;
}

void SensorDataReceiver::closeSerial()
{
    if (ser.isOpen())
    {
        ser.close();
        std::cout << "Serial port closed." << std::endl;
    }
}

bool SensorDataReceiver::initializeSerial()
{
    try
    {
        ser.setPort(port_);
        ser.setBaudrate(baud_rate_);
        serial::Timeout to = serial::Timeout::simpleTimeout(1000);
        ser.setTimeout(to);
        ser.open();
        if (ser.isOpen())
        {
            std::cout << "Succeed to open radio_alititude_port" << std::endl;
            return true;
        }
    }
    catch (serial::IOException &e)
    {
        std::cerr << "Unable to open radio_alititude_port" << std::endl;
        //throw std::runtime_error("Serial port open failed");
    }
    return false;
}


float SensorDataReceiver::decodeHexToFloat(const char *hexData)
{
    // 打印传入的十六进制字符串
    printf("Decoding hexData: %s\n", hexData);

    // 将十六进制字符串转换为整数
    int32_t i = strtol(hexData, nullptr, 16);

    // 打印转换后的整数值（调试用）
    printf("Converted int32_t value: %d\n", i);

    return i;;
}

float SensorDataReceiver::readAndDecodeData()
{
    r_buffer = std::vector<unsigned char>();
    size_t available_data = ser.available();
    //std::cout << available_data << std::endl;
    std::cout<<"无线电字节数"<<available_data<<std::endl;
    try
    {
        if (available_data > 0)
        {
            r_buffer.resize(available_data); // 根据可用数据调整vector大小
            // 读取串口数据
            ser.read(r_buffer.data(), available_data);
            //std::cout << r_buffer<< std::endl;
            // 遍历缓冲区，查找帧头和解码数据
            for (size_t i = 0; i < available_data; i++)
            {
                if(i + 1 < available_data)
                {
                    // 查找帧头
                    if (r_buffer[i] == 0x7E && r_buffer[i + 1] == 0xE7)
                    {
                        if(i + 5 < available_data)
                        {
                            if (r_buffer[i + 5] == 0x80 && i + 7 < available_data) // 静压值帧
                            {
                                // 取出 3-4 字节，转换为十六进制字符串
                                char hexData[5] = {0};  // 存储2字节的十六进制字符串
                                for (int j = 3; j <= 4; j++)
                                {
                                    sprintf(&hexData[(j - 3) * 2], "%02X", r_buffer[i + j]);
                                }

                                // 解码数据
                                float height = decodeHexToFloat(hexData);
                                std::cout << "高度为: " << (height / 100 - 300) / 10 << " m" << std::endl;

                                return (height / 100 - 300) / 10;
                            }
                        }

                    }
                }

            }
        }
    }
    catch (std::exception& e) 
    {
         std::cerr << "读取或解析串口数据时发生异常： " << std::endl;
    }
    return 0.0f;  // 如果没有可用数据，返回 0.0
}


void SensorDataReceiver::capture_serial_thread()
{
    while(capture_thread_flag)
    {
        record_radio_altitude();
    }
    std::cout<<"radio串口线程停止"<<std::endl;
}

void SensorDataReceiver::start_thread()
{
    if (!capture_thread) {
        capture_thread = std::make_unique<std::thread>(&SensorDataReceiver::capture_serial_thread, this);
    }
}

void SensorDataReceiver::stop_thread()
{
    capture_thread_flag = false;
    if (capture_thread && capture_thread->joinable()) 
    {
        capture_thread->join();
    }
}
