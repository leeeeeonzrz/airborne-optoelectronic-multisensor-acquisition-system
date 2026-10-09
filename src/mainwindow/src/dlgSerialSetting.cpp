#include "../include/dlgSerialSetting.h"
#include "ui_dlgSerialSetting.h"
#include "dlgSerialSetting.h"
#include <iostream>
#include <QDir>

DlgSerialSetting::DlgSerialSetting(QWidget *parent)
:QDialog(parent),ui(new Ui::DialogSerialSetting)
{
    ui->setupUi(this); 

    config_path = QDir::currentPath().toStdString() + "/config_fly.yaml";
    gnss_config_path = qEnvironmentVariable("ACQUISITION_GNSS_CONFIG", "./src/sbg_ros_driver/config/example/ellipse_N_default.yaml").toStdString();
    connect(ui->btn_cancle,SIGNAL(clicked()),this,SLOT(slot_btn_cancle_clicked()));
    connect(ui->btn_apply,SIGNAL(clicked()),this,SLOT(slot_btn_apply_clicked()));
}

DlgSerialSetting::~DlgSerialSetting()
{
    delete ui;
}

void DlgSerialSetting::showEvent(QShowEvent *event)
{
    find_serial();
    update_combobox();
    QDialog::showEvent(event);
}

void DlgSerialSetting::slot_btn_apply_clicked()
{
    std::string gimabl_serial = ui->cBox_gimabl->currentText().toStdString();
    std::cout<<gimabl_serial<<std::endl;
    yaml_config.writeYAML(config_path);
}

void DlgSerialSetting::slot_btn_cancle_clicked()
{
    this->close();
}

void DlgSerialSetting::update_combobox()
{
    for (const std::string& str : serial_name) {
        ui->cBox_gimabl->addItem(QString::fromStdString(str));
        ui->cBox_gnss->addItem(QString::fromStdString(str));
        ui->cBox_airpressure->addItem(QString::fromStdString(str));
        ui->cBox_radio->addItem(QString::fromStdString(str));
    }
}

void DlgSerialSetting::find_serial()
{
    struct udev *udev = udev_new();
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
 
    // 枚举所有串口设备
    udev_enumerate_add_match_subsystem(enumerate, "tty");
    udev_enumerate_scan_devices(enumerate);
 
    struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
    struct udev_list_entry *entry;
 
    // 打印每个设备的路径
    udev_list_entry_foreach(entry, devices) {
        const char *path = udev_list_entry_get_name(entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);

        if (dev) {
            const char *devnode = udev_device_get_devnode(dev);
            const char *subsystem = udev_device_get_subsystem(dev);

            // 假设硬件串口以ttyUSB或ttyACM开头
            if (devnode && subsystem && (strncmp(devnode, "/dev/ttyUSB", 11) == 0 || strncmp(devnode, "/dev/ttyCH343USB", 16) == 0)) {
                std::cout << devnode << std::endl;
                serial_name.push_back(devnode);
            }

            udev_device_unref(dev);
        }
    }
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
}