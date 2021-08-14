#pragma once

#include <osgHelper/ITextureFactory.h>

namespace osgHelper
{
namespace ioc
{
class Injector;
}

class TextureFactory : public ITextureFactory
{
public:
	explicit TextureFactory(ioc::Injector& injector);

  osg::ref_ptr<ITextureBlueprint> make() const override;
};

}