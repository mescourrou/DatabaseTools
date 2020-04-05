if (NOT TARGET sqlite_project)

INCLUDE(ExternalProject)

ExternalProject_Add(
    sqlite_project
    DOWNLOAD_COMMAND ""
    SOURCE_DIR "${DB_TOOLS_THIRDPARTY_DIR}/sqlite3"
    BINARY_DIR "${CMAKE_BINARY_DIR}/sqlite3-build"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
    )


endif()
#ExternalProject_Get_Property(sqlite_project source_dir binary_dir)


if (NOT TARGET sqlite)
    execute_process(COMMAND "${CMAKE_COMMAND}" --build .
        WORKING_DIRECTORY "${DB_TOOLS_THIRDPARTY_DIR}/sqlite3"
    )

    # Add googletest directly to our build. This adds the following targets:
    # gtest, gtest_main, gmock and gmock_main
    add_subdirectory("${DB_TOOLS_THIRDPARTY_DIR}/sqlite3"
    )

    # The gtest/gmock targets carry header search path dependencies
    # automatically when using CMake 2.8.11 or later. Otherwise we
    # have to add them here ourselves.
    if(CMAKE_VERSION VERSION_LESS 2.8.11)
        include_directories("${DB_TOOLS_THIRDPARTY_DIR}/sqlite3"
        )
    endif()

    #add_library(sqlite STATIC IMPORTED GLOBAL)
#add_dependencies(sqlite sqlite_project)

#list(APPEND sqlite_includes "${source_dir}")

#set_target_properties(sqlite PROPERTIES
#    "IMPORTED_LOCATION" "${binary_dir}/libsqlite3.so"
#	INTERFACE_INCLUDE_DIRECTORIES "${sqlite_includes}"
#)
endif()
