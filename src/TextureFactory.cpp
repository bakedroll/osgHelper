#include <osgHelper/TextureFactory.h>

namespace osgHelper
{

struct TextureBlueprint::Impl
{
  Impl() 
    : texLayer(0)
    , dataVariance(osg::Object::DYNAMIC)
    , wrapS(osg::Texture::WrapMode::CLAMP_TO_EDGE),
      wrapT(osg::Texture::WrapMode::CLAMP_TO_EDGE)
    , minFilter(osg::Texture::FilterMode::LINEAR_MIPMAP_LINEAR),
      magFilter(osg::Texture::FilterMode::LINEAR)
    , maxAnisotropy(8.0f)
  {}

  struct BpUniform
  {
    std::string                 uniformName;
    osg::ref_ptr<osg::StateSet> stateSet;
  };

  using BpUniformList = std::vector<BpUniform>;
  using StateSetList  = std::vector<osg::ref_ptr<osg::StateSet>>;

  osg::ref_ptr<osg::Image> image;

  int texLayer;

  osg::Object::DataVariance dataVariance;
  osg::Texture::WrapMode wrapS;
  osg::Texture::WrapMode wrapT;
  osg::Texture::FilterMode minFilter;
  osg::Texture::FilterMode magFilter;

  float maxAnisotropy;

  BpUniformList bpUniforms;
  StateSetList assignToStateSets;
};

TextureBlueprint::TextureBlueprint()
  : Referenced()
  , m(new Impl())
{

}

TextureBlueprint::~TextureBlueprint() = default;

osg::ref_ptr<TextureBlueprint> TextureBlueprint::image(const osg::ref_ptr<osg::Image>& img)
{
  m->image = img;

  return this;
}

osg::ref_ptr<TextureBlueprint> TextureBlueprint::texLayer(int texLayer)
{
  m->texLayer = texLayer;

  return this;
}

osg::ref_ptr<TextureBlueprint> TextureBlueprint::assign(const osg::ref_ptr<osg::StateSet>& stateSet)
{
  m->assignToStateSets.push_back(stateSet);

  return this;
}

osg::ref_ptr<TextureBlueprint> TextureBlueprint::uniform(const osg::ref_ptr<osg::StateSet>& stateSet,
                                                         const std::string&                 uniformName)
{
  Impl::BpUniform uniform;
  uniform.stateSet    = stateSet;
  uniform.uniformName = uniformName;

  m->bpUniforms.push_back(uniform);

  return this;
}

osg::ref_ptr<osg::Texture2D> TextureBlueprint::build() const
{
  auto texture = new osg::Texture2D();
  texture->setDataVariance(m->dataVariance);
  texture->setWrap(osg::Texture::WRAP_S, m->wrapS);
  texture->setWrap(osg::Texture::WRAP_T, m->wrapT);
  texture->setFilter(osg::Texture::MIN_FILTER, m->minFilter);
  texture->setFilter(osg::Texture::MAG_FILTER, m->magFilter);
  texture->setMaxAnisotropy(m->maxAnisotropy);

  if (m->image.valid())
  {
    texture->setImage(m->image);
  }

  for (const auto& stateSet : m->assignToStateSets)
  {
    stateSet->setTextureAttributeAndModes(m->texLayer, texture, osg::StateAttribute::ON);
  }

  for (const auto& uniform : m->bpUniforms)
  {
    auto uf = new osg::Uniform(osg::Uniform::SAMPLER_2D, uniform.uniformName);
    uf->set(m->texLayer);
    uniform.stateSet->addUniform(uf);
  }

  return texture;
}

}