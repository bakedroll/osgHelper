#pragma once

#include <osgHelper/ITextureFactory.h>
#include <osgHelper/ioc/Injector.h>

namespace osgHelper
{

class TextureFactory : public ITextureFactory
{
public:
	explicit TextureFactory(ioc::Injector& injector);

  osg::ref_ptr<ITextureBlueprint> make() const override;
};

}
