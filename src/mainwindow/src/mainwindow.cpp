#include "../include/mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QIntValidator>
#include "mainwindow.h"

#include <QDateTime>
#include <QFileDialog>
#include <random>
#include <QMetaType>



MainWindow::MainWindow(int argc, char** argv, QWidget *parent) :
    QMainWindow(parent),
    first_connect(true),
    focus(1),
    timerId(0),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    timerId = startTimer(0);

    //qnode
    qnode = new QNode(argc, argv);
    create_top_folder();
    create_folder();
    qnode->moveToThread(&qthread_node);
    //connect(&qthread_node, &QThread::finished, qnode, &QObject::deleteLater);
    connect(this, &MainWindow::sig_start_qnode, qnode, &QNode::run);
    connect(qnode, SIGNAL(sig_qnode_finished()), this, SLOT(slot_qnode_finished()));
    //connect(&qnode,&QNode::rosShutdown,this,&MainWindow::close);
    connect(qnode, SIGNAL(sig_update_image(QImage)), this, SLOT(slot_update_image(QImage)));//更新图片显示
    //connect(qnode, SIGNAL(sig_update_GNSS_Message(std::string)), this, SLOT(slot_update_GNSS_Message(std::string)));//更新GNSS显示
    qRegisterMetaType<Gnss_result>("Gnss_result");
    qRegisterMetaType<Gimbal_result>("Gimbal_result");
    qRegisterMetaType<AirPressure_result>("AirPressure_result");
    qRegisterMetaType<std::string>("std::string");
    connect(qnode, SIGNAL(sig_update_GNSS_result(Gnss_result)), this, SLOT(slot_update_GNSS_result(Gnss_result)));
    connect(qnode, SIGNAL(sig_update_gimbal_result(Gimbal_result)), this, SLOT(slot_update_gimbal_result(Gimbal_result)));//更新吊舱仪表信息
    connect(qnode, SIGNAL(sig_update_airPressure_result(AirPressure_result)), this, SLOT(slot_update_airPressure_result(AirPressure_result)));//更新气压计仪表信息
    connect(qnode, SIGNAL(sig_update_airPressure_status(std::string)), this, SLOT(slot_update_airPressure_status(std::string)));//更新气压计消息栏
    //connect(qnode, SIGNAL(sig_update_air_pressure_message(std::string)), this, SLOT(slot_update_air_pressure_message(std::string)));//更新气压高度计显示
    //connect(qnode, SIGNAL(sig_update_radio_altitude_message(float)), this, SLOT(slot_update_radio_altitude_message(float)));//更新无线电高度计显示
    connect(qnode, SIGNAL(sig_update_radio_altitude_result(Radio_result)), this, SLOT(slot_update_radio_altitude_result(Radio_result)));//更新无线电高度计显示
    connect(qnode, SIGNAL(sig_update_status_bar(QString)), this, SLOT(slot_update_status_bar(QString)));//更新状态栏
    connect(qnode, SIGNAL(sig_update_label_image_num(int)), this, SLOT(slot_update_label_image_num(int)));//更新图片数量
   // connect(qnode, SIGNAL(sig_update_label_time(QString)), this, SLOT(slot_update_label_time(QString)));
    connect(qnode, SIGNAL(sig_connect_succeed()), this, SLOT(slot_connect_succeed()));//传感器连接成功
    connect(qnode, SIGNAL(sig_clear_message()), this, SLOT(slot_clear_message()));//清除界面上的plainTextEdit显示
    connect(qnode, SIGNAL(sig_update_plainTextEdit_status(QString)), this, SLOT(slot_update_plainTextEdit_status(QString)));//更新消息框
    qthread_node.start();
    
    //ThreadControl控制线程
    thread_control = new ThreadControl(qnode->top_folder_path);
    thread_control->moveToThread(&qthread_control);
    connect(this, &MainWindow::sig_start_control, thread_control, &ThreadControl::set_gimbal_config);//设置吊舱参数
    connect(this, &MainWindow::sig_zoom_control, thread_control, &ThreadControl::set_gimbal_zoom);//单步调焦
    connect(this, &MainWindow::sig_set_downward, thread_control, &ThreadControl::set_gimbol_down);//一键向下
    connect(this, &MainWindow::sig_set_center, thread_control, &ThreadControl::set_gimabl_center);//一键回中
    connect(thread_control, SIGNAL(sig_update_status_bar(QString)), this, SLOT(slot_update_status_bar(QString)));
    connect(thread_control, SIGNAL(sig_update_plainTextEdit_status(QString)), this, SLOT(slot_update_plainTextEdit_status(QString)));//更新消息框
    connect(this, &MainWindow::sig_gimbol_focus_restore, thread_control, &ThreadControl::set_gimbol_focus_restore);
    connect(thread_control, SIGNAL(set_gimbal_config_finished()), this, SLOT(slot_gimbal_config_finished()));
    connect(this, &MainWindow::sig_angle_control, thread_control,&ThreadControl::set_gimbol_angle);
    qthread_control.start();

    // qDebug() << QDir::currentPath();
    
    QIntValidator* IntValidator = new QIntValidator;
    IntValidator->setRange(-180, 180);
    ui->lineEdit_pitch->setValidator(IntValidator);
    ui->lineEdit_yaw->setValidator(IntValidator);
    ui->lineEdit_roll->setValidator(IntValidator);

    ui->lineEdit_pitch->setText(QString::number(qnode->yaml_config.PITCH_ANGLE));
    ui->lineEdit_yaw->setText(QString::number(qnode->yaml_config.YAW_ANGLE));

    //吊舱参数初始化
    gimbal_para.camera_mode = 0;
    gimbal_para.focus = 1;
    gimbal_para.pitch = ui->lineEdit_pitch->text().toInt();
    gimbal_para.roll = ui->lineEdit_roll->text().toInt();
    gimbal_para.yaw = ui->lineEdit_yaw->text().toInt();

    //串口设置对话框
    dlg_serial_setting = new DlgSerialSetting();

    ui->label_roll->setText(QString::number(0));
    ui->label_yaw->setText(QString::number(0));
    ui->label_pitch->setText(QString::number(0));

    //ui->spinBox_focus->setVisible(false);

    //ui控件信号和槽的连接
    connect(ui->action_connect,&QAction::triggered, this, &MainWindow::slot_action_connect_triggred);
    connect(ui->action_GNSS_reload,&QAction::triggered, this, &MainWindow::slot_action_GNSS_reload_triggred);
    connect(ui->action_serial_setting,&QAction::triggered, this, &MainWindow::slot_action_serial_setting_triggred);
    connect(ui->btn_camera_apply,SIGNAL(clicked()),this,SLOT(slot_btn_camera_apply_clicked()));
    connect(ui->tBn_zoom_up,SIGNAL(clicked()),this,SLOT(slot_tBn_zoom_up_clicked()));
    connect(ui->tBn_zoom_down,SIGNAL(clicked()),this,SLOT(slot_tBn_zoom_down_clicked()));
    connect(ui->btn_downward,SIGNAL(clicked()),this,SLOT(slot_btn_downward_clicked()));
    connect(ui->btn_center,SIGNAL(clicked()),this,SLOT(slot_btn_center_clicked()));
    connect(ui->action_save,&QAction::triggered, this, &MainWindow::slot_action_save_triggred);
    connect(ui->rBtn_GNSS,SIGNAL(clicked(bool)),this,SLOT(slot_rBtn_GNSS_clicked(bool)));
    connect(ui->rBtn_barometric_altimeter,SIGNAL(clicked(bool)),this,SLOT(slot_rBtn_barometric_altimeter_clicked(bool)));
    connect(ui->rBtn_radio_altimeter,SIGNAL(clicked(bool)),this,SLOT(slot_rBtn_radio_altimeter_clicked(bool)));
    //connect(ui->action_open,&QAction::triggered, this, &MainWindow::slot_action_open_triggred);
    //connect(ui->action_play,&QAction::triggered, this, &MainWindow::slot_action_play_triggred);

    //connect(ui->spinBox_focus, SIGNAL(valueChanged(int)), this, SLOT(slot_spinBox_focus_valueChanged(int)));
    connect(ui->comboBox_camera_mode, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_comboBox_camera_mode_currentIndexChanged(int)));
    connect(ui->btn_scene_matching, SIGNAL(clicked()), this, SLOT(slot_btn_scene_matching_clicked()));
    connect(ui->btn_terrain_matching, SIGNAL(clicked()), this, SLOT(slot_btn_terrain_matching_clicked()));
    connect(ui->btn_high_fusion, SIGNAL(clicked()), this, SLOT(slot_btn_high_fusion_clicked()));


    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000); 
    
    //timer_control = new QTimer(this);
    //connect(timer_control, &QTimer::timeout, this, &MainWindow::slot_control_angle);
    //timer_control->start(60000); //1 分钟
 
    
}
MainWindow::~MainWindow()
{
    std::cout<<"mainwindow析构"<<std::endl;
    killTimer(timerId);
    // qthread_node.quit();
    // qthread_node.wait();
    if(qnode != nullptr)
    {
        qnode->deleteLater();
        qthread_node.quit();
        qthread_node.wait();
        qnode = nullptr;
    }

    if(thread_control != nullptr)
    {
        thread_control->deleteLater();
        qthread_control.quit();
        qthread_control.wait();
        thread_control = nullptr;
    }
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //emit closed(); // 当窗口关闭时发出信号
    std::cout<<"mainwindow关闭"<<std::endl;
    qnode->close_flag = true;
    QMainWindow::closeEvent(event);
}

