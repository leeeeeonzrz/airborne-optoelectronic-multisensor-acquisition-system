#include "acquisition/synchronizer.hpp"
#include <QApplication>
#include <QLabel>
#include <QObject>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

class SyntheticWorker : public QObject {
    Q_OBJECT
public slots:
    void start() {
        timer_ = new QTimer(this); // Construct timer in its owning worker thread.
        connect(timer_, &QTimer::timeout, this, [this] {
            const auto time = static_cast<acquisition::TimeNs>(sequence_) * 100'000'000;
            assembler_.ingest({"imu", sequence_, time, time, true, {0,0,0}});
            auto snapshot = assembler_.assemble(time, sequence_++);
            emit status(QString("Synthetic frame %1 / %2 observations")
                        .arg(qulonglong(snapshot.sequence)).arg(qulonglong(snapshot.observations.size())));
        });
        timer_->start(100);
    }
    void stop() { if (timer_) timer_->stop(); }
signals:
    void status(QString text);
private:
    QTimer* timer_{nullptr};
    std::uint64_t sequence_{0};
    acquisition::SnapshotAssembler assembler_{{{"imu"}, 10'000'000, 16}};
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QWidget window;
    auto* layout = new QVBoxLayout(&window);
    auto* label = new QLabel("Waiting for synthetic samples", &window);
    layout->addWidget(label);
    QThread thread;
    auto* worker = new SyntheticWorker;
    worker->moveToThread(&thread);
    QObject::connect(&thread, &QThread::started, worker, &SyntheticWorker::start);
    QObject::connect(worker, &SyntheticWorker::status, label, &QLabel::setText);
    QObject::connect(&thread, &QThread::finished, worker, &QObject::deleteLater);
    QObject::connect(&app, &QApplication::aboutToQuit, [&] {
        QMetaObject::invokeMethod(worker, "stop", Qt::BlockingQueuedConnection);
        thread.quit(); thread.wait();
    });
    thread.start();
    window.resize(480, 160); window.show();
    return app.exec();
}
#include "status_window.moc"
