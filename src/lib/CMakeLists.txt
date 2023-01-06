
set( LIB_NAME "glbleapi" )

# debug log
# add_definitions(-DENABLE_DEBUG )

# lib c file
aux_source_directory(./ SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/bledriver/silabs SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/bledriver/util SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/components/dev_mgr SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/components/log SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/components/thread SOURCES)
aux_source_directory(${PROJECT_SOURCE_DIR}/components/timestamp SOURCES)

# compile h file
include_directories(./include )
include_directories( ${PROJECT_SOURCE_DIR}/bledriver/model_hw_cfg )
include_directories( ${PROJECT_SOURCE_DIR}/bledriver/silabs )
include_directories( ${PROJECT_SOURCE_DIR}/bledriver/util )
include_directories( ${PROJECT_SOURCE_DIR}/components/dev_mgr )
include_directories( ${PROJECT_SOURCE_DIR}/components/log )
include_directories( ${PROJECT_SOURCE_DIR}/components/thread )
include_directories( ${PROJECT_SOURCE_DIR}/components/timestamp )

# lib h file
set( HEADERS ./include/gl_bleapi.h
				./include/gl_errno.h
				./include/gl_type.h )

add_compile_options(-fPIC)

add_library( ${LIB_NAME} SHARED ${SOURCES} )
add_library( lib_static STATIC ${SOURCES} )
SET_TARGET_PROPERTIES(lib_static PROPERTIES OUTPUT_NAME "glbleapi")

target_link_libraries( ${LIB_NAME} PRIVATE pthread uci guci)

install(
	TARGETS ${LIB_NAME}
	DESTINATION lib
)

install(
	TARGETS lib_static
	DESTINATION lib
)

install(
	FILES ${HEADERS}
	DESTINATION include
)