void MainWindow::slot_action_connect_triggred()
{
    if (ui->action_connect->isChecked())
    {
        if(first_connect)
        {
            //运行gnss驱动
            runGNSSDrive();
            //初始化 qnode线程
            emit sig_start_qnode();
            first_connect = false;
            
        }
        qnode->keep_transmitting = true;
    }
    else
    {
        qnode->keep_transmitting = false;
    }
}

void MainWindow::slot_connect_succeed()
{
    //赋值控制线程对象
    thread_control->mgimbal_control = qnode->gimbal_controller;
}

void MainWindow::slot_action_save_triggred()
{
    bool flag = ui->action_save->isChecked();
    //std::cout<<"aaaaa  "<<flag<<std::endl;
    qnode->save_flag = flag;
    //qnode->save_flag_changed = true;//出错是因为没加锁
    
}

// void MainWindow::slot_action_open_triggred()
// {
//     QString filePath = QFileDialog::getOpenFileName(nullptr, "选择文件", "../", "Text Files (*.txt);;All Files (*)");
//     if (!filePath.isEmpty()) 
//     {
//        // ui->lineEdit->setText(filePath);
//         std::string strfilepath = filePath.toStdString();
//         std::cout<<strfilepath<<std::endl;

//     }
// }

// void MainWindow::slot_action_play_triggred()
// {
//     if(ui->action_play->isChecked())
//     {
//         ui->action_play->setText("暂停");
//         ui->action_play->setIcon(QIcon(":/resources/pause.png"));
//     }
//     else
//     {
//         ui->action_play->setText("播放");
//         ui->action_play->setIcon(QIcon(":/resources/play.png"));
//     }
// }

