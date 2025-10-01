# UIAList - WinUI3 C++ Implementation
# No Qt dependencies - Pure Windows Runtime

cmake_minimum_required(VERSION 3.20)

project(UIAList VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-only project
if(NOT WIN32)
    message(FATAL_ERROR "This project only supports Windows")
endif()

# Require Windows 10 SDK version 10.0.19041.0 or later (Windows 10 2004)
set(CMAKE_SYSTEM_VERSION 10.0.19041.0)

# Enable C++/WinRT
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /await /std:c++20")

# Windows App SDK / WinUI3 package location
# This will be automatically resolved via NuGet restore
set(WINDOWS_APP_SDK_VERSION "1.5.240311000")

# Project sources
set(PROJECT_SOURCES
    src/Main.cpp
    src/pch.h
    src/pch.cpp
    src/App.xaml.cpp
    src/App.xaml.h
    src/MainWindow.xaml.cpp
    src/MainWindow.xaml.h
    src/ControlEnumerator.cpp
    src/ControlEnumerator.h
    src/ControlInteraction.cpp
    src/ControlInteraction.h
    src/SystemTrayManager.cpp
    src/SystemTrayManager.h
    src/SettingsManager.cpp
    src/SettingsManager.h
    src/SettingsDialog.xaml.cpp
    src/SettingsDialog.xaml.h
    src/AboutDialog.xaml.cpp
    src/AboutDialog.xaml.h
    src/WelcomeDialog.xaml.cpp
    src/WelcomeDialog.xaml.h
)

# XAML files
set(XAML_FILES
    src/App.xaml
    src/MainWindow.xaml
    src/SettingsDialog.xaml
    src/AboutDialog.xaml
    src/WelcomeDialog.xaml
)

# Resource files
set(RESOURCE_FILES
    src/UIAList.rc
    resources/Package.appxmanifest
)

# Create executable
add_executable(UIAList WIN32
    ${PROJECT_SOURCES}
    ${XAML_FILES}
    ${RESOURCE_FILES}
)

# Set Windows-specific properties
set_target_properties(UIAList PROPERTIES
    WIN32_EXECUTABLE TRUE
    OUTPUT_NAME "UIAList"
)

# Preprocessor definitions
target_compile_definitions(UIAList PRIVATE
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
    WINRT_LEAN_AND_MEAN
    UNICODE
    _UNICODE
)

# Include directories
target_include_directories(UIAList PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Link libraries
target_link_libraries(UIAList PRIVATE
    windowsapp
    Microsoft.WindowsAppRuntime
    Microsoft.WindowsAppRuntime.Bootstrap
    UIAutomationCore
    Shell32
    User32
    Gdi32
    ComCtl32
    Ole32
    OleAut32
)

# Copy XAML files to build directory for runtime
foreach(XAML_FILE ${XAML_FILES})
    get_filename_component(XAML_NAME ${XAML_FILE} NAME)
    add_custom_command(TARGET UIAList POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${XAML_FILE}
            $<TARGET_FILE_DIR:UIAList>/${XAML_NAME}
    )
endforeach()

# Installation rules for MSIX packaging
install(TARGETS UIAList
    RUNTIME DESTINATION bin
)

install(FILES
    resources/Package.appxmanifest
    DESTINATION .
)

install(DIRECTORY
    resources/Assets/
    DESTINATION Assets
)

install(FILES
    docs/PrivacyPolicy.html
    DESTINATION .
)

# NuGet package restore configuration
set(NUGET_PACKAGES_CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/packages.config")
set(NUGET_PACKAGES_DIR "${CMAKE_CURRENT_BINARY_DIR}/packages")

# Custom target for NuGet restore
add_custom_target(NuGetRestore
    COMMAND nuget restore ${CMAKE_CURRENT_SOURCE_DIR}/UIAList.sln -PackagesDirectory ${NUGET_PACKAGES_DIR}
    COMMENT "Restoring NuGet packages..."
)

add_dependencies(UIAList NuGetRestore)
