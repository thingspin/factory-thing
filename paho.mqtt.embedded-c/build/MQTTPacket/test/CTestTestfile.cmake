# CMake generated Testfile for 
# Source directory: /home/pi/paho.mqtt.embedded-c/MQTTPacket/test
# Build directory: /home/pi/paho.mqtt.embedded-c/build/MQTTPacket/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(test1 "/home/pi/paho.mqtt.embedded-c/build/MQTTPacket/test/test1" "--connection" "tcp://localhost:1883")
set_tests_properties(test1 PROPERTIES  TIMEOUT "540")
