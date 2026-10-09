#include "../include/gnss_output.h"
#include <cmath>
#include "gnss_output.h"

// 构造函数：初始化订阅者和同步器
GnssOutput::GnssOutput(ros::NodeHandle& nh)
    : sub_euler(nh, "/sbg/ekf_euler", 40),
      //sub_gps(nh, "/sbg/gps_pos", 10),
      sub_ekf_nav(nh, "/sbg/ekf_nav", 40),
      sub_utc_time(nh, "/sbg/utc_time", 40),
      sub_imu(nh, "/sbg/imu_data", 40),
      //sync(FullSyncPolicy(1000), sub_euler, sub_gps, sub_ekf_nav, sub_utc_time),
      high_freq_sync(HighFreqSyncPolicy(1000), sub_euler, sub_ekf_nav, sub_utc_time, sub_imu),
      //high_freq_triggered(false),
      gnss_flag(0),
      capture_num(0),
      num(0)
{
    //sync.registerCallback(boost::bind(&GnssOutput::callback, this, _1, _2, _3, _4));
    high_freq_sync.registerCallback(boost::bind(&GnssOutput::highFreqCallback, this, _1, _2, _3, _4));
}

// 析构函数：关闭文件
GnssOutput::~GnssOutput()
{
    std::cout<<"GNSS析构"<<std::endl;

}



// 高频回调函数
void GnssOutput::highFreqCallback(const sbg_driver::SbgEkfEulerConstPtr& euler_data, 
                                  const sbg_driver::SbgEkfNavConstPtr& ekf_nav_data,
                                  const sbg_driver::SbgUtcTimeConstPtr& utc_time_data,
                                  const sbg_driver::SbgImuDataConstPtr& imu_data)
{
       Gnss_result temp_result;
    // if (gnss_flag == 1)
    // {
    //     return;
    // }
    
    //if (int(euler_data->status.solution_mode) == 4)
    //{
        //high_freq_triggered = true;
        double roll = euler_data->angle.x * 180 / M_PI;
        double pitch = euler_data->angle.y * 180 / M_PI;
        double yaw = euler_data->angle.z * 180 / M_PI;

        double latitude = ekf_nav_data->latitude;
        double longitude = ekf_nav_data->longitude;
        double altitude = ekf_nav_data->altitude;

        double velocity_x = ekf_nav_data->velocity.x;//LQ 尝试
        double velocity_y = ekf_nav_data->velocity.y;
        double velocity_z = ekf_nav_data->velocity.z;

        double imu_acc_x = imu_data->accel.x;
        double imu_acc_y = imu_data->accel.y;
        double imu_acc_z = imu_data->accel.z;

        double imu_gyro_x = imu_data->gyro.x * 180 / M_PI;
        double imu_gyro_y = imu_data->gyro.y * 180 / M_PI;
        double imu_gyro_z = imu_data->gyro.z * 180 / M_PI;

        int hour;
        if (int(utc_time_data->hour) >= 16) 
        {hour = utc_time_data->hour-16;}
        else 
        {hour = utc_time_data->hour + 8;}

        /***11.14.20:01 by zc***/
        // 将GPS时间格式化为字符串并保存在全局变量 gps_time_str 中
        std::ostringstream time_stream;
        time_stream << std::setw(2) << std::setfill('0') << hour << ":" 
                    << std::setw(2) << std::setfill('0') << int(utc_time_data->min) << ":" 
                    << std::setw(2) << std::setfill('0') << int(utc_time_data->sec) << "." 
                    << std::setw(3) << std::setfill('0') << int(utc_time_data->nanosec)/1000000;

        gps_time_str = time_stream.str();  // 保存格式化的时间字符串


        /***11.14.20:01 by zc***/

        // uint8_t hour = utc_time_data->hour;
        // uint8_t min = utc_time_data->min;
        // uint8_t sec = utc_time_data->sec;
        // uint32_t nanosec = utc_time_data->nanosec;

        // // ros::Time current_time = ros::Time::now();
        // // std::time_t raw_time = current_time.sec;
        // // std::tm* time_info = std::localtime(&raw_time);

        // output_file << std::fixed << std::setprecision(10) << int(euler_data->status.solution_mode) << "  "
        //             // << hour << ":" << min << ":" << sec << "." << std::setw(9) << std::setfill('0') << nanosec << "  "
        //             << int(hour)+8 << ":" << int(min) << ":" << int(sec) << "." << int(nanosec) << "  "
        //             << roll << "  " << pitch << "  " << yaw 
        //             << "  " << latitude << "  " << longitude << "  " << altitude  << "\n";

        // ROS_WARN("solution mode = %d  x滚转 = %f°  y俯仰 = %f°  z航向 = %f°", euler_data->status.solution_mode, roll, pitch, yaw);
        // ROS_INFO("位置：北纬%f°  东经%f°  海拔 = %fm", latitude, longitude, altitude);
        // ROS_INFO("当前GPS时间: %s", gps_time_str.c_str());
        
        temp_result.mode = int(euler_data->status.solution_mode);
        temp_result.pitch = pitch;
        temp_result.roll = roll;
        temp_result.yaw = yaw;
        temp_result.altitude = altitude;
        temp_result.latitude = latitude;
        temp_result.longitude = longitude;
        temp_result.gps_time = gps_time_str;
        temp_result.speed_x = velocity_x;
        temp_result.speed_y = velocity_y;
        temp_result.speed_z = velocity_z;
        temp_result.imu_acc_x = imu_acc_x;
        temp_result.imu_acc_y = imu_acc_y;
        temp_result.imu_acc_z = imu_acc_z;
        temp_result.imu_gyro_x = imu_gyro_x;
        temp_result.imu_gyro_y = imu_gyro_y;
        temp_result.imu_gyro_z = imu_gyro_z;

        //std::cout<<"gnss:"<<gps_time_str<<std::endl;
        result = temp_result;
        capture_num++;

        // std::stringstream ss_message;
        // ss_message << "当前GPS时间: " << gps_time_str<<"\n"
        // << "solution mode = " << int(euler_data->status.solution_mode) << "\n"
        // << "x滚转 = " << roll << "°  y俯仰 = " << pitch << "°  z航向 = " << yaw << "°" << "\n"
        // << "位置：北纬" << latitude << "°  东经" << longitude << "°  海拔 = " << altitude << "m" << "\n" << "\n";
        // message = ss_message.str();
       
        // {
        //     std::lock_guard<std::mutex> lock(gnss_flag_mutex);
        //     gnss_flag = 1;
        // }
    //}
    
}

