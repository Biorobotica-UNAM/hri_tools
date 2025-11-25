#pragma once

#include <string>
#include <mutex>
#include <memory>
#include <chrono>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp/qos.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"
#include "listener_interfaces/srv/listen.hpp"

class HRI
{
private:
    rclcpp::Node::SharedPtr node_;
    
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_speech_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_speech_status_;
    
    rclcpp::Client<listener_interfaces::srv::Listen>::SharedPtr client_recog_;
    
    std::mutex mtx_;
    bool speech_done_;

    void SpeechStatusCallback(const std_msgs::msg::Bool::SharedPtr msg);
    
public:
    HRI(rclcpp::Node::SharedPtr node);
    
    void Say(const std::string& text);
    std::string Listen(const std::string& grammar);
};
