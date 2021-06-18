#pragma once

#include <cmath>

#include <osg/Referenced>
#include <osg/Vec2f>
#include <osg/Vec3f>
#include <osg/Vec4f>

namespace osgHelper
{
	enum class AnimationEase
	{
		CIRCLE_IN,
		CIRCLE_OUT,
		CIRCLE_IN_OUT,
		LINEAR,
		SMOOTH,
		SMOOTHER
	};

	template <typename T>
	class Animation : public osg::Referenced
	{
	public:
		Animation()
      : Animation(T(), 0.0, AnimationEase::SMOOTH)
    {
    }

		Animation(T value, double duration, AnimationEase ease)
      : osg::Referenced()
      , m_value(value)
      , m_fromValue(value)
	    , m_toValue(T())
      , m_duration(duration)
      , m_timeBegin(0)
      , m_animationStarted(false)
      , m_ease(ease)
    {
    }

		void beginAnimation(T fromValue, T toValue, double timeBegin)
		{
      m_fromValue = fromValue;
      m_value     = fromValue;
      m_toValue   = toValue;
      m_timeBegin = timeBegin;

      m_animationStarted = true;
		}

		void setValue(T value)
		{
			m_value = value;
			m_animationStarted = false;
		}

		void beginAnimation(T toValue, double timeBegin)
		{
			beginAnimation(m_value, toValue, timeBegin);
		}

		bool running() const
		{
			return m_animationStarted;
		}

		T getValue(double time)
		{
			if (m_animationStarted)
			{
				double elapsed = (time - m_timeBegin) / m_duration;
				if (elapsed < 1.0)
				{
					switch (m_ease)
					{
					case AnimationEase::CIRCLE_OUT:

						elapsed = sqrt(1.0 - (elapsed * elapsed - 2 * elapsed + 1));
						break;

					case AnimationEase::CIRCLE_IN:

						elapsed = 1.0 - sqrt(1.0 - elapsed * elapsed);
						break;

					case AnimationEase::CIRCLE_IN_OUT:

						if (elapsed < 0.5)
						{
							elapsed = (1.0 - sqrt(1.0 - (4.0 * elapsed * elapsed))) * 0.5;
						}
						else
						{
							elapsed = sqrt(8.0 * elapsed - 4 * elapsed * elapsed - 3.0) * 0.5 + 0.5;
						}
						break;

					case AnimationEase::SMOOTH:

						elapsed = elapsed * elapsed * (3.0 - 2.0 * elapsed);
						break;

					case AnimationEase::SMOOTHER:

						elapsed = elapsed * elapsed * elapsed * (elapsed * (elapsed * 6.0 - 15.0) + 10.0);
						break;
          default:
					  break;
          }

					m_value = relocate(m_fromValue, m_toValue, elapsed);
				}
				else
				{
					m_value = m_toValue;
					m_animationStarted = false;
				}
			}

			return m_value;
		}

		void setDuration(double duration)
		{
			m_duration = duration;
		}

		void setFromValue(T fromValue)
		{
			m_fromValue = fromValue;
			m_value = fromValue;
		}

		void setEase(AnimationEase ease)
		{
			m_ease = ease;
		}

	protected:
		virtual T relocate(const T& from, const T& to, float elapsed)
		{
			return from + (to - from) * elapsed;
		}

	private:
		T m_value;
		T m_fromValue;
		T m_toValue;

		double m_duration;
		double m_timeBegin;

		bool m_animationStarted;

		AnimationEase m_ease;
	};

	template <typename T>
	class RepeatedSpaceAnimation : public Animation <T>
	{
	public:
		RepeatedSpaceAnimation(const T min, const T max)
			: Animation<T>(),
			  _min(min),
			  _max(max)
		{

		}

		RepeatedSpaceAnimation(const T min, const T max, T value, double duration, AnimationEase ease)
			: Animation<T>(value, duration, ease),
			  _min(min),
			  _max(max)
		{

		}

	protected:
		float relocateValue(float from, float to, const float min, const float max, const float elapsed)
    {
      auto l = max - min;

      while (from < min)
				from += l;
			while (to < min)
				to += l;
			while (from > max)
				from -= l;
			while (to > max)
				to -= l;

      auto ab    = to - from;
      auto ababs = std::abs(ab);

      if (ababs <= l / 2.0f)
			{
				return from + ab * elapsed;
			}

			auto result = from - (ab * elapsed * ((l - ababs) / ababs));

			if (result < max)
			{
				result += l;
			}
			else if (result > max)
			{
				result -= l;
			}

			return result;
    }

    T _min;
		T _max;
	};

	class RepeatedScalarfAnimation : public RepeatedSpaceAnimation <float>
	{
	public:
		RepeatedScalarfAnimation(const float min, const float max)
			: RepeatedSpaceAnimation <float>(min, max)
		{

		}

		RepeatedScalarfAnimation(const float min, const float max, float value, double duration, AnimationEase ease)
			: RepeatedSpaceAnimation <float>(min, max, value, duration, ease)
		{

		}

	protected:
		float relocate(const float& from, const float& to, float elapsed) override
		{
			return relocateValue(from, to, _min, _max, elapsed);
		}
	};

	template <typename T>
	class RepeatedVectorfAnimation : public RepeatedSpaceAnimation <T>
	{
	public:
		RepeatedVectorfAnimation(const T min, const T max)
			: RepeatedSpaceAnimation <T>(min, max)
		{

		}

		RepeatedVectorfAnimation(const T min, const T max, T value, double duration, AnimationEase ease)
			: RepeatedSpaceAnimation <T>(min, max, value, duration, ease)
		{

		}

	protected:
		T relocate(const T& from, const T& to, float elapsed) override
		{
			T result;

			for (auto i = 0; i < T::num_components; i++)
			{
				result._v[i] = RepeatedSpaceAnimation<T>::relocateValue(from._v[i], to._v[i], RepeatedSpaceAnimation <T>::_min._v[i], RepeatedSpaceAnimation <T>::_max._v[i], elapsed);
			}

			return result;
		}
	};

  using RepeatedVec2fAnimation = RepeatedVectorfAnimation<osg::Vec2f>;
  using RepeatedVec3fAnimation = RepeatedVectorfAnimation<osg::Vec3f>;
  using RepeatedVec4fAnimation = RepeatedVectorfAnimation<osg::Vec4f>;
}
