# hri_tools
This repository provides a utility class for **Human-Robot Interaction (HRI)** in ROS 2.

## Description
The `HRI` class acts as a client for multiple HRI-related services, providing a simple API to:

- Send text to a TTS system (**Talker**)
- Perform speech recognition (**Listener**)
- Enable/disable human tracking (**Human follower**)

The class is designed to be used from other nodes as a lightweight wrapper around service calls.

Service timeout behavior:
- Waits up to **500 ms** for Talker and Listener services to be available.
- Waits up to **1000 ms** for the Human Tracking service to be available.
- Waits up to **10 seconds** for Talker and Listener responses.
- Waits up to **5 seconds** for Human Tracking responses.

## Behaviour
### Say (Text-to-Speech)
1. Waits for the talk service to be available.
2. Sends the requested text.
3. Blocks execution until a response is received or timeout occurs.
4. Returns:
   - `true` → TTS succeeded.
   - `false` → Service unavailable, timeout, or TTS failure.

### Listen (Speech-to-Text)
1. Waits for the listen service to be available.
2. Sends a request with:
   - `enable = true`
   - `grammar` (optional)
3. Blocks execution until a response is received or timeout occurs.
4. Returns:
   - Recognized text (`std::string`)
   - `""` → Service unavailable, timeout, or empty recognition result.

### FollowPerson (Human tracking)
1. Waits for the human tracking service to be available.
2. Sends a `SetBool` request (`true` = enable, `false` = disable).
3. Blocks execution until a response is received or timeout occurs.
4. Returns:
   - `true` → Human tracking successfully enabled/disabled.
   - `false` → Service unavailable, timeout, or tracking failure.

## Internals
The class internally handles:
- Asynchronous service calls (`async_send_request`)
- Condition variable synchronization
- Timeout management
- Basic exception-safe blocking logic
- Logging of service results

## Interfaces
### Service Clients

#### Talker
- `/hri/talker/talk` [`talker_interfaces::srv::Talk`]

Request:
- `text` [`string`]

Response:
- `success` [`bool`]
- `message` [`string`]

#### Listener
- `/hri/listener/listen` [`listener_interfaces::srv::Listen`]

Request:
- `enable` [`bool`]
- `grammar` [`string`]

Response:
- `success` [`bool`]
- `recognized_speech` [`string`]

#### Human Tracking
- `/hri/human_tracking/enable` [`std_srvs::srv::SetBool`]

Request:
- `data` [`bool`]

Response:
- `success` [`bool`]
- `message` [`string`]

## Usage example
Include the header:

```cpp
#include "hri_tools/HRI.hpp"
```

Then:
```cpp
auto hri = std::make_shared<HRI>(node);

// -------------------------------------------------
// TTS
// -------------------------------------------------
bool ok = hri->Say("Hello human!");

if (ok)
{
  RCLCPP_INFO(node->get_logger(), "Talk succeeded");
}
else
{
  RCLCPP_WARN(node->get_logger(), "Talk failed");
}

// -------------------------------------------------
// Speech recognition
// -------------------------------------------------
std::string text = hri->Listen("");

if (!text.empty())
{
  RCLCPP_INFO(node->get_logger(), "Recognized: %s", text.c_str());
}
else
{
  RCLCPP_WARN(node->get_logger(), "No speech recognized");
}

// -------------------------------------------------
// Enable human following
// -------------------------------------------------
if (hri->FollowPerson(true))
{
  RCLCPP_INFO(node->get_logger(), "Human tracking enabled");
}
else
{
  RCLCPP_ERROR(node->get_logger(), "Failed to enable human tracking");
}
```

## Notes
- This class is synchronous from the user perspective (it blocks), but internally uses asynchronous ROS 2 service calls.
- Timeouts are fixed in the current implementation: 
  - Talk: 500 ms service wait, 10 s response wait
  - Listen: 500 ms service wait, 10 s response wait
  - FollowPerson: 1000 ms service wait, 5 s response wait

- The `Listen()` method always sends `enable = true`. Disabling the listener is expected to be handled by the server side if needed.