void MainWindow::slot_btn_camera_apply_clicked()
{
    gimbal_para.camera_mode = ui->comboBox_camera_mode->currentIndex();
    gimbal_para.focus = ui->spinBox_focus->value();
    gimbal_para.pitch = ui->lineEdit_pitch->text().toInt();
    gimbal_para.roll = ui->lineEdit_roll->text().toInt();
    gimbal_para.yaw = ui->lineEdit_yaw->text().toInt();


    std::stringstream ss_message;
    ss_message << "\n"<<"调整吊舱参数为: \n" << "{ 相机模式: " << gimbal_para.camera_mode <<"; 变焦倍数: "<<gimbal_para.focus
        << "; 俯仰角: "<<gimbal_para.pitch << "; 滚转角: "<<gimbal_para.roll << "; 偏航角: "<<gimbal_para.yaw << " }\n";

    if(qnode->connect_gimbal)//如果传感器初始化成功
    {
        thread_control->control_able = false;//不允许定时调控角度(已弃用)
        //已经和设备连接后 进行吊舱控制
        //thread_control->mgimbal_control = qnode->gimbal_controller;
        thread_control->mparameter = gimbal_para;
        thread_control->gps_time_str = qnode->gps_time_str;

        //设置以下按钮不可用 避免误操作
        ui->btn_camera_apply->setEnabled(false);
        ui->btn_downward->setEnabled(false);
        ui->btn_center->setEnabled(false);
        ui->tBn_zoom_up->setEnabled(false);
        ui->tBn_zoom_down->setEnabled(false);

        emit sig_start_control();
    }

    if(first_connect)
    {
        //设备还未连接 进行吊舱控制 实际是更新吊舱初始化参数
        qnode->yaml_config.PITCH_ANGLE = gimbal_para.pitch;
        qnode->yaml_config.YAW_ANGLE = gimbal_para.yaw;
        qnode->yaml_config.CAMERA_MODE = gimbal_para.camera_mode;
        qnode->yaml_config.FOCUS = gimbal_para.focus;

        ///////////////////////////记录控制状态/////////////////////////////////
        std::ofstream output_file;
        output_file.open(thread_control->filename.str(), std::ios::app);
        if (output_file.is_open())
        {
            output_file << "初始值: " << std::endl;
            if(gimbal_para.camera_mode == 0)
            {
                output_file << "模式: 可见光"<<std::endl;
            }
            else if(gimbal_para.camera_mode == 1)
            {
                output_file << "模式: 红外"<<std::endl;
            }
            output_file << "俯仰角: "<<gimbal_para.pitch << " 偏航角: "<<gimbal_para.yaw<<" 滚转角: "<< gimbal_para.roll << std::endl;
            output_file << "变焦倍数: "<<gimbal_para.focus << "x"<<std::endl;
            output_file << std::endl; 
        }
        output_file.close();
        //////////////////////////////////////////////////////////////////////
    }

    ui->plainTextEdit_status->appendPlainText(QString::fromStdString(ss_message.str()));
}

