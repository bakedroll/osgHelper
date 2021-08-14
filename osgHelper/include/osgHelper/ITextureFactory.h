#pragma once

#include <osg/Referenced>
#include <osg/ref_ptr>

#include <osgHelper/ITextureBlueprint.h>

namespace osgHelper
{

class ITextureFactory : public osg::Referenced
{
public:
  ITextureFactory() = default;
  virtual ~ITextureFactory() = default;

  virtual osg::ref_ptr<ITextureBlueprint> make() const = 0;
};

}