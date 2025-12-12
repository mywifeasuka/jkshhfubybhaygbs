#message("ClassExampleProject dir")

cmake_minimum_required(VERSION 3.10)  #cmake版本最低3.10

# 获取当前子CMake文件所在目录的完整路径 
get_filename_component(CURRENT_DIR_PATH "${CMAKE_CURRENT_LIST_DIR}" ABSOLUTE) 
# 从完整路径中提取目录名称 
get_filename_component(CURRENT_DIR_NAME "${CURRENT_DIR_PATH}" NAME) 
project(${CURRENT_DIR_NAME})

set(CMAKE_CXX_STANDARD 11) 

if(MSVC)
	add_compile_options(/Zc:__cplusplus)
endif()

SET(CMAKE_INCLUDE_CURRENT_DIR ON)

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
SET(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Widgets Multimedia Gui REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    gamewidget.h
    gamewidget.cpp
    mole.h
    mole.cpp
    gamesettings.h
    gamesettings.cpp
    imagebutton.h     
    imagebutton.cpp  
    gamebase.h
    policegame.h
    policegame.cpp 
    molegame.cpp
    spacegame.cpp
    datamanager.cpp
    applegame.cpp
)

set(PROJECT_RESOURCES
    resources.qrc
)

qt5_add_resources(PROJECT_RESOURCES_RCC ${PROJECT_RESOURCES})


# file(GLOB UI_FILES ...)
# qt5_wrap_ui(UI_HEADERS ${UI_FILES})



add_executable(${PROJECT_NAME} WIN32
    ${PROJECT_SOURCES}
    ${PROJECT_RESOURCES_RCC}
    # ${UI_HEADERS} 
)


target_link_libraries(${PROJECT_NAME} 
	Qt5::Widgets
	Qt5::Core
	Qt5::Gui
    Qt5::Multimedia
)



