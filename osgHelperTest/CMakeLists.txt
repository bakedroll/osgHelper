begin_project(osgHelperTest EXECUTABLE OPTIONAL)

require_library(GTest)
require_library(OpenSceneGraph MODULES osg osgViewer osgUtil osgGA osgDB osgText OpenThreads)
require_library(osgPPU)

require_project(osgHelper)

add_source_directory(test)