// 低频回调函数
// void GnssOutput::callback(const sbg_driver::SbgEkfEulerConstPtr& euler_data, 
//                           const sbg_driver::SbgGpsPosConstPtr& gps_data,
//                           const sbg_driver::SbgEkfNavConstPtr& ekf_nav_data,
//                           const sbg_driver::SbgUtcTimeConstPtr& utc_time_data)
// {
//     if (high_freq_triggered) 
//     {
//         high_freq_triggered = false;
//         return;
//     }

//     double roll = euler_data->angle.x * 180 / M_PI;
//     double pitch = euler_data->angle.y * 180 / M_PI;
//     double yaw = euler_data->angle.z * 180 / M_PI;

//     double latitude = gps_data->latitude;
//     double longitude = gps_data->longitude;
//     double altitude = gps_data->altitude;

//     int hour;
//     if (int(utc_time_data->hour) >= 16) 
//     {hour = utc_time_data->hour-16;}
//     else 
//     {hour = utc_time_data->hour + 8;}

//     /***11.14.20:01 by zc***/
//     // 将GPS时间格式化为字符串并保存在全局变量 gps_time_str 中
//     std::ostringstream time_stream;
//     time_stream << std::setw(2) << std::setfill('0') << hour << ":" 
//                 << std::setw(2) << std::setfill('0') << int(utc_time_data->min) << ":" 
//                 << std::setw(2) << std::setfill('0') << int(utc_time_data->sec) << "." 
//                 << std::setw(3) << std::setfill('0') << int(utc_time_data->nanosec)/1000000;

//     gps_time_str = time_stream.str();  // 保存格式化的时间字符串

//     // 输出到文件
//     output_file << std::fixed << std::setprecision(10) 
//                 << int(euler_data->status.solution_mode) << "  "
//                 << gps_time_str << "  "
//                 << roll << "  " << pitch << "  " << yaw 
//                 << "  " << latitude << "  " << longitude << "  " << altitude  << "\n";
//     /***11.14.20:01 by zc***/

//     // uint8_t hour = utc_time_data->hour;
//     // uint8_t min = utc_time_data->min;
//     // uint8_t sec = utc_time_data->sec;
//     // uint32_t nanosec = utc_time_data->nanosec;


//     // // ros::Time current_time = ros::Time::now();
//     // // std::time_t raw_time = current_time.sec;
//     // // std::tm* time_info = std::localtime(&raw_time);

//     // output_file << std::fixed << std::setprecision(10) << int(euler_data->status.solution_mode) << "  "
//     //                 // << hour << ":" << min << ":" << sec << "." << std::setw(9) << std::setfill('0') << nanosec << "  "
//     //                 << int(hour)+8 << ":" << int(min) << ":" << int(sec) << "." << int(nanosec) << "  "
//     //                 << roll << "  " << pitch << "  " << yaw 
//     //                 << "  " << latitude << "  " << longitude << "  " << altitude  << "\n";

//     ROS_WARN("低频模式 solution mode = %d  x滚转 = %f°  y俯仰 = %f°  z航向 = %f°", euler_data->status.solution_mode, roll, pitch, yaw);
//     ROS_INFO("位置：北纬%f°  东经%f°  海拔 = %fm", latitude, longitude, altitude);
//     ROS_INFO("当前GPS时间: %s", gps_time_str.c_str());

    

//     gnss_flag = 1;
// }

// 信号处理函数：确保程序退出时关闭文件
void GnssOutput::signalHandler(int signum)
{
    ROS_INFO("Closing file...");
    ros::shutdown();
}

// 启动同步功能
void GnssOutput::start()
{
    signal(SIGINT, GnssOutput::signalHandler);

    ros::AsyncSpinner spinner(2); // 启动两个线程
    spinner.start();

    while (ros::ok()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // 示例：保持循环运行
    }

    spinner.stop();
}

void GnssOutput::start1()
{
    signal(SIGINT, GnssOutput::signalHandler);
    // 使用单线程运行 ROS 节点
}