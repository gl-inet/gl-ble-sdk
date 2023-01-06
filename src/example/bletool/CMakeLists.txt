
set( EXEC_NAME "bletool" )

aux_source_directory(./ SOURCES)

include_directories(./ )
include_directories( ${PROJECT_SOURCE_DIR}/lib/include )
include_directories( ${PROJECT_SOURCE_DIR}/bledriver/silabs )


add_executable( ${EXEC_NAME} ${SOURCES} )

# link_directories( ${PROJECT_SOURCE_DIR}/build/lib )
target_link_libraries( ${EXEC_NAME} PRIVATE json-c glbleapi readline ncurses )


install( 
    TARGETS ${EXEC_NAME} 
    RUNTIME DESTINATION bin 
)
