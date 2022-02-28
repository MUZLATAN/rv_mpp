cmake_minimum_required(VERSION 3.0.0)

# C++标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7-a_hardfp)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_C_COMPILER /opt/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER /opt/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++)

add_definitions("-D__PLATFORM_RV__ -DRV1126 -mfpu=neon -mfloat-abi=hard -O2 -ftree-vectorize")

include_directories(${CMAKE_CURRENT_LIST_DIR}/dependencies/rv/opencv/include
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/rv/ffmpeg/include
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/rv/rk_mpp/include/rockchip
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/rv/rknn/include)
link_directories(${CMAKE_CURRENT_LIST_DIR}/dependencies/rv/opencv/lib
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/rv/ffmpeg/lib
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/rv/rk_mpp/lib
    ${CMAKE_CURRENT_LIST_DIR}/dependencies/rv/rknn/lib)

add_executable(rv_mpp main.cpp mpp_dec_cfg.cpp MppDecode.cpp RtspMppImpl.cpp)
target_link_libraries(rv_mpp opencv_features2d
rknn_api
opencv_video
# opencv_videoio #hisi平台用于本地读取视频测试
opencv_imgcodecs
opencv_imgproc
opencv_core
opencv_highgui
avcodec avfilter avformat avcodec avutil swresample swscale
zlib
dl
pthread
rockchip_mpp
)