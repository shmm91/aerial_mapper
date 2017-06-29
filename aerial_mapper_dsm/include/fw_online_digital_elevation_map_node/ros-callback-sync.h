#ifndef ROS_CALLBACK_SYNC_H_
#define ROS_CALLBACK_SYNC_H_

#include <algorithm>
#include <memory>

#include <cv_bridge/cv_bridge.h>
#include <geometry_msgs/PoseStamped.h>
#include <glog/logging.h>
#include <image_transport/image_transport.h>
#include <message_filters/subscriber.h>
#include <message_filters/time_synchronizer.h>
#include <minkindr_conversions/kindr_msg.h>
#include <multiagent-mapping-common/buffered-lookup.h>
#include <opencv2/highgui/highgui.hpp>
#include <ros/ros.h>
#include <sensor_msgs/Image.h>

//#include "fw_online_planar_rectification_node/fw-online-planar-rectification.h"
#ifdef adsf
class RosCallbackSync {
public:
  RosCallbackSync()
    : image_transport_(node_handle_),
      sub_pose_sync_(node_handle_, "/fw_pose", 1),
      sub_image_sync_(node_handle_, "/fw_image", 1),
      sync_(sub_image_sync_, sub_pose_sync_, 100) {
    VLOG(0) << "start";
    const Eigen::Vector3d origin(464980, 5.27226e+06, 414.087);
    std::string ncameras_yaml_path_filename = "/home/timo/calibration/camera_fixed_wing.yaml";
    online_planar_rectification_.reset(new FwOnlinePlanarRectification(ncameras_yaml_path_filename,
                                                                       origin));
  }

  void runAndJoin() {
    ros::spin();
    ros::waitForShutdown();
  }

  void registerSubscriberAndPublisher() {
    sync_.registerCallback(boost::bind(&RosCallbackSync::syncCallback, this, _1, _2));
  }

 private:
  void syncCallback(const sensor_msgs::ImageConstPtr& image_message,
                    const geometry_msgs::PoseStampedConstPtr& pose_message) {
    VLOG(1) << "New synced message with seq: " << image_message->header.seq;

    // Get the image.
    cv_bridge::CvImageConstPtr cv_ptr;
    try {
      // Convert the image to MONO8 if necessary.
      cv_ptr = cv_bridge::toCvShare(image_message, sensor_msgs::image_encodings::MONO8);
    } catch(const cv_bridge::Exception& e) {
      LOG(FATAL) << "cv_bridge exception: " << e.what();
    }
    CHECK(cv_ptr);

    const cv::Mat& image  = cv_ptr->image.clone();
    kindr::minimal::QuatTransformation T_G_B;
    tf::poseMsgToKindr(pose_message->pose, &T_G_B);
    online_planar_rectification_->addFrame(T_G_B, image);
  }

  ros::NodeHandle node_handle_;
  image_transport::ImageTransport image_transport_;
  message_filters::Subscriber<geometry_msgs::PoseStamped> sub_pose_sync_;
  message_filters::Subscriber<sensor_msgs::Image> sub_image_sync_;
  message_filters::TimeSynchronizer<sensor_msgs::Image, geometry_msgs::PoseStamped> sync_;
  //std::unique_ptr<FwOnlinePlanarRectification> online_planar_rectification_;
};
#endif
#endif
