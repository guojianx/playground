cmake_minimum_required(VERSION 2.8)

project(HiWebRTC)

add_executable(hi main.cpp)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(ENV{PKG_CONFIG_PATH} "/root/WebRTC/gstreamer/cerbero/build/dist/linux_x86_64/lib/pkgconfig")
message($ENV{PKG_CONFIG_PATH})

find_package(PkgConfig REQUIRED)
pkg_search_module(GST REQUIRED gstreamer-1.0)
pkg_search_module(GSTRTC REQUIRED gstreamer-webrtc-1.0)
pkg_search_module(GSTSDP REQUIRED gstreamer-sdp-1.0)
pkg_search_module(JSON REQUIRED json-glib-1.0)
pkg_search_module(CURL REQUIRED libcurl)

include_directories(${GST_INCLUDE_DIRS} ${GSTRTC_INCLUDE_DIRS} ${GSTSDP_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS})
target_link_libraries(hi ${GST_LDFLAGS} ${GSTRTC_LDFLAGS} ${GSTSDP_LDFLAGS} ${JSON_LDFLAGS} ${CURL_LDFLAGS})
