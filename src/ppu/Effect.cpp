#include <osgHelper/ppu/Effect.h>

namespace osgHelper
{
namespace ppu
{

	Effect::Effect()
		: Referenced()
		, m_isInitialized(false)
	{

	}

	void Effect::initialize()
	{
		if (!m_isInitialized)
		{
			initializeUnits();
			m_isInitialized = true;
		}
	}

	bool Effect::isInitialized() const
	{
		return m_isInitialized;
	}

	Effect::InputToUniformList Effect::getInputToUniform() const
	{
		return InputToUniformList();
	}

}
}