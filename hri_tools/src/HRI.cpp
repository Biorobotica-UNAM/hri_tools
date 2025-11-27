#include "hri_tools/HRI.hpp"

using namespace std::chrono_literals;

HRI::HRI(const rclcpp::Node::SharedPtr& node)
    : node_(node)
{
    talker_client_ = node_->create_client<talker_interfaces::srv::Talk>("/HRI/talker/talk");
    listener_client_ = node_->create_client<listener_interfaces::srv::Listen>("/HRI/listener/listen");
}

// ---------------------------------------------------------
// Talker
// ---------------------------------------------------------

bool HRI::Say(const std::string& text)
{
    auto request = std::make_shared<talker_interfaces::srv::Talk::Request>();
    request->text = text;

    if (!waitForService(talker_client_, 500ms))
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "Talk service unavailable (/talk)");
        return false;
    }

    auto future = talker_client_->async_send_request(request);

    auto state = rclcpp::spin_until_future_complete(
        node_, future, 10s);

    if (state != rclcpp::FutureReturnCode::SUCCESS)
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "No response from talk service");
        return false;
    }

    auto response = future.get();

    if (response->success)
    {
        RCLCPP_INFO(node_->get_logger(),
                    "TTS OK: %s", response->message.c_str());
    }
    else
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "TTS ERROR: %s", response->message.c_str());
    }

    return response->success;
}

// ---------------------------------------------------------
// Listen
// ---------------------------------------------------------

std::string HRI::listen(const std::string& grammar)
{
    auto request = std::make_shared<listener_interfaces::srv::Listen::Request>();
    request->enable = true;
    request->grammar = grammar;

    if (!waitForService(listener_client_, 1000ms))
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "Listen service unavailable (/listen)");
        return "";
    }

    auto future = listener_client_->async_send_request(request);

    auto state = rclcpp::spin_until_future_complete(
        node_, future, 10s);

    if (state != rclcpp::FutureReturnCode::SUCCESS)
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "No response from listen service");
        return "";
    }

    return future.get()->recognized_speech;
}

// ---------------------------------------------------------
// Wait for service helper
// ---------------------------------------------------------

bool HRI::waitForService(
    const rclcpp::ClientBase::SharedPtr& client,
    const std::chrono::milliseconds& timeout) const
{
    return client->wait_for_service(timeout);
}
