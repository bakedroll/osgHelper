#pragma once

#include <memory>

#include <osgHelper/ppu/Effect.h>

#include <osgHelper/ioc/Injector.h>

namespace osgHelper
{
namespace ppu
{

  class DOF : public Effect
	{
	public:
		static const std::string Name;

		DOF(osgHelper::ioc::Injector& injector);
    ~DOF();

		std::string getName() const override;
		InitialUnitList getInitialUnits() const override;
		osg::ref_ptr<osgPPU::Unit> getResultUnit() const override;
		InputToUniformList getInputToUniform() const override;

		void setGaussSigma(float gaussSigma);
		void setGaussRadius(float gaussRadius);
		void setFocalLength(float focalLength);
		void setFocalRange(float focalRange);
		void setZNear(float zNear);
		void setZFar(float zFar);

		float getGaussSigma() const;
		float getGaussRadius() const;
		float getFocalLength() const;
		float getFocalRange() const;
		float getZNear() const;
		float getZFar() const;

	protected:
		Status initializeUnits(const osg::GL2Extensions* extensions) override;

	private:
    struct Impl;
    std::unique_ptr<Impl> m;

	};

}
}