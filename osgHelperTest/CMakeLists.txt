set(PROJECT_NAME osgHelperTest)

add_source_directory(OSG_GAMING_TEST_SOURCE test)

add_executable(${PROJECT_NAME} ${OSG_GAMING_TEST_SOURCE})

set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

# specify needed osg libraries
set(TEST_OSG_LIBRARIES_TMP "osg osgViewer osgUtil osgGA osgDB osgText OpenThreads")

get_libraries(TEST_OSG_LIBRARIES ${OSG_LIBRARY_RELEASE} "" ${TEST_OSG_LIBRARIES_TMP})
get_libraries(TEST_OSG_LIBRARIES_DEBUG ${OSG_LIBRARY_DEBUG} ${OSG_LIBRARY_SUFFIX} ${TEST_OSG_LIBRARIES_TMP})

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GTEST_INCLUDE_DIRS}
  ${OSGHELPER_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}
  osgHelper
  ${GTEST_BOTH_LIBRARIES})

target_link_libraries(${PROJECT_NAME}
  optimized ${OSGPPU_LIBRARY_RELEASE})
  
target_link_libraries(${PROJECT_NAME}
  debug ${OSGPPU_LIBRARY_DEBUG})
  
if (MSVC)
  foreach(lib ${TEST_OSG_LIBRARIES})
    target_link_libraries(${PROJECT_NAME}
      optimized ${lib})
  endforeach()

  foreach(lib ${TEST_OSG_LIBRARIES_DEBUG})
    target_link_libraries(${PROJECT_NAME}
      debug ${lib})
  endforeach()
else()
  target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${OPENSCENEGRAPH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif()