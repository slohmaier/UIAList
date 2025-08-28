cmake_minimum_required(VERSION 3.16)

project(UIAList VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES UIAList_en_US.ts)

# Convert SVG icon to PNG and ICO
find_program(INKSCAPE_EXECUTABLE inkscape)
if(NOT INKSCAPE_EXECUTABLE)
    message(WARNING "Inkscape not found. Using pre-generated PNG icon file.")
    set(USE_PREGENERATED_ICON TRUE)
else()
    set(USE_PREGENERATED_ICON FALSE)
endif()

# Convert SVG to PNG for Qt resources
if(NOT USE_PREGENERATED_ICON)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png
        COMMAND ${INKSCAPE_EXECUTABLE} --export-type=png --export-filename=${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png --export-width=32 --export-height=32 ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.svg
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.svg
        COMMENT "Converting SVG icon to PNG"
    )
else()
    # Copy pre-generated PNG file
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.png ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.png
        COMMENT "Using pre-generated PNG icon"
    )
endif()

# Generate ICO file (Windows only)
if(WIN32)
    # Use ImageMagick to create proper ICO file
    find_program(MAGICK_EXECUTABLE magick)
    if(MAGICK_EXECUTABLE)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.ico
            COMMAND ${MAGICK_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.ico
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png
            COMMENT "Creating ICO from PNG using ImageMagick"
        )
    else()
        # Fallback: copy PNG to ICO (will cause resource compiler error)
        add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.ico
            COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.ico
            DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png
            COMMENT "Creating ICO from PNG (fallback method)"
        )
        message(WARNING "ImageMagick not found. ICO file may not be properly formatted for Windows resource compiler.")
    endif()
    
    add_custom_target(convert_icon ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.ico)
else()
    add_custom_target(convert_icon ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png)
endif()

# Create Qt resource file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc.in ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc @ONLY)

set(RESOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc)
set_source_files_properties(${RESOURCE_FILES} PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png)

set(PROJECT_SOURCES
        main.cpp
        uialist.cpp
        uialist.h
        uialisticon.cpp
        uialisticon.h
        aboutdialog.cpp
        aboutdialog.h
        settingsdialog.cpp
        settingsdialog.h
        welcomedialog.cpp
        welcomedialog.h
        ${RESOURCE_FILES}
        ${TS_FILES}
)

# Add Windows resource file for executable icon
if(WIN32)
    list(APPEND PROJECT_SOURCES UIAList.rc)
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(UIAList
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET UIAList APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(UIAList SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(UIAList
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(UIAList PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# Ensure PNG is generated before compiling resources
add_dependencies(UIAList convert_icon)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.UIAList)
endif()
set_target_properties(UIAList PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

# Windows-specific packaging configurations
if(WIN32)
    # Set Windows-specific properties
    set_target_properties(UIAList PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "UIAList"
    )
    
    # Enable Windows 10/11 manifest support
    if(MSVC)
        target_compile_definitions(UIAList PRIVATE 
            WINVER=0x0A00
            _WIN32_WINNT=0x0A00
        )
    endif()
    
    # Install executable and required Qt libraries
    install(TARGETS UIAList
        RUNTIME DESTINATION bin
    )
    
    # Install Qt libraries (for deployment)
    if(QT_VERSION_MAJOR EQUAL 6)
        find_program(QT_DEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    else()
        find_program(QT_DEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt5_DIR}/../../../bin)
    endif()
    
    if(QT_DEPLOYQT_EXECUTABLE)
        # Post-build step to deploy Qt libraries to build directory
        add_custom_command(TARGET UIAList POST_BUILD
            COMMAND ${QT_DEPLOYQT_EXECUTABLE} $<TARGET_FILE:UIAList>
                $<$<CONFIG:Debug>:--debug>$<$<CONFIG:Release>:--release> --no-translations --no-system-d3d-compiler --no-opengl-sw
            COMMENT "Deploying Qt libraries to build directory"
        )
        
        # Install step for deployment
        install(CODE "
            execute_process(
                COMMAND ${QT_DEPLOYQT_EXECUTABLE} \${CMAKE_INSTALL_PREFIX}/bin/UIAList.exe
                --release --no-translations --no-system-d3d-compiler --no-opengl-sw
            )
        ")
    endif()
    
    # Copy Package.appxmanifest for MSIX packaging
    install(FILES 
        ${CMAKE_SOURCE_DIR}/Package.appxmanifest
        DESTINATION .
    )
    
    # Copy Assets directory for MSIX packaging
    install(DIRECTORY 
        ${CMAKE_SOURCE_DIR}/Assets/
        DESTINATION Assets
        PATTERN "*.md" EXCLUDE
    )
    
    # Copy privacy policy
    install(FILES 
        ${CMAKE_SOURCE_DIR}/PrivacyPolicy.html
        DESTINATION .
    )
    
else()
    # Non-Windows install
    install(TARGETS UIAList
        BUNDLE DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(UIAList)
endif()
