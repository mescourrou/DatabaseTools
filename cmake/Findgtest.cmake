find_package(Threads REQUIRED)
if (NOT TARGET googletest)

INCLUDE(ExternalProject)


ExternalProject_Add(
	googletest

	DOWNLOAD_COMMAND ""
    SOURCE_DIR "${DB_TOOLS_THIRDPARTY_DIR}/gtest"
    PREFIX "${CMAKE_BINARY_DIR}/gtest"
	INSTALL_COMMAND ""
	)


endif()
# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(googletest source_dir binary_dir)


if (NOT TARGET gtest)
# Create a libgtest target to be used as a dependency by test programs
add_library(gtest STATIC IMPORTED GLOBAL)
add_dependencies(gtest googletest)

# Set libgtest properties
set_target_properties(gtest PROPERTIES
	"IMPORTED_LOCATION" "${binary_dir}/lib/libgtest.a"
	"IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
    INTERFACE_INCLUDE_DIRECTORIES "${source_dir}/googletest/include"
)
endif()
if (NOT TARGET gmock)
# Create a libgmock target to be used as a dependency by test programs
add_library(gmock STATIC IMPORTED GLOBAL)
add_dependencies(gmock googletest)

# Set libgmock properties
set_target_properties(gmock PROPERTIES
	"IMPORTED_LOCATION" "${binary_dir}/lib/libgmock.a"
	"IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
	INTERFACE_INCLUDE_DIRECTORIES "${source_dir}/googlemock/include"
)


endif()
