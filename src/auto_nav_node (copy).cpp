#include <ros/ros.h>
#include <ros/master.h>
#include <std_msgs/Int32.h>
#include <std_msgs/String.h>
#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>

// elevator_status : 로봇 상태 구분 - 'navi_mode', 'elevator_on', 'elevator_off'로 나누어 가정함
// elevator_status가 'navi_mode'인 경우에 한하여 'elvator_floor'를 입력받으면 그에 따라 navigation에 필요한 map.pgm, map.yaml을 변경
// 자동실행 후 elevator_status가 'elevator_on' 또는 'elevator_off'라면 실행중인 navigation package 종료

// launch 수정 방법 1. ros::param::set 2. 직접 수정 3. gnome-terminal 입력 시 argument 추가 

std_msgs::String reach_goal_msg;
ros::Publisher reach_goal_pub;

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

ros::NodeHandle *nh_ptr;
//MoveBaseClient *ac_ptr;

bool nav_mode = false;
bool nav_running = false;

void move_goal(int floor, double x, double y, double o_z, double o_w, double z = 0.0, double o_x = 0.0, double o_y = 0.0)
{
    MoveBaseClient ac("move_base", true);
    ROS_INFO("nav_mode : %d", nav_mode);
    //while(!ac.waitForServer(ros::Duration(5.0))){
    //	ROS_INFO("Waiting for the move_base action server to come up");
    //}
    
    move_base_msgs::MoveBaseGoal goal;
 
    goal.target_pose.header.frame_id = "map";
    goal.target_pose.header.stamp = ros::Time::now();
    
    goal.target_pose.pose.position.x = x;
    goal.target_pose.pose.position.y = y;
    goal.target_pose.pose.position.z = z;
    
    goal.target_pose.pose.orientation.x = o_x;
    goal.target_pose.pose.orientation.y = o_y;
    goal.target_pose.pose.orientation.z = o_z;
    goal.target_pose.pose.orientation.w = o_w;

    ROS_INFO("Sending goal...");
    
    if (nav_mode){
    ac.sendGoal(goal);
    
    ac.waitForResult();
    
    ROS_INFO("floor : %d", floor);
    
    if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
    {
        ROS_INFO("Arrived at the goal location");
        ROS_INFO("state : %d", ac.getState());
        // Publish the appropriate message based on the goal location
        if (floor == 1)
        {
            reach_goal_msg.data = "reach_1_goal";
            reach_goal_pub.publish(reach_goal_msg);
        }
        else if (floor == 5)
        {
            reach_goal_msg.data = "reach_5_goal";
            reach_goal_pub.publish(reach_goal_msg);
        }
    }
    else
    {
        ROS_INFO("Failed to reach the goal location");
    }
    }
    ROS_INFO("HI");
}


void initial_pose(double x, double y, double o_z, double o_w, double z = 0.0, double o_x = 0.0, double o_y = 0.0)
{
    ros::Publisher initial_pose_pub = nh_ptr->advertise<geometry_msgs::PoseWithCovarianceStamped>("/initialpose", 1, true);

    ros::Duration(1.0).sleep();

    geometry_msgs::PoseWithCovarianceStamped initial_pose_msg;
    initial_pose_msg.header.stamp = ros::Time::now();
    initial_pose_msg.header.frame_id = "map";

    initial_pose_msg.pose.pose.position.x = x;
    initial_pose_msg.pose.pose.position.y = y;
    initial_pose_msg.pose.pose.position.z = z;

    initial_pose_msg.pose.pose.orientation.x = o_x;
    initial_pose_msg.pose.pose.orientation.y = o_y;
    initial_pose_msg.pose.pose.orientation.z = o_z;
    initial_pose_msg.pose.pose.orientation.w = o_w;

    initial_pose_pub.publish(initial_pose_msg);

    ROS_INFO("Initial pose published.");
}


void floorCallback(const std_msgs::Int32::ConstPtr& msg)
{
/*
    if (nav_mode && !nav_running) {
        int floor = msg->data;
        
        std::string map_yaml = "map_" + std::to_string(floor) + ".yaml";
        std::string nav_arg = "map_file:=$(find omo_r1_navigation)/maps/" + map_yaml;
        
        std::string pre_exe_node = "/robot_state_publisher";
        bool pre_running = ros::service::exists(pre_exe_node, true);
        if (!pre_running){
        	std::string command = "gnome-terminal -- roslaunch omo_r1_navigation omo_r1_navigation.launch";
        	const char *c_command = command.c_str();
        	system(c_command);
        	
        	nav_running = true;
        } else{
        	ROS_WARN_STREAM("The node '" << pre_exe_node << "' is already running.");
        }
    }
*/
    if (nav_running) {
    	ROS_WARN_STREAM("Navigation launch already run");
    	return;
    }
    // nav_mode 조건 추가할지는 나중에 합치면서 생각	
    int floor = msg->data;
        
    //std::string map_yaml = "map_" + std::to_string(floor) + ".yaml";
    std::string map_yaml = "map_" + std::to_string(floor);
    ROS_INFO("yaml_name : %s",map_yaml.c_str());
    //std::string nav_launch_path = "catkin_ws/src/omo_r1/omo_r1_navigation/launch/omo_r1_navigation.launch";
    std::string nav_launch_path = "catkin_ws/src/omo_r1mini/omo_r1mini_navigation/launch/omo_r1mini_navigation.launch";
    ROS_INFO("launch_path : %s",nav_launch_path.c_str());
    //std::string temp_launch_path = "catkin_ws/src/omo_r1/omo_r1_navigation/launch/omo_r1_navigation_temp.launch";
    std::string temp_launch_path = "catkin_ws/src/omo_r1mini/omo_r1mini_navigation/launch/omo_r1mini_navigation_temp.launch";
    
    //std::string nav_arg = "map_file:=$(find omo_r1_navigation)/maps/" + map_yaml;
    //std::string map_yaml = "catkin_ws/src/omo_r1/omo_r1_navigation/maps/map_yaml";
    //std::string nav_arg = "map_file:=$(find omo_r1_navigation)/maps/" + map_yaml;
    
    std::ifstream launch_file(nav_launch_path);
    if (!launch_file.is_open()) {
        ROS_ERROR_STREAM("launch file open fail");
        return;
    }

    std::ofstream temp_file(nav_launch_path + "_temp");
    if (!temp_file.is_open()) {
        ROS_ERROR_STREAM("create temp file fail");
        launch_file.close();
        return;
    }

    std::string line;
    while (std::getline(launch_file, line)) {
        //size_t pos = line.find("<arg name=\"map_file\" default=\"$(find omo_r1_navigation)/maps/map_");
        size_t pos = line.find("<arg name=\"map_file\" default=\"$(find omo_r1mini_navigation)/maps/map_");
        if (pos != std::string::npos) {
            size_t end_pos = line.find(".yaml\"/>", pos);
            if (end_pos != std::string::npos) {
                std::string new_line = line.substr(0, pos);
                //new_line += "<arg name=\"map_file\" default=\"$(find omo_r1_navigation)/maps/" + map_yaml + ".yaml\"/>";
                new_line += "<arg name=\"map_file\" default=\"$(find omo_r1mini_navigation)/maps/" + map_yaml + ".yaml\"/>";
                temp_file << new_line << std::endl;
                continue;
            }
        }
        temp_file << line << std::endl;
    }

    launch_file.close();
    temp_file.close();

    if (std::rename((nav_launch_path + "_temp").c_str(), nav_launch_path.c_str()) != 0) {
        ROS_ERROR_STREAM("temp file rename fail");
        return;
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////
    std::string pre_exe_node = "/robot_state_publisher";
    bool pre_running = ros::service::exists(pre_exe_node, true);
    if (!pre_running){
        //std::string command = "gnome-terminal -- roslaunch omo_r1_navigation omo_r1_navigation.launch" + nav_arg;
        //std::string command = "gnome-terminal -- roslaunch omo_r1_navigation omo_r1_navigation.launch";
        std::string command = "gnome-terminal -- roslaunch omo_r1mini_navigation omo_r1mini_navigation.launch";
        const char *c_command = command.c_str();
        system(c_command);
        	
        nav_running = true;
    } else{
        ROS_WARN_STREAM("The node '" << pre_exe_node << "' is already running.");
    }
    
    //////// /move_base/goal로 이동할 위치 지정하기
    if (floor == 1){ 
    	initial_pose(44.19002151489258, 19.193927764892578, -0.07071067966408575, 0.7071067657322372);
    	move_goal(floor, 0.1711464524269104, -0.14070641994476318, 0.9999847388240346, 0.005524682708284404);
    } else if (floor == 5){
    	initial_pose(-0.4041336476802826, 0.10317420959472656, -0.01094703955525251, 0.9999400793672467);
    	move_goal(floor, 4.172542095184326, -27.017969131469727, 0.998694503924573, 0.051081188620184625);
    } else{
    	ROS_WARN_STREAM("Invalid floor");
        return;
    } // 이따 테스트 하면서 값 확인하기
}


void elevatorCallback(const std_msgs::String::ConstPtr& msg)
{
    std::string status = msg->data;
    if (status == "navi_mode") {
        nav_mode = true;
    } else {
        nav_mode = false;
        if (nav_running) {
        	//ros::shutdown();
        	system("rosnode kill robot_state_publisher");
        	system("rosnode kill map_server");
        	system("rosnode kill amcl");
        	system("rosnode kill joint_state_publisher");
        	system("rosnode kill move_base");
        	nav_running = false;
        }
    }
}

int main(int argc, char **argv)
{
    ros::init(argc, argv, "navigation_controller");
    ros::NodeHandle nh;
    nh_ptr = &nh;
    
    reach_goal_pub = nh.advertise<std_msgs::String>("/reach_goal", 10);

    ros::Subscriber floor_sub = nh.subscribe("/elevator_floor", 1, floorCallback);
    ros::Subscriber elevator_sub = nh.subscribe("/Control_mode", 1, elevatorCallback);

    ros::spin();

    return 0;
}

