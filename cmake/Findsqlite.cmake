if (NOT TARGET sqlite_project)

INCLUDE(ExternalProject)

ExternalProject_Add(
	sqlite_project
	DOWNLOAD_COMMAND ""
    SOURCE_DIR "${DB_TOOLS_THIRDPARTY_DIR}/sqlite3"
	PREFIX "${CMAKE_BINARY_DIR}/sqlite3"
	INSTALL_COMMAND ""
	)


endif()
ExternalProject_Get_Property(sqlite_project source_dir binary_dir)


if (NOT TARGET sqlite)
add_library(sqlite SHARED IMPORTED GLOBAL)
add_dependencies(sqlite sqlite_project)

list(APPEND sqlite_includes "${source_dir}")

set_target_properties(sqlite PROPERTIES
	"IMPORTED_LOCATION" "${binary_dir}/libsqlite3.so"
	INTERFACE_INCLUDE_DIRECTORIES "${sqlite_includes}"
)
endif()
