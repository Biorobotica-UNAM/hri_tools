cmake_minimum_required(VERSION 3.5)
project(hri_tools)

set(CMAKE_CXX_STANDARD 14)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(listener_interfaces REQUIRED)
find_package(talker_interfaces REQUIRED)
find_package(human_interfaces REQUIRED)


include_directories(
  include
  ${rclcpp_INCLUDE_DIRS}
  ${std_msgs_INCLUDE_DIRS}
  ${listener_interfaces_INCLUDE_DIRS}
  ${talker_interfaces_INCLUDE_DIRS}
)

add_library(hri_tools SHARED
  src/HRI.cpp
)

ament_target_dependencies(hri_tools
  rclcpp
  std_msgs
  std_srvs
  nlohmann_json
  listener_interfaces
  talker_interfaces
  human_interfaces
)

install(DIRECTORY include/
  DESTINATION include
)

install(TARGETS hri_tools
  DESTINATION lib
)

install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_export_dependencies(
  rclcpp
  std_msgs
  std_srvs
  nlohmann_json
  listener_interfaces
  talker_interfaces
  human_interfaces
)

ament_export_include_directories(include)
ament_export_libraries(hri_tools)

ament_package()
