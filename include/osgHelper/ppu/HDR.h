#pragma once

#include <osgHelper/ppu/Effect.h>
#include <osgHelper/ioc/Injector.h>

#include <osgPPU/Unit.h>

#include <memory>

namespace osgHelper
{
namespace ppu
{

  class HDR : public Effect
	{
	public:
		static const std::string Name;

		HDR(osgHelper::ioc::Injector& injector);
    ~HDR();

		std::string getName() const override;
		InitialUnitList getInitialUnits() const override;
		osg::ref_ptr<osgPPU::Unit> getResultUnit() const override;
		InputToUniformList getInputToUniform() const override;

		void setMidGrey(float midGrey);
		void setBlurSigma(float blurSigma);
		void setBlurRadius(float blurRadius);
		void setGlareFactor(float glareFactor);
		void setAdaptFactor(float adaptFactor);
		void setMinLuminance(float minLuminance);
		void setMaxLuminance(float maxLuminance);

		float getMidGrey() const;
		float getBlurSigma() const;
		float getBlurRadius() const;
		float getGlareFactor() const;
		float getAdaptFactor() const;
		float getMinLuminance() const;
		float getMaxLuminance() const;

	protected:
		Status initializeUnits(const osg::GL2Extensions* extensions) override;

	private:
    struct Impl;
    std::unique_ptr<Impl> m;

	};
}
}
