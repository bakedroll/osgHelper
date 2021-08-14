#include <osgHelper/TextureFactory.h>
#include <osgHelper/TextureBlueprint.h>

namespace osgHelper
{

TextureFactory::TextureFactory(ioc::Injector& injector)
  : ITextureFactory()
{
}

osg::ref_ptr<ITextureBlueprint> TextureFactory::make() const
{
  return new TextureBlueprint();
}

}
