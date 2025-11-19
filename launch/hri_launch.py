from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():

    return LaunchDescription([

        Node(
            package='talker',
            executable='talker',
            name='speech_talker',
            output='screen'
        ),

        Node(
            package='listener',
            executable='listener',
            name='speech_listener',
            output='screen'
        ),
    ])
