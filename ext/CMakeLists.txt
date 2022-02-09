set(XMI2MID_SRCS
    xmi2mid.c
)

include_directories(ext)
# because of licensing xmi2mid must be shared... =(
add_library(xmi2mid SHARED ${XMI2MID_SRCS})

add_subdirectory(spng/)
