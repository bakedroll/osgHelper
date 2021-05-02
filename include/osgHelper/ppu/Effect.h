#pragma once

#include <vector>
#include <string>

#include <osg/Referenced>
#include <osg/GL2Extensions>

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

		enum class InitResult
		{
		  Initialized,
			UnsupportedShaders
		};

		struct Status
		{
      InitResult  result;
      std::string message;
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

    Status initialize(const osg::GL2Extensions* extensions);
    bool   isInitialized() const;
		bool   isSupported() const;

    virtual std::string                getName() const         = 0;
    virtual InitialUnitList            getInitialUnits() const = 0;
    virtual osg::ref_ptr<osgPPU::Unit> getResultUnit() const   = 0;
    virtual InputToUniformList         getInputToUniform() const;
    virtual void                       onResizeViewport(const osg::Vec2f& resolution);

  protected:
		virtual Status initializeUnits(const osg::GL2Extensions* extensions) = 0;

	private:
		bool m_isInitialized;
		bool m_isSupported;

	};
}
}
