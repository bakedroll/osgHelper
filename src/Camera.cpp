#include <osgHelper/Camera.h>
#include <osgHelper/Helper.h>

namespace osgHelper
{
  Camera::Camera(ProjectionMode mode)
    : osg::Camera()
    , m_mode(mode)
    , m_angleNearFarRatio(30.0, 1.0, 100.0, 1.0)
  {
    updateProjectionMode();
  }

  Camera::Camera(const osg::Camera& camera, const osg::CopyOp& copyOp)
    : osg::Camera(camera, copyOp)
    , m_mode(ProjectionMode::Perspective)
    , m_angleNearFarRatio(30.0, 1.0, 100.0, 1.0)
  {
    updateProjectionMode();
  }

  Camera::~Camera() = default;

  void Camera::addCameraAlignedQuad(const osg::ref_ptr<CameraAlignedQuad>& caq)
  {
    m_cameraAlignedQuads.emplace_back(caq);
  }

  void Camera::removeCameraAlignedQuad(const osg::ref_ptr<CameraAlignedQuad>& caq)
  {
    for (auto it = m_cameraAlignedQuads.begin(); it != m_cameraAlignedQuads.end(); ++it)
    {
      if ((*it) == caq)
      {
        m_cameraAlignedQuads.erase(it);
        return;
      }
    }
  }

  osg::Vec3f Camera::getPosition() const
  {
    return m_position;
  }

  osg::Quat Camera::getAttitude() const
  {
    return m_attitude;
  }

  double Camera::getProjectionNear() const
  {
    return m_angleNearFarRatio.y();
  }

  double Camera::getProjectionFar() const
  {
    return m_angleNearFarRatio.z();
  }

  double Camera::getProjectionAngle() const
  {
    return m_angleNearFarRatio.x();
  }

  double Camera::getProjectionRatio() const
  {
    return m_angleNearFarRatio.w();
  }

  osg::Vec3f Camera::getLookDirection() const
  {
    return m_lookDirection;
  }

  void Camera::setProjectionMode(ProjectionMode mode)
  {
    if (m_mode == mode)
    {
      return;
    }

    m_mode = mode;
    updateProjectionMode();
  }

  void Camera::setPosition(const osg::Vec3f& position)
  {
    m_position = position;

    updateModelViewMatrix();
    updateCameraAlignedQuads();
  }

  void Camera::setAttitude(const osg::Quat& attitude)
  {
    m_attitude = attitude;
    updateModelViewMatrix();
    updateCameraAlignedQuads();
  }

  void Camera::updateResolution(const osg::Vec2i& resolution)
  {
    m_resolution = resolution;
    updateProjectionMatrix();
    updateCameraAlignedQuads();
  }

  void Camera::pickRay(float x, float y, osg::Vec3f& point, osg::Vec3f& direction) const
  {
    const auto mappedX = (x * 2.0f) / m_resolution.x() - 1.0f;
    const auto mappedY = (y * 2.0f) / m_resolution.y() - 1.0f;

    const osg::Vec3f near(mappedX, mappedY, -1.0f);
    const osg::Vec3f far(mappedX, mappedY, 1.0f);

    const auto mat = osg::Matrix::inverse(getViewMatrix() * getProjectionMatrix());

    point     = near * mat;
    direction = (far * mat) - point;

    direction.normalize();
  }

  void Camera::updateProjectionMode()
  {
    if (m_mode == ProjectionMode::Perspective)
    {
      setReferenceFrame(osg::Camera::RELATIVE_RF);
    }
    else if (m_mode == ProjectionMode::Ortho2D)
    {
      setReferenceFrame(osg::Camera::ABSOLUTE_RF);
    }

    updateModelViewMatrix();
    updateProjectionMatrix();
    updateCameraAlignedQuads();
  }

  void Camera::updateModelViewMatrix()
  {
    if (m_mode == ProjectionMode::Ortho2D)
    {
      setViewMatrix(osg::Matrixd::identity());
    }
    else if (m_mode == ProjectionMode::Perspective)
    {
      osg::Vec3f eye(0.0f, 0.0f, 0.0f);
      osg::Vec3f center(0.0f, 1.0f, 0.0f);
      osg::Vec3f up(0.0f, 0.0f, 1.0f);

      osg::Matrixd mat;
      mat.setRotate(m_attitude);
      mat.setTrans(m_position);

      transformVector(&eye, &mat);
      transformVector(&center, &mat);
      transformVector(&up, &mat);

      m_lookDirection = center;

      up -= m_position;

      setViewMatrix(osg::Matrix::lookAt(eye, center, up));
    }
  }

  void Camera::updateProjectionMatrix()
  {
    if (m_mode == ProjectionMode::Ortho2D)
    {
      setProjectionMatrix(osg::Matrixd::identity());
    }
    else if (m_mode == ProjectionMode::Perspective)
    {
      m_angleNearFarRatio.w() =
              (m_resolution.y() == 0) ? 1.0f : static_cast<float>(m_resolution.x()) / m_resolution.y();

      setProjectionMatrix(osg::Matrix::perspective(m_angleNearFarRatio.x(), m_angleNearFarRatio.w(),
                                                   m_angleNearFarRatio.y(), m_angleNearFarRatio.z()));
    }
  }

  void Camera::updateCameraAlignedQuads()
  {
    if (m_cameraAlignedQuads.empty() || (m_mode != ProjectionMode::Perspective))
    {
      return;
    }

    osg::Vec3f v[] = {osg::Vec3f(-1.0f, -1.0f, -1.0f), osg::Vec3f(-1.0f, 1.0f, -1.0f), osg::Vec3f(1.0f, 1.0f, -1.0f),
                      osg::Vec3f(1.0f, -1.0f, -1.0f)};

    const osg::Vec3f n(0.0f, 0.0f, 1.0f);

    osg::Vec3f v_res[4];
    osg::Vec3f n_res[4];

    const auto mat = osg::Matrix::inverse(getViewMatrix() * getProjectionMatrix());

    for (auto i = 0; i < 4; i++)
    {
      v_res[i] = v[i] * mat;
      n_res[i] = ((v[i] + n) * mat) - v_res[i];
      n_res[i].normalize();
    }

    for (const auto& caq : m_cameraAlignedQuads)
    {
      auto verts   = caq->getVertexArray();
      auto normals = caq->getNormalArray();

      for (int i = 0; i < 4; i++)
      {
        verts->at(i).set(v_res[i]);
        normals->at(i).set(n_res[i]);
      }

      normals->dirty();
      verts->dirty();
      caq->getGeometry()->dirtyBound();
    }
  }
}
