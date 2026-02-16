#pragma once

#include <string>
#include <memory>
#include <chrono>

#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/set_bool.hpp"
#include "talker_interfaces/srv/talk.hpp"
#include "listener_interfaces/srv/listen.hpp"

class HRI
{
public:
    explicit HRI(const rclcpp::Node::SharedPtr& node);

    bool Say(const std::string& text);

    std::string Listen(const std::string& grammar);

    bool FollowPerson(bool enable);

private:
    rclcpp::Node::SharedPtr node;

    rclcpp::Client<talker_interfaces::srv::Talk>::SharedPtr talkerClient;
    rclcpp::Client<listener_interfaces::srv::Listen>::SharedPtr listenerClient;
    rclcpp::Client<std_srvs::srv::SetBool>::SharedPtr followerClient;
};

