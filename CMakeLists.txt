cmake_minimum_required(VERSION 3.5)


project (recog-demo)

# c++11 required
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")



# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
# OpenVC3 required
find_package(OpenCV 3.3.1 REQUIRED)

## find_package(Boost 1.5.0 REQUIRED)
find_package(Boost 1.5.0 COMPONENTS system filesystem)

message("${Qt5Widgets_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES( 
    /usr/local/cuda/include
    ${Qt5Widgets_INCLUDE_DIRS}
    /usr/local/include	
    ../../src

)

LINK_DIRECTORIES( 
    /usr/local/cuda/lib64  /usr/local/tensorrt/lib /usr/local/lib
)


add_executable(recog-demo  main.cpp recogdemo.cpp recogdemo.ui)

target_link_libraries(recog-demo ${OpenCV_LIBS} ${Boost_LIBRARIES} ${Qt5Widgets_LIBRARIES} core opencv_freetype cudnn cublas cuda cuda curand  )
