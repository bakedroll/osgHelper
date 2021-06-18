#pragma once

#include <osgHelper/CameraAlignedQuad.h>

#include <osg/Camera>

namespace osgHelper
{
  class Camera : public osg::Camera
  {
  public:
    using Ptr = osg::ref_ptr<Camera>;

    enum class ProjectionMode
    {
      Perspective,
      Ortho2D
    };

    explicit Camera(ProjectionMode mode = ProjectionMode::Perspective);
    explicit Camera(const osg::Camera& camera, const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY);
    ~Camera() override;

    void addCameraAlignedQuad(const osg::ref_ptr<CameraAlignedQuad>& caq);
    void removeCameraAlignedQuad(const osg::ref_ptr<CameraAlignedQuad>& caq);

    osg::Vec3f getPosition() const;
    osg::Quat getAttitude() const;

    double getProjectionNear() const;
    double getProjectionFar() const;
    double getProjectionAngle() const;
    double getProjectionRatio() const;

    osg::Vec3f getLookDirection() const;

    void setProjectionMode(ProjectionMode mode);

    void setPosition(const osg::Vec3f& position);
    void setAttitude(const osg::Quat& attitude);

    void updateResolution(const osg::Vec2i& resolution);

    void pickRay(float x, float y, osg::Vec3f& point, osg::Vec3f& direction) const;

  private:
    using CameraAlignedQuadList = std::vector<osg::ref_ptr<CameraAlignedQuad>>;

    void updateProjectionMode();
    void updateModelViewMatrix();
    void updateProjectionMatrix();
    void updateCameraAlignedQuads();

    ProjectionMode m_mode;
    osg::Vec3f     m_position;
    osg::Quat      m_attitude;
    osg::Vec3f     m_lookDirection;

    osg::Vec4d m_angleNearFarRatio;

    osg::Vec2i m_resolution;

    CameraAlignedQuadList m_cameraAlignedQuads;
  };
}