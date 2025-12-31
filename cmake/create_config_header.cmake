function(create_config_header targetName)
    configure_file(
            cmake/sfdm_config.hpp.in
            ${CMAKE_CURRENT_BINARY_DIR}/include/sfdm/sfdm_config.hpp
            @ONLY
    )
    target_include_directories(${targetName}
            PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    )
endfunction()