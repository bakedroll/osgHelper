#pragma once

#include <osgHelper/CameraAlignedQuad.h>

#include <osg/Camera>

#include <functional>

namespace osgHelper
{
  class Camera : public osg::Camera
  {
  public:
    using Ptr = osg::ref_ptr<Camera>;

    enum class ProjectionMode
    {
      Perspective,
      Ortho2D,
      Ortho2DRatio
    };

    using UpdateResolutionCallback = std::function<void(const osg::Vec2i&)>;

    explicit Camera(ProjectionMode mode = ProjectionMode::Perspective);
    explicit Camera(const osg::Camera& camera, const osg::CopyOp& copyOp = osg::CopyOp::SHALLOW_COPY);
    ~Camera() override;

    void addCameraAlignedQuad(const osg::ref_ptr<CameraAlignedQuad>& caq);
    void removeCameraAlignedQuad(const osg::ref_ptr<CameraAlignedQuad>& caq);

    osg::ref_ptr<osg::Node> createScreenQuad();
    void removeScreenQuad(const osg::ref_ptr<osg::Node>& node);

    osg::Vec3f getPosition() const;
    osg::Quat getAttitude() const;
    osg::Vec2i getResolution() const;

    double getProjectionNear() const;
    double getProjectionFar() const;
    double getProjectionAngle() const;
    double getProjectionRatio() const;

    osg::Vec3f getLookDirection() const;

    void setProjectionMode(ProjectionMode mode);

    void setPosition(const osg::Vec3f& position);
    void setAttitude(const osg::Quat& attitude);
    void setNearFar(double near, double far);

    void updateResolution(const osg::Vec2i& resolution);

    void pickLine(float x, float y, osg::Vec3f& origin, osg::Vec3f& target) const;
    void pickRay(float x, float y, osg::Vec3f& point, osg::Vec3f& direction) const;

    void registerUpdateResolutionCallback(const UpdateResolutionCallback& callback);

  private:
    using CameraAlignedQuadList = std::vector<osg::ref_ptr<CameraAlignedQuad>>;
    using ScreenQuadList        = std::vector<osg::ref_ptr<osg::MatrixTransform>>;

    void updateProjectionMode();
    void updateModelViewMatrix();
    void updateProjectionMatrix();
    void updateCameraAlignedQuads();
    void updateScreenQuads();

    ProjectionMode m_mode;
    osg::Vec3f     m_position;
    osg::Quat      m_attitude;
    osg::Vec3f     m_lookDirection;

    osg::Vec4d m_angleNearFarRatio;

    osg::Vec2i m_resolution;

    CameraAlignedQuadList m_cameraAlignedQuads;
    ScreenQuadList m_screenQuads;

    std::vector<UpdateResolutionCallback> m_updateResolutionCallbacks;
  };
}