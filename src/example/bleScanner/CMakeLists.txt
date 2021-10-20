
set( EXEC_NAME "bleScanner" )

aux_source_directory(./ SOURCES)

include_directories(./ )
include_directories( ${PROJECT_SOURCE_DIR}/lib/include )


add_executable( ${EXEC_NAME} ${SOURCES} )

# link_directories( ${PROJECT_SOURCE_DIR}/build/lib )
target_link_libraries( ${EXEC_NAME} PRIVATE json-c glbleapi )


install( 
    TARGETS ${EXEC_NAME} 
    RUNTIME DESTINATION bin 
)
