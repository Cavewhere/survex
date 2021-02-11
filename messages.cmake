function(svx_add_messages target_name)

    set(message_files_without_dir)
    set(output_files)
    foreach(current IN LISTS ARGN)
        get_filename_component(filename_with_ext ${current} NAME)
        list(APPEND message_files_without_dir ${filename_with_ext})

        get_filename_component(filename ${current} NAME_WE)
       # string(REGEX REPLACE "(.+)_.+" "\\1" filename ${filename})
        list(APPEND output_files ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${filename}.msg)
    endforeach()

    add_custom_command(
        OUTPUT ${output_files} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/en.msg

        WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        COMMAND ${PERL_EXECUTABLE} ${PROJECT_SOURCE_DIR}/${SRC_LIB_DIR}/po-to-msg.pl ${message_files_without_dir}

        DEPENDS ${MESSAGE_FILES}
        )

    add_custom_target(${target_name} DEPENDS ${output_files})



#    set(message_output_files)

#    foreach(infilename IN LISTS ARGN)
#        get_filename_component(filename ${infilename} NAME_WE)

#        string(REGEX REPLACE "(.+)_.+" "\\1" filename ${filename})

#        set(outfilename ${filename}.msg)

#        set(infile_path ${CMAKE_CURRENT_SOURCE_DIR}/${infilename})
#        set(outfile_path ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${outfilename})

#        add_custom_command(
#            OUTPUT ${outfile_path}
#            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
#            COMMAND ${PERL_EXECUTABLE} ${PROJECT_SOURCE_DIR}/${SRC_LIB_DIR}/po-to-msg.pl ${infile}
#            DEPENDS ${infile_path} ${PROJECT_SOURCE_DIR}/${SRC_LIB_DIR}/po-to-msg.pl
#            )

#        list(APPEND message_output_files ${outfile_path})
#    endforeach()

#    add_custom_target(${target_name} DEPENDS ${message_output_files})
endfunction()
