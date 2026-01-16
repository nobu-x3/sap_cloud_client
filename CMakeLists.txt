cmake_minimum_required(VERSION 3.20)
project(sap_cloud_client VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================
# Options
# =============================================================================

# Android OpenSSL configuration
set(ANDROID_OPENSSL_ROOT "" CACHE PATH "Path to Android OpenSSL root directory")
set(ANDROID_OPENSSL_INCLUDE_DIR "" CACHE PATH "Path to Android OpenSSL include directory (defaults to ANDROID_OPENSSL_ROOT/static/include)")
set(ANDROID_OPENSSL_LIB_DIR "" CACHE PATH "Path to Android OpenSSL library directory (defaults to ANDROID_OPENSSL_ROOT/latest/<arch>)")
set(ANDROID_OPENSSL_SSL_LIB "" CACHE STRING "Name of the SSL library (e.g., libssl.so or libssl_1_1.so)")
set(ANDROID_OPENSSL_CRYPTO_LIB "" CACHE STRING "Name of the Crypto library (e.g., libcrypto.so or libcrypto_1_1.so)")

# Android SDK versions
set(ANDROID_TARGET_SDK "34" CACHE STRING "Android target SDK version")
set(ANDROID_MIN_SDK "26" CACHE STRING "Android minimum SDK version")

# =============================================================================
# OpenSSL Configuration
# =============================================================================

if(ANDROID)
    # Validate required option
    if(NOT ANDROID_OPENSSL_ROOT)
        message(FATAL_ERROR "ANDROID_OPENSSL_ROOT must be set for Android builds. "
                "Set it via -DANDROID_OPENSSL_ROOT=/path/to/android_openssl")
    endif()

    # Map Android ABI to OpenSSL directory names
    if(ANDROID_ABI STREQUAL "arm64-v8a")
        set(OPENSSL_ARCH "arm64")
    elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
        set(OPENSSL_ARCH "arm")
    elseif(ANDROID_ABI STREQUAL "x86_64")
        set(OPENSSL_ARCH "x86_64")
    elseif(ANDROID_ABI STREQUAL "x86")
        set(OPENSSL_ARCH "x86")
    else()
        message(FATAL_ERROR "Unsupported Android ABI: ${ANDROID_ABI}")
    endif()

    # Set include directory (use provided or default)
    if(ANDROID_OPENSSL_INCLUDE_DIR)
        set(_OPENSSL_INCLUDE "${ANDROID_OPENSSL_INCLUDE_DIR}")
    else()
        set(_OPENSSL_INCLUDE "${ANDROID_OPENSSL_ROOT}/static/include")
    endif()

    # Set library directory (use provided or default)
    if(ANDROID_OPENSSL_LIB_DIR)
        set(_OPENSSL_LIB_DIR "${ANDROID_OPENSSL_LIB_DIR}")
    else()
        set(_OPENSSL_LIB_DIR "${ANDROID_OPENSSL_ROOT}/latest/${OPENSSL_ARCH}")
    endif()

    # Set library names (use provided or auto-detect)
    if(ANDROID_OPENSSL_SSL_LIB)
        set(_SSL_LIB_NAME "${ANDROID_OPENSSL_SSL_LIB}")
    else()
        # Auto-detect: prefer libssl.so, fall back to libssl_1_1.so
        if(EXISTS "${_OPENSSL_LIB_DIR}/libssl.so")
            set(_SSL_LIB_NAME "libssl.so")
        elseif(EXISTS "${_OPENSSL_LIB_DIR}/libssl_1_1.so")
            set(_SSL_LIB_NAME "libssl_1_1.so")
        else()
            message(FATAL_ERROR "Could not find OpenSSL SSL library in ${_OPENSSL_LIB_DIR}")
        endif()
    endif()

    if(ANDROID_OPENSSL_CRYPTO_LIB)
        set(_CRYPTO_LIB_NAME "${ANDROID_OPENSSL_CRYPTO_LIB}")
    else()
        # Auto-detect: prefer libcrypto.so, fall back to libcrypto_1_1.so
        if(EXISTS "${_OPENSSL_LIB_DIR}/libcrypto.so")
            set(_CRYPTO_LIB_NAME "libcrypto.so")
        elseif(EXISTS "${_OPENSSL_LIB_DIR}/libcrypto_1_1.so")
            set(_CRYPTO_LIB_NAME "libcrypto_1_1.so")
        else()
            message(FATAL_ERROR "Could not find OpenSSL Crypto library in ${_OPENSSL_LIB_DIR}")
        endif()
    endif()

    # Set CMake cache variables for FindOpenSSL
    set(OPENSSL_ROOT_DIR "${ANDROID_OPENSSL_ROOT}")
    set(OPENSSL_INCLUDE_DIR "${_OPENSSL_INCLUDE}" CACHE PATH "" FORCE)
    set(OPENSSL_SSL_LIBRARY "${_OPENSSL_LIB_DIR}/${_SSL_LIB_NAME}" CACHE FILEPATH "" FORCE)
    set(OPENSSL_CRYPTO_LIBRARY "${_OPENSSL_LIB_DIR}/${_CRYPTO_LIB_NAME}" CACHE FILEPATH "" FORCE)

    message(STATUS "Android OpenSSL configuration:")
    message(STATUS "  Include: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "  SSL lib: ${OPENSSL_SSL_LIBRARY}")
    message(STATUS "  Crypto lib: ${OPENSSL_CRYPTO_LIBRARY}")
endif()

# =============================================================================
# Dependencies
# =============================================================================

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
find_package(OpenSSL REQUIRED)

# =============================================================================
# Sources
# =============================================================================

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

# =============================================================================
# Target
# =============================================================================

qt_add_executable(sap_cloud_client ${SOURCES} ${HEADERS} ${RESOURCES})

if(ANDROID)
    set_target_properties(sap_cloud_client PROPERTIES
        QT_ANDROID_TARGET_SDK_VERSION ${ANDROID_TARGET_SDK}
        QT_ANDROID_MIN_SDK_VERSION ${ANDROID_MIN_SDK}
        QT_ANDROID_EXTRA_LIBS "${OPENSSL_SSL_LIBRARY};${OPENSSL_CRYPTO_LIBRARY}"
    )
endif()

target_include_directories(sap_cloud_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(sap_cloud_client PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    OpenSSL::SSL
    OpenSSL::Crypto
)

# =============================================================================
# Installation
# =============================================================================

install(TARGETS sap_cloud_client
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
)
