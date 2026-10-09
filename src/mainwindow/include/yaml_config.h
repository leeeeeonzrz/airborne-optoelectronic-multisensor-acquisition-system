#ifndef YAML_CONGIG_H
#define YAML_CONGIG_H
#include <iostream>
#include <string>
#include <yaml-cpp/yaml.h>

class YamlConfig
{
public:
    YamlConfig();
    ~YamlConfig();

public:
    void writeYAML(const std::string& filename);
    YAML::Node readYAML(const std::string&filename);
    void printYAML(const YAML::Node&config);

public:
    //吊舱
    std::string GIMBAL_SERIAL_PORT;
    int GIMBAL_SERIAL_BAUDRATE;
    int PITCH_ANGLE;//俯仰角
    int YAW_ANGLE;//偏航角
    int CAMERA_MODE;//相机模式 0:可见光 1：红外
    int FOCUS;//相机焦距 

    //气压高度计
    std::string AIRPRESSURE_SERIAL_PORT;//串口
    int AIRPRESSURE_SERIAL_BAUDRATE;//波特率

    //无线电高度计
    std::string RADIO_SERIAL_PORT;//串口
    int RADIO_SERIAL_BAUDRATE;//波特率

};

#endif