cmake_minimum_required(VERSION 3.20)
project(sap_cloud_client VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

# OpenSSL (system library for crypto operations)
find_package(OpenSSL REQUIRED)

set(SOURCES
    src/main.cpp
    src/api_client.cpp
    src/main_window.cpp
    src/drive_screen.cpp
    src/notes_screen.cpp
    src/ssh_auth.cpp
)

set(HEADERS
    include/sap_cloud_client/types.h
    include/sap_cloud_client/api_client.h
    include/sap_cloud_client/main_window.h
    include/sap_cloud_client/drive_screen.h
    include/sap_cloud_client/notes_screen.h
    include/sap_cloud_client/ssh_auth.h
)

set(RESOURCES
    resources/resources.qrc
)

if(ANDROID)
    qt_add_executable(sap_cloud_client ${SOURCES} ${HEADERS} ${RESOURCES})
    set_target_properties(sap_cloud_client PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_TARGET_SDK_VERSION 34
        QT_ANDROID_MIN_SDK_VERSION 24
    )
else()
    qt_add_executable(sap_cloud_client ${SOURCES} ${HEADERS} ${RESOURCES})
endif()

target_include_directories(sap_cloud_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sap_cloud_client PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    OpenSSL::Crypto
)

# Installation
install(TARGETS sap_cloud_client
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
