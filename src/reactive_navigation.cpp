#include "ros/ros.h"
#include "sensor_msgs/LaserScan.h"
#include "geometry_msgs/Twist.h"
#include <algorithm>

class ReactiveNavigation {
public:
    ReactiveNavigation() {
        // Initialize the publisher for cmd_vel
        cmd_vel_pub = n.advertise<geometry_msgs::Twist>("cmd_vel", 100);
        // Initialize the subscriber for base_scan topic
        laser_sub = n.subscribe("base_scan", 100, &ReactiveNavigation::laserCallback, this);
        // Set initial distances
        right_distance = 0.1;
        left_distance = 0.1;
    }

    void run() {
        ros::Rate loop_rate(10); // Set the loop rate to 10 Hz
        while (ros::ok()) {
            geometry_msgs::Twist cmd_vel_msg;
            double mean = (right_distance + left_distance) / 2; // Calculate the mean between both distances

            if (left_distance < 0.4) {
                // If close to the left wall, slowdown and turn right
                cmd_vel_msg.linear.x = 0.1;
                cmd_vel_msg.angular.z = -0.5;
                if (left_distance < 0.2) {
                    // If too close to the left wall, stop and turn right
                    cmd_vel_msg.linear.x = 0.0;
                    cmd_vel_msg.angular.z = -0.5;
                }
            } else {    
                // Set the linear and angular velocities to follow the left wall
                cmd_vel_msg.linear.x = mean; 
                cmd_vel_msg.angular.z = mean * (2 - left_distance); 
            }

            cmd_vel_pub.publish(cmd_vel_msg); // Publish the velocity command
            ros::spinOnce(); // Call any pending callbacks
            loop_rate.sleep(); // Sleep for the rest of the cycle
        }
    }

private:
    ros::NodeHandle n; 
    ros::Publisher cmd_vel_pub; 
    ros::Subscriber laser_sub; 
    double left_distance;
    double right_distance; 

    void laserCallback(const sensor_msgs::LaserScan::ConstPtr& msg) {
        ROS_INFO("Received a laser scan with %zu samples", msg->ranges.size());

        int middle = msg->ranges.size() / 2;

        // Find the minimum distance in the left half of the scan
        left_distance = *std::min_element(msg->ranges.begin() + middle, msg->ranges.end());
        // Find the minimum distance in the right half of the scan
        right_distance = *std::min_element(msg->ranges.begin(), msg->ranges.begin() + middle);

        ROS_INFO("Left Distance: %f", left_distance);
        ROS_INFO("Right Distance: %f", right_distance);
    }
};

int main(int argc, char **argv) {
    ros::init(argc, argv, "reactive_navigation"); // Initialize the ROS node
    ReactiveNavigation reactive_nav; // Create an instance of the ReactiveNavigation class
    reactive_nav.run(); // Enter the main loop
    return 0;
}
