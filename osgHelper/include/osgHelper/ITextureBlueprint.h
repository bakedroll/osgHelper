#pragma once

#include <osg/Referenced>
#include <osg/StateSet>
#include <osg/Image>
#include <osg/Texture2D>

namespace osgHelper
{

class ITextureBlueprint : public osg::Referenced
{
public:
  ITextureBlueprint() = default;
  virtual ~ITextureBlueprint() = default;

  virtual osg::ref_ptr<ITextureBlueprint> image(const osg::ref_ptr<osg::Image>& img) = 0;

  virtual osg::ref_ptr<ITextureBlueprint> texLayer(int texLayer) = 0;
  virtual osg::ref_ptr<ITextureBlueprint> assign(const osg::ref_ptr<osg::StateSet>& stateSet) = 0;
  virtual osg::ref_ptr<ITextureBlueprint> uniform(const osg::ref_ptr<osg::StateSet>& stateSet, const std::string& uniformName) = 0;
  virtual osg::ref_ptr<ITextureBlueprint> minFilter(osg::Texture::FilterMode filterMode) = 0;
  virtual osg::ref_ptr<ITextureBlueprint> magFilter(osg::Texture::FilterMode filterMode) = 0;

  virtual osg::ref_ptr<osg::Texture2D> build() const = 0;

};

}