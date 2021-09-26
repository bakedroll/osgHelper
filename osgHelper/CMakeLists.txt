begin_project(osgHelper LIBRARY)

require_library(OpenSceneGraph)
require_library(osgPPU)

require_project(utilsLib PATH utilsLib)

add_source_directory(src)
add_source_directory(include)

add_include_directory(include)
