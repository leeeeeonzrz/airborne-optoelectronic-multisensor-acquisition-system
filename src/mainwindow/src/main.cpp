#include <ros/ros.h>
#include "../include/mainwindow.h"
#include <QApplication>
#include <ctime>
#include <QProcess>
#include <QFile>

int main(int argc, char *argv[])
{

    system("gnome-terminal -- bash -c 'roscore'");
    // 使用QProcess启动gnome-terminal
    // QProcess terminalProcess;
    // QString command = "gnome-terminal -- bash -c 'roscore'";
    // terminalProcess.start(command);
    //   // 获取终端进程的PID
    // qint64 terminalPID = terminalProcess.processId();

    sleep(2);
    QApplication a(argc, argv);

        // 加载资源文件中的QSS样式表
    QFile file("../../../src/mainwindow/resources/style.qss");
    if (file.exists()) {
        //std::cout<<"11111"<<std::endl;
        file.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(file.readAll());
        qApp->setStyleSheet(styleSheet);
        file.close();
    }

    setlocale(LC_ALL, "");

    ros::init(argc, argv, "window_control");
    MainWindow w(argc, argv);
    w.show();
   
    return a.exec();
    
}