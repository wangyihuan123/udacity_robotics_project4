#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot

    ROS_INFO_STREAM("Drive the robot");

    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the command_robot service and pass the requested joint angles
    if (!client.call(srv))
        ROS_ERROR("Failed to call service command_robot");

}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    bool stop = true;

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    int x = 0;
    int left_threshold = (img.step / 10) * 4;
    int right_threshold = (img.step / 10) * 6;

    // check middle first
    for (int i = left_threshold; i < img.height * img.step; i += 3) {
        x = i % img.step;
        if (x > left_threshold && x < right_threshold) { 
            //middle
            if (img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel) {
                stop = false;
                ROS_INFO_STREAM("middle, go ahead!");
                drive_robot(0.3, 0.0);
                return;
            }
        } else if (x < left_threshold) {
            // left side
            continue;
        } else if (x > right_threshold) {
            // at the right side
            i += (img.step / 10) * 8;
        }
    }

    // then right side
    for (int i = right_threshold; i < img.height * img.step; i += 3) {
        x = i % img.step;
        if ( x == 0) { 
            i += (img.step / 10) * 6;
        } else  {
            // at the right side
            if (img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel) {
                stop = false;
                ROS_INFO_STREAM("right side, turn right!");
                drive_robot(0.0, -0.3);
                return;
            } 
        }
    }

    // then left side
    for (int i = 0; i < img.height * img.step; i += 3) {
        x = i % img.step;
        if ( x < left_threshold) { 
            // at the left side
            if (img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel) {
                stop = false;
                ROS_INFO_STREAM("left side, turn left!");
                drive_robot(0.0, 0.3);
                return;
            } 
        } else {
            i += (img.step / 10) * 6;          
        }
    }

    if (stop == true) {
        drive_robot(0.0, 0.0);
        return;
    } 
        
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}