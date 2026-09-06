function(gibs_add_sample_test sample_name sample_input)
    set(sample_working_directory "${CMAKE_CURRENT_BINARY_DIR}/${sample_name}")
    file(MAKE_DIRECTORY "${sample_working_directory}")

    add_test(
        NAME "gibs_sample_${sample_name}"
        COMMAND $<TARGET_FILE:gibs> "${CMAKE_SOURCE_DIR}/samples/${sample_input}"
        WORKING_DIRECTORY "${sample_working_directory}"
    )
endfunction()

gibs_add_sample_test(feature feature)
gibs_add_sample_test(folder folder)
gibs_add_sample_test(ifdefs ifdefs)
gibs_add_sample_test(multiple-files multiple-files)
gibs_add_sample_test(multiple-files-and-libs multiple-files-and-libs)
gibs_add_sample_test(single-file single-file)
gibs_add_sample_test(single-project-file single-project-file/project.gibs)
gibs_add_sample_test(subproject subproject/main.gibs)
gibs_add_sample_test(subproject-static subproject-static/main.gibs)

add_custom_target(sample_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -R "^gibs_sample_"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    DEPENDS gibs
    USES_TERMINAL
)