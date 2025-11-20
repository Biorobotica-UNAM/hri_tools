cmake_minimum_required(VERSION 3.5)
project(hri_tools)

set(CMAKE_CXX_STANDARD 14)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(listener_interfaces REQUIRED)
#find_package(geometry_msgs REQUIRED)
#find_package(robotino_msgs REQUIRED)


include_directories(
  include
  ${rclcpp_INCLUDE_DIRS}
  ${std_msgs_INCLUDE_DIRS}
#  ${geometry_msgs_INCLUDE_DIRS}
#  ${robotino_msgs_INCLUDE_DIRS}
)

add_library(hri_tools SHARED
  src/HRI.cpp
)

ament_target_dependencies(hri_tools
  rclcpp
  std_msgs
  nlohmann_json
  listener_interfaces
#  geometry_msgs
#  robotino_msgs

)

install(DIRECTORY include/
  DESTINATION include
)

install(TARGETS hri_tools
  DESTINATION lib
)
 # AQUI AGREGUE LO DEL LAUNCH
install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(hri_tools)

ament_package()
