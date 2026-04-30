#pragma once

/**
 * @brief Facade over ROS2 Human-Robot Interaction services.
 *
 * Responsibility: provide a clean, synchronous interface to HRI
 * services (speech, listening, person following) so callers never
 * need to know about ROS2 async patterns or timeouts.
 *
 * Design patterns applied:
 *   Facade          (GoF Structural)  — hides ROS2 service clients
 *                                       behind intention-revealing methods.
 *   Template Method (GoF Behavioral)  — call_service<T>() captures the
 *                                       invariant async→wait→return flow;
 *                                       each public method supplies only
 *                                       what varies (request + response
 * mapping).
 *
 * Principles applied:
 *   SRP  — one class, one responsibility: HRI communication.
 *   DRY  — the async + future.wait_for + timeout pattern lives in
 *          exactly one place.
 *   DIP  — callers depend on HRI, not on rclcpp::Client directly.
 *
 * BioRobotics Laboratory, Facultad de Ingenieria, UNAM
 */

#include <chrono>
#include <future>
#include <memory>
#include <string>

#include "listener_interfaces/srv/listen.hpp"
#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/set_bool.hpp"
#include "talker_interfaces/srv/talk.hpp"

using namespace std::chrono_literals;

class HRI {
public:
  explicit HRI(const rclcpp::Node::SharedPtr &node);

  /**
   * Send text to the TTS service and wait for confirmation.
   * @param text     Text to synthesise.
   * @param timeout  Maximum time to wait for the service (default: 10 s).
   * @return true if the service reported success.
   */
  bool Say(const std::string &text, std::chrono::seconds timeout = 10s);

  /**
   * Request one speech recognition cycle and return the result.
   * @param grammar  Optional grammar filename (empty = free-form).
   * @param timeout  Maximum time to wait for the service (default: 5 s).
   * @return Recognised text, or empty string on failure / timeout.
   */
  std::string Listen(const std::string &grammar = "",
                     std::chrono::seconds timeout = 5s);

  /**
   * Enable or disable the human-following behaviour.
   * @param enable   True to start following, false to stop.
   * @param timeout  Maximum time to wait for the service (default: 5 s).
   * @return true if the service reported success.
   */
  bool FollowPerson(bool enable, std::chrono::seconds timeout = 5s);

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::CallbackGroup::SharedPtr cb_group_clients_;

  rclcpp::Client<talker_interfaces::srv::Talk>::SharedPtr talker_client_;
  rclcpp::Client<listener_interfaces::srv::Listen>::SharedPtr listener_client_;
  rclcpp::Client<std_srvs::srv::SetBool>::SharedPtr follower_client_;

  /**
   * Template Method: invariant async → wait → return flow.
   *
   * Uses future.wait_for() with a ReentrantCallbackGroup so this
   * method can be called safely from inside any ROS2 callback without
   * blocking the executor.
   *
   * @tparam ServiceT   ROS2 service type.
   * @tparam CallbackT  Callable: (Response::SharedPtr) → void.
   *
   * @param client      The service client to use.
   * @param request     The fully populated request object.
   * @param on_response Callback that extracts results from the response.
   * @param timeout     Maximum time allowed for the call.
   * @param label       Human-readable service name for log messages.
   *
   * @return true if the call completed before the timeout.
   */
  template <typename ServiceT, typename CallbackT>
  bool call_service(typename rclcpp::Client<ServiceT>::SharedPtr &client,
                    typename ServiceT::Request::SharedPtr request,
                    CallbackT on_response, std::chrono::seconds timeout,
                    const std::string &label);
};

// ============================================================
// Template implementation (must live in the header)
// ============================================================

template <typename ServiceT, typename CallbackT>
bool HRI::call_service(typename rclcpp::Client<ServiceT>::SharedPtr &client,
                       typename ServiceT::Request::SharedPtr request,
                       CallbackT on_response, std::chrono::seconds timeout,
                       const std::string &label) {
  if (!client->wait_for_service(1s)) {
    RCLCPP_ERROR(node_->get_logger(), "%s service not available.",
                 label.c_str());
    return false;
  }

  auto future = client->async_send_request(request);

  if (future.wait_for(timeout) != std::future_status::ready) {
    RCLCPP_ERROR(node_->get_logger(), "Timeout calling %s service after %ld s.",
                 label.c_str(), timeout.count());
    return false;
  }

  on_response(future.get());
  return true;
}
