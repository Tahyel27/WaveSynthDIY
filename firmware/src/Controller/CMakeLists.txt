add_library(Controller Controller.cpp Sequencer.cpp ../InstrumentManager.cpp)
target_include_directories(Controller PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../include)
target_link_libraries(Controller PUBLIC Engine pico_stdlib)