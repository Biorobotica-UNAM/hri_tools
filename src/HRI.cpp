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
// Wait for service
// ---------------------------------------------------------

bool HRI::waitForService(
    const rclcpp::ClientBase::SharedPtr& client,
    const std::chrono::milliseconds& timeout) const
{
    return client->wait_for_service(timeout);
}

// ---------------------------------------------------------
// Talker
// ---------------------------------------------------------

bool HRI::Say(const std::string& text)
{
    if (!waitForService(talker_client_, 500ms))
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "Talk service unavailable (/talk)");
        return false;
    }
    
    auto request = std::make_shared<talker_interfaces::srv::Talk::Request>();
    request->text = text;
    
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    bool success = false;
    std::string message;

    auto callback = [&](rclcpp::Client<talker_interfaces::srv::Talk>::SharedFuture future)
        {
            auto response = future.get();
            success = response->success;
            message = response->message;
            done = true;
            cv.notify_one();
        };

	talker_client_->async_send_request(request, callback);
	
	std::unique_lock<std::mutex> lock(mtx);
	
	if (!cv.wait_for(lock, 10s, [&]{ return done; }))
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "Talk service timed out");
        return false;
    }

    if (success)
    {
        RCLCPP_INFO(node_->get_logger(),
                    "TTS OK: %s", message.c_str());
    }
    else
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "TTS ERROR: %s", message.c_str());
    }

    return response->success;
}

// ---------------------------------------------------------
// Listen
// ---------------------------------------------------------

std::string HRI::Listen(const std::string& grammar)
{
    if (!waitForService(listener_client_, 500ms))
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "Listen service unavailable (/listen)");
        return "";
    }
    
    auto request = std::make_shared<listener_interfaces::srv::Listen::Request>();
    request->enable = true;
    request->grammar = grammar;

    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    std::string recognized;
    
    auto callback = [&](rclcpp::Client<listener_interfaces::srv::Listen>::SharedFuture future)
        {
            recognized = future.get()->recognized_speech;
            done = true;
            cv.notify_one();
        };

    listener_client_->async_send_request(request, callback);
    
    std::unique_lock<std::mutex> lock(mtx);
    
    if (!cv.wait_for(lock, 10s, [&]{ return done; }))
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "Listen service timed out");
        return "";
    }

    return future.get()->recognized_speech;
}

// -------------------------------------------------------------------------
// Follow person
// -------------------------------------------------------------------------

bool HRI::FollowPerson(bool enable)
{
    if (!waitForService(human_follower_client_, 1000ms))
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "Human tracking service unavailable");
        return false;
    }

    auto request = std::make_shared<std_srvs::srv::SetBool::Request>();
    request->data = enable;

    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    bool success = false;
    std::string message;

    auto callback = [&](rclcpp::Client<std_srvs::srv::SetBool>::SharedFuture future)
        {
            auto response = future.get();
            success = response->success;
            message = response->message;
            done = true;
            cv.notify_one();
        };

    human_follower_client_->async_send_request(request, callback);

    std::unique_lock<std::mutex> lock(mtx);
    
    if (!cv.wait_for(lock, 5s, [&]{ return done; }))
    {
        RCLCPP_ERROR(node_->get_logger(),
                     "Human tracking service timed out");
        return false;
    }

    if (!success)
    {
        RCLCPP_WARN(node_->get_logger(),
                    "Human tracking failed: %s",
                    message.c_str());
        return false;
    }

    RCLCPP_INFO(node_->get_logger(),
                "Human tracking %s OK: %s",
                enable ? "ENABLED" : "DISABLED",
                message.c_str());

    return true;
}
