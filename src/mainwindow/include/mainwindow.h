#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QProcess>
#include <QThread>
#include <QTimer>
#include <QResizeEvent>
#include "qnode.h"
#include "thread_control.h"
#include "dlgSerialSetting.h"


namespace Ui {
class MainWindow;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(int argc, char** argv, QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    //连接
    void slot_action_connect_triggred();
    //连接成功 主要是为了给控制线程赋值
    void slot_connect_succeed();
    //保存
    void slot_action_save_triggred();
    //回放
    //void slot_action_open_triggred();
    //播放
    //void slot_action_play_triggred();
    //GNSS
    void slot_rBtn_GNSS_clicked(bool checked);
    //气压高度计
    void slot_rBtn_barometric_altimeter_clicked(bool checked);
    //无线电高度计
    void slot_rBtn_radio_altimeter_clicked(bool checked);
    //吊舱参数应用
    void slot_btn_camera_apply_clicked();
    //变焦值改变
    void slot_spinBox_focus_valueChanged(int value);
    //相机模式改变:可见光/红外
    void slot_comboBox_camera_mode_currentIndexChanged(int value);
    //qnode 线程结束
    void slot_qnode_finished();
    //ui图片更新
    void slot_update_image(QImage im);
    //GNSS消息更新
    void slot_update_GNSS_Message(std::string message);//已弃用
    void slot_update_GNSS_result(Gnss_result result);
    //更新吊舱信息仪表
    void slot_update_gimbal_result(Gimbal_result result);
    //气压高度计信息显示
    void slot_update_air_pressure_message(std::string message);//已弃用
    void slot_update_airPressure_result(AirPressure_result result);
    void slot_update_airPressure_status(std::string status);
    //无线电高度计信息显示
    void slot_update_radio_altitude_message(float message);//已弃用
    void slot_update_radio_altitude_result(Radio_result result);
    //状态栏更新
    void slot_update_status_bar(QString message);
    //消息框更新
    void slot_update_plainTextEdit_status(QString message);
    //更新图数量
    void slot_update_label_image_num(int num);
    //更新时间标签
    void slot_update_label_time(QString time);
    //重新运行GNSS驱动
    void slot_action_GNSS_reload_triggred();
    //可见光变焦+
    void slot_tBn_zoom_up_clicked();
    //可见光变焦-
    void slot_tBn_zoom_down_clicked();
    //吊舱控制结束
    void slot_gimbal_config_finished();
    //清空界面上的plainTextEdit
    void slot_clear_message();
    //定时控制角度(1min更新1次)  已弃用
    void slot_control_angle();
    //景象匹配计算按钮
    void slot_btn_scene_matching_clicked();
    //地形匹配计算按钮
    void slot_btn_terrain_matching_clicked();
    //高度融合计算按钮
    void slot_btn_high_fusion_clicked();
    //一键向下按钮
    void slot_btn_downward_clicked();
    //一键回中按钮
    void slot_btn_center_clicked();
    //串口设置按钮
    void slot_action_serial_setting_triggred();


    //显示系统时间
    void updateTime();

signals:
    void closed(); // 定义一个信号
    void sig_start_qnode();
    void sig_start_control();//设置吊舱参数
    void sig_zoom_control(bool up);//单步调焦
    void sig_gimbol_focus_restore();//焦距回1
    void sig_angle_control(int pitch, int yaw);//调控角度
    void sig_set_downward();//一键向下
    void sig_set_center();//一键回中


protected:
    void closeEvent(QCloseEvent *event) override;
    void timerEvent(QTimerEvent *event);
    void resizeEvent(QResizeEvent *event) override;

private:
    //运行GNSS驱动
    void runGNSSDrive();
    //创建文件夹
    void create_folder();
    //创建顶层文件夹
    void create_top_folder();

private:
    Ui::MainWindow *ui;
    int timerId;
    //打开gnss驱动进程
    QProcess *process_gnss;
    //Ros节点文件
    QNode* qnode = nullptr;
    QThread qthread_node;
    //控制线程
    ThreadControl* thread_control = nullptr;
    QThread qthread_control;
    int focus;//变换相机模式时的上一个模式的焦距

    bool first_connect;//是否是第一次点击连接的标志位
    
    //吊舱控制参数
    gimbal_config gimbal_para;

    //计时器
    QTimer *timer;//更新界面时间
    //QTimer *timer_control;//更新角度 （可能会有问题）

    //串口设置对话框
    DlgSerialSetting *dlg_serial_setting;


};
#endif // MAINWINDOW_H