void MainWindow::slot_spinBox_focus_valueChanged(int value)
{
    std::cout<<"变焦倍数: "<<value<<std::endl;
}

void MainWindow::slot_comboBox_camera_mode_currentIndexChanged(int value)
{
    if(qnode->connect_gimbal)
    {
        //如果相机的模式改变了,焦距就不是当前焦距了
        std::cout<<"模式: "<<value<<std::endl;
        int temp = ui->spinBox_focus->value();//上一个模式的焦距值
        ui->spinBox_focus->setValue(focus);
        qnode->gimbal_controller->previous_focus = focus;
        focus = temp;
    }
}

// 运行GNSS驱动
void MainWindow::runGNSSDrive()
{
    //QString command = QString("source ./devel/setup.bash && roslaunch sbg_driver sbg_ellipseN.launch; exec bash");
    QString command = QString("source ./devel/setup.bash && roslaunch sbg_driver sbg_ellipseN2.launch; exec bash");
    process_gnss = new QProcess(this);
    process_gnss->startDetached("gnome-terminal", QStringList() << "-e" << "bash -c \"" + command + "\"");
    //system("gnome-terminal -- bash -c 'source ./devel/setup.bash && roslaunch sbg_driver sbg_ellipseN.launch; exec bash'");
}

void MainWindow::slot_qnode_finished()
{
    if(qnode->connect_gimbal)
    {
        //thread_control->mgimbal_control = qnode->gimbal_controller;
        emit sig_gimbol_focus_restore();//焦距归1
    }
    
    std::cout<<"qnode线程结束了"<<std::endl;
}

