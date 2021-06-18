#pragma once

#include "osgHelper/AbstractFactory.h"

#include <string>
#include <memory>

#include <osg/Object>
#include <osg/Referenced>
#include <osg/Texture2D>
#include <osg/Image>

namespace osgHelper
{
namespace ioc
{
class Injector;
}

class TextureBlueprint : public osg::Referenced
{
public:
	TextureBlueprint();
  ~TextureBlueprint();

	osg::ref_ptr<TextureBlueprint> image(const osg::ref_ptr<osg::Image>& img);

	osg::ref_ptr<TextureBlueprint> texLayer(int texLayer);
  osg::ref_ptr<TextureBlueprint> assign(const osg::ref_ptr<osg::StateSet>& stateSet);
  osg::ref_ptr<TextureBlueprint> uniform(const osg::ref_ptr<osg::StateSet>& stateSet, const std::string& uniformName);

	osg::ref_ptr<osg::Texture2D> build() const;

private:
  struct Impl;
  std::unique_ptr<Impl> m;

};

class TextureFactory : public AbstractFactory<TextureBlueprint>
{
public:
	explicit TextureFactory(ioc::Injector& injector)
	  : AbstractFactory<TextureBlueprint>(injector)
	{
	}
};

}