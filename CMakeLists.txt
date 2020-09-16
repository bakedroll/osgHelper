set(PROJECT_NAME osgHelper)

add_source_directory(OSGHELPER_SOURCE src)
add_source_directory(OSGHELPER_SOURCE src/ioc)
add_source_directory(OSGHELPER_SOURCE src/ppu)
add_source_directory(OSGHELPER_SOURCE include/osgHelper)
add_source_directory(OSGHELPER_SOURCE include/osgHelper/ioc)
add_source_directory(OSGHELPER_SOURCE include/osgHelper/ppu)

set(OSGHELPER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${PROJECT_NAME} ${OSGHELPER_SOURCE})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")
target_compile_options(${PROJECT_NAME} PRIVATE "-fpermissive")

# set include directories
target_include_directories(${PROJECT_NAME} PUBLIC
  ${OSGHELPER_INCLUDE_DIR}
  ${OSG_INCLUDE_DIR}
  ${OSGPPU_INCLUDE_DIR})
  
# make variable globally accessable
set(OSGHELPER_INCLUDE_DIR ${OSGHELPER_INCLUDE_DIR} PARENT_SCOPE)