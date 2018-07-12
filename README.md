## Abstract
It serves as a kind of bridge between server to other-clients.
connection with OPCUA server( https://github.com/thingspin/factory-thing-opcua-server ) and gathering data, send data using MQTT or TCP 


## Using open62541
A general introduction to OPC UA and the open62541 documentation can be found at http://open62541.org/doc/current.
Past releases of the library can be downloaded at https://github.com/open62541/open62541/releases.
To use the latest improvements, download a nightly build of the *single-file distribution* (the entire library merged into a single source and header file) from http://open62541.org/releases. 
Nightly builds of MSVC binaries of the library are available [here](https://ci.appveyor.com/project/open62541/open62541/build/artifacts).
## Using json-c
JSON-C implements a reference counting object model that allows you to easily construct JSON objects in C, output them as JSON formatted strings and parse JSON formatted strings back into the C representation of JSON objects. It aims to conform to RFC 7159.
## Using paho.mqtt.embedded-c
The Paho embedded client libraries arose out of the desire to allow the smallest microcontrollers to easily connect to MQTT servers.

### Pre-condition
- mqtt : install ( https://mosquitto.org/  |  mosquitto broker/ client)
    - sudo apt-get install mosquitto
- json-c : build ( included )
    - mkdir build
    - cmake ..
    - make
- paho.mqtt.embedded-c : build ( included )
    - mkdir build
    - cmake ..
    - make


## Implementation
1. modify CmakeLists
- CMakeLists.txt file need modify
- file exist path : open62541_mqtt/mqtt/CMakeLists.txt 

    - EX) line 1: set(EXTER_MQTT_ROOT "/home/mint/paho.mqtt.embedded-c"  ->  set(EXTER_MQTT_ROOT "/home/test/factory-thing/paho.mqtt.embedded-c"
    - EX) line 2: set(EXTER_JSON_ROOT "/home/mint/json-c" ->  set(EXTER_MQTT_ROOT "/home/test/factory-thing/json-c"
    - EX) line 96 : add_excutable( opcua-mqtt-bridge -> add_excutable( factory-thing
    - EX) line 98 : target_link_libraries( opcua-mqtt-bridge -> add_excutable( factory-thing
2. build 'open62541_mqtt'
    - mkdir build
    - cd build
    - cmake .. -DUA_ENABLE_SUBSCRIPTIONS=true -DUA_ENABLE_METHODCALLS=true -DUA_ENABLE_NODEMANAGEMENT=true -DUA_ENABLE_NONSTANDARD_MQTT=true
    - make ( created bin folder : config.json, libopen62541.a, opcua-mqtt-bridge)

    Tip

    - cmake CXX can't find : sudo apt-get update && sudo apt-get install build-essential
    - can't find sphinx : sudo apt-get install python-sphinx graphviz
    & sudo apt-get install python-sphinx-rtd-theme 
    - missing LATEX : sudo apt-get install kile

3. config.json modify
- file exist path : open62541_mqtt/build/bin
```c
//EXAMPLE
    {
    "device-configuration": {
        "Device": {
            "deviceID": "sensor0"
        }
    },
    "server-configuration": {
        "opcuaServer": {
            "EndpointURL": "opc.tcp://localhost:16664",
            "publishIntervalUs":1000000,
            "asycRequestSupported": false,
            "method": "poll"
        },
        "mqttBrocker": {
            "enable": true,
            "ip" : "192.168.0.197",
            "port": 1883,
            "topicBase": "topic"
        },
        "tcpSever": {
            "enable": false,
            "ip": "192.168.2.104",
            "port": 5555,
            "sampleIntervalUs": 100,
            "singleshot": true /* at now : should be true */
        }
    },

    // 1usec = 1
    // 1msec = 1000
    // 1sec = 1000000
    // 1min = 60000000

    "node-map": [
        {
           "name": "test",
           "enable": true,
           "method": "poll",
           "intervalUSec": 1000000,
           "topic": "test_opcua",
           "mqtt": true,
           "format": "json",
           "nodes": [
        { "id": "ns=1;s=sound_data", "topic":"sound", "alias": "" },
        { "id": "ns=1;s=temp_data", "topic": "temp", "alias": "" },
        { "id": "ns=1;s=tilt_data", "topic": "tilt", "alias": "" }
            ]
        }
    ]
}
```
4. run
    - file exist path : open62541_mqtt/build/bin
    - ./opcua-mqtt-bridge --config config.json 
