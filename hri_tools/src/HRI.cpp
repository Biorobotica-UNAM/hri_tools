#include "hri_tools/HRI.hpp"

HRI::HRI(rclcpp::Node::SharedPtr node)
    : node_(node), speech_done_(false)
{
    pub_speech_ = node_->create_publisher<std_msgs::msg::String>("voice/speech", 10);
    sub_speech_status_ = node_->create_subscription<std_msgs::msg::Bool>("voice/speech_status", 10, std::bind(&HRI::speechStatusCallback, this, std::placeholders::_1));
    client_recog_ = node_->create_client<listener_interfaces::srv::Listen>("/listen", rclcpp::QoS(10));
}

void HRI::SpeechStatusCallback(const std_msgs::msg::Bool::SharedPtr msg)
{
    if(msg->data)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        speech_done_ = true;
    }
}

void HRI::Say(const std::string& text)
{
    while(pub_speech_->get_subscription_count() == 0 && rclcpp::ok())
    {
        rclcpp::spin_some(node_);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std_msgs::msg::String msg;
    msg.data = text;

    {
        std::lock_guard<std::mutex> lock(mtx_);
        speech_done_ = false;
    }

    pub_speech_->publish(msg);

    while(rclcpp::ok())
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if(speech_done_)
                break;
        }
        rclcpp::spin_some(node_);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

std::string HRI::Listen(const std::string& grammar)
{
    auto request = std::make_shared<listener_interfaces::srv::Listen::Request>();
    request->enable = true;
    request->grammar = grammar;

    if (!client_recog_->wait_for_service(std::chrono::milliseconds(500)))
        return "No hay servicio";

    auto future = client_recog_->async_send_request(request);
	if (rclcpp::spin_until_future_complete(node_, future, std::chrono::seconds(10)) == rclcpp::FutureReturnCode::SUCCESS)
		return future.get()->recognized_speech;


    return "No recibio respuesta confiable";
}
