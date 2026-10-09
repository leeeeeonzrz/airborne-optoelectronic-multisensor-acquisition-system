#include "acquisition/synchronizer.hpp"
#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/NavSatFix.h>
#include <std_msgs/String.h>
#include <sstream>

class SnapshotNode {
public:
    SnapshotNode() : private_("~"), assembler_({{"imu", "position"}, 50'000'000, 64}) {
        std::string imu_topic, position_topic;
        private_.param<std::string>("imu_topic", imu_topic, "imu/data");
        private_.param<std::string>("position_topic", position_topic, "position/fix");
        output_ = node_.advertise<std_msgs::String>("snapshot/status", 10);
        imu_ = node_.subscribe(imu_topic, 40, &SnapshotNode::on_imu, this);
        position_ = node_.subscribe(position_topic, 40, &SnapshotNode::on_position, this);
    }
private:
    void on_position(const sensor_msgs::NavSatFix::ConstPtr& msg) {
        assembler_.ingest({"position", msg->header.seq,
            static_cast<acquisition::TimeNs>(msg->header.stamp.toNSec()),
            static_cast<acquisition::TimeNs>(ros::Time::now().toNSec()),
            msg->status.status >= 0, {msg->latitude, msg->longitude, msg->altitude}});
    }
    void on_imu(const sensor_msgs::Imu::ConstPtr& msg) {
        const auto time = static_cast<acquisition::TimeNs>(msg->header.stamp.toNSec());
        assembler_.ingest({"imu", msg->header.seq, time,
            static_cast<acquisition::TimeNs>(ros::Time::now().toNSec()), true,
            {msg->orientation.w, msg->orientation.x, msg->orientation.y, msg->orientation.z}});
        auto snapshot = assembler_.assemble(time, msg->header.seq);
        std_msgs::String status;
        status.data = snapshot.complete() ? "complete" : "partial";
        output_.publish(status);
    }
    ros::NodeHandle node_, private_;
    acquisition::SnapshotAssembler assembler_;
    ros::Subscriber imu_, position_;
    ros::Publisher output_;
};
int main(int argc, char** argv) {
    ros::init(argc, argv, "snapshot_showcase");
    SnapshotNode node;
    ros::spin();
}
