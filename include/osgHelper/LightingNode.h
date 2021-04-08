#pragma once

#include <map>

#include <osg/Group>
#include <osg/Light>
#include <osg/LightModel>
#include <osg/LightSource>

namespace osgHelper
{
  constexpr auto numMaxLights = 8;

  using LightSourceDictionary = std::map<int, osg::ref_ptr<osg::LightSource>>;
	using LightSourceDictPair = std::pair<int, osg::ref_ptr<osg::LightSource>>;

	class LightingNode : public osg::Group
	{
	public:
    using Ptr = osg::ref_ptr<LightingNode>;

		LightingNode();
    ~LightingNode();

    osg::ref_ptr<osg::LightModel> getLightModel() const;

    void setLightEnabled(int lightNum, bool enabled);
		osg::ref_ptr<osg::Light> getOrCreateLight(int lightNum);

	private:
    osg::ref_ptr<osg::LightModel> m_lightModel;
    LightSourceDictionary         m_lightSources;

    void initializeStateSet();

	};
}