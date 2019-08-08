#include <iostream>

#include <cstdlib>
#include <random>

#include "ros/ros.h"
#include "sensor_msgs/LaserScan.h"
#include "geometry_msgs/Twist.h"


class ReactiveController
{
private:
    ros::NodeHandle n;
    ros::Publisher cmd_vel_pub;
    ros::Subscriber laser_sub;

    float obstacle_distance;
    float obstacle_distance_left;
    float obstacle_distance_front;
    float obstacle_distance_right;
    bool robot_stopped;
    bool right_before;

    geometry_msgs::Twist calculateCommand()
    {
        auto msg = geometry_msgs::Twist();

        if(obstacle_distance > 0.8){
            // random
            int random_number = rand() % 15 + 1 ; // random number between 1 and 15
            if(random_number==5){
                msg.angular.z = 10; 
                msg.linear.x = 0.0;
            }
            else if (random_number==4){
                msg.angular.z = -10; 
                msg.linear.x = 0.0;
            }
            else {
                msg.linear.x = 1.0;
                msg.angular.z = 0.0;
            }
        }
        else if ((obstacle_distance_right < 0.8) && (obstacle_distance_front < 0.8)){
            msg.linear.x = 0.0;
	    if (right_before){
		msg.angular.z = -10.0;
	        right_before = true;
	    }
	    else{
		msg.angular.z = 10.0;
		right_before = false;
	    }
        }
        else if ((obstacle_distance_left < 0.8) && (obstacle_distance_front < 0.8)){
            msg.linear.x = 0.0;
	    if (!right_before){
		msg.angular.z = 10.0;
		right_before = false;
	    }
	    else{
		msg.angular.z = -10.0;
	        right_before = true;
	    }
        }
        else if (obstacle_distance_left < 0.8){
            msg.linear.x = 0.0;
            msg.angular.z = -10.0;
        }
        else if (obstacle_distance_right < 0.8){
            msg.linear.x = 0.0;
            msg.angular.z = 10.0;
        }
        else {
	    ROS_INFO("else                                 ");
            int random_angular_number = rand() % 2 + 1 ; // random number between 1 and 2
            if (random_angular_number==1){
                msg.linear.x = 0.0;
                msg.angular.z = 5.0;
            }
            else {
                msg.linear.x = 0.0;
                msg.angular.z = -5.0;                
            }
        }
        return msg;
    }

    void laserCallback(const sensor_msgs::LaserScan::ConstPtr& msg)
    {
        obstacle_distance_right = *std::min_element(msg->ranges.begin(),msg->ranges.begin()+80);
        obstacle_distance_front = *std::min_element(msg->ranges.begin()+81,msg->ranges.begin()+160.0);
        obstacle_distance_left = *std::min_element(msg->ranges.begin()+160.0,msg->ranges.end());
        obstacle_distance = *std::min_element(msg->ranges.begin(),msg->ranges.end()); 


    }


public:
    ReactiveController(){
        // Initialize ROS
        this->n = ros::NodeHandle();

        // Create a publisher object, able to push messages
        this->cmd_vel_pub = this->n.advertise<geometry_msgs::Twist>("cmd_vel", 5);

        // Create a subscriber for laser scans 
        this->laser_sub = n.subscribe("base_scan", 10, &ReactiveController::laserCallback, this);

    }

    void run(){
        // Send messages in a loop
        ros::Rate loop_rate(10);
        while (ros::ok())
        {
            // Calculate the command to apply
            auto msg = calculateCommand();

            // Publish the new command
            this->cmd_vel_pub.publish(msg);

            ros::spinOnce();

            // And throttle the loop
            loop_rate.sleep();
        }
    }

};


int main(int argc, char **argv){
    // Initialize ROS
    ros::init(argc, argv, "reactive_controller");

    // Create our controller object and run it
    auto controller = ReactiveController();
    controller.run();

    // And make good on our promise
    return 0;
}
