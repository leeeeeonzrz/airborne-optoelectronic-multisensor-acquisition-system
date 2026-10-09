#include "../include/yaml_config.h"
#include "yaml_config.h"
#include <fstream>
#include <sstream>

YamlConfig::YamlConfig()
{
    // 串口端口号和波特率
    GIMBAL_SERIAL_PORT = "/dev/ttyCH343USB0";
    GIMBAL_SERIAL_BAUDRATE = 115200;
    PITCH_ANGLE = 0;
    YAW_ANGLE = 0;
    CAMERA_MODE = 0;
    FOCUS = 1;

    AIRPRESSURE_SERIAL_PORT = "/dev/ttyUSB1";
    AIRPRESSURE_SERIAL_BAUDRATE = 460800;

    RADIO_SERIAL_PORT = "/dev/ttyUSB3";
    RADIO_SERIAL_BAUDRATE = 230400;


}

YamlConfig::~YamlConfig()
{
}

void YamlConfig::writeYAML(const std::string &filename)
{
    YAML::Node config;
    config["GIMBAL_SERIAL_PORT"]="/dev/ttyv1";
    config["GIMBAL_SERIAL_BAUDRATE"]=9600;
    std::ofstream fout(filename);
    fout << config;
    fout.close();
}

YAML::Node YamlConfig::readYAML(const std::string &filename)
{
    YAML::Node config = YAML::LoadFile(filename);
    //吊舱
    GIMBAL_SERIAL_PORT = config["GIMBAL_SERIAL_PORT"].as<std::string>();
    GIMBAL_SERIAL_BAUDRATE = config["GIMBAL_SERIAL_BAUDRATE"].as<int>();
    PITCH_ANGLE = config["PITCH_ANGLE"].as<int>();
    YAW_ANGLE = config["YAW_ANGLE"].as<int>();

    //气压高
    AIRPRESSURE_SERIAL_PORT = config["AIRPRESSURE_SERIAL_PORT"].as<std::string>();
    AIRPRESSURE_SERIAL_BAUDRATE = config["AIRPRESSURE_SERIAL_BAUDRATE"].as<int>();

    //无线电
    RADIO_SERIAL_PORT = config["RADIO_SERIAL_PORT"].as<std::string>();
    RADIO_SERIAL_BAUDRATE = config["RADIO_SERIAL_BAUDRATE"].as<int>();

    return config;
    // return YAML::LoadFile(filename);
}

void YamlConfig::printYAML(const YAML::Node &config)
{
       // 输出参数
    std::cout << "GIMBAL_SERIAL_PORT: " << config["GIMBAL_SERIAL_PORT"] << std::endl;
    std::cout << "GIMBAL_SERIAL_BAUDRATE: " << config["GIMBAL_SERIAL_BAUDRATE"] << std::endl;
    //std::cout << "PX4_SERIAL_PORT: " << config["PX4_SERIAL_PORT"] << std::endl;
    //std::cout << "PX4_SERIAL_BAUDRATE: " << config["PX4_SERIAL_BAUDRATE"] << std::endl;
    std::cout << "PITCH_ANGLE: " << config["PITCH_ANGLE"] << std::endl;
    std::cout << "YAW_ANGLE: " << config["YAW_ANGLE"] << std::endl;
    
    std::cout << "AIRPRESSURE_SERIAL_PORT: " << config["AIRPRESSURE_SERIAL_PORT"] << std::endl;
    std::cout << "AIRPRESSURE_SERIAL_BAUDRATE: " << config["AIRPRESSURE_SERIAL_BAUDRATE"] << std::endl;

    std::cout << "RADIO_SERIAL_PORT: " << config["RADIO_SERIAL_PORT"] << std::endl;
    std::cout << "RADIO_SERIAL_BAUDRATE: " << config["RADIO_SERIAL_BAUDRATE"] << std::endl;

}
