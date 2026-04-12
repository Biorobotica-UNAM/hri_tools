/**
 * @brief Facade implementation for HRI services.
 *
 * Each public method is responsible for only two things:
 *   1. Build the service request.
 *   2. Map the service response to the return value.
 *
 * All clients are created in a ReentrantCallbackGroup so their
 * response callbacks can run concurrently with any caller callback.
 * call_service<T>() uses future.wait_for() instead of mutex+cv
 * to avoid deadlocking the executor.
 *
 * BioRobotics Laboratory, Facultad de Ingenieria, UNAM
 */

#include "hri_tools/HRI.hpp"

// ============================================================
// Constructor
// ============================================================

HRI::HRI(const rclcpp::Node::SharedPtr &node) : node_(node) {
  cb_group_clients_ =
      node_->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

  const rclcpp::QoS qos = rclcpp::ServicesQoS();

  talker_client_ = node_->create_client<talker_interfaces::srv::Talk>(
      "/hri/talker/talk", qos, cb_group_clients_);

  listener_client_ = node_->create_client<listener_interfaces::srv::Listen>(
      "/listener/listen", qos, cb_group_clients_);

  follower_client_ = node_->create_client<std_srvs::srv::SetBool>(
      "/hri/human_tracking/enable", qos, cb_group_clients_);
}

// ============================================================
// Say
// ============================================================

bool HRI::Say(const std::string &text, std::chrono::seconds timeout) {
  auto request = std::make_shared<talker_interfaces::srv::Talk::Request>();
  request->text = text;

  bool success = false;

  bool completed = call_service<talker_interfaces::srv::Talk>(
      talker_client_, request,
      [&](auto response) {
        success = response->success;
        RCLCPP_INFO(node_->get_logger(), "TTS %s: %s", success ? "OK" : "ERROR",
                    response->message.c_str());
      },
      timeout, "Talk");

  return completed && success;
}

// ============================================================
// Listen
// ============================================================

std::string HRI::Listen(const std::string &grammar,
                        std::chrono::seconds timeout) {
  auto request = std::make_shared<listener_interfaces::srv::Listen::Request>();
  request->enable = true;
  request->grammar = grammar;

  std::string recognized;

  call_service<listener_interfaces::srv::Listen>(
      listener_client_, request,
      [&](auto response) { recognized = response->recognized_speech; }, timeout,
      "Listen");

  return recognized;
}

// ============================================================
// FollowPerson
// ============================================================

bool HRI::FollowPerson(bool enable, std::chrono::seconds timeout) {
  auto request = std::make_shared<std_srvs::srv::SetBool::Request>();
  request->data = enable;

  bool success = false;

  bool completed = call_service<std_srvs::srv::SetBool>(
      follower_client_, request,
      [&](auto response) {
        success = response->success;
        RCLCPP_INFO(node_->get_logger(), "Human tracking %s %s: %s",
                    enable ? "ENABLE" : "DISABLE", success ? "OK" : "FAILED",
                    response->message.c_str());
      },
      timeout, "HumanTracking");

  return completed && success;
}
