file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")

foreach (test_source ${TEST_SOURCES})
    get_filename_component(test_name ${test_source} NAME_WE)
    add_executable(${test_name} ${test_source})
    target_link_libraries(${test_name} PRIVATE mymuduo)
endforeach ()

#add_executable(test_logger test_logger.cpp ../src/Timestamp.cpp)
#target_include_directories(test_logger PUBLIC ../include)