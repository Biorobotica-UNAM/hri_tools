#include "hri_tools/HRI.hpp"

using namespace std::chrono_literals;

HRI::HRI(const rclcpp::Node::SharedPtr& node)
    : node_(node)
{
    talker_client_ = node_->create_client<talker_interfaces::srv::Talk>("/hri/talker/talk");
    listener_client_ = node_->create_client<listener_interfaces::srv::Listen>("/hri/listener/listen");
}

// ---------------------------------------------------------
// Speak
// ---------------------------------------------------------

TalkResult HRI::Say(const std::string& text)
{
    TalkResult result;

    auto request = std::make_shared<talker_interfaces::srv::Talk::Request>();
    request->text = text;

    if (!WaitForService(talker_client_, 500ms))
    {
        result.success = false;
        result.message = "Talk service unavailable (/talk)";
        return result;
    }

    auto future = talker_client_->async_send_request(request);

    auto state = rclcpp::spin_until_future_complete(
        node_, future, 10s);

    if (state != rclcpp::FutureReturnCode::SUCCESS)
    {
        result.success = false;
        result.message = "No response from talk service";
        return result;
    }

    auto response = future.get();
    result.success = response->success;
    result.message = response->message;
    return result;
}

// ---------------------------------------------------------
// Listen
// ---------------------------------------------------------

std::string HRI::listen(const std::string& grammar)
{
    auto request = std::make_shared<listener_interfaces::srv::Listen::Request>();
    request->enable = true;
    request->grammar = grammar;

    if (!WaitForService(listener_client_, 500ms))
        return "Listen service unavailable (/listen)";

    auto future = listener_client_->async_send_request(request);

    auto state = rclcpp::spin_until_future_complete(
        node_, future, 10s);

    if (state != rclcpp::FutureReturnCode::SUCCESS)
        return "No response from listen service";

    return future.get()->recognized_speech;
}

// ---------------------------------------------------------
// Wait for service helper
// ---------------------------------------------------------

bool HRI::WaitForService(
    const rclcpp::ClientBase::SharedPtr& client,
    const std::chrono::milliseconds& timeout) const
{
    return client->wait_for_service(timeout);
}
