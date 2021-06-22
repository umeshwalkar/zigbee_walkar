file(GLOB_RECURSE SRCS *.c)
file(GLOB_RECURSE HDRS *.h)

add_executable(zigbeeWalker ${SRCS} ${HDRS})

target_link_libraries( zigbeeWalker -lpthread  ${OPENZWAVE_LIB} ${JSONCPP_LIB} ${DIGIXBEE_LIB})
     
install (TARGETS zigbeeWalker DESTINATION bin)
