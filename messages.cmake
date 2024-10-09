function(svx_add_messages target_name)
    set(output_files)
    set(absolute_po_files)
    foreach(current IN LISTS ARGN)
        # Get the absolute path of the .po file
        get_filename_component(absolute_po_file ${current} ABSOLUTE)
        list(APPEND absolute_po_files ${absolute_po_file})

        # Get the filename without extension
        get_filename_component(filename ${current} NAME_WE)
        # Define the output .msg file path
        set(output_file ${PROJECT_BINARY_DIR}/${filename}.msg)
        message(STATUS "Output msg:" ${output_file})
        list(APPEND output_files ${output_file})
    endforeach()

    # Ensure the output directory exists
    # file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    add_custom_command(
        OUTPUT ${output_files}
        COMMAND ${PERL_EXECUTABLE} ${PROJECT_SOURCE_DIR}/${SRC_LIB_DIR}/po-to-msg.pl ${absolute_po_files}
        DEPENDS ${absolute_po_files} ${PROJECT_SOURCE_DIR}/${SRC_LIB_DIR}/po-to-msg.pl
        WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
        VERBATIM
    )

    add_custom_target(${target_name} DEPENDS ${output_files})
endfunction()
