cmake_minimum_required(VERSION 3.16)

project(StreamPublisher LANGUAGES C CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(CycloneDDS-CXX REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0 gstreamer-video-1.0)

idlcxx_generate(
    TARGET generated
    FILES idl/VideoMessage.idl
)

add_executable(StreamPublisher
  main.cpp
)
target_include_directories(StreamPublisher PRIVATE ${GST_INCLUDE_DIRS})
target_link_libraries(StreamPublisher
  PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  CycloneDDS-CXX::ddscxx
  generated
  ${GST_LIBRARIES}
)

include(GNUInstallDirs)
install(TARGETS StreamPublisher
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
