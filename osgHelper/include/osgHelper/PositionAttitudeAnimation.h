#pragma once

#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osg/PositionAttitudeTransform>

#include <osgHelper/Animation.h>
#include <osgHelper/Helper.h>
#include <osgHelper/Camera.h>

namespace osgHelper
{

template <typename T,
          typename = typename std::enable_if<std::is_base_of<osg::PositionAttitudeTransform, T>::value ||
                                             std::is_base_of<osgHelper::Camera, T>::value>::type>
class PositionAttitudeAnimation : public osg::Referenced
{
public:
  PositionAttitudeAnimation(const osg::ref_ptr<T>& node)
    : PositionAttitudeAnimation(node, 0.0, AnimationEase::SMOOTH)
  {
    
  }

  PositionAttitudeAnimation(const osg::ref_ptr<T>& node, double duration, AnimationEase ease)
    : osg::Referenced()
    , m_positionAnimation(node->getPosition(), duration, ease)
    , m_eulerRotationAnimation(osg::Vec3f(0.0f, 0.0f, 0.0f), osg::Vec3f(C_2PI, C_2PI, C_2PI),
                               osg::Vec3f(), duration, ease)
    , m_targetPosition(node->getPosition())
    , m_node(node)
  {
  }

  osg::Vec3f getTargetPosition() const
  {
    return m_targetPosition;
  }

  osg::Vec3f getTargetEulerAttitude() const
  {
    return m_targetEulerAttitude;
  }

	void setPosition(const osg::Vec3f& position)
  {
    m_targetPosition = position;
    m_positionAnimation.setValue(position);
    m_node->setPosition(position);
  }

	void setEulerAttitude(const osg::Vec3f& eulerAttitude)
  {
    m_targetEulerAttitude = eulerAttitude;
    m_eulerAttitude = eulerAttitude;
    m_eulerRotationAnimation.setValue(eulerAttitude);
    m_node->setAttitude(osgHelper::getQuatFromEuler(eulerAttitude.x(), eulerAttitude.y(), eulerAttitude.z()));
  }

  void setTargetPosition(const osg::Vec3f& position, double time)
  {
    m_targetPosition = position;

    m_positionAnimation.beginAnimation(m_node->getPosition(), position, time);
    m_eulerRotationAnimation.beginAnimation(m_eulerAttitude, m_targetEulerAttitude, time);
  }

  void setTargetEulerAttitude(const osg::Vec3f& eulerAttitude, double time)
  {
    m_targetEulerAttitude = eulerAttitude;

    m_positionAnimation.beginAnimation(m_node->getPosition(), m_targetPosition, time);
    m_eulerRotationAnimation.beginAnimation(m_eulerAttitude, eulerAttitude, time);
  }

	void updateAnimation(double time)
  {
    m_eulerAttitude = m_eulerRotationAnimation.getValue(time);

    m_node->setPosition(m_positionAnimation.getValue(time));
    m_node->setAttitude(osgHelper::getQuatFromEuler(m_eulerAttitude.x(), m_eulerAttitude.y(), m_eulerAttitude.z()));
  }

  void stopAnimation()
  {
    setPosition(m_node->getPosition());
    setEulerAttitude(m_eulerAttitude);
  }

	void setDuration(double time)
  {
    m_positionAnimation.setDuration(time);
    m_eulerRotationAnimation.setDuration(time);
  }

  void setEase(AnimationEase ease)
  {
    m_positionAnimation.setEase(ease);
    m_eulerRotationAnimation.setEase(ease);
  }

	bool running() const
	{
    return m_positionAnimation.running();
	}

private:
  Animation<osg::Vec3f> m_positionAnimation;
  RepeatedVec3fAnimation m_eulerRotationAnimation;

  osg::Vec3f m_eulerAttitude;

  osg::Vec3f m_targetPosition;
  osg::Vec3f m_targetEulerAttitude;

  osg::ref_ptr<T> m_node;

};

using PositionAttitudeTransformAnimation = PositionAttitudeAnimation<osg::PositionAttitudeTransform>;
using CameraAnimation                    = PositionAttitudeAnimation<osgHelper::Camera>;

}