find_package(gtest REQUIRED)
include(GoogleTest)

add_executable(Database_test
    DatabaseTest.cpp DatabaseTest.hpp
    QueryTest.cpp QueryTest.hpp)
target_link_libraries(Database_test gtest DatabaseTools ${FILESYSTEM_LIB})
if (BUILD_USE_FILESYSTEM)
    target_compile_definitions(Database_test PUBLIC BUILD_USE_FILESYSTEM)
endif()
gtest_discover_tests(Database_test)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/data" AND IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data")
add_custom_command(TARGET Database_test POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
     "${CMAKE_CURRENT_SOURCE_DIR}/data"
     "${CMAKE_CURRENT_BINARY_DIR}/data"
  COMMENT "Copying test data output directory")
endif()

