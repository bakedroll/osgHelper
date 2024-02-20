#include <osgHelper/Camera.h>
#include <osgHelper/Helper.h>

#include <osg/MatrixTransform>

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

osg::ref_ptr<osg::Node> Camera::createScreenQuad()
{
  auto geo = new osg::Geometry();

  auto verts = new osg::Vec3Array();
  verts->push_back(osg::Vec3f(-1.0f, -1.0f, 0.0f));
  verts->push_back(osg::Vec3f(-1.0f,  1.0f, 0.0f));
  verts->push_back(osg::Vec3f( 1.0f, -1.0f, 0.0f));
  verts->push_back(osg::Vec3f( 1.0f,  1.0f, 0.0f));

  auto indices = new osg::DrawElementsUInt(osg::PrimitiveSet::POLYGON, 0);

  indices->push_back(2);
  indices->push_back(3);
  indices->push_back(1);
  indices->push_back(0);

  auto texcoords = new osg::Vec2Array();
  texcoords->push_back(osg::Vec2(0.0f, 0.0f));
  texcoords->push_back(osg::Vec2(0.0f, 1.0f));
  texcoords->push_back(osg::Vec2(1.0f, 0.0f));
  texcoords->push_back(osg::Vec2(1.0f, 1.0f));

  geo->setVertexArray(verts);
  geo->setTexCoordArray(0, texcoords);
  geo->addPrimitiveSet(indices);

  auto quadGeode = new osg::Geode();
  quadGeode->addDrawable(geo);

  auto transform = new osg::MatrixTransform();
  transform->addChild(quadGeode);

  m_screenQuads.emplace_back(transform);

  return transform;
}

void Camera::removeScreenQuad(const osg::ref_ptr<osg::Node>& node)
{
  for (auto it = m_screenQuads.begin(); it != m_screenQuads.end(); ++it)
  {
    if (*it == node)
    {
      m_screenQuads.erase(it);
      break;
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

osg::Vec2i Camera::getResolution() const
{
  return m_resolution;
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

  updateProjectionMatrix();
  updateModelViewMatrix();
  updateCameraAlignedQuads();
  updateScreenQuads();
}

void Camera::setAttitude(const osg::Quat& attitude)
{
  m_attitude = attitude;
  updateModelViewMatrix();
  updateCameraAlignedQuads();
  updateScreenQuads();
}

void Camera::setNearFar(double near, double far)
{
  m_angleNearFarRatio.y() = near;
  m_angleNearFarRatio.z() = far;
  updateProjectionMatrix();
}

void Camera::updateResolution(const osg::Vec2i& resolution)
{
  m_resolution = resolution;
  updateProjectionMatrix();
  updateCameraAlignedQuads();
  updateScreenQuads();

  for (const auto& callback : m_updateResolutionCallbacks)
  {
    callback(resolution);
  }
}

void Camera::pickLine(float x, float y, osg::Vec3f& origin, osg::Vec3f& target) const
{
  const auto mappedX = (x * 2.0f) / m_resolution.x() - 1.0f;
  const auto mappedY = (y * 2.0f) / m_resolution.y() - 1.0f;

  const osg::Vec3f near(mappedX, mappedY, -1.0f);
  const osg::Vec3f far(mappedX, mappedY, 1.0f);

  const auto mat = osg::Matrix::inverse(getViewMatrix() * getProjectionMatrix());

  origin = near * mat;
  target = far * mat;
}

void Camera::pickRay(float x, float y, osg::Vec3f& point, osg::Vec3f& direction) const
{
  osg::Vec3f target;
  pickLine(x, y, point, target);

  direction = target - point;
  direction.normalize();
}

void Camera::registerUpdateResolutionCallback(const UpdateResolutionCallback& callback)
{
  m_updateResolutionCallbacks.emplace_back(callback);
}

void Camera::updateProjectionMode()
{
  if (m_mode == ProjectionMode::Perspective)
  {
    setReferenceFrame(osg::Camera::RELATIVE_RF);
  }
  else if (m_mode == ProjectionMode::Ortho2D ||
    m_mode == ProjectionMode::Ortho2DRatio)
  {
    setReferenceFrame(osg::Camera::ABSOLUTE_RF);
  }

  updateModelViewMatrix();
  updateProjectionMatrix();
  updateCameraAlignedQuads();
  updateScreenQuads();
}

void Camera::updateModelViewMatrix()
{
  switch (m_mode)
  {
  case ProjectionMode::Ortho2DRatio:
  case ProjectionMode::Ortho2D:
  {
    setViewMatrix(osg::Matrixd::identity());
    break;
  }
  case ProjectionMode::Perspective:
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
    break;
  }
  default:
    break;
  }
}

void Camera::updateProjectionMatrix()
{
  switch (m_mode)
  {
  case ProjectionMode::Ortho2DRatio:
  {
    const auto width = m_resolution.x();
    const auto height = m_resolution.y();
    const auto x = m_position.x();
    const auto y = m_position.y();
    const auto zoom = m_position.z();
    if (width > 0 && height > 0)
    {
      const auto ratio = static_cast<float>(height) / static_cast<float>(width);
      const auto halfWidth = zoom == 0.0f ? 1.0f : 1.0f / zoom;
      const auto halfHeight = zoom == 0.0f ? ratio : ratio / zoom;
      setProjectionMatrix(osg::Matrixd::ortho2D(x - halfWidth, x + halfWidth, y - halfHeight, y + halfHeight));
    }

    break;
  }
  case  ProjectionMode::Ortho2D:
  {
    setProjectionMatrix(osg::Matrixd::identity());
    break;
  }
  case ProjectionMode::Perspective:
  {
    m_angleNearFarRatio.w() =
      (m_resolution.y() == 0) ? 1.0f : static_cast<float>(m_resolution.x()) / m_resolution.y();

    setProjectionMatrix(osg::Matrix::perspective(m_angleNearFarRatio.x(), m_angleNearFarRatio.w(),
      m_angleNearFarRatio.y(), m_angleNearFarRatio.z()));

    break;
  }
  default:
    break;
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

void Camera::updateScreenQuads()
{
  const auto matrix = osg::Matrix::inverse(getProjectionMatrix()) * osg::Matrix::inverse(getViewMatrix());

  for (auto& transform : m_screenQuads)
  {
    transform->setMatrix(matrix);
  }
}

}
