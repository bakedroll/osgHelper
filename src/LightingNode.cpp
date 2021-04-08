#include <osgHelper/Helper.h>
#include <osgHelper/LightingNode.h>

namespace osgHelper
{

LightingNode::LightingNode()
  : osg::Group()
{
}

LightingNode::~LightingNode() = default;

osg::ref_ptr<osg::LightModel> LightingNode::getLightModel() const
{
  return m_lightModel;
}

void LightingNode::setLightEnabled(int lightNum, bool enabled)
{
  getOrCreateStateSet()->setMode(GL_LIGHT0 + lightNum, glModeValueFromBool(enabled));

  getOrCreateLight(lightNum);
}

osg::ref_ptr<osg::Light> LightingNode::getOrCreateLight(int lightNum)
{
  const auto it = m_lightSources.find(lightNum);
  if (it == m_lightSources.end())
  {
    auto light       = new osg::Light();
    auto lightSource = new osg::LightSource();

    light->setLightNum(lightNum);
    lightSource->setLight(light);

    addChild(lightSource);
    m_lightSources[lightNum] = lightSource;

    return light;
  }

  return it->second->getLight();
}

void LightingNode::initializeStateSet()
{
  auto stateSet = getOrCreateStateSet();
  m_lightModel = new osg::LightModel();

  stateSet->setMode(GL_CULL_FACE, osg::StateAttribute::ON);
  stateSet->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
  stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
  stateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON);

  for (auto i = 0; i < numMaxLights; i++)
  {
    stateSet->setMode(GL_LIGHT0 + i, osg::StateAttribute::OFF);
  }

  stateSet->setAttributeAndModes(m_lightModel, osg::StateAttribute::ON);
  setStateSet(stateSet);
}

}