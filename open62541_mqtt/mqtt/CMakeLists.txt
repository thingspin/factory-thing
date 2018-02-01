set(EXTER_MQTT_ROOT "/home/mint/paho.mqtt.embedded-c" CACHE STRING "MQTT paho header and library root path")
set(EXTER_JSON_ROOT "/home/mint/json-c" CACHE STRING "JSON header and library root path")

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/plugins)
include_directories(${PROJECT_BINARY_DIR})

if(UA_ENABLE_AMALGAMATION)
    include_directories(${PROJECT_BINARY_DIR}) # contain open62541.h
else()
    add_definitions(-DUA_NO_AMALGAMATION)
endif()

list(APPEND LIBS ${open62541_LIBRARIES})
if(NOT WIN32)
  list(APPEND LIBS pthread)
  if (NOT APPLE)
    list(APPEND LIBS rt)
  endif()
else()
  list(APPEND LIBS ws2_32)
endif()
if(UA_ENABLE_MULTITHREADING)
  list(APPEND LIBS urcu-cds urcu urcu-common)
endif(UA_ENABLE_MULTITHREADING)

set(STATIC_OBJECTS $<TARGET_OBJECTS:open62541-object>)
if(WIN32 AND BUILD_SHARED_LIBS)
	# on windows the .dll.a file has to be used for the linker
	list(APPEND LIBS open62541)
	set(STATIC_OBJECTS "")
endif()

set(EXTER_MQTT_SRC_DIR "${EXTER_MQTT_ROOT}/MQTTPacket" CACHE STRING "MQTT paho packet source folder")
set(EXTER_MQTT_LIB_DIR "${EXTER_MQTT_ROOT}/build" CACHE STRING "MQTT paho binaly output folder")

include_directories(${EXTER_MQTT_SRC_DIR}/src)
include_directories(${EXTER_MQTT_LIB_DIR})
list(APPEND LIBS paho-embed-mqtt3c)
list(APPEND LIBS pthread)

list(APPEND STATIC_OBJECTS "transport")
link_directories(${EXTER_MQTT_LIB_DIR})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -s")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE")

remove_definitions(-Wsign-conversion  -Wconversion)
remove_definitions(-Wmissing-prototypes -Wshadow)
remove_definitions(-Wunused-variable -Wunused-but-set-variable)
remove_definitions(-Wimplicit-function-declaration)
remove_definitions(-Wint-to-pointer-cast)
remove_definitions(-Wstrict-prototypes)
remove_definitions(-std=c99)

add_definitions(-Wno-unused-but-set-variable -Wno-sign-conversion)
##add_definitions(-Wno-missing-prototypes -Wno-shadow -Wno-conversion)
##add_definitions(-Wno-implicit-function-declaration)
add_definitions(-Wno-unused-variable)
add_definitions(-Wno-int-to-pointer-cast)

set(mqtt_lib_sources 
    ${EXTER_MQTT_SRC_DIR}/src/MQTTPacket.c
    ${EXTER_MQTT_SRC_DIR}/src/MQTTConnectClient.c
    ${EXTER_MQTT_SRC_DIR}/src/MQTTSerializePublish.c
    ${EXTER_MQTT_SRC_DIR}/src/MQTTSubscribeClient.c
    ${EXTER_MQTT_SRC_DIR}/src/MQTTUnsubscribeClient.c)


#add_executable(opcua-mqtt opcua-mqtt.c pub0sub1.c lepton.c ${mqtt_lib_sources} ${STATIC_OBJECTS})
#add_dependencies(opcua-mqtt open625451_amalgamation)
#target_link_libraries(opcua-mqtt ${LIBS})

#add_executable(server server.c pub0sub1.c lepton.c ${mqtt_lib_sources} ${STATIC_OBJECTS})
#add_dependencies(server open625451_amalgamation)
#target_link_libraries(server ${LIBS})

add_definitions(-g)
include_directories(${EXTER_JSON_ROOT})
link_directories(${EXTER_JSON_ROOT}/build)
list(APPEND JSONLIBS json-c)

list(APPEND CLIENTSRCS 
  client-main.cpp
  client-config.cpp 
  client-nodeid.cpp
  client-connection.c
  client-browse.c
  client-monitoring.cpp   
  client-mqtt.c
  client-trans-tcp.cpp
  client-tcp.c

)

add_executable(opcua-mqtt-bridge ${CLIENTSRCS} ${mqtt_lib_sources} ${STATIC_OBJECTS})
#add_dependencies(opcua-mqtt-bridge open625451_amalgamation)
target_link_libraries(opcua-mqtt-bridge ${LIBS} ${JSONLIBS})
