set(EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH}/examples)

add_custom_target(examples)
set(BOOST_ROOT "/usr/local/boost_1_68_0")
find_package(Boost 1.68.0 COMPONENTS REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

FILE(GLOB EXAMPLES_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
foreach (examplesourcefile ${EXAMPLES_SOURCE_FILES})
    # I used a simple string replace, to cut off .cpp.
    string(REPLACE ".cpp" "" examplename ${examplesourcefile})
    add_executable(${examplename} EXCLUDE_FROM_ALL ${examplesourcefile})
    # Make sure YourLib is linked to each app
    target_link_libraries(${examplename} gphoto2pp ${LIBS})
    target_link_libraries(${examplename} LINK_PUBLIC ${Boost_LIBRARIES})
    # Adds this example to the `make examples` custom target
    add_dependencies(examples ${examplename})
endforeach (examplesourcefile ${EXAMPLES_SOURCE_FILES})