void MainWindow::slot_update_image(QImage im)
{
    QPixmap pixmap = QPixmap::fromImage(im);
    ui->label_display->setScaledContents(true);
    ui->label_display->setPixmap(pixmap.scaled(ui->label_display->size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::slot_update_GNSS_Message(std::string message)
{
    ui->plainTextEdit_GNSS->appendPlainText(QString::fromStdString(message));
}

void MainWindow::slot_update_GNSS_result(Gnss_result result)
{
    std::stringstream ss_message;
    ss_message << "当前GPS时间: " << result.gps_time<<"\n"
    << "模式 = " << result.mode << "\n"
    << "x滚转 = " << result.roll << "°\n"  <<"y俯仰 = " << result.pitch << "°\n" << "z航向 = " << result.yaw << "°\n"
    << "位置：北纬 = " << result.latitude << "°\n "<<"东经 = " << result.longitude << "°\n"<<"海拔 = " << result.altitude << "m\n" << "\n";
    ui->plainTextEdit_GNSS->appendPlainText(QString::fromStdString(ss_message.str()));
   
    ui->graphicsEADI->setRoll(result.roll);
    ui->graphicsEADI->setPitch(result.pitch);
    ui->graphicsEADI->setHeading(result.yaw);
    ui->graphicsEADI->setAltitude(result.altitude);
    ui->graphicsEADI->setAirspeed (result.speed);
}

void MainWindow::slot_update_gimbal_result(Gimbal_result result)
{
    ui->label_yaw->setText(QString::number(result.yaw));
    ui->label_roll->setText(QString::number(result.roll));
    ui->label_pitch->setText(QString::number(result.pitch));
    ui->graphicsHI->setHeading(result.yaw);
    ui->graphicsAI->setRoll(result.roll);
    ui->graphicsAI->setPitch(result.pitch);
}

void MainWindow::slot_update_air_pressure_message(std::string message)
{
    ui->plainTextEdit_barometric_altimeter->appendPlainText(QString::fromStdString(message));
}

void MainWindow::slot_update_airPressure_result(AirPressure_result result)
{
    //std::cout<<"ggggg"<<std::endl;
    std::string message;
    message = "大气静压: " + std::to_string(result.pressure_a) + " hPa  "
                //+ "标准气压高度: " + std::to_string(result.altitude_a) + " m  "
                //+ "爬升率: " + std::to_string(result.rate) + " m/s  "
                + "气压校正相对高度: " + std::to_string(result.altitude_b) + " m\n";
    //std::cout<<message<<std::endl;
    ui->plainTextEdit_barometric_altimeter->appendPlainText(QString::fromStdString(message));

    ui->graphicsALT->setAltitude(result.altitude_b/ 0.3048);//英尺（ft）和米（m）之间的换算
    ui->graphicsALT->setPressure(result.pressure_a/33.8639);//hPa（百帕）和inHg（英寸汞柱）换算
    
    update();

}

void MainWindow::slot_update_airPressure_status(std::string status)
{
    ui->plainTextEdit_barometric_altimeter->appendPlainText(QString::fromStdString(status));
}

void MainWindow::slot_update_radio_altitude_message(float message)
{
    ui->plainTextEdit_radio_altimeter->appendPlainText(QString::number(message));
}

void MainWindow::slot_update_radio_altitude_result(Radio_result result)
{
    std::cout<<result.altitude <<std::endl;
    if(result.altitude == -1)
    {
        ui->plainTextEdit_radio_altimeter->appendPlainText(QString::fromStdString("无效值"));
    }
    else
    {
        std::string str = result.gps_time + " " + std::to_string(result.altitude);
        ui->plainTextEdit_radio_altimeter->appendPlainText(QString::fromStdString(str));
    }
    
}

void MainWindow::slot_update_status_bar(QString message)
{
    ui->statusbar->clearMessage(); // 清除当前显示的消息
    ui->statusbar->showMessage(message, 10000); 
}

void MainWindow::slot_update_plainTextEdit_status(QString message)
{
    ui->plainTextEdit_status->appendPlainText(message);
}

void MainWindow::slot_update_label_image_num(int num)
{
    ui->label_image_num->setText(QString::number(num));
}

void MainWindow::slot_update_label_time(QString time)
{
    ui->label_time->setText(time);
}

void MainWindow::slot_action_GNSS_reload_triggred()
{
    runGNSSDrive();
}

void MainWindow::slot_tBn_zoom_up_clicked()
{
    if(qnode->connect_gimbal)//只在光电初始化成功后有效
    {
        thread_control->control_able = false;//不允许定时调控角度
        int focus =  ui->spinBox_focus->value();
        if(focus < 8)
        {
            ui->spinBox_focus->setValue(focus + 1);//更新界面上的焦距显示
        }
        //thread_control->mgimbal_control = qnode->gimbal_controller;
        thread_control->gps_time_str = qnode->gps_time_str;//传递时间

        //设置以下按钮不可用 避免误操作
        ui->btn_camera_apply->setEnabled(false);
        ui->btn_downward->setEnabled(false);
        ui->btn_center->setEnabled(false);
        ui->tBn_zoom_up->setEnabled(false);
        ui->tBn_zoom_down->setEnabled(false);

        emit sig_zoom_control(true);
        
    }
}

void MainWindow::slot_tBn_zoom_down_clicked()
{
    if(qnode->connect_gimbal)//只在光电初始化成功后有效
    {
        thread_control->control_able = false;//不允许定时调控角度
        int focus =  ui->spinBox_focus->value();
        if(focus > 1)
        {
            ui->spinBox_focus->setValue(focus - 1);
        }
        //thread_control->mgimbal_control = qnode->gimbal_controller;
        thread_control->gps_time_str = qnode->gps_time_str;//传递时间

        //设置以下按钮不可用 避免误操作
        ui->btn_camera_apply->setEnabled(false);
        ui->btn_downward->setEnabled(false);
        ui->btn_center->setEnabled(false);
        ui->tBn_zoom_up->setEnabled(false);
        ui->tBn_zoom_down->setEnabled(false);
        
        emit sig_zoom_control(false);
    }
}

//一键向下按钮对应的槽函数
void MainWindow::slot_btn_downward_clicked()
{
    if(qnode->connect_gimbal)//只在光电初始化成功后有效
    {
        //设置以下按钮不可用 避免误操作
        ui->btn_camera_apply->setEnabled(false);
        ui->btn_downward->setEnabled(false);
        ui->btn_center->setEnabled(false);
        ui->tBn_zoom_up->setEnabled(false);
        ui->tBn_zoom_down->setEnabled(false);
        thread_control->gps_time_str = qnode->gps_time_str;//传递时间

        emit sig_set_downward();//发出一键向下信号
    }
}

void MainWindow::slot_btn_center_clicked()
{
    if(qnode->connect_gimbal)//只在光电初始化成功后有效
    {
        //设置以下按钮不可用 避免误操作
        ui->btn_camera_apply->setEnabled(false);
        ui->btn_downward->setEnabled(false);
        ui->btn_center->setEnabled(false);
        ui->tBn_zoom_up->setEnabled(false);
        ui->tBn_zoom_down->setEnabled(false);
        thread_control->gps_time_str = qnode->gps_time_str;//传递时间

        emit sig_set_center();//发出一键回中信号
    }
}

void MainWindow::slot_gimbal_config_finished()
{
    ui->btn_camera_apply->setEnabled(true);
    ui->btn_downward->setEnabled(true);
    ui->btn_center->setEnabled(true);
    ui->tBn_zoom_up->setEnabled(true);
    ui->tBn_zoom_down->setEnabled(true);
    //设置完成后 需要重新建立文件夹 （已取消）
    //create_folder();
}

void MainWindow::slot_clear_message()
{
    ui->plainTextEdit_GNSS->clear();
    ui->plainTextEdit_barometric_altimeter->clear();
    ui->plainTextEdit_radio_altimeter->clear();
}

//已弃用
void MainWindow::slot_control_angle()
{
    if(qnode->connect_gimbal)
    {
        emit sig_angle_control(gimbal_para.pitch, gimbal_para.yaw);//后续优化 不需要一直计算
    }
}

void MainWindow::updateTime()
{
    // 获取当前时间
    QDateTime current = QDateTime::currentDateTime();
    // 将时间格式化为字符串
    QString timeString = current.toString("yyyy-MM-dd HH:mm:ss");
    // 更新QLabel显示的时间
    ui->label_time->setText(timeString);

    // if(qnode->connect_succeed)
    // {
    //     std::cout<<qnode->rtsp_capture->capture_num<<std::endl;
    // }

    // // 创建随机数生成器
    // std::random_device rd;  // 用于获取随机数种子
    // std::mt19937 gen(rd()); // 根据种子初始化Mersenne Twister生成器

    // // 定义浮点数范围
    // std::uniform_real_distribution<> dis(-90.0, 90.0);

    // // 生成随机浮点数
    // double randomFloat = dis(gen);

    // ui->graphicsEADI->setRoll(randomFloat);
    // ui->graphicsEADI->setPitch(randomFloat);
    // ui->graphicsEADI->setAltitude(randomFloat);
    // ui->graphicsHI->setHeading(randomFloat);
    // ui->graphicsAI->setRoll(randomFloat);
    // ui->graphicsAI->setPitch(randomFloat);
    // ui->graphicsALT->setAltitude(randomFloat);
    // ui->graphicsEADI->redraw();
    // ui->graphicsHI->redraw();
    // ui->graphicsAI->redraw();
    // ui->graphicsALT->redraw();

}

//GNSS按钮槽函数
void MainWindow::slot_rBtn_GNSS_clicked(bool checked)
{
    qnode->send_GNSS = checked;
}

//气压高度计槽函数
void MainWindow::slot_rBtn_barometric_altimeter_clicked(bool checked)
{
    qnode->send_barometric_altimeter = checked;
}

//无线电高度计槽函数
void MainWindow::slot_rBtn_radio_altimeter_clicked(bool checked)
{
    qnode->send_radio_altimeter = checked;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    QMainWindow::timerEvent(event);
    //刷新仪表
    ui->graphicsEADI->redraw();
    ui->graphicsHI->redraw();
    ui->graphicsAI->redraw();
    ui->graphicsALT->redraw();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    //update();
}


void MainWindow::slot_btn_scene_matching_clicked()
{
    ui->edit_longitude_scene->setText("0.0"); // 公开版界面占位，非测量坐标
    ui->edit_latitude_scene->setText("0.0"); // 公开版界面占位，非测量坐标
}

void MainWindow::slot_btn_terrain_matching_clicked()
{
    ui->edit_longitude_terrain->setText("0.0"); // 公开版界面占位，非测量坐标
    ui->edit_latitude_terrain->setText("0.0"); // 公开版界面占位，非测量坐标
}

void MainWindow::slot_btn_high_fusion_clicked()
{
    ui->edit_height->setText("50.653");
}   

void MainWindow::slot_action_serial_setting_triggred()
{
    if (!dlg_serial_setting->isVisible()) 
    {
        dlg_serial_setting->show(); // 如果对话框不可见，则显示它
    }
}

//创建文件夹
void MainWindow::create_folder()
{
    std::vector<std::string> photo_serial_path(2);

    /*一这个地方是为了吊舱状态更改时就新建文件夹，但考虑到吊舱发送指令延迟性，
    所以不考虑这个方法了，还是保持原来的：打开软件，建立一个文件夹*/
    // 获取当前时间
    // QDateTime current_time = QDateTime::currentDateTime();
    // // 将时间格式化为字符串
    // std::string time_string = current_time.toString("M-d-H-m-s").toStdString();
    //std::string folder_path = qnode->top_folder_path + "/" + time_string;
    // photo_serial_path[0] = folder_path + "/Photos";
    // photo_serial_path[1] = folder_path + "/SerialData";
    
    // //创建output文件夹 以"output+时间"命名
    // try {
    //     if (std::filesystem::create_directory(folder_path)) {
    //         //std::cout << "Folder created: " << folder_path << std::endl;
    //     } else {
    //         std::cout << "Folder already exists or cannot be created: " << folder_path << std::endl;
    //     }
    // } catch (const std::filesystem::filesystem_error& e) {
    //     std::cerr << "Error: " << e.what() << std::endl;
    // }
    /*-----------------------------------------------------------------------------------------*/
    std::string folder_path = qnode->top_folder_path;
    photo_serial_path[0] = folder_path + "/Photos";
    photo_serial_path[1] = folder_path + "/SerialData";

    //创建图片文件夹 以"Photos"命名
     try {
        if (std::filesystem::create_directory(photo_serial_path[0])) {
            //std::cout << "Folder created: " << photo_path << std::endl;
        } else {
            std::cout << "Folder already exists or cannot be created: " << photo_serial_path[0] << std::endl;
        }
    } 
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    //创建串口信息文件夹 以"SerialData"命名
     try {
        if (std::filesystem::create_directory(photo_serial_path[1])) {
            //std::cout << "Folder created: " << serial_path << std::endl;
        } else {
            std::cout << "Folder already exists or cannot be created: " << photo_serial_path[1] << std::endl;
        }
    } 
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    qnode->photo_serial_path = photo_serial_path;
}

//创建顶层文件夹
void MainWindow::create_top_folder()
{
    // 获取当前时间
    QDateTime current_time = QDateTime::currentDateTime();
    // 将时间格式化为字符串
    std::string time_string = current_time.toString("M-d-H-m-s").toStdString();

    std::string cutrrent_path = QDir::currentPath().toStdString();
    qnode->top_folder_path = cutrrent_path + "/output_" + time_string;

    //创建output文件夹 以"output+时间"命名
    try {
        if (std::filesystem::create_directory(qnode->top_folder_path)) {
            //std::cout << "Folder created: " << folder_path << std::endl;
        } else {
            std::cout << "Folder already exists or cannot be created: " << qnode->top_folder_path << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}