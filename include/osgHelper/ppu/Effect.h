#pragma once

#include <vector>
#include <string>

#include <osg/Referenced>
#include <osgPPU/Unit.h>

namespace osgHelper
{
namespace ppu
{
	class Effect : public osg::Referenced
	{
	public:
		enum class UnitType
		{
			BypassColor,
			BypassDepth,
			OngoingColor
		};

		struct InitialUnit
		{
      osg::ref_ptr<osgPPU::Unit> unit;
      UnitType                   type;
    };

		struct InputToUniform
		{
      osg::ref_ptr<osgPPU::Unit> unit;
      std::string                name;
      UnitType                   type;
    };

    using InitialUnitList    = std::vector<InitialUnit>;
    using InputToUniformList = std::vector<InputToUniform>;

    Effect();

		void initialize();
		bool isInitialized() const;

    virtual std::string                getName() const         = 0;
    virtual InitialUnitList            getInitialUnits() const = 0;
    virtual osg::ref_ptr<osgPPU::Unit> getResultUnit() const   = 0;
    virtual InputToUniformList         getInputToUniform() const;
    virtual void                       onResizeViewport(const osg::Vec2f& resolution);

  protected:
		virtual void initializeUnits() = 0;

	private:
		bool m_isInitialized;

	};
}
}
