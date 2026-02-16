#include "hri_tools/HRI.hpp"

using namespace std::chrono_literals;

HRI::HRI(const rclcpp::Node::SharedPtr& node)
: node(node)
{
    talkerClient =
        node->create_client<talker_interfaces::srv::Talk>(
            "/hri/talker/talk");

    listenerClient =
        node->create_client<listener_interfaces::srv::Listen>(
            "/hri/listener/listen");

    followerClient =
        node->create_client<std_srvs::srv::SetBool>(
            "/hri/human_tracking/enable");
}

// ---------------------------------------------------------
// Say
// ---------------------------------------------------------

bool HRI::Say(const std::string& text)
{
    if (!talkerClient->wait_for_service(500ms))
    {
        RCLCPP_ERROR(node->get_logger(),
                     "Talk service unavailable");
        return false;
    }

    auto request =
        std::make_shared<talker_interfaces::srv::Talk::Request>();

    request->text = text;

    auto future = talkerClient->async_send_request(request);

    if (rclcpp::spin_until_future_complete(node, future)
        != rclcpp::FutureReturnCode::SUCCESS)
    {
        RCLCPP_ERROR(node->get_logger(),
                     "Talk service call failed");
        return false;
    }

    auto response = future.get();

    if (!response->success)
    {
        RCLCPP_ERROR(node->get_logger(),
                     "TTS ERROR: %s",
                     response->message.c_str());
        return false;
    }

    RCLCPP_INFO(node->get_logger(),
                "TTS OK: %s",
                response->message.c_str());

    return true;
}


// ---------------------------------------------------------
// Listen
// ---------------------------------------------------------

std::string HRI::Listen(const std::string& grammar)
{
    if (!listenerClient->wait_for_service(500ms))
    {
        RCLCPP_ERROR(node->get_logger(),
                     "Listen service unavailable");
        return "";
    }

    auto request =
        std::make_shared<listener_interfaces::srv::Listen::Request>();

    request->enable = true;
    request->grammar = grammar;

    auto future = listenerClient->async_send_request(request);

    if (rclcpp::spin_until_future_complete(node, future)
        != rclcpp::FutureReturnCode::SUCCESS)
    {
        RCLCPP_ERROR(node->get_logger(),
                     "Listen service call failed");
        return "";
    }

    return future.get()->recognized_speech;
}


// -------------------------------------------------------------------------
// Follow person
// -------------------------------------------------------------------------

bool HRI::FollowPerson(bool enable)
{
    if (!followerClient->wait_for_service(1000ms))
    {
        RCLCPP_ERROR(node->get_logger(),
                     "Human tracking service unavailable");
        return false;
    }

    auto request =
        std::make_shared<std_srvs::srv::SetBool::Request>();

    request->data = enable;

    auto future = followerClient->async_send_request(request);

    if (rclcpp::spin_until_future_complete(node, future)
        != rclcpp::FutureReturnCode::SUCCESS)
    {
        RCLCPP_ERROR(node->get_logger(),
                     "Human tracking call failed");
        return false;
    }

    auto response = future.get();

    if (!response->success)
    {
        RCLCPP_WARN(node->get_logger(),
                    "Human tracking failed: %s",
                    response->message.c_str());
        return false;
    }

    RCLCPP_INFO(node->get_logger(),
                "Human tracking %s OK",
                enable ? "ENABLED" : "DISABLED");

    return true;
}

