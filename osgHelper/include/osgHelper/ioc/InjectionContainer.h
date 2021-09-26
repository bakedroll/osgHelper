#pragma once

#include <osgHelper/ioc/PointerTypeDefinition.h>
#include <utilsLib/InjectionContainer.h>

#include <osg/ref_ptr>
#include <osg/Referenced>

namespace osgHelper
{
namespace ioc
{

using InjectionContainer = utilsLib::InjectionContainer<osg::ref_ptr<osg::Referenced>>;

}
}
