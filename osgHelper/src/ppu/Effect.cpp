#include <osgHelper/ppu/Effect.h>

namespace osgHelper::ppu
{

Effect::Effect()
	: Referenced()
	, m_isInitialized(false)
  , m_isSupported(true)
{

}

Effect::Status Effect::initialize(const osg::GL2Extensions* extensions)
{
	Status status = { InitResult::Initialized, "" };

	if (!m_isInitialized)
	{
		status = initializeUnits(extensions);

		if (status.result == InitResult::Initialized)
		{
			m_isInitialized = true;
		}
		else
		{
			m_isSupported = false;
		}
	}

	return status;
}

bool Effect::isInitialized() const
{
	return m_isInitialized;
}

bool Effect::isSupported() const
{
	return m_isSupported;
}

Effect::InputToUniformList Effect::getInputToUniform() const
{
	return InputToUniformList();
}

void Effect::onResizeViewport(const osg::Vec2f& resolution)
{
	
}

}
