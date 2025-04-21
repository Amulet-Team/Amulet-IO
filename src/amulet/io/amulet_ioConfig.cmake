if (NOT TARGET amulet_io)
    message(STATUS "Finding amulet_io")

    set(amulet_io_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")

    add_library(amulet_io INTERFACE)
    set_target_properties(amulet_io PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${amulet_io_INCLUDE_DIR}"
    )
endif()
