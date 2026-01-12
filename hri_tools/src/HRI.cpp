#include "hri_tools/HRI.hpp"

using namespace std::chrono_literals;

HRI::HRI(const rclcpp::Node::SharedPtr& node)
    : node_(node)
{
    talker_client_ = node_->create_client<talker_interfaces::srv::Talk>("/hri/talker/talk");
    listener_client_ = node_->create_client<listener_interfaces::srv::Listen>("/hri/listener/listen");
    human_follower_client_ = node_->create_client<std_srvs::srv::SetBool>("/hri/human_tracking/enable");
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

std::string HRI::Listen(const std::string& grammar)
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

// -------------------------------------------------------------------------
// Human Follower
// -------------------------------------------------------------------------

bool HRI::FollowPerson(bool enable)
{
    auto request = std::make_shared<std_srvs::srv::SetBool::Request>();
    request->data = enable;

    // Espera a que el servicio esté disponible
    if (!waitForService(human_follower_client_, 1000ms))
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "Human tracking service unavailable (/hri/human_tracking/enable)");
        return false;
    }

    // Llamada asíncrona
    auto future = human_follower_client_->async_send_request(request);

    // Espera la respuesta
    auto state = rclcpp::spin_until_future_complete(
        node_, future, 5s);

    if (state != rclcpp::FutureReturnCode::SUCCESS)
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "No response from /hri/human_tracking/enable");
        return false;
    }

    auto response = future.get();
    if (!response->success)
    {
        RCLCPP_WARN(node_->get_logger(),
                    "Human tracking service returned false: %s",
                    response->message.c_str());
        return false;
    }

    RCLCPP_INFO(node_->get_logger(),
                "Human tracking %s OK: %s",
                enable ? "ENABLED" : "DISABLED",
                response->message.c_str());

    return true;
}
