cmake_minimum_required(VERSION 3.10)
project(SynthEngineTest)

set(CMAKE_CXX_STANDARD 17)

add_executable(test_engine 
    main.cpp
    ../firmware/src/SynthCore/Engine.cpp
    ../firmware/src/SynthCore/Components.cpp
    ../firmware/src/wavetable_data.cpp
)

target_include_directories(test_engine PRIVATE 
    ../firmware/include
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
target_include_directories(test_engine PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
target_link_libraries(test_engine ${PORTAUDIO_LIBRARIES})
