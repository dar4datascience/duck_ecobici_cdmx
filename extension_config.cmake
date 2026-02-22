# This file is included by DuckDB's build system. It specifies which extension to load

# Extension from this repo
duckdb_extension_load(ecobici
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}
    LOAD_TESTS
    DESCRIPTION "DuckDB extension for querying Ecobici (CDMX bike-sharing) data"
    DONT_LINK
)

# Any extra extensions that should be built
# e.g.: duckdb_extension_load(json)