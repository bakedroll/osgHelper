#pragma once

#include <osgHelper/ITextureBlueprint.h>

#include <memory>

namespace osgHelper
{

class TextureBlueprint : public ITextureBlueprint
{
public:
  TextureBlueprint();
  ~TextureBlueprint() override;

  osg::ref_ptr<ITextureBlueprint> image(const osg::ref_ptr<osg::Image>& img) override;

  osg::ref_ptr<ITextureBlueprint> texLayer(int texLayer) override;
  osg::ref_ptr<ITextureBlueprint> assign(const osg::ref_ptr<osg::StateSet>& stateSet) override;
  osg::ref_ptr<ITextureBlueprint> uniform(const osg::ref_ptr<osg::StateSet>& stateSet, const std::string& uniformName) override;

  osg::ref_ptr<osg::Texture2D> build() const override;

private:
  struct Impl;
  std::unique_ptr<Impl> m;

};

}