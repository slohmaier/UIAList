cmake_minimum_required(VERSION 3.16)

project(UIAList VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

set(TS_FILES UIAList_en_US.ts)

# Convert SVG icon to PNG
find_program(INKSCAPE_EXECUTABLE inkscape)
if(INKSCAPE_EXECUTABLE)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png
        COMMAND ${INKSCAPE_EXECUTABLE} --export-type=png --export-filename=${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png --export-width=32 --export-height=32 ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.svg
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/uialist_icon.svg
        COMMENT "Converting SVG icon to PNG"
    )
    add_custom_target(convert_icon ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png)
    
    # Create Qt resource file
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc.in ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc @ONLY)
    
    set(RESOURCE_FILES ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc)
    set_source_files_properties(${RESOURCE_FILES} PROPERTIES OBJECT_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/uialist_icon.png)
else()
    message(WARNING "Inkscape not found. SVG icon will not be converted to PNG automatically.")
    set(RESOURCE_FILES "")
endif()

set(PROJECT_SOURCES
        main.cpp
        uialist.cpp
        uialist.h
        uialisticon.cpp
        uialisticon.h
        aboutdialog.cpp
        aboutdialog.h
        ${RESOURCE_FILES}
        ${TS_FILES}
)

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
if(INKSCAPE_EXECUTABLE)
    add_dependencies(UIAList convert_icon)
endif()

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
