cmake_minimum_required(VERSION 3.16)

project(CANHub VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml SerialBus)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(appCANHub
    main.cpp
    src/transport/busengine.cpp
    src/canopen/sdoclient.cpp
    src/canopen/nmtmaster.cpp
    src/canopen/pdoengine.cpp
    src/canopen/heartbeatmonitor.cpp
    src/canopen/nodemanager.cpp
    src/ui/nodelistmodel.cpp
)

qt_add_qml_module(appCANHub
    URI CANHub
    QML_FILES
        Main.qml
        SOURCES src/transport/busengine.h src/transport/busengine.cpp
        SOURCES src/canopen/sdoclient.h src/canopen/sdoclient.cpp
        SOURCES src/canopen/nmtmaster.h src/canopen/nmtmaster.cpp
        SOURCES src/canopen/pdoengine.h src/canopen/pdoengine.cpp
        SOURCES src/canopen/heartbeatmonitor.h src/canopen/heartbeatmonitor.cpp
        SOURCES src/canopen/NodeSession.h
        SOURCES src/canopen/nodemanager.h src/canopen/nodemanager.cpp
        SOURCES src/ui/nodelistmodel.h src/ui/nodelistmodel.cpp
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appCANHub PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appCANHub
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appCANHub
    PRIVATE Qt6::Core
    PRIVATE Qt6::Quick
    PRIVATE Qt6::Qml
    PRIVATE Qt6::SerialBus
)

install(TARGETS appCANHub
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
