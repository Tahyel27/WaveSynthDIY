add_library(Engine Engine.cpp Components.cpp ../wavetable_data.cpp)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../include)