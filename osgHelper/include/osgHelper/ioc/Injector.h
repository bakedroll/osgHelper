#pragma once

#include <osgHelper/ioc/PointerTypeDefinition.h>
#include <utilsLib/Injector.h>

#include <osg/ref_ptr>
#include <osg/Referenced>

namespace osgHelper
{
namespace ioc
{

using Injector = utilsLib::Injector<osg::ref_ptr<osg::Referenced>>;

}
}
