project(foxtalk_handlers)

include_directories(../vendor/gc-8.2.4)
include_directories(../lib)



add_library(V4L_CamFormatHandler SHARED V4L_CamFormatHandler.cpp)