#pragma once

#include <string>
#include <chrono>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "talker_interfaces/srv/talk.hpp"
#include "listener_interfaces/srv/listen.hpp"

struct TalkResult
{
    bool success;
    std::string message;
};

class HRI
{
public:
    explicit HRI(const rclcpp::Node::SharedPtr& node);

    TalkResult Say(const std::string& text);
    std::string listen(const std::string& grammar);

private:
    bool WaitForService(
        const rclcpp::ClientBase::SharedPtr& client,
        const std::chrono::milliseconds& timeout) const;

private:
    rclcpp::Node::SharedPtr node_;

    rclcpp::Client<talker_interfaces::srv::Talk>::SharedPtr talker_client_;
    rclcpp::Client<listener_interfaces::srv::Listen>::SharedPtr listener_client_;
};
